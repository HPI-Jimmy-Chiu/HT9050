# Recipe Client完整原文（1／1）

[上層](../index.md)／[manifest](../source-manifest.json)。固定pin `2f2872da7fce4b7ae6eb6685885de626885ccd92`，來源 `web/page/ht9045_recipe_client.js`。
整檔909行含全部metadata、原註解、API objects及callbacks；本頁payload 180行，context credit0。

<a id="connect"></a> `function connect()`。
<a id="cmd"></a> `function cmd(name, extra)`。
<a id="cmd0"></a> `function cmd0(name, extra)`。
<a id="unwrapack"></a> `function unwrapAck(m)`。
<a id="acquire"></a> `function acquire()`。
<a id="takeover"></a> `function takeover()`。
<a id="tokenheld"></a> `function tokenHeld()`。
<a id="armtokenidle"></a> `function armTokenIdle()`。
<a id="tokentrack"></a> `function tokenTrack(name, extra, p, retried)`。

```javascript
<!-- preserved-content:start -->
//Steven 20260916
// ----------------------------------------------------------------------
// 兩件事：
// (1) HT9045Tags —— 執行期 tag 串流。onmessage 原本只有 ack 一個分支、沒有
//     else，伺服器每 500ms 送的 snapshot／patch 與 alarm／modal／query 全部
//     被靜默丟棄。C++ 端早就完整，缺口一直只在這一段。
// (2) HT9045System.levels —— 二進位定長陣列的個別元素寫入（system.levels.put），
//     給 Status.Security 的 179 組 radio（system\levelset.dat）用。
// 當日完整變更紀錄：D:\docs\ChangeLog\CHANGES_20260916_Steven.md
// ----------------------------------------------------------------------

//Steven 20260915
// ----------------------------------------------------------------------
// 修正 dryRun 被靜默丟棄的 bug：WebCommand 只認 {id,cmd,tag,value,connId}，
// dryRun 當兄弟欄位會被丟掉，導致 preview() 在操作員確認前就真的寫下去。
// 已改為放進 value 裡面。另新增 HT9045System 全域（list/read/write/preview）。
// 當日完整變更紀錄：D:\HT9045\CHANGES_20260915_Steven.md
// ----------------------------------------------------------------------

// ht9045_recipe_client.js -- the browser side of the HT9045 recipe read/write
// pipe, in the shape the hand-built HMI already uses.
//
// AI(W906-BA-C1CLIENT) 20260911.
//
// ---------------------------------------------------------------------------
// WHY THIS FILE EXISTS, AND WHY IT IS NOT AN INVISIBLE ADAPTER
// ---------------------------------------------------------------------------
// The HMI has its own write seam (HTJsonWriter / HTDialogHost) and it was
// tempting to override that so pages would "just work" with no edits. Measured
// 20260911, that would connect nothing: the documents the HMI writes through
// HTJsonWriter are dialog responses, motor-access / state-record (both guarded
// by cfg.offline), and a Config / General-config EXPORT whose own status
// message says 需工程師轉回 .ini. None of them is a recipe document, and the
// recipe pipe is the only write path C++ currently accepts. An override would
// have produced something that looked wired and was not.
//
// So this is an explicit client instead. It is small, it hides nothing, and it
// matches the agreed workflow: the pipe is built and proven first, then the
// pages are adapted to call it.
//
// ---------------------------------------------------------------------------
// WHAT THE SERVER OFFERS  (tools/wb_serve.cpp)
// ---------------------------------------------------------------------------
//   GET  /api/recipe/          -> {recipe, path, documents:[...]}
//   GET  /api/recipe/<doc>     -> {path, available, sections:{<sec>:{<key>:
//                                  {value, type, raw}}}}
//   ws://<host>/ht9045         -> {"type":"cmd","id":N,"cmd":"control.acquire"}
//                                 {"type":"cmd","id":N,"cmd":"recipe.doc.put",
//                                  "tag":"<doc>","value":"<json string>"}
//
// <doc> is the file stem with the first letter lowercased -- handlerCondition,
// testMode, contact, udUld ... i.e. exactly the keys Setup-current.json already
// uses under documents.*, so existing page code needs no renaming.
//
// THE PER-FIELD SHAPE IS ALREADY COMPATIBLE. settings.js:154 unwraps a value
// only when BOTH 'value' and 'raw' are present, and the server emits value,
// type and raw. Had it emitted only {raw,type}, that guard would have failed
// and the page would have received the whole object instead of the scalar --
// silently, rendering [object Object]. Do not "simplify" the payload.
//
// ---------------------------------------------------------------------------
// THREE CONSTRAINTS THAT ARE NOT NEGOTIABLE
// ---------------------------------------------------------------------------
// 1. WRITES GO OVER THE WEBSOCKET, NOT HTTP. The HTTP side answers GET and HEAD
//    only and returns 405 otherwise, because the server's request parser stops
//    at the end of the headers -- there is no request-body buffering anywhere in
//    the connection state machine. A POST would need that machinery, not just a
//    route.
// 2. value MUST BE A STRING. The command channel carries null/bool/number/string
//    and nothing else, so the document is sent as a JSON string, not an object.
//    write() does that for you.
// 3. ONE WRITER AT A TIME. control.acquire takes a single-operator token; a
//    second connection is refused while it is held. acquire() is called for you
//    on first write and the token is released on disconnect.
//
// ---------------------------------------------------------------------------
// USAGE
// ---------------------------------------------------------------------------
//   HT9045Recipe.list().then(function (r) { console.log(r.recipe, r.documents); });
//
//   HT9045Recipe.read('contact').then(function (doc) {
//     console.log(doc.sections['Test Arm1']['Pick Up'].value);   // 1.25
//   });
//
//   // Always preview first. dryRun reports what WOULD change and writes nothing.
//   HT9045Recipe.write('contact', {'Test Arm1': {'Pick Up': '1.25'}}, {dryRun: true})
//     .then(function (res) { console.log(res.changed, res.identical, res.notFound); });
//
// The server rewrites ONLY the value text after the first '=' on the lines you
// name. Everything else in the file is copied byte for byte, so comments,
// ordering, blank lines and even orphaned keys survive. That is deliberate:
// re-serialising from a parsed store would silently drop the 23 orphaned
// parameters in the one corrupted recipe document on this machine.
(function (global) {
  'use strict';

  var cfg = {
    base: '',                 // same-origin by default
    wsPath: '/ht9045',
    timeoutMs: 15000
  };

  function httpBase() {
    return cfg.base || '';
  }

  function getJson(url) {
    // XMLHttpRequest rather than fetch, to match the HMI's own loader and keep
    // this usable from the same places settings.js runs.
    return new Promise(function (resolve, reject) {
      var x = new XMLHttpRequest();
      x.open('GET', url + (url.indexOf('?') < 0 ? '?' : '&') + '_=' + Date.now(), true);
      x.onreadystatechange = function () {
        if (x.readyState !== 4) return;
        if (x.status < 200 || x.status >= 300) {
          reject(new Error('GET ' + url + ' -> HTTP ' + x.status));
          return;
        }
        try { resolve(JSON.parse(x.responseText)); }
        catch (e) { reject(new Error('GET ' + url + ' -> bad JSON: ' + e.message)); }
      };
      x.onerror = function () { reject(new Error('GET ' + url + ' -> network error')); };
      x.send();
    });
  }

  // --- the command socket -------------------------------------------------
  var sock = null, nextId = 1, pending = {}, opening = null, haveToken = false;  var tokenIdleMs = 30000, tokenTimer = null, tokenInflight = 0, tokenHolds = [];   // AI(W906-TOKEN-IDLE) 20260926: 閒置自動還權杖，見 global.HT9045Recipe 上方那段

  // Steven 20260918 (W906-FW-WINREG)：ui.windows.put 這個指令**伺服器還不接受**。
  //   null = 還沒問過（或剛重連）、true = 送得進去、false = 被拒過。
  // 斷線時要歸回 null —— 換一個伺服器版本上來就該重新問一次，
  // 而不是因為上一代不支援就永遠不再嘗試。
  var winSupported = null, winRefusal = '';  var linkWait = (function () { try { if (global.HT9045Link || global.parent === global || !document.currentScript || !document.currentScript.src) return null; var src = document.currentScript.src.replace(/[^\/?#]*([?#].*)?$/, 'ht9045_link.js'); var hubParent = (function () { try { var L = global.parent.HT9045Link; return !!(L && typeof L.hub === 'function' && L.hub(wsUrl())); } catch (e2) { return false; } })(); return new Promise(function (res) { var tries = 0; (function attempt() { var el = document.createElement('script'); el.src = src; var t = setTimeout(res, hubParent ? 30000 : 2000); el.onload = function () { clearTimeout(t); res(); }; el.onerror = function () { clearTimeout(t); if (hubParent && ++tries < 2) attempt(); else res(); }; (document.head || document.documentElement).appendChild(el); })(); }); } catch (e) { return null; } })();   // AI(W906-WSLINK) 20260929 [W906] ST01-E3：一個瀏覽器分頁只開一條 WebSocket —— 在外框（background.html）的 iframe 裡，這一頁的連線經外框的 hub（同資料夾的 ht9045_link.js）；頁面沒載它就在這裡載一次，connect() 等它（最多 2 秒），載不到／外框沒有 hub ＝照舊直接開 WebSocket。AI(W906-SCREEN-TOKEN) 20261001（RULINGS_20261001，Jimmy 1001 12:3x「1→A」一個畫面一條連線）：外框**有** hub（同來源讀得到 parent.HT9045Link.hub(url)）時不再 2 秒就放棄——載得慢就等（上限 30 秒），載入失敗再試一次，兩次都失敗才直連；讀不到外框的 hub 照舊 2 秒

  function wsUrl() {
    if (cfg.base) return cfg.base.replace(/^http/, 'ws') + cfg.wsPath;
    return (location.protocol === 'https:' ? 'wss://' : 'ws://') + location.host + cfg.wsPath;
  }

  function connect() {
    if (sock && sock.readyState === 1) return Promise.resolve(sock);
    if (opening) return opening;
    opening = (linkWait || Promise.resolve()).then(function () { return new Promise(function (resolve, reject) {   // AI(W906-WSLINK) 20260929 [W906] ST01-E3：先等上面的 linkWait
      var s;
      try { s = global.HT9045Link ? global.HT9045Link.open(wsUrl()) : new WebSocket(wsUrl()); }   // AI(W906-WSLINK) 20260929 [W906] ST01-E3：經外框的 hub（ht9045_link.js；Jimmy 20260929 機台 23 條撞 16 條上限）；沒有 HT9045Link 時跟以前一樣
      catch (e) { opening = null; reject(e); return; }
      var openTmo = setTimeout(function () {   // AI(W906-J2) 20260930: Jerry J-2 (FROM_JERRY, main 3a93a28f) -- a socket stuck in CONNECTING (TCP up, HTTP upgrade never finished) fires neither onopen nor onerror, so this promise, the shared `opening` and every cmd0() behind it never settled (the page showed 'START 送出中' forever, nothing in the Console). Give up after cfg.timeoutMs: detach and close the attempt, free `opening` (the next command opens a new socket) and reject like onerror does.
        if (sock === s) return;
        opening = null; s.onopen = s.onerror = s.onclose = s.onmessage = null;
        try { s.close(); } catch (e) {}
        reject(new Error('cannot open ' + wsUrl() + ': not open within ' + cfg.timeoutMs + 'ms'));
      }, cfg.timeoutMs);
      s.onopen = function () { clearTimeout(openTmo); sock = s; opening = null; resolve(s); };
      s.onerror = function () { clearTimeout(openTmo); opening = null; reject(new Error('cannot open ' + wsUrl())); };
      s.onclose = function () {
        sock = null; haveToken = false; if (tokenTimer) { clearTimeout(tokenTimer); tokenTimer = null; }   // AI(W906-TOKEN-IDLE) 20260926
        winSupported = null; winRefusal = '';   // Steven 20260918：重連後重新試一次
        //Steven 20260916 (W906-FW-TAGSUB): 斷線要說出來。留著 connected=true
        // 會讓看板頁面把上一代的殘值當成活的資料繼續顯示。
        tagStat.connected = false;
        // Fail everything still in flight rather than leaving callers hanging.
        Object.keys(pending).forEach(function (k) {
          pending[k].reject(new Error('socket closed before ack'));
          delete pending[k];
        });
      };
      s.onmessage = function (ev) {
        var m;
        try { m = JSON.parse(ev.data); } catch (e) { return; }
        if (!m) return;  if (m.type === 'link.token') { if (m.owner === false) { haveToken = false; if (tokenTimer) { clearTimeout(tokenTimer); tokenTimer = null; } } return; }   // AI(W906-WSLINK) 20260929 [W906] ST01-E3：hub 看到 not-operator（別的瀏覽器接管、或伺服器收回）會通知每一頁；清掉 haveToken，下一個自動動作重新 acquire（同 :170 的 not-operator）
        if (m.type === 'ack') {
          // AI(W906-MT-FIX1) 20260925：伺服器說「你不是操作員」＝權杖已經不在我們手上（閒置 10 分鐘被收回
          //   WebBridgeServer.cpp:1566-1569、或被別的連線拿走）。原本 haveToken 一直留 true，之後每一條
          //   motor.access／存檔都不再 acquire、全部被拒，直到重新整理 HMI。清掉它，下一條指令會重新 acquire。
          if (!m.ok && m.error === 'not-operator') haveToken = false;
          if (pending[m.id]) {
            var p = pending[m.id]; delete pending[m.id];
            if (m.ok) p.resolve(m); else p.reject(new Error(m.error || 'command refused'));
          }

<!-- preserved-content:end -->
```
