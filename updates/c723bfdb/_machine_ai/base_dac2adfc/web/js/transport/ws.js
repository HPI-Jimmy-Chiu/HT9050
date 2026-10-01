/* ==========================================================================
   ws.js -- the real transport: a WebSocket to the HT9045 bridge.
   ==========================================================================
   NOT YET WIRED TO ANYTHING. No backend exists at the time of writing; this
   file fixes the wire contract so the C++ side has something concrete to
   implement, and so switching over is a one-line change in index.js.

   Wire contract (JSON text frames)
   --------------------------------
   server -> browser
     { "type": "snapshot", "data": { "<tag>": <value>, ... } }   full state, on connect
     { "type": "patch",    "data": { "<tag>": <value>, ... } }   deltas, as they happen
     { "type": "ack",      "id": <n>, "ok": true|false, "error": "..." }
     { "type": "alarm",    "code": "WAR0152", "text": "...", "at": "<iso8601>" }

   browser -> server
     { "type": "cmd", "id": <n>, "cmd": "<name>", "tag": "<tag>", "value": <any> }
     { "type": "ping", "id": <n> }

   Rules
     - `tag` strings are exactly the ones in js/model/tagmap.js.
     - The server is authoritative. The browser NEVER assumes a command
       succeeded; it waits for the resulting patch. That is what keeps two
       operators on two browsers from disagreeing about machine state.
     - Values are JSON primitives. null means "unknown / not installed".
   ========================================================================== */

const RECONNECT_MS = [500, 1000, 2000, 4000, 8000, 15000];

export function createWebSocketTransport({ url } = {}) {
  const endpoint = url || defaultEndpoint();
  let sock = null;
  let onPatch = () => {};
  let onStatus = () => {};
  /* AI(W906-1203CTL-14) 20260911: ACKS WERE BEING SWALLOWED HERE.
     The ack branch below did nothing but console.warn, so no page could learn
     the outcome of a command it sent -- and one outcome is load-bearing: the
     server's single-operator token (WebBridgeServer.cpp:1212, FW-W3) refuses
     every machine command with "not-operator" until the connection has been
     granted it, and the GRANT ARRIVES ONLY AS AN ACK. Measured 20260911: every
     command the 1203 page sent came back ok:false "not-operator", and the page
     had no way to find out. Optional third argument, so every existing
     two-argument caller is untouched. */
  let onAck = () => {};
  let attempt = 0;
  let nextId = 1;
  let closedByUs = false;
  let retryTimer = null;    // pending scheduleReconnect timeout, so it can be cut short

  function defaultEndpoint() {
    const proto = location.protocol === "https:" ? "wss:" : "ws:";
    return `${proto}//${location.host}/ht9045`;
  }

  function open() {
    closedByUs = false;
    onStatus({ state: "connecting", endpoint });
    try {
      sock = globalThis.HT9045Link ? globalThis.HT9045Link.open(endpoint, "pci1203") : new WebSocket(endpoint);   // AI(W906-WSLINK) 20260929 [W906] ST01-E3: through the frame's hub (page/ht9045_link.js)
    } catch (err) {
      scheduleReconnect(err);
      return;
    }

    sock.addEventListener("open", () => {
      attempt = 0;
      onStatus({ state: "online", endpoint });
    });

    sock.addEventListener("message", ev => {
      let msg;
      try { msg = JSON.parse(ev.data); }
      catch { console.warn("[ws] non-JSON frame dropped"); return; }

      switch (msg.type) {
        case "snapshot":
        case "patch":
          if (msg.data && typeof msg.data === "object") onPatch(msg.data);
          break;
        case "ack":
          if (msg.ok === false) console.warn("[ws] command rejected:", msg);
          onAck(msg);
          break;
        case "alarm":
          onStatus({ state: "alarm", alarm: msg });
          break;
        default:
          console.info("[ws] unhandled frame type:", msg.type);
      }
    });

    sock.addEventListener("close", () => {
      if (!closedByUs) scheduleReconnect(new Error("socket closed"));
    });

    sock.addEventListener("error", () => { /* close handler does the work */ });
  }

  /*  AI(W906-1203ALM-12) 20260914: the status now carries WHEN the next attempt
      is, so a page can show a countdown instead of a motionless "offline".
      The backoff tops out at 15 s, and fifteen silent seconds is long enough
      that an operator concludes the page is dead -- which is exactly what
      happened. `retryAt` is an epoch ms; `retryInMs` is the delay chosen. */
  function scheduleReconnect(err) {
    const delay = RECONNECT_MS[Math.min(attempt, RECONNECT_MS.length - 1)];
    attempt += 1;
    retryTimer = setTimeout(() => { retryTimer = null; if (!closedByUs) open(); }, delay);
    onStatus({ state: "offline", endpoint,
               error: String(err && err.message || err),
               retryInMs: delay, retryAt: Date.now() + delay });
  }

  return {
    name: "ws",
    endpoint,

    /* AI(W906-1203CTL-14) 20260911: +ackHandler, third and OPTIONAL. Every
       existing caller passes two arguments and is unaffected; a page that needs
       to know whether the server accepted a command (the control token, above)
       passes three. js/transport/mock.js's connect takes ONE parameter, so a
       caller must not assume this exists -- see js/pci1203.js. */
    connect(handler, statusHandler, ackHandler) {
      onPatch = handler;
      if (statusHandler) onStatus = statusHandler;
      if (ackHandler) onAck = ackHandler;
      open();
      return Promise.resolve();
    },

    disconnect() {
      closedByUs = true;
      if (sock) sock.close();
      sock = null;
    },

    /*  AI(W906-1203ALM-12) 20260914: ⚠ THIS USED TO DROP THE COMMAND SILENTLY.
        It console.warn'd and returned undefined, so a page that pressed a
        button while the socket was down got NOTHING -- no note, no refusal, no
        ack. "斷線了按鈕會都沒反應" is this line, exactly. The caller could not
        tell a dropped command from a sent one because both returned undefined.
        Now it returns a boolean: true = handed to the socket, false = not sent.
        ⚠ true still means only "the browser wrote it to the wire". Whether the
        machine did anything is the ack's business and always was. */
    send({ cmd, tag, value }) {
      if (!sock || sock.readyState !== WebSocket.OPEN) {
        console.warn("[ws] dropped command, socket not open:", cmd);
        return false;
      }
      sock.send(JSON.stringify({ type: "cmd", id: nextId++, cmd, tag, value }));
      return true;
    },

    /*  AI(W906-1203ALM-12) 20260914: reconnect NOW, for an operator who does not
        want to wait out the backoff. The delay tops out at 15 s and the page
        looks dead for every one of them.
        ⚠ Resets `attempt`, so a manual retry starts the backoff ladder again
        rather than inheriting the 15 s step -- otherwise pressing the button
        would appear to do nothing for a quarter of a minute, which is the
        failure this whole change is about. */
    reconnectNow() {
      if (retryTimer) { clearTimeout(retryTimer); retryTimer = null; }
      attempt = 0;
      closedByUs = false;
      if (sock) { try { sock.close(); } catch { /* already gone */ } sock = null; }
      open();
      return true;
    },

    /* Is the wire actually usable right now? The page asks before offering a
       machine command, rather than finding out by being ignored. */
    isOpen() { return !!sock && sock.readyState === WebSocket.OPEN; },
  };
}
