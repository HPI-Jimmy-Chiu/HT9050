// =============================================================================
//  test_w7_l1_auto.cpp  --  W7-L1 Wave 2: asendic_Auto.cpp
//
//  Translation wave: W906-W7-L1 Wave 2 (Auto track)
//  Author: AI(W906-W7-L1-Auto) 20260802
//
//  WHAT IS UNDER TEST
//  ------------------
//  The Auto-tray (automatic sort-output tray) stack-feed state machine,
//  asendic_Auto.cpp -- 9 golden top-level functions, of which
//  DoAutoReceiveBinTray is a single 2,189-line switch with 56 cases -- driven
//  over the Sim HAL (Cylinder[] / MOT[] / Sen[] / SW[]) with no hardware and no
//  window.
//
//  PROOF OBJECTIVE
//  ---------------
//  Every assertion below constrains a SPECIFIC observable and has been proven
//  capable of failing -- see FAILABILITY PROOF at the bottom of this banner.
//  Three rules, each adopted because this project has already shipped the
//  corresponding defect:
//    1. TRAJECTORIES ARE EXACT AND ORDERED, never set membership.  Each run
//       records the cursor before the first tick and after every tick,
//       collapses consecutive duplicates, and compares against an ordered
//       expected list.
//    2. NO ASSERTION PUTS THE LOOP'S OWN TERMINATION CONDITION INTO ITS
//       DISJUNCTION.  There is no `done || steps == BOUND` anywhere here.
//    3. REACHED IS NOT COMMANDED.  Offline, Push() and Pop() BOTH return true,
//       so a cursor trajectory cannot distinguish an extend from a retract.
//       Every actuator claim reads STATE (Cylinder[].bCylinderOn) and every
//       actuator is PRE-DRIVEN TO THE OPPOSITE of the expected value first, so
//       "call deleted" fails the same assertion as "call inverted".
//
//  THREE FIXTURE SEAMS
//  -------------------
//  1. VIRTUAL TIME.  TQPF_Timer is a QueryPerformanceCounter wall clock with no
//     injection point.  The SM's file-scope timers are non-static, so the extern
//     declarations below reach them without touching asendic_Auto.h.
//     advanceAuto() re-arms only PROGRESS timers to 0 ms, and the DUAL-ROLE ones
//     ONLY at the cursor where their progress role is live:
//       DetectAutoTime[]      -- 20 s JAM1112 WATCHDOG at cursor 1021
//                                (golden :966-:970).  Re-armed as a 2 s / 100 ms
//                                / ULD_TrayArrivalWaitDelay PROGRESS timer for
//                                cursors 1020 (golden :898/:1024), 1025
//                                (golden :949), 1210 (golden :185/:1207) and
//                                1220 (golden :1268/:1270, the ART bAlreadyIn
//                                dwell).  Advanced ONLY on those four.
//       DetectAutoTimePause[] -- PROGRESS at cursor 970 (golden :821).  At
//                                cursors 1021 and 1025 it is golden's
//                                "were we paused?" detector: elapsing it there
//                                makes golden re-arm and `break` forever
//                                (golden :915-:924 / :1004-:1013), so it is
//                                NEVER advanced there.  MEASURED: an early
//                                version of this fixture advanced it
//                                unconditionally and the SM live-locked on 1021.
//       AutoReceiveBinTrayDelay[] -- PROGRESS at cursor 1550 (golden :2010).  At
//                                cursor 1710 it gates the JAM1103 cylinder alarm
//                                (golden :2151), so it is NOT advanced there.
//     PURE PROGRESS, advanced every tick: ULDStackDelay, htSwACAutoOffDelay,
//     RecevieLoopDelay, htUnLoaderTimeRelief, DoTrayGoOutNeedDelay,
//     htUnLoaderTimeDelay.
//     NEVER ADVANCED AT ALL: ColorSensorDelayTime[] (the 60 s anti-hang guard on
//     the WAR1151 colour-sensor arm, golden :2131) and AutoReceiveBinTrayTimer[]
//     (a LatchCycleTime stopwatch, never read through Off()).
//     ULDStackDelay is advanced unconditionally.  It has a WATCHDOG role at
//     cursor 1352 (the JAM1114 separator-cylinder timeout, golden :1497) but
//     ONLY when one of SnAutoZSelect1..4 is Enable'd; no sub-test here enables
//     any of them, so that role never arises -- stated rather than assumed.
//  2. SIM SENSOR LEVELS.  Sen[] is TRI-STATE and this file uses all three:
//       simSensorUnknown : Enable=false          -> IsOn()==false AND IsOff()==false
//       simSensorOff     : Enable + TYPE_A(1)    -> IsOff()==true , IsOn()==false
//       simSensorOn      : Enable + TYPE_B(0)    -> IsOn()==true , IsOff()==false
//     "unknown" is the load-bearing default here: golden tests
//     `Sen[X].IsOff()==false` in a dozen places on the Auto path and "unknown"
//     satisfies that without claiming the sensor said ON.  Sub-test [2] proves
//     all three, in both directions.
//  3. SIM WIRE-LEVEL SWITCH READBACK.  DoAutoReceiveBinTray's pre-switch
//     bAutoTrackCanGoRear gate READS SW[iSwAutoOut[Pos]].Status() and
//     SW[iSwAutoIn[Pos]].Status() (golden :118-:119).  Status() returns false
//     whenever Enable==false, so with an unconfigured switch that gate can never
//     fire.  simSwitchWire() gives a switch a real Sim IO address
//     (Enable + TYPE_A + eMotionNet + Ring1/IP1/Port2/Bit n), after which On()
//     really sets the wire bit and Status() really reads it back.  That is what
//     makes sub-test [10] reachable at all.
//
//  Offline: no vendor SDK, no hardware, no window.
//
//  ===========================================================================
//  NOT-COVERED REGISTER -- this is a deliverable, not an apology
//  ===========================================================================
//  Silence would read as coverage.  Each entry names the arm and the fixture
//  invariant (or missing substrate) that makes it dead.  Sub-test [14] ASSERTS
//  the register's central quantitative claim, and mutation P3 proves that
//  assertion can go red.
//
//  INVARIANTS THAT DO MOST OF THE PRUNING (measured by the exploratory harness
//  this test was derived from, printed and read -- not assumed):
//    (i)   LastSet.iRealDummy == DUMMY for every sub-test EXCEPT [8].
//    (ii)  Machine configuration is at its cmydef.cpp / Config.h default:
//          USE_ROTATE_KIT=0, TRAY_ARM_MODE=0 (eAboveCoveyor), TRAY_VIBRATION=0
//          (NonVibration), USE_COLOR_TRAY_SENSOR=0, USE_LdUldCassetteMode=0,
//          USE_AUTO_RETEST=eartUninstall, bUseAuto2Empty=0, bNoAutoZSelect=0,
//          MachineTypeChoice=100 (NOT Type_HT1032), CUSTOMER_CODE=0 (NOT
//          CC_ASE_KaohSiung / CC_KYEC_LEE / CC_TERAPOWER / CC_TSI / CC_TSMC_TAINAN
//          / CC_VTEST_Shanghai), LOAD_Z_USE_MOTOR[3..5]=false, iAutoRight=2,
//          iAutoIndex[i]==i, and every Ld_UldDelayTime.ULD_* == 0.
//    (iii) Every IniConfig / CosFunction gate this file reads is false:
//          bP37bAutoCylinderUP, bP14EnableAutoTrayRecevieDelayCount, bP61UseTrayTap,
//          bP50DisabledAutoTrackSensorDetect, bP59UnloaderICFloattingAlarmAfterExit,
//          bP18FailAutoTrayManual, bP22EnableFirstTrayNeedAlarm, bP33AutoTrayPreAlarm,
//          bP42AlarmWhenExitTrayComplete, bP53_ForcedScanBinCodeOfUnloader,
//          bP62FirstTrayCheckOnUnloader, bA10_AutoReTest, bA68_AutoLoadUnload,
//          bSPILFunction, bNewResetFunction, bVTESTFunction, bEnable_SECS_GEM,
//          b18bUseAutoTrayMap, b18bDoubleUnloadTray, bUnloadCylinerUpRelief,
//          bAutoFullDoNotStopImmediately, bUseAutoTrayPreAlarm,
//          bFullTrayAlarmAfterUnloadEnd, bAutoTrayFullAlarmCanNotSkip,
//          bAutoSideFixerCheckAlarmAutoRetry, bBarcodeTrayRecFile,
//          bFirstTrayCheckOnUnloader, bNeedAlarmAfterUnloaderFull, and
//          P54UnloaderTaryCheckHasErrorBinIC (except inside [12]).
//    (iv)  Every form predicate is offline false: fAGV->Use_AMR(),
//          fAGV->IsSPIL_AMR(), fAGV->IsATK_AMR(); TrayForm.bEnableAMR=false;
//          TestIF_File.bUseAutoTrayMap=false; fSCKART->iBundleOutCnt==0.
//
//  A. ALL SIX AMR / SECS / BUNDLE-END LIMBS ARE DEAD, by (iii)+(iv):
//     case 850's TrayForm.bEnableAMR block (golden :546-:570) and its
//     TestIF_File.bUseAutoTrayMap block (:573-:589); case 1100's whole
//     IniConfig.bEnable_SECS_GEM EventReport ladder (:1145-:1176) plus the
//     fAGV->IsATK_AMR() sSVBinAssign comma-strip (:1147-:1159); case 1400's
//     fAGV->IsSPIL_AMR() BundleEnd ladder (:1681-:1723); ALL OF case 1415 (the
//     three per-Auto cover-tray-ID blocks, :1726-:1861 -- reached only from 1400's
//     dead ladder); case 1358's KYEC bAMRFullTray block (:1638-:1663); case 1500's
//     EventReport ladder (:1976-:1989).  In particular THE THREE
//     `slDupUnloadBundlID->Find(...)` SITES (:1755, :1797, :1838) ARE NOT
//     EXERCISED -- see GOLDEN BUG 3 below, which is reported from source reading,
//     not from a test observation.
//  B. NOT ENTERED because their customer code is not selected, by (ii):
//     case 850's CC_TSI SaveTrayRecord block (:597-:610); case 1000's
//     CC_ASE_KaohSiung tray-ID rotation (:942-:959); case 910's CC_TSMC_TAINAN
//     side-push suppression (:690-:696); case 1220/1300's CC_KYEC_LEE /
//     CC_TERAPOWER belt-keep-running blocks (:1284-:1300); case 1350's
//     CC_ASE_KaohSiung finish log (:1416-:1421); case 1356's CC_ASE_KaohSiung
//     stack count (:1595-:1604); case 1720's CC_VTEST_Shanghai block (:2233-:2253);
//     every Type_HT1032 limb.
//  C. THE COLOUR-SENSOR SUB-FLOW IS COMPLETELY DEAD (USE_COLOR_TRAY_SENSOR==0 by
//     (ii), and every one of its four guards additionally needs
//     LastSet.iRealDummy!=DUMMY): cases 500, 1600 and 1620's CC_TERAPOWER /
//     non-TERAPOWER colour blocks and ALL of case 1650.  THEREFORE ALL THREE
//     ForTERAPOWERCheckColorSensor CALL SITES (golden :363, :2029, :2074) ARE
//     UNREACHED here -- the function itself is tested by the sibling
//     tests/test_w7_l1_color.cpp, which owns it.
//  D. THE OCR / CASSETTE / VIBRATION LIMBS ARE NOT ENTERED:
//     cases 110/111/112/113 (need IniConfig.bP37bAutoCylinderUP); cases 910/920's
//     TRAY_VIBRATION!=NonVibration halves; case 1030 and case 1010's
//     USE_LdUldCassetteMode==1 hand-off (DoAutoCassetteTrayFeed IS covered, but
//     STANDALONE in [13], not through case 1030); case 956 (needs
//     TrayForm.bMoveAfterTrayGoOut); case 1022 and case 1052 (need
//     Sen[SnAutoTrayDetect].IsOff()==true while DUMMY -- the fixture keeps that
//     sensor "unknown"); case 1215; case 1351 / 13520 / 13521 (need
//     LastSet.iRealDummy!=DUMMY on the ART path); case 1550; case 1620; case 1720.
//  E. THE `#ifdef SOFT_SIMULTE` ARMS ARE NOT COMPILED IN THIS BUILD (cases 1010
//     and 1710 each have one).  The `#ifndef SOFT_SIMULTE` real-machine arms ARE
//     compiled and ARE what this test drives.
//  F. THE FOURTEEN TU-LOCAL STAND-INS asendic_Auto.cpp installs: four are FULL
//     FAITHFUL TRANSLATIONS and ARE exercised --
//       * bARTUnloaderUseTwoCylin -- BOTH return values, [5] (false) vs [6] (true),
//         and the two trajectories differ, which is the strongest statement this
//         suite can make about the AutoCylinder swap (see G).
//       * PushUnLoaderTrayInAverageTime -- CALLED but its body early-returns:
//         golden's own `if(st<500 || st>8000) return;` (asendic.cpp:1249) rejects
//         the sub-millisecond LatchCycleTime this harness produces, so
//         LastSet.iUnLoaderTraySimulateTime is never written.  NOT COVERED, and
//         it is golden's own guard doing it, not a stand-in artefact.
//         (It is only reached at all when iRealDummy!=DUMMY, golden :1063.)
//       * DoAutoTrayEdgeCylinderLoop -- CALLED at golden :485/:495 and returns
//         false on golden's own first line (IniConfig.bP13EnableAutoTrayEdge-
//         PushCylinderLoop==false), so its cylinder-toggling body is NOT COVERED.
//       * CheckOutArmZ -- CALLED at golden :527 but returns false immediately
//         because OutArmSuck.iPickRow==0 offline; its per-nozzle loop and its
//         WAR0257 arm are NOT COVERED.
//     The other ten are no-ops / storage and nothing downstream of
//     SaveUnloaderInfo / SaveProductionRecord / SaveTrayRecord / EditTray /
//     fBarCode->Write_Device_Info_By_Tray / fMesSystem->bFirstMaterialsQA /
//     iAutoTrayData / bOldAutoHasTray / GetBundleInfo /
//     CheckAllAutoTrayEjectFinsh / bNeedEject is exercised.  Every one of those
//     call sites is ALSO inside an arm this register already lists as dead,
//     EXCEPT SaveUnloaderInfo (golden :1103) and SaveProductionRecord
//     (golden :607) -- both of which are only reached when iRealDummy!=DUMMY /
//     CUSTOMER_CODE==CC_TSI respectively, so neither runs here either.
//  G. THE AutoCylinderUp/Middle/Lower ARGUMENT ORDER IS NOT OBSERVABLE, AND THIS
//     IS THE MOST IMPORTANT ENTRY IN THIS REGISTER.  acatchtray_shims.cpp's three
//     stubs are `{ return true; }` and DISCARD all four arguments, so no fixture
//     can distinguish `AutoCylinderLower(Pos, C_Auto_Up, C_Auto_Selector, true)`
//     from `AutoCylinderLower(Pos, C_Auto_Selector, C_Auto_Up, true)`.  That
//     includes THE TWO RUNTIME SWAPS THIS FILE OWNS -- golden :788 vs :784 and
//     :819 vs :811.  What [5] and [6] DO prove is that BOTH SIDES of the
//     bARTUnloaderUseTwoCylin test that selects between them are live and
//     produce different behaviour, so neither arm is dead code.  The ORDER
//     itself is pinned only by the source and by acatchtray_shims.h's Wave-0
//     comment; it becomes testable when Wave 3 lands the real bodies, and the
//     wave that lands them MUST re-check these call sites.  No assertion here
//     claims otherwise.
//  H. TrayMoveStatus() HAS NO SEAM (hardwired `return 0`, see the note above its
//     declaration in asendic.h).  It is read once in this file, at case 1220's
//     CC_KYEC_LEE / CC_TERAPOWER block (golden :1284), which (B) already lists as
//     dead -- so unlike the Loader family nothing here is blocked by it.
//     TrayMoveIn / TrayMoveOut / TrayCylinMoveIn / TrayCylinMoveOut are likewise
//     Sim no-ops with no observable, so EVERY belt command in this file is
//     unobservable; no assertion claims one.
//  I. THREE FURTHER ARMS ARE DEAD BECAUSE A PORTED STUB, NOT A FIXTURE CHOICE,
//     KILLS THEM -- recorded separately so they are not mistaken for
//     configuration:
//       * case 850's "AUTO%d 中有空格" ShowMyMessage (golden :519-:524) needs
//         MOT[].Tray.HasDataIC(HAS_NULL_IC), and mytray.cpp:168's
//         `HasDataIC(int) { return false; }` is a stub.
//       * case 2470's EditTray branch inside CheckHasErrorBinOnTray needs
//         Tray.HowManyBinICInTray(...)>0, and mytray.cpp:174 returns 0.
//         [12] therefore covers the "no error bin" and the "unsupported motor"
//         exits only.
//       * case 1220's iAutoHasHod write DOES run and IS asserted by [5]; its
//         bAQL_Sorting_Finish alternative (golden :1321-:1322) is not.
//  J. NOT EXERCISED AT ALL, and named rather than left silent: cases 300's
//     TrayForm.LoaderToEmptyColor==2 limb, 750's SoftStop limb, 850's iRetry
//     3000-tick JAM1108 limb, 900/910's whole IniConfig.bP14 recevie-loop,
//     1021's JAM1112 timeout, 1025's re-arm-to-1020 limb, 1100's three
//     full-tray limbs, 1200's bHasICFloating JAM arm, 1210's sensor-abnormal
//     ShowUnloaderTrayMessage arm, 1354/1356's sensor-abnormal arms, 1400's
//     JAM1107 arm, 1420's take-off-the-bundle arm, 1500's pre-alarm arm,
//     1620's JAM1108 arm, 1710's JAM1103 arm, and DoAuto123TrayToRear's
//     cases 300/400 (reachable only from case 400's own bFlag arm, which needs
//     Sen[SnAutoFixCyPush].IsOn()).
//
//  ===========================================================================
//  GOLDEN DEFECTS FOUND WHILE TRANSLATING -- reported, translated verbatim
//  ===========================================================================
//  Every line number below was re-derived from the cp950-decoded golden in this
//  pass.  None of these was "fixed": asendic_Auto.cpp reproduces all of them.
//
//  1. IDENTICAL IF/ELSE ARMS, TWICE.  golden :176-:179 --
//     `if(IniConfig.bSPILFunction) ret=ShowErrorMessage(sJAM1108[Pos],
//     K_SKIP|K_RETRY, iMMAuto[Pos]); else ret=ShowErrorMessage(sJAM1108[Pos],
//     K_SKIP|K_RETRY, iMMAuto[Pos]);` -- byte-identical arms, so the SPIL branch
//     does nothing.  Golden's own comment on the SPIL arm says SPIL should only
//     be allowed to SKIP, which is what the arm was presumably meant to
//     implement.  The same shape again at golden :228-:231 (sJAM1110, both arms
//     K_RETRY, same comment).
//  2. A CONDITION THAT IS TRUE FOR EVERY INPUT.  golden :1079-:1080 --
//     `if(A!=(N+1) || A!=(N*2+1))` where A is iUnloaderTrayCountCal[Pos] and N is
//     IniConfig.iUnloaderTrayCount[Pos].  N+1 and 2N+1 differ for every N except
//     0, so for any non-zero N no value of A can fail BOTH tests and the guard is
//     a tautology: the Barcode device-info write and the extra
//     iUnloaderTrayCountCal++ fire on every pass through case 1050.  `&&` is
//     almost certainly what was meant.  (Not reachable in this build --
//     IniConfig.b18bUseAutoTrayMap is false -- so no test pins it.)
//  3. BINARY SEARCH OVER AN UNSORTED LIST.  golden :1753-:1755 (and identically
//     :1795-:1797 and :1836-:1838) do `Clear(); LoadFromFile(); Find()` with NO
//     `Sort()` in between.  TStringList::Find is a BINARY SEARCH defined only on
//     a sorted list (see the Wave-0 note at vclcompat/TStringList.h:168), so on
//     the raw file order it can MISS an existing duplicate bundle ID -- i.e. the
//     duplicate-bundle-ID guard silently fails OPEN.  This is an inconsistency
//     INSIDE golden, not a VCL subtlety: golden asendic_Loader.cpp:1408-:1412
//     calls the same API correctly, Clear / CommaText= / Sort / Find.
//  4. MOTOR-INDEX / TRAY-INDEX TYPE CONFUSION.  golden :1921-:1926 reads and
//     clears `MOT[iWhichAuto].sUnloaderAlarmMsg`.  iWhichAuto is a TRAY-POSITION
//     index in 0..5 -- golden compares it against eEmpty / eColor / iFixRight and
//     uses it as `bAutoEdgePush[iWhichAuto]` (acatchtray.cpp:9032, an array of
//     MAX_AUTO_TRAY) -- but MOT[] is indexed by MOTOR id, and the Auto-car motors
//     are MMAuto1_Car.. (a completely different range).  So this block inspects
//     and clears the alarm text of an unrelated low-numbered motor.  The three
//     statements immediately above and below it in the SAME block all use
//     `iMMAuto_Car[Pos]` correctly.  Reproduced verbatim; `extern int iWhichAuto;`
//     binds to the one real global (aoutarm9045.cpp:179).
//  5. AN EMPTY THEN-BRANCH USED AS A GUARD.  golden :1881-:1888 --
//     `if(IniConfig.bA68_AutoLoadUnload) { } else { ShowErrorMessage(...); }`.
//     Harmless as written, but it is the same construct that has hidden real
//     bugs elsewhere in this family, and it reads as unfinished work.
//  6. DEAD STORAGE.  golden :2299 defines
//     `TQPF_Timer tAuto123TrayToRearTimer[MAX_AUTO_TRAY];` and nothing in the
//     entire golden tree ever touches it (grepped every .cpp/.h in the cp950
//     decode).  Translated verbatim.
//  7. CONTIGUOUS-MOTOR-ID ASSUMPTION.  golden :2515, :2516, :2531 and :2540 index
//     `MOT[MAuto1Y_CCW+iPos]` / `MOT[MAuto1Y+iPos]`, assuming the per-Auto Y motor
//     ids are numerically contiguous, while the rest of this file always goes
//     through the iMMAuto[] / iMMAuto_Car[] indirection tables.  Faithful, but it
//     is exactly the assumption that breaks silently on a re-numbered motor table.
//  8. A `case 500` THAT RETURNS true WITHOUT RESETTING ITS OWN CURSOR.  golden
//     :2547-:2551, DoAutoCassetteTrayFeed -- so every subsequent call answers
//     "done" instantly until somebody calls InitAutoCassetteTrayFeedTask.  Its two
//     golden callers (asendic_Auto.cpp:1035 case 1030, and case 1010/1021's
//     USE_LdUldCassetteMode hand-offs) do call Init first, so it is latent rather
//     than live -- but every OTHER Do* in this family resets its cursor before
//     returning true.  Sub-test [13] PINS the behaviour as-is so a future "tidy"
//     cannot change it silently.
//
//  ===========================================================================
//  FAILABILITY PROOF -- REPRODUCIBLE RECIPE
//  ===========================================================================
//  Every mutation below was applied to a COPY of the translated asendic_Auto.cpp
//  in a private scratch dir, compiled off-tree with the flags CMake records for
//  target ht9045_sm (-std=c++1z -DCSYSTEM_TICK_ORACLE -DWINVER=0x0601
//  -D_WIN32_WINNT=0x0601 -Wall -Wextra -Wno-conversion-null -Wno-write-strings
//  -Wno-unused-parameter -I<tree> -I<tree>/SECSGEM), `ar r`'d into a COPY of
//  libht9045_sm.a, and linked into a private exe.  THE SHARED build/ TREE WAS
//  NEVER WRITTEN TO.  asendic_Auto.cpp was sha256-verified byte-identical before
//  and after the whole campaign.  The tables quote the ASSERTION MESSAGE, never
//  a `(line N)` suffix -- the message is stable, the line number is pushed by
//  every edit to this banner.  Knock-on failures are counted but only the
//  PRIMARY assertion is named where a mutation reddens several.
//
//  BASELINE (asendic_Auto.cpp pristine):  RESULT: 81 passed, 0 failed  (exit 0)
//
//  --- [5]/[6] the two full cycles ------------------------------------------
//  (M1)  case 800 edge-push On -> Off              -> 79/2  case 800 locked the tray...
//  (M2)  case 955 up-press Off deleted             -> 79/2  case 955 released the tray...
//  (M3)  case 1220 MoveTrayAllItem deleted         -> 75/6  case 1220 moved the tray CONTENTS onto the car
//  (M4)  case 1420 car ClearTray deleted           -> 80/1  case 1420 cleared the car tray at cycle end
//  (M5)  case 850 iUnloaderTrayCountCal++ deleted  -> 80/1  case 850 counted the tray
//  (M6)  case 900 bAUTORev=true -> false           -> 80/1  case 900 raised bAUTORev
//  (M7)  case 1220 iAutoHasHod write deleted       -> 80/1  case 1220 wrote iAutoHasHod
//  (M9)  case 1350 ART hand-off 1352 -> 1400       -> 78/3  ART run entered the 1352..1358 separator chain
//  (M10) case 1355 Pop -> Push                     -> 80/1  case 1355 retracted the Z separator
//        (M10 leaves the cursor trajectory IDENTICAL -- offline Push() and Pop()
//         both return true -- so ONLY the bCylinderOn read catches it.  That is
//         the whole reason rule 3 exists.)
//
//  --- [7] the golden fall-through -------------------------------------------
//  (M8)  explicit `break;` added after case 950's `Task=955;`
//                                                  -> 78/3  case 950 FALLS THROUGH into case 955
//        (+ both EXACT trajectories, which is the point: the fall-through is
//         load-bearing for the cursor sequence, not a cosmetic detail)
//
//  --- [8] the JAM1109 alarm pair -------------------------------------------
//  (M11) case 750 K_SKIP arm's Auto ClearTray deleted -> 80/1  JAM1109 K_SKIP cleared the Auto tray
//  (M12) case 750 else-arm side-fixer On -> Off       -> 80/1  JAM1109 K_RETRY re-locked the tray
//
//  --- [9]/[10] track detect + the pre-switch gate ---------------------------
//  (M13) DoAutoTrackDetectICFloating case 100 `return true` -> `return false`
//                                                  -> 73/8  sensor ON -> returns true
//  (M14) explicit `break;` after that function's case 1 `Task=100;`
//                                                  -> 74/7  case 1 FALLS THROUGH into case 100:
//                                                           the FIRST call from a fresh cursor already detects
//  (M15) pre-switch gate Task=920 -> 921            -> 76/5  track-detect gate parks the SM on case 920
//  (M16) pre-switch gate bAtuoTrayICDetectErr write deleted
//                                                   -> 80/1  track-detect gate latches bAtuoTrayICDetectErr
//  (M17) case 920's ShowErrorMessage(sJAM1110) deleted
//                                                   -> 77/4  case 920 raised JAM1110 (+ [14]'s ledger)
//
//  --- [11] DoAuto123TrayToRear ---------------------------------------------
//  (M18) case 500 Push -> Pop                       -> 80/1  case 500 extended the side fixer
//  (M19) case 600 Pop -> Push                       -> 80/1  case 600 retracted the side fixer before returning
//  (M20) case 200 double-tray `return true` -> false-> 80/1  both trays present -> immediate true
//  (M21) explicit `break;` after case 1's Task=100  -> 79/2  EXACT rear trajectory
//
//  --- [12] CheckHasErrorBinOnTray ------------------------------------------
//  (M22) the "not support" ShowMyMessage deleted    -> 79/2  ...-> "not support" (+ [14]'s ledger)
//  (M23) the `if(IniConfig.P54...)` guard removed   -> 80/1  P54 OFF -> completely silent
//        WHY THE PROBE USES AN UNRECOGNISED MOTOR: against the obvious
//        `CheckHasErrorBinOnTray(iMMAuto[0])` probe, M23 stayed GREEN 81/0,
//        because HowManyBinICInTray is a ported stub returning 0 so the body is
//        silent anyway.  Measured, then fixed -- this is register entry I.
//
//  --- [13] DoAutoCassetteTrayFeed ------------------------------------------
//  (M24)  `if(iPos>1)` -> `if(false)`               -> 81/0  GREEN, AND IT IS
//         CORRECT TO RECORD IT RATHER THAN CHASE IT.  Removing that guard makes
//         `int &Task=iDoAutoCassetteTrayFeed[iPos]` read PAST the end of a
//         2-element array; what happens next is undefined and depends on whatever
//         object the linker placed next (here: a value != 1, so the switch matches
//         no case and nothing observable happens).  A test cannot pin UB, so the
//         guard is pinned by two mutations that stay in-bounds instead:
//  (M24a) `if(iPos>1) return true;`                 -> 80/1  iPos==2 is rejected ... (returns false)
//  (M24b) `if(iPos>1) { SetSecAndOn(2); return false; }`
//                                                   -> 80/1  ...and it returns BEFORE case 1 arms the 2 s feed timer
//  (M25)  case 400's carrier `Off()` deleted        -> 80/1  case 400 lowered the carrier BEFORE the completion tick
//         Against the weaker "went UP then back DOWN" form this mutation stayed
//         GREEN 81/0, because case 500's Pop() masks it -- the same masked-pair
//         shape asendic_Loader's Q11/G5 recorded.  The strict `offTick < doneTick`
//         form is what separates them.
//
//  --- [1]/[3] entry points and the Pos guard -------------------------------
//  (M26) Initial_Auto_BinTray_Task body deleted     -> 76/5  Initial_Auto_BinTray_Task(0) -> BinTrayTask[0]==1
//  (P2)  the `if(Pos<0 || iAutoIndex[Pos]>iAutoRight) return;` guard removed
//                                                   -> 77/4  Pos<0 is rejected (cursor stays on 1)
//        An earlier draft of [3] probed with a bogus cursor (777) and would have
//        stayed GREEN under P2, because a cursor matching no `case` sits still
//        with or without the guard.  Both halves now start from cursor 1 with a
//        full tray, where the SM demonstrably moves 1 -> 100.
//
//  --- [14] the register self-check -----------------------------------------
//  (P3)  case 750's ShowErrorMessage duplicated     -> 79/2  EXACT alarm ledger: 3 ShowErrorMessage
//        calls in the whole suite.  This is the NOT-COVERED register's central
//        claim going red on demand, which is the entire purpose of [14].
//
//  --- SIBLING REGRESSION (the shim retirements this landing forces) ---------
//  Retiring acatchtray_shims' DoAutoReceiveBinTray body and csystem_shims'
//  Initial_Auto_BinTray_Task body replaces two no-ops with the real SM, so every
//  test that pumps csystem/acatchtray could in principle move.  MEASURED, not
//  assumed: ten sibling tests were rebuilt against the retired-shim archive and
//  their stdout diffed against the pristine build -- test_w6_6_csystem_cycle,
//  test_w6_6_hub, test_w6_canary, test_w6_1_empty_canary, test_w6_3_catchtray,
//  test_w7_c1_cleanout_finish, test_w7_c2_onecycle_finish, test_w7_l1_auto2,
//  test_w7_l1_color, test_w7_l1_loader.  ALL TEN: exit 0, ZERO-LINE DIFF.  No
//  re-baseline is needed and no sibling test file needs an edit.
// =============================================================================
#include "asendic_Auto.h"
#include "asendic_Color.h"      // ForTERAPOWERCheckColorSensor (register entry C)
#include "csystem.h"
#include "asendic.h"
#include "Motor/mymotor.h"
#include "mycylin.h"
#include "mysensor.h"
#include "myswitch.h"
#include "myTimer.h"            // TQPF_Timer (the virtual-time seam below)
#include "mytray.h"
#include "cprod.h"
#include "cpublic.h"
#include "cmydef.h"
#include "canary_support.h"     // LastSet, ShowErrorMessage seam, ShowMyMessage seam
#include "FormsFacade.h"        // fMain / fAGV / fSCKART / fFixAICCD / fProductionInfo
// AI(W906-W7-L1-Wave3) 20260802: AutoCylinderUp/Middle/Lower are no longer the
// `{ return true; }` stubs in acatchtray_shims.cpp -- asendic.cpp now carries
// golden's real closed-loop bodies.  This header supplies the physical Auto stack
// that answers their position sensors.
#include "w3_cylinder_plant.h"
#include <cstdio>
#include <vector>

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
//  VIRTUAL-TIME SEAM -- asendic_Auto.cpp's file-scope TQPF_Timer objects.
//  (golden line numbers re-derived from the cp950 decode in this pass)
// ---------------------------------------------------------------------------
extern TQPF_Timer ULDStackDelay          [MAX_AUTO_TRAY];   // golden asendic_Auto.cpp:50
extern TQPF_Timer DetectAutoTime         [MAX_AUTO_TRAY];   // golden :54  -- DUAL ROLE
extern TQPF_Timer DetectAutoTimePause    [MAX_AUTO_TRAY];   // golden :55  -- DUAL ROLE
extern TQPF_Timer ColorSensorDelayTime   [MAX_AUTO_TRAY];   // golden :56  -- NEVER advanced
extern TQPF_Timer htUnLoaderTimeDelay    [MAX_AUTO_TRAY];   // golden :64
extern TQPF_Timer htSwACAutoOffDelay     [MAX_AUTO_TRAY];   // golden :65
extern TQPF_Timer AutoReceiveBinTrayDelay[MAX_AUTO_TRAY];   // golden :66  -- DUAL ROLE
extern TQPF_Timer RecevieLoopDelay       [MAX_AUTO_TRAY];   // golden :67
extern TQPF_Timer htUnLoaderTimeRelief   [MAX_AUTO_TRAY];   // golden :69
extern TQPF_Timer DoTrayGoOutNeedDelay   [MAX_AUTO_TRAY];   // golden :71
extern TQPF_Timer hAuto123TrayToRear         [MAX_AUTO_TRAY];  // golden :2291 -- DUAL ROLE
extern TQPF_Timer hAuto123TrayToRearForDummy [MAX_AUTO_TRAY];  // golden :2292
extern TQPF_Timer hAutoCassetteTrayFeed;                       // golden :2482 -- DUAL ROLE
extern bool bAutoEdgePush[MAX_AUTO_TRAY];                   // golden acatchtray.cpp:55

//  Pure progress + the four cursor-pinned dual-role advances (see banner seam 1).
static void advanceAuto(int P)
{
    ULDStackDelay[P].SetMSAndOn(0);
    htSwACAutoOffDelay[P].SetMSAndOn(0);
    RecevieLoopDelay[P].SetMSAndOn(0);
    htUnLoaderTimeRelief[P].SetMSAndOn(0);
    DoTrayGoOutNeedDelay[P].SetMSAndOn(0);
    htUnLoaderTimeDelay[P].SetMSAndOn(0);
    if (BinTrayTask[P] == 970)  DetectAutoTimePause[P].SetMSAndOn(0);
    if (BinTrayTask[P] == 1020 || BinTrayTask[P] == 1025 ||
        BinTrayTask[P] == 1210 || BinTrayTask[P] == 1220)
        DetectAutoTime[P].SetMSAndOn(0);
    if (BinTrayTask[P] == 1550) AutoReceiveBinTrayDelay[P].SetMSAndOn(0);
    // AI(W906-W7-L1-Wave3) 20260802: one tick of the Auto-stack plant.  When no
    // pair is registered (the non-ART sub-tests, which reach AutoCylinderLower
    // only, and converge on unwired cylinders exactly as golden does) this just
    // expires AutoTime[][] -- the same virtual-time idiom as the lines above.
    w3::Tick();
}
//  DoAuto123TrayToRear: hAuto123TrayToRear is the 20 s JAM1101 watchdog at
//  cursor 200 (golden :2354) but the LD_TrayArrivalDely settle timer armed by
//  case 450 and read by case 460 (golden :2409/:2413).  Advanced ONLY on 460.
static void advanceRear(int P)
{
    if (iAuto123TrayToRearTask[P] == 460) hAuto123TrayToRear[P].SetMSAndOn(0);
    hAuto123TrayToRearForDummy[P].SetMSAndOn(0);
}

// ---------------------------------------------------------------------------
//  SIM SENSOR SEAM -- see banner seam 2.
// ---------------------------------------------------------------------------
static void simSensorOff(int idx)       // IsOff()==true , IsOn()==false
{
    Sen[idx].Enable  = true;
    Sen[idx].Type    = TYPE_A;          // cmydef.cpp TYPE_A==1
    Sen[idx].ISABase = eISABase;        // -> mysensor.cpp raw-port stub (false)
}
static void simSensorOn(int idx)        // IsOn()==true  , IsOff()==false
{
    Sen[idx].Enable  = true;
    Sen[idx].Type    = TYPE_B;          // cmydef.cpp TYPE_B==0 (the DEFAULT type)
    Sen[idx].ISABase = eISABase;
}
static void simSensorUnknown(int idx)   // IsOn()==false AND IsOff()==false
{
    Sen[idx].Enable = false;
}

// ---------------------------------------------------------------------------
//  SIM WIRE SEAM -- see banner seam 3.
// ---------------------------------------------------------------------------
static void simSwitchWire(int idx, int bit)
{
    SW[idx].Enable  = true;
    SW[idx].Type    = TYPE_A;
    SW[idx].ISABase = eMotionNet;       // -> MyLaneIO Sim backend (real bit store)
    SW[idx].Ring    = 1;
    SW[idx].IP      = 1;
    SW[idx].Port    = 2;
    SW[idx].Bit     = bit;
}

// ---------------------------------------------------------------------------
//  Ordered trajectory recorder: the cursor BEFORE the first tick and after
//  every tick, consecutive duplicates collapsed -> the ordered sequence of
//  distinct resting states.  NOT a set.
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
static bool trajHas(const Traj &t, int v)
{
    for (size_t i = 0; i < t.size(); ++i) if (t[i] == v) return true;
    return false;
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
//  ALARM / DIALOG LEDGER.  setupAutoFixture() resets both seams at the start of
//  every sub-test, so [14] would otherwise only measure the LAST sub-test --
//  exactly the quietly-vacuous shape this file exists to avoid.  The counts are
//  BANKED here before each reset and [14] asserts bank + current.
// ---------------------------------------------------------------------------
static int g_alarmsBanked = 0, g_dialogsBanked = 0;

// ---------------------------------------------------------------------------
//  Fixture.  Enables the actuators golden guards on, sets the whole Auto sensor
//  bank to "unknown" (an unconfigured offline IO table -- golden reads these as
//  `IsOff()==false`, which "unknown" satisfies) and PRE-DRIVES every observed
//  cylinder to the OPPOSITE of what the assertions expect.
// ---------------------------------------------------------------------------
static void setupAutoFixture(int P)
{
    g_alarmsBanked  += W906_ShowErrorMessage_Count;
    g_dialogsBanked += W906_ShowMyMessage_Count;

    LastSet.iRealDummy = DUMMY;

    Cylinder[C_AutoSide_Fixer[P]].Enable = true;
    Cylinder[C_AutoEdgePush[P]].Enable   = true;
    Cylinder[C_AutoUpPress[P]].Enable    = true;
    Cylinder[C_Auto_Up[P]].Enable        = true;
    Cylinder[C_Auto_Selector[P]].Enable  = true;
    Cylinder[C_AutoZ_Select[P]].Enable   = true;
    Cylinder[C_TrayX_UpDown].Enable      = true;

    // PRE-DRIVE TO THE OPPOSITE (rule 3): every "ON at cursor N" assertion below
    // starts from OFF, and the one "OFF at cursor N" pair is preceded in the same
    // run by a proven ON, so deleted == inverted for all of them.
    Cylinder[C_AutoSide_Fixer[P]].Off();
    Cylinder[C_AutoEdgePush[P]].Off();
    Cylinder[C_AutoUpPress[P]].Off();
    Cylinder[C_Auto_Selector[P]].Off();
    Cylinder[C_Auto_Up[P]].Off();
    Cylinder[C_AutoZ_Select[P]].On();       // opposite of the case-1355 Pop()

    simSensorUnknown(SnAutoTrayDetect[P]);
    simSensorUnknown(SnAutoTrayCar[P]);
    simSensorUnknown(SnAutoPreDete[P]);
    simSensorUnknown(SnAutoUpSafe[P]);
    simSensorUnknown(SnAutoIsFull[P]);
    simSensorUnknown(SnAutoTrackDetect[P]);
    simSensorUnknown(SnAutoHasCoverTray[P]);
    simSensorUnknown(SnAutoTrayHasTray[P]);
    simSensorUnknown(SnAutoZSelect1[P]);
    simSensorUnknown(SnAutoZSelect2[P]);
    simSensorUnknown(SnAutoZSelect3[P]);
    simSensorUnknown(SnAutoZSelect4[P]);
    simSensorUnknown(SnAutoCWPreDetect[P]);
    simSensorUnknown(SnAutoFixCyPush[P]);
    simSensorUnknown(SnAutoIsPreAlarm[P]);

    bAtuoTrayICDetectErr   = false;
    bAutoTrackCanGoRear    = false;
    bHandlerPause          = false;
    SoftStop               = false;
    bAutoEdgePush[P]       = true;          // golden case 300 needs this to leave 300
    bAutoIsFullNeedStop[P] = false;
    bResetUnLoadTray[P]    = false;
    bAUTORev[P]            = false;
    iUnloaderTrayCountCal[P] = 0;
    iAutoHasHod[P]         = -1;            // sentinel: case 1220 must overwrite it
    iAuto_CarCarDeviceCnt[P] = -1;

    W906_ShowErrorMessage_Reset();
    W906_ShowMyMessage_Reset();

    // AI(W906-W7-L1-Wave3) 20260802: no Auto-stack pair is registered by default.
    // MEASURED, not assumed: the non-ART cycle reaches only AutoCylinderLower --
    // at golden :819, `AutoCylinderLower(Pos, C_Auto_Selector[Pos], C_Auto_Up[Pos])`,
    // the REVERSED order -- and golden's real AutoCylinderLower converges on
    // unwired cylinders, because its case 100 reads the Ifor-20200506
    // bOffStatus/bMidOnStatus pair (asendic.cpp :954-970) which substitutes
    // true/false when the sensors are disabled.  Every other non-ART lifter call
    // site in this SM is `Cylinder[C_Auto_Selector[Pos]].Push()` (golden :1391),
    // not AutoCylinder*.  The ART sub-test [6] registers the pair itself.
    w3::Forget();
    w3::ResetCursors();
    Ld_UldDelayTime.LD_BeforeDownDelay    = 0;
    Ld_UldDelayTime.LD_StackMiddLockDelay = 0;
    Ld_UldDelayTime.LD_LiftDownDelay      = 0;
}

//  Give the Auto tray a FULL 1x1 tray.  TTrayMotor::SetTray() only fills the
//  data grid when fHTary is set (mymotor.cpp:1225-1233) and fHTary is protected
//  and false offline, so the grid is written directly -- otherwise FullIC() stays
//  false and golden case 850 never advances.
static void giveFullTray(int mot)
{
    MOT[mot].fHasTray = true;
    MOT[mot].Tray.SetData(HAS_IC);
}
static void giveNoTray(int mot)
{
    MOT[mot].fHasTray = false;
    MOT[mot].Tray.ClearData();
}

// ===========================================================================
//  [1] Init* entry points reset their own cursor
// ===========================================================================
static void test_inits()
{
    printf("\n[1] Init* reset their own task cursor\n");

    BinTrayTask[0] = 999;                       Initial_Auto_BinTray_Task(0);
    CHECK(BinTrayTask[0] == 1, "Initial_Auto_BinTray_Task(0) -> BinTrayTask[0]==1");

    BinTrayTask[2] = 999;                       Initial_Auto_BinTray_Task(2);
    CHECK(BinTrayTask[2] == 1, "Initial_Auto_BinTray_Task(2) -> BinTrayTask[2]==1 (goes through iAutoIndex[])");

    iAutoTrackDetectICFloatingTask[1] = 999;    InitAutoTrackDetectICFloatingTask(1);
    CHECK(iAutoTrackDetectICFloatingTask[1] == 1,
          "InitAutoTrackDetectICFloatingTask(1) -> cursor==1");

    iAuto123TrayToRearTask[2] = 999;            InitAuto123TrayToRearTask(2);
    CHECK(iAuto123TrayToRearTask[2] == 1, "InitAuto123TrayToRearTask(2) -> cursor==1");
}

// ===========================================================================
//  [2] Fixture seams -- both directions of each, so no single broken behaviour
//      can satisfy them all.
// ===========================================================================
static void test_seams()
{
    printf("\n[2] fixture seams (sensor tri-state, switch wire, virtual time)\n");

    const int s = SnAuto1TrayDetect;
    simSensorUnknown(s);
    CHECK(Sen[s].IsOn() == false && Sen[s].IsOff() == false,
          "simSensorUnknown -> IsOn()==false AND IsOff()==false (tri-state, not complements)");
    simSensorOff(s);
    CHECK(Sen[s].IsOff() == true && Sen[s].IsOn() == false,
          "simSensorOff -> IsOff() only");
    simSensorOn(s);
    CHECK(Sen[s].IsOn() == true && Sen[s].IsOff() == false,
          "simSensorOn (TYPE_B, the default type) -> IsOn() only");
    simSensorUnknown(s);

    const int w = SwACAuto1;
    simSwitchWire(w, 4);
    SW[w].Off();
    CHECK(SW[w].Status() == false, "simSwitchWire + Off() -> Status() reads the wire bit LOW");
    SW[w].On();
    CHECK(SW[w].Status() == true,  "simSwitchWire + On()  -> Status() reads the wire bit HIGH");
    SW[w].Off();

    TQPF_Timer t;
    t.SetMSAndOn(60000);
    CHECK(t.Off() == false, "TQPF_Timer armed 60 s -> Off()==false");
    t.SetMSAndOn(0);
    CHECK(t.Off() == true,  "TQPF_Timer armed 0 ms -> Off()==true (this is how advanceAuto works)");
}

// ===========================================================================
//  [3] The Pos guard (golden :73-:74) -- two-directional
// ===========================================================================
static void test_pos_guard()
{
    printf("\n[3] DoAutoReceiveBinTray Pos guard\n");

    // BOTH halves start from a cursor the SM WOULD move: case 1 with a full Auto
    // tray and an empty car deterministically hands off to 100 (golden :255).  A
    // bogus cursor like 777 would sit still with or without the guard -- that
    // would have been a tautology, and an earlier draft of this sub-test was
    // exactly that until the run showed it.
    setupAutoFixture(0);
    giveFullTray(iMMAuto[0]);
    giveNoTray(iMMAuto_Car[0]);
    Initial_Auto_BinTray_Task(0);
    DoAutoReceiveBinTray(-1);
    CHECK(BinTrayTask[0] == 1, "Pos<0 is rejected (cursor stays on 1)");
    DoAutoReceiveBinTray(0);
    CHECK(BinTrayTask[0] == 100, "...while Pos==0 on the SAME fixture advances 1 -> 100");
    giveNoTray(iMMAuto[0]);
    BinTrayTask[0] = 1;

    // Pos 3 is beyond iAutoRight (==2 by default).
    setupAutoFixture(3);
    giveFullTray(iMMAuto[3]);
    giveNoTray(iMMAuto_Car[3]);
    Initial_Auto_BinTray_Task(3);
    const int savedRight = iAutoRight;
    DoAutoReceiveBinTray(3);
    CHECK(BinTrayTask[3] == 1, "Pos beyond iAutoRight is rejected (cursor stays on 1)");

    // ... and the SAME call with the bound raised DOES run, which is what makes
    // the claim above two-directional instead of a tautology.
    iAutoRight = 3;
    DoAutoReceiveBinTray(3);
    CHECK(BinTrayTask[3] == 100,
          "raising iAutoRight to 3 lets the SAME call through (1 -> 100; the guard is load-bearing)");
    iAutoRight = savedRight;
    giveNoTray(iMMAuto[3]);
    BinTrayTask[3] = 1;
}

// ===========================================================================
//  [4] USE_ROTATE_KIT early return (golden :103-:113)
// ===========================================================================
static void test_rotate_kit_early_return()
{
    printf("\n[4] USE_ROTATE_KIT rotate-output early return\n");

    const int P = 1;                         // iRotate_Out_Tray6 defaults to 1
    setupAutoFixture(P);
    BinTrayTask[P] = 777;

    const int savedKit  = USE_ROTATE_KIT;
    const int savedType = TrayForm.iRotateKIT_OutputType;
    USE_ROTATE_KIT = 1;
    TrayForm.iRotateKIT_OutputType = 1;

    CHECK(P == iRotate_Out_Tray6, "[4] precondition: iRotate_Out_Tray6 selects Auto2");
    DoAutoReceiveBinTray(P);

    CHECK(Cylinder[C_AutoSide_Fixer[P]].bCylinderOn == true &&
          Cylinder[C_AutoEdgePush[P]].bCylinderOn   == true &&
          Cylinder[C_AutoUpPress[P]].bCylinderOn    == true,
          "rotate-kit arm clamps the tray: side-fixer+edge-push+up-press all ON");
    CHECK(BinTrayTask[P] == 777,
          "rotate-kit arm RETURNS without touching the cursor");

    USE_ROTATE_KIT = savedKit;
    TrayForm.iRotateKIT_OutputType = savedType;
    BinTrayTask[P] = 1;
}

// ---------------------------------------------------------------------------
//  Shared driver for the two full-cycle sub-tests.  Captures the states the
//  assertions need AT THE CURSOR WHERE GOLDEN COMMANDS THEM, not at the end.
// ---------------------------------------------------------------------------
struct CycleObs
{
    Traj traj;
    int lockedAt850;        // fixer+edge+press all ON at cursor 850
    int releasedAt960;      // fixer+edge+press all OFF at cursor 960
    int carHasTrayAt1350;   // MOT[Car].fHasTray            at cursor 1350
    int carDataAt1350;      // MOT[Car].Tray.Data[0][0]     at cursor 1350
    int carCntAt1500;       // iAuto_CarCarDeviceCnt[P]     at cursor 1500
    int zSelOffAt1356;      // C_AutoZ_Select bCylinderOn   at cursor 1356
    CycleObs() : lockedAt850(-1), releasedAt960(-1), carHasTrayAt1350(-1),
                 carDataAt1350(-1), carCntAt1500(-1), zSelOffAt1356(-1) {}
};

static void runCycle(int P, int ticks, CycleObs &o)
{
    trajPush(o.traj, BinTrayTask[P]);
    for (int i = 0; i < ticks; ++i)
    {
        advanceAuto(P);
        DoAutoReceiveBinTray(P);
        const int c = BinTrayTask[P];
        trajPush(o.traj, c);
        if (c == 850 && o.lockedAt850 < 0)
            o.lockedAt850 = (Cylinder[C_AutoSide_Fixer[P]].bCylinderOn &&
                             Cylinder[C_AutoEdgePush[P]].bCylinderOn   &&
                             Cylinder[C_AutoUpPress[P]].bCylinderOn) ? 1 : 0;
        if (c == 960 && o.releasedAt960 < 0)
            o.releasedAt960 = (!Cylinder[C_AutoSide_Fixer[P]].bCylinderOn &&
                               !Cylinder[C_AutoEdgePush[P]].bCylinderOn   &&
                               !Cylinder[C_AutoUpPress[P]].bCylinderOn) ? 1 : 0;
        if (c == 1350 && o.carHasTrayAt1350 < 0)
        {
            o.carHasTrayAt1350 = MOT[iMMAuto_Car[P]].fHasTray ? 1 : 0;
            o.carDataAt1350    = MOT[iMMAuto_Car[P]].Tray.Data[0][0];
        }
        if (c == 1356 && o.zSelOffAt1356 < 0)
            o.zSelOffAt1356 = Cylinder[C_AutoZ_Select[P]].bCylinderOn ? 1 : 0;
        if (c == 1500 && o.carCntAt1500 < 0)
            o.carCntAt1500 = iAuto_CarCarDeviceCnt[P];
    }
}

// ===========================================================================
//  [5] DoAutoReceiveBinTray -- the non-ART DUMMY receive/eject cycle
// ===========================================================================
static CycleObs g_nonArt;

static void test_cycle_non_art()
{
    printf("\n[5] DoAutoReceiveBinTray -- non-ART DUMMY cycle (bARTUnloaderUseTwoCylin==false)\n");

    const int P = 0;
    setupAutoFixture(P);
    giveFullTray(iMMAuto[P]);
    giveNoTray(iMMAuto_Car[P]);
    bHas1DCoverTray[P] = true;              // pre-drive opposite of case 850's write
    bNeedCoverTray[P]  = true;
    Initial_Auto_BinTray_Task(P);

    runCycle(P, 120, g_nonArt);

    static const int want[] = { 1, 100, 200, 500, 800, 850, 900, 950, 960, 970,
                                1000, 1010, 1020, 1021, 1025, 1050, 1100, 1200,
                                1210, 1220, 1350, 1400, 1420, 1500, 300 };
    const bool ok = trajEq(g_nonArt.traj, want, (int)(sizeof(want)/sizeof(want[0])));
    if (!ok) trajPrint("non-ART", g_nonArt.traj, want, (int)(sizeof(want)/sizeof(want[0])));
    CHECK(ok, "EXACT non-ART trajectory 1,100,200,500,800,850,900,950,960,970,1000,1010,1020,1021,1025,1050,1100,1200,1210,1220,1350,1400,1420,1500,300");

    CHECK(g_nonArt.lockedAt850 == 1,
          "case 800 locked the tray: side-fixer+edge-push+up-press all ON at cursor 850");
    CHECK(g_nonArt.releasedAt960 == 1,
          "case 955 released the tray: all three OFF at cursor 960");
    CHECK(g_nonArt.carHasTrayAt1350 == 1,
          "case 1220 handed the tray to the car (Car.fHasTray at cursor 1350)");
    CHECK(g_nonArt.carDataAt1350 == HAS_IC,
          "case 1220 moved the tray CONTENTS onto the car (Data[0][0]==HAS_IC at cursor 1350)");
    CHECK(g_nonArt.carCntAt1500 == 1,
          "case 1420 counted the ejected devices (iAuto_CarCarDeviceCnt==1 at cursor 1500)");
    CHECK(MOT[iMMAuto_Car[P]].fHasTray == false,
          "case 1420 cleared the car tray at cycle end");
    CHECK(bAUTORev[P] == true,  "case 900 raised bAUTORev");
    CHECK(iUnloaderTrayCountCal[P] == 1, "case 850 counted the tray (iUnloaderTrayCountCal 0 -> 1)");
    CHECK(bHas1DCoverTray[P] == false && bNeedCoverTray[P] == false,
          "case 850 cleared both cover-tray flags once the bundle count was met");
    CHECK(iAutoHasHod[P] == 0, "case 1220 wrote iAutoHasHod (-1 sentinel -> 0 HAS_NULL_IC cells)");
    CHECK(W906_ShowErrorMessage_Count == 0,
          "the whole non-ART cycle runs WITHOUT raising any alarm");
}

// ===========================================================================
//  [6] DoAutoReceiveBinTray -- the ART cycle.  Same fixture, one config flip.
// ===========================================================================
static void test_cycle_art()
{
    printf("\n[6] DoAutoReceiveBinTray -- ART cycle (bARTUnloaderUseTwoCylin==true)\n");

    const int P = 0;
    const int savedRetest = USE_AUTO_RETEST;
    const int savedUld    = UNLOADER_ART[P];
    USE_AUTO_RETEST  = eartInstall;
    UNLOADER_ART[P]  = eartInstall;         // + bNoAutoZSelect==false, USE_LdUldCassetteMode!=1

    setupAutoFixture(P);
    // AI(W906-W7-L1-Wave3) 20260802: the ART cycle DOES reach AutoCylinderLower's
    // and AutoCylinderMiddle's ART arms (golden :811 and :1350), and those arms
    // read real position sensors -- with the pair unwired they can never close and
    // the cycle parks on cursor 960 forever (MEASURED before this line was added).
    // Register the pair under the ART sensor law, IN CALL-SITE ORDER: golden :811
    // and :1350 both pass (C_Auto_Up[Pos], C_Auto_Selector[Pos]) on the ART side --
    // note that golden :819, the NON-ART twin of :811, passes them REVERSED.
    // AI(W906-W7-L1-W3fixB) 20260802: that sentence used to end "...which is exactly
    // the asymmetry [13] pins".  [13] IN THIS FILE is DoAutoCassetteTrayFeed and
    // touches no AutoCylinder* parameter order at all.  The asymmetry is pinned by
    // tests/test_w7_l1_auto2.cpp SUB-TEST [12] (`test_argument_order_pin`).
    w3::WireAutoStackART(C_Auto_Up[P], C_Auto_Selector[P], 21 + P);
    // WireOne leaves both cylinders retracted; restore the two pre-drives the
    // fixture relies on for its "OFF at cursor N" observables.
    Cylinder[C_Auto_Selector[P]].Off();
    Cylinder[C_Auto_Up[P]].Off();
    giveFullTray(iMMAuto[P]);
    giveNoTray(iMMAuto_Car[P]);
    Initial_Auto_BinTray_Task(P);

    CycleObs o;
    runCycle(P, 400, o);

    static const int want[] = { 1, 100, 200, 500, 800, 850, 900, 950, 960, 970,
                                1000, 1010, 1020, 1021, 1050, 1100, 1200, 1210,
                                1220, 1350, 1352, 1353, 1354, 1355, 1356, 1357,
                                1358, 1400, 1420, 1500, 300 };
    const bool ok = trajEq(o.traj, want, (int)(sizeof(want)/sizeof(want[0])));
    if (!ok) trajPrint("ART", o.traj, want, (int)(sizeof(want)/sizeof(want[0])));
    CHECK(ok, "EXACT ART trajectory ...,1350,1352,1353,1354,1355,1356,1357,1358,1400,...");

    CHECK(trajHas(o.traj, 1352) && trajHas(o.traj, 1358),
          "ART run entered the 1352..1358 separator chain (case 1350's bARTUnloaderUseTwoCylin arm)");
    CHECK(!trajHas(g_nonArt.traj, 1352),
          "the non-ART run did NOT (same cursor, the OTHER arm of the same predicate)");
    CHECK(trajHas(g_nonArt.traj, 1025) && !trajHas(o.traj, 1025),
          "case 1021's ART split is live too: 1025 in the non-ART run, absent in the ART run");
    CHECK(o.zSelOffAt1356 == 0,
          "case 1355 retracted the Z separator (pre-driven ON -> OFF at cursor 1356)");
    CHECK(o.lockedAt850 == 1 && o.releasedAt960 == 1,
          "the ART run locks at 800 and releases at 955 exactly like the non-ART run");
    CHECK(W906_ShowErrorMessage_Count == 0,
          "the whole ART cycle runs WITHOUT raising any alarm");

    USE_AUTO_RETEST = savedRetest;
    UNLOADER_ART[P] = savedUld;
    // AI(W906-W7-L1-Wave3) 20260802: un-register the ART pair so later sub-tests
    // see the default (unwired) machine again.
    // AI(W906-W7-L1-W3fixB) 20260802: that sentence was ASPIRATIONAL until now --
    // Forget() only cleared the registration list, so the pair stayed Enable==true
    // with its Sim sensor bits frozen at this walk's last state, and the four
    // later sub-tests that drive the SM ([8]..[11], [13]) ran on it by accident.
    // Forget() now really un-wires (w3_cylinder_plant.h, Unwire), so the sentence
    // is true as written.
    w3::Forget();
}

// ===========================================================================
//  [7] The case 950 -> case 955 GOLDEN FALL-THROUGH
// ===========================================================================
static void test_fallthrough_950_955()
{
    printf("\n[7] golden fall-through case 950 -> case 955\n");

    CHECK(trajHas(g_nonArt.traj, 950) && !trajHas(g_nonArt.traj, 955),
          "case 950 FALLS THROUGH into case 955 (955 never observed as a resting cursor)");
    CHECK(trajHas(g_nonArt.traj, 960),
          "...and the same tick that runs 955's body lands the cursor on 960");
}

// ===========================================================================
//  [8] JAM1109 -- the K_RETRY and K_SKIP arms of case 750, both proven
// ===========================================================================
static void test_jam1109_retry_and_skip()
{
    printf("\n[8] case 500 -> 750 JAM1109: K_RETRY re-locks, K_SKIP clears\n");

    const int P = 0;
    const int savedX = Prod.iXTrayAuto[0];
    Prod.iXTrayAuto[0] = 100000;            // clear golden :392's tray-arm break

    // --- K_RETRY (the seam's reset value) ---------------------------------
    setupAutoFixture(P);
    LastSet.iRealDummy = REALLY;            // case 500 needs a NON-dummy run ...
    simSensorOff(SnAutoTrayDetect[P]);      // ... and the tray sensor genuinely OFF
    giveFullTray(iMMAuto[P]);
    giveNoTray(iMMAuto_Car[P]);
    BinTrayTask[P] = 500;                   // enter at 500 (see NOT-COVERED (i))

    advanceAuto(P); DoAutoReceiveBinTray(P);            // 500 -> 750
    CHECK(BinTrayTask[P] == 750, "case 500 with the tray sensor OFF hands off to 750");
    advanceAuto(P); DoAutoReceiveBinTray(P);            // 750, answer K_RETRY
    CHECK(W906_ShowErrorMessage_LastCode == sJAM1109[P],
          "case 750 raised JAM1109");
    CHECK(W906_ShowErrorMessage_LastKCode == (K_SKIP | K_RETRY),
          "case 750 offered BOTH Skip and Retry");
    CHECK(Cylinder[C_AutoSide_Fixer[P]].bCylinderOn == true &&
          Cylinder[C_AutoEdgePush[P]].bCylinderOn   == true &&
          Cylinder[C_AutoUpPress[P]].bCylinderOn    == true,
          "JAM1109 K_RETRY re-locked the tray (all three ON again)");
    CHECK(BinTrayTask[P] == 500, "JAM1109 K_RETRY sends the cursor back to 500");
    CHECK(MOT[iMMAuto[P]].fHasTray == true, "JAM1109 K_RETRY did NOT discard the tray");

    // --- K_SKIP -----------------------------------------------------------
    BinTrayTask[P] = 750;
    W906_ShowErrorMessage_SimReturn = K_SKIP;
    advanceAuto(P); DoAutoReceiveBinTray(P);
    CHECK(MOT[iMMAuto[P]].fHasTray == false, "JAM1109 K_SKIP cleared the Auto tray");
    CHECK(BinTrayTask[P] == 100, "JAM1109 K_SKIP sends the cursor to 100");
    CHECK(W906_ShowErrorMessage_Count == 2, "exactly 2 JAM1109 dialogs in this sub-test");

    Prod.iXTrayAuto[0] = savedX;
    LastSet.iRealDummy = DUMMY;
    simSensorUnknown(SnAutoTrayDetect[P]);
    BinTrayTask[P] = 1;
}

// ===========================================================================
//  [9] DoAutoTrackDetectICFloating (golden :2267-:2287)
// ===========================================================================
static void test_track_detect_floating()
{
    printf("\n[9] DoAutoTrackDetectICFloating -- both exits + the case 1 fall-through\n");

    const int P = 2;
    setupAutoFixture(P);
    InitAutoTrackDetectICFloatingTask(P);
    simSensorUnknown(SnAutoTrackDetect[P]);

    const bool r0 = DoAutoTrackDetectICFloating(P);
    CHECK(r0 == false, "sensor unknown -> returns false (no IC floating)");
    CHECK(iAutoTrackDetectICFloatingTask[P] == 100,
          "case 1 advances the cursor to 100");

    const bool r1 = DoAutoTrackDetectICFloating(P);
    CHECK(r1 == false && iAutoTrackDetectICFloatingTask[P] == 100,
          "it stays parked on 100 while the sensor stays quiet");

    simSensorOn(SnAutoTrackDetect[P]);
    const bool r2 = DoAutoTrackDetectICFloating(P);
    CHECK(r2 == true, "sensor ON -> returns true (IC floating detected)");
    CHECK(iAutoTrackDetectICFloatingTask[P] == 1, "sensor ON -> cursor reset to 1");

    // THE FALL-THROUGH PIN.  From a FRESH cursor 1 with the sensor already ON,
    // golden's `case 1: ... Task=100;` falls into `case 100:` in the SAME call,
    // so the very first call must answer true.  An explicit `break;` after
    // `Task=100;` would make it answer false and only detect on the second call
    // -- which is exactly what mutation M14 does, and it goes red here.
    InitAutoTrackDetectICFloatingTask(P);
    CHECK(DoAutoTrackDetectICFloating(P) == true,
          "case 1 FALLS THROUGH into case 100: the FIRST call from a fresh cursor already detects");

    simSensorUnknown(SnAutoTrackDetect[P]);
    InitAutoTrackDetectICFloatingTask(P);
}

// ===========================================================================
//  [10] The pre-switch bAutoTrackCanGoRear gate (golden :115-:127) -> case 920
// ===========================================================================
static void test_track_gate_920()
{
    printf("\n[10] bAutoTrackCanGoRear gate -> case 920 -> JAM1110\n");

    const int P = 0;
    setupAutoFixture(P);
    InitAutoTrackDetectICFloatingTask(P);
    InitAuto123TrayToRearTask(P);
    giveNoTray(iMMAuto[P]);
    giveNoTray(iMMAuto_Car[P]);
    Initial_Auto_BinTray_Task(P);

    bAutoTrackCanGoRear = true;
    simSwitchWire(iSwAutoOut[P], 6);
    simSwitchWire(iSwAutoIn[P],  7);
    SW[iSwAutoOut[P]].On();                 // belt running out ...
    SW[iSwAutoIn[P]].Off();                 // ... and not in
    simSensorOn(SnAutoTrackDetect[P]);      // an IC is riding under the tray

    advanceAuto(P);
    DoAutoReceiveBinTray(P);
    CHECK(BinTrayTask[P] == 920, "track-detect gate parks the SM on case 920");
    CHECK(bAtuoTrayICDetectErr == true, "track-detect gate latches bAtuoTrayICDetectErr");

    // The tray now backs out and the sensor clears, so the gate stops re-firing
    // and case 920 can run its nested DoAuto123TrayToRear to completion.
    simSensorUnknown(SnAutoTrackDetect[P]);
    for (int i = 0; i < 40 && W906_ShowErrorMessage_Count == 0; ++i)
    {
        advanceAuto(P);
        advanceRear(P);
        DoAutoReceiveBinTray(P);
    }
    CHECK(W906_ShowErrorMessage_LastCode == sJAM1110[P], "case 920 raised JAM1110");
    CHECK(W906_ShowErrorMessage_Count == 1, "case 920 raised it exactly once");
    CHECK(BinTrayTask[P] == 950 || BinTrayTask[P] == 960 || BinTrayTask[P] == 970,
          "case 920 hands off into the 950 release chain after the alarm");

    bAutoTrackCanGoRear = false;
    SW[iSwAutoOut[P]].Enable = false;
    SW[iSwAutoIn[P]].Enable  = false;
    bAtuoTrayICDetectErr = false;
    BinTrayTask[P] = 1;
}

// ===========================================================================
//  [11] DoAuto123TrayToRear (golden :2300-:2436)
// ===========================================================================
static void test_auto123_tray_to_rear()
{
    printf("\n[11] DoAuto123TrayToRear -- exact trajectory + fixer states + double-tray exit\n");

    const int P = 1;
    setupAutoFixture(P);
    giveNoTray(iMMAuto[P]);
    giveNoTray(iMMAuto_Car[P]);
    InitAuto123TrayToRearTask(P);

    // pre-drive OPPOSITE of what case 1 / case 100 command
    Cylinder[C_AutoEdgePush[P]].On();
    Cylinder[C_AutoUpPress[P]].On();
    Cylinder[C_AutoSide_Fixer[P]].On();

    Traj t;
    trajPush(t, iAuto123TrayToRearTask[P]);
    bool done = false;
    int fixerAt600 = -1, edgeAfterFirstTick = -1, pressAfterFirstTick = -1;
    for (int i = 0; i < 60 && !done; ++i)
    {
        advanceRear(P);
        done = DoAuto123TrayToRear(P);
        trajPush(t, iAuto123TrayToRearTask[P]);
        if (i == 0)
        {
            edgeAfterFirstTick  = Cylinder[C_AutoEdgePush[P]].bCylinderOn ? 1 : 0;
            pressAfterFirstTick = Cylinder[C_AutoUpPress[P]].bCylinderOn  ? 1 : 0;
        }
        if (iAuto123TrayToRearTask[P] == 600 && fixerAt600 < 0)
            fixerAt600 = Cylinder[C_AutoSide_Fixer[P]].bCylinderOn ? 1 : 0;
    }

    static const int want[] = { 1, 200, 450, 460, 500, 600, 1 };
    const bool ok = trajEq(t, want, (int)(sizeof(want)/sizeof(want[0])));
    if (!ok) trajPrint("rear", t, want, (int)(sizeof(want)/sizeof(want[0])));
    CHECK(ok, "EXACT rear trajectory 1,200,450,460,500,600,1 (100 is a fall-through, never a resting cursor)");
    CHECK(done == true, "DoAuto123TrayToRear reports the tray reached the rear");
    CHECK(edgeAfterFirstTick == 0 && pressAfterFirstTick == 0,
          "case 1 released edge-push and up-press on the very first tick");
    CHECK(fixerAt600 == 1, "case 500 extended the side fixer (ON at cursor 600)");
    CHECK(Cylinder[C_AutoSide_Fixer[P]].bCylinderOn == false,
          "case 600 retracted the side fixer before returning");
    CHECK(W906_ShowErrorMessage_Count == 0, "the rear walk raises no alarm");

    // --- the case-200 double-tray early return ----------------------------
    setupAutoFixture(P);
    InitAuto123TrayToRearTask(P);
    giveFullTray(iMMAuto[P]);
    giveFullTray(iMMAuto_Car[P]);
    advanceRear(P);
    const bool d0 = DoAuto123TrayToRear(P);      // case 1 -> 100 -> 200
    advanceRear(P);
    const bool d1 = DoAuto123TrayToRear(P);      // case 200 sees BOTH trays
    CHECK(d0 == false && d1 == true && iAuto123TrayToRearTask[P] == 1,
          "both trays present -> immediate true + cursor reset to 1");
    giveNoTray(iMMAuto[P]);
    giveNoTray(iMMAuto_Car[P]);
}

// ===========================================================================
//  [12] CheckHasErrorBinOnTray (golden :2438-:2479)
// ===========================================================================
static void test_check_has_error_bin()
{
    printf("\n[12] CheckHasErrorBinOnTray -- P54 gate + name resolution\n");

    const bool saved = IniConfig.P54UnloaderTaryCheckHasErrorBinIC;

    setupAutoFixture(0);
    IniConfig.P54UnloaderTaryCheckHasErrorBinIC = false;
    // Use the UNRECOGNISED motor for the OFF probe on purpose: it is the only
    // input that makes the body speak at all offline (HowManyBinICInTray is a
    // stub returning 0, see register entry I), so probing with a recognised
    // motor would have made this assertion vacuous.  Measured: mutation M23
    // (guard removed) stayed GREEN against the earlier iMMAuto[0] probe.
    CheckHasErrorBinOnTray(MTrayX);
    CHECK(W906_ShowMyMessage_Count == 0, "P54 OFF -> completely silent, even for an unrecognised motor");

    IniConfig.P54UnloaderTaryCheckHasErrorBinIC = true;
    W906_ShowMyMessage_Reset();
    CheckHasErrorBinOnTray(iMMAuto[0]);
    CHECK(W906_ShowMyMessage_Count == 0,
          "P54 ON + a recognised Auto motor with no error bin -> still silent");

    W906_ShowMyMessage_Reset();
    CheckHasErrorBinOnTray(iMMAuto_Car[1]);
    CHECK(W906_ShowMyMessage_Count == 0,
          "P54 ON + a recognised Auto-CAR motor with no error bin -> still silent");

    W906_ShowMyMessage_Reset();
    CheckHasErrorBinOnTray(MTrayX);
    CHECK(W906_ShowMyMessage_Count == 1 &&
          W906_ShowMyMessage_LastS1 == AnsiString("Unloader not support [P54] Function"),
          "P54 ON + a motor that is neither iMMAuto[] nor iMMAuto_Car[] -> \"not support\"");

    IniConfig.P54UnloaderTaryCheckHasErrorBinIC = saved;
}

// ===========================================================================
//  [13] DoAutoCassetteTrayFeed (golden :2488-:2560)
// ===========================================================================
static void test_cassette_tray_feed()
{
    printf("\n[13] DoAutoCassetteTrayFeed -- bound guard + full push cycle\n");

    const int P = 0;
    setupAutoFixture(P);
    Cylinder[C_AutoCarrier[P]].Enable = true;
    Cylinder[C_AutoCarrier[P]].Off();       // pre-drive opposite of case 200's Push()

    // The iPos>1 bound (golden :2492).  Returning false is NOT discriminating --
    // MEASURED: with the guard removed the call still returns false, because
    // case 1's DUMMY arm answers false too.  What IS discriminating is that the
    // guard returns BEFORE case 1 arms hAutoCassetteTrayFeed: leave the timer
    // elapsed, call, and it must still be elapsed.
    hAutoCassetteTrayFeed.SetMSAndOn(0);
    CHECK(DoAutoCassetteTrayFeed(2) == false,
          "iPos==2 is rejected by the [2]-element bound (returns false)");
    CHECK(hAutoCassetteTrayFeed.Off() == true,
          "...and it returns BEFORE case 1 arms the 2 s feed timer (nothing was started)");

    InitAutoCassetteTrayFeedTask(P);
    bool done = false, sawUp = false;
    int  doneTick = -1, offTickAfterUp = -1;
    for (int i = 0; i < 60 && !done; ++i)
    {
        hAutoCassetteTrayFeed.SetMSAndOn(0);   // the 2 s / 15 s progress arm
        done = DoAutoCassetteTrayFeed(P);
        if (done) doneTick = i;
        const bool on = Cylinder[C_AutoCarrier[P]].bCylinderOn;
        if (on) sawUp = true;
        else if (sawUp && offTickAfterUp < 0) offTickAfterUp = i;
    }
    CHECK(done == true, "DoAutoCassetteTrayFeed completes the push cycle");
    CHECK(sawUp && offTickAfterUp >= 0,
          "carrier cylinder went UP then back DOWN (ordered, not set membership)");
    // case 400 lowers it (golden :2542) and case 500 only CONFIRMS with Pop()
    // (golden :2547).  Without this strict-less-than the two sites mask each
    // other -- MEASURED: mutation M25 (case 400's Off deleted) stayed GREEN
    // against the plain "went down afterwards" form.
    CHECK(offTickAfterUp >= 0 && doneTick >= 0 && offTickAfterUp < doneTick,
          "case 400 lowered the carrier BEFORE the completion tick (not case 500's Pop doing it)");
    CHECK(W906_ShowErrorMessage_Count == 0, "the cassette feed raises no alarm");

    // Init must genuinely rewind the cursor: case 500 returns true WITHOUT
    // resetting Task (golden :2551), so a second call would answer instantly.
    CHECK(DoAutoCassetteTrayFeed(P) == true,
          "without Init the finished cursor answers true immediately (golden leaves Task==500)");
    InitAutoCassetteTrayFeedTask(P);
    CHECK(DoAutoCassetteTrayFeed(P) == false,
          "InitAutoCassetteTrayFeedTask rewinds it (the next call has to walk again)");
}

// ===========================================================================
//  [14] NOT-COVERED REGISTER SELF-CHECK
//       The register above claims the suite enters the alarm surface exactly
//       three times and the operator-dialog surface exactly once.  Assert it.
// ===========================================================================
static void test_register_selfcheck()
{
    printf("\n[14] NOT-COVERED register self-check\n");

    const int alarms  = g_alarmsBanked  + W906_ShowErrorMessage_Count;
    const int dialogs = g_dialogsBanked + W906_ShowMyMessage_Count;
    printf("    (whole-suite ShowErrorMessage=%d  ShowMyMessage=%d)\n", alarms, dialogs);

    CHECK(alarms == 3,
          "EXACT alarm ledger: 3 ShowErrorMessage calls in the whole suite (2 JAM1109 in [8], 1 JAM1110 in [10])");
    CHECK(dialogs == 1,
          "EXACT dialog ledger: 1 ShowMyMessage call in the whole suite (the [P54] not-support line in [12])");
    CHECK(g_nonArt.traj.size() > 0 &&
          !trajHas(g_nonArt.traj, 1415) && !trajHas(g_nonArt.traj, 1650) &&
          !trajHas(g_nonArt.traj, 1030) && !trajHas(g_nonArt.traj, 1022),
          "register entries A/C/D hold: cases 1415, 1650, 1030 and 1022 are NOT visited by the driven cycle");
}

static void W906_ContainUnloaderInfoRoot();   // =========== AI(W906-FLOW-2) 20260928: forward declaration; the body is at the END of this file (line numbers stay put)
int main()
{   W906_ContainUnloaderInfoRoot();   //AI(W906-FLOW-2) 20260928: second containment layer for SaveUnloaderInfo (cinitial.cpp:5535), before any sub-test can reach it
    printf("=== W7-L1 Wave 2: asendic_Auto.cpp ===\n");

    test_inits();
    test_seams();
    test_pos_guard();
    test_rotate_kit_early_return();
    test_cycle_non_art();
    test_cycle_art();
    test_fallthrough_950_955();
    test_jam1109_retry_and_skip();
    test_track_detect_floating();
    test_track_gate_920();
    test_auto123_tray_to_rear();
    test_check_has_error_bin();
    test_cassette_tray_feed();
    test_register_selfcheck();

    printf("\nRESULT: %d passed, %d failed\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}

// ===========================================================================
//AI(W906-FLOW-2) 20260928: second containment layer for SaveUnloaderInfo.  Since 5d1694a4 the driven Auto cycle reaches it
//  (asendic_Auto.cpp:1465, golden :1103), and it writes <W906_UNLOADERINFO_ROOT>\LastTrayInfo.txt -- unset, that is the
//  machine's D:\UnloaderInfo\LastTrayInfo.txt, which the next wb_serve boot reads back (cinitial.cpp:9519-9549).  ctest
//  sets the variable (tests/CMakeLists.txt:4064 and the per-test overrides); this covers a direct run of the exe (gdb,
//  F5, by hand), like test_mt_e3b_engine guards machinerecord.dat.  Same CRT-putenv idiom as tests/test_agv_e84.cpp:158-178
//  (it MUST stay a CRT call: getenv() reads the CRT copy of the environment, not the process block).
// ===========================================================================
#include <cstdlib>
#if defined(__MINGW32__) && !defined(__MINGW64_VERSION_MAJOR)
extern "C" int _putenv(const char*);
#define HT9045_TEST_PUTENV _putenv
#else
#define HT9045_TEST_PUTENV putenv
#endif
static void W906_ContainUnloaderInfoRoot()
{
    const char* cur = getenv("W906_UNLOADERINFO_ROOT");
    if (cur != 0 && *cur != '\0')
        return;                                          // ctest (or the caller) already contained it
    const char* tmp = getenv("TEMP");
    if (tmp == 0) tmp = getenv("TMP");
    if (tmp == 0) tmp = ".";
    static char buf[600];
    snprintf(buf, sizeof(buf), "W906_UNLOADERINFO_ROOT=%s\\ht9045_w7l1auto_unloader", tmp);
    HT9045_TEST_PUTENV(buf);
    printf("[containment] W906_UNLOADERINFO_ROOT was unset -> %s\n", getenv("W906_UNLOADERINFO_ROOT"));
}
