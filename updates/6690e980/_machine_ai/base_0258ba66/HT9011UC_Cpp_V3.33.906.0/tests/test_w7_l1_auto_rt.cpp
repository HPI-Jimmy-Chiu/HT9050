// =============================================================================
//  test_w7_l1_auto_rt.cpp  --  W7-L1 Wave 2: asendic_Auto_RT
//
//  Translation wave: W906-W7-L1 Wave 2 (AUTO_RT track)
//  Author: AI(W906-W7-L1-AutoRT) 20260802
//
//  WHAT IS UNDER TEST
//  ------------------
//  The Auto1..Auto6 unloader-tray ROUND-TRIP (auto-retest) state machines --
//  asendic_Auto_RT.cpp, all 12 golden top-level functions -- driven over the Sim
//  HAL (Cylinder[] / MOT[] / Sen[] / SW[]) with no hardware and no window.
//
//  PROOF OBJECTIVE
//  ---------------
//  Every assertion below constrains a SPECIFIC observable, and the FAILABILITY
//  PROOF section at the end records a real mutation -- applied to a scratch copy
//  of the translated .cpp, rebuilt, re-run -- for every assertion FAMILY.  It
//  also names, explicitly, the handful of assertions that NO single mutation
//  reddens on its own; those are constraining but not individually proven, and
//  saying so is the point.  Four rules, each adopted because this project has
//  already shipped the corresponding defect:
//    1. TRAJECTORIES ARE EXACT AND ORDERED, never set membership.  Each drive
//       records the cursor after every tick, collapses consecutive duplicates,
//       and compares against an ordered expected list.
//    2. NO ASSERTION PUTS THE LOOP'S OWN TERMINATION CONDITION INTO ITS
//       DISJUNCTION.  There is no `done || steps == BOUND` anywhere here.
//       Convergence is asserted as its own separate CHECK.
//    3. REACHED IS NOT COMMANDED.  Offline, Push() and Pop() BOTH return true
//       and On()/Off() cannot fail, so a cursor trajectory cannot distinguish an
//       extend from a retract.  Every actuator claim therefore reads the REAL
//       SIM WIRE BIT (Cylinder[].GetOutBit()), and every actuator is PRE-DRIVEN
//       TO THE OPPOSITE of the expected value first, so "call deleted" fails the
//       same assertion as "call inverted".
//    4. ALARM ARMS ARE ENTERED FOR REAL.  SEVEN sub-tests ([6], [14], [17], [18],
//       [21], [23], [24]) drive a genuine ShowErrorMessage arm through the
//       canary_support seam; THREE of them ([14a], [17a], [23]) take the K_SKIP
//       branch, and two of those three are paired with their K_RETRY twin so the
//       SKIP-vs-RETRY divergence is asserted, not assumed.  Where an arm could
//       NOT be entered it is listed in the NOT-COVERED register, never silently
//       claimed.
//
//  WHY Cylinder[].GetOutBit() IS THE ACTUATOR ORACLE:  TMyCylinder::On()/Off()
//  set Status/Change/bCylinderOn unconditionally, but the IO write is inside the
//  .Enable-gated OnSwitch()/OffSwitch().  With OutISABase=eMotionNet + OutType=
//  TYPE_A + a non-all-zero (Ring,IP,Port,Bit), OnSwitch() calls
//  MyLaneIO.IOBitOn which does `OutPortData[..] |= MyBitMask[Bit]` BEFORE the
//  (stubbed) hardware write, and GetOutBit() -> IOOutBitStatus reads that same
//  cache back.  So the wire bit is a faithful record of the last command, and
//  TLaneIO::CheckPortRangeErr short-circuits to 0 because InitialOK is false
//  offline.  simWire() below installs that address; sub-test [2] proves it.
//
//  THREE FIXTURE SEAMS
//  -------------------
//  1. VIRTUAL TIME.  TQPF_Timer is a QueryPerformanceCounter wall clock with no
//     injection point, so a 20 s deadline can never be outlasted by a tight
//     loop.  The SM's eight file-scope timers are non-static, so the extern
//     declarations below reach them without touching either .h.  Each timer is
//     classified and advanced ONLY where its PROGRESS role is live:
//       AutoDelay[]                 DUAL-ROLE.  Progress: DoLoadNewEmptyTrayToCar_RT
//                                   case 400 (armed golden :49/:145).  WATCHDOG:
//                                   case 420's JAM1113 arm (golden :179).
//                                   Advanced ONLY while the cursor is on 400.
//       AutoRTDelay                 Progress only: DoTrayZAutoTrayToWait case 200
//                                   (armed golden :978, read :981).  NOTE it is a
//                                   SCALAR, not a per-Pos array like every other
//                                   timer in the file -- see GOLDEN DEFECT H.
//       hAutoTrayToFront[]          PURE 20 s JAM1112 WATCHDOG (golden :250).
//                                   Advanced ONLY in sub-test [14], which exists
//                                   to enter that alarm.
//       hAutoTrayToFrontForDummy[]  TRIPLE-ROLE.  5 s/10 s DUMMY-timeout at case
//                                   100 (golden :237), 10 s JAM1112 WATCHDOG at
//                                   case 101 (golden :277), 100 ms settle at
//                                   cases 103/104 (golden :286/:293).  Advanced
//                                   at 103/104 always; at 100 only in [13]/[25].
//       hAutoTrayToRear[]           DUAL-ROLE.  20 s JAM1101 WATCHDOG at case 200
//                                   (golden :377); LD_TrayArrivalDely settle at
//                                   460 (armed :430) and 0.5 s settle at 510
//                                   (armed :443).  Advanced ONLY at 460 / 510,
//                                   except in [17] which exists to fire the JAM.
//       hAutoTrayToRearForDummy[]   5 s DUMMY-timeout at case 200 only.  Not
//                                   needed by any driven path (the sensor arm
//                                   fires first) and never advanced.
//       DoUnLoadNewAutoToStackDelay[] Progress only (cases 45/50-ART/150/400/402).
//                                   Advanced freely.
//       hUnLoadSafetyCheckTimer[]   PURE 5 s JAM1158 WATCHDOG (golden :709).
//                                   Advanced ONLY in sub-test [21].
//  2. SIM SENSOR LEVELS.  Sen[] is TRI-STATE and this file uses all three.
//     `IsOn()` and `IsOff()` are NOT complements -- both are false when
//     Enable==false -- and the distinction is load-bearing here, because golden
//     tests `IsOff()==false` in five places, which "unknown" satisfies:
//       simSensorUnknown : Enable=false            -> IsOn 0, IsOff 0
//       simSensorOff     : Enable + Type=TYPE_A(1) -> IsOn 0, IsOff 1
//       simSensorOn      : Enable + Type=TYPE_B(0) -> IsOn 1, IsOff 0
//     TYPE_B is cmydef.cpp's DEFAULT (0), so "Enable=true" alone means ON.
//     Sub-test [2] proves all three, both directions each.
//  3. OPERATOR DIALOGS.  canary_support.h's W906_ShowErrorMessage_SimReturn /
//     _LastCode / _LastKCode / _Count and W906_ShowMyMessage_LastS1 / _Count.
//
//  ONE DELIBERATE NON-SM SEAM: sub-tests [18] and [24] WRITE A CURSOR DIRECTLY
//  (iAutoTrayToRearTask / iUnLoadNewAutoTrayTask).  That is not a back door
//  invented for the test -- golden main.cpp registers BOTH of them as writable
//  QueueTaskList entries (&iAutoTrayToRearTask[0..2] at main.cpp:9870-9873,
//  &iUnLoadNewAutoTrayTask[0..2] at main.cpp:9880-9882, re-derived by grepping
//  the decoded golden in this pass), i.e. the shipped operator Task List UI can
//  set exactly these ints to exactly these values.
//  Both sub-tests say so at the call site.  Without it, golden's DoAutoTrayToRear
//  cases 300/400 and DoUnLoadNewAutoToStack case 350's alarm arm are unreachable
//  (see GOLDEN DEFECT C and NOT-COVERED item B).
//
//  Offline: no vendor SDK, no hardware, no window.
//
//  ===========================================================================
//  NOT-COVERED REGISTER -- this is a deliverable, not an apology
//  ===========================================================================
//  Silence would read as coverage.  Each entry names what is not observed and
//  the fixture invariant (or missing substrate) responsible.  Sub-test [27]
//  ASSERTS this register's central accounting claim -- the exact alarm count, the
//  exact set of alarm codes, the exact dialog count -- and SEVEN separate
//  mutations below drive it red: (B3) and (G6) and (I5) change the alarm COUNT,
//  (B5) and (J5) and (L1) change the code SET, and (C1) changes the dialog count.
//  A register that cannot go red is decoration; this one can.
//
//  A. THE THREE AutoCylinder* HELPERS ARE NO-OP STUBS, SO NONE OF THIS FILE'S
//     25 LIFTER CALLS IS OBSERVABLE.  acatchtray_shims.cpp defines
//     AutoCylinderUp/Middle/Lower as `{ return true; }`; the real bodies
//     (golden asendic.cpp:562/:767/:937) are Wave 3, and this test must NOT add
//     a seam to a shim file it does not own.  Three consequences, all of them
//     real holes and none of them fixable here:
//       * NO ARGUMENT-ORDER CLAIM IN THIS SUITE.  The 21 common-order and 4
//         REVERSED-order sites (golden :783/:816/:856/:872) are indistinguishable
//         today.  What IS proven is the SELECTOR: sub-test [20] shows the
//         bARTUnloaderUseTwoCylin arm being taken (via the C_Auto_Selector wire
//         it drives directly), and [19] shows the plain arm.  The
//         bARTUnloaderUseOneCylin arm -- the one carrying three of the four
//         reversed calls -- reaches the SAME cursor (100) by the SAME route as
//         the plain arm, so entering it is unobservable and NOT claimed.
//       * GOLDEN DEFECT B (the hard-coded Part `1` at golden :983) is NOT
//         observable.  Sub-tests [9]-[11] run DoTrayZAutoTrayToWait at Pos==2
//         precisely so the bug is LIVE while they run, but nothing can see it.
//       * DoUnLoadNewAutoToStack case 401's IniConfig.bP37bAutoCylinderUP
//         short-circuit (golden :863-866) reaches the same cursor as the
//         else-limb, so it is not entered distinguishably and not claimed.
//  B. TrayMoveStatus() HAS NO SEAM BEFORE WAVE 3 -- its body is a hardwired
//     `return 0` (see the note above its declaration in asendic.h).  That makes
//     DoUnLoadNewAutoToStack's NON-DUMMY happy path STRUCTURALLY UNREACHABLE:
//     case 50's guard `TrayMoveStatus(Pos+3,__FUNC__)==0 && iRealDummy!=DUMMY`
//     (golden :756-757) is permanently true off DUMMY, so the SM ping-pongs
//     50 -> 45 -> 50 forever.  Sub-test [22] ASSERTS exactly that ping-pong --
//     the park is the observation, not a failure.  Everything downstream of it
//     in non-DUMMY (cases 100..402 with real belt behaviour, and case 350's
//     MES1121 arm on its natural path) is therefore NOT COVERED; [24] reaches
//     350's alarm only by writing the cursor.  TrayMoveIn/TrayMoveOut are empty
//     Sim no-ops for the same reason, so none of this file's 8 belt commands is
//     observable either.
//  C. THE TRAY DATA ARGUMENT OF MOT[].SetTray(data, Func) IS NOT OBSERVABLE HERE.
//     TTrayMotor::SetTray sets fHasTray=true unconditionally but only fills
//     Tray.Data `if(fHTary)`, and fHTary is PROTECTED and false unless SetSimuScreenPara ran (no caller of it is reached here, grep 20260930; AI(W906-HTRAY))
//     (Motor/mymotor.h:323, verified by compiling an access attempt in this
//     pass).  So `SetTray(HAS_IC,...)` at golden :456 and `SetTray(NULL_IC,...)`
//     at golden :60/:173/:242 are distinguishable only by their PRESENCE, not by
//     their payload.  Every assertion below reads fHasTray and says so; a
//     mutation that swaps HAS_IC for NULL_IC would NOT be caught, and that is
//     stated rather than implied.
//  D. GOLDEN DEFECT A IS COVERED, AND IT IS THE ONLY GOLDEN DEFECT THIS SUITE
//     CAN SEE.  InitTrayZAutoTrayToWaitTask() writing the LOADER's cursor
//     (golden :951) is pinned twice -- directly in [1] and as a live side effect
//     of the ASE path in [8].  Defects C/D/E/F/G/H (see the .cpp banner) are
//     reported but NOT asserted: C is asserted only in its reachable half ([18]
//     reaches 300/400 through the operator-cursor seam, which does not prove
//     they are unreachable normally -- that claim rests on the golden grep in
//     the .cpp banner, not on this test); D would require an out-of-bounds write
//     to demonstrate and is deliberately NOT triggered (every iReceiveAutoTray
//     sub-test uses Pos<=2); E/F/G/H are static/structural.
//  E. THE `#ifdef SOFT_SIMULTE` ARMS ARE NOT COMPILED IN THIS BUILD, and the
//     193-line commented-out DoAutoTray block (golden :475-667) is not code.
//  F. NOT ENTERED because their gate is off in every sub-test:
//       * DoLoadNewEmptyTrayToCar_RT case 420's JAM1113 arm (golden :181) -- the
//         DUMMY/sensor arm above it always fires first, and the arm's own
//         watchdog (AutoDelay) is never advanced at cursor 420 by rule.
//       * DoAutoTrayToFront case 101's JAM1112 arm (golden :279) -- reaching it
//         needs the CW-predetect/tray-detect pair to read IsOff()==true while
//         SnAutoTrayCar reads IsOff()==false, and no sub-test wants that state.
//       * bHandlerPause is false everywhere, so the three pause re-entry arms
//         (golden :162, :216, :348) and the case-402 timer re-arm (golden :884)
//         are not entered.
//       * DoUnLoadNewAutoToStack case 402's K_RETRY half is entered (the alarm
//         repeats) but its `bHandlerPause` companion is not.
//  G. `iAutoReceiveTask` (golden :27) is dead in golden itself (GOLDEN DEFECT E)
//     and is therefore not exercised; it is not even declared in the header.
//
//  ===========================================================================
//  FAILABILITY PROOF -- REPRODUCIBLE RECIPE
//  ===========================================================================
//  Everything below was run in a private scratch dir.  The shared build/ tree was
//  never written to.  Each mutation was applied to a COPY of the translated
//  asendic_Auto_RT.cpp, compiled off-tree with the flags CMake records for target
//  ht9045_sm (-std=c++1z -DCSYSTEM_TICK_ORACLE -DWINVER=0x0601 -D_WIN32_WINNT=
//  0x0601 -Wall -Wextra -I<tree> -I<tree>/SECSGEM), `ar r`'d into a COPY of
//  libht9045_sm.a, and linked into a private exe.  Each table row quotes the
//  ASSERTION MESSAGE, never a `(line N)` suffix -- the message is stable, the
//  line number is pushed by every edit to this banner.
//
//  BASELINE (asendic_Auto_RT.cpp pristine):  RESULT: 180 passed, 0 failed (exit 0)
//  The translated source was sha256-verified UNCHANGED after the whole run.
//
//  76 MUTATIONS, EACH REBUILT AND RE-RUN.  Format:
//     (tag) golden:LINE  what was changed  -> passed/failed
//  followed by the verbatim FAIL messages (first three; the count of the rest is
//  given).  75 of the 76 go RED; the one that stays GREEN is a CONTROL and is
//  explained under THE ONE GREEN, below.
//
//  (I1 ) golden:36   InitLoadNewAutoTrayToCarTask sets 2                  -> 144/36
//        FAIL: InitLoadNewAutoTrayToCarTask(1) -> cursor==1
//        FAIL: [3] DoLoadNewEmptyTrayToCar_RT returns non-zero (converges)
//        FAIL: [3] EXACT cursor trajectory 51,100,200,300,400,410,420,1
//        ... and 33 more
//  (I2 ) golden:951  GOLDEN DEFECT A "corrected"                          -> 177/3 
//        FAIL: GOLDEN DEFECT A: InitTrayZAutoTrayToWaitTask writes the LOADER cursor (==1)
//        FAIL: GOLDEN DEFECT A: it does NOT reset any iTrayZAutoTrayToWaitTask[] entry
//        FAIL: [8] GOLDEN DEFECT A is LIVE on this path: the ASE branch clobbered the LOADER cursor
//  (A1 ) golden:112  case 51 hand-off retargeted                          -> 178/2 
//        FAIL: [3] EXACT cursor trajectory 51,100,200,300,400,410,420,1
//        FAIL: [7] EXACT trajectory unchanged -- the pause is a one-tick DWELL on 200, not a detour
//  (A2 ) golden:173  case 420 car SetTray deleted                         -> 179/1 
//        FAIL: [3] case 420 latched a tray onto the Auto car (SetTray, golden :173)
//  (A3 ) golden:174  case 420 AutoZ ClearTray deleted                     -> 179/1 
//        FAIL: [4] AutoZ motor cleared at case 420
//  (A4 ) golden:63   case 1 short-circuit return                          -> 179/1 
//        FAIL: [5] short-circuit returns 1 in ONE tick
//  (A5 ) golden:61   case 1 short-circuit AutoZ ClearTray deleted         -> 179/1 
//        FAIL: [5] AutoZ cleared (golden :61)
//  (A6 ) golden:50   case 1 fast-path target                              -> 179/1 
//        FAIL: [4] EXACT trajectory 400,410,420,1 -- case 1 jumps STRAIGHT to 400, skipping 51/100/200/300
//  (B1 ) golden:85   else-arm C_Auto_Up inverted                          -> 179/1 
//        FAIL: [6a] golden :85 dropped C_Auto_Up (wire cleared)
//  (B2 ) golden:86   else-arm C_Auto_Selector inverted                    -> 179/1 
//        FAIL: [6a] golden :86 dropped C_Auto_Selector (wire cleared)
//  (B3 ) golden:87   else-arm break deleted -> falls to alarm             -> 178/2 
//        FAIL: [6a] the cylinder-down limb BREAKS before the alarm (golden :87)
//        FAIL: [27] EXACTLY 9 ShowErrorMessage calls across the suite -- the number the register accounts for
//  (B4 ) golden:92   MES1121 offers SKIP too                              -> 179/1 
//        FAIL: [6b] golden offers K_RETRY only -- no SKIP button (golden :92)
//  (B5 ) golden:92   MES1121 code swapped (also reddens [27])             -> 177/3 
//        FAIL: [6b] the alarm code is sMES1121[Pos] (== MES1221 for Auto2), proving Pos indexing
//        FAIL: [27] EXACTLY 7 distinct alarm codes -- MES1221, MES1321, JAM1113, JAM1201, JAM1203, JAM1212, JAM2558
//        FAIL: [27] the seven codes are exactly the seven the register names (no unlisted arm fired)
//  (C1 ) golden:128  case 200 bEmptyPause not cleared                     -> 175/5 
//        FAIL: [7] the split-pause path still converges
//        FAIL: [7] EXACT trajectory unchanged -- the pause is a one-tick DWELL on 200, not a detour
//        FAIL: [7] golden :128 cleared bEmptyPause (one-shot)
//        ... and 2 more
//  (C2 ) golden:129  split-wait dialog text                               -> 179/1 
//        FAIL: [7] the dialog is golden :129's "Auto split wait"
//  (D1 ) golden:69   ASE branch retargeted to the plain path              -> 177/3 
//        FAIL: [8] EXACT outer trajectory 50,1 -- the ASE branch NEVER visits 51/100/200/300
//        FAIL: [8] EXACT nested DoTrayZAutoTrayToWait trajectory 1,2,100,200,201,1000,1100,1200,1
//        FAIL: [8] returns exactly 2 -- the 'last tray, do not feed it' code (golden :105), NOT 1
//  (D2 ) golden:105  case 50 last-tray return code                        -> 179/1 
//        FAIL: [8] returns exactly 2 -- the 'last tray, do not feed it' code (golden :105), NOT 1
//  (D3 ) golden:103  case 50 AutoZ ClearTray deleted                      -> 179/1 
//        FAIL: [8] golden :103 cleared the AutoZ motor
//  (E1 ) golden:1041 case 1200 return code                                -> 177/3 
//        FAIL: [8] EXACT outer trajectory 50,1 -- the ASE branch NEVER visits 51/100/200/300
//        FAIL: [8] returns exactly 2 -- the 'last tray, do not feed it' code (golden :105), NOT 1
//        FAIL: [9] returns exactly 2 (golden :1041)
//  (E2 ) golden:1032 case 1100 AutoZ Pop -> Push                          -> 179/1 
//        FAIL: [9] case 1100 RETRACTED C_AutoZ_Select (wire cleared, golden :1032)
//  (E3 ) golden:976  case 100 AutoZ Push -> Pop                           -> 179/1 
//        FAIL: [10] case 100 EXTENDED C_AutoZ_Select (wire set, golden :976)
//  (E4 ) golden:1012 case 300 AutoZ Pop -> Push                           -> 179/1 
//        FAIL: [10] case 300 RETRACTED C_AutoZ_Select (wire cleared, golden :1012)
//  (E5 ) golden:1020 case 400 MMTrayZ latch deleted                       -> 179/1 
//        FAIL: [10] and latches MMTrayZ.fHasTray (golden :1020)
//  (E6 ) golden:1018 case 400 SwACTrayY gate inverted                     -> 175/5 
//        FAIL: [10] with SwACTrayY ON the SM walks 2,100,200,201,300,400 and PARKS on 400
//        FAIL: [10] parked on 400 -> still returns 0
//        FAIL: [10] parked -> MMTrayZ NOT latched
//        ... and 2 more
//  (E7 ) golden:995  case 201 last-tray hand-off                          -> 174/6 
//        FAIL: [10] with SwACTrayY ON the SM walks 2,100,200,201,300,400 and PARKS on 400
//        FAIL: [10] parked on 400 -> still returns 0
//        FAIL: [10] case 300 RETRACTED C_AutoZ_Select (wire cleared, golden :1012)
//        ... and 3 more
//  (E8 ) golden:1001 cover-tray dialog index                              -> 179/1 
//        FAIL: [11] the dialog text carries Pos+1 == 3 (golden :1001), proving Pos indexing
//  (F1 ) golden:214  explicit break after case 1 (kills the fall-through) -> 178/2 
//        FAIL: [12] EXACT trajectory 101,102,103,104,105,1 -- the FIRST entry is 101, which PINS the golden case 1 -> c
//        FAIL: [12] case 100 dropped C_AutoEdgePush AND C_AutoUpPress (golden :223-224)
//  (F2 ) golden:223  case 100 EdgePush inverted                           -> 179/1 
//        FAIL: [12] case 100 dropped C_AutoEdgePush AND C_AutoUpPress (golden :223-224)
//  (F3 ) golden:224  case 100 UpPress inverted                            -> 179/1 
//        FAIL: [12] case 100 dropped C_AutoEdgePush AND C_AutoUpPress (golden :223-224)
//  (F4 ) golden:273  case 101 EdgePush inverted                           -> 179/1 
//        FAIL: [12] case 101 EXTENDED C_AutoEdgePush (golden :273)
//  (F5 ) golden:284  case 102 SideFixer Push -> Pop                       -> 179/1 
//        FAIL: [12] case 102 EXTENDED C_AutoSide_Fixer (golden :284)
//  (F6 ) golden:294  case 103 SideFixer Off -> On                         -> 179/1 
//        FAIL: [12] case 103 RETRACTED C_AutoSide_Fixer (golden :294)
//  (F7 ) golden:301  case 104 EdgePush Off -> On                          -> 179/1 
//        FAIL: [12] case 104 RETRACTED C_AutoEdgePush (golden :301)
//  (F8 ) golden:306  case 105 Auto-motor ClearTray deleted                -> 179/1 
//        FAIL: [12] case 105 cleared BOTH the Auto motor and the Auto car (golden :306-307)
//  (F9 ) golden:307  case 105 Auto-car ClearTray deleted                  -> 179/1 
//        FAIL: [12] case 105 cleared BOTH the Auto motor and the Auto car (golden :306-307)
//  (G1 ) golden:242  case 100 DUMMY-arm car SetTray deleted               -> 177/3 
//        FAIL: [13] golden :242 SET a tray on the Auto car (this is what later lets DoAutoTrayReceive reach its case 20
//        FAIL: [25] EXACT trajectory 100,200,1
//        FAIL: [25] iReceiveAutoTray[Pos] was raised to 2 mid-chain (golden :923) -- the busy flag the rest of the hand
//  (G2 ) golden:243  case 100 DUMMY-arm motor ClearTray deleted           -> 179/1 
//        FAIL: [13] golden :243 cleared the Auto motor
//  (G3 ) golden:252  JAM1112 drops the SKIP button                        -> 179/1 
//        FAIL: [14a] golden offers BOTH buttons (golden :252)
//  (G4 ) golden:262  JAM1112 SKIP arm motor ClearTray deleted             -> 179/1 
//        FAIL: [14a] K_SKIP cleared BOTH tray flags (golden :262-263)
//  (G5 ) golden:263  JAM1112 SKIP arm car ClearTray deleted               -> 179/1 
//        FAIL: [14a] K_SKIP cleared BOTH tray flags (golden :262-263)
//  (G6 ) golden:253  JAM1112 SKIP/RETRY arms swapped                      -> 174/6 
//        FAIL: [14a] the K_SKIP arm returns true
//        FAIL: [14a] exactly one JAM1112 raised
//        FAIL: [14a] K_SKIP cleared BOTH tray flags (golden :262-263)
//        ... and 3 more
//  (H1 ) golden:340  explicit break after case 1 (kills the fall-through) -> 177/3 
//        FAIL: [15] EXACT trajectory 200,450,460,500,510,600,1 -- the FIRST entry is 200, which PINS the golden case 1 
//        FAIL: [15] case 100 RETRACTED C_AutoSide_Fixer (golden :342)
//        FAIL: [16] EXACT trajectory 200,1 -- it bails at case 200 and never reaches 450/500/600
//  (H2 ) golden:342  case 100 SideFixer Pop -> Push                       -> 179/1 
//        FAIL: [15] case 100 RETRACTED C_AutoSide_Fixer (golden :342)
//  (H3 ) golden:440  case 500 SideFixer Push -> Pop                       -> 179/1 
//        FAIL: [15] case 500 EXTENDED C_AutoSide_Fixer (golden :440)
//  (H4 ) golden:454  case 600 SideFixer Pop -> Push                       -> 179/1 
//        FAIL: [15] case 600 RETRACTED C_AutoSide_Fixer (golden :454)
//  (H5 ) golden:456  case 600 SetTray(HAS_IC) deleted                     -> 179/1 
//        FAIL: [15] case 600 LATCHED the tray onto the Auto motor (golden :456) -- this is the line asendic_Auto.cpp's 
//  (H6 ) golden:457  case 600 car ClearTray deleted                       -> 179/1 
//        FAIL: [15] case 600 cleared the Auto car (golden :457)
//  (H7 ) golden:338  case 1 EdgePush inverted                             -> 179/1 
//        FAIL: [15] case 1 dropped C_AutoEdgePush AND C_AutoUpPress (golden :338-339)
//  (H8 ) golden:339  case 1 UpPress inverted                              -> 179/1 
//        FAIL: [15] case 1 dropped C_AutoEdgePush AND C_AutoUpPress (golden :338-339)
//  (H9 ) golden:358  double-tray guard no longer returns true             -> 178/2 
//        FAIL: [16] the double-tray guard returns true
//        FAIL: [16] EXACT trajectory 200,1 -- it bails at case 200 and never reaches 450/500/600
//  (I3 ) golden:379  JAM1101 drops the SKIP button                        -> 179/1 
//        FAIL: [17a] golden offers BOTH buttons (golden :379)
//  (I4 ) golden:388  JAM1101 SKIP arm car ClearTray deleted               -> 179/1 
//        FAIL: [17a] K_SKIP cleared the Auto car (golden :388)
//  (I5 ) golden:380  JAM1101 SKIP/RETRY arms swapped                      -> 174/6 
//        FAIL: [17a] the K_SKIP arm returns true
//        FAIL: [17a] exactly one JAM1101 raised
//        FAIL: [17a] K_SKIP cleared the Auto car (golden :388)
//        ... and 3 more
//  (J1 ) golden:395  case 300 SideFixer Push -> Pop                       -> 179/1 
//        FAIL: [18a] case 300 EXTENDED C_AutoSide_Fixer (golden :395)
//  (J2 ) golden:415  case 400 alarm arm SideFixer Off -> On               -> 179/1 
//        FAIL: [18b] case 400's alarm limb RETRACTED C_AutoSide_Fixer (golden :415)
//  (J3 ) golden:411  case 400 DUMMY limb hand-off                         -> 179/1 
//        FAIL: [18a] EXACT trajectory 400,450,460,500,510,600,1 -- case 400's DUMMY limb rejoins at 450
//  (J4 ) golden:425  case 400 bFlag==false limb hand-off                  -> 179/1 
//        FAIL: [18c] with bFlag==false case 400 goes to 500, SKIPPING the 450 settle (golden :425)
//  (J5 ) golden:416  JAM1103 code swapped                                 -> 177/3 
//        FAIL: [18b] the code is sJAM1103[Pos] (== JAM1203 for Auto2)
//        FAIL: [27] EXACTLY 7 distinct alarm codes -- MES1221, MES1321, JAM1113, JAM1201, JAM1203, JAM1212, JAM2558
//        FAIL: [27] the seven codes are exactly the seven the register names (no unlisted arm fired)
//  (K1 ) golden:826  case 300 AutoZ Pop -> Push                           -> 179/1 
//        FAIL: [19] case 300 RETRACTED C_AutoZ_Select (wire cleared, golden :826)
//  (K2 ) golden:838  case 350 Auto-motor ClearTray deleted                -> 179/1 
//        FAIL: [19] case 350 cleared the Auto motor (golden :838)
//  (K3 ) golden:742  case 1 ART Selector On -> Off                        -> 179/1 
//        FAIL: [20] case 1's bARTUnloaderUseTwoCylin limb EXTENDED C_Auto_Selector (golden :742) -- this is what proves
//  (K4 ) golden:771  case 50 ART Selector Off -> On                       -> 179/1 
//        FAIL: [20] case 50's bAlreadyIn hand-shake RETRACTED C_Auto_Selector (golden :771)
//  (K5 ) golden:709  JAM1158 protection timer armed to 0 s                -> 180/0 
//  (K6 ) golden:733  JAM1158 gains a SKIP button                          -> 179/1 
//        FAIL: [21] K_RETRY only (golden :733)
//  (K7 ) golden:757  case 50 TrayMoveStatus guard inverted                -> 161/19
//        FAIL: [19] DoUnLoadNewAutoToStack converges
//        FAIL: [19] EXACT trajectory 50,100,150,200,300,350,400,401,402 -- case 45 is NEVER entered (its TrayMoveStatus
//        FAIL: [19] case 300 RETRACTED C_AutoZ_Select (wire cleared, golden :826)
//        ... and 16 more
//  (K9 ) golden:891  JAM1113 drops the SKIP button                        -> 179/1 
//        FAIL: [23] golden offers BOTH buttons (golden :891)
//  (L1 ) golden:846  case 350 MES1121 code swapped                        -> 178/2 
//        FAIL: [24] the code is sMES1121[Pos] (== MES1321 for Auto3)
//        FAIL: [27] the seven codes are exactly the seven the register names (no unlisted arm fired)
//  (L2 ) golden:772  ART bAlreadyIn latch removed                         -> 178/2 
//        FAIL: [20] the ART two-cylinder arm converges
//        FAIL: [20] EXACT trajectory matches the plain arm (the cursor cannot see the swap)
//  (N1 ) golden:923  case 100 iReceiveAutoTray=2 deleted                  -> 179/1 
//        FAIL: [25] iReceiveAutoTray[Pos] was raised to 2 mid-chain (golden :923) -- the busy flag the rest of the hand
//  (N2 ) golden:940  case 200 iReceiveAutoTray=0 deleted                  -> 179/1 
//        FAIL: [25] and cleared back to 0 at case 200 (golden :940)
//  (N3 ) golden:939  case 200 car ClearTray deleted                       -> 179/1 
//        FAIL: [25] case 200 cleared the Auto car (golden :939)
//  (N4 ) golden:927  case 100 unload hand-off skipped                     -> 177/3 
//        FAIL: [25] EXACT trajectory 100,200,1
//        FAIL: [25] and cleared back to 0 at case 200 (golden :940)
//        FAIL: [25] case 200 cleared the Auto car (golden :939)
//  (N5 ) golden:931  case 100 no-car branch retargeted                    -> 178/2 
//        FAIL: [26] the short-circuit returns to idle
//        FAIL: [26] EXACT trajectory 100,1 -- case 200 is NEVER entered when the car has no tray
//  (N6 ) golden:917  case 1 InitAutoTrayToFrontTask deleted               -> 174/6 
//        FAIL: [25] the receive chain returns to its idle cursor
//        FAIL: [25] EXACT trajectory 100,200,1
//        FAIL: [25] iReceiveAutoTray[Pos] was raised to 2 mid-chain (golden :923) -- the busy flag the rest of the hand
//        ... and 3 more
//  (A7 ) golden:61   short-circuit given a spurious lifter command        -> 179/1 
//        FAIL: [5] NEITHER lifter cylinder was commanded -- both still raised
//  (N7 ) golden:931  Init moved onto the no-car branch (kills [26] sentin -> 179/1 
//        FAIL: [26] the 9999 SENTINEL survived -- InitUnLoadNewAutoTrayTask (golden :926) belongs to the OTHER branch a
//
//  THE ONE GREEN -- (K5), AND IT IS EVIDENCE, NOT A HOLE
//  ------------------------------------------------------
//  (K5) shortens DoUnLoadNewAutoToStack's 5 s JAM1158 protection timer (golden
//  :709) to 0 s and the suite stays 180/0.  That is CORRECT, and it tells you
//  exactly what [21]'s first assertion pins: golden's case 1 ARMS the timer and
//  sets bUnLoadSafetyTimerStarted in one branch and only TESTS timer.Off() in the
//  other branch of the SAME if/else (golden :707-712), so tick 1 can never alarm
//  no matter what the duration is.  [21] pins that two-phase structure, not the
//  5 s value.  A mutation that collapsed the two phases would go red -- the
//  duration alone cannot.  Recorded rather than quietly dropped.
//
//  ASSERTIONS NOT INDIVIDUALLY MUTATION-PROVEN -- NAMED, NOT IMPLIED
//  -----------------------------------------------------------------
//  The 76 mutations above cover every assertion family and most individual
//  assertions.  These are the ones that no SINGLE mutation above reddens on its
//  own.  Each still constrains something real; none is claimed as proven:
//    * [2]'s twelve fixture-seam checks.  They observe TQPF_Timer / TMySensor /
//      TMyCylinder / TMySwitch, which are NOT this agent's files, so mutating
//      them was out of scope.  What makes them non-vacuous is that each asserts
//      BOTH directions of the same object in one breath (armed -> not elapsed,
//      then elapsed; unknown -> neither; off -> IsOff only; on -> IsOn only;
//      On -> bit set, Off -> bit clear), so no single broken behaviour satisfies
//      all of them.
//    * [3]'s pre-condition check and [18c]'s pre-condition check.  Both assert
//      that the FIXTURE reached the state the sub-test needs; a fixture bug
//      reddens them, an SM bug does not, and that is their job.
//    * [12]/[15]/[16]/[19]/[20]/[22]'s "no alarm on this path" checks.  Each is
//      reddened only through [27]'s global accounting (mutations B3/G6/I5), not
//      by a mutation local to its own sub-test.
//    * [16]'s "the guard does NOT clear the car".  (H9) makes [16] fail on
//      convergence and trajectory before it can reach case 600, so the car claim
//      itself never gets its own red.
//    * [19]'s "the NON-ART path never commands C_Auto_Selector directly".  It is
//      a negative claim over a path that has no selector command to delete; only
//      an INVENTED command would redden it, and (K3)/(K4) invert the ART arm at a
//      different Pos.
//    * [5]'s "short-circuit raises no alarm" and [13]'s "the DUMMY timeout is
//      silent" -- same shape as the "no alarm" family above.
// =============================================================================
#include "asendic_Auto_RT.h"
#include "asendic.h"
#include "Motor/mymotor.h"
#include "mycylin.h"
#include "mysensor.h"
#include "myswitch.h"
#include "myTimer.h"            // TQPF_Timer (the virtual-time seam below)
#include "cprod.h"
#include "cmydef.h"
#include "MachineType.h"
#include "Config.h"
#include "CosFunction.h"
#include "canary_support.h"     // LastSet, ShowErrorMessage seam, ShowMyMessage seam
// AI(W906-W7-L1-Wave3) 20260802: AutoCylinderUp/Middle/Lower are no longer the
// `{ return true; }` stubs in acatchtray_shims.cpp -- asendic.cpp now carries
// golden's real closed-loop bodies.  This header supplies the physical Auto stack
// that answers their position sensors.
#include "w3_cylinder_plant.h"
#include <cstdio>
#include <cstring>
#include <vector>
#include <set>
#include <string>

// ---------------------------------------------------------------------------
//  Minimal PASS / FAIL harness (same style as tests/test_w7_l1_loader.cpp)
// ---------------------------------------------------------------------------
static int g_pass = 0, g_fail = 0;

#define CHECK(cond, msg)                                                        \
    do {                                                                        \
        if (cond) { printf("  PASS: %s\n", msg); ++g_pass; }                    \
        else      { printf("  FAIL: %s  (line %d)\n", msg, __LINE__); ++g_fail; } \
    } while (0)

// ---------------------------------------------------------------------------
//  VIRTUAL-TIME SEAM -- the file-scope TQPF_Timer objects of asendic_Auto_RT.cpp.
//  Classification (progress / dual-role / pure watchdog) is in the banner.
// ---------------------------------------------------------------------------
extern TQPF_Timer AutoDelay[MAX_AUTO_TRAY];                     // golden asendic_Auto_RT.cpp:31
extern TQPF_Timer AutoRTDelay;                                  // golden asendic_Auto_RT.cpp:32
extern TQPF_Timer hAutoTrayToFront[MAX_AUTO_TRAY];              // golden asendic_Auto_RT.cpp:195
extern TQPF_Timer hAutoTrayToFrontForDummy[MAX_AUTO_TRAY];      // golden asendic_Auto_RT.cpp:196
extern TQPF_Timer hAutoTrayToRear[MAX_AUTO_TRAY];               // golden asendic_Auto_RT.cpp:316
extern TQPF_Timer hAutoTrayToRearForDummy[MAX_AUTO_TRAY];       // golden asendic_Auto_RT.cpp:317
extern TQPF_Timer DoUnLoadNewAutoToStackDelay[MAX_AUTO_TRAY];   // golden asendic_Auto_RT.cpp:670
extern TQPF_Timer hUnLoadSafetyCheckTimer[MAX_AUTO_TRAY];       // golden asendic_Auto_RT.cpp:671

// ---------------------------------------------------------------------------
//  SIM WIRE SEAM (cylinders) + SIM SENSOR SEAM + SIM SWITCH SEAM
// ---------------------------------------------------------------------------
static void simWire(int cyl, int bit)           // real, readable output bit
{
    Cylinder[cyl].Enable     = true;
    Cylinder[cyl].OutType    = TYPE_A;          // On/Push -> IOBitOn, Off/Pop -> IOBitOff
    Cylinder[cyl].OutISABase = eMotionNet;      // -> MyLaneIO Sim backend (real bit store)
    Cylinder[cyl].OutRing    = 1;
    Cylinder[cyl].OutIP      = 1;
    Cylinder[cyl].OutPort    = 2;
    Cylinder[cyl].OutBit     = bit;
}
static void simSensorOff(int i)     { Sen[i].Enable=true;  Sen[i].Type=TYPE_A; Sen[i].ISABase=eISABase; }
static void simSensorOn (int i)     { Sen[i].Enable=true;  Sen[i].Type=TYPE_B; Sen[i].ISABase=eISABase; }
static void simSensorUnknown(int i) { Sen[i].Enable=false; }
static void simSwitchWire(int sw, int bit)
{
    SW[sw].Enable  = true;
    SW[sw].Type    = TYPE_A;
    SW[sw].ISABase = eMotionNet;
    SW[sw].Ring    = 1;
    SW[sw].IP      = 1;
    SW[sw].Port    = 3;                         // a different port from the cylinders
    SW[sw].Bit     = bit;
}

// ---------------------------------------------------------------------------
//  Ordered trajectory recorder.  Records the cursor after every tick and
//  collapses consecutive duplicates, so the result is the ORDERED sequence of
//  distinct states the SM passed through -- not a set.
// ---------------------------------------------------------------------------
typedef std::vector<int> Traj;

static void trajPush(Traj &t, int cursor)
{
    if (t.empty() || t.back() != cursor) t.push_back(cursor);
}
static bool trajEq(const Traj &t, const int *want, int nWant)
{
    if ((int)t.size() != nWant) return false;
    for (int i = 0; i < nWant; ++i) if (t[i] != want[i]) return false;
    return true;
}
static void trajPrint(const char *what, const Traj &t, const int *want, int nWant)
{
    printf("    (%s got:", what);
    for (size_t i = 0; i < t.size(); ++i) printf(" %d", t[i]);
    printf(" | want:");
    for (int i = 0; i < nWant; ++i) printf(" %d", want[i]);
    printf(")\n");
}

// ---------------------------------------------------------------------------
//  Fixture.  Everything the Auto_RT family reads that is not a per-sub-test
//  variable is pinned here, so no sub-test can inherit another one's state.
//  The six cylinders of the ACTIVE Auto get a real wire address; the seven
//  sensors are set to "unknown" (an unconfigured offline IO table), which golden
//  reads as `IsOff()==false` -- the real machine's "sensor says nothing yet".
//
//  ALARM ACCUMULATOR: the fixture resets the ShowErrorMessage seam at the start
//  of every sub-test (so a sub-test that WANTS to reason about alarms starts
//  clean).  That would make [27]'s register self-check measure only the LAST
//  sub-test, which is exactly the quietly-vacuous shape this file exists to
//  avoid -- so the counts are BANKED here before each reset and [27] asserts
//  bank + current.
// ---------------------------------------------------------------------------
static int g_alarmsBanked = 0, g_dialogsBanked = 0;
static std::set<std::string> g_alarmCodesSeen;

static void bankAlarms()
{
    if (W906_ShowErrorMessage_Count > 0)
        g_alarmCodesSeen.insert(std::string(W906_ShowErrorMessage_LastCode.c_str()));
    g_alarmsBanked  += W906_ShowErrorMessage_Count;
    g_dialogsBanked += W906_ShowMyMessage_Count;
}

static int P = 1;                                // the Auto index under test

static void setupAutoRTFixture(int pos)
{
    bankAlarms();
    P = pos;

    LastSet.iRealDummy = DUMMY;                  // dummy feed unless a sub-test says otherwise
    CUSTOMER_CODE      = 0;                      // NOT CC_ASE_KaohSiung
    USE_AUTO_RETEST    = eartUninstall;
    for (int i = 0; i < MAX_AUTO_TRAY; ++i) UNLOADER_ART[i] = eartUninstall;
    bNoAutoZSelect        = false;
    USE_LdUldCassetteMode = 0;
    bHandlerPause         = false;
    bEmptyPause           = false;
    IniConfig.bP37bAutoCylinderUP = false;
    IniConfig.bLastLoaderNoInSide = false;
    CosFunction.bUseSCKART        = false;
    // Operator-tunable machine settings, not SM logic -- pinned to 0 s so the
    // settle waits are one tick each.
    Ld_UldDelayTime.LD_TrayArrivalDely = 0;
    Ld_UldDelayTime.ULD_TrayBackDelay  = 0;

    simWire(C_Auto_Up[P],         1);
    simWire(C_Auto_Selector[P],   2);
    simWire(C_AutoZ_Select[P],    3);
    simWire(C_AutoSide_Fixer[P],  4);
    simWire(C_AutoEdgePush[P],    5);
    simWire(C_AutoUpPress[P],     6);

    simSensorUnknown(SnAutoTrayHasTray[P]);
    simSensorUnknown(SnAutoPreDete[P]);
    simSensorUnknown(SnAutoTrayCar[P]);
    simSensorUnknown(SnAutoCWPreDetect[P]);
    simSensorUnknown(SnAutoTrayDetect[P]);
    simSensorUnknown(SnAutoFixCyPush[P]);
    simSensorUnknown(SnAutoUpSafe[P]);

    MOT[iMMAuto[P]].ClearTray("test_w7_l1_auto_rt");
    MOT[iMMAuto_Car[P]].ClearTray("test_w7_l1_auto_rt");
    MOT[iAutoZMot[P]].ClearTray("test_w7_l1_auto_rt");
    MOT[MMTrayZ].fHasTray = false;

    SW[SwACTrayY].Enable = false;                // Status() -> false (the "belt stopped" read)

    if (P < 3) iReceiveAutoTray[P] = 0;          // golden dims it [3] -- see GOLDEN DEFECT D

    // AI(W906-W7-L1-Wave3) 20260802: simWire() above gives each cylinder a real
    // OUTPUT bit but no position sensors, which was enough while
    // AutoCylinderUp/Middle/Lower were `{ return true; }` stubs and is not enough
    // now that asendic.cpp carries golden's real closed loops.  Register the
    // lift/selector pair with the plant, IN CALL-SITE ORDER.
    //
    // ORDER, and why the NON-ART order is the right one here: this fixture pins
    // USE_AUTO_RETEST=eartUninstall and UNLOADER_ART[]=eartUninstall above, so
    // bARTUnloaderUseOneCylin and bARTUnloaderUseTwoCylin are BOTH false and every
    // three-way call site in asendic_Auto_RT.cpp takes its plain `else` arm --
    // golden :818, :858, :874, :791 -- all of which pass
    // (C_Auto_Up[Pos], C_Auto_Selector[Pos]).  The REVERSED spellings in this file
    // (:783, :816, :856, :872) are the bARTUnloaderUseOneCylin arms and are not
    // reached under this fixture; they must not be "normalised" either.
    w3::Forget();
    w3::WireAutoStack(C_Auto_Up[P], C_Auto_Selector[P], 21 + P);
    w3::ResetCursors();
    Ld_UldDelayTime.LD_BeforeDownDelay    = 0;
    Ld_UldDelayTime.LD_StackMiddLockDelay = 0;
    Ld_UldDelayTime.LD_LiftDownDelay      = 0;

    W906_ShowErrorMessage_Reset();
    W906_ShowMyMessage_Reset();
}

// Gated timer advances -- each fires only where that timer's PROGRESS role is
// live (see banner seam 1).  Deliberately NOT collapsed into one helper.
static void advCarSettle()   { if (iLoadNewAutoTrayToCarTask[P] == 400) AutoDelay[P].SetMSAndOn(0); }
static void advTrayZ()       { AutoRTDelay.SetMSAndOn(0); }
static void advFrontSettle() { if (iAutoTrayToFrontTask[P] == 103 ||
                                   iAutoTrayToFrontTask[P] == 104) hAutoTrayToFrontForDummy[P].SetMSAndOn(0); }
static void advRearSettle()  { if (iAutoTrayToRearTask[P] == 460 ||
                                   iAutoTrayToRearTask[P] == 510) hAutoTrayToRear[P].SetMSAndOn(0); }
static void advUnload()      { DoUnLoadNewAutoToStackDelay[P].SetMSAndOn(0); }

static int bit(int cyl) { return Cylinder[cyl].GetOutBit() ? 1 : 0; }

// =============================================================================
int main()
{
    printf("=== test_w7_l1_auto_rt : asendic_Auto_RT (W7-L1 Wave 2) ===\n");

    // -----------------------------------------------------------------------
    printf("\n[1] Init* cursor resets + the InitTrayZAutoTrayToWaitTask golden defect\n");
    // -----------------------------------------------------------------------
    {
        setupAutoRTFixture(1);
        iLoadNewAutoTrayToCarTask[1] = 4242;  InitLoadNewAutoTrayToCarTask(1);
        CHECK(iLoadNewAutoTrayToCarTask[1] == 1, "InitLoadNewAutoTrayToCarTask(1) -> cursor==1");
        iAutoTrayToFrontTask[1] = 4242;       InitAutoTrayToFrontTask(1);
        CHECK(iAutoTrayToFrontTask[1] == 1,    "InitAutoTrayToFrontTask(1) -> cursor==1");
        iAutoTrayToRearTask[1] = 4242;        InitAutoTrayToRearTask(1);
        CHECK(iAutoTrayToRearTask[1] == 1,     "InitAutoTrayToRearTask(1) -> cursor==1");
        iUnLoadNewAutoTrayTask[1] = 4242;     InitUnLoadNewAutoTrayTask(1);
        CHECK(iUnLoadNewAutoTrayTask[1] == 1,  "InitUnLoadNewAutoTrayTask(1) -> cursor==1");
        iAutoTrayReceiveTask[1] = 4242;       InitAutoTrayReceiveTask(1);
        CHECK(iAutoTrayReceiveTask[1] == 1,    "InitAutoTrayReceiveTask(1) -> cursor==1");

        // Per-Pos independence: Init(1) must not touch Auto3's cursor.
        iAutoTrayToRearTask[2] = 4242;        InitAutoTrayToRearTask(1);
        CHECK(iAutoTrayToRearTask[2] == 4242,  "InitAutoTrayToRearTask(1) leaves Pos 2 untouched");
        iAutoTrayToRearTask[2] = 1;

        // GOLDEN DEFECT A (golden :949-952): the Init named for the
        // TrayZAutoTrayToWait SM resets iTrayZLoadTrayToWaitTask -- the LOADER's
        // cursor (golden asendic.h:14 / asendic_Loader.cpp:51) -- and does NOT
        // touch the iTrayZAutoTrayToWaitTask[] that DoTrayZAutoTrayToWait binds
        // to at golden :957.  Translated verbatim; pinned here so a future
        // "tidy-up" that silently corrects it is caught.
        iTrayZLoadTrayToWaitTask = 777;
        for (int i = 0; i < MAX_AUTO_TRAY; ++i) iTrayZAutoTrayToWaitTask[i] = 888;
        InitTrayZAutoTrayToWaitTask();
        CHECK(iTrayZLoadTrayToWaitTask == 1,
              "GOLDEN DEFECT A: InitTrayZAutoTrayToWaitTask writes the LOADER cursor (==1)");
        bool untouched = true;
        for (int i = 0; i < MAX_AUTO_TRAY; ++i) if (iTrayZAutoTrayToWaitTask[i] != 888) untouched = false;
        CHECK(untouched,
              "GOLDEN DEFECT A: it does NOT reset any iTrayZAutoTrayToWaitTask[] entry");
        for (int i = 0; i < MAX_AUTO_TRAY; ++i) iTrayZAutoTrayToWaitTask[i] = 1;
    }

    // -----------------------------------------------------------------------
    printf("\n[2] fixture seams -- sensor tri-state, cylinder wire bit, switch wire, timer\n");
    // -----------------------------------------------------------------------
    {
        setupAutoRTFixture(1);
        const int s = SnAutoTrayDetect[P];
        simSensorUnknown(s);
        CHECK(Sen[s].IsOn() == false && Sen[s].IsOff() == false,
              "sensor UNKNOWN -> IsOn()==false AND IsOff()==false (they are NOT complements)");
        simSensorOff(s);
        CHECK(Sen[s].IsOff() == true && Sen[s].IsOn() == false, "sensor OFF -> IsOff only");
        simSensorOn(s);
        CHECK(Sen[s].IsOn() == true && Sen[s].IsOff() == false, "sensor ON  -> IsOn only");
        simSensorUnknown(s);

        const int c = C_AutoEdgePush[P];
        Cylinder[c].Off();
        CHECK(bit(c) == 0, "cylinder wire: Off() clears the sim output bit");
        Cylinder[c].On();
        CHECK(bit(c) == 1, "cylinder wire: On() sets the sim output bit");
        Cylinder[c].Pop();
        CHECK(bit(c) == 0, "cylinder wire: Pop() clears the sim output bit");
        Cylinder[c].Push();
        CHECK(bit(c) == 1, "cylinder wire: Push() sets the sim output bit");
        CHECK(Cylinder[c].Push() == true && Cylinder[c].Pop() == true,
              "cylinder: offline Push() and Pop() BOTH return true (so the bit, not the "
              "return value, is the oracle)");
        Cylinder[c].Off();

        simSwitchWire(SwACTrayY, 0);
        SW[SwACTrayY].On();
        CHECK(SW[SwACTrayY].Status() == true,  "switch wire: On() -> Status()==true");
        SW[SwACTrayY].Off();
        CHECK(SW[SwACTrayY].Status() == false, "switch wire: Off() -> Status()==false");
        SW[SwACTrayY].Enable = false;

        TQPF_Timer t;
        t.SetMSAndOn(60000);
        CHECK(t.Off() == false, "TQPF_Timer: armed 60 s is not elapsed");
        t.SetMSAndOn(0);
        CHECK(t.Off() == true,  "TQPF_Timer: re-armed to 0 ms is elapsed (the virtual-time seam)");
    }

    // -----------------------------------------------------------------------
    printf("\n[3] DoLoadNewEmptyTrayToCar_RT -- DUMMY main walk (golden :39-192)\n");
    // -----------------------------------------------------------------------
    {
        setupAutoRTFixture(1);
        InitLoadNewAutoTrayToCarTask(P);
        MOT[iAutoZMot[P]].SetTray(NULL_IC, "pre");   // pre-drive OPPOSITE: ClearTray must clear it
        CHECK(MOT[iAutoZMot[P]].fHasTray == true, "[3] pre-condition: AutoZ motor starts WITH a tray");

        Traj tr; int ret = 0; bool converged = false;
        for (int i = 0; i < 30; ++i)
        {
            w3::Tick();     // AI(W906-W7-L1-Wave3) 20260802: Auto-stack plant tick
            advCarSettle();
            ret = DoLoadNewEmptyTrayToCar_RT(P);
            trajPush(tr, iLoadNewAutoTrayToCarTask[P]);
            if (ret != 0) { converged = true; break; }
        }
        static const int want[] = {51, 100, 200, 300, 400, 410, 420, 1};
        trajPrint("[3]", tr, want, 8);
        CHECK(converged, "[3] DoLoadNewEmptyTrayToCar_RT returns non-zero (converges)");
        CHECK(trajEq(tr, want, 8), "[3] EXACT cursor trajectory 51,100,200,300,400,410,420,1");
        CHECK(ret == 1, "[3] returns exactly 1 (the 'new tray loaded' code, golden :176)");
        CHECK(MOT[iMMAuto_Car[P]].fHasTray == true,
              "[3] case 420 latched a tray onto the Auto car (SetTray, golden :173)");
        CHECK(MOT[iAutoZMot[P]].fHasTray == false,
              "[3] case 420 cleared the AutoZ motor (ClearTray, golden :174)");
        CHECK(W906_ShowErrorMessage_Count == 0, "[3] no alarm on the DUMMY main walk");
    }

    // -----------------------------------------------------------------------
    printf("\n[4] DoLoadNewEmptyTrayToCar_RT -- case-1 fast path (car already has a tray)\n");
    // -----------------------------------------------------------------------
    {
        setupAutoRTFixture(1);
        InitLoadNewAutoTrayToCarTask(P);
        MOT[iMMAuto_Car[P]].SetTray(NULL_IC, "pre");     // golden :47 short-circuit
        MOT[iAutoZMot[P]].SetTray(NULL_IC, "pre");

        Traj tr; int ret = 0; bool converged = false;
        // AI(W906-W7-L1-Wave3) 20260802: BOUND 20 -> 60.  Bound only -- the EXACT
        // trajectory asserted below is unchanged.  Cases 51/200 now pump golden's
        // real AutoCylinderUp/AutoCylinderMiddle instead of stubs that returned
        // true on their first call, so the same walk costs more ticks.
        for (int i = 0; i < 60; ++i)
        {
            w3::Tick();     // AI(W906-W7-L1-Wave3) 20260802: Auto-stack plant tick
            advCarSettle();
            ret = DoLoadNewEmptyTrayToCar_RT(P);
            trajPush(tr, iLoadNewAutoTrayToCarTask[P]);
            if (ret != 0) { converged = true; break; }
        }
        static const int want[] = {400, 410, 420, 1};
        trajPrint("[4]", tr, want, 4);
        CHECK(converged, "[4] fast path converges");
        CHECK(trajEq(tr, want, 4),
              "[4] EXACT trajectory 400,410,420,1 -- case 1 jumps STRAIGHT to 400, skipping 51/100/200/300");
        CHECK(ret == 1, "[4] fast path returns 1");
        CHECK(MOT[iAutoZMot[P]].fHasTray == false, "[4] AutoZ motor cleared at case 420");
    }

    // -----------------------------------------------------------------------
    printf("\n[5] DoLoadNewEmptyTrayToCar_RT -- case-1 sensor short-circuit (golden :57-64)\n");
    // -----------------------------------------------------------------------
    {
        setupAutoRTFixture(1);
        InitLoadNewAutoTrayToCarTask(P);
        simSensorOn(SnAutoPreDete[P]);                    // golden :57
        MOT[iAutoZMot[P]].SetTray(NULL_IC, "pre");
        Cylinder[C_Auto_Up[P]].On();                      // pre-drive OPPOSITE of "untouched"
        Cylinder[C_Auto_Selector[P]].On();

        int ret = DoLoadNewEmptyTrayToCar_RT(P);
        CHECK(ret == 1, "[5] short-circuit returns 1 in ONE tick");
        CHECK(iLoadNewAutoTrayToCarTask[P] == 1, "[5] cursor stays on 1 (golden :62)");
        CHECK(MOT[iMMAuto_Car[P]].fHasTray == true,  "[5] car got a tray (golden :60)");
        CHECK(MOT[iAutoZMot[P]].fHasTray == false,   "[5] AutoZ cleared (golden :61)");
        CHECK(bit(C_Auto_Up[P]) == 1 && bit(C_Auto_Selector[P]) == 1,
              "[5] NEITHER lifter cylinder was commanded -- both still raised");
        CHECK(W906_ShowErrorMessage_Count == 0, "[5] short-circuit raises no alarm");
    }

    // -----------------------------------------------------------------------
    printf("\n[6] DoLoadNewEmptyTrayToCar_RT -- case-1 ELSE arm, both limbs (golden :77-93)\n");
    // -----------------------------------------------------------------------
    {
        // 6a: the cylinder-down limb.  Sensors say the stack is empty AND the car
        // sensors read OFF, so golden drops both lifters and breaks (golden :83-88).
        setupAutoRTFixture(1);
        InitLoadNewAutoTrayToCarTask(P);
        LastSet.iRealDummy = HAS_TRAY;                    // NOT dummy
        simSensorOff(SnAutoTrayHasTray[P]);
        simSensorOff(SnAutoTrayCar[P]);
        simSensorOff(SnAutoPreDete[P]);
        Cylinder[C_Auto_Up[P]].On();                      // pre-drive OPPOSITE
        Cylinder[C_Auto_Selector[P]].On();
        // AI(W906-W7-L1-Wave3) 20260802: settle the plant on that pre-drive.
        // Golden :81 gates this limb on `Cylinder[C_Auto_Selector[Pos]].OffStatus()
        // ==false` -- "the selector is still up".  Before Wave 3 the fixture left
        // OffSenEnable false, so OffStatus() answered false for the WRONG reason
        // (no sensor at all) and the gate opened by accident.  The selector now has
        // a real OFF sensor, so the fixture has to actually hold it UP: one plant
        // tick on the pre-drive does that.  Deliberately the only tick here -- this
        // sub-test calls the SM exactly once.
        w3::Tick();

        int ret = DoLoadNewEmptyTrayToCar_RT(P);
        CHECK(ret == 0 && iLoadNewAutoTrayToCarTask[P] == 1, "[6a] else-arm keeps the cursor on 1");
        CHECK(bit(C_Auto_Up[P]) == 0,
              "[6a] golden :85 dropped C_Auto_Up (wire cleared)");
        CHECK(bit(C_Auto_Selector[P]) == 0,
              "[6a] golden :86 dropped C_Auto_Selector (wire cleared)");
        CHECK(W906_ShowErrorMessage_Count == 0,
              "[6a] the cylinder-down limb BREAKS before the alarm (golden :87)");

        // 6b: the MES1121 limb.  Same else-arm, but the car sensors read UNKNOWN
        // (IsOff()==false), so the inner guard at golden :83 fails and control
        // falls through to the alarm at golden :92.
        setupAutoRTFixture(1);
        InitLoadNewAutoTrayToCarTask(P);
        LastSet.iRealDummy = HAS_TRAY;
        simSensorOff(SnAutoTrayHasTray[P]);               // SnAutoTrayCar / SnAutoPreDete stay UNKNOWN

        ret = DoLoadNewEmptyTrayToCar_RT(P);
        CHECK(ret == 0 && iLoadNewAutoTrayToCarTask[P] == 1, "[6b] alarm limb keeps the cursor on 1");
        CHECK(W906_ShowErrorMessage_Count == 1, "[6b] exactly one alarm raised");
        CHECK(strcmp(W906_ShowErrorMessage_LastCode.c_str(), sMES1121[P].c_str()) == 0,
              "[6b] the alarm code is sMES1121[Pos] (== MES1221 for Auto2), proving Pos indexing");
        CHECK(W906_ShowErrorMessage_LastKCode == K_RETRY,
              "[6b] golden offers K_RETRY only -- no SKIP button (golden :92)");
    }

    // -----------------------------------------------------------------------
    printf("\n[7] DoLoadNewEmptyTrayToCar_RT -- case-200 bEmptyPause arm (golden :126-132)\n");
    // -----------------------------------------------------------------------
    {
        setupAutoRTFixture(1);
        InitLoadNewAutoTrayToCarTask(P);
        bEmptyPause = true;

        Traj tr; int ret = 0; bool converged = false;
        // AI(W906-W7-L1-Wave3) 20260802: BOUND 20 -> 60.  Bound only -- the EXACT
        // trajectory asserted below is unchanged.  Cases 51/200 now pump golden's
        // real AutoCylinderUp/AutoCylinderMiddle instead of stubs that returned
        // true on their first call, so the same walk costs more ticks.
        for (int i = 0; i < 60; ++i)
        {
            w3::Tick();     // AI(W906-W7-L1-Wave3) 20260802: Auto-stack plant tick
            advCarSettle();
            ret = DoLoadNewEmptyTrayToCar_RT(P);
            trajPush(tr, iLoadNewAutoTrayToCarTask[P]);
            if (ret != 0) { converged = true; break; }
        }
        static const int want[] = {51, 100, 200, 300, 400, 410, 420, 1};
        trajPrint("[7]", tr, want, 8);
        CHECK(converged, "[7] the split-pause path still converges");
        CHECK(trajEq(tr, want, 8),
              "[7] EXACT trajectory unchanged -- the pause is a one-tick DWELL on 200, not a detour");
        CHECK(bEmptyPause == false, "[7] golden :128 cleared bEmptyPause (one-shot)");
        CHECK(W906_ShowMyMessage_Count == 1, "[7] exactly one operator dialog");
        CHECK(strcmp(W906_ShowMyMessage_LastS1.c_str(), "Auto split wait") == 0,
              "[7] the dialog is golden :129's \"Auto split wait\"");
    }

    // -----------------------------------------------------------------------
    printf("\n[8] DoLoadNewEmptyTrayToCar_RT -- CC_ASE_KaohSiung case-50 path (golden :66-70, :95-107)\n");
    // -----------------------------------------------------------------------
    {
        setupAutoRTFixture(1);
        InitLoadNewAutoTrayToCarTask(P);
        CUSTOMER_CODE = CC_ASE_KaohSiung;
        iTrayZAutoTrayToWaitTask[P] = 1;
        iTrayZLoadTrayToWaitTask    = 777;                // GOLDEN DEFECT A observation point
        MOT[iAutoZMot[P]].SetTray(NULL_IC, "pre");        // pre-drive OPPOSITE

        Traj outer, inner; int ret = 0; bool converged = false;
        for (int i = 0; i < 30; ++i)
        {
            w3::Tick();     // AI(W906-W7-L1-Wave3) 20260802: Auto-stack plant tick
            advTrayZ();
            advCarSettle();
            ret = DoLoadNewEmptyTrayToCar_RT(P);
            trajPush(outer, iLoadNewAutoTrayToCarTask[P]);
            trajPush(inner, iTrayZAutoTrayToWaitTask[P]);
            if (ret != 0) { converged = true; break; }
        }
        static const int wantOuter[] = {50, 1};
        static const int wantInner[] = {1, 2, 100, 200, 201, 1000, 1100, 1200, 1};
        trajPrint("[8] outer", outer, wantOuter, 2);
        trajPrint("[8] inner", inner, wantInner, 9);
        CHECK(converged, "[8] the ASE path converges");
        CHECK(trajEq(outer, wantOuter, 2),
              "[8] EXACT outer trajectory 50,1 -- the ASE branch NEVER visits 51/100/200/300");
        CHECK(trajEq(inner, wantInner, 9),
              "[8] EXACT nested DoTrayZAutoTrayToWait trajectory 1,2,100,200,201,1000,1100,1200,1");
        CHECK(ret == 2,
              "[8] returns exactly 2 -- the 'last tray, do not feed it' code (golden :105), NOT 1");
        CHECK(MOT[iAutoZMot[P]].fHasTray == false, "[8] golden :103 cleared the AutoZ motor");
        CHECK(iTrayZLoadTrayToWaitTask == 1,
              "[8] GOLDEN DEFECT A is LIVE on this path: the ASE branch clobbered the LOADER cursor");
    }

    // -----------------------------------------------------------------------
    printf("\n[9] DoTrayZAutoTrayToWait -- DUMMY walk, Pos 2 (golden :954-1046)\n");
    // -----------------------------------------------------------------------
    {
        setupAutoRTFixture(2);                       // Pos 2 so GOLDEN DEFECT B (hard-coded
                                                     // Part 1 at golden :983) is LIVE -- see
                                                     // NOT-COVERED item A: it is not observable.
        iTrayZAutoTrayToWaitTask[P] = 1;
        Cylinder[C_AutoZ_Select[P]].On();            // pre-drive OPPOSITE of the case-1100 Pop

        Traj tr; int ret = 0; bool converged = false;
        int bitAt1100 = -1;
        for (int i = 0; i < 30; ++i)
        {
            w3::Tick();     // AI(W906-W7-L1-Wave3) 20260802: Auto-stack plant tick
            advTrayZ();
            bool wasOn1100 = (iTrayZAutoTrayToWaitTask[P] == 1100);
            ret = DoTrayZAutoTrayToWait(P);
            if (wasOn1100) bitAt1100 = bit(C_AutoZ_Select[P]);
            trajPush(tr, iTrayZAutoTrayToWaitTask[P]);
            if (ret != 0) { converged = true; break; }
        }
        static const int want[] = {2, 100, 200, 201, 1000, 1100, 1200, 1};
        trajPrint("[9]", tr, want, 8);
        CHECK(converged, "[9] DUMMY walk converges");
        CHECK(trajEq(tr, want, 8),
              "[9] EXACT trajectory 2,100,200,201,1000,1100,1200,1 -- the 300/400 limb is NOT taken");
        CHECK(ret == 2, "[9] returns exactly 2 (golden :1041)");
        CHECK(bitAt1100 == 0, "[9] case 1100 RETRACTED C_AutoZ_Select (wire cleared, golden :1032)");
        CHECK(MOT[MMTrayZ].fHasTray == false,
              "[9] the DUMMY limb never latches MMTrayZ (golden :1020 is on the OTHER limb)");
    }

    // -----------------------------------------------------------------------
    printf("\n[10] DoTrayZAutoTrayToWait -- non-DUMMY last-tray limb + the SwACTrayY gate\n");
    // -----------------------------------------------------------------------
    {
        setupAutoRTFixture(2);
        iTrayZAutoTrayToWaitTask[P] = 1;
        LastSet.iRealDummy = HAS_TRAY;
        IniConfig.bLastLoaderNoInSide = true;             // golden :990
        simSensorOn(SnAutoTrayHasTray[P]);                // golden :992 -> the 300 limb
        simSwitchWire(SwACTrayY, 0);
        SW[SwACTrayY].On();                               // belt RUNNING -> case 400 must park
        Cylinder[C_AutoZ_Select[P]].Off();                // pre-drive OPPOSITE of the case-100 Push

        Traj tr; int ret = 0;
        int bitAt100 = -1, bitAt300 = -1;
        // AI(W906-W7-L1-Wave3) 20260802: BOUND 12 -> 40.  Bound only -- the EXACT
        // trajectory asserted below is unchanged, and "we EXPECT no convergence"
        // is still what is asserted (the SM must PARK on 400 while SwACTrayY is
        // on).  The walk simply costs more ticks now that its lifter legs run
        // golden's real AutoCylinder* instead of stubs that returned true at once.
        for (int i = 0; i < 40; ++i)                      // bounded: we EXPECT no convergence
        {
            w3::Tick();     // AI(W906-W7-L1-Wave3) 20260802: Auto-stack plant tick
            advTrayZ();
            bool was100 = (iTrayZAutoTrayToWaitTask[P] == 100);
            bool was300 = (iTrayZAutoTrayToWaitTask[P] == 300);
            ret = DoTrayZAutoTrayToWait(P);
            if (was100) bitAt100 = bit(C_AutoZ_Select[P]);
            if (was300) bitAt300 = bit(C_AutoZ_Select[P]);
            trajPush(tr, iTrayZAutoTrayToWaitTask[P]);
            if (ret != 0) break;
        }
        static const int wantPark[] = {2, 100, 200, 201, 300, 400};
        trajPrint("[10] parked", tr, wantPark, 6);
        CHECK(trajEq(tr, wantPark, 6),
              "[10] with SwACTrayY ON the SM walks 2,100,200,201,300,400 and PARKS on 400");
        CHECK(ret == 0, "[10] parked on 400 -> still returns 0");
        CHECK(MOT[MMTrayZ].fHasTray == false, "[10] parked -> MMTrayZ NOT latched");
        CHECK(bitAt100 == 1, "[10] case 100 EXTENDED C_AutoZ_Select (wire set, golden :976)");
        CHECK(bitAt300 == 0, "[10] case 300 RETRACTED C_AutoZ_Select (wire cleared, golden :1012)");

        // Release the gate: the belt reads stopped, so case 400 completes.
        SW[SwACTrayY].Off();
        advTrayZ();
        ret = DoTrayZAutoTrayToWait(P);
        CHECK(ret == 1, "[10] with SwACTrayY OFF case 400 returns exactly 1 (golden :1022)");
        CHECK(iTrayZAutoTrayToWaitTask[P] == 1, "[10] and resets its own cursor to 1");
        CHECK(MOT[MMTrayZ].fHasTray == true,   "[10] and latches MMTrayZ.fHasTray (golden :1020)");
        SW[SwACTrayY].Enable = false;
    }

    // -----------------------------------------------------------------------
    printf("\n[11] DoTrayZAutoTrayToWait -- 'please cover one tray' operator dialog (golden :998-1006)\n");
    // -----------------------------------------------------------------------
    {
        setupAutoRTFixture(2);
        iTrayZAutoTrayToWaitTask[P] = 1;
        LastSet.iRealDummy = HAS_TRAY;
        IniConfig.bLastLoaderNoInSide = true;
        simSensorOff(SnAutoTrayHasTray[P]);               // stack empty -> NOT the 300 limb
        CosFunction.bUseSCKART = false;                   // golden :998
        iAutoTrayCount[P] = 3;                            // golden :999

        Traj tr;
        for (int i = 0; i < 12; ++i)
        {
            w3::Tick();     // AI(W906-W7-L1-Wave3) 20260802: Auto-stack plant tick
            advTrayZ();
            DoTrayZAutoTrayToWait(P);
            trajPush(tr, iTrayZAutoTrayToWaitTask[P]);
            if (W906_ShowMyMessage_Count > 0) break;
        }
        static const int want[] = {2, 100, 200, 201, 1};
        trajPrint("[11]", tr, want, 5);
        CHECK(trajEq(tr, want, 5),
              "[11] EXACT trajectory 2,100,200,201,1 -- case 201 RESTARTS the SM, it does not reach 1000");
        CHECK(W906_ShowMyMessage_Count == 1, "[11] exactly one operator dialog");
        CHECK(strcmp(W906_ShowMyMessage_LastS1.c_str(), "Auto 3 Tray, Please Cover One Tray") == 0,
              "[11] the dialog text carries Pos+1 == 3 (golden :1001), proving Pos indexing");
        CHECK(W906_ShowErrorMessage_Count == 0, "[11] it is a DIALOG, not an alarm");
        iAutoTrayCount[P] = 0;
    }

    // -----------------------------------------------------------------------
    printf("\n[12] DoAutoTrayToFront -- default sensor walk (golden :202-313)\n");
    // -----------------------------------------------------------------------
    {
        setupAutoRTFixture(1);
        InitAutoTrayToFrontTask(P);
        MOT[iMMAuto[P]].SetTray(NULL_IC, "pre");          // pre-drive OPPOSITE of the case-105 clears
        MOT[iMMAuto_Car[P]].SetTray(NULL_IC, "pre");
        Cylinder[C_AutoEdgePush[P]].On();                 // pre-drive OPPOSITE of case 100's Off
        Cylinder[C_AutoUpPress[P]].On();
        Cylinder[C_AutoSide_Fixer[P]].On();

        Traj tr; bool done = false, converged = false;
        int edgeAt101 = -1, fixAt102 = -1, fixAt103 = -1, edgeAt104 = -1;
        int edgeAfter100 = -1, pressAfter100 = -1;
        for (int i = 0; i < 30; ++i)
        {
            w3::Tick();     // AI(W906-W7-L1-Wave3) 20260802: Auto-stack plant tick
            advFrontSettle();
            int before = iAutoTrayToFrontTask[P];
            done = DoAutoTrayToFront(P);
            if (before == 1)   { edgeAfter100 = bit(C_AutoEdgePush[P]); pressAfter100 = bit(C_AutoUpPress[P]); }
            if (before == 101) edgeAt101 = bit(C_AutoEdgePush[P]);
            if (before == 102) fixAt102  = bit(C_AutoSide_Fixer[P]);
            if (before == 103) fixAt103  = bit(C_AutoSide_Fixer[P]);
            if (before == 104) edgeAt104 = bit(C_AutoEdgePush[P]);
            trajPush(tr, iAutoTrayToFrontTask[P]);
            if (done) { converged = true; break; }
        }
        static const int want[] = {101, 102, 103, 104, 105, 1};
        trajPrint("[12]", tr, want, 6);
        CHECK(converged, "[12] DoAutoTrayToFront converges");
        CHECK(trajEq(tr, want, 6),
              "[12] EXACT trajectory 101,102,103,104,105,1 -- the FIRST entry is 101, which PINS the "
              "golden case 1 -> case 100 FALL-THROUGH (golden :214-215): with a break it would be 100");
        CHECK(edgeAfter100 == 0 && pressAfter100 == 0,
              "[12] case 100 dropped C_AutoEdgePush AND C_AutoUpPress (golden :223-224)");
        CHECK(edgeAt101 == 1, "[12] case 101 EXTENDED C_AutoEdgePush (golden :273)");
        CHECK(fixAt102  == 1, "[12] case 102 EXTENDED C_AutoSide_Fixer (golden :284)");
        CHECK(fixAt103  == 0, "[12] case 103 RETRACTED C_AutoSide_Fixer (golden :294)");
        CHECK(edgeAt104 == 0, "[12] case 104 RETRACTED C_AutoEdgePush (golden :301)");
        CHECK(MOT[iMMAuto[P]].fHasTray == false && MOT[iMMAuto_Car[P]].fHasTray == false,
              "[12] case 105 cleared BOTH the Auto motor and the Auto car (golden :306-307)");
        CHECK(W906_ShowErrorMessage_Count == 0, "[12] no alarm on the sensor walk");
    }

    // -----------------------------------------------------------------------
    printf("\n[13] DoAutoTrayToFront -- DUMMY-timeout arm (golden :237-248)\n");
    // -----------------------------------------------------------------------
    {
        setupAutoRTFixture(1);
        InitAutoTrayToFrontTask(P);
        simSensorOff(SnAutoTrayCar[P]);                   // golden :228 fails -> no 101 limb
        MOT[iMMAuto[P]].SetTray(NULL_IC, "pre");          // pre-drive OPPOSITE (must be CLEARED)
        MOT[iMMAuto_Car[P]].ClearTray("pre");             // pre-drive OPPOSITE (must be SET)

        Traj tr; bool done = false, converged = false;
        for (int i = 0; i < 12; ++i)
        {
            w3::Tick();     // AI(W906-W7-L1-Wave3) 20260802: Auto-stack plant tick
            if (iAutoTrayToFrontTask[P] == 100) hAutoTrayToFrontForDummy[P].SetMSAndOn(0);
            done = DoAutoTrayToFront(P);
            trajPush(tr, iAutoTrayToFrontTask[P]);
            if (done) { converged = true; break; }
        }
        static const int want[] = {100, 1};
        trajPrint("[13]", tr, want, 2);
        CHECK(converged, "[13] the DUMMY-timeout arm converges");
        CHECK(trajEq(tr, want, 2),
              "[13] EXACT trajectory 100,1 -- it NEVER enters the 101..105 hook sequence");
        CHECK(MOT[iMMAuto_Car[P]].fHasTray == true,
              "[13] golden :242 SET a tray on the Auto car (this is what later lets DoAutoTrayReceive "
              "reach its case 200 -- see [25])");
        CHECK(MOT[iMMAuto[P]].fHasTray == false, "[13] golden :243 cleared the Auto motor");
        CHECK(W906_ShowErrorMessage_Count == 0, "[13] the DUMMY timeout is silent (no JAM1112)");
    }

    // -----------------------------------------------------------------------
    printf("\n[14] DoAutoTrayToFront -- JAM1112 watchdog: K_SKIP vs K_RETRY (golden :250-267)\n");
    // -----------------------------------------------------------------------
    {
        // 14a -- K_SKIP.  The operator presses SKIP; golden abandons the tray.
        setupAutoRTFixture(1);
        InitAutoTrayToFrontTask(P);
        LastSet.iRealDummy = HAS_TRAY;                    // so the DUMMY arm above cannot fire
        simSensorOff(SnAutoTrayCar[P]);
        MOT[iMMAuto[P]].SetTray(NULL_IC, "pre");          // pre-drive OPPOSITE: both must be CLEARED
        MOT[iMMAuto_Car[P]].SetTray(NULL_IC, "pre");

        bool done = false, converged = false;
        for (int i = 0; i < 8; ++i)
        {
            w3::Tick();     // AI(W906-W7-L1-Wave3) 20260802: Auto-stack plant tick
            if (iAutoTrayToFrontTask[P] == 100)
            {
                hAutoTrayToFront[P].SetMSAndOn(0);        // fire the 20 s JAM1112 watchdog ON PURPOSE
                W906_ShowErrorMessage_SimReturn = K_SKIP; // (seam resets to K_RETRY after each read)
            }
            done = DoAutoTrayToFront(P);
            if (done) { converged = true; break; }
        }
        CHECK(converged, "[14a] the K_SKIP arm returns true");
        CHECK(W906_ShowErrorMessage_Count == 1, "[14a] exactly one JAM1112 raised");
        CHECK(strcmp(W906_ShowErrorMessage_LastCode.c_str(), sJAM1112[P].c_str()) == 0,
              "[14a] the code is sJAM1112[Pos] (== JAM1212 for Auto2)");
        CHECK(W906_ShowErrorMessage_LastKCode == (K_SKIP | K_RETRY),
              "[14a] golden offers BOTH buttons (golden :252)");
        CHECK(MOT[iMMAuto[P]].fHasTray == false && MOT[iMMAuto_Car[P]].fHasTray == false,
              "[14a] K_SKIP cleared BOTH tray flags (golden :262-263)");
        CHECK(iAutoTrayToFrontTask[P] == 1, "[14a] K_SKIP resets the cursor to 1");

        // 14b -- K_RETRY.  Same alarm, opposite answer: golden must NOT abandon.
        setupAutoRTFixture(1);
        InitAutoTrayToFrontTask(P);
        LastSet.iRealDummy = HAS_TRAY;
        simSensorOff(SnAutoTrayCar[P]);
        MOT[iMMAuto[P]].SetTray(NULL_IC, "pre");
        MOT[iMMAuto_Car[P]].SetTray(NULL_IC, "pre");

        bool anyTrue = false;
        for (int i = 0; i < 2; ++i)                       // exactly one alarm, then stop
        {
            w3::Tick();     // AI(W906-W7-L1-Wave3) 20260802: Auto-stack plant tick
            if (iAutoTrayToFrontTask[P] == 100) hAutoTrayToFront[P].SetMSAndOn(0);
            if (DoAutoTrayToFront(P)) anyTrue = true;
            if (W906_ShowErrorMessage_Count > 0) break;
        }
        CHECK(anyTrue == false, "[14b] K_RETRY does NOT return true");
        CHECK(W906_ShowErrorMessage_Count == 1, "[14b] exactly one JAM1112 raised");
        CHECK(iAutoTrayToFrontTask[P] == 1, "[14b] K_RETRY also resets the cursor to 1 (golden :256)");
        CHECK(MOT[iMMAuto[P]].fHasTray == true && MOT[iMMAuto_Car[P]].fHasTray == true,
              "[14b] K_RETRY left BOTH tray flags ALONE -- this is the whole SKIP/RETRY difference");
    }

    // -----------------------------------------------------------------------
    printf("\n[15] DoAutoTrayToRear -- default walk (golden :324-465)\n");
    // -----------------------------------------------------------------------
    {
        setupAutoRTFixture(1);
        InitAutoTrayToRearTask(P);
        MOT[iMMAuto[P]].ClearTray("pre");                 // pre-drive OPPOSITE of case 600's SetTray
        MOT[iMMAuto_Car[P]].SetTray(NULL_IC, "pre");      // pre-drive OPPOSITE of case 600's ClearTray
        Cylinder[C_AutoSide_Fixer[P]].On();               // pre-drive OPPOSITE of case 100's Pop
        Cylinder[C_AutoEdgePush[P]].On();                 // pre-drive OPPOSITE of case 1's Off
        Cylinder[C_AutoUpPress[P]].On();

        Traj tr; bool done = false, converged = false;
        int fixAt100 = -1, fixAt500 = -1, fixAt600 = -1, edgeAfter1 = -1, pressAfter1 = -1;
        for (int i = 0; i < 30; ++i)
        {
            w3::Tick();     // AI(W906-W7-L1-Wave3) 20260802: Auto-stack plant tick
            advRearSettle();
            int before = iAutoTrayToRearTask[P];
            done = DoAutoTrayToRear(P);
            if (before == 1)   { edgeAfter1 = bit(C_AutoEdgePush[P]); pressAfter1 = bit(C_AutoUpPress[P]);
                                 fixAt100 = bit(C_AutoSide_Fixer[P]); }
            if (before == 500) fixAt500 = bit(C_AutoSide_Fixer[P]);
            if (before == 600) fixAt600 = bit(C_AutoSide_Fixer[P]);
            trajPush(tr, iAutoTrayToRearTask[P]);
            if (done) { converged = true; break; }
        }
        static const int want[] = {200, 450, 460, 500, 510, 600, 1};
        trajPrint("[15]", tr, want, 7);
        CHECK(converged, "[15] DoAutoTrayToRear converges");
        CHECK(trajEq(tr, want, 7),
              "[15] EXACT trajectory 200,450,460,500,510,600,1 -- the FIRST entry is 200, which PINS "
              "the golden case 1 -> case 100 FALL-THROUGH (golden :340-341); cases 300 and 400 are "
              "NEVER visited (GOLDEN DEFECT C)");
        CHECK(edgeAfter1 == 0 && pressAfter1 == 0,
              "[15] case 1 dropped C_AutoEdgePush AND C_AutoUpPress (golden :338-339)");
        CHECK(fixAt100 == 0, "[15] case 100 RETRACTED C_AutoSide_Fixer (golden :342)");
        CHECK(fixAt500 == 1, "[15] case 500 EXTENDED C_AutoSide_Fixer (golden :440)");
        CHECK(fixAt600 == 0, "[15] case 600 RETRACTED C_AutoSide_Fixer (golden :454)");
        CHECK(MOT[iMMAuto[P]].fHasTray == true,
              "[15] case 600 LATCHED the tray onto the Auto motor (golden :456) -- this is the line "
              "asendic_Auto.cpp's DoAuto123TrayToRear twin does NOT have");
        CHECK(MOT[iMMAuto_Car[P]].fHasTray == false,
              "[15] case 600 cleared the Auto car (golden :457)");
        CHECK(W906_ShowErrorMessage_Count == 0, "[15] no alarm on the default walk");
    }

    // -----------------------------------------------------------------------
    printf("\n[16] DoAutoTrayToRear -- double-tray early exit (golden :354-359)\n");
    // -----------------------------------------------------------------------
    {
        setupAutoRTFixture(1);
        InitAutoTrayToRearTask(P);
        MOT[iMMAuto[P]].SetTray(NULL_IC, "pre");
        MOT[iMMAuto_Car[P]].SetTray(NULL_IC, "pre");      // BOTH -> the Q4 double-tray guard

        Traj tr; bool done = false, converged = false;
        for (int i = 0; i < 10; ++i)
        {
            w3::Tick();     // AI(W906-W7-L1-Wave3) 20260802: Auto-stack plant tick
            advRearSettle();
            done = DoAutoTrayToRear(P);
            trajPush(tr, iAutoTrayToRearTask[P]);
            if (done) { converged = true; break; }
        }
        static const int want[] = {200, 1};
        trajPrint("[16]", tr, want, 2);
        CHECK(converged, "[16] the double-tray guard returns true");
        CHECK(trajEq(tr, want, 2),
              "[16] EXACT trajectory 200,1 -- it bails at case 200 and never reaches 450/500/600");
        CHECK(MOT[iMMAuto_Car[P]].fHasTray == true,
              "[16] the guard does NOT clear the car (case 600's ClearTray was not reached)");
        CHECK(W906_ShowErrorMessage_Count == 0, "[16] the double-tray guard is silent");
    }

    // -----------------------------------------------------------------------
    printf("\n[17] DoAutoTrayToRear -- JAM1101 watchdog: K_SKIP vs K_RETRY (golden :377-392)\n");
    // -----------------------------------------------------------------------
    {
        // 17a -- K_SKIP
        setupAutoRTFixture(1);
        InitAutoTrayToRearTask(P);
        LastSet.iRealDummy = HAS_TRAY;
        simSensorOff(SnAutoTrayDetect[P]);                // golden :362 fails -> no 450 limb
        MOT[iMMAuto_Car[P]].SetTray(NULL_IC, "pre");      // pre-drive OPPOSITE: must be CLEARED

        bool done = false, converged = false;
        for (int i = 0; i < 8; ++i)
        {
            w3::Tick();     // AI(W906-W7-L1-Wave3) 20260802: Auto-stack plant tick
            advRearSettle();
            if (iAutoTrayToRearTask[P] == 200)
            {
                hAutoTrayToRear[P].SetMSAndOn(0);         // fire the 20 s JAM1101 watchdog ON PURPOSE
                W906_ShowErrorMessage_SimReturn = K_SKIP;
            }
            done = DoAutoTrayToRear(P);
            if (done) { converged = true; break; }
        }
        CHECK(converged, "[17a] the K_SKIP arm returns true");
        CHECK(W906_ShowErrorMessage_Count == 1, "[17a] exactly one JAM1101 raised");
        CHECK(strcmp(W906_ShowErrorMessage_LastCode.c_str(), sJAM1101[P].c_str()) == 0,
              "[17a] the code is sJAM1101[Pos] (== JAM1201 for Auto2)");
        CHECK(W906_ShowErrorMessage_LastKCode == (K_SKIP | K_RETRY),
              "[17a] golden offers BOTH buttons (golden :379)");
        CHECK(MOT[iMMAuto_Car[P]].fHasTray == false, "[17a] K_SKIP cleared the Auto car (golden :388)");

        // 17b -- K_RETRY
        setupAutoRTFixture(1);
        InitAutoTrayToRearTask(P);
        LastSet.iRealDummy = HAS_TRAY;
        simSensorOff(SnAutoTrayDetect[P]);
        MOT[iMMAuto_Car[P]].SetTray(NULL_IC, "pre");

        bool anyTrue = false;
        for (int i = 0; i < 3; ++i)
        {
            w3::Tick();     // AI(W906-W7-L1-Wave3) 20260802: Auto-stack plant tick
            advRearSettle();
            if (iAutoTrayToRearTask[P] == 200) hAutoTrayToRear[P].SetMSAndOn(0);
            if (DoAutoTrayToRear(P)) anyTrue = true;
            if (W906_ShowErrorMessage_Count > 0) break;
        }
        CHECK(anyTrue == false, "[17b] K_RETRY does NOT return true");
        CHECK(W906_ShowErrorMessage_Count == 1, "[17b] exactly one JAM1101 raised");
        CHECK(MOT[iMMAuto_Car[P]].fHasTray == true,
              "[17b] K_RETRY left the car tray flag ALONE -- the SKIP/RETRY difference");
        CHECK(iAutoTrayToRearTask[P] == 1, "[17b] K_RETRY resets the cursor to 1 (golden :383)");
    }

    // -----------------------------------------------------------------------
    printf("\n[18] DoAutoTrayToRear -- the 300/400 island (GOLDEN DEFECT C), entered the way\n");
    printf("     golden main.cpp:9870's QueueTaskList entry lets the operator enter it\n");
    // -----------------------------------------------------------------------
    {
        // 18a -- DUMMY: bFlag true (no off-sensor configured -> OffSensor()==true) -> 450
        setupAutoRTFixture(1);
        iAutoTrayToRearTask[P] = 300;                     // <- the operator Task List write
        Cylinder[C_AutoSide_Fixer[P]].Off();              // pre-drive OPPOSITE of case 300's Push

        Traj tr; bool done = false, converged = false;
        int fixAt300 = -1;
        for (int i = 0; i < 20; ++i)
        {
            w3::Tick();     // AI(W906-W7-L1-Wave3) 20260802: Auto-stack plant tick
            advRearSettle();
            int before = iAutoTrayToRearTask[P];
            done = DoAutoTrayToRear(P);
            if (before == 300) fixAt300 = bit(C_AutoSide_Fixer[P]);
            trajPush(tr, iAutoTrayToRearTask[P]);
            if (done) { converged = true; break; }
        }
        static const int wantA[] = {400, 450, 460, 500, 510, 600, 1};
        trajPrint("[18a]", tr, wantA, 7);
        CHECK(converged, "[18a] the forced 300 entry converges");
        CHECK(trajEq(tr, wantA, 7),
              "[18a] EXACT trajectory 400,450,460,500,510,600,1 -- case 400's DUMMY limb rejoins at 450");
        CHECK(fixAt300 == 1, "[18a] case 300 EXTENDED C_AutoSide_Fixer (golden :395)");
        CHECK(W906_ShowErrorMessage_Count == 0, "[18a] the DUMMY limb of case 400 is silent");

        // 18b -- non-DUMMY, bFlag true -> JAM1103 and BACK to 300 (the retry loop)
        setupAutoRTFixture(1);
        LastSet.iRealDummy = HAS_TRAY;
        iAutoTrayToRearTask[P] = 300;
        Cylinder[C_AutoSide_Fixer[P]].Off();

        Traj tr2;
        int fixAt400 = -1;
        for (int i = 0; i < 3; ++i)
        {
            w3::Tick();     // AI(W906-W7-L1-Wave3) 20260802: Auto-stack plant tick
            int before = iAutoTrayToRearTask[P];
            DoAutoTrayToRear(P);
            if (before == 400) fixAt400 = bit(C_AutoSide_Fixer[P]);
            trajPush(tr2, iAutoTrayToRearTask[P]);
            if (W906_ShowErrorMessage_Count > 0 && iAutoTrayToRearTask[P] == 300) break;
        }
        static const int wantB[] = {400, 300};
        trajPrint("[18b]", tr2, wantB, 2);
        CHECK(trajEq(tr2, wantB, 2), "[18b] EXACT trajectory 400,300 -- golden loops back, it does not advance");
        CHECK(W906_ShowErrorMessage_Count == 1, "[18b] exactly one JAM1103 raised");
        CHECK(strcmp(W906_ShowErrorMessage_LastCode.c_str(), sJAM1103[P].c_str()) == 0,
              "[18b] the code is sJAM1103[Pos] (== JAM1203 for Auto2)");
        CHECK(W906_ShowErrorMessage_LastKCode == K_RETRY,
              "[18b] golden offers K_RETRY ONLY here -- there is no way out of this loop but a fixed "
              "cylinder (golden :416)");
        CHECK(fixAt400 == 0, "[18b] case 400's alarm limb RETRACTED C_AutoSide_Fixer (golden :415)");

        // 18c -- bFlag FALSE (a real off-sensor that reads not-off) -> straight to 500
        setupAutoRTFixture(1);
        LastSet.iRealDummy = HAS_TRAY;
        iAutoTrayToRearTask[P] = 400;
        simSensorOff(SnAutoFixCyPush[P]);                                    // golden :400 IsOn()==false
        Cylinder[C_AutoSide_Fixer[P]].OffSenEnable  = true;                  // golden :401 OffSensor()==false
        Cylinder[C_AutoSide_Fixer[P]].OffSenISABase = eISABase;
        Cylinder[C_AutoSide_Fixer[P]].OffSenType    = TYPE_A;
        CHECK(Cylinder[C_AutoSide_Fixer[P]].OffSensor() == false,
              "[18c] fixture pre-condition: the fixer off-sensor now reads FALSE");

        DoAutoTrayToRear(P);
        CHECK(iAutoTrayToRearTask[P] == 500,
              "[18c] with bFlag==false case 400 goes to 500, SKIPPING the 450 settle (golden :425)");
        CHECK(W906_ShowErrorMessage_Count == 0, "[18c] the bFlag==false limb is silent");
        Cylinder[C_AutoSide_Fixer[P]].OffSenEnable = false;
    }

    // -----------------------------------------------------------------------
    printf("\n[19] DoUnLoadNewAutoToStack -- DUMMY walk (golden :677-902)\n");
    // -----------------------------------------------------------------------
    {
        setupAutoRTFixture(1);
        InitUnLoadNewAutoTrayTask(P);
        MOT[iMMAuto[P]].SetTray(NULL_IC, "pre");          // pre-drive OPPOSITE of case 350's ClearTray
        Cylinder[C_AutoZ_Select[P]].On();                 // pre-drive OPPOSITE of case 300's Pop
        // AI(W906-W7-L1-Wave3) 20260802: pre-drive OFF, exactly as [20] does, so the
        // two sub-tests are mirror images of the same predicate: [20] pre-drives
        // OFF and requires case 1 to have EXTENDED the selector (golden :742, the
        // bARTUnloaderUseTwoCylin limb); [19] pre-drives OFF and requires case 1 to
        // have left it alone.  The old pre-drive was ON with "must stay ON: the
        // non-ART path never touches the selector directly", which stopped being a
        // usable observable once golden's real AutoCylinder* bodies started driving
        // parameter 3 later in the same walk.
        Cylinder[C_Auto_Selector[P]].Off();

        Traj tr; bool done = false, converged = false;
        int zAt300 = -1, selAfterCase1 = -1;
        for (int i = 0; i < 40; ++i)
        {
            w3::Tick();     // AI(W906-W7-L1-Wave3) 20260802: Auto-stack plant tick
            advUnload();
            int before = iUnLoadNewAutoTrayTask[P];
            done = DoUnLoadNewAutoToStack(P);
            if (before == 1)   selAfterCase1 = bit(C_Auto_Selector[P]);
            if (before == 300) zAt300 = bit(C_AutoZ_Select[P]);
            trajPush(tr, iUnLoadNewAutoTrayTask[P]);
            if (done) { converged = true; break; }
        }
        static const int want[] = {50, 100, 150, 200, 300, 350, 400, 401, 402};
        trajPrint("[19]", tr, want, 9);
        CHECK(converged, "[19] DoUnLoadNewAutoToStack converges");
        CHECK(trajEq(tr, want, 9),
              "[19] EXACT trajectory 50,100,150,200,300,350,400,401,402 -- case 45 is NEVER entered "
              "(its TrayMoveStatus guard is DUMMY-gated, golden :756-757)");
        CHECK(zAt300 == 0, "[19] case 300 RETRACTED C_AutoZ_Select (wire cleared, golden :826)");
        CHECK(MOT[iMMAuto[P]].fHasTray == false, "[19] case 350 cleared the Auto motor (golden :838)");
        // AI(W906-W7-L1-Wave3) 20260802: ASSERTION CHANGED -- the old form
        // (`bit(C_Auto_Selector[P]) == 1`, "never commanded, still raised") was only
        // ever true because AutoCylinderUp/Middle/Lower were `{ return true; }`
        // stubs that touched nothing.  Golden's real bodies DO drive parameter 3
        // (AutoCylinderUp case 1 `if(Cylinder[CylinderNameMid].Enable)
        // Cylinder[CylinderNameMid].On();`, AutoCylinderMiddle case 1 `.Off()`,
        // AutoCylinderLower case 50 `.Off()`), so the end-of-walk wire level can no
        // longer distinguish "the SM commanded it" from "the lifter did".
        // The claim this assertion actually exists to make is the CONTRAST with
        // [20]: on the ART path case 1 extends the selector DIRECTLY (golden :742)
        // and on the non-ART path it does not.  That is now asserted in the same
        // shape [20] uses -- sampled right after case 1, before any AutoCylinder*
        // call has happened -- so the two sub-tests pin the two arms of the same
        // predicate against each other.  Same count, sharper claim.
        CHECK(selAfterCase1 == 0,
              "[19] case 1's NON-ART limb did NOT command C_Auto_Selector (the mirror of [20]'s golden :742)");
        CHECK(W906_ShowErrorMessage_Count == 0, "[19] no alarm on the DUMMY walk");
    }

    // -----------------------------------------------------------------------
    printf("\n[20] DoUnLoadNewAutoToStack -- ART TWO-CYLINDER arm (golden :740-745, :765-780)\n");
    // -----------------------------------------------------------------------
    {
        // Pos 4 (Auto5) is used ONLY here, because golden's bAlreadyIn[] is a
        // FUNCTION-LEVEL STATIC (golden :683) that this sub-test leaves latched
        // true; isolating it on its own Pos keeps every other sub-test clean.
        setupAutoRTFixture(4);
        InitUnLoadNewAutoTrayTask(P);
        USE_AUTO_RETEST  = eartInstall;                   // golden asendic.cpp:141
        UNLOADER_ART[P]  = eartInstall;
        bNoAutoZSelect   = false;                         // -> TwoCylin true, OneCylin false
        // AI(W906-W7-L1-Wave3) 20260802: re-register the pair under the ART sensor
        // law.  The fixture registers the NON-ART law (it pins eartUninstall); this
        // sub-test flips the predicate on, and golden's AutoCylinder* ART arms read
        // the OPPOSITE sensor pattern from the non-ART arms -- AutoCylinderMiddle
        // case 100 ART wants CylinderNameMid.OnStatus()==true where the non-ART arm
        // wants it false.  Without this the SM parks on cursor 50 repeating
        // "Lifter Middle error" (MEASURED).  Call-site order is unchanged:
        // golden :791 / :814 / :854 / :870 all pass (C_Auto_Up, C_Auto_Selector).
        w3::Forget();
        w3::WireAutoStackART(C_Auto_Up[P], C_Auto_Selector[P], 21 + P);
        MOT[iMMAuto[P]].SetTray(NULL_IC, "pre");
        Cylinder[C_Auto_Up[P]].Off();
        Cylinder[C_Auto_Selector[P]].Off();               // pre-drive OPPOSITE of case 1's On

        Traj tr; bool done = false, converged = false;
        int selAfterCase1 = -1, selAfterFirst50 = -1;
        int seen50 = 0;
        for (int i = 0; i < 40; ++i)
        {
            w3::Tick();     // AI(W906-W7-L1-Wave3) 20260802: Auto-stack plant tick
            advUnload();
            int before = iUnLoadNewAutoTrayTask[P];
            done = DoUnLoadNewAutoToStack(P);
            if (before == 1)  selAfterCase1 = bit(C_Auto_Selector[P]);
            if (before == 50) { ++seen50; if (seen50 == 1) selAfterFirst50 = bit(C_Auto_Selector[P]); }
            trajPush(tr, iUnLoadNewAutoTrayTask[P]);
            if (done) { converged = true; break; }
        }
        static const int want[] = {50, 100, 150, 200, 300, 350, 400, 401, 402};
        trajPrint("[20]", tr, want, 9);
        CHECK(converged, "[20] the ART two-cylinder arm converges");
        CHECK(trajEq(tr, want, 9), "[20] EXACT trajectory matches the plain arm (the cursor cannot see the swap)");
        CHECK(selAfterCase1 == 1,
              "[20] case 1's bARTUnloaderUseTwoCylin limb EXTENDED C_Auto_Selector (golden :742) -- "
              "this is what proves the ART arm was taken, because the cursor cannot");
        CHECK(selAfterFirst50 == 0,
              "[20] case 50's bAlreadyIn hand-shake RETRACTED C_Auto_Selector (golden :771)");
        CHECK(seen50 >= 2,
              "[20] case 50 ran at least twice -- the ART arm inserts a bAlreadyIn dwell the plain arm has not");
        CHECK(W906_ShowErrorMessage_Count == 0, "[20] no alarm on the ART walk");
        UNLOADER_ART[P] = eartUninstall;
        USE_AUTO_RETEST = eartUninstall;
        w3::Forget();       // AI(W906-W7-L1-Wave3) 20260802: drop the ART pair
    }

    // -----------------------------------------------------------------------
    printf("\n[21] DoUnLoadNewAutoToStack -- JAM1158 sensor-safety watchdog (golden :705-735)\n");
    // -----------------------------------------------------------------------
    {
        setupAutoRTFixture(3);                            // Pos 3 (Auto4): its own bUnLoadSafetyTimerStarted
        InitUnLoadNewAutoTrayTask(P);
        LastSet.iRealDummy = HAS_TRAY;                    // so the DUMMY bypass at golden :700 is off
        simSensorOff(SnAutoPreDete[P]);                   // NOT all three on
        simSensorOn(SnAutoTrayCar[P]);
        simSensorOff(SnAutoUpSafe[P]);

        DoUnLoadNewAutoToStack(P);                        // tick 1: arms the 5 s timer, no alarm yet
        CHECK(W906_ShowErrorMessage_Count == 0,
              "[21] tick 1 only ARMS the 5 s protection timer -- it does not alarm (golden :707-711)");
        CHECK(iUnLoadNewAutoTrayTask[P] == 1, "[21] tick 1 leaves the cursor on 1");

        hUnLoadSafetyCheckTimer[P].SetMSAndOn(0);         // fire the watchdog ON PURPOSE
        DoUnLoadNewAutoToStack(P);
        CHECK(W906_ShowErrorMessage_Count == 1, "[21] tick 2 raises exactly one alarm");
        CHECK(strcmp(W906_ShowErrorMessage_LastCode.c_str(), sJAM1158[P].c_str()) == 0,
              "[21] the code is sJAM1158[Pos] (== JAM2558 for Auto4)");
        CHECK(W906_ShowErrorMessage_LastKCode == K_RETRY, "[21] K_RETRY only (golden :733)");
        CHECK(iUnLoadNewAutoTrayTask[P] == 1,
              "[21] the alarm does NOT advance the cursor -- the SM is held at 1 until the sensors agree");

        // Satisfy the sensors and the SM releases.
        simSensorOn(SnAutoPreDete[P]);
        simSensorOn(SnAutoUpSafe[P]);
        DoUnLoadNewAutoToStack(P);
        CHECK(iUnLoadNewAutoTrayTask[P] == 50,
              "[21] with all three sensors ON the SM advances to 50 (golden :703)");
        CHECK(W906_ShowErrorMessage_Count == 1, "[21] and raises no further alarm");
    }

    // -----------------------------------------------------------------------
    printf("\n[22] DoUnLoadNewAutoToStack -- the WAVE-3 TrayMoveStatus park (NOT-COVERED item B)\n");
    // -----------------------------------------------------------------------
    {
        setupAutoRTFixture(5);                            // Pos 5 (Auto6): isolated statics
        InitUnLoadNewAutoTrayTask(P);
        LastSet.iRealDummy = HAS_TRAY;
        simSensorOn(SnAutoPreDete[P]);
        simSensorOn(SnAutoTrayCar[P]);
        simSensorOn(SnAutoUpSafe[P]);
        CHECK(TrayMoveStatus(P + 3, "test") == 0,
              "[22] fixture pre-condition: asendic.cpp's TrayMoveStatus is a hardwired 0 (no seam before Wave 3)");

        Traj tr; bool anyTrue = false;
        for (int i = 0; i < 10; ++i)
        {
            w3::Tick();     // AI(W906-W7-L1-Wave3) 20260802: Auto-stack plant tick
            advUnload();
            if (DoUnLoadNewAutoToStack(P)) anyTrue = true;
            trajPush(tr, iUnLoadNewAutoTrayTask[P]);
        }
        static const int want[] = {50, 45, 50, 45, 50, 45, 50, 45, 50, 45};
        trajPrint("[22]", tr, want, 10);
        CHECK(trajEq(tr, want, 10),
              "[22] EXACT alternating trajectory 50,45,50,45,... -- golden ping-pongs forever because "
              "`TrayMoveStatus(...)==0 && iRealDummy!=DUMMY` is permanently true offline (golden :756)");
        CHECK(anyTrue == false, "[22] and it never completes");
        CHECK(W906_ShowErrorMessage_Count == 0, "[22] the park is SILENT -- no watchdog guards it");
    }

    // -----------------------------------------------------------------------
    printf("\n[23] DoUnLoadNewAutoToStack -- case 402 JAM1113 K_SKIP (golden :889-894)\n");
    // -----------------------------------------------------------------------
    {
        setupAutoRTFixture(0);                            // Pos 0 (Auto1)
        InitUnLoadNewAutoTrayTask(P);
        MOT[iMMAuto[P]].SetTray(NULL_IC, "pre");

        bool done = false, skipped = false;
        for (int i = 0; i < 40; ++i)
        {
            w3::Tick();     // AI(W906-W7-L1-Wave3) 20260802: Auto-stack plant tick
            if (iUnLoadNewAutoTrayTask[P] == 402)
            {
                simSensorOn(SnAutoPreDete[P]);            // golden :889 -> the tray never left
                simSensorOn(SnAutoTrayCar[P]);
                W906_ShowErrorMessage_SimReturn = K_SKIP;
            }
            else advUnload();
            done = DoUnLoadNewAutoToStack(P);
            if (done) break;
            if (W906_ShowErrorMessage_Count > 0 && iUnLoadNewAutoTrayTask[P] == 1) { skipped = true; break; }
        }
        CHECK(skipped, "[23] the K_SKIP arm rewound the cursor 402 -> 1 (golden :893)");
        CHECK(done == false, "[23] and it did NOT return true -- SKIP restarts the unload, it does not finish it");
        CHECK(W906_ShowErrorMessage_Count == 1, "[23] exactly one JAM1113 raised");
        CHECK(strcmp(W906_ShowErrorMessage_LastCode.c_str(), sJAM1113[P].c_str()) == 0,
              "[23] the code is sJAM1113[Pos] (== JAM1113 for Auto1)");
        CHECK(W906_ShowErrorMessage_LastKCode == (K_RETRY | K_SKIP),
              "[23] golden offers BOTH buttons (golden :891)");
    }

    // -----------------------------------------------------------------------
    printf("\n[24] DoUnLoadNewAutoToStack -- case 350 MES1121 park (golden :841-848), reached\n");
    printf("     through the operator-cursor seam because NOT-COVERED item B blocks its normal path\n");
    // -----------------------------------------------------------------------
    {
        setupAutoRTFixture(2);                            // Pos 2 (Auto3)
        LastSet.iRealDummy = HAS_TRAY;
        iUnLoadNewAutoTrayTask[P] = 350;                  // <- the operator Task List write
        MOT[iMMAuto[P]].SetTray(NULL_IC, "pre");          // must NOT be cleared on this limb
        simSensorOn(SnAutoPreDete[P]);                    // golden :832's all-IsOff test fails

        DoUnLoadNewAutoToStack(P);
        CHECK(iUnLoadNewAutoTrayTask[P] == 350, "[24] the alarm limb PARKS on 350 (no Task write, golden :846)");
        CHECK(MOT[iMMAuto[P]].fHasTray == true,
              "[24] and does NOT clear the Auto motor -- that only happens on the success limb (golden :838)");
        CHECK(W906_ShowErrorMessage_Count == 1, "[24] exactly one alarm raised");
        CHECK(strcmp(W906_ShowErrorMessage_LastCode.c_str(), sMES1121[P].c_str()) == 0,
              "[24] the code is sMES1121[Pos] (== MES1321 for Auto3)");
        CHECK(W906_ShowErrorMessage_LastKCode == K_RETRY, "[24] K_RETRY only (golden :846)");
    }

    // -----------------------------------------------------------------------
    printf("\n[25] DoAutoTrayReceive -- full chain (golden :911-945)\n");
    // -----------------------------------------------------------------------
    {
        setupAutoRTFixture(1);
        InitAutoTrayReceiveTask(P);
        // The chain can ONLY reach case 200 through DoAutoTrayToFront's
        // DUMMY-TIMEOUT arm, because that is the single arm that SETS the car's
        // tray flag (golden :242).  Through the sensor arm, case 105 CLEARS it
        // (golden :307), so golden's `if(MOT[car].fHasTray)` at :924 is false and
        // the chain always short-circuits back to 1.  Measured, not assumed.
        simSensorOff(SnAutoTrayCar[P]);
        MOT[iMMAuto[P]].SetTray(NULL_IC, "pre");
        MOT[iMMAuto_Car[P]].ClearTray("pre");
        // SENTINEL: only golden :917's InitAutoTrayToFrontTask can rescue this
        // cursor.  Without it DoAutoTrayToFront(9999) matches no case and the
        // chain can never leave 100.  Added because the first mutation run proved
        // that without it, DELETING golden :917 left the whole suite GREEN.
        iAutoTrayToFrontTask[P] = 9999;

        Traj tr; bool sawReceive2 = false; bool converged = false;
        for (int i = 0; i < 60; ++i)
        {
            w3::Tick();     // AI(W906-W7-L1-Wave3) 20260802: Auto-stack plant tick
            if (iAutoTrayToFrontTask[P] == 100) hAutoTrayToFrontForDummy[P].SetMSAndOn(0);
            advUnload();
            DoAutoTrayReceive(P);
            if (iReceiveAutoTray[P] == 2) sawReceive2 = true;
            trajPush(tr, iAutoTrayReceiveTask[P]);
            if (i > 2 && iAutoTrayReceiveTask[P] == 1) { converged = true; break; }
        }
        static const int want[] = {100, 200, 1};
        trajPrint("[25]", tr, want, 3);
        CHECK(converged, "[25] the receive chain returns to its idle cursor");
        CHECK(trajEq(tr, want, 3), "[25] EXACT trajectory 100,200,1");
        CHECK(sawReceive2,
              "[25] iReceiveAutoTray[Pos] was raised to 2 mid-chain (golden :923) -- the busy flag "
              "the rest of the handler polls");
        CHECK(iReceiveAutoTray[P] == 0, "[25] and cleared back to 0 at case 200 (golden :940)");
        CHECK(MOT[iMMAuto_Car[P]].fHasTray == false, "[25] case 200 cleared the Auto car (golden :939)");
        CHECK(W906_ShowErrorMessage_Count == 0, "[25] the full chain raises no alarm");
    }

    // -----------------------------------------------------------------------
    printf("\n[26] DoAutoTrayReceive -- the no-car-tray short-circuit (golden :929-933)\n");
    // -----------------------------------------------------------------------
    {
        setupAutoRTFixture(1);
        InitAutoTrayReceiveTask(P);
        MOT[iMMAuto_Car[P]].ClearTray("pre");             // the sensor arm of Front will leave it clear
        iUnLoadNewAutoTrayTask[P] = 9999;                 // SENTINEL: only golden :926 would overwrite it
        iAutoTrayToFrontTask[P]   = 9999;                 // SENTINEL: only golden :917 can rescue this one

        Traj tr; bool converged = false;
        for (int i = 0; i < 30; ++i)
        {
            w3::Tick();     // AI(W906-W7-L1-Wave3) 20260802: Auto-stack plant tick
            advFrontSettle();
            DoAutoTrayReceive(P);
            trajPush(tr, iAutoTrayReceiveTask[P]);
            if (i > 2 && iAutoTrayReceiveTask[P] == 1) { converged = true; break; }
        }
        static const int want[] = {100, 1};
        trajPrint("[26]", tr, want, 2);
        CHECK(converged, "[26] the short-circuit returns to idle");
        CHECK(trajEq(tr, want, 2),
              "[26] EXACT trajectory 100,1 -- case 200 is NEVER entered when the car has no tray");
        CHECK(iReceiveAutoTray[P] == 0,
              "[26] iReceiveAutoTray is set to 2 and back to 0 inside ONE tick (golden :923 then :932), "
              "so only the final 0 is observable -- stated rather than claimed as coverage");
        CHECK(iUnLoadNewAutoTrayTask[P] == 9999,
              "[26] the 9999 SENTINEL survived -- InitUnLoadNewAutoTrayTask (golden :926) belongs to "
              "the OTHER branch and was NOT called here");
        CHECK(iAutoTrayToFrontTask[P] != 9999,
              "[26] the OTHER 9999 sentinel did NOT survive -- case 1 really did call "
              "InitAutoTrayToFrontTask (golden :917)");
        iUnLoadNewAutoTrayTask[P] = 1;
    }

    // -----------------------------------------------------------------------
    printf("\n[27] NOT-COVERED register self-check\n");
    // -----------------------------------------------------------------------
    {
        bankAlarms();                                     // fold in sub-test [26]
        printf("    (alarms raised across the whole suite: %d ; operator dialogs: %d)\n",
               g_alarmsBanked, g_dialogsBanked);
        printf("    (distinct alarm codes seen:");
        for (std::set<std::string>::const_iterator it = g_alarmCodesSeen.begin();
             it != g_alarmCodesSeen.end(); ++it) printf(" %s", it->c_str());
        printf(")\n");

        // The register above accounts for SEVEN alarm sub-tests ([6], [14], [17],
        // [18], [21], [23], [24]) contributing NINE raises, each of exactly one
        // alarm: [6b] MES1221, [14a] JAM1212, [14b] JAM1212, [17a] JAM1201,
        // [17b] JAM1201, [18b] JAM1203, [21] JAM2558, [23] JAM1113, [24] MES1321
        // -- nine raises across SEVEN distinct codes.  Anything else means an arm
        // fired that this file does not account for, or one it claims stopped
        // firing.
        CHECK(g_alarmsBanked == 9,
              "[27] EXACTLY 9 ShowErrorMessage calls across the suite -- the number the register accounts for");
        CHECK(g_alarmCodesSeen.size() == 7,
              "[27] EXACTLY 7 distinct alarm codes -- MES1221, MES1321, JAM1113, JAM1201, JAM1203, "
              "JAM1212, JAM2558");
        CHECK(g_alarmCodesSeen.count("MES1221") == 1 && g_alarmCodesSeen.count("MES1321") == 1 &&
              g_alarmCodesSeen.count("JAM1113") == 1 && g_alarmCodesSeen.count("JAM1201") == 1 &&
              g_alarmCodesSeen.count("JAM1203") == 1 && g_alarmCodesSeen.count("JAM1212") == 1 &&
              g_alarmCodesSeen.count("JAM2558") == 1,
              "[27] the seven codes are exactly the seven the register names (no unlisted arm fired)");
        CHECK(g_dialogsBanked == 2,
              "[27] EXACTLY 2 ShowMyMessage dialogs -- [7]'s \"Auto split wait\" and [11]'s cover-tray prompt");
    }

    printf("\n=== RESULT: %d passed, %d failed ===\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
