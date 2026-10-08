// =============================================================================
//  aRotateKIT_In.cpp  --  IN-side rotate-station (RotateKit) motion engine
//
//  Faithful translation of golden RotateKit/aRotateKIT_In.cpp (3907 lines, BCB6,
//  Big5/cp950).
//  Translator: AI(W906-ROTKIT-IN) 20260807
//  Translation wave: W906-ROTKIT-IN (pure-translation completion campaign).  The
//  big IN half of the RotateKit family; it sits directly on top of
//  RotateKit/aRotateKIT.{h,cpp} (landed hours earlier by AI(W906-ROTKIT)) and is
//  the twin of the still-untranslated RotateKit/aRotateKIT_Out.cpp.
//
//  ROLE: owns everything the IN rotate station does between "the in-arm holds
//  ICs" and "the in-arm holds the same ICs, rotated".
//    * TWO switch(Task) cursor state machines over the SHARED cursor
//      `iInArmRotateKit` (golden :29, defined here, extern'd by aRotateKIT.h:67):
//        - DoInArmRotateKIT_Cylinder()  (golden :201)  pneumatic +/-90 kit
//        - M_DoInArmRotateKIT_Motor()   (golden :1666) independent-motor kit
//      plus the small third cursor M_DoInArmRotate_InOutArm() (golden :1369) for
//      the shared In/Out single-motor rotator, and the shake cursor
//      M_DoShakeInArmRotateKIT() (golden :3279) over `iShakeInArmRotateKit`.
//    * The dispatcher DoInArmRotateKIT() (golden :691) that picks between them
//      from iRotate_Type / tRotate.ActiveRotate.
//    * Arm XY / Z geometry for the rotate station (MoveInArmXY_ToRotateKIT,
//      MoveInArmZ_ToRotateKIT_Place/_Pick and their M_ motor-version twins,
//      CalcPosition_InArm, AutoCalculateInArmRotatePitch_Y).
//    * The nozzle<->kit cursor helpers GotInRotateKitForSingle /
//      GotInRotateKitForAxxG / GotInRotateKitForAxEx_2RotMot (golden :1420/:1532/
//      :1583).
//    * The rotate movers DoRotate_In / M1_DoInRotateMove / M1_DoInRotateMove1Mot /
//      M2_ / M4_ / M8_DoInRotateMove and MoveInRotateToDegreeAtSameTime.
//    * The IN rotate sensor + angle predicates CheckInRotateSensor /
//      CheckInRotateAnglePostion and the record writer RecordRotateAngle.
//  28 functions, 9 file-scope globals.  All free functions (no class).
//
//  ---------------------------------------------------------------------------
//  WAVE SCOPE -- ACTIVE vs satisfied-by-substrate vs GATED
//  ---------------------------------------------------------------------------
//  ACTIVE (faithful, verbatim): ALL 28 golden functions, whole file, every
//  switch/case label and every fall-through.  Nothing was summarised, nothing
//  was dropped, no function body is a stub.
//
//  SATISFIED-BY-SUBSTRATE (already real in this tree -- verified by grep over
//  every ported .cpp/.h outside build*/ before writing a single line):
//    RotateKit/aRotateKIT.{h,cpp}  -- i2x2Suck, iRotateKIT_Start|Pitch_X|Y_H,
//        RotateKIT_FormSet, SetMotorResolution, InitSuckState, CheckRotateSensor
//    aHotPlateSubstrate.{h,cpp}    -- TMyKitSuck InArmSuck/OutArmSuck (Item/Suck/
//        PordRec/CopyToTray/CopyFromTray/SetItemData/ResetAll/FindNoIC/iMaxRow/
//        iMaxCol/iPickRow/iPickCol), tRotate, bInArmCheckDestroyACT,
//        MoveInArmZToPlateSafe, SetInArmHome, GetInArmPitchX|Y_9045,
//        bUseAxxGPicker, iCloseSiteModeFor1x4
//    Motor/mymotor.{h,cpp}         -- MOT[] TTrayMotor (Tray.Data/iCurrRotAng/
//        iNeedRotAng/PordRec/HasIC/HasRealIC/FullIC/SetTray/SetTraySingleData/
//        SetHasNullIcToNullIc), MotorMove, MotorMove2SpeedForPicker, ReadPos,
//        GetRotatorBacklash, InArmContinuousMove_9045
//    csystem.h  -- CheckInArmSuckICFallDownToHasNullIC, CheckInArmDestroyICFail,
//        InitDoArmZHome
//    acatchtray_shims.{h,cpp} -- MoveInArm2XYToWait
//    cmydef / cprod / cpublic / Config / MachineType / canary_support /
//    FormsFacade / forms/fRotate.h / forms/fAOI.h -- the scalar+form universe.
//
//  ---------------------------------------------------------------------------
//  GATE REGISTER -- TEN numbered gates, every one re-cited at its call site.
//  Every one was checked against the WHOLE port tree first (grep over every
//  .cpp/.h outside build*/), not guessed.  Eight are the tree's #if 0/#else
//  named-macro idiom (aTester_Front.cpp:150-156, RotateKit/aRotateKIT.cpp:238-242)
//  so golden's expression stays VERBATIM in the #if 0 arm; two are bare #if 0
//  around a statement.
//  ---------------------------------------------------------------------------
//   (1) `int iInRotateFinish = 0;`  -- golden :30.  DUPLICATE SYMBOL, not a
//       missing one.  Golden defines this global HERE and nowhere else, but this
//       tree already defines it at ported acatchtray.cpp:138 (mis-attributed
//       there to golden cmydef.cpp; RotateKit/aRotateKIT.h:32-35 records the same
//       finding).  Landing golden's definition too would be
//       `multiple definition of iInRotateFinish`.  The DEFINITION is gated; the
//       `extern int iInRotateFinish;` in aRotateKIT.h:73 still resolves, to
//       acatchtray.cpp's object, so every reader in the tree sees ONE variable
//       with golden's initial value 0.  ZERO behaviour delta.  Retire by deleting
//       acatchtray.cpp's stray definition, not by ungating here.
//
//   (2) W906RKIN_DUTNUM  <-  `tRotate.DutNum`   (18 golden sites; 17 code + the
//       comment at golden :811 which is left untouched).
//       Golden's TRotate (RotateKit/fRotate.h:28-58) has 25 members; this tree's
//       stand-in `tRotateShim` (aHotPlateSubstrate.h:824-832) carries THREE
//       (ActiveRotate / RotateDutDate / bRotateUseRTmode).  DutNum is not one of
//       them, and aHotPlateSubstrate.h is a SHARED header this wave may not
//       touch, so `tRotate.DutNum` is a COMPILE error here, not a link error.
//       ACTIVE default `tDutType_4` (=0): golden's DutNum is loaded from the
//       rotate ini by TFrmRotate::fRotate_ReadFile, and 0/tDutType_4 is both the
//       zero-init value a not-yet-loaded TRotate holds and golden's own default
//       arm (golden aRotateKIT.cpp:261 spells the final else
//       `else// if(tRotate.DutNum==tDutType_4)`).  So the 4-DUT ladders are
//       ACTIVE and the 8-DUT ladders are present-but-not-selected; NO branch was
//       deleted -- every one still compiles and can be reached the moment the
//       real TRotate lands.
//
//   (3) W906RKIN_ROTKIT_PITCHX / (4) W906RKIN_ROTKIT_PITCHY
//         <-  `tRotate.RotateKit_PitchX` (golden :815, :1701)
//         <-  `tRotate.RotateKit_PitchY` (golden :1701).   Same missing-member
//       cause as (2); golden's type is `double`, so the defaults are `(0.0)` and
//       the golden `*100` arithmetic stays double-then-truncate, unchanged.
//       BEHAVIOUR DELTA, STATED PLAINLY: golden :1701
//       `RotateKIT_FormSet(true, 0, 0, PitchX*100, PitchY*100)` therefore writes
//       iRotateKIT_Pitch_X_H = iRotateKIT_Pitch_Y_H = 0 instead of the taught
//       pitch, so every rotate-station coordinate derived from those two in the
//       MOTOR version collapses to its base position.  That is a data value, not
//       a control-flow change -- no branch is bypassed.  (The CYLINDER version is
//       unaffected: golden :230 passes literals 7800/7975/8000/6000.)
//
//   (5) W906RKIN_TROT_ROTATIONCOUNT(i)  <-  `tRotate.RotationCount[i]`
//       (golden :2414, :2423).  Same cause as (2); default `(0)`, the zero-init
//       of golden's `int RotationCount[8]`.  Site :2423 is inside the
//       FrmRotate->bShowRotateBySite arm, which gate (7) already resolves to
//       false, so it is unreachable today; site :2414 is a golden no-op (see the
//       GOLDEN DEFECTS list, item d).
//
//   (6) W906RKIN_TROT_IROTATEOFFSET(i)  <-  `tRotate.iRotateOffset[i]`
//       (golden :3443).  Same cause as (2); default `(0)`, i.e. no rotate offset,
//       which is exactly what golden holds until the ini supplies one.  Only
//       reached when iRotate_Type==eInOutArm1Motor and the angle is non-zero.
//
//   (7) W906RKIN_SHOWROTATEBYSITE  <-  `FrmRotate->bShowRotateBySite`
//       (golden :1305, :2353, :2409, :2812, :2905).
//       golden RotateKit/fRotate.h:135 puts this on TFrmRotate; this tree's
//       stand-in forms/fRotate.h:47-120 carries only bRotateInHome /
//       bRotateOutHome / b8Rotate*Home / InitialIn|OutRotateHome, and
//       forms/fRotate.h is not this wave's file.  COMPILE error, not link.
//       ACTIVE default `false` = "do not drive the rotate station per-site",
//       which is golden's classic whole-kit behaviour and the value a
//       freshly-constructed TFrmRotate holds (VCL zero-fill; golden's ctor
//       fRotate.cpp:42 never assigns it -- forms/fRotate.h:56-63 makes exactly
//       this argument for its two siblings).  With false the SM takes the
//       Prod.RotationCount[rCount-1] angle-compare paths rather than the
//       per-site Tray.iCurrRotAng paths -- both are golden, neither is deleted.
//
//   (8) `FrmRotate->SetInRotateSpeed(...)` -- golden :1411 and :3271.  Same
//       stand-in gap as (7), but a METHOD, so no macro can stand in: the ONE
//       statement is bare-#if-0'd at each site.  The surrounding cursor
//       transition (Task=100 / Task=2600) stays ACTIVE, so the recovery path
//       still completes; what is lost is only the re-application of
//       ArmSpeed[InRotate] to the rotate axis after a JAM0453 re-home.
//
//   (9) W906RKIN_CHECKINARMZ()  <-  `CheckInArmZ()` (golden :53, :878).   //AI(W906-I04) 20261001 (Ifor01): GATE (9) LIFTED at :376 -- the text below is the 20260807 reason, kept as history
//       Golden home is ainarm2.cpp:1188; ported ainarm2.cpp exists but does NOT
//       contain it -- the identifier `CheckInArmZ` occurs NOWHERE in the port
//       tree, not even as a declaration.  An active call link-fails.
//       ACTIVE default `false` = "every in-arm Z is already at its taught safe
//       position", which is golden's own non-exceptional outcome (golden returns
//       true only when some MOT[..].ReadPos() differs from Prod.ZInArmSafe, and
//       that path additionally calls SetInArmHome() and aborts the move).  false
//       therefore keeps the two XY movers on golden's normal path instead of
//       parking them in a permanent Z-home request.
//
//  (10) W906RKIN_ARMCANSUCK4IC(d)  <-  `ArmCanSuck4IC(0)` (golden :813 twice,
//       :829, :2966).  DECLARED at ported csystem.h:162 but there is NO COMPILED
//       BODY anywhere in this tree (golden csystem.cpp:723 is untranslated), so
//       an active call link-fails with `undefined reference to
//       ArmCanSuck4IC(int, bool)`.  ACTIVE default `true`, which is (a) golden's
//       terminal `return true;` at csystem.cpp:766 after five restriction
//       early-outs, and (b) the SAME default the sibling
//       RotateKit/aRotateKIT.cpp:238-242 already chose for the same symbol, so
//       the two RotateKit units cannot disagree about the machine.
//       BEHAVIOUR DELTA: on a real large-IC / wide-site-pitch 2x4/4x4/2x8
//       machine golden would return false and take the "one rotate per two
//       nozzles" geometry at golden :815-826; here the "suck 8, use 4 rotates"
//       geometry at :829-835 is taken instead.  Retire the moment
//       csystem.cpp:723 lands.
//
//  (11) W906RKIN_HAS_NO_IC()  <-  `InArmSuck.HAS_NO_IC()` (golden :1823).
//       golden mykitsuck.h:296 / mykitsuck.cpp:247; the port's TMyKitSuck mirror
//       (aHotPlateSubstrate.h) does not carry it and that header is not this
//       wave's file.  ACTIVE default `true`.  Note golden's method is
//       MIS-NAMED: it returns true when SOME nozzle is NOT NULL_IC, i.e. "the arm
//       IS carrying something" (golden's call-site comment 確認沒有空IC).  `true`
//       is therefore "the arm has ICs to place", the state the SM is in whenever
//       it is called.  The site sits inside the tRotate.DutNum==tDutType_8 arm,
//       which gate (2) leaves unselected, so the default is unobservable today.
//
//  (12) `FrmAOI->ttbInsp->iRotateKitAngOffset_In` -- golden :3427-3428.  The
//       port's TFrmAOI stand-in (forms/fAOI.h:75-110) carries exactly one member
//       (bSimulateTopBtm); there is no ttbInsp sub-object and no
//       iRotateKitAngOffset_In anywhere in the tree.  The WHOLE two-line golden
//       `if` is bare-#if-0'd (gating only the assignment would silently capture
//       the next statement as the if-body).  `iAOIAngle` keeps its initialiser 0,
//       which is what golden computes whenever USE_Scanner_AOI_Inspection is not
//       eBtnAOI_TopBottomInstall -- the ordinary machine.  Note the sibling
//       aRotateKIT.cpp has the identical golden lines but COMMENTED OUT in golden
//       itself (:85-89 there), so this unit is the only live user.
//
//  ---------------------------------------------------------------------------
//  ONE NON-GATE ADAPTATION -- `SetNeedDestroy`
//  ---------------------------------------------------------------------------
//  golden mykitsuck.h:122 is the inline setter
//  `void SetNeedDestroy(bool Value) {bNeedDestroy=Value;}`.  The port's TMySucker
//  mirror (aHotPlateSubstrate.h:149-151) landed the STORAGE `bNeedDestroy` and
//  the getter GetNeedDestroyStatus() but not the setter, and that header is not
//  this wave's file.  Rather than gate 12 golden statements, a function-like
//  macro reproduces golden's inline body EXACTLY:
//      #define SetNeedDestroy(v)   bNeedDestroy = (v)
//  so `X.SetNeedDestroy(true);` expands to `X.bNeedDestroy = (true);` -- the same
//  write to the same public member the golden setter performs.  This is NOT a
//  gate: no behaviour is defaulted or lost, and every golden call site is
//  byte-identical to golden.  The macro is defined AFTER all #includes (nothing
//  in any included header spells that token) and is retired by adding the real
//  one-line setter to aHotPlateSubstrate.h.
//
//  ---------------------------------------------------------------------------
//  GOLDEN DEFECTS PRESERVED VERBATIM -- translated as-is, NOT fixed
//  ---------------------------------------------------------------------------
//   (a) golden :3117-3133 and :3139-:3196 (M_DoInArmRotateKIT_Motor case 4600,
//       K_SKIP arm): the IN-arm skip loops iterate `OutArmSuck.iMaxCol` and index
//       `iSuckR=iRotato_Out_Row` -- OUT-side objects -- and then write
//       `InArmSuck.PordRec[iSuckR][iSuckC]` / `InArmSuck.SetItemData(...)`.
//       Copy-paste from aRotateKIT_Out.cpp.  Kept.
//   (b) golden :2382-2390: `for(iRotateCount=4; iRotateCount>=0; iRotateCount--)`
//       reads `Prod.RotationCount[iRotateCount-1]`, i.e. `RotationCount[-1]` on
//       the last iteration -- an out-of-bounds read one int before the array.
//       Kept.
//   (c) golden :1610-1616 (GotInRotateKitForAxEx_2RotMot): the fourth else-if
//       tests `iCount==0`, but the two branches above already handle every
//       iCount==0 case, so that arm is unreachable dead code (it was plainly
//       meant to be iCount==1, matching the branch above it).  Kept.
//   (d) golden :2414: `tRotate.RotationCount[rCount-1];` is a bare expression
//       statement -- an array read whose value is discarded.  Kept (behind gate
//       (5), so it expands to the equally-inert `(0);`).
//   (e) golden :1984 and :2019 (M_DoInArmRotateKIT_Motor case 1300): the
//       post-loop hang-up re-check reads `iSuckR` AFTER the for-loop that assigns
//       it; when the loop body never executes, `iSuckR` is read uninitialised.
//       Kept.
//   (f) golden :180 (MoveInArmZ_ToRotateKIT_Pick, the non-1X4 arm): the PICK
//       function moves Z to `Prod.iInArmRotateToHotPlate_PlaceZ[i][j2]` while the
//       1X4 arm two branches up correctly uses `..._PickZ[0][j2]`.  Kept.
//   (g) golden :575-597: `case 1600` of DoInArmRotateKIT_Cylinder is never
//       reachable -- no statement anywhere in the file assigns Task=1600.  Dead
//       golden case, translated in full.  Kept.
//   (h) golden :3826 + :3851 (CheckInRotateSensor): `iCount++` runs once at the
//       top and again inside the failure else, so a persistently-failing sensor
//       reaches the `iCount>50` alarm in ~25 polls, not 50.  Kept.
//   (i) golden :3384: `iRet=M8_DoInRotateMove(rCount, bReset);;` -- stray empty
//       statement.  Kept.
//   (j) golden :71: `int iMovePitchX =iRotateKIT_Pitch_X_H*3/2;;` -- stray empty
//       statement, and the *3/2 is INTEGER arithmetic (truncating).  Kept exactly.
//   (k) LATENT OUT-OF-BOUNDS, config-dependent: the e1MotRotate1Dut / e2MotRotate2Dut
//       arms iterate `for(int j=iKit; j<InArmSuck.iMaxCol; j+=4)` (golden :936,
//       :1111, :1136, :1956, :1991, :2580, :2611, :3117, :3139) and then index the
//       LOCAL `bool flag[2][4]` and the STATIC
//       `bSuckDuplicateErr[MAX_ARM_Row][MAX_ARM_Col]` (= [2][4]) with that j.  On a
//       machine where InArmSuck.iMaxCol > 4 (an 8-picker arm) j reaches 4 and both
//       writes run past the row.  TMySucker Suck[4][8] / Item are wide enough; only
//       these two 4-wide arrays are not.  Golden's, unchanged here.
//
//  ---------------------------------------------------------------------------
//  VCL/Borland conversions: #pragma hdrstop (golden :2) + #pragma
//  package(smart_init) (golden :27) dropped.  No __fastcall / __property /
//  __published anywhere in this file -- golden has none either.  AnsiString is
//  used (ErrPart / sInArmAngleRecord) and comes from vclcompat via
//  MachineDefine.h; `ErrPart+=InArmSuck.Suck[i][j].sName;` and
//  `StrToInt(sInArmAngleRecord)` are verbatim.  ZeroMemory comes from <windows.h>
//  via MachineDefine.h, exactly as the other ported SMs get it.
//  SOFT_SIMULTE is NOT defined and this file contains no #ifdef SOFT_SIMULTE.
//  Numeric semantics kept EXACT: every integer division (/90, /2, *3/2, /2*3,
//  /100.0-free) and every `j*2+iKit` / `j*i2x2Suck+iKit` index expression is
//  golden's, unrewritten; no int was promoted to float.
//
//  Big5: every Chinese comment decoded via cp950 and preserved as UTF-8.
//  Final gate: ZERO U+FFFD.
// =============================================================================
// BCB6 ORIGINAL include block (mirrored as a comment for provenance):
//   #include "MachineDefine.h" ; #pragma hdrstop
//   #include "aRotateKIT_In.h" / "aRotateKIT.h" / "aArmHeader.h" / "cprod.h" /
//     "csystem.h" / "mymotor.h" / "MyKitSuck.h" / "mycylin.h" / "mysensor.h" /
//     "main.h" / "note.h" / "fRotate.h" / "uhome.h" / "cinitial.h" / "cMyDB.h" /
//     "cmydef.h" / "SCK_ART.h" / "mymessbox.h" / "fAOI.h"
//   ; #pragma package(smart_init)
//
//   NOT re-included here, and why:
//     uhome.h / cinitial.h / SCK_ART.h -- golden's god-stack; not one symbol from
//       any of them is referenced by the translated bodies (fSCKART itself comes
//       from FormsFacade.h -> forms/fSCKART.h, which is where this tree keeps it).
//     mymessbox.h -- golden's home for ShowMyMessage; this tree serves it from
//       canary_support.h:26-28.  The TMyMessageBox OBJECT is never touched here.
//     cMyDB.h -- golden's home for RecordProcess; also served by
//       canary_support.h:25.
//     main.h / note.h -- golden's homes for fMain / fNote / ShowErrorMessage;
//       served by FormsFacade.h (forms/fMain.h, forms/fNote.h) and
//       canary_support.h respectively.
//     MyKitSuck.h -- this tree keeps TMyKitSuck/TMySucker + InArmSuck/OutArmSuck
//       in aHotPlateSubstrate.h.
//     fRotate.h / fAOI.h -- this tree splits golden's fRotate.h in two: the
//       TRotate DATA (`tRotate`) lives in aHotPlateSubstrate.h and the TFrmRotate
//       FORM (`FrmRotate`) in forms/fRotate.h; both are included below.
// =============================================================================
#include "MachineDefine.h"          // de-VCL'd include hub (vclcompat umbrella + <windows.h> ZeroMemory + STL + using namespace std)

#include "aRotateKIT_In.h"          // this unit's own contract
#include "aRotateKIT.h"             // sibling shared contract: i2x2Suck / iRotateKIT_Start|Pitch_X|Y_H / RotateKIT_FormSet / SetMotorResolution / InitSuckState / CheckRotateSensor / extern iInRotateFinish
#include "aArmHeader.h"             // golden's arm god-header -- guard-only shim in this tree (aArmHeader.h:8-23)
#include "cprod.h"                  // Prod (iInArmRotateToHotPlate* / iInArm_Rotate* / RotateDutDate / RotationCount / RotationTimeIn / iIn_iRotateA), TestIF, TrayForm, DeviceForm, ArmSpeed[]
#include "cpublic.h"                // CosFunction (bOPCanPressStepAndTStart / bUseSCKART)
#include "csystem.h"                // CheckInArmSuckICFallDownToHasNullIC / CheckInArmDestroyICFail / InitDoArmZHome ; ArmCanSuck4IC DECLARATION only -- see GATE (10)
#include "Motor/mymotor.h"          // MOT[] (TTrayMotor + TMyTray) / InArmContinuousMove_9045
#include "mysensor.h"               // Sen[] (SnRKManualTStart / SnRotateRowIn1 / SnRotateRowIn2)
#include "mycylin.h"                // Cylinder[] (C_InputRotateKIT: On/Off/Push/Pop)
#include "aHotPlateSubstrate.h"     // golden MyKitSuck.h -> InArmSuck/OutArmSuck ; golden fRotate.h -> tRotate ; MoveInArmZToPlateSafe / SetInArmHome / bInArmCheckDestroyACT / GetInArmPitchX|Y_9045 / bUseAxxGPicker / iCloseSiteModeFor1x4
#include "acatchtray_shims.h"       // MoveInArm2XYToWait (golden ainarm2.cpp:1310; shim body acatchtray_shims.cpp:110)
#include "cmydef.h"                 // global scalar universe: MInRotateKit / MInRotate[][] / InArmZIndex / iRotate_Type / USE_ROTATE_KIT / iRotato_In|Out_Row / bWaitRotateFinish / bContinusRotate / bEnterOffset / bNeedArmZHome / bIn_* / NULL_IC / HAS_NULL_IC / K_RETRY / K_SKIP / iInArmType / AccessLevel
#include "MachineType.h"            // eRotateType (eCynRotate/e1MotRotate/e4MotRotate/e8MotRotate/e1MotRotate1Dut/e2MotRotate2Dut/eInOutArm1Motor), test-mode enum, InArm/InRotate speed-part enum, InOfsLoader/InOfsRotate_In, MAX_ARM_Row|Col, X_PITCH_COUNT
#include "Config.h"                 // IniConfig (bAlarmNeedServoOff / bE34InOutArmPitchZOffsetSameOne / bA21RotateDetectErrNeedShake)
#include "FormsFacade.h"            // fMain->Pause / fNote->bMyServoOffInArm / fSCKART->iInputJamCnt
#include "forms/fRotate.h"          // FrmRotate (bRotateInHome / InitialInRotateHome) -- NOT re-exported by FormsFacade.h
#include "forms/fAOI.h"             // FrmAOI -- NOT re-exported by FormsFacade.h; see GATE (12)
#include "canary_support.h"         // ShowErrorMessage / ShowMyMessage / RecordProcess / __FUNC__ / LastSet.h -> TECH Tech (Tech.iInRotate[][])

// =============================================================================
//  LOCAL SUBSTRATE  --  everything below exists ONLY because a shared surface
//  this unit needs is not on this tree's stand-in yet, and the shared header is
//  not this wave's file.  Each item is one of the numbered entries in the GATE
//  REGISTER above; read that register for the full reasoning.  Nothing here
//  invents behaviour -- every ACTIVE default is a value or branch golden itself
//  produces.
// =============================================================================

//AI(W906-ROTKIT-IN) 20260807: golden RotateKit/fRotate.h:21-25 `enum {tDutType_4..}`.
//  golden's fRotate.h is split in this tree (data -> aHotPlateSubstrate.h, form ->
//  forms/fRotate.h) and NEITHER half carries this enum -- `tDutType_4` / `tDutType_8`
//  occur nowhere in the port.  Reproduced VERBATIM (same enumerators, same values) in
//  the .cpp rather than in aRotateKIT_In.h, so it cannot collide with whatever the
//  untranslated aRotateKIT_Out.cpp sibling chooses to do.  Macro-guarded in case a
//  future shared header supplies it first.
#ifndef HT9045_TDUTTYPE_ENUM_DEFINED
#define HT9045_TDUTTYPE_ENUM_DEFINED
enum {tDutType_4=0,
      tDutType_8=1,
      tDutType_1=2,
      tDutType_2=3,
      tDutType_Total};
#endif

// ---- GATE (2) : tRotate.DutNum -- not on aHotPlateSubstrate.h's tRotateShim ---
#if 1 // was: #if 0 -- opened AI(W906-POOL2) 20261008 (Ifor01): GATE (2) reason expired -- tRotateShim is `typedef TRotate tRotateShim` since AI(W906-FRW-NoPage) 20260926 (aHotPlateSubstrate.h:868), the golden full type, filled from Rotate.Data at boot (FileRW_Rotate_ReadFile); TRotate::DutNum is forms/fRotate.h:73 = golden RotateKit/fRotate.h:34
#define W906RKIN_DUTNUM                 (tRotate.DutNum)
#else
#define W906RKIN_DUTNUM                 (tDutType_4)
#endif

// ---- GATE (3)/(4) : tRotate.RotateKit_PitchX / _PitchY (golden `double`) ------
#if 1 // was: #if 0 -- opened AI(W906-POOL2) 20261008 (Ifor01): GATE (3)/(4) reason expired -- tRotateShim is `typedef TRotate tRotateShim` since AI(W906-FRW-NoPage) 20260926 (aHotPlateSubstrate.h:868), the golden full type, filled from Rotate.Data at boot (FileRW_Rotate_ReadFile); TRotate::RotateKit_PitchX/_PitchY (double) is forms/fRotate.h:85-86 = golden RotateKit/fRotate.h:46-47
#define W906RKIN_ROTKIT_PITCHX          (tRotate.RotateKit_PitchX)
#define W906RKIN_ROTKIT_PITCHY          (tRotate.RotateKit_PitchY)
#else
#define W906RKIN_ROTKIT_PITCHX          (0.0)
#define W906RKIN_ROTKIT_PITCHY          (0.0)
#endif

// ---- GATE (5) : tRotate.RotationCount[] --------------------------------------
#if 1 // was: #if 0 -- opened AI(W906-POOL2) 20261008 (Ifor01): GATE (5) reason expired -- tRotateShim is `typedef TRotate tRotateShim` since AI(W906-FRW-NoPage) 20260926 (aHotPlateSubstrate.h:868), the golden full type, filled from Rotate.Data at boot (FileRW_Rotate_ReadFile); TRotate::RotationCount[8] is forms/fRotate.h:80 = golden RotateKit/fRotate.h:41
#define W906RKIN_TROT_ROTATIONCOUNT(i)  (tRotate.RotationCount[i])
#else
#define W906RKIN_TROT_ROTATIONCOUNT(i)  (0)
#endif

// ---- GATE (6) : tRotate.iRotateOffset[] --------------------------------------
#if 1 // was: #if 0 -- opened AI(W906-POOL2) 20261008 (Ifor01): GATE (6) reason expired -- tRotateShim is `typedef TRotate tRotateShim` since AI(W906-FRW-NoPage) 20260926 (aHotPlateSubstrate.h:868), the golden full type, filled from Rotate.Data at boot (FileRW_Rotate_ReadFile); TRotate::iRotateOffset[2] is forms/fRotate.h:96 = golden RotateKit/fRotate.h:57
#define W906RKIN_TROT_IROTATEOFFSET(i)  (tRotate.iRotateOffset[i])
#else
#define W906RKIN_TROT_IROTATEOFFSET(i)  (0)
#endif

// ---- GATE (7) : FrmRotate->bShowRotateBySite ---------------------------------
#if 1 // was: #if 0 -- opened AI(W906-POOL2) 20261008 (Ifor01): GATE (7) reason expired -- TFrmRotate has bShowRotateBySite (forms/fRotate.h:184 = golden RotateKit/fRotate.h:133); FrmRotate is a static new (forms/fRotate.cpp:127) and FileRW_Rotate_Boot sets the flag as golden's constructor does
#define W906RKIN_SHOWROTATEBYSITE       (FrmRotate->bShowRotateBySite)
#else
#define W906RKIN_SHOWROTATEBYSITE       (false)
#endif

bool CheckInArmZ();   // ---- GATE (9) : CheckInArmZ() ------------------------------------------------   //AI(W906-I04) 20261001 (Ifor01): declared here (ainarm2.cpp:734 has the body, no header declares it), so the #if 1 arm below compiles
#if 1 // TODO(W7): golden ainarm2.cpp:1188 CheckInArmZ -- identifier absent from the whole port tree (ported ainarm2.cpp does not carry it)   //AI(W906-I04) 20261001 (Ifor01): LIFTED -- CheckInArmZ 現在有真本體（ainarm2.cpp:734，golden ainarm2.cpp:1188），RULINGS_20261001 第 0 條：依賴到位就照 golden 接上。golden 的意思是「入料臂 Z 不剛好在 Prod.ZInArmSafe 就不移 XY、先歸零 Z」——閘成 false 等於拿掉這道互鎖。解開後 RotateKit 的重試路徑不會再卡：同一顆的 case 4710（:3695）先把 Z 移到安全高度
#define W906RKIN_CHECKINARMZ()          (CheckInArmZ())
#else
#define W906RKIN_CHECKINARMZ()          (false)
#endif

// ---- GATE (10) : ArmCanSuck4IC() ---------------------------------------------
#if 1 // was: #if 0 -- opened AI(W906-POOL2) 20261008 (Ifor01): GATE (10) reason expired -- ArmCanSuck4IC has its translated body now (csystem.cpp, `bool ArmCanSuck4IC(int Direct, bool flag)`, golden csystem.cpp:723), declared csystem.h:162 with golden's default flag=true
#define W906RKIN_ARMCANSUCK4IC(Direct)  (ArmCanSuck4IC(Direct))
#else
#define W906RKIN_ARMCANSUCK4IC(Direct)  (true)
#endif

// ---- GATE (11) : InArmSuck.HAS_NO_IC() ---------------------------------------
#if 1 // AI(W906-W2-A46) 20260926: gate FLIPPED to the golden arm -- A4-6 (8ff6c754) merged TMyKitSuck/TMySucker into the one golden layout (mykitsuck.h), premise dead: TMyKitSuck::HAS_NO_IC() is mykitsuck.h:400 (nm T); the #else arm always said true
#define W906RKIN_HAS_NO_IC()            (InArmSuck.HAS_NO_IC())
#else
#define W906RKIN_HAS_NO_IC()            (true)
#endif

//AI(W906-ROTKIT-IN) 20260807: NOT A GATE -- exact reproduction of golden
//  mykitsuck.h:122 `void SetNeedDestroy(bool Value) {bNeedDestroy=Value;}`.  The port's
//  TMySucker mirror landed the storage + getter but not the setter, and
//  aHotPlateSubstrate.h is not this wave's file.  The macro makes every one of the 12
//  golden call sites below byte-identical to golden while writing the very same public
//  member the golden setter writes.  Defined AFTER every #include on purpose (no
//  included header spells this token).  Retire by adding the one-line setter to
//  aHotPlateSubstrate.h and deleting these three lines.
#define SetNeedDestroy(v)               bNeedDestroy = (v)

int iInArmRotateKit         =1;
//AI(W906-ROTKIT-IN) 20260807: GATE (1) -- golden :30 DEFINES iInRotateFinish, but this
//  tree already defines it at ported acatchtray.cpp:138 (see RotateKit/aRotateKIT.h:32-35),
//  so landing golden's definition too is `multiple definition of iInRotateFinish`.  The
//  DEFINITION is parked; the `extern int iInRotateFinish;` at aRotateKIT.h:73 still resolves
//  -- to acatchtray.cpp's object, which carries golden's own initial value 0.  Zero delta.
//  Retire by deleting acatchtray.cpp's stray definition, not by ungating here.
#if 0 // TODO(W7): GATE (1) -- golden :30 iInRotateFinish -- DUPLICATE definition (this tree already defines it at acatchtray.cpp:138)
int iInRotateFinish         =0;                                                 //kevin 20130524 確保IC有放到旋轉馬達轉向  1:初始化  2:旋轉完成  3:放置SHUTTLE 0:放完SHUTTLE    //Steven 20160809 : 改成全域變數
#endif
int iShakeInArmRotateKit    =1;                                                 //JerryYang 20160803
int iInArmShakeCnt          =0;                                                 //JerryYang 20160803 旋轉次數計數
int iInRotateWhichKit       =0;                                                 //kevin 20130629 使用4個rotate 0:表示放第一次 Aa Ab Ae Ae  1:表示放第二次 Ac Ad Ag Ah
int InAngle90               =0;                                                 //90度等於多少pulse
int InAngle45               =0;                                                 //45度等於多少pulse   //kevin 20130710 旋轉45 度 add

int InArmRotate_PitchY      =6000;
int InArmRotate_PitchX      =12000;
//---------------------------------------------------------------------------
void InitialInArmRotateKIT()
{
    iInArmRotateKit=1;
}
//---------------------------------------------------------------------------
void InitialShakeInArmRotateKIT()                                               //JerryYang 20160803 初始化Rotate旋轉功能
{
    iShakeInArmRotateKit=1;
    iInArmShakeCnt=0;
}
//---------------------------------------------------------------------------
bool MoveInArmXY_ToRotateKIT(int iKit)
{
    //AI(W906-ROTKIT-IN) 20260807: GATE (9) -- golden :53 CheckInArmZ() has no compiled
    //  home in this tree (golden ainarm2.cpp:1188); macro resolves to golden's normal-path
    //  `false` ("all in-arm Z already at Prod.ZInArmSafe").  Golden text in the #if 0 arm.
    if(W906RKIN_CHECKINARMZ())
    {
        bNeedArmZHome=true;
        InitDoArmZHome();
        return false;
    }

    if(IniConfig.bAlarmNeedServoOff)                                            //Steven 20111226 : In Arm 咖啡杯解除
    {
        if(fNote->bMyServoOffInArm)
        {
            return false;
        }
    }

    int iXPos=0, iYPos=0;
    int iXVariable[X_PITCH_COUNT];
    int iYVariable  =0;
    int iMovePitchX =iRotateKIT_Pitch_X_H*3/2;;
    int iMovePitchY =6000;
    bool bZFlag[MAX_ARM_Row][MAX_ARM_Col];
    int iZPos[MAX_ARM_Row][MAX_ARM_Col];
    int iOffsetPos=-1;
    ZeroMemory(iXVariable, sizeof(iXVariable));
    ZeroMemory(bZFlag, sizeof(bZFlag));
    ZeroMemory(iZPos, sizeof(iZPos));

    if(IniConfig.bE34InOutArmPitchZOffsetSameOne==true)                         //jou 2010-12-20 Pitch & Z 縮減為一個
    {
        iOffsetPos=InOfsLoader;
    }
    else
    {
        iOffsetPos=InOfsRotate_In;                                              //Sam 20240808 : 修正 Pitch Offset
    }

    if(iKit==1)                                                                 // Z2 & Z4
    {
        iXPos    =Prod.iInArmRotateToHotPlateX+iRotateKIT_Start_X_H+iRotateKIT_Pitch_X_H/2;

        if(TestIF.iTestMode==QualSite1X4 || TestIF.iTestMode==_8Site1X4)        //ChungHung 20150528 add for 海思 _8Site1x4
            iYPos=Prod.iInArmRotateToHotPlateY-iRotateKIT_Start_Y_H-iRotateKIT_Pitch_Y_H;
        else
            iYPos=Prod.iInArmRotateToHotPlateY-iRotateKIT_Start_Y_H;
    }
    else // iKit = 0                                                            // Z1 & Z3(F)
    {
        iXPos   =Prod.iInArmRotateToHotPlateX+iRotateKIT_Start_X_H+iRotateKIT_Pitch_X_H;
        iYPos   =Prod.iInArmRotateToHotPlateY-iRotateKIT_Start_Y_H;
    }

    iYVariable=GetInArmPitchY_9045(iMovePitchY, iOffsetPos);
    for(int i=0; i<X_PITCH_COUNT; i++)
        iXVariable[i]=GetInArmPitchX_9045(iMovePitchX, i, iOffsetPos);

    if(InArmContinuousMove_9045(iXPos, iYPos, iXVariable, iYVariable, bZFlag, iZPos, false))
    {
        return true;
    }
    return false;
}
//---------------------------------------------------------------------------
bool MoveInArmZ_ToRotateKIT_Place(int iKit)
{
    bool flag[2][4]={{true, true, true, true}, {true, true, true, true}};
    for(int i=0; i<MAX_ARM_Row; i++)
    {
        for(int j=iKit; j<MAX_ARM_Col; j+=2)                                    //(0, 2) & (1, 3)
        {
            if(InArmSuck.Item[i][j]!=NULL_IC &&
               InArmSuck.Item[i][j]!=HAS_NULL_IC)
            {
                InArmSuck.Suck[i][j].SetNeedDestroy(true);
                flag[i][j]=MOT[InArmZIndex[i][j]].MotorMove(Prod.iInArmRotateToHotPlate_PlaceZ[i][j]);
            }
            else
            {
                flag[i][j]=true;
            }
        }
    }

    for(int i=0; i<MAX_ARM_Row; i++)
    {
        for(int j=0; j<MAX_ARM_Col; j++)
        {
            if(flag[i][j]==false)
                return false;
        }
    }
    return true;
}
//---------------------------------------------------------------------------
bool MoveInArmZ_ToRotateKIT_Pick(int iKit)
{
    bool flag[2][4]={{true, true, true, true}, {true, true, true, true}};
    int  j2;

    if(TestIF.iTestMode==QualSite1X4 || TestIF.iTestMode==_8Site1X4)            //ChungHung 20150528 add for 海思 _8Site1x4
    {
        int i=iKit;
        for(int j=0; j<MAX_ARM_Col/2; j++)                                      //(0, 2) & (1, 3)
        {
            j2=j*2+iKit;
            //Kit(0, 1) -> Suck(0, 2) or Suck(1, 3)
            if(MOT[MInRotateKit].Tray.Data[j][i]!=NULL_IC && MOT[MInRotateKit].Tray.Data[j][i]!=HAS_NULL_IC)
            {
                InArmSuck.Suck[0][j2].SetNeedSuck(true);
                flag[0][j2]=MOT[InArmZIndex[0][j2]].MotorMove(Prod.iInArmRotateToHotPlate_PickZ[0][j2]);
            }
            else
            {
                flag[0][j2]=true;
            }
        }
    }
    else
    {
        for(int i=0; i<MAX_ARM_Row; i++)
        {
            for(int j=0; j<MAX_ARM_Col/2; j++)                                  //(0, 2) & (1, 3)
            {
                j2=j*2+iKit;
                //Kit(0, 1) -> Suck(0, 2) or Suck(1, 3)
                if(MOT[MInRotateKit].Tray.Data[j][i]!=NULL_IC && MOT[MInRotateKit].Tray.Data[j][i]!=HAS_NULL_IC)
                {
                    InArmSuck.Suck[i][j2].SetNeedSuck(true);
                    flag[i][j2]=MOT[InArmZIndex[i][j2]].MotorMove(Prod.iInArmRotateToHotPlate_PlaceZ[i][j2]);
                }
                else
                {
                    flag[i][j2]=true;
                }
            }
        }
    }

    for(int i=0; i<MAX_ARM_Row; i++)
    {
        for(int j=0; j<MAX_ARM_Col; j++)
        {
            if(flag[i][j]==false)
                return false;
        }
    }
    return true;
}
//---------------------------------------------------------------------------
bool DoInArmRotateKIT_Cylinder()
{
    int &Task=iInArmRotateKit;
    int j2;

    static int iRetryCT=0;
    static int iKit=0;

    int ret;
    bool flag=true;
    bool bHasDuplicateErr=false;
    static bool bSuckDuplicateErr[2][4]={{false, false, false, false}, {false, false, false, false}};  //Steven 20091218 : Avoid duplicate message
    AnsiString ErrPart="";
    int iMAX_ARM_Row,iMAX_ARM_Col;
    if(W906RKIN_DUTNUM == tDutType_8)                                           //kevin 20130517 add 8 site
    {
         iMAX_ARM_Row= MAX_ARM_Row;
         iMAX_ARM_Col= MAX_ARM_Col;
    }
    else
    {
         iMAX_ARM_Row= MAX_ARM_Row;
         iMAX_ARM_Col= MAX_ARM_Col/2;
    }

    switch(Task)
    {
        case 1:
            //RotateKIT_FormSet(bool bInput,int iStartX, int iStartY, int iPitchX, int iPitchY)
            RotateKIT_FormSet(         true,       7800,        7975,        8000,        6000);
            iKit=0;     //初始化
            Task=100;
        case 100:
            if(TrayForm.iRotateKIT_InputType==1)            // +90
            {
                Cylinder[C_InputRotateKIT].Off();
            }
            else if(TrayForm.iRotateKIT_InputType==2)       // -90
            {
                Cylinder[C_InputRotateKIT].On();
            }
            else // if(TrayForm.iRotateKIT_InputType==0)
            {
                 return true;
            }
            MOT[MInRotateKit].SetTray(NULL_IC, __FUNC__);
            ZeroMemory(bSuckDuplicateErr, sizeof(bSuckDuplicateErr));
            Task=1000;
        case 1000:                                                              //確認有沒有要放IC
            if((InArmSuck.Item[0][0+iKit]!=NULL_IC && InArmSuck.Item[0][0+iKit]!=HAS_NULL_IC) ||
               (InArmSuck.Item[0][2+iKit]!=NULL_IC && InArmSuck.Item[0][2+iKit]!=HAS_NULL_IC) ||
               (InArmSuck.Item[1][0+iKit]!=NULL_IC && InArmSuck.Item[1][0+iKit]!=HAS_NULL_IC) ||
               (InArmSuck.Item[1][2+iKit]!=NULL_IC && InArmSuck.Item[1][2+iKit]!=HAS_NULL_IC))
            {
                Task=1100;
            }
            else
            {
                Task=2000;
                break;
            }
        case 1100:                                                              //X, Y移動到放的位置
            if(CheckInArmSuckICFallDownToHasNullIC(false)==true)                //Steven 20110516 : 修改成整合式Alarm
            {
                Task=1110;                                                      //JerryYang 20200422 修正掉料可能檢查不出來的問題, 檢查到有掉料要重新再確認全部吸嘴
                break;
            }

            if(MoveInArmXY_ToRotateKIT(iKit)==true)
            {
                if((TrayForm.iRotateKIT_InputType==1 && Cylinder[C_InputRotateKIT].Pop() ==true) ||  // +90
                   (TrayForm.iRotateKIT_InputType==2 && Cylinder[C_InputRotateKIT].Push()==true))    // -90
                {
                    for(int i=0; i<MAX_ARM_Row; i++)
                    {
                        for(int j=0; j<MAX_ARM_Col; j++)
                        {
                            InArmSuck.Suck[i][j].SetNeedDestroy(false);
                        }
                    }
                    Task=1200;
                }
            }
            break;
        case 1110:
            CheckInArmSuckICFallDownToHasNullIC();                              //JerryYang 20200422 修正掉料可能檢查不出來的問題, 檢查到有掉料要重新再確認全部吸嘴
            Task=1100;
            break;
        case 1200:                                                              //Z軸向下移動
            if(MoveInArmZ_ToRotateKIT_Place(iKit)==true)
            {
                if((AccessLevel>=1 || CosFunction.bOPCanPressStepAndTStart) &&  //JerryYang 20170417 (wei) OP權限也能按Step跟T.Start
                    Sen[SnRKManualTStart].IsOn())
                {
                    bEnterOffset=false;
                    fMain->Pause("DoInArmRotateKIT_Cylinder 1200");
                    Task=1500;
                    break;
                }
                Task=1300;
                InArmSuck.ResetAll();                                           //Steven 20160323 : 避免未開啟真空
            }
            else
            {
                break;
            }
        case 1300:                                                              //吹下IC
            flag=true;
            for(int i=0; i<MAX_ARM_Row; i++)
            {
                for(int j=0; j<MAX_ARM_Col/2; j++)
                {
                   j2=j*2+iKit;
                   if(InArmSuck.Item[i][j2] && InArmSuck.Suck[i][j2].GetNeedDestroyStatus())
                    {
                        if(InArmSuck.Suck[i][j2].Destroy())
                        {
                            bSuckDuplicateErr[i][j2]=false;
                            bInArmCheckDestroyACT[i][j2]=true;                  //jou 981130 確認device確實destroy完成
                            if(TestIF.iTestMode==QualSite1X4 || TestIF.iTestMode==_8Site1X4)  //ChungHung 20150528 add for 海思 _8Site1x4
                            {
                                MOT[MInRotateKit].SetTraySingleData(j, i+iKit, InArmSuck.Item[i][j2]);
                            }
                            else
                            {
                                MOT[MInRotateKit].SetTraySingleData(j, i, InArmSuck.Item[i][j2]);
                            }
                            InArmSuck.SetItemData(i,j2,NULL_IC);
                        }
                        else if(InArmSuck.Suck[i][j2].Error==false)
                        {
                            flag=false;
                        }
                    }
                }
            }

            if(flag==false)                                                     //kevin 20131011 在下真空誤判, 換到上面來
                break;

            for(int i=0; i<MAX_ARM_Row; i++)
                for(int j=0; j<MAX_ARM_Col; j++)
                    if(InArmSuck.Item[i][j] && InArmSuck.Suck[i][j].GetNeedDestroyStatus())
                        return false;

            ret=CheckInArmDestroyICFail();                                      //Steven 20111223 : 檢查破壞錯誤
            if(ret==false)
                return false;

            ZeroMemory(bSuckDuplicateErr, sizeof(bSuckDuplicateErr));
            Task=2000;
            break;
        case 2000:                                                              //放下結束, Z軸升起
            if(MoveInArmZToPlateSafe(Task))
            {
                if(iKit==0 && MOT[MInRotateKit].FullIC()==false)
                {
                    iKit++;
                    Task=1000;
                    break;
                }
                else
                {
                    if(InArmSuck.Item[0][0]==NULL_IC && InArmSuck.Item[0][2]==NULL_IC &&
                       InArmSuck.Item[1][0]==NULL_IC && InArmSuck.Item[1][2]==NULL_IC)
                        iKit=0;
                    Task=3000;
                }
            }
            else
            {
                break;
            }
        case 3000:                                                              //旋轉      //kevin 20131106 start   黏貨判斷
            flag=CheckInArmDestroyICFail();                                     //Steven 20111223 : 檢查吹氣IC是否粘黏錯誤
            if(flag==false)
            {
                RecordProcess("VOFTask=3000");
                return false;
            }
            Task++;
            break;                                                              //kevin 20131106 end
        case 3001:
            if(TrayForm.iRotateKIT_InputType==1)            // +90
            {
                Cylinder[C_InputRotateKIT].On();
                Task=3100;
            }
            else if(TrayForm.iRotateKIT_InputType==2)       // -90
            {
                Cylinder[C_InputRotateKIT].Off();
                Task=3100;
            }
            else
            {
                break;
            }
        case 3100:                                                              //吸取IC
            if((MOT[MInRotateKit].Tray.Data[0][0]!=NULL_IC && MOT[MInRotateKit].Tray.Data[0][0]!=HAS_NULL_IC) ||
               (MOT[MInRotateKit].Tray.Data[1][0]!=NULL_IC && MOT[MInRotateKit].Tray.Data[1][0]!=HAS_NULL_IC) ||
               (MOT[MInRotateKit].Tray.Data[0][1]!=NULL_IC && MOT[MInRotateKit].Tray.Data[0][1]!=HAS_NULL_IC) ||
               (MOT[MInRotateKit].Tray.Data[1][1]!=NULL_IC && MOT[MInRotateKit].Tray.Data[1][1]!=HAS_NULL_IC))
            {
                Task=4000;
            }
            else
            {
                Task=6000;
                break;
            }
        case 4000:
            if(CheckInArmSuckICFallDownToHasNullIC(false)==true)                //Steven 20110516 : 修改成整合式Alarm
            {
                Task=4010;                                                      //JerryYang 20200422 修正掉料可能檢查不出來的問題, 檢查到有掉料要重新再確認全部吸嘴
                break;
            }

            if(MoveInArmXY_ToRotateKIT(iKit)==true)
            {
                if((TrayForm.iRotateKIT_InputType==1 && Cylinder[C_InputRotateKIT].Push()==true) ||         // +90
                   (TrayForm.iRotateKIT_InputType==2 && Cylinder[C_InputRotateKIT].Pop() ==true))           // -90
                {
                    for(int i=0; i<MAX_ARM_Row; i++)
                    {
                        for(int j=0; j<MAX_ARM_Col; j++)
                        {
                            InArmSuck.Suck[i][j].SetNeedSuck(true);             //kevin 20130517
                        }
                    }
                    Task=4100;
                }
            }
            break;
        case 4010:
            CheckInArmSuckICFallDownToHasNullIC();                              //JerryYang 20200422 修正掉料可能檢查不出來的問題, 檢查到有掉料要重新再確認全部吸嘴
            Task=4000;
            break;
        case 4100:
            if(MoveInArmZ_ToRotateKIT_Pick(iKit)==true)
            {
                if((AccessLevel>=1 || CosFunction.bOPCanPressStepAndTStart) &&  //JerryYang 20170417 (wei) OP權限也能按Step跟T.Start
                    Sen[SnRKManualTStart].IsOn())
                {
                    bEnterOffset=false;
                    fMain->Pause("DoInArmRotateKIT_Cylinder 4100");
                    Task=4500;
                    break;
                }

                InArmSuck.ResetAll();                                           //Steven 20160323 : 避免未開啟真空
                iRetryCT=0;
                Task=4200;
            }
            else
            {
                break;
            }
        case 4200:
            flag=true;
            if(TestIF.iTestMode==QualSite1X4 || TestIF.iTestMode==_8Site1X4)    //ChungHung 20150528 add for 海思 _8Site1x4
            {
                for(int j=0; j<MAX_ARM_Col/2; j++)
                {
                    if(MOT[MInRotateKit].Tray.Data[j][iKit]!=NULL_IC)
                    {
                        j2=j*2+iKit;
                        if(MOT[MInRotateKit].Tray.Data[j][iKit]==HAS_NULL_IC || InArmSuck.Suck[0][j2].Suck())
                        {
                            bSuckDuplicateErr[0][j2]=false;
                            InArmSuck.SetItemData(0, j2, MOT[MInRotateKit].Tray.Data[j][iKit]);
                            MOT[MInRotateKit].SetTraySingleData(j, iKit, NULL_IC);
                        }
                        else if(InArmSuck.Suck[0][j2].Error==false)
                        {
                            flag=false;
                        }
                    }
                }
            }
            else
            {
                for(int i=0; i<iMAX_ARM_Row; i++)
                {
                    for(int j=0; j<iMAX_ARM_Col; j++)
                    {
                        if(MOT[MInRotateKit].Tray.Data[j][i]!=NULL_IC)
                        {
                            j2=j*2+iKit;
                            if(MOT[MInRotateKit].Tray.Data[j][i]==HAS_NULL_IC || InArmSuck.Suck[i][j2].Suck())
                            {
                                bSuckDuplicateErr[i][j2]=false;
                                InArmSuck.SetItemData(i, j2, MOT[MInRotateKit].Tray.Data[j][i]);
                                MOT[MInRotateKit].SetTraySingleData(j, i, NULL_IC);
                            }
                            else if(InArmSuck.Suck[i][j2].Error==false)
                            {
                                flag=false;
                            }
                        }
                    }
                }
            }

            if(flag==false)
                break;

            for(int i=0; i<MAX_ARM_Row; i++)
            {
                for(int j=0; j<MAX_ARM_Col; j++)
                {
                    if(InArmSuck.Suck[i][j].Error)
                    {
                        iRetryCT++;
                        Task=4600;
                        return false;
                    }
                }
            }

            iRetryCT=0;
            Task=4300;
            break;
        case 4300:
            if(MoveInArmZToPlateSafe(Task))
            {
                if(iKit==0)
                {
                    iKit++;
                    if(TestIF.iTestMode==_8Site2X4 || TestIF.iTestMode==_16Site2X8 || TestIF.iTestMode==_16Site4X4) //Sam 20190226 : 16Site4X4
                        Task=100;
                    else
                        Task=3100;
                    break;
                }
                else
                {
                    iKit=0;
                    Task=6000;
                }
            }
            else
            {
                break;
            }
        case 6000:
            if(TrayForm.iRotateKIT_InputType==1)            // +90
            {
                Cylinder[C_InputRotateKIT].Off();
            }
            else if(TrayForm.iRotateKIT_InputType==2)       // -90
            {
                Cylinder[C_InputRotateKIT].On();
            }

            iKit=0;
            return true;
        // 例外處理 -----------------------------
        case 1500:                                                              //(0, 2)調整Offset
            if(bEnterOffset==true)
            {
                bEnterOffset=false;
                Task=1510;
            }
            else
            {
                Task=1200;
            }
            break;
        case 1510:
            if(MoveInArmZToPlateSafe(Task))
            {
                Task=1100;
            }
            break;
        case 1600:                                                              //(0, 2)吹氣異常
            ErrPart=" ";
            for(int i=0; i<MAX_ARM_Row; i++)
            {
                for(int j=0; j<MAX_ARM_Col; j++)
                {
                    if(bSuckDuplicateErr[i][j])
                        bHasDuplicateErr=true;
                    if(InArmSuck.Suck[i][j].Error)
                    {
                        ErrPart+=InArmSuck.Suck[i][j].sName;
                        bSuckDuplicateErr[i][j]=true;
                        InArmSuck.Suck[i][j].Error=false;
                    }
                    else
                    {
                        bSuckDuplicateErr[i][j]=false;
                    }
                }
            }
            ShowErrorMessage("JAM0127", K_RETRY, MInArmX, bHasDuplicateErr, ErrPart);
            Task=1300;
            break;
        case 4500:                                                              //(1, 3)調整Offset
            if(bEnterOffset==true)
            {
                bEnterOffset=false;
                Task=4510;
            }
            else
            {
                Task=4100;
            }
            break;
        case 4510:
            if(MoveInArmZToPlateSafe(Task))
            {
                Task=4000;
            }
            break;
        case 4600:
            if(MoveInArmZToPlateSafe(Task))
            {
                if(iRetryCT>ArmSpeed[InArm].iRetryCT)                    //Sam 20250206 : Rotate pick error retry for setting
                {
                    ErrPart=" ";
                    for(int i=0; i<MAX_ARM_Row; i++)
                    {
                        for(int j=0; j<MAX_ARM_Col; j++)
                        {
                            if(bSuckDuplicateErr[i][j])
                                bHasDuplicateErr=true;
                            if(InArmSuck.Suck[i][j].Error)
                            {
                                ErrPart+=InArmSuck.Suck[i][j].sName;
                                bSuckDuplicateErr[i][j]=true;
                                InArmSuck.Suck[i][j].Error=false;
                            }
                            else
                            {
                                bSuckDuplicateErr[i][j]=false;
                            }
                        }
                    }
                    ret=ShowErrorMessage("JAM0112", K_RETRY|K_SKIP, MInArmX, bHasDuplicateErr, ErrPart);
                }
                else
                {
                    ret=K_RETRY;
                }

                if(ret==K_SKIP)
                {
                    for(int i=0; i<iMAX_ARM_Row; i++)
                    {
                        for(int j=0; j<iMAX_ARM_Col; j++)
                        {
                            if(W906RKIN_DUTNUM==tDutType_8)                     //kevin 20130517 add 8 site
                                j2=j;
                            else
                                j2=j*2+iKit;

                            if(bSuckDuplicateErr[i][j])                         //kevin 20130524
                            {
                                InArmSuck.PordRec[i][j2].AddErrorRecord("JAM0112");     //Steven 20161214 : Add Jam Record
                                if(CosFunction.bUseSCKART)                              //Steven 20161214 (wei) : For SCK ART
                                    fSCKART->iInputJamCnt++;

                                MOT[MInRotateKit].SetTraySingleData(j, i, NULL_IC);    //kevin 20130524
                                bSuckDuplicateErr[i][j]=false;                  //kevin 20130524
                                InArmSuck.SetItemData(i, j2, HAS_NULL_IC);
                            }
                        }
                    }

                    for(int i=0; i<MAX_ARM_Row; i++)
                    {
                        for(int j=0; j<MAX_ARM_Col; j++)
                        {
                            InArmSuck.Suck[i][j].Error=false;
                            bSuckDuplicateErr[i][j]=false;
                        }
                    }
                    Task=4300;
                    break;
                }
                Task=4000;
            }
            break;
        // --------------------------------------
    }
    return false;
}
//---------------------------------------------------------------------------
//2013-04-12    Dell :旋轉站;馬達版  底下都是馬達版的Code
//---------------------------------------------------------------------------
bool DoInArmRotateKIT()
{
    bool bRet=false;

    bWaitRotateFinish=true;
    if(iRotate_Type==eCynRotate)                                                //氣缸版
    {
        bRet=DoInArmRotateKIT_Cylinder();
    }
    else
    {
        if(tRotate.ActiveRotate)
        {
            if(iRotate_Type==eInOutArm1Motor)                                   //add One sucker with rotate
                bRet=M_DoInArmRotate_InOutArm();
            else
                bRet=M_DoInArmRotateKIT_Motor();
        }
        else
        {
            bRet=true;                                                          //不使用旋轉機構
        }
    }
    return bRet;
}
//---------------------------------------------------------------------------
//吸嘴No.3 在DUT E的中心
void CalcPosition_InArm(int &iXpos, int &iYpos, int &iXPitch, int iYPitch, int iKit)
{
    int OneSuckPitch;
    iYpos=Prod.iInArm_RotateY+(iYPitch*iRotato_In_Row);

    if(USE_IN_Y_IS_AUTO_PITCH==true && iRotato_In_Row==0)                       //JerryYang 20251218 : IN/OUT ARM支援不同模組
    {
        iYpos=iYpos-iYPitch;
    }

    if(iRotate_Type==e1MotRotate1Dut)                                           //Frank 20180512 (Steven) : modify 1 Dut 1 Motor
    {
        if(USE_PICKER_COUNT==ep16Picker)
        {
            iXPitch=iXpitchMaxX7;
        }
        else
        {
            iXPitch=iXpitchMaxX3;
        }

        iXpos=Prod.iInArm_RotateX-(iXpitchMax*(InArmSuck.Suck[iRotato_In_Row][iKit].iMyCol-iInArmXBase));
    }
    else if(iRotate_Type==e2MotRotate2Dut)                                      //wei 20190515 modify 2 Dut 2 Motor
    {                                                                           //Only Check Picker A and C Position
        if(InArmSuck.iPickCol==1)
        {
            if(USE_PICKER_COUNT==ep16Picker)
            {
                iXPitch=iXpitchMaxX7;
            }
            else
            {
                iXPitch=iXpitchMaxX3;
            }
            OneSuckPitch=iXpitchMax;
        }
        else if(bUseAxxGPicker() ||
                iCloseSiteModeFor1x4==e1x4CloseAbAc)                            //Steven 20241111 : for 1x4 close 2 site
        {
            if(USE_PICKER_COUNT==ep16Picker)
            {
                iXPitch=iRotateKIT_Pitch_X_H;
            }
            else
            {
                iXPitch=iRotateKIT_Pitch_X_H;
            }
            OneSuckPitch=iRotateKIT_Pitch_X_H/3;
        }
        else
        {
            if(USE_PICKER_COUNT==ep16Picker)
            {
                iXPitch=iRotateKIT_Pitch_X_H/2*3;
            }
            else
            {
                iXPitch=iRotateKIT_Pitch_X_H/2*3;
            }
            OneSuckPitch=iXPitch/3;
        }
        iXpos=Prod.iInArm_RotateX-(OneSuckPitch*(InArmSuck.Suck[iRotato_In_Row][iKit].iMyCol-iInArmXBase))-iRotateKIT_Pitch_X_H;
    }
    else if(W906RKIN_DUTNUM==tDutType_8)
    {
        if(iInArmType==e9045_1x2_2_14 || iInArmType==e9045_2x2_4_14 || iInArmType==e9045_2x4_4_14)    //Steven 20201028 : for rotator
        {
            iXPitch =iXpitchMaxX3;
            OneSuckPitch = iXPitch;
            if(TestIF.iTestMode==QualSite2X2 && DeviceForm.XDimension>=5000)    //Alick 20160105 (wei) modify IC>=50x50 放一個Rotate
            {
                iXpos   = Prod.iInArm_RotateX + (iRotateKIT_Pitch_X_H) - ((iRotateKIT_Pitch_X_H*2)*iKit);
                iYpos   = Prod.iInArm_RotateY + (iRotateKIT_Pitch_Y_H*iRotato_In_Row) ;                       //- (iRotateKIT_Pitch_Y_H*iKit)
            }
            else
            {
                if(TestIF.iTestMode==_8Site2X4 || TestIF.iTestMode==_16Site4X4)//Sam 20190226 : 16Site4X4
                    iXpos   = Prod.iInArm_RotateX + (iRotateKIT_Pitch_X_H) - ((iRotateKIT_Pitch_X_H*2)*iKit);
                else
                    iXpos   = Prod.iInArm_RotateX + (iRotateKIT_Pitch_X_H) - ((iRotateKIT_Pitch_X_H)*iKit);

                iYpos   = Prod.iInArm_RotateY - (iRotateKIT_Pitch_Y_H*iKit) + (iRotateKIT_Pitch_Y_H*iRotato_In_Row) ;
            }
        }
        else
        {
            iXPitch =iXpitchMaxX3;                                              //Steven 20140424 : 避免Rotate Pitch錯誤     //Isaac 20171204 (Steven) : Xpitch40->50mm, 12000->iXpitchMaxX3
            OneSuckPitch = iXPitch;
            iXpos   = Prod.iInArm_RotateX ;
            iYpos   = Prod.iInArm_RotateY ;
        }
    }
    else// if(tRotate.DutNum == tDutType_4)
    {
        //AI(W906-ROTKIT-IN) 20260807: GATE (10) -- ArmCanSuck4IC has no body in this tree
        //  (declared csystem.h:162 only).  Both calls on this line go through the macro, which
        //  resolves to golden's terminal `true` -- same choice the sibling aRotateKIT.cpp made.
        //  Golden's unparenthesised `A && B || C && D` precedence is left exactly as written.
        if((TestIF.iTestMode==_8Site2X4 || TestIF.iTestMode==_16Site4X4) && W906RKIN_ARMCANSUCK4IC(0)==false || TestIF.iTestMode==_16Site2X8 && W906RKIN_ARMCANSUCK4IC(0)==false) //Sam 20190226 : 16Site4X4
        {
            //AI(W906-ROTKIT-IN) 20260807: GATE (3) -- tRotate.RotateKit_PitchX is not on this
            //  tree's tRotateShim; macro resolves to (0.0), so this compares 0.0==40 -> false and the
            //  else arm (iXPitch = iRotateKIT_Pitch_X_H) is taken.
            if(W906RKIN_ROTKIT_PITCHX==40)                                      //20140307 wei Pitch_X 40   需*2 ，  Pitch_X 80 不需要
            {
                iXPitch = iRotateKIT_Pitch_X_H*2;                               //吸嘴1中心到吸嘴4中心 //Steven 20140305 : 要兩個Rotate Pitch //jou 2014-03-06 Rotate位置錯誤,改成一個pitch
            }
            else
            {
                iXPitch = iRotateKIT_Pitch_X_H;
            }
            OneSuckPitch = iXPitch / 3;
            iXpos   = Prod.iInArm_RotateX + iRotateKIT_Start_X_H - OneSuckPitch;
            iYpos   = Prod.iInArm_RotateY - iRotateKIT_Start_Y_H;
        }
        else
        {
            //AI(W906-ROTKIT-IN) 20260807: GATE (10) again -- with the macro at `true` this arm IS
            //  taken (golden's "suck 8, use 4 rotates" geometry).  Stated because it is the opposite
            //  selection from :813 above and both follow from the same default.
            if(W906RKIN_ARMCANSUCK4IC(0)==true)                                 //Steven 20140207 : 修正Rotate吸八顆放四顆
            {                                                                   //kevin 20130629    //吸8顆 使用4個rotate
                iXPitch=iRotateKIT_Pitch_X_H/2*3;                               //20140318 wei  Rotate PitchX=80    |40|40|40|
                OneSuckPitch = iXPitch;
                iXpos   = Prod.iInArm_RotateX -(iRotateKIT_Pitch_X_H/2)*iKit;   //20140318 wei  PitchX=80mm   移動40mm
                iYpos   = Prod.iInArm_RotateY ;
            }
            else
            {
                if(TestIF.iTestMode==QualSite2X2 ||                             //Steven 20141225 : 修正2x2使用Aa & Ac的Rotate異常
                   TestIF.iTestMode==DualSite)                                  //JerryYang 20161007 修正DualSite Rotate XPitch異常
                {
                    iXPitch = iRotateKIT_Pitch_X_H/2*3;
                }
                else
                {
                    iXPitch = iRotateKIT_Pitch_X_H/3;
                }

                OneSuckPitch = iXPitch;
                iXpos   = Prod.iInArm_RotateX + iRotateKIT_Start_X_H;
                iYpos   = Prod.iInArm_RotateY - iRotateKIT_Start_Y_H;
            }
        }
    }
}
//==============================================================================
int AutoCalculateInArmRotatePitch_Y()
{
    if(USE_IN_Y_IS_AUTO_PITCH==true)                                            //JerryYang 20251218 : IN/OUT ARM支援不同模組
    {
        if(InArmSuck.iPickRow==1)
        {
            InArmRotate_PitchY=TestIF.iARM_Y_PITCH;
        }
        else
        {
            InArmRotate_PitchY=iRotateKIT_Pitch_Y_H;                            //RogerYang 20250916 : Fix 單位錯誤，需透過RotateKIT_FormSet轉換
        }
    }
    else
    {
        InArmRotate_PitchY=TestIF.iARM_Y_PITCH;
    }
    return InArmRotate_PitchY;
}
//---------------------------------------------------------------------------
bool M_MoveInArmXY_ToRotateKIT(int iKit)
{
    //AI(W906-ROTKIT-IN) 20260807: GATE (9) -- same CheckInArmZ() gap as golden :53.
    if(W906RKIN_CHECKINARMZ())
    {
        bNeedArmZHome=true;
        InitDoArmZHome();
        return false;
    }

    if(IniConfig.bAlarmNeedServoOff)                                            //Steven 20111226 : In Arm 咖啡杯解除
    {
        if(fNote->bMyServoOffInArm)
        {
            return false;
        }
    }

    int iXPos=0;
    int iYPos=0;
    int iXVariable[X_PITCH_COUNT];
    int iYVariable  =0;
    int iMovePitchX =iXpitchMaxX3;
    int iMovePitchY =AutoCalculateInArmRotatePitch_Y();

    bool bZFlag[MAX_ARM_Row][MAX_ARM_Col];
    int iZPos[MAX_ARM_Row][MAX_ARM_Col];
    int iOffsetPos=-1;
    ZeroMemory(iXVariable, sizeof(iXVariable));
    ZeroMemory(bZFlag, sizeof(bZFlag));
    ZeroMemory(iZPos, sizeof(iZPos));

    if(IniConfig.bE34InOutArmPitchZOffsetSameOne==true)                         //jou 2010-12-20 Pitch & Z 縮減為一個
    {
        iOffsetPos=InOfsLoader;
    }
    else
    {
        iOffsetPos=InOfsRotate_In;                                              //Sam 20240808 : 修正 Pitch Offset
    }

    CalcPosition_InArm(iXPos, iYPos, iMovePitchX, iMovePitchY, iKit);

    iYVariable=GetInArmPitchY_9045(iMovePitchY, iOffsetPos);
    for(int i=0; i<X_PITCH_COUNT; i++)
        iXVariable[i]=GetInArmPitchX_9045(iMovePitchX, i, iOffsetPos);

    if(InArmContinuousMove_9045(iXPos, iYPos, iXVariable, iYVariable, bZFlag, iZPos, false))
    {
        return true;
    }
    return false;
}
//---------------------------------------------------------------------------
bool M_MoveInArmZ_ToRotateKIT_Place(int iKit)
{
    bool flag[2][4]={{true, true, true, true}, {true, true, true, true}};
    int iSuckR, iSuckC;

    if(iRotate_Type==e1MotRotate1Dut)                                           //Frank 20180512 (Steven) : modify 1 Dut 1 Motor
    {
        for(int j=iKit; j<InArmSuck.iMaxCol; j+=4)
        {
            iSuckR=iRotato_In_Row;
            iSuckC=j;

            if(InArmSuck.Item[iSuckR][iSuckC]!=NULL_IC && InArmSuck.Item[iSuckR][iSuckC]!=HAS_NULL_IC)
            {
                InArmSuck.Suck[iSuckR][iSuckC].SetNeedDestroy(true);
                flag[iSuckR][iSuckC]=MOT[InArmSuck.Suck[iSuckR][iSuckC].iMotNo].MotorMove2SpeedForPicker(Prod.iInArm_RotatePlace[iSuckR][iSuckC], &ArmSpeed[InArm]);
            }
            else
            {
                flag[iSuckR][iSuckC]=true;
            }
        }
    }
    else if(iRotate_Type==e2MotRotate2Dut)
    {
        if(InArmSuck.iPickCol==1)
        {
            iSuckR=iRotato_In_Row;
            iSuckC=iKit;

            if(InArmSuck.Item[iSuckR][iSuckC]!=NULL_IC && InArmSuck.Item[iSuckR][iSuckC]!=HAS_NULL_IC)
            {
                InArmSuck.Suck[iSuckR][iSuckC].SetNeedDestroy(true);
                flag[iSuckR][iSuckC]=MOT[InArmSuck.Suck[iSuckR][iSuckC].iMotNo].MotorMove2SpeedForPicker(Prod.iInArm_RotatePlace[iSuckR][iSuckC], &ArmSpeed[InArm]);
            }
            else
            {
                flag[iSuckR][iSuckC]=true;
            }
        }
        else if(bUseAxxGPicker() ||
                iCloseSiteModeFor1x4==e1x4CloseAbAc)                            //Steven 20241111 : for 1x4 close 2 site
        {
            for(int j=0; j<2; j++)
            {
                iSuckR=iRotato_In_Row;
                iSuckC=j*3;

                if(InArmSuck.Item[iSuckR][iSuckC]!=NULL_IC && InArmSuck.Item[iSuckR][iSuckC]!=HAS_NULL_IC)
                {
                    InArmSuck.Suck[iSuckR][iSuckC].SetNeedDestroy(true);
                    flag[iSuckR][iSuckC]=MOT[InArmSuck.Suck[iSuckR][iSuckC].iMotNo].MotorMove2SpeedForPicker(Prod.iInArm_RotatePlace[iSuckR][iSuckC], &ArmSpeed[InArm]);
                }
                else
                {
                    flag[iSuckR][iSuckC]=true;
                }
            }
        }
        else
        {
            for(int j=0; j<2; j++)
            {
                iSuckR=iRotato_In_Row;
                iSuckC=j*2+iKit;

                if(InArmSuck.Item[iSuckR][iSuckC]!=NULL_IC && InArmSuck.Item[iSuckR][iSuckC]!=HAS_NULL_IC)
                {
                    InArmSuck.Suck[iSuckR][iSuckC].SetNeedDestroy(true);
                    flag[iSuckR][iSuckC]=MOT[InArmSuck.Suck[iSuckR][iSuckC].iMotNo].MotorMove2SpeedForPicker(Prod.iInArm_RotatePlace[iSuckR][iSuckC], &ArmSpeed[InArm]);
                }
                else
                {
                    flag[iSuckR][iSuckC]=true;
                }
            }
        }
    }
    else if(W906RKIN_DUTNUM==tDutType_8)
    {
        if(iInArmType==e9045_1x2_2_14 || iInArmType==e9045_2x2_4_14 || iInArmType==e9045_2x4_4_14)    //Steven 20201028 : for rotator
        {
            if(TestIF.iTestMode==QualSite2X2 && DeviceForm.XDimension>=5000)    //wei 20151229  Xilinx 52.5x52.5 放一個Rotate  //Alick 20160105 (wei) modify IC>=50x50 放一個Rotate
            {
                for(int j=iKit; j<MAX_ARM_Col; j+=4)
                {
                    iSuckR=iRotato_In_Row;
                    iSuckC=j*2;

                    if(InArmSuck.Item[iSuckR][iSuckC]!=NULL_IC && InArmSuck.Item[iSuckR][iSuckC]!=HAS_NULL_IC)
                    {
                        InArmSuck.Suck[iSuckR][iSuckC].SetNeedDestroy(true);
                        flag[iSuckR][iSuckC]=MOT[InArmZIndex[iSuckR][iSuckC]].MotorMove2SpeedForPicker(Prod.iInArm_RotatePlace[iSuckR][iSuckC], &ArmSpeed[InArm]);  //Sam 20250207 : Rotate function 吸放也要二段速
                    }
                    else
                    {
                        flag[iSuckR][iSuckC]=true;
                    }
                }
            }
            else
            {
                for(int j=iKit; j<MAX_ARM_Col; j+=4)
                {
                    iSuckR=iRotato_In_Row;
                    if(TestIF.iTestMode==_8Site2X4 || TestIF.iTestMode==_16Site4X4) //Sam 20190226 : 16Site4X4
                        iSuckC=j*3;
                    else
                        iSuckC=j*2;

                    if(InArmSuck.Item[iSuckR][iSuckC]!=NULL_IC && InArmSuck.Item[iSuckR][iSuckC]!=HAS_NULL_IC)
                    {
                        InArmSuck.Suck[iSuckR][iSuckC].SetNeedDestroy(true);
                        flag[iSuckR][iSuckC]=MOT[InArmZIndex[iSuckR][iSuckC]].MotorMove2SpeedForPicker(Prod.iInArm_RotatePlace[iSuckR][iSuckC], &ArmSpeed[InArm]);
                    }
                    else
                    {
                        flag[iSuckR][iSuckC]=true;
                    }
                }
            }
        }
        else
        {
            for(int i=0; i<MAX_ARM_Row; i++)
            {
                for(int j=iKit; j<MAX_ARM_Col; j++)
                {
                    iSuckR=i;
                    iSuckC=j;
                    if(InArmSuck.Item[iSuckR][iSuckC]!=NULL_IC && InArmSuck.Item[iSuckR][iSuckC]!=HAS_NULL_IC)
                    {
                        InArmSuck.Suck[iSuckR][iSuckC].SetNeedDestroy(true);
                        flag[iSuckR][iSuckC]=MOT[InArmZIndex[iSuckR][iSuckC]].MotorMove2SpeedForPicker(Prod.iInArm_RotatePlace[iSuckR][iSuckC], &ArmSpeed[InArm]);
                    }
                    else
                    {
                        flag[iSuckR][iSuckC]=true;
                    }
                }
            }
        }
    }
    else
    {
        for(int i=0; i<MAX_ARM_Row; i++)
        {
            for(int j=iKit; j<MAX_ARM_Col; j+=i2x2Suck)   //(0, 2) & (1, 3)
            {
                iSuckR=i;
                iSuckC=j;
                if(InArmSuck.Item[iSuckR][iSuckC]!=NULL_IC && InArmSuck.Item[iSuckR][iSuckC]!=HAS_NULL_IC)
                {
                    InArmSuck.Suck[iSuckR][iSuckC].SetNeedDestroy(true);
                    flag[iSuckR][iSuckC]=MOT[InArmZIndex[iSuckR][iSuckC]].MotorMove2SpeedForPicker(Prod.iInArm_RotatePlace[iSuckR][iSuckC], &ArmSpeed[InArm]);
                }
                else
                {
                    flag[iSuckR][iSuckC]=true;
                }
            }
        }
    }

    for(int i=0; i<MAX_ARM_Row; i++)
    {
        for(int j=0; j<MAX_ARM_Col; j++)
        {
            if(flag[i][j]==false)
                return false;
        }
    }
    return true;
}
//------------------------------------------------------------------------------
bool M_MoveInArmZ_ToRotateKIT_Pick(int iKit, int PickAngle)
{
    bool flag[2][4]={{true, true, true, true}, {true, true, true, true}};
    int iSuckR=0, iSuckC=0, iTrayY=0, iTrayX=0;

    if(iRotate_Type==e1MotRotate1Dut)                                           //Frank 20180512 (Steven) : modify 1 Dut 1 Motor
    {
        for(int j=iKit; j<InArmSuck.iMaxCol; j+=4)
        {
            iTrayY=0;
            iTrayX=0;
            iSuckR=iRotato_In_Row;
            iSuckC=j;
            if(MOT[MInRotateKit].Tray.Data[iTrayX][iTrayY]!=NULL_IC &&
               MOT[MInRotateKit].Tray.Data[iTrayX][iTrayY]!=HAS_NULL_IC)
            {
                if(CheckInRotateAnglePostion(iSuckR, iSuckC, iTrayX, iTrayY))
                {
                    InArmSuck.Suck[iSuckR][iSuckC].SetNeedSuck(true);
                    flag[iSuckR][iSuckC]=MOT[InArmSuck.Suck[iSuckR][iSuckC].iMotNo].MotorMove2SpeedForPicker(Prod.iInArm_RotatePick[iSuckR][iSuckC], &ArmSpeed[InArm]);
                }
            }
            else
            {
                flag[iSuckR][iSuckC]=true;
            }
        }
    }
    else if(iRotate_Type==e2MotRotate2Dut)                                      //wei 20190515 modify 2 Dut 2 Motor
    {
        if(InArmSuck.iPickCol==1)
        {
            for(int j=iKit; j<InArmSuck.iMaxCol; j+=4)
            {
                iTrayY=0;
                iTrayX=0;
                iSuckR=iRotato_In_Row;
                iSuckC=j;
                if(MOT[MInRotateKit].Tray.Data[iTrayX][iTrayY]!=NULL_IC &&
                   MOT[MInRotateKit].Tray.Data[iTrayX][iTrayY]!=HAS_NULL_IC)
                {
                    if(CheckInRotateAnglePostion(iSuckR, iSuckC, iTrayX, iTrayY))
                    {
                        InArmSuck.Suck[iSuckR][iSuckC].SetNeedSuck(true);
                        flag[iSuckR][iSuckC]=MOT[InArmSuck.Suck[iSuckR][iSuckC].iMotNo].MotorMove2SpeedForPicker(Prod.iInArm_RotatePick[iSuckR][iSuckC], &ArmSpeed[InArm]);
                    }
                }
                else
                {
                    flag[iSuckR][iSuckC]=true;
                }
            }
        }
        else if(bUseAxxGPicker() ||
                iCloseSiteModeFor1x4==e1x4CloseAbAc)                            //Steven 20241111 : for 1x4 close 2 site
        {
            for(int j=0; j<2; j++)
            {
                iTrayY=0;
                iTrayX=j;
                iSuckR=iRotato_In_Row;
                iSuckC=j*3;
                if(MOT[MInRotateKit].Tray.Data[iTrayX][iTrayY]!=NULL_IC &&
                   MOT[MInRotateKit].Tray.Data[iTrayX][iTrayY]!=HAS_NULL_IC)
                {
                    if(CheckInRotateAnglePostion(iSuckR, iSuckC, iTrayX, iTrayY))
                    {
                        InArmSuck.Suck[iSuckR][iSuckC].SetNeedSuck(true);
                        flag[iSuckR][iSuckC]=MOT[InArmSuck.Suck[iSuckR][iSuckC].iMotNo].MotorMove2SpeedForPicker(Prod.iInArm_RotatePick[iSuckR][iSuckC], &ArmSpeed[InArm]);
                    }
                }
                else
                {
                    flag[iSuckR][iSuckC]=true;
                }
            }
        }
        else
        {
            for(int j=0; j<2; j++)
            {
                iTrayY=0;
                iTrayX=j;
                iSuckR=iRotato_In_Row;
                iSuckC=j*2+iKit;
                if(MOT[MInRotateKit].Tray.Data[iTrayX][iTrayY]!=NULL_IC &&
                   MOT[MInRotateKit].Tray.Data[iTrayX][iTrayY]!=HAS_NULL_IC)
                {
                    if(CheckInRotateAnglePostion(iSuckR, iSuckC, iTrayX, iTrayY))
                    {
                        InArmSuck.Suck[iSuckR][iSuckC].SetNeedSuck(true);
                        flag[iSuckR][iSuckC]=MOT[InArmSuck.Suck[iSuckR][iSuckC].iMotNo].MotorMove2SpeedForPicker(Prod.iInArm_RotatePick[iSuckR][iSuckC], &ArmSpeed[InArm]);
                    }
                }
                else
                {
                    flag[iSuckR][iSuckC]=true;
                }
            }
        }
    }
    else if(W906RKIN_DUTNUM==tDutType_8)
    {
        if(iInArmType==e9045_1x2_2_14 || iInArmType==e9045_2x2_4_14 || iInArmType==e9045_2x4_4_14)    //Steven 20201028 : for rotator
        {
            if(TestIF.iTestMode==QualSite2X2 && DeviceForm.XDimension>=5000)    //wei 20151229  Xilinx 52.5x52.5 放一個Rotate  //Alick 20160105 (wei) modify IC>=50x50 放一個Rotate
            {
                for(int j=iKit; j<MAX_ARM_Col; j+=4)                            //(0, 2) & (1, 3)
                {
                    iSuckR=iRotato_In_Row;
                    iSuckC=j*2;
                    if(MOT[MInRotateKit].Tray.Data[1][0]!=NULL_IC && MOT[MInRotateKit].Tray.Data[1][0]!=HAS_NULL_IC)
                    {
                        if(Prod.RotateDutDate[0][0][1]==PickAngle)              //Ifor 20170412 (Steven) Modify 角度已算過不需在減掉原本角度
                        {
                            InArmSuck.Suck[iSuckR][iSuckC].SetNeedSuck(true);
                            flag[iSuckR][iSuckC]=MOT[InArmZIndex[iSuckR][iSuckC]].MotorMove2SpeedForPicker(Prod.iInArm_RotatePick[iSuckR][iSuckC], &ArmSpeed[InArm]);
                        }
                    }
                    else
                    {
                        flag[iSuckR][iSuckC]=true;
                    }
                }
            }
            else
            {
                for(int j=iKit; j<MAX_ARM_Col; j+=4)                            //(0, 2) & (1, 3)
                {
                    iSuckR=iRotato_In_Row;
                    if(TestIF.iTestMode==_8Site2X4 || TestIF.iTestMode==_16Site4X4) //Sam 20190226 : 16Site4X4
                        iSuckC=j*3;
                    else
                        iSuckC=j*2;
                    if(MOT[MInRotateKit].Tray.Data[j+1][iKit]!=NULL_IC && MOT[MInRotateKit].Tray.Data[j+1][iKit]!=HAS_NULL_IC)
                    {
                        if(Prod.RotateDutDate[0][iKit][j+1]==PickAngle)         //Ifor 20170412 (Steven) Modify 角度已算過不需在減掉原本角度
                        {
                            InArmSuck.Suck[iSuckR][iSuckC].SetNeedSuck(true);
                            flag[iSuckR][iSuckC]=MOT[InArmZIndex[iSuckR][iSuckC]].MotorMove2SpeedForPicker(Prod.iInArm_RotatePick[iSuckR][iSuckC], &ArmSpeed[InArm]);
                        }
                    }
                    else
                    {
                        flag[iSuckR][iSuckC]=true;
                    }
                }
            }
        }
        else
        {
            for(int i=0; i<MAX_ARM_Row; i++)
            {
                for(int j=0; j<MAX_ARM_Col; j++)   //(0, 2) & (1, 3)
                {
                    iSuckR=i;
                    iSuckC=j;
                    if(MOT[MInRotateKit].Tray.Data[j][i]!=NULL_IC && MOT[MInRotateKit].Tray.Data[j][i]!=HAS_NULL_IC)
                    {
                        if(Prod.RotateDutDate[0][i][j]==PickAngle) //Ifor 20170412 (Steven) Modify 角度已算過不需在減掉原本角度
                        {
                            InArmSuck.Suck[iSuckR][iSuckC].SetNeedSuck(true);
                            flag[iSuckR][iSuckC]=MOT[InArmZIndex[iSuckR][iSuckC]].MotorMove2SpeedForPicker(Prod.iInArm_RotatePick[iSuckR][iSuckC], &ArmSpeed[InArm]);
                        }
                    }
                    else
                    {
                        flag[iSuckR][iSuckC]=true;
                    }
                }
            }
        }
    }
    else
    {
        if(TestIF.iTestMode==QualSite1X4 || TestIF.iTestMode==_8Site1X4)        //ChungHung 20150528 add for 海思 _8Site1x4
        {
            int i=iKit;
            for(int j=0; j<2; j++)   //(0, 2) & (1, 3)
            {
                //Kit(0, 1) -> Suck(0, 2) or Suck(1, 3)
                iSuckR=0;
                iSuckC=j*i2x2Suck+iKit;
                if(MOT[MInRotateKit].Tray.Data[j][i]!=NULL_IC && MOT[MInRotateKit].Tray.Data[j][i]!=HAS_NULL_IC)
                {
                    if(Prod.RotateDutDate[0][i][j]==PickAngle)                  //Ifor 20170412 (Steven) Modify 角度已算過不需在減掉原本角度
                    {
                        InArmSuck.Suck[iSuckR][iSuckC].SetNeedSuck(true);
                        flag[iSuckR][iSuckC]=MOT[InArmZIndex[iSuckR][iSuckC]].MotorMove2SpeedForPicker(Prod.iInArm_RotatePick[iSuckR][iSuckC], &ArmSpeed[InArm]);
                    }
                }
                else
                {
                    flag[iSuckR][iSuckC]=true;
                }
            }
        }
        else
        {
            //Steven 20170329 (wei) : Add individual rotate motor
            //==>
            //AI(W906-ROTKIT-IN) 20260807: GATE (7) -- TFrmRotate::bShowRotateBySite is not on
            //  forms/fRotate.h's stand-in; macro resolves to false, so this per-site angle path is
            //  present but not selected and control falls to the golden else at :1331.
            if(W906RKIN_SHOWROTATEBYSITE==true)                                 //wei 20190515 modify 2 Dut 2 Motor
            {
                for(int i=0; i<2; i++)
                {
                    for(int j=0; j<2; j++)   //(0, 2) & (1, 3)
                    {
                        //Kit(0, 1) -> Suck(0, 2) or Suck(1, 3)
                        iSuckR=i;
                        iSuckC=j*i2x2Suck+iKit;
                        if(MOT[MInRotateKit].Tray.Data[j][i]!=NULL_IC && MOT[MInRotateKit].Tray.Data[j][i]!=HAS_NULL_IC)
                        {
                            if(Prod.RotateDutDate[0][i][j]==MOT[MInRotateKit].Tray.iCurrRotAng[j][i])
                            {
                                InArmSuck.Suck[iSuckR][iSuckC].SetNeedSuck(true);
                                flag[iSuckR][iSuckC]=MOT[InArmZIndex[iSuckR][iSuckC]].MotorMove2SpeedForPicker(Prod.iInArm_RotatePick[iSuckR][iSuckC], &ArmSpeed[InArm]);
                            }
                        }
                        else
                        {
                            flag[iSuckR][iSuckC]=true;
                        }
                    }
                }
            }
            //<==
            //Steven 20170329 (wei) : Add individual rotate motor
            else
            {
                for(int i=0; i<2; i++)
                {
                    for(int j=0; j<2; j++)   //(0, 2) & (1, 3)
                    {
                        //Kit(0, 1) -> Suck(0, 2) or Suck(1, 3)
                        iSuckR=i;
                        iSuckC=j*i2x2Suck+iKit;
                        if(MOT[MInRotateKit].Tray.Data[j][i]!=NULL_IC && MOT[MInRotateKit].Tray.Data[j][i]!=HAS_NULL_IC)
                        {
                            if(Prod.RotateDutDate[0][i][j]==PickAngle)          //Ifor 20170412 (Steven) Modify 角度已算過不需在減掉原本角度
                            {
                                InArmSuck.Suck[iSuckR][iSuckC].SetNeedSuck(true);
                                flag[iSuckR][iSuckC]=MOT[InArmZIndex[iSuckR][iSuckC]].MotorMove2SpeedForPicker(Prod.iInArm_RotatePick[iSuckR][iSuckC], &ArmSpeed[InArm]);
                            }
                        }
                        else
                        {
                            flag[iSuckR][iSuckC]=true;
                        }
                    }
                }
            }
        }
    }

    for(int i=0; i<MAX_ARM_Row; i++)
    {
        for(int j=0; j<MAX_ARM_Col; j++)
        {
            if(flag[i][j]==false)
                return false;
        }
    }
    return true;
}
//---------------------------------------------------------------------------
bool M_DoInArmRotate_InOutArm()                                                 //add One sucker with rotate
{
    int &Task=iInArmRotateKit;
    int ret;
    static int rCount;

    switch(Task)
    {
        case 1:
            rCount=1;
//            DoRotate_In(rCount, true);
            ret=SetMotorResolution(InAngle45, InAngle90, true);
            Task=100;
            break;
        case 100:
            ret=DoRotate_In(rCount);

            if(ret==1)
            {
                return true;
            }
            else if(ret==-999)                                                  //Error
            {
                Task=2000;
            }
            break;
        case 2000:
            ret=ShowErrorMessage("JAM0453", K_RETRY, MInRotateKit);             //kevin 20130524

            if(ret==K_RETRY)
            {
                Task=2100;
            }
            break;
        case 2100:
            FrmRotate->bRotateInHome=true;                                      //kevin 20130706
            FrmRotate->InitialInRotateHome();                                   //Steven 20170329 (wei) : Add individual rotate motor
            Task=2200;
            break;
        case 2200:
            if(FrmRotate->bRotateInHome==false)                                 //kevin 20130706
            {
                //AI(W906-ROTKIT-IN) 20260807: GATE (8) -- TFrmRotate::SetInRotateSpeed is not on
                //  forms/fRotate.h's stand-in (a METHOD, so no macro can stand in).  Golden statement
                //  parked verbatim; `Task=100` below stays ACTIVE so the JAM0453 recovery still
                //  completes -- only the re-application of ArmSpeed[InRotate] is lost.
#if 0 // TODO(W7): GATE (8) -- golden :1411 TFrmRotate::SetInRotateSpeed -- absent from forms/fRotate.h stand-in
                FrmRotate->SetInRotateSpeed(ArmSpeed[InRotate].iACDCBodySP, ArmSpeed[InRotate].iBodySP);    //Steven 20170329 (wei) : Add individual rotate motor
#endif
                Task=100;
            }
            break;
    }
    return false;
}
//---------------------------------------------------------------------------
//------------------------------------------------------------------------------
int GotInRotateKitForSingle(int &iKit, int &iCount, bool bRelease)
{
    int Task=1000;
    if(bRelease)
    {
        Task=1000;
        if(InArmSuck.Item[iRotato_In_Row][0]!=NULL_IC && InArmSuck.Item[iRotato_In_Row][0]!=HAS_NULL_IC && iCount==0)
        {
            iKit=0;
            Task=1100;
        }
        else if((InArmSuck.Item[iRotato_In_Row][0]==NULL_IC || InArmSuck.Item[iRotato_In_Row][0]==HAS_NULL_IC) && iCount==0)
        {
            iCount=1;
            Task=1000;
        }
        else if(InArmSuck.Item[iRotato_In_Row][1]!=NULL_IC && InArmSuck.Item[iRotato_In_Row][1]!=HAS_NULL_IC && iCount==1)
        {
            iKit=1;
            Task=1100;
        }
        else if((InArmSuck.Item[iRotato_In_Row][1]==NULL_IC || InArmSuck.Item[iRotato_In_Row][1]==HAS_NULL_IC) && iCount==1)
        {
            iCount=2;
            Task=1000;
        }
        else if(InArmSuck.Item[iRotato_In_Row][2]!=NULL_IC && InArmSuck.Item[iRotato_In_Row][2]!=HAS_NULL_IC && iCount==2)
        {
            iKit=2;
            Task=1100;
        }
        else if((InArmSuck.Item[iRotato_In_Row][2]==NULL_IC || InArmSuck.Item[iRotato_In_Row][2]==HAS_NULL_IC) && iCount==2)
        {
            iCount=3;
            Task=1000;
        }
        else if(InArmSuck.Item[iRotato_In_Row][3]!=NULL_IC && InArmSuck.Item[iRotato_In_Row][3]!=HAS_NULL_IC && iCount==3)
        {
            iKit=3;
            Task=1100;
        }
        else if(iRotato_In_Row==1)
        {
            Task=6000;
        }
        else
        {
            iRotato_In_Row=1;
            Task=1000;
        }
    }
    else
    {
        Task=1;
        if(iRotato_In_Row==0)
        {
            if(iCount==0)
            {
                iKit=1;
                iCount=1;
            }
            else if(iCount==1)
            {
                iKit=2;
                iCount=2;
            }
            else if(iCount==2)
            {
                iKit=3;
                iCount=3;
            }
            else
            {
                iCount=0;
                iRotato_In_Row=1;
                iKit=0;
            }
            Task=10;
        }
        else
        {
            if(iCount==0)
            {
                iKit=1;
                iCount=1;
                Task=10;
            }
            else if(iCount==1)
            {
                iKit=2;
                iCount=2;
                Task=10;
            }
            else if(iCount==2)
            {
                iKit=3;
                iCount=3;
                Task=10;
            }
            else
            {
                iCount=0;
                iRotato_In_Row=0;
                iKit=0;
                MOT[MInRotateKit].SetHasNullIcToNullIc();               //Frank 20170822 (wei): 清空Rotate資料
                Task=1;
            }
        }
    }
    return Task;
}
//------------------------------------------------------------------------------
int GotInRotateKitForAxxG(int &iKit, int &iCount, bool bRelease)
{
    int Task=1000;
    if(bRelease)
    {
        if(((InArmSuck.Item[iRotato_In_Row][0]!=NULL_IC && InArmSuck.Item[iRotato_In_Row][0]!=HAS_NULL_IC) ||
            (InArmSuck.Item[iRotato_In_Row][3]!=NULL_IC && InArmSuck.Item[iRotato_In_Row][3]!=HAS_NULL_IC)) &&
           iCount==0)
        {
            iKit=0;
            Task=1100;
        }
        else if((InArmSuck.Item[iRotato_In_Row][0]==NULL_IC || InArmSuck.Item[iRotato_In_Row][0]==HAS_NULL_IC ||
                 InArmSuck.Item[iRotato_In_Row][3]==NULL_IC || InArmSuck.Item[iRotato_In_Row][3]==HAS_NULL_IC) &&
                iCount==0)
        {
            iCount=1;
            Task=1000;
        }
        else if(iRotato_In_Row==1)
        {
            Task=6000;
        }
        else
        {
            iRotato_In_Row=1;
            Task=1000;
        }
    }
    else
    {
        Task=10;
        if(iRotato_In_Row==0)
        {
            iCount=0;
            iRotato_In_Row=1;
            iKit=0;
            Task=10;
        }
        else
        {
            iCount=0;
            iRotato_In_Row=0;
            iKit=0;
            MOT[MInRotateKit].SetHasNullIcToNullIc();                           //Frank 20170822 (wei): 清空Rotate資料
            Task=1;
        }
    }
    return Task;
}
//------------------------------------------------------------------------------
int GotInRotateKitForAxEx_2RotMot(int &iKit, int &iCount, bool bRelease)
{
    int Task=1000;
    if(bRelease)
    {
        Task=1000;
        if(InArmSuck.Item[iRotato_In_Row][0]!=NULL_IC && InArmSuck.Item[iRotato_In_Row][0]!=HAS_NULL_IC &&
           InArmSuck.Item[iRotato_In_Row][2]!=NULL_IC && InArmSuck.Item[iRotato_In_Row][2]!=HAS_NULL_IC &&
           iCount==0)
        {
            iKit=0;
            Task=1100;
        }
        else if((InArmSuck.Item[iRotato_In_Row][0]==NULL_IC || InArmSuck.Item[iRotato_In_Row][0]==HAS_NULL_IC ||
                 InArmSuck.Item[iRotato_In_Row][2]==NULL_IC || InArmSuck.Item[iRotato_In_Row][2]==HAS_NULL_IC) &&
                iCount==0)
        {
            iCount=1;
            Task=1000;
        }
        else if(InArmSuck.Item[iRotato_In_Row][1]!=NULL_IC && InArmSuck.Item[iRotato_In_Row][1]!=HAS_NULL_IC &&
                InArmSuck.Item[iRotato_In_Row][3]!=NULL_IC && InArmSuck.Item[iRotato_In_Row][3]!=HAS_NULL_IC &&
                iCount==1)
        {
            iKit=1;
            Task=1100;
        }
        //AI(W906-ROTKIT-IN) 20260807: GOLDEN BUG preserved VERBATIM (golden :1610-1616) -- this
        //  arm tests `iCount==0`, but the two arms above already cover every iCount==0 case, so it
        //  is unreachable; it was plainly meant to read iCount==1.  Not fixed.
        else if((InArmSuck.Item[iRotato_In_Row][1]==NULL_IC || InArmSuck.Item[iRotato_In_Row][1]==HAS_NULL_IC ||
                 InArmSuck.Item[iRotato_In_Row][3]==NULL_IC || InArmSuck.Item[iRotato_In_Row][3]==HAS_NULL_IC) &&
                iCount==0)
        {
            iCount=2;
            Task=1000;
        }
        else if(iRotato_In_Row==1)
        {
            Task=6000;
        }
        else
        {
            iRotato_In_Row=1;
            Task=1000;
        }
    }
    else
    {
        Task=1;
        if(iRotato_In_Row==0)
        {
            if(iCount==0)
            {
                iKit=1;
                iCount=1;
            }
            else
            {
                iCount=0;
                iRotato_In_Row=1;
                iKit=0;
            }
            Task=10;
        }
        else
        {
            if(iCount==0)
            {
                iKit=1;
                iCount=1;
                Task=10;
            }
            else
            {
                iCount=0;
                iRotato_In_Row=0;
                iKit=0;
                MOT[MInRotateKit].SetHasNullIcToNullIc();                       //Frank 20170822 (wei): 清空Rotate資料
                Task=1;
            }
        }
    }
    return Task;
}
//------------------------------------------------------------------------------
bool M_DoInArmRotateKIT_Motor()
{
    int &Task=iInArmRotateKit;
    int j2, j3;
    int iSuckR, iSuckC, iTrayY, iTrayX;

    static int iRetryCT=0;
    static int iKit=0;
    static bool bResetKit0=true;                                                //Sam 20221209 : 修正 4Dut 模式 InArm CleanOut時最後剛好只放 Kit1 吸取錯問題
    static int rCount=0;
    bool flag=true;
    int iRotateCount=0;                                                         //wei 20141230 Rotate旋轉問題 for 大IC單獨吸
    int ret;
    static int iCount=0;
    bool bHasDuplicateErr=false;
    static bool bSuckDuplicateErr[MAX_ARM_Row][MAX_ARM_Col]={false};            //Steven 20091218 : Avoid duplicate message
    static bool bRotateMoveOk=false, bInXYMoveOk=false;
    AnsiString ErrPart="";

    int iMAX_ARM_Row, iMAX_ARM_Col;
    if(W906RKIN_DUTNUM==tDutType_8)                                             //kevin 20130517 add 8 site
    {
        iMAX_ARM_Row=MAX_ARM_Row;
        iMAX_ARM_Col=MAX_ARM_Col;
    }
    else
    {
        iMAX_ARM_Row=MAX_ARM_Row;
        iMAX_ARM_Col=MAX_ARM_Col/2;
    }

    switch(Task)
    {
        case 1:
            //RotateKIT_FormSet(bool bInput,int iStartX, int iStartY, int iPitchX, int iPitchY)
            //AI(W906-ROTKIT-IN) 20260807: GATES (3)+(4) -- tRotate.RotateKit_PitchX/_PitchY are
            //  not on the tRotateShim; both macros resolve to (0.0), so this call sets
            //  iRotateKIT_Pitch_X_H = iRotateKIT_Pitch_Y_H = 0 instead of the taught pitch.  The
            //  golden `double * 100` arithmetic and the implicit truncation to the int parameter
            //  are unchanged.  See banner GATE (3)/(4) for the behaviour delta.
            RotateKIT_FormSet(         true,       0,        0,   (W906RKIN_ROTKIT_PITCHX*100), (W906RKIN_ROTKIT_PITCHY*100));
            InitSuckState();

            if(bContinusRotate)
            {
                bContinusRotate = false;
                if(MOT[MInRotateKit].FullIC())
                    Task=2500;
                else
                    Task=4300;
            }
            else
            {
                iKit=0;                                                         //初始化
                Task=10;
                iCount=0;
                if(W906RKIN_DUTNUM==tDutType_4)                                 //Sam 20221209 : 修正 4Dut 模式 InArm CleanOut時最後剛好只放 Kit1 吸取錯問題
                {
                    if((InArmSuck.Item[0][0+iKit]!=NULL_IC         && InArmSuck.Item[0][0+iKit]!=HAS_NULL_IC)          ||
                       (InArmSuck.Item[0][i2x2Suck+iKit]!=NULL_IC  && InArmSuck.Item[0][i2x2Suck+iKit]!=HAS_NULL_IC)   ||
                       (InArmSuck.Item[1][0+iKit]!=NULL_IC         && InArmSuck.Item[1][0+iKit]!=HAS_NULL_IC)          ||
                       (InArmSuck.Item[1][i2x2Suck+iKit]!=NULL_IC  && InArmSuck.Item[1][i2x2Suck+iKit]!=HAS_NULL_IC))
                    {
                        bResetKit0=true;
                    }
                    else
                    {
                        bResetKit0=false;
                    }
                }
            }
            bRotateMoveOk=false;
            bInXYMoveOk=false;
            MoveInRotateToDegreeAtSameTime(0, true);                            //Steven 20170504 (wei) : 針對獨立馬達旋轉站, 一起旋轉到特定角度
            break;
        case 10:                                                                //jou 2013-05-08 不等旋轉完成
            ret=SetMotorResolution(InAngle45, InAngle90, true);                 //Steven 20131202   //kevin 20131017
            Task=100;
//            break;
        case 100:
            MOT[MInRotateKit].SetTray(NULL_IC, __FUNC__);
            ZeroMemory(bSuckDuplicateErr, sizeof(bSuckDuplicateErr));
            Task=1000;
//            break;
        case 1000:                                                              //確認有沒有要放IC
            if(iRotate_Type==e1MotRotate1Dut)                                   //Frank 20180512 (Steven) : modify 1 Dut 1 Motor
            {
                Task=GotInRotateKitForSingle(iKit, iCount, true);
            }
            else if(iRotate_Type==e2MotRotate2Dut)
            {
                if(InArmSuck.iPickCol==1)
                {
                    Task=GotInRotateKitForSingle(iKit, iCount, true);
                }
                else if(bUseAxxGPicker() ||
                        iCloseSiteModeFor1x4==e1x4CloseAbAc)                    //Steven 20241111 : for 1x4 close 2 site
                {
                    Task=GotInRotateKitForAxxG(iKit, iCount, true);
                }
                else
                {
                    Task=GotInRotateKitForAxEx_2RotMot(iKit, iCount, true);
                }
            }
            else if(W906RKIN_DUTNUM==tDutType_8)                                //kevin 20130517 add 8 site
            {
                if(iInArmType==e9045_1x2_2_14 || iInArmType==e9045_2x2_4_14 || iInArmType==e9045_2x4_4_14)    //Steven 20201028 : for rotator
                {
                    if(TestIF.iTestMode==QualSite2X2 && DeviceForm.XDimension>=5000)    //wei 20151229  Xilinx 52.5x52.5 放一個Rotate  //Alick 20160105 (wei) modify IC>=50x50 放一個Rotate
                    {
                        if(InArmSuck.Item[iRotato_In_Row][0]!=NULL_IC && InArmSuck.Item[iRotato_In_Row][0]!=HAS_NULL_IC && iCount==0)
                        {
                            iKit=0;
                            Task=1100;
                        }
                        else if((InArmSuck.Item[iRotato_In_Row][0]==NULL_IC || InArmSuck.Item[iRotato_In_Row][0]==HAS_NULL_IC) && iCount==0)
                        {
                            iCount=1;
                            Task=1000;
                        }
                        else if(InArmSuck.Item[iRotato_In_Row][2]!=NULL_IC && InArmSuck.Item[iRotato_In_Row][2]!=HAS_NULL_IC && iCount==1)   //(1,3)
                        {
                            iKit=1;
                            Task=1100;
                        }
                        else if(iRotato_In_Row==1)
                        {
                            Task=6000;
                        }
                        else
                        {
                            iRotato_In_Row=1;
                            Task=1000;
                        }
                    }
                    else
                    {
                        if(InArmSuck.Item[iRotato_In_Row][0]!=NULL_IC && InArmSuck.Item[iRotato_In_Row][0]!=HAS_NULL_IC)
                        {
                            iKit=0;
                            Task=1100;
                        }
                        else if(((TestIF.iTestMode==QualSite2X2 || TestIF.iTestMode==DualSite)   && InArmSuck.Item[iRotato_In_Row][2]!=NULL_IC && InArmSuck.Item[iRotato_In_Row][2]!=HAS_NULL_IC) ||  //(1,3)
                                ((TestIF.iTestMode==_8Site2X4   || TestIF.iTestMode==_16Site4X4) && InArmSuck.Item[iRotato_In_Row][3]!=NULL_IC && InArmSuck.Item[iRotato_In_Row][3]!=HAS_NULL_IC)) //Sam 20190226 : 16Site4X4 //(1,4)
                        {
                            iKit=1;
                            Task=1100;
                        }
                        else if(iRotato_In_Row==1)
                        {
                            Task=6000;
                        }
                        else
                        {
                            iRotato_In_Row=1;
                            Task=1000;
                        }
                    }
                }
                else
                {
                    //AI(W906-ROTKIT-IN) 20260807: GATE (11) -- TMyKitSuck::HAS_NO_IC is not on this tree's
                    //  mirror; macro resolves to `true` (golden's method is mis-named: it means "some nozzle
                    //  is NOT NULL_IC").  Unreachable today anyway -- the enclosing arm needs DutNum==
                    //  tDutType_8, which GATE (2) leaves unselected.
                    if(W906RKIN_HAS_NO_IC())                                    //確認沒有空IC
                        Task=1100;
                    else
                        Task=2000;
                }
            }
            else
            {
                if((InArmSuck.Item[0][0+iKit]!=NULL_IC          && InArmSuck.Item[0][0+iKit]!=HAS_NULL_IC) ||
                   (InArmSuck.Item[0][i2x2Suck+iKit]!=NULL_IC   && InArmSuck.Item[0][i2x2Suck+iKit]!=HAS_NULL_IC) ||
                   (InArmSuck.Item[1][0+iKit]!=NULL_IC          && InArmSuck.Item[1][0+iKit]!=HAS_NULL_IC) ||
                   (InArmSuck.Item[1][i2x2Suck+iKit]!=NULL_IC   && InArmSuck.Item[1][i2x2Suck+iKit]!=HAS_NULL_IC))
                {
                    Task=1100;
                }
                else
                {
                    Task=2000;
                }
            }
            break;
         case 1100:                                                             //wei 20160106 增加放Rotate前偵測
            if(CheckRotateSensor(true))
            {
                Task=1150;
            }
            else
            {
                Task=1120;
            }
            break;
        case 1120:
            if(MoveInArm2XYToWait()==false)                                     //如果要移開,而且還沒移到定位
            {
                return false;                                                   //先離開等In Arm
            }
            Task=1130;
            break;
        case 1130:                                                              //In Rotate 疊料
            ShowErrorMessage("JAM0452", K_RETRY, MInRotateKit);
            Task=1100;
            break;
        case 1150:                                                              //X, Y移動到放的位置
            bRotateMoveOk=false;
            bInXYMoveOk=false;
            MoveInRotateToDegreeAtSameTime(0, true);                            //Steven 20170504 : 針對獨立馬達旋轉站, 一起旋轉到特定角度
            Task=1160;
        case 1160:
            if(bRotateMoveOk==false)
                bRotateMoveOk=MoveInRotateToDegreeAtSameTime(0);                //Steven 20170504 : 針對獨立馬達旋轉站, 一起旋轉到特定角度

            if(bInXYMoveOk==false)
                bInXYMoveOk=M_MoveInArmXY_ToRotateKIT(iKit);

            if(CheckInArmSuckICFallDownToHasNullIC(false)==true)                //Steven 20110516 : 修改成整合式Alarm
            {
                Task=1170;                                                      //JerryYang 20200422 修正掉料可能檢查不出來的問題, 檢查到有掉料要重新再確認全部吸嘴
                break;
            }

            if(bRotateMoveOk && bInXYMoveOk)
            {
                if(iRotate_Type==e1MotRotate1Dut ||                             //Frank 20180512 (Steven) : modify 1 Dut 1 Motor
                   iRotate_Type==e2MotRotate2Dut)
                {
                }
                else
                {
                    for(int i=0; i<iMAX_ARM_Row; i++)
                    {
                        for(int j=0; j<iMAX_ARM_Col; j++)
                        {
                            if(TestIF.iTestMode==QualSite2X2)                   //Alick 20160711 修改2x2放Rotate異常，需破真空建立資料位置問題
                            {
                                if(TestIF.dSiteXPitch>iXpitchMaxX2)             //Alick 20160711 修改對應2x2_4_14
                                {
                                    j3=j*3;
                                }
                                else if(TestIF.dSiteXPitch>iXpitchMinX2)        //Alick 20160711 修改對應2x2_4
                                {
                                    j3=j*2;
                                }
                                else                                            //Alick 20160711 修改對應2x2_4_12
                                {
                                    j3=j;
                                }
                                InArmSuck.Suck[i][j3].SetNeedDestroy(true);     //Alick 20160711 修改2x2放Rotate異常問題
                            }
                            else
                            {
                                InArmSuck.Suck[i][j].SetNeedDestroy(true);      //kevin 20130520
                            }
                        }
                    }
                }
                bIn_XYMoveFinishOnRotationKit=true;                             //Sam 20240726 : 修正移動完 XY 準備放 IC 到 RotationKit 時 Home
                Task=1200;
            }
            break;
        case 1170:
            CheckInArmSuckICFallDownToHasNullIC();                              //JerryYang 20200422 修正掉料可能檢查不出來的問題, 檢查到有掉料要重新再確認全部吸嘴
            Task=1160;
            break;
        case 1200:                                                              //Z軸向下移動
            if(bIn_XYMoveFinishOnRotationKit==false)                            //Sam 20240726 : 修正移動完 XY 準備放 IC 到 RotationKit 時 Home
                Task=1160;

            if(M_MoveInArmZ_ToRotateKIT_Place(iKit)==true)
            {
                if((AccessLevel>=1 || CosFunction.bOPCanPressStepAndTStart) &&  //JerryYang 20170417 (wei) OP權限也能按Step跟T.Start
                    Sen[SnRKManualTStart].IsOn())
                {
                    bEnterOffset=false;
                    fMain->Pause("M_DoInArmRotateKIT_Motor 1200");
                    Task=1201;
                    break;
                }

                InArmSuck.ResetAll();                                           //Steven 20160323 : 避免未開啟真空
                Task=1300;
            }
            break;
         case 1201:                                                             //kevin 20130520 add
            if(MoveInArmZToPlateSafe(Task))
            {
                bEnterOffset=false;
                Task=1100;
            }
            break;
        case 1300:                                                              //吹下IC
            flag=true;
            if(iRotate_Type==e1MotRotate1Dut)                                   //Frank 20180512 (Steven) : modify 1 Dut 1 Motor
            {
                for(int j=iKit; j<InArmSuck.iMaxCol; j+=4)
                {
                    iTrayY=0;
                    iTrayX=0;
                    iSuckR=iRotato_In_Row;
                    iSuckC=j;
                    if(InArmSuck.Item[iSuckR][iSuckC]==HAS_NULL_IC ||
                       (InArmSuck.Item[iSuckR][iSuckC] &&
                        InArmSuck.Suck[iSuckR][iSuckC].GetNeedDestroyStatus()))
                    {
                        if(InArmSuck.Item[iSuckR][iSuckC]==HAS_NULL_IC ||
                           InArmSuck.Suck[iSuckR][iSuckC].Destroy())
                        {
                            bSuckDuplicateErr[iSuckR][iSuckC]=false;
                            InArmSuck.CopyToTray(iSuckR, iSuckC, NULL_IC, MOT[MInRotateKit], iTrayY, iTrayX, InArmSuck.Item[iSuckR][iSuckC]);
                            bInArmCheckDestroyACT[iSuckR][iSuckC]=true;         //kevin 20140220 回吸檢測使用 jou 981130 確認device確實destroy完成
                        }
                        else if(InArmSuck.Suck[iSuckR][iSuckC].Error==false)
                        {
                            flag=false;
                        }
                    }
                }

                if(flag==false)
                    break;

                for(int j=iKit; j<InArmSuck.iMaxCol; j+=4)                      //JerryYang 20160718 修正Rotate hang up
                    if(InArmSuck.Item[iSuckR][j] && InArmSuck.Suck[iSuckR][j].GetNeedDestroyStatus())
                        return false;
            }
            else if(iRotate_Type==e2MotRotate2Dut)
            {
                if(InArmSuck.iPickCol==1)
                {
                    for(int j=iKit; j<InArmSuck.iMaxCol; j+=4)
                    {
                        iTrayY=0;
                        iTrayX=0;
                        iSuckR=iRotato_In_Row;
                        iSuckC=j;
                        if(InArmSuck.Item[iSuckR][iSuckC]==HAS_NULL_IC ||
                           (InArmSuck.Item[iSuckR][iSuckC] &&
                            InArmSuck.Suck[iSuckR][iSuckC].GetNeedDestroyStatus()))
                        {
                            if(InArmSuck.Item[iSuckR][iSuckC]==HAS_NULL_IC ||
                               InArmSuck.Suck[iSuckR][iSuckC].Destroy())
                            {
                                bSuckDuplicateErr[iSuckR][iSuckC]=false;
                                InArmSuck.CopyToTray(iSuckR, iSuckC, NULL_IC, MOT[MInRotateKit], iTrayY, iTrayX, InArmSuck.Item[iSuckR][iSuckC]);
                                bInArmCheckDestroyACT[iSuckR][iSuckC]=true;     //kevin 20140220 回吸檢測使用 jou 981130 確認device確實destroy完成
                            }
                            else if(InArmSuck.Suck[iSuckR][iSuckC].Error==false)
                            {
                                flag=false;
                            }
                        }
                    }

                    if(flag==false)
                        break;

                    for(int j=iKit; j<InArmSuck.iMaxCol; j+=4)                  //JerryYang 20160718 修正Rotate hang up
                        if(InArmSuck.Item[iSuckR][j] && InArmSuck.Suck[iSuckR][j].GetNeedDestroyStatus())
                            return false;
                }
                else if(bUseAxxGPicker() ||
                        iCloseSiteModeFor1x4==e1x4CloseAbAc)                    //Steven 20241111 : for 1x4 close 2 site
                {
                    for(int j=0; j<2; j++)
                    {
                        iTrayY=0;
                        iTrayX=j;
                        iSuckR=iRotato_In_Row;
                        iSuckC=j*3;
                        if(InArmSuck.Item[iSuckR][iSuckC]==HAS_NULL_IC ||
                           (InArmSuck.Item[iSuckR][iSuckC] &&
                            InArmSuck.Suck[iSuckR][iSuckC].GetNeedDestroyStatus()))
                        {
                            if(InArmSuck.Item[iSuckR][iSuckC]==HAS_NULL_IC ||
                               InArmSuck.Suck[iSuckR][iSuckC].Destroy())
                            {
                                bSuckDuplicateErr[iSuckR][iSuckC]=false;
                                InArmSuck.CopyToTray(iSuckR, iSuckC, NULL_IC, MOT[MInRotateKit], iTrayY, iTrayX, InArmSuck.Item[iSuckR][iSuckC]);
                                bInArmCheckDestroyACT[iSuckR][iSuckC]=true;     //kevin 20140220 回吸檢測使用 jou 981130 確認device確實destroy完成
                            }
                            else if(InArmSuck.Suck[iSuckR][iSuckC].Error==false)
                            {
                                flag=false;
                            }
                        }
                    }

                    if(flag==false)
                        break;

                    for(int j=0; j<2; j++)
                    {
                        iSuckR=iRotato_In_Row;
                        iSuckC=j*3;
                        if(InArmSuck.Item[iSuckR][iSuckC] && InArmSuck.Suck[iSuckR][iSuckC].GetNeedDestroyStatus())
                            return false;
                    }
                }
                else
                {
                    for(int j=0; j<2; j++)
                    {
                        iTrayY=0;
                        iTrayX=j;
                        iSuckR=iRotato_In_Row;
                        iSuckC=j*2+iKit;
                        if(InArmSuck.Item[iSuckR][iSuckC]==HAS_NULL_IC ||
                           (InArmSuck.Item[iSuckR][iSuckC] &&
                            InArmSuck.Suck[iSuckR][iSuckC].GetNeedDestroyStatus()))
                        {
                            if(InArmSuck.Item[iSuckR][iSuckC]==HAS_NULL_IC ||
                               InArmSuck.Suck[iSuckR][iSuckC].Destroy())
                            {
                                bSuckDuplicateErr[iSuckR][iSuckC]=false;
                                InArmSuck.CopyToTray(iSuckR, iSuckC, NULL_IC, MOT[MInRotateKit], iTrayY, iTrayX, InArmSuck.Item[iSuckR][iSuckC]);
                                bInArmCheckDestroyACT[iSuckR][iSuckC]=true;         //kevin 20140220 回吸檢測使用 jou 981130 確認device確實destroy完成
                            }
                            else if(InArmSuck.Suck[iSuckR][iSuckC].Error==false)
                            {
                                flag=false;
                            }
                        }
                    }

                    if(flag==false)
                        break;

                    for(int j=0; j<2; j++)
                    {
                        iSuckR=iRotato_In_Row;
                        iSuckC=j*2+iKit;
                        if(InArmSuck.Item[iSuckR][iSuckC] && InArmSuck.Suck[iSuckR][iSuckC].GetNeedDestroyStatus())
                            return false;
                    }
                }
            }
            else if(W906RKIN_DUTNUM==tDutType_8)                                //kevin 20130517 add 8 site
            {
                if(iInArmType==e9045_1x2_2_14 || iInArmType==e9045_2x2_4_14 || iInArmType==e9045_2x4_4_14)    //Steven 20201028 : for rotator
                {
                    for(int j=iKit; j<MAX_ARM_Col; j+=4)
                    {
                        iSuckR=iRotato_In_Row;

                        if(TestIF.iTestMode==_8Site2X4 || TestIF.iTestMode==_16Site4X4)  //Sam 20190226 : 16Site4X4
                            iSuckC=j*3;
                        else
                            iSuckC=j*2;

                        if(TestIF.iTestMode==QualSite2X2 && DeviceForm.XDimension>=5000)
                        {                                                       //wei 20151229  Xilinx 52.5x52.5 放一個Rotate  //Alick 20160105 (wei) modify IC>=50x50 放一個Rotate
                            iTrayX=1;
                            iTrayY=0;
                        }
                        else
                        {
                            iTrayX=1+iKit;
                            iTrayY=iKit;
                        }

                        if(InArmSuck.Item[iSuckR][iSuckC] &&
                           InArmSuck.Suck[iSuckR][iSuckC].GetNeedDestroyStatus() ||
                           InArmSuck.Item[iSuckR][iSuckC]==HAS_NULL_IC)
                        {
                            if(InArmSuck.Item[iSuckR][iSuckC]==HAS_NULL_IC ||
                               InArmSuck.Suck[iSuckR][iSuckC].Destroy())
                            {
                                bSuckDuplicateErr[iSuckR][iSuckC]=false;
                                InArmSuck.CopyToTray(iSuckR, iSuckC, NULL_IC, MOT[MInRotateKit], iTrayY, iTrayX, InArmSuck.Item[iSuckR][iSuckC]);
                                bInArmCheckDestroyACT[iSuckR][iSuckC]=true;     //kevin 20140220 回吸檢測使用 jou 981130 確認device確實destroy完成
                            }
                            else if(InArmSuck.Suck[iSuckR][iSuckC].Error==false)
                            {
                                flag=false;
                            }
                        }
                    }

                    if(flag==false)
                        break;

                    for(int j=iKit; j<iMAX_ARM_Col; j+=4)                       //JerryYang 20160718 修正Rotate hang up
                        if(InArmSuck.Item[0][j*2] && InArmSuck.Suck[0][j*2].GetNeedDestroyStatus())
                            return false;
                }
                else
                {
                    for(int i=0; i<MAX_ARM_Row; i++)                            //for 2X2
                    {
                        for(int j=0; j<MAX_ARM_Col; j++)
                        {
                            iSuckR=i;
                            iSuckC=j;

                            if(TestIF.iTestMode==QualSite1X4 || TestIF.iTestMode==_8Site1X4)  //ChungHung 20150528 add for 海思 _8Site1x4
                            {
                                iTrayX=j;
                                iTrayY=i+iKit;
                            }
                            else
                            {
                                iTrayX=j;
                                iTrayY=i;
                            }

                            if(InArmSuck.Item[iSuckR][iSuckC] &&
                               InArmSuck.Suck[iSuckR][iSuckC].GetNeedDestroyStatus() ||
                               InArmSuck.Item[iSuckR][iSuckC]==HAS_NULL_IC)
                            {
                                if(InArmSuck.Item[iSuckR][iSuckC]==HAS_NULL_IC ||
                                   InArmSuck.Suck[iSuckR][iSuckC].Destroy())
                                {
                                    bSuckDuplicateErr[iSuckR][iSuckC]=false;
                                    InArmSuck.CopyToTray(iSuckR, iSuckC, NULL_IC, MOT[MInRotateKit], iTrayY, iTrayX, InArmSuck.Item[iSuckR][iSuckC]);
                                    bInArmCheckDestroyACT[iSuckR][iSuckC]=true; //kevin 20140220 回吸檢測使用 jou 981130 確認device確實destroy完成
                                }
                                else if(InArmSuck.Suck[iSuckR][iSuckC].Error==false)
                                {
                                    flag=false;
                                }
                            }
                        }
                    }

                    if(flag==false)
                        break;

                    for(int i=0; i<iMAX_ARM_Row; i++)                           //JerryYang 20160718 修正Rotate hang up
                        for(int j=0; j<iMAX_ARM_Col; j++)
                            if(InArmSuck.Item[i][j] && InArmSuck.Suck[i][j].GetNeedDestroyStatus())
                                return false;
                }
            }
            else
            {
                for(int i=0; i<iMAX_ARM_Row; i++)                               //for 2X2
                {
                    for(int j=0; j<iMAX_ARM_Col; j++)
                    {
                        iSuckR=i;
                        iSuckC=j*i2x2Suck+iKit;

                        if(TestIF.iTestMode==QualSite1X4 || TestIF.iTestMode==_8Site1X4)  //ChungHung 20150528 add for 海思 _8Site1x4
                        {
                            iTrayX=j;
                            iTrayY=i+iKit;
                        }
                        else
                        {
                            iTrayX=j;
                            iTrayY=i;
                        }

                        if(InArmSuck.Item[iSuckR][iSuckC] &&
                           InArmSuck.Suck[iSuckR][iSuckC].GetNeedDestroyStatus() ||
                           InArmSuck.Item[iSuckR][iSuckC]==HAS_NULL_IC)
                        {
                            if(InArmSuck.Item[iSuckR][iSuckC]==HAS_NULL_IC ||
                               InArmSuck.Suck[iSuckR][iSuckC].Destroy())
                            {
                                bSuckDuplicateErr[iSuckR][iSuckC]=false;
                                InArmSuck.CopyToTray(iSuckR, iSuckC, NULL_IC, MOT[MInRotateKit], iTrayY, iTrayX, InArmSuck.Item[iSuckR][iSuckC]);
                                bInArmCheckDestroyACT[iSuckR][iSuckC]=true;     //kevin 20140220 回吸檢測使用 jou 981130 確認device確實destroy完成
                            }
                            else if(InArmSuck.Suck[iSuckR][iSuckC].Error==false)
                            {
                                flag=false;
                            }
                        }
                    }
                }

                if(flag==false)
                    break;

                for(int i=0; i<iMAX_ARM_Row; i++)
                    for(int j=iKit; j<iMAX_ARM_Col; j+=i2x2Suck)                //JerryYang 20160718 修正Rotate hang up
                        if(InArmSuck.Item[i][j] && InArmSuck.Suck[i][j].GetNeedDestroyStatus())
                            return false;
            }

            ZeroMemory(bSuckDuplicateErr, sizeof(bSuckDuplicateErr));
            Task=2000;
            break;
        case 2000:                                                              //放下IC結束, Z軸升起
            if(MoveInArmZToPlateSafe(Task))
            {
                if(iRotate_Type==e1MotRotate1Dut ||                             //Frank 20180512 (Steven) : modify 1 Dut 1 Motor
                   iRotate_Type==e2MotRotate2Dut)
                {
                    Task=2500;
                }
                else if(W906RKIN_DUTNUM==tDutType_8)                            //kevin 20130517 add 8 site
                {
                    if(iInArmType==e9045_1x2_2_14 || iInArmType==e9045_2x2_4_14 || iInArmType==e9045_2x4_4_14)    //Steven 20201028 : for rotator
                    {
                        if(TestIF.iTestMode==QualSite2X2 && DeviceForm.XDimension>=5000)       //wei 20151229  Xilinx 52.5x52.5 放一個Rotate  //Alick 20160105 (wei) modify IC>=50x50 放一個Rotate
                        {
                            Task=2500;
                        }
                        else
                        {
                            if(iKit==0)
                            {
                                if(((TestIF.iTestMode==QualSite2X2 || TestIF.iTestMode==DualSite)   && InArmSuck.Item[iRotato_In_Row][2]!=NULL_IC && InArmSuck.Item[iRotato_In_Row][2]!=HAS_NULL_IC) ||  //(1,3)
                                    ((TestIF.iTestMode==_8Site2X4  || TestIF.iTestMode==_16Site4X4) && InArmSuck.Item[iRotato_In_Row][3]!=NULL_IC && InArmSuck.Item[iRotato_In_Row][3]!=HAS_NULL_IC)) //Sam 20190226 : 16Site4X4 //(1,4)
                                {
                                    iKit=1;
                                    Task=1000;
                                }
                                else
                                {
                                    iKit=0;
                                    Task=2500;
                                }
                            }
                            else
                            {
                                iKit=0;
                                Task=2500;
                            }
                        }
                        break;
                    }
                    else if(InArmSuck.FindNoIC())                               //kevkin 20130519 IC已放下
                    {
                        iKit=0;
                        Task=2500;
                        break;
                    }
                }
                else
                {
                    if(InArmSuck.Item[0][0]==NULL_IC &&
                       InArmSuck.Item[0][i2x2Suck]==NULL_IC &&
                       InArmSuck.Item[1][0]==NULL_IC &&
                       InArmSuck.Item[1][i2x2Suck]==NULL_IC)
                    {
                        if(bResetKit0)                                          //Sam 20221209 : 修正 4Dut 模式 InArm CleanOut時最後剛好只放 Kit1 吸取錯問題
                            iKit=0;
                    }

                    Task=2500;
                    break;
                }
            }
            break;
        case 2500:                                                              //偵測IC 是否疊料
            if(CheckRotateSensor(true))
            {
                Task=2510;
            }
            else
            {
                if(IniConfig.bA21RotateDetectErrNeedShake)
                {
                    InitialShakeInArmRotateKIT();
                    Task=2501;
                }
                else
                {
                    Task=2502;
                }
            }
            break;
        case 2501:
            if(M_DoShakeInArmRotateKIT()==true)                                 //JerryYang 20160804 偵測到異常要轉三下
            {
                if(CheckRotateSensor(true))                                     //JerryYang 20160804 轉完再偵測一次是否疊料
                {
                    Task=2510;
                }
                else
                {
                    Task=2502;
                }
            }
            break;
        case 2502:
            if(MoveInArm2XYToWait()==false)                                     //如果要移開,而且還沒移到定位
            {
                return false;                                                   //先離開等In Arm
            }
            Task=2503;
            break;
        case 2503:                                                              //In Rotate 疊料
            ShowErrorMessage("JAM0452", K_RETRY, MInRotateKit);
            Task=2500;
            break;
        case 2510:
            DoRotate_In(rCount, true);
            //AI(W906-ROTKIT-IN) 20260807: GATE (7) -- bShowRotateBySite -> false, so Task goes on to
            //  the tRotate.DutNum test below rather than straight to 2600.
            if(W906RKIN_SHOWROTATEBYSITE==true)                                 //Steven 20170329 (wei) : Add individual rotate motor  //wei 20190515 modify 2 Dut 2 Motor
            {
                Task=2600;
            }
            else if(W906RKIN_DUTNUM==tDutType_8)
            {
                if(iInArmType==e9045_1x2_2_14 ||                                //Steven 20201028 : for rotator
                   iInArmType==e9045_2x2_4_14 ||
                   iInArmType==e9045_2x4_4_14 ||
                   iRotate_Type==eInOutArm1Motor)
                {
                    Task=2550;                                                  //wei 20141230 Rotate旋轉問題 for 大IC單獨吸
                }
                else
                {
                    Task=2600;
                }
            }
            else
            {
                Task=2600;
            }
            break;
        case 2550:                                                              //wei 20141230 Rotate旋轉問題 for 大IC單獨吸
            if(iInArmType==e9045_1x2_2_14 ||                                    //Steven 20201028 : for rotator
               iInArmType==e9045_2x2_4_14 ||
               iInArmType==e9045_2x4_4_14 ||
               iRotate_Type==eInOutArm1Motor)
            {
                //AI(W906-ROTKIT-IN) 20260807: GOLDEN BUG preserved VERBATIM (golden :2382-2390) -- the
                //  loop runs down to iRotateCount==0 and then reads Prod.RotationCount[-1], one int
                //  before the array.  Not fixed.
                for(iRotateCount=4; iRotateCount>=0; iRotateCount--)
                {
                    if(Prod.RotateDutDate[0][iKit][iKit+1]==Prod.RotationCount[iRotateCount-1])   //Ifor 20170412 (Steven) Modify 角度已算過不需在減掉原本角度
                    {
                        rCount=iRotateCount;
                        Task=2600;
                        break;
                    }
                }
            }
            break;
        case 2600:                                                              //旋轉
            if(iRotate_Type==e1MotRotate1Dut)                                   //Frank 20180512 (Steven) : modify 1 Dut 1 Motor
            {
            }
            else if(iRotate_Type==e2MotRotate2Dut)
            {
                for(int j=0; j<2; j++)
                {
                    if(MOT[MInRotateKit].Tray.Data[j][0]!=NULL_IC &&
                       MOT[MInRotateKit].Tray.Data[j][0]!=HAS_NULL_IC &&
                       MOT[MInRotateKit].Tray.iNeedRotAng[j][0]!=MOT[MInRotateKit].Tray.iCurrRotAng[j][0])
                    {
                        DoRotate_In(rCount, true);
                    }
                }
            }
            //AI(W906-ROTKIT-IN) 20260807: GATE (7) -- bShowRotateBySite -> false; this whole
            //  "skip the turn if every target is HAS_NULL_IC" arm (Sam 20240110) is present but not
            //  selected, so control reaches DoRotate_In(rCount) below unconditionally, as golden does
            //  on a machine that is not in per-site rotate mode.
            else if(W906RKIN_SHOWROTATEBYSITE==true)                            //Sam 20240110 : 檢查這次的選轉位置若都為 Has_Null_IC 都不要轉了
            {
                if(rCount-1>0)
                {
                    bool bNeedR=false;
                    //AI(W906-ROTKIT-IN) 20260807: GOLDEN BUG preserved VERBATIM (golden :2414) -- a bare
                    //  expression statement that reads tRotate.RotationCount[rCount-1] and discards it.  It
                    //  has no effect in golden either.  Not fixed.  (Also GATE (5): the member is not on the
                    //  tRotateShim, so it expands to the equally inert `(0);`.)
                    W906RKIN_TROT_ROTATIONCOUNT(rCount-1);
                    for(int i=0; i<MAX_ARM_Row; i++)
                    {
                        for(int j=0; j<MAX_ARM_Col; j++)
                        {
                            iTrayX=j;
                            iTrayY=i;
                            if(MOT[MInRotateKit].Tray.Data[iTrayX][iTrayY]!=NULL_IC     &&
                               MOT[MInRotateKit].Tray.Data[iTrayX][iTrayY]!=HAS_NULL_IC &&
                               //AI(W906-ROTKIT-IN) 20260807: GATE (5) -- tRotate.RotationCount[] -> (0).
                               //  tRotate.RotateDutDate on the same line is REAL (aHotPlateSubstrate.h:829) and is left
                               //  untouched.
                               tRotate.RotateDutDate[0][iTrayY][iTrayX]==W906RKIN_TROT_ROTATIONCOUNT(rCount-1))
                            {
                                bNeedR=true;
                            }
                        }
                    }

                    if(bNeedR==false)
                    {
                        rCount--;
                        break;
                    }
                }
            }

            ret=DoRotate_In(rCount);

            if(ret==1)
            {
                bIn_ICRotationCompleteOnKit=true;                               //Sam 20210623 : 修正 IC 轉完後被 Home，導致 IC 轉向異常。
                Task=3000;
            }
            else if(ret==-999)                                                  //Error
            {
                Task=4800;
            }
            break;
        case 3000:                                                              //旋轉
            Task=3100;
        case 3100:                                                              //判斷ROTATE有IC吸取IC
            if(iRotate_Type==e1MotRotate1Dut ||                                 //Frank 20180512 (Steven) : modify 1 Dut 1 Motor
               iRotate_Type==e2MotRotate2Dut)
            {
                if(MOT[MInRotateKit].Tray.HasIC())
                {
                    iRetryCT=0;
                    Task=4000;
                }
                else
                {
                    Task=4400;
                }
            }
            else if(W906RKIN_DUTNUM==tDutType_8)                                //kevin 20130517 add 8 site
            {
                if(iInArmType==e9045_1x2_2_14 || iInArmType==e9045_2x2_4_14 || iInArmType==e9045_2x4_4_14)    //Steven 20201028 : for rotator
                {
                    if(TestIF.iTestMode==QualSite2X2 && DeviceForm.XDimension>=5000)            //wei 20151229  Xilinx 52.5x52.5 放一個Rotate  //Alick 20160105 (wei) modify IC>=50x50 放一個Rotate
                    {
                        if(MOT[MInRotateKit].Tray.Data[1][0]!=NULL_IC)
                        {
                            Task=4000;
                        }
                    }
                    else
                    {
                        if(MOT[MInRotateKit].Tray.Data[1][0]!=NULL_IC)
                        {
                            iKit=0;
                            Task=4000;
                        }
                        else if(MOT[MInRotateKit].Tray.Data[2][1]!=NULL_IC)
                        {
                            iKit=1;
                            Task=4000;
                        }
                        else
                        {
                            Task=6000;
                        }
                    }
                }
                else
                {
                    if(MOT[MInRotateKit].Tray.HasRealIC())
                    {
                        iRetryCT=0;
                        Task=4000;
                    }
                    else
                    {
                        Task=6000;
                    }
                }
            }
            else
            {
                if(MOT[MInRotateKit].Tray.Data[0][0]!=NULL_IC ||
                   MOT[MInRotateKit].Tray.Data[1][0]!=NULL_IC ||
                   MOT[MInRotateKit].Tray.Data[0][1]!=NULL_IC ||
                   MOT[MInRotateKit].Tray.Data[1][1]!=NULL_IC)
                {
                    iRetryCT=0;
                    Task=4000;
                }
                else
                {
                    // Task=6000;                                               //jou 2014-03-18 Task=6000->4400; kit=0 關site不需要旋轉,kit=1沒有檢查就return true
                    Task=4400;
                    break;
                }
            }
            break;
        case 4000:
            if(CheckInArmSuckICFallDownToHasNullIC(false)==true)                //Steven 20110516 : 修改成整合式Alarm
            {
                Task=4010;                                                      //JerryYang 20200422 修正掉料可能檢查不出來的問題, 檢查到有掉料要重新再確認全部吸嘴
                break;
            }

            if(M_MoveInArmXY_ToRotateKIT(iKit)==true)
            {
                for(int i=0; i<InArmSuck.iMaxRow; i++)
                {
                    for(int j=0; j<InArmSuck.iMaxCol; j++)
                    {
                        InArmSuck.Suck[i][j].SetNeedSuck(false);
                    }
                }
                Task=4100;
            }
            break;
        case 4010:
            CheckInArmSuckICFallDownToHasNullIC();                              //JerryYang 20200422 修正掉料可能檢查不出來的問題, 檢查到有掉料要重新再確認全部吸嘴
            Task=4000;
            break;
        case 4100:
            if(bIn_ICRotationCompleteOnKit==false)                              //Sam 20210623 : 修正 IC 轉完後被 Home，導致 IC 轉向異常。
            {
                DoRotate_In(rCount, true);
                Task=2600;
                break;
            }

            if(M_MoveInArmZ_ToRotateKIT_Pick(iKit, Prod.RotationCount[rCount-1])==true)
            {
                if((AccessLevel>=1 || CosFunction.bOPCanPressStepAndTStart) &&  //JerryYang 20170417 (wei) OP權限也能按Step跟T.Start
                    Sen[SnRKManualTStart].IsOn())
                {
                    bEnterOffset=false;
                    fMain->Pause("M_DoInArmRotateKIT_Motor 4100");
                    Task=4500;
                    break;
                }

                InArmSuck.ResetAll();                                           //Steven 20160323 : 避免未開啟真空
                Task=4200;
            }
            else
            {
                break;
            }
        case 4200:
            flag=true;

            if(iRotate_Type==e1MotRotate1Dut)                                   //Frank 20180512 (Steven) : modify 1 Dut 1 Motor
            {
                for(int j=iKit; j<InArmSuck.iMaxCol; j+=4)
                {
                    iTrayY=0;
                    iTrayX=0;
                    iSuckR=iRotato_In_Row;
                    iSuckC=j;
                    if(MOT[MInRotateKit].Tray.Data[iTrayX][iTrayY]==HAS_NULL_IC)
                    {
                        bSuckDuplicateErr[iSuckR][iSuckC]=false;
                        InArmSuck.CopyFromTray(iSuckR, iSuckC, MOT[MInRotateKit].Tray.Data[iTrayX][iTrayY], MOT[MInRotateKit], iTrayY, iTrayX, NULL_IC);
                    }
                    else if(MOT[MInRotateKit].Tray.Data[iTrayX][iTrayY]!=NULL_IC &&
                            CheckInRotateAnglePostion(iSuckR, iSuckC, iTrayX, iTrayY))
                    {
                        if(InArmSuck.Suck[iSuckR][iSuckC].Suck())
                        {
                            bSuckDuplicateErr[iSuckR][iSuckC]=false;
                            RecordRotateAngle(iSuckR, iSuckC, iTrayY, iTrayX);  //Sam 20221103 : 新增In旋轉角度
                            InArmSuck.CopyFromTray(iSuckR, iSuckC, MOT[MInRotateKit].Tray.Data[iTrayX][iTrayY], MOT[MInRotateKit], iTrayY, iTrayX, NULL_IC);
                        }
                        else if(InArmSuck.Suck[iSuckR][iSuckC].Error==false)
                        {
                            flag=false;
                        }
                    }
                }
            }
            else if(iRotate_Type==e2MotRotate2Dut)                              //wei 20190515 modify 2 Dut 2 Motor
            {
                if(InArmSuck.iPickCol==1)
                {
                    for(int j=iKit; j<InArmSuck.iMaxCol; j+=4)
                    {
                        iTrayY=0;
                        iTrayX=0;
                        iSuckR=iRotato_In_Row;
                        iSuckC=j;
                        if(MOT[MInRotateKit].Tray.Data[iTrayX][iTrayY]==HAS_NULL_IC)
                        {
                            bSuckDuplicateErr[iSuckR][iSuckC]=false;
                            InArmSuck.CopyFromTray(iSuckR, iSuckC, MOT[MInRotateKit].Tray.Data[iTrayX][iTrayY], MOT[MInRotateKit], iTrayY, iTrayX, NULL_IC);
                        }
                        else if(MOT[MInRotateKit].Tray.Data[iTrayX][iTrayY]!=NULL_IC &&
                                CheckInRotateAnglePostion(iSuckR, iSuckC, iTrayX, iTrayY))
                        {
                            if(InArmSuck.Suck[iSuckR][iSuckC].Suck())
                            {
                                bSuckDuplicateErr[iSuckR][iSuckC]=false;
                                RecordRotateAngle(iSuckR, iSuckC, iTrayY, iTrayX);
                                InArmSuck.CopyFromTray(iSuckR, iSuckC, MOT[MInRotateKit].Tray.Data[iTrayX][iTrayY], MOT[MInRotateKit], iTrayY, iTrayX, NULL_IC);
                            }
                            else if(InArmSuck.Suck[iSuckR][iSuckC].Error==false)
                            {
                                flag=false;
                            }
                        }
                    }
                }
                else if(bUseAxxGPicker() ||
                        iCloseSiteModeFor1x4==e1x4CloseAbAc)                    //Steven 20241111 : for 1x4 close 2 site
                {
                    for(int j=0; j<2; j++)
                    {
                        iTrayY=0;
                        iTrayX=j;
                        iSuckR=iRotato_In_Row;
                        iSuckC=j*3+iKit;
                        if(MOT[MInRotateKit].Tray.Data[iTrayX][iTrayY]==HAS_NULL_IC)
                        {
                            bSuckDuplicateErr[iSuckR][iSuckC]=false;
                            InArmSuck.CopyFromTray(iSuckR, iSuckC, MOT[MInRotateKit].Tray.Data[iTrayX][iTrayY], MOT[MInRotateKit], iTrayY, iTrayX, NULL_IC);
                        }
                        else if(MOT[MInRotateKit].Tray.Data[iTrayX][iTrayY]!=NULL_IC &&
                                CheckInRotateAnglePostion(iSuckR, iSuckC, iTrayX, iTrayY))
                        {
                            if(InArmSuck.Suck[iSuckR][iSuckC].Suck())
                            {
                                bSuckDuplicateErr[iSuckR][iSuckC]=false;
                                RecordRotateAngle(iSuckR, iSuckC, iTrayY, iTrayX);  //Sam 20221103 : 新增In旋轉角度
                                InArmSuck.CopyFromTray(iSuckR, iSuckC, MOT[MInRotateKit].Tray.Data[iTrayX][iTrayY], MOT[MInRotateKit], iTrayY, iTrayX, NULL_IC);
                            }
                            else if(InArmSuck.Suck[iSuckR][iSuckC].Error==false)
                            {
                                flag=false;
                            }
                        }
                    }
                }
                else
                {
                    for(int j=0; j<2; j++)
                    {
                        iTrayY=0;
                        iTrayX=j;
                        iSuckR=iRotato_In_Row;
                        iSuckC=j*2+iKit;
                        if(MOT[MInRotateKit].Tray.Data[iTrayX][iTrayY]==HAS_NULL_IC)
                        {
                            bSuckDuplicateErr[iSuckR][iSuckC]=false;
                            InArmSuck.CopyFromTray(iSuckR, iSuckC, MOT[MInRotateKit].Tray.Data[iTrayX][iTrayY], MOT[MInRotateKit], iTrayY, iTrayX, NULL_IC);
                        }
                        else if(MOT[MInRotateKit].Tray.Data[iTrayX][iTrayY]!=NULL_IC &&
                                CheckInRotateAnglePostion(iSuckR, iSuckC, iTrayX, iTrayY))
                        {
                            if(InArmSuck.Suck[iSuckR][iSuckC].Suck())
                            {
                                bSuckDuplicateErr[iSuckR][iSuckC]=false;
                                RecordRotateAngle(iSuckR, iSuckC, iTrayY, iTrayX);  //Sam 20221103 : 新增In旋轉角度
                                InArmSuck.CopyFromTray(iSuckR, iSuckC, MOT[MInRotateKit].Tray.Data[iTrayX][iTrayY], MOT[MInRotateKit], iTrayY, iTrayX, NULL_IC);
                            }
                            else if(InArmSuck.Suck[iSuckR][iSuckC].Error==false)
                            {
                                flag=false;
                            }
                        }
                    }
                }
            }
            else if(W906RKIN_DUTNUM==tDutType_8)                                //kevin 20130517 add 8 site
            {
                if(iInArmType==e9045_1x2_2_14 || iInArmType==e9045_2x2_4_14 || iInArmType==e9045_2x4_4_14)    //Steven 20201028 : for rotator
                {
                    for(int j=iKit; j<MAX_ARM_Col; j+=4)
                    {
                        iSuckR=iRotato_In_Row;
                        if(TestIF.iTestMode==_8Site2X4 || TestIF.iTestMode==_16Site4X4)  //Sam 20190226 : 16Site4X4
                            iSuckC=j*3;
                        else
                            iSuckC=j*2;

                        if(TestIF.iTestMode==QualSite2X2 && DeviceForm.XDimension>=5000)
                        {   //wei 20151229  Xilinx 52.5x52.5 放一個Rotate //Alick 20160105 (wei) modify IC>=50x50 放一個Rotate
                            iTrayX=1;
                            iTrayY=0;
                        }
                        else
                        {
                            iTrayX=iKit;
                            iTrayY=j+1;
                        }

                        if(MOT[MInRotateKit].Tray.Data[iTrayX][iTrayY]==HAS_NULL_IC)
                        {
                            bSuckDuplicateErr[iSuckR][iSuckC]=false;
                            InArmSuck.CopyFromTray(iSuckR, iSuckC, MOT[MInRotateKit].Tray.Data[iTrayX][iTrayY], MOT[MInRotateKit], iTrayY, iTrayX, NULL_IC);
                        }
                        else if(MOT[MInRotateKit].Tray.Data[iTrayX][iTrayY]!=NULL_IC &&
                                Prod.RotateDutDate[0][iTrayY][iTrayX]==Prod.RotationCount[rCount-1])   //Ifor 20170412 (Steven) Modify 角度已算過不需在減掉原本角度
                        {
                            if(InArmSuck.Suck[iSuckR][iSuckC].Suck())
                            {
                                bSuckDuplicateErr[iSuckR][iSuckC]=false;
                                RecordRotateAngle(iSuckR, iSuckC, iTrayY, iTrayX);   //Sam 20221103 : 新增In旋轉角度
                                InArmSuck.CopyFromTray(iSuckR, iSuckC, MOT[MInRotateKit].Tray.Data[iTrayX][iTrayY], MOT[MInRotateKit], iTrayY, iTrayX, NULL_IC);
                            }
                            else if(InArmSuck.Suck[iSuckR][iSuckC].Error==false)
                            {
                                flag=false;
                            }
                        }
                    }
                }
                else
                {
                    for(int i=0; i<MAX_ARM_Row; i++)
                    {
                        for(int j=0; j<MAX_ARM_Col; j++)
                        {
                            iSuckR=i;
                            iSuckC=j;

                            iTrayX=j;
                            iTrayY=i;

                            if(MOT[MInRotateKit].Tray.Data[iTrayX][iTrayY]==HAS_NULL_IC)
                            {
                                bSuckDuplicateErr[iSuckR][iSuckC]=false;
                                InArmSuck.CopyFromTray(iSuckR, iSuckC, MOT[MInRotateKit].Tray.Data[iTrayX][iTrayY], MOT[MInRotateKit], iTrayY, iTrayX, NULL_IC);
                            }
                            else if(MOT[MInRotateKit].Tray.Data[iTrayX][iTrayY]!=NULL_IC &&
                                    Prod.RotateDutDate[0][iTrayY][iTrayX]==Prod.RotationCount[rCount-1])   //Ifor 20170412 (Steven) Modify 角度已算過不需在減掉原本角度
                            {
                                if(InArmSuck.Suck[iSuckR][iSuckC].Suck())
                                {
                                    bSuckDuplicateErr[iSuckR][iSuckC]=false;
                                    RecordRotateAngle(iSuckR, iSuckC, iTrayY, iTrayX);   //Sam 20221103 : 新增In旋轉角度
                                    InArmSuck.CopyFromTray(iSuckR, iSuckC, MOT[MInRotateKit].Tray.Data[iTrayX][iTrayY], MOT[MInRotateKit], iTrayY, iTrayX, NULL_IC);
                                }
                                else if(InArmSuck.Suck[iSuckR][iSuckC].Error==false)
                                {
                                    flag=false;
                                }
                            }
                        }
                    }
                }
            }
            else
            {
                if(TestIF.iTestMode==QualSite1X4 || TestIF.iTestMode==_8Site1X4)  //ChungHung 20150528 add for 海思 _8Site1x4
                {
                    for(int j=iKit; j<2; j++) //kevin 20130629
                    {
                        iSuckR=0;
                        iSuckC=j*i2x2Suck+iKit;

                        iTrayX=j;
                        iTrayY=iKit;

                        if(MOT[MInRotateKit].Tray.Data[iTrayX][iTrayY]==HAS_NULL_IC)
                        {
                            bSuckDuplicateErr[iSuckR][iSuckC]=false;
                            InArmSuck.CopyFromTray(iSuckR, iSuckC, MOT[MInRotateKit].Tray.Data[iTrayX][iTrayY], MOT[MInRotateKit], iTrayY, iTrayX, NULL_IC);
                        }
                        else if(MOT[MInRotateKit].Tray.Data[iTrayX][iTrayY]!=NULL_IC &&
                                Prod.RotateDutDate[0][iTrayY][iTrayX]==Prod.RotationCount[rCount-1])   //Ifor 20170412 (Steven) Modify 角度已算過不需在減掉原本角度
                        {
                            if(InArmSuck.Suck[iSuckR][iSuckC].Suck())
                            {
                                bSuckDuplicateErr[iSuckR][iSuckC]=false;
                                RecordRotateAngle(iSuckR, iSuckC, iTrayY, iTrayX);   //Sam 20221103 : 新增In旋轉角度
                                InArmSuck.CopyFromTray(iSuckR, iSuckC, MOT[MInRotateKit].Tray.Data[iTrayX][iTrayY], MOT[MInRotateKit], iTrayY, iTrayX, NULL_IC);
                            }
                            else if(InArmSuck.Suck[iSuckR][iSuckC].Error==false)
                            {
                                flag=false;
                            }
                        }
                    }
                }
                else
                {
                    //AI(W906-ROTKIT-IN) 20260807: GATE (7) -- bShowRotateBySite -> false, so the golden else
                    //  at :2845 (the Prod.RotationCount angle-compare pick loop) is the ACTIVE arm.
                    if(USE_ROTATE_KIT==1 && W906RKIN_SHOWROTATEBYSITE==true)    //wei 20190515 modify 2 Dut 2 Motor
                    {
                        for(int i=0; i<2; i++)
                        {
                            for(int j=0; j<2; j++)                              //(0, 2) & (1, 3)
                            {
                                iSuckR=i;
                                iSuckC=j*i2x2Suck+iKit;

                                iTrayX=j;
                                iTrayY=i;

                                if(MOT[MInRotateKit].Tray.Data[iTrayX][iTrayY]==HAS_NULL_IC)
                                {
                                    bSuckDuplicateErr[iSuckR][iSuckC]=false;
                                    InArmSuck.CopyFromTray(iSuckR, iSuckC, MOT[MInRotateKit].Tray.Data[iTrayX][iTrayY], MOT[MInRotateKit], iTrayY, iTrayX, NULL_IC);
                                }
                                else if(MOT[MInRotateKit].Tray.Data[iTrayX][iTrayY]!=NULL_IC && MOT[MInRotateKit].Tray.Data[iTrayX][iTrayY]!=HAS_NULL_IC)
                                {
                                    if(InArmSuck.Suck[iSuckR][iSuckC].Suck())
                                    {
                                        bSuckDuplicateErr[iSuckR][iSuckC]=false;
                                        RecordRotateAngle(iSuckR, iSuckC, iTrayY, iTrayX);   //Sam 20221103 : 新增In旋轉角度
                                        InArmSuck.CopyFromTray(iSuckR, iSuckC, MOT[MInRotateKit].Tray.Data[iTrayX][iTrayY], MOT[MInRotateKit], iTrayY, iTrayX, NULL_IC);
                                    }
                                    else if(InArmSuck.Suck[iSuckR][iSuckC].Error==false)
                                    {
                                        flag=false;
                                    }
                                }
                            }
                        }
                    }
                    else
                    {
                        for(int i=0; i<MAX_ARM_Row; i++)
                        {
                            for(int j=0; j<iMAX_ARM_Col; j++) //kevin 20130629
                            {
                                iSuckR=i;
                                iSuckC=j*i2x2Suck+iKit;

                                iTrayX=j;
                                iTrayY=i;

                                if(MOT[MInRotateKit].Tray.Data[iTrayX][iTrayY]==HAS_NULL_IC)
                                {
                                    bSuckDuplicateErr[iSuckR][iSuckC]=false;
                                    InArmSuck.CopyFromTray(iSuckR, iSuckC, MOT[MInRotateKit].Tray.Data[iTrayX][iTrayY], MOT[MInRotateKit], iTrayY, iTrayX, NULL_IC);
                                }
                                else if(MOT[MInRotateKit].Tray.Data[iTrayX][iTrayY]!=NULL_IC &&
                                        Prod.RotateDutDate[0][iTrayY][iTrayX]==Prod.RotationCount[rCount-1])   //Ifor 20170412 (Steven) Modify 角度已算過不需在減掉原本角度
                                {
                                    if(InArmSuck.Suck[iSuckR][iSuckC].Suck())
                                    {
                                        bSuckDuplicateErr[iSuckR][iSuckC]=false;
                                        RecordRotateAngle(iSuckR, iSuckC, iTrayY, iTrayX);   //Sam 20221103 : 新增In旋轉角度
                                        InArmSuck.CopyFromTray(iSuckR, iSuckC, MOT[MInRotateKit].Tray.Data[iTrayX][iTrayY], MOT[MInRotateKit], iTrayY, iTrayX, NULL_IC);
                                    }
                                    else if(InArmSuck.Suck[iSuckR][iSuckC].Error==false)
                                    {
                                        flag=false;
                                    }
                                }
                            }
                        }
                    }
                }
            }

            if(flag==false)                                                     //kevin 20130517
                break;

            for(int i=0; i<MAX_ARM_Row; i++)
            {
                for(int j=0; j<MAX_ARM_Col; j++)
                {
                    if(InArmSuck.Suck[i][j].Error)
                    {
                        iRetryCT++;
                        Task=4600;
                        return false;
                    }
                }
            }

            rCount--;
            iRetryCT=0;
            Task=4300;
            break;
        case 4300:
            if(MoveInArmZToPlateSafe(Task))
            {
                //AI(W906-ROTKIT-IN) 20260807: GATE (7) -- bShowRotateBySite -> false, so the rCount>0
                //  else arm below is the ACTIVE one.
                if(W906RKIN_SHOWROTATEBYSITE==true)                             //wei 20190515 modify 2 Dut 2 Motor
                {
                    Task=4400;
                }
                else
                {
                    if(rCount>0)
                        Task=2600;
                    else
                        Task=4400;
                }
            }
            break;
        case 4400:
            if(iRotate_Type==e1MotRotate1Dut)                                   //Frank 20180512 (Steven) : modify 1 Dut 1 Motor
            {
                if(MOT[MInRotateKit].Tray.Data[0][0]==NULL_IC)
                    Task=5000;
                else
                    Task=2600;
            }
            else if(iRotate_Type==e2MotRotate2Dut)                              //wei 20190515 modify 2 Dut 2 Motor
            {
                if(MOT[MInRotateKit].Tray.Data[0][0]==NULL_IC &&
                   MOT[MInRotateKit].Tray.Data[1][0]==NULL_IC)
                    Task=5000;
                else
                    Task=2600;
            }
            else if(iInArmType==e9045_1x2_2_14 || iInArmType==e9045_2x2_4_14 || iInArmType==e9045_2x4_4_14)    //Steven 20201028 : for rotator
            {
                if(W906RKIN_DUTNUM==tDutType_8)
                {
                    if(MOT[MInRotateKit].Tray.Data[1][0]==NULL_IC && MOT[MInRotateKit].Tray.Data[2][1]==NULL_IC)
                    {
                        iKit=0;
                        Task=5000;
                    }
                    else
                    {
                        Task=2550;
                    }
                }
                else
                {
                    if(MOT[MInRotateKit].Tray.Data[1][0]==NULL_IC && MOT[MInRotateKit].Tray.Data[1][1]==NULL_IC)
                    {
                        iKit=0;
                        Task=5000;
                    }
                    else
                    {
                        Task=2600;
                    }
                }
            }
            else if(iKit==0)
            {
                iKit++;
                if(TestIF.iTestMode==_8Site2X4 || TestIF.iTestMode==_16Site4X4 || TestIF.iTestMode==_16Site2X8 || (TestIF.iTestMode==QualSite2X2 && TestIF.iUseSuckMode==8))  //Sam 20190226 : 16Site4X4  //JerryYang 20170606 (Steven) 獨立rotate 支援2x2 8吸嘴模式
                {
                    //AI(W906-ROTKIT-IN) 20260807: GATES (2)+(10) -- DutNum -> tDutType_4 and ArmCanSuck4IC
                    //  -> true, so this condition is TRUE and Task=10 (re-run the place/rotate/pick cycle for
                    //  the second kit) is the ACTIVE arm.
                    if(W906RKIN_DUTNUM==tDutType_4 && W906RKIN_ARMCANSUCK4IC(0)==true)    //Steven 20140207 : 修正Rotate吸八顆放四顆
                        Task=10;                                                //kevin 20130629
                    else                                                        //己做完所有旋轉
                        Task=5000;
                }
                else
                {
                    Task=3100;
                }
            }
            else
            {
                iKit=0;
                Task=5000;
            }
            break;
        case 5000:
            MoveInRotateToDegreeAtSameTime(0, true);                            //Steven 20170504 (wei) : 針對獨立馬達旋轉站, 一起旋轉到特定角度
            Task=6000;
        case 6000:
            if(iRotate_Type==e1MotRotate1Dut)                                   //Frank 20180512 (Steven) : modify 1 Dut 1 Motor
            {
                Task=GotInRotateKitForSingle(iKit, iCount, false);
                if(Task==1)
                    return true;
            }
            else if(iRotate_Type==e2MotRotate2Dut)                              //wei 20190515 modify 2 Dut 2 Motor
            {
                if(InArmSuck.iPickCol==1)
                {
                    Task=GotInRotateKitForSingle(iKit, iCount, false);
                }
                else if(bUseAxxGPicker() ||
                        iCloseSiteModeFor1x4==e1x4CloseAbAc)                    //Steven 20241111 : for 1x4 close 2 site
                {
                    Task=GotInRotateKitForAxxG(iKit, iCount, false);
                }
                else
                {
                    Task=GotInRotateKitForAxEx_2RotMot(iKit, iCount, false);
                }

                if(Task==1)
                    return true;
            }
            else if(W906RKIN_DUTNUM==tDutType_4)
            {
                iKit=0;
                MOT[MInRotateKit].SetHasNullIcToNullIc();                       //Frank 20170822 (wei): 清空Rotate資料
                return true;
            }
            else if(iInArmType==e9045_1x2_2_14 || iInArmType==e9045_2x2_4_14 || iInArmType==e9045_2x4_4_14)    //Steven 20201028 : for rotator
            {
                if((TestIF.iTestMode==QualSite2X2 && DeviceForm.XDimension>=5000))     //wei 20151229  Xilinx 52.5x52.5 放一個Rotate  //Alick 20160105 (wei) modify IC>=50x50 放一個Rotate
                {
                    if(iRotato_In_Row==0)
                    {
                        if(iCount==0)
                        {
                            iKit=1;
                            iCount=1;
                        }
                        else
                        {
                            iCount=0;
                            iRotato_In_Row=1;
                            iKit=0;
                        }
                        Task=10;
                    }
                    else
                    {
                        if(iCount==0)
                        {
                            iKit=1;
                            iCount=1;
                            Task=10;
                        }
                        else
                        {
                            iCount=0;
                            iRotato_In_Row=0;
                            iKit=0;
                            MOT[MInRotateKit].SetHasNullIcToNullIc();           //Frank 20170822 (wei): 清空Rotate資料
                            return true;
                        }
                    }
                }
                else if(iRotato_In_Row==0)
                {
                    iKit=0;
                    iRotato_In_Row=1;
                    Task=10;
                }
                else
                {
                    iKit=0;
                    iRotato_In_Row=0;
                    MOT[MInRotateKit].SetHasNullIcToNullIc();                   //Frank 20170822 (wei): 清空Rotate資料
                    return true;
                }
            }
            else
            {
                iKit=0;
                MOT[MInRotateKit].SetHasNullIcToNullIc();                       //Frank 20170822 (wei): 清空Rotate資料
                return true;
            }
            break;
        case 4500:                                                              //(1, 3)調整Offset
            if(MoveInArmZToPlateSafe(Task))
            {
                bEnterOffset=false;
                Task=4000;
            }
            break;
        case 4600:
            if(MoveInArmZToPlateSafe(Task))
            {
                if(iRetryCT>ArmSpeed[InArm].iRetryCT)                           //Sam 20250206 : Rotate pick error retry for setting
                {
                    ErrPart=" ";
                    for(int i=0; i<MAX_ARM_Row; i++)
                    {
                        for(int j=0; j<MAX_ARM_Col; j++)
                        {
                            if(bSuckDuplicateErr[i][j])
                                bHasDuplicateErr=true;
                            if(InArmSuck.Suck[i][j].Error)
                            {
                                ErrPart+=InArmSuck.Suck[i][j].sName;
                                bSuckDuplicateErr[i][j]=true;
                                InArmSuck.Suck[i][j].Error=false;
                            }
                            else
                            {
                                bSuckDuplicateErr[i][j]=false;
                            }
                        }
                    }
                    ret=ShowErrorMessage("JAM0112", K_RETRY|K_SKIP, MInArmX, bHasDuplicateErr, ErrPart);    //Sam 20250204 : Add Skip
                }
                else
                {
                    ret=K_RETRY;
                }

                if(ret==K_SKIP)
                {
                    if(iRotate_Type==e1MotRotate1Dut)                           //Frank 20180512 (Steven) : modify 1 Dut 1 Motor
                    {
                        //AI(W906-ROTKIT-IN) 20260807: GOLDEN BUG preserved VERBATIM (golden :3117-3133) -- this
                        //  is the IN-arm skip path, yet it bounds the loop with OutArmSuck.iMaxCol and rows it
                        //  with iRotato_Out_Row before writing InArmSuck.PordRec / InArmSuck.SetItemData.
                        //  Copy-paste from aRotateKIT_Out.cpp.  The same defect repeats in the two e2MotRotate2Dut
                        //  arms below (golden :3139, :3160, :3180).  Not fixed.
                        for(int j=iKit; j<OutArmSuck.iMaxCol; j+=4)
                        {
                            iTrayY=0;
                            iTrayX=0;
                            iSuckR=iRotato_Out_Row;
                            iSuckC=j;
                            if(bSuckDuplicateErr[iSuckR][iSuckC])
                            {
                                InArmSuck.PordRec[iSuckR][iSuckC].AddErrorRecord("JAM0112");
                                if(CosFunction.bUseSCKART)
                                    fSCKART->iInputJamCnt++;

                                MOT[MInRotateKit].SetTraySingleData(iTrayX, iTrayY, NULL_IC);
                                bSuckDuplicateErr[iSuckR][iSuckC]=false;
                                InArmSuck.SetItemData(iSuckR, iSuckC, HAS_NULL_IC);
                            }
                        }
                    }
                    else if(iRotate_Type==e2MotRotate2Dut)                      //wei 20190515 modify 2 Dut 2 Motor
                    {
                        if(InArmSuck.iPickCol==1)
                        {
                            for(int j=iKit; j<OutArmSuck.iMaxCol; j+=4)
                            {
                                iTrayY=0;
                                iTrayX=0;
                                iSuckR=iRotato_Out_Row;
                                iSuckC=j;
                                if(bSuckDuplicateErr[iSuckR][iSuckC])
                                {
                                    InArmSuck.PordRec[iSuckR][iSuckC].AddErrorRecord("JAM0112");
                                    if(CosFunction.bUseSCKART)
                                        fSCKART->iInputJamCnt++;

                                    MOT[MInRotateKit].SetTraySingleData(iTrayX, iTrayY, NULL_IC);
                                    bSuckDuplicateErr[iSuckR][iSuckC]=false;
                                    InArmSuck.SetItemData(iSuckR, iSuckC, HAS_NULL_IC);
                                }
                            }
                        }
                        else if(bUseAxxGPicker() ||
                                iCloseSiteModeFor1x4==e1x4CloseAbAc)            //Steven 20241111 : for 1x4 close 2 site
                        {
                            for(int j=0; j<2; j++)
                            {
                                iTrayY=0;
                                iTrayX=0;
                                iSuckR=iRotato_Out_Row;
                                iSuckC=j*3;
                                if(bSuckDuplicateErr[iSuckR][iSuckC])
                                {
                                    InArmSuck.PordRec[iSuckR][iSuckC].AddErrorRecord("JAM0112");
                                    if(CosFunction.bUseSCKART)
                                        fSCKART->iInputJamCnt++;

                                    MOT[MInRotateKit].SetTraySingleData(iTrayX, iTrayY, NULL_IC);
                                    bSuckDuplicateErr[iSuckR][iSuckC]=false;
                                    InArmSuck.SetItemData(iSuckR, iSuckC, HAS_NULL_IC);
                                }
                            }
                        }
                        else
                        {
                            for(int j=0; j<2; j++)
                            {
                                iTrayY=0;
                                iTrayX=0;
                                iSuckR=iRotato_Out_Row;
                                iSuckC=j*2+iKit;
                                if(bSuckDuplicateErr[iSuckR][iSuckC])
                                {
                                    InArmSuck.PordRec[iSuckR][iSuckC].AddErrorRecord("JAM0112");
                                    if(CosFunction.bUseSCKART)
                                        fSCKART->iInputJamCnt++;

                                    MOT[MInRotateKit].SetTraySingleData(iTrayX, iTrayY, NULL_IC);
                                    bSuckDuplicateErr[iSuckR][iSuckC]=false;
                                    InArmSuck.SetItemData(iSuckR, iSuckC, HAS_NULL_IC);
                                }
                            }
                        }
                    }
                    else
                    {
                        for(int i=0; i<iMAX_ARM_Row; i++)
                        {
                            for(int j=0; j<iMAX_ARM_Col; j++)
                            {
                                if(W906RKIN_DUTNUM == tDutType_8)               //kevin 20130517 add 8 site
                                    j2=j;
                                else
                                    j2=j*i2x2Suck+iKit;
                                if(bSuckDuplicateErr[i][j2])                    //kevin 20140220  4 Dut rotate SKYPE會有問題
                                {
                                    InArmSuck.PordRec[i][j2].AddErrorRecord("JAM0112");
                                    if(CosFunction.bUseSCKART)
                                        fSCKART->iInputJamCnt++;

                                    MOT[MInRotateKit].SetTraySingleData(j, i, NULL_IC);
                                    bSuckDuplicateErr[i][j2]=false;             //kevin 20130524
                                    InArmSuck.SetItemData(i, j2, HAS_NULL_IC);
                                }
                            }
                        }
                    }

                    for(int i=0; i<MAX_ARM_Row; i++)
                    {
                        for(int j=0; j<MAX_ARM_Col; j++)
                        {
                            InArmSuck.Suck[i][j].Error=false;
                            bSuckDuplicateErr[i][j]=false;
                        }
                    }
                    Task=4300;
                    break;                                                      //kevin 20130524
                }

                Task=4000;

                if(ret==K_RETRY && iRetryCT>ArmSpeed[InArm].iRetryCT)           //Sam 20250206 : Rotate pick error retry for setting
                {
                    Task=4700;
                }
            }
            break;
        case 4700:
            SetInArmHome();
            iRetryCT=0;
            Task=4710;                                                          //AI(W906-I04) 20261001 (Ifor01): golden :3246 是 Task=4000。改得跟 golden 不一樣：Jimmy 1001 13:2x 裁決（RULINGS_20261001 第 15 條；Ifor 0922 在 V912 插 case 4710 的修法），先到下一行的 case 4710 把 Z 移到安全高度
            break;        case 4710: if(MoveInArmZToPlateSafe(Task)) { Task=4000; } break;   //AI(W906-I04) 20261001 (Ifor01): 改得跟 golden 不一樣：Jimmy 1001 13:2x 裁決（RULINGS_20261001 第 15 條；Ifor 0922 在 V912 插 case 4710 的修法）。SetInArmHome() 把 Z 留在原點（0），不是 Prod.ZInArmSafe；case 4000 的 M_MoveInArmXY_ToRotateKIT() 先做 CheckInArmZ()（ainarm2.cpp:734：Z 不剛好在 ZInArmSafe 就 SetInArmHome() 並成立；GATE (9) 同一顆解開，:376）⇒ golden 在這裡一直歸零 Z、點動 pitch，要按 HOME 才停。重試前先把 Z 移回安全高度（ainarm2.cpp:4634），到了才回 4000。ctest RotateKitRetry
        case 4800:                                                              //旋轉失敗
            if(MoveInArm2XYToWait()==false)                                     //如果要移開,而且還沒移到定位
            {
                return false;                                                   //先離開等In Arm
            }
            Task=4801;
            break;
        case 4801:                                                              //旋轉失敗
            ret=ShowErrorMessage("JAM0453", K_RETRY, MInRotateKit);             //kevin 20130524

            if(ret==K_RETRY)
            {
                Task=4810;
            }
            break;
        case 4810:
            FrmRotate->bRotateInHome=true;                                      //kevin 20130706
            FrmRotate->InitialInRotateHome();                                   //Steven 20170329 (wei) : Add individual rotate motor
            Task=4820;
            break;
        case 4820:
            if(FrmRotate->bRotateInHome==false)                                 //kevin 20130706
            {
                //AI(W906-ROTKIT-IN) 20260807: GATE (8) -- TFrmRotate::SetInRotateSpeed, same gap as
                //  golden :1411.  `Task=2600` below stays ACTIVE.
#if 0 // TODO(W7): GATE (8) -- golden :3271 TFrmRotate::SetInRotateSpeed -- absent from forms/fRotate.h stand-in
                FrmRotate->SetInRotateSpeed(ArmSpeed[InRotate].iACDCBodySP, ArmSpeed[InRotate].iBodySP);    //Steven 20170329 (wei) : Add individual rotate motor
#endif
                Task=2600;
            }
            break;
    }
    return false;
}
//---------------------------------------------------------------------------
bool M_DoShakeInArmRotateKIT()                                                  //JerryYang 20160825 Rotate sensor偵測異常,要先試著旋轉三次再跳alarm
{
    int &iTask=iShakeInArmRotateKit;
    int ret, iShakeLimitNum=3;

    if(USE_ROTATE_KIT==1 && iRotate_Type==e1MotRotate)
    {
        switch(iTask)
        {
            case 1:
                ret=SetMotorResolution(InAngle45, InAngle90, true);
                ret=ret-100;                                                    //Rotate旋轉功能,反轉4.5度
                if(MOT[MInRotateKit].MotorMove(ret))
                    iTask=10;
                break;
            case 10:
                ret=SetMotorResolution(InAngle45, InAngle90, true);             //Rotate旋轉功能,正轉4.5度
                ret=ret+100;
                if(MOT[MInRotateKit].MotorMove(ret))
                {
                    iInArmShakeCnt++;
                    if(iInArmShakeCnt>=iShakeLimitNum)                          //完成旋轉三次
                    {
                        iInArmShakeCnt=0;
                        iTask=20;
                    }
                    else
                    {
                        iTask=1;                                                //還沒轉完要繼續轉
                    }
                }
                break;
            case 20:
                ret=SetMotorResolution(InAngle45, InAngle90, true);             //回到原本的角度
                if(MOT[MInRotateKit].MotorMove(ret))
                    return true;
        }
    }
    else
    {
        //Steven 20170329 (wei) : Add individual rotate motor
        //==>
        switch(iTask)
        {
            case 1:
                MoveInRotateToDegreeAtSameTime(0, true);                        //Steven 20170504 : 針對獨立馬達旋轉站, 一起旋轉到特定角度
                iTask=10;
                break;
            case 10:
                ret=MoveInRotateToDegreeAtSameTime(0);                          //Steven 20170504 : 針對獨立馬達旋轉站, 一起旋轉到特定角度
                if(ret)
                {
                    iInArmShakeCnt++;
                    if(iInArmShakeCnt>=iShakeLimitNum)                          //完成旋轉三次
                    {
                        iInArmShakeCnt=0;
                        return true;
                    }
                    else
                    {
                        iTask=20;                                               //還沒轉完要繼續轉
                    }
                }
                break;
            case 20:
                MoveInRotateToDegreeAtSameTime(0, true);                        //Steven 20170504 : 針對獨立馬達旋轉站, 一起旋轉到特定角度
                iTask=30;
            case 30:
                ret=MoveInRotateToDegreeAtSameTime(90);                         //Steven 20170504 : 針對獨立馬達旋轉站, 一起旋轉到特定角度
                if(ret)
                    iTask=1;
                break;
        }
        //<==
        //Steven 20170329 (wei) : Add individual rotate motor
    }
    return false;
}
//---------------------------------------------------------------------------
//Steven 20170329 (wei) : Add individual rotate motor
//---------------------------------------------------------------------------
int DoRotate_In(int &rCount, bool bReset)
{
    int iRet=0;

    if(USE_ROTATE_KIT==1)
    {
        if(iRotate_Type==e1MotRotate1Dut)                                    //Frank 20180512 (Steven) : modify 1 Dut 1 Motor
        {
            iRet=M1_DoInRotateMove1Mot(rCount, bReset);
        }
        else if(iRotate_Type==e1MotRotate || iRotate_Type==eInOutArm1Motor)     //Frank 20250812 : modify InOutArm1Motor //Ifor 20251023 Fix In/Out Arm Rotate 異常
        {
            iRet=M1_DoInRotateMove(rCount, bReset);
        }
        else if(iRotate_Type==e2MotRotate2Dut)
        {
            iRet=M2_DoInRotateMove(rCount, bReset);
        }
        else if(iRotate_Type==e4MotRotate)
        {
            iRet=M4_DoInRotateMove(rCount, bReset);
        }
        else if(iRotate_Type==e8MotRotate)
        {
            iRet=M8_DoInRotateMove(rCount, bReset);;
        }
    }
    else
    {
        iRet=1;
    }

    return iRet;
}
//---------------------------------------------------------------------------
int M1_DoInRotateMove(int &rCount, bool bReset)
{
    int iResult     =0;
    int iAngle      =0;
    int iPos        =0;
    int iBacklash   =0;
//    int iNowPos     =0;
    int iAOIAngle   =0;
//    static bool bDoHomeFlag=false;
//    int iCount      =-1;
//    static bool QQ  =false;              //Frank 20190813 add

    if(bReset)
    {
        rCount=Prod.RotationTimeIn;                                             //Ifor 20170412 (Steven) RotationTime ==> RotationTimeIn   //jou 20231122 : Rotate Use RT mode
        CheckInRotateSensor(true);                                              //Steven 20170504 (wei) : 針對入料旋轉站的Sensor檢查
//        QQ=false;
        return iResult;
    }

    //jou 20231122 : Rotate Use RT mode
    if(Prod.RotationCount[rCount-1]!=0)                                         //20140318    如果設為大於0才進入的話-90度會不會旋轉
    {
        iAngle=(int)Prod.RotationCount[rCount-1]/90;
    }

    //Ifor 20211220 add:旋轉角度270度時改反轉減少旋轉時間
    if(iAngle==3)
        iAngle=-1;
    else if(iAngle==-3)
        iAngle=1;

    //AI(W906-ROTKIT-IN) 20260807: GATE (12) -- FrmAOI->ttbInsp->iRotateKitAngOffset_In has no
    //  home in this tree (forms/fAOI.h's TFrmAOI stand-in carries only bSimulateTopBtm).  The
    //  WHOLE two-line golden `if` is parked -- gating only the assignment would silently make
    //  the next statement the if-body.  iAOIAngle keeps its initialiser 0, which is exactly
    //  what golden computes on any machine that is not in Top/Bottom AOI inspection mode.
#if 0 // TODO(W7): GATE (12) -- golden :3427-3428 FrmAOI->ttbInsp->iRotateKitAngOffset_In -- absent from forms/fAOI.h stand-in
    if(USE_Scanner_AOI_Inspection==(int)eBtnAOI_TopBottomInstall)
        iAOIAngle=FrmAOI->ttbInsp->iRotateKitAngOffset_In;
#endif

    iPos=(InAngle90*iAngle)+Prod.iIn_iRotateA+ iAOIAngle;
    iBacklash=MOT[MInRotateKit].GetRotatorBacklash(iPos, true);                 //RogerYang 20260113 : Rotator新增背隙補償
    iPos+=iBacklash;

//    iCount=CheckInRotateSensor();

//    if(iCount==-100)
//    {
//        QQ=true;
//    }

    if(iRotate_Type==eInOutArm1Motor && iAngle!=0)                              //Ifor 20251204 add:rotate Offset
    {
        iPos=iPos+W906RKIN_TROT_IROTATEOFFSET(0);
    }

    if(MOT[MInRotateKit].MotorMove(iPos))
    {
        //int btest=MOT[MInRotateKit].iLastRotatorDirP;                           //RogerYang 20260113 : Rotator新增背隙補償  //Eastsun 20260522 W8004
        if(iRotate_Type==eInOutArm1Motor)                                       //Frank 20250812 : modify InOutArm1Motor
            iResult=1;
        else
            iResult=CheckInRotateSensor();                                      //Steven 20170504 (wei) : 針對入料旋轉站的Sensor檢查
    }

//    if(iResult==1 && QQ==true)          //Frank 20190813 add
//    {
//        iResult=-100;
//    }
    return iResult;
}
//---------------------------------------------------------------------------
//V3.27C.523 Steven 20170504 (wei) : For 2顆獨立旋轉馬達同時轉動
//---------------------------------------------------------------------------
int M1_DoInRotateMove1Mot(int &rCount, bool bReset)
{
    int iResult =1;
    int iAngle  =0;
    int iPos    =0;

    if(bReset)
    {
        CheckInRotateSensor(true);                                              //Steven 20170504 (wei) : 針對入料旋轉站的Sensor檢查
        return iResult;
    }

    if(MOT[MInRotateKit].Tray.Data[0][0]!=NULL_IC && MOT[MInRotateKit].Tray.Data[0][0]!=HAS_NULL_IC &&
       MOT[MInRotateKit].Tray.iNeedRotAng[0][0]!=MOT[MInRotateKit].Tray.iCurrRotAng[0][0])
        iAngle=MOT[MInRotateKit].Tray.iNeedRotAng[0][0]/90;
    else
        iAngle=MOT[MInRotateKit].Tray.iCurrRotAng[0][0]/90;

    iPos=(InAngle90*iAngle)+Prod.iIn_iRotateA;
    rCount=iAngle;
    iResult=MOT[MInRotate[0][0]].MotorMove(iPos);

    if(iResult!=0)
    {
        MOT[MInRotateKit].Tray.iCurrRotAng[0][0]=MOT[MInRotateKit].Tray.iNeedRotAng[0][0];
        iResult=CheckInRotateSensor();                                          //Steven 20170504 (wei) : 針對入料旋轉站的Sensor檢查
    }

    return iResult;
}
//---------------------------------------------------------------------------
//V3.27C.523 Steven 20170504 (wei) : For 2顆獨立旋轉馬達同時轉動
//---------------------------------------------------------------------------
int M2_DoInRotateMove(int &rCount, bool bReset)
{
    static bool bM8Result[2][4]={false};
    int j2;
    int iResult =1;
    int iAngle  =0;
    int iPos    =0;
    int iCol    =2;

    if(bReset)
    {
        for(int i=0; i<2; i++)
        {
            for(int j=0; j<4; j++)
            {
                bM8Result[i][j]=false;
            }
        }
        CheckInRotateSensor(true);                                              //Steven 20170504 (wei) : 針對入料旋轉站的Sensor檢查
        return iResult;
    }

    for(int i=0; i<1; i++)
    {
        for(int j=0; j<iCol; j++)
        {
            j2=j*2;
            if(bM8Result[i][j2]==false)
            {
                if(MOT[MInRotateKit].Tray.Data[j][i]!=NULL_IC &&
                   MOT[MInRotateKit].Tray.Data[j][i]!=HAS_NULL_IC &&
                   MOT[MInRotateKit].Tray.iNeedRotAng[j][i]!=MOT[MInRotateKit].Tray.iCurrRotAng[j][i])
                    iAngle=MOT[MInRotateKit].Tray.iNeedRotAng[j][i]/90;
                else
                    iAngle=MOT[MInRotateKit].Tray.iCurrRotAng[j][i]/90;

                iPos=(InAngle90*iAngle)+Tech.iInRotate[i][j2];
                bM8Result[i][j2]=MOT[MInRotate[i][j2]].MotorMove(iPos);

                if(bM8Result[i][j2]==false)
                {
                    iResult=0;
                }
                else
                {
                    MOT[MInRotateKit].Tray.iCurrRotAng[j][i]=MOT[MInRotateKit].Tray.iNeedRotAng[j][i];
                }
            }
        }
    }

    if(iResult!=0)
    {
        iResult=CheckInRotateSensor();                                          //Steven 20170504 (wei) : 針對入料旋轉站的Sensor檢查
    }

    return iResult;
}
//---------------------------------------------------------------------------
//V3.27C.523 Steven 20170504 (wei) : For 4顆獨立旋轉馬達同時轉動
//---------------------------------------------------------------------------
int M4_DoInRotateMove(int &rCount, bool bReset)
{
    static bool bM8Result[2][4]={false};
//    static bool QQ=false;
//    int iCount      =-1;
    int j2;
    int iResult =1;
    int iAngle  =0;
    int iPos    =0;

    if(bReset)
    {
        for(int i=0; i<2; i++)
        {
            for(int j=0; j<4; j++)
            {
                bM8Result[i][j]=false;
            }
        }
        CheckInRotateSensor(true);                                              //Steven 20170504 (wei) : 針對入料旋轉站的Sensor檢查
//        QQ=false;
        return iResult;
    }

//    iCount=CheckInRotateSensor();

//    if(iCount==-100)
//    {
//        QQ=true;
//    }

    for(int i=0; i<2; i++)
    {
        for(int j=0; j<2; j++)
        {
            j2=j*2;
            if(bM8Result[i][j2]==false)
            {
                if(MOT[MInRotateKit].Tray.Data[j][i]!=NULL_IC && MOT[MInRotateKit].Tray.Data[j][i]!=HAS_NULL_IC &&
                   MOT[MInRotateKit].Tray.iNeedRotAng[j][i]!=MOT[MInRotateKit].Tray.iCurrRotAng[j][i])
                    iAngle=MOT[MInRotateKit].Tray.iNeedRotAng[j][i]/90;
                else
                    iAngle=MOT[MInRotateKit].Tray.iCurrRotAng[j][i]/90;

                iPos=(InAngle90*iAngle)+Tech.iInRotate[i][j2];

                bM8Result[i][j2]=MOT[MInRotate[i][j2]].MotorMove(iPos);
                if(bM8Result[i][j2]==false)
                {
                    iResult=0;
                }
                else
                {
                    MOT[MInRotateKit].Tray.iCurrRotAng[j][i]=MOT[MInRotateKit].Tray.iNeedRotAng[j][i];
                }
            }
        }
    }

    if(iResult!=0)
    {
        iResult=CheckInRotateSensor();  //Steven 20170504 (wei) : 針對入料旋轉站的Sensor檢查
    }

//    if(iResult==1 && QQ==true)
//    {
//        iResult=-100;
//    }
    return iResult;
}
//---------------------------------------------------------------------------
//V3.27C.523 Steven 20170504 (wei) : For 8顆獨立旋轉馬達同時轉動
//---------------------------------------------------------------------------
int M8_DoInRotateMove(int &rCount, bool bReset)
{
    static bool bM8Result[2][4]={false};
//    static bool QQ=false;
//    int iCount      =-1;
    int j2;
    int iResult =1;
    int iAngle  =0;
    int iPos    =0;

    if(bReset)
    {
        for(int i=0; i<2; i++)
        {
            for(int j=0; j<4; j++)
            {
                bM8Result[i][j]=false;
            }
        }
        CheckInRotateSensor(true);  //Steven 20170504 (wei) : 針對入料旋轉站的Sensor檢查
//        QQ=false;
        return iResult;
    }

//    iCount=CheckInRotateSensor();

//    if(iCount==-100)
//    {
//        QQ=true;
//    }

    for(int i=0; i<2; i++)
    {
        for(int j=0; j<4; j++)
        {
            j2=j;
            if(bM8Result[i][j2]==false)
            {
                if(MOT[MInRotateKit].Tray.Data[j][i]!=NULL_IC && MOT[MInRotateKit].Tray.Data[j][i]!=HAS_NULL_IC &&
                   MOT[MInRotateKit].Tray.iNeedRotAng[j][i]!=MOT[MInRotateKit].Tray.iCurrRotAng[j][i])
                    iAngle=MOT[MInRotateKit].Tray.iNeedRotAng[j][i]/90;
                else
                    iAngle=MOT[MInRotateKit].Tray.iCurrRotAng[j][i]/90;

                iPos=(InAngle90*iAngle)+Tech.iInRotate[i][j2];

                bM8Result[i][j2]=MOT[MInRotate[i][j2]].MotorMove(iPos);
                if(bM8Result[i][j2]==false)
                {
                    iResult=0;
                }
                else
                {
                    MOT[MInRotateKit].Tray.iCurrRotAng[j][i]=MOT[MInRotateKit].Tray.iNeedRotAng[j][i];
                }
            }
        }
    }

    if(iResult!=0)
    {
        iResult=CheckInRotateSensor();  //Steven 20170504 (wei) : 針對入料旋轉站的Sensor檢查
    }

//    if(iResult==1 && QQ==true)
//    {
//        iResult=-100;
//    }
    return iResult;
}
//---------------------------------------------------------------------------
//V3.27C.523 Steven 20170504 (wei) : 針對獨立馬達旋轉站, 一起旋轉到特定角度
//---------------------------------------------------------------------------
bool MoveInRotateToDegreeAtSameTime(int iDegree, bool bInitial)
{
    static bool bMoveFinish[2][4];
    int buffer, j2;
    bool bResult=true;

    if(bInitial)
    {
        for(int i=0; i<2; i++)
        {
            for(int j=0; j<4; j++)
            {
                bMoveFinish[i][j]=false;
            }
        }
        return bResult;
    }

    if(USE_ROTATE_KIT==1 &&
       (iRotate_Type==e1MotRotate ||
        iRotate_Type==e1MotRotate1Dut ||
        iRotate_Type==eInOutArm1Motor))                                         //Frank 20250812 : modify InOutArm1Motor
    {
        buffer=SetMotorResolution(InAngle45, InAngle90, true);
        if(MOT[MInRotateKit].MotorMove(buffer))                                 //旋轉成和shuttle一樣方向
        {
            for(int i=0; i<2; i++)
            {
                for(int j=0; j<4; j++)
                {
                    MOT[MInRotateKit].Tray.iCurrRotAng[j][i]=iDegree;
                }
            }
        }
        else
        {
            bResult=false;
        }
    }
    else
    {
        for(int i=0; i<2; i++)
        {
            for(int j=0; j<4; j++)
            {
                if(USE_ROTATE_KIT==1 && iRotate_Type==e4MotRotate)
                {
                    if(j>=2)
                    {
                        continue;
                    }
                    j2=j*2;
                }
                else if(USE_ROTATE_KIT==1 && iRotate_Type==e2MotRotate2Dut)     //wei 20190515 modify 2 Dut 2 Motor
                {
                    j2=j*2;
                    if(j>=2)
                    {
                        continue;
                    }

                    if(i>=1)
                    {
                        continue;
                    }
                }
                else
                {
                    j2=j;
                }

                if(bMoveFinish[i][j]==false)
                {
                    //日本Tamagawa 步進馬達, 每2000 pluse = 轉90度
                    if(USE_ROTATE_KIT==1 && iRotate_Type==e1MotRotate)
                        buffer=Tech.iInRotate[i][j2]+2000*iDegree;
                    else
                        buffer=Tech.iInRotate[i][j2]+800*iDegree;
                    if(MOT[MInRotate[i][j2]].ReadPos()!=buffer)
                    {
                        bResult=false;
                        bMoveFinish[i][j]=MOT[MInRotate[i][j2]].MotorMove(buffer);
                    }
                    else
                    {
                        bMoveFinish[i][j]=true;
                    }
                }

                if(bMoveFinish[i][j]==true)
                {
                    MOT[MInRotateKit].Tray.iCurrRotAng[j][i]=iDegree;
                }
            }
        }
    }

    return bResult;
}
//---------------------------------------------------------------------------
bool CheckInRotateAnglePostion(int iSuckR, int iSuckC, int iTrayX, int iTrayY)
{
    bool bResult=false;
    if(Prod.RotateDutDate[0][iSuckR][iSuckC]==MOT[MInRotateKit].Tray.iCurrRotAng[iTrayX][iTrayY])
        bResult=true;

    return bResult;
}
//---------------------------------------------------------------------------
//V3.27C.523 Steven 20170504 (wei) : 針對入料旋轉站的Sensor檢查
//---------------------------------------------------------------------------
int CheckInRotateSensor(bool bReset)
{
    static int iCount=0;
    int iResult=0;

    if(bReset)
    {
        iCount=0;
        return iResult;
    }

    iCount++;
    //Ifor 20180110 (Steven) : add 僅有一個Rotate sensor
    //==>
    if(Sen[SnRotateRowIn1].Enable==true &&
       Sen[SnRotateRowIn2].Enable==true &&
       Sen[SnRotateRowIn1].IsOn()       &&
       Sen[SnRotateRowIn2].IsOn()       )
    {
        iResult=1;
        iCount=0;
    }
    else if(Sen[SnRotateRowIn1].Enable==true    &&
            Sen[SnRotateRowIn2].Enable==false   &&
            Sen[SnRotateRowIn1].IsOn()          )
    {
        iResult=1;
        iCount=0;
    }
    else if(Sen[SnRotateRowIn1].Enable==false)
    {
        iResult=1;
        iCount=0;
    }
    else
    {
        iCount++;
        if(iCount>50)
        {
            iCount=0;
            iResult=-999;     //Alarm
        }

//        if(iCount>20)
//        {
//            iResult=-100;     //Alarm
//        }
    }
    //<==
    //Ifor 20180110 (Steven) : add 僅有一個Rotate sensor
//    if(Sen[SnRotateRowIn1].Enable==false)
//    {
//        iResult=1;
//        iCount=0;
//    }
//    else if(Sen[SnRotateRowIn1].IsOn() && Sen[SnRotateRowIn2].IsOn())
//    {
//        iResult=1;
//        iCount=0;
//    }
//    else
//    {
//        iCount++;
//        if(iCount>50)
//        {
//            iCount=0;
//            iResult=-999;     //Alarm
//        }
//    }

    return iResult;
}
//---------------------------------------------------------------------------
void RecordRotateAngle(int iSuckR, int iSuckC, int iTrayR, int iTrayC)          //Sam 20221103 : 新增In旋轉角度
{
    int iAngle=0;
    AnsiString sInArmAngleRecord="";
    sInArmAngleRecord=InArmSuck.PordRec[iSuckR][iSuckC].GetInRotationAngRecord();
    if(sInArmAngleRecord=="")                                                   //確認此 IC 之前沒有被旋轉過
    {
        iAngle=Prod.RotateDutDate[0][iTrayR][iTrayC];
        MOT[MInRotateKit].Tray.PordRec[iTrayC][iTrayR]->AddInRotationAngRecord(iAngle);
        if(iAngle==360)
            ShowMyMessage("Rotate Data error . Please call engineer", "旋轉角度錯誤，請通知工程師處理");
    }
    else                                                                        //若有被旋轉過需要加上之前旋轉的角度
    {
        ShowMyMessage("Rotate Data error . Please call engineer", "多次旋轉錯誤，請通知工程師處理");
        iAngle=Prod.RotateDutDate[0][iTrayR][iTrayC]+StrToInt(sInArmAngleRecord);
        MOT[MInRotateKit].Tray.PordRec[iTrayC][iTrayR]->AddInRotationAngRecord(iAngle);
    }
}
//---------------------------------------------------------------------------
