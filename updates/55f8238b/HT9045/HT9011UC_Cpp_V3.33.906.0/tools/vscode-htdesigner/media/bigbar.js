// AI(W906-HTDESIGNER) 20261008 (EastSun「這邊我需要按鈕尺寸放大 並且做得明顯一點 看得超不清楚」, the title bar's run buttons -- VS
// Code draws those 16 px and gives no way to enlarge them; EastSun chose「外掛做一排大按鈕」): the run / debug buttons big, in
// colour, with their words and keys -- 方案總管's toolbar and the 除錯 group draw the same bar. A button that cannot be used
// now is grey. The state is RunBar.snapshot() (st, b: { start, pause, stop, restart, over, into, out: { cmd, tip } }).
(function () {
  'use strict';
  var CH = { 'debug-start': 0xead3, 'debug-continue': 0xeacf, 'debug-pause': 0xead1, 'debug-stop': 0xead7, 'debug-restart': 0xead2,
    'debug-step-over': 0xead6, 'debug-step-into': 0xead4, 'debug-step-out': 0xead5, 'sync': 0xea77 };
  // (one = the strip on top of the editor area: every button in one row, a gap between the run and the step ones)
  function draw(el, s, send, ch, one) {
    if (ch) for (var k0 in ch) if (ch[k0]) CH[k0] = ch[k0];   // (the code points of the VS Code that runs it)
    s = s || { st: 'idle', b: {} };
    var B = s.b || {};
    el.textContent = '';
    el.className = 'bigbar' + (one ? ' one' : '');
    var row = function () { var d = document.createElement('div'); d.className = 'bbrow'; el.appendChild(d); return d; };
    var btn = function (into, key, icon, text, keyName, cls) {
      var r = B[key] || {};
      var b = document.createElement('button');
      b.type = 'button';
      b.className = 'bb ' + cls;
      b.dataset.key = key;
      var i = document.createElement('span'); i.className = 'bi'; i.textContent = String.fromCharCode(CH[icon] || 0x3F); b.appendChild(i);
      var t = document.createElement('span'); t.className = 'bt'; t.textContent = text; b.appendChild(t);
      if (keyName) { var k = document.createElement('span'); k.className = 'bk'; k.textContent = keyName; b.appendChild(k); }
      b.title = (r.tip || text) + (keyName ? '（' + keyName + '）' : '');
      if (r.cmd) b.dataset.cmd = r.cmd; else b.disabled = true;
      into.appendChild(b);
      return b;
    };
    var r1 = row(), r2 = one ? r1 : row();
    var gap = function () { var g = document.createElement('span'); g.className = 'bgap'; r1.appendChild(g); };
    // (1008 EastSun「圖片上這排不用了 都可以加到剛剛加的按鈕列了 重複功能不要加入」: the strip on top takes the title bar row's
    //  own buttons too -- save, save all, back, forward; find, the tab list, close others -- none of them twice)
    if (one) {
      B.save = B.save || { cmd: 'ht9045Designer.saveFile', tip: '儲存目前的檔案' };
      B.saveAll = B.saveAll || { cmd: 'ht9045Designer.saveAllFiles', tip: '全部儲存' };
      B.back = B.back || { cmd: 'ht9045Designer.navBack', tip: '回到上一個位置' };
      B.fwd = B.fwd || { cmd: 'ht9045Designer.navForward', tip: '到下一個位置' };
      btn(r1, 'save', 'save', '儲存', 'Ctrl+S', 'edit');
      btn(r1, 'saveAll', 'save-all', '全部儲存', '', 'edit');
      btn(r1, 'back', 'arrow-left', '', 'Alt+←', 'edit nav');
      btn(r1, 'fwd', 'arrow-right', '', 'Alt+→', 'edit nav');
      gap();
    }
    var st = s.st;
    var startText = st === 'building' ? '建置中 ' + (s.pct || 0) + '%' : st === 'paused' ? '繼續' : st === 'running' ? '執行中' : '啟動';
    /* (1009 review: the keys as they are bound -- BCB's only with bcbDebugKeys on (package.json + lib/bcbkeys.js); VS Code's
       F5 / Shift+F5 / F10 / F11 / Shift+F11 without them) */
    var K = s.bcb === false ? { go: 'F5', stop: 'Shift+F5', over: 'F10', into: 'F11', out: 'Shift+F11' } : { go: 'F9', stop: 'Ctrl+F2', over: 'F8', into: 'F7', out: 'Shift+F8' };
    var sb = btn(r1, 'start', st === 'paused' ? 'debug-continue' : 'debug-start', startText, st === 'paused' ? K.go : '', 'go' + (st === 'building' ? ' busy' : st === 'running' ? ' live' : ''));
    if (st === 'building' || st === 'running') sb.classList.add('state');
    btn(r1, 'pause', 'debug-pause', '暫停', '', 'pause');
    btn(r1, 'stop', 'debug-stop', '停止', K.stop, 'stop');
    btn(r1, 'restart', 'debug-restart', '重新啟動', '', 'restart');
    if (one) { var g = document.createElement('span'); g.className = 'bgap'; r1.appendChild(g); }
    btn(r2, 'over', 'debug-step-over', '單步', K.over, 'step');
    btn(r2, 'into', 'debug-step-into', '進入', K.into, 'step');
    btn(r2, 'out', 'debug-step-out', '跳出', K.out, 'step');
    if (one) {
      gap();
      B.find = B.find || { cmd: 'ht9045Designer.find', tip: '尋找（游標所在的字）' };
      B.tabs = B.tabs || { cmd: 'ht9045Designer.tabList', tip: '開著的檔案清單' };
      B.closeOthers = B.closeOthers || { cmd: 'ht9045Designer.tabCloseOthers', tip: '關閉這一區的其他檔案' };
      btn(r1, 'find', 'search', '尋找', 'Ctrl+F', 'edit');
      btn(r1, 'tabs', 'list-flat', '分頁清單', '', 'edit');
      btn(r1, 'closeOthers', 'close-all', '關閉其他', '', 'edit');
    }
    if (!el.__htdBound) {
      el.__htdBound = true;
      el.addEventListener('click', function (e) {
        var b = e.target.closest && e.target.closest('.bb');
        if (b && !b.disabled && b.dataset.cmd) send(b.dataset.cmd);
      });
    }
  }
  window.HtdBigBar = { draw: draw };
})();
