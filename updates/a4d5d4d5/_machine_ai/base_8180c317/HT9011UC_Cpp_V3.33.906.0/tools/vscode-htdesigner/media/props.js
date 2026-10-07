/* AI(W906-HTDESIGNER) 20260929: the 屬性與事件 panel (a sidebar webview).
   Everything is built with textContent (never innerHTML): the values come from the
   page and the source trees and are shown, not interpreted. */
(function () {
  'use strict';
  // 0.149 (EastSun: "還沒選到元件時 請用 button 參數 然後全部反灰 讓人知道裡面可以設定啥東西"): nothing selected = a button's
  // properties and events (media/props_sample.json, sent by the extension), all grey, nothing sent from them.
  // (VS Code's API object is frozen -- assigning to it throws under 'use strict' and the panel stays blank: wrap it)
  var api = acquireVsCodeApi();
  var sample = null;
  var sampleMode = false;
  var vscode = {
    /* (1006 audit: every message says which component the panel was showing -- the extension refuses an edit meant for
       another one than is selected now: a pending number, a menu or the Font dialog used to land on the NEW selection) */
    postMessage: function (m) {
      if (sampleMode && m && m.type !== 'ready') return;
      if (m && typeof m === 'object' && data && data.comp && data.comp.key != null) m.forKey = data.comp.key;
      api.postMessage(m);
    },
    getState: function () { return api.getState(); },
    setState: function (s) { if (sampleMode) return; api.setState(s); },
  };
  var root = document.getElementById('root');
  /* 1006 audit: what is still pending for the component shown (a number run's write, a scrub, a menu, the Font dialog) --
     settled for IT before the panel shows another one */
  var PENDING = [];
  function pending(fn) { PENDING.push(fn); return function () { var i = PENDING.indexOf(fn); if (i >= 0) PENDING.splice(i, 1); }; }
  function settle() {
    var a = document.activeElement;
    /* the field being typed in: its value goes to the component it belongs to (WPF: leaving a field commits it) */
    if (a && a.getAttribute && a.getAttribute('data-field') && /^(INPUT|TEXTAREA|SELECT)$/.test(a.tagName) && root.contains(a)) {
      try { a.dispatchEvent(new Event('change', { bubbles: true })); } catch (x) { /* ignore */ }
    }
    var list = PENDING.slice(); PENDING.length = 0;
    list.forEach(function (fn) { try { fn(); } catch (x) { /* ignore */ } });
  }
  /* 1006 (the user: Ctrl+Z for the components): Ctrl+Z / Ctrl+Y / Ctrl+Shift+Z with no text box in use = the page's undo /
     redo (the panel itself has nothing to undo); in a text box they stay the box's own (its typing) */
  document.addEventListener('keydown', function (e) {
    if (!(e.ctrlKey || e.metaKey) || e.altKey) return;
    var k = String(e.key || '').toLowerCase();
    var undo = k === 'z' && !e.shiftKey, redo = k === 'y' || (k === 'z' && e.shiftKey);
    if (!undo && !redo) return;
    var t = e.target;
    var typing = t && (t.isContentEditable || t.tagName === 'TEXTAREA' || (t.tagName === 'INPUT' && !/^(checkbox|radio|button|color|range)$/i.test(t.type || '')));
    if (typing) return;
    e.preventDefault();
    api.postMessage({ type: undo ? 'undo' : 'redo' });
  }, true);
  /* 1005 (WPF audit: the property grid's description pane): the row being edited / pointed at -- its name and what it
     does -- in a pane under the grid that stays put (outside #root, which every render clears). The text is the same
     as the name cell's tooltip (describeKey). */
  var descPane = document.createElement('div');
  descPane.id = 'descPane';
  descPane.innerHTML = '<div class="dn"></div><div class="dd"></div>';
  document.body.appendChild(descPane);
  function showDesc(tr) {
    var k = tr && tr.querySelector && tr.querySelector('td.k.hasdesc');
    var dn = descPane.firstChild, dd = descPane.lastChild;
    if (!k) { if (!descPane.classList.contains('on')) { dn.textContent = ''; dd.textContent = '點一個屬性，這裡說明它是做什麼的'; } return; }
    descPane.classList.add('on');
    dn.textContent = k.getAttribute('data-dname') || '';
    dd.textContent = k.getAttribute('data-ddesc') || '';
  }
  showDesc(null);
  /* 1005 (Blend): press on a number row's name and drag = change its value (num() gives the field its _scrubStart) */
  document.addEventListener('pointerdown', function (e) {
    var k = e.target.closest && e.target.closest('table.edit td.k');
    var n = k && k.parentNode && k.parentNode.querySelector('td.v input.num');
    if (n && n._scrubStart) n._scrubStart(e);
  });
  document.addEventListener('focusin', function (e) { var tr = e.target.closest && e.target.closest('tr'); if (tr) showDesc(tr); });
  document.addEventListener('mouseover', function (e) { var tr = e.target.closest && e.target.closest('tr'); if (tr && tr.querySelector('td.k.hasdesc')) showDesc(tr); });
  var data = null;
  var state = vscode.getState() || { q: '', closed: {} };
  if (state.q == null) state.q = state.filter || '';   // (the DFM section's own filter became the search box)
  if (state.view !== 'events' && state.view !== 'code') state.view = 'props';
  var searching = false;

  function save() { vscode.setState(state); }

  /* WPF's Properties window shows ONE list at a time: the properties, or (the Events button, the lightning bolt)
     the events (learn.microsoft.com "Create UIs with Visual Studio XAML Designer" > Properties window) */
  var EVENT_SECS = { events: 1, jsevents: 1 };
  /* (EastSun 20260930: the property and event lists as clear as WPF's -- what the code does with the component, the
     page's listeners, the commands it sends, its data field, is a third list of its own, not under the properties) */
  var CODE_SECS = { listeners: 1, mentions: 1, cmds: 1, fields: 1, tags: 1, uses: 1 };
  function viewOf(key) { return EVENT_SECS[key] ? 'events' : CODE_SECS[key] ? 'code' : 'props'; }
  function inView(key) { return viewOf(key) === state.view; }
  /* WPF's "Arrange by": Category is its default */
  function arrangeBy() { return state.propSort === 'name' || state.propSort === 'dfm' ? state.propSort : 'cat'; }

  /* a text box that commits ONCE: Enter (or its key release -- a suggestion list may take the key press), a name
     picked from its list, leaving it; Esc puts it back (EastSun: "我更改alias後按下enter需要操作兩次") */
  function textBox(field, value, onCommit, list) {
    var i = el('input', 'txt');
    i.type = 'text'; i.value = value || ''; i.spellcheck = false;
    i.setAttribute('data-field', field);
    if (list) i.setAttribute('list', list);
    var sent = value || '';
    var commit = function () { var v = i.value.trim(); if (v === sent) return; sent = v; onCommit(v); };
    i.addEventListener('change', commit);
    i.addEventListener('keydown', function (e) {
      if (e.key === 'Enter') { e.preventDefault(); commit(); }
      else if (e.key === 'Escape') { i.value = sent; i.blur(); }
    });
    i.addEventListener('keyup', function (e) { if (e.key === 'Enter') commit(); });
    i.addEventListener('input', function (e) { if (list && e.inputType === 'insertReplacementText') commit(); });
    return i;
  }

  function el(tag, cls, text) {
    var e = document.createElement(tag);
    if (cls) e.className = cls;
    if (text != null) e.textContent = String(text);
    return e;
  }
  function base(p) { return String(p || '').split(/[\\/]/).pop(); }
  /* the DFM property's category (by its first part: Font.Size -> Font): BCB6's Object
     Inspector groups, in the four a user of this designer looks for */
  var PROP_CATS = ['版面', '外觀', '行為', '其他'];
  var CAT_OF = {
    '版面': /^(Left|Top|Width|Height|Align|Anchors|Constraints|AutoSize|ClientWidth|ClientHeight|ClientLeft|ClientTop|BorderWidth|Margins|AlignWithMargins|Position|Scaled|PixelsPerInch|HorzScrollBar|VertScrollBar|AutoScroll|DefaultMonitor)$/,
    '外觀': /^(Caption|Text|Color|Font|ParentFont|ParentColor|Visible|Bevel\w*|BorderStyle|BorderIcons|Ctl3D|ParentCtl3D|Transparent|Picture|Glyph|Layout|Alignment|WordWrap|Flat|Style|Brush|Pen|Shape|Stretch|Center|Proportional|Icon|Kind|NumGlyphs|Spacing|FormStyle|WindowState|TitleFont|FixedColor|Lines|Items|Tabs|Columns|ShowCaption|FullRepaint|Images|ImageIndex|Orientation)$/,
    '行為': /^(Enabled|TabOrder|TabStop|ReadOnly|Cursor|Hint|ShowHint|ParentShowHint|PopupMenu|Default|Cancel|ModalResult|Checked|State|AllowGrayed|ItemIndex|ItemHeight|MaxLength|GroupIndex|Down|AllowAllUp|Interval|TabIndex|ActivePage|DragKind|DragMode|DragCursor|KeyPreview|Sorted|MultiSelect|Options|PasswordChar|CharCase|HideSelection|ScrollBars|WantReturns|WantTabs|Action|Increment|Min|Max|MaxValue|MinValue|Value|Frequency)$/,
  };
  function propCat(name) {
    var head = String(name || '').split('.')[0];
    for (var i = 0; i < 3; i++) if (CAT_OF[PROP_CATS[i]].test(head)) return PROP_CATS[i];
    return '其他';
  }
  /* WPF's Properties window: the mouse on a property's name says what the property is (the VCL property, in
     plain words); Font.Size -> its own line, else the part before the dot (Font) */
  var PROP_DESC = {
    Name: '元件的名稱：程式用這個名字找它（HTML 的 id）',
    Alias: '這個元件代表的 IO 名稱（寫在 title 裡，網頁程式照它對 IO）',
    // the IO lamp (TALed / TMyLed / TMyLedLane) and panel button (TBtnPanel / TBtnPanelLane) -- the company's own components
    LEDStyle: '燈的形狀：LEDSmall／LEDLarge 圓、LEDSqSmall／LEDSqLarge 方、LEDVertical 直、LEDHorizontal 橫',
    Value: '燈亮著（設計時畫成亮的；機台上由它的 Alias 那個 IO 決定）', Blink: '燈閃爍（Interval 是半週期）',
    Style: '按鈕樣式：tsButtons 立體、tsFlatButtons 平面（新版畫面）',
    Down: '按鈕按下的樣子（設計時畫成按下；機台上由輸出的回讀決定）',
    TrueColor: '亮（燈）／按下（按鈕）時的顏色', FalseColor: '滅（燈）／沒按（按鈕）時的顏色',
    TrueFontColor: '按鈕按下時的字色', FalseFontColor: '按鈕沒按時的字色',
    Left: '左邊到容器左邊的距離（像素）', Top: '上面到容器上面的距離（像素）', Width: '寬度（像素）', Height: '高度（像素）',
    Caption: '顯示在元件上的文字', Text: '元件裡的文字（輸入框的內容）', Color: '背景顏色',
    Font: '文字的字型', 'Font.Name': '字型名稱', 'Font.Size': '字的大小', 'Font.Color': '字的顏色', 'Font.Style': '粗體／斜體／底線／刪除線',
    'Font.Bold': '粗體', 'Font.Italic': '斜體', 'Font.Height': '字高（像素；負數＝不含行距）', 'Font.Charset': '字元集（中文字型用哪一套字）',
    Visible: '執行時看不看得到', Enabled: '可不可以操作（關掉＝變灰、不能點）', AutoSize: '大小自動跟著內容（文字）',
    Alignment: '文字靠左／置中／靠右', WordWrap: '文字太長時自動換行', Transparent: '背景透明（看得到後面的東西）',
    Hint: '滑鼠停在上面時顯示的提示', ShowHint: '要不要顯示 Hint', ParentShowHint: 'ShowHint 跟著容器',
    TabOrder: '按 Tab 鍵時輪到它的順序', TabStop: '按 Tab 鍵會不會停在它上面', ParentFont: '字型跟著容器', ParentColor: '顏色跟著容器',
    Align: '貼齊容器的哪一邊（上／下／左／右／填滿）', Anchors: '容器改大小時，它跟著哪幾邊',
    BevelInner: '內側的立體框', BevelOuter: '外側的立體框', BevelWidth: '立體框的寬度', BorderStyle: '邊框樣式', BorderWidth: '邊框和內容之間的距離',
    Cursor: '滑鼠移到上面時的游標', PopupMenu: '按右鍵時出現的選單', ReadOnly: '只能看、不能改', MaxLength: '最多可以打幾個字（0＝不限制）',
    PasswordChar: '輸入時顯示成這個字（密碼用）', Checked: '有沒有打勾', Down: '按鈕是不是按下的狀態',
    GroupIndex: '同一組（同一個數字）的按鈕一次只有一個按下', AllowAllUp: '同一組的按鈕可以全部放開',
    Glyph: '按鈕上的圖', NumGlyphs: '圖裡有幾個狀態（一般／變灰／按下…）', Layout: '圖和文字的相對位置', Spacing: '圖和文字之間的距離',
    Flat: '平面外觀（滑鼠移上去才出現框）', ItemIndex: '目前選的是第幾項（-1＝沒選）', Items: '清單的項目', Lines: '多行文字的內容',
    Interval: '計時器多久觸發一次（毫秒）', Picture: '顯示的圖片', Stretch: '圖片拉到跟元件一樣大', Center: '圖片置中', Proportional: '圖片等比例縮放',
    Shape: '形狀（方形、圓形…）', Brush: '填滿的顏色／樣式', Pen: '外框線的顏色／樣式', ActivePage: '目前顯示的分頁', TabIndex: '目前顯示第幾個分頁',
    Style: '外觀樣式', ModalResult: '按下後對話框回傳的結果', Default: '按 Enter 等於按這個鈕', Cancel: '按 Esc 等於按這個鈕',
    ClientWidth: '內部可以放東西的寬度', ClientHeight: '內部可以放東西的高度', Constraints: '最小／最大的寬高',
    ScrollBars: '捲軸', Orientation: '方向（水平／垂直）', Min: '最小值', Max: '最大值', Position: '位置（表單＝出現在螢幕哪裡；捲軸／進度條＝目前的值）',
  };
  function propDesc(name) {
    var n = String(name || '');
    return PROP_DESC[n] || PROP_DESC[n.split('.')[0]] || '';
  }
  /* the name cell: its description as the tooltip (more = what the row itself says, e.g. double-click to the .dfm) */
  function describeKey(td, name, more) {
    var d = propDesc(name);
    if (!d) return;
    td.title = name + '\n' + d + (more ? '\n\n' + more : '');
    td.classList.add('hasdesc');
    td.setAttribute('data-dname', name);
    td.setAttribute('data-ddesc', d + (more ? '　（' + more + '）' : ''));
  }

  /* WPF's property grid: a category's heading folds it away / opens it again (remembered per category -- the
     editable section's and the DFM section's apart); a search shows what it finds in a folded one too */
  function catToggle(node, label, key) {
    node.setAttribute('data-catkey', key);
    node.classList.add('cattog');
    node.tabIndex = 0;
    node.title = '點一下（或 Enter）：收合／展開這一類';
    label.insertBefore(el('span', 'tw'), label.firstChild);
    var flip = function () {
      state.catClosed = state.catClosed || {};
      if (state.catClosed[key]) delete state.catClosed[key]; else state.catClosed[key] = true;
      save();
      applyCats();
    };
    node.addEventListener('click', flip);
    node.addEventListener('keydown', function (e) { if (e.key === 'Enter' || e.key === ' ') { e.preventDefault(); flip(); } });
  }
  function applyCats() {
    var q = String(state.q || '').trim();
    var cc = state.catClosed || {};
    Array.prototype.forEach.call(root.querySelectorAll('[data-catkey]'), function (h) {
      var closed = !q && !!cc[h.getAttribute('data-catkey')];
      h.classList.toggle('closed', closed);
      h.setAttribute('aria-expanded', closed ? 'false' : 'true');
      var tw = h.querySelector('.tw');
      if (tw) tw.textContent = closed ? '▸' : '▾';
    });
    Array.prototype.forEach.call(root.querySelectorAll('[data-catof]'), function (x) {
      x.classList.toggle('catclosed', !q && !!cc[x.getAttribute('data-catof')]);
    });
  }
  /* BCB6's 16 standard TColor values (lib/format.js CL has the same) */
  var VCL16 = [['clBlack', '#000000'], ['clMaroon', '#800000'], ['clGreen', '#008000'], ['clOlive', '#808000'],
    ['clNavy', '#000080'], ['clPurple', '#800080'], ['clTeal', '#008080'], ['clGray', '#808080'],
    ['clSilver', '#c0c0c0'], ['clRed', '#ff0000'], ['clLime', '#00ff00'], ['clYellow', '#ffff00'],
    ['clBlue', '#0000ff'], ['clFuchsia', '#ff00ff'], ['clAqua', '#00ffff'], ['clWhite', '#ffffff']];

  /* a colour typed in: '#rrggbb' / '#rgb' / a VCL16 name (clRed) -> '#rrggbb', else '' */
  function colorText(s) {
    var t = String(s || '').trim().toLowerCase();
    var m = /^#?([0-9a-f]{6})$/.exec(t);
    if (m) return '#' + m[1];
    m = /^#?([0-9a-f])([0-9a-f])([0-9a-f])$/.exec(t);
    if (m) return '#' + m[1] + m[1] + m[2] + m[2] + m[3] + m[3];
    var nm = VCL16.concat(VCLSYS).filter(function (x) { return x[0].toLowerCase() === t; })[0];
    return nm ? nm[1] : '';
  }
  /* 1005 (WPF/BCB6 audit: a colour typed as a system colour name was refused): BCB6's system colours as the classic
     Windows scheme draws them -- the same values as lib/format.js CL (what the page shows for a .dfm clBtnFace) */
  var VCLSYS = [['clBtnFace', '#d4d0c8'], ['clWindow', '#ffffff'], ['clWindowText', '#000000'], ['clBtnText', '#000000'],
    ['clBtnShadow', '#808080'], ['clBtnHighlight', '#ffffff'], ['clHighlight', '#0a246a'], ['clHighlightText', '#ffffff'],
    ['clGrayText', '#808080'], ['clInfoBk', '#ffffe1'], ['clInfoText', '#000000'], ['clMenu', '#d4d0c8'], ['clMenuText', '#000000'],
    ['clActiveCaption', '#0a246a'], ['clInactiveCaption', '#808080'], ['clCaptionText', '#ffffff'], ['clInactiveCaptionText', '#d4d0c8'],
    ['clWindowFrame', '#000000'], ['clScrollBar', '#d4d0c8'], ['clBackground', '#3a6ea5'], ['clAppWorkSpace', '#808080'],
    ['clActiveBorder', '#d4d0c8'], ['clInactiveBorder', '#d4d0c8'], ['cl3DDkShadow', '#404040'], ['cl3DLight', '#d4d0c8'],
    ['clMoneyGreen', '#c0dcc0'], ['clSkyBlue', '#a6caf0'], ['clCream', '#fffbf0'], ['clMedGray', '#a0a0a4']];

  var KIND = {
    web: { chip: '網頁', title: '網頁 JS：瀏覽器實際執行的程式' },
    port: { chip: 'C++', title: 'C++ 移植樹（HT9011UC_Cpp）' },
    golden: { chip: 'BCB6', title: 'BCB6 原始碼（golden，Big5，唯讀開啟）' },
    dfm: { chip: 'DFM', title: 'BCB6 表單檔 .dfm（golden，Big5，唯讀開啟）' },
    html: { chip: 'HTML', title: '頁面 HTML 原始碼' },
  };

  /* the HTML is folded until opened (WPF keeps the rest behind an expander): the properties list opens on the grid and
     the DFM properties it does not show (they are not repeated any more, 0.112 -- the one list WPF has); opened /
     folded is remembered */
  var DEFAULT_CLOSED = { html: 1 };
  function isClosed(key) {
    if (Object.prototype.hasOwnProperty.call(state.closed, key)) return !!state.closed[key];
    if (!DEFAULT_CLOSED[key]) return false;
    /* (only behind the editable grid: a component without it -- page-JS made, not in the source -- shows its DFM list
       open, or the properties list would look empty) */
    var e0 = data && data.edit;
    return !!(e0 && (e0.look || e0.layout || e0.caption));
  }
  function section(key, title, count, build) {
    if (!inView(key)) return null;
    var d = el('details', 'sec');
    d.open = !isClosed(key);
    var s = el('summary', null, title);
    if (count != null) s.appendChild(el('span', 'count', count));
    d.appendChild(s);
    d.setAttribute('data-key', key);
    // (a search opens the sections it finds something in: that is not the user closing / opening one)
    // ('toggle' comes later than the change, so the search box's text is what tells)
    d.addEventListener('toggle', function () { if (searching || String(state.q || '').trim()) return; state.closed[key] = !d.open; save(); });
    var body = el('div', 'body');
    build(body);
    d.appendChild(body);
    root.appendChild(d);
    return d;
  }

  /* one clickable code location; double-click (or Enter) opens it */
  function targetRow(i, indent) {
    var t = data.targets[i];
    if (!t) return el('div');
    var r = el('div', 'row target' + (indent ? ' ind' : '') + (t.warn ? ' warn' : '') + (t.weak ? ' weak' : ''));
    r.tabIndex = 0;
    r.title = KIND[t.kind].title + '\n' + t.file + ':' + t.line + '\n雙擊開啟';
    r.appendChild(el('span', 'chip k-' + t.kind, KIND[t.kind].chip));
    var loc = el('span', 'loc', base(t.file) + ':' + t.line);
    r.appendChild(loc);
    if (t.note) r.appendChild(el('span', 'note', t.note));
    if (t.snippet) r.appendChild(el('div', 'snip', t.snippet));
    var open = function () { vscode.postMessage({ type: 'open', i: i }); };
    r.addEventListener('dblclick', open);
    r.addEventListener('keydown', function (e) { if (e.key === 'Enter') open(); });
    r.addEventListener('click', function () { mark(r); });
    return r;
  }
  function mark(r) {
    var old = root.querySelectorAll('.row.sel');
    for (var i = 0; i < old.length; i++) old[i].classList.remove('sel');
    r.classList.add('sel');
  }
  /* a small right-click menu: items [label, enabled, fn]; closes on a click elsewhere / Esc */
  function menu(x, y, items) {
    var old = document.querySelector('.ctxmenu');
    if (old) old.parentNode.removeChild(old);
    var m = el('div', 'ctxmenu');
    var close = function () { if (m.parentNode) m.parentNode.removeChild(m); document.removeEventListener('mousedown', away, true); document.removeEventListener('keydown', esc, true); };
    var away = function (e) { if (!m.contains(e.target)) close(); };
    var esc = function (e) { if (e.key === 'Escape') close(); };
    items.forEach(function (it) {
      var b = el('div', 'ctxitem' + (it[1] ? '' : ' off'), it[0]);
      b.setAttribute('data-item', it[0]);
      if (it[1]) b.addEventListener('click', function () { close(); it[2](); });
      m.appendChild(b);
    });
    document.body.appendChild(m);
    pending(close);
    m.style.left = Math.max(0, Math.min(x, window.innerWidth - m.offsetWidth - 4)) + 'px';
    m.style.top = Math.max(0, Math.min(y, window.innerHeight - m.offsetHeight - 4)) + 'px';
    document.addEventListener('mousedown', away, true);
    document.addEventListener('keydown', esc, true);
    return m;
  }

  /* a name / value row whose value becomes an input on a click on the value (as the property grid's: click and type)
     or a double-click on the row; Enter or leaving the field commits (only a changed value), Esc cancels */
  function editRow(tb, name, value, canEdit, onCommit, tip) {
    var tr = el('tr', canEdit ? 'editable' : null);
    tr.setAttribute('data-name', name);
    tr.appendChild(el('td', 'k', name));
    var v = el('td', 'v');
    var shown = el('span', null, value);
    v.appendChild(shown);
    tr.appendChild(v);
    tr.title = tip || '';
    if (canEdit) {
      var start = function () {
        if (v.querySelector('input')) return;
        var i = el('input', 'txt');
        i.value = value;
        i.setAttribute('data-field', 'html:' + name);
        v.textContent = '';
        v.appendChild(i);
        i.focus();
        try { i.select(); } catch (x) { /* ignore */ }
        var done = false;
        var finish = function (commit) {
          if (done) return;
          done = true;
          if (commit && i.value !== value) { onCommit(i.value); return; }
          v.textContent = '';
          v.appendChild(shown);
        };
        i.addEventListener('keydown', function (e) {
          if (e.key === 'Enter') finish(true);
          else if (e.key === 'Escape') finish(false);
        });
        i.addEventListener('blur', function () { finish(true); });
      };
      v.addEventListener('click', start);
      tr.addEventListener('dblclick', start);
    }
    tb.appendChild(tr);
    return tr;
  }

  /* onDbl(p) -> true when it handled the double-click (else the value is copied) */
  function kv(body, rows, withColor, onDbl) {
    var tb = el('table', 'kv');
    rows.forEach(function (p) {
      var tr = el('tr');
      var k = el('td', 'k', p[0]);
      if (withColor) describeKey(k, p[0], onDbl && p[4] ? '雙擊：跳到 .dfm 第 ' + p[4] + ' 行' : '');
      var v = el('td', 'v');
      if (withColor && p[2]) {
        var sw = el('span', 'sw');
        sw.style.background = p[2];
        v.appendChild(sw);
      }
      v.appendChild(document.createTextNode(p[1] == null ? '' : String(p[1])));
      if (p[3]) v.appendChild(el('span', 'hint', p[3]));
      if (onDbl && p[4]) tr.title = '雙擊：跳到 .dfm 第 ' + p[4] + ' 行';
      tr.appendChild(k); tr.appendChild(v);
      tr.addEventListener('dblclick', function () {
        if (onDbl && onDbl(p)) return;
        vscode.postMessage({ type: 'copy', text: String(p[1]) });
      });
      tb.appendChild(tr);
    });
    body.appendChild(tb);
    return tb;
  }

  function render() {
    root.classList.remove('sample');
    sampleMode = false;
    if (!data && sample) {
      // (the button's panel drawn as usual, then made a grey example: the controls off, the view tabs still switch)
      sampleMode = true;
      data = sample;
      // (a saved search word would filter the example while its box is off: the example shows everything)
      var q0 = state.q;
      state.q = '';
      try { render0(); } finally { data = null; state.q = q0; }
      root.classList.add('sample');
      Array.prototype.forEach.call(root.querySelectorAll('input, select, textarea, button'), function (x) {
        if (!(x.closest && x.closest('.views'))) { x.disabled = true; x.tabIndex = -1; }
      });
      // (0.150, EastSun: "不需要圖片上這多餘的東西 不用給我說明 直接反灰就好 名稱給我空白": no note on top, the name empty)
      var nb = root.querySelector('.namebox');
      if (nb) nb.value = '';
      return;
    }
    render0();
  }

  function render0() {
    root.textContent = '';
    if (!data) {
      var e = el('div', 'empty');
      e.appendChild(el('p', null, '在設計檢視裡點一個元件，這裡會顯示它的屬性和事件。'));
      e.appendChild(el('p', null, '雙擊事件，會跳到那個事件的程式碼。在畫面上直接雙擊元件，會跳到它的預設事件（通常是 OnClick）。'));
      root.appendChild(e);
      return;
    }
    var c = data.comp;

    /* header */
    var h = el('div', 'head');
    /* 1005 (WPF audit: the instance list on top of BCB6's Object Inspector / WPF's Properties window): every component
       of the page, "name : type", indented by depth -- picking one selects it (no going back to the page to click) */
    if (data.comps && data.comps.length > 1) {
      var cur = c.isForm ? '@form' : (c.htmlId || '');
      var is = el('select', 'instsel');
      is.title = '這一頁的元件（依頁面順序，縮排＝在誰裡面）：選一個＝在設計畫面選取它';
      data.comps.forEach(function (x) {
        var o = document.createElement('option');
        o.value = x.id;
        o.textContent = new Array(x.depth + 1).join('  ') + x.name + (x.cls ? ' : ' + x.cls : '');
        if (x.id === cur) o.selected = true;
        is.appendChild(o);
      });
      if (!data.comps.some(function (x) { return x.id === cur; })) { var o0 = document.createElement('option'); o0.value = ''; o0.textContent = c.name; o0.selected = true; is.insertBefore(o0, is.firstChild); }
      is.addEventListener('change', function () { if (is.value && is.value !== cur) vscode.postMessage({ type: 'selectId', id: is.value }); });
      h.appendChild(is);
    }
    var t1 = el('div', 'title');
    /* WPF: "Change the name of the currently selected element in the Name box" (the type beside it) */
    if (!c.isForm && c.htmlId && data.edit && data.edit.inSource && !(data.edit && data.edit.multi)) {
      t1.appendChild(el('span', 'nlabel', '名稱'));
      var nb = textBox('name', c.htmlId, function (v) { if (v && v !== c.htmlId) vscode.postMessage({ type: 'rename', name: v }); });
      nb.classList.add('namebox');
      nb.title = '改名稱＝id、title、<label for>、網頁事件跟著改（BCB6 .dfm 或網頁程式用到時會先問）；元件樹上按 F2 也可以（畫面上的 F2＝直接改文字，跟 WPF 一樣）';
      t1.appendChild(nb);
    } else t1.appendChild(el('span', 'name', c.name));
    if (c.cls) t1.appendChild(el('span', 'cls', c.cls));
    h.appendChild(t1);
    /* the Properties / Events buttons (the lightning bolt): one list at a time */
    var vw = el('div', 'views');
    [['props', '屬性', '屬性（外觀、版面、DFM、HTML…）'], ['events', '⚡ 事件', '事件（WPF 的閃電按鈕）：每個事件和它的函式'],
      ['code', '</> 程式碼', '程式碼與接線：網頁 JS 監聽器、送到 C++ 的命令、資料欄位、顯示的資料標籤、用到這個元件的程式']].forEach(function (v) {
      var b = el('button', state.view === v[0] ? 'on' : null, v[1]);
      b.setAttribute('data-view', v[0]);
      b.title = v[2];
      b.addEventListener('click', function () {
        if (state.view === v[0]) return;
        state.view = v[0]; save(); render();
        /* AI(W906-HTDESIGNER) 20261001 (〔EH〕"click the Events button. The events for the control are listed and the default
           event is selected"): its value field has the focus -- Enter there makes it */
        if (v[0] === 'events') {
          var evs = Array.prototype.map.call(root.querySelectorAll('.row.evg[data-event]'), function (x) { return x.getAttribute('data-event'); });
          var dn = ['OnClick', 'OnChange', 'OnTimer', 'OnCreate'].filter(function (n) { return evs.indexOf(n) >= 0; })[0] || evs[0];
          var di = dn ? root.querySelector('.row.evg[data-event="' + dn + '"] input.evin') : null;
          if (di) { di.focus(); try { di.select(); } catch (x) { /* ignore */ } }
        }
      });
      vw.appendChild(b);
    });
    h.appendChild(vw);
    if (c.note) h.appendChild(el('div', 'sub', c.note));
    /* WPF's tag navigator ("lets you move to any parent tag of the currently selected tag"): the parents on the page,
       each a click away; the DFM's own path in its tooltip */
    if (c.chain && c.chain.length && !c.isForm) {
      var cr = el('div', 'path crumbs');
      c.chain.forEach(function (pid) {
        var a = el('span', 'crumb', pid === '@form' ? '表單' : pid);
        a.setAttribute('data-id', pid);
        a.title = '選取上層：' + pid;
        a.addEventListener('click', function () { vscode.postMessage({ type: 'selectId', id: pid }); });
        cr.appendChild(a);
        cr.appendChild(el('span', 'sep', ' › '));
      });
      cr.appendChild(el('span', 'here', c.name));
      if (c.path) cr.title = 'DFM：' + c.path;
      h.appendChild(cr);
    } else {
      var crumbs = c.path ? c.path : (c.chain.length ? c.chain.concat([c.name]).join('.') : '');
      if (crumbs) h.appendChild(el('div', 'path', crumbs));
    }
    if (!c.inIr && !c.isForm) h.appendChild(el('div', 'sub dim', c.htmlId ? '（DFM 裡沒有這個名稱：可能是網頁自己加的元件）' : '（沒有 id 的 HTML 元素：' + (c.cssPath || '<' + c.tag + '>') + '）'));
    /* (the header as short as WPF's: the jump buttons at the end of the tabs' line, small) */
    var bar = el('span', 'vtools');
    if (c.dfm != null) {
      var b0 = el('button', null, 'DFM');
      b0.title = 'DFM 定義：跳到 BCB6 .dfm 裡定義這個元件的那一行（唯讀）';
      b0.addEventListener('click', function () { vscode.postMessage({ type: 'open', i: c.dfm }); });
      bar.appendChild(b0);
    }
    if (c.html != null) {
      var b1 = el('button', null, 'HTML');
      b1.title = 'HTML 原始碼：跳到這個元件在 HTML 裡的位置';
      b1.addEventListener('click', function () { vscode.postMessage({ type: 'revealSource' }); });
      bar.appendChild(b1);
    }
    if (c.htmlId) {
      var b2 = el('button', null, '複製');
      b2.title = '複製名稱：' + (c.isForm ? c.name : c.htmlId);
      b2.addEventListener('click', function () { vscode.postMessage({ type: 'copy', text: c.isForm ? c.name : c.htmlId }); });
      bar.appendChild(b2);
    }
    vw.appendChild(bar);
    /* WPF: one search box over the whole property grid */
    var sb = el('div', 'search');
    var sq = el('input', 'filter');
    sq.type = 'search';
    sq.placeholder = state.view === 'events' ? '搜尋事件、函式…' : state.view === 'code' ? '搜尋檔名、命令、程式…' : '搜尋屬性、值…';
    sq.value = state.q || '';
    sq.setAttribute('data-field', 'search');
    var sn = el('span', 'dim small shits');
    sq.addEventListener('input', function () { state.q = sq.value; save(); sn.textContent = applySearch(); });
    sq.addEventListener('keydown', function (e) { if (e.key === 'Escape') { sq.value = ''; state.q = ''; save(); sn.textContent = applySearch(); } });
    sb.appendChild(sq);
    sb.appendChild(sn);
    h.appendChild(sb);
    /* WPF: "To arrange the properties by category or alphabetically, click Category, Name, or Source in the Arrange by
       list" -- beside the search box, as in WPF (the DFM properties follow it) */
    if (state.view === 'props' && ((data.props && data.props.length) || (data.edit && (data.edit.look || data.edit.layout)))) {
      var srt = arrangeBy();
      var sbar = el('span', 'sortbar');
      [['cat', '類別', '排列：依類別（WPF 的預設）'], ['name', '名稱', '排列：依名稱 A→Z'], ['dfm', 'DFM', '排列：BCB6 .dfm 寫的順序']].forEach(function (o) {
        var b = el('button', srt === o[0] ? 'on' : null, o[1]);
        b.setAttribute('data-sort', o[0]);
        b.title = o[2];
        b.addEventListener('click', function () { state.propSort = o[0]; save(); render(); });
        sbar.appendChild(b);
      });
      sb.insertBefore(sbar, sn);
    }
    root.appendChild(h);

    /* editable properties -- the WPF property grid: commit on Enter or leaving the field */
    var ed = data.edit;
    /* WPF has ONE property list: what the grid shows is left out of the DFM list below it (the grid's rows name the
       .dfm names they stand for) -- the DFM list is then the rest, read only */
    var gridLabels = {};
    var dfmNamesOf = function (label) {
      if (label === 'Font.Size') return ['Font.Height', 'Font.Size'];
      if (label === 'Font.Bold' || label === 'Font.Italic') return ['Font.Style'];
      return [label];
    };
    var dfmPropOf = function (label) {
      var names = dfmNamesOf(label);
      return (data.props || []).filter(function (p) { return names.indexOf(p[0]) >= 0; })[0] || null;
    };
    var inGrid = function (p) {
      var n = p[0];
      /* Font.Style: the grid has Bold and Italic only -- an underline / strike-out stays in the DFM list */
      // (0.152: with Underline / StrikeOut rows too -- an underline / strike-out stays in the DFM list only where those rows are not)
      if (n === 'Font.Style') return !!(gridLabels['Font.Bold'] && gridLabels['Font.Italic']) && (!/fsUnderline/i.test(String(p[1])) || !!gridLabels['Font.Underline']) && (!/fsStrikeOut/i.test(String(p[1])) || !!gridLabels['Font.StrikeOut']);
      if (n === 'Font.Height') return !!gridLabels['Font.Size'];
      // (0.157: the .dfm's Items.Strings / Lines.Strings -- the editable Items / Lines row stands for them)
      if (n === 'Items.Strings' || n === 'Items') return !!gridLabels['Items'];
      if (n === 'Lines.Strings' || n === 'Lines') return !!gridLabels['Lines'];
      if (/^Picture(\.|$)/.test(n)) return !!gridLabels['Picture'];
      return !!gridLabels[n];
    };
    if (ed && ed.multi) {
      /* WPF Format > Align: the primary (this one) is the reference */
      section('align', '多選：對齊與大小', ed.multi.length + 1, function (body) {
        body.appendChild(el('div', 'pad small', '主要：' + c.name + '　其他：' + ed.multi.join('、')));
        var bar2 = el('div', 'bar pad');
        [['left', '靠左'], ['hcenter', '水平置中'], ['right', '靠右'], ['top', '靠上'], ['vcenter', '垂直置中'], ['bottom', '靠下'],
          ['width', '同寬'], ['height', '同高'], ['size', '同大小']].forEach(function (a) {
          var b = el('button', null, a[1]);
          b.setAttribute('data-align', a[0]);
          b.title = '以「' + c.name + '」為基準';
          b.addEventListener('click', function () { vscode.postMessage({ type: 'align', how: a[0] }); });
          bar2.appendChild(b);
        });
        body.appendChild(bar2);
        /* spacing / centring: every selected one may move, none is the reference */
        var bar3 = el('div', 'bar pad');
        [['hspace', '水平等距', 3], ['vspace', '垂直等距', 3], ['hcenterIn', '容器中水平置中', 1], ['vcenterIn', '容器中垂直置中', 1]].forEach(function (a) {
          var b = el('button', null, a[1]);
          b.setAttribute('data-align', a[0]);
          if (ed.multi.length + 1 < a[2]) { b.disabled = true; b.title = '要選 ' + a[2] + ' 個以上'; }
          else b.title = a[2] > 1 ? '最外面兩個不動，中間的平均分配' : '整組一起置中';
          b.addEventListener('click', function () { vscode.postMessage({ type: 'align', how: a[0] }); });
          bar3.appendChild(b);
        });
        body.appendChild(bar3);
        /* every selected one back to its own .dfm values (one undo step) */
        var bar4 = el('div', 'bar pad');
        var rm = el('button', null, '全部改回 DFM（' + (ed.multi.length + 1) + ' 個）');
        rm.setAttribute('data-act', 'resetToDfm');
        rm.title = '每個選取的元件的位置、大小、文字、外觀都改回它自己在 .dfm 的值';
        rm.addEventListener('click', function () { vscode.postMessage({ type: 'resetToDfm' }); });
        bar4.appendChild(rm);
        body.appendChild(bar4);
        var allNote = el('div', 'pad small multinote', '下面「外觀與版面」改的值會套用到全部 ' + (ed.multi.length + 1) + ' 個選取的元件（顯示的是「' + c.name + '」的值）；一次＝一個復原步驟。HTML 原始碼那一格只改「' + c.name + '」。');
        body.appendChild(allNote);
        body.appendChild(el('div', 'dim pad small', '整組一起拖曳、一起用方向鍵移動；一次對齊＝一個復原步驟。Ctrl／Shift＋點可以加入或移出選取。'));
      });
    }
    if (ed && (ed.layout || ed.caption || ed.look)) {
      var dfm = ed.dfm || {};
      var diffs = 0;
      section('edit', ed.multi ? '外觀與版面（可編輯，套用到全部 ' + (ed.multi.length + 1) + ' 個）' : '屬性', null, function (body) {
        var tb = el('table', 'kv edit');
        /* AI(W906-HTDESIGNER) 20261001: BCB6's Object Inspector -- "toggle their settings by double-clicking in the Value
           column": a double click on a True / False value switches it ONCE (a checkbox took the two clicks as two
           switches: back where it was, two undo steps); the second click is dropped, and a double click on the value's
           empty part switches it too */
        tb.addEventListener('click', function (e) { var t = e.target; if (t && t.type === 'checkbox' && e.detail >= 2) e.preventDefault(); }, true);
        tb.addEventListener('dblclick', function (e) {
          var t = e.target;
          if (!t || /^(INPUT|SELECT|BUTTON|OPTION)$/.test(t.tagName)) return;
          var td = t.closest ? t.closest('td.v') : null;
          var cb = td && td.querySelector('input[type=checkbox]');
          if (cb && !cb.disabled) { cb.click(); return; }
          /* 1006 (Object Inspector: "double-clicking in the Value column" cycles an enumerated value too): a list value's
             empty part double-clicked = the next value (round at the end), ONE change */
          var sl = td && td.querySelector('select');
          if (sl && !sl.disabled && sl.options.length > 1) {
            sl.selectedIndex = (sl.selectedIndex + 1) % sl.options.length;
            sl.dispatchEvent(new Event('change', { bubbles: true }));
          }
        });
        /* AI(W906-HTDESIGNER) 20261001 (real input): Up / Down on a focused drop-down CHANGED its value and wrote it
           (Alignment taLeftJustify -> taCenter) -- the Object Inspector's "Up/Down Arrow Keys ... select properties":
           on a drop-down, a check box or a text value they go to the row above / below; Alt+Down opens the list.
           (A number field keeps Blend's +1 / -1; a field with a suggestion list keeps its list.) */
        tb.addEventListener('keydown', function (e) {
          var t = e.target;
          if ((e.key !== 'ArrowUp' && e.key !== 'ArrowDown') || e.altKey || e.ctrlKey || e.metaKey || e.shiftKey || !t) return;
          var ok = t.tagName === 'SELECT' || t.type === 'checkbox' || (t.tagName === 'INPUT' && t.type === 'text' && !t.getAttribute('list'));
          if (!ok) return;
          e.preventDefault();
          var all = Array.prototype.filter.call(tb.querySelectorAll('input, select'), function (x) {
            return x.tabIndex >= 0 && !x.disabled && x.type !== 'color' && x.getClientRects().length;
          });
          var n = all[all.indexOf(t) + (e.key === 'ArrowUp' ? -1 : 1)];
          if (n) { n.focus(); if (n.tagName === 'INPUT' && n.type !== 'checkbox') { try { n.select(); } catch (x) { /* ignore */ } } }
        });
        /* (the column widths: with table-layout fixed the first row decides -- a category band spans all three) */
        var cg = el('colgroup');
        ['k', 'v', 'm'].forEach(function (c0) { cg.appendChild(el('col', 'c' + c0)); });
        tb.appendChild(cg);
        /* dv = the DFM (BCB6) value, cur = the page's; different -> marked, ↺ resets */
        function same(a, b) {
          if (typeof a === 'number' || typeof b === 'number') return Math.round(+a) === Math.round(+b);
          if (typeof a === 'boolean' || typeof b === 'boolean') return !!a === !!b;
          return String(a == null ? '' : a).trim().toLowerCase() === String(b == null ? '' : b).trim().toLowerCase();
        }
        /* info = { text, title }: a DFM value shown but not compared */
        /* dvText: how to show the DFM value (default: dv itself) */
        /* WPF's grid (EastSun: "看起來條理分明"): three columns -- the name | the value | the property marker; nothing
           under the value: a short unit (px) stays, a longer note is the ⓘ's tooltip, the DFM value shows only when it
           differs (the marker's tooltip / menu always says it) */
        function row(label, input, hint, dv, cur, onReset, info, dvText) {
          gridLabels[label] = 1;
          var tr = el('tr');
          var kd = el('td', 'k', label);
          describeKey(kd, label);
          /* a double-click on the name: to where it is written in the HTML (the value selected) -- the code a click away */
          kd.title = (kd.title || label) + '\n\n雙擊：跳到 HTML 原始碼裡的這個值';
          kd.addEventListener('dblclick', function (e) { e.stopPropagation(); vscode.postMessage({ type: 'revealProp', prop: label }); });
          tr.appendChild(kd);
          var td = el('td', 'v');
          td.appendChild(input);
          var tm = el('td', 'm');
          var tips = [];
          if (hint) {
            if (String(hint).length <= 4) td.appendChild(el('span', 'hint', hint));
            else tips.push(String(hint).replace(/^（/, '').replace(/）$/, ''));
          }
          /* WPF's property marker: a box right of EVERY value -- hollow = the default (here: the .dfm's value, or the .dfm
             does not give it), filled = set to something else; a click (or a right-click on the row) opens its menu:
             Reset, and to where the value is written in the HTML */
          var hasDv = dv !== null && dv !== undefined && !!onReset;
          var differs = hasDv && !same(dv, cur);
          var shown = hasDv ? dvText || (typeof dv === 'boolean' ? (dv ? 'True' : 'False') : String(dv)) : '';
          var rb = el('button', 'reset marker' + (differs ? ' set' : '') + (hasDv ? '' : ' none'), '');
          rb.tabIndex = -1;   /* (Tab goes value to value, as in WPF's grid; the marker is a click) */
          if ((dv === null || dv === undefined) && info) {
            var sy = el('span', 'dfmv', info.text);
            sy.title = info.title;
            td.appendChild(sy);
            tips.push(info.text + '：' + info.title);
          } else if (hasDv) {
            var d = el('span', 'dfmv', 'DFM ' + shown);
            if (differs) {
              diffs++;
              tr.className = 'diff';
              d.title = '和 BCB6 .dfm 的值不同';
            } else d.title = '和 BCB6 .dfm 的值相同';
            td.appendChild(d);
          }
          rb.title = (differs ? '和 BCB6 .dfm 的值不同（DFM：' + shown + '）' : hasDv ? '和 BCB6 .dfm 的值相同（DFM：' + shown + '）' :
            info ? info.text + '，不比對' : 'BCB6 .dfm 沒有寫這一項（用預設值），不比對') + '\n點一下：重設、跳到 HTML 原始碼、跳到 .dfm';
          rb.setAttribute('aria-label', rb.title);
          var openMenu = function (x, y) {
            /* (several selected with different values: nothing to reset to -- the row is marked mixed) */
            /* several selected: each one back to its OWN .dfm value (the extension asks each), their values may differ */
            var many = !!(ed && ed.multi && ed.multi.length);
            var canReset = !input.disabled && (many ? !!onReset : differs && !tr.classList.contains('mixed'));
            /* (and where BCB6 wrote it: the .dfm line -- the grid's rows are not in the DFM list below any more) */
            var dp = dfmPropOf(label);
            menu(x, y, [
              [many ? '重設（選取的每一個改回它自己的 DFM 值）' : hasDv ? '重設（改回 DFM：' + shown + '）' : '重設（DFM 沒有寫這一項）', canReset,
                function () { if (many) vscode.postMessage({ type: 'resetProp', prop: label }); else onReset(dv); }],
              ['跳到 HTML 原始碼（雙擊名稱也可以）', true, function () { vscode.postMessage({ type: 'revealProp', prop: label }); }],
              [dp && dp[4] ? '跳到 .dfm（第 ' + dp[4] + ' 行 ' + dp[0] + '）' : '跳到 .dfm（DFM 沒有寫這一項）', !!(dp && dp[4]), function () { vscode.postMessage({ type: 'openDfmLine', line: dp[4] }); }],
            ]);
          };
          rb.addEventListener('click', function (e) {
            e.stopPropagation();
            var r = rb.getBoundingClientRect();
            openMenu(r.left, r.bottom + 2);
          });
          tr.addEventListener('contextmenu', function (e) {
            if (e.target && e.target.tagName === 'INPUT' && /^(text|number)$/.test(e.target.type)) return;   /* (a text field: its own cut / copy / paste) */
            e.preventDefault();
            openMenu(e.clientX, e.clientY);
          });
          tm.appendChild(rb);
          if (tips.length) {
            var ib = el('span', 'tip', 'ⓘ');
            ib.title = tips.join('\n');
            td.appendChild(ib);
          }
          tr.appendChild(td);
          tr.appendChild(tm);
          tb.appendChild(tr);
        }
        var setLay = function (k) { return function (v) { var msg = { type: 'setLayout' }; msg[k] = v; vscode.postMessage(msg); }; };
        var setLook = function (k) { return function (v) { vscode.postMessage({ type: 'setLook', prop: k, value: v }); }; };
        function num(field, value, disabled, onCommit) {
          var i = el('input', 'num');
          i.type = 'number'; i.step = '1'; i.value = String(value);
          i.setAttribute('data-field', field);
          i.disabled = !!disabled;
          /* Up / Down = one more / less (Shift = 10), as Blend's number fields: the value changes at once, the edit is
             written when the keys stop (or on leaving the field) -- one undo step for a run of presses */
          var runT = null, sentV = String(value);
          var flush = function () { clearTimeout(runT); runT = null; if (i.value !== '' && isFinite(+i.value) && i.value !== sentV) { sentV = i.value; onCommit(Math.round(+i.value)); } };
          i.addEventListener('change', function () { if (runT) return; if (i.value !== '' && isFinite(+i.value) && i.value !== sentV) { sentV = i.value; onCommit(Math.round(+i.value)); } });
          i.addEventListener('keydown', function (e) {
            /* Enter = written, and you stay in the field with the value selected (WPF): type the next one, or Tab on */
            if (e.key === 'Enter') { e.preventDefault(); flush(); i.select(); return; }
            /* Esc = never mind (WPF): back to the value it had, nothing written */
            if (e.key === 'Escape') { clearTimeout(runT); runT = null; i.value = sentV; i.blur(); return; }
            if ((e.key === 'ArrowUp' || e.key === 'ArrowDown') && !i.disabled) {
              e.preventDefault();
              var st = (e.shiftKey ? 10 : 1) * (e.key === 'ArrowUp' ? 1 : -1);
              i.value = String(Math.round((+i.value || 0) + st));
              clearTimeout(runT);
              runT = setTimeout(flush, 400);
            }
          });
          i.addEventListener('blur', function () { if (runT) flush(); });
          pending(function () { if (runT) flush(); });
          /* AI(W906-HTDESIGNER) 20261001 (real input): the wheel over a focused number field changed it (Left 54 -> 52,
             written on leaving) -- scrolling the panel moved the component. The wheel scrolls the panel now. */
          i.addEventListener('wheel', function () { if (document.activeElement === i) i.blur(); }, { passive: true });
          /* 1005 (WPF audit: Blend's number fields -- press and drag to change the value): drag on the row's NAME left /
             right, 1 per 2 px (Shift = 10 per step), the value changes as you go, written ONCE on letting go (one undo);
             Esc while dragging = back. Under 3 px it is a click (the name's double-click to the source still works). */
          i._scrubStart = function (e) {
            if (i.disabled || e.button !== 0) return;
            var x0 = e.clientX, v0 = +i.value || 0, moved = false;
            var mv = function (ev) {
              var dx = ev.clientX - x0;
              if (!moved && Math.abs(dx) < 3) return;
              if (!moved) { moved = true; document.body.classList.add('scrubbing'); }
              ev.preventDefault();
              i.value = String(Math.round(v0 + Math.round(dx / 2) * (ev.shiftKey ? 10 : 1)));
            };
            var end = function (ev, cancel) {
              document.removeEventListener('pointermove', mv, true);
              document.removeEventListener('pointerup', up, true);
              document.removeEventListener('keydown', esc, true);
              document.body.classList.remove('scrubbing');
              if (!moved) return;
              if (cancel) { i.value = String(v0); return; }
              flush();
            };
            var up = function (ev) { end(ev, false); };
            var esc = function (ev) { if (ev.key === 'Escape') { ev.preventDefault(); ev.stopPropagation(); end(ev, true); } };
            document.addEventListener('pointermove', mv, true);
            document.addEventListener('pointerup', up, true);
            document.addEventListener('keydown', esc, true);
            var unp = pending(function () { end(null, false); });
            var end0 = end;
            end = function (ev, cancel) { unp(); end0(ev, cancel); };
          };
          return i;
        }
        /* Alias (the IO an IO page's component stands for, in its title): typed in, written into the HTML (EastSun:
           "名稱都要讓我改 alias"); the Name is the box at the top (WPF's) */
        if (!ed.multi && !c.isForm && c.htmlId && ed.inSource) {
          var txtIn = textBox;
          if (ed.aliasOn) {
            var dl = null;
            if (ed.ioAliases && ed.ioAliases.names && ed.ioAliases.names.length) {
              dl = el('datalist');
              dl.id = 'ioAliasList';
              ed.ioAliases.names.forEach(function (n) { var o = el('option'); o.value = n; dl.appendChild(o); });
              body.appendChild(dl);
            }
            row('Alias', txtIn('alias', ed.alias, function (v) { if (v !== (ed.alias || '')) vscode.postMessage({ type: 'setAlias', value: v }); }, dl ? 'ioAliasList' : null),
              dl ? '（IO 表 ' + ed.ioAliases.names.length + ' 個名稱可選；寫在 title 裡，網頁程式照它對 IO）' : '（寫在 title 裡，網頁程式照它對 IO）');
          }
        }
        var lay = ed.layout;
        var layOk = lay && ed.layoutInSource;
        if (lay) {
          /* a size the source does not give (an AutoSize label) is the text's, in another font: not compared
             (the same rule as lib/dfmdiff.js sizeInSource) */
          var raw = lay.raw;
          var PX = /^\s*-?\d+(\.\d+)?px\s*$/;     /* "width:136px; … width:auto" = auto */
          var sizeSet = function (k) {
            if (!raw) return true;
            if (k === 'width') return PX.test(raw.width || '') || (PX.test(raw.left || '') && PX.test(raw.right || ''));
            if (k === 'height') return PX.test(raw.height || '') || (PX.test(raw.top || '') && PX.test(raw.bottom || ''));
            return true;
          };
          /* a wrapper the generator adds (span.lled) moves the origin: compare in the DFM's coordinates */
          var shift = { left: lay.ox || 0, top: lay.oy || 0, width: 0, height: 0 };
          (lay.root ? ['width', 'height'] : ['left', 'top', 'width', 'height']).forEach(function (k) {
            var label = k.charAt(0).toUpperCase() + k.slice(1);
            var auto = dfm[k] != null && !sizeSet(k);
            var hint = k === 'left' && lay.target === 'parent' ? '（外層 span 定位）' : '';
            if (shift[k]) hint = '（外層包裝位移 ' + shift[k] + '）';
            row(label, num(k, lay[k], !layOk, setLay(k)), hint,
              auto || lay[k] == null ? null : dfm[k], lay[k] == null ? null : lay[k] + shift[k], function (v) { setLay(k)(v - shift[k]); },
              auto ? { text: 'DFM ' + dfm[k] + '（大小由文字決定）', title: '原始碼沒有寫這個大小（例如 AutoSize 的 Label），大小由文字和字型決定，不比對' } : null,
              shift[k] ? dfm[k] + '（在 DFM 座標；這裡是 ' + (dfm[k] - shift[k]) + '）' : '');
          });
        }
        var cap = ed.caption;
        if (cap) {
          var ti = el('input', 'txt');
          ti.type = 'text'; ti.value = cap.value; ti.setAttribute('data-field', 'caption');
          ti.disabled = !ed.inSource;
          /* (written once: on Enter -- staying in the field, WPF -- or on leaving it) */
          var capSent = cap.value;
          var capCommit = function () { if (ti.value === capSent) return; capSent = ti.value; vscode.postMessage({ type: 'setCaption', value: ti.value }); };
          ti.addEventListener('change', capCommit);
          /* (Esc: back to what it was when the editing started, WPF) */
          ti.addEventListener('focus', function () { ti._v0 = ti.value; });
          ti.addEventListener('keydown', function (e) {
            if (e.key === 'Enter') { e.preventDefault(); capCommit(); ti.select(); }
            else if (e.key === 'Escape') { ti.value = ti._v0 != null ? ti._v0 : cap.value; ti.blur(); }
          });
          row(cap.kind === 'value' ? 'Text' : 'Caption', ti, '', cap.kind === 'value' ? dfm.text : dfm.caption, cap.value,
            function (v) { vscode.postMessage({ type: 'setCaption', value: v }); });
        }
        var lk = ed.look;
        if (lk) {
          var vis = el('input');
          vis.type = 'checkbox'; vis.checked = !!lk.visible; vis.setAttribute('data-field', 'visible'); vis.disabled = !ed.inSource;
          vis.addEventListener('change', function () { vscode.postMessage({ type: 'setLook', prop: 'visible', value: vis.checked }); });
          /* like the BCB6 designer: it shows while designing either way; this is what happens when it runs */
          row('Visible', vis, !lk.visible ? '（執行時隱藏；設計時照樣顯示）' :
            lk.runHidden === 'rule' ? '（執行時由頁面決定，例如機種專用、選配；設計時照樣顯示）' : '', dfm.visible, lk.visible, setLook('visible'));
          if (lk.enabled !== null && lk.enabled !== undefined) {
            var en = el('input');
            en.type = 'checkbox'; en.checked = !!lk.enabled; en.setAttribute('data-field', 'enabled'); en.disabled = !ed.inSource;
            en.addEventListener('change', function () { vscode.postMessage({ type: 'setLook', prop: 'enabled', value: en.checked }); });
            row('Enabled', en, '', dfm.enabled, lk.enabled, setLook('enabled'));
          }
          if (lk.autoSize !== null && lk.autoSize !== undefined) {
            /* BCB6 AutoSize (a TLabel): on = its size follows the text; off = it keeps the size it has */
            var asz = el('input');
            asz.type = 'checkbox'; asz.checked = !!lk.autoSize; asz.setAttribute('data-field', 'autoSize'); asz.disabled = !ed.inSource || !!ed.locked;
            asz.addEventListener('change', function () { vscode.postMessage({ type: 'setLook', prop: 'autoSize', value: asz.checked }); });
            row('AutoSize', asz, lk.autoSize ? '（大小跟著文字；要拉大小先關掉）' : '', dfm.autoSize, lk.autoSize, function (v) {
              /* back to the DFM's False: with the .dfm's own size, so the label matches in one step */
              var msg = { type: 'setLook', prop: 'autoSize', value: v };
              if (v === false && typeof dfm.width === 'number' && typeof dfm.height === 'number') msg.size = { width: dfm.width, height: dfm.height };
              vscode.postMessage(msg);
            });
          }
          if (lk.alignment) {
            /* BCB6 Alignment of a label: shows with a fixed width (AutoSize off) */
            var alg = el('select', 'txt');
            [['taLeftJustify', '靠左'], ['taCenter', '置中'], ['taRightJustify', '靠右']].forEach(function (o) {
              var op = el('option', null, o[0] + '（' + o[1] + '）');
              op.value = o[0];
              if (lk.alignment === o[0]) op.selected = true;
              alg.appendChild(op);
            });
            alg.setAttribute('data-field', 'alignment'); alg.disabled = !ed.inSource;
            alg.addEventListener('change', function () { vscode.postMessage({ type: 'setLook', prop: 'alignment', value: alg.value }); });
            row('Alignment', alg, lk.autoSize && lk.alignment !== 'taLeftJustify' ? '（AutoSize 開著時看不出來）' : '', dfm.alignment, lk.alignment, setLook('alignment'));
          }
          var fname = el('input', 'txt');
          fname.type = 'text'; fname.value = lk.fontName || ''; fname.setAttribute('data-field', 'fontName'); fname.disabled = !ed.inSource;
          var fnSent = lk.fontName || '';
          var fnCommit = function () { if (fname.value === fnSent) return; fnSent = fname.value; vscode.postMessage({ type: 'setLook', prop: 'fontName', value: fname.value }); };
          fname.addEventListener('change', fnCommit);
          fname.addEventListener('focus', function () { fname._v0 = fname.value; });
          fname.addEventListener('keydown', function (e) {
            if (e.key === 'Enter') { e.preventDefault(); fnCommit(); fname.select(); }
            else if (e.key === 'Escape') { fname.value = fname._v0 != null ? fname._v0 : (lk.fontName || ''); fname.blur(); }
          });
          /* the page's font list names the DFM font (e.g. "Microsoft JhengHei","MS Sans Serif") -> same */
          var fams = lk.fontFamilies || [];
          /* WPF's FontFamily is a drop-down: the DFM's font, the ones this control's list names, the usual ones */
          var fdl = el('datalist');
          fdl.id = 'fontNameList';
          var fseen = {};
          [dfm.fontName].concat(fams, ['MS Sans Serif', 'Microsoft JhengHei', '新細明體', 'PMingLiU', '標楷體', 'Arial', 'Tahoma', 'Segoe UI',
            'Verdana', 'Times New Roman', 'Courier New', 'Consolas']).forEach(function (f) {
            var k = String(f || '').trim();
            if (!k || fseen[k.toLowerCase()]) return;
            fseen[k.toLowerCase()] = 1;
            var o = el('option'); o.value = k; fdl.appendChild(o);
          });
          body.appendChild(fdl);
          fname.setAttribute('list', 'fontNameList');
          /* a name picked from the list: set at once */
          fname.addEventListener('input', function (e) { if (e.inputType === 'insertReplacementText' && fseen[fname.value.trim().toLowerCase()]) fnCommit(); });
          var famHit = dfm.fontName && fams.some(function (f) { return f.toLowerCase() === String(dfm.fontName).toLowerCase(); });
          var inh = lk.fontOwn === false && dfm.fontName != null;   /* the form's UI font, not given for this control */
          row('Font.Name', fname, fams.length > 1 ? '（清單：' + fams.join(', ') + '）' : '', inh ? null : dfm.fontName, famHit ? dfm.fontName : lk.fontName, setLook('fontName'),
            inh ? { text: 'DFM ' + dfm.fontName + '（沿用表單字型，不比對）', title: '網頁沒有給這個元件字型，用的是表單的 UI 字型（網頁產生器對 Label／按鈕的做法），不比對' } : null);
          /* 1006 (WinForms / the Object Inspector: Font's "…" = one dialog for name, size and style): the changed ones sent
             together -- ONE source edit, one Ctrl+Z */
          if (ed.inSource && fname.parentNode) {
            var fdb = el('button', 'fontdlg', '…');
            fdb.type = 'button'; fdb.tabIndex = -1; fdb.title = '字型…（一次設定字型、大小、粗體、斜體、底線、刪除線；一個 Ctrl+Z 復原）';
            fdb.addEventListener('click', function (ev) { ev.preventDefault(); openFontDialog(lk, fams); });
            fname.parentNode.appendChild(fdb);
          }
          /* DFM Font.Height -13 -> the 11px a generated page has (|Height| - 2) */
          /* (BCB6's Font.Size is in points: Size = -Height * 72 / 96 -- said beside the px, so a BCB6 number is not typed as px) */
          var ptOf = function (px) { return typeof px === 'number' && px > 0 ? Math.round((px + 2) * 72 / 96) : null; };
          var ptNow = ptOf(lk.fontSize);
          var fsIn = num('fontSize', lk.fontSize, !ed.inSource, setLook('fontSize'));
          if (ptNow) fsIn.title = '這裡是 px；BCB6 的 Font.Size 是點數：' + ptNow + '（Size = -Height × 72 / 96）';
          row('Font.Size', fsIn, 'px', dfm.fontSize, lk.fontSize, setLook('fontSize'), null,
            dfm.fontHeight != null ? dfm.fontSize + '（Height ' + dfm.fontHeight + '）' : '');
          [['bold', 'Font.Bold'], ['italic', 'Font.Italic']].forEach(function (p) {
            var cb = el('input');
            cb.type = 'checkbox'; cb.checked = !!lk[p[0]]; cb.setAttribute('data-field', p[0]); cb.disabled = !ed.inSource;
            cb.addEventListener('change', function () { vscode.postMessage({ type: 'setLook', prop: p[0], value: cb.checked }); });
            row(p[1], cb, '', dfm[p[0]], lk[p[0]], setLook(p[0]));
          });
          /* 0.152 (WPF: every property in the grid is editable -- these were grey .dfm-only rows): Font.Underline / StrikeOut,
             a label's WordWrap, an edit's ReadOnly / MaxLength, a check box's Checked, TabOrder; a row only where the control has it */
          var boolRow = function (key, label, cur, dv) {
            var cb2 = el('input');
            cb2.type = 'checkbox'; cb2.checked = !!cur; cb2.setAttribute('data-field', key); cb2.disabled = !ed.inSource;
            cb2.addEventListener('change', function () { vscode.postMessage({ type: 'setLook', prop: key, value: cb2.checked }); });
            row(label, cb2, '', dv, cur, setLook(key));
          };
          if (lk.underline != null) boolRow('underline', 'Font.Underline', lk.underline, dfm.underline);
          if (lk.strikeout != null) boolRow('strikeout', 'Font.StrikeOut', lk.strikeout, dfm.strikeout);
          if (lk.wordWrap != null) boolRow('wordWrap', 'WordWrap', lk.wordWrap, dfm.wordWrap);
          if (lk.readOnly != null) boolRow('readOnly', 'ReadOnly', lk.readOnly, dfm.readOnly);
          if (lk.checked != null) boolRow('checked', 'Checked', lk.checked, dfm.checked);
          if (lk.maxLength != null) row('MaxLength', num('maxLength', lk.maxLength, !ed.inSource, setLook('maxLength')), '0＝不限', dfm.maxLength, lk.maxLength, setLook('maxLength'));
          /* 0.157 (WPF's Items collection editor / String Collection Editor): a drop-down's / radio group's items, a memo's
             lines -- one per line, 套用 (or Ctrl+Enter) writes them; the one selected now stays selected */
          /* 0.158 a TImage's Picture (WPF Image.Source): its src typed, or 選擇… an image file (copied into img\ when outside) */
          /* 1006 a TLabeledEdit's EditLabel.Caption (the text beside the box) */
          if (lk.editLabel != null) {
            var elIn = el('input', 'txt');
            elIn.type = 'text'; elIn.value = lk.editLabel; elIn.spellcheck = false; elIn.setAttribute('data-field', 'editLabel'); elIn.disabled = !ed.inSource;
            elIn.title = '輸入框旁邊的標籤文字（BCB6 TLabeledEdit 的 EditLabel.Caption）；Enter＝寫進 HTML';
            var elSend = function () { if (elIn.value !== lk.editLabel) vscode.postMessage({ type: 'setLook', prop: 'editLabel', value: elIn.value }); };
            elIn.addEventListener('keydown', function (e) { if (e.key === 'Enter') { e.preventDefault(); elSend(); } });
            elIn.addEventListener('change', elSend);
            row('EditLabel.Caption', elIn, '', null, null, null);
          }
          if (lk.src != null) {
            var sIn = el('input', 'txt');
            sIn.type = 'text'; sIn.value = lk.src; sIn.spellcheck = false; sIn.setAttribute('data-field', 'src'); sIn.disabled = !ed.inSource;
            sIn.title = '圖片檔的路徑（相對於這一頁，例如 img/logo.png）；Enter＝寫進 HTML';
            sIn.addEventListener('keydown', function (e) { if (e.key === 'Enter') { e.preventDefault(); vscode.postMessage({ type: 'setLook', prop: 'src', value: sIn.value.trim() }); } });
            sIn.addEventListener('change', function () { if (sIn.value.trim() !== lk.src) vscode.postMessage({ type: 'setLook', prop: 'src', value: sIn.value.trim() }); });
            var sPick = el('button', 'srcPick', '選擇…');
            sPick.type = 'button'; sPick.disabled = !ed.inSource;
            sPick.addEventListener('click', function () { vscode.postMessage({ type: 'pickImage' }); });
            var sBox = el('div', 'srcBox'); sBox.appendChild(sIn); sBox.appendChild(sPick);
            row('Picture', sBox, lk.src ? '' : '（沒有圖）', null, null, null);
          }
          if (ed.items) {
            var itx = el('textarea', 'items');
            itx.value = (ed.items.items || []).join('\n');
            itx.rows = Math.max(3, Math.min(10, (ed.items.items || []).length + 1));
            itx.spellcheck = false;
            itx.setAttribute('data-field', 'items');
            itx.disabled = !ed.inSource;
            var itGo = el('button', 'itemsGo', '套用');
            itGo.type = 'button';
            itGo.disabled = !ed.inSource;
            itGo.title = '寫進 HTML（Ctrl+Enter 也可以）；Ctrl+Z 復原';
            var itSend = function () { vscode.postMessage({ type: 'setItems', items: itx.value === '' ? [] : itx.value.replace(/\r\n/g, '\n').split('\n') }); };
            itGo.addEventListener('click', itSend);
            itx.addEventListener('keydown', function (e) { if (e.key === 'Enter' && (e.ctrlKey || e.metaKey)) { e.preventDefault(); itSend(); } });
            /* 1006 (the enumeration of every row: the list typed into and then left -- a click elsewhere -- was dropped without a
               word, every other field writes when it is left): written when left changed, like the rest (套用 / Ctrl+Enter stay) */
            itx.addEventListener('change', itSend);
            var itBox = el('div', 'itemsBox');
            itBox.appendChild(itx); itBox.appendChild(itGo);
            /* 1006 (WinForms' String Collection Editor): the line the caret is on up / down, every line sorted -- written at once */
            var itLines = function () { return itx.value.replace(/\r\n/g, '\n').split('\n'); };
            var itLineAt = function () { return itx.value.slice(0, itx.selectionStart).split('\n').length - 1; };
            var itPut = function (ls, caretLine) {
              itx.value = ls.join('\n');
              var off = ls.slice(0, caretLine).reduce(function (a, l) { return a + l.length + 1; }, 0);
              try { itx.setSelectionRange(off, off + (ls[caretLine] || '').length); } catch (x) { /* ignore */ }
              itSend();
            };
            [['itemsUp', '上移', '游標所在的那一項往上移', -1], ['itemsDown', '下移', '游標所在的那一項往下移', 1], ['itemsSort', '排序', '全部項目照字母／數字排序']].forEach(function (b) {
              var bt = el('button', 'itemsMv ' + b[0], b[1]);
              bt.type = 'button'; bt.tabIndex = -1; bt.title = b[2]; bt.disabled = !ed.inSource;
              bt.setAttribute('data-act', b[0]);
              bt.addEventListener('mousedown', function (ev) { ev.preventDefault(); });   /* (the caret stays in the box) */
              bt.addEventListener('click', function () {
                var ls = itLines();
                if (b[0] === 'itemsSort') { ls.sort(function (x, y) { var a = x.trim() !== '' && isFinite(+x), c = y.trim() !== '' && isFinite(+y); return a && c ? (+x) - (+y) : a !== c ? (a ? -1 : 1) : x.localeCompare(y); }); return itPut(ls, 0); }
                var i = itLineAt(), j = i + b[3];
                if (j < 0 || j >= ls.length) return;
                var t = ls[i]; ls[i] = ls[j]; ls[j] = t;
                itPut(ls, j);
              });
              itBox.appendChild(bt);
            });
            var itName = ed.items.kind === 'memo' ? 'Lines' : 'Items';
            row(itName, itBox, (ed.items.items || []).length + (ed.items.kind === 'memo' ? ' 行' : ' 項') + '（一行一個）', null, null, null);
          }
          if (lk.tabOrder !== undefined && (lk.tabOrder != null || dfm.tabOrder != null)) {
            var toIn = num('tabOrder', lk.tabOrder == null ? '' : lk.tabOrder, !ed.inSource, setLook('tabOrder'));
            toIn.title = 'Tab 鍵的順序（tabindex）；空白＝照網頁的順序';
            row('TabOrder', toIn, lk.tabOrder == null ? '（未設定）' : '', dfm.tabOrder, lk.tabOrder, setLook('tabOrder'));
          }
          /* a colour row: the well + its #rrggbb in the row, BCB6's 16 colours and the eyedropper behind ▾ (WPF's brush
             editor) -- Font.Color / Color, and the IO lamp / panel button's TrueColor / FalseColor / TrueFontColor / FalseFontColor
             p = [the look property, the grid's name], cur = the page's value, dv = the .dfm's, sys = the .dfm's system colour */
          var colorRow = function (p, cur, dv, sys, deflt) {
            var lkv = {}; lkv[p[0]] = cur;
            var c = el('input', 'col');
            c.type = 'color'; c.value = cur || deflt;
            c.setAttribute('data-field', p[0]); c.disabled = !ed.inSource;
            c.addEventListener('change', function () { vscode.postMessage({ type: 'setLook', prop: p[0], value: c.value }); });
            var named = VCL16.filter(function (x) { return x[1] === String(lkv[p[0]] || '').toLowerCase(); })[0];
            c.title = lkv[p[0]] ? lkv[p[0]] + (named ? '（' + named[0] + '）' : '') : '（未設定）';
            row(p[1], c, lkv[p[0]] ? '' : '（未設定）', dv, lkv[p[0]], setLook(p[0]),
              sys ? { text: 'DFM ' + sys + '（系統色）', title: '系統色的實際顏色看 Windows 佈景，不比對' } : null);
            /* the value (the well + its #rrggbb) in the row; BCB6's 16 colours and the eyedropper behind ▾ (WPF's brush
               editor opens from the value too) */
            var td2 = tb.lastChild && tb.lastChild.querySelector('td.v');
            if (td2) {
              var pal = el('div', 'pal');
              pal.setAttribute('data-for', p[0]);
              var pt = el('button', 'paltog', '▾');
              pt.setAttribute('data-paltog', p[0]);
              pt.title = 'BCB6 的 16 色、系統色和滴管';
              /* (Tab: value to value -- the #rrggbb box is the value here; the well and ▾ are clicks) */
              pt.tabIndex = -1;
              c.tabIndex = -1;
              pt.addEventListener('click', function () { pal.classList.toggle('open'); pt.textContent = pal.classList.contains('open') ? '▴' : '▾'; });
              VCL16.forEach(function (x) {
                var sw = el('button', 'sw' + (named && named[0] === x[0] ? ' cur' : ''));
                sw.style.backgroundColor = x[1];
                sw.title = x[0] + '　' + x[1];
                sw.setAttribute('data-color', x[1]);
                sw.disabled = !ed.inSource;
                sw.addEventListener('click', function () { vscode.postMessage({ type: 'setLook', prop: p[0], value: x[1] }); });
                pal.appendChild(sw);
              });
              /* 1005: BCB6's system colours too, a row of their own (clBtnFace / clWindow ... -- the classic Windows values) */
              var sysl = el('div', 'palsys', '系統色');
              pal.appendChild(sysl);
              VCLSYS.slice(0, 16).forEach(function (x) {
                var sw = el('button', 'sw sys');
                sw.style.backgroundColor = x[1];
                sw.title = x[0] + '　' + x[1] + '（BCB6 系統色，傳統 Windows 配色）';
                sw.setAttribute('data-color', x[1]);
                sw.setAttribute('data-name', x[0]);
                sw.disabled = !ed.inSource;
                sw.addEventListener('click', function () { vscode.postMessage({ type: 'setLook', prop: p[0], value: x[1] }); });
                pal.appendChild(sw);
              });
              /* WPF's brush editor: the value typed in (#rrggbb / #rgb / a BCB6 name like clRed) and the eyedropper
                 (a colour picked anywhere on the screen -- the design surface too) */
              var hx = el('input', 'hex');
              hx.type = 'text'; hx.value = lkv[p[0]] || ''; hx.placeholder = '#rrggbb'; hx.spellcheck = false; hx.disabled = !ed.inSource;
              hx.setAttribute('data-field', p[0] + 'Hex');
              hx.title = '輸入顏色：#rrggbb、#rgb，或 BCB6 的名稱（clRed、clNavy…、系統色 clBtnFace、clWindow…），按 Enter';
              hx.addEventListener('focus', function () { hx._v0 = hx.value; });
              hx.addEventListener('keydown', function (e) {
                if (e.key === 'Escape') { hx.value = hx._v0 != null ? hx._v0 : (lkv[p[0]] || ''); hx.classList.remove('bad'); hx.blur(); return; }
                if (e.key !== 'Enter') return;
                e.preventDefault();
                var v = colorText(hx.value);
                hx.classList.toggle('bad', !v);
                if (v) vscode.postMessage({ type: 'setLook', prop: p[0], value: v });
              });
              td2.insertBefore(hx, c.nextSibling);
              td2.insertBefore(pt, hx.nextSibling);
              var dp = el('button', 'drop', '滴管');
              dp.setAttribute('data-drop', p[0]);
              dp.title = '滴管（WPF 的 eyedropper）：點一下，再點畫面上任何一點＝取那個顏色（設計畫面上的也行）；Esc 取消';
              dp.disabled = !ed.inSource || typeof window.EyeDropper !== 'function';
              dp.addEventListener('click', function () {
                try {
                  new window.EyeDropper().open().then(function (r) {
                    var v = r && colorText(r.sRGBHex);
                    if (v) vscode.postMessage({ type: 'setLook', prop: p[0], value: v });
                  }, function () { /* given up (Esc) */ });
                } catch (x) { /* no eyedropper here */ }
              });
              pal.appendChild(dp);
              td2.appendChild(pal);
            }
          };
          [['color', 'Font.Color'], ['background', 'Color']].forEach(function (p) {
            colorRow(p, lk[p[0]], dfm[p[0]], dfm[p[0] + 'Sys'], p[0] === 'color' ? '#000000' : '#ffffff');
          });
          /* the IO lamp (TALed / TMyLed / TMyLedLane) and panel button (TBtnPanel / TBtnPanelLane) properties, BCB6's
             names (EastSun 20261001 "把舊版的功能架構仿造到現有架構"): on the machine page the IO decides lit / down
             (ht9045_io_widgets.js); here they are the drawn state */
          var io = lk.io, dio = (dfm && dfm.io) || {};
          var ioSel = function (field, opts, cur, onPick) {
            var s = el('select', 'txt');
            opts.forEach(function (o) { var op = el('option', null, o); op.value = o; if (o === cur) op.selected = true; s.appendChild(op); });
            s.setAttribute('data-field', field); s.disabled = !ed.inSource;
            s.addEventListener('change', function () { onPick(s.value); });
            return s;
          };
          var ioCheck = function (field, cur) {
            var cb = el('input');
            cb.type = 'checkbox'; cb.checked = !!cur; cb.setAttribute('data-field', field); cb.disabled = !ed.inSource;
            cb.addEventListener('change', function () { vscode.postMessage({ type: 'setLook', prop: field, value: cb.checked }); });
            return cb;
          };
          if (io && io.kind === 'led') {
            var LEDS = ['LEDSmall', 'LEDLarge', 'LEDSqSmall', 'LEDSqLarge', 'LEDVertical', 'LEDHorizontal'];
            row('LEDStyle', ioSel('io.ledStyle', LEDS, io.ledStyle, setLook('io.ledStyle')), '', dio.ledStyle, io.ledStyle, setLook('io.ledStyle'));
            row('Value', ioCheck('io.value', io.value), '（亮著畫；機台上由 IO 決定）', dio.value, io.value, setLook('io.value'));
            row('Blink', ioCheck('io.blink', io.blink), '', dio.blink, io.blink, setLook('io.blink'));
            colorRow(['io.trueColor', 'TrueColor'], io.trueColor, dio.trueColor, null, '#00ff00');
            colorRow(['io.falseColor', 'FalseColor'], io.falseColor, dio.falseColor, null, '#c0c0c0');
          } else if (io && io.kind === 'btn') {
            var ioStyleName = function (flat) { return flat ? 'tsFlatButtons' : 'tsButtons'; };
            var setFlat = function (v) { setLook('io.flat')(v === 'tsFlatButtons' || v === true); };
            row('Style', ioSel('io.flat', ['tsButtons', 'tsFlatButtons'], ioStyleName(io.flat), setFlat), '',
              dio.flat == null ? null : ioStyleName(dio.flat), ioStyleName(io.flat), setFlat);
            row('Down', ioCheck('io.down', io.down), '（按下的樣子；機台上由輸出回讀決定）', dio.down, io.down, setLook('io.down'));
            colorRow(['io.trueColor', 'TrueColor'], io.trueColor, dio.trueColor, null, '#00c000');
            colorRow(['io.falseColor', 'FalseColor'], io.falseColor, dio.falseColor, null, '#ece9d8');
            colorRow(['io.trueFontColor', 'TrueFontColor'], io.trueFontColor, dio.trueFontColor, null, '#ffffff');
            colorRow(['io.falseFontColor', 'FalseFontColor'], io.falseFontColor, dio.falseFontColor, null, '#000000');
          }
          /* 1006 (the audit of component-specific editing): a TTrackBar / TScrollBar's Min / Max / Position */
          if (lk.range) {
            row('Min', num('range.min', lk.range.min, !ed.inSource, setLook('range.min')), '', null, null, null);
            row('Max', num('range.max', lk.range.max, !ed.inSource, setLook('range.max')), '', null, null, null);
            row('Position', num('range.value', lk.range.value, !ed.inSource, setLook('range.value')), '（開啟時的位置）', null, null, null);
          }
          /* 1006 a TTMyTray (the tray grid): XItem / YItem (cells), XBlockItem / YBlockItem (a gap every n cells, 0 = none),
             TrayColor, TrayDirect (the corner mark) -- the grid drawn again at once */
          if (lk.tray) {
            var trs = lk.tray;
            [['xitem', 'XItem', '（橫的格數）'], ['yitem', 'YItem', '（直的格數）'], ['xblockItem', 'XBlockItem', '每幾格分一區，0＝不分'], ['yblockItem', 'YBlockItem', '每幾格分一區，0＝不分']].forEach(function (p) {
              row(p[1], num('tray.' + p[0], trs[p[0]] == null ? 0 : trs[p[0]], !ed.inSource, setLook('tray.' + p[0])), p[2], null, null, null);
            });
            colorRow(['tray.trayColor', 'TrayColor'], trs.trayColor, null, null, '#8a9a8a');
            row('TrayDirect', ioSel('tray.trayDirect', ['csNull', 'csLeftTop', 'csLeftBottom', 'csRightTop', 'csRightBottom'], trs.trayDirect || 'csNull', setLook('tray.trayDirect')), '方向角標', null, null, null);
          }
        }
        /* several selected and their values differ: WPF shows the field empty (a checkbox half
           set); typing a value still sets it on all of them. Not compared with the DFM there. */
        var MX = ed.mixed || {};
        Object.keys(MX).forEach(function (f) {
          var inp = tb.querySelector('[data-field="' + f + '"]');
          if (!inp || !MX[f]) return;
          if (inp.type === 'checkbox') inp.indeterminate = true;
          else if (inp.type !== 'color') { inp.value = ''; inp.placeholder = '（不同）'; }
          var mtr = inp.closest('tr');
          if (!mtr) return;
          mtr.classList.remove('diff');
          mtr.classList.add('mixed');
          mtr.title = '選取的元件這一項的值不一樣；改了就全部一起改';
          var mh = mtr.querySelector('.hint');
          if (inp.type === 'color') { if (mh) mh.textContent = '（不同）'; else inp.parentNode.insertBefore(el('span', 'hint', '（不同）'), inp.nextSibling); }
          var mdv = mtr.querySelector('.dfmv');
          if (mdv) mdv.textContent = '（值不同，不比對）';
          /* (the marker stays -- the column the same on every row -- hollow, its Reset off) */
          var mrb = mtr.querySelector('button.reset');
          if (mrb) { mrb.classList.remove('set'); mrb.classList.add('none'); mrb.title = '選取的元件這一項的值不一樣，不比對\n點一下：跳到 HTML 原始碼'; mrb.setAttribute('aria-label', mrb.title); }
        });
        diffs = tb.querySelectorAll('tr.diff').length;
        /* AI(W906-HTDESIGNER) 20261001 (EastSun "屬性表格和事件可以像 wpf 讓使用者看起來條理分明"; WPF: ONE list, each
           category once): the .dfm's other properties (read only, greyed) are rows of this same table, in the same
           categories -- they were a second list below with its own 版面 / 外觀, and by name two A-Z runs. (Several
           selected: the primary one's .dfm list stays apart, below.) */
        if (!ed.multi) (data.props || []).filter(function (p) { return !inGrid(p); }).forEach(function (p) {
          var rtr = el('tr', 'ro');
          var rk = el('td', 'k', p[0]);
          describeKey(rk, p[0], p[4] ? '雙擊：跳到 .dfm 第 ' + p[4] + ' 行' : '');
          var rv = el('td', 'v');
          if (p[2]) { var rsw = el('span', 'sw'); rsw.style.background = p[2]; rv.appendChild(rsw); }
          rv.appendChild(document.createTextNode(p[1] == null ? '' : String(p[1])));
          if (p[3]) rv.appendChild(el('span', 'hint', p[3]));
          var rmc = el('td', 'm');
          var rmk = el('button', 'reset marker none ro', '');
          rmk.tabIndex = -1;
          rmk.title = '唯讀：BCB6 .dfm 的值（這裡不能改）\n點一下：跳到 .dfm、複製值';
          var ropen = function () { if (p[4]) vscode.postMessage({ type: 'openDfmLine', line: p[4] }); else vscode.postMessage({ type: 'copy', text: String(p[1]) }); };
          var rmenu = function (x, y) {
            menu(x, y, [[p[4] ? '跳到 .dfm（第 ' + p[4] + ' 行）' : '跳到 .dfm（沒有行號）', !!p[4], ropen],
              ['複製值', true, function () { vscode.postMessage({ type: 'copy', text: String(p[1]) }); }]]);
          };
          rmk.addEventListener('click', function (e) { e.stopPropagation(); var r = rmk.getBoundingClientRect(); rmenu(r.left, r.bottom + 2); });
          rtr.addEventListener('contextmenu', function (e) { e.preventDefault(); rmenu(e.clientX, e.clientY); });
          rmc.appendChild(rmk);
          rtr.appendChild(rk); rtr.appendChild(rv); rtr.appendChild(rmc);
          rtr.setAttribute('data-ro', p[0]);
          if (p[4]) rtr.setAttribute('data-line', p[4]);
          rtr.title = '唯讀（BCB6 .dfm 的值，網頁沒有對應的屬性可以改）';
          rtr.addEventListener('dblclick', ropen);
          tb.appendChild(rtr);
        });
        /* WPF's property grid: "Arrange by" Category (its default) / Name / Source -- the categories WPF puts these in
           (Common, Layout, Appearance / Brush, Text); Source = the order they are built in */
        var CAT_ED = { Alias: '一般', Caption: '一般', Text: '一般', Enabled: '一般', Visible: '外觀', Color: '外觀', 'Font.Color': '外觀',
          Left: '版面', Top: '版面', Width: '版面', Height: '版面', AutoSize: '版面',
          Alignment: '文字', 'Font.Name': '文字', 'Font.Size': '文字', 'Font.Bold': '文字', 'Font.Italic': '文字',
          'Font.Underline': '文字', 'Font.StrikeOut': '文字', WordWrap: '文字', Checked: '一般', Items: '一般', Lines: '一般', Picture: '外觀', ReadOnly: '行為', MaxLength: '行為', TabOrder: '行為',
          // (the IO lamp / panel button's own: one category, as WPF gives a control's own properties theirs)
          LEDStyle: 'IO 元件', Value: 'IO 元件', Blink: 'IO 元件', Style: 'IO 元件', Down: 'IO 元件', TrueColor: 'IO 元件', FalseColor: 'IO 元件',
          TrueFontColor: 'IO 元件', FalseFontColor: 'IO 元件' };
        var arrange = arrangeBy();
        var edRows = Array.prototype.filter.call(tb.children, function (x) { return x.tagName === 'TR'; });
        var rowKey = function (tr) { var k = tr.querySelector('td.k'); return k ? k.textContent : ''; };
        /* one set of categories for both kinds of row: the grid's (CAT_ED), and the .dfm names by what they are */
        var catOfRo = function (n) {
          var head = String(n).split('.')[0];
          if (head === 'Font' || /^(Alignment|WordWrap|Layout)$/.test(head)) return '文字';
          if (/^(Caption|Text|Hint|ShowHint|ParentShowHint|Tag|Action|PopupMenu)$/.test(head)) return '一般';
          return propCat(n);
        };
        var catOfRow = function (tr) { var k = rowKey(tr); return tr.classList.contains('ro') ? catOfRo(k) : (CAT_ED[k] || '其他'); };
        var lineOf = function (tr) {
          if (tr.classList.contains('ro')) return +tr.getAttribute('data-line') || 1e9;
          var dp = dfmPropOf(rowKey(tr));
          return dp && dp[4] ? dp[4] : 1e9;
        };
        if (arrange === 'dfm' && edRows.length > 1) {
          /* the .dfm's own order (by line), one list: a row without a line keeps its place at the end */
          edRows.forEach(function (tr) { tb.removeChild(tr); });
          edRows.map(function (tr, i) { return [tr, i]; }).sort(function (x, y) { return (lineOf(x[0]) - lineOf(y[0])) || (x[1] - y[1]); })
            .forEach(function (x) { tb.appendChild(x[0]); });
        }
        if (arrange !== 'dfm' && edRows.length > 1) {
          edRows.forEach(function (tr) { tb.removeChild(tr); });
          if (arrange === 'name') {
            edRows.sort(function (a, b) { return rowKey(a).localeCompare(rowKey(b), 'en', { sensitivity: 'base' }); }).forEach(function (tr) { tb.appendChild(tr); });
          } else ['一般', 'IO 元件', '版面', '外觀', '文字', '行為', '其他'].forEach(function (g) {
            var inG = edRows.filter(function (tr) { return catOfRow(tr) === g; });
            if (!inG.length) return;
            var hr = el('tr', 'cathead');
            hr.setAttribute('data-cat', g);
            var hd = el('td', 'subhead', g);
            hd.colSpan = 3;
            hr.appendChild(hd);
            catToggle(hr, hd, 'ed:' + g);
            tb.appendChild(hr);
            inG.forEach(function (tr) { tr.setAttribute('data-catof', 'ed:' + g); tb.appendChild(tr); });
          });
        }
        if (diffs) {
          var dn = el('div', 'pad small diffnote', '⚠ ' + diffs + ' 項和 DFM 不同 ');
          dn.title = '和 BCB6 .dfm 的值不同的列會標色，值後面寫著 DFM 的值；點那一列右邊的小方塊＝改回 DFM 的值';
          var ra = el('button', 'resetall', ed.multi ? '選取的 ' + (ed.multi.length + 1) + ' 個全部改回 DFM' : '全部改回 DFM');
          ra.setAttribute('data-act', 'resetToDfm');
          ra.title = '位置、大小、文字、外觀一次改回 .dfm 的值（一個 Ctrl+Z 可以全部復原）';
          ra.disabled = !ed.inSource;
          ra.addEventListener('click', function () { vscode.postMessage({ type: 'resetToDfm' }); });
          dn.appendChild(ra);
          body.appendChild(dn);
        }
        body.appendChild(tb);
        var note;
        if (!ed.inSource) note = '這個元件是頁面 JS 產生的，HTML 原始碼裡沒有它，不能在這裡改。';
        else if (ed.locked) note = '🔒「' + ed.locked + '」鎖定了：位置／大小不能改（在「元件」樹上按鎖頭解除）；文字和外觀可以改。';
        else if (lay && !ed.layoutInSource) note = '位置由外層元素決定、原始碼對不上，位置／大小不能改；文字和外觀可以。';
        else note = '';
        if (note) body.appendChild(el('div', 'dim pad small', note));
        else {
          /* (the everyday note: one short line, the rest in its tooltip -- WPF's grid has no text under it) */
          var nt = el('div', 'dim pad small', 'ⓘ 改的是 HTML 原始碼' + (ed.dirty ? '（有未存檔的修改）' : '') + '：Ctrl+Z 復原、Ctrl+S 存檔');
          nt.title = '每一列右邊的小方塊（或在那一列按右鍵）：重設、跳到 HTML 原始碼；雙擊名稱也會跳過去。\n也可以直接在畫面上拖曳、拉控制點、用方向鍵微調。這一頁由 sync_web.py 同步，--apply 會覆蓋。';
          body.appendChild(nt);
        }
      });
    }

    /* events: the Events tab of BCB6's Object Inspector / WPF's Properties window -- every event of the class
       (EastSun: "像WPF一樣有很多表格事件，如果有設定事件上面就會顯示是什麼函式，並且雙擊兩下可以跑到對應CODE，
       如果沒有設定就會自動新增相關設定"). A set one: its handler, and where it is wired (the page sends it /
       the server's form.event table has it / the C++ port / BCB6); a double-click opens its code. An empty
       one: a double-click adds the handler to the form's class in the C++ port tree (like BCB6). */
    var eg = data.eventGrid;
    if (eg && eg.rows && eg.rows.length) {
      var nSet = eg.rows.filter(function (x) { return !!x.handler; }).length;
      /* WPF / Windows Forms (learn.microsoft.com "How to: Create a Simple Event Handler", "How to add or remove an
         event handler"): two columns, the event and its handler. "Select an event and put the cursor in the value
         column. Type the event handler name or leave it blank to use the default name. To create the event handler,
         press ENTER or double-click the value column." A set one: double-click = "navigates to the existing handler".
         The value's list = "all methods that have a compatible method signature". Right-click -> Reset. Nothing else
         in the grid: where it is wired is its tooltip (EastSun 20260930: "你確定這是件是WPF 操作相似?"). */
      /* AI(W906-HTDESIGNER) 20261001 (BCB6's Object Inspector / Windows Forms with several selected): only the events
         they all have; handlers that differ = the value empty; a name typed = that function for all of them */
      var many = eg.multi && eg.multi.length ? eg.multi.length + 1 : 0;
      var evSecEl = section('events', many ? '元件事件（C++，' + many + ' 個共有的）' : '元件事件（C++）', nSet + ' / ' + eg.rows.length, function (body) {
        if (many) {
          var mn = el('div', 'pad small multinote', '多選 ' + many + ' 個：只列它們都有的事件；打名稱＋Enter＝全部接到同一個函式，清掉＝全部拿掉（顯示的是「' + c.name + '」的；不一樣的留白）。');
          body.appendChild(mn);
        }
        var tip = function (rw) {
          var t = [];
          if (rw.main != null && data.targets[rw.main]) {
            var mt = data.targets[rw.main];
            t.push('會執行的 C++：' + (rw.mainName || rw.handler) + '（' + base(mt.file) + ':' + mt.line + '）');
          } else if (rw.handler) t.push(rw.portState === 'dead' ? '移植樹的 ' + rw.handler + ' 在 #if 0 裡，不會執行' : '移植樹沒有會執行的 C++ 函式');
          if (rw.dfm && rw.mainName && rw.mainName !== rw.handler) t.push('BCB6 .dfm：' + rw.handler);
          if (rw.cmd) t.push('網頁 → WS ' + rw.cmd + (rw.web != null ? '' : '（網頁沒有送）') + (rw.server != null ? ' → 伺服器' : '（伺服器還沒接上：存檔、重建伺服器程式）'));
          else if (rw.fe && (rw.web != null || rw.server != null)) t.push('網頁 form.event：' + (rw.web != null ? '網頁會送' : '網頁沒有送') + '、' + (rw.server != null ? '伺服器有' : '伺服器沒有'));
          if (rw.idx >= 0 && data.events[rw.idx]) data.events[rw.idx].targets.forEach(function (k) { var g = data.targets[k]; if (g && g.kind === 'golden') t.push('BCB6：' + base(g.file) + ':' + g.line); });
          if (rw.portState === 'new') t.push('（還沒存檔）');
          t.push(rw.handler ? '雙擊＝跳到程式碼；右鍵＝重設、所有找到的程式碼' : '打名稱＋Enter、或雙擊（空白＝' + rw.conv + '）＝新增；清單＝用已經有的函式');
          return t.join('\n');
        };
        /* WPF's Events tab lists them A-Z */
        eg.rows.slice().sort(function (a, b) { return a.name.localeCompare(b.name, 'en', { sensitivity: 'base' }); }).forEach(function (rw) {
          var r = el('div', 'row event evg' + (rw.handler ? ' set' : ' empty'));
          r.tabIndex = -1;   /* (Tab goes field to field, as in WPF's Events tab; the row still takes a click) */
          r.setAttribute('data-event', rw.name);
          r.title = tip(rw);
          r.appendChild(el('span', 'evn', rw.name));
          /* the value: the function that runs (EastSun: "只對應到 W906_Main_CloseProgramOp"), else the handler, else empty */
          var shown = rw.main != null && rw.mainName ? String(rw.mainName).replace(/（.*$/, '') : (rw.handler || '');
          var mine = !rw.dfm && !(rw.cmd && rw.via !== 'htd');   // BCB6's .dfm / another page script's: read only
          /* several selected: read only when any of them has a .dfm / page-script one; their handlers differ = empty */
          if (many) { mine = !rw.multiRo; if (rw.mixed) shown = ''; }
          var h = el('input', 'evh evin');
          h.type = 'text'; h.spellcheck = false; h.value = shown;
          h.setAttribute('data-field', 'event:' + rw.name);
          if (many && rw.mixed) {
            h.placeholder = '（不同）';
            r.classList.remove('set');
            r.classList.add('mixed');
            r.title = '選取的元件這個事件的處理函式不一樣；打名稱＋Enter＝全部接到同一個，雙擊＝全部接到 ' + rw.conv + (rw.multiRo ? '\n（有 BCB6 .dfm 指定的，唯讀：一個一個選來改）' : '');
          }
          if (!mine) { h.readOnly = true; h.classList.add('ro'); }
          if (mine && rw.pick && rw.pick.length) {
            var lid = 'evpick-' + rw.name;
            var dl = el('datalist');
            dl.id = lid;
            rw.pick.forEach(function (n) { var o = el('option'); o.value = n; dl.appendChild(o); });
            r.appendChild(dl);
            h.setAttribute('list', lid);
          }
          var sent = null;
          var mixed = !!(many && rw.mixed);
          var commit = function () {
            var v = h.value.trim();
            if (v === sent) return;
            /* (handlers that differ: the primary one's name typed still attaches it to all; empty there = nothing typed) */
            if (mixed && !v) return;
            if (!mixed && v === (rw.handler || '')) { if (!v && !rw.handler) return; if (v) return; }
            sent = v;
            vscode.postMessage({ type: 'eventName', event: rw.name, value: v });
          };
          var open = function () { vscode.postMessage({ type: 'eventGrid', name: rw.name }); };
          h.addEventListener('keydown', function (e) {
            e.stopPropagation();
            if (e.key === 'Enter') {
              /* blank + Enter on an empty one = the default name (WPF) */
              if (mine && !h.value.trim() && (!rw.handler || mixed)) { sent = ''; open(); return; }
              if (mine) commit(); else open();
            } else if (e.key === 'Escape') { h.value = shown; h.blur(); }
          });
          h.addEventListener('change', function () { if (mine) commit(); });
          h.addEventListener('dblclick', function (e) { e.stopPropagation(); open(); });
          r.appendChild(h);
          r.addEventListener('dblclick', open);
          r.addEventListener('keydown', function (e) { if (e.key === 'Enter') open(); });
          r.addEventListener('click', function () { mark(r); });
          r.addEventListener('contextmenu', function (e) {
            e.preventDefault();
            mark(r);
            menu(e.clientX, e.clientY, [
              ['跳到程式碼', !!rw.handler && !mixed, open],
              ['所有找到的程式碼…', !!rw.handler, function () { vscode.postMessage({ type: 'eventAll', name: rw.name }); }],
              [many ? '重設（' + many + ' 個全部）' : '重設', (!!rw.handler || mixed) && mine, function () { vscode.postMessage({ type: 'eventReset', name: rw.name }); }],
            ]);
          });
          body.appendChild(r);
        });
      });
      if (evSecEl && evSecEl.firstChild) evSecEl.firstChild.title = 'BCB6 元件的事件（OnClick…）和它在 C++ 移植樹裡的處理函式：打名稱＋Enter 或雙擊＝新增（空白＝預設名稱），雙擊有函式的＝跳到程式碼，右鍵＝重設';
    } else
    section('events', '元件事件（C++）', data.events.length, function (body) {
      if (data.cppPending) body.appendChild(el('div', 'ind dim pad', '（正在整理這個元件型別的所有事件…）'));
      if (!data.events.length) {
        body.appendChild(el('div', 'dim pad', c.inIr ? 'DFM 沒有給這個元件指定事件。' : '沒有 DFM 資料，事件看下面的「網頁 JS 監聽器」。'));
        return;
      }
      data.events.forEach(function (ev, i) {
        var r = el('div', 'row event');
        r.tabIndex = 0;
        r.title = '雙擊：跳到 ' + ev.handler + ' 的程式碼';
        r.appendChild(el('span', 'evn', ev.name));
        r.appendChild(el('span', 'evh', ev.handler));
        var kinds = {};
        ev.targets.forEach(function (k) { var t = data.targets[k]; if (t) kinds[t.kind] = 1; });
        ['web', 'port', 'golden'].forEach(function (k) {
          var chip = el('span', 'chip mini k-' + k + (kinds[k] ? '' : ' off'), KIND[k].chip);
          chip.title = kinds[k] ? KIND[k].title : '找不到（' + KIND[k].title + '）';
          r.appendChild(chip);
        });
        var open = function () { vscode.postMessage({ type: 'openEvent', i: i }); };
        r.addEventListener('dblclick', open);
        r.addEventListener('keydown', function (e) { if (e.key === 'Enter') open(); });
        r.addEventListener('click', function () { mark(r); });
        body.appendChild(r);
        ev.targets.forEach(function (k) { body.appendChild(targetRow(k, true)); });
        if (ev.cppPending) body.appendChild(el('div', 'ind dim pad', '（正在找 C++ 與 BCB6 的程式碼…）'));
        else if (!ev.targets.length) body.appendChild(el('div', 'ind dim pad', '三個地方都找不到這個函式。'));
      });
    });

    /* 網頁事件: the page's own JS handler of each DOM event, typed in (EastSun: "網路js監聽器也要讓我填function"):
       a name adds the function and its binding to the page's <script id="htdEvents"> block, a new name renames it,
       clearing takes the binding out; a double-click opens the function */
    var je = data.edit && data.edit.jsEvents;
    if (je && !(data.edit && data.edit.multi)) {
      var nJs = Object.keys(je.handlers || {}).length;
      var jsSecEl = section('jsevents', '網頁事件（JS）', nJs + ' / ' + je.types.length, function (body) {
        je.types.slice().sort(function (a, b) { return a.localeCompare(b, 'en', { sensitivity: 'base' }); }).forEach(function (ty) {
          var cur = (je.handlers || {})[ty];
          var r = el('div', 'row event jsev' + (cur ? ' set' : ' empty'));
          r.setAttribute('data-jsevent', ty);
          r.appendChild(el('span', 'evn', ty));
          var inp = el('input', 'evh evin');
          inp.type = 'text'; inp.spellcheck = false; inp.value = cur ? cur.fn : '';
          /* an empty one looks empty (WPF); its default name shows once you are in it */
          var defName = (c.htmlId || 'x') + ty.charAt(0).toUpperCase() + ty.slice(1);
          inp.addEventListener('focus', function () { inp.placeholder = defName; });
          inp.addEventListener('blur', function () { inp.placeholder = ''; });
          r.title = cur ? '雙擊＝打開 ' + cur.fn + '；改名稱＝跟著改；清掉＝拿掉' : '打函式名稱＋Enter，或雙擊／空白按 Enter（＝' + defName + '）＝在這一頁產生函式並接好';
          inp.setAttribute('data-field', 'js:' + ty);
          /* (written once: Enter -- staying in the field -- or leaving it) */
          var jsSent = cur ? cur.fn : '';
          var jsCommit = function () { var v = inp.value.trim(); if (v === jsSent) return; jsSent = v; vscode.postMessage({ type: 'jsEvent', ev: ty, fn: v }); };
          inp.addEventListener('change', jsCommit);
          /* AI(W906-HTDESIGNER) 20261001: like the C++ table (〔EH〕"leave it blank to use the default name. To create the event
             handler, press ENTER or double-click the value column"): blank + Enter / a double click on an empty one = the default name */
          var jsDefault = function () { if (cur || inp.value.trim()) return false; inp.value = defName; jsCommit(); return true; };
          inp.addEventListener('keydown', function (e) {
            e.stopPropagation();
            if (e.key === 'Enter') { e.preventDefault(); if (!jsDefault()) jsCommit(); inp.select(); }
            else if (e.key === 'Escape') { inp.value = cur ? cur.fn : ''; inp.blur(); }
          });
          inp.addEventListener('dblclick', function (e) { e.stopPropagation(); if (cur) vscode.postMessage({ type: 'openJsEvent', ev: ty }); else jsDefault(); });
          r.appendChild(inp);
          /* right-click: the same menu as the component's events (WPF's Events tab: Reset) */
          r.addEventListener('contextmenu', function (e) {
            e.preventDefault();
            menu(e.clientX, e.clientY, [
              ['跳到程式碼', !!cur, function () { vscode.postMessage({ type: 'openJsEvent', ev: ty }); }],
              ['重設（拿掉這個連線）', !!cur, function () { inp.value = ''; jsCommit(); }],
            ]);
          });
          /* other JS on it (the page's own wire files), measured in the preview */
          var others = data.listeners.filter(function (l) { return l.on === 'self' && l.type === ty; }).length - (cur ? 1 : 0);
          if (others > 0) {
            var oh = el('span', 'hint jsmore', '＋' + others);
            oh.title = '這一頁的程式另外還有 ' + others + ' 個 ' + ty + ' 監聽器（在〔</> 程式碼〕頁的「網頁 JS 監聽器」）';
            r.appendChild(oh);
          }
          r.addEventListener('dblclick', function () { if (cur) vscode.postMessage({ type: 'openJsEvent', ev: ty }); else jsDefault(); });
          body.appendChild(r);
        });
      });
      if (jsSecEl && jsSecEl.firstChild) jsSecEl.firstChild.title = '這一頁 HTML 自己的 JS：打函式名稱＝在檔尾 <script id="htdEvents"> 產生函式和 addEventListener；改名稱＝跟著改；清掉＝拿掉；雙擊＝打開函式';
    }

    /* web listeners */
    var own = data.listeners.filter(function (l) { return l.on !== 'up'; });
    var upRel = data.listeners.filter(function (l) { return l.on === 'up' && l.relevant; });
    var upOther = data.listeners.filter(function (l) { return l.on === 'up' && !l.relevant; });
    section('listeners', '網頁 JS 監聽器（執行期實測）', own.length + upRel.length, function (body) {
      function lrow(l) {
        var wrap = el('div', 'lst');
        var head = el('div', 'lhead');
        head.appendChild(el('span', 'ltype', l.type));
        var where = l.on === 'self' ? '綁在這個元件' : l.on === 'attr' ? 'HTML 屬性 on' + l.type : '綁在上層 ' + l.onLabel + '（事件委派）';
        head.appendChild(el('span', 'lwhere', where));
        if (l.fnName) head.appendChild(el('span', 'lfn', l.fnName + '()'));
        if (l.via === 'prop') head.appendChild(el('span', 'lfn', '.on' + l.type + ' ='));
        if (l.cap) head.appendChild(el('span', 'lfn', 'capture'));
        wrap.appendChild(head);
        if (l.code) wrap.appendChild(el('div', 'snip ind', l.code));
        /* the handler written where it is bound (addEventListener('click', function …)): ONE row, not the same line twice */
        var th = l.handler != null ? data.targets[l.handler] : null, tbd = l.bind != null ? data.targets[l.bind] : null;
        var oneLine = th && tbd && l.handler !== l.bind && th.file === tbd.file && th.line === tbd.line;
        if (oneLine) {
          var r1 = targetRow(l.bind, true);
          var n1 = r1.querySelector('.note');
          if (n1) { n1.textContent = '綁定＋處理函式'; n1.title = '處理函式就寫在綁定的那一行（addEventListener 裡的 function）'; }
          r1.setAttribute('data-oneline', '1');
          wrap.appendChild(r1);
        } else {
          if (l.handler != null && l.handler !== l.bind) wrap.appendChild(targetRow(l.handler, true));
          if (l.bind != null) wrap.appendChild(targetRow(l.bind, true));
        }
        if (l.handler == null && l.bind == null && !l.code) wrap.appendChild(el('div', 'ind dim pad', '找不到原始碼位置（可能是動態產生的函式）'));
        return wrap;
      }
      if (!own.length && !upRel.length) body.appendChild(el('div', 'dim pad', '這個元件本身沒有綁任何監聽器。'));
      own.forEach(function (l) { body.appendChild(lrow(l)); });
      upRel.forEach(function (l) { body.appendChild(lrow(l)); });
      if (upOther.length) {
        var more = el('details', 'more');
        more.appendChild(el('summary', null, '其他上層監聽器（整頁通用的，例如 theme.js）' + upOther.length + ' 個'));
        upOther.forEach(function (l) { more.appendChild(lrow(l)); });
        body.appendChild(more);
      }
      body.appendChild(el('div', 'dim pad small', '這些是頁面在預覽裡真的綁上去的；只在連線成功後才綁的監聽器看不到（預覽不連線）。'));
    });

    /* mentions */
    if (data.mentions.length) {
      section('mentions', '網頁 JS 提到此元件', data.mentions.length, function (body) {
        data.mentions.forEach(function (k) { body.appendChild(targetRow(k, false)); });
      });
    }

    /* commands to C++: web handler -> ... -> cmd('x.y') -> server dispatch -> C++ function */
    if (data.cmds.length || data.cppPending) {
      section('cmds', '送到 C++ 的命令', data.cmds.length, function (body) {
        if (data.cppPending && !data.cmds.length) body.appendChild(el('div', 'dim pad', '（追蹤中…）'));
        data.cmds.forEach(function (cm) {
          var r = el('div', 'row cmd');
          r.appendChild(el('span', 'evh', cm.cmd));
          body.appendChild(r);
          if (cm.via) body.appendChild(el('div', 'ind dim small', '路徑：' + cm.via + ' → 送出'));
          (cm.send || []).forEach(function (k) { body.appendChild(targetRow(k, true)); });
          (cm.dispatch || []).forEach(function (k) { body.appendChild(targetRow(k, true)); });
          (cm.handlers || []).forEach(function (k) { body.appendChild(targetRow(k, true)); });
          if (!(cm.dispatch || []).length && !(cm.handlers || []).length) {
            body.appendChild(el('div', 'ind dim pad', data.cppPending ? '（正在找 C++ 端…）' : 'C++ 移植樹裡找不到分派這個命令的地方'));
          }
          if ((cm.other || []).length) {
            var more = el('details', 'more ind');
            more.appendChild(el('summary', null, '其他出現這個字串的地方 ' + cm.other.length + ' 處'));
            cm.other.forEach(function (k) { more.appendChild(targetRow(k, true)); });
            body.appendChild(more);
          }
        });
        body.appendChild(el('div', 'dim pad small', '從網頁的處理函式往下追最多 3 層呼叫，找 cmd()／rawCmd()／raw()／run() 送出的命令；命令名稱是組出來的就追不到。'));
      });
    }

    /* the setting / recipe field this control edits, and who reads/writes it in C++ */
    if ((data.fields && data.fields.length) || (data.cppPending && c.htmlId && !c.isForm && !data.fields)) {
      section('fields', '資料欄位', data.fields ? data.fields.length : null, function (body) {
        if (!data.fields) { body.appendChild(el('div', 'dim pad', '（搜尋中…）')); return; }
        data.fields.forEach(function (f) {
          var r = el('div', 'row cmd');
          r.appendChild(el('span', 'evh', '[' + f.section + '] ' + f.key));
          if (f.note) r.appendChild(el('span', 'note', f.note));
          body.appendChild(r);
          (Array.isArray(f.web) ? f.web : [f.web]).forEach(function (k) { body.appendChild(targetRow(k, true)); });
          if (f.port.length) { body.appendChild(el('div', 'ind subhead', 'C++ 移植樹')); f.port.forEach(function (k) { body.appendChild(targetRow(k, true)); }); }
          if (f.golden.length) { body.appendChild(el('div', 'ind subhead', 'BCB6 原始碼')); f.golden.forEach(function (k) { body.appendChild(targetRow(k, true)); }); }
          if (!f.port.length && !f.golden.length) body.appendChild(el('div', 'ind dim pad', 'C++ 和 BCB6 裡都找不到 "' + f.key + '" 這個字串'));
        });
        body.appendChild(el('div', 'dim pad small', '來自 wire 腳本的欄位對照（' + c.htmlId + ": ['section', 'key']）。C++ 那邊先找同一行同時寫了 section 和 key 的；找不到才只比對 key。"));
      });
    }

    /* live data shown by this control: the tag and who publishes it in C++ */
    if (data.tags && data.tags.length) {
      section('tags', '顯示的資料標籤', data.tags.length, function (body) {
        data.tags.forEach(function (t) {
          var r = el('div', 'row cmd');
          r.appendChild(el('span', 'evh', t.tag));
          r.appendChild(el('span', 'note', '顯示為 ' + t.prop));
          body.appendChild(r);
          body.appendChild(targetRow(t.web, true));
          if (t.port.length) t.port.forEach(function (k) { body.appendChild(targetRow(k, true)); });
          else body.appendChild(el('div', 'ind dim pad', 'C++ 移植樹裡找不到 "' + t.tag + '"'));
        });
        body.appendChild(el('div', 'dim pad small', '來自 wire 腳本的標籤對照（\'tag\': [\'' + c.htmlId + '\', …]）。值由 C++ 發布，網頁只負責顯示。'));
      });
    }

    /* where the form's code touches this control */
    if (data.uses || (data.cppPending && c.htmlId && !c.isForm)) {
      var u = data.uses || { port: [], golden: [], portMore: 0, goldenMore: 0 };
      section('uses', '程式碼用到此元件', u.port.length + u.golden.length, function (body) {
        if (!data.uses) { body.appendChild(el('div', 'dim pad', '（搜尋中…）')); return; }
        if (!u.port.length && !u.golden.length) { body.appendChild(el('div', 'dim pad', '表單的程式碼沒有直接用到「' + c.htmlId + '」。')); return; }
        if (u.port.length) body.appendChild(el('div', 'subhead', 'C++ 移植樹' + (u.portMore ? '（另有 ' + u.portMore + ' 處）' : '')));
        u.port.forEach(function (k) { body.appendChild(targetRow(k, false)); });
        if (u.golden.length) body.appendChild(el('div', 'subhead', 'BCB6 原始碼' + (u.goldenMore ? '（另有 ' + u.goldenMore + ' 處）' : '')));
        u.golden.forEach(function (k) { body.appendChild(targetRow(k, false)); });
        body.appendChild(el('div', 'dim pad small', '只搜這個表單自己的檔案（實作或宣告了表單類別的檔），找「' + c.htmlId + '->」和宣告。'));
      });
    }

    /* DFM properties -- WPF's "Arrange by: Name / Category", and the .dfm's own order (by line). With the grid above:
       only what the grid does not show (WPF has one list; the grid's rows reach their .dfm line from the marker) */
    var gridOn = Object.keys(gridLabels).length > 0;
    var dprops = gridOn ? data.props.filter(function (p) { return !inGrid(p); }) : data.props;
    /* (with the grid and one selected they are rows of the grid itself, above) */
    var merged = gridOn && !(ed && ed.multi);
    var dfmSecEl = (gridOn && !dprops.length) || merged ? null : section('props', gridOn ? 'DFM 其他屬性' : 'DFM 屬性', dprops.length, function (body) {
      if (!dprops.length) { body.appendChild(el('div', 'dim pad', '沒有 DFM 屬性資料。')); return; }
      /* (arranged by the "排列" buttons at the top, WPF's "Arrange by") */
      var sort = arrangeBy();
      var onDbl = function (p) {
        if (!p[4]) return false;
        vscode.postMessage({ type: 'openDfmLine', line: p[4] });
        return true;
      };
      if (sort === 'dfm') {
        /* the order BCB6 wrote them in (their .dfm line); one without a line keeps its place at the end */
        kv(body, dprops.map(function (p, i) { return [p, i]; }).sort(function (a, b) {
          return ((a[0][4] || 1e9) - (b[0][4] || 1e9)) || (a[1] - b[1]);
        }).map(function (x) { return x[0]; }), true, onDbl);
      } else if (sort === 'cat') {
        var groups = {};
        dprops.forEach(function (p) { var g = propCat(p[0]); (groups[g] = groups[g] || []).push(p); });
        PROP_CATS.forEach(function (g) {
          if (!groups[g]) return;
          var sh = el('div', 'subhead', g + '（' + groups[g].length + '）');
          sh.setAttribute('data-cat', g);
          catToggle(sh, sh, 'dfm:' + g);
          body.appendChild(sh);
          kv(body, groups[g], true, onDbl).setAttribute('data-catof', 'dfm:' + g);
        });
      } else kv(body, dprops, true, onDbl);
    });
    if (dfmSecEl && dfmSecEl.firstChild) dfmSecEl.firstChild.title = gridOn ?
      '上面表格沒有的 BCB6 .dfm 屬性（唯讀，照上面的「排列」）：雙擊一列＝跳到 .dfm 那一行（找不到行號＝複製值）。上面表格裡的屬性：點那一列右邊的小方塊 →「跳到 .dfm」' :
      'BCB6 .dfm 裡這個元件寫的全部屬性（唯讀，照上面的「排列」）：雙擊一列＝跳到 .dfm 那一行（找不到行號＝複製值）；上面的搜尋框可以篩選';

    /* HTML */
    var htmlSecEl = section('html', 'HTML', null, function (body) {
      var hm = data.html;
      var rows = [];
      if (hm.geom) rows.push(['位置', 'left ' + hm.geom.left + '　top ' + hm.geom.top + '（相對' + (hm.geom.rel === 'form' ? '表單' : '頁面') + '）']);
      if (hm.geom) rows.push(['大小', hm.geom.width + ' × ' + hm.geom.height]);
      if (hm.dfmGeom) rows.push(['DFM 位置', 'left ' + hm.dfmGeom.left + '　top ' + hm.dfmGeom.top + '　' + hm.dfmGeom.width + ' × ' + hm.dfmGeom.height]);
      rows.push(['可見', hm.visible ? '是' : '否（在未顯示的分頁裡，或被隱藏）']);
      if (hm.text) rows.push(['文字', hm.text]);
      kv(body, rows, false);
      if (hm.srcAttrs) {
        /* what the source writes -- a click on a value changes it (WPF edits the XAML) */
        body.appendChild(el('div', 'subhead', '屬性（原始碼）'));
        var ta = el('table', 'kv html');
        hm.srcAttrs.forEach(function (a) {
          editRow(ta, a[0], a[1], !!a[2], function (v) { vscode.postMessage({ type: 'setAttrProp', name: a[0], value: v === '' ? null : v }); },
            a[2] ? '點一下值（或雙擊）：改這個屬性（清空＝刪掉）' : a[0] === 'id' ? 'id 不在這裡改：網頁程式靠它找這個元件' : '這個屬性不在這裡改');
        });
        body.appendChild(ta);
      } else if (hm.attrs.length) { body.appendChild(el('div', 'subhead', '屬性')); kv(body, hm.attrs, false); }
      if (hm.srcStyle) {
        body.appendChild(el('div', 'subhead', 'style（原始碼）'));
        var ts = el('table', 'kv html');
        hm.srcStyle.forEach(function (s) {
          editRow(ts, s[0], s[1], true, function (v) { vscode.postMessage({ type: 'setStyleProp', name: s[0], value: v === '' ? null : v }); },
            '點一下值（或雙擊）：改這個值（清空＝刪掉這一項）');
        });
        /* a new declaration: name + value, Enter */
        var trn = el('tr', 'addrow');
        var kn = el('td', 'k');
        var ni = el('input', 'txt');
        ni.placeholder = '＋ 樣式名稱';
        ni.setAttribute('data-field', 'html:+name');
        kn.appendChild(ni);
        var vn = el('td', 'v');
        var vi = el('input', 'txt');
        vi.placeholder = '值（例：5、1px solid #888）';
        vi.setAttribute('data-field', 'html:+value');
        vn.appendChild(vi);
        var addDecl = function () {
          if (ni.value.trim() && vi.value.trim()) vscode.postMessage({ type: 'setStyleProp', name: ni.value.trim(), value: vi.value.trim() });
        };
        vi.addEventListener('keydown', function (e) { if (e.key === 'Enter') addDecl(); });
        ni.addEventListener('keydown', function (e) { if (e.key === 'Enter') vi.focus(); });
        trn.appendChild(kn); trn.appendChild(vn);
        ts.appendChild(trn);
        body.appendChild(ts);
      } else if (hm.style.length) { body.appendChild(el('div', 'subhead', 'style')); kv(body, hm.style, false); }
      if (hm.computed.length) {
        var more = el('details', 'more');
        more.appendChild(el('summary', null, '計算後樣式'));
        kv(more, hm.computed, false);
        body.appendChild(more);
      }
    });
    if (htmlSecEl && htmlSecEl.firstChild) htmlSecEl.firstChild.title = '這個元素在 HTML 裡的樣子：位置、大小、屬性、style（點一下值＝改，改的是原始碼）、計算後樣式';

    /* page */
    var foot = el('div', 'foot');
    foot.appendChild(el('div', 'fname', data.page.name));
    data.page.notes.forEach(function (n) { foot.appendChild(el('div', 'fnote', n)); });
    root.appendChild(foot);
    var shits = root.querySelector('.shits');
    if (shits) shits.textContent = applySearch();
  }

  /* the search box: every row of every section (a table row, a code location, an event,
     a listener) stays when its text -- or the value in its input -- has the words; a
     section with nothing left folds away, one with a hit opens. Returns the count text. */
  function applySearch() {
    var q = String(state.q || '').trim().toLowerCase();
    var words = q ? q.split(/\s+/) : [];
    var hits = 0;
    searching = true;
    var secs = root.querySelectorAll('details.sec');
    for (var s = 0; s < secs.length; s++) {
      var sec = secs[s];
      var body = sec.querySelector(':scope > .body');
      var title = sec.querySelector(':scope > summary');
      var leaves = body ? body.querySelectorAll('tr, .row, .lhead') : [];
      var shown = 0;
      for (var i = 0; i < leaves.length; i++) {
        var lf = leaves[i];
        var txt = lf.textContent;
        var ins = lf.querySelectorAll('input');
        for (var j = 0; j < ins.length; j++) txt += ' ' + (ins[j].type === 'checkbox' ? (ins[j].checked ? 'true' : 'false') : ins[j].value);
        /* an event row: what its tooltip says counts too (the BCB6 name, the command, the file) */
        if (lf.classList.contains('evg')) txt += ' ' + (lf.title || '');
        txt = txt.toLowerCase();
        var ok = words.every(function (w) { return txt.indexOf(w) >= 0; });
        lf.style.display = ok ? '' : 'none';
        if (ok) shown++;
      }
      /* a listener block (.lst) with nothing left goes too; the notes / sub-headings only without a search */
      var lsts = body ? body.querySelectorAll('.lst') : [];
      for (var k = 0; k < lsts.length; k++) {
        var vis = Array.prototype.some.call(lsts[k].querySelectorAll('.row, .lhead'), function (x) { return x.style.display !== 'none'; });
        lsts[k].style.display = !words.length || vis ? '' : 'none';
      }
      /* a folded sub-list (計算後樣式) with a hit opens */
      var mores = body ? body.querySelectorAll('details.more') : [];
      for (var mo = 0; mo < mores.length; mo++) {
        if (words.length && Array.prototype.some.call(mores[mo].querySelectorAll('tr'), function (x) { return x.style.display !== 'none'; })) mores[mo].open = true;
      }
      var extras = body ? body.querySelectorAll(':scope > .subhead, :scope > .pad, :scope > .dim, :scope > .diffnote') : [];
      for (var x = 0; x < extras.length; x++) extras[x].style.display = words.length ? 'none' : '';
      var titleHit = words.length && words.every(function (w) { return (title ? title.textContent.toLowerCase() : '').indexOf(w) >= 0; });
      if (titleHit) { for (var i2 = 0; i2 < leaves.length; i2++) leaves[i2].style.display = ''; shown = leaves.length; }
      if (!words.length) {
        sec.style.display = '';
        sec.open = !isClosed(sec.getAttribute('data-key'));
      } else {
        sec.style.display = shown ? '' : 'none';
        if (shown) sec.open = true;
      }
      hits += shown;
    }
    searching = false;
    applyCats();
    return words.length ? (hits ? '找到 ' + hits + ' 項' : '沒有符合的項目') : '';
  }

  /* 1006 the Font "…" dialog: name, size (px), bold / italic / underline / strike-out together; 確定 = only what changed,
     as one message (setFontAll -> one batch edit); Esc / 取消 = nothing */
  function openFontDialog(lk, fams) {
    var old = document.getElementById('fontDlg');
    if (old) old.parentNode.removeChild(old);
    var bg = el('div', 'fdlg-bg'); bg.id = 'fontDlg';
    pending(function () { if (bg.parentNode) bg.parentNode.removeChild(bg); });
    var box = el('div', 'fdlg');
    box.appendChild(el('div', 'fdlg-t', '字型'));
    var mk = function (label, input) { var r = el('label', 'fdlg-r'); r.appendChild(el('span', null, label)); r.appendChild(input); box.appendChild(r); return input; };
    var nm = el('input'); nm.type = 'text'; nm.value = lk.fontName || ''; nm.setAttribute('list', 'fontNameList'); nm.setAttribute('data-f', 'fontName');
    mk('字型', nm);
    var sz = el('input'); sz.type = 'number'; sz.min = '4'; sz.max = '200'; sz.value = typeof lk.fontSize === 'number' ? lk.fontSize : ''; sz.setAttribute('data-f', 'fontSize');
    mk('大小（px）', sz);
    var cbs = {};
    [['bold', '粗體'], ['italic', '斜體'], ['underline', '底線'], ['strikeout', '刪除線']].forEach(function (p) {
      if (lk[p[0]] == null && (p[0] === 'underline' || p[0] === 'strikeout')) return;   /* (only where the control has it) */
      var c = el('input'); c.type = 'checkbox'; c.checked = !!lk[p[0]]; c.setAttribute('data-f', p[0]);
      cbs[p[0]] = mk(p[1], c);
    });
    var smp = el('div', 'fdlg-s', 'AaBb 中文 123');
    box.appendChild(smp);
    var paint = function () {
      smp.style.fontFamily = nm.value || ''; smp.style.fontSize = (+sz.value > 0 ? +sz.value : 12) + 'px';
      smp.style.fontWeight = cbs.bold && cbs.bold.checked ? 'bold' : 'normal'; smp.style.fontStyle = cbs.italic && cbs.italic.checked ? 'italic' : 'normal';
      smp.style.textDecoration = [cbs.underline && cbs.underline.checked ? 'underline' : '', cbs.strikeout && cbs.strikeout.checked ? 'line-through' : ''].join(' ').trim() || 'none';
    };
    box.addEventListener('input', paint); box.addEventListener('change', paint);
    var bt = el('div', 'fdlg-b');
    var ok = el('button', 'fdlg-ok', '確定'), no = el('button', null, '取消');
    bt.appendChild(ok); bt.appendChild(no); box.appendChild(bt);
    var close = function () { if (bg.parentNode) bg.parentNode.removeChild(bg); };
    ok.addEventListener('click', function () {
      var v = {};
      if (nm.value.trim() && nm.value.trim() !== (lk.fontName || '')) v.fontName = nm.value.trim();
      if (+sz.value > 0 && +sz.value !== lk.fontSize) v.fontSize = Math.round(+sz.value);
      Object.keys(cbs).forEach(function (k) { if (cbs[k].checked !== !!lk[k]) v[k] = cbs[k].checked; });
      close();
      if (Object.keys(v).length) vscode.postMessage({ type: 'setFontAll', values: v });
    });
    no.addEventListener('click', close);
    bg.addEventListener('keydown', function (e) { if (e.key === 'Escape') { e.preventDefault(); close(); } else if (e.key === 'Enter' && e.target.tagName === 'INPUT' && e.target.type !== 'checkbox') { e.preventDefault(); ok.click(); } });
    bg.addEventListener('mousedown', function (e) { if (e.target === bg) close(); });
    bg.appendChild(box);
    document.body.appendChild(bg);
    paint();
    nm.focus();
    return bg;
  }

  window.addEventListener('message', function (e) {
    var m = e.data;
    if (m && m.type === 'sample') { sample = m.data || null; if (!data) render(); return; }
    /* 1006: a field asked for from outside (the smart tag's 編輯項目 = the Items box): scrolled to and focused */
    if (m && m.type === 'focusField') {
      var ff = root.querySelector('[data-field="' + String(m.field || '').replace(/"/g, '') + '"]');
      if (ff) { if (ff.scrollIntoView) ff.scrollIntoView({ block: 'center' }); ff.focus(); }
      return;
    }
    if (m && m.type === 'show') {
      // keep the focused field (typing Left, Tab to Top …) across the re-render an edit causes
      var a = document.activeElement;
      var field = a && a.getAttribute ? a.getAttribute('data-field') : null;
      var sameComp = data && m.data && data.comp && m.data.comp && data.comp.key === m.data.comp.key;
      if (!sameComp) settle();
      data = m.data;
      render();
      if (field && sameComp) {
        var again = root.querySelector('[data-field="' + field + '"]');
        if (again) { again.focus(); if (again.select) try { again.select(); } catch (x) { /* ignore */ } }
      }
    }
  });
  render();
  vscode.postMessage({ type: 'ready' });
})();
