// ht9045_link.js -- one WebSocket per browser tab: the frame (background.html) owns the only
// connection to wb_serve, and every page inside it talks through the frame.
//
// AI(W906-WSLINK) 20260929 [W906] ST01-E3 (St01).  Jimmy 20260929 mail, measured on the machine 9/26:
// every iframe opened its own socket -> 23 connections against a server cap of 16; the extra ones
// queue forever, so Motor Test was dead with no error; the server diffed ~6,400 tags per connection
// (network thread 100 %, 0.7 s per request).  Design note + ST01-E's conditions:
// D:\AI_TempFile\st01_coord\ST01-M_ST01-E3.md, entries 20260929 13:4x and 13:3x (addendum).
//
// ---------------------------------------------------------------------------------------------------
// USE
//   var s = HT9045Link.open(url);   // instead of  new WebSocket(url)
//   The result is WebSocket-shaped (readyState, send, close, onopen/onmessage/onclose/onerror,
//   addEventListener), so a client keeps its code and changes only that one line.
//     * frame (top window, HT9045Link.startHub(url) was called): a local virtual socket on the hub;
//     * iframe whose parent runs a hub: a virtual socket over a MessageChannel to the hub;
//     * anything else (page opened on its own, an old frame that has no hub, a different URL such as
//       the C# simulator's ws://127.0.0.1:9045/ht9045-json/): a real WebSocket, exactly as before.
//   window.WebSocket is NOT patched (no invisible adapter).
//
// PORT MESSAGES (iframe <-> hub; `data` is the exact socket text frame, so client parsing is unchanged)
//   iframe -> hub   {ht9045link:'hello', v:1, url, name}  (window.postMessage, transfers port2)
//                   {k:'send', data}   {k:'bye'}   {k:'pong'}
//   hub -> iframe   {k:'welcome'}  {k:'open'}  {k:'msg', data}  {k:'close', code, reason}  {k:'ping'}
//                   {k:'suspend'}  {k:'resume'}   AI(W906-J11-RECONNECT-A) 20261005: the transport dropped /
//                   came back. NOT a close: the page keeps its VSock (readyState stays OPEN) and its
//                   half-typed input; it sees 'reconnecting' / 'reconnected' and then a fresh snapshot.
//
// HUB RULES
//   * every command is forwarded EXACTLY ONCE, never retried here (WebCmdGuardGlobal would call a retry
//     inside 400 ms "busy"; retries stay in each page's client, as today);
//   * a frame with a numeric `id` gets a hub-wide id; its `ack` is mapped back and goes ONLY to the asker;
//   * frames without an id (snapshot / patch / alarm / modal / query ...) fan out to every client;
//   * tag cache (snapshot + patches, null kept as a value) -> a late client gets a synthetic snapshot
//     carrying the last seq;
//   * operator token (one connection = one browser = one operator): control.acquire is answered locally
//     while the browser holds the token; otherwise ONE goes to the server and the others wait on its ack.
//     control.takeover (an operator press) always goes to the server -- it takes the token back from
//     another browser; from this connection it keeps the server's owner, so an in-browser hand-over
//     cancels nothing (one in flight at a time, the others wait on it).
//     control.release is forwarded only when the last holder releases.
//     A `not-operator` ack means the token is gone: every client gets {"type":"link.token","owner":false};
//   * the SCREEN holds the token (AI(W906-SCREEN-TOKEN) 20261001, RULINGS_20261001: Jimmy 1001 12:3x "1->A", the newest
//     screen wins): when the hub's real socket opens for the FIRST time it sends ONE control.takeover of its own, so a
//     window opened again (HMI-KEEP, the user, F5) owns the token at once and a stale window / tab left connected becomes
//     view-only (a later reconnect takes over only if this screen held the token when its socket dropped -- review R1; its
//     automatic acquires get control-held; an operator press there -- takeover -- still takes it back). Once held, the
//     screen keeps it (pseudo-holder 0): a page's release is answered locally and never reaches the server, so the
//     token is freed only when this socket closes or another connection takes it over. Every SCREEN_KEEP_MS (10 s; 60 s
//     SCREEN_REGAIN_MS while not held) the hub
//     sends control.acquire: it renews the server's 10-minute idle timer while held, reports a takeover by another
//     connection (control-held -> link.token owner:false), and regains the token when the other holder has gone
//     (acquire never takes it from a live holder). HT9045LinkConfig.screenTakeover:false restores the per-page rules;
//   * one connection per screen: an iframe whose parent HAS a hub for the URL never opens a direct socket on a hello
//     timeout (the frame is only busy, e.g. loading 68 windows); it asks again on a new channel and waits. Only a
//     parent without a hub ('nohub', or no HT9045Link at all) still makes the socket direct;
//   * dead-man: a client that goes away (bye, closed, 3 pings missed) while it HOLDS a jog (motor.access
//     jogP / jogN not yet released) makes the hub release that jog -- motor.stop with the jog's own button,
//     the server's DoJogRelease (that axis only). The hub NEVER sends a STOP: 20260930 M1 (St02-E2 review B)
//     -- the first version sent `button:'link.pageGone'` (= DoStop: StopAllMotor + CancelAllJobs) for any
//     page that ever sent motor.access, so reloading Motor Test / Teach during production stopped every axis.
//     A gone page's HOME / Loop are the server's page-table close edge; the connection-gone dead-man is still
//     the server's (HT9011UC_Cpp_V3.33.906.0\WebMotorAccess.cpp MotorAccessTick);
//   * window hidden (the frame's closeWin / minimizeWin only set display:none -- the page stays loaded, sends no
//     bye and keeps answering pings, so the dead-man above never fires on a normal close; Jimmy 20260929): the
//     frame calls HT9045Link.windowHidden(iframe.contentWindow) and the hub releases every jog that page still
//     holds (motor.stop with the jog's own button = that axis only). Its HOME / Loop are left alone (golden);
//     the page's own release of that same jog right after (HT_WIN -> releaseHeld) is not sent a second time: it
//     gets the hub release's ack, or is forwarded as a retry if the hub's release was refused (20260930 m11);
//   * relay probe per URL (20260930 m13): 'nohub' from the frame is remembered for that URL only; a hello timeout
//     makes only that socket direct (console warning) and the next open() asks the frame again;
//   * ui.windows.put is NOT touched here: the frame stays its only sender (background.html).
// ---------------------------------------------------------------------------------------------------
(function (g) {
  'use strict';
  if (g.HT9045Link) return;

  var CFG = g.HT9045LinkConfig || {};                    // tests shorten the timers (tools/webprobe/ws_link_selftest.cjs)
  var HELLO_WAIT_MS = CFG.helloWaitMs || 1500;          // no welcome -> no hub -> real socket
  var DUP_RELEASE_MS = CFG.dupReleaseMs || 2000;        // m11: a page's release of a jog the hub released < this ago = the same release
  var PING_MS = CFG.pingMs || 5000;                     // hub -> iframe liveness
  var PING_LOSS = CFG.pingLoss || 3;                    // missed pings before the client counts as gone
  var IDLE_CLOSE_MS = CFG.idleCloseMs || 1500;          // no client left -> close the real socket (grace for an iframe reload)
  var SCREEN_TAKE = CFG.screenTakeover !== false;       // AI(W906-SCREEN-TOKEN) 20261001: the screen takes the token on connect and keeps it
  var SCREEN_KEEP_MS = CFG.screenKeepMs || 10000;       // AI(W906-SCREEN-TOKEN) 20261001: keep-alive acquire while held (server idle forfeit is 10 min; a
                                                        //   takeover elsewhere is noticed within this -- review R1: a stale screen that still believes it holds
                                                        //   the token would take it back on a reconnect). A held acquire is not op-logged (wb_serve OpQuiet).
  var SCREEN_REGAIN_MS = CFG.screenRegainMs || 60000;   // AI(W906-SCREEN-TOKEN) 20261001: while NOT held, try to regain this often (a refusal is op-logged)
  //AI(W906-J11-RECONNECT) 20261002: 斷線自動重連（Jimmy 1002 09:1x 裁決＝A）
  var RECONNECT_MIN_MS  = CFG.reconnectMinMs  || 1000;  //   第一次等 1 s，之後加倍
  var RECONNECT_MAX_MS  = CFG.reconnectMaxMs  || 30000; //   上限 30 s（除錯時停很久也不會把伺服器敲爛）
  var RX_IDLE_MS        = CFG.rxIdleMs        || 20000; //   超過這麼久沒收到任何 server frame ⇒ 視同死線
                                                        //   ⚠ 要大於 wb_serve 最慢的推送間隔。節拍 500 ms、
                                                        //     api 快取與 publish 都掛在同一圈，正常每秒都有東西；
                                                        //     20 s 留給「停一下中斷點看個變數」不誤殺。
  var RX_IDLE_CHECK_MS  = CFG.rxIdleCheckMs   || 5000;  //   多久檢查一次
  //AI(W906-J11-RECONNECT-A) 20261005: Jimmy 1002 22:5x 特別交代的「連線逾時」。
  //  wb_serve 停在中斷點時，OS 會把 TCP 三次握手做完（listen backlog 還在），但應用層
  //  不會回 WebSocket 升級 ⇒ 瀏覽器的 socket 卡在 CONNECTING，onopen／onclose **都不會來**。
  //  hub 的 state 就永遠停在 'connecting'，_reSchedule 的 `state !== 'idle'` 讓它不再排重連，
  //  整條重連鏈就死在這裡。所以要自己計時收掉。
  var CONNECT_TIMEOUT_MS = CFG.connectTimeoutMs || 10000;
  var CONNECTING = 0, OPEN = 1, CLOSING = 2, CLOSED = 3;

  function now() { return Date.now(); }
  function parse(s) { try { return JSON.parse(s); } catch (e) { return null; } }
  function normUrl(u) { return String(u || '').replace(/\/+$/, ''); }
  function isTop() { try { return !g.parent || g.parent === g; } catch (e) { return true; } }

  // ------------------------------------------------------------------ virtual socket
  function VSock(url) {
    this.url = url;
    this.readyState = CONNECTING;
    this.protocol = '';
    this.onopen = this.onmessage = this.onclose = this.onerror = null;
    this._ls = { open: [], message: [], close: [], error: [], reconnecting: [], reconnected: [] };   // AI(W906-J11-RECONNECT-A) 20261005
    this._up = null;        // function(text): hand a frame to the transport
    this._down = null;      // function(code, reason): tell the transport we close
    this.via = 'pending';   // 'hub' | 'relay' | 'direct'
    this.linkState = 'live';   // AI(W906-J11-RECONNECT-A) 20261005: 'live' | 'reconnecting' —— readyState 之外的真實連線狀況，見 _suspend
  }
  VSock.CONNECTING = CONNECTING; VSock.OPEN = OPEN; VSock.CLOSING = CLOSING; VSock.CLOSED = CLOSED;
  VSock.prototype.addEventListener = function (t, f) { (this._ls[t] || (this._ls[t] = [])).push(f); };
  VSock.prototype.removeEventListener = function (t, f) {
    var a = this._ls[t]; if (!a) return; var i = a.indexOf(f); if (i >= 0) a.splice(i, 1);
  };
  VSock.prototype._emit = function (type, ev) {
    ev = ev || {}; ev.type = type; ev.target = this;
    var h = this['on' + type];
    if (typeof h === 'function') { try { h.call(this, ev); } catch (e) { setTimeout(function () { throw e; }, 0); } }
    var self = this;
    (this._ls[type] || []).slice().forEach(function (f) {
      try { f.call(self, ev); } catch (e) { setTimeout(function () { throw e; }, 0); }
    });
  };
  VSock.prototype.send = function (data) {
    if (this.readyState !== OPEN) throw new Error('InvalidStateError: HT9045Link socket is not open');
    this._up(String(data));
  };
  VSock.prototype.close = function (code, reason) {
    if (this.readyState === CLOSED || this.readyState === CLOSING) return;
    this.readyState = CLOSING;
    if (this._down) this._down(code, reason);
    var self = this;
    setTimeout(function () { self._closed(code || 1000, reason || '', true); }, 0);   // async, like a real WebSocket
  };
  VSock.prototype._opened = function () {
    if (this.readyState !== CONNECTING) return;
    this.readyState = OPEN;
    this.linkState = 'live';
    this._emit('open');
  };
  // ===========================================================================
  //  AI(W906-J11-RECONNECT-A) 20261005 —— 「重連中」狀態（Jimmy 1002 22:5x，RULINGS_20261002 第 23 條＝A）
  //
  //  階段一（654bf62f）讓 hub 的 socket 會自己接回來，但畫面還是死的：_allClosed 把
  //  this.clients 清空、每個 iframe 的 VSock 走 _closed() 進入終局 CLOSED，沒有任何
  //  路徑回到 OPEN ⇒ 只有整頁重新載入才會活。Jimmy 的裁決是「讓畫面活過重連」。
  //
  //  ⚠ 為什麼 readyState 在重連期間**維持 OPEN**，而不是退回 CONNECTING：
  //    頁面端大量程式碼是 `if (sock.readyState === OPEN) sock.send(...)`，而 VSock.send
  //    在非 OPEN 時會 **throw**。退回 CONNECTING 會讓按鈕的 click handler 直接丟例外，
  //    等於把「暫時沒網路」升級成「頁面壞掉」—— 那正是這張卡要消滅的症狀。
  //    所以對頁面維持 WebSocket 的契約（OPEN），另外用 `linkState` 表達真實狀況，
  //    並發 'reconnecting' / 'reconnected' 自訂事件給想顯示指示燈的頁面。
  //    斷線期間真的送出去的指令不會被吞掉，hub 會當場回 ok:false（見 Hub.fromClient）。
  // ===========================================================================
  VSock.prototype._suspend = function () {
    if (this.readyState !== OPEN || this.linkState === 'reconnecting') return;
    this.linkState = 'reconnecting';
    this._emit('reconnecting');
  };
  VSock.prototype._resume = function () {
    if (this.linkState !== 'reconnecting') return;
    this.linkState = 'live';
    this._emit('reconnected');
  };
  VSock.prototype._message = function (data) {
    if (this.readyState === OPEN) this._emit('message', { data: data });
  };
  VSock.prototype._closed = function (code, reason, clean) {
    if (this.readyState === CLOSED) return;
    var wasConnecting = this.readyState === CONNECTING;
    this.readyState = CLOSED;
    if (wasConnecting) this._emit('error', {});
    this._emit('close', { code: code || 1006, reason: reason || '', wasClean: !!clean });
  };

  // a VSock that simply wraps a real WebSocket (the fallback path)
  function bindDirect(vs, WS) {
    var ws;
    try { ws = new WS(vs.url); } catch (e) { setTimeout(function () { vs._closed(1006, String(e && e.message || e), false); }, 0); return; }
    vs.via = 'direct';
    vs._up = function (t) { ws.send(t); };
    vs._down = function (code, reason) { try { ws.close(code, reason); } catch (e) {} };
    ws.onopen = function () { vs._opened(); };
    ws.onmessage = function (ev) { vs._message(ev.data); };
    ws.onerror = function () { if (vs.readyState === OPEN) vs._emit('error', {}); };
    ws.onclose = function (ev) { vs._closed(ev && ev.code, ev && ev.reason, ev && ev.wasClean); };
  }

  // ------------------------------------------------------------------ hub
  function Hub(url, WS) {
    this.url = url;
    this.WS = WS;
    this.sock = null;
    this.state = 'idle';            // idle | connecting | open
    this.clients = {};              // cid -> client
    this.nextCid = 1;
    this.nextGid = 1;
    this.inflight = {};             // gid -> {cid, lid, kind}
    this.cache = {};
    this.snapMeta = null;           // last snapshot frame without data
    this.lastSeq = null;
    this.token = { held: false, holders: {}, pend: {} };   // pend: cmd name -> {gid, waiters:[{cid, lid}]}
    this.idleTimer = null;
    this.pingTimer = null;
    this.keepTimer = null;          // AI(W906-SCREEN-TOKEN) 20261001: screen keep-alive (SCREEN_KEEP_MS)
    this.takeNext = true;           // AI(W906-SCREEN-TOKEN) 20261001: review R1 / R3 -- the next socket open sends takeover (else acquire); see _screenStart
    this.lastHeldOkAt = 0;          //   time of the last ok answer that proved this screen holds the token (screen / keep / page acquire or takeover)
    this.reTimer = null;            // AI(W906-J11-RECONNECT) 20261002: backoff reconnect timer
    this.reDelay = 0;               //   current backoff, ms (0 = not backing off); reset to 0 only by a socket that really OPENED
    this.lastRxAt = 0;              //   time of the last frame FROM wb_serve (idle watchdog; 0 = nothing yet)
    this.wdTimer = null;            //   idle watchdog timer
    this.stats = { realOpens: 0, up: 0, down: 0, fanout: 0, localAcks: 0, releasesSent: 0, tagSkipped: 0, tagResync: 0,   // AI(W906-STREAM-F) 20260930: tagSkipped / tagResync
                   screenTakes: 0, screenRejoins: 0, screenTakeRefused: 0, screenKeeps: 0, screenLost: 0, screenRegained: 0,   // AI(W906-SCREEN-TOKEN) 20261001
                   reconnects: 0, reconnectGaveUp: 0, wdFires: 0,   // AI(W906-J11-RECONNECT) 20261002
                   upDropped: 0, connectTimeouts: 0 };   // AI(W906-J11-RECONNECT-A) 20261005
  }

  Hub.prototype.connect = function () {
    if (this.state !== 'idle') return;
    var self = this, ws, opened = false;   // opened: AI(W906-SCREEN-TOKEN) 20261001 review R3 -- a connect attempt that never opened says nothing about the token
    this.state = 'connecting';
    try { ws = new this.WS(this.url); } catch (e) { this.state = 'idle'; this._allClosed(1006, 'cannot open ' + this.url); return; }
    this.sock = ws;
    this.stats.realOpens++;
    //AI(W906-J11-RECONNECT-A) 20261005: 見 CONNECT_TIMEOUT_MS —— 卡在 CONNECTING 的那一路
    var cto = setTimeout(function () {
      cto = null;
      if (self.sock !== ws || self.state !== 'connecting') return;
      self.stats.connectTimeouts++;
      try { ws.close(4002, 'connect timeout'); } catch (e) {}
      //  有些瀏覽器對還沒開起來的 socket 呼叫 close() 不會觸發 onclose ⇒ 自己收尾，
      //  並把 sock 設成 null，之後那條 socket 若遲到才開，上面每個 handler 的
      //  `self.sock !== ws` 都會把它擋掉。
      if (self.sock === ws) { self.sock = null; self.state = 'idle'; self._allClosed(4002, 'connect timeout'); }
    }, CONNECT_TIMEOUT_MS);
    ws.onopen = function () {
      if (self.sock !== ws) return;
      if (cto) { clearTimeout(cto); cto = null; }
      self.state = 'open'; opened = true;
      //AI(W906-J11-RECONNECT) 20261002: 退避**只有在真的開起來**才歸零。connect() 連不上那一路
      //  （下面 catch）自己會呼叫 _allClosed，若在那裡歸零就變成固定間隔狂敲伺服器。
      self.reDelay = 0;
      self.lastRxAt = now();        //   看門狗從這一刻開始算（還沒收到任何 frame 不代表死掉）
      self._wdSchedule();
      self._screenStart();          // AI(W906-SCREEN-TOKEN) 20261001: FIRST on the socket, so every page command after it finds this connection the owner
      Object.keys(self.clients).forEach(function (cid) { self._clientOpen(self.clients[cid]); });
    };
    ws.onmessage = function (ev) { if (self.sock === ws) self._fromServer(String(ev.data)); };
    ws.onclose = function (ev) {
      if (self.sock !== ws) return;
      if (cto) { clearTimeout(cto); cto = null; }   // AI(W906-J11-RECONNECT-A) 20261005
      self.sock = null; self.state = 'idle';
      // AI(W906-SCREEN-TOKEN) 20261001: review R1 / R3 (read before _allClosed resets the token): an OPENED socket that drops while
      //   this screen holds the token -- proven by an answer within 1.5 keep periods (a throttled hidden tab's belief is older) --
      //   takes it back on the next open (a wb_serve restart). Failed attempts in between change nothing.
      if (opened && self.token.held && now() - self.lastHeldOkAt <= SCREEN_KEEP_MS * 1.5) self.takeNext = true;
      self._screenStop();           // AI(W906-SCREEN-TOKEN) 20261001
      self._allClosed(ev && ev.code, ev && ev.reason);
    };
    ws.onerror = function () {};
  };

  Hub.prototype._resetSession = function () {
    this.inflight = {};
    this.cache = {}; this.snapMeta = null; this.lastSeq = null;
    this.token = { held: false, holders: {}, pend: {} };
  };

  //AI(W906-J11-RECONNECT-A) 20261005: 傳輸層掉線**不再**是每個 client 的終局。
  //  以前這裡 `this.clients = {}` ＋ 逐一 `closed()`，等於告訴每個 iframe「你死了」；
  //  VSock._closed 的 CLOSED 是終局（:145 `if (this.readyState === CLOSED) return;`），
  //  所以即使 socket 之後重連成功也沒有對象可以通知，只有整頁重新載入才會活。
  //  現在：**保留註冊表**，把每個 client 標成未開啟並通知「重連中」；
  //  重連成功後 connect() 的 ws.onopen 既有的那一行
  //  （`Object.keys(self.clients).forEach(... _clientOpen ...)`）就會把它們一一喚醒，
  //  並補一份合成快照。真正的終局（bye／port 關閉／ping 掉光）仍然走 remove()＋closed()。
  Hub.prototype._allClosed = function (code, reason) {
    var self = this;
    this._resetSession();
    Object.keys(this.clients).forEach(function (cid) {
      var c = self.clients[cid];
      c.isOpen = false;                                   // 讓重連後的 _clientOpen 會再跑一次
      c.held = null;                                      // 伺服器那邊已經沒有這條連線了，持有的 jog 不再屬於我們
      if (c.suspended) { try { c.suspended(code || 1006, reason || ''); } catch (e) {} }
    });
    this._pingSchedule();
    this._wdStop();                 // AI(W906-J11-RECONNECT) 20261002
    this._reSchedule(this.clients); // AI(W906-J11-RECONNECT) 20261002: 見下方（A 之後註冊表還在，直接傳它）
  };

  // ===========================================================================
  //  AI(W906-J11-RECONNECT) 20261002 —— 斷線自動重連（Jimmy 1002 09:1x 裁決＝A，FROM_JERRY J-11）
  //
  //  以前：ws.onclose -> _allClosed 只做清理，state 留在 'idle' 就停住，**沒有人再連**。
  //  唯一會重連的路徑是 Hub.add()（`else this.connect()`）—— 所以操作員 Ctrl+Shift+R
  //  產生新 client 才會好。這就是 1002 實測的那條鏈：停在中斷點 → socket 掉 →
  //  10 秒後 page-table 判定「沒有 HMI 畫面」正常 STOP ＋ MES16441 → 之後告警框按了沒反應
  //  （modal.answer 走 WebSocket）→ 只能重新整理。
  //
  //  權杖不在這裡處理：ws.onclose 已經在 _allClosed 清 token **之前**決定好 takeNext
  //  （AI(W906-SCREEN-TOKEN) 20261001，:201 "read before _allClosed resets the token"），
  //  重連成功後 _screenStart 會照那個決定送 takeover 或 acquire ——
  //  正是 Jimmy 裁決的「重連的畫面就是最新的畫面 → 照 RULINGS_20261001 第 31 條 takeover」。
  //  C++ 不用改：停機對話框伺服器對每條新連線本來就補送（NB2 R124 實測）。
  // ===========================================================================
  Hub.prototype._reSchedule = function (hadClients) {
    var self = this;
    if (this.reTimer) return;                                   // 已經排了
    //  沒有任何 iframe 在等資料就不要重連：這與 remove() 的 IDLE_CLOSE_MS（沒有 client 就
    //  關 socket）是同一個語意，不然兩邊會互相打架。下一個 Hub.add() 會自己 connect()。
    if (!hadClients || !Object.keys(hadClients).length) { this.stats.reconnectGaveUp++; return; }
    this.reDelay = this.reDelay ? Math.min(this.reDelay * 2, RECONNECT_MAX_MS) : RECONNECT_MIN_MS;
    var wait = this.reDelay;
    this.reTimer = setTimeout(function () {
      self.reTimer = null;
      if (self.state !== 'idle') return;                        // 期間已經有人連上了（例如 Hub.add）
      //AI(W906-J11-RECONNECT-A) 20261005: A 之後註冊表活著，但斷線這段期間 iframe 仍可能
      //  全部關掉（bye／ping 掉光 → remove()）。那時 this.sock 已經是 null，remove() 的
      //  IDLE_CLOSE_MS 分支（`&& this.sock`）不會排 —— 若還是重連就留下一條沒人用的連線。
      if (!Object.keys(self.clients).length) { self.stats.reconnectGaveUp++; return; }
      //  ⚠ 階段一（654bf62f）時這裡**不可以**看 self.clients —— 當時 _allClosed 會先把它
      //    清成 {}（第一版就是這樣寫的，結果每次都走 reconnectGaveUp、一次都沒重連，
      //    Jerry 實測「中斷 30 秒後繼續，畫面還是死的」），所以證據只能是傳進來的 hadClients。
      //    AI(W906-J11-RECONNECT-A) 20261005：A 之後 _allClosed **不再清空**註冊表，
      //    hadClients 就是 this.clients 本身；改成在上面多一道「現在還有沒有 client」的檢查。
      self.stats.reconnects++;
      self.connect();                                           // 失敗會再走 _allClosed，退避繼續加倍
    }, wait);
  };

  //  閒置看門狗：wb_serve 停在中斷點時是「活著但不回應」—— TCP 沒有關，onclose 不會觸發，
  //  瀏覽器端的 socket 可能一直是 OPEN。所以光靠 onclose 救不了 1002 實測的那個情境。
  //  ⚠ 這條是 hub ↔ wb_serve；既有的 PING_MS(5000) 是 hub ↔ iframe，兩者不同。
  Hub.prototype._wdSchedule = function () {
    var self = this;
    this._wdStop();
    this.wdTimer = setTimeout(function () {
      self.wdTimer = null;
      if (self.state !== 'open' || !self.sock) return;
      if (now() - self.lastRxAt < RX_IDLE_MS) { self._wdSchedule(); return; }   // 還有在收，繼續看
      self.stats.wdFires++;
      try { self.sock.close(4001, 'rx idle'); } catch (e) {}                    // 走正常的 onclose -> _allClosed -> 重連
    }, RX_IDLE_CHECK_MS);
  };

  Hub.prototype._wdStop = function () {
    if (this.wdTimer) { clearTimeout(this.wdTimer); this.wdTimer = null; }
  };

  // client = {id, name, deliver(text), opened(), closed(code, reason), port?: bool, win?, missed, held:{button: {source, motors}}, source}
  Hub.prototype.add = function (c) {
    c.id = this.nextCid++;
    c.missed = 0; c.held = null; c.source = '';  c.tagsOff = winOff(c.win); c.snapSent = false;   // AI(W906-STREAM-F) 20260930: a closed window's page gets no snapshot / patch (windowState below the Hub methods)
    this.clients[c.id] = c;
    if (this.idleTimer) { clearTimeout(this.idleTimer); this.idleTimer = null; }
    if (this.state === 'open') this._clientOpen(c);
    else this.connect();
    this._pingSchedule();
    return c.id;
  };

  Hub.prototype._clientOpen = function (c) {
    if (c.isOpen) return;
    c.isOpen = true;
    //AI(W906-J11-RECONNECT-A) 20261005: 第一次開 = opened()（頁面收到 WebSocket 的 'open'）；
    //  斷線後再回來 = resumed()，頁面**不會**再收到第二次 'open'（它從來沒收過 'close'），
    //  只會收到 'reconnected' 與下面那份新的合成快照。
    if (c.wasOpen && c.resumed) { try { c.resumed(); } catch (e) {} }
    else { c.opened(); c.wasOpen = true; }
    if (this.snapMeta) {
      var snap = {};
      Object.keys(this.snapMeta).forEach(function (k) { snap[k] = this.snapMeta[k]; }, this);
      snap.type = 'snapshot'; snap.data = c.tagsOff ? bootTags(this.cache) : this.cache; snap.synthetic = true;  c.snapSent = true;   // AI(W906-STREAM-F) 20260930: a closed window's page: BOOT_TAGS only
      if (this.lastSeq !== null) snap.seq = this.lastSeq;
      c.deliver(JSON.stringify(snap));
    }
  };

  Hub.prototype.remove = function (cid, why) {
    var c = this.clients[cid];
    if (!c) return;
    delete this.clients[cid];
    var self = this;
    Object.keys(this.inflight).forEach(function (gid) { if (self.inflight[gid].cid === +cid) self.inflight[gid].cid = 0; });
    Object.keys(this.token.pend).forEach(function (k) { var P = self.token.pend[k]; P.waiters = P.waiters.filter(function (w) { return w.cid !== +cid; }); });
    var wasHolder = !!this.token.holders[cid];
    delete this.token.holders[cid];
    // AI(W906-WSLINK) 20260930 ST01-E3: M1 (St02-E2 review B): the page is gone (bye / port closed / pings lost) ->
    //   release only the jogs it still holds, each with its own button (the server's DoJogRelease, that axis only).
    //   NEVER a STOP: the old `button:'link.pageGone'` was DoStop = GoldenStopAll + CancelAllJobs + Stop1203All,
    //   armed by any motor.access (formShow / query too), so reloading Motor Test / Teach mid-production stopped
    //   every 1203 axis. Golden TfMotorTest::FormClose stops nothing; the page's HOME / Loop are the server's
    //   page-table close edge (Jimmy's (b)).
    this._releaseHeld(c, 'link.pageGone:' + String(why || 'gone'));
    if (wasHolder && this.token.held && !Object.keys(this.token.holders).length) this._releaseToServer();
    if (!Object.keys(this.clients).length && this.sock) {
      var s = this.sock;
      this.idleTimer = setTimeout(function () {
        self.idleTimer = null;
        if (!Object.keys(self.clients).length && self.sock === s) { try { s.close(1000, 'no client'); } catch (e) {} }
      }, IDLE_CLOSE_MS);
    }
    this._pingSchedule();
  };

  // The frame hid a window (closeWin / minimizeWin: display:none, the iframe stays loaded, no bye, pings go on).
  // A jog its page still holds can no longer be released there (pointerup does not reach a hidden frame; HW.teach.html
  // has no HT_WIN release) -> release each held jog now: motor.stop with that jog's own button = the server's
  // DoJogRelease (that axis only). Not the page-gone STOP: a hidden page's HOME / Loop keep going, like golden
  // (minimize is not FormClose; the close itself is the server's page-table close edge).
  Hub.prototype.windowHidden = function (win, why) {
    if (!win) return 0;
    var self = this, n = 0;
    Object.keys(this.clients).forEach(function (cid) {
      var c = self.clients[cid];
      if (c.win === win) n += self._releaseHeld(c, 'link.windowHidden:' + String(why || ''));
    });
    return n;
  };

  // One jog release per jog the client still holds: motor.stop with that jog's own button, action 'stop' -- the
  // server's DoJogRelease (that axis only). The only kind of stop the hub ever sends on its own (windowHidden /
  // page gone); it never sends a STOP (btnStop / a non-jog button = DoStop = StopAllMotor).
  // AI(W906-WSLINK) 20260930 E-017 m11 (St02-E2 review B): each release is remembered in c.hubRel[button] until its
  //   ack, so the page's own release of the same jog right after (Motor Test / Teach release on HT_WIN too) is not
  //   sent a second time: it waits for this ack and gets it (ok), or -- if the hub's release was refused -- it is
  //   forwarded as a retry. Nothing is delayed: the hub's release has already gone out.
  Hub.prototype._releaseHeld = function (c, reason) {
    if (!c.held || this.state !== 'open') { c.held = {}; return 0; }
    var self = this, n = 0;
    if (!c.hubRel) c.hubRel = {};
    Object.keys(c.held).forEach(function (button) {
      var hd = c.held[button], gid = self.nextGid++;
      self.inflight[gid] = { cid: 0, lid: 0, kind: 'hubrelease', owner: c.id, button: button };
      c.hubRel[button] = { gid: gid, at: now(), done: false, ok: false, ack: null, waiters: [] };
      self.stats.releasesSent++;
      self._up(JSON.stringify({ type: 'cmd', id: gid, cmd: 'motor.stop', tag: hd.motors[0] || '',
        value: JSON.stringify({ source: hd.source || c.source || 'ht9045_link', action: 'stop', motors: hd.motors,
                                button: button, kind: 'control', reason: reason }) }));
      n++;
    });
    c.held = {};
    return n;
  };

  Hub.prototype._pingSchedule = function () {
    var self = this;
    var anyPort = Object.keys(this.clients).some(function (cid) { return self.clients[cid].port; });
    if (anyPort && !this.pingTimer) {
      this.pingTimer = setInterval(function () {
        Object.keys(self.clients).forEach(function (cid) {
          var c = self.clients[cid];
          if (!c.port) return;
          if (++c.missed > PING_LOSS) { c.closed(1001, 'ping lost'); self.remove(cid, 'ping lost'); return; }
          c.ping();
        });
      }, PING_MS);
    } else if (!anyPort && this.pingTimer) {
      clearInterval(this.pingTimer); this.pingTimer = null;
    }
  };

  //AI(W906-J11-RECONNECT-A2) 20261005: 「這條線現在真的能送嗎」的單一判準。
  //  ⚠ 只看 this.state 不夠 —— Jerry 1005 用 F6（暫停 wb_serve，不是停止）實測抓到：
  //    伺服器凍住時 TCP 不會斷，所以 hub.state 一直是 'open'、this.sock 也非 null；
  //    閒置看門狗在 20 s 時呼叫 sock.close(4001)，socket 進入 CLOSING，但 onclose 要等
  //    對方回關閉握手 —— 對方凍住，所以那個空窗期很長。期間 send() 會丟
  //    「WebSocket is already in CLOSING or CLOSED state」。所以必須看 socket 自己的 readyState。
  Hub.prototype._linkUp = function () {
    return this.state === 'open' && !!this.sock && this.sock.readyState === OPEN;
  };

  Hub.prototype._up = function (text) {
    //AI(W906-J11-RECONNECT-A) 20261005: 斷線期間 this.sock 是 null。A 之前不會走到這裡
    //  （client 都死了），A 之後要自己擋，否則是 TypeError: sock is null。
    if (!this._linkUp()) { this.stats.upDropped++; return false; }
    this.stats.up++;
    try { this.sock.send(text); } catch (e) { this.stats.upDropped++; return false; }   // AI(W906-J11-RECONNECT-A2) 20261005: readyState 與 send 之間仍有競態
    return true;
  };

  Hub.prototype._localAck = function (c, lid, ok, error, extra) {
    this.stats.localAcks++;
    var a = { type: 'ack', id: lid, ok: !!ok, error: error || '' , link: 'local' };
    if (extra) Object.keys(extra).forEach(function (k) { a[k] = extra[k]; });
    c.deliver(JSON.stringify(a));
  };

  Hub.prototype._releaseToServer = function () {
    if (this.state !== 'open') { this.token.held = false; return; }
    var gid = this.nextGid++;
    this.inflight[gid] = { cid: 0, lid: 0, kind: 'release' };
    this.token.held = false;
    this._up(JSON.stringify({ type: 'cmd', id: gid, cmd: 'control.release' }));
  };

  // AI(W906-SCREEN-TOKEN) 20261001: the screen's own token traffic (header: "the SCREEN holds the token").
  //   kind 'screen' = the one control.takeover when the real socket opens; kind 'keep' = the control.acquire every
  //   SCREEN_KEEP_MS. Both are answered on the server's socket thread (WebBridgeServer.cpp control.acquire branch), in
  //   order with the page commands on this one socket. The takeover goes first, so a page's acquire right behind it
  //   finds this connection the owner. Pseudo-holder 0 = the screen itself: with it in T.holders a page's release (or
  //   a page going away) is answered locally and never releases the server's token.
  //   Review R1 / R3 (20261001): takeover ONLY while takeNext is set: from the hub's start (this screen was just opened /
  //   reloaded = the newest) until a screen takeover is answered ok, and again after an opened socket dropped while this
  //   screen held the token with a fresh proof (a wb_serve restart: give it back to the screen that had it).
  //   Any other reconnect sends control.acquire: after a restart a stale tab that reconnects last must not take the token
  //   from the newest screen (control-held leaves it view-only; its keep-alive regains the token only if nobody holds it).
  Hub.prototype._screenSend = function (name, kind) {
    if (this.state !== 'open' || !this.sock) return;
    var gid = this.nextGid++;
    this.inflight[gid] = { cid: 0, lid: 0, kind: kind, name: name };
    this._up(JSON.stringify({ type: 'cmd', id: gid, cmd: name }));
  };
  Hub.prototype._screenStart = function () {
    if (!SCREEN_TAKE) return;
    this._screenStop();
    var take = this.takeNext;
    if (take) this.stats.screenTakes++; else this.stats.screenRejoins++;
    this._screenSend(take ? 'control.takeover' : 'control.acquire', 'screen');
    var self = this;
    this.lastRegainAt = 0;
    this.keepTimer = setInterval(function () {
      if (self.state !== 'open' || self.keepBusy) return;
      if (!self.token.held) {                                          // view-only: ask now and then, never take
        var t = now();
        if (t - self.lastRegainAt < SCREEN_REGAIN_MS) return;
        self.lastRegainAt = t;
      }
      self.keepBusy = true;
      self.stats.screenKeeps++;
      self._screenSend('control.acquire', 'keep');
    }, SCREEN_KEEP_MS);
  };
  Hub.prototype._screenStop = function () {
    if (this.keepTimer) { clearInterval(this.keepTimer); this.keepTimer = null; }
    this.keepBusy = false;
  };
  Hub.prototype._screenAck = function (r, m) {
    var T = this.token;
    if (r.kind === 'keep') this.keepBusy = false;
    if (m.ok) {
      if (r.kind === 'keep' && !T.held) this.stats.screenRegained++;   // the other holder went away: this screen operates again
      if (r.kind === 'screen' && r.name === 'control.takeover') this.takeNext = false;   // spent only when it worked (review R3)
      this.lastHeldOkAt = now();
      T.held = true;
      T.holders[0] = true;
      return;
    }
    if (r.kind === 'screen') {
      if (r.name === 'control.acquire') return;                        // a reconnect while another connection holds it: view-only, the keep-alive watches
      this.stats.screenTakeRefused++;                                  // a read-only bridge / no command queue: nothing to hold
      try { if (g.console && g.console.warn) g.console.warn('[HT9045Link] screen control.takeover refused: ' + (m.error || '?')); } catch (ew) {}
      return;
    }
    // keep refused (control-held): another connection took the token over -- view-only here until an operator press
    //   (takeover) or that connection goes; the pages are told once (link.token owner:false), as for not-operator
    if (T.held || Object.keys(T.holders).length) { this.stats.screenLost++; this._tokenLost(); }
  };

  // a frame from one client
  Hub.prototype.fromClient = function (cid, text) {
    var c = this.clients[cid];
    if (!c) return;
    //AI(W906-J11-RECONNECT-A) 20261005: A 之後 client 活過斷線 ⇒ 斷線期間**真的會**有指令送進來
    //  （以前 client 當場死掉，所以沒人會送，這裡只是 `return` 靜默丟掉）。
    //  現在當場回 ok:false，讓頁面知道失敗、操作員自己重按。
    //  ⛔ 絕不排隊重送：把幾秒前的 STOP／JOG 補送到會動的機台上，比誠實失敗危險得多。
    if (!this._linkUp()) {
      var mq = parse(text);
      if (mq && typeof mq === 'object' && typeof mq.id === 'number')
        this._localAck(c, mq.id, false, 'link.reconnecting');
      return;
    }
    var m = parse(text);
    if (!m || typeof m !== 'object' || typeof m.id !== 'number') { this._up(text); return; }
    var lid = m.id, name = m.type === 'cmd' ? String(m.cmd || '') : '';
    var T = this.token;
    if (name === 'control.acquire' || name === 'control.takeover') {
      // acquire (automatic) is answered here while this browser holds the token.  takeover (an operator press) always
      // goes to the server: it must take the token back from ANOTHER browser, which this hub cannot see.  From this same
      // connection the server keeps the owner, so a hand-over between pages of this browser changes nothing there -- no
      // cancel (tools\wb_serve.cpp W906_OwnerHeldSince: "Same-page takeovers keep the owner and change nothing").
      // control.* is answered on the server's socket thread, never through WebCmdGuard (WebBridgeServer.cpp:1388).
      if (T.held && name === 'control.acquire') { T.holders[cid] = true; this._localAck(c, lid, true); return; }
      var P = T.pend[name];
      if (P) { P.waiters.push({ cid: +cid, lid: lid }); return; }    // one in flight per kind; the others wait on its ack
      var ga = this.nextGid++;
      T.pend[name] = { gid: ga, waiters: [{ cid: +cid, lid: lid }] };
      this.inflight[ga] = { cid: +cid, lid: lid, kind: 'acquire', name: name };
      m.id = ga;
      this._up(JSON.stringify(m));
      return;
    }
    if (name === 'control.release') {
      delete T.holders[cid];
      if (!T.held || Object.keys(T.holders).length) { this._localAck(c, lid, true); return; }
      var gr = this.nextGid++;
      this.inflight[gr] = { cid: +cid, lid: lid, kind: 'release' };
      T.held = false;
      m.id = gr;
      this._up(JSON.stringify(m));
      return;
    }
    if (name === 'motor.access') {
      var v = typeof m.value === 'string' ? parse(m.value) : m.value;
      if (v && typeof v === 'object') {
        if (typeof v.source === 'string') c.source = v.source;
        // a press-and-hold jog (motor-access.json release:'stop' = jogP / jogN): held until the page sends its release.
        //   AI(W906-WSLINK) 20260930 M1: this is the ONLY thing that arms the hub's release -- formShow / formClose /
        //   query / moves / HOME / Loop no longer record "touched motors" (the old c.motors fed the page-gone STOP)
        if ((v.action === 'jogP' || v.action === 'jogN') && typeof v.button === 'string') {
          if (!c.held) c.held = {};
          c.held[v.button] = { source: typeof v.source === 'string' ? v.source : '', motors: Array.isArray(v.motors) ? v.motors.slice() : [] };
          if (c.hubRel) delete c.hubRel[v.button];     // a new press: its release is a new release (m11)
        }
      }
    }
    if (name === 'motor.stop') {
      var sv = typeof m.value === 'string' ? parse(m.value) : m.value;
      var sb = sv && typeof sv === 'object' && typeof sv.button === 'string' ? sv.button : '';
      // m11: the page releasing a jog the hub has just released for it (same button, no new press since): don't send
      //   it again -- answer with the hub's result, or forward it as a retry if the hub's release was refused
      var hr = c.hubRel && c.hubRel[sb];
      if (hr && !(c.held && c.held[sb]) && now() - hr.at < DUP_RELEASE_MS) {
        if (!hr.done) { hr.waiters.push({ lid: lid, m: m }); return; }
        if (hr.ok) { this._localAck(c, lid, true, '', this._dupExtra(hr)); return; }
      }
      if (c.held) {
        // the page's own release of a jog button ends that hold only; a real STOP (btnStop: golden StopAllMotor) ends every hold
        if (c.held[sb]) delete c.held[sb];
        else if (!/jog/i.test(sb)) c.held = {};
      }
    }
    this._forward(cid, lid, m);
  };

  Hub.prototype._forward = function (cid, lid, m) {
    var gid = this.nextGid++;
    this.inflight[gid] = { cid: +cid, lid: lid, kind: 'cmd' };
    m.id = gid;
    //AI(W906-J11-RECONNECT-A2) 20261005: 送不出去就**當場回覆並撤掉 inflight**。
    //  以前（A 的第一版）只是讓 _up 回 false，那筆 inflight 變成孤兒，頁面要空等到自己的
    //  15 秒逾時 —— Jerry 1005 的 F6 實測 log 裡的 `vacuum.get: no ack within 15000ms`
    //  與 `ui.windows.put: no ack within 15000ms` 就是這個。
    if (!this._up(JSON.stringify(m))) {   // exactly once; no retry here
      delete this.inflight[gid];
      var c = this.clients[cid];
      if (c && lid) this._localAck(c, lid, false, 'link.reconnecting');
    }
  };

  Hub.prototype._dupExtra = function (hr) {
    var x = { state: 'done', result: 'ok', message: 'jog already released by the frame hub (window hidden / page gone); not sent twice',
              linkDup: true };
    if (hr.ack && typeof hr.ack === 'object') Object.keys(hr.ack).forEach(function (k) {
      if (k !== 'type' && k !== 'id' && k !== 'ok' && k !== 'error' && !(k in x)) x[k] = hr.ack[k];
    });
    return x;
  };

  // the server's ack for a hub release (m11): settle the page releases that waited on it
  Hub.prototype._hubReleaseAck = function (r, m) {
    var c = this.clients[r.owner], hr = c && c.hubRel && c.hubRel[r.button];
    if (!hr || hr.gid !== m.id) return;                // the client is gone, or a newer press / release replaced it
    hr.done = true; hr.ok = !!m.ok; hr.ack = m;
    var ws = hr.waiters, self = this;
    hr.waiters = [];
    if (!hr.ok) delete c.hubRel[r.button];             // refused: the page's releases go to the server themselves
    ws.forEach(function (w) {
      if (hr.ok) self._localAck(c, w.lid, true, '', self._dupExtra(hr));
      else self._forward(c.id, w.lid, w.m);
    });
  };

  Hub.prototype._tokenLost = function () {
    var T = this.token, self = this;
    var was = T.held || Object.keys(T.holders).length;
    T.held = false; T.holders = {};
    if (!was) return;
    var f = JSON.stringify({ type: 'link.token', owner: false });
    Object.keys(this.clients).forEach(function (cid) { self.clients[cid].deliver(f); });
  };

  Hub.prototype._fromServer = function (text) {
    this.stats.down++;
    this.lastRxAt = now();          // AI(W906-J11-RECONNECT) 20261002: 任何一個 server frame 都算「還活著」（看門狗看這個）
    var m = parse(text), self = this;
    if (m && m.type === 'ack' && typeof m.id === 'number') {
      var r = this.inflight[m.id];
      if (!r) return;
      delete this.inflight[m.id];
      if (r.kind === 'acquire') {
        var T = this.token, P = T.pend[r.name], ws = P && P.gid === m.id ? P.waiters : [{ cid: r.cid, lid: r.lid }];
        if (P && P.gid === m.id) delete T.pend[r.name];
        if (m.ok) {
          T.held = true;
          if (SCREEN_TAKE) { T.holders[0] = true; this.lastHeldOkAt = now(); }   // AI(W906-SCREEN-TOKEN) 20261001: regained by a page (e.g. an operator press after a takeover elsewhere) -> the screen keeps it again
          ws.forEach(function (w) { if (self.clients[w.cid]) T.holders[w.cid] = true; });
        }
        ws.forEach(function (w) {
          var cw = self.clients[w.cid];
          if (!cw) return;
          var a = {}; Object.keys(m).forEach(function (k) { a[k] = m[k]; }); a.id = w.lid;
          cw.deliver(JSON.stringify(a));
        });
        return;
      }
      if (r.kind === 'hubrelease') { this._hubReleaseAck(r, m); return; }
      if (r.kind === 'screen' || r.kind === 'keep') { this._screenAck(r, m); return; }   // AI(W906-SCREEN-TOKEN) 20261001
      if (!m.ok && m.error === 'not-operator') this._tokenLost();
      if (!r.cid) return;
      var c = this.clients[r.cid];
      if (!c) return;
      m.id = r.lid;
      c.deliver(JSON.stringify(m));
      return;
    }
    if (m && (m.type === 'snapshot' || m.type === 'patch') && m.data && typeof m.data === 'object') {
      if (m.type === 'snapshot') {
        this.cache = {};
        this.snapMeta = {};
        Object.keys(m).forEach(function (k) { if (k !== 'data') self.snapMeta[k] = m[k]; });
      }
      var d = m.data;
      Object.keys(d).forEach(function (k) { self.cache[k] = d[k]; });
      if (typeof m.seq === 'number') this.lastSeq = m.seq;
    }
    this.stats.fanout++;  var isTag = !!(m && (m.type === 'snapshot' || m.type === 'patch')), isSnap = isTag && m.type === 'snapshot';   // AI(W906-STREAM-F) 20260930
    Object.keys(this.clients).forEach(function (cid) { var c = self.clients[cid]; if (isTag && c.tagsOff) { if (isSnap && !c.snapSent) { c.snapSent = true; c.deliver(bootFrame(m)); } else self.stats.tagSkipped++; return; } if (isSnap) c.snapSent = true; c.deliver(text); });
  };

  Hub.prototype.status = function () {
    var self = this;
    return { url: this.url, state: this.state, clients: Object.keys(this.clients).map(function (cid) {
               var c = self.clients[cid]; return { id: +cid, name: c.name, port: !!c.port, holder: !!self.token.holders[cid] }; }),
             tokenHeld: this.token.held, screen: SCREEN_TAKE, screenHeld: !!this.token.holders[0], takeNext: this.takeNext,   // AI(W906-SCREEN-TOKEN) 20261001
             inflight: Object.keys(this.inflight).length, cachedTags: Object.keys(this.cache).length,  tagsOff: Object.keys(this.clients).filter(function (cid) { return self.clients[cid].tagsOff; }).length,   // AI(W906-STREAM-F) 20260930
             stats: this.stats };
  };

  // ------------------------------------------------------------------ module state
  var hubs = {};           // normUrl -> Hub   (top window only)
  // iframe: per URL -- true = the parent's hub answered, 'nohub' = the parent said it has no hub for that URL (sticky
  //   for that URL only), absent = not asked yet or the last hello timed out. AI(W906-WSLINK) 20260930 E-017 m13
  //   (St02-E2 review B): this was ONE global flag, and one 1.5 s hello timeout (a busy frame) or a 'nohub' for
  //   another URL (e.g. the simulator's) sent every later socket of the iframe straight to the server, silently
  //   bringing the 16-connection problem back. A timeout now only makes THIS socket direct, warns, and the next
  //   open() asks again.
  var relayState = {};
  var relayMisses = 0;     // hello timeouts so far (status())
  var mySocks = [];        // iframe: virtual sockets that ride the relay (for bye on pagehide)
  var looseTries = [];     // iframe: hello channels not (or no longer) bound -- bye on pagehide too (AI(W906-SCREEN-TOKEN) 20261001, review R2)
  // AI(W906-SCREEN-TOKEN) 20261001: review R2 -- a hello channel we give up on gets a bye BEFORE it goes: if the frame handles that
  //   hello later, hub.add() runs and the queued bye removes the client at once (no ghost client fed every patch until ping
  //   loss, ~20 s). The port is closed only later, so the queued bye is never lost with it.
  function dropTry(p) {
    looseTries = looseTries.filter(function (x) { return x !== p; });
    try { p.postMessage({ k: 'bye' }); } catch (e) {}
    setTimeout(function () { try { p.close(); } catch (e2) {} }, 30000);
  }

  function WSClass() { return g.WebSocket; }

  function attachLocal(hub, vs, name) {
    // Callbacks go through setTimeout(0), like a real socket's events: the caller sets onopen / onmessage
    // AFTER open() returns, and the hub may answer inside open() (already connected). FIFO order is kept.
    function later(f) { setTimeout(f, 0); }
    var c = {
      name: name || 'frame', port: false,
      deliver: function (t) { later(function () { vs._message(t); }); },
      opened: function () { later(function () { vs._opened(); }); },
      closed: function (code, reason) { later(function () { vs._closed(code, reason, false); }); },
      suspended: function () { later(function () { vs._suspend(); }); },   // AI(W906-J11-RECONNECT-A) 20261005
      resumed: function () { later(function () { vs._resume(); }); },      // AI(W906-J11-RECONNECT-A) 20261005
    };
    var cid = hub.add(c);
    vs.via = 'hub';
    vs._up = function (t) { hub.fromClient(cid, t); };
    vs._down = function () { hub.remove(cid, 'closed'); };
  }

  // top window: accept iframes' hellos
  function onHello(ev) {
    var d = ev.data;
    if (!d || d.ht9045link !== 'hello' || !ev.ports || !ev.ports[0]) return;
    try { if (ev.source && ev.source.parent !== g) return; } catch (e0) { return; }   // only our own iframes
    var port = ev.ports[0];
    var hub = hubs[normUrl(d.url)];
    if (!hub) { try { port.postMessage({ k: 'nohub' }); } catch (e) {} return; }
    var c = {
      name: String(d.name || 'iframe'), port: true,
      win: ev.source || null,        // the iframe's WindowProxy: windowHidden(iframe.contentWindow) finds its clients
      deliver: function (t) { port.postMessage({ k: 'msg', data: t }); },
      opened: function () { port.postMessage({ k: 'open' }); },
      closed: function (code, reason) { try { port.postMessage({ k: 'close', code: code, reason: reason }); } catch (e) {} },
      suspended: function () { try { port.postMessage({ k: 'suspend' }); } catch (e) {} },   // AI(W906-J11-RECONNECT-A) 20261005
      resumed: function () { try { port.postMessage({ k: 'resume' }); } catch (e) {} },      // AI(W906-J11-RECONNECT-A) 20261005
      ping: function () { port.postMessage({ k: 'ping' }); },
    };
    // welcome FIRST: hub.add() may answer 'open' at once (already connected), and the iframe must have
    // its send path bound (on welcome) before it is told the socket is open.
    port.postMessage({ k: 'welcome' });
    var cid = hub.add(c);
    port.onmessage = function (e) {
      var x = e.data || {};
      if (x.k === 'send') hub.fromClient(cid, String(x.data));
      else if (x.k === 'pong') { if (hub.clients[cid]) hub.clients[cid].missed = 0; }
      else if (x.k === 'bye') hub.remove(cid, 'bye');
    };
  }

  function startHub(url, opts) {
    var key = normUrl(url);
    if (!hubs[key]) {
      hubs[key] = new Hub(url, (opts && opts.WebSocket) || WSClass());
      if (!startHub.listening) { g.addEventListener('message', onHello); startHub.listening = true; }
    }
    return hubs[key];
  }

  // AI(W906-SCREEN-TOKEN) 20261001: does the parent (the frame) run a hub for this URL? Same-origin read of its
  //   HT9045Link.hub(url); false for a cross-origin parent, a parent without HT9045Link, or one without that hub.
  function parentHasHub(url) {
    try { var L = g.parent && g.parent !== g && g.parent.HT9045Link; return !!(L && typeof L.hub === 'function' && L.hub(url)); }
    catch (e) { return false; }
  }

  // iframe: one MessageChannel per virtual socket (one more per re-asked hello, see late())
  function attachRelay(vs, name) {
    var MC = g.MessageChannel;
    if (!MC) { bindDirect(vs, WSClass()); return; }
    var key = normUrl(vs.url), bound = null, tries = [], timer = null, warned = false, wait = HELLO_WAIT_MS;
    function closeTries() { tries.forEach(dropTry); tries = []; }
    function gone() { return vs.readyState === CLOSING || vs.readyState === CLOSED; }   // the caller gave up (e.g. the recipe client's open timeout)
    function late() {
      timer = null;
      if (bound || vs.via === 'direct') return;
      if (gone()) { closeTries(); return; }           // no socket for a closed virtual socket (it would stay open, unowned)
      relayMisses++;                                  // not sticky: the next open() asks the parent again
      // AI(W906-SCREEN-TOKEN) 20261001: the frame HAS a hub for this URL -> its answer is only late (busy frame). A direct
      //   socket here would be a second server connection with its own token holder (the 'control-held' of 1001), so ask
      //   again on a new channel and keep waiting. Earlier channels stay open: whichever welcome comes first binds, any
      //   later welcome is answered with bye at once (no ghost client in the hub).
      if (parentHasHub(vs.url)) {
        if (!warned) {
          warned = true;
          try { if (g.console && g.console.warn) g.console.warn('[HT9045Link] the frame hub did not answer within ' + HELLO_WAIT_MS + ' ms for ' + vs.url +
            ': asking again and waiting (no direct socket: one connection per screen)'); } catch (ew) {}
        }
        wait = Math.min(wait * 2, HELLO_WAIT_MS * 8);  // back off: a few channels a minute at most
        hello();
        return;
      }
      try { if (g.console && g.console.warn) g.console.warn('[HT9045Link] no hub answered within ' + HELLO_WAIT_MS + ' ms for ' + vs.url +
        ': this socket opens directly (one more server connection); the next open() asks the frame again'); } catch (ew2) {}
      closeTries();
      bindDirect(vs, WSClass());
    }
    function onPort(port, x) {
      if (x.k === 'welcome') {
        if (bound || vs.via === 'direct' || gone()) { // a later answer to an earlier hello (or the socket was given up): leave the hub at once
          dropTry(port);
          return;
        }
        bound = port; relayState[key] = true; clearTimeout(timer); timer = null;
        tries = tries.filter(function (p) { return p !== port; });   // the others stay open until their welcome (-> bye) or close
        looseTries = looseTries.filter(function (p) { return p !== port; });
        vs.via = 'relay';
        vs._up = function (t) { port.postMessage({ k: 'send', data: t }); };
        vs._down = function () { try { port.postMessage({ k: 'bye' }); } catch (e2) {} try { port.close(); } catch (e3) {} forget(vs); closeTries(); };
        mySocks.push({ vs: vs, port: port });
        return;
      }
      if (x.k === 'nohub') {
        if (bound || vs.via === 'direct') return;
        clearTimeout(timer); timer = null; relayState[key] = 'nohub'; closeTries();
        if (gone()) return;                           // AI(W906-SCREEN-TOKEN) 20261001: review R3 -- no socket for a closed virtual socket
        bindDirect(vs, WSClass());
        return;
      }
      if (port !== bound) return;                     // traffic for a channel that did not bind
      if (x.k === 'open') vs._opened();
      else if (x.k === 'suspend') vs._suspend();   // AI(W906-J11-RECONNECT-A) 20261005
      else if (x.k === 'resume') vs._resume();     // AI(W906-J11-RECONNECT-A) 20261005
      else if (x.k === 'msg') vs._message(x.data);
      else if (x.k === 'ping') port.postMessage({ k: 'pong' });
      else if (x.k === 'close') { forget(vs); try { port.close(); } catch (e5) {} closeTries(); vs._closed(x.code, x.reason, false); }
    }
    function hello() {
      var ch = new MC(), port = ch.port1;
      tries.push(port); looseTries.push(port);
      port.onmessage = function (e) { onPort(port, e.data || {}); };
      if (port.start) port.start();
      try {
        g.parent.postMessage({ ht9045link: 'hello', v: 1, url: vs.url, name: name || (g.location && g.location.pathname) || 'iframe' },
                             '*', [ch.port2]);
      } catch (e) {
        if (bound) return;
        clearTimeout(timer); timer = null; relayState[key] = 'nohub'; closeTries(); bindDirect(vs, WSClass());   // the parent is not reachable at all
        return;
      }
      timer = setTimeout(late, wait);
    }
    hello();
  }
  function forget(vs) { mySocks = mySocks.filter(function (x) { return x.vs !== vs; }); }

  function onPageHide() {
    mySocks.slice().forEach(function (x) { try { x.port.postMessage({ k: 'bye' }); } catch (e) {} });
    mySocks = [];
    looseTries.slice().forEach(function (p) { try { p.postMessage({ k: 'bye' }); } catch (e) {} });   // AI(W906-SCREEN-TOKEN) 20261001: review R2
    looseTries = [];
  }

  function open(url, name) {
    var vs = new VSock(url);
    var hub = hubs[normUrl(url)];
    if (hub) attachLocal(hub, vs, name);                     // the frame itself
    else if (isTop() || relayState[normUrl(url)] === 'nohub') bindDirect(vs, WSClass());
    else attachRelay(vs, name);                               // iframe: ask the parent
    return vs;
  }

  if (g.addEventListener) g.addEventListener('pagehide', onPageHide);

  // frame: a window was hidden (background.html setWinState: closed / minimized) -> release its page's held jogs
  function windowHidden(win, why) {
    var n = 0;
    Object.keys(hubs).forEach(function (k) { n += hubs[k].windowHidden(win, why); });
    return n;
  }

  // AI(W906-STREAM-F) 20260930: RULINGS_20260930 #12 (only open pages update data). The frame (background.html postWinState) reports
  //   each .win iframe's state: open = 'open' or 'minimized' (golden: a minimized form keeps fShow and its timers),
  //   closed = 'closed' or 'never'. A closed window's page gets no snapshot / patch frames, except one small snapshot of
  //   BOOT_TAGS when its socket opens -- what pages decide once at load (stage-F audit: Status.Security auth.level,
  //   HW.IoSetView / Main.MotionView9050 / teach / temperf machine.gpibModel via HT9045Live, Setup.Contact
  //   site.arm{1,2}.s{n}) -- so its load-time behaviour is unchanged. The moment it opens it gets a synthetic snapshot
  //   of the whole cache (the late-client frame), before any later patch: the recipe client resets its seq baseline on
  //   every snapshot, and patches are never filtered key by key (a hole in seq = drop + stream.resync).
  //   A client the frame never reported (the frame itself, dialog-bridge overlays, standalone pages) is served as today;
  //   alarm / modal / query / link.* / acks reach every client as before.
  var BOOT_TAGS = /^(auth\.level|machine\.gpibModel|site\.arm[12]\.s\d+)$/;
  var WIN_OPEN = [];   // [{win, open}] as the frame reported it; a window never reported = open
  function winOff(win) {
    if (!win) return false;
    for (var i = 0; i < WIN_OPEN.length; ++i) if (WIN_OPEN[i].win === win) return !WIN_OPEN[i].open;
    return false;
  }
  function bootTags(data) {
    var out = {};
    Object.keys(data || {}).forEach(function (k) { if (BOOT_TAGS.test(k)) out[k] = data[k]; });
    return out;
  }
  function bootFrame(m) {   // the server's snapshot, BOOT_TAGS only (a closed window's page, first snapshot of the session)
    var o = {};
    Object.keys(m).forEach(function (k) { if (k !== 'data') o[k] = m[k]; });
    o.data = bootTags(m.data);
    return JSON.stringify(o);
  }
  Hub.prototype.windowState = function (win, open) {
    var self = this;
    Object.keys(this.clients).forEach(function (cid) {
      var c = self.clients[cid];
      if (c.win !== win) return;
      var wasOff = !!c.tagsOff;
      c.tagsOff = !open;
      if (wasOff && open && c.isOpen && self.snapMeta) {
        var snap = {};
        Object.keys(self.snapMeta).forEach(function (k) { snap[k] = self.snapMeta[k]; });
        snap.type = 'snapshot'; snap.data = self.cache; snap.synthetic = true;
        if (self.lastSeq !== null) snap.seq = self.lastSeq;
        self.stats.tagResync++;
        c.snapSent = true;
        c.deliver(JSON.stringify(snap));
      }
    });
  };
  // frame: a .win iframe's window state (background.html postWinState: every edge + once after the iframe loaded)
  function windowState(win, open) {
    if (!win) return;
    var i = 0;
    while (i < WIN_OPEN.length && WIN_OPEN[i].win !== win) ++i;
    if (i === WIN_OPEN.length) WIN_OPEN.push({ win: win, open: !!open }); else WIN_OPEN[i].open = !!open;
    Object.keys(hubs).forEach(function (k) { hubs[k].windowState(win, !!open); });
  }

  g.HT9045Link = {
    open: open,
    startHub: startHub,
    windowHidden: windowHidden,  windowState: windowState,   // AI(W906-STREAM-F) 20260930
    hub: function (url) { return hubs[normUrl(url)] || null; },
    status: function () {
      return { top: isTop(), relay: relayState, relayMisses: relayMisses, hubs: Object.keys(hubs).map(function (k) { return hubs[k].status(); }),
               relayedSockets: mySocks.length };
    },
    _VSock: VSock,
    _consts: { HELLO_WAIT_MS: HELLO_WAIT_MS, PING_MS: PING_MS, PING_LOSS: PING_LOSS, IDLE_CLOSE_MS: IDLE_CLOSE_MS, DUP_RELEASE_MS: DUP_RELEASE_MS },
  };
})(typeof window !== 'undefined' ? window : this);
