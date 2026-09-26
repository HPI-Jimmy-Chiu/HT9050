/* ht9045_towerlight_wire.js -- Status.TowerLight.html <-> fTowerLight（golden V912 cTowerLight.cpp TfTowerLight）
 * ---------------------------------------------------------------------------
 * Steven 團隊 20260925（手寫；AI(W906-TOWERLIGHT)。檔名刻意不叫 ht9045_wire_<slug>.js，免得產生器覆蓋）。
 *
 * 後端：tools/wb_serve.cpp 的 towerlight.op 分派 → WebTowerLight.cpp → forms/fTowerLight.cpp（逐行翻譯的 golden）。
 * 網頁只是畫面：燈號輪替、鉗制、權限、寫檔全部在 C++。
 *
 *   開頁        towerlight.op {op:'get'}                         = golden FormShow（:83-127）
 *   點燈        towerlight.op {op:'click', led:'RGB12'}          = golden RGB00Click（:55-81）：0→1→2→0，WriteLastDataFile
 *   換音樂      towerlight.op {op:'music', combo:'cbJam', index}  = FormShow → 換框 → FormClose（:129-140）→ WriteLastDataFile
 *   Music Test  不接：試聽是 SW[SwMusic1..4] 實體輸出（golden rgMusicTestClick :142-147）
 *
 * 燈號顯示照 golden UpdateTowerLed（:24-53）：0 滅、1 亮、2 閃（golden Timer1 10ms、每 6 拍翻一次，這裡用 100ms）。
 * 下方狀態列另外顯示 tower.green／tower.amber／tower.red 三個 tag（= ShowRunLed 算出來的 fMain 塔燈，預覽用）。
 * ---------------------------------------------------------------------------
 */
(function () {
  'use strict';

  var COMBOS = ['cbRunning', 'cbJam', 'cbPause', 'cbMessage', 'cbHeating', 'cbHome', 'cbOffLine', 'cbART'];
  var LEDS = [];
  for (var i = 0; i < 8; i++) for (var j = 0; j < 3; j++) LEDS.push('RGB' + i + j);

  var state = null, busy = false, lastError = '', lastResult = null, loads = 0, blink = false;

  function $(id) { return document.getElementById(id); }

  function statusLine() {
    var s = $('tlStatus');
    if (!s) {
      s = document.createElement('div');
      s.id = 'tlStatus';
      s.style.cssText = 'position:absolute;left:460px;top:330px;width:290px;font-size:11px;color:#234;white-space:pre-wrap;';
      var host = $('Panel3') || document.body;
      host.appendChild(s);
    }
    return s;
  }
  function towerLine() {
    var s = $('tlTower');
    if (!s) {
      s = document.createElement('div');
      s.id = 'tlTower';
      s.style.cssText = 'position:absolute;left:460px;top:300px;width:290px;font-size:11px;color:#234;white-space:nowrap;';
      var host = $('Panel3') || document.body;
      host.appendChild(s);
    }
    return s;
  }
  function say(msg, bad) {
    var s = statusLine();
    s.textContent = msg;
    s.style.color = bad ? '#b00' : '#234';
  }

  function unwrap(m) {
    if (m && typeof m.value === 'string') { try { var j = JSON.parse(m.value); if (j && typeof j === 'object') return j; } catch (e) {} }
    return m;
  }
  function errText(e) {
    var t = (e && e.message) || String(e);
    try { var j = JSON.parse(t); if (j && j.guard) return j.guard + (j.detail ? '：' + j.detail : ''); } catch (x) {}
    return t;
  }

  function paintLeds() {
    if (!state || !state.widgets) return;
    LEDS.forEach(function (id) {
      var w = state.widgets[id], el = $(id);
      if (!w || !el) return;
      el.classList.remove('unknown');
      var on = w.value === 1 || (w.value === 2 && blink);
      el.classList.toggle('on', on);
      el.title = id + ' : TALed  [' + (w.value === 0 ? '滅' : w.value === 1 ? '亮' : '閃') + ']' +
                 (w.clickChanges ? '' : '（這台機台超出 iOfflineRun，golden 點了不會變）');
    });
  }

  function apply(s) {
    if (!s || !s.widgets) return;
    state = s;
    COMBOS.forEach(function (id) {
      var w = s.widgets[id], el = $(id);
      if (!w || !el) return;
      if (w.itemIndex >= 0 && w.itemIndex < el.options.length) el.selectedIndex = w.itemIndex;
      el.disabled = !w.enabled;
    });
    var p11 = $('Panel11');
    if (p11 && s.widgets.Panel11) p11.style.display = s.widgets.Panel11.visible ? '' : 'none';
    var pa = $('palART');
    if (pa && s.widgets.palART) { var cap = pa.querySelector('.pnlCap'); if (cap) cap.textContent = s.widgets.palART.caption; }
    var rg = $('rgMusicTest');
    if (rg && s.widgets.rgMusicTest) {
      var rw = s.widgets.rgMusicTest;
      rg.querySelectorAll('input[type="radio"]').forEach(function (r, k) { r.disabled = !rw.enabled; r.checked = (k === rw.itemIndex); });
      rg.title = 'rgMusicTest : TRadioGroup — ' + (rw.reason || '');
    }
    paintLeds();
  }

  function cmd(extra) {
    if (!window.HT9045Recipe || !HT9045Recipe.rawCmd) return Promise.reject(new Error('ht9045_recipe_client.js 沒有載入'));
    return HT9045Recipe.rawCmd('towerlight.op', { value: JSON.stringify(extra) }).then(unwrap);
  }
  function acquire() {
    if (!window.HT9045Recipe || !HT9045Recipe.rawCmd) return Promise.resolve();
    return HT9045Recipe.rawCmd('control.acquire').catch(function () { /* 已持有或他人持有：後續指令自己會回錯 */ });
  }

  function load() {
    return acquire().then(function () { return cmd({ op: 'get' }); }).then(function (s) {
      apply(s); loads++; lastError = '';
      say('已讀取 C++ fTowerLight（golden FormShow）');
      return s;
    }).catch(function (e) {
      lastError = errText(e);
      say('讀取失敗：' + lastError, true);
      throw e;
    });
  }

  function run(req, doneMsg) {
    if (busy) return Promise.reject(new Error('busy'));
    busy = true;
    return cmd(req).then(function (r) {
      lastResult = r; lastError = '';
      apply(r);
      if (r.guard === 'A01_2') say(r.message || '[A01_2] Operator 權限不可修改', true);
      else say(doneMsg(r));
      return r;
    }).catch(function (e) {
      lastError = errText(e); lastResult = null;
      say('被拒：' + lastError, true);
      if (state) apply(state);            // 回到伺服器的狀態
      throw e;
    }).then(function (r) { busy = false; return r; }, function (e) { busy = false; throw e; });
  }

  function click(id) {
    return run({ op: 'click', led: id }, function (r) {
      return id + '：' + r.before + ' → ' + r.after + (r.written ? '（已寫 lastdata.dat）' : '（未寫檔）');
    });
  }
  function music(id) {
    var el = $(id);
    return run({ op: 'music', combo: id, index: el ? el.selectedIndex : -1 }, function (r) {
      return id + '：' + r.before + ' → ' + r.after + (r.written ? '（已寫 lastdata.dat）' : '（寫檔失敗）');
    });
  }

  function bind() {
    LEDS.forEach(function (id) {
      var el = $(id);
      if (!el) return;
      el.classList.add('unknown');
      el.style.cursor = 'pointer';
      el.addEventListener('click', function () { click(id).catch(function () {}); });
    });
    COMBOS.forEach(function (id) {
      var el = $(id);
      if (el) el.addEventListener('change', function () { music(id).catch(function () {}); });
    });
    setInterval(function () { blink = !blink; paintLeds(); }, 100);
    if (window.HT9045Tags && HT9045Tags.subscribe) {
      var show = function () {
        var g = HT9045Tags.get('tower.green'), a = HT9045Tags.get('tower.amber'), r = HT9045Tags.get('tower.red');
        var f = function (v) { return v === true ? '●' : v === false ? '○' : '—'; };
        towerLine().textContent = '目前塔燈（tower.*）  綠 ' + f(g) + '  黃 ' + f(a) + '  紅 ' + f(r);
      };
      HT9045Tags.subscribe(show);
      show();
    }
  }

  window.HT9045TowerLight = {
    reload: load,
    click: click,
    music: music,
    state: function () { return state; },
    lastResult: function () { return lastResult; },
    lastError: function () { return lastError; },
    loads: function () { return loads; },
    busy: function () { return busy; }
  };

  function start() { bind(); load().catch(function () {}); }
  if (document.readyState === 'loading') document.addEventListener('DOMContentLoaded', start);
  else start();
})();
