/* ==========================================================================
   pci1203.js -- the PCIE-1203 monitor page, WIRING half.

   AI(W906-MW2) 20260908. All rendering lives in js/pci1203/view.js, which is
   pure (tag Map -> DOM) and therefore testable by
   web/tools/jsprobe/probe_pci1203.mjs. This file owns only the things a probe
   cannot exercise: the transport, the repaint timer, and the document.
   ========================================================================== */

import { createTransport } from "./transport/index.js";
import { buildReport, buildRail, refresh, subtitle, footerHtml,
         AXIS_SLOTS, DI_PORTS, DO_PORTS, SLAVE_SLOTS } from "./pci1203/view.js";

const tags = new Map();          // tag -> value (absent key => not on the wire)

/* ===========================================================================
   AI(W906-SJSON-S9b) 20260923: UNPACK io.di / io.do INTO THE PER-PORT KEYS
   THIS PAGE HAS ALWAYS READ.

   使用者 20260923：「改成新的方式, 兩邊同時修改!」 —— 同一顆 commit 裡
   C++ 不再送 512 個 pci1203.di<N>／do<N> 值 tag（WebBridgeTags.cpp），
   改送 4 個位元打包 tag（JsonBridge/ChanIo.cpp）：

       io.di        base64(320 bytes)  每個 port 一個 byte      <- 資料平面
       io.di.valid  base64( 40 bytes)  每個 port 一個 bit       <- 在位平面
       io.do        base64(192 bytes)
       io.do.valid  base64( 24 bytes)

   ⚠⚠ 展開放在**資料入口**，不是在 view.js 的 5 個讀取點，而且是刻意的：
     view.js 的 refresh() 用 data-k（tag 名）在每次 patch 之後**重新查 tags**
     （view.js「IN-PLACE REFRESH」那段）。只改建畫面的讀取點的話，refresh()
     還是會去查 pci1203.diN，查不到就把每一格寫成 n/a ——
     畫面第一次畫對、0.2 秒後全部變空。那正是 ChanIo.h 檔頭警告的
     「一邊改了另一邊沒改，畫面不會報錯，只會安靜地空掉」。
   ⇒ 在入口展開，view.js 一行都不用改，refresh() 也自動正確。

   ⚠ 三態必須原樣保留（view.js cellOf 就是照這三態畫的）：
       key 不存在        -> "n/a"   這一格從來沒上過線
       key 存在但 null   -> "---"   上線了但這次讀不到 / 沒有這個 port
       number            -> 值
     舊的 C++ 用 stageInt(snap, tag, ok, v) 做到這件事：ok=false 送 null。
     所以這裡**一定要把 valid=0 的 port 設成 null**，不是不設 key、也不是設 0。
     設 0 會把「沒讀到」畫成「讀到 0x00」——
     Pci1203Monitor.h:1133 原話：a dark lamp means either that coil is off or
     nobody looked，兩者導向相反的維修動作。

   ⚠ 位元序照 C++：ChanIo.cpp `dv[i / 8] |= 1u << (i % 8)` —— 每個 byte 內
     **LSB 先**。寫成 (7 - i%8) 會讓每 8 個 port 的在位狀態左右顛倒，
     而且第 8 個 port 之後都還「看起來有值」，極難發現。
   =========================================================================== */
const IO_DI_PORTS = 320;   // = kPci1203TagDiPorts（EtherCAT/Pci1203Monitor.h:1637）
const IO_DO_PORTS = 192;   // = kPci1203TagDoPorts（同上 :1638）

/** base64 -> Uint8Array；不是字串或解不開一律回 null（呼叫端當「沒有平面」）。 */
function b64Bytes(v) {
  if (typeof v !== "string" || v === "") return null;
  try {
    const bin = atob(v);
    const out = new Uint8Array(bin.length);
    for (let i = 0; i < bin.length; i++) out[i] = bin.charCodeAt(i);
    return out;
  } catch (_e) {
    return null;                    // 一個壞 tag 不該打掉整個 patch
  }
}

/** 把一對平面展開成 `${prefix}${i}` 的 per-port key。 */
function unpackPlane(prefix, nPorts, dataB64, validB64) {
  const data  = b64Bytes(dataB64);
  const valid = b64Bytes(validB64);
  for (let i = 0; i < nPorts; i++) {
    let v = null;
    /*  兩個平面都在才敢給數字。少了任一個就是 null ——
        「拿不到在位資訊」與「這個 port 讀不到」對畫面是同一件事：別畫數字。 */
    if (data && valid && i < data.length && (i >> 3) < valid.length) {
      if ((valid[i >> 3] & (1 << (i & 7))) !== 0) v = data[i];
    }
    tags.set(prefix + i, v);
  }
}

/** 每次 patch 進來之後呼叫。
 *  ⚠ 只在該平面**這次有送**時才重算：沒帶 io.di 的 patch 若也重算，
 *    會用上一輪的值重寫一次（無害）或在 tags 還沒有 io.di 時寫成全 null
 *    （有害 —— 畫面會在每個不含 io.* 的 patch 之後閃一次空白）。 */
function unpackIoPlanes(data) {
  const has = k => Object.prototype.hasOwnProperty.call(data, k);
  if (has("io.di") || has("io.di.valid")) {
    unpackPlane("pci1203.di", IO_DI_PORTS,
                tags.get("io.di"), tags.get("io.di.valid"));
  }
  if (has("io.do") || has("io.do.valid")) {
    unpackPlane("pci1203.do", IO_DO_PORTS,
                tags.get("io.do"), tags.get("io.do.valid"));
  }
}
let   linkState = { state: "connecting" };
let   frames    = 0;
let   dirty     = true;
let   transport = null;          // set in boot(); the only sender on this page

/*  AI(W906-1203CTL-14) 20260911: DO WE HOLD THE SERVER'S OPERATOR TOKEN?
 *
 *  Tracked HERE and not read from a tag, and the reason is the sidecar split:
 *  the token lives in wb_gateway's WebBridgeServer (per connection), while the
 *  control.owner TAG is staged by wb_publish -- a DIFFERENT PROCESS that has no
 *  server and no idea who is connected. Reading control.owner here would show a
 *  permanently empty field and look like "nobody has control" forever.
 *
 *  So the only truthful source is the ack to our own control.acquire, which is
 *  why js/transport/ws.js grew an ack callback on the same day.
 *
 *  ⚠ RESET ON EVERY RECONNECT. The token is per CONNECTION; a dropped socket
 *  loses it, and a page that kept believing it held one would send commands
 *  that are all refused while showing "HELD".
 */
let   hasControl = false;
const TOKEN_CMDS = ["control.acquire", "control.release"];
const pendingAcks = new Map();   // wire id -> the command name we sent
//AI(W906-1203WHYBOX-1) 20260918: the last command name this page SENT, for
//  labelling a gateway refusal (which never reaches the machine and so never
//  updates pci1203.control.lastCmd). Label only -- nothing branches on it.
let pendingCmdName = null;

/*  AI(W906-1203ALM-11) 20260914: HOLD THE TOKEN AUTOMATICALLY.
 *  User: "我現在每次就是要控制了 就是永遠都是取得控制權的狀態".
 *
 *  The token is per CONNECTION, so every reconnect used to drop it and leave a
 *  page full of buttons that refused locally until somebody pressed TAKE
 *  CONTROL again. On a machine where operating the panel IS the normal state,
 *  that is a prompt for a question with only one answer.
 *
 *  ⚠ THREE THINGS THIS MUST NOT DO, each of which it would do if written as a
 *  bare "acquire on connect":
 *    1. FIGHT AN EXPLICIT RELEASE. Pressing 釋放操作權 and having the page grab
 *       the token straight back makes that button look broken. So a successful
 *       release turns auto-acquire OFF until the operator asks for it again by
 *       pressing 取得操作權.
 *    2. SPAM THE LOG. Another tab or operator legitimately holds the token
 *       sometimes. The retry is slow, and the refusal is reported only when the
 *       REASON changes -- not once every few seconds.
 *    3. STEAL SILENTLY. The grant is announced in the note line like any other,
 *       so "this tab took control from another one" is visible rather than
 *       something the next person discovers by finding their buttons dead.
 */
let   autoControl   = true;   // off only after a deliberate 釋放操作權
let   lastAcqRefusal = null;  // last refusal text, so it prints once not forever
let   nextAcqAt      = 0;     // Date.now() of the next permitted attempt

/*  ⚠ WHAT THIS PAGE CANNOT SHOW, and why it does not pretend to:
 *
 *  `gen` -- the publisher stamps every frame with a generation counter
 *  (measured off the live feed: {"type":"snapshot","gen":63,...}), and it would
 *  be the natural liveness indicator to display. It is NOT displayed because
 *  js/transport/ws.js:67 forwards only `msg.data` to its handler and drops
 *  `gen`. Rendering "gen —" forever would be a small lie about a liveness
 *  indicator, which is worse than not offering one. The frame counter IS real,
 *  because this file counts it itself.
 *
 *  `removed` -- the wire carries a removed-tags list and ws.js ignores it, so a
 *  tag that genuinely went away would linger in this Map. Harmless here on
 *  purpose rather than by luck: the pci1203 tag set is a FIXED-SHAPE wire
 *  (WebBridgeTags.h publishes all 8 axis / 4 DI / 8 slave slots every frame,
 *  nulls included), so nothing in this family is ever removed. If a future wave
 *  makes the tag set variable, this comment is the thing to revisit.
 */

/*  AI(W906-MW2b) 20260910: ?station=N narrows the IO section to one module.
 *
 *  WHY A URL PARAMETER AND NOT A DROPDOWN. view.js is PURE -- a function of a
 *  tag Map returning DOM -- which is what lets probe_pci1203.mjs execute it
 *  headless against a real captured snapshot. A <select> would put state in the
 *  document and a listener in the view, and the filter would stop being
 *  testable. As a parameter it is just another argument, so the probe can
 *  assert the filtered rendering directly.
 *    It also survives a reload and can be bookmarked or pasted to somebody
 *    standing at the machine, which a dropdown cannot.
 */
function stationFilter() {
  const raw = new URLSearchParams(location.search).get("station");
  if (raw === null) return null;
  const n = Number(raw);
  /* An unparsable value must NOT silently become "show everything": somebody
     who typed ?station=8O (letter O) would get a full page and believe it was
     one module's. Fall back to no filter, but say so on the page. */
  return Number.isFinite(n) ? n : NaN;
}

/*  AI(W906-1203CTL-19) 20260911: which pane the rail selected. Same reasoning
 *  as stationFilter: a URL parameter rather than document state, so view.js
 *  stays a pure function of (tags, opts) and the probe can render every pane
 *  headless. Unknown values fall back to the overview rather than a blank pane.
 */
function viewMode() {
  const v = (new URLSearchParams(location.search).get("view") || "").toLowerCase();
  return (v === "axes" || v === "io" || v === "diag") ? v : "";
}

/*  AI(W906-1203CTL-30) 20260911: which axis the rail selected, or null for all.
 *  Same rule as stationFilter: an unparsable value must NOT silently become
 *  "show everything", because ?ax=1O (letter O) would then quietly widen a pane
 *  that exists to narrow one -- and on this page a pane showing sixteen sets of
 *  JOG buttons instead of one is the thing it was built to stop. */
function axisFilter() {
  const raw = new URLSearchParams(location.search).get("ax");
  if (raw === null) return null;
  const n = Number(raw);
  return (Number.isInteger(n) && n >= 0 && n < AXIS_SLOTS) ? n : null;
}

/*  AI(W906-1203CTL-27) 20260911: WHAT MAKES THE PAGE'S SHAPE DIFFERENT.
 *
 *  ⚠ REBUILDING ON EVERY PATCH MADE THE PAGE UNUSABLE, and that was my bug.
 *  render() replaced the whole report ~5 times a second, so a button destroyed
 *  between mousedown and mouseup never received its click -- every control on
 *  the page was dead, a number box lost what was being typed, and the rail
 *  flickered. The user reported all three: "左邊你怎一直更新? 我按鈕都按不下去".
 *
 *  A 0.2 s IO POLL never required a 0.2 s DOM rebuild. Those were conflated by
 *  me, not by the requirement: "我是說IO更新率 不是畫面更新率0.2秒".
 *
 *  So the DOM is rebuilt only when this signature changes -- which pane, which
 *  card, whether controls exist, which stations and axes are present -- and
 *  every other frame updates the VALUES in place. The signature deliberately
 *  includes the things that change the NUMBER OR IDENTITY of nodes and nothing
 *  that merely changes their text; a signature that included a lamp value would
 *  rebuild on the very event this exists to survive.
 */
/*  AI(W906-1203CTL-31) 20260911: TWO KEYS, because the rail and the pane change
 *  for different reasons and the user is paying for them being one.
 *
 *  User: "我點左邊頁面時 左邊頁面可以不要刷新嗎? 刷新右邊頁面就好 / 因為我滑下去
 *  點選後 又被滑到最上面 很難控制".
 *
 *  ⚠ TWO SEPARATE CAUSES PRODUCED THAT ONE SYMPTOM, and fixing either alone
 *  leaves it:
 *    1. every rail entry is an <a href="?..."> -- a real navigation, so the
 *       whole document reloaded and the rail's scrollTop went to 0;
 *    2. even without a reload, the selection is part of the shape key, so the
 *       rail was REBUILT on every selection change, which also resets scroll.
 *  So the click is now intercepted (history.pushState, no reload) AND the rail
 *  key excludes the selection, so choosing a card rebuilds only the pane.
 *
 *  The URL still changes, so a pane is still bookmarkable and can still be read
 *  down a phone -- that property was load-bearing and is kept.
 */
function railKey() {
  /* WHAT EXISTS, never WHAT IS SELECTED. A selection in here would rebuild the
     rail on every click, which is half of the bug above. */
  const parts = [];
  for (let i = 0; i < SLAVE_SLOTS; i++) {
    if (tags.get(`pci1203.slave${i}.present`) === true) {
      /*  AI(W906-1203ALM-14) 20260914: was slaveN.alias, which is never published
        (the ESC 0x0012 read is gated off). So this contributed the string
        "sundefined" for every slot and the rail key stopped depending on the
        slave scan at all -- a module appearing or leaving the ring would not
        rebuild the rail. Same dead field as stationCards/stationMatches. */
    parts.push("s" + tags.get(`pci1203.slave${i}.addr`));
    }
  }
  for (let i = 0; i < DI_PORTS; i++) {
    const s = tags.get(`pci1203.di${i}.station`);
    if (typeof s === "number") parts.push("di" + i + ":" + s);
  }
  for (let i = 0; i < DO_PORTS; i++) {
    const s = tags.get(`pci1203.do${i}.station`);
    if (typeof s === "number") parts.push("do" + i + ":" + s);
  }
  for (let a = 0; a < AXIS_SLOTS; a++) {
    if (tags.get(`pci1203.ax${a}.opened`) === true) {
      /*  AI(W906-1203KEEP-1) 20260916: ⚠ THE STATE IS NO LONGER PART OF THE
          SHAPE, and the comment that used to be here is why it took so long to
          find: it said the state "changes rarely (ERROR_STOP <-> READY), not
          per frame" -- true, and it left out the one moment it DOES change,
          which is when the operator presses MOVE. So every move rebuilt the
          rail AND the stage, and the stage rebuild cleared the distance box
          they had just typed into. User: "每次移動後 數值都會被重製".
          The rail subtitle now carries a [data-k] span that refresh() updates
          in place, so the state stays live without being a shape. What is left
          here is what the rail's CONTENTS are: which axes exist. */
      parts.push("ax" + a);
    }
  }
  parts.push(String(tags.get("pci1203.control.armed")));
  return parts.join("|");
}

function shapeKey() {
  const parts = [viewMode(), String(stationFilter()), String(axisFilter()),
                 String(hasControl),
                 /*  AI(W906-1203ALM-12) 20260914: the link state is part of the
                     SHAPE now, because the offline banner is a piece of the
                     report rather than a value inside it. Without this the
                     stage is never rebuilt on a disconnect and the banner
                     would appear only after some unrelated change happened to
                     alter the shape -- i.e. a disconnection notice that shows
                     up late and at random. */
                 String(linkState && linkState.state),
                 String(tags.get("pci1203.control.armed")),
                 String(tags.get("pci1203.control.dryRun")),
                 String(tags.get("pci1203.phase")),
                 String(linkState && linkState.state),
                 railKey()];
  return parts.join("|");
}

/*  Mark the selected rail entry WITHOUT rebuilding the rail. Compares the
 *  link's own parameters with the current ones rather than the raw href, so a
 *  harmless difference in parameter order cannot silently un-highlight
 *  everything -- which would look exactly like a click that did not register. */
function markRail(rail) {
  const view = viewMode(), st = stationFilter(), ax = axisFilter();
  for (const a of rail.querySelectorAll("a.railitem")) {
    const q = new URLSearchParams((a.getAttribute("href") || "").replace(/^\?/, ""));
    const lv = (q.get("view") || "").toLowerCase();
    const ls = q.has("station") ? Number(q.get("station")) : null;
    const la = q.has("ax") ? Number(q.get("ax")) : null;
    const on = lv === view && ls === (st === null ? null : st) && la === ax;
    a.classList.toggle("railon", on);
  }
}

let lastShape = null;
let lastRail  = null;

function render() {
  const rail = document.getElementById("rail");
  const stage = document.getElementById("stage");
  const st = stationFilter();
  const opts = { hasControl, view: viewMode() };
  if (st !== null && Number.isFinite(st)) opts.station = st;
  const ax = axisFilter();
  if (ax !== null) opts.axis = ax;

  /*  AI(W906-1203CTL-31) 20260911: THE RAIL IS REBUILT ONLY WHEN ITS CONTENTS
      CHANGE -- a card appearing, an axis opening, a state flipping. Choosing a
      different card does NOT rebuild it, so its scroll position survives.
      When it does have to be rebuilt, the scroll position is carried across:
      a rack coming online should not throw the operator back to the top of a
      list they were part way down. */
  const rk = railKey();
  if (rail && rk !== lastRail) {
    const keepScroll = rail.scrollTop;
    lastRail = rk;
    rail.textContent = "";
    rail.appendChild(buildRail(tags, opts));
    rail.scrollTop = keepScroll;
  }
  if (rail) markRail(rail);

  const shape = shapeKey();
  if (shape !== lastShape) {
    lastShape = shape;
    /*  AI(W906-1203KEEP-1) 20260916: ⚠ CARRY THE TYPED NUMBERS ACROSS A REBUILD.
        User: "每次移動後 數值都會被重製 我可以保留我輸入的值嗎?".

        The "values only" branch below already preserves a half-typed box, and
        the comment there says so -- which is exactly why this was missed: the
        preservation was real and the REBUILD path had none. Pressing MOVE
        changes the axis state, the state was part of the shape, so every move
        took the rebuild path and cleared the field the operator had just
        filled in. They then retyped 1000 for every single move.

        railKey() is fixed separately so a move stops rebuilding at all. This
        stays anyway, because it is the general guarantee: ANY future shape
        change -- an axis coming online, the link dropping, a card being
        fitted -- would otherwise eat the number again, and each of those would
        be found the same way, by someone losing their typing.

        ⓘ Keyed on data-role, which is what valueOf() looks the box up by, so a
        box that is restored is exactly a box a command can read. Boxes that no
        longer exist after the rebuild are simply not restored. */
    const keepBox = new Map();
    for (const b of stage.querySelectorAll("input[data-role]")) {
      if (b.value !== "") keepBox.set(b.dataset.role, b.value);
    }
    const active = document.activeElement;
    const keepFocus = (active && active.dataset && active.dataset.role)
                    ? active.dataset.role : null;
    const keepSel = keepFocus ? [active.selectionStart, active.selectionEnd] : null;

    stage.textContent = "";
    stage.appendChild(buildReport(tags, linkState, opts));

    for (const b of stage.querySelectorAll("input[data-role]")) {
      const v = keepBox.get(b.dataset.role);
      if (v !== undefined) b.value = v;
    }
    /*  And the caret, because restoring the text but not the cursor turns
        "typing 1000" into "typing 1000 and then finding the caret moved". */
    if (keepFocus) {
      const again = stage.querySelector(`input[data-role="${keepFocus}"]`);
      if (again) {
        again.focus();
        try { again.setSelectionRange(keepSel[0], keepSel[1]); } catch (e) { /* number inputs may refuse */ }
      }
    }
    /*  The rail is only rebuilt when railKey changes, so on a stage-only
        rebuild its bound values would sit stale for a tick. One call, and the
        rail's live state matches the stage's. */
    if (rail) refresh(rail, tags);
  } else {
    /* The common path: values only. Nothing is created, nothing destroyed, so
       a button under the cursor and a half-typed number box both survive. */
    refresh(stage, tags);
    if (rail) refresh(rail, tags);
  }
  document.getElementById("sub").textContent = subtitle(tags, linkState, frames);
  /* AI(W906-1203CTL-10) 20260911: the footer depends on whether the write
     surface is armed -- see view.js. A constant here would keep asserting
     "no machine control of any kind" on a page that had grown buttons. */
  document.getElementById("foot").innerHTML = footerHtml(tags);

  if (st !== null && !Number.isFinite(st)) {
    const w = document.getElementById("mockwarn");
    w.hidden = false;
    w.textContent =
      "?station= was not a number, so NO filter was applied and every station is " +
      "shown below. Reload with ?station=80 (decimal) to narrow to one module.";
  }
}

/* Repaint on a frame timer rather than per patch: patches arrive ~4/s and a
   full rebuild per patch would fight the browser for no benefit on a
   diagnostic page. State is applied immediately; only the PAINT is coalesced,
   so nothing displayed is ever older than one frame. */
function tick() {
  if (dirty) { dirty = false; render(); }
  requestAnimationFrame(tick);
}

/* ==========================================================================
   AI(W906-1203CTL-10) 20260911: THE ONLY PLACE THIS PAGE SENDS ANYTHING.

   ONE delegated listener on the stage, bound ONCE at boot. Not one listener
   per button, and none inside view.js, for two reasons that both matter:

     1. view.js stays PURE, so probe_pci1203.mjs can keep executing the whole
        rendering headless -- including the armed rendering, which it could not
        do if building a panel required a transport.
     2. render() rebuilds the entire report on every frame. Per-button
        listeners would be attached ~4 times a second and every one of them
        would have to be torn down; delegation makes the rebuild free and makes
        "did we leak a listener" a question that cannot arise.

   ⚠ THE HANDLER TRUSTS NOTHING AND VALIDATES NOTHING.
   It reads the command triple off the element and sends it. It does not check
   an axis number, does not clamp a speed, does not decide whether a move is
   safe. All of that lives in EtherCAT/Pci1203Control.cpp behind its allowlist
   and its gate. A browser that pre-validated would be a SECOND place the
   machine's rules are written down, and the two would drift -- and the one that
   drifts silently is the one in the browser, because nothing gates it.
   ========================================================================== */
function valueOf(role) {
  const box = document.querySelector(`[data-role="${role}"]`);
  if (!box) return null;
  const n = Number(box.value);
  /* An unparsable box must NOT become 0. "move relative by 0" is a legal
     command that does nothing, so a silent fallback would look like a button
     that works and a machine that ignores it. Refuse and say so. */
  return Number.isFinite(n) ? n : null;
}

/*  AI(W906-1203ALM-5) 20260912: the confirm dialog for an irreversible command.
    Built rather than using window.confirm() for two reasons that both matter:
      * window.confirm gives one OK button, and one OK button is a click. This
        needs the operator to state WHICH drive, so it asks them to type the
        station number and keeps the button disabled until it matches.
      * a browser can suppress repeated window.confirm dialogs ("prevent this
        page from creating additional dialogs"). A suppressed confirm returns
        false, so it would fail safe -- but it would also make the button look
        broken, and this page has spent long enough looking broken.
    ⚠ The typed number is what travels, as "confirm=<n>". The C++ side compares
    it against the axis's own station and refuses a mismatch. So a stale tab
    whose axis list has re-ordered cannot reset a drive the operator never saw. */
/*  AI(W906-1203WHYBOX-1) 20260918: A DIALOG WHEN A COMMAND DOES NOT GO IN.
    User: "如果參數或是齒輪比那些 寫不進去驅動器 我需要回報 要出現視窗 為啥寫不進去".

    ⚠ THE REASON WAS ALREADY BEING PRODUCED, IN FULL, AND WAS NOT BEING READ --
    which is why this is a dialog and not another line of text. The refusal text
    is published as pci1203.control.lastWhy and rendered in a `.cmdresult` strip
    on the panel; that strip was itself added (20260915) because the same text
    had previously only existed on a DIFFERENT VIEW. So this is the third
    attempt at the same problem, and the first two both failed the same way: a
    message that does not interrupt is a message that competes with fifty other
    rows on a busy screen.
    ⓘ The strip STAYS. It is the record of the last command and it is useful
    after the dialog is dismissed. This adds the interruption, not a second copy
    of the information.

    ⚠ IT FIRES ONCE PER RESULT, KEYED ON lastId. render() runs ~4 times a
    second, so anything keyed on "the tags currently say failed" would reopen
    the dialog the instant it was closed, forever -- a page nobody can use. The
    wire id is the card's own per-command number, so "have I already shown this
    one" is answerable without guessing. */
let lastShownFailureId = null;

function showFailureBox(o) {
  /*  o: { title, cmd, why, ret, retText, call, hint } */
  const back = document.createElement("div");
  back.className = "confirmback";
  const box = document.createElement("div");
  box.className = "confirmbox";

  const h = document.createElement("div");
  h.className = "confirmhead";
  h.textContent = o.title;
  box.appendChild(h);

  const body = document.createElement("div");
  body.className = "confirmbody";
  /*  textContent, not innerHTML: `why` carries the vendor's own sentence and
      the operator's own typed numbers, neither of which this page should be
      parsing as markup. */
  const parts = [];
  if (o.cmd)     parts.push(`指令：${o.cmd}`);
  if (o.why)     parts.push(`\n原因：\n${o.why}`);
  if (typeof o.ret === "number" && o.ret !== 0) {
    parts.push(`\n驅動器/卡片回傳碼：0x${(o.ret >>> 0).toString(16).toUpperCase().padStart(8, "0")}` +
               (o.retText ? `\n　${o.retText}` : ""));
  }
  if (o.call)    parts.push(`\n實際呼叫：\n${o.call}`);
  if (o.hint)    parts.push(`\n\n${o.hint}`);
  body.textContent = parts.join("\n");
  body.style.whiteSpace = "pre-wrap";
  box.appendChild(body);

  const row = document.createElement("div");
  row.className = "confirmrow";
  const ok = document.createElement("button");
  ok.type = "button";
  ok.className = "cmd";
  ok.textContent = "知道了";
  const close = () => {
    document.removeEventListener("keydown", onKey);
    if (back.parentNode) back.parentNode.removeChild(back);
  };
  const onKey = (e) => { if (e.key === "Escape" || e.key === "Enter") close(); };
  ok.addEventListener("click", close);
  /*  Clicking the backdrop closes it too. A modal with exactly one way out is
      a modal an operator with a hand on a jog button resents. */
  back.addEventListener("click", (e) => { if (e.target === back) close(); });
  document.addEventListener("keydown", onKey);
  row.appendChild(ok);
  box.appendChild(row);

  back.appendChild(box);
  document.body.appendChild(back);
  ok.focus();
}

/*  Turn the published result tags into a dialog, at most once per command.
    ⚠ Called from the TAG handler, not from render(): render is a redraw and
    can run for reasons that have nothing to do with a new result. */
function checkCommandFailure() {
  const id = tags.get("pci1203.control.lastId");
  if (id === undefined || id === null) return;
  if (id === lastShownFailureId) return;

  const cmd = tags.get("pci1203.control.lastCmd");
  if (typeof cmd !== "string" || !cmd.length) return;

  const ok  = tags.get("pci1203.control.lastOk");
  const ret = tags.get("pci1203.control.lastRet");
  const why = tags.get("pci1203.control.lastWhy");

  const refused = (ok === false);
  const drivebad = (typeof ret === "number" && ret !== 0);
  /*  ⚠ A THIRD CASE, AND IT IS THE ONE THE USER ACTUALLY HIT: accepted, issued,
      return code 0, and the C++ side still wrote a `why` -- that is the
      read-back verification reporting "the drive said OK and the number did not
      change". Testing only ok/ret would have missed exactly the failure this
      dialog was asked for. */
  const softbad = (!refused && !drivebad && typeof why === "string" && why.length > 0);

  if (!refused && !drivebad && !softbad) {
    /*  Remember the successful id too, so the NEXT failure is still new. */
    lastShownFailureId = id;
    return;
  }
  lastShownFailureId = id;

  showFailureBox({
    title: refused  ? "⚠ 這個指令被拒絕了，沒有送到驅動器"
         : drivebad ? "⚠ 指令送出去了，但驅動器回了錯誤"
                    : "⚠ 寫進去了，但值沒有變成你要的",
    cmd,
    why: why || "(沒有給原因)",
    ret,
    retText: tags.get("pci1203.control.lastRetText"),
    call: tags.get("pci1203.control.lastCall"),
    hint: refused
      ? "「拒絕」是這個軟體擋下來的，機器沒有收到任何東西 —— 上面那段就是擋下來的理由。"
      : drivebad
      ? "回傳碼是驅動器或卡片自己給的，不是這個軟體寫的。"
      : "寫入的封包被收下了，但參數沒有採用那個值。"
  });
}

function askConfirm(btn, cmd, tag) {
  const station = btn.dataset.station;
  const what    = btn.dataset.confirm || "";

  const back = document.createElement("div");
  back.className = "confirmback";
  const box = document.createElement("div");
  box.className = "confirmbox";

  const h = document.createElement("div");
  h.className = "confirmhead";
  h.textContent = "⚠ 這個動作無法復原";
  box.appendChild(h);

  /*  AI(W906-1203HOME-1) 20260915: ⚠ THIS DIALOG USED TO BE HARD-CODED TO
      Fn008, AND BY THEN THREE DIFFERENT COMMANDS WERE USING IT.
      It said "即將對 X 執行 Fn008 絕對編碼器重置" no matter which button opened
      it, so pressing 套用電子齒輪比 asked the operator to confirm an absolute
      encoder reset -- a description of a DIFFERENT, irreversible action, on the
      one screen whose entire job is to say what is about to happen. It survived
      review because the tests that exercised those buttons deliberately never
      opened the dialog.
      Each button now supplies its own wording via data-confirmbody and its own
      go-button label via data-confirmgo. No default text: a confirmation that
      cannot say what it is confirming must not be shown at all. */
  const body = document.createElement("div");
  body.className = "confirmbody";
  body.textContent = (btn.dataset.confirmbody || "")
      .replace(/\{what\}/g, what)
      .replace(/\{station\}/g, String(station)) +
    /*  AI(W906-1203HEX-1) 20260915: ⚠ SAY WHICH BASE. The label above now shows
        the station as "0x36  (54)" because Common Motion Utility's tree is hex
        and ours was decimal, so the same drive looked like two. But the number
        the operator TYPES is compared against the axis's own station on the C++
        side, which is decimal -- and a typed "36" is a real, different station.
        So the prompt names the decimal explicitly instead of saying "站號". */
    `\n\n確認請輸入站號的十進位數字 ${station}：`;
  box.appendChild(body);

  const inp = document.createElement("input");
  inp.type = "text";
  inp.className = "confirminput";
  inp.setAttribute("autocomplete", "off");
  box.appendChild(inp);

  const row = document.createElement("div");
  row.className = "confirmrow";
  const cancel = document.createElement("button");
  cancel.type = "button";
  cancel.className = "cmd";
  cancel.textContent = "取消";
  const go = document.createElement("button");
  go.type = "button";
  go.className = "cmd warnbtn";
  go.textContent = btn.dataset.confirmgo || "執行";
  go.disabled = true;
  row.appendChild(cancel);
  row.appendChild(go);
  box.appendChild(row);
  back.appendChild(box);
  document.body.appendChild(back);
  inp.focus();

  const close = () => { try { document.body.removeChild(back); } catch { /* already gone */ } };
  const match = () => inp.value.trim() === String(station);

  inp.addEventListener("input", () => { go.disabled = !match(); });
  inp.addEventListener("keydown", e => {
    if (e.key === "Enter" && match()) go.click();
    if (e.key === "Escape") { close(); }
  });
  cancel.addEventListener("click", () => {
    close();
    note(`${cmd} 已取消 — 沒有送出任何東西。`);
  });
  go.addEventListener("click", () => {
    if (!match()) return;              // belt and braces; the button is disabled
    close();
    /*  AI(W906-1203HOME-1) 20260915: a confirmed command may ALSO carry a
        choice. 馬達方向 does: the operator picks CCW or CW and then confirms
        the station, so both have to travel. data-value is the choice; the
        typed station is always "confirm=". Commands with no choice (Fn008,
        套用電子齒輪比) send exactly what they always did, so this widening does
        not change their wire form.

        AI(W906-1203OT-1) 20260915: ⚠ AND THE KEY IS PER-BUTTON NOW. It was the
        literal "dir=", which was right for the one command that existed when it
        was written and silently wrong for the next one: 極限 PASS sends "val=",
        and a hard-coded "dir=" would have produced a well-formed message the
        C++ parser rejects with "has no val=" -- a refusal that looks like a
        server bug rather than a wire-format mismatch. Same shape as the
        confirmation TEXT being hard-coded to Fn008 while three commands used
        it. Default stays "dir" so the existing buttons are untouched. */
    /*  AI(W906-1203ONE-1) 20260916: a confirmed command may also carry a whole
        SET of fields. 一鍵設定電子齒輪比 does: data-collect names the boxes to
        gather, and only the ones the operator actually filled in travel.
        ⚠ EMPTY MEANS ABSENT, NOT ZERO. An operator changing only the
        denominator leaves seven boxes blank, and sending 0 for those would ask
        the drive to set eight parameters when they asked for one. The C++ side
        distinguishes the two as well (gearHas[]), so both ends agree. */
    const key = btn.dataset.confirmkey || "dir";
    let value;
    if (btn.dataset.collect) {
      const parts = [];
      for (const role of btn.dataset.collect.split(",")) {
        const box = document.querySelector(`[data-role="${role}"]`);
        if (!box) continue;
        const raw = String(box.value).trim();
        if (raw === "") continue;                    // not filled in
        const n = Number(raw);
        if (!Number.isFinite(n)) {
          note(`${cmd} 沒有送出：「${role}」不是數字。`);
          return;
        }
        parts.push(`${role.split(".").pop()}=${n}`);
      }
      if (parts.length === 0) {
        note(`${cmd} 沒有送出：一個欄位都沒填。`);
        return;
      }
      parts.push(`confirm=${inp.value.trim()}`);
      value = parts.join(";");
    } else if (btn.dataset.valueraw) {
      /*  AI(W906-1203ENC-1) 20260916: a command whose choice is SEVERAL fields
          at once. Pn21D is one write carrying two digits -- enable and
          resolution -- and they are not independent: "off" still has to say
          which selection digit to leave in place. Splitting them into two
          controls would let an operator set half of a read-modify-write and
          wonder why nothing changed, so each button carries a COMPLETE choice
          and data-valueraw is that choice, already formatted. */
      value = `${btn.dataset.valueraw};confirm=${inp.value.trim()}`;
    } else if (btn.dataset.from !== undefined) {
      /*  AI(W906-1203DHOME-1) 20260917: A TYPED NUMBER **AND** A CONFIRMATION.
          The drive's homing speeds are the first control that needs both: the
          value is free-form (a speed, not a choice of two), and the write is
          addressed by STATION on a ring where two drives share an address, so
          it has to be confirmed like every other drive write.
          ⚠ data-from was ALREADY handled in the unconfirmed path below and was
          NOT handled here, so a button carrying both would have fallen through
          to the final `else` and sent "confirm=41" with NO VALUE. The C++ side
          refuses that ("has no val="), which is the good outcome -- but it
          reads as a server bug rather than as the browser dropping the number,
          and this page has spent long enough looking broken.
          ⓘ Blank is refused rather than sent as 0. On 6099h:2 a zero is a
          zero-speed search for the origin, i.e. a home that never finishes. */
      const n = valueOf(btn.dataset.from);
      if (n === null) {
        note(`${cmd} 沒有送出：「${btn.dataset.from}」不是數字。`);
        return;
      }
      value = `${key}=${n};confirm=${inp.value.trim()}`;
    } else {
      value = (btn.dataset.value !== undefined)
        ? `${key}=${btn.dataset.value};confirm=${inp.value.trim()}`
        : `confirm=${inp.value.trim()}`;
    }
    transport.send({ cmd, tag, value });
    note(`sent ${cmd} ${tag || ""} ${JSON.stringify(value)} — 看「最後一筆指令」` +
         `確認驅動器怎麼回應。這一行只代表瀏覽器送出去了。`);
  });
  back.addEventListener("click", e => { if (e.target === back) close(); });
}

function onStageClick(ev) {
  const btn = ev.target.closest ? ev.target.closest("button[data-cmd]") : null;
  if (!btn) return;
  ev.preventDefault();

  const cmd = btn.dataset.cmd;
  const tag = btn.dataset.tag;
  let value;

  /*  AI(W906-1203CTL-14) 20260911: REFUSE LOCALLY RATHER THAN LET THE SERVER
      REFUSE SILENTLY. Without the operator token every machine command comes
      back ok:false "not-operator" -- correct, and invisible: the ack is not
      rendered anywhere and the machine's own last-command tags never change,
      because the command never reached the publisher at all. So a whole page of
      buttons would appear dead. Say it here instead, in the one place that
      knows.
      ⚠ The token commands themselves are exempt, obviously -- they are how the
      condition is fixed, and gating them behind itself would make the page
      permanently unusable. */
  /*  AI(W906-1203ALM-12) 20260914: RECONNECT IS HANDLED HERE AND FIRST.
      It is the one control that must work while the link is DOWN, so it sits
      ahead of both the token gate and the offline gate -- gating the way out of
      a broken connection behind the connection is how a page becomes
      permanently stuck. */
  if (cmd === "ui.reconnect") {
    if (transport && transport.reconnectNow) {
      transport.reconnectNow();
      note("重新連線中…");
    } else {
      note("這個 transport 不支援重新連線（mock 模式沒有連線可重建）。");
    }
    return;
  }

  /*  ⚠ A COMMAND SENT INTO A CLOSED SOCKET USED TO VANISH. js/transport/ws.js
      dropped it with a console.warn and returned undefined, so pressing a
      button while offline produced no note, no refusal and no ack -- the page
      simply did nothing, which is precisely "斷線了按鈕會都沒反應". Ask the
      transport before offering to send, and say so when the answer is no. */
  if (transport && transport.isOpen && !transport.isOpen()) {
    const when = linkState && linkState.retryAt
      ? `，約 ${Math.max(0, Math.ceil((linkState.retryAt - Date.now()) / 1000))} 秒後自動重試`
      : "";
    note(`${cmd} 沒有送出：目前與伺服器斷線${when}。` +
         `上方的「重新連線」可以立刻重試，不必等。`);
    return;
  }

  if (!hasControl && TOKEN_CMDS.indexOf(cmd) === -1) {
    /*  AI(W906-1203ALM-11) 20260914: the page now takes the token by itself, so
        "press TAKE CONTROL first" is no longer the useful instruction -- it
        would send the operator to a button that is already being pressed for
        them. Say which of the two real causes applies instead. */
    note(`${cmd} 沒有送出：這個分頁目前沒有操作權。` +
         (autoControl
           ? `自動取得正在重試 —— ${lastAcqRefusal
                ? `伺服器上次的理由是「${lastAcqRefusal}」。`
                : `等連線就緒。`}`
           : `你先前按過「釋放操作權」，自動取得已關閉；按「取得操作權」重新開啟。`));
    maybeAutoAcquire();
    return;
  }

  if (btn.dataset.homefrom !== undefined) {
    /* Home carries TWO operator choices and the C++ side refuses a home whose
       mode is unspecified rather than assuming one -- mode 0 is MODE1_ABS, and
       on an axis wired for a limit-switch home that commands a move toward a
       switch that will not stop it. So both are read, both must parse, and the
       pair travels as one explicit string. */
    const ax   = btn.dataset.homefrom;
    const mode = valueOf(`${ax}.homeMode`);
    const dir  = valueOf(`${ax}.homeDir`);
    if (mode === null || dir === null) {
      note(`HOME not sent: mode and direction must both be numbers (got mode=` +
           `${mode === null ? "?" : mode}, dir=${dir === null ? "?" : dir}).`);
      return;
    }
    value = `mode=${Math.trunc(mode)};dir=${dir < 0 ? -1 : 1}`;
  } else if (btn.dataset.imposefrom !== undefined) {
    /*  AI(W906-1203CTL-44) 20260911: 疊加運動 carries TWO operator numbers --
        Acm_AxMoveImpose(ax, Position, NewVel) -- so the pair travels as one
        explicit string for the same reason HOME does: neither half has a safe
        default, and a composite that can arrive half-filled is worse than one
        that refuses. */
    const ax  = btn.dataset.imposefrom;
    const pos = valueOf(`${ax}.impPos`);
    const vel = valueOf(`${ax}.impVel`);
    if (pos === null || vel === null) {
      note(`疊加運動 not sent: 疊加距離 and 疊加速度 must both be numbers (got pos=` +
           `${pos === null ? "?" : pos}, vel=${vel === null ? "?" : vel}).`);
      return;
    }
    value = `pos=${pos};vel=${vel}`;
  } else if (btn.dataset.from !== undefined) {
    const v = valueOf(btn.dataset.from);
    if (v === null) {
      note(`${cmd} not sent: "${btn.dataset.from}" is not a number.`);
      return;
    }
    /*  AI(W906-1203ALM-10) 20260914: one distance box, two arrows. The left one
        carries data-negate so it sends −distance from the SAME field the right
        one reads, which is how the Utility's single 距離 works. Negating here
        rather than in the view keeps the box holding what the operator typed. */
    value = (btn.dataset.negate === "1") ? -v : v;
  } else if (btn.dataset.rawvalue !== undefined) {
    /*  AI(W906-1203HOMEBTN-1) 20260917: a button that carries its WHOLE value
        string, for a command whose payload is a composite this button already
        knows. 一鍵歸原點 is that: the method and the direction are fixed by
        which button was pressed, so there is nothing to read from a box and
        nothing that can arrive half-filled.
        ⚠ Distinct from data-value, which is a single NUMBER and is coerced
        with Number() -- "mode=128;dir=-1" through that path becomes NaN and
        the machine gets a command it cannot parse. */
    value = btn.dataset.rawvalue;
  } else if (btn.dataset.value !== undefined) {
    value = Number(btn.dataset.value);
  }

  /*  ws.js assigns the wire id internally and does not hand it back, so the
      correlation is by NAME: only one token command can be outstanding at a
      time in practice (they are two buttons a human presses). Recording the
      name is enough to interpret the next ack, and it is honest about being
      approximate -- see onAck. */
  /*  AI(W906-1203ALM-11) 20260914: pressing 取得操作權 by hand re-arms the
      automatic hold, which a previous 釋放操作權 had switched off. The button
      is how the operator says "I want it again", and that includes wanting it
      back after the next reconnect. */
  if (cmd === "control.acquire") { autoControl = true; lastAcqRefusal = null; nextAcqAt = 0; }

  //AI(W906-1203TOKEN-1) 20260922: ⚠ THE SAME SET-BEFORE-SEND SHAPE IS HERE, and
  //  for control.release it has a different but real consequence. This marks the
  //  command pending; the actual transport.send happens BELOW, and for a
  //  confirmed command it may not happen at all (askConfirm returns and the
  //  operator can press 取消). A stale "control.release" entry is not a latch --
  //  nothing guards on it -- but onAck() checks it FIRST, so the next ack of ANY
  //  kind would be consumed as if it were the answer to a release that was never
  //  sent, and hasControl would be cleared on a page that still holds the token.
  //  ⓘ Marked only for commands that are about to go out unconditionally. The
  //  token commands never carry data-confirm, so this is equivalent today; it is
  //  written defensively because "equivalent today" is how the acquire latch got
  //  in.
  if (TOKEN_CMDS.indexOf(cmd) !== -1 && btn.dataset.confirm === undefined) {
    pendingAcks.set(cmd, Date.now());
  }

  /*  AI(W906-1203WHYBOX-1) 20260918: remember the name of the last command this
      page sent, purely so a GATEWAY refusal can name it. On that path the
      machine's own pci1203.control.lastCmd is unchanged -- the frame never got
      there -- so without this the dialog could only say "something was
      refused". ⚠ Approximate on purpose, exactly like pendingAcks above: it is
      the last one SENT, and in practice these are buttons a human presses one
      at a time. It is never used to decide anything, only to label a message. */
  pendingCmdName = cmd;

  /*  AI(W906-1203ALM-5) 20260912: IRREVERSIBLE COMMANDS GO THROUGH A DIALOG.
      User request: "按下時 有格確認視窗 按下事之後 才啟動Fn008".
      The operator must TYPE the station number, not click a second OK -- a
      second OK is muscle memory by the third use, while typing the station is
      them saying which drive they mean. The typed number becomes the wire
      value "confirm=<n>" and the C++ side refuses it if it does not match that
      axis's own station, so the confirmation is checked, not merely collected. */
  if (btn.dataset.confirm !== undefined) {
    askConfirm(btn, cmd, tag);
    return;
  }

  transport.send({ cmd, tag, value });
  note(`sent ${cmd} ${tag || ""} ${value === undefined ? "" : JSON.stringify(value)} ` +
       `— watch “last command” in the Control section for what the machine did ` +
       `with it. This line only means the browser sent it.`);
}

/*  AI(W906-1203CTL-14) 20260911: the ack path.
 *
 *  ⚠ THE ONLY THING THIS INFERS IS THE TOKEN. An ack tells us the GATEWAY
 *  accepted and relayed a command; it says nothing about what the machine did,
 *  and the machine's answer arrives as tags like every other machine fact. So
 *  this never reports success for a machine command -- it would be reporting
 *  the relay's opinion of an operation it cannot see.
 *
 *  The token is different: control.acquire and control.release are answered BY
 *  THE SERVER ITSELF (WebBridgeServer.cpp:1212-1231, from the socket thread,
 *  touching nothing but server state), so for those two the ack IS the outcome.
 */
/*  AI(W906-1203ALM-11) 20260914: ask for the token if we should have it and do
    not. Safe to call often -- it is gated on the link being up, on not already
    holding it, on no acquire being in flight, and on a retry interval. */
/*  AI(W906-1203TOKEN-1) 20260922: ⚠⚠ THIS FUNCTION COULD LATCH ITSELF OFF
    PERMANENTLY, AND THAT IS THE "開久就沒辦法控制馬達" BUG.

    Reported twice and never proven until an operator sent a screenshot of the
    new failure dialog reading:
        ⚠ 指令沒有離開這台電腦
        指令：pci1203.do.setBit
        原因：not-operator
    on a page whose header said 14758 frames -- about an hour at a 250 ms tick.

    THE LATCH, exactly:
        if (pendingAcks.has("control.acquire")) return;   // the guard
        ...
        pendingAcks.set("control.acquire", now);          // marked BEFORE send
        try { transport.send(...); } catch { }            // send failure SWALLOWED

    onAck() is the ONLY place that deletes that entry. So if the send throws --
    which is exactly what happens while a socket is closing, and the old catch
    block's own comment ("socket going") says the author knew it could -- the
    page is left marked "an acquire is in flight" for a request THAT WAS NEVER
    SENT. No
    ack can ever arrive, the guard returns on every later call, and the page
    never asks for the token again for the rest of its life. The feed keeps
    reconnecting and the screen keeps updating, so everything looks healthy;
    only the buttons are dead.
    ⓘ The timestamp stored in that Map was never read by anything. It was
    plainly meant to expire and the expiry was never written.

    THREE FIXES, because the latch has three independent ways in:
      1. Mark AFTER a successful send, not before. A request that never left
         must not leave a flag behind claiming it did.
      2. Expire a pending acquire. Any other way an ack can vanish -- a dropped
         frame, a gateway restart between send and reply -- self-heals in
         seconds instead of never. This is what the unread timestamp was for.
      3. Drop it when the link goes down (see the status handler). An ack for a
         connection that no longer exists is never coming. */
const ACQ_PENDING_MS = 5000;

function maybeAutoAcquire() {
  if (!autoControl || hasControl) return;
  if (!transport || !transport.send) return;
  if (!linkState || linkState.state !== "online") return;

  const now = Date.now();
  //  (2) EXPIRE A STALE PENDING ACQUIRE instead of treating it as forever.
  //  ⚠ The comparison is on the value that was already being stored and never
  //  read -- this is not new state, it is the state the original code created
  //  and then ignored.
  const pendingSince = pendingAcks.get("control.acquire");
  if (pendingSince !== undefined) {
    if (now - pendingSince < ACQ_PENDING_MS) return;
    pendingAcks.delete("control.acquire");
    note("取得操作權的回應超過 " + (ACQ_PENDING_MS / 1000) +
         " 秒沒有回來，視為遺失，重新要求一次。");
  }

  if (now < nextAcqAt) return;
  nextAcqAt = now + 4000;

  //  (1) MARK ONLY IF IT ACTUALLY WENT OUT.
  try {
    transport.send({ cmd: "control.acquire" });
    pendingAcks.set("control.acquire", now);
  } catch {
    /*  The socket is going away. Leave NO pending mark: the next call must be
        free to try again once the link is back, which is the whole failure
        this function had. */
  }
}

function onAck(msg) {
  if (!msg || msg.type !== "ack") return;

  if (pendingAcks.has("control.acquire")) {
    pendingAcks.delete("control.acquire");
    hasControl = (msg.ok === true);
    dirty = true;
    if (hasControl) {
      lastAcqRefusal = null;
      note("已取得操作權 — this page holds the operator token.");
    } else {
      /*  ⚠ PRINT A REFUSAL ONCE, NOT EVERY RETRY. Auto-acquire retries every
          few seconds, and a page that logged "refused" on each attempt would
          bury everything else -- including the answer the operator is waiting
          for. Repeat only when the REASON changes. */
      const why = msg.error || "(no reason given)";
      if (why !== lastAcqRefusal) {
        lastAcqRefusal = why;
        note(`取得操作權 refused: ${why}. ` +
             `"control-held" means another connection has it (another tab, or ` +
             `another operator); "read-only" means the gateway was started ` +
             `without WB_GATEWAY_ALLOW_COMMANDS=1. Retrying quietly.`);
      }
    }
    return;
  }
  if (pendingAcks.has("control.release")) {
    pendingAcks.delete("control.release");
    if (msg.ok === true) {
      hasControl = false;
      dirty = true;
      /*  A deliberate release turns auto-acquire off. Grabbing the token
          straight back would make 釋放操作權 look like a broken button. */
      autoControl = false;
      note("已釋放操作權 — 自動取得已關閉，按「取得操作權」會重新開啟。");
    } else {
      note(`RELEASE refused: ${msg.error || "(no reason given)"}`);
    }
    return;
  }

  /* Any other failing ack is worth surfacing verbatim: it means the command
     never reached the publisher, so the machine's last-command tags will NOT
     change and the operator would otherwise see nothing at all. */
  if (msg.ok === false) {
    note(`the gateway refused that command: ${msg.error || "(no reason given)"} ` +
         `— it did not reach the machine, so “last command” below is unchanged.`);
    /*  AI(W906-1203WHYBOX-1) 20260918: ⚠ THIS PATH NEEDS ITS OWN DIALOG AND IS
        EASY TO MISS. A gateway refusal means the frame never reached the
        publisher, so pci1203.control.* DOES NOT CHANGE -- checkCommandFailure()
        is keyed on lastId and would never fire. From the operator's seat this
        is the most confusing failure of all: the button does nothing AND the
        "last command" strip still shows the previous, successful command.
        The commonest cause by far is not holding the operator token. */
    showFailureBox({
      title: "⚠ 指令沒有離開這台電腦",
      cmd: pendingCmdName || "(不明)",
      why: msg.error || "(沒有給原因)",
      hint: "這是閘道擋下來的，機器完全沒有收到 —— 所以下面「最後一筆指令」" +
            "顯示的還是上一筆，不是這一筆。\n" +
            "最常見的原因是沒有「操作權」：看畫面最上面那一列有沒有寫「已取得操作權」，" +
            "沒有的話按「取得操作權」。\n" +
            "若寫的是 read-only，代表 wb_gateway 啟動時沒有帶 " +
            "WB_GATEWAY_ALLOW_COMMANDS=1。"
    });
  }
}

/* A one-line notice above the report. Deliberately NOT an outcome: the outcome
   comes back through the tag feed like every other machine fact, and a browser
   that printed "OK" here would be reporting its own optimism. */
function note(text) {
  const w = document.getElementById("cmdnote");
  if (!w) return;
  w.hidden = false;
  w.textContent = text;
}

function boot() {
  const q = new URLSearchParams(location.search);
  const onMock = (q.get("src") || "mock").toLowerCase() === "mock";

  if (onMock) {
    /*  Two independent defences against the mock, because this tree records
        that a page which shows plausible fake data is the hardest kind of
        "it works" to catch:
          (1) this banner, and
          (2) the rail link in js/panels/chrome.js hard-codes ?src=ws.
        js/transport/index.js:18 is `q.get("src") || "mock"` and the mock
        carries no pci1203.* tags, so without the parameter every cell here
        reads "n/a" -- honest, but useless. */
    const w = document.getElementById("mockwarn");
    w.hidden = false;
    w.textContent =
      "This page is on the MOCK transport, which carries no pci1203.* tags. " +
      "Everything below will read n/a. Reload with ?src=ws to see the real card.";

    /*  js/transport/mock.js:52 is `connect(handler)` -- ONE parameter. It never
        calls a status handler, so linkState would read "connecting" forever and
        the subtitle would describe a connection attempt that is not happening.
        State it correctly instead. */
    linkState = { state: "mock (no live feed)" };
  }

  transport = createTransport();
  /*  THREE arguments. js/transport/mock.js's connect takes ONE and simply
      ignores the rest, which is why the ack handler is optional on the wire
      side too -- a page on the mock transport never sends anything and never
      needs one. */
  transport.connect(
    data => {
      for (const [k, v] of Object.entries(data)) tags.set(k, v);
      //AI(W906-SJSON-S9b) 20260923: 展開 io.di／io.do 兩個平面 -> 512 個
      //  per-port key。**必須在上面那個 tags.set 迴圈之後**（unpackPlane 讀的
      //  是 tags 裡剛寫進去的 io.di），也必須在 dirty=true 之前。
      unpackIoPlanes(data);
      frames += 1;
      dirty = true;
      /*  AI(W906-1203WHYBOX-1) 20260918: check for a failed command HERE, after
          the tags are in and before the redraw. ⚠ Not inside render(): render
          runs on a timer and for reasons unrelated to a new result, so a check
          living there would have to re-derive "is this new" from state render
          does not own. The tag handler is the one place that knows a result
          just arrived. */
      checkCommandFailure();
    },
    status => {
      /*  ⚠ THE TOKEN IS PER CONNECTION, so a reconnect loses it. A page that
          went on believing it held one would send commands that are all
          refused while the screen said "HELD" -- the exact mismatch this whole
          section exists to remove. */
      /*  AI(W906-1203TOKEN-1) 20260922: (3) A PENDING ACQUIRE DIES WITH THE
          CONNECTION. The ack would have come back on a socket that no longer
          exists, so it is never coming -- and leaving the mark in place is one
          of the three ways maybeAutoAcquire() used to latch itself off for
          good. ⚠ Cleared on ANY non-online status, not only when we held the
          token: the request in flight is just as dead either way, and the case
          that bites is precisely the one where we did NOT have it yet. */
      if (status && status.state !== "online") {
        pendingAcks.delete("control.acquire");
      }
      if (hasControl && status && status.state !== "online") {
        hasControl = false;
        /*  AI(W906-1203ALM-11) 20260914: no longer "press TAKE CONTROL again".
            Auto-acquire takes it back when the link returns, unless the
            operator had deliberately released it. */
        note(autoControl
          ? "the feed dropped, so the operator token went with the connection. " +
            "It will be taken again automatically when the link is back."
          : "the feed dropped and the operator token went with it.");
      }
      linkState = status;
      dirty = true;
      /*  Ask immediately on the transition rather than waiting for the next
          timer tick: the common case is a reconnect, and a visible gap where
          every button refuses is exactly what this change removes. */
      maybeAutoAcquire();
    },
    onAck,
  );

  /*  Hand the token back on the way out. Best-effort: if the socket has
      already gone there is nothing to send, and the server drops a departed
      connection's token anyway -- this just makes the common case (closing the
      tab) release it immediately instead of leaving the next operator locked
      out until the server notices. */
  window.addEventListener("beforeunload", () => {
    if (hasControl && transport && transport.send) {
      try { transport.send({ cmd: "control.release" }); } catch { /* going away */ }
    }
  });

  /* AI(W906-1203CTL-10) 20260911: bound ONCE, on the container rather than on
     any button -- render() replaces the report's entire contents ~4 times a
     second, so a listener on a button would not survive the next frame. */
  const stage = document.getElementById("stage");
  if (stage) stage.addEventListener("click", onStageClick);

  /*  AI(W906-1203CTL-31) 20260911: RAIL CLICKS DO NOT RELOAD THE PAGE.
   *
   *  Each entry is still a real <a href="?..."> -- so it is still a link, still
   *  bookmarkable, still openable in a new tab with the middle button, and
   *  view.js is still pure. What this listener does is handle the ordinary
   *  left-click itself: pushState + re-render, no document reload, so the
   *  rail's scroll position simply never moves.
   *
   *  ⚠ MODIFIED CLICKS ARE LEFT ALONE. Ctrl/Shift/middle-click mean "open this
   *  somewhere else" and swallowing them would break a link that still looks
   *  like one. Only the plain left-click is intercepted.
   */
  const railEl = document.getElementById("rail");
  if (railEl) {
    railEl.addEventListener("click", ev => {
      if (ev.defaultPrevented || ev.button !== 0 ||
          ev.metaKey || ev.ctrlKey || ev.shiftKey || ev.altKey) return;
      const a = ev.target.closest ? ev.target.closest("a.railitem") : null;
      if (!a) return;
      const href = a.getAttribute("href");
      if (!href || href.charAt(0) !== "?") return;
      ev.preventDefault();
      history.pushState(null, "", href);
      dirty = true;
      render();
      /* ⚠ The PANE is scrolled to the top, the RAIL is not. Selecting a card
         should show that card from its beginning; it should not move the list
         the operator is choosing from. That asymmetry is the whole request. */
      const st = document.getElementById("stage");
      if (st) st.scrollTop = 0;
      window.scrollTo(0, 0);
    });
  }
  /* Back/forward must work, or the URL stops being a real address. */
  window.addEventListener("popstate", () => { dirty = true; render(); });

  /*  AI(W906-1203ALM-11) 20260914: keep the operator token held.
      ⚠ On its own timer, NOT inside tick(). tick() runs on
      requestAnimationFrame, which the browser THROTTLES OR STOPS ENTIRELY in a
      background tab -- so the one case that most needs the token taken back (a
      panel left open on a second monitor while the operator works elsewhere)
      is the case rAF would not serve. maybeAutoAcquire is internally gated on
      the link, on not already holding it, and on its own 4 s floor, so a 2 s
      timer costs nothing when there is nothing to do. */
  setInterval(() => {
    maybeAutoAcquire();
    /*  AI(W906-1203ALM-12) 20260914: ⚠ REPAINT WHILE OFFLINE, or the countdown
        freezes. render() only runs when `dirty`, and `dirty` is set by arriving
        patches -- which is exactly what a disconnection stops. So the one
        moment the page must keep moving is the one moment nothing would move
        it. A frozen "15 秒後自動重試" is worse than no countdown: it reads as a
        hung page. */
    if (linkState && linkState.state !== "online") {
      dirty = true;
      /*  ⚠ PUSH THE CURRENT retryAt INTO THE NODE. Measured: the countdown sat
          on "正在重試…" forever while its data-retryat stayed frozen at the
          FIRST attempt's timestamp. Each scheduleReconnect does send a fresh
          retryAt, but the state string stays "offline", so shapeKey does not
          change, so the banner is never rebuilt and keeps the stale dataset.
          Updating the attribute here is better than adding retryAt to shapeKey:
          that would rebuild the whole report on every retry, destroying a
          half-typed box each time. */
      const cd = document.querySelector(".offlinecd");
      if (cd && linkState.retryAt) cd.dataset.retryat = String(linkState.retryAt);
    }
  }, 1000);
  maybeAutoAcquire();

  requestAnimationFrame(tick);
}

boot();
