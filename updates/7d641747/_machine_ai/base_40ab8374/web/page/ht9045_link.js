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
    this._ls = { open: [], message: [], close: [], error: [] };
    this._up = null;        // function(text): hand a frame to the transport
    this._down = null;      // function(code, reason): tell the transport we close
    this.via = 'pending';   // 'hub' | 'relay' | 'direct'
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
    this._emit('open');
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
    this.stats = { realOpens: 0, up: 0, down: 0, fanout: 0, localAcks: 0, releasesSent: 0 };
  }

  Hub.prototype.connect = function () {
    if (this.state !== 'idle') return;
    var self = this, ws;
    this.state = 'connecting';
    try { ws = new this.WS(this.url); } catch (e) { this.state = 'idle'; this._allClosed(1006, 'cannot open ' + this.url); return; }
    this.sock = ws;
    this.stats.realOpens++;
    ws.onopen = function () {
      if (self.sock !== ws) return;
      self.state = 'open';
      Object.keys(self.clients).forEach(function (cid) { self._clientOpen(self.clients[cid]); });
    };
    ws.onmessage = function (ev) { if (self.sock === ws) self._fromServer(String(ev.data)); };
    ws.onclose = function (ev) {
      if (self.sock !== ws) return;
      self.sock = null; self.state = 'idle';
      self._allClosed(ev && ev.code, ev && ev.reason);
    };
    ws.onerror = function () {};
  };

  Hub.prototype._resetSession = function () {
    this.inflight = {};
    this.cache = {}; this.snapMeta = null; this.lastSeq = null;
    this.token = { held: false, holders: {}, pend: {} };
  };

  Hub.prototype._allClosed = function (code, reason) {
    var self = this, cs = this.clients;
    this.clients = {};
    this._resetSession();
    Object.keys(cs).forEach(function (cid) { cs[cid].closed(code || 1006, reason || ''); });
    this._pingSchedule();
  };

  // client = {id, name, deliver(text), opened(), closed(code, reason), port?: bool, win?, missed, held:{button: {source, motors}}, source}
  Hub.prototype.add = function (c) {
    c.id = this.nextCid++;
    c.missed = 0; c.held = null; c.source = '';
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
    c.opened();
    if (this.snapMeta) {
      var snap = {};
      Object.keys(this.snapMeta).forEach(function (k) { snap[k] = this.snapMeta[k]; }, this);
      snap.type = 'snapshot'; snap.data = this.cache; snap.synthetic = true;
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

  Hub.prototype._up = function (text) {
    this.stats.up++;
    this.sock.send(text);
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

  // a frame from one client
  Hub.prototype.fromClient = function (cid, text) {
    var c = this.clients[cid];
    if (!c || this.state !== 'open') return;
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
    this._up(JSON.stringify(m));   // exactly once; no retry here
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
    this.stats.fanout++;
    Object.keys(this.clients).forEach(function (cid) { self.clients[cid].deliver(text); });
  };

  Hub.prototype.status = function () {
    var self = this;
    return { url: this.url, state: this.state, clients: Object.keys(this.clients).map(function (cid) {
               var c = self.clients[cid]; return { id: +cid, name: c.name, port: !!c.port, holder: !!self.token.holders[cid] }; }),
             tokenHeld: this.token.held, inflight: Object.keys(this.inflight).length, cachedTags: Object.keys(this.cache).length,
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

  // iframe: one MessageChannel per virtual socket
  function attachRelay(vs, name) {
    var MC = g.MessageChannel;
    if (!MC) { bindDirect(vs, WSClass()); return; }
    var ch = new MC(), port = ch.port1, welcomed = false, key = normUrl(vs.url);
    var timer = setTimeout(function () {
      if (welcomed) return;
      relayMisses++;                                  // not sticky: the next open() asks the parent again
      try { if (g.console && g.console.warn) g.console.warn('[HT9045Link] no hub answered within ' + HELLO_WAIT_MS + ' ms for ' + vs.url +
        ': this socket opens directly (one more server connection); the next open() asks the frame again'); } catch (ew) {}
      try { port.close(); } catch (e) {}
      bindDirect(vs, WSClass());
    }, HELLO_WAIT_MS);
    port.onmessage = function (e) {
      var x = e.data || {};
      if (x.k === 'welcome') {
        welcomed = true; relayState[key] = true; clearTimeout(timer);
        vs.via = 'relay';
        vs._up = function (t) { port.postMessage({ k: 'send', data: t }); };
        vs._down = function () { try { port.postMessage({ k: 'bye' }); } catch (e2) {} try { port.close(); } catch (e3) {} forget(vs); };
        mySocks.push({ vs: vs, port: port });
      } else if (x.k === 'nohub') {
        clearTimeout(timer); relayState[key] = 'nohub'; try { port.close(); } catch (e4) {}
        bindDirect(vs, WSClass());
      } else if (x.k === 'open') vs._opened();
      else if (x.k === 'msg') vs._message(x.data);
      else if (x.k === 'ping') port.postMessage({ k: 'pong' });
      else if (x.k === 'close') { forget(vs); try { port.close(); } catch (e5) {} vs._closed(x.code, x.reason, false); }
    };
    if (port.start) port.start();
    try {
      g.parent.postMessage({ ht9045link: 'hello', v: 1, url: vs.url, name: name || (g.location && g.location.pathname) || 'iframe' },
                           '*', [ch.port2]);
    } catch (e) {
      clearTimeout(timer); relayState[key] = 'nohub'; bindDirect(vs, WSClass());   // the parent is not reachable at all
    }
  }
  function forget(vs) { mySocks = mySocks.filter(function (x) { return x.vs !== vs; }); }

  function onPageHide() {
    mySocks.slice().forEach(function (x) { try { x.port.postMessage({ k: 'bye' }); } catch (e) {} });
    mySocks = [];
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

  g.HT9045Link = {
    open: open,
    startHub: startHub,
    windowHidden: windowHidden,
    hub: function (url) { return hubs[normUrl(url)] || null; },
    status: function () {
      return { top: isTop(), relay: relayState, relayMisses: relayMisses, hubs: Object.keys(hubs).map(function (k) { return hubs[k].status(); }),
               relayedSockets: mySocks.length };
    },
    _VSock: VSock,
    _consts: { HELLO_WAIT_MS: HELLO_WAIT_MS, PING_MS: PING_MS, PING_LOSS: PING_LOSS, IDLE_CLOSE_MS: IDLE_CLOSE_MS, DUP_RELEASE_MS: DUP_RELEASE_MS },
  };
})(typeof window !== 'undefined' ? window : this);
