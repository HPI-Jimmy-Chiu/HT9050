// =============================================================================
//  WebBridgeTags.cpp -- machine globals -> webbridge::TagSnapshot.
//  Contract and the measured live/dead source inventory are in WebBridgeTags.h.
//  Read that first; the "publish null, never 0" rule is the whole design.
// =============================================================================
#include "WebBridgeTags.h"

#include "database.h"
#include "cprod.h"
#include "Config.h"
#include "LastSet.h"
#include "cmydef.h"
#include "cpublic.h"        // AI(W906-LOT-W1) 20260919: GetTimeInfo()
//AI(W906-SimPump) 20260813: pump mode needs the spine entry + instrumentation
// (MainProc/InitAllProcessTask/GetMainProcCallCount/IsMainProcAlive), the
// machine-shape enums the guard fixture pins (eartUninstall/NonVibration), and
// bShuttleShake, whose home is the shims TU rather than a golden header.
#include "csystem.h"
#include "forms/fMain.h"      // AI(W906-FW1d) 20260820: fMain->cbUserSelect (user.level mirror)
#include "forms/fShowBinSelect.h" // AI(W906-FW-BIN1) 20260820: fShowBinSelect->MyBinSel (bin.* captions)
#include "csystem_shims.h"
#include "MachineType.h"
#include "myswitch.h"     // AI(W906-FW-1Tb) 20260915: SW[] (:43) for light.off
#include "aHotPlateSubstrate.h"  // AI(W906-P1-HASIC) 20260919: InArmSuck / OutArmSuck (:624)
#include <chrono>           //AI(W906-P1) 20260923: FlushFlag 翻轉段的掛鐘；放在原本的空白行上，不移動本檔其後的行號
#include "WebBridge/TagValue.h"
#include <cstdlib>          //AI(W906-Q34-9) 20260923: std::strtol（1203PTS-1 的 doPoints），明寫出來不靠間接 include；放在原本的空白行上，不移動本檔其後的行號
#include <cstdio>
#include <ctime>
#include <vector>
#include "EtherCAT/Pci1203Monitor.h"
//   AI(W906-BU-C-P4) 20260918: the CONTROL header joins the monitor's.
//
//   It used to say "the MONITOR header only. Pci1203Control.h is NOT included --
//   that surface is BU-C4 and its tags are gated below."  BU-C4 landed on
//   20260918 (the write surface is in the tree and compiles on BOTH sides of
//   HAVE_PCI1203, verified 0 diagnostics each way), so that reason is gone.
//
//   ⓘ What this include buys is READ ONLY.  Everything the un-gated block below
//   touches is a const accessor -- Pci1203Control() is literally
//   `{ return g_control; }` (Pci1203Control.cpp:2993), Pci1203ControlLinked() is
//   a compile-time constant (:2995), and IsDryRun/acceptedCount/refusedCount/
//   issuedCount/last() are all `const`.  Publishing these tags cannot arm, open,
//   or command anything; it only makes the armed state VISIBLE, which is the
//   safer direction given the 20260918 "甲" ruling left LIVE on.
#include "EtherCAT/Pci1203Control.h"

//AI(W906-SimPump) 20260813: engine cursors, declared here rather than by including
// their owning headers. Every one is a plain int, so an extern declaration is
// exact -- and pulling the real headers in would be actively risky: iArmTask's
// home, aHotPlateSubstrate.h, also carries a SECOND TMyKitSuck with a DIFFERENT
// layout from mykitsuck.h's, and 14 globals collide across the two. Picking the
// wrong one links cleanly and reads every field at the wrong offset. A plain
// `extern int` cannot be wrong that way.
extern int iArmTask;              // in-arm      -- aHotPlateSubstrate.h:945
extern int OutArmTask;            // out-arm     -- aoutarm.h:77
extern int AutoSHT1Task;          // shuttle 1   -- acarry.h:39
extern int AutoSHT2Task;          // shuttle 2   -- acarry.h:40
extern int iTestHeadMotorTask;    // index/tester-- atester.h:169
extern int LoadTask;              // loader      -- asendic_Loader.h:53
// CatchTrayTask is already declared by csystem.h:63 (also acatchtray.h:48).

//AI(W906-SimPump) 20260813: the tick oracle. CSYSTEM_TICK_ORACLE is compiled
// UNCONDITIONALLY into ht9045_sm (CMakeLists.txt:2196), so these two symbols are
// present in every binary that links it -- no test-only lib variant is involved.
// That also means g_csystemTrace GROWS ON EVERY ENGINE CALL for the life of the
// process (csystem.cpp:200) and csystemTraceClear() is the ONLY thing that bounds
// it. Calling it each tick is not optional in a process that runs for hours.
extern std::vector<int> g_csystemTrace;   // csystem.cpp:198
extern void csystemTraceClear();          // csystem.cpp:199

namespace ht9045 {

// AI(W906-P4-SECS) 20260920: 額外發布者。見 PublishHandlerTags() 內的註解。
static ExtraTagPublisher g_extraPublisher = 0;

void SetExtraTagPublisher(ExtraTagPublisher fn) { g_extraPublisher = fn; }  static StreamWantedHook g_streamWanted = 0;  void SetStreamWantedHook(StreamWantedHook fn) { g_streamWanted = fn; }   //AI(W906-STREAM-2B) 20260930
static PublishYieldHook g_publishYield = 0; static bool g_inPublish = false; static bool g_inPublishYield = false;  void SetPublishYieldHook(PublishYieldHook fn) { g_publishYield = fn; }  static void PublishYield_() { if (g_publishYield == 0 || g_inPublishYield) return; g_inPublishYield = true; g_publishYield(); g_inPublishYield = false; }   // AI(W906-LAT-1) 20260925: the publish's yield points (WebBridgeTags.h SetPublishYieldHook). g_inPublishYield: a hook that somehow reached a yield point again is not re-entered; g_inPublish: see the guard at the top of PublishHandlerTags(). On the old blank line, so no line below moves

using webbridge::TagValue;

namespace {

// ---------------------------------------------------------------------------
//  Source liveness predicates.
//
//  Each answers "has anything actually loaded this?" for one source. They are
//  deliberately separate from the value getters: a getter that also decided
//  liveness would make it far too easy to fall back to 0 and call it a reading.
//
//  These are conservative on purpose. A source that MIGHT be loaded is treated
//  as dead, because a wrong number on an operator screen costs more than a
//  missing one.
// ---------------------------------------------------------------------------

// IniConfig's STRING fields are set by ReadLastSetIni (cprod.cpp). Its hundreds
// of numeric/bool feature flags are NOT -- those come from cConfiguration.cpp,
// which is untranslated. So liveness has to be asked per field group, not for
// "IniConfig" as a whole.
bool IniConfigStringsLoaded()
{
    return IniConfig.sMachineType.Length() > 0;
}

// CUSTOMER_CODE is assigned inside ReadGeneralIni. 0 is also a legal customer
// code in principle, but in this tree it is the not-yet-read value -- and
// treating a real 0 as "unknown" is the safe direction of the two.
bool CustomerCodeLoaded()
{
    return CUSTOMER_CODE != 0;
}

// LastSet arrives as one raw blob from system\lastdata.dat. An all-zero struct
// means no file was read; a partially-zero one is normal (that file is only
// ~7% non-zero on a typical box), so liveness is a property of the BLOB, not of
// any single field.
bool LastSetLoaded()
{
    const unsigned char* raw = reinterpret_cast<const unsigned char*>(&LastSet);
    for (std::size_t i = 0; i < sizeof(LAST_GENERAL_SET); ++i) {
        if (raw[i] != 0) return true;
    }
    return false;
}

// AI(W906-FW-TEMP2) 20260820: the answer flips from hardcoded false to a real
// raw-byte scan (same idiom as CosFunctionLoaded/LastSetLoaded below) now that
// wb_serve actually calls fTemp_Set->ReadTempFile(true) -- see
// WebBridgeTags.h's AI(W906-FW-TEMP2) block for the wiring and the Init()
// audit that cleared it. This predicate answers "has ANYTHING written to the
// SYSTEM_TEMPERATURE struct", which is broader than the 3 fields this file
// stages (ReadTempFile touches ~90 of them) -- see its one caller below for
// why that breadth is exactly the point.
bool TemperatureLoaded()
{
    const unsigned char* raw = reinterpret_cast<const unsigned char*>(&Temperature);
    for (std::size_t i = 0; i < sizeof(SYSTEM_TEMPERATURE); ++i) {
        if (raw[i] != 0) return true;
    }
    return false;
}

// UN150Read[] holds every temperature present-value. AI(W906-FW-TEMP1)
// 20260820 re-measured this rather than trust the prior wave's "no controller
// polling in the port" -- bthermo.cpp DOES write it in ~80 places now, but
// every path to those writes is unreachable offline: the owning thread
// (THeaterThread::Execute, uHeaterThread.cpp:397 -- was :366, drifted; re-measured 20260915) is never started (Resume()
// is a documented no-op, :389-392); the two other writers (uTemp_Set.cpp:
// 7228-7229, forms/fLotInfo.cpp:1291-1297) are both `#if 0`'d out; and
// ShowThermo's SOFT_SIMULTE debug-fill branch (cTemperFrom.cpp:338-348) is
// both call-unreachable outside tests/ and compiled out (SOFT_SIMULTE is
// commented at MachineType.h:48). Checked at runtime rather than hard-coded
// false, because this one will light up the moment any of those three
// reachability gaps closes.
bool TemperaturePvLoaded()
{
    for (int i = 0; i < tcTotalCount; ++i) {
        if (UN150Read[i] != 0.0) return true;
    }
    return false;
}

// The 140 customer-profile flags. CustomerFunctionSelect() runs, but every
// branch in it tests an IniConfig flag that is not loaded, so it sets nothing:
// 0 of 488 bytes, measured.
bool CosFunctionLoaded()
{
    const unsigned char* raw = reinterpret_cast<const unsigned char*>(&CosFunction);
    for (std::size_t i = 0; i < sizeof(CosFunction); ++i) {
        if (raw[i] != 0) return true;
    }
    return false;
}

// ---------------------------------------------------------------------------
//  Staging helpers. Every one takes a liveness flag, so that the null-vs-value
//  decision is visible at each call site instead of buried in a getter.
// ---------------------------------------------------------------------------
void stageStr(webbridge::TagSnapshot& s, const char* tag, bool live, const AnsiString& v)
{
    s.stage(tag, live ? TagValue::makeString(std::string(v.c_str()))
                      : TagValue::makeNull());
}

void stageInt(webbridge::TagSnapshot& s, const char* tag, bool live, long long v)
{
    s.stage(tag, live ? TagValue::makeInt(static_cast<std::int64_t>(v))
                      : TagValue::makeNull());
}

//AI(W906-FW-TEMP2) 20260820: double needs its own stager -- temp.sv/temp.soak
// (Temperature.fWorkTemperBase/fSoakTime, both `double`, cprod.h:1392/1378)
// are the first fields this file publishes that are not int/bool/string.
// TagValue already carries a Double alternative (WebBridge/TagValue.h) so no
// int/string workaround is needed; routing a double through stageInt would
// truncate a fractional degree, which is exactly the kind of silently-wrong
// number this file's whole design exists to prevent.
void stageDouble(webbridge::TagSnapshot& s, const char* tag, bool live, double v)
{
    s.stage(tag, live ? TagValue::makeDouble(v) : TagValue::makeNull());
}

void stageNull(webbridge::TagSnapshot& s, const char* tag)
{
    s.stage(tag, TagValue::makeNull());
}

//AI(W906-SimPump) 20260813: bool needs its own stager -- routing a bool through
// stageInt would publish 1/0, and the UI's boolean modes test truthiness
// (data-lit / btn--on, web/js/ui/bind.js:141-149) where a JSON number and a JSON
// bool are not interchangeable for a reader trying to tell a flag from a count.
void stageBool(webbridge::TagSnapshot& s, const char* tag, bool live, bool v)
{
    s.stage(tag, live ? TagValue::makeBool(v) : TagValue::makeNull());
}

// The home screen's tags whose source is measurably not loaded. Publishing them
// as null is not a placeholder -- it is the correct value, and it makes the
// extent of the gap visible on the screen instead of hiding it behind zeros.
const char* const kUnloadedTags[] = {
    // AI(W906-FW-TEMP2) 20260820: temp.sv/soak/mode WIRED this wave (see the
    // dedicated block in PublishHandlerTags below) -- moved OUT of this
    // array. temp.pv + the 8 zone.* tags key on UN150Read[], which stays
    // genuinely dead: its only writer thread never starts offline, its other
    // two write sites are `#if 0`'d out, and its SOFT_SIMULTE debug-fill
    // branch is both unreachable and compiled out. Full evidence +
    // verification commands: WebBridgeTags.h's AI(W906-FW-TEMP1)/(FW-TEMP2)
    // blocks.
    "temp.pv",
    "zone.hotplate.1", "zone.hotplate.2",
    "zone.shuttle.1",  "zone.shuttle.2",
    "zone.index.1",    "zone.index.2",
    "zone.heatgun.1",  "zone.heatgun.2",
    // tester identity comes from TestIF_File, which is not loaded
    "tester.name",
    //AI(W906-FW-1Tc) 20260915: the old reason here was "driven by IO output
    //   state; the IO layer is offline".  That points at the wrong end of the
    //   chain.  Measured, the chain is:
    //
    //     LastSet.MessageLight[RunState][0..2]   <- config, LOADED (LastSet.h:115,
    //                                               arrives in the lastdata.dat blob)
    //       + RunState + FlushFlag               <- KERNEL TICK products
    //         -> fMain->ledRed/Yellow/Green->Value      (ckernel.cpp:1748+, TRANSLATED)
    //           -> SW[SwTowerRed/Yellow/Green]          (golden ckernel.cpp:915-917)
    //
    //   So the IO switch is the far END of the chain, not its source, and the
    //   whole chain IS translated in this tree.  What is missing is that
    //   **the kernel tick does not run in wb_serve** -- the same root as pump.*
    //   (PumpInit() is called only from tools/wb_publish.cpp).  RunState and
    //   FlushFlag therefore never advance, so ledX->Value would be "the lamp
    //   state for an initial RunState nobody set".
    //
    //   ⚠ NOT wired for that reason, and note it is NOT the same situation as
    //   light.off above: there the liveness predicate (SW[].Enable) can become
    //   true if the IO table is ever loaded, so the wiring self-corrects.  Here
    //   the predicate would be "is the tick running", and under wb_serve that is
    //   structurally never -- wiring it would add code that can never fire in
    //   the process that actually serves the browser.
    /* "tower.red", "tower.amber", "tower.green", */  // AI(W906-TOWER) 20260924: 移出 null 清單。上面那段「kernel tick 在 wb_serve 不跑（PumpInit 只在 wb_publish）」的前提已過期 —— wb_serve 開機就 PumpInit，印「spine pump: ARMED -- MainProc() runs on the 500 ms server tick」，gdb 實測 ledGreen／ledYellow 隨 START／PAUSE 變（20260924）。改在 pump 區段以 pumping 為 liveness 發布（找 AI(W906-TOWER)）
    // AI(W906-FW-1Ta) 20260915: the reason on the next group USED TO READ
    //   "run/mode panels need IniConfig feature flags (cConfiguration.cpp
    //    untranslated)".  BOTH HALVES OF THAT ARE WRONG, measured:
    //
    //   (a) cConfiguration.cpp IS translated -- 6,722 lines, and it is in the
    //       build (CMakeLists.txt:2387, landed by AI(W906-FW3-Config-WA)
    //       20260820, i.e. the same week that comment was written).
    //   (b) These tags do not read IniConfig at all.  Per the campaign's own
    //       binding table (docs/web-fw-legacy/js/model/tagmap.js:56-88) every
    //       one of them maps to an fMain WIDGET:
    //         run.ft/rt/offline -> Panel2.palFT / palRT / palOffLine
    //         runmode.*         -> palSetting.palRunMode_1 / palNormal / palPrime
    //         light.off         -> palSetting.spbLight   (TSpeedButton)
    //         fan.off           -> palSetting.spbFan     (TSpeedButton)
    //
    //   ★ THE RULE THAT MATTERS FOR WHOEVER WIRES THESE:
    //     a widget DISPLAYS something; it is not the state.  Do not wait for
    //     the widget and do not stub it -- find the state it displays and
    //     measure whether THAT is live.  Three outcomes, all seen here:
    //       state is live            -> wireable
    //       state absent from port   -> genuinely missing, say so
    //       state is an IO/thread product and that layer is offline
    //                                -> null IS the correct answer
    //
    //   ★★ AI(W906-FW-1Tc) 20260915 -- THE HALF THAT RULE WAS MISSING, and it
    //   nearly made me wire tower.* backwards one wave after writing it:
    //
    //     BEFORE looking for "the state behind the widget", establish WHICH END
    //     THE WIDGET IS ON.  A widget can be the SINK or the SOURCE, and the
    //     discriminator is the direction of assignment:
    //
    //       widget is the SINK    spbLight->Caption = "Light OFF";
    //       (state lives in SW[]) SW[SwCCDLight].Off();
    //                             -> read SW[SwCCDLight].OutValue    (light.off)
    //
    //       widget is the SOURCE  SW[SwTowerRed].OnOff(fMain->ledRed->Value);
    //       (SW[] is the OUTPUT)  -> reading SW[] here reads the OUTPUT, which is
    //                                only correct while the writer loop runs
    //                                                                  (tower.*)
    //
    //     Both look like "a widget and a switch next to each other".  Only the
    //     assignment direction tells you which one holds the truth.
    //
    //   Measured per tag (20260915):
    //     fan.off   -> golden's state is LastSet.bBigFan (main.cpp:26049; the
    //                  caption is derived from it).  `bBigFan` DOES NOT EXIST in
    //                  this tree's cprod.h/.cpp (0 hits), and it is not a key in
    //                  the machine's own config/LastSet.ini either -- in golden
    //                  it is a pure runtime toggle.  So: missing struct field.
    //     light.off -> WIRED 20260915 (FW-1T-b), see the stage call below.
    //                  The open question from FW-1T-a ("is the switch state
    //                  meaningful with the IO layer offline?") is now ANSWERED:
    //                  TMySwitch::On()/Off() (myswitch.cpp:74/:124) set OutValue
    //                  UNCONDITIONALLY, BEFORE the `if(Enable==false) return;`
    //                  that guards the hardware drive.  So OutValue is the
    //                  COMMANDED state and it is valid offline; Status()
    //                  (:176) is the hardware readback and is not.
    //     run.* / runmode.* -> TRACED 20260915 (FW-1T-d).  Single source for all
    //         six: the bare global `iTestRunMode` (cmydef.h:3111, defined
    //         cmydef.cpp:3339 `int iTestRunMode=0;` with the comment `//0=RT`).
    //         golden assigns it 29 times (FT / RT / FT_ART / RT_ART / FT_MRT /
    //         RT_MRT / OffT) and EVERY ONE of those is inside main.cpp's
    //         SetRunStartMode/SetTestRunMode region, i.e. the untranslated form.
    //         Measured: **zero writers in this tree** (only the definition).
    //
    //         Its deriving function is golden SetTestRunMode() (main.cpp:1117-1338,
    //         222 lines; :233 is only the extern).  Translating it would NOT make
    //         these tags live: it derives iTestRunMode from LastSet.iRunStartMode,
    //         and that input is itself FROZEN -- loaded once from lastdata.dat and
    //         never updated, because SetRunStartMode(int) is `{}`
    //         (aHotPlateSubstrate.cpp:1074).  See START_CAMPAIGN_PLAN.md §5.4.
    //
    //         ⚠ Publishing them from the default would be worse than null:
    //         iTestRunMode==0 means RT, so `run.rt` would read TRUE on every
    //         machine -- a default presented as a measurement.
    //
    //         ⚠ docs/RECON_binstar_datasource.md:144 gave a different reason
    //         for not translating SetTestRunMode ("-> ReadLastDataFile() ->
    //         hardcoded lastdata.dat, and it calls WriteLastDataFile()
    //         internally").  Measured against the body: :1117-1338 contains ZERO
    //         Read/WriteLastDataFile calls and calls only IsHanaArtAvailable()
    //         and IsPrimeTest().  ⚠ The DANGER IS REAL but belongs to a
    //         DIFFERENT function -- that same doc elsewhere establishes that
    //         ReadLastDataFile() does conditionally call WriteLastDataFile().
    //         It was attached to the wrong callee, not invented.  Corrected in
    //         place 20260915; the conclusion (do not translate it yet) survives
    //         on the frozen-input reason above.
    "run.ft", "run.rt", "run.offline",
    "runmode.value", "runmode.normal", "runmode.prime",
    //AI(W906-FW-1Tb) 20260915: light.off MOVED OUT -- wired below.
    // fan.off stays: its state is LastSet.bBigFan (golden main.cpp:26049)
    // and that member DOES NOT EXIST in this tree (0 hits in cprod.h/.cpp),
    // nor is it a key in the machine's own config/LastSet.ini -- in golden
    // it is a pure runtime toggle. Missing struct field, not a load gap.
    "fan.off",
    // AI(W906-FW-1Ta) 20260915: same correction of kind for the group below --
    //   these are fMain display widgets (tagmap.js:100-112: pnlPowerSaving /
    //   pnlCleanCount / lbEPenconder / lbCheckSafeDoorDisable / labRTCCD /
    //   lbFTPOnoffStatus / lbl_TriTempState / pnlSaveSummary; three of the
    //   eleven -- rms / indexTime / uph -- have NO design-time caption match at
    //   all and are marked unresolved in that table).
    //
    // AI(W906-FW-1Te) 20260915 -- TRACED, and the blocker is NOT what either the
    //   original note or my own FW-1T-a correction assumed.  It is a WIRE
    //   CONTRACT MISMATCH, and resolving it is a design decision, not a
    //   translation.  Three measurements:
    //
    //   (1) THE TAGS ARE STRINGS, not booleans.  The browser's state model types
    //       every one of them as the widget's CAPTION TEXT
    //       (docs/web-fw-legacy/js/model/state.js:127-137, e.g.
    //        "status.cleanCount": "Cleaned Count 61 / 300",
    //        "status.rtc": "RTC Off Line", "status.uph": "UPH: 155").
    //
    //   (2) BUT FOR SEVERAL OF THEM GOLDEN PUTS THE STATE IN `Visible`, NOT IN
    //       THE TEXT.  lbl_TriTempState and lbCheckSafeDoorDisable each have
    //       exactly ONE assignment in all of golden main.cpp and it is a
    //       visibility one (:9618 `Visible=(Tri_Temp_Machine==1)`,
    //       :18604 `Visible=flag2`); their captions are design-time literals in
    //       the .dfm that golden never rewrites.  A hidden label and a shown
    //       label therefore carry the SAME string -- the web's string model
    //       cannot distinguish them without a convention.
    //
    //   (3) AND THE SOURCE IS ACTUALLY LOADED, contrary to the old note.
    //       Tri_Temp_Machine is read from Gerneral.ini at database.cpp:1587
    //       (`CheckAndReadIniDataGeneral("System","Tri_Temp_Machine",0)`), which
    //       runs inside ReadGeneralIni() i.e. inside LoadMachineConfig().  On
    //       THIS machine it reads 0 (feature absent) -- see docs/GL_CAMPAIGN_PLAN.md:495.
    //
    //   ⇒ WHAT HAS TO BE DECIDED BEFORE ANY OF THESE CAN BE WIRED:
    //       does C++ publish the CAPTION (duplicating a .dfm literal into code,
    //       with null meaning "hidden"), or does it publish a BOOLEAN/STATE and
    //       let the browser own the wording?  The campaign's own direction
    //       (browser renders, C++ publishes state) argues for the second, but
    //       that CHANGES THE WIRE CONTRACT the browser already codes against.
    //       Deliberately NOT decided here -- inventing a contract is exactly the
    //       class of thing this file's banner exists to prevent.
    //
    //   ⚠ The three unresolved ones (rms / indexTime / uph) are a different
    //   problem again: they have no design-time widget at all, so there is no
    //   caption to publish and no visibility to read.  Those genuinely need the
    //   untranslated main.cpp composition.
    "status.indexTime", "status.uph",
    "status.powerSaving", "status.cleanCount", "status.ep",
    "status.safeDoor", "status.rtc", "status.rms", "status.ftp",
    "status.triTemp", "status.saveSummary"
};

const std::size_t kUnloadedCount = sizeof(kUnloadedTags) / sizeof(kUnloadedTags[0]);

// ---------------------------------------------------------------------------
//  PUMP MODE internals.  Contract, and the reachability measurement that forces
//  the "SIM" prefix, are in WebBridgeTags.h -- read that before changing any of
//  this.  AI(W906-SimPump) 20260813.
// ---------------------------------------------------------------------------

// Engine ids the tick oracle records; must match the enum at csystem.cpp:205-208.
enum {
    CT_SENSORSCAN = 1, CT_DOLOAD = 2, CT_DOINARM = 3, CT_SHT1 = 4, CT_SHT2 = 5,
    CT_SHT3 = 6, CT_TESTHEAD = 7, CT_CATCHTRAY = 8, CT_OUTARM = 9, CT_SORTARM = 10
};

bool               g_pumpActive     = false;
unsigned long long g_pumpTicks      = 0;
unsigned long long g_pumpExceptions = 0;
int                g_lastTickKind   = 0;   // 1 = A tick, 2 = B tick, 0 = neither

bool traceHas(const std::vector<int>& v, int id)
{
    for (std::size_t i = 0; i < v.size(); ++i) {
        if (v[i] == id) return true;
    }
    return false;
}

// Classify one tick, same rule as tests/test_w6_6_csystem_cycle.cpp:161-170.
// bDoProcess (csystem.cpp:4220, toggled :4452) makes the spine alternate:
//   A = DoInArm + SHT1, no OutArm/SHT2      B = DoOutArm + SHT2, no DoInArm/SHT1
// "neither" is not an error on a guard-truncated tick -- it is the honest answer.
int classifyTick(const std::vector<int>& v)
{
    const bool inarm  = traceHas(v, CT_DOINARM);
    const bool sht1   = traceHas(v, CT_SHT1);
    const bool outarm = traceHas(v, CT_OUTARM);
    const bool sht2   = traceHas(v, CT_SHT2);
    if ( inarm &&  sht1 && !outarm && !sht2) return 1;
    if (!inarm && !sht1 &&  outarm &&  sht2) return 2;
    return 0;
}

// The three master-guard terms, re-read live. This is the ONLY thing behind the
// state word -- csystem.cpp:4239 (golden csystem.cpp:10049), re-evaluated by the
// spine 8 times per tick, so a mid-tick flip truncates the tick.
bool guardAllowsEngines()
{
    return SoftStop == false && SystemStart == true && fAllMotorHome == true;
}

}  // namespace

// ---------------------------------------------------------------------------
//AI(W906-SimPump) 20260813: see the PUMP MODE block in WebBridgeTags.h.
bool PumpInit(std::string& whyNot)
{
    // ---- sim canary -------------------------------------------------------
    // LastSet.iRealDummy==DUMMY(0) != REALLY, together with a non-Contec motion
    // card, is what makes CheckInShuttleSensor_Latch return 1 (Finish) at
    // ainarm9045.cpp:1891-1893 BEFORE any CCLink/Ltc/MOT body runs. Those bodies
    // have never executed offline. If a config load has flipped both terms, the
    // interlock is live and pumping would be the first thing to ever run it --
    // refuse rather than find out.
    if (MOTION_CARD_TYPE == MotionCard_Contec && LastSet.iRealDummy == REALLY) {
#ifdef SOFT_SIMULTE
        // AI(W906-ST-S3-B2a) 20260918: under SOFT_SIMULTE the canary's premise is
        //   GONE, so it does not refuse -- it reports and continues.
        //
        //   The canary is NOT deleted and NOT bypassed by a flag of our own. It
        //   still runs and still says what it found; it just stops being a reason
        //   to refuse, because SOFT_SIMULTE is the mechanism this codebase has
        //   always used to simulate machine motion (BCB6 development ran this
        //   way). Inventing a second, bespoke "ignore the canary" switch next to
        //   an existing, designed one would have been the wrong answer -- the
        //   user's model is that there are exactly TWO builds: SOFT_SIMULTE on
        //   = simulation, off = it runs on a machine.
        //
        //   WHY THE PREMISE IS GONE, traced line by line 20260918 rather than
        //   assumed. The canary exists because with both terms true, golden's own
        //   entry guard at ainarm9045.cpp:1896 stops short-circuiting and
        //   CheckInShuttleSensor_Latch runs its 364-line body. Under SOFT_SIMULTE
        //   that body is a different program:
        //     * ainarm9045.cpp:1986 `iSHAutoLtcErr = DoCheckShuttle1ICByLTC_
        //       AutoLatch(...)` sits inside `#ifndef SOFT_SIMULTE` -> COMPILED OUT.
        //       That call is iSHAutoLtcErr's ONLY non-zero source; the variable is
        //       `static int iSHAutoLtcErr=0` (:1907). So it stays 0.
        //     * therefore the `Task=1260` error branch never arms, so `case 1265`
        //       is unreachable, so its four ShowErrorMessage(JAM0401/0403/0404/
        //       0406) cannot fire. Those alarms were the one real risk here --
        //       wb_serve forwards them to the browser and blocks, and unattended
        //       it sim-answers K_RETRY forever (see AI(W906-IdlePump) 20260817).
        //     * the 14 MOT[..].MotorMove() calls are stubs regardless:
        //       Motor/mymotor.cpp:843 assigns Position=p and returns 1 when
        //       Motor==NULL||!Enable, and its real-driver arm is an untranslated
        //       TODO. No vendor call exists on either arm.
        //     * measured in the same body: 0 file writes, 0 IO switch writes.
        //
        //   ⇒ Under SOFT_SIMULTE nothing in that body can move, write or alarm.
        //   With SOFT_SIMULTE OFF the #else below still refuses, unchanged.
        std::printf("sim canary: tripped but NOT refusing -- SOFT_SIMULTE is on.\n"
                    "            (MOTION_CARD_TYPE==Contec AND iRealDummy==REALLY are both\n"
                    "             true, but ainarm9045.cpp:1986's latch check is compiled out,\n"
                    "             so the JAM path is unreachable and MotorMove is a stub.)\n");
        whyNot.clear();
#else
        whyNot = "sim canary violated (MOTION_CARD_TYPE==Contec AND "
                 "LastSet.iRealDummy==REALLY): the hardware interlock at "
                 "ainarm9045.cpp:1891 is live -- refusing to pump";
        return false;
#endif
    }

    // ---- the offline fixture: OPENED, NOT STARTED --------------------------
    //AI(W906-IdlePump) 20260817: this used to be the "Run" fixture and it FORCED
    // SystemStart=true and fAllMotorHome=true. The user rejected that, and was
    // right to: "軟體開啟正常是不會 Start" -- a freshly opened BCB6 HT9045 sits
    // IDLE waiting for the operator to press HOME then START. Forcing the guard
    // made the port show a machine mid-production-attempt, which is a state the
    // real machine is never in on startup, and the visible symptom was a console
    // filling with `[ShowErrorMessage] Code=MES0920 KCode=5 Pos=168` -- DoLoad
    // case 800 "Loader has no tray" (asendic_Loader.cpp:3091) retrying forever
    // because the sim ShowErrorMessage always answers K_RETRY.
    //
    // The two lines are simply GONE, not replaced by false: these are globals the
    // data layer already zero-initialises, and writing false here would imply this
    // function had a say in the machine's run state. It does not any more.
    //
    // WHAT STILL HAPPENS EVERY TICK, so this is not a downgrade to nothing:
    // MainProc (csystem.cpp:3003) is entered, its InitialOK head guard (:3005) passes,
    // and DoAllProcess (:4215) returns at the master guard (:4239) before DoLoad. (The
    // one ScanSystemSensor call, :778, is #if 0.) So ticks, mainProcCalls, exceptions=0
    // and alive still prove the spine is live; the engine cursors correctly do not
    // move, because no engine ran.
    //
    // HOW START WILL ARRIVE (SCOPE.md section 2.6, the browser->core write path):
    // set SoftStart=true. ScanSystemSensor already tests `if(SoftStart==true)`
    // (ckernel.cpp:816) and, on that arm, runs golden's REAL admission sequence --
    // lamp reset, shuttle floodgates, DoInArm_SuckerMap, the safe-door check -- and
    // sets SystemStart itself at ckernel.cpp:1015. That path is LIVE in this tree
    // and one flag away; do NOT reintroduce a direct SystemStart write.
    //
    // The rest of this block is unchanged and is NOT run state -- it is machine
    // SHAPE (which machine this is), still verbatim from
    // tests/test_w6_6_csystem_cycle.cpp:135-156 and still load-bearing on tick
    // shape once something does start the machine.
    // AI(W906-BU-C-P3) 20260918: the csystem.cpp line citations in the block below
    // were re-measured, one grep each. Eleven had drifted (csystem.cpp gained
    // lines above 4246 after they were written); InitialOK :3005 and SoftStop
    // :4239 were still correct and are untouched.
    //
    // ⚠ The drift was NOT uniform -- measured deltas were +2, +3, +4, +6 and -5.
    // Adding a constant to all of them would have produced a different wrong
    // number for most, and a more convincing one. Re-measure, never shift.
    //
    // Each citation points at the GUARD that reads the global, not at the call
    // the guard protects -- the guard is what setting this global changes.
    InitialOK                 = true;            // MainProc head guard, csystem.cpp:3005
                                                 //   -- kept: a real opened program
                                                 //   DOES finish initialisation.
    SoftStop                  = false;           // master guard, csystem.cpp:4239
                                                 //   -- kept: "not soft-stopped" is
                                                 //   the state of a fresh boot.
//  bShuttleShake             = false;           // else shuttle+index block skipped, :4269   // AI(W906-INBOX114) 20260929: pin RETIRED -- runtime state golden never loads (static false, csystem_shims.cpp:38); InitAllProcessTask below resets it again (InitInArmTask, ainarm2.cpp)

//  TrayForm.bEnableAMR       = false;           // plain DoLoad path, :4246   // AI(W906-INBOX114) 20260929: pin RETIRED -- recipe Tray.Data [AMR] "Enahle AMR" (golden cTrayAssignment.cpp:259-263; port FileRW/TrayForm.gen.inc:962); the pin switched OFF golden's AMR docking interlocks around DoLoad / DoAutoReceiveBinTray / DoAutoEmpty / DoAutoColor
//  USE_OUT_SORT_ARM          = eartUninstall;   // no SHT3/SortArm, :4290/:4319   // AI(W906-INBOX114) 20260929: pin RETIRED -- Gerneral.ini [OutSortArm] USE_OUT_SORT_ARM (database.cpp:826)

//  AUTO_EMPTY_COLOR          = 0;               // :4339 -> cmpt=3; see NOTE below   // AI(W906-INBOX114) 20260929: pin RETIRED -- Gerneral.ini [System] AUTO_EMPTY_COLOR (database.cpp:570/:574)
//  AUTO3_IS_MAGAZINE         = 0;               // no DoAuto3Magazine, :4333   // AI(W906-INBOX114) 20260929: pin RETIRED -- Gerneral.ini [System] AUTO3_IS_MAGAZINE (database.cpp:1553); the magazine engine is still a stub, so a machine with 1 gets the boot warning printed below
//  TRAY_VIBRATION            = NonVibration;    // no tray-edge cylinder loop, :4387   // AI(W906-INBOX114) 20260929: pin RETIRED -- Gerneral.ini [System] TRAY_VIBRATION (database.cpp:713)
//  SUPPORT_2_EMPTY_EMPTY     = false;           // no DoAutoEmpty1, :4415   // AI(W906-INBOX114) 20260929: pin RETIRED -- Gerneral.ini [System] SUPPORT_2_EMPTY_EMPTY (database.cpp:547)
//  bUseAuto2Empty            = false;           // no DoAuto2, :4412   // AI(W906-INBOX114) 20260929: pin RETIRED -- Gerneral.ini [System] bUseAuto2Empty (database.cpp:537)

//  bLoaderNeedTrayMustFinish = false;   // AI(W906-INBOX114) 20260929: pin RETIRED -- runtime state golden never loads (static false)
//  bAutoNeedTrayMustFinish   = false;   // AI(W906-INBOX114) 20260929: pin RETIRED -- runtime state golden never loads (static false)
//  bRunInArmAutoAlignment    = false;           // else DoAllProcess returns at :4443   // AI(W906-INBOX114) 20260929: pin RETIRED -- runtime state golden never loads (static false)
//  bRunOutArmAutoAlignment   = false;           //   BEFORE the bDoProcess toggle :4452,   // AI(W906-INBOX114) 20260929: pin RETIRED -- runtime state golden never loads (static false)
                                                 //   freezing the A/B parity.
    // NOTE, verified 20260813: the test's own comment at
    // tests/test_w6_6_csystem_cycle.cpp:146 says AUTO_EMPTY_COLOR=0 "skip[s] the
    // DoAutoReceiveBinTray loop". That comment is WRONG -- 0 takes the `<3` branch
    // at csystem.cpp:4339 and sets cmpt=3, so DoAutoReceiveBinTray(0..2) runs every
    // tick. Harmless here (it is why BinTrayTask moves at all), but recorded so the
    // next reader does not inherit the mistake.

    InitAllProcessTask();  std::printf("PumpInit: machine shape from config (INBOX 114, no pins): USE_OUT_SORT_ARM=%d AUTO_EMPTY_COLOR=%d AUTO3_IS_MAGAZINE=%d TRAY_VIBRATION=%d SUPPORT_2_EMPTY_EMPTY=%d bUseAuto2Empty=%d TrayForm.bEnableAMR=%d%s\n", (int)USE_OUT_SORT_ARM, (int)AUTO_EMPTY_COLOR, (int)AUTO3_IS_MAGAZINE, (int)TRAY_VIBRATION, (int)SUPPORT_2_EMPTY_EMPTY, (int)bUseAuto2Empty, (int)TrayForm.bEnableAMR, AUTO3_IS_MAGAZINE ? "  WARNING: AUTO3_IS_MAGAZINE=1 but the magazine engine is still a stub (csystem_shims.cpp)" : "");   // per-engine cursor reset, csystem.cpp:245   // AI(W906-INBOX114) 20260929: the values the tick now runs with (they used to be overwritten by the pins above) -- the print sits BEFORE the comment (20260929 22:2x: the first version was appended after it and so compiled as a comment)
    g_pumpActive = true;
    whyNot.clear();
    return true;
}

void PumpTick()
{
    GetTimeInfo(); if (!g_pumpActive) return;   // AI(W906-CLOCK) 20260924: 掛鐘更新移到守衛之前。golden 在 TfMain ctor（main.cpp:1683）無條件呼叫一次、ProcessTimeUpdate（:7812）週期呼叫，都不看機台初始化成不成功；原本放在 g_pumpActive 之後，非模擬建置沒有硬體時 PumpInit 失敗 ⇒ 從來不更新 ⇒ lot.start 把 LotStartTime=9999-9999-00 9999:9999:9999 寫進真實 config.ini（20260924 非模擬探針實測）。放在同一行，不移動其後行號
    { extern void W906_Timer2HeaterTick(); W906_Timer2HeaterTick(); }  { extern void W906_DTME08TimerTick(); W906_DTME08TimerTick(); }  /* AI(W906-I03) 20261002 (Ifor01): golden TfrmDTME08 TimerUpdate (100 ms, fDTME08.dfm:725-729) -- fires only after golden FormShow's arm (W906_DTME08_FormShowArm, not called yet); body forms/fDTME08.cpp EOF */  { extern void W906_TemperFromTimer1Tick(); W906_TemperFromTimer1Tick(); }  /* AI(W906-TP1) 20261002 (Ifor01): golden TfTemperFrom Timer1 (100 ms, cTemperFrom.dfm:3285-3289) -- 71 channels ShowThermo, RunInfo.ShowTempComp (SECS), CCD cooling; body cTemperFrom.cpp EOF */   // AI(W906-I01) 20261001 (Ifor01): golden TfMain::Timer2Timer 的加熱段（main.cpp:21528-21643：HotplateHeatMode／IndexHeatMode，機台沒在跑時每秒一次；本體 forms/fMain_Heater.cpp，自己限速 1000 ms、SystemStart 時不跑、自己擋例外）。在 g_pumpActive 守衛之後＝golden Timer2 的 InitialOK 守衛。筆電保留給 I-01 的空行，行號不動
    // Bounds g_csystemTrace. Not cosmetic: nothing else trims it, so skipping this
    // is an unbounded leak for the life of the process (csystem.cpp:200).
    csystemTraceClear();
    { extern void W906_St02TimersTick(); W906_St02TimersTick(); }   //AI(W906-S15) 20261001 (St02-E, claim): golden TfMain TTimers that St02 ports, each at golden's 1000 ms (MainTimersSt02.cpp): Timer8Timer (906_0625_Steven main.cpp:32154-32246) + TimerTemperatureStorageMinuteTimer (:31175-31180); S-13 TimerESD / S-14 Timer3 join there, not on this line
    //AI(W906-LOT-W1) 20260919: 每個 tick 更新掛鐘全域。
    //
    // ⚠ 為什麼需要：`SystemYear/Month/Date/Hour/Min/Sec` 的**初值是哨兵 9999**
    //   （cmydef.cpp:292 `Word SystemYear=9999, SystemMonth=9999, SystemDate;`），
    //   而這個行程原本**沒有任何地方**呼叫 `GetTimeInfo()` 去更新它們。
    //   20260919 端到端實跑抓到後果：`lot.start` 把
    //       LotStartTime=9999-9999-00 9999:9999:9999
    //   寫進了真實的 `D:\HT9045\config\config.ini`（已用備份還原）。
    //   全樹有 100+ 處用這六個全域組時間字串或路徑（BarCode 的日期資料夾、
    //   Jam 報表、OEE…），所以這不是 lot.start 一個地方的問題。
    //
    // 位置為什麼在這裡：golden 在兩個地方呼叫 `GetTimeInfo()` ——
    //   `TfMain` ctor（main.cpp:1683，開機一次）與
    //   `TfMain::ProcessTimeUpdate()`（main.cpp:7812，週期性）。
    //   這個行程沒有那兩個，`PumpTick()` 是它唯一的週期性入口。
    //   ⇒ 這是**整合接縫**（wb_serve 走 golden 週期性工作的哪一段），
    //     不是翻譯偏離 —— `GetTimeInfo()` 本身是 cpublic.cpp:450 的逐字翻譯。
    //
    // ⓘ 成本：一次 `Now()` 加兩次 Decode，每 500 ms 一次。
    // GetTimeInfo();  已移到本函式第一行（AI(W906-CLOCK) 20260924），不在守衛之後重複呼叫   // golden main.cpp:7812（ProcessTimeUpdate）
    { extern void W906_FlushFlagTick(); W906_FlushFlagTick(); }  { extern void W906_SafeDoorLockTick(); W906_SafeDoorLockTick(); }  { extern void W906_Timer2FanTick(); W906_Timer2FanTick(); }  /* AI(W906-DOORLOCK) 20261001: golden Timer1Timer :3051-3064 safe-door lock + Timer2 :20964-20968 big fan (bodies MainTimerSegments.cpp) */     //AI(W906-P1) 20260923: golden Timer1Timer 的 FlushFlag 翻轉段（本體在檔尾；佔用原本的空行，不移動行號）
    // The compiled MainProc path (csystem.cpp:3024-3026) has NO try/catch of its
    // own -- its #ifdef DEBUG_TRY_CATCH pair is dead because DEBUG_TRY_CATCH is
    // defined nowhere in the build. One escaping exception would otherwise take the
    // whole publisher down and, without SetErrorMode, do it behind a modal box.
    try {
        { extern void W906_HeaterSimTick(); extern bool W906_FastClockOwnsHeater(); if (!W906_FastClockOwnsHeater()) W906_HeaterSimTick(); }  MainProc();   // AI(W906-S12) 20260929 (St02-E, claim): golden THeaterThread::HeaterThreadProcess (906 uHeaterThread.cpp:56-66), SIM build only (body HeaterSimTick.cpp; RULINGS_20260929 #10 = A)   AI(W906-FASTCLK) 20261003: skipped while the serve loop's 20 ms fast clock owns the heater (MachineType.h W906_FASTCLK_HEATER; FastClockWbServe.cpp sets the flag, so every ctest that pumps keeps this call); same line
    } catch (...) {
        ++g_pumpExceptions;
    }
    { extern void W906_CounterRefreshTick(); W906_CounterRefreshTick(); }   // AI(W906-PROD-S111) 20260926（Steven 團隊）：golden TfMain::Timer10Timer 的 bRefreshCount 收尾（main.cpp:35332-35351；本體在檔尾，自己擋例外；佔用原本的空行，不移動行號）
    ++g_pumpTicks;
    g_lastTickKind = classifyTick(g_csystemTrace);
}

bool PumpActive()
{
    return g_pumpActive;
}

PumpStats PumpTelemetry()
{
    PumpStats st;
    st.ticks         = g_pumpTicks;
    st.exceptions    = g_pumpExceptions;
    st.mainProcCalls = g_pumpActive ? GetMainProcCallCount() : 0u;
    st.alive         = g_pumpActive ? IsMainProcAlive(60) : false;
    return st;
}

// ---------------------------------------------------------------------------
// AI(W906-FW-W3) 20260819: see WebBridgeTags.h SetWebControlOwner.
static unsigned long long g_webControlOwner = 0;
void SetWebControlOwner(unsigned long long connId) { g_webControlOwner = connId; }

// AI(W906-FW-BIN1) 20260820: see WebBridgeTags.h SetWebBinSelLoaded.
static bool g_webBinSelLoaded = false;
void SetWebBinSelLoaded(bool loaded) { g_webBinSelLoaded = loaded; }

// AI(W906-FW-TEMP2) 20260820: see WebBridgeTags.h SetWebTempLoaded.
static bool g_webTempLoaded = false;
void SetWebTempLoaded(bool loaded) { g_webTempLoaded = loaded; }

std::size_t PublishHandlerTags(webbridge::TagSnapshot& snap)
{
    const bool strs  = IniConfigStringsLoaded();
    const bool cust  = CustomerCodeLoaded();
    const bool lastS = LastSetLoaded();
    if (g_inPublish) return 0;   struct InPublishGuard_ { InPublishGuard_() { g_inPublish = true; } ~InPublishGuard_() { g_inPublish = false; } } inPublishGuard_;   // AI(W906-LAT-1) 20260925: a publish reached from INSIDE a publish (an output served at a yield point that ended up publishing) would beginPublish() -- clear the staging map this call is half-way through filling -- and commit, and this call would then commit the remainder as a generation missing every tag staged before the yield. No path does that today (the three callers are wb_serve's loop and its two start-up publishes); refused rather than trusted, and the outer call's follow-up publish (g_outputsServed) covers it. On the old blank line, so no line below moves
    snap.beginPublish();
    // --- BUILD IDENTITY: compile-time facts, ALWAYS non-null ----------------
    //AI(W906-FW-1e) 20260908. A new tag CATEGORY, and the category matters more
    // than the individual tags.
    //
    // WHY THIS EXISTS. Measured on the live feed 20260908: of 328 published
    // tags, the only families that carry values are `pump` (15/15) and `clock`
    // (1/1) -- everything machine-sourced is null, because no machine is
    // attached and no config is loaded. So a person looking at the screen can
    // see almost nothing about the software they are looking AT.
    //
    // ⚠ AND THE TREE RECORDS A SAFETY-RELEVANT CASE OF EXACTLY THAT. CLAUDE.md,
    // on the SIM arm: "接 web START 按鈕之前要先確認自己在哪一條線上 -- 兩條線
    // 的差別正好是「有沒有門連鎖」，而它們的 exe 檔名一模一樣." Two builds whose
    // executables have IDENTICAL FILENAMES differ in whether the safety-door
    // interlock is compiled in, and nothing on screen said which one was
    // running. That is what build.simulte and build.safeDoorInterlock answer.
    //
    // MEASURED THIS WAVE, by object-file symbol table (this tree's authoritative
    // method -- not by reading the source):
    //     ckernel.cpp:1051   #ifndef SOFT_SIMULTE
    //     ckernel.cpp:1052   if(CheckSafeDoorIsClosed()==false)
    //     ckernel.cpp:1054-6     SystemStart=false; StopAllMotor(); return false;
    //     ckernel.cpp:1058   #endif
    // and `nm ckernel.cpp.obj` shows `U CheckSafeDoorIsClosed` in BOTH
    // build_nonoracle (32-bit) and build_x64 -- so on today's default lanes the
    // interlock IS compiled in, and the linked definition is the REAL one
    // (csystem.cpp:23068, which scans Sen[iSafeDoor[i]] and can return false;
    // the `return true` stub at csystem_predicates.cpp:383 is inside #if 0).
    // ⓘ This discharges a TODO CLAUDE.md had registered as unmeasured
    // ("要用在今天的預設線上必須重量，那件事列為待辦、尚未做").
    //
    // ⚠ WHY THESE ARE ALWAYS NON-NULL, AND WHY THAT IS NOT A LEAK.
    // test_wb_tags.cpp's invariant is that no MACHINE-SOURCED tag may carry a
    // value before LoadMachineConfig -- a machine tag with a value pre-load is
    // inventing it. A compile-time fact is the opposite: it is knowable with no
    // config, no hardware and no ini, exactly like pci1203.linked, which that
    // test already allows for the same reason. So `build.*` is staged with a
    // literal `true` predicate and the test gates the CATEGORY by count.
    //
    // ⚠ NOTHING HERE MAY EVER READ CONFIG OR HARDWARE. The whole value of this
    // family is that its answers are true before anything is loaded. A tag
    // added here that keys on a config flag would silently become null and
    // would break the category's promise while looking fine.
    {
#ifdef SOFT_SIMULTE
        const bool simulte = true;
#else
        const bool simulte = false;
#endif
        stageBool(snap, "build.simulte", true, simulte);

        // The consequence an operator actually needs, derived from the SAME
        // macro at the SAME compile, rather than restated in a document that
        // can drift away from the binary.
        stageBool(snap, "build.safeDoorInterlock", true, !simulte);

#ifdef INSTALL_1203_MONITOR
        stageBool(snap, "build.install1203Monitor", true, true);
#else
        stageBool(snap, "build.install1203Monitor", true, false);
#endif
#ifdef WB_PUMP_WITH_CONFIG
        stageBool(snap, "build.pumpWithConfig", true, true);
#else
        stageBool(snap, "build.pumpWithConfig", true, false);
#endif

        // Bitness. `sizeof(void*)` rather than a macro: it cannot disagree with
        // the binary it is compiled into.
        stageInt(snap, "build.bits", true, (long long)(sizeof(void*) * 8));

        // __DATE__/__TIME__ -- WHICH build is this?
        // ⚠ This is the tag that answers the failure mode this repo keeps
        // paying for: a stale executable that runs and looks correct. Earlier
        // today an F5 lane "worked" only because its binary predated the source
        // edit by six hours, and CLAUDE.md records a killed link leaving 27
        // truncated exes that still printed ALL PASS. A build stamp on the wire
        // makes "am I looking at what I just built" answerable from the screen.
        stageStr(snap, "build.stamp", true,
                 AnsiString(__DATE__) + " " + AnsiString(__TIME__));

        // Language level and toolchain, so a reader can tell the oracle lane
        // (MinGW.org 6.3.0, C++17) from the non-oracle lanes (WinLibs 16.2.0,
        // C++14) -- a distinction CLAUDE.md makes load-bearing, because only
        // the oracle lane's numbers may be compared with BCB6.
        stageInt(snap, "build.cxx", true, (long long)__cplusplus);
#ifdef __VERSION__
        stageStr(snap, "build.compiler", true, AnsiString(__VERSION__));
#else
        stageStr(snap, "build.compiler", true, AnsiString("unknown"));
#endif

        // Machine model identity from MachineType.h -- compile-time, so it is
        // the ONE identity fact available before the ini is read (contrast
        // machine.id.type below, which needs config and is null until then).
        stageStr(snap, "build.alias", true, AnsiString(ALIAS));
        stageInt(snap, "build.atcHeadCount", true, (long long)ATC_HEAD_COUNT);
    }


    // --- machine identity: the part that IS loaded today --------------------
    stageStr(snap, "machine.id.type",   strs, IniConfig.sMachineType);
    stageStr(snap, "machine.id.gpib",   strs, IniConfig.sGPIBMachineID);
    stageStr(snap, "machine.id.tester", strs, IniConfig.RMSTesterID);
    stageInt(snap, "machine.customerCode", cust, CUSTOMER_CODE);
    stageInt(snap, "machine.typeChoice", cust, (long long)MachineTypeChoice);  stageStr(snap, "machine.typeName", cust, AnsiString(W906_MachineTypeName(MachineTypeChoice)));  stageStr(snap, "machine.gpibModel", cust, W906_GpibModel);  stageStr(snap, "machine.caption", cust, W906_MainCaption());  /* AI(W906-CAPTION) 20260930: golden fMain->Caption（本檔檔尾），主視窗標題列用；同一行插入，不移動其後行號 */  // AI(W906-HT9050-ID) 20260924: Steven c56af0b 定的機種身分三個 tag（JSON/Machine-type-index.json 的 tags 節，規格 .claude/skills/ht9050-hw/references/machine-type-entry.md）；liveness 照規格用 cust。兩個 W906_* 宣告在 database.h 檔尾、本體在 database.cpp 檔尾；放在原本的空白行上，不移動本檔其後的行號
    // AI(W906-FW-W2) 20260819: the browser's login state mirror (design doc
    // section 2 -- replaces golden's ChangeLevelAttr widget-enable pass).
    // AccessLevel is a plain int global (cmydef.h:3503). Liveness keys on
    // the config-loaded signal (cust) to PRESERVE test_wb_tags' load-bearing
    // invariant "every tag is null before the data layer is loaded": a
    // pre-config snapshot publishes auth.level as null; post-config it is
    // live, and 0 there is the real Operator state.
    stageInt(snap, "auth.level", cust, AccessLevel);

    // AI(W906-FW1d) 20260820: user.level -- the level NAME the dashboard
    // binds (tagmap.js: fMain.cbUserSelect mirror). fMain->DoChangeLevel()
    // (golden main.cpp:15127, translated this wave) writes it on every
    // login/logout; before the first call it is the honest "" (golden always
    // runs DoChangeLevel early via ChangeLevelAttr; this process has no such
    // startup pass). Same cust key as auth.level.
    stageStr(snap, "user.level", cust, fMain->cbUserSelect->Text);

    // AI(W906-FW1e) 20260820: recipe.current -- the active setup name, golden
    // main.cpp:9440's startup mirror (cbSetupFileName->Text=GetLastOpenFN(),
    // one read of setup.inf done by the host process at boot; wb_serve does
    // it before installing the modal hook). "" = the host never mirrored;
    // "Fail Open" = golden's own text for an unreadable setup.inf, published
    // as-is. Same cust key as the other identity mirrors.
    stageStr(snap, "recipe.current", cust, fMain->cbSetupFileName->Text);

    // AI(W906-FW-W3) 20260819: control-token mirror (see SetWebControlOwner).
    // Same liveness key as auth.level; "" = nobody holds the token.
    {
        char ownerBuf[32];
        // AI(W906-FW-1Tg) 20260915: MinGW warns twice on the next line
        //   ("unknown conversion type character 'l' in format" [-Wformat=] and
        //    [-Wformat-extra-args]).  **MEASURED: the warning is wrong for this
        //   runtime -- %llu prints correctly.**  Probe compiled with this TU's
        //   own flags and RUN:
        //       snprintf(buf, n, "conn-%llu", 1234567890123ULL)
        //         -> "conn-1234567890123"      (identical to %I64u)
        //       snprintf(buf, n, "conn-%llu", 7ULL) -> "conn-7"
        //   GCC assumes the target printf is MSVCRT-without-%ll; the runtime
        //   this build actually links handles it.
        //   ⚠ DO NOT "fix" it to %I64u: that would be churn against a measured
        //   non-problem, and it is the less portable spelling of the two.
        //   ⚠ DO NOT silence it with a pragma either -- the same two warnings
        //   are how a REAL varargs defect showed up once before in this tree
        //   (DEVLOG 20260912: cinitial.cpp:16813).  Keeping the channel noisy
        //   but understood beats turning the instrument off.
        if (g_webControlOwner != 0)
            std::snprintf(ownerBuf, sizeof(ownerBuf), "conn-%llu",
                          (unsigned long long)g_webControlOwner);
        else ownerBuf[0] = '\0';
        stageStr(snap, "control.owner", cust, AnsiString(ownerBuf));
    }

    // --- LastSet-derived scalars --------------------------------------------
    // These read ONLY LastSet, so the blob's liveness settles them. Their
    // meaning is deliberately not interpreted here (no "Real"/"Dummy" label):
    // that mapping lives in golden's UI code and has not been verified, and a
    // confidently wrong label is worse than a raw code.
    stageInt(snap, "lastset.tester",     lastS, LastSet.iTester);
    stageInt(snap, "lastset.realDummy",  lastS, LastSet.iRealDummy);
    stageInt(snap, "lastset.runStartMode", lastS, LastSet.iRunStartMode);
    stageInt(snap, "lastset.temperature",  lastS, LastSet.iTemperature);

    // --- FW-1a: the start-mode word + the 2-arm x 16-site map ----------------
    //AI(W906-FW1) 20260817: first tag-wiring batch of the FW campaign
    // (docs/DFM2WEB_CAMPAIGN_PLAN.md 4-FW-1). Both families read ONLY the
    // LastSet blob, so the blob's liveness settles them like the scalars above.
    //
    // startmode.value decodes through golden's OWN display array,
    // StartModeName[rsmRunModeTotal] (cmydef.cpp:62, translated verbatim) --
    // the same lookup fMain does at golden main.cpp:560 -- rather than a
    // hand-written mapping that could drift. The raw code stays on the wire as
    // lastset.runStartMode so a reader can always cross-check the decode.
    // Out-of-range codes (rsmNull = -1, or a corrupt blob) publish null: a
    // missing word costs less than a confidently wrong one.
    {
        const int m = LastSet.iRunStartMode;
        const bool decodable = lastS && m >= 0 && m < rsmRunModeTotal;
        stageStr(snap, "startmode.value", decodable,
                 decodable ? StartModeName[m] : AnsiString());
    }

    // site.arm{a}.s{n} <- LastSet.bUseTestSocket[a-1][(n-1)/8][(n-1)%8]
    // ([2][4][8], LastSet.h:431).
    //
    // Source choice, verified against cprod.cpp:3630-3807 (ReadTestMode) and
    // deliberately NOT the audit's first suggestion TestMode.iDutOnOff: when
    // CosFunction.bLastSetInSetUpFile==false that function returns at :3642
    // BEFORE filling iDutOnOff, so the TestMode array can sit all-zero while
    // the real site map exists -- publishing it would show "every site off" as
    // a confident value. bUseTestSocket IS the persisted truth in both
    // branches: ReadTestMode seeds its ini reads from it AND writes its result
    // back into it (:3712-3713, :3797-3798).
    //
    // First-dimension semantics measured, not assumed: [0] is Arm1 (ini key
    // "Dut <name>", :3689->:3693), [1] is Arm2 ("Dut <name>2", :3773->:3777);
    // FT/RT selects which ini SECTION is read (DutOnOff vs DutOnOff_RT), never
    // the dimension. Site numbering follows tagmap.js "site.*" (XItem=8):
    // sites 1..8 = row 0, 9..16 = row 1, col = (n-1)%8. Rows 2..3 of the
    // 4-row array belong to shapes this screen does not show; not published.
    for (int arm = 0; arm < 2; ++arm) {
        for (int nSite = 1; nSite <= 16; ++nSite) {
            char tag[24];
            std::sprintf(tag, "site.arm%d.s%d", arm + 1, nSite);
            stageInt(snap, tag, lastS,
                     LastSet.bUseTestSocket[arm][(nSite - 1) / 8][(nSite - 1) % 8] ? 1 : 0);
        }
    }
    //AI(W906-SITEPANEL-SIZE) 20261005: EastSun 1005「這個紅框處應該是要照著Tools->Setup裡面所設定的Site數改變，現在的設定為Single Site，
    //  所以應該Show出單框，單框內容是1，然後亮綠色」-- the grid's shape, golden TfMain::DrawTestSitePanel (906 main.cpp:22319-22342):
    //  mtDutOnOff->XItem = TestSocket.iShtCol, ->YItem = TestSocket.iShtRow (x2 when IniConfig.bA09_ByArmCloseSite && !bUseTwoArm32Site,
    //  the two-arm panel). site.byArm says which. The page (web/page/ht9045_sitepanel_size.js) shows only those cells of its fixed 8x4.
    //  Not ported: golden's ASE_KaohSiung Octal_12Kit 8->6 columns and the NN-mode row labels (CUSTOMER_CODE / test modes this machine
    //  does not use); the site on/off values themselves are still the site.arm*.s* tags above.
    stageInt(snap, "site.cols",  lastS, TestSocket.iShtCol);
    stageInt(snap, "site.rows",  lastS, TestSocket.iShtRow);
    stageInt(snap, "site.byArm", lastS, (IniConfig.bA09_ByArmCloseSite && bUseTwoArm32Site == false) ? 1 : 0);

    // --- FW-1b: sort counters (Auto1..3 / Fix1..3 place-to-tray counts) ------
    //AI(W906-FW1b) 20260817: source LastSet.BinCT[4][256] (LastSet.h:179 =
    // golden LastSet.h:106) -- the exact cell golden's fSortCT displays:
    // cSortCT.cpp:399  pnlCount->Caption = LastSet.BinCT[0][iTo3Unload[i]].
    //
    // TWO index spaces, verified against golden main.cpp:1954-1966 and
    // MachineType.h; conflating them is the exact trap this block dodges:
    //   display gate uses e6TrayName:  eAuto1..3 = 0..2,  eFix1..3 = 6..8
    //   BinCT column uses e3TrayName:  e3Auto1..3 = 0..2, e3Fix1..3 = 3..5
    // The port's own iTo3Unload[] global is DECLARED but never initialized
    // (golden fills it in the untranslated TfMain ctor) -- it reads all-zero
    // today, so publishing through it would silently point every Fix at
    // Auto1's cell. Constants are used directly instead. (That all-zero
    // global is ALSO consumed by translated runtime code, e.g.
    // aoutarm.cpp:3332 -- recorded as a latent port defect, separate issue.)
    //
    //AI(W906-SJSON-S9b) 20260923: ⛔ **THE PARAGRAPH ABOVE IS OUT OF DATE.**
    //  iTo3Unload[] IS initialized, and has been since 20260817 -- by a
    //  same-TU static initializer in cmydef.cpp, added by AI(W906-FW-Fix1)
    //  on that very date, one day after the note above was written:
    //      cmydef.cpp:4281   struct W906_TrayIndexMapInit { ctor fills both maps }
    //      cmydef.cpp:4386+  iTo3Unload[eAuto1] = e3Auto1;  ... (33 assignments)
    //      cmydef.cpp:4431   W906_TrayIndexMapInit g_w906TrayIndexMapInit;
    //  It links (nm on ht9045_globals.dir/cmydef.cpp.obj shows the instance
    //  and _iTo3Unload together) and it is order-safe: zero-init precedes
    //  dynamic init, and both live in the same TU as the arrays.
    //  ⇒ aoutarm.cpp:3332 is NOT a latent defect. There is nothing to fix.
    //
    //  ⚠ THIS BLOCK'S OWN CHOICE IS UNCHANGED AND STILL CORRECT: it keeps
    //    using the e3* constants directly. The reason was never "the global
    //    is empty" -- it is the TWO INDEX SPACES trap named at the top of
    //    this comment (e6TrayName vs e3TrayName). Going through iTo3Unload[]
    //    would work today but would re-introduce the ambiguity this block
    //    exists to dodge.
    //
    //  ⚠⚠ Corrected rather than deleted, per this file's own convention --
    //    and because the stale sentence had already been copied once into
    //    the skill's porting-gaps.md as a phantom to-do ("iTo3Unload[] is
    //    all zero, QtyLog writes BinCT[0][0] six times"), which cost a review
    //    round to retract. A stale claim that is merely deleted gets copied
    //    again from someone's memory; one that is contradicted in place does
    //    not. 20260923 measured.
    //
    // Per-tag liveness mirrors golden's own display gate
    // (Prod.iTrayType[i]!=tNotUse, cSortCT.cpp:396): an unconfigured station
    // publishes null, exactly like golden draws nothing for it. With Prod
    // unloaded (all zero == tNotUse) every station is null -- honest.
    {
        static const struct { const char* tag; int typeIdx; int binIdx; } kSort[6] = {
            {"sort.auto1.count", eAuto1, e3Auto1},
            {"sort.auto2.count", eAuto2, e3Auto2},
            {"sort.auto3.count", eAuto3, e3Auto3},
            {"sort.fix1.count",  eFix1,  e3Fix1 },
            {"sort.fix2.count",  eFix2,  e3Fix2 },
            {"sort.fix3.count",  eFix3,  e3Fix3 },
        };
        for (int i = 0; i < 6; ++i) {
            const bool binLive = lastS && Prod.iTrayType[kSort[i].typeIdx] != tNotUse;
            stageInt(snap, kSort[i].tag, binLive, LastSet.BinCT[0][kSort[i].binIdx]);
        }
        { extern std::size_t W906_StageSortCTTags(webbridge::TagSnapshot&, bool, bool); W906_StageSortCTTags(snap, lastS, cust); }   //Steven 20260925 (Data.SortCT)：其餘 sort.*（33 站的良率／可見度、其他 27 站數量、總良率、ART 分頁、IC Count 分頁）—— golden TfSortCT 顯示的值；本體與 liveness 理由在檔尾。佔用原本的空行，不移動本檔其後的行號
        // --- FW-1c: sort.loading + sort.total --------------------------------
        //AI(W906-FW1c) 20260819: sort.loading = LastSet.SendCT[0], the exact
        // cell golden's fSortCT shows as the Loader count (cSortCT.cpp:214
        // pnlLoader->Caption=LastSet.SendCT[0]) -- same blob, same lastS key as
        // the station counters above.
        stageInt(snap, "sort.loading", lastS, LastSet.SendCT[0]);

        //AI(W906-FW1c) 20260819: sort.total. Golden's Sum (cSortCT.cpp:355-380)
        // walks LastSet.BinCT[0][iTo3Unload[i]] -- the SAME uninitialized
        // iTo3Unload[] mine the block comment above records (all-zero in the
        // port, every station would collapse onto Auto1's cell). This tag
        // therefore reuses FW-1b's already-ruled shape instead of the golden
        // loop: sum the six e3-constant cells directly, and only over stations
        // golden itself would count (iTrayType configured). Live whenever the
        // blob is, like sort.loading; an all-unconfigured machine publishes an
        // honest 0, mirroring golden's Sum staying 0 when nothing accumulates.
        {
            long long total = 0;
            { extern long long W906_SortCTSum(bool); total = W906_SortCTSum(false); }   //Steven 20260925 (Data.SortCT)：改成 golden ShowSortIC 的 Sum（V912 cSortCT.cpp:352-357）—— eTrayCount（33 站）中 Prod.iTrayType!=tNotUse 的站；本體在檔尾
            //   ⛔ 上面那段 AI(W906-FW1c) 的理由已過期（更正不刪）：iTo3Unload[] 早在 20260817 就由 cmydef.cpp:4281/:4431 填好（本檔 :906-931 已記），
            //   所以這裡改走 golden 的 iTo3Unload 迴圈。原本的六站版（Auto1-3＋Fix1-3）少算 Auto4-6／Fix4-12／Bulk Box／Mag1-14 ——
            //   HT-9011UC（AUTO_EMPTY_COLOR>=3）與 Magazine 機型的總數會跟 golden 畫面不同。佔用原本 for 迴圈的四行，不移動其後行號
            stageInt(snap, "sort.total", lastS, total);
        }
    }

    // --- FW-BIN1: recipe bin assignments -------------------------------------
    //AI(W906-FW-BIN1) 20260820: MyBinSel[].Caption after the host ran the
    // BinSelect->SetTechDataToProd_Yield->ShowBinSel chain (wb_serve boot,
    // DataPath dry-redirected -- docs/RECON_binstar_datasource.md section 4).
    // Same e6TrayName station indices as the sort counters above. Null until
    // the host marks the chain loaded; the captions are golden's own display
    // strings (bin lists, "LINK ..." forms, "X" for unconfigured).
    {
        static const struct { const char* tag; int idx; } kBin[6] = {
            {"bin.auto1", eAuto1}, {"bin.auto2", eAuto2}, {"bin.auto3", eAuto3},
            {"bin.fix1",  eFix1 }, {"bin.fix2",  eFix2 }, {"bin.fix3",  eFix3 },
        };
        for (int i = 0; i < 6; ++i) {
            stageStr(snap, kBin[i].tag, g_webBinSelLoaded,
                     fShowBinSelect->MyBinSel[kBin[i].idx]->Caption);
        }
    }

    // --- FW-TEMP2: recipe temperature setpoint/soak/mode ---------------------
    //AI(W906-FW-TEMP2) 20260820: after fTemp_Set->ReadTempFile(true) (wb_serve
    // boot, SAME DataPath dry-redirect as FW-BIN1 above -- WebBridgeTags.h's
    // AI(W906-FW-TEMP2) block has the full ruling and evidence). Null until
    // the host marks the chain loaded (g_webTempLoaded, keyed off golden's own
    // iSendChangeTempError "file missing" signal, not merely "we tried").
    //
    // temp.mode publishes the RAW ini code (0=Hot/1=Ambient/2=ATC/3=AmbientHot,
    // ReadTempFile uTemp_Set.cpp:2248-2264) rather than a decoded label: the
    // "Hot Mode"/"Ambient Mode" text tagmap.js's lblTemperatureMode expects is
    // set on fMain, a form this file does not touch, and this file does not
    // guess captions it has not read.
    //AI(W906-FW-TEMP3) 20260824: user ruled the decode lives browser-side --
    // web/js/ui/bind.js `tempMode` formatter maps the code to the caption.
    // This file keeps publishing the raw int; wire contract unchanged.
    stageDouble(snap, "temp.sv",   g_webTempLoaded, Temperature.fWorkTemperBase);
    stageDouble(snap, "temp.soak", g_webTempLoaded, Temperature.fSoakTime);
    stageInt   (snap, "temp.mode", g_webTempLoaded, Temperature.iMachineTempMode);

    // --- FW-1c: SPIL_AMR bundle tray IDs -------------------------------------
    //AI(W906-FW1c) 20260819: asBundleTrayID[] (cmydef.h:5684) is written only
    // on fAGV->IsSPIL_AMR() customer stations (fLotInfo.cpp Wave C, golden
    // uLotInfo.cpp FormShow S28-adjacent). On every other machine it stays the
    // AnsiString default "" -- an honest empty, not a defect (RECON_FW1 §6#6).
    // cust is the liveness proxy: the value only means anything once the
    // customer identity is loaded.
    stageStr(snap, "lot.loaderLastBundleId", cust, asBundleTrayID[ePortLoader]);
    stageStr(snap, "lot.loadercarBundleId",  cust, asBundleTrayID[ePortEmpty]);
    { extern std::size_t W906_StageLotInfoTags(webbridge::TagSnapshot&, bool); W906_StageLotInfoTags(snap, cust); }   //Steven 20260925 (Data.LotInfo)：lot.id／operator／runMode／startTime／device —— golden uLotInfo「Lot」分頁（tsLotID）顯示的 fLotInfo 表單文字；本體與 liveness 理由在檔尾。佔用原本的空行，不移動本檔其後的行號
    // --- FW-1T-b: light.off ---------------------------------------------------
    //AI(W906-FW-1Tb) 20260915.  First tag moved OUT of kUnloadedTags by the
    // "a widget displays something; find the state it displays" rule written in
    // that array's own banner.
    //
    // WHAT GOLDEN DOES (main.cpp:26056-26095, TfMain::LightOn / spbLightClick):
    //   LightOn():  spbLight->Tag=1; Caption="Light ON";  SW[SwCCDLight].On();
    //   off paths:  SW[SwCCDLight].Off(); Tag=0; Caption="Light OFF";
    // The button is the DISPLAY; SW[SwCCDLight] is the STATE.  Tag/Caption are
    // set alongside the switch every time, so they are a mirror, never a source.
    //
    // WHY OutValue AND NOT Status():
    //   TMySwitch::On()/Off() (myswitch.cpp:74 / :124) assign OutValue as their
    //   FIRST statement, ABOVE the `if(Enable==false) return;` that guards the
    //   hardware write.  OutValue therefore records what software last COMMANDED
    //   and is meaningful with the IO layer offline.  Status() (:176) polls the
    //   card and returns false when !Enable -- that is a readback, not a state,
    //   and using it here would publish "off" for "cannot ask".
    //
    // LIVENESS = Enable, not "did anyone command it":
    //   Enable comes from the IO table (InitialSwitch(), cinitial.cpp:1506).
    //   !Enable means this machine has no such switch configured -- i.e. we do
    //   not know anything about a CCD light, which is exactly what null means in
    //   this file.  A machine that HAS the switch but has never toggled it is a
    //   different case: OutValue==false is then the true answer (it is off).
    //
    // POLARITY (the thing most likely to be wrong, so here is the evidence):
    //   `light.off` is a bool in the browser's state model
    //   (docs/web-fw-legacy/js/model/state.js:93) rendered with data-bind="on"
    //   (js/ui/widgets.js:35-43), the same binding used by the mutually exclusive
    //   run.ft / run.rt / run.offline trio -- i.e. it marks the CURRENT state,
    //   not an action.  Its button issues `light.toggle`, a toggle, whereas the
    //   action-style buttons next to it issue explicit setters (run.setFT,
    //   runmode.setNormal).  So: light.off == true  <=>  the light is OFF
    //   <=>  SW[SwCCDLight].OutValue == false.
    stageBool(snap, "light.off",
              SW[SwCCDLight].Enable,
              SW[SwCCDLight].OutValue == false);
    { extern std::size_t W906_StageTestCategoryTags(webbridge::TagSnapshot&, bool); W906_StageTestCategoryTags(snap, cust); }   //Steven 20260925 (Data.TestCategory)：tcat.* —— golden TfTestCategory 兩張格子（sgArm1DrawCell 畫出的文字／底色／粗體＋AdjFormData 的列欄與寬高）；本體與 liveness 理由在檔尾。佔用原本的空行，不移動本檔其後的行號
    // --- everything whose source is measurably dead --------------------------
    for (std::size_t i = 0; i < kUnloadedCount; ++i) {
        stageNull(snap, kUnloadedTags[i]);
    }

    // --- machine state + pump telemetry -------------------------------------
    //AI(W906-SimPump) 20260813: WITHOUT a pump these are NULL, not "HALT". We are
    // not driving the spine, so we have no state to report, and "HALT" would be a
    // claim about a machine we are not observing. With a pump the word reports the
    // LIVE master guard, re-read here every tick, prefixed "SIM " because this
    // process forced the guard terms rather than sensing them (WebBridgeTags.h).
    const bool pumping = g_pumpActive;

    // --- the wall clock ------------------------------------------------------
    //AI(W906-SimPump) 20260813: gated on `pumping` like every other process tag,
    // and that gate is deliberate rather than incidental.
    //
    // My first version published it unconditionally, on the reasoning that the
    // publisher always knows its own clock. That broke a REAL invariant asserted at
    // tests/test_wb_tags.cpp:68 -- "every tag is null before the data layer is
    // loaded (nothing is inventing values)" -- whose stated purpose (:53-55) is to
    // be the control for the liveness checks after it. Relaxing that assertion to
    // accommodate one convenience tag would have traded a strong, simple invariant
    // for a weaker one coupled to a tag-naming convention. Gating instead keeps the
    // invariant untouched and costs nothing that matters: the F5 compound runs
    // --pump, and a publisher that is not driving the spine has a static screen by
    // definition, so a ticking clock on it would be the misleading part.
    //
    // Format: golden TfMain::ProcessTimeUpdate sAlarmTime (V912 main.cpp:8239 -> StatusBar1 Panels[6]); was "%04d / %02d / %02d   %02d:%02d" (web-fw-legacy state.js:63) until AI(W906-PROD-S112) 20260926.
    if (pumping) {
        const std::time_t now = std::time(0);
        const std::tm*    lt  = std::localtime(&now);
        if (lt != 0) {
            char buf[64];
            std::sprintf(buf, "%04d-%02d-%02d %02d:%02d:%02d",   // AI(W906-PROD-S112) 20260926（Steven 團隊，Jimmy 21:3x 同意由 St01 改）：golden ProcessTimeUpdate（V912 main.cpp:8231-8242）的 sAlarmTime 格式，有秒 ⇒ 每秒變一次；同三行改寫，不移動行號
                         lt->tm_year + 1900, lt->tm_mon + 1, lt->tm_mday,
                         lt->tm_hour, lt->tm_min, lt->tm_sec);
            snap.stage("clock.text", TagValue::makeString(buf));
        } else {
            stageNull(snap, "clock.text");
        }
    } else {
        stageNull(snap, "clock.text");
    }

    if (pumping) {
        //AI(W906-IdlePump) 20260817: three words, not two, and the third is the
        // normal one now. HALT and IDLE are NOT the same machine state and an
        // operator-facing word must not conflate them: HALT means something stopped
        // the machine (SoftStop), IDLE means it was opened and nobody has started it
        // yet. Since PumpInit stopped forcing the guard, IDLE is what a freshly
        // launched publisher reports -- which is the point of the change.
        //
        // Safe to add a third string: web/js/panels/left.js:23 binds machine.state
        // as plain text into div.halt__state with no tone tag and no switch on the
        // value, so the page renders whatever it is told (checked 20260817).
        const bool run  = guardAllowsEngines();
        const char* word = run      ? "SIM RUN"
                         : SoftStop ? "SIM HALT"
                                    : "SIM IDLE";
        // === AI(W906-MSTATE-P1) 20260923: machine.state 改發**真值** ==========
        //
        //  ## 它現在是什麼
        //  `fMain->palMainStatus->Caption`（forms/fMain.h:287，TfMainPanel =
        //  vclcompat::TPanel）。那就是操作員畫面上那塊 48pt 大字，golden 的
        //  巢狀是 `Panel2 > Off_lineDisplay > palMainStatus`；`Off_lineDisplay`
        //  只管邊框與底色，**沒有任何 caption 寫入者**（golden 6 個引用全是
        //  BorderWidth/Color）。命名依據：docs/DESIGN_RouteB_V899SnapshotPort.md §153。
        //
        //  ## 為什麼把 SIM RUN/HALT/IDLE 拿掉
        //  那三個字描述的是**這個發布行程的守衛**，不是機台狀態。把它送到一個
        //  標著「機台狀態」的欄位，操作員會把 SIM IDLE 讀成機台的狀態。
        //  守衛本身沒有遺失 —— `pump.guard.softStop/systemStart/allMotorHome`
        //  （本檔下方）照舊發送，而且那才是它們正確的名字。
        //  ⓘ 安全性：改語意前查過現行消費者。`machine.state` 在 D:\HT9045\web\
        //    全樹（os.walk，該目錄不在版控所以 git grep 看不到）只有 4 個命中：
        //    ht9045_recipe_client.js:231/:493 是**註解**、screenshot_meta.js:1601
        //    自己標 "status":"nohtml"、tests/tagstream.test.js 拿它當泛用 tag 名。
        //    ⇒ **零個渲染消費者**，改語意不會弄壞任何畫面。
        //    ⚠ 本檔原註解說「web/js/panels/left.js:23 binds machine.state」——
        //      **那個檔不存在**（20260923 實測），是上一代 HMI 的過期引用。
        //
        //  ## 現在還沒有值，而這正是重點
        //  寫 caption 的是 golden `TfMain::ShowNowStatus`（main.cpp:22190-22318，
        //  129 行），餵它的是 `ShowRunLabel`（golden ckernel.cpp:935-1726，792 行，
        //  內含 88 個 ShowNowStatus 呼叫）。**兩個都還沒翻**，所以 caption 是空的。
        //  空的時候這裡發 **null**，網頁顯示 "---"（不可知），不是編一個字。
        //  ⇒ 這一步的價值就在這裡：網頁上那個寫死的 "PAUSE" 是在說謊
        //    （機台在 IDLE 它也寫 PAUSE）。換成誠實的 "---" 才能開始相信這個欄位。
        const AnsiString cap = (fMain != 0 && fMain->palMainStatus != 0)
                             ? fMain->palMainStatus->Caption
                             : AnsiString("");
        const bool haveState = (cap.Length() > 0);
        stageStr(snap, "machine.state", haveState, cap);  stageBool(snap, "n07.alarm", IniConfig.bEnable_SECS_GEM && IniConfig.bN07_Alarm, bN07AlarmActive);   //AI(W906-C15-N07-UI) 20261003 (St02-E, claim): the N07 SECS/GEM disconnect banner's tag -- golden 906_0625_Steven main.cpp:20974-20982 paints Off_lineDisplay red / labTesterMode "SECS DISCONNECTED" while bN07AlarmActive (MR !137, SECSGEM/N07Alarm_St02.cpp); null when [N07] is off; drawn by web/page/ht9045_n07_banner.js; same line

        //AI(W906-MSTATE-P3) 20260923: machine.stateCode —— 使用者裁決「一起接」。
        //  值是 `iSECSGEMMachineState`（cmydef.cpp:5444），1-based 索引進
        //  `sMacStatus[30]`，同一顆變數已經以 SVID 1020 發布給 SECS host
        //  （SECSGEM/uHGemHT9045_SV.cpp:467）。數字比字串不怕大小寫與空白。
        //
        //  ⚠⚠ **它現在沒有任何寫入點。** golden 唯一寫它的地方在 ShowNowStatus
        //    內（main.cpp:22283 的 `if(Capstr==sMacStatus[i]) iSECSGEMMachineState=i+1;`），
        //    而那支沒翻 ⇒ 這顆變數恆為 0。
        //    所以 liveness 判準**不能**只看 pumping：那會發一個永遠是 0 的數字，
        //    而 0 在 1-based 索引裡代表「不在表上」，網頁卻會把它當成一個真的代碼。
        //    這正是「符號存在有三級（能編／實例在／值被維護）」的第三級陷阱。
        //  ⇒ 判準綁在 haveState 與 >0 兩者：沒有 caption 就沒有 code，
        //    ShowNowStatus 一翻好，兩個會在同一拍一起變真。
        stageInt(snap, "machine.stateCode",
                 haveState && iSECSGEMMachineState > 0, iSECSGEMMachineState);
        //AI(W906-START-WIRE) 20260919: 更正這句話。原本的 not-run 分支寫
        // 「(SoftStart never raised)」—— 那在 ScanSystemSensor() 還沒接進
        // MainProc() 之前是**正確**的：當時 SoftStart 沒有任何讀者
        // （§C10）。20260919 把那一跳接上之後（csystem.cpp 的活 MainProc），
        // SoftStart 會被設、會被讀、而且會在**同一次呼叫裡**被 ckernel.cpp:837
        // 清回 false，所以「never raised」已經變成假的敘述，而且它會把讀的人
        // 指向錯的地方 —— 實測 20260919 的第一個真正阻擋不是 SoftStart，是
        // StartFromWeb 內部 WebStart.cpp 的 CompareTechData() 否決。
        //AI(W906-CITE-REMEASURE) 20260921: 原文引用的 `WebStart.cpp:1125` 已經漂了
        //  （20260921 重量 = :1229）。⚠ 不要再往這裡寫死行號 —— 那個檔一加註解
        //  就位移，而漂掉的引用會把讀的人送到別的分支去。
        //  要當場定位就跑 `python tools/start_reject_map.py`：它會把
        //  StartFromWeb 裡 102 個 return false（活 71 / 閘 31）連同訊息字串與
        //  golden 行號一起列出來，並標出哪些在本函式內找不到訊息。
        // 現在不猜是哪一項不成立，直接把人指回三個原始輸入。
        //AI(W906-MSTATE-P1) 20260923: stateSource 現在回答「這個值從哪來／為什麼沒有」。
        //  沒有值的時候**要說出原因**，而不是留白 —— 留白與「機台沒在跑」
        //  從瀏覽器看是同一件事。守衛那句話保留（`word`），因為它仍然是
        //  「為什麼引擎沒在動」的答案，只是它不再冒充機台狀態。
        if (haveState) {
            snap.stage("machine.stateSource", TagValue::makeString(
                "palMainStatus->Caption -- golden TfMain::ShowNowStatus"));
        } else {
            const AnsiString why =
                AnsiString("no writer yet: TfMain::ShowNowStatus (golden "
                           "main.cpp:22190-22318) and ckernel ShowRunLabel "
                           "(golden ckernel.cpp:935-1726) are not translated. "
                           "pump guard says ")
                + AnsiString(word)
                + AnsiString("; raw guard terms are in pump.guard.*");
            snap.stage("machine.stateSource", TagValue::makeString(why.c_str()));
        }
        (void)run;
    } else {
        stageNull(snap, "machine.state");
        stageNull(snap, "machine.stateSource");
        stageNull(snap, "machine.stateCode");            // AI(W906-MSTATE-P3) 20260923
    }

    // Raw guard terms, published individually so the derived word above can always
    // be checked against its inputs rather than trusted.
    stageBool(snap, "pump.guard.softStop",     pumping, SoftStop);
    stageBool(snap, "pump.guard.systemStart",  pumping, SystemStart);
    stageBool(snap, "pump.guard.allMotorHome", pumping, fAllMotorHome);  stageBool(snap, "tower.red", pumping, fMain->ledRed->Value); stageBool(snap, "tower.amber", pumping, fMain->ledYellow->Value); stageBool(snap, "tower.green", pumping, fMain->ledGreen->Value);  // AI(W906-TOWER) 20260924: 塔燈 = golden 的 fMain->ledRed/Yellow/Green->Value（ckernel.cpp:1748+ 已翻，SW[SwTower*] 是鏈的末端不是源頭）；pump 不在跑時 null（沒有 kernel tick 就沒有 RunState，值沒有意義）。W1 普查：頁面在讀、原本恆 null；放在同一行，不移動行號

    // -----------------------------------------------------------------------
    //AI(W906-GUARD-TAGS) 20260919: `guard.*` —— Steven 的 SYSTEMSTART_SIGNAL_
    // CONTRACT.md（交付於 U:\共用區\HT-9050\HT9045_V906_changes_20260919_fShow）
    // 要的四個訊號。web HMI 用它決定 HW.IoSetView / HW.MotorTest / HW.teach /
    // Setup.Contact 這四頁能不能開 —— 那是使用者 20260918 的規則 A，golden 的
    // 寫法是每個開啟處理常式第一行 `if(SystemStart) return;`（golden
    // main.cpp:28801 sbTeachingClick）。
    //
    // 為什麼另開前綴而不是叫瀏覽器讀 pump.guard.*：`pump.*` 在這個檔的既有契約
    // 裡是「publisher 自己的遙測」，而且刻意不算進 TagCoverage。這四個是**機台
    // 狀態**，語意不同。前三個與上面同值，是刻意的別名 —— 同一個全域、同一個
    // liveness，只是換一個語意正確的名字給瀏覽器用。
    //
    // ⚠ liveness 用 `pumping`，不是 `true`。這一格選錯是這份契約唯一會傷到人的
    // 地方：送 `false` 會讓瀏覽器放行它本該擋住的操作，而那個 false 的真正意思
    // 是「這個行程從來沒跑過 kernel tick」。null（不可知）在瀏覽器端被當成
    // `true` 處理（保守），這是 Steven 契約 §3 明訂的。
    //
    // ⚠ 要回報給 Steven 的一條過期敘述：他契約 §1 寫「`PumpInit()` 只在
    // `tools/wb_publish.cpp` 呼叫，所以在 `wb_serve` 之下恆為 null」。
    // 20260918 起不成立 —— `tools/wb_serve.cpp:2136` 已經呼叫 PumpInit()
    // （AI(W906-ST-S3-B2)），實測 20260919 在 wb_serve 下 pump 是 ARMED 的，
    // 而 wb_publish 本身 20260919 已退役。
    //
    // ⚠ TagCoverage 刻意**不**計這四個。理由與上面 pump.* 那段、以及 FW1c 的
    // lot.auto*.trayCount 完全相同：它們 gate 在 `pumping` 上 —— 那是這個行程
    // 自己發動的一次執行，不是它從機台讀到的一個資料來源。計進去會讓 live 變
    // 好看而沒有任何新來源被讀取。
    stageBool(snap, "guard.softStop",     pumping, SoftStop);
    stageBool(snap, "guard.systemStart",  pumping, SystemStart);
    stageBool(snap, "guard.allMotorHome", pumping, fAllMotorHome);
    //AI(W906-GUARD-TAGS) 20260919: guard.contactMode —— 互斥判斷用，
    // 0 == CONTACT_NORMAL，非 0 就要互斥（Steven 契約 §5.2，對照 golden
    // Command.cpp:7350 `fContact->rbModeNormal->Checked==false`）。
    //
    // ⓘ 量到的事實，寫下來免得日後被當成 bug：`iContactMode` 在這棵樹**只有一個
    //   寫入點**，csystem.cpp:1583 `iContactMode=CONTACT_NORMAL;`，而那一行落在
    //   csystem.cpp:598 起的 `#if 0` golden 逐字副本裡（量法：git grep 出所有
    //   `iContactMode =` 的賦值，只有 cmydef.cpp:3338 的定義、csystem.cpp:1583、
    //   以及兩支測試）。⇒ 在這支 binary 裡沒有任何路徑能把它變成非 0。
    //   所以今天送出去的一定是 0（NORMAL，放行），而那是**真的**，不是
    //   「沒人跑過所以還是初值」—— 機台不可能處於診斷用的 contact 模式，因為
    //   能設定它的碼沒有被編進來。
    //   這條敘述會在 Contact 那一波落地時自動失效（值會變成真的活的），
    //   不需要回來改這裡 —— 我們發的就是那個全域本身。
    stageInt (snap, "guard.contactMode",  pumping, static_cast<long long>(iContactMode));  if (W906_UiPagesJsonHook) stageStr(snap, "ui.pages", true, AnsiString(W906_UiPagesJsonHook()));   //AI(W906-PAGETAB-Q51) 20260928 [W906] 頁面表整張送回網頁（background.html 照 want 開／關視窗、核對列數；WebPageTable.cpp PageTableJson）。hook 只有 wb_serve 裝（tools/wb_serve.cpp:4389），wb_publish／test_wb_tags／test_wb_simpump 是 0 ＝ 不送；接在同一行，不移動行號

    // -----------------------------------------------------------------------
    //AI(W906-P1-HASIC) 20260919: `guard.machine.*` —— Steven `CPP_REQUESTS_
    // 20260918.md` §0.1（契約 SYSTEMSTART_SIGNAL_CONTRACT.md §8）要的
    // **機台淨空四述詞 ＋ 一個彙總**。
    //
    // 為什麼需要：golden `main.cpp:33658` `sbShuttleMaintainClick` 的守衛是
    // **兩條**，不是一條 ——
    //     if(SystemStart) return;                              <- guard.systemStart 已交付
    //     if(InArmSuck.HasIC()==false && OutArmSuck.HasIC()==false &&
    //        ShuttleHasIC()==false && IndexHasIC()==false)     <- 本段
    //         進維護模式
    //     else ShowMyMessage("Please Clean Out or One Cycle");
    // 只交付第一條，瀏覽器只擋得住一半。
    //
    // ⇒ 這是**發佈**請求不是翻譯請求：四個符號在這棵樹都已經存在、已經編譯，
    //   `Command.cpp:15170` 今天就整條在用它們。
    //     InArmSuck / OutArmSuck   aHotPlateSubstrate.h:624 起（extern TMyKitSuck）
    //     ShuttleHasIC()           csystem.cpp:21276 = InputShuttleHasIC()||OutputShuttleHasIC()
    //     IndexHasIC()             csystem.cpp:21306 = TestHeadHasIC()||TestSocketHasIC()
    //
    // ⚠⚠ **發佈之前必須先修 HasIC()，而那正是 P1 做的事。**
    //   Steven 自己在 §0.1 的警告欄寫了：移植樹的 `TMyKitSuck::HasIC()` 只認
    //   `HAS_IC` / `HAS_HOT_IC`，而 golden 是「任何 `Item!=NULL_IC` 都算有料」
    //   ⇒ 吸嘴上掛著 `HAS_CLEAN_IC(7)` / `HAS_TRY_SUCK_IC(10)` 時，
    //   **本樹會誤判成「淨空、可以進維護模式」**。
    //   在修好之前把這五個 tag 發出去，等於把一個**錯的**淨空判斷變成
    //   瀏覽器的守衛 —— 比不發還糟。
    //   20260919 的 P1 已經把三支述詞改回 golden（`aHotPlateSubstrate.cpp` 的
    //   W906-P1-HASIC 區塊，驗收見 tests/test_kitsuck_predicates.cpp），
    //   所以現在發是安全的。**順序是承重的，不要把這兩件事拆開搬。**
    //
    // ⚠ null 的保守值與 `guard.systemStart` **相反**，這是 Steven 契約 §8 特別
    //   點名「最容易寫反」的一格：
    //       guard.systemStart   不可知 -> 當成 true （當成在跑）
    //       guard.machine.empty 不可知 -> 當成 false（當成**還有料**）
    //   兩者方向一致（都往擋得住倒），但寫出來的值相反。
    //   C++ 端的做法是一樣的：liveness 用 `pumping`，不可知就送 null，
    //   由瀏覽器照契約解釋。**liveness 絕不可以用 `true`** —— 那會送出一個
    //   「機台是空的」，而它的真正意思只是「這個行程從來沒跑過 kernel tick」。
    //
    // 彙總與分項兩種都送：**彙總是守衛**（瀏覽器只讀它來 gate），
    // **分項是說明**（擋下來時要能告訴操作員是哪一站還有料）。
    // 同一個模式本檔已有先例：上面的 `pump.guard.*` 就是推導值與原始項一起送。
    //
    // ⚠ TagCoverage 刻意**不**計這五個，理由與上面 `guard.*` 那段相同：
    //   它們 gate 在 `pumping` 上 —— 那是這個行程自己發動的一次執行，
    //   不是它從機台讀到的一個新資料來源。
    const bool inArmHasIC   = pumping && InArmSuck.HasIC();
    const bool outArmHasIC  = pumping && OutArmSuck.HasIC();
    const bool shuttleHasIC = pumping && ShuttleHasIC();
    const bool indexHasIC   = pumping && IndexHasIC();
    stageBool(snap, "guard.machine.inArmHasIC",   pumping, inArmHasIC);
    stageBool(snap, "guard.machine.outArmHasIC",  pumping, outArmHasIC);
    stageBool(snap, "guard.machine.shuttleHasIC", pumping, shuttleHasIC);
    stageBool(snap, "guard.machine.indexHasIC",   pumping, indexHasIC);
    stageBool(snap, "guard.machine.empty",        pumping,
              !(inArmHasIC || outArmHasIC || shuttleHasIC || indexHasIC));

    // Liveness. guMainProcCallCount is file-static in csystem.cpp, so it is read
    // through golden's own accessor (csystem.h:52) rather than an extern.
    stageInt (snap, "pump.ticks",         pumping, static_cast<long long>(g_pumpTicks));
    stageInt (snap, "pump.mainProcCalls", pumping, GetMainProcCallCount());
    stageInt (snap, "pump.exceptions",    pumping, static_cast<long long>(g_pumpExceptions));
    stageBool(snap, "pump.alive",         pumping, IsMainProcAlive(60));
    if (pumping) {
        snap.stage("pump.tickKind",
                   TagValue::makeString(g_lastTickKind == 1 ? "A" :
                                        g_lastTickKind == 2 ? "B" : "?"));
    } else {
        stageNull(snap, "pump.tickKind");
    }

    // Engine cursors. These are the things that actually MOVE, and therefore the
    // evidence that the translated engines are cycling rather than merely linked.
    stageInt(snap, "pump.task.load",      pumping, LoadTask);
    stageInt(snap, "pump.task.inArm",     pumping, iArmTask);
    stageInt(snap, "pump.task.outArm",    pumping, OutArmTask);
    stageInt(snap, "pump.task.sht1",      pumping, AutoSHT1Task);
    stageInt(snap, "pump.task.sht2",      pumping, AutoSHT2Task);
    stageInt(snap, "pump.task.testHead",  pumping, iTestHeadMotorTask);
    stageInt(snap, "pump.task.catchTray", pumping, CatchTrayTask);

    // --- FW-1c: unloader tray counters ---------------------------------------
    //AI(W906-FW1c) 20260819: iUnloaderTrayCountCal[] (cmydef.h:5216) is a
    // RUNTIME accumulator -- incremented by the asendic_Auto spine as trays
    // fill (increment fidelity re-verified this wave: 5/5 ++ sites, 25/25 refs
    // match golden asendic_Auto.cpp), zeroed by acatchtray on tray exchange.
    // Without a pump there is no run and the number has no meaning, so it
    // gates on `pumping` like the task cursors above -- "not loaded" and
    // "not running" are different questions, and this source answers the
    // second (RECON_FW1 §6#3).
    stageInt(snap, "lot.auto1.trayCount", pumping, iUnloaderTrayCountCal[0]);
    stageInt(snap, "lot.auto2.trayCount", pumping, iUnloaderTrayCountCal[1]);
    stageInt(snap, "lot.auto3.trayCount", pumping, iUnloaderTrayCountCal[2]);
    PublishYield_();   // AI(W906-LAT-1) 20260925: yield point -- the machine half is staged, the pci1203 half (the bulk: ~110 tags per axis slot) follows. ⚠ Before `mon` is fetched below, so the pointer is taken after any output served here. On the old blank line
    // --- 1203MON: the PCIE-1203 card, axes and digital inputs ---------------
    //AI(W906-1203MON-5) 20260907: user requirement 20260907 -- monitor the 1203
    // from a BCB6 screen over TCPIP. The BCB6 client is a SECOND consumer of the
    // same line-JSON feed wb_gateway already reads (TcpTagLink.h, maxConnections
    // 4), so no new transport was written; these tags are the payload it needs.
    //
    // ⚠ EVERY TAG HERE IS NULL UNLESS THE MONITOR IS ACTUALLY OPEN. That is not
    // a placeholder, it is this file's own rule (WebBridgeTags.h, "NULL, NEVER
    // ZERO") applied to the highest-stakes source in the tree: the card is
    // PHYSICALLY PRESENT on this machine with Status OK, and an axis reported at
    // position 0.000 because nobody read it is indistinguishable from an axis
    // parked at its home position. On a real handler that is the difference
    // between "safe to open the door" and "not".
    //   AI(W906-MW1) 20260908: the sentence above used to read "UNLESS SOMEONE
    //   TYPED --pci1203". That flag no longer exists -- the switch is
    //   `#define INSTALL_1203_MONITOR` at the end of MachineType.h and it is
    //   DEFAULT ON, by user ruling, because this box is a handler with the card
    //   fitted. So on this machine these tags normally DO carry values, and the
    //   null case now means one of: the macro was commented out, the exe was
    //   built without the Advantech SDK (pci1203.linked=false), or the open
    //   failed (pci1203.lastErrorText says why). The NULL-never-zero rule is
    //   unchanged and matters just as much -- what changed is which case is
    //   normal.
    //
    // pci1203.linked is the exception that makes the rest readable: it is a
    // COMPILE-time fact (was HAVE_PCI1203 armed for this binary) and is always
    // published. Without it, "no card present", "SDK not compiled in" and
    // "monitor not enabled" are three different faults that render identically.
    {
        // 0 unless INSTALL_1203_MONITOR is defined AND the open succeeded
        TPci1203Monitor* mon = Pci1203Monitor();

        // Card layer. `linked` answers a question about the BINARY, so it is
        // truthful even with no monitor object at all -- ask the module, not
        // the instance.
        stageBool(snap, "pci1203.linked", true, Pci1203MonitorLinked());
        stageBool(snap, "pci1203.enabled", true, mon != 0);
        //AI(W906-1203CTL-22) 20260911: WHAT IS IT DOING RIGHT NOW -- always
        // published, like linked/enabled, and for the same reason one step
        // further on. Opening the card takes up to ~10 s of ring scan and axis
        // sweep, and from the browser that was indistinguishable from a hang.
        // The user said so: "根本不知道發生怎回事擋掉?".
        stageStr(snap, "pci1203.phase", true, AnsiString(Pci1203OpenPhase()));
        if (g_streamWanted == 0 || g_streamWanted("pci1203")) {   //AI(W906-STREAM-2B) 20260930: the rest of the 1203 block (~5,900 tags) only while some browser has the 1203 settings window (background.html id pci1203) open or minimized -- RULINGS_20260930 #12; linked / enabled / phase above stay always-published (St01 stream-by-open-page.md s4.3). Closed: the tags vanish, the server sends them once as null ("---"); opened: the next publish stages them again. On the old blank line
        if (mon == 0) {
            //AI(W906-1203MON-8) 20260907
            stageNull(snap, "pci1203.mode");
            stageNull(snap, "pci1203.reattaches");
            stageNull(snap, "pci1203.detaches");
            stageNull(snap, "pci1203.open");
            stageNull(snap, "pci1203.enumerated");
            stageNull(snap, "pci1203.devCount");
            stageNull(snap, "pci1203.devNum");
            stageNull(snap, "pci1203.devName");
            stageNull(snap, "pci1203.subDevices");
            stageNull(snap, "pci1203.slaveState");
            stageNull(snap, "pci1203.axesOpened");
            stageNull(snap, "pci1203.pollMs");
            stageNull(snap, "pci1203.pollCount");
            stageNull(snap, "pci1203.pollErrors");
            stageNull(snap, "pci1203.lastError");
            //AI(W906-MW1) 20260908: this string is the ONLY instruction a person
            // reading the screen gets, and it used to say "start with
            // --pci1203" -- a command that now exits 2. Both consumers render
            // it verbatim (the web 1203 page and D:\BCB6_1203_UI's Unit1.cpp),
            // so a stale instruction here becomes a stale instruction on two
            // screens. It names the file and the macro rather than a command.
            stageStr (snap, "pci1203.lastErrorText", true,
                      AnsiString("monitor not enabled -- #define "
                                 "INSTALL_1203_MONITOR at the end of "
                                 "MachineType.h and rebuild"));
            stageNull(snap, "pci1203.disabled");
            stageNull(snap, "pci1203.disabledReason");
            //AI(W906-1203MON-18) 20260910: the card's self-reported IO width
            // and ring populations. Present-and-null, never absent -- a
            // consumer must be able to bind these slots on the first snapshot
            // whether or not the monitor is enabled.
            stageNull(snap, "pci1203.diMaxChan");
            stageNull(snap, "pci1203.doMaxChan");
            stageNull(snap, "pci1203.ring0Slaves");
            stageNull(snap, "pci1203.ring1Slaves");
            stageNull(snap, "pci1203.idConflict");   //AI(W906-1203MON-19) 20260910
            //AI(W906-1203COLD-1) 20260918: the cold-boot open wait. Present-and-
            // null on the no-monitor path for the same reason as everything
            // else in this block: a page that binds a tag only when the monitor
            // happens to exist shows nothing in the case where the number
            // matters most.
            stageNull(snap, "pci1203.openWaitSec");
            stageNull(snap, "pci1203.openAttempts");
            //AI(W906-1203CTL-33) 20260911: same present-and-null rule for the
            // IO-width truncation trio. A page that only binds these when the
            // monitor happens to be enabled would show nothing at all in the
            // case where the warning matters most.
            stageNull(snap, "pci1203.di.portsOnCard");
            stageNull(snap, "pci1203.do.portsOnCard");
            stageNull(snap, "pci1203.di.portsSampled");
            stageNull(snap, "pci1203.do.portsSampled");
            stageNull(snap, "pci1203.di.truncated");
            stageNull(snap, "pci1203.do.truncated");  stageNull(snap, "pci1203.di.mode"); stageNull(snap, "pci1203.di.batchMatches"); stageNull(snap, "pci1203.di.batchChecks"); stageNull(snap, "pci1203.di.batchMismatches"); stageNull(snap, "pci1203.di.batchFirstMismatch"); stageNull(snap, "pci1203.di.batchFails"); stageNull(snap, "pci1203.di.batchWhy"); stageNull(snap, "pci1203.torque.focusAxis"); stageNull(snap, "pci1203.torque.focusWhy"); stageNull(snap, "pci1203.torque.sdoReads"); stageNull(snap, "pci1203.torque.sdoErrors"); stageNull(snap, "pci1203.torque.sdoMs"); stageNull(snap, "pci1203.torque.sdoMaxMs");  stageNull(snap, "pci1203.di.spotPorts"); stageNull(snap, "pci1203.di.spotDrops"); stageNull(snap, "pci1203.di.stationDrops"); stageNull(snap, "pci1203.torque.sdoSuspended"); stageNull(snap, "pci1203.torque.sdoFailRun"); stageNull(snap, "pci1203.torque.sdoWhy");   //AI(W906-MT-E3a) 20260925: batch DI mode + torque focus, present-and-null with no monitor (same rule as the truncation trio above). Same line, no line below moves   //AI(W906-MT-FIX1) 20260926: + the batch spot check, the station-state drops and the focus SDO backoff, same rule
        } else {
            const Pci1203CardSample& c = mon->card();
            const bool live = c.open;

            //AI(W906-1203MON-8) 20260907: mode is the FIRST thing to read on
            // the card panel, because "owned" on a running machine is a
            // WARNING and not a success -- it means INSTALL_ETHETCAT() is
            // false, so the machine's own 1203 path never opened the card and
            // nothing on screen reflects production's view of it.
            stageStr (snap, "pci1203.mode",        true,
                      AnsiString(Pci1203ModeText(c.mode)));
            stageInt (snap, "pci1203.reattaches",  true, (long long)c.reattaches);
            stageInt (snap, "pci1203.detaches",    true, (long long)c.detaches);
            stageBool(snap, "pci1203.open",        true, c.open);
            stageBool(snap, "pci1203.enumerated",  true, c.enumerated);
            //AI(W906-1203MON-8) 20260907: devCount/devNum/devName/subDevices
            // are learned ONLY by enumerating, and the ATTACHED path does not
            // enumerate (production already did). So all four now gate on
            // `c.enumerated` rather than `live`: in attached mode they publish
            // null, which is the truthful "we did not look" instead of a
            // plausible-looking blank name and a zero device number.
            stageInt (snap, "pci1203.devCount",    c.enumerated, (long long)c.devCount);
            stageInt (snap, "pci1203.devNum",      c.enumerated, (long long)c.devNum);
            stageStr (snap, "pci1203.devName",     c.enumerated, AnsiString(c.devName.c_str()));
            stageInt (snap, "pci1203.subDevices",  c.enumerated, c.subDevices);
            // slaveValid, not `live`: the device can be open while the EtherCAT
            // ring read fails, and those are different faults.
            stageInt (snap, "pci1203.slaveState",  c.slaveValid, (long long)c.slaveState);
            stageInt (snap, "pci1203.axesOpened",  live, c.axesOpened);
            stageInt (snap, "pci1203.pollMs",      c.pollCount > 0, (long long)c.pollMs);
            stageInt (snap, "pci1203.pollCount",   true, (long long)c.pollCount);
            stageInt (snap, "pci1203.pollErrors",  true, (long long)c.pollErrors);
            stageInt (snap, "pci1203.lastError",   true, (long long)c.lastError);
            stageStr (snap, "pci1203.lastErrorText", true, AnsiString(c.lastErrorText.c_str()));
            //AI(W906-1203MON-18) 20260910: what the CARD says its IO image is,
            // and how many stations each ring reports. A panel sizes its lamp
            // grid from these rather than from this file's constants, so an
            // added rack shows up as more lamps instead of silently missing
            // ones.
            //   ⚠ Gated on the SEPARATE validity flag, not on the value: 0 DI
            //   channels is a legitimate reading (an empty ring reports it,
            //   and the vendor example prints "There is no Ethcat DI Slaves"),
            //   whereas a failed property read must show as null. Publishing 0
            //   for both would merge "no inputs" with "did not look".
            stageInt (snap, "pci1203.diMaxChan",   c.diMaxChanValid, (long long)c.diMaxChan);
            stageInt (snap, "pci1203.doMaxChan",   c.doMaxChanValid, (long long)c.doMaxChan);

            //AI(W906-1203CTL-33) 20260911: THE CARD'S WIDTH AGAINST THE WIDTH WE
            // ACTUALLY SAMPLED, and a boolean that says whether they differ.
            //
            // diMaxChan was already on the wire and was already enough to work
            // this out -- 888 channels is 111 ports, the page requested 96 --
            // but NOBODY DIVIDES BY EIGHT AND COMPARES. A user found the gap
            // instead: station 0x51 straddled the ceiling so three of its four
            // bytes were absent, which reads as "that module is broken" rather
            // than "that module was never polled". Three further modules behind
            // it (0x52/0x53/0x54) were missing in full, silently.
            //
            // ⚠ These are computed from the CARD's own property, not from a
            // constant in this tree, so they stay true when the rack changes --
            // which is the failure mode every previous sizing comment missed.
            // Gated on the property read succeeding: "the card did not tell us
            // its width" is a different fact from "the widths match".
            const long long diOnCard = c.diMaxChanValid ? (long long)(c.diMaxChan / 8) : 0;
            const long long doOnCard = c.doMaxChanValid ? (long long)(c.doMaxChan / 8) : 0;
            const long long diSampled = (long long)(mon ? mon->diCount() : 0);
            const long long doSampled = (long long)(mon ? mon->doCount() : 0);
            stageInt (snap, "pci1203.di.portsOnCard",    c.diMaxChanValid, diOnCard);
            stageInt (snap, "pci1203.do.portsOnCard",    c.doMaxChanValid, doOnCard);
            stageInt (snap, "pci1203.di.portsSampled",   mon != 0, diSampled);
            stageInt (snap, "pci1203.do.portsSampled",   mon != 0, doSampled);
            stageBool(snap, "pci1203.di.truncated", c.diMaxChanValid, diOnCard > diSampled);
            stageBool(snap, "pci1203.do.truncated", c.doMaxChanValid, doOnCard > doSampled);  stageStr(snap, "pci1203.di.mode", live, AnsiString(c.diBatchOk ? "batch" : "perByte")); stageInt(snap, "pci1203.di.batchMatches", live, (long long)c.diBatchMatches); stageInt(snap, "pci1203.di.batchChecks", true, (long long)c.diBatchChecks); stageInt(snap, "pci1203.di.batchMismatches", true, (long long)c.diBatchMismatches); stageInt(snap, "pci1203.di.batchFirstMismatch", c.diBatchFirstMismatch >= 0, (long long)c.diBatchFirstMismatch); stageInt(snap, "pci1203.di.batchFails", true, (long long)c.diBatchFails); stageStr(snap, "pci1203.di.batchWhy", !c.diBatchWhy.empty(), AnsiString(c.diBatchWhy.c_str())); stageInt(snap, "pci1203.torque.focusAxis", c.torqueFocusAxis >= 0, (long long)c.torqueFocusAxis); stageStr(snap, "pci1203.torque.focusWhy", !c.torqueFocusWhy.empty(), AnsiString(c.torqueFocusWhy.c_str())); stageInt(snap, "pci1203.torque.sdoReads", true, (long long)c.torqueSdoReads); stageInt(snap, "pci1203.torque.sdoErrors", true, (long long)c.torqueSdoErrors); stageDouble(snap, "pci1203.torque.sdoMs", c.torqueSdoReads > 0, c.torqueSdoMs); stageDouble(snap, "pci1203.torque.sdoMaxMs", c.torqueSdoReads > 0, c.torqueSdoMaxMs);  stageInt(snap, "pci1203.di.spotPorts", true, (long long)c.diSpotPorts); stageInt(snap, "pci1203.di.spotDrops", true, (long long)c.diSpotDrops); stageInt(snap, "pci1203.di.stationDrops", true, (long long)c.diStationDrops); stageBool(snap, "pci1203.torque.sdoSuspended", c.torqueFocusAxis >= 0, c.torqueSdoSuspended); stageInt(snap, "pci1203.torque.sdoFailRun", c.torqueFocusAxis >= 0, (long long)c.torqueSdoFailRun); stageStr(snap, "pci1203.torque.sdoWhy", c.torqueFocusAxis >= 0 && !c.torqueSdoWhy.empty(), AnsiString(c.torqueSdoWhy.c_str()));   //AI(W906-MT-FIX1) 20260926 (review 20260926): spotPorts = ports the batch spot check read per byte (cumulative), spotDrops = spot checks that dropped batch mode, stationDrops = DI-station state events that dropped it; sdoSuspended / sdoFailRun / sdoWhy = the focus 6077h read's failure backoff and why the focus axis is not being read -- null without a focus, since they describe the focused drive   //AI(W906-MT-E3a) 20260925: WHICH DI READ PRODUCED THE BYTES, said on the wire rather than inferred -- "batch" only after the one-call read matched the per-byte read twice (Pci1203CardSample::diBatchOk), and batchWhy says why in words; plus the torque SDO focus and what each 6077h mailbox read costs (the on-machine measurement EastSun ruled for later, R3). Same line, no line below moves
            stageInt (snap, "pci1203.ring0Slaves", c.ringCountValid, (long long)c.ring0Slaves);
            stageInt (snap, "pci1203.ring1Slaves", c.ringCountValid, (long long)c.ring1Slaves);
            //AI(W906-1203MON-19) 20260910: open, but with conflicting
            // SubDevice IDs. A populated panel gives an operator no way to
            // know that a reading may belong to a different station, so this
            // is always present (never absent) and never null once there is a
            // monitor object.
            stageBool(snap, "pci1203.idConflict",  true, c.idConflict);
            //AI(W906-1203COLD-1) 20260918: how long Acm_DevOpen had to wait for
            // the EtherCAT slaves, and how many attempts it took. 0 / 0 on a
            // warm start. A rising number here is the ring taking longer to come
            // up -- a fact about the hardware, visible before it becomes a
            // failure, which is the whole point of putting it on the wire rather
            // than only in the start-up log.
            stageInt(snap, "pci1203.openWaitSec",  true, (long long)c.openWaitSec);
            stageInt(snap, "pci1203.openAttempts", true, (long long)c.openAttempts);
            stageBool(snap, "pci1203.disabled",    true, mon->Disabled());
            stageStr (snap, "pci1203.disabledReason", true,
                      AnsiString(mon->disabledReason().c_str()));
        }

        // -------------------------------------------------------------------
        //  CONTROL layer.  AI(W906-1203CTL-7) 20260911.
        //
        //  ⚠ THE FIRST THREE ARE THE ONES THAT MUST NEVER BE NULL, because they
        //  answer "can this screen command anything at all?" -- and the three
        //  ways the answer is no (not compiled in, not armed, armed but dry)
        //  render IDENTICALLY on a page full of buttons that do nothing. The
        //  same reasoning that made pci1203.linked a compile-time tag.
        //
        //  ⚠ AND control.dryRun IS NOT A DEBUG FLAG ON THIS SCREEN. It is the
        //  difference between a button that moves a servo and one that writes a
        //  line in a log. A page showing operable controls without showing this
        //  would be lying about what a click does.
// ===== BU-C4 UN-GATED 20260918 -- the 14 pci1203.control.* tags =====
//   AI(W906-BU-C-P4) 20260918: this block spent 20260916-20260918 inside
//   `#if 0`.  The gate's own stated reason was "EtherCAT/Pci1203Control.{h,cpp}
//   is deliberately not in this tree yet".  That premise is now dead -- the two
//   files landed with BU-C4 and P1 verified them at 0 self-diagnostics on both
//   sides of HAVE_PCI1203.
//
//   ⚠ A dead premise is not by itself a licence to un-gate, so the question was
//   asked again from scratch: does publishing these 14 tags DO anything?
//   Measured, not assumed -- every call in this block is a pure read:
//       Pci1203Control()        Pci1203Control.cpp:2993   `{ return g_control; }`
//       Pci1203ControlLinked()  Pci1203Control.cpp:2995   compile-time constant
//       IsDryRun / acceptedCount / refusedCount / issuedCount   all `const`
//       last()                  returns a const ref to a POD
//   Nothing here opens the device, arms the layer, or issues a command.  There
//   is no lazy construction: the accessor returns whatever another caller has
//   already put in g_control, and 0 when nobody has.
//
//   ⓘ And the direction matters.  Since the 20260918 "甲" ruling,
//   WB_PUMP_1203_CONTROL_LIVE is ACTIVE, so on a HAVE_PCI1203=1 build a browser
//   click can move the machine.  Leaving `armed` and `dryRun` unpublished means
//   the page cannot show the operator which of those two worlds they are in.
//   Publishing them is the safety-POSITIVE half of that ruling.
        {
            TPci1203Control* ctl = Pci1203Control();
            stageBool(snap, "pci1203.control.linked",  true, Pci1203ControlLinked());
            stageBool(snap, "pci1203.control.armed",   true, ctl != 0);
            stageBool(snap, "pci1203.control.dryRun",  true, ctl == 0 || ctl->IsDryRun());

            if (ctl == 0) {
                stageNull(snap, "pci1203.control.accepted");
                stageNull(snap, "pci1203.control.refused");
                stageNull(snap, "pci1203.control.issued");
                stageNull(snap, "pci1203.control.lastId");
                stageNull(snap, "pci1203.control.lastCmd");
                stageNull(snap, "pci1203.control.lastOk");
                stageNull(snap, "pci1203.control.lastIssued");
                stageNull(snap, "pci1203.control.lastRet");
        stageNull(snap, "pci1203.control.lastRetText");
                stageNull(snap, "pci1203.control.lastWhy");
                stageNull(snap, "pci1203.control.lastCall");
            } else {
                stageInt(snap, "pci1203.control.accepted", true,
                         (long long)ctl->acceptedCount());
                stageInt(snap, "pci1203.control.refused",  true,
                         (long long)ctl->refusedCount());
                stageInt(snap, "pci1203.control.issued",   true,
                         (long long)ctl->issuedCount());

                // ⚠ `valid` false means NOTHING HAS BEEN COMMANDED YET, which is
                // a different fact from "the last command failed" and must
                // render as "---" rather than as a refusal.
                const Pci1203LastCmd& L = ctl->last();
                stageInt (snap, "pci1203.control.lastId",     L.valid, L.wireId);
                stageStr (snap, "pci1203.control.lastCmd",    L.valid,
                          AnsiString(L.name.c_str()));
                stageBool(snap, "pci1203.control.lastOk",     L.valid, L.accepted);
                stageBool(snap, "pci1203.control.lastIssued", L.valid, L.issued);
                // Only meaningful when something was actually issued -- a return
                // code from a command that was never sent would read as a
                // vendor SUCCESS (0).
                stageInt (snap, "pci1203.control.lastRet",    L.valid && L.issued,
                          (long long)L.ret);
                //AI(W906-1203CTL-36) 20260911: and what that code MEANS, in the
                // vendor's own words. Present whenever a command was issued --
                // empty string for a successful one, which the page renders as
                // nothing rather than as a claim.
                stageStr (snap, "pci1203.control.lastRetText", L.valid && L.issued,
                          AnsiString(L.retText.c_str()));
                stageStr (snap, "pci1203.control.lastWhy",    L.valid,
                          AnsiString(L.why.c_str()));
                stageStr (snap, "pci1203.control.lastCall",   L.valid,
                          AnsiString(L.wouldCall.c_str()));
            }
        }
// ===== end BU-C4 block =====

        // Axis layer. The tag SHAPE is fixed at kPci1203TagAxes regardless of
        // how many axes exist, and that is on purpose: a wire whose tag set
        // changes size between frames makes a browser or a BCB6 grid rebuild
        // its bindings mid-run, and the "removed" list exists for tags that
        // genuinely went away, not for a display that has not been enabled yet.
        for (int i = 0; i < kPci1203TagAxes; ++i) {
            char tag[48];
            const Pci1203AxisSample* a = 0;
            if (mon != 0 && i < mon->axisCount()) a = &mon->axis(i);
            const bool ok = (a != 0 && a->valid);

            std::snprintf(tag, sizeof(tag), "pci1203.ax%d.opened", i);
            if (a) stageBool(snap, tag, true, a->opened); else stageNull(snap, tag);

            //AI(W906-1203MON-12) 20260907: the ADDRESS this slot was opened at,
            // which is what makes a row correlatable to a Mot_Table.csv line
            // (same BoardID/Port key). Gated on `byId` and NOT on `opened`:
            // a physical-index open has no board/port, and publishing 0/0
            // there would invent an address that maps to a real motor row.
            //AI(W906-1203CTL-32) 20260911: `boardId`/`port` RETIRED as tag names
            // in favour of `station`/`stationAxis`, which is what the vendor API
            // calls those two arguments. ⚠ The old names were actively
            // misleading: this machine has exactly ONE motion card, so a page
            // reading "BoardID 3" was describing hardware that is not in the
            // chassis. `poolIndex` is published beside them because station and
            // stationAxis are a ROUTE to a motor, not its identity -- see
            // Pci1203Monitor.h. Equal poolIndex == same physical motor.
            //AI(W906-1203CTL-46) 20260912: gate on the thing these tags actually
            // depend on -- a known station -- not on `byId`, which named the
            // open call and stopped being true when CTL-43 switched to
            // Acm_AxOpen. Gating a station label on "which function opened the
            // handle" was always indirect; it only looked right while the two
            // happened to coincide.
            const bool addressed = (a != 0 && a->station >= 0);
            std::snprintf(tag, sizeof(tag), "pci1203.ax%d.byId", i);
            if (a) stageBool(snap, tag, true, a->byId); else stageNull(snap, tag);
            std::snprintf(tag, sizeof(tag), "pci1203.ax%d.station", i);
            stageInt(snap, tag, addressed, (long long)(a ? a->station : 0));
            //AI(W906-1203PHYS-1) 20260916: ⚠ the axis is real, its station number is
            //  not unique. Published so the page can refuse to offer parameter
            //  writes instead of offering them and having the C++ side refuse --
            //  a button that is always refused teaches an operator to ignore
            //  refusals. See Pci1203AxisSample::stationAmbiguous.
            std::snprintf(tag, sizeof(tag), "pci1203.ax%d.stationAmbiguous", i);
            stageBool(snap, tag, addressed, (a != 0 && a->stationAmbiguous));
            std::snprintf(tag, sizeof(tag), "pci1203.ax%d.stationAxis", i);
            stageInt(snap, tag, addressed, (long long)(a ? a->stationAxis : 0));
            std::snprintf(tag, sizeof(tag), "pci1203.ax%d.stationAxes", i);
            stageInt(snap, tag, addressed, (long long)(a ? a->stationAxes : 0));
            std::snprintf(tag, sizeof(tag), "pci1203.ax%d.poolIndex", i);
            stageInt(snap, tag, addressed, (long long)(a ? a->poolIndex : 0));

            std::snprintf(tag, sizeof(tag), "pci1203.ax%d.state", i);
            stageInt(snap, tag, ok, (long long)(a ? a->state : 0));

            // Decoded alongside the raw value, never instead of it. The 16
            // STA_AX_* constants were verified 20260907 to hold IDENTICAL
            // values in the installed 2.0.13.2 header and the tree's 2.0.15.2
            // one, so this decode is safe across the version skew -- that check
            // is the only reason a text label is published at all.
            std::snprintf(tag, sizeof(tag), "pci1203.ax%d.stateText", i);
            stageStr(snap, tag, ok,
                     AnsiString(ok ? Pci1203AxisStateText(a->state) : ""));

            std::snprintf(tag, sizeof(tag), "pci1203.ax%d.cmdPos", i);
            stageDouble(snap, tag, ok, a ? a->cmdPos : 0.0);

            std::snprintf(tag, sizeof(tag), "pci1203.ax%d.actPos", i);
            stageDouble(snap, tag, ok, a ? a->actPos : 0.0);

            std::snprintf(tag, sizeof(tag), "pci1203.ax%d.motionIO", i);
            stageInt(snap, tag, ok, (long long)(a ? a->motionIO : 0));

            //AI(W906-1203CTL-34) 20260911: +命令速度, matching the Utility's
            // 當前狀態 block.
            std::snprintf(tag, sizeof(tag), "pci1203.ax%d.cmdVel", i);
            stageDouble(snap, tag, ok, a ? a->cmdVel : 0.0);  { const bool tv = (a != 0 && a->torqueValid); std::snprintf(tag, sizeof(tag), "pci1203.ax%d.torque", i); stageDouble(snap, tag, tv, tv ? (double)a->torqueRaw : 0.0); std::snprintf(tag, sizeof(tag), "pci1203.ax%d.torquePct", i); stageDouble(snap, tag, (a != 0 && a->torquePctValid), (a != 0 && a->torquePctValid) ? a->torquePct : 0.0); std::snprintf(tag, sizeof(tag), "pci1203.ax%d.torqueNm", i); stageDouble(snap, tag, (a != 0 && a->torqueNmValid), (a != 0 && a->torqueNmValid) ? a->torqueNm : 0.0); std::snprintf(tag, sizeof(tag), "pci1203.ax%d.torqueSrc", i); stageStr(snap, tag, tv, AnsiString(tv ? (a->torqueSrc == 2 ? "sdo" : "pdo") : "")); std::snprintf(tag, sizeof(tag), "pci1203.ax%d.torqueUnitVerified", i); stageBool(snap, tag, tv, tv && a->torqueUnitVerified); std::snprintf(tag, sizeof(tag), "pci1203.ax%d.torqueErr", i); stageInt(snap, tag, (a != 0 && a->opened), (long long)(a ? a->torqueErr : 0)); }   //AI(W906-MT-E3a) 20260925: ACTUAL TORQUE (EastSun R6). torque = the raw reading in the drive's [Trq.unit] (6077h/6877h); torquePct / torqueNm only when its unit is KNOWN (SDO, or a PDO value that agreed with SDO -- torqueUnitVerified) and 2704h / 6076h were read; torqueSrc "pdo" (Acm_AxGetActTorque) | "sdo" (the focus axis); torqueErr = the PDO read's last return (0x8000009F = the ring's PDO map has no torque). All null when not read THIS poll -- never 0. Deliberately NOT gated on `ok`: an unmapped torque must not blank a position, and a position failure must not hide a torque. Same line, no line below moves

            //AI(W906-1203CTL-34) 20260911: the DIAL NUMBER, beside the address.
            // `station` is what Acm_AxOpenbyID takes; `stationAlias` is what is
            // printed on the drive and what Common Motion Utility displays.
            // They are DIFFERENT on this ring (addr 30 is the drive labelled 7),
            // and publishing only the first is how this page came to show a
            // number no cabinet door carries.
            std::snprintf(tag, sizeof(tag), "pci1203.ax%d.stationAlias", i);
            stageInt(snap, tag, (a != 0 && a->stationAlias >= 0),
                     (long long)(a ? a->stationAlias : 0));

            //AI(W906-1203CTL-34) 20260911: THE DRIVE'S OWN ERROR AND ITS OWN
            // WORDS. User: "有異常他也會說明是甚麼異常". The text comes from
            // Acm_GetErrorMessage -- the vendor's string, not a sentence
            // written in this tree -- so it says exactly what Common Motion
            // Utility's 錯誤資訊 field says and cannot drift from it.
            //   ⚠ Present whenever there IS an axis, not gated on `ok`: a drive
            //   error is most worth showing when the ordinary reads are failing.
            std::snprintf(tag, sizeof(tag), "pci1203.ax%d.driveErr", i);
            stageInt(snap, tag, a != 0, (long long)(a ? a->driveErr : 0));
            std::snprintf(tag, sizeof(tag), "pci1203.ax%d.driveErrText", i);
            stageStr(snap, tag, a != 0,
                     AnsiString(a ? a->driveErrText.c_str() : ""));

            //AI(W906-1203ALM-1) 20260912: THE ALARM NUMBER OFF THE DRIVE ITSELF
            // -- CoE 603Fh, which is the A.xxx its LED panel shows. driveErr
            // above is the MASTER's word and it stops at the family: ADVMOT.dll
            // renders 0x83100000, measured verbatim, as "Drive error
            // (0x8310xxxx), please refer to drive manual or drive LED panel for
            // error xxxx." These three tags ARE the xxxx.
            //
            //   ⚠ driveAlarm is gated on driveAlarmValid, not published as 0.
            //   "No alarm code" and "nobody has read one" are different facts
            //   and a zero renders identically to both -- the same null-never-
            //   zero rule the speed properties follow two blocks down.
            //
            //   ⚠ driveAlarmName is EMPTY for a drive that is not Sigma-X, by
            //   design. 603Fh exists on every CiA402 drive but its meaning is
            //   vendor-defined, so naming another vendor's 0x0720 "Continuous
            //   Overload" would be a confident wrong diagnosis on a machine
            //   screen. User ruling 20260912: "如果是使用yaskawa 才寫入面板".
            //   The raw number still publishes; only the NAME is withheld.
            //   driveModel is published beside it so the page can SAY why a
            //   name is missing rather than just showing a blank.
            std::snprintf(tag, sizeof(tag), "pci1203.ax%d.driveAlarm", i);
            stageInt(snap, tag, (a != 0 && a->driveAlarmValid),
                     (long long)(a ? a->driveAlarm : 0));
            std::snprintf(tag, sizeof(tag), "pci1203.ax%d.driveAlarmName", i);
            stageStr(snap, tag, (a != 0 && a->driveAlarmValid),
                     AnsiString(a ? a->driveAlarmName.c_str() : ""));
            std::snprintf(tag, sizeof(tag), "pci1203.ax%d.driveModel", i);
            stageStr(snap, tag, (a != 0 && !a->driveModel.empty()),
                     AnsiString(a ? a->driveModel.c_str() : ""));

            //AI(W906-1203ALM-21) 20260915: THE LIMIT CONFIGURATION THE CARD
            // HOLDS. User: "我希望可以在介面 可以控制每顆馬達 設定正負極限
            // 相關參數".
            //   ⚠ Gated per-property on its own read succeeding, like the
            //   speeds. On a protection the difference matters more than it
            //   does on a speed: "the card did not tell us whether the limit is
            //   on" and "the limit is off" are opposite facts, and a 0 renders
            //   identically to both.
            //   Names match Pci1203LimitParam so a value on screen can be
            //   checked against the SDK header and against what the set button
            //   writes.
            {
                static const char* kLimitName[Pci1203AxisSample::kLimitReadCount] = {
                    "elEnable", "pelEnable", "melEnable", "elReact",
                    "swPelEnable", "swMelEnable", "swPelValue", "swMelValue",
                    //AI(W906-1203LOGIC-1) 20260917: HLMT+ / HLMT- Logic. ⚠ The
                    //  order here is the THIRD copy of Pci1203LimitParam's
                    //  order -- enum, monitor read table, these names -- and
                    //  nothing checks that the three agree. A name appended in
                    //  the wrong place does not fail, it relabels every value
                    //  from that slot on, which on a protection panel reads as
                    //  "the limits are configured wrongly".
                    "pelLogic", "melLogic"
                };
                for (int q = 0; q < Pci1203AxisSample::kLimitReadCount; ++q) {
                    std::snprintf(tag, sizeof(tag), "pci1203.ax%d.limit.%s",
                                  i, kLimitName[q]);
                    stageDouble(snap, tag, (a != 0 && a->limitValid[q]),
                                a ? a->limitVal[q] : 0.0);
                }
            }

            //AI(W906-1203GEAR-1) 20260915: THE DRIVE'S ELECTRONIC GEAR.
            // User: "我現在想在介面設置電子齒輪比 / 幫我把相關參數設定介面都
            // 寫上去 / 是要真的可以設定到驅動器的".
            //   ⚠ THESE COME FROM THE SERVOPACK, NOT THE CARD, and the page
            //   says so -- every other number on the axis panel is the card's.
            //   Same per-value gating as the limits and for the same reason:
            //   a gear that was not read renders as 0, and 0 is not a legal
            //   numerator, so publishing it unconditionally would put an
            //   impossible ratio on screen and invite someone to "fix" it.
            //   Names match Pci1203GearParam and the setGear wire names, so
            //   what is displayed, what is written and what the manual calls
            //   it are all checkable against each other.
            //   ⚠ posNum/posDen ARE Pn20E/Pn210 -- measured 20260915, same
            //   storage, see EtherCAT/Pci1203Gear.h fact (1).
            {
                static const char* kGearName[Pci1203AxisSample::kGearReadCount] = {
                    "posNum", "posDen", "velNum", "velDen",
                    "accNum", "accDen", "trqNum", "trqDen"
                };
                for (int q = 0; q < Pci1203AxisSample::kGearReadCount; ++q) {
                    std::snprintf(tag, sizeof(tag), "pci1203.ax%d.gear.%s",
                                  i, kGearName[q]);
                    stageDouble(snap, tag, (a != 0 && a->gearValid[q]),
                                a ? (double)a->gearVal[q] : 0.0);
                }
                //  Which half of a two-axis SERVOPACK this is. The panel needs
                //  it to name the object it is about to write (2701h vs 2F01h),
                //  and an operator comparing this screen with SigmaWin+ needs
                //  it to know which axis pane to open.
                std::snprintf(tag, sizeof(tag), "pci1203.ax%d.gear.subAxis", i);
                stageDouble(snap, tag,
                            (a != 0 && (a->stationAxis == 0 || a->stationAxis == 1)),
                            a ? (double)a->stationAxis : 0.0);
            }

            //AI(W906-1203GEAR-2) 20260915: MOTOR AND ENCODER.
            // User: "我介面上面也要顯示 馬達型號與編碼器位數".
            //   ⚠ These are published RAW and decoded on the page, because the
            //   decode is conditional and the condition matters: Pn21D only
            //   describes the operating resolution when its own enable digit is
            //   1, and on this machine it is 0 on all nine stations. Publishing
            //   a pre-chewed "24 bit" here would put a number on screen that the
            //   drive is not actually using. The page shows the digits and says
            //   which case applies. See Pci1203AxisSample::encVal.
            {
                //AI(W906-1203POT-1) 20260915: +pn50A/+pn50B, the DRIVE half of
                // the overtravel limit. The page showed only the card's half
                // and a note saying a second half existed; a user turned the
                // card's 正極限 off, still could not move, and reported the
                // switch as broken. Publishing the drive's own allocation is
                // what turns that note into something checkable on screen.
                static const char* kEncName[Pci1203AxisSample::kEncReadCount] = {
                    "pn21D", "pn002", "ratedTorque", "pn50A", "pn50B"
                };
                for (int q = 0; q < Pci1203AxisSample::kEncReadCount; ++q) {
                    std::snprintf(tag, sizeof(tag), "pci1203.ax%d.enc.%s",
                                  i, kEncName[q]);
                    stageDouble(snap, tag, (a != 0 && a->encValid[q]),
                                a ? (double)a->encVal[q] : 0.0);
                }
                //  The EXACT unit from 1008h, beside the family name the card's
                //  scan gives. Both are published because they answer different
                //  questions and only one of them identifies a specific drive.
                std::snprintf(tag, sizeof(tag), "pci1203.ax%d.enc.model", i);
                stageStr(snap, tag, (a != 0 && !a->driveModelCoE.empty()),
                         AnsiString(a ? a->driveModelCoE.c_str() : ""));
            }
            { static const char* kTrqLimTag[2] = { "pos", "neg" };  for (int k = 0; k < 2; ++k) { const bool tv = (a != 0 && a->trqLimValid[k]); const bool tried = (a != 0 && (a->trqLimValid[k] || a->trqLimRet[k] != 0ul)); std::snprintf(tag, sizeof(tag), "pci1203.ax%d.trqLim.%s", i, kTrqLimTag[k]); stageDouble(snap, tag, tv, tv ? (double)a->trqLimVal[k] : 0.0); std::snprintf(tag, sizeof(tag), "pci1203.ax%d.trqLim.%sRead", i, kTrqLimTag[k]); stageStr(snap, tag, a != 0, AnsiString(a != 0 ? Pci1203TorqueLimitReadText(a->trqLimValid[k], a->trqLimRet[k]) : "")); std::snprintf(tag, sizeof(tag), "pci1203.ax%d.trqLim.%sRet", i, kTrqLimTag[k]); stageInt(snap, tag, tried, (long long)(a != 0 ? a->trqLimRet[k] : 0ul)); } }   //AI(W906-ONSITE-1) 20260926: THE DRIVE'S TORQUE LIMIT READ-BACK, beside the other drive-configuration tags (EastSun 20260926: measure whether the SGDXS / SGDXW drives support 60E0h / 60E1h; read-only, nothing here or in the monitor writes them). Six tags per axis slot: trqLim.pos / trqLim.neg = 60E0h / 60E1h (68E0h / 68E1h on axis B) RAW in 0.1 % of rated torque (1000 = 100 %), null unless THIS read succeeded -- never 0 for "not read", because 0 is a legal limit that means "no torque"; trqLim.posRead / negRead = "ok" | "readFailed" | "notRead" (Pci1203TorqueLimitReadText), gated on the SLOT EXISTING (`a != 0`), not on an axis being opened there -- OpenAxes_ sizes the slot array to the monitor's maxAxes (axes.assign), and wb_serve / ioweb_probe enable it with kPci1203TagAxes, so once the card has opened all 32 slots publish a word and a slot with no opened axis reads "notRead" (true: nothing was read there); null only when there is no monitor or it never got as far as opening its axes. That is the convention of the status tags beside it (opened / byId / driveErr / driveErrText are gated the same way), while the MEASUREMENT (trqLim.pos / neg) stays null until read [AI(W906-ONSITE-1) 20260926, adversarial review: the old wording "null only when the slot has no axis" was wrong -- comment corrected, behaviour unchanged]; trqLim.posRet / negRet = the vendor return of the last attempt (0 = SUCCESS), null when no read was ever attempted, so a failed read of an unsupported object and "not read yet" are told apart on the wire. Same per-slot fixed shape as every tag here. On the old blank line, so no line below moves
            //AI(W906-1203HOME-1) 20260915: THE HOMING PARAMETERS THE CARD HOLDS.
            // User: "介面也可以寫入 回HOME的相關參數 ... 這些功能我都是確定要
            // 可以寫入資料的".
            //   Same per-value gating as the limits and the speeds: "the card
            //   did not tell us this one" and "this one is zero" are different
            //   facts, and on a homing SPEED zero is the one that silently
            //   makes a home never finish.
            {
                static const char* kHomeName[Pci1203AxisSample::kHomeReadCount] = {
                    "velHigh", "velLow", "acc", "dec", "position",
                    "crossDistance", "offsetDistance", "offsetVel", "resetEnable"
                };
                for (int q = 0; q < Pci1203AxisSample::kHomeReadCount; ++q) {
                    std::snprintf(tag, sizeof(tag), "pci1203.ax%d.home.%s",
                                  i, kHomeName[q]);
                    stageDouble(snap, tag, (a != 0 && a->homeValid[q]),
                                a ? a->homeVal[q] : 0.0);
                }
                //  Pn000 RAW, decoded on the page. Raw because the direction is
                //  one digit of four and the page shows both the digit and the
                //  whole register -- an operator about to reverse a motor
                //  should see what else lives in the value being rewritten.
                std::snprintf(tag, sizeof(tag), "pci1203.ax%d.home.driveDir", i);
                stageDouble(snap, tag, (a != 0 && a->driveDirValid),
                            a ? (double)a->driveDir : 0.0);
            }

            //AI(W906-1203DHOME-1) 20260917: THE HOMING PARAMETERS THE DRIVE
            // HOLDS -- published NEXT TO the card's four above, deliberately.
            // User: "我發現我們HOME 速度好像沒用 你是不是設錯地方了?"
            //   The card's four are inert on a DS402 axis and the drive's four
            //   are the ones a home obeys. Publishing only the drive's would
            //   have been tidier and wrong: the operator has been looking at
            //   the card's numbers for two days, and a screen that simply
            //   replaces them cannot show WHY the old ones did nothing.
            //   ⚠ Same per-value gating. "Not read" and "zero" are different,
            //   and on 6099h:2 a zero is a zero-speed search for the origin.
            {
                static const char* kDriveHomeTag[
                        Pci1203AxisSample::kDriveHomeReadCount] = {
                    "velSwitch", "velZero", "acc", "offset"
                };
                for (int q = 0; q < Pci1203AxisSample::kDriveHomeReadCount; ++q) {
                    std::snprintf(tag, sizeof(tag), "pci1203.ax%d.dhome.%s",
                                  i, kDriveHomeTag[q]);
                    stageDouble(snap, tag, (a != 0 && a->driveHomeValid[q]),
                                a ? (double)a->driveHomeVal[q] : 0.0);
                }
                //  6098h, read-only: Acm_AxHome writes it from its mode
                //  argument, so the page shows what the drive is holding rather
                //  than offering a second place to set it.
                std::snprintf(tag, sizeof(tag), "pci1203.ax%d.dhome.method", i);
                stageDouble(snap, tag, (a != 0 && a->driveHomeMethodValid),
                            a ? (double)a->driveHomeMethod : 0.0);
            }

            //AI(W906-1203CTL-35) 20260911: the speed profile the CARD holds,
            // under the vendor's own property spellings so a value on screen
            // can be checked against the SDK header and against Common Motion
            // Utility's 運動參數設置 block side by side.
            //   ⚠ Gated per-property on its own read succeeding. "The card did
            //   not tell us this one" and "this one is zero" are different
            //   facts, and zero is the one that silently makes a jog do nothing.
            {
                //AI(W906-1203CTL-44) 20260911: +jerk, jerkFactor and the three
                // CFG_AxMax* ceilings. Same fixed order as kSpeedProp in the
                // monitor -- the two lists are read positionally and a mismatch
                // would publish every value under the wrong name.
                static const char* kSpeedTag[Pci1203AxisSample::kSpeedCount] = {
                    "velLow", "velHigh", "acc", "dec",
                    "jogVelLow", "jogVelHigh", "jogAcc", "jogDec",
                    "jerk", "jerkFactor",
                    "maxVel", "maxAcc", "maxDec"
                };
                for (int q = 0; q < Pci1203AxisSample::kSpeedCount; ++q) {
                    std::snprintf(tag, sizeof(tag), "pci1203.ax%d.%s", i, kSpeedTag[q]);
                    stageDouble(snap, tag, (a != 0 && a->speedValid[q]),
                                a ? a->speed[q] : 0.0);
                }
            }

            // The individual MotionIO bits an operator actually reads. Same
            // version-verified constants as above.
            //AI(W906-1203CTL-34) 20260911: 8 bits -> ALL 21. Common Motion
            // Utility's I/O 狀態 panel shows twenty-one lamps and this page
            // showed eight, so thirteen live signals the vendor tool displays
            // had no representation here at all -- including SLMT+/SLMT- (the
            // SOFT limits, which stop an axis for a completely different reason
            // than LMT+/LMT- and were indistinguishable by their absence).
            // Every one of them comes out of the SAME Acm_AxGetMotionIO word
            // that was already being read: no new vendor call, no extra poll
            // cost. The names match the Utility's lamp labels so the two
            // screens can be read side by side.
            const unsigned long io = (a ? a->motionIO : 0ul);
            struct { const char* suffix; unsigned long bit; } kBits[] = {
                { "rdy",     0x00000001ul },   // AX_MOTION_IO_RDY
                { "alm",     0x00000002ul },   // AX_MOTION_IO_ALM
                { "limitP",  0x00000004ul },   // AX_MOTION_IO_LMTP
                { "limitN",  0x00000008ul },   // AX_MOTION_IO_LMTN
                { "org",     0x00000010ul },   // AX_MOTION_IO_ORG
                { "dir",     0x00000020ul },   // AX_MOTION_IO_DIR
                { "emg",     0x00000040ul },   // AX_MOTION_IO_EMG
                { "pcs",     0x00000080ul },   // AX_MOTION_IO_PCS
                { "erc",     0x00000100ul },   // AX_MOTION_IO_ERC
                { "ez",      0x00000200ul },   // AX_MOTION_IO_EZ
                { "clr",     0x00000400ul },   // AX_MOTION_IO_CLR
                { "ltc",     0x00000800ul },   // AX_MOTION_IO_LTC
                { "sd",      0x00001000ul },   // AX_MOTION_IO_SD
                { "inp",     0x00002000ul },   // AX_MOTION_IO_INP
                { "svOn",    0x00004000ul },   // AX_MOTION_IO_SVON
                { "ralm",    0x00008000ul },   // AX_MOTION_IO_ALRM  (Utility: RALM)
                { "sLimitP", 0x00010000ul },   // AX_MOTION_IO_SLMTP
                { "sLimitN", 0x00020000ul },   // AX_MOTION_IO_SLMTN
                { "cmp",     0x00040000ul },   // AX_MOTION_IO_CMP
                { "camDo",   0x00080000ul },   // AX_MOTION_IO_CAMDO
                { "torLmt",  0x00100000ul },   // AX_MOTION_IO_MAXTORLMT
            };
            const int kBitCount = (int)(sizeof(kBits) / sizeof(kBits[0]));
            for (int b = 0; b < kBitCount; ++b) {
                std::snprintf(tag, sizeof(tag), "pci1203.ax%d.%s", i, kBits[b].suffix);
                stageBool(snap, tag, ok, (io & kBits[b].bit) != 0);
            }  PublishYield_();   // AI(W906-LAT-1) 20260925: yield point after each axis slot (each is ~110 stage() calls). The next slot re-reads its sample pointer, so an output served here is seen by every slot after it
        }

        // IO layer -- digital INPUT bytes.
        //
        //AI(W906-1203MON-18) 20260910: TWO THINGS ABOVE THIS LINE USED TO BE
        // TRUE AND ARE NOT ANY MORE. Both are corrected rather than deleted,
        // because a reader who remembers the old rule needs to find the
        // correction, not an absence.
        //
        //   (a) "there is no pci1203.do.* here and there must not be one" --
        //       there IS one now, and it is a READ. Acm_DaqDoGetByte reports
        //       what the outputs are currently driving. The forbidden family
        //       is Acm_DaqDoSet*, which is still absent and still caught by
        //       tools/pci1203_readonly_gate.ps1. Get vs Set is the line, not
        //       Di vs Do. Without the read-back a dark lamp means either "that
        //       coil is off" or "nobody looked", which lead to opposite repairs.
        //
        //   (b) "ring 0 / slave 0 only" -- the DI bytes are now read through
        //       the FLAT port image (Acm_DaqDiGetByte), which is how the vendor
        //       example enumerates a machine's inputs and how Common Motion
        //       Utility numbers its own DI page. The old per-station read was
        //       measured on 20260910 to be aimed at slave 0x001, an ECAT-2515
        //       junction with no IO at all. `pci1203.diN.flat` says which
        //       scheme produced the byte.
        for (int i = 0; i < kPci1203TagDiPorts; ++i) {
            char tag[48];
            const Pci1203DiSample* d = 0;
            if (mon != 0 && i < mon->diCount()) d = &mon->di(i);
            //AI(W906-SJSON-S9b) 20260923: **VALUE TAG DELETED.** The 320
            //  `pci1203.diN` byte-value tags moved to the packed pair
            //  `io.di` + `io.di.valid` (JsonBridge/ChanIo.cpp), staged into
            //  THIS SAME snapshot a few lines later in PublishHandlerTags
            //  (wb_serve.cpp:2421). The browser unpacks them back into these
            //  exact key names at its data entry point
            //  (web/js/pci1203.js unpackIoPlanes), so nothing downstream of
            //  the wire had to change -- view.js is untouched.
            //
            //  ⚠ Deleted in the SAME COMMIT as that unpacker, on the user's
            //    instruction ("改成新的方式, 兩邊同時修改!", 20260923).
            //    Shipping either half alone is the failure ChanIo.h's header
            //    warns about: the page does not error, it just goes quietly
            //    blank. If you are reading this because the DI table is empty,
            //    check that web/js/pci1203.js still has unpackIoPlanes().
            //
            //  ⚠ The ATTRIBUTE tags below (.ring/.addr/.flat/.station/.chan)
            //    are deliberately NOT part of this: they are the wiring
            //    diagram, not the measurement. They settle once at startup and
            //    then never patch again, so packing them would save nothing
            //    and cost the page its per-port attribution.
            (void)d;

            //AI(W906-1203MON-11) 20260907: WHERE the byte came from, published
            // beside it. The address used to be a literal in Poll() that never
            // reached the wire, so "DI port 2 = 0x0C" was unfalsifiable -- and
            // a real read of the WRONG station is indistinguishable from a
            // correct one. These are gated on the ADDRESS being known, not on
            // the read succeeding: knowing which station was tried is exactly
            // what you need when the read failed.
            const bool addrKnown = (d != 0 && d->ring >= 0);
            std::snprintf(tag, sizeof(tag), "pci1203.di%d.ring", i);
            stageInt(snap, tag, addrKnown, (long long)(d ? d->ring : 0));
            std::snprintf(tag, sizeof(tag), "pci1203.di%d.addr", i);
            stageInt(snap, tag, addrKnown, (long long)(d ? d->addr : 0));

            //AI(W906-1203MON-18) 20260910: WHICH addressing produced the byte.
            // Same reason ring/addr went on the wire in 1203MON-11: a byte read
            // through the wrong scheme is still a real byte, so the scheme has
            // to be visible next to the value. Under flat addressing ring/addr
            // are -1 and publish null -- which is correct, not missing data.
            std::snprintf(tag, sizeof(tag), "pci1203.di%d.flat", i);
            stageBool(snap, tag, d != 0, (d != 0 && d->flat));

            //AI(W906-1203MON-21) 20260910: WHICH STATION OWNS THIS BYTE, from
            // the master's own IO map. This is what lets a panel show only one
            // module's inputs instead of 21 anonymous bytes -- and it is the
            // same number as slaveN.alias, i.e. the one on the module's dials.
            //   ⚠ Gated on the attribution being KNOWN, not on the byte being
            //   readable: "I read this byte but do not know whose it is" is a
            //   real and different state from "I could not read it".
            std::snprintf(tag, sizeof(tag), "pci1203.di%d.station", i);
            stageInt(snap, tag, (d != 0 && d->station >= 0), (long long)(d ? d->station : 0));
            std::snprintf(tag, sizeof(tag), "pci1203.di%d.chan", i);
            stageInt(snap, tag, (d != 0 && d->stationChan >= 0), (long long)(d ? d->stationChan : 0));  if ((i & 31) == 31) PublishYield_();   // AI(W906-LAT-1) 20260925: yield point every 32 DI slots
        }

        // IO layer -- digital OUTPUT read-back. See (a) above: this is
        // Acm_DaqDoGetByte, an observation of what the card is driving.
        for (int i = 0; i < kPci1203TagDoPorts; ++i) {
            char tag[48];
            const Pci1203DoSample* o = 0;
            if (mon != 0 && i < mon->doCount()) o = &mon->do_(i);
            //AI(W906-SJSON-S9b) 20260923: **VALUE TAG DELETED** -- the 192
            //  `pci1203.doN` read-back values now travel as `io.do` +
            //  `io.do.valid`. Same reasoning, same commit, same unpacker as
            //  the DI side above; see that comment for the whole story.
            //  The .station/.chan/.ring attribute tags below stay.
            (void)o;
            //AI(W906-1203CTL-20) 20260911: which module owns this output byte,
            // from the card's own OUTPUT map. Same three-state rule as the DI
            // side -- station -1 publishes null, never 0, because "the map had
            // nothing for this port" and "station zero" are different facts.
            std::snprintf(tag, sizeof(tag), "pci1203.do%d.station", i);
            stageInt(snap, tag, (o != 0 && o->station >= 0),
                     (long long)(o ? o->station : 0));
            std::snprintf(tag, sizeof(tag), "pci1203.do%d.chan", i);
            stageInt(snap, tag, (o != 0 && o->stationChan >= 0),
                     (long long)(o ? o->stationChan : 0));
            //AI(W906-1203RING-1) 20260922: THE RING. Station numbers are unique
            //  only within a ring -- measured, station 0x001 is a SERVOPACK on
            //  ring 0 and an ECx-C32-HON 32DO on ring 1 -- so a page that groups
            //  output bytes by station alone puts two devices under one heading.
            //  ⚠ Same three-state rule: -1 publishes null, because "the map said
            //  nothing" and "ring 0" are different facts and the page decides
            //  what to draw from the difference.
            std::snprintf(tag, sizeof(tag), "pci1203.do%d.ring", i);
            stageInt(snap, tag, (o != 0 && o->ring >= 0),
                     (long long)(o ? o->ring : 0));  if ((i & 31) == 31) PublishYield_();   // AI(W906-LAT-1) 20260925: yield point every 32 DO slots
        }

        // --- discovered fieldbus stations -----------------------------------
        //AI(W906-1203MON-11) 20260907: the answer to "which ring and slave is
        // this machine actually using", measured off the card instead of taken
        // from deviceInfo.cfg -- which is MotionNet's map, not the 1203's, and
        // would have been a guess wearing the costume of a setting.
        stageInt (snap, "pci1203.scan.ms",        mon != 0, (long long)(mon ? mon->card().scanMs : 0));
        stageInt (snap, "pci1203.scan.rings",     mon != 0, (long long)(mon ? mon->card().scanRings : 0));
        stageInt (snap, "pci1203.scan.slots",     mon != 0, (long long)(mon ? mon->card().scanSlaves : 0));
        stageInt (snap, "pci1203.scan.found",     mon != 0, (long long)(mon ? mon->card().slavesFound : 0));
        // ⚠ NEVER hide this one. A truncated scan found FEWER stations than
        // exist, and reading the short list as the topology turns "we ran out
        // of time" into "that station is dead" -- the wrong repair entirely.
        stageBool(snap, "pci1203.scan.truncated", mon != 0, (mon != 0 && mon->card().scanTruncated));
        //AI(W906-1203COLD-2) 20260924: the two numbers that turn an empty 輸出 /
        //  輸入 list from "the page is broken" into a statement about the ring.
        //    master == 0 && found == 0   the ring really is empty
        //    master >  0 && found == 0   swept too early / swept wrong -- and
        //                                `retries` says how hard it has tried
        //  Published unconditionally alongside scan.found, because a reader who
        //  has found == 0 needs these in the SAME snapshot, not on the next one.
        stageInt (snap, "pci1203.scan.master",    mon != 0, (long long)(mon ? mon->card().masterSlaves : 0));
        stageInt (snap, "pci1203.scan.retries",   mon != 0, (long long)(mon ? mon->card().scanRetries : 0));
        stageInt (snap, "pci1203.poll.vendorCalls", mon != 0, (long long)(mon ? mon->card().vendorCalls : 0));
        { const bool seg = (mon != 0 && mon->card().pollSegValid); stageDouble(snap, "pci1203.poll.stationsMs", seg, seg ? mon->card().pollStationsMs : 0.0); stageDouble(snap, "pci1203.poll.axesMs", seg, seg ? mon->card().pollAxesMs : 0.0); stageDouble(snap, "pci1203.poll.diMs", seg, seg ? mon->card().pollDiMs : 0.0); stageDouble(snap, "pci1203.poll.doMs", seg, seg ? mon->card().pollDoMs : 0.0); }   // AI(W906-LAT-1) 20260925: where the last Poll's own time went, segment by segment (stations / axes / DI / DO, output time served inside it excluded; Pci1203CardSample::pollSegValid). Null until one full pass has run -- a detached or not-yet-polled card has no segments, and 0 would read as "free". Replaces the batchinput report's ~0.18 ms-per-call estimate with a measurement. On the old blank line, so no line below moves
        //AI(W906-1203MON-12) 20260907: the axis ID sweep. `axScan.byIdMode` is
        // the gate on trusting a motor NAME beside an axis row: false means the
        // numbers came from the physical-index fallback, which cannot be
        // matched to a Mot_Table row at all, so a name shown next to it would
        // be a confident mislabel of a real encoder reading.
        stageBool(snap, "pci1203.axScan.byIdMode",  mon != 0, (mon != 0 && mon->card().axByIdMode));
        stageInt (snap, "pci1203.axScan.byIdFound", mon != 0, (long long)(mon ? mon->card().axByIdFound : 0));
        stageInt (snap, "pci1203.axScan.tried",     mon != 0, (long long)(mon ? mon->card().axScanTried : 0));
        stageInt (snap, "pci1203.axScan.ms",        mon != 0, (long long)(mon ? mon->card().axScanMs : 0));
        stageBool(snap, "pci1203.axScan.truncated", mon != 0, (mon != 0 && mon->card().axScanTruncated));
        //AI(W906-1203CTL-32) 20260911: the SHAPE of the axis space, so a reader
        // can tell "this card has N motors" from "this module opened N handles".
        // ⚠ axesOpened > poolSize means a motor was opened twice under two
        // different station addresses -- the defect this pass removed, and the
        // one thing on this screen that cannot be spotted by looking at it.
        stageInt (snap, "pci1203.axScan.stations",  mon != 0, (long long)(mon ? mon->card().axStations : 0));
        stageInt (snap, "pci1203.axScan.poolSize",  mon != 0, (long long)(mon ? mon->card().axPoolSize : 0));
        //AI(W906-1203AXMAP-1) 20260915: the one-axis-per-slave check. The axis
        // labels are built on that assumption; these three tags are what let a
        // reader see whether it still holds, instead of trusting a mapping that
        // had been wrong for twelve of fourteen axes while looking plausible.
        stageInt (snap, "pci1203.axScan.cardAxes",   (mon != 0 && mon->card().axCountValid),
                  (long long)(mon ? mon->card().axCount : 0));
        stageInt (snap, "pci1203.axScan.ringSlaves", mon != 0,
                  (long long)(mon ? mon->card().axRingSlaves : 0));
        stageInt (snap, "pci1203.axScan.expected",   mon != 0,
                  (long long)(mon ? mon->card().axExpected : 0));
        stageBool(snap, "pci1203.axScan.mapConsistent", mon != 0,
                  (mon != 0 && mon->card().axMapConsistent));
        PublishYield_();   // AI(W906-LAT-1) 20260925: yield point before the station slots (and every 8 of them below -- each present station also costs one Pci1203Io.ini profile read). On the old blank line
        for (int i = 0; i < kPci1203TagSlaves; ++i) {
            char tag[48];
            const Pci1203SlaveSample* s = 0;
            if (mon != 0 && i < mon->slaveCount()) s = &mon->slave(i);
            const bool here = (s != 0 && s->present);

            std::snprintf(tag, sizeof(tag), "pci1203.slave%d.present", i);
            stageBool(snap, tag, mon != 0, here);
            std::snprintf(tag, sizeof(tag), "pci1203.slave%d.ring", i);
            stageInt(snap, tag, here, (long long)(s ? s->ring : 0));
            //AI(W906-1203POS-1) 20260916: ⚠ addr is published ONLY when an
            //  address actually reaches this station. A station whose address
            //  is claimed by its neighbour has addr = -1 in the sample, and
            //  publishing that as a number would put "-1" on screen beside a
            //  real model name -- or worse, tempt the page into offering a
            //  control keyed on it. It publishes as null, and `unaddressable`
            //  below is what says why.
            std::snprintf(tag, sizeof(tag), "pci1203.slave%d.addr", i);
            stageInt(snap, tag, here && s != 0 && s->addr >= 0,
                     (long long)(s ? s->addr : 0));
            //AI(W906-1203POS-1) 20260916: the master counts it, no address
            //  answers for it. See Pci1203SlaveSample::unaddressable for the
            //  position-by-position measurement and for why this is a row on
            //  the page rather than a silently shorter list.
            std::snprintf(tag, sizeof(tag), "pci1203.slave%d.unaddressable", i);
            stageBool(snap, tag, here, (s != 0 && s->unaddressable));
            //  ⚠ For an unaddressable station the cable POSITION is the only
            //  identity it has -- it is assigned by wiring, so it cannot
            //  collide the way the address did. The rail names the row by it.
            std::snprintf(tag, sizeof(tag), "pci1203.slave%d.position", i);
            stageInt(snap, tag, here && s != 0 && s->position >= 0,
                     (long long)(s ? s->position : 0));
            // stateValid, not `here`: a station that answered the SCAN and now
            // refuses is a dropped slave -- a real fault. The row must stay
            // visible showing the failure instead of the station vanishing.
            std::snprintf(tag, sizeof(tag), "pci1203.slave%d.state", i);
            stageInt(snap, tag, (s != 0 && s->stateValid), (long long)(s ? s->state : 0));

            //AI(W906-1203MON-20) 20260910: STATION IDENTITY.
            //
            //  ★ `alias` is the number set on the module's own rotary dials
            //  (ESC 0x0012, loaded from its EEPROM). It is what an operator
            //  standing next to the hardware can read off it, and on every IO
            //  module in this machine it DIFFERS from `addr`:
            //      pos 8  addr 9  alias 0x50 (80)   ECx-P32-HON 32DI
            //  Publishing only one of them under the label "station" would put
            //  the wrong number in front of that operator, and both look
            //  equally plausible on a screen. So all three travel together and
            //  the UI is expected to show all three.
            std::snprintf(tag, sizeof(tag), "pci1203.slave%d.alias", i);
            stageInt(snap, tag, (s != 0 && s->aliasValid), (long long)(s ? s->alias : 0));
            std::snprintf(tag, sizeof(tag), "pci1203.slave%d.pos", i);
            stageInt(snap, tag, (s != 0 && s->position >= 0), (long long)(s ? s->position : 0));
            // ESC 0x0010 -- separate from `.addr`, which is the key the API
            // wants. Measured 20260910: a module addressable as SlaveIP 80 has
            // 0x0010 == 9. Two different numbers, two different tags.
            std::snprintf(tag, sizeof(tag), "pci1203.slave%d.escAddr", i);
            stageInt(snap, tag, (s != 0 && s->escAddrValid), (long long)(s ? s->escAddr : 0));
            std::snprintf(tag, sizeof(tag), "pci1203.slave%d.name", i);
            stageStr(snap, tag, (s != 0 && s->infoValid),
                     AnsiString(s ? s->name.c_str() : ""));

            //AI(W906-1203PTS-1) 20260918: HOW MANY OF THOSE OUTPUT BITS ARE
            //  ACTUALLY WIRED TO SOMETHING.
            //  User, pointing at 0x0B CTEU-MPL-007F11 drawn as seven bytes of
            //  clickable coils: "這顆模組 點為數量是不是錯誤了? ... 目前機構上
            //  每顆 只有20點輸出".
            //
            //  ⚠⚠ THE CARD CANNOT ANSWER THIS AND IT IS NOT A DEFECT IN THE
            //  CARD. Acm_DevUpLoadMapInfo(MapType 0) attributes SEVEN flat ports
            //  to station 0x00b (offsets 16..22, PortChanID 0..6, described as
            //  "DO0 ~ DO7 on 0x00b" ... "DO48 ~ DO55 on 0x00b") -- 56 bits. That
            //  is the module's PROCESS IMAGE, which is fixed by its ESI/PDO
            //  configuration, not by how many valves are plugged into it.
            //  Measured the same run: ByteLength is 0 and ModuleName is
            //  "Unknown" on every single entry, so the card is not withholding a
            //  better number -- it does not have one.
            //  ⓘ Same family as the defect already recorded in view.js: a
            //  SERVOPACK's RxPDO bytes were offered as clickable coils, the
            //  write returned SUCCESS, and nothing moved. A wide process image
            //  is not a wide set of outputs.
            //
            //  So the count has to be DECLARED by whoever wired the machine.
            //  It lives in D:\HT9045\config\Pci1203Io.ini, keyed by the STATION
            //  the map attributes the bytes to:
            //      [station11]
            //      doPoints=20
            //  ⚠ Keyed by station, not by slave index: the slave index is a
            //  position in a scan order this campaign has watched change under
            //  it, and a point count filed under "slave 7" would be re-applied
            //  to whatever slave 7 becomes after a rewire. Same rule, and the
            //  same reason, as Pci1203Axis.ini's station+sub-axis key.
            //
            //  ⚠ ABSENT MEANS "DO NOT KNOW", NOT ZERO, and it publishes as null
            //  so the page keeps showing every bit. A missing entry must not
            //  hide outputs that do exist -- that failure is invisible, whereas
            //  showing too many is at least visible and is what prompted this.
            {
                //  ⓘ Read per publish rather than cached: this file is edited
                //  by hand while looking at the screen, and a cache would make
                //  the edit appear to do nothing until a restart -- which is
                //  the exact complaint this whole panel keeps earning.
                //  It is a local profile read, not a fieldbus transaction.
                long long pts = 0;
                bool have = false;
                if (s != 0 && s->addr >= 0) {
                    char sec[48];
                    std::snprintf(sec, sizeof(sec), "station%d", s->addr);
                    //  ⓘ memset, not std::memset: this TU does not include
                    //  <cstring>, and the std:: form is the one that fails to
                    //  compile rather than the one that is more correct.
                    char buf[32];
                    memset(buf, 0, sizeof(buf));
                    ::GetPrivateProfileStringA(
                        sec, "doPoints", "", buf, (DWORD)sizeof(buf),
                        "D:\\HT9045\\config\\Pci1203Io.ini");
                    if (buf[0] != '\0') {
                        char* end = 0;
                        const long v = std::strtol(buf, &end, 10);
                        //  A key that is present but not a number is the same
                        //  as absent: neither is a count we may draw a panel
                        //  from, and inventing one would be worse than showing
                        //  everything.
                        if (end != buf && v > 0) { pts = (long long)v; have = true; }
                    }
                }
                std::snprintf(tag, sizeof(tag), "pci1203.slave%d.doPoints", i);
                stageInt(snap, tag, have, pts);
            }
            std::snprintf(tag, sizeof(tag), "pci1203.slave%d.vendorId", i);
            stageInt(snap, tag, (s != 0 && s->infoValid), (long long)(s ? s->vendorId : 0));
            std::snprintf(tag, sizeof(tag), "pci1203.slave%d.productId", i);
            stageInt(snap, tag, (s != 0 && s->infoValid), (long long)(s ? s->productId : 0));
            //AI(W906-1203RAIL-1) 20260915: the CiA profile from 1000h, so the
            // rail can tell a DRIVE from an IO card by what the module says it
            // is rather than by whether its name contains "SERVOPACK".
            //   ⚠ Gated on profileValid, not infoValid. They are different
            //   reads: ADV_SLAVE_INFO comes from the master's scan, 1000h comes
            //   from the module's own mailbox, and a module can answer one and
            //   not the other. Publishing an unread 0 would classify it as
            //   profile 0 -- a number that means nothing and would look like an
            //   answer.
            std::snprintf(tag, sizeof(tag), "pci1203.slave%d.profile", i);
            stageInt(snap, tag, (s != 0 && s->profileValid), (long long)(s ? s->profile : 0));  if ((i & 7) == 7) PublishYield_();   // AI(W906-LAT-1) 20260925: yield point every 8 station slots
        }  }   //AI(W906-STREAM-2B) 20260930: closes the pci1203 page gate opened after pci1203.phase
    }
    PublishYield_();   // AI(W906-LAT-1) 20260925: yield point before the extra publisher (SECS: 741 SV entries). On the old blank line
    // AI(W906-P4-SECS) 20260920: 額外發布者（目前只有 SECS 的 SV 目錄）。
    //
    //   ⚠ **一定要在 commitPublish() 之前**跑。第一版是從 PublishHandlerTags()
    //   外面、之後才呼叫，結果全部落在 commit 之後被下一輪 beginPublish 清掉 ——
    //   開站 printf 說「741 entries」看起來成功，但線上快照一個 secs.* 都沒有。
    //   抓到它的是 snap_dump 的線上快照，不是任何本地計數。
    //
    //   ⚠ 為什麼是函式指標而不是直接呼叫：`tests/test_wb_tags.cpp` 與
    //   `test_wb_simpump.cpp` **把本檔當來源自己編**，直接呼叫會讓它們
    //   undefined reference（gate p4 實測）。而且就算補上來源也不該那樣修 ——
    //   那兩支斷言的是**精確的快照筆數**，無條件多發 741 個 tag 會把它們
    //   的期望值一起打壞，等於為了新功能去改別人測試的答案。
    //   ⇒ 預設沒有登錄者，測試行為**完全不變**；只有 wb_serve 去登錄。
    if (g_extraPublisher != 0)
        (void)g_extraPublisher(snap);   // 回傳的筆數已計入 stagedTagCount()

    const std::size_t staged = snap.stagedTagCount();
    snap.commitPublish();
    return staged;
}

// ---------------------------------------------------------------------------
TagCoverage HandlerTagCoverage()
{
    TagCoverage c;

    const bool strs  = IniConfigStringsLoaded();
    const bool cust  = CustomerCodeLoaded();
    const bool lastS = LastSetLoaded();

    // 3 identity strings + 1 customer code + 4 LastSet scalars
    // + FW-1a's 33 LastSet-blob tags (startmode.value + 32 site cells)
    // + the dead set.
    //AI(W906-FW1) 20260817: the 33 count under lastS because coverage asks
    // about SOURCES -- the blob either loaded or it did not; startmode's
    // per-value range guard is a publish-time concern, not a liveness one.
    //AI(W906-FW-W2) 20260819: +1 = auth.level (always-live login mirror).
    //AI(W906-FW-W3) 20260819: +1 more = control.owner (same cust key).
    c.total = 3 + 1 + 2 + 4 + 33 + kUnloadedCount;   // AI(W906-TOWER) 20260924: tower.* 移出 kUnloadedTags 後是 process tag（跟 pump.* 一樣由 pump 推導），照設計不進覆蓋率分母（test_wb_simpump O1）
    c.live  = (strs ? 3u : 0u) + (cust ? (1u + 2u /*auth.level + control.owner*/) : 0u) + (lastS ? (4u + 33u) : 0u);

    //AI(W906-FW1b) 20260817: the 6 sort counters are gated per station
    // (LastSet blob AND Prod.iTrayType configured), so their live count is
    // genuinely dynamic -- counted station by station, same rule as the
    // publish path, never flattered to "6 if the blob loaded".
    {
        static const int kSortTypeIdx[6] = {eAuto1, eAuto2, eAuto3, eFix1, eFix2, eFix3};
        c.total += 6;
        for (int i = 0; i < 6; ++i) {
            if (lastS && Prod.iTrayType[kSortTypeIdx[i]] != tNotUse) c.live += 1;
        }
    }
    { extern void W906_SortCTCoverage(TagCoverage&, bool, bool); W906_SortCTCoverage(c, lastS, cust); }   //Steven 20260925 (Data.SortCT)：檔尾那批 sort.*（來源 LastSet／Prod／Gerneral.ini／TestIF_File，都是機台資料來源，照 bin.*／temp.* 的規則計入；站數量／良率逐站照 iTrayType 算 live）。佔用原本的空行，不移動本檔其後的行號
    //AI(W906-FW1c) 20260819: +2 lastS blob tags (sort.loading, sort.total)
    // and +2 cust-keyed strings (lot.loaderLastBundleId/loadercarBundleId).
    // The 3 lot.auto*.trayCount tags are DELIBERATELY NOT counted, same rule
    // as the pump.* block below: they gate on `pumping`, a run this process
    // started, not a machine data source it loaded -- counting them would
    // flatter `live` without a new source being read.
    c.total += 2 + 2;
    c.live  += (lastS ? 2u : 0u) + (cust ? 2u : 0u);

    //AI(W906-FW1e) 20260820: +2 cust-keyed identity mirrors -- user.level
    // (FW-1d, whose commit missed this ledger line; corrected here) and
    // recipe.current (this wave).
    c.total += 2;
    c.live  += (cust ? 2u : 0u);

    //AI(W906-FW-BIN1) 20260820: +6 bin.* recipe assignments, keyed on the
    // host's bin-chain-loaded marker -- a real machine data source (the
    // recipe's Binasgn table), unlike pump telemetry, so it IS counted.
    c.total += 6;
    c.live  += (g_webBinSelLoaded ? 6u : 0u);

    //AI(W906-FW-TEMP2) 20260820: +3 temp.sv/soak/mode, keyed on the host's
    // temp-chain-loaded marker -- a real machine data source (the recipe's
    // Temperature.Data), same accounting rule as bin.* above.
    c.total += 3;
    c.live  += (g_webTempLoaded ? 3u : 0u);

    //AI(W906-FW-1Tb) 20260915: +1 light.off.  A real machine data source
    // (SW[SwCCDLight].OutValue), so it counts -- same rule as bin.*/temp.* above.
    // Liveness is the switch's own Enable, i.e. "is this switch configured on
    // this machine", which is what decides whether the value means anything.
    c.total += 1;
    c.live  += (SW[SwCCDLight].Enable ? 1u : 0u);
    { extern void W906_LotInfoCoverage(TagCoverage&, bool); W906_LotInfoCoverage(c, cust); }   //Steven 20260925 (Data.LotInfo)：+5 lot.*（來源 config.ini [Lot Info] 經 golden 的 SetLotID／ReadWriteLotInfo 進 fLotInfo 表單，是機台資料來源，照 bin.*／temp.* 的規則計入）；本體在檔尾。佔用原本的空行，不移動本檔其後的行號
    //AI(W906-SimPump) 20260813: the 18 clock/state/pump tags are DELIBERATELY NOT
    // counted here, and the reason is the same one this file exists for.
    //
    // TagCoverage answers exactly one question: "how many MACHINE DATA SOURCES are
    // actually loaded?". clock.text is the publisher's own wall clock and pump.* is
    // the publisher's own telemetry -- neither is a reading taken from the machine.
    // machine.state is excluded too, because under --pump it reports a fixture this
    // process FORCED rather than a condition it sensed (see WebBridgeTags.h).
    //
    // Counting any of them would push `live` up without a single new machine source
    // having been read -- precisely the flattering denominator this file's own rule
    // is meant to prevent, and it would also have broken the invariant asserted at
    // tests/test_wb_tags.cpp:75 ("0 live before loading"), which is a genuine
    // property worth keeping rather than an assertion to update.
    //
    // The true count of tags on the wire is PublishHandlerTags()'s return value
    // (snap.stagedTagCount()).  MEASURED 20260917: the snapshot is 4,608 tags =
    // 4,481 pci1203.* + 10 build.* + 117 rest.  Do not hand-edit this number.
    // Coverage and wire-count are different questions; this struct answers the first.
    { extern void W906_TestCategoryCoverage(TagCoverage&, bool); W906_TestCategoryCoverage(c, cust); }   //Steven 20260925 (Data.TestCategory)：檔尾那批 tcat.*（來源 TfTestCategory 的 TestResult／ColorPtr ＋ config.ini [Visible] ＋ TestIF，都是機台資料來源，照 bin.*／temp.* 的規則計入）。佔用原本的空行，不移動本檔其後的行號
    //AI(W906-FW-TEMP2) 20260820: TemperatureLoaded() is EXPECTED to agree
    // with g_webTempLoaded now that ReadTempFile is wired -- that is no
    // longer a "dead source came alive" surprise, it is this wave's own
    // intended effect. The genuinely interesting case is the mismatch: raw
    // Temperature bytes set while g_webTempLoaded is false means something
    // OTHER than wb_serve's own chain wrote to the struct, and THIS file's
    // inventory (which still only credits the one chain it knows about) has
    // gone stale again.
    //
    // TemperaturePvLoaded()/CosFunctionLoaded() keep the original rot check:
    // both are still expected to be false, and reaching here for either one
    // still means a source WebBridgeTags.h calls dead has come alive.
    if ((TemperatureLoaded() && !g_webTempLoaded) ||
        TemperaturePvLoaded() || CosFunctionLoaded()) {
        // Deliberately empty: reaching here means a source listed as dead in
        // WebBridgeTags.h has come alive, and that header's inventory -- plus
        // the kUnloadedTags table -- needs updating.
    }

    return c;
}

}  // namespace ht9045
extern bool SECS_GEM_PPSIGNALTOWER_CONTROL_flag;   //AI(W906-D1-PPCTL) 20261001 (St02, claim): ckernel.cpp:1275 (defined since 20260924); file scope on purpose -- inside namespace ht9045 a block-scope extern would name ht9045::SECS_GEM_PPSIGNALTOWER_CONTROL_flag
// =============================================================================
//  AI(W906-P1) 20260923: 使用者裁決 P1＝甲 —— 翻 golden TfMain::Timer1Timer 的 FlushFlag 翻轉段。
//  由上面 PumpTick() 在 GetTimeInfo() 之後每拍呼叫（PumpTick 是本行程唯一的週期入口，同 GetTimeInfo 的理由）。
//  本體放在檔尾而不是 PumpTick 裡：本檔有 200 多處外部引用指向 600 行以後的行號，插在中間會讓它們全部位移。
//
//  FlushFlag 是全機「閃爍同步」旗標（面板燈 bLamp*、塔燈 ledRed/Green/Yellow 都讀它：ckernel.cpp:240/:309/:354/:1738 起），
//  但移植樹在這之前**零生產寫入者** —— 解閘 ShowRunLabel／ShowRunLed（MSTATE-P2）後畫面會定格。
//  golden 用計數器 ct 算「每 250 ms 翻一次」（main.cpp:2943 / :3182 `ct > 250/Timer1->Interval`），與 timer 週期無關；
//  這裡照裁決用**掛鐘**（差值 >= 250 ms），兩種 tick 模型下語意都對 —— 今天 tick 是 500 ms，所以等於每拍都翻；
//  P10 若改回 golden 的執行模型，這段不用改就回到 250 ms。
// =============================================================================
namespace ht9045 {
void W906_FlushFlagTick()
{
    static std::chrono::steady_clock::time_point s_lastFlush;
    const std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
    if (bSECSGEMAlarm && bSECSGEM_NoteAlarm == false && SECS_GEM_PPSIGNALTOWER_CONTROL_flag == false)   // golden main.cpp:2938 / :2940   //AI(W906-D1-PPCTL) 20261001 (St02, claim): tower flag on -> fall through to the normal 250 ms flip below (golden :2941-2947 flips then returns; nothing else follows here), keeping the 6-phase compensation ShowRunLed needs to see an edge (ckernel.cpp:1738-1742)
    {
        // golden :2940-2947：SECS/GEM 警報時只在 SECS_GEM_PPSIGNALTOWER_CONTROL_flag 開著才翻，然後整個 timer return。
        // （原本這兩行說旗標只有 extern、寫者沒連進 wb_serve —— 20260924 W906-MSTATE-P2b 起 ckernel.cpp:1275 有定義；寫者 G17 20261001 解閘，ht9045_secsgem 在 wb_serve 的 RESCAN 群組）
        // 旗標關著：照 golden 不翻、直接 return（下一行）；旗標開著：上一行條件不成立，往下走一般的 250 ms 翻轉（AI(W906-D1-PPCTL) 20261001）。
        return;
    }
    //AI(W906-MSTATE-P2) 20260924: 慢節拍的取樣相位補償 —— 不是 golden，是 B13（500 ms tick）逼出來的。
    //  ShowRunLabel（golden ckernel.cpp:944-950）只在 FlushFlag「跟上次呼叫不同」且「為 true」時重畫；
    //  它由 DoSystemMessage 每 6 拍叫一次（golden :1903-1914 的 6 相輪轉，tests W7_L2 PART I6 釘著）。
    //  golden 的 MainProc 約 1 ms 一次（wb_serve.cpp kServeTickMs 的註解），取樣遠快於 250 ms 的翻轉，
    //  所以每個上升緣都會被看到；這裡是 500 ms 一拍、每拍翻一次，6 拍後必然回到同一個值 ——
    //  20260924 實測（scratchpad p2_state_probe.py）：machine.state 在 START 後畫出 "Running" 就定格，
    //  歸零、PAUSE（SystemStart 1->0）都沒再變。計畫書 §3 第 1 條警告過這個「動一下然後定格」的症狀。
    //  補償：只在慢節拍（本次與上次呼叫相隔 >= 250 ms）時，每 6 次翻轉機會略過 1 次 ⇒ 任意相隔 6 拍的兩個
    //  取樣點之間恰好翻 5 次（奇數）⇒ ShowRunLabel 每兩次取樣重畫一次（約 6 秒）。
    //  代價：讀 FlushFlag 閃燈的 9 個 csystem 消費者（例 SW[SwManualZ1].OnOff(FlushFlag)）每 3 秒多停一拍。
    //  快節拍（日後改回 golden 執行模型）時 slowTick 恆 false，本段不作用，回到純 250 ms 翻轉。
    static std::chrono::steady_clock::time_point s_lastCall;
    static unsigned s_slowToggleCount = 0;  static unsigned s_w906FlushCalls = 0;   // AI(W906-FLUSH-100) 20261007: + the beat counter of the next lines
    const bool slowTick = (now - s_lastCall >= std::chrono::milliseconds(250));  (void)slowTick;
    s_lastCall = now;
    if ((++s_w906FlushCalls % 2u) == 0u)                                        // AI(W906-FLUSH-100) 20261007: EastSun「回完home 畫面怎還是都一樣?」-- was `now - s_lastFlush >= 250 ms` (wall clock). Since the beat went 500 -> 100 ms (ced388a, 10-05) that flipped every 3rd beat = 2 flips per 6-beat DoSystemMessage window (even), so ShowRunLabel / ShowRunLed (ckernel.cpp:1949 / :1738, golden edge check) never saw a change: machine.state stayed "Homing" after HOME, and the tower / panel lamps they drive froze too. Now golden's own model -- a beat COUNT like golden's ct (main.cpp:3182) -- flipping every 2nd beat: 3 flips per 6-beat window (odd) at any beat, so every ShowRunLabel call sees an edge (100 ms beat: 200 ms flip, golden 250)
    {
        if (false && slowTick && (++s_slowToggleCount % 6u) == 0u)              // AI(W906-FLUSH-100) 20261007: the 500 ms skip-1-in-6 compensation is off -- with the 2-beat count above every window already has an odd number of flips (a skip would make it even again).  見上：6 = DoSystemMessage 的相數
        {
            s_lastFlush = now;                                                  // 略過這一次，維持原值
            return;
        }
        FlushFlag = !FlushFlag;                                                 // golden :3184  for system all flush 同步
        // golden :3185-3186 的 ProcessKeyFlush()／UpdateMotorHomeLed() 移植樹沒有（20260923 git grep 0 筆）——
        // 只翻旗標，不發明替代品（裁決原文）。
        s_lastFlush = now;
    }
}
}  // namespace ht9045

// =============================================================================
//  Steven 20260925 (Data.LotInfo)：lot.* —— golden uLotInfo「Lot」分頁（tsLotID）的五個欄位。
//
//  golden 畫面顯示的是 TfLotInfo 自己的表單文字（V912 uLotInfo.dfm）：
//    lot.id         edtSysLotID->Text        dfm:460   「Lot ID :」
//    lot.operator   edtSysOperatorID->Text   dfm:517   「Operator ID :」（Panel28）
//    lot.runMode    cbRunMode->Text          dfm:529   「Run Mode :」（Panel28）
//    lot.startTime  lbledtStarTime->Text     dfm:976   「Start Time」（pgcLotInfo 的子分頁 tsChipAdv:973；
//                                                      一般客戶改名「Lot Info」顯示，uLotInfo.cpp:1281-1296）
//    lot.device     lbledtDeviceName->Text   dfm:1071  「Device Name」（同上 tsChipAdv）
//  所以 tag 直接讀移植樹同名的 fLotInfo 元件（forms/fLotInfo.h），不另外去讀 config.ini：
//  元件文字就是 golden 畫面上的值；config.ini [Lot Info] 是它的持久化，兩者由 golden 自己的
//  ReadWriteLotInfo／SetLotID 同步（uLotInfo.cpp:1446 / :1534；移植樹 forms/fLotInfo.cpp:5205 / :5338）。
//  ⓘ 不選 edDeviceName：它在另一個分頁 tsDeviceInfo（dfm:99/186，只在 IniConfig.bEnableRms 時顯示，
//    uLotInfo.cpp:374），是 FTP／RMS 下載配方用的裝置名，不是 Lot 分頁的欄位。
//
//  誰寫這些元件（移植樹，20260925 實測）：
//    edtSysLotID／edtSysOperatorID  wb_serve 的 lot.start 設 Text → SetLotStart(write) → SetLotID(write)
//                                   → ReadWriteLotInfo(false) 寫 config.ini [Lot Info]
//    lbledtStarTime                 SetLotStart(write) 空白時補 yyyymmdd_hhnnss（uLotInfo.cpp:1630-1634）
//    cbRunMode／lbledtDeviceName    只有 ReadWriteLotInfo(true) 會讀進來；lot.start 不設（網頁沒有輸入框）
//
//  ⚠ 開機讀取不存在：golden 開機 TfMain::DoShowUserDefFrom 強制 fLotInfo->Show()（main.cpp:9238）
//    → FormShow → SetLotStart("fLotInfo::FormShow", true)（uLotInfo.cpp:1199）把 config.ini 讀進元件；
//    移植樹那一行是 GATE WC-19（forms/fLotInfo.cpp:4043-4045 #if 0），wb_serve 也不呼叫 FormShow。
//    SetLotStart(true) 不只是讀：config.ini 有 Lot No 時會 SetLotComponents(false) ⇒ RunInfo.bLotStart=true
//    （uLotInfo.cpp:1748-1751）—— 那是 START 的前置閘，不是這個顯示層可以順手打開的東西。
//
//  liveness：cust（組態已載入；保住 tests/test_wb_tags.cpp 第 1 段「載入前全部 null」）
//            && fLotInfo 已建構 && RunInfo.LotNo 非空。
//    RunInfo.LotNo 只有 SetLotID 寫（uLotInfo.cpp:1607-1610；移植樹 forms/fLotInfo.cpp:5413／:5415 是建構子初值
//    cprod.cpp:247 以外全樹唯二寫者，20260925 git grep 實測），讀檔模式一律非空（Lot ID 空時補 "[yyyymmdd.hhnnss]"），寫入模式等於新的 Lot ID
//    ⇒「非空」＝這個行程裡已經有人照 golden 的路建立過工單身分。
//    在那之前元件是建構時的空字串，**不是** config.ini 裡的值（上面那條開機讀取不存在）；
//    把它當 "" 送出去，畫面會說「沒有工單」，而 config.ini 可能正掛著一張 —— 所以送 null（"---"）。
//    golden SetLotEnd 會 SetLotID("")（uLotInfo.cpp:2345）⇒ LotNo="" ⇒ 這裡回到 null；
//    那時 golden 畫面是空白、網頁是 "---"，都是「沒有工單」（移植樹目前沒有 SetLotEnd）。
// =============================================================================
#include "forms/fLotInfo.h"   //Steven 20260925 (Data.LotInfo)：fLotInfo（forms/fLotInfo.cpp:4434 eager new）
namespace ht9045 {

static bool W906_LotInfoKnown(bool cust)
{
    return cust && fLotInfo != 0 && RunInfo.LotNo.Length() > 0;
}

static const std::size_t kLotInfoTagCount = 5;   std::size_t W906_StageLotInfoTabTags(webbridge::TagSnapshot&, bool); void W906_LotInfoTabCoverage(TagCoverage&, bool);   //Steven 20260925 (Data.LotInfo 其餘分頁)：本體在檔尾；宣告放在同一行，不移動本檔行號

std::size_t W906_StageLotInfoTags(webbridge::TagSnapshot& snap, bool cust)
{
    const bool live = W906_LotInfoKnown(cust);
    stageStr(snap, "lot.id",        live, live ? fLotInfo->edtSysLotID->Text      : AnsiString());
    stageStr(snap, "lot.operator",  live, live ? fLotInfo->edtSysOperatorID->Text : AnsiString());
    stageStr(snap, "lot.runMode",   live, live ? fLotInfo->cbRunMode->Text        : AnsiString());
    stageStr(snap, "lot.startTime", live, live ? fLotInfo->lbledtStarTime->Text   : AnsiString());
    stageStr(snap, "lot.device",    live, live ? fLotInfo->lbledtDeviceName->Text : AnsiString());
    return kLotInfoTagCount + W906_StageLotInfoTabTags(snap, cust);   //Steven 20260925 (Data.LotInfo 其餘分頁)：lot.tab.*／lot.atc.*／lot.barcode.*／lot.testerLog.*＋Lot Info 子分頁 12 格（檔尾）；同一行，不移動行號
}

void W906_LotInfoCoverage(TagCoverage& c, bool cust)
{
    c.total += kLotInfoTagCount;
    c.live  += W906_LotInfoKnown(cust) ? kLotInfoTagCount : 0u;   W906_LotInfoTabCoverage(c, cust);   //Steven 20260925 (Data.LotInfo 其餘分頁)：同一行，不移動行號
}
}  // namespace ht9045

// =============================================================================
//  Steven 20260925 (Data.SortCT)：sort.* —— golden TfSortCT（V912 cSortCT.dfm／cSortCT.cpp）三個分頁顯示的值。
//
//  來源（直接讀 golden 顯示的那個變數；golden 行號一律 V912 cSortCT.cpp）：
//    站數量      ShowSortIC :399  myCountPanel[i].pnlCount->Caption = LastSet.BinCT[0][iTo3Unload[i]]
//    站良率      :397-398         RunInfo.sT6AutoYield[i] = ChangeToPercentage(BinCT[0][iTo3Unload[i]], Sum)
//    總數        :420             pnlTotal->Caption = RunInfo.iUnloadCount = Sum（:352-357，eTrayCount 中 iTrayType!=tNotUse）
//    總良率      :430-437         pnlYield->Caption = FormatFloat("0.00%", (ArmData[0]+ArmData[1]).GetByBinLowYieldPCA/2)，
//                                 只在 TestIF_File.bLowYieldAlarmByBin；可見度＝同一個旗標（golden uYieldMonitoring.cpp:1726）
//    ART 分頁    :362-363 Sum_ART（ART 計數條件 :358-360 成立才累加）、:401-403 站數量／良率、:421 總數、:436 總良率；
//                ShowLoadingIC_ART :284 pnlLoadingART = LastSet.SendCT_ART[0]
//    列可見度    UpForm :718-790 的 myCountPanel[i].bVisible —— 每拍呼叫一次 fSortCT->UpForm()（只寫元件版面，不碰機台狀態）；
//                ART 分頁的列與一般列共用同一個 bVisible（_MyCountPanel::SetVisible :78-87 一次設六個元件）
//    分頁        FormShow :196-207 —— fSortCT->W906_SetTabVisibility()（ART 分頁可見度＋標題、IC Count 分頁可見度）
//    IC Count    pnlLoad／pnlHP1／pnlHP2 的 Caption —— SM 端（acatchtray／ainarm／cinitial／csystem）直接寫這三個元件，golden 同
//
//  為什麼站數量／良率不讀 fSortCT 的 Caption：golden 的 Caption 只在 ShowSortIC 被呼叫時更新（FormShow、Clarn_Data、asortarm
//    每放一顆……）。移植樹 wb_serve 開機不跑 FormShow（golden 開機會跑，main.cpp:9162-9163；tools/wb_serve.cpp 不在這一波的
//    寫入範圍）、csystem.cpp 的兩個呼叫點仍被 W7C2 巨集擋著（forms/fSortCT.h 更正段）—— Caption 可能還停在 dfm 的 "100"。
//    LastSet 是 golden 每個呼叫點都會重算出來的那個值（golden 改完 BinCT 一定接著 ShowSortIC），直接讀它與 golden 穩態畫面一致。
//
//  liveness：
//    站數量／良率（一般＋ART）  lastS && Prod.iTrayType[i]!=tNotUse —— golden 只在這個條件下寫 Caption（:354/:395）；
//                               沒設定的站 golden 不動 Caption（停在 dfm 值），這裡送 null（"---"）。
//    總數、ART 總數／Loading    lastS（同既有 sort.loading）。
//    列／分頁可見度、IC Count   cust（Gerneral.ini／config.ini 已載入）。
//    總良率                     lastS && cust && TestIF_File.bLowYieldAlarmByBin（golden 只在這時寫 Caption）。
//
//  ⚠ golden 的怪處（送來源值，不複製畫面錯位）：建構子 :109 把 eFix12 的良率面板綁成 pnlFix11Yield（不是 pnlFix12Yield）
//    ⇒ golden 畫面上那一格被 UpForm 移到 Fix12 列、顯示 Fix12 的良率，Fix11 列沒有良率格。forms/fSortCT.cpp 照 golden 綁；
//    這裡 sort.fix11.yield／sort.fix12.yield 各送各站的 RunInfo.sT6AutoYield[i]（golden 算出來的來源值）。
//    Fix7-12 只在 AUTO_EMPTY_COLOR>=3 且 TrayForm.iFixTrayMode 時可見（本機 Gerneral.ini AUTO_EMPTY_COLOR=1）。
//  ⚠ 六個既有的 sort.{auto1..3,fix1..3}.count 由上面 FW-1b 迴圈送（同一格 BinCT[0][e3*]＝iTo3Unload[i]、同一個條件），這裡跳過。
//  ⚠ W906_SortCTInstall() 在這裡每拍呼叫，是 cSortCT.cpp 的**連結錨點**（見該檔檔頭）—— 拿掉這一行，wb_serve 仍會編過、
//    但 cSortCT.cpp.obj 可能不再被抽出，fSortCT 的四個方法悄悄退回 no-op。
// =============================================================================
#include "CosFunction.h"      //Steven 20260925 (Data.SortCT)：CosFunction.bUseARTSortCount（ART 計數條件）
#include "cSocket.h"          //Steven 20260925 (Data.SortCT)：ArmData[0..1]->GetByBinLowYieldPCA（總良率）
#include "forms/fSortCT.h"    //Steven 20260925 (Data.SortCT)：fSortCT（forms/fSortCT.cpp eager new）
bool W906_SortCTInstall();    //Steven 20260925 (Data.SortCT)：cSortCT.cpp（全域命名空間）
namespace ht9045 {

namespace {
// e6TrayName 順序（MachineType.h:1191-1226）＝ golden myCountPanel[] 的索引＝畫面由上而下的列序
const char* const kSortSlug[33] = {
    "auto1", "auto2", "auto3", "auto4", "auto5", "auto6",
    "fix1", "fix2", "fix3", "fix4", "fix5", "fix6", "fix7", "fix8", "fix9", "fix10", "fix11", "fix12",
    "bulkbox",
    "mag1", "mag2", "mag3", "mag4", "mag5", "mag6", "mag7", "mag8", "mag9", "mag10", "mag11", "mag12", "mag13", "mag14",
};
static_assert(sizeof(kSortSlug) / sizeof(kSortSlug[0]) == eTrayCount, "one slug per golden e6TrayName station");

// golden ShowSortIC :358-360 的 ART 計數條件（同 FormShow :188-191、ShowLoadingIC :220-223）
bool W906_SortArtCounting()
{
    return (USE_AUTO_RETEST==eartInstall &&
            (bAutoReTest_ART || IniConfig.bA10_AutoReTest)) ||
           CosFunction.bUseARTSortCount;
}

bool W906_IsFw1bStation(int i)
{
    return i == eAuto1 || i == eAuto2 || i == eAuto3 || i == eFix1 || i == eFix2 || i == eFix3;
}

// 固定的 12 個：sort.yield、sort.yield.visible、sort.art.loading、sort.art.total、sort.art.yield、sort.art.active、
//             sort.art.tabVisible、sort.art.tabCaption、sort.ic.tabVisible、sort.ic.load、sort.ic.hp1、sort.ic.hp2
const std::size_t kSortFixedTags = 12;
// 每站：.yield／.visible／art.count／art.yield（4 個）＋ .count（FW-1b 那六站除外）
const std::size_t kSortPerStation = 4;
}  // namespace

// golden ShowSortIC :352-364 的 Sum（art=false）／Sum_ART（art=true）。unsigned 照 golden :347。
long long W906_SortCTSum(bool art)
{
    unsigned int Sum = 0;
    const bool artOn = W906_SortArtCounting();
    for (int i = 0; i < eTrayCount; i++) {
        if (Prod.iTrayType[i] != tNotUse) {
            if (!art)       Sum += LastSet.BinCT[0][iTo3Unload[i]];
            else if (artOn) Sum += LastSet.BinCT_ART[0][iTo3Unload[i]];
        }
    }
    return (long long)Sum;
}

std::size_t W906_StageSortCTTags(webbridge::TagSnapshot& snap, bool lastS, bool cust)
{
    W906_SortCTInstall();                                                       // 連結錨點（見上）＋冪等安裝
    TfSortCT* f = fSortCT;
    if (f != 0 && cust) {
        f->UpForm();                                                            // golden :712-815 —— 只算列可見度／版面
        f->W906_SetTabVisibility();                                             // golden FormShow :196-207
    }
    const unsigned int Sum    = (unsigned int)W906_SortCTSum(false);
    const unsigned int SumArt = (unsigned int)W906_SortCTSum(true);

    std::size_t n = 0;
    char tag[48];
    for (int i = 0; i < eTrayCount; ++i) {
        const bool live = lastS && Prod.iTrayType[i] != tNotUse;
        const unsigned int cnt    = LastSet.BinCT[0][iTo3Unload[i]];
        const unsigned int cntArt = LastSet.BinCT_ART[0][iTo3Unload[i]];
        if (!W906_IsFw1bStation(i)) {
            std::snprintf(tag, sizeof(tag), "sort.%s.count", kSortSlug[i]);
            stageInt(snap, tag, live, cnt);                                     // golden :399
            ++n;
        }
        std::snprintf(tag, sizeof(tag), "sort.%s.yield", kSortSlug[i]);
        stageStr(snap, tag, live, live ? ChangeToPercentage(cnt, Sum) : AnsiString());          // golden :397-398
        std::snprintf(tag, sizeof(tag), "sort.%s.visible", kSortSlug[i]);
        stageBool(snap, tag, cust && f != 0, cust && f != 0 && f->myCountPanel[i].bVisible);   // golden UpForm
        std::snprintf(tag, sizeof(tag), "sort.art.%s.count", kSortSlug[i]);
        stageInt(snap, tag, live, cntArt);                                      // golden :403
        std::snprintf(tag, sizeof(tag), "sort.art.%s.yield", kSortSlug[i]);
        stageStr(snap, tag, live, live ? ChangeToPercentage(cntArt, SumArt) : AnsiString());   // golden :401-402
        n += kSortPerStation;
    }

    // 總良率（golden :430-437）
    const bool lowYield = cust && TestIF_File.bLowYieldAlarmByBin;
    const bool yLive = lastS && lowYield && ArmData[0] != 0 && ArmData[1] != 0;
    AnsiString y;
    if (yLive) {
        double Sum2 = ArmData[0]->GetByBinLowYieldPCA();
        Sum2 += ArmData[1]->GetByBinLowYieldPCA();
        y = FormatFloat("0.00%", Sum2 / 2.0);
    }
    stageStr (snap, "sort.yield",          yLive, y);                           // pnlYield     :435
    stageBool(snap, "sort.yield.visible",  cust, lowYield);                     // uYieldMonitoring.cpp:1726
    stageInt (snap, "sort.art.loading",    lastS, LastSet.SendCT_ART[0]);       // ShowLoadingIC_ART :284
    stageInt (snap, "sort.art.total",      lastS, SumArt);                      // :421 RunInfo.iUnloadCount_ART
    stageStr (snap, "sort.art.yield",      yLive, y);                           // pnlYieldART  :436
    stageBool(snap, "sort.art.active",     cust, W906_SortArtCounting());       // :358-360（false 時 Sum_ART 恆 0）
    const bool fLive = cust && f != 0;
    stageBool(snap, "sort.art.tabVisible", fLive, fLive && f->ARTSortCount->TabVisible);          // FormShow :196-203
    stageStr (snap, "sort.art.tabCaption", fLive, fLive ? f->ARTSortCount->Caption : AnsiString());  // dfm "ARTSortCount"／:203 "Auto Sort Count"
    stageBool(snap, "sort.ic.tabVisible",  fLive, fLive && f->tsICCount->TabVisible);             // FormShow :207
    stageStr (snap, "sort.ic.load",        fLive, fLive ? f->pnlLoad->Caption : AnsiString());    // SM：MOT[MMTrayY].Tray.HowManyIC()
    stageStr (snap, "sort.ic.hp1",         fLive, fLive ? f->pnlHP1->Caption  : AnsiString());    // SM：MOT[MMPlate1].Tray.HowManyIC()
    stageStr (snap, "sort.ic.hp2",         fLive, fLive ? f->pnlHP2->Caption  : AnsiString());    // SM：MOT[MMPlate2].Tray.HowManyIC()
    { extern std::size_t W906_StageShowBinSelectTags(webbridge::TagSnapshot&, bool, bool); n += W906_StageShowBinSelectTags(snap, lastS, cust); }   return n + kSortFixedTags;   // AI(W906-PROD-S114) 20260926（Steven 團隊）：Status.ShowBinSelect 其餘分頁的 tag（本體在檔尾）。golden ShowSortIC（V912 cSortCT.cpp:423-428）每次重畫 Sort Count 就同時更新 ShowBinSelect 的 Category Info 與 Index 四格，所以接在這裡；同一行，不移動本檔行號
}

void W906_SortCTCoverage(TagCoverage& c, bool lastS, bool cust)
{
    for (int i = 0; i < eTrayCount; ++i) {
        const bool live = lastS && Prod.iTrayType[i] != tNotUse;
        const std::size_t k = kSortPerStation - 1 + (W906_IsFw1bStation(i) ? 0u : 1u);   // 數量／良率類（visible 另計）
        c.total += k + 1;
        c.live  += (live ? k : 0u) + (cust ? 1u : 0u);
    }
    // 固定 12 個：yield、art.yield 看 lowYield；loading／total 看 lastS；其餘 8 個看 cust
    const bool lowYield = cust && TestIF_File.bLowYieldAlarmByBin;
    c.total += kSortFixedTags;
    c.live  += (lastS && lowYield ? 2u : 0u) + (lastS ? 2u : 0u) + (cust ? 8u : 0u);   { extern void W906_ShowBinSelectCoverage(TagCoverage&, bool, bool); W906_ShowBinSelectCoverage(c, lastS, cust); }   // AI(W906-PROD-S114) 20260926（Steven 團隊）：檔尾那批 bin.*／binsel.*（來源 LastSet／Prod／配方 Binasgn，都是機台資料來源，照 bin.*／temp.* 的規則計入）；同一行，不移動本檔行號
}

}  // namespace ht9045

// =============================================================================
//  Steven 20260925 (Data.TestCategory)：tcat.* —— golden TfTestCategory（V912 cTestCategory.cpp／.dfm）
//  畫在畫面上的兩張 TStringGrid，逐格送「畫出來的結果」，不送 TestResult／ColorPtr 原始陣列：
//  數字怎麼變成字（-1 → 空白、關 site → "X"、>=iTestBinCount → I20 Error Bin 字樣、Barcode → "H"）、
//  NN 模式的列對應、By Arm 與否，全部是 golden sgArm1DrawCell（:145-359）的邏輯，留在 C++，網頁只貼字與底色。
//
//  來源（golden 行號一律 V912 cTestCategory.cpp）：
//    tcat.<g>.r<R>.c<C>        文字   sgArm1DrawCell 的 MyDrawText(pCanvas, Rect, str, ...) 的 str
//    tcat.<g>.r<R>.c<C>.bg     底色   同一呼叫的 BrushColor（golden TColor 整數；系統色保留 0x80000000|n，
//                                     例 clBtnFace=0x8000000F、clWindow=0x80000005 —— 轉 CSS 是網頁的事）
//    tcat.<g>.r<R>.c<C>.bold   粗體   當下 pCanvas->Font->Style 是否含 fsBold（內容格沿用列標題設的粗體）
//                              golden 沒畫的格子＝VCL 預設繪製：文字 ""、固定列欄 clBtnFace、其餘 clWindow、不粗
//    tcat.<g>.rowCount／colCount       AdjFormData :50-141 設的 RowCount／ColCount（沒跑過＝dfm 的 3／3）
//    tcat.<g>.colWidths／rowHeights    同上的 ColWidths[]／RowHeights[]，逗號串（dfm DefaultColWidth=80、VCL 預設列高 24）
//    tcat.<g>.visible          sgArm1 恆 true；sgArm2 ＝ bCateByArm。理由：SetShowCateMode :511-525 把表單 Height 設成
//                              192（By Arm）／105；dfm 裡 sgArm1 是 alBottom Height=80、sgArm2 是 alClient 拿剩下的 ——
//                              105 扣掉 bsToolWindow 標題列與邊框後只剩約 80，sgArm2 剩不到一列高，畫面上看不到。
//    tcat.show                 fTestCategory->bShow（golden DoShowUserDefFrom main.cpp:9170-9178：
//                              IniConfig.bShowTestCate 才 Show）。false＝golden 這個視窗是關的；網頁仍照畫（值照樣是真的）。
//    tcat.cateByArm            bCateByArm（SetShowCateMode :513，IniConfig.iShowCateByArm!=0）
//    tcat.width／tcat.height   表單 Width（AdjFormData 269/309/349/429）／Height（105/192）；0＝沒設過 → null
//  <g> ＝ sgArm1（MyStringGD[0]、Tag 0、dfm 在下方）／sgArm2（MyStringGD[1]、Tag 1、dfm 在上方）。
//  R 0..2 × C 0..8 固定送（AdjFormData 的上限：RowCount<=3、ColCount<=9），超出目前 RowCount／ColCount 的格子送 null。
//
//  重畫時機：每拍呼叫一次兩張格子的 Refresh()（＝golden 視窗收到 WM_PAINT；golden 自己在每個測試循環
//    ShowTestCategory 也會 Refresh）。sgArm1DrawCell 是純函式（讀 TestResult／ColorPtr／IniConfig／LastSet／TestIF），
//    重畫只改擷取緩衝，不碰機台狀態。
//
//  liveness：cust && W906_TestCategoryBooted() —— 開機序列要先跑 golden main.cpp:9882 InitCateCell ＋ :10594
//    DoShowUserDefFrom（cTestCategory.cpp 的 W906_BootTestCategory()，由 wb_serve 呼叫）。沒跑之前 TestResult／
//    ColorPtr 是 C++ 零初值（畫出來會是黑底 "0"），不是 golden 的狀態（golden 開機就 InitCateCell：-1／clWhite），
//    所以全部送 null（網頁顯示 "---"）。
//  ⚠ W906_TestCategoryInstall() 在這裡每拍呼叫，是 cTestCategory.cpp 的連結錨點（同 W906_SortCTInstall）。
// =============================================================================
#include "forms/fTestCategory.h"   //Steven 20260925 (Data.TestCategory)：fTestCategory（forms/fTestCategory.cpp eager new）＋ W906_* 宣告
namespace ht9045 {

namespace {
const int kTcatRows = 3;                                 // golden AdjFormData: RowCount 2 or 3 (dfm 3)
const int kTcatCols = 9;                                 // golden AdjFormData: ColCount 2..9 (dfm 3)
const char* const kTcatGrid[2] = { "sgArm1", "sgArm2" }; // MyStringGD[0] / [1]
// 固定 4 個：tcat.show、tcat.cateByArm、tcat.width、tcat.height
const std::size_t kTcatFixedTags = 4;
// 每張格子：visible、rowCount、colCount、colWidths、rowHeights（5）＋ 3x9 格 × (文字, bg, bold)
const std::size_t kTcatPerGrid = 5 + (std::size_t)(kTcatRows * kTcatCols * 3);

bool W906_TcatLive(bool cust)
{
    return cust && fTestCategory != 0 && W906_TestCategoryBooted();
}

AnsiString W906_TcatJoin(TfTestCategoryGrid* g, int n, bool heights)
{
    AnsiString s;
    char buf[16];
    for (int i = 0; i < n; ++i) {
        std::snprintf(buf, sizeof(buf), i ? ",%d" : "%d", heights ? g->RowHeights[i] : g->ColWidths[i]);
        s += buf;
    }
    return s;
}
}  // namespace

std::size_t W906_StageTestCategoryTags(webbridge::TagSnapshot& snap, bool cust)
{
    W906_TestCategoryInstall();                                                 // 連結錨點（見上）＋冪等綁 OnDrawCell
    TfTestCategory* f = fTestCategory;
    const bool live = W906_TcatLive(cust);
    if (live) {
        f->sgArm1->Refresh();                                                   // golden WM_PAINT（兩張都重畫）
        f->sgArm2->Refresh();
    }
    stageBool(snap, "tcat.show",      live, live && f->bShow);   //AI(W906-FSHOW-B2) 20260929: 門面成員（頁面據此判斷 bShowTestCate=0）；問頁面表在頁面開著時恆真 ⇒ 不問
    stageBool(snap, "tcat.cateByArm", live, live && f->bCateByArm);
    stageInt (snap, "tcat.width",     live && f->Width  > 0, live ? f->Width  : 0);
    stageInt (snap, "tcat.height",    live && f->Height > 0, live ? f->Height : 0);

    char tag[64];
    for (int g = 0; g < 2; ++g) {
        TfTestCategoryGrid* grid = live ? f->MyStringGD[g] : 0;
        const bool gl  = grid != 0;
        const int rows = gl ? (int)grid->RowCount : 0;
        const int cols = gl ? (int)grid->ColCount : 0;
        std::snprintf(tag, sizeof(tag), "tcat.%s.visible", kTcatGrid[g]);
        stageBool(snap, tag, gl, gl && (g == 0 || f->bCateByArm));
        std::snprintf(tag, sizeof(tag), "tcat.%s.rowCount", kTcatGrid[g]);
        stageInt(snap, tag, gl, rows);
        std::snprintf(tag, sizeof(tag), "tcat.%s.colCount", kTcatGrid[g]);
        stageInt(snap, tag, gl, cols);
        std::snprintf(tag, sizeof(tag), "tcat.%s.colWidths", kTcatGrid[g]);
        stageStr(snap, tag, gl, gl ? W906_TcatJoin(grid, cols, false) : AnsiString());
        std::snprintf(tag, sizeof(tag), "tcat.%s.rowHeights", kTcatGrid[g]);
        stageStr(snap, tag, gl, gl ? W906_TcatJoin(grid, rows, true) : AnsiString());
        for (int r = 0; r < kTcatRows; ++r) {
            for (int c = 0; c < kTcatCols; ++c) {
                TfTestCategoryGrid::W906_CellPaint* p = (gl && r < rows && c < cols) ? grid->W906_Cell(c, r) : 0;
                const bool cl = p != 0;
                std::snprintf(tag, sizeof(tag), "tcat.%s.r%d.c%d", kTcatGrid[g], r, c);
                stageStr(snap, tag, cl, cl ? p->Text : AnsiString());
                std::snprintf(tag, sizeof(tag), "tcat.%s.r%d.c%d.bg", kTcatGrid[g], r, c);
                stageInt(snap, tag, cl, cl ? (long long)p->Brush : 0);
                std::snprintf(tag, sizeof(tag), "tcat.%s.r%d.c%d.bold", kTcatGrid[g], r, c);
                stageBool(snap, tag, cl, cl && p->Bold);
            }
        }
    }
    return kTcatFixedTags + 2 * kTcatPerGrid;
}

void W906_TestCategoryCoverage(TagCoverage& c, bool cust)
{
    c.total += kTcatFixedTags + 2 * kTcatPerGrid;
    if (!W906_TcatLive(cust))
        return;
    TfTestCategory* f = fTestCategory;
    c.live += 2u + (f->Width > 0 ? 1u : 0u) + (f->Height > 0 ? 1u : 0u);
    for (int g = 0; g < 2; ++g) {
        TfTestCategoryGrid* grid = f->MyStringGD[g];
        if (grid == 0) continue;
        int rows = (int)grid->RowCount, cols = (int)grid->ColCount;
        if (rows > kTcatRows) rows = kTcatRows;
        if (cols > kTcatCols) cols = kTcatCols;
        // 格子只有在上一次 Refresh 的緩衝裡才算 live（同 stage 的 W906_Cell 判斷）
        if (rows > grid->W906_PaintRows) rows = grid->W906_PaintRows;
        if (cols > grid->W906_PaintCols) cols = grid->W906_PaintCols;
        c.live += 5u + (std::size_t)(rows > 0 && cols > 0 ? rows * cols * 3 : 0);
    }
}

}  // namespace ht9045

// =============================================================================
//  Steven 20260925 (Data.LotInfo 其餘分頁)：golden V912 uLotInfo 其餘分頁的 tag。
//  由上面 W906_StageLotInfoTags 的 return 那一行呼叫（同一行，不移動本檔行號）；本體（golden 敘述的翻譯）在
//  forms/fLotInfo.cpp 檔尾「Data.LotInfo 其餘分頁」橫幅，這裡只每拍呼叫它們、再把元件狀態原樣送出。
//
//  tag（全部 golden V912 uLotInfo；null＝不可知，網頁顯示 "---"）
//    lot.tab.<dfm 名>             37 個 TTabSheet 的 TabVisible（pgLotinfo 27、pgcLotInfo 8、pgcYieldMonitor 2）
//    lot.tab.active               FormShow 開窗選中的分頁（dfm 名；W906_FormShowActivePage）
//    lot.tab.tsChipAdv.caption    SettsChipAdvVisible 改的標題（一般客戶 "Lot Info"，:1295）
//    lot.<欄位>                   Lot 分頁子分頁 tsChipAdv（"Lot Info"）的另外 12 格 —— liveness 與上面 lot.* 相同
//                                 （W906_LotInfoKnown：RunInfo.LotNo 非空之前，元件不是 golden 開機讀進來的值）
//    lot.atc.*                    ATC 分頁：標題、Working Temperature、ATC On/Off Line、ATC Power 燈、各面板與 CH 的可見度
//                                 （CH 溫度／Chiller SV／露點／ATC 配方檔名沒有來源 —— 本樹沒有 ATC 連線 —— 不送，網頁固定 "---"）
//    lot.barcode.r<R>.c<C>        BarCode 分頁 sgBarcode 的格子，R=1..6（Load/Pass/Fail/Rate/Retry/Duplicate）、C=1..5（1_A..2_B、Total）
//    lot.barcode.*                Check duplicate code by lot 面板、Change File 鈕與 2DID 顯示群組的可見度
//    lot.testerLog.*              Tester Log 分頁：labTCPIPStatus（Caption／Color）、mmTesterLog 行數與最後 50 行
//
//  liveness
//    cust（Gerneral.ini／config.ini 已載入）是全部的前提 —— 保住 tests/test_wb_tags.cpp 第 1 段「載入前全部 null」。
//    Temperature 相關（ATC On/Off Line、Power 燈、參考溫度可見度）另要 g_webTempLoaded；Working Temperature 另要 lastS
//    （LastSet.iTemperature）與「golden 那兩行真的寫過 Caption」（W906_bATCWorkTempShown）。
//    ATC Power 燈只有 golden 確定是熄的那一支才有值（W906_bATCPowerKnown），其餘看 ATC 連線 -> null。
//    Check duplicate code by lot：golden 只在讀配方時由 BarCode.cpp SetCheckCodeByLot 寫（移植樹 BarCode/BarCode.cpp:273-278），
//    沒寫過（Caption 還是空的）時 golden 顯示 dfm 字樣 —— 送 null，網頁保留 dfm 字樣。
//  ⚠ BarCode 格子開機時 golden 是空白（FormShow 只填表頭，DoBarcodeCount 第一次被叫才有數字）；這裡從第一拍就送計數器
//    （cmydef.cpp:6031 開機歸零，0 是真值）—— 與 SortCT 讀 LastSet 同一個理由：golden 每次改計數器都接著 DoBarcodeCount。
// =============================================================================
namespace ht9045 {

namespace {
struct W906_LiTab { const char* name; TTabSheet* TfLotInfo::* member; };
// dfm 順序（V912 uLotInfo.dfm:99-14525）＝ TPageControl 顯示順序；pgcLotInfo（:610）、pgcYieldMonitor（:9869）接在後面
const W906_LiTab kLiTabs[] = {
    { "tsDeviceInfo",         &TfLotInfo::tsDeviceInfo },          // dfm :99
    { "tsLotID",              &TfLotInfo::tsLotID },               // :285
    { "tsFTP",                &TfLotInfo::tsFTP },                 // :1927
    { "tsRTCFullViewImg",     &TfLotInfo::tsRTCFullViewImg },      // :2092
    { "tsATC",                &TfLotInfo::tsATC },                 // :2233
    { "ts_OCRInterface",      &TfLotInfo::ts_OCRInterface },       // :4043
    { "ts_SocketInterface",   &TfLotInfo::ts_SocketInterface },    // :4135
    { "tsSelection",          &TfLotInfo::tsSelection },           // :4299
    { "tsBarCode",            &TfLotInfo::tsBarCode },             // :4576
    { "ts_AutoCleanMonitor",  &TfLotInfo::ts_AutoCleanMonitor },   // :4709
    { "ts_AutoRetestMonitor", &TfLotInfo::ts_AutoRetestMonitor },  // :4871
    { "tsOCRBarCode",         &TfLotInfo::tsOCRBarCode },          // :5537
    { "tsESDMonitor",         &TfLotInfo::tsESDMonitor },          // :5788
    { "tsASEMARMS",           &TfLotInfo::tsASEMARMS },            // :7189
    { "tsASECLEventLog",      &TfLotInfo::tsASECLEventLog },       // :7228
    { "tsChamberBoost",       &TfLotInfo::tsChamberBoost },        // :9145
    { "ATC_WinWay",           &TfLotInfo::ATC_WinWay },            // :9413
    { "tsRFMD",               &TfLotInfo::tsRFMD },                // :9613
    { "ts_FTPAutomation",     &TfLotInfo::ts_FTPAutomation },      // :9647
    { "tsYieldMonitior",      &TfLotInfo::tsYieldMonitior },       // :9866
    { "tsTesterLog",          &TfLotInfo::tsTesterLog },           // :11215
    { "ts_ATC6_1",            &TfLotInfo::ts_ATC6_1 },             // :11311
    { "tsBundle",             &TfLotInfo::tsBundle },              // :13255
    { "tsSetupFileCheck",     &TfLotInfo::tsSetupFileCheck },      // :13617
    { "tsAMR",                &TfLotInfo::tsAMR },                 // :13647
    { "tsKYEC_AMR",           &TfLotInfo::tsKYEC_AMR },            // :14024
    { "tsOtherTool",          &TfLotInfo::tsOtherTool },           // :14525
    { "tsMurata",             &TfLotInfo::tsMurata },              // pgcLotInfo :619
    { "tsSigurd_CX",          &TfLotInfo::tsSigurd_CX },           // :687
    { "tsSPIL_SZ",            &TfLotInfo::tsSPIL_SZ },             // :747
    { "tsOEE",                &TfLotInfo::tsOEE },                 // :766
    { "ts2DSort",             &TfLotInfo::ts2DSort },              // :861
    { "tsChipAdv",            &TfLotInfo::tsChipAdv },             // :973
    { "tsVTest",              &TfLotInfo::tsVTest },               // :1267
    { "tsPATSetUp",           &TfLotInfo::tsPATSetUp },            // :1331
    { "tsSigurd",             &TfLotInfo::tsSigurd },              // pgcYieldMonitor :9879
    { "tsTPW",                &TfLotInfo::tsTPW },                 // :10707
};
const std::size_t kLiTabCount = sizeof(kLiTabs) / sizeof(kLiTabs[0]);
static_assert(sizeof(kLiTabs) / sizeof(kLiTabs[0]) == 37, "27 pgLotinfo + 8 pgcLotInfo + 2 pgcYieldMonitor TTabSheets (V912 uLotInfo.dfm)");

const std::size_t kLiTabExtra     = 2;    // lot.tab.active、lot.tab.tsChipAdv.caption
const std::size_t kLiChipAdvCount = 12;   // Lot Info 子分頁另外 12 格
const std::size_t kLiAtcFixed     = 16;   // caption、workTemp、online、online.color、power、8 個元件可見度、use4/use8/use32
const std::size_t kLiAtcPerHead   = 2;    // ch<N>.visible、ch<N>.ref.visible
const std::size_t kLiBarcodeCells = 30;   // 6 列 × 5 欄
const std::size_t kLiBarcodeFixed = 4;    // checkByLot、checkByLot.color、changeFile.visible、display.visible
const std::size_t kLiTesterLog    = 4;    // status、status.color、count、tail
const int         kLiTesterTail   = 50;   // lot.testerLog.tail 的行數（golden memo 最多約 2000 行，TesterTCP_Socket.cpp:387；整段送太大）

bool W906_LiLive(bool cust) { return cust && fLotInfo != 0; }
}  // namespace

std::size_t W906_StageLotInfoTabTags(webbridge::TagSnapshot& snap, bool cust)
{
    const bool live  = W906_LiLive(cust);
    const bool lastS = LastSetLoaded();
    const bool temp  = g_webTempLoaded;
    TfLotInfo* f = live ? fLotInfo : 0;
    if (f) {
        f->W906_RefreshTabVisible();          // golden FormShow／Timer2Timer 的 TabVisible（forms/fLotInfo.cpp 檔尾）
        f->W906_ShowATCThermoDisplay();       // ATC 分頁
        W906_DoBarcodeCount();                // sgBarcode（golden TfBarCode::DoBarcodeCount）
    }

    std::size_t n = 0;
    char tag[64];

    // -- 分頁可見度 ------------------------------------------------------------------------------------------------
    for (std::size_t i = 0; i < kLiTabCount; ++i) {
        std::snprintf(tag, sizeof(tag), "lot.tab.%s", kLiTabs[i].name);
        TTabSheet* t = f ? (f->*(kLiTabs[i].member)) : 0;
        stageBool(snap, tag, t != 0, t != 0 && t->TabVisible);
        ++n;
    }
    stageStr(snap, "lot.tab.active",            f != 0, f ? f->W906_FormShowActivePage() : AnsiString());
    stageStr(snap, "lot.tab.tsChipAdv.caption", f != 0, f ? f->tsChipAdv->Caption : AnsiString());
    n += kLiTabExtra;

    // -- Lot 分頁子分頁 tsChipAdv（"Lot Info"）另外 12 格（V912 uLotInfo.dfm:995-1247；ReadWriteLotInfo :1446-1532） --------
    const bool lk = W906_LotInfoKnown(cust);
    stageStr(snap, "lot.endTime",     lk, lk ? fLotInfo->lbledtEndTime->Text     : AnsiString());   // dfm :995  End Time
    stageStr(snap, "lot.testerOsVer", lk, lk ? fLotInfo->lbledtTesterOsVer->Text : AnsiString());   // :1014 Tester OS Ver
    stageStr(snap, "lot.customer",    lk, lk ? fLotInfo->lbledtCustomer->Text    : AnsiString());   // :1033 Customer
    stageStr(snap, "lot.testProg",    lk, lk ? fLotInfo->lbledtTestProg->Text    : AnsiString());   // :1052 Test Program
    stageStr(snap, "lot.testerId",    lk, lk ? fLotInfo->lbledtTesterID->Text    : AnsiString());   // :1090 Tester ID
    stageStr(snap, "lot.subLotNo",    lk, lk ? fLotInfo->lbledtSubLotNo->Text    : AnsiString());   // :1109 Sub Lot NO
    stageStr(snap, "lot.testCode",    lk, lk ? fLotInfo->lbledtTestCode->Text    : AnsiString());   // :1128 Test Code
    stageStr(snap, "lot.machineId",   lk, lk ? fLotInfo->lbledtMachineID->Text   : AnsiString());   // :1147 Machine ID（只有 ReadWriteLotInfo(true) :1486 寫 IniConfig.SocketHandlerID）
    stageStr(snap, "lot.testBinNo",   lk, lk ? fLotInfo->lbledtTestBinNo->Text   : AnsiString());   // :1168 Test Bin No
    stageStr(snap, "lot.modeCode",    lk, lk ? fLotInfo->lbledtModeCode->Text    : AnsiString());   // :1187 Mode Code
    stageStr(snap, "lot.stage",       lk, lk ? fLotInfo->edtStage->Text          : AnsiString());   // :1228 Stage
    stageStr(snap, "lot.step",        lk, lk ? fLotInfo->edtStep->Text           : AnsiString());   // :1247 Step
    n += kLiChipAdvCount;

    // -- ATC 分頁 -----------------------------------------------------------------------------------------------------
    const bool aT = f != 0 && temp;
    stageStr (snap, "lot.atc.caption",      f != 0, f ? f->palATC->Caption : AnsiString());                      // palATC（SetATCFormVisible）
    stageStr (snap, "lot.atc.workTemp",     aT && lastS && f->W906_bATCWorkTempShown,
                                            f ? f->palATCWorkingTemp->Caption : AnsiString());                   // ShowATCThermo :5605／:6604
    stageStr (snap, "lot.atc.online",       aT, f ? f->pl_ATC_Online->Caption : AnsiString());                   // FormShow :670-675（dfm "ATC On Line"）
    stageInt (snap, "lot.atc.online.color", aT, f ? f->pl_ATC_Online->Color : 0);                                // golden TColor（clRed 0x0000FF／clLime 0x00FF00）
    stageBool(snap, "lot.atc.power",        aT && f->W906_bATCPowerKnown, f != 0 && f->aldATCPower->Value);      // NetATCTimeTimer :8812-8818
    stageBool(snap, "lot.atc.chiller.visible",      f != 0, f && f->aldATCChillerStatus->Visible);
    stageBool(snap, "lot.atc.chillerLabel.visible", f != 0, f && f->lblChiller->Visible);
    stageBool(snap, "lot.atc.atc70.visible",        f != 0, f && f->aldATC7Status->Visible);
    stageBool(snap, "lot.atc.atc70Label.visible",   f != 0, f && f->lblATC70->Visible);
    stageBool(snap, "lot.atc.chillerSV.visible",    f != 0, f && f->pan_ATCChillerSV->Visible);
    stageBool(snap, "lot.atc.chillerSVValue.visible", f != 0, f && f->pl_ATCChillerSV->Visible);
    stageBool(snap, "lot.atc.recipeFile.visible",   f != 0, f && f->lblATC_Now_RecipeFile->Visible);
    stageBool(snap, "lot.atc.dewPoint.visible",     f != 0, f && f->pan_DewPoint->Visible);
    stageBool(snap, "lot.atc.use4.visible",         f != 0, f && f->Pan_ATC_Use_4Head->Visible);
    stageBool(snap, "lot.atc.use8.visible",         f != 0, f && f->Pan_ATC_Use_8Head->Visible);
    stageBool(snap, "lot.atc.use32.visible",        f != 0, f && f->Pan_ATC_Use_32Head->Visible);
    n += kLiAtcFixed;
    for (int i = 0; i < ATC_HEAD_COUNT; ++i) {
        std::snprintf(tag, sizeof(tag), "lot.atc.ch%d.visible", i + 1);
        stageBool(snap, tag, f != 0, f && f->ATCChPal[i]->Visible);                    // ShowATCTempPanel：i<iATC_Use_Heat_Count
        std::snprintf(tag, sizeof(tag), "lot.atc.ch%d.ref.visible", i + 1);
        stageBool(snap, tag, aT, f && f->ATCReferPtr[i]->Visible);                     // 另要 Temperature.bUseReferTempSensor
        n += kLiAtcPerHead;
    }

    // -- BarCode 分頁 -------------------------------------------------------------------------------------------------
    for (int r = 1; r <= 6; ++r) {
        for (int c = 1; c <= 5; ++c) {
            std::snprintf(tag, sizeof(tag), "lot.barcode.r%d.c%d", r, c);
            stageStr(snap, tag, f != 0, f ? f->sgBarcode->Cells[c][r] : AnsiString());
            ++n;
        }
    }
    const bool cl = f != 0 && f->lbCheckCodeByLot->Caption.Length() > 0;
    stageStr (snap, "lot.barcode.checkByLot",         cl, cl ? f->lbCheckCodeByLot->Caption : AnsiString());   // SetCheckCodeByLot（forms/fLotInfo.cpp:2398）
    stageInt (snap, "lot.barcode.checkByLot.color",   cl, cl ? f->lbCheckCodeByLot->Color : 0);
    stageBool(snap, "lot.barcode.changeFile.visible", f != 0, f && f->btChangeFile->Visible);                  // FormShow :572
    stageBool(snap, "lot.barcode.display.visible",    f != 0, f && f->grpBarcodeDisplayLotInfo->Visible);      // Timer2Timer :7244
    n += kLiBarcodeFixed;

    // -- Tester Log 分頁 ----------------------------------------------------------------------------------------------
    int lines = -1;
    const AnsiString tail = f ? f->W906_TesterLogTail(kLiTesterTail, &lines) : AnsiString();
    const bool tl = f != 0 && lines >= 0;
    stageStr(snap, "lot.testerLog.status",       f != 0, f ? f->labTCPIPStatus->Caption : AnsiString());       // dfm "OFF-LINE"；TesterTCP_Socket.cpp:208／:221／:236
    stageInt(snap, "lot.testerLog.status.color", f != 0, f ? f->labTCPIPStatus->Color : 0);
    stageInt(snap, "lot.testerLog.count",        tl, lines);                                                    // golden mmTesterLog->Lines->Count
    stageStr(snap, "lot.testerLog.tail",         tl, tail);
    n += kLiTesterLog;
    return n;
}

void W906_LotInfoTabCoverage(TagCoverage& c, bool cust)
{
    const std::size_t atc = kLiAtcFixed + kLiAtcPerHead * (std::size_t)ATC_HEAD_COUNT;
    c.total += kLiTabCount + kLiTabExtra + kLiChipAdvCount + atc + kLiBarcodeCells + kLiBarcodeFixed + kLiTesterLog;
    if (!W906_LiLive(cust))
        return;
    TfLotInfo* f = fLotInfo;
    const bool temp  = g_webTempLoaded;
    const bool lastS = LastSetLoaded();
    c.live += kLiTabCount + kLiTabExtra;
    c.live += W906_LotInfoKnown(cust) ? kLiChipAdvCount : 0u;
    // ATC：cust 就有的 12 個（caption、8 個元件可見度中的 11、3 個面板…）—— 逐一照 stage 的 liveness 算
    c.live += 1u;                                                               // caption
    c.live += (temp && lastS && f->W906_bATCWorkTempShown) ? 1u : 0u;           // workTemp
    c.live += temp ? 2u : 0u;                                                   // online、online.color
    c.live += (temp && f->W906_bATCPowerKnown) ? 1u : 0u;                       // power
    c.live += 11u;                                                              // 8 個元件可見度 ＋ use4／use8／use32
    c.live += (std::size_t)ATC_HEAD_COUNT * (temp ? 2u : 1u);                   // ch.visible（cust）／ch.ref.visible（temp）
    c.live += kLiBarcodeCells + 2u;                                             // 格子、changeFile／display 可見度
    c.live += (f->lbCheckCodeByLot->Caption.Length() > 0) ? 2u : 0u;
    int lines = -1;
    (void)f->W906_TesterLogTail(0, &lines);
    c.live += 2u + (lines >= 0 ? 2u : 0u);
}

}  // namespace ht9045

// =============================================================================
//  AI(W906-PROD-S114) 20260926（Steven 團隊）：Status.ShowBinSelect —— golden V912 TfShowBinSelect（cShowBinSelect.cpp／.dfm）
//  Test Bin 分頁補到 33 站，並發布 Category Info／Index 兩個分頁顯示的資料。由上面 W906_StageSortCTTags 的 return 那一行
//  呼叫（同一行，不移動本檔行號）：golden ShowSortIC（cSortCT.cpp:423-428）每次重畫 Sort Count 就同時呼叫
//  fShowBinSelect->ShowCategoryBin() 並寫 Index 四格，兩者本來就是同一個時機。
//
//  Test Bin（golden ShowBinSel :420-817；列可見度 SetAutoVisible :1557-1608）：
//    bin.<站>        FW-BIN1 已送的六站（bin.auto1..3／fix1..3，本檔 :984-993）以外 27 站的 MyBinSel[i]->Caption。
//                    站名用上面 Data.SortCT 段的 kSortSlug（e6TrayName 順序＝golden dfm laAuto1..laMag14 由上而下的 Top）。
//    bin.visible     33 個字元 '1'／'0'（同一順序）：MyBinSel[i]->Visible。每拍先跑一次 golden SetAutoVisible（golden 在
//                    FormShow :906 與 PageControl1Change :1613 每次都跑；只改元件可見度，iHWFix_BinBox 時把 Bulk Box 那一格寫成 "E"，
//                    不碰機台狀態）。Prod.iTrayType==tNotUse 的站 golden 隱藏（名稱欄 MyBinSelLab 同一個旗標）。
//    bin.color       33 個 TColor 整數（逗號串）：MyBinSel[i]->Font->Color（ShowBinSel :690-702：clGray，或 tcBinColor[Prod.iIsFailT6[i]]；
//                    tNotUse 的站 golden 不設，留著上一次的值）。
//                    ⚠ 移植樹的 tcBinColor[] 目前是全 0 的占位（cmydef.cpp:4179；golden 色表在 :4177 的 #if 0 裡），所以非灰色的站
//                      現在送 0（黑）—— 送的是移植樹的真狀態，這裡不補顏色；色表還原後自動正確（片段交 Jimmy）。
//    liveness：g_webBinSelLoaded（同 bin.auto1）；bin.visible 另要 cust（SetAutoVisible 讀 Gerneral.ini 的機構旗標）。
//
//  Category Info（golden ShowCategoryBin :1832-2199 的「顯示」那一半；列名 ShowInitialString :1765-1830）：
//    binsel.cat.counts  iTestBinCount+1 個整數（逗號串）：LastSet.iBinData32[0][0..iTestBinCount]；最後一個是 "Error Bin"（:1794）
//    binsel.cat.sum     golden 的分母 sum（:1862-1874）：CosFunction.bLowYieldUseContactCounts ? Σ iBinData32[0][0..iTestBinCount-1]
//                                                                       : Σ 33 站 LastSet.BinCT[0][iTo3Unload[i]]（⚠ golden 這裡不看 iTrayType）
//    binsel.cat.sort2d  LastSet.iTester==_2D_SORT —— golden 換另一種畫法（:1879-1925：只列 iByBinCnt>0 的 bin、多一欄 expect count、
//                       列名 "BIN n"）。那是 ATK 的 2DID 分揀分支（S25），網頁只標「這個分支沒接」，不猜。
//    百分比由網頁照 golden ChangeToPercentage（MachineType.h:1713；golden V912 同名 :1653）自己算：sum>0 ? "%0.2f%%" : "0.00%"。
//    ⚠ 不在 tick 裡呼叫 golden ShowCategoryBin：SystemStart 時它會 DoLowYieldAlarm（WAR07357／WAR07358）、
//      fYieldMonitoring->ClearYieldCount()、fCounterClear->ClearCount(ctBinCount)、InitialAutoCleanAllTask() —— 那是機台行為，不是顯示。
//    ⚠ golden 在 fContact->fShow==true（Contact 視窗開著）時整張格子不更新（:1860）；這裡照送目前值，不複製這個凍結（交件報告註明）。
//    liveness：lastS && cust && g_webBinSelLoaded && 0 < iTestBinCount < TEST_MAX_BIN（iTestBinCount 由配方 TestIF 讀進來，
//              wb_serve 的 bin 鏈在它之後；sum 的分支看 CosFunction，cust 之後才有）。
//
//  Index（golden cSortCT.cpp:425-428 ShowSortIC 寫的 IndexInput／IndexOut／OutArm_input／labInArm_input 四格）：
//    binsel.index.io            4 個整數（逗號串）：LastSet.iIndexInputOutPut[0..3]（Index Input／Index Output／OutArm Pick／InArm Pick）
//    binsel.index.clearVisible  btnClearCount 看不看得到（golden FormShow :832-835：CC_KYEC_LEE 隱藏）
//    liveness：lastS／cust。Clear Counter 鈕走 WS act.showBinSelect.clearCount（WebShowBinSelect.cpp，golden btnClearCountClick :2411）。
//
//  UPH Information 與主畫面狀態列 [2] UPH／[5] 測試秒數：見本段檔尾 AI(W906-PROD-S114) 20260926 的 W906_StageUphStatusTags（Jimmy J3
//    58b01200 之後 CalculateUPH 寫的就是真的 fShowBinSelect->UPH_StringGrid）。Bin Display Status（NUMBER_PANEL 選配）：不發布，網頁 "---"。
//    AI(W906-PROD-S123) 20260927：同一支再加狀態列 [0] Index 節拍、[3] 版本，mainsb.* 改成只送數值（Steven 裁決 S123），8 格對照也在那裡。
//  流量：這 34 個 tag 幾乎是靜態的（換配方才變）；會跟著生產變的只有 binsel.cat.counts／sum 與 binsel.index.io，
//        而且 TagSnapshot 只送值變了的（patch）：iTestBinCount=16 時 counts 一筆約 60 bytes、io 約 40 bytes。
//  整數一律 "%ld"：這個 MinGW 的 msvcrt printf 不收 "%lld"（cObserver.cpp W906Obs_Int 的同一條理由）。
// =============================================================================
#include "common.h"   // AI(W906-PROD-S123) 20260927（Steven 團隊）：SVNRevision（common.h:197）—— 狀態列 [3] 版本的 AMD 尾碼（golden main.cpp:10716）
namespace ht9045 {

namespace {
// bin.visible、bin.color、binsel.cat.counts、binsel.cat.sum、binsel.cat.sort2d、binsel.index.io、binsel.index.clearVisible
const std::size_t kBinSelFixedTags = 7;
// 33 站扣掉 FW-BIN1 那六站（W906_IsFw1bStation）
const std::size_t kBinSelCaptionTags = (std::size_t)eTrayCount - 6u;

bool W906_BinSelCatLive(bool lastS, bool cust)
{
    return lastS && cust && g_webBinSelLoaded && iTestBinCount > 0 && iTestBinCount < TEST_MAX_BIN;
}

void W906_BinSelAppend(AnsiString& s, int idx, long v)
{
    char buf[24];
    std::snprintf(buf, sizeof(buf), idx ? ",%ld" : "%ld", v);
    s += buf;
}

// -----------------------------------------------------------------------------
//  AI(W906-PROD-S114) 20260926（Steven 團隊）：UPH Information 分頁＋主畫面狀態列 [2] UPH／[5] 測試秒數。
//  AI(W906-PROD-S123) 20260927（Steven 團隊）：＋狀態列 [0] Index 節拍、[3] 版本；[2] 改成送數值。RULINGS_20260926 S123（todo ★ Q1，
//    連帶 ★ Q36、R31），Steven 原話：「C++端會計算好數值之後, 發送到html, html端要決定格式；index time大概是3秒發送一次
//    (以index動作時間)；UPH是loader換盤的時候計算, 最快大概是1分鐘一次；版本號只有開機的時候發送一次」。
//    ⇒ mainsb.* 只送數值；"UPH = "、" Sec"、小數三位這些字由 web\page\main.html 照 golden 組（那邊的 AI(W906-PROD-S123) 段）。
//
//  golden StatusBar1（V912 main.dfm:4111-4143，8 格，dfm 沒有任何 Text ⇒ 開機時每一格都是空白）每一格的寫入點
//  （V912 全樹 grep `StatusBar1->Panels->Items[`，20260927）：
//    [0] Index 節拍  csystem.cpp:20795 ShowIndexTime(0)（IniConfig.bShowIndexTime 開）FormatFloat("0.000",秒)+" Sec"；:20805 關著時寫 ""；
//                    cCounterSel.cpp:72-73 FormClose 關掉 bShowIndexTime 的當下寫 ""；main.cpp:27997 Ctrl+A 寫 ""        → mainsb.indexTime
//    [1]             只有 csystem.cpp:20807（ShowIndexTime(Item!=0) 且 bShowIndexTime 關）寫 ""，而 Item!=0 的呼叫在 golden 全部是註解
//                    ⇒ 永遠空白，不發布
//    [2] UPH         ainarm9045.cpp:5757／:5762 CalculateUPH 最後 "UPH = "+UPH_StringGrid->Cells[3][1]；main.cpp:27998 Ctrl+A 寫 ""   → mainsb.uph
//    [3] 版本        main.cpp:10689-10754 TfMain::FormShow（開機一次）；HS_Function.cpp:5349 CheckMTKFileName(true) 在 V912 沒有呼叫者 → mainsb.version
//    [4]             沒有寫入點（SECSGEM.cpp:613 把 SV 103 指到這一格，讀到的永遠是空字串；移植樹 SV 1003 讀 RunInfo.SoftwareVersion，
//                    uHGemHT9045_SV.cpp:449）⇒ 永遠空白，不發布
//    [5] 測試秒數    main.cpp:21863／:21902 TfMain::Timer2Timer                                                               → mainsb.testSec
//    [6] 機台時間    main.cpp:8241 ProcessTimeUpdate                                     → clock.text（Jimmy 的 tag，本檔 :1092-1106，不在本段）
//    [7] 即時 UPH    cShowBinSelect.cpp:2464／:2468 TfShowBinSelect::CaculateUPH（IniConfig.bG10ShowImmediateUPH && SystemStart，
//                    TimerAutoCleanCountTimer 呼叫）"Net UPH: n/g"（CC_ASE_CL）／"Curr UPH: n"；ainarm9045.cpp:5756 寫 ""。移植樹
//                    TimerAutoCleanCountTimer（cShowBinSelect.cpp:2049）沒有呼叫者 ⇒ iNetUPH／iGrossUPH 從來不算 ⇒ 沒有真值，不發布
//                    （main.html 也沒有這一格）
//
//  binsel.uph.grid  字串：fShowBinSelect->UPH_StringGrid 的全部格子，照 golden 畫出來的樣子 —— RowCount 列（dfm 14）× ColCount 欄
//                   （dfm 4；偉測 IniConfig.bVTESTFunction 時 golden ShowBinSel :439-442 改成 7），列之間 "\n"、格之間 "\t"，
//                   每列固定 ColCount 格（空格就是空字串），所以網頁 split 之後的形狀＝golden 的格子。第 0 列＝表頭（golden FixedRows 預設 1）。
//                   唯一寫入者 golden CalculateUPH（V912 ainarm9045.cpp:5566-5764 → 移植樹 ainarm9045.cpp:8692；Jimmy J3 58b01200 起寫真的
//                   fShowBinSelect）：表頭 :5582-5585、第 1..10 列最近 10 次（每次往下推一列）、第 12 列 "Avg UPH :"＋平均。另外 KYEC_LEE 的
//                   Clean Out 會清第 1..12 列（golden csystem.cpp:16449-16459）、雙擊刪一列（UPH_StringGridDblClick :2201，網頁沒接）。
//                   golden 開機到第一次 CalculateUPH 之前整張是空的（dfm 沒有 Cells），這裡照送空格。
//                   ⚠ 格子裡不會有 \t／\n（golden 只寫 hh:nn:ss、整數、"Avg UPH :"）；萬一有，換成空白，免得格子錯位。
//  binsel.uph.tabVisible  布林：golden FormShow（V912 cShowBinSelect.cpp:863）Tab_UPH->TabVisible=IniConfig.bShowUPH。
//  mainsb.uph       整數（S123 起；S114 送的是伺服器組好的字 "UPH = 533"）：golden [2] 裡的數字 —— UPH_StringGrid->Cells[3][1]，
//                   也就是 CalculateUPH 寫進去的 RunInfo.iUPH（golden :5652／:5656）。golden [2] 只會是三種字，型別一律用整數表示：
//                     n ≥ 0  "UPH = n"  Cells[3][1] 的數字（std::atoi；golden :5706 自己讀這一欄也用 atoi）
//                     -1     "UPH = "   CalculateUPH 跑過（表頭 Cells[3][0]=="UPH"）但最新一列還沒有數字：開機後第一次呼叫一定走重設
//                                       分支（bOneTimes 初值 true，:5574-5606），tTempTime<=tUPH_StartTime 時也重設（:5721-5749）
//                     -2     ""         golden 這格空白：開機到第一次 CalculateUPH 之前（dfm 沒有 Text）；CC_ASE_KaohSiung 且 [G10] 開時
//                                       CalculateUPH 不寫 [2]（:5752-5758）
//                     null   "---"      設定還沒載入（liveness 同 binsel.uph.grid）
//                   移植樹 CalculateUPH 畫 [2] 那兩行在 GATE k7-G7a／G7b（fMain 沒有 StatusBar1），所以這裡從同一張表、照同一個式子取值。
//                   何時變：CalculateUPH(false) 只在 InArm 到 Loader 取「新盤第一顆」那一步呼叫（ainarm9045.cpp:9145-9149、
//                   ainarm9045_2x8_32.cpp:1223-1227；bRecordUPH 由 asendic_Loader.cpp:1509 Loader 換新盤時設，golden :1277）
//                   ⇒ Loader 每換一盤最多變一次（Steven：最快約 1 分鐘）。
//                   已知差異（只差在時機）：KYEC_LEE Clean Out 清表之後，golden 狀態列留著舊字到下一次 CalculateUPH，這裡立刻變 -1；
//                   golden Ctrl+A 熱鍵（main.cpp:27998 清成 ""）移植樹沒有。
//  mainsb.testSec   整數：golden 狀態列 Panels->Items[5]->Text（TfMain::Timer2Timer，V912 main.cpp:21855-21903）。golden 在測試中每換一秒
//                   寫 AnsiString(iCurrentTime)，不在測試時寫 "0" 且 iCurrentTime=0 —— 兩條路寫的字都等於 iCurrentTime，所以送 iCurrentTime，
//                   網頁顯示 String(n)。產生端 Jimmy J12（57027ca3，forms/fMain_TestSeconds.cpp，wb_serve 主迴圈每拍呼叫）。
//  mainsb.indexTime 浮點數（S123 新增，★ Q36）：golden [0] 的秒數 —— RunInfo.IndexTime（csystem.cpp:24169＝golden :20791），就是 golden
//                   接上 " Sec" 畫到 [0] 的那個 FormatFloat("0.000", …) 字串；這裡 std::atof 換回數值，網頁 toFixed(3)+" Sec" 得到同樣的字
//                   （那個字串本來就只有三位小數，換回來再印不會進位）。移植樹畫 [0] 那一行在 GATE H1-03（csystem.cpp:24194）。
//                     x ≥ 0  "x.xxx Sec"
//                     -1     ""         golden 這格空白：開機後還沒有 Index 完成過（RunInfo.IndexTime 還是 ""）；IniConfig.bShowIndexTime
//                                       關著（golden 關的當下 cCounterSel.cpp:72-73 清空，之後每次 Index :20805 也寫 ""）；關了再打開、
//                                       到下一次 Index 完成之前（golden 這段期間沒有人重寫 [0]，一樣空白）
//                     null   "---"      設定還沒載入（cust）
//                   何時變：ShowIndexTime(0) 的三個呼叫點 aTester_Front.cpp:9311、aTester_Rear.cpp:9276、atester_32Site.cpp:1882
//                   （golden aTester_Front.cpp:7004、aTester_Rear.cpp:7272、atester_32Site.cpp:1542）—— Index 一次交換完成那一步
//                   ⇒ 每個 Index 循環最多一次（Steven：約 3 秒）；秒數到毫秒位都跟上一次一樣就不送。
//                   已知差異：(a)「關了再打開」靠比 RunInfo.IndexTime 字串認出新的一次 Index，萬一下一次的秒數跟關之前最後一次到毫秒位
//                   都一樣，空白會多留一個循環；(b) bShowIndexTime 從 CounterSel 以外的地方關掉時，golden 要到下一次 Index 才清空，這裡
//                   立刻空白；(c) golden Ctrl+A（main.cpp:27997）移植樹沒有。
//  mainsb.version   字串（S123 新增）：golden [3] 的字 —— W906_MainSbVersionText()（本段下方），照 golden TfMain::FormShow
//                   main.cpp:10689-10754 組：asHandlerVersion，再依客戶碼／IniConfig／TestIF_File.i2DIDFormat／編譯旗標換字或接尾碼。
//                   版本號本來就是字串，尾碼由機台設定決定 ⇒ C++ 算好，網頁原樣顯示。golden 只在 FormShow 寫一次（DoReadLastData :9995
//                   之後），這裡也只算一次：第一次 cust 且 asHandlerVersion 不是空字串的發布就鎖住，之後不再重算（asHandlerVersion 由
//                   tools/wb_serve.cpp 開機段「Steven 20260925」那一塊算，在 DoReadLastData 與第一次發布之前）⇒ 線上只出現在開機後的
//                   第一次快照，之後不再 patch（每個新連上的瀏覽器照例收到一次完整快照）。
//                   null＝還沒鎖（設定沒載入，或 asHandlerVersion 是空的：wb_publish／ctest 不跑那段開機序列）。
//                   ⚠ golden :10757 RunInfo.SoftwareVersion=[3] 移植樹沒有人寫（SECS SV 1003、cMyDB.cpp:2006 讀到空字串）——
//                     要補就在 wb_serve 開機序列 DoReadLastData 之後呼叫 ht9045::W906_MainSbVersionText()（交 Jimmy，本段不改機台資料）。
//  liveness：binsel.uph.grid／mainsb.uph ＝ cust && fShowBinSelect（CalculateUPH 讀 CUSTOMER_CODE／IniConfig；在設定載入之前
//            這張表的形狀與狀態列的式子都不確定）；tabVisible／testSec／indexTime ＝ cust；version ＝ 鎖住之後。
//  流量：只送值變了的。grid 每次 CalculateUPH（Loader 每換一盤取第一顆，golden ainarm9045.cpp:5889-5892）變一次，一筆約 300-450 bytes
//        （4 欄×14 列；VTEST 7 欄約 600 bytes）；mainsb.uph 同時變，約 20 bytes；indexTime 每個 Index 循環一筆約 30 bytes；
//        testSec 測試中每秒一筆約 25 bytes，不在測試時不動；version 開機一次。
//  不動 kUnloadedTags 的 status.uph／status.indexTime（Jimmy 的名字），這裡用 St01 自己的 mainsb.*（R31）。
// -----------------------------------------------------------------------------
const std::size_t kBinSelUphTags = 2;   // binsel.uph.grid、binsel.uph.tabVisible
const std::size_t kMainSbTags    = 4;   // mainsb.uph、mainsb.testSec、mainsb.indexTime、mainsb.version（AI(W906-PROD-S123) 20260927：2 → 4）

// AI(W906-PROD-S123) 20260927：「golden 這格沒有數字」的值（見上方；main.html 用同樣的數字判斷）
const long long kMainSbUphNoNumber = -1;    // golden [2] ＝ "UPH = "
const long long kMainSbUphBlank    = -2;    // golden [2] ＝ ""
const double    kMainSbIndexBlank  = -1.0;  // golden [0] ＝ ""

// AI(W906-PROD-S123) 20260927：[0]「關了再打開、到下一次 Index 之前空白」與 [3]「開機算一次」要記住的狀態
bool       g_mainSbIdxBlank   = true;       // golden [0] 目前是空白（開機時 dfm 沒有 Text）
AnsiString g_mainSbIdxAtBlank;              // 變成空白那一刻的 RunInfo.IndexTime；它變了＝又有一次 Index 完成
bool       g_mainSbVerLatched = false;
AnsiString g_mainSbVer;

bool W906_UphLive(bool cust)
{
    return cust && fShowBinSelect != 0;
}

bool W906_MainSbVersionLive(bool cust)
{
    return g_mainSbVerLatched || (cust && asHandlerVersion != "");
}

void W906_UphCellAppend(AnsiString& s, const AnsiString& cell)
{
    const char* p = cell.c_str();
    for (int i = 0; p[i] != '\0'; ++i) {
        const char ch = (p[i] == '\t' || p[i] == '\n' || p[i] == '\r') ? ' ' : p[i];
        s += AnsiString(ch);
    }
}
}  // namespace

// -----------------------------------------------------------------------------
//  AI(W906-PROD-S123) 20260927（Steven 團隊）：golden 狀態列 [3] 的字 —— TfMain::FormShow（V912 main.cpp:10689-10754）逐行照抄，
//  只把 `StatusBar1->Panels->Items[3]->Text` 換成區域變數 t。純函式：只讀 asHandlerVersion／MainVersion／SVNRevision／CUSTOMER_CODE／
//  IniConfig／TestIF_File，不寫任何機台資料（golden :10757 RunInfo.SoftwareVersion=t 那一行不在這裡，見上方 mainsb.version）。
//  :10692-10697 CC_SCS 那一支 golden 另外把 lbArm0Torque／lbArm1Torque 藏起來 —— 那是別的畫面元件，不在狀態列，這裡不做。
//  :10729-10739 fAGV（E84）那一段夾在中間，跟 [3] 無關，略過。
//  對外（不在匿名 namespace）：給 wb_serve 開機序列補 RunInfo.SoftwareVersion 用（交 Jimmy）。
// -----------------------------------------------------------------------------
AnsiString W906_MainSbVersionText()
{
    AnsiString t;                                                               // golden main.dfm:4127-4129 Panels[3] 沒有 Text
#ifdef HiSilicon
    t=asHandlerVersion;                                                         // golden :10690
#else
    if(CUSTOMER_CODE==CC_SCS)                                                   //Steven 20140805   // golden :10692-10694
    {
        t="";
    }
    else if(IniConfig.bKoreaFunction)                                           // golden :10698-10700
    {
        t="V3.21";
    }
    else if(CUSTOMER_CODE==CC_SCC)                                              //Steven 20200430 : JSCC要求版號固定在V3.29   // golden :10702-10704
    {
        t="V3.29";
    }
    else
    {
        #ifdef MTK_Version
            t=AnsiString("MTK_")+MainVersion;                                   // golden :10709
        #else
            t=asHandlerVersion;                                                 //jou 20220916 : MainVersion -> asHandlerVersion   // golden :10711
        #endif
        #ifndef BETA_VERSION
        if(TestIF_File.i2DIDFormat==eAMD || IniConfig.bAMDFunction)             // golden :10714
        {
            t=asHandlerVersion+"_"+AnsiString(SVNRevision);                     // golden :10716
        }
        #endif
    }
#endif
    if(CUSTOMER_CODE==CC_ASE_KaohSiung)                                         //kevin 20210721 add Ver   // golden :10721-10722
        t=asHandlerVersion;

    if(IniConfig.bA38_SLT_Summary)                                              // golden :10724-10726
    {
        t=t+"_SLT";
    }

#ifdef FOR_ASECL_L8
    t=t+"_L8";                                                                  //JerryYang 20250120 : add   // golden :10742（asHandlerVersion 在 :9775 已經接過一次 _L8，golden 就是兩次）
#endif

#ifndef ASE_KaohSiung
    if(IniConfig.bEnableCCDUSETCPIP && CUSTOMER_CODE!=CC_KYEC_LEE && CUSTOMER_CODE!=CC_KYEC_XILINX)   // golden :10749
        t=t+"_CCD";                                                             // golden :10750
#endif

#ifdef DEBUG
    t=t+"_Debug";                                                               // golden :10754（golden .bpr 只定義 _DEBUG；本檔編譯時 DEBUG 也沒有定義，20260927 用 g++ -E -dM 量過）
#endif
    return t;
}

namespace {
std::size_t W906_StageUphStatusTags(webbridge::TagSnapshot& snap, bool cust)
{
    const bool live = W906_UphLive(cust);
    AnsiString grid;
    long long uph = kMainSbUphBlank;                                            // golden [2] 開機時空白（dfm 沒有 Text）
    if (live) {
        TfShowBinSelectGrid* g = fShowBinSelect->UPH_StringGrid;
        const int rows = g->RowCount, cols = g->ColCount;
        for (int r = 0; r < rows; ++r) {
            if (r) grid += "\n";
            for (int c = 0; c < cols; ++c) {
                if (c) grid += "\t";
                W906_UphCellAppend(grid, AnsiString(g->Cells[c][r]));
            }
        }
        if (CUSTOMER_CODE == CC_ASE_KaohSiung && IniConfig.bG10ShowImmediateUPH == true)
            uph = kMainSbUphBlank;                                              // golden :5752-5758：這一支不寫 [2]
        else if (AnsiString(g->Cells[3][0]) == "UPH") {                         // CalculateUPH 至少跑過一次（第一次一定走重設分支寫表頭）
            const AnsiString cell = AnsiString(g->Cells[3][1]);
            uph = (cell == "") ? kMainSbUphNoNumber                             // golden :5757／:5762 "UPH = "+""（還沒有數字）
                               : (long long)std::atoi(cell.c_str());            // golden :5757／:5762 "UPH = "+Cells[3][1]
        }
    }

    // AI(W906-PROD-S123) 20260927：[0] Index 節拍（見上方 mainsb.indexTime）
    double idx = kMainSbIndexBlank;
    if (cust) {
        if (IniConfig.bShowIndexTime == false) {                                // golden cCounterSel.cpp:72-73／csystem.cpp:20804-20805
            g_mainSbIdxBlank   = true;
            g_mainSbIdxAtBlank = RunInfo.IndexTime;
        } else if (g_mainSbIdxBlank && RunInfo.IndexTime != g_mainSbIdxAtBlank) {
            g_mainSbIdxBlank = false;                                           // golden :20795：又一次 Index 完成，重寫 [0]
        }
        if (!g_mainSbIdxBlank && RunInfo.IndexTime != "")
            idx = std::atof(RunInfo.IndexTime.c_str());                         // golden :20788-20795 AStr（"0.000"）+" Sec"
    }

    // AI(W906-PROD-S123) 20260927：[3] 版本 —— 開機算一次就鎖住（golden FormShow 只寫一次）
    if (!g_mainSbVerLatched && cust && asHandlerVersion != "") {
        g_mainSbVer        = W906_MainSbVersionText();
        g_mainSbVerLatched = true;
    }

    stageStr   (snap, "binsel.uph.grid",       live, grid);
    stageBool  (snap, "binsel.uph.tabVisible", cust, IniConfig.bShowUPH);       // golden cShowBinSelect.cpp:863
    stageInt   (snap, "mainsb.uph",            live, uph);                      // [2]（S123：數值；網頁組 "UPH = "）
    stageInt   (snap, "mainsb.testSec",        cust, (long long)iCurrentTime);  // [5] golden main.cpp:21863／:21902
    stageDouble(snap, "mainsb.indexTime",      cust, idx);                      // [0]（S123；網頁組 toFixed(3)+" Sec"）
    stageStr   (snap, "mainsb.version",        g_mainSbVerLatched, g_mainSbVer);   // [3]（S123；開機一次）
    return kBinSelUphTags + kMainSbTags;
}
}  // namespace

std::size_t W906_StageShowBinSelectTags(webbridge::TagSnapshot& snap, bool lastS, bool cust)
{
    TfShowBinSelect* f = fShowBinSelect;
    const bool binLive = g_webBinSelLoaded && f != 0;
    const bool visLive = binLive && cust;
    if (visLive)
        f->SetAutoVisible();                                                    // golden :1557-1608（見上）

    std::size_t n = 0;
    char tag[48];
    AnsiString vis, col;
    for (int i = 0; i < eTrayCount; ++i) {
        if (!W906_IsFw1bStation(i)) {
            std::snprintf(tag, sizeof(tag), "bin.%s", kSortSlug[i]);
            stageStr(snap, tag, binLive, binLive ? f->MyBinSel[i]->Caption : AnsiString());   // golden ShowBinSel :668／:709
            ++n;
        }
        if (visLive) vis += f->MyBinSel[i]->Visible ? "1" : "0";              // golden SetLabelVisible :1537
        if (binLive) W906_BinSelAppend(col, i, (long)f->MyBinSel[i]->Font->Color);   // golden ShowBinSel :690-702
    }
    stageStr(snap, "bin.visible", visLive, vis);
    stageStr(snap, "bin.color",   binLive, col);

    // Category Info（golden ShowCategoryBin 的顯示那一半，只讀）
    const bool catLive = W906_BinSelCatLive(lastS, cust);
    AnsiString counts;
    long sum = 0;
    if (catLive) {
        for (int i = 0; i < iTestBinCount + 1; ++i)                             // golden :1929 i<iTestBinCount+1（含 Error Bin）
            W906_BinSelAppend(counts, i, (long)LastSet.iBinData32[0][i]);
        if (CosFunction.bLowYieldUseContactCounts) {                            // golden :1862-1866
            for (int i = 0; i < iTestBinCount; ++i)
                sum += (long)LastSet.iBinData32[0][i];
        } else {                                                                // golden :1868-1874
            for (int i = 0; i < eTrayCount; ++i)
                sum += (long)LastSet.BinCT[0][iTo3Unload[i]];
        }
    }
    stageStr (snap, "binsel.cat.counts", catLive, counts);
    stageInt (snap, "binsel.cat.sum",    catLive, sum);
    stageBool(snap, "binsel.cat.sort2d", catLive, catLive && LastSet.iTester == _2D_SORT);   // golden :1879 / ShowInitialString :1783

    // Index（golden cSortCT.cpp:425-428）
    AnsiString io;
    if (lastS)
        for (int k = 0; k < 4; ++k)
            W906_BinSelAppend(io, k, (long)LastSet.iIndexInputOutPut[k]);
    stageStr (snap, "binsel.index.io",           lastS, io);
    stageBool(snap, "binsel.index.clearVisible", cust,  CUSTOMER_CODE != CC_KYEC_LEE);   // golden FormShow :832-835
    return n + kBinSelFixedTags + W906_StageUphStatusTags(snap, cust);          // AI(W906-PROD-S114) 20260926：UPH 分頁＋狀態列 [2]／[5]（本段上方）
}

void W906_ShowBinSelectCoverage(TagCoverage& c, bool lastS, bool cust)
{
    const bool binLive = g_webBinSelLoaded && fShowBinSelect != 0;
    c.total += kBinSelCaptionTags + kBinSelFixedTags;
    c.live  += (binLive ? kBinSelCaptionTags + 1u : 0u)                        // 27 站 caption ＋ bin.color
             + (binLive && cust ? 1u : 0u)                                      // bin.visible
             + (W906_BinSelCatLive(lastS, cust) ? 3u : 0u)                      // binsel.cat.counts／sum／sort2d
             + (lastS ? 1u : 0u) + (cust ? 1u : 0u);                            // binsel.index.io／clearVisible
    c.total += kBinSelUphTags + kMainSbTags;                                    // AI(W906-PROD-S114) 20260926：binsel.uph.*／mainsb.*（S123 起 2+4）
    c.live  += (W906_UphLive(cust) ? 2u : 0u)                                   // binsel.uph.grid／mainsb.uph
             + (cust ? 3u : 0u)                                                 // binsel.uph.tabVisible／mainsb.testSec／mainsb.indexTime（S123）
             + (W906_MainSbVersionLive(cust) ? 1u : 0u);                        // mainsb.version（S123：開機鎖住一次）
}

}  // namespace ht9045

// =============================================================================
//  AI(W906-PROD-S111) 20260926（Steven 團隊）：RULINGS_20260926 S111／S120-2「照 golden 補齊」——
//  golden TfMain::Timer10Timer（V912 main.cpp:35308-35352，Timer10 Interval=100 ms，main.dfm:17429-17431）裡
//  清除計數之後的收尾段（:35332-35351）。
//
//  為什麼要做：golden 每一次 ClearCount（cCounterClear.cpp:116-392）最後都設 bRefreshCount=true（:391，「清完數量後的顯示要
//    delay 一下」），真正的收尾在 Timer10：把清完的 Site 計數存進 system\Arm*.dat、重算 SortCT 的 Loading／各站數量、
//    清 Site Yield 警報旗標、清 Yield 監控的累計。移植樹在這之前沒有任何人讀 bRefreshCount（cCounterClear.cpp:412 只寫），
//    所以清完之後 Arm*.dat 一直是清除前的數字 —— 重開機 ReadCTInfo（tools/wb_serve.cpp 開機段）又把舊計數讀回來。
//  呼叫：上面 PumpTick() 在 MainProc 之後每拍一次（PumpTick 是本行程唯一的週期入口；做法同 W906_FlushFlagTick）。
//    golden 是 100 ms 一次，這裡是 500 ms 一拍（B13）——清除到存檔最多晚半秒，golden 的註解本來就要它「delay 一下」。
//    PumpTick 在 g_pumpActive（PumpInit 成功，InitialOK=true）之後才走到這裡；golden 第一行 `if(InitialOK==false) return;`
//    也照留。
//  誰會設 bRefreshCount（20260926 git grep，平行作業中可能再變）：WS counterclear.exe／counter.clear／act.counterClear.exe
//    （ClearCount 各類）、act.sortCT.clearCount 與 lot.start 等經 Clarn_Data 的清除、act.showBinSelect.clearCount
//    （ClearCount(ctIndexCount)）、MainProc 裡 csystem.cpp 的 ClearCount(ctBinCount)（Smart Auto Clean Initial Start）
//    與 ShowCategoryBin 的三處低良率 ClearCount(ctBinCount)。
//  ⚠ 會寫的真實檔：D:\HT9045\system\Arm{0,1,2}.dat、ArmHis{0,1,2}.dat、ArmByLot{0,1,2}.dat 與各自 _backup.dat（18 檔；
//    TestIF_File.bLowYieldAlarmByBin 時另寫同名 .ini）—— 每次清除後的下一拍整份覆寫；W906_MACHINERECORD_DIR 可轉開
//    （ctest 全體都設，tests/CMakeLists.txt 的 ENVIRONMENT）。ShowSortIC 會重算 RunInfo.iUnloadCount／SECS Pass／Fail，
//    並經 fShowBinSelect->ShowCategoryBin() 檢查低良率（golden 同）。
//  golden 跟這裡不同的一行：:35349 fContactCT->sgYield->Refresh() 是 VCL 重繪，Data.ContactCT 頁由 contactct.get 取值 ——
//    不呼叫（同 JsonBridge/actions/MainClarnData.cpp golden :15516 的偏離 D1）。:35313-35330 六顆 LED（五個敲擊氣缸＋Fix3 氣缸位置）是畫面，不在本段。
//  例外：PumpTick 對 wb_serve 保證「不會丟出例外」（MainProc 外面包了 try），這一段有檔案 I/O 與表單方法，所以自己包一層；
//    例外只計數並印一行，bRefreshCount 已經先清成 false（golden :35334 的順序），不會每拍重試。
// =============================================================================
#include "forms/fCounterClear.h"      // AI(W906-PROD-S111) 20260926：fCounterClear（cCounterClear.cpp:26 eager new）
#include "forms/fYieldMonitoring.h"   // AI(W906-PROD-S111) 20260926：fYieldMonitoring（uYieldMonitoring.cpp eager new）
namespace ht9045 {
void W906_CounterRefreshTick()
{
    static unsigned s_refreshExceptions = 0;
    if(InitialOK==false)                                                        // golden :35310-35311
        return;

    if(bRefreshCount==true)                                                     //Stteve 20250219 : 清除數量後的重整要delay一下   // golden :35332
    {
        bRefreshCount=false;                                                    // golden :35334
        try
        {
            fCounterClear->WriteCTInfo();                                       //Steven 20110801 : 清完要存檔   // golden :35335
            fSortCT->ShowLoadingIC();                                           // golden :35336
            fSortCT->ShowSortIC();                                              // golden :35337

            if((USE_AUTO_RETEST==eartInstall &&
                (bAutoReTest_ART || IniConfig.bA10_AutoReTest)) ||              //Ifor 20171011 (wei) : add 資料顯示更新
               CosFunction.bUseARTSortCount)                                    //kevin 20150615   // golden :35339-35341
            {
                fSortCT->ShowLoadingIC_ART();                                   // golden :35343
            }

            for(int i=0; i<32; i++)                                             //Site Yield Alarm(%)   // golden :35346
                fYieldMonitoring->bShowSiteYield[i]=false;

            // golden :35349 fContactCT->sgYield->Refresh(); —— VCL 重繪，不呼叫（見上方說明）
            fYieldMonitoring->ClearYieldCount();                                //Steven 20140830 : Yield相關的Alarm, 要清掉全部的Ignore的Count重算   // golden :35350
            std::printf("S111 counter refresh (golden Timer10Timer :35332-35351): Arm*.dat written, SortCT recalculated "
                        "(iUnloadCount=%d), site-yield flags and yield counters cleared\n", (int)RunInfo.iUnloadCount);
        }
        catch(...)
        {
            ++s_refreshExceptions;
            std::printf("S111 counter refresh: exception #%u (bRefreshCount already cleared, golden order :35334)\n", s_refreshExceptions);
        }
    }
}
}  // namespace ht9045

// =============================================================================
//AI(W906-CAPTION) 20260930: EastSun「圖片上應該是要對應機型顯示 目前機台是9050 設定檔裡面應該也有相關設定 請修正」.
//  The main window's title bar was the literal 'HT-9132（main.dfm）' (web/background.html WINDOWS[0]). golden sets
//  fMain->Caption from the machine type -- twice: the TfMain constructor (main.cpp:9216-9253, V906 golden) and again in
//  FormShow (main.cpp:9590-9607) for Type_HT9046_LS only, which wins because it runs later (and it is the one that feeds
//  RunInfo.MachineDefine at :9788). This is that final result, one function, published as tag machine.caption.
//  NOT golden: the first arm. golden has no HT9050 caption; D:\GPIB9045\system\general.ini [Version] Model=9050GPIB is the
//  HT9050 selector (database.cpp:492, docs/HT9050_1203_BRINGUP_PARAMETERS.md #1) and it decodes to Type_HT9046_LS
//  (RULINGS_20260926 #25), so without it an HT9050 would show golden's LS caption. W906_GpibModel keeps the model read.
//  "" = golden leaves the .dfm caption (the page keeps its static title).
#include "mysensor.h"       // Sen[] (golden FormShow :9601 Sen[SnNegativePressureAir2].Enable)
AnsiString W906_MainCaption()
{
    if (W906_GpibModel == "9050GPIB" || MachineTypeChoice == Type_HT9050) return "HT-9050";
    if (MachineTypeChoice == Type_HT9045)        return "HT-9045";                     // golden :9216-9219
    if (MachineTypeChoice == Type_HT9046)        return "HT-9046";                     // :9220-9223
    if (MachineTypeChoice == Type_HT9045_12Site) return "HT-9045_12Site";              // :9224-9227
    if (MachineTypeChoice == Type_HT502)         return "HT-502";                      // :9228-9231
    if (MachineTypeChoice == Type_HT1032)        return "HT-1032";                     // :9232-9235
    if (MachineTypeChoice == Type_HT7080)        return "HT-7080";                     // :9236-9239
    if (MachineTypeChoice == Type_HT9046_LS)                                           // FormShow :9590-9607 (overrides the ctor's :9240-9253)
    {
        if (SubMachineType == Type_HT9046LA)       return "HT-9046LA";
        else if (SubMachineType == Type_HT9016C)   return "HT-9046CN";
        else if (SubMachineType == Type_HT9046AU)  return "HT-9046AU";
        else if (SubMachineType == Type_HT9046CR)  return "HT-9046CR";
        #ifndef SOFT_SIMULTE
        else if (Sen[SnNegativePressureAir2].Enable == false) return "HT-9046LS";
        #endif
        else if (USE_IN_OUT_ARM_X_PITCH == iXPitch50mm) return "HT-9132LS";
        else                                       return "HT-9132";
    }
    return "";
}
