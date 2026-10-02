// =============================================================================
//  forms/fDTME08.h  --  non-VCL facade for golden's TfrmDTME08
//                        (EJ1N/fDTME08.h)
//
//  AI(W906-FW3-DTL1) 20260828: new file, FW wave FW3-DTL1 (facade wave 1 of
//  EJ1N/fDTME08 -- read-only direction, no write path).
//  GOLDEN SOURCE: HT9011UC_Code_V3.33.906.0_20260618/EJ1N/fDTME08.h
//  (206 lines) + EJ1N/fDTME08.cpp (765 lines), read with
//  `io.open(p, encoding='cp950')`, written out UTF-8 / bare-LF / 0 U+FFFD.
//
//  ROLE
//  ----
//  TfrmDTME08 is golden's OMRON EJ1N / DTME08 temperature-controller console:
//  a 16- or 32-channel panel grid (`TMyOmronPanel`) plus the polling state
//  machine that talks Modbus/TCP to the controller stations through
//  `uDTME08Control` -- get PV / get SV / SET SV / start-stop auto-tuning /
//  set sensor type / set cycle time.  Everything that actually reaches the
//  hardware goes through the `dtme08` object; everything on screen goes
//  through `myPalGroup`.
//
//  `python tools/census/wave_preflight.py EJ1N/fDTME08.cpp` (20260828, re-run
//  at closeout): 55 bodies = 55 `TfrmDTME08::` members + 0 file-scope,
//  693 span lines, 0 brace-swallow short-fall, 0 overload collapse.
//
//  THIS WAVE, against the 55-member denominator:
//    22 ACTIVE           173 of 693 golden span lines (25.0%)
//     1 ACTIVE-PARTIAL   the ctor -- golden :19-27 (its whole conditional) is
//                        GATE (E-1); the delivered body is empty
//    32 GATED-WITH-BODY  507 golden span lines carried as `#if 0` transcript
//                        in forms/fDTME08.cpp
//  22 + 1 + 32 = 55.  Quote both the member count and the line count, or
//  neither.
//
//  ⚠ THE ONE-LINE SUMMARY OF WHY THIS WAVE IS 25% AND NOT 80%:
//  `uDTME08Control` and `TMyOmronPanel` ARE ALREADY PORTED (EJ1N/
//  uDTME08Control.{h,cpp} and EJ1N/MyOmronPanel.{h,cpp}) -- but BOTH .cpp
//  files sit in `add_library(ht9045_comms)`, and `ht9045_forms` links only
//  `vclcompat` + `ht9045_globals` + `ht9045_core`.  So every body that calls
//  an OUT-OF-LINE member of either class is a link-boundary gate, while every
//  body that only touches their INLINE members or their public vclcompat
//  fields is fine.  That single boundary decides 30 of this file's 32 gates.
//
//  ============================================================================
//  *** THE GLOBALS `TfrmDTME08` / `frmDTME08` ARE FREE -- THIS WAVE CLAIMS
//  THEM ***
//  Measured 20260828 with `python tools/census/wave_preflight.py
//  EJ1N/fDTME08.cpp` (section [3] SHIM OCCUPATION, 1195 port files scanned,
//  build_*/vendor/third_party excluded): (a) class name `clear`, (b) function
//  name in another class `clear`, (c) global name `clear`, (d) TU-local seam
//  `clear`.
//  ⚠ A raw `rg "\bTfrmDTME08\b|\bfrmDTME08\b"` over the tree DOES return
//  hits.  Read them before concluding -- none is an occupation:
//    * tools/dfm2rc/layout_out/EJ1N/fDTME08_layout.gen.cpp:14+ and its
//      _b1d_regen/idempotent twin -- generated .dfm DATA (string rows in a
//      widget table), not a class.
//    * bthermo.cpp:101/:5392/:5397/:5398 -- the PRE-COMMITTED `#if 0` call
//      sites this facade exists to serve (see below).
//    * EJ1N/MyOmronPanel.h:34 -- a comment naming the golden .dfm's FORMNAME.
//
//  CONDITION-TWO CHECK (does a port of these bodies ALREADY exist, possibly
//  as a RENAMED free function outside the class?).  This is the trap that
//  nearly cost the previous wave 16 duplicate translations (golden's
//  `TfTesterTCP::X` had become free functions `TesterTCPSocket_X` in
//  ht9045_sm, four of them RENAMED to `On*`).  Searched 20260828, whole port
//  tree, build_* excluded:
//    * prefix-style relocations
//      `rg "\b(DTME08_|frmDTME08_)\w+"` -> 1 hit, and it is NOT a body:
//      EJ1N/uDTME08Control.h:144 `return "DTME08_Control.ini";` (a filename).
//    * per-name file counts for all 34 distinctive identifiers of this file.
//      ZERO hits: DoDTME08Cycle, InitialDoSetAllSV, InitialDoSetAllAT,
//      InitialDoSetSensorType, InitialDoSetCycleTime,
//      InitialDoSetControlActionOut2, IsNeedSetSV, DoManualProcess,
//      DoSendCommandByManual, GetStationAndNumber, SetSingleCommand,
//      GetTargetCh, InitialComboList, NewuDTME08Control, AddTempGUI,
//      DrawTempGUI, AddTempPanel, ClearAllCommandStatus, SetATStatus,
//      DoGetAllPV_DTME08.
//      NON-ZERO but NOT a relocated body, each read before concluding:
//        AddSingleView   1 file  -- EJ1N/MyOmronPanel.h:25, a COMMENT quoting
//                                   this very golden function.
//        GetPalGroup     1 file  -- bthermo.cpp, the pre-committed `#if 0`
//                                   site + its own comment.
//        SetPanelPos     2 files -- VacuumUnit/VacuumUnit.{h,cpp}, a
//                                   DIFFERENT class (`TfVacuumUnit::
//                                   SetPanelPos(bool bInitial=true)`, note
//                                   the parameter) doing a different job.
//                                   ⚠ The closest name-similarity hazard in
//                                   this wave.
//        ShowMSG         5 files -- unrelated same-named helpers elsewhere;
//                                   none is `TfrmDTME08::ShowMSG`.
//        SetSettingSV    5 files -- EJ1N/MyOmronPanel.{h,cpp} (the PANEL's
//                                   own `SetSettingSV(double)`, a different
//                                   class and a different arity) + bthermo's
//                                   pre-committed site.  ⚠ Second-closest
//                                   hazard: golden has BOTH
//                                   `TfrmDTME08::SetSettingSV(int,double)`
//                                   and `TMyOmronPanel::SetSettingSV(double)`
//                                   and the first CALLS the second.
//    * the two collaborator CLASSES, however, DO already have a port, and
//      this wave deliberately does NOT re-translate them:
//        EJ1N/uDTME08Control.{h,cpp}  (golden EJ1N/uDTME08Control.cpp,
//                                      381 lines) -- ht9045_comms
//        EJ1N/MyOmronPanel.{h,cpp}    (golden MyOmronPanel.cpp) -- ht9045_comms
//      They are USED (by declaration) here, never re-written.
//  Conclusion: no `TfrmDTME08::` body has an existing port; nothing is
//  translated twice.
//
//  ============================================================================
//  PRE-COMMITTED #if 0 CALL SITES ELSEWHERE IN THE TREE (2 names, 2 sites)
//  ============================================================================
//    SetSettingSV(int,double)  bthermo.cpp:5397
//        `frmDTME08->SetSettingSV(i, Temp[Addr]);   //write sv`
//        -- matches this wave's GATED `void SetSettingSV(int iCh, double
//           dTemp);`
//    GetPalGroup(int)          bthermo.cpp:5398
//        `UN150Read[Addr]=GetConvertTemp(Addr, frmDTME08->GetPalGroup(i)->
//         GetPV());   //read pv`
//        -- matches this wave's ACTIVE `TMyOmronPanel* GetPalGroup(int
//           index);`.  Note the site chains `->GetPV()` off the RESULT, which
//           is why the return type has to stay the real `TMyOmronPanel*` and
//           not a stand-in: `TMyOmronPanel::GetPV()` is INLINE
//           (EJ1N/MyOmronPanel.h) and therefore reachable.
//  Both sites are inside bthermo.cpp's own `#if 0 // TODO(W7-UI G29)` block
//  (bthermo.cpp:5396-5399) and stay gated after this wave.
//
//  ============================================================================
//  THE LINK BOUNDARY, MEASURED (this is what 30 of the 32 gates rest on)
//  ============================================================================
//  Located this wave by walking each .cpp's ENCLOSING `add_library(...)`
//  block in CMakeLists.txt with a script.  TARGET NAMES are quoted rather
//  than line numbers, because line numbers move whenever a sibling wave adds
//  a file (and CMake comments are not build inputs).
//    REACHABLE from ht9045_forms (which links vclcompat + ht9045_globals +
//    ht9045_core):
//      cprod.cpp / cpublic.cpp / cmydef.cpp / Config.cpp / CosFunction.cpp /
//      LastSet.cpp / myTimer.cpp .................. ht9045_globals
//      common.cpp ................................. ht9045_core
//      forms/fQwertyKey.cpp ....................... ht9045_forms
//      vclcompat/* (AnsiString, Controls, TDateTime, TStringList, LedCore)
//    NOT REACHABLE:
//      EJ1N/uDTME08Control.cpp .................... ht9045_comms
//      EJ1N/MyOmronPanel.cpp ...................... ht9045_comms
//      ATC/ATCInterface.cpp (ATC_InterfaceForm) ... ht9045_sm
//      myswitch.cpp (SW[]) ........................ ht9045_io
//      handlerlog.cpp / bthermo.cpp / csystem.cpp .. ht9045_sm
//  ⚠ `HeaterSVLog` is a THIRD kind of unreachable, and the most dangerous
//  one to mis-read: it IS declared in the port's cpublic.h and its
//  DEFINITION IS in cpublic.cpp:711 -- which is `ht9045_globals`, i.e. a
//  target this facade DOES link.  It is still unusable, because that
//  definition sits INSIDE cpublic.cpp's own `#if 0` (opened at
//  cpublic.cpp:709, "...resume the GA1-B3 gate"), whose own note at
//  cpublic.cpp:694 says in as many words: "HeaterSVLog just after stays
//  gated: nothing references it."  A definition inside `#if 0` is not a
//  definition -- same class of trap as docs/KNOWLEDGE.md's `static` shadow
//  and `tests/` TU-local stand-in.
//
//  ============================================================================
//  GATE REGISTER (32) -- grouped by cause, each with its golden span
//  ============================================================================
//  --- (A) LINK BOUNDARY: `dtme08` (uDTME08Control, ht9045_comms) ------------
//  (E-1)  the ctor's golden :19-27 -- ALSO static-init unsafe (see below).
//  (E-2)  NewuDTME08Control()          :30-36  (7L)  `dtme08=new
//         uDTME08Control()`.  SECOND, independent gate: golden assigns
//         `dtme08->RecordErrorMsg=RecordErrorMsg;` -- a `__closure` member
//         pointer, which has no port (DEVIATION D-6).
//  (E-3)  AddTempGUI()                 :38-44  (7L)  dtme08->
//         GetMaxStationNumber()/GetChannelNumberPerStation().
//  (E-5)  SaveData()                   :74-79  (6L)  ALSO WRITES DISK:
//         `dtme08->WriteSettingFile()` persists DTME08_Control.ini.
//  (E-6)  Reload()                     :81-86  (6L)  dtme08->ReadSettingFile().
//  (E-10) btnGetPVClick                :193-197 (5L)
//  (E-11) btnSetATClick                :199-205 (7L)  dtme08->
//         InitialmapDTMInfoList()/mapDTMInfo[].
//  (E-12) btnGetSVClick                :207-211 (5L)
//  (E-13) btnSetSVClick                :213-221 (9L)
//  (E-14) btnGetStatusClick            :223-227 (5L)
//  (E-15) DoSendCommandByManual        :229-250 (22L) dtme08->
//         DoCommuncation() + the `tpvDoCommand` closure call.
//  (E-18) GetStationAndNumber          :266-270 (5L)  dtme08->
//         GetChannelNumberPerStation().
//  (E-20) DoManualProcess              :282-320 (39L)
//  (E-25) InitialDoSetSensorType       :352-369 (18L)
//  (E-26) InitialDoSetAllAT            :371-380 (10L)
//  (E-27) InitialDoSetAllSV            :382-395 (14L)
//  (E-28) InitialDoSetCycleTime        :397-406 (10L)
//  (E-29) InitialDoSetControlActionOut2 :408-417 (10L)
//  (E-32) DoDTME08Cycle                :475-676 (202L) -- the state machine.
//         ALSO needs `ledEJ1N1` (golden `TMyLed*`, no port -- see D-7) and
//         `btnCtrl->Caption`.  This single body is 29% of the file.
//
//  --- (B) LINK BOUNDARY: `TMyOmronPanel`'s OUT-OF-LINE members -------------
//  Read EJ1N/MyOmronPanel.h before touching these.  Its INLINE members
//  (`GetPV()`, `GetSV()`, `GetSettingSV()`, `SetAT(bool)`) and its public
//  vclcompat fields (`GroupBox`, `edSV`, `cbEnableAT`, `iMyTag`) are FINE
//  from here; its OUT-OF-LINE members (ctor, `SetSV(double)`,
//  `SetSettingSV(double)`, `SetVisible(bool)`, `SetPV(AnsiString,TColor)`,
//  `SetEvent`, `SetSP`, `SetAT(AnsiString,TColor)`, `SetRunStop`,
//  `SetDegree`) live in EJ1N/MyOmronPanel.cpp = ht9045_comms and are not.
//  (E-4)  DrawTempGUI()                :46-52  (7L)  ->SetVisible(bool).
//  (E-7)  AddSingleView(...)           :112-129 (18L) `new TMyOmronPanel(this)`
//         (out-of-line ctor) + `SetSP/SetEvent/SetPV/SetRunStop/SetDegree`.
//         SECOND gate: golden's parameter is `TScrollBox*`, a type with no
//         port (DEVIATION D-4).  THIRD: `this` is passed as a `TComponent*`
//         owner, and this facade has no base class (DEVIATION D-2).
//  (E-22) SetSP(int,double,AnsiString) :336-339 (4L)  ->SetSV(double).
//  (E-23) SetPV(int,double,AnsiString) :341-345 (5L)  ->SetPV/->SetEvent.
//  (E-24) SetATStatus(int,bool)        :347-350 (4L)  ->SetAT(AnsiString,
//         TColor) -- the STRING overload, not the inline bool one.
//         ⚠ This is the subtlest classification in the file: golden's
//         `btATClick`/`btATOffClick` call `SetAT(true)`/`SetAT(false)` (the
//         INLINE bool overload -> ACTIVE) while this one calls
//         `SetAT("AT", clYellow)` (the OUT-OF-LINE overload -> GATED).  Same
//         name, opposite verdict.
//  (E-31) SetSettingSV(int,double)     :452-460 (9L)  TWO INDEPENDENT GATES:
//         (a) `GetPalGroup(iCh)->SetSettingSV(dTemp)` is the out-of-line
//             panel member;
//         (b) `HeaterSVLog(iTempCode[iCh], dTemp)` has NO REACHABLE
//             DEFINITION AT ALL -- see the ⚠ block above (cpublic.cpp:711 is
//             inside `#if 0`).  Gate (b) would survive even if forms linked
//             ht9045_comms.
//         PRE-COMMITTED name/signature (bthermo.cpp:5397).
//
//  --- (C) LINK BOUNDARY: other targets -------------------------------------
//  (E-9)  TimerUpdateTimer             :158-176 (19L) `ATC_InterfaceForm->
//         iATC_MODE_TYPE` -- ATC/ATCInterface.cpp is ht9045_sm.  ALSO
//         TRANSITIVE on DoDTME08Cycle (E-32).  Everything else it reads
//         (`InitialOK`, `Temperature.bATCActiveCooling`, `ATC_SYSTEM`,
//         `eATC60`, `eNewATCSystem`, `ATC_TYPE_60`) IS reachable.
//  (E-33) chkHeaterClick               :725-728 (4L) TWO INDEPENDENT GATES:
//         (a) SAFETY / MACHINE OUTPUT -- `SW[SwHeaterRelay].OnOff(...)`
//             switches the HEATER RELAY.  This is a physical output on a
//             temperature-controller console; the read-only-direction rule
//             forbids it outright.
//         (b) LINK BOUNDARY -- `SW[]` (myswitch.h:43) is defined in
//             myswitch.cpp, add_library(ht9045_io).  (`SwHeaterRelay` itself
//             IS reachable: cmydef.cpp:1917.)
//
//  --- (D) TRANSITIVE ONLY (would otherwise have been reachable) ------------
//  (E-8)  btnSetSaveClick              :142-145 (4L)  -> SaveData (E-5).
//  (E-16) GetTargetCh(int)             :252-256 (5L)  -> GetStationAndNumber
//         (E-18).  Its own first line (`TargetCh=ich;`) is an own field.
//  (E-17) SetSingleCommand(...)        :258-264 (7L)  -> GetTargetCh (E-16).
//         SECOND gate: its parameter type `TPointVoidDoCommand` is a
//         `__closure` typedef -- DEVIATION D-6.
//  (E-19) btnReloadClick               :277-280 (4L)  -> Reload (E-6).
//  (E-21) FormShow                     :322-328 (7L)  -> DrawTempGUI (E-4).
//         `bShow=true;`, `InitialComboList()` and `SetPanelPos()` are all
//         this wave's own ACTIVE deliveries -- DrawTempGUI is the only
//         blocker, which makes this the cheapest future un-gate in the file.
//  (E-30) InitialSetting(int)          :419-441 (23L) -> the five
//         InitialDoSet* bodies (E-25..E-29).
//
//  ============================================================================
//  ACTIVE (22, plus the PARTIAL ctor) -- reachability evidence
//  ============================================================================
//   TfrmDTME08()            :16-28, ACTIVE-PARTIAL.  Delivered body is EMPTY;
//        golden's whole `if(USE_16_HEATER==eht16HeaterDTME08 || ...)` block
//        is GATE (E-1).  Two independent reasons: its five callees are all
//        gated (E-2/E-6/E-3, plus InitialData/InitialComboList which ARE
//        active but must not run here), and this facade is created by a
//        file-scope `new`, so the ctor runs BEFORE `main()` -- docs/
//        KNOWLEDGE.md's hard rule is "facade 的 ctor 只准塞欄位", the
//        fLaserSensor precedent having turned 88 of 134 ctest binaries into
//        SEGFAULTs and `TfMonitor`'s ctor having read the file MVData.ini
//        under D:/HT9045/system/ before main().  `Reload()` here would read
//        DTME08_Control.ini at static-init time -- exactly that shape.
//   InitialData()           :54-72 (19L) -- own fields + `USE_16_HEATER` /
//        `eht16HeaterDTME08` / `eht32HeaterDTME08` (cmydef.cpp +
//        MachineType.h:661-662, ht9045_globals) + `TimerUpdate->Enabled`
//        (own stand-in, DEVIATION D-5) + own `ClearAllCommandStatus()`.
//   ShowMSG(AnsiString)     :88-98 (11L) -- own `brecordmsgLock` +
//        `meLogs->Lines->Count/Clear/Add` (vclcompat::TMemo::Lines is a real
//        TStringList, vclcompat/Controls.h:369).
//   RecordMsg(AnsiString)   :100-104 -- `FormatDateTime(...)`/`Now()`
//        (vclcompat/TDateTime.h:56/:67) + own ShowMSG.
//   RecordErrorMsg(AnsiString) :106-110 -- own field + own RecordMsg.
//   AddTempPanel(TMyOmronPanel*) :131-134 -- `myPalGroup[GetSize()]=tmop;`,
//        a pure std::map insert.  Needs TMyOmronPanel DECLARED, not linked.
//   GetPalGroup(int)        :136-140 -- index clamp + `myPalGroup[index]`.
//        PRE-COMMITTED (bthermo.cpp:5398).  See ZERO-WRITER below.
//   InitialComboList()      :147-155 -- `cbCh->Clear()/Items->Add()/
//        ItemIndex` (vclcompat::TComboBox, Controls.h:386-394) + `IntToStr`
//        (vclcompat/SysUtils.h:23).
//   GetErrorMsg()           :178-191 (14L) -- own `asErrorDTME08` +
//        `DTMTimeout.Off()`.  `TQPF_Timer` is myTimer.h/.cpp =
//        **ht9045_globals**, which this facade links -- one of only two
//        non-obvious REACHABLE results in this file.
//   btSetThermoTypeClick    :272-275 -- one own-field assignment.
//   FormClose               :330-334 -- `bShow=false;` (DEVIATION D-3).
//   ClearAllCommandStatus() :443-450 -- own fields only.
//        ⚠ Golden's own oddity, kept verbatim: the `if(bCycleFirstRun==false)`
//        at :445 guards ONLY the next line (no braces), so the other three
//        assignments run unconditionally.  That reads like a missing brace.
//        NOT changed -- 改行為要留給使用者決定.
//   IsNeedSetSV()           :462-472 (11L) -- `GetPalGroup(i)->GetSettingSV()`
//        and `->GetSV()`, BOTH INLINE in EJ1N/MyOmronPanel.h.
//   btATClick               :678-685 -- `GetPalGroup(i)->SetAT(true)`, the
//        INLINE bool overload (contrast GATE E-24).
//   btSetSVClick            :687-690 -- one own-field assignment.
//   btAT1by1Click           :692-695 -- one own-field assignment.
//        ⚠ Golden's body is IDENTICAL to btATOffClick's tail and to
//        btATClick's tail (`iDoManualCommandStatus=DoSetAT;`) -- the "1 by 1"
//        in the name has no distinct behaviour in golden.  Verbatim.
//   btATOffClick            :697-704 -- `SetAT(false)`, inline overload.
//   btnSetSensorClick       :706-709 -- one own-field assignment.
//        ⚠ It writes `iDoAutoCommandStatus`, whereas the visually-parallel
//        btSetThermoTypeClick (:272-275) writes `iDoManualCommandStatus`.
//        Golden's asymmetry, kept.
//   btnCtrlClick            :711-714 -- `bCtrlSwitch=!bCtrlSwitch;`.
//   btnSetSVFollowCh1Click  :716-723 -- `GetPalGroup(i)->edSV->Text=
//        GetPalGroup(0)->edSV->Text;` -- `edSV` is a public
//        `vclcompat::TEdit*` FIELD of TMyOmronPanel and `Text` is a plain
//        AnsiString member, so no ht9045_comms symbol is referenced.
//   edPortClick             :730-733 -- `fQwertyKey->ShowQwertyKey((TEdit*)
//        Sender, N_PORT)` (forms/fQwertyKey.h:370/:406, ht9045_forms;
//        `N_PORT` cmydef.h:296).  Same shape as forms/fHotPlate.cpp:109.
//   edSVClick               :735-738 -- ditto with `N_DOUBLE` and `dTempMax`
//        (cmydef.cpp:3649).  ⚠ Golden passes `dTempMax` as the MIN argument
//        and `0.0` as the MAX (`ShowQwertyKey(..., true, dTempMax, 0.0)`),
//        i.e. the range looks inverted against fQwertyKey's own
//        `(min, max)` parameter order.  Transcribed verbatim and flagged.
//   SetPanelPos()           :740-764 (25L) -- `ScrollBox1->Width`, own
//        stand-in (DEVIATION D-4); `GetPalGroup(iMem)->GroupBox->Left/Top`,
//        a public `vclcompat::TGroupBox*` FIELD (Left/Top on
//        vclcompat::TControl); `ChangeToFloatNonPcnt`, a HEADER TEMPLATE
//        (MachineType.h:1601) so it needs no link edge at all.
//
//  ============================================================================
//  DEVIATIONS (port-only; each with why)
//  ============================================================================
//  (D-1) `__fastcall` dropped from every signature -- tree-wide convention.
//  (D-2) `TfrmDTME08` has NO base class (golden: `: public TForm`).  Matches
//      forms/fCleaning.h / forms/fHotPlate.h (D-2) / forms/fPrecaution.h
//      (D-4).  Consequence: the GATED AddSingleView cannot pass `this` as a
//      `TComponent*` owner -- recorded in GATE (E-7).
//  (D-3) `FormClose(TObject*, TCloseAction&)` -> `void FormClose();`.
//      `TCloseAction` has no port; golden's body reads neither parameter.
//      Precedent forms/fMesSystem.h D-5 / forms/fHotPlate.h (:281).
//  (D-4) `TScrollBox` has no port.  `TfrmDTME08ScrollBox` is a facade-local,
//      distinctly-named stand-in carrying the one property golden touches,
//      `Width`.  Distinctly named rather than `TScrollBox` for the same
//      reason forms/fTemperFrom.h:170-183 gives for `TfTemperFromTimer`
//      (this tree has already paid for a two-headers-one-classname collision:
//      docs/KNOWLEDGE.md, the two `TMyKitSuck` layouts).  Existing precedent
//      for the substitution itself: ATC/ATCInterface.h:313/:358 stands a
//      `TPanel` in for golden `TScrollBox` -- a `TPanel` is NOT reused here
//      because vclcompat has no `Width` on any widget.
//      ⚠ VALUE PROVENANCE (docs/KNOWLEDGE.md "解 gate 前先查值從哪來"):
//      `Width` is initialised to **859**, the design-time value recorded in
//      tools/dfm2rc/layout_out/EJ1N/fDTME08_layout.gen.cpp:64 for
//      `frmDTME08.Panel1.GroupBox2.ScrollBox1`.  This is deliberate, and it
//      matters: golden computes `numOfCol = ScrollBox1->Width/210`, so a
//      0-default would give `numOfCol==0` and turn golden's `i%numOfCol`
//      into a division by zero.  859/210 == 4, which is exactly the 4-column
//      layout golden's own `iPanelSort[32]` table is written for.  (As
//      DELIVERED the loop body never executes -- see ZERO-WRITER -- so the
//      UB is unreachable today either way; the value is set so that it stays
//      unreachable when the map is eventually populated.)  Nothing in this
//      port LOADS a layout file; this is a hand-carried constant with its
//      citation, not a hydration path.
//  (D-5) `TTimer` has no vclcompat port -- and SEVEN unrelated, mutually
//      incompatible `class TTimer` definitions exist in this tree (counted
//      20260828 with `rg "^\s*class\s+TTimer\b" -g '*.h' -g '*.cpp'`):
//      ATC/ATCInterface.h:189, BinDisplay/MyBinDisp.h:231,
//      Automation/uRENESAS_Server.h:168, forms/fGroundMan.h:456,
//      OmronLaser/LaserSensor.h:174, forms/fMonitor.h:221,
//      PowerSavingMode.h:75 -- so reusing the bare name would risk a
//      multi-definition collision.  `TfrmDTME08Timer` is a facade-local
//      one-field stand-in -- `->Enabled` is the only member any delivered
//      body touches (golden :59).  Same idiom as forms/fTemperFrom.h:247-251
//      and forms/fMotorTest.h:468-474.
//  (D-6) golden `typedef bool (__closure *TPointVoidDoCommand)(bool,int);`
//      -- `__closure` is a Borland extension (a method-pointer + object pair)
//      with no port.  Declared here as a PLAIN function pointer of the same
//      signature shape, purely so the GATED `SetSingleCommand`'s parameter
//      type and the GATED `tpvDoCommand` field can be spelled with golden's
//      own names.  ⚠ It is NOT equivalent: golden assigns `dtme08->DoGetPV`
//      (a member of another object) to it, which a plain function pointer
//      cannot hold.  A future un-gate must replace this with
//      `std::function<bool(bool,int)>` or a bound member pointer.
//  (D-7) `TMyLed` (golden `ledEJ1N1`) is NOT declared -- only the GATED
//      DoDTME08Cycle touches `ledEJ1N1`, so no stand-in is needed and none is
//      invented.  Counted 20260828 with `rg "^\s*class\s+TMyLed\b"`: exactly
//      ONE definition tree-wide, ATC/ATCInterface.h:208, and it is not in
//      vclcompat.  (A looser `rg "\bTMyLed\b"` returns 35 files -- almost all
//      of them comments or the `TMyLedLane` prefix; do not read that count as
//      35 competing types.)  A future un-gate of (E-32) must supply a
//      `Value`-carrying stand-in, following DEVIATION (D-5)'s naming rule.
//  (D-8) golden's file-scope `bool bFirstTime;` (fDTME08.cpp:157) and
//      `const int iAutoCycleCount=10;` (:474) are FILE-SCOPE VARIABLES, not
//      members -- census counts 55 bodies + 0 file-scope, so they are outside
//      the denominator.  `iAutoCycleCount` is translated (it has internal
//      linkage as a namespace-scope `const`, so it cannot collide).
//      `bFirstTime` is deliberately NOT translated: it is DEAD in golden too
//      (zero references anywhere in golden's fDTME08.cpp) and a
//      namespace-scope `bool bFirstTime;` with external linkage would be a
//      standing multiple-definition hazard for any future wave -- the four
//      existing `bFirstTime`s in this port (atester.cpp:5651/:10336,
//      cShowBinSelect.cpp:488, csystem.cpp:17208) are all FUNCTION-LOCAL
//      `static`s, so there is no collision today, but there is nothing to
//      gain by creating the first global one.
//  (D-9) golden declares `bool DoGetAllPV_DTME08();` at EJ1N/fDTME08.h:192
//      and DEFINES IT NOWHERE.  Verified 20260828 against the WHOLE golden
//      tree: `rg -l "DoGetAllPV_DTME08" HT9011UC_Code_V3.33.906.0_20260618/`
//      -> exactly one file, EJ1N/fDTME08.h, one hit.  It is an orphan
//      declaration in GOLDEN ITSELF, not something this port lost.  Declared
//      here for shape-fidelity, with no definition -- which is what golden
//      does.
//  (D-10) golden's other 40-odd `__published` widgets (Panel1..Panel11,
//      GroupBox/GroupBox1/GroupBox2/GroupBox3, PageControl1, tsAuto/tsManual/
//      tsSimple/tsRecord, edSV, Memo1, btGetModuler/btRunStop/btSetThermoType/
//      Button1/btSetSV/btAT/btATOff/btAT1by1/btSaveData/btnSetSave/btnReload/
//      btnCtrl/btnSetSVFollowCh1, rgSensorType/rgRunStop, cbEnableAT/
//      cbGetRecord/cbSimulation/chkHeater, grpStatus/gbAT, ledEJ1N1,
//      Label25/45/46/47/48, edAddress/edPort) are NOT declared -- no ACTIVE
//      body and no pre-committed site touches them (minimal-declaration
//      idiom, matches forms/fHotPlate.h's own FIELD LIST practice).
//
//  ============================================================================
//  FIELD LIST (4 widgets + 2 timers + 1 map + 22 scalars)
//  ============================================================================
//    bShow (bool)                      -- InitialData / FormClose
//    meLogs (TMemo*)                   -- ShowMSG
//    cbCh (TComboBox*)                 -- InitialComboList / GetChannelIndex
//    ScrollBox1 (TfrmDTME08ScrollBox*) -- SetPanelPos (->Width), DEVIATION D-4
//    TimerUpdate (TfrmDTME08Timer*)    -- InitialData (->Enabled), D-5
//    DTMTimeout / DTMReconnectTime (TQPF_Timer) -- GetErrorMsg (Off())
//    myPalGroup (std::map<int,TMyOmronPanel*>) -- AddTempPanel / GetPalGroup
//    asErrorDTME08 (AnsiString)        -- GetErrorMsg / RecordErrorMsg
//    the 13 golden ints + 4 golden bools -- InitialData /
//      ClearAllCommandStatus / the eight own-field-only click handlers
//    dtme08 (uDTME08Control*, forward-declared) / tpvDoCommand -- GATED only,
//      declared so the transcripts and the golden shape stay spellable
//  NOT declared: see DEVIATION (D-7)/(D-10).
//
//  ============================================================================
//  ZERO-WRITER FIELDS -- READ THIS BEFORE UN-GATING ANYTHING
//  ============================================================================
//  `myPalGroup` is populated ONLY by `AddTempPanel`, which is called ONLY by
//  `AddSingleView` (GATE E-7), which is called ONLY by `AddTempGUI`
//  (GATE E-3), which is called ONLY by the ctor's gated conditional
//  (GATE E-1).  So AS DELIVERED THE MAP IS ALWAYS EMPTY and `GetSize()` is
//  always 0.  Consequences, all deliberate and all safe today:
//    * every ACTIVE body that loops `for(int i=0;i<GetSize();i++)`
//      (IsNeedSetSV, btATClick, btATOffClick, btnSetSVFollowCh1Click,
//      SetPanelPos) executes ZERO iterations -- no null dereference, and
//      SetPanelPos's `i%numOfCol` is never evaluated.
//    * `IsNeedSetSV()` therefore always returns false.
//    * `GetPalGroup(index)` clamps to 0 and evaluates `myPalGroup[0]`, which
//      on an empty std::map INSERTS a value-initialised (null) entry and
//      returns it.  It returns nullptr, and it MUTATES the map by one entry.
//      That is golden's own operator[] semantics, faithfully carried; in
//      golden it never shows because the ctor fills the map first.  The one
//      caller that would dereference the result -- bthermo.cpp:5398's
//      `GetPalGroup(i)->GetPV()` -- is itself still `#if 0`.
//      ⚠ ANY WAVE THAT UN-GATES bthermo.cpp:5396-5399 MUST FIRST UN-GATE
//      (E-1)/(E-3)/(E-7), or it will dereference nullptr on the first
//      temperature poll.
//  `bShow` is written by ACTIVE InitialData (false) and FormClose (false) and
//  by the GATED FormShow (true) -- so it is permanently false today.  No
//  pre-committed site reads it (unlike the other three facades in this wave).
//
//  ============================================================================
//  DISCLOSURE (task rule 8 -- what was NOT read to completion / NOT verified)
//  ============================================================================
//  - EJ1N/uDTME08Control.{h,cpp} was NOT read as a whole.  Only (i) the
//    owning `add_library` (ht9045_comms) and (ii) its banner's own function
//    inventory were used.  Every gate that names `dtme08` rests on (i) alone,
//    which is sufficient; but a future ht9045_comms-reaching wave must
//    re-verify that `DoGetSV/DoGetPV/DoSetSV/DoSetAT/DoSetSensorType/
//    DoSetCycleTime/DoSetControlActionOut2/DoCommuncation/
//    InitialmapDTMInfoList/mapDTMInfo/GetMaxStationNumber/
//    GetChannelNumberPerStation/esstPT100/esstKType/ecaDisable` all exist
//    with golden's shapes -- that header carries its own GATE (1), so some of
//    them may themselves be gated.
//  - EJ1N/MyOmronPanel.h WAS read in full (its whole public section), because
//    the inline-vs-out-of-line split is what separates 6 ACTIVE bodies from 6
//    GATED ones.  MyOmronPanel.cpp was NOT read; the inline/out-of-line
//    classification comes from the header's declarations, which is the
//    linkage-relevant fact.
//  - `DoDTME08Cycle` (202L) was read in full for gate classification, but its
//    internal global surface beyond the disqualifying `dtme08`/`ledEJ1N1`
//    references was NOT individually traced for reachability.  Moot -- it is
//    gated regardless -- but a future un-gate cannot reuse this file's
//    reachability notes for it.
//  - golden `fDTME08.dfm` was NOT opened.  The ONE design-time value this
//    wave needed (ScrollBox1->Width) was taken from the port's own generated
//    layout table with a citation (DEVIATION D-4); nothing else is supplied,
//    so `TMyOmronPanel::iMyTag`-style design-time values are absent.
//  - `HeaterSVLog`'s gating was verified by locating the enclosing `#if 0`
//    (cpublic.cpp:709) and reading cpublic.cpp:694's own note; the function's
//    BODY (cpublic.cpp:711-716, `fMain->slHeaterLog->AddTextWithDateTime`)
//    was read but `slHeaterLog` was NOT chased -- immaterial, the enclosing
//    `#if 0` already settles it.
// =============================================================================
#ifndef FORMS_FDTME08_H
#define FORMS_FDTME08_H

#include <map>

#include "forms/FormWidgets.h"      // AnsiString / TObject / TMemo / TComboBox (vclcompat)
#include "myTimer.h"                // TQPF_Timer (myTimer.cpp, ht9045_globals)
#include "EJ1N/MyOmronPanel.h"      // TMyOmronPanel -- DECLARED here, never
                                    // re-translated: the real class is
                                    // EJ1N/MyOmronPanel.{h,cpp}.  Its INLINE
                                    // members are usable from ht9045_forms;
                                    // its out-of-line ones are ht9045_comms
                                    // (see the LINK BOUNDARY block above).

// golden EJ1N/fDTME08.h:15 `#include "uDTME08Control.h"` -- a forward
// declaration is enough here: `dtme08` is a POINTER member and every body
// that dereferences it is GATED (link boundary, ht9045_comms).  Declaring it
// this way keeps ht9045_forms free of a comms header it must not depend on.
class uDTME08Control;

// ---------------------------------------------------------------------------
//  TfrmDTME08ScrollBox -- stand-in for golden `TScrollBox *ScrollBox1`
//  (EJ1N/fDTME08.h:27).  See DEVIATION (D-4), including the VALUE PROVENANCE
//  note for why `Width` is 859 and not 0.
// ---------------------------------------------------------------------------
struct TfrmDTME08ScrollBox
{
    int Width = 859;   // .dfm design-time value, tools/dfm2rc/layout_out/
                       // EJ1N/fDTME08_layout.gen.cpp:64.  859/210 == 4
                       // columns, matching golden's iPanelSort[32] layout.
};

// ---------------------------------------------------------------------------
//  TfrmDTME08Timer -- stand-in for golden `TTimer *TimerUpdate`
//  (EJ1N/fDTME08.h:82).  See DEVIATION (D-5).  `->Enabled` is the only member
//  any delivered body touches.
// ---------------------------------------------------------------------------
struct TfrmDTME08Timer
{
    bool Enabled = false;   // InitialData golden :59 writes false
};

// ===========================================================================
//  TfrmDTME08 -- non-VCL facade (golden EJ1N/fDTME08.h:20-202,
//  `class TfrmDTME08 : public TForm`; NO base class here, DEVIATION D-2).
// ===========================================================================
class TfrmDTME08
{
    // golden :22.  DEVIATION (D-6): golden's `__closure` becomes a plain
    // function pointer -- same signature shape, NOT the same semantics.
    typedef bool (*TPointVoidDoCommand)(bool, int);

public:
    TfrmDTME08();

    // -- golden __published widgets touched by an ACTIVE body ----------------
    TMemo     *meLogs = new TMemo();                       // golden :35
    TComboBox *cbCh   = new TComboBox();                   // golden :81
    TfrmDTME08ScrollBox *ScrollBox1 = new TfrmDTME08ScrollBox();  // golden :27, DEVIATION D-4
    TfrmDTME08Timer     *TimerUpdate = new TfrmDTME08Timer();     // golden :82, DEVIATION D-5

    // -- golden public: User declarations ------------------------------------
    bool bShow = false;                                    // golden :172
    TPointVoidDoCommand tpvDoCommand = 0;                   // golden :179, DEVIATION D-6 (GATED use only)

    // -- ACTIVE this wave (defined in forms/fDTME08.cpp) ---------------------
    void InitialData();                                     // golden :54-72    ACTIVE
    void ShowMSG(AnsiString asStr);                         // golden :88-98    ACTIVE
    void RecordMsg(AnsiString asMsg);                       // golden :100-104  ACTIVE
    void RecordErrorMsg(AnsiString asMsg);                  // golden :106-110  ACTIVE
    TMyOmronPanel* GetPalGroup(int index);                  // golden :136-140  ACTIVE, PRE-COMMITTED (bthermo.cpp:5398)
    bool IsNeedSetSV();                                     // golden :462-472  ACTIVE
    void FormClose();                                       // golden :330-334  ACTIVE, DEVIATION D-3
    void btSetThermoTypeClick(TObject *Sender);             // golden :272-275  ACTIVE
    void btATClick(TObject *Sender);                        // golden :678-685  ACTIVE (SetAT(bool), inline)
    void btSetSVClick(TObject *Sender);                     // golden :687-690  ACTIVE
    void btAT1by1Click(TObject *Sender);                    // golden :692-695  ACTIVE
    void btATOffClick(TObject *Sender);                     // golden :697-704  ACTIVE (SetAT(bool), inline)
    void btnSetSensorClick(TObject *Sender);                // golden :706-709  ACTIVE
    void btnCtrlClick(TObject *Sender);                     // golden :711-714  ACTIVE
    void btnSetSVFollowCh1Click(TObject *Sender);           // golden :716-723  ACTIVE (edSV field only)
    void edPortClick(TObject *Sender);                      // golden :730-733  ACTIVE
    void edSVClick(TObject *Sender);                        // golden :735-738  ACTIVE

    // -- golden header-inline accessors (not census bodies; translated here
    //    exactly as golden declares them) -----------------------------------
    void SetChannelNumber(int iNum){ChannelNumber=iNum;}     // golden :185
    int  GetChannelNumber(){return ChannelNumber;}           // golden :186
    int  GetChannelIndex(){return cbCh->ItemIndex;}          // golden :187

    // -- GATED this wave -- declared here, golden body translated in
    //    forms/fDTME08.cpp inside `#if 0`.  Calling one is a LINK ERROR by
    //    design (see GATE REGISTER above). -----------------------------------
    void SaveData();                                        // golden :74-79    GATE (E-5)
    void Reload();                                          // golden :81-86    GATE (E-6)
    bool DoSendCommandByManual(bool bIsFirst, int iStation); // golden :229-250  GATE (E-15)
    void GetTargetCh(int ich);                              // golden :252-256  GATE (E-16)
    void SetSingleCommand(TPointVoidDoCommand tcdc, int ich=0);  // golden :258-264  GATE (E-17)
    void GetStationAndNumber(int iCh, int &iStation, int &iNumber);  // golden :266-270  GATE (E-18)
    bool DoManualProcess();                                 // golden :282-320  GATE (E-20)
    void SetSP(int iCh, double SV, AnsiString Event);       // golden :336-339  GATE (E-22)
    void SetPV(int iCh, double PV, AnsiString Event);       // golden :341-345  GATE (E-23)
    void SetATStatus(int iCh, bool bOn);                    // golden :347-350  GATE (E-24)
    void DoDTME08Cycle();                                   // golden :475-676  GATE (E-32)
    void InitialSetting(int iStation);                      // golden :419-441  GATE (E-30)
    void InitialDoSetAllAT(int iStation);                   // golden :371-380  GATE (E-26)
    void InitialDoSetAllSV(int iStation);                   // golden :382-395  GATE (E-27)
    void InitialDoSetSensorType(int iStation);              // golden :352-369  GATE (E-25)
    void InitialDoSetCycleTime(int iStation);               // golden :397-406  GATE (E-28)
    void InitialDoSetControlActionOut2(int iStation);       // golden :408-417  GATE (E-29)
    void SetSettingSV(int iCh, double dTemp);               // golden :452-460  GATE (E-31), PRE-COMMITTED (bthermo.cpp:5397)
    void btnSetSaveClick(TObject *Sender);                  // golden :142-145  GATE (E-8)
    void TimerUpdateTimer(TObject *Sender);                 // golden :158-176  GATE (E-9)
    void btnGetPVClick(TObject *Sender);                    // golden :193-197  GATE (E-10)
    void btnSetATClick(TObject *Sender);                    // golden :199-205  GATE (E-11)
    void btnGetSVClick(TObject *Sender);                    // golden :207-211  GATE (E-12)
    void btnSetSVClick(TObject *Sender);                    // golden :213-221  GATE (E-13)
    void btnGetStatusClick(TObject *Sender);                // golden :223-227  GATE (E-14)
    void btnReloadClick(TObject *Sender);                   // golden :277-280  GATE (E-19)
    void FormShow(TObject *Sender);                         // golden :322-328  GATE (E-21)
    void chkHeaterClick(TObject *Sender);                   // golden :725-728  GATE (E-33), SAFETY: heater relay

    // -- DECLARED BY GOLDEN, DEFINED BY NOBODY -- see DEVIATION (D-9).
    //    golden EJ1N/fDTME08.h:192; zero definitions in the whole golden tree.
    bool DoGetAllPV_DTME08();

private:
    // golden :111-122
    enum eDoCommand
    {
        DoNone=0,
        DoGetSV,
        DoGetPV,
        DoSetSV,
        DoSetAT,
        DoGetStatus,
        DoSetSeonsorType,
        DoSetCycleTime,
        DoSetControlActionOut2,
    };
    // golden :123-131
    enum eDoCommandInitial
    {
        DoInitialNone=0,
        DoInitialSetTypeSeonsor,
        DoInitialSetCycleTime,
        DoInitialSetControlActionOut2,
        DoInitialSetSV,
        DoInitialTotal,
    };
    int iDoInitialCommandStatus = 0;                        // golden :132
    int iDoCommandStatus        = 0;                        // golden :133
    int iDoManualCommandStatus  = 0;                        // golden :134
    int iDoAutoCommandStatus    = 0;                        // golden :135
    int iNowActiveStation       = 0;                        // golden :136
    void ClearAllCommandStatus();                           // golden :443-450  ACTIVE

    int iTaskCommand            = 0;                        // golden :139
    uDTME08Control *dtme08      = 0;                        // golden :140 (GATED use only)
    bool brecordmsgLock         = false;                    // golden :141
    void NewuDTME08Control();                               // golden :30-36    GATE (E-2)
    void AddTempGUI();                                      // golden :38-44    GATE (E-3)
    void DrawTempGUI();                                     // golden :46-52    GATE (E-4)
    TQPF_Timer DTMTimeout;                                  // golden :145
    TQPF_Timer DTMReconnectTime;                            // golden :146
    void AddSingleView(TfrmDTME08ScrollBox *ScrollBox1);    // golden :112-129  GATE (E-7), DEVIATION D-4
    //myPalGroup
    std::map<int,TMyOmronPanel*> myPalGroup;                // golden :149
    void AddTempPanel(TMyOmronPanel* tmop);                 // golden :131-134  ACTIVE
    int GetSize(){return myPalGroup.size();}                // golden :151, verbatim (golden's own size_t->int narrowing)
    //
    void InitialComboList();                                // golden :147-155  ACTIVE
    bool bTimerUpdateLock       = false;                    // golden :154
    AnsiString asErrorDTME08;                               // golden :155
    bool GetErrorMsg();                                     // golden :178-191  ACTIVE
    int TargetCh                = 0;                        // golden :157
    int TargetStation           = 0;                        // golden :158
    int TargetNo                = 0;                        // golden :159
    int ChannelNumber           = 0;                        // golden :160
    int iSingleStationTask      = 0;                        // golden :161
    int iTaskDTME08Cycle        = 0;                        // golden :162
    int iCycleRunNumber         = 0;                        // golden :163
    int GetDTMTimeout(){return 2;}                          // golden :164
    int GetCycleTime(){return 2;}                           // golden :165
    int GetReconnectTime(){return 10;}                      // golden :166
    bool bCycleFirstRun         = false;                    // golden :167
    bool bCtrlSwitch            = false;                    // golden :168
    void SetPanelPos();                                     // golden :740-764  ACTIVE, Jimmychiu 20230628 : DTME08物件排列
};

// AI(W906-FW3-DTL1) 20260828: TfrmDTME08/frmDTME08 were FREE tree-wide (see
// banner's measured provenance) -- declared here so the 2 pre-committed
// `#if 0` sites (bthermo.cpp:5397 and :5398, inside that file's own
// `#if 0 // TODO(W7-UI G29)`) have something real to bind to once bthermo's
// OWN gate is separately lifted (NOT done by this wave).
// ⚠ Read the ZERO-WRITER section before lifting it: GetPalGroup returns
// nullptr for as long as GATE (E-1)/(E-3)/(E-7) stand.
extern TfrmDTME08 *frmDTME08;

#endif // FORMS_FDTME08_H
