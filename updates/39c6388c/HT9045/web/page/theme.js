/* HT9xxx Manual 佈景主題套用器
   - localStorage('ht9xxx-theme') 記憶選擇（file:// 同源共享）
   - 接收 postMessage {type:'HT_THEME', theme} 讓 background.html 廣播給所有 iframe
   - window.HTTheme.set(name) / .get() / .list 供程式切換 */
(function () {
  var KEY = 'ht9xxx-theme';
  var THEMES = ['classic', 'dark', 'steel', 'contrast'];

  // ---- 版本模式（release / debug）：?mode= 決定，設 <html data-mode>；供各頁 CSS 隱藏 debug 專用元件 ----
  var mm = /[?&]mode=(release|debug)/.exec(location.search);
  var MODE = mm ? mm[1] : 'release';                 // 無參數預設 release
  document.documentElement.setAttribute('data-mode', MODE);
  if (MODE === 'release') {
    // release：把 title 移到 data-htitle（去掉 hover 露出的 cpp/dfm 名，但保留值供程式判斷）
    var stripTitles = function () {
      var els = document.querySelectorAll('[title]');
      for (var i = 0; i < els.length; i++) {
        var el = els[i];
        el.setAttribute('data-htitle', el.getAttribute('title'));
        el.removeAttribute('title');
      }
    };
    if (document.readyState === 'loading') document.addEventListener('DOMContentLoaded', stripTitles);
    else stripTitles();

    // release：封鎖滑鼠右鍵與瀏覽器快速鍵（每個 iframe 各自掛；Ctrl+W/Alt+F4/Win 鍵瀏覽器不放行，交給 Edge --kiosk）
    document.addEventListener('contextmenu', function (e) { e.preventDefault(); }, true);
    document.addEventListener('keydown', function (e) {
      var k = e.key || '';
      var fn = /^F\d{1,2}$/.test(k);                                   // F1~F12（F5 重整、F11 全螢幕、F12 DevTools）
      var combo = (e.ctrlKey || e.altKey || e.metaKey) &&
        !(e.ctrlKey && !e.altKey && /^[cvxaz]$/i.test(k) &&           // 編輯框內保留 Ctrl+C/V/X/A/Z
          /^(INPUT|TEXTAREA)$/.test((e.target && e.target.tagName) || ''));
      if (fn || combo || k === 'Escape') { e.preventDefault(); e.stopPropagation(); }
    }, true);
    // 中鍵/右鍵 auxclick、拖曳檔案、雙指縮放
    document.addEventListener('auxclick', function (e) { e.preventDefault(); }, true);
    document.addEventListener('dragover', function (e) { e.preventDefault(); }, true);
    document.addEventListener('drop', function (e) { e.preventDefault(); }, true);
    document.addEventListener('wheel', function (e) { if (e.ctrlKey) e.preventDefault(); }, { passive: false, capture: true });
  }

  function apply(name) {
    if (THEMES.indexOf(name) < 0) name = 'classic';
    if (name === 'classic') document.documentElement.removeAttribute('data-theme');
    else document.documentElement.setAttribute('data-theme', name);
    try { localStorage.setItem(KEY, name); } catch (e) { /* file:// 可能禁用 */ }
    return name;
  }

  var saved = 'classic';
  try { saved = localStorage.getItem(KEY) || 'classic'; } catch (e) {}
  // file:// 各頁 localStorage 可能不同源，URL ?theme= 優先（例：Setup.Speed.html?theme=dark）
  var m = /[?&]theme=(\w+)/.exec(location.search);
  if (m) saved = m[1];
  apply(saved);

  window.addEventListener('message', function (ev) {
    if (ev.data && ev.data.type === 'HT_THEME') apply(ev.data.theme);
    if (ev.data && ev.data.type === 'HT_FONT') applyFont(ev.data.font);
    if (ev.data && ev.data.type === 'HT_FSX') applyFsx(ev.data.fsx);
  });

  /* ---- 字體規範：--font-ui 動態覆寫（StyleGuide 設定畫面） ---- */
  var FKEY = 'ht9xxx-font-ui';
  function applyFont(fam) {
    if (!fam || fam === 'default') {
      document.documentElement.style.removeProperty('--font-ui');
      try { localStorage.removeItem(FKEY); } catch (e) {}
      return 'default';
    }
    document.documentElement.style.setProperty('--font-ui', fam);
    try { localStorage.setItem(FKEY, fam); } catch (e) {}
    return fam;
  }
  var savedFont = null;
  try { savedFont = localStorage.getItem(FKEY); } catch (e) {}
  if (savedFont) applyFont(savedFont);

  window.HTFont = {
    get: function () { return document.documentElement.style.getPropertyValue('--font-ui') || 'default'; },
    set: applyFont
  };

  /* ---- 字級縮放 --fsx（StyleGuide 字級規範）：ht9xxx.css 共用類別 font-size 乘上此係數 ---- */
  var SKEY = 'ht9xxx-fsx';
  function applyFsx(v) {
    v = parseFloat(v) || 1;
    if (v === 1) {
      document.documentElement.style.removeProperty('--fsx');
      try { localStorage.removeItem(SKEY); } catch (e) {}
    } else {
      document.documentElement.style.setProperty('--fsx', v);
      try { localStorage.setItem(SKEY, String(v)); } catch (e) {}
    }
    return v;
  }
  var savedFsx = null;
  try { savedFsx = localStorage.getItem(SKEY); } catch (e) {}
  if (savedFsx) applyFsx(savedFsx);

  window.HTFsx = {
    get: function () { return parseFloat(document.documentElement.style.getPropertyValue('--fsx')) || 1; },
    set: applyFsx
  };

  window.HTTheme = {
    list: THEMES.slice(),
    get: function () { return document.documentElement.getAttribute('data-theme') || 'classic'; },
    set: apply
  };
})();

/* ===== 版面拖曳模式（Debug ▾ → 🧲 版面拖曳模式）=====
   - HT_LAYOUT_EDIT{on}：進出編輯模式；編輯中元件可拖（transform 偏移）、原 click 被攔截
   - 偏移存 localStorage('ht9xxx-layout')＝{頁面key:{選擇器:[dx,dy]}}，載入頁面時自動套用
   - HT_LAYOUT_SAVE：拖曳即時已存，僅離開模式；HT_LAYOUT_CANCEL：還原進入模式時的快照 */
(function () {
  var KEY = 'ht9xxx-layout';
  var pm = location.pathname.match(/page\/.*$/);
  var PAGE = pm ? pm[0] : location.pathname;
  // 基底版面（ht9xxx-layout-base.js，匯出 JSON 固化）；localStorage 同頁鍵優先
  function baseOf(k) { var b = window.HTLAYOUT_BASE || {}; return b[k] || {}; }
  function loadAll() { try { return Object.assign({}, baseOf('offsets'), JSON.parse(localStorage.getItem(KEY) || '{}')); } catch (e) { return Object.assign({}, baseOf('offsets')); } }
  function saveAll(m) { try { localStorage.setItem(KEY, JSON.stringify(m)); } catch (e) {} }
  function selOf(el) {
    if (el.id) return '#' + el.id;
    var p = [];
    while (el && el !== document.body) {
      var par = el.parentNode; if (!par) break;
      p.unshift(el.tagName + ':nth-child(' + (Array.prototype.indexOf.call(par.children, el) + 1) + ')');
      el = par;
    }
    return 'body>' + p.join('>');
  }
  function q(s) { try { return s.charAt(0) === '#' ? document.getElementById(s.slice(1)) : document.querySelector(s); } catch (e) { return null; } }
  function setTf(el, d) { el.style.transform = (d[0] || d[1]) ? 'translate(' + d[0] + 'px,' + d[1] + 'px)' : ''; }
  function applyLayout() {
    var m = loadAll()[PAGE] || {};
    Object.keys(m).forEach(function (s) { var el = q(s); if (el) setTf(el, m[s]); });
  }

  /* ---- 自訂新增元件（拖曳模式 ➕ 元件盤拖入；右鍵刪除） ---- */
  var KEY2 = 'ht9xxx-layout-add';
  function loadAdds() { try { return Object.assign({}, baseOf('adds'), JSON.parse(localStorage.getItem(KEY2) || '{}')); } catch (e) { return Object.assign({}, baseOf('adds')); } }
  function saveAdds(m) { try { localStorage.setItem(KEY2, JSON.stringify(m)); } catch (e) {} }
  function makeComp(rec) {
    var el = null, W = window.HTWidgets;
    try {
      switch (rec.type) {
        case 'button': el = document.createElement('button'); el.textContent = rec.text || rec.name; break;
        case 'edit': el = document.createElement('input'); el.type = 'text'; el.value = rec.text || ''; el.style.width = '90px'; break;
        case 'checkbox':
          el = document.createElement('label');
          el.appendChild(document.createElement('input')).type = 'checkbox';
          el.appendChild(document.createTextNode(' ' + (rec.text || rec.name)));
          el.style.font = '12px var(--font-ui,sans-serif)'; break;
        case 'select':
          el = document.createElement('select');
          (rec.text || rec.name).split('|').forEach(function (t) {
            var o = document.createElement('option'); o.textContent = t; el.appendChild(o);
          });
          break;
        case 'panel':
          el = document.createElement('div');
          el.style.cssText = 'width:160px;height:90px;border:1px solid var(--gbx-border,#999);border-radius:3px;';
          var cap = document.createElement('div'); cap.textContent = rec.text || rec.name;
          cap.style.cssText = 'font:11px var(--font-ui,sans-serif);padding:1px 6px;color:var(--navy,#036);';
          el.appendChild(cap); break;
        case 'led':
          if (W && W.makeALed) el = W.makeALed({ name: rec.name, value: true });
          break;
        case 'labeledled':
          if (W && W.makeMyLabeledLed) el = W.makeMyLabeledLed({ name: rec.name, caption: rec.text || rec.name, value: true, captionPos: rec.pos || 'lpRight' });
          break;
        case 'btnpanel':
          if (W && W.makeBtnPanel) el = W.makeBtnPanel({ name: rec.name, caption: rec.text || rec.name, width: 80, height: 24 });
          break;
        case 'speedbtn':
          if (W && W.makeSpeedButton) el = W.makeSpeedButton({ name: rec.name, caption: rec.text || rec.name, w: 80, h: 26, img: rec.img || null });
          break;
        case 'tray':
          if (W && W.makeMyTray) el = W.makeMyTray({ name: rec.name, xitem: rec.cols || 7, yitem: rec.rows || 13, cellW: 9, cellH: 6,
            colorMap: ['#fff', '#2e8b57'], cells: [], showFont: false, trayDirect: 'csLeftTop' });
          break;
      }
    } catch (e) { el = null; }
    if (!el) {   // label 預設／無 HTWidgets 時的退回外觀
      el = document.createElement('div');
      if (rec.type === 'led') el.style.cssText = 'width:15px;height:15px;border-radius:50%;background:#0c0;border:1px solid #666;';
      else if (rec.type === 'labeledled') { el.innerHTML = '<span style="display:inline-block;width:15px;height:15px;border-radius:50%;background:#0c0;border:1px solid #666;vertical-align:middle;"></span> ' + (rec.text || rec.name); el.style.font = '11px var(--font-ui,sans-serif)'; }
      else if (rec.type === 'tray') el.style.cssText = 'width:63px;height:78px;background:#ddd;border:2px solid #555;';
      else if (rec.type === 'btnpanel') { el.textContent = rec.text || rec.name; el.style.cssText = 'width:80px;height:22px;background:#8f8;border:1px solid #666;text-align:center;font:12px var(--font-ui,sans-serif);'; }
      else if (rec.type === 'speedbtn') { el.textContent = rec.text || rec.name; el.style.cssText = 'width:80px;height:26px;border:1px outset #ccc;background:#ece9d8;text-align:center;font:11px var(--font-ui,sans-serif);'; }
      else { el.textContent = rec.text || rec.name; el.style.font = '12px var(--font-ui,sans-serif)'; }
    }
    el.setAttribute('data-htadd', rec.name);
    if (!document.getElementById(rec.name)) el.id = rec.name;
    el.title = rec.name + '（自訂元件:' + rec.type + '）';
    el.style.position = 'absolute'; el.style.left = rec.x + 'px'; el.style.top = rec.y + 'px'; el.style.zIndex = 500;
    document.body.appendChild(el);
    return el;
  }
  function applyAdds() {
    document.querySelectorAll('[data-htadd]').forEach(function (n) { n.remove(); });
    (loadAdds()[PAGE] || []).forEach(makeComp);
  }

  /* ---- 屬性盤持久化：{頁面key:{選擇器:{text,fs,color,bg,w,h,hide}}} ---- */
  var KEY3 = 'ht9xxx-layout-prop';
  function loadProps() { try { return Object.assign({}, baseOf('props'), JSON.parse(localStorage.getItem(KEY3) || '{}')); } catch (e) { return Object.assign({}, baseOf('props')); } }
  function saveProps(m) { try { localStorage.setItem(KEY3, JSON.stringify(m)); } catch (e) {} }
  /* ---- 群組持久化：{頁面key:[[選擇器,...],...]}；同群組拖曳連動 ---- */
  var KEY4 = 'ht9xxx-layout-group';
  function loadGroups() { try { return Object.assign({}, baseOf('groups'), JSON.parse(localStorage.getItem(KEY4) || '{}')); } catch (e) { return Object.assign({}, baseOf('groups')); } }
  function saveGroups(m) { try { localStorage.setItem(KEY4, JSON.stringify(m)); } catch (e) {} }
  function groupOf(sel) {
    var gs = loadGroups()[PAGE] || [];
    for (var i = 0; i < gs.length; i++) if (gs[i].indexOf(sel) >= 0) return gs[i];
    return null;
  }
  var ORIG = {};   // 選擇器 → 修改前原始 {style,text}，供欄位清空／放棄還原
  function getCompText(el) {
    if (el.tagName === 'INPUT') return el.value;
    var w = document.createTreeWalker(el, NodeFilter.SHOW_TEXT, null), n, last = null;
    while ((n = w.nextNode())) if (n.nodeValue.trim()) last = n;
    return last ? last.nodeValue.trim() : '';
  }
  function setCompText(el, t) {   // 取最後一個非空文字節點替換，兼容 checkbox/lledf 等複合元件
    if (el.tagName === 'INPUT') { el.value = t; return; }
    var w = document.createTreeWalker(el, NodeFilter.SHOW_TEXT, null), n, last = null;
    while ((n = w.nextNode())) if (n.nodeValue.trim()) last = n;
    if (last) last.nodeValue = t; else el.textContent = t;
  }
  function applyProp(el, p, s) {
    if (!el) return;
    if (!ORIG[s]) ORIG[s] = { style: el.getAttribute('style') || '', text: getCompText(el), tt: 0 };
    el.setAttribute('style', ORIG[s].style);   // 先還原再套用，欄位清空即回復原狀
    var off = (loadAll()[PAGE] || {})[s]; if (off) setTf(el, off);
    p = p || {};
    if (p.fs) el.style.fontSize = p.fs + 'px';
    if (p.color) el.style.color = p.color;
    if (p.bg) el.style.background = p.bg;
    if (p.w) el.style.width = p.w + 'px';
    if (p.h) el.style.height = p.h + 'px';
    if (p.hide) { if (editing) el.style.opacity = .3; else el.style.visibility = 'hidden'; }
    if (p.text != null) { setCompText(el, p.text); ORIG[s].tt = 1; }
    else if (ORIG[s].tt) { setCompText(el, ORIG[s].text); ORIG[s].tt = 0; }
  }
  function applyProps() {
    var m = loadProps()[PAGE] || {};
    Object.keys(m).forEach(function (s) { applyProp(q(s), m[s], s); });
  }
  // 先載基底版面再套用（file:// 不能 fetch → script 標籤；缺檔或已由頁面引入時照常套用）
  if (window.HTLAYOUT_BASE) { applyAdds(); applyLayout(); applyProps(); }
  else (function () {
    var s = document.createElement('script');
    s.src = (location.pathname.indexOf('/shot/') >= 0 ? '../' : '') + 'ht9xxx-layout-base.js';
    s.onload = s.onerror = function () { applyAdds(); applyLayout(); applyProps(); };
    document.head.appendChild(s);
  })();

  var editing = false, snap = null, snap2 = null, snap3 = null, snap4 = null, moved = {}, cur = null, curSel = '', sx = 0, sy = 0, ox = 0, oy = 0, hov = null;
  var grpDrag = null;   // 拖曳中的群組成員 [{s,el,o:[x,y]}]
  var msel = [];        // Ctrl+點選的多選選擇器（群組用）
  function mselToggle(el) {
    var s = selOf(el), i = msel.indexOf(s);
    if (i >= 0) { msel.splice(i, 1); el.classList.remove('hlMulti'); }
    else { msel.push(s); el.classList.add('hlMulti'); }
    if (panel && selEl && panel.style.display !== 'none') updateGrpInfo();
  }
  function mselClear() {
    msel.forEach(function (s) { var el = q(s); if (el) el.classList.remove('hlMulti'); });
    msel = [];
  }
  var placeType = null;   // 點擊放置模式：待放置的元件型別（元件盤點選後廣播）
  function placeComp(type, x, y) {
    var all = loadAdds(), list = all[PAGE] || (all[PAGE] = []);
    var name = prompt('元件名稱（將作為 id／建議照 dfm 命名）', type + (list.length + 1));
    if (!name) return false;
    name = name.replace(/\s+/g, '_');
    while (document.getElementById(name)) name += '_2';
    var text = null;
    if (/^(label|button|checkbox|panel|btnpanel|select|labeledled|speedbtn)$/.test(type))
      text = prompt('顯示文字（select 可用 | 分隔多個選項）', name) || name;
    var rec = { name: name, type: type, x: x, y: y, text: text };
    list.push(rec); saveAdds(all);
    return makeComp(rec);
  }

  /* ---- 屬性盤 UI：編輯模式點選元件後顯示 ---- */
  var panel = null, selEl = null, selStr = '';
  function P(s) { return panel.querySelector(s); }
  function addRec(el) {
    var ad = el.getAttribute('data-htadd');
    if (!ad) return null;
    var list = loadAdds()[PAGE] || [];
    for (var i = 0; i < list.length; i++) if (list[i].name === ad) return list[i];
    return null;
  }
  function compType(el) {
    var r = addRec(el);
    if (r) return r.type;
    var t = el.title || '';
    if (/TMyLabeledLed|TLabeledALed/.test(t)) return 'labeledled';
    if (/TMyLedLane|TMyLed\b|TALed|TLed\b/.test(t)) return 'led';
    if (/TT?MyTray/.test(t)) return 'tray';
    if (/TSpeedButton|TBitBtn/.test(t)) return 'speedbtn';
    if (/TGroupBox/.test(t)) return 'groupbox';
    if (/TPanel/.test(t)) return 'panel';
    return '';
  }
  /* 自訂元件改定義後重建（LabeledLed 文字位置、Tray 行列數），保留偏移與屬性 */
  function rebuildAdd(el, mut) {
    var name = el.getAttribute('data-htadd'); if (!name) return;
    var all = loadAdds(), list = all[PAGE] || [], rec = null;
    for (var i = 0; i < list.length; i++) if (list[i].name === name) { rec = list[i]; break; }
    if (!rec) return;
    mut(rec); saveAdds(all);
    el.remove();
    var ne = makeComp(rec), sk = '#' + name;
    var off = (loadAll()[PAGE] || {})[sk]; if (off) setTf(ne, off);
    delete ORIG[sk];
    applyProp(ne, (loadProps()[PAGE] || {})[sk], sk);
    if (off) setTf(ne, off);
    showPanel(ne);
  }
  function toHex(c) {
    var m = (c || '').match(/\d+/g); if (!m) return '#000000';
    return '#' + m.slice(0, 3).map(function (v) { return ('0' + (+v).toString(16)).slice(-2); }).join('');
  }
  function setField(k, v) {
    if (!selEl) return;
    var all = loadProps(), m = all[PAGE] || (all[PAGE] = {}), p = m[selStr] || (m[selStr] = {});
    if (v === '' || v == null || v === false || (typeof v === 'number' && isNaN(v))) delete p[k]; else p[k] = v;
    if (!Object.keys(p).length) delete m[selStr];
    if (!Object.keys(m).length) delete all[PAGE];
    saveProps(all);
    applyProp(selEl, p, selStr);
  }
  function buildPanel() {
    if (panel) return;
    panel = document.createElement('div'); panel.id = 'htPropPanel';
    panel.innerHTML =
      '<div class="ppTitle">🛠 屬性盤　<span class="ppTgt"></span></div>' +
      '<div class="ppRow"><label>名稱</label><input class="ppName" type="text"></div>' +
      '<div class="ppRow ppRowText"><label>文字</label><input class="ppText" type="text"></div>' +
      '<div class="ppRow ppRowPos"><label>文字位</label><select class="ppPos"><option value="lpLeft">左</option><option value="lpRight">右</option><option value="lpTop">上</option><option value="lpBottom">下</option></select></div>' +
      '<div class="ppRow ppRowTray"><label>格數</label><input class="ppTRow" type="number" min="1" max="60" title="Row 數（YItem）"><span>行 ×</span><input class="ppTCol" type="number" min="1" max="60" title="Col 數（XItem）"><span>列</span></div>' +
      '<div class="ppRow"><label>字級</label><input class="ppFs" type="number" min="6" max="96">' +
      '<label style="min-width:auto;margin-left:8px;"><input class="ppHide" type="checkbox"> 隱藏</label></div>' +
      '<div class="ppRow"><label>文字色</label><input class="ppColor" type="color"><button class="ppClrC" title="清除文字色">✕</button>' +
      '<label>底色</label><input class="ppBg" type="color"><button class="ppClrB" title="清除底色">✕</button></div>' +
      '<div class="ppRow"><label>寬</label><input class="ppW" type="number"><label>高</label><input class="ppH" type="number"></div>' +
      '<div class="ppRow"><label>偏移X</label><input class="ppX" type="number"><label>Y</label><input class="ppY" type="number"></div>' +
      '<div class="ppRow ppRowAlign"><label>對齊</label><button class="ppAlL" title="多選（綠框）靠齊基準（藍框）左緣">⬅左</button><button class="ppAlR" title="靠齊基準右緣">右➡</button><button class="ppAlT" title="靠齊基準上緣">⬆上</button><button class="ppAlB" title="靠齊基準下緣">下⬇</button><button class="ppAlCH" title="水平置中對齊基準">╋X</button><button class="ppAlCV" title="垂直置中對齊基準">╋Y</button></div>' +
      '<div class="ppRow ppRowDist"><label>均分</label><button class="ppDsH" title="多選（含基準）≥3 個：首尾固定，水平間距平均分配">↔水平均分</button><button class="ppDsV" title="多選（含基準）≥3 個：首尾固定，垂直間距平均分配">↕垂直均分</button></div>' +
      '<div class="ppRow ppRowSnap"><label>貼容器</label><button class="ppSnL" title="貼齊所屬 GroupBox/Panel 左內緣">⇤左</button><button class="ppSnR" title="貼齊右內緣">右⇥</button><button class="ppSnT" title="貼齊上內緣">⇡上</button><button class="ppSnB" title="貼齊下內緣">下⇣</button><span style="margin-left:4px;">內縮</span><input class="ppSnIn" type="number" value="2" min="0" max="99" style="width:40px;" title="貼齊後與容器內緣的間距 px"><span>px</span></div>' +
      '<div class="ppRow"><label>群組</label><span class="ppGrpInfo" style="flex:1;color:#9fd;"></span><button class="ppGrp" title="Ctrl+點選多個元件（綠框）後按此建立群組，拖曳連動">群組</button><button class="ppUngrp">解群組</button></div>' +
      '<div class="ppRow ppBtns"><button class="ppReset">還原屬性</button><button class="ppDel">刪除</button><button class="ppClose">關閉</button></div>';
    document.body.appendChild(panel);
    P('.ppName').addEventListener('change', function () { if (selEl) renameAdd(selEl, this.value); });
    P('.ppText').addEventListener('input', function () { setField('text', this.value); });
    P('.ppPos').addEventListener('change', function () {
      var v = this.value;
      if (selEl && selEl.hasAttribute('data-htadd')) rebuildAdd(selEl, function (r) { r.pos = v; });
    });
    P('.ppTRow').addEventListener('change', function () {
      var v = Math.max(1, +this.value || 13);
      if (selEl && selEl.hasAttribute('data-htadd')) rebuildAdd(selEl, function (r) { r.rows = v; });
    });
    P('.ppTCol').addEventListener('change', function () {
      var v = Math.max(1, +this.value || 7);
      if (selEl && selEl.hasAttribute('data-htadd')) rebuildAdd(selEl, function (r) { r.cols = v; });
    });
    P('.ppFs').addEventListener('input', function () { setField('fs', this.value === '' ? '' : +this.value); });
    P('.ppColor').addEventListener('input', function () { setField('color', this.value); });
    P('.ppClrC').addEventListener('click', function () { setField('color', ''); if (selEl) showPanel(selEl); });
    P('.ppBg').addEventListener('input', function () { setField('bg', this.value); });
    P('.ppClrB').addEventListener('click', function () { setField('bg', ''); if (selEl) showPanel(selEl); });
    P('.ppW').addEventListener('input', function () { setField('w', this.value === '' ? '' : +this.value); posRsz(); });
    P('.ppH').addEventListener('input', function () { setField('h', this.value === '' ? '' : +this.value); posRsz(); });
    P('.ppHide').addEventListener('change', function () { setField('hide', this.checked ? 1 : false); });
    function setOff() {
      if (!selEl) return;
      var x = +P('.ppX').value || 0, y = +P('.ppY').value || 0;
      var all = loadAll(), m = all[PAGE] || (all[PAGE] = {});
      if (!x && !y) delete m[selStr]; else m[selStr] = [x, y];
      if (!Object.keys(m).length) delete all[PAGE];
      saveAll(all); moved[selStr] = 1; setTf(selEl, [x, y]);
    }
    P('.ppX').addEventListener('input', setOff);
    P('.ppY').addEventListener('input', setOff);
    /* 靠邊對齊：多選（綠框 msel）對齊基準（藍框 selEl），以偏移寫回 offsets 持久化 */
    function alignSel(mode) {
      if (!selEl) return;
      var list = msel.filter(function (s) { return s !== selStr; });
      if (!list.length) { alert('先按住 Ctrl 點選要對齊的元件（綠框），再按對齊鈕（以藍框選取元件為基準）'); return; }
      var ref = selEl.getBoundingClientRect();
      var all = loadAll(), m = all[PAGE] || (all[PAGE] = {});
      list.forEach(function (s) {
        var el = q(s); if (!el) return;
        var r = el.getBoundingClientRect(), dx = 0, dy = 0;
        if (mode === 'l') dx = ref.left - r.left;
        else if (mode === 'r') dx = ref.right - r.right;
        else if (mode === 't') dy = ref.top - r.top;
        else if (mode === 'b') dy = ref.bottom - r.bottom;
        else if (mode === 'ch') dx = (ref.left + ref.width / 2) - (r.left + r.width / 2);
        else if (mode === 'cv') dy = (ref.top + ref.height / 2) - (r.top + r.height / 2);
        dx = Math.round(dx); dy = Math.round(dy);
        if (!dx && !dy) return;
        var o = m[s] || [0, 0], no = [o[0] + dx, o[1] + dy];
        if (!no[0] && !no[1]) delete m[s]; else m[s] = no;
        setTf(el, no); moved[s] = 1;
      });
      if (!Object.keys(m).length) delete all[PAGE];
      saveAll(all);
    }
    P('.ppAlL').addEventListener('click', function () { alignSel('l'); });
    P('.ppAlR').addEventListener('click', function () { alignSel('r'); });
    P('.ppAlT').addEventListener('click', function () { alignSel('t'); });
    P('.ppAlB').addEventListener('click', function () { alignSel('b'); });
    P('.ppAlCH').addEventListener('click', function () { alignSel('ch'); });
    P('.ppAlCV').addEventListener('click', function () { alignSel('cv'); });
    /* 均分：多選（含基準）依位置排序，首尾固定，中間元件間距平均分配 */
    function distSel(axis) {
      var keys = msel.slice();
      if (selStr && keys.indexOf(selStr) < 0) keys.push(selStr);
      var items = [];
      keys.forEach(function (s) {
        var el = q(s); if (!el) return;
        items.push({ s: s, el: el, r: el.getBoundingClientRect() });
      });
      if (items.length < 3) { alert('均分需至少 3 個元件：按住 Ctrl 點選多個元件（綠框）後再按均分鈕'); return; }
      var h = (axis === 'h');
      items.sort(function (a, b) { return h ? a.r.left - b.r.left : a.r.top - b.r.top; });
      var first = items[0].r, last = items[items.length - 1].r;
      var span = h ? (last.right - first.left) : (last.bottom - first.top);
      var sum = 0;
      items.forEach(function (it) { sum += h ? it.r.width : it.r.height; });
      var gap = (span - sum) / (items.length - 1);
      var all = loadAll(), m = all[PAGE] || (all[PAGE] = {});
      var cur = (h ? first.right : first.bottom) + gap;
      for (var i = 1; i < items.length - 1; i++) {
        var it = items[i];
        var d = Math.round(cur - (h ? it.r.left : it.r.top));
        cur += (h ? it.r.width : it.r.height) + gap;
        if (!d) continue;
        var o = m[it.s] || [0, 0], no = h ? [o[0] + d, o[1]] : [o[0], o[1] + d];
        if (!no[0] && !no[1]) delete m[it.s]; else m[it.s] = no;
        setTf(it.el, no); moved[it.s] = 1;
      }
      if (!Object.keys(m).length) delete all[PAGE];
      saveAll(all);
    }
    P('.ppDsH').addEventListener('click', function () { distSel('h'); });
    P('.ppDsV').addEventListener('click', function () { distSel('v'); });
    /* 貼容器：選取（含多選）元件貼齊各自所屬 GroupBox/Panel 內緣，可設內縮 px，寫回 offsets */
    function containerOf(el) {
      var p = el.parentElement;
      while (p && p !== document.body) {
        if (p.tagName === 'FIELDSET') return p;
        var t = (p.getAttribute && p.getAttribute('title')) || '';
        if (/TPanel|TGroupBox|TScrollBox|TTabSheet/.test(t)) return p;
        if (p.classList && p.classList.contains('form')) return p;
        p = p.parentElement;
      }
      return document.querySelector('.form') || document.body;
    }
    function snapSel(mode) {
      if (!selEl) return;
      var inset = Math.max(0, +P('.ppSnIn').value || 0);
      var keys = msel.slice();
      if (keys.indexOf(selStr) < 0) keys.push(selStr);
      var all = loadAll(), m = all[PAGE] || (all[PAGE] = {});
      keys.forEach(function (s) {
        var el = q(s); if (!el) return;
        var box = containerOf(el); if (!box) return;
        var cr = box.getBoundingClientRect(), r = el.getBoundingClientRect(), dx = 0, dy = 0;
        if (mode === 'l') dx = (cr.left + inset) - r.left;
        else if (mode === 'r') dx = (cr.right - inset) - r.right;
        else if (mode === 't') dy = (cr.top + inset) - r.top;
        else if (mode === 'b') dy = (cr.bottom - inset) - r.bottom;
        dx = Math.round(dx); dy = Math.round(dy);
        if (!dx && !dy) return;
        var o = m[s] || [0, 0], no = [o[0] + dx, o[1] + dy];
        if (!no[0] && !no[1]) delete m[s]; else m[s] = no;
        setTf(el, no); moved[s] = 1;
      });
      if (!Object.keys(m).length) delete all[PAGE];
      saveAll(all);
      if (selEl) showPanel(selEl);
    }
    P('.ppSnL').addEventListener('click', function () { snapSel('l'); });
    P('.ppSnR').addEventListener('click', function () { snapSel('r'); });
    P('.ppSnT').addEventListener('click', function () { snapSel('t'); });
    P('.ppSnB').addEventListener('click', function () { snapSel('b'); });
    P('.ppReset').addEventListener('click', function () {
      if (!selEl) return;
      var all = loadProps(); if (all[PAGE]) { delete all[PAGE][selStr]; if (!Object.keys(all[PAGE]).length) delete all[PAGE]; }
      saveProps(all); applyProp(selEl, {}, selStr); showPanel(selEl);
    });
    P('.ppDel').addEventListener('click', function () { if (selEl) delAdd(selEl); });
    P('.ppClose').addEventListener('click', hidePanel);
    P('.ppGrp').addEventListener('click', function () {
      if (!selEl) return;
      var list = msel.slice();
      if (list.indexOf(selStr) < 0) list.push(selStr);
      if (list.length < 2) { alert('先按住 Ctrl 點選要一起群組的元件（綠框），再按「群組」'); return; }
      var all = loadGroups(), gs = all[PAGE] || (all[PAGE] = []);
      for (var i = gs.length - 1; i >= 0; i--) {   // 成員脫離原群組，避免重複歸屬
        gs[i] = gs[i].filter(function (s) { return list.indexOf(s) < 0; });
        if (gs[i].length < 2) gs.splice(i, 1);
      }
      gs.push(list);
      if (!gs.length) delete all[PAGE];
      saveGroups(all);
      mselClear(); showPanel(selEl);
    });
    P('.ppUngrp').addEventListener('click', function () {
      if (!selEl) return;
      var all = loadGroups(), gs = all[PAGE] || [];
      for (var i = gs.length - 1; i >= 0; i--) if (gs[i].indexOf(selStr) >= 0) gs.splice(i, 1);
      if (!gs.length) delete all[PAGE];
      saveGroups(all); showPanel(selEl);
    });
    // 抓標題列拖拉換位
    var pdrag = null;
    P('.ppTitle').addEventListener('mousedown', function (e) {
      var r = panel.getBoundingClientRect();
      pdrag = [e.clientX - r.left, e.clientY - r.top];
      e.preventDefault(); e.stopPropagation();
    });
    document.addEventListener('mousemove', function (e) {
      if (!pdrag) return;
      panel.style.left = (e.clientX - pdrag[0]) + 'px';
      panel.style.top = (e.clientY - pdrag[1]) + 'px';
      panel.style.right = 'auto'; panel.style.bottom = 'auto';
    }, true);
    document.addEventListener('mouseup', function () { pdrag = null; }, true);
  }
  /* 自訂元件改名：同步遷移定義、偏移、屬性、ORIG 快照鍵；dfm 元件唯讀（重生成後 id 固定） */
  function renameAdd(el, nn) {
    var old = el.getAttribute('data-htadd');
    if (!old) return;
    nn = (nn || '').replace(/\s+/g, '_');
    if (!nn || nn === old) { showPanel(el); return; }
    if (document.getElementById(nn)) { alert('id 已存在：' + nn); showPanel(el); return; }
    var all = loadAdds(), list = all[PAGE] || [];
    for (var i = 0; i < list.length; i++) if (list[i].name === old) { list[i].name = nn; break; }
    saveAdds(all);
    el.setAttribute('data-htadd', nn); el.id = nn;
    if (el.title) el.title = el.title.split(old).join(nn);
    var ok = '#' + old, nk = '#' + nn;
    var lay = loadAll(); if (lay[PAGE] && lay[PAGE][ok]) { lay[PAGE][nk] = lay[PAGE][ok]; delete lay[PAGE][ok]; saveAll(lay); }
    var pr = loadProps(); if (pr[PAGE] && pr[PAGE][ok]) { pr[PAGE][nk] = pr[PAGE][ok]; delete pr[PAGE][ok]; saveProps(pr); }
    var gr = loadGroups(); if (gr[PAGE]) { gr[PAGE].forEach(function (g) { var i = g.indexOf(ok); if (i >= 0) g[i] = nk; }); saveGroups(gr); }
    if (ORIG[ok]) { ORIG[nk] = ORIG[ok]; delete ORIG[ok]; }
    if (moved[ok]) { moved[nk] = 1; delete moved[ok]; }
    selStr = nk;
    showPanel(el);
  }
  function updateGrpInfo() {
    var g = groupOf(selStr);
    P('.ppGrpInfo').textContent = g ? '×' + g.length + ' 成員' : (msel.length ? '已多選 ' + msel.length : '—');
    P('.ppUngrp').style.display = g ? '' : 'none';
    document.querySelectorAll('.hlGrpM').forEach(function (n) { n.classList.remove('hlGrpM'); });
    if (g) g.forEach(function (s) { var m = q(s); if (m && m !== selEl) m.classList.add('hlGrpM'); });
  }
  /* ---- 拖拉縮放把手（groupbox / panel / tray）---- */
  var rsz = null, rszDrag = null;
  function ensureRsz() {
    if (rsz) return;
    rsz = document.createElement('div'); rsz.id = 'htRszHandle'; rsz.title = '拖拉調整大小';
    document.body.appendChild(rsz);
    rsz.addEventListener('mousedown', function (e) {
      if (!selEl) return;
      rszDrag = { x: e.clientX, y: e.clientY, w: selEl.offsetWidth, h: selEl.offsetHeight };
      e.preventDefault(); e.stopPropagation();
    }, true);
  }
  function posRsz() {
    if (!rsz || !selEl || rsz.style.display !== 'block') return;
    var r = selEl.getBoundingClientRect();
    rsz.style.left = (r.right - 7) + 'px'; rsz.style.top = (r.bottom - 7) + 'px';
  }
  function showRsz(on) {
    ensureRsz();
    rsz.style.display = on ? 'block' : 'none';
    if (on) posRsz();
  }
  function showPanel(el) {
    buildPanel();
    if (selEl) selEl.classList.remove('hlSel');
    selEl = el; selStr = selOf(el); el.classList.add('hlSel');
    var p = (loadProps()[PAGE] || {})[selStr] || {};
    var cs = getComputedStyle(el);
    P('.ppTgt').textContent = el.id ? '#' + el.id : selStr;
    var isAdd = el.hasAttribute('data-htadd');
    P('.ppName').value = isAdd ? el.getAttribute('data-htadd') : (el.id || '');
    P('.ppName').disabled = !isAdd;
    P('.ppName').title = isAdd ? '改名後同步遷移已儲存的偏移/屬性' : 'dfm 元件名稱唯讀（重生成後固定）';
    // 型別分流：LED 無文字、LabeledLed 可選文字方位、Tray 可調行列、容器類可拖拉縮放
    var ct = compType(el), rec = addRec(el);
    P('.ppRowText').style.display = ct === 'led' ? 'none' : '';
    P('.ppRowPos').style.display = ct === 'labeledled' ? '' : 'none';
    if (ct === 'labeledled') {
      var pm = (el.title || '').match(/CaptionPos=(lp\w+)/);
      P('.ppPos').value = (rec && rec.pos) || (pm && pm[1]) || 'lpRight';
      P('.ppPos').disabled = !isAdd;
      P('.ppPos').title = isAdd ? '文字在燈號的位置' : 'dfm 元件位置固定（座標由 dfm 重生成）';
    }
    P('.ppRowTray').style.display = ct === 'tray' ? '' : 'none';
    if (ct === 'tray') {
      var tm = (el.title || '').match(/XItem=(\d+) YItem=(\d+)/);
      P('.ppTRow').value = (rec && rec.rows) || (tm && +tm[2]) || 13;
      P('.ppTCol').value = (rec && rec.cols) || (tm && +tm[1]) || 7;
      P('.ppTRow').disabled = P('.ppTCol').disabled = !isAdd;
      P('.ppTRow').title = P('.ppTCol').title = isAdd ? 'Tray 内部格數' : 'dfm 元件格數固定';
    }
    showRsz(ct === 'groupbox' || ct === 'panel' || ct === 'tray');
    P('.ppText').value = p.text != null ? p.text : getCompText(el);
    P('.ppFs').value = p.fs || ''; P('.ppFs').placeholder = parseInt(cs.fontSize) || '';
    P('.ppColor').value = p.color || toHex(cs.color);
    P('.ppBg').value = p.bg || toHex(cs.backgroundColor);
    P('.ppW').value = p.w || ''; P('.ppW').placeholder = Math.round(el.offsetWidth);
    P('.ppH').value = p.h || ''; P('.ppH').placeholder = Math.round(el.offsetHeight);
    var off = (loadAll()[PAGE] || {})[selStr] || [0, 0];
    P('.ppX').value = off[0]; P('.ppY').value = off[1];
    P('.ppHide').checked = !!p.hide;
    P('.ppDel').style.display = el.hasAttribute('data-htadd') ? '' : 'none';
    updateGrpInfo();
    panel.style.display = 'block';
    // 小視窗（iframe）內：面板不得超過可視區，超過時內部捲動
    panel.style.maxWidth = (innerWidth - 8) + 'px';
    panel.style.maxHeight = (innerHeight - 8) + 'px';
    panel.style.overflowY = 'auto'; panel.style.overflowX = 'hidden';
    // 出現在選取元件旁（優先右側，不夠改左側，夾限可視區）
    var r = el.getBoundingClientRect(), pw = panel.offsetWidth || 240, ph = panel.offsetHeight || 300;
    var x = r.right + 10, y = r.top;
    if (x + pw > innerWidth - 4) x = r.left - pw - 10;
    if (x < 4) x = Math.min(innerWidth - pw - 4, Math.max(4, Math.round(r.left)));
    y = Math.max(4, Math.min(y, innerHeight - ph - 4));
    panel.style.left = x + 'px'; panel.style.top = y + 'px';
    panel.style.right = 'auto'; panel.style.bottom = 'auto';
  }
  function hidePanel() {
    if (selEl) selEl.classList.remove('hlSel');
    selEl = null;
    showRsz(false);
    document.querySelectorAll('.hlGrpM').forEach(function (n) { n.classList.remove('hlGrpM'); });
    if (panel) panel.style.display = 'none';
  }
  function delAdd(el) {
    var name = el.getAttribute('data-htadd');
    if (!confirm('刪除自訂元件「' + name + '」？')) return;
    var all = loadAdds();
    all[PAGE] = (all[PAGE] || []).filter(function (r) { return r.name !== name; });
    if (!all[PAGE].length) delete all[PAGE];
    saveAdds(all);
    var lay = loadAll(); if (lay[PAGE]) { delete lay[PAGE]['#' + name]; saveAll(lay); }
    var pr = loadProps(); if (pr[PAGE]) { delete pr[PAGE]['#' + name]; saveProps(pr); }
    var gAll = loadGroups();
    if (gAll[PAGE]) {
      gAll[PAGE] = gAll[PAGE].map(function (g) { return g.filter(function (s) { return s !== '#' + name; }); })
                             .filter(function (g) { return g.length >= 2; });
      if (!gAll[PAGE].length) delete gAll[PAGE];
      saveGroups(gAll);
    }
    if (selEl === el) hidePanel();
    el.remove();
  }
  function pick(t) {
    if (!t || !t.closest) return null;
    if (t.id === 'htRszHandle') return null;
    if (t.closest('#htPropPanel')) return null;
    var tray = t.closest('.traypos');   // TTMyTray 為整體：內部格子不可個別拖拉/選取
    if (tray) return tray;
    var el = t.closest('[title],img,button,input,select,label');
    return (el && el !== document.body && el !== document.documentElement) ? el : null;
  }
  document.addEventListener('mousedown', function (e) {
    if (!editing || placeType) return;
    if (e.target.id === 'htRszHandle') return;   // 縮放把手自行處理
    if (e.target.closest && e.target.closest('#htPropPanel')) return;
    var el = pick(e.target); if (!el) return;
    e.preventDefault(); e.stopPropagation();
    if (e.ctrlKey) return;   // Ctrl+點選=多選（click 處理），不拖曳
    cur = el; curSel = selOf(el); sx = e.clientX; sy = e.clientY;
    var d = (loadAll()[PAGE] || {})[curSel] || [0, 0]; ox = d[0]; oy = d[1];
    grpDrag = null;
    var g = groupOf(curSel);
    if (g) {
      var offs = loadAll()[PAGE] || {};
      grpDrag = [];
      g.forEach(function (s) { var m = q(s); if (m) grpDrag.push({ s: s, el: m, o: offs[s] || [0, 0] }); });
    }
  }, true);
  document.addEventListener('mousemove', function (e) {
    if (!editing) return;
    if (rszDrag && selEl) {   // 拖拉縮放中：即時改寬高
      var rw = Math.max(10, rszDrag.w + e.clientX - rszDrag.x), rh = Math.max(10, rszDrag.h + e.clientY - rszDrag.y);
      selEl.style.width = rw + 'px'; selEl.style.height = rh + 'px';
      posRsz();
      e.preventDefault(); e.stopPropagation();
      return;
    }
    if (cur) {
      var dx = e.clientX - sx, dy = e.clientY - sy;
      if (grpDrag) grpDrag.forEach(function (it) { setTf(it.el, [it.o[0] + dx, it.o[1] + dy]); });
      else setTf(cur, [ox + dx, oy + dy]);
      if (cur === selEl) posRsz();
      return;
    }
    var el = pick(e.target);
    if (el !== hov) { if (hov) hov.classList.remove('hlDrag'); hov = el; if (hov) hov.classList.add('hlDrag'); }
  }, true);
  document.addEventListener('mouseup', function (e) {
    if (!editing) return;
    if (rszDrag && selEl) {   // 縮放結束：寫入 w/h 屬性並回寫欄位
      var fw = selEl.offsetWidth, fh = selEl.offsetHeight;
      rszDrag = null;
      setField('w', fw); setField('h', fh);
      if (panel) { P('.ppW').value = fw; P('.ppH').value = fh; }
      posRsz();
      e.preventDefault(); e.stopPropagation();
      return;
    }
    if (!cur) return;
    var dx = e.clientX - sx, dy = e.clientY - sy;
    var all = loadAll(), m = all[PAGE] || (all[PAGE] = {});
    var items = grpDrag || [{ s: curSel, el: cur, o: [ox, oy] }];
    items.forEach(function (it) {
      var vx = it.o[0] + dx, vy = it.o[1] + dy;
      if (!vx && !vy) delete m[it.s]; else m[it.s] = [vx, vy];
      moved[it.s] = 1;
    });
    if (!Object.keys(m).length) delete all[PAGE];
    saveAll(all);
    if (panel && selEl === cur && panel.style.display !== 'none') {
      var d2 = ((loadAll()[PAGE] || {})[curSel]) || [0, 0];
      P('.ppX').value = d2[0]; P('.ppY').value = d2[1];
    }
    cur = null; grpDrag = null;
  }, true);
  document.addEventListener('click', function (e) {
    if (!editing) return;
    if (e.target.id === 'htRszHandle') { e.stopPropagation(); e.preventDefault(); return; }
    if (e.target.closest && e.target.closest('#htPropPanel')) return;   // 屬性盤內正常操作
    e.stopPropagation(); e.preventDefault();   // 攞截原功能點擊
    if (placeType) {   // 點擊放置模式：在點擊處建立元件
      var t = placeType; placeType = null; document.body.classList.remove('layoutPlace');
      var created = placeComp(t, e.pageX, e.pageY);
      if (window.parent !== window) window.parent.postMessage({ compPlaced: 1 }, '*');
      if (created) showPanel(created);
    } else if (e.ctrlKey) {   // Ctrl+點選：多選（群組用，綠框）
      var el2 = pick(e.target); if (el2) mselToggle(el2);
    } else {
      var el = pick(e.target);
      if (el) showPanel(el); else hidePanel();
    }
  }, true);
  // 從 background 元件盤拖入：命名→顯示文字→建立並持久化
  document.addEventListener('dragover', function (e) { if (editing) e.preventDefault(); }, true);
  document.addEventListener('drop', function (e) {
    if (!editing) return;
    var t = (e.dataTransfer && e.dataTransfer.getData('text/plain')) || '';
    if (t.indexOf('HTCOMP:') !== 0) return;
    e.preventDefault(); e.stopPropagation();
    placeComp(t.slice(7), e.pageX, e.pageY);
  }, true);
  // 右鍵刪除自訂元件（並清其偏移與屬性）
  document.addEventListener('contextmenu', function (e) {
    if (!editing) return;
    var el = e.target.closest ? e.target.closest('[data-htadd]') : null;
    if (!el) return;
    e.preventDefault(); e.stopPropagation();
    delAdd(el);
  }, true);
  var st = document.createElement('style');
  st.textContent = 'body.layoutEdit,body.layoutEdit *{cursor:move!important;}' +
    'body.layoutEdit .hlDrag{outline:2px dashed #e6007e;outline-offset:1px;}' +
    'body.layoutEdit.layoutPlace,body.layoutEdit.layoutPlace *{cursor:crosshair!important;}' +
    'body.layoutEdit .hlSel{outline:2px solid #2196f3!important;outline-offset:1px;}' +
    'body.layoutEdit .hlMulti{outline:2px dashed #4caf50!important;outline-offset:1px;}' +
    'body.layoutEdit .hlGrpM{outline:2px dotted #ff9800!important;outline-offset:1px;}' +
    '#htPropPanel{position:fixed;bottom:10px;right:10px;z-index:100000;background:#28323c;color:#eee;border:1px solid #567;border-radius:8px;padding:8px 10px;box-shadow:0 4px 16px rgba(0,0,0,.45);display:none;width:224px;font:12px var(--font-ui,"Microsoft JhengHei",sans-serif);}' +
    '#htPropPanel .ppTitle{font-weight:bold;margin-bottom:6px;color:#ffd54a;}' +
    '#htPropPanel .ppTgt{font-weight:normal;color:#9fd;word-break:break-all;font-size:11px;}' +
    '#htPropPanel .ppRow{display:flex;align-items:center;gap:4px;margin:4px 0;}' +
    '#htPropPanel label{min-width:38px;color:#cde;}' +
    '#htPropPanel input[type=text]{flex:1;min-width:0;}' +
    '#htPropPanel input[type=number]{width:52px;}' +
    '#htPropPanel input[type=color]{width:34px;height:22px;padding:0;border:none;background:none;}' +
    '#htPropPanel button{font-size:11px;padding:2px 8px;}' +
    '#htPropPanel .ppBtns{justify-content:flex-end;margin-top:6px;}' +
    'body.layoutEdit #htPropPanel,body.layoutEdit #htPropPanel *{cursor:auto!important;}' +
    'body.layoutEdit #htPropPanel button,body.layoutEdit #htPropPanel input[type=checkbox],body.layoutEdit #htPropPanel input[type=color]{cursor:pointer!important;}' +
    '#htPropPanel .ppTitle,body.layoutEdit #htPropPanel .ppTitle{cursor:move!important;user-select:none;}' +
    '#htRszHandle{position:fixed;width:14px;height:14px;background:#2196f3;border:2px solid #fff;border-radius:3px;z-index:100001;display:none;box-shadow:0 1px 4px rgba(0,0,0,.4);}' +
    'body.layoutEdit #htRszHandle{cursor:nwse-resize!important;}';
  document.head.appendChild(st);

  function exitMode() {
    editing = false; document.body.classList.remove('layoutEdit'); document.body.classList.remove('layoutPlace');
    placeType = null; rszDrag = null;
    hidePanel(); mselClear();
    if (hov) { hov.classList.remove('hlDrag'); hov = null; } cur = null; grpDrag = null;
  }
  window.addEventListener('message', function (ev) {
    var d = ev.data || {};
    if (d.type === 'HT_LAYOUT_PLACE') {
      placeType = d.ctype || null;
      document.body.classList.toggle('layoutPlace', !!placeType);
    }
    if (d.type === 'HT_LAYOUT_EDIT') {
      if (d.on) {
        editing = true; snap = localStorage.getItem(KEY) || '{}'; snap2 = localStorage.getItem(KEY2) || '{}'; snap3 = localStorage.getItem(KEY3) || '{}'; snap4 = localStorage.getItem(KEY4) || '{}';
        moved = {}; document.body.classList.add('layoutEdit');
        applyProps();   // 隱藏屬性在編輯中改顯半透明可選
      }
      else { exitMode(); applyProps(); }
    }
    if (d.type === 'HT_LAYOUT_SAVE') { exitMode(); applyProps(); }
    if (d.type === 'HT_LAYOUT_CANCEL') {
      if (snap !== null) { try { localStorage.setItem(KEY, snap); } catch (e) {} }
      if (snap2 !== null) { try { localStorage.setItem(KEY2, snap2); } catch (e) {} }
      if (snap3 !== null) { try { localStorage.setItem(KEY3, snap3); } catch (e) {} }
      if (snap4 !== null) { try { localStorage.setItem(KEY4, snap4); } catch (e) {} }
      Object.keys(moved).forEach(function (s) { var el = q(s); if (el) el.style.transform = ''; });
      Object.keys(ORIG).forEach(function (s) { var el = q(s); if (el) { el.setAttribute('style', ORIG[s].style); if (ORIG[s].tt) setCompText(el, ORIG[s].text); } });
      ORIG = {};
      applyAdds(); applyLayout(); exitMode(); applyProps();
    }
  });
  window.HTLayout = { key: KEY, page: PAGE, apply: applyLayout };
})();

/* AI(W906-SMM) 20260925：C++ 的 ShowMyMessage／MyMessageBox 要在「任何頁面」照 golden 顯示並回答
 * （Steven 20260925 指示）。本檔幾乎每一頁都載，所以從這裡帶進 ht9045_modal.js，不逐頁加 <script>。
 * 由誰畫由 ht9045_modal.js 自己判斷（iframe／file:／dialog 頁不畫；background.html 交給 dialog-bridge.js）。
 * 路徑相對於本檔自己（background.html 載的是 page/theme.js，page/*.html 載的是 theme.js）。 */
(function () {
  if (window.__HT9045_MODAL__) return;
  var s = document.currentScript, src = (s && s.src) ? String(s.src) : '';
  if (!src) return;
  var el = document.createElement('script');
  el.src = src.substring(0, src.lastIndexOf('/') + 1) + 'ht9045_modal.js';
  (document.head || document.documentElement).appendChild(el);
})();

/* AI(W906-ES02-W161) 20261008：W-161（EastSun 1007 經機台 cpp 0297「請派工jimmy 偵測所有畫面是否有擋到」，規則「不要蓋住畫面」）——
 * 訊息框 #ht9045WireBar（引擎與七支頁面補件各自建同一個 id）出現或內容一變，就挑一個「蓋到最少控制項與文字」的位置：
 * 右下（theme.css 的 WIREBAR-CORNER 預設）、右上、左下、左上、上中、下中。20261003 固定放右下角，但 Setup.Contact 的
 * Save／Exit、Contact Force Calibration 也在右下 —— 真機上這頁比螢幕高、視窗被限高，框就壓在它們上面（W-161 量測：
 * tools/webprobe/w161_overlap_scan.cjs）。點穿（pointer-events:none）照舊；這裡只決定位置。
 * inline 的 !important 蓋得過 theme.css 的 !important（WIREBAR-CORNER）。 */
(function () {
  if (window.__HT9045_WIREBAR_PLACE__) return;
  window.__HT9045_WIREBAR_PLACE__ = true;
  var SEL = 'button, input:not([type=hidden]), select, textarea, a[href], [onclick], .btn3d, label, .lb, legend, .pcTabs > .tab';
  var placing = false, last = '';
  function vis(el) {
    var r = el.getBoundingClientRect();
    if (r.width < 2 || r.height < 2) return null;
    var cs = getComputedStyle(el);
    if (cs.visibility === 'hidden' || +cs.opacity === 0) return null;
    return r;
  }
  function place(b) {
    if (placing || !b || b.style.display === 'none' || !b.textContent) return;
    var W = window.innerWidth, H = window.innerHeight;
    if (!W || !H) return;
    placing = true;
    try {
      var set = function (k, v) { b.style.setProperty(k, v, 'important'); };
      set('max-width', Math.max(200, Math.min(420, W - 16)) + 'px');
      ['top', 'bottom', 'left', 'right'].forEach(function (k) { set(k, 'auto'); });
      set('right', '8px'); set('bottom', '28px');
      var br = b.getBoundingClientRect(), bw = br.width, bh = br.height;
      if (!bw || !bh) return;
      // what must stay visible: controls (weight 4) and text (1), each by the part of it the box would hide
      var items = [], els = document.querySelectorAll(SEL);
      for (var i = 0; i < els.length && items.length < 4000; i++) {
        var el = els[i];
        if (el === b || b.contains(el)) continue;
        var r = vis(el);
        if (!r || r.right < 0 || r.bottom < 0 || r.left > W || r.top > H) continue;
        var text = (/^(LABEL|LEGEND)$/.test(el.tagName) && !el.querySelector('input')) || / lb( |$)/.test(' ' + el.className);   // (a label holding a check box / radio is that control)
        items.push({ r: r, w: text ? 1 : 4, a: Math.max(1, r.width * r.height) });
      }
      var cx = Math.max(8, Math.round((W - bw) / 2));
      var C = [
        { name: 'br', css: { right: '8px', bottom: '28px' }, x: W - 8 - bw, y: H - 28 - bh },
        { name: 'tr', css: { right: '8px', top: '8px' }, x: W - 8 - bw, y: 8 },
        { name: 'bl', css: { left: '8px', bottom: '28px' }, x: 8, y: H - 28 - bh },
        { name: 'tl', css: { left: '8px', top: '8px' }, x: 8, y: 8 },
        { name: 'tc', css: { left: cx + 'px', top: '8px' }, x: cx, y: 8 },
        { name: 'bc', css: { left: cx + 'px', bottom: '28px' }, x: cx, y: H - 28 - bh }
      ];
      var best = null;
      var costAt = function (x, y) {
        var cost = 0;
        for (var k = 0; k < items.length; k++) {
          var r = items[k].r;
          var ox = Math.min(r.right, x + bw) - Math.max(r.left, x), oy = Math.min(r.bottom, y + bh) - Math.max(r.top, y);
          if (ox > 0 && oy > 0) cost += (ox * oy / items[k].a) * items[k].w;   // (how much of each one is hidden: many small controls weigh more than one big one)
        }
        return cost;
      };
      C.forEach(function (c) {
        var cost = costAt(c.x, c.y);
        if (!best || cost < best.cost) best = { c: c, cost: cost };
      });
      // (W-161 measured: on a small window every corner holds Save / Exit -- then any free place of the window, on a 16 px grid,
      //  the one nearest the bottom right first; none free = the least covering of all)
      if (best.cost > 0) {
        var step = 16, g = null;
        for (var gy = H - 28 - bh; gy >= 8; gy -= step) {
          for (var gx = W - 8 - bw; gx >= 8; gx -= step) {
            var gc = costAt(gx, gy);
            if (!g || gc < g.cost) g = { x: gx, y: gy, cost: gc };
            if (gc === 0) break;
          }
          if (g && g.cost === 0) break;
        }
        if (g && g.cost < best.cost) best = { c: { name: 'grid', css: { left: Math.round(g.x) + 'px', top: Math.round(g.y) + 'px' } }, cost: g.cost };
      }
      ['top', 'bottom', 'left', 'right'].forEach(function (k) { set(k, best.c.css[k] || 'auto'); });
      b.setAttribute('data-place', best.c.name + (best.cost ? '' : ' free'));
    } catch (e) { /* stays where theme.css put it */ } finally { placing = false; }
  }
  var watched = null, timer = null;
  function soon() { clearTimeout(timer); timer = setTimeout(function () { place(watched); }, 30); }
  function watch(b) {
    if (!b || b === watched || !window.MutationObserver) return;
    watched = b;
    new MutationObserver(function () { if (placing) return; var k = b.style.display + '|' + b.textContent; if (k !== last) { last = k; soon(); } })   // (only a new text or shown / hidden: its own style changes do not move it again)
      .observe(b, { attributes: true, attributeFilter: ['style'], childList: true, characterData: true, subtree: true });
    soon();
  }
  function boot() {
    if (!document.body || !window.MutationObserver) return;
    watch(document.getElementById('ht9045WireBar'));
    new MutationObserver(function (recs) {
      for (var i = 0; i < recs.length; i++) for (var j = 0; j < recs[i].addedNodes.length; j++) if (recs[i].addedNodes[j].id === 'ht9045WireBar') watch(recs[i].addedNodes[j]);
    }).observe(document.body, { childList: true });
    window.addEventListener('resize', soon);
  }
  if (document.readyState === 'loading') document.addEventListener('DOMContentLoaded', boot, { once: true }); else boot();
})();
