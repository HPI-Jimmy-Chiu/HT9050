// =============================================================================
//  forms/fTemp_Set.h  --  non-VCL facade for golden's TfTemp_Set
//  (uTemp_Set.h / uTemp_Set.cpp -- "Handler 溫度設定" dialog)
//
//  AI(W906-FW3-TempSet-WA) 20260820: new file, FW-3 batch 3 continuation,
//  uTemp_Set Wave A. Golden ref: HT9011UC_Code_V3.33.906.0_20260618/
//  uTemp_Set.h (1,077 lines) + uTemp_Set.cpp (6,970 lines, cp950 -- decoded
//  with `python3 -c "open(...,'rb').read().decode('cp950')"`, 0 U+FFFD, this
//  wave). Full golden .cpp read start-to-finish this wave (6,970/6,970 lines,
//  100%) before any line of this header was written.
//
//  ROLE
//  ----
//  TfTemp_Set is golden's temperature-setting dialog: one TMyTempPanel row
//  (see MyTempPanel.h, already ported PT-W3) per heat channel, plus ~700
//  individually-named widgets across 20 tab pages (Hot/Ambient/Other/ATC/
//  ATC70/ATC_PID/HeatGun/DUT/Arm1/Arm2/BoostFunc/LBCtr/ATC_FFC/
//  ATCMultiSensor/TriTempSet/...) covering: base-point selection, soak-time/
//  initial-delay timers, ATC offset tables (32-channel + package + FFC +
//  multi-sensor), Boost/LB-temp function, TJ mapping, and the Tri-Temp
//  (HT-1032) defrost sub-system. This wave translates the WHOLE class:
//  every golden method except the two genuinely-unspellable-signature/
//  unported-dependency cases documented below (FormShortCut, ATC_Power/
//  CheckAirMachineStatus have no golden body to translate in the first
//  place -- see GATE REGISTER).
//
//  THIS IS A NEW CLASS -- NO EXISTING SHIM
//  --------------------------------------------------------------------------
//  Grepped tree-wide before writing a line: `class\s+TfTemp_Set\b` and
//  `\bfTemp_Set\s*[;=]` -- 0 hits for a real (non-`#if 0`, non-comment)
//  declaration of either, EXCEPT one narrow, differently-named, differently-
//  shaped TU-local stand-in the coordinator flagged mid-wave:
//  Automation/auto9045.cpp:399-404 declares `W5FA_TfTemp_SetExt
//  W5FA_FTemp_Set` (its own scoped name, no ODR contact with the real
//  `TfTemp_Set`/`fTemp_Set` declared/defined here). No collision.
//  Two ALREADY-GATED consumers exist and are cited here only so a future
//  integration wave can find them (this wave does not touch either file):
//    * TempCtrl/uHeaterThread.cpp GATE 6 (four `#if 0` call sites,
//      `fTemp_Set->ControlATC60AirFlow(...)`).
//    * cprod.cpp:3989-4003 (same function, same `#if 0` reason).
//  Both need `ControlATC60AirFlow(int iStatus=-1)` to stay a PUBLIC member
//  with the golden default-argument signature so a later un-gate wave can
//  call it directly -- preserved verbatim below.
//
//  WAVE SCOPE -- every golden method, golden line span, this file's status
//  --------------------------------------------------------------------------
//  See the parallel table in uTemp_Set.cpp's own banner (kept there, not
//  duplicated in full here, to avoid the two drifting) -- summary of the
//  handful of DEVIATIONs/GATEs that shape the declarations below:
//
//   * ctor (golden :95-408) -- per this wave's brief, the ctor body carries
//     NO logic; every line of golden's ctor (asTempCtrl[] seeding,
//     myTempPal[]/listNormal/listArm1/listArm2 population, SetParent/
//     SetIndexTag wiring, ATCOffsetEdit[]/ATCPackageOffsetEdit[]/
//     ATC_FFCOffset*Edit[][]/ZoneTempUse[]/MultiSensorOffsetUse[]/
//     ATC_MultiSensorOffsetEdit[] global-array seeding) moves to an explicit
//     `Init()` method declared below, for a later wave to call once. The
//     ctor itself is empty (widget members are NSDMI'd inline below, exactly
//     like every other translated form facade in this tree).
//   * `TfTemp_Set *fTemp_Set;` (golden :47) -- declared at GLOBAL scope,
//     ZERO-initialized (`TfTemp_Set *fTemp_Set;`, NOT `= new TfTemp_Set()`)
//     per this tree's static-init-order rule; defined in uTemp_Set.cpp.
//   * DEVIATION -- unspellable-parameter signatures simplified to
//     `(TObject *Sender)` only, matching this tree's established
//     FormClose-drops-TCloseAction idiom (cTemperFrom.h, fCounterClear.h,
//     OmronLaser/LaserSensor.h, ATC/ATCInterface.h, SECSGEM/uHGemEquipment.h
//     all cite the identical rationale): golden's `TMouseButton`/
//     `TShiftState` have ZERO port anywhere in this tree (re-confirmed this
//     wave, `grep -rn "class TMouseButton\|class TShiftState"` over
//     forms/*.h and the tree root, 20260820 -- 0 hits; MyTempPanel.h's own
//     GATE (1) and fSetup.h's FormShortCut note already established this
//     independently). Every `...MouseDown(TObject*, TMouseButton, TShiftState,
//     int X, int Y)` and the one `...KeyUp(TObject*, WORD&, TShiftState)`
//     below drop the unspellable middle parameter(s); `sgTjMapMouseDown`
//     keeps `int X, int Y` (spellable, and genuinely read by its body via
//     `MouseToCell`, itself gated separately -- see uTemp_Set.cpp).
//     `edSoakTimeKeyPress(TObject*, char&Key)` is fully spellable (no
//     unspellable type) and keeps its exact golden signature.
//   * DEVIATION -- `FormClose(TObject *Sender, TCloseAction &Action)`
//     collapses to `FormClose(TObject *Sender)`, same established idiom
//     (`TCloseAction` has no port anywhere in this tree).
//   * OMITTED ENTIRELY -- `FormShortCut(TWMKey &Msg, bool &Handled)`.
//     `TWMKey` has ZERO port anywhere in this tree (`grep -rn "TWMKey"`,
//     20260820: only hit outside this file is forms/fSetup.h's own,
//     already-established note on this EXACT golden method shape) --
//     AND, per that note's own distinction, this is NOT a droppable-unused-
//     parameter case: golden's body genuinely reads `Msg.CharCode` (compared
//     against `VK_F5`), so there is no way to keep the body faithful without
//     the real type. Declaration and definition both omitted, matching
//     fSetup.h's own posture for the identical golden method shape.
//   * OMITTED ENTIRELY (declared, never defined) -- `ATC_Power()` and
//     `CheckAirMachineStatus()` (golden uTemp_Set.h :1063/:1065). Both are
//     declared in golden's OWN header but have NO definition anywhere in
//     golden's uTemp_Set.cpp (`grep -c "TfTemp_Set::ATC_Power\|
//     TfTemp_Set::CheckAirMachineStatus" uTemp_Set.cpp`, 20260820: 0 hits for
//     either) AND no call site anywhere in the golden 906 tree
//     (`grep -rn "fTemp_Set->ATC_Power\|fTemp_Set->CheckAirMachineStatus"`,
//     20260820: 0 hits). `CheckAirMachineStatus` is NOT a typo for the
//     same-named but unrelated `TfLotInfo::CheckAirMachineStatus`
//     (golden uLotInfo.cpp:14805, a completely different class's method) --
//     verified both are genuinely separate declarations. Declared below
//     (matching golden's own header) with NO body in uTemp_Set.cpp,
//     reproducing golden's own "declared but dead" state exactly rather than
//     inventing a plausible-looking implementation for a function nothing
//     ever calls.
//
//  GATE REGISTER (full rationale + golden line numbers in uTemp_Set.cpp's
//  own banner and inline at each call site; summarised here per class of
//  gate so the pattern is legible from this header alone)
//  --------------------------------------------------------------------------
//   (G-Qwerty) `fQwertyKey->ShowQwertyKey(...)` -- ALREADY AN ESTABLISHED
//      tree-wide finding, cited verbatim by ATC/ATCInterface.h, forms/
//      fConfiguration.h (its own "WA-1"), forms/fLotInfo.h, Public/
//      HTEdit.cpp GATE (6): `class TfQwertyKey`/a real `fQwertyKey` global
//      do not exist anywhere in this tree (re-confirmed this wave,
//      `grep -rn "class TfQwertyKey\|fQwertyKey\s*="`, 20260820, 0 hits for
//      either outside comments). This is BY FAR the single most common gate
//      in this file -- essentially every small MouseDown/Click handler whose
//      entire golden body is "pop the numeric keypad" reduces to this one
//      `#if 0` line at its call site; the handler's OTHER real logic (guard
//      returns, AccessLevel checks, dTempMax/dTempMin recompute, Tag-indexed
//      branch selection) stays ACTIVE.
//      OPENED 20260824 (FW-QWKEY2): FW-QWKEY1 (fc08e09) landed forms/
//      fQwertyKey.{h,cpp} + myQwertyKeyBoard.cpp, so the absence-claim
//      above EXPIRED; all 50 G-Qwerty sites in uTemp_Set.cpp are un-gated
//      and live (latent: runtime instance NULL until HTEdit GATE (6)
//      wiring). G-Barcode / G-Image remain CLOSED -- their absences hold.
//   (G-Barcode) `Barcode_Reader(bcTemperature)` -- ALREADY AN ESTABLISHED
//      tree-wide finding (forms/fSetup.h's own note on the identical golden
//      idiom): ⚠ AI(W906-FW-SETUP-W19) 20260826 更正——Barcode_Reader 的真本體
//      在 BarcodeReader.cpp:445，這條 absence claim 已過期。原文保留為沿革：
//      `Barcode_Reader` has ZERO port anywhere in this tree
//      (re-confirmed, `grep -rn "Barcode_Reader\("`, 20260820 -- the only
//      hits outside golden/docs are the unrelated `ProcessLastSetIni_
//      Barcode_Reader` name and BarcodeReader.cpp's own class, neither is
//      this free function). Gates the `if(Buffer->Tag==1){ if(Barcode_
//      Reader(...)==0) return; }` guard block wherever it appears; the
//      qwerty-popup line right after it is separately gated under G-Qwerty.
//   (G-Image) `Image1`/`Image2` (golden `TImage*`) -- ALREADY AN ESTABLISHED
//      tree-wide finding (forms/fLotInfo.h WB-1, EJ1N/MyOmronPanel.h:93,
//      VacuumUnit/MyVacuumPanel.h:58): `class TImage` has ZERO port anywhere
//      in this tree. Both members OMITTED from this header entirely (not
//      even declared); `SetBasePointIMG` (whose ENTIRE body is an
//      `Image1->Picture->LoadFromFile` call) and the one `Image2->Visible=
//      false;` line in `UpDateEdit` are gated at their call sites.
//   (G-FTestIF) `FTestIF->ReadTestIFFile()` -- ALREADY AN ESTABLISHED
//      tree-wide finding (Automation/SCK_ART_Remainder.h gate #2, Command.cpp
//      gates #11/#12): golden `TFTestIF` (cTesterIF.h) is a whole
//      untranslated VCL form with no facade anywhere in this port. Gates the
//      2 call sites in `DoIniDataToForm`/`SaveSetupFile`.
//   (G-Align) golden `->Align=alTop/alBottom` (TAlign) and `->BringToFront()`
//      -- pure Z-order/paint properties; `vclcompat::TPanel`/`TControl`
//      carry neither (by design -- this tree's established "logic faithful,
//      rendering = web" split, same class of omission as TPageControl/
//      TTabSheet carrying no geometry). Gates `UpDateEdit`'s `->Align=...`
//      assignments and the whole of `ShowLineOnTop` (which is ONLY
//      `->BringToFront()` calls end to end).
//   (G-RGColor) `rgBasePoint->Color=...` (2 sites, `UpDateEdit`) and
//      `rgIndexHeatMode->Controls[i]->Visible` (1 site, `ReadTempFile`) --
//      `vclcompat::TRadioGroup` (vclcompat/Controls.h) carries only
//      `ItemIndex`/`Items`, no `Color` and no `Controls[]` child-widget
//      accessor (verified against that header directly this wave). Gated at
//      each of the 3 call sites.
//   (G-Grid) `sgTjMap->MouseToCell(...)` -- `vclcompat::TStringGrid`
//      (vclcompat/StringGrid.h) explicitly scopes OUT mouse/paint surface
//      per that header's own SCOPE note ("STILL NO rendering... NO
//      MouseToCell"). Gates the whole of `sgTjMapMouseDown` (its only
//      payload).
//   (G-Delegate) `fLotInfo->pl_ATC_Online->OnClick(fLotInfo);` (in
//      `ATC_Online_Or_offline`) -- golden invokes a VCL `TNotifyEvent`
//      delegate directly as a function call, a BCB6-only idiom; no
//      vclcompat stock-widget stand-in carries an assignable/invokable
//      `OnClick` delegate slot (only the no-op virtual `TControl::Click()`
//      exists). This is a DEPENDENCY gap, ORTHOGONAL to the task's SAFETY
//      classification below -- `ATC_Online_Or_offline` is on the "translate
//      plainly" list because its OWN logic has no safety concern; the gate
//      here exists purely because the call target cannot be spelled, not
//      because the call is unsafe to make.
//
//  ADDITIONAL DEPENDENCY GATES found ONLY during the mandatory `-fsyntax-only`
//  pass (i.e. NOT anticipated when the ABOVE gate list was first drafted --
//  recorded here for the same reason G-Delegate/G-Grid/etc. are: so a reader
//  does not have to re-derive them from a diff). All orthogonal to SAFETY --
//  each is "the call target cannot be spelled", not "the call is unsafe":
//   (dep-Tag) golden `TComponent->Tag` has no port on bare vclcompat
//      TEdit/TSpeedButton/TRadioGroup (verified against vclcompat/Controls.h
//      directly). Two NEW facade-local wrapper types with a plain `int Tag`
//      member were added (`TfTemp_SetTagEdit`/`TfTemp_SetTagButton`, same
//      "compose, don't fork" idiom as TfTemp_SetTimer above) for the 3
//      widgets where Tag is genuine business state (btnSort's sort-mode
//      counter; edt_SetIndexAirstreamTemp/edtSetTempature2AirMachine's
//      Tag-indexed dispatch in edtSetTempature2AirMachineClick). Every OTHER
//      `->Tag` read in this file is a KYEC-barcode-guard check whose own
//      payload (Barcode_Reader) is ALREADY G-Barcode-gated, so those 9 sites
//      (edSoakTimeClick/edJamSoakTimeClick/edLHP1MouseDown/
//      edLowBaseMouseDown/edAmbHotGuartbentClick/edtATCInPC1MouseDown/
//      edATCAmbTempMouseDown/edATCInitialOffset1MouseDown/
//      edtHeatGunTempATCMouseDown) and rb1PointClick's `Ptr->Tag` (whose
//      only consumer, SetBasePointIMG, is itself already fully G-Image-gated)
//      fold the `if(Buffer->Tag==1)`/`Ptr->Tag` guard ITSELF into the
//      existing gate rather than adding a 3rd wrapper type for dead reads.
//   (dep-SetFocus) `TRadioButton::SetFocus()` -- no port (pure keyboard-focus
//      chrome). Gates the single `rbTemp->SetFocus();` in FormShow.
//   (dep-fMain-edATCAmbientTemper) `fMain->edATCAmbientTemper` -- no member
//      on the current fMain facade (grepped, 0 hits; fMain.h outside this
//      wave's write boundary). Gates 1 line in spbSaveClick's validation
//      section (dbSetTemp keeps its `=0` default on that branch).
//   (dep-fLotInfo-SetATCOffset) `fLotInfo->SetATCOffset` -- no member on the
//      current fLotInfo facade (only the unrelated `fTempATCOffset[]` data
//      array exists). Gates 1 line in ReadRemoteTempOffset.
//   (dep-ATC-IsConnect) `ATC_InterfaceForm->IsConnect()` -- acarry_shims.h's
//      `TATC_InterfaceFormShim` is scoped to exactly `iATC_MODE_TYPE`.
//      Gates a block in tmr_ATC_DeforstTimer.
//   (dep-ATC-asATC_SW_Ver) same shim-scope gap, different member. Whole body
//      of CheckVerSupportFunction gated; returns golden's own "no version
//      string" default (`iReturnStatus=1`) instead of a value that would
//      imply a real ATC firmware read happened.
//   (dep-TriTemp_Ch) golden `extern int TriTemp_Ch[ATC_MAX_SITE]`
//      (ATC/ATC_Handler_Side.h:497) does not exist anywhere in this port tree
//      yet (MainCalcCore.h's own 20260728 recon note says so explicitly).
//      Folded into SAFETY GATE (S9) below -- SetSingleWorkTemperature's
//      `bUse` lookup needs it, so there is no "safe half" left un-gated;
//      S9 is now a WHOLE-BODY gate, not the originally-planned partial one.
//   (dep-COM2-ATCInitialTask) golden `TCOM2Shim` (atester_shims.h) is scoped
//      to exactly what the W5 tester-comm engine derefs; ATCInitialTask is
//      not among them. Gates 1 line in ReadTempFile's tail.
//   (dep-FormHS) `FormHS`/`CheckTempOffset` are golden `HS_Function.h`/`.cpp`
//      -- NOT forms/fHandlerSys.h's `THandlerSystem` (a same-sounding but
//      UNRELATED golden class: "Handler System Setup" vs golden HS_Function's
//      own "Handler Self-test" helper; confirmed directly against golden,
//      where both names resolve only inside HS_Function.h/.cpp). Neither
//      HS_Function nor a `FormHS` global exists anywhere in this port tree.
//      Gates the temperature-offset-limit validation block in spbSaveClick
//      (bHasErrorSet keeps its `=false` default while gated).
//   (dep-TabSheet-PageIndex) `TTabSheet::TabIndex`/`PageIndex` and
//      `TPageControl::ActivePage` -- none exist on vclcompat::TTabSheet/
//      TPageControl (that type carries only Caption/TabVisible and
//      ActivePageIndex respectively). Whole body of pgcTempOffsetChange
//      gated -- golden's own .dfm tab ORDER (which index tsATC/tsArm1/tsArm2
//      occupy) is not reconstructable from this file alone, so there is no
//      safe substitute expression.
//   `clRed` -- ATC/ATCInterface.h:171 already promotes `vclcompat::clLime`
//      into global scope but not its sibling `clRed` (vclcompat/LedCore.h:57);
//      this file adds its own `using vclcompat::clRed;` (not a gate, a plain
//      one-line visibility fix, listed here only so the pattern isn't missed).
//
//  SAFETY GATE REGISTER (per the task brief's explicit "動真機/寫檔類函式"
//  list -- real hardware dispatch / real file writes, reserved for explicit
//  user sign-off; full golden line numbers cited in uTemp_Set.cpp)
//  --------------------------------------------------------------------------
//   (S1) SaveSetupFile -- ENTIRE body is WriteIniData calls (~150 of them).
//        Gated whole-body; SaveSetupFile becomes a safe no-op.
//   (S2) spbSaveClick -- validation/guard logic (temperature-offset-limit
//        checks, FFC time-ordering checks, the [A01_2] operator-lockout
//        check) stays ACTIVE (pure computation + ShowMyMessage, no I/O);
//        the mutating tail (the `SaveSetupFile(...)` call onward: DefineTemp
//        path selection, CopyFile/DeleteFile, WriteIniData, SaveLastSetIni,
//        fMain->BackupSetupFile) is gated as one block.
//   (S3) SaveRemoteTempOffset -- bounds-check/decision logic stays ACTIVE;
//        the two `WriteIniData(...)` calls (the actual persist) are gated.
//   (S4) SaveRemoteTempOffsetFromGPIB -- parses `Msg` and dispatches to (S3)
//        (itself already internally gated) -- no additional gate needed here.
//   (S5) SendATCSelfTest -- whole-body gate: its entire purpose is ATC
//        command dispatch (ATCInterfaceForm/ATC_InterfaceForm ...SelfTest*
//        calls + FormHS->Show()); no "safe half" to keep active.
//   (S6) sbSafeTestATCClick -- thin wrapper, calls (S5); left un-gated at
//        this level since (S5) already self-gates.
//   (S7) SwitchOnOffRefrigeratorNo -- whole-body gate past the early
//        `iRefrigeratorNo<=0` guard (the rest is 3 ATC_InterfaceForm calls).
//   (S8) UseCloseSiteTemperatureCtrl -- NOT gated. Golden's own body is
//        `iUseCloseSiteTempCtrl[iChannel]=bUse; //No Use` -- golden's OWN
//        comment marks this dead; a plain array write with zero hardware
//        reach. Translated plainly.
//   (S9) SetSingleWorkTemperature -- decision logic (`bUse` lookup) ACTIVE;
//        both `ATC_InterfaceForm->SetSingleTemp(...)` calls gated.
//   (S10) SetCloseSiteTemperature -- NOT gated, same "No Use" golden comment
//        as (S8); a bounds check plus a plain `fCloseSiteTemp=fTemp;`.
//   (S11) btnDefrostStartClick -- guard chain (SystemStart/iATC_On/
//        AccessLevel/iWorkTemp checks, all ShowErrorMessage+return, no I/O)
//        stays ACTIVE; the terminal `fTriMachineSelectDeviceDefrost_Start(...)`
//        + `bManualDefrost_Start=true;` pair is gated.
//   (S12) btnDefrostEndClick -- whole-body gate (only the Defrost_End call).
//   (S13) ControlATC60AirFlow -- decision logic (iATC60Air/fWorkTemperBase
//        branch selection) ACTIVE; the 3 `ATC_InterfaceForm->SetAirValve(...)`
//        calls gated.
//   (S14) Handler_Send_To_ATC_DewPoint -- whole-body gate (only the
//        `Send_ATC_DewPoint` call).
//   (S15) btn_DefrostAllUseStartClick / (S16) btn_DefrostAllUseEndClick --
//        whole-body gates (pure action, no guard logic in golden either).
//   (S17) tmr_ATC_DeforstTimer -- NOT gated directly: its own body has no
//        direct hardware call, only guard checks and delegation to
//        `Tri_Temp_Low_Temperature_Use_Heater`/`Tri_Temp_High_Temperature_
//        UseNo_Heater` (both listed next, both already self-contained).
//   (S18/S19) Tri_Temp_Low_Temperature_Use_Heater / Tri_Temp_High_
//        Temperature_UseNo_Heater -- NOT gated: their one arguably-mutating
//        call, `spbSave->Click();`, resolves to `vclcompat::TControl::
//        Click()`, an established permanent NO-OP (no OnClick delegate
//        wiring exists anywhere in this tree -- see G-Delegate above) --
//        so the call is already inert by construction; ReadTempFile(true)
//        is a read, not a write. Translated plainly with a one-line comment
//        at the Click() call noting why no gate is needed.
//   (sbtExitClick, not on the brief's list but touches real hardware) --
//        the ATC-online-toggle Caption/Color bookkeeping stays ACTIVE; the
//        2 `ATCInterfaceForm->btOnLine->Click()`/`btOffLine->Click()` calls
//        (a real online/offline dispatch in golden, via the SAME BCB6
//        delegate-as-function-call idiom as G-Delegate) are gated.
//
//  DEPENDENCY AUDIT (this wave, 20260820) -- confirmed ALREADY PRESENT with
//  matching shape before translating a single call site:
//   * `SYSTEM_TEMPERATURE Temperature` (cprod.h :1375-1645, extern :1646) --
//      every one of the ~90 distinct `Temperature.*` fields this file reads/
//      writes (ATC7ChannelEnabled/ATCInitialOffset/dATCConFailOffset/
//      dATCQAModeOffset/dATCTestTimeOffset/dATCTempOffset/
//      dATCSecondTempOffset/iATC_FFCTimeOn/iATC_FFCTimeOff/dATC_FFCOffset/
//      bATC_FFCPointUse/bATC_FFCEnable/bMultiZoneEnable/bZoneTempEnable/
//      dZoneTempSetting/dATC_MultiSensorOfs/bATC_MultiSensorEnable/
//      bATC_MultiSensorUse/dTJTempRange_High/Low/bEnableTempRise/
//      dTempRiseTemp/dTempRiseDelay/EnableAirMachineSocket/
//      dSetAirstreamTemperatureRang_Socket/bUseOutShuttleDesoakTime/
//      iOutShuttleDesoakTime/bEnableTesterDryAirControl/
//      SetAirstreamTemperatureRang_Index/bEnableArm_1_Air/bEnableArm_2_Air/
//      bEnableSocket_Air/iAirVolumeLmt/dAirStreamSocket_Offset/
//      dAirStreamIndex_Offset/iDefrostTimeTooLower/
//      dIndexATCSecondTempOffset/dIndexATCInitTempOffset/ATCTypeName/
//      dATC_HotGunTemp/dATC_HotGunTime/Arm1Offset/Arm2Offset/
//      Arm{1,2}NoFullsiteOffset_{1..5}/dSocketAirCoolingOn/OffTimer/
//      bByPassChamber/bUseTriTempHeater_Ini/bPowerFollower_*/iTempMode/
//      f{Low,Midd,High,AmbientHotLow,AmbientHotMidd,SHigh}Base/
//      fTempOffSet[17][tcTotalCount]/... -- all measured present, 0 missing)
//      grepped individually against cprod.h :1375-1645 before writing
//      ReadTempFile/DoIniDataToForm/SaveSetupFile (a Python loop over ~60
//      distinct field names, 20260820 -- 0 misses).
//   * `TATCInterfaceForm *ATCInterfaceForm` (ATC/ATCInterface.h:525, the
//      "old" ATC interface -- ATC_SYS_PAL/ATC_60_SYS/SetWorkTemperature/
//      SetOffsetTemperature/SetChillerTemperature all present) is a
//      GENUINELY DIFFERENT global object from `TATC_InterfaceForm
//      *ATC_InterfaceForm` (ATC/ATC_Handler_Side.h:483, the "new" ATC
//      interface -- iATC_MODE_TYPE/SetSingleTemp/SetAirValve/
//      Send_ATC_DewPoint/etc). Golden genuinely uses BOTH names throughout
//      this file, not a typo of one for the other -- verified by reading
//      both class declarations in golden before translating a single call
//      site that used either spelling.
//   * `TMyTempPanel` (MyTempPanel.h, already ported PT-W3) -- reused as-is,
//      NOT reinvented. `listNormal`/`listArm1`/`listArm2` (`extern TList
//      *...`, MyTempPanel.h:170-172) resolve to the GLOBAL (non-vclcompat)
//      `class TList` defined in aHotPlateSubstrate.h:80 (`HT9045_TLIST_SHIM`)
//      once this file includes that header for `Init()`'s ctor-logic body --
//      `vclcompat::TList` is DELIBERATELY NOT `using`'d into the global
//      namespace anywhere in this tree (vclcompat/vcl_compat.h's own
//      documented reason: it would collide with this EXACT
//      aHotPlateSubstrate.h global `TList`), so `aHotPlateSubstrate.h` is
//      the only correct include for a COMPLETE, dereferenceable global
//      `TList` here -- matching the tree-wide precedent (cUnitConvert.cpp,
//      Motor/mymotor.cpp, BarCode/BarCode_Shuttle1_Scan.h all include
//      aHotPlateSubstrate.h for the identical reason). Also supplies the
//      canonical (177-TU) `TMyKitSuck InArmSuck` this file's `FormShow`
//      reads via `InArmSuck.HasIC()` -- the OTHER, incompatible
//      `mykitsuck.h` definition is NOT included anywhere in this file (per
//      this tree's own two-TMyKitSuck-headers gotcha).
//   * `fQwertyKey`/`ShowQwertyKey`/`Barcode_Reader`/`FTestIF`/`class TImage`/
//      `TWMKey` -- see GATE REGISTER above; each is an ALREADY-ESTABLISHED
//      tree-wide absence at translation time. fQwertyKey/ShowQwertyKey
//      EXPIRED 20260824 (FW-QWKEY1 fc08e09) -- see G-Qwerty OPENED note;
//      the others still hold.
//   * `fMain`/`fLotInfo`/`fContact` (via atester_shims.h's `TfContactShim`)
//      all already have real facades (forms/fMain.h:1053, forms/
//      fLotInfo.h:1613). ONE genuine gap found: `fMain->palIndivisual`
//      (golden `TPanel*`, `ReadTempFile`'s
//      `fMain->palIndivisual->Visible=...`/`->Visible` read) -- grepped
//      `palIndivisual` across `forms/*.h`, 20260820, 0 hits. `fMain.h` is
//      outside this wave's 3-file write boundary (cannot add the member),
//      so the 2 lines that touch it are gated in `ReadTempFile` (a genuine
//      DEPENDENCY gate, not a safety one).
//
//  Big5: golden decoded via cp950, every Chinese comment preserved as UTF-8.
//  Final gate: ZERO U+FFFD (checked over both delivered files this wave).
// =============================================================================
#ifndef FORMS_FTEMP_SET_H
#define FORMS_FTEMP_SET_H

#include "vclcompat/Controls.h"     // TPanel/TEdit/TLabel/TCheckBox/... stock widgets
#include "vclcompat/StringGrid.h"   // TStringGrid (sgTjMap/sgDefrostStatus)
#include "MyTempPanel.h"            // TMyTempPanel (already ported, PT-W3) + listNormal/listArm1/listArm2 externs
#include "MachineType.h"            // tc* channel enum, ATC_HEAD_COUNT, MAX_Index_Row/Col
#include "forms/fDynamicTemp.h"     // TfDynamicTemp -- sibling wave's deliverable (see banner: NULL-checked
#include "vclcompat/ShiftState.h"   // AI(W906-FW-SIG-W18) 20260826
                                    //   exactly like golden; if this header does not exist yet at integration
                                    //   time, that is the main loop's merge to resolve, not this file's)

using vclcompat::TStringGrid;

// golden uTemp_Set.h:26-41 -- the fTempOffSet[][] row-index constants.
// `extern` declared here (golden's own header), DEFINED in uTemp_Set.cpp
// (this is THIS wave's canonical home for these symbols -- three OTHER
// already-ported files, bthermo.cpp:233/uHeaterThread.cpp:297/
// cTemperFrom.cpp, each independently carry their OWN internal-linkage
// (`static const int`) mirror of `InitTempOffset=8` alone, per their own
// banners' "uTemp_Set is unported" note; none of those three include this
// header, so no linkage conflict exists today -- flagged here so a FUTURE
// wave that wires this header into any of them knows to drop the local
// shadow copy first, not because one exists yet).
extern const int LowBase;
extern const int MidBase;
extern const int HigBase;
extern const int UserOffSet;
extern const int SingleLimit;
extern const int TargetTemp;
extern const int AmbientHotLow;
extern const int AmbientHotMid;
extern const int InitTempOffset;
extern const int TestOverTimeTempOffset;
extern const int KitLowBase;
extern const int KitMidBase;
extern const int KitHigBase;
extern const int KitAmbientHotLow;
extern const int KitAmbientHotMid;
extern const int SHigBase;

// golden uTemp_Set.h:23 `#define MAX_DEFROST_AREA 7` -- file-scope macro,
// verbatim (DefrostFunction's 5 `bool[MAX_DEFROST_AREA]`/1
// `AnsiString[MAX_DEFROST_AREA]`/1 `int[MAX_DEFROST_AREA]` arrays below all
// need it at this exact width).
#define MAX_DEFROST_AREA 7

// ---------------------------------------------------------------------------
//  TfTemp_SetTimer -- golden `TTimer *tmr_ATC_Deforst;`/`*tmrDefrost;`
//  (uTemp_Set.h :794/:1045). A distinctly-named, minimal stand-in, NOT the
//  bare name `TTimer` -- this tree already has 4 OTHER, differently-shaped
//  bare `class TTimer` definitions (ATC/ATCInterface.h:190,
//  PowerSavingMode.h:75, OmronLaser/LaserSensor.h:174, Automation/
//  uRENESAS_Server.h:168; forms/fTemperFrom.h's own `TfTemperFromTimer`
//  cites the identical precedent/rationale for the identical golden TTimer
//  shape). `Enabled` is the only property any translated method in this
//  wave touches (the ctor's `if(AirStream_Select==1) tmr_ATC_Deforst->
//  Enabled=true;`); default `true` matches a real VCL TTimer's design-time
//  default (this class's OWN ctor conditionally sets it, so unlike
//  TfTemperFromTimer -- whose only touch was disabling on FormClose -- there
//  is no single "this wave's actual default state" to cite; the plain VCL
//  default is used).
// ---------------------------------------------------------------------------
struct TfTemp_SetTimer
{
    bool Enabled = true;
};

// ---------------------------------------------------------------------------
//  TfTemp_SetTagEdit / TfTemp_SetTagButton -- golden TComponent->Tag (a
// property EVERY VCL component has) had no port on `vclcompat::TControl` when
// this wave ran (0 hits for a `Tag` member in that header).  SUPERSEDED
// 20260825 (FW-TAG1): Tag is now a base member on vclcompat::TControl and the
// two wrappers below no longer carry their own. This tree's own established
// convention for a genuinely-needed Tag (forms/fObserver.h:283 `int Tag;`,
// forms/fConfiguration.h:286 `int Tag = 0;`) adds it as a plain member on a
// facade-local wrapper, NOT on the shared vclcompat type -- same "compose,
// don't fork" idiom as TfTemperFromPanel/TfTemp_SetTimer above. Two thin
// wrappers, each used ONLY where golden's own Tag read is REAL business
// state (not merely a KYEC-barcode-guard already gated away, see
// uTemp_Set.cpp's own GATE(G-Barcode) notes for those):
//   * `edtSetTempature2AirMachineClick` (golden :6903-6929) dispatches on
//     `((TEdit*)Sender)->Tag` to pick between `edt_SetIndexAirstreamTemp`/
//     `edtSetTempature2AirMachine`.  CORRECTION 20260825 (FW-TAG1), this
//     comment used to say "(Tag 0/1 at golden design-time)" and that is
//     FALSE: all FOUR widgets golden wires to that handler
//     (edt_SetIndexAirstreamTemp dfm:4894, edt_SetAirstreamTemperatureRang_
//     Index dfm:4905, edtSetTempature2AirMachine dfm:5024, edt_SetAirstream
//     TemperatureRang_Socket dfm:5035) carry NO Tag line, and golden's
//     uTemp_Set.cpp writes Tag exactly once in the whole file (btnSort->Tag=0
//     at golden :5768).  Golden therefore always evaluates Tag==0 here and
//     always writes edt[0]; the port reproduces that exactly.  -- the
//     ONLY 2 TEdit members in this whole file where Tag is genuinely read as
//     data, not a guard.
//   * `btnSort` (golden :5764-5781, :3671-4101) cycles a 3-state sort mode
//     through its own Tag (0/1/2) -- read back by UpDateEdit to choose the
//     Align/sort branch.
// ---------------------------------------------------------------------------
// AI(W906-FW-TAG1) 20260825: both `int Tag = 0;` members removed -- Tag now
// lives on vclcompat::TControl for every widget (provenance note there); a
// copy here would shadow it.  The two classes stay because the member
// declarations below and their casts name these types.
class TfTemp_SetTagEdit : public vclcompat::TEdit
{
};
class TfTemp_SetTagButton : public vclcompat::TSpeedButton
{
};

// ===========================================================================
//  TfTemp_Set -- non-VCL facade (golden uTemp_Set.h, TfTemp_Set:public TForm)
// ===========================================================================
class TfTemp_Set
{
public:
    TfTemp_Set();

    // golden ctor's ENTIRE body (asTempCtrl[] seeding, myTempPal[]/
    // listNormal/listArm1/listArm2 population, SetParent/SetIndexTag wiring,
    // ATCOffsetEdit[]/ATCPackageOffsetEdit[]/ATC_FFCOffset*Edit[][]/
    // ZoneTempUse[]/MultiSensorOffsetUse[]/ATC_MultiSensorOffsetEdit[]
    // global-array seeding, golden :95-408) lives here instead, per this
    // wave's "ctor only fields, real logic -> explicit Init()" convention.
    // Not called by anything yet this wave -- a later wiring wave's job.
    void Init();

    // -- plain state (golden :1017-1018, :1043) -------------------------------
    bool fShow = false;
    int  iTempMoldSet = 0;
    bool bNeedChange = false;                   // golden :1043

    // -- myTempPal[] (golden :1019) -- one TMyTempPanel* per channel, built
    // by Init(), NOT NSDMI'd here (each entry needs asTempCtrl[i]/i from
    // Init()'s own logic, matching golden's ctor-time `new TMyTempPanel(...)`
    // -- see MyTempPanel.h, already ported).
    TMyTempPanel *myTempPal[tcTotalCount] = {};

    // -- lblIndexLabel[2][16] (golden :1030) -- zero call sites touch this
    // array anywhere in golden's OWN uTemp_Set.cpp (grepped, 20260820, 0
    // hits beyond the declaration) -- NSDMI'd to nullptr, no Init() touch
    // either, matching golden's own dead-declaration posture (same class of
    // finding as ATC_Power/CheckAirMachineStatus above, just for a data
    // member instead of a method).
    TLabel *lblIndexLabel[2][16] = {};

    void SetBasePointIMG(int iPoint);                                       // golden :410-419, GATE(G-Image)
    void UpDateEdit();                                                      // golden :3671-4101
    void ReadTempFile(bool bUpdateAll);                                     // golden :1976-3143
    void DoIniDataToForm(bool bUpdateAll);                                  // golden :3145-3669
    void SaveSetupFile(AnsiString szDir, AnsiString S);                     // golden :4532-5099, SAFETY GATE (S1)
    void SendATCSelfTest(int Source);                                       // golden :5503-5548, SAFETY GATE (S5)
    int  SaveRemoteTempOffset(int iArm, int iSite, double dOffset);         // golden :5856-5973, SAFETY GATE (S3)
    void SaveRemoteTempOffsetFromGPIB(AnsiString Msg);                      // golden :5975-6059, SAFETY GATE (S4, delegates)
    void ReadRemoteTempOffset();                                            // golden :6061-6115 (read-only, no gate)

    double MaxTempSetting();                                                // golden :5619-5685
    double MinTempSetting();                                                // golden :5687-5721
    void ControlATC60AirFlow(int iStatus = -1);                             // golden :6132-6160, SAFETY GATE (S13)
                                                                             // -- KEEP this exact signature/default:
                                                                             // TempCtrl/uHeaterThread.cpp GATE 6 and
                                                                             // cprod.cpp:3989-4003 both already carry
                                                                             // `#if 0`-gated `fTemp_Set->
                                                                             // ControlATC60AirFlow(...)` call sites
                                                                             // waiting on this exact public member.

    // -- ATC channel<->site translation matrices (golden :1035-1041) --------
    int iAddrToATC[tcTotalCount] = {};
    int iATCToAddr[ATC_HEAD_COUNT] = {};
    int iSiteToATC[2][MAX_Index_Row][MAX_Index_Col] = {};
    int iSiteToOfs[2][MAX_Index_Row][MAX_Index_Col] = {};
    int iATCToSiteArm[ATC_HEAD_COUNT] = {};
    int iATCToSiteRow[ATC_HEAD_COUNT] = {};
    int iATCToSiteCol[ATC_HEAD_COUNT] = {};
    int InitialAddrToATC();                                                 // golden :6329-6662 (no gate, pure data calc)

    // -- Tri-Temp (HT-1032) defrost sub-system (golden :1046-1056) ----------
    struct stcDefrostFunction
    {
        bool       bDefrostReq[MAX_DEFROST_AREA] = {};
        bool       bDefrostSetTemp[MAX_DEFROST_AREA] = {};
        bool       bDefrostHeart_OK[MAX_DEFROST_AREA] = {};
        bool       bDefrostStart[MAX_DEFROST_AREA] = {};
        bool       bDefrostEnd[MAX_DEFROST_AREA] = {};
        AnsiString strDefrostDevice[MAX_DEFROST_AREA];
        int        iDefrostCountingDownTime[MAX_DEFROST_AREA] = {};
    } DefrostFunction;
    bool bTestDefrost = false;

    void SwitchOnOffRefrigeratorNo(int iRefrigeratorNo, int iOnOff);        // golden :6672-6683, SAFETY GATE (S7)
    void UseCloseSiteTemperatureCtrl(int iChannel, bool bUse);              // golden :6685-6688 (no gate, golden "No Use")
    void SetSingleWorkTemperature(int iChannel, double fTemp, bool bDefrost = false); // golden :6690-6698, SAFETY GATE (S9)
    void SetCloseSiteTemperature(double fTemp);                            // golden :6700-6706 (no gate, golden "No Use")
    void SetTempPanelCaption();                                            // golden :1323-1974 (no gate, pure Caption)
    int  CheckVerSupportFunction();                                        // golden :6778-6814 (no gate)
    void ATC_Online_Or_offline();                                          // golden :6840-6848, GATE(G-Delegate)

    // golden uTemp_Set.h:1063/:1065 -- declared, NEVER DEFINED anywhere in
    // golden (see banner OMITTED note above). Declared here to mirror
    // golden's header exactly; deliberately has NO body in uTemp_Set.cpp,
    // and nothing calls either, so this is not an ODR problem.
    void ATC_Power();
    void CheckAirMachineStatus();

    void Tri_Temp_Low_Temperature_Use_Heater();                            // golden :6850-6871 (no gate, see banner)
    void Handler_Send_To_ATC_DewPoint(double dDewPoint_Index, double dDewPoint_InSht, double dDewPointOutSht); // golden :6891-6894, SAFETY GATE (S14)
    void Tri_Temp_High_Temperature_UseNo_Heater();                         // golden :6873-6889 (no gate, see banner)

    // -- private in golden (uTemp_Set.h :1010-1014), collapsed to public
    // here -- same convention as every other translated form facade in this
    // tree (C++ access control buys this class nothing; matches forms/
    // fLotInfo.h/fContactCT.h's own established collapse).
    void   DisableOffset();                                                 // golden :5137-5148
    void   DisplayTargetTempEdit(bool Visible);                             // golden :5233-5311
    bool   CheckTempSettingChange();                                        // golden :5555-5615
    void   ShowLineOnTop();                                                 // golden :5823-5854, GATE(G-Align)

    // -- DEVIATION: golden `Close()` (real VCL TForm::Close(), called from
    // spbSaveClick :4207/sbtExitClick :5123) -- no window exists here to
    // close; permanent no-op, matching this tree's established precedent for
    // the identical golden idiom (forms/fObserver.h's own `Close()`
    // DEVIATION note, cObserver.cpp:2147). Does NOT forward into FormClose()
    // (unlike ATC/ATCInterface.cpp's OWN Close()->FormClose() idiom for a
    // DIFFERENT golden shape where FormClose carries the state golden's
    // Close() call site actually depends on) -- neither of THIS file's two
    // Close() call sites reads anything FormClose sets afterward.
    void Close();

    // -- __published event handlers (golden :905-1009) ----------------------
    void rb1PointClick(TObject *Sender);
    void FormCreate(TObject *Sender);
    void FormShow(TObject *Sender);
    void edSoakTimeKeyPress(TObject *Sender, char &Key);
    void edSoakTimeClick(TObject *Sender);
    void edLHP1MouseDown(TObject *Sender);                                  // DEVIATION: TMouseButton/TShiftState dropped
    void FormClose(TObject *Sender);                                       // DEVIATION: TCloseAction dropped
    void edLHP1KeyUp(TObject *Sender, WORD &Key, TShiftState Shift);                                     // DEVIATION: WORD&/TShiftState dropped
    void edLowBaseMouseDown(TObject *Sender, TMouseButton Button, TShiftState Shift, int X, int Y);                              // DEVIATION
    void rgIndexHeatModeClick(TObject *Sender);
    void spbSaveClick(TObject *Sender);                                    // SAFETY GATE (S2)
    void edLHeatGun1MouseDown(TObject *Sender, TMouseButton Button, TShiftState Shift, int X, int Y);                            // DEVIATION
    void rgTemperatureModeClick(TObject *Sender);
    void edTargetHP1MouseDown(TObject *Sender, TMouseButton Button, TShiftState Shift, int X, int Y);                            // DEVIATION
    void cbEnableIndividualModeClick(TObject *Sender);
    void edtATCInPC1MouseDown(TObject *Sender, TMouseButton Button, TShiftState Shift, int X, int Y);                            // DEVIATION
    void rbATC70ActiveOnClick(TObject *Sender);
    void rbATCActiveOnClick(TObject *Sender);
    void sbtExitClick(TObject *Sender);
    void btClearAllClick(TObject *Sender);
    void pgcTempOffsetChange(TObject *Sender);
    void edAtcFileNameMouseDown(TObject *Sender, TMouseButton Button, TShiftState Shift, int X, int Y);                          // DEVIATION
    void edATCAmbTempMouseDown(TObject *Sender, TMouseButton Button, TShiftState Shift, int X, int Y);                           // DEVIATION
    void edArm1OffsetMouseDown(TObject *Sender, TMouseButton Button, TShiftState Shift, int X, int Y);                           // DEVIATION
    void edATCTestTimeOffsetMouseDown(TObject *Sender, TMouseButton Button, TShiftState Shift, int X, int Y);                    // DEVIATION
    void edATCInitialOffset1MouseDown(TObject *Sender, TMouseButton Button, TShiftState Shift, int X, int Y);                    // DEVIATION
    void edATCTempAlwaysSameAlarmMouseDown(TObject *Sender, TMouseButton Button, TShiftState Shift, int X, int Y);               // DEVIATION
    void edArm1NoFullsiteOffset_1MouseDown(TObject *Sender, TMouseButton Button, TShiftState Shift, int X, int Y);               // DEVIATION
    void edTSDTimeOutMouseDown(TObject *Sender, TMouseButton Button, TShiftState Shift, int X, int Y);                           // DEVIATION
    void sbSafeTestATCClick(TObject *Sender);                              // SAFETY GATE (S6, delegates)
    void edATCChillerTempMouseDown(TObject *Sender, TMouseButton Button, TShiftState Shift, int X, int Y);                       // DEVIATION
    void edAmbTempClick(TObject *Sender);
    void edtIdleTime_LongClick(TObject *Sender);
    void edtBoostOffset_LongClick(TObject *Sender);
    void edAmbHotGuartbentClick(TObject *Sender);
    void edtBoostOffsetClick(TObject *Sender);
    void edtInitialDelay_1Click(TObject *Sender);
    void btnSortClick(TObject *Sender);
    void edLimitHP1MouseDown(TObject *Sender, TMouseButton Button, TShiftState Shift, int X, int Y);                             // DEVIATION
    void edtLBTimeOutClick(TObject *Sender);
    void FormDestroy(TObject *Sender);
    void edTJTempRange_HighClick(TObject *Sender);
    void edTJTempRange_LowClick(TObject *Sender);
    void edtThresholdClick(TObject *Sender);
    void edDewPointRangeMouseDown(TObject *Sender, TMouseButton Button, TShiftState Shift, int X, int Y);                        // DEVIATION
    void edDewPointAlarmIntervalMouseDown(TObject *Sender, TMouseButton Button, TShiftState Shift, int X, int Y);                // DEVIATION
    void btnSameAsArm1Click(TObject *Sender);
    void edtChamberBoostTimeClick(TObject *Sender);
    void edtChamberBoostOffsetClick(TObject *Sender);
    void edTempOffsetCountClick(TObject *Sender);
    void edTempDownContactDelayClick(TObject *Sender);
    void edtHeatGunTempATCMouseDown(TObject *Sender, TMouseButton Button, TShiftState Shift, int X, int Y);                      // DEVIATION
    void chkTempCalByRecipeClick(TObject *Sender);
    // FormShortCut(TWMKey&,bool&) OMITTED -- see banner OMITTED note.
    void edLBTempAlarmTimeMouseDown(TObject *Sender, TMouseButton Button, TShiftState Shift, int X, int Y);                      // DEVIATION
    void edtLBAirOnTempMouseDown(TObject *Sender, TMouseButton Button, TShiftState Shift, int X, int Y);                         // DEVIATION
    void edtATCPIDOffset_MinPClick(TObject *Sender);
    void edJamSoakTimeClick(TObject *Sender);
    void edFFC_Arm1TimeOn_01Click(TObject *Sender);
    void edtATC_HotGunTimeClick(TObject *Sender);
    void edtLBTempMinClick(TObject *Sender);
    void edAmbGuardbandClick(TObject *Sender);
    void rgBasePointClick(TObject *Sender);
    void edtSetTJ_OffsetClick(TObject *Sender);
    void edtSetTJ_SlopeChange(TObject *Sender);
    void sgTjMapMouseDown(TObject *Sender, TMouseButton Button, TShiftState Shift, int X, int Y);   //AI(W906-FW-SIG-W18) 20260826: 簽章回填為 golden 原文; GATE(G-Grid) 仍在
    void rgTjMapTypeClick(TObject *Sender);
    void edTempReadyRangeMouseDown(TObject *Sender);                       // DEVIATION
    void edATCOfsTimeMouseDown(TObject *Sender, TMouseButton Button, TShiftState Shift, int X, int Y);                           // DEVIATION
    void cbSelectDeviceChange(TObject *Sender);
    void btnDefrostStartClick(TObject *Sender);                           // SAFETY GATE (S11)
    void btnDefrostEndClick(TObject *Sender);                             // SAFETY GATE (S12)
    void tmr_ATC_DeforstTimer(TObject *Sender);                            // golden :6816-6838 (no direct gate, see S17)
    void edt_SetAirstreamTemperatureRang_SocketClick(TObject *Sender);
    void edtSetTempature2AirMachineClick(TObject *Sender);
    void edt_AirVolumeLmt_IndexClick(TObject *Sender);
    void btn_DefrostAllUseStartClick(TObject *Sender);                    // SAFETY GATE (S15)
    void edt_Defrost_Time_Too_LowerClick(TObject *Sender);
    void btn_DefrostAllUseEndClick(TObject *Sender);                      // SAFETY GATE (S16)
    void cbATCReferTempSensorClick(TObject *Sender);

    // -- widget members (golden __published, EXACT declaration order) -------
    // Mechanically generated from golden uTemp_Set.h's __published block
    // (python re over the cp950-decoded header, one XForm-type -> vclcompat
    // NSDMI line per widget, verified 858/858 declarations accounted for --
    // 856 real widgets + 2 TImage omissions below). TImage members (Image1/
    // Image2) OMITTED per GATE(G-Image); TTimer members use TfTemp_SetTimer
    // per the DESIGN NOTE above.
    TPanel *Panel7 = new TPanel();
    TSpeedButton *spbSave = new TSpeedButton();
    TSpeedButton *sbtExit = new TSpeedButton();
    TPanel *Panel8 = new TPanel();
    TPanel *Panel1 = new TPanel();
    TGroupBox *gbTempOffset = new TGroupBox();
    TPageControl *pgcTempOffset = new TPageControl();
    TTabSheet *thNormal = new TTabSheet();
    TTabSheet *tsArm1 = new TTabSheet();
    TTabSheet *tsArm2 = new TTabSheet();
    TTabSheet *tsATC = new TTabSheet();
    TTabSheet *tsHeatGun = new TTabSheet();
    TPanel *Panel6 = new TPanel();
    TPanel *Panel11 = new TPanel();
    TPanel *Panel2 = new TPanel();
    TPanel *Panel13 = new TPanel();
    TPanel *pnlHeatMode = new TPanel();
    TGroupBox *gbBasePoint = new TGroupBox();
    TRadioButton *rb1Point = new TRadioButton();
    TRadioButton *rb2Point = new TRadioButton();
    TRadioButton *rb3Point = new TRadioButton();
    TCheckBox *cbHeaterGun = new TCheckBox();
    TTabSheet *tsDUT = new TTabSheet();
    TPanel *Panel15 = new TPanel();
    TPanel *Panel16 = new TPanel();
    // OMITTED (GATE G-Image): TImage *Image2; -- TImage has zero port anywhere in this tree.
    TCheckBox *cbUseFixTemp = new TCheckBox();
    TEdit *edFixedTemp = new TEdit();
    TCheckBox *cbShowFixTemp = new TCheckBox();
    TPanel *Panel21 = new TPanel();
    TPanel *Panel27 = new TPanel();
    TPanel *Panel28 = new TPanel();
    TRadioButton *rb5Point = new TRadioButton();
    TPanel *Panel4 = new TPanel();
    TPanel *Panel31 = new TPanel();
    TPanel *Panel32 = new TPanel();
    TPanel *palATC = new TPanel();
    TTabSheet *tsATC_PID = new TTabSheet();
    TPanel *pnlATC_PIDOffset = new TPanel();
    TLabel *Label371 = new TLabel();
    TLabel *Label372 = new TLabel();
    TLabel *Label373 = new TLabel();
    TLabel *Label374 = new TLabel();
    TLabel *Label375 = new TLabel();
    TLabel *Label376 = new TLabel();
    TPanel *pnlATC_PIDOffsetTop = new TPanel();
    TEdit *edtATCPIDOffset_MinP = new TEdit();
    TEdit *edtATCPIDOffset_MinI = new TEdit();
    TEdit *edtATCPIDOffset_MinD = new TEdit();
    TEdit *edtATCPIDOffset_MaxP = new TEdit();
    TEdit *edtATCPIDOffset_MaxI = new TEdit();
    TEdit *edtATCPIDOffset_MaxD = new TEdit();
    TCheckBox *cdATCUsePIDCtrl = new TCheckBox();
    TTabSheet *tsATC70 = new TTabSheet();
    TGroupBox *gbATCInitialOffset = new TGroupBox();
    TLabel *labATCInitialOffset1 = new TLabel();
    TLabel *labATCInitialOffset2 = new TLabel();
    TLabel *labATCInitialOffset4 = new TLabel();
    TLabel *labATCInitialOffset3 = new TLabel();
    TEdit *edATCInitialOffset1 = new TEdit();
    TEdit *edATCInitialOffset2 = new TEdit();
    TEdit *edATCInitialOffset3 = new TEdit();
    TEdit *edATCInitialOffset4 = new TEdit();
    TGroupBox *gbATC70 = new TGroupBox();
    TCheckBox *rbATC70ActiveOn = new TCheckBox();
    TGroupBox *gbATCConsFail = new TGroupBox();
    TLabel *labATCContFailOffset1 = new TLabel();
    TLabel *labATCContFailOffset2 = new TLabel();
    TLabel *labATCContFailOffset4 = new TLabel();
    TLabel *labATCContFailOffset3 = new TLabel();
    TLabel *Label26 = new TLabel();
    TEdit *edATCContFailOffset1 = new TEdit();
    TEdit *edATCContFailOffset2 = new TEdit();
    TEdit *edATCContFailOffset3 = new TEdit();
    TEdit *edATCContFailOffset4 = new TEdit();
    TCheckBox *cbEnableATCConsFailOffset = new TCheckBox();
    TEdit *edATCContFailOffsetCnt = new TEdit();
    TGroupBox *gbATCQAModeOffset = new TGroupBox();
    TLabel *labATCQAModeOffset1 = new TLabel();
    TLabel *labATCQAModeOffset2 = new TLabel();
    TLabel *labATCQAModeOffset4 = new TLabel();
    TLabel *labATCQAModeOffset3 = new TLabel();
    TEdit *edATCQAModeOffset1 = new TEdit();
    TEdit *edATCQAModeOffset2 = new TEdit();
    TEdit *edATCQAModeOffset3 = new TEdit();
    TEdit *edATCQAModeOffset4 = new TEdit();
    TCheckBox *cbEnableATCQAModeOffset = new TCheckBox();
    TEdit *edWorkTemp = new TEdit();
    TButton *btClearAll = new TButton();
    TPanel *pnlOffsetBase = new TPanel();
    TGroupBox *gbATCTestTimeOffset = new TGroupBox();
    TLabel *labATCTestTimeOffset1 = new TLabel();
    TLabel *labATCTestTimeOffset2 = new TLabel();
    TLabel *labATCTestTimeOffset4 = new TLabel();
    TLabel *labATCTestTimeOffset3 = new TLabel();
    TEdit *edATCTestTimeOffset1 = new TEdit();
    TEdit *edATCTestTimeOffset2 = new TEdit();
    TEdit *edATCTestTimeOffset3 = new TEdit();
    TEdit *edATCTestTimeOffset4 = new TEdit();
    TCheckBox *cbATCTestTimeOffset = new TCheckBox();
    TLabel *labATCTestTimeOffset = new TLabel();
    TEdit *edATCTestTimeOffset = new TEdit();
    TPanel *Panel36 = new TPanel();
    TPanel *Panel41 = new TPanel();
    TPanel *Panel42 = new TPanel();
    TGroupBox *gbTSD = new TGroupBox();
    TCheckBox *rbATC7TSDFunction = new TCheckBox();
    TLabel *Label17 = new TLabel();
    TEdit *edTSDTimeOut = new TEdit();
    TPageControl *pgcTempSetting = new TPageControl();
    TTabSheet *tsHot = new TTabSheet();
    TGroupBox *gbHotModeSetting = new TGroupBox();
    TLabel *Label1 = new TLabel();
    TLabel *Label2 = new TLabel();
    TLabel *Label3 = new TLabel();
    TLabel *Label30 = new TLabel();
    TLabel *Label10 = new TLabel();
    TLabel *Label79 = new TLabel();
    TLabel *Label21 = new TLabel();
    TLabel *Label22 = new TLabel();
    TLabel *lblShtSoakTime = new TLabel();
    TLabel *lbllblShtSoakTimeSec = new TLabel();
    TLabel *Label44 = new TLabel();
    TLabel *Label45 = new TLabel();
    TEdit *edSoakTime = new TEdit();
    TEdit *edJamSoakTime = new TEdit();
    TEdit *edInitialWaitTime = new TEdit();
    TEdit *edInitialStart1Time = new TEdit();
    TEdit *edInitialStart2Time = new TEdit();
    TEdit *edChamberCoolTemp = new TEdit();
    TRadioGroup *rgShuttleSoakTime = new TRadioGroup();
    TRadioGroup *rgZ1Down = new TRadioGroup();
    TPanel *PnlIndexSoakTime = new TPanel();
    TLabel *Label54 = new TLabel();
    TLabel *Label53 = new TLabel();
    TLabel *Label55 = new TLabel();
    TLabel *Label56 = new TLabel();
    TEdit *edIndexSoakTime = new TEdit();
    TEdit *edOSTime = new TEdit();
    TCheckBox *chkTesterDock = new TCheckBox();
    TEdit *edtTestDock = new TEdit();
    TCheckBox *cbEnableIndividualMode = new TCheckBox();
    TGroupBox *gbNoFullsiteOffset = new TGroupBox();
    TLabel *Label109 = new TLabel();
    TLabel *Label110 = new TLabel();
    TLabel *Label111 = new TLabel();
    TLabel *Label112 = new TLabel();
    TLabel *Label113 = new TLabel();
    TLabel *Label114 = new TLabel();
    TLabel *Label83 = new TLabel();
    TLabel *Label115 = new TLabel();
    TLabel *Label9 = new TLabel();
    TLabel *Label11 = new TLabel();
    TLabel *Label12 = new TLabel();
    TLabel *Label13 = new TLabel();
    TEdit *edArm1NoFullsiteOffset_1 = new TEdit();
    TEdit *edArm1NoFullsiteOffset_2 = new TEdit();
    TEdit *edArm1NoFullsiteOffset_3 = new TEdit();
    TEdit *edArm2NoFullsiteOffset_1 = new TEdit();
    TEdit *edArm2NoFullsiteOffset_2 = new TEdit();
    TEdit *edArm2NoFullsiteOffset_3 = new TEdit();
    TEdit *edArm1NoFullsiteOffset_4 = new TEdit();
    TEdit *edArm2NoFullsiteOffset_4 = new TEdit();
    TEdit *edArm1NoFullsiteOffset_5 = new TEdit();
    TEdit *edArm2NoFullsiteOffset_5 = new TEdit();
    TCheckBox *chkSoakTimeReduce = new TCheckBox();
    TTabSheet *tsAmbient = new TTabSheet();
    TGroupBox *gbAmbientModeSetting = new TGroupBox();
    TLabel *Label4 = new TLabel();
    TLabel *Label5 = new TLabel();
    TLabel *Label6 = new TLabel();
    TLabel *Label7 = new TLabel();
    TLabel *Label31 = new TLabel();
    TLabel *Label80 = new TLabel();
    TCheckBox *cbUseAbitCHK = new TCheckBox();
    TRadioGroup *rgAmbCheck = new TRadioGroup();
    TEdit *edAmbTemp = new TEdit();
    TEdit *edAbitInitWaitTime = new TEdit();
    TEdit *edAbitColdTime = new TEdit();
    TCheckBox *chkAmbUsingAFan = new TCheckBox();
    TGroupBox *gbSocketAirCooling = new TGroupBox();
    TLabel *Label19 = new TLabel();
    TLabel *Label20 = new TLabel();
    TLabel *Label27 = new TLabel();
    TLabel *Label28 = new TLabel();
    TEdit *edSocketAirCoolingOn = new TEdit();
    TEdit *edSocketAirCoolingOff = new TEdit();
    TGroupBox *rgAmbientBrand = new TGroupBox();
    TLabel *Label14 = new TLabel();
    TLabel *Label15 = new TLabel();
    TCheckBox *cbUseAmbGuard = new TCheckBox();
    TEdit *edAmbGuardband = new TEdit();
    TTabSheet *tsAmbCtr = new TTabSheet();
    TTabSheet *tsOther = new TTabSheet();
    TPageControl *pgcOtherFunc = new TPageControl();
    TTabSheet *tsInitOffset = new TTabSheet();
    TGroupBox *bgTempOffset = new TGroupBox();
    TLabel *labContctCount = new TLabel();
    TLabel *labTempDownContactDelay = new TLabel();
    TCheckBox *cbTempOffset = new TCheckBox();
    TEdit *edTempOffsetCount = new TEdit();
    TEdit *edTempDownContactDelay = new TEdit();
    TPanel *pnlUseOtherArmToTestAfterInitialDelay = new TPanel();
    TCheckBox *cbUseOtherArmToTestAfterInitialDelay = new TCheckBox();
    TGroupBox *GroupBox1 = new TGroupBox();
    TPanel *pnl1stDevice = new TPanel();
    TCheckBox *cbEveryFirstDeviceUseInitialDelay = new TCheckBox();
    TEdit *edtInitialDelay_1 = new TEdit();
    TPanel *pnlTitle = new TPanel();
    TLabel *lblDelayTime = new TLabel();
    TLabel *lblTriggerTime = new TLabel();
    TPanel *pnlPause = new TPanel();
    TCheckBox *cbWhenPressStopOverUseInitialDelay = new TCheckBox();
    TEdit *edtInitialDelay_6 = new TEdit();
    TEdit *edOverSec = new TEdit();
    TPanel *pnlLowTestTime = new TPanel();
    TCheckBox *cbWhenHappenTestedTimeBelowUseInitialDelay = new TCheckBox();
    TEdit *edtInitialDelay_4 = new TEdit();
    TEdit *edBelowSec = new TEdit();
    TPanel *pnlHeatDoor = new TPanel();
    TCheckBox *cbAfterOpenHeatDoorUseInitialDelay = new TCheckBox();
    TEdit *edtInitialDelay_5 = new TEdit();
    TPanel *pnlAutoClean = new TPanel();
    TEdit *edtInitialDelay_3 = new TEdit();
    TCheckBox *cbAfterAutoCleanFunctionUseInitialDelay = new TCheckBox();
    TPanel *pnlAlarmMsg = new TPanel();
    TCheckBox *cbAfterShowAlarmMessageUseInitialDelay = new TCheckBox();
    TEdit *edtInitialDelay_2 = new TEdit();
    TPanel *pnlOTD = new TPanel();
    TCheckBox *cbOTDUnlockDelay = new TCheckBox();
    TEdit *edtInitialDelay_9 = new TEdit();
    TPanel *pnlEOTTime = new TPanel();
    TCheckBox *cbTestFinishToNextTestOver = new TCheckBox();
    TEdit *iTestFinishToNextTestOver = new TEdit();
    TEdit *edtInitialDelay_8 = new TEdit();
    TPanel *pnlNoFullsite = new TPanel();
    TCheckBox *cbWhenNoFullSiteUseInitialDelay = new TCheckBox();
    TEdit *edtInitialDelay_7 = new TEdit();
    TTabSheet *tsBoostFunc = new TTabSheet();
    TGroupBox *grpMidPrecondition = new TGroupBox();
    TPanel *pnlIdleTime_Mid = new TPanel();
    TLabel *lblIdleTime_Mid = new TLabel();
    TEdit *edtIdleTime_Mid = new TEdit();
    TPanel *pnlBoostOffset_Mid = new TPanel();
    TLabel *lblBoostOffset_Mid = new TLabel();
    TEdit *edtBoostOffset_Mid = new TEdit();
    TPanel *pnlBoostDuration_Mid = new TPanel();
    TLabel *lblBoostDuration_Mid = new TLabel();
    TEdit *edtBoostDuration_Mid = new TEdit();
    TPanel *pnlPostBoost_Mid = new TPanel();
    TLabel *lblPostBoost_Mid = new TLabel();
    TEdit *edtPostBoost_Mid = new TEdit();
    TPanel *Panel14 = new TPanel();
    TCheckBox *chkBoostFunction = new TCheckBox();
    TGroupBox *grpShortPrecondition = new TGroupBox();
    TPanel *pnlIdleTime_Short = new TPanel();
    TLabel *lblIdleTime_Short1 = new TLabel();
    TLabel *lblIdleTime_Short = new TLabel();
    TEdit *edtIdleTime_Short = new TEdit();
    TPanel *pnlBoostOffset_Short = new TPanel();
    TLabel *lblBoostOffset_Short1 = new TLabel();
    TLabel *lblBoostOffset_Short = new TLabel();
    TEdit *edtBoostOffset_Short = new TEdit();
    TPanel *pnlBoostDuration_Short = new TPanel();
    TLabel *lblBoostDuration_Short1 = new TLabel();
    TLabel *lblBoostDuration_Short = new TLabel();
    TEdit *edtBoostDuration_Short = new TEdit();
    TPanel *pnlPostBoost_Short = new TPanel();
    TLabel *lblPostBoost_Short1 = new TLabel();
    TLabel *lblPostBoost_Short = new TLabel();
    TEdit *edtPostBoost_Short = new TEdit();
    TGroupBox *grpLongPrecondition = new TGroupBox();
    TPanel *pnlIdleTime_Long = new TPanel();
    TLabel *lblIdleTime_Long1 = new TLabel();
    TLabel *lblIdleTime_Long = new TLabel();
    TEdit *edtIdleTime_Long = new TEdit();
    TPanel *pnlBoostOffset_Long = new TPanel();
    TLabel *lblBoostOffset_Long1 = new TLabel();
    TLabel *lblBoostOffset_Long = new TLabel();
    TEdit *edtBoostOffset_Long = new TEdit();
    TPanel *pnlBoostDuration_Long = new TPanel();
    TLabel *lblBoostDuration_Long1 = new TLabel();
    TLabel *lblBoostDuration_Long = new TLabel();
    TEdit *edtBoostDuration_Long = new TEdit();
    TPanel *pnlPostBoost_Long = new TPanel();
    TLabel *lblPostBoost_Long1 = new TLabel();
    TLabel *lblPostBoost_Long = new TLabel();
    TEdit *edtPostBoost_Long = new TEdit();
    TGroupBox *grpTempCtrl = new TGroupBox();
    TLabel *lblAmbGuardBand = new TLabel();
    TLabel *lblAmbGuardBandDeg = new TLabel();
    TEdit *edAmbHotGuartbent = new TEdit();
    TCheckBox *chkShuttleNoHeatUp = new TCheckBox();
    TGroupBox *rgTemperatureMode1 = new TGroupBox();
    TRadioGroup *rgTemperatureMode = new TRadioGroup();
    TGroupBox *rgIndexHeatMode1 = new TGroupBox();
    TPanel *Panel5 = new TPanel();
    TPanel *Panel10 = new TPanel();
    TPanel *Panel25 = new TPanel();
    TPanel *Panel29 = new TPanel();
    TPanel *Panel34 = new TPanel();
    TPanel *Panel17 = new TPanel();
    TLabel *lblNotice = new TLabel();
    TMemo *palIndiTemp = new TMemo();
    TMemo *Memo1 = new TMemo();
    TMemo *Memo2 = new TMemo();
    TMemo *memoInitialTempOffset = new TMemo();
    TMemo *Memo3 = new TMemo();
    TPanel *pnlArm2Offset = new TPanel();
    TLabel *lblArm2Offset = new TLabel();
    TEdit *edArm2Offset = new TEdit();
    TPanel *pnlArm1Offset = new TPanel();
    TLabel *lblArm1Offset = new TLabel();
    TEdit *edArm1Offset = new TEdit();
    TGroupBox *grpBoostMode = new TGroupBox();
    TRadioGroup *rgBoostMode = new TRadioGroup();
    TPanel *Panel18 = new TPanel();
    TGroupBox *grbATCTempAlarm = new TGroupBox();
    TLabel *Label46 = new TLabel();
    TEdit *edtATCSP2 = new TEdit();
    TGroupBox *grpTemperatureCheck = new TGroupBox();
    TCheckBox *rbTestStartOn = new TCheckBox();
    TCheckBox *rbHandlerStartOn = new TCheckBox();
    TGroupBox *gb_TjFunction = new TGroupBox();
    TCheckBox *chk_UseTj = new TCheckBox();
    TRadioGroup *rg_TjMode = new TRadioGroup();
    TEdit *edt_TjAvgTimes = new TEdit();
    TPanel *Panel19 = new TPanel();
    TGroupBox *gbChillerTemp = new TGroupBox();
    TEdit *edChillerTemp = new TEdit();
    TGroupBox *gbATCAmbientTemp = new TGroupBox();
    TLabel *Label16 = new TLabel();
    TEdit *edATCAmbTemp = new TEdit();
    TGroupBox *grpTempAlwaysSameTime = new TGroupBox();
    TLabel *lb1 = new TLabel();
    TEdit *edTempAlwaysSameAlarm = new TEdit();
    TPanel *Panel20 = new TPanel();
    TGroupBox *grpATCTempOffset = new TGroupBox();
    TLabel *lb2 = new TLabel();
    TLabel *lb3 = new TLabel();
    TLabel *lb4 = new TLabel();
    TLabel *lb5 = new TLabel();
    TLabel *lb6 = new TLabel();
    TLabel *lb7 = new TLabel();
    TEdit *edtATCOffset_01 = new TEdit();
    TEdit *edtATCOffset_02 = new TEdit();
    TEdit *edtATCOffset_03 = new TEdit();
    TEdit *edtATCOffset_04 = new TEdit();
    TEdit *edtATCOffset_05 = new TEdit();
    TEdit *edtATCOffset_06 = new TEdit();
    TEdit *edtATCOffset_07 = new TEdit();
    TEdit *edtATCOffset_08 = new TEdit();
    TEdit *edtATCOffset_09 = new TEdit();
    TEdit *edtATCOffset_10 = new TEdit();
    TEdit *edtATCOffset_11 = new TEdit();
    TEdit *edtATCOffset_12 = new TEdit();
    TEdit *edtATCOffset_13 = new TEdit();
    TEdit *edtATCOffset_14 = new TEdit();
    TEdit *edtATCOffset_15 = new TEdit();
    TEdit *edtATCOffset_16 = new TEdit();
    TEdit *edtATCOffset_17 = new TEdit();
    TEdit *edtATCOffset_25 = new TEdit();
    TEdit *edtATCOffset_26 = new TEdit();
    TEdit *edtATCOffset_18 = new TEdit();
    TEdit *edtATCOffset_19 = new TEdit();
    TEdit *edtATCOffset_27 = new TEdit();
    TEdit *edtATCOffset_28 = new TEdit();
    TEdit *edtATCOffset_20 = new TEdit();
    TEdit *edtATCOffset_21 = new TEdit();
    TEdit *edtATCOffset_29 = new TEdit();
    TEdit *edtATCOffset_30 = new TEdit();
    TEdit *edtATCOffset_22 = new TEdit();
    TEdit *edtATCOffset_23 = new TEdit();
    TEdit *edtATCOffset_31 = new TEdit();
    TEdit *edtATCOffset_32 = new TEdit();
    TEdit *edtATCOffset_24 = new TEdit();
    TGroupBox *grpInPC = new TGroupBox();
    TLabel *lblATCInPC1 = new TLabel();
    TLabel *lblATCInPC2 = new TLabel();
    TLabel *lblATCInPC4 = new TLabel();
    TLabel *lblATCInPC3 = new TLabel();
    TEdit *edtATCInPC1 = new TEdit();
    TEdit *edtATCInPC2 = new TEdit();
    TEdit *edtATCInPC3 = new TEdit();
    TEdit *edtATCInPC4 = new TEdit();
    // OMITTED (GATE G-Image): TImage *Image1; -- TImage has zero port anywhere in this tree.
    TLabel *lblCurr = new TLabel();
    TLabel *Label18 = new TLabel();
    TEdit *edtCurrTemp = new TEdit();
    TCheckBox *chkUseCDAOnly = new TCheckBox();
    TTabSheet *tsLBCtr = new TTabSheet();
    TPanel *Panel22 = new TPanel();
    TCheckBox *chkLBTempFunction = new TCheckBox();
    TGroupBox *grpLBPre = new TGroupBox();
    TPanel *Panel24 = new TPanel();
    TLabel *lblLBTempMin_1 = new TLabel();
    TLabel *lblLBTempMin = new TLabel();
    TEdit *edtLBTempMin = new TEdit();
    TPanel *Panel26 = new TPanel();
    TLabel *lblLBTempOffset_1 = new TLabel();
    TLabel *lblLBTempOffset = new TLabel();
    TEdit *edtLBTempOffset = new TEdit();
    TGroupBox *grpLBInitialOffset = new TGroupBox();
    TPanel *pnlLBInitialOffset1 = new TPanel();
    TLabel *lblIdleTime_LB_1 = new TLabel();
    TLabel *lblIdleTime_LB = new TLabel();
    TEdit *edtIdleTime_LB = new TEdit();
    TPanel *pnlLBInitialOffset2 = new TPanel();
    TLabel *lblBoostOffset_LB_1 = new TLabel();
    TLabel *lblBoostOffset_LB = new TLabel();
    TEdit *edtBoostOffset_LB = new TEdit();
    TPanel *pnlLBInitialOffset3 = new TPanel();
    TLabel *lblBoostDuration_LB_1 = new TLabel();
    TLabel *lblBoostDuration_LB = new TLabel();
    TEdit *edtBoostDuration_LB = new TEdit();
    TPanel *pnlLBInitialOffset4 = new TPanel();
    TLabel *lblPostBoost_LB_1 = new TLabel();
    TLabel *lblPostBoost_LB = new TLabel();
    TEdit *edtPostBoost_LB = new TEdit();
    TTabSheet *tsUnUse = new TTabSheet();
    TGroupBox *grpBoostOffset = new TGroupBox();
    TPanel *pnlBoostOffset1 = new TPanel();
    TLabel *lblBoostOffset2 = new TLabel();
    TLabel *lblBoostOffset = new TLabel();
    TEdit *edtBoostTempMin = new TEdit();
    TPanel *pnlBoostOffset2 = new TPanel();
    TLabel *lblBoostOffset3 = new TLabel();
    TLabel *lblBoostOffset1 = new TLabel();
    TEdit *edtBoostOffset = new TEdit();
    TPanel *pnlBoostOffset = new TPanel();
    TCheckBox *chkBoostOffset = new TCheckBox();
    TPanel *pnlLBInitialOffset = new TPanel();
    TCheckBox *chkLBInitOffset = new TCheckBox();
    TfTemp_SetTagButton *btnSort = new TfTemp_SetTagButton();   // Tag used, see DESIGN NOTE above
    TPanel *Panel30 = new TPanel();
    TCheckBox *rbATCActiveOn = new TCheckBox();
    TGroupBox *grpRS232 = new TGroupBox();
    TCheckBox *rbTemperatureSetOn = new TCheckBox();
    TCheckBox *cbATCReferTempSensor = new TCheckBox();
    TSpeedButton *sbSafeTestATC = new TSpeedButton();
    TGroupBox *gbFileName = new TGroupBox();
    TEdit *edAtcFileName = new TEdit();
    TComboBox *cbbATC_RecipeFile = new TComboBox();
    TRadioButton *RaWriteATCFile = new TRadioButton();
    TRadioButton *RachoiceATCFile = new TRadioButton();
    TPanel *Panel33 = new TPanel();
    TLabel *Label8 = new TLabel();
    TLabel *Label23 = new TLabel();
    TEdit *edtLBTimeOut = new TEdit();
    TPanel *Panel38 = new TPanel();
    TCheckBox *cbTestStartToNextTestStart = new TCheckBox();
    TEdit *iTeststartToNextTestStart = new TEdit();
    TEdit *edtInitialDelay_10 = new TEdit();
    TLabel *lblFT = new TLabel();
    TLabel *lblRT = new TLabel();
    TEdit *edtInitialDelay_1_RT = new TEdit();
    TEdit *edtInitialDelay_2_RT = new TEdit();
    TEdit *edtInitialDelay_3_RT = new TEdit();
    TEdit *edtInitialDelay_5_RT = new TEdit();
    TEdit *edtInitialDelay_4_RT = new TEdit();
    TEdit *edtInitialDelay_6_RT = new TEdit();
    TEdit *edtInitialDelay_7_RT = new TEdit();
    TEdit *edtInitialDelay_9_RT = new TEdit();
    TEdit *edtInitialDelay_8_RT = new TEdit();
    TEdit *edtInitialDelay_10_RT = new TEdit();
    TPanel *Panel3 = new TPanel();
    TPanel *Panel12 = new TPanel();
    TPanel *Panel23 = new TPanel();
    TPanel *Panel9 = new TPanel();
    TPanel *Panel37 = new TPanel();
    TGroupBox *gbATCPackageOffset = new TGroupBox();
    TLabel *Label24 = new TLabel();
    TLabel *Label25 = new TLabel();
    TLabel *Label32 = new TLabel();
    TEdit *edtATC_PackageOffset_01 = new TEdit();
    TEdit *edtATC_PackageOffset_02 = new TEdit();
    TEdit *edtATC_PackageOffset_03 = new TEdit();
    TEdit *edtATC_PackageTemp_03 = new TEdit();
    TEdit *edtATC_PackageTemp_02 = new TEdit();
    TEdit *edtATC_PackageTemp_01 = new TEdit();
    TGroupBox *grpTJTempRange = new TGroupBox();
    TEdit *edTJTempRange_High = new TEdit();
    TLabel *lbTJTempRange_High = new TLabel();
    TEdit *edTJTempRange_Low = new TEdit();
    TLabel *lbTJTempRange_Low = new TLabel();
    TGroupBox *gb_HotGunFlow = new TGroupBox();
    TLabel *Label29 = new TLabel();
    TLabel *Label33 = new TLabel();
    TEdit *edtHotGunFLowLimit_H = new TEdit();
    TEdit *edtHotGunFLowLimit_L = new TEdit();
    TPanel *pnlthreshold = new TPanel();
    TLabel *lblthreshold_1 = new TLabel();
    TLabel *lblthreshold = new TLabel();
    TEdit *edtThreshold = new TEdit();
    TPanel *PnlDewPointAlarm = new TPanel();
    TLabel *Label34 = new TLabel();
    TLabel *Label35 = new TLabel();
    TLabel *Label36 = new TLabel();
    TLabel *Label37 = new TLabel();
    TCheckBox *cbWaitDewPoint = new TCheckBox();
    TEdit *edDewPointAlarmInterval = new TEdit();
    TEdit *edDewPointRange = new TEdit();
    TTabSheet *tsATC_FFC = new TTabSheet();
    TPanel *Panel131 = new TPanel();
    TLabel *lbIndiVidualTemperArm2Enable = new TLabel();
    TLabel *lbIndiVidualTemperArm2TimeOn = new TLabel();
    TLabel *lbIndiVidualTemperArm2TimeOff = new TLabel();
    TLabel *lbIndiVidualTemperArm2Offset = new TLabel();
    TLabel *Label729 = new TLabel();
    TLabel *Label730 = new TLabel();
    TLabel *Label731 = new TLabel();
    TLabel *Label732 = new TLabel();
    TLabel *Label733 = new TLabel();
    TLabel *Label734 = new TLabel();
    TLabel *Label735 = new TLabel();
    TLabel *Label736 = new TLabel();
    TLabel *Label737 = new TLabel();
    TLabel *Label738 = new TLabel();
    TLabel *Label739 = new TLabel();
    TLabel *Label740 = new TLabel();
    TLabel *Label741 = new TLabel();
    TLabel *Label742 = new TLabel();
    TLabel *Label743 = new TLabel();
    TLabel *Label744 = new TLabel();
    TLabel *Label745 = new TLabel();
    TLabel *Label746 = new TLabel();
    TLabel *Label747 = new TLabel();
    TLabel *Label748 = new TLabel();
    TLabel *Label749 = new TLabel();
    TLabel *Label750 = new TLabel();
    TLabel *Label751 = new TLabel();
    TLabel *Label752 = new TLabel();
    TLabel *Label753 = new TLabel();
    TLabel *Label754 = new TLabel();
    TLabel *Label755 = new TLabel();
    TLabel *Label756 = new TLabel();
    TLabel *Label757 = new TLabel();
    TLabel *Label758 = new TLabel();
    TPanel *Panel132 = new TPanel();
    TCheckBox *edFFC_Arm2Enable_01 = new TCheckBox();
    TCheckBox *edFFC_Arm2Enable_02 = new TCheckBox();
    TCheckBox *edFFC_Arm2Enable_03 = new TCheckBox();
    TCheckBox *edFFC_Arm2Enable_04 = new TCheckBox();
    TCheckBox *edFFC_Arm2Enable_05 = new TCheckBox();
    TCheckBox *edFFC_Arm2Enable_06 = new TCheckBox();
    TCheckBox *edFFC_Arm2Enable_07 = new TCheckBox();
    TCheckBox *edFFC_Arm2Enable_08 = new TCheckBox();
    TCheckBox *edFFC_Arm2Enable_09 = new TCheckBox();
    TCheckBox *edFFC_Arm2Enable_10 = new TCheckBox();
    TEdit *edFFC_Arm2TimeOn_01 = new TEdit();
    TEdit *edFFC_Arm2TimeOn_02 = new TEdit();
    TEdit *edFFC_Arm2TimeOff_01 = new TEdit();
    TEdit *edFFC_Arm2TimeOff_02 = new TEdit();
    TEdit *edFFC_Arm2Offset_01 = new TEdit();
    TEdit *edFFC_Arm2Offset_02 = new TEdit();
    TEdit *edFFC_Arm2TimeOn_03 = new TEdit();
    TEdit *edFFC_Arm2TimeOff_03 = new TEdit();
    TEdit *edFFC_Arm2Offset_03 = new TEdit();
    TEdit *edFFC_Arm2Offset_04 = new TEdit();
    TEdit *edFFC_Arm2TimeOff_04 = new TEdit();
    TEdit *edFFC_Arm2TimeOn_04 = new TEdit();
    TEdit *edFFC_Arm2TimeOn_05 = new TEdit();
    TEdit *edFFC_Arm2TimeOff_05 = new TEdit();
    TEdit *edFFC_Arm2Offset_05 = new TEdit();
    TEdit *edFFC_Arm2Offset_06 = new TEdit();
    TEdit *edFFC_Arm2TimeOff_06 = new TEdit();
    TEdit *edFFC_Arm2TimeOn_06 = new TEdit();
    TEdit *edFFC_Arm2TimeOn_07 = new TEdit();
    TEdit *edFFC_Arm2TimeOn_08 = new TEdit();
    TEdit *edFFC_Arm2TimeOn_09 = new TEdit();
    TEdit *edFFC_Arm2TimeOn_10 = new TEdit();
    TEdit *edFFC_Arm2TimeOff_10 = new TEdit();
    TEdit *edFFC_Arm2TimeOff_09 = new TEdit();
    TEdit *edFFC_Arm2TimeOff_08 = new TEdit();
    TEdit *edFFC_Arm2TimeOff_07 = new TEdit();
    TEdit *edFFC_Arm2Offset_10 = new TEdit();
    TEdit *edFFC_Arm2Offset_09 = new TEdit();
    TEdit *edFFC_Arm2Offset_08 = new TEdit();
    TEdit *edFFC_Arm2Offset_07 = new TEdit();
    TButton *btnSameAsArm1 = new TButton();
    TPanel *Panel129 = new TPanel();
    TLabel *lbIndiVidualTemperArm1Enable = new TLabel();
    TLabel *lbIndiVidualTemperArm1TimeOn = new TLabel();
    TLabel *lbIndiVidualTemperArm1TimeOff = new TLabel();
    TLabel *lbIndiVidualTemperArm1Offset = new TLabel();
    TLabel *Label699 = new TLabel();
    TLabel *Label700 = new TLabel();
    TLabel *Label701 = new TLabel();
    TLabel *Label702 = new TLabel();
    TLabel *Label703 = new TLabel();
    TLabel *Label704 = new TLabel();
    TLabel *Label705 = new TLabel();
    TLabel *Label706 = new TLabel();
    TLabel *Label707 = new TLabel();
    TLabel *Label708 = new TLabel();
    TLabel *Label709 = new TLabel();
    TLabel *Label710 = new TLabel();
    TLabel *Label711 = new TLabel();
    TLabel *Label712 = new TLabel();
    TLabel *Label713 = new TLabel();
    TLabel *Label714 = new TLabel();
    TLabel *Label715 = new TLabel();
    TLabel *Label716 = new TLabel();
    TLabel *Label717 = new TLabel();
    TLabel *Label718 = new TLabel();
    TLabel *Label719 = new TLabel();
    TLabel *Label720 = new TLabel();
    TLabel *Label721 = new TLabel();
    TLabel *Label722 = new TLabel();
    TLabel *Label723 = new TLabel();
    TLabel *Label724 = new TLabel();
    TLabel *Label725 = new TLabel();
    TLabel *Label726 = new TLabel();
    TLabel *Label727 = new TLabel();
    TLabel *Label728 = new TLabel();
    TPanel *Panel130 = new TPanel();
    TCheckBox *edFFC_Arm1Enable_01 = new TCheckBox();
    TCheckBox *edFFC_Arm1Enable_02 = new TCheckBox();
    TCheckBox *edFFC_Arm1Enable_03 = new TCheckBox();
    TCheckBox *edFFC_Arm1Enable_04 = new TCheckBox();
    TCheckBox *edFFC_Arm1Enable_06 = new TCheckBox();
    TCheckBox *edFFC_Arm1Enable_05 = new TCheckBox();
    TCheckBox *edFFC_Arm1Enable_07 = new TCheckBox();
    TCheckBox *edFFC_Arm1Enable_09 = new TCheckBox();
    TCheckBox *edFFC_Arm1Enable_08 = new TCheckBox();
    TCheckBox *edFFC_Arm1Enable_10 = new TCheckBox();
    TEdit *edFFC_Arm1TimeOn_07 = new TEdit();
    TEdit *edFFC_Arm1TimeOff_01 = new TEdit();
    TEdit *edFFC_Arm1Offset_01 = new TEdit();
    TEdit *edFFC_Arm1TimeOn_06 = new TEdit();
    TEdit *edFFC_Arm1TimeOff_02 = new TEdit();
    TEdit *edFFC_Arm1Offset_02 = new TEdit();
    TEdit *edFFC_Arm1TimeOn_08 = new TEdit();
    TEdit *edFFC_Arm1TimeOff_03 = new TEdit();
    TEdit *edFFC_Arm1Offset_03 = new TEdit();
    TEdit *edFFC_Arm1TimeOn_10 = new TEdit();
    TEdit *edFFC_Arm1TimeOff_04 = new TEdit();
    TEdit *edFFC_Arm1Offset_04 = new TEdit();
    TEdit *edFFC_Arm1TimeOn_09 = new TEdit();
    TEdit *edFFC_Arm1TimeOff_05 = new TEdit();
    TEdit *edFFC_Arm1Offset_05 = new TEdit();
    TEdit *edFFC_Arm1TimeOn_02 = new TEdit();
    TEdit *edFFC_Arm1TimeOff_06 = new TEdit();
    TEdit *edFFC_Arm1Offset_06 = new TEdit();
    TEdit *edFFC_Arm1TimeOn_01 = new TEdit();
    TEdit *edFFC_Arm1TimeOff_07 = new TEdit();
    TEdit *edFFC_Arm1Offset_07 = new TEdit();
    TEdit *edFFC_Arm1TimeOn_03 = new TEdit();
    TEdit *edFFC_Arm1TimeOff_08 = new TEdit();
    TEdit *edFFC_Arm1Offset_08 = new TEdit();
    TEdit *edFFC_Arm1TimeOn_05 = new TEdit();
    TEdit *edFFC_Arm1TimeOff_09 = new TEdit();
    TEdit *edFFC_Arm1Offset_09 = new TEdit();
    TEdit *edFFC_Arm1TimeOn_04 = new TEdit();
    TEdit *edFFC_Arm1TimeOff_10 = new TEdit();
    TEdit *edFFC_Arm1Offset_10 = new TEdit();
    TCheckBox *cbFFCOffsetEnable = new TCheckBox();
    TGroupBox *grpChamberBoost = new TGroupBox();
    TLabel *lblChamberBoostTime = new TLabel();
    TLabel *lblChamberBoostOffset = new TLabel();
    TCheckBox *chkChamberBoost = new TCheckBox();
    TEdit *edtChamberBoostTime = new TEdit();
    TEdit *edtChamberBoostOffset = new TEdit();
    TTabSheet *tsSigmaForTemp = new TTabSheet();
    TGroupBox *gbcb3SigmaTempMonitior = new TGroupBox();
    TLabel *Label803 = new TLabel();
    TLabel *Label804 = new TLabel();
    TCheckBox *cb3SigmaTempMonitior_Enable = new TCheckBox();
    TEdit *edt3SigmaTempMonitior_SetCount = new TEdit();
    TEdit *edt3SigmaTempMonitior_Set3xSigmaValue = new TEdit();
    TCheckBox *cbATCHeaterGun = new TCheckBox();
    TPanel *pnlCover = new TPanel();
    TGroupBox *gbHeatGunTempATC = new TGroupBox();
    TLabel *Label38 = new TLabel();
    TEdit *edtHeatGunTempATC = new TEdit();
    TCheckBox *chkTempCalByRecipe = new TCheckBox();
    TPanel *palLBAir = new TPanel();
    TLabel *labLB1 = new TLabel();
    TLabel *labLB3 = new TLabel();
    TLabel *labLB2 = new TLabel();
    TLabel *labLB4 = new TLabel();
    TCheckBox *cbBlowLB = new TCheckBox();
    TEdit *edLBTempAlarmTime = new TEdit();
    TEdit *edtLBAirOnTemp = new TEdit();
    TGroupBox *grpATC_HotAir = new TGroupBox();
    TLabel *Label39 = new TLabel();
    TLabel *Label40 = new TLabel();
    TEdit *edtATC_HotGunTemp = new TEdit();
    TEdit *edtATC_HotGunTime = new TEdit();
    TCheckBox *chkSLKNoHeatUp = new TCheckBox();
    TCheckBox *cbByPassChamber = new TCheckBox();
    TRadioGroup *rgBasePoint = new TRadioGroup();
    TRadioButton *rbTemp = new TRadioButton();
    TGroupBox *grpTJFunction = new TGroupBox();
    TLabel *Label822 = new TLabel();
    TLabel *Label824 = new TLabel();
    TCheckBox *cbEnableTJFunction = new TCheckBox();
    TEdit *edtSetTJ_Offset = new TEdit();
    TEdit *edtSetTJ_Slope = new TEdit();
    TCheckBox *cbTjWatchdog = new TCheckBox();
    TLabel *Label41 = new TLabel();
    TEdit *edtDelayAfterSOT = new TEdit();
    TLabel *Label42 = new TLabel();
    TEdit *edtInputVLow = new TEdit();
    TEdit *edtInputVHigh = new TEdit();
    //AI(W906-FW-TEMP2) 20260820: dims from golden uTemp_Set.dfm (ColCount=17,
    // RowCount=2) -- golden streamed them; the default-ctor grid was 5x5 and
    // DoIniDataToForm's Cells[TjMap] writes threw vector::at out_of_range at
    // wb_serve e2e (gdb bt). Same ctor-dims convention as fBinSel.h:576.
    TStringGrid *sgTjMap = new TStringGrid(/*cols*/17, /*rows*/2);
    TRadioGroup *rgTjMapType = new TRadioGroup();
    TRadioGroup *rgATC60AirOn = new TRadioGroup();
    TLabel *labReadyTempRange = new TLabel();
    TEdit *edTempReadyRange = new TEdit();
    TGroupBox *gbMultiZone = new TGroupBox();
    TCheckBox *cbMultiZoneFunction = new TCheckBox();
    TCheckBox *cbZoneEnable1 = new TCheckBox();
    TEdit *edZoneSetting1 = new TEdit();
    TCheckBox *cbZoneEnable2 = new TCheckBox();
    TEdit *edZoneSetting2 = new TEdit();
    TCheckBox *cbZoneEnable3 = new TCheckBox();
    TEdit *edZoneSetting3 = new TEdit();
    TCheckBox *cbZoneEnable4 = new TCheckBox();
    TEdit *edZoneSetting4 = new TEdit();
    TTabSheet *tsOthers = new TTabSheet();
    TEdit *edLBTempHighSettingValue = new TEdit();
    TEdit *edLBTempLowSettingValue = new TEdit();
    TEdit *edATCOfsTime = new TEdit();
    TTabSheet *tsTriTempSet = new TTabSheet();
    TGroupBox *gbDefrostFunction = new TGroupBox();
    TGroupBox *GroupBox3 = new TGroupBox();
    //AI(W906-FW-TEMP2) 20260820: dims from golden uTemp_Set.dfm (ColCount=2,
    // RowCount=7, FixedCols=0) -- same streamed-dims gap as sgTjMap above.
    TStringGrid *sgDefrostStatus = new TStringGrid(/*cols*/2, /*rows*/7);
    TGroupBox *GroupBox4 = new TGroupBox();
    TButton *btnDefrostStart = new TButton();
    TButton *btnDefrostEnd = new TButton();
    TComboBox *cbSelectDevice = new TComboBox();
    TGroupBox *GroupBox5 = new TGroupBox();
    TButton *btn_DefrostAllUseStart = new TButton();
    TButton *btn_DefrostAllUseEnd = new TButton();
    TGroupBox *GroupBox6 = new TGroupBox();
    TLabel *Label43 = new TLabel();
    TCheckBox *cbEnableTesterDryAirControl = new TCheckBox();
    TEdit *edt_Defrost_Time_Too_Lower = new TEdit();
    TLabel *lblTempBase = new TLabel();
    TEdit *edAbLow = new TEdit();
    TEdit *edAbMid = new TEdit();
    TEdit *edLowBase = new TEdit();
    TEdit *edMidBase = new TEdit();
    TEdit *edHighBase = new TEdit();
    TEdit *edSHighBase = new TEdit();
    TfTemp_SetTimer *tmr_ATC_Deforst = new TfTemp_SetTimer();
    TGroupBox *grpIndexAirStream = new TGroupBox();
    TLabel *Label63 = new TLabel();
    TLabel *Label64 = new TLabel();
    TLabel *Label47 = new TLabel();
    TLabel *Label66 = new TLabel();
    TfTemp_SetTagEdit *edt_SetIndexAirstreamTemp = new TfTemp_SetTagEdit();   // Tag used, see DESIGN NOTE above
    TEdit *edt_SetAirstreamTemperatureRang_Index = new TEdit();
    TCheckBox *chk_EnableArm_1_Air = new TCheckBox();
    TCheckBox *chk_EnableArm_2_Air = new TCheckBox();
    TEdit *edt_AirVolumeLmt_Index = new TEdit();
    TGroupBox *grpSocketAirStream = new TGroupBox();
    TLabel *Label577 = new TLabel();
    TLabel *Label578 = new TLabel();
    TLabel *Label48 = new TLabel();
    TLabel *Label67 = new TLabel();
    TfTemp_SetTagEdit *edtSetTempature2AirMachine = new TfTemp_SetTagEdit();   // Tag used, see DESIGN NOTE above
    TEdit *edt_SetAirstreamTemperatureRang_Socket = new TEdit();
    TCheckBox *cb_EnableAirMachineSocket = new TCheckBox();
    TCheckBox *chk_EnableSocket_Air = new TCheckBox();
    TEdit *edt_AirVolumeLmt_Socket = new TEdit();
    TRadioButton *rb6Point = new TRadioButton();
    TPageControl *PageControl1 = new TPageControl();
    TTabSheet *tsStandard = new TTabSheet();
    TRadioGroup *rgIndexHeatMode = new TRadioGroup();
    TTabSheet *tsTriTemp = new TTabSheet();
    TCheckBox *chkTriTempEnableOutSht = new TCheckBox();
    TCheckBox *chkTriTempEnableIndex = new TCheckBox();
    TCheckBox *chkTriTempEnableBase = new TCheckBox();
    TCheckBox *chk_IndexHeaterDoor = new TCheckBox();
    TGroupBox *gbPowerFollowFuncAct = new TGroupBox();
    TLabel *Label883 = new TLabel();
    TLabel *Label884 = new TLabel();
    TLabel *Label885 = new TLabel();
    TLabel *Label49 = new TLabel();
    TCheckBox *cbPowerFollow_Enable = new TCheckBox();
    TEdit *edtPowerFollower_PFSlope = new TEdit();
    TEdit *edtPowerFollow_FullPower = new TEdit();
    TEdit *edtPowerFollower_WGain = new TEdit();
    TEdit *edtPowerFollower_Many2one = new TEdit();
    TGroupBox *grpTemperatureRise = new TGroupBox();
    TLabel *Label50 = new TLabel();
    TLabel *Label51 = new TLabel();
    TCheckBox *cbATCTempRise = new TCheckBox();
    TEdit *edt_TempRiseTemp = new TEdit();
    TEdit *edt_TempRise_Delay = new TEdit();
    TCheckBox *cbUseTC2Offset = new TCheckBox();
    TCheckBox *chkTempErrorAction = new TCheckBox();
    TGroupBox *gbATC_Type = new TGroupBox();
    TEdit *edtATC_Type = new TEdit();
    TEdit *edContinuousSec = new TEdit();
    TCheckBox *cbLBTempHighAlarmEnable = new TCheckBox();
    TCheckBox *cbLBTempLowAlarmEnable = new TCheckBox();
    TTabSheet *tsATCMultiSensor = new TTabSheet();
    TCheckBox *cbMultiSensorOffsetEnable = new TCheckBox();
    TCheckBox *cbMultiSensorEnable1 = new TCheckBox();
    TCheckBox *cbMultiSensorEnable2 = new TCheckBox();
    TCheckBox *cbMultiSensorEnable3 = new TCheckBox();
    TCheckBox *cbMultiSensorEnable4 = new TCheckBox();
    TPanel *Panel35 = new TPanel();
    TPanel *Panel39 = new TPanel();
    TEdit *edATC_Arm1_S1_TC2 = new TEdit();
    TEdit *edATC_Arm1_S1_TC3 = new TEdit();
    TEdit *edATC_Arm1_S1_TC4 = new TEdit();
    TEdit *edATC_Arm1_S1_TC1 = new TEdit();
    TLabel *Label52 = new TLabel();
    TLabel *Label57 = new TLabel();
    TLabel *Label58 = new TLabel();
    TLabel *Label59 = new TLabel();
    TLabel *Label60 = new TLabel();
    TEdit *edATC_Arm1_S2_TC1 = new TEdit();
    TEdit *edATC_Arm1_S2_TC2 = new TEdit();
    TEdit *edATC_Arm1_S2_TC3 = new TEdit();
    TEdit *edATC_Arm1_S2_TC4 = new TEdit();
    TLabel *Label61 = new TLabel();
    TEdit *edATC_Arm1_S3_TC1 = new TEdit();
    TEdit *edATC_Arm1_S3_TC2 = new TEdit();
    TEdit *edATC_Arm1_S3_TC3 = new TEdit();
    TEdit *edATC_Arm1_S3_TC4 = new TEdit();
    TLabel *Label62 = new TLabel();
    TEdit *edATC_Arm1_S4_TC1 = new TEdit();
    TEdit *edATC_Arm1_S4_TC2 = new TEdit();
    TEdit *edATC_Arm1_S4_TC3 = new TEdit();
    TEdit *edATC_Arm1_S4_TC4 = new TEdit();
    TLabel *Label65 = new TLabel();
    TPanel *Panel40 = new TPanel();
    TLabel *Label68 = new TLabel();
    TLabel *Label69 = new TLabel();
    TLabel *Label70 = new TLabel();
    TLabel *Label71 = new TLabel();
    TLabel *Label72 = new TLabel();
    TLabel *Label73 = new TLabel();
    TLabel *Label74 = new TLabel();
    TLabel *Label75 = new TLabel();
    TPanel *Panel43 = new TPanel();
    TEdit *edATC_Arm2_S1_TC2 = new TEdit();
    TEdit *edATC_Arm2_S1_TC3 = new TEdit();
    TEdit *edATC_Arm2_S1_TC4 = new TEdit();
    TEdit *edATC_Arm2_S1_TC1 = new TEdit();
    TEdit *edATC_Arm2_S2_TC1 = new TEdit();
    TEdit *edATC_Arm2_S2_TC2 = new TEdit();
    TEdit *edATC_Arm2_S2_TC3 = new TEdit();
    TEdit *edATC_Arm2_S2_TC4 = new TEdit();
    TEdit *edATC_Arm2_S3_TC1 = new TEdit();
    TEdit *edATC_Arm2_S3_TC2 = new TEdit();
    TEdit *edATC_Arm2_S3_TC3 = new TEdit();
    TEdit *edATC_Arm2_S3_TC4 = new TEdit();
    TEdit *edATC_Arm2_S4_TC1 = new TEdit();
    TEdit *edATC_Arm2_S4_TC2 = new TEdit();
    TEdit *edATC_Arm2_S4_TC3 = new TEdit();
    TEdit *edATC_Arm2_S4_TC4 = new TEdit();
    TfTemp_SetTimer *tmrDefrost = new TfTemp_SetTimer();   //Ztex 2023.04.19 Add HT-1032 TriTemp Function
};

// golden uTemp_Set.cpp:47 `TfTemp_Set *fTemp_Set;` -- global, ZERO-init
// (NOT `= new TfTemp_Set()`), per this tree's static-init-order rule (18
// NULL-global-pointer precedent). Defined in uTemp_Set.cpp.
extern TfTemp_Set *fTemp_Set;

#endif // FORMS_FTEMP_SET_H
