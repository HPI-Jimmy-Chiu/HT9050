/* ==========================================================================
   pci1203/view.js -- the PCIE-1203 monitor page, PURE rendering half.

   AI(W906-MW2) 20260908. Renders the pci1203.* tag family that
   WebBridgeTags.cpp publishes from EtherCAT/Pci1203Monitor.h.

   WHY THIS IS SPLIT FROM js/pci1203.js
   Everything here is a function of ONE argument -- a Map of tag -> value -- and
   returns DOM nodes. It touches no transport, no location, no timer, no
   getElementById. That is what lets web/tools/jsprobe/probe_pci1203.mjs EXECUTE
   it against a real captured snapshot off the live publisher, rather than
   someone eyeballing the page and calling that a test. The first version of
   this file did its own booting at import time and could not be tested at all;
   the shim would have needed location/requestAnimationFrame/WebSocket stubs
   just to reach the logic worth checking.

   READ-ONLY, AND STRUCTURALLY SO -- NOT AS A PROMISE
   Nothing here creates a form control, binds a listener, or sends anything.
   That is not politeness: the observer behind these tags can only make the
   eighteen read-only vendor calls on its allowlist (Pci1203Monitor.h),
   enforced mechanically by tools/pci1203_readonly_gate.ps1. A control on this
   page would have nothing to talk to, so a control that LOOKED live would be a
   lie about the machine.

   ⚠ TWO CORRECTIONS TO THIS PARAGRAPH, AI(W906-MW2b) 20260910:

   (a) "twelve" was right on 20260908 and is not now. The allowlist grew to
       eighteen when station identity, the IO map and DO read-back were added.
       A paragraph that states a count has to be re-stated when the count
       changes, or it becomes the decoration it was written to avoid being.

   (b) "The probe asserts the built DOM contains zero controls" WAS NOT TRUE.
       No such check existed anywhere in probe_pci1203.mjs -- the only matches
       for "control" were the phrase "control group" in a test name. A stated
       assertion that does not exist is worse than an unstated one, because it
       gets cited as evidence. It exists now (section 5c), and it is narrower
       and more honest than the old sentence: ZERO FORM CONTROLS
       (button/input/select/textarea/form/option) and ZERO LISTENERS, while
       LINKS ARE ALLOWED. A link re-filters this page; a button implies
       something to send, and there is no inbound path for one to use.

   ===========================================================================
   THE ONE THING THIS PAGE EXISTS TO GET RIGHT
   ===========================================================================
   On this machine, TODAY, almost every value here is null -- measured
   20260908 on the first real card open:

       card OPEN  devName "PCIE-1203-32AE (M0)"  subDevices=0  axesOpened=0
       mode=owned  slavesFound=0  axByIdMode=false
       disabled=true  "auto-disabled after 10 consecutive polls in which
                       every read failed"

   A screen full of "---" has at least FIVE different causes, and they need
   five different actions from whoever is looking at it:

     1. the tag is not on this wire at all   -> on the mock transport, or
                                                talking to an older build
     2. pci1203.linked = false               -> this exe was built without the
                                                Advantech SDK
     3. pci1203.enabled = false              -> INSTALL_1203_MONITOR is not
                                                defined in MachineType.h
     4. pci1203.open = false                 -> the card would not open;
                                                lastErrorText says why
     5. all of the above are fine and values are STILL null
                                             -> the card is present and EMPTY:
                                                nothing on the EtherCAT ring

   Collapsing any of those into "no data" is the failure mode this page is
   designed against. #5 is the current state of this machine and it is NOT a
   fault of the software.

   ===========================================================================
   THREE DISPLAY STATES, NOT TWO  (the same rule D:\BCB6_1203_UI uses)
   ===========================================================================
       "n/a"   the tag is absent from the feed        -> a SOFTWARE question
       "---"   the tag is present and explicitly null -> nobody read it
       value   a real reading
   The BCB6 client shipped a bug once by collapsing null into false and then
   asserting a specific, WRONG cause for an empty panel (recorded at Unit1.cpp's
   AI(W906-1203MON-17) note). Same trap, same rule, here.

   ⚠ NULL IS NEVER RENDERED AS 0. An axis at position 0.000 and an axis nobody
   could read are different facts, and on a machine that homes to 0 they are
   dangerously similar. WebBridgeTags.h calls this "NULL, NEVER ZERO"; this file
   is the display end of that contract, and the probe checks it.
   ========================================================================== */

/* AI(W906-MW2b) 20260910: sizes raised to the fieldbus this machine ACTUALLY
   has, and a DO family added.
     ⚠ THE RING MOVED THREE TIMES WHILE THIS WAS BEING WRITTEN, so no single
     measurement below is "the" topology -- the machine is being wired:
       13 stations, 168 DI ch   8 junctions + 5 x ECx-P32-HON 32DI
       18 stations, 264 DI ch   + a DO module (ECx-C32-HON 32DO)
       18 stations, 744 DI ch   + 9 Yaskawa SGDXW/SGDXS SERVOPACKs, all
                                  reading alias 0 -- which is what put
                                  EC_SubDeviceIDConflicted back
     The slot counts below are therefore a WINDOW, not a description of the
     hardware, and they are sized generously on purpose.
   The old DI_PORTS=4 could show 32 of 168 channels and the old SLAVE_SLOTS=8
   could hold 8 of 13 stations -- and BOTH overflowed silently: the loop simply
   stops, producing a short list that looks complete. That is the same failure
   shape as a truncated scan, which this page already warns loudly about.
     ⚠ THESE MIRROR kPci1203Tag* IN EtherCAT/Pci1203Monitor.h. A mismatch is
     not an error anywhere: extra rows just read "n/a" forever, which looks
     exactly like a card that is not answering. Check 6 of probe_pci1203.mjs
     (the typo gate) is what actually catches a drift, because it asserts every
     tag this file reads exists on a REAL captured wire. */
/* AI(W906-1203CTL-9) 20260911: AXIS_SLOTS 8 -> 16, mirroring kPci1203TagAxes
   which went 8 -> 16 the same day. The servo drives arrived on the Motion ring
   overnight: ring 0 carries NINE Yaskawa SGDXS/SGDXW SERVOPACKs, and eight
   published slots could not show the ninth. Exactly the silent truncation the
   paragraph above describes, one day later and on a different family. */
/* AI(W906-1203CTL-17) 20260911: DI 32 -> 96, DO 8 -> 72, slaves 32 -> 48,
   mirroring kPci1203Tag* the same hour. Measured on the card: 744 DI channels
   (93 ports) and 552 DO channels (69 ports). The page was showing 32 of 93 DI
   bytes and 8 of 69 DO bytes -- and showing them with nothing to say the rest
   existed, because the loop simply stops. Third time in four days.
   ⚠ These MUST NOT exceed the C++ side's kPci1203Tag* or the extra rows read
   "n/a" forever, which looks exactly like a card that is not answering. Check 6
   of probe_pci1203.mjs (the typo gate) is what actually catches a drift, by
   asserting every tag this file reads exists on a REAL captured wire. */
/* AI(W906-1203CTL-33) 20260911: DI 96 -> 128, DO 72 -> 96 -- and, much more
   importantly, THESE STOPPED BEING THE NUMBER THE PAGE ACTUALLY ITERATES.

   The comment directly above already said "third time in four days" and still
   mirrored a constant. It happened a fourth time, and a user found it:
   "為什麼我站號50的IO是好好的，但是51的IO都沒辦法正常顯示". Station 0x51 owns
   ports 95..98 and so straddled DI_PORTS=96 -- one byte of four. Stations
   0x52/0x53/0x54, entirely past it, produced NO symptom at all: they simply
   were not in the list, which reads as "not installed" rather than "not read".

   Then the same user asked the right question: "你應該不是寫固定的吧? 不是去
   偵測每個模組有幾個IO 去配置嗎?". Mirroring a constant by hand is exactly the
   thing that keeps failing, so the count now comes OFF THE WIRE:
   pci1203.di.portsSampled is what the C++ side actually polled, which it in
   turn sized from the card's own FT_DaqDiMaxChan. Use diPortCount(tags) /
   doPortCount(tags), not these.

   ⚠ The constants remain for two jobs they are still right for: an upper bound
   when scanning for tags before the count is known, and a fallback for a
   publisher too old to send the count. They MUST NOT exceed the C++ side's
   kPci1203Tag* or the extra rows read "n/a" forever, which looks exactly like a
   card that is not answering. */
/*  AI(W906-1203TRIM-1) 20260917: ⚠ THE homemodes.js IMPORT IS GONE.
        import { HOME_MODES, homeModeByValue } from "./homemodes.js";
    It fed the mode picker and the vendor diagram, both removed today, and an
    unused ES import is not free: the browser still fetches and parses the whole
    11.5 KB module on every page load. Worse than the bytes, it described the
    sixteen "typical" card modes -- the ones these axes answer 0x8000510F to --
    so anyone who went reading it would have been reading about a path this
    machine cannot take.

    ⓘ web/js/pci1203/homemodes.js and the sixteen web/img/homemode/*.png STAY ON
    DISK. They are generated artefacts (scratchpad/genhomemodes.ps1 from
    Advantech's own FrmHomeMode.resx, not typed by hand -- the mode NUMBER and
    the card VALUE differ by one, MODE13 is 12, and sixteen transcribed Chinese
    descriptions is how a wrong explanation ends up on a machine screen looking
    authoritative). A machine whose axes DO accept the card's own home modes
    needs them. Nothing loads them here any more. */

/*  AI(W906-1203AXMAP-2) 20260915: 16 -> 32, mirroring kPci1203TagAxes the same
    hour. With the axis count corrected this ring needs 20 (6 SGDXW x2 + 3 SGDXS
    + 5 SW3D-680) and sixteen slots published four fewer than exist -- the
    silent truncation this file's own banner warns about, again. 32 is
    FT_DevSupportAxesCount, the card's measured hardware ceiling. */
export const AXIS_SLOTS  = 32;   // = kPci1203TagAxes    (Pci1203Monitor.h)
//AI(W906-1203IOWIDE-1) 20260918: 128 -> 320 and 96 -> 192, tracking
//  kPci1203TagDiPorts / kPci1203TagDoPorts. ⚠ THIS IS THE SECOND HAND-COPY OF
//  A C++ CONSTANT and it has to move in the same commit: if the publisher emits
//  320 DI ports and this file still says 128, the page silently stops at 128
//  and the extra modules are invisible again -- the same symptom the C++ change
//  was made to fix, arriving from the other side.
//  The card measured 172 DI / 94 DO ports on 20260918.
export const DI_PORTS    = 320;  // = kPci1203TagDiPorts -- upper bound/fallback
export const DO_PORTS    = 192;  // = kPci1203TagDoPorts -- upper bound/fallback
export const SLAVE_SLOTS = 48;   // = kPci1203TagSlaves

/*  AI(W906-1203ENC-1) 20260916: Pn21D n.□□X□, transcribed from the manual on
    this machine -- SIEPC71081205 Sigma-XW EtherCAT Product Manual p.193:
        4 Operate as 20-bit encoder      6 Operate as 22-bit encoder
        8 Default: operate as 24-bit     A Operate as 26-bit encoder
        Other values: Reserved (Do not use.)
    ⚠ The keys are DECIMAL because that is what the wire carries, so 26-bit is
    10 here and "A" in the manual. Anything not in this table is Reserved, and
    the C++ side refuses it -- these are not the only values that FIT in the
    nibble, they are the only ones that are defined. */
const ENC_BITS   = { 4: 20, 6: 22, 8: 24, 10: 26 };
const ENC_COUNTS = { 4: 1048576, 6: 4194304, 8: 16777216, 10: 67108864 };

/* How many IO ports this card actually has, as reported by the card itself and
   relayed by the publisher. Falls back to the wire shape when the tag is absent
   (older publisher) -- never to 0, which would render as "this card has no IO". */
export function diPortCount(tags) {
  const n = tags.get("pci1203.di.portsSampled");
  return (Number.isInteger(n) && n >= 0 && n <= DI_PORTS) ? n : DI_PORTS;
}
export function doPortCount(tags) {
  const n = tags.get("pci1203.do.portsSampled");
  return (Number.isInteger(n) && n >= 0 && n <= DO_PORTS) ? n : DO_PORTS;
}

/* ---------------------------------------------------------------------------
   AI(W906-1203CTL-9) 20260911: CONTROLS -- and how this file stays pure.

   This page can now command the card. That does NOT make view.js impure, and
   keeping it pure is what keeps probe_pci1203.mjs able to execute the whole
   rendering headless.

   THE RULE: this file emits DECLARATIVE elements and attaches NOTHING.
     * a control carries data-cmd / data-tag / data-value (or data-from, naming
       an input to read the value out of at click time);
     * it never has an inline handler and never gets addEventListener here;
     * js/pci1203.js attaches ONE delegated listener on the container and is the
       only place that talks to the transport.
   So the probe can still assert "zero listeners in the built DOM", and it gains
   a stronger assertion it could not make before: every actionable element
   carries a COMPLETE command triple. A button with a typo'd cmd name is caught
   in the probe instead of on a machine.

   ⚠ AND THE CONTROLS ONLY EXIST WHEN THE FEED SAYS THEY CAN WORK.
   pci1203.control.armed false -> no buttons are rendered at all, anywhere.
   Not disabled buttons: ABSENT ones. A disabled button still tells an operator
   "this is the thing that would do it", and on a machine screen the difference
   between "cannot" and "did not" is the whole message.
   --------------------------------------------------------------------------- */

/** Is the write surface armed on the far end? Three-state, and the three states
 *  are different facts:
 *    absent  -- an older publisher; this page cannot know, so: no controls
 *    false   -- compiled in but not armed, or compiled out entirely
 *    true    -- commands will be accepted (dry or live -- see controlDry) */
export function controlArmed(tags) {
  return tags.get("pci1203.control.armed") === true;
}

/** True when an accepted command will be VALIDATED AND LOGGED BUT NOT ISSUED.
 *  Defaults to true for anything other than an explicit false: if this page
 *  cannot establish that the far end is live, it must not tell an operator that
 *  a button moves a motor. */
export function controlDry(tags) {
  return tags.get("pci1203.control.dryRun") !== false;
}

/* The eight MotionIO bits the publisher decodes, in the order an operator reads
   them. Their VALUES were verified identical across the two vendor SDK versions
   on this box before any label was published at all -- see WebBridgeTags.cpp's
   kBits table. The keys here must match that table's suffixes exactly, or a
   lamp lights from the wrong tag while looking entirely plausible.
     good:true  -> ON is reassuring (SVON, RDY)
     good:false -> ON is alarming   (ALM, LMT+, LMT-, EMG)
     good:null  -> ON is neutral    (ORG, INP are positions, not verdicts) */
/*  AI(W906-1203CTL-34) 20260911: EIGHT LAMPS -> ALL TWENTY-ONE, in Common
    Motion Utility's own order and with its own labels, so the two screens can
    be read side by side. User: "可以把這張圖片有的東西 都加入到馬達介面?"

    The thirteen that were missing were not unavailable -- every one of them is
    a bit of the SAME Acm_AxGetMotionIO word this page was already reading, so
    they cost nothing to add and their absence cost real diagnosis. SLMT+/SLMT-
    are the sharpest example: those are the SOFT limits, and an axis stopped by
    a soft limit looked identical to one stopped for no visible reason.

    `good` is three-state on purpose: true = lit is healthy, false = lit is a
    problem, null = lit is neither (a direction or a latch), so the page can
    colour ALM red and DIR neutral without a second table. */
export const IO_BITS = [
  { key: "rdy",     label: "RDY",    good: true  },
  { key: "alm",     label: "ALM",    good: false },
  { key: "limitP",  label: "LMT+",   good: false },
  { key: "limitN",  label: "LMT-",   good: false },
  { key: "org",     label: "ORG",    good: null  },
  { key: "dir",     label: "DIR",    good: null  },
  { key: "emg",     label: "EMG",    good: false },
  { key: "pcs",     label: "PCS",    good: null  },
  { key: "erc",     label: "ERC",    good: null  },
  { key: "ez",      label: "EZ",     good: null  },
  { key: "clr",     label: "CLR",    good: null  },
  { key: "ltc",     label: "LTC",    good: null  },
  { key: "sd",      label: "SD",     good: false },
  { key: "inp",     label: "INP",    good: null  },
  { key: "svOn",    label: "SVON",   good: true  },
  { key: "ralm",    label: "RALM",   good: false },
  { key: "sLimitP", label: "SLMT+",  good: false },
  { key: "sLimitN", label: "SLMT-",  good: false },
  { key: "cmp",     label: "CMP",    good: null  },
  { key: "camDo",   label: "CAM-DO", good: null  },
  { key: "torLmt",  label: "TORLMT", good: false },
];

/* ---------------------------------------------------------------------------
   Cell formatting. `has` and `val` are deliberately separate: every render
   decision that could mislead keys on "is it on the wire" FIRST, then on the
   value. Merging them is how "absent" becomes indistinguishable from "null".
   --------------------------------------------------------------------------- */

/** The three-state cell. Returns {text, cls} -- never a bare string, because
 *  the caller must be able to style "n/a" differently from "---". */
export function cellOf(tags, tag, fmt) {
  if (!tags.has(tag)) return { text: "n/a", cls: "na" };
  const v = tags.get(tag);
  if (v === null || v === undefined) return { text: "---", cls: "null" };
  return { text: fmt ? fmt(v) : String(v), cls: "val" };
}

/* ===========================================================================
   AI(W906-1203CTL-27) 20260911: IN-PLACE REFRESH -- because rebuilding the
   whole page four times a second made it UNUSABLE.

   User: "左邊你怎一直更新? 我按鈕都按不下去 / 我是說IO更新率 不是畫面更新率
   0.2秒".

   ⚠ BOTH HALVES OF THAT ARE MY BUG, and the second is the cause of the first.
   render() replaced the entire report on every patch, ~5 times a second. A
   button that is destroyed and recreated between mousedown and mouseup NEVER
   RECEIVES THE CLICK -- so every control on the page was dead, and a number box
   lost what was being typed into it. The rail flickered for the same reason.

   And the conflation was mine: the user asked for a 0.2 s IO POLL. Nothing
   about that requires the DOM to be rebuilt at 0.2 s. The card is polled at
   200 ms; the page now UPDATES THE VALUES IN PLACE and only rebuilds when its
   STRUCTURE changes.

   HOW: every node whose content comes from a tag carries data-k (the tag) and
   optionally data-f (which formatter). refresh() walks those and writes text
   and class only -- it creates and destroys nothing, so a button under the
   cursor survives, and an <input> keeps both its value and the caret.
   =========================================================================== */

/*  ⚠ A FUNCTION, NOT A const OBJECT. The first version was
        const FORMATTERS = { pos: fmtPos, ... };
    declared here, above the `export const fmtPos = ...` definitions -- and a
    const is in the temporal dead zone until its own line runs, so merely
    LOADING this module threw "Cannot access 'fmtPos' before initialization".
    The probe caught it on the first run. A hoisted function defers the lookup
    to call time, which is when they exist. */
function fmtByName(name) {
  switch (name) {
    case "pos":     return fmtPos;
    case "hex2":    return fmtHex2;
    case "hex8":    return fmtHex8;
    case "bool":    return fmtBool;
    case "station": return fmtStation;
    case "jerkType": return jerkTypeText;   //AI(W906-1203CTL-44) 20260911
    //AI(W906-1203ALM-21) 20260915: limit flags. Shows the NUMBER as well as the
    //  word, because the number is what the set button writes and what the SDK
    //  header documents -- an operator comparing this against CFG_AxElEnable
    //  needs to see the 0 or the 1, not only "關".
    case "onoff":   return v => (v === 1 || v === "1") ? "1  開"
                              : (v === 0 || v === "0") ? "0  關"
                              : String(v);
    //  0 = 立即停止, 1 = 減速停止 -- CFG_AxElReact. Same rule: number first.
    case "elReact": return v => (v === 0 || v === "0") ? "0  立即停止"
                              : (v === 1 || v === "1") ? "1  減速停止"
                              : String(v);
    //AI(W906-1203GEAR-1/2) 20260915: the drive-side formatters.
    //  `int` keeps a gear numerator readable -- these run to 1073741824 and a
    //  float render would turn that into 1.073741824e+9 on a machine screen.
    case "int":     return v => (typeof v === "number") ? String(Math.round(v)) : String(v);
    case "torque":  return v => (typeof v === "number")
                              ? `${Math.round(v)} mN·m  (${(v / 1000).toFixed(2)} N·m)`
                              : String(v);
    /*  ⚠ Pn21D IS NOT "THE ENCODER RESOLUTION" and this formatter exists to stop
        the page saying it is. Manual p.178/p.193: n.□□□X is a COMPATIBILITY
        switch. Only when it is 1 does n.□□X□ (4/6/8/A = 20/22/24/26 bit)
        describe how the drive operates. When it is 0 -- the default, and
        measured n.0080 on all nine stations here -- the resolution is the
        MOTOR'S OWN and the manual sends you to the model number for it.
        Printing "24 bit" from the digit in that case would be a confident,
        wrong number on a machine screen. */
    case "encBits": return v => {
      if (typeof v !== "number") return String(v);
      const raw = Math.round(v);
      const en  = raw & 0xF;
      const sel = (raw >> 4) & 0xF;
      const bits = { 4: "20", 6: "22", 8: "24", 10: "26" }[sel];
      const hex = "n." + [(raw >> 12) & 0xF, (raw >> 8) & 0xF, sel, en]
                          .map(d => d.toString(16).toUpperCase()).join("");
      if (en === 1) return `${bits ? bits + " 位" : "保留值"}　${hex}　相容模式開啟`;
      return `依馬達規格（驅動器未公開）　${hex}　相容模式關閉` +
             (bits ? `　※若開啟會變成 ${bits} 位` : "");
    };
    /*  Pn002 n.□X□□ Encoder Usage -- p.709. This machine measured n.0111 on all
        nine stations, i.e. digit 2 = 1 = "use the encoder as an incremental
        encoder". That is worth showing because it explains behaviour elsewhere
        on this page: absolute position is not retained across power cycles, and
        the manual (p.604) lists this very setting as a condition under which
        Fn008 絕對編碼器重置 CANNOT be executed. */
    /*  AI(W906-1203HOME-1) 20260915: Pn000 n.□□□X Rotation Direction Selection
        (p.707). ⚠ Shows the WHOLE register, not just the digit, because an
        operator about to rewrite Pn000 should see what else is in it -- the
        write preserves n.X□□□ and the two reserved digits, and that claim is
        only checkable if the value is on screen. */
    /*  AI(W906-1203STORE-2) 20260915: can 套用 / 存檔 run right now?
        Both 2700h (s14.6.2 step 1) and 1010h:1 (p.585) require the drive to be
        in Switch ON Disabled -- servo OFF -- and the C++ side refuses otherwise.
        ⚠ Reported as a STATE, not as a disabled button. A greyed-out 套用 would
        say "cannot" without saying why, and "why" here is one SVOFF away. */
    case "canApply": return v => {
      if (typeof v !== "boolean" && typeof v !== "number") return String(v);
      const on = (v === true || v === 1);
      return on
        ? "⚠ 伺服 ON（運轉中）—— 現在按套用/存檔會被拒絕，也代表螢幕上的新值還沒被採用。先按 SVOFF。"
        : "伺服 OFF —— 可以按「套用」讓新值生效，或按「存入驅動器」存檔。";
    };
    /*  AI(W906-1203POT-1) 20260915: the overtravel ALLOCATION digits.
        Pn50A n.X□□□ allocates P-OT, Pn50B n.□□□X allocates N-OT. Manual
        s5.10.2: the allocation value 8 means "signal is always inactive", i.e.
        the drive stops listening; 0..7 name an input terminal (SI0..SI7 and the
        always-active forms), so anything that is not 8 means the drive IS
        listening and WILL refuse that direction regardless of the card.
        ⚠ Shows the digit AND the whole register: an operator about to change
        Pn50A needs to see the three digits the write must preserve. */
    /*  ⚠ TWO closures, not one taking the field name: cellOf() calls a
        formatter with the VALUE ONLY, so a shared function could not tell P-OT
        from N-OT -- and they are different DIGITS of different registers
        (Pn50A's top, Pn50B's bottom). One function reading the wrong digit
        would print a confident allocation for the wrong direction. */
    case "potA":  return v => fmtOtDigit(v, 12);   // Pn50A n.X□□□
    case "potB":  return v => fmtOtDigit(v,  0);   // Pn50B n.□□□X
    /*  AI(W906-1203KEEP-1) 20260916: the axis state in the rail's subtitle.
        Same fallback the interpolated string had -- "state ---" rather than an
        empty gap, because a rail entry with no state reads as one that failed
        to render rather than one whose state has not arrived. */
    case "railState":
      return v => (typeof v === "string" && v.length) ? v : "state ---";
    /*  AI(W906-1203LOGIC-1) 20260917: HLMT+ / HLMT- Logic. Names from
        AdvMotDrv.h:650-651 (HLMT_ACT_LOW = 0, HLMT_ACT_HIGH = 1), spelled the
        way the vendor's example spells them so the two can be compared without
        a translation step. ⚠ Never 開/關 -- this says which LEVEL means
        pressed, not whether the limit is armed. */
    case "hlmtLogic":
      return v => {
        if (typeof v !== "number") return String(v);
        const n = Math.round(v);
        if (n === 0) return "0　ACT_LOW（低準位＝被壓住）";
        if (n === 1) return "1　ACT_HIGH（高準位＝被壓住）";
        return `${n}　⚠ 不是 0/1，SDK 只定義 HLMT_ACT_LOW 與 HLMT_ACT_HIGH`;
      };
    case "rotDir":  return v => {
      if (typeof v !== "number") return String(v);
      const raw = Math.round(v);
      const d = raw & 0xF;
      const hex = "n." + [(raw >> 12) & 0xF, (raw >> 8) & 0xF, (raw >> 4) & 0xF, d]
                          .map(x => x.toString(16).toUpperCase()).join("");
      const name = { 0: "CCW 為正向", 1: "CW 為正向（反轉模式）" }[d];
      return `${d}  ${name || "保留值"}　${hex}`;
    };
    case "encUse":  return v => {
      if (typeof v !== "number") return String(v);
      const raw = Math.round(v);
      const d = (raw >> 8) & 0xF;
      const hex = "n." + [(raw >> 12) & 0xF, d, (raw >> 4) & 0xF, raw & 0xF]
                          .map(x => x.toString(16).toUpperCase()).join("");
      const name = { 0: "依編碼器規格使用", 1: "當成增量式編碼器使用",
                     2: "當成單圈絕對式使用" }[d];
      return `${d}  ${name || "保留值"}　${hex}`;
    };
    default:        return undefined;
  }
}

/** Mark a node as carrying `tag`, so refresh() can update it without a rebuild.
 *  `fname` must be a name fmtByName() knows, or omitted for the raw value. */
function bindCell(node, tag, fname, baseCls) {
  node.dataset.k = tag;
  if (fname) node.dataset.f = fname;
  //AI(W906-1203CTL-45) 20260912: presentation classes that must SURVIVE a
  //  refresh. Without this refresh() replaces the whole className with the
  //  value class and the node quietly loses its styling the first time a
  //  value arrives -- see the note in refresh().
  if (baseCls) node.dataset.basecls = baseCls;
  return node;
}

/** A cell built from a tag AND bound for in-place refresh. */
function tdOf(tags, tag, fname) {
  const c = cellOf(tags, tag, fname ? fmtByName(fname) : undefined);
  return bindCell(el("td", c.cls, c.text), tag, fname);
}

/** Update every bound node under `root` from `tags`. Creates nothing.
 *  Returns how many nodes were touched, which is what the probe asserts on --
 *  a refresh that silently matched nothing would look exactly like a page
 *  whose values never change, which is the bug this exists to prevent. */
export function refresh(root, tags) {
  let n = 0;

  /*  AI(W906-1203ALM-12) 20260914: tick the offline countdown in place.
      It lives here rather than in the shape rebuild because a number that
      changes every second must not rebuild the report every second -- that
      would destroy a half-typed distance box once per tick, on the pane where
      the operator is most likely to be typing one. */
  const cds = root.querySelectorAll ? root.querySelectorAll("[data-retryat]") : [];
  for (const cd of cds) {
    const left = Math.ceil((Number(cd.dataset.retryat) - Date.now()) / 1000);
    cd.textContent = (left > 0) ? `${left} 秒後自動重試` : "正在重試…";
  }

  /*  AI(W906-1203GEAR-1) 20260915: VALUES BUILT FROM TWO TAGS, walked
      separately from [data-k].
      The electronic gear RATIO is the number an operator reasons about, and
      neither half means anything alone -- but a formatter receives one value,
      so a "gearRatio" formatter could not have reached the denominator.
      ⚠ IT DOES NOT BORROW data-k, and that is the fix for a real defect: when
      this cell carried data-k="...gear.posNum" there were TWO nodes answering
      that selector, the composed one came first in the DOM, and anything
      reaching for the posNum ROW got this SPAN -- whose parent holds no input.
      A probe driving the 分子 set button threw and sent nothing while the 分母
      button beside it worked.
      ⚠ Both halves must be present. A numerator over a missing denominator, or
      a division by a 0 that only means "not read yet", would put an invented
      ratio on a screen whose whole purpose is to report what the drive holds. */
  const pairs = root.querySelectorAll ? root.querySelectorAll("[data-num][data-den]") : [];
  for (const node of pairs) {
    const num = tags.get(node.dataset.num);
    const den = tags.get(node.dataset.den);
    const both = (typeof num === "number" && typeof den === "number");
    /*  AI(W906-1203WHY-1) 20260915: data-f selects WHICH two-tag rendering.
        denRange answers "what may I type in the other box", which is the
        question the per-field "1..1073741824" hint fails to answer -- the pair
        is what refuses, not the field. */
    if (node.dataset.f === "denRange") {
      if (!both || num <= 0) {
        node.textContent = "---";
      } else {
        //  0.001 <= num/den <= 64000  =>  num/64000 <= den <= num/0.001
        const lo = Math.ceil(num / 64000);
        const hi = Math.min(1073741824, Math.floor(num / 0.001));
        node.textContent = `${lo} ～ ${hi}` +
          (both ? `　（目前 ${Math.round(den)}，比值 ${(num / den).toFixed(4)}）` : "");
      }
    } else if (both && den !== 0) {
      const r = num / den;
      node.textContent = `${Math.round(num)} / ${Math.round(den)} = ` +
                         (Number.isInteger(r) ? String(r) : r.toFixed(6));
    } else {
      node.textContent = "---";
    }
    node.className = "val";
    n++;
  }

  /*  AI(W906-1203WHY-1) 20260915: the last command's outcome, rendered beside
      the buttons instead of only on the diagnostics view. Three tags, so it
      gets its own pass like the ratio pair does.
      ⚠ SILENT WHEN THERE IS NOTHING TO SAY. A row that always shows something
      becomes background; this one is empty until a command has been answered,
      and loud only when one was REFUSED. */
  const crs = root.querySelectorAll ? root.querySelectorAll("[data-cmdresult]") : [];
  for (const node of crs) {
    const cmd = tags.get("pci1203.control.lastCmd");
    const ok  = tags.get("pci1203.control.lastOk");
    const why = tags.get("pci1203.control.lastWhy");
    const id  = tags.get("pci1203.control.lastId");
    if (typeof cmd !== "string" || !cmd.length) {
      node.textContent = "";
      node.className = "cmdresult";
      n++;
      continue;
    }
    const idTxt = (typeof id === "number") ? `  #${id}` : "";
    if (ok === false) {
      node.textContent = `⚠ 上一個指令被拒絕${idTxt}：${cmd}\n${why || "(沒有給原因)"}`;
      node.className = "cmdresult cmdbad";
    } else {
      const ret = tags.get("pci1203.control.lastRet");
      const rt  = tags.get("pci1203.control.lastRetText");
      const bad = (typeof ret === "number" && ret !== 0);
      node.textContent = bad
        ? `⚠ 上一個指令送出了但驅動器回錯誤${idTxt}：${cmd}\n0x${(ret >>> 0).toString(16).toUpperCase().padStart(8, "0")}  ${rt || ""}`
        : `上一個指令${idTxt}：${cmd}  —  已接受`;
      node.className = "cmdresult " + (bad ? "cmdbad" : "cmdok");
    }
    n++;
  }

  /*  AI(W906-1203WHY-2) 20260915: the "why will this axis not move" summary.
      Joins facts that were already published in four separate rows. It is a
      LIST, not a single verdict, because on the axis that prompted it THREE
      things were true at once (servo off, LMT+ pressed, drive still listening
      for P-OT) and naming only the first would have sent the operator to fix
      one of three. */
  const wss = root.querySelectorAll ? root.querySelectorAll("[data-whystuck]") : [];
  for (const node of wss) {
    const a     = node.dataset.whystuck;
    const state = tags.get(`pci1203.ax${a}.stateText`);
    const svOn  = tags.get(`pci1203.ax${a}.svOn`);
    const alm   = tags.get(`pci1203.ax${a}.alm`);
    const lp    = tags.get(`pci1203.ax${a}.limitP`);
    const ln    = tags.get(`pci1203.ax${a}.limitN`);
    const emg   = tags.get(`pci1203.ax${a}.emg`);
    const pn50A = tags.get(`pci1203.ax${a}.enc.pn50A`);
    const pn50B = tags.get(`pci1203.ax${a}.enc.pn50B`);
    const potOn = (typeof pn50A === "number") ? (((Math.round(pn50A) >> 12) & 0xF) !== 8) : null;
    const notOn = (typeof pn50B === "number") ? ((Math.round(pn50B) & 0xF) !== 8) : null;

    const why = [];
    if (state === "ERROR_STOP") why.push("軸在 ERROR_STOP —— 所有運動指令都會被拒絕，要先 reset error");
    if (alm === true)  why.push("驅動器有警報（ALM）");
    if (emg === true)  why.push("EMG 緊急停止觸發中");
    if (svOn === false) why.push("伺服沒有激磁（SVON = 0）—— 命令送得出去，但馬達不會跟著走");
    if (lp === true) {
      why.push("正極限 LMT+ 觸發中" +
        (potOn === true  ? "，而且驅動器的 P-OT 還是啟用的（Pn50A 最高位不是 8）—— 就算卡片的極限關掉，驅動器還是會擋住正方向"
       : potOn === false ? "，但驅動器的 P-OT 已停用，所以擋你的是卡片那一層"
                         : ""));
    }
    if (ln === true) {
      why.push("負極限 LMT− 觸發中" +
        (notOn === true  ? "，而且驅動器的 N-OT 還是啟用的（Pn50B 最低位不是 8）"
       : notOn === false ? "，但驅動器的 N-OT 已停用" : ""));
    }

    if (why.length === 0) {
      node.textContent = "";
      node.className = "cmdresult";
    } else {
      node.textContent = "⚠ 這一軸現在動不了的原因：\n· " + why.join("\n· ") +
        "\n（判斷方法：按下去之後看「命令」有沒有變。命令有變、回饋沒變 = 卡片送出去了，是驅動器或伺服的問題。）";
      node.className = "cmdresult cmdbad";
    }
    n++;
  }

  const nodes = root.querySelectorAll ? root.querySelectorAll("[data-k]") : [];
  for (const node of nodes) {
    const tag = node.dataset.k;
    const c = cellOf(tags, tag, fmtByName(node.dataset.f));

    /*  AI(W906-1203GEAR-1) 20260915: A VALUE BUILT FROM TWO TAGS.
        The electronic gear RATIO is the number an operator reasons about, and
        neither half means anything alone -- but a formatter receives one value,
        so a `gearRatio` formatter could not have reached the denominator. It
        gets its own branch instead of a special formatter, because the honest
        shape of this cell is "two tags in, one string out".
        ⚠ Both halves must be present. Showing a numerator over a missing
        denominator, or dividing by a 0 that only means "not read yet", would
        put an invented ratio on a screen whose whole purpose is to say what the
        drive currently holds. */
    /* ⚠ LAMPS ARE TESTED BEFORE BUTTONS, because both carry data-bit and only
       the lamp carries data-lampgood. Testing data-bit first would have styled
       every lamp as a button. */
    if (node.dataset.lampgood !== undefined) {
      const st = lampState(tags, tag, node.dataset.lampgood, node.dataset.bit);
      node.textContent = st.text;
      /*  AI(W906-1203CTL-45) 20260912: data-lampbase, because this line used to
          hard-code "lamp " and the enlarged I/O panel needs a different base
          class. Without it the big lamps would render correctly at first paint
          and silently shrink to table-lamp styling on the next refresh -- a
          layout bug that only appears once data starts arriving, which is the
          worst moment to notice one. */
      node.className = (node.dataset.lampbase || "lamp") + " " + st.cls;
      n++;
      continue;
    }
    if (node.dataset.bit !== undefined) {
      /* A DO bit button: its FACE is the current read-back and the value it
         sends is the OPPOSITE. Both have to move together or the button starts
         writing the value already there -- a control that appears to do
         nothing, which is the hardest symptom to diagnose. */
      const b = Number(node.dataset.bit);
      const raw = tags.get(tag);
      if (typeof raw === "number") {
        const on = ((raw >> b) & 1) === 1;
        node.textContent = on ? "1" : "0";
        node.dataset.value = on ? "0" : "1";
        node.className = "cmd " + (on ? "lampon" : "lampoff");
        node.title = `DO channel ${Number(node.dataset.chan)} — click to turn ` +
                     (on ? "OFF" : "ON");
      }
      n++;
      continue;
    }
    /*  AI(W906-1203CTL-35) 20260911: a value that is itself the health signal.
        data-statebad names the string that means trouble, so the node recolours
        as the value changes instead of keeping the colour it was painted with.
        Placed AFTER the lamp and bit branches for the same reason those are
        ordered: each branch is recognised by an attribute only it carries. */
    /*  AI(W906-1203ALM-8) 20260914: a BOOLEAN that is itself the health signal.
        data-statebad below compares the RAW value against a string, so it
        cannot colour `pci1203.control.lastOk`, which is a boolean. Without
        this branch a refusal renders in the same neutral colour as a success
        -- and "the command was refused, here is why" looked identical to
        "nothing happened", which is the confusion that made a whole panel of
        working buttons read as dead.
        Recognised by an attribute only it carries, like every branch here. */
    if (node.dataset.boolbad !== undefined) {
      const raw = tags.get(tag);
      const known = (typeof raw === "boolean");
      node.textContent = !known ? "" : (raw ? "已接受" : "被拒絕");
      node.className = !known ? "null" : (raw ? "ok" : "bad");
      n++;
      continue;
    }
    if (node.dataset.statebad !== undefined) {
      const raw = tags.get(tag);
      const known = (typeof raw === "string" && raw.length > 0);
      node.textContent = known ? "  " + raw : "";
      node.className = !known ? "null"
                     : (raw === node.dataset.statebad ? "bad" : "ok");
      n++;
      continue;
    }
    node.textContent = c.text;
    /*  AI(W906-1203CTL-45) 20260912: data-basecls, the plain-node twin of
        data-lampbase. This line replaced the WHOLE className with the value
        class, so any node carrying presentation classes as well -- the enlarged
        position readout, the speed read-backs -- rendered correctly at first
        paint and lost its styling on the first refresh. Measured: 21 big lamps
        found, 0 big numbers, because .bignumval had already been overwritten by
        the time the check ran. A layout bug that only appears once data starts
        arriving is the hardest kind to catch by looking. */
    node.className = node.dataset.keepcls
      ? node.className
      : (node.dataset.basecls ? node.dataset.basecls + " " + c.cls : c.cls);
    n++;
  }
  return n;
}

export const fmtPos  = v => (typeof v === "number" ? v.toFixed(3) : String(v));
export const fmtHex2 = v => (typeof v === "number"
  ? "0x" + (v & 0xff).toString(16).padStart(2, "0").toUpperCase() + `  (${v})`
  : String(v));
export const fmtHex8 = v => (typeof v === "number"
  ? "0x" + (v >>> 0).toString(16).padStart(8, "0").toUpperCase()
  : String(v));
export const fmtBool = v => (v === true ? "yes" : v === false ? "no" : String(v));
/*  AI(W906-1203CTL-44) 20260911: PAR_AxJerk renders as the PROFILE IT SELECTS,
    with the raw value kept beside it. Showing a bare "0" next to two buttons
    labelled 梯形 and S形 leaves the reader to guess which one 0 means, and the
    vendor example is the only place that says (PTP/Form1.cs:451-458). */
export function jerkTypeText(v) {
  if (v === 0) return "0 = 梯形 T";
  if (v === 1) return "1 = S形 S";
  if (typeof v === "number") return String(v) + " = ?";
  return "—";
}

/*  A STATION NUMBER IS NOT A BYTE. fmtHex2 masks with 0xff because it formats
    DI/DO port bytes; a station alias is a U16 and masking it produces a hex
    that CONTRADICTS the decimal beside it -- ?station=9999 rendered as
    "0x0F (9999)", found 20260910 by rendering the page in a real browser after
    the probe had passed. Two numbers that disagree is the exact failure this
    page exists to prevent, so station numbers get their own formatter with no
    mask and a width that grows with the value. */
/*  AI(W906-1203POT-1) 20260915: one overtravel allocation digit.
    Manual s5.10.2: the allocation value 8 means "signal is always inactive" --
    the drive stops listening to that overtravel input. 0..7 name an input
    terminal, so ANYTHING that is not 8 means the drive IS listening and will
    refuse that direction no matter what the card's limit is set to.
    ⚠ Prints the digit AND the whole register, because an operator about to
    change Pn50A has to see the three digits a write must preserve. */
function fmtOtDigit(v, shift) {
  if (typeof v !== "number") return String(v);
  const raw = Math.round(v);
  const hex = "n." + [(raw >> 12) & 0xF, (raw >> 8) & 0xF, (raw >> 4) & 0xF, raw & 0xF]
                      .map(x => x.toString(16).toUpperCase()).join("");
  const d = (raw >> shift) & 0xF;
  if (d === 8) return `${d}  停用（驅動器不理這個極限）　${hex}`;
  return `${d}  啟用中 — 接在輸入 SI${d}　${hex}　⚠ 卡片關掉也擋不住它`;
}

export const fmtStation = v => (typeof v === "number"
  ? "0x" + (v >>> 0).toString(16).toUpperCase().padStart(2, "0") + `  (${v})`
  : String(v));

function el(tag, cls, text) {
  const n = document.createElement(tag);
  if (cls) n.className = cls;
  if (text !== undefined && text !== null) n.textContent = text;
  return n;
}
function td(c) { return el("td", c.cls, c.text); }

/* ---------------------------------------------------------------------------
   AI(W906-1203CTL-9) 20260911: the control primitives.

   ⚠ EVERY ONE OF THESE IS INERT UNTIL js/pci1203.js BINDS IT. They are markup
   carrying a command description, nothing more -- which is exactly what lets
   the probe read a button and check the command it claims to send without any
   transport existing.
   --------------------------------------------------------------------------- */

/** A button that sends {cmd, tag, value}.
 *  `from` names a data-role input whose value is read AT CLICK TIME instead;
 *  a control may carry `value` or `from`, never both -- a button with a
 *  constant AND a source would leave which one wins to the reader. */
function cmdButton(label, cmd, tag, opts) {
  const o = opts || {};
  const b = el("button", "cmd" + (o.cls ? " " + o.cls : ""), label);
  b.type = "button";
  b.dataset.cmd = cmd;
  if (tag) b.dataset.tag = tag;
  if (o.from !== undefined)  b.dataset.from  = o.from;
  else if (o.value !== undefined) b.dataset.value = String(o.value);
  /*  AI(W906-1203ALM-10) 20260914: read `from` and send its NEGATIVE. Lets one
      distance box feed a left and a right arrow, the way the Utility's single
      距離 field feeds both of its arrows. */
  if (o.negate) b.dataset.negate = "1";
  if (o.title) b.title = o.title;
  /*  AI(W906-1203ALM-5) 20260912: an irreversible command may carry a confirm
      step. `confirm` is what the dialog shows; `station` is the number the
      operator must TYPE to proceed, and it is the same number that travels on
      the wire as "confirm=<n>" for the C++ side to check against the axis.
      ⚠ Typing beats clicking here on purpose. A second OK button is muscle
      memory after the third time; typing the station is the operator saying
      WHICH drive they mean, and the C++ side refuses a mismatch rather than
      picking one. */
  if (o.confirm !== undefined) b.dataset.confirm = String(o.confirm);
  if (o.station !== undefined) b.dataset.station = String(o.station);
  return b;
}

/** A number box a button reads from. Not bound to anything; its only job is to
 *  hold a value until a sibling button is pressed. */
function numBox(role, initial, width) {
  const i = el("input", "numbox");
  i.type = "number";
  i.dataset.role = role;
  i.value = String(initial);
  if (width) i.style.width = width;
  return i;
}

/*  AI(W906-1203LIM-2) 20260915: the HOME_MODE table, transcribed from the
    vendor's own enum -- AdvMotDrv.h:2394-2409, `typedef enum { MODE1_ABS = 0,
    ... MODE16_LMT_SEARCH_REFIND_NEG_REF = 15, CIA402_MODE1 = 101, ... }`.
    ⚠ The numbers are NOT the mode names: MODE2 is 1, MODE8 is 7, MODE13 is 12.
    Typing "13" meaning MODE13 selects MODE14_ABS_SEARCH_REFIND_REF, which
    hunts an ORG signal that a limit-switch axis does not have -- and that is
    the entire reason this stopped being a number box.

/*  AI(W906-1203TRIM-1) 20260917: ⚠ kHomeModes (the 25-entry mode table) AND
    selectBox() WERE HERE AND ARE DELETED -- roughly 60 lines, both unreferenced
    once the mode picker was removed. User: "介面可以精簡就精簡".

    They are removed rather than left unused because an unused table that still
    LOOKS authoritative is the more expensive kind of dead code: it listed the
    sixteen "typical" card modes with the vendor's own recommendations
    ("★建議 12 MODE13_LMT_SEARCH_REFIND"), and every one of those sixteen is a
    mode these axes answer 0x8000510F to. Twice today a note somewhere else on
    the page was found still quoting it at the operator. Leaving the source of
    those quotes in place invites the third one.

    ⓘ Recoverable in full from git (the commit that removes this) and from
    scratchpad/genhomemodes.ps1, which generated the descriptions from
    Advantech's FrmHomeMode.resx in the first place. A machine whose axes DO
    accept the card's own home modes regenerates both in one run.
    ⓘ selectBox() had no other caller -- checked, not assumed. */

/** One clickable output bit. Renders the CURRENT read-back state as its face
 *  and sends the OPPOSITE -- a toggle, like the vendor's EthcatDO example
 *  (Unit1.cpp:359-365 flips m_bitData[m_DO] then writes it).
 *
 *  ⚠ WHEN THE BYTE IS NOT READABLE THE BIT IS NOT CLICKABLE, and that is not
 *  caution for its own sake: a toggle whose current state is unknown cannot
 *  know what to send. Sending "1" blindly would turn "nobody read this output"
 *  into "energise this coil". */
function doBitControl(tags, port, bit) {
  const tag = `pci1203.do${port}`;
  const raw = tags.has(tag) ? tags.get(tag) : undefined;
  if (typeof raw !== "number") {
    const c = cellOf(tags, tag);
    return el("td", c.cls, c.text === "n/a" ? "n/a" : "---");
  }
  /*  AI(W906-1203ALM-14) 20260914: ⚠ A SERVOPACK'S DO BYTES ARE NOT COILS.
      User: "output 你有確定有送出訊號? 我已經有點了 但是實體IO沒觸發".

      MEASURED 20260914: of the 536 clickable output bits this page offered,
      only 48 belonged to a real output module. The rest were SERVOPACK bytes:
          卡片 0x01 (1)  SERVOPACK  64 clickable
          卡片 0x03 (3)  SERVOPACK  64 clickable
          卡片 0x0E (14) SERVOPACK  32 clickable
          卡片 0xA0(160) VC4-ODM1   24 clickable   <- the only real outputs
      A drive's DO bytes are its RxPDO command image -- the words the master
      sends it every cycle. Writing a bit there is not wired to anything, the
      call still returns SUCCESS, and the next cyclic frame overwrites it. So
      the button reported success and no coil moved, which is precisely the
      report. Acm_DaqDoSetBit was never the problem: a standalone probe wrote
      530 of 536 channels successfully at the same moment.

      These bits are now rendered READ-ONLY on a SERVOPACK. Not disabled
      buttons -- plain values, because a disabled button still says "this is
      the thing that would do it" and there is no coil behind it to do. */
  /*  AI(W906-1203RAIL-1) 20260915: ⚠ THE FOURTH PLACE THE NAME GATE LIVED, and
      the one with teeth: this is what stops a drive's RxPDO command bytes being
      offered as clickable coils. It tested /SERVOPACK/i, so a SW3D-680's output
      PDO was still rendered as buttons -- the exact false affordance
      AI(W906-1203ALM-14) was written to remove, surviving on five modules
      because they are not Yaskawas. 1000h says drive; drive means read-only. */
  //AI(W906-1203RING-1) 20260922: ask about THIS BYTE'S ring, not just its
  //  station number. See the banner on stationIsDrive -- without the ring, a
  //  real output card sharing a number with a drive had every one of its coils
  //  rendered as a read-only lamp.
  const stn = tags.get(`${tag}.station`);
  const rng = tags.get(`${tag}.ring`);
  if (typeof stn === "number" && stationIsDrive(tags, stn, rng)) {
    const on = ((raw >> bit) & 1) === 1;
    const td = el("td", "lampcell");
    const v = el("span", "lamp " + (on ? "lampon" : "lampoff"), on ? "1" : "0");
    v.dataset.k = tag;
    v.dataset.bit = String(bit);
    v.dataset.lampgood = "";           // neither state is an alarm
    v.dataset.lampbase = "lamp";
    v.title = "驅動器 PDO 命令字，不是實體線圈 — 唯讀";
    td.appendChild(v);
    return td;
  }
  const on = ((raw >> bit) & 1) === 1;
  const cell = el("td", "lampcell");
  const b = cmdButton(on ? "1" : "0",
                      "pci1203.do.setBit",
                      `pci1203.do${port}.bit${bit}`,
                      { value: on ? 0 : 1,
                        cls: on ? "lampon" : "lampoff",
                        title: `DO channel ${port * 8 + bit} — click to turn ` +
                               (on ? "OFF" : "ON") });
  /*  AI(W906-1203CTL-27) 20260911: bound for in-place refresh. data-k is the
      BYTE tag it reads (not the bit tag it sends to -- those are different
      strings and conflating them is how the face and the payload drift apart). */
  b.dataset.k    = tag;
  b.dataset.bit  = String(bit);
  b.dataset.chan = String(port * 8 + bit);
  cell.appendChild(b);
  return cell;
}

/* ---------------------------------------------------------------------------
   THE BANNER -- the page's whole reason for existing.

   Answers "why does this screen look empty" in ONE sentence, chosen from the
   five causes in the header block. The ORDER OF THE TESTS IS THE ORDER OF THE
   CAUSES, and that is load-bearing: each later test is only meaningful once
   the earlier ones pass. Testing "is it empty" before "is it linked" would
   report an empty fieldbus on a binary that has no SDK in it.
   --------------------------------------------------------------------------- */
export function diagnose(tags, linkState) {
  const has = t => tags.has(t);
  const val = t => tags.get(t);
  const state = (linkState && linkState.state) || "unknown";

  if (!has("pci1203.linked")) {
    return {
      level: "bad",
      head: "no pci1203 tags on this feed",
      body: state === "online"
        ? "The feed is connected but carries no pci1203.* tags at all. Either the " +
          "publisher is an older build, or this page is on the MOCK transport. " +
          "Open it with ?src=ws."
        : "Not connected to a publisher yet (" + state + "). This page needs " +
          "?src=ws and a running wb_gateway.",
    };
  }
  if (val("pci1203.linked") !== true) {
    return {
      level: "bad",
      head: "publisher built WITHOUT the Advantech SDK",
      body: "pci1203.linked is false, so HAVE_PCI1203 was not armed for that binary. " +
            "No card can be read no matter what else is configured. This is a question " +
            "about the BUILD, not about the hardware.",
    };
  }
  if (val("pci1203.enabled") !== true) {
    return {
      level: "warn",
      head: "monitor not enabled",
      body: "The SDK is linked but the monitor was never started. Define " +
            "INSTALL_1203_MONITOR at the end of MachineType.h and rebuild." +
            (val("pci1203.lastErrorText")
              ? "  Publisher says: " + val("pci1203.lastErrorText") : ""),
    };
  }
  if (val("pci1203.open") !== true) {
    return {
      level: "bad",
      head: "the card did NOT open",
      body: "Reason from the publisher: " +
            (val("pci1203.lastErrorText") || "(none reported)") +
            "  --  lastError " + cellOf(tags, "pci1203.lastError", fmtHex8).text,
    };
  }

  /* AI(W906-MW2b) 20260910: OPEN, BUT WITH CONFLICTING SubDevice IDs.
     Tested here -- after "did it open" and BEFORE "is it empty" -- because it
     is the one state in which the page can be FULL OF PLAUSIBLE NUMBERS THAT
     BELONG TO THE WRONG STATION. An empty ring is honest; a conflicted ring
     is not, and ranking it below "empty" would let a populated-looking screen
     pass without the warning. */
  if (val("pci1203.idConflict") === true) {
    return {
      level: "bad",
      head: "card is open, but the SubDevice IDs CONFLICT — readings may belong to the wrong station",
      body:
        "Acm_DevOpen returned 0x8300002B EC_SubDeviceIDConflicted and the monitor carried " +
        "on anyway, which is what Advantech's own wrapper does. The numbers below are real " +
        "reads, but while station numbers clash a byte can be attributed to the wrong " +
        "module — which is worse than no data, because it looks like data. Fix: reassign " +
        "the SubDevice IDs, then POWER-CYCLE the EtherCAT stations. A PC reboot does not " +
        "do it: the stations are separately powered and keep their old IDs across one.",
    };
  }

  /* Open. Now the interesting case: open and empty. */
  const empty = val("pci1203.subDevices") === 0 &&
                val("pci1203.scan.found") === 0 &&
                val("pci1203.axesOpened") === 0;
  if (empty) {
    return {
      level: "info",
      head: "card is OPEN and the fieldbus behind it is EMPTY — this is not a software fault",
      body:
        "The PCIE-1203 is an EtherCAT MASTER: it is the fieldbus interface, and axes " +
        "and IO live in SLAVE devices cabled out of its EtherCAT port, not on the card " +
        "itself. The card reports 0 sub-devices, a " +
        cellOf(tags, "pci1203.scan.rings").text + "-ring x " +
        cellOf(tags, "pci1203.scan.slots").text + "-slot sweep found 0 slaves, and 0 " +
        "axes could be opened by EITHER addressing scheme. That is exactly what an " +
        "unwired, unpowered, or not-yet-existing ring looks like. To tell hardware from " +
        "software, scan the bus with Advantech's own Common Motion Utility: if IT sees " +
        "no slaves either, the gap is cabling or hardware, not this software.",
    };
  }

  const mode = val("pci1203.mode");
  return {
    level: mode === "attached" ? "ok" : "warn",
    head: mode === "attached"
      ? "attached to production's card handle — read-only passenger"
      : "monitor OWNS the card (production has not opened it)",
    body: mode === "attached"
      ? "Production opened the card and this monitor reads through its handle. It never " +
        "closes a handle it did not open."
      : "mode=owned means INSTALL_ETHETCAT() is false, so the machine's own 1203 path " +
        "never opened the card. Nothing shown here reflects production's view of it. " +
        "Expected while SHUTTLE_SENSOR_TYPE and VacuUnitType are 0.",
  };
}

/* ---------------------------------------------------------------------------
   Sections
   --------------------------------------------------------------------------- */

function banner(level, head, body) {
  const b = el("div", "banner " + level);
  b.appendChild(el("div", "bhead", head));
  b.appendChild(el("div", "bbody", body));
  return b;
}

function kvTable(tags, rows) {
  const t = el("table", "kv");
  for (const [label, c, note] of rows) {
    const tr = el("tr");
    tr.appendChild(el("th", null, label));
    tr.appendChild(td(c));
    tr.appendChild(el("td", "note", note || ""));
    t.appendChild(tr);
  }
  return t;
}

/** Wide tables scroll inside their own box; the page body must never scroll
 *  sideways. */
function wrapScroll(node) {
  const d = el("div", "scroll");
  d.appendChild(node);
  return d;
}

export function sectionCard(tags) {
  const c = (t, f) => cellOf(tags, t, f);
  const s = el("section");
  s.appendChild(el("h2", null, "Card"));

  /* ⚠ `disabled` is shown FIRST and loudly. It is the difference between "the
     card is empty" and "the monitor gave up reading it", which render
     identically as rows of "---" everywhere else on this page. On this machine
     it is currently TRUE, by design: the observer self-disables after 10
     consecutive polls in which every read failed, so a card that has gone away
     cannot block a machine tick forever (Pci1203Monitor.h,
     kMaxConsecutiveFailures). Measured 20260908 -- on a card with nothing on
     its ring, every read failing is the EXPECTED outcome, so this banner is the
     steady state here rather than an incident. */
  if (tags.get("pci1203.disabled") === true) {
    s.appendChild(banner("warn",
      "monitor has SELF-DISABLED — polling has stopped",
      (tags.get("pci1203.disabledReason") || "(no reason published)") +
      "  ·  This is the designed safety valve, not a crash: after " +
      "kMaxConsecutiveFailures all-failed polls the observer stops so it cannot block a " +
      "machine tick. The card-level facts below are the last good sample. On a card with " +
      "nothing on its ring, every read failing is the expected outcome."));
  }

  s.appendChild(kvTable(tags, [
    ["linked (HAVE_PCI1203)", c("pci1203.linked", fmtBool), "a fact about the BINARY, always published"],
    ["monitor enabled",       c("pci1203.enabled", fmtBool), "#define INSTALL_1203_MONITOR"],
    ["device open",           c("pci1203.open", fmtBool), ""],
    ["mode",                  c("pci1203.mode"), "attached = production owns the handle; owned = we do"],
    ["enumerated",            c("pci1203.enumerated", fmtBool), "Acm_GetAvailableDevs returned SUCCESS"],
    ["device name",           c("pci1203.devName"), ""],
    ["device number",         c("pci1203.devNum"), ""],
    ["device count",          c("pci1203.devCount"), ""],
    ["sub-devices",           c("pci1203.subDevices"), "DEVLIST.nNumOfSubdevices — 0 = nothing under the card"],
    /* AI(W906-MW2b) 20260910: the CARD's own answer for how wide its IO image
       is, and how many stations each ring reports. These are the numbers that
       say whether the fieldbus is populated at all -- the vendor's example
       prints "There is no Ethcat DI Slaves" when diMaxChan is 0.
         ⚠ 0 is a LEGITIMATE reading here (an empty ring reports it), which is
         why the publisher gates them on a separate validity flag and they can
         still arrive null. "0 channels" and "nobody asked" stay distinct. */
    ["DI channels (card)",    c("pci1203.diMaxChan"), "FT_DaqDiMaxChan — 0 means no EtherCAT DI slaves"],
    ["DO channels (card)",    c("pci1203.doMaxChan"), "FT_DaqDoMaxChan"],
    ["ring 0 slaves",         c("pci1203.ring0Slaves"), "FT_MasCyclicCnt_R0 — the Motion ring"],
    ["ring 1 slaves",         c("pci1203.ring1Slaves"), "FT_MasCyclicCnt_R1 — the Fast IO ring"],
    ["SubDevice ID conflict", c("pci1203.idConflict", fmtBool), "true = open succeeded but station numbers clash"],
    ["axes opened",           c("pci1203.axesOpened"), ""],
    ["re-attaches",           c("pci1203.reattaches"), "production re-opened the card underneath us"],
    ["detaches",              c("pci1203.detaches"), "production dropped to uiDevhand == 0"],
    ["poll count",            c("pci1203.pollCount"), ""],
    ["polls with errors",     c("pci1203.pollErrors"), ""],
    ["poll duration",         c("pci1203.pollMs", v => v + " ms"), ""],
    ["vendor calls / poll",   c("pci1203.poll.vendorCalls"), "read-only calls made in the LAST poll"],
    ["ring0/slave0 state",    c("pci1203.slaveState"), ""],
    ["last error",            c("pci1203.lastError", fmtHex8), ""],
    ["last error text",       c("pci1203.lastErrorText"), ""],
    ["self-disabled",         c("pci1203.disabled", fmtBool), ""],
    ["self-disable reason",   c("pci1203.disabledReason"), ""],
  ]));
  return s;
}

/** A bit lamp. Off / on / unknown are three visibly different things. */
/*  AI(W906-1203CTL-35) 20260911: ⚠ THIS LAMP WAS FROZEN AT FIRST PAINT.
    User: "你馬達讀回來的IO 又是一樣的狀況 我去遮實體 沒有閃滅".

    It built a span with a class and NO data-k, so refresh() -- which walks
    querySelectorAll("[data-k]") and touches nothing else -- could never see it.
    Every one of the axis MotionIO lamps showed whatever the card happened to
    report at the instant the pane was built and then never moved again.

    ⚠ WHY IT SURVIVED: a frozen lamp is indistinguishable from a signal that
    genuinely is not changing, and on this machine most of these bits genuinely
    do not change. It only becomes visible when someone covers a sensor and
    watches -- which is exactly how the user found it, twice now, in two
    different families. byteLamps() (the IO card bytes) was given this binding
    when the same bug was fixed there; this function was missed because the two
    render the same thing by different routes.

    The binding is byteLamps()'s: data-k names the tag, data-lampgood says how
    ON should be coloured. No data-bit -- these tags are booleans, not bytes,
    and lampState() reads the value directly when bit is absent.
    ⚠ data-lampgood must be the STRING "true"/"false": lampState compares
    against string literals because dataset values are always strings. */
function lamp(tags, tag, spec) {
  const good = (spec && spec.good === true)  ? "true"
             : (spec && spec.good === false) ? "false" : "";
  const n = el("td", "lampcell");
  const st = lampState(tags, tag, good, undefined);
  const sp = el("span", "lamp " + st.cls, st.text);
  sp.dataset.k = tag;
  sp.dataset.lampgood = good;
  n.appendChild(sp);
  return n;
}

/* ---------------------------------------------------------------------------
   AI(W906-1203CTL-9) 20260911: WHAT DOES A CLICK ON THIS PAGE DO?

   Rendered FIRST among the sections, above everything that offers a button,
   because every control below is meaningless without its answer. Four states,
   and they are four different facts:

     not linked   this binary has no Advantech SDK in it; nothing can command
     not armed    the SDK is in, WB_PUMP_1203_CONTROL is not -- no controls
     armed, dry   clicks are validated, formatted and LOGGED; nothing is issued
     armed, LIVE  clicks energise coils and move servos

   ⚠ THE DRY/LIVE DISTINCTION IS NOT A DEBUG DETAIL ON THIS SCREEN. It is the
   difference between a button that writes a line in a log and one that turns a
   motor. A page that offered operable-looking controls without saying which is
   in force would be lying about what a click does.
   --------------------------------------------------------------------------- */
export function sectionControl(tags, opts) {
  const c = (t, f) => cellOf(tags, t, f);
  const s = el("section");
  s.appendChild(el("h2", null, "Control — what a click on this page does"));

  const linked = tags.get("pci1203.control.linked");
  const armed  = controlArmed(tags);
  const dry    = controlDry(tags);
  const held   = !!(opts && opts.hasControl);

  /* ⚠ ABSENT AND FALSE ARE DIFFERENT FACTS HERE, and saying "this build has no
     SDK" for an absent tag would be an invented diagnosis: the tag is absent on
     any publisher older than 20260911, which says nothing about the binary. */
  if (!tags.has("pci1203.control.linked")) {
    s.appendChild(banner("info",
      "this feed does not report a control surface at all — no controls are shown",
      "The pci1203.control.* tags are absent from this wire, which means the publisher " +
      "predates them (they were added 20260911). That is NOT the same as a binary " +
      "without the vendor SDK, and this page will not guess between the two. Everything " +
      "below is an observation."));
  } else if (linked !== true) {
    s.appendChild(banner("info",
      "this build cannot command the card — no controls are shown",
      "pci1203.control.linked is false, so EtherCAT/Pci1203Control.cpp was compiled " +
      "without the Advantech SDK. Every section below is an observation. This is the " +
      "same question pci1203.linked answers for the observer, and it is answerable with " +
      "no card because it is a fact about the BINARY."));
  } else if (!armed) {
    s.appendChild(banner("info",
      "the write surface is NOT armed — no controls are shown",
      "The code is in this binary but WB_PUMP_1203_CONTROL is not defined, so " +
      "wb_publish never called Pci1203ControlEnable() and the tag feed discards inbound " +
      "bytes exactly as it has since 20260812. Buttons are ABSENT rather than disabled: " +
      "a greyed-out button still says 'this is the thing that would do it', and on a " +
      "machine screen 'cannot' and 'did not' must not look alike."));
  } else if (dry) {
    s.appendChild(banner("ok",
      "DRY RUN — every command is validated and logged, and NOTHING is issued",
      "This is the mode the 20260911 verification runs in. A click below is checked " +
      "against the same bounds a live command would be, the exact vendor call and its " +
      "arguments are recorded, and no vendor function is called. That is what lets this " +
      "page's flow be compared with Common Motion Utility's without a machine moving. " +
      "Read 'last call' below to see what WOULD have been sent."));
  } else {
    s.appendChild(banner("bad",
      "LIVE — a click below will energise an output or MOVE A SERVO",
      "WB_PUMP_1203_CONTROL_LIVE is defined. Motion is still refused while an axis reads " +
      "ERROR_STOP, and an error is never cleared as a side effect of a move — but " +
      "everything else on this page reaches the machine. Check the area is clear."));
  }

  /* ---------------------------------------------------------------------
     AI(W906-1203CTL-14) 20260911: THE SINGLE-OPERATOR TOKEN.

     ⚠ FOUND BY RUNNING IT, NOT BY READING IT. Every command this page sent
     over a real WebSocket came back ok:false "not-operator", because
     WebBridgeServer (FW-W3, 20260819) hands machine commands only to the ONE
     connection holding the control token. Without this row the page's buttons
     would all have failed silently in a browser while passing every headless
     test -- the probe drives view.js, which never sees an ack.

     ⚠ AND IT IS NOT ACQUIRED AUTOMATICALLY ON LOAD. The token is exclusive
     across browser tabs and operators; grabbing it because a page was opened
     would take the machine away from whoever is actually standing at it. It is
     a button, pressed on purpose, and released on purpose.
     --------------------------------------------------------------------- */
  if (armed) {
    const row = el("div", "panelrow");
    row.appendChild(el("span", "panellabel", "operator token"));
    row.appendChild(el("span", held ? "ok" : "bad",
      held ? "HELD by this page" : "NOT held — machine commands will be refused"));
    if (held) {
      row.appendChild(cmdButton("RELEASE CONTROL", "control.release", "",
        { cls: "warnbtn", title: "Hand the token back so another operator can take it" }));
    } else {
      row.appendChild(cmdButton("TAKE CONTROL", "control.acquire", "",
        { title: "WebBridgeServer grants machine commands to one connection at a time" }));
    }
    s.appendChild(row);
    if (!held) {
      s.appendChild(el("div", "emptynote",
        "Until this page holds the token the server answers every machine command with " +
        "“not-operator”. The buttons below are still shown, because they are " +
        "real and the refusal is explicit — pressing one says so rather than doing " +
        "nothing. If TAKE CONTROL is refused with “control-held”, another " +
        "connection has it."));
    }
  }

  s.appendChild(kvTable(tags, [
    ["write surface compiled in", c("pci1203.control.linked", fmtBool),
     "a fact about the BINARY — answerable with no card"],
    ["armed",                     c("pci1203.control.armed", fmtBool),
     "#define WB_PUMP_1203_CONTROL in MachineType.h"],
    ["dry run",                   c("pci1203.control.dryRun", fmtBool),
     "true = validated and logged, never issued"],
    ["commands accepted",         c("pci1203.control.accepted"), "passed validation"],
    ["commands refused",          c("pci1203.control.refused"),
     "rejected with a reason — never clamped into range"],
    ["commands issued",           c("pci1203.control.issued"),
     "actually reached the vendor API (always 0 in dry run)"],
  ]));

  /* ⚠ THE LAST-COMMAND BLOCK IS THE ONLY FEEDBACK AN OPERATOR GETS, so its
     three-state rendering matters more here than anywhere else on the page:
     all of these read "---" until something has been commanded, which is NOT
     the same as a command having failed. */
  s.appendChild(el("h3", null, "last command"));
  s.appendChild(kvTable(tags, [
    ["id",        c("pci1203.control.lastId"),
     "the requester's own number — match it against the button you pressed"],
    ["command",   c("pci1203.control.lastCmd"), ""],
    ["accepted",  c("pci1203.control.lastOk", fmtBool),
     "false = refused; the reason is below"],
    ["issued",    c("pci1203.control.lastIssued", fmtBool),
     "false in dry run even when accepted"],
    ["vendor return", c("pci1203.control.lastRet", fmtHex8),
     "0x00000000 = SUCCESS; null when nothing was issued, NEVER 0"],
    /*  AI(W906-1203CTL-36) 20260911: what that code MEANS, from
        Acm_GetErrorMessage -- the same call behind Common Motion Utility's
        錯誤資訊 field. A bare 0x83100000 beside a button that appeared to do
        nothing is not an explanation, and "the button did nothing" has cost
        this campaign several rounds already. */
    ["錯誤資訊", c("pci1203.control.lastRetText"),
     "the vendor's own sentence for the return code above — not written in this tree"],
    ["would call", c("pci1203.control.lastCall"),
     "the exact vendor call and arguments — compare this with Common Motion Utility"],
    ["why",       c("pci1203.control.lastWhy"), ""],
  ]));
  return s;
}

/*  AI(W906-1203CTL-30) 20260911: ONE AXIS AT A TIME, selected from the rail --
    the same shape the IO cards got, for the same reason and on the same
    request: "我現在馬達也不想擠在一起 / 要跟IO一樣 分卡片在左邊 / 點選左邊馬達
    卡片 可以單獨操作該馬達".

    ⚠ IT IS ALSO A SAFETY IMPROVEMENT, not only a layout one. Sixteen panels on
    one page put a JOG button for ax7 two centimetres from the JOG button for
    ax8, all of them identical, on a machine with nine servo drives fitted. One
    axis per pane means the axis an operator is about to move is the one whose
    name is at the top of the pane and highlighted in the rail.

    `only` is an axis index, or null for every opened axis. */
export function sectionAxes(tags, opts) {
  const only = (opts && typeof opts.axis === "number") ? opts.axis : null;
  const c = (t, f) => cellOf(tags, t, f);
  const s = el("section");
  /*  AI(W906-1203CTL-32) 20260911: the heading names the STATION when there is
      one, because that is the number on the drive in the cabinet -- "馬達 ax7"
      told an operator nothing they could match against hardware. `ax<n>` stays
      in the row label and the sub-line: it is this module's slot index and the
      key every tag is published under, so hiding it entirely would make the
      screen impossible to correlate with the wire. */
  let title = only === null ? "馬達 — 全部軸" : ("馬達 ax" + only);
  if (only !== null) {
    //AI(W906-1203CTL-39) 20260911: address first, dial second -- same ruling as
    //  the rail. Both stay, because they are both real and each is the one
    //  somebody will be holding when they read this heading.
    const addr = tags.get(`pci1203.ax${only}.station`);
    const dial = tags.get(`pci1203.ax${only}.stationAlias`);
    const sub  = tags.get(`pci1203.ax${only}.stationAxis`);
    const cnt  = tags.get(`pci1203.ax${only}.stationAxes`);
    if (typeof addr === "number" && addr >= 0 && typeof sub === "number" && sub >= 0) {
      //AI(W906-1203HEX-1) 20260915: hex + decimal, matching the IO rail and
      //  Common Motion Utility's tree. See the note on the rail label.
      title = (cnt === 1 ? `馬達 位址 ${fmtStation(addr)}`
                         : `馬達 位址 ${fmtStation(addr)} · 軸 ${sub}`) +
              ((typeof dial === "number" && dial >= 0) ? `  （站 ${fmtStation(dial)}）` : "") +
              `  (ax${only})`;
    }
  }
  s.appendChild(el("h2", null, title));

  /*  AI(W906-1203CTL-46) 20260912: THREE BANNERS REMOVED. User: "這圖片上的說明
      我不想顯示". They were an addressing guard, a long note about what the LMT
      lamps can and cannot see, and a note about the two station numberings.

      Each earned its place while the matching question was open, and each of
      those questions is now closed -- the rail labels by 定址位址 per the user's
      ruling, the lamps have their own panel, and the LMT finding is recorded at
      the Acm_AxGetMotionIO call site in Pci1203Monitor.cpp where the next person
      to doubt it will actually be reading. Prose that has outlived its question
      is just something between an operator and the numbers.

      ⚠ ONE OF THEM WAS ALSO LYING. "addressing: Acm_AxOpenbyID — production's
      own scheme" stayed true-by-default after CTL-43 replaced the ByID sweep
      with Acm_AxOpen over physical indices. Deleting the banner does not fix
      that; see the byId note in Pci1203Monitor.cpp and the gating change in
      WebBridgeTags.cpp, which were corrected in the same commit. */

  /*  AI(W906-1203CTL-32) 20260911: ⚠ THERE ARE TWO STATION NUMBERINGS ON THIS
      SCREEN AND THEY ARE NOT THE SAME LIST. Saying so is the whole point of
      this banner: the IO cards above are numbered from the DI/DO map
      (Acm_DevUpLoadMapInfo's SlaveID -- 0x01, 0x02, 0x03, 0x10, 0x11, 0x12,
      0x50, 0x51, 0xA0, 0xA1), while the axis IDs are whatever
      Acm_AxOpenbyID answers to. Measured 20260911 those are 0, 1, 3, 10, 14,
      30, 41, 124 and 153 -- nine of them, matching ring0=9 servo drives, so
      they do identify the nine drives. But station 2 IS a SGDXW on the IO map
      and REFUSES every axis open (0x8000004A InvalidSlaveIP), so the two lists
      are demonstrably not interchangeable, and the card is reporting a
      SubDevice ID conflict on top of that.
      The number below is therefore stated as what it provably is -- the ID the
      axis API answers to -- and not asserted to be the rotary dial, which is a
      thing only someone standing at the cabinet can confirm. */
  /*  AI(W906-1203CTL-34) 20260911: THIS BANNER USED TO ASK A QUESTION THAT IS
      NOW ANSWERED, so it states the answer instead. It previously warned that
      the station numbers shown (0, 1, 3, 10, 14, 30, 41, 124, 153) had not been
      checked against the cabinet. They have been, by comparing this page with
      Common Motion Utility's own topology tree, and they were the WRONG NUMBERS
      -- the Utility lists 0x001..0x009. Both numbers are real and this page now
      shows both, under names that say which is which. */
  /*  AI(W906-1203CTL-37) 20260911: ⚠ THESE LAMPS CANNOT SEE THE MACHINE'S
      SENSORS, AND SAYING SO IS THE WHOLE POINT OF THIS BANNER.

      User, for the third time and with justified irritation: "我去碰極限 訊號
      那邊一樣不會變阿". It was not the page. MEASURED at the tag layer with no
      browser involved, 90 s while the user worked the limit switch:
          DI card bytes   changed 643 times across 8 ports
          axis MotionIO   changed 0 times across all 15 axes
      and the ports that moved belong to stations 0xA0/0xA1 (ECAT-VC4-ODM1).

      So the machine's limit sensors are wired to the EtherCAT DI modules. The
      LMT+/LMT- bits in Acm_AxGetMotionIO come from the SERVOPACK's own P-OT/
      N-OT terminals over PDO -- a different, unconnected path. A lamp labelled
      LMT+ sitting beside a motor, which physically cannot reflect that motor's
      limit switch, is exactly the "correct-looking value under the wrong name"
      failure this codebase keeps warning about, and it cost the user three
      rounds of testing before anyone measured the right thing.

      ⚠ The constant LMT+=1 on most axes is consistent with those drive
      terminals being unwired (NC contacts read as overtravel), which is why the
      value looked plausible and never moved. */
  s.appendChild(kvTable(tags, [
    //AI(W906-1203CTL-46) 20260912: relabelled. The row used to read "by-ID
    //  mode" and report a flag that stayed true after CTL-43 stopped using
    //  Acm_AxOpenbyID -- an inherited name outliving the thing it named.
    ["sweep: 站號對應已知",     c("pci1203.axScan.byIdMode", fmtBool),
     "軸是否都對應到一台已掃描到的伺服站"],
    ["sweep: axes found",       c("pci1203.axScan.byIdFound"), ""],
    /*  AI(W906-1203CTL-32) 20260911: stations and pool size are published so the
        screen can be checked against itself. "axes found" is how many handles
        this module holds; "pool size" is how many motors the card actually has.
        They were 16 and 15 before this pass -- one motor shown twice -- and
        nothing on the old screen could have revealed that. */
    ["sweep: stations answering", c("pci1203.axScan.stations"),
     "drives that responded to Acm_AxOpenbyID"],
    ["sweep: distinct axes on card", c("pci1203.axScan.poolSize"),
     "⚠ if 'axes found' EXCEEDS this, a motor is being shown twice"],
    ["sweep: addresses probed", c("pci1203.axScan.tried"), ""],
    ["sweep: duration",         c("pci1203.axScan.ms", v => v + " ms"), ""],
    ["sweep: truncated",        c("pci1203.axScan.truncated", fmtBool),
     "TRUE means the time budget ran out — fewer axes found than exist"],
  ]));

  const t = el("table", "grid");
  const head = el("tr");
  /*  AI(W906-1203CTL-34) 20260911: 站號 is now the DIAL number and the address
      Acm_AxOpenbyID was called with gets its own column. Both matter and they
      are different numbers on this ring -- keeping only one of them is what let
      "站 30" stand for the drive labelled 7 for a whole afternoon. Column names
      follow Common Motion Utility's 命令 / 回饋 so the two screens line up. */
  for (const h of ["#", "opened", "by ID", "站號", "站內軸", "定址位址",
                   "state", "state text",
                   "命令 cmd", "回饋 act", "命令速度", "MotionIO",
                   //AI(W906-1203CTL-45) 20260912: lamp columns only on the
                   // all-axes view; the single-axis pane has its own panel.
                   ...(only === null ? IO_BITS.map(b => b.label) : [])]) {
    head.appendChild(el("th", null, h));
  }
  t.appendChild(head);

  for (let a = 0; a < AXIS_SLOTS; a++) {
    /*  On a single-axis pane the table is that axis's readout, not a directory
        of sixteen. Showing all of them would put the numbers an operator is
        about to act on in the middle of fifteen rows they are not. */
    if (only !== null && a !== only) continue;
    const p = `pci1203.ax${a}.`;
    const tr = el("tr");
    tr.appendChild(el("th", null, "ax" + a));
    /*  AI(W906-1203CTL-35) 20260911: tdOf, not td -- BOUND for in-place refresh.
        These were built with td(c(...)), which produces a cell with no data-k,
        so refresh() skipped the entire axis table and every number in it was
        the value at first paint. The positions were the worst of it: an encoder
        reading that has silently stopped updating looks exactly like an axis
        holding still, which is the failure this tree's own comments call out
        and which this table was quietly committing. */
    tr.appendChild(tdOf(tags, p + "opened", "bool"));
    tr.appendChild(tdOf(tags, p + "byId", "bool"));
    //AI(W906-1203HEX-1) 20260915: 站號 and 定址位址 render as "0x36  (54)" like
    //  every other station number on this page. They were raw decimal here
    //  while the IO tables beside them were hex+decimal, and Common Motion
    //  Utility's tree is hex -- so the same drive read as two different
    //  numbers depending on which screen you were looking at.
    //  ⓘ 站內軸 stays a plain integer: it is an index within a station (0 or 1),
    //  not an address, and hex would imply it shares the address namespace.
    tr.appendChild(tdOf(tags, p + "stationAlias", "station"));
    tr.appendChild(tdOf(tags, p + "stationAxis"));
    tr.appendChild(tdOf(tags, p + "station", "station"));
    tr.appendChild(tdOf(tags, p + "state"));
    tr.appendChild(tdOf(tags, p + "stateText"));
    tr.appendChild(tdOf(tags, p + "cmdPos", "pos"));
    tr.appendChild(tdOf(tags, p + "actPos", "pos"));
    tr.appendChild(tdOf(tags, p + "cmdVel", "pos"));
    tr.appendChild(tdOf(tags, p + "motionIO", "hex8"));
    /*  AI(W906-1203CTL-45) 20260912: the twenty-one lamps stay in this table
        ONLY on the all-axes view, where the table IS the overview and a row per
        axis is the point. On a single-axis pane they moved out to their own
        panel -- user: "馬達IO可以分到另一行嗎? 並且字體放大". Twenty-one columns
        after eleven others pushed them off the right edge of the screen, so the
        signals an operator is actually watching were the ones behind a
        horizontal scrollbar. */
    if (only === null) {
      for (const b of IO_BITS) tr.appendChild(lamp(tags, p + b.key, b));
    }
    t.appendChild(tr);
  }
  s.appendChild(wrapScroll(t));

  //AI(W906-1203CTL-9) 20260911: the operation panel, only when armed.
  if (controlArmed(tags)) s.appendChild(axisPanels(tags, only));
  return s;
}

/* ---------------------------------------------------------------------------
   AI(W906-1203CTL-9) 20260911: THE SINGLE-AXIS PANEL.

   Mirrors Common Motion Utility's 單軸運動 tab, which is what the user asked
   for on 20260911, and mirrors it in ORDER as well as in content -- an operator
   who knows the Utility must find the same controls in the same places:

     servo on / off        Acm_AxSetSvOn
     reset error           Acm_AxResetError
     jog -  / stop / jog + Acm_AxJog / Acm_AxStopDec
     emergency stop        Acm_AxStopEmg
     PTP relative / abs    Acm_AxMoveRel / Acm_AxMoveAbs
     home (mode + dir)     Acm_AxHome
     speeds, TWO families  Acm_SetF64Property PAR_Ax* (PTP) / CFG_AxJog* (JOG)

   ⚠ ONLY AXES THAT ACTUALLY OPENED GET A PANEL. A panel for a slot the monitor
   never opened would offer buttons that can only ever be refused, and sixteen
   of those would bury the nine that work.

   ⚠ THE ERROR_STOP NOTICE IS NOT DECORATION, and it is PER AXIS. Measured
   20260911: in the morning all 8 opened axes read ERROR_STOP; the same
   afternoon 16 were open and ax2/ax3 read READY. The C++ side refuses motion
   per axis and never clears the error as a side effect, so an operator who
   presses JOG on one axis and sees nothing happen needs to be told why on THAT
   panel -- and an operator pressing JOG on ax2 needs to know that one will
   actually move.

   ⚠ AND THE TWO SPEED FAMILIES ARE SHOWN SEPARATELY ON PURPOSE. Setting the PTP
   run speed does not change how JOG moves -- they are different property IDs on
   the same axis. One combined "speed" box would produce the most confusing
   possible outcome: a value accepted, a vendor SUCCESS, and no change in the
   thing the operator was watching.
   --------------------------------------------------------------------------- */
function axisPanels(tags, only) {
  const wrap = el("div", "panels");
  const dry = controlDry(tags);
  let shown = 0;

  for (let a = 0; a < AXIS_SLOTS; a++) {
    if (tags.get(`pci1203.ax${a}.opened`) !== true) continue;
    if (only !== null && only !== undefined && a !== only) continue;
    shown++;
    const tag = `pci1203.ax${a}`;
    const st  = tags.get(`pci1203.ax${a}.stateText`);
    const inError = st === "ERROR_STOP";

    const p = el("div", "panel");
    /*  AI(W906-1203CTL-32) 20260911: station first, slot index after it. The
        operator matches the panel to a drive in the cabinet by its station
        number; ax<n> stays because it is the key every tag on this panel is
        published under. */
    //AI(W906-1203CTL-39) 20260911: address first, dial in brackets -- the panel
    //  head is what an operator reads with a hand on the jog button, so it
    //  carries the number they will quote if it goes wrong.
    const pAddr = tags.get(`pci1203.ax${a}.station`);
    const pDial = tags.get(`pci1203.ax${a}.stationAlias`);
    const pSub  = tags.get(`pci1203.ax${a}.stationAxis`);
    const pCnt  = tags.get(`pci1203.ax${a}.stationAxes`);
    const headText =
      (typeof pAddr === "number" && pAddr >= 0 && typeof pSub === "number" && pSub >= 0)
        //AI(W906-1203HEX-1) 20260915: hex + decimal. See the rail label note.
        ? ((pCnt === 1 ? `位址 ${fmtStation(pAddr)}`
                       : `位址 ${fmtStation(pAddr)} · 軸 ${pSub}`) +
           ((typeof pDial === "number" && pDial >= 0) ? `  （站 ${fmtStation(pDial)}）` : "") +
           `   (ax${a})`)
        : `ax${a}`;
    const h = el("div", "panelhead", headText);
    /*  AI(W906-1203CTL-35) 20260911: bound, and colour-coded BY VALUE on every
        refresh rather than only at first paint. data-statebad names the value
        that means trouble; refresh() reads it. Without this the head could say
        READY in green while the axis had since faulted -- a stale label that
        LOOKS authoritative is worse than no label. */
    const stTag = `pci1203.ax${a}.stateText`;
    const stSpan = el("span", inError ? "bad" : "ok",
                      (typeof st === "string" && st.length) ? "  " + st : "");
    stSpan.dataset.k = stTag;
    stSpan.dataset.statebad = "ERROR_STOP";
    h.appendChild(stSpan);
    /*  AI(W906-1203CTL-25) 20260911: THE POSITION IS IN THE PANEL HEAD, beside
        the buttons that move it. User: "也可以看到馬達數值 / 可以左右移動 /
        跟範例程式 差不多的馬達介面" -- and Common Motion Utility's 單軸運動 tab
        shows command and actual position directly above its jog controls.
        It was previously only in the table at the top of the pane, so an
        operator jogging ax7 had to look 16 rows away to see whether it moved.
        ⚠ BOTH POSITIONS, not just actual: they differ while a move is in
        progress and while a drive is in error, and which one is moving is the
        first thing a jog tells you. */
    /*  AI(W906-1203CTL-34) 20260911: labels follow Common Motion Utility's
        位置 block -- 命令 / 回饋 -- and 命令速度 joins them, which is what the
        Utility shows under 當前狀態. User: "裡面有回饋位置數值 你這邊好像都沒
        加入". */
    p.appendChild(h);

    /*  AI(W906-1203WHY-1) 20260915: ⚠ THE REFUSAL BELONGS WHERE THE BUTTON IS.
        User pressed set on 電子齒輪比 分母 with 162 against a numerator of
        10485760 and reported "分母為什麼寫不進去?". The command WAS refused,
        correctly -- 10485760/162 = 64726.9 and the manual's bound is 64000 --
        and the reason was written, in full, to pci1203.control.lastWhy.

        Which is rendered in the "last command" table, on a DIFFERENT VIEW.

        So from the operator's seat: press, nothing moves, no message. Measured
        on the axis pane at the moment of the refusal -- the word "outside" from
        the refusal text appeared NOWHERE in document.innerText. This module has
        spent weeks making refusals explain themselves and then put the
        explanation on a page nobody was looking at.

        ⚠ Bound to the CONTROL tags, not to this axis: there is one "last
        command" for the whole card. The id is shown with it so an operator can
        tell a fresh refusal from one left over from a different pane -- without
        that, a stale reason under a button that just worked is worse than
        silence. */
    {
      const wr = el("div", "cmdresult");
      wr.dataset.cmdresult = "1";
      wr.textContent = "";
      p.appendChild(wr);
    }

    /*  AI(W906-1203WHY-2) 20260915: WHY THIS AXIS WILL NOT MOVE.
        User turned the card's 正極限 off, the axis still would not go positive,
        and concluded "命令一樣不會發送給馬達". Measured on that exact axis:

            Acm_AxMoveRel(+1) -> 0x00000000 SUCCESS
            cmdPos  +1.000     <- the master DID send the target
            actPos  +0.000     <- the motor did not follow

        So the card's switch had worked completely and the command WAS going
        out. Two other things were true at once: SVON was 0, and the drive's
        P-OT was active with Pn50A=n.0881 (still allocated to SI0).

        ⚠ "the command was not sent" and "the command was sent and ignored" look
        identical if you only watch the motor -- and this page showed 命令 and
        回饋 side by side without ever saying what their divergence MEANS. Every
        fact needed was already on screen, in four different rows, and the
        operator still could not get an answer out of it. This row does the
        joining. */
    {
      const ws = el("div", "cmdresult");
      ws.dataset.whystuck = String(a);
      ws.textContent = "";
      p.appendChild(ws);
    }

    /*  AI(W906-1203CTL-45) 20260912: 位置 IS ITS OWN PANEL NOW.
        User: "回饋數值 獨立面板 不要跟其他混淆". It used to be three small spans
        floated into the panel heading, sharing a line with the station name and
        the state -- which is exactly where a number gets misread as belonging
        to something else. It is now a block of its own with large tabular
        figures, because this is the readout an operator stares at while
        pressing JOG to decide whether anything actually moved.
        ⚠ Still bound (data-k): these three froze at first paint once already,
        and a stale position is indistinguishable from an axis holding still. */
    const posPanel = el("div", "subpanel");
    posPanel.appendChild(el("div", "subhead", "位置 / 回饋"));
    const posGrid = el("div", "bignums");
    const bigNum = (tag, label, unit) => {
      const cell = el("div", "bignum");
      cell.appendChild(el("div", "bignumlabel", label));
      const c0 = cellOf(tags, tag, fmtPos);
      cell.appendChild(bindCell(el("div", "bignumval " + c0.cls, c0.text), tag, "pos", "bignumval"));
      if (unit) cell.appendChild(el("div", "bignumunit", unit));
      posGrid.appendChild(cell);
    };
    bigNum(`pci1203.ax${a}.cmdPos`, "命令 Command", "PPU");
    bigNum(`pci1203.ax${a}.actPos`, "回饋 Feedback", "PPU");
    bigNum(`pci1203.ax${a}.cmdVel`, "命令速度 Cmd Vel", "PPU/S");
    posPanel.appendChild(posGrid);
    p.appendChild(posPanel);

    /*  AI(W906-1203CTL-45) 20260912: 馬達 I/O ON ITS OWN ROW, ENLARGED.
        User: "馬達IO可以分到另一行嗎? 並且字體放大". These were twenty-one narrow
        columns at the far right of a table eleven columns wide -- the signals
        being watched were the ones behind the scrollbar. Here each lamp gets
        its label above its state, wraps naturally, and is legible from a step
        back from the cabinet.
        ⚠ Same lampState + data-lampgood binding as the table used, so they
        update in place; this is presentation only, not a second reader. */
    const ioPanel = el("div", "subpanel");
    const ioHead = el("div", "subhead", "馬達 I/O 狀態");
    const mioC = cellOf(tags, `pci1203.ax${a}.motionIO`, fmtHex8);
    ioHead.appendChild(bindCell(el("span", "subheadval " + mioC.cls, mioC.text),
                                `pci1203.ax${a}.motionIO`, "hex8", "subheadval"));
    ioPanel.appendChild(ioHead);
    const ioGrid = el("div", "lampgrid");
    for (const b of IO_BITS) {
      const good = (b.good === true) ? "true" : (b.good === false) ? "false" : "";
      const cell = el("div", "lampbig");
      cell.appendChild(el("div", "lampbiglabel", b.label));
      const st = lampState(tags, `pci1203.ax${a}.${b.key}`, good, undefined);
      const sp = el("div", "lampbigval " + st.cls, st.text);
      sp.dataset.k = `pci1203.ax${a}.${b.key}`;
      sp.dataset.lampgood = good;
      sp.dataset.lampbase = "lampbigval";
      cell.appendChild(sp);
      ioGrid.appendChild(cell);
    }
    ioPanel.appendChild(ioGrid);
    p.appendChild(ioPanel);

    /*  AI(W906-1203CTL-34) 20260911: 最新錯誤狀態 -- Common Motion Utility has
        this block at the bottom of its 單軸運動 tab and the user asked for it:
        "有異常他也會說明是甚麼異常 叫我怎解除".

        ⚠ THE EXPLANATION TEXT IS THE VENDOR'S, NOT OURS. It comes from
        Acm_GetErrorMessage on the C++ side, which is the same call that
        produces the Utility's 錯誤資訊 string, so the two cannot drift apart.
        Writing a friendlier sentence here would be inventing a diagnosis and
        putting it on a machine screen, which sends someone to fix the wrong
        thing with confidence.

        The "how to clear it" half IS ours, but it is PROCEDURE, not diagnosis:
        which button, in what order, and what the result tells you. That is
        knowledge about this page, not a claim about the drive. */
    const errCode = tags.get(`pci1203.ax${a}.driveErr`);
    const errText = tags.get(`pci1203.ax${a}.driveErrText`);
    /*  AI(W906-1203ALM-1) 20260912: THE DRIVE'S OWN A.xxx, read from CoE 603Fh.
        Until today this page said the A.xxx was only visible on the drive's
        seven-segment display; that sentence is now false and has been corrected
        below rather than left to age.

        driveAlarm is absent (not 0) when nothing has been read -- the C++ side
        gates it on driveAlarmValid -- so "no alarm" and "not asked" stay
        distinguishable here too.

        driveAlarmName is absent for a drive that is not Sigma-X, deliberately:
        603Fh is vendor-defined, so Yaskawa's table must not name another
        vendor's code. User ruling 20260912: "如果是使用yaskawa 才寫入面板".
        driveModel is published so this page can SAY that rather than show a
        blank nobody can explain. */
    const almCode  = tags.get(`pci1203.ax${a}.driveAlarm`);
    const almName  = tags.get(`pci1203.ax${a}.driveAlarmName`);
    const drvModel = tags.get(`pci1203.ax${a}.driveModel`);
    const drvIsSigmaX = typeof drvModel === "string" && drvModel.indexOf("SGDX") >= 0;
    /*  Declared here, not inside the faulted branch below, because the alarm
        row has to distinguish "not read yet" from "not applicable" even on an
        axis the card does not consider faulted. */
    const almBitOn = tags.get(`pci1203.ax${a}.alm`) === true;
    /*  AI(W906-1203CTL-45) 20260912: 異常 IS ITS OWN PANEL, ALWAYS PRESENT.
        User: "異常也獨立面板". Two changes, and the second matters more than the
        layout: it used to render only when something was wrong, so an operator
        could not tell "no fault" from "this pane does not report faults". The
        vendor's own 最新錯誤狀態 block is always on screen and reads
        "錯誤代碼 0 / 錯誤資訊 Success." when healthy -- so this one does too,
        and only turns red when there is something to be red about. */
    {
      const faulted = inError || (typeof errCode === "number" && errCode !== 0);
      const eb = el("div", "subpanel " + (faulted ? "subpanelbad" : ""));
      eb.appendChild(el("div", "subhead", "最新錯誤狀態"));
      const row = el("div", "errrow");
      row.appendChild(el("span", "panelhint", "錯誤代碼"));
      row.appendChild(el("span", (typeof errCode === "number" && errCode !== 0)
                                  ? "bad" : "ok",
        (typeof errCode === "number") ? fmtHex8(errCode) : "—"));
      row.appendChild(el("span", "panelhint", "錯誤資訊"));
      row.appendChild(el("span", faulted ? "val" : "ok",
        (typeof errText === "string" && errText.length) ? errText
          : (faulted ? "（卡片這一層沒有錯誤碼可報 —— 見下面「怎麼解除」）"
                     : "Success.")));
      eb.appendChild(row);

      /*  AI(W906-1203ALM-1) 20260912: 驅動器警報 -- the number off the DRIVE,
          not off the card. Its own row because it answers a different question
          from 錯誤代碼 above: that one is "did an API call fail", this one is
          "what is the servo complaining about".

          ⚠ Formatting reproduces Yaskawa's own spelling: uppercase hex with B
          and D in lower case, which is how a seven-segment display writes them
          (A.d00, not A.D00). VERIFIED against the manual, not assumed -- the
          rule reproduces all 135 hex-spelled codes in the extracted table. */
      {
        const ar = el("div", "errrow");
        ar.appendChild(el("span", "panelhint", "驅動器警報"));
        if (typeof almCode === "number") {
          /*  padStart, never slice: a code wider than three digits would be
              TRUNCATED by a fixed-width slice, and a truncated alarm number is
              a different alarm reported confidently. Pad up, never cut down. */
          const disp = "A." + almCode.toString(16).toUpperCase()
                                .padStart(3, "0")
                                .replace(/B/g, "b").replace(/D/g, "d");
          ar.appendChild(el("span", almCode !== 0 ? "bad" : "ok",
                            almCode !== 0 ? disp : "無警報"));
          if (almCode !== 0) {
            ar.appendChild(el("span", "panelhint", "警報名稱"));
            ar.appendChild(el("span", "val",
              (typeof almName === "string" && almName.length)
                ? almName
                : (drvIsSigmaX
                    ? "（此代碼不在 SIEPC71081202 的清單中 —— 韌體可能比手冊新）"
                    : `（型號 ${drvModel || "未知"} 不是 Sigma-X，不套用安川對照表）`)));
          }
        } else {
          /*  Absent, not zero. Says which of the two reasons applies rather
              than rendering both as a dash. */
          ar.appendChild(el("span", "panelhint",
            almBitOn ? "尚未讀回（ALM 亮，下一次輪詢會讀 603Fh）"
                     : "—（此軸未報警，不讀）"));
        }
        eb.appendChild(ar);
      }

      if (!faulted) {
        eb.appendChild(el("div", "errhow2",
          "此軸目前沒有回報錯誤。⚠ 上面的「錯誤代碼」是卡片 API 這一層的，" +
          "「驅動器警報」才是驅動器機身七段顯示的 A.xxx —— 由 CoE 物件 603Fh 讀回。"));
      }
      if (faulted) {

      /*  What the lamps above already say, turned into the next action. Keyed
          on the live MotionIO bits -- not on a guess about the drive. */
      const alm  = tags.get(`pci1203.ax${a}.alm`)  === true;
      const emg  = tags.get(`pci1203.ax${a}.emg`)  === true;
      const lp   = tags.get(`pci1203.ax${a}.limitP`) === true;
      const ln   = tags.get(`pci1203.ax${a}.limitN`) === true;
      const slp  = tags.get(`pci1203.ax${a}.sLimitP`) === true;
      const sln  = tags.get(`pci1203.ax${a}.sLimitN`) === true;
      const how = [];
      if (emg) how.push("EMG 亮：緊急停止輸入生效中，先解除它，否則清了也會立刻回來。");
      /*  AI(W906-1203ALM-1) 20260912: this used to say the A.xxx "只看得到 ...
          在驅動器機身七段顯示上", which stopped being true the moment 603Fh was
          read. A page that sends an operator to the cabinet for a number it is
          already showing wastes a trip and undermines the rest of the panel. */
      if (alm) how.push(
        "ALM 亮＝驅動器自己在報警。實際的 A.xxx 編號顯示在上面的「驅動器警報」" +
        "（由 CoE 物件 603Fh 讀回，與機身七段顯示同一個號碼）。" +
        "先照那個代碼排除原因，再按下面的「reset error」。");
      if (lp || ln) how.push(
        `硬體極限 ${lp ? "LMT+" : ""}${(lp && ln) ? " 與 " : ""}${ln ? "LMT−" : ""} 亮：` +
        "先往反方向脫離，再清除。⚠ 兩端同時亮通常代表接線或極性問題，不是真的撞到兩邊。");
      if (slp || sln) how.push(
        `軟體極限 ${slp ? "SLMT+" : ""}${(slp && sln) ? " 與 " : ""}${sln ? "SLMT−" : ""} 亮：` +
        "這是設定值不是感測器，檢查軟極限參數。");
      how.push("按「reset error」之後如果狀態立刻跳回 ERROR_STOP，" +
               "代表故障條件還在；維持乾淨才表示已經是歷史。");
      const ul = el("ul", "errhow");
      for (const line of how) ul.appendChild(el("li", null, line));
      eb.appendChild(el("div", "errhead2", "怎麼解除"));
      eb.appendChild(ul);
      }   // if (faulted)
      p.appendChild(eb);
    }

    if (inError) {
      p.appendChild(el("div", "emptynote",
        "This axis is in ERROR_STOP, so every motion command below will be REFUSED. " +
        "Press “reset error” deliberately first — the C++ side will not clear it as a " +
        "side effect of a move, because that hides the reason it stopped."));
    }

    /*  ================== THE FIVE SECTIONS ==================================
        AI(W906-1203SECT-1) 20260917. User: "幫我介面分區塊 / 不同區塊用不同顏色
        區分 / 顏色不要差異太大 / 有相關的才分到同一區塊".

        ⚠ WHY THE CONTAINERS ARE CREATED AND MOUNTED HERE, ALL AT ONCE, rather
        than wrapped around each group where it is built: the rows below are
        appended in SOURCE order, and the source order was already wrong. 移動
        and 絕對位置 are built here, then the limits panel, then the gear panel,
        and only THEN 連續 and 疊加運動 -- so two halves of "make the axis move"
        sat either side of two unrelated settings panels. Wrapping in place
        would have preserved that, producing five tidy boxes in a nonsensical
        order. Mounting the containers now fixes the order once: each group is
        routed to its section, and the sections appear in the order below
        whatever order the code happens to build them in.

        ⚠ THE COLOURS ARE DELIBERATELY CLOSE TOGETHER -- user: "顏色不要差異太大".
        All five are the same lightness and saturation, separated only by hue,
        and each is a 3px left border plus a ~4% wash rather than a filled
        block. A filled block would collide with .subpanel, which already uses
        background to mean "a different kind of thing".
        ⓘ 極限 gets the warm one. It is the only section where a wrong setting
        removes a physical protection, and the page's existing palette already
        means "careful" with amber (--warn) -- so the one hue that stands out
        slightly is on the one group where that is the right signal. */
    const mkSec = (cls, title, sub) => {
      const s = el("div", "sec " + cls);
      const h = el("div", "sechead", title);
      if (sub) h.appendChild(el("span", "sechint", sub));
      s.appendChild(h);
      p.appendChild(s);
      return s;
    };
    const secRun   = mkSec("sec-run",   "運轉 MOTION",
                           "激磁、移動、停止 — 按下去馬達就會動");
    const secSpeed = mkSec("sec-speed", "速度 SPEED",
                           "上面那些移動用的速度曲線");
    const secHome  = mkSec("sec-home",  "歸原點 HOMING",
                           "回原點的動作與驅動器端的歸原點參數");
    const secLimit = mkSec("sec-limit", "極限 LIMITS",
                           "⚠ 卡片與驅動器兩層，關掉就沒有保護");
    const secMotor = mkSec("sec-motor", "馬達與編碼器 MOTOR / ENCODER",
                           "方向、解析度、電子齒輪比 — 改了要存檔");

    // --- servo + error ---
    const r1 = el("div", "panelrow");
    r1.appendChild(el("span", "panellabel", "servo"));
    r1.appendChild(cmdButton("SVON", "pci1203.ax.svOn", tag,
      { value: 1, title: "Acm_AxSetSvOn(ax, 1)" }));
    r1.appendChild(cmdButton("SVOFF", "pci1203.ax.svOn", tag,
      { value: 0, title: "Acm_AxSetSvOn(ax, 0)" }));
    r1.appendChild(cmdButton("reset error", "pci1203.ax.resetError", tag,
      { cls: "warnbtn", title: "Acm_AxResetError(ax) — clears the drive error, moves nothing" }));
    secRun.appendChild(r1);

    /*  AI(W906-1203ALM-5) 20260912: 絕對編碼器重置 (Fn008). User request:
        "你可以幫我每個馬達頁面寫個 啟動Fn008 的按鈕 / 按下時 有格確認視窗".

        ⚠ ITS OWN ROW, AND NOT BESIDE THE MOTION BUTTONS, deliberately. Every
        other control on this panel does something the machine can be driven
        back out of. This one resets the encoder's multiturn data to between -2
        and +2 rotations: the axis's ORIGIN IS GONE and every stored coordinate
        for it now means somewhere else. Sigma-X manual SIEPC71081202 s5.15 puts
        that in a WARNING box. Putting it in the same row as SVON would make it
        look like a peer of SVON.

        ⚠ It is shown only when the drive is actually carrying A.810/A.820. The
        C++ side refuses otherwise, but a button that is present and always
        refused teaches an operator to ignore refusals. */
    {
      const almc = tags.get(`pci1203.ax${a}.driveAlarm`);
      const stn  = tags.get(`pci1203.ax${a}.station`);
      const needsReset = (almc === 0x0810 || almc === 0x0820);
      if (needsReset && typeof stn === "number") {
        const rz = el("div", "panelrow");
        rz.appendChild(el("span", "panellabel", "絕對編碼器"));
        const disp = "A." + almc.toString(16).toUpperCase()
                              .padStart(3, "0").replace(/B/g, "b").replace(/D/g, "d");
        /*  AI(W906-1203HOME-1) 20260915: the dialog's wording now travels WITH
            the button. It used to be hard-coded inside askConfirm(), which was
            fine while Fn008 was the only confirmed command and became a defect
            the moment it was not -- 套用電子齒輪比 opened the same dialog and
            asked the operator to confirm an absolute encoder reset. */
        const fn008 = cmdButton(`啟動 Fn008 絕對編碼器重置`,
          "pci1203.ax.absEncoderReset", tag, {
            cls: "warnbtn",
            //AI(W906-1203HEX-1) 20260915: hex AND decimal here too -- the
            //  operator is being asked to confirm WHICH DRIVE, and they are
            //  reading it off Common Motion Utility's tree, which is hex.
            //  ⚠ The TYPED value stays decimal and the prompt says so: the C++
            //  side compares it against the axis's own station number, and a
            //  hex "36" would be decimal 54's neighbour-but-one. Showing both
            //  and naming which to type is the only combination that cannot be
            //  read two ways.
            confirm: `站 ${fmtStation(stn)} — ${disp}`,
            station: stn,
            title: "SDO 2710h / 1008h — Fn008。清除 A.810/A.820，並且會清掉這一軸的原點"
          });
        fn008.dataset.confirmbody =
          "即將對 {what} 執行 Fn008 絕對編碼器重置。\n\n" +
          "• 多圈資料會被重置到 −2 ～ +2 轉之間\n" +
          "• 這一軸的原點會消失，機械參考位置改變\n" +
          "• 完成後必須重新 Home，否則任何座標都不再是原來的意思\n" +
          "• 驅動器必須斷電再上電才會生效（PC 重開機不算）";
        fn008.dataset.confirmgo = "執行 Fn008";
        rz.appendChild(fn008);
        secMotor.appendChild(rz);
        secMotor.appendChild(el("div", "errhow2",
          `⚠ ${disp} 無法用「reset error」清除（手冊 §5.15.1 明說 Fault Reset 對 ` +
          `A.810/A.820 無效）。Fn008 是唯一的辦法，但它會把多圈資料重置到 −2～+2 轉之間 ` +
          `—— 這一軸的原點會消失，必須重新 Home，而且驅動器要斷電再上電才會生效。`));
      }
    }

    /*  --- the arrows, and STOP ---
        AI(W906-1203ALM-10) 20260914: ⚠ THESE WERE Acm_AxJog AND IT DOES NOTHING
        ON THIS CARD. Measured on ax3, all three back to back from the same
        state, moving away from the asserted limit:
            Acm_AxJog(ax, dir)      SUCCESS, cmdPos moved 0.000, state stayed
                                    READY, and Acm_GetLastError read 0x8000510B
                                    "Invalid axis states." for the whole 2 s
            Acm_AxMoveVel(ax, dir)  CONTINUE_MOTION, cmdPos moved -17647
            Acm_AxMoveRel(ax, -d)   PTP_MOTION, cmdPos moved -10000 exactly
        So the button reported success and the machine did nothing -- the single
        hardest failure to diagnose from a screen, and the one the user hit.

        WHY Acm_AxJog IS THE WRONG CALL HERE, not just an unlucky one: on this
        SDK jog is a HARDWARE function. AdvMotPropID.h carries CFG_AxJogPAssign
        and CFG_AxJogNAssign (jog assigned to DI CHANNELS) and FT_AxJogMap; the
        call arms that, and with nothing assigned the axis is in no state to
        jog. The vendor ships ZERO examples using Acm_AxJog -- it appears only
        in the API headers.

        And Common Motion Utility's arrows are not jogs either. Its 運動模式
        radio decides: P to P sends Acm_AxMoveRel(±距離), Continue sends
        Acm_AxMoveVel(dir). The user's screenshots are on P to P with 距離
        10000, and 命令 moved by exactly -10000 per press. So these arrows now
        do what that panel does, with the same distance box feeding them. */
    const r2 = el("div", "panelrow");
    r2.appendChild(el("span", "panellabel", "移動 Move"));
    /*  ONE distance box feeding BOTH arrows, exactly like the Utility's 距離
        field. The left arrow negates it rather than reading a second box: two
        boxes drift apart, and an operator who set 1000 in one and 5000 in the
        other gets a different move depending which arrow they press. */
    r2.appendChild(cmdButton("◀ 負向", "pci1203.ax.moveRel", tag,
      { from: `ax${a}.rel`, negate: true,
        title: "Acm_AxMoveRel(ax, −距離) — Common Motion Utility 的「←」，P to P" }));
    r2.appendChild(numBox(`ax${a}.rel`, 1000, "7em"));
    r2.appendChild(cmdButton("正向 ▶", "pci1203.ax.moveRel", tag,
      { from: `ax${a}.rel`,
        title: "Acm_AxMoveRel(ax, +距離) — Common Motion Utility 的「→」，P to P" }));
    r2.appendChild(cmdButton("STOP", "pci1203.ax.stop", tag,
      { cls: "warnbtn", title: "Acm_AxStopDec(ax) — controlled decelerated stop" }));
    /* Emergency stop sits apart and is styled apart. It is the one control on
       this page whose whole value is being found without reading. */
    r2.appendChild(cmdButton("EMG STOP", "pci1203.ax.emgStop", tag,
      { cls: "emgbtn", title: "Acm_AxStopEmg(ax) — immediate stop, no deceleration ramp" }));
    secRun.appendChild(r2);

    // --- PTP ---
    const r3 = el("div", "panelrow");
    /*  AI(W906-1203ALM-10) 20260914: MOVE REL and its box moved up into the
        arrow row -- they were the same command with the same distance, and two
        controls that do one thing is how a panel starts lying about itself. */
    r3.appendChild(el("span", "panellabel", "絕對位置 Abs"));
    r3.appendChild(numBox(`ax${a}.abs`, 0, "7em"));
    r3.appendChild(cmdButton("MOVE ABS", "pci1203.ax.moveAbs", tag,
      { from: `ax${a}.abs`, title: "Acm_AxMoveAbs(ax, position)" }));
    secRun.appendChild(r3);

    /*  AI(W906-1203ALM-21) 20260915: 極限設定 -- its own panel, per axis.
        User: "我希望可以在介面 可以控制每顆馬達 設定正負極限 相關參數".

        ⚠ EVERY ROW SHOWS THE CARD'S READ-BACK BESIDE THE WRITE BOX, and on a
        protection that is not the same nicety it is on a speed: a limit that is
        OFF while the screen implies it is ON is discovered when an axis reaches
        a hard stop. Nothing here is pre-filled from what was last written.

        ⚠⚠ AND THE CARD IS ONLY HALF OF THE LIMIT ON THIS MACHINE. Measured
        20260915, station 1: Pn50A = n.0881 (P-OT allocated to input SI0),
        Pn50B = n.8881 (N-OT on SI1). The switches are wired to the DRIVE, so
        the SERVOPACK refuses that direction by itself. Turning the card's
        limit off stops the CARD faulting and does not make the drive move --
        which reads exactly like "the setting did nothing". The panel says so
        rather than letting the operator find out. */
    /*  AI(W906-1203PHYS-1) 20260916: ⚠ THIS AXIS IS REAL, ITS STATION NUMBER
        IS NOT UNIQUE.
        User: "你讀出來了但是 Common Motion Utility 是把該模組分配到馬達ㄟ" --
        so these drives DO get axes, and they now do here too, opened by
        physical index exactly as the Utility does.

        The distinction this banner exists to make, because it is not obvious
        and getting it wrong is expensive either way:
            MOVING it   is safe -- the handle is a physical axis index, the
                        card knows precisely which drive that is
            WRITING a parameter is NOT -- every SDO write is addressed by
                        STATION, and another drive answers to the same number.
                        It would return SUCCESS having reconfigured the twin.
        Saying only "this axis is unreliable" would stop an operator using a
        motor that works; saying nothing would let them write a gear ratio into
        the wrong SERVOPACK. So it says which half. */
    if (tags.get(`pci1203.ax${a}.stationAmbiguous`) === true) {
      const w = el("div", "subpanel");
      w.appendChild(el("div", "subhead", "⚠ 這一軸的站號跟別台重複"));
      w.appendChild(el("div", "errhow2",
        `這一顆是用「實體軸編號」開的（跟 Common Motion Utility 一樣），` +
        `所以它是真的、位置是真的、移動也是真的 —— 可以 JOG、可以 MOVE。`));
      w.appendChild(el("div", "errhow2",
        `⚠ 但下面「極限設定 / 電子齒輪比 / 回HOME設定」那些是**寫到站號**的，` +
        `而這個站號（${fmtStation(tags.get(`pci1203.ax${a}.station`))}）` +
        `環上有兩台在用。寫下去會落在另一台身上，而且會回報成功 —— ` +
        `所以這一軸的參數寫入一律被拒絕，不是故障。`));
      w.appendChild(el("div", "errhow2",
        `要讓它能改參數：用 Common Motion Utility 把 SubDevice ID 改成唯一，` +
        `再把那些站斷電上電（重開電腦沒用，站號存在從站裡）。` +
        `哪兩台在撞，看左邊側欄「⚠ 站號相撞」那一組。`));
      p.insertBefore(w, secRun);
    }

    {
      const lp = el("div", "subpanel");
      /*  AI(W906-1203SECT-1) 20260917: the subpanel's own "極限設定 LIMITS"
          heading is gone -- the section it now lives in is titled 極限 LIMITS,
          two lines above it, so the page was saying the same word twice in two
          different type styles. The subpanel box itself stays: it still groups
          the card half and the drive half. */

      const limRow = (label, key, cmd, hint, isFlag) => {
        const row = el("div", "panelrow");
        row.appendChild(el("span", "panellabel", label));
        row.appendChild(el("span", "panelhint", "卡片目前"));
        const cur = el("span", "val");
        cur.dataset.k = `pci1203.ax${a}.limit.${key}`;
        /*  AI(W906-1203LOGIC-1) 20260917: ⚠ THE LOGIC ROWS ARE NOT 開/關.
            They were, for one build, because they arrived as isFlag rows and
            isFlag meant "onoff" -- so ACT_LOW rendered as "0 關", which reads
            as "this limit is switched off". It is the opposite kind of fact:
            the limit is on, and 0 says which electrical level counts as
            pressed. A protection that reads as disabled when it is armed is
            the same class of mislabel as one that reads as armed when it is
            off, and this panel exists to prevent exactly that. */
        const isLogic = (key === "pelLogic" || key === "melLogic");
        cur.dataset.f = (key === "elReact") ? "elReact"
                      : isLogic ? "hlmtLogic"
                      : (isFlag ? "onoff" : "pos");
        cur.textContent = "---";
        row.appendChild(cur);
        if (isLogic) {
          //  Labelled with the SDK's own constant names, so a value on screen
          //  can be checked against AdvMotDrv.h and against the vendor example.
          row.appendChild(cmdButton("0 ACT_LOW", cmd, tag, { value: 0 }));
          row.appendChild(cmdButton("1 ACT_HIGH", cmd, tag, { value: 1 }));
        } else if (isFlag) {
          row.appendChild(cmdButton("開 1", cmd, tag, { value: 1 }));
          row.appendChild(cmdButton("關 0", cmd, tag, { cls: "warnbtn", value: 0 }));
        } else {
          row.appendChild(numBox(`ax${a}.${key}`, 0, "8em"));
          row.appendChild(cmdButton("set", cmd, tag, { from: `ax${a}.${key}` }));
        }
        if (hint) row.appendChild(el("span", "panelhint", hint));
        lp.appendChild(row);
      };

      limRow("硬體極限 總開關", "elEnable",  "pci1203.ax.setLimit.elEnable",
             "關掉＝卡片不再因極限停止", true);
      limRow("正極限 LMT+",    "pelEnable", "pci1203.ax.setLimit.pelEnable",  "", true);
      limRow("負極限 LMT−",    "melEnable", "pci1203.ax.setLimit.melEnable",  "", true);
      limRow("觸發後動作",      "elReact",   "pci1203.ax.setLimit.elReact",
             "0 = 立即停止　1 = 減速停止", true);
      limRow("軟體正極限 開關", "swPelEnable", "pci1203.ax.setLimit.swPelEnable", "", true);
      limRow("軟體負極限 開關", "swMelEnable", "pci1203.ax.setLimit.swMelEnable", "", true);
      limRow("軟體正極限 位置", "swPelValue",  "pci1203.ax.setLimit.swPelValue",  "PPU", false);
      limRow("軟體負極限 位置", "swMelValue",  "pci1203.ax.setLimit.swMelValue",  "PPU", false);

      /*  AI(W906-1203LOGIC-1) 20260917: HLMT+ / HLMT- Logic.
          User sent a screenshot of the vendor example listing
              HLMT+ Enable / HLMT+ Logic / HLMT- Logic / HLMT React
          and asked for the missing ones. Enable and React were already here;
          the two LOGIC settings were not.

          ⚠ LOGIC IS NOT ENABLE. Enable decides whether the card watches the
          input; logic decides which electrical level MEANS "pressed". Getting
          it backwards gives one of two failures, and the second is the bad one:
            * card thinks the switch is permanently pressed -> the axis refuses
              that direction from the start, which at least LOOKS wrong
            * card thinks it is permanently clear -> no protection at all, and
              nothing looks wrong until the mechanism hits its hard stop
          AdvMotDrv.h:650-651 -- HLMT_ACT_LOW = 0, HLMT_ACT_HIGH = 1. */
      limRow("正極限 觸發準位", "pelLogic", "pci1203.ax.setLimit.pelLogic",
             "0 = ACT_LOW　1 = ACT_HIGH", true);
      limRow("負極限 觸發準位", "melLogic", "pci1203.ax.setLimit.melLogic",
             "0 = ACT_LOW　1 = ACT_HIGH", true);

      /*  AI(W906-1203POT-1) 20260915: ⚠ THE DRIVE'S HALF, ON THE PANEL.
          This block used to be card-only, with a NOTE below saying the drive
          was the other half. A user turned 正極限 off here, still could not
          move positive, and reported the switch as broken.

          It was not broken. Measured at that moment:
              ax3 站1  卡片 Pel=0   Pn50A=n.0881  60FDh P-OT=1   <- drive refuses
              ax7 站41 卡片 Pel=1   Pn50A=n.8881  60FDh P-OT=0   <- drive disabled
          The card obeyed. The drive was still listening on SI0 and the switch
          was pressed. A note that a second layer EXISTS is worth much less than
          that layer's actual value, so the value is here now. */
      {
        /*  AI(W906-1203OT-1) 20260915: this block's own station / sub-axis, not
            borrowed from an enclosing one -- there isn't one here. The gear and
            HOME panels each declare their own for the same reason. ⚠ `sub`
            decides 250Ah vs 2A0Ah, so on a two-axis SGDXW getting it wrong
            rewrites the OTHER motor's overtravel setting. */
        const stn  = tags.get(`${tag}.station`);
        const sub  = tags.get(`pci1203.ax${a}.gear.subAxis`);
        const half = (sub === 1) ? "B" : (sub === 0) ? "A" : "?";

        const dp = el("div", "panelrow");
        dp.appendChild(el("span", "panellabel", "驅動器 正極限 P-OT"));
        dp.appendChild(el("span", "panelhint", "Pn50A"));
        const v = el("span", "val");
        v.dataset.k = `pci1203.ax${a}.enc.pn50A`;
        v.dataset.f = "potA";
        v.textContent = "---";
        dp.appendChild(v);

        /*  AI(W906-1203OT-1) 20260915: ⚠ AND NOW IT IS SETTABLE, which is what
            the user asked for on day one and kept asking for:
                "我就是想要把LIMIT+ PASS 掉 / 我一開始就要把它PASS掉 / 但是一直沒辦法"

            Everything before this -- the card's 正極限 switch, the diagnosis
            rows, this panel showing Pn50A -- told them WHY it did not work
            without giving them the thing that makes it work. Showing someone
            the blocker is not the same as removing it.

            ⚠ Value 8 = "the signal is always inactive" (s5.10.2). It is a
            READ-MODIFY-WRITE: Pn50A's other three digits hold the input
            allocation mode and the /S-ON and /P-CON assignments, so writing a
            bare 8 would leave the axis unable to ENABLE. The C++ side refuses
            outright when it has not read the register back. */
        const otBody = (which, on) =>
          (on
            ? "即將把 {what} 的" + which + "重新啟用（配置值 0 = 接回輸入端子）。\n\n" +
              "• 驅動器會重新聽那顆極限開關\n" +
              "• 如果開關現在正被壓住，這一軸立刻會拒絕往那個方向動"
            : "即將把 {what} 的" + which + "在驅動器端停用（配置值 8 = 訊號永遠不作用）。\n\n" +
              "⚠ 這是關掉一道硬體保護。關掉之後：\n" +
              "• 就算極限開關被壓住，驅動器也不會再擋那個方向\n" +
              "• 撞到機械端點時沒有任何東西會停下馬達\n" +
              "• 手冊 §5.10.4：超程訊號停用時不能用極限開關歸原點\n\n" +
              "只有在確定那個方向的行程是安全的、或是要把軸退離開關時才這樣做。\n" +
              "退出來以後請按旁邊的「啟用」把它接回去。") +
          "\n\n讀出來只改這一位，其餘三位原樣寫回（它們是 /S-ON 這些配置，清掉會變成無法激磁）。\n" +
          "⚠ 寫完要按下面的「存檔」才會留住，否則斷電就回舊值。";

        const otBtns = (row, wire, which, objA, objB) => {
          if (typeof stn !== "number") {
            row.appendChild(el("span", "panelhint",
              "這一軸的站號還沒讀到，無法設定 —— 這是寫到「站」不是寫到「軸」。"));
            return;
          }
          const mk = (v, text, cls, on) => {
            const b = cmdButton(text, wire, tag, {
              cls: cls,
              value: v,
              confirm: `站 ${fmtStation(stn)} — 軸 ${half}`,
              station: stn,
              title: "SDO " + ((sub === 1) ? objB : objA) +
                     " — 讀出後只改這一位，其餘位數原樣寫回"
            });
            /*  ⚠ "val", not the default "dir" -- see pci1203.js's askConfirm. */
            b.dataset.confirmkey  = "val";
            b.dataset.confirmbody = otBody(which, on);
            b.dataset.confirmgo   = on ? "重新啟用" : "停用這道極限";
            return b;
          };
          row.appendChild(mk(8, "8 停用（PASS）", "warnbtn", false));
          row.appendChild(mk(0, "0 啟用", "", true));
        };
        otBtns(dp, "pci1203.ax.setOtP", "正極限 P-OT", "250Ah", "2A0Ah");
        lp.appendChild(dp);

        const dn = el("div", "panelrow");
        dn.appendChild(el("span", "panellabel", "驅動器 負極限 N-OT"));
        dn.appendChild(el("span", "panelhint", "Pn50B"));
        const v2 = el("span", "val");
        v2.dataset.k = `pci1203.ax${a}.enc.pn50B`;
        v2.dataset.f = "potB";
        v2.textContent = "---";
        dn.appendChild(v2);
        otBtns(dn, "pci1203.ax.setOtN", "負極限 N-OT", "250Bh", "2A0Bh");
        lp.appendChild(dn);

        lp.appendChild(el("div", "errhow2",
          "ⓘ 上面兩列是「驅動器有沒有在聽極限開關」，跟卡片那一層是分開的兩道。" +
          "要往某個方向走不動時，先看這兩列：只要顯示「啟用中」，就算卡片的極限關掉了，" +
          "驅動器還是會擋。開關現在有沒有被踩住，看馬達 I/O 狀態裡的 LMT+ / LMT− 燈。"));
        lp.appendChild(el("div", "errhow2",
          "⚠ 按「8 停用（PASS）」之後還有兩件事：(1) 手冊 §14.6.2 的 2700h 套用只涵蓋 " +
          "2701h~2704h，不含 Pn50A/Pn50B，所以請把驅動器斷電再上電才會生效；" +
          "(2) 要它斷電後還在，按「馬達 / 編碼器 / 電子齒輪比」那一區的「存檔到驅動器」。" +
          "⚠ 停用是關掉一道保護，退出開關以後請按「0 啟用」接回去。"));
      }

      /*  AI(W906-1203LOGIC-1) 20260917: ⚠⚠ THE PERSISTENCE ANSWER, AND IT IS
          NOT THE ONE THE OPERATOR EXPECTS.
          User: "並且一樣可以斷電重開的" -- i.e. like the drive parameters.
          It cannot be, and the panel had never said so.

          Drive parameters (Pn20E, Pn21D, Pn000, Pn50A) live in the SERVOPACK's
          own non-volatile memory and the 存檔 button (1010h) puts them there.
          EVERYTHING IN THIS PANEL IS THE CARD'S, and the card has no equivalent.
          Measured, three ways, 20260917:
            * the SDK exports 591 Acm_* entries and NONE of them is an
              Acm_Ax*Save / *Flash / *Store for axis configuration
            * nothing in this tree calls Acm_DevLoadConfig, the one API that
              could restore a saved card configuration
            * the monitor issues no Acm_Set*Property at open at all
          So every value on this panel is RAM in the card, and at the next power
          cycle it returns to the card's own default -- including the ones that
          were here before today. That was already true and unsaid, which is
          worse than it being true. */
      lp.appendChild(el("div", "errhow2",
        "⚠⚠ 這一區的設定「斷電就沒了」，跟電子齒輪比那一區不一樣。" +
        "齒輪比/Pn21D/Pn000 是寫進驅動器自己的記憶體，按「存檔」就留得住；" +
        "這一區全部是寫到卡片的 RAM，卡片重新上電就回到它的預設值。" +
        "實測依據：SDK 的 591 個 Acm_* 裡沒有任何一個是軸設定的 Save/Flash；" +
        "這套軟體也沒有呼叫 Acm_DevLoadConfig；監控開卡片時一個屬性都不寫。" +
        "⚠ 這件事今天之前就成立了，只是畫面沒講 —— 包括上面那幾列極限開關。"));

      lp.appendChild(el("div", "errhow2",
        "⚠ 這裡改的是「卡片」這一層。這台的極限開關接在驅動器的 P-OT / N-OT 端子上" +
        "（實測 站1 Pn50A = n.0881、Pn50B = n.8881），所以關掉卡片的極限只會讓" +
        "卡片不再進 ERROR_STOP，驅動器仍然會拒絕往那個方向動 —— 看起來會像「設定沒用」。" +
        "要真的 PASS 掉必須同時改驅動器的 Pn50A / Pn50B（配置值 8 = 該訊號永遠不作用）。"));
      /*  AI(W906-1203SECT-1) 20260917: ⚠ THE SECOND STALE NOTE, found the way
          the first one should have been -- by grepping for MODE13, the name of
          a thing that was deleted, instead of for the container it lived in.
          It sent the operator to "下面 home 那一列的下拉選單", which is gone, to
          pick MODE13, which this machine refuses. Rewritten to say the part
          that is still true and checkable on this machine. */
      lp.appendChild(el("div", "errhow2",
        "ⓘ 這台的原點是獨立的 /Home 開關（實測 Pn511 = n.5432，分配在 SI5），" +
        "不是拿極限當原點，所以關掉極限不會把原點一起關掉。" +
        "⚠ 但如果哪一軸真的是共用同一顆開關，就不要關掉它歸原點要用的那個方向。"));
      secLimit.appendChild(lp);
    }

    /*  AI(W906-1203GEAR-1/2) 20260915: 馬達 / 編碼器 / 電子齒輪比.
        User: "我現在想在介面設置電子齒輪比 / 幫我把相關參數設定介面都寫上去 /
        是要真的可以設定到驅動器的" and "我介面上面也要顯示 馬達型號與編碼器位數".

        ⚠ EVERYTHING IN THIS BLOCK COMES FROM THE SERVOPACK, NOT THE CARD. Every
        other number on this page is the Advantech card's; these are CoE reads
        from the drive over the wire, and the heading says so, because "the card
        says 64" and "the drive says 64" are different claims. */
    {
      const gp = el("div", "subpanel");
      /*  AI(W906-1203SECT-1) 20260917: heading removed for the same reason as
          the limits one -- the section head above it already reads
          馬達與編碼器 MOTOR / ENCODER. */

      const stn  = tags.get(`${tag}.station`);
      const sub  = tags.get(`pci1203.ax${a}.gear.subAxis`);
      const half = (sub === 1) ? "B" : (sub === 0) ? "A" : "?";
      /*  The CoE index this axis's writes go to. Shown because on a two-axis
          SGDXW the SAME parameter lives at two indexes 0x800 apart, and an
          operator cross-checking against SigmaWin+ needs to know which pane
          they are looking at. 0x2701 -> 2701h (A) or 2F01h (B). */
      const gearIdx = (sub === 1) ? "2F01h" : "2701h";

      const infoRow = (label, value, hint) => {
        const r = el("div", "panelrow");
        r.appendChild(el("span", "panellabel", label));
        r.appendChild(value);
        if (hint) r.appendChild(el("span", "panelhint", hint));
        gp.appendChild(r);
      };

      /*  ⚠ THE AXIS'S OWN driveModel, NOT stationModel(). stationModel() looks a
          station number up across BOTH rings, and this machine has a station 1
          on each -- so it answered
              "ring 0: SGDXW-xxxxA0x SERVOPACK  ·or·  ring 1: ECAT-2515 Junction"
          which is correct as a lookup and useless as an identity: an axis is on
          exactly one of them, and naming a passive junction next to a servo
          invites the reader to wonder which one they are configuring. The
          per-axis tag is unambiguous because it came from THIS axis's slave. */
      const drv = el("span", "val");
      drv.dataset.k = `pci1203.ax${a}.enc.model`;
      drv.textContent = "---";
      infoRow("驅動器型號", drv,
              `站 ${stn === undefined ? "?" : fmtStation(stn)}　軸 ${half}　(CoE 1008h — 實際機種)`);

      /*  The card's scan name, kept beside it rather than instead of it. It is
          the ESI/EEPROM FAMILY ("SGDXW-xxxxA0x", with the x's literally there),
          so it cannot identify a unit -- but it IS what the card believes, and
          a mismatch between the two would be worth seeing. */
      const drvEsi = el("span", "val");
      drvEsi.dataset.k = `pci1203.ax${a}.driveModel`;
      drvEsi.textContent = "---";
      infoRow("卡片掃到的名稱", drvEsi, "ESI 系列名，不是實際機種");

      /*  ⚠ 馬達型號 IS NOT AVAILABLE, and this row says so rather than sitting
          empty. Measured 20260915 by sweeping 1000h-10FFh, 2000h-20FFh,
          2200h-22FFh, 2700h-27FFh and 6000h-60FFh on station 14: the only
          readable strings on the whole drive are 1008h (the SERVOPACK) and
          100Ah (firmware). An empty field would read as "not fetched yet";
          this is a measured negative and the operator should stop looking. */
      const mt = el("span", "val");
      mt.textContent = "驅動器未提供";
      infoRow("馬達型號", mt, "請看馬達銘牌或 SigmaWin+ — 說明見下方");

      const rt = el("span", "val");
      rt.dataset.k = `pci1203.ax${a}.enc.ratedTorque`;
      rt.dataset.f = "torque";
      rt.textContent = "---";
      infoRow("馬達額定轉矩", rt, "6076h — 這是驅動器唯一給得出的馬達識別");

      const enc = el("span", "val");
      enc.dataset.k = `pci1203.ax${a}.enc.pn21D`;
      enc.dataset.f = "encBits";
      enc.textContent = "---";
      infoRow("編碼器位數", enc, "Pn21D (221Dh)");

      const use = el("span", "val");
      use.dataset.k = `pci1203.ax${a}.enc.pn002`;
      use.dataset.f = "encUse";
      use.textContent = "---";
      infoRow("編碼器使用方式", use, "Pn002 (2002h) n.□X□□");

      /*  AI(W906-1203DIR-2) 20260916: 馬達正方向 -- CW / CCW, on the DRIVE.
          MOVED HERE from inside the 回HOME設定 collapsible, where it had sat
          since 20260915 and where the operator could not find it. See the
          pointer left behind there for why duplicating it would be worse.

          Manual on this machine, SIEPC71081205 p.149 s5.4 Motor Direction
          Setting, quoted:
              "You can change the direction of servomotor rotation without
               changing the polarity of the speed or position reference by
               setting Pn000 to n.□□□X (Rotation Direction Selection)."
              Pn000 (A:2000h, B:2800h)
                  n.□□□0  Use CCW as the forward direction (default setting)
                  n.□□□1  Use CW as the forward direction (Reverse Rotation Mode)

          ⚠ THE LIMIT SWITCHES FOLLOW THE REFERENCE, NOT THE MOTOR. The same
          table keeps P-OT on the FORWARD REFERENCE in both rows -- so after the
          flip, P-OT still prohibits "forward", but forward now points the other
          way physically. The switch that used to stop the + end now stops the
          − end. That is in the confirmation, because it is the consequence
          nobody predicts. */
      {
        const dirRow = el("div", "panelrow");
        dirRow.appendChild(el("span", "panellabel", "馬達正方向 CW/CCW"));
        dirRow.appendChild(el("span", "panelhint", "驅動器目前"));
        const dirCur = el("span", "val");
        dirCur.dataset.k = `pci1203.ax${a}.home.driveDir`;
        dirCur.dataset.f = "rotDir";
        dirCur.textContent = "---";
        dirRow.appendChild(dirCur);
        if (typeof stn === "number") {
          const dirBody =
            "即將把 {what} 的馬達正方向對調（Pn000 n.□□□X，手冊 p.149 §5.4）。\n\n" +
            "• 這一軸之後的每一次 JOG、MOVE、歸原點都會往反方向跑\n" +
            "• ⚠ 極限開關跟著「指令方向」走，不是跟著馬達：P-OT 仍然擋「正向指令」，" +
            "但正向現在是反過來的那一邊 —— 原本擋 ＋ 端的開關會變成擋 − 端\n" +
            "• 已經教好的座標與原點全部要重新確認\n" +
            "• 驅動器必須斷電再上電才會生效（2700h 套用不涵蓋 Pn000）";
          const mk = (v, text, cls) => {
            const b = cmdButton(text, "pci1203.ax.setDriveDir", tag, {
              cls: cls,
              value: v,
              confirm: `站 ${fmtStation(stn)} — 軸 ${half}`,
              station: stn,
              title: "SDO " + ((sub === 1) ? "2800h" : "2000h") +
                     " Pn000 n.□□□X — 讀出後只改這一位，其餘位數原樣寫回"
            });
            b.dataset.confirmbody = dirBody;
            b.dataset.confirmgo   = "對調馬達方向";
            return b;
          };
          dirRow.appendChild(mk(0, "0 CCW 為正向（預設）", ""));
          dirRow.appendChild(mk(1, "1 CW 為正向（反轉）", "warnbtn"));
        } else {
          dirRow.appendChild(el("span", "panelhint",
            "這一軸的站號還沒讀到，無法設定 —— 這是寫到「站」不是寫到「軸」。"));
        }
        gp.appendChild(dirRow);

        gp.appendChild(el("div", "errhow2",
          "ⓘ 這個參數在做什麼：手冊 p.149 §5.4 原話是「change the direction of " +
          "servomotor rotation without changing the polarity of the speed or " +
          "position reference」—— 也就是你送的數字正負號不用改，馬達自己轉的方向反過來。" +
          "預設是「從負載端看過去，逆時針(CCW) 為正向」。"));
        gp.appendChild(el("div", "errhow2",
          "⚠ Pn000 同一個暫存器裡還有別的位數（n.X□□□ 是「編碼器沒接時的啟動選擇」）。" +
          "所以這裡是「讀回來、只改方向那一位、其餘原樣寫回」，" +
          "沒讀到 Pn000 時會直接拒絕，不會拿 0/1 蓋掉整個暫存器。實測這台九個站兩個軸全是 n.0000。" +
          "⚠ 它是 After restart —— 改完一定要把驅動器斷電再上電，2700h 的套用不涵蓋它。"));
      }

      /*  AI(W906-1203ENC-1) 20260916: Pn21D, SETTABLE.
          User: "那你可以把此功能寫在介面 讓我設? 並要說明此參數功能".

          It came up because the electronic gear is bounded -- the ratio must
          stay ≤ 64000, which from 1048576/200 is 11.8x -- and the operator
          needed a coarser step than that. Pn21D reaches further, and the panel
          has to say what it costs, because the two look interchangeable and
          are not:
              gear   -- each command unit covers more encoder counts; the drive
                        still servos on all 26 bits. Nothing is lost.
              Pn21D  -- the drive DISCARDS encoder bits. The step gets coarser
                        AND the achievable accuracy gets worse.

          ⚠ Each button carries a COMPLETE choice (enable + selection), because
          the write is one read-modify-write over two digits. "Off" still has to
          name a selection digit, so it reuses whatever the drive currently
          holds rather than silently rewriting that half too. */
      {
        const raw = tags.get(`pci1203.ax${a}.enc.pn21D`);
        const curOn  = (typeof raw === "number") ? (Math.round(raw) & 0xF) : null;
        const curSel = (typeof raw === "number") ? ((Math.round(raw) >> 4) & 0xF) : null;

        const er = el("div", "panelrow");
        er.appendChild(el("span", "panellabel", "編碼器位數 相容模式"));
        if (typeof stn === "number" && typeof raw === "number") {
          const body = (on, sel, bits, counts) =>
            (on
              ? `即將讓 {what} 的驅動器「把馬達當成 ${bits} 位編碼器」使用` +
                `（一圈 ${counts.toLocaleString()} counts）。\n\n` +
                `⚠ 這不是換算而已 —— 驅動器會真的丟掉編碼器位元，` +
                `定位精度會跟著變差。能用電子齒輪比解決就不要用這個。\n`
              : `即將關閉 {what} 的相容模式，讓驅動器改用「馬達自己規格」的解析度。\n\n` +
                `⚠ 馬達實際位數要看銘牌 —— 驅動器不公開馬達型號` +
                `（6404h 不存在），所以這一步之後一圈幾個 counts 我算不出來。\n`) +
            `\n這一軸的每一個位置、速度、已教座標都會跟著改變意義，` +
            `原點與教點全部要重新確認。\n` +
            `⚠ 手冊 p.193 標注「After restart」—— 一定要把驅動器斷電再上電才生效。\n` +
            `⚠ 手冊限制：全閉環時不能用（這台 Pn22A = 0，符合）；` +
            `所選位數不可以高於馬達實際位數 —— 這一項軟體無法替你檢查。`;

          const mk = (on, sel, label, cls) => {
            const b = cmdButton(label, "pci1203.ax.setEncCompat", tag, {
              cls: cls,
              confirm: `站 ${fmtStation(stn)} — 軸 ${half}`,
              station: stn,
              title: "SDO " + ((sub === 1) ? "2A1Dh" : "221Dh") +
                     " Pn21D n.□□□X + n.□□X□ — 讀出後只改這兩位，其餘原樣寫回"
            });
            b.dataset.valueraw = `on=${on};sel=${sel}`;
            b.dataset.confirmbody = body(on, sel, ENC_BITS[sel] || "?",
                                         ENC_COUNTS[sel] || 0);
            b.dataset.confirmgo = on ? `當成 ${ENC_BITS[sel]} 位` : "關閉相容模式";
            return b;
          };
          //  Off first: it is the default and the one that loses nothing.
          er.appendChild(mk(0, (curSel === null ? 8 : curSel), "關閉（用馬達原本解析度）", ""));
          for (const sel of [4, 6, 8, 10]) {
            er.appendChild(mk(1, sel, `開啟 ${ENC_BITS[sel]} 位`, "warnbtn"));
          }
        } else {
          er.appendChild(el("span", "panelhint",
            "站號或 Pn21D 還沒讀到 —— 這是讀-改-寫，沒讀到就不會寫。"));
        }
        gp.appendChild(er);

        gp.appendChild(el("div", "errhow2",
          "ⓘ 這個參數在做什麼：驅動器平常用「馬達自己的」編碼器解析度。" +
          "打開相容模式之後，它會改用你選的位數去跑 —— " +
          "手冊 p.193 原話是「the servomotor can be operated with an encoder " +
          "resolution that differs from the servomotor specifications」。" +
          "位數越低，馬達一圈的 counts 越少，所以同一組齒輪比下「一格」走得越遠。"));
        gp.appendChild(el("div", "errhow2",
          "⚠ 它跟電子齒輪比不能互換：齒輪比只是把一格做大，驅動器仍然用全部位元在定位；" +
          "Pn21D 是真的把位元丟掉，一格變大的同時定位精度也變差。" +
          "所以順序是「先調齒輪比，不夠用才動這裡」。齒輪比的極限是比值 ≤ 64000。"));
        gp.appendChild(el("div", "errhow2",
          "⚠⚠ 現在讀到的是 " +
          ((typeof raw === "number")
            ? `n.${[(Math.round(raw)>>12)&0xF,(Math.round(raw)>>8)&0xF,curSel,curOn].map(d=>d.toString(16).toUpperCase()).join("")}` +
              `：相容模式 ${curOn === 1 ? "開啟" : "關閉"}、選擇位數 ${ENC_BITS[curSel] || "?"}。` +
              (curOn !== 1
                ? "⚠ 相容模式是關的，所以「選擇位數」那一位目前沒有作用 —— " +
                  "讀到 8 不代表這台是 24 位，它實際上用的是馬達的位數。"
                : "")
            : "---") ));
      }

      /*  The ratio, computed on the page from the two published halves. It is
          the number the operator actually reasons about, and neither half alone
          means anything. */
      /*  ⚠ data-num, NOT data-k -- and that is a bug fix, not a style choice.
          This cell first carried data-k="...gear.posNum" so refresh() would
          find it, which meant TWO nodes on the page answered
          querySelector('[data-k="pci1203.ax3.gear.posNum"]') and the ratio cell
          won because it comes first in the DOM. Anything reaching for the
          posNum ROW got the ratio SPAN instead -- whose parent holds no input,
          so the lookup died. Measured: a probe driving the 分子 set button threw
          and sent nothing, while the 分母 button beside it worked.
          A composed value is not "the posNum tag", so it does not claim that
          key; refresh() has its own branch for data-num + data-den.
          No data-f either: naming a formatter that is never consulted would
          send the next reader looking for a "gearRatio" case that does not
          exist. */
      const ratio = el("span", "val");
      ratio.dataset.num = `pci1203.ax${a}.gear.posNum`;
      ratio.dataset.den = `pci1203.ax${a}.gear.posDen`;
      ratio.textContent = "---";
      infoRow("目前齒輪比", ratio, `分子 ÷ 分母　(${gearIdx})`);

      /*  Eight settable values. `pos` is the electronic gear proper; the other
          three are the user units the manual groups with it (s14.6.4-14.6.6)
          and which the user asked for as 相關參數. */
      const gearRow = (label, key, cmd, hint) => {
        const row = el("div", "panelrow");
        row.appendChild(el("span", "panellabel", label));
        row.appendChild(el("span", "panelhint", "驅動器目前"));
        const cur = el("span", "val");
        cur.dataset.k = `pci1203.ax${a}.gear.${key}`;
        cur.dataset.f = "int";
        cur.textContent = "---";
        row.appendChild(cur);
        /*  AI(W906-1203ONE-1) 20260916: ⚠ EMPTY, not 0. The one-press button
            treats a blank box as "leave this parameter alone", so a default of
            0 would make every press try to set all eight -- and 0 is out of
            range for all of them, so the operator would get a refusal naming a
            field they never touched. The per-field set button is unaffected:
            it already refuses a box that is not a number. */
        row.appendChild(numBox(`ax${a}.${key}`, "", "9em"));
        /*  AI(W906-1203TRIM-1) 20260917: ⚠ THE PER-FIELD "set" BUTTON WAS HERE
            AND IS GONE -- eight of them. User: "沒用的按鈕刪刪掉 / 介面可以精簡
            就精簡", and before that "我在設定電子齒輪比時 都要按set 我可以不要案
            set嗎? / 我也不想案套用 / 我想要一個按鈕就可以設定全部了".

            ⚠ THEY WERE NOT MERELY REDUNDANT, THEY WERE THE WORSE OPTION. A
            per-field set writes 2701h and stops: it does not write 2700h, so
            the drive keeps using the OLD ratio, and it does not write 1010h, so
            the value is gone at the next power cycle. Both of those have
            already been reported as "你是不是根本沒設進去?" on this machine.
            一鍵設定 below does all three in order and names the step if one
            fails, so every press of a per-field set was a chance to land in the
            half-applied state the one-press button exists to prevent.
            ⓘ The boxes stay -- they are what 一鍵設定 collects. A blank box
            still means "leave this one alone", not zero.
            ⓘ kCmdAxSetGear and its eight wire names are untouched in the C++;
            this removes the page's buttons, not the capability. */
        if (hint) row.appendChild(el("span", "panelhint", hint));
        gp.appendChild(row);
      };

      gearRow("電子齒輪比 分子", "posNum", "pci1203.ax.setGear.posNum", "Pn20E　1..1073741824");
      gearRow("電子齒輪比 分母", "posDen", "pci1203.ax.setGear.posDen", "Pn210　1..1073741824");

      /*  AI(W906-1203WHY-1) 20260915: THE RANGE THAT ACTUALLY BINDS, computed
          from the partner value.
          "1..1073741824" is true of each half on its own and is not the limit
          anybody hits. The real constraint is the PAIR: 0.001 <= n/d <= 64000.
          A user with numerator 10485760 typed denominator 162 -- both numbers
          individually legal, the pair 64726.9, refused. Printing the per-field
          range while the pair is what refuses is a hint that answers the
          question nobody asked. This row says what the OTHER field may be,
          given what the drive currently holds. */
      {
        const rr = el("div", "panelrow");
        rr.appendChild(el("span", "panellabel", "分母可填範圍"));
        const rv = el("span", "val");
        rv.dataset.num = `pci1203.ax${a}.gear.posNum`;
        rv.dataset.den = `pci1203.ax${a}.gear.posDen`;
        rv.dataset.f   = "denRange";
        rv.textContent = "---";
        rr.appendChild(rv);
        rr.appendChild(el("span", "panelhint",
          "真正會擋你的是「比值」0.001 ≤ 分子÷分母 ≤ 64000，不是單一個數字"));
        gp.appendChild(rr);
      }
      gearRow("速度單位 分子",   "velNum", "pci1203.ax.setGear.velNum", "2702h:1");
      gearRow("速度單位 分母",   "velDen", "pci1203.ax.setGear.velDen", "2702h:2");
      gearRow("加速度單位 分子", "accNum", "pci1203.ax.setGear.accNum", "2703h:1");
      gearRow("加速度單位 分母", "accDen", "pci1203.ax.setGear.accDen", "2703h:2");
      gearRow("轉矩單位 分子",   "trqNum", "pci1203.ax.setGear.trqNum", "2704h:1");
      gearRow("轉矩單位 分母",   "trqDen", "pci1203.ax.setGear.trqDen", "2704h:2");

      /*  AI(W906-1203ONE-1) 20260916: ONE BUTTON FOR THE WHOLE THING.
          User: "我在設定電子齒輪比時 都要按set 我可以不要案set嗎? / 我也不想案套用
          / 我想要一個按鈕就可以設定全部了".

          The three steps were never independent -- write, 2700h apply, 1010h
          store -- so splitting them across ten buttons made the operator
          responsible for remembering a sequence the manual imposes, and
          forgetting the last one silently loses everything at the next power
          cycle. That is the defect this campaign already paid for once.

          ⚠ It gathers only the boxes that were FILLED IN (data-collect, and
          see askConfirm): blank means "leave this one alone", not zero.
          ⚠ The per-field set buttons above stay. They are the granular path --
          write one value without applying it -- and removing a working control
          because a better one appeared costs someone their workflow. */
      {
        const one = el("div", "panelrow");
        one.appendChild(el("span", "panellabel", "一鍵設定"));
        if (typeof stn === "number") {
          const roles = ["posNum","posDen","velNum","velDen",
                         "accNum","accDen","trqNum","trqDen"]
                        .map(k => `ax${a}.${k}`).join(",");
          const ob = cmdButton("寫入 ＋ 套用 ＋ 存檔（一次做完）",
            "pci1203.ax.gearSetAll", tag, {
              cls: "warnbtn",
              confirm: `站 ${fmtStation(stn)} — ${tags.get(`pci1203.ax${a}.driveModel`) || "SERVOPACK"}　軸 ${half}`,
              station: stn,
              title: "上面有填的欄位全部寫入 → SDO " +
                     ((sub === 1) ? "2F00h" : "2700h") + " = 1 套用 → SDO 1010h:1 存檔"
            });
          ob.dataset.collect = roles;
          ob.dataset.confirmbody =
            "即將對 {what} 一次做完三件事：\n\n" +
            "① 把上面「有填的」欄位寫進驅動器（空白的不動）\n" +
            "② 套用（2700h＝1），讓新的換算立刻生效\n" +
            "③ 存檔（1010h:1），斷電之後才不會掉回舊值\n\n" +
            "⚠ 這一軸的位置／速度／加速度換算方式會在這一刻改變，" +
            "之前教好的座標與原點都要重新確認。\n" +
            "⚠ 驅動器必須在伺服 OFF，否則②③會被拒絕 —— 這裡會先擋下來，" +
            "不會讓你寫進去卻沒生效。";
          ob.dataset.confirmgo = "一次做完";
          one.appendChild(ob);
        } else {
          one.appendChild(el("span", "panelhint",
            "這一軸的站號還沒讀到 —— 這些是寫到「站」不是寫到「軸」。"));
        }
        gp.appendChild(one);
        gp.appendChild(el("div", "errhow2",
          "ⓘ 上面每一列的 set 仍然可以用，那是「只改一個值、先不要生效」的做法。" +
          "平常要改齒輪比，填好數字按這一顆就好 —— 它會把三個步驟一起做完，" +
          "而少做第三步（存檔）正是「設定完重開電源又變回去」的原因。"));
      }

      /*  AI(W906-1203TRIM-1) 20260917: ⚠ THE STANDALONE "套用" BUTTON WAS HERE
          AND IS GONE. User: "介面可以精簡就精簡", and before that "我也不想案
          套用".

          2700h (User Parameter Configuration) covers 2701h..2704h and NOTHING
          ELSE -- s14.6.2 lists exactly those four. Every other drive parameter
          this page writes is "When Enabled: After restart" with no software
          apply at all: Pn000 (direction), Pn21D (encoder compatibility), Pn50A
          /Pn50B (overtravel). So this button could only ever have been about
          the gear, and 一鍵設定 above already issues it as its second step, in
          order, naming the step if it fails.
          Left on its own it was a button whose only correct use was recovering
          from a sequence the page no longer asks anyone to perform by hand.

          ⓘ 存檔 below is NOT removed with it, and the difference matters: 1010h
          is needed after the OT / 馬達方向 / Pn21D / 驅動器歸原點 writes too,
          none of which 一鍵設定 touches. It is the one persistence step that
          still has to be reachable on its own.

          AI(W906-1203STORE-1) 20260915: THE THIRD BUTTON, AND THE ONE THAT WAS
          MISSING. Without it every value written here is lost at the next power
          cycle -- and the read-back follows the RAM value the whole time, so
          nothing on screen says so. */
      const sr = el("div", "panelrow");
      sr.appendChild(el("span", "panellabel", "存檔"));
      if (typeof stn === "number") {
        const sb = cmdButton("存入驅動器（斷電也不會掉）",
          "pci1203.ax.storeParams", tag, {
            cls: "warnbtn",
            confirm: `站 ${fmtStation(stn)} — ${tags.get(`pci1203.ax${a}.driveModel`) || "SERVOPACK"}`,
            station: stn,
            title: "SDO 1010h:1 = \"save\" — Store Parameters (§14.3.5)"
          });
        sb.dataset.confirmbody =
          "即將把 {what} 目前的參數寫進非揮發記憶體。\n\n" +
          "• 這會存下這一站「所有」的參數，不只是電子齒輪比\n" +
          "• 這一輪裡改錯的東西，也會在這一刻變成永久的\n" +
          "• 驅動器必須在 Switch ON Disabled（伺服 OFF），否則會被拒絕\n" +
          "• 存完之後要把驅動器斷電再上電";
        sb.dataset.confirmgo = "寫入非揮發記憶體";
        sr.appendChild(sb);
      } else {
        sr.appendChild(el("span", "panelhint", "這一軸的站號還沒讀到，無法存檔。"));
      }
      gp.appendChild(sr);

      /*  AI(W906-1203STORE-2) 20260915: ⚠ SAY WHETHER THE VALUE ON SCREEN IS
          THE ONE THE DRIVE IS USING.

          User set 1048576/400, saw it persist, and reported it "not enabled".
          Measured on station 0x29 axis A: 2701h and Pn20E/Pn210 BOTH read
          1048576/400, 603Fh is clear (so the drive did not reject it), 2700h is
          idle -- and 6041h reads 0x1637, i.e. Operation Enabled. Nothing was
          wrong with the value. The drive was simply still RUNNING on the
          scaling it loaded at its last start-up.

          That is the hardest state this panel can be in: the read-back is the
          NEW number, the drive is using the OLD one, and every field looks
          correct. The manual's own preconditions are what resolve it -- s14.6.2
          step 1 is "Change the SERVOPACK to the Switch ON Disabled state" and
          Pn20E/Pn210 are "When Enabled: After restart" -- so this row reports
          the servo state and what it means for 套用, rather than leaving the
          operator to discover that 套用 refuses. */
      {
        const ar2 = el("div", "panelrow");
        ar2.appendChild(el("span", "panellabel", "能不能套用"));
        const st = el("span", "val");
        st.dataset.k = `pci1203.ax${a}.svOn`;
        st.dataset.f = "canApply";
        st.textContent = "---";
        ar2.appendChild(st);
        gp.appendChild(ar2);
      }

      gp.appendChild(el("div", "errhow2",
        "⚠⚠ 改一個驅動器參數要「三步」，少任何一步看起來都像「設定沒用」，" +
        "而且畫面上的讀回值從第一步之後就一直是新的，不會提醒你：\n" +
        "① set —— 只改到 RAM，讀回值馬上變（所以看起來好像成功了）\n" +
        "② 套用 2700h —— 驅動器才開始「使用」這個新值；不按就要重新上電\n" +
        "③ 存入驅動器 1010h —— 才會活過斷電；不按的話下次上電就變回舊值"));
      gp.appendChild(el("div", "errhow2",
        "⚠ 這一段之前寫錯，在此更正：原本寫「分子/分母寫進去的當下就存進 EEPROM」，" +
        "根據是手冊 p.601 那一欄的「Saving to EEPROM: Yes」。那一欄的意思是" +
        "「這個物件可以被存起來」，不是「已經存起來」。手冊 p.141 講得很清楚：" +
        "「使用 EtherCAT 通訊物件時，必須把 SERVOPACK 參數寫入非揮發記憶體；" +
        "要寫入非揮發記憶體，請設定 Store Parameters (1010h)」。" +
        "所以只做 ①② 的話，斷電後讀回來就是預設值。"));
      gp.appendChild(el("div", "errhow2",
        "ⓘ 範圍不只是單一個數字：位置是 0.001 ≤ 分子/分母 ≤ 64000，超出去驅動器是" +
        "「發警報」不是「拒絕寫入」，所以這邊會先擋下來。"));
      gp.appendChild(el("div", "errhow2",
        "ⓘ 馬達型號為什麼是「驅動器未提供」：20260915 實測掃過 1000h–10FFh、" +
        "2000h–20FFh、2200h–22FFh、2700h–27FFh、6000h–60FFh，整台驅動器只有兩個" +
        "字串讀得回來 —— 1008h（驅動器型號）和 100Ah（韌體 0009.0000）。" +
        "編碼器解析度也沒有以數字形式公開（整個範圍找不到任何 2^16~2^28 的值）。" +
        "所以馬達型號與實際位數要看馬達銘牌，或用 SigmaWin+ 連驅動器讀。"));
      gp.appendChild(el("div", "errhow2",
        "⚠ 編碼器位數要看 Pn21D 的「相容模式」開關：n.□□□X = 0（預設，本機九站全部" +
        "都是）代表相容模式關閉，這時位數是「馬達本身的規格」，不是 Pn21D 裡那個數字" +
        "（手冊 p.178：要從馬達型號去查）。只有 n.□□□X = 1 時，n.□□X□ 的 4/6/8/A " +
        "＝ 20/22/24/26 位才是驅動器實際在用的位數。"));
      secMotor.appendChild(gp);
    }

    /*  AI(W906-1203CTL-44) 20260911: 運動模式 Continue, from the Utility's
        運動測試 block -- its other radio setting.
        AI(W906-1203ALM-10) 20260914: ⚠ THIS COMMENT USED TO CONTRAST THIS ROW
        WITH "the JOG row above", which no longer exists. It said the two moved
        at different speeds because Acm_AxJog obeys CFG_AxJogVelHigh -- true of
        the API and irrelevant here, because Acm_AxJog does nothing at all on
        this card (0x8000510B "Invalid axis states"; jog is a DI-assigned
        hardware function, see the arrow row). The real distinction now is the
        one the Utility's own radio makes:
            負向/正向 above   P to P   Acm_AxMoveRel, moves 距離 and STOPS
            連續 here         Continue Acm_AxMoveVel, runs until STOP
        Both obey the PTP speed family (PAR_AxVelHigh).
        ⚠ 連續 does not stop by itself -- STOP above is the only way out. */
    const r3b = el("div", "panelrow");
    r3b.appendChild(el("span", "panellabel", "連續 Continue"));
    r3b.appendChild(cmdButton("◀ 連續 −", "pci1203.ax.moveVel", tag,
      { value: -1, title: "Acm_AxMoveVel(ax, DIRECTION_NEG) — 跑 PAR_AxVelHigh，不會自己停" }));
    r3b.appendChild(cmdButton("連續 + ▶", "pci1203.ax.moveVel", tag,
      { value: 1, title: "Acm_AxMoveVel(ax, DIRECTION_POS) — 跑 PAR_AxVelHigh，不會自己停" }));
    r3b.appendChild(el("span", "panelhint", "用上面的 STOP 停止"));
    secRun.appendChild(r3b);

    /*  AI(W906-1203CTL-44) 20260911: 疊加運動 -- Acm_AxMoveImpose(ax, 疊加距離,
        疊加速度), Examples/Windows/C#/MoveImpose/Form1.cs:180. It imposes a new
        target and speed on a move ALREADY RUNNING, which is why it sits under
        the move rows rather than beside them. */
    const r3c = el("div", "panelrow");
    r3c.appendChild(el("span", "panellabel", "疊加運動"));
    r3c.appendChild(el("span", "panelhint", "疊加距離"));
    r3c.appendChild(numBox(`ax${a}.impPos`, 0, "7em"));
    r3c.appendChild(el("span", "panelhint", "疊加速度"));
    r3c.appendChild(numBox(`ax${a}.impVel`, 0, "7em"));
    const ib = cmdButton("疊加運動", "pci1203.ax.moveImpose", tag,
      { title: "Acm_AxMoveImpose(ax, Position, NewVel) — 疊加在「正在進行中」的運動上" });
    ib.dataset.imposefrom = `ax${a}`;
    r3c.appendChild(ib);
    secRun.appendChild(r3c);

    /* --- HOME. Two boxes and no defaults anywhere, deliberately.
       Acm_AxHome(HAND, HomeMode, DirMode) takes two operator choices; the C++
       side REFUSES a home whose mode is unspecified rather than assuming 0,
       because mode 0 is MODE1_ABS and on an axis wired for a limit-switch home
       that commands a move toward a switch that will not stop it.

       AI(W906-1203LIM-2) 20260915: ⚠ THE MODE WAS A NUMBER BOX STARTING AT 0,
       AND 0 IS THE ONE VALUE THAT IS WRONG ON THIS MACHINE. Its comment argued
       that "a number box must start somewhere" -- true, and it started on
       MODE1_ABS, which hunts an ORG signal these axes do not have. Worse, the
       mode NUMBERS ARE OFF BY ONE FROM THE MODE NAMES (MODE13 is 12), so an
       operator typing the number they read in a manual selects the neighbouring
       mode. A picker removes both: it names every mode, groups the Lmt series
       that a shared home/limit switch needs, and its placeholder is -1 so
       "nobody chose" is still refused by ValidateHome rather than defaulted. */
    /*  AI(W906-1203TRIM-1) 20260917: ⚠⚠ THE 25-OPTION MODE PICKER AND THE
        GENERIC "HOME" BUTTON WERE HERE AND ARE GONE. User: "沒用的按鈕刪刪掉 /
        沒用的選項刪刪掉 / 介面可以精簡就精簡", and earlier and more specifically
        "我歸原點方式只需要 128 和 124 其他不用".

        This is a DELETION OF WORKING CODE, so the evidence is written down
        rather than the intent:
          * The sixteen "typical" card modes in that list all answer
            0x8000510F "Invalid home mode" on this machine's axes -- measured.
            Homing here belongs to the DRIVE (6098h exists, 6502h bit5 hm = 1,
            and PAR_AxHomeMode / PAR_AxCiA402HomeMode / CFG_AxHomeMode all
            answer 0x8000000A). So sixteen of the twenty-five options were
            offering the operator a refusal.
          * Of the CiA402 ones, this SGDXW implements 0,1,2,7-14,24,28,33,34,35
            and 37 (manual p.560-561); 3,4,5,6 are absent. So the list was also
            offering methods the drive does not have.
          * The two that this machine actually homes with are 124 and 128, and
            they already have their own one-press buttons below -- which cannot
            get the direction wrong, because the direction is part of the method
            number.
        ⓘ Nothing is lost that the machine could do: kCmdAxHome still takes any
        mode on the wire, and the two buttons below cover the two that work.
        The picker, `selectBox`, `kHomeModes` and the data-homefrom path are all
        still present in the code for whoever needs a different machine. */

    /*  AI(W906-1203HOMEBTN-1) 20260917: ONE-PRESS HOMING, the two methods this
        machine actually uses.
        User: "我歸原點方式只需要 128 和 124 其他不用 並且往正方向 或往負方向
               我希望可以做成按鈕直接做動歸HOME".

        ⚠ THE DIRECTION IS ALREADY IN THE METHOD NUMBER, and that is why there
        are two buttons rather than four. Manual p.560-561, 6098h:
            24  /Home switch, STARTING IN THE FORWARD direction, no index
            28  /Home switch, STARTING IN THE REVERSE direction, no index
        They are the same method mirrored. Offering "124 往負方向" would be
        offering a combination the drive does not have, and the dir argument
        would not rescue it -- so each button sends the dir that MATCHES its
        method instead of letting the two disagree.

        ⚠ Card values are 100 + the DS402 method (CiA402_MODEn = 100 + n,
        AdvMotDrv.h:879-916). 124 and 128 here are already the card values.

        ⚠ MEASURED WHY THE OTHER MODES ARE GONE FROM THIS ROW: the sixteen
        "typical" card modes all answer 0x8000510F Invalid home mode on this
        axis, because homing here belongs to the DRIVE (6098h exists, 6502h
        bit5 hm = 1, and PAR_AxHomeMode / PAR_AxCiA402HomeMode / CFG_AxHomeMode
        all answer 0x8000000A PropertyID is not supported). The full picker
        below still offers everything -- this row is the shortcut for the two
        that work.

        ⓘ No confirmation dialog, matching JOG and MOVE: homing is motion, and
        this page confirms PARAMETER writes rather than movement. */
    {
      const hr = el("div", "panelrow");
      hr.appendChild(el("span", "panellabel", "一鍵歸原點"));
      const mk = (mode, dir, text, hint) => {
        const b = cmdButton(text, "pci1203.ax.home", tag, {
          title: `Acm_AxHome(ax, ${mode}, ${dir > 0 ? "正" : "負"}) — ` +
                 `CiA402 = DS402 方法 ${mode - 100}（手冊 p.560-561）`
        });
        b.dataset.rawvalue = `mode=${mode};dir=${dir}`;
        return b;
      };
      /*  AI(W906-1203HOMEDIR-1) 20260918: ⚠⚠ THE TWO LABELS ARE SWAPPED
          RELATIVE TO THE MANUAL, ON THE OPERATOR'S OBSERVATION OF THE MACHINE.
          User: "圖片上 按鈕名稱幫我對調".

          ⚠ ONLY THE TEXT MOVED. mode=124 still sends dir=+1 and mode=128 still
          sends dir=−1; nothing about what the drive is asked to do changed.
          Writing that down because the tempting "fix" is to swap the MODE
          numbers instead, and that would be wrong twice over: 24 and 28 are
          different methods (p.561 -- 24 is "same as method 8", 28 is "same as
          method 12"), and the dir argument would then disagree with the method
          it is paired with.

          WHY THE MANUAL'S WORD AND THE OPERATOR'S WORD DIFFER, and why the
          operator's is the one on the button: DS402 "forward" is the drive's
          own idea of positive, which Pn000 n.□□□X can invert and which the
          mechanism's mounting can invert again. The manual is describing the
          motor shaft; the operator is describing the carriage they can see. The
          label on a machine button has to name what the machine does.
          ⓘ So this is NOT evidence that anything is mis-set -- it is the
          ordinary case of the drive's positive not being the rig's positive.
          If Pn000 is ever changed on this axis (馬達與編碼器 區), these two
          labels have to be re-checked, because that is exactly the setting that
          flips which end the carriage runs to. */
      hr.appendChild(mk(124,  1, "◀ 往負方向歸原點", ""));
      hr.appendChild(mk(128, -1, "▶ 往正方向歸原點", ""));
      hr.appendChild(el("span", "panelhint",
        "124 / 128 = /Home 開關，不需要 Z 相；名稱依實機做動方向，" +
        "與手冊的 forward/reverse 相反是正常的（Pn000 與機構安裝都會反向）"));
      secHome.appendChild(hr);

      /*  AI(W906-1203SECT-1) 20260917: ⚠ THESE TWO NOTES WERE STALE THE MOMENT
          THE PICKER WAS REMOVED, and the staleness was the dangerous kind --
          they told the operator to "選上面第一組（Lmt 系列）" and recommended
          "12 MODE13_LMT_SEARCH_REFIND" by name. There is no picker any more,
          and those sixteen card modes are exactly the ones this machine answers
          0x8000510F to. A note that names a control that does not exist reads
          as "the page is broken"; a note that recommends a mode that cannot run
          is worse, because someone will go looking for a way to select it.
          Rewritten for what this machine actually has. */
      /*  AI(W906-1203SECT-1) 20260917: FIVE STACKED PARAGRAPHS BECAME TWO.
          User: "介面可以精簡就精簡". Each of the five was written in response to
          a separate question on a separate day and each was correct; stacked,
          they were a wall of prose above the only two buttons in the section,
          and the one sentence an operator needs before pressing -- "clear the
          error and get off the limit first" -- was the second of five.
          ⚠ What is DROPPED rather than merged is named here so it is a decision
          and not an erosion: the DS402 method 1/2 note (there is no button for
          them, so it was answering a question the page cannot act on) and the
          Pn00D detail (it says the overtravel alarm is NOT enabled here, i.e.
          it describes a thing that is not happening). Both remain in the git
          history and in Pci1203Gear.h. */
      secHome.appendChild(el("div", "errwarn",
        "⚠ 按之前先確認：(1) 軸不能在 ERROR_STOP —— 先按上面 運轉 區的" +
        "「reset error」；(2) 起始方向那一端的極限不能正被壓住，否則一出發就被擋住。"));
      secHome.appendChild(el("div", "errhow2",
        "ⓘ 124 與 128 是同一個方法的正反兩面（手冊 p.560-561），方向已經包在編號裡，" +
        "所以不用再選一次。兩個都是找獨立的 /Home 原點開關、不依賴 Z 相 —— " +
        "實測這台的 Pn511 (2511h) = n.5432，/Home 確實分配在 SI5。" +
        "⚠ 按鈕上的正／負是「機構實際跑的方向」，跟手冊寫的 forward/reverse 相反 —— " +
        "驅動器的正方向受 Pn000 與機構安裝影響，兩者不一致是正常的。"));
    }

    /*  AI(W906-1203HOME-1) 20260915: 回HOME設定 -- the expandable block.
        User: "我點選回home設定後 下面會長出 一堆相關設定這樣", "每種模式希望都有
        圖示可以看", "因為每種模式都有說明 所以需要可以縮放", and "這些功能我都是
        確定要可以寫入資料的".

        Collapsed by default because it is a dozen rows on a page that already
        has several, and an operator who is jogging does not need the homing
        profile in the way. */
    {
      /*  Re-read here rather than reaching into the gear block's scope: these
          are block-locals there, and sharing them would tie two panels together
          for no gain. Both come from the same tags. */
      const stn  = tags.get(`${tag}.station`);
      const sub  = tags.get(`pci1203.ax${a}.gear.subAxis`);
      const half = (sub === 1) ? "B" : (sub === 0) ? "A" : "?";

      const hp = el("div", "subpanel");
        /*  AI(W906-1203SECT-1) 20260917: ⚠ THE LABEL NAMED FOUR THINGS AND THREE
          OF THEM WERE NO LONGER INSIDE IT. It read "回HOME設定（模式圖示、速度、
          原點座標、馬達方向）": 模式圖示 was deleted today, 馬達方向 moved to the
          motor panel on 20260916, and 原點座標 was never in it at all
          (CFG_AxHomePosition does not exist on this card -- 0x8000000A).
          A heading that promises contents it does not have is a bug of exactly
          the kind this page keeps paying for: the operator opens it, does not
          find 馬達方向, and reports the feature as missing. That has literally
          happened here once already. */
    const label = "回HOME設定（驅動器的歸原點參數）";
      const hdr = el("button", "cmd subheadbtn", "▸ " + label);
      hdr.type = "button";
      /*  ⚠ NOT data-cmd. This button opens a panel; it must never look like a
          command to onStageClick, which dispatches on data-cmd. */
      hdr.title = "點開/收合 — 這裡面的每一項都會真的寫進卡片或驅動器";
      hp.appendChild(hdr);

      const body = el("div", "homedetail");
      /*  style.display rather than the `hidden` attribute: this page's CSS is
          not guaranteed to carry [hidden]{display:none}, and a collapsible that
          does not collapse is worse than no collapsible. */
      /*  AI(W906-1203HOMEUI-1) 20260917: ⚠ OPEN BY DEFAULT NOW, and that is a
          deliberate reversal of part of the original request.
          User asked for "我點選回home設定後 下面會長出 一堆相關設定這樣", so it
          started collapsed. Since then the same operator has reported three
          things as MISSING that were inside it: 馬達方向 (moved out), the mode
          diagram (moved out), and now "回HOME 速度 相關的一些參數好像也都沒在
          介面". Three is not a coincidence; a drawer people do not open is a
          drawer whose contents do not exist.
          The toggle stays -- collapsing still works and is one click -- but the
          default is the state in which the settings can be found. */
      let homeOpen = true;
      const applyHomeOpen = () => {
        body.style.display = homeOpen ? "block" : "none";
        hdr.textContent = (homeOpen ? "▾ " : "▸ ") + label;
      };
      applyHomeOpen();
      hdr.addEventListener("click", () => { homeOpen = !homeOpen; applyHomeOpen(); });

      /*  --- 這台實際會用的兩個歸原點方法，用文字講清楚 -------------------
          AI(W906-1203TRIM-1) 20260917: ⚠⚠ THE VENDOR DIAGRAM BLOCK WAS HERE
          -- an <img>, three zoom buttons, a description box and showMode() --
          AND IT HAS BEEN REPLACED BY THIS TEXT. User: "沒用的按鈕刪刪掉 /
          介面可以精簡就精簡".

          It is deleted because on THIS machine it was inverted, not merely
          redundant. Advantech ships drawings for the sixteen "typical" card
          modes and for nothing else. Those sixteen are exactly the modes these
          axes REFUSE (0x8000510F -- homing here is DS402, done by the drive).
          The two methods this machine homes with, 124 and 128, have no vendor
          drawing at all, so the block showed pictures only for things that
          cannot run and printed "廠商沒有附這一組的圖示" for the things that can.
          Three zoom buttons existed to look more closely at the useless half.

          ⚠ The earlier request it was built for -- "我選了 但是沒有人和文字描述
          做動方式 也沒有圖說" -- is SATISFIED, NOT DROPPED: what they wanted was
          to know what the thing they selected would do. That is what the text
          below says, quoted from the manual, for the two methods that are left.
          Deleting the picture and keeping nothing would have re-created the
          original complaint.

          Source: SIEPC71081205 p.561 s13.4.2, quoted rather than paraphrased --
          "24 Homing with the home switch input (/Home) signal and starting in
          the forward direction. This method is same as method 8 except that the
          home position does not depend on the index pulse. Here, it depends
          only on changes in the relevant /Home signal or limit switch."
          and for 7-10 (which 8 belongs to): "if the /Home signal is already
          active when homing is started, the initial homing direction depends on
          the required edge ... If the initial movement direction is away from
          the /Home signal, the motor will reverse direction when the limit
          switch in the movement direction is input." */
      {
        /*  ⚠ white-space:pre-wrap, and it is not cosmetic. These strings carry
            numbered steps separated by \n, and HTML collapses \n to a space --
            so the first render ran "1. 往正方向起跑 2. 碰到 /Home 3. ..." into
            one flowing paragraph, which is precisely the form a sequence must
            not be read in. textContent (not innerHTML) keeps the manual's text
            un-parsed; pre-wrap is what makes the line breaks in it survive. */
        const mk = (t) => {
          const d = el("div", "errhow2");
          d.style.whiteSpace = "pre-wrap";
          d.textContent = t;
          return d;
        };
        secHome.appendChild(el("div", "panelrow"))
         .appendChild(el("span", "panellabel", "動作說明"));
        /*  AI(W906-1203HOMEDIR-1) 20260918: ⚠ THESE FOLLOW THE BUTTONS, NOT THE
            MANUAL. When the labels were swapped on the operator's observation,
            leaving this text keyed to the manual's forward/reverse would have
            put an explanation saying "往正方向起跑" directly under a button
            labelled 往負方向 -- the page contradicting itself in two adjacent
            elements, which is worse than either wording alone.
            The manual's own words are still quoted, tagged as the manual's. */
        secHome.appendChild(mk(
          "◀ 往負方向歸原點（124 = DS402 方法 24）\n" +
          "　1. 往這一端起跑，去找 /Home 開關。\n" +
          "　2. 碰到 /Home 的邊緣就是原點 —— 不看 Z 相（編碼器零點），" +
          "所以不需要馬達轉滿一圈。\n" +
          "　3. 如果按下去的時候「已經壓在 /Home 上」，起始方向會由它要找的是哪一個邊緣決定。\n" +
          "　4. 如果一出發就跑離 /Home，撞到那個方向的極限時會自己反向回來找。\n" +
          "　（手冊把方法 24 寫成 starting in the forward direction —— " +
          "那是驅動器自己的正方向，跟機構看到的方向相反是正常的。）"));
        secHome.appendChild(mk(
          "▶ 往正方向歸原點（128 = DS402 方法 28）\n" +
          "　跟 124 完全一樣，只是起跑方向相反（手冊：same as method 12 " +
          "except that the home position does not depend on the index pulse）。"));
        secHome.appendChild(mk(
          "ⓘ 兩段速度：往前衝去找開關那一段跑「找開關速度」(6099h:1)，" +
          "碰到之後退出／慢慢壓回原點那一段跑「找零點速度」(6099h:2)。" +
          "找零點速度設太快，原點的重現性就會從微米級掉到毫米級 —— " +
          "這兩個值在下面「驅動器」那一組。"));
      }

      /*  --- the card's homing profile, all writable --------------------- */
      const homeRow = (label2, key, cmd, hint, isFlag) => {
        const row = el("div", "panelrow");
        row.appendChild(el("span", "panellabel", label2));
        row.appendChild(el("span", "panelhint", "卡片目前"));
        const cur = el("span", "val");
        cur.dataset.k = `pci1203.ax${a}.home.${key}`;
        cur.dataset.f = isFlag ? "onoff" : "pos";
        cur.textContent = "---";
        row.appendChild(cur);
        if (isFlag) {
          row.appendChild(cmdButton("開 1", cmd, tag, { value: 1 }));
          row.appendChild(cmdButton("關 0", cmd, tag, { cls: "warnbtn", value: 0 }));
        } else {
          row.appendChild(numBox(`ax${a}.h_${key}`, 0, "8em"));
          row.appendChild(cmdButton("set", cmd, tag, { from: `ax${a}.h_${key}` }));
        }
        if (hint) row.appendChild(el("span", "panelhint", hint));
        body.appendChild(row);
      };

      /*  AI(W906-1203TRIM-1) 20260917: ⚠⚠ SIX CARD HOME ROWS WERE HERE AND ARE
          GONE -- 搜尋速度 / 找回速度 / 加速度 / 減速度 / 偏移距離 / 偏移速度.
          User: "沒用的按鈕刪刪掉".

          They were added this morning with a warning printed above them saying
          they do not work, on the argument that an operator who had been
          looking at them for two days needed the explanation more than the tidy
          panel. That argument survives -- the explanation is still here -- but
          the SIX SET BUTTONS did not need to survive with it. A button that is
          documented not to work is still a button that writes the card.

          The evidence, so this is a deletion of code and not of knowledge:
            * velHigh/velLow/acc/dec: PROVEN inert. Set to 1234/4321/111111 on
              ax3, confirmed by read-back, homed with method 124 -- the drive
              still held 8000/2000/10000. Full-machine scan agreed: only the two
              axes homed today carry non-default drive homing speeds.
            * offsetDistance/offsetVel: NOT separately measured, and this line
              says so. They are the same card-side home profile that the four
              speeds belong to, and this axis never runs that profile, so the
              inference is that they are inert for the same reason. They are
              removed rather than left with an unverified label because the
              DRIVE's own 607Ch offset is right below them, is proven to reach
              the hardware, and two controls named "原點偏移" where one is real
              and one is probably dead is worse than one control.
            * resetEnable stays, below, and is the one that is genuinely
              uncertain: it is card-side bookkeeping ("after home, zero the
              coordinate") rather than part of the motion profile, so the
              argument that kills the other six does not obviously reach it.
              It is labelled 未驗證 rather than quietly presented as working.

          ⚠ 原點座標值 (CFG_AxHomePosition) and 越過距離 (CFG_AxHomeCrossDistance)
          were never rows: measured 20260915 with BOTH accessors on ax3,
          F64 -> 0x800000EF "The data size is wrong.", U32 -> 0x8000000A
          "PropertyID is not supported." The second is the answer -- this card
          does not have them. CFG_AxHomeMode / Dir / SwitchMode answer the same,
          which is why mode and direction are ARGUMENTS to Acm_AxHome. */
      body.appendChild(el("div", "errwarn",
        "⚠ 「卡片」的那組回HOME速度（PAR_AxHomeVelHigh/VelLow/Acc/Dec）已經從這裡移除。" +
        "實測：把它們設成 1234／4321／111111、確認讀回來、按歸原點，" +
        "驅動器裡拿到的還是 8000／2000／10000 —— 這台的軸是 DS402，歸原點是驅動器做的，" +
        "卡片那組參數屬於它自己那 16 個「典型模式」，而這台的軸會拒絕那 16 個" +
        "（0x8000510F）。真正生效的是下面「驅動器」那一組。"));

      homeRow("歸零後重設座標", "resetEnable", "pci1203.ax.setHome.resetEnable",
              "CFG_AxHomeResetEnable — ⚠ 未驗證：歸原點完成後把座標歸零，" +
              "這是卡片自己記帳，理論上不受 DS402 影響，但沒有實測過", true);

      /*  ================= THE DRIVE'S OWN HOMING PARAMETERS =================
          AI(W906-1203DHOME-1) 20260917.

          These are the ones a home on this machine obeys. The measurement that
          established it is in EtherCAT/Pci1203Gear.h; the short version is that
          the full-machine scan found exactly two axes carrying non-default
          homing speeds, and they are the two that were homed through this page
          today -- with station 30's 6098h reading back 28, the method that was
          commanded there, while every never-homed axis reads the default 37.

          ⚠ THREE OF THE FOUR ARE RE-SEEDED BY THE NEXT HOME. Acm_AxHome pushes
          the axis's PTP speeds into 6099h:1/:2 and 609Ah when it is called, so
          a speed typed here survives until the operator homes and no longer. It
          is still worth setting and showing -- it is what the drive is holding
          right now, and reading it is the only way to see what a home actually
          ran at -- but a row that is about to be overwritten must not look like
          one that persists. Hence the per-row 有效期 hint.
          607Ch is the exception: the card never writes it. */
      {
        const dhBody = body;
        dhBody.appendChild(el("div", "errhow2",
          "★ 下面這組才是這台實際用的。歸原點是「驅動器」做的（6098h 存在、" +
          "6502h bit5 = 1），所以它跑的是驅動器自己的 6099h／609Ah。"));

        const dhRow = (label2, key, wire, obj, hint, clobbered) => {
          const row = el("div", "panelrow");
          row.appendChild(el("span", "panellabel", label2));
          row.appendChild(el("span", "panelhint", "驅動器目前"));
          const cur = el("span", "val");
          cur.dataset.k = `pci1203.ax${a}.dhome.${key}`;
          cur.dataset.f = "pos";
          cur.textContent = "---";
          row.appendChild(cur);
          /*  ⚠ Same guard the other drive writes use: this is addressed by
              STATION, and without a station number there is nothing to confirm
              against. Showing a set button that cannot work is the defect this
              page keeps being fixed for. */
          if (typeof stn !== "number") {
            row.appendChild(el("span", "panelhint",
              "這一軸的站號還沒讀到，無法設定 —— 這是寫到「站」不是寫到「軸」。"));
          } else {
            row.appendChild(numBox(`ax${a}.dh_${key}`, 0, "9em"));
            const b = cmdButton("set", wire, tag, {
              from: `ax${a}.dh_${key}`,
              confirm: `站 ${fmtStation(stn)} — 軸 ${half}`,
              station: stn,
              title: `SDO ${obj}（軸 B 為 +0x800）— 手冊 s14.9 / p.559`
            });
            /*  ⚠ "val", not the default "dir". See pci1203.js's askConfirm --
                a hard-coded "dir=" produces a well-formed message the C++ side
                rejects with "has no val=", which reads as a server bug. */
            b.dataset.confirmkey = "val";
            b.dataset.confirmbody =
              `${label2}  →  ${obj}\n\n` +
              (clobbered
                ? "⚠ 這個值下一次歸原點就會被蓋掉。\n" +
                  "卡片在呼叫 Acm_AxHome 的當下，會把這一軸的「PTP 速度」" +
                  "（運行速度／初速度／加速度）推進驅動器的 6099h／609Ah。\n" +
                  "所以要讓歸原點「持續地」用某個速度，今天要改的是 PTP 速度那一列。\n" +
                  "這裡設的值，是用來看驅動器現在握著什麼、以及單次確認用的。\n\n"
                : "ⓘ 這個值不會被歸原點蓋掉 —— 卡片從不寫 607Ch。\n\n") +
              "⚠ 寫完要按「存檔」（1010h）才不會斷電消失。";
            b.dataset.confirmgo = "寫入驅動器";
            row.appendChild(b);
          }
          if (hint) row.appendChild(el("span", "panelhint", hint));
          dhBody.appendChild(row);
        };

        dhRow("找開關速度", "velSwitch", "pci1203.ax.setDriveHome.velSwitch",
              "6099h:1", "圖上的搜尋段（原廠 500000）", true);
        dhRow("找零點速度", "velZero", "pci1203.ax.setDriveHome.velZero",
              "6099h:2", "圖上的退出／壓回段，決定重現性（原廠 100000）", true);
        dhRow("歸原點加速度", "acc", "pci1203.ax.setDriveHome.acc",
              "609Ah", "原廠 1000", true);
        dhRow("原點偏移量", "offset", "pci1203.ax.setDriveHome.offset",
              "607Ch", "★ 不會被歸原點蓋掉，可以是負的（±536870911）", false);

        /*  6098h is READ-ONLY here on purpose: Acm_AxHome writes it from its
            mode argument, so a setter would be a second source of truth for one
            value and whichever lost would look like the drive ignoring the
            operator. Shown because it is the receipt -- it is what proved the
            card writes this block at all. */
        {
          const row = el("div", "panelrow");
          row.appendChild(el("span", "panellabel", "驅動器目前的模式"));
          const cur = el("span", "val");
          cur.dataset.k = `pci1203.ax${a}.dhome.method`;
          cur.dataset.f = "pos";
          cur.textContent = "---";
          row.appendChild(cur);
          row.appendChild(el("span", "panelhint",
            "6098h，唯讀 — 按歸原點時由卡片寫入（= 你選的模式減 100）。" +
            "原廠預設 37；還沒歸過原點的軸就是 37。"));
          dhBody.appendChild(row);
        }

        dhBody.appendChild(el("div", "errwarn",
          "★ 想改「回HOME 速度」，今天要改的是上面「PTP speed」那一列的" +
          "運行速度／初速度／加速度。實測：全機掃描 12 個站，只有今天歸過原點的" +
          "兩顆軸帶著 8000／2000／10000（就是卡片的 PTP 速度），其餘全部停在" +
          "驅動器原廠的 500000／100000／1000；而站 30 的 6098h 讀回 28，正是當時" +
          "下給它的模式。所以是「按歸原點的那一刻，卡片拿 PTP 速度去填驅動器」。"));
      }

      /*  AI(W906-1203SECT-1) 20260917: ⚠⚠ A NOTE THAT REFERRED TO THREE DELETED
          THINGS WAS HERE. It read: "「往正方向歸、碰到原點、再往負方向歸」就是
          上面 home 那一列的 mode ＋ dir 兩個選擇合起來決定的 ... 請選 Refind 系列
          （12 MODE13 / 15 MODE16）... 上面的圖會直接畫給你看."
          By the time it was read today there was no "home 那一列", no diagram,
          and MODE13/MODE16 are two of the sixteen this machine answers
          0x8000510F to. Three dead references in one paragraph.

          ⚠ IT SURVIVED THE EARLIER SWEEP BECAUSE IT APPENDS TO `body`, NOT `p`
          -- it is inside the collapsible, and the sweep that fixed the others
          was looking at the panel. That is the general shape of this failure:
          deleting a control does not delete the prose that points at it, and
          the prose is wherever it happens to live. The rule that catches it
          next time is to grep for the NAME of whatever was removed
          ("mode", "圖", the mode numbers) rather than for the container. */
      body.appendChild(el("div", "errhow2",
        "ⓘ 這張卡沒有「原點座標值」和「越過距離」這兩個參數。20260915 用兩種型別各問過一次：" +
        "F64 回 0x800000EF「The data size is wrong.」、U32 回 0x8000000A" +
        "「PropertyID is not supported.」—— 後面那個才是答案。同一族的 CFG_AxHomeMode / " +
        "CFG_AxHomeDir / CFG_AxHomeSwitchMode 也一樣不支援。"));

      /*  AI(W906-1203DIR-2) 20260916: ⚠ 馬達方向 USED TO BE BUILT HERE AND IS
          NOT ANY MORE -- it moved to the 馬達/編碼器 panel, which is always
          visible, and this is only a pointer to it.

          User: "我需要你幫我找馬達方向反向的參數 就是CW CCW對調的參數
                 寫到介面上可以讓我設定".
          It had been on the page since 20260915. They could not find it,
          because it was inside THIS collapsible -- a panel you have to know to
          open, titled 回HOME設定. It landed here originally because they asked
          for it while we were building the homing settings, and "where it was
          asked for" is not the same as "where someone would look for it".

          A control nobody can find is a control that does not exist, and the
          symptom is indistinguishable from the feature being missing. It is not
          duplicated in both places: two buttons issuing the same drive write is
          two things to keep in step, and the one that drifts is the one nobody
          opened. */
      body.appendChild(el("div", "errhow2",
        "ⓘ 馬達正方向（Pn000，CW／CCW 對調）不在這一區 —— " +
        "它在上面「馬達 / 編碼器 / 電子齒輪比」那一區，不用展開就看得到。" +
        "放在那裡是因為它跟編碼器、齒輪比一樣是驅動器的基本設定，" +
        "不是只有歸原點才會用到。"));

      hp.appendChild(body);
      secHome.appendChild(hp);
    }

    /*  AI(W906-1203CTL-35) 20260911: EACH SPEED NOW SHOWS WHAT THE CARD HOLDS,
        beside the box for changing it -- which is how Common Motion Utility's
        運動參數設置 block reads, and the user asked for parity with it.

        ⚠ THE OLD LAYOUT WAS ACTIVELY MISLEADING: an empty input box renders as
        "0", so every speed on this panel appeared to be zero. A jog issued with
        a jog velocity of zero returns SUCCESS and moves nothing, which is
        indistinguishable from a broken button -- so a panel that implies zero
        when the card holds 8000 is set up to send someone hunting the wrong
        fault. MEASURED on this card: PTP and JOG are both
        2000 / 8000 / 10000 / 10000, matching the Utility's own display exactly.

        The readback is bound (data-k), so pressing "set" shows its effect here
        rather than requiring a reload to confirm anything happened. */
    /*  AI(W906-1203SECT-1) 20260917: ⚠ ONE ROW PER SPEED, NOT FOUR SPEEDS ON
        ONE ROW. All four on a single line is sixteen elements wide and it
        WRAPPED on this machine's 1280px panel: the screenshot showed 減速度's
        label at the right edge with its value, box and set button orphaned on
        the next line, under 初速度's label. A control whose label is directly
        above a different control's box is how the wrong speed gets set.
        It also made the row the odd one out -- every other settable row on this
        page is 標籤 / 目前值 / 輸入 / 按鈕, and this one was that four times. */
    const speedRow = (label, fam) => {
      const rows = [];
      let first = true;
      for (const sp of fam) {
        const r = el("div", "panelrow");
        r.appendChild(el("span", "panellabel", first ? label : ""));
        first = false;
        r.appendChild(el("span", "panelhint", sp[1]));
        const cur = cellOf(tags, `pci1203.ax${a}.${sp[2]}`, fmtPos);
        r.appendChild(bindCell(el("span", "curval " + cur.cls, cur.text),
                               `pci1203.ax${a}.${sp[2]}`, "pos", "curval"));
        r.appendChild(numBox(`ax${a}.sp.${sp[0]}`, 0, "7em"));
        r.appendChild(cmdButton("set", `pci1203.ax.setSpeed.${sp[0]}`, tag,
          { from: `ax${a}.sp.${sp[0]}`, cls: "small", title: sp[3] }));
        if (sp[4]) r.appendChild(el("span", "panelhint", sp[4]));
        rows.push(r);
      }
      const wrapDiv = el("div");
      for (const r of rows) wrapDiv.appendChild(r);
      return wrapDiv;
    };

    // --- speeds: PTP (PAR_Ax*) ---
    secSpeed.appendChild(speedRow("PTP speed", [
      ["init", "初速度",   "velLow",  "Acm_SetF64Property(ax, PAR_AxVelLow, v)",
       "★ 也是歸原點的「找零點速度」—— 按歸原點時卡片會把它推進驅動器 6099h:2"],
      ["run",  "運行速度", "velHigh", "Acm_SetF64Property(ax, PAR_AxVelHigh, v)",
       "★ 也是歸原點的「找開關速度」—— 推進 6099h:1"],
      ["acc",  "加速度",   "acc",     "Acm_SetF64Property(ax, PAR_AxAcc, v)",
       "★ 也是歸原點的加速度 —— 推進 609Ah"],
      ["dec",  "減速度",   "dec",     "Acm_SetF64Property(ax, PAR_AxDec, v)"],
    ]));

    /*  AI(W906-1203TRIM-1) 20260917: ⚠⚠ THE "JOG speed" ROW WAS HERE -- four
        more boxes and four more set buttons, CFG_AxJogVelLow / VelHigh / Acc /
        Dec -- AND IT IS GONE. User: "沒用的按鈕刪刪掉".

        ⚠ THIS PAGE CANNOT ISSUE A JOG, AND THAT IS DELIBERATE AND MEASURED.
        The arrows were Acm_AxJog until 20260914, when it was measured on ax3:
            Acm_AxJog(ax, dir)      SUCCESS, cmdPos moved 0.000, state stayed
                                    READY, Acm_GetLastError 0x8000510B
                                    "Invalid axis states." for two full seconds
            Acm_AxMoveVel(ax, dir)  CONTINUE_MOTION, cmdPos moved -17647
            Acm_AxMoveRel(ax, -d)   PTP_MOTION, cmdPos moved -10000 exactly
        On this SDK jog is a HARDWARE function -- CFG_AxJogPAssign / JogNAssign
        assign it to DI CHANNELS -- so with nothing assigned the axis is in no
        state to jog, and the vendor ships zero examples using the call. The
        arrows became MoveRel / MoveVel that day.

        So these four rows configured the speeds of a command that this page has
        not issued for three days and that does nothing when it is issued. They
        were left behind by that change: four numbers an operator can set, that
        read back correctly, and that cannot affect anything on this machine.
        ⓘ 移動 Move and 連續 Continue both run on the PTP family above, which is
        the row that actually governs the arrows. */

    /*  AI(W906-1203CTL-44) 20260911: 速度類型, from the Utility's 運動參數設置.
        ⚠ PAR_AxJerk IS A SELECTOR, NOT A MAGNITUDE -- the vendor example sets
        it to 0 for the 梯形 radio and 1 for S形 with the comment "Set the type
        of velocity profile: t-curve or s-curve" (PTP/Form1.cs:451-463). So it
        gets two buttons, not a number box: a box invites "5000" and would
        silently select the S-curve. The C++ side refuses anything but 0 or 1.
        MEASURED on this card: PAR_AxJerk reads 0 and the Utility shows 梯形
        selected -- the readback beside the buttons is that same value. */
    const r7 = el("div", "panelrow");
    r7.appendChild(el("span", "panellabel", "速度類型"));
    const jerkC = cellOf(tags, `pci1203.ax${a}.jerk`, fmtPos);
    r7.appendChild(el("span", "panelhint", "目前"));
    r7.appendChild(bindCell(el("span", "curval " + jerkC.cls,
                               jerkTypeText(tags.get(`pci1203.ax${a}.jerk`))),
                            `pci1203.ax${a}.jerk`, "jerkType", "curval"));
    r7.appendChild(cmdButton("梯形 T-curve", "pci1203.ax.setSpeed.jerk", tag,
      { value: 0, cls: "small", title: "Acm_SetF64Property(ax, PAR_AxJerk, 0)" }));
    r7.appendChild(cmdButton("S形 S-curve", "pci1203.ax.setSpeed.jerk", tag,
      { value: 1, cls: "small", title: "Acm_SetF64Property(ax, PAR_AxJerk, 1)" }));
    /*  AI(W906-1203SECT-1) 20260917: "Jeck Factor" -> "Jerk Factor". A typo, but
        a searchable one: the property is PAR_AxJerkFactor, and an operator
        cross-checking this box against the SDK header or Common Motion Utility
        was looking for a word that exists nowhere. */
    r7.appendChild(el("span", "panelhint", "Jerk Factor"));
    const jfC = cellOf(tags, `pci1203.ax${a}.jerkFactor`, fmtPos);
    r7.appendChild(bindCell(el("span", "curval " + jfC.cls, jfC.text),
                            `pci1203.ax${a}.jerkFactor`, "pos", "curval"));
    r7.appendChild(numBox(`ax${a}.sp.jerkFactor`, 0, "5em"));
    r7.appendChild(cmdButton("set", "pci1203.ax.setSpeed.jerkFactor", tag,
      { from: `ax${a}.sp.jerkFactor`, cls: "small",
        title: "Acm_SetF64Property(ax, PAR_AxJerkFactor, v)" }));
    secSpeed.appendChild(r7);

    /*  AI(W906-1203CTL-44) 20260911: 查看範圍>>. These are the CEILINGS, read
        from the card -- the example's own comments say PAR_AxVelHigh "must be
        smaller than CFG_AxMaxVel" and PAR_AxAcc "smaller than or equal to
        CFG_AxMaxAcc". A speed refused for exceeding a limit nobody can see is
        exactly the dead end this page exists to prevent, so the limits are
        shown next to the boxes that can violate them.
        ⚠ Read-only by design: raising a machine's maximum velocity is not a
        panel gesture, and CFG_AxMax* is deliberately absent from the write
        allowlist in Pci1203Control.h. */
    const r8 = el("div", "panelrow");
    r8.appendChild(el("span", "panellabel", "查看範圍"));
    for (const lim of [["最大速度", "maxVel"], ["最大加速度", "maxAcc"], ["最大減速度", "maxDec"]]) {
      r8.appendChild(el("span", "panelhint", lim[0]));
      const c0 = cellOf(tags, `pci1203.ax${a}.${lim[1]}`, fmtPos);
      r8.appendChild(bindCell(el("span", "curval " + c0.cls, c0.text),
                              `pci1203.ax${a}.${lim[1]}`, "pos", "curval"));
    }
    r8.appendChild(el("span", "panelhint", "（唯讀，上面的設定值不能超過這些）"));
    secSpeed.appendChild(r8);

    /*  AI(W906-1203TRIM-1) 20260917: ⚠ THE 復位計數器 ROW WAS HERE AND IS GONE.
        User: "沒用的按鈕刪刪掉" -- and this was the most literal example on the
        page: a button rendered with `disabled = true`, which could not be
        pressed by anyone, ever.

        It was put there on purpose in 20260911 so that an operator comparing
        this page against Common Motion Utility would see that the Utility's
        復位計數器 is missing DELIBERATELY rather than by oversight. That reason
        was sound then and does not survive the request to strip the page: it
        spent a row and a permanently-greyed button answering a question nobody
        on this machine has asked in six days.

        ⓘ The underlying decision is UNCHANGED and is not lost -- it lives in
        Pci1203Control.h's forbidden list, with its reason: Acm_AxSetCmdPosition
        and Acm_AxSetActualPosition (Examples/Windows/C#/PTP/Form1.cs:138,145)
        redefine where the axis thinks it is, so the next MOVE ABS goes
        somewhere else entirely. One word from the user enables it. */

    /*  AI(W906-1203SECT-1) 20260917: the DRY RUN banner goes ABOVE the sections,
        not after them. It is a statement about every button on the panel, and a
        panel-wide caveat printed at the bottom is read after the decision it
        was meant to inform. */
    if (dry) {
      p.insertBefore(el("div", "emptynote",
        "DRY RUN: these buttons record the exact vendor call they would make and issue " +
        "nothing. Read “last call” in the Control section to see it."), secRun);
    }
    wrap.appendChild(p);
  }

  if (shown === 0) {
    wrap.appendChild(el("div", "emptynote",
      "No axis slot reports opened=true, so there is nothing to operate. A panel for an " +
      "axis the monitor never opened could only ever produce refusals."));
  }
  return wrap;
}

export function sectionRing(tags) {
  const c = (t, f) => cellOf(tags, t, f);
  const s = el("section");
  s.appendChild(el("h2", null, "EtherCAT ring — discovered stations"));

  /* ⚠ scan.truncated must never be hidden. A scan that hit its time budget
     found FEWER stations than exist, and reading the short list as the topology
     turns "we ran out of time" into "that station is dead" -- the wrong repair
     entirely. WebBridgeTags.cpp carries the same warning at the staging site. */
  if (tags.get("pci1203.scan.truncated") === true) {
    s.appendChild(banner("bad",
      "SCAN TRUNCATED — this list is INCOMPLETE",
      "The sweep hit its wall-clock budget, so it found fewer stations than exist. " +
      "Do NOT read a missing station here as a dead station."));
  }

  s.appendChild(kvTable(tags, [
    ["stations found", c("pci1203.scan.found"), ""],
    ["rings swept",    c("pci1203.scan.rings"), ""],
    ["slots per ring", c("pci1203.scan.slots"), ""],
    ["scan duration",  c("pci1203.scan.ms", v => v + " ms"), ""],
    ["truncated",      c("pci1203.scan.truncated", fmtBool), ""],
  ]));

  /* AI(W906-MW2b) 20260910: A STATION HAS THREE DIFFERENT NUMBERS AND THE
     VENDOR API CALLS TWO OF THEM "SlaveIP". All three are shown, side by side,
     with names that say which is which:

       STATION (dials)  ESC 0x0012, the Configured Station ALIAS. Loaded by the
                        slave controller from the MODULE'S OWN EEPROM at every
                        power-up -- on this machine it is the ROTARY SWITCH
                        SETTING. This is the number an operator standing next
                        to the hardware can read off it, so it comes FIRST.
       api addr         what the SDK wants back as "SlaveIP".
       ESC 0x10         Configured Station Address, master-assigned, and what
                        Acm_DevSetSlaveID changes.

     ⚠ MEASURED 20260910 -- they DISAGREE on every IO module:
           pos 8  api 80  ESC 0x10 = 9   alias 0x50 (80)   ECx-P32-HON 32DI
     Showing any ONE of them under the label "station" would be wrong two
     thirds of the time.
       ⓘ On the eight ECAT-2515 junctions all three happen to agree (1..8), and
       that coincidence briefly supported a WRONG conclusion during this work.
       Do not re-derive the relationship from the junctions. */
  /*  AI(W906-MW2b) 20260910: NAME THE STATIONS THAT CLASH.
      pci1203.idConflict says THAT the station numbers conflict; this says
      WHICH, by counting aliases across the published slots. That is the
      difference between repeating an error code back at the operator and
      telling them which modules to go and set. Measured on this machine the
      same day: nine Yaskawa SERVOPACKs arrived on the ring all reading alias
      0, and that was the entire cause. */
  /*  AI(W906-1203ALM-14) 20260914: ⚠ COUNTED `alias`, WHICH IS NEVER PUBLISHED.
      This whole diagnostic has been silently rendering NOTHING since the ESC
      0x0012 read was gated off on 20260911 -- `typeof a !== "number"` skipped
      every slave, dupes was always empty, and the banner that exists to say
      WHICH modules collide simply never appeared. A diagnostic that cannot
      fire is worse than an absent one: its silence reads as "no conflict".
      Counting `addr` instead answers the same question with data that IS on
      the wire -- and addr is what EC_SubDeviceIDConflicted is actually about,
      since the card raises it when two stations on DIFFERENT RINGS answer to
      the same configured address. */
  const aliasCount = new Map();
  for (let i = 0; i < SLAVE_SLOTS; i++) {
    if (tags.get(`pci1203.slave${i}.present`) !== true) continue;
    const a = tags.get(`pci1203.slave${i}.addr`);
    if (typeof a !== "number") continue;
    aliasCount.set(a, (aliasCount.get(a) || 0) + 1);
  }
  const dupes = [...aliasCount.entries()].filter(([, n]) => n > 1)
                                         .sort((x, y) => x[0] - y[0]);
  if (dupes.length > 0) {
    s.appendChild(banner("bad",
      "station numbers are NOT unique — this is what EC_SubDeviceIDConflicted means",
      dupes.map(([a, n]) =>
        n + " stations share " + (a === 0 ? "NO station number (alias 0)"
                                          : "station " + fmtStation(a))).join("; ") +
      ".  Set a distinct number on each module's rotary switches, then POWER-CYCLE " +
      "the stations — the alias is loaded from each module's own EEPROM at power-up, " +
      "so a PC reboot does not apply it. Until then the master cannot address them " +
      "individually and no DI byte can be attributed to a module."));
  }

  const t = el("table", "grid");
  const head = el("tr");
  for (const h of ["slot", "STATION (dials)", "model", "pos", "api addr",
                   "ESC 0x10", "ring", "state (raw EC_SLAVE_STATE_*)"]) {
    head.appendChild(el("th", null, h));
  }
  t.appendChild(head);

  let shown = 0;
  for (let i = 0; i < SLAVE_SLOTS; i++) {
    const p = `pci1203.slave${i}.`;
    /* Absent slots are SKIPPED, not rendered as rows of "---". With 32 slots
       holding 13 stations that would be 19 rows of noise, and noise is what
       stops a real fault being noticed. The count of published slots is in the
       kv table above, so nothing is hidden without being stated. */
    if (tags.get(p + "present") !== true) continue;
    shown++;

    const tr = el("tr");
    tr.appendChild(el("th", null, "s" + i));

    /* The station cell is a LINK to this page filtered to that station. A link
       navigates; it does not command the machine. That distinction is the
       whole reason this page can have one at all -- see the read-only note in
       the file header and the assertion in probe_pci1203.mjs. */
    const alias = tags.get(p + "alias");
    const stCell = el("td", null);
    if (alias === 0) {
      /* ⚠ ALIAS 0 IS "NO STATION NUMBER SET", NOT "STATION ZERO", and it is
         NOT a link. Measured on this machine 20260910: nine Yaskawa SERVOPACKs
         arrived on the ring all reading alias 0, which is precisely why
         Acm_DevOpen reports EC_SubDeviceIDConflicted -- they are not unique.
         Linking them would produce nine identical dead links to ?station=0,
         and a dead link on a diagnostic page is worse than no link: it invites
         a click that answers nothing and looks like the page is broken. */
      stCell.className = "unset";
      stCell.textContent = "not set (0)";
      stCell.setAttribute("title",
        "the module's own EEPROM alias is 0 -- no station number is set on the hardware");
    } else if (typeof alias === "number") {
      const a = el("a", "stlink", fmtStation(alias));
      a.setAttribute("href", `?src=ws&station=${alias}`);
      a.setAttribute("title", "show only this station's DI bytes");
      stCell.appendChild(a);
    } else {
      const cc = c(p + "alias", fmtHex2);
      stCell.className = cc.cls;
      stCell.textContent = cc.text;
    }
    tr.appendChild(stCell);

    tr.appendChild(td(c(p + "name")));
    tr.appendChild(td(c(p + "pos")));
    tr.appendChild(td(c(p + "addr")));
    tr.appendChild(td(c(p + "escAddr")));
    tr.appendChild(td(c(p + "ring")));
    /* state is published gated on stateValid, not on present: a station that
       answered the SCAN and now refuses is a DROPPED SLAVE -- a real fault --
       so the row stays visible showing the failure instead of vanishing. */
    tr.appendChild(td(c(p + "state")));
    t.appendChild(tr);
  }

  if (shown === 0) {
    /* ⚠ NOT class "null". An earlier draft used it and probe_pci1203.mjs
       failed immediately: `.null` means "this tag is present and its VALUE is
       null", and the probe asserts every such cell reads exactly "---". Using
       it for prose would have quietly weakened the one assertion that would
       catch a future refactor turning nulls into zeros. Explanatory text gets
       its own class. */
    const tr = el("tr");
    const cell = el("td", "emptynote", "no station is present on any ring");
    cell.setAttribute("colspan", "8");
    tr.appendChild(cell);
    t.appendChild(tr);
  }

  s.appendChild(wrapScroll(t));
  return s;
}

/** One byte rendered as eight lamps, b7 leftmost so the row reads like the hex
 *  beside it -- the same order Common Motion Utility's "Bit 7 ... 0" header
 *  uses. A row running the other way to its own number is a misreading waiting
 *  to happen. */
/*  AI(W906-1203CTL-27) 20260911: one lamp's appearance, as DATA, so that
    building it and refreshing it in place cannot drift apart. `good` encodes
    the IO_BITS spec ("true"/"false"/"") for the axis lamps, and is absent for
    a plain byte bit. */
function lampState(tags, tag, good, bit) {
  const cc = cellOf(tags, tag);
  if (cc.cls !== "val") {
    return { text: cc.text === "n/a" ? "?" : "–", cls: "unknown" };
  }
  const v = tags.get(tag);
  const on = (bit === undefined || bit === null || bit === "")
    ? (v === true)
    : (((v >> Number(bit)) & 1) === 1);
  let cls = on ? "on" : "off";
  if (on && good === "false") cls = "alarm";
  if (on && good === "true")  cls = "good";
  return { text: on ? "1" : "0", cls };
}

function byteLamps(tr, tags, tag) {
  for (let b = 7; b >= 0; b--) {
    const n = el("td", "lampcell");
    const st = lampState(tags, tag, "", b);
    const sp = el("span", "lamp " + st.cls, st.text);
    /*  Bound for in-place refresh: data-k is the byte, data-bit which bit,
        data-lampgood how ON should be coloured. Without these the page has to
        be rebuilt to show a sensor change, and rebuilding it is what made every
        button unclickable. */
    sp.dataset.k = tag;
    sp.dataset.bit = String(b);
    sp.dataset.lampgood = "";
    tr.appendChild(n);
    n.appendChild(sp);
  }
}

/** The station that owns flat DI port `i`, or null when the master's IO map
 *  had nothing for it. null is NOT 0 -- station 0 would be a real station. */
export function diStationOf(tags, i) {
  const v = tags.get(`pci1203.di${i}.station`);
  return (typeof v === "number") ? v : null;
}

/* ---------------------------------------------------------------------------
   AI(W906-MW2b) 20260910: IO, GROUPED BY THE MODULE THAT OWNS THE BYTES.

   Acm_DaqDiGetByte hands back a FLAT image with no station attribution, so the
   old version of this section could show 21 anonymous bytes and could not
   answer the first question anyone asks when a sensor does not read: WHICH
   MODULE IS THIS INPUT ON. The publisher now carries the answer --
   Acm_DevUpLoadMapInfo(MapType 1) maps every flat offset to its owning
   station -- and this section groups by it. Measured layout 20260910:
       station 80 -> ports 0..3     83 -> 12..15
       station 81 -> ports 4..7     84 -> 16..19
       station 82 -> ports 8..11
   `opts.station` narrows to one group; the ring table's station cells link
   here with ?station=N.

   ⚠ THE FLAT PORT NUMBER IS ALWAYS SHOWN BESIDE THE STATION, never instead of
   it. The flat port is what Acm_DaqDiGetByte was actually asked for; dropping
   it would make every reading on this page unverifiable.
   --------------------------------------------------------------------------- */
export function sectionIO(tags, opts) {
  const only = (opts && typeof opts.station === "number") ? opts.station : null;
  const s = el("section");
  s.appendChild(el("h2", null, "IO — digital input, by station"));

  /* ⚠ SAY WHAT IS BEING HIDDEN. A section showing 4 rows out of 20 has to
     announce that it is filtered, or it is indistinguishable from a page that
     lost 16 rows. */
  if (only !== null) {
    s.appendChild(banner("info",
      "filtered to STATION " + fmtStation(only),
      "Only this module's DI bytes are shown. Remove ?station= from the URL, or " +
      "follow this link, to see every attributed byte again."));
  }

  /* Group ports by owning station, preserving flat order within a group. */
  const groups = new Map();          // station -> [port, ...]
  const orphans = [];                // attributed to nothing
  for (let i = 0; i < diPortCount(tags); i++) {
    if (!tags.has(`pci1203.di${i}`)) continue;
    const st = diStationOf(tags, i);
    if (st === null) { orphans.push(i); continue; }
    if (only !== null && st !== only) continue;
    if (!groups.has(st)) groups.set(st, []);
    groups.get(st).push(i);
  }

  const mkTable = () => {
    const t = el("table", "grid");
    const head = el("tr");
    for (const h of ["port", "byte", "b7", "b6", "b5", "b4", "b3", "b2", "b1", "b0"]) {
      head.appendChild(el("th", null, h));
    }
    t.appendChild(head);
    return t;
  };

  const stations = [...groups.keys()].sort((a, b) => a - b);
  for (const st of stations) {
    /* The group heading names the module as well as the number: the number is
       what is on the dials, the model is what is in the cabinet, and matching
       one to the other is the entire job. */
    const model = stationModel(tags, st);
    s.appendChild(el("div", "grouphead",
      "STATION " + fmtStation(st) + (model ? "   ·   " + model : "")));
    const t = mkTable();
    for (const i of groups.get(st)) {
      const tag = `pci1203.di${i}`;
      const chan = tags.get(tag + ".chan");
      const tr = el("tr");
      tr.appendChild(el("th", null,
        (typeof chan === "number" ? "b" + chan + "  " : "") + "(port " + i + ")"));
      tr.appendChild(tdOf(tags, tag, "hex2"));
      byteLamps(tr, tags, tag);
      t.appendChild(tr);
    }
    s.appendChild(wrapScroll(t));
  }

  if (stations.length === 0) {
    s.appendChild(banner("warn",
      only !== null ? "no DI bytes are attributed to station " + fmtStation(only)
                    : "no DI byte is attributed to any station",
      "The master's IO map (Acm_DevUpLoadMapInfo) returned nothing for these ports. " +
      "The bytes themselves may still be readable — see the unattributed table below — " +
      "but nothing on this page can say which module they belong to."));
  }

  /* Unattributed bytes are shown SEPARATELY and labelled as such, never folded
     into a station's group. A byte whose owner is unknown is a real byte; a
     byte filed under the wrong module is the failure this whole attribution
     exists to prevent. */
  if (only === null && orphans.length > 0) {
    s.appendChild(el("div", "grouphead",
      "UNATTRIBUTED — the IO map does not say which module owns these"));
    const t = mkTable();
    for (const i of orphans) {
      const tag = `pci1203.di${i}`;
      const tr = el("tr");
      tr.appendChild(el("th", null, "port " + i));
      tr.appendChild(tdOf(tags, tag, "hex2"));
      byteLamps(tr, tags, tag);
      t.appendChild(tr);
    }
    s.appendChild(wrapScroll(t));
  }

  return s;
}

/** The model name of the station whose ALIAS is `station`, or "" if no
 *  published slot carries that alias.
 *
 *  ⚠ AN EARLIER DRAFT RETURNED A SENTINEL TAG NAME for the not-found case, and
 *  the typo gate in probe_pci1203.mjs caught it immediately: that check asserts
 *  every tag name this file reads exists on a real captured wire, and a
 *  synthesised name can never exist. Returning the VALUE instead of a
 *  fabricated tag name is both simpler and keeps the gate meaningful -- a name
 *  in the read-set should always be one somebody could have typed wrong, never
 *  one this file invented.
 *    ⓘ And the sentinel could not even be NAMED in this comment: the gate
 *    harvests quoted tag-shaped strings from the SOURCE TEXT, comments
 *    included, so writing it here re-created the failure it describes. That is
 *    the gate being right, not over-eager -- a tag name in a comment is
 *    exactly how a stale name survives a refactor. */
/* ---------------------------------------------------------------------------
   AI(W906-1203CTL-18) 20260911: EVERY SLAVE CARRYING THIS ALIAS, not the first.

   ⚠ THE USER CAUGHT THIS: "卡片你是讀取卡片上所設的ID嗎? 怎又跟之前不一樣了?"
   Measured answer, from the card's own IO map dumped at Open():

     offsets  8..39   Name = 0x002,0x011,0x012,0x050..0x054   4 bytes each
                      -> the ECx-P32-HON 32DI racks. These ARE the rotary-dial
                         aliases, and they are unchanged from 20260910.
     offsets 60..119  Name = 0x00e,0x001,0x003,0x029,0x00a,
                             0x01e,0x07c,0x099                8 bytes mostly
                      -> the SERVO DRIVES' PDO region, which did not exist on
                         20260910 because ring 0 was empty.

   NO STATION ON EITHER RING HAS ALIAS 41, 30, 124 OR 153. So those Names are
   not station numbers at all, and rendering them as "STATION 0x29" invented
   cards that do not exist -- the precise failure ("a real value under a wrong
   name") this whole page was built to prevent, committed by this page.

   So a Name is trusted as a station ONLY when a discovered slave actually
   carries it as its alias, and the lookup returns ALL matches because the
   SubDevice IDs currently conflict: alias 1..8 exist on BOTH rings. Picking
   the first match would print a junction's model beside a servo's bytes. */
/*  AI(W906-1203ALM-14) 20260914: ⚠ SECOND PLACE KEYED ON THE DEAD `alias`.
    stationCards() was fixed earlier the same day and THIS one was missed, so
    the symptom only half went away: the rail listed cards again, but every
    card header rendered without its model name and every model-dependent
    decision silently took the "unknown" branch -- including the new
    SERVOPACK test, which is why a drive's PDO bytes were still being offered
    as clickable coils after the fix that was supposed to stop that.
    Grepping for the field name, not just fixing the function I was looking at,
    is what would have caught it the first time. */
function stationMatches(tags, station) {
  const out = [];
  for (let i = 0; i < SLAVE_SLOTS; i++) {
    if (tags.get(`pci1203.slave${i}.addr`) === station) {
      const n = tags.get(`pci1203.slave${i}.name`);
      out.push({ slot: i,
                 ring: tags.get(`pci1203.slave${i}.ring`),
                 profile: tags.get(`pci1203.slave${i}.profile`),
                 name: (typeof n === "string") ? n : "" });
    }
  }
  return out;
}

/** "" when no discovered station carries this alias -- which now MEANS
 *  something (see stationMatches) rather than merely "the name was missing". */
function stationModel(tags, station) {
  const m = stationMatches(tags, station);
  if (m.length === 0) return "";
  if (m.length === 1) return m[0].name;
  /* Ambiguous under the ID conflict. Name both rather than choose. */
  return m.map(x => `ring ${x.ring}: ${x.name}`).join("   ·or·   ");
}

/*  AI(W906-1203RAIL-1) 20260915: is the station at this address a motion DRIVE?
    CoE 1000h Device Type's low word is the CiA profile; 402 = drives and
    motion. ⚠ The name test is kept beside it, not replaced: a module that does
    not answer 1000h has no profile tag, and an unknown profile must not
    reclassify something that was already being described correctly.
    ⓘ true when ANY match at this address is a drive -- under the SubDevice ID
    conflict one address can name a station on each ring, and a drive on either
    is enough for the PDO wording to be right. */
/*  AI(W906-1203RING-1) 20260922: ⚠⚠ `ring` IS NOT OPTIONAL WHEN A STATION
    NUMBER IS SHARED, AND THIS FUNCTION WAS THE LAST THING STILL BLOCKING THE
    OPERATOR. User: "為啥我還是不能控? 範例程式都可以控".

    The old comment above it reasoned: "on a conflict one address can name a
    station on each ring, and a drive on EITHER is enough for the PDO wording to
    be right." That is correct for a WORDING. It is wrong for an AFFORDANCE.

    Station 1 is a SERVOPACK on ring 0 and an ECx-C32-HON 32DO on ring 1. Under
    the old rule stationIsDrive(1) was true, so doBitControl rendered EVERY byte
    filed under station 1 read-only -- including the 32DO's four bytes, which
    are real coils on a real output card. The operator could see the module, and
    every single one of its bits was a plain number with no way to click it.
    That is the "不能控" -- not a refusal, not an error, just an affordance that
    was never offered, which is why nothing on screen explained it.

    ⚠ With a ring, the question is answerable exactly: is the device AT THIS
    RING AND THIS STATION a drive? Callers that genuinely mean "either ring"
    (the prose that explains what PDO bytes are) pass no ring and keep the old
    behaviour. */
function stationIsDrive(tags, station, ring) {
  const m = stationMatches(tags, station);
  for (const x of m) {
    if (typeof ring === "number" && ring >= 0 &&
        typeof x.ring === "number" && x.ring !== ring) continue;
    if (x.profile === 402) return true;
    if (/SERVOPACK/i.test(x.name || "")) return true;
  }
  return false;
}

/* ---------------------------------------------------------------------------
   AI(W906-MW2b) 20260910: DIGITAL OUTPUT READ-BACK.

   ⚠ THE OLD VERSION OF THIS FILE SAID "there is no DO here and there must not
   be". That was true and is not any more, and the correction matters more than
   the feature: the observer now calls Acm_DaqDoGetByte, which READS what the
   card reports it is currently driving. The forbidden family is
   Acm_DaqDoSet*, which is still absent and still caught by
   tools/pci1203_readonly_gate.ps1.

   THE LINE IS Get vs Set, NOT Di vs Do. Without the read-back a dark lamp
   means either "that coil is off" or "nobody looked", and on a handler those
   two lead to opposite repairs.
   --------------------------------------------------------------------------- */
export function sectionDO(tags) {
  const armed = controlArmed(tags);
  const dry   = controlDry(tags);
  const s = el("section");
  s.appendChild(el("h2", null,
    armed ? "IO — digital output (CLICK A BIT TO TOGGLE IT)"
          : "IO — digital output (READ-BACK ONLY)"));

  if (!armed) {
    /* ⚠ THE WRITE FAMILY IS NAMED HERE ON PURPOSE, and the naming survived a
       rewrite that nearly dropped it: probe_pci1203.mjs asserts this banner
       says "Acm_DaqDoSet" in as many words, because naming what is absent is
       what stops it drifting back in. What CHANGED on 20260911 is only WHERE it
       is forbidden -- it is no longer forbidden everywhere, it is forbidden in
       the OBSERVER, and it lives in a different file with a different gate. */
    s.appendChild(banner("info",
      "these are reads, not controls — nothing on this page can drive an output",
      "Acm_DaqDoGetByte reports what the card says it is currently driving. The write " +
      "family — Acm_DaqDoSetBit / Acm_DaqDoSetByte — is absent from the OBSERVER's " +
      "allowlist and tools/pci1203_readonly_gate.ps1 fails the build if it appears in " +
      "EtherCAT/Pci1203Monitor.cpp. It lives in a separate translation unit, " +
      "EtherCAT/Pci1203Control.cpp, which is NOT armed in this build — so there is " +
      "nothing on this page that can drive an output. " +
      "Reading an output is what tells 'that coil is off' apart from 'nobody looked'."));
  } else {
    /* AI(W906-1203CTL-9) 20260911: the bits are buttons now. The banner says
       what one press sends, in the vendor's own spelling, because the flat
       channel number that goes on the wire is NOT the (port, bit) pair shown in
       the table -- and an operator comparing this page with Common Motion
       Utility's 數位輸出 panel is reading flat channel numbers there. */
    s.appendChild(banner(dry ? "ok" : "bad",
      dry ? "DRY RUN — clicking a bit records the call it would make and issues nothing"
          : "LIVE — clicking a bit ENERGISES OR DE-ENERGISES A REAL OUTPUT",
      "Each button shows the CURRENT read-back value and sends the opposite, the same " +
      "toggle the vendor's own EthcatDO example performs. What goes on the wire is " +
      "Acm_DaqDoSetBit(chan, value) with a FLAT channel index — chan = port*8 + bit — " +
      "which is the numbering Common Motion Utility's output panel also uses. A bit " +
      "whose byte could not be read is NOT clickable: a toggle that does not know the " +
      "current state cannot know what to send."));
  }

  const t = el("table", "grid");
  const head = el("tr");
  for (const h of ["port", "byte", "chan", "b7", "b6", "b5", "b4", "b3", "b2", "b1", "b0"]) {
    head.appendChild(el("th", null, h));
  }
  t.appendChild(head);
  for (let i = 0; i < doPortCount(tags); i++) {
    const tag = `pci1203.do${i}`;
    const tr = el("tr");
    tr.appendChild(el("th", null, "do" + i));
    tr.appendChild(tdOf(tags, tag, "hex2"));
    /* The flat channel range this row covers, shown because it is what the
       vendor API is actually given and what the Utility displays. */
    tr.appendChild(el("td", "val", `${i * 8}..${i * 8 + 7}`));
    if (armed) {
      for (let b = 7; b >= 0; b--) tr.appendChild(doBitControl(tags, i, b));
    } else {
      byteLamps(tr, tags, tag);
    }
    t.appendChild(tr);
  }
  s.appendChild(wrapScroll(t));
  return s;
}

/** AI(W906-FW-1e) 20260908: WHICH BUILD AM I LOOKING AT.
 *
 *  The `build.*` family is compile-time facts, published always-non-null
 *  (WebBridgeTags.cpp, and gated by tests/test_wb_tags.cpp's count+non-null
 *  assertions). It is shown HERE, on the diagnostic page, because two of its
 *  ten answers are things this tree has repeatedly had to dig for:
 *
 *    build.safeDoorInterlock -- CLAUDE.md records that the SIM and non-SIM
 *      builds have IDENTICAL exe filenames and differ in whether the
 *      safety-door interlock is compiled in ("接 web START 按鈕之前要先確認
 *      自己在哪一條線上"). Answering that used to need an object-file symbol
 *      dump; now it is on the wire.
 *    build.stamp -- __DATE__ " " __TIME__. This repo keeps paying for stale
 *      executables that run and look correct: an F5 lane that "worked" only
 *      because its binary predated the source by six hours, and a killed link
 *      that left 27 truncated exes still printing ALL PASS. A build stamp makes
 *      "am I looking at what I just built" answerable from the screen.
 *
 *  ⚠ Rendered with the SAME three-state cell as everything else, so a build
 *  tag that is absent reads "n/a" (older publisher) rather than blank. These
 *  must never show "---": a compile-time fact that came back null would mean
 *  the category's promise is broken, and the C++ test asserts that too.
 */
export function sectionBuild(tags) {
  const c = (t, f) => cellOf(tags, t, f);
  const s = el("section");
  s.appendChild(el("h2", null, "Build — which binary is this"));

  const lane = tags.get("build.simulte");
  if (lane === true) {
    s.appendChild(banner("warn",
      "SIM BUILD — the safety-door interlock is COMPILED OUT",
      "build.simulte is true, so ckernel.cpp's `#ifndef SOFT_SIMULTE` block is not in " +
      "this binary and pressing START does NOT check the doors. That is golden's own SIM " +
      "design, not a port defect — but the SIM and non-SIM executables have IDENTICAL " +
      "FILENAMES, so this line is the only thing on screen that tells them apart."));
  } else if (lane === false) {
    s.appendChild(banner("ok",
      "production build — the safety-door interlock IS compiled in",
      "build.simulte is false, so ckernel.cpp:1052's CheckSafeDoorIsClosed() call is in " +
      "this binary: START with a door open sets SystemStart=false and stops all motors. " +
      "Verified 20260908 by object-file symbol table on both lanes."));
  }

  s.appendChild(kvTable(tags, [
    ["SOFT_SIMULTE",          c("build.simulte", fmtBool), "true = SIM arm; doors NOT checked"],
    ["safe-door interlock",   c("build.safeDoorInterlock", fmtBool), "derived from the same macro, at the same compile"],
    ["INSTALL_1203_MONITOR",  c("build.install1203Monitor", fmtBool), "this page's whole subject"],
    ["WB_PUMP_WITH_CONFIG",   c("build.pumpWithConfig", fmtBool), "off = no ini is read or written"],
    ["build stamp",           c("build.stamp"), "__DATE__ __TIME__ — is this the exe you just built?"],
    ["bitness",               c("build.bits", v => v + "-bit"), "from sizeof(void*), so it cannot disagree with the binary"],
    ["compiler",              c("build.compiler"), "6.3.0 = oracle lane (numbers comparable with BCB6); 16.2.0 = not"],
    ["C++ standard",          c("build.cxx"), "201402 = C++14 (non-oracle), 201703 = C++17 (oracle)"],
    ["machine alias",         c("build.alias"), "MachineType.h ALIAS — available before any ini is read"],
    ["ATC head count",        c("build.atcHeadCount"), ""],
  ]));
  return s;
}

/* ===========================================================================
   AI(W906-1203CTL-19) 20260911: THE TWO-PANE SHELL -- left rail, right pane.

   User requirement: "我希望可以跟BCB介面一樣 / 左邊是可以選擇卡片 / 右邊是操作
   視窗". That is the BCB6 client's TreeView + TabControl shape, and the reason
   it is the right shape here is the same reason it was there: this fieldbus now
   has 27 stations, 93 DI bytes, 69 DO bytes and 16 axes, and ONE flat scrolling
   report makes the operator hunt for the module in front of them.

   The rail is NAVIGATION ONLY. Every entry is an <a href="?..."> -- no state in
   the document, no listener in the view, so the whole thing stays pure and the
   probe can execute it. It also means a card selection can be bookmarked or
   read out to somebody standing at the machine, which a tree widget cannot.

   ⚠ THE RAIL LISTS WHAT THE CARD REPORTS, NOT WHAT THE MAP CLAIMS. Only
   stations the slave scan actually discovered appear, so the phantom "stations"
   the IO map's servo region produced (0x029, 0x01e, 0x07c, 0x099 -- no module
   on either ring carries those aliases) are NOT offered as cards. See
   stationMatches.
   =========================================================================== */

/** Which stations have DI bytes attributed to them AND are real discovered
 *  modules. Returns [{station, ports, model, ambiguous}] in station order. */
/*  AI(W906-1203CTL-25) 20260911: BUILT FROM THE STATIONS, NOT FROM THE IO MAP.
    User: "卡片的ID 是否都是先讀取 該站的ID".

    ⚠ THE DIRECTION MATTERS AND IT USED TO BE BACKWARDS. The first version
    walked the IO map, took the station number out of its Name field, and THEN
    checked a station existed with that alias. Same list, opposite authority:
    the number on screen originated in a map entry and was merely corroborated.

    Now the list is the SCANNED STATIONS. Every entry's ID is
    pci1203.slaveN.alias -- ESC register 0x0012, which the slave controller
    loads from THE MODULE'S OWN EEPROM at power-up and which on these ECx racks
    is the x16/x1 rotary pair. So the number beside a card is the number an
    operator reads off that card's dials, sourced from that card.

    The IO map is then used for what it is actually authoritative about: WHICH
    FLAT BYTES belong to that station. A station with no bytes is dropped,
    because the user asked to see IO ("我只想看到IO") and a junction with no
    inputs is not something to select.
      ⓘ This also removes the phantom cards by construction rather than by
      filtering: a Name like 0x029 in the servo PDO region can no longer create
      an entry, because entries only ever come from scanned stations. */
/*  AI(W906-1203CARDS-1) 20260922: ⚠⚠ KEYED ON (RING, STATION) NOW, AND KEYING
    ON THE STATION ALONE IS WHY A REAL OUTPUT CARD WAS MISSING FROM 輸出.
    User: "我圖片上還是沒有出現01阿".

    This is the SAME defect already fixed in the DO byte grouping and in
    stationIsDrive on 20260922, in a third place I did not check. Station
    numbers are unique only within a ring: station 1 is a SERVOPACK on ring 0
    and an ECx-C32-HON 32DO on ring 1.

    With one key per station the two collapsed into ONE card:
      * doBy.get(1) returned the servo's 8 RxPDO bytes AND the 32DO's 4 coil
        bytes as one list of 12;
      * whichever slave index came first won the entry, and the second was
        merged away with `ambiguous = true` and `continue`;
      * the merged card had a SERVOPACK among its matches, so the rail's
        drive/IO split filed it under drives -- and the 32DO never appeared in
        輸出 at all. The group header even read "ECx-C32-HON ... x1" while two
        are fitted.

    ⚠ The operator could see the module in the collision list at the bottom and
    nowhere else. That is the same shape as the missing rail LINK fixed
    earlier: not a refusal, not an error, just absence -- and absence is the one
    failure mode a page cannot explain.
    ⓘ Byte lists are keyed the same way, so a card's ports are only its own. */
function stationCards(tags) {
  const key = (ring, st) => `${typeof ring === "number" ? ring : "?"}/${st}`;
  const diBy = new Map(), doBy = new Map();
  for (let i = 0; i < diPortCount(tags); i++) {
    const st = diStationOf(tags, i);
    if (st === null) continue;
    const k = key(tags.get(`pci1203.di${i}.ring`), st);
    if (!diBy.has(k)) diBy.set(k, []);
    diBy.get(k).push(i);
  }
  for (let i = 0; i < doPortCount(tags); i++) {
    const st = tags.get(`pci1203.do${i}.station`);
    if (typeof st !== "number") continue;
    const k = key(tags.get(`pci1203.do${i}.ring`), st);
    if (!doBy.has(k)) doBy.set(k, []);
    doBy.get(k).push(i);
  }

  /*  AI(W906-1203ALM-13) 20260914: ⚠ KEYED ON `addr`, NOT `alias`, AND THAT IS
      WHY THE CARD LIST WENT EMPTY. User: "IO 那邊之前不是都有分卡片 在左邊可以
      讓我選 ... 之前都有說過 怎都不見了".

      `alias` is the module's rotary-switch number, read from ESC register
      0x0012 -- and that read was gated off on 20260911 because it STOPPED THE
      RING. So alias has been absent ever since, this loop skipped every slave
      on `typeof alias !== "number"`, and the rail rendered "IO 卡片 CARDS (0)".
      The rail code was never broken; its join key had been switched off
      underneath it. A feature removed by a change somewhere else, silently.

      MEASURED 20260914 on the live wire before changing anything:
          slaveN.alias   0 of 24 present slaves carry one
          slaveN.addr    24 of 24 carry one
          DI stations    15 distinct, EVERY one matches a slave addr
          DO stations    10 distinct, every one matches
      So `addr` -- the ESC 0x0010 configured address, which the scan reads
      anyway -- is both available and the same numbering the DI/DO map already
      uses. Joining on it is not a workaround; it is the key that matches.

      ⚠ The two are NOT interchangeable in general: addr is what the master
      assigned, alias is what the dials say, and on this machine's IO modules
      they disagree. The rail therefore now labels cards by the ADDRESSING
      address, which is the same thing the axis rail was settled on
      (AI(W906-1203CTL-39): "你左邊卡號名稱應該要以 定位位址 命名"). */
  const seen = new Map();          // "ring/addr" -> card
  for (let i = 0; i < SLAVE_SLOTS; i++) {
    if (tags.get(`pci1203.slave${i}.present`) !== true) continue;
    const alias = tags.get(`pci1203.slave${i}.addr`);
    if (typeof alias !== "number") continue;
    const rg  = tags.get(`pci1203.slave${i}.ring`);
    const k   = key(rg, alias);
    const di  = diBy.get(k) || [];
    const dos = doBy.get(k) || [];
    if (di.length === 0 && dos.length === 0) continue;   // no IO -> not a card

    const name = tags.get(`pci1203.slave${i}.name`);
    const prev = seen.get(k);
    if (prev) {
      /*  ⚠⚠ SAME RING AND SAME ADDRESS -- and WHICH OF THE TWO NAMES THE CARD
          DECIDES WHETHER ITS BYTES ARE SHOWN AT ALL.
          User: "我圖片上還是沒有出現01阿".

          On this ring the pair is always an ECAT-2515 junction and a real
          module (junction @pos0 vs ECx-C32-HON 32DO @pos12 at address 1;
          junction @pos1 vs ECx-P32-HON 32DI @pos13 at address 2). The junction
          has the lower slave index, so it won the entry and the card was
          labelled with ITS name -- and buildRail then filtered the whole card
          out with `isRelay`, on the correct rule that passive infrastructure is
          not a destination for looking at field IO. The 32DO's four bytes of
          real coils went out with it. The operator could see the module in the
          collision list at the bottom and nowhere else, and the group header
          read "ECx-C32-HON ... x1" while two are fitted.

          ★ A JUNCTION CARRIES NO PROCESS DATA -- confirmed, the card's IO map
          attributes no DI or DO byte to an ECAT-2515 anywhere on this ring. So
          when bytes ARE attributed to this (ring, address), they belong to the
          other device, and the card must take its identity from that one.
          ⚠ Identity only. `ambiguous` stays set and the collision list still
          lists it, because two devices really do answer here and that remains
          worth saying. */
      const infra = (n) => /junction|hub|splitter|repeater/i.test(n || "");
      if (infra(prev.model) && !infra(name)) {
        prev.model    = (typeof name === "string") ? name : "";
        prev.profile  = tags.get(`pci1203.slave${i}.profile`);
        prev.state    = tags.get(`pci1203.slave${i}.state`);
      }
      prev.ambiguous = true;
      prev.matches.push({ ring: rg, name });
      continue;
    }
    seen.set(k, {
      station: alias, ports: di, doPorts: dos,
      model: (typeof name === "string") ? name : "",
      ambiguous: false,
      /*  AI(W906-1203RAIL-1) 20260915: the CiA profile from CoE 1000h, so the
          groups below can tell a DRIVE from an IO card by what the module SAYS
          IT IS. 402 = drives and motion, 401 = generic IO. undefined when the
          module did not answer -- which is a third state, not a "no". */
      profile: tags.get(`pci1203.slave${i}.profile`),
      ring: tags.get(`pci1203.slave${i}.ring`),
      state: tags.get(`pci1203.slave${i}.state`),
      matches: [{ ring: tags.get(`pci1203.slave${i}.ring`), name }],
    });
  }
  const out = [...seen.values()];
  //AI(W906-1203CARDS-1) 20260922: ring first, then station. Two cards can now
  //  share a station number, and sorting on the number alone would interleave
  //  them unpredictably -- the operator reads this list top to bottom against
  //  the cabinet, and the ring is the outer grouping in the vendor tree too.
  out.sort((a, b) => (a.ring - b.ring) || (a.station - b.station));
  return out;
}

/*  AI(W906-1203RAIL-1) 20260915: the EtherCAT slave state, as a word.
    Values are AdvMotDrv.h's EC_SLAVE_STATE_* (UNKNOWN 0x00, INIT 0x01,
    PREOP 0x02, SAFEOP 0x04, OP 0x08), and they are a BITFIELD in the sense that
    the ESC can report an error flag alongside -- so the low nibble is what is
    decoded and anything else is shown raw rather than guessed at.
    ⚠ SAFEOP says "inputs only": in SAFEOP every command returns SUCCESS and
    changes nothing while positions keep updating. That is worth a word on the
    screen, because it is the single most misleading state this ring can be in. */
function ecStateText(v) {
  if (typeof v !== "number") return "state ---";
  const s = v & 0x0F;
  const name = { 0x00: "UNKNOWN", 0x01: "INIT", 0x02: "PREOP",
                 0x04: "SAFEOP", 0x08: "OP" }[s];
  if (!name) return `state 0x${v.toString(16).toUpperCase()}`;
  const why = { "PREOP": "（不交換過程資料，開不出軸）",
                "SAFEOP": "（只收輸入，指令會回成功但不動作）",
                "INIT": "（尚未組態）" }[name] || "";
  return name + why;
}

function railLink(label, href, opts) {
  const o = opts || {};
  const a = el("a", "railitem" + (o.on ? " railon" : ""));
  a.setAttribute("href", href);
  a.appendChild(el("div", "railmain", label));
  /*  AI(W906-1203KEEP-1) 20260916: `sub` may be a NODE, not only a string.
      The axis rail's subtitle carries the EtherCAT state, and while that was
      plain text the only way to keep it current was to put the state into
      railKey() -- which made every MOVE rebuild the whole page and wipe the
      operator's typed distance. As a node it can hold a [data-k] span that
      refresh() updates in place, so the state stays live and the shape stops
      depending on it. */
  if (o.sub) {
    if (o.sub instanceof Node) {
      const d = el("div", "railsub");
      d.appendChild(o.sub);
      a.appendChild(d);
    } else {
      a.appendChild(el("div", "railsub", o.sub));
    }
  }
  return a;
}

/* ===========================================================================
   AI(W906-1203CTL-23) 20260911: THE STATUS STRIP -- three facts, one line each,
   on every pane.

   User, after using it: "我怎沒看到有一直讀取? / 裡面也太多資訊了 / 我只需要
   IO 可以讀 / IO可以寫入 / 馬達按鈕 依照Common Motion Utility 介面 可以動作"
   and "如果軸卡還在讀取 我需要有提示 還在讀取中".

   So everything explanatory moved to a Diagnostics pane, and what stays is the
   three things an operator cannot work without:

     1. IS IT STILL READING?  pci1203.pollCount increments every IO poll (200 ms).
        Showing the NUMBER is the honest liveness indicator: a spinner spins
        whether or not data is arriving, and on this machine the inputs can sit
        unchanged for minutes -- which is exactly what made a live page look
        dead. Measured 20260911: 20 polls in 6 s, DI unchanged, ax0.actPos
        moving. The feed was fine; nothing on screen said so.
     2. IS IT STILL OPENING?  pci1203.phase. Opening the card takes up to ~10 s
        of ring scan and axis sweep, and it used to look like a hang.
     3. DOES A CLICK DO ANYTHING?  armed / dry / live, plus the operator token.
   =========================================================================== */
export function statusBar(tags, opts) {
  const o = opts || {};
  const bar = el("div", "statusbar");

  /* --- 1. still opening? The loudest thing on the page while it is true. --- */
  const phase = tags.get("pci1203.phase");
  const opening = (typeof phase === "string" && phase !== "open" &&
                   phase !== "not started");
  if (opening) {
    const b = el("div", "statusbusy");
    b.appendChild(el("span", "spin", "◐"));
    b.appendChild(el("span", null, "讀取中 — " + phase));
    bar.appendChild(b);
  }

  /* --- 2. still reading? --- */
  const polls = tags.get("pci1203.pollCount");
  const item = el("div", "statusitem");
  item.appendChild(el("span", "statuslabel", "讀取"));
  if (typeof polls === "number") {
    /*  AI(W906-1203CTL-27) 20260911: bound, and keepcls so refresh() does not
        overwrite the colour with the generic value class. This counter is the
        page's proof of life, so it must keep moving WITHOUT a rebuild --
        rebuilding is exactly what made every button unclickable. */
    const n = el("span", "statuslive", String(polls));
    n.dataset.k = "pci1203.pollCount";
    n.dataset.keepcls = "1";
    item.appendChild(el("span", "statuslive", "● "));
    item.appendChild(n);
    item.appendChild(el("span", "statushint", "次 — 卡片每 0.2 秒讀一次"));
  } else {
    item.appendChild(el("span", "statusdead", "沒有在讀"));
  }
  bar.appendChild(item);

  /* --- 3. what does a click do --- */
  const armed = controlArmed(tags);
  const dry   = controlDry(tags);
  const held  = !!o.hasControl;
  const ctl = el("div", "statusitem");
  ctl.appendChild(el("span", "statuslabel", "輸出/馬達"));
  if (!armed) {
    ctl.appendChild(el("span", "statusdead", "未武裝 — 按鈕不會出現"));
  } else if (dry) {
    ctl.appendChild(el("span", "statusdry", "DRY RUN — 只記錄，不發出"));
  } else {
    ctl.appendChild(el("span", "statuslivecmd", "● LIVE — 會真的動"));
  }
  if (armed) {
    ctl.appendChild(el("span", held ? "statuslive" : "statusdead",
                       held ? "已取得操作權" : "未取得操作權"));
    ctl.appendChild(held
      ? cmdButton("釋放操作權", "control.release", "", { cls: "warnbtn small" })
      : cmdButton("取得操作權", "control.acquire", "", { cls: "small" }));
  }
  bar.appendChild(ctl);
  return bar;
}

/* ---------------------------------------------------------------------------
   AI(W906-1203CTL-23) 20260911: ONE CARD -- its inputs and its outputs, and
   nothing else. This is the pane the rail's card entries open, and it is
   deliberately the smallest thing on the site: a station heading, the DI bytes
   that belong to it, and the DO bytes that belong to it.

   ⚠ THE OUTPUTS ARE ON THE SAME PANE AS THE INPUTS, which was not possible
   until 20260911 because DO bytes had no station at all. MapType 0 turned out
   to be the OUTPUT map once an output module was actually fitted. An operator
   interlocking a coil against a sensor should not have to change page.
   --------------------------------------------------------------------------- */
export function sectionStation(tags, station) {
  const s = el("section");
  const model = stationModel(tags, station);
  s.appendChild(el("h2", null,
    "卡片 " + fmtStation(station) + (model ? "  ·  " + model : "")));

  /*  AI(W906-1203RING-1) 20260922: ⚠⚠ A STATION NUMBER IS NOT UNIQUE ON ITS
      OWN -- IT IS UNIQUE WITHIN A RING, and this pane merged two devices.
      User: "為啥我還是不能控? 範例程式都可以控".

      MEASURED on the live ring: station 0x001 is a SERVOPACK on ring 0 (eight
      RxPDO bytes) AND an ECx-C32-HON 32DO on ring 1 (four bytes = 32 channels).
      The card's IO map returns both under the same Name; its `Index` field is
      the ring and this whole module had been ignoring it. So this pane put a
      drive's command image and a real output card's coils in one table, under
      one heading, with one set of clickable bits.

      ⚠ THE FIX HERE IS TO SPLIT, NOT TO PICK. There is no ring in the URL --
      "?station=1" genuinely names two things -- and choosing one would hide the
      other with nothing on screen to say so. Grouping by ring shows everything
      the card attributes to this number and says which ring each group is on,
      which is also what the vendor Utility's tree does. */
  const byRing = new Map();      // ring -> { di: [], do: [] }
  const slot = (r) => {
    const k = (typeof r === "number") ? r : -1;
    if (!byRing.has(k)) byRing.set(k, { di: [], do: [] });
    return byRing.get(k);
  };
  for (let i = 0; i < diPortCount(tags); i++) {
    if (diStationOf(tags, i) !== station) continue;
    slot(tags.get(`pci1203.di${i}.ring`)).di.push(i);
  }
  for (let i = 0; i < doPortCount(tags); i++) {
    if (tags.get(`pci1203.do${i}.station`) !== station) continue;
    slot(tags.get(`pci1203.do${i}.ring`)).do.push(i);
  }
  /*  The pane below still renders one DI list and one DO list. When the station
      lives on exactly one ring -- which is every station on this machine except
      the three colliding numbers -- that is unchanged behaviour. When it does
      not, each ring gets its own heading and its own tables. */
  const rings = [...byRing.keys()].sort((a, b) => a - b);
  const multi = rings.length > 1;
  const diPorts = multi ? [] : (rings.length ? byRing.get(rings[0]).di : []);
  const doPorts = multi ? [] : (rings.length ? byRing.get(rings[0]).do : []);
  if (multi) {
    s.appendChild(el("div", "errwarn",
      `⚠ 這個站號在 ${rings.length} 個環上都有裝置 —— 站號只在同一個環裡才唯一。` +
      `下面依「環」分開列出，不要把它們當成同一顆模組。` +
      `（這也是 Common Motion Utility 的樹狀圖分開顯示它們的原因。）`));
    for (const rg of rings) {
      const g = byRing.get(rg);
      s.appendChild(el("h3", null,
        `環 ${rg < 0 ? "?" : rg}  ·  站 ${fmtStation(station)}` +
        `　（${g.di.length} DI / ${g.do.length} DO bytes）`));
      if (g.di.length) s.appendChild(wrapScroll(ioTable(tags, g.di, "di", false)));
      if (g.do.length) {
        const armed = controlArmed(tags);
        s.appendChild(el("div", "errhow2",
          armed ? "OUTPUT（點一下切換）—— 寫入用「環＋站＋站內通道」定址，" +
                  "所以就算站號跟別的環重複也不會打錯。"
                : "OUTPUT（唯讀 — 未武裝）"));
        s.appendChild(wrapScroll(
          ioTable(tags, g.do, "do", armed, declaredDoPoints(tags, station))));
      }
    }
    return s;
  }

  /*  AI(W906-1203ALM-14) 20260914: SAY WHAT THESE BYTES ARE.
      User: "模組有分 output 和 input 你每個out 怎都有一組input?"

      Both questions have one answer: an EtherCAT slave can carry input AND
      output process data, and most of the stations in this list are not
      digital IO modules at all. Measured attribution, from the card's own
      Acm_DevUpLoadMapInfo:
          ECx-P32-HON 32DI (80..84)   4 DI bytes, 0 DO   real field inputs
          ECAT-VC4-ODM1    (160,161)  9 DI bytes, 3 DO   3 DO = real outputs,
                                                         9 DI = its status PDO
          SGDXW/SGDXS SERVOPACK       8 DI, 8 DO         entirely PDO
      The VC4-ODM1's nine "input" bytes read 0x38 on every other byte -- a
      16-bit status word pattern, not field wiring. Calling that "INPUT (讀取)"
      beside a 32DI module's real inputs put two different things under one
      heading, which is what made the question necessary.

      ⚠ The judgement is made from the module's OWN reported name, not from
      guesswork about address ranges -- the same rule the rail's servo split
      already uses. A module that advertises "DI" has field inputs; anything
      else has an input PDO whose meaning belongs to that module's manual. */
  /*  AI(W906-1203RAIL-1) 20260915: ⚠ THIS WAS /SERVOPACK/i AND IT IS THE THIRD
      PLACE THAT GATE LIVED. On a SW3D-680 -- a CiA 402 stepper drive -- the
      page said "這個模組送回主站的輸入過程資料" where it should have said
      "驅動器", and the OUTPUT block said "這個模組不是純數位輸出卡" as though a
      motion drive were an unusual sort of IO card. That is the very pane the
      user screenshotted. Now asks the module: 1000h low word 402 = a drive. */
  const isServo = stationIsDrive(tags, station);
  const saysDI  = /\d+\s*DI\b/i.test(model || "");
  const saysDO  = /\d+\s*DO\b/i.test(model || "");

  if (diPorts.length) {
    s.appendChild(el("h3", null,
      saysDI ? "INPUT  (讀取 — 實體輸入點)"
             : "輸入 PDO  (讀取 — 模組的狀態位元組，不是現場輸入點)"));
    if (!saysDI) {
      s.appendChild(el("div", "errhow2",
        `這 ${diPorts.length} 個位元組是 ${isServo ? "驅動器" : "這個模組"}` +
        `送回主站的輸入過程資料（TxPDO）。EtherCAT 的從站可以同時有輸入和輸出` +
        `過程資料，所以一張輸出模組一樣會有一組輸入位元組 —— 那是它的狀態回饋，` +
        `不是可以接線的輸入點。逐位元的意義要看該模組自己的手冊。`));
    }
    s.appendChild(wrapScroll(ioTable(tags, diPorts, "di", false)));
  }
  if (doPorts.length) {
    const armed = controlArmed(tags);
    s.appendChild(el("h3", null,
      isServo ? "輸出 PDO  (唯讀 — 驅動器命令字，不是線圈)"
              : "OUTPUT  " + (armed ? "(點一下切換)" : "(唯讀 — 未武裝)")));
    if (isServo) {
      s.appendChild(el("div", "errhow2",
        "⚠ 這些不是實體輸出。它們是主站每個週期送給驅動器的 RxPDO 命令字。" +
        "點它們不會讓任何線圈動作 —— 寫入會回報成功，然後下一個循環影格就覆蓋掉，" +
        "所以「按了沒反應」是正確行為而不是故障。要動馬達請用左邊的馬達面板。"));
    } else if (!saysDO) {
      s.appendChild(el("div", "errhow2",
        "這個模組不是純數位輸出卡；這些位元組是它的輸出過程資料。" +
        "是否對應到實體點，依該模組的手冊而定。"));
    }
    /*  AI(W906-1203PTS-1) 20260918: HOW MANY OF THESE BITS ARE REALLY WIRED.
        User: "這顆模組 點為數量是不是錯誤了? ... 目前機構上 每顆 只有20點輸出".

        The card attributes 7 flat ports to 0x0B and the table drew 56 toggles.
        Those seven bytes are the module's PROCESS IMAGE, fixed by its ESI/PDO
        configuration; the card has no idea how many valves are plugged in
        (measured: every map entry came back ByteLength 0, ModuleName
        "Unknown"). Only the person who wired it knows, so it is declared in
        D:\HT9045\config\Pci1203Io.ini and published as slaveN.doPoints.

        ⚠ WHY THE EXTRA BITS ARE DIMMED RATHER THAN DELETED. Dropping them would
        make the panel disagree with the card's own map and with Common Motion
        Utility, and an operator checking one against the other would find a byte
        missing with nothing to explain it. They stay, visible, unclickable, and
        labelled -- which is also the honest state of affairs: the bytes exist in
        the process image, they are simply not wired to a coil.
        ⓘ No entry in the INI means "not declared", and then nothing is dimmed.
        Hiding outputs that do exist is the failure that cannot be seen. */
    const pts = declaredDoPoints(tags, station);
    if (typeof pts === "number") {
      s.appendChild(el("div", "errhow2",
        `ⓘ 這一站宣告了 ${pts} 個實體輸出點（Pci1203Io.ini 的 [station${station}] doPoints）。` +
        `卡片的對應表給了 ${doPorts.length} 個位元組 = ${doPorts.length * 8} bit —— ` +
        `那是模組的過程資料寬度，不是接線點數。超過 ${pts} 的位元以灰色顯示且不能點。`));
    } else if (!isServo && doPorts.length * 8 > 32) {
      s.appendChild(el("div", "errwarn",
        `⚠ 卡片的對應表給這一站 ${doPorts.length} 個位元組 = ${doPorts.length * 8} bit，` +
        `但那是過程資料寬度，不一定等於實際接線的點數。` +
        `要讓畫面只顯示真正的點數，請在 D:\\HT9045\\config\\Pci1203Io.ini 加上：\n` +
        `[station${station}]\ndoPoints=<實際點數>`));
    }
    s.appendChild(wrapScroll(ioTable(tags, doPorts, "do", armed && !isServo, pts)));
  }
  if (!diPorts.length && !doPorts.length) {
    s.appendChild(el("div", "emptynote",
      "This station carries no DI or DO byte in the card's own IO map."));
  }
  return s;
}

/** One byte-per-row table. `kind` is "di" or "do"; `clickable` turns DO bits
 *  into toggles. Deliberately the SAME table shape for both, because an
 *  operator reading a card should not have to re-learn the layout halfway
 *  down it. */
function ioTable(tags, ports, kind, clickable, points) {
  const t = el("table", "grid");
  const head = el("tr");
  for (const h of ["", "byte", "chan",
                   "b7", "b6", "b5", "b4", "b3", "b2", "b1", "b0"]) {
    head.appendChild(el("th", null, h));
  }
  t.appendChild(head);
  for (let idx = 0; idx < ports.length; idx++) {
    const p = ports[idx];
    const tag = `pci1203.${kind}${p}`;
    const tr = el("tr");
    const ch = tags.get(tag + ".chan");
    tr.appendChild(el("th", null, (typeof ch === "number" ? "b" + ch : kind + p)));
    tr.appendChild(tdOf(tags, tag, "hex2"));
    /* The FLAT port is always shown. It is what Acm_Daq{Di,Do}GetByte was
       actually asked for, and without it a reading cannot be checked against
       anything -- the same rule the grouped DI view has carried since MW2b. */
    tr.appendChild(el("td", "val", `port ${p}`));
    if (clickable) {
      /*  AI(W906-1203PTS-1) 20260918: the point NUMBER of a bit, so a declared
          count can cut the row off at the right place.
          ⚠ It uses the card's own PortChanID (`chan`) -- the byte's index
          WITHIN its station -- not the position in this array. They agree today
          because the ports come back sorted and contiguous, and they would stop
          agreeing the moment a station's bytes were not contiguous in the flat
          image. Deriving "which point is this" from an array position would
          then dim the wrong bits, silently, on the one panel whose job is to
          say which coil is which. The array index is the FALLBACK, used only
          when the card gave no chan. */
      const byteIdx = (typeof ch === "number") ? ch : idx;
      for (let b = 7; b >= 0; b--) {
        const pointNo = byteIdx * 8 + b;
        if (typeof points === "number" && pointNo >= points) {
          const td = el("td", "val unwired", "·");
          td.title = `bit ${pointNo} — 超過宣告的 ${points} 點，未接線；` +
                     `它在模組的過程資料裡存在，但不對應實體輸出`;
          tr.appendChild(td);
        } else {
          tr.appendChild(doBitControl(tags, p, b));
        }
      }
    } else {
      byteLamps(tr, tags, tag);
    }
    t.appendChild(tr);
  }
  return t;
}

/*  AI(W906-1203PTS-1) 20260918: the declared physical output-point count for a
    station, or null when nobody has declared one.
    ⚠ It looks the number up by STATION across the slave rows rather than taking
    a slave index, because every caller here has the station and the slave index
    is a scan position this campaign has watched change. */
function declaredDoPoints(tags, station) {
  if (typeof station !== "number") return null;
  /*  ⚠ SEVERAL SLAVES CAN CARRY THE SAME addr ON THIS RING -- measured
      20260918: addr 1 appears three times (a SERVOPACK on ring 0, a junction,
      and an ECx-C32-HON on ring 1), addr 10 three times. So "the first row
      whose addr matches" is not a safe answer: it would hand back a
      declaration made for one module while rendering another.
      Taking the first row that actually HAS a value is what makes the common
      case right -- a declaration exists for exactly one of the colliding
      modules, and that is the one meant.
      ⓘ If two colliding modules were BOTH declared the answer really is
      ambiguous, and it is ambiguous in the INI too, because the INI is keyed by
      station like everything else that addresses this ring. That is the same
      collision the parameter writes already refuse over, and fixing it means
      giving the modules unique SubDevice IDs, not guessing here. */
  for (let i = 0; i < SLAVE_SLOTS; i++) {
    if (tags.get(`pci1203.slave${i}.addr`) !== station) continue;
    const v = tags.get(`pci1203.slave${i}.doPoints`);
    if (typeof v === "number" && v > 0) return v;
  }
  return null;
}

export function buildRail(tags, opts) {
  const o = opts || {};
  const nav = el("nav", "rail");
  const view  = o.view || "";
  const st    = (typeof o.station === "number") ? o.station : null;
  const axSel = (typeof o.axis === "number") ? o.axis : null;
  const none  = (st === null && view === "");

  /* AI(W906-1203CTL-23) 20260911: CARDS FIRST, diagnostics last and out of the
     way. The user's complaint was "裡面也太多資訊了" -- so nothing was deleted,
     it MOVED. Everything that explains the machine rather than operating it now
     lives behind one rail entry, where it is still one click away when a screen
     goes blank and somebody needs to know why. */
  /*  AI(W906-1203CTL-25) 20260911: IO CARDS AND SERVO PDO ARE LISTED APART.
      User: "我只想看到IO". Three SERVOPACKs carry bytes in the same flat DI/DO
      image as the IO racks -- real bytes, really theirs -- so hiding them would
      be deleting data. But they are not what an operator means by "IO 卡", and
      mixed into the same list they were the confusing half of it.
      ⚠ THE SPLIT USES THE MODULE'S OWN REPORTED NAME (ADV_SLAVE_INFO.Name),
      not a guess about alias ranges: the card tells us it is a SERVOPACK. */
  const all   = stationCards(tags);
  /*  AI(W906-1203RAIL-1) 20260915: ⚠ A DRIVE IS NOT ONLY A "SERVOPACK".
      User: "這個模組式馬達 你放錯位置了吧".

      This test used to be /SERVOPACK/i on the module's name, and it was right
      about the nine Yaskawas and blind to everything else. This machine also
      carries FIVE SW3D-680 stepper drives on ring 0 -- the MOTION ring -- and
      every one of them was being offered under 輸入 / 輸出 as a place to go and
      look at field IO. Their DI/DO bytes are their PDO image, exactly like a
      SERVOPACK's, so the rail was making the same mistake it had already been
      fixed for once, on a module whose name happened not to contain the magic
      word.

      The module can be ASKED instead. CoE 1000h Device Type's low word is the
      CiA profile number, and 402 is drives-and-motion. Measured 20260915 with
      the SERVOPACK as a positive control:
          SGDXW-2R8AA0A1000   1000h = 0x00020192  -> 402
          SW3D-680 (x5)       1000h = 0x00040192  -> 402
          SW3D-680            6502h = 0x1E7 -> pp, vl, pv, hm, ip, csp, csv
      So both families say "I am a motion drive" in the same words, and a name
      regex was never going to generalise.

      ⚠ THE NAME TEST IS KEPT AS A FALLBACK, not replaced. A module that does
      not answer 1000h has profile === undefined, and an unknown profile must
      not reclassify something that was already being grouped correctly. */
  const isServo = c => c.profile === 402 || /SERVOPACK/i.test(c.model || "");
  /*  AI(W906-1203ALM-18) 20260914: INFRASTRUCTURE IS NOT AN IO CARD.
      User: "2515 的模組也不用顯示在上面 / 只要是中繼通訊用的都不用顯示".

      An ECAT-2515 is a 5-port junction: it forwards frames and carries no
      field wiring. One of them (0x02) was nevertheless appearing under 輸入
      INPUT, because the card's DI map attributes bytes to it -- so the rail was
      proposing a cabling component as somewhere to go and look at inputs.

      ⚠ Matched on the module's OWN reported name, like every other split in
      this file, never on an address range. "Junction" is the word these
      modules use for themselves; hub/splitter/repeater are included so the next
      piece of passive infrastructure does not have to be found the same way.
      ⓘ Their bytes still publish and still appear in 全部 IO. This hides a
      destination, not data -- the same rule as the 伺服 PDO removal. */
  const isRelay = c => /junction|hub|splitter|repeater/i.test(c.model || "");
  const cards = all.filter(c => !isServo(c) && !isRelay(c));
  const servo = all.filter(isServo);

  const addCard = c => nav.appendChild(railLink(
    fmtStation(c.station),
    `?src=ws&station=${c.station}`,
    { on: st === c.station,
      /*  AI(W906-1203CARDS-1) 20260922: the old wording was "同一個 ID 兩個 ring
          都有", which described the cross-ring case. Cross-ring duplicates are
          now two separate cards -- they always were two devices -- so reaching
          this branch means something narrower and worth naming: two devices on
          the SAME ring answering to one number. */
      sub: (c.ambiguous ? "⚠ 同一個環上有兩台同號（另一台是轉接器，無 IO） — " : "") +
           (c.model || "model unknown") +
           `  ·  環 ${c.ring}  ·  ${c.ports.length} DI / ${c.doPorts.length} DO bytes` }));

  /*  AI(W906-1203ALM-15) 20260914: OUTPUT AND INPUT ARE SEPARATE GROUPS.
      User: "可以幫我把左邊改成 Output一個群組 input 一個群組? 其他不要動".

      Classified from the module's OWN reported name first, and only from the
      ports it carries when the name does not say. Name first because a module
      can have both: ECAT-VC4-ODM1 carries 3 real DO bytes AND 9 input bytes
      that are its status PDO, so counting ports alone would put a genuine
      output module in whichever group its status bytes outnumbered -- which is
      the same confusion that made the user ask what the inputs were doing on
      an output card.

      Measured on this machine:
          ECx-P32-HON 32DI (80..84)   name says DI      -> INPUT
          ECAT-VC4-ODM1    (160,161)  name says neither
                                      but carries 3 DO  -> OUTPUT
      ⚠ SERVOPACKs are NOT in either group; they were already split out above
      and stay under 伺服 PDO, because none of their bytes are field IO. */
  const saysDO = c => /\d+\s*DO\b/i.test(c.model || "");
  const saysDI = c => /\d+\s*DI\b/i.test(c.model || "");
  const isOut  = c => saysDO(c) || (!saysDI(c) && c.doPorts.length > 0);

  const outCards = cards.filter(isOut);
  const inCards  = cards.filter(c => !isOut(c));

  /*  AI(W906-1203ALM-16) 20260914: WITHIN EACH GROUP, ONE SUB-GROUP PER MODEL.
      User: "然後偵測到相同名稱模組 再放一個群組".

      ⚠ The sub-heading is only drawn when the group holds MORE THAN ONE model.
      A single heading over the only kind of module present is noise -- it
      repeats the group's own identity and pushes the stations down a line for
      nothing. It appears exactly when it earns its place: when there is
      something to tell apart.

      ⚠ Grouped on the module's reported name TRIMMED OF ITS PORT SUFFIX. The
      ECAT-2515 junctions report names like "...Junction(IN/OUT1)" and
      "...Junction(OUT3)" -- the same model, a different port, and grouping on
      the raw string would scatter identical modules into one sub-group each,
      which is the opposite of what was asked for. */
  const modelKey = c => String(c.model || "unknown")
                          .replace(/\s*\([^)]*\)\s*$/, "")   // drop "(OUT3)" etc
                          .replace(/\s+Rev[0-9.]+\s*$/i, "")  // drop firmware rev
                          .trim();

  /*  AI(W906-1203ALM-19) 20260914: each group heading carries its own class so
      it can be coloured apart. User: "左邊output input 馬達Axis 都放大 用不同
      顏色標 加粗".
      ⚠ Colour is the SECOND signal, never the only one -- the headings still
      read 輸出 / 輸入 / 馬達 in words, so a dimmed panel or a colour-blind
      reader loses nothing. */
  const addGroup = (title, list, emptyText, cls) => {
    nav.appendChild(el("div", "railhead " + (cls || ""),
                       `${title}  (${list.length})`));
    if (list.length === 0) {
      nav.appendChild(el("div", "emptynote", emptyText));
      return;
    }
    const byModel = new Map();
    for (const c of list) {
      const k = modelKey(c);
      if (!byModel.has(k)) byModel.set(k, []);
      byModel.get(k).push(c);
    }
    const models = [...byModel.keys()].sort();
    for (const m of models) {
      if (models.length > 1) {
        nav.appendChild(el("div", "railsubhead",
                           `${m}  ×${byModel.get(m).length}`));
      }
      byModel.get(m).forEach(addCard);
    }
  };

  addGroup("輸出 OUTPUT", outCards, "沒有任何已發現的模組帶有輸出位元組。", "rhout");
  addGroup("輸入 INPUT",  inCards,  "沒有任何已發現的模組帶有輸入位元組。", "rhin");

  /*  AI(W906-1203ALM-17) 20260914: 伺服 PDO IS NOT LISTED. User: "圖片上的
      伺服IO不用顯示".

      Those eight entries were the drives' process-data bytes, and after
      AI(W906-1203ALM-14) every one of their pages is read-only -- the inputs
      are status words and the outputs are RxPDO command words, not coils. So
      the group offered eight rail entries that lead to nothing operable, above
      the axis list that does operate those same drives. Removing it is removing
      a duplicate route to a read-only view, not removing data.

      ⚠ NOTHING IS HIDDEN. The bytes still publish, still appear in 全部 IO, and
      a servo station's own page still renders if its ?station= URL is opened
      directly -- with the PDO labelling that says what those bytes are. The
      rail simply stops proposing them as a place to go.
      ⓘ `servo` is still computed above, because it is what keeps SERVOPACKs out
      of the 輸出/輸入 groups. Dropping the variable would put nine drives back
      into the IO lists. */

  /*  AI(W906-1203CTL-30) 20260911: ONE RAIL ENTRY PER AXIS, like the IO cards.
      User: "要跟IO一樣 分卡片在左邊 / 點選左邊馬達卡片 可以單獨操作該馬達".
      ⚠ AND ONLY AXES THAT ACTUALLY OPENED. An entry for a slot the monitor
      never opened would lead to a pane whose every button can only be refused;
      sixteen of those would bury the ones that work. Same rule as the cards:
      the rail lists what the card reports, not what the wire shape allows. */
  const axes = [];
  for (let a = 0; a < AXIS_SLOTS; a++) {
    if (tags.get(`pci1203.ax${a}.opened`) === true) axes.push(a);
  }
  /*  AI(W906-1203AXMAP-1) 20260915: ⚠ THE AXIS LABELS REST ON ONE ASSUMPTION,
      and it was wrong for twelve of fourteen axes until today: that the card
      publishes ONE AXIS PER MOTION-RING SLAVE, in cable order. That is measured
      and it is what Common Motion Utility shows, but it is a property of how
      this card is CONFIGURED, not a law.

      So the two numbers that have to agree are shown when they do not. A
      shifted mapping is the hardest defect on this page to see -- every station
      present, every count plausible, just attached to the wrong drive -- and it
      survived for days because nothing compared the card's own axis count with
      the number of slaves on the ring. */
  if (tags.get("pci1203.axScan.mapConsistent") === false) {
    const cardAxes = tags.get("pci1203.axScan.cardAxes");
    const expect   = tags.get("pci1203.axScan.expected");
    const ringSl   = tags.get("pci1203.axScan.ringSlaves");
    /*  AI(W906-1203IDX-1) 20260916: name the collided stations HERE, because
        they are usually the whole explanation for the gap. cardAxes counts
        every axis the CARD knows about, including the ones on stations whose
        address is claimed by a neighbour; `expected` is derived only from
        stations this software can address. So the difference is normally
        exactly the collision count -- and saying so turns "two numbers
        disagree, be careful" into "these two drives are why". */
    let collided = 0;
    for (let i = 0; i < SLAVE_SLOTS; i++) {
      if (tags.get(`pci1203.slave${i}.present`) === true &&
          tags.get(`pci1203.slave${i}.unaddressable`) === true &&
          tags.get(`pci1203.slave${i}.ring`) === 0) collided++;
    }
    nav.appendChild(el("div", "railhead rhax", "⚠ 馬達編號可能對不上"));
    nav.appendChild(el("div", "emptynote",
      `卡片說它有 ${cardAxes === undefined ? "?" : cardAxes} 個軸，` +
      `但依運動環上「定得到址」的 ${ringSl === undefined ? "?" : ringSl} 個從站（每站問它有沒有` +
      `第二個參數視窗）算出來是 ${expect === undefined ? "?" : expect} 個。` +
      (collided > 0
        ? `差的 ${(typeof cardAxes === "number" && typeof expect === "number")
             ? (cardAxes - expect) : "?"} 個，就是下面「站號相撞」那 ${collided} 台 —— ` +
          `卡片認得它們、這套軟體定不到它們的址。把 SubDevice ID 改成唯一再斷電上電，` +
          `這兩個數字就會一致。`
        : `兩個數字不一樣時，下面的「位址」標籤會從某一顆開始整串位移 —— ` +
          `每一項看起來都合理，只是接到別顆馬達。先確認卡片組態，不要照這些編號操作。`)));
  }
  nav.appendChild(el("div", "railhead rhax", `馬達 AXES  (${axes.length})`));
  if (axes.length === 0) {
    nav.appendChild(el("div", "emptynote", "No axis slot reports opened=true."));
  }
  for (const a of axes) {
    const st = tags.get(`pci1203.ax${a}.stateText`);
    /*  AI(W906-1203CTL-32) 20260911: LABEL THE CARD WITH THE STATION NUMBER.
        User: "馬達編號 可以用站號表示嗎?".
        The old label said "board B · port P", which was wrong twice over: the
        vendor call is Acm_AxOpenbyID(dev, SlaveID, SubID, ...) so those two
        arguments are a STATION and a sub-axis, and this machine has exactly
        ONE motion card -- so "board 3" named a card that is not in the
        chassis. The monitor now enumerates by station and each axis is opened
        through the station that owns it, so `站N-軸K` is a unique name for one
        motor and matches the number set on the drive itself. */
    /*  AI(W906-1203CTL-39) 20260911: THE LABEL IS THE ADDRESSING ADDRESS.
        User ruling: "你左邊卡號名稱應該要以 定位位址 命名 不是流水號".

        This is the third naming this list has had today and the user settled
        it, correctly, by how they actually talk about the machine: every time
        they have reported a fault it has been "定址位址 30", never "站 7".
        The address is what Acm_AxOpenbyID takes and what appears in every log
        line and vendor call, so labelling by it makes a report and a rail entry
        the same word.
        ⚠ The dial alias is NOT dropped -- it moves to the sub-line. It is the
        number on the cabinet and the one Common Motion Utility's tree shows, so
        losing it would just recreate the confusion in the other direction. The
        pane heading and the detail table still carry both. */
    const addr = tags.get(`pci1203.ax${a}.station`);        // ESC 0x0010
    const dial = tags.get(`pci1203.ax${a}.stationAlias`);   // ESC 0x0012
    const sub  = tags.get(`pci1203.ax${a}.stationAxis`);
    const cnt  = tags.get(`pci1203.ax${a}.stationAxes`);
    const named = (typeof addr === "number" && addr >= 0 &&
                   typeof sub === "number" && sub >= 0);
    /*  A single-axis drive has nothing to disambiguate, so it is just the
        address; a two-axis SERVOPACK needs the sub-axis to say which half. */
    const title = named
      /*  AI(W906-1203HEX-1) 20260915: ⚠ HEX AND DECIMAL, because the axis rail
          printed DECIMAL while the IO rail beside it printed fmtStation's
          "0x35  (53)" -- two notations for the same thing on one page. Common
          Motion Utility's tree is hex, so our 位址 54 and its 0x036 read as two
          different motors. User: "最後五軸 ... 應該要35~40", which is exactly
          0x35..0x40 -- the five SW3D-680 we were calling 53, 54, 56, 57, 64.
          Same numbers, and the page was the one being inconsistent. */
      ? (cnt === 1 ? `位址 ${fmtStation(addr)}`
                   : `位址 ${fmtStation(addr)} · 軸 ${sub}`)
      : "ax" + a;
    /*  ⚠ THE STATE IS IN THE RAIL, not only inside the pane. On this machine
        most axes sit in ERROR_STOP and a handful are READY, and which is which
        decides whether pressing JOG moves a motor. An operator should be able
        to see that before choosing, not after. */
    const dialTxt = (typeof dial === "number" && dial >= 0) ? `站 ${dial} · ` : "";
    /*  AI(W906-1203KEEP-1) 20260916: the state is a VALUE, not part of the
        shape. It used to be interpolated into this string, which forced
        railKey() to include it, which made a MOVE (READY -> running -> READY)
        rebuild the stage and clear the distance box the operator had just
        typed. As a bound span it updates in place and nothing is rebuilt. */
    const subEl = el("span", "");
    const stEl  = el("span", "");
    stEl.dataset.k = `pci1203.ax${a}.stateText`;
    stEl.dataset.f = "railState";
    stEl.textContent = (typeof st === "string" && st.length) ? st : "state ---";
    subEl.appendChild(stEl);
    if (named) subEl.appendChild(document.createTextNode(`  ·  ${dialTxt}ax${a}`));
    nav.appendChild(railLink(
      title,
      `?src=ws&view=axes&ax=${a}`,
      { on: view === "axes" && axSel === a, sub: subEl }));
  }

  /*  AI(W906-1203RAIL-1) 20260915: DRIVES THIS CARD OPENED NO AXIS FOR.
      User: "這個模組式馬達 你放錯位置了吧".

      Moving the SW3D-680s out of 輸出 / 輸入 was the fix, but on its own it
      would have made them DISAPPEAR: they are drives, so they are excluded from
      the IO groups, and the card opened no axis on them, so they are not in
      馬達 AXES either. "In the wrong group" would have become "gone", which is
      not better -- the user was pointing at a module they wanted to find.

      MEASURED 20260915: five SW3D-680 on ring 0 at addresses 53, 54, 56, 57 and
      64, ALL IN PREOP while every SERVOPACK is in OP. That is the fact worth
      surfacing. A drive in PREOP exchanges no process data, so the card cannot
      open an axis on it -- the empty axis list and the PREOP state are the same
      fact seen twice, and neither is visible if the module is simply hidden.

      ⚠ These entries deliberately do NOT link anywhere. There is no axis to
      operate and their PDO page is read-only, so a link would lead to a pane
      whose every control can only be refused -- the same reason the 伺服 PDO
      group was removed above. This group reports; it does not offer a route. */
  {
    const axStations = new Set();
    for (const a of axes) {
      const s = tags.get(`pci1203.ax${a}.station`);
      if (typeof s === "number" && s >= 0) axStations.add(s);
    }
    const noAxis = servo.filter(c => !axStations.has(c.station));
    if (noAxis.length > 0) {
      nav.appendChild(el("div", "railhead rhax",
                         `驅動器 · 此卡未開軸  (${noAxis.length})`));
      for (const c of noAxis) {
        const item = el("div", "railitem raildead");
        item.appendChild(el("div", "railmain", fmtStation(c.station)));
        /*  ⚠ The EtherCAT state is the headline, not a footnote. PREOP is why
            there is no axis, and an operator looking for a motor that will not
            move needs that word before anything else. */
        item.appendChild(el("div", "railsub",
          `${ecStateText(c.state)}  ·  ${c.model || "model unknown"}` +
          (typeof c.ring === "number" ? `  ·  ring ${c.ring}` : "")));
        nav.appendChild(item);
      }
      nav.appendChild(el("div", "emptynote",
        "這些模組自己回報 CoE 1000h = 402（drives and motion），所以不是 IO 卡，" +
        "不列在 輸出 / 輸入。它們在 PREOP 時不交換過程資料，卡片也就開不出軸 —— " +
        "要讓它們能動，得先讓它們進到 OP。"));
    }
  }

  /*  AI(W906-1203POS-1) 20260916: STATIONS THE SCAN CANNOT ADDRESS AT ALL.
      User, after a day of looking at a rail that was two drives short:
      "為什麼圖片上這幾個軸 之前的版本都可以偵測得到 你現在這版本就不見了"
      and then, plainly: "一樣沒有啊".

      They were right and the rail was lying by omission. The whole scan is an
      address sweep (Acm_DevGetSlaveInfo takes a station address -- measured, it
      is not an index), so two stations claiming ONE address collapse into one
      answer and the other is never seen. It is not missing, not faulty, not in
      PREOP: it is unaddressable, and the group above cannot show it because
      that group is keyed on the address it does not have.

      MEASURED 20260916 on ring 0 -- master counts 14, sweep answers for 12:
          pos 9   addr 0x00A   SW3D-680   ← same address as pos 5  (SGDXW)
          pos 13  addr 0x00E   SW3D-680   ← same address as pos 0  (SGDXS)
      The vendor's own tree lists both duplicates, which is why it shows all
      fourteen and this page showed twelve.

      ⚠ NO LINK, and not for the usual reason. There is no address to send
      anything to; a row that looked clickable would open a pane whose controls
      would reach the TWIN -- a different motor. */
  {
    /*  Every station, keyed by position, so a collided one can NAME the station
        that holds its address instead of just saying "someone has it". */
    const byPos = new Map();
    const stuck = [];
    for (let i = 0; i < SLAVE_SLOTS; i++) {
      if (tags.get(`pci1203.slave${i}.present`) !== true) continue;
      const rec = { pos:  tags.get(`pci1203.slave${i}.position`),
                    ring: tags.get(`pci1203.slave${i}.ring`),
                    addr: tags.get(`pci1203.slave${i}.addr`),
                    name: tags.get(`pci1203.slave${i}.name`),
                    bad:  tags.get(`pci1203.slave${i}.unaddressable`) === true };
      if (typeof rec.pos === "number") byPos.set(`${rec.ring}/${rec.pos}`, rec);
      if (rec.bad) stuck.push(rec);
    }
    if (stuck.length > 0) {
      /*  AI(W906-1203PHYS-1) 20260916: ⚠ THE HEADING CHANGED AND THE OLD ONE
          WAS WRONG BY THE TIME IT SHIPPED. It said 定不到址 -- "no address
          reaches it" -- which was true of the whole station for one commit,
          and stopped being true the moment these drives got axes through
          Acm_AxOpen. Two of the rows below now appear in 馬達 AXES as well,
          and a heading that says they are unreachable would contradict the
          axis that is sitting there working. What is still unreachable is
          narrower: the station NUMBER, which is what parameter writes use. */
      /*  AI(W906-1203COLLIDE-1) 20260922: ⚠⚠ THE HEADING PROMISED A
          CONSEQUENCE THAT IS NOT HAPPENING, AND AN OPERATOR CALLED IT.
          User: "為什麼01站號衝突 範例程式可以用 你就不能用?"

          It read "站號相撞 · 參數寫不進去" unconditionally. That was true when
          it was written -- the colliding stations were SERVOPACKs on the motion
          ring, and every parameter this page writes is addressed by station
          number. It is not true today, MEASURED on the live ring:

            ring 0 (motion)  addr 14,0,1,3,41,10,30,108 -- ALL UNIQUE
            ring 1 (IO)      the three collisions: a junction pair at addr 10,
                             a junction vs ECx-C32-HON 32DO at addr 1, and a
                             junction vs ECx-P32-HON 32DI at addr 2
            pci1203.ax*.stationAmbiguous   True: 0   False: 14

          So no axis is flagged, no parameter write is refused, and the three
          rows below are IO-side stations that this page never sends a
          station-addressed parameter to in the first place. Clicking an output
          bit is unaffected either way -- Acm_DaqDoSetBit takes a FLAT channel
          (port*8+bit), not a station.

          ⚠ So the banner was announcing a refusal that was not happening, on
          the page whose whole value is that its warnings can be trusted. That
          is worse than saying nothing: the operator compared it against Common
          Motion Utility, saw the Utility using station 01 perfectly well, and
          reasonably concluded this software was the broken one.

          The heading is COMPUTED now. A collision only earns the "寫不進去"
          wording when it lands on a station this page actually addresses by
          number -- i.e. one that carries an axis. */
      /*  ⚠⚠ THE FIRST VERSION OF THIS TEST WAS WRONG IN THE SAME WAY THE
          BANNER WAS, AND THE RENDER CAUGHT IT. It asked "does any axis have
          this station number?" and reported 2 of 3 blocking -- because the
          collisions are at addr 1, 2 and 10 on ring 1, and ring 0 happens to
          have SERVOPACKs at 1 and 10 as well. Those are DIFFERENT DEVICES ON
          DIFFERENT RINGS. Every SDO write passes ring 0 with the station, so
          ring-0 station 1 resolves to exactly one drive and is not ambiguous at
          all. Comparing station numbers across rings is the very mistake that
          made me suspect the C++ side of it in the first place.

          The authoritative answer is already computed, per axis, by the module
          that owns the addressing: pci1203.ax*.stationAmbiguous. Measured on
          this ring -- True: 0, False: 14. Use it rather than re-deriving it
          here from a number that does not carry the ring. */
      const blocking = stuck.filter(s => {
        for (let i = 0; i < AXIS_SLOTS; i++) {
          if (tags.get(`pci1203.ax${i}.station`) !== s.addr) continue;
          if (tags.get(`pci1203.ax${i}.stationAmbiguous`) === true) return true;
        }
        return false;
      });

      /*  AI(W906-1203CLASS-1) 20260922: ⚠ CLASSIFY THE COLLISION INSTEAD OF
          CALLING ALL OF THEM "ID 衝突". User: "你的ID衝突 可以分類好嗎? 感覺根本
          不是ID衝突" -- and the instinct is right.

          "SubDevice ID conflicted" is the card's own return code (0x8300002B)
          and it describes the SYMPTOM: two slaves answer to one address. It
          does not say WHY, and the page repeated it as if it did -- then told
          the operator to go and reassign IDs with Common Motion Utility, which
          is advice about a dial that may never have been set.

          MEASURED on this ring, and every collision has the same shape:
              addr 1   ECAT-2515 junction @pos 0   vs  ECx-C32-HON 32DO @pos 12
              addr 2   ECAT-2515 junction @pos 1   vs  ECx-P32-HON 32DI @pos 13
              addr 10  ECAT-2515 junction @pos 10  vs  ECAT-2515 junction @pos 11
          The partner is ALWAYS a junction. And the junction addresses form
          runs that follow cable order -- positions 0,1,2,3 hold 1,2,3,4 and
          positions 8,9,10,11 hold 8,9,10,10 -- which is what an automatically
          assigned address looks like, not what somebody's dial setting looks
          like. The modules that ARE deliberately numbered on this ring carry
          16,17,18 / 80..84 / 160..162 / 11,12,13.

          ⚠⚠ WHAT I CANNOT MEASURE, AND WILL NOT GUESS: whether a number was
          SET by a person or ASSIGNED by the master. That is ESC register
          0x0012 (Configured Station Alias), and reading ESC registers on this
          card was measured on 20260910 to KILL the ring's cyclic data -- the
          axis lamps froze for a whole day and stayed frozen. It is behind
          WB_PUMP_1203_READ_ESC_REGS, off by default, and this classification
          deliberately does not turn it on. So the wording below says what the
          PARTNER is, which is observable, and stops there.

          ⓘ Junctions carry no process data -- confirmed, the card's IO map
          attributes no DI or DO byte to an ECAT-2515 -- so a number "shared"
          with one is shared with something that has nothing to address. */
      const isJunction = (n) => /junction|2515/i.test(n || "");
      const classify = (s) => {
        let owner = null;
        for (const r of byPos.values()) {
          if (!r.bad && r.ring === s.ring && r.addr === s.addr) { owner = r; break; }
        }
        if (isJunction(s.name) || (owner && isJunction(owner.name))) {
          return { kind: "junction", owner,
                   label: "轉接器占號",
                   why: "跟它撞號的是 ECAT-2515 轉接器 —— 轉接器沒有任何 IO 資料，" +
                        "所以這個號碼被占著不影響讀寫。" };
        }
        return { kind: "both", owner,
                 label: "兩台都是有資料的模組",
                 why: "⚠ 這一組兩邊都會佔用過程資料，是真正需要處理的重號。" };
      };
      /*  AI(W906-1203CLASS-1) 20260922: the heading names the CLASS, not the
          card's raw return code. "ID 衝突" was the card's word for the symptom
          and it implied someone had set two dials the same. */
      const junctionOnly = stuck.every(s => classify(s).kind === "junction");
      nav.appendChild(el("div", "railhead rhax",
        blocking.length > 0
          ? `⚠ 站號重複 · 參數寫不進去  (${blocking.length}/${stuck.length})`
          : junctionOnly
            ? `ⓘ 站號被轉接器占用（不影響操作）  (${stuck.length})`
            : `ⓘ 站號重複（目前沒有擋到任何操作）  (${stuck.length})`));
      for (const s of stuck) {
        /* who is holding this address? the one with the same addr that is NOT
           flagged -- named explicitly, because "go fix the duplicate" is not
           actionable until you know which two. */
        const cls = classify(s);
        const owner = cls.owner;
        /*  AI(W906-1203RING-1) 20260922: ⚠ THESE ROWS ARE LINKS AGAIN, AND THE
            MISSING LINK WAS THE THING ACTUALLY STOPPING THE OPERATOR.
            User: "為啥我還是不能控? 範例程式都可以控".

            They were rendered as `raildead` with the reasoning, written here,
            that "a row that looked clickable would open a pane whose controls
            would reach the TWIN -- a different motor". That was written about
            SERVOPACKs, where every control is addressed by station number. It
            is not true of an IO module: output bits are now written with
            Acm_DaqDoSetBitEx using the RING, the station and the channel the
            card's own map attributes to that byte, so the twin on the other
            ring is a different address and cannot be reached by accident.
            ⓘ The pane itself splits by ring and says so when a number is shared
            -- see sectionStation. Nothing is hidden and nothing is merged.
            ⚠ What made the old refusal so expensive is that it was invisible as
            a refusal: there was no message, just a row that did not respond.
            The operator compared it against the vendor Utility, which opens the
            same module perfectly well, and concluded this software was broken. */
        const item = railLink(
          `${fmtStation(s.addr)}  ${s.name || "model unknown"}`,
          `?src=ws&station=${s.addr}`,
          { sub: `[${cls.label}]  環 ${s.ring} · 線序位置 ${s.pos}  ·  同號的是` +
                 (owner ? `線序位置 ${owner.pos}（${owner.name || "?"}）` : "另一台") });
        nav.appendChild(item);
      }
      /*  AI(W906-1203COLLIDE-1) 20260922: the prose branches for the same
          reason the heading does. The old text asserted "運動環上的兩台已經在
          上面「馬達 AXES」裡" -- describing the ring as it was six days ago. */
      /*  AI(W906-1203CLASS-1) 20260922: this text used to end with "用 Common
          Motion Utility 重新指派 SubDevice ID". That is the right instruction
          for two modules whose dials were set the same. It is the WRONG
          instruction here, and telling an operator to go and change a setting
          that may not exist is how an afternoon disappears. */
      nav.appendChild(el("div", "emptynote",
        "這幾台是「照線序（index）」問出來的 —— 型號、站號、位置都是真的，不是推出來的。" +
        (blocking.length > 0
          ? "⚠ 其中有站號是「馬達」在用的，而極限、電子齒輪比、回HOME、Pn000、Fn008 " +
            "都是寫到站號的 —— 寫下去會落在另一台身上而且回報成功，所以那些寫入一律拒絕。" +
            "運動本身不受影響（那是走實體軸編號）。"
          : "★ 目前沒有任何操作被擋住。實測：14 顆軸的 stationAmbiguous 全是 false；" +
            "IO 的讀寫已改用「環＋站＋站內通道」定址（Acm_DaqDoSetBitEx），" +
            "所以同號的另一台在別的位址上，打不到。") +
        "\n\nⓘ 為什麼說「不是 ID 衝突」：0x8300002B 是卡片回的碼，它描述的是「症狀」" +
        "（兩台答同一個位址），不是原因。實測這條環上每一組重號，另一邊都是 " +
        "ECAT-2515 轉接器；而轉接器的位址是照線序連號的（位置 0~3 拿 1,2,3,4；" +
        "位置 8~11 拿 8,9,10,10），那是「自動指派」的長相，不是有人撥指撥開關的長相 —— " +
        "這條環上真的被設過號的模組是 16,17,18 / 80~84 / 160~162 / 11,12,13。" +
        "\n\n⚠ 但「到底是人設的還是主站給的」我沒有量，也不猜：那要讀 ESC 暫存器 0x0012，" +
        "而 20260910 實測那個呼叫會把環的循環資料弄死（軸的燈號整天不動，而且之後一直不動）。" +
        "它被關在 WB_PUMP_1203_READ_ESC_REGS 後面，預設不開，這裡也不會為了分類而打開它。" +
        "\n\n要根治的話：把轉接器或那兩顆模組其中一邊改成沒有人用的號碼，" +
        "然後把那些站斷電再上電（重開電腦沒有用，站號存在從站裡）。" +
        "⚠ 以目前的狀況，不改也不會有問題。"));
    }
  }

  /*  The three "everything at once" views, together at the bottom. They are
      kept because a per-card pane cannot answer "is anything moving anywhere",
      which is the question at the start of a fault -- but they are below the
      per-card list, because that is where the work happens. */
  nav.appendChild(el("div", "railhead", "其他 OTHER"));
  nav.appendChild(railLink("全部 IO  All IO", "?src=ws&view=io",
    { on: view === "io",
      sub: `${diPortCount(tags)} DI · ${doPortCount(tags)} DO ports` }));
  nav.appendChild(railLink("全部軸 All axes", "?src=ws&view=axes",
    { on: view === "axes" && axSel === null,
      sub: `${axes.length} opened · Common Motion Utility 單軸運動` }));
  nav.appendChild(railLink("診斷 Diagnostics", "?src=ws&view=diag",
    { on: view === "diag", sub: "build, card, ring, why a screen is blank" }));
  return nav;
}

/** Build the whole report into a fresh container and return it. Pure: the
 *  caller decides where it goes. */
export function buildReport(tags, linkState, opts) {
  const o    = opts || {};
  const view = o.view || "";
  const st   = (typeof o.station === "number") ? o.station : null;

  const root = document.createElement("div");

  /*  AI(W906-1203ALM-12) 20260914: ⚠ DISCONNECTED IS THE FIRST THING ON THE
      PAGE, ABOVE EVERYTHING. User: "如果斷線了 需要在上面顯示已經斷線 並且要
      有按鈕可以讓我重新連線".

      Until now a dropped link showed only as a word in the subtitle, while
      every value on screen kept its last number -- so the page looked alive and
      the buttons looked broken. They were not broken: js/transport/ws.js was
      dropping each command into a closed socket with a console.warn. Both
      halves are fixed; this is the half you can see.

      ⚠ The banner states the AGE of the data as well as the state. A screen
      full of numbers that stopped updating is the most dangerous thing this
      page can show, because every one of them still looks like a reading. */
  /*  AI(W906-1203DEAD-1) 20260917: ⚠⚠ THE MONITOR HAVING GIVEN UP IS ALSO A
      PAGE-WIDE FACT, AND IT WAS ONLY DRAWN ON THE DIAGNOSTICS VIEW.
      User: "我每次開久沒有動 就沒辦法控制馬達了".

      The observer self-disables after kMaxConsecutiveFailures (10) polls in
      which EVERY read failed, and it never re-enables on its own -- only
      Open() or Rescan() clears it. That is the designed safety valve so a card
      that has gone away cannot block a machine tick.

      But the banner announcing it lived in buildCardSection(), which only the
      診斷 view draws. An operator sitting on an axis pane when it latched saw
      nothing at all: the numbers simply stopped changing and the buttons
      stopped working, which is indistinguishable from "the software broke".
      Exactly the failure the offline banner below was added to remove, in the
      other half of the same problem -- and the fix is the same one.

      ⚠ It carries the RECOVERY, not just the news. 重新掃描 (Rescan) is what
      clears it, and an operator who cannot see the state cannot know to press
      it. A banner that reports without offering the way out is half a fix.

      ⓘ This does not diagnose WHY the reads failed, and does not pretend to.
      It answers "is the monitor still reading?", which is the question that
      separates "the card stopped answering" from "the page is broken" -- and
      those need different next steps. */
  if (tags.get("pci1203.disabled") === true) {
    const dz = el("div", "offlinebar");
    const dh = el("div", "offlinehead");
    dh.appendChild(el("span", null, "⛔ 監控已自我停用 —— 已經停止讀取這張卡"));
    dz.appendChild(dh);
    const db = el("div", "offlinebody");
    db.textContent =
      (tags.get("pci1203.disabledReason") || "(沒有發布原因)") +
      "　⚠ 畫面上所有的值都是停用前的最後一筆，不會再更新 —— " +
      "它們看起來仍然像讀數。這段期間馬達指令也不會有作用。" +
      "　這是設計好的安全閥（連續 10 次輪詢每一個讀取都失敗就停下來，" +
      "以免一張已經不見的卡把機台的 tick 卡住），不是當機。" +
      "　要恢復：按下面的「重新掃描」，它會重新開卡片並重新列舉。";
    dz.appendChild(db);
    const drow = el("div", "offlinerow");
    //  ⚠ The same wire name the 診斷 bar uses (:4687). A second spelling here
    //  would be a button that looks identical and silently does nothing.
    const rb = cmdButton("⟳ 重新掃描（恢復）", "pci1203.card.rescan", "",
                         { cls: "warnbtn",
                           title: "TPci1203Monitor::Rescan() — 重新開卡片並重新列舉整個環" });
    drow.appendChild(rb);
    dz.appendChild(drow);
    root.appendChild(dz);
  }

  if (linkState && linkState.state && linkState.state !== "online"
      && linkState.state.indexOf("mock") === -1) {
    const off = el("div", "offlinebar");
    const head = el("div", "offlinehead");
    head.appendChild(el("span", null,
      (linkState.state === "connecting") ? "⟳ 連線中…" : "⛔ 已斷線"));
    off.appendChild(head);

    const body = el("div", "offlinebody");
    const bits = [];
    if (linkState.state === "connecting") {
      bits.push("正在建立與伺服器的連線。");
    } else {
      bits.push("與伺服器的連線中斷了。");
      bits.push("⚠ 畫面上的數值是斷線前的最後一筆，不會再更新 —— 它們看起來仍然像讀數。");
      bits.push("按鈕在這段期間送不出去（會告訴你，不會靜靜消失）。");
    }
    if (linkState.error) bits.push(`原因：${linkState.error}`);
    body.textContent = bits.join(" ");
    off.appendChild(body);

    const row = el("div", "offlinerow");
    /*  A live countdown to the automatic retry. The backoff tops out at 15 s
        and fifteen motionless seconds is long enough to conclude the page is
        dead -- data-retryat lets refresh() tick it without a rebuild. */
    if (linkState.retryAt) {
      const cd = el("span", "offlinecd");
      cd.dataset.retryat = String(linkState.retryAt);
      cd.textContent = "自動重試中…";
      row.appendChild(cd);
    }
    row.appendChild(cmdButton("重新連線", "ui.reconnect", "", { cls: "warnbtn" }));
    off.appendChild(row);
    root.appendChild(off);
  }

  /* AI(W906-1203CTL-23) 20260911: the status strip is on EVERY pane, and it is
     three LINES rather than three sections. It replaces sectionControl's prose
     on the operating panes: "is it reading / is it still opening / does a click
     do anything" is what an operator needs continuously, while the reasoning
     behind those three answers is a diagnostic and belongs on the diagnostics
     pane. User: "裡面也太多資訊了". */
  /*  AI(W906-1203ALM-20) 20260914: 重新掃描 — re-open the card and re-scan the
      ring, from the top of every pane. User: "最上方我需要一個Refresh 按鈕
      讓我重開卡片 掃模組", after "因為剛剛我開了其他模組 所以ID重複 會導致
      開卡錯誤".

      Everything this page knows about the ring is decided ONCE when the card
      opens: which modules exist, the DI/DO map, which axes are there. Fit a
      module and the page goes on describing the machine as it used to be, with
      no hint that it is out of date. Until now the only cure was restarting
      the process.

      ⚠ SHOWN EVEN WHEN THE CARD IS CLOSED, and the C++ side exempts this one
      command from its "monitor must be open" precondition. Re-opening a failed
      card is the whole point; refusing it for being closed would lock the
      operator out of the only control that could recover.

      ⚠ NOT gated on the write surface being armed either. It commands no
      motion and energises nothing -- it closes and re-opens a handle this
      process owns. */
  {
    const bar = el("div", "rescanbar");
    bar.appendChild(cmdButton("⟳ 重新掃描", "pci1203.card.rescan", "",
      { title: "關閉再重開卡片，重新掃描環上的模組與軸 — 換過模組之後用這個" }));
    bar.appendChild(el("span", "rescanhint",
      "換過模組、改過站號、或卡片開不起來時按這個。" +
      "頁面上的模組與軸清單是開卡當下決定的，不會自己更新。"));
    root.appendChild(bar);
  }

  root.appendChild(statusBar(tags, o));

  /*  AI(W906-1203ALM-8) 20260914: ⚠ THE OUTCOME OF THE LAST COMMAND, ON EVERY
      VIEW. This is the fix for "其他按鈕 我看也沒訊號".

      MEASURED 20260914, end to end through the real page with the relay armed:
        button pressed   ◀ JOG −  on ax3, tag pci1203.ax3, value -1
        publisher log    cmd #1 pci1203.ax.jog REFUSED
                         Acm_AxJog(ax=3, dir=1) [wire -1 -> DIRECTION_NEG]
                         "axis is in ERROR_STOP -- clear the error deliberately
                          first; this module will not reset it as a side effect"
      So the signal DID arrive and the C++ side answered. What was missing was
      anywhere on screen to read that answer: the `last command` table lives in
      sectionControl, which the axes view does not render, and #cmdnote says
      only "sent" by design. Press a button, get nothing back, conclude the
      button is dead -- which is exactly what happened, repeatedly.

      ⚠ A REFUSAL IS AN ANSWER AND MUST LOOK LIKE ONE. It is shown in red with
      its reason, not hidden: "nothing happened" and "I refused, here is why"
      are completely different facts and they had been rendering identically.

      Bound by data-k like everything else, so refresh() keeps it live without
      a rebuild, and it sits directly under the status bar where the operator
      already looks. */
  {
    const cb = el("div", "cmdresult");
    cb.appendChild(el("span", "cmdresultlabel", "最後一筆指令"));
    cb.appendChild(bindCell(el("span", "cmdresultcmd"), "pci1203.control.lastCmd",
                            undefined, "cmdresultcmd"));
    const ok = el("span", "cmdresultok");
    ok.dataset.k = "pci1203.control.lastOk";
    ok.dataset.boolbad = "1";
    cb.appendChild(ok);
    cb.appendChild(bindCell(el("span", "cmdresultcall"), "pci1203.control.lastCall",
                            undefined, "cmdresultcall"));
    cb.appendChild(bindCell(el("span", "cmdresultwhy"), "pci1203.control.lastWhy",
                            undefined, "cmdresultwhy"));
    cb.appendChild(bindCell(el("span", "cmdresultret"), "pci1203.control.lastRetText",
                            undefined, "cmdresultret"));
    root.appendChild(cb);
  }

  /*  AI(W906-1203CTL-33) 20260911: ⚠ A MODULE THAT IS NOT BEING POLLED MUST SAY
      SO ON EVERY VIEW, not just on diagnostics. This is the fourth time this
      page has been narrower than the rack, and the reason it kept surviving
      review is that truncation renders as ABSENCE -- the missing cards simply
      are not in the list, so the screen looks complete and correct.

      What made it visible at all this time was a user noticing that ONE module
      (0x51, ports 95..98) straddled the old 96-port ceiling and so showed one
      byte of four; the three modules entirely past the ceiling
      (0x52/0x53/0x54) produced no symptom whatsoever. So the banner is keyed on
      the card's own port count against what was actually sampled -- not on
      anything this page can get wrong by being out of date. */
  const diTrunc = tags.get("pci1203.di.truncated") === true;
  const doTrunc = tags.get("pci1203.do.truncated") === true;
  if (diTrunc || doTrunc) {
    const onDi = tags.get("pci1203.di.portsOnCard");
    const smDi = tags.get("pci1203.di.portsSampled");
    const onDo = tags.get("pci1203.do.portsOnCard");
    const smDo = tags.get("pci1203.do.portsSampled");
    root.appendChild(banner("bad",
      "⚠ 有 IO 模組沒有被讀取 — 這個畫面不完整",
      `卡片回報 ${onDi} 個 DI port / ${onDo} 個 DO port，但本程式只取樣了 ` +
      `${smDi} / ${smDo}。超過上限的 port 完全沒有被讀，所以它們的模組不會出現在` +
      `左邊清單裡 —— 缺的卡看起來就像「不存在」，不像「壞掉」。` +
      `修法是把 EtherCAT/Pci1203Monitor.h 的 kPci1203TagDiPorts / ` +
      `kPci1203TagDoPorts 調高並重建；緩衝區大小本身是依卡片回報值自動配置的。`));
  }

  if (view === "axes") {
    root.appendChild(sectionAxes(tags, o));
    return root;
  }
  if (view === "diag") {
    /* ⚠ NOTHING WAS DELETED WHEN THE PAGE WAS SIMPLIFIED -- it MOVED. Every
       explanatory section the operating panes lost is here, in the order it was
       always in: the diagnosis banner first, because "why is this screen blank"
       is what this page was originally built to answer, and that did not stop
       being true when it stopped being in the way. */
    const d = diagnose(tags, linkState);
    root.appendChild(banner(d.level, d.head, d.body));
    root.appendChild(sectionControl(tags, o));
    root.appendChild(sectionBuild(tags));
    root.appendChild(sectionCard(tags));
    root.appendChild(sectionRing(tags, o));
    return root;
  }
  if (st !== null) {
    /* One card: its inputs and its outputs, and nothing else. */
    root.appendChild(sectionStation(tags, st));
    return root;
  }
  /* Default and ?view=io: every attributed byte, grouped by station. This is
     the landing pane because "IO 可以讀" is the first thing asked of the page. */
  root.appendChild(sectionIO(tags, o));
  root.appendChild(sectionDO(tags));
  return root;
}

/** The subtitle line. `gen` is deliberately absent -- see js/pci1203.js. */
export function subtitle(tags, linkState, frames) {
  const pci = [...tags.keys()].filter(k => k.startsWith("pci1203")).length;
  const state = (linkState && linkState.state) || "unknown";
  return `${state}  ·  ${frames} frames  ·  ${tags.size} tags (${pci} pci1203.*)`;
}

/** AI(W906-1203CTL-9) 20260911: THE FOOTER IS NOW STATE-DEPENDENT, and it had
 *  to become so the moment this page could command anything.
 *
 *  FOOTER_HTML below says "This page has no machine control of any kind". That
 *  sentence was true from 20260908 to 20260911 and is FALSE on an armed feed.
 *  Leaving it would have been the worst kind of stale claim: a safety assertion,
 *  restated on every render, describing a page that had grown buttons.
 *
 *  So the unarmed text is kept verbatim (it is still exactly right when nothing
 *  is armed, which is the default) and an armed feed gets a different footer
 *  that says what IS true. The probe asserts both forms name their enforcing
 *  gate, because the mechanism is the part that stops a claim being decoration.
 */
export function footerHtml(tags) {
  if (!controlArmed(tags)) return FOOTER_HTML;
  const dry = controlDry(tags);
  return (dry
    ? "<b>ARMED, DRY RUN.</b> The write surface is enabled and every command is " +
      "validated, formatted and logged — and <b>none is issued</b>. "
    : "<b>ARMED, LIVE.</b> Buttons on this page reach the machine: outputs energise " +
      "and servos move. ") +
    "The commands this page can send are the allowlist in " +
    "<code>EtherCAT/Pci1203Control.cpp</code>, checked by " +
    "<code>tools/pci1203_control_gate.ps1</code>, which also fails if either " +
    "<code>#define</code> is left on in a committed tree. Position redefinition " +
    "(<code>Acm_AxSetCmdPosition</code>), fieldbus reconfiguration " +
    "(<code>Acm_DevSetSlaveID</code>, <code>Acm_DevWriteRegData</code>) and firmware " +
    "download are absent by design.<br>" +
    "<b>The OBSERVER is still read-only.</b> <code>EtherCAT/Pci1203Monitor.cpp</code> " +
    "makes nineteen read-only calls and no others; " +
    "<code>tools/pci1203_readonly_gate.ps1</code> still fails the build if a mutating " +
    "call appears there. Commanding and observing are different files with different " +
    "gates, which is why one can be armed without weakening the other.<br>" +
    "<b>Motion is refused in ERROR_STOP</b>, and an error is never cleared as a side " +
    "effect of a move — “reset then go” hides the reason it stopped.<br>" +
    "<b>null is never drawn as 0.</b> <span class=\"null\">---</span> means nobody read " +
    "it; <span class=\"na\">n/a</span> means the tag is not on this feed at all.";
}

/** The read-only footer. Restates the claim WITH ITS MECHANISM on every
 *  render, so it cannot drift into decoration. Correct whenever the write
 *  surface is not armed, which is the default; see footerHtml(). */
export const FOOTER_HTML =
  /* AI(W906-MW2b) 20260910: "twelve" was correct on 20260908 and is not now --
     the observer's allowlist grew to eighteen when station identity, the IO map
     and DO read-back were added. A footer that restates a count has to be
     re-stated when the count changes, or it becomes the decoration it exists to
     avoid being. probe_pci1203.mjs asserts this line names its enforcing gate;
     the NUMBER is checked against the gate's own output by hand.
     ⚠ AI(W906-1203CTL-9) 20260911: "eighteen" -> NINETEEN. Measured, not
     counted by eye: tools/pci1203_readonly_gate.ps1 prints the calls it found
     in the .cpp and its list has 19 entries today. This is the second time this
     number has gone stale in three days, which is the predictable cost of a
     restated count -- but the alternative (drop the number) loses the only part
     of the sentence a reader can check. */
  "<b>READ-ONLY.</b> This page has no machine control of any kind, and the observer " +
  "behind it can only make the nineteen read-only vendor calls on its allowlist " +
  "(<code>EtherCAT/Pci1203Monitor.h</code>), checked by " +
  "<code>tools/pci1203_readonly_gate.ps1</code>. Move / Jog / MoveHome / SetSvOn / " +
  "DaqDoSet* / ResetError / WriteRegData / DownLoadMapInfo are absent by design. " +
  "The station links here only re-filter this page.<br>" +
  "<b>Digital OUTPUT is a read-back.</b> <code>Acm_DaqDoGetByte</code> reports what the " +
  "card says it is driving. Get and Set are different families; only Get is allowed.<br>" +
  "<b>null is never drawn as 0.</b> <span class=\"null\">---</span> means nobody read it; " +
  "<span class=\"na\">n/a</span> means the tag is not on this feed at all; a number means " +
  "a real reading. Those are three different questions.";
