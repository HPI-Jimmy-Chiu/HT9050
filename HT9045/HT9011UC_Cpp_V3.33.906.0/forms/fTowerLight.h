// =============================================================================
//  forms/fTowerLight.h  --  non-VCL facade for golden's TfTowerLight
//                            (cTowerLight.h)
//
//  AI(W906-FW3-DTL1) 20260828: new file, FW wave FW3-DTL1 (facade wave 1 of
//  cTowerLight -- read-only direction, no write path).
//  GOLDEN SOURCE: HT9011UC_Code_V3.33.906.0_20260618/cTowerLight.h (88 lines)
//  + cTowerLight.cpp (159 lines), read with `io.open(p, encoding='cp950')`,
//  written out UTF-8 / bare-LF / 0 U+FFFD.
//
//  ROLE
//  ----
//  TfTowerLight is golden's tower-light + buzzer configuration dialog: an
//  8-state x 3-colour grid of clickable LEDs (Running / Jam / Pause / Message
//  / Heating / Home / Off-Line / Auto-Retest x Green,Yellow,Red) whose
//  tri-state values live in `LastSet.MessageLight[8][3]` (0=off, 1=on,
//  2=blink), plus 8 combo boxes selecting `LastSet.MusicSelect[i]`.  The form
//  only EDITS those two arrays -- the physical tower light and buzzer are
//  driven elsewhere, by ckernel.cpp:1777-1786 / :1696-1731 (`ht9045_sm`).
//
//  `python tools/census/wave_preflight.py cTowerLight.cpp` (20260828, re-run
//  at closeout): 8 bodies = 8 `TfTowerLight::` members + 0 file-scope,
//  133 span lines, 0 brace-swallow short-fall, 0 overload collapse.
//
//  THIS WAVE, against the 8-member denominator:
//     4 ACTIVE           ctor / UpdateTowerLed / Timer1Timer / spbExitClick
//     2 ACTIVE-PARTIAL   FormShow / FormClose -- everything except each
//                        body's single trailing `myLog.Do_Log(...)` line
//                        (GATE (T-2), a LINE-level gate; established
//                        precedent forms/fHandlerSys.h GATE (H22-1))
//     2 GATED-WITH-BODY  RGB00Click / rgMusicTestClick, golden text carried
//                        as `#if 0` transcript in forms/fTowerLight.cpp
//  BY LINES: the 6 ACTIVE(-PARTIAL) bodies' golden spans total 101 of 133
//  (75.9%); the 2 fully-GATED bodies are 32 of 133 (24.1%).  Quote both
//  numbers or neither.
//
//  ============================================================================
//  *** THE GLOBALS `TfTowerLight` / `fTowerLight` ARE FREE -- THIS WAVE
//  CLAIMS THEM ***
//  Measured 20260828 with `python tools/census/wave_preflight.py
//  cTowerLight.cpp` (section [3] SHIM OCCUPATION, 1195 port files scanned,
//  build_*/vendor/third_party excluded): (a) class name `clear`, (c) global
//  name `clear`, (d) TU-local seam / #define `clear`.  Cross-checked the same
//  day with `rg -c -g '!build_*' -g '*.h' -g '*.cpp' "\bTfTowerLight\b"` /
//  `"\bfTowerLight\b"` -- 0 files.
//  `.h` declares `extern TfTowerLight *fTowerLight;`, `.cpp` defines
//  `TfTowerLight *fTowerLight = new TfTowerLight();` -- same idiom as
//  forms/fPrecaution.cpp:14 / forms/fHotPlate.cpp:24 / forms/fTesterTCP.cpp:49.
//
//  CONDITION-TWO CHECK (does a port of these bodies ALREADY exist, possibly
//  as a RENAMED free function outside the class?).  Searched 20260828, whole
//  port tree, build_* excluded:
//    * `rg "\b(TowerLight_|fTowerLight_)\w+"`                      0 hits
//    * per-name file counts for every distinctive identifier of this file
//      (`UpdateTowerLed`, `rgMusicTestClick`, `Timer1Timer`)        0 hits
//    * distinctive data the bodies touch: `MessageLight` 4 files
//      (ckernel.cpp / cprod.cpp / LastSet.h / uHGemHT9045_EC.cpp),
//      `MusicSelect` 4 files (same set) -- all of them are OTHER consumers of
//      the same LastSet arrays (the tower-light DRIVER and the SECS EC
//      registration), none is a port of a `TfTowerLight::` body.
//  Conclusion: nothing to re-use, nothing re-translated twice.
//
//  ============================================================================
//  PRE-COMMITTED #if 0 CALL SITES ELSEWHERE IN THE TREE (1 name, 3 sites)
//  ============================================================================
//    fShow    ckernel.cpp:1690, Command.cpp:10246, Command.cpp:15034
//             -- matches this facade's `bool fShow;`.
//  ⚠ Command.cpp:10246 and :15034 are each ONE line that names BOTH
//  `fLd_ULd->fShow` AND `fTowerLight->fShow` (a long "is ANY form open" OR
//  chain) -- that is not a duplicate report, it is the shape of the source.
//  ⚠ The SAME chain spells `fCCLink->bShow` with a **b**, not `fShow`.  Do
//  not normalise it.
//  All 3 sites stay `#if 0` after this wave -- declaring `fShow` does not
//  un-gate them (that is each owning file's own separate edit).
//
//  ============================================================================
//  GATE REGISTER (3: two whole-body, one line-level)
//  ============================================================================
//  (T-1) RGB00Click(TObject*)   golden :55-80 (26L) -- WRITES DISK.
//        Line :78 `WriteLastDataFile();` is UNCONDITIONAL, in the same body,
//        after the LastSet mutation.  `WriteLastDataFile` is REAL and
//        REACHABLE (cprod.cpp:2001, `add_library(ht9045_globals)` --
//        ht9045_forms links ht9045_globals, so this is NOT a link-boundary
//        gate), which is exactly why it has to be gated on the SAFETY rule
//        instead: it serialises the whole LastSet block to disk.  Its target
//        path is a HARD-CODED literal (cprod.cpp:1701/:1729 for the
//        Read/Write pair), so `wb_publish --dry`'s ini redirect does NOT
//        cover it -- docs/KNOWLEDGE.md records that this tree has already
//        destroyed teach/lastdata once this way.  This is the
//        read-only-direction campaign's hard rule ("會改到磁碟 -> gated"),
//        so the whole body is gated rather than split: dropping just :78
//        would leave the in-memory LastSet edit with no persistence, which is
//        an INVENTED behaviour, not a translation.
//        NOTE for a future write-path wave: the rest of this body is clean
//        and reachable (`IniConfig.bA02DisableSaveParsWhenSwitchToOp` /
//        `IniConfig.bSPILFunction` -- Config.cpp, ht9045_globals;
//        `AccessLevel` -- cmydef.cpp, same; `ShowMyMessage` --
//        canary_support.cpp, one of the four sanctioned forms->sm edges;
//        `LastSet.MessageLight` -- LastSet.cpp, same).  This is the single
//        highest-value revisit target in this file.
//  (T-2) `myLog.Do_Log(Sender, asUser, asLogPath);`
//        golden :125 (inside FormShow) and golden :138 (inside FormClose)
//        -- LINE-LEVEL gate, NOT a whole-body gate.  Both surrounding bodies
//        are otherwise fully translated and ACTIVE.  Three independent
//        reasons, all re-verified this wave and all matching the established
//        precedent forms/fHandlerSys.h GATE (H22-1):
//          (a) LINK BOUNDARY.  `TMyLog` is declared in handlerlog.h:139 but
//              its DEFINITION TU is handlerlog.cpp, whose enclosing
//              `add_library` block is **ht9045_sm** (located this wave by
//              walking the enclosing add_library() with a script; target name
//              quoted rather than a line number because line numbers move
//              whenever a sibling wave adds a file).  `ht9045_forms` links
//              only `vclcompat` + `ht9045_globals` + `ht9045_core`, so
//              ht9045_sm is unreachable from here.
//          (b) WRITES DISK.  `TMyLog::Do_Log` funnels into `Save_Log()` /
//              the machine EventLog (handlerlog.h's own file-head note) --
//              the campaign's read-only rule forbids it independently of (a).
//          (c) NO BEHAVIOUR EVEN IF WIRED.  Do_Log's first gate is
//              `dynamic_cast<TWinControl*>(PCtrl)!=NULL`; this facade has NO
//              base class at all (DEVIATION D-2), so golden's `Sender` (the
//              form itself, in VCL) can never pass it here.
//        Consequence of DEVIATION D-3 (Sender dropped from FormClose): the
//        line is not even spellable in the ported signature.
//  (T-3) rgMusicTestClick(TObject*)  golden :141-146 (6L) -- TWO INDEPENDENT
//        GATES.
//          (a) LINK BOUNDARY.  `SW[SwMusic1+i].Off()` / `.On()` -- `SW[]` is
//              `extern class TMySwitch SW[MAX_SWITCH_ITEM]` (myswitch.h:43)
//              and its defining TU is myswitch.cpp, whose `add_library` block
//              is **ht9045_io**.  ht9045_forms does not link ht9045_io.
//              (`SwMusic1` itself IS reachable -- cmydef.cpp, ht9045_globals
//              -- the switch OBJECT is what is out of reach.)
//          (b) MACHINE OUTPUT.  These are the four physical buzzer/music
//              relays.  Calling this body makes the machine make a noise, so
//              it is gated on the safety rule too, independently of (a).
//
//  ============================================================================
//  ACTIVE (6, two of them PARTIAL) -- reachability evidence
//  ============================================================================
//   TfTowerLight()          golden :19-22 -- EMPTY body past `: TForm(Owner)`.
//        Fields only, no global deref, no call.  SIOF-safe (docs/KNOWLEDGE.md
//        "static-init ctor 不可碰那 18 個 NULL 全域"; the fLaserSensor
//        incident that rule comes from turned 88 of 134 ctest binaries into
//        SEGFAULTs).
//   UpdateTowerLed(int)     golden :24-53 (30L), private -- reads
//        `LastSet.MessageLight[i][j]` (LastSet.cpp, `ht9045_globals`) and
//        writes 24 OWN LED stand-in fields.  No disk, no packet, no hardware:
//        `TALed` here is the on-screen indicator, not the tower lamp.
//   FormShow(TObject*)      golden :82-126 (45L) ACTIVE-PARTIAL (all but :125,
//        GATE (T-2)) -- `LastSet.MusicSelect[]` clamp+read (ht9045_globals),
//        `CUSTOMER_CODE`/`CC_KYEC_XILINX` (cmydef.cpp / MachineType.h:295),
//        `CosFunction.bAutoRetestGPIBmode` / `.bTowerLightUseLD`
//        (CosFunction.cpp, ht9045_globals), `IniConfig.bSPILFunction`
//        (Config.cpp, same), own widgets only otherwise.
//        ⚠ It DOES write `LastSet.MusicSelect[i]` (golden :90/:92, the
//        0..4 clamp).  That is an IN-MEMORY global write, which this campaign
//        permits -- the rule is about disk / packets / hardware.  Flagged
//        because it is a write, and because the clamp is golden's own.
//   FormClose(...)          golden :128-139 (12L) ACTIVE-PARTIAL (all but
//        :138, GATE (T-2)) -- same in-memory-write note as FormShow.
//   Timer1Timer(TObject*)   golden :148-153 -- `fShow` guard + UpdateTowerLed.
//   spbExitClick(TObject*)  golden :155-158 -- `Close();` (DEVIATION D-4).
//
//  ============================================================================
//  DEVIATIONS (port-only; each with why)
//  ============================================================================
//  (D-1) `__fastcall` dropped from every signature -- tree-wide convention.
//  (D-2) `TfTowerLight` has NO base class (golden: `: public TForm`).  Matches
//      forms/fCleaning.h / forms/fHotPlate.h (D-2) / forms/fPrecaution.h (D-4).
//      `Left`/`Top` therefore become plain int members (see FIELD LIST) --
//      same shape as forms/fTrayAssignment.h:678-679, which records the same
//      "FormShow writes 75 / 10" pair verbatim.
//  (D-3) `FormClose(TObject*, TCloseAction&)` -> `void FormClose();`.
//      `TCloseAction` has no port anywhere in this tree; established
//      precedent forms/fMesSystem.h D-5 / forms/fHotPlate.h (:281).  Golden's
//      body reads NEITHER parameter except in the (T-2)-gated Do_Log line.
//  (D-4) `Close()` is a PORT-ONLY no-op (`{}`) -- golden's own inherited
//      `TForm::Close`, which D-2 removes.  Needed by ACTIVE spbExitClick and
//      by the GATED RGB00Click transcript.
//  (D-5) `TALed` (golden ALed.hpp, a third-party VCL component) has no port.
//      `TfTowerLightLed` is a facade-local, distinctly-named stand-in
//      carrying the two properties golden touches: `Value` (bool) and `Tag`
//      (inherited from vclcompat::TControl).  Distinctly named ON PURPOSE:
//      `class TALed` already exists -- exactly ONE definition tree-wide,
//      mykitsuck.h:132 (counted 20260828, same command as D-6) -- with a
//      DIFFERENT shape, and this tree has already paid for a
//      two-headers-one-classname
//      collision once (docs/KNOWLEDGE.md, the two `TMyKitSuck` layouts) --
//      same reasoning forms/fTemperFrom.h:170-183 gives for
//      `TfTemperFromTimer`.
//  (D-6) `TTimer` has no vclcompat port.  SEVEN unrelated, mutually
//      incompatible `class TTimer` definitions exist in this tree (counted
//      20260828 with `rg "^\s*class\s+TTimer\b" -g '*.h' -g '*.cpp'`):
//      ATC/ATCInterface.h:189, BinDisplay/MyBinDisp.h:231,
//      Automation/uRENESAS_Server.h:168, forms/fGroundMan.h:456,
//      OmronLaser/LaserSensor.h:174, forms/fMonitor.h:221,
//      PowerSavingMode.h:75.  Reusing the bare name would risk a
//      multi-definition collision the moment any TU included two of them.
//      `TfTowerLightTimer` is a facade-local one-field stand-in --
//      `->Enabled` is the ONLY member golden touches (`:100`, `:135`).
//      Same idiom and same reasoning as forms/fTemperFrom.h:247-251
//      (`TfTemperFromTimer`) and forms/fMotorTest.h:468-474.
//  (D-7) golden `TBevel *Bevel1..4`, `TLabel *Label2/3/4/6/43`,
//      `TPanel *Panel1..9`, `TPanel *Panel2` etc. are NOT declared -- no
//      ACTIVE body and no pre-committed site touches them (minimal-
//      declaration idiom, forms/fMesSystem.h / forms/fHotPlate.h).
//      `TBevel` has no port in this tree either (0 hits).
//
//  ============================================================================
//  FIELD LIST (24 LEDs + 8 combos + 1 radio group + 2 panels + 1 timer +
//  1 bool + 2 ints)
//  ============================================================================
//    fShow (bool)                                  -- PRE-COMMITTED (3 sites)
//    RGB00/01/02 .. RGB70/71/72 (TfTowerLightLed*) -- UpdateTowerLed, and
//        RGB00Click's `(TfTowerLightLed*)Sender->Tag` (GATED)
//    cbRunning/cbJam/cbPause/cbMessage/cbHeating/cbHome/cbOffLine/cbART
//        (TComboBox*)                              -- FormShow / FormClose
//    rgMusicTest (TRadioGroup*)                    -- FormShow (ItemIndex=0)
//    Panel11 / palART (TPanel*)                    -- FormShow
//    Timer1 (TfTowerLightTimer*)                   -- FormShow / FormClose
//    Left / Top (int)                              -- FormShow writes 75 / 10
//  NOT declared (touched by no ACTIVE body and no pre-committed site):
//    Panel1..Panel9, Panel2, Label2/3/4/6/43, Bevel1..4.
//
//  ============================================================================
//  ZERO-WRITER FIELDS / VALUE PROVENANCE
//  ============================================================================
//  `Left`/`Top` are written ONLY by the ACTIVE FormShow (75 / 10) and read by
//  nobody in this port -- vclcompat::TControl's own note is explicit that
//  Left/Top只存值, nothing moves.  Recorded, not claimed as layout behaviour.
//  The 24 LED `Value` fields are written by ACTIVE UpdateTowerLed and read by
//  nobody else in this port, exactly as in golden (golden's reader is the
//  screen).  `LastSet.MessageLight[][]` is an EXTERNAL global whose writers
//  in THIS file are FormShow's clamp (ACTIVE) and RGB00Click (GATED T-1) --
//  cprod.cpp:1904-1906 also seeds `[6][*]` process-wide, so this is not a
//  zero-writer field in the "member this wave introduces" sense.
//
//  ============================================================================
//  DISCLOSURE (task rule 8 -- what was NOT read to completion / NOT verified)
//  ============================================================================
//  - golden `cTowerLight.dfm` was NOT opened.  The facade's Left/Top default
//    to 0 rather than to their design-time values, and no LED's design-time
//    `Tag` is supplied.  That matters for the GATED RGB00Click, which
//    dispatches on `Ptr->Tag/3` and `Ptr->Tag%3`: with Tag==0 for every LED
//    the whole grid would resolve to cell [0][0].  This is the exact
//    "解 gate 前先查值從哪來" hazard (docs/KNOWLEDGE.md) -- a future wave
//    un-gating (T-1) MUST supply Tag first, and 0 is NOT a neutral default
//    here.  Recorded rather than fixed, because inventing Tag values is a
//    behaviour decision.
//  - `WriteLastDataFile`'s body (cprod.cpp:2001-...) was NOT read in full;
//    only its name, its target-path provenance (docs/KNOWLEDGE.md) and its
//    owning add_library were established.  Moot while (T-1) is gated.
//  - `TMySwitch::On/Off/OnOff` bodies were NOT read; only myswitch.cpp's
//    owning add_library (`ht9045_io`) was established, which is what (T-3)(a)
//    turns on.
// =============================================================================
#ifndef FORMS_FTOWERLIGHT_H
#define FORMS_FTOWERLIGHT_H

#include "forms/FormWidgets.h"   // AnsiString / TObject / TControl / TPanel /
                                 // TComboBox / TRadioGroup (vclcompat)

// ---------------------------------------------------------------------------
//  TfTowerLightLed -- stand-in for golden `TALed *` (cTowerLight.h:18-35,
//  :61-63, :68-70).  See DEVIATION (D-5) for why the name is not `TALed`.
//  `Value` is the LED's on/off state; `Tag` comes from vclcompat::TControl
//  (golden TComponent::Tag) and is what the GATED RGB00Click dispatches on.
// ---------------------------------------------------------------------------
class TfTowerLightLed : public TControl
{
public:
    bool Value;
    TfTowerLightLed() : Value(false) {}
    virtual ~TfTowerLightLed() {}
};

// ---------------------------------------------------------------------------
//  TfTowerLightTimer -- stand-in for golden `TTimer *Timer1`
//  (cTowerLight.h:59).  See DEVIATION (D-6).  `->Enabled` is the only member
//  golden touches (:100 true, :135 false).
// ---------------------------------------------------------------------------
struct TfTowerLightTimer
{
    bool Enabled = false;   // golden .dfm default not read -- see DISCLOSURE
};

// ===========================================================================
//  TfTowerLight -- non-VCL facade (golden cTowerLight.h:14-84,
//  `class TfTowerLight : public TForm`; NO base class here, DEVIATION D-2).
// ===========================================================================
class TfTowerLight
{
public:
    TfTowerLight();

    // -- golden public: User declarations ------------------------------------
    bool fShow = false;                                  // golden :83, PRE-COMMITTED (ckernel.cpp:1690, Command.cpp:10246, Command.cpp:15034)

    // -- DEVIATION D-2: golden inherited these from TForm --------------------
    int Left = 0;                                        // FormShow golden :98 writes 75
    int Top  = 0;                                        // FormShow golden :99 writes 10

    // -- golden __published widgets touched by an ACTIVE body (or by the
    //    GATED transcripts) -- see FIELD LIST ------------------------------
    TfTowerLightLed *RGB00 = new TfTowerLightLed();       // golden :23
    TfTowerLightLed *RGB01 = new TfTowerLightLed();       // golden :24
    TfTowerLightLed *RGB02 = new TfTowerLightLed();       // golden :35
    TfTowerLightLed *RGB10 = new TfTowerLightLed();       // golden :22
    TfTowerLightLed *RGB11 = new TfTowerLightLed();       // golden :25
    TfTowerLightLed *RGB12 = new TfTowerLightLed();       // golden :34
    TfTowerLightLed *RGB20 = new TfTowerLightLed();       // golden :21
    TfTowerLightLed *RGB21 = new TfTowerLightLed();       // golden :26
    TfTowerLightLed *RGB22 = new TfTowerLightLed();       // golden :33
    TfTowerLightLed *RGB30 = new TfTowerLightLed();       // golden :20
    TfTowerLightLed *RGB31 = new TfTowerLightLed();       // golden :27
    TfTowerLightLed *RGB32 = new TfTowerLightLed();       // golden :32
    TfTowerLightLed *RGB40 = new TfTowerLightLed();       // golden :19
    TfTowerLightLed *RGB41 = new TfTowerLightLed();       // golden :28
    TfTowerLightLed *RGB42 = new TfTowerLightLed();       // golden :31
    TfTowerLightLed *RGB50 = new TfTowerLightLed();       // golden :18
    TfTowerLightLed *RGB51 = new TfTowerLightLed();       // golden :29
    TfTowerLightLed *RGB52 = new TfTowerLightLed();       // golden :30
    TfTowerLightLed *RGB60 = new TfTowerLightLed();       // golden :61
    TfTowerLightLed *RGB61 = new TfTowerLightLed();       // golden :62
    TfTowerLightLed *RGB62 = new TfTowerLightLed();       // golden :63
    TfTowerLightLed *RGB70 = new TfTowerLightLed();       // golden :70
    TfTowerLightLed *RGB71 = new TfTowerLightLed();       // golden :69
    TfTowerLightLed *RGB72 = new TfTowerLightLed();       // golden :68

    TComboBox *cbRunning = new TComboBox();               // golden :51
    TComboBox *cbJam     = new TComboBox();               // golden :52
    TComboBox *cbPause   = new TComboBox();               // golden :53
    TComboBox *cbMessage = new TComboBox();               // golden :54
    TComboBox *cbHeating = new TComboBox();               // golden :55
    TComboBox *cbHome    = new TComboBox();               // golden :56
    TComboBox *cbOffLine = new TComboBox();               // golden :64
    TComboBox *cbART     = new TComboBox();               // golden :67

    TRadioGroup *rgMusicTest = new TRadioGroup();          // golden :57
    TPanel *Panel11 = new TPanel();                        // golden :66
    TPanel *palART  = new TPanel();                        // golden :71

    TfTowerLightTimer *Timer1 = new TfTowerLightTimer();   // golden :59, DEVIATION D-6

    // -- ACTIVE this wave (defined in forms/fTowerLight.cpp) ------------------
    void FormShow(TObject *Sender);                        // golden :82-126   ACTIVE-PARTIAL (:125 -> GATE T-2)
    void FormClose();                                      // golden :128-139  ACTIVE-PARTIAL (:138 -> GATE T-2), DEVIATION D-3
    void Timer1Timer(TObject *Sender);                     // golden :148-153  ACTIVE
    void spbExitClick(TObject *Sender);                    // golden :155-158  ACTIVE
    void Close() {}                                        // DEVIATION D-4, port-only no-op

    // -- GATED this wave -- declared here, golden body translated in
    //    forms/fTowerLight.cpp inside `#if 0`.  Calling one is a LINK ERROR
    //    by design (see GATE REGISTER above). ---------------------------------
    void RGB00Click(TObject *Sender);                      // golden :55-80    GATE (T-1)  //AI(W906-TOWERLIGHT) 20260925: 已解閘，照 golden V912 :55-81（含 V912 :62 的 return;）＋建構子補 dfm Tag；呼叫者 WebTowerLight.cpp（WS towerlight.op）
    void rgMusicTestClick(TObject *Sender);                // golden :141-146  GATE (T-3)

private:
    void UpdateTowerLed(int Attr);                         // golden :24-53    ACTIVE (private, matches golden :79)
};

// AI(W906-FW3-DTL1) 20260828: TfTowerLight/fTowerLight were FREE tree-wide
// (see banner's measured provenance) -- declared here so the 3 pre-committed
// `#if 0` sites (ckernel.cpp:1690, Command.cpp:10246, Command.cpp:15034) have
// something real to bind to once each owning file's OWN gate is separately
// lifted (NOT done by this wave).
extern TfTowerLight *fTowerLight;

#endif // FORMS_FTOWERLIGHT_H
