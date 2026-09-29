// =============================================================================
//  SECSGEM/uHGemHT9045_SV.cpp  --  HT9045's SECS/GEM Status-Variable (SV)
//                                  registration table
//
//  Golden source: HT9011UC_Code_V3.33.906.0_20260618/SECSGEM/uHGemHT9045_SV.cpp
//                 (1014 lines / 935 code lines, cp950)
//  Translation wave: PT-W4 "uHGemHT9045_SV"
//  Translator: AI(W906-PT-W4) 20260808
//
//  ROLE
//  ----
//  ONE function, no header of its own.  `HT9045Gem::AddSV()` is the AP-side
//  bulk registration pass that tells the SECS/GEM engine which live process
//  variable backs each Status Variable ID a host may query: 877 consecutive
//  `HGemPtr->SetSVDataPointer(SVID, HType.<fmt>, name, unit, &target, remark)`
//  calls, SVID 1000 through 65095, all inside one
//  `if(CosFunction.bEnable_SECS_GEM==true)` guard, with exactly one branch in
//  the whole body (golden :307/:318, `iATC_Use_Heat_Count<=4` vs `<=8`, which
//  picks a 4-head or an 8-head ATC temperature SVID map -- and registers NO ATC
//  temperature SVs at all when the count exceeds 8, golden having no trailing
//  `else`).  It registers POINTERS only: it reads no hardware, writes no state,
//  calls no other engine, and returns void.  Golden's sole call site is
//  SECSGEM/UsecegemMainFrom.cpp:202 (`HSys.MyGem->AddSV();`), a form that is
//  itself untranslated; nothing in this port calls it yet.
//
//  The engine side is ALREADY PORTED and is NOT re-implemented here:
//  SECSGEM/SecsSvEcRegistration.{h,cpp} (class `SecsSvEcRegistration`, the
//  SV/EC bookkeeping slice of THGem) carries all four SetSVDataPointer
//  overloads.  That header's own "INTEGRATE-AGENT WIRING POINT" note asked
//  that "the eventual _SV.cpp/_EC.cpp translation wave should call through a
//  SecsSvEcRegistration instance (or a THGem that embeds one) rather than a
//  bespoke registration API" -- this file does exactly that, via THGem's
//  by-value `SvEcReg` member (uHGemEquipment.h:753).
//
//  WAVE SCOPE  (golden has exactly one function)
//  --------------------------------------------
//    HT9045Gem::AddSV()            golden :55-1012   ACTIVE, body-complete but
//                                  under GATE [G1]'s free-function head:
//                                  742 of golden's 877 SetSVDataPointer calls
//                                  are ACTIVE and byte-identical to golden;
//                                  135 are gated in 20 blocks ([G2]-[G21]),
//                                  every one of them because the golden TARGET
//                                  object does not exist in this port yet, none
//                                  because of a translation difficulty.  116 of
//                                  the 135 are compile-visible gaps; the other
//                                  19 ([G19]-[G21]) COMPILE FINE and would only
//                                  have failed at LINK -- see those entries.
//    (file-scope, golden :52-53)   two dead externs, carried verbatim -- see
//                                  the comment at them below.
//
//  Golden's 40 `#include`s are replaced by the 14-line port-equivalent block
//  below, each line naming the golden include it stands for.  Deliberately NOT
//  carried over, because NOTHING in golden's own body uses them (verified
//  against all 877 target expressions): database.h, cSortCT.h, mysensor.h,
//  uShowMessage.h, csystem.h, rs232.h, common.h, mymessbox.h, INPUT.h,
//  cMyDB.h, cTowerLight.h, note.h, cContactCT.h, cSpeed.h, cOffSet.h,
//  uTemp_Set.h, cSecurity.h, cShowBinSelect.h, fRotate.h, cBinSel.h,
//  GroundMan.h, SmartDiagnostic.h.  One of those omissions is deliberate risk
//  avoidance rather than mere tidiness: golden :28 `#include "MyKitSuck.h"`
//  maps to a path (mykitsuck.h) that declares a SECOND, DIFFERENTLY-LAID-OUT
//  `TMyKitSuck` from the one aHotPlateSubstrate.h publishes to 177 TUs -- the
//  known ODR trap recorded for this tree.  This unit never names a KitSuck
//  object, so pulling that header in could only have created the hazard, never
//  resolved anything.
//
//  GATE REGISTER
//  -------------
//  ALL 21 gates keep golden's text VERBATIM in the `#if 0` arm.  Only [G1] has
//  an `#else` arm (a substitute definition head); [G2]-[G21] have none, because
//  the faithful degradation of "cannot register this SV" is "do not register
//  it" -- there is no value to substitute.  BEHAVIOUR DELTA COMMON TO
//  [G2]-[G21]: the named SVIDs are absent from SV_ID, so a host S1F3/S1F11
//  naming one gets whatever SecsSvEcRegistration/THGem return for an
//  unregistered SVID instead of the live value, and an S2F23 trace or S2F33
//  report definition mentioning one is rejected.  Nothing else in the machine
//  changes: AddSV has no side effect beyond filling those lists.
//
//  [G19]-[G21] ARE A DIFFERENT SPECIES from [G2]-[G18] and are listed last on
//  purpose: they COMPILE PERFECTLY.  Their targets are declared in cmydef.h, so
//  `-fsyntax-only` is silent about them; the definitions are inside cmydef.cpp's
//  `#if 0 // TODO(W6)` block (opens at cmydef.cpp:5816), so they would have
//  become undefined references at LINK -- in a static archive, only when
//  something first pulled this object in, which could have been months later.
//  They were found by compiling this file to an object and diffing `nm -u` of it
//  against `nm --defined-only` of the five archives it needs; the exact commands
//  are in the hand-off report.
//
//  [G1]  golden :55 + :61 -- `void HT9045Gem::AddSV()` and its first statement
//        `HGemPtr=HGem;`.  WHY: the port's SECSGEM/uHGemHT9045.h declares
//        HT9045Gem with its constructor and ZERO of golden's 22 virtual
//        overrides (that header states this itself and lists AddSV among them);
//        this wave may not edit a pre-existing file, and C++ cannot define a
//        member a class does not declare, so the body ships as
//        `void HT9045Gem_AddSV(HT9045Gem *Self)`.  FAITHFUL BECAUSE: same
//        statements, same order, same `Self->HGemPtr=HGem;` assignment, one
//        added NULL guard (documented at the line).  BEHAVIOUR DELTA: the
//        virtual slot `HTGem::AddSV` still resolves to HTGem's inline `{}`, so
//        a `MyGem->AddSV()` call registers NOTHING until this gate is retired
//        -- i.e. today this file is translated but not yet wired.  RETIREMENT:
//        add `virtual void AddSV();` to SECSGEM/uHGemHT9045.h and flip both
//        arms.  Whoever does it must expect a -Wshadow warning from the
//        `HGemPtr` alias below (deliberate -- see its own comment).
//  [G2]  :74-75    SVID 1012/1013, fMain->edTorue0 / edTorue1 (golden main.h
//        :466/:467, TEdit*).  forms/fMain.h does not declare either.  This is
//        not a discovery: fMain.h:383-402 already predicted this exact gap,
//        citing "uHGemHT9045_SV.cpp:74 / :75", and deliberately left the
//        members out for "whichever wave translates AddSV".  It also warns that
//        they must be added as widget STAND-IN types (TfLotInfoEdit /
//        TfMainPanel), never raw pointers, or they hit the void*-overload
//        hazard.  DELTA: host cannot read the two index-arm torque readouts.
//  [G3]  :76-78    SVID 1014/1015/1016, &iSECS_GEM_PPSIGNALTOWER_CONTROL_RED /
//        _YELLOW / _GREEN.  These three are DECLARED (ckernel.cpp:1276-1278)
//        and DEFINED NOWHERE in the port -- ckernel.cpp's own comment says the
//        externs were kept undefined on purpose because their only golden uses
//        live in the deferred ShowRunLed.  Taking their address here would turn
//        a documented no-link-edge into an undefined-reference at link time
//        (this is the trap that `-fsyntax-only` cannot see).  DELTA: host
//        cannot read the three tower-lamp states.  RETIREMENT: whichever wave
//        translates main.cpp's globals (or ShowRunLed) defines them.
//  [G4]  :100      SVID 1041, fMain->lbEPenconder (golden main.h:796, TPanel*).
//        Same fMain.h:388 prediction as [G2].  DELTA: host cannot read the EP
//        encoder readout.
//  [G5]  :105-106  SVID 1047/1048, fContact->lblEPValueKg.  fContact in this
//        port is atester_shims.h's TfContactShim -- a deliberately minimal
//        offline stand-in with no widget members at all.  DELTA: host cannot
//        read contact EP-in-kg (registered twice by golden, as FT_8 and ASCII).
//  [G6]  :309-316  SVID 1351/1352/1355/1356/1359/1360/1363/1364 in the
//        `iATC_Use_Heat_Count<=4` branch -- fLotInfo->pl_ATCTempHead01..04 and
//        pl_ATCRefHead01..04.  forms/fLotInfo.h's TfLotInfo has none of the
//        pl_ATC* panels.  The BRANCH ITSELF IS NOT GATED: golden's
//        `if`/`else if` structure, condition and constants are intact, so the
//        control flow a reader sees is golden's.  DELTA: host cannot read the
//        4-head ATC arm/reference temperatures.
//  [G7]  :320-335  the same 8 members plus pl_ATCTempHead05..08 /
//        pl_ATCRefHead05..08, SVID 1351-1366, in the `<=8` branch.  Same cause,
//        same delta at 8 heads.
//  [G8]  :374-377  SVID 2003/2004/2005/2007, fContact->edForcePerDeviceKG /
//        edForcePerPinN / edAirForceN / edForcePerDeviceN.  TfContactShim, as
//        [G5].  DELTA: host cannot read the four contact-force readouts.
//  [G9]  :383-385  SVID 2018/2019/2020, fContact->edAirKPA / lblReadEP /
//        edSetKg.  As [G5].  DELTA: host cannot read contact air pressure, its
//        feedback, or the set-kg target.
//  [G10] :389      SVID 2024, fContact->lblDieForceEP.  As [G5].  NOTE the
//        neighbouring SVID 2022 (:387, `&DeviceForm_File.bUseDieForce`) stays
//        ACTIVE -- only the widget-backed half of golden's Dual-Force pair is
//        gated, and golden's own `//==> Eastsun 20260526 #026-1.79` markers
//        around it are preserved in place.
//  [G11] :426-434  SVID 2240-2248, fShowBinSelect->lblMag1..9.  There is no
//        `fShowBinSelect` object and no TfShowBinSelect class anywhere in the
//        port (absence claim #3 below).  DELTA: host cannot read the 9
//        magazine-to-bin assignments.
//  [G12] :437      SVID 2631, fContact->rgHandlerMode.  As [G5].  DELTA: host
//        cannot read the height-calibration mode.
//  [G13] :439-445  SVID 2665-2667 and 2754-2756,
//        fTrayAssignment->edAuto1Type..edAuto6Type.  No `fTrayAssignment`
//        object anywhere in the port (absence claim #4).  Golden's blank line
//        :442 sits inside the gate so the two sub-blocks stay verbatim.  DELTA:
//        host cannot read the six Auto-tray alias names.
//  [G14] :459-460  SVID 9003/9004, fCleaning->edAutoCleanAirForce_Kg / _N.
//        forms/fCleaning.h's TfCleaning declares edCleaningCount but not these
//        two.  SVID 9001 (:458, edCleaningCount) is therefore ACTIVE and is
//        deliberately OUTSIDE this gate.  DELTA: host cannot read auto-clean
//        arm total force in kg or N.
//  [G15] :705-706  SVID 37200/37201, fLotInfo->lbESDReportData /
//        lbESDDecayReportData.  As [G6].  DELTA: host cannot read the SIMCO ion
//        fan data / decay data strings.
//  [G16] :739-746  SVID 37501-37508, fObserver->lbSerialNumber01..04 /
//        lbFirmwareNumber01..04.  fObserver in this port is atester_shims.h's
//        TfObserverShim, which has labModel/labFactory/... but none of these
//        eight.  DELTA: host cannot read the four ATC power-supply serial
//        numbers or their four firmware versions.
//  [G17] :853-880  SVID 43300-43327, fGroundMan->labValue_0_2..labValue_3_5.
//        No `fGroundMan` object anywhere in the port (absence claim #5).
//        DELTA: host cannot read the 28 Ground-Man channel values.
//  [G18] :882-901  SVID 43350-43369, fSmartDiagnostic->iPushAvgTime[0..9] and
//        iPopAvgTime[0..9].  No `fSmartDiagnostic` object anywhere in the port
//        (absence claim #6 -- mycylin.cpp's four `#if 0 // TODO(W7-UI)` blocks
//        reference the same pointer and are the precedent for gating it).
//        DELTA: host cannot read the 20 cylinder average push/pop times.
//  [G19] :96-97    SVID 1038/1039, &iGrossUPH / &iNetUPH.  LINK-ONLY gap:
//        declared cmydef.h:5905 / :5904, but the sole definitions
//        (cmydef.cpp:5993 / :5992) are inside the `#if 0 // TODO(W6)` block that
//        opens at cmydef.cpp:5816.  Verified against the built archives, not
//        just by reading: `nm --defined-only` over libht9045_{sm,secsgem,forms,
//        globals}.a + libvclcompat.a does not define either symbol.  DELTA: host
//        cannot read gross UPH or net UPH.  RETIREMENT: whichever wave ungates
//        that cmydef.cpp block (or gives the four symbols a live home) frees
//        [G19]-[G21] together.
//  [G20] :215-230  SVID 1164-1179, &iSVByBinCount[0..15].  Same LINK-ONLY cause
//        (cmydef.h:5916 declares, cmydef.cpp:6000 defines inside the gate).
//        NOTE SVID 1190 (:231, fMain->SVID1190_OSSetup) sits between this gate
//        and [G21] and is deliberately left ACTIVE -- it is the one member of
//        this neighbourhood that IS real.  DELTA: host cannot read the 16
//        per-bin counts.
//  [G21] :232      SVID 1191, &iSV_ErrBinCnt.  Same LINK-ONLY cause
//        (cmydef.h:5911 / cmydef.cpp:5999).  DELTA: host cannot read the error
//        bin count.
//
//  [G2]-[G18] all share ONE root cause: golden's registration target is a live
//  VCL widget on a form (or, for [G3], a main.cpp global) that this port has
//  not stood up yet.  Counted per form: fLotInfo 26, fGroundMan 28,
//  fSmartDiagnostic 20, fContact 11, fShowBinSelect 9, fObserver 8,
//  fTrayAssignment 6, fMain 3, fCleaning 2, plus [G3]'s 3 tower-light globals
//  = 116.  ([G19]-[G21] add 19 more for the separate link-only reason above,
//  giving 135.)  The 116 decomposition matches, independently, the prior recon in
//  tests/test_w7_f1_wall2_probe.cpp:162-164 ("AddSV -- 8: fCleaning, fContact,
//  fGroundMan, fLotInfo, fObserver, fShowBinSelect, fSmartDiagnostic,
//  fTrayAssignment (PLUS 3 missing fMain members of its own)") -- 8 non-fMain
//  forms + 3 fMain widgets, exactly what is gated here.  What that recon did
//  NOT predict is [G3]; the tower-light globals are not a form dependency and
//  appear in no earlier list.
//
//  NOT A GATE -- same-result substitutions, listed so a reviewer does not read
//  "no #if 0" as "skipped":
//   (1) `SecsSvEcRegistration *HGemPtr = &Self->HGemPtr->SvEcReg;` -- one added
//       local so that all 761 active call lines stay byte-identical to golden.
//       See the long comment at the declaration for why the shadowing name is
//       deliberate.  THGem in this port does not declare SetSVDataPointer; it
//       owns the table as SvEcReg, and uHGemEquipment.cpp:2227+ already calls
//       it that way.  Zero behaviour delta: the same overload set, the same
//       object, the same 877-slot table.
//   (2) The SVID first argument stays an int literal at every call site.  The
//       port's overloads take `AnsiString SVID`, and AnsiString's `int`
//       constructor is non-explicit (vclcompat/AnsiString.h:67) -- the same
//       implicit conversion BCB6 performed, and the same one
//       uHGemEquipment.cpp's own calls rely on.
//   (3) `__fastcall` / `#pragma hdrstop` / `#pragma package(smart_init)`
//       dropped per project convention.
//
//  GOLDEN QUIRKS PRESERVED, NOT FIXED (flagged so nobody "corrects" them):
//   * :279 SVID 1269 is named "Auto 5 Countunt ART" and points at
//     `LastSet.BinCT_ART[0][e3Auto6]`, not e3Auto5 -- and SVID 1270 ("Auto 6
//     Count ART") does not exist at all, so Auto-5-ART is unreadable and
//     Auto-6-ART is readable under the Auto-5 name.  Compare :298, where SVID
//     1288 correctly uses e3Auto6.  Kept verbatim.
//   * Format/type mismatches, all kept: :698 SVID 37010 registers
//     `&iJamSkipIC` as INT_4_TYPE although cmydef.h:4023 declares it
//     `AnsiString`; :103 SVID 1045 registers `&bRunATC` (bool) as ASCII_TYPE;
//     :837 SVID 38303 registers `&LastSet.iEmptyTrayCount_ART` (int) as
//     BOOLEAN_TYPE; :838 SVID 38304 registers `&sOutputBinCode` (AnsiString) as
//     BOOLEAN_TYPE.
//   * Duplicate registrations of the same target under different SVIDs (:79
//     SVID 1017 and :690 SVID 37002 both take &LastSet.iAutoRetestCount_ART;
//     :769 SVID 37700 and :849 SVID 38314 are both named "PC_NAME" and both
//     take &PC_NAME), and duplicate SV NAMES across different SVIDs (:72 SVID
//     1010 and :82 SVID 1020 are both "Machine Pre State").
//   * :838 `&sOutputBinCode` -- Automation/AGV_PortScan.h:211 records that this
//     global is unreferenced even inside golden's own AGV.cpp, i.e. it is
//     already dead in golden.  Registered anyway, as golden does.
//   * SVIDs 1351-1366 are registered by BOTH arms of the :307/:318 branch with
//     different head mappings; only one arm ever runs.  Preserved.
//
//  ENCODING: golden is Big5; decoded with python cp950 and written here as real
//  UTF-8, LF only, zero U+FFFD (checked -- see the report).  Chinese comments
//  are TRANSCRIBED, not paraphrased or translated.
//
//  ABSENCE CLAIMS -- every one was measured at the START of this wave and
//  RE-RUN, verbatim, at 2026-08-08 23:35-23:40 +0800 as the last action before
//  hand-off, because six sibling PT-W4 units were landing files concurrently and
//  a start-of-wave grep proves nothing.  The exact commands are in the hand-off
//  report.  Each claim is stated as "no USABLE declaration", and where a
//  same-named TU-LOCAL stand-in does exist elsewhere it is named -- a TU-local
//  struct in another .cpp is NOT a member of the object golden dereferences and
//  cannot satisfy these call sites:
//   #1 no header in the port declares edTorue0 / edTorue1 / lbEPenconder; in
//      particular forms/fMain.h's TfMain does not (its only hits are the
//      fMain.h:386-388 prose that predicted this gate).  atester.cpp:1609 has a
//      TU-LOCAL `TEdit edTorue0, edTorue1;` inside its own W7T1 torque-seam
//      struct (reached via the W7T1_FMAIN_EDTORUE0/1 macros, :1621-1622) -- file
//      scope in atester.cpp, unreachable and unrelated here.
//   #2 no header in the port declares any of the 10 fContact widgets golden
//      registers (lblEPValueKg, edForcePerDeviceKG, edForcePerPinN, edAirForceN,
//      edForcePerDeviceN, edAirKPA, lblReadEP, edSetKg, lblDieForceEP,
//      rgHandlerMode); atester_shims.h's TfContactShim -- which IS what
//      `fContact` is in this port -- has none, and the port's own cContact.h
//      declares no fContact object at all.  Automation/auto9045.cpp:300-307 has
//      a TU-LOCAL `TEdit *edAirForceN, *edAirKPA, *edSetKg, *edForcePerDeviceKG,
//      *edForcePerDeviceN, ...` stand-in struct; same reasoning as #1.
//   #3 no declaration and no definition of a `fShowBinSelect` OBJECT anywhere in
//      the port (regex over every .h/.cpp for an `extern`/definition of that
//      name: zero hits).  Automation/auto9045.cpp:326 defines a TU-LOCAL
//      `struct W5FA_TfShowBinSelectExt`; AutoClean/AutoClean.cpp:1612/3932/3940/
//      4000/9011 only mention the name in comments explaining its own drops.
//   #4 no declaration and no definition of a `fTrayAssignment` OBJECT anywhere
//      in the port (same regex, zero hits).  Automation/auto9045.cpp:340-355 has
//      a TU-LOCAL `W5FA_TfTrayAssignmentExt W5FA_FTrayAssignment;`.
//   #5 no declaration and no definition of `fGroundMan` anywhere in the port;
//      its ONLY two tokens in the whole tree are prose in
//      tests/test_w7_f1_wall2_probe.cpp:162/:190 and generated dfm2rc layout
//      string data (tools/dfm2rc/layout_out/GroundMan/GroundMan_layout.gen.cpp).
//   #6 no declaration and no definition of `fSmartDiagnostic` anywhere in the
//      port.  Its only non-comment tokens are inside four `#if 0 //
//      TODO(W7-UI)` blocks in mycylin.cpp (:277-278, :294-295, :319-320,
//      :336-337, all `fSmartDiagnostic->GetCyliderOnCount/OffCount`) -- gated
//      code, no declaration, so those do not make the name usable either.
//   #7 iSECS_GEM_PPSIGNALTOWER_CONTROL_RED/GREEN/YELLOW are declared exactly
//      once (ckernel.cpp:1276-1278, an intentionally-undefined file-scope
//      extern block) and defined nowhere; they are also not in any header, so
//      this file could not even see them.
//   #8 pl_ATCTempHead01..08 / pl_ATCRefHead01..08 / lbESDReportData /
//      lbESDDecayReportData / edAutoCleanAirForce_Kg / edAutoCleanAirForce_N /
//      lbSerialNumber01..04 / lbFirmwareNumber01..04: a tree-wide regex over
//      every .h/.cpp returns ZERO hits outside THIS file.  So forms/fLotInfo.h,
//      forms/fCleaning.h and atester_shims.h's TfObserverShim are missing them,
//      and so is everything else -- there is no other candidate home.
//   #9 iGrossUPH / iNetUPH / iSV_ErrBinCnt / iSVByBinCount have NO LIVE
//      DEFINITION in any CMake-registered .cpp: the only definitions in the tree
//      are cmydef.cpp:5992/:5993/:5999/:6000, all inside the `#if 0 //
//      TODO(W6)` block opening at cmydef.cpp:5816.  Cross-checked against the
//      BUILT archives (build_0808_w3_final) with nm, not just by reading source.
//
//  CALLER PRECONDITION -- THE ONE RUNTIME HAZARD IN THIS FILE, and it is not a
//  translation defect.  218 of the 761 active calls take the address of a member
//  reached THROUGH `ArmData[0..2]` / `ArmHistory[0..2]` (`TArm*`, cSocket.h
//  :253/:255).  In this port those six pointers are zero-initialised globals
//  (cSocket.cpp:169/:171) and NOTHING in production code allocates them -- the
//  only `new TArm` in the whole tree is in tests/test_SCK_ART_Remainder.cpp
//  :1066-1067, and it allocates ArmDataLot, a different array.  Golden allocates
//  them long before its `HSys.MyGem->AddSV()` call site ever runs, so golden has
//  no such window; this port does.  Therefore: DO NOT CALL AddSV (or
//  HT9045Gem_AddSV) until ArmData[0..2] and ArmHistory[0..2] are allocated, or
//  the 218 calls dereference NULL.  This is deliberately NOT papered over with a
//  guard: guarding would silently drop 218 SVs on a real machine that HAS
//  allocated them, and wrapping the golden blocks in an `if` would re-indent 218
//  otherwise byte-identical golden lines.  Nothing in the tree calls this
//  function today, so nothing is at risk right now -- but the wave that wires
//  the call must land the allocation first.  (Everything else the active body
//  dereferences is already a live object: fMain is `new TfMain()` with
//  palMainStatus/tTestResult allocated in its ctor, fCleaning is
//  `new TfCleaning()` with edCleaningCount allocated, TArm's own ctor allocates
//  sPass/sFail/sTotal/iIFErr and every ArmSKET[i][j].)
//
//  CMakeLists PLACEMENT -- READ BEFORE REGISTERING THIS FILE.  It must go in
//  `ht9045_sm`, NOT in `ht9045_secsgem`.  THREE ht9045_sm members define objects
//  the ACTIVE body takes the address of: cSocket.cpp (ArmData/ArmHistory, 218
//  calls), Automation/AGV_PortScan.cpp (iLoadStateATK 9 calls, sOutputBinCode 1)
//  and acatchtray_shims.cpp:189 (iPortStatus, 15 calls -- note cmydef.cpp:5950
//  ALSO defines iPortStatus but inside the `#if 0 // TODO(W6)` block, so
//  acatchtray_shims.cpp holds the only live definition).
//  ht9045_secsgem deliberately does NOT link ht9045_sm
//  (adding that edge is the documented CMake cycle, CMakeLists.txt:579-616).
//  ht9045_sm links ht9045_secsgem + ht9045_forms + ht9045_globals + vclcompat,
//  which closes every symbol this file needs.  Putting it in ht9045_secsgem
//  would still CONFIGURE and still pass `-fsyntax-only`, and would stay silent
//  until something first referenced the symbol -- the archive-extraction trap.
//  NOTE the two ht9045_sm objects that are NOT a placement argument, because
//  gating removed them: fContact (atester_shims.cpp) is referenced only by
//  [G5]/[G8]/[G9]/[G10]/[G12], and fObserver only by [G16] -- both are dead
//  references in the file as it ships, and both come back the moment those
//  gates are retired.
// =============================================================================
#include "MachineDefine.h"                 // golden :1 (verbatim); #pragma hdrstop dropped

#include "SECSGEM/uHGemHT9045.h"            // golden :4 "uHGemHT9045.h" -- HT9045Gem + `extern THGem *HGem;`
#include "SECSGEM/uHGemEquipment.h"         // PORT-ONLY: THGem's COMPLETE type (golden gets it transitively via
                                            //   uHGemHT9045.h->uHGemClass.h; the port's uHGemClass.h only
                                            //   forward-declares THGem, so the SvEcReg member deref below needs
                                            //   this explicitly -- same include this port's SECSGEM/uHGemHT9045.cpp
                                            //   already adds for the same reason.

#include "MachineType.h"                    // golden :6  -- MAX_SOCKET_ROW/MAX_SOCKET_COL
#include "cprod.h"                          // golden :11 -- RunInfo / TestIF_File (golden cTesterIF.h) /
                                            //   DeviceForm_File / Temperature (golden cTemperFrom.h) / tc* enum
#include "LastSet.h"                        // PORT: LastSet (golden reaches it transitively via main.h/cmydef.h)
#include "Config.h"                         // PORT equivalent of golden :37 "cConfiguration.h" -- IniConfig
#include "CosFunction.h"                    // PORT: CosFunction (golden reaches it via cprod.h/cmydef.h chain)
#include "cmydef.h"                         // golden :24/:43 -- the int/AnsiString global block + iPortStatus
#include "cSocket.h"                        // golden :26 -- ArmData[] / ArmHistory[] (TArm/TMySocket)
#include "atester.h"                        // golden :19 -- iTesterBIN's declaring header (see file-scope extern below)
#include "atester_shims.h"                  // PORT equivalent of golden :30 "cContact.h" (fContact) and :10
                                            //   "cObserver.h" (fObserver) -- both live here as TfContactShim /
                                            //   TfObserverShim, NOT in the port's own cContact.h
#include "FormsFacade.h"                    // PORT equivalent of golden :8 "main.h" (fMain), :17 "uLotInfo.h"
                                            //   (fLotInfo), :35/:40 "uCleaning.h" (fCleaning)
#include "Automation/AGV_PortScan.h"        // PORT equivalent of golden :44 "AGV.h" -- iLoadStateATK / sOutputBinCode

// golden :52-53, verbatim.  NEITHER is referenced anywhere in golden's own AddSV
// body (checked every one of the 877 calls) -- they are dead file-scope
// declarations golden carries.  Kept because they ARE golden's text:
//   * iTesterBIN has a real definition (atester.cpp:117, exported via atester.h
//     -- this re-declaration is a duplicate `extern` of an identical
//     declaration, which is legal).
//   * ShowTempComp (the TPanel* ARRAY -- a DIFFERENT object from
//     RunInfo.ShowTempComp[], the AnsiString array at cprod.h:2730 that 55 of
//     the calls below DO use) has NO definition anywhere in the port; because
//     nothing references it, no relocation and no link edge is created.  Same
//     "keep the interface, define nothing" treatment ckernel.cpp:1261-1273
//     already applies to its own six undefined file-scope externs.
extern int iTesterBIN[MAX_SOCKET_ROW][MAX_SOCKET_COL];
extern TPanel *ShowTempComp[tcTotalCount];
//------------------------------------------------------------------------------
// GATE [G1] -- golden :55 (the definition head) and golden :61 (`HGemPtr=HGem;`).
// The port's SECSGEM/uHGemHT9045.h declares HT9045Gem with its ctor ONLY and
// ZERO of golden's 22 virtual overrides (that header says so itself, and names
// AddSV in the list); this wave may not edit that pre-existing header, and C++
// forbids defining a member that its class does not declare.  So the body is
// emitted under a free-function head instead, and the member head is preserved
// verbatim in the #if 0 arm.  RETIREMENT IS TWO EDITS: add
// `virtual void AddSV();` to SECSGEM/uHGemHT9045.h, then flip both arms here.
// See the file-head GATE REGISTER for the behaviour delta and for the -Wshadow
// note that applies once the member arm is live.
#if 0
void HT9045Gem::AddSV()
#else
void HT9045Gem_AddSV(HT9045Gem *Self)
#endif
{
    //============================================================================================================================================
    // MAINFORM
    //============================================================================================================================================
//     SV
#if 0
    HGemPtr=HGem;
#else
    Self->HGemPtr=HGem;                                                         // G1: golden :61, through the explicit `Self`
    // PORT-ONLY null guard (NOT golden).  `THGem *HGem` is initialised to NULL
    // (SECSGEM/uHGemEquipment.cpp:3521) and stays NULL until main.cpp -- which
    // is unported -- constructs the engine, whereas golden's only call site
    // (SECSGEM/UsecegemMainFrom.cpp:202, `HSys.MyGem->AddSV();`) runs long
    // after that construction.  Golden would dereference NULL here in the
    // port's state; refusing to register instead of crashing is the same
    // NULL-tolerance this port's SECSGEM/uHGemHT9045.cpp ctor already added for
    // the identical reason (see its own comment).  No golden-reachable state
    // changes: whenever HGem is non-NULL this line does nothing.
    if (Self->HGemPtr == NULL) return;
#endif

    if(CosFunction.bEnable_SECS_GEM==true)  //Ifor 20220719 add 有SECS GEM 功能即載入資料避免後續開啟造成異常
    {
        // PORT-ONLY ALIAS -- NOT A GATE, zero behaviour delta.  Golden calls
        // `HGemPtr->SetSVDataPointer(...)` 877 times; in this port THGem does
        // NOT declare SetSVDataPointer -- it OWNS the registration table as a
        // by-value `SecsSvEcRegistration SvEcReg;` member (uHGemEquipment.h:753),
        // and uHGemEquipment.cpp's own ~30 registration calls (e.g. :2227) go
        // through `SvEcReg.SetSVDataPointer(...)`.  Binding a local of that
        // pointer type, named exactly `HGemPtr`, lets all 877 golden call lines
        // below stay BYTE-IDENTICAL to golden instead of taking an
        // `->SvEcReg.` edit each.  Declared INSIDE golden's own
        // `if(CosFunction.bEnable_SECS_GEM==true)` block on purpose: golden
        // never touches HGemPtr when that flag is false, and neither may this.
        // CAUTION for whoever retires GATE [G1]: in the member form this local
        // SHADOWS the inherited `THGem *HGemPtr` member (a -Wshadow warning,
        // not an error) -- that is intended; the shadow is what keeps the call
        // lines verbatim.  It is a DIFFERENT TYPE from golden's HGemPtr.
        SecsSvEcRegistration *HGemPtr = &Self->HGemPtr->SvEcReg;
        HGemPtr->SetSVDataPointer(1000 , HType.ASCII_TYPE,   "Machine Define",                                                      "", &RunInfo.MachineDefine,                                 "機台定義"                                                  );
        HGemPtr->SetSVDataPointer(1001 , HType.ASCII_TYPE,   "Machine Model",                                                       "", &IniConfig.sMachineType,                                "Machine Type"                                                  );
        HGemPtr->SetSVDataPointer(1002 , HType.ASCII_TYPE,   "Machine ID",                                                          "", &IniConfig.sGPIBMachineID,                              "Machine ID"                                                    );
        HGemPtr->SetSVDataPointer(1003 , HType.ASCII_TYPE,   "Software Version",                                                    "", &RunInfo.SoftwareVersion,                               "Software Version"                                                  );
        HGemPtr->SetSVDataPointer(1004 , HType.ASCII_TYPE,   "Software Release Date",                                               "", &RunInfo.SoftwareDate,                                  "Software Release Date"                                     );
        HGemPtr->SetSVDataPointer(1005 , HType.ASCII_TYPE,   "Factory",                                                             "", &RunInfo.Factory,                                       "Factory"                                                   );
        HGemPtr->SetSVDataPointer(1009 , HType.ASCII_TYPE,   "Lot Start Time",                                                      "", &RunInfo.LotStartTime,                                  "Lot Start Time"                                            );
        HGemPtr->SetSVDataPointer(1010 , HType.ASCII_TYPE,   "Machine Pre State",                                                   "", &sMachinePreState,                                      "Machine Pre State"                                         );
        HGemPtr->SetSVDataPointer(1011 , HType.ASCII_TYPE,   "Machine State",                                                       "", fMain->palMainStatus,                                   "Machine Status"                                                  );
#if 0 // GATE [G2] -- golden :74-75.  SVID 1012/1013 -- fMain->edTorue0 / edTorue1 (golden main.h:466/467, TEdit*): NOT members of forms/fMain.h's TfMain.
        HGemPtr->SetSVDataPointer(1012 , HType.ASCII_TYPE,   "Index Arm1 Torque",                                                   "", fMain->edTorue0,                                        "Index Arm1 Torque"                                         );
        HGemPtr->SetSVDataPointer(1013 , HType.ASCII_TYPE,   "Index Arm2 Torque",                                                   "", fMain->edTorue1 ,                                       "Index Arm2 Torque"                                         );
#endif // GATE [G2]
#if 0 // GATE [G3] -- golden :76-78.  SVID 1014/1015/1016 -- iSECS_GEM_PPSIGNALTOWER_CONTROL_RED/YELLOW/GREEN: declared extern in ckernel.cpp:1276-1278, DEFINED NOWHERE in the port.
        HGemPtr->SetSVDataPointer(1014 , HType.INT_4_TYPE,   "Tower Light Red",                                                     "", &iSECS_GEM_PPSIGNALTOWER_CONTROL_RED,                   "三色燈-紅; 0:關; 1:開; 2:閃爍"                             );
        HGemPtr->SetSVDataPointer(1015 , HType.INT_4_TYPE,   "Tower Light Yellow",                                                  "", &iSECS_GEM_PPSIGNALTOWER_CONTROL_YELLOW,                "三色燈-紅; 0:關; 1:開; 2:閃爍"                             );
        HGemPtr->SetSVDataPointer(1016 , HType.INT_4_TYPE,   "Tower Light Green",                                                   "", &iSECS_GEM_PPSIGNALTOWER_CONTROL_GREEN,                 "三色燈-紅; 0:關; 1:開; 2:閃爍"                             );
#endif // GATE [G3]
        HGemPtr->SetSVDataPointer(1017 , HType.INT_4_TYPE,   "Number of RT",                                                        "", &LastSet.iAutoRetestCount_ART,                          "Number of RT"                                              );
        HGemPtr->SetSVDataPointer(1018 , HType.ASCII_TYPE,   "Galil_Driver_Version",                                                "", &RunInfo.GalilVersion,                                  "Gaili Driver Version"                                      );
        HGemPtr->SetSVDataPointer(1019 , HType.INT_4_TYPE,   "Machine Pre Status",                                                  "", &iSECSGEMMachinePreState,                               "Machine PreStatus"                                         );  //KaiChen 20200325 ：Add for ASECL  //JerryYang 20220215 : MachinePreState改成int
        HGemPtr->SetSVDataPointer(1020 , HType.INT_4_TYPE,   "Machine Pre State",                                                   "", &iSECSGEMMachineState,                                  "Machine Pre State"                                         );  //KaiChen 20200325 ：Add for ASECL  //JerryYang 20220215 : MachinePreState改成int
        HGemPtr->SetSVDataPointer(1021 , HType.INT_4_TYPE,   "UPH",                                                                 "", &RunInfo.iUPH,                                          "UPH"                                                       );
        HGemPtr->SetSVDataPointer(1023 , HType.ASCII_TYPE,   "Index Time",                                                          "", &RunInfo.IndexTime,                                     "Index Time"                                                );
        HGemPtr->SetSVDataPointer(1024 , HType.ASCII_TYPE,   "Index Cycle Time",                                                    "", &RunInfo.IndexCycleTime,                                "Index Cycle Time"                                          );
        HGemPtr->SetSVDataPointer(1025 , HType.ASCII_TYPE,   "Test Time",                                                           "", &RunInfo.TestTime,                                      "Test Time"                                                  );
        HGemPtr->SetSVDataPointer(1026 , HType.ASCII_TYPE,   "MTBA",                                                                "", &RunInfo.MTBA,                                          "MTBA"                                                      );
        HGemPtr->SetSVDataPointer(1027 , HType.ASCII_TYPE,   "System Time",                                                         "", &RunInfo.SystemTime,                                    "System Time"                                                  );
        HGemPtr->SetSVDataPointer(1028 , HType.ASCII_TYPE,   "Avg UPH",                                                             "", &RunInfo.iAvgUPH,                                       "Average UPH"                                                   );
        HGemPtr->SetSVDataPointer(1031 , HType.ASCII_TYPE,   "MUBA",                                                                "", &RunInfo.MUBA,                                          "MUBA"                                                      );
        HGemPtr->SetSVDataPointer(1032 , HType.INT_4_TYPE,   "Power On Time",                                                       "", &LastSet.SystemAccSecond[0][stPowerOn],                 "Power On Time"                                             );
        HGemPtr->SetSVDataPointer(1033 , HType.INT_4_TYPE,   "Running Time",                                                        "", &LastSet.SystemAccSecond[0][stStartTime],               "Running Time"                                              );
        HGemPtr->SetSVDataPointer(1034 , HType.INT_4_TYPE,   "Production Time",                                                     "", &LastSet.SystemAccSecond[0][stProductTime],             "Production Time"                                           );
        HGemPtr->SetSVDataPointer(1035 , HType.INT_4_TYPE,   "Pause Time",                                                          "", &LastSet.SystemAccSecond[0][stPauseTime],               "Pause Time"                                                );
        HGemPtr->SetSVDataPointer(1036 , HType.INT_4_TYPE,   "Jam Count",                                                           "", &LastSet.iJamCount[1],                                  "Jam Count"                                                 );
#if 0 // GATE [G19] -- golden :96-97.  SVID 1038/1039 -- &iGrossUPH / &iNetUPH: DECLARED (cmydef.h:5905/:5904) but the only DEFINITIONS (cmydef.cpp:5993/:5992) sit inside `#if 0 // TODO(W6)` (cmydef.cpp:5816) -> undefined at LINK, invisible to -fsyntax-only.
        HGemPtr->SetSVDataPointer(1038 , HType.INT_4_TYPE,   "Gross UPH",                                                           "", &iGrossUPH,                                             "Gross UPH"                                                 );  //JerryYang 20250120 : add
        HGemPtr->SetSVDataPointer(1039 , HType.INT_4_TYPE,   "Net UPH",                                                             "", &iNetUPH,                                               "Net UPH"                                                   );
#endif // GATE [G19]

        HGemPtr->SetSVDataPointer(1040 , HType.INT_4_TYPE,   "ATC SYSTEM",                                                          "", &ATC_SYSTEM,                                            "ATC SYSTEM"                                                );
#if 0 // GATE [G4] -- golden :100-100.  SVID 1041 -- fMain->lbEPenconder (golden main.h:796, TPanel*): NOT a member of forms/fMain.h's TfMain.
        HGemPtr->SetSVDataPointer(1041 , HType.ASCII_TYPE,   "EP Penconder",                                                        "", fMain->lbEPenconder,                                    "EP Penconder"                                              );  //KenHsieh 20220913 : 因常顯示無法將INT_4_TYPE -> ASCII_TYPE
#endif // GATE [G4]
        HGemPtr->SetSVDataPointer(1043 , HType.BOOLEAN_TYPE, "ATC State",                                                           "", &iATCOnLine,                                            "ATC State"                                                 );
        HGemPtr->SetSVDataPointer(1044 , HType.BOOLEAN_TYPE, "ATC RUN",                                                             "", &bRunATC,                                               "ATC RUN"                                                   );
        HGemPtr->SetSVDataPointer(1045 , HType.ASCII_TYPE,   "ATC Chiller",                                                         "", &bRunATC,                                               "ATC Chiller"                                               );
        HGemPtr->SetSVDataPointer(1046 , HType.ASCII_TYPE,   "Chiller Temp",                                                        "", &Temperature.iATCChillerTemp,                           "Chiller Temp"                                              );  //Steven 20200723 : 沒ATC的去取用對應的變數會出現例外  //Jimmychiu 20260130 : 修正SECS Chiller Temp亂碼問題
#if 0 // GATE [G5] -- golden :105-106.  SVID 1047/1048 -- fContact->lblEPValueKg: NOT a member of atester_shims.h's TfContactShim.
        HGemPtr->SetSVDataPointer(1047 , HType.FT_8_TYPE,    "Read KG",                                                             "", fContact->lblEPValueKg,                                 "Read KG"                                                   );
        HGemPtr->SetSVDataPointer(1048 , HType.ASCII_TYPE,   "Test Arm EP value transform to Kg",                                   "", fContact->lblEPValueKg,                                 "Test Arm EP value transform to Kg"                         );
#endif // GATE [G5]
        HGemPtr->SetSVDataPointer(1049 , HType.INT_4_TYPE,   "Which Arm is in socket area",                                         "", &iWhichArmDown,                                         "Which Arm is in socket area"                               );  //JerryYang 20200316 add SVID 哪支arm下壓在測區
        HGemPtr->SetSVDataPointer(1050 , HType.ASCII_TYPE,   "Lot End Time",                                                        "", &RunInfo.LotEndTime,                                    "Lot End Time"                                              );  //Ifor 20221018 add: SVID Lot End Time
        HGemPtr->SetSVDataPointer(1051 , HType.ASCII_TYPE,   "Hot Plate1 Temperature",                                              "", &RunInfo.ShowTempComp[tcHotPlate1],                     "Hot Plate1 Temperature"                                           );
        HGemPtr->SetSVDataPointer(1052 , HType.ASCII_TYPE,   "Hot Plate2 Temperature",                                              "", &RunInfo.ShowTempComp[tcHotPlate2],                     "Hot Plate2 Temperature"                                           );
        HGemPtr->SetSVDataPointer(1053 , HType.ASCII_TYPE,   "Shuttle1 Temperature",                                                "", &RunInfo.ShowTempComp[tcShuttle1],                      "Shuttle1 Temperature"                                             );
        HGemPtr->SetSVDataPointer(1054 , HType.ASCII_TYPE,   "Shuttle2 Temperature",                                                "", &RunInfo.ShowTempComp[tcShuttle2],                      "Shuttle2 Temperature"                                             );
        HGemPtr->SetSVDataPointer(1055 , HType.ASCII_TYPE,   "Head1 Temperature",                                                   "", &RunInfo.ShowTempComp[tcHead1],                         "Head1 Temperature"                                                );
        HGemPtr->SetSVDataPointer(1056 , HType.ASCII_TYPE,   "Head2 Temperature",                                                   "", &RunInfo.ShowTempComp[tcHead2],                         "Head2 Temperature"                                                );
        HGemPtr->SetSVDataPointer(1057 , HType.ASCII_TYPE,   "Head5 Temperature",                                                   "", &RunInfo.ShowTempComp[tcHead3],                         "Head5 Temperature"                                                );
        HGemPtr->SetSVDataPointer(1058 , HType.ASCII_TYPE,   "Head6 Temperature",                                                   "", &RunInfo.ShowTempComp[tcHead4],                         "Head6 Temperature"                                                );
        HGemPtr->SetSVDataPointer(1059 , HType.ASCII_TYPE,   "DUT1 Temperature",                                                    "", &RunInfo.ShowTempComp[tcDUT1],                          "DUT1 Temperature"                                                 );
        HGemPtr->SetSVDataPointer(1060 , HType.ASCII_TYPE,   "DUT2 Temperature",                                                    "", &RunInfo.ShowTempComp[tcDUT2],                          "DUT2 Temperature"                                                 );
        HGemPtr->SetSVDataPointer(1061 , HType.ASCII_TYPE,   "Chamber Temperature",                                                 "", &RunInfo.ShowTempComp[tcChamber],                       "Chamber Temperature"                                       );
        HGemPtr->SetSVDataPointer(1062 , HType.ASCII_TYPE,   "CCD Temperature",                                                     "", &RunInfo.ShowTempComp[tcCCD],                           "CCD Temperature"                                           );
        HGemPtr->SetSVDataPointer(1063 , HType.ASCII_TYPE,   "Aa 1 Temperature",                                                    "", &RunInfo.ShowTempComp[tcAa1],                           "Aa 1 Temperature"                                          );
        HGemPtr->SetSVDataPointer(1064 , HType.ASCII_TYPE,   "Ab 1 Temperature",                                                    "", &RunInfo.ShowTempComp[tcAb1],                           "Ab 1 Temperature"                                          );
        HGemPtr->SetSVDataPointer(1065 , HType.ASCII_TYPE,   "Ac 1 Temperature",                                                    "", &RunInfo.ShowTempComp[tcAc1],                           "Ac 1 Temperature"                                          );
        HGemPtr->SetSVDataPointer(1066 , HType.ASCII_TYPE,   "Ad 1 Temperature",                                                    "", &RunInfo.ShowTempComp[tcAd1],                           "Ad 1 Temperature"                                          );
        HGemPtr->SetSVDataPointer(1067 , HType.ASCII_TYPE,   "Ba 1 Temperature",                                                    "", &RunInfo.ShowTempComp[tcBa1],                           "Ba 1 Temperature"                                          );
        HGemPtr->SetSVDataPointer(1068 , HType.ASCII_TYPE,   "Bb 1 Temperature",                                                    "", &RunInfo.ShowTempComp[tcBb1],                           "Bb 1 Temperature"                                          );
        HGemPtr->SetSVDataPointer(1069 , HType.ASCII_TYPE,   "Bc 1 Temperature",                                                    "", &RunInfo.ShowTempComp[tcBc1],                           "Bc 1 Temperature"                                          );
        HGemPtr->SetSVDataPointer(1070 , HType.ASCII_TYPE,   "Bd 1 Temperature",                                                    "", &RunInfo.ShowTempComp[tcBd1],                           "Bd 1 Temperature"                                          );
        HGemPtr->SetSVDataPointer(1071 , HType.ASCII_TYPE,   "Aa 2 Temperature",                                                    "", &RunInfo.ShowTempComp[tcAa2],                           "Aa 2 Temperature"                                          );
        HGemPtr->SetSVDataPointer(1072 , HType.ASCII_TYPE,   "Ab 2 Temperature",                                                    "", &RunInfo.ShowTempComp[tcAb2],                           "Ab 2 Temperature"                                          );
        HGemPtr->SetSVDataPointer(1073 , HType.ASCII_TYPE,   "Ac 2 Temperature",                                                    "", &RunInfo.ShowTempComp[tcAc2],                           "Ac 2 Temperature"                                          );
        HGemPtr->SetSVDataPointer(1074 , HType.ASCII_TYPE,   "Ad 2 Temperature",                                                    "", &RunInfo.ShowTempComp[tcAd2],                           "Ad 2 Temperature"                                          );
        HGemPtr->SetSVDataPointer(1075 , HType.ASCII_TYPE,   "Ba 2 Temperature",                                                    "", &RunInfo.ShowTempComp[tcBa2],                           "Ba 2 Temperature"                                          );
        HGemPtr->SetSVDataPointer(1076 , HType.ASCII_TYPE,   "Bb 2 Temperature",                                                    "", &RunInfo.ShowTempComp[tcBb2],                           "Bb 2 Temperature"                                          );
        HGemPtr->SetSVDataPointer(1077 , HType.ASCII_TYPE,   "Bc 2 Temperature",                                                    "", &RunInfo.ShowTempComp[tcBc2],                           "Bc 2 Temperature"                                          );
        HGemPtr->SetSVDataPointer(1078 , HType.ASCII_TYPE,   "Bd 2 Temperature",                                                    "", &RunInfo.ShowTempComp[tcBd2],                           "Bd 2 Temperature"                                          );
        HGemPtr->SetSVDataPointer(1079 , HType.ASCII_TYPE,   "Heat Gun 1 Temperature",                                              "", &RunInfo.ShowTempComp[tcHeatGun1],                      "Heat Gun 1 Temperature"                                    );
        HGemPtr->SetSVDataPointer(1080 , HType.ASCII_TYPE,   "Heat Gun 2 Temperature",                                              "", &RunInfo.ShowTempComp[tcHeatGun2],                      "Heat Gun 2 Temperature"                                    );
        HGemPtr->SetSVDataPointer(1081 , HType.ASCII_TYPE,   "DUT 3 Temperature",                                                   "", &RunInfo.ShowTempComp[tcDUT3],                          "DUT 3 Temperature"                                         );
        HGemPtr->SetSVDataPointer(1082 , HType.ASCII_TYPE,   "DUT 4 Temperature",                                                   "", &RunInfo.ShowTempComp[tcDUT4],                          "DUT 4 Temperature"                                         );
        HGemPtr->SetSVDataPointer(1083 , HType.ASCII_TYPE,   "Socket Temperature",                                                  "", &RunInfo.ShowTempComp[tcSocket],                        "Socket Temperature"                                        );
        HGemPtr->SetSVDataPointer(1084 , HType.ASCII_TYPE,   "Ae 1 Temperature",                                                    "", &RunInfo.ShowTempComp[tcAe1],                           "Ae 1 Temperature"                                          );
        HGemPtr->SetSVDataPointer(1085 , HType.ASCII_TYPE,   "Af 1 Temperature",                                                    "", &RunInfo.ShowTempComp[tcAf1],                           "Af 1 Temperature"                                          );
        HGemPtr->SetSVDataPointer(1086 , HType.ASCII_TYPE,   "Ag 1 Temperature",                                                    "", &RunInfo.ShowTempComp[tcAg1],                           "Ag 1 Temperature"                                          );
        HGemPtr->SetSVDataPointer(1087 , HType.ASCII_TYPE,   "Ah 1 Temperature",                                                    "", &RunInfo.ShowTempComp[tcAh1],                           "Ah 1 Temperature"                                          );
        HGemPtr->SetSVDataPointer(1088 , HType.ASCII_TYPE,   "Be 1 Temperature",                                                    "", &RunInfo.ShowTempComp[tcBe1],                           "Be 1 Temperature"                                          );
        HGemPtr->SetSVDataPointer(1089 , HType.ASCII_TYPE,   "Bf 1 Temperature",                                                    "", &RunInfo.ShowTempComp[tcBf1],                           "Bf 1 Temperature"                                          );
        HGemPtr->SetSVDataPointer(1090 , HType.ASCII_TYPE,   "Bg 1 Temperature",                                                    "", &RunInfo.ShowTempComp[tcBg1],                           "Bg 1 Temperature"                                          );
        HGemPtr->SetSVDataPointer(1091 , HType.ASCII_TYPE,   "Bh 1 Temperature",                                                    "", &RunInfo.ShowTempComp[tcBh1],                           "Bh 1 Temperature"                                          );
        HGemPtr->SetSVDataPointer(1092 , HType.ASCII_TYPE,   "Ae 2 Temperature",                                                    "", &RunInfo.ShowTempComp[tcAe2],                           "Ae 2 Temperature"                                          );
        HGemPtr->SetSVDataPointer(1093 , HType.ASCII_TYPE,   "Af 2 Temperature",                                                    "", &RunInfo.ShowTempComp[tcAf2],                           "Af 2 Temperature"                                          );
        HGemPtr->SetSVDataPointer(1094 , HType.ASCII_TYPE,   "Ag 2 Temperature",                                                    "", &RunInfo.ShowTempComp[tcAg2],                           "Ag 2 Temperature"                                          );
        HGemPtr->SetSVDataPointer(1095 , HType.ASCII_TYPE,   "Ah 2 Temperature",                                                    "", &RunInfo.ShowTempComp[tcAh2],                           "Ah 2 Temperature"                                          );
        HGemPtr->SetSVDataPointer(1096 , HType.ASCII_TYPE,   "Be 2 Temperature",                                                    "", &RunInfo.ShowTempComp[tcBe2],                           "Be 2 Temperature"                                          );
        HGemPtr->SetSVDataPointer(1097 , HType.ASCII_TYPE,   "Bf 2 Temperature",                                                    "", &RunInfo.ShowTempComp[tcBf2],                           "Bf 2 Temperature"                                          );
        HGemPtr->SetSVDataPointer(1098 , HType.ASCII_TYPE,   "Bg 2 Temperature",                                                    "", &RunInfo.ShowTempComp[tcBg2],                           "Bg 2 Temperature"                                          );
        HGemPtr->SetSVDataPointer(1099 , HType.ASCII_TYPE,   "Bh 2 Temperature",                                                    "", &RunInfo.ShowTempComp[tcBh2],                           "Bh 2 Temperature"                                          );
        HGemPtr->SetSVDataPointer(1101 , HType.INT_4_TYPE,   "Loader Count",                                                        "", &LastSet.SendCT[0],                                     "Loading Count"                                                  );
        HGemPtr->SetSVDataPointer(1102 , HType.INT_4_TYPE,   "Output Total Count",                                                  "", &RunInfo.iUnloadCount,                                  "Unloading Count"                                                );
        HGemPtr->SetSVDataPointer(1103 , HType.INT_4_TYPE,   "Auto1 Count",                                                         "", &LastSet.BinCT[0][e3Auto1],                             "Auto1 Count"                                                );
        HGemPtr->SetSVDataPointer(1104 , HType.INT_4_TYPE,   "Auto2 Count",                                                         "", &LastSet.BinCT[0][e3Auto2],                             "Auto2 Count"                                                );
        HGemPtr->SetSVDataPointer(1105 , HType.INT_4_TYPE,   "Auto3 Count",                                                         "", &LastSet.BinCT[0][e3Auto3],                             "Auto3 Count"                                                );
        HGemPtr->SetSVDataPointer(1106 , HType.INT_4_TYPE,   "Fix1 Count",                                                          "", &LastSet.BinCT[0][e3Fix1 ],                             "Fix1 Count"                                                 );
        HGemPtr->SetSVDataPointer(1107 , HType.INT_4_TYPE,   "Fix2 Count",                                                          "", &LastSet.BinCT[0][e3Fix2 ],                             "Fix2 Count"                                                 );
        HGemPtr->SetSVDataPointer(1108 , HType.INT_4_TYPE,   "Fix3 Count",                                                          "", &LastSet.BinCT[0][e3Fix3 ],                             "Fix3 Count"                                                 );
        HGemPtr->SetSVDataPointer(1109 , HType.INT_4_TYPE,   "Fix4 Count",                                                          "", &LastSet.BinCT[0][e3Fix4 ],                             "Fix4 Count"                                                 );
        HGemPtr->SetSVDataPointer(1110 , HType.INT_4_TYPE,   "Fix5 Count",                                                          "", &LastSet.BinCT[0][e3Fix5 ],                             "Fix5 Count"                                                 );
        HGemPtr->SetSVDataPointer(1111 , HType.INT_4_TYPE,   "Fix6 Count",                                                          "", &LastSet.BinCT[0][e3Fix6 ],                             "Fix6 Count"                                                 );
        HGemPtr->SetSVDataPointer(1112 , HType.INT_4_TYPE,   "Loader Count ART",                                                    "", &LastSet.SendCT_ART[0],                                 "ART Loading Count"                                               );
        HGemPtr->SetSVDataPointer(1113 , HType.INT_4_TYPE,   "Output Total Count ART",                                              "", &RunInfo.iUnloadCount_ART,                              "ART Unloading Count"                                             );
        HGemPtr->SetSVDataPointer(1114 , HType.INT_4_TYPE,   "Auto1 Count ART",                                                     "", &LastSet.BinCT_ART[0][e3Auto1],                         "ART Auto1 Count"                                            );
        HGemPtr->SetSVDataPointer(1115 , HType.INT_4_TYPE,   "Auto2 Count ART",                                                     "", &LastSet.BinCT_ART[0][e3Auto2],                         "ART Auto2 Count"                                            );
        HGemPtr->SetSVDataPointer(1116 , HType.INT_4_TYPE,   "Auto3 Count ART",                                                     "", &LastSet.BinCT_ART[0][e3Auto3],                         "ART Auto3 Count"                                            );
        HGemPtr->SetSVDataPointer(1117 , HType.INT_4_TYPE,   "Fix1 Count ART",                                                      "", &LastSet.BinCT_ART[0][e3Fix1 ],                         "ART Fix1 Count"                                             );
        HGemPtr->SetSVDataPointer(1118 , HType.INT_4_TYPE,   "Fix2 Count ART",                                                      "", &LastSet.BinCT_ART[0][e3Fix2 ],                         "ART Fix2 Count"                                             );
        HGemPtr->SetSVDataPointer(1119 , HType.INT_4_TYPE,   "Fix3 Count ART",                                                      "", &LastSet.BinCT_ART[0][e3Fix3 ],                         "ART Fix3 Count"                                             );
        HGemPtr->SetSVDataPointer(1120 , HType.INT_4_TYPE,   "Fix4 Count ART",                                                      "", &LastSet.BinCT_ART[0][e3Fix4 ],                         "ART Fix4 Count"                                             );
        HGemPtr->SetSVDataPointer(1121 , HType.INT_4_TYPE,   "Fix5 Count ART",                                                      "", &LastSet.BinCT_ART[0][e3Fix5 ],                         "ART Fix5 Count"                                             );
        HGemPtr->SetSVDataPointer(1122 , HType.INT_4_TYPE,   "Fix6 Count ART",                                                      "", &LastSet.BinCT_ART[0][e3Fix6 ],                         "ART Fix6 Count"                                             );
        HGemPtr->SetSVDataPointer(1123 , HType.INT_4_TYPE,   "Bin 0 Count ART",                                                     "", &LastSet.iBinData32_ART[0][0],                          "ART Bin 0 Count"                                            );
        HGemPtr->SetSVDataPointer(1124 , HType.INT_4_TYPE,   "Bin 1 Count ART",                                                     "", &LastSet.iBinData32_ART[0][1],                          "ART Bin 1 Count"                                            );
        HGemPtr->SetSVDataPointer(1125 , HType.INT_4_TYPE,   "Bin 2 Count ART",                                                     "", &LastSet.iBinData32_ART[0][2],                          "ART Bin 2 Count"                                            );
        HGemPtr->SetSVDataPointer(1126 , HType.INT_4_TYPE,   "Bin 3 Count ART",                                                     "", &LastSet.iBinData32_ART[0][3],                          "ART Bin 3 Count"                                            );
        HGemPtr->SetSVDataPointer(1127 , HType.INT_4_TYPE,   "Bin 4 Count ART",                                                     "", &LastSet.iBinData32_ART[0][4],                          "ART Bin 4 Count"                                            );
        HGemPtr->SetSVDataPointer(1128 , HType.INT_4_TYPE,   "Bin 5 Count ART",                                                     "", &LastSet.iBinData32_ART[0][5],                          "ART Bin 5 Count"                                            );
        HGemPtr->SetSVDataPointer(1129 , HType.INT_4_TYPE,   "Bin 6 Count ART",                                                     "", &LastSet.iBinData32_ART[0][6],                          "ART Bin 6 Count"                                            );
        HGemPtr->SetSVDataPointer(1130 , HType.INT_4_TYPE,   "Bin 7 Count ART",                                                     "", &LastSet.iBinData32_ART[0][7],                          "ART Bin 7 Count"                                            );
        HGemPtr->SetSVDataPointer(1131 , HType.INT_4_TYPE,   "Bin 8 Count ART",                                                     "", &LastSet.iBinData32_ART[0][8],                          "ART Bin 8 Count"                                            );
        HGemPtr->SetSVDataPointer(1132 , HType.INT_4_TYPE,   "Bin 9 Count ART",                                                     "", &LastSet.iBinData32_ART[0][9],                          "ART Bin 9 Count"                                            );
        HGemPtr->SetSVDataPointer(1133 , HType.INT_4_TYPE,   "Bin 10 Count ART",                                                    "", &LastSet.iBinData32_ART[0][10],                         "ART Bin 10 Count"                                           );
        HGemPtr->SetSVDataPointer(1134 , HType.INT_4_TYPE,   "Bin 11 Count ART",                                                    "", &LastSet.iBinData32_ART[0][11],                         "ART Bin 11 Count"                                           );
        HGemPtr->SetSVDataPointer(1135 , HType.INT_4_TYPE,   "Bin 12 Count ART",                                                    "", &LastSet.iBinData32_ART[0][12],                         "ART Bin 12 Count"                                           );
        HGemPtr->SetSVDataPointer(1136 , HType.INT_4_TYPE,   "Bin 13 Count ART",                                                    "", &LastSet.iBinData32_ART[0][13],                         "ART Bin 13 Count"                                           );
        HGemPtr->SetSVDataPointer(1137 , HType.INT_4_TYPE,   "Bin 14 Count ART",                                                    "", &LastSet.iBinData32_ART[0][14],                         "ART Bin 14 Count"                                           );
        HGemPtr->SetSVDataPointer(1138 , HType.INT_4_TYPE,   "Bin 15 Count ART",                                                    "", &LastSet.iBinData32_ART[0][15],                         "ART Bin 15 Count"                                           );

        HGemPtr->SetSVDataPointer(1139 , HType.INT_4_TYPE,   "ATR FT PassCount",                                                    "", &iATRPassCount[0],                                      "ATR FT Pass TotalCount"                                     );//wei 20170119 (Steven) TSMC Secs Gem 回傳Pass/Fail顆數
        HGemPtr->SetSVDataPointer(1140 , HType.INT_4_TYPE,   "ATR FT FailCount",                                                    "", &iATRFailCount[0],                                      "ATR FT Fail TotalCount"                                     );//wei 20170119 (Steven) TSMC Secs Gem 回傳Pass/Fail顆數
        HGemPtr->SetSVDataPointer(1141 , HType.INT_4_TYPE,   "ATR FT LoaderCount",                                                  "", &iATRTotalCount[0],                                     "ATR FT Loader TotalCount"                                   );//wei 20170119 (Steven) TSMC Secs Gem 回傳Pass/Fail顆數
        HGemPtr->SetSVDataPointer(1142 , HType.INT_4_TYPE,   "ATR RT PassCount",                                                    "", &iATRPassCount[1],                                      "ATR RT Pass TotalCount"                                     );//wei 20170119 (Steven) TSMC Secs Gem 回傳Pass/Fail顆數
        HGemPtr->SetSVDataPointer(1143 , HType.INT_4_TYPE,   "ATR RT FailCount",                                                    "", &iATRFailCount[1],                                      "ATR RT Fail TotalCount"                                     );//wei 20170119 (Steven) TSMC Secs Gem 回傳Pass/Fail顆數
        HGemPtr->SetSVDataPointer(1144 , HType.INT_4_TYPE,   "ATR RT LoaderCount",                                                  "", &iATRTotalCount[1],                                     "ATR RT Loader TotalCount"                                   );//wei 20170119 (Steven) TSMC Secs Gem 回傳Pass/Fail顆數

        HGemPtr->SetSVDataPointer(1151 , HType.ASCII_TYPE,   "Auto1 Yield",                                                         "", &RunInfo.sT6AutoYield[eAuto1],                          "Auto 1 Yield"                                              );
        HGemPtr->SetSVDataPointer(1152 , HType.ASCII_TYPE,   "Auto2 Yield",                                                         "", &RunInfo.sT6AutoYield[eAuto2],                          "Auto 2 Yield"                                              );
        HGemPtr->SetSVDataPointer(1153 , HType.ASCII_TYPE,   "Auto3 Yield",                                                         "", &RunInfo.sT6AutoYield[eAuto3],                          "Auto 3 Yield"                                              );
        HGemPtr->SetSVDataPointer(1154 , HType.ASCII_TYPE,   "Fix1 Yield",                                                          "", &RunInfo.sT6AutoYield[eFix1 ],                          "Fix 1 Yield"                                               );
        HGemPtr->SetSVDataPointer(1155 , HType.ASCII_TYPE,   "Fix2 Yield",                                                          "", &RunInfo.sT6AutoYield[eFix2 ],                          "Fix 2 Yield"                                               );
        HGemPtr->SetSVDataPointer(1156 , HType.ASCII_TYPE,   "Fix3 Yield",                                                          "", &RunInfo.sT6AutoYield[eFix3 ],                          "Fix 3 Yield"                                               );
        HGemPtr->SetSVDataPointer(1157 , HType.ASCII_TYPE,   "Fix4 Yield",                                                          "", &RunInfo.sT6AutoYield[eFix4 ],                          "Fix 4 Yield"                                               );
        HGemPtr->SetSVDataPointer(1158 , HType.ASCII_TYPE,   "Fix5 Yield",                                                          "", &RunInfo.sT6AutoYield[eFix5 ],                          "Fix 5 Yield"                                               );
        HGemPtr->SetSVDataPointer(1159 , HType.ASCII_TYPE,   "Fix6 Yield",                                                          "", &RunInfo.sT6AutoYield[eFix6 ],                          "Fix 6 Yield"                                               );
        HGemPtr->SetSVDataPointer(1160 , HType.INT_4_TYPE,   "PassTotalCount",                                                      "", &iSECSGEMPass,                                          "Pass Total Count"                                            );//wei 20160923 Secs Gem 回傳Pass/Fail顆數
        HGemPtr->SetSVDataPointer(1161 , HType.INT_4_TYPE,   "FailTotalCount",                                                      "", &iSECSGEMFail,                                          "Fail Total"                                            );//wei 20160923 Secs Gem 回傳Pass/Fail顆數
#if 0 // GATE [G20] -- golden :215-230.  SVID 1164-1179 -- &iSVByBinCount[0..15]: DECLARED (cmydef.h:5916), definition (cmydef.cpp:6000) inside the same `#if 0 // TODO(W6)` block -> undefined at LINK.
        HGemPtr->SetSVDataPointer(1164 , HType.INT_4_TYPE,   "Bin 0 Count",                                                         "", &iSVByBinCount[0],                                      "Bin 0 Count"                                                ); //JerryYang 20250120 : add
        HGemPtr->SetSVDataPointer(1165 , HType.INT_4_TYPE,   "Bin 1 Count",                                                         "", &iSVByBinCount[1],                                      "Bin 1 Count"                                                );
        HGemPtr->SetSVDataPointer(1166 , HType.INT_4_TYPE,   "Bin 2 Count",                                                         "", &iSVByBinCount[2],                                      "Bin 2 Count"                                                );
        HGemPtr->SetSVDataPointer(1167 , HType.INT_4_TYPE,   "Bin 3 Count",                                                         "", &iSVByBinCount[3],                                      "Bin 3 Count"                                                );
        HGemPtr->SetSVDataPointer(1168 , HType.INT_4_TYPE,   "Bin 4 Count",                                                         "", &iSVByBinCount[4],                                      "Bin 4 Count"                                                );
        HGemPtr->SetSVDataPointer(1169 , HType.INT_4_TYPE,   "Bin 5 Count",                                                         "", &iSVByBinCount[5],                                      "Bin 5 Count"                                                );
        HGemPtr->SetSVDataPointer(1170 , HType.INT_4_TYPE,   "Bin 6 Count",                                                         "", &iSVByBinCount[6],                                      "Bin 6 Count"                                                );
        HGemPtr->SetSVDataPointer(1171 , HType.INT_4_TYPE,   "Bin 7 Count",                                                         "", &iSVByBinCount[7],                                      "Bin 7 Count"                                                );
        HGemPtr->SetSVDataPointer(1172 , HType.INT_4_TYPE,   "Bin 8 Count",                                                         "", &iSVByBinCount[8],                                      "Bin 8 Count"                                                );
        HGemPtr->SetSVDataPointer(1173 , HType.INT_4_TYPE,   "Bin 9 Count",                                                         "", &iSVByBinCount[9],                                      "Bin 9 Count"                                                );
        HGemPtr->SetSVDataPointer(1174 , HType.INT_4_TYPE,   "Bin 10 Count",                                                        "", &iSVByBinCount[10],                                     "Bin 10 Count"                                               );
        HGemPtr->SetSVDataPointer(1175 , HType.INT_4_TYPE,   "Bin 11 Count",                                                        "", &iSVByBinCount[11],                                     "Bin 11 Count"                                               );
        HGemPtr->SetSVDataPointer(1176 , HType.INT_4_TYPE,   "Bin 12 Count",                                                        "", &iSVByBinCount[12],                                     "Bin 12 Count"                                               );
        HGemPtr->SetSVDataPointer(1177 , HType.INT_4_TYPE,   "Bin 13 Count",                                                        "", &iSVByBinCount[13],                                     "Bin 13 Count"                                               );
        HGemPtr->SetSVDataPointer(1178 , HType.INT_4_TYPE,   "Bin 14 Count",                                                        "", &iSVByBinCount[14],                                     "Bin 14 Count"                                               );
        HGemPtr->SetSVDataPointer(1179 , HType.INT_4_TYPE,   "Bin 15 Count",                                                        "", &iSVByBinCount[15],                                     "Bin 15 Count"                                               );
#endif // GATE [G20]
        HGemPtr->SetSVDataPointer(1190 , HType.ASCII_TYPE,   "GET OS Tester Data",                                                  "", &fMain->SVID1190_OSSetup,                               "Data= Workfile, ClampV, UpLimit, LowLimit, Current, delayms"); //Steven 20230505 : 取得OS Tester資訊
#if 0 // GATE [G21] -- golden :232-232.  SVID 1191 -- &iSV_ErrBinCnt: DECLARED (cmydef.h:5911), definition (cmydef.cpp:5999) inside the same `#if 0 // TODO(W6)` block -> undefined at LINK.
        HGemPtr->SetSVDataPointer(1191 , HType.INT_4_TYPE,   "Error Bin Count",                                                     "", &iSV_ErrBinCnt,                                         "Error Bin Count"                                            );         //JerryYang 20250120 : add  //Steven 20260401 : USE_RFID_READER moved to ECID 1192
#endif // GATE [G21]
        //Ifor 20170316 add KYEC MRT Mode
        //==>
        HGemPtr->SetSVDataPointer(1220 , HType.INT_4_TYPE,   "Loader Count AUTO",                                                    "", &LastSet.SendCT_ART[0],                                "MRT Loading Count"                                               );
        HGemPtr->SetSVDataPointer(1221 , HType.INT_4_TYPE,   "Output Total Count AUTO",                                              "", &RunInfo.iUnloadCount_ART,                             "MRT Unloading Count"                                             );
        HGemPtr->SetSVDataPointer(1222 , HType.INT_4_TYPE,   "Auto1 Count AUTO",                                                     "", &LastSet.BinCT_ART[0][e3Auto1],                        "MRT Auto1 Count"                                            );
        HGemPtr->SetSVDataPointer(1223 , HType.INT_4_TYPE,   "Auto2 Count AUTO",                                                     "", &LastSet.BinCT_ART[0][e3Auto2],                        "MRT Auto2 Count"                                            );
        HGemPtr->SetSVDataPointer(1224 , HType.INT_4_TYPE,   "Auto3 Count AUTO",                                                     "", &LastSet.BinCT_ART[0][e3Auto3],                        "MRT Auto3 Count"                                            );
        HGemPtr->SetSVDataPointer(1225 , HType.INT_4_TYPE,   "Fix1 Count AUTO",                                                      "", &LastSet.BinCT_ART[0][e3Fix1 ],                        "MRT Fix1 Count"                                             );
        HGemPtr->SetSVDataPointer(1226 , HType.INT_4_TYPE,   "Fix2 Count AUTO",                                                      "", &LastSet.BinCT_ART[0][e3Fix2 ],                        "MRT Fix2 Count"                                             );
        HGemPtr->SetSVDataPointer(1227 , HType.INT_4_TYPE,   "Fix3 Count AUTO",                                                      "", &LastSet.BinCT_ART[0][e3Fix3 ],                        "MRT Fix3 Count"                                             );
        HGemPtr->SetSVDataPointer(1228 , HType.INT_4_TYPE,   "Fix4 Count AUTO",                                                      "", &LastSet.BinCT_ART[0][e3Fix4 ],                        "MRT Fix4 Count"                                             );
        HGemPtr->SetSVDataPointer(1229 , HType.INT_4_TYPE,   "Fix5 Count AUTO",                                                      "", &LastSet.BinCT_ART[0][e3Fix5 ],                        "MRT Fix5 Count"                                             );
        HGemPtr->SetSVDataPointer(1230 , HType.INT_4_TYPE,   "Fix6 Count AUTO",                                                      "", &LastSet.BinCT_ART[0][e3Fix6 ],                        "MRT Fix6 Count"                                             );
        HGemPtr->SetSVDataPointer(1231 , HType.INT_4_TYPE,   "Bin 0 Count AUTO",                                                     "", &LastSet.iBinData32_ART[0][0],                         "MRT Bin 0 Count"                                            );
        HGemPtr->SetSVDataPointer(1232 , HType.INT_4_TYPE,   "Bin 1 Count AUTO",                                                     "", &LastSet.iBinData32_ART[0][1],                         "MRT Bin 1 Count"                                            );
        HGemPtr->SetSVDataPointer(1233 , HType.INT_4_TYPE,   "Bin 2 Count AUTO",                                                     "", &LastSet.iBinData32_ART[0][2],                         "MRT Bin 2 Count"                                            );
        HGemPtr->SetSVDataPointer(1234 , HType.INT_4_TYPE,   "Bin 3 Count AUTO",                                                     "", &LastSet.iBinData32_ART[0][3],                         "MRT Bin 3 Count"                                            );
        HGemPtr->SetSVDataPointer(1235 , HType.INT_4_TYPE,   "Bin 4 Count AUTO",                                                     "", &LastSet.iBinData32_ART[0][4],                         "MRT Bin 4 Count"                                            );
        HGemPtr->SetSVDataPointer(1236 , HType.INT_4_TYPE,   "Bin 5 Count AUTO",                                                     "", &LastSet.iBinData32_ART[0][5],                         "MRT Bin 5 Count"                                            );
        HGemPtr->SetSVDataPointer(1237 , HType.INT_4_TYPE,   "Bin 6 Count AUTO",                                                     "", &LastSet.iBinData32_ART[0][6],                         "MRT Bin 6 Count"                                            );
        HGemPtr->SetSVDataPointer(1238 , HType.INT_4_TYPE,   "Bin 7 Count AUTO",                                                     "", &LastSet.iBinData32_ART[0][7],                         "MRT Bin 7 Count"                                            );
        HGemPtr->SetSVDataPointer(1239 , HType.INT_4_TYPE,   "Bin 8 Count AUTO",                                                     "", &LastSet.iBinData32_ART[0][8],                         "MRT Bin 8 Count"                                            );
        HGemPtr->SetSVDataPointer(1240 , HType.INT_4_TYPE,   "Bin 9 Count AUTO",                                                     "", &LastSet.iBinData32_ART[0][9],                         "MRT Bin 9 Count"                                            );
        HGemPtr->SetSVDataPointer(1241 , HType.INT_4_TYPE,   "Bin 10 Count AUTO",                                                    "", &LastSet.iBinData32_ART[0][10],                        "MRT Bin 10 Count"                                           );
        HGemPtr->SetSVDataPointer(1242 , HType.INT_4_TYPE,   "Bin 11 Count AUTO",                                                    "", &LastSet.iBinData32_ART[0][11],                        "MRT Bin 11 Count"                                           );
        HGemPtr->SetSVDataPointer(1243 , HType.INT_4_TYPE,   "Bin 12 Count AUTO",                                                    "", &LastSet.iBinData32_ART[0][12],                        "MRT Bin 12 Count"                                           );
        HGemPtr->SetSVDataPointer(1244 , HType.INT_4_TYPE,   "Bin 13 Count AUTO",                                                    "", &LastSet.iBinData32_ART[0][13],                        "MRT Bin 13 Count"                                           );
        HGemPtr->SetSVDataPointer(1245 , HType.INT_4_TYPE,   "Bin 14 Count AUTO",                                                    "", &LastSet.iBinData32_ART[0][14],                        "MRT Bin 14 Count"                                           );
        HGemPtr->SetSVDataPointer(1246 , HType.INT_4_TYPE,   "Bin 15 Count AUTO",                                                    "", &LastSet.iBinData32_ART[0][15],                        "MRT Bin 15 Count"                                           );
        //<==
        //Ifor 20170316 add KYEC MRT Mode

        HGemPtr->SetSVDataPointer(1250 , HType.INT_4_TYPE,   "Enabled Site Count",                                                   "", &iEnabledSiteCount,                                    "Enabled Site Count"                                         );

        //Steven 20230907 : For HT-9011UC
        HGemPtr->SetSVDataPointer(1259 , HType.INT_4_TYPE,   "Auto 4 Count",                                                         "", &LastSet.BinCT[0][e3Auto4],                            "Auto 4 Count"                                                );
        HGemPtr->SetSVDataPointer(1260 , HType.INT_4_TYPE,   "Auto 5 Count",                                                         "", &LastSet.BinCT[0][e3Auto5],                            "Auto 5 Count"                                                );
        HGemPtr->SetSVDataPointer(1261 , HType.INT_4_TYPE,   "Auto 6 Count",                                                         "", &LastSet.BinCT[0][e3Auto6],                            "Auto 6 Count"                                                );
        HGemPtr->SetSVDataPointer(1262 , HType.INT_4_TYPE,   "Fix 7 Count",                                                          "", &LastSet.BinCT[0][e3Fix7 ],                            "Fix 7 Count"                                                 );
        HGemPtr->SetSVDataPointer(1263 , HType.INT_4_TYPE,   "Fix 8 Count",                                                          "", &LastSet.BinCT[0][e3Fix8 ],                            "Fix 8 Count"                                                 );
        HGemPtr->SetSVDataPointer(1264 , HType.INT_4_TYPE,   "Fix 9 Count",                                                          "", &LastSet.BinCT[0][e3Fix9 ],                            "Fix 9 Count"                                                 );
        HGemPtr->SetSVDataPointer(1265 , HType.INT_4_TYPE,   "Fix 10 Count",                                                         "", &LastSet.BinCT[0][e3Fix10],                            "Fix 10 Count"                                                );
        HGemPtr->SetSVDataPointer(1266 , HType.INT_4_TYPE,   "Fix 11 Count",                                                         "", &LastSet.BinCT[0][e3Fix11],                            "Fix 11 Count"                                                );
        HGemPtr->SetSVDataPointer(1267 , HType.INT_4_TYPE,   "Fix 12 Count",                                                         "", &LastSet.BinCT[0][e3Fix12],                            "Fix 12 Count"                                                );

        HGemPtr->SetSVDataPointer(1268 , HType.INT_4_TYPE,   "Auto 4 Count ART",                                                     "", &LastSet.BinCT_ART[0][e3Auto4],                        "ART Auto 4 Count"                                            );
        HGemPtr->SetSVDataPointer(1269 , HType.INT_4_TYPE,   "Auto 5 Countunt ART",                                                  "", &LastSet.BinCT_ART[0][e3Auto6],                        "ART Auto 6 Count"                                            );
        HGemPtr->SetSVDataPointer(1271 , HType.INT_4_TYPE,   "Fix 7 Count ART",                                                      "", &LastSet.BinCT_ART[0][e3Fix7 ],                        "ART Fix 7 Count"                                             );
        HGemPtr->SetSVDataPointer(1272 , HType.INT_4_TYPE,   "Fix 8 Count ART",                                                      "", &LastSet.BinCT_ART[0][e3Fix8 ],                        "ART Fix 8 Count"                                             );
        HGemPtr->SetSVDataPointer(1273 , HType.INT_4_TYPE,   "Fix 9 Count ART",                                                      "", &LastSet.BinCT_ART[0][e3Fix9 ],                        "ART Fix 9 Count"                                             );
        HGemPtr->SetSVDataPointer(1274 , HType.INT_4_TYPE,   "Fix 10 Count ART",                                                     "", &LastSet.BinCT_ART[0][e3Fix10],                        "ART Fix 10 Count"                                            );
        HGemPtr->SetSVDataPointer(1275 , HType.INT_4_TYPE,   "Fix 11 Count ART",                                                     "", &LastSet.BinCT_ART[0][e3Fix11],                        "ART Fix 11 Count"                                            );
        HGemPtr->SetSVDataPointer(1276 , HType.INT_4_TYPE,   "Fix 12 Count ART",                                                     "", &LastSet.BinCT_ART[0][e3Fix12],                        "ART Fix 12 Count"                                            );

        HGemPtr->SetSVDataPointer(1277 , HType.ASCII_TYPE,   "Auto 4 Yield",                                                         "", &RunInfo.sT6AutoYield[eAuto4],                         "Auto 4 Yield"                                               );
        HGemPtr->SetSVDataPointer(1278 , HType.ASCII_TYPE,   "Auto 5 Yield",                                                         "", &RunInfo.sT6AutoYield[eAuto5],                         "Auto 5 Yield"                                               );
        HGemPtr->SetSVDataPointer(1279 , HType.ASCII_TYPE,   "Auto 6 Yield",                                                         "", &RunInfo.sT6AutoYield[eAuto6],                         "Auto 6 Yield"                                               );
        HGemPtr->SetSVDataPointer(1280 , HType.ASCII_TYPE,   "Fix 7 Yield",                                                          "", &RunInfo.sT6AutoYield[eFix7 ],                         "Fix 7 Yield"                                                );
        HGemPtr->SetSVDataPointer(1281 , HType.ASCII_TYPE,   "Fix 8 Yield",                                                          "", &RunInfo.sT6AutoYield[eFix8 ],                         "Fix 8 Yield"                                                );
        HGemPtr->SetSVDataPointer(1282 , HType.ASCII_TYPE,   "Fix 9 Yield",                                                          "", &RunInfo.sT6AutoYield[eFix9 ],                         "Fix 9 Yield"                                                );
        HGemPtr->SetSVDataPointer(1283 , HType.ASCII_TYPE,   "Fix 10 Yield",                                                         "", &RunInfo.sT6AutoYield[eFix10],                         "Fix 10 Yield"                                               );
        HGemPtr->SetSVDataPointer(1284 , HType.ASCII_TYPE,   "Fix 11 Yield",                                                         "", &RunInfo.sT6AutoYield[eFix11],                         "Fix 11 Yield"                                               );
        HGemPtr->SetSVDataPointer(1285 , HType.ASCII_TYPE,   "Fix 12 Yield",                                                         "", &RunInfo.sT6AutoYield[eFix12],                         "Fix 12 Yield"                                               );

        HGemPtr->SetSVDataPointer(1286 , HType.INT_4_TYPE,   "Auto 4 Count AUTO",                                                    "", &LastSet.BinCT_ART[0][e3Auto4],                        "MRTAuto 4 Count"                                             );
        HGemPtr->SetSVDataPointer(1287 , HType.INT_4_TYPE,   "Auto 5 Count AUTO",                                                    "", &LastSet.BinCT_ART[0][e3Auto5],                        "MRTAuto 5 Count"                                             );
        HGemPtr->SetSVDataPointer(1288 , HType.INT_4_TYPE,   "Auto 6 Count AUTO",                                                    "", &LastSet.BinCT_ART[0][e3Auto6],                        "MRTAuto 6 Count"                                             );
        HGemPtr->SetSVDataPointer(1289 , HType.INT_4_TYPE,   "Fix 7 Count AUTO",                                                     "", &LastSet.BinCT_ART[0][e3Fix7 ],                        "MRTFix 7 Count"                                              );
        HGemPtr->SetSVDataPointer(1290 , HType.INT_4_TYPE,   "Fix 8 Count AUTO",                                                     "", &LastSet.BinCT_ART[0][e3Fix8 ],                        "MRTFix 8 Count"                                              );
        HGemPtr->SetSVDataPointer(1291 , HType.INT_4_TYPE,   "Fix 9 Count AUTO",                                                     "", &LastSet.BinCT_ART[0][e3Fix9 ],                        "MRTFix 9 Count"                                              );
        HGemPtr->SetSVDataPointer(1292 , HType.INT_4_TYPE,   "Fix 10 Count AUTO",                                                    "", &LastSet.BinCT_ART[0][e3Fix10],                        "MRTFix 10 Count"                                             );
        HGemPtr->SetSVDataPointer(1293 , HType.INT_4_TYPE,   "Fix 11 Count AUTO",                                                    "", &LastSet.BinCT_ART[0][e3Fix11],                        "MRTFix 11 Count"                                             );
        HGemPtr->SetSVDataPointer(1294 , HType.INT_4_TYPE,   "Fix 12 Count AUTO",                                                    "", &LastSet.BinCT_ART[0][e3Fix12],                        "MRTFix 12 Count"                                             );

        if(iATC_Use_Heat_Count<=4)                                              //Steven 20220610 : 針對不同的ATC配置，提供不同的ATC Temp SVID
        {
#if 0 // GATE [G6] -- golden :309-316.  SVID 1351-1364 (iATC_Use_Heat_Count<=4 branch) -- fLotInfo->pl_ATCTempHead01..04 / pl_ATCRefHead01..04: NOT members of forms/fLotInfo.h's TfLotInfo.
            HGemPtr->SetSVDataPointer(1351 , HType.ASCII_TYPE,   "ATC Arm1 Head 1",                                                  "", fLotInfo->pl_ATCTempHead01,                            "ATC Arm1 Head 1"                                           );
            HGemPtr->SetSVDataPointer(1352 , HType.ASCII_TYPE,   "ATC Arm1 Head 2",                                                  "", fLotInfo->pl_ATCTempHead02,                            "ATC Arm1 Head 2"                                           );
            HGemPtr->SetSVDataPointer(1355 , HType.ASCII_TYPE,   "ATC Arm2 Head 1",                                                  "", fLotInfo->pl_ATCTempHead03,                            "ATC Arm2 Head 1"                                           );
            HGemPtr->SetSVDataPointer(1356 , HType.ASCII_TYPE,   "ATC Arm2 Head 2",                                                  "", fLotInfo->pl_ATCTempHead04,                            "ATC Arm2 Head 2"                                           );
            HGemPtr->SetSVDataPointer(1359 , HType.ASCII_TYPE,   "ATC Arm1 Ref Head 1",                                              "", fLotInfo->pl_ATCRefHead01,                             "ATC Arm1 Ref Head 1"                                       );
            HGemPtr->SetSVDataPointer(1360 , HType.ASCII_TYPE,   "ATC Arm1 Ref Head 2",                                              "", fLotInfo->pl_ATCRefHead02,                             "ATC Arm1 Ref Head 2"                                       );
            HGemPtr->SetSVDataPointer(1363 , HType.ASCII_TYPE,   "ATC Arm2 Ref Head 1",                                              "", fLotInfo->pl_ATCRefHead03,                             "ATC Arm2 Ref Head 1"                                       );
            HGemPtr->SetSVDataPointer(1364 , HType.ASCII_TYPE,   "ATC Arm2 Ref Head 2",                                              "", fLotInfo->pl_ATCRefHead04,                             "ATC Arm2 Ref Head 2"                                       );
#endif // GATE [G6]
        }
        else if(iATC_Use_Heat_Count<=8)
        {
#if 0 // GATE [G7] -- golden :320-335.  SVID 1351-1366 (iATC_Use_Heat_Count<=8 branch) -- fLotInfo->pl_ATCTempHead01..08 / pl_ATCRefHead01..08: ditto.
            HGemPtr->SetSVDataPointer(1351 , HType.ASCII_TYPE,   "ATC Arm1 Head 1",                                                  "", fLotInfo->pl_ATCTempHead01,                            "ATC Arm1 Head 1"                                           );
            HGemPtr->SetSVDataPointer(1352 , HType.ASCII_TYPE,   "ATC Arm1 Head 2",                                                  "", fLotInfo->pl_ATCTempHead02,                            "ATC Arm1 Head 2"                                           );
            HGemPtr->SetSVDataPointer(1353 , HType.ASCII_TYPE,   "ATC Arm1 Head 3",                                                  "", fLotInfo->pl_ATCTempHead03,                            "ATC Arm1 Head 3"                                           );
            HGemPtr->SetSVDataPointer(1354 , HType.ASCII_TYPE,   "ATC Arm1 Head 4",                                                  "", fLotInfo->pl_ATCTempHead04,                            "ATC Arm1 Head 4"                                           );
            HGemPtr->SetSVDataPointer(1355 , HType.ASCII_TYPE,   "ATC Arm2 Head 1",                                                  "", fLotInfo->pl_ATCTempHead05,                            "ATC Arm2 Head 1"                                           );
            HGemPtr->SetSVDataPointer(1356 , HType.ASCII_TYPE,   "ATC Arm2 Head 2",                                                  "", fLotInfo->pl_ATCTempHead06,                            "ATC Arm2 Head 2"                                           );
            HGemPtr->SetSVDataPointer(1357 , HType.ASCII_TYPE,   "ATC Arm2 Head 3",                                                  "", fLotInfo->pl_ATCTempHead07,                            "ATC Arm2 Head 3"                                           );
            HGemPtr->SetSVDataPointer(1358 , HType.ASCII_TYPE,   "ATC Arm2 Head 4",                                                  "", fLotInfo->pl_ATCTempHead08,                            "ATC Arm2 Head 4"                                           );
            HGemPtr->SetSVDataPointer(1359 , HType.ASCII_TYPE,   "ATC Arm1 Ref Head 1",                                              "", fLotInfo->pl_ATCRefHead01,                             "ATC Arm1 Ref Head 1"                                       );
            HGemPtr->SetSVDataPointer(1360 , HType.ASCII_TYPE,   "ATC Arm1 Ref Head 2",                                              "", fLotInfo->pl_ATCRefHead02,                             "ATC Arm1 Ref Head 2"                                       );
            HGemPtr->SetSVDataPointer(1361 , HType.ASCII_TYPE,   "ATC Arm1 Ref Head 3",                                              "", fLotInfo->pl_ATCRefHead03,                             "ATC Arm1 Ref Head 3"                                       );
            HGemPtr->SetSVDataPointer(1362 , HType.ASCII_TYPE,   "ATC Arm1 Ref Head 4",                                              "", fLotInfo->pl_ATCRefHead04,                             "ATC Arm1 Ref Head 4"                                       );
            HGemPtr->SetSVDataPointer(1363 , HType.ASCII_TYPE,   "ATC Arm2 Ref Head 1",                                              "", fLotInfo->pl_ATCRefHead05,                             "ATC Arm2 Ref Head 1"                                       );
            HGemPtr->SetSVDataPointer(1364 , HType.ASCII_TYPE,   "ATC Arm2 Ref Head 2",                                              "", fLotInfo->pl_ATCRefHead06,                             "ATC Arm2 Ref Head 2"                                       );
            HGemPtr->SetSVDataPointer(1365 , HType.ASCII_TYPE,   "ATC Arm2 Ref Head 3",                                              "", fLotInfo->pl_ATCRefHead07,                             "ATC Arm2 Ref Head 3"                                       );
            HGemPtr->SetSVDataPointer(1366 , HType.ASCII_TYPE,   "ATC Arm2 Ref Head 4",                                              "", fLotInfo->pl_ATCRefHead08,                             "ATC Arm2 Ref Head 4"                                       );
#endif // GATE [G7]
        }
        HGemPtr->SetSVDataPointer(1420 , HType.ASCII_TYPE,   "Site 1 to 32 Test Result",                                             "", fMain->tTestResult,                                     "Site 1 to 32 Test Result; Bin -1 is no test; CSV Format"   );

        HGemPtr->SetSVDataPointer(1432 , HType.INT_4_TYPE,   "Arm 1 Contact Count",                                                  "", &LastSet.iContactCT[0],                                 "Arm 1 Contact Count"                                       );
        HGemPtr->SetSVDataPointer(1433 , HType.INT_4_TYPE,   "Arm 2 Contact Count",                                                  "", &LastSet.iContactCT[1],                                 "Arm 2 Contact Count"                                       );

        HGemPtr->SetSVDataPointer(1451 , HType.ASCII_TYPE,   "Auto1 Test Bin",                                                       "", &sSVBinAssign[eAuto1],                               "Auto 1 Test Bin"                                           );        //JerryYang 20250320 : SPIL改用逗號分隔
        HGemPtr->SetSVDataPointer(1452 , HType.ASCII_TYPE,   "Auto2 Test Bin",                                                       "", &sSVBinAssign[eAuto2],                               "Auto 2 Test Bin"                                           );
        HGemPtr->SetSVDataPointer(1453 , HType.ASCII_TYPE,   "Auto3 Test Bin",                                                       "", &sSVBinAssign[eAuto3],                               "Auto 3 Test Bin"                                           );
        HGemPtr->SetSVDataPointer(1454 , HType.ASCII_TYPE,   "Fix1 Test Bin",                                                        "", &sSVBinAssign[eFix1],                                "Fix 1 Test Bin"                                            );
        HGemPtr->SetSVDataPointer(1455 , HType.ASCII_TYPE,   "Fix2 Test Bin",                                                        "", &sSVBinAssign[eFix2],                                "Fix 2 Test Bin"                                            );
        HGemPtr->SetSVDataPointer(1456 , HType.ASCII_TYPE,   "Fix3 Test Bin",                                                        "", &sSVBinAssign[eFix3],                                "Fix 3 Test Bin"                                            );
        HGemPtr->SetSVDataPointer(1457 , HType.ASCII_TYPE,   "Fix4 Test Bin",                                                        "", &sSVBinAssign[eFix4],                                "Fix 4 Test Bin"                                            );
        HGemPtr->SetSVDataPointer(1458 , HType.ASCII_TYPE,   "Fix5 Test Bin",                                                        "", &sSVBinAssign[eFix5],                                "Fix 5 Test Bin"                                            );
        HGemPtr->SetSVDataPointer(1459 , HType.ASCII_TYPE,   "Fix6 Test Bin",                                                        "", &sSVBinAssign[eFix6],                                "Fix 6 Test Bin"                                            );

        HGemPtr->SetSVDataPointer(1460, HType.ASCII_TYPE,    "Auto4 Test Bin",                                                       "", &sSVBinAssign[eAuto4],                                "Auto 4 Test Bin"                                           );
        HGemPtr->SetSVDataPointer(1461, HType.ASCII_TYPE,    "Auto5 Test Bin",                                                       "", &sSVBinAssign[eAuto5],                                "Auto 5 Test Bin"                                           );
        HGemPtr->SetSVDataPointer(1462, HType.ASCII_TYPE,    "Auto6 Test Bin",                                                       "", &sSVBinAssign[eAuto6],                                "Auto 6 Test Bin"                                           );
        HGemPtr->SetSVDataPointer(1463, HType.ASCII_TYPE,    "Fix7 Test Bin",                                                        "", &sSVBinAssign[eFix7],                              "Fix 7 Test Bin"                                            );
        HGemPtr->SetSVDataPointer(1464, HType.ASCII_TYPE,    "Fix8 Test Bin",                                                        "", &sSVBinAssign[eFix8],                              "Fix 8 Test Bin"                                            );
        HGemPtr->SetSVDataPointer(1465, HType.ASCII_TYPE,    "Fix9 Test Bin",                                                        "", &sSVBinAssign[eFix9],                              "Fix 9 Test Bin"                                            );
        HGemPtr->SetSVDataPointer(1466, HType.ASCII_TYPE,    "Fix10 Test Bin",                                                       "", &sSVBinAssign[eFix10],                              "Fix 10 Test Bin"                                           );
        HGemPtr->SetSVDataPointer(1467, HType.ASCII_TYPE,    "Fix11 Test Bin",                                                       "", &sSVBinAssign[eFix11],                              "Fix 11 Test Bin"                                           );
        HGemPtr->SetSVDataPointer(1468, HType.ASCII_TYPE,    "Fix12 Test Bin",                                                       "", &sSVBinAssign[eFix12],                              "Fix 12 Test Bin"                                           );

        HGemPtr->SetSVDataPointer(1470 , HType.ASCII_TYPE,   "CCD2 Temperature",                                                     "", &RunInfo.ShowTempComp[tcCCD_2],                         "CCD2 Temperature"                                          );
        HGemPtr->SetSVDataPointer(1471 , HType.ASCII_TYPE,   "2DID Temperature",                                                     "", &RunInfo.ShowTempComp[tc2D],                            "2DID Temperature"                                          );
        HGemPtr->SetSVDataPointer(1472 , HType.ASCII_TYPE,   "LB Temperature",                                                       "", &RunInfo.ShowTempComp[tcLB],                            "LB Temperature"                                            );
        HGemPtr->SetSVDataPointer(1473 , HType.ASCII_TYPE,   "ESD Air Temperature",                                                  "", &RunInfo.ShowTempComp[tcIndexESD],                      "ESD Air Temperature"                                       );

        HGemPtr->SetSVDataPointer(1702 , HType.INT_4_TYPE,   "Auto1 car Count",                                                      "", &iAuto_CarCarDeviceCnt[eAuto1],                         "Auto1 car Count"                                            );    //Sam20250331 : Unloader Auto 退出時上面的 IC 數量。
        HGemPtr->SetSVDataPointer(1703 , HType.INT_4_TYPE,   "Auto2 car Count",                                                      "", &iAuto_CarCarDeviceCnt[eAuto2],                         "Auto2 car Count"                                            );
        HGemPtr->SetSVDataPointer(1704 , HType.INT_4_TYPE,   "Auto3 car Count",                                                      "", &iAuto_CarCarDeviceCnt[eAuto3],                         "Auto3 car Count"                                            );
        HGemPtr->SetSVDataPointer(1705 , HType.INT_4_TYPE,   "Auto4 car Count",                                                      "", &iAuto_CarCarDeviceCnt[eAuto4],                         "Auto4 car Count"                                            );
        HGemPtr->SetSVDataPointer(1706 , HType.INT_4_TYPE,   "Auto5 car Count",                                                      "", &iAuto_CarCarDeviceCnt[eAuto5],                         "Auto5 car Count"                                            );
        HGemPtr->SetSVDataPointer(1707 , HType.INT_4_TYPE,   "Auto6 car Count",                                                      "", &iAuto_CarCarDeviceCnt[eAuto6],                         "Auto6 car Count"                                            );

#if 0 // GATE [G8] -- golden :374-377.  SVID 2003/2004/2005/2007 -- fContact->edForcePerDeviceKG / edForcePerPinN / edAirForceN / edForcePerDeviceN: not on TfContactShim.
        HGemPtr->SetSVDataPointer(2003 , HType.FT_8_TYPE,    "Force Per Device KG",                                                  "", fContact->edForcePerDeviceKG,                           "Force Per Device KG"                                       );
        HGemPtr->SetSVDataPointer(2004 , HType.FT_8_TYPE,    "Force Per Pin N",                                                      "", fContact->edForcePerPinN,                               "Force Per Pin N"                                           );
        HGemPtr->SetSVDataPointer(2005 , HType.FT_8_TYPE,    "Arm Total Force N",                                                    "", fContact->edAirForceN,                                  "Arm Total Force N"                                         );
        HGemPtr->SetSVDataPointer(2007 , HType.FT_8_TYPE,    "Force Per Device N",                                                   "", fContact->edForcePerDeviceN,                            "Force Per Device N"                                        );
#endif // GATE [G8]

        HGemPtr->SetSVDataPointer(2015 , HType.INT_4_TYPE,   "Contact Alarm Count Setting Grop 1",                                   "", &TestIF_File.iContactAlarmCount[0],                     "Contact Alarm Count Setting Grop 1"                        );//Ifor 20200112 add:Contact Alarm Count Setting
        HGemPtr->SetSVDataPointer(2016 , HType.INT_4_TYPE,   "Contact Alarm Count Setting Grop 2",                                   "", &TestIF_File.iContactAlarmCount[1],                     "Contact Alarm Count Setting Grop 2"                        );//Ifor 20200112 add:Contact Alarm Count Setting
        HGemPtr->SetSVDataPointer(2017 , HType.INT_4_TYPE,   "Contact Alarm Count Setting Grop 3",                                   "", &TestIF_File.iContactAlarmCount[2],                     "Contact Alarm Count Setting Grop 3"                        );//Ifor 20200112 add:Contact Alarm Count Setting

#if 0 // GATE [G9] -- golden :383-385.  SVID 2018/2019/2020 -- fContact->edAirKPA / lblReadEP / edSetKg: not on TfContactShim.
        HGemPtr->SetSVDataPointer(2018 , HType.FT_8_TYPE,    "Contact Air Force",                                                    "", fContact->edAirKPA,                                     "Contact Air Force"                                         );
        HGemPtr->SetSVDataPointer(2019 , HType.FT_8_TYPE,    "Contact Air Force Feedback",                                           "", fContact->lblReadEP,                                     "Contact Air Force Feedback"                                );
        HGemPtr->SetSVDataPointer(2020 , HType.FT_8_TYPE,    "Contact Set KG",                                                       "", fContact->edSetKg,                                      "Contact Set KG"                                            );
#endif // GATE [G9]
        //==> Eastsun 20260526 #026-1.79 Ifor 20220218 add:KYEC 要求新增Dual Force 開關
        HGemPtr->SetSVDataPointer(2022 , HType.BOOLEAN_TYPE, "Use Die Force",                                                       "", &DeviceForm_File.bUseDieForce,                          "Use Die Force"                                             );
        //<== Eastsun 20260526 #026-1.79
#if 0 // GATE [G10] -- golden :389-389.  SVID 2024 -- fContact->lblDieForceEP: not on TfContactShim.
        HGemPtr->SetSVDataPointer(2024 , HType.FT_8_TYPE,    "Die Force Feedback",                                                   "", fContact->lblDieForceEP,                                "Die Force Feedback"                                        );
#endif // GATE [G10]

        HGemPtr->SetSVDataPointer(2101 , HType.FT_8_TYPE,    "Arm1 Contact Height",                                                  "", &DeviceForm_File.IndexContact[0],                       "Arm1 Contact Height"                                       );
        HGemPtr->SetSVDataPointer(2102 , HType.FT_8_TYPE,    "Arm2 Contact Height",                                                  "", &DeviceForm_File.IndexContact[1],                       "Arm2 Contact Height"                                       );
        HGemPtr->SetSVDataPointer(2103 , HType.FT_8_TYPE,    "Shuttle1 Pick Height",                                                 "", &DeviceForm_File.IndexArmPick[0],                       "Shuttle1 Pick Height"                                      );
        HGemPtr->SetSVDataPointer(2104 , HType.FT_8_TYPE,    "Shuttle2 Pick Height",                                                 "", &DeviceForm_File.IndexArmPick[1],                       "Shuttle2 Release Height"                                   );
        HGemPtr->SetSVDataPointer(2105 , HType.FT_8_TYPE,    "Shuttle1 Release Height",                                              "", &DeviceForm_File.IndexPlace[0],                         "Shuttle1 Release Height"                                   );
        HGemPtr->SetSVDataPointer(2106 , HType.FT_8_TYPE,    "Shuttle2 Release Height",                                              "", &DeviceForm_File.IndexPlace[1],                         "Shuttle2 Release Height"                                   );
        //==> Eastsun 20260526 #026-4.A8 Ifor 20250717 add :KYEC
        HGemPtr->SetSVDataPointer(2200 , HType.INT_4_TYPE,   "Magazine 1 Count",                                                  "", &LastSet.BinCT[0][10],                                  "Magazine 1 Count"                                                       );
        HGemPtr->SetSVDataPointer(2201 , HType.INT_4_TYPE,   "Magazine 2 Count",                                                  "", &LastSet.BinCT[0][11],                                  "Magazine 2 Count"                                                       );
        HGemPtr->SetSVDataPointer(2202 , HType.INT_4_TYPE,   "Magazine 3 Count",                                                  "", &LastSet.BinCT[0][12],                                  "Magazine 3 Count"                                                       );
        HGemPtr->SetSVDataPointer(2203 , HType.INT_4_TYPE,   "Magazine 4 Count",                                                  "", &LastSet.BinCT[0][13],                                  "Magazine 4 Count"                                                       );
        HGemPtr->SetSVDataPointer(2204 , HType.INT_4_TYPE,   "Magazine 5 Count",                                                  "", &LastSet.BinCT[0][14],                                  "Magazine 5 Count"                                                       );
        HGemPtr->SetSVDataPointer(2205 , HType.INT_4_TYPE,   "Magazine 6 Count",                                                  "", &LastSet.BinCT[0][15],                                  "Magazine 6 Count"                                                       );
        HGemPtr->SetSVDataPointer(2206 , HType.INT_4_TYPE,   "Magazine 7 Count",                                                  "", &LastSet.BinCT[0][16],                                  "Magazine 7 Count"                                                       );
        HGemPtr->SetSVDataPointer(2207 , HType.INT_4_TYPE,   "Magazine 8 Count",                                                  "", &LastSet.BinCT[0][17],                                  "Magazine 8 Count"                                                       );
        HGemPtr->SetSVDataPointer(2208 , HType.INT_4_TYPE,   "Magazine 9 Count",                                                  "", &LastSet.BinCT[0][18],                                  "Magazine 9 Count"                                                       );
        HGemPtr->SetSVDataPointer(2209 , HType.INT_4_TYPE,   "Magazine 10 Count",                                                  "", &LastSet.BinCT[0][19],                                  "Magazine 10 Count"                                                       );
        HGemPtr->SetSVDataPointer(2210 , HType.INT_4_TYPE,   "Magazine 11 Count",                                                  "", &LastSet.BinCT[0][20],                                  "Magazine 11 Count"                                                       );
        HGemPtr->SetSVDataPointer(2211 , HType.INT_4_TYPE,   "Magazine 12 Count",                                                  "", &LastSet.BinCT[0][21],                                  "Magazine 12 Count"                                                       );
        HGemPtr->SetSVDataPointer(2212 , HType.INT_4_TYPE,   "Magazine 13 Count",                                                  "", &LastSet.BinCT[0][22],                                  "Magazine 13 Count"                                                       );
        HGemPtr->SetSVDataPointer(2213 , HType.INT_4_TYPE,   "Magazine 14 Count",                                                  "", &LastSet.BinCT[0][23],                                  "Magazine 14 Count"                                                       );
        HGemPtr->SetSVDataPointer(2220 , HType.INT_4_TYPE,   "Magazine 1 Count ART",                                              "", &LastSet.BinCT_ART[0][10],                              "Magazine 1 Count ART"                                                       );
        HGemPtr->SetSVDataPointer(2221 , HType.INT_4_TYPE,   "Magazine 2 Count ART",                                              "", &LastSet.BinCT_ART[0][11],                              "Magazine 2 Count ART"                                                       );
        HGemPtr->SetSVDataPointer(2222 , HType.INT_4_TYPE,   "Magazine 3 Count ART",                                              "", &LastSet.BinCT_ART[0][12],                              "Magazine 3 Count ART"                                                       );
        HGemPtr->SetSVDataPointer(2223 , HType.INT_4_TYPE,   "Magazine 4 Count ART",                                              "", &LastSet.BinCT_ART[0][13],                              "Magazine 4 Count ART"                                                       );
        HGemPtr->SetSVDataPointer(2224 , HType.INT_4_TYPE,   "Magazine 5 Count ART",                                              "", &LastSet.BinCT_ART[0][14],                              "Magazine 5 Count ART"                                                       );
        HGemPtr->SetSVDataPointer(2225 , HType.INT_4_TYPE,   "Magazine 6 Count ART",                                              "", &LastSet.BinCT_ART[0][15],                              "Magazine 6 Count ART"                                                       );
        HGemPtr->SetSVDataPointer(2226 , HType.INT_4_TYPE,   "Magazine 7 Count ART",                                              "", &LastSet.BinCT_ART[0][16],                              "Magazine 7 Count ART"                                                       );
        HGemPtr->SetSVDataPointer(2227 , HType.INT_4_TYPE,   "Magazine 8 Count ART",                                              "", &LastSet.BinCT_ART[0][17],                              "Magazine 8 Count ART"                                                       );
        HGemPtr->SetSVDataPointer(2228 , HType.INT_4_TYPE,   "Magazine 9 Count ART",                                              "", &LastSet.BinCT_ART[0][18],                              "Magazine 9 Count ART"                                                       );
        HGemPtr->SetSVDataPointer(2229 , HType.INT_4_TYPE,   "Magazine 10 Count ART",                                              "", &LastSet.BinCT_ART[0][19],                              "Magazine 10 Count ART"                                                       );
        HGemPtr->SetSVDataPointer(2230 , HType.INT_4_TYPE,   "Magazine 11 Count ART",                                              "", &LastSet.BinCT_ART[0][20],                              "Magazine 11 Count ART"                                                       );
        HGemPtr->SetSVDataPointer(2231 , HType.INT_4_TYPE,   "Magazine 12 Count ART",                                              "", &LastSet.BinCT_ART[0][21],                              "Magazine 12 Count ART"                                                       );
        HGemPtr->SetSVDataPointer(2232 , HType.INT_4_TYPE,   "Magazine 13 Count ART",                                              "", &LastSet.BinCT_ART[0][22],                              "Magazine 13 Count ART"                                                       );
        HGemPtr->SetSVDataPointer(2233 , HType.INT_4_TYPE,   "Magazine 14 Count ART",                                              "", &LastSet.BinCT_ART[0][23],                              "Magazine 14 Count ART"                                                       );
#if 0 // GATE [G11] -- golden :426-434.  SVID 2240-2248 -- fShowBinSelect->lblMag1..9: the fShowBinSelect global/form does not exist in the port at all.
        HGemPtr->SetSVDataPointer(2240 , HType.ASCII_TYPE,   "Magazine 1 Test Bin",                                               "", fShowBinSelect->lblMag1,                                  "Magazine 1 Test Bin"                                                       );
        HGemPtr->SetSVDataPointer(2241 , HType.ASCII_TYPE,   "Magazine 2 Test Bin",                                               "", fShowBinSelect->lblMag2,                                  "Magazine 2 Test Bin"                                                       );
        HGemPtr->SetSVDataPointer(2242 , HType.ASCII_TYPE,   "Magazine 3 Test Bin",                                               "", fShowBinSelect->lblMag3,                                  "Magazine 3 Test Bin"                                                       );
        HGemPtr->SetSVDataPointer(2243 , HType.ASCII_TYPE,   "Magazine 4 Test Bin",                                               "", fShowBinSelect->lblMag4,                                  "Magazine 4 Test Bin"                                                       );
        HGemPtr->SetSVDataPointer(2244 , HType.ASCII_TYPE,   "Magazine 5 Test Bin",                                               "", fShowBinSelect->lblMag5,                                  "Magazine 5 Test Bin"                                                       );
        HGemPtr->SetSVDataPointer(2245 , HType.ASCII_TYPE,   "Magazine 6 Test Bin",                                               "", fShowBinSelect->lblMag6,                                  "Magazine 6 Test Bin"                                                       );
        HGemPtr->SetSVDataPointer(2246 , HType.ASCII_TYPE,   "Magazine 7 Test Bin",                                               "", fShowBinSelect->lblMag7,                                  "Magazine 7 Test Bin"                                                       );
        HGemPtr->SetSVDataPointer(2247 , HType.ASCII_TYPE,   "Magazine 8 Test Bin",                                               "", fShowBinSelect->lblMag8,                                  "Magazine 8 Test Bin"                                                       );
        HGemPtr->SetSVDataPointer(2248 , HType.ASCII_TYPE,   "Magazine 9 Test Bin",                                               "", fShowBinSelect->lblMag9,                                  "Magazine 9 Test Bin"                                                       );
#endif // GATE [G11]
        HGemPtr->SetSVDataPointer(2630 , HType.BOOLEAN_TYPE, "Use One Touch Contact Height",                                       "", &RunInfo.bUseOneTouchContactHeight,                            "Use One Touch Contact Height"                                 );
        //<== Eastsun 20260526 #026-4.A8
#if 0 // GATE [G12] -- golden :437-437.  SVID 2631 -- fContact->rgHandlerMode: not on TfContactShim.
        HGemPtr->SetSVDataPointer(2631 , HType.INT_4_TYPE,   "Height Calibration Mode",                                              "", fContact->rgHandlerMode,                                "0:Normal; 1:Auto Height; 2:Contact Test; 3:Manual Test; "  );
#endif // GATE [G12]

#if 0 // GATE [G13] -- golden :439-445.  SVID 2665-2667 + 2754-2756 -- fTrayAssignment->edAuto1Type..edAuto6Type: the fTrayAssignment global/form does not exist in the port at all.
        HGemPtr->SetSVDataPointer(2665 , HType.ASCII_TYPE,   "Auto 4 Tray From Alias",                                               "", fTrayAssignment->edAuto4Type,                           "Auto 4 Tray From Alias"                                    );
        HGemPtr->SetSVDataPointer(2666 , HType.ASCII_TYPE,   "Auto 5 Tray From Alias",                                               "", fTrayAssignment->edAuto5Type,                           "Auto 5 Tray From Alias"                                    );
        HGemPtr->SetSVDataPointer(2667 , HType.ASCII_TYPE,   "Auto 6 Tray From Alias",                                               "", fTrayAssignment->edAuto6Type,                           "Auto 6 Tray From Alias"                                    );

        HGemPtr->SetSVDataPointer(2754 , HType.ASCII_TYPE,   "Auto 1 Tray From Alias",                                               "", fTrayAssignment->edAuto1Type,                           "Auto 1 Tray From Alias"                                    );
        HGemPtr->SetSVDataPointer(2755 , HType.ASCII_TYPE,   "Auto 2 Tray From Alias",                                               "", fTrayAssignment->edAuto2Type,                           "Auto 2 Tray From Alias"                                    );
        HGemPtr->SetSVDataPointer(2756 , HType.ASCII_TYPE,   "Auto 3 Tray From Alias",                                               "", fTrayAssignment->edAuto3Type,                           "Auto 3 Tray From Alias"                                    );
#endif // GATE [G13]
        //==> Eastsun 20260526 #026-4.A8 Ifor 20250717 add :KYEC
        HGemPtr->SetSVDataPointer(4490 , HType.INT_4_TYPE,   "EESUG Offest Data select location",                                  "", &RunInfo.iEESUGSelectSVID,                                     "EESUG Offest Data select location"                            );
        HGemPtr->SetSVDataPointer(4491 , HType.INT_4_TYPE,   "EESUG Offest Data modify location",                                  "", &RunInfo.iEESUGModify,                                         "EESUG Offest Data modify location"                            );
        HGemPtr->SetSVDataPointer(4492 , HType.ASCII_TYPE,   "EESUG Offest Unit",                                                  "", &RunInfo.asEESUGUnit,                                          "EESUG Offest Unit"                                            );
        HGemPtr->SetSVDataPointer(4850 , HType.BOOLEAN_TYPE, "Use pre-offset when change work temp",                               "", &Temperature.bATCPreOffset,                                    "Use pre-offset when change work temp"                         );
        HGemPtr->SetSVDataPointer(4851 , HType.BOOLEAN_TYPE, "Use Test complete, awaiting temperature.",                           "", &Temperature.bTestCompleteWaitTemp,                            "Use Test complete, awaiting temperature."                     );
        HGemPtr->SetSVDataPointer(4852 , HType.BOOLEAN_TYPE, "Use Test time below Setting, Next Contact Need Delay.",              "", &Temperature.bUseTestTimeBelowNeedDelay,                       "Use Test time below Setting, Next Contact Need Delay."        );
        HGemPtr->SetSVDataPointer(4853 , HType.FT_8_TYPE,    "Below Test Time Setting",                                            "", &Temperature.dTestBelowTime,                                   "Below Test Time Setting"                                      );
        HGemPtr->SetSVDataPointer(4854 , HType.FT_8_TYPE,    "Next Contact Need Delay Time Setting",                               "", &Temperature.dTestBelowDelayTime,                              "Next Contact Need Delay Time Setting"                         );
        //<== Eastsun 20260526 #026-4.A8
        HGemPtr->SetSVDataPointer(6001 , HType.INT_4_TYPE,   "Arm1 Total Contact Count",                                             "", &ArmData[0]->iContactCT,                                "Arm1 Total Contact Count"                                  );   //Steven 20150518 : 修正Contact Count
        HGemPtr->SetSVDataPointer(6002 , HType.INT_4_TYPE,   "Arm2 Total Contact Count",                                             "", &ArmData[1]->iContactCT,                                "Arm2 Total Contact Count"                                  );   //Steven 20150518 : 修正Contact Count
        HGemPtr->SetSVDataPointer(9001 , HType.INT_4_TYPE,   "Auto Clean Cleaning Count",                                            "", fCleaning->edCleaningCount,                             "Auto Clean Cleaning Count"                                 );
#if 0 // GATE [G14] -- golden :459-460.  SVID 9003/9004 -- fCleaning->edAutoCleanAirForce_Kg / _N: not members of forms/fCleaning.h's TfCleaning (edCleaningCount, SVID 9001, IS -- and stays ACTIVE).
        HGemPtr->SetSVDataPointer(9003 , HType.ASCII_TYPE,   "Auto Clean Arm Total Force(KG)",                                       "", fCleaning->edAutoCleanAirForce_Kg,                      "Auto Clean Arm Total Force(KG)"                            );  //Ifor 20190506 :TStaticText  ==>  TEdit
        HGemPtr->SetSVDataPointer(9004 , HType.ASCII_TYPE,   "Auto Clean Arm Total Force(N)",                                        "", fCleaning->edAutoCleanAirForce_N,                       "Auto Clean Arm Total Force(N)"                             );  //Ifor 20190506 :TStaticText  ==>  TEdit
#endif // GATE [G14]

        //==> Eastsun 20260526 #026-4.A8 Ifor 20250717 add :KYEC
        HGemPtr->SetSVDataPointer(9518 , HType.INT_4_TYPE,   "Auto Clean Mode",                                                    "", &TestIF_File.iAutoClean_Mode,                                  "Auto Clean Mode"                                              );
        //<== Eastsun 20260526 #026-4.A8

        HGemPtr->SetSVDataPointer(15500, HType.INT_4_TYPE,   "Pre Alarm Message",                                                    "", &RunInfo.iPreAlarmPosition,                             "Pre Alarm Message"                                         );  //Ifor 20171024 : add Pre Alarm Position 1:LD 2:Auto1 3:Auto2 4:Auto3 5:Fix1 6:Fix2 7:Fix3 8:Empty 9:Color 10:Auto4 11:Auto5 12:Auto6 13:Fix4 14:Fix5 15:Fix6

        HGemPtr->SetSVDataPointer(16200, HType.ASCII_TYPE,   "Site Aa Bin 0-255 Total",                                              "", ArmData[2]->ArmSKET[0][0]->sBinCT,                      "Site Aa Bin 0-255 TotalCSV Format"                         );
        HGemPtr->SetSVDataPointer(16201, HType.ASCII_TYPE,   "Site Ab Bin 0-255 Total",                                              "", ArmData[2]->ArmSKET[0][1]->sBinCT,                      "Site Ab Bin 0-255 TotalCSV Format"                         );
        HGemPtr->SetSVDataPointer(16202, HType.ASCII_TYPE,   "Site Ac Bin 0-255 Total",                                              "", ArmData[2]->ArmSKET[0][2]->sBinCT,                      "Site Ac Bin 0-255 TotalCSV Format"                         );
        HGemPtr->SetSVDataPointer(16203, HType.ASCII_TYPE,   "Site Ad Bin 0-255 Total",                                              "", ArmData[2]->ArmSKET[0][3]->sBinCT,                      "Site Ad Bin 0-255 TotalCSV Format"                         );
        HGemPtr->SetSVDataPointer(16204, HType.ASCII_TYPE,   "Site Ae Bin 0-255 Total",                                              "", ArmData[2]->ArmSKET[0][4]->sBinCT,                      "Site Ae Bin 0-255 TotalCSV Format"                         );
        HGemPtr->SetSVDataPointer(16205, HType.ASCII_TYPE,   "Site Af Bin 0-255 Total",                                              "", ArmData[2]->ArmSKET[0][5]->sBinCT,                      "Site Af Bin 0-255 TotalCSV Format"                         );
        HGemPtr->SetSVDataPointer(16206, HType.ASCII_TYPE,   "Site Ag Bin 0-255 Total",                                              "", ArmData[2]->ArmSKET[0][6]->sBinCT,                      "Site Ag Bin 0-255 TotalCSV Format"                         );
        HGemPtr->SetSVDataPointer(16207, HType.ASCII_TYPE,   "Site Ah Bin 0-255 Total",                                              "", ArmData[2]->ArmSKET[0][7]->sBinCT,                      "Site Ah Bin 0-255 TotalCSV Format"                         );
        HGemPtr->SetSVDataPointer(16208, HType.ASCII_TYPE,   "Site Ba Bin 0-255 Total",                                              "", ArmData[2]->ArmSKET[1][0]->sBinCT,                      "Site Ba Bin 0-255 TotalCSV Format"                         );
        HGemPtr->SetSVDataPointer(16209, HType.ASCII_TYPE,   "Site Bb Bin 0-255 Total",                                              "", ArmData[2]->ArmSKET[1][1]->sBinCT,                      "Site Bb Bin 0-255 TotalCSV Format"                         );
        HGemPtr->SetSVDataPointer(16210, HType.ASCII_TYPE,   "Site Bc Bin 0-255 Total",                                              "", ArmData[2]->ArmSKET[1][2]->sBinCT,                      "Site Bc Bin 0-255 TotalCSV Format"                         );
        HGemPtr->SetSVDataPointer(16211, HType.ASCII_TYPE,   "Site Bd Bin 0-255 Total",                                              "", ArmData[2]->ArmSKET[1][3]->sBinCT,                      "Site Bd Bin 0-255 TotalCSV Format"                         );
        HGemPtr->SetSVDataPointer(16212, HType.ASCII_TYPE,   "Site Be Bin 0-255 Total",                                              "", ArmData[2]->ArmSKET[1][4]->sBinCT,                      "Site Be Bin 0-255 TotalCSV Format"                         );
        HGemPtr->SetSVDataPointer(16213, HType.ASCII_TYPE,   "Site Bf Bin 0-255 Total",                                              "", ArmData[2]->ArmSKET[1][5]->sBinCT,                      "Site Bf Bin 0-255 TotalCSV Format"                         );
        HGemPtr->SetSVDataPointer(16214, HType.ASCII_TYPE,   "Site Bg Bin 0-255 Total",                                              "", ArmData[2]->ArmSKET[1][6]->sBinCT,                      "Site Bg Bin 0-255 TotalCSV Format"                         );
        HGemPtr->SetSVDataPointer(16215, HType.ASCII_TYPE,   "Site Bh Bin 0-255 Total",                                              "", ArmData[2]->ArmSKET[1][7]->sBinCT,                      "Site Bh Bin 0-255 TotalCSV Format"                         );
        HGemPtr->SetSVDataPointer(16216, HType.ASCII_TYPE,   "Site Ca Bin 0-255 Total",                                              "", ArmData[2]->ArmSKET[2][0]->sBinCT,                      "Site Ca Bin 0-255 TotalCSV Format"                         );
        HGemPtr->SetSVDataPointer(16217, HType.ASCII_TYPE,   "Site Cb Bin 0-255 Total",                                              "", ArmData[2]->ArmSKET[2][1]->sBinCT,                      "Site Cb Bin 0-255 TotalCSV Format"                         );
        HGemPtr->SetSVDataPointer(16218, HType.ASCII_TYPE,   "Site Cc Bin 0-255 Total",                                              "", ArmData[2]->ArmSKET[2][2]->sBinCT,                      "Site Cc Bin 0-255 TotalCSV Format"                         );
        HGemPtr->SetSVDataPointer(16219, HType.ASCII_TYPE,   "Site Cd Bin 0-255 Total",                                              "", ArmData[2]->ArmSKET[2][3]->sBinCT,                      "Site Cd Bin 0-255 TotalCSV Format"                         );
        HGemPtr->SetSVDataPointer(16220, HType.ASCII_TYPE,   "Site Ce Bin 0-255 Total",                                              "", ArmData[2]->ArmSKET[2][4]->sBinCT,                      "Site Ce Bin 0-255 TotalCSV Format"                         );
        HGemPtr->SetSVDataPointer(16221, HType.ASCII_TYPE,   "Site Cf Bin 0-255 Total",                                              "", ArmData[2]->ArmSKET[2][5]->sBinCT,                      "Site Cf Bin 0-255 TotalCSV Format"                         );
        HGemPtr->SetSVDataPointer(16222, HType.ASCII_TYPE,   "Site Cg Bin 0-255 Total",                                              "", ArmData[2]->ArmSKET[2][6]->sBinCT,                      "Site Cg Bin 0-255 TotalCSV Format"                         );
        HGemPtr->SetSVDataPointer(16223, HType.ASCII_TYPE,   "Site Ch Bin 0-255 Total",                                              "", ArmData[2]->ArmSKET[2][7]->sBinCT,                      "Site Ch Bin 0-255 TotalCSV Format"                         );
        HGemPtr->SetSVDataPointer(16224, HType.ASCII_TYPE,   "Site Da Bin 0-255 Total",                                              "", ArmData[2]->ArmSKET[3][0]->sBinCT,                      "Site Da Bin 0-255 TotalCSV Format"                         );
        HGemPtr->SetSVDataPointer(16225, HType.ASCII_TYPE,   "Site Db Bin 0-255 Total",                                              "", ArmData[2]->ArmSKET[3][1]->sBinCT,                      "Site Db Bin 0-255 TotalCSV Format"                         );
        HGemPtr->SetSVDataPointer(16226, HType.ASCII_TYPE,   "Site Dc Bin 0-255 Total",                                              "", ArmData[2]->ArmSKET[3][2]->sBinCT,                      "Site Dc Bin 0-255 TotalCSV Format"                         );
        HGemPtr->SetSVDataPointer(16227, HType.ASCII_TYPE,   "Site Dd Bin 0-255 Total",                                              "", ArmData[2]->ArmSKET[3][3]->sBinCT,                      "Site Dd Bin 0-255 TotalCSV Format"                         );
        HGemPtr->SetSVDataPointer(16228, HType.ASCII_TYPE,   "Site De Bin 0-255 Total",                                              "", ArmData[2]->ArmSKET[3][4]->sBinCT,                      "Site De Bin 0-255 TotalCSV Format"                         );
        HGemPtr->SetSVDataPointer(16229, HType.ASCII_TYPE,   "Site Df Bin 0-255 Total",                                              "", ArmData[2]->ArmSKET[3][5]->sBinCT,                      "Site Df Bin 0-255 TotalCSV Format"                         );
        HGemPtr->SetSVDataPointer(16230, HType.ASCII_TYPE,   "Site Dg Bin 0-255 Total",                                              "", ArmData[2]->ArmSKET[3][6]->sBinCT,                      "Site Dg Bin 0-255 TotalCSV Format"                         );
        HGemPtr->SetSVDataPointer(16231, HType.ASCII_TYPE,   "Site Dh Bin 0-255 Total",                                              "", ArmData[2]->ArmSKET[3][7]->sBinCT,                      "Site Dh Bin 0-255 TotalCSV Format"                         );
        HGemPtr->SetSVDataPointer(16232, HType.ASCII_TYPE,   "Arm 1 Site Aa Bin 0-255 Total",                                        "", ArmData[0]->ArmSKET[0][0]->sBinCT,                      "Arm 1 Site Aa Bin 0-255 TotalCSV Format"                   );
        HGemPtr->SetSVDataPointer(16233, HType.ASCII_TYPE,   "Arm 1 Site Ab Bin 0-255 Total",                                        "", ArmData[0]->ArmSKET[0][1]->sBinCT,                      "Arm 1 Site Ab Bin 0-255 TotalCSV Format"                   );
        HGemPtr->SetSVDataPointer(16234, HType.ASCII_TYPE,   "Arm 1 Site Ac Bin 0-255 Total",                                        "", ArmData[0]->ArmSKET[0][2]->sBinCT,                      "Arm 1 Site Ac Bin 0-255 TotalCSV Format"                   );
        HGemPtr->SetSVDataPointer(16235, HType.ASCII_TYPE,   "Arm 1 Site Ad Bin 0-255 Total",                                        "", ArmData[0]->ArmSKET[0][3]->sBinCT,                      "Arm 1 Site Ad Bin 0-255 TotalCSV Format"                   );
        HGemPtr->SetSVDataPointer(16236, HType.ASCII_TYPE,   "Arm 1 Site Ae Bin 0-255 Total",                                        "", ArmData[0]->ArmSKET[0][4]->sBinCT,                      "Arm 1 Site Ae Bin 0-255 TotalCSV Format"                   );
        HGemPtr->SetSVDataPointer(16237, HType.ASCII_TYPE,   "Arm 1 Site Af Bin 0-255 Total",                                        "", ArmData[0]->ArmSKET[0][5]->sBinCT,                      "Arm 1 Site Af Bin 0-255 TotalCSV Format"                   );
        HGemPtr->SetSVDataPointer(16238, HType.ASCII_TYPE,   "Arm 1 Site Ag Bin 0-255 Total",                                        "", ArmData[0]->ArmSKET[0][6]->sBinCT,                      "Arm 1 Site Ag Bin 0-255 TotalCSV Format"                   );
        HGemPtr->SetSVDataPointer(16239, HType.ASCII_TYPE,   "Arm 1 Site Ah Bin 0-255 Total",                                        "", ArmData[0]->ArmSKET[0][7]->sBinCT,                      "Arm 1 Site Ah Bin 0-255 TotalCSV Format"                   );
        HGemPtr->SetSVDataPointer(16240, HType.ASCII_TYPE,   "Arm 1 Site Ba Bin 0-255 Total",                                        "", ArmData[0]->ArmSKET[1][0]->sBinCT,                      "Arm 1 Site Ba Bin 0-255 TotalCSV Format"                   );
        HGemPtr->SetSVDataPointer(16241, HType.ASCII_TYPE,   "Arm 1 Site Bb Bin 0-255 Total",                                        "", ArmData[0]->ArmSKET[1][1]->sBinCT,                      "Arm 1 Site Bb Bin 0-255 TotalCSV Format"                   );
        HGemPtr->SetSVDataPointer(16242, HType.ASCII_TYPE,   "Arm 1 Site Bc Bin 0-255 Total",                                        "", ArmData[0]->ArmSKET[1][2]->sBinCT,                      "Arm 1 Site Bc Bin 0-255 TotalCSV Format"                   );
        HGemPtr->SetSVDataPointer(16243, HType.ASCII_TYPE,   "Arm 1 Site Bd Bin 0-255 Total",                                        "", ArmData[0]->ArmSKET[1][3]->sBinCT,                      "Arm 1 Site Bd Bin 0-255 TotalCSV Format"                   );
        HGemPtr->SetSVDataPointer(16244, HType.ASCII_TYPE,   "Arm 1 Site Be Bin 0-255 Total",                                        "", ArmData[0]->ArmSKET[1][4]->sBinCT,                      "Arm 1 Site Be Bin 0-255 TotalCSV Format"                   );
        HGemPtr->SetSVDataPointer(16245, HType.ASCII_TYPE,   "Arm 1 Site Bf Bin 0-255 Total",                                        "", ArmData[0]->ArmSKET[1][5]->sBinCT,                      "Arm 1 Site Bf Bin 0-255 TotalCSV Format"                   );
        HGemPtr->SetSVDataPointer(16246, HType.ASCII_TYPE,   "Arm 1 Site Bg Bin 0-255 Total",                                        "", ArmData[0]->ArmSKET[1][6]->sBinCT,                      "Arm 1 Site Bg Bin 0-255 TotalCSV Format"                   );
        HGemPtr->SetSVDataPointer(16247, HType.ASCII_TYPE,   "Arm 1 Site Bh Bin 0-255 Total",                                        "", ArmData[0]->ArmSKET[1][7]->sBinCT,                      "Arm 1 Site Bh Bin 0-255 TotalCSV Format"                   );
        HGemPtr->SetSVDataPointer(16248, HType.ASCII_TYPE,   "Arm 1 Site Ca Bin 0-255 Total",                                        "", ArmData[0]->ArmSKET[2][0]->sBinCT,                      "Arm 1 Site Ca Bin 0-255 TotalCSV Format"                   );
        HGemPtr->SetSVDataPointer(16249, HType.ASCII_TYPE,   "Arm 1 Site Cb Bin 0-255 Total",                                        "", ArmData[0]->ArmSKET[2][1]->sBinCT,                      "Arm 1 Site Cb Bin 0-255 TotalCSV Format"                   );
        HGemPtr->SetSVDataPointer(16250, HType.ASCII_TYPE,   "Arm 1 Site Cc Bin 0-255 Total",                                        "", ArmData[0]->ArmSKET[2][2]->sBinCT,                      "Arm 1 Site Cc Bin 0-255 TotalCSV Format"                   );
        HGemPtr->SetSVDataPointer(16251, HType.ASCII_TYPE,   "Arm 1 Site Cd Bin 0-255 Total",                                        "", ArmData[0]->ArmSKET[2][3]->sBinCT,                      "Arm 1 Site Cd Bin 0-255 TotalCSV Format"                   );
        HGemPtr->SetSVDataPointer(16252, HType.ASCII_TYPE,   "Arm 1 Site Ce Bin 0-255 Total",                                        "", ArmData[0]->ArmSKET[2][4]->sBinCT,                      "Arm 1 Site Ce Bin 0-255 TotalCSV Format"                   );
        HGemPtr->SetSVDataPointer(16253, HType.ASCII_TYPE,   "Arm 1 Site Cf Bin 0-255 Total",                                        "", ArmData[0]->ArmSKET[2][5]->sBinCT,                      "Arm 1 Site Cf Bin 0-255 TotalCSV Format"                   );
        HGemPtr->SetSVDataPointer(16254, HType.ASCII_TYPE,   "Arm 1 Site Cg Bin 0-255 Total",                                        "", ArmData[0]->ArmSKET[2][6]->sBinCT,                      "Arm 1 Site Cg Bin 0-255 TotalCSV Format"                   );
        HGemPtr->SetSVDataPointer(16255, HType.ASCII_TYPE,   "Arm 1 Site Ch Bin 0-255 Total",                                        "", ArmData[0]->ArmSKET[2][7]->sBinCT,                      "Arm 1 Site Ch Bin 0-255 TotalCSV Format"                   );
        HGemPtr->SetSVDataPointer(16256, HType.ASCII_TYPE,   "Arm 1 Site Da Bin 0-255 Total",                                        "", ArmData[0]->ArmSKET[3][0]->sBinCT,                      "Arm 1 Site Da Bin 0-255 TotalCSV Format"                   );
        HGemPtr->SetSVDataPointer(16257, HType.ASCII_TYPE,   "Arm 1 Site Db Bin 0-255 Total",                                        "", ArmData[0]->ArmSKET[3][1]->sBinCT,                      "Arm 1 Site Db Bin 0-255 TotalCSV Format"                   );
        HGemPtr->SetSVDataPointer(16258, HType.ASCII_TYPE,   "Arm 1 Site Dc Bin 0-255 Total",                                        "", ArmData[0]->ArmSKET[3][2]->sBinCT,                      "Arm 1 Site Dc Bin 0-255 TotalCSV Format"                   );
        HGemPtr->SetSVDataPointer(16259, HType.ASCII_TYPE,   "Arm 1 Site Dd Bin 0-255 Total",                                        "", ArmData[0]->ArmSKET[3][3]->sBinCT,                      "Arm 1 Site Dd Bin 0-255 TotalCSV Format"                   );
        HGemPtr->SetSVDataPointer(16260, HType.ASCII_TYPE,   "Arm 1 Site De Bin 0-255 Total",                                        "", ArmData[0]->ArmSKET[3][4]->sBinCT,                      "Arm 1 Site De Bin 0-255 TotalCSV Format"                   );
        HGemPtr->SetSVDataPointer(16261, HType.ASCII_TYPE,   "Arm 1 Site Df Bin 0-255 Total",                                        "", ArmData[0]->ArmSKET[3][5]->sBinCT,                      "Arm 1 Site Df Bin 0-255 TotalCSV Format"                   );
        HGemPtr->SetSVDataPointer(16262, HType.ASCII_TYPE,   "Arm 1 Site Dg Bin 0-255 Total",                                        "", ArmData[0]->ArmSKET[3][6]->sBinCT,                      "Arm 1 Site Dg Bin 0-255 TotalCSV Format"                   );
        HGemPtr->SetSVDataPointer(16263, HType.ASCII_TYPE,   "Arm 1 Site Dh Bin 0-255 Total",                                        "", ArmData[0]->ArmSKET[3][7]->sBinCT,                      "Arm 1 Site Dh Bin 0-255 TotalCSV Format"                   );
        HGemPtr->SetSVDataPointer(16264, HType.ASCII_TYPE,   "Arm 2 Site Aa Bin 0-255 Total",                                        "", ArmData[1]->ArmSKET[0][0]->sBinCT,                      "Arm 2 Site Aa Bin 0-255 TotalCSV Format"                   );
        HGemPtr->SetSVDataPointer(16265, HType.ASCII_TYPE,   "Arm 2 Site Ab Bin 0-255 Total",                                        "", ArmData[1]->ArmSKET[0][1]->sBinCT,                      "Arm 2 Site Ab Bin 0-255 TotalCSV Format"                   );
        HGemPtr->SetSVDataPointer(16266, HType.ASCII_TYPE,   "Arm 2 Site Ac Bin 0-255 Total",                                        "", ArmData[1]->ArmSKET[0][2]->sBinCT,                      "Arm 2 Site Ac Bin 0-255 TotalCSV Format"                   );
        HGemPtr->SetSVDataPointer(16267, HType.ASCII_TYPE,   "Arm 2 Site Ad Bin 0-255 Total",                                        "", ArmData[1]->ArmSKET[0][3]->sBinCT,                      "Arm 2 Site Ad Bin 0-255 TotalCSV Format"                   );
        HGemPtr->SetSVDataPointer(16268, HType.ASCII_TYPE,   "Arm 2 Site Ae Bin 0-255 Total",                                        "", ArmData[1]->ArmSKET[0][4]->sBinCT,                      "Arm 2 Site Ae Bin 0-255 TotalCSV Format"                   );
        HGemPtr->SetSVDataPointer(16269, HType.ASCII_TYPE,   "Arm 2 Site Af Bin 0-255 Total",                                        "", ArmData[1]->ArmSKET[0][5]->sBinCT,                      "Arm 2 Site Af Bin 0-255 TotalCSV Format"                   );
        HGemPtr->SetSVDataPointer(16270, HType.ASCII_TYPE,   "Arm 2 Site Ag Bin 0-255 Total",                                        "", ArmData[1]->ArmSKET[0][6]->sBinCT,                      "Arm 2 Site Ag Bin 0-255 TotalCSV Format"                   );
        HGemPtr->SetSVDataPointer(16271, HType.ASCII_TYPE,   "Arm 2 Site Ah Bin 0-255 Total",                                        "", ArmData[1]->ArmSKET[0][7]->sBinCT,                      "Arm 2 Site Ah Bin 0-255 TotalCSV Format"                   );
        HGemPtr->SetSVDataPointer(16272, HType.ASCII_TYPE,   "Arm 2 Site Ba Bin 0-255 Total",                                        "", ArmData[1]->ArmSKET[1][0]->sBinCT,                      "Arm 2 Site Ba Bin 0-255 TotalCSV Format"                   );
        HGemPtr->SetSVDataPointer(16273, HType.ASCII_TYPE,   "Arm 2 Site Bb Bin 0-255 Total",                                        "", ArmData[1]->ArmSKET[1][1]->sBinCT,                      "Arm 2 Site Bb Bin 0-255 TotalCSV Format"                   );
        HGemPtr->SetSVDataPointer(16274, HType.ASCII_TYPE,   "Arm 2 Site Bc Bin 0-255 Total",                                        "", ArmData[1]->ArmSKET[1][2]->sBinCT,                      "Arm 2 Site Bc Bin 0-255 TotalCSV Format"                   );
        HGemPtr->SetSVDataPointer(16275, HType.ASCII_TYPE,   "Arm 2 Site Bd Bin 0-255 Total",                                        "", ArmData[1]->ArmSKET[1][3]->sBinCT,                      "Arm 2 Site Bd Bin 0-255 TotalCSV Format"                   );
        HGemPtr->SetSVDataPointer(16276, HType.ASCII_TYPE,   "Arm 2 Site Be Bin 0-255 Total",                                        "", ArmData[1]->ArmSKET[1][4]->sBinCT,                      "Arm 2 Site Be Bin 0-255 TotalCSV Format"                   );
        HGemPtr->SetSVDataPointer(16277, HType.ASCII_TYPE,   "Arm 2 Site Bf Bin 0-255 Total",                                        "", ArmData[1]->ArmSKET[1][5]->sBinCT,                      "Arm 2 Site Bf Bin 0-255 TotalCSV Format"                   );
        HGemPtr->SetSVDataPointer(16278, HType.ASCII_TYPE,   "Arm 2 Site Bg Bin 0-255 Total",                                        "", ArmData[1]->ArmSKET[1][6]->sBinCT,                      "Arm 2 Site Bg Bin 0-255 TotalCSV Format"                   );
        HGemPtr->SetSVDataPointer(16279, HType.ASCII_TYPE,   "Arm 2 Site Bh Bin 0-255 Total",                                        "", ArmData[1]->ArmSKET[1][7]->sBinCT,                      "Arm 2 Site Bh Bin 0-255 TotalCSV Format"                   );
        HGemPtr->SetSVDataPointer(16280, HType.ASCII_TYPE,   "Arm 2 Site Ca Bin 0-255 Total",                                        "", ArmData[1]->ArmSKET[2][0]->sBinCT,                      "Arm 2 Site Ca Bin 0-255 TotalCSV Format"                   );
        HGemPtr->SetSVDataPointer(16281, HType.ASCII_TYPE,   "Arm 2 Site Cb Bin 0-255 Total",                                        "", ArmData[1]->ArmSKET[2][1]->sBinCT,                      "Arm 2 Site Cb Bin 0-255 TotalCSV Format"                   );
        HGemPtr->SetSVDataPointer(16282, HType.ASCII_TYPE,   "Arm 2 Site Cc Bin 0-255 Total",                                        "", ArmData[1]->ArmSKET[2][2]->sBinCT,                      "Arm 2 Site Cc Bin 0-255 TotalCSV Format"                   );
        HGemPtr->SetSVDataPointer(16283, HType.ASCII_TYPE,   "Arm 2 Site Cd Bin 0-255 Total",                                        "", ArmData[1]->ArmSKET[2][3]->sBinCT,                      "Arm 2 Site Cd Bin 0-255 TotalCSV Format"                   );
        HGemPtr->SetSVDataPointer(16284, HType.ASCII_TYPE,   "Arm 2 Site Ce Bin 0-255 Total",                                        "", ArmData[1]->ArmSKET[2][4]->sBinCT,                      "Arm 2 Site Ce Bin 0-255 TotalCSV Format"                   );
        HGemPtr->SetSVDataPointer(16285, HType.ASCII_TYPE,   "Arm 2 Site Cf Bin 0-255 Total",                                        "", ArmData[1]->ArmSKET[2][5]->sBinCT,                      "Arm 2 Site Cf Bin 0-255 TotalCSV Format"                   );
        HGemPtr->SetSVDataPointer(16286, HType.ASCII_TYPE,   "Arm 2 Site Cg Bin 0-255 Total",                                        "", ArmData[1]->ArmSKET[2][6]->sBinCT,                      "Arm 2 Site Cg Bin 0-255 TotalCSV Format"                   );
        HGemPtr->SetSVDataPointer(16287, HType.ASCII_TYPE,   "Arm 2 Site Ch Bin 0-255 Total",                                        "", ArmData[1]->ArmSKET[2][7]->sBinCT,                      "Arm 2 Site Ch Bin 0-255 TotalCSV Format"                   );
        HGemPtr->SetSVDataPointer(16288, HType.ASCII_TYPE,   "Arm 2 Site Da Bin 0-255 Total",                                        "", ArmData[1]->ArmSKET[3][0]->sBinCT,                      "Arm 2 Site Da Bin 0-255 TotalCSV Format"                   );
        HGemPtr->SetSVDataPointer(16289, HType.ASCII_TYPE,   "Arm 2 Site Db Bin 0-255 Total",                                        "", ArmData[1]->ArmSKET[3][1]->sBinCT,                      "Arm 2 Site Db Bin 0-255 TotalCSV Format"                   );
        HGemPtr->SetSVDataPointer(16290, HType.ASCII_TYPE,   "Arm 2 Site Dc Bin 0-255 Total",                                        "", ArmData[1]->ArmSKET[3][2]->sBinCT,                      "Arm 2 Site Dc Bin 0-255 TotalCSV Format"                   );
        HGemPtr->SetSVDataPointer(16291, HType.ASCII_TYPE,   "Arm 2 Site Dd Bin 0-255 Total",                                        "", ArmData[1]->ArmSKET[3][3]->sBinCT,                      "Arm 2 Site Dd Bin 0-255 TotalCSV Format"                   );
        HGemPtr->SetSVDataPointer(16292, HType.ASCII_TYPE,   "Arm 2 Site De Bin 0-255 Total",                                        "", ArmData[1]->ArmSKET[3][4]->sBinCT,                      "Arm 2 Site De Bin 0-255 TotalCSV Format"                   );
        HGemPtr->SetSVDataPointer(16293, HType.ASCII_TYPE,   "Arm 2 Site Df Bin 0-255 Total",                                        "", ArmData[1]->ArmSKET[3][5]->sBinCT,                      "Arm 2 Site Df Bin 0-255 TotalCSV Format"                   );
        HGemPtr->SetSVDataPointer(16294, HType.ASCII_TYPE,   "Arm 2 Site Dg Bin 0-255 Total",                                        "", ArmData[1]->ArmSKET[3][6]->sBinCT,                      "Arm 2 Site Dg Bin 0-255 TotalCSV Format"                   );
        HGemPtr->SetSVDataPointer(16295, HType.ASCII_TYPE,   "Arm 2 Site Dh Bin 0-255 Total",                                        "", ArmData[1]->ArmSKET[3][7]->sBinCT,                      "Arm 2 Site Dh Bin 0-255 TotalCSV Format"                   );
        HGemPtr->SetSVDataPointer(16296, HType.ASCII_TYPE,   "Total Pass Count by Site Aa to Dh",                                    "", ArmData[2]->sPass,                                      "Pass Count by Site Aa to DhCSV Format"                     );
        HGemPtr->SetSVDataPointer(16297, HType.ASCII_TYPE,   "Total Fail Count by Site Aa to Dh",                                    "", ArmData[2]->sFail,                                      "Fail Count by Site Aa to DhCSV Format"                     );
        HGemPtr->SetSVDataPointer(16298, HType.ASCII_TYPE,   "Total IF Error Count by Site Aa to Dh",                                "", ArmData[2]->iIFErr,                                     "IF Error Count by Site Aa to DhCSV Format"                 );
        HGemPtr->SetSVDataPointer(16299, HType.ASCII_TYPE,   "Total Count by Site Aa to Dh",                                         "", ArmData[2]->sTotal,                                     "Total Count by Site Aa to DhCSV Format"                    );
        HGemPtr->SetSVDataPointer(16300, HType.ASCII_TYPE,   "Arm 1 Pass Count by Site Aa to Dh",                                    "", ArmData[0]->sPass,                                      "Arm 1 Pass Count by Site Aa to DhCSV Format"               );
        HGemPtr->SetSVDataPointer(16301, HType.ASCII_TYPE,   "Arm 1 Fail Count by Site Aa to Dh",                                    "", ArmData[0]->sFail,                                      "Arm 1 Fail Count by Site Aa to DhCSV Format"               );
        HGemPtr->SetSVDataPointer(16302, HType.ASCII_TYPE,   "Arm 1 IF Error Count by Site Aa to Dh",                                "", ArmData[0]->iIFErr,                                     "Arm 1 IF Error Count by Site Aa to DhCSV Format"           );
        HGemPtr->SetSVDataPointer(16303, HType.ASCII_TYPE,   "Arm 1 Total Count by Site Aa to Dh",                                   "", ArmData[0]->sTotal,                                     "Arm 1 Total Count by Site Aa to DhCSV Format"              );
        HGemPtr->SetSVDataPointer(16304, HType.ASCII_TYPE,   "Arm 2 Pass Count by Site Aa to Dh",                                    "", ArmData[1]->sPass,                                      "Arm 2 Pass Count by Site Aa to DhCSV Format"               );
        HGemPtr->SetSVDataPointer(16305, HType.ASCII_TYPE,   "Arm 2 Fail Count by Site Aa to Dh",                                    "", ArmData[1]->sFail,                                      "Arm 2 Fail Count by Site Aa to DhCSV Format"               );
        HGemPtr->SetSVDataPointer(16306, HType.ASCII_TYPE,   "Arm 2 IF Error Count by Site Aa to Dh",                                "", ArmData[1]->iIFErr,                                     "Arm 2 IF Error Count by Site Aa to DhCSV Format"           );
        HGemPtr->SetSVDataPointer(16307, HType.ASCII_TYPE,   "Arm 2 Total Count by Site Aa to Dh",                                   "", ArmData[1]->sTotal,                                     "Arm 2 Total Count by Site Aa to DhCSV Format"              );
        HGemPtr->SetSVDataPointer(16308, HType.ASCII_TYPE,   "Site Aa Bin 0-255 Total (History)",                                    "", ArmHistory[2]->ArmSKET[0][0]->sBinCT,                   "Site Aa Bin 0-255 Total (History)CSV Format"               );
        HGemPtr->SetSVDataPointer(16309, HType.ASCII_TYPE,   "Site Ab Bin 0-255 Total (History)",                                    "", ArmHistory[2]->ArmSKET[0][1]->sBinCT,                   "Site Ab Bin 0-255 Total (History)CSV Format"               );
        HGemPtr->SetSVDataPointer(16310, HType.ASCII_TYPE,   "Site Ac Bin 0-255 Total (History)",                                    "", ArmHistory[2]->ArmSKET[0][2]->sBinCT,                   "Site Ac Bin 0-255 Total (History)CSV Format"               );
        HGemPtr->SetSVDataPointer(16311, HType.ASCII_TYPE,   "Site Ad Bin 0-255 Total (History)",                                    "", ArmHistory[2]->ArmSKET[0][3]->sBinCT,                   "Site Ad Bin 0-255 Total (History)CSV Format"               );
        HGemPtr->SetSVDataPointer(16312, HType.ASCII_TYPE,   "Site Ae Bin 0-255 Total (History)",                                    "", ArmHistory[2]->ArmSKET[0][4]->sBinCT,                   "Site Ae Bin 0-255 Total (History)CSV Format"               );
        HGemPtr->SetSVDataPointer(16313, HType.ASCII_TYPE,   "Site Af Bin 0-255 Total (History)",                                    "", ArmHistory[2]->ArmSKET[0][5]->sBinCT,                   "Site Af Bin 0-255 Total (History)CSV Format"               );
        HGemPtr->SetSVDataPointer(16314, HType.ASCII_TYPE,   "Site Ag Bin 0-255 Total (History)",                                    "", ArmHistory[2]->ArmSKET[0][6]->sBinCT,                   "Site Ag Bin 0-255 Total (History)CSV Format"               );
        HGemPtr->SetSVDataPointer(16315, HType.ASCII_TYPE,   "Site Ah Bin 0-255 Total (History)",                                    "", ArmHistory[2]->ArmSKET[0][7]->sBinCT,                   "Site Ah Bin 0-255 Total (History)CSV Format"               );
        HGemPtr->SetSVDataPointer(16316, HType.ASCII_TYPE,   "Site Ba Bin 0-255 Total (History)",                                    "", ArmHistory[2]->ArmSKET[1][0]->sBinCT,                   "Site Ba Bin 0-255 Total (History)CSV Format"               );
        HGemPtr->SetSVDataPointer(16317, HType.ASCII_TYPE,   "Site Bb Bin 0-255 Total (History)",                                    "", ArmHistory[2]->ArmSKET[1][1]->sBinCT,                   "Site Bb Bin 0-255 Total (History)CSV Format"               );
        HGemPtr->SetSVDataPointer(16318, HType.ASCII_TYPE,   "Site Bc Bin 0-255 Total (History)",                                    "", ArmHistory[2]->ArmSKET[1][2]->sBinCT,                   "Site Bc Bin 0-255 Total (History)CSV Format"               );
        HGemPtr->SetSVDataPointer(16319, HType.ASCII_TYPE,   "Site Bd Bin 0-255 Total (History)",                                    "", ArmHistory[2]->ArmSKET[1][3]->sBinCT,                   "Site Bd Bin 0-255 Total (History)CSV Format"               );
        HGemPtr->SetSVDataPointer(16320, HType.ASCII_TYPE,   "Site Be Bin 0-255 Total (History)",                                    "", ArmHistory[2]->ArmSKET[1][4]->sBinCT,                   "Site Be Bin 0-255 Total (History)CSV Format"               );
        HGemPtr->SetSVDataPointer(16321, HType.ASCII_TYPE,   "Site Bf Bin 0-255 Total (History)",                                    "", ArmHistory[2]->ArmSKET[1][5]->sBinCT,                   "Site Bf Bin 0-255 Total (History)CSV Format"               );
        HGemPtr->SetSVDataPointer(16322, HType.ASCII_TYPE,   "Site Bg Bin 0-255 Total (History)",                                    "", ArmHistory[2]->ArmSKET[1][6]->sBinCT,                   "Site Bg Bin 0-255 Total (History)CSV Format"               );
        HGemPtr->SetSVDataPointer(16323, HType.ASCII_TYPE,   "Site Bh Bin 0-255 Total (History)",                                    "", ArmHistory[2]->ArmSKET[1][7]->sBinCT,                   "Site Bh Bin 0-255 Total (History)CSV Format"               );
        HGemPtr->SetSVDataPointer(16324, HType.ASCII_TYPE,   "Site Ca Bin 0-255 Total (History)",                                    "", ArmHistory[2]->ArmSKET[2][0]->sBinCT,                   "Site Ca Bin 0-255 Total (History)CSV Format"               );
        HGemPtr->SetSVDataPointer(16325, HType.ASCII_TYPE,   "Site Cb Bin 0-255 Total (History)",                                    "", ArmHistory[2]->ArmSKET[2][1]->sBinCT,                   "Site Cb Bin 0-255 Total (History)CSV Format"               );
        HGemPtr->SetSVDataPointer(16326, HType.ASCII_TYPE,   "Site Cc Bin 0-255 Total (History)",                                    "", ArmHistory[2]->ArmSKET[2][2]->sBinCT,                   "Site Cc Bin 0-255 Total (History)CSV Format"               );
        HGemPtr->SetSVDataPointer(16327, HType.ASCII_TYPE,   "Site Cd Bin 0-255 Total (History)",                                    "", ArmHistory[2]->ArmSKET[2][3]->sBinCT,                   "Site Cd Bin 0-255 Total (History)CSV Format"               );
        HGemPtr->SetSVDataPointer(16328, HType.ASCII_TYPE,   "Site Ce Bin 0-255 Total (History)",                                    "", ArmHistory[2]->ArmSKET[2][4]->sBinCT,                   "Site Ce Bin 0-255 Total (History)CSV Format"               );
        HGemPtr->SetSVDataPointer(16329, HType.ASCII_TYPE,   "Site Cf Bin 0-255 Total (History)",                                    "", ArmHistory[2]->ArmSKET[2][5]->sBinCT,                   "Site Cf Bin 0-255 Total (History)CSV Format"               );
        HGemPtr->SetSVDataPointer(16330, HType.ASCII_TYPE,   "Site Cg Bin 0-255 Total (History)",                                    "", ArmHistory[2]->ArmSKET[2][6]->sBinCT,                   "Site Cg Bin 0-255 Total (History)CSV Format"               );
        HGemPtr->SetSVDataPointer(16331, HType.ASCII_TYPE,   "Site Ch Bin 0-255 Total (History)",                                    "", ArmHistory[2]->ArmSKET[2][7]->sBinCT,                   "Site Ch Bin 0-255 Total (History)CSV Format"               );
        HGemPtr->SetSVDataPointer(16332, HType.ASCII_TYPE,   "Site Da Bin 0-255 Total (History)",                                    "", ArmHistory[2]->ArmSKET[3][0]->sBinCT,                   "Site Da Bin 0-255 Total (History)CSV Format"               );
        HGemPtr->SetSVDataPointer(16333, HType.ASCII_TYPE,   "Site Db Bin 0-255 Total (History)",                                    "", ArmHistory[2]->ArmSKET[3][1]->sBinCT,                   "Site Db Bin 0-255 Total (History)CSV Format"               );
        HGemPtr->SetSVDataPointer(16334, HType.ASCII_TYPE,   "Site Dc Bin 0-255 Total (History)",                                    "", ArmHistory[2]->ArmSKET[3][2]->sBinCT,                   "Site Dc Bin 0-255 Total (History)CSV Format"               );
        HGemPtr->SetSVDataPointer(16335, HType.ASCII_TYPE,   "Site Dd Bin 0-255 Total (History)",                                    "", ArmHistory[2]->ArmSKET[3][3]->sBinCT,                   "Site Dd Bin 0-255 Total (History)CSV Format"               );
        HGemPtr->SetSVDataPointer(16336, HType.ASCII_TYPE,   "Site De Bin 0-255 Total (History)",                                    "", ArmHistory[2]->ArmSKET[3][4]->sBinCT,                   "Site De Bin 0-255 Total (History)CSV Format"               );
        HGemPtr->SetSVDataPointer(16337, HType.ASCII_TYPE,   "Site Df Bin 0-255 Total (History)",                                    "", ArmHistory[2]->ArmSKET[3][5]->sBinCT,                   "Site Df Bin 0-255 Total (History)CSV Format"               );
        HGemPtr->SetSVDataPointer(16338, HType.ASCII_TYPE,   "Site Dg Bin 0-255 Total (History)",                                    "", ArmHistory[2]->ArmSKET[3][6]->sBinCT,                   "Site Dg Bin 0-255 Total (History)CSV Format"               );
        HGemPtr->SetSVDataPointer(16339, HType.ASCII_TYPE,   "Site Dh Bin 0-255 Total (History)",                                    "", ArmHistory[2]->ArmSKET[3][7]->sBinCT,                   "Site Dh Bin 0-255 Total (History)CSV Format"               );
        HGemPtr->SetSVDataPointer(16340, HType.ASCII_TYPE,   "Arm 1 Site Aa Bin 0-255 Total (History)",                              "", ArmHistory[0]->ArmSKET[0][0]->sBinCT,                   "Arm 1 Site Aa Bin 0-255 Total (History)CSV Format"         );
        HGemPtr->SetSVDataPointer(16341, HType.ASCII_TYPE,   "Arm 1 Site Ab Bin 0-255 Total (History)",                              "", ArmHistory[0]->ArmSKET[0][1]->sBinCT,                   "Arm 1 Site Ab Bin 0-255 Total (History)CSV Format"         );
        HGemPtr->SetSVDataPointer(16342, HType.ASCII_TYPE,   "Arm 1 Site Ac Bin 0-255 Total (History)",                              "", ArmHistory[0]->ArmSKET[0][2]->sBinCT,                   "Arm 1 Site Ac Bin 0-255 Total (History)CSV Format"         );
        HGemPtr->SetSVDataPointer(16343, HType.ASCII_TYPE,   "Arm 1 Site Ad Bin 0-255 Total (History)",                              "", ArmHistory[0]->ArmSKET[0][3]->sBinCT,                   "Arm 1 Site Ad Bin 0-255 Total (History)CSV Format"         );
        HGemPtr->SetSVDataPointer(16344, HType.ASCII_TYPE,   "Arm 1 Site Ae Bin 0-255 Total (History)",                              "", ArmHistory[0]->ArmSKET[0][4]->sBinCT,                   "Arm 1 Site Ae Bin 0-255 Total (History)CSV Format"         );
        HGemPtr->SetSVDataPointer(16345, HType.ASCII_TYPE,   "Arm 1 Site Af Bin 0-255 Total (History)",                              "", ArmHistory[0]->ArmSKET[0][5]->sBinCT,                   "Arm 1 Site Af Bin 0-255 Total (History)CSV Format"         );
        HGemPtr->SetSVDataPointer(16346, HType.ASCII_TYPE,   "Arm 1 Site Ag Bin 0-255 Total (History)",                              "", ArmHistory[0]->ArmSKET[0][6]->sBinCT,                   "Arm 1 Site Ag Bin 0-255 Total (History)CSV Format"         );
        HGemPtr->SetSVDataPointer(16347, HType.ASCII_TYPE,   "Arm 1 Site Ah Bin 0-255 Total (History)",                              "", ArmHistory[0]->ArmSKET[0][7]->sBinCT,                   "Arm 1 Site Ah Bin 0-255 Total (History)CSV Format"         );
        HGemPtr->SetSVDataPointer(16348, HType.ASCII_TYPE,   "Arm 1 Site Ba Bin 0-255 Total (History)",                              "", ArmHistory[0]->ArmSKET[1][0]->sBinCT,                   "Arm 1 Site Ba Bin 0-255 Total (History)CSV Format"         );
        HGemPtr->SetSVDataPointer(16349, HType.ASCII_TYPE,   "Arm 1 Site Bb Bin 0-255 Total (History)",                              "", ArmHistory[0]->ArmSKET[1][1]->sBinCT,                   "Arm 1 Site Bb Bin 0-255 Total (History)CSV Format"         );
        HGemPtr->SetSVDataPointer(16350, HType.ASCII_TYPE,   "Arm 1 Site Bc Bin 0-255 Total (History)",                              "", ArmHistory[0]->ArmSKET[1][2]->sBinCT,                   "Arm 1 Site Bc Bin 0-255 Total (History)CSV Format"         );
        HGemPtr->SetSVDataPointer(16351, HType.ASCII_TYPE,   "Arm 1 Site Bd Bin 0-255 Total (History)",                              "", ArmHistory[0]->ArmSKET[1][3]->sBinCT,                   "Arm 1 Site Bd Bin 0-255 Total (History)CSV Format"         );
        HGemPtr->SetSVDataPointer(16352, HType.ASCII_TYPE,   "Arm 1 Site Be Bin 0-255 Total (History)",                              "", ArmHistory[0]->ArmSKET[1][4]->sBinCT,                   "Arm 1 Site Be Bin 0-255 Total (History)CSV Format"         );
        HGemPtr->SetSVDataPointer(16353, HType.ASCII_TYPE,   "Arm 1 Site Bf Bin 0-255 Total (History)",                              "", ArmHistory[0]->ArmSKET[1][5]->sBinCT,                   "Arm 1 Site Bf Bin 0-255 Total (History)CSV Format"         );
        HGemPtr->SetSVDataPointer(16354, HType.ASCII_TYPE,   "Arm 1 Site Bg Bin 0-255 Total (History)",                              "", ArmHistory[0]->ArmSKET[1][6]->sBinCT,                   "Arm 1 Site Bg Bin 0-255 Total (History)CSV Format"         );
        HGemPtr->SetSVDataPointer(16355, HType.ASCII_TYPE,   "Arm 1 Site Bh Bin 0-255 Total (History)",                              "", ArmHistory[0]->ArmSKET[1][7]->sBinCT,                   "Arm 1 Site Bh Bin 0-255 Total (History)CSV Format"         );
        HGemPtr->SetSVDataPointer(16356, HType.ASCII_TYPE,   "Arm 1 Site Ca Bin 0-255 Total (History)",                              "", ArmHistory[0]->ArmSKET[2][0]->sBinCT,                   "Arm 1 Site Ca Bin 0-255 Total (History)CSV Format"         );
        HGemPtr->SetSVDataPointer(16357, HType.ASCII_TYPE,   "Arm 1 Site Cb Bin 0-255 Total (History)",                              "", ArmHistory[0]->ArmSKET[2][1]->sBinCT,                   "Arm 1 Site Cb Bin 0-255 Total (History)CSV Format"         );
        HGemPtr->SetSVDataPointer(16358, HType.ASCII_TYPE,   "Arm 1 Site Cc Bin 0-255 Total (History)",                              "", ArmHistory[0]->ArmSKET[2][2]->sBinCT,                   "Arm 1 Site Cc Bin 0-255 Total (History)CSV Format"         );
        HGemPtr->SetSVDataPointer(16359, HType.ASCII_TYPE,   "Arm 1 Site Cd Bin 0-255 Total (History)",                              "", ArmHistory[0]->ArmSKET[2][3]->sBinCT,                   "Arm 1 Site Cd Bin 0-255 Total (History)CSV Format"         );
        HGemPtr->SetSVDataPointer(16360, HType.ASCII_TYPE,   "Arm 1 Site Ce Bin 0-255 Total (History)",                              "", ArmHistory[0]->ArmSKET[2][4]->sBinCT,                   "Arm 1 Site Ce Bin 0-255 Total (History)CSV Format"         );
        HGemPtr->SetSVDataPointer(16361, HType.ASCII_TYPE,   "Arm 1 Site Cf Bin 0-255 Total (History)",                              "", ArmHistory[0]->ArmSKET[2][5]->sBinCT,                   "Arm 1 Site Cf Bin 0-255 Total (History)CSV Format"         );
        HGemPtr->SetSVDataPointer(16362, HType.ASCII_TYPE,   "Arm 1 Site Cg Bin 0-255 Total (History)",                              "", ArmHistory[0]->ArmSKET[2][6]->sBinCT,                   "Arm 1 Site Cg Bin 0-255 Total (History)CSV Format"         );
        HGemPtr->SetSVDataPointer(16363, HType.ASCII_TYPE,   "Arm 1 Site Ch Bin 0-255 Total (History)",                              "", ArmHistory[0]->ArmSKET[2][7]->sBinCT,                   "Arm 1 Site Ch Bin 0-255 Total (History)CSV Format"         );
        HGemPtr->SetSVDataPointer(16364, HType.ASCII_TYPE,   "Arm 1 Site Da Bin 0-255 Total (History)",                              "", ArmHistory[0]->ArmSKET[3][0]->sBinCT,                   "Arm 1 Site Da Bin 0-255 Total (History)CSV Format"         );
        HGemPtr->SetSVDataPointer(16365, HType.ASCII_TYPE,   "Arm 1 Site Db Bin 0-255 Total (History)",                              "", ArmHistory[0]->ArmSKET[3][1]->sBinCT,                   "Arm 1 Site Db Bin 0-255 Total (History)CSV Format"         );
        HGemPtr->SetSVDataPointer(16366, HType.ASCII_TYPE,   "Arm 1 Site Dc Bin 0-255 Total (History)",                              "", ArmHistory[0]->ArmSKET[3][2]->sBinCT,                   "Arm 1 Site Dc Bin 0-255 Total (History)CSV Format"         );
        HGemPtr->SetSVDataPointer(16367, HType.ASCII_TYPE,   "Arm 1 Site Dd Bin 0-255 Total (History)",                              "", ArmHistory[0]->ArmSKET[3][3]->sBinCT,                   "Arm 1 Site Dd Bin 0-255 Total (History)CSV Format"         );
        HGemPtr->SetSVDataPointer(16368, HType.ASCII_TYPE,   "Arm 1 Site De Bin 0-255 Total (History)",                              "", ArmHistory[0]->ArmSKET[3][4]->sBinCT,                   "Arm 1 Site De Bin 0-255 Total (History)CSV Format"         );
        HGemPtr->SetSVDataPointer(16369, HType.ASCII_TYPE,   "Arm 1 Site Df Bin 0-255 Total (History)",                              "", ArmHistory[0]->ArmSKET[3][5]->sBinCT,                   "Arm 1 Site Df Bin 0-255 Total (History)CSV Format"         );
        HGemPtr->SetSVDataPointer(16370, HType.ASCII_TYPE,   "Arm 1 Site Dg Bin 0-255 Total (History)",                              "", ArmHistory[0]->ArmSKET[3][6]->sBinCT,                   "Arm 1 Site Dg Bin 0-255 Total (History)CSV Format"         );
        HGemPtr->SetSVDataPointer(16371, HType.ASCII_TYPE,   "Arm 1 Site Dh Bin 0-255 Total (History)",                              "", ArmHistory[0]->ArmSKET[3][7]->sBinCT,                   "Arm 1 Site Dh Bin 0-255 Total (History)CSV Format"         );
        HGemPtr->SetSVDataPointer(16372, HType.ASCII_TYPE,   "Arm 2 Site Aa Bin 0-255 Total (History)",                              "", ArmHistory[1]->ArmSKET[0][0]->sBinCT,                   "Arm 2 Site Aa Bin 0-255 Total (History)CSV Format"         );
        HGemPtr->SetSVDataPointer(16373, HType.ASCII_TYPE,   "Arm 2 Site Ab Bin 0-255 Total (History)",                              "", ArmHistory[1]->ArmSKET[0][1]->sBinCT,                   "Arm 2 Site Ab Bin 0-255 Total (History)CSV Format"         );
        HGemPtr->SetSVDataPointer(16374, HType.ASCII_TYPE,   "Arm 2 Site Ac Bin 0-255 Total (History)",                              "", ArmHistory[1]->ArmSKET[0][2]->sBinCT,                   "Arm 2 Site Ac Bin 0-255 Total (History)CSV Format"         );
        HGemPtr->SetSVDataPointer(16375, HType.ASCII_TYPE,   "Arm 2 Site Ad Bin 0-255 Total (History)",                              "", ArmHistory[1]->ArmSKET[0][3]->sBinCT,                   "Arm 2 Site Ad Bin 0-255 Total (History)CSV Format"         );
        HGemPtr->SetSVDataPointer(16376, HType.ASCII_TYPE,   "Arm 2 Site Ae Bin 0-255 Total (History)",                              "", ArmHistory[1]->ArmSKET[0][4]->sBinCT,                   "Arm 2 Site Ae Bin 0-255 Total (History)CSV Format"         );
        HGemPtr->SetSVDataPointer(16377, HType.ASCII_TYPE,   "Arm 2 Site Af Bin 0-255 Total (History)",                              "", ArmHistory[1]->ArmSKET[0][5]->sBinCT,                   "Arm 2 Site Af Bin 0-255 Total (History)CSV Format"         );
        HGemPtr->SetSVDataPointer(16378, HType.ASCII_TYPE,   "Arm 2 Site Ag Bin 0-255 Total (History)",                              "", ArmHistory[1]->ArmSKET[0][6]->sBinCT,                   "Arm 2 Site Ag Bin 0-255 Total (History)CSV Format"         );
        HGemPtr->SetSVDataPointer(16379, HType.ASCII_TYPE,   "Arm 2 Site Ah Bin 0-255 Total (History)",                              "", ArmHistory[1]->ArmSKET[0][7]->sBinCT,                   "Arm 2 Site Ah Bin 0-255 Total (History)CSV Format"         );
        HGemPtr->SetSVDataPointer(16380, HType.ASCII_TYPE,   "Arm 2 Site Ba Bin 0-255 Total (History)",                              "", ArmHistory[1]->ArmSKET[1][0]->sBinCT,                   "Arm 2 Site Ba Bin 0-255 Total (History)CSV Format"         );
        HGemPtr->SetSVDataPointer(16381, HType.ASCII_TYPE,   "Arm 2 Site Bb Bin 0-255 Total (History)",                              "", ArmHistory[1]->ArmSKET[1][1]->sBinCT,                   "Arm 2 Site Bb Bin 0-255 Total (History)CSV Format"         );
        HGemPtr->SetSVDataPointer(16382, HType.ASCII_TYPE,   "Arm 2 Site Bc Bin 0-255 Total (History)",                              "", ArmHistory[1]->ArmSKET[1][2]->sBinCT,                   "Arm 2 Site Bc Bin 0-255 Total (History)CSV Format"         );
        HGemPtr->SetSVDataPointer(16383, HType.ASCII_TYPE,   "Arm 2 Site Bd Bin 0-255 Total (History)",                              "", ArmHistory[1]->ArmSKET[1][3]->sBinCT,                   "Arm 2 Site Bd Bin 0-255 Total (History)CSV Format"         );
        HGemPtr->SetSVDataPointer(16384, HType.ASCII_TYPE,   "Arm 2 Site Be Bin 0-255 Total (History)",                              "", ArmHistory[1]->ArmSKET[1][4]->sBinCT,                   "Arm 2 Site Be Bin 0-255 Total (History)CSV Format"         );
        HGemPtr->SetSVDataPointer(16385, HType.ASCII_TYPE,   "Arm 2 Site Bf Bin 0-255 Total (History)",                              "", ArmHistory[1]->ArmSKET[1][5]->sBinCT,                   "Arm 2 Site Bf Bin 0-255 Total (History)CSV Format"         );
        HGemPtr->SetSVDataPointer(16386, HType.ASCII_TYPE,   "Arm 2 Site Bg Bin 0-255 Total (History)",                              "", ArmHistory[1]->ArmSKET[1][6]->sBinCT,                   "Arm 2 Site Bg Bin 0-255 Total (History)CSV Format"         );
        HGemPtr->SetSVDataPointer(16387, HType.ASCII_TYPE,   "Arm 2 Site Bh Bin 0-255 Total (History)",                              "", ArmHistory[1]->ArmSKET[1][7]->sBinCT,                   "Arm 2 Site Bh Bin 0-255 Total (History)CSV Format"         );
        HGemPtr->SetSVDataPointer(16388, HType.ASCII_TYPE,   "Arm 2 Site Ca Bin 0-255 Total (History)",                              "", ArmHistory[1]->ArmSKET[2][0]->sBinCT,                   "Arm 2 Site Ca Bin 0-255 Total (History)CSV Format"         );
        HGemPtr->SetSVDataPointer(16389, HType.ASCII_TYPE,   "Arm 2 Site Cb Bin 0-255 Total (History)",                              "", ArmHistory[1]->ArmSKET[2][1]->sBinCT,                   "Arm 2 Site Cb Bin 0-255 Total (History)CSV Format"         );
        HGemPtr->SetSVDataPointer(16390, HType.ASCII_TYPE,   "Arm 2 Site Cc Bin 0-255 Total (History)",                              "", ArmHistory[1]->ArmSKET[2][2]->sBinCT,                   "Arm 2 Site Cc Bin 0-255 Total (History)CSV Format"         );
        HGemPtr->SetSVDataPointer(16391, HType.ASCII_TYPE,   "Arm 2 Site Cd Bin 0-255 Total (History)",                              "", ArmHistory[1]->ArmSKET[2][3]->sBinCT,                   "Arm 2 Site Cd Bin 0-255 Total (History)CSV Format"         );
        HGemPtr->SetSVDataPointer(16392, HType.ASCII_TYPE,   "Arm 2 Site Ce Bin 0-255 Total (History)",                              "", ArmHistory[1]->ArmSKET[2][4]->sBinCT,                   "Arm 2 Site Ce Bin 0-255 Total (History)CSV Format"         );
        HGemPtr->SetSVDataPointer(16393, HType.ASCII_TYPE,   "Arm 2 Site Cf Bin 0-255 Total (History)",                              "", ArmHistory[1]->ArmSKET[2][5]->sBinCT,                   "Arm 2 Site Cf Bin 0-255 Total (History)CSV Format"         );
        HGemPtr->SetSVDataPointer(16394, HType.ASCII_TYPE,   "Arm 2 Site Cg Bin 0-255 Total (History)",                              "", ArmHistory[1]->ArmSKET[2][6]->sBinCT,                   "Arm 2 Site Cg Bin 0-255 Total (History)CSV Format"         );
        HGemPtr->SetSVDataPointer(16395, HType.ASCII_TYPE,   "Arm 2 Site Ch Bin 0-255 Total (History)",                              "", ArmHistory[1]->ArmSKET[2][7]->sBinCT,                   "Arm 2 Site Ch Bin 0-255 Total (History)CSV Format"         );
        HGemPtr->SetSVDataPointer(16396, HType.ASCII_TYPE,   "Arm 2 Site Da Bin 0-255 Total (History)",                              "", ArmHistory[1]->ArmSKET[3][0]->sBinCT,                   "Arm 2 Site Da Bin 0-255 Total (History)CSV Format"         );
        HGemPtr->SetSVDataPointer(16397, HType.ASCII_TYPE,   "Arm 2 Site Db Bin 0-255 Total (History)",                              "", ArmHistory[1]->ArmSKET[3][1]->sBinCT,                   "Arm 2 Site Db Bin 0-255 Total (History)CSV Format"         );
        HGemPtr->SetSVDataPointer(16398, HType.ASCII_TYPE,   "Arm 2 Site Dc Bin 0-255 Total (History)",                              "", ArmHistory[1]->ArmSKET[3][2]->sBinCT,                   "Arm 2 Site Dc Bin 0-255 Total (History)CSV Format"         );
        HGemPtr->SetSVDataPointer(16399, HType.ASCII_TYPE,   "Arm 2 Site Dd Bin 0-255 Total (History)",                              "", ArmHistory[1]->ArmSKET[3][3]->sBinCT,                   "Arm 2 Site Dd Bin 0-255 Total (History)CSV Format"         );
        HGemPtr->SetSVDataPointer(16400, HType.ASCII_TYPE,   "Arm 2 Site De Bin 0-255 Total (History)",                              "", ArmHistory[1]->ArmSKET[3][4]->sBinCT,                   "Arm 2 Site De Bin 0-255 Total (History)CSV Format"         );
        HGemPtr->SetSVDataPointer(16401, HType.ASCII_TYPE,   "Arm 2 Site Df Bin 0-255 Total (History)",                              "", ArmHistory[1]->ArmSKET[3][5]->sBinCT,                   "Arm 2 Site Df Bin 0-255 Total (History)CSV Format"         );
        HGemPtr->SetSVDataPointer(16402, HType.ASCII_TYPE,   "Arm 2 Site Dg Bin 0-255 Total (History)",                              "", ArmHistory[1]->ArmSKET[3][6]->sBinCT,                   "Arm 2 Site Dg Bin 0-255 Total (History)CSV Format"         );
        HGemPtr->SetSVDataPointer(16403, HType.ASCII_TYPE,   "Arm 2 Site Dh Bin 0-255 Total (History)",                              "", ArmHistory[1]->ArmSKET[3][7]->sBinCT,                   "Arm 2 Site Dh Bin 0-255 Total (History)CSV Format"         );
        HGemPtr->SetSVDataPointer(16404, HType.ASCII_TYPE,   "Total Pass Count by Site Aa to Dh (History)",                          "", ArmHistory[2]->sPass,                                   "Pass Count by Site Aa to Dh (History)CSV Format"           );
        HGemPtr->SetSVDataPointer(16405, HType.ASCII_TYPE,   "Total Fail Count by Site Aa to Dh (History)",                          "", ArmHistory[2]->sFail,                                   "Fail Count by Site Aa to Dh (History)CSV Format"           );
        HGemPtr->SetSVDataPointer(16406, HType.ASCII_TYPE,   "Total IF Error Count by Site Aa to Dh (History)",                      "", ArmHistory[2]->iIFErr,                                  "IF Error Count by Site Aa to Dh (History)CSV Format"       );
        HGemPtr->SetSVDataPointer(16407, HType.ASCII_TYPE,   "Total Count by Site Aa to Dh (History)",                               "", ArmHistory[2]->sTotal,                                  "Total Count by Site Aa to Dh (History)CSV Format"          );
        HGemPtr->SetSVDataPointer(16408, HType.ASCII_TYPE,   "Arm 1 Pass Count by Site Aa to Dh (History)",                          "", ArmHistory[0]->sPass,                                   "Arm 1 Pass Count by Site Aa to Dh (History)CSV Format"     );
        HGemPtr->SetSVDataPointer(16409, HType.ASCII_TYPE,   "Arm 1 Fail Count by Site Aa to Dh (History)",                          "", ArmHistory[0]->sFail,                                   "Arm 1 Fail Count by Site Aa to Dh (History)CSV Format"     );
        HGemPtr->SetSVDataPointer(16410, HType.ASCII_TYPE,   "Arm 1 IF Error Count by Site Aa to Dh (History)",                      "", ArmHistory[0]->iIFErr,                                  "Arm 1 IF Error Count by Site Aa to Dh (History)CSV Format" );
        HGemPtr->SetSVDataPointer(16411, HType.ASCII_TYPE,   "Arm 1 Total Count by Site Aa to Dh (History)",                         "", ArmHistory[0]->sTotal,                                  "Arm 1 Total Count by Site Aa to Dh (History)CSV Format"    );
        HGemPtr->SetSVDataPointer(16412, HType.ASCII_TYPE,   "Arm 2 Pass Count by Site Aa to Dh (History)",                          "", ArmHistory[1]->sPass,                                   "Arm 2 Pass Count by Site Aa to Dh (History)CSV Format"     );
        HGemPtr->SetSVDataPointer(16413, HType.ASCII_TYPE,   "Arm 2 Fail Count by Site Aa to Dh (History)",                          "", ArmHistory[1]->sFail,                                   "Arm 2 Fail Count by Site Aa to Dh (History)CSV Format"     );
        HGemPtr->SetSVDataPointer(16414, HType.ASCII_TYPE,   "Arm 2 IF Error Count by Site Aa to Dh (History)",                      "", ArmHistory[1]->iIFErr,                                  "Arm 2 IF Error Count by Site Aa to Dh (History)CSV Format" );
        HGemPtr->SetSVDataPointer(16415, HType.ASCII_TYPE,   "Arm 2 Total Count by Site Aa to Dh (History)",                         "", ArmHistory[1]->sTotal,                                  "Arm 2 Total Count by Site Aa to Dh (History)CSV Format"    );
        HGemPtr->SetSVDataPointer(20001, HType.ASCII_TYPE,   "EC Change ID",                                                         "", &SYS_ECChangeID,                                        "機台EC改變的ID"                                            );
        HGemPtr->SetSVDataPointer(20002, HType.ASCII_TYPE,   "EC Change Origina Value",                                              "", &SYS_ECChangeIDOriginaValue,                            "機台EC改變ID原本的值"                                      );
        HGemPtr->SetSVDataPointer(20003, HType.ASCII_TYPE,   "EC Change New Value",                                                  "", &SYS_ECChangeIDNewValue,                                "機台EC改變ID新的值"                                        );
        HGemPtr->SetSVDataPointer(35915, HType.INT_4_TYPE,   "USE BARCODE MODE",                                                     "", &BAR_CODE_INSTALL,                                      ""                                                          );    //wei 20150820 Use ART 機構
        HGemPtr->SetSVDataPointer(37000, HType.BOOLEAN_TYPE, "Enable LoaderTrayCount_ART",                                           "", &LastSet.bLoaderTrayCount_ART,                          ""                                                          );
        HGemPtr->SetSVDataPointer(37001, HType.INT_4_TYPE,   "LoaderTrayCount_ART",                                                  "", &LastSet.iLoaderTrayCount_ART,                          ""                                                          );
        HGemPtr->SetSVDataPointer(37002, HType.INT_4_TYPE,   "AutoRetestCount_ART",                                                  "", &LastSet.iAutoRetestCount_ART,                          ""                                                          );
        HGemPtr->SetSVDataPointer(37003, HType.INT_4_TYPE,   "UnloaderTrayCount_ART_Auto1",                                          "", &LastSet.iUnloaderTrayCount_ART[eAuto1],                ""                                                          );
        HGemPtr->SetSVDataPointer(37004, HType.INT_4_TYPE,   "UnloaderTrayCount_ART_Auto2",                                          "", &LastSet.iUnloaderTrayCount_ART[eAuto2],                ""                                                          );
        HGemPtr->SetSVDataPointer(37005, HType.INT_4_TYPE,   "UnloaderTrayCount_ART_Auto3",                                          "", &LastSet.iUnloaderTrayCount_ART[eAuto3],                ""                                                          );
        HGemPtr->SetSVDataPointer(37006, HType.INT_4_TYPE,   "Input Loader Count",                                                   "", &LastSet.iInputLoaderCount,                             ""                                                          );
        //HGemPtr->SetSVDataPointer(37007, HType.INT_4_TYPE,   "LoaderTotalTray",                                                      "", &LastSet.iLoaderTotalTray,                             ""                                                           );  //ChungHung 20150511 change positon
        HGemPtr->SetSVDataPointer(37008, HType.ASCII_TYPE,   "Enter Barcode Reader",                                                 "", &asSecsGemBarCode,                                      ""                                                          );    //wei 20150811 SecsGem BarcodeReader  //Ifor 20180227 : 修正SECS GEM 上傳資料異常 asBarCode=>asSecsGemBarCode
        HGemPtr->SetSVDataPointer(37009, HType.BOOLEAN_TYPE, "USE AUTO RETEST",                                                      "", &USE_AUTO_RETEST,                                       ""                                                          );    //wei 20150820 Use ART 機構
        HGemPtr->SetSVDataPointer(37010, HType.INT_4_TYPE,   "Enter Skip IC Count",                                                  "", &iJamSkipIC,                                            ""                                                          );    //wei 20160302 Jam Skip輸入顆數
        HGemPtr->SetSVDataPointer(37012, HType.ASCII_TYPE,   "Enter Barcode Reader Password",                                        "", &asSecsGemBarCodePasswoed,                              ""                                                          );    //Ifor 20180227 add BarCode Password

        HGemPtr->SetSVDataPointer(37013, HType.INT_4_TYPE,   "UnloaderTrayCount_ART_Auto4",                                          "", &LastSet.iUnloaderTrayCount_ART[eAuto4],                ""                                                          );
        HGemPtr->SetSVDataPointer(37014, HType.INT_4_TYPE,   "UnloaderTrayCount_ART_Auto5",                                          "", &LastSet.iUnloaderTrayCount_ART[eAuto5],                ""                                                          );
        HGemPtr->SetSVDataPointer(37015, HType.INT_4_TYPE,   "UnloaderTrayCount_ART_Auto6",                                          "", &LastSet.iUnloaderTrayCount_ART[eAuto6],                ""                                                          );

#if 0 // GATE [G15] -- golden :705-706.  SVID 37200/37201 -- fLotInfo->lbESDReportData / lbESDDecayReportData: not on TfLotInfo.
        HGemPtr->SetSVDataPointer(37200, HType.ASCII_TYPE,   "SIMCO Ion fan Data",                                                   "", fLotInfo->lbESDReportData,                              ""                                                          );    //Ifor 20160315 SIMCO ESD Data
        HGemPtr->SetSVDataPointer(37201, HType.ASCII_TYPE,   "SIMCO Ion fan Decay Data",                                             "", fLotInfo->lbESDDecayReportData,                         ""                                                          );    //Ifor 20160315 SIMCO ESD Auto Decay Data
#endif // GATE [G15]
        HGemPtr->SetSVDataPointer(37202, HType.ASCII_TYPE,   "ATC Software Version",                                                 "", &RunInfo.ATCSoftwareVersion,                            "ATC Software Version"                                               );    //Ifor 20160318 ATC 軟體版本
        HGemPtr->SetSVDataPointer(37203, HType.ASCII_TYPE,   "GPIB Software Version",                                                "", &RunInfo.GPIBSoftwareVersion,                           "GPIB Software Version"                                              );    //Ifor 20160318 GPIB 軟體版本

        HGemPtr->SetSVDataPointer(37204, HType.INT_4_TYPE,   "ION FAN 1 Status",                                                     "", &iIONFANStatus_Secs[0],                                 "ION FAN 1 Status"                                          );
        HGemPtr->SetSVDataPointer(37205, HType.INT_4_TYPE,   "ION FAN 2 Status",                                                     "", &iIONFANStatus_Secs[1],                                 "ION FAN 2 Status"                                          );
        HGemPtr->SetSVDataPointer(37206, HType.INT_4_TYPE,   "ION FAN 3 Status",                                                     "", &iIONFANStatus_Secs[2],                                 "ION FAN 3 Status"                                          );
        HGemPtr->SetSVDataPointer(37207, HType.INT_4_TYPE,   "ION FAN 4 Status",                                                     "", &iIONFANStatus_Secs[3],                                 "ION FAN 4 Status"                                          );
        HGemPtr->SetSVDataPointer(37208, HType.INT_4_TYPE,   "ION FAN 5 Status",                                                     "", &iIONFANStatus_Secs[4],                                 "ION FAN 5 Status"                                          );
        HGemPtr->SetSVDataPointer(37209, HType.INT_4_TYPE,   "ION FAN 6 Status",                                                     "", &iIONFANStatus_Secs[5],                                 "ION FAN 6 Status"                                          );
        HGemPtr->SetSVDataPointer(37210, HType.INT_4_TYPE,   "ION FAN 7 Status",                                                     "", &iIONFANStatus_Secs[6],                                 "ION FAN 7 Status"                                          );
        HGemPtr->SetSVDataPointer(37211, HType.INT_4_TYPE,   "ION FAN 8 Status",                                                     "", &iIONFANStatus_Secs[7],                                 "ION FAN 8 Status"                                          );
        HGemPtr->SetSVDataPointer(37212, HType.INT_4_TYPE,   "ION FAN 9 Status",                                                     "", &iIONFANStatus_Secs[8],                                 "ION FAN 9 Status"                                          );
        HGemPtr->SetSVDataPointer(37213, HType.INT_4_TYPE,   "ION FAN 10 Status",                                                    "", &iIONFANStatus_Secs[9],                                 "ION FAN 10 Status"                                         );
        HGemPtr->SetSVDataPointer(37214, HType.INT_4_TYPE,   "ION FAN 11 Status",                                                    "", &iIONFANStatus_Secs[10],                                "ION FAN 11 Status"                                         );
        HGemPtr->SetSVDataPointer(37215, HType.INT_4_TYPE,   "ION FAN 12 Status",                                                    "", &iIONFANStatus_Secs[11],                                "ION FAN 12 Status"                                         );

        HGemPtr->SetSVDataPointer(37216, HType.INT_4_TYPE,   "ION FAN 1 Power",                                                      "", &iIONFANPower_Secs[0],                                  "ION FAN 1 Power"                                           );
        HGemPtr->SetSVDataPointer(37217, HType.INT_4_TYPE,   "ION FAN 2 Power",                                                      "", &iIONFANPower_Secs[1],                                  "ION FAN 2 Power"                                           );
        HGemPtr->SetSVDataPointer(37218, HType.INT_4_TYPE,   "ION FAN 3 Power",                                                      "", &iIONFANPower_Secs[2],                                  "ION FAN 3 Power"                                           );
        HGemPtr->SetSVDataPointer(37219, HType.INT_4_TYPE,   "ION FAN 4 Power",                                                      "", &iIONFANPower_Secs[3],                                  "ION FAN 4 Power"                                           );
        HGemPtr->SetSVDataPointer(37220, HType.INT_4_TYPE,   "ION FAN 5 Power",                                                      "", &iIONFANPower_Secs[4],                                  "ION FAN 5 Power"                                           );
        HGemPtr->SetSVDataPointer(37221, HType.INT_4_TYPE,   "ION FAN 6 Power",                                                      "", &iIONFANPower_Secs[5],                                  "ION FAN 6 Power"                                           );
        HGemPtr->SetSVDataPointer(37222, HType.INT_4_TYPE,   "ION FAN 7 Power",                                                      "", &iIONFANPower_Secs[6],                                  "ION FAN 7 Power"                                           );
        HGemPtr->SetSVDataPointer(37223, HType.INT_4_TYPE,   "ION FAN 8 Power",                                                      "", &iIONFANPower_Secs[7],                                  "ION FAN 8 Power"                                           );
        HGemPtr->SetSVDataPointer(37224, HType.INT_4_TYPE,   "ION FAN 9 Power",                                                      "", &iIONFANPower_Secs[8],                                  "ION FAN 9 Power"                                           );
        HGemPtr->SetSVDataPointer(37225, HType.INT_4_TYPE,   "ION FAN 10 Power",                                                     "", &iIONFANPower_Secs[9],                                  "ION FAN 10 Power"                                          );
        HGemPtr->SetSVDataPointer(37226, HType.INT_4_TYPE,   "ION FAN 11 Power",                                                     "", &iIONFANPower_Secs[10],                                 "ION FAN 11 Power"                                          );
        HGemPtr->SetSVDataPointer(37227, HType.INT_4_TYPE,   "ION FAN 12 Power",                                                     "", &iIONFANPower_Secs[11],                                 "ION FAN 12 Power"                                          );
        HGemPtr->SetSVDataPointer(37228, HType.ASCII_TYPE,   "ESD Software Version",                                                 "", &RunInfo.ESDSoftwareVersion,                            "ESD Software Version"                                               );      //Ifor 20170320 add SECS GEM  ESD Sofware Version

        //Ifor 20170320 add SECS GEM ATC Power Supply Firmware Version && Serial Number
        //==>
#if 0 // GATE [G16] -- golden :739-746.  SVID 37501-37508 -- fObserver->lbSerialNumber01..04 / lbFirmwareNumber01..04: not on atester_shims.h's TfObserverShim.
        HGemPtr->SetSVDataPointer(37501, HType.ASCII_TYPE,   "ATC Power Supply Serial Number 01",                                    "", fObserver->lbSerialNumber01,                            "ATC Power Supply Serial Number 01"                         );
        HGemPtr->SetSVDataPointer(37502, HType.ASCII_TYPE,   "ATC Power Supply Serial Number 02",                                    "", fObserver->lbSerialNumber02,                            "ATC Power Supply Serial Number 02"                         );
        HGemPtr->SetSVDataPointer(37503, HType.ASCII_TYPE,   "ATC Power Supply Serial Number 03",                                    "", fObserver->lbSerialNumber03,                            "ATC Power Supply Serial Number 03"                         );
        HGemPtr->SetSVDataPointer(37504, HType.ASCII_TYPE,   "ATC Power Supply Serial Number 04",                                    "", fObserver->lbSerialNumber04,                            "ATC Power Supply Serial Number 04"                         );
        HGemPtr->SetSVDataPointer(37505, HType.ASCII_TYPE,   "ATC Power Supply Firmware Version 01",                                 "", fObserver->lbFirmwareNumber01,                          "ATC Power Supply Firmware Version 01"                      );
        HGemPtr->SetSVDataPointer(37506, HType.ASCII_TYPE,   "ATC Power Supply Firmware Version 02",                                 "", fObserver->lbFirmwareNumber02,                          "ATC Power Supply Firmware Version 02"                      );
        HGemPtr->SetSVDataPointer(37507, HType.ASCII_TYPE,   "ATC Power Supply Firmware Version 03",                                 "", fObserver->lbFirmwareNumber03,                          "ATC Power Supply Firmware Version 03"                      );
        HGemPtr->SetSVDataPointer(37508, HType.ASCII_TYPE,   "ATC Power Supply Firmware Version 04",                                 "", fObserver->lbFirmwareNumber04,                          "ATC Power Supply Firmware Version 04"                      );
#endif // GATE [G16]
        //<==
        //Ifor 20170320 add SECS GEM ATC Power Supply Firmware Version && Serial Number

        //Steven 20221229 : Add SVID for Picker Count
        HGemPtr->SetSVDataPointer(37600, HType.INT_4_TYPE,   "In Arm Picker A Count",                                                "", &TestIF_File.InArmPickerLifeCnt[0][0],                  "In Arm Picker A Count"                                     );
        HGemPtr->SetSVDataPointer(37601, HType.INT_4_TYPE,   "In Arm Picker C Count",                                                "", &TestIF_File.InArmPickerLifeCnt[0][1],                  "In Arm Picker C Count"                                     );
        HGemPtr->SetSVDataPointer(37602, HType.INT_4_TYPE,   "In Arm Picker E Count",                                                "", &TestIF_File.InArmPickerLifeCnt[0][2],                  "In Arm Picker E Count"                                     );
        HGemPtr->SetSVDataPointer(37603, HType.INT_4_TYPE,   "In Arm Picker G Count",                                                "", &TestIF_File.InArmPickerLifeCnt[0][3],                  "In Arm Picker G Count"                                     );
        HGemPtr->SetSVDataPointer(37604, HType.INT_4_TYPE,   "In Arm Picker B Count",                                                "", &TestIF_File.InArmPickerLifeCnt[1][0],                  "In Arm Picker B Count"                                     );
        HGemPtr->SetSVDataPointer(37605, HType.INT_4_TYPE,   "In Arm Picker D Count",                                                "", &TestIF_File.InArmPickerLifeCnt[1][1],                  "In Arm Picker D Count"                                     );
        HGemPtr->SetSVDataPointer(37606, HType.INT_4_TYPE,   "In Arm Picker F Count",                                                "", &TestIF_File.InArmPickerLifeCnt[1][2],                  "In Arm Picker F Count"                                     );
        HGemPtr->SetSVDataPointer(37607, HType.INT_4_TYPE,   "In Arm Picker H Count",                                                "", &TestIF_File.InArmPickerLifeCnt[1][3],                  "In Arm Picker H Count"                                     );

        HGemPtr->SetSVDataPointer(37650, HType.INT_4_TYPE,   "Out Arm Picker A Count",                                               "", &TestIF_File.OutArmPickerLifeCnt[0][0],                 "Out Arm Picker A Count"                                    );
        HGemPtr->SetSVDataPointer(37651, HType.INT_4_TYPE,   "Out Arm Picker C Count",                                               "", &TestIF_File.OutArmPickerLifeCnt[0][1],                 "Out Arm Picker C Count"                                    );
        HGemPtr->SetSVDataPointer(37652, HType.INT_4_TYPE,   "Out Arm Picker E Count",                                               "", &TestIF_File.OutArmPickerLifeCnt[0][2],                 "Out Arm Picker E Count"                                    );
        HGemPtr->SetSVDataPointer(37653, HType.INT_4_TYPE,   "Out Arm Picker G Count",                                               "", &TestIF_File.OutArmPickerLifeCnt[0][3],                 "Out Arm Picker G Count"                                    );
        HGemPtr->SetSVDataPointer(37654, HType.INT_4_TYPE,   "Out Arm Picker B Count",                                               "", &TestIF_File.OutArmPickerLifeCnt[1][0],                 "Out Arm Picker B Count"                                    );
        HGemPtr->SetSVDataPointer(37655, HType.INT_4_TYPE,   "Out Arm Picker D Count",                                               "", &TestIF_File.OutArmPickerLifeCnt[1][1],                 "Out Arm Picker D Count"                                    );
        HGemPtr->SetSVDataPointer(37656, HType.INT_4_TYPE,   "Out Arm Picker F Count",                                               "", &TestIF_File.OutArmPickerLifeCnt[1][2],                 "Out Arm Picker F Count"                                    );
        HGemPtr->SetSVDataPointer(37657, HType.INT_4_TYPE,   "Out Arm Picker H Count",                                               "", &TestIF_File.OutArmPickerLifeCnt[1][3],                 "Out Arm Picker H Count"                                    );

        HGemPtr->SetSVDataPointer(37700, HType.ASCII_TYPE,   "PC_NAME",                                                              "", &PC_NAME,                                               "PC_NAME"                                                   ); //kevin 20180803

        HGemPtr->SetSVDataPointer(37701, HType.INT_4_TYPE,   "Index Arm 1 Picker Aa Count",                                          "", &TestIF_File.Arm1PickerLifeCnt[0][0],                   "Index Arm 1 Picker Aa Count"                               );
        HGemPtr->SetSVDataPointer(37702, HType.INT_4_TYPE,   "Index Arm 1 Picker Ab Count",                                          "", &TestIF_File.Arm1PickerLifeCnt[0][1],                   "Index Arm 1 Picker Ab Count"                               );
        HGemPtr->SetSVDataPointer(37703, HType.INT_4_TYPE,   "Index Arm 1 Picker Ac Count",                                          "", &TestIF_File.Arm1PickerLifeCnt[0][2],                   "Index Arm 1 Picker Ac Count"                               );
        HGemPtr->SetSVDataPointer(37704, HType.INT_4_TYPE,   "Index Arm 1 Picker Ad Count",                                          "", &TestIF_File.Arm1PickerLifeCnt[0][3],                   "Index Arm 1 Picker Ad Count"                               );
        HGemPtr->SetSVDataPointer(37705, HType.INT_4_TYPE,   "Index Arm 1 Picker Ae Count",                                          "", &TestIF_File.Arm1PickerLifeCnt[0][4],                   "Index Arm 1 Picker Ae Count"                               );
        HGemPtr->SetSVDataPointer(37706, HType.INT_4_TYPE,   "Index Arm 1 Picker Af Count",                                          "", &TestIF_File.Arm1PickerLifeCnt[0][5],                   "Index Arm 1 Picker Af Count"                               );
        HGemPtr->SetSVDataPointer(37707, HType.INT_4_TYPE,   "Index Arm 1 Picker Ag Count",                                          "", &TestIF_File.Arm1PickerLifeCnt[0][6],                   "Index Arm 1 Picker Ag Count"                               );
        HGemPtr->SetSVDataPointer(37708, HType.INT_4_TYPE,   "Index Arm 1 Picker Ah Count",                                          "", &TestIF_File.Arm1PickerLifeCnt[0][7],                   "Index Arm 1 Picker Ah Count"                               );

        HGemPtr->SetSVDataPointer(37709, HType.INT_4_TYPE,   "Index Arm 1 Picker Ba Count",                                          "", &TestIF_File.Arm1PickerLifeCnt[1][0],                   "Index Arm 1 Picker Ba Count"                               );
        HGemPtr->SetSVDataPointer(37710, HType.INT_4_TYPE,   "Index Arm 1 Picker Bb Count",                                          "", &TestIF_File.Arm1PickerLifeCnt[1][1],                   "Index Arm 1 Picker Bb Count"                               );
        HGemPtr->SetSVDataPointer(37711, HType.INT_4_TYPE,   "Index Arm 1 Picker Bc Count",                                          "", &TestIF_File.Arm1PickerLifeCnt[1][2],                   "Index Arm 1 Picker Bc Count"                               );
        HGemPtr->SetSVDataPointer(37712, HType.INT_4_TYPE,   "Index Arm 1 Picker Bd Count",                                          "", &TestIF_File.Arm1PickerLifeCnt[1][3],                   "Index Arm 1 Picker Bd Count"                               );
        HGemPtr->SetSVDataPointer(37713, HType.INT_4_TYPE,   "Index Arm 1 Picker Be Count",                                          "", &TestIF_File.Arm1PickerLifeCnt[1][4],                   "Index Arm 1 Picker Be Count"                               );
        HGemPtr->SetSVDataPointer(37714, HType.INT_4_TYPE,   "Index Arm 1 Picker Bf Count",                                          "", &TestIF_File.Arm1PickerLifeCnt[1][5],                   "Index Arm 1 Picker Bf Count"                               );
        HGemPtr->SetSVDataPointer(37715, HType.INT_4_TYPE,   "Index Arm 1 Picker Bg Count",                                          "", &TestIF_File.Arm1PickerLifeCnt[1][6],                   "Index Arm 1 Picker Bg Count"                               );
        HGemPtr->SetSVDataPointer(37716, HType.INT_4_TYPE,   "Index Arm 1 Picker Bh Count",                                          "", &TestIF_File.Arm1PickerLifeCnt[1][7],                   "Index Arm 1 Picker Bh Count"                               );

        HGemPtr->SetSVDataPointer(37717, HType.INT_4_TYPE,   "Index Arm 2 Picker Aa Count",                                          "", &TestIF_File.Arm2PickerLifeCnt[0][0],                   "Index Arm 2 Picker Aa Count"                               );
        HGemPtr->SetSVDataPointer(37718, HType.INT_4_TYPE,   "Index Arm 2 Picker Ab Count",                                          "", &TestIF_File.Arm2PickerLifeCnt[0][1],                   "Index Arm 2 Picker Ab Count"                               );
        HGemPtr->SetSVDataPointer(37719, HType.INT_4_TYPE,   "Index Arm 2 Picker Ac Count",                                          "", &TestIF_File.Arm2PickerLifeCnt[0][2],                   "Index Arm 2 Picker Ac Count"                               );
        HGemPtr->SetSVDataPointer(37720, HType.INT_4_TYPE,   "Index Arm 2 Picker Ad Count",                                          "", &TestIF_File.Arm2PickerLifeCnt[0][3],                   "Index Arm 2 Picker Ad Count"                               );
        HGemPtr->SetSVDataPointer(37721, HType.INT_4_TYPE,   "Index Arm 2 Picker Ae Count",                                          "", &TestIF_File.Arm2PickerLifeCnt[0][4],                   "Index Arm 2 Picker Ae Count"                               );
        HGemPtr->SetSVDataPointer(37722, HType.INT_4_TYPE,   "Index Arm 2 Picker Af Count",                                          "", &TestIF_File.Arm2PickerLifeCnt[0][5],                   "Index Arm 2 Picker Af Count"                               );
        HGemPtr->SetSVDataPointer(37723, HType.INT_4_TYPE,   "Index Arm 2 Picker Ag Count",                                          "", &TestIF_File.Arm2PickerLifeCnt[0][6],                   "Index Arm 2 Picker Ag Count"                               );
        HGemPtr->SetSVDataPointer(37724, HType.INT_4_TYPE,   "Index Arm 2 Picker Ah Count",                                          "", &TestIF_File.Arm2PickerLifeCnt[0][7],                   "Index Arm 2 Picker Ah Count"                               );

        HGemPtr->SetSVDataPointer(37725, HType.INT_4_TYPE,   "Index Arm 2 Picker Ba Count",                                          "", &TestIF_File.Arm2PickerLifeCnt[1][0],                   "Index Arm 2 Picker Ba Count"                               );
        HGemPtr->SetSVDataPointer(37726, HType.INT_4_TYPE,   "Index Arm 2 Picker Bb Count",                                          "", &TestIF_File.Arm2PickerLifeCnt[1][1],                   "Index Arm 2 Picker Bb Count"                               );
        HGemPtr->SetSVDataPointer(37727, HType.INT_4_TYPE,   "Index Arm 2 Picker Bc Count",                                          "", &TestIF_File.Arm2PickerLifeCnt[1][2],                   "Index Arm 2 Picker Bc Count"                               );
        HGemPtr->SetSVDataPointer(37728, HType.INT_4_TYPE,   "Index Arm 2 Picker Bd Count",                                          "", &TestIF_File.Arm2PickerLifeCnt[1][3],                   "Index Arm 2 Picker Bd Count"                               );
        HGemPtr->SetSVDataPointer(37729, HType.INT_4_TYPE,   "Index Arm 2 Picker Be Count",                                          "", &TestIF_File.Arm2PickerLifeCnt[1][4],                   "Index Arm 2 Picker Be Count"                               );
        HGemPtr->SetSVDataPointer(37730, HType.INT_4_TYPE,   "Index Arm 2 Picker Bf Count",                                          "", &TestIF_File.Arm2PickerLifeCnt[1][5],                   "Index Arm 2 Picker Bf Count"                               );
        HGemPtr->SetSVDataPointer(37731, HType.INT_4_TYPE,   "Index Arm 2 Picker Bg Count",                                          "", &TestIF_File.Arm2PickerLifeCnt[1][6],                   "Index Arm 2 Picker Bg Count"                               );
        HGemPtr->SetSVDataPointer(37732, HType.INT_4_TYPE,   "Index Arm 2 Picker Bh Count",                                          "", &TestIF_File.Arm2PickerLifeCnt[1][7],                   "Index Arm 2 Picker Bh Count"                               );

        HGemPtr->SetSVDataPointer(38120, HType.INT_4_TYPE,   "Load Port Transfer State - LOADER",                                    "", &iPortStatus[ePortLoader],                              "Load Port Transfer State - LOADER"                         );
        HGemPtr->SetSVDataPointer(38121, HType.INT_4_TYPE,   "Load Port Transfer State - EMPTY",                                     "", &iPortStatus[ePortEmpty],                               "Load Port Transfer State - EMPTY"                          );
        HGemPtr->SetSVDataPointer(38122, HType.INT_4_TYPE,   "Load Port Transfer State - COLOR",                                     "", &iPortStatus[ePortColor],                               "Load Port Transfer State - COLOR"                          );
        HGemPtr->SetSVDataPointer(38123, HType.INT_4_TYPE,   "Load Port Transfer State - AUTO1",                                     "", &iPortStatus[ePortAuto1],                               "Load Port Transfer State - AUTO1"                          );
        HGemPtr->SetSVDataPointer(38124, HType.INT_4_TYPE,   "Load Port Transfer State - AUTO2",                                     "", &iPortStatus[ePortAuto2],                               "Load Port Transfer State - AUTO2"                          );
        HGemPtr->SetSVDataPointer(38125, HType.INT_4_TYPE,   "Load Port Transfer State - AUTO3",                                     "", &iPortStatus[ePortAuto3],                               "Load Port Transfer State - AUTO3"                          );
        HGemPtr->SetSVDataPointer(38126, HType.INT_4_TYPE,   "Load Port Transfer State - FIX1",                                      "", &iPortStatus[ePortFix1],                                "Load Port Transfer State - FIX1"                           );
        HGemPtr->SetSVDataPointer(38127, HType.INT_4_TYPE,   "Load Port Transfer State - FIX2",                                      "", &iPortStatus[ePortFix2],                                "Load Port Transfer State - FIX2"                           );
        HGemPtr->SetSVDataPointer(38128, HType.INT_4_TYPE,   "Load Port Transfer State - FIX3",                                      "", &iPortStatus[ePortFix3],                                "Load Port Transfer State - FIX3"                           );
        HGemPtr->SetSVDataPointer(38129, HType.INT_4_TYPE,   "Load Port Transfer State - AUTO4",                                     "", &iPortStatus[ePortAuto4],                               "Load Port Transfer State - AUTO4"                          );
        HGemPtr->SetSVDataPointer(38130, HType.INT_4_TYPE,   "Load Port Transfer State - AUTO5",                                     "", &iPortStatus[ePortAuto5],                               "Load Port Transfer State - AUTO5"                          );
        HGemPtr->SetSVDataPointer(38131, HType.INT_4_TYPE,   "Load Port Transfer State - AUTO6",                                     "", &iPortStatus[ePortAuto6],                               "Load Port Transfer State - AUTO6"                          );
        HGemPtr->SetSVDataPointer(38132, HType.INT_4_TYPE,   "Load Port Transfer State - FIX4",                                      "", &iPortStatus[ePortFix4],                                "Load Port Transfer State - FIX4"                           );
        HGemPtr->SetSVDataPointer(38133, HType.INT_4_TYPE,   "Load Port Transfer State - FIX5",                                      "", &iPortStatus[ePortFix5],                                "Load Port Transfer State - FIX5"                           );
        HGemPtr->SetSVDataPointer(38134, HType.INT_4_TYPE,   "Load Port Transfer State - FIX6",                                      "", &iPortStatus[ePortFix6],                                "Load Port Transfer State - FIX6"                           );
        //==> Eastsun 20260513 F013 AMR SV bindings 38222-38233
        HGemPtr->SetSVDataPointer(38222, HType.INT_4_TYPE,   "AMR Loader Tray Count", "", &TestIF_File.iAMRTrayCount[0], "AMR Loader Tray Count");
        HGemPtr->SetSVDataPointer(38223, HType.INT_4_TYPE,   "AMR Empty Tray Count", "", &TestIF_File.iAMRTrayCount[1], "AMR Empty Tray Count");
        HGemPtr->SetSVDataPointer(38224, HType.INT_4_TYPE,   "AMR Color Tray Count", "", &TestIF_File.iAMRTrayCount[2], "AMR Color Tray Count");
        HGemPtr->SetSVDataPointer(38225, HType.INT_4_TYPE,   "AMR Auto1 Tray Count", "", &TestIF_File.iAMRTrayCount[3], "AMR Auto1 Tray Count");
        HGemPtr->SetSVDataPointer(38226, HType.INT_4_TYPE,   "AMR Auto2 Tray Count", "", &TestIF_File.iAMRTrayCount[4], "AMR Auto2 Tray Count");
        HGemPtr->SetSVDataPointer(38227, HType.INT_4_TYPE,   "AMR Auto3 Tray Count", "", &TestIF_File.iAMRTrayCount[5], "AMR Auto3 Tray Count");
        HGemPtr->SetSVDataPointer(38228, HType.INT_4_TYPE,   "AMR Loader Device Count", "", &TestIF_File.iAMRDeviceCount[0], "AMR Loader Device Count");
        HGemPtr->SetSVDataPointer(38229, HType.INT_4_TYPE,   "AMR Empty Device Count", "", &TestIF_File.iAMRDeviceCount[1], "AMR Empty Device Count");
        HGemPtr->SetSVDataPointer(38230, HType.INT_4_TYPE,   "AMR Color Device Count", "", &TestIF_File.iAMRDeviceCount[2], "AMR Color Device Count");
        HGemPtr->SetSVDataPointer(38231, HType.INT_4_TYPE,   "AMR Auto1 Device Count", "", &TestIF_File.iAMRDeviceCount[3], "AMR Auto1 Device Count");
        HGemPtr->SetSVDataPointer(38232, HType.INT_4_TYPE,   "AMR Auto2 Device Count", "", &TestIF_File.iAMRDeviceCount[4], "AMR Auto2 Device Count");
        HGemPtr->SetSVDataPointer(38233, HType.INT_4_TYPE,   "AMR Auto3 Device Count", "", &TestIF_File.iAMRDeviceCount[5], "AMR Auto3 Device Count");
        //<== Eastsun 20260513 F013

        HGemPtr->SetSVDataPointer(38303, HType.BOOLEAN_TYPE, "Empty Tray Count",                                                    "", &LastSet.iEmptyTrayCount_ART,                            "Empty Tray Count"                                          );
        HGemPtr->SetSVDataPointer(38304, HType.BOOLEAN_TYPE, "Output BIN Code",                                                     "", &sOutputBinCode,                                         "Output BIN Code"                                          );

        HGemPtr->SetSVDataPointer(38305, HType.INT_4_TYPE,   "Input Port State",                                                    "", &iLoadStateATK[etLoader],                                "Input Port State"                        );
        HGemPtr->SetSVDataPointer(38306, HType.INT_4_TYPE,   "Empty Port State",                                                    "", &iLoadStateATK[etEmpty],                                 "Empty Port State"                        );
        HGemPtr->SetSVDataPointer(38307, HType.INT_4_TYPE,   "Output 1 Port State",                                                 "", &iLoadStateATK[etAuto1],                                 "Output 1 Port State"                     );
        HGemPtr->SetSVDataPointer(38308, HType.INT_4_TYPE,   "Output 2 Port State",                                                 "", &iLoadStateATK[etAuto2],                                 "Output 2 Port State"                     );
        HGemPtr->SetSVDataPointer(38309, HType.INT_4_TYPE,   "Output 3 Port State",                                                 "", &iLoadStateATK[etAuto3],                                 "Output 3 Port State"                     );
        HGemPtr->SetSVDataPointer(38310, HType.INT_4_TYPE,   "Output 4 Port State",                                                 "", &iLoadStateATK[etAuto4],                                 "Output 4 Port State"                     );
        HGemPtr->SetSVDataPointer(38311, HType.INT_4_TYPE,   "Output 5 Port State",                                                 "", &iLoadStateATK[etAuto5],                                 "Output 5 Port State"                     );
        HGemPtr->SetSVDataPointer(38312, HType.INT_4_TYPE,   "Output 6 Port State",                                                 "", &iLoadStateATK[etAuto6],                                 "Output 6 Port State"                     );
        HGemPtr->SetSVDataPointer(38313, HType.INT_4_TYPE,   "CoverTray Port State",                                                "", &iLoadStateATK[etColor],                                 "CoverTray Port State"                    );
        HGemPtr->SetSVDataPointer(38314, HType.ASCII_TYPE,   "PC_NAME",                                                             "", &PC_NAME,                                                "PC_NAME"                                                   ); //kevin 20180803

        HGemPtr->SetSVDataPointer(21023, HType.BOOLEAN_TYPE, "Enable Tester Dry Air Control",                                        "", &TestIF_File.bUseTesterDry,                             "Enable Tester Dry Air Control"                             );    //wei 20150820 Use ART 機構

#if 0 // GATE [G17] -- golden :853-880.  SVID 43300-43327 -- fGroundMan->labValue_0_2..labValue_3_5: the fGroundMan global/form does not exist in the port at all.
        HGemPtr->SetSVDataPointer(43300, HType.ASCII_TYPE,   "Ground man CH1 Value",                                                "", fGroundMan->labValue_0_2,                                 "Ground man CH1 Value"                                       );   //JerryYang 20250120 : add
        HGemPtr->SetSVDataPointer(43301, HType.ASCII_TYPE,   "Ground man CH2 Value",                                                "", fGroundMan->labValue_0_3,                                 "Ground man CH2 Value"                                       );
        HGemPtr->SetSVDataPointer(43302, HType.ASCII_TYPE,   "Ground man CH3 Value",                                                "", fGroundMan->labValue_0_4,                                 "Ground man CH3 Value"                                       );
        HGemPtr->SetSVDataPointer(43303, HType.ASCII_TYPE,   "Ground man CH4 Value",                                                "", fGroundMan->labValue_0_5,                                 "Ground man CH4 Value"                                       );
        HGemPtr->SetSVDataPointer(43304, HType.ASCII_TYPE,   "Ground man CH5 Value",                                                "", fGroundMan->labValue_0_6,                                 "Ground man CH5 Value"                                       );
        HGemPtr->SetSVDataPointer(43305, HType.ASCII_TYPE,   "Ground man CH6 Value",                                                "", fGroundMan->labValue_0_7,                                 "Ground man CH6 Value"                                       );
        HGemPtr->SetSVDataPointer(43306, HType.ASCII_TYPE,   "Ground man CH7 Value",                                                "", fGroundMan->labValue_1_0,                                 "Ground man CH7 Value"                                       );
        HGemPtr->SetSVDataPointer(43307, HType.ASCII_TYPE,   "Ground man CH8 Value",                                                "", fGroundMan->labValue_1_1,                                 "Ground man CH8 Value"                                       );
        HGemPtr->SetSVDataPointer(43308, HType.ASCII_TYPE,   "Ground man CH9 Value",                                                "", fGroundMan->labValue_1_2,                                 "Ground man CH9 Value"                                       );
        HGemPtr->SetSVDataPointer(43309, HType.ASCII_TYPE,   "Ground man CH10 Value",                                               "", fGroundMan->labValue_1_3,                                 "Ground man CH10 Value"                                      );
        HGemPtr->SetSVDataPointer(43310, HType.ASCII_TYPE,   "Ground man CH11 Value",                                               "", fGroundMan->labValue_1_4,                                 "Ground man CH11 Value"                                      );
        HGemPtr->SetSVDataPointer(43311, HType.ASCII_TYPE,   "Ground man CH12 Value",                                               "", fGroundMan->labValue_1_5,                                 "Ground man CH12 Value"                                      );
        HGemPtr->SetSVDataPointer(43312, HType.ASCII_TYPE,   "Ground man CH13 Value",                                               "", fGroundMan->labValue_1_6,                                 "Ground man CH13 Value"                                      );
        HGemPtr->SetSVDataPointer(43313, HType.ASCII_TYPE,   "Ground man CH14 Value",                                               "", fGroundMan->labValue_1_7,                                 "Ground man CH14 Value"                                      );
        HGemPtr->SetSVDataPointer(43314, HType.ASCII_TYPE,   "Ground man CH15 Value",                                               "", fGroundMan->labValue_2_0,                                 "Ground man CH15 Value"                                      );
        HGemPtr->SetSVDataPointer(43315, HType.ASCII_TYPE,   "Ground man CH16 Value",                                               "", fGroundMan->labValue_2_1,                                 "Ground man CH16 Value"                                      );
        HGemPtr->SetSVDataPointer(43316, HType.ASCII_TYPE,   "Ground man CH17 Value",                                               "", fGroundMan->labValue_2_2,                                 "Ground man CH17 Value"                                      );
        HGemPtr->SetSVDataPointer(43317, HType.ASCII_TYPE,   "Ground man CH18 Value",                                               "", fGroundMan->labValue_2_3,                                 "Ground man CH18 Value"                                      );
        HGemPtr->SetSVDataPointer(43318, HType.ASCII_TYPE,   "Ground man CH19 Value",                                               "", fGroundMan->labValue_2_4,                                 "Ground man CH19 Value"                                      );
        HGemPtr->SetSVDataPointer(43319, HType.ASCII_TYPE,   "Ground man CH20 Value",                                               "", fGroundMan->labValue_2_5,                                 "Ground man CH20 Value"                                      );
        HGemPtr->SetSVDataPointer(43320, HType.ASCII_TYPE,   "Ground man CH21 Value",                                               "", fGroundMan->labValue_2_6,                                 "Ground man CH21 Value"                                      );
        HGemPtr->SetSVDataPointer(43321, HType.ASCII_TYPE,   "Ground man CH22 Value",                                               "", fGroundMan->labValue_2_7,                                 "Ground man CH22 Value"                                      );
        HGemPtr->SetSVDataPointer(43322, HType.ASCII_TYPE,   "Ground man CH23 Value",                                               "", fGroundMan->labValue_3_0,                                 "Ground man CH23 Value"                                      );
        HGemPtr->SetSVDataPointer(43323, HType.ASCII_TYPE,   "Ground man CH24 Value",                                               "", fGroundMan->labValue_3_1,                                 "Ground man CH24 Value"                                      );
        HGemPtr->SetSVDataPointer(43324, HType.ASCII_TYPE,   "Ground man CH25 Value",                                               "", fGroundMan->labValue_3_2,                                 "Ground man CH25 Value"                                      );
        HGemPtr->SetSVDataPointer(43325, HType.ASCII_TYPE,   "Ground man CH26 Value",                                               "", fGroundMan->labValue_3_3,                                 "Ground man CH26 Value"                                      );
        HGemPtr->SetSVDataPointer(43326, HType.ASCII_TYPE,   "Ground man CH27 Value",                                               "", fGroundMan->labValue_3_4,                                 "Ground man CH27 Value"                                      );
        HGemPtr->SetSVDataPointer(43327, HType.ASCII_TYPE,   "Ground man CH28 Value",                                               "", fGroundMan->labValue_3_5,                                 "Ground man CH28 Value"                                      );
#endif // GATE [G17]

#if 0 // GATE [G18] -- golden :882-901.  SVID 43350-43369 -- fSmartDiagnostic->iPushAvgTime[0..9] / iPopAvgTime[0..9]: the fSmartDiagnostic global/form does not exist in the port at all.
        HGemPtr->SetSVDataPointer(43350, HType.ASCII_TYPE,   "C_LoaderEdgePush AVG push time",                                      "", &fSmartDiagnostic->iPushAvgTime[0],                     "C_LoaderEdgePush AVG push time"                             );
        HGemPtr->SetSVDataPointer(43351, HType.ASCII_TYPE,   "C_TrayY_Fixer AVG push time",                                         "", &fSmartDiagnostic->iPushAvgTime[1],                     "C_TrayY_Fixer AVG push time"                                );
        HGemPtr->SetSVDataPointer(43352, HType.ASCII_TYPE,   "C_Empty_Fix  AVG push time",                                          "", &fSmartDiagnostic->iPushAvgTime[2],                     "C_Empty_Fix  AVG push time"                                 );
        HGemPtr->SetSVDataPointer(43353, HType.ASCII_TYPE,   "C_Color_Fix AVG push time",                                           "", &fSmartDiagnostic->iPushAvgTime[3],                     "C_Color_Fix AVG push time"                                  );
        HGemPtr->SetSVDataPointer(43354, HType.ASCII_TYPE,   "C_Auto1EdgePush AVG push time",                                       "", &fSmartDiagnostic->iPushAvgTime[4],                     "C_Auto1EdgePush AVG push time"                              );
        HGemPtr->SetSVDataPointer(43355, HType.ASCII_TYPE,   "C_Auto1Side_Fixer AVG push time",                                     "", &fSmartDiagnostic->iPushAvgTime[5],                     "C_Auto1Side_Fixer AVG push time"                            );
        HGemPtr->SetSVDataPointer(43356, HType.ASCII_TYPE,   "C_Auto2EdgePush AVG push time",                                       "", &fSmartDiagnostic->iPushAvgTime[6],                     "C_Auto2EdgePush AVG push time"                              );
        HGemPtr->SetSVDataPointer(43357, HType.ASCII_TYPE,   "C_Auto2Side_Fixer AVG push time",                                     "", &fSmartDiagnostic->iPushAvgTime[7],                     "C_Auto2Side_Fixer AVG push time"                            );
        HGemPtr->SetSVDataPointer(43358, HType.ASCII_TYPE,   "C_Auto3EdgePush AVG push time",                                       "", &fSmartDiagnostic->iPushAvgTime[8],                     "C_Auto3EdgePush AVG push time"                              );
        HGemPtr->SetSVDataPointer(43359, HType.ASCII_TYPE,   "C_Auto3Side_Fixer AVG push time",                                     "", &fSmartDiagnostic->iPushAvgTime[9],                     "C_Auto3Side_Fixer AVG push time"                            );
        HGemPtr->SetSVDataPointer(43360, HType.ASCII_TYPE,   "C_LoaderEdgePush AVG pop time",                                      "", &fSmartDiagnostic->iPopAvgTime[0],                     "C_LoaderEdgePush AVG pop time"                             );
        HGemPtr->SetSVDataPointer(43361, HType.ASCII_TYPE,   "C_TrayY_Fixer AVG pop time",                                         "", &fSmartDiagnostic->iPopAvgTime[1],                     "C_TrayY_Fixer AVG pop time"                                );
        HGemPtr->SetSVDataPointer(43362, HType.ASCII_TYPE,   "C_Empty_Fix  AVG pop time",                                          "", &fSmartDiagnostic->iPopAvgTime[2],                     "C_Empty_Fix  AVG pop time"                                 );
        HGemPtr->SetSVDataPointer(43363, HType.ASCII_TYPE,   "C_Color_Fix AVG pop time",                                           "", &fSmartDiagnostic->iPopAvgTime[3],                     "C_Color_Fix AVG pop time"                                  );
        HGemPtr->SetSVDataPointer(43364, HType.ASCII_TYPE,   "C_Auto1EdgePush AVG pop time",                                       "", &fSmartDiagnostic->iPopAvgTime[4],                     "C_Auto1EdgePush AVG pop time"                              );
        HGemPtr->SetSVDataPointer(43365, HType.ASCII_TYPE,   "C_Auto1Side_Fixer AVG pop time",                                     "", &fSmartDiagnostic->iPopAvgTime[5],                     "C_Auto1Side_Fixer AVG pop time"                            );
        HGemPtr->SetSVDataPointer(43366, HType.ASCII_TYPE,   "C_Auto2EdgePush AVG pop time",                                       "", &fSmartDiagnostic->iPopAvgTime[6],                     "C_Auto2EdgePush AVG pop time"                              );
        HGemPtr->SetSVDataPointer(43367, HType.ASCII_TYPE,   "C_Auto2Side_Fixer AVG pop time",                                     "", &fSmartDiagnostic->iPopAvgTime[7],                     "C_Auto2Side_Fixer AVG pop time"                            );
        HGemPtr->SetSVDataPointer(43368, HType.ASCII_TYPE,   "C_Auto3EdgePush AVG pop time",                                       "", &fSmartDiagnostic->iPopAvgTime[8],                     "C_Auto3EdgePush AVG pop time"                              );
        HGemPtr->SetSVDataPointer(43369, HType.ASCII_TYPE,   "C_Auto3Side_Fixer AVG pop time",                                     "", &fSmartDiagnostic->iPopAvgTime[9],                     "C_Auto3Side_Fixer AVG pop time"                            );
#endif // GATE [G18]

        HGemPtr->SetSVDataPointer(62105, HType.BOOLEAN_TYPE, "SafeDoorStatus",                                                       "", &RunInfo.bSafeDoorIsOpen,                               "Safe Door Status"                                          );  //JerryYang 20240403
        HGemPtr->SetSVDataPointer(62106, HType.BOOLEAN_TYPE, "SenSafeDoor_1",                                                        "", &RunInfo.bSafeDoor[0],                                  "SenSafeDoor_1"                                             );  //Ifor 20200416 add SafeDoor report
        HGemPtr->SetSVDataPointer(62107, HType.BOOLEAN_TYPE, "SenSafeDoor_2",                                                        "", &RunInfo.bSafeDoor[1],                                  "SenSafeDoor_2"                                             );  //Ifor 20200416 add SafeDoor report
        HGemPtr->SetSVDataPointer(62108, HType.BOOLEAN_TYPE, "SenSafeDoor_3",                                                        "", &RunInfo.bSafeDoor[2],                                  "SenSafeDoor_3"                                             );  //Ifor 20200416 add SafeDoor report
        HGemPtr->SetSVDataPointer(62109, HType.BOOLEAN_TYPE, "SenSafeDoor_4",                                                        "", &RunInfo.bSafeDoor[3],                                  "SenSafeDoor_4"                                             );  //Ifor 20200416 add SafeDoor report
        HGemPtr->SetSVDataPointer(62110, HType.BOOLEAN_TYPE, "SenSafeDoor_5",                                                        "", &RunInfo.bSafeDoor[4],                                  "SenSafeDoor_5"                                             );  //Ifor 20200416 add SafeDoor report
        HGemPtr->SetSVDataPointer(62111, HType.BOOLEAN_TYPE, "SenSafeDoor_6",                                                        "", &RunInfo.bSafeDoor[5],                                  "SenSafeDoor_6"                                             );  //Ifor 20200416 add SafeDoor report
        HGemPtr->SetSVDataPointer(62112, HType.BOOLEAN_TYPE, "SenSafeDoor_7",                                                        "", &RunInfo.bSafeDoor[6],                                  "SenSafeDoor_7"                                             );  //Ifor 20200416 add SafeDoor report
        HGemPtr->SetSVDataPointer(62113, HType.BOOLEAN_TYPE, "SenSafeDoor_8",                                                        "", &RunInfo.bSafeDoor[7],                                  "SenSafeDoor_8"                                             );  //Ifor 20200416 add SafeDoor report
        HGemPtr->SetSVDataPointer(62114, HType.BOOLEAN_TYPE, "SenSafeDoor_9",                                                        "", &RunInfo.bSafeDoor[8],                                  "SenSafeDoor_9"                                             );  //Ifor 20200416 add SafeDoor report
        HGemPtr->SetSVDataPointer(62115, HType.BOOLEAN_TYPE, "SenSafeDoor_10",                                                       "", &RunInfo.bSafeDoor[9],                                  "SenSafeDoor_10"                                            );  //Ifor 20200416 add SafeDoor report

        HGemPtr->SetSVDataPointer(65000, HType.INT_4_TYPE,   "HeadCondition1 Head 1 Contact Count Aa",                               "", &IniConfig.HeadContactCount[0][0][0],                   "HeadCondition1 Head 1 Contact Count Aa"                    );   //Steven 20230314 : Add SVID for HeadContactCount
        HGemPtr->SetSVDataPointer(65001, HType.INT_4_TYPE,   "HeadCondition1 Head 1 Contact Count Ab",                               "", &IniConfig.HeadContactCount[0][0][2],                   "HeadCondition1 Head 1 Contact Count Ab"                    );
        HGemPtr->SetSVDataPointer(65002, HType.INT_4_TYPE,   "HeadCondition1 Head 1 Contact Count Ac",                               "", &IniConfig.HeadContactCount[0][0][4],                   "HeadCondition1 Head 1 Contact Count Ac"                    );
        HGemPtr->SetSVDataPointer(65003, HType.INT_4_TYPE,   "HeadCondition1 Head 1 Contact Count Ad",                               "", &IniConfig.HeadContactCount[0][0][6],                   "HeadCondition1 Head 1 Contact Count Ad"                    );
        HGemPtr->SetSVDataPointer(65004, HType.INT_4_TYPE,   "HeadCondition1 Head 1 Contact Count Ae",                               "", &IniConfig.HeadContactCount[0][0][8],                   "HeadCondition1 Head 1 Contact Count Ae"                    );
        HGemPtr->SetSVDataPointer(65005, HType.INT_4_TYPE,   "HeadCondition1 Head 1 Contact Count Af",                               "", &IniConfig.HeadContactCount[0][0][10],                  "HeadCondition1 Head 1 Contact Count Af"                    );
        HGemPtr->SetSVDataPointer(65006, HType.INT_4_TYPE,   "HeadCondition1 Head 1 Contact Count Ag",                               "", &IniConfig.HeadContactCount[0][0][12],                  "HeadCondition1 Head 1 Contact Count Ag"                    );
        HGemPtr->SetSVDataPointer(65007, HType.INT_4_TYPE,   "HeadCondition1 Head 1 Contact Count Ah",                               "", &IniConfig.HeadContactCount[0][0][14],                  "HeadCondition1 Head 1 Contact Count Ah"                    );
        HGemPtr->SetSVDataPointer(65008, HType.INT_4_TYPE,   "HeadCondition1 Head 1 Contact Count Ba",                               "", &IniConfig.HeadContactCount[0][0][1],                   "HeadCondition1 Head 1 Contact Count Ba"                    );
        HGemPtr->SetSVDataPointer(65009, HType.INT_4_TYPE,   "HeadCondition1 Head 1 Contact Count Bb",                               "", &IniConfig.HeadContactCount[0][0][3],                   "HeadCondition1 Head 1 Contact Count Bb"                    );
        HGemPtr->SetSVDataPointer(65010, HType.INT_4_TYPE,   "HeadCondition1 Head 1 Contact Count Bc",                               "", &IniConfig.HeadContactCount[0][0][5],                   "HeadCondition1 Head 1 Contact Count Bc"                    );
        HGemPtr->SetSVDataPointer(65011, HType.INT_4_TYPE,   "HeadCondition1 Head 1 Contact Count Bd",                               "", &IniConfig.HeadContactCount[0][0][7],                   "HeadCondition1 Head 1 Contact Count Bd"                    );
        HGemPtr->SetSVDataPointer(65012, HType.INT_4_TYPE,   "HeadCondition1 Head 1 Contact Count Be",                               "", &IniConfig.HeadContactCount[0][0][9],                   "HeadCondition1 Head 1 Contact Count Be"                    );
        HGemPtr->SetSVDataPointer(65013, HType.INT_4_TYPE,   "HeadCondition1 Head 1 Contact Count Bf",                               "", &IniConfig.HeadContactCount[0][0][11],                  "HeadCondition1 Head 1 Contact Count Bf"                    );
        HGemPtr->SetSVDataPointer(65014, HType.INT_4_TYPE,   "HeadCondition1 Head 1 Contact Count Bg",                               "", &IniConfig.HeadContactCount[0][0][13],                  "HeadCondition1 Head 1 Contact Count Bg"                    );
        HGemPtr->SetSVDataPointer(65015, HType.INT_4_TYPE,   "HeadCondition1 Head 1 Contact Count Bh",                               "", &IniConfig.HeadContactCount[0][0][15],                  "HeadCondition1 Head 1 Contact Count Bh"                    );
        HGemPtr->SetSVDataPointer(65016, HType.INT_4_TYPE,   "HeadCondition1 Head 2 Contact Count Aa",                               "", &IniConfig.HeadContactCount[0][1][0],                   "HeadCondition1 Head 2 Contact Count Aa"                    );
        HGemPtr->SetSVDataPointer(65017, HType.INT_4_TYPE,   "HeadCondition1 Head 2 Contact Count Ab",                               "", &IniConfig.HeadContactCount[0][1][2],                   "HeadCondition1 Head 2 Contact Count Ab"                    );
        HGemPtr->SetSVDataPointer(65018, HType.INT_4_TYPE,   "HeadCondition1 Head 2 Contact Count Ac",                               "", &IniConfig.HeadContactCount[0][1][4],                   "HeadCondition1 Head 2 Contact Count Ac"                    );
        HGemPtr->SetSVDataPointer(65019, HType.INT_4_TYPE,   "HeadCondition1 Head 2 Contact Count Ad",                               "", &IniConfig.HeadContactCount[0][1][6],                   "HeadCondition1 Head 2 Contact Count Ad"                    );
        HGemPtr->SetSVDataPointer(65020, HType.INT_4_TYPE,   "HeadCondition1 Head 2 Contact Count Ae",                               "", &IniConfig.HeadContactCount[0][1][8],                   "HeadCondition1 Head 2 Contact Count Ae"                    );
        HGemPtr->SetSVDataPointer(65021, HType.INT_4_TYPE,   "HeadCondition1 Head 2 Contact Count Af",                               "", &IniConfig.HeadContactCount[0][1][10],                  "HeadCondition1 Head 2 Contact Count Af"                    );
        HGemPtr->SetSVDataPointer(65022, HType.INT_4_TYPE,   "HeadCondition1 Head 2 Contact Count Ag",                               "", &IniConfig.HeadContactCount[0][1][12],                  "HeadCondition1 Head 2 Contact Count Ag"                    );
        HGemPtr->SetSVDataPointer(65023, HType.INT_4_TYPE,   "HeadCondition1 Head 2 Contact Count Ah",                               "", &IniConfig.HeadContactCount[0][1][14],                  "HeadCondition1 Head 2 Contact Count Ah"                    );
        HGemPtr->SetSVDataPointer(65024, HType.INT_4_TYPE,   "HeadCondition1 Head 2 Contact Count Ba",                               "", &IniConfig.HeadContactCount[0][1][1],                   "HeadCondition1 Head 2 Contact Count Ba"                    );
        HGemPtr->SetSVDataPointer(65025, HType.INT_4_TYPE,   "HeadCondition1 Head 2 Contact Count Bb",                               "", &IniConfig.HeadContactCount[0][1][3],                   "HeadCondition1 Head 2 Contact Count Bb"                    );
        HGemPtr->SetSVDataPointer(65026, HType.INT_4_TYPE,   "HeadCondition1 Head 2 Contact Count Bc",                               "", &IniConfig.HeadContactCount[0][1][5],                   "HeadCondition1 Head 2 Contact Count Bc"                    );
        HGemPtr->SetSVDataPointer(65027, HType.INT_4_TYPE,   "HeadCondition1 Head 2 Contact Count Bd",                               "", &IniConfig.HeadContactCount[0][1][7],                   "HeadCondition1 Head 2 Contact Count Bd"                    );
        HGemPtr->SetSVDataPointer(65028, HType.INT_4_TYPE,   "HeadCondition1 Head 2 Contact Count Be",                               "", &IniConfig.HeadContactCount[0][1][9],                   "HeadCondition1 Head 2 Contact Count Be"                    );
        HGemPtr->SetSVDataPointer(65029, HType.INT_4_TYPE,   "HeadCondition1 Head 2 Contact Count Bf",                               "", &IniConfig.HeadContactCount[0][1][11],                  "HeadCondition1 Head 2 Contact Count Bf"                    );
        HGemPtr->SetSVDataPointer(65030, HType.INT_4_TYPE,   "HeadCondition1 Head 2 Contact Count Bg",                               "", &IniConfig.HeadContactCount[0][1][13],                  "HeadCondition1 Head 2 Contact Count Bg"                    );
        HGemPtr->SetSVDataPointer(65031, HType.INT_4_TYPE,   "HeadCondition1 Head 2 Contact Count Bh",                               "", &IniConfig.HeadContactCount[0][1][15],                  "HeadCondition1 Head 2 Contact Count Bh"                    );
        HGemPtr->SetSVDataPointer(65032, HType.INT_4_TYPE,   "HeadCondition2 Head 1 Contact Count Aa",                               "", &IniConfig.HeadContactCount[1][0][0],                   "HeadCondition2 Head 1 Contact Count Aa"                    );
        HGemPtr->SetSVDataPointer(65033, HType.INT_4_TYPE,   "HeadCondition2 Head 1 Contact Count Ab",                               "", &IniConfig.HeadContactCount[1][0][2],                   "HeadCondition2 Head 1 Contact Count Ab"                    );
        HGemPtr->SetSVDataPointer(65034, HType.INT_4_TYPE,   "HeadCondition2 Head 1 Contact Count Ac",                               "", &IniConfig.HeadContactCount[1][0][4],                   "HeadCondition2 Head 1 Contact Count Ac"                    );
        HGemPtr->SetSVDataPointer(65035, HType.INT_4_TYPE,   "HeadCondition2 Head 1 Contact Count Ad",                               "", &IniConfig.HeadContactCount[1][0][6],                   "HeadCondition2 Head 1 Contact Count Ad"                    );
        HGemPtr->SetSVDataPointer(65036, HType.INT_4_TYPE,   "HeadCondition2 Head 1 Contact Count Ae",                               "", &IniConfig.HeadContactCount[1][0][8],                   "HeadCondition2 Head 1 Contact Count Ae"                    );
        HGemPtr->SetSVDataPointer(65037, HType.INT_4_TYPE,   "HeadCondition2 Head 1 Contact Count Af",                               "", &IniConfig.HeadContactCount[1][0][10],                  "HeadCondition2 Head 1 Contact Count Af"                    );
        HGemPtr->SetSVDataPointer(65038, HType.INT_4_TYPE,   "HeadCondition2 Head 1 Contact Count Ag",                               "", &IniConfig.HeadContactCount[1][0][12],                  "HeadCondition2 Head 1 Contact Count Ag"                    );
        HGemPtr->SetSVDataPointer(65039, HType.INT_4_TYPE,   "HeadCondition2 Head 1 Contact Count Ah",                               "", &IniConfig.HeadContactCount[1][0][14],                  "HeadCondition2 Head 1 Contact Count Ah"                    );
        HGemPtr->SetSVDataPointer(65040, HType.INT_4_TYPE,   "HeadCondition2 Head 1 Contact Count Ba",                               "", &IniConfig.HeadContactCount[1][0][1],                   "HeadCondition2 Head 1 Contact Count Ba"                    );
        HGemPtr->SetSVDataPointer(65041, HType.INT_4_TYPE,   "HeadCondition2 Head 1 Contact Count Bb",                               "", &IniConfig.HeadContactCount[1][0][3],                   "HeadCondition2 Head 1 Contact Count Bb"                    );
        HGemPtr->SetSVDataPointer(65042, HType.INT_4_TYPE,   "HeadCondition2 Head 1 Contact Count Bc",                               "", &IniConfig.HeadContactCount[1][0][5],                   "HeadCondition2 Head 1 Contact Count Bc"                    );
        HGemPtr->SetSVDataPointer(65043, HType.INT_4_TYPE,   "HeadCondition2 Head 1 Contact Count Bd",                               "", &IniConfig.HeadContactCount[1][0][7],                   "HeadCondition2 Head 1 Contact Count Bd"                    );
        HGemPtr->SetSVDataPointer(65044, HType.INT_4_TYPE,   "HeadCondition2 Head 1 Contact Count Be",                               "", &IniConfig.HeadContactCount[1][0][9],                   "HeadCondition2 Head 1 Contact Count Be"                    );
        HGemPtr->SetSVDataPointer(65045, HType.INT_4_TYPE,   "HeadCondition2 Head 1 Contact Count Bf",                               "", &IniConfig.HeadContactCount[1][0][11],                  "HeadCondition2 Head 1 Contact Count Bf"                    );
        HGemPtr->SetSVDataPointer(65046, HType.INT_4_TYPE,   "HeadCondition2 Head 1 Contact Count Bg",                               "", &IniConfig.HeadContactCount[1][0][13],                  "HeadCondition2 Head 1 Contact Count Bg"                    );
        HGemPtr->SetSVDataPointer(65047, HType.INT_4_TYPE,   "HeadCondition2 Head 1 Contact Count Bh",                               "", &IniConfig.HeadContactCount[1][0][15],                  "HeadCondition2 Head 1 Contact Count Bh"                    );
        HGemPtr->SetSVDataPointer(65048, HType.INT_4_TYPE,   "HeadCondition2 Head 2 Contact Count Aa",                               "", &IniConfig.HeadContactCount[1][1][0],                   "HeadCondition2 Head 2 Contact Count Aa"                    );
        HGemPtr->SetSVDataPointer(65049, HType.INT_4_TYPE,   "HeadCondition2 Head 2 Contact Count Ab",                               "", &IniConfig.HeadContactCount[1][1][2],                   "HeadCondition2 Head 2 Contact Count Ab"                    );
        HGemPtr->SetSVDataPointer(65050, HType.INT_4_TYPE,   "HeadCondition2 Head 2 Contact Count Ac",                               "", &IniConfig.HeadContactCount[1][1][4],                   "HeadCondition2 Head 2 Contact Count Ac"                    );
        HGemPtr->SetSVDataPointer(65051, HType.INT_4_TYPE,   "HeadCondition2 Head 2 Contact Count Ad",                               "", &IniConfig.HeadContactCount[1][1][6],                   "HeadCondition2 Head 2 Contact Count Ad"                    );
        HGemPtr->SetSVDataPointer(65052, HType.INT_4_TYPE,   "HeadCondition2 Head 2 Contact Count Ae",                               "", &IniConfig.HeadContactCount[1][1][8],                   "HeadCondition2 Head 2 Contact Count Ae"                    );
        HGemPtr->SetSVDataPointer(65053, HType.INT_4_TYPE,   "HeadCondition2 Head 2 Contact Count Af",                               "", &IniConfig.HeadContactCount[1][1][10],                  "HeadCondition2 Head 2 Contact Count Af"                    );
        HGemPtr->SetSVDataPointer(65054, HType.INT_4_TYPE,   "HeadCondition2 Head 2 Contact Count Ag",                               "", &IniConfig.HeadContactCount[1][1][12],                  "HeadCondition2 Head 2 Contact Count Ag"                    );
        HGemPtr->SetSVDataPointer(65055, HType.INT_4_TYPE,   "HeadCondition2 Head 2 Contact Count Ah",                               "", &IniConfig.HeadContactCount[1][1][14],                  "HeadCondition2 Head 2 Contact Count Ah"                    );
        HGemPtr->SetSVDataPointer(65056, HType.INT_4_TYPE,   "HeadCondition2 Head 2 Contact Count Ba",                               "", &IniConfig.HeadContactCount[1][1][1],                   "HeadCondition2 Head 2 Contact Count Ba"                    );
        HGemPtr->SetSVDataPointer(65057, HType.INT_4_TYPE,   "HeadCondition2 Head 2 Contact Count Bb",                               "", &IniConfig.HeadContactCount[1][1][3],                   "HeadCondition2 Head 2 Contact Count Bb"                    );
        HGemPtr->SetSVDataPointer(65058, HType.INT_4_TYPE,   "HeadCondition2 Head 2 Contact Count Bc",                               "", &IniConfig.HeadContactCount[1][1][5],                   "HeadCondition2 Head 2 Contact Count Bc"                    );
        HGemPtr->SetSVDataPointer(65059, HType.INT_4_TYPE,   "HeadCondition2 Head 2 Contact Count Bd",                               "", &IniConfig.HeadContactCount[1][1][7],                   "HeadCondition2 Head 2 Contact Count Bd"                    );
        HGemPtr->SetSVDataPointer(65060, HType.INT_4_TYPE,   "HeadCondition2 Head 2 Contact Count Be",                               "", &IniConfig.HeadContactCount[1][1][9],                   "HeadCondition2 Head 2 Contact Count Be"                    );
        HGemPtr->SetSVDataPointer(65061, HType.INT_4_TYPE,   "HeadCondition2 Head 2 Contact Count Bf",                               "", &IniConfig.HeadContactCount[1][1][11],                  "HeadCondition2 Head 2 Contact Count Bf"                    );
        HGemPtr->SetSVDataPointer(65062, HType.INT_4_TYPE,   "HeadCondition2 Head 2 Contact Count Bg",                               "", &IniConfig.HeadContactCount[1][1][13],                  "HeadCondition2 Head 2 Contact Count Bg"                    );
        HGemPtr->SetSVDataPointer(65063, HType.INT_4_TYPE,   "HeadCondition2 Head 2 Contact Count Bh",                               "", &IniConfig.HeadContactCount[1][1][15],                  "HeadCondition2 Head 2 Contact Count Bh"                    );
        HGemPtr->SetSVDataPointer(65064, HType.INT_4_TYPE,   "HeadCondition3 Head 1 Contact Count Aa",                               "", &IniConfig.HeadContactCount[2][0][0],                   "HeadCondition3 Head 1 Contact Count Aa"                    );
        HGemPtr->SetSVDataPointer(65065, HType.INT_4_TYPE,   "HeadCondition3 Head 1 Contact Count Ab",                               "", &IniConfig.HeadContactCount[2][0][2],                   "HeadCondition3 Head 1 Contact Count Ab"                    );
        HGemPtr->SetSVDataPointer(65066, HType.INT_4_TYPE,   "HeadCondition3 Head 1 Contact Count Ac",                               "", &IniConfig.HeadContactCount[2][0][4],                   "HeadCondition3 Head 1 Contact Count Ac"                    );
        HGemPtr->SetSVDataPointer(65067, HType.INT_4_TYPE,   "HeadCondition3 Head 1 Contact Count Ad",                               "", &IniConfig.HeadContactCount[2][0][6],                   "HeadCondition3 Head 1 Contact Count Ad"                    );
        HGemPtr->SetSVDataPointer(65068, HType.INT_4_TYPE,   "HeadCondition3 Head 1 Contact Count Ae",                               "", &IniConfig.HeadContactCount[2][0][8],                   "HeadCondition3 Head 1 Contact Count Ae"                    );
        HGemPtr->SetSVDataPointer(65069, HType.INT_4_TYPE,   "HeadCondition3 Head 1 Contact Count Af",                               "", &IniConfig.HeadContactCount[2][0][10],                  "HeadCondition3 Head 1 Contact Count Af"                    );
        HGemPtr->SetSVDataPointer(65070, HType.INT_4_TYPE,   "HeadCondition3 Head 1 Contact Count Ag",                               "", &IniConfig.HeadContactCount[2][0][12],                  "HeadCondition3 Head 1 Contact Count Ag"                    );
        HGemPtr->SetSVDataPointer(65071, HType.INT_4_TYPE,   "HeadCondition3 Head 1 Contact Count Ah",                               "", &IniConfig.HeadContactCount[2][0][14],                  "HeadCondition3 Head 1 Contact Count Ah"                    );
        HGemPtr->SetSVDataPointer(65072, HType.INT_4_TYPE,   "HeadCondition3 Head 1 Contact Count Ba",                               "", &IniConfig.HeadContactCount[2][0][1],                   "HeadCondition3 Head 1 Contact Count Ba"                    );
        HGemPtr->SetSVDataPointer(65073, HType.INT_4_TYPE,   "HeadCondition3 Head 1 Contact Count Bb",                               "", &IniConfig.HeadContactCount[2][0][3],                   "HeadCondition3 Head 1 Contact Count Bb"                    );
        HGemPtr->SetSVDataPointer(65074, HType.INT_4_TYPE,   "HeadCondition3 Head 1 Contact Count Bc",                               "", &IniConfig.HeadContactCount[2][0][5],                   "HeadCondition3 Head 1 Contact Count Bc"                    );
        HGemPtr->SetSVDataPointer(65075, HType.INT_4_TYPE,   "HeadCondition3 Head 1 Contact Count Bd",                               "", &IniConfig.HeadContactCount[2][0][7],                   "HeadCondition3 Head 1 Contact Count Bd"                    );
        HGemPtr->SetSVDataPointer(65076, HType.INT_4_TYPE,   "HeadCondition3 Head 1 Contact Count Be",                               "", &IniConfig.HeadContactCount[2][0][9],                   "HeadCondition3 Head 1 Contact Count Be"                    );
        HGemPtr->SetSVDataPointer(65077, HType.INT_4_TYPE,   "HeadCondition3 Head 1 Contact Count Bf",                               "", &IniConfig.HeadContactCount[2][0][11],                  "HeadCondition3 Head 1 Contact Count Bf"                    );
        HGemPtr->SetSVDataPointer(65078, HType.INT_4_TYPE,   "HeadCondition3 Head 1 Contact Count Bg",                               "", &IniConfig.HeadContactCount[2][0][13],                  "HeadCondition3 Head 1 Contact Count Bg"                    );
        HGemPtr->SetSVDataPointer(65079, HType.INT_4_TYPE,   "HeadCondition3 Head 1 Contact Count Bh",                               "", &IniConfig.HeadContactCount[2][0][15],                  "HeadCondition3 Head 1 Contact Count Bh"                    );
        HGemPtr->SetSVDataPointer(65080, HType.INT_4_TYPE,   "HeadCondition3 Head 2 Contact Count Aa",                               "", &IniConfig.HeadContactCount[2][1][0],                   "HeadCondition3 Head 2 Contact Count Aa"                    );
        HGemPtr->SetSVDataPointer(65081, HType.INT_4_TYPE,   "HeadCondition3 Head 2 Contact Count Ab",                               "", &IniConfig.HeadContactCount[2][1][2],                   "HeadCondition3 Head 2 Contact Count Ab"                    );
        HGemPtr->SetSVDataPointer(65082, HType.INT_4_TYPE,   "HeadCondition3 Head 2 Contact Count Ac",                               "", &IniConfig.HeadContactCount[2][1][4],                   "HeadCondition3 Head 2 Contact Count Ac"                    );
        HGemPtr->SetSVDataPointer(65083, HType.INT_4_TYPE,   "HeadCondition3 Head 2 Contact Count Ad",                               "", &IniConfig.HeadContactCount[2][1][6],                   "HeadCondition3 Head 2 Contact Count Ad"                    );
        HGemPtr->SetSVDataPointer(65084, HType.INT_4_TYPE,   "HeadCondition3 Head 2 Contact Count Ae",                               "", &IniConfig.HeadContactCount[2][1][8],                   "HeadCondition3 Head 2 Contact Count Ae"                    );
        HGemPtr->SetSVDataPointer(65085, HType.INT_4_TYPE,   "HeadCondition3 Head 2 Contact Count Af",                               "", &IniConfig.HeadContactCount[2][1][10],                  "HeadCondition3 Head 2 Contact Count Af"                    );
        HGemPtr->SetSVDataPointer(65086, HType.INT_4_TYPE,   "HeadCondition3 Head 2 Contact Count Ag",                               "", &IniConfig.HeadContactCount[2][1][12],                  "HeadCondition3 Head 2 Contact Count Ag"                    );
        HGemPtr->SetSVDataPointer(65087, HType.INT_4_TYPE,   "HeadCondition3 Head 2 Contact Count Ah",                               "", &IniConfig.HeadContactCount[2][1][14],                  "HeadCondition3 Head 2 Contact Count Ah"                    );
        HGemPtr->SetSVDataPointer(65088, HType.INT_4_TYPE,   "HeadCondition3 Head 2 Contact Count Ba",                               "", &IniConfig.HeadContactCount[2][1][1],                   "HeadCondition3 Head 2 Contact Count Ba"                    );
        HGemPtr->SetSVDataPointer(65089, HType.INT_4_TYPE,   "HeadCondition3 Head 2 Contact Count Bb",                               "", &IniConfig.HeadContactCount[2][1][3],                   "HeadCondition3 Head 2 Contact Count Bb"                    );
        HGemPtr->SetSVDataPointer(65090, HType.INT_4_TYPE,   "HeadCondition3 Head 2 Contact Count Bc",                               "", &IniConfig.HeadContactCount[2][1][5],                   "HeadCondition3 Head 2 Contact Count Bc"                    );
        HGemPtr->SetSVDataPointer(65091, HType.INT_4_TYPE,   "HeadCondition3 Head 2 Contact Count Bd",                               "", &IniConfig.HeadContactCount[2][1][7],                   "HeadCondition3 Head 2 Contact Count Bd"                    );
        HGemPtr->SetSVDataPointer(65092, HType.INT_4_TYPE,   "HeadCondition3 Head 2 Contact Count Be",                               "", &IniConfig.HeadContactCount[2][1][9],                   "HeadCondition3 Head 2 Contact Count Be"                    );
        HGemPtr->SetSVDataPointer(65093, HType.INT_4_TYPE,   "HeadCondition3 Head 2 Contact Count Bf",                               "", &IniConfig.HeadContactCount[2][1][11],                  "HeadCondition3 Head 2 Contact Count Bf"                    );
        HGemPtr->SetSVDataPointer(65094, HType.INT_4_TYPE,   "HeadCondition3 Head 2 Contact Count Bg",                               "", &IniConfig.HeadContactCount[2][1][13],                  "HeadCondition3 Head 2 Contact Count Bg"                    );
        HGemPtr->SetSVDataPointer(65095, HType.INT_4_TYPE,   "HeadCondition3 Head 2 Contact Count Bh",                               "", &IniConfig.HeadContactCount[2][1][15],                  "HeadCondition3 Head 2 Contact Count Bh"                    );
    }
}
//------------------------------------------------------------------------------

//==============================================================================
//  AI(pt-wave) 20260811 PT-W9: the member wrapper that turns the free function
//  above into golden's real `HT9045Gem::AddSV()` override.
//
//  GATE [G1]'s own instruction was "add `virtual void AddSV();` to
//  SECSGEM/uHGemHT9045.h, then flip both arms here".  The declaration is added
//  (golden uHGemHT9045.h:346).  I did NOT flip the arms: flipping means editing
//  the definition head plus all 5 `Self->` sites, i.e. touching the verified
//  1,000-line body, to gain nothing a wrapper does not already give.  The
//  wrapper is also exactly the shape the sibling _EC.cpp already uses for
//  AddEC (its GATE g31), so both halves of the SV/EC pair now look the same.
//  The `#if 0` arms in the body stay as golden-verbatim reference text.
//
//  EFFECT: a host S1F3/S1F11 now reaches this dictionary instead of HTGem's
//  empty base virtual.  That is the whole point of the change and it is a
//  BEHAVIOUR CHANGE -- see this wave's DEVLOG entry.
//==============================================================================
void HT9045Gem::AddSV()
{
    HT9045Gem_AddSV(this);
}
