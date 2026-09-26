// =============================================================================
//  forms/fIoSetView.h  --  non-VCL facade for golden's Tfiosetview
//  (iosetview.h / iosetview.cpp) -- the low-level hardware IO test/bring-up
//  screen: manual switch/LED panels for every sensor, cylinder, sucker, TTL
//  line and analog EP/light channel on the machine, plus the raw IO table
//  grid (Sensor/Sucker/Switch/Cylinder) and the Safe-PLC LED bank.
//
//  AI(W906-FW3-IOSV1) 20260827: new file, FW wave FW3-IOSV1 (facade wave 1
//  of iosetview -- read-only direction, no write path).
//  Golden source of truth: HT9011UC_Code_V3.33.906.0_20260618/iosetview.cpp
//  (4,125 lines) + iosetview.h (3,046 lines, __published block alone is
//  2,929 widget declarations) + iosetview.dfm.  Decoded this wave with
//  `io.open(p, encoding='cp950')`, written out UTF-8, 0 U+FFFD.
//
//  ============================================================================
//  *** SAFETY POSTURE -- READ BEFORE TOUCHING ANY GATE IN THIS FILE ***
//  ============================================================================
//  Tfiosetview is the ENGINEER'S HARDWARE BRING-UP PANEL.  Its entire reason
//  to exist is to let a human directly command machine outputs while the
//  handler's normal state machine is not running: it opens/closes the index
//  vacuum suckers (mySuckerTemp->Suck()/Destroy(), FTestSuck/BTestSuck
//  .Suck()/.Destroy()/.Normal()), pulses the TTL start lines to the tester
//  (SW[SwStart0/1].On()/Off()), writes analog EP-pressure and light-position
//  channels (ADAM_DirectWriteData), actuates cylinders (Cylinder[...].Off()),
//  and forces an index-motor home/lock cycle
//  (LockIndexMotorAndDoHomeProcess()) from its Timer1Timer poll.  Of golden's
//  65 method definitions, this wave delivers 5 and gates 60 -- see WAVE SCOPE.
//
//  Every gated method is DECLARED here and DELIBERATELY NOT DEFINED in
//  forms/fIoSetView.cpp (same idiom as forms/fContact.h / forms/fMotorTest.cpp).
//  The LINKER is the interlock: the moment any future wave -- or the web
//  write path -- tries to CALL one, the build fails with `undefined
//  reference to Tfiosetview::<method>` naming the exact entry point.  No
//  member here is virtual, so an undefined member function costs nothing at
//  link time until somebody calls it.
//
//  FOUR declarations are OMITTED ENTIRELY rather than gated -- see ABSENT
//  PARAMETER TYPE below.  Omission (not a stub, not a gate) is deliberate:
//  their parameter type has an unresolved multi-way naming collision
//  tree-wide, and writing a declaration that picks one arm would be a new,
//  undocumented landmine, not a safety decision this wave is positioned to
//  make.
//
//  UN-GATING PROTOCOL: translate the body in a wave that owns the interlock
//  story (who may issue it, what must be homed / servo-on / door-closed
//  first, and what re-checks it on the C++ side).  Never as a side effect of
//  "finishing the facade".
//
//  ============================================================================
//  DENOMINATOR -- MEASURED THIS WAVE, NOT INHERITED
//  ============================================================================
//  `python tools/census/coverage_probe.py iosetview.cpp` (single-file mode,
//  20260827): golden has 65 `Tfiosetview::` method DEFINITIONS (span_sanity.py
//  independently parses 65 with 0 short-fall, so no comment-swallowed-brace
//  span risk here).  Because forms/fIoSetView.cpp did not exist before this
//  wave, STRICT/loose/none are 0/1530/2478 golden SPAN LINES (not gcode) --
//  i.e. every golden function currently reads NONE or loose-noise, which is
//  the expected shape for a brand-new facade and not itself informative.
//  THIS WAVE: 5 of 65 delivered (7.7% of golden method DEFINITIONS), 60
//  gated, 4 omitted (counted inside the 60 for the method-count denominator
//  above; itemised separately below because they are declared nowhere, not
//  merely undefined).
//
//  BY LINES THE SHARE IS SMALLER STILL: the 5 delivered bodies' golden spans
//  total 147 lines (ctor's ACTUAL translated content is 0 -- see DEVIATION
//  D-2; ResetIndexSuck 9 + ResetIndexDestroy 9 + LabSiteMap 123 +
//  strngrdIoTableSelectCell 6 = 147, and LabSiteMap alone is 84% of that)
//  out of golden's ~4,000-line body.  SetCompomentIO alone (585 lines,
//  GATED) is 4x this whole wave.  Quote both numbers or neither.
//
//  ⚠ edtPulseDelayMouseDown was PLANNED active (delegates to fQwertyKey->
//  ShowQwertyKey, same archive, zero widget-member footprint) and was moved
//  to GATED only after `g++ -fsyntax-only` rejected it: golden's body reads
//  `Ptr->Name` (Ptr is a TEdit*), and vclcompat has NO `Name` property on
//  TControl/TEdit/TCustomEdit at all -- see GATE (X-16) for the citation
//  trap this uncovered.  Recorded exactly like forms/fContact.h's own (X-32)
//  precedent: "reading a body for MACHINE ACTIONS does not catch a missing
//  [property]... the compiler is the only reliable checker."
//
//  ============================================================================
//  *** THE GLOBAL `fiosetview` IS ALREADY TAKEN -- THIS FILE DOES NOT CLAIM IT ***
//  ============================================================================
//  Golden ends iosetview.h:3041 with `extern PACKAGE Tfiosetview *fiosetview;`.
//  This header deliberately declares NO such global, because the port tree
//  already has a live, registered stand-in for that exact name:
//
//      atester_shims.h:348   class TfiosetviewShim { ... };
//      atester_shims.h:354   extern TfiosetviewShim *fiosetview;
//      atester_shims.cpp:380 TfiosetviewShim *fiosetview = new TfiosetviewShim();
//
//  with LIVE call sites in atester.cpp / atester_32Site.cpp / aTester_Front.cpp
//  / aTester_Rear.cpp / AutoClean/AutoClean.cpp.  Declaring `extern
//  Tfiosetview *fiosetview;` here would be a hard redefinition in any TU that
//  sees both headers.  The CLASS name `Tfiosetview` is NOT occupied (the shim
//  uses `TfiosetviewShim`), so the class lands here; the global pointer does
//  not.  Re-verified 20260827 (see ABSENCE RE-RUN).
//
//  ============================================================================
//  ABSENT PARAMETER TYPE -- 4 DECLARATIONS OMITTED, NOT GATED
//  ============================================================================
//  golden SetCompomentIO / SetCompomentHint / SetPanelToEnable / ScanLed all
//  take `TWinControl *PCtrl` and recurse over ->ControlCount / ->Controls[i].
//  `class TWinControl` exists in THREE places tree-wide, each a distinct,
//  unqualified, global-scope definition, NOT the same type:
//      handlerlog.h:104   class TWinControl : public TControl { AnsiString Name;
//                          TWinControl *Parent; int ControlCount;
//                          std::vector<TControl*> Controls; ... };
//      language.h:79      class TWinControl : public TControl { ... };
//      Public/HTEdit.h:140 class TWinControl : public TControl { ... };
//  (grepped 20260827, `class\s+TWinControl\b` over all .h/.hpp excluding
//  build*/.git/.svn/.pti_frames/web -- exactly 3 hits, all definitions).
//  Each was added independently by a wave that needed generic control-tree
//  reflection and, per rule 7 (no editing pre-existing files outside this
//  wave's write boundary), declared its own header-local type rather than a
//  shared one.  Picking any ONE of the three to `#include` here to spell a
//  gated declaration's parameter type would silently commit this file to
//  that header's specific layout, and the moment a future TU includes this
//  facade alongside a DIFFERENT one of the three, it is a hard `error:
//  redefinition of class TWinControl`.  That is a consolidation decision
//  (which TWinControl becomes canonical) belonging to a wave that owns all
//  three headers -- none of which is forms/fIoSetView.h.  So: these four
//  method NAMES are recorded below (GATE table) with their golden span, but
//  no declaration for them exists in this file.  A future wave that either
//  consolidates TWinControl or deliberately picks one arm should add the
//  four declarations then, with that choice stated out loud.
//
//  `TCloseAction` (golden FormClose's second parameter) has the SAME
//  "no port" shape, already resolved by an earlier wave and reused verbatim
//  here: ATC/ATCInterface.h:47-54's own banner states "no port of
//  TCloseAction exists anywhere ... `FormClose(Sender,TCloseAction&)` becomes
//  `FormClose()` here" (ATC/ATCInterface.h:391 `void FormClose();`).  This
//  file's gated `FormClose()` declaration follows that established
//  precedent exactly (both TObject*Sender and TCloseAction&Action dropped),
//  rather than inventing a second answer to an already-answered question.
//
//  ============================================================================
//  BATCH CRITERION FOR THIS WAVE (why these 6 and not others)
//  ============================================================================
//  A method is ACTIVE here only if ALL of the following hold, each checked by
//  reading the FULL golden body (never by name):
//    1. It issues no machine action -- no MOT[]/cylinder/SW[]/TMySucker/
//       ADAM_DirectWriteData/motor-home call.
//    2. It persists nothing -- no WriteIniData, no fopen/CreateFile.
//    3. It sends nothing outbound -- no SECS/TCP/GPIB.
//    4. Every symbol it needs already exists in the port tree AND is
//       reachable from ht9045_forms's actual link set (vclcompat +
//       ht9045_globals + ht9045_core only -- ht9045_forms does NOT link
//       ht9045_sm, ht9045_motor, or ht9045_db).  Checked with a script that
//       finds each global's ACTUAL DEFINITION line (not just its `extern`
//       declaration) and cross-references CMakeLists.txt's per-target source
//       list, run 20260827:
//         FTestSuck/BTestSuck/OutArmSuck  -> DEFINED aHotPlateSubstrate.cpp,
//             which CMakeLists.txt:1432 lists inside `add_library(ht9045_sm
//             STATIC ...)`.  UNREACHABLE from ht9045_forms.  (This is also
//             explicitly named in this wave's own brief as a known landmine.)
//         HSys (IOTable[])                -> DEFINED database.cpp:101, listed
//             inside `add_library(ht9045_db ...)` (CMakeLists.txt:762).
//             UNREACHABLE from ht9045_forms.
//         TTLCfg / TestIF_File            -> DEFINED cprod.cpp:44 / :31,
//             listed inside `add_library(ht9045_globals STATIC ...)`
//             (CMakeLists.txt:470-474).  REACHABLE -- used by LabSiteMap.
//         _8Bit/_10Bit/_10BitPE/_10BitPO  -> DEFINED cmydef.cpp:217-223,
//             same ht9045_globals archive.  REACHABLE.
//         bPLCInData                       -> DEFINED MyLaneIo.cpp:101,
//             listed inside `add_library(ht9045_io STATIC ...)`
//             (CMakeLists.txt:963-965).  UNREACHABLE from ht9045_forms (see
//             the correction recorded at GATE X-15 -- an earlier pass of
//             this banner wrongly said "zero hits", scoped to too narrow a
//             grep).
//    5. Every symbol's definition line is NOT inside `#if 0` (checked with
//       census.py's gate_depth_map over cprod.cpp/cmydef.cpp at the exact
//       definition lines above -- all four are `live`, none gated).
//  Everything left is a machine action, a write, an outbound send, or reaches
//  a symbol that fails criterion 4 or 5.
//
//  TRANSITIVE RULE, applied strictly: a method that CALLS a gated method, or
//  that touches a widget type with zero port, is itself gated.  That is what
//  removes chkShowIndexAllClick (-> ShowSuckMode, itself gated on
//  FTestSuck/BTestSuck), cbbTypeChange / edtSearchIOChange (-> LoadIoTable,
//  gated on HSys), Timer2Timer (-> DoTTL_Spin, gated on SW[] hardware write),
//  TrackBarLightChange (-> SetLightZI0, gated on SW[] hardware write), and
//  every method touching a TBtnPanelLane/TMyLedLane/TALed/TBtnPanel/
//  THeader/TPage widget (see MISSING WIDGET TYPES below).
//
//  ============================================================================
//  MISSING WIDGET TYPES -- the other reason almost the whole file is gated
//  ============================================================================
//  golden's __published block (iosetview.h:22-2996, 2,929 declarations) is
//  built almost entirely from TWO widget classes this tree does not have:
//      TBtnPanelLane   -- every hardware-mapped manual switch button
//                          (btnSwFKPowerOff, btnC_Load_UpClick's own
//                          btnC_Load_Up, btnSwHeatGun, ... hundreds)
//      TMyLedLane      -- every hardware-mapped sensor/status LED
//                          (ledSnFKPowerOff, MyLedInShuttleNumR8, ... hundreds)
//  Also absent: `TBtnPanel` (butPa1.h), `THeader`, `TPage`, and `TALed`
//  (ALed.hpp -- the Safe-PLC LED bank type, aLedSPLC000..077).  Verified
//  20260827 by a comment-and-string-stripped scan of every .h/.hpp/.cpp under
//  this tree (excluding build*/.git/.svn/.pti_frames): `class TMyLedLane`,
//  `class TBtnPanelLane`, `class TBtnPanel`, `class THeader`, `class TPage` --
//  ZERO hits, all five.  `class TALed` -- ONE hit, mykitsuck.h, which is the
//  DISCARDED sibling header in the tree's own documented TMyKitSuck ODR trap
//  (docs/KNOWLEDGE.md `ht9045-v906-two-tmykitsuck-odr-trap`; mykitsuck.cpp is
//  not registered in any CMake target, so even if TALed's declaration were
//  usable, nothing defines its methods).  `class TMyLed` (singular, no
//  "Lane") DOES exist and IS real (ATC/ATCInterface.h:208) -- but golden's
//  Timer1Timer-driven ScanLed walks TMyLedLane, not the singular TMyLed, so
//  that one real type does not rescue any delivered function this wave.
//  Porting BtnPanelLane.h / MyLedLane.h / butPa1.h / ALed.hpp is its own
//  wave; inventing facade-local minimal stand-ins for widget classes this
//  large and this widely reused (unlike forms/fContact.h's D-4, which stood
//  in for ONE or TWO specific missing PROPERTIES on an otherwise-real type)
//  would pre-empt that wave's design and is out of scope here.
//
//  ============================================================================
//  WAVE SCOPE -- ACTIVE (5).  Each was read line-by-line in full first.
//  ============================================================================
//    Tfiosetview ctor          golden :49-91     PORT SPLIT -- fields only,
//                                                 golden's body (widget-array
//                                                 wiring + LoadIoTable(0,0,0)
//                                                 call) NOT translated this
//                                                 wave -- see DEVIATION D-2.
//    ResetIndexSuck            golden :1861-1869 ACTIVE (touches only own
//                                                 bIndexSuck[2][4][8] member)
//    ResetIndexDestroy         golden :1871-1879 ACTIVE (touches only own
//                                                 bIndexDestroy[2][4][8] member)
//    LabSiteMap                golden :2861-2983 ACTIVE (TTLCfg/TestIF_File,
//                                                 16 TLabel widgets, no HW)
//    strngrdIoTableSelectCell  golden :3143-3148 ACTIVE (stores ACol/ARow into
//                                                 own iSelectCol/iSelectRow)
//
//  ============================================================================
//  GATED -- MACHINE ACTION / HARDWARE WRITE (16)
//  ============================================================================
//   (S-01) Timer1Timer        golden :93-214    LockIndexMotorAndDoHomeProcess()
//       (forces an index-motor home/lock cycle) + mySuckerTemp->Suck()/
//       Destroy()/Sensor()/Error (manual sucker toggle) + ADAM_ReadPA/
//       ADAM_ReadVoltage (analog HW reads feeding this poll) + calls
//       ProcessIndexSuckDestroy1/2 (S-08/S-09) and RefreshSafePLCLed (X-15).
//   (S-02) FormClose           golden :216-234   Close_ADAM_6024()/
//       Open_ADAM_6024() (HW connection reset) + ResetIndexDestroy() call is
//       fine on its own (ACTIVE above) but Cylinder[C_Shuttle_Knocker_1/2]
//       .Off() is a real cylinder actuation.
//   (S-03) FormShow            golden :236-1023  788 lines, NOT read
//       line-by-line this wave -- gated on size/risk alone, before any
//       content review; see WHAT REMAINS / NOT DONE THIS WAVE at the end of
//       this file.  forms/fContact.h has an analogous precedent (its own
//       FormShow, 724 lines, gated X-08) but that is a DIFFERENT golden
//       method in a different file, cited for the pattern only -- this
//       gate is NOT inherited from it.
//   (S-04) DoTTL_Spin          golden :1294-1311 SW[SwStart0/1].On()/Off() --
//       pulses the tester's TTL start lines.
//   (S-05) Timer2Timer         golden :1275-1279 -> DoTTL_Spin (S-04),
//       transitively gated (DoColorSensor_Spin half is independently gated,
//       see X-14).
//   (S-06) btnAllVacuumClick   golden :1831-1859 arms bIndexSuck/bIndexDestroy
//       for FOUR whole shuttle grids at once (Click; also arms S-08/S-09).
//   (S-07) ContactIndexSuckDestroy1 golden :1964-2023 FTestSuck/BTestSuck
//       .Suck()/.Destroy() -- direct vacuum actuation (+ ht9045_sm boundary,
//       see LINK BOUNDARY).
//   (S-08) ProcessIndexSuckDestroy1 golden :1922-1962 ditto (front grid).
//   (S-09) ProcessIndexSuckDestroy2 golden :1881-1920 ditto (rear grid).
//   (S-10) CheckIndexSuck      golden :2805-2822 FTestSuck/BTestSuck.Suck[i][j]
//       .Normal() -- resets sucker hardware state (+ ht9045_sm boundary).
//   (S-11) tbarIndexEPChange   golden :1800-1806 ADAM_DirectWriteData -- EP
//       analog output write.
//   (S-11) tbarLoaderEPChange  golden :1808-1824 ditto.
//   (S-11) tbarDieForceChange  golden :1826-1829 ditto.
//   (S-11) tbarIndexEP2Change  golden :3965-3968 ditto.
//   (S-12) SetLightZI0         golden :4101-4122 SW[SwPRGSEL0..6].On()/Off()
//       -- light-position hardware output.
//   (S-12) TrackBarLightChange golden :4094-4099 -> SetLightZI0 (S-12).
//
//  ============================================================================
//  GATED -- LINK BOUNDARY (ht9045_forms cannot reach ht9045_sm/ht9045_db) (6)
//  ============================================================================
//  ht9045_forms links ONLY vclcompat + ht9045_globals + ht9045_core
//  (CMakeLists.txt:714-721).  These read real machine/IO state but the
//  storage lives outside that link set -- see BATCH CRITERION 4 for how each
//  was traced to its actual DEFINING .cpp and CMake target:
//   (L-01) ShowSuckMode        golden :1313-1669 (357L) FTestSuck.iShtRow/
//       iShtCol/.Suck[i][j].iMyRow/iMyCol, BTestSuck (ditto), OutArmSuck
//       .iPickRow/iPickCol -- all three defined in aHotPlateSubstrate.cpp
//       (ht9045_sm).  Otherwise this method is 100% TPanel (Index_8/16,
//       InArm_8/16, OutArm_8/16, palArm1_Ab/palArm2_Ab) and would have been
//       the wave's second-largest ACTIVE delivery; recorded here in enough
//       detail that a wave which already owns an ht9045_sm-reaching edge for
//       forms/fIoSetView.cpp can un-gate it directly.
//   (L-02) ShowShuttleSensor   golden :1671-1758 SThreadPara/FLCarryKit/Sen[]
//       (need re-verification of their own link boundary) PLUS
//       MyLedInShuttleNumR8/R9/F8/F9 are TMyLedLane (missing widget type,
//       see above) -- gated on BOTH grounds independently.
//   (L-03) LoadIoTable         golden :2985-3116 HSys.IOTable[] -- defined
//       database.cpp:101, ht9045_db target.  (Also called from golden's own
//       ctor -- see DEVIATION D-2 for why that call is not translated.)
//   (L-04) cbbTypeChange       golden :3118-3121 -> LoadIoTable (L-03).
//   (L-05) edtSearchIOChange   golden :3773-3880 HSys.IOTable[] directly
//       (108L, same shape as LoadIoTable) + falls back to LoadIoTable (L-03)
//       when the search box is cleared.
//   (L-06) btnModifyClick      golden :3123-3141 reads/writes
//       strngrdIoTable->Cells (populated only by the gated L-03/L-05 pair,
//       so a delivered btnModifyClick would silently operate on an
//       always-empty grid) + it is also *Click (S-category by name alone).
//       (RefreshSafePLCLed is NOT listed here despite also reading
//       machine-adjacent state -- its blocker is a genuinely MISSING global,
//       not an out-of-reach one, so it is counted once, under X-15, below.)
//
//  ============================================================================
//  GATED -- MISSING GLOBAL / MISSING METHOD / MISSING PROPERTY (3)
//  ============================================================================
//   (X-14) DoColorSensor_Spin  golden :1281-1292 calls
//       `fTrayForm->GetColorSensor("ColorSensor_FT")->GetColorName(iColorNum)`.
//       `fTrayForm` IS a real port global (forms/fTrayForm.h:48) and
//       `IsEnableColorSensor()` IS a real ported method (returns hard-coded
//       `false`, forms/fTrayForm.cpp:10 -- "offline: no MU-N colour sensor
//       installed", meaning golden's `if` branch that reaches GetColorSensor
//       is dead in this port's current runtime).  But `GetColorSensor` has NO
//       member on TfTrayForm at all -- verified by reading forms/fTrayForm.h
//       in full.  Per rule 4 ("no shim invented for a cross-file gap"), this
//       wave does not add a fake GetColorSensor/GetColorName pair; the
//       method is gated on the missing symbol, not translated with the dead
//       branch silently dropped (dropping it would be an behaviour decision,
//       not a translation).
//   (X-15) RefreshSafePLCLed   golden :3970-4043 references the global
//       `bPLCInData[...]`.  ⚠ CORRECTED 20260827 -- an EARLIER PASS of this
//       banner claimed "ZERO hits tree-wide", scoped only to a grep of
//       cmydef.h/cprod.h.  A wave-close, whole-tree re-run (per task rule
//       "absence-claim 附指令與時間，收工重跑") found it IS real:
//       `bool bPLCInData[INPUT_MAX_Slave][INPUT_MAX_REGISTER][8] = {};`
//       (MyLaneIo.cpp:101, `extern` re-declared MyPLC/MyPLC_IO_Modbus.h:46).
//       That correction does NOT change the gate outcome, for TWO independent
//       reasons, each sufficient alone: (1) MyLaneIo.cpp is listed inside
//       `add_library(ht9045_io STATIC ...)` (CMakeLists.txt:963-965) --
//       `ht9045_io` is NOT in ht9045_forms's link set (LINK BOUNDARY, same
//       shape as L-01..L-06); (2) every one of its 64 write targets
//       (aLedSPLC000..077) is `TALed*` (see MISSING WIDGET TYPES) regardless
//       of where bPLCInData lives.  Kept as a single X-category entry (not
//       split into L + missing-widget) because the widget-type absence alone
//       already fully blocks it -- see the wrong-then-fixed claim as the
//       reason this file names its correction instead of silently editing it
//       away (docs/KNOWLEDGE.md `agent-justifications-fail-more-than-code`).
//   (X-16) edtPulseDelayMouseDown golden :3887-3905 reads `Ptr->Name` where
//       `Ptr` is a `TEdit*` (golden `TObject *Sender` cast via
//       `(TEdit*)Sender`).  `g++ -fsyntax-only` (20260827) rejects it:
//       `error: 'class vclcompat::TEdit' has no member named 'Name'`.
//       ⚠ CITATION TRAP: `vclcompat/Controls.h:183` DOES contain the text
//       `AnsiString Name;` -- but that line is inside `class TFont` (golden
//       Graphics.hpp TFont, :180-189), not TControl (:213 onward, which
//       carries only Visible/Enabled/hCtl/Tag).  A prior wave's citation
//       (forms/fContact.h D-10, "TControl::Name, Controls.h:183") pointed at
//       this exact line for a DIFFERENT, GATED method and was never
//       compiler-checked because gated methods have no body.  This wave's
//       attempt to actually DEFINE a `->Name`-reading method is what caught
//       it.  Recorded so nobody re-trusts that line number for `TControl::
//       Name` again; TControl/TEdit/TCustomEdit have no `Name` property
//       anywhere in this tree (grepped 20260827, `\bName\b` over
//       vclcompat/Controls.h -- exactly 2 hits: the comment at :171 and the
//       TFont field at :183).  Otherwise this method is fully deliverable
//       (fQwertyKey->ShowQwertyKey exists, same ht9045_forms archive,
//       N_INTEGER/N_NO_SYMBOL both live in ht9045_globals) -- it is gated on
//       this ONE missing property alone.
//
//  ============================================================================
//  GATED -- *Click / MISSING WIDGET TYPE, undifferentiated (remaining 31)
//  ============================================================================
//  golden requires "所有 *Click event handler 必須退出" unconditionally, and
//  separately every method below touches at least one TBtnPanelLane /
//  TMyLedLane / TBtnPanel / THeader / TPage widget (verified by reading the
//  golden body AND cross-checking each touched member's declared type in
//  iosetview.h).  Where a method is gated for BOTH reasons independently,
//  only the type-absence citation is given (it is the harder blocker of the
//  two -- *Click could in principle be re-decided by a future wave's
//  write-path design, a missing widget type cannot).  btnModifyClick is
//  documented under LINK BOUNDARY (L-06) and NOT re-counted here; the four
//  TWinControl-parameter methods (SetCompomentIO/SetCompomentHint/
//  SetPanelToEnable/ScanLed) are OMITTED, not gated, and are documented under
//  ABSENT PARAMETER TYPE and NOT counted in this bucket either -- they are
//  cross-referenced below only where SetPanelElable transitively touches one:
//    btnToolClick(golden :1025-1028) BtnPanelClick(:1030-1250, 221L, walks
//      the whole panel tree) btnReloadClick(:1252-1266) btnTTLTestClick
//      (:1268-1273) chkShowIndexAllClick(:1760-1766, -> ShowSuckMode L-01)
//      BtnNumPanelTestClick(:1768-1798, TBtnPanelLane Sender cast)
//      btnLatchCheckClick(:2025-2028, fLtcSensor->ShowModal()) btIPSettingClick
//      (:2824-2829) btComPoetClick(:2831-2836) btKVMClick(:2838-2859)
//      strngrdIoTableDblClick(:3150-3153, -> btnModify->Click()) btnAddIOClick
//      (:3155-3164) btnDeleteIOClick(:3166-3181) sbUpdateClick(:3183-3321,
//      139L, writes the IO table -- also GATE by rule "寫檔／持久化")
//      Label120Click(:3323-3326) btnCloseClick(:3328-3333) btnC_Load_UpClick
//      (:3335-3379, 45L, TBtnPanelLane Cylinder[C_Load_Up] actuation --
//      WRITES GLOBAL MACHINE STATE, see WRITES GLOBAL STATE below)
//      btnC_Empty_UpClick(:3381-3424) btnC_Color_UpClick(:3426-3470)
//      btnC_Auto1_UpClick(:3472-3516) btnC_Auto2_UpClick(:3518-3562)
//      btnC_Auto3_UpClick(:3564-3633, 70L) btnC_Auto4_SelectorClick
//      (:3635-3679) btnC_Auto5_SelectorClick(:3681-3725) btnC_Auto6_SelectorClick
//      (:3727-3771) sb_IO_CommunicationPadClick(:3882-3885,
//      fPadInterface->ShowModal()) btn500KPAClick(:4045-4048)
//      btnAllLockClick(:4050-4069) btnAllUnLockClick(:4072-4091)
//      SetPanelElable(:2073-2083, -> SetPanelToEnable [OMITTED, see ABSENT
//      PARAMETER TYPE]; ALSO reads global `fiosetview` under `#ifdef
//      SOFT_SIMULTE`, which this port would rewrite as `this` per the
//      no-global-pointer deviation above -- moot while SetPanelToEnable
//      itself is omitted) Hide_1032_IO(:3907-3963, 57L, TBtnPanelLane/
//      TMyLedLane Floodgate widgets).
//  That is 31 named above.  Arithmetic check against the golden total: 5
//  ACTIVE + 16 machine-action + 6 link-boundary + 3 missing-global/method/
//  property + 4 omitted (TWinControl parameter) + 31 here = 65.
//
//  ⚠ WRITES GLOBAL MACHINE STATE (flagged per task rule, even though gated):
//  btnAllVacuumClick, btnC_Load_UpClick, btnC_Empty_UpClick,
//  btnC_Color_UpClick, btnC_Auto1..6_UpClick/SelectorClick all write the
//  tree-wide Cylinder[]/bIndexSuck/bIndexDestroy machine-state arrays that
//  drive real hardware on the next Timer1Timer/ProcessIndexSuckDestroy tick.
//  ResetIndexSuck/ResetIndexDestroy (ACTIVE above) only ever WRITE THOSE
//  ARRAYS TO false (the disarm direction) and read/write no other global --
//  that asymmetry is why they are ACTIVE while every arming call site stays
//  gated.
//
//  ============================================================================
//  GOLDEN NOTES (faithful; recorded, NOT "fixed")
//  ============================================================================
//  (G-1) LabSiteMap's golden switch (:2891-2981) has NO `default:` case.  Any
//      TestIF_File.iTestMode outside the eleven listed values leaves every
//      lblTTL_*/lblTTLIn* Caption at whatever it held before the call (on a
//      freshly-constructed facade, the ctor's default "" from TLabel's own
//      default AnsiString).  Verbatim; do not add a default.
//  (G-2) LabSiteMap's `_32Site4X8N`/`_32Site4X8M` arms (:2961-2980) assign the
//      literal string "Aa" to EVERY ONE of the 8 lblTTL_* labels (not "Ab",
//      "Ac", ... -- golden really does write the same 2-character literal to
//      all eight).  Looks like a copy-paste artefact but is translated
//      verbatim per task rule ("golden 不合理處照翻並在註解說明").  Note also
//      that this arm does NOT touch lblTTLInAa..Bd at all (only the
//      non-"In" set) -- also verbatim.
//  (G-3) LabSiteMap's `_6Site2X3` arm indexes `asString1` (9-entry: "Aa","Ab",
//      "Ac","Ba","Bb","Bc","X","X","X") with the SAME `iCH[i][j]-1`-style
//      indices computed for the 8-entry `asString` table used by the first
//      arm, and cross-maps i/j in a non-obvious order
//      (lblTTL_Ab=asString1[iCH[1][0]], lblTTL_Ba=asString1[iCH[0][2]], ...).
//      Translated verbatim; the cross-map is golden's own site-numbering
//      convention for the 2x3 layout, not re-derived here.
//  (G-4) ResetIndexSuck/ResetIndexDestroy have no bounds/enable guard --
//      they unconditionally zero every [2][4][8] slot regardless of
//      bIndexSuck's real dimensionality vs FTestSuck/BTestSuck's actual
//      iShtRow/iShtCol (which may be smaller than 4x8 on non-32-site
//      configs).  Verbatim; golden does the same over-write.
//
//  ============================================================================
//  DEVIATIONS (port-only; each with why)
//  ============================================================================
//  (D-1) `__fastcall` dropped from every signature -- tree-wide convention.
//  (D-2) CTOR carries FIELDS ONLY (task rule 5, "ctor 只塞欄位" -- SIOF/
//      static-init trap, docs/KNOWLEDGE.md 88-SEGFAULT precedent).  UNLIKE
//      forms/fContact.h's ctor/Init() split, this wave does NOT add an
//      `Init()` holding golden's ctor body, because that body's only
//      substantive content -- the Index_8/16/InArm_8/16/OutArm_8/16 array
//      wiring and the `btnSwHeatGun/btnSwCDAGun->Visible=` lines -- is used
//      exclusively by ShowSuckMode (L-01, gated on the ht9045_sm boundary)
//      and touches TBtnPanelLane (missing type), and the ctor's own
//      `LoadIoTable(0,0,0);` call is independently gated (L-03, ht9045_db
//      boundary).  Translating an Init() for logic that only feeds gated
//      consumers would be dead weight this wave; the widget-array members
//      themselves (Index_8/16/InArm_8/16/OutArm_8/16, palArm1_Ab/palArm2_Ab)
//      are therefore also NOT declared here -- add them in the wave that
//      delivers ShowSuckMode.
//  (D-3) The 16 TLabel widgets LabSiteMap touches are OWNING raw pointers
//      with in-class `new TLabel()` initialisers (forms/fContact.h D-3
//      precedent) and are freed by a port-only destructor.  Golden's
//      Tfiosetview has NO explicit destructor (TForm's is inherited); this
//      file's `~Tfiosetview()` is a pure port addition, not a translation.
//      Copy/assign are `= delete` so a double-free is a compile error.
//  (D-4) `Tfiosetview` has NO base class here (matches forms/fContact.h,
//      NOT `: public TForm` -- there is no facade-side TForm consumed by
//      any delivered method, and adding one only to satisfy an unused base
//      would pull in the handlerlog.h/language.h/Public/HTEdit.h TWinControl
//      three-way collision discussed above for zero behavioural gain).
//  (D-5) `TObject *Sender` parameters are kept on every delivered handler
//      signature (golden signature, vclcompat::TObject exists).  Handlers
//      are NOT wired to anything -- task rule 5; vclcompat widgets carry no
//      OnClick/OnMouseDown delegate slot (tree-wide idiom, forms/fContact.h
//      D-10 precedent).
//  (D-6) `bIndexSuck[2][4][8]` / `bIndexDestroy[2][4][8]` default to all-false
//      via `= {}` in-class initialisation.  Golden's ctor does not
//      explicitly zero them, but BCB6/VCL allocates TForm-derived instances
//      via GetMem+zero-fill before any field initialiser runs, so an
//      unexplained-in-source-but-actually-zero start state is the faithful
//      default (same reasoning as forms/fContact.h's treatment of
//      unexplicitly-initialised scalar members).
//  (D-7) `FormClose()` (GATED, no body) takes NO parameters -- follows
//      ATC/ATCInterface.h:391's already-established precedent for the
//      identical "golden takes TCloseAction&, this tree has no port of
//      TCloseAction" situation, rather than re-deciding it here.
//
//  ============================================================================
//  ABSENCE RE-RUN AT WAVE CLOSE -- 20260827
//  ============================================================================
//  Absence claims go stale (docs/KNOWLEDGE.md) and this repo runs parallel
//  waves.  Re-run immediately before writing this banner's final text:
//    `class TfiosetviewShim` / `TfiosetviewShim *fiosetview` --
//        atester_shims.h:348/:354, atester_shims.cpp:380 -- STILL the only
//        owner of the bare name `fiosetview`.  This file still declares none.
//    `class Tfiosetview` (outside this file) -- 0 hits.
//    `class TWinControl` -- still exactly 3 (handlerlog.h, language.h,
//        Public/HTEdit.h), still each a distinct definition.
//    `class TBtnPanelLane` / `TMyLedLane` / `TBtnPanel` / `THeader` / `TPage`
//        -- still 0 each.
//    `bPLCInData` -- REAL (MyLaneIo.cpp:101, ht9045_io target); see the
//        correction at GATE X-15 -- this file's own EARLIER claim of "zero
//        hits" was wrong (too-narrow grep) and is corrected in place there,
//        not silently edited away.  The gate outcome (RefreshSafePLCLed
//        stays gated) is unchanged: bPLCInData is unreachable from
//        ht9045_forms regardless, and TALed (next line) is still absent.
//    `TALed` -- still 1 (mykitsuck.h only, still not registered in any
//        CMake target).
//  Method: comment-and-string-stripped scan (the same character-state-machine
//  approach as forms/fContact.h's re-runs), corpus = all .h/.hpp/.c/.cpp
//  under HT9011UC_Cpp_V3.33.906.0 excluding build*/.git/.svn/.pti_frames/web.
//  This wave's own re-run (20260827, unscoped broad grep, NOT comment-
//  stripped) is what caught the bPLCInData error above -- recorded so the
//  method note above is not read as "and therefore infallible".
//
//  ============================================================================
//  SYMBOLS THIS TU NEEDS (for the CMakeLists landing decision -- NOT made here)
//  ============================================================================
//  This wave may not touch CMakeLists.txt.
//    vclcompat        -- AnsiString / TObject / Controls.h (TLabel; TEdit
//                        appears only in a GATED signature, no definition
//                        needs it) / ShiftState.h (TMouseButton/TShiftState,
//                        likewise only in a GATED signature)
//    ht9045_globals   -- TTLCfg, TestIF_File (cprod.cpp); _8Bit/_10Bit/
//                        _10BitPE/_10BitPO, eTestMode's SingleSite/DualSite/
//                        TriSite1X3/QualSite1X4/DualSite2x1/QualSite2X2/
//                        _8Site1X4/_6Site2X3/_32Site4X8N/_32Site4X8M
//                        (MachineType.h, header-only enum, no link needed)
//  Expected landing: ht9045_forms (same layer as forms/fContact.cpp,
//  forms/fMotorTest.cpp -- vclcompat + ht9045_globals + ht9045_core only).
//  Introduces NO new archive edge: every reachable symbol above already
//  lives in ht9045_globals or ht9045_forms itself.
//
//  ============================================================================
//  WHAT REMAINS / NOT DONE THIS WAVE (explicit, so scope-narrowing is not
//  silently absorbed -- task rule 9)
//  ============================================================================
//  - FormShow (788L) and BtnPanelClick (221L) were NOT read line-by-line.
//    Both are *Click-shaped or FormShow-shaped (already excluded categories
//    by name), so reading them would not have changed this wave's ACTIVE
//    set -- but that means their internal machine-action surface, if any
//    beyond what the name implies, is UNCATALOGUED.  Do not treat their
//    absence from the S/L/X tables above as "read and found clean"; they
//    were never read.
//  - sbUpdateClick (139L, writes the IO table) was read only far enough to
//    confirm it persists data (rule "寫檔／持久化") and was not further
//    analysed for machine-action content.
//  - ShowShuttleSensor's SThreadPara/FLCarryKit/Sen[] link-boundary claim
//    (L-02) is NOT independently traced to a defining .cpp/CMake target the
//    way FTestSuck/BTestSuck/OutArmSuck/HSys/bPLCInData were -- it is gated
//    on the MyLedInShuttleNumR8/R9/F8/F9 (TMyLedLane) widget-type absence
//    alone, which is sufficient by itself, so the second ground was left
//    unverified rather than spending the budget on a gate with only one
//    outcome either way.  A future wave un-gating this needs to check it.
//  - The ~2,929-member golden __published widget block was not enumerated
//    or cross-checked beyond the ~20 names this wave's 5 ACTIVE methods
//    actually touch.  No claim is made about the other ~2,900+ widgets'
//    types, .dfm-sourced default values, or whether any of them individually
//    have a usable port -- only the SIX widget classes named in MISSING
//    WIDGET TYPES were checked, because those are the ones the read
//    functions actually needed.
//  - No attempt was made to consolidate the TWinControl three-way collision,
//    port BtnPanelLane.h/MyLedLane.h/butPa1.h/ALed.hpp, or resolve the
//    TMyKitSuck-style aHotPlateSubstrate.h-vs-mykitsuck.h fork for THIS
//    file's own dependencies (ShowSuckMode did not end up needing that
//    choice made, since it is gated on the ht9045_sm link boundary before
//    the header-choice question would even arise) -- all four are follow-up
//    waves' work, named here so they are not rediscovered from scratch.
//  - No build/ctest was run.  Verification this wave is `g++ -fsyntax-only`
//    on both files, a full `-c` compile of forms/fIoSetView.cpp to a .o, and
//    `nm --undefined-only -C` / `nm --defined-only -C` over that .o (see the
//    delivery report for the exact symbol lists) -- NOT a cmake configure/
//    build, which this wave's task rules explicitly say not to run.
// =============================================================================
#ifndef FORMS_FIOSETVIEW_H
#define FORMS_FIOSETVIEW_H

#include "vclcompat/vcl_compat.h"    // AnsiString / TObject at global scope
#include "vclcompat/Controls.h"      // TLabel / TEdit
#include "vclcompat/ShiftState.h"    // TMouseButton / TShiftState

// ===========================================================================
//  Tfiosetview -- non-VCL facade (golden iosetview.h:20-3039,
//  `class Tfiosetview : public TForm`; NO base class here, see DEVIATION D-4).
// ===========================================================================
class Tfiosetview
{
public:
    Tfiosetview();
    ~Tfiosetview();
    Tfiosetview(const Tfiosetview&) = delete;
    Tfiosetview& operator=(const Tfiosetview&) = delete;

    // -- golden non-widget data members, iosetview.h:3000-3019/:3031 --------
    // (Index_8/16/InArm_8/16/OutArm_8/16 widget-array members are NOT
    // declared this wave -- see DEVIATION D-2.)
    bool fShow = false;                                                        // golden :3011
    bool bOutDataChange = false;                                               // golden :3012
    bool bIndexSuck[2][4][8] = {};                                             // golden :3014, see DEVIATION D-6
    bool bIndexDestroy[2][4][8] = {};                                          // golden :3015, see DEVIATION D-6
    int  bChangeSuckStatus = 0;                                                // golden :3000 (Timer1Timer-only; DEVIATION D-6 zero-fill reasoning)
    int  iSelectRow = 0;                                                       // golden :3003
    int  iSelectCol = 0;                                                       // golden :3004
    int  iDragCol = 0;                                                         // golden :3005
    int  iDragRow = 0;                                                         // golden :3006

    // -- golden .dfm widgets touched by THIS WAVE's delivered LabSiteMap
    //    only (golden iosetview.h widget block, TLabel throughout) ----------
    TLabel *lblTTL_Aa   = new TLabel();
    TLabel *lblTTL_Ab   = new TLabel();
    TLabel *lblTTL_Ac   = new TLabel();
    TLabel *lblTTL_Ad   = new TLabel();
    TLabel *lblTTL_Ba   = new TLabel();
    TLabel *lblTTL_Bb   = new TLabel();
    TLabel *lblTTL_Bc   = new TLabel();
    TLabel *lblTTL_Bd   = new TLabel();
    TLabel *lblTTLInAa  = new TLabel();
    TLabel *lblTTLInAb  = new TLabel();
    TLabel *lblTTLInAc  = new TLabel();
    TLabel *lblTTLInAd  = new TLabel();
    TLabel *lblTTLInBa  = new TLabel();
    TLabel *lblTTLInBb  = new TLabel();
    TLabel *lblTTLInBc  = new TLabel();
    TLabel *lblTTLInBd  = new TLabel();

    // -- ACTIVE this wave (defined in forms/fIoSetView.cpp) -----------------
    void ResetIndexSuck();                                                     // golden :1861-1869
    void ResetIndexDestroy();                                                  // golden :1871-1879
    void LabSiteMap();                                                         // golden :2861-2983
    void strngrdIoTableSelectCell(TObject *Sender, int ACol, int ARow,
                                   bool &CanSelect);                            // golden :3143-3148

    // -- GATED this wave -- declared, NOT defined.  Calling one of these is
    //    a LINK ERROR by design (see safety posture banner above). ---------
    void Timer1Timer(TObject *Sender);                                         // golden :93-214    GATE (S-01)
    void FormClose();                                                          // golden :216-234   GATE (S-02); see DEVIATION D-7
    void FormShow(TObject *Sender);                                            // golden :236-1023  GATE (S-03)
    void btnToolClick(TObject *Sender);                                        // golden :1025-1028
    void BtnPanelClick(TObject *Sender);                                       // golden :1030-1250  ⚠ AI(W906-IOWEB-P17) 20260925: STILL undefined here (the linker interlock stands for THIS member), but its TBtnPanelLane/ePCI1203 arm is now served to the web by the free function W906_IoBtnPanelClick (JsonBridge/IoBtnPanelClick.cpp, command io.btnPanelClick) -- read that file's header for the interlock story; do not assume the web cannot issue it
    void btnReloadClick(TObject *Sender);                                      // golden :1252-1266
    void btnTTLTestClick(TObject *Sender);                                     // golden :1268-1273
    void Timer2Timer(TObject *Sender);                                         // golden :1275-1279 GATE (S-05)
    void DoColorSensor_Spin();                                                 // golden :1281-1292 GATE (X-14)
    void DoTTL_Spin();                                                         // golden :1294-1311 GATE (S-04)
    void ShowSuckMode(int iSel);                                               // golden :1313-1669 GATE (L-01)
    void ShowShuttleSensor();                                                  // golden :1671-1758 GATE (L-02)
    void chkShowIndexAllClick(TObject *Sender);                                // golden :1760-1766
    void BtnNumPanelTestClick(TObject *Sender);                                // golden :1768-1798
    void edtPulseDelayMouseDown(TObject *Sender, TMouseButton Button,
                                 TShiftState Shift, int X, int Y);              // golden :3887-3905 GATE (X-16)
    void tbarIndexEPChange(TObject *Sender);                                   // golden :1800-1806 GATE (S-11)
    void tbarLoaderEPChange(TObject *Sender);                                  // golden :1808-1824 GATE (S-11)
    void tbarDieForceChange(TObject *Sender);                                  // golden :1826-1829 GATE (S-11)
    void btnAllVacuumClick(TObject *Sender);                                   // golden :1831-1859 GATE (S-06)
    bool ProcessIndexSuckDestroy1(int iType = 0);                              // golden :1922-1962 GATE (S-08)
    bool ProcessIndexSuckDestroy2(int iType = 0);                              // golden :1881-1920 GATE (S-09)
    bool ContactIndexSuckDestroy1(int iArm, int iCol, int iRow);               // golden :1964-2023 GATE (S-07)
    void btnLatchCheckClick(TObject *Sender);                                  // golden :2025-2028
    void SetPanelElable();                                                     // golden :2073-2083
    void CheckIndexSuck();                                                     // golden :2805-2822 GATE (S-10)
    void btIPSettingClick(TObject *Sender);                                    // golden :2824-2829
    void btComPoetClick(TObject *Sender);                                      // golden :2831-2836
    void btKVMClick(TObject *Sender);                                          // golden :2838-2859
    void LoadIoTable(int iType = 0, int iLane = 0, int iIP = 0);               // golden :2985-3116 GATE (L-03)
    void cbbTypeChange(TObject *Sender);                                       // golden :3118-3121 GATE (L-04)
    void btnModifyClick(TObject *Sender);                                      // golden :3123-3141 GATE (L-06)
    void strngrdIoTableDblClick(TObject *Sender);                              // golden :3150-3153
    void btnAddIOClick(TObject *Sender);                                       // golden :3155-3164
    void btnDeleteIOClick(TObject *Sender);                                    // golden :3166-3181
    void sbUpdateClick(TObject *Sender);                                       // golden :3183-3321
    void Label120Click(TObject *Sender);                                       // golden :3323-3326
    void btnCloseClick(TObject *Sender);                                       // golden :3328-3333
    void btnC_Load_UpClick(TObject *Sender);                                   // golden :3335-3379
    void btnC_Empty_UpClick(TObject *Sender);                                  // golden :3381-3424
    void btnC_Color_UpClick(TObject *Sender);                                  // golden :3426-3470
    void btnC_Auto1_UpClick(TObject *Sender);                                  // golden :3472-3516
    void btnC_Auto2_UpClick(TObject *Sender);                                  // golden :3518-3562
    void btnC_Auto3_UpClick(TObject *Sender);                                  // golden :3564-3633
    void btnC_Auto4_SelectorClick(TObject *Sender);                            // golden :3635-3679
    void btnC_Auto5_SelectorClick(TObject *Sender);                            // golden :3681-3725
    void btnC_Auto6_SelectorClick(TObject *Sender);                            // golden :3727-3771
    void edtSearchIOChange(TObject *Sender);                                   // golden :3773-3880 GATE (L-05)
    void sb_IO_CommunicationPadClick(TObject *Sender);                         // golden :3882-3885
    void Hide_1032_IO();                                                       // golden :3907-3963
    void tbarIndexEP2Change(TObject *Sender);                                  // golden :3965-3968 GATE (S-11)
    void RefreshSafePLCLed();                                                  // golden :3970-4043 GATE (X-15)
    void btn500KPAClick(TObject *Sender);                                      // golden :4045-4048
    void btnAllLockClick(TObject *Sender);                                     // golden :4050-4069
    void btnAllUnLockClick(TObject *Sender);                                   // golden :4072-4091
    void TrackBarLightChange(TObject *Sender);                                 // golden :4094-4099 GATE (S-12)
    void SetLightZI0(int iPos);                                                // golden :4101-4122 GATE (S-12)

    // -- OMITTED entirely, not declared (see ABSENT PARAMETER TYPE above) --
    //    void SetCompomentIO(TWinControl *PCtrl);       golden :2085-2669
    //    void SetCompomentHint(TWinControl *PCtrl);      golden :2671-2739
    //    void SetPanelToEnable(TWinControl *PCtrl);      golden :2030-2071
    //    void ScanLed(TWinControl *PCtrl);                golden :2741-2803
};

#endif // FORMS_FIOSETVIEW_H
