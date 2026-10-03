// =============================================================================
//  WebBridgeTags.h -- the ONE place where the machine and the browser meet.
//
//  AI(W906-WebBridge) 20260806.  NOT in golden.
//
//  WHY IT LIVES HERE AND NOT IN WebBridge/
//  WebBridge/ is deliberately free of vclcompat and of every machine header, so
//  the socket layer can be reasoned about on its own (see WebBridge/Sync.h and
//  the ht9045_webbridge target's comment in CMakeLists.txt). This file is the
//  deliberate exception: it includes BOTH worlds, reads machine globals, and
//  stages them into a webbridge::TagSnapshot. Keeping that mixing confined to
//  one translation unit is the entire point -- if this were inside WebBridge/,
//  that layer's independence would be gone.
//
//  THE RULE THIS FILE EXISTS TO ENFORCE
//  A tag is published with a REAL value only when its source is actually
//  loaded. Everything else is published as NULL, never as 0.
//
//  That is not fussiness. The browser renders null as "---" and 0 as "0.00"
//  (web/README.md, "Formats"), so publishing 0 for a heater zone nobody has
//  read is a screen that states a measurement which was never taken. On a
//  machine that runs at 130 C, "0.00" and "---" mean very different things to
//  the operator standing in front of it.
//
//  PRECISION ON THAT, measured 20260813: "null renders as ---" holds only for the
//  formatters that say so -- dash/int/fixed*/pct*/hms (web/js/ui/bind.js:49-72).
//  The DEFAULT format is "raw" (bind.js:86), and raw renders null as the EMPTY
//  STRING (bind.js:48). Both are honest -- neither invents a number -- but a tag
//  bound without an explicit data-fmt goes BLANK rather than dashed, so do not
//  expect "---" as proof a tag arrived. machine.state is exactly such a tag.
//
//  WHAT IS ACTUALLY LOADED TODAY -- measured 20260806, not assumed
//  (tests/test_wb_datalayer.cpp and the survey behind it):
//
//    LIVE    CUSTOMER_CODE                      957
//            IniConfig.sMachineType             "HT-9046LS"
//            IniConfig.sGPIBMachineID           "GLY320"
//            IniConfig.RMSTesterID              "HT9046"
//            LastSet.*                          from system\lastdata.dat
//                                               (12,358 non-zero bytes)
//            HSys.*                             420 keys from Gerneral.ini
//
//    LIVE    Temperature.fWorkTemperBase/       via ReadTempFile, called at
//            fSoakTime/iMachineTempMode         wb_serve boot inside the SAME
//                                               DataPath-redirected window
//                                               bin.* already uses -- see
//                                               AI(W906-FW-TEMP2) 20260820
//                                               below. FW-TEMP1's "unsafe,
//                                               do not wire" verdict for
//                                               THESE THREE fields was
//                                               overruled by the main loop:
//                                               it was stricter than the
//                                               already-shipped bin.*
//                                               precedent, which protects
//                                               the exact same shared write
//                                               risk the same way.
//    DEAD    every other Temperature.* field    ReadTempFile writes ~90
//                                               fields; only the 3 above are
//                                               staged. The rest have a real
//                                               loader too now but no tag
//                                               reads them yet.
//            IniConfig numeric/bool flags       loaded by cConfiguration.cpp,
//                                               which is untranslated (7,808
//                                               golden lines).  Only the few
//                                               STRING fields ReadLastSetIni
//                                               sets are live.
//            CosFunction (140 flags)            0 of 488 bytes set --
//                                               CustomerFunctionSelect runs but
//                                               every branch tests an IniConfig
//                                               flag that is not loaded yet
//            UN150Read (all temperature PVs)    genuinely dead, re-measured
//                                               20260820 (AI(W906-FW-TEMP1)):
//                                               its one writer thread never
//                                               starts offline, and its other
//                                               two write sites are compiled
//                                               out. See the block below.
//
//  AI(W906-FW-TEMP1) 20260820: temp.* / zone.* recon (12 tags: temp.pv/sv/
//  soak/mode, zone.{hotplate,shuttle,index,heatgun}.{1,2}). Both halves of
//  "DEAD Temperature.*" above turned out to need a MORE careful answer than
//  the previous wave's blanket claim -- measured this wave, not assumed:
//
//   (a) temp.sv/soak/mode  (Temperature.fWorkTemperBase/fSoakTime/
//       iMachineTempMode, cprod.h:1377/1378/1392) -- SUPERSEDED 20260820, see
//       AI(W906-FW-TEMP2) below. FW3-TempSet (commit c60e9f4) landed
//       uTemp_Set.cpp's ReadTempFile as a FAITHFUL, ACTIVE translation
//       (golden :1976-3143 -> port uTemp_Set.cpp:2172-3143). This wave's
//       first pass (FW-TEMP1) found real write risk in it -- unconditional
//       `MyForceDirectories(szDir)` (:2191) before even the FileExists check,
//       and dozens of fields read via `CheckAndReadIniData`'s missing-key-
//       seeds-a-write pattern -- and refused to wire it on that basis alone.
//       That refusal was ITSELF WRONG, per the main-loop's same-day review:
//       FW-BIN1's recipe-folder scratch-copy (already shipped, wb_serve.cpp)
//       redirects `DataPath` for the WHOLE run, and Temperature.Data lives in
//       that exact same folder -- so ReadTempFile's writes already land in
//       scratch, identically to BinSelect's. FW-TEMP1's "no scratch-redirect
//       workaround this time" was a stricter bar than the shipped precedent,
//       not a new, independently-justified one. See AI(W906-FW-TEMP2) for the
//       corrected wiring and the Init()-dependency audit that went with it.
//
//   (b) temp.pv + the 8 zone.* tags (UN150Read[tc*], defined cmydef.cpp:2768) --
//       genuinely dead.
//
//       ⚠⚠ AI(W906-FW-1Tf) 20260915 RE-MEASURED.  **The conclusion survives;
//       every line number in the original evidence had drifted.**  Anyone
//       following the old citations would have landed on unrelated code.  The
//       counts were wrong too.  Corrected inventory, measured today:
//
//         * DoThermo() -- bthermo.cpp:1262.  The old note said "~80 sites";
//           MEASURED: bthermo.cpp has **125** UN150Read[] write sites, **34 of
//           them NOT inside any `#if 0`** (DoThermoReal, ClearAllHotBuffer, ...).
//           So the gating story never covered this file at all -- what makes it
//           dead is REACHABILITY, and that is the only load-bearing claim here:
//             DoThermo() has exactly ONE caller: uHeaterThread.cpp:386
//               (inside THeaterThread::HeaterThreadProcess, defined :373
//                -- old note said :342/:375)
//             which has exactly ONE caller: THeaterThread::Execute :406
//               (defined :397 -- old note said :366)
//             which is reachable only via StartHeaterThread() :430 -> :437
//               HeaterThread->Resume()
//             and THeaterThread::Resume() is **uHeaterThread.cpp:420-423 with an
//               EMPTY BODY** (old note said :389-392).  Verified by reading the
//               body, not the header comment -- uHeaterThread.h:69 only DECLARES
//               it with an "offline no-op" comment, and a comment is not proof.
//
//         * The other two writers are still gated, but not where the note said:
//             forms/fLotInfo.cpp **:1351-1357** (4 sites, all GATED)
//                 -- old note said :1291-1297
//             uTemp_Set.cpp **:7250-7251** (2 sites, both GATED)
//                 -- old note said :7228-7229
//
//         * cTemperFrom.cpp -- the old note described ONE SOFT_SIMULTE debug-fill
//           branch at :338-348.  MEASURED: **5** write sites, and **:424
//           `UN150Read[Addr]=0;` is NOT gated**.  It is nonetheless unreachable
//           in production for the OTHER reason the note gave and which still
//           holds: ShowThermo (defined cTemperFrom.cpp:234) has **0 production
//           callers** -- the only four are in tests/test_temperfrom_core.cpp.
//           ⚠ "live code inside an unreachable function" is a different claim
//           from "gated code"; the note conflated them.
//
//       `//#define SOFT_SIMULTE` is still commented at MachineType.h:48 (re-checked).
//       zone.* channel mapping (for whenever a writer lands): tcHotPlate1/2=0/1,
//       tcShuttle1/2=2/3, tcHead1/2=4/5 ("index" = the test-head channels),
//       tcHeatGun1/2=27/28 (MachineType.h:637-641) -- one UN150Read[] cell
//       each, same predicate as temp.pv.
//
//  Verification commands (20260820): `Grep "DoThermo\(\)\|HeaterThreadProcess\|
//  THeaterThread::Execute" *.cpp`; `Grep "ShowThermo\s*\(" *.cpp` (4 hits, all
//  tests/test_temperfrom_core.cpp); `Grep "SOFT_SIMULTE" MachineType.h`
//  (commented at :48); `Read uTemp_Set.cpp:2172-2340` (ReadTempFile head);
//  `Read uHeaterThread.cpp:330-415` (THeaterThread::Execute/Resume).
//
//  AI(W906-FW-TEMP2) 20260820: temp.sv/soak/mode WIRED, per the main-loop
//  ruling in AI(W906-FW-TEMP1)(a) above. wb_serve.cpp calls
//  `fTemp_Set->ReadTempFile(true)` inside the SAME `if (pathReady)` /
//  DataPath-redirected block bin.* already uses (golden boot shape:
//  main.cpp:8868, `TfMain::DoReadLastData`'s `fTemp_Set->ReadTempFile(true)`;
//  the wb_serve.cpp call site carries the full evidence, not repeated here).
//
//  `fTemp_Set` is lazily `new`'d there (`TfTemp_Set *fTemp_Set;` is a
//  zero-initialized BSS global, uTemp_Set.cpp:220-274 -- the ctor is
//  fields-only, matching this tree's established
//  new-a-facade-global-at-first-use idiom, e.g. fBinSel/fShowBinSelect,
//  cBinSel.cpp:150 / cShowBinSelect.cpp:159).
//
//  Init() (uTemp_Set.cpp:281-597, golden ctor body :95-408) is DELIBERATELY
//  NOT CALLED -- audited both directions this wave: Init() itself does zero
//  file/hardware I/O and zero fMain/other-form dereferences (safe to call),
//  but ReadTempFile does not read anything Init() populates (myTempPal[]/
//  listNormal/listArm1/listArm2/ATCOffsetEdit[]/etc. -- zero hits grepping
//  ReadTempFile's span for any of them); every widget ReadTempFile DOES touch
//  (rgTemperatureMode/edChillerTemp/cbbATC_RecipeFile/rgIndexHeatMode) is
//  NSDMI'd in forms/fTemp_Set.h (`= new T...()`), so each is already live the
//  moment `new TfTemp_Set()` runs. Calling Init() anyway would be unused
//  surface, against the same minimal-footprint precedent bin.* set.
//
//  Liveness: `g_webTempLoaded` (SetWebTempLoaded), set from golden's OWN
//  "file missing" signal `iSendChangeTempError` (uTemp_Set.cpp:2200) rather
//  than from "did we reach the call" -- Temperature.Data can be absent from
//  an otherwise-valid recipe folder even when Binasgn.Data is present, so
//  bin.*'s cruder `pathReady` signal is not precise enough to reuse here.
//
//  Types: fWorkTemperBase/fSoakTime are `double` (cprod.h:1392/1378) ->
//  `TagValue::makeDouble` via a new `stageDouble` helper (WebBridge/TagValue.h
//  already carries a Double alternative; no workaround needed).
//  iMachineTempMode is `int` -> `stageInt`, published as golden's RAW ini
//  code (0=Hot/1=Ambient/3=AmbientHot, ReadTempFile :2248-2264) -- NOT
//  decoded into the "Hot Mode"/"Ambient Mode" text tagmap.js's
//  lblTemperatureMode expects, because that label lives on fMain (a
//  DIFFERENT form, outside TfTemp_Set entirely -- grepped uTemp_Set.cpp and
//  forms/fTemp_Set.h for "Hot Mode"/"Ambient Mode"/"lblTemperatureMode": 0
//  hits) and guessing its exact caption format would be exactly the
//  "confidently wrong label" this file's own rule refuses.
//
//  So the live tag set is small on purpose. It grows when the sources do, and
//  every addition has to answer "is this source loaded?" with a measurement.
//
//  THREADING
//  PublishHandlerTags() reads machine globals, so it must be called from the UI
//  thread, on the same tick that owns them -- never from the socket thread.
//  That is the contract in web/docs/ARCHITECTURE.md section 5, and it is the
//  reason the snapshot exists at all.
// =============================================================================
#ifndef WEBBRIDGETAGS_H
#define WEBBRIDGETAGS_H

#include "WebBridge/TagSnapshot.h"

#include <string>

namespace ht9045 {

// Fill `snap` with one full generation of tag values and commit it.
// UI THREAD ONLY. Returns the number of tags staged.
std::size_t PublishHandlerTags(webbridge::TagSnapshot& snap);

// AI(W906-P4-SECS) 20260920: 讓持有機台資料的行程（wb_serve）掛一個額外的
//   tag 發布者，**在 commitPublish() 之前**被呼叫。
//
//   ⚠ 存在的理由是相依方向：`WebBridgeTags.cpp` 被好幾支測試**當來源直接編**，
//     所以它不能相依 SECS 那一整疊。預設沒有登錄者 ⇒ 那些測試的
//     快照筆數斷言完全不受影響。
typedef std::size_t (*ExtraTagPublisher)(webbridge::TagSnapshot& snap);
void SetExtraTagPublisher(ExtraTagPublisher fn);
typedef void (*PublishYieldHook)(); void SetPublishYieldHook(PublishYieldHook fn);  typedef bool (*StreamWantedHook)(const char* webId); void SetStreamWantedHook(StreamWantedHook fn);   //AI(W906-STREAM-2B) 20260930: 0 (the default, every ctest) = publish everything as before; wb_serve installs W906_PageStreamWanted   // AI(W906-LAT-1) 20260925: OUTPUT FIRST inside the publish too (user 20260925: 「輸出一定要第一優先發出去」). PublishHandlerTags() calls the hook at iteration boundaries only -- before the pci1203 block, after each axis, every 32 DI / 32 DO ports, every 8 stations, before the extra publisher -- and wb_serve installs one that runs queued OUTPUT commands there (W906_ServiceOutputs(7), same service the 1203 Poll lends its thread to). Default null => the tests that compile this file directly are unchanged. ⚠ A snapshot built across a yield can mix state from before and after that output; the caller must publish once more afterwards (wb_serve: g_outputsServed is cleared BEFORE the publish). ⚠ The hook must not publish: a nested PublishHandlerTags() is refused (returns 0, stages nothing). On the old blank line, so no line below moves
// AI(W906-FW-W3) 20260819: the web server's control-token holder, injected by
// the serving loop before each PublishHandlerTags() call so the snapshot can
// carry it as the `control.owner` tag (design doc section 3). 0 = nobody.
// Published as "conn-<id>" / "" -- every browser sees who operates.
void SetWebControlOwner(unsigned long long connId);

// AI(W906-FW-BIN1) 20260820: host-process marker that the BinSelect->
// SetTechDataToProd_Yield->ShowBinSel chain actually ran (wb_serve does it at
// boot under the DataPath dry-redirect). The six bin.* tags publish null
// until this says true -- "the recipe's bin table was loaded" is a source
// question, same philosophy as every other liveness key here.
void SetWebBinSelLoaded(bool loaded);

// AI(W906-FW-TEMP2) 20260820: host-process marker that fTemp_Set->
// ReadTempFile(true) actually ran AND found Temperature.Data (wb_serve calls
// it in the SAME DataPath-redirected window as bin.* above, then reads
// golden's own iSendChangeTempError signal to tell "ran and found the file"
// from "ran, file missing, Temperature.* untouched"). Gates temp.sv/soak/mode.
void SetWebTempLoaded(bool loaded);

// ---------------------------------------------------------------------------
//  PUMP MODE  (wb_publish --pump)
//
//  AI(W906-SimPump) 20260813: the translated spine can be DRIVEN offline, and the
//  browser already binds machine.state -- "the big HALT/RUN word",
//  web/js/model/tagmap.js:47-48. This block is what connects the two, and it is
//  the first time anything outside tests/ advances the state machine.
//
//  AI(W906-IdlePump) 20260817: THE PUMP NO LONGER STARTS THE MACHINE.
//  PumpInit used to force SystemStart=true and fAllMotorHome=true so the engines
//  would run. The user rejected that -- "軟體開啟正常是不會 Start" -- because a
//  freshly opened BCB6 HT9045 sits IDLE until the operator presses HOME then
//  START. Both forced writes are gone. The spine is still driven every tick, the
//  master guard now correctly refuses, and the word reads "SIM IDLE".
//
//  20260917 RE-CORRECTION. The correction below was ITSELF wrong, in a way worth
//  keeping visible: it read the call site (now csystem.cpp:782) as EITHER inside
//  `#if 0` OR inside `#ifdef DEBUG_TRY_CATCH`, and picked the second. It is inside
//  BOTH, and the LOAD-BEARING one is the outer `#if 0 // GOLDEN VERBATIM` opened
//  at csystem.cpp:598 and closed at :2989 -- which contains the entire MainProc()
//  at :599. The LIVE MainProc() is :3003 and does not reach it. Therefore:
//  the counts below stand, but the CONCLUSION drawn from them does not:
//    * SystemStart=true  -- exactly ONE non-test writer, ckernel.cpp:1015, inside
//      `if(SoftStart==true)`. The OTHER caller, ckernel.cpp:4009-4010, does
//      `SoftStart=false;` on the line before the call, so it cannot take that arm.
//      And csystem.cpp:782 CANNOT either: it is inside the gated golden MainProc
//      (`#if 0` :598-:2989). The LIVE MainProc() :3003 gates that ladder out with
//      `#if 0 // TODO(W7)`, which NAMES ScanSystemSensor. => SoftStart has NO live
//      reader; S3 needs a per-tick ScanSystemSensor() call, not "one flag away".
//    * fAllMotorHome=true -- exactly ONE, csystem.cpp:9724 in DoHomeProcess, which
//      this file's own tree records as having no caller and being unreachable at
//      runtime (csystem.cpp:9632 "HAS NO CALLER TODAY", csystem.cpp:8678). So the
//      browser will eventually need HOME as well as START, in that order, which is
//      also the real machine's operating sequence.
//
//  WHY THE WORD STILL SAYS "SIM". Nothing here is sensed: there is no hardware,
//  no tray, no IC, and the machine SHAPE below is still a test fixture rather than
//  a config load. "SIM IDLE"/"SIM HALT"/"SIM RUN" plus the machine.stateSource tag
//  beside it, and the raw guard terms published individually as pump.guard.* --
//  nothing derived is published without the inputs it was derived from.
//
//  WHAT IT IS AND IS NOT. Ticks advance, mainProcCalls tracks them, exceptions
//  stay 0: that is a LIVENESS harness, and it still is one. What it is NOT, and
//  now visibly is not, is a machine cycle -- the engine cursors do not move,
//  because the guard stops DoAllProcess before any engine runs. Do not report an
//  idle pump as production.
//
//  HISTORICAL NOTE, kept because it explains a symptom someone will hit again if
//  the guard is ever forced back on: with the engines running but no tray or IC
//  seeded, DoLoad sits in case 800 "Loader has no tray" (asendic_Loader.cpp:3091)
//  and the sim ShowErrorMessage answers K_RETRY every time
//  (canary_support.cpp:97), so the console fills at roughly 2 lines/sec with
//  `[ShowErrorMessage] Code=MES0920 KCode=5 Pos=168` (Pos 168 == MMTrayY_Car,
//  cmydef.cpp:2537; KCode 5 == K_RETRY|K_CLEAN_OUT, cmydef.cpp:337/339). That is
//  a machine asking a question with nobody to answer it -- never a hang.
// ---------------------------------------------------------------------------

// Put the translated spine into the offline OPENED-BUT-NOT-STARTED fixture and
// reset every engine cursor. Call ONCE, before the first PumpTick().
//AI(W906-IdlePump) 20260817: this used to say "Run" fixture. It no longer starts
// the machine -- see the block above.
//
// ORDERING IS LOAD-BEARING: call this AFTER any config load, never before.
// ReadGeneralIni OVERWRITES five of the very globals set here -- AUTO_EMPTY_COLOR   // AI(W906-INBOX114) 20260929: stale: PumpInit no longer pins machine shape (the 12 pins are retired, WebBridgeTags.cpp:569-583), so the config values stand; the order is still load-bearing because the sim canary and InitAllProcessTask read config. Real reads now: database.cpp:537/:547/:570/:574/:713/:826/:1553
// (database.cpp:454), TRAY_VIBRATION (:593), USE_OUT_SORT_ARM (:703),
// MOTION_CARD_TYPE (:1079), AUTO3_IS_MAGAZINE (:1430) -- so the wrong order
// silently changes the tick shape with everything still green.
//
// Returns false, having changed NOTHING, if the sim canary does not hold; `whyNot`
// then carries the reason. A false return means DO NOT PUMP.
bool PumpInit(std::string& whyNot);

// One tick of the spine. Bounds the tick-oracle trace, then MainProc() inside a
// try/catch -- the compiled MainProc has none of its own, because DEBUG_TRY_CATCH
// is defined nowhere in the build (csystem.cpp:3019-3023). Never throws.
// No-op unless PumpInit() has succeeded.
void PumpTick();

// True once PumpInit() has succeeded. PublishHandlerTags() consults this itself,
// so the publish call needs no extra argument.
bool PumpActive();

// Publisher-side telemetry, for the tool's log line. All zero/false when not
// pumping. This exists so the TOOL never has to include csystem.h: keeping the
// god-stack confined to this translation unit is the same rule the file header
// states, and a printf is not a good enough reason to break it.
//
// `exceptions` is the one to watch. A publisher can serve frames perfectly while
// every single tick throws, and from the browser those two look identical.
struct PumpStats {
    unsigned long long ticks;
    unsigned int       mainProcCalls;   // golden's own counter, csystem.h:52
    unsigned long long exceptions;      // ticks that threw out of MainProc()
    bool               alive;           // IsMainProcAlive(60), csystem.h:56
};
PumpStats PumpTelemetry();

// How many of the tags this file knows about currently have a loaded source.
// Reported so "coverage" is a measured number rather than an impression.
//
//AI(W906-SimPump) 20260813: this counts MACHINE DATA SOURCES ONLY, and therefore
// excludes the 18 clock/state/pump tags added with pump mode -- those are the
// publisher's own clock and telemetry, not readings taken from the machine, so
// counting them would raise `live` without a single new machine source having been
// read. `total` here is consequently NOT the number of tags on the wire; for that,
// use PublishHandlerTags()'s return value.  MEASURED 20260917: 4,608 on the wire.
struct TagCoverage {
    std::size_t total;   // machine-sourced tags this file publishes
    std::size_t live;    // ...of which carry a real value right now
};
TagCoverage HandlerTagCoverage();

}  // namespace ht9045

#endif  // WEBBRIDGETAGS_H
