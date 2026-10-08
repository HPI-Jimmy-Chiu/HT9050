// =============================================================================
//  test_w7_l2_ckernel.cpp  --  W7-L2 regression suite for ported ckernel.cpp
//
//  Unit under test : HT9011UC_Cpp_V3.33.906.0/ckernel.cpp  (2958 lines,
//                    17 free functions, source of ht9045_sm)
//  Golden reference: HT9011UC_Code_V3.33.906.0_20260618/ckernel.cpp
//                    (2589 lines, BCB6 / cp950, READ-ONLY)
//  Author          : AI(W906-W7-L2) 20260803
//
//  WHY THIS FILE EXISTS
//  --------------------
//  The W7-L2 translation landed, compiled, and had ZERO tests.  In an earlier
//  wave of this same project a suite was delivered that compiled, ran, printed
//  PASS and asserted essentially nothing; the DEVLOG records it as "the
//  translation was right, the test was empty".  This file is written against
//  that failure mode.  EVERY expected value below was derived by READING GOLDEN
//  -- decoded with cp950, line by line -- and every one carries its
//  `// golden ckernel.cpp:NNNN` citation.  Not one expectation was obtained by
//  running the port and recording what came out.
//
//  THIS PROJECT PRESERVES GOLDEN'S BUGS ON PURPOSE.  So most of what follows
//  pins DEFECTS.  A "fix" to any of them must fail this suite loudly.
//
// -----------------------------------------------------------------------------
//  WHAT IS PROVEN, AND THE GOLDEN LINE THAT SAYS SO
// -----------------------------------------------------------------------------
//  [P1] PART C -- THE HEADLINE BUG.  ScanPannelKey's rear Power-Off / Power-On
//       keys DOUBLE-OFFSET their press-latch slot and therefore AUTO-REPEAT
//       while physically held.
//         golden :2381-2383  p = ret;  if(bFrontPadActive==false) p += SnRKPowerOff;
//         golden :2385-2393  if(bK[p]==false){ bK[p]=true; return ret; } else ret=-1;
//         golden :2395-2404  for(i=0..31) if(bK[i] && Sen[i].IsOn()==false) bK[i]=false;
//       The rear arm answers `ret = SnRKPowerOff` (golden :2174-2177), an id that
//       is ALREADY rear, so the +14 lands a SECOND time:
//         SnRKPowerOff   = 14  (golden cmydef.cpp:804, ported cmydef.cpp:808)
//         SnRKManualStep = 28  (golden cmydef.cpp:819, ported cmydef.cpp:823)
//         14 + 14 = 28
//       The latch is written at bK[28]; the release sweep then asks
//       Sen[28].IsOn() -- the MANUAL-STEP key, a completely different button --
//       finds it off, and clears the latch.  The Power-Off key re-fires on the
//       next-but-one call with no release in between.
//       PROVED TWO-SIDEDLY, which is what makes it a proof rather than an
//       observation:
//         C1  rear Power-Off HELD  -> 14, -1, 14, -1, 14 ......... it re-fires.
//             If the slot were bK[14] the sweep would find Sen[14] ON (it is the
//             key being held) and keep the latch, so this sequence ALONE rules
//             out slot 14.
//         C2  rear Power-Off HELD *and* Sen[SnRKManualStep] also held ON
//                                  -> 14, -1, -1, -1 ............ it STOPS.
//             Nothing in ScanPannelKey reads sensor 28 (verified: the rear arm
//             polls 16,17,3,18,19,20,14,15,21,22,23,24,25,26,12 and nothing
//             else).  The ONLY way holding an unrelated key can suppress the
//             Power-Off re-fire is if the release decision for this latch is
//             taken on Sen[28].  C1 + C2 together identify the slot as exactly
//             28 -- no weaker reading survives both.
//         C3  FRONT Power-Off HELD -> 0, -1, -1, -1 ..... correctly latched.
//             SnFKPowerOff = 0, bFrontPadActive true so NO offset, bK[0] swept
//             against Sen[0], which IS on.  This is the contrast that shows the
//             offset is the cause and not something incidental to Power-Off.
//         C4  front Power-Off RELEASED then re-pressed -> fires again, so C3 is
//             not the vacuous "this key can only ever fire once".
//         C5  rear Power-ON repeats the whole shape one slot up: 15 + 14 = 29
//             == SnRKManualTStart (golden cmydef.cpp:820).
//
//  [P2] PART B -- the REAR Start key answers with the FRONT id.
//         golden :2158-2161  if(Sen[SnRKStart].IsOn() || bAseStart) ret=SnFKStart;
//       SnRKStart = 19, SnFKStart = 5 (ported cmydef.cpp:798/813 -- byte-identical
//       to golden cmydef.cpp:794/809, diffed this pass).  A rear Start press is
//       reported as 5, NEVER 19.  Compare golden :1981-1984, which produces the
//       same 5 from the FRONT key -- B2 pins that both keys answer 5.
//       B3 then pins WHY the +14 exists at all: for a front id the offset lands
//       on the MATCHING rear sensor (5+14 == 19 == SnRKStart), so the latch
//       holds while the key is held -- 5, -1, -1 -- the exact opposite of C1.
//       That contrast is the whole story of P1 in two assertions.
//
//       NOTE ON A WRONG NUMBER IN THE BRIEF, recorded rather than quietly used:
//       the wave brief states SnFKStart == 6.  It is 5.  Six is
//       `epn_SnFKStart` from golden uPadInterface.h:171, a DIFFERENT enum -- the
//       serial pad-protocol index, whose numbering is shifted because it carries
//       an epn_SnFrontPadActive=2 slot that the Sen[] map does not.  ckernel.cpp
//       indexes Sen[] and therefore uses the cmydef.cpp constants.  Verified by
//       reading both files this pass.  (epn_SnRKPowerOff=14 and
//       epn_SnRKManualStep=28 happen to agree with cmydef.cpp, which is why the
//       P1 arithmetic in the brief is right even though its P2 id is not.)
//
//  [P3] PART D -- WaitManualStepKey and WaitManualStartKey answer with OPPOSITE
//       SIGNS on a structurally IDENTICAL condition.
//         golden :56-57  if(fContact->fShow && ...Checked==true) return true;
//         golden :98-99  if(fContact->fShow && ...Checked==true) return false;
//       D1 drives ONE fixture state through BOTH functions and asserts they
//       DISAGREE.  D3 turns the one-touch mode off and asserts they now AGREE
//       (both false) -- so the disagreement is caused by that early-out and
//       nothing else.  D2 proves the answers really came from :56-57 / :98-99
//       and not from the polling bodies, by showing the bodies' FIRST side
//       effect (`bLampManualSetp = FlushFlag`, golden :59 / :101) did not
//       happen.  D5 goes further: with a genuine on-screen START press pending,
//       WaitManualStartKey still returns false AND LEAVES bSetupStart LATCHED --
//       golden :99 returns before :127 can clear it, so the press is not merely
//       refused, it is not even consumed.
//
//  [P4] PART E -- CheckThermo has exactly ONE false path.
//         golden :157-158  iTemperature==Tempture_Hot -> return true
//         golden :159-160  iHeaterCooling!=0          -> return false
//         golden :161-183  a for loop whose body is nothing but `continue`
//                          statements inside iIndexHeatMode conditionals: no
//                          return, no break, no assignment, no call
//         golden :184      unconditional return true
//       E1 sweeps all SIX eIndexHeatMode values (MachineType.h:538, HeadOnly..
//       HeadChamberSocket) x five temperature modes x three iHeaterCooling
//       values = 90 evaluations and asserts the answer is EXACTLY
//       `(iTemperature==Tempture_Hot || iHeaterCooling==0)` every time.  That is
//       the dead loop, stated as a falsifiable equation: restore any comparison
//       into golden's loop and E1 goes red.
//       SOFT_SIMULTE: golden :153-155 wraps the whole body in
//       `#ifdef SOFT_SIMULTE return true; #else ... #endif`.  In THIS build the
//       macro is COMMENTED OUT (ported MachineType.h:48
//       `//#define SOFT_SIMULTE`), so the #else arm -- the arm the real machine
//       builds -- is what compiles and what E1-E3 measure.  The #error guard
//       below makes it impossible for a future build to flip that macro and
//       leave this suite silently proving `return true`.
//
//  [P5] PART F -- StopAllDestroy's iteration FOOTPRINT, two golden defects in
//       three lines.
//         golden :191-197  for(i=0;i<InArmSuck.iMaxRow;i++)
//                            for(j=0;j<InArmSuck.iMaxRow;j++)   <-- iMaxRow!
//                              { InArmSuck.Suck[i][j].OffDestroy();
//                                OutArmSuck.Suck[i][j].OffDestroy(); }
//       (a) the INNER bound is iMaxRow where iMaxCol was meant, and (b)
//       OutArmSuck is indexed with InArmSuck's dimensions.  Both are pinned by
//       asserting WHICH CELLS WERE TOUCHED and WHICH WERE NOT, per nozzle,
//       through the OffDestroy call-counter seam (aHotPlateSubstrate.h
//       "OBSERVABILITY SEAM", body aHotPlateSubstrate.cpp:216-226):
//         F1  iMaxRow=2, iMaxCol=4 (a real 2x4 in-arm) -> the swept region is the
//             2x2 SQUARE; columns 2 and 3 are MISSED, i.e. their blow-off
//             solenoid stays asserted after a "stop all".
//         F3  iMaxRow=3, iMaxCol=1 -> a 3x3 region.  With iMaxCol==1 an
//             iMaxCol-bounded inner loop could only ever touch column 0, so F3
//             is the assertion that most directly kills `j<iMaxCol`.
//         F2  OutArmSuck given DIFFERENT dims of its own (iMaxRow=1, iMaxCol=8):
//             cell [1][1] IS touched (beyond OutArmSuck's own iMaxRow) and cell
//             [0][2] is NOT (well inside OutArmSuck's own iMaxCol).  Only
//             "indexed by InArmSuck's iMaxRow twice" explains both.
//         F4/F5/F6 pin the rest of golden's footprint exactly: the test heads
//             over MAX_Index_Row(=2, MachineType.h:389) x NEW_MAX_Index_Col(=8,
//             cmydef.cpp:3513) (golden :200-207), the single hard-coded tray-arm
//             nozzle [0][0] (golden :208), and the exact call totals.
//       Bounds safety: Suck is [4][8] (_MAX_SUCK_ROW_ITEM/_MAX_SUCK_COL_ITEM,
//       aHotPlateSubstrate.h:102-103) and the largest iMaxRow used here is 3, so
//       this suite never indexes out of bounds itself.
//
//  [P6] PART G -- InitialTestDelayStatus, golden :2529-2587.  Eleven
//       non-exclusive `if`s over bInitialTestDelayStatus[], so the LAST MATCH IN
//       SOURCE ORDER wins, and that order is [11],[6],[3],[2],[4],[5],[1],[7],
//       [9],[8],[10] -- NOT numeric order.  Pinned: every one of the eleven
//       literals byte-for-byte (including golden :2564's misspelling
//       "Fisrt"), that [0] and [12]-[14] are never read at all, and two
//       source-order collisions ([1] beats [11]; [10] beats [1]).
//
//  [P7] PART H -- DoScanLevelSensor, golden :2408-2430.  An `else if` chain in
//       source order Ope(:2413,->0), Eng(:2417,->1), Sup(:2421,->2),
//       Hon(:2425,->3), so the FIRST source branch wins and therefore the LOWEST
//       privilege wins.  H3 is the pin: Ope AND Hon both presented -> 0, not 3.
//
//  [P8] PART I -- ShowRunLed / ShowRunLabel are DELIBERATE deferred stubs for
//       W7-U (golden :704-932 and :935-1726, 1,021 lines of VCL painting).  What
//       makes a deferral auditable rather than a silent hole is that it does
//       NOTHING and says so.  Pinned: each bumps ONLY its own counter, the reset
//       zeroes both, and -- the substantive part -- NO observable machine state
//       changes across either call.  "Observable" here is concrete and is the
//       state golden's real bodies would have written: all 370 SW[] entries
//       (OutValue / SetValue / Enable -- golden :915-917 drives
//       SwTowerRed/Yellow/Green, :792-824 drives SwMusic1+n, :894-900 drives
//       SwFKAlarmReset/SwRKAlarmReset), all MAX_SENSOR_ITEM (900; 820 before AI(W906-F9050-BD) 20261004) Sen[].State fields (golden
//       ShowRunLabel reads six EMG/power inputs, and TMySensor::IsOn/IsOff write
//       .State on every read -- ported mysensor.cpp:129/:146/:152), plus
//       RunState, bAlarmBuzzer, bNeedMusicAndAlarmOn and the three lamp latches.
//       I5 additionally pins DoSystemMessage's 1-in-6 duty cycle (golden
//       :1899-1915) -- which is the reason the counters exist at all -- in the
//       phase-INDEPENDENT form ckernel.cpp:1907-1910 recommends, because golden
//       makes iMyCounter a function-local static with no reset and this wave did
//       not invent one.
//
// -----------------------------------------------------------------------------
//  DELIBERATE EXCLUSIONS -- pins NOT made, and the precise reason
//  (established pattern: tests/test_TesterTCP.cpp:25-42,
//   tests/test_auto9045.cpp's own block)
// -----------------------------------------------------------------------------
//  X1  ShowRunLed / ShowRunLabel REAL BODIES.  Not tested, and must not be:
//      golden :704-932 and :935-1726 are intentionally ABSENT from the port
//      (W7-L2 charter, ckernel.h:103-136).  Testing for golden's tower-light
//      selection or its 88 fMain->ShowNowStatus paints would assert behaviour
//      that does not exist yet.  PART I pins the deferral itself instead.
//
//  X2  `Enable &&` as an INDEPENDENT conjunct in DoScanLevelSensor (golden
//      :2413 etc.).  Offline it cannot be isolated: ported mysensor.cpp:127-131
//      makes IsOn() return false whenever Enable==false, so there is no
//      reachable state with Enable==false and IsOn()==true.  PART H therefore
//      pins the BRANCH ORDER, which is the golden oddity, and makes no claim
//      about the redundant conjunct.
//
//  X3  The four one-touch/lamp pins in PART D that need `fMain->BtnSTEP->Color`
//      are made, but the two golden colour VALUES are the only thing checked --
//      NOT that a real VCL panel repaints.  fMain is the offline facade
//      (forms/fMain.cpp:380 `TfMain *fMain = new TfMain();`), TfMainPanel is
//      vclcompat::TPanel (forms/FormWidgets.h:132); there is no window.  Stated
//      rather than dressed up: golden's `->Color` writes are display-sink writes
//      and nothing in golden or in this tree ever reads them back.
//
//  X4  ScanSystemSensor (golden :359-702), CheckBinSet (:211-250),
//      ChangeUseSuckMode (:252-357), DoPanelLamp (:1728-1897),
//      GetMotorAlarmCode (:2432-2499) and ProcessAlarm (:2501-2527) are OUT OF
//      SCOPE for this wave's brief (P1-P8) and are NOT pinned here.  DoPanelLamp
//      is nevertheless EXECUTED, once per six ticks, by PART I5's
//      DoSystemMessage loop -- so PART I snapshots and restores around it.
//      They need their own suite; this file does not pretend to cover them.
//
//  X5  RETIRED -- IsSafeLockCheck's own LOGIC is now IN SCOPE and IS tested.
//AI(W906-W7-L2-substrate) 20260803: this exclusion used to read "golden
//      csystem.cpp:16461-16508 has NO translation in this tree at all ... there is
//      nothing here to test".  That is no longer true: the real body was landed in
//      ported csystem.cpp this pass, so PART D7c-D7g pin its FULL branch table
//      (all five arms, plus the SW[] side effects golden makes on each) against
//      expectations derived by reading golden, and PART A2 / PART D7a-D7b now
//      engage the real lock through its real inputs (Sen[SnRKCoverOpen],
//      Sen[SnRKSafeLock], iControlPanelMode) instead of a test-local boolean.
//      PART A still ALSO pins the ORDER in which ScanPannelKey consults the lock
//      relative to golden :1927 -- an ordering property of ckernel.cpp that holds
//      whatever the lock answers.
//      Still genuinely out of scope, and the ONE arm not covered: golden
//      :16502-16506's `if(Tri_Temp_Machine==1)` -> IsTriSafeDoor6LockCheck() arm,
//      which is `#if 0`-gated in the port because golden TempCtrl/TriTemp.cpp has
//      no translation at all.  Unreachable in this tree regardless
//      (Tri_Temp_Machine is 0 and never assigned anywhere here), so D7g pins the
//      fall-through to golden :16507 that the gate leaves behind.
//
// -----------------------------------------------------------------------------
//  FAILABILITY PROOF -- MEASURED BY MUTATION, NOT ARGUED
// -----------------------------------------------------------------------------
//  Every mutation below was applied to a temporary copy of ckernel.cpp, built
//  with `cmake --build build_0803_l2 --target test_w7_l2_ckernel`, run, and then
//  REVERTED (ckernel.cpp md5 verified back to 2603d177e43444f375610b14c900f5a5
//  and the suite re-run at 66/0).  Counts are what the runs printed.
//
//AI(W906-W7-L2-substrate) 20260803: the table below is the ORIGINAL 66-assertion
//  measurement and is left exactly as it was measured.  This pass added SIX
//  assertions (D7a2 + D7c-D7g), so the baseline is now 72 PASS / 0 FAIL.  The
//  mutation table was NOT re-run for the new six -- stated plainly rather than
//  implied: their failability rests on the pre-set/SURVIVE construction (each case
//  distinguishes "golden wrote false" from "golden never touched it", so a dropped
//  or misdirected SW[] write fails the case), not on a measured mutation run.
//  ckernel.cpp is untouched by this pass, so every mutation result below still
//  describes the code it was measured against.
//
//    BASELINE, unmutated ............................. 66 PASS /  0 FAIL
//
//    M1  "fix" the headline bug: skip the +14 when ret is already a rear id
//        (`ret!=SnRKPowerOff && ret!=SnRKPowerOn`) .... 64 / 2   C1, C5
//    M2  golden :193 `j<InArmSuck.iMaxRow` -> `iMaxCol` 62 / 4   F1, F2, F3, F6
//    M3  index OutArmSuck by its OWN iMaxRow/iMaxCol .. 63 / 3   F2, F3, F6
//    M4  WaitManualStartKey's early-out returns TRUE
//        (harmonise the two signs) ................... \
//    M5  "Fisrt" -> "First" ..........................  |
//    M6  rear Start answers SnRKStart, not SnFKStart ..  } applied together
//    M7  `if(i==5) return false;` inside CheckThermo's  |  56 / 10
//        HeadOnly arm (revive the dead loop) .........  |  D1, D5 (M4)
//    M9  ShowRunLed also bumps the LABEL counter ...... /   G2[6], G4 (M5)
//                                                          B1, B4 (M6)
//                                                          E1 (M7)
//                                                          I2, I4, I6 (M9)
//    M8  release sweep drops the `Sen[i].IsOn()==false`
//        test (always clear) ......................... \
//    M10 hoist `bFrontPadActive=...` ABOVE the          } applied together
//        SystemInitialOK guard .......................  |  58 / 8
//    M11 reverse the DoScanLevelSensor else-if chain    |  B3, C2, C3, C6 (M8)
//        (Hon first) ................................. /   A1 (M10)
//                                                          H3, H4, H5 (M11)
//    FULL NEUTER -- all eight bodies emptied
//        (WaitManualStepKey/StartKey -> false,
//         CheckThermo -> true, StopAllDestroy -> return,
//         ScanPannelKey -> -1, DoScanLevelSensor -> -1,
//         InitialTestDelayStatus -> "", DoSystemMessage
//         -> return) AND both stub counter bumps
//        stripped .................................... 17 / 49
//
//  SO 49 OF THE 66 CHECKS GO RED ON A BODY THAT DOES NOTHING.  The other 17,
//  named rather than glossed over, are:
//      A1 A3 B3 C0 D2 D3 D4 D5 E2 E4 G1 G3 H1 I1 I3 I5 J
//  Every one of them is a NEGATIVE or IDENTITY assertion -- "answers -1",
//  "answers false", "answers \"\"", "nothing changed", "the counters are zero",
//  "the constants are 14 and 28" -- and an empty body satisfies all of those
//  vacuously.  That is not a defect in them; it is their job.  They establish
//  the baseline that makes their POSITIVE sibling a measured CHANGE rather than
//  a lone reading: D2 is what turns D1 from "it returned true" into "it returned
//  true from :56-57 specifically"; G1/G3 are what turn G2 from "it returned a
//  string" into "it returned THAT string and reads only THOSE indices"; I3/I5
//  are the whole content of the deferral claim and CANNOT be failable on an
//  empty body by construction.  Three of the seventeen are nevertheless killed
//  by a targeted mutation anyway -- A1 by M10, B3 by M8, D5 by M4 -- and C0 is a
//  constants check that does not touch a body at all.
//
//  TWO HONEST NOTES about what a particular check does and does not carry:
//   * C2 / C6 are NOT killed by M1.  With the offset removed the latch lands on
//     bK[14], Sen[14] is the held key, the latch holds, and C2's expected
//     14,-1,-1,-1 is produced for the WRONG reason.  C2's job is slot
//     IDENTIFICATION in conjunction with C1, and M8 is what proves it can fail.
//     C1 is the check that catches a "fix".
//   * B3 is likewise not killed by M6: with ret=19 the latch lands on bK[33],
//     which golden's 0..31 sweep never visits, so it stays latched and B3's
//     -1,-1,-1 still holds.  B1 is the check that catches that mutation.
//
// -----------------------------------------------------------------------------
//  HOW CASES ARE ISOLATED FROM EACH OTHER
// -----------------------------------------------------------------------------
//  Three mechanisms, all explicit:
//   1. GLOBAL SAVE/RESTORE.  main() snapshots every global it will mutate --
//      SystemInitialOK, bEnableEmployeeIDCheck, bFrontPadActive, all eleven
//      bAse* flags, FlushFlag, the lamp latches, bButtonManualStep/TStart,
//      bSTEP/bTSTART, bPushHomeDetect, iAseHome/iAseOneCycle,
//      CosFunction.bEnableSoftWareControlButton,
//      IniConfig.bDisibleResetButton, LastSet.iTemperature, iHeaterCooling,
//      Temperature.iIndexHeatMode, InArmSuck/OutArmSuck iMaxRow+iMaxCol, all 15
//      bInitialTestDelayStatus[] flags, fContact->fShow / ->bSetupStep /
//      ->bSetupStart / ->cbOneTouchAutoContactHight->Checked and the two button
//      captions, fMain->BtnSTEP/BtnT_Start->Color, RunState, bAlarmBuzzer,
//      bNeedMusicAndAlarmOn -- and restores all of them at the end.  The whole
//      Sen[] Enable/Type/ISABase triple (MAX_SENSOR_ITEM = 900 entries; 820 before AI(W906-F9050-BD) 20261004) is snapshotted and
//      restored wholesale.
//   2. THE SUCKER EPOCH.  W906_TMySucker_OffDestroy_ResetAll()
//      (aHotPlateSubstrate.cpp:69-75) bumps a generation counter, so every
//      per-nozzle count in the tree goes stale in O(1).  Called before each
//      StopAllDestroy fixture, so no count can leak between them.
//   3. A LATCH DRAIN for ScanPannelKey.  golden makes bK[64] a function-local
//      static (:1935) with a one-shot init (:1940-1945) and NO reset -- and this
//      wave did not invent one.  drainLatch() instead uses golden's OWN release
//      sweep: with every Sen[0..31] off, a call answers -1, skips the early
//      `return ret`, runs the sweep at :2395-2404 and clears every latch bit the
//      function can ever set (the largest p it can produce is 29).  Each PART B
//      / PART C case begins with a drain and asserts it answered -1, so a case
//      cannot inherit a latch from its predecessor.
//
// -----------------------------------------------------------------------------
//  LINK-TIME SUBSTRATE FILL -- RETIRED 20260803.  THE TRIPWIRE FIRED AS DESIGNED.
// -----------------------------------------------------------------------------
//AI(W906-W7-L2-substrate) 20260803: this block used to document three symbols that
//  this test TU DEFINED itself, because ckernel.cpp.obj referenced them and nothing
//  in the ported tree defined them.  Measured then, not assumed: `nm -C
//  --undefined-only` on the built object, diffed against the defined symbols of all
//  thirteen ht9045_* / vclcompat archives, left exactly
//      AccelateTask          golden csystem.cpp:16458  `int AccelateTask=1;`
//      IsSafeLockCheck()     golden csystem.cpp:16461-16508
//      InitDoArmZHome()      golden csystem.cpp:4858-4862
//  The fill carried an explicit note that it was a deliberate duplicate-symbol
//  TRIPWIRE: "when csystem.cpp finally lands any of the three, THIS FILE WILL FAIL
//  TO LINK ... That is the intended outcome -- delete the fill then."
//
//  THAT MOMENT ARRIVED.  All three now have REAL translated golden bodies in ported
//  csystem.cpp, so the fill is DELETED -- keeping it would be the duplicate-symbol
//  link error the tripwire existed to force, and shadowing a production symbol from
//  a test TU would be far worse than that error.  What replaced each:
//    * AccelateTask -- ported csystem.cpp, `int AccelateTask=1;` (golden :16458's
//      initialiser verbatim).  Nothing in this suite reads it; it exists so
//      ckernel.cpp:828 can link.
//    * InitDoArmZHome() -- ported csystem.cpp, golden's REAL two-assignment body
//      (`iAllArmZHomeTask=1; iAllArmZHomeCount=0;`), not the empty stub this file
//      used to supply.  PART J still proves the suite never enters it, but now by
//      watching the real body's own effect (the iAllArmZHomeCount sentinel) rather
//      than by counting calls into a local stub.
//    * IsSafeLockCheck() -- ported csystem.cpp, golden :16461-16508 in full,
//      INCLUDING the SW[] side effects on every arm.  The retired stand-in just
//      answered a `g_simSafeLock` boolean and had NO side effects at all, which an
//      earlier review flagged as a future trap; PART D7c-D7g now pin golden's whole
//      branch table, and PART A2 / D7a / D7b drive the real body through its real
//      inputs (Sen[SnRKCoverOpen], Sen[SnRKSafeLock], iControlPanelMode).
//      The suite-wide default is still "released / returns false" -- but for
//      golden's own reason now: allKeysUnknown() disables sensor 27
//      (SnRKCoverOpen==27, ported cmydef.cpp:822), so `if(Sen[SnRKCoverOpen].Enable)`
//      at golden :16471 is false, control takes golden :16496-16500 (which turns
//      BOTH locks off), `if(Tri_Temp_Machine==1)` at :16502 is false (ported
//      cmydef.cpp:5506 `int Tri_Temp_Machine=0;`, never assigned in this tree), and
//      golden :16507 returns false.
//
// -----------------------------------------------------------------------------
//  WHAT THIS SUITE DOES *NOT* TOUCH
// -----------------------------------------------------------------------------
//  No file, no socket, no directory, no window.  Purely in-process globals.
//
//  No external test framework: a tiny local harness prints PASS/FAIL per case
//  and a final summary, and returns non-zero on ANY failure.
// =============================================================================

#include "ckernel.h"                 // the unit: ScanPannelKey / StopAllDestroy / ...

#include "MachineDefine.h"
#include "MachineType.h"             // MAX_SENSOR_ITEM, MAX_Index_Row, eIndexHeatMode, eISABase
#include "cprod.h"                   // Temperature (SYSTEM_TEMPERATURE, cprod.h:1646)
#include "cmydef.h"                  // Sn*/Sw* ids, TYPE_A/TYPE_B, the global universe
#include "cpublic.h"
#include "mysensor.h"                // Sen[]
#include "myswitch.h"                // SW[]
#include "csystem.h"                 // declarations of IsSafeLockCheck / InitDoArmZHome
#include "canary_support.h"          // LastSet
#include "aHotPlateSubstrate.h"      // InArmSuck / OutArmSuck / F,BTestSuck / CatchTraySuck + OffDestroy seam
#include "acatchtray_shims.h"        // TColor / clYellow
#include "atester_shims.h"           // fContact
#include "FormsFacade.h"             // fMain
#include "forms/fShuttleMove.h"      // fShuttleMove
#include "forms/fNote.h"             // fNote
#include "Config.h"                  // IniConfig
#include "CosFunction.h"             // CosFunction

#include <cstdio>
#include <cstring>

// ---------------------------------------------------------------------------
//  Functions ckernel.cpp gives external linkage but golden ckernel.h does not
//  declare (see the ported ckernel.h "NOT DECLARED HERE" note, ckernel.h:52-62).
//  Declared here so the suite can call them; no header is modified.
// ---------------------------------------------------------------------------
bool CheckThermo();      // ckernel.cpp:396, golden ckernel.cpp:151
void ShowRunLed();       // ckernel.cpp:1513 (DEFERRED stub), golden ckernel.cpp:704
void ShowRunLabel();     // ckernel.cpp:1585 (DEFERRED stub), golden ckernel.cpp:935

// ---------------------------------------------------------------------------
//  SOFT_SIMULTE GUARD -- see [P4] in the banner.  Same device as
//  tests/test_w7_l4_copyostestresult.cpp:110-113.
// ---------------------------------------------------------------------------
//AI(W906-ST-S3-B2b) 20260918: the `#error` that stood here is replaced by a
//  SKIP guard. Recording why in full, because "someone removed a tripwire" is
//  exactly the shape that must never pass unexplained.
//
//  THE TRIPWIRE WAS RIGHT, AND STILL IS. Under SOFT_SIMULTE golden
//  ckernel.cpp:153-155 compiles CheckThermo down to a bare `return true;`, so
//  PART E's claim ("exactly ONE false path") has no paths left to test. Whoever
//  wrote it anticipated this build months in advance and said so.
//
//  WHAT CHANGED IS THE PREMISE, NOT THE ARGUMENT. Its text says "Ported
//  MachineType.h:48 has it commented out, which is the configuration PART E is
//  written for". The user turned SOFT_SIMULTE ON on 20260918 -- his model is
//  that there are exactly two builds, SOFT_SIMULTE = simulation, otherwise it
//  runs on a machine -- so that sentence is no longer true.
//
//  DELETING THE GUARD OUTRIGHT WOULD HAVE BEEN THE WORST OPTION: the suite
//  would go green while proving nothing, which is this tree's own
//  "a gate that cannot fail is not a gate". So PART E is SKIPPED with a loud
//  line, and every other PART still runs and still means what it says.
//
//  NOT COVERED WHILE THIS IS ON: CheckThermo's false path.
//  TO RESTORE: re-derive PART E against golden's #if SOFT_SIMULTE arm
//  (golden ckernel.cpp:153-155), then delete the #define below.
#ifdef SOFT_SIMULTE
#define W906_SKIP_PART_E_SOFT_SIMULTE 1
#endif

// ===========================================================================
//  LINK-TIME SUBSTRATE FILL -- GONE.  See the banner block of the same name.
//AI(W906-W7-L2-substrate) 20260803: the three definitions that used to sit here
// (`int AccelateTask = 1;`, `bool IsSafeLockCheck()`, `void InitDoArmZHome()`) are
// DELETED.  All three now come from the real translated golden bodies in ported
// csystem.cpp, so re-defining them here would be exactly the duplicate-symbol link
// error this fill was deliberately left in place to force.  The `g_simSafeLock`
// knob and the `g_initZHomeHits` counter went with them -- their replacements are
// safeLockEngage()/safeLockRelease() below and PART J's iAllArmZHomeCount sentinel.
// ===========================================================================

// golden csystem.cpp:4857 defines `int iAllArmZHomeCount=0;` at file scope and
// declares it extern in NO header -- golden's only consumer is csystem.cpp itself
// (its writes live in golden's InitDoArmZHome :4861 and DoArmZHome :5537-5600) --
// so the port faithfully added no header declaration either.  PART J reads it as a
// single-writer sentinel, so it is declared here, in the TU that needs it: the same
// "no header is modified" idiom as the three function declarations just above.
extern int iAllArmZHomeCount;

// ---------------------------------------------------------------------------
//  Harness
// ---------------------------------------------------------------------------
static int g_pass = 0;
static int g_fail = 0;

#define CHECK(cond, msg)                                                        \
    do {                                                                        \
        if (cond) { std::printf("PASS  %s\n", msg); ++g_pass; }                 \
        else      { std::printf("FAIL  %s  (line %d)\n", msg, __LINE__); ++g_fail; } \
    } while (0)

// ---------------------------------------------------------------------------
//  SIM SENSOR SEAM -- copied verbatim from tests/test_w7_l1_auto.cpp:486-502
//  (re-read this pass to confirm the counter-intuitive polarity).
//  ported mysensor.cpp: IsOn() returns false when Enable==false (:127-131);
//  otherwise it reads the offline raw-port stub (false) and then
//  `if(Type) return ret; else return !ret;` (:146-155).  TYPE_A==1
//  (cmydef.cpp:269) therefore reads as OFF and TYPE_B==0 as ON.  IsOff() is the
//  mirror (:163-...).  Sen[] is TRI-STATE: with Enable==false BOTH IsOn() and
//  IsOff() are false.
// ---------------------------------------------------------------------------
static void simSensorOff(int idx)       // IsOff()==true , IsOn()==false
{
    Sen[idx].Enable  = true;
    Sen[idx].Type    = TYPE_A;
    Sen[idx].ISABase = eISABase;
}
static void simSensorOn(int idx)        // IsOn()==true  , IsOff()==false
{
    Sen[idx].Enable  = true;
    Sen[idx].Type    = TYPE_B;
    Sen[idx].ISABase = eISABase;
}
static void simSensorUnknown(int idx)   // IsOn()==false AND IsOff()==false
{
    Sen[idx].Enable = false;
}

// ---------------------------------------------------------------------------
//  SAFETY-LOCK DRIVE -- the REAL inputs of the REAL IsSafeLockCheck
//  (golden csystem.cpp:16461-16508, translated in ported csystem.cpp).
//AI(W906-W7-L2-substrate) 20260803: replaces the retired `g_simSafeLock` boolean.
//  Both answers are DERIVED BY READING golden, not by running the port:
//
//  safeLockEngage() -> golden returns TRUE.
//      simSensorOff(SnRKCoverOpen) gives Enable==true and IsOff()==true, so golden
//      :16465 sets bSafeLockIOflag=true and golden :16471's outer guard is entered.
//      The callers below leave iControlPanelMode at the suite baseline 0 (ported
//      cmydef.cpp:5124 `int iControlPanelMode=0;`), so golden :16466 is false and
//      :16469 leaves bSafeLockComflag=false.  golden :16473 (`&&`) is therefore
//      false and golden :16479 `else if(bSafeLockIOflag)` wins: :16481
//      SW[SwSafeLock].On(), :16482 `return true`.  SW[SwRKSafeLock] is NOT written.
//
//  safeLockRelease() -> golden returns FALSE.
//      simSensorUnknown(SnRKCoverOpen) gives Enable==false, so golden :16471 is
//      false and the :16496-16500 else arm runs -- SW[SwSafeLock].Off() (:16498)
//      and SW[SwRKSafeLock].Off() (:16499) -- then :16502 `Tri_Temp_Machine==1` is
//      false and :16507 returns false.  This is also the state allKeysUnknown() /
//      drainLatch() leave sensor 27 in (SnRKCoverOpen==27, ported cmydef.cpp:822,
//      inside their 0..31 sweep), i.e. RELEASED is the suite-wide default -- the
//      same answer the retired stand-in's `false` gave, but now with golden's real
//      side effects actually happening.
// ---------------------------------------------------------------------------
static void safeLockEngage()  { simSensorOff(SnRKCoverOpen);     }
static void safeLockRelease() { simSensorUnknown(SnRKCoverOpen); }

// ---------------------------------------------------------------------------
//  Whole-array Sen[] snapshot/restore (isolation mechanism 1).
// ---------------------------------------------------------------------------
static bool s_senEnable [MAX_SENSOR_ITEM];
static int  s_senType   [MAX_SENSOR_ITEM];
static int  s_senISABase[MAX_SENSOR_ITEM];

static void senSaveAll()
{
    for (int i = 0; i < MAX_SENSOR_ITEM; ++i)
    {
        s_senEnable[i]  = Sen[i].Enable;
        s_senType[i]    = Sen[i].Type;
        s_senISABase[i] = Sen[i].ISABase;
    }
}
static void senRestoreAll()
{
    for (int i = 0; i < MAX_SENSOR_ITEM; ++i)
    {
        Sen[i].Enable  = s_senEnable[i];
        Sen[i].Type    = s_senType[i];
        Sen[i].ISABase = s_senISABase[i];
    }
}

// Put every key/pad input ScanPannelKey can read into the "unwired" state, so a
// case only sees the keys it deliberately presses.  0..31 covers every front and
// rear key id plus the whole span of golden's release sweep (:2395); 94 is
// SnRearPadActive (ported cmydef.cpp:914) which selects the arm.
static void allKeysUnknown()
{
    for (int i = 0; i < 32; ++i) simSensorUnknown(i);
    simSensorUnknown(SnRearPadActive);
}

// Zero every ASE software-key flag: each is an alternative disjunct beside the
// physical key in both arms (golden :1951, :1958, :1970, :1981, ...), so a stale
// true would fire a key this suite never pressed.
static void clearAseFlags()
{
    bAseReset      = false;
    bAsePause      = false;
    bAseHome       = false;
    bAseStart      = false;
    bAseOneCycle   = false;
    bAseRetry      = false;
    bAseSKIP       = false;
    bAseCleanOut   = false;
    bAseTrayFeed   = false;
    bAseTrayEnd    = false;
    bAseAlarmReset = false;
}

// ---------------------------------------------------------------------------
//  LATCH DRAIN (isolation mechanism 3).  Uses golden's OWN release sweep
//  (:2395-2404): with nothing pressed the call answers -1, so it does NOT take
//  the early `return ret` at :2388, the sweep runs, and every bK[i] for i<32 is
//  cleared.  golden can never produce p>31 (max is SnRKManualTStart==29), so
//  this clears the whole reachable latch.  Returns what the drain call answered
//  so the caller can assert it was -1.
// ---------------------------------------------------------------------------
static int drainLatch()
{
    allKeysUnknown();
    clearAseFlags();
    return ScanPannelKey();
}

// ---------------------------------------------------------------------------
//  Ordered key-sequence recorder: call ScanPannelKey() n times WITHOUT touching
//  anything in between -- i.e. the key stays physically held -- and compare the
//  answers against an exact ordered expectation.  Not a set, not a membership
//  test: golden's bug is entirely about WHICH CALL fires.
// ---------------------------------------------------------------------------
static bool keySeqEq(int n, const int *want, char *out, int outSize)
{
    bool ok = true;
    int  used = 0;
    out[0] = '\0';
    for (int i = 0; i < n; ++i)
    {
        int got = ScanPannelKey();
        if (got != want[i]) ok = false;
        if (used < outSize - 24)
            used += std::sprintf(out + used, "%s%d", (i ? "," : ""), got);
    }
    return ok;
}

// ---------------------------------------------------------------------------
//  PART I observability snapshot: the machine state golden's ShowRunLed /
//  ShowRunLabel bodies would have written.  See [P8] in the banner for why each
//  field is in the list.
// ---------------------------------------------------------------------------
struct MachineSnap
{
    bool swOut  [MAX_SWITCH_ITEM];
    bool swSet  [MAX_SWITCH_ITEM];
    bool swEn   [MAX_SWITCH_ITEM];
    int  senState[MAX_SENSOR_ITEM];
    int  runState;
    bool alarmBuzzer;
    bool needMusic;
    bool lampSetp;
    bool lampStart;
    bool lampRetry;
};

static void snapMachine(MachineSnap &s)
{
    for (int i = 0; i < MAX_SWITCH_ITEM; ++i)
    {
        s.swOut[i] = SW[i].OutValue;
        s.swSet[i] = SW[i].SetValue;
        s.swEn[i]  = SW[i].Enable;
    }
    for (int i = 0; i < MAX_SENSOR_ITEM; ++i) s.senState[i] = Sen[i].State;
    s.runState    = RunState;
    s.alarmBuzzer = bAlarmBuzzer;
    s.needMusic   = bNeedMusicAndAlarmOn;
    s.lampSetp    = bLampManualSetp;
    s.lampStart   = bLampManualStart;
    s.lampRetry   = bLampRetry;
}

// Returns the number of DIFFERING observables (0 == nothing moved).
static int diffMachine(const MachineSnap &a, const MachineSnap &b)
{
    int n = 0;
    for (int i = 0; i < MAX_SWITCH_ITEM; ++i)
    {
        if (a.swOut[i] != b.swOut[i]) ++n;
        if (a.swSet[i] != b.swSet[i]) ++n;
        if (a.swEn[i]  != b.swEn[i])  ++n;
    }
    for (int i = 0; i < MAX_SENSOR_ITEM; ++i) if (a.senState[i] != b.senState[i]) ++n;
    if (a.runState    != b.runState)    ++n;
    if (a.alarmBuzzer != b.alarmBuzzer) ++n;
    if (a.needMusic   != b.needMusic)   ++n;
    if (a.lampSetp    != b.lampSetp)    ++n;
    if (a.lampStart   != b.lampStart)   ++n;
    if (a.lampRetry   != b.lampRetry)   ++n;
    return n;
}

static MachineSnap s_snapBefore;
static MachineSnap s_snapAfter;

// ---------------------------------------------------------------------------
//  PART F helper: the total OffDestroy calls a golden StopAllDestroy must make
//  for a given InArmSuck.iMaxRow.
//    inner+outer loop : iMaxRow*iMaxRow cells x 2 grids (InArmSuck + OutArmSuck)
//    test heads       : MAX_Index_Row * NEW_MAX_Index_Col x 2 grids
//    tray arm         : 1
// ---------------------------------------------------------------------------
static long expectedOffDestroyTotal(int iMaxRow)
{
    return (long)iMaxRow * iMaxRow * 2
         + (long)MAX_Index_Row * NEW_MAX_Index_Col * 2
         + 1;
}

int main()
{
    std::printf("=== test_w7_l2_ckernel  (ported ckernel.cpp vs golden ckernel.cpp) ===\n");

    // -----------------------------------------------------------------
    //  SAVE every global this suite mutates (isolation mechanism 1).
    // -----------------------------------------------------------------
    senSaveAll();

    const bool svSystemInitialOK   = SystemInitialOK;
    const bool svEmployeeIDCheck   = bEnableEmployeeIDCheck;
    const bool svFrontPadActive    = bFrontPadActive;
    const bool svFlushFlag         = FlushFlag;
    const bool svLampSetp          = bLampManualSetp;
    const bool svLampStart         = bLampManualStart;
    const bool svLampRetry         = bLampRetry;
    const bool svButtonStep        = bButtonManualStep;
    const bool svButtonTStart      = bButtonManualTStart;
    const bool svSTEP              = bSTEP;
    const bool svTSTART            = bTSTART;
    const bool svPushHomeDetect    = bPushHomeDetect;
    const int  svAseHome           = iAseHome;
    const bool svSoftBtn           = CosFunction.bEnableSoftWareControlButton;
    const bool svDisibleReset      = IniConfig.bDisibleResetButton;
    const int  svTemperatureMode   = LastSet.iTemperature;
    const int  svHeaterCooling     = iHeaterCooling;
    const int  svIndexHeatMode     = Temperature.iIndexHeatMode;
    const int  svInMaxRow          = InArmSuck.iMaxRow;
    const int  svInMaxCol          = InArmSuck.iMaxCol;
    const int  svOutMaxRow         = OutArmSuck.iMaxRow;
    const int  svOutMaxCol         = OutArmSuck.iMaxCol;
    const bool svContactShow       = fContact->fShow;
    const bool svContactSetupStep  = fContact->bSetupStep;
    const bool svContactSetupStart = fContact->bSetupStart;
    const bool svOneTouch          = fContact->cbOneTouchAutoContactHight->Checked;
    const AnsiString svCapStep     = fContact->btnTStep->Caption;
    const AnsiString svCapStart    = fContact->btnTStart->Caption;
    const int  svColStep           = fMain->BtnSTEP->Color;
    const int  svColStart          = fMain->BtnT_Start->Color;
    const int  svRunState          = RunState;
    const bool svAlarmBuzzer       = bAlarmBuzzer;
    const bool svNeedMusic         = bNeedMusicAndAlarmOn;
    const bool svNoteShow          = fNote->fShow;
    bool svDelay[15];
    for (int i = 0; i < 15; ++i) svDelay[i] = bInitialTestDelayStatus[i];
    //AI(W906-W7-L2-substrate) 20260803: four more globals to restore, all newly
    // reachable now that IsSafeLockCheck is the REAL body.  Sen[] itself is already
    // covered wholesale by senSaveAll/senRestoreAll, so sensor 27 / 550 need no
    // entry here.
    const int  svControlPanelMode = iControlPanelMode;
    const bool svSwSafeLockOut    = SW[SwSafeLock].OutValue;
    const bool svSwRKSafeLockOut  = SW[SwRKSafeLock].OutValue;
    const int  svAllArmZHomeCount = iAllArmZHomeCount;

    // Baseline fixture shared by PARTS A-C.
    SystemInitialOK        = true;
    bEnableEmployeeIDCheck = false;
    //AI(W906-W7-L2-substrate) 20260803: was `g_simSafeLock = false;`.  The lock is
    // now released through its real input, and iControlPanelMode is pinned to
    // golden :16466's else arm so every engage below is the single-flag :16479 path
    // unless a case deliberately says otherwise.
    safeLockRelease();
    iControlPanelMode      = 0;
    // PART J sentinel.  InitDoArmZHome (golden csystem.cpp:4858-4862) is the ONLY
    // writer of iAllArmZHomeCount anywhere in the ported tree, so if this value is
    // still here at PART J the function was never entered.
    iAllArmZHomeCount      = 23117;
    IniConfig.bDisibleResetButton = false;   // golden :1949/:2126 -- keeps the Reset block live
    fNote->fShow           = false;          // golden :2007/:2038/:2184/:2215 take the simple else arms
    allKeysUnknown();
    clearAseFlags();

    // =================================================================
    //  PART A -- ScanPannelKey's three entry guards, and the ORDER of the
    //  bFrontPadActive assignment relative to them.
    //    golden :1924-1925  if(SystemInitialOK==false) return -1;
    //    golden :1927       bFrontPadActive = Sen[SnRearPadActive].IsOff();
    //    golden :1929-1930  if(IsSafeLockCheck())       return -1;
    //    golden :1932-1933  if(bEnableEmployeeIDCheck)  return -1;
    //  The two-sided ordering proof: A1 shows the FIRST guard runs BEFORE the
    //  assignment (the pad selection is not updated), A2 shows the SECOND guard
    //  runs AFTER it (the pad selection IS updated).  Neither depends on what
    //  IsSafeLockCheck actually computes -- see EXCLUSION X5.
    // =================================================================
    std::printf("\n-- PART A: ScanPannelKey entry guards (golden :1924-1933) --\n");
    {
        // A1: sensor says "rear pad active" (IsOff()==false -> bFrontPadActive
        // would become false), but we pre-set bFrontPadActive TRUE and disarm the
        // system.  golden :1924 must return before :1927 can overwrite it.
        simSensorOn(SnRearPadActive);
        bFrontPadActive  = true;
        SystemInitialOK  = false;
        int rA1 = ScanPannelKey();
        CHECK(rA1 == -1 && bFrontPadActive == true,
              "PART A1: SystemInitialOK==false -> -1 AND bFrontPadActive left UNTOUCHED -- the guard at golden :1924-1925 precedes the pad-select assignment at golden :1927");

        // A2: same sensor state, same pre-set, but the system IS initialised and
        // the safety lock is engaged.  Now :1927 has already run when :1929
        // returns, so the pad selection HAS been updated to false.
        SystemInitialOK = true;
        bFrontPadActive = true;
        //AI(W906-W7-L2-substrate) 20260803: was `g_simSafeLock = true;`.  The guard
        // is now driven by the REAL IsSafeLockCheck through its REAL input -- golden
        // csystem.cpp:16479-16482 answers true for this sensor state.  Note ScanPannelKey
        // never reads Sen[SnRKCoverOpen] as a key (ported ckernel.cpp names it only in
        // the comment at :2045), so engaging the lock cannot forge a key press here.
        safeLockEngage();
        int rA2 = ScanPannelKey();
        CHECK(rA2 == -1 && bFrontPadActive == false,
              "PART A2: safety lock engaged -> -1 BUT bFrontPadActive already updated to false -- golden :1927 runs BEFORE the guard at golden :1929-1930 (the lock is the REAL golden body now, engaged via Sen[SnRKCoverOpen])");
        safeLockRelease();

        // A3: the employee-ID lockout, golden :1932-1933.
        bEnableEmployeeIDCheck = true;
        simSensorOn(SnRKStart);                 // a real key press that must be ignored
        int rA3 = ScanPannelKey();
        CHECK(rA3 == -1,
              "PART A3: bEnableEmployeeIDCheck==true -> -1 even with the rear Start key held (golden :1932-1933)");
        bEnableEmployeeIDCheck = false;
        simSensorUnknown(SnRKStart);
    }

    // =================================================================
    //  PART B -- [P2] the rear arm answers with FRONT ids, and the +14 latch
    //  offset is exactly what makes that work for those ids.
    // =================================================================
    std::printf("\n-- PART B: rear keys answer with FRONT ids (golden :2126-2298) --\n");
    {
        // -- B0: the drain itself, asserted once so the isolation mechanism is
        //    not merely claimed.
        simSensorOn(SnRearPadActive);           // IsOff()==false -> REAR arm (golden :1927)
        int rB0 = drainLatch();
        simSensorOn(SnRearPadActive);           // drainLatch() unwires it; re-select the rear arm
        CHECK(rB0 == -1 && bFrontPadActive == false,
              "PART B0: with nothing pressed ScanPannelKey answers -1 and selects the REAR arm -- this is the latch drain every case below relies on (golden :1927, :2405)");

        // -- B1: rear Start -> SnFKStart (5), NOT SnRKStart (19).
        //    golden :2158-2161.  Ids: ported cmydef.cpp:798 SnFKStart=5, :813
        //    SnRKStart=19 (byte-identical to golden cmydef.cpp:794/:809).
        simSensorOn(SnRKStart);
        int rB1 = ScanPannelKey();
        CHECK(rB1 == SnFKStart && rB1 == 5 && SnRKStart == 19,
              "PART B1: rear Start key (Sen[19]) is reported as SnFKStart==5, NOT 19 -- golden :2158-2161 answers with the FRONT id");

        // -- B3: and while it stays held the latch HOLDS, because 5+14==19 is the
        //    very sensor being read.  golden :2381-2383 + :2395-2404.
        {
            char got[64];
            const int want[3] = { -1, -1, -1 };   // call 1 already consumed above by B1
            bool ok = keySeqEq(3, want, got, sizeof(got));
            CHECK(ok,
                  "PART B3: rear Start HELD -> 5 then -1,-1,-1 -- p=5+14=19=SnRKStart, so the sweep at golden :2395-2404 finds Sen[19] ON and KEEPS the latch (contrast PART C1)");
            if (!ok) std::printf("      got after the first press: %s\n", got);
        }

        // -- B4: release, then press again: the latch really does clear on a
        //    genuine release, so B3 is not "this key can only fire once ever".
        simSensorUnknown(SnRKStart);
        int rB4a = ScanPannelKey();               // sweep clears bK[19]
        simSensorOn(SnRKStart);
        int rB4b = ScanPannelKey();
        CHECK(rB4a == -1 && rB4b == SnFKStart,
              "PART B4: rear Start RELEASED (-1, latch swept) then re-pressed fires 5 again -- proves PART B3's -1s are the latch, not exhaustion");

        // -- B5: rear Home -> SnFKHome (4) and golden's bPushHomeDetect side
        //    effect.  golden :2147-2156.
        drainLatch();
        simSensorOn(SnRearPadActive);
        bPushHomeDetect = false;
        iAseHome        = 99;
        simSensorOn(SnRKHome);
        int rB5 = ScanPannelKey();
        CHECK(rB5 == SnFKHome && rB5 == 4 && bPushHomeDetect == true && iAseHome == 0,
              "PART B5: rear Home key (Sen[18]) reports SnFKHome==4 and sets bPushHomeDetect=true, iAseHome=0 (golden :2147-2156, the bAseHome==false arm)");

        // -- B2: the SAME id comes back from the FRONT Start key. golden :1981-1984.
        drainLatch();
        simSensorOff(SnRearPadActive);            // IsOff()==true -> FRONT arm
        simSensorOn(SnFKStart);
        int rB2 = ScanPannelKey();
        CHECK(rB2 == SnFKStart && bFrontPadActive == true,
              "PART B2: front Start key (Sen[5]) also reports 5 (golden :1981-1984) -- the SAME id from two different physical keys, which is what makes B1 a real observation");
    }

    // =================================================================
    //  PART C -- [P1] THE HEADLINE BUG.  See the banner for the full derivation
    //  and for why C1+C2 together identify the latch slot as exactly 28.
    // =================================================================
    std::printf("\n-- PART C: rear Power-Off double-offset mis-latch (golden :2174-2177 + :2381-2404) --\n");
    {
        // Sanity on the arithmetic itself, straight from the constants.
        CHECK(SnRKPowerOff == 14 && SnRKManualStep == 28 &&
              SnRKPowerOff + SnRKPowerOff == SnRKManualStep &&
              SnRKPowerOn == 15 && SnRKManualTStart == 29 &&
              SnRKPowerOn + SnRKPowerOff == SnRKManualTStart,
              "PART C0: the offset arithmetic -- SnRKPowerOff(14)+14 == SnRKManualStep(28) and SnRKPowerOn(15)+14 == SnRKManualTStart(29) (ported cmydef.cpp:808/809/823/824, == golden cmydef.cpp:804/805/819/820)");

        // -- C1: rear Power-Off HELD.  golden :2174-2177 answers ret=SnRKPowerOff
        //    (already a REAR id), so p=14+14=28.  The sweep clears bK[28] because
        //    Sen[28] is off -> the key re-fires on every odd call, never released.
        drainLatch();
        simSensorOn(SnRearPadActive);
        simSensorOn(SnRKPowerOff);
        {
            char got[64];
            const int want[5] = { 14, -1, 14, -1, 14 };
            bool ok = keySeqEq(5, want, got, sizeof(got));
            CHECK(ok,
                  "PART C1: *** THE BUG *** rear Power-Off HELD DOWN answers 14,-1,14,-1,14 -- it RE-TRIGGERS without ever being released (golden :2381-2383 latches bK[28]; :2395-2404 clears it against Sen[28], the manual-step key)");
            if (!ok) std::printf("      got: %s  (expected 14,-1,14,-1,14)\n", got);
        }

        // -- C2: identical fixture, plus the completely unrelated manual-step key
        //    held ON.  Nothing in ScanPannelKey reads Sen[28] (the rear arm polls
        //    16,17,3,18,19,20,14,15,21,22,23,24,25,26,12), so if the re-fire stops
        //    the ONLY explanation is that this latch's release is decided on
        //    Sen[28] -- i.e. the slot really is 28.
        drainLatch();
        simSensorOn(SnRearPadActive);
        simSensorOn(SnRKPowerOff);
        simSensorOn(SnRKManualStep);              // sensor 28 -- read by NOTHING in this function
        {
            char got[64];
            const int want[4] = { 14, -1, -1, -1 };
            bool ok = keySeqEq(4, want, got, sizeof(got));
            CHECK(ok,
                  "PART C2: *** THE PROOF *** holding the unrelated Sen[SnRKManualStep] (28) SUPPRESSES the rear Power-Off re-fire (14,-1,-1,-1) -- with C1 this identifies the mis-latched slot as exactly bK[28]");
            if (!ok) std::printf("      got: %s  (expected 14,-1,-1,-1)\n", got);
        }
        simSensorUnknown(SnRKManualStep);

        // -- C3: THE CONTRAST.  Front Power-Off, no offset (golden :1997-2000
        //    answers SnFKPowerOff==0 with bFrontPadActive true), bK[0] swept
        //    against Sen[0] which IS the key being held -> stays latched.
        drainLatch();
        simSensorOff(SnRearPadActive);            // FRONT arm
        simSensorOn(SnFKPowerOff);
        {
            char got[64];
            const int want[4] = { 0, -1, -1, -1 };
            bool ok = keySeqEq(4, want, got, sizeof(got));
            CHECK(ok,
                  "PART C3: FRONT Power-Off HELD answers 0,-1,-1,-1 -- correctly latched, because SnFKPowerOff==0 takes no offset and bK[0] is swept against Sen[0], the key itself (golden :1997-2000)");
            if (!ok) std::printf("      got: %s  (expected 0,-1,-1,-1)\n", got);
        }

        // -- C4: release the front key, then press it again -> it fires.  So C3's
        //    three -1s are the latch doing its job, not the key being dead.
        simSensorOff(SnFKPowerOff);               // physically released (IsOn()==false)
        int rC4a = ScanPannelKey();               // sweep clears bK[0]
        simSensorOn(SnFKPowerOff);
        int rC4b = ScanPannelKey();
        CHECK(rC4a == -1 && rC4b == SnFKPowerOff && rC4b == 0,
              "PART C4: front Power-Off RELEASED then re-pressed fires 0 again -- PART C3's -1s are the press latch, not exhaustion");

        // -- C5: the SAME defect one slot up.  golden :2179-2182 answers
        //    ret=SnRKPowerOn (15) -> p=15+14=29==SnRKManualTStart.
        drainLatch();
        simSensorOn(SnRearPadActive);
        simSensorOn(SnRKPowerOn);
        {
            char got[64];
            const int want[3] = { 15, -1, 15 };
            bool ok = keySeqEq(3, want, got, sizeof(got));
            CHECK(ok,
                  "PART C5: rear Power-ON HELD answers 15,-1,15 -- the identical mis-latch at bK[29] (golden :2179-2182 + :2381-2383)");
            if (!ok) std::printf("      got: %s  (expected 15,-1,15)\n", got);
        }
        drainLatch();
        simSensorOn(SnRKPowerOn);
        simSensorOn(SnRKManualTStart);            // sensor 29
        {
            char got[64];
            const int want[3] = { 15, -1, -1 };
            bool ok = keySeqEq(3, want, got, sizeof(got));
            CHECK(ok,
                  "PART C6: holding the unrelated Sen[SnRKManualTStart] (29) SUPPRESSES the rear Power-ON re-fire (15,-1,-1) -- the slot is exactly bK[29]");
            if (!ok) std::printf("      got: %s  (expected 15,-1,-1)\n", got);
        }
        drainLatch();
    }

    // =================================================================
    //  PART D -- [P3] WaitManualStepKey vs WaitManualStartKey.
    //    golden :56-57  return TRUE  on (fShow && Checked)
    //    golden :98-99  return FALSE on the structurally identical test
    // =================================================================
    std::printf("\n-- PART D: WaitManualStepKey / WaitManualStartKey DISAGREE (golden :56-57 vs :98-99) --\n");
    {
        allKeysUnknown();
        bButtonManualStep   = false;
        bButtonManualTStart = false;
        bSTEP               = false;
        bTSTART             = false;
        fContact->bSetupStep  = false;
        fContact->bSetupStart = false;
        CosFunction.bEnableSoftWareControlButton = false;

        // -- D1/D2: one-touch mode ON.  Both early-outs fire.  The lamp latches
        //    are pre-set to the OPPOSITE of FlushFlag, so if either body had run
        //    its very first statement (golden :59 / :101, `bLamp... = FlushFlag`)
        //    the sentinel would have moved.
        fContact->fShow = true;
        fContact->cbOneTouchAutoContactHight->Checked = true;
        FlushFlag        = true;
        bLampManualSetp  = false;                 // != FlushFlag  -> sentinel
        bLampManualStart = false;                 // != FlushFlag  -> sentinel
        fContact->btnTStep->Caption  = "SENTINEL-STEP";
        fContact->btnTStart->Caption = "SENTINEL-START";

        bool dStep  = WaitManualStepKey();
        bool dStart = WaitManualStartKey();

        CHECK(dStep == true && dStart == false && dStep != dStart,
              "PART D1: ONE fixture, BOTH functions -- WaitManualStepKey()==true and WaitManualStartKey()==false. They DISAGREE, and the disagreement is load-bearing (golden :56-57 vs :98-99)");
        CHECK(bLampManualSetp == false && bLampManualStart == false &&
              fContact->btnTStep->Caption  == "SENTINEL-STEP" &&
              fContact->btnTStart->Caption == "SENTINEL-START",
              "PART D2: neither body ran -- the lamp latches and both captions are untouched, so D1's answers came from the early-outs at golden :56-57 / :98-99 and not from the polling bodies at golden :59-93 / :101-130");

        // -- D5: with a real on-screen START press pending, START still refuses
        //    AND does not consume it: golden :99 returns before :127 clears it.
        fContact->bSetupStart = true;
        bool dStart2 = WaitManualStartKey();
        CHECK(dStart2 == false && fContact->bSetupStart == true,
              "PART D5: one-touch mode swallows nothing -- WaitManualStartKey() still returns false AND leaves fContact->bSetupStart LATCHED, because golden :99 returns before golden :127 can clear it");
        fContact->bSetupStart = false;

        // -- D4: the test is a CONJUNCTION.  Form shown but the box unchecked ->
        //    no early-out (golden :56 / :98 both need Checked==true).
        fContact->cbOneTouchAutoContactHight->Checked = false;
        bool dStep4  = WaitManualStepKey();
        bool dStart4 = WaitManualStartKey();
        CHECK(dStep4 == false && dStart4 == false,
              "PART D4: fShow==true but Checked==false -> NO early-out, both fall through to their polling bodies and both answer false (golden :56 / :98 are conjunctions)");

        // -- D3: one-touch mode OFF entirely -> they AGREE.  This is what makes
        //    D1 attributable to the early-out rather than to anything else about
        //    these two functions.
        fContact->fShow = false;
        fContact->cbOneTouchAutoContactHight->Checked = true;   // still checked; fShow closes it
        bool dStep3  = WaitManualStepKey();
        bool dStart3 = WaitManualStartKey();
        CHECK(dStep3 == false && dStart3 == false && dStep3 == dStart3,
              "PART D3: with fShow==false the two functions AGREE (both false) -- so PART D1's disagreement is caused by the one-touch early-out and by nothing else");

        // -- D6: the polling bodies really do run in D3's configuration -- golden
        //    :59-72 / :101-114.  FlushFlag drives the lamp latch, the caption and
        //    (under CosFunction) the panel colour.  Both phases checked.
        CosFunction.bEnableSoftWareControlButton = true;   // golden :66 / :108
        FlushFlag = true;
        WaitManualStepKey();
        WaitManualStartKey();
        bool d6a = (bLampManualSetp  == true) &&
                   (bLampManualStart == true) &&
                   (fContact->btnTStep->Caption  == "T.Step")  &&
                   (fContact->btnTStart->Caption == "T.Start") &&
                   (fMain->BtnSTEP->Color    == (int)clYellow) &&
                   (fMain->BtnT_Start->Color == (int)clYellow);
        CHECK(d6a,
              "PART D6a: FlushFlag==true -> bLamp*=true, captions \"T.Step\"/\"T.Start\", BtnSTEP/BtnT_Start->Color=clYellow (golden :59-72 / :101-114)");

        FlushFlag = false;
        WaitManualStepKey();
        WaitManualStartKey();
        bool d6b = (bLampManualSetp  == false) &&
                   (bLampManualStart == false) &&
                   (fContact->btnTStep->Caption  == "")  &&
                   (fContact->btnTStart->Caption == "") &&
                   (fMain->BtnSTEP->Color    == 0x00804000) &&
                   (fMain->BtnT_Start->Color == 0x00804000);
        CHECK(d6b,
              "PART D6b: FlushFlag==false -> bLamp*=false, both captions \"\", both Colors 0x00804000 -- golden's exact literal (golden :71 / :113)");
        CosFunction.bEnableSoftWareControlButton = false;

        // -- D7: golden's post-consumption safety swallow, WaitManualStepKey only.
        //    golden :74-92 clears bButtonManualStep (:76) and bSetupStep (:77)
        //    BEFORE testing IsSafeLockCheck() (:85), so a rejected press is GONE.
        //AI(W906-W7-L2-substrate) 20260803: the lock is no longer a fiction.  This
        //    used to say "the lock's own answer comes from this file's substrate
        //    fill"; the fill is retired and the lock is the REAL translated golden
        //    body (ported csystem.cpp; golden csystem.cpp:16461-16508), engaged
        //    through its REAL input Sen[SnRKCoverOpen] via safeLockEngage().  The
        //    retired stand-in had NO side effects, so D7a/D7b could not see the SW[]
        //    writes golden actually makes -- D7a2 asserts them here, and D7c-D7g
        //    below cover the whole branch table.
        //    Its twin WaitManualStartKey has NO safety gate at all -- golden
        //    :96-131 never calls IsSafeLockCheck -- which D7b pins.
        bButtonManualStep   = true;
        bButtonManualTStart = true;
        SW[SwSafeLock].OutValue   = false;   // golden :16481 must turn this ON
        SW[SwRKSafeLock].OutValue = true;    // golden writes this on NO arm of the :16479 path -- must SURVIVE
        safeLockEngage();
        bool d7Step  = WaitManualStepKey();
        CHECK(d7Step == false && bButtonManualStep == false,
              "PART D7a: safety lock engaged -> WaitManualStepKey() returns false but the press is ALREADY CONSUMED (bButtonManualStep cleared) -- golden :76 runs before golden :85, so the press is swallowed, not deferred");
        CHECK(SW[SwSafeLock].OutValue == true && SW[SwRKSafeLock].OutValue == true,
              "PART D7a2: the REAL IsSafeLockCheck ran and took golden csystem.cpp:16479-16482 -- SW[SwSafeLock].On() fired (:16481) while SW[SwRKSafeLock] was left ALONE (the pre-set true survives), proving neither :16475 nor :16492 ran. The retired stand-in had no side effects and could not have caught this");
        bool d7Start = WaitManualStartKey();
        CHECK(d7Start == true,
              "PART D7b: the SAME safety lock does NOT gate WaitManualStartKey -- it still returns true, because golden :96-131 contains no IsSafeLockCheck call at all");
        safeLockRelease();
        bButtonManualStep   = false;
        bButtonManualTStart = false;

        // -- D7c..D7g: IsSafeLockCheck's OWN branch table, exhaustively.
        //AI(W906-W7-L2-substrate) 20260803: NEW this pass, and the point of this
        //    wave.  Now that golden csystem.cpp:16461-16508 has a real translated
        //    body, the suite pins golden's ACTUAL logic rather than a stand-in.
        //    Every expected value below was DERIVED BY READING golden :16461-16508
        //    (the deciding line is cited in each case), NOT by running the port and
        //    recording what it did.
        //
        //    THE OBSERVABLE.  Ported myswitch.cpp:76 (`On()`) and :126 (`Off()`)
        //    both write OutValue UNCONDITIONALLY, ahead of the `Enable==false` early
        //    return at :86 / :136 -- so SW[i].OutValue is an exact record of which
        //    call golden made, with no switch needing to be wired to anything.  Each
        //    case pre-sets BOTH OutValue fields so that "golden wrote false" and
        //    "golden never touched it" stay distinguishable: wherever golden leaves a
        //    switch alone the pre-set is TRUE and must SURVIVE.
        //
        //    THE INPUTS (sim seam re-read this pass -- note the polarity):
        //      simSensorOff(x)     -> Enable==true , IsOff()==true , IsOn()==false
        //      simSensorOn(x)      -> Enable==true , IsOff()==false, IsOn()==true
        //      simSensorUnknown(x) -> Enable==false, and BOTH IsOff() and IsOn()
        //                             answer false (ported mysensor.cpp:127-131 /
        //                             :177-181 -- the tri-state).
        {   // D7c -- golden :16473-16478.  BOTH flags true.
            //   :16465 bSafeLockIOflag  = Sen[SnRKCoverOpen].IsOff() -> true
            //   :16466 iControlPanelMode==1 -> :16467
            //          bSafeLockComflag = Sen[SnRKSafeLock].IsOff()  -> true
            //   :16471 Sen[SnRKCoverOpen].Enable -> true, enter the outer if
            //   :16473 true && true    -> :16475 SW[SwRKSafeLock].On()
            //                             :16476 SW[SwSafeLock].On()
            //                             :16477 return true
            iControlPanelMode = 1;
            simSensorOff(SnRKCoverOpen);
            simSensorOff(SnRKSafeLock);
            SW[SwSafeLock].OutValue   = false;
            SW[SwRKSafeLock].OutValue = false;
            bool r7c = IsSafeLockCheck();
            CHECK(r7c == true &&
                  SW[SwRKSafeLock].OutValue == true &&
                  SW[SwSafeLock].OutValue   == true,
                  "PART D7c: cover-open IsOff AND (panel mode 1) safe-lock IsOff -> golden :16473 wins: BOTH SW[SwRKSafeLock].On() (:16475) and SW[SwSafeLock].On() (:16476) fire, return true (:16477)");
        }
        {   // D7d -- golden :16479-16483.  IO flag ONLY.  Sensors are IDENTICAL to
            //   D7c and ONLY iControlPanelMode differs, which makes this pair a
            //   two-sided proof of golden :16466-16469.
            //   :16466 iControlPanelMode!=1 -> :16469 bSafeLockComflag=false
            //   :16473 true && false   -> false
            //   :16479 else if(true)   -> :16481 SW[SwSafeLock].On()
            //                             :16482 return true
            //   golden writes NOTHING to SW[SwRKSafeLock] on this path.
            iControlPanelMode = 0;
            simSensorOff(SnRKCoverOpen);
            simSensorOff(SnRKSafeLock);         // left as D7c had it, ON PURPOSE
            SW[SwSafeLock].OutValue   = false;
            SW[SwRKSafeLock].OutValue = true;   // must SURVIVE
            bool r7d = IsSafeLockCheck();
            CHECK(r7d == true &&
                  SW[SwSafeLock].OutValue   == true &&
                  SW[SwRKSafeLock].OutValue == true,
                  "PART D7d: sensors IDENTICAL to D7c but iControlPanelMode==0 -> golden :16469 kills bSafeLockComflag and :16479 wins instead: ONLY SW[SwSafeLock].On() (:16481), SW[SwRKSafeLock] untouched, return true (:16482)");
        }
        {   // D7e -- golden :16484-16488.  COM flag ONLY.  Needs bSafeLockIOflag
            //   false while Sen[SnRKCoverOpen].Enable stays TRUE -- which is exactly
            //   what simSensorOn gives (Enable true, IsOff() false).  simSensorUnknown
            //   would NOT do: it would also clear Enable and take the :16496 else arm.
            //   :16465 bSafeLockIOflag=false ; :16467 bSafeLockComflag=true
            //   :16471 Enable true -> :16473 false -> :16479 false
            //   :16484 else if(true)   -> :16486 SW[SwRKSafeLock].On()
            //                             :16487 return true
            //   golden writes NOTHING to SW[SwSafeLock] on this path.
            iControlPanelMode = 1;
            simSensorOn(SnRKCoverOpen);
            simSensorOff(SnRKSafeLock);
            SW[SwSafeLock].OutValue   = true;   // must SURVIVE
            SW[SwRKSafeLock].OutValue = false;
            bool r7e = IsSafeLockCheck();
            CHECK(r7e == true &&
                  SW[SwRKSafeLock].OutValue == true &&
                  SW[SwSafeLock].OutValue   == true,
                  "PART D7e: cover-open NOT IsOff but (panel mode 1) safe-lock IsOff -> golden :16484 wins: ONLY SW[SwRKSafeLock].On() (:16486), SW[SwSafeLock] untouched, return true (:16487)");
        }
        {   // D7f -- golden :16489-16494.  NEITHER flag, Enable still TRUE.
            //   :16471 Enable true, :16473/:16479/:16484 all false
            //   :16489 else -> :16491 SW[SwSafeLock].Off()
            //                  :16492 SW[SwRKSafeLock].Off()
            //                  :16493 return false
            iControlPanelMode = 0;
            simSensorOn(SnRKCoverOpen);
            SW[SwSafeLock].OutValue   = true;
            SW[SwRKSafeLock].OutValue = true;
            bool r7f = IsSafeLockCheck();
            CHECK(r7f == false &&
                  SW[SwSafeLock].OutValue   == false &&
                  SW[SwRKSafeLock].OutValue == false,
                  "PART D7f: cover-open ENABLED but neither flag set -> golden :16489 else: BOTH SW[SwSafeLock].Off() (:16491) and SW[SwRKSafeLock].Off() (:16492) fire, return false (:16493)");
        }
        {   // D7g -- golden :16496-16500, THE arm the retired stand-in silently got
            //   wrong: it answered false with NO side effects, but golden turns BOTH
            //   locks OFF here.  This is also this suite's own default state
            //   (allKeysUnknown() disables sensor 27), so it is the arm that runs on
            //   nearly every ScanPannelKey call in this file.
            //   iControlPanelMode is deliberately left at 1 WITH the safe-lock sensor
            //   IsOff, so bSafeLockComflag is TRUE going in -- which proves golden
            //   :16471's Enable gate DOMINATES the com flag rather than being OR-ed
            //   with it (that mattered: golden's own :16471 guard was added by
            //   "Sam 20250603 : 修正 SafeLock 失效問題").
            //   :16471 Sen[SnRKCoverOpen].Enable==false -> :16496 else
            //   :16498 SW[SwSafeLock].Off() ; :16499 SW[SwRKSafeLock].Off()
            //   :16502 Tri_Temp_Machine==1 is false (ported cmydef.cpp:5506, and
            //          nothing in this tree ever assigns it; that arm is #if 0-gated
            //          in the port because golden TempCtrl/TriTemp.cpp is untranslated)
            //   :16507 return false
            iControlPanelMode = 1;
            simSensorUnknown(SnRKCoverOpen);
            simSensorOff(SnRKSafeLock);
            SW[SwSafeLock].OutValue   = true;
            SW[SwRKSafeLock].OutValue = true;
            bool r7g = IsSafeLockCheck();
            CHECK(r7g == false &&
                  SW[SwSafeLock].OutValue   == false &&
                  SW[SwRKSafeLock].OutValue == false,
                  "PART D7g: Sen[SnRKCoverOpen].Enable==false -> golden :16496 else arm runs EVEN WITH bSafeLockComflag true: SW[SwSafeLock].Off() (:16498) and SW[SwRKSafeLock].Off() (:16499) both fire, then :16507 returns false -- these are the side effects the retired stand-in silently dropped");
            //AI(W906-TRIDOOR6) 20261001: RULINGS_20261001 #5 (census 129 (a)(b) #13) -- golden :16502-16506 is live now:
            //   `if(Tri_Temp_Machine==1) { if(IsTriSafeDoor6LockCheck()) return true; }` (TempCtrl/TriTemp.cpp:989).
            //   D7h1: Tri_Temp_Machine==1, door-6 lock sensor IsOn()==false -> TriTemp :996-1000 SW[SwTriTempSafeDoor6Lock].On(),
            //         false -> :16507 return false.
            //   D7h2: the sensor IsOn() -> TriTemp :1004-1010 SW Off, WAR31003 once, true -> IsSafeLockCheck returns TRUE.
            //   D7h3: Tri_Temp_Machine==0 again -> the arm is skipped (the D7g fall-through).
            {
                bool IsTriSafeDoor6LockCheck();     // TempCtrl/TriTemp.h:72 (body TempCtrl/TriTemp.cpp:989)
                const int oldTri = Tri_Temp_Machine;
                Tri_Temp_Machine = 1;
                simSensorUnknown(SnRKCoverOpen);
                simSensorOff(SnTriTempSafeDoor6Lock);
                SW[SwTriTempSafeDoor6Lock].OutValue = false;
                const bool r7h1 = IsSafeLockCheck();
                CHECK(r7h1 == false && SW[SwTriTempSafeDoor6Lock].OutValue == true,
                      "PART D7h1: Tri_Temp_Machine==1, door 6 lock IsOn()==false -> golden TriTemp :998 SW[SwTriTempSafeDoor6Lock].On(), IsSafeLockCheck false (:16507)");
                const int nErr0 = W906_ShowErrorMessage_Count;
                simSensorOn(SnTriTempSafeDoor6Lock);
                const bool r7h2 = IsSafeLockCheck();
                CHECK(r7h2 == true && SW[SwTriTempSafeDoor6Lock].OutValue == false &&
                      W906_ShowErrorMessage_Count == nErr0 + 1 && W906_ShowErrorMessage_LastCode == "WAR31003",
                      "PART D7h2: door 6 lock IsOn() -> golden TriTemp :1004 SW Off + WAR31003 once, IsSafeLockCheck TRUE (golden :16504-16505)");
                const bool r7h2b = IsSafeLockCheck();
                CHECK(r7h2b == true && W906_ShowErrorMessage_Count == nErr0 + 1,
                      "PART D7h2b: still open -> true again, WAR31003 not repeated (golden TriTemp static blogWrite)");
                Tri_Temp_Machine = 0;
                const bool r7h3 = IsSafeLockCheck();
                CHECK(r7h3 == false, "PART D7h3: Tri_Temp_Machine==0 -> the tri-temp arm is skipped, :16507 false");
                simSensorOff(SnTriTempSafeDoor6Lock);
                IsTriSafeDoor6LockCheck();          // resets TriTemp's static blogWrite for any later suite
                simSensorUnknown(SnTriTempSafeDoor6Lock);
                SW[SwTriTempSafeDoor6Lock].OutValue = false;
                Tri_Temp_Machine = oldTri;
            }
        }

        iControlPanelMode = 0;              // back to the suite baseline (golden :16466 else arm)
        safeLockRelease();
    }

    // =================================================================
#ifdef W906_SKIP_PART_E_SOFT_SIMULTE
    std::printf("\n-- PART E: SKIPPED (SOFT_SIMULTE makes CheckThermo a bare `return true;`) --\n");
#else
    //  PART E -- [P4] CheckThermo.  golden :151-186, #else arm live here.
    // =================================================================
    std::printf("\n-- PART E: CheckThermo has exactly ONE false path (golden :151-186) --\n");
    {
        // The exhaustive sweep.  If golden's dead loop at :161-183 could ever
        // affect the answer, some cell of this 6 x 5 x 3 grid would break the
        // equation.
        const int heatModes[6] = { HeadOnly, ChamberOnly, HeadChamber,
                                   SocketChamber, HeadSocket, HeadChamberSocket };
        const int temps[5]     = { Tempture_Ambient, Tempture_Hot,
                                   Tempture_AmbientHot, 2, 7 };
        const int coolings[3]  = { 0, 1, -3 };
        int  cells = 0, bad = 0;
        for (int m = 0; m < 6; ++m)
        {
            for (int t = 0; t < 5; ++t)
            {
                for (int c = 0; c < 3; ++c)
                {
                    Temperature.iIndexHeatMode = heatModes[m];
                    LastSet.iTemperature       = temps[t];
                    iHeaterCooling             = coolings[c];
                    bool want = (temps[t] == Tempture_Hot) || (coolings[c] == 0);
                    ++cells;
                    if (CheckThermo() != want) ++bad;
                }
            }
        }
        CHECK(cells == 90 && bad == 0,
              "PART E1: over all 6 eIndexHeatMode values x 5 temperature modes x 3 iHeaterCooling values (90 cells) CheckThermo() == (iTemperature==Tempture_Hot || iHeaterCooling==0) EXACTLY -- golden's for loop at :161-183 is pure `continue` and cannot influence the answer");
        if (bad) std::printf("      %d of %d cells disagreed\n", bad, cells);

        // The three named paths, called out individually so a failure says which.
        LastSet.iTemperature = Tempture_Hot;
        iHeaterCooling       = 5;
        CHECK(CheckThermo() == true,
              "PART E2: iTemperature==Tempture_Hot wins even with iHeaterCooling!=0 -- golden :157-158 is tested BEFORE golden :159-160");

        LastSet.iTemperature = Tempture_Ambient;
        iHeaterCooling       = 5;
        CHECK(CheckThermo() == false,
              "PART E3: iHeaterCooling!=0 with iTemperature!=Tempture_Hot is the ONLY false CheckThermo can return (golden :159-160)");

        iHeaterCooling = 0;
        CHECK(CheckThermo() == true,
              "PART E4: not Hot and not cooling -> true, reached through golden's dead loop to the unconditional `return true` at golden :184 -- the temperature interlock never reports \"not yet at temperature\"");
    }

    // =================================================================
#endif  // W906_SKIP_PART_E_SOFT_SIMULTE

    //  PART F -- [P5] StopAllDestroy's iteration footprint.
    // =================================================================
    std::printf("\n-- PART F: StopAllDestroy footprint -- iMaxRow used as BOTH bounds (golden :191-197) --\n");
    {
        // ---- F1/F2: a REAL non-square in-arm (2 rows x 4 cols) and an out-arm
        //      given DIFFERENT dimensions of its own.
        InArmSuck.iMaxRow  = 2;
        InArmSuck.iMaxCol  = 4;
        OutArmSuck.iMaxRow = 1;      // deliberately smaller than InArmSuck's
        OutArmSuck.iMaxCol = 8;      // deliberately larger  than InArmSuck's
        W906_TMySucker_OffDestroy_ResetAll();
        StopAllDestroy();

        bool f1hit  = InArmSuck.Suck[0][0].W906_GetOffDestroyCount() == 1 &&
                      InArmSuck.Suck[0][1].W906_GetOffDestroyCount() == 1 &&
                      InArmSuck.Suck[1][0].W906_GetOffDestroyCount() == 1 &&
                      InArmSuck.Suck[1][1].W906_GetOffDestroyCount() == 1;
        bool f1miss = InArmSuck.Suck[0][2].W906_GetOffDestroyCount() == 0 &&
                      InArmSuck.Suck[0][3].W906_GetOffDestroyCount() == 0 &&
                      InArmSuck.Suck[1][2].W906_GetOffDestroyCount() == 0 &&
                      InArmSuck.Suck[1][3].W906_GetOffDestroyCount() == 0;
        CHECK(f1hit && f1miss,
              "PART F1: *** GOLDEN BUG *** on a 2x4 in-arm StopAllDestroy sweeps the 2x2 SQUARE only -- columns 2 and 3 are NEVER OffDestroy'd, so their blow-off stays asserted after a \"stop all\" (golden :193 bounds j by iMaxRow, not iMaxCol)");

        bool f2beyondRow = OutArmSuck.Suck[1][1].W906_GetOffDestroyCount() == 1;
        bool f2insideCol = OutArmSuck.Suck[0][2].W906_GetOffDestroyCount() == 0;
        CHECK(f2beyondRow && f2insideCol,
              "PART F2: *** GOLDEN BUG *** OutArmSuck is indexed with INArmSuck's dimensions -- cell [1][1] IS swept (beyond OutArmSuck.iMaxRow==1) while [0][2] is NOT (well inside OutArmSuck.iMaxCol==8) (golden :196)");

        // ---- F4: the test heads use golden's own mix of a COMPILE-TIME row bound
        //      and a RUNTIME column bound (golden :200-207).
        bool f4 = true;
        for (int i = 0; i < MAX_Index_Row; ++i)
            for (int j = 0; j < NEW_MAX_Index_Col; ++j)
                if (FTestSuck.Suck[i][j].W906_GetOffDestroyCount() != 1 ||
                    BTestSuck.Suck[i][j].W906_GetOffDestroyCount() != 1) f4 = false;
        bool f4row2 = FTestSuck.Suck[2][0].W906_GetOffDestroyCount() == 0 &&
                      BTestSuck.Suck[2][0].W906_GetOffDestroyCount() == 0;
        CHECK(MAX_Index_Row == 2 && NEW_MAX_Index_Col == 8 && f4 && f4row2,
              "PART F4: FTestSuck and BTestSuck are swept over exactly MAX_Index_Row(2) x NEW_MAX_Index_Col(8) and row 2 is untouched -- golden :200-207, and unaffected by InArmSuck's bounds");

        // ---- F5: the tray arm gets ONE hard-coded nozzle (golden :208).
        CHECK(CatchTraySuck.Suck[0][0].W906_GetOffDestroyCount() == 1 &&
              CatchTraySuck.Suck[0][1].W906_GetOffDestroyCount() == 0 &&
              CatchTraySuck.Suck[1][0].W906_GetOffDestroyCount() == 0,
              "PART F5: CatchTraySuck gets exactly ONE OffDestroy, at the hard-coded [0][0] -- no loop, no bounds member consulted (golden :208)");

        // ---- F6: the exact total, so a stray extra call anywhere shows up.
        CHECK(W906_TMySucker_OffDestroyTotal == expectedOffDestroyTotal(2) &&
              W906_TMySucker_OffDestroyTotal == 41,
              "PART F6: exactly 41 OffDestroy calls for iMaxRow==2 -- 2*2*2 (in/out arms) + 2*8*2 (test heads) + 1 (tray arm), golden :191-208 and nothing else");

        // ---- F3: THE assertion that kills `j<iMaxCol`.  With iMaxCol==1 an
        //      iMaxCol-bounded inner loop could only ever reach column 0; golden
        //      reaches columns 0..2.
        InArmSuck.iMaxRow = 3;
        InArmSuck.iMaxCol = 1;
        W906_TMySucker_OffDestroy_ResetAll();
        StopAllDestroy();
        bool f3hit = true, f3miss = true;
        for (int i = 0; i < 3; ++i)
            for (int j = 0; j < 3; ++j)
                if (InArmSuck.Suck[i][j].W906_GetOffDestroyCount() != 1 ||
                    OutArmSuck.Suck[i][j].W906_GetOffDestroyCount() != 1) f3hit = false;
        for (int i = 0; i < 4; ++i)
            if (InArmSuck.Suck[i][3].W906_GetOffDestroyCount() != 0) f3miss = false;
        for (int j = 0; j < 8; ++j)
            if (InArmSuck.Suck[3][j].W906_GetOffDestroyCount() != 0) f3miss = false;
        CHECK(f3hit && f3miss &&
              W906_TMySucker_OffDestroyTotal == expectedOffDestroyTotal(3) &&
              W906_TMySucker_OffDestroyTotal == 51,
              "PART F3: with iMaxRow==3 and iMaxCol==1 the swept region is the 3x3 SQUARE (51 calls total) -- an iMaxCol-bounded inner loop could only ever reach column 0, so this pins golden :193 as `j<iMaxRow`");

        InArmSuck.iMaxRow  = svInMaxRow;
        InArmSuck.iMaxCol  = svInMaxCol;
        OutArmSuck.iMaxRow = svOutMaxRow;
        OutArmSuck.iMaxCol = svOutMaxCol;
        W906_TMySucker_OffDestroy_ResetAll();
    }

    // =================================================================
    //  PART G -- [P6] InitialTestDelayStatus.  golden :2529-2587.
    // =================================================================
    std::printf("\n-- PART G: InitialTestDelayStatus literals + source-order priority (golden :2529-2587) --\n");
    {
        // golden's eleven `if`s, in SOURCE order with the index each one reads.
        // Read off golden :2532/:2537/:2542/:2547/:2552/:2557/:2562/:2567/:2572/
        // :2577/:2582 and their literals at :2534/:2539/:2544/:2549/:2554/:2559/
        // :2564/:2569/:2574/:2579/:2584.
        struct Row { int idx; const char *text; int goldenLine; };
        const Row rows[11] = {
            { 11, "Initial delay--By KL",                   2534 },
            {  6, "Initial delay--Press stop time over",     2539 },
            {  3, "Initial delay--Auto clean",               2544 },
            {  2, "Initial delay--Show alarm message",       2549 },
            {  4, "Initial delay--Open heat door",           2554 },
            {  5, "Initial delay--Happen tested time blow",  2559 },
            {  1, "Initial delay--Fisrt device delay",       2564 },
            {  7, "Initial delay--No fullSite",              2569 },
            {  9, "Initial delay--OTD unlock",               2574 },
            {  8, "EOT monitor time over delay",             2579 },
            { 10, "SOT monitor time over delay",             2584 },
        };

        for (int i = 0; i < 15; ++i) bInitialTestDelayStatus[i] = false;
        CHECK(InitialTestDelayStatus() == "",
              "PART G1: no delay flag set -> \"\" (golden :2531 initialiser survives all eleven ifs)");

        for (int r = 0; r < 11; ++r)
        {
            for (int i = 0; i < 15; ++i) bInitialTestDelayStatus[i] = false;
            bInitialTestDelayStatus[rows[r].idx] = true;
            char msg[220];
            std::sprintf(msg,
                "PART G2[%d]: bInitialTestDelayStatus[%d] alone -> \"%s\" (golden :%d)%s",
                r, rows[r].idx, rows[r].text, rows[r].goldenLine,
                (rows[r].idx == 1) ? "  <-- GOLDEN MISSPELLING \"Fisrt\" PRESERVED" : "");
            CHECK(InitialTestDelayStatus() == AnsiString(rows[r].text), msg);
        }

        for (int i = 0; i < 15; ++i) bInitialTestDelayStatus[i] = false;
        bInitialTestDelayStatus[0]  = true;
        bInitialTestDelayStatus[12] = true;
        bInitialTestDelayStatus[13] = true;
        bInitialTestDelayStatus[14] = true;
        CHECK(InitialTestDelayStatus() == "",
              "PART G3: indices 0, 12, 13 and 14 are NEVER read by golden :2529-2586 -- setting all four still yields \"\" (the array is declared [15], cmydef.cpp:4564)");

        for (int i = 0; i < 15; ++i) bInitialTestDelayStatus[i] = false;
        bInitialTestDelayStatus[11] = true;
        bInitialTestDelayStatus[1]  = true;
        CHECK(InitialTestDelayStatus() == "Initial delay--Fisrt device delay",
              "PART G4: [11] and [1] both set -> [1] wins. Priority is SOURCE order (golden :2532 then :2562), NOT index order");

        for (int i = 0; i < 15; ++i) bInitialTestDelayStatus[i] = false;
        bInitialTestDelayStatus[1]  = true;
        bInitialTestDelayStatus[10] = true;
        CHECK(InitialTestDelayStatus() == "SOT monitor time over delay",
              "PART G5: [1] and [10] both set -> [10] wins, because golden :2582 is the LAST if and overwrites (golden :2564 -> :2584)");

        for (int i = 0; i < 15; ++i) bInitialTestDelayStatus[i] = true;
        CHECK(InitialTestDelayStatus() == "SOT monitor time over delay",
              "PART G6: every flag set -> \"SOT monitor time over delay\" -- eleven non-exclusive ifs, last write wins (golden :2582-2585)");
    }

    // =================================================================
    //  PART H -- [P7] DoScanLevelSensor.  golden :2408-2430.
    // =================================================================
    std::printf("\n-- PART H: DoScanLevelSensor priority is source order, lowest privilege wins (golden :2408-2430) --\n");
    {
        simSensorUnknown(SnFPLevelOpe);
        simSensorUnknown(SnFPLevelEng);
        simSensorUnknown(SnFPLevelSup);
        simSensorUnknown(SnFPLevelHon);
        CHECK(DoScanLevelSensor() == -1,
              "PART H1: no badge presented -> -1 (golden :2410 initialiser, no branch taken)");

        simSensorOn(SnFPLevelOpe);
        CHECK(DoScanLevelSensor() == 0, "PART H2a: Ope alone -> 0 (golden :2413-2416)");
        simSensorUnknown(SnFPLevelOpe);

        simSensorOn(SnFPLevelEng);
        CHECK(DoScanLevelSensor() == 1, "PART H2b: Eng alone -> 1 (golden :2417-2420)");
        simSensorUnknown(SnFPLevelEng);

        simSensorOn(SnFPLevelSup);
        CHECK(DoScanLevelSensor() == 2, "PART H2c: Sup alone -> 2 (golden :2421-2424)");
        simSensorUnknown(SnFPLevelSup);

        simSensorOn(SnFPLevelHon);
        CHECK(DoScanLevelSensor() == 3, "PART H2d: Hon alone -> 3 (golden :2425-2428)");

        // THE pin: Hon (highest privilege, last branch) is already on; add Ope
        // (lowest privilege, first branch).
        simSensorOn(SnFPLevelOpe);
        CHECK(DoScanLevelSensor() == 0,
              "PART H3: *** THE PIN *** Ope AND Hon both presented -> 0 (Ope), NOT 3 -- the else-if chain at golden :2413-2428 makes the LOWEST privilege win");
        simSensorUnknown(SnFPLevelOpe);

        simSensorOn(SnFPLevelEng);
        simSensorOn(SnFPLevelSup);
        CHECK(DoScanLevelSensor() == 1,
              "PART H4: Eng + Sup + Hon, no Ope -> 1 (Eng) -- first matching source branch wins (golden :2417)");
        simSensorUnknown(SnFPLevelEng);

        CHECK(DoScanLevelSensor() == 2,
              "PART H5: Sup + Hon -> 2 (Sup) -- same rule one step down (golden :2421)");
        simSensorUnknown(SnFPLevelSup);
        simSensorUnknown(SnFPLevelHon);
    }

    // =================================================================
    //  PART I -- [P8] the two DEFERRED stubs, and DoSystemMessage's schedule.
    //  See EXCLUSION X1: golden's real bodies are deliberately absent and must
    //  NOT be tested for.
    // =================================================================
    std::printf("\n-- PART I: ShowRunLed and ShowRunLabel are both LIVE since MSTATE-P2/P2b (still counted) --\n");
    {
        W906_ShowRunLedLabel_ResetCounts();
        CHECK(W906_ShowRunLed_Count == 0 && W906_ShowRunLabel_Count == 0,
              "PART I1: W906_ShowRunLedLabel_ResetCounts() zeroes BOTH counters (ckernel.cpp:1500-1504)");

        snapMachine(s_snapBefore);
        ShowRunLed();
        snapMachine(s_snapAfter);
        int dLed = diffMachine(s_snapBefore, s_snapAfter);
        CHECK(W906_ShowRunLed_Count == 1 && W906_ShowRunLabel_Count == 0,
              "PART I2: ShowRunLed() bumps ONLY W906_ShowRunLed_Count (ckernel.cpp:1513-1515)");
        //AI(W906-MSTATE-P2b) 20260924: I3 RECALIBRATED -- ShowRunLed is no longer a deferred stub
        //  (ckernel.cpp:1604 is the live golden body :704-932).  "changes NOTHING" was the stub's
        //  property.  What golden's body may touch among THIS snapshot's observables, read off the body:
        //    * SW[] -- ONLY its twelve physical outputs: SwMusic1+0..3, SwTowerRed/Yellow/Green,
        //      SwFKAlarmReset, SwRKAlarmReset (ckernel.cpp banner; grep of :1604-:1832 finds exactly these).
        //    * Sen[].State -- ONLY what IsEMGPressed() (csystem.cpp:19582) reads, which ShowRunLed calls at
        //      golden :708: SnFrontLeftEMG/FrontRightEMG/RearLeftEMG/RearRightEMG/SnAllEMG/SnServo.
        //    * RunState / bAlarmBuzzer / bNeedMusic... / the three lamp latches -- MAY move (banner list).
        //  NOT COVERED any more (moved to e2e): WHICH tower colour / buzzer state golden selects for a
        //  given machine state; this only pins that nothing OUTSIDE ShowRunLed's own outputs moves.
        {
            const int kLedSw[9] = { SwMusic1 + 0, SwMusic1 + 1, SwMusic1 + 2, SwMusic1 + 3,
                                    SwTowerRed, SwTowerYellow, SwTowerGreen, SwFKAlarmReset, SwRKAlarmReset };
            const int kEmgSen[6] = { SnFrontLeftEMG, SnFrontRightEMG, SnRearLeftEMG, SnRearRightEMG, SnAllEMG, SnServo };
            int swOwn = 0, swOther = 0, senOwn = 0, senOther = 0;
            for (int i = 0; i < MAX_SWITCH_ITEM; ++i)
            {
                if (s_snapBefore.swOut[i] == s_snapAfter.swOut[i] &&
                    s_snapBefore.swSet[i] == s_snapAfter.swSet[i] &&
                    s_snapBefore.swEn[i]  == s_snapAfter.swEn[i]) continue;
                bool own = false;
                for (int k = 0; k < 9; ++k) if (kLedSw[k] == i) own = true;
                if (own) ++swOwn; else { ++swOther; std::printf("      SW[%d] moved -- not a ShowRunLed output\n", i); }
            }
            for (int i = 0; i < MAX_SENSOR_ITEM; ++i)
            {
                if (s_snapBefore.senState[i] == s_snapAfter.senState[i]) continue;
                bool own = false;
                for (int k = 0; k < 6; ++k) if (kEmgSen[k] == i) own = true;
                if (own) ++senOwn; else { ++senOther; std::printf("      Sen[%d] moved -- not an input IsEMGPressed reads\n", i); }
            }
            std::printf("      ShowRunLed moved: %d observables total, SW(own) %d, SW(other) %d, Sen(EMG) %d, Sen(other) %d\n",
                        dLed, swOwn, swOther, senOwn, senOther);
            CHECK(swOther == 0 && senOther == 0,
                  "PART I3: live ShowRunLed() moves no SW[] outside its twelve physical outputs and no Sen[] outside "
                  "the six IsEMGPressed() inputs (golden :704-932, csystem.cpp:19582)");
        }

        snapMachine(s_snapBefore);
        ShowRunLabel();
        snapMachine(s_snapAfter);
        int dLabel = diffMachine(s_snapBefore, s_snapAfter);
        CHECK(W906_ShowRunLed_Count == 1 && W906_ShowRunLabel_Count == 1,
              "PART I4: ShowRunLabel() bumps ONLY W906_ShowRunLabel_Count and leaves the led counter at 1 (ckernel.cpp:1585-1587)");
        //AI(W906-MSTATE-P2) 20260924: I5 RECALIBRATED -- ShowRunLabel is no longer a deferred stub.
        //  ckernel.cpp:1940 is now the live golden body (golden ckernel.cpp:935-1726, the 0-diff copy
        //  un-gated), so "changes NOTHING" is the scaffolding's property, not golden's.  What golden's body
        //  may and may not touch among THIS snapshot's observables, read off the body itself:
        //    * Sen[].State -- MAY move, but only for the inputs it reads: six directly (golden :998-1015:
        //      SnFrontLeftEMG/SnFrontRightEMG/SnRearLeftEMG/SnRearRightEMG/SnAllEMG/SnMotorPower) plus SnServo,
        //      read by IsEMGPressed() (csystem.cpp:19589) which ShowRunLabel calls at golden :983.  The banner's
        //      "Sen[] reads 6" counts direct reads only; the 7th was found by this assertion (Sen[268]=SnServo).
        //      TMySensor::IsOn/IsOff write State as a side effect of reading (the [P8] note above).
        //    * SW[] -- must NOT move: ShowRunLabel has zero SW[] writes (ckernel.cpp's own banner,
        //      "SW[] writes 0"); the 12 physical outputs belong to ShowRunLed, still deferred.
        //    * RunState / bAlarmBuzzer / bNeedMusic... / the three lamp latches -- must NOT move: those
        //      writers are ShowRunLed's (golden :740-897), not this function's.
        //  NOT COVERED any more by I5 (moved to e2e): the status WORD itself and the state ShowRunLabel
        //  really writes (bHALTing, bSECSGEMAlarm, tUPH_PauseStartTime ... -- banner list) are not in this
        //  snapshot; I5b below only checks that a word was painted at all.
        {
            const int kRead[7] = { SnFrontLeftEMG, SnFrontRightEMG, SnRearLeftEMG,
                                   SnRearRightEMG, SnAllEMG, SnMotorPower, SnServo };
            int swMoved = 0, senOther = 0, senSix = 0, misc = 0;
            for (int i = 0; i < MAX_SWITCH_ITEM; ++i)
                if (s_snapBefore.swOut[i] != s_snapAfter.swOut[i] ||
                    s_snapBefore.swSet[i] != s_snapAfter.swSet[i] ||
                    s_snapBefore.swEn[i]  != s_snapAfter.swEn[i]) ++swMoved;
            for (int i = 0; i < MAX_SENSOR_ITEM; ++i)
            {
                if (s_snapBefore.senState[i] == s_snapAfter.senState[i]) continue;
                bool inSix = false;
                for (int k = 0; k < 7; ++k) if (kRead[k] == i) inSix = true;
                if (inSix) ++senSix; else { ++senOther; std::printf("      Sen[%d] (%s) moved -- not an input ShowRunLabel reads\n", i, Sen[i].Name.c_str()); }
            }
            if (s_snapBefore.runState    != s_snapAfter.runState)    ++misc;
            if (s_snapBefore.alarmBuzzer != s_snapAfter.alarmBuzzer) ++misc;
            if (s_snapBefore.needMusic   != s_snapAfter.needMusic)   ++misc;
            if (s_snapBefore.lampSetp    != s_snapAfter.lampSetp)    ++misc;
            if (s_snapBefore.lampStart   != s_snapAfter.lampStart)   ++misc;
            if (s_snapBefore.lampRetry   != s_snapAfter.lampRetry)   ++misc;
            std::printf("      ShowRunLabel moved: %d observables total, SW %d, Sen(read) %d, Sen(other) %d, misc %d\n",
                        dLabel, swMoved, senSix, senOther, misc);
            CHECK(swMoved == 0 && senOther == 0 && misc == 0,
                  "PART I5: live ShowRunLabel() moves no SW[] output, no RunState/buzzer/lamp latch, and no Sen[] "
                  "outside the seven inputs it reads (golden :998-1015 + IsEMGPressed's SnServo) -- the rest belong to ShowRunLed");
            CHECK(fMain != 0 && fMain->palMainStatus != 0 && fMain->palMainStatus->Caption.Length() > 0,
                  "PART I5b: live ShowRunLabel() painted a status word into palMainStatus->Caption");
            if (fMain != 0 && fMain->palMainStatus != 0)
                std::printf("      palMainStatus->Caption = \"%s\"\n", fMain->palMainStatus->Caption.c_str());
        }

        // -- I6: DoSystemMessage's 1-in-6 duty cycle (golden :1899-1915).  Phase-
        //    INDEPENDENT by construction, because golden makes iMyCounter a
        //    function-local static (:1901) with no reset and this wave did not
        //    invent one (ckernel.cpp:1902-1910).  DoPanelLamp is REAL and fires on
        //    one of these ticks (EXCLUSION X4), so the machine snapshot is taken
        //    around the whole loop and restored afterwards -- PART I6 asserts the
        //    COUNTS only.
        snapMachine(s_snapBefore);
        W906_ShowRunLedLabel_ResetCounts();
        for (int t = 0; t < 6; ++t) DoSystemMessage();
        bool i6a = (W906_ShowRunLed_Count == 1 && W906_ShowRunLabel_Count == 1);
        for (int t = 0; t < 6; ++t) DoSystemMessage();
        bool i6b = (W906_ShowRunLed_Count == 2 && W906_ShowRunLabel_Count == 2);
        CHECK(i6a && i6b,
              "PART I6: DoSystemMessage() dispatches ShowRunLed+ShowRunLabel exactly ONCE per SIX ticks (1 after 6 ticks, 2 after 12) -- golden :1903-1914's 6-phase round robin, measured phase-independently");

        W906_ShowRunLedLabel_ResetCounts();
    }

    // =================================================================
    //  PART J -- InitDoArmZHome is still never reached by this suite.
    //AI(W906-W7-L2-substrate) 20260803: RE-DERIVED, not deleted.  The old form
    //  counted calls into this file's own empty fill; that fill is retired, so the
    //  assertion now watches the REAL body's own effect instead.  golden
    //  csystem.cpp:4858-4862 sets iAllArmZHomeTask=1 AND iAllArmZHomeCount=0, and
    //  InitDoArmZHome is the ONLY writer of iAllArmZHomeCount anywhere in the ported
    //  tree (golden's other writers all live in DoArmZHome, golden :5537-5600, still
    //  untranslated), so a surviving sentinel is proof the function was never
    //  entered.  Strictly stronger than the retired counter: it observes the real
    //  golden body, and it would also catch a call made from anywhere else in the
    //  linked image rather than only from this TU.  iAllArmZHomeTask is deliberately
    //  NOT used as the sentinel -- InitAllProcessTask (ported csystem.cpp) also
    //  writes it, so it has more than one writer.
    // =================================================================
    std::printf("\n-- PART J: InitDoArmZHome (golden csystem.cpp:4858-4862) is never reached --\n");
    CHECK(iAllArmZHomeCount == 23117,
          "PART J: the iAllArmZHomeCount sentinel survived the whole suite, so InitDoArmZHome() -- now the REAL two-assignment golden body in csystem.cpp, no longer this file's empty fill -- was never entered, and its behaviour therefore cannot silently affect any assertion above");

    // -----------------------------------------------------------------
    //  RESTORE everything.
    // -----------------------------------------------------------------
    senRestoreAll();
    SystemInitialOK        = svSystemInitialOK;
    bEnableEmployeeIDCheck = svEmployeeIDCheck;
    bFrontPadActive        = svFrontPadActive;
    FlushFlag              = svFlushFlag;
    bLampManualSetp        = svLampSetp;
    bLampManualStart       = svLampStart;
    bLampRetry             = svLampRetry;
    bButtonManualStep      = svButtonStep;
    bButtonManualTStart    = svButtonTStart;
    bSTEP                  = svSTEP;
    bTSTART                = svTSTART;
    bPushHomeDetect        = svPushHomeDetect;
    iAseHome               = svAseHome;
    CosFunction.bEnableSoftWareControlButton = svSoftBtn;
    IniConfig.bDisibleResetButton            = svDisibleReset;
    LastSet.iTemperature       = svTemperatureMode;
    iHeaterCooling             = svHeaterCooling;
    Temperature.iIndexHeatMode = svIndexHeatMode;
    InArmSuck.iMaxRow  = svInMaxRow;
    InArmSuck.iMaxCol  = svInMaxCol;
    OutArmSuck.iMaxRow = svOutMaxRow;
    OutArmSuck.iMaxCol = svOutMaxCol;
    fContact->fShow       = svContactShow;
    fContact->bSetupStep  = svContactSetupStep;
    fContact->bSetupStart = svContactSetupStart;
    fContact->cbOneTouchAutoContactHight->Checked = svOneTouch;
    fContact->btnTStep->Caption  = svCapStep;
    fContact->btnTStart->Caption = svCapStart;
    fMain->BtnSTEP->Color    = svColStep;
    fMain->BtnT_Start->Color = svColStart;
    RunState             = svRunState;
    bAlarmBuzzer         = svAlarmBuzzer;
    bNeedMusicAndAlarmOn = svNeedMusic;
    fNote->fShow         = svNoteShow;
    for (int i = 0; i < 15; ++i) bInitialTestDelayStatus[i] = svDelay[i];
    clearAseFlags();
    W906_TMySucker_OffDestroy_ResetAll();
    //AI(W906-W7-L2-substrate) 20260803: was `g_simSafeLock = false;`.  Sen[] (both
    // SnRKCoverOpen and SnRKSafeLock) is already restored wholesale by the
    // senRestoreAll() above, so nothing here may touch Sen[] again -- these are the
    // non-sensor globals the real IsSafeLockCheck path reaches.
    iControlPanelMode         = svControlPanelMode;
    SW[SwSafeLock].OutValue   = svSwSafeLockOut;
    SW[SwRKSafeLock].OutValue = svSwRKSafeLockOut;
    iAllArmZHomeCount         = svAllArmZHomeCount;

    std::printf("\n=== %d PASS, %d FAIL (of %d) ===\n", g_pass, g_fail, g_pass + g_fail);
    return (g_fail == 0) ? 0 : 1;
}
