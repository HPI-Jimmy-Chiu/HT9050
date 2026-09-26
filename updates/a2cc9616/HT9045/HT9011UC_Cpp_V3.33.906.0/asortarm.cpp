// =============================================================================
//  asortarm.cpp  --  SORT-ARM (HT-9046AU 2nd out-arm) state machines
//
//  Faithful translation of golden asortarm.cpp (5060 lines, BCB6, Big5/cp950).
//  Translator: AI(W906-PT-W4-asortarm) 20260808
//  Translation wave: W906-PT-W4 (the deps-heavy tier of genuinely-missing
//  non-form golden units), unit asortarm.cpp.
//
//  ROLE
//  ----
//  The HT-9046AU machine has a SECOND output arm ("sort arm") whose only job is
//  the Auto4/5/6 half of the unloader: it picks tested IC out of the SORT
//  shuttle kit (OutSht3Kit, which the ordinary out-arm loads via
//  DoPlaceToSortShuttle_2x4_8_AU) and places them into the Auto4/5/6 trays,
//  with its own X/Y/Z/Pitch motors (MOutSortX / MOutSortY / MOutSortSht /
//  MOutSortPitchX), its own nozzle grid (OutArm2Suck), its own offsets
//  (SortArmOffSet[]) and its own production-record grid (SortArmPordRec).
//  This file is the sort-side MIRROR of aoutarm.cpp: DoSortArm is the top-level
//  SM (dispatched from csystem.cpp when USE_OUT_SORT_ARM != eartUninstall and
//  iInArmType is e9045_2x4_8 or e9045_2x8_8), DoSortPickFromShuttle{,_2x4_8,
//  _2x8_8} is the shuttle-pick SM, DoSortArmPlaceToAuto /
//  DoSortArmPlaceToWitchAuto is the place-to-tray SM, and
//  CheckSortArmDestroyActive is the blow-off ("destroy") verification SM.
//
//  Offline this whole engine is INERT but LINKED: USE_OUT_SORT_ARM defaults to
//  eartUninstall, so DoSortArm returns at golden :357 on the very first line of
//  real work.  Every body still links, which is the point of the wave -- it
//  replaces six no-op stand-ins with the real code (see INTEGRATION below).
//
//  WAVE SCOPE -- every golden function, one line each, with its golden line.
//  71 definitions: 68 free functions ACTIVE, 1 free function GATED-OUT-WHOLE,
//  and the 3 TMyQASample methods GATED-OUT-WHOLE (golden defines them twice --
//  see GATE REGISTER (7)/(8)).
//    CheckSortArmZ                             golden :73    ACTIVE
//    bSortShtwaitPick                          golden :99    ACTIVE
//    CheckSortArmCleanOut                      golden :109   ACTIVE
//    MoveSortArmToAutoSafe                     golden :131   ACTIVE
//    CheckSortArmInitState                     golden :230   ACTIVE (one gated call, GATE (1))
//    InitPickFromSortShtTask                   golden :275   ACTIVE
//    PickFromSortSht                           golden :280   ACTIVE
//    InitSortPlaceToAutoTask                   golden :292   ACTIVE
//    InitSortArmTask                           golden :301   ACTIVE
//    SortArmAddSpeed                           golden :306   ACTIVE
//    SortArmSubSpeed                           golden :335   ACTIVE
//    DoSortArm                                 golden :349   ACTIVE (two gated calls, GATE (3)/(4))
//    Find_SortArm_PickerMaxUseCountOnTime      golden :1142  ACTIVE
//    Find_SortArm_Single                       golden :1419  ACTIVE
//    AdjustSortArmClosePitchCondition          golden :1633  ACTIVE
//    CheckSortSuckICFallDown                   golden :1673  ACTIVE
//    ShowSortShuttleDataMiss                   golden :1887  ACTIVE
//    SortArmAddBlockPitch                      golden :1892  ACTIVE
//    MoveOutArmZToPlateSafe                    golden :1906  ACTIVE  (out-arm-named, but asortarm.cpp
//                                                            is its SOLE golden home -- golden
//                                                            aoutarm.cpp does NOT define it; verified
//                                                            this wave)
//    SortTransferAutoRatio                     golden :2007  ACTIVE
//    SortTransferOutShuttleRatio               golden :2029  ACTIVE
//    SortArmPickShuttleAlarm                   golden :2064  ACTIVE
//    PorcessJAM0201SortArmPickUpErrorSkip      golden :2160  ACTIVE
//    GetSortArmXToShuttleOffset                golden :2174  ACTIVE
//    GetSortArmYToShuttleOffset                golden :2181  ACTIVE
//    AutoCalculateSortArmYClosePitch           golden :2202  ACTIVE (golden body is `return 0` + a
//                                                            fully commented-out remainder)
//    CheckOutArmSuckOnlyGOrH                   golden :2249  GATED-OUT-WHOLE -- GATE (7)
//    AutoCalculateSortArmXClosePitch           golden :2273  ACTIVE
//    GetSortArmToUnLoaderPosition              golden :2348  ACTIVE
//    TMyQASample::TMyQASample                  golden :2442  GATED-OUT-WHOLE -- GATE (8)
//    TMyQASample::ClearCount                   golden :2447  GATED-OUT-WHOLE -- GATE (8)
//    TMyQASample::FindDeviceToQA               golden :2455  GATED-OUT-WHOLE -- GATE (8)
//    GetSortArmPitchX                          golden :2459  ACTIVE
//    GetSortArmZDownPos                        golden :2471  ACTIVE
//    GetSortArmToSortShuttleOffset             golden :2508  ACTIVE
//    SetSortArmNeedDestory                     golden :2517  ACTIVE
//    MoveSortArmXYToSortShtWait                golden :2523  ACTIVE
//    SetSortArmHome                            golden :2553  ACTIVE
//    DoSortArmAfterPlaceToAuto                 golden :2573  ACTIVE
//    InitialSortArmNeedSuck                    golden :2650  ACTIVE
//    SetSortArmNeedSuck                        golden :2674  ACTIVE (one gated call, GATE (2))
//    SortArmZNeedDown_2x4_8                    golden :2688  ACTIVE
//    CheckSortArmXYPitch_2x4_8                 golden :2794  ACTIVE
//    GetSortShuttleStatus_Pick                 golden :2872  ACTIVE
//    IsCheckSortArmDestroyActiveFinish         golden :2895  ACTIVE
//    DoSortArmSuckPreOn                        golden :2924  ACTIVE (golden body's own working code is
//                                                            already commented out by golden)
//    SwapShuttleDataToSortArm                  golden :2955  ACTIVE
//    DoSortPickFromShuttle_2x4_8               golden :3026  ACTIVE
//    DoSortArmPlaceToWitchAuto                 golden :3292  ACTIVE (five gated calls, GATE (5)/(6)/
//                                                            (9)/(10)/(11))
//    DoSortArmPlaceToAuto                      golden :3709  ACTIVE (one gated call, GATE (12))
//    AutoCalculateSortArmXClosePitch_Sht_2x4   golden :3903  ACTIVE (golden body is `return 0`, whole
//                                                            remainder commented out by golden --
//                                                            golden's own marker: 還不能用)
//    AdjustSortArmClosePitchCondition_Sht      golden :3964  ACTIVE (golden body is EMPTY, ditto)
//    DoMoveSortArmXYToPlace                    golden :3983  ACTIVE (4-param; see asortarm.h (Q2))
//    SearchSortShuttleUpDown                   golden :4043  ACTIVE (golden body is `return true`, ditto)
//    Find_SortArm_PickerMaxUseCountOnTime_Sht  golden :4074  ACTIVE (golden body is `return 0`, ditto)
//    CheckSortArmDestroyActive                 golden :4286  ACTIVE
//    SearchUnLoadTrayUpDown_Sort               golden :4399  ACTIVE
//    SetSortArm                                golden :4432  ACTIVE
//    MoveSortArmToShuttleIncludeZ              golden :4449  ACTIVE
//    DoSortPickFromShuttle                     golden :4464  ACTIVE
//    GetNowSortShuttleMode_2x4_8               golden :4487  ACTIVE
//    MoveSortArmToShuttleIncludeZ_2x4_8        golden :4589  ACTIVE
//    GetNowSortShuttleMode_2x8_8               golden :4639  ACTIVE
//    SortArmZNeedDown_2x8_8                    golden :4669  ACTIVE
//    CheckSortArmXYPitch_2x8_8                 golden :4711  ACTIVE
//    MoveSortArmToShuttleIncludeZ_2x8_8        golden :4761  ACTIVE
//    DoSortPickFromShuttle_2x8_8               golden :4802  ACTIVE
//  Plus golden's 37 file-scope globals (golden :35-71, :2268-2271, :2870-2871,
//  :3898-3901) -- all ACTIVE, all transcribed with golden's initialisers.
//
//  SATISFIED-BY-SUBSTRATE (real bodies already in this tree; no stub invented):
//    OutArm2Suck / OutArmSuck / InArmSuck + TMyKitSuck + TMySucker + TMyTray
//      -- aHotPlateSubstrate.{h,cpp} (see (T5) below on WHICH TMyKitSuck).
//    OutSht3Kit                    -- acarry_shims.cpp:67 (sole registered def).
//    MOT[] / TMyMotor / TTrayMotor / SortArmZSafe / OutArmZSafe /
//      SortArmContinuousMove / sSortArmZHomeState / ZSafePos -- Motor/mymotor.{h,cpp}.
//    SortArmOffSet[] (ARM_OFFSET::GetX/GetY/GetXPitch) -- cprod.{h,cpp}:71.
//    Prod / TestIF / TestIF_File / ArmSpeed / AutoArmSpeed / AutoForm[] /
//      LoadForm / DeviceForm / ReversionEmptyPoint / ReserveEmptyPoint -- cprod.h.
//    LastSet / IniConfig / ShowErrorMessage / ShowMyMessage / MyDBIProcess /
//      RecordProcess / K_RETRY|K_SKIP|K_HOME / __FUNC__ -- canary_support.h.
//    CosFunction                   -- cpublic.h.
//    ShuttleHasIC / IndexHasIC / HasICUnderMachine / HasICUnderHotPlate /
//      IsInArmCleanOutFinish / IsOutArmCleanOutFinish / SortSHTInLF /
//      DetectAutoTray / InitDoArmZHome -- csystem.h (+ acarry_shims.cpp:108 for
//      SortSHTInLF, csystem_predicates.cpp for the HasIC family).
//    SearchTrayToPlace_9045 / Check_QA_ModeUnloadCount / iOUTARM_SUCK
//                                  -- aoutarm9045.{h,cpp}:1510/:1516/:49.
//    GetShuttleCol                 -- ainarm9045.cpp:492.
//    WhichAutoNeedTray             -- acatchtray.h / canary_support.
//    CheckBin / CheckOutputShuttleDataMiss / MoveOutArmXY_ToFix_Tray_Full /
//      IfUseOnebyOne / SetOutArmHome / ShowOutputShuttleDataMiss /
//      OutArmPickShuttleAlarm / bOutArmSuckActive / class TMyQASample
//                                  -- aoutarm.{h,cpp}  *** SEE (T1) ***
//    MoveOutArmXY_ToShuttleAlarmArea / IsCatchTrayReadySupplyNewTray
//                                  -- acarry_shims.cpp:145ish / aoutarm_shims.cpp:90.
//    LotSummary (AddCount / AddByLotCount) -- cSocket.{h,cpp}:955/:961.
//    sl2DMappingLog->MyInsert2DMappingToFile -- cmydef.h + Public/MyStringList.h:263.
//    TMyProductionRecord (AddErrorRecord / AddErrorRecordNoSave /
//      AddTestResultRecord / AddUnloadRecord / InitialRecord / GetAutoX /
//      GetAutoY / GetWhcihAuto / GetLoaderX / GetLoaderY / asBuffer / bUse)
//                                  -- Public/MyProductionRecord.{h,cpp}.
//    NewRecordProcess              -- acatchtray_shims.h:388.
//    Sen[] / SnRKManualStep        -- mysensor.h / cmydef.h.
//    fSortCT->ShowSortIC()         -- forms/fSortCT.{h,cpp}:72/:29  *** SEE (T1) ***
//    fSCKART->iOutputJamCnt        -- forms/fSCKART.h:155.
//    TrayForm                      -- forms/fTrayForm.h (via FormsFacade.h).
//    every scalar global (USE_OUT_SORT_ARM, TRAY_ARM_MODE, iMMAuto[],
//      iTo3Unload[], iByBinTotal[], iAutoCnt, iAutoRight, iFixRight,
//      iFixRightHalf, iBinBoxAtFix, iBinBoxType, iMagAtAuto, iCleanOut,
//      iTestBinCount, iPauseBackUp, iCloseSiteState, iQAModePassCT,
//      iE1..iENotDefinedCount, bSortingAllBinTrayFinish, bResetSortArm(Task),
//      bSortArmZNeedHome[][], bSortArmPitchNeedHomeCheck, bIsSortArmHome,
//      bSortArmPickShtHasRetry, iSortArmXBase/YBase, iSortArmShtX/YCenterPos,
//      SortArmClose_PitchX, OutArmClose_PitchY, iSortArmBlockIndex,
//      iXpitchMax/Min, USE_OUT_SORT_X_PITCH_MAX/MIN, SortOfsSht, SortOfsAuto4,
//      AseIcRecord, asBarCodeErrorSend, ...) -- cmydef.{h,cpp} / MachineType.h.
//
//  ONE GLOBAL DEFINED HERE THAT GOLDEN DEFINES ELSEWHERE -- read this
//  --------------------------------------------------------------------
//  (D1) `TMyProductionRecord SortArmPordRec[_MAX_SUCK_ROW_ITEM][_MAX_SUCK_COL_ITEM]`
//  is golden mykitsuck.cpp:236 (declared golden MyKitSuck.h / this tree's
//  mykitsuck.h:490).  THIS TREE DELIBERATELY DOES NOT REGISTER mykitsuck.cpp
//  (CMakeLists.txt:2011-2024 states why at length -- its TMyKitSuck would
//  collide with aHotPlateSubstrate.h's, which 177 TUs use).  So the array has
//  NO compiled definition anywhere today, and CheckSortArmDestroyActive (golden
//  :4357-4376) is the only consumer in the whole tree.  Rather than gate away
//  golden's "clear the unloader-tray cell when a destroy error drops the IC"
//  behaviour, this file DEFINES the array, exactly the way acarry_shims.cpp:67
//  already relocates mykitsuck.cpp's OutSht3Kit for exactly the same reason
//  ("Sole def").  It is behind #ifndef HT9045_SORTARMPORDREC_DEFINED so that
//  the day mykitsuck.cpp is registered the fix is a one-line deletion, and a
//  duplicate would be a LOUD link error, never a silent divergence.
//  STATIC-INIT SAFETY (trap 4): TMyProductionRecord's ctor
//  (Public/MyProductionRecord.cpp:135) does `asBuffer = new TStringList()` and
//  fills its own asBuffer strings -- it touches NO global object at all, so
//  none of PT_CAMPAIGN_PLAN.md section 8's 18 NULL globals. Same for the file's
//  other non-trivially-constructed globals: TQPF_Timer's ctor (myTimer.h) only
//  calls QueryPerformanceFrequency/Counter, and `TMyQASample QASamSort`'s ctor
//  (body now in aoutarm.cpp, GATE (8)) only ZeroMemory's its own iCount grid.
//  Verified by reading all three ctors this wave.
//
//  TWO CROSS-FILE TRAPS THIS FILE HAD TO NAVIGATE
//  ----------------------------------------------
//  (T1) THE SIBLING aoutarm.cpp LANDED WHILE THIS FILE WAS BEING WRITTEN.
//  `aoutarm.{cpp,h}` did not exist in this tree at 22:55 on 20260808 and did
//  exist at 23:18 (same PT-W4 wave, parallel group).  FIVE of the symbols this
//  file needs come from it -- CheckBin(int&,int) (aoutarm.h:122 / .cpp:597),
//  CheckOutputShuttleDataMiss (aoutarm.h:214 / .cpp:2603),
//  MoveOutArmXY_ToFix_Tray_Full(bool=false) (aoutarm.h:104 / .cpp:306),
//  bOutArmSuckActive[][] (aoutarm.h:73 / .cpp:34) and `class TMyQASample`
//  (aoutarm.h:218) -- so this file's link closure DEPENDS on that sibling
//  landing and being registered.  As of this wave aoutarm.cpp still carries a
//  "PLACEHOLDER BANNER -- replaced at the end of the wave" first line and does
//  NOT yet compile clean on its own, so the dependency is REPORTED, not
//  assumed.  It is also why the include list below pulls aoutarm.h rather than
//  re-declaring those five.
//  (T2) aoutarm.h:63 `#include "mykitsuck.h"` is INSIDE that header's own gate
//  (H1) and does not fire -- verified by compiling a probe TU that includes
//  aoutarm.h then aHotPlateSubstrate.h and checking that MyKitSuckH is NOT
//  defined afterwards while aHotPlateSubstrateH is.  Had it fired, this TU
//  would have had TWO different `class TMyKitSuck` layouts (trap 5) and would
//  not compile.  Re-verify that probe if aoutarm.h's gate (H1) is ever retired.
//
//  (T5) WHICH TMyKitSuck / TMySucker THIS FILE COMPILED AGAINST: aHotPlateSubstrate.h
//  :365 (the "MINIMAL mirror"), NOT mykitsuck.h:274.  That is the layout the
//  objects it derefs are actually DEFINED with -- OutArm2Suck/OutArmSuck/InArmSuck
//  in aHotPlateSubstrate.cpp:80-93, and OutSht3Kit in acarry_shims.cpp:67 (whose
//  own header acarry_shims.h:32 includes aHotPlateSubstrate.h for exactly that
//  type).  mykitsuck.cpp's rival definitions of all four are in an unregistered
//  TU.  Checked by reading both definitions, not inferred from the include line.
//
//  LINK CLOSURE, MEASURED NOT ASSUMED
//  ----------------------------------
//  Measured by `nm -C --undefined-only` on this file's own .o (214 undefined
//  symbols; 188 after dropping libstdc++/CRT/vclcompat), then resolving each
//  against the .cpp files actually listed in CMakeLists.txt.  183 of 188 resolve
//  to a REGISTERED .cpp.  The remaining FIVE all live in the sibling
//  `aoutarm.cpp`, which is NOT yet in CMakeLists.txt -- so this file CANNOT LINK
//  until the main loop registers it (see (T1)):
//    CheckBin(int&,int)                          aoutarm.cpp  (golden :597ish)
//    CheckOutputShuttleDataMiss(TMyKitSuck*,int,int)  aoutarm.cpp (golden :2603ish)
//    TMyQASample::TMyQASample()                  aoutarm.cpp  (golden :3580)
//    bOutArmSuckActive[][]                       aoutarm.cpp  (golden :34)
//    IfUseOnebyOne(int)                          aoutarm.cpp  (golden :2621)
//  NOTE on the last one specifically: aoutarm9045.cpp:165 also has an
//  IfUseOnebyOne, but it is `static` -- INTERNAL linkage, invisible from here --
//  so it does NOT satisfy this reference.  The line numbers above are golden's;
//  the port's shift as that sibling is still being edited.
//
//  GATE REGISTER -- 14 entries: 13 are `#if 0 / #else / #endif` gates that keep
//  golden's text VERBATIM in the `#if 0` arm so retirement is a deletion, and
//  ONE (12) is a TU-local shim following an idiom this tree already established
//  twice.  Numbered (1)-(14); (12) is the shim.
//
//   (1) golden :243 `flag[i][j]=CheckSuckInitialStatus(OutArm2Suck, i, j);`
//       inside CheckSortArmInitState.  GATED: `flag[i][j]=0;`.
//       WHY FAITHFUL: CheckSuckInitialStatus (declared by golden itself at
//       asortarm.cpp:225 as an extern; golden body lives in cinitial.cpp) has
//       NO compiled body anywhere in this port -- its only other consumer,
//       aoutarm9045.cpp's CheckOutArmInitState_9045, is gated for the SAME
//       reason at aoutarm9045.cpp:570 (`#if 0 // TODO(W7) ... CheckSuckInitialStatus`).
//       0 is golden's own "this nozzle's initial vacuum state is fine" value
//       (golden compares `flag[i][j]!=0` and `flag[i][j]>=Vaccum_Initial_Off`).
//       BEHAVIOUR DELTA on a real machine: a sort-arm nozzle that is already
//       holding vacuum (or already blowing) at cycle start is not detected, so
//       WAR0226 is never raised from here and the K_SKIP arm that would mark
//       those nozzles HAS_NULL_IC never runs; CheckSortArmInitState always
//       returns true. Retire together with aoutarm9045.cpp:570.
//       INTERACTION WITH GOLDEN BUG (B1), CHECKED NOT ASSUMED: because bHasFail
//       is now always false, golden's uninitialised `int ret;` (golden :234) is
//       ALWAYS read un-assigned at `if(ret==K_SKIP)` (golden :255).  That is
//       harmless here, and the reason is arithmetic, not luck: the only thing
//       inside that branch is a loop guarded by `flag[i][j]>=Vaccum_Initial_Off`,
//       and Vaccum_Initial_Off==4 (cmydef.cpp:263) while flag[][] is
//       zero-initialised at golden :235 AND set to 0 by this gate -- so 0>=4 is
//       false for every cell and the body (AddErrorRecord + SetItemData
//       HAS_NULL_IC) cannot execute whatever indeterminate value `ret` holds
//       (K_SKIP==0x0002, cmydef.cpp:338).  Golden itself takes this same path
//       whenever all nozzles happen to be clean, so this is golden's bug being
//       reached more often, not a new one being created.
//
//   (2) golden :2679 `OutArm2Suck.Suck[iSuckRow][iSuckCol].SetNeedDestroy(true);`
//       inside SetSortArmNeedSuck's bPlace==true arm.  GATED: nothing.
//       WHY FAITHFUL: aHotPlateSubstrate.h's TMySucker has `bool bNeedDestroy;`
//       and `GetNeedDestroyStatus()` (:147-155 block) but NO SetNeedDestroy
//       SETTER -- the setter exists only on mykitsuck.h:251's rival TMySucker,
//       which this TU does not and must not compile against (T5).  bNeedDestroy
//       is zero-initialised, i.e. false, which is exactly what golden's
//       else-branch leaves it as.  BEHAVIOUR DELTA: the sort arm never marks a
//       nozzle "needs destroy" from the PLACE path.  Reachability today: ZERO --
//       SetSortArmNeedSuck's only two call sites (SortArmZNeedDown_2x4_8 golden
//       :2762/:2767 and SortArmZNeedDown_2x8_8 golden :2692-:2704) both pass
//       `bPlace` initialised to false and never changed (golden :2743 / :4679),
//       so the gated arm is dead code in golden too.  RETIRE BY: adding
//       `void SetNeedDestroy(bool v){bNeedDestroy=v;}` to aHotPlateSubstrate.h's
//       TMySucker (see INTEGRATION REQUESTS) -- a 1-line header widening.
//
//   (3) golden :356 `OutSht3Kit.SetHasNullIcToNullIc();` -- first statement of
//       DoSortArm.  GATED: nothing.
//       WHY FAITHFUL: TMyKitSuck has no SetHasNullIcToNullIc in this tree (it
//       has the differently-named SetNullIcToHasNullIc, which does the OPPOSITE
//       promotion, and SetUnuseAndHasNullICToNullIC, which also clears unused
//       nozzles -- neither is a substitute).  This is an ALREADY-ESTABLISHED
//       absence with three existing consumers, all of which no-op it the same
//       way: atester_32Site.cpp:218/:3424-3425 (W5_32S_SETHASNULLIC) and
//       aTester_Front.cpp:169/:678/:863 (W64B_FRCARRYKIT_SETHASNULLIC).
//       BEHAVIOUR DELTA: HAS_NULL_IC cells in the sort shuttle are not demoted
//       to NULL_IC at the top of each DoSortArm tick, so a sort-shuttle cell
//       that was marked "empty on purpose" stays HAS_NULL_IC.  Same delta the
//       two tester TUs already carry.  Retire all five together.
//
//   (4) golden :381-387 -- the `IniConfig.bAlarmNeedServoOff` /
//       `fNote->bMyServoOffOutArm` early-return in DoSortArm.  GATED: nothing
//       (fall through to CheckSortArmDestroyActive).
//       WHY FAITHFUL: forms/fNote.h states at :87-95 that bMyServoOffOutArm is
//       "NOT LANDED, ON PURPOSE" and names ported aoutarm9045.cpp:388 as an
//       existing consumer that gates it identically.  The facade's TfNote has
//       bMyServoOffInArm only.  BEHAVIOUR DELTA: after an alarm-triggered
//       servo-off on the out/sort arm, DoSortArm keeps ticking instead of
//       parking, so an operator hand-pushing the sort arm during that window
//       would not be protected from a point-position error (golden's stated
//       reason for the flag, JerryYang 20161227).  Retire when TfNote gains the
//       member -- together with aoutarm9045.cpp:388.
//
//   (5) golden :3343-3350 -- `if(fSCKART->iInfo_MultiLotCnt>1){LotSummary.
//       AddByLotCount(...);} else {asInfo.sprintf("picker %d - %d site CH is -1",...);}`
//       GATED: golden's OWN else-branch, verbatim.
//       WHY FAITHFUL: forms/fSCKART.h has iOutputJamCnt (:155) but NOT
//       iInfo_MultiLotCnt; the two existing consumers gate it exactly this way
//       (aTester_Front.cpp:2374-2375, aTester_Rear.cpp:86-88).  Offline a run
//       is single-lot, i.e. iInfo_MultiLotCnt would be 0 or 1, so golden itself
//       would take the else-branch -- the #else arm is not a degraded stand-in,
//       it is the branch golden runs.  BEHAVIOUR DELTA: on a genuine MULTI-lot
//       ART run the per-lot bin tally (LotSummary.AddByLotCount) is not fed
//       from the sort arm, so a multi-lot lot-summary would under-count the
//       Auto4/5/6 half.  LotSummary.AddByLotCount itself IS real
//       (cSocket.cpp:961) -- the missing piece is only the facade member.
//
//   (6) golden :3433-3436 -- the four fBarCode->list2DByLot / ->map2DList /
//       WriteDataToFile statements inside the bBarcodeDuplicateFileByOutArm
//       guard.  GATED: nothing; the guard and all surrounding control flow stay
//       LIVE.
//       WHY FAITHFUL: this tree's fBarCode is `TfBarCode *`
//       (aHotPlateSubstrate.h:975-1017) and that class has neither list2DByLot
//       nor map2DList -- the tree carries them as FREE globals instead
//       (BarCode/BarCode_Bottom2DID.h:144 `extern TStringList *list2DByLot;`),
//       which is a different object, not a rename this file may silently
//       substitute.  The enclosing guard also requires
//       CosFunction.bBarcodeDuplicateFileByOutArm, which is false offline.
//       BEHAVIOUR DELTA: with that customer option ON, the per-lot 2D-ID
//       duplicate list is not appended-to / rewritten from the sort-arm place
//       path, so a duplicate 2DID placed by the SORT arm would not be recorded
//       in asBarCodeLot.  (Golden's own logic here is odd -- it Clear()s the
//       list and then Add()s one code, so the file ends up with a single entry;
//       that is golden's behaviour, kept, not corrected.)
//
//   (7) golden :2249-2266 `bool CheckOutArmSuckOnlyGOrH()` -- GATED-OUT-WHOLE,
//       no #else (this file simply does not define the symbol).
//       WHY FAITHFUL: **GOLDEN DEFINES THIS FUNCTION TWICE** -- asortarm.cpp
//       :2249-2266 and aoutarm.cpp:3013-3030 -- with byte-identical bodies
//       (compared line by line this wave: same 6 OutArmSuck.Item tests, same
//       order, same returns), and golden's HT9045.bpr OBJFILES contains BOTH
//       `..\Obj\aoutarm.obj` AND `..\Obj\asortarm.obj` (grepped the .bpr this
//       wave).  Reproducing both would be a duplicate-symbol link error, and it
//       is not this port's job to reproduce a link ambiguity -- the same
//       decision, with the same reasoning, that OmronLaser/LaserSensorShuttle.cpp's
//       GATE (1) already made for GetRowCol.  This tree's aoutarm.cpp:3008 owns
//       the symbol.  BEHAVIOUR DELTA: none -- the two bodies are identical, and
//       nothing in asortarm.cpp calls it (its only golden callers are on the
//       out-arm side).
//
//   (8) golden :2442-2457 -- `__fastcall TMyQASample::TMyQASample()`,
//       `TMyQASample::ClearCount()`, `TMyQASample::FindDeviceToQA()`.
//       GATED-OUT-WHOLE, no #else.
//       WHY FAITHFUL: identical situation to (7) -- golden defines all three in
//       BOTH aoutarm.cpp (:3580/:3585/:3593) and asortarm.cpp, byte-identically,
//       and both .obj are linked.  golden asortarm.h even has its own copy of
//       the class declaration COMMENTED OUT (:132-145), i.e. golden itself
//       treats aoutarm.h:155 as the owner.  This tree's aoutarm.cpp:3575-3590
//       carries all three.  The OBJECT is NOT duplicated -- `TMyQASample
//       QASamSort;` (golden :70) exists only in asortarm.cpp, so it stays here
//       and is defined below.  BEHAVIOUR DELTA: none.  QASamSort's ctor resolves
//       to aoutarm.cpp's body, which only ZeroMemory's the object's own iCount.
//
//   (9) golden :3597 `fOCR->SaveOCRAndBinData(asOCRCode, OutArm2Suck.PordRec[i][j].
//       asBuffer->Strings[eTestCate], Prod.iIsPassT6[iToWhichAuto]);`  GATED:
//       nothing; the three-term `IniConfig.bOCRAndBinLog && TestIF.bOcrFunction
//       && CosFunction.bTrayOCR` guard stays LIVE.
//       WHY FAITHFUL: forms/fOCR.h's TfOCR declares exactly one method,
//       ChangeLightValue (:33); SaveOCRAndBinData does not exist anywhere in
//       this tree.  BEHAVIOUR DELTA: with the OCR+Bin-log option ON, the
//       sort-arm half of the OCR/bin CSV is not written.  Retire when TfOCR
//       gains the method (see INTEGRATION REQUESTS).
//
//  (10) golden :3599 `OutArm2Suck.PordRec[i][j].SaveRecord();`  GATED: nothing.
//       WHY FAITHFUL: TMyProductionRecord::SaveRecord is EXPLICITLY out of
//       scope tree-wide and not even DECLARED -- Public/MyProductionRecord.h
//       :24-30 lists it first among "STILL OUT OF SCOPE ... NOT declared here",
//       and MyProductionRecord.cpp:36 records that AddErrorRecord itself was
//       left body-less for the same reason.  BEHAVIOUR DELTA: the per-IC
//       production-log row the sort arm just filled in is never flushed to the
//       production CSV.  The row is still BUILT (AddUnloadRecord runs, and the
//       CommaText copy into MOT[Motor].Tray.PordRec at golden :3570 runs), so
//       the in-memory tray mapping is correct -- only the file write is absent.
//       This is the single largest behaviour delta in this file.
//
//  (11) golden :3704 `fSortCT->CheckTheYieldAfterPlaceAuto();`  GATED: nothing.
//       WHY FAITHFUL: forms/fSortCT.h declares ShowLoadingIC and ShowSortIC only
//       (:71-72); CheckTheYieldAfterPlaceAuto is absent, and golden cSortCT.cpp
//       is unported as a whole (forms/fSortCT.h's own note).  BEHAVIOUR DELTA:
//       the low-yield check that golden runs after every sort-arm place is not
//       performed, so a low-yield alarm that should fire from the Auto4/5/6 path
//       does not.  NOTE the sibling call one screen up, golden :3538
//       `fSortCT->ShowSortIC()`, is NOT gated -- that member DOES exist
//       (forms/fSortCT.h:72, body forms/fSortCT.cpp:29, added 20260808 by the
//       PT-W3 integrate pass for Automation/uRENESAS_Server.cpp).  Its body is
//       an offline no-op, which is a different thing from the member being
//       missing, and is why one is gated and the other is not.
//
//  (12) `SetSortArmSpeed(bool)` (golden call sites :317, :330, :345 inside
//       SortArmAddSpeed / SortArmSubSpeed) -- NOT #if 0'd; handled with the
//       tree's established TU-LOCAL SHIM idiom instead, see just below the
//       includes.  WHY that shape rather than a gate: the symbol has no
//       declaration in ANY header in this tree (grepped) and no compiled body,
//       and the two existing consumers each define their own file-scope no-op --
//       AutoClean/AutoClean.cpp:178 (`static void SetSortArmSpeed(bool){}`) and
//       csystem.cpp:1269-1270 (a static + #define pair).  A third #if 0 would
//       just re-decide what has already been decided twice.  BEHAVIOUR DELTA:
//       the sort arm's auto-speed ramp updates AutoArmSpeed[OutArm].iBodySP /
//       iACDCBodySP exactly as golden does (that arithmetic is LIVE), but the
//       new speed is never pushed to the motion controller.
//
//  (13) golden :1936 `MyDBIProcess("Motion", sData, "MoveOutArmZToPlateSafe");`
//       inside MoveOutArmZToPlateSafe's CC_ATEC arm.  GATED: the same call with
//       golden's THIRD argument folded into the message text.
//       WHY FAITHFUL: an ARITY divergence, not a missing feature -- golden's
//       MyDBIProcess is `(AnsiString asTable, AnsiString S1, AnsiString S2="")`
//       (golden cMyDB.h:20) and this tree's is 2-arg (aHotPlateSubstrate.h:924).
//       Folding rather than dropping is the established idiom, spelled out at
//       AutoClean/AutoClean.cpp:1170-1180 ("preserves the information instead of
//       silently dropping it"); canary_support.cpp:389-393 records the same
//       divergence for its own call site.  BEHAVIOUR DELTA: none observable --
//       this tree's MyDBIProcess body (aHotPlateSubstrate.cpp:1072) is a no-op
//       counter/last-value seam, so neither arity writes a DB row.  The sibling
//       `ShowMyMessage(sData, "", "MoveOutArmZToPlateSafe")` on the else-branch
//       (golden :1938) is NOT gated -- ShowMyMessage really does take 3+ args
//       here (canary_support.cpp:130).
//
//  (14) golden :3574-3586 -- the three
//       `...asBuffer->Strings[eErrorCode].Pos("<literal>")>0` conditions of the
//       E1/E2/E3 error-category tally.  GATED: the identical chain with each
//       subject wrapped as `AnsiString(...)`.
//       WHY FAITHFUL: a TYPE adaptation, not a logic change.  vclcompat's
//       TStringList::Strings[] returns a `StringsProxy` (vclcompat/TStringList.h
//       :90-99) that has `operator AnsiString()` but no `.Pos()`; golden's VCL
//       Strings[] returns AnsiString directly.  Every literal, every `>0`
//       threshold, the `||` pairing and the branch ORDER are unchanged, and the
//       trailing `else { iENotDefinedCount++; }` (golden :3587-3590) is left
//       OUTSIDE the gate so it still attaches to the last `else if`.
//       BEHAVIOUR DELTA: none.
//
//  GOLDEN BUGS / ODDITIES FOUND AND DELIBERATELY KEPT (not "fixed")
//  ---------------------------------------------------------------
//   (B1) golden :234/:255 -- `int ret;` is UNINITIALISED and then read by
//        `if(ret==K_SKIP)` on the path where bHasFail is false (so the
//        assignment at :253 never happened).  Reading an indeterminate value.
//        Kept verbatim; with GATE (1) making bHasFail always false, this path
//        is now ALWAYS taken, so the read happens every call.  Flagged in the
//        report as the one gate/bug interaction worth a second opinion.
//   (B2) golden :1335/:1373 -- `iSortArmYPosition=iRow;` where `iRow` is the
//        FUNCTION PARAMETER (Find_SortArm_PickerMaxUseCountOnTime's `int &iRow`),
//        read before it has been assigned on the TRAY_ARM_MODE==eUnderCoveyor
//        path (:1292-1338), and at :1373 assigned from the just-written iRow --
//        golden clearly meant itStartRow in the first case.  Kept verbatim.
//   (B3) golden :2862-2863 (CheckSortArmXYPitch_2x4_8) and :4753-4754
//        (_2x8_8) -- the X and Y offsets are CROSSED:
//        `*iY += GetSortArmXToShuttleOffset(...)` and
//        `*iX += GetSortArmYToShuttleOffset(...)`.  Kept verbatim.
//   (B4) golden :3106 `iShtCol   =j+iPickKitCol;;` -- double semicolon.  Kept.
//   (B5) golden :3365/:3380 `if(asCheck==0 || asCheck=="")` -- comparing an
//        AnsiString to the integer 0 = BCB6 AnsiString(0) = "0" (NB2 R89).  AI(W906-NUMCMP) 20260927:
//        written =="0" below -- vclcompat took 0 as a null const char* ("").
//   (B6) golden :991-998 -- `if(iSortWhichAuto<=6){ Task=3300; if(Task!=3300)
//        break; }` then `Task=3300;` again: the inner test is dead by
//        construction.  Kept.
//   (B7) golden :709 -- a stray `break;` sitting between two commented-out
//        case labels, i.e. it is the tail of `case 1200:`.  Kept exactly where
//        golden put it (it is reachable and it matters).
//   (B8) golden :2405 calls AutoCalculateSortArmXClosePitch(iSortWhichAuto,
//        true) INSIDE GetSortArmToUnLoaderPosition and then immediately
//        recomputes iXPos from dSortArmXPitch_1Step -- so the recompute at
//        :2407 is what takes effect, and the identical assignment at :2392 is
//        overwritten.  Kept.
//
//  VCL/Borland conversions: `#pragma hdrstop` (golden :2) and
//  `#pragma package(smart_init)` (golden :33) dropped; `__fastcall` appears
//  exactly once (golden :2442, the TMyQASample ctor) and is inside GATE (8).
//  No __published / __property / PACKAGE anywhere in this file.  AnsiString
//  (1-based) + AnsiString::sprintf/printf via vclcompat, kept verbatim.
//  SOFT_SIMULTE is NOT defined -> the `#ifndef SOFT_SIMULTE` arms stay ACTIVE
//  and the `#ifdef SOFT_SIMULTE` arms (golden :1675-1677, :3113-3122,
//  :3469-3476, :4891-4900) are compiled OUT but kept VERBATIM.
//
//  INCLUDE REWRITE (mirror of the golden header set, translated subset):
//    golden: MachineDefine.h ; asortarm.h ; aArmHeader.h ; MyKitSuck.h ;
//            cprod.h ; mymotor.h ; mySensor.h ; note.h ; csystem.h ;
//            mymessbox.h ; cinitial.h ; cSortCT.h ; aseTest.h ; main.h ;
//            mycylin.h ; cmydef.h ; common.h ; SCK_ART.h ; InOutArmZteach.h ;
//            cSocket.h ; BarCode.h ; OCR.h ; acatchtray.h ;
//            #pragma package(smart_init)
//  NOT INCLUDED ON PURPOSE: acarry_shims.h.  It declares
//  `void MoveSortArmToAutoSafe();` (:193) while golden's -- and therefore this
//  file's -- definition returns `bool` (golden :131).  Including it is a hard
//  compile error here, and the two definitions would mangle IDENTICALLY on
//  MinGW (return type is not mangled for ordinary functions) so they are also a
//  duplicate symbol.  See INTEGRATION REQUESTS.  The two symbols this file
//  actually needed from it (OutSht3Kit, MoveOutArmXY_ToShuttleAlarmArea) are
//  forward-declared below with golden's own signatures.
//
//  Big5: every Chinese comment decoded via cp950 and preserved as UTF-8.
//  Final gate: ZERO U+FFFD.
// =============================================================================
#include "MachineDefine.h"          // <windows.h> (ZeroMemory), vclcompat
#include "MachineType.h"            // MAX_ARM_Row / MAX_ARM_Col / SortOfsSht / SortOfsAuto4 / eAuto6 / eTrayCount

#include "asortarm.h"
#include "aArmHeader.h"             // guard-only shim in this tree (its golden include list is #if 0)
#include "aoutarm.h"                // class TMyQASample + CheckBin + CheckOutputShuttleDataMiss + MoveOutArmXY_ToFix_Tray_Full + IfUseOnebyOne + SetOutArmHome + ShowOutputShuttleDataMiss + OutArmPickShuttleAlarm + bOutArmSuckActive   -- SEE (T1)
#include "aoutarm9045.h"            // SearchTrayToPlace_9045 / Check_QA_ModeUnloadCount / iOUTARM_SUCK
#include "ainarm9045.h"             // GetShuttleCol
#include "csystem.h"                // ShuttleHasIC / IndexHasIC / HasICUnderMachine / HasICUnderHotPlate / IsInArmCleanOutFinish / IsOutArmCleanOutFinish / SortSHTInLF / DetectAutoTray / InitDoArmZHome
#include "Motor/mymotor.h"          // MOT[] / TMyMotor / TTrayMotor / SortArmZSafe / OutArmZSafe / SortArmContinuousMove / sSortArmZHomeState / ZSafePos
#include "cmydef.h"                 // global scalar universe + IC consts + iInArmType + iSortArm* + USE_OUT_SORT_ARM + iXpitchMax/Min + sl2DMappingLog
#include "cprod.h"                  // Prod / TestIF / TestIF_File / ArmSpeed / AutoArmSpeed / AutoForm[] / LoadForm / DeviceForm / SortArmOffSet[] / ReversionEmptyPoint / ReserveEmptyPoint
#include "cpublic.h"                // CosFunction
#include "common.h"                 // WriteDataToFile / asBarCodeLot
#include "mysensor.h"               // Sen[] / SnRKManualStep
#include "mycylin.h"                // ChungHung 20150303 add for AutoRetest (golden :20)
#include "cSocket.h"                // LotSummary (TLotSummary::AddCount / AddByLotCount)
#include "acatchtray.h"             // WhichAutoNeedTray
#include "aHotPlateSubstrate.h"     // TMyKitSuck / TMySucker (SEE (T5)) + OutArm2Suck / OutArmSuck / InArmSuck + fBarCode
#include "acatchtray_shims.h"       // NewRecordProcess
#include "Public/MyStringList.h"    // TMyStringList::MyInsert2DMappingToFile
#include "FormsFacade.h"            // TrayForm / fSortCT / fSCKART / fNote / fOCR offline stand-ins
#include "canary_support.h"         // ShowErrorMessage / ShowMyMessage / MyDBIProcess / RecordProcess / LastSet / IniConfig / K_RETRY|K_SKIP|K_HOME / DUMMY / __FUNC__

// -----------------------------------------------------------------------------
//  SUBSTRATE EXTERNS this file needs whose declaring header cannot be included
//  here (see the INCLUDE REWRITE note on acarry_shims.h).  Signatures are
//  VERBATIM golden.  These are LINK EXTERNS pointing at REAL bodies that
//  already exist in CMakeLists-registered .cpp files -- not stubs:
//    OutSht3Kit                       -> acarry_shims.cpp:67   (sole def)
//    MoveOutArmXY_ToShuttleAlarmArea  -> acarry_shims.cpp
//    IsCatchTrayReadySupplyNewTray    -> aoutarm_shims.cpp:90
//  Guarded so a later integration that DOES make these visible cannot
//  double-declare.
// -----------------------------------------------------------------------------
#ifndef ASORTARM_SUBSTRATE_FWD
#define ASORTARM_SUBSTRATE_FWD
extern TMyKitSuck OutSht3Kit;                                                   // golden MyKitSuck.h (Steven 20240822, HT-9046AU sort kit)
bool MoveOutArmXY_ToShuttleAlarmArea();                                         // golden aoutarm.h
bool IsCatchTrayReadySupplyNewTray();                                           // golden aoutarm.h:64 / acatchtray.h
#endif // ASORTARM_SUBSTRATE_FWD

// -----------------------------------------------------------------------------
//  GATE (12) -- TU-LOCAL SHIM, established idiom (AutoClean/AutoClean.cpp:178,
//  csystem.cpp:1269-1270).  golden's SetSortArmSpeed body is cinitial.cpp:5425,
//  which is not translated; the symbol has no declaration in ANY header in this
//  tree, so a file-scope static cannot collide with anything.  See the GATE
//  REGISTER entry (12) for the behaviour delta.
// -----------------------------------------------------------------------------
static void SetSortArmSpeed(bool /*bShow*/) {}                  // golden cinitial.cpp:5425 -- not yet translated

// -----------------------------------------------------------------------------
//  (D1) golden mykitsuck.cpp:236's SortArmPordRec, relocated here because
//  mykitsuck.cpp is deliberately unregistered.  Read (D1) in the banner before
//  touching this.  Precedent: acarry_shims.cpp:67 does the same for OutSht3Kit.
// -----------------------------------------------------------------------------
#ifndef HT9045_SORTARMPORDREC_DEFINED
#define HT9045_SORTARMPORDREC_DEFINED
// AI(W906-A4-6) 20260924 退役（mykitsuck.cpp 有 golden 定義，本顆起註冊；兩份會重複定義）：TMyProductionRecord SortArmPordRec[_MAX_SUCK_ROW_ITEM][_MAX_SUCK_COL_ITEM];      //RogerYang 20250515 Add for 9046AU  -- golden mykitsuck.cpp:236 (see banner (D1))
#endif // HT9045_SORTARMPORDREC_DEFINED

//==============================================================================
bool bSortArmXOverLimit=false;                                                   //Steven 20171206 (Wei) : In ATK, 修正Out Arm同時放料但是超出軟體極限縮Pitch導致疊料的問題
bool bSortArmSuckActive[MAX_ARM_Row][MAX_ARM_Col]={{false, false, false, false}, {false, false, false, false}};
bool bSortArmSuckActiveBack[MAX_ARM_Row][MAX_ARM_Col]={{false, false, false, false}, {false, false, false, false}};
bool bSortArmManualStepPress=false;
int SortArmTask=1;
int iDoSortArmAfterPlaceToAutoTask=1;                                            //Steven 20220526 : 針對放下IC到Unloader後的動作做整合
int iSortArmPlaceOrder=0;
int iSortArmPickOrder=0;                                                         //JerryYang 20221215 : Magazine把fix區當buffer區功能
int iSortWhichAuto;

int iPickFromSortShtTask=1;
int iSortPlaceToAutoTask=1;
//TQPF_Timer HInspectWait;
TQPF_Timer DoPickFromSortShtDelay;
TQPF_Timer DoSortPlaceToAutoDelay;

int iSortArmXPosition[MAX_ARM_Row][MAX_ARM_Col]={{0, 0, 0, 0}, {0, 0, 0, 0}};
int iSortArmYPosition=0;

bool bCarryControlSortarm=false;

bool bSortPickShuttleError=false;

bool bSortUseOnebyOne=false;                                                        //2010-12-07

bool bSortOverTray=false;                                                           //jou 2012-09-04 修正極限無法到達時,回填Null_IC會造成找不到地方放,會放在 x=-1,y=-1的位置上

bool bCheckSortArmDestroyActiveFinish[MAX_ARM_Row][MAX_ARM_Col]={{true, true, true, true}, {true, true, true, true}};
bool bSortArmCheckDestroyACT[MAX_ARM_Row][MAX_ARM_Col]={{false, false, false, false}, {false, false, false, false}};
bool bCheckSortArmDestroyActive[MAX_ARM_Row][MAX_ARM_Col]={{false,false,false,false},{false,false,false,false}};
TQPF_Timer CheckSortArmDestroyActiveDelay[MAX_ARM_Row][MAX_ARM_Col];

int iLocModeRow=0;
int iLocModeCol=0;

TMyQASample QASamSort;
TQPF_Timer DoSortSHTWaitTime;                                                       //kevin 20210525 out arm 在shuttle 上等待
//==============================================================================
bool CheckSortArmZ(bool bMessage)
{
    AnsiString S1, S2;
    int iMotZ;
    for(int i=0; i<OutArm2Suck.iPickRow; i++)
    {
        for(int j=0; j<OutArm2Suck.iPickCol; j++)
        {
            iMotZ=OutArm2Suck.Suck[i][j].iMotNo;
            if(MOT[iMotZ].ReadPos()!=Prod.ZOutArmSafe[i][j])
            {
                if(bMessage)
                {
                    ShowErrorMessage("WAR0257", 0, MOutSortX, false, MOT[iMotZ].Alias);       //出料手臂的吸嘴 %s 沒有在安全位置
                }
                return true;
            }
        }
    }
    return false;
}
//==============================================================================
//  Ver : 2003_10_06          0:shuttle 1     1:shuttle 2
//==============================================================================

//==============================================================================
bool bSortShtwaitPick()                                                          //kevin 20210602 Out arm sht on wait
{
    if(ArmSpeed[OutArm].dWaitOnSH!=0)                                           //kevin 20210525 out arm 起動在shuttle 上等待 時間
    {
        if(DoSortSHTWaitTime.Off())
              return true;
    }
    return false;
}
//==============================================================================
int CheckSortArmCleanOut(int Task)
{
    if(iCleanOut==1 &&
       IniConfig.bP27AutoSortingBinTrayByOutArmwhenCleanOut==true &&            //JerryYang 20150910 START:Auto Sorting BinTray by Out Arm when Clean Out
       CosFunction.bSortingBinTraywhenCleanOut==true)                           //JerryYang 20170911 (Steven) Clean Out時整盤
    {
        if(InArmSuck.HasIC()==false   &&
           OutArmSuck.HasIC()==false  &&
           ShuttleHasIC()==false      &&
           IndexHasIC()==false        &&
           OutArm2Suck.HasIC()==false &&
           IsInArmCleanOutFinish()==true)                                       //RogerYang 20170609 (wei) 修正 bP27AutoSortingBinTrayByOutArmwhenCleanOut==false 時, Auto Sorting HangUp 問題
        {
            return 5000;
        }
    }
    return Task;
}
//==============================================================================
//  Ver : 2003_07
// 輸出臂移到自動 TRAY安全高度
//==============================================================================
bool MoveSortArmToAutoSafe()                                                     //只上升吸嘴到安全高度
{
    //return MoveSortArmToAutoSafe_AU();
    bool OK=true;
    int iAlarmSuck=0;
    static int iRetryCount=0;
    static int iRetryFail=0;
    int iMot=0;
    #ifndef SOFT_SIMULTE
    int iZPos=0;
    bool bHomeFlag=false;
    AnsiString sData;
    #endif

    for(int i=0; i<OutArm2Suck.iMotRow; i++)
    {
        for(int j=0; j<OutArm2Suck.iMotCol; j++)
        {
            iMot=OutArm2Suck.Suck[i][j].iMotNo;
            if(MOT[iMot].MotorMove(Prod.ZSortArmSafe[i][j])==false)
            {
                OK=false;
                #ifndef SOFT_SIMULTE
                MOT[iMot].ScanMotorStatus();
                bHomeFlag=MOT[iMot].Led[iHomeLed];
                if(bHomeFlag)                                                   //JerryYang 20200924 : Z軸失步偵測
                {
                   iZPos=MOT[iMot].ReadPos();
                   if(iZPos<-200)
                   {
                        sData.sprintf("%s pos %d home sensor on", MOT[iMot].Alias, iZPos);
                        ShowMyMessage(sData, "", "MoveSortArmToAutoSafe");
                        SetOutArmHome();
                   }
                }
                #endif
            }
        }
    }

    if(OK)
    {
        iAlarmSuck=SortArmZSafe(DETECT_ALL_FLAG);
        if(iAlarmSuck!=-1)
        {
            if(iRetryCount>50)
            {
                iRetryCount=0;
                if(iRetryFail<3)
                {
                    iRetryFail++;
                    SetSortArmHome();
                }
                else
                {
                    ShowErrorMessage("WAR0257", 0, MOutSortX, false, MOT[iAlarmSuck].NumberAlias);       //出料手臂的吸嘴 %s 沒有在安全位置
                    iRetryFail=0;
                }
                return false;
            }
            else
            {
                iRetryCount++;
                return false;
            }
        }
        iRetryFail=0;
        iRetryCount=0;
    }

    if(OK)
    {
        for(int i=0; i<OutArm2Suck.iMotRow; i++)
        {
            for(int j=0; j<OutArm2Suck.iMotCol; j++)
            {
                iMot=OutArm2Suck.Suck[i][j].iMotNo;
                MOT[iMot].fCMD=false;
            }
        }
    }

    if(OK==true && iAlarmSuck==-1)                                              //Jou 2011-02-09
    {
        return true;
    }
    else
    {
        return false;
    }
}
//==============================================================================
//-----------------------------------------------------------------------------

extern int CheckSuckInitialStatus(TMyKitSuck &Ptr, int iR, int iC);
//==============================================================================
// 檢查輸出臂啟始狀態
//  Ver : 2003_07
//==============================================================================
bool CheckSortArmInitState()
{
    //return CheckSortArmInitState_AU();
    bool bHasFail=false;
    int ret;
    int flag[MAX_ARM_Row][MAX_ARM_Col]={{0, 0, 0, 0}, {0, 0, 0, 0}};
    AnsiString errSuck="at Sort Arm Suck";
    int iXpos=MOT[MOutSortX].ReadPos();
    int iYpos=MOT[MOutSortY].ReadPos();
    for(int i=0; i<OutArm2Suck.iPickRow; i++)
    {
        for(int j=0; j<OutArm2Suck.iPickCol; j++)
        {
#if 0 // GATE(1) -- golden asortarm.cpp:243 (CheckSuckInitialStatus has no compiled body anywhere in this tree)
            flag[i][j]=CheckSuckInitialStatus(OutArm2Suck, i, j);
#else
            flag[i][j]=0;                                                        // GATE(1) offline: nozzle initial vacuum state reported OK (0 == golden's own "fine" value)
#endif
            if(flag[i][j]!=0)
            {
                bHasFail=true;
                errSuck+=OutArm2Suck.Suck[i][j].sName;
            }
        }
    }

    if(bHasFail)
        ret=ShowErrorMessage("WAR0226", K_RETRY|K_SKIP, MOutSortX, false, errSuck);

    if(ret==K_SKIP)
    {
        for(int i=0; i<OutArm2Suck.iMaxRow; i++)
        {
            for(int j=0; j<OutArm2Suck.iMaxCol; j++)
            {
                if(flag[i][j]>=Vaccum_Initial_Off)
                {
                    OutArm2Suck.PordRec[i][j].AddErrorRecord("WAR0226", true, iXpos, iYpos);         //Steven 20160114 (jou) : Add Production Error Log
                    OutArm2Suck.SetItemData(i, j, HAS_NULL_IC);
                }
            }
        }
    }
    return !bHasFail;
}
//==============================================================================
// 輸出臂從 Shuttle  吸取 IC 流程動作
//  Ver : 2003_07
//==============================================================================
void  InitPickFromSortShtTask()
{
    iPickFromSortShtTask=1;
}
//==============================================================================
void PickFromSortSht()                                            //kevin 20210612 add int iwhichKit=0
{
    if(ArmSpeed[OutArm].dWaitOnSH!=0)                          //kevin 20210525 out arm 起動在shuttle 上等待 時間
    {
        DoSortSHTWaitTime.SetSecAndOn(ArmSpeed[OutArm].dWaitOnSH);                  //kevin 20210525 out arm 在shuttle 上等待
    }
}
//==============================================================================

//==============================================================================
// 設定輸出臂放到 Auto Tray 流程動作
//==============================================================================
void InitSortPlaceToAutoTask()
{
    iSortPlaceToAutoTask=1;
}
//------------------------------------------------------------------------------

//==============================================================================
// 設定輸出臂主流程動作
//==============================================================================
void InitSortArmTask()
{
    SortArmTask=1;
}
//-----------------------------------------------------------------------------
void SortArmAddSpeed()
{
    if(AutoArmSpeed[OutArm].iBodySP<=(ArmSpeed[OutArm].iBodySP-5) ||
       AutoArmSpeed[OutArm].iACDCBodySP<=(ArmSpeed[OutArm].iACDCBodySP-5))
    {
        if(AutoArmSpeed[OutArm].iBodySP<=(ArmSpeed[OutArm].iBodySP-5))
            AutoArmSpeed[OutArm].iBodySP+=5;

        if(AutoArmSpeed[OutArm].iACDCBodySP<=(ArmSpeed[OutArm].iACDCBodySP-5))
            AutoArmSpeed[OutArm].iACDCBodySP+=5;

        SetSortArmSpeed(true);
    }
    else
    {
        if(AutoArmSpeed[OutArm].iBodySP<ArmSpeed[OutArm].iBodySP ||
           AutoArmSpeed[OutArm].iACDCBodySP<ArmSpeed[OutArm].iACDCBodySP)
        {
            if(AutoArmSpeed[OutArm].iBodySP<ArmSpeed[OutArm].iBodySP)
                AutoArmSpeed[OutArm].iBodySP=ArmSpeed[OutArm].iBodySP;

            if(AutoArmSpeed[OutArm].iACDCBodySP<ArmSpeed[OutArm].iACDCBodySP)
                AutoArmSpeed[OutArm].iACDCBodySP=ArmSpeed[OutArm].iACDCBodySP;

            SetSortArmSpeed(true);
        }
    }
}
//-----------------------------------------------------------------------------
void SortArmSubSpeed()
{
    if(AutoArmSpeed[OutArm].iBodySP>25 ||
       AutoArmSpeed[OutArm].iACDCBodySP>25)
    {
        if(AutoArmSpeed[OutArm].iBodySP>25)
            AutoArmSpeed[OutArm].iBodySP-=2;

        if(AutoArmSpeed[OutArm].iACDCBodySP>25)
            AutoArmSpeed[OutArm].iACDCBodySP-=2;
        SetSortArmSpeed(true);
    }
}
//------------------------------------------------------------------------------
void DoSortArm()
{
    int ret;
//    static bool bCheckSpeed1=false;                                             //Add Speed Display
    static bool FirstEnter=true, bCheckSpeed=false;
    int &Task=SortArmTask;

#if 0 // GATE(3) -- golden asortarm.cpp:356 (TMyKitSuck::SetHasNullIcToNullIc absent tree-wide; same no-op as atester_32Site.cpp:3424 / aTester_Front.cpp:678)
    OutSht3Kit.SetHasNullIcToNullIc();
#endif
    if(USE_OUT_SORT_ARM==eartUninstall)
    {
        SortArmTask=1;
        return;
    }

    for(int i=0; i<OutArm2Suck.iShtRow; i++)
    {
        for(int j=0; j<OutArm2Suck.iShtCol; j++)
        {
            if(OutSht3Kit.Item[i][j]==HAS_IC || OutSht3Kit.Item[i][j]==HAS_HOT_IC)
                OutSht3Kit.Item[i][j]=(TEST_PASS+iTestBinCount);
        }
    }

   //DoSortArm_AU();

   if(IniConfig.bEnableTestingNeedStopAllMotor==true &&                        //jou 2013-09-25 Testing Need Stop All Motor
       IniConfig.bI24TestingNeedStopAllMotor==true &&
       bTestingStopAllMotor==true)
    {
        return;
    }

#if 0 // GATE(4) -- golden asortarm.cpp:381-387 (TfNote has no bMyServoOffOutArm -- forms/fNote.h:87 "NOT LANDED, ON PURPOSE"; same gate as aoutarm9045.cpp:388)
    if(IniConfig.bAlarmNeedServoOff==true)                                      //JerryYang 20161227 (Steven) 修正發生JAM,servo off後推動out arm, 再重新start會發生點位錯誤
    {
        if(fNote->bMyServoOffOutArm==true)
        {
            return;
        }
    }
#endif

    CheckSortArmDestroyActive();                                                 //確認device確實destroy完成

    for(int i=0; i<OutArm2Suck.iPickRow; i++)
        for(int j=0; j<OutArm2Suck.iPickCol; j++)
            if(bCheckSortArmDestroyActive[i][j]==true)
                return;

    if(IniConfig.bQAMode==true &&
       LastSet.iRunStartMode==rsmQAMode &&
       CosFunction.bQAModeUseUnloadCnt)                                         //JerryYang 20221004 : Maxim版本QA mode
    {
        Check_QA_ModeUnloadCount();
    }

    if(iPauseBackUp!=-1 &&
       OutArm2Suck.IsPickSuckFinish()==true &&
       OutArm2Suck.IsPickDestroyFinish()==true)
    {
        return;
    }

    if(bResetSortArmTask==true)
    {
        bResetSortArmTask=false;
        InitSortArmTask();
        return;
    }

    bAutoNeedTrayMustFinish=false;

//    if(iInArmType==e9045_1x1_1 ||
//       iInArmType==e9045_1x4_1_Ac)
//    {
//        DoOutArm_9045_1x1_1();
//    }
//    else if(iInArmType==e9045_1x2_2_14 ||
//            iInArmType==e9045_1x2_2_13)
//    {
//        DoOutArm_9045_1x2_2();
//    }
//    else if(iInArmType==e9045_1x2_4_Hot)
//    {
//        DoOutArm_9045_1x2_4();
//    }
//    else if(iInArmType==e9045_1x3_2_14)
//    {
//        DoOutArm_9045_1x3_2_14();
//    }
//    else if(iInArmType==e9045_1x3_4)                                            //Steven 20220425 : 1x3_14
//    {
//        DoOutArm_9045_1x3_4();
//    }
//    else if(iInArmType==e9045_1x4_2_14)
//    {
//        DoOutArm_9045_1x4_2();
//    }
//    else if(iInArmType==e9045_1x4_4_13)
//    {
//        DoOutArm_9045_1x4_4S();
//    }
//    else if(iInArmType==e9045_1x4_4_Back)
//    {
////
//    }
//    else if(iInArmType==e9045_1x4_4)
//    {
//        DoOutArm_9045_1x4_4();
//    }
//    else if(iInArmType==e9045_1x4_8_Hot)
//    {
//        DoOutArm_9045_1x4_8();
//    }
//    else if(iInArmType==e9045_2x1_2_13)
//    {
//        DoOutArm_9045_2x1_2();
//    }
//    else if(iInArmType==e9045_2x2_4_12 ||
//            iInArmType==e9045_2x2_4_13 ||
//            iInArmType==e9045_2x2_4_14)
//    {
//        DoOutArm_9045_2x2_4();
//    }
//    else if(iInArmType==e9045_2x2_8_Hot)
//    {
//        DoOutArm_9045_2x2_8();
//    }
//    else if(iInArmType==e9045_2x3_6_14)
//    {
//        DoOutArm_9045_2x3_6_14();
//    }
//    else if(iInArmType==e9045_2x3_6)
//    {
//        DoOutArm_9045_2x3_6();
//    }
//    else if(iInArmType==e9045_2x4_4_13 ||
//            iInArmType==e9045_2x4_4_14)
//    {
//        DoOutArm_9045_2x4_4();
//    }

    if(iInArmType==e9045_2x4_8 ||
        iInArmType==e9045_2x8_8 )
    {
        //DoSortArm_2x4_8();
        switch(Task)
        {
            case 1:
                bSortingAllBinTrayFinish=false;                                     //是否所有的Bin Tray IC都整盤完成
                Task=5;
            case 5:
                if(MoveSortArmToAutoSafe())
                    Task=10;

                if(Task!=10)                                                        //add Out arm speed
                    break;
            case 10:
                if(CheckSortArmInitState())
                    Task=50;

                if(Task!=50)                                                        //add Out arm speed
                    break;
            case 50:
                FirstEnter=true;                                                    // for judge which shuttle to pick
                if(MoveSortArmToAutoSafe())
                {
                    MOT[MOutSortSht].fCanMoveL=true;
                    if(OutArm2Suck.HasIC())
                        Task=3000;
                    else
                        Task=100;
                }

                if(Task!=100)                                                      //add Out arm speed
                    break;
            case 100:
                if(FirstEnter)
                {
                    if((OutSht3Kit.UseSiteHasIC()) && SortSHTInLF())
                    {
                        FirstEnter=false;
                    }
                }

                if(FirstEnter)
                {
                    Task=200;
                    break;
                }

                Task=1000;

                break;
            case 200:
                if(MoveSortArmToAutoSafe())
                    Task=300;
                break;
            case 300:
                Task=CheckSortArmCleanOut();

                if(Task==5000)                                                      //Sam 20250204 : 修正 P27 整盤功能 Hangeup
                {
                    DoSortArmAfterPlaceToAuto(bCheckSpeed, true);
                    Task=3500;
                }
                break;
            case 1000:
                Task=1140;
                break;
           case 1140:
                if((iCleanOut==1 && HasICUnderMachine()==false) ||
                   (iCleanOut==1 && LastSet.iTemperature==Tempture_Hot &&
                                    (TrayForm.bAutoFeed==false && bMustCleanAllTray==false) &&
                                    HasICUnderHotPlate()==false))
                {
                    FirstEnter=true;
                    Task=1;
                    break;
                }
                //if(SortSHTInLF() && bCheckShuttle3Flag==false)
                if(SortSHTInLF())
                {
                    if(OutSht3Kit.UseSiteHasIC())
                    {
                        Task=1150;
                    }
                    else
                    {
                        Task=1200;
                    }
                }
                else
                {
                    if(iInArmType==e9045_2x8_8)
                    {
                        if(OutSht3Kit.HasRealIC_Left(OutArm2Suck.iShtRow, OutArm2Suck.iShtKitStep))
                        {
                            iOutArmiWhichKit=0;
                        }
                        else if(OutSht3Kit.HasRealIC_Right(OutArm2Suck.iShtRow, OutArm2Suck.iShtKitStep))
                        {
                            iOutArmiWhichKit=1;
                        }
                    }
                    else
                        iOutArmiWhichKit=0;

                    Task=1150;
                }

                if(Task!=1150)
                    break;
            case 1150:
                if(MoveSortArmToShuttleIncludeZ())                              //移動過去 但Z還沒有下降
                {
//                    if(OutArm2Suck.iXStep!=1 || OutArm2Suck.iYStep!=1)
//                    {
//                        iBackMode1=GetNowSortShuttleMode_2x4_8();
//                    }
                    Task=1100;
                }

                if(Task!=1100)
                    break;
            case 1100:
//                if(WhichAutoNeedTray()>3)                                       //在sort區域 Auto 4 5 6才要等
//                {
//                    //MoveOutArmXY_ToFix_Tray_Full();                           //先檢查有沒有要換Tray
//                    //Task=1160;
//                    Task=1100;
//                }
                //else if(OutSht3Kit.UseSiteHasIC() && SortSHTInLF() && bCheckShuttle3Flag==false)
                if(OutSht3Kit.UseSiteHasIC() && SortSHTInLF())
                {
                    MOT[MOutSortSht].fCanMoveR=false;

//                    if(OutSht3Kit.LeftSideNoIC(2)==false)
//                        iSortArmiWhichKit=0;
//                    else
//                        iSortArmiWhichKit=1;

                    InitPickFromSortShtTask();
                    Task=1200;

//                    if(OutArm2Suck.iXStep!=1 || OutArm2Suck.iYStep!=1)
//                    {
//                        iMode1=GetNowSortShuttleMode_2x4_8();
//                        if(iMode1!=iBackMode1)
//                        {
//                            Task=1150;
//                            break;
//                        }
//                        else
//                        {
//                            iMode1=0;
//                            iBackMode1=0;
//                        }
//                    }
                    break;
                }
                else if(IsOutArmCleanOutFinish())
                {
                    Task=50;
                }
                break;
            case 1160:
                if(MoveOutArmXY_ToFix_Tray_Full())    //目前不給進
                {
                    Task=1100;
                }
                break;
            case 1200:
                if(DoSortPickFromShuttle())
                {
                    MOT[MOutSortSht].fCanMoveR=true;                            //要解除，不然Sort Shuttle會Hang
//                    PreSetOutAdditionalFlag();
//                    OutSht3Kit.SetUnuseAndHasNullICToNullIC();

//                    if(OutSht3Kit.UseSiteHasIC() &&                             //這裡需要優化，看能不能arm吸滿再走
//                        OutArm2Suck.HasIC()==false)
//                    if(OutArm2Suck.HasIC()==false)
//                    {
//                        if(OutSht3Kit.LeftSideNoIC(2)==false)
//                            iSortArmiWhichKit=0;
//                        else
//                            iSortArmiWhichKit=1;

//                        InitPickFromSortShtTask();
//                        return;
//                    }

                    if(OutArm2Suck.HasIC()==false)
                    {
                        Task=50;
                    }
                    else
                    {
//                        iOutRotateFinish=0;                                         //kevin 20130524 目前放到 auto tray
//                        bPickSH1Flag=true;
//                        bPickSH2Flag=false;
                        Task=3000;
                    }
                }
//                else
//                {
//                    if(bOutArmManualStepPress)
//                        Task=1235;
//                }
                break;
//            case 1235:
//                bOutArmManualStepPress=false;
//                if(MoveSortArmToAutoSafe())
//                    Task=1150;
//                break;
//            case 2000:
//                if(USE_Y_AUTO_PITCH==true && IniConfig.bE57YPitchHome)              //kevin 20180827 (Steven) : add put shuttle Y pitch home AutoYPitch
//                {
//                    Task=2010;
//                    return;
//                }
//                Task=2040;
                break;
//            case 2010:   //kevin 20180827
//                if(MoveOutArmToAutoSafe())
//                {
//                    hOutArmYpitchHomeTimer.SetSecAndOn(10);
//                    Task=2020;
//                }
//                break;
//            case 2020:
//                if(bCheckYPitchRunHomeSen(1))
//                {
//                     Task=2040;
//                }
//                else if(hOutArmYpitchHomeTimer.Off())
//                {
//                    InitProcessSingleMotorTask(MOutArmPitchY);
//                    hOutArmYpitchHomeTimer.SetSecAndOn(30);
//                    bYpitchNeddHome=true;
//                    Task=2030;
//                }
//                break;
//            case 2030:
//                if(bCheckYPitchHome(1))
//                {
//                    SetMotorSpeed();
//                    bYpitchNeddHome=false;
//                    Task=2040;
//                }
//                else if(hOutArmYpitchHomeTimer.Off())
//                {
//                    ShowErrorMessage("WAR0227", K_RETRY, MInArmPitchY);
//                    Task=2020;
//                }
//                break;
//            case 2040:
//                if(CheckOutArmToTask50(iWhichShuttle))
//                {
//                    Task=50;
//                    break;
//                }
//
//                if((iCleanOut==1 && HasICUnderMachine()==false) ||
//                   (iCleanOut==1 && LastSet.iTemperature==Tempture_Hot &&
//                                   (TrayForm.bAutoFeed==false && bMustCleanAllTray==false) &&
//                                   HasICUnderHotPlate()==false))                    //JerryYang 20190807 fix 快速clean out hang up
//                {
//                    FirstEnter=true;
//                    Task=1;
//                    break;
//                }
//
//                if(OutSHT2InRT() && bCheckShuttle2Flag==false)
//                {
//                    if(BRCarryKit.UseSiteHasIC())
//                    {
//                        if(CheckShuttleICPos(2))
//                        {
//                            Task=2050;
//                        }
//                        else
//                        {
//                            InitPickFromShuttle2Task();                             //Sam 20230208 : 修正 Fix AOI 未拍照問題
//                            Task=2200;
//    //                        #ifdef Carry4
//    //                            MOT[MOutShuttle2].fCanMoveR=false;
//    //                        #else
//                                MOT[MInShuttle2].fCanMoveR=false;
//    //                        #endif
//                        }
//                    }
//                    else
//                    {
//                        Task=2050;
//                    }
//                }
//                else
//                {
//                    Task=2050;
//                }
//
//                if(Task!=2050)                                                      //Steven 20180813 : add Out arm speed
//                    break;
//            case 2050:
//                if(MoveOutArmToShuttleIncludeZ_9045_2x4_8(iWhichShuttle))
//                {
//                    IsCatchTrayReadySupplyNewTray();
//
//                    if(OutArmSuck.iXStep!=1 || OutArmSuck.iYStep!=1)
//                    {
//                        iBackMode2=GetNowShuttleMode_2x4_8(1);
//                    }
//                    Task=2100;
//                }
//
//                if(Task!=2100)                                                      //Steven 20180813 : add Out arm speed
//                    break;
//            case 2100:
//                IsCatchTrayReadySupplyNewTray();
//
//                if(BRCarryKit.UseSiteHasIC() && OutSHT2InRT() && bCheckShuttle2Flag==false)
//                {
//    //#ifdef Carry4
//    //                MOT[MOutShuttle2].fCanMoveR=false;
//    //#else
//                    MOT[MInShuttle2].fCanMoveR=false;
//    //#endif
//                    InitPickFromShuttle2Task();
//                    Task=2200;
//
//                    if(OutArmSuck.iXStep!=1 || OutArmSuck.iYStep!=1)
//                    {
//                        iMode2=GetNowShuttleMode_2x4_8(1);
//                        if(iMode2!=iBackMode2)
//                        {
//                            Task=2050;
//                            break;
//                        }
//                        else
//                        {
//                            iMode2=0;
//                            iBackMode2=0;
//                        }
//                    }
//                    break;
//                }
//                else if(FRCarryKit.UseSiteHasIC() && OutSHT1InRT() && bCheckShuttle1Flag==false)
//                {
//                    Task=50;
//                    break;
//                }
//                break;
//            case 2200:
//                if(bCheckSpeed)                                                     //Steven 20110525 : Auto Speed
//                {
//                    bCheckSpeed=false;
//                    OutArmAddSpeed();
//                }
//
//                if(bCheckSpeed1)                                                    //KaiChen 20171225 (Steven)：Add Speed Display
//                {
//                    bCheckSpeed1=false;
//                    OutArmAddSpeedDisplay();
//                }
//
//                IsCatchTrayReadySupplyNewTray();
//
//                if(DoPickFromShuttle_9045_2x4_8(1))
//                {
//                    PreSetOutAdditionalFlag();                                      //Steven 20220614 : Fixed for out arm additional function
//                    BRCarryKit.SetUnuseAndHasNullICToNullIC();
//                    if(BRCarryKit.UseSiteHasIC())
//                    {
//                        InitPickFromShuttle2Task();                                 //Sam 20230208 : 修正 Fix AOI 未拍照問題
//                        return;
//                    }
//
//                    if(OutArmSuck.HasIC()==false)
//                    {
//                        Task=50;
//                    }
//                    else
//                    {
//                        iOutRotateFinish=0;                                         //kevin 20130524 目前放到 auto tray
//    //                    iOutArmPickPlaceCnt[OutOfsOutSh2]++;                      //JerryYang 20230523 : 沒在用, Mark掉  //JerryYang 20180921 Setup Teach功能
//                        bPickSH1Flag=false;
//                        bPickSH2Flag=true;
//                        Task=3000;
//                    }
//                }
//                else
//                {
//                    if(bOutArmManualStepPress)
//                        Task=2235;
//                }
//                break;
//            case 2235:
//                bOutArmManualStepPress=false;
//                if(MoveOutArmToAutoSafe())
//                    Task=2050;
//                break;
            case 3000:
                IsCatchTrayReadySupplyNewTray();
                if(MoveSortArmToAutoSafe())
                {
                    //CheckSortArmSuckICFallDown();  //未實作

                    MOT[MOutSortSht].fCanMoveL=true;

                    Task=3010;
                    AseIcRecord="@e02001";
                }
                break;
            case 3010:
                IsCatchTrayReadySupplyNewTray();
                if(OutArm2Suck.HasIC()==false)
                {
                    Task=100;
                    break;
                }
//                iOutRotateFinish=3;
                iSortWhichAuto=SearchTrayToPlace_9045(true);

//                ret=VerifyFixTrayLink();
//                if(ret==4000 || ret==10000)
//                {
//                    Task=ret;
//                    break;
//                }
//                else if(ret==1)
//                {
//                    return;
//                }

//                if(TRAY_ARM_MODE==eAboveCoveyor &&
//                   MOT[MTrayX].ReadPos()>Prod.iXTrayColor)
//                {
//                    MoveOutArmXY_ToFix_Tray_Full();
//                    break;
//                }
                InitSortPlaceToAutoTask();

//                if(iSortWhichAuto<=iAutoRight)
//                {
//                    if(TRAY_ARM_MODE==eAboveCoveyor &&
//                       (MOT[iMMAuto[iSortWhichAuto]].fHasTray==false ||
//                        MOT[iMMAuto[iSortWhichAuto]].FullIC()))
//                    {
//                        break;
//                    }
//                }
//                else
//                {
//                    if(AUTO3_IS_MAGAZINE==1             &&                          //JerryYang 20221215 : Magazine把fix區當buffer區功能
//                       TestIF_File.iMagFixTrayType==1   &&
//                       iSortWhichAuto>=iMagMin)
//                    {
//
//                    }
//                    else
//                    {
//                        SetFixTrayFullIC();
//                        if(TRAY_ARM_MODE==eAboveCoveyor &&
//                           (MOT[iMMAuto[iSortWhichAuto]].fHasTray==false ||
//                            MOT[iMMAuto[iSortWhichAuto]].FullIC()))
//                        {
//                            if(iHWFix_BinBox==1 && iSortWhichAuto==eBulkBox)
//                                DoFixTrayFullAlarm();                               //kevin 20201207 check Error bin box Fix 1 put tray
//                            break;
//                        }
//                    }
//                }

//                if(TRAY_ARM_MODE==eAboveCoveyor &&
//                   (iCatchTrayControlManual>=2 || WhichAutoNeedTray()!=0))          //Sam 20180822 : OutArm 是否在安全位置，是否讓位，不然會與 TrayArm 補 Tray 動作互卡 Hange up。
//                {
//                    MoveOutArmXY_ToFix_Tray_Full();
//                    break;
//                }

                if(TRAY_ARM_MODE==eUnderCoveyor&&
                       (iCatchTrayControlManual>=2 || WhichAutoNeedTray()>3))    //在sort區域 Auto 4 5 6才要等
                {
                    MoveSortArmToAutoSafe();
                    break;
                }
                Task=3100;
                break;
//            case 3020:                                                              //jou 2010-01-25 start : one cycle後，可以將tray盤拉出來
//                IsCatchTrayReadySupplyNewTray();
//                if(MoveOutArmXY_ToFix_Tray_Full())
//                {
//                    if(MOT[iMMAuto[iSortWhichAuto]].fHasTray==true)
//                        Task=3100;
//                }
//                break;
//            case 3050:
//                if(AutoTrayReCheck(iWhichAuto))                                     //Ifor 20171031 add Auto Tray ReCheck
//                {
//                    Task=3100;
//                }
//                break;
            case 3100:
                if(iSortWhichAuto<=6)
                {
                    //Task=VerifyTrayStatus();
                    Task=3300;
                    if(Task!=3300)
                        break;
                }
                Task=3300;                                                          //kevin 20151203 沒有fixtray放ic
            case 3300:
//                if(AUTO3_IS_MAGAZINE==1             &&                              //JerryYang 20221215 : Magazine把fix區當buffer區功能
//                   TestIF_File.iMagFixTrayType==1   &&
//                   iSortWhichAuto>=iMagMin              &&
//                   iWhichBuff>=iFixMin              &&
//                   iWhichBuff<=iFixMax)
//                {
//                    ct=iAutoIndex[iWhichBuff];
//                    if(iWhichBuff>=iFixMin &&
//                       Sen[SnFixedTrayDetect[ct]].IsOff() &&
//                       LastSet.iRealDummy!=DUMMY)                                   //wei 20151217 add iWhichAuto>=3避免沒要放FIX盤也偵測有沒有FIX盤
//                    {
//                        Task=4000;
//                        break;
//                    }
//                }
//                else
//                {
//                    ct=iAutoIndex[iSortWhichAuto];
//                    if(iSortWhichAuto>=iAutoCnt &&
//                       Sen[SnFixedTrayDetect[ct]].IsOff() &&
//                       LastSet.iRealDummy!=DUMMY)                                   //wei 20151217 add iWhichAuto>=3避免沒要放FIX盤也偵測有沒有FIX盤
//                    {
//                        Task=4000;
//                        break;
//                    }
//                }
                Task=3301;
                if(Task!=3301)                                                      //Steven 20180813 : add Out arm speed
                    break;
            case 3301:
                //if(DoOutArmPlaceToAuto_9045())                                      // 吸嘴可能還在下方
                if(DoSortArmPlaceToAuto())
                {
                    DoSortArmAfterPlaceToAuto(bCheckSpeed, true);                    //Steven 20220526 : 針對放下IC到Unloader後的動作做整合
                    Task=3500;
                }
                else
                {
                    break;
                }
            case 3500:
                ret=DoSortArmAfterPlaceToAuto(bCheckSpeed);                          //Steven 20220526 : 針對放下IC到Unloader後的動作做整合
                if(ret!=0)
                {
                    Task=ret;
                    if(Task==100)
                    {
                        //if(OutSht3Kit.LeftSideNoIC(2)==false)
                        if(OutSht3Kit.UseSiteHasIC()==true)
                        {
                            Task=1000;
                            return;
                        }
                    }
                }
                break;
//            case 4000:
//                if(MoveOutArmXY_ToFix_Tray_Full())
//                    Task=4100;
//                break;
//            case 4100:
//                if(DoFixTrayFullAlarm())                                            //Steven 20181113 : 修正Fix Link Alarm的問題
//                {
//                    Task=3010;
//                }
//                break;
//            case 7000:
//                flag3=DoOutArmAdditionalFunction();                                 //Steven 20210609 : 整合Precisor, Rotator, Bottom CCD, Die Clean
//                if(flag3)
//                {
//                    AseIcRecord="@e02001";                                          //kevin 20161228 (jou) 傳送給 ASE 抬頭資料
//                    Task=3010;
//                }
//                break;
//            case 10000:                                                             //JerryYang 20220909 : add magazine
//                if(CheckPlaceToMagazineTray(iWhichMag))
//                {
//                    Task=12000;                                                     // To Auto 3
//                }
//                break;
//            case 11100:
//                if(DoPickFromMagazineBuffer())
//                {
//                    iSortWhichAuto=SearchTrayToPlace_9045();
//                    iOutArmWhichAuto=iMagAtAuto;
//                    if(iAuto3MagazineIndex!=iWhichMag && iWhichMag!=-1)
//                    {
//                        bMagNeedChangeTray=true;
//                    }
//                    Task=12000;
//                }
//                break;
//            case 12000:
//                IsCatchTrayReadySupplyNewTray();
//                if(iAuto3MagazineIndex!=iWhichMag ||
//                   MOT[iMMAuto[iMagAtAuto]].fHasTray==false ||
//                   MOT[iMMAuto[iMagAtAuto]].FullIC())
//                {
//                    MoveOutArmXY_ToFix_Tray_Full();                                 // change Magazine Tray
//                    break;
//                }
//
//                bMagNeedChangeTray=false;
//                if(bMagCatchTrayfalg)                                               //JerryYang 20221121 : AUTOI3 Magazine還在夾TRAY的旗標
//                {
//                    break;
//                }
//
//                if(MOT[MTrayX].ReadPos()>Prod.iXTrayColor)
//                {
//                    MoveOutArmXY_ToFix_Tray_Full();
//                    break;
//                }
//
//                InitPlaceToAutoTask();
//                if(iCatchTrayControlManual>=2)                                      // catch tray busy
//                {
//                    MoveOutArmXY_ToFix_Tray_Full();
//                    break;
//                }
//                Task=3100;
//                break;
        }
    }
//    else if(iInArmType==e9045_2x5_8)                                            //wei 20190614 10 site
//    {
//        DoOutArm_9045_2x5_8();
//    }
//    else if(iInArmType==e9045_2x6_8)
//    {
//        DoOutArm_9045_2x6_8();
//    }
//    else if(iInArmType==e9045_2x8_8 ||
//            iInArmType==e9045_2x8_32)
//    {
//        DoOutArm_9045_2x8_8();
//    }

    return;
}
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
int Find_SortArm_PickerMaxUseCountOnTime(int iUseSuck, int &iRow, int &iCol)
{
    int  iTrayCol;
    bool bBackupSortArmSuckActive[MAX_ARM_Row][MAX_ARM_Col]={{false, false, false, false}, {false, false, false, false}};
    int iBackupSortArmXPosition[MAX_ARM_Row][MAX_ARM_Col]={{0, 0, 0, 0}, {0, 0, 0, 0}};
    int iBackupSortArmYPosition;
    int iCT=0, itStartCol=0, itStartRow=0;
    int iMaxCT=0, isStartX=0;
    int Motor;
    int iBackix=0, iBackiy=0;
    int ixStart=0, ixEnd, ixStep;
    int iyStart=0, iyEnd, iyStep;
    int Direction;
    static int iRecY=0, iRecWhichAuto;

    bool bSearchLast=(OutArm2Suck.iPickRow==2)?TestIF.bSearchLastMode:false;

    int iSortArmWhichAuto=iSortWhichAuto;

    for(int i=0; i<OutArm2Suck.iPickRow; i++)
    {
        for(int j=0; j<OutArm2Suck.iPickCol; j++)
        {
            if(OutArm2Suck.Item[i][j] &&
               OutArm2Suck.iWhichAuto[i][j]==iSortArmWhichAuto)
            {
                isStartX=j;
                i=OutArm2Suck.iMaxRow;
                j=OutArm2Suck.iMaxCol;
                break;
            }
        }
    }

    Motor=iMMAuto[iSortWhichAuto];
    Direction=AutoForm[iSortWhichAuto]->Direction;

    if(Direction>=4)
        Direction=0;

    if(Direction==0)
    {
        ixStart=0;
        ixEnd=MOT[Motor].Tray.XItem;
        ixStep=1;

        if(TRAY_ARM_MODE==eUnderCoveyor &&
           (MOT[Motor].fHasTray==false ||
            MOT[Motor].FullIC()==true))
        {
            iyStart=0;
            iRecY=0;
        }
        else
        {
            if(iSortWhichAuto<iAutoCnt)
            {
                if(bSearchLast && iCloseSiteState==0)
                {
                    if(OutArm2Suck.iPickRow==2)
                    {
                        if(MOT[Motor].HasIC()==false)
                        {
                            iyStart=0;
                            iRecY=0;
                        }
                        else
                        {
                            if(iSortArmPlaceOrder==1 && iRecWhichAuto==iSortWhichAuto)
                            {
                                //iyStart=iRecY+(TestIF.iARM_Y_PITCH/AutoForm[iWhichAuto]->YPitch); //ChungHung 20120505 9045WS
                                if(iyStart>=AutoForm[iSortWhichAuto]->YDivision)
                                {
                                    iyStart=0;
                                    iRecY=0;
                                }

                                if(MOT[Motor].SearchHasEmpryToPlace(iyStart)==false)
                                {
                                    iyStart=0;
                                    iRecY=0;
                                }
                            }
                            else
                            {
                                iyStart=0;
                            }
                        }
                        iyEnd=MOT[Motor].Tray.YItem;
                        iyStep=1;
                    }
                    else
                    {
                        iyStart=0;
                        iyEnd=MOT[Motor].Tray.YItem;
                        iyStep=1;
                    }
                }
                else
                {
                    iyStart=0;
                    iyEnd=MOT[Motor].Tray.YItem;
                    iyStep=1;
                }
            }
        }
    }
    else if(Direction==1)
    {
        ixStart=MOT[Motor].Tray.XItem-1;
        ixEnd=-1;
        ixStep=-1;
        if(iSortWhichAuto<iAutoCnt)
        {
            iyStart=0;
            iyEnd=MOT[Motor].Tray.YItem;
            iyStep=1;
        }
    }
    else if(Direction==2)
    {
        ixStart=0;
        ixEnd=MOT[Motor].Tray.XItem;
        ixStep=1;
        if(iSortWhichAuto<iAutoCnt)
        {
            iyStart=MOT[Motor].Tray.YItem-1;
            iyEnd=-1;
            iyStep=-1;
        }
    }
    else
    {
        ixStart=MOT[Motor].Tray.XItem-1;
        ixEnd=-1;
        ixStep=-1;
        if(iSortWhichAuto<iAutoCnt)
        {
            iyStart=MOT[Motor].Tray.YItem-1;
            iyEnd=-1;
            iyStep=-1;
        }
    }

    //iOutArmYStep=AutoCalculateOutArmYClosePitch(iWhichAuto)/AutoForm[iWhichAuto]->YPitch;   //Steven 20230523 : LoadForm->YPitch --> AutoForm[iWhichAuto]->YPitch

    if(TRAY_ARM_MODE==eUnderCoveyor &&
       (MOT[Motor].fHasTray==false ||
        MOT[Motor].FullIC()==true))
    {
        for(int iC=isStartX; iC<OutArm2Suck.iPickCol; iC++)
        {
            iTrayCol=iC-isStartX;
            iTrayCol=itStartCol+iTrayCol*iSortArmXStep;
//            if(ArmSpeed_File[OutArm].bYPitchUseSearchLastMode && iOutArmPlaceOrder==0 &&
//               UnloaderTrayYDirForArmYPitch(iWhichAuto) &&
//               (itStartRow+iSortArmYStep<AutoForm[iWhichAuto]->YDivision) &&     //JerryYang 20221128 : 避免Y超出Tray盤
//               (itStartRow+iSortArmYStep<iyEnd))                                 //JerryYang 20181112 (Steven) : ATP要求修改unloader擺放方式，盡量不要有空格
//            {
//                if(OutArm2Suck.Item[0][iC]!=NULL_IC &&
//                    OutArm2Suck.iWhichAuto[0][iC]==iOutArmWhichAuto &&           //JerryYang 20221215 : 改用iOutArmWhichAuto
//                    iTrayCol<AutoForm[iWhichAuto]->XDivision)
//                {
//                    iCT++;
//                    bOutArmSuckActive[0][iC]=true;
//                    iOutArmXPosition[0][iC]=iTrayCol;
//                    bBackupOutArmSuckActive[0][iC]=true;
//                    iBackupOutArmXPosition[0][iC]=iTrayCol;
//                    iOutArmYPosition=iRow;
//                }
//                if(OutArm2Suck.Item[1][iC]!=NULL_IC &&
//                   OutArm2Suck.iWhichAuto[1][iC]==iOutArmWhichAuto &&            //JerryYang 20221215 : 改用iOutArmWhichAuto
//                   iTrayCol<AutoForm[iWhichAuto]->XDivision)
//                {
//                    iCT++;
//                    bOutArmSuckActive[1][iC]=true;
//                    iOutArmXPosition[1][iC]=iTrayCol;
//                    bBackupOutArmSuckActive[1][iC]=true;
//                    iBackupOutArmXPosition[1][iC]=iTrayCol;
//                    iOutArmYPosition=iRow;
//                }
//            }
//            else
//            {
                if(OutArm2Suck.Item[iSortArmPlaceOrder][iC]!=NULL_IC &&
                   OutArm2Suck.iWhichAuto[iSortArmPlaceOrder][iC]==iSortArmWhichAuto &&
                   iTrayCol<AutoForm[iSortWhichAuto]->XDivision)
                {
                    iCT++;
                    bSortArmSuckActive[iSortArmPlaceOrder][iC]=true;
                    iSortArmXPosition[iSortArmPlaceOrder][iC]=iTrayCol;
                    bBackupSortArmSuckActive[iSortArmPlaceOrder][iC]=true;
                    iBackupSortArmXPosition[iSortArmPlaceOrder][iC]=iTrayCol;
                    iSortArmYPosition=iRow;
                }
//            }
        }
        iMaxCT=iCT;
    }
    else
    {
        for(int y=iyStart; y!=iyEnd; y+=iyStep)
        {
            if(iMaxCT!=0)
                break;
            for(int x=ixStart; x!=ixEnd; x+=ixStep)
            {
                if(MOT[Motor].Tray.Data[x][y]==NULL_IC)
                {
                    itStartCol=x;
                    itStartRow=y;
                    iCol=itStartCol;
                    iRow=itStartRow;
                    for(int i=0; i<OutArm2Suck.iMaxRow; i++)
                        for(int j=0; j<OutArm2Suck.iMaxCol; j++)
                            bSortArmSuckActive[i][j]=false;
                    iCT=0;

                    for(int j=isStartX; j<OutArm2Suck.iPickCol; j++)
                    {
                        iTrayCol=j-isStartX;
                        iTrayCol=itStartCol+iTrayCol*iSortArmXStep;

                        if(OutArm2Suck.Item[iSortArmPlaceOrder][j]!=NULL_IC &&
                           OutArm2Suck.iWhichAuto[iSortArmPlaceOrder][j]==iSortArmWhichAuto &&
                           iTrayCol<MOT[Motor].Tray.XItem &&
                           MOT[Motor].Tray.Data[iTrayCol][itStartRow]==NULL_IC)
                        {
                            iCT++;
                            bSortArmSuckActive[iSortArmPlaceOrder][j]=true;
                            iSortArmXPosition[iSortArmPlaceOrder][j] =iTrayCol;
                            iSortArmYPosition=iRow;
                        }
                    }

                    if(iCT>iMaxCT)
                    {
                        iMaxCT=iCT;

                        for(int j=0; j<OutArm2Suck.iPickCol; j++)
                        {
                            bBackupSortArmSuckActive[iSortArmPlaceOrder][j]=bSortArmSuckActive[iSortArmPlaceOrder][j];
                            iBackupSortArmXPosition[iSortArmPlaceOrder][j] =iSortArmXPosition[iSortArmPlaceOrder][j];
                        }
                        iBackupSortArmYPosition=iSortArmYPosition;
                        iBackix=iCol;
                        iBackiy=iRow;
                    }
                }
            }
        }
    }

    for(int i=0; i<OutArm2Suck.iPickCol; i++)
    {
        bSortArmSuckActive[iSortArmPlaceOrder][i]=bBackupSortArmSuckActive[iSortArmPlaceOrder][i];
        iSortArmXPosition[iSortArmPlaceOrder][i] =iBackupSortArmXPosition[iSortArmPlaceOrder][i];
    }
    iSortArmYPosition=iBackupSortArmYPosition;
    iCol=iBackix;
    iRow=iBackiy;

    if(bSearchLast && iCloseSiteState==0)
    {
        if(OutArm2Suck.iPickRow==2)
        {
            if(iSortArmPlaceOrder==0)
            {
                iRecY=iRow;
                iRecWhichAuto=iSortWhichAuto;
            }
        }
    }

    return iMaxCT;
}
//------------------------------------------------------------------------------
void Find_SortArm_Single(int iUseSuck, int &iRow, int &iCol)
{
    int itStartCol, itStartRow;
    int isStartCol;
    int Motor;
    int ixStart, ixEnd, ixStep;
    int iyStart, iyEnd, iyStep;
    int Direction=0;
    int serchtable[4]={0, 1, 2, 3};
    //int Yitem=0;
    //int iRegion=5;                                                              //Magazine 14bin, 1個fix盤5個bin
    //int iRy=0;

    int iSortArmWhichAuto=iSortWhichAuto;

    Motor=iMMAuto[iSortWhichAuto];

    itStartCol=-1;
    itStartRow=-1;

    for(int i=0; i<OutArm2Suck.iMaxRow; i++)
        for(int j=0; j<OutArm2Suck.iMaxCol; j++)
            bSortArmSuckActive[i][j]=false;

    if(IniConfig.bI37_EnableFIFOMode && LastSet.iRunStartMode==rsmFIFOMode)     //Steven 20160303 : FIFO Mode
    {
        for(int i=0; i<OutArm2Suck.iMaxRow; i++)
        {
            for(int j=0; j<OutArm2Suck.iMaxCol; j++)
            {
                if(OutArm2Suck.Item[i][j]!=NULL_IC)
                {
                    bSortArmSuckActive[i][j]=true;
                    iCol=OutArm2Suck.PordRec[i][j].GetLoaderX();
                    iRow=OutArm2Suck.PordRec[i][j].GetLoaderY();
                    if(CUSTOMER_CODE==CC_KYEC_LEE)                              //Ifor 20181025 : Fix ProductionRecord KYEC +1 導致shift
                    {
                        iCol=iCol-1;
                        iRow=iRow-1;
                    }
                    return ;
                }
            }
        }
    }

    Direction=AutoForm[iSortWhichAuto]->Direction;

    if(Direction==0 || Direction==4)    //左上到右下                            //Steven 20141118 : Add Out Arm Direction 4~7
    {
        ixStart=0;
        ixEnd=MOT[Motor].Tray.XItem;
        ixStep=1;

        if(iSortWhichAuto<iAutoCnt)
        {
            iyStart=0;
            iyEnd=MOT[Motor].Tray.YItem;
            iyStep=1;
            if(IniConfig.bUseTrayBlockMode &&                                   //Frank 20160928 add Subtray Function
               IniConfig.bP06_LoaderUseCarrierTray==true)                       //Ifor 20161129 add P06 判斷避免OutArm 一顆一顆放料
            {
                int BX, BY, BlockTotal;
                bool bBlockEmpty = true;
                if((AutoForm[iSortWhichAuto]->BlockXItem>0) &&
                   (AutoForm[iSortWhichAuto]->BlockYItem>0))
                {
                    BX=MOT[Motor].Tray.XItem/AutoForm[iSortWhichAuto]->BlockXItem;
                    BY=MOT[Motor].Tray.YItem/AutoForm[iSortWhichAuto]->BlockYItem;
                    BlockTotal = AutoForm[iSortWhichAuto]->BlockXItem*AutoForm[iSortWhichAuto]->BlockYItem;
                    iSortArmBlockIndex =0;
                    for(int k=0; k<BlockTotal; k++)
                    {
                        ixEnd=BX+(iSortArmBlockIndex%AutoForm[iSortWhichAuto]->BlockXItem*BX);
                        iyEnd=BY+(iSortArmBlockIndex/AutoForm[iSortWhichAuto]->BlockXItem*BY);

                        for(int iR=0; iR<iyEnd; iR++)
                        {
                            for(int iC=0; iC<ixEnd; iC++)
                            {
                                if(MOT[Motor].Tray.Data[iC][iR]==NULL_IC)
                                {
                                    bBlockEmpty=false;
                                }
                            }
                        }

                        if(bBlockEmpty)
                        {
                            iSortArmBlockIndex=k+1;
                            ixEnd=BX+(iSortArmBlockIndex%AutoForm[iSortWhichAuto]->BlockXItem*BX);
                            iyEnd=BY+(iSortArmBlockIndex/AutoForm[iSortWhichAuto]->BlockXItem*BY);
                        }
                        else
                        {
                            break;
                        }
                    }
                }
            }
        }
    }
    else if(Direction==1 || Direction==6)                                       //右上到左下
    {
        ixStart=MOT[Motor].Tray.XItem-1;
        ixEnd=-1;
        ixStep=-1;
        if(iSortWhichAuto<iAutoCnt)
        {
            iyStart=0;
            iyEnd=MOT[Motor].Tray.YItem;
            iyStep=1;
        }
    }
    else if(Direction==2 || Direction==5)                                       //左下到右上
    {
        ixStart=0;
        ixEnd=MOT[Motor].Tray.XItem;
        ixStep=1;
        if(iSortWhichAuto<iAutoCnt)
        {
            iyStart=MOT[Motor].Tray.YItem-1;
            iyEnd=-1;
            iyStep=-1;
        }
    }
    else //右下到左上
    {
        ixStart=MOT[Motor].Tray.XItem-1;
        ixEnd=-1;
        ixStep=-1;
        if(iSortWhichAuto<iAutoCnt)
        {
            iyStart=MOT[Motor].Tray.YItem-1;
            iyEnd=-1;
            iyStep=-1;
        }
    }

    if(Direction<4)                                                        //橫排先放
    {
        for(int iR=iyStart; iR!=iyEnd; iR+=iyStep)
        {
            for(int iC=ixStart; iC!=ixEnd; iC+=ixStep)
            {
                if(MOT[Motor].Tray.Data[iC][iR]==NULL_IC)
                {
                    itStartCol=iC;
                    itStartRow=iR;
                    break;
                }
            }

            if(itStartCol!=-1 && itStartRow!=-1)
                break;
        }
    }
    else
    {
        for(int iC=ixStart; iC!=ixEnd; iC+=ixStep)
        {
            for(int iR=iyStart; iR!=iyEnd; iR+=iyStep)
            {
                if(MOT[Motor].Tray.Data[iC][iR]==NULL_IC)
                {
                    itStartCol=iC;
                    itStartRow=iR;
                    break;
                }
            }

            if(itStartCol!=-1 && itStartRow!=-1)
                break;
        }
    }

    if(iSortWhichAuto==iAutoRight)                                                  //near right side
    {
        for(int i=0; i<OutArm2Suck.iPickRow; i++)
        {
            for(int j=OutArm2Suck.iPickCol-1; j>=0; j--)
            {
                isStartCol=serchtable[j];
                if(OutArm2Suck.Item[i][isStartCol]!=NULL_IC &&
                   OutArm2Suck.iWhichAuto[i][isStartCol]==iSortArmWhichAuto)
                {
                    bSortArmSuckActive[i][isStartCol]=true;
                    iCol=itStartCol;
                    iRow=itStartRow;
                    return ;
                }
            }
        }
    }
    else
    {
        for(int i=0; i<OutArm2Suck.iPickRow; i++)
        {
            for(int j=0; j<OutArm2Suck.iPickCol; j++)
            {
                isStartCol=serchtable[j];
                if(OutArm2Suck.Item[i][j]!=NULL_IC &&
                   OutArm2Suck.iWhichAuto[i][j]==iSortArmWhichAuto)
                {
                    bSortArmSuckActive[i][j]=true;
                    iCol=itStartCol;
                    iRow=itStartRow;
                    return;
                }
            }
        }
    }
}
//------------------------------------------------------------------------------
void AdjustSortArmClosePitchCondition(int iAuto, bool &bCanPick2ICAtOnceTime)
{
    int iResult=0;
    bCanPick2ICAtOnceTime=false;

    if(IniConfig.bI37_EnableFIFOMode &&
       IniConfig.bI37_EnableFIFOSiteOrder &&
       LastSet.iRunStartMode==rsmFIFOMode)                                      //Frank 20180810 (Steven) : add FIFO By Tester SiteMap
    {
        bCanPick2ICAtOnceTime=false;
        iSortArmXStep=1;
        return;
    }

    iResult=AutoCalculateSortArmXClosePitch(iAuto);
    if(ArmSpeed[OutArm].bVariModeFIX==false && !bSortUseOnebyOne)                   //jou 2010-09-07 當clean out時,out arm排列自動改為One by one排列
    {
        if(iResult==-1)
        {
            bCanPick2ICAtOnceTime=false;
        }
        else
        {
            bCanPick2ICAtOnceTime=true;
        }
    }

    if(AutoForm[iAuto]->Direction>=4)
        bCanPick2ICAtOnceTime=false;

    if(IniConfig.bUseTrayBlockMode &&                                           //Frank 20160928 add Subtray Function
       IniConfig.bP06_LoaderUseCarrierTray==true)                               //Ifor 20161129 add P06 判斷避免OutArm 一顆一顆放料
    {
        if((LoadForm->BlockXItem>0) || (LoadForm->BlockYItem>0))
        {
            bCanPick2ICAtOnceTime=false;
        }
    }
}
//------------------------------------------------------------------------------
bool CheckSortSuckICFallDown(bool AutoClean)
{
    #ifdef SOFT_SIMULTE
        return false;
    #else
    {
        int ret, iMotZ;
        AnsiString ErrPart="", Str;                                             //Steven 20110216 : 合併Alarm
        bool bHasDuplicateErr=false;                                            //Steven 20110216 : 合併Alarm
        bool bHasErr=false;                                                     //Steven 20110216 : 合併Alarm
        static bool bSuckErr[MAX_ARM_Row][MAX_ARM_Col]={{false, false, false, false}, {false, false, false, false}};  //Steven 20110216 : Avoid duplicate message
        int iXPos, iYPos;
        bOutArmDropICSkip=false;
        if(LastSet.iRealDummy==DUMMY)
            return false;

        if(AutoClean==true)                                                     //Steven 20110216 : 這邊是AutoClean的
        {
            if(IniConfig.bInOutArmPlaceSkipSuckDetect==true)                    //Steven 20171219 (Wei) : 修正[E35], 改成判斷Z軸是不是在Home點
            {
                for(int i=0; i<OutArmSuck.iMaxRow; i++)
                {
                    for(int j=0; j<OutArmSuck.iMaxCol; j++)
                    {
                        iMotZ=(InOutArmPickerUseMotor==eptUseMotCyn)?MOutArmZA:OutArmSuck.Suck[i][j].iMotNo;
                        if(MOT[iMotZ].Motor->Enable==true  &&
                           MOT[iMotZ].Led[iHomeLed]==true  &&
                           OutArmSuck.Item[i][j]!=NULL_IC &&
                           OutArmSuck.Item[i][j]!=HAS_NULL_IC)
                        {
                            if(OutArmSuck.Suck[i][j].GetStatus()==false)
                            {
                                iXPos=MOT[MOutArmX].ReadPos();
                                iYPos=MOT[MOutArmY].ReadPos();
                                ShowErrorMessage("JAM0203", K_SKIP, MOutArmX, false);
                                OutArmSuck.PordRec[i][j].AddErrorRecord("JAM0203", true, iXPos, iYPos); //Steven 20160114 (jou) : Add Production Error Log
                                OutArmSuck.SetItemData(i, j, NULL_IC);
                            }
                        }
                    }
                }
            }
            else
            {
                for(int i=0; i<OutArmSuck.iMaxRow; i++)
                {
                    for(int j=0; j<OutArmSuck.iMaxCol; j++)
                    {
                        if(OutArmSuck.Item[i][j]!=NULL_IC  &&
                           OutArmSuck.Item[i][j]!=HAS_NULL_IC)
                        {
                            if(OutArmSuck.Suck[i][j].GetStatus()==false)
                            {
                                iXPos=MOT[MOutArmX].ReadPos();
                                iYPos=MOT[MOutArmY].ReadPos();
                                ShowErrorMessage("JAM0203", K_SKIP, MOutArmX, false);
                                OutArmSuck.PordRec[i][j].AddErrorRecord("JAM0203", true, iXPos, iYPos); //Steven 20160114 (jou) : Add Production Error Log
                                OutArmSuck.SetItemData(i, j, NULL_IC);
                            }
                        }
                    }
                }
            }
            bHasDuplicateErr=false;
            return false;
        }
        else                                                                    //Steven 20110216 : 這邊是新增加的
        {
            ErrPart=" ";
            if(IniConfig.bInOutArmPlaceSkipSuckDetect==true)                    //Steven 20171219 (Wei) : 修正[E35], 改成判斷Z軸是不是在Home點
            {
                for(int i=0; i<OutArmSuck.iMaxRow; i++)
                {
                    for(int j=0; j<OutArmSuck.iMaxCol; j++)
                    {
                        if(OutArmSuck.Item[i][j]!=NULL_IC               &&
                           OutArmSuck.Item[i][j]!=HAS_NULL_IC           &&
                           OutArmSuck.Suck[i][j].GetStatus()==false)
                        {
                            if(LastSet.iRealDummy==REALLY)                      //suck ic
                            {
                                iMotZ=(InOutArmPickerUseMotor==eptUseMotCyn)?MOutArmZA:OutArmSuck.Suck[i][j].iMotNo;
                                if(MOT[iMotZ].Motor->Enable==true  &&
                                   MOT[iMotZ].Motor->ReadPos()>=0)              //Steven 20210929 : Out Arm掉料改用ReadPos判斷
                                {
                                    bSuckErr[i][j]=true;
                                    ErrPart+=OutArmSuck.Suck[i][j].sName;
                                    bHasErr=true;
                                }
                                else
                                {
                                    bSuckErr[i][j]=false;                       //Steven 20210913 : 修改out arm掉料判斷, 只針對有掉的地方做資料清除
                                }

                                iXPos=MOT[MOutArmX].ReadPos();
                                iYPos=MOT[MOutArmY].ReadPos();
                                OutArmSuck.PordRec[i][j].AddErrorRecord("JAM0203", true, iXPos, iYPos); //Steven 20160114 (jou) : Add Production Error Log
                            }
                        }
                        else
                        {
                            bSuckErr[i][j]=false;                               //Steven 20210913 : 修改out arm掉料判斷, 只針對有掉的地方做資料清除
                        }
                    }
                }
            }
            else
            {
                for(int i=0; i<OutArmSuck.iMaxRow; i++)
                {
                    for(int j=0; j<OutArmSuck.iMaxCol; j++)
                    {
                        if(OutArmSuck.Item[i][j]!=NULL_IC               &&
                           OutArmSuck.Item[i][j]!=HAS_NULL_IC           &&
                           OutArmSuck.Suck[i][j].GetStatus()==false)            //Steven 20220812 : Fixed for Skip後卡死
                        {
                            if(LastSet.iRealDummy==REALLY)                      //suck ic
                            {
                                iXPos=MOT[MOutArmX].ReadPos();
                                iYPos=MOT[MOutArmY].ReadPos();
                                OutArmSuck.PordRec[i][j].AddErrorRecordNoSave("JAM0203", false, iXPos, iYPos); //Steven 20160114 (jou) : Add Production Error Log //Steven 20190110 : Error Log Add Motor Position.
                                bSuckErr[i][j]=true;                            //Steven 20210913 : 修改out arm掉料判斷, 只針對有掉的地方做資料清除
                                ErrPart+=OutArmSuck.Suck[i][j].sName;
                                bHasErr=true;
                            }
                        }
                        else
                        {
                            bSuckErr[i][j]=false;
                        }
                    }
                }
            }

            if(bHasErr==true)
            {
                Str="";
                for(int i=0; i<OutArmSuck.iMaxRow; i++)
                {
                    for(int j=0; j<OutArmSuck.iMaxCol; j++)
                    {
                        iMotZ=(InOutArmPickerUseMotor==eptUseMotCyn)?MOutArmZA:OutArmSuck.Suck[i][j].iMotNo;
                        Str+=(OutArmSuck.Suck[i][j].sName+AnsiString(":"));
                        Str+=AnsiString(MOT[iMotZ].ReadPos())+AnsiString(", ");
                    }
                }
                RecordProcess("CheckOutSuckICFallDown JAM0203 - Picker Position", Str); //Steven 20210929 : 紀錄out arm掉料發生時, 吸嘴的位置

                if((IniConfig.bNewResetFunction==true && bResetOutArm==true) ||
                   IniConfig.bOurArmDropICSkip)                                 //kevin 20171005 out arm drop ic 只能強至取出ic 開6號門
                {
                    bResetOutArm=false;
                    if(IniConfig.bOurArmDropICSkip)                             //kevin 20171005 out arm drop ic 只能強至取出ic 開6號門
                    {
                        bOutArmDropICSkip=true;                                 //kevin 20171005
                        ret=ShowErrorMessage("JAM0203", K_SKIP, MOutArmX, bHasDuplicateErr, ErrPart);
                    }
                    else
                    {
                        RecordProcess("CheckOutSuckICFallDown JAM0203 - Auto SKIP", ErrPart);
                        ret=K_SKIP;
                    }
                }
                else
                {
                    if(CUSTOMER_CODE==CC_SIGURD_HUKOU)                          //KaiChen 20180326 ：矽格-湖口 OutArm Drop Error 只能 Skip
                    {
                        ret=ShowErrorMessage("JAM0203", K_SKIP, MOutArmX, bHasDuplicateErr, ErrPart);
                    }
                    else
                    {
                        ret=ShowErrorMessage("JAM0203", K_SKIP|K_RETRY, MOutArmX, bHasDuplicateErr, ErrPart);
                    }
                }

                if(ret==K_SKIP)
                {
                    for(int i=0; i<OutArmSuck.iMaxRow; i++)
                    {
                        for(int j=0; j<OutArmSuck.iMaxCol; j++)
                        {
                            if(bSuckErr[i][j]==true)
                            {
                                iXPos=MOT[MOutArmX].ReadPos();
                                iYPos=MOT[MOutArmY].ReadPos();
                                OutArmSuck.PordRec[i][j].AddErrorRecord("JAM0203", true, iXPos, iYPos); //Steven 20160114 (jou) : Add Production Error Log //Steven 20190110 : Error Log Add Motor Position.
                                OutArmSuck.SetItemData(i, j, NULL_IC);
                                OutArmSuck.Suck[i][j].Normal();
                                bOutArmSuckActive[i][j]=false;
                                bSuckErr[i][j]=false;
                                if(CosFunction.bUseSCKART && AutoClean==false)  //Steven 20161214 (wei) : For SCK ART
                                    fSCKART->iOutputJamCnt++;
                            }
                        }
                    }
                    bHasDuplicateErr=true;

                    if(IniConfig.bVTESTFunction==true)
                        bOutarmDropError=true;                                  //jou 20241024 : out arm drop error show alarm提醒檢查出Tray是否疊料
                    return true;
                }
                else
                {
                    bHasDuplicateErr=true;
                    return true;
                }
            }
            bHasDuplicateErr=false;
            return false;
        }
    }
    #endif
}
//------------------------------------------------------------------------------
void ShowSortShuttleDataMiss()                                                  //Steven 20110527
{
    ShowErrorMessage("WAR0256", 0, MOutSortX);                                  //Outut shuttle偵測bin資料不正確,將自動放置於interface error bin
}
//------------------------------------------------------------------------------
void SortArmAddBlockPitch(int &nowX, int &nowY, int iRow, int iCol)              //Frank 20160928 add Subtray Function
{
    if(IniConfig.bUseTrayBlockMode &&                                           //2014-03-04    Dell    for SPIL WLP Add Tray Block
       IniConfig.bP06_LoaderUseCarrierTray==true)                               //Ifor 20181012 (Steven) : add Block Pitch Calculation
    {
        if(AutoForm[0]->XDivision==0 || AutoForm[0]->YDivision==0)
            return;
        nowX=nowX+((iCol/AutoForm[0]->XDivision)* AutoForm[0]->BlockPitchX);
        nowX=nowX-((iCol/AutoForm[0]->XDivision)*(AutoForm[0]->XPitch*AutoForm[0]->XDivision));
        nowY=nowY-((iRow/AutoForm[0]->YDivision)* AutoForm[0]->BlockPitchY);
        nowY=nowY+((iRow/AutoForm[0]->YDivision)*(AutoForm[0]->YPitch*AutoForm[0]->YDivision));
    }
}
//------------------------------------------------------------------------------
bool MoveOutArmZToPlateSafe()                                                   //ChungHung 20140313 add Fix3 can Full Tray
{
    bool OK=true;
    int iAlarmSuck=0, iAlarmSuck1=0;
    static int iRetryCount=0;
    static int iRetryFail=0;
    int iMot=0;
    #ifndef SOFT_SIMULTE
    int iZPos=0;
    bool bHomeFlag=false;
    AnsiString sData;
    #endif
    for(int i=0; i<OutArmSuck.iMotRow; i++)
    {
        for(int j=0; j<OutArmSuck.iMotCol; j++)
        {
            iMot=(InOutArmPickerUseMotor==eptUseMotCyn)?MOutArmZA:OutArmSuck.Suck[i][j].iMotNo;
            if(MOT[iMot].MotorMove(Prod.ZOutArmSafe[i][j])==false)
            {
                OK=false;
                #ifndef SOFT_SIMULTE
                MOT[iMot].ScanMotorStatus();
                bHomeFlag=MOT[iMot].Led[iHomeLed];
                if(bHomeFlag)                                                   //JerryYang 20200924 : Z軸失步偵測
                {
                    iZPos=MOT[iMot].ReadPos();
                    if(iZPos<-200)
                    {
                        sData.sprintf("%s pos %d home sensor on", MOT[iMot].Alias, iZPos);
                        if(CUSTOMER_CODE==CC_ATEC)                              //Steven 20221224 : 艾科暫時不檢查Z軸Home Sensor
#if 0 // GATE(13) -- golden asortarm.cpp:1936 (golden's MyDBIProcess is 3-arg (golden cMyDB.h:20); this tree's is 2-arg (aHotPlateSubstrate.h:924))
                            MyDBIProcess("Motion", sData, "MoveOutArmZToPlateSafe");
#else
                            MyDBIProcess("Motion", sData+AnsiString(" [MoveOutArmZToPlateSafe]"));   // GATE(13): golden's 3rd arg (the DB function-name column) folded into the message rather than dropped -- established idiom, AutoClean/AutoClean.cpp:1170-1180
#endif
                        else
                            ShowMyMessage(sData, "", "MoveOutArmZToPlateSafe");
                        SetOutArmHome();
                    }
                }
                #endif
            }
        }
    }

    if(OK)
    {
        iAlarmSuck=OutArmZSafe(DETECT_ALL_FLAG);
        if(iAlarmSuck!=-1)
        {
            if(iRetryCount>50)
            {
                iRetryCount=0;
                if(iRetryFail<iAutoCnt)
                {
                    iRetryFail++;
                    SetOutArmHome();                                            //JerryYang 20210903 : 修正SetOutArmHome
                }
                else
                {
                    ShowErrorMessage("WAR0257", 0, MOutArmX, false, MOT[iAlarmSuck].NumberAlias);       //入料手臂 Z 沒有在歸零的位  //Steven 20171211 (wei): WAR0157 -->WAR0257
                    iRetryFail=0;
                }
                return false;
            }
            else
            {
                iRetryCount++;
                return false;
            }
        }
        iRetryFail=0;
        iRetryCount=0;
    }

    if(OK)
    {
        for(int i=0; i<OutArmSuck.iMotRow; i++)
        {
            for(int j=0; j<OutArmSuck.iMotCol; j++)
            {
                iMot=(InOutArmPickerUseMotor==eptUseMotCyn)?MOutArmZA:OutArmSuck.Suck[i][j].iMotNo;
                MOT[iMot].fCMD=false;
            }
        }
    }

    if(OK==true && iAlarmSuck==-1)                                              //Jou 2011-02-09
    {
        iAlarmSuck1=OutArmZSafe(DETECT_SENSOR_FLAG);
        if(iAlarmSuck1==-1)
        {
            return true;
        }
        else
        {
            return false;
        }
    }
    else
    {
        return false;
    }
}
//------------------------------------------------------------------------------
void SortTransferAutoRatio(int iSortWhichAuto, int *iXPos, int *iYPos)                  //Steven 20141029 : Auto的軟體齒輪比
{
//    if(MACHINE_HAS_AUTO_ALIGNMENT_CCD && TestIF.bEnableAutoAlignment==true)     //KenHsieh 20210813 : add CCD AUTO ALIGNMENT
//    {
//        CheckOutArmXYScaleByAutoTeach(*iXPos, *iYPos, iTo3Unload[iWhichAuto]);  //JerryYang 20240306 : fix AOA異常, 改用iTo3Unload
//    }
//    else                                                                        //KenHsieh 20210824 : add AOA功能開啟不補償
//    {
        if(IniConfig.bE31OutArmUseDifferentScale)                               //Steven 20090710 Start
        {
            double fi;
            fi=*iXPos-Prod.XStart[iSortWhichAuto][iSortArmYBase][iSortArmXBase];
            fi*=IniConfig.dTrayXScale[iSortWhichAuto];
            *iXPos=fi+Prod.XStart[iSortWhichAuto][iSortArmYBase][iSortArmXBase];

            fi=*iYPos-Prod.YStart[iSortWhichAuto][iSortArmYBase][iSortArmXBase];
            fi*=IniConfig.dTrayYScale[iSortWhichAuto];
            *iYPos=fi+Prod.YStart[iSortWhichAuto][iSortArmYBase][iSortArmXBase];
        }
//    }
}
//------------------------------------------------------------------------------
void SortTransferOutShuttleRatio(int iSht, int *iXPos, int *iYPos, int iRow, int iCol)//Steven 20141029 : Out Shuttle的軟體齒輪比
{
//    if(MACHINE_HAS_AUTO_ALIGNMENT_CCD && TestIF.bEnableAutoAlignment==true)     //KenHsieh 20210813 : add CCD AUTO ALIGNMENT
//    {
//        CheckOutArmXYScaleByAutoTeach(*iXPos, *iYPos, iSht+AOA_OutSht1);        //JerryYang 20241119 : fix AOA
//    }
//    else                                                                        //KenHsieh 20210824 : add AOA功能開啟不補償
//    {
        if(IniConfig.bE32ShuttleUseDifferentScale)                              //jou 980716 start : add Scale
        {
            double fi=0.0;
            if(iSht==0)
            {
                fi=*iXPos-Prod.XOutArm_Shuttle1_Pick[iRow][iCol];
                fi*=LastSet.fInShuttleXScale[0];
                *iXPos=fi+Prod.XOutArm_Shuttle1_Pick[iRow][iCol];

                fi=*iYPos-Prod.YOutArm_Shuttle1_Pick[iRow][iCol];
                fi*=LastSet.fInShuttleYScale[0];
                *iYPos=fi+Prod.YOutArm_Shuttle1_Pick[iRow][iCol];
            }
            else
            {
                fi=*iXPos-Prod.XOutArm_Shuttle2_Pick[iRow][iCol];
                fi*=LastSet.fInShuttleXScale[1];
                *iXPos=fi+Prod.XOutArm_Shuttle2_Pick[iRow][iCol];

                fi=*iYPos-Prod.YOutArm_Shuttle2_Pick[iRow][iCol];
                fi*=LastSet.fInShuttleYScale[1];
                *iYPos=fi+Prod.YOutArm_Shuttle2_Pick[iRow][iCol];
            }
        }
//    }
}
//------------------------------------------------------------------------------
int SortArmPickShuttleAlarm(int iSht, bool bHasDuplicateErr, AnsiString ErrPart) //Steven 20141121 : 整合OutArm吸取異常
{
    int ret=0, iCT=0;                                                           //Jimmychiu 20220901 加入初始值
    AnsiString sBuffer="";                                                      //Jimmychiu 20220901 加入初始值

    if(CUSTOMER_CODE==CC_GT)                                                    //jou 20220922 : 寰邦設備經理Chris 要求 Jam0201,Jam0202,Jam0203 只有Skip選項
    {
        if(iSht==0)
            ret=ShowErrorMessage("JAM0201", K_SKIP, MOutSortX, bHasDuplicateErr, ErrPart);
        else
            ret=ShowErrorMessage("JAM0202", K_SKIP, MOutSortX, bHasDuplicateErr, ErrPart);
    }
    else if(IniConfig.iE50_OutArmPickUpErrorOption==0)                          //RETRY & SKIP  //JerryYang 20210813 : RETRY+SKIP
    {
        if(IniConfig.bInOutArmCanPushHome)
        {
            if(iSht==0)
                ret=ShowErrorMessage("JAM0201", K_RETRY|K_SKIP|K_HOME, MOutSortX, bHasDuplicateErr, ErrPart); //ChungHung HT9045 2011/12/13 //Input pickup device error時,按"retry"鍵,機台都會自動home
            else
                ret=ShowErrorMessage("JAM0202", K_RETRY|K_SKIP|K_HOME, MOutSortX, bHasDuplicateErr, ErrPart); //ChungHung HT9045 2011/12/13 //Input pickup device error時,按"retry"鍵,機台都會自動home
        }
        else
        {
            if(iSht==0)
                ret=ShowErrorMessage("JAM0201", K_RETRY|K_SKIP, MOutSortX, bHasDuplicateErr, ErrPart);
            else
                ret=ShowErrorMessage("JAM0202", K_RETRY|K_SKIP, MOutSortX, bHasDuplicateErr, ErrPart);
        }
    }
    else if(IniConfig.iE50_OutArmPickUpErrorOption==1)                          //JerryYang 20210813 : RETRY  //Steven 20141121 : OutArm吸取異常只能Retry
    {
        if(IniConfig.bInOutArmCanPushHome)
        {
            if(iSht==0)
                ret=ShowErrorMessage("JAM0201", K_RETRY|K_HOME, MOutSortX, bHasDuplicateErr, ErrPart); //ChungHung HT9045 2011/12/13 //Input pickup device error時,按"retry"鍵,機台都會自動home
            else
                ret=ShowErrorMessage("JAM0202", K_RETRY|K_HOME, MOutSortX, bHasDuplicateErr, ErrPart); //ChungHung HT9045 2011/12/13 //Input pickup device error時,按"retry"鍵,機台都會自動home
        }
        else
        {
            if(iSht==0)
                ret=ShowErrorMessage("JAM0201", K_RETRY, MOutSortX, bHasDuplicateErr, ErrPart);
            else
                ret=ShowErrorMessage("JAM0202", K_RETRY, MOutSortX, bHasDuplicateErr, ErrPart);
        }
    }
    else                                                                        //JerryYang 20210813 : SKIP
    {
        if(IniConfig.bInOutArmCanPushHome)
        {
            if(iSht==0)
                ret=ShowErrorMessage("JAM0201", K_SKIP|K_HOME, MOutSortX, bHasDuplicateErr, ErrPart); //ChungHung HT9045 2011/12/13 //Input pickup device error時,按"retry"鍵,機台都會自動home
            else
                ret=ShowErrorMessage("JAM0202", K_SKIP|K_HOME, MOutSortX, bHasDuplicateErr, ErrPart); //ChungHung HT9045 2011/12/13 //Input pickup device error時,按"retry"鍵,機台都會自動home
        }
        else
        {
            if(iSht==0)
                ret=ShowErrorMessage("JAM0201", K_SKIP, MOutSortX, bHasDuplicateErr, ErrPart);
            else
                ret=ShowErrorMessage("JAM0202", K_SKIP, MOutSortX, bHasDuplicateErr, ErrPart);
        }
    }

    if(ret!=K_SKIP)
    {
        bSortArmPickShtHasRetry=true;
    }

    if(IniConfig.bA35SetErrBinWhenOutShtLoseAndPickupErr)                       //JerryYang 20220215 : 松諭要求的功能Out shuttle lose IC 以及 out arm pick up error set to error bin
    {
        OutSht3Kit.SetAllRealIC2InterfaceBin();
        iCT=OutSht3Kit.CountRealIC();                                   //JerryYang 20190226 : FR -> BR
        sBuffer.printf("Sort arm pick up error set to error bin : Sort Shttle Device=%d;", iCT);
        MyDBIProcess("Message", sBuffer);

        for(int i=0; i<OutArm2Suck.iPickRow; i++)
        {
            for(int j=0; j<OutArm2Suck.iPickCol; j++)
            {
                if(OutArm2Suck.Item[i][j]!=NULL_IC && OutArm2Suck.Item[i][j]!=HAS_NULL_IC)
                {
                    OutArm2Suck.SetItemData(i, j, TEST_PASS+iTestBinCount);
                    OutArm2Suck.PordRec[i][j].AddTestResultRecord(iTestBinCount, OutArm2Suck.cSBin[i][j]); //Frank 20160505 add
                    OutArm2Suck.iWhichAuto[i][j]=Prod.iIfErrorT6;
                }
            }
        }
        iCT=OutArm2Suck.CountRealIC();
        sBuffer.printf("Sort arm pick up error set to error bin : Sort arm Device=%d;", iCT);
        MyDBIProcess("Message", sBuffer);
    }

    return ret;
}
//------------------------------------------------------------------------------
void PorcessJAM0201SortArmPickUpErrorSkip(int iShtRow, int iShtCol, int iSuckRow, int iSuckCol)        //OutArm吸取異常Skip的ErrorLog
{
    int iXPos=MOT[MOutSortX].ReadPos();
    int iYPos=MOT[MOutSortY].ReadPos();

    OutSht3Kit.PordRec[iShtRow][iShtCol].AddErrorRecord("JAM0202", true, iXPos, iYPos);
    OutSht3Kit.SetItemData(iShtRow, iShtCol, NULL_IC);

    OutArm2Suck.Suck[iSuckRow][iSuckCol].Error=false;

//    if(CosFunction.bUseSCKART)                                                  //Steven 20161214 (wei) : For SCK ART
//        fSCKART->iOutputJamCnt++;
}
//==============================================================================
int GetSortArmXToShuttleOffset(int iOffsetPos)
{
    int iPos=0;
    iPos=SortArmOffSet[iOffsetPos]->GetX();
    return iPos;
}
//==============================================================================
int GetSortArmYToShuttleOffset(int iOffsetPos)
{
    int iPos=0;
    iPos=SortArmOffSet[iOffsetPos]->GetY();
    return iPos;
}
//------------------------------------------------------------------------------
//bool UnloaderTrayYDirForArmYPitch(int iAuto)
//{
//    if(USE_Y_AUTO_PITCH==true)                                                  //ChungHung 20131231 alter AutoYPitch
//    {
//        AutoCalculateOutArmYClosePitch(iAuto);
//        return true;
//    }
//    else if(LoadTrayCanUse8Suck()==1 && TestIF.bSearchLastMode)                 //kevin 20171120 (wei) 判斷LOAD TRAY Y PITCH是否是 ARM Y PITCH可以一次8吸嘴吸IC
//    {
//        return true;
//    }
//    return false;
//}
//==============================================================================
int AutoCalculateSortArmYClosePitch(int iAuto)                                   //ChungHung 20131231 alter AutoYPitch
{
    //int iMin=1500;                                                              //kevin 20190518 : OutArm 放置AUTO最後一排 合到最小PITCH IC 掉落
    OutArmClose_PitchY=0;
    return OutArmClose_PitchY;

//    if(USE_Y_AUTO_PITCH==false)
//    {
//        iSortArmYStep=1;
//        return TestIF.iARM_Y_PITCH;
//    }
//
//    if(IN_OUT_ARM_Y_PITCH_MIN>iMin)
//        iMin=IN_OUT_ARM_Y_PITCH_MIN;
//
//    if(AutoForm[iAuto]->YPitch>=iMin &&
//       AutoForm[iAuto]->YPitch<=IN_OUT_ARM_Y_PITCH_MAX)
//    {
//        OutArmClose_PitchY=AutoForm[iAuto]->YPitch;
//        return OutArmClose_PitchY;
//    }
//
//    if(AutoForm[iAuto]->YPitch<iMin)
//    {
//        int i=0;
//        do
//        {
//            i++;
//            if(AutoForm[iAuto]->YPitch*i>=iMin)
//            {
//                OutArmClose_PitchY=AutoForm[iAuto]->YPitch*i;
//                break;
//            }
//        }while(AutoForm[iAuto]->YPitch*i<=IN_OUT_ARM_Y_PITCH_MAX);
//
//        if(LoadTrayCanUse8Suck()==1 && TestIF.bSearchLastMode)                  //kevin 20171120 (wei) 判斷LOAD TRAY Y PITCH是否是 ARM Y PITCH可以一次8吸嘴吸IC
//            return TestIF.iARM_Y_PITCH;
//
//        if(OutArmClose_PitchY<=IN_OUT_ARM_Y_PITCH_MAX)
//            return OutArmClose_PitchY;
//        else
//            return TestIF.iARM_Y_PITCH;                                         //Steven 20160627 : Add for Y-Pitch  -1 --> 6000
//    }
//
//    return TestIF.iARM_Y_PITCH;                                                 //Steven 20160627 : Add for Y-Pitch  -1 --> 6000
}
//==============================================================================
#if 0 // GATE(7) -- golden asortarm.cpp:2249-2266 (golden defines CheckOutArmSuckOnlyGOrH TWICE (here and aoutarm.cpp:3013, byte-identical); this tree's aoutarm.cpp:3008 owns it)
bool CheckOutArmSuckOnlyGOrH()                                                  //Steven 20141128 : for 只開Ad,Bd Auto1 第一列沒有放IC
{
    if((OutArmSuck.Item[0][3]!=NULL_IC && OutArmSuck.Item[0][3]!=HAS_NULL_IC) ||
       (OutArmSuck.Item[1][3]!=NULL_IC && OutArmSuck.Item[1][3]!=HAS_NULL_IC))
    {
        if((OutArmSuck.Item[0][1]!=NULL_IC && OutArmSuck.Item[0][1]!=HAS_NULL_IC) ||
           (OutArmSuck.Item[0][2]!=NULL_IC && OutArmSuck.Item[0][2]!=HAS_NULL_IC) ||
           (OutArmSuck.Item[0][0]!=NULL_IC && OutArmSuck.Item[0][0]!=HAS_NULL_IC) ||
           (OutArmSuck.Item[1][1]!=NULL_IC && OutArmSuck.Item[1][1]!=HAS_NULL_IC) ||
           (OutArmSuck.Item[1][2]!=NULL_IC && OutArmSuck.Item[1][2]!=HAS_NULL_IC) ||
           (OutArmSuck.Item[1][0]!=NULL_IC && OutArmSuck.Item[1][0]!=HAS_NULL_IC))
        {
            return false;
        }
    }

    return true;
}
#endif
//==============================================================================
double dSortArmXPitch_1Step=0.0;                                                 //1吸嘴之間的X-Pitch
double dSortArmXPitch_MovePitch=0.0;                                             //張到最開的X-Pitch
int    iSortArmXStep=1;                                                          //Sort Arm在Tray X一次要跳幾格
int    iSortArmYStep=1;                                                          //Sort Arm在Tray Y一次要跳幾格
//------------------------------------------------------------------------------
int AutoCalculateSortArmXClosePitch(int iAuto, bool bUseDeviceDinemsion)
{
//    int iStep;
    SortArmClose_PitchX=0;
    int iMaxXPitchPick   =iXpitchMax;
    int iMinXPitchPick   =iXpitchMin;

    if(OutArm2Suck.iPickCol==1)                                                 //1x1 mode直接使用最小X-Pitch
    {
        SortArmClose_PitchX      =iXpitchMin;
        dSortArmXPitch_1Step     =SortArmClose_PitchX;
        dSortArmXPitch_MovePitch =SortArmClose_PitchX;
        return dSortArmXPitch_MovePitch;
    }
    else
    {
        if(bUseDeviceDinemsion==true)
        {
            for(int i=1; i<AutoForm[iAuto]->XDivision; i++)
            {
                iSortArmXStep=i;
                SortArmClose_PitchX=DeviceForm.XDimension*iSortArmXStep+100.0;  //要比IC大一點, 避免撞到

                if(SortArmClose_PitchX>=iMinXPitchPick && SortArmClose_PitchX<=iMaxXPitchPick)
                {
                    dSortArmXPitch_1Step     =SortArmClose_PitchX;
                    dSortArmXPitch_MovePitch =SortArmClose_PitchX*1.0;
                    return dSortArmXPitch_MovePitch;
                }
            }
        }
        else
        {
            int iStart=AutoForm[iAuto]->XDivision-1;
            if((AutoForm[iAuto]->XDivision%2)==0)
            {
                iStart=(AutoForm[iAuto]->XDivision/2);
            }

            if(iStart<1)
                iStart=1;

            if(AutoForm[iAuto]->XDivision==1)                                   //Steven 20240801 : for tray x item=1
            {
                SortArmClose_PitchX=iMaxXPitchPick;
                dSortArmXPitch_1Step=iMaxXPitchPick;
                dSortArmXPitch_MovePitch=iMaxXPitchPick;
                return dSortArmXPitch_MovePitch;
            }
            else
            {
                for(int i=iStart; i>0; i--)
                {
                    iSortArmXStep=i;
                    SortArmClose_PitchX=AutoForm[iAuto]->XPitch*iSortArmXStep;
                    if(SortArmClose_PitchX>=iMinXPitchPick && SortArmClose_PitchX<=iMaxXPitchPick)
                    {
                        dSortArmXPitch_1Step     =SortArmClose_PitchX;
                        dSortArmXPitch_MovePitch =dSortArmXPitch_1Step*1.0;
                        return dSortArmXPitch_MovePitch;
                    }
                }
            }
        }
    }

    iSortArmXStep=1;
    if(SortArmClose_PitchX==0)                                                   //Steven 20240205 : 修正有可能Pitch是0
        SortArmClose_PitchX=iMinXPitchPick;
    dSortArmXPitch_1Step     =iMaxXPitchPick;       //Jimmychiu 20241217 : OutArmClose_PitchX->iMaxXPitch4Pick
    dSortArmXPitch_MovePitch =dSortArmXPitch_1Step*1.0;
    SortArmClose_PitchX=iMaxXPitchPick;
    return dSortArmXPitch_MovePitch;
}
//------------------------------------------------------------------------------
bool GetSortArmToUnLoaderPosition(int bCanPick2ICAtOnceTime, int iSortPutTray, int &iXPos, int &iYPos, int iR, int iC, bool RealMove)
{
    int iVariablePara[]={2, 1, 0, -1, -1, -1, -1, -1};                          //實體吸嘴相對於基準軸的Pitch數量
    int iRealUseSuck=0;

    iVariablePara[0]=1;
    iVariablePara[1]=0;

    if(bCanPick2ICAtOnceTime==true)
    {
        iYPos=Prod.YStart[iSortPutTray][iSortArmYBase][iSortArmXBase]-iR*AutoForm[iSortPutTray]->YPitch;
        for(int j=0; j<OutArm2Suck.iPickCol; j++)
        {
            if(bSortArmSuckActive[iSortArmPlaceOrder][j]==true)
            {
                iRealUseSuck=OutArm2Suck.Suck[iSortArmPlaceOrder][j].iMyCol;
                break;
            }
        }

        iXPos=Prod.XStart[iSortPutTray][iSortArmYBase][iSortArmXBase]+iC*AutoForm[iSortPutTray]->XPitch+dSortArmXPitch_1Step*iVariablePara[iRealUseSuck];
        SortTransferAutoRatio(iSortPutTray, &iXPos, &iYPos);                         //Steven 20141029 : Auto的軟體齒輪比
        if(iXPos<(MOT[MOutSortX].Motor->PSoftLimitN+10) ||
           iXPos>(MOT[MOutSortX].Motor->PSoftLimitP-50))                         //Steven 20141217 : For Auto 1使用G & H吸嘴
        {
            bSortArmXOverLimit=true;                                             //Steven 20171206 (Wei) : In ATK, 修正Out Arm同時放料但是超出軟體極限縮Pitch導致疊料的問題
            return false;
        }
        else
        {
            if(RealMove==false)
                return true;
        }
    }
    else
    {
        for(int i=0; i<OutArm2Suck.iPickRow; i++)
        {
            for(int j=0; j<OutArm2Suck.iPickCol; j++)
            {
                if(bSortArmSuckActive[i][j])
                {
                    iYPos=Prod.YStart[iSortPutTray][i][j]-iR*AutoForm[iSortPutTray]->YPitch;
                    iRealUseSuck=OutArm2Suck.Suck[iSortArmPlaceOrder][j].iMyCol;
                    iXPos=Prod.XStart[iSortPutTray][iSortArmYBase][iSortArmXBase]+iC*AutoForm[iSortPutTray]->XPitch+dSortArmXPitch_1Step*iVariablePara[iRealUseSuck];

//                    if(IniConfig.bUseTrayBlockMode)                             //2014-03-04    Dell    for SPIL WLP Add Tray Block
//                        OutArmAddBlockPitch(iXPos, iYPos, iR, iC);

                    SortTransferAutoRatio(iSortPutTray, &iXPos, &iYPos);             //Steven 20141029 : Auto的軟體齒輪比
                    if(iXPos<(MOT[MOutSortX].Motor->PSoftLimitN+10) ||           //Steven 20141217 : For Auto 1使用G & H吸嘴
                       iXPos>(MOT[MOutSortX].Motor->PSoftLimitP-50))             //jou 2014-11-20 100->50 fix out arm device > 25mm place to Fix3 hang up
                    {
                        bSortArmXOverLimit=true;                                 //Steven 20240501 : 修正超出極限後, 要重新算一次X-Pitch
                    }

                    if(bSortArmXOverLimit==true)                                 //Steven 20240719 : 修正超出極限後, 要重新算一次X-Pitch
                        AutoCalculateSortArmXClosePitch(iSortWhichAuto, true);

                    iXPos=Prod.XStart[iSortPutTray][iSortArmYBase][iSortArmXBase]+iC*AutoForm[iSortPutTray]->XPitch+dSortArmXPitch_1Step*iVariablePara[iRealUseSuck];
//                    if(IniConfig.bUseTrayBlockMode)                             //2014-03-04    Dell    for SPIL WLP Add Tray Block
//                        SortArmAddBlockPitch(iXPos, iYPos, iR, iC);

                    SortTransferAutoRatio(iSortPutTray, &iXPos, &iYPos);             //Steven 20141029 : Auto的軟體齒輪比
                    iSortArmXPosition[i][j]=iC;
                    iSortArmYPosition=iR;
                    for(int k=i; k<OutArm2Suck.iPickRow; k++)
                        for(int l=j+1; l<OutArm2Suck.iPickCol; l++)
                            bSortArmSuckActive[k][l]=false;
                    break;
                }
            }
        }
    }
    return true;
}
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
//void SortArmAddSpeedDisplay()                                                    //KaiChen 20171225 (Steven)：Add Speed Display
//{
//    iSortArmSpeed1++;
//}
//------------------------------------------------------------------------------
//void SortArmSubSpeedDisplay()                                                    //KaiChen 20171225 (Steven)：Add Speed Display
//{
//    iSortArmSpeed1--;
//    if(iSortArmSpeed1>99)
//        iSortArmSpeed1=99;
//    if(iSortArmSpeed1<-99)
//        iSortArmSpeed1=-99;
//}
//------------------------------------------------------------------------------
#if 0 // GATE(8) -- golden asortarm.cpp:2442-2457 (golden defines all three TMyQASample methods TWICE (here and aoutarm.cpp:3580/:3585/:3593, byte-identical); this tree's aoutarm.cpp:3575-3590 owns them. The OBJECT QASamSort (golden :70) is NOT duplicated and stays defined above)
__fastcall TMyQASample::TMyQASample()
{
    ClearCount();
}
//------------------------------------------------------------------------------
void TMyQASample::ClearCount()
{
    ZeroMemory(iCount, sizeof(iCount));
    iMinX=0;
    iMinY=0;
    iCurrCount=0;
}
//------------------------------------------------------------------------------
void TMyQASample::FindDeviceToQA()
{
}
#endif
//------------------------------------------------------------------------------
int GetSortArmPitchX(int w, int iOffsetPos)                                  //RogerYang 20250510 Add for 9046AU
{
    if(iOffsetPos>=0)
        w+=SortArmOffSet[iOffsetPos]->GetXPitch(0);
    double m=0.0, r=0.0;
    double dX_40=Prod.iSortArmXMaxPitch;
    double dX_13=Prod.iSortArmXMinPitch;
    m=((double)(dX_40-dX_13))/(USE_OUT_SORT_X_PITCH_MAX-USE_OUT_SORT_X_PITCH_MIN);
    r=(double)dX_13+m*((double)w-double(USE_OUT_SORT_X_PITCH_MIN));
    return r;
}
//----------------------------------------------------------------------------- //RogerYang 20250510 Add for 9046AU
void GetSortArmZDownPos(int iSortWhichAuto, bool bNeedDown, bool ZDownSel[MAX_ARM_Row][MAX_ARM_Col], int iZPos[MAX_ARM_Row][MAX_ARM_Col])  //Steven 20230323 : For HT1032
{
    for(int i=0; i<OutArm2Suck.iMaxRow; i++)
    {
        for(int j=0; j<OutArm2Suck.iMaxCol; j++)
        {
            if(i<OutArm2Suck.iPickRow && j<OutArm2Suck.iPickCol)
            {
                if(bNeedDown==true)
                {
                    ZDownSel[i][j]=bSortArmSuckActive[i][j];
                }
                else
                {
                    iSortArmXPosition[i][j]=j;
                    bSortArmSuckActive[i][j]=true;
                    ZDownSel[i][j]=false;
                }

                if(ZDownSel[i][j])
                {
                    iZPos[i][j]=Prod.ZPlace[iSortWhichAuto][i][j];
                }
                else
                {
                    iZPos[i][j]=ZSafePos;
                }
            }
            else
            {
                ZDownSel[i][j]=false;
                iZPos[i][j]=ZSafePos;
            }
        }
    }
}
//-----------------------------------------------------------------------------
int GetSortArmToSortShuttleOffset()                                          //RogerYang 20250510 Add for 9046AU
{
    int iOffsetPos=-1;

    iOffsetPos=SortOfsSht;

    return iOffsetPos;
}
//-----------------------------------------------------------------------------
void SetSortArmNeedDestory(int iSuckRow, int iSuckCol, int iShtRow, int iShtCol)
{
    if(OutSht3Kit.Item[iShtRow][iShtCol]>NULL_IC)
        OutArm2Suck.Suck[iSuckRow][iSuckCol].SetNeedSuck(true);
}
//------------------------------------------------------------------------------
bool MoveSortArmXYToSortShtWait()                                               //RogerYang 20250510 Add for 9046AU
{
    bool bZFlag[MAX_ARM_Row][MAX_ARM_Col];
    int iZPos[MAX_ARM_Row][MAX_ARM_Col];
    int iXVariable;
    int iXPos               =Prod.XSortArm_SortShuttle_Pick[iSortArmYBase][iSortArmXBase];
    int iYPos               =Prod.YSortArm_SortShuttle_Pick[iSortArmYBase][iSortArmXBase];
    int iMovePitchX         =iXpitchMax;
//    int iMovePitchY         =0;                                                 //AU只有XPitch
    int iOffsetPos          =GetSortArmToSortShuttleOffset();
//    int iYVariable          =0;
    ZeroMemory(bZFlag,      sizeof(bZFlag));
    ZeroMemory(iZPos,       sizeof(iZPos));

//    if(IniConfig.bAlarmNeedServoOff)                                            //Steven 20111226 : In Arm 咖啡杯解除
//    {
//        if(fNote->bMyServoOffInArm)
//        {
//            return false;
//        }
//    }

    iXVariable=GetSortArmPitchX(iMovePitchX, iOffsetPos);

    if(SortArmContinuousMove(iXPos, iYPos, iXVariable, bZFlag, iZPos, false))
        return true;

    return false;
}
//-----------------------------------------------------------------------------
void SetSortArmHome()                                                           //RogerYang 20250510 Add for 9046AU
{
    AnsiString sHomeState="";
    sHomeState=sSortArmZHomeState();
    NewRecordProcess("","SortArm home sensor status",sHomeState);
    InitDoArmZHome();
    bNeedArmZHome=true;
    bIsSortArmHome=true;

    bSortArmPitchNeedHomeCheck=true;

    for(int i=0; i<OutArm2Suck.iPickRow; i++)
    {
        for(int j=0; j<OutArm2Suck.iPickCol; j++)
        {
            bSortArmZNeedHome[i][j]=true;
        }
    }
}
//------------------------------------------------------------------------------
int DoSortArmAfterPlaceToAuto(bool &bCheckSpeed, bool bInitial)                  //Steven 20220526 : 針對放下IC到Unloader後的動作做整合
{
    int iResult=0;
    int &Task=iDoSortArmAfterPlaceToAutoTask;
    if(bInitial)
    {
        Task=1;
        return iResult;
    }

    switch(Task)
    {
        case 1:
            if(OutArm2Suck.HasRealIC()==false)
            {
                Task=3000;
            }
            else
            {
                Task=1;
                iResult=3010;
            }
            break;
        case 3000:
            Task=CheckSortArmCleanOut(3100);
            if(ArmSpeed[InArm].bAutoSpeed)                                      //Steven 20110525 : Auto Speed
                bCheckSpeed=true;
            else
                bCheckSpeed=false;
            break;
        case 3100:
            Task=1;
            iResult=100;
            break;
        case 5000:
            if(IniConfig.bP27AutoSortingBinTrayByOutArmwhenCleanOut==true &&    //JerryYang 20170911 (Steven) Clean Out時整盤
               CosFunction.bSortingBinTraywhenCleanOut==true &&                 //是否使用Auto Sorting AutoTray Auto Sorting AutoTray的功能
               bSortingAllBinTrayFinish==false)                                 //是否所有的Bin Tray IC都整盤完成
            {
                //SortingBinTray_Data.iMAXSUCK_OutArmSuckX=4;                   //Sam 20250207 : 移位置 Mark
                //SortingBinTray_SetMotorPosData();                             //設定馬達、及點位資料
                //SortingBinTray_SetTraySpecData();                             //設定Tray盤資料
                //DoSortingBinTray(0);                                            //Bin IC排序主流程
                Task=5100;
                break;
            }

            if(MoveSortArmToAutoSafe())
            {
                if(iCleanOut==1)
                {
                    if(InArmSuck.HasIC()    ||
                       OutArmSuck.HasIC()   ||
                       ShuttleHasIC()       ||
                       OutArm2Suck.HasIC()  ||
                       IndexHasIC())
                    {
                        Task=1;
                        iResult=1;
                    }
                }
            }
            break;
        case 5100:                                                              //JerryYang 20150910 Auto Sorting BinTray by Out Arm when Clean Out
            //if(DoSortingBinTray()==true)                                        //Bin IC排序主流程
            {
                //if(bSortingAllBinTrayFinish==true)                              //是否所有的Bin Tray IC都整盤完成
                {
                    Task=5000;                                                  //暫時不整盤
                }
            }
            break;
    }

    return iResult;
}
//------------------------------------------------------------------------------
bool InitialSortArmNeedSuck()
{
//    int iPos1=0;
    bool bLedFlag1=false;

    for(int i=0; i<OutArm2Suck.iMaxRow; i++)
    {
        for(int j=0; j<OutArm2Suck.iMaxCol; j++)
        {
            OutArm2Suck.Suck[i][j].SetNeedSuck(false);                           //inital
        }
    }

//    iPos1=MOT[MOutSortSht].ReadPos();
    MOT[MOutSortSht].ScanMotorStatus();
    bLedFlag1=MOT[MOutSortSht].Led[iInposLed];
    if(bLedFlag1)
    {
        return false;
    }

    return true;
}
//------------------------------------------------------------------------------
void SetSortArmNeedSuck(int iSuckRow, int iSuckCol, int iShtRow, int iShtCol, bool bPlace)
{
    if(bPlace==true)
    {
        if(OutSht3Kit.Item[iShtRow][iShtCol]==NULL_IC)
#if 1 // AI(W906-W2-A46) 20260926: gate FLIPPED to the golden arm -- A4-6 (8ff6c754) merged TMyKitSuck/TMySucker into the one golden layout (mykitsuck.h), premise dead: TMySucker::SetNeedDestroy is mykitsuck.h:251. NOTE the old #else was NOT equivalent (golden has no else here: an empty shuttle pocket flags the nozzle to destroy) -- golden :2679
            OutArm2Suck.Suck[iSuckRow][iSuckCol].SetNeedDestroy(true);
#else
            ;                                                                   // GATE(2) offline: bNeedDestroy is already false, which is what golden's else-branch leaves it as
#endif
    }
    else
    {
        if(OutSht3Kit.Item[iShtRow][iShtCol]>NULL_IC)
            OutArm2Suck.Suck[iSuckRow][iSuckCol].SetNeedSuck(true);
    }
}
//------------------------------------------------------------------------------
bool SortArmZNeedDown_2x4_8()
{
    if(InitialSortArmNeedSuck()==false)
        return false;

    //RogerYang 20250526  只有兩隻吸嘴，或許不需要分這麼多mode，盡可能把吸嘴吸滿就可以去放料了
    /*
    1. Sort 左邊吸嘴優先吸料
    2. 從Shuttle Kit的左上開始找，由左至右，由上而下
    */

    //左吸嘴先找
//    bool bSortSuckUseL=false;
//    for(int iShtRowL=0; iShtRowL<2; iShtRowL++)       //上至下
//    {
//        for(int iShtColL=0; j<iShtColL; iShtColL++)   //左至右
//        {
//            if(OutSht3Kit.Item[iSiShtRowLhtRow][iShtColL]>NULL_IC)
//            {
//                SetSortArmNeedSuck(0, 0, iShtRowL, iShtColL, false);
//                bSortSuckUseL=true;
//                break;
//            }
//        }
//    }
//
//    if(bSortSuckUseL==true) //左吸嘴用掉了，才準備用右吸嘴
//    {
//        //先看能不能一次吸兩顆
//        bool bCanPick2Once=iXpitchMax-TestIF.dSiteXPitch>=0?true:false;
//        //換右吸嘴找
//        if(bCanPick2Once==true && iShtRowL<4)
//        {
//            for(int iShtRowR=0; iShtRowR<2; iShtRowR++)       //上至下
//            {
//                for(int iShtColR=0; j<iShtColR; iShtColR++)   //左至右
//                {
//                    if
//
//                    if(OutSht3Kit.Item[iShtRowR][iShtColR]>NULL_IC)
//                    {
//                        SetSortArmNeedSuck(0, 0, iShtRowR, iShtColR, false);
//                        bSortSuckUseL=true;
//                        break;
//                    }
//                }
//            }
//        }
//    }

    int iMode   =GetNowSortShuttleMode_2x4_8();
    int iModeRow=iMode%100;
    int iModeCol=iMode/100;
    int iShtJ   =0;
    int iRow    =0;
    bool bPlace=false;

    iLocModeRow=iModeRow;
    iLocModeCol=iModeCol;   //這是要讓SortA吸嘴知道現在吸的是Shuttle的哪一區
//
    for(int i=0; i<OutArm2Suck.iPickRow; i++)
    {
        for(int j=0; j<OutArm2Suck.iPickCol; j++)
        {
            iShtJ=GetShuttleCol(i, j);
//            if(iModeCol==0 &&
//               (iModeRow==2 || iModeRow==i))                                    //Steven 20240205 : 修正 Out arm 放料
//            {
//                SetSortArmNeedSuck(i, j, i, iShtJ, bPlace);
//            }
//            else
            if(iModeCol==100 && (j==0 || j==1))                                 // O O            O O X X
            {
                iRow=iModeRow==0?0:1;                                                                //                O O X X
                SetSortArmNeedSuck(i, j, iRow, iShtJ, bPlace);
            }
            else if(iModeCol==110 && (j==0 || j==1))                               // O O            X X O O
            {
                iRow=iModeRow==0?0:1;                                                             //                X X O O
                SetSortArmNeedSuck(i, j, iRow, iShtJ+2, bPlace);
            }
//            else if(iModeCol==200 && j==0 &&                                    //吸嘴       ==>  蝦頭
//                    (iModeRow==2 || iModeRow==i))                               // O X X X        O X X X
//            {                                                                   // O X X X        O X X X
//                SetOutArmNeedDestory(i, j, i, iShtJ, bPlace);
//            }
//            else if(iModeCol==201 && j==1 &&                                    //吸嘴       ==>  蝦頭
//                    (iModeRow==2 || iModeRow==i))                               // X O X X        X O X X
//            {                                                                   // X O X X        X O X X
//                SetOutArmNeedDestory(i, j, i, iShtJ, bPlace);
//            }
//            else if(iModeCol==210 && j==2 &&                                    //吸嘴       ==>  蝦頭
//                    (iModeRow==2 || iModeRow==i))                               // X X O X        X X O X
//            {                                                                   // X X O X        X X O X
//                SetOutArmNeedDestory(i, j, i, iShtJ, bPlace);
//            }
//            else if(iModeCol==211 && j==3 &&                                    //吸嘴       ==>  蝦頭
//                    (iModeRow==2 || iModeRow==i))                               // X X X O        X X X O
//            {                                                                   // X X X O        X X X O
//                SetOutArmNeedDestory(i, j, i, iShtJ, bPlace);
//            }
        }
    }
    return true;
}
//------------------------------------------------------------------------------
bool CheckSortArmXYPitch_2x4_8(int *iX, int *iY, int iMovePitchX)
{
    int iMode       =GetNowSortShuttleMode_2x4_8();
    int iModeRow    =iMode%100;
    int iModeCol    =iMode/100;
    int iOffsetPos  =GetSortArmToSortShuttleOffset();
//    double dMovePitchX;

    *iX+=Prod.XSortArm_SortShuttle_Pick[iSortArmYBase][iSortArmXBase];
    *iY+=Prod.YSortArm_SortShuttle_Pick[iSortArmYBase][iSortArmXBase];

    if(iModeRow==0)                                                             //Row A
    {
        *iY=*iY+TestIF.dSiteYPitch/2;
    }
    else if(iModeRow==1)                                                        //Row B
    {
        *iY=*iY-TestIF.dSiteYPitch/2;
    }
    else                                                                        //Both
    {
    }

    //基準為第二隻吸嘴
//    dMovePitchX=double(iMovePitchX);                                            //40mm
    if(iModeCol==0)                                                         //Aa --> Aa
    {
//        *iX=*iX+(dMovePitchX*2.0-TestIF.dSiteXPitch*1.5);
    }
    else if(iModeCol==100)                                                  //Aa --> Aa
    {
        //*iX=*iX+(dMovePitchX*1.0-TestIF.dSiteXPitch*1.5);
        *iX=*iX-TestIF.dSiteXPitch/2;
    }
    else if(iModeCol==110)                                                  //Ab --> Ac
    {
        //*iX=*iX+(dMovePitchX*1.0+TestIF.dSiteXPitch*0.5)
        *iX=*iX+TestIF.dSiteXPitch/2*3;
    }
//    else if(iModeCol==200)                                                  //Aa --> Aa
//    {
//        if(TestIF_File.bEnableUseXCenterPitch==true)
//            *iX=*iX+(dMovePitchX*2.0-TestIF.dSiteXCenterPitch*1-TestIF.dSiteXPitch*0.5);
//        else
//            *iX=*iX+(dMovePitchX*2.0-TestIF.dSiteXPitch*1.5);
//    }
//    else if(iModeCol==201)                                                  //Ab --> Ab
//    {
//        if(TestIF_File.bEnableUseXCenterPitch==true)
//            *iX=*iX+(dMovePitchX*1.0-TestIF.dSiteXCenterPitch*0.5);
//        else
//            *iX=*iX+(dMovePitchX*1.0-TestIF.dSiteXPitch*0.5);
//    }
//    else if(iModeCol==210)                                                  //Ac --> Ac
//    {
//        if(TestIF_File.bEnableUseXCenterPitch==true)
//            *iX=*iX+(dMovePitchX*0.0+TestIF.dSiteXCenterPitch*0.5);
//        else
//            *iX=*iX+(dMovePitchX*0.0+TestIF.dSiteXPitch*0.5);
//    }
//    else if(iModeCol==211)                                                  //Ad --> Ad
//    {
//        if(TestIF_File.bEnableUseXCenterPitch==true)
//            *iX=*iX+(-dMovePitchX*1.0+TestIF.dSiteXCenterPitch*0.5+TestIF.dSiteXPitch*1.0);
//        else
//            *iX=*iX+(-dMovePitchX*1.0+TestIF.dSiteXPitch*1.5);
//    }

    *iY=*iY+GetSortArmXToShuttleOffset(iOffsetPos);
    *iX=*iX+GetSortArmYToShuttleOffset(iOffsetPos);
    //TransferSortShuttleRatio(iSht, iX, iY, iOutArmYBase, iOutArmXBase);       //暫不考慮
    if(SortArmZNeedDown_2x4_8()==false)
        return false;
    return true;
}
//------------------------------------------------------------------------------
int iSortShtRetryCount=0;
bool bSortSuckShtDupErr[MAX_ARM_Row][MAX_ARM_Col]={{false, false, false, false}, {false, false, false, false}};
void GetSortShuttleStatus_Pick(int iZPos[MAX_ARM_Row][MAX_ARM_Col], bool bZDown, bool bZFlag[MAX_ARM_Row][MAX_ARM_Col])
{
    for(int i=0; i<OutArm2Suck.iPickRow; i++)
    {
        for(int j=0; j<OutArm2Suck.iPickCol; j++)
        {
            if(bZDown && OutArm2Suck.Suck[i][j].GetNeedSuckStatus())
                bZFlag[i][j]=true;
            else
                bZFlag[i][j]=false;

            if(bZDown)
            {
                iZPos[i][j]=Prod.ZSortArm_SortShuttle_Pick[i][j]+iSortShtRetryCount*ArmSpeed[OutArm].dRetryDown;
            }
            else
            {
                iZPos[i][j]=ZSafePos;
            }
        }
    }
}
//------------------------------------------------------------------------------
bool IsCheckSortArmDestroyActiveFinish(int iXPos, int iYPos)
{
    int iPosX=0, iPosY=0;
    int iPreRange=100;
    bool bRet=true;
    iPosX=MOT[MOutSortX].ReadPos();
    iPosY=MOT[MOutSortY].ReadPos();

    if(iPosX>=iXPos-iPreRange && iPosX<=iXPos+iPreRange &&
       iPosY>=iYPos-iPreRange && iPosY<=iYPos+iPreRange)
    {
        for(int i=0; i<OutArm2Suck.iPickRow; i++)
        {
            for(int j=0; j<OutArm2Suck.iPickCol; j++)
            {
                if(bCheckSortArmDestroyActiveFinish[i][j]==false)
                    bRet=false;
            }
        }
    }

    if(bRet==false)                                                             //為了避免卡死, 先把吸嘴抬起來
    {
        MoveSortArmToAutoSafe();
    }

    return bRet;
}
//------------------------------------------------------------------------------
void DoSortArmSuckPreOn(int iKit, int iXPos, int iYPos, bool bZDown)
{
    if(bZDown==false)
        return;

//    int iPosX=0, iPosY=0;
//    int iPreRange=50;
    //iKit=(iKit==0)?0:OutArm2Suck.iPickKitStep;

    if(LastSet.iRealDummy==REALLY &&
       ArmSpeed[OutArm].bSuckOnDown &&
       Sen[SnRKManualStep].IsOn()==false)
    {
//        iPosX=MOT[MOutSortX].ReadPos();
//        iPosY=MOT[MOutSortY].ReadPos();
//        if(iPosX>=iXPos-iPreRange && iPosX<=iXPos+iPreRange &&
//           iPosY>=iYPos-iPreRange && iPosY<=iYPos+iPreRange)
//        {
//            for(int i=0; i<OutArm2Suck.iPickRow; i++)
//            {
//                for(int j=0; j<OutArm2Suck.iPickCol; j++)
//                {
//                    if(OutSht3Kit->Item[i][j+iKit] &&
//                       OutArm2Suck.Suck[i][j].GetNeedSuckStatus())
//                        OutArm2Suck.Suck[i][j].On();
//                }
//            }
//        }
    }
}
//==============================================================================
bool SwapShuttleDataToSortArm(int iShtRow, int iShtCol, int iSuckRow, int iSuckCol, int *iWitchErrBin)
{
    bool bHasErr=false;
    int iT6;

    //LastSet.iIndexInputOutPut[2]++;                                             //kevin 20130125 計數吸取ic
//    if(IniConfig.bO20InOutArmPickerLifeTimeCount==true)                         //JerryYang 20220331 : add 吸真空次數計數
//    {
//        TestIF_File.OutArmPickerLifeCnt[iSuckRow][iSuckCol]++;
//    }
    OutArm2Suck.Suck[iSuckRow][iSuckCol].SetNeedSuck(false);

    if(CheckOutputShuttleDataMiss(&OutSht3Kit, iShtRow, iShtCol))
        bHasErr=true;

    iT6=OutSht3Kit.Item[iShtRow][iShtCol]-TEST_PASS;
    OutArm2Suck.MoveSuckDataDiff(OutSht3Kit, iShtRow, iShtCol, iSuckRow, iSuckCol);

    //OutArmSuck.PordRec[iSuckRow][iSuckCol].AddOutArmShuttlePickRecord(iSuckRow, iSuckCol);

//    if(iSht==0 &&
//       IsNNMode()==NN_2Row)
//    {
//        OutArmSuck.PordRec[iSuckRow][iSuckCol].AddOutShuttleDetectRecord(TestIF.iSiteMap[iShtRow+2][iShtCol], iShtRow+2, iShtCol); //Sam 20201216 : Add record
//    }
//    else if(IsNNMode()==NN_1Row)
//    {
//        OutArmSuck.PordRec[iSuckRow][iSuckCol].AddOutShuttleDetectRecord(TestIF.iSiteMap[iShtRow+1][iShtCol], iShtRow+1, iShtCol); //Sam 20201216 : Add record
//    }
//    else
//    {
//        OutArmSuck.PordRec[iSuckRow][iSuckCol].AddOutShuttleDetectRecord(TestIF.iSiteMap[iShtRow][iShtCol], iShtRow, iShtCol);    //Sam 20201216 : Add record
//    }

//    OutArmSuck.iNeedRotAng[iSuckRow][iSuckCol]=tRotate.RotateDutDate[1][iShtRow][iShtCol];   //預計旋轉的角度

    CheckBin(iT6, 2); //這個一定要做
//    if(CheckBin(iT6, iSht)==eBulkBox)                                           //kevin 20160906 by arm error bin check
//        *iWitchErrBin=eBulkBox;

//    if(fAutoTeach->IsRun())                                                     //JimmyChiu 20211020 : Auto alignment mode
//    {
//        OutArmSuck.iWhichAuto[iSuckRow][iSuckCol]=eFix3;
//    }
//    else if(IniConfig.bUseAutoSiteMapping && LastSet.iRunStartMode==rsmAutoSiteMap && IniConfig.bASMAutoSiteMapBinComBine)//kevin 20150115  Auto Site map 所有bin 別 放在盤FIX 2 //kevin 20150115
//    {
//        OutArmSuck.iWhichAuto[iSuckRow][iSuckCol]=eFix2;                        //AutoSiteMap 所有ic放到fix 2
//    }
//    else if(CosFunction.bHaveFIFOMode && LastSet.iRunStartMode==rsmFIFOMode)    //Steven 20160303 : FIFO Mode
//    {
//        OutArmSuck.iWhichAuto[iSuckRow][iSuckCol]=eAuto2;
//    }
//    else if(TestIF_File.bEnableQASampling && OutArmSuck.bQATray[iSuckRow][iSuckCol]==true && Prod.iQASamplingT6Pos!=ePosNoUse &&     //Steven 20190326 : QA Sampling
//            TestIF_File.bSCKART_EnableART && fSCKART->iLotCount!=0)
//    {
//        OutArmSuck.iWhichAuto[iSuckRow][iSuckCol]=Prod.iQASamplingT6;
//    }
//    else if(IniConfig.bA30SetupTeachFunction && LastSet.iTester==OFF_LINE && LastSet.bNeedSetupTeach && iSortUnloadT6>=0)  //JerryYang 20180921 Setup Teach功能
//    {
//        OutArmSuck.iWhichAuto[iSuckRow][iSuckCol]=iSortUnloadT6;
//    }
//    else
//    {
        OutArm2Suck.iWhichAuto[iSuckRow][iSuckCol]=iT6;
//    }

//    InspectSortArmPosition(MOutShuttle1+iSht, iSuckRow, iSuckCol, iShtRow, iShtCol, iOutPickFromSht);  //未實作

    return bHasErr;
}
//------------------------------------------------------------------------------
bool DoSortPickFromShuttle_2x4_8()
{
    int ret;
    int &Task=iPickFromSortShtTask;
    static int iWitchErrBin=0;
    AnsiString ErrPart="";                                                      //Steven 20110216 : 合併Alarm
    bool bHasDuplicateErr=false;                                                //Steven 20110216 : 合併Alarm
    bool bHasErr=false;                                                         //Steven 20110216 : 合併Alarm
    bool flag=false;
    int iSuckRow, iSuckCol, iShtRow, iShtCol;
    //int iPickKit=(iSortArmiWhichKit==0)?0:OutArm2Suck.iPickKitStep;
    int iPickKitRow=iLocModeRow==0?0:1;         //由Model決定
    int iPickKitCol=iLocModeCol==100?0:OutArm2Suck.iPickKitStep;

    switch(Task)
    {
        case 1:
            if(ArmSpeed[OutArm].dWaitOnSH!=0)                                   //kevin 20210525 out arm 起動在shuttle 上等待 時間
            {
                if(OutArm2Suck.NoIC())                                           //kevin 20210615 第一次 吸取IC 需等待
                {
                    if(MoveSortArmToShuttleIncludeZ_2x4_8(false))      //移到 Shuttle 上等待
                    {
                        PickFromSortSht();                                     //kevin 20210612 add int iwhichKit=0
                        Task=2;
                    }
                }
                else
                {
                    Task=10;
                }
            }
            else
            {
                Task=10;
            }
            break;
        case 2:
            if(bSortShtwaitPick())                                               //kevin 20210602 Out arm sht on wait
                Task=10;
            break;
        case 10:
            iWitchErrBin=0;
            Task=200;
        case 200:
            if(MoveSortArmToShuttleIncludeZ_2x4_8(true))
            {
//                if(SortArmNeedCheckOffset(false))      //還沒寫
//                {
//                    Task=500;
//                    break;
//                }
                OutArm2Suck.ResetAll();                                        //Steven 20160323 : 避免未開啟真空
                Task=iOUTARM_SUCK;
            }
            break;
        case 500:
            if(bEnterOffset)
            {
                bEnterOffset=false;
                bSortArmManualStepPress=true;
                Task=1;
                return false;
            }
            else
            {
                Task=iOUTARM_SUCK;
                OutArm2Suck.ResetAll();                                          //Steven 20160323 : 避免未開啟真空
            }
            break;
        case iOUTARM_SUCK:
            flag=true;
            bHasErr=false;
            for(int i=0; i<OutArm2Suck.iPickRow; i++)
            {
                for(int j=0; j<OutArm2Suck.iPickCol; j++)
                {
                    iSuckRow  =i;
                    iSuckCol  =j;
                    iShtRow   =i+iPickKitRow;   //_todo
                    iShtCol   =j+iPickKitCol;;
                    if(OutSht3Kit.Item[iShtRow][iShtCol] &&
                       OutArm2Suck.Suck[iSuckRow][iSuckCol].GetNeedSuckStatus())//SetSortArmNeedSuck設定要不要吸
                    {
                        if(OutArm2Suck.Suck[iSuckRow][iSuckCol].Suck())
                        {
                            //模擬ic不見了
                            #ifdef SOFT_SIMULTE
                            if(fMain->chkInPickLoadError->Checked==true)
                            {
                                if(j==1)
                                {
                                    OutArm2Suck.Suck[iSuckRow][iSuckCol].Error=true;
                                    continue;
                                }
                            }
                            #endif

                            bHasErr=SwapShuttleDataToSortArm(iShtRow, iShtCol, iSuckRow, iSuckCol, &iWitchErrBin);   //Steven 20170428 : 統一Out Arm從Shuttle吸料後的資料交換
                            bSortSuckShtDupErr[iSuckRow][iSuckCol]=false;
                        }
                        else if(OutArm2Suck.Suck[iSuckRow][iSuckCol].Error==false)
                        {
                            flag=false;
                        }
                    }
                }
            }

            if(bHasErr)
            {
                ShowOutputShuttleDataMiss();  //未針對sort寫
            }

            if(flag==false)
                break;

            for(int i=0; i<OutArm2Suck.iPickRow; i++)
            {
                for(int j=0; j<OutArm2Suck.iPickCol; j++)
                {
                    if(OutArm2Suck.Suck[i][j].Error)
                    {
                        iSortShtRetryCount++;
                        Task=2000;
                        return false;
                    }
                }
            }

            for(int i=0; i<OutArm2Suck.iPickRow; i++)
            {
                for(int j=0; j<OutArm2Suck.iPickCol; j++)
                    if(OutArm2Suck.Item[i][j]==NULL_IC && OutArm2Suck.Suck[i][j].GetNeedSuckStatus())
                        return false;
            }

//            if(iHWFix_BinBox==1 && ErrorBinBoxDetect(iWitchErrBin)==false)
//                return false;                                                   //kevin 20160906 add error bin box

//            if(USE_Fix_AI_CCD==true && TestIF_File.bEnableFix2BGAAICCD==true)   //KaiChen 20190124 ：矽格湖口 AI CCD Function
//            {
//                fFixAICCD->OutArmCycleCounterUpdate();
//            }

            iSortShtRetryCount=0;
            Task=1;
            return true;
        case 2000:
            if(MoveSortArmToAutoSafe())
            {
                bHasErr=false;
                ErrPart=" ";
                bHasDuplicateErr=false;                                         //Steven 20110216 Start : 整合Alarm
                for(int i=0; i<OutArm2Suck.iPickRow; i++)
                {
                    for(int j=0; j<OutArm2Suck.iPickCol; j++)
                    {
                        if(bSortSuckShtDupErr[i][j])
                            bHasDuplicateErr=true;
                        if(OutArm2Suck.Suck[i][j].Error)
                        {
                            ErrPart+=OutArm2Suck.Suck[i][j].sName;
                            bHasErr=true;
                        }
                        else
                        {
                            bSortSuckShtDupErr[i][j]=false;
                        }
                    }
                }

                if(bHasErr)
                {
                    if(iSortShtRetryCount>ArmSpeed[OutArm].iRetryCT)
                    {
                        if(!MoveOutArmXY_ToShuttleAlarmArea())
                            return false;

                        ret=OutArmPickShuttleAlarm(1, bHasDuplicateErr, ErrPart);   //暫時隨便呼叫一個alarm

                        for(int i=0; i<OutArm2Suck.iPickRow; i++)
                        {
                            for(int j=0; j<OutArm2Suck.iPickCol; j++)
                            {
                                if(OutArm2Suck.Suck[i][j].Error)
                                {
                                    bSortSuckShtDupErr[i][j]=true;
                                }
                                else
                                {
                                    bSortSuckShtDupErr[i][j]=false;
                                }
                            }
                        }
                    }
                    else
                    {
                        ret=K_RETRY;
                    }

                    if(ret==K_SKIP)
                    {
                        for(int i=0; i<OutArm2Suck.iPickRow; i++)
                        {
                            for(int j=0; j<OutArm2Suck.iPickCol; j++)
                            {
                                if(OutArm2Suck.Suck[i][j].Error)
                                {
                                    iShtCol=GetShuttleCol(i, j);
                                    PorcessJAM0201SortArmPickUpErrorSkip(i, iShtCol, i, j);
                                }
                                bSortSuckShtDupErr[i][j]=false;
                            }
                        }

//                        if(USE_Fix_AI_CCD==true &&                              //KaiChen 20190124 ：矽格湖口 AI CCD Function
//                           TestIF_File.bEnableFix2BGAAICCD==true)
//                        {
//                            fFixAICCD->OutArmCycleCounterUpdate();
//                        }

                        iSortShtRetryCount=0;
                        ZeroMemory(bSortSuckShtDupErr, sizeof(bSortSuckShtDupErr));
                        Task=1;
                        return true;
                    }
                    else                                                        //Steven 20190313 : Fixed for out arm pick error need home
                    {
                        if(iSortShtRetryCount>ArmSpeed[OutArm].iRetryCT)
                        {
                            iSortShtRetryCount=0;
                            if(IniConfig.bInOutArmCanPushHome)                  //ChungHung HT9045 2011/12/13 //Input pickup device error時,按"retry"鍵,機台都會自動home
                            {
                                if(ret==K_HOME)
                                    Task=2200;
                                else
                                    Task=1;
                            }
                            else
                            {
                                if(ret==K_RETRY)
                                    Task=2200;
                                else
                                    Task=1;
                            }
                            return false;
                        }
                    }
                }
                else
                {
                    ZeroMemory(bSortSuckShtDupErr, sizeof(bSortSuckShtDupErr));
                }
                Task=1;
            }
            break;
        case 2200:
            SetSortArmHome();
            iSortShtRetryCount=0;
            Task=1;
            break;
    }
    return false;
}
//==============================================================================
bool DoSortArmPlaceToWitchAuto(int iToWhichAuto)        //需考慮之後與DoOutArmPlaceToAuto(int iToWhichAuto)整合
{
    static bool bSuckDuplicateErr[MAX_ARM_Row][MAX_ARM_Col]={{false, false, false, false}, {false, false, false, false}};  //Steven 20091218 : Avoid duplicate message
    int Motor=iMMAuto[iToWhichAuto];
    int iXPos=0, iYPos=0;
    int iXPitchPos=0, iXPitch2Pos=0, iYPitchPos=0;                              //JerryYang 20210119 : add x,y pitch位置
    int iTrayRow=0, iTrayCol=0;
    bool flag=true;
    bool bHasDuplicateErr=false;
    bool bHasErr=false;
    AnsiString ErrPart=" ", str="";
    AnsiString asCheck="";                                                      //2012.06.01 Q_Q
    AnsiString asOCRCode="";                                                    //wei 20161005 紀錄OCR Code
    AnsiString asInfo="";

    iSortArmYStep=0;

    Motor=iMMAuto[iToWhichAuto];

    for(int i=0; i<OutArm2Suck.iPickRow; i++)
    {
        for(int j=0; j<OutArm2Suck.iPickCol; j++)
        {
            if(bSortArmSuckActive[i][j])
            {
                if(OutArm2Suck.Item[i][j]==HAS_NULL_IC)                          //Steven 20170113 : 沒IC的地方不檢查吹氣
                {
                    OutArm2Suck.SetItemData(i, j, NULL_IC);
                }
                else
                {
                    flag=OutArm2Suck.Suck[i][j].Destroy();
                    if(flag)
                    {
                        iTrayCol=iSortArmXPosition[i][j];
                        iTrayRow=iSortArmYPosition+iSortArmYStep*i;
                        MOT[Motor].Tray.iWhichSite [iTrayCol][iTrayRow]=OutArm2Suck.iWhichSite[i][j];
                        MOT[Motor].Tray.iWhichIndex[iTrayCol][iTrayRow]=OutArm2Suck.iWhichIndex[i][j];
                        MOT[Motor].Tray.iBinCode   [iTrayCol][iTrayRow]=OutArm2Suck.Item[i][j];
                        MOT[Motor].Tray.iWhichAuto [iTrayCol][iTrayRow]=OutArm2Suck.iWhichAuto[i][j];      //JerryYang 20221215 : Magazine把fix區當buffer區功能
                        MOT[Motor].Tray.iBinData   [iTrayCol][iTrayRow]=OutArm2Suck.iBinData[i][j];
                        MOT[Motor].Tray.iAOIResult [iTrayCol][iTrayRow]=OutArm2Suck.iAOIResult[i][j];      //Sam 20240325 : 新增 DamageTrayMapping 功能

                        //InspectSortArmPosition(iToWhichAuto, OutArm2Suck.Suck[i][j].iMyRow, OutArm2Suck.Suck[i][j].iMyCol, iTrayRow, iTrayCol, iOutPlaceToAuto); //未實作

                        if(OutArm2Suck.Item[i][j]>=START_TEST)
                        {
                            if(OutArm2Suck.iWhichSite[i][j]>=0 &&
                               OutArm2Suck.iWhichSite[i][j]<32)                  //Steven 20240320 : 避免溢位
                                LotSummary.AddCount(OutArm2Suck.iWhichSite[i][j], OutArm2Suck.Item[i][j]-TEST_PASS);

#if 0 // GATE(5) -- golden asortarm.cpp:3343-3350 (TfSCKART has no iInfo_MultiLotCnt -- same gate as aTester_Front.cpp:2374; the #else arm IS golden's own else-branch, which is what a single-lot run executes)
                            if(fSCKART->iInfo_MultiLotCnt>1)
                            {
                                LotSummary.AddByLotCount(OutArm2Suck.iWhichSite[i][j], OutArm2Suck.Item[i][j]-TEST_PASS, OutArm2Suck.cDeviceInf[i][j]);
                            }
                            else
                            {
                                asInfo.sprintf("picker %d - %d site CH is -1", i, j);
                            }
#else
                            {
                                asInfo.sprintf("picker %d - %d site CH is -1", i, j);
                            }
#endif
                        }

                        if(LastSet.iTester==_2D_SORT &&
                           CosFunction.bSortingBy2DList &&
                           TestIF_File.bSortingBy2DIDList)                      //JerryYang 20230814 : 有開2D SORT才存
                        {
                            sl2DMappingLog->MyInsert2DMappingToFile(OutArm2Suck.cDeviceInf[i][j], true); //JerryYang 20230322 : add 2D mapping result
                        }

                        if(OutArm2Suck.Item[i][j]>=START_TEST)                   //Steven 20171031 (wei) : Add TTMyTray滑鼠滑過去顯示Hint
                        {
                            if(TestIF_File.bEnableBarCode)
                            {
                                asCheck.sprintf("%s", OutArm2Suck.cReDeviceInf[i][j]);
                                if(asCheck=="0" || asCheck=="")  //AI(W906-NUMCMP) 20260927: golden compares with AnsiString(0) = "0" in BCB6 (NULL/0 is the int 0 there; NB2 R89, bcc32 5.6.4 measured); vclcompat would take it as a null const char* = "" -- written as "0" so the port keeps the golden meaning
                                    asInfo.sprintf("BIN: %d, 2DID: ERROR %s", OutArm2Suck.Item[i][j]-TEST_PASS, OutArm2Suck.cDeviceInf[i][j]);
                                else
                                    asInfo.sprintf("BIN: %d, 2DID: %s", OutArm2Suck.Item[i][j]-TEST_PASS, OutArm2Suck.cDeviceInf[i][j]);
                            }
                            else
                            {
                                asInfo.sprintf("BIN: %d", OutArm2Suck.Item[i][j]-TEST_PASS);
                            }
                        }
                        else
                        {
                            if(TestIF_File.bEnableBarCode)
                            {
                                asCheck.sprintf("%s", OutArm2Suck.cReDeviceInf[i][j]);
                                if(asCheck=="0" || asCheck=="")  //AI(W906-NUMCMP) 20260927: golden compares with AnsiString(0) = "0" in BCB6 (NULL/0 is the int 0 there; NB2 R89, bcc32 5.6.4 measured); vclcompat would take it as a null const char* = "" -- written as "0" so the port keeps the golden meaning
                                    asInfo.sprintf("BIN: %d, 2DID: ERROR %s", OutArm2Suck.Item[i][j]-TEST_PASS, OutArm2Suck.cDeviceInf[i][j]);
                                else
                                    asInfo.sprintf("BIN: %d, 2DID: %s", OutArm2Suck.Item[i][j], OutArm2Suck.cDeviceInf[i][j]);
                            }
                            else
                            {
                                asInfo.sprintf("BIN: %d", OutArm2Suck.Item[i][j]);
                            }
                        }

                        if((OutArm2Suck.Item[i][j]-TEST_PASS)>iTestBinCount)     //error bin
                            iByBinTotal[iTestBinCount]++;                       //kevin 20180705 (wei) bin 數量 Bin total[0]
                        else
                            iByBinTotal[OutArm2Suck.Item[i][j]-TEST_PASS]++;     //kevin 20180705 (wei) bin 數量 Bin total[0]

                        if(CosFunction.bTrayOCR && TestIF.bOcrFunction)         //wei 20161005 紀錄OCR Code
                            asOCRCode=OutArm2Suck.cDeviceInf[i][j];
                        else
                            asOCRCode="na";

                        if(IniConfig.bN14_9_ULQtyReport==true)
                        {
                            if(iToWhichAuto>=iFixRight)
                                LastSet.iN14_9_TrayICCnt[iToWhichAuto-3]++;     //Sam 20190801 : Bug Fix
                            else
                                LastSet.iN14_9_TrayICCnt[iToWhichAuto]++;       //Sam 20190801 : Bug Fix
                        }

                        iXPos=MOT[MOutSortX].ReadPos();                          //Frank 20160727 add iXPos iYPos
                        iYPos=MOT[MOutSortY].ReadPos();                          //Frank 20160727 add iXPos iYPos
                        iXPitchPos=MOT[MOutSortPitchX].ReadPos();                 //JerryYang 20210119 : add x,y pitch位置

                        bSortArmCheckDestroyACT[i][j]=true;                      //kevin 20140220 回吸檢測 jou 981130 確認device確實destroy完成

                        if(TestIF_File.bEnableBarCode)
                        {
                            asCheck.sprintf("%s", OutArm2Suck.cReDeviceInf[i][j]);
                            if(asCheck=="0" || asCheck=="")  //AI(W906-NUMCMP) 20260927: golden compares with AnsiString(0) = "0" in BCB6 (NULL/0 is the int 0 there; NB2 R89, bcc32 5.6.4 measured); vclcompat would take it as a null const char* = "" -- written as "0" so the port keeps the golden meaning (result code 0 -> HAS_IC like golden, not HAS_BARCODEERROR_IC)
                                MOT[Motor].SetTrayBinData(iTrayCol, iTrayRow, HAS_IC, asInfo);              //ChungHung 20131231 alter AutoYPitch
                            else
                                MOT[Motor].SetTrayBinData(iTrayCol, iTrayRow, HAS_BARCODEERROR_IC, asInfo); //ChungHung 20131231 alter AutoYPitch

                            if(LastSet.iRealDummy==REALLY  &&                   //jou 20190930 : Barcode Tray record file
                               TestIF_File.bEnableBarCode==true)                //jou 20191008 : Barcode duplicate file by out arm
                            {
                                if(CosFunction.bBarcodeTrayRecFile==true)
                                    MOT[Motor].Tray.cDeviceInf[iTrayCol][iTrayRow]=OutArm2Suck.cDeviceInf[i][j];

                                if(CosFunction.bBarcodeDuplicateFileByOutArm==true && TestIF_File.bCheckCodeByLot==true &&
                                   OutArm2Suck.Item[i][j]!=HAS_NULL_IC && OutArm2Suck.Item[i][j]!=NULL_IC &&
                                   OutArm2Suck.cDeviceInf[i][j]!=asBarCodeErrorSend && OutArm2Suck.cDeviceInf[i][j]!="")
                                {
#if 0 // GATE(6) -- golden asortarm.cpp:3433-3436 (TfBarCode (BarCode/BarCode.h, partial) has neither list2DByLot nor map2DList (the tree carries them as free globals, a different object); enclosing guard stays live)
                                    fBarCode->list2DByLot->Clear();
                                    fBarCode->map2DList[OutArm2Suck.cDeviceInf[i][j]]=i;
                                    fBarCode->list2DByLot->Add(OutArm2Suck.cDeviceInf[i][j]);
                                    WriteDataToFile(asBarCodeLot, fBarCode->list2DByLot->Text);   //Steven 20160429 : 開程式要把2D List讀回來
#endif
                                }
                            }
                        }
                        else
                        {
//                            if(IniConfig.bBinBox==true &&                       //jou 2012-12-11 support Bin Box
//                               iToWhichAuto==iBinBoxAtFix &&
//                               AutoForm[iBinBoxAtFix]->iTrayType==iBinBoxType)
//                            {
//                                LastSet.iBinBoxCount++;
//                            }
//                            else
//                            {
                                MOT[Motor].SetTrayBinData(iTrayCol, iTrayRow, HAS_IC, asInfo);  //ChungHung 20131231 alter AutoYPitch
//                            }
                        }
                        bSortArmSuckActive[i][j]=false;

                        if(AUTO3_IS_MAGAZINE==1             &&                  //JerryYang 20221215 : Magazine把fix區當buffer區功能
                           TestIF_File.iMagFixTrayType==1   &&
                           iToWhichAuto>=iMagAtAuto)
                        {
                        }
                        else
                        {
                            for(int k=0; k<4; k++)
                            {
                                LastSet.BinCT[k][iTo3Unload[OutArm2Suck.iWhichAuto[i][j]]]++;

                                if(IniConfig.bVTESTFunction==true &&            //marvin 20200424 (Kirin) Added always record report by time.
                                   IniConfig.bCheckFile==true)
                                {
                                    #ifdef SOFT_SIMULTE
                                    if(OutArm2Suck.iWhichAuto[i][j]<=iFixRightHalf)
                                        LastSet.iBinCTForAlways[k][OutArm2Suck.iWhichAuto[i][j]]++;
                                    #else
                                    if(LastSet.iTester==ON_LINE &&
                                       OutArm2Suck.iWhichAuto[i][j]<=iFixRightHalf)
                                        LastSet.iBinCTForAlways[k][OutArm2Suck.iWhichAuto[i][j]]++;
                                    #endif
                                }

                                if(CosFunction.bSpecailLowYeild &&
                                   Prod.bFailAlarmLowYieldSpecial)              //Sam 20210505 : PTI 要求的兩段 Low Yeild
                                    LastSet.BinCT_PTI[k][OutArm2Suck.iWhichAuto[i][j]]++;

                                if(CUSTOMER_CODE==CC_TERAPOWER &&
                                   Prod.bD22SupportMultiDoubleContact)     //Sam 20180612 : 有開啟 D22 Double Contact Contact， 以第一次的測試結果來做 ProcessCount
                                {
                                    if(OutArm2Suck.iBinDataBackUp[i][j]<0)
                                        LastSet.iBinData32[k][iTestBinCount]++;
                                    else
                                        LastSet.iBinData32[k][OutArm2Suck.iBinDataBackUp[i][j]]++;
                                }
                                else
                                {
                                    if(OutArm2Suck.iBinData[i][j]<0)             //jou 20170606 (wei) : 修正iBinData陣列被填入-1
                                        LastSet.iBinData32[k][iTestBinCount]++;
                                    else
                                        LastSet.iBinData32[k][OutArm2Suck.iBinData[i][j]]++;
                                }

                                if((USE_AUTO_RETEST==eartInstall && (bAutoReTest_ART || IniConfig.bA10_AutoReTest)) || CosFunction.bUseARTSortCount) //kevin 20150615   //Ifor 20170315 (wei) add 新增使用ART Sort Count 計數功能
                                {
                                    if(CUSTOMER_CODE==CC_TERAPOWER &&
                                       Prod.bD22SupportMultiDoubleContact) //Sam 20180612 : 有開啟 D22 Double Contact Contact， 以第一次的測試結果來做 ProcessCount
                                    {
                                        if(OutArm2Suck.iBinDataBackUp[i][j]<0)
                                            LastSet.iBinData32_ART[k][iTestBinCount]++;
                                        else
                                            LastSet.iBinData32_ART[k][OutArm2Suck.iBinDataBackUp[i][j]]++;
                                    }
                                    else
                                    {
                                        LastSet.BinCT_ART[k][iTo3Unload[OutArm2Suck.iWhichAuto[i][j]]]++;
                                        if(OutArm2Suck.iBinData[i][j]<0)         //jou 20170606 (wei) : 修正iBinData陣列被填入-1
                                            LastSet.iBinData32_ART[k][iTestBinCount]++;
                                        else
                                            LastSet.iBinData32_ART[k][OutArm2Suck.iBinData[i][j]]++;
                                    }
                                }
                            }

                            if(bCanRunSCKART==true)                             //Steven 20161201 (wei) : For SCK 93K ART
                            {
                                LastSet.lSCKARTBinCT[OutArm2Suck.iWhichAuto[i][j]]++;  //Steven 20170113 (wei) : For count unload count
                                if(TestIF_File.iSCKART_SortMode==1)
                                {
                                    if(Prod.bCateRTo6Tray[OutArm2Suck.iWhichAuto[i][j]])
                                        LastSet.iSCKART_RTUnitCount++;
                                    else if(Prod.bART6Tray[OutArm2Suck.iWhichAuto[i][j]])   //Steven 20190605 : 可能沒選CateR
                                        LastSet.iSCKART_RTUnitCount++;
                                }
                                else
                                {
                                    if(Prod.bART6Tray[OutArm2Suck.iWhichAuto[i][j]])
                                        LastSet.iSCKART_RTUnitCount++;
                                }
                            }
                        }

                        fSortCT->ShowSortIC();

                        if(AUTO3_IS_MAGAZINE==1             &&                  //JerryYang 20221215 : Magazine把fix區當buffer區功能
                           TestIF_File.iMagFixTrayType==1   &&
                           iToWhichAuto>=iMagAtAuto)
                        {
                            MOT[Motor].Tray.PordRec[iTrayCol][iTrayRow]->asBuffer->CommaText=OutArm2Suck.PordRec[i][j].asBuffer->CommaText;  //把production log資料過到fix buffer區
                        }
                        else
                        {
                            if(AUTO3_IS_MAGAZINE==1 &&
                               iToWhichAuto==iMagAtAuto)                        //JerryYang 20230509 : Magazine production log
                            {
                                OutArm2Suck.PordRec[i][j].AddUnloadRecord(OutArm2Suck.iWhichAuto[i][j],
                                                                         LastSet.iUnloaderTrayCount_ART[OutArm2Suck.iWhichAuto[i][j]],
                                                                         iTrayRow, iTrayCol,                                             //Frank 20160505 add
                                                                         iXPos, iYPos,                                                   //Frank 20160727 add iXPos iYPos
                                                                         asOCRCode,                                                      //wei 20161005 紀錄OCR Code
                                                                         iXPitchPos, iXPitch2Pos, iYPitchPos,                            //JerryYang 20210119 : add x,y pitch位置
                                                                         "");                                                            //Steven 20220120 : Amlogic需要收SBIN
                            }
                            else
                            {
                                OutArm2Suck.PordRec[i][j].AddUnloadRecord(iToWhichAuto,
                                                                         LastSet.iUnloaderTrayCount_ART[iToWhichAuto],                   //Frank 20160505 add
                                                                         iTrayRow, iTrayCol,                                             //Frank 20160505 add
                                                                         iXPos, iYPos,                                                   //Frank 20160727 add iXPos iYPos
                                                                         asOCRCode,                                                      //wei 20161005 紀錄OCR Code
                                                                         iXPitchPos, iXPitch2Pos, iYPitchPos,                            //JerryYang 20210119 : add x,y pitch位置
                                                                         "");                                                            //Steven 20220120 : Amlogic需要收SBIN
                            }

                            MOT[Motor].Tray.PordRec[iTrayCol][iTrayRow]->asBuffer->CommaText=OutArm2Suck.PordRec[i][j].asBuffer->CommaText;      //Steven 20221005 : Production Log減少記憶體使用量

                            if(OutArm2Suck.PordRec[i][j].asBuffer->Strings[eTestCate]=="E")  //JerryYang 20230322 : Lot summary要計算各類型ERR的數量
                            {
#if 0 // GATE(14) -- golden asortarm.cpp:3574-3586 (vclcompat TStringList::Strings[] yields a StringsProxy (operator AnsiString), which has no .Pos(); each subject is wrapped in AnsiString(...) -- pure type adaptation, identical strings/thresholds/branch order)
                                if(OutArm2Suck.PordRec[i][j].asBuffer->Strings[eErrorCode].Pos("Duplicate 2DID error")>0)
                                {
                                    iE2Count++;
                                }
                                else if(OutArm2Suck.PordRec[i][j].asBuffer->Strings[eErrorCode].Pos("NonTestToRBin")>0 ||
                                        OutArm2Suck.PordRec[i][j].asBuffer->Strings[eErrorCode].Pos("No2DCodeDevice")>0)
                                {
                                    iE1Count++;
                                }
                                else if(OutArm2Suck.PordRec[i][j].asBuffer->Strings[eErrorCode].Pos("2DID is not in sorting list")>0)
                                {
                                    iE3Count++;
                                }
#else
                                if(AnsiString(OutArm2Suck.PordRec[i][j].asBuffer->Strings[eErrorCode]).Pos("Duplicate 2DID error")>0)
                                {
                                    iE2Count++;
                                }
                                else if(AnsiString(OutArm2Suck.PordRec[i][j].asBuffer->Strings[eErrorCode]).Pos("NonTestToRBin")>0 ||
                                        AnsiString(OutArm2Suck.PordRec[i][j].asBuffer->Strings[eErrorCode]).Pos("No2DCodeDevice")>0)
                                {
                                    iE1Count++;
                                }
                                else if(AnsiString(OutArm2Suck.PordRec[i][j].asBuffer->Strings[eErrorCode]).Pos("2DID is not in sorting list")>0)
                                {
                                    iE3Count++;
                                }
#endif
                                else
                                {
                                    iENotDefinedCount++;
                                }
                            }

                            if(IniConfig.bOCRAndBinLog &&
                               TestIF.bOcrFunction &&
                               CosFunction.bTrayOCR)                            //KenHsieh 20230406 : 新增OCR Data + Bin Log功能
                            {
#if 0 // GATE(9) -- golden asortarm.cpp:3597 (TfOCR declares only ChangeLightValue; SaveOCRAndBinData exists nowhere in this tree)
                                fOCR->SaveOCRAndBinData(asOCRCode, OutArm2Suck.PordRec[i][j].asBuffer->Strings[eTestCate], Prod.iIsPassT6[iToWhichAuto]);   //Steven 20240105 : Prod.bIsPass --> Prod.iIsPassT6
#endif
                            }
#if 0 // GATE(10) -- golden asortarm.cpp:3599 (TMyProductionRecord::SaveRecord is explicitly out of scope tree-wide and not even declared -- Public/MyProductionRecord.h:24-30)
                            OutArm2Suck.PordRec[i][j].SaveRecord();              //Frank 20160505 add
#endif
                            if(IniConfig.bQAMode==true &&
                               LastSet.iRunStartMode==rsmQAMode &&
                               CosFunction.bQAModeUseUnloadCnt)                 //JerryYang 20221004 : Maxim版本QA mode
                            {
                                if((OutArm2Suck.Item[i][j]-TEST_PASS)==1)
                                {
                                    iQAModePassCT++;
                                    if(iQAModePassCT==Prod.iQAModeCount)
                                    {
                                        MOT[Motor].SetTray(HAS_IC, __FUNC__);
                                        str.sprintf("Bin 1 count met QA Sample count:%d ea", Prod.iQAModeCount);
                                        ShowMyMessage(str);
                                    }
                                }
                            }
                        }
                        OutArm2Suck.SetItemData(i, j, NULL_IC);

                        if(CosFunction.bTraySortCntFunc &&                      //Steven 20210608 : Tray指定數量退盤功能
                           iToWhichAuto<iAutoCnt &&
                           LastSet.iRunStartMode==rsmContinuStart)
                        {
                            if(TrayForm.bTraySortCntFunc[iToWhichAuto]==true &&
                               LastSet.iTrayCntForSortFunc[iToWhichAuto]<6)
                            {
                                LastSet.iTrayICCntForSortFunc[iToWhichAuto]++;
                            }
                        }
                    }
                    else if(OutArm2Suck.Suck[i][j].Error)
                    {
                        ErrPart+=OutArm2Suck.Suck[i][j].sName;
                        bHasErr=true;
                    }
                    else
                    {
                        bSuckDuplicateErr[i][j]=false;
                    }
                }
            }

            if(bSortArmSuckActive[i][j])
                flag=false;
        }
    }

    if(bHasErr)
    {
        ShowErrorMessage("JAM0217", K_RETRY, MOutSortX, bHasDuplicateErr, ErrPart);  //jou 2015-03-27 JAM0203->JAM0217 Device drop error -> Vacuum sensor OFF error
        for(int i=0; i<OutArm2Suck.iMaxRow; i++)
        {
            for(int j=0; j<OutArm2Suck.iMaxCol; j++)
            {
                if(OutArm2Suck.Suck[i][j].Error)
                {
                    bSuckDuplicateErr[i][j]=true;
                    iXPos=MOT[MOutSortX].ReadPos();
                    iYPos=MOT[MOutSortY].ReadPos();
                    OutArm2Suck.PordRec[i][j].AddErrorRecordNoSave("JAM0217", true, iXPos, iYPos);   //Steven 20160114 (jou) : Add Production Error Log
                }
                else
                {
                    bSuckDuplicateErr[i][j]=false;
                }
            }
        }
    }

    for(int i=0; i<OutArm2Suck.iMaxRow; i++)
        for(int j=0; j<OutArm2Suck.iMaxCol; j++)
            if(bSortArmSuckActive[i][j])
                return false;

    if(IniConfig.bBinBox==true &&                                               //jou 2012-12-11 support Bin Box
       iToWhichAuto==iBinBoxAtFix &&
       AutoForm[iBinBoxAtFix]->iTrayType==iBinBoxType)
    {
        if(LastSet.iBinBoxCount>=AutoForm[iBinBoxAtFix]->iBinBoxAlarm)
        {
            MOT[Motor].SetTray(HAS_IC, __FUNC__);
        }
    }

    if(CosFunction.bTraySortCntFunc && iToWhichAuto<iAutoCnt)                   //Steven 20210608 : Tray指定數量退盤功能
    {
        if(TrayForm.bTraySortCntFunc[iToWhichAuto]==true &&
           LastSet.iTrayCntForSortFunc[iToWhichAuto]<6 &&
           LastSet.iRunStartMode==rsmContinuStart)
        {
            int iTrayCnt=LastSet.iTrayCntForSortFunc[iToWhichAuto];
            if(LastSet.iTrayICCntForSortFunc[iToWhichAuto]>=TrayForm.iTraySortCntFunc[iToWhichAuto][iTrayCnt])  //當目前IC大於設定值,就要退盤
            {
                MOT[Motor].SetNullIcToHasNullIc();                              //退盤
                LastSet.iTrayICCntForSortFunc[iToWhichAuto]=0;
                do
                {
                    LastSet.iTrayCntForSortFunc[iToWhichAuto]++;                //下一盤
                }
                while(LastSet.iTrayCntForSortFunc[iToWhichAuto]<6 &&            //先判斷陣列內的值,要小於6
                      TrayForm.iTraySortCntFunc[iToWhichAuto][LastSet.iTrayCntForSortFunc[iToWhichAuto]]==0);
            }
        }
    }

#if 0 // GATE(11) -- golden asortarm.cpp:3704 (TfSortCT declares ShowLoadingIC/ShowSortIC only; CheckTheYieldAfterPlaceAuto absent (golden cSortCT.cpp unported))
    fSortCT->CheckTheYieldAfterPlaceAuto();                                     //Sam 20230306 : 修正 Initial Start 誤報警問題
#endif

    return true;
}
//------------------------------------------------------------------------------
bool DoSortArmPlaceToAuto()
{
    int ct;
    bool flag;
    static int iRetry=0;
    int &Task=iSortPlaceToAutoTask;
    AnsiString asAutoOffset;

    switch(Task)
    {
        case 1:
            bSortArmXOverLimit=false;                                            //Steven 20171206 (Wei) : In ATK, 修正Out Arm同時放料但是超出軟體極限縮Pitch導致疊料的問題
//            InitialFix3CanFullTask();                                           //ChungHung 20140313 add Fix3 can Full Tray
            Task=10;
        case 10:
            if(CheckSortSuckICFallDown(false)==true)                             //Steven 20210916 : 全時檢查out arm掉料
            {
                Task=220;
                return false;
            }

            iSortWhichAuto=SearchTrayToPlace_9045(true);                                //Steven 20160524 : 簡化Out Arm程式碼
            IfUseOnebyOne(iSortWhichAuto);                                          //Steven 20111101 : 整合成function     //Steven 20210608 : 加入iWhichAuto判斷

//            if(AUTO3_IS_MAGAZINE==1 &&                                          //JerryYang 20221215 : Magazine把fix區當buffer區功能
//               iWhichAuto>=iMagMin  &&
//               TestIF_File.iMagFixTrayType==1)
//            {
//                ct=iWhichBuff;
//            }
//            else
//            {
                ct=iSortWhichAuto;
//            }

//            if(TRAY_ARM_MODE==eAboveCoveyor &&
//               (MOT[iMMAuto[ct]].fHasTray==false ||
//                (CheckUseFixBinBoxFunction()==false &&                          //kevin 20160912
//                 MOT[iMMAuto[ct]].FullIC())))                                   //Steven 20091002 : 當要放IC時，編輯成滿盤會Hang Up，修正
//            {
//                InitOutArmTask();
//                break;
//            }

//            if(iWhichAuto!=eBulkBox && (EnableFix3UseCylinder()))
//            {
//                if(UseFix3Cylinder(ct)==false)
//                    break;
//            }

            flag=SetSortArm();
            if(flag)
            {
                if(MOT[iMMAuto[ct]].fHasTray==false ||
                   MOT[iMMAuto[ct]].FullIC())
                    Task=30;
                else
                    Task=50;
            }
            else                                                                //jou 2012-09-04 修正極限無法到達時,回填Null_IC會造成找不到地方放,會放在 x=-1,y=-1的位置上
            {
                if(bSortOverTray==true)
                {
                    MOT[iMMAuto[ct]].SetTray(HAS_IC, __FUNC__);
                    return true;
                }
            }
            break;
        case 30:
            IsCatchTrayReadySupplyNewTray();
//            if(AUTO3_IS_MAGAZINE==1 &&                                          //JerryYang 20221215 : Magazine把fix區當buffer區功能
//               iWhichAuto>=iMagMin  &&
//               TestIF_File.iMagFixTrayType==1)
//            {
//                ct=iWhichBuff;
//            }
//            else
//            {
                ct=iSortWhichAuto;
//            }

            if(MOT[iMMAuto[ct]].fHasTray &&
               MOT[iMMAuto[ct]].FullIC()==false)
                Task=10;
            break;
        case 50:
            if(CheckSortSuckICFallDown(false)==false)                            //Steven 20110216 Start : 整合Alarm Code
            {
                iRetry=0;
                Task=100;
            }
            else
            {
                Task=220;
                return false;
            }
        case 100:
//            if(SortArmNeedCheckOffset(true, iWhichAuto))                        //還沒寫
//            {
//                Task=200;
//                break;
//            }

            if(ArmSpeed[OutArm].bSuckOnDown==false &&                           //jou 2010-12-01 start : 小IC時,會偶發性置偏
               ArmSpeed[OutArm].dDestroyPauseTime!=0)
            {
                DoSortPlaceToAutoDelay.SetSecAndOn(ArmSpeed[OutArm].dDestroyPauseTime);
                Task=110;
            }
            else
            {
                Task=300;
            }
            break;
        case 110:
            if(DoSortPlaceToAutoDelay.Off()==true)
            {
                Task=300;
            }
            break;
        case 200:
            if(bEnterOffset==false)
            {
                Task=300;
            }
            else
            {
                bEnterOffset=false;
                Task=220;
            }
            break;
        case 220:
            if(MoveSortArmToAutoSafe())
            {
                Task=1;
                if(OutArm2Suck.HasIC()==false)                                   //Steven 20220812 : 避免IC掉光後卡死
                    return true;
            }
            break;
        case 300:
//            if(CosFunction.bManualSteplAutoTeach &&
//               IniConfig.bA56EnableAutoTeachFunciton &&
//               fAutoTeach->IsRun())                                             //Jimmychiu 20211020 : Auto alignment mode
//            {
//                iWhichAuto=iFixRight;
//            }

            if(DoSortArmPlaceToWitchAuto(iSortWhichAuto)==true)                           //Steven 20110216 : 整合AlarmCode
            {
                Task=400;
            }
            break;
        case 400:
            if(CosFunction.bNewAutoTrayDetect==true &&                          //JerryYang 20170822 (Steven) 更改Auto區偵測tray盤時機 避免out arm吸嘴下降時造成誤判
               iSortWhichAuto<iAutoRight)
            {
                if(MOT[iMMAuto[iSortWhichAuto]].Tray.FullIC())                      //JerryYang 20170919 (Steven) Auto擺滿的時候會自動退tray,不要再偵測auto tray
                {
                    ReversionEmptyPoint();
                    Task=1;
                    return true;
                }
                else
                {
                    if(MoveSortArmToAutoSafe())
                    {
                        Task=500;
                    }
                }
            }
            else
            {
                ReversionEmptyPoint();
                Task=1;
                return true;
            }
            break;
        case 500:
            if(DetectAutoTray(iSortWhichAuto, &iRetry)==true)                       //JerryYang 20170822 (Steven) 更改Auto區偵測tray盤時機 避免out arm吸嘴下降時造成誤判
            {
                ReversionEmptyPoint();
                Task=1;
                return true;
            }
    }
    return false;
}
//------------------------------------------------------------------------------
//==============================================================================//RogerYang 20250526 Add for 9046AU for Pick From Shuttle  //還不能用
double dSortArmXPitch_1Step_Sht=0.0;                                                 //1吸嘴之間的X-Pitch
double dSortArmXPitch_MovePitch_Sht=0.0;                                             //張到最開的X-Pitch
int    iSortArmXStep_Sht=1;                                                          //Sort Arm在Tray X一次要跳幾格
int    iSortArmYStep_Sht=1;                                                          //Sort Arm在Tray Y一次要跳幾格
//------------------------------------------------------------------------------
int AutoCalculateSortArmXClosePitch_Sht_2x4()                                   //
{
//    int iStep;
//    int SortArmClose_PitchX=0;
//    int iMaxXPitchPick   =iXpitchMax;
//    int iMinXPitchPick   =iXpitchMin;
//    int XDivision        =4;
//
//
//    if(OutArm2Suck.iPickCol==1)                                                 //1x1 mode直接使用最小X-Pitch
//    {
//        SortArmClose_PitchX      =iXpitchMin;
//        dSortArmXPitch_1Step_Sht     =SortArmClose_PitchX;
//        dSortArmXPitch_MovePitch_Sht =SortArmClose_PitchX;
//        return dSortArmXPitch_MovePitch_Sht;
//    }
//    else
//    {
//        int iStart=XDivision-1;
//        if((XDivision%2)==0)
//        {
//            iStart=(XDivision/2);
//        }
//
//        if(iStart<1)
//            iStart=1;
//
//        if(XDivision==1)                                   //Steven 20240801 : for tray x item=1
//        {
//            SortArmClose_PitchX=iMaxXPitchPick;
//            dSortArmXPitch_1Step_Sht=iMaxXPitchPick;
//            dSortArmXPitch_MovePitch_Sht=iMaxXPitchPick;
//            return dSortArmXPitch_MovePitch_Sht;
//        }
//        else
//        {
//            for(int i=iStart; i>0; i--)
//            {
//                iSortArmXStep_Sht=i;
//                SortArmClose_PitchX=XPitch*iSortArmXStep_Sht;
//                if(SortArmClose_PitchX>=iMinXPitchPick && SortArmClose_PitchX<=iMaxXPitchPick)
//                {
//                    dSortArmXPitch_1Step_Sht     =SortArmClose_PitchX;
//                    dSortArmXPitch_MovePitch_Sht =dSortArmXPitch_1Step_Sht*1.0;
//                    return dSortArmXPitch_MovePitch_Sht;
//                }
//            }
//        }
//    }
//
//
//    iSortArmXStep_Sht=1;
//    if(SortArmClose_PitchX==0)                                                   //Steven 20240205 : 修正有可能Pitch是0
//        SortArmClose_PitchX=iMinXPitchPick;
//    dSortArmXPitch_1Step_Sht     =iMaxXPitchPick;       //Jimmychiu 20241217 : OutArmClose_PitchX->iMaxXPitch4Pick
//    dSortArmXPitch_MovePitch_Sht =dSortArmXPitch_1Step_Sht*1.0;
//    SortArmClose_PitchX=iMaxXPitchPick;
//    return dSortArmXPitch_MovePitch_Sht;
    return 0;
}
//------------------------------------------------------------------------------
void AdjustSortArmClosePitchCondition_Sht(bool &bCanPick2ICAtOnceTime)           //還不能用
{
//    int iResult=0;
//    bCanPick2ICAtOnceTime=false;
//
//    iResult=AutoCalculateSortArmXClosePitch();
//    if(ArmSpeed[OutArm].bVariModeFIX==false && !bSortUseOnebyOne)                   //jou 2010-09-07 當clean out時,out arm排列自動改為One by one排列
//    {
//        if(iResult==-1)
//        {
//            bCanPick2ICAtOnceTime=false;
//        }
//        else
//        {
//            bCanPick2ICAtOnceTime=true;
//        }
//    }
}
//------------------------------------------------------------------------------
bool DoMoveSortArmXYToPlace(bool &bCanPick2ICAtOnceTime, int iRow, int iCol, bool RealMove)   //RogerYang 20250515 Add for 9046AU
{
    int iSortPutTray=0;
    int iXPos=0, iYPos=0;
    int iXVariable=0;
//    int iYVariable =0;
    int iMovePitchX=AutoCalculateSortArmXClosePitch(iSortWhichAuto);                 //Steven 20240501 : 修正超出極限後, 要重新算一次X-Pitch
//    int iMovePitchY=0;
    bool bZFlag[MAX_ARM_Row][MAX_ARM_Col];
    int iZPos[MAX_ARM_Row][MAX_ARM_Col];
    int iOffsetPos=-1;
    bool bNeedDown=true;
    bool bFlag=false;
    ZeroMemory(bZFlag, sizeof(bZFlag));
    ZeroMemory(iZPos, sizeof(iZPos));

    iSortPutTray=iSortWhichAuto;

    bFlag=GetSortArmToUnLoaderPosition(bCanPick2ICAtOnceTime, iSortPutTray, iXPos, iYPos, iRow, iCol, RealMove);

    if(bCanPick2ICAtOnceTime==false)
        iMovePitchX=AutoCalculateSortArmXClosePitch(iSortWhichAuto, bSortArmXOverLimit);  //Steven 20240501 : 修正超出極限後, 要重新算一次X-Pitch

    if(RealMove==false)
        return bFlag;

    if(iXPos<(MOT[MOutSortX].Motor->PSoftLimitN+10) ||                           //jou 2010-02-01 fix 3 吸嘴超過 軟體極限時 , 設成 has null ic
       iXPos>(MOT[MOutSortX].Motor->PSoftLimitP-10))
    {
        if(MOT[iMMAuto[iSortPutTray]].Tray.Data[iCol][iRow]==NULL_IC)            //ChungHung 20111215 嘗試將Fix3放滿
        {
            MOT[iMMAuto[iSortPutTray]].SetTraySingleData(iCol, iRow, HAS_NULL_IC);
            ReserveEmptyPoint(iMMAuto[iSortPutTray], iCol, iRow);
        }
        return false;
    }

    if(IniConfig.bE34InOutArmPitchZOffsetSameOne==true)
    {
        iOffsetPos=SortOfsAuto4;
    }
    else
    {
        iOffsetPos=SortOfsAuto4+(iSortPutTray-3);
    }

    iXVariable=GetSortArmPitchX(iMovePitchX, iOffsetPos);

    if(MOT[iMMAuto[iSortPutTray]].fHasTray==false ||
       MOT[iMMAuto[iSortPutTray]].FullIC())
        bNeedDown=false;
    GetSortArmZDownPos(iSortWhichAuto, bNeedDown, bZFlag, iZPos);

    if(SortArmContinuousMove(iXPos, iYPos, iXVariable, bZFlag, iZPos, true))
    {
        return true;
    }
    return false;
}
//==============================================================================
bool SearchSortShuttleUpDown(bool &bCanPick2ICAtOnceTime, int &iRow, int &iCol)  //RogerYang 20250515 Add for 9046AU        //還不能用
{
//    AdjustSortArmClosePitchCondition_Sht(bCanPick2ICAtOnceTime);
//
//
//    if(bCanPick2ICAtOnceTime==true)
//    {
//        int ct=Find_SortArm_PickerMaxUseCountOnTime(OutArm2Suck.iPickCol, iRow, iCol);
//        if(ct<=1)
//        {
//            bCanPick2ICAtOnceTime=false;
//        }
//        else if(iSortWhichAuto==eAuto6)
//        {
//            if(DoMoveSortArmXYToPlace(bCanPick2ICAtOnceTime, iRow, iCol, false)==true)
//                return true;
//            bCanPick2ICAtOnceTime=false;
//        }
////        else
////        {
////            SearchBinBoxUpDown(bCanPick2ICAtOnceTime, iRow, iCol);              //Steven 20230601 : 重新整理Bin Box
////            return true;
////        }
//    }
//    else
//    {
//        Find_SortArm_Single(OutArm2Suck.iPickCol, iRow, iCol);
//    }
    return true;
}
//==============================================================================      //還不能用
int Find_SortArm_PickerMaxUseCountOnTime_Sht(int iUseSuck, int &iRow, int &iCol)
{
//    int  iTrayCol;
//    bool bBackupSortArmSuckActive[MAX_ARM_Row][MAX_ARM_Col]={{false, false, false, false}, {false, false, false, false}};
//    int iBackupSortArmXPosition[MAX_ARM_Row][MAX_ARM_Col]={{0, 0, 0, 0}, {0, 0, 0, 0}};
//    int iBackupSortArmYPosition;
//    int iCT=0, itStartCol=0, itStartRow=0;
//    int iMaxCT=0, isStartX=0;
//    int Motor;
//    int iBackix=0, iBackiy=0;
//    int ixStart=0, ixEnd, ixStep;
//    int iyStart=0, iyEnd, iyStep;
//    int Direction;
//    static int iRecY=0, iRecWhichAuto;
//
//    bool bSearchLast=(OutArm2Suck.iPickRow==2)?TestIF.bSearchLastMode:false;
//
//
//    //int iSortArmWhichAuto=iSortWhichAuto;
//
//    for(int i=0; i<OutArm2Suck.iPickRow; i++)
//    {
//        for(int j=0; j<OutArm2Suck.iPickCol; j++)
//        {
//            if(OutArm2Suck.Item[i][j] &&
//               OutArm2Suck.iWhichAuto[i][j]==iSortArmWhichAuto)
//            {
//                isStartX=j;
//                i=OutArm2Suck.iMaxRow;
//                j=OutArm2Suck.iMaxCol;
//                break;
//            }
//        }
//    }
//
//    Motor=iMMAuto[iSortWhichAuto];
//    Direction=AutoForm[iSortWhichAuto]->Direction;
//
//
//    if(Direction==0)
//    {
//        ixStart=0;
//        ixEnd=MOT[Motor].Tray.XItem;
//        ixStep=1;
//
//        if(TRAY_ARM_MODE==eUnderCoveyor &&
//           (MOT[Motor].fHasTray==false ||
//            MOT[Motor].FullIC()==true))
//        {
//            iyStart=0;
//            iRecY=0;
//        }
//        else
//        {
//            if(iSortWhichAuto<iAutoCnt)
//            {
//                if(bSearchLast && iCloseSiteState==0)
//                {
//                    if(OutArm2Suck.iPickRow==2)
//                    {
//                        if(MOT[Motor].HasIC()==false)
//                        {
//                            iyStart=0;
//                            iRecY=0;
//                        }
//                        else
//                        {
//                            if(iSortArmPlaceOrder==1 && iRecWhichAuto==iSortWhichAuto)
//                            {
//                                //iyStart=iRecY+(TestIF.iARM_Y_PITCH/AutoForm[iWhichAuto]->YPitch); //ChungHung 20120505 9045WS
//                                if(iyStart>=AutoForm[iSortWhichAuto]->YDivision)
//                                {
//                                    iyStart=0;
//                                    iRecY=0;
//                                }
//
//                                if(MOT[Motor].SearchHasEmpryToPlace(iyStart)==false)
//                                {
//                                    iyStart=0;
//                                    iRecY=0;
//                                }
//                            }
//                            else
//                            {
//                                iyStart=0;
//                            }
//                        }
//                        iyEnd=MOT[Motor].Tray.YItem;
//                        iyStep=1;
//                    }
//                    else
//                    {
//                        iyStart=0;
//                        iyEnd=MOT[Motor].Tray.YItem;
//                        iyStep=1;
//                    }
//                }
//                else
//                {
//                    iyStart=0;
//                    iyEnd=MOT[Motor].Tray.YItem;
//                    iyStep=1;
//                }
//            }
//        }
//    }
//
//
//    if(TRAY_ARM_MODE==eUnderCoveyor &&
//       (MOT[Motor].fHasTray==false ||
//        MOT[Motor].FullIC()==true))
//    {
//        for(int iC=isStartX; iC<OutArm2Suck.iPickCol; iC++)
//        {
//            iTrayCol=iC-isStartX;
//            iTrayCol=itStartCol+iTrayCol*iSortArmXStep;
//                if(OutArm2Suck.Item[iSortArmPlaceOrder][iC]!=NULL_IC &&
//                   OutArm2Suck.iWhichAuto[iSortArmPlaceOrder][iC]==iSortArmWhichAuto &&
//                   iTrayCol<AutoForm[iSortWhichAuto]->XDivision)
//                {
//                    iCT++;
//                    bSortArmSuckActive[iSortArmPlaceOrder][iC]=true;
//                    iSortArmXPosition[iSortArmPlaceOrder][iC]=iTrayCol;
//                    bBackupSortArmSuckActive[iSortArmPlaceOrder][iC]=true;
//                    iBackupSortArmXPosition[iSortArmPlaceOrder][iC]=iTrayCol;
//                    iSortArmYPosition=iRow;
//                }
////            }
//        }
//        iMaxCT=iCT;
//    }
//    else
//    {
//        for(int y=iyStart; y!=iyEnd; y+=iyStep)
//        {
//            if(iMaxCT!=0)
//                break;
//            for(int x=ixStart; x!=ixEnd; x+=ixStep)
//            {
//                if(MOT[Motor].Tray.Data[x][y]==NULL_IC)
//                {
//                    itStartCol=x;
//                    itStartRow=y;
//                    iCol=itStartCol;
//                    iRow=itStartRow;
//                    for(int i=0; i<OutArm2Suck.iMaxRow; i++)
//                        for(int j=0; j<OutArm2Suck.iMaxCol; j++)
//                            bSortArmSuckActive[i][j]=false;
//                    iCT=0;
//
//                    for(int j=isStartX; j<OutArm2Suck.iPickCol; j++)
//                    {
//                        iTrayCol=j-isStartX;
//                        iTrayCol=itStartCol+iTrayCol*iSortArmXStep;
//
//                        if(OutArm2Suck.Item[iSortArmPlaceOrder][j]!=NULL_IC &&
//                           OutArm2Suck.iWhichAuto[iSortArmPlaceOrder][j]==iSortArmWhichAuto &&
//                           iTrayCol<MOT[Motor].Tray.XItem &&
//                           MOT[Motor].Tray.Data[iTrayCol][itStartRow]==NULL_IC)
//                        {
//                            iCT++;
//                            bSortArmSuckActive[iSortArmPlaceOrder][j]=true;
//                            iSortArmXPosition[iSortArmPlaceOrder][j] =iTrayCol;
//                            iSortArmYPosition=iRow;
//                        }
//
//                    }
//
//                    if(iCT>iMaxCT)
//                    {
//                        iMaxCT=iCT;
//
//                        for(int j=0; j<OutArm2Suck.iPickCol; j++)
//                        {
//                            bBackupSortArmSuckActive[iSortArmPlaceOrder][j]=bSortArmSuckActive[iSortArmPlaceOrder][j];
//                            iBackupSortArmXPosition[iSortArmPlaceOrder][j] =iSortArmXPosition[iSortArmPlaceOrder][j];
//                        }
//                        iBackupSortArmYPosition=iSortArmYPosition;
//                        iBackix=iCol;
//                        iBackiy=iRow;
//
//                    }
//                }
//            }
//        }
//    }
//
//    for(int i=0; i<OutArm2Suck.iPickCol; i++)
//    {
//        bSortArmSuckActive[iSortArmPlaceOrder][i]=bBackupSortArmSuckActive[iSortArmPlaceOrder][i];
//        iSortArmXPosition[iSortArmPlaceOrder][i] =iBackupSortArmXPosition[iSortArmPlaceOrder][i];
//    }
//    iSortArmYPosition=iBackupSortArmYPosition;
//    iCol=iBackix;
//    iRow=iBackiy;
//
//    if(bSearchLast && iCloseSiteState==0)
//    {
//        if(OutArm2Suck.iPickRow==2)
//        {
//            if(iSortArmPlaceOrder==0)
//            {
//                iRecY=iRow;
//                iRecWhichAuto=iSortWhichAuto;
//            }
//        }
//    }
//
//    return iMaxCT;
    return 0;
}
//------------------------------------------------------------------------------
void CheckSortArmDestroyActive()                                                 //確認device確實destroy完成
{
    int iPos[MAX_ARM_Row][MAX_ARM_Col];
    static int iTask[MAX_ARM_Row][MAX_ARM_Col]={{0, 0, 0, 0}, {0, 0, 0, 0}};
    static int iXPos=0, iYPos=0;                                                //kevin 20120514 開吹氣時xy 馬達移動就關吹氣
    int iXpos;
    int iYpos;
    int iMotZ;
    int iSortWhichAuto, iCol, iRow, iMotor;
    AnsiString Msg;

    for(int i=0; i<OutArm2Suck.iPickRow; i++)
    {
        for(int j=0; j<OutArm2Suck.iPickCol; j++)
        {
            if(bSortArmCheckDestroyACT[i][j]==false)
            {
                iTask[i][j]=1;
                continue;
            }

            switch(iTask[i][j])
            {
                case 1:
                    bCheckSortArmDestroyActiveFinish[i][j]=false;                //ChungHung 201112296 add Pick 前先確認CheckInArmDestroyActive 已完成
                    iMotZ=OutArm2Suck.Suck[i][j].iMotNo;
                    iPos[i][j]=MOT[iMotZ].ReadPos();
                    if(iXPos!=MOT[MOutSortX].ReadPos() &&
                       iYPos!=MOT[MOutSortY].ReadPos())                          //kevin 20120514 Xy 移動關吹氣
                    {
                        if(OutArm2Suck.Item[i][j]==NULL_IC)                      //Steven 20120507 : 要持續吹氣
                            OutArm2Suck.Suck[i][j].OffDestroy();
                    }

                    if(iPos[i][j]>-500)
                    {
                        iTask[i][j]=200;
                        OutArm2Suck.Suck[i][j].OffDestroy();
                    }
                    else
                    {
                        if(OutArm2Suck.Item[i][j]==NULL_IC)                      //Steven 20120507 : 要持續吹氣
                        {
                            if(ArmSpeed[OutArm].bSuckOnDown)
                                OutArm2Suck.Suck[i][j].OnDestroy();
                            iXPos=MOT[MOutSortX].ReadPos();
                            iYPos=MOT[MOutSortY].ReadPos();
                        }
                    }
                    break;
                case 200:
                    OutArm2Suck.Suck[i][j].OnSuck();
                    CheckSortArmDestroyActiveDelay[i][j].SetSecAndOn(ArmSpeed[OutArm].dDestroyCheckTime); //2013-08-01    Dell    modify  公司統一回吸功能
                    if(ArmSpeed[OutArm].bSuckOnDown==false ||
                       ArmSpeed[OutArm].bDestroyPauseCheck)                     //2013-08-01    Dell    modify  公司統一回吸功能
                    {
                        bCheckSortArmDestroyActive[i][j]=true;                   //Steven 20120507 : 要偵測完才可以動，會影響UPH
                    }

                    iTask[i][j]=300;
                    break;
                case 300:
                    if(CheckSortArmDestroyActiveDelay[i][j].Off())
                    {
                        bResetSortArm=false;
                        if(OutArm2Suck.Suck[i][j].GetStatus())
                        {
                            iXpos=MOT[MOutSortX].ReadPos();
                            iYpos=MOT[MOutSortY].ReadPos();
                            ShowErrorMessage("WAR0258", 0, MOutSortX, false, OutArm2Suck.Suck[i][j].sName);       //破壞錯誤, 請將IC取下

                            iSortWhichAuto=SortArmPordRec[i][j].GetWhcihAuto();
                            iCol=SortArmPordRec[i][j].GetAutoX();
                            iRow=SortArmPordRec[i][j].GetAutoY();
                            if(iSortWhichAuto>=0)                                   //Steven 20210316 : 掉料的時候, 清除Unloader tray上的資料
                            {
                                iMotor=iMMAuto[iSortWhichAuto];

                                if(IniConfig.bE65_ClearTrayDataWhenOutArmDestoryErr)
                                {
                                    MOT[iMotor].SetTrayBinData(iCol, iRow, NULL_IC, AnsiString(""));

                                    if(iSortWhichAuto<0)
                                        Msg.sprintf("(Sort)Clear unloader data error");
                                    else if(iSortWhichAuto<iAutoCnt)
                                        Msg.sprintf("(Sort)Clear data of Auto%d, X:%d, Y%d", iSortWhichAuto+1, iCol, iRow);

                                    MyDBIProcess("Process", Msg);
                                }
                                SortArmPordRec[i][j].bUse=true;
                                SortArmPordRec[i][j].AddErrorRecord("WAR0258", true, iXpos, iYpos);
                            }
                            bCheckSortArmDestroyActive[i][j]=true;
                            iTask[i][j]=200;                                    //Steven 20120507 : 有出錯就繼續吸吸看，看IC有沒有被拿掉。
                        }
                        else
                        {
                            bCheckSortArmDestroyActiveFinish[i][j]=true;         //ChungHung 201112296 add Pick 前先確認CheckInArmDestroyActive 已完成
                            OutArm2Suck.Suck[i][j].Normal();
                            SortArmPordRec[i][j].InitialRecord();                //Steven 20210316 : 掉料的時候, 清除Unloader tray上的資料
                            bSortArmCheckDestroyACT[i][j]=false;
                            bCheckSortArmDestroyActive[i][j]=false;
                            iTask[i][j]=1;                                      //ChungHung 20111229 add 防止跳出後bInArmCheckDestroyACT馬上被設為true 會照成有一次無效
                        }
                    }
                    break;
                default :
                    break;
            }
        }
    }
}
//------------------------------------------------------------------------------
bool SearchUnLoadTrayUpDown_Sort(bool &bCanPick2ICAtOnceTime, int &iRow, int &iCol)  //RogerYang 20250515 Add for 9046AU
{
    AdjustSortArmClosePitchCondition(iSortWhichAuto, bCanPick2ICAtOnceTime);

    if(bSortArmXOverLimit)                                                       //Steven 20171206 (Wei) : In ATK, 修正Out Arm同時放料但是超出軟體極限縮Pitch導致疊料的問題
        bCanPick2ICAtOnceTime=false;

    if(bCanPick2ICAtOnceTime==true)
    {
        int ct=Find_SortArm_PickerMaxUseCountOnTime(OutArm2Suck.iPickCol, iRow, iCol);
        if(ct<=1)
        {
            bCanPick2ICAtOnceTime=false;
        }
        else if(iSortWhichAuto==eAuto6)
        {
            if(DoMoveSortArmXYToPlace(bCanPick2ICAtOnceTime, iRow, iCol, false)==true)
                return true;
            bCanPick2ICAtOnceTime=false;
        }
//        else
//        {
//            SearchBinBoxUpDown(bCanPick2ICAtOnceTime, iRow, iCol);              //Steven 20230601 : 重新整理Bin Box
//            return true;
//        }
    }
    else
    {
        Find_SortArm_Single(OutArm2Suck.iPickCol, iRow, iCol);
    }
    return true;
}
//==============================================================================
bool SetSortArm()                                                               //RogerYang 20250515 Add for 9046AU
{
    int iCol=0, iRow=0;
    bool bCanPick2ICAtOnceTime;

    if(SearchUnLoadTrayUpDown_Sort(bCanPick2ICAtOnceTime, iRow, iCol)==false)
        return false;

    bSortOverTray=false;
    if(iCol<0 || iRow<0)                                                            //jou 2012-09-04 修正極限無法到達時,回填Null_IC會造成找不到地方放,會放在 x=-1,y=-1的位置上
    {
        bSortOverTray=true;
        return false;
    }
    return DoMoveSortArmXYToPlace(bCanPick2ICAtOnceTime, iRow, iCol, true);
}
//------------------------------------------------------------------------------
bool MoveSortArmToShuttleIncludeZ(bool bZDown)
{
    bool rlt=false;
    switch(iInArmType)
    {
        case e9045_2x4_8:
            rlt=MoveSortArmToShuttleIncludeZ_2x4_8();
            break;
        case e9045_2x8_8:
            rlt=MoveSortArmToShuttleIncludeZ_2x8_8();
            break;
    }
    return rlt;
}
//------------------------------------------------------------------------------
bool DoSortPickFromShuttle()
{
    bool rlt=false;
     switch(iInArmType)
    {
        case e9045_2x4_8:
            rlt=DoSortPickFromShuttle_2x4_8();
            break;
        case e9045_2x8_8:

            rlt=DoSortPickFromShuttle_2x8_8();
            break;
    }
    return rlt;
}
//==============================================================================
//  e9045_2x4_8
//==============================================================================
//個位數為Y座標, 2=標準, 0=A排, 1=B排
//百位數以上為X座標, 分配方式如下, 千位數為Offset左右之分
//                    0                     =標準
//         100        |         110         =左右分兩次
//    200       201   |    210       211    =左右分四次
int GetNowSortShuttleMode_2x4_8()                                               //RogerYang 20250513 Add for 9046AU
{
    if(OutArm2Suck.iXStep==1 && OutArm2Suck.iYStep==1)
    {
        return 2;
    }
//    else if(ptrOutSHT->UseSiteNoIC())
//    {
//        if(OutArm2Suck.iXStep==1)
//        {
//            return 0;
//        }
//        else if(OutArm2Suck.iXStep==2)
//        {
//            if(OutArm2Suck.iYStep==1)
//                return 10002;                                                   //左邊八顆
//            else
//                return 10000;                                                   //左上四顆
//        }
//        else //if(OutArmSuck.iXStep==4)
//        {
//            if(OutArm2Suck.iYStep==1)
//                return 20002;                                                   //左邊四顆
//            else
//                return 20000;                                                   //左上兩顆
//        }
//    }
    else
    {
        if(OutArm2Suck.iXStep==1)
        {
//            for(int j=0; j<OutArm2Suck.iPickCol; j++)
//            {
//                if(ptrOutSHT->Item[0][j]>=HAS_IC)
//                    return 0;                                                   //A排四顆
//            }
            return 1;                                                           //B排四顆
        }
        else if(OutArm2Suck.iXStep==2)
        {
//            if(OutArm2Suck.iYStep==1)
//            {
//                if(ptrOutSHT->Item[0][0]>=HAS_IC || ptrOutSHT->Item[0][1]>=HAS_IC ||
//                   ptrOutSHT->Item[1][0]>=HAS_IC || ptrOutSHT->Item[1][1]>=HAS_IC)
//                    return 10002;                                               //左邊四顆
//                else //if(ptrOutSHT->[0][2]>=HAS_IC || ptrOutSHT->[0][3]>=HAS_IC ||
//                     //   ptrOutSHT->[1][2]>=HAS_IC || ptrOutSHT->[1][3]>=HAS_IC)
//                    return 11002;                                               //右邊四顆
//            }
//            else  //if(OutArm2Suck.iXStep==2 && OutArm2Suck.iYStep==2)
            {
                if(OutSht3Kit.Item[0][0]>=HAS_IC || OutSht3Kit.Item[0][1]>=HAS_IC)
                    return 10000;                                               //左上兩顆
                else if(OutSht3Kit.Item[1][0]>=HAS_IC || OutSht3Kit.Item[1][1]>=HAS_IC)
                    return 10001;                                               //左下兩顆
                else if(OutSht3Kit.Item[0][2]>=HAS_IC || OutSht3Kit.Item[0][3]>=HAS_IC)
                    return 11000;                                               //右上兩顆
                else //if(ptrOutSHT->Item[1][2]>=HAS_IC || ptrOutSHT->Item[1][3]>=HAS_IC)
                    return 11001;                                               //右下兩顆
            }
        }
        else //if(OutArmSuck.iXStep==4)
        {
//            if(OutArm2Suck.iYStep==1)
//            {
//                if(ptrOutSHT->Item[0][0]>=HAS_IC ||
//                   ptrOutSHT->Item[1][0]>=HAS_IC)
//                    return 20002;
//                else if(ptrOutSHT->Item[0][1]>=HAS_IC ||
//                        ptrOutSHT->Item[1][1]>=HAS_IC)
//                    return 20102;
//                else if(ptrOutSHT->Item[0][2]>=HAS_IC ||
//                        ptrOutSHT->Item[1][2]>=HAS_IC)
//                    return 21002;
//                else //if(ptrOutSHT->Item[0][3]>=HAS_IC ||
//                     //   ptrOutSHT->Item[1][3]>=HAS_IC)
//                    return 21102;
//            }
//            else
//            {
//                if(ptrOutSHT->Item[0][0]>=HAS_IC)
//                    return 20000;
//                else if(ptrOutSHT->Item[1][0]>=HAS_IC)
//                    return 20001;
//                else if(ptrOutSHT->Item[0][1]>=HAS_IC)
//                    return 20100;
//                else if(ptrOutSHT->Item[1][1]>=HAS_IC)
//                    return 20101;
//                else if(ptrOutSHT->Item[0][2]>=HAS_IC)
//                    return 21000;
//                else if(ptrOutSHT->Item[1][2]>=HAS_IC)
//                    return 21001;
//                else if(ptrOutSHT->Item[0][3]>=HAS_IC)
//                    return 21100;
//                else //if(ptrOutSHT->Item[1][3]>=HAS_IC)
//                    return 21101;
//            }
            return 2;
        }
    }
}
//------------------------------------------------------------------------------
bool MoveSortArmToShuttleIncludeZ_2x4_8(bool bZDown)                         //RogerYang 20250517 Add for 9046AU
{
    bool bZFlag[MAX_ARM_Row][MAX_ARM_Col];
    int iZPos[MAX_ARM_Row][MAX_ARM_Col];
    int iXVariable;
    int iXPos               =iSortArmShtXCenterPos;
    int iYPos               =iSortArmShtYCenterPos;
//    int iMode               =GetNowSortShuttleMode_2x4_8();
//    int iModeRow            =iMode%100;
//    int iModeCol            =iMode/100;
    int iMovePitchX         =iXpitchMax;
//    int iMovePitchY         =0;
    int iOffsetPos          =GetSortArmToSortShuttleOffset();
//    int iYVariable          =0;
    ZeroMemory(bZFlag,      sizeof(bZFlag));
    ZeroMemory(iZPos,       sizeof(iZPos));

    iXVariable=GetSortArmPitchX(iMovePitchX, iOffsetPos);

//    if(OutArmSuck.iXStep==1)
//        iMovePitchX=TestIF.dSiteXPitch*3;
//    else if(OutArmSuck.iXStep==2)
//        iMovePitchX=TestIF.dSiteXPitch/2*3;
//    else
//        iMovePitchX=iXpitchMaxX3;

    //if(CheckOutArmXYPitch_2x4_8(&iXPos, &iYPos, iSht, iMovePitchX, iMovePitchY)==false && bZDown==true)
    if(CheckSortArmXYPitch_2x4_8(&iXPos, &iYPos, iMovePitchX)==false && bZDown==true)
        return false;

    GetSortShuttleStatus_Pick(iZPos, bZDown, bZFlag);
    if(IsCheckSortArmDestroyActiveFinish(iXPos, iYPos)==false)
        return false;

    //DoOutArmSuckPreOn(iSht, 0, iXPos, iYPos, bZDown);
    DoSortArmSuckPreOn(0, iXPos, iYPos, bZDown);
    if(SortArmContinuousMove(iXPos, iYPos, iXVariable, bZFlag, iZPos, bZDown))
        return true;

    return false;
}
//---------------------------------------------------------------------------
//==============================================================================
//  e9045_2x8_8
//==============================================================================
//個位數為Y座標, 2=標準, 0=A排, 1=B排
//百位數以上為X座標, 分配方式如下, 千位數為Offset左右之分
//                    0                     =標準
//         100        |         110         =左右分兩次
//    200       201   |    210       211    =左右分四次
int GetNowSortShuttleMode_2x8_8()                                               //RogerYang 20250710 Add for 9046AU
{
    int iKit=0;
    if(OutSht3Kit.LeftSideNoIC(OutArm2Suck.iShtKitStep))
        iKit=OutArm2Suck.iShtKitStep;

    if(iKit==0)     //左半8顆
    {
        if(OutSht3Kit.Item[0][0]>=HAS_IC || OutSht3Kit.Item[0][1]>=HAS_IC)
            return 20000;
        else if(OutSht3Kit.Item[1][0]>=HAS_IC || OutSht3Kit.Item[1][1]>=HAS_IC)
            return 20001;
        else if(OutSht3Kit.Item[0][2]>=HAS_IC || OutSht3Kit.Item[0][3]>=HAS_IC)
            return 20100;
        else //(OutSht3Kit.Item[1][2]>=HAS_IC || OutSht3Kit.Item[1][3]>=HAS_IC)
            return 20101;
    }
    else           //右半8顆
    {
        if(OutSht3Kit.Item[0][0+iKit]>=HAS_IC || OutSht3Kit.Item[0][1+iKit]>=HAS_IC)
            return 21000;
        else if(OutSht3Kit.Item[1][0+iKit]>=HAS_IC || OutSht3Kit.Item[1][1+iKit]>=HAS_IC)
            return 21001;
        else if(OutSht3Kit.Item[0][2+iKit]>=HAS_IC || OutSht3Kit.Item[0][3+iKit]>=HAS_IC)
            return 21100;
        else //(OutSht3Kit.Item[1][2+iKit]>=HAS_IC || OutSht3Kit.Item[1][3+iKit]>=HAS_IC)
            return 21101;
    }
}
//------------------------------------------------------------------------------
bool SortArmZNeedDown_2x8_8()
{
    if(InitialSortArmNeedSuck()==false)
        return false;

    int iMode   =GetNowSortShuttleMode_2x8_8();
    int iModeRow=iMode%100;
    int iModeCol=iMode/100;
    int iShtJ   =0;
    int iRow    =0;
    bool bPlace=false;

    iLocModeRow=iModeRow;
    iLocModeCol=iModeCol;   //這是要讓SortA吸嘴知道現在吸的是Shuttle的哪一區
//
    for(int i=0; i<OutArm2Suck.iPickRow; i++)
    {
        for(int j=0; j<OutArm2Suck.iPickCol; j++)
        {
            iShtJ=GetShuttleCol(i, j);
            iRow=iModeRow==0?0:1;
            if(iModeCol==200 && (j==0 || j==1))
            {
                SetSortArmNeedSuck(i, j, iRow, iShtJ, bPlace);
            }
            else if(iModeCol==201 && (j==0 || j==1))
            {
                SetSortArmNeedSuck(i, j, iRow, iShtJ+2, bPlace);
            }
            else if(iModeCol==210 && (j==0 || j==1))
            {
                SetSortArmNeedSuck(i, j, iRow, iShtJ+4, bPlace);
            }
            else if(iModeCol==211 && (j==0 || j==1))
            {
                SetSortArmNeedSuck(i, j, iRow, iShtJ+6, bPlace);
            }
        }
    }
    return true;
}
//------------------------------------------------------------------------------
bool CheckSortArmXYPitch_2x8_8(int *iX, int *iY, int iMovePitchX)
{
    int iMode       =GetNowSortShuttleMode_2x8_8();
    int iModeRow    =iMode%100;
    int iModeCol    =iMode/100;
    int iOffsetPos  =GetSortArmToSortShuttleOffset();
    double dMovePitchX;

    //基準軸在kit中心點的位置
    *iX+=Prod.XSortArm_SortShuttle_Pick[iSortArmYBase][iSortArmXBase];
    *iY+=Prod.YSortArm_SortShuttle_Pick[iSortArmYBase][iSortArmXBase];

    if(iModeRow==0)                                                             //Row A
    {
        *iY=*iY+TestIF.dSiteYPitch/2;
    }
    else if(iModeRow==1)                                                        //Row B
    {
        *iY=*iY-TestIF.dSiteYPitch/2;
    }
    else                                                                        //Both
    {
    }

    //基準為第二隻吸嘴
    dMovePitchX=double(iMovePitchX);                                            //40mm

    switch(iModeCol)
    {
        case 200:
            *iX=*iX+(dMovePitchX*1.0-TestIF.dSiteXPitch*3.5);
            break;
        case 201:
            *iX=*iX+(dMovePitchX*1.0-TestIF.dSiteXPitch*1.5);
            break;
        case 210:
            *iX=*iX+(dMovePitchX*1.0+TestIF.dSiteXPitch*0.5);
            break;
        case 211:
            *iX=*iX+(dMovePitchX*1.0+TestIF.dSiteXPitch*2.5);
            break;
    }
    *iY=*iY+GetSortArmXToShuttleOffset(iOffsetPos);
    *iX=*iX+GetSortArmYToShuttleOffset(iOffsetPos);
    //TransferSortShuttleRatio(iSht, iX, iY, iOutArmYBase, iOutArmXBase);       //暫不考慮
    if(SortArmZNeedDown_2x8_8()==false)
        return false;
    return true;
}
//------------------------------------------------------------------------------
bool MoveSortArmToShuttleIncludeZ_2x8_8(bool bZDown)                            //RogerYang 20250710 Add for 9046AU
{
    bool bZFlag[MAX_ARM_Row][MAX_ARM_Col];
    int iZPos[MAX_ARM_Row][MAX_ARM_Col];
    int iXVariable;
    int iXPos               =iSortArmShtXCenterPos;
    int iYPos               =iSortArmShtYCenterPos;
    //int iMode               =GetNowSortShuttleMode_2x8_8();
    //int iModeRow            =iMode%100;
    //int iModeCol            =iMode/100;
    int iMovePitchX         =iXpitchMax;
//    int iMovePitchY         =0;
    int iOffsetPos          =GetSortArmToSortShuttleOffset();
//    int iYVariable          =0;
    ZeroMemory(bZFlag,      sizeof(bZFlag));
    ZeroMemory(iZPos,       sizeof(iZPos));

    iXVariable=GetSortArmPitchX(iMovePitchX, iOffsetPos);

//    if(OutArmSuck.iXStep==1)
//        iMovePitchX=TestIF.dSiteXPitch*3;
//    else if(OutArmSuck.iXStep==2)
//        iMovePitchX=TestIF.dSiteXPitch/2*3;
//    else
//        iMovePitchX=iXpitchMaxX3;

    if(CheckSortArmXYPitch_2x8_8(&iXPos, &iYPos, iMovePitchX)==false && bZDown==true)
        return false;

    GetSortShuttleStatus_Pick(iZPos, bZDown, bZFlag);
    if(IsCheckSortArmDestroyActiveFinish(iXPos, iYPos)==false)
        return false;

    //DoOutArmSuckPreOn(iSht, 0, iXPos, iYPos, bZDown);
    DoSortArmSuckPreOn(0, iXPos, iYPos, bZDown);
    if(SortArmContinuousMove(iXPos, iYPos, iXVariable, bZFlag, iZPos, bZDown))
        return true;

    return false;
}
//---------------------------------------------------------------------------
bool DoSortPickFromShuttle_2x8_8()
{
    int ret;
    int &Task=iPickFromSortShtTask;
    static int iWitchErrBin=0;
    AnsiString ErrPart="";                                                      //Steven 20110216 : 合併Alarm
    bool bHasDuplicateErr=false;                                                //Steven 20110216 : 合併Alarm
    bool bHasErr=false;                                                         //Steven 20110216 : 合併Alarm
    bool flag=false;
    int iSuckRow, iSuckCol, iShtRow, iShtCol;
    //int iPickKit=(iSortArmiWhichKit==0)?0:OutArm2Suck.iPickKitStep;
    int iPickKitRow=iLocModeRow==0?0:1;         //由Model決定
    int iPickKitCol=(iLocModeCol==200||iLocModeCol==201)?                       //目前只寫一般模式200 201 210 211
        (iLocModeCol%200)*2:
        (iLocModeCol%210)*2+OutArm2Suck.iPickKitStep;

    switch(Task)
    {
        case 1:
            if(ArmSpeed[OutArm].dWaitOnSH!=0)                                   //kevin 20210525 out arm 起動在shuttle 上等待 時間
            {
                if(OutArm2Suck.NoIC())                                           //kevin 20210615 第一次 吸取IC 需等待
                {
                    if(MoveSortArmToShuttleIncludeZ_2x8_8(false))      //移到 Shuttle 上等待
                    {
                        PickFromSortSht();                                     //kevin 20210612 add int iwhichKit=0
                        Task=2;
                    }
                }
                else
                {
                    Task=10;
                }
            }
            else
            {
                Task=10;
            }
            break;
        case 2:
            if(bSortShtwaitPick())                                               //kevin 20210602 Out arm sht on wait
                Task=10;
            break;
        case 10:
            iWitchErrBin=0;
            Task=200;
        case 200:
            if(MoveSortArmToShuttleIncludeZ_2x8_8(true))
            {
//                if(SortArmNeedCheckOffset(false))      //還沒寫
//                {
//                    Task=500;
//                    break;
//                }
                OutArm2Suck.ResetAll();                                        //Steven 20160323 : 避免未開啟真空
                Task=iOUTARM_SUCK;
            }
            break;
        case 500:
            if(bEnterOffset)
            {
                bEnterOffset=false;
                bSortArmManualStepPress=true;
                Task=1;
                return false;
            }
            else
            {
                Task=iOUTARM_SUCK;
                OutArm2Suck.ResetAll();                                          //Steven 20160323 : 避免未開啟真空
            }
            break;
        case iOUTARM_SUCK:
            flag=true;
            bHasErr=false;
            for(int i=0; i<OutArm2Suck.iPickRow; i++)
            {
                for(int j=0; j<OutArm2Suck.iPickCol; j++)
                {
                    iSuckRow  =i;
                    iSuckCol  =j;
                    iShtRow   =i+iPickKitRow;
                    iShtCol   =j+iPickKitCol;
                    if(OutSht3Kit.Item[iShtRow][iShtCol] &&
                       OutArm2Suck.Suck[iSuckRow][iSuckCol].GetNeedSuckStatus())//SetSortArmNeedSuck設定要不要吸
                    {
                        if(OutArm2Suck.Suck[iSuckRow][iSuckCol].Suck())
                        {
                            //模擬ic不見了
                            #ifdef SOFT_SIMULTE
                            if(fMain->chkInPickLoadError->Checked==true)
                            {
                                if(j==1)
                                {
                                    OutArm2Suck.Suck[iSuckRow][iSuckCol].Error=true;
                                    continue;
                                }
                            }
                            #endif

                            bHasErr=SwapShuttleDataToSortArm(iShtRow, iShtCol, iSuckRow, iSuckCol, &iWitchErrBin);   //Steven 20170428 : 統一Out Arm從Shuttle吸料後的資料交換
                            bSortSuckShtDupErr[iSuckRow][iSuckCol]=false;
                        }
                        else if(OutArm2Suck.Suck[iSuckRow][iSuckCol].Error==false)
                        {
                            flag=false;
                        }
                    }
                }
            }

            if(bHasErr)
            {
                ShowOutputShuttleDataMiss();  //未針對sort寫
            }

            if(flag==false)
                break;

            for(int i=0; i<OutArm2Suck.iPickRow; i++)
            {
                for(int j=0; j<OutArm2Suck.iPickCol; j++)
                {
                    if(OutArm2Suck.Suck[i][j].Error)
                    {
                        iSortShtRetryCount++;
                        Task=2000;
                        return false;
                    }
                }
            }

            for(int i=0; i<OutArm2Suck.iPickRow; i++)
            {
                for(int j=0; j<OutArm2Suck.iPickCol; j++)
                    if(OutArm2Suck.Item[i][j]==NULL_IC && OutArm2Suck.Suck[i][j].GetNeedSuckStatus())
                        return false;
            }

            iSortShtRetryCount=0;
            Task=1;
            return true;
        case 2000:
            if(MoveSortArmToAutoSafe())
            {
                bHasErr=false;
                ErrPart=" ";
                bHasDuplicateErr=false;                                         //Steven 20110216 Start : 整合Alarm
                for(int i=0; i<OutArm2Suck.iPickRow; i++)
                {
                    for(int j=0; j<OutArm2Suck.iPickCol; j++)
                    {
                        if(bSortSuckShtDupErr[i][j])
                            bHasDuplicateErr=true;
                        if(OutArm2Suck.Suck[i][j].Error)
                        {
                            ErrPart+=OutArm2Suck.Suck[i][j].sName;
                            bHasErr=true;
                        }
                        else
                        {
                            bSortSuckShtDupErr[i][j]=false;
                        }
                    }
                }

                if(bHasErr)
                {
                    if(iSortShtRetryCount>ArmSpeed[OutArm].iRetryCT)
                    {
                        if(!MoveOutArmXY_ToShuttleAlarmArea())
                            return false;

                        ret=OutArmPickShuttleAlarm(1, bHasDuplicateErr, ErrPart);   //暫時隨便呼叫一個alarm

                        for(int i=0; i<OutArm2Suck.iPickRow; i++)
                        {
                            for(int j=0; j<OutArm2Suck.iPickCol; j++)
                            {
                                if(OutArm2Suck.Suck[i][j].Error)
                                {
                                    bSortSuckShtDupErr[i][j]=true;
                                }
                                else
                                {
                                    bSortSuckShtDupErr[i][j]=false;
                                }
                            }
                        }
                    }
                    else
                    {
                        ret=K_RETRY;
                    }

                    if(ret==K_SKIP)
                    {
                        for(int i=0; i<OutArm2Suck.iPickRow; i++)
                        {
                            for(int j=0; j<OutArm2Suck.iPickCol; j++)
                            {
                                if(OutArm2Suck.Suck[i][j].Error)
                                {
                                    iShtCol=GetShuttleCol(i, j);
                                    PorcessJAM0201SortArmPickUpErrorSkip(i, iShtCol, i, j);
                                }
                                bSortSuckShtDupErr[i][j]=false;
                            }
                        }

//                        if(USE_Fix_AI_CCD==true &&                              //KaiChen 20190124 ：矽格湖口 AI CCD Function
//                           TestIF_File.bEnableFix2BGAAICCD==true)
//                        {
//                            fFixAICCD->OutArmCycleCounterUpdate();
//                        }

                        iSortShtRetryCount=0;
                        ZeroMemory(bSortSuckShtDupErr, sizeof(bSortSuckShtDupErr));
                        Task=1;
                        return true;
                    }
                    else                                                        //Steven 20190313 : Fixed for out arm pick error need home
                    {
                        if(iSortShtRetryCount>ArmSpeed[OutArm].iRetryCT)
                        {
                            iSortShtRetryCount=0;
                            if(IniConfig.bInOutArmCanPushHome)                  //ChungHung HT9045 2011/12/13 //Input pickup device error時,按"retry"鍵,機台都會自動home
                            {
                                if(ret==K_HOME)
                                    Task=2200;
                                else
                                    Task=1;
                            }
                            else
                            {
                                if(ret==K_RETRY)
                                    Task=2200;
                                else
                                    Task=1;
                            }
                            return false;
                        }
                    }
                }
                else
                {
                    ZeroMemory(bSortSuckShtDupErr, sizeof(bSortSuckShtDupErr));
                }
                Task=1;
            }
            break;
        case 2200:
            SetSortArmHome();
            iSortShtRetryCount=0;
            Task=1;
            break;
    }
    return false;
}
//==============================================================================
