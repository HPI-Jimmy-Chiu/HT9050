// =============================================================================
//  forms/fContact.h  --  non-VCL facade for golden's TfContact (cContact.h /
//  cContact.cpp) -- the Contact / contact-force / auto-height engineering form
//
//  AI(W906-FW-CONTACT-W26) 20260826: new file, FW wave FW-CONTACT-W26.
//  Golden source of truth: HT9011UC_Code_V3.33.906.0_20260618/cContact.cpp
//  (22,762 lines -- the largest file in the corpus) + cContact.h (671 lines)
//  + cContact.dfm (18,633 lines).  All three are cp950; decoded this wave with
//  `io.open(p, encoding='cp950')`, written out UTF-8, 0 U+FFFD.
//
//  ============================================================================
//  *** SAFETY POSTURE -- READ BEFORE TOUCHING ANY GATE IN THIS FILE ***
//  ============================================================================
//  TfContact is the form that PRESSES ICs INTO SOCKETS.  Its state machines
//  drive MTestY1/MTestY2/MTestZ1/MTestZ2 (the index down-press axes), the
//  FTestSuck/BTestSuck vacuum, the EP air-pressure regulator
//  (ADAM_WriteVoltage), the OTD dock cylinders, and the TTL start/clear lines
//  to the tester.  Of golden's 134 method definitions, this wave delivers 44
//  and gates 90 -- see WAVE SCOPE.
//
//  Every gated method is DECLARED here and DELIBERATELY NOT DEFINED in
//  forms/fContact.cpp.  That makes the LINKER the interlock: the moment any
//  future wave -- or the web write path -- tries to CALL one, the build fails
//  with `undefined reference to TfContact::<method>` naming the exact entry
//  point.  Chosen over `#if 0`-around-a-verbatim-body (the tree's other idiom)
//  for the same reason forms/fMotorTest.cpp chose it: an `#if 0` body is one
//  keystroke from moving an axis, and an empty stub reads to the next person as
//  "already translated".  Neither TfContact nor any type below has a virtual
//  member, so an undefined member function costs nothing at link time until
//  somebody calls it.
//
//  UN-GATING PROTOCOL: translate the body in a wave that owns the interlock
//  story (who may issue it, what must be homed / servo-on / door-closed first,
//  and what re-checks it on the C++ side).  Never as a side effect of
//  "finishing the facade".
//
//  ============================================================================
//  DENOMINATOR -- MEASURED THIS WAVE, NOT INHERITED
//  ============================================================================
//  The brief said "137 處 TfContact::".  Re-measured here: 134.
//    Method: a character-state-machine comment/string stripper (not a regex --
//    golden has `}` inside comments and whole functions wrapped in `/* */`,
//    both of which have silently mis-scoped past waves), then
//    `\bTfContact\s*::\s*(~?\w+)` over the stripped text, then a forward scan
//    from each hit for the first depth-0 `{` (definition) or `;` (declaration).
//      -> 134 occurrences, 134 definitions, 0 declarations, 0 in comments.
//    Brace-matched spans total 22,213 of the file's 22,762 lines.
//  So the denominator for every percentage below is 134 golden METHOD
//  DEFINITIONS in cContact.cpp -- not lines, and not "the form's behaviour",
//  most of which is the gated motion half.
//
//  THIS WAVE: 44 of 134 delivered (32.8% of golden method DEFINITIONS).
//  90 gated.  Both figures re-measured from the FINISHED files at wave close
//  (scripted: strip comments from forms/fContact.cpp, collect every
//  `TfContact::<name>` it actually DEFINES, intersect with the golden span
//  table above) -- not from the plan, because a wave's plan and its output
//  diverge (this one lost 2 methods late; see GATE (X-32)).
//
//  ⚠ BY LINES THE SHARE IS MUCH SMALLER, and quoting only the method count
//  would overstate this wave by ~9x: the 44 delivered bodies are 794 golden
//  lines of 22,213 (3.6%).  The six biggest GATED methods alone
//  (Do_Z1_AutoGetHeight 2,780 / Do_Z2_AutoGetHeight 2,337 /
//  DoTestContactFunction 2,209 / DoZ1PickFromShuttle 1,392 /
//  DoZ2PickFromShuttle 1,373 / Do_ContactTest_32Site 840) are 10,931 lines --
//  13.8x this whole wave.  Quote both numbers or neither.
//
//  ============================================================================
//  *** THE GLOBAL `fContact` IS ALREADY TAKEN -- THIS FILE DOES NOT CLAIM IT ***
//  ============================================================================
//  Golden ends cContact.h:667 with `extern PACKAGE TfContact *fContact;`.
//  This header deliberately declares NO such global, because the port tree
//  already has a live, registered stand-in for that name:
//
//      atester_shims.h:154-250   class TfContactShim { ... };
//      atester_shims.h:251       extern TfContactShim *fContact;
//
//  Measured this wave (20260826, comment- and string-stripped, `#if 0` regions
//  excluded by an #if/#endif depth scan -- a first pass that only skipped
//  full-line `//` comments over-counted, because dozens of port lines carry a
//  `// golden fContact->X` TRAILING comment): the shim has exactly 10 live
//  member surfaces tree-wide --
//      IsRun2DCheck 60, fShow 46, btnTStep 8, btnTStart 8, bSetupStart 8,
//      cbOneTouchAutoContactHight 7, bSetupStep 6, Do_ROILearning 3,
//      InitDoFullViewCheck 2, DoFullViewCheck 2
//  -- plus 46 further distinct members reached only from inside `#if 0`
//  (147 refs).  There are also two SEAM MACROS that stand in for a golden
//  `fContact->` call without naming the pointer at all
//  (csystem.cpp:5133 `W7C1_FCONTACT_READFILE`, csystem.cpp:6424
//  `W7C2_FCONTACT_RGHANDLER_ENABLE`), and a THIRD, independent mirror --
//  `W5FA_TfContactExt W5FA_FContact;` (Automation/auto9045.cpp:319), a TU-local
//  struct that auto9045.cpp reads instead of `fContact->edAirForceN` etc.
//
//  Declaring `extern TfContact *fContact;` here would therefore be a hard
//  conflict in any TU that sees both headers, and silently repointing the
//  global is a CONSOLIDATION decision (which of the 10 live members change
//  answer, and how the 90 gates below interact with 46 dead ones) that belongs
//  to a wave that owns atester_shims.{h,cpp} -- files outside this wave's write
//  boundary.  So: the CLASS lands here, the POINTER does not.  Nothing in the
//  tree can reach this class yet, which is the intended state.
//
//  ⚠ Corollary for whoever does that consolidation: `TfContactShim::
//  DoFullViewCheck()` returns offline `true` ("complete"), while golden's real
//  DoFullViewCheck (cContact.cpp:15069-15162) is a 7-state RTC vision
//  handshake that drives MTestZ1/MTestY1.  It is GATE (S-14) below.  Swapping
//  the pointer without deciding that case turns "AutoClean's pre-clean full-view
//  check always passes" into "undefined reference", which is the SAFE failure --
//  but it is a build break, not a no-op, so plan for it.
//
//  ============================================================================
//  BATCH CRITERION FOR THIS WAVE (why these 46 and not others)
//  ============================================================================
//  A method is ACTIVE here only if ALL of the following hold, each checked by
//  reading the FULL golden body (never by name):
//    1. It issues no machine action -- no MOT[]/Gali_* move or position-define,
//       no Cylinder[], no SW[] output, no TMySucker, no Home, no servo.
//    2. It persists nothing -- no WriteIniData, no fopen/CreateFile, no
//       Save*/SaveToFile.  ⚠ `CheckAndReadIniData` COUNTS AS A WRITE in this
//       tree: common.cpp:602-603 WriteInteger()s the default back when the key
//       is absent.  Six such calls in golden's constructor are GATE (C-1).
//    3. It sends nothing outbound -- no SECS EventReport, no TCP, no GPIB.
//    4. Every symbol it needs already exists in the port tree.  No shim is
//       invented for a cross-file gap; the gap is recorded below instead.
//  Everything left is display, current-value read, UI-state toggle, pure
//  predicate, string assembly, or arithmetic.  Calculation is in scope;
//  writing the result anywhere durable is not.
//
//  TRANSITIVE RULE, applied strictly: a method that CALLS a gated method is
//  itself gated.  That is what removes scrbSLKChange / rgKitDiameterClick /
//  chkUseAddWeightClick (they reach edAirForceChange -> ADAM_WriteVoltage,
//  i.e. they command the EP air-pressure regulator) and the four
//  ShowArmAndDeviceForce consumers.
//
//  ============================================================================
//  WAVE SCOPE -- ACTIVE (44).  Each was read line-by-line in full first.
//  ============================================================================
//    TfContact ctor            (PORT split)      fields + .dfm values only
//    Init()                    golden :107-276   ACTIVE + GATE (C-1)(C-2)
//    ~TfContact                golden :279-281   ACTIVE (empty) + port dtor
//    InitContactCbList         golden :352-354   ACTIVE (golden body is empty)
//    InitContactEdtList        golden :356-361   ACTIVE (golden body is 3 commented lines)
//    edDropWaitTimeKeyPressInt golden :1809-1813 ACTIVE
//    DutCount                  golden :1959-2072 ACTIVE + GATE (C-3)
//    edPinCountKeyPress        golden :2083-2087 ACTIVE
//    btnTStartClick            golden :2251-2254 ACTIVE
//    btnTStepClick             golden :2256-2259 ACTIVE
//    Inital_ZTask              golden :2275-2278 ACTIVE
//    InitCarlibrationTask      golden :11748-11751 ACTIVE
//    cbContactModeChange       golden :13977-14062 ACTIVE
//    sbtExitClick              golden :14478-14487 ACTIVE
//    InitROILearningTask       golden :14490-14494 ACTIVE
//    coD41Change               golden :15049-15061 ACTIVE
//    InitDoFullViewCheck       golden :15064-15067 ACTIVE
//    edContactOffsetArm1MouseDown golden :15164-15168 ACTIVE
//    edD41MouseDown            golden :15321-15328 ACTIVE
//    ChangeContactMode         golden :15436-15454 ACTIVE
//    edShtPickOffset1MouseDown golden :16867-16874 ACTIVE
//    labMPaDblClick            golden :16943-16948 ACTIVE
//    CheckContactSettingChange golden :17054-17094 ACTIVE (see G-DimCmp)
//    cbEnableUKClick           golden :17139-17182 ACTIVE
//    edDropByPassDetectMouseDown golden :17189-17193 ACTIVE
//    ResetDMCTrayData          golden :18339-18348 ACTIVE
//    edtPinOfDieMouseDown      golden :18350-18355 ACTIVE
//    edDoubleForceMouseDown    golden :18357-18369 ACTIVE
//    CountDieForceKg           golden :18371-18394 ACTIVE
//    edAutoKSHTReleaseOfsMouseDown golden :18396-18400 ACTIVE
//    edForcePerDeviceKGClick   golden :18408-18423 ACTIVE
//    edtPinOfDieKeyPress       golden :18425-18428 ACTIVE (golden body is `return;`)
//    edForcePerPinNMouseDown   golden :18542-18563 ACTIVE (see D-7 -- Tag provenance)
//    GotIndexYSpeed            golden :18663-18667 ACTIVE
//    GotIndexZSpeed            golden :18669-18673 ACTIVE
//    edForcePerPinGMouseDown   golden :19029-19050 ACTIVE (see D-7)
//    rgDieForceKitDiameterClick golden :19503-19506 ACTIVE
//    InitRTCAutoTuning         golden :19853-19856 ACTIVE
//    cbRTCAutoTuningClick      golden :20365-20378 ACTIVE
//    IsRun2DCheck              golden :21203-21219 ACTIVE
//    Inital_Z1PickFromSocketTask golden :21221-21224 ACTIVE
//    timerContactTimer         golden :21355-21366 ACTIVE (see G-NoElse)
//    ShowMSG                   golden :21368-21378 ACTIVE
//    UpdateContactRelative     golden :22315-22323 ACTIVE (see D-8 -- const_cast)
//    InitChkInSHLatchTask      golden :22608-22615 ACTIVE
//
//  ============================================================================
//  GATED -- MOTION / VACUUM / IO / SAFETY-CRITICAL (32).
//  Each entry names the exact golden call that makes it unsafe.
//  ============================================================================
//   (S-01) TestZ1Y2OutRandge      golden :283-306   MOT[MTestZ1].Gali_ReadPos/Gali_ReadEncoderPos
//   (S-02) TestZ1Y2SetPos         golden :308-315   Gali_Command("DP,...") -- REDEFINES the axis origin
//   (S-03) TestZ2Y1OutRandge      golden :317-341   as (S-01)
//   (S-04) TestZ2Y1SetPos         golden :343-350   as (S-02)
//       ⚠ (S-01)/(S-03) are pure READS and would pass criterion 1 on their
//       face.  They are gated anyway for a specific reason: in THIS tree
//       `TMyMotor::Gali_ReadPos()` is the degraded stub `{ return 0; }`
//       (Motor/mymotor.cpp:1525, whole Galil layer TODO(W6-Galil),
//       Motor/mymotor.h:22-24).  Delivering them would make an
//       "is the axis out of range?" safety predicate answer a confident,
//       silent `false` on every call.  A missing symbol is a better failure
//       than a wrong `false`.  They are also the read half of the (S-02)/(S-04)
//       origin-redefine pair; splitting the pair invites someone to wire it.
//   (S-05) DoZ1PickFromShuttle    golden :2487-3878   index Z/Y moves + test suck
//   (S-06) DoZ2PickFromShuttle    golden :3880-5252   ditto
//   (S-07) Do_Z1_AutoGetHeight    golden :5389-8168   auto-height press sequence
//   (S-08) Do_Z2_AutoGetHeight    golden :8174-10510  ditto
//   (S-09) Do_ContactTest_32Site  golden :10513-11352 32-site contact press
//   (S-10) DoZPlaceToShuttle      golden :11355-11739 index place-to-shuttle
//   (S-11) DoTestContactFunction  golden :11755-13963 the contact-test master SM
//   (S-12) btnStartClick          golden :13965-13970 fMain->BtnStartClick -- STARTS THE MACHINE
//   (S-13) btnPauseClick          golden :13972-13975 fMain->BtnPauseClick
//   (S-14) DoFullViewCheck        golden :15069-15162 MTestZ1/MTestY1 moves + COM2 vision
//   (S-15) palOTD_4Click          golden :15170-15193 Cylinder[C_DockYAxisOn].On()
//   (S-16) palOTD_6Click          golden :15195-15218 Cylinder[C_DockXAxisOn].On()
//   (S-17) OTDTimerTimer          golden :15220-15282 SW[SwUnDock]/SW[SwDockError] outputs
//   (S-18) Do_AutoContactTest     golden :15458-15876 auto contact-test SM
//   (S-19) DoStepContactLoadDevice golden :15878-16405 in-arm pick/place
//   (S-20) DoStepContactUnloadDevice golden :16407-16851 in-arm pick/place
//   (S-21) btnIndexArmJogMove_UpClick   golden :16950-17000 JOGS THE INDEX ARM
//   (S-22) btnIndexArmJogMove_DownClick golden :17002-17052 JOGS THE INDEX ARM
//   (S-23) Do_LoadCellAutoHigh    golden :17197-17997 load-cell auto-height press
//   (S-24) DoDeviceMapCheck       golden :17999-18337 device-map press sequence
//   (S-25) AutoZTeachButtonClick  golden :18430-18451 bInoutArmTeach() -- teach motion
//   (S-26) ATC_SwitchTjSignal     golden :18565-18626 ATC Tj signal + arm-down
//   (S-27) SetIndexDownPos        golden :18628-18661 -- **RETIRED 20260923**
//       (AI(W906-CT-READFILE), body in forms/fContact.cpp).  Re-read line by
//       line it is not motion: a four-arm if/else assigning the global double
//       fIndexDownPos from cContact.h:175-177's three constants.  ReadFile
//       calls it first (golden :371) because the result is the upper clamp of
//       its own ReadWriteIni calls at :404/:408/:420/:421.
//   (S-28) TransformDMCMap        golden :19052-19062 MOT[MMTrayY].SetTraySiteMap -- rewrites the tray map
//   (S-29) DoIndecxCHECkFunction  golden :19067-19224 index-check SM
//   (S-30) DoSocketSensorCheckRemainIC golden :19235-19275 socket residual-IC probe
//   (S-31) DoArm1PlaceToShuttle   golden :19277-19382 index arm place
//   (S-32) DoArm2PlaceToShuttle   golden :19384-19501 index arm place
//   (S-33) ClearTTLSignal         golden :19508-19521 SW[TTL_StartData[i]].Off() + SwClear0/4 pulse
//   (S-34) DoContactDeviceLoopTest golden :19525-19695 loop-test SM
//   (S-35) DoContactKTemperatureTest golden :19699-19851 K-temp index move SM
//   (S-36) DoRTCAutoTuning        golden :19859-20363 RTC auto-tune SM (moves index)
//   (S-37) Do2DIDMapCheck         golden :20380-20812 2DID map-check SM
//   (S-38) Do2DIDMapCheckLoadDevice   golden :20814-20882 ditto
//   (S-39) Do2DIDMapCheckUnloadDevice golden :20884-21176 ditto
//   (S-40) DoZ1PickFromSocket     golden :21227-21353 index pick from socket
//   (S-41) DoStepContactKitDevice golden :21431-21804 daily-correlation kit load
//   (S-42) DoStepContactUnKitDevice golden :21807-22313 daily-correlation kit unload
//   (S-43) DoCalibrateAboveHeightZ1 golden :22326-22461 above-height calibration move
//   (S-44) DoCalibrateAboveHeightZ2 golden :22463-22606 ditto
//   (S-45) CheckInShuttleSensor_Latch_Contact golden :22617-22760 in-shuttle latch probe
//   (S-46) Do_ROILearning         golden :14497-15010 RTC ROI learn (drives Z1/Z2 + arms)
//   (S-47) edAirForceChange       golden :2268-2273  ADAM_WriteVoltage(DeviceForm.dPress)
//       -- i.e. it COMMANDS THE EP AIR-PRESSURE REGULATOR.  This is the single
//       most-reached gate in the file: three otherwise-clean UI handlers become
//       gates purely because they call it (see S-48..S-50).
//   (S-48) scrbSLKChange          golden :2074-2081  -> edAirForceChange (S-47)
//   (S-49) rgKitDiameterClick     golden :15284-15319 -> edAirForceChange (S-47)
//   (S-50) chkUseAddWeightClick   golden :17096-17100 -> edAirForceChange (S-47)
//   (S-51) TestZ_CompensationHight golden :18453-18540 writes iTotalOffset_1/2 (down-press compensation)
//   (S-52) RestoreData_2D_Check   golden :21178-21201 InArmSuck/FLCarryKit/BLCarryKit ClearAll()
//       -- clears the IC-tracking state for three real kit-suck objects.  Not a
//       motion command, but corrupting residual-IC tracking is how double-IC
//       and JAM0202-class events happen; out of a read-only wave by definition.
//
//  ============================================================================
//  GATED -- WRITES A FILE / PERSISTS (4)
//  ============================================================================
//   (W-01) ReadFile          golden :363-718   356L.
//       **RETIRED 20260923** -- AI(W906-CT-READFILE), body in forms/fContact.cpp.
//       The original entry said "in-scope for a later wave -- but only after
//       someone splits the write-back out".  That precondition is SUPERSEDED.
//       Four ReadFile translations landed this week (TfSetup, TfHotPlate,
//       TfYieldMonitoring, TfOffSet) and every one of them kept
//       CheckAndReadIniData's write-back VERBATIM, on the rule that splitting
//       it invents a behaviour the machine has never had.  Splitting is a
//       behaviour change and therefore the user's decision, not the
//       translator's.  This entry follows the same treatment.
//       ⚠ It still writes -- 11 explicit writes, including the index contact
//       height (golden :404/:408/:420/:421, "0.0 會撞機").  See the banner on
//       the body.
//   (W-02) DoIniDataToForm   golden :720-1076  357L, same shape as (W-01).
//   (W-03) spbSaveClick      golden :14072-14201 -> SaveSetupFile
//   (W-04) SaveSetupFile     golden :14205-14476 writes the job/setup file
//
//  ============================================================================
//  GATED -- MISSING PORT-SIDE CAPABILITY (52 methods, grouped by what is absent).
//  Not motion; simply not translatable today without inventing a cross-file
//  shim, which task rule 5 forbids.
//  ============================================================================
//   (X-01) CalculateTotalAirForce  golden :18675-18891 (217L)
//       ⚠ REWRITTEN BY AI(W906-FW-CONTACT-W28) 20260826.  The version of this
//       entry written at FW-CONTACT-W26 said "pure arithmetic ... the ONLY
//       thing blocking five otherwise-clean read-only methods", and predicted
//       that porting it into cContact.{h,cpp} would drop all six from this
//       list together.  W28 did that port.  ALL SIX ARE STILL GATED, and the
//       old prediction was wrong on both halves.  Both corrections are kept
//       visible rather than edited away, because the reason is reusable.
//
//       (a) IT IS NOT PURE ARITHMETIC.  Read line by line, the golden body also
//           does SEVEN VCL widget writes (edAirForce/edAirKPA/edSetKg/
//           edForcePerDeviceKG/edForcePerDeviceN ->Color at :18730-18734,
//           :18746-18749, :18874-18877, :18886; lblMaxForcePerIC ->Visible/
//           ->Caption at :18738/:18742/:18756; lblMinForce->Caption at :18881;
//           edForcePerDeviceKG/N ->Text at :18750-18751), TWO widget reads
//           (scrbSLK->Position :18693, rgKitDiameter :18707-18714), and THREE
//           non-local writes (members dDutCount :18689 and dMinForce :18763/
//           :18868, and the GLOBAL IniConfig.iEP_Min_KG :18857).
//
//       (b) THE REAL BLOCKER IS `fContactForce->SLKClass`, NOT "not yet
//           ported".  Golden :18710-18727 walks that container to turn the
//           selected rgKitDiameter item into (iTag, dKitDiameter).  The port
//           has the ELEMENT type and the element FACTORY -- ContactForce.h
//           `struct SlkForceData` (dDiameter/dContactOffset/dContactOffset_NS)
//           and `ComputeSlkForce` -- but NO runtime container and no
//           `fContactForce` global (see (X-12) and the ABSENCE RE-RUN).
//           Without it the loop cannot run, and golden's locals keep their
//           initialisers: iTag=-1 (:18683), which :18761 forces to 0, and
//           dKitDiameter=30.0 (:18680).
//           A silent 30.0 is NOT an inert default.  It actively selects
//           ComputeMinForce's 30mm arm (dMinForce=1.5), it feeds the :18740
//           area formula (((30*30*3.14)/4)*coef*0.0101972) that CLAMPS the
//           per-device force whenever IniConfig.bD28MaxForceLimitByDiameter is
//           on, and it satisfies the ASE-Kaohsiung `if(dKitDiameter==30)
//           dMinForce=1;` at :18866-18869.  The output is DeviceForm.dPress
//           (via ShowArmAndDeviceForce :1909), i.e. the EP air-pressure
//           setpoint that the gated edAirForceChange (S-47) writes to the
//           regulator.  So shipping the six against a 30mm default would not
//           be "slightly off" -- it would be a confident, silent, wrong
//           contact force for every kit that is not 30mm.  That is strictly
//           worse than W26's rejected option of shipping five methods that
//           cannot be called: those fail loudly.
//
//       WHAT W28 DID DELIVER: the arithmetic itself, as
//       `ComputeTotalAirForce` in cContact.{h,cpp}, next to
//       ComputeMaxIndexForceLimit / ComputeMinForce and following the same
//       extract-calc-core convention -- every external read is a parameter,
//       every external write is a result field, and the SLK walk's two results
//       (dKitDiameter + the chosen entry's two contact offsets) are inputs the
//       caller must supply.  Both of golden's defaultless switches are
//       preserved and documented there.
//
//       WHAT REMAINS, for the wave that finishes this: give the tree a real
//       SLK table.  That means porting whatever fills golden's
//       `fContactForce->SLKClass` (TfContactForce's ReadFile path) into
//       something ContactForce.{h,cpp} can own -- a container of SlkForceData
//       plus its loader.  ContactForce.{h,cpp} was OUTSIDE W28's write
//       boundary, which is why it is written down here instead of done.  Once
//       that exists, (X-01)..(X-06) fall together, and the caller-side
//       `TfContact::CalculateTotalAirForce` becomes a thin wrapper: resolve the
//       SLK entry, call ComputeTotalAirForce, then apply the returned colour /
//       caption / Text / IniConfig.iEP_Min_KG effects to the real widgets.
//       The five it still blocks --
//           ShowArmAndDeviceForce   golden :1898-1918  (X-02)
//           edPinCountChange        golden :1920-1925  (X-03)
//           edForcePerPinNChange    golden :1927-1939  (X-04)
//           edDieForcePerPinGChange golden :1941-1957  (X-05)
//           rgOutKitDiameterClick   golden :17102-17137 (X-06)
//       -- were each re-read in full at W28 and are otherwise clean; every one
//       of them reaches CalculateTotalAirForce through ShowArmAndDeviceForce
//       (:1908), so they are transitively gated, nothing more.
//   (X-07) LoadImage        golden :1803-1807  `imgIndex->Picture->LoadFromFile`
//   (X-08) FormShow         golden :1078-1801  (724L) TImage + fSecurity x N + motion
//   (X-09) FormClose        golden :1815-1896  writes back + closes
//       TImage / TPicture have ZERO port anywhere in this tree -- established,
//       not re-litigated: forms/fTrayAssignment.h:168 (T1),
//       forms/fConfiguration.h:333, forms/fTeach.h:330, cSetUp.cpp:241.
//       (X-08)/(X-09) are gated on much more than that; (X-07) is gated on it
//       alone.  See GATE (C-3) for how DutCount's single LoadFromFile line is
//       handled without gating the whole method.
//   (X-10) SetContactMode   golden :15341-15434
//   (X-11) rbModeNormalClick golden :15330-15339 -> SetContactMode
//       ⚠ UPDATED BY AI(W906-FW-CONTACT-W28) 20260826.  W26 said "port all ten
//       into cContact.h ... that single edit un-gates (X-10), (X-11) and both
//       halves of (X-32)".  W28 did the consolidation.  It landed EIGHT of the
//       twelve; the ninth, CONTACT_TEST, is blocked by a file outside the write
//       boundary, and all four of these gates need exactly that one.  So they
//       stay.  Three separate figures in the old text were wrong; corrections
//       below, kept visible.
//
//       (i) THE BLOCK IS TWELVE CONSTANTS, NOT TEN.  golden cContact.cpp:74-85,
//           mirrored `extern const int` at golden cContact.h:20-28.  The "ten"
//           silently dropped CONTACT_IN_SHUTTLE_CHECK(6) and
//           CONTACT_OUT_SHUTTLE_CHECK(7).  Recounted by reading golden
//           :73-92 directly (20260826); :73 iSuckDelay and :87-88
//           PICK_UP_OFFSET/CONTACT_UP_OFFSET are adjacent but are not modes.
//
//      (ii) THE TREE HAD SIX FORK SITES, NOT "four partial copies".  W26's
//           re-run found two; a definition-shaped scan of all 1,578 .h/.cpp
//           files (comments AND string literals stripped by a character state
//           machine) found three more:
//               cContact.h                               NORMAL, MANUAL, DEVICE_MAP
//               ATC/ATCInterface.cpp:206                  CONTACT_NORMAL
//               AutoClean/AutoClean.cpp:125               CONTACT_DEVICE_MAP_CHECK
//               Command.cpp:317                           CONTACT_TEST
//               uTemp_Set.cpp:174                         CONTACT_TEST
//               BarCode/BarCode_Shuttle2_CCDScan.h:187-189  NORMAL, AUTO_GET_HEIGHT, TEST
//           Each carries some form of "no ODR risk, `const int` has internal
//           linkage" note.  That is true about LINK time and irrelevant at
//           COMPILE time: two definitions of one name at namespace scope in one
//           TU is a hard redefinition error whatever the linkage.
//
//     (iii) THE ODR LANDMINE IS NOT THE ONE W26 NAMED.  W26 worried about a TU
//           including both cContact.h and BarCode_Shuttle2_CCDScan.h.  An
//           include-closure walk that HONOURS `#if 0` (20260826) says that pair
//           is empty and the exposure is far smaller than it looks:
//               cContact.h reaches SIX TUs -- Command.cpp, ainarm9045.cpp,
//                 atester_shims.cpp, cContact.cpp, forms/fContact.cpp,
//                 tests/test_cContact.cpp
//               BarCode_Shuttle2_CCDScan.h reaches THREE --
//                 BarCode/BarCode_Shuttle2_CCDScan.cpp, aHotPlateSubstrate.cpp,
//                 tests/test_barcode_shuttle2_ccdscan.cpp
//               BOTH: zero.
//           ⚠ A FIRST PASS OF THIS SAME WALK SAID 164 TUs AND WAS WRONG:
//           MachineDefine.h:137 really does `#include "cContact.h"`, but
//           MachineDefine.h:34-146 is one `#if 0` block, so that edge does not
//           exist.  Any future include-reachability claim about this tree has
//           to honour `#if 0` or it will be off by two orders of magnitude.
//           The ACTUAL blocker is Command.cpp: it includes cContact.h at its
//           :239 AND defines its own CONTACT_TEST at its :317.
//           Verified by compiler, not by reasoning -- with CONTACT_TEST added
//           to cContact.h, `g++ -fsyntax-only` on Command.cpp gives
//           `Command.cpp:317:11: error: redefinition of 'const int
//           CONTACT_TEST'`.  It was backed out; the tree compiles.
//
//       STATE AFTER W28: cContact.h now defines ELEVEN of the twelve
//       (CONTACT_NORMAL, CONTACT_AUTO_GET_HEIGHT, CONTACT_MANUAL_GET_HEIGHT,
//       AUTO_CONTACT_TEST, STEP_CONTACT_TEST, CONTACT_IN_SHUTTLE_CHECK,
//       CONTACT_OUT_SHUTTLE_CHECK, CONTACT_LoadCell_AUTO_GET_HEIGHT,
//       CONTACT_DEVICE_MAP_CHECK, CONTACT_DEVICE_LOOP_TEST, K_TEMP_INDEX_MOVE).
//       CONTACT_TEST is absent, deliberately, with the retirement order written
//       into cContact.h's own HAZARD block.  Step 1 of that order -- delete
//       Command.cpp:317 (which already includes cContact.h, so it inherits the
//       value) and add the constant -- is the ONE edit that un-gates (X-10),
//       (X-11) and both halves of (X-32).  Command.cpp was explicitly outside
//       W28's write boundary.
//
//       ⚠ HONEST ACCOUNTING: the eight constants W28 did add un-gate NOTHING in
//       this file.  Every other golden method that reads them --
//       DoZ1/Z2PickFromShuttle, Do_Z1/Z2_AutoGetHeight, DoTestContactFunction,
//       DoZPlaceToShuttle, DoArm1/2PlaceToShuttle, Do_LoadCellAutoHigh,
//       CheckInShuttleSensor_Latch_Contact, FormClose -- is gated for MOTION or
//       for a FILE WRITE, not for a missing constant.  Their value is
//       consolidation plus being ready for those waves, and that is the whole
//       of it.
//
//       ⚠ RISK NOTE, unchanged and still owed by whoever un-gates these:
//       SetContactMode WRITES the global `iContactMode` (cmydef.h:3102), which
//       the gated motion SMs above branch on.  Golden treats it as a UI-state
//       setter and it issues no motion itself -- but it is a MODE SWITCH, and
//       CLAUDE.md puts mode switching in the safety-critical bucket.  Deliver
//       it with that said out loud, not silently.
//       Re-read in full at W28: apart from CONTACT_TEST, both bodies are
//       otherwise deliverable today -- SetContactMode needs only ->Checked /
//       ->Visible / ->Enabled and Memo1->Lines->Add (vclcompat::TMemo::Lines is
//       real, Controls.h:369), and rbModeNormalClick needs Sender->Name
//       (TControl::Name, Controls.h:183) and the global fAllMotorHome
//       (cmydef.h:222, real).  Both were checked this wave so the next one does
//       not have to.
//   (X-12) btContactForceClick golden :15012-15015 `fContactForce->Show()`
//       `fContactForce` has NO port global: `Grep "^\s*extern\s+\w+\s*\*\s*
//       fContactForce\s*;"` over the whole tree -> 0 (20260826).  Its 367
//       textual hits are cContact.h prose and tools/dfm2rc generated layout
//       tables.
//   (X-13) btShowDynaTempClick golden :15017-15021 `fDynamicTemp->Show()`
//       forms/fDynamicTemp.h declares `class TfDynamicTemp` (:586) but NO
//       `extern TfDynamicTemp *fDynamicTemp;` -- the only extern in the tree is
//       TU-local, uTemp_Set.cpp:212.  Nor does it carry Show()/BringToFront().
//   (X-14) btAtClick          golden :15023-15027 `fOmron->Show()` -- fOmron has
//       no port global (bthermo.cpp:3915 states the same finding).
//   (X-15) btTempOffsetClick  golden :15029-15033 `fTemp_Set->Show()`
//   (X-16) btnTempSetClick    golden :17184-17187 `fTemp_Set->Show()`
//       forms/fTemp_Set.h HAS the global (:1563) and `Close()` (:603) but no
//       `Show()`; adding one is that facade's call, not this file's.
//   (X-17) btOffsetClick      golden :16876-16879 `fOffSet->Show()`
//   (X-18) edContactHeight1Change golden :2261-2266 `fOffSet->bEnterSpecialOffset`
//       forms/fOffSet.h:18-24 is a two-method stub: UseAutoOffsetFunction /
//       UseInArmSetupTeach.  It has neither Show() nor bEnterSpecialOffset.
//       ⚠ A plain grep for `bEnterSpecialOffset` is NOT zero (14 hits): the
//       tree already has a TU-LOCAL MIRROR, `struct W4G_TfOffSetExt { bool
//       bEnterSpecialOffset; }` at Motor/myGALILmotor.cpp:680-684, whose own
//       comment says "write-only from this unit".  Same shape as
//       Automation/auto9045.cpp:319's `W5FA_TfContactExt W5FA_FContact`.
//       That pattern is available but is NOT used here: golden's write is the
//       only reason this handler exists, so a local mirror would make
//       edContactHeight1Change a function that writes to nothing and reads to
//       the next person as ported.  The real fix is one `bool
//       bEnterSpecialOffset;` on forms/fOffSet.h -- that file's wave, not this
//       one's.
//   (X-19) btBarcodeClick     golden :16881-16884 `fBarCode->Show()` --
//       aHotPlateSubstrate.h:984 `TfBarCode` has no Show().
//   (X-20) btTCPIPClick       golden :19024-19027 `fTesterTCP->Show()` -- no port
//       global (aoutarm.cpp:331 records the same).
//   (X-21) btnTrayMapClick    golden :18402-18406 `fTrayMapping->Show()` --
//       acatchtray_shims.h:313 has the global but no Show(); the body also
//       calls RecordProcess(), a log WRITE.
//   (X-22) btEditTrayClick    golden :16853-16859 `EditTray(MMTrayY, n)` -- opens
//       the tray editor ON A MOTOR OBJECT.
//   (X-23) pnlSensorAdjClick  golden :14064-14070 `fCCLink->ShowUseSensor(...)`
//       + `fCCLink->Show()` -- fCCLink has no port global.
//   (X-24) Edit16MouseDown    golden :2241-2249  `MyInputBox(...)`
//   (X-25) edSpeedZMouseDown  golden :15035-15041 `MyInputBox(...)`
//   (X-26) edD41Click         golden :15043-15047 `MyFloatInputBox(...)`
//       `MyInputBox` and `MyFloatInputBox` each return 0 hits tree-wide
//       (20260826, re-run at wave close).  Note this is a DIFFERENT keypad from
//       `fQwertyKey->ShowQwertyKey`, which IS ported (forms/fQwertyKey.h:370)
//       and which nine delivered handlers here use.  Do not "fix" (X-24)..
//       (X-26) by substituting ShowQwertyKey -- golden picks between the two
//       deliberately and they have different input-limit semantics.
//   (X-27) FormShortCut       golden :19226-19233 `TWMKey &Msg` has zero port.
//       OMITTED rather than half-ported, matching the four existing decisions
//       on the identical signature: cSetUp.cpp:250, uTemp_Set.cpp:6618,
//       uYieldMonitoring.cpp:2614, cBinSel.cpp:4222.
//   (X-28) TimerEPTimer       golden :16886-16941 ADAM_ReadPA + COM2 + fSecurity;
//       also WRITES the global bRTCAutoTuning from a poll.
//   (X-29) GetAutoHeightMaxKGTorque golden :5273-5382 -- the calc core is ALREADY
//       ported as the free function ComputeAutoHeightMaxKGTorque
//       (cContact.h:238); the member wrapper is left for the wave that decides
//       whether TfContact should delegate or duplicate.  Recorded so nobody
//       re-translates it thinking it is missing.
//   (X-30) CalculateTotalAirForce's siblings GetMaxIndexForceLimit
//       golden :18893-18955 and GetMinForce golden :18957-19022 -- same
//       situation as (X-29): bodies already live as ComputeMaxIndexForceLimit
//       (cContact.h:110) and ComputeMinForce (cContact.h:142).
//   (X-31) edDropWaitTimeMouseDown golden :2089-2239 (151L) -- reaches
//       MyInputBox and the teach path.
//   (X-32) OneCycleProcess   golden :11741-11746
//   (X-32) spbOneCycleClick  golden :16861-16865
//       ⚠ FOUND BY THE `-fsyntax-only` GATE, NOT BY READING -- recorded that way
//       on purpose.  Both were planned ACTIVE and both are otherwise entirely
//       clean; OneCycleProcess's two-line body simply needs `CONTACT_TEST`
//       (golden cContact.cpp:77, value 3), which is one of the mode constants
//       the port's cContact.h has not ported (see X-10).
//       spbOneCycleClick then falls transitively (its whole body is
//       `OneCycleProcess(); ledOneCycle->Value=bContinueContact;`).
//       LESSON, worth keeping: reading a body for MACHINE ACTIONS does not
//       catch a missing CONSTANT.  Symbol availability (batch criterion 4) has
//       to be checked per-identifier, and the compiler is the only reliable
//       checker.  This is also why the delivered count in this banner was
//       re-measured from the finished .cpp rather than from the plan.
//       ⚠ STILL GATED AFTER AI(W906-FW-CONTACT-W28) 20260826, and now for a
//       precisely located reason.  W28 added eight of the twelve mode constants
//       to cContact.h, but CONTACT_TEST -- the only one these two need -- could
//       not be added: Command.cpp includes cContact.h (its :239) and defines
//       its own `const int CONTACT_TEST = 3;` (its :317), so adding it makes
//       Command.cpp fail with `error: redefinition of 'const int
//       CONTACT_TEST'` (compiler-verified, then backed out).  Command.cpp is
//       outside W28's write boundary.  DELETING Command.cpp:317 and adding
//       CONTACT_TEST to cContact.h -- one line removed, one line added -- is
//       the whole unlock for (X-10), (X-11) and both halves of (X-32).
//
//  (The four remaining gated names not itemised above -- ShowArmAndDeviceForce
//  and its three Change handlers -- are (X-02)..(X-05).)
//
//  ============================================================================
//  GATE REGISTER -- gates INSIDE otherwise-ACTIVE methods
//  ============================================================================
//  (C-1) `CheckAndReadIniData(FileName, "SLK Type", ...)` x6 -- Init(), golden
//      :149, :160, :161, :162, :186, :198.  THE NAME LIES: common.cpp:602-603
//      WriteInteger()s the default back into the .ini when the key is absent,
//      so every one of these is a WRITE to D:\HT9045\system\ContactInfo.ini on
//      a development box.  The previous wave exited 9 methods for exactly this,
//      and this tree has already lost system\Gerneral.ini once (20260817) to
//      the same coupling.
//      HOW IT IS GATED, and why this is faithful rather than a guess: golden
//      itself wraps every one of them in `if(FileExists(FileName)) ... else
//      <literal defaults>`.  The port keeps the `else` arm VERBATIM and gates
//      the `if` arm, i.e. it behaves exactly as golden does on a machine where
//      ContactInfo.ini has not been created yet.  That is a real golden state,
//      not an invented one.  The literal defaults are golden's own:
//      "28,40,58,56" / "30,40,60" / "40,55,75" / "40,55,75,108" / "30,40,60,56"
//      / "20,30,40,50".
//  (C-2) `this->BorderIcons = TBorderIcons();` -- Init(), golden :110
//      (RogerYang 20260129, CC_JCET: hide the window X so nobody closes the form
//      mid-press and drops an IC).  `TBorderIcons` has zero DEFINITION anywhere
//      in the port -- verified at wave close, and stated as "definition" rather
//      than "hits" because a plain grep is NOT zero: Automation/
//      uRENESAS_Server.cpp:3215 and :3278 both write
//      `this->BorderIcons=TBorderIcons()<<biSystemMenu;`, and BOTH are inside
//      `#if 0` (opened at :3207 and :3270, confirmed by an #if/#endif depth
//      scan).  `biSystemMenu` likewise has those two gated uses and no
//      definition.  The facade has no window chrome for any of it to mean
//      anything against; gating it changes no translated value.
//  (C-3) `imgSLK->Picture->LoadFromFile(asStr);` -- DutCount, golden :2070.
//      TImage/TPicture have no port (see X-07).  The `asStr.sprintf("%sContact
//      %d.bmp", BmpPath, scrbSLK->Position)` that BUILDS the path is kept
//      ACTIVE and its result is stored, so the intent is observable; only the
//      load is gated.  The whole of DutCount's dDutCount arithmetic -- the part
//      the force calculation depends on -- is unaffected.
//      Same treatment as forms/fTrayAssignment.cpp:705.
//
//  ============================================================================
//  GOLDEN NOTES (faithful; recorded, NOT "fixed")
//  ============================================================================
//  (G-DimCmp) CheckContactSettingChange, golden :17088-17091:
//      `iUnitMultiply100(atoi(edXDimension->Text.c_str())) != FormatFloat("0.00", DeviceForm.XDimension)`
//      compares an INT against an ANSISTRING.  AI(W906-NUMCMP) 20260927: CORRECTED -- with the
//      number on the LEFT, bcc32 5.6.4 builds Variant(int)!=Variant(str), a NUMERIC
//      compare (NB2 R89; this note used to say TEXT).  Still TRUE for most values.  Two further
//      layers of the same oddity: `atoi` truncates the edit's decimals before
//      iUnitMultiply100 (whose own parameter is a double) ever sees them, and
//      the two operands are in different units (0.01mm vs mm).  Net effect in
//      golden: once control reaches :17088 the function almost always returns
//      true, i.e. "the contact setting changed".  TRANSLATED VERBATIM -- the
//      port USED TO rely on the non-explicit `AnsiString(int)` ctor
//      (vclcompat/AnsiString.h:67) + free `operator!=` (:224) = a TEXT compare, which
//      differed when a Dimension is 0; fContact.cpp:1021/:1023 now compare numerically.  Do not
//      "correct" it without a decision: it is the Run-check trigger.
//  (G-NoElse) timerContactTimer, golden :21355-21366, has an `if(...) { ...
//      Enabled=false; return; }` and NO else.  So the two controls are DISABLED
//      when the machine holds an IC and are never RE-ENABLED by this timer.
//      Verbatim.
//  (G-Latch4) palOTD_4Click / palOTD_6Click (S-15/S-16) each own a
//      `static bool bDown` and clear the OTHER panel's bevel but not the other
//      panel's latch, so the two statics can disagree with what is displayed.
//      Recorded for whoever un-gates them; not translated here.
//  (G-DutFallthrough) DutCount's switch (golden :1962-2059) has NO `default:`.
//      Any iTestMode outside the 17 listed leaves `dDutCount` at its previous
//      value -- which for a freshly constructed form is the ctor's `2`
//      (golden :240).  Verbatim; do not add a default.
//  (G-EmptyBodies) InitContactCbList (golden :352-354) is an EMPTY function and
//      InitContactEdtList (golden :356-361) is three commented-out declarations.
//      They are delivered as-is, with golden's comment text kept, precisely so a
//      later reader does not "discover" them as unported.  InitContactEdtList
//      IS called -- from golden's ctor, :192 -- so it is a real, reachable
//      no-op, not dead code.
//  (G-ItemsFromDfm) Golden's ctor REPLACES rgKitDiameter->Items /
//      rgDieForceKitDiameter->Items only under specific customer/config
//      branches.  Outside them the .dfm lists stand, and
//      rgDieForceKitDiameterClick READS them
//      (`atof(Items->Strings[ItemIndex])/10.0`).  See DEVIATION (D-6).
//
//  ============================================================================
//  DEVIATIONS (port-only; each with why)
//  ============================================================================
//  (D-1) `__fastcall` dropped from every signature -- tree-wide convention
//      (vclcompat/vcl_compat.h neutralises it; forms/fQwertyKey.h and
//      forms/fMotorTest.h do the same).
//  (D-2) CTOR / Init() SPLIT (task-mandated).  `TfContact::TfContact()` stuffs
//      FIELDS ONLY.  Golden's ENTIRE ctor body (cContact.cpp:107-276) moves
//      verbatim into an explicit `Init()`.
//      WHY THIS IS LOAD-BEARING HERE SPECIFICALLY: golden's ctor body reads
//      CUSTOMER_CODE, CosFunction, IniConfig, MachineTypeChoice and
//      INSTALL_DOUBLE_EP, calls FileExists() on an absolute path, calls
//      InitContactEdtList() and InitRTCAutoTuning(), and `new`s a TStringList.
//      Those are cross-TU objects whose own construction order is unspecified.
//      That is the exact shape that cost this tree 88 SEGFAULTs out of 134
//      ctests when fLaserSensor's ctor called InitLaserEdtList() with elLaser
//      still NULL (docs/KNOWLEDGE.md).  With the body in Init() there is no
//      static-initialisation exposure at all.
//  (D-3) Widget members are OWNING raw pointers with in-class `new`
//      initialisers (forms/fMotorTest.h:617, forms/fTemp_Set.h:1446,
//      forms/fShowBinSelect.h:880 precedent) and are freed by a port-only
//      destructor.  Golden's TForm owner frees them; there is no owner here.
//      Copy/assign are `= delete` so the mistake is a compile error, not a
//      double free.  Golden's own `~TfContact()` body (:279-281) is empty and
//      stays empty -- the frees are additional, and marked as such.
//  (D-4) FIVE facade-local widget types, for members whose golden class has
//      no port or lacks the one property golden writes:
//        TfContactImage    -- golden `TImage *imgSLK/*imgIndex`.  Empty by
//            design: TImage/TPicture have zero port (X-07) and every use is
//            gated.  Declared so the published member list stays complete and
//            diffable against golden cContact.h:61/:86.
//        TfContactLed      -- golden `TMyLed *ledOTD/*ledOneCycle`.  NOT
//            `ATC/ATCInterface.h:208 class TMyLed` (which IS a real global-scope
//            port) -- reusing that name here would be fine, but this file
//            deliberately does not depend on the ATC header for two LEDs, and
//            forms/fMotorTest.h:484 set the facade-local precedent.  Only
//            ->Value is touched by golden -- but only from spbOneCycleClick,
//            which is GATE (X-32), so NO delivered body reaches it today.
//        TfContactTimer    -- golden `TTimer *OTDTimer/*TimerEP/*timerContact`.
//            `{bool Enabled;}`, the established per-form shape
//            (forms/fMotorTest.h:468, forms/fLotInfo.h:1167).
//        TfContactRadioGroup -- vclcompat::TRadioGroup (Controls.h:450) has
//            ItemIndex and Items but NO Caption, and golden writes
//            `rgKitDiameter->Caption="Kit Diameter (Square)"` (:119) and
//            `="Kit Diameter"` (:135).  Compose, don't fork vclcompat
//            (forms/fDynamicTemp.h:311 precedent).
//        TfContactGroupBox -- vclcompat::TGroupBox (Controls.h:308) has no
//            Height, and golden writes `gbContactParameter->Height=230/265`
//            (:259/:261/:265).
//  (D-5) `Show()` / `ShowModal()` / `Close()` / `BringToFront()` are permanent
//      offline no-ops (forms/fQwertyKey.h:365-366, forms/fTemp_Set.h:603
//      precedent).  Only `Close()` is reached from delivered code (sbtExitClick,
//      golden :14486).
//  (D-6) `.dfm`-sourced initial values that translated code READS are set in the
//      port ctor (forms/fMotorTest.h D-7 / forms/fBinSel.h precedent).  Exactly
//      six, each load-bearing, each cited:
//        scrbSLK Max=5 / Min=2 / Position=2   (cContact.dfm:16129/:16130/:16132)
//            -- Position is read by DutCount (:2063-2068) AND by
//            CheckContactSettingChange (:17074), and is used to index
//            "Contact%d.bmp".  At a zero default the SingleSite fix-up at
//            :2063 would not fire and the Run-check would compare against 0.
//        edForcePerPinN Tag=1                 (cContact.dfm:16202)
//        edForcePerPinG Tag=1                 (cContact.dfm:16020)
//            -- see (D-7); these two are the important ones.
//        edPinCount Tag=1                     (cContact.dfm:16181)
//        edOutDiameter Text="40,55,75,108"    (cContact.dfm:350)
//        rgSuckShuttleDeviceAfterTested ItemIndex=1    (cContact.dfm:17693)
//        rgSuckShuttleDeviceWaitOnShuttle ItemIndex=1  (cContact.dfm:17638)
//            -- both are read by CheckContactSettingChange (:17067/:17069);
//            vclcompat::TRadioGroup defaults ItemIndex to 0, so leaving them
//            would silently invert the Run-check for those two settings.
//      Plus the four `Items` lists that delivered code reads -- see (D-9).
//      Values whose vclcompat default already equals the .dfm value
//      (cbContactMode ItemIndex=0, rgKitDiameter ItemIndex=0, coD41
//      ItemIndex=0, rgShuttleWaitingOutSiteChamber ItemIndex=0) are NOT
//      restated; they are listed here so the omission is deliberate, not
//      forgotten.
//  (D-7) ⚠ THE `Tag` PROVENANCE TRAP, hit head-on by this wave.
//      edForcePerPinNMouseDown (:18547) and edForcePerPinGMouseDown (:19034)
//      both branch on `if(Buffer->Tag==1) { if(Barcode_Reader(bcContact)==0)
//      return; }` -- i.e. Tag==1 turns the field into a barcode-scan target and
//      an unsuccessful scan ABORTS the handler before the keypad opens.
//      vclcompat::TControl::Tag (Controls.h:255) exists but its VALUE has no
//      loader: dfm2rc keeps Tag in its stage-1 IR and in
//      web/forms/*.layout.json, but the .rc/layout/uimap emitters drop it and
//      no C++ reads a layout (Controls.h:238-254 states this in full).  So an
//      un-initialised Tag reads 0 -- and 0 here is NOT a neutral "else": it
//      SKIPS the barcode gate entirely.
//      This wave therefore sets Tag=1 in the port ctor from the .dfm, with the
//      citation above.  That is what (D-6) exists for, and it is the difference
//      between "faithful" and "compiles".
//      GENERALISATION FOR THE NEXT WAVE: golden cContact.dfm carries Tag lines
//      on further widgets that only GATED methods read.  Anyone un-gating one
//      must re-check its Tag provenance the same way; a silent 0 will pick the
//      wrong arm, not no arm.
//  (D-8) `Get0_01MMType(char *str)` (cpublic.h:19) takes a NON-const char*,
//      because BCB6's `AnsiString::c_str()` returned `char*` while
//      `vclcompat::AnsiString::c_str()` returns `const char*`
//      (vclcompat/AnsiString.h:100).  UpdateContactRelative (golden
//      :22318/:22321) is the only delivered caller.  Ported with an explicit
//      `const_cast<char*>`, which is SAFE HERE and machine-checked, not
//      assumed: Get0_01MMType's entire body is `double i=atof(str); ...`
//      (cpublic.cpp:411-418) -- it never writes through the pointer.
//      The correct long-term fix is `const char*` on cpublic.h:19; that file is
//      outside this wave's write boundary.  Precedent for the cast already
//      exists at tests/test_cpublic_foundation.cpp:359 (`(char*)"1.50"`).
//  (D-9) Golden's ctor does NOT populate rgKitDiameter/rgOutKitDiameter/
//      rgDieForceKitDiameter/cbVacuumMode/coD41 Items on every path -- outside
//      the customer branches the `.dfm` lists stand.  vclcompat::TRadioGroup /
//      TComboBox allocate an EMPTY Items, and rgDieForceKitDiameterClick reads
//      `Items->Strings[ItemIndex]` -- which on an empty list yields "" and
//      makes dDieForceKitDiameter 0.0, a silent wrong kit diameter.  So the
//      ctor seeds the four lists that delivered code can read, from the .dfm:
//        rgKitDiameter         "30","40","60"    (cContact.dfm:16174-16176)
//        rgOutKitDiameter      "40mm","55mm","75mm" (cContact.dfm:16267-16269)
//        rgDieForceKitDiameter "30","40","60"    (cContact.dfm:16498-16500)
//        coD41 "Inside socket","Above Socket"    (cContact.dfm:18593-18594)
//      cbContactMode's list is NOT seeded from the .dfm: golden's own ctor
//      Clear()s and rebuilds it on BOTH arms of its branch (:205-235), so the
//      .dfm value is unreachable once Init() has run.
//  (D-10) `TObject *Sender` parameters are kept on every handler (golden
//      signature, vclcompat::TObject exists).  Handlers are NOT wired to
//      anything -- task rule 3; vclcompat widgets carry no OnClick delegate
//      slot (cSetUp.cpp GATE(G-Delegate), the tree-wide idiom).
//  (D-11) Unused golden parameters get a leading `(void)Sender;` /
//      `(void)Button; (void)Shift; (void)X; (void)Y;` line.  The project's own
//      flags already carry `-Wno-unused-parameter` (build_*/CMakeFiles/
//      ht9045_sm.dir/flags.make), so these are NOT needed to pass the gate --
//      they are there so this file also survives a stricter flag set, and they
//      are confined to one clearly-marked line per body so the translated
//      statements underneath stay verbatim.
//
//  ============================================================================
//  ABSENCE RE-RUN AT WAVE CLOSE -- 20260826 17:01
//  ============================================================================
//  Absence claims go stale (docs/KNOWLEDGE.md), and a sibling wave was editing
//  forms/fLotInfo.{h,cpp} while this one ran.  So every "X does not exist"
//  statement above was re-run against the FINISHED tree, with a scanner that
//  strips comments AND string literals first (a first pass that skipped only
//  full-line `//` produced false positives -- see the fContact-global section).
//  Corpus: 1,582 .h/.hpp/.cpp/.c files under HT9011UC_Cpp_V3.33.906.0,
//  excluding build*/ .git/ .svn/ docs/ __pycache__.
//
//  STILL ABSENT (12): class TfContact (outside this file) | MyInputBox |
//    MyFloatInputBox | extern *fContactForce | extern *fTesterTCP |
//    extern *fOmron | extern *fCCLink | class TImage / class TPicture |
//    class/struct TWMKey | CalculateTotalAirForce (outside this file) |
//    STEP_CONTACT_TEST | AUTO_CONTACT_TEST | CONTACT_LoadCell_AUTO_GET_HEIGHT |
//    CONTACT_DEVICE_LOOP_TEST | K_TEMP_INDEX_MOVE
//
//  ============================================================================
//  ABSENCE RE-RUN #2 -- AI(W906-FW-CONTACT-W28) 20260826 18:23
//  ============================================================================
//  Re-run at W28's close because absence claims expire (docs/KNOWLEDGE.md) and
//  because a sibling session was running the V899->V910 MG port campaign in
//  this same repo while W28 ran.  Same method as re-run #1: comments AND string
//  literals stripped by a character state machine before matching.  Corpus:
//  1,578 .h/.hpp/.c/.cpp files under HT9011UC_Cpp_V3.33.906.0, excluding
//  build*/ .git/ .svn/ __pycache__/ .pti_frames/.
//
//  CONFIRMED STILL ABSENT -- 0 hits each, outside comments and strings:
//    MyInputBox | MyFloatInputBox | extern *fTesterTCP | extern *fOmron |
//    extern *fCCLink | class TImage | class TPicture | class/struct TWMKey
//  class TfContact: 1 hit, this file -- i.e. still nothing outside it.
//  CalculateTotalAirForce: 1 hit, this file's own gated declaration -- i.e.
//    still no definition anywhere, which is the claim that matters.
//
//  ⚠ STRENGTHENED, not merely re-confirmed: `fContactForce` and `SLKClass` each
//  return ZERO TOKENS tree-wide -- not "no extern declaration", but the
//  identifiers do not occur at all outside comments.  W26 could only say the
//  `extern` pattern found nothing (367 textual hits, all prose and generated
//  layout tables).  This is the stronger form of the claim, and it is what
//  makes (X-01)'s blocker structural rather than a naming problem: there is no
//  SLK container under any name, so the golden :18710-18727 walk has nothing to
//  walk.  Whoever adds one owns ContactForce.{h,cpp}.
//
//  CHANGED FROM ABSENT TO PRESENT -- by this wave, deliberately:
//    AUTO_CONTACT_TEST | STEP_CONTACT_TEST | CONTACT_IN_SHUTTLE_CHECK |
//    CONTACT_OUT_SHUTTLE_CHECK | CONTACT_LoadCell_AUTO_GET_HEIGHT |
//    CONTACT_DEVICE_LOOP_TEST | K_TEMP_INDEX_MOVE | CONTACT_AUTO_GET_HEIGHT
//      -> cContact.h, one definition each, no other copy anywhere except
//         CONTACT_AUTO_GET_HEIGHT's pre-existing BarCode_Shuttle2_CCDScan.h:188
//         (no TU sees both -- see (X-10)(iii)).
//    ComputeTotalAirForce -> cContact.h + cContact.cpp (declaration + body).
//
//  STILL ABSENT AND DELIBERATELY SO: CONTACT_TEST in cContact.h.  Its 11
//  tree-wide hits are Command.cpp (:317 definition + 4 real uses at :1793,
//  :1795, :1818, :1822), uTemp_Set.cpp (:174 definition + :6378, :6380), and
//  BarCode/BarCode_Shuttle2_CCDScan.h:189 (+ :2079, :2197 in its .cpp).  The
//  four uses in Command.cpp are why its retirement must be atomic: delete
//  Command.cpp:317 and add the constant to cContact.h in the SAME change, or
//  Command.cpp loses the symbol it still needs.  See (X-10) and cContact.h's
//  HAZARD block.
//
//  ⚠ FOUND TO BE PRESENT -- i.e. four claims an earlier draft got WRONG.  All
//  four are corrected in place above rather than deleted, because the wrong
//  version is the more instructive artefact:
//    * CONTACT_TEST            -> Command.cpp:317 + BarCode/
//      CONTACT_AUTO_GET_HEIGHT     BarCode_Shuttle2_CCDScan.h:187-189.
//                                Two pre-existing TU-local forks.  See (X-10).
//    * TBorderIcons            -> 2 uses, BOTH inside `#if 0`.  No definition.
//                                Claim re-worded from "no hits" to "no
//                                definition".  See GATE (C-2).
//    * bEnterSpecialOffset     -> a TU-local mirror struct,
//                                Motor/myGALILmotor.cpp:680-684.  Claim
//                                re-scoped to forms/fOffSet.h.  See (X-18).
//  NONE of the four changes a gate decision; all four change what the banner is
//  allowed to assert.  Method matters more than the result here: "grep returned
//  nothing" is not the same claim as "the symbol has no definition", and this
//  file now says which one it means, every time.
//
//  ============================================================================
//  SYMBOLS THIS TU NEEDS (for the CMakeLists landing decision -- NOT made here)
//  ============================================================================
//  This wave may not touch CMakeLists.txt.  Measured needs, so the main loop
//  does not have to rediscover them:
//    vclcompat        -- AnsiString / TStringList / TObject / Controls.h widget
//                        set / ScrollBar.h / SysUtils.h (FormatFloat, FileExists)
//    ht9045_globals   -- CUSTOMER_CODE, iContactMode, iTotalGf, bKitOutDiameter,
//                        sKitOutDiameter[], bRTCAutoTuning, bRun2DCheck,
//                        bDoRTCLearning, EP_Install, INSTALL_DOUBLE_EP,
//                        BAR_CODE_INSTALL, MachineTypeChoice, N_DOUBLE/N_INTEGER,
//                        MTestY1/MTestZ1 (cmydef); DeviceForm, DeviceForm_File,
//                        TestIF_File, InputLimit, Teach (cprod); IniConfig
//                        (Config.h); CosFunction (CosFunction.h); LastSet
//                        (LastSet.h); iCloseSiteModeFor2x8, InArmSuck,
//                        OutArmSuck (aHotPlateSubstrate.h)
//    ht9045_core      -- common.h (ReadIniData / GetRecipeFileName /
//                        OnlyNumberInPut / OnlyNumberAndDotInPut / BmpPath),
//                        cpublic.h (Get0_01MMType / ConvertTouMType),
//                        cUnitConvert.h (iUnitMultiply100),
//                        EJ1N/TextProcess.h (MyFormatFloat),
//                        canary_support.h (ShowMyMessage),
//                        cContact.h (ComputeIsRun2DCheck / ComputeIndexYSpeed /
//                        ComputeIndexZSpeed and the ported mode constants --
//                        ELEVEN of golden's twelve since W28, see (X-10))
//  ⚠ AI(W906-FW-CONTACT-W28) 20260826 -- NO CHANGE TO THE LANDING DECISION.
//  W28 added ComputeTotalAirForce to cContact.{h,cpp}, which is already in
//  ht9045_core (build_*/CMakeFiles/ht9045_core.dir/cContact.cpp.obj).  It
//  introduces NO new archive edge, because it is deliberately self-contained:
//    * it calls only ComputeMinForce and ComputeMaxIndexForceLimit, both
//      defined in the SAME TU (cContact.cpp), and
//    * ChangeToFloatNonPcnt is a header template (MachineType.h:1601), so it
//      needs no external symbol either.
//  cContact.cpp still `#include`s exactly one header, "cContact.h" -- unchanged
//  by W28, and worth keeping that way: it is the reason every global read is a
//  parameter.  The eight new mode constants are `const int` at namespace scope,
//  i.e. internal linkage, so they add no link-time symbol at all.
//  forms/fContact.cpp was NOT edited by W28 (nothing became deliverable), so
//  its ht9045_sm registration and the csystem.h diet risk flagged below are
//  exactly as W26 left them.
//    ht9045_forms     -- forms/fQwertyKey.h (fQwertyKey->ShowQwertyKey)
//    Motor/mymotor.h  -- MOT[].GailSpeed  (a plain int READ; no motion call is
//                        made from any delivered body)
//    BarcodeReader.h  -- Barcode_Reader() (BarcodeReader.cpp:445)
//    csystem.h        -- IndexHasIC() / ShuttleHasIC()  ⚠ THE ONE DIET RISK:
//                        their bodies are in csystem_predicates.cpp, an
//                        ht9045_sm-layer TU.  If forms/fContact.cpp lands in
//                        `ht9045_forms` (the natural home, next to
//                        forms/fQwertyKey.cpp and forms/fMotorTest.cpp,
//                        CMakeLists.txt:654-713) that ONE edge -- reached only
//                        from timerContactTimer (golden :21357/:21360) -- may
//                        break the target's link diet.  forms/fMotorTest.h
//                        (M-Destroy) hit the same wall and GATED the two calls;
//                        forms/fLotInfo.cpp:58 took the other route, a TU-local
//                        `extern`.  Flagged so the choice is deliberate: if the
//                        diet bites, gate timerContactTimer rather than adding
//                        ht9045_sm to ht9045_forms.
//  ⚠ REMEMBER: registering this file makes the linker able to PULL it.  Until
//  something references a symbol in it, none of the 90 gates can be reached --
//  but that is a property of the gates, not of the registration.
// =============================================================================
#ifndef FORMS_FCONTACT_H
#define FORMS_FCONTACT_H

#include "vclcompat/vcl_compat.h"    // AnsiString / TObject / TStringList at global scope
#include "vclcompat/Controls.h"      // TLabel/TPanel/TEdit/TCheckBox/TComboBox/... (global `using`s at :533-551)
#include "vclcompat/ScrollBar.h"     // vclcompat::TScrollBar -- MUST stay qualified (handlerlog.h:122 has a global TScrollBar)
#include "vclcompat/ShiftState.h"    // TShiftState / TMouseButton
#include "myTimer.h"                 // TQPF_Timer (golden hContactDeley / DriverDelay)
#include "MachineType.h"             // MAX_SOCKET_TOTAL (:393) -- sizes iDMCTrayX/Y/Channel below

// ---------------------------------------------------------------------------
//  DEVIATION (D-4) -- five facade-local widget stand-ins.
// ---------------------------------------------------------------------------

// golden `TImage *imgSLK` / `*imgIndex` (cContact.h:61/:86).  TImage and
// TPicture have ZERO port in this tree (forms/fTrayAssignment.h:168 T1); every
// statement that touches them is GATE (X-07) or GATE (C-3).  Declared empty so
// the published member list stays diffable against golden.
class TfContactImage : public TControl
{
public:
    virtual ~TfContactImage() {}
};

// golden `TMyLed *ledOTD` / `*ledOneCycle` (cContact.h:147/:212).
// golden touches only ->Value, from spbOneCycleClick (golden :16864) -- which
// is GATE (X-32), so no delivered body reaches it in this wave.
class TfContactLed : public TControl
{
public:
    bool Value;
    TfContactLed() : Value(false) {}
    virtual ~TfContactLed() {}
};

// golden `TTimer *OTDTimer` / `*TimerEP` / `*timerContact`
// (cContact.h:148/:230/:306).  Only ->Enabled is ever touched, and only from
// gated bodies.
class TfContactTimer : public TObject
{
public:
    bool Enabled;
    TfContactTimer() : Enabled(false) {}
    virtual ~TfContactTimer() {}
};

// golden `TRadioGroup *` members.  vclcompat::TRadioGroup (Controls.h:450) has
// ItemIndex + Items but no Caption; golden writes rgKitDiameter->Caption at
// cContact.cpp:119 and :135.
class TfContactRadioGroup : public TRadioGroup
{
public:
    AnsiString Caption;
    virtual ~TfContactRadioGroup() {}
};

// golden `TGroupBox *` members.  vclcompat::TGroupBox (Controls.h:308) has
// Caption but no Height; golden writes gbContactParameter->Height at
// cContact.cpp:259/:261/:265.
class TfContactGroupBox : public TGroupBox
{
public:
    int Height;
    TfContactGroupBox() : Height(0) {}
    virtual ~TfContactGroupBox() {}
};

// ===========================================================================
//  TfContact -- non-VCL facade (golden cContact.h:51-665,
//  `class TfContact : public TForm`).
//
//  Widget members below keep golden's ORDER and golden's .dfm leaf NAMES
//  verbatim (golden cContact.h:54-323) so the two lists diff line-for-line.
//  Members are OWNING raw pointers with in-class `new` initialisers
//  (DEVIATION D-3), which is safe because nothing constructs TfContact at
//  static-initialisation time -- this header declares no global at all (see the
//  `fContact` note in the banner).
// ===========================================================================
class TfContact
{
public:
    // -- golden __published, cContact.h:54-323 -------------------------------
    TPanel                *pnlBottom            = new TPanel();
    TSpeedButton          *sbtExit              = new TSpeedButton();
    TSpeedButton          *spbSave              = new TSpeedButton();
    TButton               *btnStart             = new TButton();
    TButton               *btnPause             = new TButton();
    TPanel                *pnlSLKSetting        = new TPanel();
    TfContactGroupBox     *gbContactForce       = new TfContactGroupBox();
    TfContactImage        *imgSLK               = new TfContactImage();
    TLabel                *lblContactForce      = new TLabel();
    TLabel                *lblForceN            = new TLabel();
    TLabel                *lblMaxForce          = new TLabel();
    TLabel                *lblForcePerDevice    = new TLabel();
    TLabel                *lblForcePerDeviceKg  = new TLabel();
    TLabel                *lblForcePerDeviceN   = new TLabel();
    TLabel                *lblYDimension        = new TLabel();
    TLabel                *lblXDimension        = new TLabel();
    TLabel                *lblReadEP            = new TLabel();
    TEdit                 *edAirForce           = new TEdit();
    TEdit                 *edAirForceN          = new TEdit();
    TEdit                 *edForcePerDeviceKG   = new TEdit();
    TEdit                 *edForcePerDeviceN    = new TEdit();
    TEdit                 *edYDimension         = new TEdit();
    TEdit                 *edXDimension         = new TEdit();
    vclcompat::TScrollBar *scrbSLK              = new vclcompat::TScrollBar();
    TButton               *btnTStart            = new TButton();
    TButton               *btnTStep             = new TButton();
    TButton               *SetMPaBtn            = new TButton();
    TfContactRadioGroup   *rgKitDiameter        = new TfContactRadioGroup();
    TPanel                *btContactForce       = new TPanel();
    TPanel                *palOTD_4             = new TPanel();
    TPanel                *pnlTop               = new TPanel();
    TfContactGroupBox     *gbContactHeight      = new TfContactGroupBox();
    TfContactImage        *imgIndex             = new TfContactImage();
    TLabel                *lblReleaseHeight     = new TLabel();
    TLabel                *lblPickHeight        = new TLabel();
    TLabel                *lblDropOffset        = new TLabel();
    TLabel                *lblContactHeight     = new TLabel();
    TLabel                *lblReleaseHeightMM   = new TLabel();
    TLabel                *lblPickHeightMM      = new TLabel();
    TLabel                *lblDropOfsMM         = new TLabel();
    TLabel                *lblContactHighMM     = new TLabel();
    TEdit                 *edReleaseHeight1     = new TEdit();
    TEdit                 *edPickUp1            = new TEdit();
    TEdit                 *edDropOffset1        = new TEdit();
    TEdit                 *edtSht1              = new TEdit();
    TEdit                 *edtSht2              = new TEdit();
    TEdit                 *edtArm1              = new TEdit();
    TEdit                 *edContactHeight1     = new TEdit();
    TEdit                 *edReleaseHeight2     = new TEdit();
    TEdit                 *edPickUp2            = new TEdit();
    TEdit                 *edDropOffset2        = new TEdit();
    TEdit                 *edContactHeight2     = new TEdit();
    TEdit                 *edtArm2              = new TEdit();
    TfContactGroupBox     *gbContactParameter   = new TfContactGroupBox();
    TLabel                *lblContactMode       = new TLabel();
    TLabel                *lblVacMode           = new TLabel();
    TLabel                *lblDummyContact      = new TLabel();
    TLabel                *lblDropWaitTime      = new TLabel();
    TLabel                *lblSuckShuttleDeviceAfterTested   = new TLabel();
    TLabel                *lblShuttleWaitingOutSiteChamber   = new TLabel();
    TLabel                *lblDropSpeed         = new TLabel();
    TComboBox             *cbContactMode        = new TComboBox();
    TComboBox             *cbVacuumMode         = new TComboBox();
    TPanel                *pnlDummyContact      = new TPanel();
    TRadioButton          *rbNormal             = new TRadioButton();
    TRadioButton          *rbDummyMode          = new TRadioButton();
    TEdit                 *edDropWaitTime       = new TEdit();
    TfContactRadioGroup   *rgSuckShuttleDeviceAfterTested    = new TfContactRadioGroup();
    TfContactRadioGroup   *rgShuttleWaitingOutSiteChamber    = new TfContactRadioGroup();
    TEdit                 *edDropSpeed          = new TEdit();
    TfContactGroupBox     *gbDynaTemp           = new TfContactGroupBox();
    TButton               *btAt                 = new TButton();
    TButton               *btShowDynaTemp       = new TButton();
    TButton               *btTempOffset         = new TButton();
    TEdit                 *edSpeed              = new TEdit();
    TEdit                 *edSpeedZ             = new TEdit();
    TLabel                *lblIndexSpeedY       = new TLabel();
    TLabel                *lblIndexSpeedZ       = new TLabel();
    TfContactRadioGroup   *rgPSDT               = new TfContactRadioGroup();
    TLabel                *lblPSDT              = new TLabel();
    TLabel                *lblOrgHeight         = new TLabel();
    TEdit                 *edContactBackUp1     = new TEdit();
    TEdit                 *edContactBackUp2     = new TEdit();
    TPanel                *pnlTorqueControl     = new TPanel();
    TLabel                *lblTorqueMax         = new TLabel();
    TLabel                *lblTorqueCompare     = new TLabel();
    TEdit                 *edtTorqueMax         = new TEdit();
    TEdit                 *edtTorqueCmp         = new TEdit();
    TLabel                *labContactOffset     = new TLabel();
    TEdit                 *edContactOffsetArm1  = new TEdit();
    TEdit                 *edContactOffsetArm2  = new TEdit();
    TPanel                *palOTD_6             = new TPanel();
    TPanel                *pnlSensorAdj         = new TPanel();
    TfContactLed          *ledOTD               = new TfContactLed();
    TfContactTimer        *OTDTimer             = new TfContactTimer();
    TLabel                *lblLayoutkit         = new TLabel();
    TLabel                *lblSLKWarning        = new TLabel();
    TLabel                *lblOrgPick           = new TLabel();
    TEdit                 *edOrgPick1           = new TEdit();
    TEdit                 *edOrgPick2           = new TEdit();
    TPanel                *palHeightCalibration = new TPanel();
    TPanel                *pnlFuncSelect        = new TPanel();
    TCheckBox             *chkShuttle           = new TCheckBox();
    TCheckBox             *cbRTCModel           = new TCheckBox();
    TLabel                *lblContactOfsMM      = new TLabel();
    TLabel                *lblOrgHeightMM       = new TLabel();
    TPanel                *panHeightMode        = new TPanel();
    TfContactGroupBox     *rgHandlerMode        = new TfContactGroupBox();
    TLabel                *lbAutoContactTest    = new TLabel();
    TRadioButton          *rbModeNormal         = new TRadioButton();
    TRadioButton          *rbAutoHeight         = new TRadioButton();
    TRadioButton          *rbManualHeight       = new TRadioButton();
    TRadioButton          *rbContactTest        = new TRadioButton();
    TEdit                 *edAutoContactTest    = new TEdit();
    TfContactGroupBox     *gbContactStatus      = new TfContactGroupBox();
    TMemo                 *Memo1                = new TMemo();
    TfContactGroupBox     *gbHeightStatus       = new TfContactGroupBox();
    TLabel                *lblContact           = new TLabel();
    TLabel                *lblTorque            = new TLabel();
    TLabel                *lblArm1Sts           = new TLabel();
    TLabel                *lblArm2Sts           = new TLabel();
    TLabel                *lblContactMM         = new TLabel();
    TLabel                *lblTorque1           = new TLabel();
    TPanel                *palArm1Height        = new TPanel();
    TPanel                *palArm2Height        = new TPanel();
    TPanel                *PnlTorue0            = new TPanel();
    TPanel                *PnlTorue1            = new TPanel();
    TPanel                *palTestSocket        = new TPanel();
    TLabel                *labD41               = new TLabel();
    TComboBox             *coD41                = new TComboBox();
    TEdit                 *edD41                = new TEdit();
    TLabel                *lblICCheck           = new TLabel();
    TPanel                *pnlPurge             = new TPanel();
    TLabel                *lbAirPurgeBeforePickFromShuttle = new TLabel();
    TLabel                *lblPurgeTime         = new TLabel();
    TLabel                *lblPurgeInterval     = new TLabel();
    TLabel                *lblPurgeOffset       = new TLabel();
    TLabel                *lblPurgeTimeS        = new TLabel();
    TLabel                *lblPurgeIntervalT    = new TLabel();
    TLabel                *lblPurgeOffsetMM     = new TLabel();
    TfContactRadioGroup   *rgPurgeBeforePickShuttle = new TfContactRadioGroup();
    TEdit                 *edtPurgeBeforePickShuttleTime     = new TEdit();
    TEdit                 *edtPurgeBeforePickShuttleInterval = new TEdit();
    TEdit                 *edtPurgeBdforePickShuttleOffSet   = new TEdit();
    TLabel                *lblPinCount          = new TLabel();
    TEdit                 *edPinCount           = new TEdit();
    TEdit                 *edForcePerPinN       = new TEdit();
    TEdit                 *edForcePerPinG       = new TEdit();
    TLabel                *lblPerGf             = new TLabel();
    TLabel                *lblPerPinN           = new TLabel();
    TRadioButton          *rbAutoContactTest    = new TRadioButton();
    TRadioButton          *rbStepContactTest    = new TRadioButton();
    TButton               *btEditTray           = new TButton();
    TLabel                *labSetKg             = new TLabel();
    TEdit                 *edSetKg              = new TEdit();
    TLabel                *labTransfer          = new TLabel();
    TEdit                 *edTransfer           = new TEdit();
    TSpeedButton          *spbOneCycle          = new TSpeedButton();
    TfContactLed          *ledOneCycle          = new TfContactLed();
    TLabel                *labShtOffset         = new TLabel();
    TEdit                 *edShtPickOffset1     = new TEdit();
    TEdit                 *edShtPickOffset2     = new TEdit();
    TLabel                *lblOrgPickMM         = new TLabel();
    TLabel                *lblShtPickOfsMM      = new TLabel();
    TButton               *btOffset             = new TButton();
    TCheckBox             *cb2DMatrix           = new TCheckBox();
    TLabel                *lblEPValueKpa        = new TLabel();
    TLabel                *lblEPValueKg         = new TLabel();
    TPanel                *palSoakTimeWating    = new TPanel();
    TLabel                *lblWarning1          = new TLabel();
    TPanel                *palSoakTime          = new TPanel();
    TLabel                *lbHisiVer            = new TLabel();
    TButton               *btBarcode            = new TButton();
    TLabel                *lblAirForce          = new TLabel();
    TEdit                 *edAirKPA             = new TEdit();
    TLabel                *labMPa               = new TLabel();
    TfContactTimer        *TimerEP              = new TfContactTimer();
    TCheckBox             *cbSFCAutoTune        = new TCheckBox();
    TPanel                *palSFCInformation    = new TPanel();
    TPanel                *plIndexArmJogMove    = new TPanel();
    TLabel                *lblJogMove           = new TLabel();
    TButton               *btnIndexArmJogMove_Up   = new TButton();
    TButton               *btnIndexArmJogMove_Down = new TButton();
    TComboBox             *cbIndexArmJogMove_mm = new TComboBox();
    TLabel                *lblJogMoveMM         = new TLabel();
    TLabel                *lblSuckDevicesDuringTest = new TLabel();
    TfContactRadioGroup   *rgSuckDevicesDuringTest = new TfContactRadioGroup();
    TLabel                *lblIndexUpSpeed      = new TLabel();
    TfContactRadioGroup   *rgIndexUpSpeed       = new TfContactRadioGroup();
    TfContactRadioGroup   *rgOutKitDiameter     = new TfContactRadioGroup();
    TCheckBox             *chkUseAddWeight      = new TCheckBox();
    TPanel                *palTorqueArm1        = new TPanel();
    TPanel                *palTorqueArm2        = new TPanel();
    TCheckBox             *cbEnableUK           = new TCheckBox();
    TCheckBox             *chkTeachInOutArmZ    = new TCheckBox();
    TfContactRadioGroup   *rgSuckShuttleDeviceWaitOnShuttle = new TfContactRadioGroup();
    TLabel                *lblSuckShuttleDeviceWaitOnShuttle = new TLabel();
    TPanel                *pnlHandlerSatus      = new TPanel();
    TLabel                *labDelayStatus       = new TLabel();
    TLabel                *lblCountDown         = new TLabel();
    TButton               *btnTempSet           = new TButton();
    TLabel                *lblMinForce          = new TLabel();
    TRadioButton          *rbDeviceMapping      = new TRadioButton();
    TfContactGroupBox     *gbDoubleEPSet        = new TfContactGroupBox();
    TLabel                *lblDieForceEP        = new TLabel();
    TLabel                *lblLoadCellHeight    = new TLabel();
    TEdit                 *edLoadCellHeight1    = new TEdit();
    TEdit                 *edLoadCellHeight2    = new TEdit();
    TLabel                *lblLoadCellHighMM    = new TLabel();
    TRadioButton          *rbLoadCellAutoHigh   = new TRadioButton();
    TfContactRadioGroup   *rgDieForceKitDiameter = new TfContactRadioGroup();
    TLabel                *lblDieForce          = new TLabel();
    TEdit                 *edDoubleForce        = new TEdit();
    TLabel                *lblDieForceKg        = new TLabel();
    TLabel                *lblNoOfBall          = new TLabel();
    TEdit                 *edtPinOfDie          = new TEdit();
    TEdit                 *edAutoKSHTReleaseOfs = new TEdit();
    TLabel                *lblReleaseOfs        = new TLabel();
    TButton               *btnTrayMap           = new TButton();
    TLabel                *lblMaxForcePerIC     = new TLabel();
    TCheckBox             *chk2DID              = new TCheckBox();
    TSpeedButton          *AutoZTeachButton     = new TSpeedButton();
    TEdit                 *edOutDiameter        = new TEdit();
    TLabel                *lblUpWaitTime        = new TLabel();
    TEdit                 *edUpWaitTime         = new TEdit();
    TLabel                *lblUpSpeed           = new TLabel();
    TEdit                 *edUpSpeed            = new TEdit();
    TLabel                *lblUpOffset          = new TLabel();
    TLabel                *lblUpOfsMM           = new TLabel();
    TEdit                 *edUpOffset2          = new TEdit();
    TEdit                 *edUpOffset1          = new TEdit();
    TButton               *btTCPIP              = new TButton();
    TEdit                 *edDropByPassDetect   = new TEdit();
    TLabel                *labbUserDefMaxContactHeight = new TLabel();
    TPanel                *pnlSidePush          = new TPanel();
    TLabel                *lbTesterSidePush     = new TLabel();
    TfContactRadioGroup   *rgTesterSidePush     = new TfContactRadioGroup();
    TEdit                 *edSidePushWaitTime   = new TEdit();
    TLabel                *lblSidePushWaitTime  = new TLabel();
    TfContactRadioGroup   *rgSidePushMode       = new TfContactRadioGroup();
    TLabel                *lblSitePushMode      = new TLabel();
    TRadioButton          *rbTemp               = new TRadioButton();
    TLabel                *labEPValue           = new TLabel();
    TLabel                *labEPValue_1032      = new TLabel();
    TLabel                *lblReadEP2           = new TLabel();
    TCheckBox             *cbRTCAutoTuning      = new TCheckBox();
    TLabel                *lblDropByPassDetect  = new TLabel();
    TCheckBox             *chk_K_Temperature    = new TCheckBox();
    TRadioButton          *rbKTempIndexMove     = new TRadioButton();
    TRadioButton          *rbDeviceLoopTest     = new TRadioButton();
    TEdit                 *edDeviceLoopTest     = new TEdit();
    TCheckBox             *cbOneTouchAutoContactHight = new TCheckBox();
    TfContactTimer        *timerContact         = new TfContactTimer();
    TEdit                 *edDieForcePerPinN    = new TEdit();
    TEdit                 *edDieForcePerPinG    = new TEdit();
    TLabel                *Label1               = new TLabel();
    TLabel                *Label2               = new TLabel();
    TCheckBox             *cbTestContactMode    = new TCheckBox();
    TEdit                 *edTestContactCount   = new TEdit();
    TLabel                *lblNowCount          = new TLabel();
    TLabel                *lblCount             = new TLabel();
    TLabel                *lblTsetSec           = new TLabel();
    TEdit                 *edTestSec            = new TEdit();
    TCheckBox             *chkDailyCorrelation  = new TCheckBox();
    TLabel                *labContactRelative   = new TLabel();
    TEdit                 *edContactRelativeZ1  = new TEdit();
    TEdit                 *edContactRelativeZ2  = new TEdit();
    TCheckBox             *cbCalibrateAboveHeight = new TCheckBox();
    TCheckBox             *cbTeachInSHSen       = new TCheckBox();
    TLabel                *lblATCTempWait       = new TLabel();

    // -- golden event handlers, cContact.h:324-397.  Bodies translated where
    //    read-only; NOT wired to anything (DEVIATION D-10).
    void FormShow(TObject *Sender);                                             // golden :1078-1801  GATE (X-08)
    void edDropWaitTimeKeyPressInt(TObject *Sender, char &Key);                 // golden :1809-1813  ACTIVE
    void FormClose(TObject *Sender);                                            // golden :1815-1896  GATE (X-09); TCloseAction dropped (fCounterClear.h:71 precedent)
    void edPinCountChange(TObject *Sender);                                     // golden :1920-1925  GATE (X-03)
    void edForcePerPinNChange(TObject *Sender);                                 // golden :1927-1939  GATE (X-04)
    void edDieForcePerPinGChange(TObject *Sender);                              // golden :1941-1957  GATE (X-05)
    void scrbSLKChange(TObject *Sender);                                        // golden :2074-2081  GATE (S-48)
    void edPinCountKeyPress(TObject *Sender, char &Key);                        // golden :2083-2087  ACTIVE
    void edDropWaitTimeMouseDown(TObject *Sender, TMouseButton Button,
          TShiftState Shift, int X, int Y);                                     // golden :2089-2239  GATE (X-31)
    void Edit16MouseDown(TObject *Sender, TMouseButton Button,
          TShiftState Shift, int X, int Y);                                     // golden :2241-2249  GATE (X-24)
    void btnTStartClick(TObject *Sender);                                       // golden :2251-2254  ACTIVE
    void btnTStepClick(TObject *Sender);                                        // golden :2256-2259  ACTIVE
    void edContactHeight1Change(TObject *Sender);                               // golden :2261-2266  GATE (X-18)
    void edAirForceChange(TObject *Sender);                                     // golden :2268-2273  GATE (S-47)
    void btnStartClick(TObject *Sender);                                        // golden :13965-13970 GATE (S-12)
    void btnPauseClick(TObject *Sender);                                        // golden :13972-13975 GATE (S-13)
    void cbContactModeChange(TObject *Sender);                                  // golden :13977-14062 ACTIVE
    void pnlSensorAdjClick(TObject *Sender);                                    // golden :14064-14070 GATE (X-23)
    void spbSaveClick(TObject *Sender);                                         // golden :14072-14201 GATE (W-03)
    void sbtExitClick(TObject *Sender);                                         // golden :14478-14487 ACTIVE
    void btContactForceClick(TObject *Sender);                                  // golden :15012-15015 GATE (X-12)
    void btShowDynaTempClick(TObject *Sender);                                  // golden :15017-15021 GATE (X-13)
    void btAtClick(TObject *Sender);                                            // golden :15023-15027 GATE (X-14)
    void btTempOffsetClick(TObject *Sender);                                    // golden :15029-15033 GATE (X-15)
    void edSpeedZMouseDown(TObject *Sender, TMouseButton Button,
          TShiftState Shift, int X, int Y);                                     // golden :15035-15041 GATE (X-25)
    void edD41Click(TObject *Sender);                                           // golden :15043-15047 GATE (X-26)
    void coD41Change(TObject *Sender);                                          // golden :15049-15061 ACTIVE
    void edContactOffsetArm1MouseDown(TObject *Sender, TMouseButton Button,
          TShiftState Shift, int X, int Y);                                     // golden :15164-15168 ACTIVE
    void palOTD_4Click(TObject *Sender);                                        // golden :15170-15193 GATE (S-15)
    void palOTD_6Click(TObject *Sender);                                        // golden :15195-15218 GATE (S-16)
    void OTDTimerTimer(TObject *Sender);                                        // golden :15220-15282 GATE (S-17)
    void rgKitDiameterClick(TObject *Sender);                                   // golden :15284-15319 GATE (S-49)
    void edD41MouseDown(TObject *Sender, TMouseButton Button,
          TShiftState Shift, int X, int Y);                                     // golden :15321-15328 ACTIVE
    void rbModeNormalClick(TObject *Sender);                                    // golden :15330-15339 GATE (X-11)
    void btEditTrayClick(TObject *Sender);                                      // golden :16853-16859 GATE (X-22)
    void spbOneCycleClick(TObject *Sender);                                     // golden :16861-16865 GATE (X-32)
    void edShtPickOffset1MouseDown(TObject *Sender, TMouseButton Button,
          TShiftState Shift, int X, int Y);                                     // golden :16867-16874 ACTIVE
    void btOffsetClick(TObject *Sender);                                        // golden :16876-16879 GATE (X-17)
    void btBarcodeClick(TObject *Sender);                                       // golden :16881-16884 GATE (X-19)
    void TimerEPTimer(TObject *Sender);                                         // golden :16886-16941 GATE (X-28)
    void labMPaDblClick(TObject *Sender);                                       // golden :16943-16948 ACTIVE
    void btnIndexArmJogMove_UpClick(TObject *Sender);                           // golden :16950-17000 GATE (S-21)
    void btnIndexArmJogMove_DownClick(TObject *Sender);                         // golden :17002-17052 GATE (S-22)
    void chkUseAddWeightClick(TObject *Sender);                                 // golden :17096-17100 GATE (S-50)
    void rgOutKitDiameterClick(TObject *Sender);                                // golden :17102-17137 GATE (X-06)
    void cbEnableUKClick(TObject *Sender);                                      // golden :17139-17182 ACTIVE
    void btnTempSetClick(TObject *Sender);                                      // golden :17184-17187 GATE (X-16)
    void edDropByPassDetectMouseDown(TObject *Sender, TMouseButton Button,
          TShiftState Shift, int X, int Y);                                     // golden :17189-17193 ACTIVE
    void edtPinOfDieMouseDown(TObject *Sender, TMouseButton Button,
          TShiftState Shift, int X, int Y);                                     // golden :18350-18355 ACTIVE
    void edDoubleForceMouseDown(TObject *Sender, TMouseButton Button,
          TShiftState Shift, int X, int Y);                                     // golden :18357-18369 ACTIVE
    void edAutoKSHTReleaseOfsMouseDown(TObject *Sender, TMouseButton Button,
          TShiftState Shift, int X, int Y);                                     // golden :18396-18400 ACTIVE
    void btnTrayMapClick(TObject *Sender);                                      // golden :18402-18406 GATE (X-21)
    void edForcePerDeviceKGClick(TObject *Sender);                              // golden :18408-18423 ACTIVE
    void edtPinOfDieKeyPress(TObject *Sender, char &Key);                       // golden :18425-18428 ACTIVE
    void AutoZTeachButtonClick(TObject *Sender);                                // golden :18430-18451 GATE (S-25)
    void edForcePerPinNMouseDown(TObject *Sender, TMouseButton Button,
          TShiftState Shift, int X, int Y);                                     // golden :18542-18563 ACTIVE (D-7)
    void btTCPIPClick(TObject *Sender);                                         // golden :19024-19027 GATE (X-20)
    void edForcePerPinGMouseDown(TObject *Sender, TMouseButton Button,
          TShiftState Shift, int X, int Y);                                     // golden :19029-19050 ACTIVE (D-7)
    // FormShortCut(TWMKey&, bool&)  golden :19226-19233 -- OMITTED, GATE (X-27).
    // Not even declared: `TWMKey` has zero port, so a declaration would have to
    // invent or drop the parameter.  Four sibling facades made the same call
    // (cSetUp.cpp:250, uTemp_Set.cpp:6618, uYieldMonitoring.cpp:2614,
    // cBinSel.cpp:4222).
    void rgDieForceKitDiameterClick(TObject *Sender);                           // golden :19503-19506 ACTIVE
    void cbRTCAutoTuningClick(TObject *Sender);                                 // golden :20365-20378 ACTIVE
    void timerContactTimer(TObject *Sender);                                    // golden :21355-21366 ACTIVE

private:   // golden cContact.h:398-541 (User declarations)
    void LoadImage();                                                           // golden :1803-1807  GATE (X-07)
    bool bAutoHighFinish;
    int  iSpeedZ;
    int  iSpeedZSlow;
    int  iSpeed;
    int  TorqueData;
    bool MotorStatus;
    bool bContinueContact;
    TQPF_Timer hContactDeley;
    TQPF_Timer DriverDelay;
    int  RELEASE_UP_BIG;
    int  RELEASE_UP_SMALL;
    void Inital_ZTask();                                                        // golden :2275-2278  ACTIVE
    bool DoZ1PickFromShuttle();                                                 // golden :2487-3878  GATE (S-05)
    bool DoZ2PickFromShuttle();                                                 // golden :3880-5252  GATE (S-06)
    bool Do_Z1_AutoGetHeight();                                                 // golden :5389-8168  GATE (S-07)
    bool Do_Z2_AutoGetHeight();                                                 // golden :8174-10510 GATE (S-08)
    bool Do_ContactTest_32Site();                                               // golden :10513-11352 GATE (S-09)
    bool DoZPlaceToShuttle();                                                   // golden :11355-11739 GATE (S-10)
    bool CheckContactSettingChange();                                           // golden :17054-17094 ACTIVE (G-DimCmp)
    bool bOldRTCAutoTuning;                                                     // Sam 20240711 : RTC Auto Tuning

    // golden cContact.h:423-523 -- the four SM case enums, verbatim.
    enum eROILearningCase
    {
        rlcLightOn=1,                                                           //jou 2012-03-29 RTC啟動時,自動將燈箱打開
        rlcInitTask,
        rlcSendPassOn,
        rlcWaitPassOn,
        rlcSendReall,
        rlcSendReallDelay,
        rlcSendArmWork,
        rlcGetArmWork,
        rlcWaitReall,
        rlcSendPassOff,
        rlcWaitPassOff,
        rlcZ1DownZ2Up,
        rlcZ1UpZ2Down,
        rlcArmMoveCount,
        rlcSendArmIndex1,
        rlcWaitArmIndex1,
        rlcSendArmIndex2,
        rlcWaitArmIndex2,
        rlcArm2Down,
        rlcArm2Up,
        rlcDropICMoveZ,
        rlcDropICMoveY,
        rlcArmAwayForCheckSocket,
        rlcArm2BackToCenter,
        rlcWaitForCheckSocket,
        rlcGetFullOk,
        rlcGetHome,
        rlcArmOpen,
        rlcOnlyFullview,
        rlcTriggerAutoSTD,                                                      //JerryYang 20240829 : SPIL訓永 要求手動觸發RTC AUTO STD
        rlcResultAutoSTD,
    };

    enum eAutoContactTestCase
    {
        actInitial=1,
        actArm2ToCenter,
        actArm2ZDown,
        actZ1DownZ2Up,
        actStartDelayTimeArm1,
        actSOTArm1,
        actGetTestResult1,
        actArmMoveCount1,
        actZ1UpZ2Down,
        actStartDelayTimeArm2,
        actSOTArm2,
        actGetTestResult2,
        actArmMoveCount2,
        actDropICMoveZ,
        actDropICMoveY,
        actDropICAlarm,
        actGetHome,
        actArmOpen,
        actHasErrorBin,
        actFinish
    };

    enum eStepContactTestLoadCase
    {
        sctlInit=1,
        sctlStep1,
        sctlLoadTray,
        sctlEditTray,
        sctlEditTrayOK,
        sctlFindICOnLoad,
        sctlInXYToLoad,
        sctlInZToLoadPick,
        sctlInLoadPick,
        sctlInLoadPickErrZUp,
        sctlInLoadPickErrAlarm,
        sctlInZLoadUp,
        sctlInXYToShuttle,
        sctlInZShuttlePut,
        sctlInShuttlePut,
        sctlInZShutleUp,
        sctlInputDevice,
        sctlBottom2DInit,
        sctlBottom2DScan
    };

    enum eStepContactTestUnloadCase
    {
        sctuInit=1,
        sctuInXYToShuttle,
        sctuInZToShuttlePick,
        sctuInShuttlePick,
        sctuInShuttlePickErrZUp,
        sctuInShuttlePickErrAlarm,
        sctuInZShutleUp,
        sctuInSerachLoader,
        sctuInXYToLoad,
        sctuInZToLoadPut,
        sctuInLoadPut,
        sctuInZLoadUp,
        sctuInXYAway,
        sctuInLoadPutTstart,
        sctuInLoadPutTstartZsafe
    };

    int  iFTestBackItem[4][8];
    int  iBTestBackItem[4][8];

    bool TestZ1Y2OutRandge();                                                   // golden :283-306   GATE (S-01)
    void TestZ1Y2SetPos();                                                      // golden :308-315   GATE (S-02)
    bool TestZ2Y1OutRandge();                                                   // golden :317-341   GATE (S-03)
    void TestZ2Y1SetPos();                                                      // golden :343-350   GATE (S-04)
    void SetContactMode();                                                      // golden :15341-15434 GATE (X-10)
    void ChangeContactMode(bool ret);                                           // golden :15436-15454 ACTIVE
    int  itemp;                                                                 //短暫判斷使用 //JimmyChiu 20211028 : All speed can set by speed setting.
    bool brecordmsgLock;
    // golden cContact.h:536 `cDMP_RICHTEK dmp_RICHTEK;` -- OMITTED.
    // cDMP_RICHTEK (golden cContact.h:30-49) is a separate class whose three
    // real methods (TransformRICHTEKMap / DMP_RICHTEK_Finish /
    // GetMapTrayPosFromEncorder) are defined OUTSIDE cContact.cpp and are not
    // in this file's 134.  No delivered body here touches dmp_RICHTEK; porting
    // the class belongs to the wave that ports its methods.
    int  iSH1AutoLtcErr;                                                        //KenHsieh 20250811 : InSht sensor latch for contact
    int  iSH2AutoLtcErr;
    AnsiString ErrPart_InSH1Ltc;
    AnsiString ErrPart_InSH2Ltc;

public:    // golden cContact.h:542-664 (User declarations)
    int  iDoFullViewCheck;                                                      //ChungHung 20130122 add for SCK
    int  Z_Height_Task;
    int  Z_PickFromSocketTask;
    bool bSetupStart;

    TfContact();                        // PORT: fields + .dfm only -- DEVIATION (D-2)
    ~TfContact();                       // golden :279-281 (empty) + port-only widget frees (D-3)
    void Init();                        // golden ctor body :107-276

    // PORT-ONLY (D-3): every widget member above is an OWNING raw pointer freed
    // by the destructor, so an implicit copy would double-free.  Golden's TForm
    // base is non-copyable for the same structural reason.
    TfContact(const TfContact&) = delete;
    TfContact& operator=(const TfContact&) = delete;

    void ReadFile();                                                            // golden :363-718   (was GATE (W-01), retired 20260923)
    bool fShow;
    void DoIniDataToForm();                                                     // golden :720-1076  GATE (W-02)
    void ShowArmAndDeviceForce();                                               // golden :1898-1918 GATE (X-02)
    bool bSetupStep;
    bool bOneCycleFinish;
    double dDutCount;
    int  CarlibrationTask;
    int  iChkInSHLatchTask;                                                     //KenHsieh 20250811 : InSht sensor latch for contact

    void InitCarlibrationTask();                                                // golden :11748-11751 ACTIVE
    void OneCycleProcess();                                                     // golden :11741-11746 GATE (X-32)
    void DoTestContactFunction();                                               // golden :11755-13963 GATE (S-11)
    int  GetAutoHeightMaxKGTorque();                                            // golden :5273-5382  GATE (X-29)

    bool Do_ROILearning(bool bQuickLearn=false);                                // golden :14497-15010 GATE (S-46)
    void InitROILearningTask();                                                 // golden :14490-14494 ACTIVE
    bool Do_AutoContactTest(bool Reset=false);                                  // golden :15458-15876 GATE (S-18)
    bool DoStepContactLoadDevice(bool Reset=false);                             // golden :15878-16405 GATE (S-19)
    bool DoStepContactUnloadDevice(bool Reset=false);                           // golden :16407-16851 GATE (S-20)

    bool Do2DIDMapCheck(bool Reset=false);                                      // golden :20380-20812 GATE (S-37)
    bool Do2DIDMapCheckLoadDevice(bool Reset=false);                            // golden :20814-20882 GATE (S-38)
    bool Do2DIDMapCheckUnloadDevice(bool Reset=false);                          // golden :20884-21176 GATE (S-39)
    bool DoStepContactKitDevice(bool Reset=false);                              // golden :21431-21804 GATE (S-41)
    bool DoStepContactUnKitDevice(bool Reset=false);                            // golden :21807-22313 GATE (S-42)
    int  ROILearningTimes;                                                      //Steven 20120222 : ROI的學習次數
    void DutCount();                                                            // golden :1959-2072 ACTIVE + GATE (C-3)
    void InitDoFullViewCheck();                                                 // golden :15064-15067 ACTIVE
    bool DoFullViewCheck();                                                     // golden :15069-15162 GATE (S-14)
    void SaveSetupFile(AnsiString szDir, AnsiString S);                         // golden :14205-14476 GATE (W-04)
    bool Do_LoadCellAutoHigh(int iIndex);                                       // golden :17197-17997 GATE (S-23)
    bool bUniversalkitflag[2];
    bool bSetHasIC;                                                             //Steven 20190626 : Fixed 2DID error in contact test
    bool DoDeviceMapCheck(bool Reset=false);                                    // golden :17999-18337 GATE (S-24)
    int  iDeviceMapCheckTask;
    int  i2DIDCheckTask;                                                        //JerryYang 20250220
    int  i2DIDMapCheckLoadTask;
    int  i2DIDMapCheckUnloadTask;

    int  iDMCTrayX[MAX_SOCKET_TOTAL], iDMCTrayY[MAX_SOCKET_TOTAL], iDMCArm, iDMCChannel[MAX_SOCKET_TOTAL];
    void TransformDMCMap();                                                     // golden :19052-19062 GATE (S-28)
    void ResetDMCTrayData();                                                    // golden :18339-18348 ACTIVE
    bool bWaitLoadXY, bTrayFeed, bPlaceLoad;
    int  iTrayX, iTrayY;
    void CountDieForceKg(bool bByPinCount);                                     // golden :18371-18394 ACTIVE
    bool DoSocketSensorCheckRemainIC();                                         // golden :19235-19275 GATE (S-30)
    int  ROILearningTask;
    int  AutoContactTestTask;
    int  StepContactTestLoadTask;
    int  StepContactTestUnloadTask;
    void InitContactEdtList();                                                  // golden :356-361  ACTIVE (G-EmptyBodies)
    void InitContactCbList();                                                   // golden :352-354  ACTIVE (G-EmptyBodies)
    TStringList *sOutDiameter;                                                  //kevin 20210814 add OutDiameter data 40,50,75,108
    void ATC_SwitchTjSignal(int iIndexArm, bool bSnedArmDown=true);             // golden :18565-18626 GATE (S-26)
    void SetIndexDownPos();                                                     // golden :18628-18661 (was GATE (S-27), retired 20260923)
    int  GotIndexYSpeed(int _fixValue);                                         // golden :18663-18667 ACTIVE
    int  GotIndexZSpeed(int _fixValue);                                         // golden :18669-18673 ACTIVE
    double CalculateTotalAirForce(double dBallCount, double dSingleGf);         // golden :18675-18891 GATE (X-01)
    double GetMaxIndexForceLimit();                                             // golden :18893-18955 GATE (X-30)
    double GetMinForce(double dKitDiameter, int iTag);                          // golden :18957-19022 GATE (X-30)
    void TestZ_CompensationHight();                                             // golden :18453-18540 GATE (S-51)
    bool DoIndecxCHECkFunction(bool reset);                                     // golden :19067-19224 GATE (S-29)

    int  iDoArm1PlaceToShuttleTask;                                             //JimmyChiu 20220708 : add Index Arm Axis
    bool DoArm1PlaceToShuttle(bool bIsFirst);                                   // golden :19277-19382 GATE (S-31)
    int  iDoArm2PlaceToShuttleTask;
    bool DoArm2PlaceToShuttle(bool bIsFirst);                                   // golden :19384-19501 GATE (S-32)

    void ClearTTLSignal();                                                      // golden :19508-19521 GATE (S-33)
    double dMinForce;
    int  iRTCAutoTuningTask;                                                    //Sam 20230419
    void InitRTCAutoTuning();                                                   // golden :19853-19856 ACTIVE
    bool DoRTCAutoTuning();                                                     // golden :19859-20363 GATE (S-36)
    bool DoContactDeviceLoopTest(bool reset);                                   // golden :19525-19695 GATE (S-34)
    bool DoContactKTemperatureTest(bool reset);                                 // golden :19699-19851 GATE (S-35)

    bool RestoreData_2D_Check();                                                // golden :21178-21201 GATE (S-52)
    bool IsRun2DCheck();                                                        // golden :21203-21219 ACTIVE
    void Inital_Z1PickFromSocketTask();                                         // golden :21221-21224 ACTIVE
    bool DoZ1PickFromSocket();                                                  // golden :21227-21353 GATE (S-40)

    enum e2DIDCheckStep                                                         //JerryYang 20250220 : 2DID硬體順序檢查功能
    {
        eSHT1RowA=0,
        eSHT1RowB,
        eSHT2RowA,
        eSHT2RowB,
        eBottomBarCode1_1,
        eBottomBarCode1_2,
        eBottomBarCode2_1,
        eBottomBarCode2_2,
        eCOMPLETE
    };
    void ShowMSG(AnsiString asStr);                                             // golden :21368-21378 ACTIVE
    int  iStepContactTestKitTask;                                               //KaiChen 20200525
    int  iStepContactTestUnKitTask;
    bool bUseTwoArm;                                                            //KaiHuang 20200606 : For ASE-CL Daily Correlation
    int  iRunWhichArm;
    void UpdateContactRelative();                                               // golden :22315-22323 ACTIVE (D-8)
    int  iTaskCaliAboveHeight;
    bool DoCalibrateAboveHeightZ1(bool Reset=false);                            // golden :22326-22461 GATE (S-43)
    bool DoCalibrateAboveHeightZ2(bool Reset=false);                            // golden :22463-22606 GATE (S-44)

    int  iAutoTeachInSHLtcStatus;                                               //KenHsieh 20250909
    enum eInSHLtcStatus
    {
        eTeachSHLtcNomal=0,
        eTeachSH1LtcAlarm,
        eTeachSH1LtcFin,
        eTeachSH2LtcAlarm,
        eTeachSH2LtcFin
    };

    bool CheckInShuttleSensor_Latch_Contact();                                  // golden :22617-22760 GATE (S-45)
    void InitChkInSHLatchTask();                                                // golden :22608-22615 ACTIVE

    // DEVIATION (D-5): golden TForm::Close()/Show()/ShowModal()/BringToFront()
    // -- permanent offline no-ops (no window, no modal loop).  Only Close() is
    // reached from delivered code (sbtExitClick, golden :14486).
    void Close()       {}
    void Show()        {}
    void ShowModal()   {}
    void BringToFront(){}
};

// golden cContact.h:667 `extern PACKAGE TfContact *fContact;` is DELIBERATELY
// NOT declared here -- the global name `fContact` is currently owned by
// `TfContactShim` (atester_shims.h:251) with 10 live member surfaces.  See the
// banner section "THE GLOBAL fContact IS ALREADY TAKEN".


// AI(W906-CT-READFILE) 20260923: the tree's only TfContact instance, defined in
// forms/fContact.cpp.  DELIBERATELY NOT NAMED `fContact` -- that name belongs to
// `TfContactShim *fContact` (atester_shims.h:251) and the note at :93-100 above
// still holds: re-pointing it is a consolidation owned by a wave that owns
// atester_shims.{h,cpp}.  This pointer adds a reachable object without changing
// what any existing TU means.
// Today's only caller is wb_serve's boot, which does Init() then ReadFile().
extern TfContact *fContactForm;

#endif // FORMS_FCONTACT_H
