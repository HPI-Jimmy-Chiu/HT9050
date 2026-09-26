// =============================================================================
//  forms/fTeach.h  --  non-VCL facade for golden's TfTeach (uteach.h / uteach.cpp)
//  -- the machine TEACH form (手動教導畫面).
//
//  AI(W906-FW-TEACH-W25) 20260826: new file, FW wave FW-TEACH-W25.
//  Golden source of truth: HT9011UC_Code_V3.33.906.0_20260618/uteach.cpp
//  (6,056 lines) + uteach.h (2,400 lines), both cp950, decoded with
//  io.open(p, encoding='cp950'), 0 U+FFFD, this wave (20260826).
//
//  ┌───────────────────────────────────────────────────────────────────────┐
//  │  THIS WAVE IS READ-ONLY-DIRECTION ONLY.  NO MACHINE MOTION, NO WRITE. │
//  └───────────────────────────────────────────────────────────────────────┘
//  TfTeach is the form that MOVES the machine to record positions and then
//  PERSISTS them to teach.ini / tech.dat.  Both halves are deliberately OUT
//  OF SCOPE here (wave brief, and memory
//  ht9045-v899-teach-ini-migration-destroys-old-teach-data: this project has
//  already paid once for a teach-data write path that failed silently).
//  Every deferred method is listed with its reason in the DEFERRED REGISTER
//  below -- nothing was dropped without being named.
//
//
//  DENOMINATOR -- MEASURED THIS WAVE, NOT INHERITED
//  ==========================================================================
//  Golden uteach.cpp contains **156 top-level function definitions**:
//      146  TfTeach::            (golden's own class)
//        3  TECH_PARA::          (ctor + ReadFromFile + SaveToFile)
//        3  TECH_TWOPARA::       (ctor + ReadFromFile + SaveToFile)
//        2  TECH_SUCKPARA::      (ReadFromFile + SaveToFile; ctor is inline
//                                 in golden uteach.h:72)
//        1  TECH_MotorAxle::     (ctor)
//        1  free function        VerifyMotorAction() (golden :5346)
//  Method obtained by a comment/string-stripped, brace-matched scanner over
//  the cp950 source (the two traps in memory golden-span-extraction-brace-trap
//  -- `}` inside a comment, and whole functions inside /* */ -- are therefore
//  structurally excluded).  Cross-check: `grep -c "TfTeach::"` returns 147,
//  which is 146 definitions + ONE STRING LITERAL,
//  `MyDBIProcess("Exception", "TfTeach::FormDestroy")` at golden :4588.  The
//  wave brief quoted 147/3; the correct figures are 146/3 for those two
//  classes and 156 overall.  156 is the denominator used everywhere below.
//
//  THIS WAVE TRANSLATES 48 OF 156 (30.8%, unit = golden top-level function
//  definitions).  108 remain.
//
//
//  SELECTION RULE (stated so it is checkable, not "we picked some")
//  ==========================================================================
//  A method is IN this wave iff BOTH hold:
//    (R1) its golden body contains no machine action -- no MotorMove /
//         Gali_MotMove / Gali_MovePR / JogP / JogN / Gali_JogP / Gali_JogN /
//         ServoOnOff / Gali_Command / SetSpeed / SetGroup /
//         SetMotorScaleSpeed / InitProcessSingleMotorTask /
//         InitMOTParameter / StopAllMotor / IOBitOn / IOBitOff /
//         Send_Command_TTL / cylinder or vacuum actuation; AND
//    (R2) its golden body performs no persistence -- no WriteIniData /
//         WriteIniDataNoLog / WriteData / CheckAndReadIniData /
//         SaveEditTextToFile / SaveToFile / ReadFromFile / fopen.
//         ⚠ CheckAndReadIniData is on this list DELIBERATELY: the port's
//         `CheckAndReadIniData(...,int)` SEEDS THE MISSING KEY BY WRITING IT
//         (common.cpp:602-603, `if(!INIFile->ValueExists(...))
//         INIFile->WriteInteger(...)`).  So golden's *ReadFromFile* family is
//         a WRITE path against asTeachPath, not a read path, and is excluded
//         by (R2) like the Save* family.  This is the single most important
//         measurement in this wave and the reason all 9 TECH_* methods are
//         deferred.
//
//  Motor STATUS POLLING is NOT a machine action and is allowed: ScanMotorStatus
//  / Gali_ScanMotStatus / ReadPos / ReadEncoderPos / Gali_ReadPos /
//  Gali_ReadEncoderPos / MotionDone are query-only (Motor/mymotor.h:168/:199/
//  :171/:269, Motor/myGALILmotor.cpp).  Reading Sen[]/Cylinder[] sensors is
//  likewise a read.  Stated explicitly because two of the translated bodies
//  (IsCanQuickJogMove, ScanNowMotorStatus) do talk to the motion cards.
//
//  (R1)+(R2) admit exactly 65 of the 156.  The wave brief caps a batch at
//  30-50, so the 65 are split:
//    * ALL 26 non-keypad-launcher admitted methods are taken (exhaustive for
//      that shape -- not a sample).
//    * The keypad-launcher family (`fQwertyKey->ShowQwertyKey(...)`
//      one-liners) has 39 members, all admitted.  22 are taken -- golden's
//      single contiguous run :5159-5319 up to and including setEditTrayXClick;
//      the remaining 17 are deferred with exact line numbers below.  The cut
//      is by golden line order and is stated as such: the family is
//      homogeneous (one call, no control flow beyond two 2-way branches), so
//      splitting it costs nothing but honesty about where the line falls.
//  26 + 22 = 48.
//
//
//  ⚠ INTEGRATION REQUIREMENT -- REGISTER THIS FILE IN ht9045_sm, NOT
//  ht9045_forms
//  ==========================================================================
//  This wave does NOT touch CMakeLists.txt (brief).  The main loop must add
//  `forms/fTeach.cpp` to **ht9045_sm**, and must NOT add it to ht9045_forms.
//  Measured reasons, not preference:
//    * ht9045_forms links only vclcompat + ht9045_globals + ht9045_core
//      (CMakeLists.txt:714-721).
//    * This TU's bodies reach OUTSIDE that diet:
//        MOT[] / TTrayMotor / HTMotor      -> ht9045_motor (CMakeLists.txt:823-826,
//                                             Motor/mymotor.h:385)
//        Sen[] / Cylinder[]                -> ht9045_io    (CMakeLists.txt:963-973,
//                                             mysensor.h:48 / mycylin.h:176)
//        ShowMyMessage / ShowErrorMessage  -> canary_support.cpp (ht9045_sm)
//        IsEMGPressed                      -> csystem.cpp  (csystem.h:147, ht9045_sm)
//        fLtcSensor                        -> acarry_shims.cpp (ht9045_sm)
//    * ht9045_sm already links io + motor + globals + vclcompat + secsgem +
//      forms (CMakeLists.txt:2237-2248), so every one of the above resolves
//      there, and `fQwertyKey` (ht9045_forms) resolves over the DECLARED
//      sm->forms edge.  This is the same target that already holds cSpeed.cpp
//      (CMakeLists.txt:1957), whose FW-SPEED-W21 batch is this wave's shape
//      precedent.
//    * Putting it in ht9045_forms would recreate exactly the UNDECLARED
//      forms->sm archive edge CMakeLists.txt:618-626 documents having been
//      caught once already (libht9045_forms.a carrying 3 undefined refs that
//      only GNU ld's lazy extraction hid).
//  The alternative -- splitting the sm-reaching bodies into a root
//  `uteach.cpp` the way myQwertyKeyBoard.cpp/cSetUp.cpp do -- is NOT used
//  here because it would leave forms/fTeach.cpp holding ~4 trivial methods
//  and put 44 in the root file; the split earns nothing at this ratio.  If a
//  later wave translates the ctor / FormShow / InitialTeachEditList, revisit.
//
//
//  WHAT THIS FILE UNBLOCKS -- AND WHAT IT DOES **NOT**
//  ==========================================================================
//  Five `#if 0` gates elsewhere in the tree are blocked on `fTeach->fShow`
//  (csystem.cpp:840 / :1638 / :23112 "GATE G22", Command.cpp:10239 / :15027).
//  This file supplies that MEMBER.  It does NOT make any of them openable,
//  and no one should read "TfTeach now exists" as "GATE G22 can be un-gated":
//    * `fTeach` is a NULL pointer until some wave CreateForm-equivalents it
//      (see CTOR / Init() SPLIT).  Every one of those five sites DEREFERENCES
//      it unguarded, so un-gating today buys a segfault, not behaviour.
//    * csystem.cpp:851, inside the same `#if 0` as :840, calls
//      `VerifyMotorAction()`, which this wave DECLARES but deliberately does
//      NOT DEFINE ([MOTION], golden :5346-5426 calls StopAllMotor).
//    * csystem.cpp:1638 additionally needs `Zteach`, Command.cpp:10239/:15027
//      additionally need other form globals -- neither audited by this wave.
//  Three further gates -- cinitial.cpp:10902 (InitialTeachEditList),
//  cinitial.cpp:12250 (SaveFile), cinitial.cpp:15891 (ReadFile) -- stay
//  blocked BY DESIGN: all three are in the DEFERRED REGISTER, and the last two
//  are the teach-data WRITE path this wave exists to stay away from.
//
//  ⚠ STALE COMMENTS THIS FILE CREATES (outside this wave's write boundary,
//  listed so the main loop can retire them, NOT edited here):
//      cinitial.cpp:10891-10895, :12242, :15865-15874
//      csystem.cpp:20140, :23099-23103
//      TempCtrl/TriTemp.cpp:140
//  Each asserts some form of "TfTeach is ABSENT / fTeach has NO port
//  anywhere / 0 hits".  That was true when written and is false from this
//  commit onward.  The GATES themselves must stay closed for the reasons
//  above; only the justification text is out of date.
//
//  ✅ AI(W906-NL-ABSENCE) 20260916: **RETIRED -- all six sites are annotated now**
//  (commit cdd45e0, found by tools/absence_sentinel.py).  The surviving reason was
//  re-measured the same night and still holds: `git grep "new TfTeach|fTeach *= *new"`
//  returns nothing but this file's own `new TfTeachPageControl()` at :639, i.e.
//  **nobody constructs `fTeach`, so it is still a NULL pointer** and every one of
//  those sites dereferences it unguarded.  `VerifyMotorAction()` likewise still has
//  no definition (only the gated call at csystem.cpp:851).
//  The list above is kept verbatim as history -- do NOT re-run it as a TODO.
//
//
//  FORM CLASS SHAPE
//  ==========================================================================
//  Plain non-VCL class, NOT TForm-derived -- the posture of every translated
//  form facade in this tree (forms/fQwertyKey.h:294, forms/fSetup.h,
//  forms/fSecurity.h's own survey).  Widget stand-ins STORE DATA; they do not
//  render.  "Logic faithful, rendering = web."
//
//  CTOR / Init() SPLIT
//  --------------------------------------------------------------------------
//  The ctor stuffs FIELDS ONLY.  Concretely it carries golden's ctor lines
//  :257-266 (nine scalar field assignments) and :276-284 (two loops that
//  write only this object's own bool arrays) -- and nothing else.  Golden's
//  ctor line :267 onward touches widgets (`EdtTemp->Left=-200`), assigns
//  OnClick delegates, loads .BMP files from disk and builds the
//  TechPara/TechTwoPara/TechSuckPara registries; ALL of that is deferred (see
//  the ctor's DEFERRED entry).
//
//  `Init()` IS DELIBERATELY **NOT DECLARED** THIS WAVE.  An empty `Init()`
//  would be a stub that satisfies a future caller's link while doing none of
//  golden's work -- form (2) of the archive-extraction trap in
//  docs/KNOWLEDGE.md ("stub 先滿足需求"), which `nm --undefined-only` cannot
//  see.  The wave that translates golden :267-995 declares and defines it
//  then.  Nothing in this tree can call it in the meantime.
//
//  Why there is no SIOF exposure at all: the global below is a bare pointer,
//  zero-initialized and NEVER constructed at static-init time (golden's own
//  pre-CreateForm state, same as forms/fQwertyKey.cpp:41-42).  Contrast
//  memory ht9045-v906-homecoming-siof-lottery / fLaserSensor's ctor calling
//  InitLaserEdtList() with elLaser NULL -> 88 of 134 ctest SEGFAULTs.
//
//  NOT WIRED
//  --------------------------------------------------------------------------
//  Event-handler BODIES are translated; no OnClick / OnChange / OnMouseDown /
//  OnDblClick delegate is assigned anywhere (stock vclcompat widgets carry no
//  delegate slot -- forms/fTemp_Set.h GATE(G-Delegate), the tree-wide idiom).
//  Wiring is the web write-path layer's job.  Consequence, stated because it
//  matters for safety review: nothing in this tree can reach any body below,
//  so the NULL-pointer exposures listed under GOLDEN NOTES are unreachable
//  today and MUST be resolved by whichever wave wires them.
//
//
//  NO EXISTING PORT -- ABSENCE CLAIM WITH ITS COMMAND AND TIME
//  ==========================================================================
//  Measured 20260826 with `Grep "\bfTeach\b" --glob *.{h,cpp,hpp}` over
//  d:\HT9045\HT9011UC_Cpp_V3.33.906.0 (6 files hit) plus a preprocessor-aware
//  `#if 0` tracer over each hit:
//    * csystem.cpp:840, :1638, :23112       -- ALL inside `#if 0`
//    * Command.cpp:10239, :15027            -- ALL inside `#if 0`
//    * cinitial.cpp:10902, :12249-12250, :15889-15891 -- ALL inside `#if 0`
//    * TempCtrl/TriTemp.cpp:479             -- TU-LOCAL `struct W7TT_TfTeach`
//      with its own TU-local `W7TT_FTeach` pointer.  DIFFERENT NAMES, no ODR
//      relationship to this class or this global.
//    * tools/dfm2rc/**/uteach_layout.gen.cpp -- dfm2rc DATA artifact (string
//      literals "fTeach.pnlMotion..."), not a declaration.
//  So there are ZERO compiled declarations of `TfTeach` or `fTeach` today and
//  this file introduces the first.  Also measured 0 compiled hits for
//  `TECH_PARA` / `TECH_TWOPARA` / `TECH_SUCKPARA` / `TECH_MotorAxle`
//  (this wave declares none of them, but the names are recorded as unclaimed
//  for the wave that does).  `VerifyMotorAction` has exactly one textual hit,
//  csystem.cpp:851, itself inside the same `#if 0` as :840.
//  ⚠ Absence claims go stale under parallel waves (memory
//  wave-agents-stale-absence-claims).  Both measurements were RE-RUN at wave
//  close; see the wave report.  Sibling waves were concurrently creating
//  forms/fTrayAssignment.* and forms/fMotorTest.* -- neither declares any
//  name in this file.
//
//  KNOWN-PRESENT ports this file USES (so nothing here is a re-invention):
//    fQwertyKey / TfQwertyKey::ShowQwertyKey  forms/fQwertyKey.h:370, :406
//    TfLedValue (TALed stand-in)              forms/FormWidgets.h:272
//    vclcompat::TPageControl / TTabSheet      vclcompat/Controls.h:497 / :506
//    vclcompat::clYellow                      vclcompat/LedCore.h:60
//    TfLtcSensor *fLtcSensor                  acarry_shims.h:76, :90
//  NO NEW SHIM IS CREATED FOR ANY CROSS-FILE GAP (brief rule 5).  Where a
//  golden dependency has no port, the method is deferred, not stubbed.
//
//
//  SCOPE TABLE -- 48 ACTIVE (every body read line-by-line in full)
//  ==========================================================================
//  Line spans are golden uteach.cpp, signature line .. closing brace line.
//
//  (a) Pure computation / string assembly -- 9
//      GetMinXPitchCaptionName        :2014-2021
//      GetMaxXPitchCaptionName        :2023-2030
//      GetMinYPitchCaptionName        :2032-2039
//      GetMaxYPitchCaptionName        :2041-2048
//      GetMinXPitchMM                 :2050-2056
//      GetMaxXPitchMM                 :2058-2064
//      GetMinYPitchMM                 :2066-2069
//      GetMaxYPitchMM                 :2071-2074
//      CheckEditNumber                :3702-3725
//
//  (b) Pure predicates -- the interlock GUARDS themselves -- 3
//      CheckCanMove                   :2457-2468
//      CheckShuttleCanMove            :5428-5484
//      IsCanQuickJogMove              :1036-1221
//
//  (c) Status read -> display -- 3
//      ScanNowMotorStatus             :1322-1357
//      GetTechPos                     :3474-3515
//      btnRecordY1Click               :3697-3700
//
//  (d) UI state only -- 8
//      LockAllButton                  :5321-5329
//      AllBtnUp                       :5331-5343
//      Label2Click                    :4295-4298
//      PageControl2Change             :3625-3628
//      btClearMemoClick               :4710-4714
//      btnSetToClick                  :2098-2102
//      btnSetToOffsetClick            :2200-2203
//      pnlEncoderPosDblClick          :3044-3047
//
//  (e) Coordinate arithmetic from UI values (compute, never write back) -- 2
//      GetPitchOffSet                 :2956-2989
//      SetComputeInSh2Click           :5147-5157
//
//  (f) Read-only data listing -- 1
//      btnGetLatchClick               :4753-4777
//
//  (g) fQwertyKey numeric-keypad launchers, golden run :5159-5319 -- 22
//      edtMoveToClick                 :5159-5165
//      setEditZ1AClick                :5167-5170
//      setEditInXPitch40Click         :5172-5175
//      setEditInY15Click              :5177-5180
//      setEditOutXPitch40Click        :5182-5185
//      setEditOutY15Click             :5187-5190
//      setEditIndex1ToSocketYClick    :5192-5195
//      setEditIndex2ToSocketYClick    :5197-5201
//      setEditIndex1ToSht1ZClick      :5203-5206
//      setEditInSht1LeftClick         :5208-5211
//      setEditInSht2LeftClick         :5213-5216
//      setEditInXClick                :5218-5221
//      setEditInYClick                :5223-5226
//      setEditRotateAClick            :5228-5231
//      edtEditRotateInBacklashClick   :5233-5236
//      edShtCheckRangeClick           :5238-5241
//      SetEditPickLoaderClick         :5243-5249
//      setEditInZSafeHeightClick      :5251-5254
//      setEditOutXClick               :5256-5259
//      setEditOutYClick               :5261-5264
//      setFix3LClick                  :5266-5269
//      setEditTrayXClick              :5271-5274
//
//
//  GATE REGISTER (statements gated INSIDE a translated body)
//  ==========================================================================
//  (T-G1)  NONE.  There is no `#if 0` inside any of the 48 bodies: the
//          selection rule kept out every method that would have needed one.
//          Recorded as an explicit zero so a reader does not go looking.
//
//
//  DEFERRED REGISTER -- 108 of 156, every one named with its reason
//  ==========================================================================
//  Categories:  [MOTION] fails (R1).  [WRITE] fails (R2).  [BOTH] fails both.
//               [DEP] read-only but needs a golden dependency with no port in
//                     this tree (deferring is required by brief rule 5 -- no
//                     new shims).  [SIZE] read-only and portable, but out of
//                     this wave's declared batch.
//
//  --- golden's helper classes (9) ---------------------------------------
//  [WRITE] TECH_PARA::ReadFromFile          :77-116   CheckAndReadIniData seeds asTeachPath
//  [WRITE] TECH_PARA::SaveToFile            :118-157  WriteIniData x6
//  [WRITE] TECH_TWOPARA::ReadFromFile       :180-189  CheckAndReadIniData seeds asTeachPath
//  [WRITE] TECH_TWOPARA::SaveToFile         :191-208  WriteIniData x2
//  [WRITE] TECH_SUCKPARA::ReadFromFile      :220-233  CheckAndReadIniData seeds asTeachPath
//  [WRITE] TECH_SUCKPARA::SaveToFile        :235-252  WriteIniData in a 2x8 loop
//  [DEP]   TECH_PARA::TECH_PARA             :61-75    writes SetEdit/funButton/btGo ->Visible
//                                                     on widget members this wave does not
//                                                     declare; lands with the registry wave
//  [DEP]   TECH_TWOPARA::TECH_TWOPARA       :159-178  same shape as above
//  [DEP]   TECH_MotorAxle::TECH_MotorAxle   :210-218  same shape (+ ->GroupIndex/->Tag/
//                                                     ->AllowAllUp; AllowAllUp has no port)
//
//  --- the ctor (1, PARTIAL) ---------------------------------------------
//  [DEP]   TfTeach::TfTeach                 :254-995  PARTIALLY translated: golden :257-266
//          and :276-284 are carried (pure own-field writes -- see CTOR/Init()
//          SPLIT above).  DEFERRED remainder :267-275 and :285-995 --
//          `EdtTemp->Left=-200`, `gbOutSh1/2->Visible=false`, four
//          `SetButton06x->OnClick=SetButton140Click` delegate assignments (no
//          delegate slot -- G-Delegate), the 34-element InZ/OutZ/SortZEditPtr
//          wiring, 9 `Image*->Picture->LoadFromFile(GrapicPath+"*.BMP")` disk
//          reads (no TPicture port), and the whole TechPara/TechTwoPara/
//          TechSuckPara/TechMotorAxle registry construction ending at
//          `TECH_MAX_ITEM=TechPara.size()` (:993-994).  NOT counted as
//          translated in the 48.
//
//  --- machine action (R1) ----------------------------------------------
//  [MOTION] btnJogPMouseDown                :997-1034   JogP / Gali_JogP
//  [MOTION] btnJogNMouseDown                :1223-1260  JogN / Gali_JogN
//  [MOTION] btnJogPMouseUp                  :1262-1273  PCIL132_StopMotor / Gali_Command("ST")
//  [MOTION] UpdateMotorTeachMonitor         :1275-1292  MOT[Index].SetSpeed (drive parameter write)
//  [MOTION] UpdateMotorTeachTwoMonitor      :1294-1310  MOT[Index].SetSpeed
//  [MOTION] ScrollBar1Change                :1312-1320  MOT[].SetSpeed(Speed/3,true)
//  [MOTION] Timer1Timer                     :1359-1411  ProcessSingleMotorHome / DoGaliHome / DoPitch_Home
//  [MOTION] btnMovePClick                   :2104-2131  MotorMove / Gali_MovePR
//  [MOTION] btnHomeClick                    :2133-2198  ServoOnOff / StopAllMotor / SetSpeed / home task init
//  [MOTION] btnMoveNClick                   :2205-2232  MotorMove / Gali_MovePR
//  [MOTION] btnMoveToClick                  :2391-2407  MotorMove / Gali_MotMove
//  [MOTION] BtnPanel1Click                  :2409-2434  MyLaneIO.IOBitOn/Off (raw output actuation)
//  [MOTION] edtSpeedChange                  :2442-2455  MOT[].SetSpeed
//  [MOTION] DoAutoTech                      :2470-2917  auto-teach state machine (body is one
//                                                        big golden /* */ block plus live tail;
//                                                        motion either way)
//  [MOTION] btnArm1YServoClick              :2919-2940  Gali_Command("MOX"/"SHX") -- servo on/off
//  [MOTION] btnStopClick                    :2948-2954  StopAllMotor + Gali_Command("ST")
//  [MOTION] DoAutoTechPitch                 :2991-3042  MotorMove loops
//  [MOTION] spOutArmPitchClick              :3049-3058  iGali_SingalHomeTask=1 (arms a home task)
//  [MOTION] Button39Click                   :3060-3065  arms bAutoGetHomeSensor2Position
//  [MOTION] Button40Click                   :3067-3072  arms bAutoGetHomeSensor2Position
//  [MOTION] BtnPanelLane1Click              :3074-3129  MyLaneIO.IOBitOn/Off + clears HomeFlag
//  [MOTION] SetButton140Click               :3360-3399  ServoOnOff(false/true) around a modal
//  [MOTION] GoButton140Click                :3401-3472  InitMOTParameter / SetSpeed / MotorMove
//  [MOTION] SetButton020Click               :3517-3557  ServoOnOff(false/true)
//  [MOTION] GoButton020Click                :3559-3623  SetSpeed / MotorMove / Gali_MotMove
//  [MOTION] MotorTrayXClick                 :3630-3648  -> UpdateMotorTeachMonitor (SetSpeed)
//  [MOTION] SetButton064Click               :3650-3695  ServoOnOff(false/true)
//  [MOTION] btnZ1ServoClick                 :4253-4268  Gali_Command("MOY"/"SHY")
//  [MOTION] btnZ2ServoClick                 :4270-4285  Gali_Command("MOZ"/"SHZ")
//  [MOTION] btnServoClick                   :4287-4293  ServoOnOff
//  [MOTION] btnInZAllUpClick                :4466-4478  InitProcessSingleMotorTask (starts Z home)
//  [MOTION] btnOutZAllUpClick               :4480-4492  InitProcessSingleMotorTask
//  [MOTION] btnSortZAllUpClick              :4494-4503  InitProcessSingleMotorTask
//  [MOTION] DoZHome                         :4505-4540  drives the Z home sequence
//  [MOTION] btnOutZAllDownClick             :4542-4562  MotorMove
//  [MOTION] btnAutoTeachZClick              :4593-4597  bLoadInarmAutoHigh=true (opens vacuum) + modal
//  [MOTION] SpeedButtonInRotatePos90Click   :4605-4655  MotorMove
//  [MOTION] SpeedButtonInRotateNeg90Click   :4657-4707  MotorMove
//  [MOTION] btnSht1GoLatchClick             :4721-4735  InitMOTParameter / SetSpeed / MotorMove
//  [MOTION] btnSht2GoLatchClick             :4737-4751  InitMOTParameter / SetSpeed / MotorMove
//  [MOTION] spTTLResetClick                 :4951-4954  fMain->GetTTLState() -- RS232 to the TTL board
//  [MOTION] cbEnableTTLButtonUseClick       :4956-4962  fMain->Send_Command_TTL("@WENBU...")
//  [MOTION] SpeedButton1Click               :5034-5050  -> UpdateMotorTeachMonitor (SetSpeed)
//  [MOTION] VerifyMotorAction (free fn)     :5346-5426  StopAllMotor + LockAllButton/AllBtnUp
//  [MOTION] btnInClosePitchClick            :5486-5516  pitch motor move
//  [MOTION] btnInOpenPitchClick             :5518-5548  pitch motor move
//  [MOTION] btnPitchTestClick               :5550-5556  arms the pitch test loop
//  [MOTION] btn_InPitchX_13Click            :5558-5603  pitch motor move
//  [MOTION] btn_InPX_13HomeClick            :5605-5649  pitch home
//  [MOTION] btn_OutPitch_13Click            :5651-5696  pitch motor move
//  [MOTION] DoPitch_Home                    :5698-5782  pitch home state machine
//  [MOTION] Set_Pitch_SetGroup              :5784-5801  Motor->SetGroup (motion-card group config)
//  [MOTION] btn_PitchX1_3_LoopClick         :5817-5832  arms the pitch loop
//  [MOTION] DoPitchLoopMove                 :5834-5927  pitch loop motion
//  [MOTION] tmr_PitchLoopTimer              :5929-5932  -> DoPitchLoopMove
//  [MOTION] TriTemp_Teach                   :5934-5946  SetMotorScaleSpeed(i,10) for every motor
//  [MOTION] btnSetAllInArmZ_MoveClick       :5948-5971  MotorMove over the whole Z grid
//  [MOTION] btnSetAllOutArmZ_MoveClick      :5973-5996  MotorMove over the whole Z grid
//
//  --- persistence (R2) --------------------------------------------------
//  [WRITE] btnSaveClick                     :2261-2389  the teach SAVE button (-> SaveFile)
//  [WRITE] SaveFile                         :4924-4949  WriteData("d:\HT9045\system\tech.dat")
//                                                        + WriteIniData + SaveEditTextToFile.
//                                                        ⚠ THE teach-data write path.  Note the
//                                                        hard-coded absolute path: `--dry` cannot
//                                                        redirect it (same shape as cprod.cpp:1701/
//                                                        :1729 ReadLastDataFile/WriteLastDataFile).
//  [WRITE] ReadFile                         :4781-4920  CheckAndReadIniData -> seeds asTeachPath
//  [WRITE] SetIndexYPhasePosition           :4964-5024  WriteIniData(asGeneralPath, "IndexDriver", ...)
//                                                        -- writes the SHARED production Gerneral.ini
//                                                        (memory ht9045-gerneral-ini-normalized-20260817)
//  [BOTH]  UpdateTempTech                   :2234-2259  overwrites the live Tech.* teach globals from
//                                                        the edit boxes; it is the staging half of the
//                                                        save pipeline, so it is treated as a write
//  [BOTH]  btnSetAllInArmZClick             :4300-4365  rewrites the Z edit grid + home flags
//  [BOTH]  btnSetAllOutArmZClick            :4367-4435  as above
//  [BOTH]  btnSetAllSortArmZClick           :4437-4464  as above
//  [BOTH]  setEditLimitClick                :3728-4251  523-line dispatcher: opens the keypad AND
//                                                        writes Tech/limit values back
//  [BOTH]  sbAutoCalibrationInArmCCDResolutionClick   :5055-5077  CCD auto-calibration (moves + stores)
//  [BOTH]  sbAutoCalibrationOutArmCCDResolutionClick  :5080-5102  as above
//  [BOTH]  sbAutoCalibrationInArmCCDPickerDisClick    :5105-5122  as above
//  [BOTH]  sbAutoCalibrationOutArmCCDPickerDisClick   :5125-5142  as above
//
//  --- read-only but blocked by a missing dependency ---------------------
//  [DEP]   FormShow                         :1413-2012  599 lines; needs the registry the ctor
//                                                        builds, plus fSecurity / elTeach /
//                                                        HTEditList visibility plumbing
//  [DEP]   FormClose                        :2076-2096  calls btnStop->Click() (=> StopAllMotor,
//                                                        i.e. also [MOTION]) and
//                                                        myLog.Do_Log(Sender,...) which needs the
//                                                        TObject* Sender this port drops
//  [DEP]   FormDestroy                      :4564-4591  MyDBIProcess / registry teardown
//  [DEP]   InitialTeachEditList             :3131-3358  builds the HTEditList edit registry (elTeach)
//  [DEP]   InitialFormOncetime              :5998-6014  needs FrmAOI->ttbInsp (TTopBottomInspect)
//                                                        AND pushes TECH_TWOPARA registry entries
//  [DEP]   btnMotorTestClick                :2436-2440  fMotorTest->ShowModal() -- a sibling wave is
//                                                        creating forms/fMotorTest.* right now; wiring
//                                                        it here would race that file
//  [DEP]   btnIOClick                       :2942-2946  fiosetview->ShowModal()
//  [DEP]   sbBarCodeClick                   :4599-4603  fBarCode->Show()
//  [DEP]   pnlExitClick                     :5026-5032  MessageDlg(...) + Close(); `mrNo` has exactly
//                                                        one compiled home (cinitial.cpp) and no header
//                                                        declaration reachable from forms/
//  [DEP]   sbShuttleSensorClick             :6016-6029  fSecurity->Insufficient(45) + fCCLink->ShowUseSensor
//
//  --- read-only, portable, simply out of this batch (17 keypad launchers) -
//  [SIZE]  EditSh1SpeedClick                :4716-4719
//  [SIZE]  setLoaderZClick                  :5276-5279
//  [SIZE]  setEditHingeRotateLoaderClick    :5281-5284
//  [SIZE]  setEditBuffer1XClick             :5286-5289
//  [SIZE]  setEditBuffer1ZClick             :5291-5294
//  [SIZE]  setEditLoadPortZClick            :5296-5299
//  [SIZE]  setEditTrayBracketSaftZClick     :5301-5304
//  [SIZE]  setEditUnloadPortBufferZClick    :5306-5309
//  [SIZE]  setEditStackedLoaderXClick       :5311-5314
//  [SIZE]  setEditStackedLoaderZClick       :5316-5319
//  [SIZE]  edtOutSortShtClick               :5804-5807
//  [SIZE]  edtsetSortXPitchClick            :5809-5812
//  [SIZE]  InSHZDownRangeClick              :6031-6034
//  [SIZE]  setEditInSh1LtcSenZ1Click        :6036-6039
//  [SIZE]  setEditInSh1LtcSenZ2Click        :6041-6044
//  [SIZE]  setEditInSh2LtcSenZ1Click        :6046-6049
//  [SIZE]  setEditInSh2LtcSenZ2Click        :6051-6054
//
//  COUNT CHECK (scripted, not eyeballed).  The register above has exactly one
//  row per deferred definition -- no duplicates, no omissions -- verified by
//  set-differencing the register's row names against the 108 names the
//  brace-matched scanner reports as not-taken (both directions empty):
//      [MOTION]  58
//      [SIZE]    17
//      [DEP]     14
//      [WRITE]   10
//      [BOTH]     9
//      -------------
//      total    108      and  48 translated + 108 deferred = 156.
//  A method with more than one disqualifying property is filed under its
//  STRONGEST one only ([BOTH] > [MOTION]/[WRITE] > [DEP] > [SIZE]); e.g.
//  FormClose is filed [DEP] for myLog.Do_Log(Sender,...) although it also
//  calls btnStop->Click() (=> StopAllMotor).  The category label is advisory
//  prose; the 156 / 48 / 108 figures are the ones to trust.
//
//
//  GOLDEN NOTES (faithful; recorded for the ledger, NOT fixed)
//  ==========================================================================
//  (G-a) ScanNowMotorStatus golden :1324 --
//            `static TALed *pLed[10]={ALed1, ..., ALed10};`
//        A FUNCTION-LOCAL STATIC initialized from *member* pointers: it
//        captures whichever instance first entered the function and keeps
//        those LEDs forever.  With one TfTeach instance (golden's own case)
//        this is invisible; with two it would drive the wrong form's LEDs.
//        Translated verbatim, `static` included.
//  (G-b) GetTechPos golden :3509-3512 -- `if(iTechSelMot[0]>=0)`.  Golden
//        never initializes iTechSelMot in its ctor and relies on
//        TObject's zero-fill, so before any SetButton020Click/SetButton064Click
//        the guard sees 0, and 0 is a VALID motor index (MInArmX).  The zero
//        state therefore READS MOT[0]'s encoder into EditPtr, it does not
//        skip.  This port reproduces the zero-fill exactly (member
//        initializer `{0,0}`) so the behaviour is golden's, not the port's.
//        Same class of hazard as memory
//        ht9045-v906-ungate-check-value-provenance ("0 is not a neutral else").
//  (G-c) GetMaxYPitchMM golden :2073 -- `return 60.0;` with golden's own
//        commented-out `(double)(IN_OUT_ARM_Y_PITCH_MAX*0.01)` beside it
//        (Jimmychiu 20221208 "依照已出機機台都是儲存Pitch=60").  The config
//        value is deliberately ignored.  Verbatim.
//  (G-d) GetPitchOffSet carries golden's own marker `//ChungHung 20140116
//        沒有用到` (dead in golden).  Its `m=(InArm80-InArm50)/3000.0` uses a
//        double divisor so the division is real, not integer -- checked
//        against memory ht9045-divide-safety-int-to-float-regression.  `r` is
//        left UNINITIALIZED on the two early-return paths in golden; those
//        paths return before touching it, so it is not a live bug, but g++
//        -Wmaybe-uninitialized may say otherwise.  Verbatim.
//  (G-e) IsCanQuickJogMove golden :1038-1040 -- `#ifdef SOFT_SIMULTE return
//        true; #else { ... } #endif`, with NO return after `#endif`.  Kept
//        verbatim including the #ifdef; SOFT_SIMULTE is undefined in this tree
//        (MachineType.h:48 has it commented out), matching golden's shipping
//        build.  Also golden :1210 shows the OutArm branch printing the
//        *InArm* message ("Please let InArm Z at home position first!!") --
//        a copy/paste slip in golden, preserved.
//  (G-f) IsCanQuickJogMove golden :1150 tests `Cylinder[C_TrayX_UpDown].Status`
//        (the commanded state) while every other cylinder test in the same
//        function uses `.OffSensor()` (the sensed state).  Golden's own
//        inconsistency; verbatim.
//  (G-g) The keypad launchers pass their bounds in golden's house order --
//        `ShowQwertyKey(edit, N_INTEGER, 0, true, PSoftLimitP, PSoftLimitN)`,
//        i.e. the LARGER bound into the parameter named `min`.  Two of the 22
//        invert even that (setEditInZSafeHeightClick :5253 passes (0,200);
//        setEditTrayXClick :5273 passes (PSoftLimitP,-5000)).  Both orders
//        clamp identically because CheckRange opens with
//        `if(Maximum<Minimum)` (MachineType.h:1524-1544).  Verbatim, argument
//        for argument -- identical finding to FW-SPEED-W21 (W21-b).
//  (G-h) edShtCheckRangeClick golden :5240 passes (500, 2500) -- min>max in
//        the *numeric* sense on a parameter pair already reversed by (G-g), so
//        the effective window is [500,2500] either way.  Verbatim.
//  (G-i) NULL-POINTER EXPOSURES INHERITED FROM GOLDEN, unreachable today only
//        because nothing is wired (see NOT WIRED):
//          * `fQwertyKey->` in all 22 launchers -- both globals stay NULL until
//            a wiring wave constructs them (forms/fQwertyKey.h:404-407).
//          * `MOT[x].Motor->PSoftLimitP` in 18 of the 22 -- `Motor` is a
//            HTMotor* filled by cinitial's motor table build.
//          * `EditPtr->Text` in btnSetToClick / btnSetToOffsetClick /
//            pnlEncoderPosDblClick / GetTechPos -- EditPtr is set by the
//            deferred SetButton*Click family, so it is NULL until they land.
//          * `fLtcSensor->` in btnGetLatchClick.
//        Golden guards none of these.  A wiring wave MUST resolve all four
//        before enabling any of these handlers.
//
//
//  PORT DEVIATIONS (all mechanical; each says what and why)
//  ==========================================================================
//  (D-1) `__fastcall` dropped everywhere (tree-wide convention).
//  (D-2) `TObject *Sender` DROPPED from every handler whose golden body never
//        reads it -- checked one by one: btnSetToClick, btnSetToOffsetClick,
//        pnlEncoderPosDblClick, Label2Click, PageControl2Change,
//        btClearMemoClick, btnRecordY1Click, btnGetLatchClick,
//        SetComputeInSh2Click.  Same rule as FW-SPEED-W21 (W21-D1).
//  (D-3) For the 22 launchers `Sender` IS read (golden casts it), so it is
//        KEPT and typed as golden's cast target: `TEdit *Sender`, collapsing
//        golden's `(TEdit *)Sender` into the signature.  FW-SPEED-W21 (W21-D2).
//  (D-4) `TALed *` -> `TfLedValue *` (forms/FormWidgets.h:272).  Only ->Value
//        is touched by the translated body.
//  (D-5) `PageControl2->ActivePage` (a TTabSheet* in real VCL) has no
//        equivalent on vclcompat::TPageControl, which carries only
//        ActivePageIndex.  Handled by COMPOSING, not by forking vclcompat:
//        TfTeachPageControl below adds the one member (the
//        forms/fQwertyKey.h:262 TfQwertyKeyPanel precedent).
//  (D-6) golden `pnlStop->Color=(bLock)?clYellow:(TColor)0x00DFD9CC;` --
//        `clYellow` is spelled `vclcompat::clYellow` (vclcompat/LedCore.h:60);
//        `TColor` resolves to the global `typedef int TColor` in cmydef.h:16.
//        Value 0x00DFD9CC kept as golden's literal.
//  (D-7) Only the widget members the 48 bodies actually touch are declared.
//        This is NOT the full golden __published list (golden uteach.h:97-2295
//        is ~1,750 controls).  Adding a member is a later wave's job; nothing
//        here reads a .dfm-designer value, so the "0 is not a neutral else"
//        provenance hazard (vclcompat/Controls.h:238-254) does not arise --
//        every field the translated code reads is written by golden code
//        first, or is a golden ctor default reproduced below.
//  (D-8) `Init()` not declared -- see CTOR / Init() SPLIT.
//
//
//  ENCODING: UTF-8, bare LF, 0 U+FFFD (asserted at write time).
// =============================================================================
#ifndef FORMS_FTEACH_H
#define FORMS_FTEACH_H

#include "vclcompat/vcl_compat.h"     // AnsiString, TObject
#include <vector>                    // TechPara / TechTwoPara / TechMotorAxle 的容器
#include "vclcompat/Controls.h"       // TEdit/TPanel/TSpeedButton/TMemo/TPageControl/TTabSheet
#include "forms/FormWidgets.h"        // TfLedValue (golden TALed stand-in) -- :272
#include "forms/fTeachPara.h"        // TECH_PARA / TECH_TWOPARA / TECH_MotorAxle /
                                     // TECH_SUCKPARA（AI(W906-TEACH-W1) 20260919）

// ---------------------------------------------------------------------------
//  TfTeachPageControl -- facade-only wrapper (DEVIATION (D-5)).
//  vclcompat::TPageControl carries ActivePageIndex only; golden's
//  Label2Click assigns `PageControl2->ActivePage=tsAxleCtrl;` (uteach.cpp:4297)
//  and golden's MotorTrayXClick (deferred) compares `ActivePage==TabSheet14`.
//  Compose, don't fork vclcompat -- forms/fQwertyKey.h:262 precedent.
//  Offline: storing a page pointer changes nothing on screen; there is no
//  window.  The value is kept so golden's statement holds literally and is
//  observable to a test.
// ---------------------------------------------------------------------------
class TfTeachPageControl : public vclcompat::TPageControl
{
public:
    TTabSheet *ActivePage = nullptr;   // golden TPageControl::ActivePage
    virtual ~TfTeachPageControl() {}
};

// ===========================================================================
//  TfTeach -- non-VCL facade (golden uteach.h:94, `class TfTeach : public TForm`)
// ===========================================================================
class TfTeach
{
public:
    TfTeach();   // PORT: golden ctor :257-266 + :276-284 ONLY -- see banner

    // -- widget stand-ins, named by their .dfm leaf name (DEVIATION (D-7)) --
    //    Motion panel (golden uteach.h:97-140)
    TEdit        *edtNowPosition = new TEdit();          // golden uteach.h:116
    TEdit        *edtSetToOffset = new TEdit();          // golden uteach.h:120
    TPanel       *pnlEncoderPos  = new TPanel();         // golden uteach.h:136
    TSpeedButton *btnMoveP       = new TSpeedButton();   // golden uteach.h:104
    TSpeedButton *btnMoveN       = new TSpeedButton();   // golden uteach.h:105
    TSpeedButton *btnHome        = new TSpeedButton();   // golden uteach.h:106
    TSpeedButton *btnMoveTo      = new TSpeedButton();   // golden uteach.h:125
    TSpeedButton *btnSetTo       = new TSpeedButton();   // golden uteach.h:118
    TSpeedButton *btnJogP        = new TSpeedButton();   // golden uteach.h:762
    TSpeedButton *btnJogN        = new TSpeedButton();   // golden uteach.h:761
    TPanel       *pnlStop        = new TPanel();         // golden uteach.h:1441

    //    Status LEDs (golden uteach.h:126-138, TALed -> TfLedValue, (D-4))
    TfLedValue *ALed1  = new TfLedValue();
    TfLedValue *ALed2  = new TfLedValue();
    TfLedValue *ALed3  = new TfLedValue();
    TfLedValue *ALed4  = new TfLedValue();
    TfLedValue *ALed5  = new TfLedValue();
    TfLedValue *ALed6  = new TfLedValue();
    TfLedValue *ALed7  = new TfLedValue();
    TfLedValue *ALed8  = new TfLedValue();          // golden uteach.h:133
    TfLedValue *ALed9  = new TfLedValue();          // golden uteach.h:134
    TfLedValue *ALed10 = new TfLedValue();          // golden uteach.h:138

    //    Tab plumbing (golden uteach.h:140/:173)
    TfTeachPageControl *PageControl2 = new TfTeachPageControl();
    TTabSheet          *tsAxleCtrl   = new TTabSheet();

    //    Latch memo pair (golden uteach.h:355/:366)
    TMemo *Memo1 = new TMemo();
    TMemo *Memo2 = new TMemo();

    //    X-pitch calibration edits (golden uteach.h:1304-1305/:1327-1328)
    TEdit *setEditInXPitch40   = new TEdit();
    TEdit *setEditInXPitch120  = new TEdit();
    TEdit *setEditOutXPitch40  = new TEdit();
    TEdit *setEditOutXPitch120 = new TEdit();

    //    In-shuttle-2 placement-offset compute strip (golden uteach.h:185/:191/:749/:750)
    TEdit *setEditInSht2X = new TEdit();
    TEdit *setEditInSht2Y = new TEdit();
    TEdit *setEditInarmPlacementXOffsetByBasicSuck = new TEdit();
    TEdit *setEditInarmPlacementYOffsetByBasicSuck = new TEdit();

    // -- golden public: User declarations (uteach.h:2318-2392) --------------
    //    Only the members the 48 translated bodies need, plus the scalars
    //    golden's ctor :257-266 writes.  Defaults reproduce BCB6
    //    TObject zero-fill where golden's ctor is silent (see GOLDEN NOTE (G-b)).
    bool  fShow = false;                     // golden uteach.h:2323 (ctor :261)
    TEdit *EditPtr  = nullptr;               // golden uteach.h:2324 (zero-fill)
    TEdit *EditPtr1 = nullptr;               // golden uteach.h:2325 (zero-fill)
    int   ActiveMotorIndex    = -1;          // golden uteach.h:2326 (ctor :259)
    int   TwoActiveMotorIndex = -1;          // golden uteach.h:2327 (ctor :260)
    int   OldIndex = 0;                      // golden uteach.h:2328 (zero-fill)
    bool  ActiveFlag[3] = {false, false, false};  // golden uteach.h:2329 (zero-fill) 0 single 1 Two 2 Eight

    bool  fTechAuto  = false;                // golden uteach.h:2341 (ctor :262)
    int   iTechIndex = 1;                    // golden uteach.h:2342 (ctor :263)
    int   iTechTask  = 1;                    // golden uteach.h:2343 (ctor :264)

    bool  bAutoGetHomeSensor2Position  = false;   // golden uteach.h:2373 (ctor :265)
    int   iGetHomeSensor2PositionTask  = 1;       // golden uteach.h:2374 (ctor :266)
    bool  bUseIniFile = false;               // golden uteach.h:2382 (ctor :257) Steven 20240429 : Teach轉成INI

    TSpeedButton *PtrGoBtn020 = nullptr;     // golden uteach.h:2384 (zero-fill; AllBtnUp NULL-guards it)
    TSpeedButton *PtrGoBtn140 = nullptr;     // golden uteach.h:2385 (zero-fill; AllBtnUp NULL-guards it)

    // -- golden private: User declarations (uteach.h:2297-2316) -------------
    //    PORT: collapsed to public.  Golden's `private:` cannot be honoured
    //    while only half the class is translated -- the deferred SetButton*
    //    family (public in golden) is what writes iTechSelBot/EditPtr today,
    //    and a future web write-path layer reaches the predicates directly.
    //    Same collapse as forms/fQwertyKey.h DEVIATION (D-6).
    bool bInArmZHome[2][8];                  // golden uteach.h:2303 (ctor :280)
    bool bOutArmZHome[2][8];                 // golden uteach.h:2304 (ctor :281)
    bool bSortArmZHome[2];                   // golden uteach.h:2305 (ctor :283) RogerYang 20250416 for HT9046AU add
    int  iTechSelMot[2] = {0, 0};            // golden uteach.h:2302 (zero-fill -- see GOLDEN NOTE (G-b))

    // =======================================================================
    //  (a) Pure computation / string assembly -- 9
    // =======================================================================
    AnsiString GetMinXPitchCaptionName(AnsiString Str = "");   // golden :2014-2021
    AnsiString GetMaxXPitchCaptionName(AnsiString Str = "");   // golden :2023-2030
    AnsiString GetMinYPitchCaptionName(AnsiString Str = "");   // golden :2032-2039
    AnsiString GetMaxYPitchCaptionName(AnsiString Str = "");   // golden :2041-2048
    double     GetMinXPitchMM();                               // golden :2050-2056
    double     GetMaxXPitchMM();                               // golden :2058-2064
    double     GetMinYPitchMM();                               // golden :2066-2069
    double     GetMaxYPitchMM();                               // golden :2071-2074
    AnsiString CheckEditNumber(int iMax, int iMin, AnsiString asCurrent);  // golden :3702-3725

    // =======================================================================
    //  (b) Pure predicates -- the interlock guards -- 3
    // =======================================================================
    bool CheckCanMove();                          // golden :2457-2468
    bool CheckShuttleCanMove(int iMotorIndex);     // golden :5428-5484
    bool IsCanQuickJogMove();                      // golden :1036-1221

    // =======================================================================
    //  (c) Status read -> display -- 3
    // =======================================================================
    void ScanNowMotorStatus();                     // golden :1322-1357
    void GetTechPos();                             // golden :3474-3515
    void btnRecordY1Click();                       // golden :3697-3700 (D-2)

    // =======================================================================
    //  (d) UI state only -- 8
    // =======================================================================
    void LockAllButton(bool bLock);                // golden :5321-5329
    void AllBtnUp();                               // golden :5331-5343
    void Label2Click();                            // golden :4295-4298 (D-2)
    void PageControl2Change();                     // golden :3625-3628 (D-2)
    void btClearMemoClick();                       // golden :4710-4714 (D-2)
    void btnSetToClick();                          // golden :2098-2102 (D-2)
    void btnSetToOffsetClick();                    // golden :2200-2203 (D-2)
    void pnlEncoderPosDblClick();                  // golden :3044-3047 (D-2)

    // =======================================================================
    //  (e) Coordinate arithmetic from UI values -- 2
    // =======================================================================
    int  GetPitchOffSet(int WhichPitch, int Pos);   // golden :2956-2989
    void SetComputeInSh2Click();                    // golden :5147-5157 (D-2)

    // =======================================================================
    //  (f) Read-only data listing -- 1
    // =======================================================================
    void btnGetLatchClick();                        // golden :4753-4777 (D-2)

    // =======================================================================
    //  (g) fQwertyKey numeric-keypad launchers -- 22, golden run :5159-5319.
    //      All take `TEdit *Sender` per DEVIATION (D-3).
    // =======================================================================
    void edtMoveToClick(TEdit *Sender);                 // golden :5159-5165
    void setEditZ1AClick(TEdit *Sender);                // golden :5167-5170
    void setEditInXPitch40Click(TEdit *Sender);         // golden :5172-5175
    void setEditInY15Click(TEdit *Sender);              // golden :5177-5180
    void setEditOutXPitch40Click(TEdit *Sender);        // golden :5182-5185
    void setEditOutY15Click(TEdit *Sender);             // golden :5187-5190
    void setEditIndex1ToSocketYClick(TEdit *Sender);    // golden :5192-5195
    void setEditIndex2ToSocketYClick(TEdit *Sender);    // golden :5197-5201
    void setEditIndex1ToSht1ZClick(TEdit *Sender);      // golden :5203-5206
    void setEditInSht1LeftClick(TEdit *Sender);         // golden :5208-5211
    void setEditInSht2LeftClick(TEdit *Sender);         // golden :5213-5216
    void setEditInXClick(TEdit *Sender);                // golden :5218-5221
    void setEditInYClick(TEdit *Sender);                // golden :5223-5226
    void setEditRotateAClick(TEdit *Sender);            // golden :5228-5231
    void edtEditRotateInBacklashClick(TEdit *Sender);   // golden :5233-5236
    void edShtCheckRangeClick(TEdit *Sender);           // golden :5238-5241
    void SetEditPickLoaderClick(TEdit *Sender);         // golden :5243-5249
    void setEditInZSafeHeightClick(TEdit *Sender);      // golden :5251-5254
    void setEditOutXClick(TEdit *Sender);               // golden :5256-5259
    void setEditOutYClick(TEdit *Sender);               // golden :5261-5264
    void setFix3LClick(TEdit *Sender);                  // golden :5266-5269
    void setEditTrayXClick(TEdit *Sender);              // golden :5271-5274

    // =======================================================================
    //  (h) teach 參數登錄表 + 讀檔  --  AI(W906-TEACH-W1) 20260919
    //
    //  golden uteach.h:2349-2354 的四個登錄容器，加上 golden uteach.h:2379
    //  的 `ReadFile()`。這一組是 START 能不能按下去的唯一阻擋：
    //  `StartFromWeb` -> `CompareTechData()` 要 `Tech.*` 有值，而填 `Tech.*`
    //  的就是 `ReadFile()` -> `TechPara[i]->ReadFromFile()`。
    //  完整設計與偏離：`forms/fTeachPara.h` 檔頭。
    // =======================================================================
    std::vector<TECH_TWOPARA   *> TechTwoPara;      // golden uteach.h:2349
    std::vector<TECH_MotorAxle *> TechMotorAxle;    // golden uteach.h:2350
    std::vector<TECH_PARA      *> TechPara;         // golden uteach.h:2351
    TECH_SUCKPARA TechSuckPara[3];                  // golden uteach.h:2354  RogerYang 20250416 for HT9046AU add

    void ReadFile();                                // golden uteach.h:2379  kevin 20190305

    //AI(W906-TEACH-W1) 20260919: PORT-ONLY 方法，golden 沒有這個名字。
    // golden 把登錄表 654 行直接寫在 ctor 裡（uteach.cpp:342-994）；這裡拆成
    // 一個方法，由 ctor 在最後呼叫一次 —— 語意零差異（同物件、同順序、同時機），
    // 但避免把 654 行塞進 forms/fTeach.cpp 的 ctor 中間，那會推掉既有行號引用。
    // 本體是**產生的**：forms/fTeachRegistry.cpp，來源 tools/gen_teach_registry.py。
    void BuildTechRegistry();

    // GATED -- golden uteach.h:2378 `void SaveFile(bool bSaveByTeach);`
    // 缺相依：它逐項呼叫 TECH_*::SaveToFile，而那三支以 widget 為資料來源
    // （見 forms/fTeachPara.h 檔頭）。宣告都不放，避免有人以為可以呼叫。
};

// AI(W906-FW-TEACH-W25) 20260826: golden uteach.h:2395
// `extern PACKAGE TfTeach *fTeach;` -- PACKAGE glue dropped.  DEFINED
// (zero-initialized, NEVER constructed at static init -- golden's own
// pre-CreateForm state; golden uteach.cpp:41 is a bare `TfTeach *fTeach;`)
// in forms/fTeach.cpp.  This is the FIRST compiled declaration of the name
// anywhere in the tree; see the ABSENCE CLAIM in the banner.
extern TfTeach *fTeach;

// golden uteach.h:2396 `extern bool Teachshow;` -- golden uteach.cpp:45
// defines `bool Teachshow=false;`.  DECLARED here and DEFINED in
// forms/fTeach.cpp because it is uteach.cpp's global and this file is
// uteach.cpp's port; nothing in this tree reads it yet (0 compiled hits,
// measured 20260826).
extern bool Teachshow;

// golden uteach.h:2398 `extern bool VerifyMotorAction();` -- DECLARED ONLY,
// NOT DEFINED.  Its golden body (uteach.cpp:5346-5426) calls StopAllMotor()
// and is [MOTION]-deferred; its one call site, csystem.cpp:851, is inside the
// same `#if 0` as csystem.cpp:840.  Deliberately left as an undefined
// declaration so nothing can link against a lying stub (see CTOR / Init()
// SPLIT for the same argument applied to Init()).
bool VerifyMotorAction();

//AI(W906-W5-b) 20260925: W5-b 覆核 W5B-3 —— IsCanQuickJogMove（forms/fTeach.cpp:135）本體的兩個掛鉤與四支小函式
//  （定義在 forms/fTeach.cpp 檔尾；不插在前面是為了不推動該檔被別處引用的行號）。
//  * 為什麼：V906 預設建置有 SOFT_SIMULTE，golden 本體第一行 `return true;`（golden :1039）—— golden 的 SOFT_SIMULTE 建置
//    從不驅動真硬體所以安全；V906 的 1203 軸在同一個建置裡是實彈（0918 裁決甲）⇒ 被移動的是「真的會動的 1203 軸」時，
//    SOFT_SIMULTE 也跑本體（Z 在原點、MTrayX 閘門、TrayArm Z、氣缸在下點、CheckShuttleCanMove 飛梭閘門）。
//  * 1203 軸的狀態不讀 golden MOT[].ScanMotorStatus()／Led[iHomeLed]：那些軸是 EastSun 監看器開的，golden 的 m_Axishand 沒開
//    （讀了回 WAR16121 告警＋舊值）⇒ 讀監看器樣本的 motionIO ORG 位元；狀態不明（沒樣本、命令後還沒新樣本）一律當「不在原點」。
//    非 1203 軸照 golden 原本讀的。
//  * 掛鉤沒裝（單元測試、其他執行檔）⇒ 行為與 golden 完全相同。
extern bool (*W906_TeachLive1203Hook)(int motIndex);   // 1203 且控制層可用（真的會動）
extern int  (*W906_TeachHomeLedHook)(int motIndex);    // -2 不是 1203（走 golden）；1 在原點；0 不在；-1 不明
bool W906_TeachBodyInSim(int motIndex);                 // SOFT_SIMULTE 組態要不要跑本體
void W906_TeachScanMotorStatus(int motIndex);           // golden MOT[i].ScanMotorStatus()（1203 軸不打 golden handle）
void W906_TeachGaliScanMotStatus(int motIndex);         // golden MOT[i].Gali_ScanMotStatus()（同上）
bool W906_TeachHomeLed(int motIndex);                   // golden MOT[i].Led[iHomeLed]（1203 軸讀監看器 ORG；不明 = false）

#endif // FORMS_FTEACH_H
