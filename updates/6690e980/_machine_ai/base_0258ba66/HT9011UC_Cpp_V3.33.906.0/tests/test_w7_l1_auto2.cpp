// =============================================================================
//  test_w7_l1_auto2.cpp  --  W7-L1 asendic_Auto2 (Auto2-tray stack-feed) SM test
//
//  Translation wave: W906-W7-L1
//  Author: AI(W906-W7-L1) 20260729
//  AI(W906-W7-L1fix) 20260729: REWRITTEN.  The original version of this file
//  asserted `cleared || steps == 1000` / `done || cursor != 1 || steps == 500`
//  -- both TAUTOLOGIES (the loop can only exit by setting the first disjunct or
//  by exhausting the bound, so the assertion was true by construction).  It
//  therefore reported PASS while DoUnLoadNewAuto2ToStack (golden
//  asendic_Auto2.cpp:600-706) had ZERO coverage and DoLoadNewAuto2TrayToCar
//  converged on step 0 through a short-circuit that never touched the
//  AutoCylinder* helpers the file header claimed it exercised end-to-end.
//  Every assertion below now constrains a specific observable of the state
//  machine and has been PROVEN capable of failing (see FAILABILITY PROOF).
//
//  AI(W906-W7-L1fix3) 20260801: THREE COVERAGE HOLES CLOSED + TWO OVERSTATED
//  CLAIMS CORRECTED.  The trajectory assertions were load-bearing but read ONLY
//  the Task cursors, so the whole actuator surface was unconstrained: inverting
//  the conveyor commands, swapping a cylinder Push()/Pop(), or deleting every
//  MOT[MMAuto2Z].ClearTray() each left the suite at 46 passed, 0 failed.  All
//  three are now observed, each with its own red-then-green mutation below.  The
//  two corrected claims are the over-broad "ANY single Task=N transition"
//  sentence (see PROOF OBJECTIVE) and the FAILABILITY PROOF banner, which pasted
//  `(line N)` suffixes the shipped file could not print.
//
//  PROOF OBJECTIVE
//  ---------------
//  Drive the Auto2-tray SM family over the Sim HAL (Cylinder[]/MOT[]/Sen[]/SW[])
//  with no hardware, and assert each SM walks the SPECIFIC cursor trajectory the
//  golden prescribes -- not merely "it did not crash" -- AND that it leaves the
//  actuators (SW[] belt outputs, Cylinder[] extend/retract, MOT[] tray flags) in
//  the state golden prescribes at the point golden prescribes it.
//
//  AI(W906-W7-L1fix3) 20260801: THE TRAJECTORY CLAIM HERE WAS OVER-BROAD AND IS
//  NOW SCOPED.  It used to read "deleting or re-targeting ANY single `Task=N`
//  transition drops N (and everything downstream of it) out of the observed set
//  and the test goes red" -- which the file's own NOT-COVERED register already
//  contradicted.  Both ends of the range were re-run this session against THIS
//  file (see mutations S1/S2 in the FAILABILITY PROOF below):
//    * ON A DRIVEN PATH the claim holds.  Re-targeting DoAuto2 case 30's
//      `Task=40;` (golden :469) to `Task=999;` took the suite to 52 passed,
//      8 failed.
//    * ON A DEAD ARM it cannot hold.  Re-targeting the `Task=50;` in DoAuto2
//      case 40's ELSE arm (golden :490) to `Task=999;` left the suite at
//      60 passed, 0 failed -- that arm is never entered under this fixture
//      (Sen[SnAuto2FixCyPush] reads "unknown", so `IsOff()==false` is true and
//      the then-arm at golden :472-478 always wins; see NOT COVERED below).
//      Both mutations were also run against the PREVIOUS 46-assertion revision
//      of this file, which answered 38/8 and 46/0 -- same conclusion, so the
//      over-broad sentence was wrong when it was written, not only now.
//  SCOPED FORM: a trajectory assertion is load-bearing for `Task=N` transitions
//  ON THE DRIVEN PATHS enumerated under WHAT IT DRIVES.  It says nothing about
//  the dead arms the NOT-COVERED register enumerates, and is not claimed to.
//
//  TWO TEST SEAMS THIS FILE INTRODUCES (both fixture-only; zero production change)
//  ------------------------------------------------------------------------------
//  1. VIRTUAL TIME.  TQPF_Timer (myTimer.h) calls QueryPerformanceCounter
//     directly and has NO injection point -- unlike MyTickCount (common.cpp:1907,
//     which atester.cpp:1519-1520 shadows with a TU-local stand-in + `#define`)
//     there is no seam to override, and unlike uHGemEquipment's TimerSetMSAndOn
//     there is no <=0 shortcut inside the class.  So instead of overriding the
//     clock we RE-ARM the pending timer with a 0 ms deadline before each SM tick:
//     SetMS(0) leaves rSetTime==0 and On() then sets rEnd = rStart - 2*overhead
//     (myTimer.cpp:32-38), so the very next Off() is true (myTimer.cpp:40-44,
//     via SetMSAndOn myTimer.cpp:94-98).  The Auto2 timers are non-static
//     file-scope objects in asendic_Auto2.cpp, so the extern declarations below
//     reach them without touching asendic_Auto2.h.
//
//     AI(W906-W7-L1fix2) 20260731: THE WATCHDOG RATIONALE HERE WAS WRONG AND IS
//     CORRECTED.  It used to claim the two 20 s watchdogs (hAuto2TrayToFront /
//     hAuto2TrayToRear, golden :176 / :313) were "deliberately left RUNNING so
//     that a stalled transition surfaces as a JAM alarm in the trace".  It cannot:
//     those deadlines are real QPC wall-clock, and every loop in this file is
//     bounded in VIRTUAL ticks (200-400 iterations of pure computation, i.e. a
//     few ms of wall clock), so a 20 s deadline can never elapse inside the bound.
//     A stall therefore surfaces as CHECK(done) / CHECK(reached*) FAILING on a
//     bound-exhausted loop -- never as a JAM.  Measured this session: an
//     instrumented run of the whole file emitted ZERO `[ShowErrorMessage]` and
//     ZERO `[ShowMyMessage]` lines, so no alarm arm is entered at all.
//     One nuance worth stating, because it is why [5] converges without the test
//     touching that timer: hAuto2TrayToRear is not purely a watchdog.
//     DoAuto2TrayToRear case 450 RE-ARMS it as the tray-arrival settle timer
//     (golden :382, SetSecAndOn(Ld_UldDelayTime.LD_TrayArrivalDely), pinned to 0
//     by setupDummyFeed below), and case 460 waits on that (golden :386).
//     hAuto2TrayToFront has no such re-arm -- it is armed once at golden :191 and
//     read only by the JAM1212 guard at golden :245, which never fires here.
//  2. SIM SENSOR LEVELS.  Sen[] defaults to Enable==false, which makes IsOn() AND
//     IsOff() both return false (mysensor.cpp:127-131 / :177-181) -- an "unknown"
//     reading that is NOT the same as "off", and which is exactly what let the old
//     [3] short-circuit out of DoLoadNewAuto2TrayToCar on step 0.  To read a
//     deterministic level with no IO plumbing: Enable=true + ISABase=eISABase
//     routes to mysensor.cpp:29's raw-port stub (always false), and Type then
//     picks the polarity -- TYPE_A(1) => IsOff()==true / IsOn()==false ("off"),
//     TYPE_B(0) => IsOn()==true / IsOff()==false ("on").
//
//  WHAT IT DRIVES
//    * All 6 Init* -- each resets its own cursor int to 1.
//    * DoAuto2() (top dispatcher, golden :414-592) over FOUR scenarios, because a
//      single scenario cannot reach the whole switch: A the feed walk
//      (1,2,3,20,30,40,50,60,70,100,150), B the case-100 -> 200 feed handoff,
//      C the case-100 -> 300 recycle handoff, D the case-60 -> 65 tray re-check
//      and its DoAuto2Receive hand-off (iReceiveAuto2Tray=2 / iAuto2ReceiveTask=200).
//    * DoLoadNewAuto2TrayToCar() (golden :38-173) BOTH ways: the full lifter walk
//      50,100,200,201,300,400,410,420 with the stack sensors reading "off", AND
//      the case-1 short-circuit (sensors "unknown") that returns true on step 0.
//    * DoAuto2TrayToFront() (golden :183-310) long path 101,102,103,104,105.
//    * DoAuto2TrayToRear() (golden :321-404) path 100,200,450,460,500.
//    * DoUnLoadNewAuto2ToStack() (golden :600-706) path 50,100,150,200,300,310,
//      400,401,402 -- driven directly AND through DoAuto2Receive().
//    * DoAuto2Receive() (golden :715-749) full chain to the unload hand-off.
//    * The fAuto2CanSupplyNewTray gate: closed -> DoAuto2() is a no-op.
//
//  ACTUATOR STATE THAT IS ASSERTED (AI(W906-W7-L1fix3) 20260801: NEW)
//  -----------------------------------------------------------------
//  Until this revision no executable statement in this file touched SW[] or
//  Cylinder[] at all, and MOT[MMAuto2Z] was only ever CLEARED by the fixture (in
//  [3a] and [3b]) and never read -- so three whole classes of translation slip
//  were invisible.  Each bullet below names the golden site, the observable, and
//  the sub-test that reads it:
//    * BELT OUTPUT POLARITY (SW[].OutValue -- TMySwitch::On() and TMySwitch::Off()
//      each assign OutValue UNCONDITIONALLY, as their first statement, before the
//      `if(Enable==false) return;` guard, so it is a faithful record of the last
//      call even with Enable==false; TMySwitch::Status() is NOT usable here, it
//      returns false whenever Enable==false):
//        golden :215-216  DoAuto2TrayToFront case 100  belt ON  + CW ON   -> [4b]
//        golden :346-347  DoAuto2TrayToRear  case 200  belt ON  + CW OFF  -> [5]
//        golden :389      DoAuto2TrayToRear  case 460  belt OFF on arrival-> [5]
//        golden :622-623  DoUnLoadNewAuto2ToStack case 50 both OFF        -> [6]
//    * CYLINDER ACTUATION DIRECTION (Cylinder[].bCylinderOn -- TMyCylinder::On()
//      and the Push() success exit set it true, TMyCylinder::Off() and the Pop()
//      success exit set it false; offline (OnSenEnable/OffSenEnable both false,
//      OnDelayTime/OffDelayTime both 0) BOTH Push() and Pop() return true, so the
//      RETURN VALUE cannot distinguish them and only the state can):
//        golden :270  case 101 edge-push EXTEND (.On())    -> [4]
//        golden :281  case 102 side-fixer PUSH  (.Push())  -> [4]
//        golden :291  case 103 side-fixer release (.Off()) -> [4]
//        golden :298  case 104 edge-push retract (.Off())  -> [4]
//    * Z-STAGE TRAY BOOKKEEPING (MOT[MMAuto2Z].fHasTray):
//        golden :60   DoLoadNewAuto2TrayToCar case 1 short-circuit -> [3b]
//        golden :87   DoLoadNewAuto2TrayToCar case 50              -> [3a]
//        golden :154  DoLoadNewAuto2TrayToCar case 420 success     -> [3a]
//      ([3a] re-arms the flag between the two so each clear is independently
//      load-bearing.  The fourth golden site, :165, is in the JAM1213 K_SKIP arm
//      and is dead here -- see NOT COVERED.)
//
//  AutoCylinderUp/Middle/Lower ARE NOT REAL BODIES IN THIS TREE.  The original
//  header claimed this test exercised them "end-to-end"; it did not, and there is
//  nothing to exercise: golden defines them in asendic.cpp:562/767/937 (declared
//  golden asendic.h:19-21) and asendic.cpp's Auto* lifter SMs are NOT translated,
//  so acatchtray_shims.cpp holds no-op `return true` stubs.  What the trajectory
//  assertions below prove is exactly this much and no more: every AutoCylinder*
//  CALL SITE is reached and its true-return is consumed by the caller (call sites
//  live in DoLoadNewAuto2TrayToCar cases 1/50/200/201/400/410 and
//  DoUnLoadNewAuto2ToStack cases 1/50/150/200/400/401 -- all on the asserted
//  trajectories).  It proves nothing about real lifter behaviour.
//
//  NOT COVERED (stated so nobody reads coverage into silence)
//  AI(W906-W7-L1fix2) 20260731: this list was too short and its silence read as
//  coverage.  Re-derived from the fixture state + golden and, where reachability
//  was dynamic rather than a logical consequence of a pinned constant, MEASURED
//  with temporary printf probes in the production source (probes removed; the
//  file was restored byte-exactly and its sha256 re-verified at the time).
//  AI(W906-W7-L1fix3) 20260801: PROVENANCE.  Every entry below that says
//  "MEASURED" or "instrumented run" is a 20260731 (L1fix2) measurement; this
//  revision did NOT re-run those probes.  What it DID do is re-derive, entry by
//  entry, whether its own fixture changes ([4b]'s SnAuto2TrayDetect exception and
//  [3a]/[3b]/[5]/[6]'s new pre-drives) could have invalidated any of them; where
//  the reason an arm is dead has changed, the entry now says so explicitly, and
//  the new residual gaps this revision leaves are enumerated in their own section
//  after this list.
//
//  Four fixture invariants do most of the pruning:
//    (i)   NO SENSOR IS EVER DRIVEN "ON".  The only two seam helpers are
//          simSensorOff (Enable+TYPE_A -> IsOff()==true, IsOn()==false) and
//          simSensorUnknown (Enable=false -> BOTH false).  There is no
//          simSensorOn, so `Sen[x].IsOn()` is false for every x at every tick.
//    (ii)  Only SnAuto2PreDete and SnAuto2_Tray_Car are raised to "off".
//          SnAuto2CWPreDetect / SnAuto2FixCyPush / SnAuto2TrayHasTray /
//          SnAutoUpSafedetect1 stay "unknown" throughout, so their IsOff() is
//          false too.
//          AI(W906-W7-L1fix3) 20260801: (ii) HAS EXACTLY ONE EXCEPTION and it is
//          no longer silent.  Sub-test [4b] raises SnAuto2TrayDetect to "off" for
//          the single tick it needs to park DoAuto2TrayToFront inside case 100
//          with the belt running; it restores "unknown" before returning, and
//          every other sub-test re-runs setupDummyFeed() first, so no NOT-COVERED
//          entry below that leans on (ii) is affected.  Where [4b] changes what
//          an arm's deadness FOLLOWS FROM, the entry says so.
//    (iii) LastSet.iRealDummy == DUMMY (0) always.
//    (iv)  bHandlerPause == false and bEmptyPause == false always.
//
//    * EVERY JAM/MES ALARM ARM -- and the reason is stronger than the old note
//      claimed.  The old note said ShowErrorMessage always answers K_RETRY
//      (canary_support.cpp:44-50) so "the K_SKIP arms are unreachable".  Measured:
//      ShowErrorMessage is never CALLED AT ALL -- an instrumented run of the whole
//      file emitted zero `[ShowErrorMessage]` lines.  Each arm is dead for its own
//      reason, none of which is the K_RETRY answer:
//        - JAM1213, DoLoadNewAuto2TrayToCar case 420 (golden :159-169): the DUMMY
//          disjunct of the preceding `if` (golden :151) always wins, so the
//          `else if` is dead.
//        - JAM1212, DoAuto2TrayToFront case 100 (golden :245-264): guarded by the
//          20 s hAuto2TrayToFront deadline, which cannot elapse (see seam 1).
//          AI(W906-W7-L1fix3) 20260801: [4b] deliberately PARKS the SM in case 100
//          for one tick, so this arm is now evaluated on a path that does not
//          break out early -- it is still dead, and for the same reason: one tick
//          of pure computation cannot burn a 20 s real-QPC deadline.
//        - JAM1212, DoAuto2TrayToFront case 101 (golden :274-278): the then-arm
//          (golden :267-268) always holds by (ii).  AI(W906-W7-L1fix3) 20260801:
//          case 101 is only ever ENTERED by [4] and [7], both of which leave
//          SnAuto2TrayDetect "unknown"; [4b], the one sub-test that raises it to
//          "off", parks in case 100 and never reaches 101.
//        - JAM1201, DoAuto2TrayToRear case 200 (golden :363-378): the DUMMY arm at
//          golden :354-360 breaks out first.
//        - JAM1203, DoAuto2 case 40 (golden :479-485): the DUMMY sub-branch at
//          golden :474-478 wins.
//        - JAM1113, DoUnLoadNewAuto2ToStack case 402 (golden :686-695): needs BOTH
//          stack sensors IsOn(), impossible by (i).
//        - MES1024, DoLoadNewAuto2TrayToCar case 1 (golden :81): the guard at
//          golden :53-54 is always true by (iii), so the whole `else` arm is dead
//          -- INCLUDING the Cylinder[C_Auto2_Up] re-lowering block, golden :69-80.
//        - MES1024, DoUnLoadNewAuto2ToStack case 310 (golden :662-669): the
//          `|| DUMMY` disjunct at golden :656 always wins.
//    * The three bHandlerPause arms, all dead by (iv): the case-420 timer-reset
//      arm (golden :144-147), and the two abort-to-Task-1 arms in
//      DoAuto2TrayToFront case 100 (golden :195-199) and DoAuto2TrayToRear
//      case 200 (golden :340-344).
//    * The bEmptyPause SPLIT-PAUSE arm, DoLoadNewAuto2TrayToCar case 201
//      (golden :108-114), dead by (iv).  Measured: zero `[ShowMyMessage]` lines,
//      so its ShowMyMessage call is never made either.
//    * DoLoadNewAuto2TrayToCar case 1's fast entry `if(MOT[MMAuto2].fHasTray)`
//      -> Task=400 (golden :46-51), which would skip 50/100/200/201/300 entirely.
//      MEASURED with a probe: 0 hits across the whole file -- every sub-test
//      clears MOT[MMAuto2] before driving this SM, so this entry is never taken
//      (not even inside [8] scenario B's case-200 hand-off).
//    * DoAuto2TrayToFront case 100's DUMMY-TIMEOUT success arm (golden :230-243).
//      AI(W906-W7-L1fix3) 20260801: this entry used to say the arm was dead
//      because "the block at golden :218-228 always holds by (ii) and breaks to
//      101 first".  That is now only HALF the reason, because [4b] exists:
//        - in [4] and [7] the hand-off at golden :218-228 does break out first
//          (SnAuto2TrayDetect "unknown" -> `IsOff()==false` is true), exactly as
//          before; and
//        - in [4b] that hand-off is deliberately blocked, but the arm is still
//          unreached because hAuto2TrayToFrontForDummy was armed to 5 s by case 1
//          in the same tick (golden :192) and [4b] runs ONE tick without calling
//          advanceAuto2Time(), so golden :230's `Off()` is false.
//      Consequence: the SW[]/MOT[] writes inside this arm (golden :234-238) are
//      still never executed.
//    * DoAuto2TrayToRear case 200's GENUINE arrival arm
//      `if(Sen[SnAuto2TrayDetect].IsOn())` (golden :348-352), dead by (i) -- case
//      200 always leaves via the DUMMY-timeout arm at golden :354-360 instead.
//    * DoAuto2 case 3's whole tail: the `else { MOT[MMAuto2].ClearTray(); }`
//      (golden :445-448) AND the `if(MOT[MMAuto2].fHasTray==false)` block
//      (golden :450-460) that holds the file's ONLY WhichAutoNeedTray() call site
//      (golden :454).  By (iii) the first disjunct at golden :440 always wins and
//      breaks to Task=20.  (The old note named only the `WhichAutoNeedTray()==0`
//      guard arm; the dead region is the whole tail, and the call never happens.)
//    * DoAuto2 case 40's else arm (golden :487-491) -- the "sensor genuinely reads
//      off" path.  By (ii) SnAuto2FixCyPush is "unknown", so `IsOff()==false` is
//      TRUE and the then-arm always runs.
//    * DoAuto2 case 65's STALE-TRAY ClearTray arm (golden :518-524): requires
//      LastSet.iRealDummy>=HAS_TRAY, false under DUMMY(0) by (iii).
//    * DoAuto2 case 70's genuine tray-detect disjunct (golden :540-541), dead by
//      (i); case 70 always takes the DUMMY/non-AMR disjunct (golden :542-544).
//    * DoAuto2Receive case 100's NO-TRAY-ON-CAR arm (golden :733-737).  MEASURED
//      with a probe: 0 hits.  In [7] SUPPORT_2_EMPTY_EMPTY leaves a tray on the
//      car (golden :208), so the Task=200 arm always wins.
//    * All four Cylinder[C_Auto2LoaderZ_Select] Push()/Pop() alternatives, each
//      short-circuited by an `iRealDummy==DUMMY` disjunct: DoLoadNewAuto2TrayToCar
//      case 100 (golden :94-95) and case 300 (golden :123-126),
//      DoUnLoadNewAuto2ToStack case 100 (golden :628-629) and case 300
//      (golden :647-648).
//    * DoUnLoadNewAuto2ToStack case 1's three-sensor conjunction (golden :611-613),
//      short-circuited by the `|| DUMMY` disjunct at golden :614.
//    * The #ifdef SOFT_SIMULTE arms (not defined in this build).
//    * Real lifter/cylinder motion -- see the AutoCylinder* note above.
//
//  ACTUATOR SITES THAT ARE EXECUTED BUT STILL NOT ASSERTED
//  AI(W906-W7-L1fix3) 20260801: NEW section.  The three holes this revision
//  closes (belt polarity, cylinder direction, MMAuto2Z bookkeeping) are closed at
//  the sites listed under ACTUATOR STATE THAT IS ASSERTED and NOWHERE ELSE.  The
//  sites below are on driven paths -- they DO run -- but no assertion observes
//  them, so a polarity slip at any of them would still ship green:
//    * SW[] writes that are overwritten inside the same tick, hence invisible at
//      any tick boundary this harness can sample:
//        - golden :221 / :223 (DoAuto2TrayToFront case 100 hand-off arm) undo
//          golden :215-216 in the same tick.  [4b] observes the ON pair only by
//          blocking that hand-off; on [4]'s own path the OFF pair is unobserved.
//        - golden :205 / :207 (the SUPPORT_2_EMPTY_EMPTY arm taken by [7]).
//    * golden :388, SW[SwACAuto2CW].Off() in DoAuto2TrayToRear case 460.  It runs,
//      but case 200 (golden :347) already left that output OFF, so deleting :388
//      changes nothing observable.  Only its sibling :389 is load-bearing, and
//      only :389 is asserted.
//    * Cylinder DIRECTION at every site OUTSIDE DoAuto2TrayToFront cases 101-104
//      (golden :270, :281, :291, :298 -- those four, and only those four, are the
//      ones asserted):
//        - Cylinder[C_Auto2Side_Fixer]: .Off() golden :201, :463 (and :481, dead);
//          .Pop() golden :334, :394, :468, :494.
//        - Cylinder[C_Auto2EdgePush]:   .Off() golden :202, :331, :381, :464.
//      Several of these feed cursor transitions this file DOES assert, but only
//      through Push()/Pop()'s return value -- which is unconditionally true
//      offline for BOTH -- so the transition assertions cannot see direction.
//    * MOT[MMAuto2Z] golden :165 (the JAM1213 K_SKIP arm of case 420).  Dead here
//      for the reason given under EVERY JAM/MES ALARM ARM; it is the one of the
//      four golden MMAuto2Z clears that no assertion can reach.
//
//  FAILABILITY PROOF -- REPRODUCIBLE RECIPE, NOT A POINTER
//  AI(W906-W7-L1fix3) 20260801: THE PREVIOUS VERSION OF THIS BANNER PASTED OUTPUT
//  THAT THE SHIPPED FILE COULD NOT PRODUCE.  It quoted the harness's `(line 683)`
//  / `(line 681)` suffixes, but the assertions it named printed `(line 804)` and
//  `(line 802)` -- off by exactly the 121 lines of banner added AFTER the capture
//  and never re-run.  That is precisely why the tables below quote the ASSERTION
//  MESSAGE and never the `(line N)` suffix: the message is stable, the line number
//  is pushed by every edit to this banner (KNOWLEDGE gotcha 10).
//
//  RECIPE.  Everything below was run in this session, in a private scratch dir --
//  the shared `build/` tree was never written to and `asendic_Auto2.cpp` was never
//  modified on disk (sha256 22d9e518e2cd4c09f0e1eb583ece0054c1cdf049dcf5e00330f
//  2cb8d0a0ace9e before and after, re-checked).  Each mutation was applied to a
//  COPY of asendic_Auto2.cpp, compiled off-tree with the flags CMake records for
//  target ht9045_sm (`-std=c++1z -DCSYSTEM_TICK_ORACLE -I<tree> -I<tree>/SECSGEM`),
//  `ar r`'d into a COPY of libht9045_sm.a, and linked into a private exe.
//
//  BASELINE (pristine asendic_Auto2.cpp):     RESULT: 60 passed, 0 failed  (exit 0)
//
//  WHICH MUTATIONS WERE ALSO RUN AGAINST THE PREVIOUS 46-ASSERTION REVISION.
//  Be precise about this, because "it used to be silent" is the whole reason each
//  probe exists.  A1, A2, B1, B2 and C0 WERE re-run against the previous revision
//  and each left it at exactly `RESULT: 46 passed, 0 failed` -- that is a measured
//  result, not an inference.  A3, A4, B3, B4, C1, C2 and C3 were NOT: they are
//  finer-grained probes added alongside those five.  They could not have been
//  caught there either -- in the previous revision's 922 lines, `SW[` and
//  `Cylinder[` occurred ONLY inside banner comments and in no executable
//  statement at all, and MOT[MMAuto2Z] occurred only as the two fixture
//  ClearTray() calls in [3a] and [3b], with fHasTray never read -- but that is a
//  reading of the old file, not a measurement, and is labelled as such.
//
//  --- (a) BELT OUTPUT POLARITY --------------------------------------------------
//  (A1) golden :215-216, DoAuto2TrayToFront case 100:
//         `SW[SwACAuto2].On(); SW[SwACAuto2CW].On();` -> `.Off(); .Off();`
//       RESULT: 58 passed, 2 failed
//         FAIL: case 100 drove the Auto2 belt output ON (golden :215)
//         FAIL: case 100 drove the Auto2 CW belt output ON (golden :216)
//  (A2) golden :346-347, DoAuto2TrayToRear case 200:
//         `SW[SwACAuto2].On(); SW[SwACAuto2CW].Off();` -> `.Off(); .On();`
//       RESULT: 59 passed, 1 failed
//         FAIL: case 200 drove the belt ON with CW OFF while pulling the tray
//               back (golden :346-347)
//  (A3) golden :389, DoAuto2TrayToRear case 460: `SW[SwACAuto2].Off();` deleted
//       RESULT: 59 passed, 1 failed
//         FAIL: case 460 stopped the belt once the tray settled (golden :389)
//  (A4) golden :622-623, DoUnLoadNewAuto2ToStack case 50: both `.Off()` -> `.On()`
//       RESULT: 59 passed, 1 failed
//         FAIL: case 50 stopped BOTH belt outputs before handing to 100
//               (golden :622-623)
//
//  --- [4b]'s TWO PARK PRECONDITIONS --------------------------------------------
//  A1 is only meaningful if the SM really is sitting in case 100 when SW[] is
//  read, so the two assertions that establish that are proven as well:
//  (P1) golden :219, the case-100 hand-off guard:
//         `Sen[SnAuto2TrayDetect].IsOff()==false` -> `...IsOff()==true`
//       RESULT: 53 passed, 7 failed, including
//         FAIL: cursor parked at 100 (hand-off guard blocked, no timeout arm
//               reachable)
//       (The other six are [4], which needs that same guard to reach 101 at all.)
//  (P2) golden :203: `if(SUPPORT_2_EMPTY_EMPTY)` -> `if(!SUPPORT_2_EMPTY_EMPTY)`
//       RESULT: 48 passed, 12 failed, including
//         FAIL: case 100 did not converge -- SM parked with the belt running
//       (The rest are [4] and [7], which both depend on that arm's polarity.)
//
//  --- (b) CYLINDER ACTUATION DIRECTION ------------------------------------------
//  (B1) golden :270, case 101: `Cylinder[C_Auto2EdgePush].On();` -> `.Off();`
//       RESULT: 59 passed, 1 failed
//         FAIL: case 101 EXTENDED the edge-push cylinder before handing to 102
//               (golden :270 On(), not Off())
//  (B2) golden :281, case 102: `Cylinder[C_Auto2Side_Fixer].Push()` -> `.Pop()`
//       RESULT: 59 passed, 1 failed
//         FAIL: case 102 PUSHED the side fixer before handing to 103
//               (golden :281 Push(), not Pop())
//       This is the mutation that matters most: BOTH Push() and Pop() return true
//       offline, so the cursor walk is byte-identical and every trajectory
//       assertion in this file stays green.  Only Cylinder[].bCylinderOn sees it.
//  (B3) golden :291, case 103: `Cylinder[C_Auto2Side_Fixer].Off();` deleted
//       RESULT: 59 passed, 1 failed
//         FAIL: case 103 released the side fixer again (golden :291)
//  (B4) golden :298, case 104: `Cylinder[C_Auto2EdgePush].Off();` deleted
//       RESULT: 59 passed, 1 failed
//         FAIL: case 104 retracted the edge-push cylinder again (golden :298)
//
//  --- (c) MMAuto2Z TRAY BOOKKEEPING ---------------------------------------------
//  (C0) ALL FOUR `MOT[MMAuto2Z].ClearTray(__FUNC__);` deleted (golden :60, :87,
//       :154, :165)
//       RESULT: 57 passed, 3 failed
//         FAIL: case 50 cleared the MMAuto2Z tray flag before handing to 100
//               (golden :87)
//         FAIL: case 420 cleared the RE-ARMED MMAuto2Z tray flag on success
//               (golden :154)
//         FAIL: short-circuit also cleared the MMAuto2Z tray flag (golden :60)
//       (Three FAILs, not four: golden :165 is in the JAM1213 K_SKIP arm, which is
//        dead here -- see NOT COVERED.  This test does NOT cover it and does not
//        claim to.)
//  Each live site is INDEPENDENTLY load-bearing, which is what the [3a] re-arm
//  exists to guarantee -- deleted one at a time:
//  (C1) golden :87  only  -> 59/1, FAIL: case 50 cleared the MMAuto2Z tray flag
//                                        before handing to 100 (golden :87)
//  (C2) golden :154 only  -> 59/1, FAIL: case 420 cleared the RE-ARMED MMAuto2Z
//                                        tray flag on success (golden :154)
//  (C3) golden :60  only  -> 59/1, FAIL: short-circuit also cleared the MMAuto2Z
//                                        tray flag (golden :60)
//
//  --- SCOPE OF THE TRAJECTORY CLAIM (see PROOF OBJECTIVE) ----------------------
//  (S1) golden :469, DoAuto2 case 30 (DRIVEN): `Task=40;` -> `Task=999;`
//       RESULT: 52 passed, 8 failed -- 8 distinct trajectory/hand-off assertions
//       across [8] scenarios A-D go red.
//  (S2) golden :490, DoAuto2 case 40 ELSE arm (DEAD): `Task=50;` -> `Task=999;`
//       RESULT: 60 passed, 0 failed -- unchanged, because the arm is never
//       entered.  This is the counter-example that forced the scoped wording.
//
//  --- THE CURSOR-EDGE ASSERTION OF [8] SCENARIO C (carried over, re-run) -------
//  (M1) golden :588, DoAuto2 case 300: `Task=50;` -> `Task=1;`
//       RESULT: 59 passed, 1 failed
//         FAIL: case 300 consumed DoAuto2TrayToRear()==true and the cursor
//               stepped 300 -> 50 in ONE tick (golden :588)
//       The same run still printed
//         (DoAuto2 scenario C visited: 1 2 3 20 30 40 50 60 65 70 100 300)
//       -- 50 IS in the set even with case 300's own transition destroyed, the
//       direct demonstration that the older set-membership form of this assertion
//       was a tautology.
//  (M2) golden :558, DoAuto2 case 100: `Task=300;` -> `Task=1;` (300 never entered)
//       RESULT: 58 passed, 2 failed
//         FAIL: scenario C reached case 300 (case-100 recycle arm, golden :557-558)
//         FAIL: case 300 consumed DoAuto2TrayToRear()==true and the cursor
//               stepped 300 -> 50 in ONE tick (golden :588)
//
//  Offline: no vendor SDK, no hardware, no window.
// =============================================================================
#include "asendic_Auto2.h"
#include "csystem.h"
#include "asendic.h"
#include "acatchtray.h"
#include "Motor/mymotor.h"
#include "mycylin.h"
#include "mysensor.h"
#include "myswitch.h"
#include "myTimer.h"            // TQPF_Timer (the virtual-time seam below)
#include "cprod.h"
#include "cpublic.h"
#include "cmydef.h"
#include "canary_support.h"     // LastSet, ShowErrorMessage, WhichAutoNeedTray
#include "FormsFacade.h"        // fAGV
// AI(W906-W7-L1-Wave3) 20260802: AutoCylinderUp/Middle/Lower are no longer the
// `{ return true; }` stubs in acatchtray_shims.cpp -- asendic.cpp now carries
// golden's real closed-loop bodies, which wait on Cylinder[].OnStatus()/
// OffStatus().  This header is the physical Auto stack that answers them.
#include "w3_cylinder_plant.h"

// AI(W906-W7-L1-Wave3) 20260802: golden declares AutoCylinderMidIsOn in NO header
// (asendic.cpp:529 defines it with external linkage and only asendic.cpp calls
// it), so asendic.h deliberately does not publish it either.  Sub-test [12c]
// needs it, so it is declared locally here -- the same shape golden main.cpp:9138
// and golden Magazine.cpp:33 use for the other asendic.cpp free functions they
// reach.
extern bool AutoCylinderMidIsOn(int Part, int CylinderNameMid);   // golden asendic.cpp:529
#include <cstdio>
#include <set>

// ---------------------------------------------------------------------------
//  Minimal PASS / FAIL harness (same style as test_w6_1_empty_canary.cpp)
// ---------------------------------------------------------------------------
static int g_pass = 0, g_fail = 0;

#define CHECK(cond, msg)                                                        \
    do {                                                                        \
        if (cond) { printf("  PASS: %s\n", msg); ++g_pass; }                   \
        else      { printf("  FAIL: %s  (line %d)\n", msg, __LINE__); ++g_fail; } \
    } while (0)

// ---------------------------------------------------------------------------
//  VIRTUAL-TIME SEAM -- the file-scope TQPF_Timer objects of asendic_Auto2.cpp.
//  Progress timers (advanced every tick):
// ---------------------------------------------------------------------------
extern TQPF_Timer Auto2Delay;                   // golden asendic_Auto2.cpp:31
extern TQPF_Timer DoAuto2Delay;                 // golden asendic_Auto2.cpp:407
extern TQPF_Timer CheckAuto2TrayDelay;          // golden asendic_Auto2.cpp:408
extern TQPF_Timer hAuto2TrayToFrontForDummy;    // golden asendic_Auto2.cpp:177
extern TQPF_Timer hAuto2TrayToRearForDummy;     // golden asendic_Auto2.cpp:314
//  JAM watchdogs -- declared for documentation; deliberately NEVER advanced:
//    extern TQPF_Timer hAuto2TrayToFront;      // golden asendic_Auto2.cpp:176 (20 s)
//    extern TQPF_Timer hAuto2TrayToRear;       // golden asendic_Auto2.cpp:313 (20 s)

// Expire whatever progress timer the SM is currently waiting on, so one loop
// iteration == one "tick" of virtual time instead of one nanosecond of real time.
static void advanceAuto2Time()
{
    Auto2Delay.SetMSAndOn(0);
    DoAuto2Delay.SetMSAndOn(0);
    CheckAuto2TrayDelay.SetMSAndOn(0);
    hAuto2TrayToFrontForDummy.SetMSAndOn(0);
    hAuto2TrayToRearForDummy.SetMSAndOn(0);
    // AI(W906-W7-L1-Wave3) 20260802: one tick of the Auto-stack plant, folded
    // into the existing per-tick hook so every drive loop in this file gets it
    // without changing its shape.  w3::Tick() expires AutoTime[][] (the
    // AutoCylinder* motion/alarm timers -- the same virtual-time idiom as the
    // five lines above) and recomputes the stack's position sensors from the
    // commanded outputs.  It deliberately does NOT expire CylinderAlarmTime[],
    // the case-201 10-second watchdog.
    w3::Tick();
}

// ---------------------------------------------------------------------------
//  SIM SENSOR SEAM -- see banner note 2.
// ---------------------------------------------------------------------------
static void simSensorOff(int idx)       // IsOff()==true, IsOn()==false
{
    Sen[idx].Enable  = true;
    Sen[idx].Type    = TYPE_A;          // cmydef.cpp:269 TYPE_A==1
    Sen[idx].ISABase = eISABase;        // -> mysensor.cpp:29 raw-port stub (false)
}
static void simSensorUnknown(int idx)   // IsOn()==false AND IsOff()==false (default)
{
    Sen[idx].Enable = false;
}

// DoAuto2()'s documented state set (golden :430-591 -> 1,2,3,20,30,40,50,60,65,
// 70,100,150,200,300).  A wild cursor means corruption.
static bool auto2CursorSane(int t)
{
    switch (t)
    {
        case 1: case 2: case 3: case 20: case 30: case 40: case 50: case 60:
        case 65: case 70: case 100: case 150: case 200: case 300:
            return true;
        default:
            return false;
    }
}

// ---------------------------------------------------------------------------
//  Trajectory recorder + reporter
// ---------------------------------------------------------------------------
typedef std::set<int> StateSet;

static bool covers(const StateSet &seen, const int *want, int nWant)
{
    for (int i = 0; i < nWant; ++i)
        if (seen.find(want[i]) == seen.end())
            return false;
    return true;
}

static void printSeen(const char *what, const StateSet &seen)
{
    printf("    (%s visited:", what);
    for (StateSet::const_iterator it = seen.begin(); it != seen.end(); ++it)
        printf(" %d", *it);
    printf(")\n");
}

static void printMissing(const char *what, const StateSet &seen,
                         const int *want, int nWant)
{
    printf("    (%s MISSING:", what);
    for (int i = 0; i < nWant; ++i)
        if (seen.find(want[i]) == seen.end())
            printf(" %d", want[i]);
    printf(")\n");
}

// ---------------------------------------------------------------------------
//  Run-mode globals: the DUMMY tray-feed configuration the SM expects.
//  The two settle delays are pinned to 0 s because they are operator-tunable
//  machine settings (Ld_ULd form), not SM logic: SetSecAndOn(0) makes the
//  arrival/back settle waits immediately elapsed, which is what makes the SM
//  steppable in a headless test without faking any transition.
// ---------------------------------------------------------------------------
static void setupDummyFeed()
{
    LastSet.iRealDummy      = DUMMY;    // dummy feed (no real IC/tray sensors)
    fAuto2CanSupplyNewTray  = true;     // allow supply
    iReceiveAuto2Tray       = 0;        // not in receive mode
    bEmptyPause             = false;
    bHandlerPause           = false;
    SUPPORT_2_EMPTY_EMPTY   = false;    // cmydef.cpp:3050 default
    Ld_UldDelayTime.LD_TrayArrivalDely = 0;   // golden DoAuto2TrayToRear case 450
    Ld_UldDelayTime.ULD_TrayBackDelay  = 0;   // golden DoUnLoadNewAuto2ToStack 100/310

    // Auto2 stack/track sensors: "unknown" by default (matches an unconfigured
    // offline IO table); individual sub-tests raise the ones they need.
    simSensorUnknown(SnAuto2PreDete);
    simSensorUnknown(SnAuto2_Tray_Car);
    simSensorUnknown(SnAuto2TrayDetect);
    simSensorUnknown(SnAuto2CWPreDetect);
    simSensorUnknown(SnAuto2FixCyPush);
    simSensorUnknown(SnAuto2TrayHasTray);
    simSensorUnknown(SnAutoUpSafedetect1);

    // AI(W906-W7-L1-Wave3) 20260802: wire the Auto2 stack the way a real machine
    // wires it -- Enable, distinct Sim IO addresses for the output bit and the
    // two position sensors, non-zero alarm windows -- so golden's real
    // AutoCylinderUp/Middle/Lower can actually close their loops.
    //
    // NOTE THE ARGUMENT ORDER, IT IS NOT A TYPO.  asendic_Auto2.cpp calls
    // `AutoCylinderXxx(1, C_Auto2_Selector, C_Auto2_Up)` at ALL TWELVE of its
    // sites (golden asendic_Auto2.cpp :65, :85, :101, :106, :132, :137, :617,
    // :620, :638, :643, :674, :679) -- the REVERSE of what asendic_Auto.cpp /
    // _Auto_RT.cpp / csystem.cpp / acatchtray.cpp pass.  Parameters 2 and 3 are
    // not interchangeable in the real bodies, so the fixture must be wired in
    // the SAME order the SM passes them.  This line is therefore a PIN: if
    // anyone "normalises" Auto2's twelve call sites to (Up, Selector), the SM
    // stops converging against this fixture and [3a]/[6]/[7]/[8] go red.
    w3::Forget();
    w3::WireAutoStack(C_Auto2_Selector, C_Auto2_Up, 22);
    w3::ResetCursors();
    Ld_UldDelayTime.LD_BeforeDownDelay    = 0;
    Ld_UldDelayTime.LD_StackMiddLockDelay = 0;
    Ld_UldDelayTime.LD_LiftDownDelay      = 0;
}

// ---------------------------------------------------------------------------
//  [1] Init* reset task cursors to 1
// ---------------------------------------------------------------------------
static void test_inits()
{
    printf("\n[1] Init* reset task cursors to 1\n");

    iLoadNewAuto2TrayToCarTask = 999;
    InitLoadNewAuto2TrayToCarTask();
    CHECK(iLoadNewAuto2TrayToCarTask == 1, "InitLoadNewAuto2TrayToCarTask() -> iLoadNewAuto2TrayToCarTask==1");

    iAuto2TrayToFrontTask = 999;
    InitAuto2TrayToFrontTask();
    CHECK(iAuto2TrayToFrontTask == 1, "InitAuto2TrayToFrontTask() -> iAuto2TrayToFrontTask==1");

    iAuto2TrayToRearTask = 999;
    InitAuto2TrayToRearTask();
    CHECK(iAuto2TrayToRearTask == 1, "InitAuto2TrayToRearTask() -> iAuto2TrayToRearTask==1");

    iAutoAuto2Task = 999;
    InitAuto2Task();
    CHECK(iAutoAuto2Task == 1, "InitAuto2Task() -> iAutoAuto2Task==1");

    iUnLoadNewAuto2TrayTask = 999;
    InitUnLoadNewAuto2TrayTask();
    CHECK(iUnLoadNewAuto2TrayTask == 1, "InitUnLoadNewAuto2TrayTask() -> iUnLoadNewAuto2TrayTask==1");

    iAuto2ReceiveTask = 999;
    InitAuto2ReceiveTask();
    CHECK(iAuto2ReceiveTask == 1, "InitAuto2ReceiveTask() -> iAuto2ReceiveTask==1");
}

// ---------------------------------------------------------------------------
//  [2] The virtual-time seam itself must work, or every trajectory below is
//      vacuous.  Assert BOTH directions: a re-armed 1 s timer is NOT elapsed,
//      and after advanceAuto2Time() it IS.
// ---------------------------------------------------------------------------
static void test_virtual_time_seam()
{
    printf("\n[2] Virtual-time seam (TQPF_Timer re-arm to 0 ms)\n");

    DoAuto2Delay.SetMSAndOn(1000);
    CHECK(DoAuto2Delay.Off() == false, "a freshly armed 1000 ms timer is NOT elapsed");
    advanceAuto2Time();
    CHECK(DoAuto2Delay.Off() == true,  "after advanceAuto2Time() the same timer IS elapsed");

    // And the sensor seam: prove both polarities, since 'unknown' vs 'off' is
    // exactly the distinction the old test got wrong.
    simSensorUnknown(SnAuto2PreDete);
    CHECK(Sen[SnAuto2PreDete].IsOn() == false && Sen[SnAuto2PreDete].IsOff() == false,
          "Enable==false sensor reads neither On nor Off ('unknown', mysensor.cpp:127/177)");
    simSensorOff(SnAuto2PreDete);
    CHECK(Sen[SnAuto2PreDete].IsOff() == true && Sen[SnAuto2PreDete].IsOn() == false,
          "Enable+TYPE_A sensor reads Off (IsOff()==true, IsOn()==false)");
    simSensorUnknown(SnAuto2PreDete);
}

// ---------------------------------------------------------------------------
//  [3] DoLoadNewAuto2TrayToCar() -- BOTH branches of case 1.
//
//  [3a] Stack sensors "off"  -> the FULL lifter walk (golden :64-65 takes
//       Task=50), trajectory 50,100,200,201,300,400,410,420, converges with a
//       tray latched onto MMAuto2_Car (case 420, golden :153).
//  [3b] Stack sensors "unknown" -> the case-1 short-circuit (golden :56-63)
//       returns true on step 0 WITHOUT ever reaching 50.  This is the branch the
//       old test hit while claiming to exercise the lifter end-to-end; pinned
//       here so the difference can never again be mistaken for coverage.
// ---------------------------------------------------------------------------
static void test_load_new_auto2_full_lifter_walk()
{
    printf("\n[3a] DoLoadNewAuto2TrayToCar() full lifter walk (stack sensors OFF)\n");

    setupDummyFeed();
    InitLoadNewAuto2TrayToCarTask();
    MOT[MMAuto2].ClearTray("test_w7_l1_auto2");
    MOT[MMAuto2_Car].ClearTray("test_w7_l1_auto2");
    // AI(W906-W7-L1fix3) 20260801: MMAuto2Z now starts WITH a tray.  It used to be
    // cleared here, which made every MOT[MMAuto2Z].ClearTray() in this SM a no-op
    // that no assertion could distinguish from its absence -- deleting all four
    // golden call sites (:60, :87, :154, :165) left the whole suite green.
    MOT[MMAuto2Z].SetTray(NULL_IC, "test_w7_l1_auto2");

    // Both stack sensors must read a definite OFF, else case 1 short-circuits.
    simSensorOff(SnAuto2PreDete);
    simSensorOff(SnAuto2_Tray_Car);

    StateSet seen;
    bool done  = false;
    int  steps = 0;
    int  zAt100  = -1;      // MOT[MMAuto2Z].fHasTray sampled the first time the
                            // cursor rests on 100, i.e. right after case 50 ran
    bool rearmed = false;
    for (steps = 0; steps < 200 && !done; ++steps)
    {
        advanceAuto2Time();
        // Re-arm the Z-stage flag ONCE, after case 50 has cleared it and before
        // case 420 runs, so that the case-420 clear is INDEPENDENTLY load-bearing:
        // without this, deleting golden :154 alone would still find fHasTray
        // already false, left over from golden :87.
        if (iLoadNewAuto2TrayToCarTask == 410 && rearmed == false)
        {
            MOT[MMAuto2Z].SetTray(NULL_IC, "test_w7_l1_auto2");
            rearmed = true;
        }
        done = DoLoadNewAuto2TrayToCar();
        seen.insert(iLoadNewAuto2TrayToCarTask);
        if (iLoadNewAuto2TrayToCarTask == 100 && zAt100 < 0)
            zAt100 = MOT[MMAuto2Z].fHasTray ? 1 : 0;
    }

    static const int want[] = {50, 100, 200, 201, 300, 400, 410, 420};
    const int nWant = (int)(sizeof(want)/sizeof(want[0]));

    CHECK(done, "DoLoadNewAuto2TrayToCar() returns true (converged, not bound-exhausted)");
    CHECK(covers(seen, want, nWant),
          "trajectory covers 50,100,200,201,300,400,410,420 (all AutoCylinder* call sites reached)");
    CHECK(MOT[MMAuto2_Car].fHasTray == true,
          "case 420 latched the tray onto MMAuto2_Car (golden :153 SetTray)");
    CHECK(zAt100 == 0,
          "case 50 cleared the MMAuto2Z tray flag before handing to 100 (golden :87)");
    CHECK(rearmed && MOT[MMAuto2_Car].fHasTray && MOT[MMAuto2Z].fHasTray == false,
          "case 420 cleared the RE-ARMED MMAuto2Z tray flag on success (golden :154)");
    CHECK(iLoadNewAuto2TrayToCarTask == 1, "cursor reset to 1 on convergence");
    printSeen("DoLoadNewAuto2TrayToCar", seen);
    if (!covers(seen, want, nWant)) printMissing("DoLoadNewAuto2TrayToCar", seen, want, nWant);
    printf("    (converged=%d after %d steps, MMAuto2Z@100=%d rearmed=%d)\n",
           (int)done, steps, zAt100, (int)rearmed);
}

static void test_load_new_auto2_case1_shortcircuit()
{
    printf("\n[3b] DoLoadNewAuto2TrayToCar() case-1 short-circuit (sensors UNKNOWN)\n");

    setupDummyFeed();                       // leaves all Auto2 sensors 'unknown'
    InitLoadNewAuto2TrayToCarTask();
    MOT[MMAuto2].ClearTray("test_w7_l1_auto2");
    MOT[MMAuto2_Car].ClearTray("test_w7_l1_auto2");
    // AI(W906-W7-L1fix3) 20260801: was ClearTray -- see [3a].  The short-circuit
    // clears MMAuto2Z too (golden :60), which is unobservable if it starts clear.
    MOT[MMAuto2Z].SetTray(NULL_IC, "test_w7_l1_auto2");

    advanceAuto2Time();
    bool firstStep = DoLoadNewAuto2TrayToCar();

    CHECK(firstStep == true,
          "returns true on the FIRST step (case-1 short-circuit, golden :56-63)");
    CHECK(iLoadNewAuto2TrayToCarTask == 1, "cursor never left 1 -- case 50 NOT reached");
    CHECK(MOT[MMAuto2_Car].fHasTray == true, "short-circuit still latches MMAuto2_Car (golden :59)");
    CHECK(MOT[MMAuto2Z].fHasTray == false,
          "short-circuit also cleared the MMAuto2Z tray flag (golden :60)");
}

// ---------------------------------------------------------------------------
//  [4] DoAuto2TrayToFront() long path (golden :183-310).
//      case 1 falls through into case 100 (GOLDEN QUIRK, golden :194), which
//      hands off to 101 -> 102 -> 103 -> 104 -> 105 -> return true.
//
//      FALL-THROUGH PIN: because case 1 has no `break`, case 1 and case 100 both
//      run in the SAME tick, so the cursor value 100 is never observable between
//      ticks -- after ONE call it is already 101.  Asserting exactly that is a
//      direct test of the golden quirk: insert a `break;` after golden :193's
//      `Task=100;` and the first tick would leave the cursor at 100 instead.
//
//      AI(W906-W7-L1fix3) 20260801: THIS TEST ALSO NOW READS Cylinder[] STATE.
//      The trajectory assertions alone cannot see actuation DIRECTION: golden :281
//      is `if(Cylinder[C_Auto2Side_Fixer].Push())` and offline Pop() returns true
//      just as Push() does, so swapping them keeps the cursor walk identical while
//      leaving the tray UNCLAMPED at the front-edge stop.  Proven silent before
//      this revision: `.Push()` -> `.Pop()` at golden :281 and `.On()` -> `.Off()`
//      at golden :270 each left the suite at 46 passed, 0 failed.  Both are now
//      observed on Cylinder[].bCylinderOn, sampled at the cursor value golden
//      guarantees the actuator is in that position.
// ---------------------------------------------------------------------------
static void test_tray_to_front_long_path()
{
    printf("\n[4] DoAuto2TrayToFront() long path 101,102,103,104,105\n");

    setupDummyFeed();
    InitAuto2TrayToFrontTask();
    MOT[MMAuto2].SetTray(NULL_IC, "test_w7_l1_auto2");
    MOT[MMAuto2_Car].SetTray(NULL_IC, "test_w7_l1_auto2");
    simSensorOff(SnAuto2_Tray_Car);         // IsOn()==false -> case 100 hands to 101

    StateSet seen;
    bool done  = false;
    int  steps = 0;
    int  afterFirstTick = -1;
    // Both cylinders are driven to their RETRACTED state by case 100 (golden
    // :201-202) on tick 1, so each sample below observes a transition the SM made
    // itself, not a leftover from an earlier sub-test.
    int  edgePushAt102 = -1;    // Cylinder[C_Auto2EdgePush].bCylinderOn  @ cursor 102
    int  fixerAt103    = -1;    // Cylinder[C_Auto2Side_Fixer].bCylinderOn @ cursor 103
    for (steps = 0; steps < 200 && !done; ++steps)
    {
        advanceAuto2Time();
        done = DoAuto2TrayToFront();
        seen.insert(iAuto2TrayToFrontTask);
        if (steps == 0) afterFirstTick = iAuto2TrayToFrontTask;
        if (iAuto2TrayToFrontTask == 102 && edgePushAt102 < 0)
            edgePushAt102 = Cylinder[C_Auto2EdgePush].bCylinderOn ? 1 : 0;
        if (iAuto2TrayToFrontTask == 103 && fixerAt103 < 0)
            fixerAt103 = Cylinder[C_Auto2Side_Fixer].bCylinderOn ? 1 : 0;
    }

    static const int want[] = {101, 102, 103, 104, 105};
    const int nWant = (int)(sizeof(want)/sizeof(want[0]));

    CHECK(afterFirstTick == 101,
          "GOLDEN QUIRK pin: case 1 fell through into case 100 in one tick (cursor 1 -> 101)");
    CHECK(done, "DoAuto2TrayToFront() returns true (converged)");
    CHECK(covers(seen, want, nWant), "trajectory covers 101,102,103,104,105");
    CHECK(MOT[MMAuto2].fHasTray == false && MOT[MMAuto2_Car].fHasTray == false,
          "case 105 cleared BOTH MMAuto2 and MMAuto2_Car (golden :303-304)");
    CHECK(edgePushAt102 == 1,
          "case 101 EXTENDED the edge-push cylinder before handing to 102 (golden :270 On(), not Off())");
    CHECK(fixerAt103 == 1,
          "case 102 PUSHED the side fixer before handing to 103 (golden :281 Push(), not Pop())");
    CHECK(Cylinder[C_Auto2Side_Fixer].bCylinderOn == false,
          "case 103 released the side fixer again (golden :291)");
    CHECK(Cylinder[C_Auto2EdgePush].bCylinderOn == false,
          "case 104 retracted the edge-push cylinder again (golden :298)");
    printSeen("DoAuto2TrayToFront", seen);
    if (!covers(seen, want, nWant)) printMissing("DoAuto2TrayToFront", seen, want, nWant);
    printf("    (edgePush@102=%d sideFixer@103=%d)\n", edgePushAt102, fixerAt103);
}

// ---------------------------------------------------------------------------
//  [4b] DoAuto2TrayToFront() case 100 -- BELT-DRIVE OUTPUT POLARITY.
//
//  AI(W906-W7-L1fix3) 20260801: NEW SUB-TEST.  Nothing in this file read SW[]
//  state, so inverting the two conveyor commands at golden :215-216
//  (`SW[SwACAuto2].On(); SW[SwACAuto2CW].On();` -> `.Off(); .Off();`) left the
//  suite at 46 passed, 0 failed -- a translation slip that stops the Auto2 belt
//  dead shipped invisibly.  Observed here on SW[].OutValue (see the ACTUATOR
//  STATE block in the file banner for why OutValue and not Status()).
//
//  WHY THIS CANNOT LIVE INSIDE [4].  On [4]'s path the belt-ON at golden :215-216
//  is not observable at ANY tick boundary: case 100 immediately takes the hand-off
//  at golden :218-228, which turns both outputs back off (golden :221 / :223) in
//  the SAME tick.  To park the SM inside case 100 with the belt still running this
//  sub-test does two things, and both are needed:
//    * raises SnAuto2TrayDetect to a definite "off", so the hand-off guard at
//      golden :219 (`Sen[SnAuto2TrayDetect].IsOff()==false`) is FALSE.  This is
//      the ONE place in the file that departs from fixture invariant (ii); it is
//      restored to "unknown" before returning; and
//    * runs exactly ONE tick WITHOUT advanceAuto2Time(), so
//      hAuto2TrayToFrontForDummy still carries the 5 s deadline case 1 armed it
//      with in that same tick (golden :192) and the DUMMY-timeout success arm at
//      golden :230-243 cannot fire either.
//  Case 100 has exactly four other ways out and all four are shut here: the
//  bHandlerPause abort (golden :195) is dead by invariant (iv); the
//  SUPPORT_2_EMPTY_EMPTY success arm (golden :203) is false because
//  setupDummyFeed() pins that flag; and the last two are real-QPC deadlines of
//  5 s (golden :230) and 20 s (golden :245), which a single tick of pure
//  computation cannot reach.  So the park is deterministic, not a wall-clock race.
// ---------------------------------------------------------------------------
static void test_tray_to_front_belt_polarity()
{
    printf("\n[4b] DoAuto2TrayToFront() case 100 belt-drive polarity (SW[] state)\n");

    setupDummyFeed();
    InitAuto2TrayToFrontTask();
    MOT[MMAuto2].SetTray(NULL_IC, "test_w7_l1_auto2");
    MOT[MMAuto2_Car].SetTray(NULL_IC, "test_w7_l1_auto2");
    simSensorOff(SnAuto2TrayDetect);        // blocks the golden :218-228 hand-off

    // Pre-drive BOTH belt outputs to the OPPOSITE of what case 100 must write, so
    // a DELETED call fails the same assertion an INVERTED call does.
    SW[SwACAuto2].Off();
    SW[SwACAuto2CW].Off();

    bool converged = DoAuto2TrayToFront();  // NO advanceAuto2Time() -- see banner

    CHECK(converged == false,
          "case 100 did not converge -- SM parked with the belt running");
    CHECK(iAuto2TrayToFrontTask == 100,
          "cursor parked at 100 (hand-off guard blocked, no timeout arm reachable)");
    CHECK(SW[SwACAuto2].OutValue == true,
          "case 100 drove the Auto2 belt output ON (golden :215)");
    CHECK(SW[SwACAuto2CW].OutValue == true,
          "case 100 drove the Auto2 CW belt output ON (golden :216)");

    // Restore the fixture: invariant (ii) holds again from here on.
    SW[SwACAuto2].Off();
    SW[SwACAuto2CW].Off();
    simSensorUnknown(SnAuto2TrayDetect);
    InitAuto2TrayToFrontTask();
}

// ---------------------------------------------------------------------------
//  [5] DoAuto2TrayToRear() (golden :321-404).  case 1 falls through into case
//      100 (GOLDEN QUIRK, golden :333) -> 200 -> 450 -> 460 -> 500 -> true.
//
//      Same FALL-THROUGH PIN as [4], and here it is even tighter: case 100's own
//      body (`if(Cylinder[C_Auto2Side_Fixer].Pop()) Task=200;`) also completes in
//      that first tick offline, so the cursor goes 1 -> 200 in ONE call and the
//      value 100 is never observable between ticks.  That is why 100 is NOT in
//      the wanted set below -- its execution is proven by the 1 -> 200 jump.
//      (This is exactly what the first run of this rewritten test caught: an
//      earlier draft wanted 100 in the set and went red.)
//
//      AI(W906-W7-L1fix3) 20260801: BELT POLARITY IS NOW ASSERTED HERE TOO.  Case
//      200 runs the belt FORWARD with CW OFF (golden :346-347) -- an inverted pair
//      reverses the Auto2 belt on real hardware, and it used to be invisible
//      (`.On()/.Off()` -> `.Off()/.On()` left the suite at 46 passed, 0 failed).
//      Unlike DoAuto2TrayToFront case 100, this pair IS observable at a tick
//      boundary: case 200 leaves via the DUMMY-timeout arm (golden :354-360) with
//      a plain `break`, so the outputs survive to the end of the tick that lands
//      the cursor on 450.  Sampled exactly there.
// ---------------------------------------------------------------------------
static void test_tray_to_rear_path()
{
    printf("\n[5] DoAuto2TrayToRear() path 200,450,460,500 (100 runs in tick 1)\n");

    setupDummyFeed();
    InitAuto2TrayToRearTask();
    MOT[MMAuto2].ClearTray("test_w7_l1_auto2");
    MOT[MMAuto2_Car].SetTray(NULL_IC, "test_w7_l1_auto2");

    // Pre-drive both belt outputs to the OPPOSITE of what case 200 must write
    // (golden :346 ON, :347 OFF), so an inversion AND a deletion both go red.
    SW[SwACAuto2].Off();
    SW[SwACAuto2CW].On();

    StateSet seen;
    bool done  = false;
    int  steps = 0;
    int  afterFirstTick = -1;
    int  beltMainAt450  = -1;   // SW[SwACAuto2].OutValue   @ cursor 450
    int  beltCWAt450    = -1;   // SW[SwACAuto2CW].OutValue @ cursor 450
    for (steps = 0; steps < 200 && !done; ++steps)
    {
        advanceAuto2Time();
        done = DoAuto2TrayToRear();
        seen.insert(iAuto2TrayToRearTask);
        if (steps == 0) afterFirstTick = iAuto2TrayToRearTask;
        if (iAuto2TrayToRearTask == 450 && beltMainAt450 < 0)
        {
            beltMainAt450 = SW[SwACAuto2].OutValue   ? 1 : 0;
            beltCWAt450   = SW[SwACAuto2CW].OutValue ? 1 : 0;
        }
    }

    static const int want[] = {200, 450, 460, 500};
    const int nWant = (int)(sizeof(want)/sizeof(want[0]));

    CHECK(afterFirstTick == 200,
          "GOLDEN QUIRK pin: case 1 fell through into case 100 AND case 100 ran (cursor 1 -> 200)");
    CHECK(done, "DoAuto2TrayToRear() returns true (converged)");
    CHECK(covers(seen, want, nWant), "trajectory covers 200,450,460,500");
    CHECK(MOT[MMAuto2].fHasTray == true && MOT[MMAuto2_Car].fHasTray == false,
          "case 500 moved the tray MMAuto2_Car -> MMAuto2 (golden :396-397)");
    CHECK(beltMainAt450 == 1 && beltCWAt450 == 0,
          "case 200 drove the belt ON with CW OFF while pulling the tray back (golden :346-347)");
    CHECK(SW[SwACAuto2].OutValue == false,
          "case 460 stopped the belt once the tray settled (golden :389)");
    printSeen("DoAuto2TrayToRear", seen);
    if (!covers(seen, want, nWant)) printMissing("DoAuto2TrayToRear", seen, want, nWant);
    printf("    (belt@450 main=%d cw=%d)\n", beltMainAt450, beltCWAt450);
}

// ---------------------------------------------------------------------------
//  [6] DoUnLoadNewAuto2ToStack() (golden :600-706) driven DIRECTLY.
//      This is the function the old test claimed to drive and never entered.
//
//      AI(W906-W7-L1fix3) 20260801: case 50 also STOPS BOTH BELT OUTPUTS before
//      handing to 100 (golden :622-623) -- the third and last belt-command site in
//      this file, and unasserted until now.  Both outputs are pre-driven ON below
//      so the stop is a transition the SM made, not the state it inherited.
// ---------------------------------------------------------------------------
static void test_unload_to_stack_direct()
{
    printf("\n[6] DoUnLoadNewAuto2ToStack() direct: 50,100,150,200,300,310,400,401,402\n");

    setupDummyFeed();
    InitUnLoadNewAuto2TrayTask();
    MOT[MMAuto2].SetTray(NULL_IC, "test_w7_l1_auto2");
    MOT[MMAuto2_Car].SetTray(NULL_IC, "test_w7_l1_auto2");
    // case 402 must NOT see both stack sensors ON, or it raises JAM1113 forever.
    simSensorOff(SnAuto2PreDete);
    simSensorOff(SnAuto2_Tray_Car);
    fAuto2CanSupplyNewTray = false;          // case 402 sets it true on success

    // Pre-drive both belt outputs ON -- the opposite of what case 50 must write.
    SW[SwACAuto2].On();
    SW[SwACAuto2CW].On();

    StateSet seen;
    bool done  = false;
    int  steps = 0;
    int  beltAt100 = -1;    // both belt outputs OFF? sampled @ cursor 100
    for (steps = 0; steps < 200 && !done; ++steps)
    {
        advanceAuto2Time();
        done = DoUnLoadNewAuto2ToStack();
        seen.insert(iUnLoadNewAuto2TrayTask);
        if (iUnLoadNewAuto2TrayTask == 100 && beltAt100 < 0)
            beltAt100 = (SW[SwACAuto2].OutValue == false &&
                         SW[SwACAuto2CW].OutValue == false) ? 1 : 0;
    }

    static const int want[] = {50, 100, 150, 200, 300, 310, 400, 401, 402};
    const int nWant = (int)(sizeof(want)/sizeof(want[0]));

    CHECK(done, "DoUnLoadNewAuto2ToStack() returns true (converged)");
    CHECK(covers(seen, want, nWant),
          "trajectory covers 50,100,150,200,300,310,400,401,402");
    CHECK(MOT[MMAuto2].fHasTray == false,
          "case 310 cleared MMAuto2 (golden :659)");
    CHECK(fAuto2CanSupplyNewTray == true,
          "case 402 re-opened the supply gate on success (golden :700)");
    CHECK(beltAt100 == 1,
          "case 50 stopped BOTH belt outputs before handing to 100 (golden :622-623)");
    CHECK(iUnLoadNewAuto2TrayTask == 1, "cursor reset to 1 on convergence");
    printSeen("DoUnLoadNewAuto2ToStack", seen);
    if (!covers(seen, want, nWant)) printMissing("DoUnLoadNewAuto2ToStack", seen, want, nWant);
    printf("    (bothBeltsOff@100=%d)\n", beltAt100);
}

// ---------------------------------------------------------------------------
//  [7] DoAuto2Receive() full chain -> DoAuto2TrayToFront -> hand-off ->
//      DoUnLoadNewAuto2ToStack -> car cleared, receive mode exited.
//
//      SUPPORT_2_EMPTY_EMPTY=true is REQUIRED to reach the hand-off, and this is
//      a real property of the golden, not a test convenience: it is the ONLY
//      DoAuto2TrayToFront exit that returns true with a tray still ON the car
//      (golden :208 SetTray).  Every other success arm (case 105 :304, the DUMMY
//      timeout arm :237, the JAM1212 skip arm :260) CLEARS MMAuto2_Car first, and
//      DoAuto2Receive case 100 only hands off `if(MOT[MMAuto2_Car].fHasTray)`
//      (golden :728) -- so with SUPPORT_2_EMPTY_EMPTY==false the unload branch of
//      DoAuto2Receive is unreachable, which is why the old test sat at
//      iAuto2ReceiveTask==100 for all 1000 iterations.
// ---------------------------------------------------------------------------
static void test_receive_chain_converges()
{
    printf("\n[7] DoAuto2Receive() chain -> DoUnLoadNewAuto2ToStack hand-off\n");

    setupDummyFeed();
    InitAuto2ReceiveTask();
    InitAuto2TrayToFrontTask();
    InitUnLoadNewAuto2TrayTask();
    SUPPORT_2_EMPTY_EMPTY = true;            // see banner above
    MOT[MMAuto2_Car].SetTray(NULL_IC, "test_w7_l1_auto2");
    MOT[MMAuto2].ClearTray("test_w7_l1_auto2");
    simSensorOff(SnAuto2PreDete);
    simSensorOff(SnAuto2_Tray_Car);
    iReceiveAuto2Tray = 1;                   // receive mode active

    StateSet seenRecv, seenUnload;
    bool handedOff = false, cleared = false;
    int  steps = 0;
    for (steps = 0; steps < 200 && !cleared; ++steps)
    {
        advanceAuto2Time();
        DoAuto2Receive();
        seenRecv.insert(iAuto2ReceiveTask);
        seenUnload.insert(iUnLoadNewAuto2TrayTask);
        if (iAuto2ReceiveTask == 200)
            handedOff = true;
        if (handedOff && iAuto2ReceiveTask == 1 && !MOT[MMAuto2_Car].fHasTray)
            cleared = true;
    }

    static const int wantUnload[] = {50, 100, 150, 200, 300, 310, 400, 401, 402};
    const int nWantUnload = (int)(sizeof(wantUnload)/sizeof(wantUnload[0]));

    CHECK(handedOff,
          "DoAuto2Receive reached case 200 (front-feed handed the car tray to unload)");
    CHECK(covers(seenUnload, wantUnload, nWantUnload),
          "DoUnLoadNewAuto2ToStack walked its full trajectory INSIDE the chain");
    CHECK(cleared, "chain converged: MMAuto2_Car cleared and cursor back to 1");
    CHECK(iReceiveAuto2Tray == 0, "receive mode exited (iReceiveAuto2Tray==0, golden :744)");
    printSeen("DoAuto2Receive", seenRecv);
    printSeen("DoUnLoadNewAuto2ToStack (in chain)", seenUnload);
    if (!covers(seenUnload, wantUnload, nWantUnload))
        printMissing("DoUnLoadNewAuto2ToStack (in chain)", seenUnload, wantUnload, nWantUnload);
    printf("    (converged after %d steps)\n", steps);
    SUPPORT_2_EMPTY_EMPTY = false;           // restore for later sub-tests
}

// ---------------------------------------------------------------------------
//  [8] DoAuto2() dispatcher -- four scenarios.
//
//  Scenario A: the feed walk.  The gate is re-armed every tick because case 70
//      deliberately closes it (golden :548) once the stage tray is latched; a
//      real machine re-opens it from DoUnLoadNewAuto2ToStack case 402.
//  Scenario B: at case 100, an EMPTY stage + empty car takes the feed arm
//      (Task=200 -> DoLoadNewAuto2TrayToCar -> iReceiveAuto2Tray=1, golden :581).
//  Scenario C: at case 100, a car that HAS a tray takes the recycle arm
//      (Task=300 -> DoAuto2TrayToRear -> Task=50, golden :588).
//  Scenario D: at case 60, a stage that HAS a tray takes the re-check arm
//      (Task=65) and, with MMTrayY holding an empty tray, fires the
//      DoAuto2Receive hand-off (golden :532-533).
//
//  THE THIRD GOLDEN FALL-THROUGH IS NOT PINNED HERE, ON PURPOSE.  DoAuto2 case 1
//  also falls through (into case 2, golden :433) but -- unlike the two pinned in
//  [4]/[5] -- that fall-through is behaviourally INERT and therefore not
//  observable: case 1's only action is `DoAuto2Delay.SetMSAndOn(1000); Task=2;`
//  and case 2's only action is gated on `DoAuto2Delay.Off()`, which case 1 just
//  re-armed to 1000 ms, so case 2's body can never do anything in that same tick.
//  Adding a `break;` after golden :432 would be unobservable from outside.  Stated
//  here rather than asserted, because a test that cannot fail is worse than none.
// ---------------------------------------------------------------------------
static void test_drive_auto2()
{
    printf("\n[8] DoAuto2() dispatcher -- 4 scenarios over the documented switch\n");

    // ---- Scenario A ------------------------------------------------------
    setupDummyFeed();
    InitAuto2Task();
    MOT[MMAuto2].ClearTray("test_w7_l1_auto2");
    MOT[MMAuto2_Car].ClearTray("test_w7_l1_auto2");
    MOT[MMTrayY].ClearTray("test_w7_l1_auto2");
    MOT[MTrayX].ClearTray("test_w7_l1_auto2");

    StateSet seenA;
    bool cursorAlwaysSane = true;
    for (int i = 0; i < 200; ++i)
    {
        advanceAuto2Time();
        fAuto2CanSupplyNewTray = true;      // re-arm the gate (see banner)
        DoAuto2();
        seenA.insert(iAutoAuto2Task);
        if (!auto2CursorSane(iAutoAuto2Task)) cursorAlwaysSane = false;
    }
    static const int wantA[] = {1, 2, 3, 20, 30, 40, 50, 60, 70, 100, 150};
    const int nWantA = (int)(sizeof(wantA)/sizeof(wantA[0]));
    CHECK(cursorAlwaysSane, "DoAuto2 cursor stayed within the documented state set");
    CHECK(covers(seenA, wantA, nWantA),
          "scenario A trajectory covers 1,2,3,20,30,40,50,60,70,100,150");
    printSeen("DoAuto2 scenario A", seenA);
    if (!covers(seenA, wantA, nWantA)) printMissing("DoAuto2 scenario A", seenA, wantA, nWantA);

    // ---- Scenario B: case 100 -> 200 feed arm -----------------------------
    setupDummyFeed();
    InitAuto2Task();
    InitLoadNewAuto2TrayToCarTask();
    MOT[MMAuto2].ClearTray("test_w7_l1_auto2");
    MOT[MMAuto2_Car].ClearTray("test_w7_l1_auto2");
    MOT[MMTrayY].ClearTray("test_w7_l1_auto2");
    MOT[MTrayX].ClearTray("test_w7_l1_auto2");
    simSensorOff(SnAuto2PreDete);           // let the fed tray take the lifter walk
    simSensorOff(SnAuto2_Tray_Car);

    StateSet seenB;
    bool reached200 = false;
    for (int i = 0; i < 400 && !reached200; ++i)
    {
        advanceAuto2Time();
        fAuto2CanSupplyNewTray = true;
        // Model the tray being consumed off the stage: case 100 only takes its
        // feed arm while the stage is empty (golden :564-570 guard).
        if (iAutoAuto2Task == 100)
            MOT[MMAuto2].ClearTray("test_w7_l1_auto2");
        DoAuto2();
        seenB.insert(iAutoAuto2Task);
        if (iAutoAuto2Task == 200) reached200 = true;
    }
    CHECK(reached200, "scenario B reached case 200 (case-100 feed arm, golden :562-563)");
    // Let case 200 run DoLoadNewAuto2TrayToCar to completion.
    for (int i = 0; i < 200 && iReceiveAuto2Tray == 0; ++i)
    {
        advanceAuto2Time();
        fAuto2CanSupplyNewTray = true;
        DoAuto2();
        seenB.insert(iAutoAuto2Task);
    }
    CHECK(iReceiveAuto2Tray == 1,
          "case 200 consumed DoLoadNewAuto2TrayToCar()==true -> iReceiveAuto2Tray=1 (golden :581)");
    printSeen("DoAuto2 scenario B", seenB);

    // ---- Scenario C: case 100 -> 300 recycle arm --------------------------
    setupDummyFeed();
    InitAuto2Task();
    InitAuto2TrayToRearTask();
    MOT[MMAuto2].ClearTray("test_w7_l1_auto2");
    MOT[MMAuto2_Car].ClearTray("test_w7_l1_auto2");
    MOT[MMTrayY].ClearTray("test_w7_l1_auto2");
    MOT[MTrayX].ClearTray("test_w7_l1_auto2");

    StateSet seenC;
    bool reached300 = false;
    // AI(W906-W7-L1fix2) 20260731: OBSERVE THE TRANSITION, NOT SET MEMBERSHIP.
    // This assertion used to be `seenC.find(50) != seenC.end()`, which was a
    // TAUTOLOGY: `Task=50` is assigned in THREE places in DoAuto2 (ported
    // asendic_Auto2.cpp:620 and :633, both inside case 40, plus :731 in case 300),
    // and the fixture walks case 40 many ticks BEFORE case 300 is ever entered --
    // so 50 was in seenC by construction.  Proven both ways this session: with
    // case 300's own `Task=50` retargeted to `Task=1` the old assertion still
    // PASSED.  A cursor-transition edge (300 -> 50) is what case 300 actually
    // promises, and it is observable only by sampling the cursor on BOTH sides of
    // the DoAuto2() call.
    bool saw300to50 = false;
    for (int i = 0; i < 400; ++i)
    {
        advanceAuto2Time();
        fAuto2CanSupplyNewTray = true;
        // A tray sitting on the car at case 100 selects the recycle arm.
        if (iAutoAuto2Task == 100)
            MOT[MMAuto2_Car].SetTray(NULL_IC, "test_w7_l1_auto2");
        int prevC = iAutoAuto2Task;          // cursor BEFORE the tick
        DoAuto2();
        seenC.insert(iAutoAuto2Task);
        if (iAutoAuto2Task == 300) reached300 = true;
        if (prevC == 300 && iAutoAuto2Task == 50) saw300to50 = true;
    }
    CHECK(reached300, "scenario C reached case 300 (case-100 recycle arm, golden :557-558)");
    CHECK(saw300to50,
          "case 300 consumed DoAuto2TrayToRear()==true and the cursor stepped 300 -> 50 in ONE tick (golden :588)");
    printSeen("DoAuto2 scenario C", seenC);

    // ---- Scenario D: case 60 -> 65 re-check + receive hand-off ------------
    setupDummyFeed();
    InitAuto2Task();
    MOT[MMAuto2].ClearTray("test_w7_l1_auto2");
    MOT[MMAuto2_Car].ClearTray("test_w7_l1_auto2");
    MOT[MMTrayY].SetTray(NULL_IC, "test_w7_l1_auto2");      // tray, no IC
    MOT[MTrayX].ClearTray("test_w7_l1_auto2");
    CHECK(MOT[MMTrayY].fHasTray && MOT[MMTrayY].HasIC() == false,
          "scenario D precondition: MMTrayY holds an empty tray");

    StateSet seenD;
    bool reached65 = false;
    for (int i = 0; i < 400 && !reached65; ++i)
    {
        advanceAuto2Time();
        fAuto2CanSupplyNewTray = true;
        // case 60 takes the re-check arm only when the stage HAS a tray (golden
        // :509-513), and the hand-off at 65 additionally wants a tray on the car
        // (golden :527-533).
        if (iAutoAuto2Task == 60)
        {
            MOT[MMAuto2].SetTray(NULL_IC, "test_w7_l1_auto2");
            MOT[MMAuto2_Car].SetTray(NULL_IC, "test_w7_l1_auto2");
        }
        DoAuto2();
        seenD.insert(iAutoAuto2Task);
        if (iAutoAuto2Task == 65) reached65 = true;
    }
    CHECK(reached65, "scenario D reached case 65 (case-60 re-check arm, golden :511-512)");
    // one more tick to run the case-65 body
    advanceAuto2Time();
    fAuto2CanSupplyNewTray = true;
    DoAuto2();
    seenD.insert(iAutoAuto2Task);
    CHECK(iReceiveAuto2Tray == 2 && iAuto2ReceiveTask == 200,
          "case 65 fired the receive hand-off (iReceiveAuto2Tray=2, iAuto2ReceiveTask=200, golden :532-533)");
    printSeen("DoAuto2 scenario D", seenD);

    // leave the SM in a clean state for [9]
    setupDummyFeed();
    InitAuto2Task();
    InitAuto2ReceiveTask();
    MOT[MMAuto2].ClearTray("test_w7_l1_auto2");
    MOT[MMAuto2_Car].ClearTray("test_w7_l1_auto2");
    MOT[MMTrayY].ClearTray("test_w7_l1_auto2");
}

// ---------------------------------------------------------------------------
//  [9] fAuto2CanSupplyNewTray gate: DoAuto2() no-ops while closed
// ---------------------------------------------------------------------------
static void test_supply_gate()
{
    printf("\n[9] fAuto2CanSupplyNewTray gate\n");

    setupDummyFeed();
    InitAuto2Task();
    iReceiveAuto2Tray      = 0;
    fAuto2CanSupplyNewTray = false;   // gate closed

    int before = iAutoAuto2Task;
    for (int i = 0; i < 20; ++i)
    {
        advanceAuto2Time();
        DoAuto2();
    }
    CHECK(iAutoAuto2Task == before,
          "DoAuto2 no-ops while fAuto2CanSupplyNewTray==false (early return, golden :425-426)");
}

// ---------------------------------------------------------------------------
//  [10] fAGV satellite links and answers (DoAuto2 case 70 derefs
//       fAGV->IsATK_AMR(), golden :543).
//
//  AI(W906-W7-L1fix2) 20260731: THIS WAS MISLABELLED A "STUB" AND IS NOT ONE.
//  forms/fAGV.cpp:36 is `bool TfAGV::IsATK_AMR() { return AGV_IsATK_AMR(); }` --
//  a one-line delegation to REAL TRANSLATED CODE in
//  Automation/AGV_predicates.cpp:37-47, which evaluates
//  `USE_COVER_TRAYID==tCID_NFC && IniConfig.bA65_BundleIDList && CUSTOMER_CODE==
//  CC_AMKOR_Korea`.  It returns false here because those globals hold their
//  untouched defaults (USE_COVER_TRAYID==tCIDNotUse, cmydef.cpp:5722;
//  CUSTOMER_CODE==0, cmydef.cpp:3412) -- NOT because the body is hardcoded false.
//  What IS a no-op stub in this file's dependency set is the AutoCylinder* trio;
//  see the STAND-IN note in asendic_Auto2.cpp's banner.  The first CHECK below is
//  a construction/link smoke check and claims no more than that.
// ---------------------------------------------------------------------------
static void test_fagv_predicate()
{
    printf("\n[10] FormsFacade fAGV predicate (DoAuto2 case 70 dependency)\n");
    CHECK(fAGV != 0,                 "fAGV is constructed (non-null)");
    CHECK(fAGV->IsATK_AMR() == false,
          "fAGV->IsATK_AMR()==false -- real predicate (AGV_predicates.cpp:37) on default globals");
}

// ---------------------------------------------------------------------------
//  [12] THE ARGUMENT-ORDER PIN -- AI(W906-W7-L1-Wave3) 20260802.
//
//  AutoCylinderUp / AutoCylinderMiddle / AutoCylinderLower take
//      (int Part, int CylinderName, int CylinderNameMid, bool bReset=false)
//  and parameters 2 and 3 are NOT symmetric in golden's real bodies
//  (asendic.cpp):
//    * CylinderName    (param 2) is commanded UNCONDITIONALLY at case 1 / case 50.
//    * CylinderNameMid (param 3) is Enable-guarded, and is the SOLE input to
//      AutoCylinderMidIsOn -- the "arrived at the top" witness that case 100 and
//      case 201 consult.
//  Golden callers DISAGREE about which physical cylinder goes in which slot, and
//  none of that may be "normalised" (see the warning above the declarations in
//  asendic.h).  THIS FILE is the extreme case: asendic_Auto2.cpp passes
//  (C_Auto2_Selector, C_Auto2_Up) -- the REVERSE of asendic_Auto.cpp /
//  asendic_Auto_RT.cpp / csystem.cpp / acatchtray.cpp -- at ALL TWELVE of its
//  sites (golden :65, :85, :101, :106, :132, :137, :617, :620, :638, :643, :674,
//  :679).
//
//  WHY THIS PIN EXISTS.  Until Wave 3 the three functions were
//  `{ return true; }` stubs in acatchtray_shims.cpp.  Under a stub the argument
//  order is INVISIBLE: every call succeeds instantly whatever you pass, so no
//  test in this repository could tell a correct call site from a swapped one, and
//  a well-meaning "consistency" edit to those twelve lines would have shipped
//  silently and driven the OPPOSITE PHYSICAL CYLINDER on a real machine.  The
//  real bodies close that window; this sub-test is the alarm on it.
//
//  WHAT IT ASSERTS, and what it deliberately does NOT.  One physical stack is
//  wired (the plant in tests/w3_cylinder_plant.h, registered by setupDummyFeed()
//  in the SAME order asendic_Auto2.cpp calls with), and then:
//    [12a] all three functions converge in the golden order;
//    [12b] AutoCylinderUp does NOT converge when the two arguments trade places,
//          and raises the "Lifter Up error" operator dialog that the correct order
//          never raises;
//    [12c] AutoCylinderMiddle drives the OPPOSITE PHYSICAL CYLINDER when the
//          arguments trade places.  This is the sharpest form of the claim and the
//          one that matters on real metal: golden's CylinderName is driven ON and
//          CylinderNameMid OFF, so a swapped site extends the lock and retracts
//          the lift instead of the other way round;
//    [12d] AutoCylinderMidIsOn answers about its OWN argument -- the mechanical
//          root of [12b] and [12c].
//  MEASURED AND NOT CLAIMED: with the arguments swapped, AutoCylinderMiddle and
//  AutoCylinderLower still RETURN TRUE under this stack's sensor law.  Their
//  non-ART guards happen to be satisfiable from both sides; only the CYLINDER
//  IDENTITY distinguishes them, which is exactly why [12c] asserts identity
//  rather than convergence.  Stating this is the point -- "the swapped call also
//  converges" is precisely the property that makes a swapped site dangerous.
//
//  MUTATION-PROVEN: swapping the arguments at golden call site
//  asendic_Auto2.cpp:85 (DoLoadNewAuto2TrayToCar case 50's AutoCylinderUp) makes
//  sub-test [3a] go red as well, because the fixture wiring and the call site then
//  disagree.
// ---------------------------------------------------------------------------
static bool pinDrive(int which, int Part, int p2, int p3, int budget, int *ticksOut)
{
    //  which: 0 = Up, 1 = Middle, 2 = Lower
    bool ok = false;
    int  t  = 0;
    for (; t < budget && !ok; ++t)
    {
        advanceAuto2Time();                 // plant tick + AutoTime[][] expiry
        if      (which == 0) ok = AutoCylinderUp    (Part, p2, p3);
        else if (which == 1) ok = AutoCylinderMiddle(Part, p2, p3);
        else                 ok = AutoCylinderLower (Part, p2, p3);
    }
    if (ticksOut) *ticksOut = t;
    return ok;
}

static void test_argument_order_pin()
{
    printf("\n[12] ARGUMENT-ORDER PIN: AutoCylinderUp/Middle/Lower params 2 and 3\n");

    const int  Part  = 1;                   // Auto2's Part, golden asendic_Auto2.cpp
    const int  kName = C_Auto2_Selector;    // parameter 2, AS ASENDIC_AUTO2.CPP PASSES IT
    const int  kMid  = C_Auto2_Up;          // parameter 3, AS ASENDIC_AUTO2.CPP PASSES IT
    const int  kBudget = 80;                // ~16x the measured requirement
    int ticks = 0;

    // --- [12a] the golden order converges, for all three --------------------
    setupDummyFeed();  initAutoTask();  W906_ShowMyMessage_Reset();
    const bool upOk       = pinDrive(0, Part, kName, kMid, kBudget, &ticks);
    const int  dlgsGolden = W906_ShowMyMessage_Count;
    CHECK(upOk, "[12a] AutoCylinderUp(Part, C_Auto2_Selector, C_Auto2_Up) -- asendic_Auto2.cpp's OWN "
                "argument order -- CONVERGES");
    printf("    (golden order: AutoCylinderUp converged in %d ticks, %d dialogs)\n", ticks, dlgsGolden);

    setupDummyFeed();  initAutoTask();
    CHECK(pinDrive(1, Part, kName, kMid, kBudget, 0),
          "[12a] AutoCylinderMiddle in the same order CONVERGES");

    setupDummyFeed();  initAutoTask();
    CHECK(pinDrive(2, Part, kName, kMid, kBudget, 0),
          "[12a] AutoCylinderLower in the same order CONVERGES");

    // --- [12b] AutoCylinderUp: the swapped order cannot succeed -------------
    //  Same machine, same wiring, same budget -- only the two arguments trade
    //  places, exactly as a "tidy these call sites" edit would do.
    setupDummyFeed();  initAutoTask();  W906_ShowMyMessage_Reset();
    const bool upSwapped   = pinDrive(0, Part, kMid, kName, kBudget, 0);
    const int  dlgsSwapped = W906_ShowMyMessage_Count;
    CHECK(upSwapped == false,
          "[12b] SWAPPING parameters 2 and 3 makes AutoCylinderUp NEVER return true -- the two are "
          "NOT interchangeable");
    CHECK(dlgsSwapped > 0 && dlgsGolden == 0,
          "[12b] ...and the swapped run raises golden's \"Lifter Up error\" (asendic.cpp case 100) "
          "which the CORRECT order never raises -- 0 dialogs vs many, on identical hardware");
    CHECK(iAutoTask[0][Part] == 1,
          "[12b] the swapped run is left parked back on cursor 1 (case 100's alarm arm resets it, "
          "golden asendic.cpp:669-671)");

    // --- [12c] AutoCylinderMiddle drives the OPPOSITE cylinder --------------
    //  golden asendic.cpp CylinderMiddle-for-Auto: case 1 does
    //      Cylinder[CylinderName].On();
    //      if(Cylinder[CylinderNameMid].Enable) Cylinder[CylinderNameMid].Off();
    //  so which cylinder EXTENDS and which RETRACTS is decided purely by the
    //  argument order.  Both runs below converge; only the metal differs.
    setupDummyFeed();  initAutoTask();
    pinDrive(1, Part, kName, kMid, kBudget, 0);
    CHECK(Cylinder[kName].bCylinderOn == true && Cylinder[kMid].bCylinderOn == false,
          "[12c] golden order: AutoCylinderMiddle EXTENDED C_Auto2_Selector (param 2) and RETRACTED "
          "C_Auto2_Up (param 3)");

    setupDummyFeed();  initAutoTask();
    pinDrive(1, Part, kMid, kName, kBudget, 0);
    CHECK(Cylinder[kMid].bCylinderOn == true && Cylinder[kName].bCylinderOn == false,
          "[12c] SWAPPED: the SAME call drove the OPPOSITE pair -- C_Auto2_Up extended, "
          "C_Auto2_Selector retracted.  On a real machine that is the wrong metal moving, and it "
          "still returns true, so nothing downstream notices");

    // --- [12d] the mechanical root: AutoCylinderMidIsOn reads its argument --
    //  golden asendic.cpp:529-560.  Drive the stack to "lift extended, selector
    //  retracted" and ask the predicate about each cylinder in turn.
    setupDummyFeed();  initAutoTask();
    Cylinder[kMid].On();                    // C_Auto2_Up       commanded ON
    Cylinder[kName].Off();                  // C_Auto2_Selector commanded OFF
    w3::Tick();                             // let the plant answer
    // AI(W906-W7-L1-W3fixB) 20260802: STRENGTHENED.  This was one assertion,
    // `AutoCylinderMidIsOn(Part, kMid) != AutoCylinderMidIsOn(Part, kName)`, which
    // pins only that the two answers DIFFER -- a defect that inverted BOTH would
    // have kept it green, in the sub-test whose whole job is to be the alarm on a
    // silent swap.  Split into two one-sided `==` assertions: strictly stronger,
    // same fixture, no extra cost.  Each is mutation-proven independently.
    CHECK(AutoCylinderMidIsOn(Part, kMid) == true,
          "[12d] AutoCylinderMidIsOn(Part, C_Auto2_Up) is TRUE -- C_Auto2_Up is the cylinder that "
          "was commanded ON, and the predicate reads ITS OnStatus (golden asendic.cpp:529-560)");
    CHECK(AutoCylinderMidIsOn(Part, kName) == false,
          "[12d] AutoCylinderMidIsOn(Part, C_Auto2_Selector) is FALSE in the SAME state -- so which "
          "cylinder lands in parameter 3 decides what 'arrived at the top' means, and an inversion "
          "of BOTH answers (which the old `!=` form would have accepted) goes red here");

    setupDummyFeed();  initAutoTask();
}

int main()
{
    printf("==============================================================\n");
    printf(" W7-L1 -- asendic_Auto2 (Auto2-tray stack feed) SM test\n");
    printf("==============================================================\n");

    test_inits();
    test_virtual_time_seam();
    test_load_new_auto2_full_lifter_walk();
    test_load_new_auto2_case1_shortcircuit();
    test_tray_to_front_long_path();
    test_tray_to_front_belt_polarity();
    test_tray_to_rear_path();
    test_unload_to_stack_direct();
    test_receive_chain_converges();
    test_drive_auto2();
    test_supply_gate();
    test_fagv_predicate();
    test_argument_order_pin();      // AI(W906-W7-L1-Wave3) 20260802

    printf("\n--------------------------------------------------------------\n");
    printf(" RESULT: %d passed, %d failed\n", g_pass, g_fail);
    printf("--------------------------------------------------------------\n");
    return (g_fail == 0) ? 0 : 1;
}
