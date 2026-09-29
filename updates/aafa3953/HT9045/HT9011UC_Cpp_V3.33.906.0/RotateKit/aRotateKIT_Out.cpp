// =============================================================================
//  RotateKit/aRotateKIT_Out.cpp  --  OUT-side rotate-kit (旋轉站) state machines
//
//  Faithful translation of golden RotateKit/aRotateKIT_Out.cpp (4039 lines,
//  BCB6, Big5/cp950).
//  Translator: AI(W906-RKOUT) 20260807
//  Translation wave: W906-RKOUT (pure-translation completion campaign).  This is
//  the OUT-side mirror of RotateKit/aRotateKIT_In.cpp; it was translated line by
//  line from the GOLDEN OUT FILE and NOT copied from the In file -- the two
//  differ in cursor values, tray indexing, the Got*RotateKit* helpers, the
//  1-Dut / 2-Dut branches and the whole cylinder rig.
//
//  ROLE: the out-arm rotate-kit (IC 出料旋轉站) owner.  Everything the out-arm
//  does between "picked from shuttle" and "place to Auto/Fix tray" when the
//  device must be rotated lives here:
//    * cursors        : iOutArmRotateKit (main SM), iShakeOutArmRotateKit (shake
//                       SM), iOutArmShakeCnt, iOutRotateWhichKit, OutAngle45/90,
//                       OutArmRotate_PitchY.
//    * entry points   : InitialOutArmRotateKIT / DoOutArmRotateKIT (the topology
//                       dispatcher: cylinder rig vs motor rig vs in/out-1-motor).
//    * cylinder rig   : DoOutArmRotateKIT_Cylinder + MoveOutArmXY_ToRotateKIT /
//                       MoveOutArmZ_ToRotateKIT_Place / MoveOutArmZ_ToRotateKIT_Pick.
//    * motor rig      : M_DoOutArmRotateKIT_Motor + M_MoveOutArmXY_ToRotateKIT /
//                       M_MoveOutArmZ_ToRotateKIT_Place / M_MoveOutArmZ_ToRotateKIT_Pick
//                       + CalcPosition_OutArm + AutoCalculateOutArmRotatePitch_Y.
//    * kit selectors  : GotOutRotateKitForSingle / ForAxxG / ForAxEx_2RotMot.
//    * rotate movers  : DoRotate_Out -> M1_/M1_1Mot/M2_/M4_/M8_DoOutRotateMove,
//                       MoveOutRotateToDegreeAtSameTime, M_DoShakeOutArmRotateKIT.
//    * predicates     : CheckOutRotateSensor, CheckOutRotateAnglePostion,
//                       CheckRotateOutAnglePostion, CheckRotateOutNotFinish.
//    * one-motor rig  : M_DoOutArmRotate_InOutArm.
//
//  WAVE SCOPE -- ACTIVE vs satisfied-by-shim:
//   ACTIVE (faithful, verbatim): ALL 29 golden top-level definitions.  Every
//     switch(Task) cursor value, every intentional fall-through, every integer
//     truncation and every golden defect is transcribed as-is.
//   SATISFIED BY THE SIBLING UNIT (landed this same wave):
//     RotateKit/aRotateKIT.{h,cpp} -- RotateKIT_FormSet / SetMotorResolution /
//     CheckRotateSensor / i2x2Suck / i2x2Suck_Out / the eight iRotateKIT_* teach
//     globals, all reached through `#include "aRotateKIT.h"` exactly as golden
//     does.  ORDERING CONSTRAINT for CMake: aRotateKIT.cpp must be in the same
//     target (it is; see the report's CMake fragment).
//   SATISFIED BY EXISTING SUBSTRATE (no shim written by this wave): MOT[] /
//     Cylinder[] / Sen[] (Sim HAL), OutArmSuck+InArmSuck (aHotPlateSubstrate.h),
//     Prod/TestIF/TrayForm/DeviceForm/ArmSpeed (cprod.h), Tech (LastSet.h),
//     CosFunction (cpublic.h), IniConfig (Config.h), fMain/fNote/fSCKART
//     (FormsFacade.h), FrmRotate (forms/fRotate.h), FrmAOI (forms/fAOI.h),
//     ShowErrorMessage/__FUNC__/K_RETRY/K_SKIP (canary_support.h),
//     MoveOutArmToAutoSafe / MoveOutArmXY_ToShuttleAlarmArea (acarry_shims.h),
//     SetOutArmHome (aoutarm_shims.cpp), CheckOutArmSuckICFallDown /
//     InitDoArmZHome (csystem.h), GetOutArmPitchX|Y_9045 (aoutarm9045.h).
//   GATE (#if 0): 10 numbered gates + 1 parked definition.  Register below.
//     Every gate is individually re-cited at its call site.
//
//  ---------------------------------------------------------------------------
//  ## A. GATE REGISTER -- 11 entries (10 behaviour gates + 1 parked            ##
//  ##    definition).  Each is re-cited at its call site.                      ##
//  ---------------------------------------------------------------------------
//  Gates 1-6 share ONE root cause and it is NOT "not translated yet": in this
//  port tree the golden object `TRotate tRotate` (golden RotateKit/fRotate.h:148)
//  is represented by `struct tRotateShim` (aHotPlateSubstrate.h:824-832), which
//  carries only ActiveRotate / RotateDutDate / bRotateUseRTmode -- there is NO
//  STORAGE for DutNum, bPassBinNoRotate, RotateKit_PitchX, bUseDifferentAngle or
//  iRotateOffset, and the tDutType_* enum (golden fRotate.h:22-26) has no home
//  either.  Same story for TFrmRotate (forms/fRotate.h) and TFrmAOI
//  (forms/fAOI.h): both are deliberately minimal non-VCL stand-ins.  golden
//  RotateKit/fRotate.h and golden fAOI.h are FORM units, explicitly out of scope
//  for this non-form campaign, and this wave may not edit shared headers.  These
//  are therefore COMPILE errors, not link errors.
//  Re-declaring `tRotate` with a wider type in this TU would LINK (a global's
//  mangled name carries no type) and then read past the shim's real storage --
//  memory corruption dressed up as fidelity.  So each missing member is isolated
//  in ONE macro holding golden's expression VERBATIM inside `#if 0`, with an
//  ACTIVE default that mirrors golden's own default arm.  Every golden call site
//  keeps its exact golden shape (`W906RKO_DUTNUM==tDutType_8` reads the same as
//  golden's `tRotate.DutNum==tDutType_8`).  This is the SAME mechanism and the
//  SAME defaults the two sibling units chose (RotateKit/aRotateKIT.cpp:238-242,
//  RotateKit/aRotateKIT_In.cpp:329-394), so the three RotateKit files retire
//  together.
//
//   GATE 1  W906RKO_DUTNUM  <- `tRotate.DutNum`  (golden fRotate.h:32; 15 sites)
//           ACTIVE default `tDutType_4` (=0).  WHY: golden loads DutNum from the
//           rotate ini and writes its own fallback comment as
//           `else// if(tRotate.DutNum == tDutType_4)` (golden :797), i.e. 4-DUT
//           is the arm golden itself treats as the default.  Identical default to
//           aRotateKIT_In.cpp GATE (2).
//           BEHAVIOUR DELTA: yes, on an 8-DUT rotate kit.  Stated, not hidden.
//   GATE 2  W906RKO_TROT_PASSBINNOROTATE <- `tRotate.bPassBinNoRotate`
//           (golden fRotate.h, jou 20231020; 7 sites).  ACTIVE default false.
//           ZERO BEHAVIOUR DELTA: every golden use is
//           `CosFunction.bPassBinNoRotate==true && tRotate.bPassBinNoRotate==true`
//           and CosFunction.bPassBinNoRotate (CosFunction.h:406) is false unless
//           a customer profile sets it, so the conjunction is false either way.
//   GATE 3  W906RKO_ROTKIT_PITCHX <- `tRotate.RotateKit_PitchX` (golden :808).
//           ACTIVE default 0.0 -> `0.0==40` is false -> golden's else arm
//           `iXPitch = iRotateKIT_Pitch_X_H;` (golden :814).  Same default and
//           same type (golden's field is a double) as aRotateKIT_In.cpp GATE (3).
//   GATE 4  W906RKO_TROT_USEDIFFERENTANGLE <- `tRotate.bUseDifferentAngle`
//           (golden :3953).  ACTIVE default false -> golden's else arm
//           `dbTempAngle=0-Prod.RotateDutDate[0][iTrayY][iTrayX];` (golden :3959).
//   GATE 5  W906RKO_TROT_IROTATEOFFSET(i) <- `tRotate.iRotateOffset[i]`
//           (golden :3530, Ifor 20251204).  ACTIVE default 0 -- additive identity,
//           so golden's `iPos=iPos+tRotate.iRotateOffset[1];` becomes a no-op.
//   GATE 6  W906RKO_SHOWROTATEBYSITE <- `FrmRotate->bShowRotateBySite`
//           (golden RotateKit/fRotate.h:133; 4 sites).  forms/fRotate.h's
//           TFrmRotate stand-in landed only bRotateInHome / bRotateOutHome /
//           b8Rotate*Home / InitialIn|OutRotateHome (its own banner says so).
//           ACTIVE default false selects golden's non-per-site arm everywhere.
//   GATE 7  W906RKO_ARMCANSUCK4IC(...) <- `ArmCanSuck4IC(...)` (golden :806,
//           :830, :3056).  This one IS a link fact: golden csystem.cpp:723 is
//           DECLARED at ported csystem.h:162 but has NO compiled body anywhere in
//           this tree, so a live call is an undefined reference.  ACTIVE default
//           `true` -- byte-identical to the choice already made at
//           RotateKit/aRotateKIT.cpp:238-242 and aRotateKIT_In.cpp:382-387, so the
//           three files stay consistent and retire together.
//   GATE 8  FrmRotate->SetOutRotateSpeed(int,int) (golden RotateKit/fRotate.h;
//           the port stand-in deliberately did not land SetIn/SetOutRotateSpeed).
//           2 call sites (golden :1421 and :3363).  Inline `#if 0` on the CALL
//           only; the surrounding `if(FrmRotate->bRotateOutHome==false)` guard and
//           the Task transition stay ACTIVE, so the alarm-recovery SM converges.
//   GATE 9  FrmAOI->ttbInsp->iRotateKitAngOffset_Out (golden fAOI.h; golden
//           :3522).  forms/fAOI.h's TFrmAOI stand-in carries ONE member
//           (bSimulateTopBtm); there is no ttbInsp sub-object anywhere in the
//           tree.  Inline `#if 0`; `iAOIAngle` stays 0, exactly its value when the
//           Top/Bottom AOI angle offset is not configured.  Mirrors
//           aRotateKIT_In.cpp's gate (12) for iRotateKitAngOffset_In.
//   GATE 10 (PARKED DEFINITION, not a behaviour gate) `int iOutRotateFinish=0;`
//           golden :29 defines it HERE, but this tree already defines it at
//           aoutarm9045.cpp:185 -- verbatim, same initialiser -- and
//           RotateKit/aRotateKIT.h:36-37 records that fact.  Two definitions is a
//           `multiple definition` link error, so golden's line is kept verbatim
//           inside `#if 0`; the `extern int iOutRotateFinish;` at aRotateKIT.h:74
//           still resolves, to aoutarm9045.cpp's object.  ZERO BEHAVIOUR DELTA.
//           Exactly the treatment aRotateKIT_In.cpp gave iInRotateFinish.
//   GATE 11 fNote->bMyServoOffOutArm (golden note.h:417; golden :58 and :879).   //AI(W906-FLOW-5) 20260929: GATE 11 RETIRED at both sites (:466 :1318) -- bMyServoOffOutArm is on TfNote (forms/fNote.h:305); the note below is history
//           forms/fNote.h's TfNote stand-in landed bMyServoOffInArm but NOT
//           bMyServoOffOutArm, and forms/fNote.h:87-94 says that is deliberate --
//           it names ported aoutarm9045.cpp:388/:1460 as the other two sites and
//           records that both are already `#if 0 // TODO(W7)`.  Identical
//           treatment here: the `if(IniConfig.bAlarmNeedServoOff)` guard stays
//           ACTIVE, only the inner `if(fNote->bMyServoOffOutArm) return false;`
//           early-return is gated.  The ACTIVE path is golden's "no
//           alarm-triggered ServoOff recovery pending" arm -- the normal-running
//           one, and the only one reachable on a machine that has not just
//           alarmed.  Retires with the other two the moment the member lands.
//
//  ---------------------------------------------------------------------------
//  ## B. DUPLICATE-SYMBOL DECISIONS THAT NEED AN INTEGRATOR ACTION            ##
//  ---------------------------------------------------------------------------
//  D-1  `bool CheckRotateOutNotFinish()` -- aoutarm_shims.cpp:91 currently holds
//       an OFFLINE CONSTANT (`return false;`).  The real golden body (golden
//       :4028-4038) is translated here.  That shim line MUST be deleted in the
//       same commit that adds this file, else duplicate symbol.
//  D-2  `bool MoveOutRotateToDegreeAtSameTime(int,bool)` -- aoutarm_shims.cpp:156
//       likewise holds an offline constant (`return true;`).  Real body here
//       (golden :3813-3912); that shim line MUST be deleted in the same commit.
//       (D-1/D-2 are NOT parked like GATE 10 precisely because the existing
//        definitions are FAKE: parking would silently keep the fake behaviour.
//        GATE 10's existing definition is golden's own, so parking is free.)
//  D-3  `InitialOutArmRotateKIT()` / `DoOutArmRotateKIT()` appear in
//       aoutarm9045.cpp ONLY inside `#if 0 // TODO(W7)` blocks (:717-762,
//       :770-795).  No definition exists -> NO collision.  Landing this file
//       makes those two W7 gates ungate-able; that is a separate wave's call and
//       this file does not touch aoutarm9045.cpp.
//
//  ---------------------------------------------------------------------------
//  ## C. TWO ADAPTATIONS THAT ARE NOT GATES (behaviour-identical)             ##
//  ---------------------------------------------------------------------------
//  A-1  `#define SetNeedDestroy(v) bNeedDestroy = (v)`.  This tree's TMySucker
//       (aHotPlateSubstrate.h:147-151) landed the public DATA member
//       `bNeedDestroy` and the getter GetNeedDestroyStatus(), but not the setter.
//       golden MyKitSuck.h:122 is literally
//       `void SetNeedDestroy(bool Value) {bNeedDestroy=Value;}`, so the macro
//       expands to the very same assignment golden performs and every one of the
//       9 golden call sites below stays byte-identical to golden.  Same macro,
//       same reasoning, as aRotateKIT_In.cpp:404.  Retire by adding the one-line
//       setter to aHotPlateSubstrate.h and deleting the three lines.
//  A-2  `CheckOutArmZ(bool)` -- golden aoutarm.cpp:95-116, a unit not translated
//       and not assigned to this wave; the identifier exists nowhere in the port
//       (Motor/mymotor.h:450's CheckOutArmZNeedHome is a different function).
//       DELIBERATE DIVERGENCE FROM THE In SIBLING, WHICH GATED ITS CheckInArmZ TO
//       `false` (aRotateKIT_In.cpp:375-380): this TU instead carries a file-local
//       VERBATIM copy of golden aoutarm.cpp:95-116, the precedent
//       asendic_Auto.cpp:371-390 (`W7L1A_CheckOutArmZ`) set.  Reason: gating it to
//       false silently deletes an out-arm-Z SAFETY interlock (it is the guard that
//       raises bNeedArmZHome before an XY move), and every dependency of golden's
//       body exists here (InOutArmPickerUseMotor / eptUseMotCyn / MOutArmZA /
//       Prod.ZOutArmSafe / MOT[].Alias / MOutArmX).  It is `static`, so it cannot
//       collide when golden aoutarm.cpp lands; the `#define` keeps the two call
//       sites (golden :49 and :870) textually identical to golden.
//
//  ---------------------------------------------------------------------------
//  ## D. GOLDEN DEFECTS PRESERVED VERBATIM (never "fixed")                    ##
//  ---------------------------------------------------------------------------
//  E-1  golden :174 -- MoveOutArmZ_ToRotateKIT_Pick's non-1X4 arm moves Z to
//       `Prod.iOutArmRotateToUnloader_PlaceZ[i][j]` while every sibling line in
//       the same function uses ..._PickZ.  Copy/paste defect in the PICK path.
//  E-2  golden :368-379 -- case 3000 sets `Task=3100;` inside the `==1` arm and
//       then unconditionally again after the if/else.  Dead store; the `==2` arm
//       depends on the unconditional line.  Kept.
//  E-3  golden :273-274 -- case 1200 opens with
//       `if(bOut_XYMoveFinishOnRotationKit==false) Task=1200;` -- assigning Task
//       the value it already holds.  It does NOT block; execution falls straight
//       into the MoveOutArmZ call.  Kept.
//  E-4  golden :1619-1621 -- GotOutRotateKitForAxEx_2RotMot's 4th `else if` tests
//       `iCount==0`, already consumed by the 2nd `else if`.  UNREACHABLE, so
//       iCount can never become 2 on that path.  Kept.
//  E-5  golden :300 / :947 / :1980 ... -- `if(A.Item[i][j] && A.Suck[i][j].Get...()
//       || A.Item[i][j]==HAS_NULL_IC)` mixes && and || with no parentheses.
//       Kept exactly as written; no clarifying parens added.
//  E-6  golden :305 -- the CYLINDER rig writes `bInArmCheckDestroyACT[i][j2]`
//       (the IN-arm destroy-confirm grid) in an OUT-arm path; the motor rig at
//       golden :1954 writes bOutArmCheckDestroyACT.  Kept.
//  E-7  golden :3535 -- `int btest=MOT[MInRotateKit].iLastRotatorDirP;` inside
//       M1_DoOutRotateMove reads the IN rotate kit and is never used.  Retained
//       with an explicit `(void)btest;` per house rule, NOT deleted.
//  E-8  golden :2628 / :2658 / :3085 / :3601 -- these read `InArmSuck.iPickCol`
//       to steer OUT-arm rotate branches, while the matching place-side code
//       (golden :954 / :974 / :1727 / :1969) reads `OutArmSuck.iPickCol`.  The
//       asymmetry is golden's; kept exactly.
//  E-9  golden :934 / :1115 / :2598 / :3211 -- `for(int j=iKit; j<OutArmSuck.iMaxCol; j+=4)`
//       then indexes `flag[iSuckR][j]` where flag is [2][4].  If iMaxCol ever
//       exceeds 4 this writes out of bounds.  Golden's own risk; kept.
//  E-10 golden :3304 -- the generic SKIP arm computes `j2=j*i2x2Suck+iKit;`
//       (the IN-side stride) whereas every other OUT-side site uses i2x2Suck_Out.
//       Kept.
//  E-11 golden .h:16 declares `bool M_MoveOutArmXY_ToRotateKIT();` (zero-arg)
//       while golden .cpp:868 defines `(int iKit)`.  Two different overloads; the
//       zero-arg one is never defined and never called.  Both kept -- see the
//       header banner.
//  E-12 golden .h:32 spells CheckOutRotateAnglePostion's last two parameters
//       (iTrayY, iTrayX) while golden .cpp:3914 defines them (iTrayX, iTrayY).
//       Same types -> same function; only the names disagree.  Both kept.
//  E-13 golden :3484/:3580/:3665/:3739 take `int &rCount` and never read or write
//       it (M2/M4/M8 ignore it entirely).  Kept.
//
//  VCL/Borland conversions: #pragma hdrstop + #pragma package(smart_init) dropped
//  (golden :2 and :26).  AnsiString (1-based) via vclcompat, verbatim.
//  NO __fastcall / __property / __published in this .cpp.  SOFT_SIMULTE is NOT
//  defined; this golden file contains no #ifdef SOFT_SIMULTE block, so there is
//  nothing to reproduce inert here (golden aRotateKIT.cpp's CheckRotateSensor has
//  one, and that belongs to the sibling unit, which already carries it).
//  Numeric semantics kept EXACT: `(int)Prod.OutRotationCount[rCount-1]/90`,
//  `iRotateKIT_Pitch_X_H/2*3`, `iRotateKIT_Pitch_X_A*3/2`, `iXPitch/3`,
//  `MAX_ARM_Col/2` are all integer division/truncation and stay integer.
//
//  Big5: every Chinese comment decoded via cp950 and preserved as UTF-8.
//  Final gate: ZERO U+FFFD.
// =============================================================================
// BCB6 ORIGINAL include block (mirrored as a comment for provenance):
//   #include "MachineDefine.h" ; #pragma hdrstop
//   #include "aRotateKIT_Out.h"
//   #include "aRotateKIT.h"
//   #include "aArmHeader.h" / "cprod.h" / "csystem.h" / "mymotor.h"
//   #include "MyKitSuck.h" / "mycylin.h" / "mysensor.h" / "main.h" / "note.h"
//   #include "fRotate.h" / "uhome.h" / "cinitial.h" / "cMyDB.h" / "cmydef.h"
//   #include "SCK_ART.h" / "fAOI.h"
//   ; #pragma package(smart_init)
// =============================================================================
#include "MachineDefine.h"          // de-VCL'd include hub (vclcompat umbrella + <windows.h> ZeroMemory + STL)

#include "aRotateKIT_Out.h"         // this unit's own golden contract (resolves in RotateKit/)
#include "aRotateKIT.h"             // sibling shared contract: RotateKIT_FormSet / SetMotorResolution /
                                    //   CheckRotateSensor / i2x2Suck / i2x2Suck_Out / iRotateKIT_Start|Pitch_X|Y_H|A /
                                    //   extern iOutRotateFinish (see GATE 10)
#include "aArmHeader.h"             // golden's arm god-header -- guard-only shim in this tree (aArmHeader.h:8-23)
#include "cprod.h"                  // Prod / TestIF / TrayForm / DeviceForm / ArmSpeed[]
#include "cpublic.h"                // CosFunction (bOPCanPressStepAndTStart / bUseSCKART / bPassBinNoRotate)
#include "csystem.h"                // CheckOutArmSuckICFallDown / InitDoArmZHome ; ArmCanSuck4IC DECLARATION only -- see GATE 7
#include "Motor/mymotor.h"          // MOT[] (TTrayMotor + TMyTray) / OutArmContinuousMove_9045 / OutArm speed enum
#include "mysensor.h"               // Sen[] (SnRKManualStep / SnRotateRowOut1 / SnRotateRowOut2)
#include "mycylin.h"                // Cylinder[] (C_OutputRotateKIT: On/Off/Push/Pop)
#include "aHotPlateSubstrate.h"     // golden MyKitSuck.h -> OutArmSuck / InArmSuck (TMyKitSuck + TMySucker) ;
                                    //   golden fRotate.h data half -> tRotate (tRotateShim) ; bInArmCheckDestroyACT ;
                                    //   bUseAxxGPicker / iCloseSiteModeFor1x4
#include "aoutarm9045.h"            // GetOutArmPitchX|Y_9045 / bOutArmCheckDestroyACT
#include "acarry_shims.h"           // MoveOutArmToAutoSafe / MoveOutArmXY_ToShuttleAlarmArea
#include "cmydef.h"                 // global scalar universe: MOutRotateKit / MInRotateKit / MOutRotate[][] /
                                    //   OutArmZIndex[][] / iRotate_Type / USE_ROTATE_KIT / USE_PICKER_COUNT /
                                    //   USE_OUT_Y_IS_AUTO_PITCH / USE_Scanner_AOI_Inspection / iRotato_Out_Row /
                                    //   iRotate_Out_Tray6 / iOutArmXBase / iXpitch* / iInArmType / AccessLevel /
                                    //   bEnterOffset / bNeedArmZHome / bOut_XYMoveFinishOnRotationKit /
                                    //   bOut_ICRotationCompleteOnKit / InOutArmPickerUseMotor / MOutArmX / MOutArmZA /
                                    //   NULL_IC / HAS_NULL_IC
#include "MachineType.h"            // eRotateType / test-mode enum / OutArm|OutRotate speed parts / OutOfsAuto1 /
                                    //   OutOfsRotate_Out / MAX_ARM_Row|Col / X_PITCH_COUNT / ep16Picker / eptUseMotCyn /
                                    //   eBtnAOI_TopBottomInstall / e1x4CloseAbAc
#include "Config.h"                 // IniConfig (bAlarmNeedServoOff / bE34InOutArmPitchZOffsetSameOne /
                                    //            bA21RotateDetectErrNeedShake)
#include "FormsFacade.h"            // was main.h + note.h(form) + SCK_ART.h: fMain->Pause /
                                    //   fNote->bMyServoOffOutArm / fSCKART->AddOutputJamCnt
#include "forms/fRotate.h"          // FrmRotate (bRotateOutHome / InitialOutRotateHome) -- NOT re-exported by FormsFacade.h
#include "forms/fAOI.h"             // FrmAOI -- NOT re-exported by FormsFacade.h; see GATE 9
#include "canary_support.h"         // was note.h(fn) + cMyDB.h: ShowErrorMessage / K_RETRY / K_SKIP / __FUNC__ ;
                                    //   pulls LastSet.h -> TECH Tech (Tech.iOutRotate[2][4])

// -----------------------------------------------------------------------------
//  Out-arm engine surface whose golden home (aoutarm.h) is not translated yet but
//  whose DEFINITIONS already exist in this tree, so these link today.  Same
//  #ifndef-guarded forward-declaration idiom as aoutarm9045_1x1_1.cpp:288-300.
// -----------------------------------------------------------------------------
#ifndef HT9045_RKOUT_OUTARM_FWD
#define HT9045_RKOUT_OUTARM_FWD
extern int  iWhichAuto;                   // golden aoutarm.h  (defined at ported aoutarm9045.cpp:179)
void SetOutArmHome();                     // golden aoutarm.h:97 (defined at ported aoutarm_shims.cpp:99)
#endif

// =============================================================================
//  LOCAL SUBSTRATE  --  everything below exists ONLY because a shared surface
//  this unit needs is not on this tree's stand-in yet, and the shared header is
//  not this wave's file.  Each item is one of the numbered entries in the GATE
//  REGISTER above.  Nothing here invents behaviour -- every ACTIVE default is a
//  value or a branch golden itself produces.  Kept deliberately identical in
//  shape and in default to RotateKit/aRotateKIT_In.cpp:313-404 so the three
//  RotateKit files can be retired in one edit.
// =============================================================================

//AI(W906-RKOUT) 20260807: golden RotateKit/fRotate.h:21-25 `enum {tDutType_4..}`.
//  golden's fRotate.h is split in this tree (data -> aHotPlateSubstrate.h, form ->
//  forms/fRotate.h) and NEITHER half carries this enum.  Reproduced VERBATIM (same
//  enumerators, same values) in the .cpp, macro-guarded, so it cannot collide with
//  the identical block in aRotateKIT_In.cpp:329-336 nor with a future shared header.
#ifndef HT9045_TDUTTYPE_ENUM_DEFINED
#define HT9045_TDUTTYPE_ENUM_DEFINED
enum {tDutType_4=0,
      tDutType_8=1,
      tDutType_1=2,
      tDutType_2=3,
      tDutType_Total};
#endif

// ---- GATE 1 : tRotate.DutNum -- not on aHotPlateSubstrate.h's tRotateShim ----
#if 0 // TODO(W7): golden RotateKit/fRotate.h:32 TRotate::DutNum -- absent from this tree's tRotateShim (aHotPlateSubstrate.h:824-832)
#define W906RKO_DUTNUM                   (tRotate.DutNum)
#else
#define W906RKO_DUTNUM                   (tDutType_4)
#endif

// ---- GATE 2 : tRotate.bPassBinNoRotate ---------------------------------------
#if 0 // TODO(W7): golden RotateKit/fRotate.h TRotate::bPassBinNoRotate (jou 20231020) -- absent from tRotateShim
#define W906RKO_TROT_PASSBINNOROTATE     (tRotate.bPassBinNoRotate)
#else
#define W906RKO_TROT_PASSBINNOROTATE     (false)
#endif

// ---- GATE 3 : tRotate.RotateKit_PitchX (golden `double`) ---------------------
#if 0 // TODO(W7): golden RotateKit/fRotate.h TRotate::RotateKit_PitchX -- absent from tRotateShim
#define W906RKO_ROTKIT_PITCHX            (tRotate.RotateKit_PitchX)
#else
#define W906RKO_ROTKIT_PITCHX            (0.0)
#endif

// ---- GATE 4 : tRotate.bUseDifferentAngle -------------------------------------
#if 0 // TODO(W7): golden RotateKit/fRotate.h TRotate::bUseDifferentAngle -- absent from tRotateShim
#define W906RKO_TROT_USEDIFFERENTANGLE   (tRotate.bUseDifferentAngle)
#else
#define W906RKO_TROT_USEDIFFERENTANGLE   (false)
#endif

// ---- GATE 5 : tRotate.iRotateOffset[] ----------------------------------------
#if 0 // TODO(W7): golden RotateKit/fRotate.h:57 TRotate::iRotateOffset[2] -- absent from tRotateShim
#define W906RKO_TROT_IROTATEOFFSET(i)    (tRotate.iRotateOffset[i])
#else
#define W906RKO_TROT_IROTATEOFFSET(i)    (0)
#endif

// ---- GATE 6 : FrmRotate->bShowRotateBySite -----------------------------------
#if 0 // TODO(W7): golden RotateKit/fRotate.h:133 TFrmRotate::bShowRotateBySite -- absent from forms/fRotate.h's stand-in
#define W906RKO_SHOWROTATEBYSITE         (FrmRotate->bShowRotateBySite)
#else
#define W906RKO_SHOWROTATEBYSITE         (false)
#endif

// ---- GATE 7 : ArmCanSuck4IC() ------------------------------------------------
#if 0 // TODO(W7): golden csystem.cpp:723 ArmCanSuck4IC -- declared csystem.h:162, NO compiled body in this tree (same gate as RotateKit/aRotateKIT.cpp:238-242 and aRotateKIT_In.cpp:382-387)
#define W906RKO_ARMCANSUCK4IC(...)       (ArmCanSuck4IC(__VA_ARGS__))
#else
#define W906RKO_ARMCANSUCK4IC(...)       (true)
#endif

//AI(W906-RKOUT) 20260807: A-1 (banner C) -- NOT A GATE.  Exact reproduction of
//  golden MyKitSuck.h:122 `void SetNeedDestroy(bool Value) {bNeedDestroy=Value;}`.
//  The port's TMySucker mirror (aHotPlateSubstrate.h:147-151) landed the public
//  storage + the getter but not the setter, and aHotPlateSubstrate.h is not this
//  wave's file.  The macro makes all 9 golden call sites below byte-identical to
//  golden while writing the very same public member golden's setter writes.
//  Defined AFTER every #include on purpose (no included header spells this token
//  -- verified by a token scan of every .h outside build*/).  Retire by adding the
//  one-line setter to aHotPlateSubstrate.h and deleting these three lines (the
//  identical macro at aRotateKIT_In.cpp:404 retires with it).
#define SetNeedDestroy(v)                bNeedDestroy = (v)

// -----------------------------------------------------------------------------
//  A-2 (banner C): file-local VERBATIM copy of golden aoutarm.cpp:95-116.
//  Precedent: asendic_Auto.cpp:371-390 (`W7L1A_CheckOutArmZ`).  `static`, so it
//  cannot collide when golden aoutarm.cpp is translated.  The #define keeps the
//  two golden call sites (golden :49 and :870) textually identical to golden.
// -----------------------------------------------------------------------------
static bool W906RKO_CheckOutArmZ(bool bMessage)
{
    int iMotZ;
    for(int i=0; i<OutArmSuck.iPickRow; i++)
    {
        for(int j=0; j<OutArmSuck.iPickCol; j++)
        {
            iMotZ=(InOutArmPickerUseMotor==eptUseMotCyn)?MOutArmZA:OutArmSuck.Suck[i][j].iMotNo;
            if(MOT[iMotZ].ReadPos()!=Prod.ZOutArmSafe[i][j])
            {
                if(bMessage)
                {
                    ShowErrorMessage("WAR0257", 0, MOutArmX, false, MOT[iMotZ].Alias);      //出料手臂的吸嘴 %s 沒有在安全位置
                }
                return true;
            }
        }
    }
    return false;
}
#define CheckOutArmZ                     W906RKO_CheckOutArmZ
//---------------------------------------------------------------------------

int iOutArmRotateKit        =1;
//AI(W906-RKOUT) 20260807: D-1 (see banner).  golden :29 defines iOutRotateFinish
//  here, but aoutarm9045.cpp:185 already carries the identical definition
//  (`int iOutRotateFinish=0;`) for the out-arm engine.  Two definitions = link
//  error, so golden's line is kept VERBATIM but gated; this TU declares it extern
//  above.  Same variable, same initial value -> zero behaviour delta.
#if 0 // TODO(integrate): golden :29 -- ALREADY DEFINED at aoutarm9045.cpp:185 (duplicate symbol)
int iOutRotateFinish        =0;                                                 //kevin 20130524 確保IC有放到旋轉馬達轉向  1:初始化  2:旋轉完成  3:放置SHUTTLE 0:放完SHUTTLE    //Steven 20160809 : 改成全域變數
#endif
int iShakeOutArmRotateKit   =1;                                                 //JerryYang 20160803
int iOutArmShakeCnt         =0;                                                 //JerryYang 20160803 旋轉次數計數
int iOutRotateWhichKit      =0;                                                 //kevin 20130629 使用4個rotate 0:表示放第一次 Aa Ab Ae Ae  1:表示放第二次 Ac Ad Ag Ah
int OutAngle90              =0;                                                 //90度等於多少pulse
int OutAngle45              =0;                                                 //45度等於多少pulse   //kevin 20130710 旋轉45 度 add
//---------------------------------------------------------------------------
void InitialOutArmRotateKIT()
{
    iOutArmRotateKit=1;
}
//---------------------------------------------------------------------------
void InitialShakeOutArmRotateKIT()                                              //JerryYang 20160803 初始化Rotate旋轉功能
{
    iShakeOutArmRotateKit=1;
    iOutArmShakeCnt=0;
}
//---------------------------------------------------------------------------
bool MoveOutArmXY_ToRotateKIT(int iKit)
{
    if(CheckOutArmZ(false))                                                     //AI(W906-RKOUT) 20260807: A-2 -- file-local verbatim copy of golden aoutarm.cpp:95-116
    {
        bNeedArmZHome=true;
        InitDoArmZHome();
        return false;
    }

    //AI(W906-RKOUT) 20260807: GATE 11 (banner A) -- `fNote->bMyServoOffOutArm`   //AI(W906-FLOW-5) 20260929: RETIRED -- see :466
    //  (golden note.h:417) is deliberately NOT on forms/fNote.h's TfNote stand-in;
    //  forms/fNote.h:87-94 states so and names ported aoutarm9045.cpp:388/:1460 as
    //  the other two sites, both already `#if 0 // TODO(W7)`.  Same treatment here,
    //  same shape: the `if(IniConfig.bAlarmNeedServoOff)` guard stays ACTIVE, only
    //  the inner servo-off early-return is gated, so the ACTIVE path is golden's
    //  "no alarm-triggered ServoOff pending" arm -- the normal-running one.
    if(IniConfig.bAlarmNeedServoOff)                                            //Steven 20111226 : In Arm 咖啡杯解除
    {
//#if 0 // TODO(W7-form): golden :58 fNote->bMyServoOffOutArm -- not landed on forms/fNote.h's TfNote (see forms/fNote.h:87-94); ported aoutarm9045.cpp:389 gates the same member   //AI(W906-FLOW-5) 20260929: gate RETIRED -- golden RotateKit/aRotateKIT_Out.cpp:56-62 (MoveOutArmXY_ToRotateKIT) restored; the stated reason is false since AI(W906-HOME-C2) 20260920: TfNote::bMyServoOffOutArm exists (forms/fNote.h:305, ctor false forms/fNote.cpp:42) on the one live fNote (forms/fNote.cpp:48) and is already read live at aoutarm9045.cpp:762 (DoOutArm_9045, FLOW-2); the only reason a gate may stay is a missing dependency, and there is none. Inert until a writer sets it (golden main.cpp:7182/:7215, note.cpp:2074 -- not translated yet); when one lands this refuses the move exactly as golden does
        if(fNote->bMyServoOffOutArm)
        {
            return false;
        }
//#endif   //AI(W906-FLOW-5) 20260929: end of the retired GATE 11 (:466)
    }

    int iXPos=0, iYPos=0;
    int iXVariable[X_PITCH_COUNT];
    int iYVariable  =0;
    int iMovePitchX =iRotateKIT_Pitch_X_A*3/2;;
    int iMovePitchY =6000;
    bool bZFlag[MAX_ARM_Row][MAX_ARM_Col];
    int iZPos[MAX_ARM_Row][MAX_ARM_Col];
    int iOffsetPos=-1;
    ZeroMemory(iXVariable, sizeof(iXVariable));
    ZeroMemory(bZFlag, sizeof(bZFlag));
    ZeroMemory(iZPos, sizeof(iZPos));

    if(iKit==1)                                                                 // Z2 & Z4
    {
        iXPos=Prod.iOutArmRotateToUnloaderX+iRotateKIT_Start_X_A+iRotateKIT_Pitch_X_A/2;
        if(TestIF.iTestMode==QualSite1X4 || TestIF.iTestMode==_8Site1X4)        //ChungHung 20150528 add for 海思 _8Site1x4
            iYPos=Prod.iOutArmRotateToUnloaderY-iRotateKIT_Start_Y_A-iRotateKIT_Pitch_Y_A;
        else
            iYPos=Prod.iOutArmRotateToUnloaderY-iRotateKIT_Start_Y_A;
    }
    else // iKit = 0                                                            // Z1 & Z3(F)
    {
        iXPos   =Prod.iOutArmRotateToUnloaderX+iRotateKIT_Start_X_A+iRotateKIT_Pitch_X_A;
        iYPos   =Prod.iOutArmRotateToUnloaderY-iRotateKIT_Start_Y_A;
    }

    if(IniConfig.bE34InOutArmPitchZOffsetSameOne==true)
    {
        if(iWhichAuto>=0 && iWhichAuto<iAutoCnt)
        {
            iOffsetPos=OutOfsAuto1;
        }
    }
    else
    {
        iOffsetPos=OutOfsAuto1+iRotate_Out_Tray6;
    }

    iYVariable=GetOutArmPitchY_9045(iMovePitchY, iOffsetPos);
    for(int i=0; i<X_PITCH_COUNT; i++)
        iXVariable[i]=GetOutArmPitchX_9045(iMovePitchX, i, iOffsetPos);

    if(OutArmContinuousMove_9045(iXPos, iYPos, iXVariable, iYVariable, bZFlag, iZPos, false))
    {
        return true;
    }
    return false;
}
//---------------------------------------------------------------------------
bool MoveOutArmZ_ToRotateKIT_Place(int iKit)
{
    bool flag[2][4]={{true, true, true, true}, {true, true, true, true}};
    for(int i=0; i<MAX_ARM_Row; i++)
    {
        for(int j=iKit; j<MAX_ARM_Col; j+=2)                                    //(0, 2) & (1, 3)
        {
            if(OutArmSuck.Item[i][j]!=NULL_IC && OutArmSuck.Item[i][j]!=HAS_NULL_IC)
            {
                OutArmSuck.Suck[i][j].SetNeedDestroy(true);                        //AI(W906-RKOUT) 20260807: A-1 (banner C) -- SetNeedDestroy is the TU-local macro for golden MyKitSuck.h:122
                flag[i][j]=MOT[OutArmZIndex[i][j]].MotorMove(Prod.iOutArmRotateToUnloader_PlaceZ[i][j]);
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
bool MoveOutArmZ_ToRotateKIT_Pick(int iKit)
{
    bool flag[2][4]={{true, true, true, true}, {true, true, true, true}};
    int  j2;

    if(TestIF.iTestMode==QualSite1X4 || TestIF.iTestMode==_8Site1X4)            //ChungHung 20150528 add for 海思 _8Site1x4
    {
        int i=iKit;
        for(int j=0; j<MAX_ARM_Col/2; j++)                                      //(0, 2) & (1, 3)
        {
            j2=j*2+iKit;                                                        //Kit(0, 1) -> Suck(0, 2) or Suck(1, 3)
            if(MOT[MOutRotateKit].Tray.Data[j][i]!=NULL_IC && MOT[MOutRotateKit].Tray.Data[j][i]!=HAS_NULL_IC)
            {
                OutArmSuck.Suck[0][j2].SetNeedSuck(true);
                flag[0][j2]=MOT[OutArmZIndex[0][j2]].MotorMove(Prod.iOutArmRotateToUnloader_PickZ[0][j2]);
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
            {                                                                   //Kit(0, 1) -> Suck(0, 2) or Suck(1, 3)
                if(MOT[MOutRotateKit].Tray.Data[j][i]!=NULL_IC && MOT[MOutRotateKit].Tray.Data[j][i]!=HAS_NULL_IC)
                {
                    OutArmSuck.Suck[i][j].SetNeedSuck(true);
                    //AI(W906-RKOUT) 20260807: E-1 GOLDEN BUG preserved VERBATIM (golden :174) --
                    //  the PICK path moves Z to iOutArmRotateToUnloader_PlaceZ, not _PickZ, while
                    //  the 1X4 arm 17 lines above correctly uses _PickZ.  Not fixed.
                    flag[i][j]=MOT[OutArmZIndex[i][j]].MotorMove(Prod.iOutArmRotateToUnloader_PlaceZ[i][j]);
                }
                else
                {
                    flag[i][j]=true;
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
bool DoOutArmRotateKIT_Cylinder()
{
    int &Task=iOutArmRotateKit;
    int j2;

    static int iRetryCT=0;
    static int iKit=0;

    int ret;
    bool bHasError=false, flag=true;
    bool bHasDuplicateErr=false;
    static bool bSuckDuplicateErr[2][4]={{false, false, false, false}, {false, false, false, false}};  //Steven 20091218 : Avoid duplicate message
    AnsiString ErrPart="";

    switch(Task)
    {
        case 1:
            //RotateKIT_FormSet(bool bInput,int iStartX, int iStartY, int iPitchX, int iPitchY)
            RotateKIT_FormSet(        false,       2800,        7000,        8000,        6000);
            iKit=0;
            Task=100;
            // NOTE: faithful fall-through to case 100 (no break in golden :215)
        case 100:
            if(TrayForm.iRotateKIT_OutputType==1)            // +90
            {
                Cylinder[C_OutputRotateKIT].Off();
            }
            else if(TrayForm.iRotateKIT_OutputType==2)       // -90
            {
                Cylinder[C_OutputRotateKIT].On();
            }
            else // if(TrayForm.iRotateKIT_OutputType==0)
            {
                 return true;
            }
            MOT[MOutRotateKit].SetTray(NULL_IC, __FUNC__);
            ZeroMemory(bSuckDuplicateErr, sizeof(bSuckDuplicateErr));
            Task=1000;
            // NOTE: faithful fall-through to case 1000 (no break in golden :231)
        case 1000:
            if((OutArmSuck.Item[0][0+iKit]!=NULL_IC && OutArmSuck.Item[0][0+iKit]!=HAS_NULL_IC) ||
               (OutArmSuck.Item[0][2+iKit]!=NULL_IC && OutArmSuck.Item[0][2+iKit]!=HAS_NULL_IC) ||
               (OutArmSuck.Item[1][0+iKit]!=NULL_IC && OutArmSuck.Item[1][0+iKit]!=HAS_NULL_IC) ||
               (OutArmSuck.Item[1][2+iKit]!=NULL_IC && OutArmSuck.Item[1][2+iKit]!=HAS_NULL_IC))
            {
                Task=1100;
            }
            else
            {
                Task=2000;
                break;
            }
            // NOTE: faithful fall-through to case 1100 (golden :244 -- only the else breaks)
        case 1100:
            if(MoveOutArmXY_ToRotateKIT(iKit)==true)
            {
                CheckOutArmSuckICFallDown();
                if((TrayForm.iRotateKIT_OutputType==1 && Cylinder[C_OutputRotateKIT].Pop() ==true) ||  // +90
                   (TrayForm.iRotateKIT_OutputType==2 && Cylinder[C_OutputRotateKIT].Push()==true))    // -90
                {
                    for(int i=0; i<MAX_ARM_Row; i++)
                    {
                        for(int j=0; j<MAX_ARM_Col; j++)
                        {
                            OutArmSuck.Suck[i][j].SetNeedDestroy(false);           //AI(W906-RKOUT) 20260807: A-1 (banner C) -- SetNeedDestroy is the TU-local macro (golden MyKitSuck.h:122)
                        }
                    }
                    bOut_XYMoveFinishOnRotationKit=true;                        //Sam 20240726 : 修正移動完 XY 準備放 IC 到 RotationKit 時 Home
                    Task=1200;
                }
                else
                {
                    break;
                }
            }
            else
            {
                CheckOutArmSuckICFallDown();
                break;
            }
            // NOTE: faithful fall-through to case 1200 (golden :271 -- both else arms break)
        case 1200:
            //AI(W906-RKOUT) 20260807: E-3 GOLDEN BUG preserved VERBATIM (golden :273-274) --
            //  `Task=1200;` assigns the value Task already holds and does NOT block; control
            //  falls straight into the MoveOutArmZ call below.  Not fixed.
            if(bOut_XYMoveFinishOnRotationKit==false)                           //Sam 20240726 : 修正移動完 XY 準備放 IC 到 RotationKit 時 Home
                Task=1200;

            if(MoveOutArmZ_ToRotateKIT_Place(iKit)==true)
            {
                if((AccessLevel>=1 || CosFunction.bOPCanPressStepAndTStart) &&  //JerryYang 20170417 (wei) OP權限也能按Step跟T.Start
                    Sen[SnRKManualStep].IsOn())
                {
                    bEnterOffset=false;
                    fMain->Pause("DoOutArmRotateKIT_Cylinder 1200");
                    Task=1500;
                    break;
                }
                Task=1300;
                OutArmSuck.ResetAll();                                          //Steven 20160323 : 避免未開啟真空
            }
            else
            {
                break;
            }
            // NOTE: faithful fall-through to case 1300 (golden :292)
        case 1300:
            flag=true;
            for(int i=0; i<MAX_ARM_Row; i++)
            {
                for(int j=0; j<MAX_ARM_Col/2; j++)
                {
                    j2=j*2+iKit;
                    //AI(W906-RKOUT) 20260807: E-5 -- golden :300 mixes && and || without parens;
                    //  kept exactly as written (C++ precedence gives golden's intent).
                    if(OutArmSuck.Item[i][j2] && OutArmSuck.Suck[i][j2].GetNeedDestroyStatus() || OutArmSuck.Item[i][j2]==HAS_NULL_IC)
                    {
                        if(OutArmSuck.Item[i][j2]==HAS_NULL_IC || OutArmSuck.Suck[i][j2].Destroy())
                        {
                            bSuckDuplicateErr[i][j2]=false;
                            //AI(W906-RKOUT) 20260807: E-6 GOLDEN BUG preserved VERBATIM (golden :305) --
                            //  the OUT-arm cylinder rig writes the IN-arm destroy-confirm grid
                            //  bInArmCheckDestroyACT; the motor rig (golden :1954) writes
                            //  bOutArmCheckDestroyACT.  Not fixed.
                            bInArmCheckDestroyACT[i][j2]=true;                  //jou 981130 確認device確實destroy完成
                            if(TestIF.iTestMode==QualSite1X4 || TestIF.iTestMode==_8Site1X4)  //ChungHung 20150528 add for 海思 _8Site1x4
                            {
                                MOT[MOutRotateKit].SetTraySingleData(j, i+iKit, OutArmSuck.Item[i][j2]);
                            }
                            else
                            {
                                MOT[MOutRotateKit].SetTraySingleData(j, i, OutArmSuck.Item[i][j2]);
                            }
                            OutArmSuck.SetItemData(i, j2, NULL_IC);
                        }
                        else if(OutArmSuck.Suck[i][j2].Error==false)
                        {
                            flag=false;
                        }

                        if(OutArmSuck.Suck[i][j2].Error)
                        {
                            bHasError=true;
                        }
                    }
                }
            }

            if(flag==false)
                break;

            for(int i=0; i<MAX_ARM_Row; i++)
                for(int j=0; j<MAX_ARM_Col; j++)
                    if(OutArmSuck.Item[i][j] && OutArmSuck.Suck[i][j].GetNeedDestroyStatus())
                        return false;

            if(bHasError)
            {
                Task=1600;
                break;
            }

            ZeroMemory(bSuckDuplicateErr, sizeof(bSuckDuplicateErr));
            Task=2000;
            break;
        case 2000:
            if(MoveOutArmToAutoSafe())
            {
                if(iKit==0 && MOT[MOutRotateKit].FullIC()==false)
                {
                    iKit++;
                    Task=1000;
                    break;
                }
                else
                {
                    if(OutArmSuck.Item[0][0]==NULL_IC && OutArmSuck.Item[0][2]==NULL_IC &&
                       OutArmSuck.Item[1][0]==NULL_IC && OutArmSuck.Item[1][2]==NULL_IC)
                        iKit=0;
                    Task=3000;
                }
            }
            else
            {
                break;
            }
            // NOTE: faithful fall-through to case 3000 (golden :366)
        case 3000:
            //AI(W906-RKOUT) 20260807: E-2 GOLDEN BUG preserved VERBATIM (golden :368-379) --
            //  the `==1` arm assigns Task=3100 and then the unconditional `Task=3100;`
            //  below assigns it again (dead store); the `==2` arm relies on that
            //  unconditional line.  Not fixed.
            if(TrayForm.iRotateKIT_OutputType==1)            // +90
            {
                Cylinder[C_OutputRotateKIT].On();
                Task=3100;
            }
            else if(TrayForm.iRotateKIT_OutputType==2)       // -90
            {
                Cylinder[C_OutputRotateKIT].Off();
            }

            Task=3100;
            break;
        case 3100:  //吸取IC
            if((MOT[MOutRotateKit].Tray.Data[0][0]!=NULL_IC && MOT[MOutRotateKit].Tray.Data[0][0]!=HAS_NULL_IC) ||
               (MOT[MOutRotateKit].Tray.Data[1][0]!=NULL_IC && MOT[MOutRotateKit].Tray.Data[1][0]!=HAS_NULL_IC) ||
               (MOT[MOutRotateKit].Tray.Data[0][1]!=NULL_IC && MOT[MOutRotateKit].Tray.Data[0][1]!=HAS_NULL_IC) ||
               (MOT[MOutRotateKit].Tray.Data[1][1]!=NULL_IC && MOT[MOutRotateKit].Tray.Data[1][1]!=HAS_NULL_IC))
            {
                Task=4000;
            }
            else
            {
                Task=6000;
                break;
            }
            // NOTE: faithful fall-through to case 4000 (golden :392 -- only the else breaks)
        case 4000:
            if(MoveOutArmXY_ToRotateKIT(iKit)==true)
            {
                CheckOutArmSuckICFallDown();                                    //Steven 20110516 : 修改成整合式Alarm
                if((TrayForm.iRotateKIT_OutputType==1 && Cylinder[C_OutputRotateKIT].Push()==true) ||         // +90
                   (TrayForm.iRotateKIT_OutputType==2 && Cylinder[C_OutputRotateKIT].Pop() ==true))           // -90
                {
                    for(int i=0; i<MAX_ARM_Row; i++)
                    {
                        for(int j=0; j<MAX_ARM_Col; j++)
                        {
                            OutArmSuck.Suck[i][j].SetNeedSuck(false);
                        }
                    }
                    Task=4100;
                }
                else
                {
                    break;
                }
            }
            else
            {
                CheckOutArmSuckICFallDown();                                    //Steven 20110516 : 修改成整合式Alarm
                break;
            }
            // NOTE: faithful fall-through to case 4100 (golden :418)
        case 4100:
            if(MoveOutArmZ_ToRotateKIT_Pick(iKit)==true)
            {
                if((AccessLevel>=1 || CosFunction.bOPCanPressStepAndTStart) &&  //JerryYang 20170417 (wei) OP權限也能按Step跟T.Start
                    Sen[SnRKManualStep].IsOn())
                {
                    bEnterOffset=false;
                    fMain->Pause("DoOutArmRotateKIT_Cylinder 4100");
                    Task=4500;
                    break;
                }

                OutArmSuck.ResetAll();                                          //Steven 20160323 : 避免未開啟真空
                iRetryCT=0;
                Task=4200;
            }
            else
            {
                break;
            }
            // NOTE: faithful fall-through to case 4200 (golden :438)
        case 4200:
            flag=true;
            if(TestIF.iTestMode==QualSite1X4 || TestIF.iTestMode==_8Site1X4)    //ChungHung 20150528 add for 海思 _8Site1x4
            {
                for(int j=0; j<MAX_ARM_Col/2; j++)
                {
                    if(MOT[MOutRotateKit].Tray.Data[j][iKit]!=NULL_IC)
                    {
                        j2=j*2+iKit;
                        if(MOT[MOutRotateKit].Tray.Data[j][iKit]==HAS_NULL_IC || OutArmSuck.Suck[0][j2].Suck())
                        {
                            bSuckDuplicateErr[0][j2]=false;
                            OutArmSuck.SetItemData(0, j2, MOT[MOutRotateKit].Tray.Data[j][iKit]);
                            MOT[MOutRotateKit].SetTraySingleData(j, iKit,NULL_IC);
                        }
                        else if(OutArmSuck.Suck[0][j2].Error==false)
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
                    for(int j=0; j<MAX_ARM_Col/2; j++)
                    {
                        if(MOT[MOutRotateKit].Tray.Data[j][i]!=NULL_IC)
                        {
                            j2=j*2+iKit;
                            if(MOT[MOutRotateKit].Tray.Data[j][i]==HAS_NULL_IC || OutArmSuck.Suck[i][j2].Suck())
                            {
                                bSuckDuplicateErr[i][j2]=false;
                                OutArmSuck.SetItemData(i, j2, MOT[MOutRotateKit].Tray.Data[j][i]);
                                MOT[MOutRotateKit].SetTraySingleData(j, i,NULL_IC);
                            }
                            else if(OutArmSuck.Suck[i][j2].Error==false)
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
                    if(OutArmSuck.Suck[i][j].Error)
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
            if(MoveOutArmToAutoSafe())
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
            // NOTE: faithful fall-through to case 6000 (golden :525)
        case 6000:
            if(TrayForm.iRotateKIT_OutputType==1)            // +90
            {
                Cylinder[C_OutputRotateKIT].Off();
            }
            else if(TrayForm.iRotateKIT_OutputType==2)       // -90
            {
                Cylinder[C_OutputRotateKIT].On();
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
            if(MoveOutArmToAutoSafe())
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
                    if(OutArmSuck.Suck[i][j].Error)
                    {
                        ErrPart+=OutArmSuck.Suck[i][j].sName;
                        bSuckDuplicateErr[i][j]=true;
                        OutArmSuck.Suck[i][j].Error=false;
                    }
                    else
                    {
                        bSuckDuplicateErr[i][j]=false;
                    }
                }
            }
            ShowErrorMessage("JAM0217", K_RETRY, MOutArmX, bHasDuplicateErr, ErrPart);
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
            if(MoveOutArmToAutoSafe())
            {
                Task=4000;
            }
            break;
        case 4600:
            if(MoveOutArmToAutoSafe())
            {
                if(iRetryCT>ArmSpeed[OutArm].iRetryCT)                          //Sam 20250206 : Rotate pick error retry for setting
                {
                    ErrPart=" ";
                    for(int i=0; i<MAX_ARM_Row; i++)
                    {
                        for(int j=0; j<MAX_ARM_Col; j++)
                        {
                            if(bSuckDuplicateErr[i][j])
                                bHasDuplicateErr=true;
                            if(OutArmSuck.Suck[i][j].Error)
                            {
                                ErrPart+=OutArmSuck.Suck[i][j].sName;
                                bSuckDuplicateErr[i][j]=true;
                                OutArmSuck.Suck[i][j].Error=false;
                            }
                            else
                            {
                                bSuckDuplicateErr[i][j]=false;
                            }
                        }
                    }
                    ret=ShowErrorMessage("JAM0210", K_RETRY|K_SKIP, MOutArmX, bHasDuplicateErr, ErrPart);
                }
                else
                    ret=K_RETRY;

                if(ret==K_SKIP)
                {
                    for(int i=0; i<MAX_ARM_Row; i++)
                    {
                        for(int j=0; j<MAX_ARM_Col/2; j++)
                        {
                            if(W906RKO_DUTNUM==tDutType_8)                             //GATE 1 (banner A): golden `if(tRotate.DutNum == tDutType_8)` -- kevin 20130517 add 8 site
                                j2=j;
                            else
                                j2=j*2+iKit;

                            if(bSuckDuplicateErr[i][j])                         //kevin 20130524
                            {
                                MOT[MOutRotateKit].Tray.PordRec[i][j]->AddErrorRecord("JAM0210");   //Steven 20161215 (jou) : Add Production Error Log  //Steven 20221005 : Production Log減少記憶體使用量
                                MOT[MOutRotateKit].SetTraySingleData(j, i, NULL_IC);                //kevin 20130524
                                bSuckDuplicateErr[i][j]=false;                                      //kevin 20130524
                                OutArmSuck.SetItemData(i, j2, HAS_NULL_IC);
                                if(CosFunction.bUseSCKART)                      //Steven 20161214 (wei) : For SCK ART
                                    fSCKART->AddOutputJamCnt(i, j, ret);        //RogerYang 20250923 : 整合ART OutArm JamCount
                            }
                        }
                    }

                    for(int i=0; i<MAX_ARM_Row; i++)
                    {
                        for(int j=0; j<MAX_ARM_Col; j++)
                        {
                            OutArmSuck.Suck[i][j].Error=false;
                            bSuckDuplicateErr[i][j]=false;
                        }
                    }
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
bool DoOutArmRotateKIT()
{
    bool bRet=false;

    if(iRotate_Type==eCynRotate)                                                //氣缸版
    {
        bRet=DoOutArmRotateKIT_Cylinder();
    }
    else
    {
        if(tRotate.ActiveRotate)
        {
            if(iRotate_Type==eInOutArm1Motor)                                   //add One sucker with rotate
                bRet=M_DoOutArmRotate_InOutArm();
            else
                bRet=M_DoOutArmRotateKIT_Motor();
        }
        else
        {
            bRet=true;                                                          //不使用旋轉機構
        }
    }
    return bRet;
}
//---------------------------------------------------------------------------
void CalcPosition_OutArm(int &iXpos, int &iYpos, int &iXPitch, int iYPitch, int iKit)
{
    int OneSuckPitch;
    iYpos=Prod.iOutArm_RotateY+(iYPitch*iRotato_Out_Row);

    if(USE_OUT_Y_IS_AUTO_PITCH==true && iRotato_Out_Row==0)                     //JerryYang 20251218 : IN/OUT ARM支援不同模組
    {
        iYpos=iYpos-iYPitch;
    }

    if(iRotate_Type==e1MotRotate1Dut)                                           //Frank 20180512 (Steven) : modify 1 Dut 1 Motor
    {
        if(OutArmSuck.iPickCol==1)                                              //Steven 20241012 : 1x1 mode直接使用最小X-Pitch
        {
            if(USE_PICKER_COUNT==ep16Picker)
            {
                iXPitch=iXpitchMinX7;
            }
            else
            {
                iXPitch=iXpitchMinX3;
            }
            OneSuckPitch=iXpitchMin;
        }
        else
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

        iXpos=Prod.iOutArm_RotateX-(OneSuckPitch*(OutArmSuck.Suck[iRotato_Out_Row][iKit].iMyCol-iOutArmXBase));
    }
    else if(iRotate_Type==e2MotRotate2Dut)                                      //wei 20190515 modify 2 Dut 2 Motor
    {
        if(OutArmSuck.iPickCol==1)                                              //Steven 20241012 : 1x1 mode直接使用最小X-Pitch
        {
            if(USE_PICKER_COUNT==ep16Picker)
            {
                iXPitch=iXpitchMinX7;
            }
            else
            {
                iXPitch=iXpitchMinX3;
            }
            OneSuckPitch=iXpitchMin;
            iXpos=Prod.iOutArm_RotateX-(OneSuckPitch*(OutArmSuck.Suck[iRotato_Out_Row][iKit].iMyCol-iOutArmXBase));
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
            iXpos=Prod.iOutArm_RotateX-(OneSuckPitch*(OutArmSuck.Suck[iRotato_Out_Row][iKit].iMyCol-iOutArmXBase))-iRotateKIT_Pitch_X_H;
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
            iXpos=Prod.iOutArm_RotateX-(OneSuckPitch*(OutArmSuck.Suck[iRotato_Out_Row][iKit].iMyCol-iOutArmXBase))-iRotateKIT_Pitch_X_H;
        }
    }
    else if(W906RKO_DUTNUM==tDutType_8)                                                //GATE 1 (banner A): golden `else if(tRotate.DutNum==tDutType_8)`
    {
        if(iInArmType==e9045_1x2_2_14 || iInArmType==e9045_2x2_4_14 || iInArmType==e9045_2x4_4_14)    //Steven 20201028 : for rotator
        {
            iXPitch=iXpitchMaxX3;
            if(TestIF.iTestMode==QualSite2X2 && DeviceForm.XDimension>=5000)    //wei 20151229  Xilinx 52.5x52.5 放一個Rotate  //Alick 20160105 (wei) modify IC>=50x50 放一個Rotate
            {
                iXpos   = Prod.iOutArm_RotateX + (iRotateKIT_Pitch_X_H) - ((iRotateKIT_Pitch_X_H*2)*iKit);
                iYpos   = Prod.iOutArm_RotateY + (iRotateKIT_Pitch_Y_H*iRotato_Out_Row) ;
            }
            else
            {
                iXpos   = Prod.iOutArm_RotateX + (iRotateKIT_Pitch_X_H) - ((iRotateKIT_Pitch_X_H)*iKit);
                iYpos   = Prod.iOutArm_RotateY - (iRotateKIT_Pitch_Y_H*iKit) + (iRotateKIT_Pitch_Y_H*iRotato_Out_Row) ;
            }
        }
        else
        {
            iXPitch =iXpitchMaxX3;                                              //Steven 20140424 : 避免Rotate Pitch錯誤     //Isaac 20171204 (Steven) : Xpitch40->50mm, 12000->iXpitchMaxX3
            iXpos   = Prod.iOutArm_RotateX;
            iYpos   = Prod.iOutArm_RotateY ;
        }
    }
    else// if(tRotate.DutNum == tDutType_4)
    {
        if(iRotate_Type==e2MotRotate2Dut)                                       //wei 20190515 modify 2 Dut 2 Motor
        {
            iXPitch =iXpitchMaxX3;
            iXpos   =Prod.iOutArm_RotateX - (iXpitchMax*iKit);
            iYpos   =Prod.iOutArm_RotateY + (6000*iRotato_Out_Row);
        }
        else if((TestIF.iTestMode==_8Site2X4 || TestIF.iTestMode==_16Site2X8 || TestIF.iTestMode==_16Site4X4) &&  //Sam 20190226 : 16Site4X4
                W906RKO_ARMCANSUCK4IC(0)==false)
        {
            if(W906RKO_ROTKIT_PITCHX==40)                                   //GATE 3 (banner A): golden `if(tRotate.RotateKit_PitchX==40)` //20140307 wei Pitch_X 40   需*3 ，  Pitch_X 80 需要  /2*3
            {
                iXPitch = iRotateKIT_Pitch_X_H *2;                              //Sam 20211224 : 修正為使用 1 4 吸嘴
            }
            else
            {
                iXPitch = iRotateKIT_Pitch_X_H;                                 //Sam 20211224 : 修正為使用 1 4 吸嘴
            }
            OneSuckPitch = iXPitch / 3;                                         //Sam 20211224 : 修正為使用 1 4 吸嘴
            iXpos   = Prod.iOutArm_RotateX + iRotateKIT_Start_X_H - OneSuckPitch;   //Sam 20211224 : 修正為使用 1 4 吸嘴
            iYpos   = Prod.iOutArm_RotateY - iRotateKIT_Start_Y_H;
        }
        else
        {
            /*if(bSingleDut)                                                      //Ifor 20180110 (Steven) : add 僅有一個Rotate sensor
            {
                iXPitch = iXpitchMaxX3;
                iXpos   = (Prod.iOutArm_RotateX)-4000;
                iYpos   = Prod.iOutArm_RotateY;
            }
            else   */
            {
                if(W906RKO_ARMCANSUCK4IC(0)==true)                                      //Steven 20140207 : 修正Rotate吸八顆放四顆
                {                                                               //kevin 20130629    //吸8顆 使用4個rotate
                    iXPitch = iRotateKIT_Pitch_X_H *3/2;                        //吸嘴1中心到吸嘴3中心   //20140318 wei Rotate PitchX=80    |40|40|40|
                    iXpos   = Prod.iOutArm_RotateX-(iRotateKIT_Pitch_X_H/2)*iKit;     //20140318 wei  PitchX=80 移動40
                    iYpos   = Prod.iOutArm_RotateY ;
                }
                else
                {
                    iXPitch = iRotateKIT_Pitch_X_H * 3/2;                       //吸嘴1中心到吸嘴3中心
                    iXpos   = Prod.iOutArm_RotateX + iRotateKIT_Start_X_H;
                    iYpos   = Prod.iOutArm_RotateY - iRotateKIT_Start_Y_H;
                }
            }
        }
    }
}
int OutArmRotate_PitchY;
//==============================================================================
int AutoCalculateOutArmRotatePitch_Y()
{
    if(USE_OUT_Y_IS_AUTO_PITCH==true)                                           //JerryYang 20251218 : IN/OUT ARM支援不同模組
    {
        if(OutArmSuck.iPickRow==1)
        {
            OutArmRotate_PitchY=TestIF.iARM_Y_PITCH;
        }
        else
        {
            OutArmRotate_PitchY=iRotateKIT_Pitch_Y_A;                           //RogerYang 20250916 : Fix 單位錯誤，需透過RotateKIT_FormSet轉換
        }
    }
    else
    {
        OutArmRotate_PitchY=TestIF.iARM_Y_PITCH;
    }
    return OutArmRotate_PitchY;
}
//---------------------------------------------------------------------------
bool M_MoveOutArmXY_ToRotateKIT(int iKit)
{
    if(CheckOutArmZ(false))                                                     //AI(W906-RKOUT) 20260807: A-2 -- file-local verbatim copy of golden aoutarm.cpp:95-116
    {
        bNeedArmZHome=true;
        InitDoArmZHome();
        return false;
    }

    //AI(W906-RKOUT) 20260807: GATE 11 (banner A) -- second of the two   //AI(W906-FLOW-5) 20260929: RETIRED -- see :1318
    //  fNote->bMyServoOffOutArm sites (golden :879).  Same reason and same shape as
    //  golden :58; see the note there and forms/fNote.h:87-94.
    if(IniConfig.bAlarmNeedServoOff)                                            //Steven 20111226 : In Arm 咖啡杯解除
    {
//#if 0 // TODO(W7-form): golden :879 fNote->bMyServoOffOutArm -- not landed on forms/fNote.h's TfNote (see forms/fNote.h:87-94); ported aoutarm9045.cpp:1461 gates the same member   //AI(W906-FLOW-5) 20260929: gate RETIRED -- golden RotateKit/aRotateKIT_Out.cpp:877-883 (M_MoveOutArmXY_ToRotateKIT) restored; same reason as :466
        if(fNote->bMyServoOffOutArm)
        {
            return false;
        }
//#endif   //AI(W906-FLOW-5) 20260929: end of the retired GATE 11 (:1318)
    }

    int iXPos=0, iYPos=0;
    int iXVariable[X_PITCH_COUNT];
    int iYVariable  =0;
    int iMovePitchX =iXpitchMaxX3;
    int iMovePitchY =AutoCalculateOutArmRotatePitch_Y();
    bool bZFlag[MAX_ARM_Row][MAX_ARM_Col];
    int iZPos[MAX_ARM_Row][MAX_ARM_Col];
    int iOffsetPos=-1;
    ZeroMemory(iXVariable, sizeof(iXVariable));
    ZeroMemory(bZFlag, sizeof(bZFlag));
    ZeroMemory(iZPos, sizeof(iZPos));

    if(IniConfig.bE34InOutArmPitchZOffsetSameOne==true)                         //jou 2010-12-20 Pitch & Z 縮減為一個
    {
        iOffsetPos=OutOfsAuto1;
    }
    else
    {
        iOffsetPos=OutOfsRotate_Out;
    }

    CalcPosition_OutArm(iXPos, iYPos, iMovePitchX, iMovePitchY, iKit);

    iYVariable=GetOutArmPitchY_9045(iMovePitchY, iOffsetPos);
    for(int i=0; i<X_PITCH_COUNT; i++)
        iXVariable[i]=GetOutArmPitchX_9045(iMovePitchX, i, iOffsetPos);

    if(OutArmContinuousMove_9045(iXPos, iYPos, iXVariable, iYVariable, bZFlag, iZPos, false))
    {
        return true;
    }
    return false;
}
//---------------------------------------------------------------------------
bool M_MoveOutArmZ_ToRotateKIT_Place(int iKit)
{
    bool flag[2][4]={{true, true, true, true}, {true, true, true, true}};
    bool bPass[2][4]={{false, false, false, false}, {false, false, false, false}};
    int  iSuckR, iSuckC;

    if(CosFunction.bPassBinNoRotate==true && W906RKO_TROT_PASSBINNOROTATE==true)    //GATE 2 (banner A): golden `&& tRotate.bPassBinNoRotate==true` //jou 20231020 : Pass bin no rotate
    {
        for(int i=0; i<MAX_ARM_Row; i++)
            for(int j=0; j<MAX_ARM_Col; j++)
                bPass[i][j]=OutArmSuck.bPass[i][j];
    }

    if(iRotate_Type==e1MotRotate1Dut)                                           //Frank 20180512 (Steven) : modify 1 Dut 1 Motor
    {
        //AI(W906-RKOUT) 20260807: E-9 -- golden :934 walks j to OutArmSuck.iMaxCol
        //  but indexes flag[2][4]; if iMaxCol>4 this is out of bounds.  Golden's
        //  own risk, kept verbatim.
        for(int j=iKit; j<OutArmSuck.iMaxCol; j+=4)
        {
            iSuckR=iRotato_Out_Row;
            iSuckC=j;

            if(OutArmSuck.Item[iSuckR][iSuckC]!=NULL_IC &&
               OutArmSuck.Item[iSuckR][iSuckC]!=HAS_NULL_IC &&
               bPass[iSuckR][iSuckC]==false)                                    //jou 20231020 : Pass bin no rotate
            {
                OutArmSuck.Suck[iSuckR][iSuckC].SetNeedDestroy(true);              //AI(W906-RKOUT) 20260807: A-1 (banner C) -- SetNeedDestroy is the TU-local macro (golden MyKitSuck.h:122)
                flag[iSuckR][iSuckC]=MOT[OutArmSuck.Suck[iSuckR][iSuckC].iMotNo].MotorMove2SpeedForPicker(Prod.iOutArm_RotatePlace[iSuckR][iSuckC], &ArmSpeed[OutArm]);  //Sam 20250207 : Rotate function 吸放也要二段速
            }
            else
            {
                flag[iSuckR][iSuckC]=true;
            }
        }
    }
    else if(iRotate_Type==e2MotRotate2Dut)                                      //wei 20190515 modify 2 Dut 2 Motor
    {
        if(OutArmSuck.iPickCol==1)
        {
            for(int j=iKit; j<OutArmSuck.iMaxCol; j+=4)
            {
                iSuckR=iRotato_Out_Row;
                iSuckC=j;

                if(OutArmSuck.Item[iSuckR][iSuckC]!=NULL_IC &&
                   OutArmSuck.Item[iSuckR][iSuckC]!=HAS_NULL_IC &&
                   bPass[iSuckR][iSuckC]==false)                                //jou 20231020 : Pass bin no rotate
                {
                    OutArmSuck.Suck[iSuckR][iSuckC].SetNeedDestroy(true);          //AI(W906-RKOUT) 20260807: A-1
                    flag[iSuckR][iSuckC]=MOT[OutArmSuck.Suck[iSuckR][iSuckC].iMotNo].MotorMove2SpeedForPicker(Prod.iOutArm_RotatePlace[iSuckR][iSuckC], &ArmSpeed[OutArm]);  //Sam 20250207 : Rotate function 吸放也要二段速
                }
                else
                {
                    flag[iSuckR][iSuckC]=true;
                }
            }
        }
        else if(OutArmSuck.iPickCol==2)                                         //Steven 20241111 : for 1x4 close 2 site
        {
            for(int j=0; j<2; j++)
            {
                iSuckR=iRotato_Out_Row;
                iSuckC=j;

                if(OutArmSuck.Item[iSuckR][iSuckC]!=NULL_IC &&
                   OutArmSuck.Item[iSuckR][iSuckC]!=HAS_NULL_IC &&
                   bPass[iSuckR][iSuckC]==false)                                //jou 20231020 : Pass bin no rotate
                {
                    OutArmSuck.Suck[iSuckR][iSuckC].SetNeedDestroy(true);          //AI(W906-RKOUT) 20260807: A-1
                    flag[iSuckR][iSuckC]=MOT[OutArmSuck.Suck[iSuckR][iSuckC].iMotNo].MotorMove2SpeedForPicker(Prod.iOutArm_RotatePlace[iSuckR][iSuckC], &ArmSpeed[OutArm]);  //Sam 20250207 : Rotate function 吸放也要二段速
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
                iSuckR=iRotato_Out_Row;
                iSuckC=j*2+iKit;

                if(OutArmSuck.Item[iSuckR][iSuckC]!=NULL_IC &&
                   OutArmSuck.Item[iSuckR][iSuckC]!=HAS_NULL_IC &&
                   bPass[iSuckR][iSuckC]==false)                                //jou 20231020 : Pass bin no rotate
                {
                    OutArmSuck.Suck[iSuckR][iSuckC].SetNeedDestroy(true);          //AI(W906-RKOUT) 20260807: A-1
                    flag[iSuckR][iSuckC]=MOT[OutArmSuck.Suck[iSuckR][iSuckC].iMotNo].MotorMove2SpeedForPicker(Prod.iOutArm_RotatePlace[iSuckR][iSuckC], &ArmSpeed[OutArm]);  //Sam 20250207 : Rotate function 吸放也要二段速
                }
                else
                {
                    flag[iSuckR][iSuckC]=true;
                }
            }
        }
    }
    else if(W906RKO_DUTNUM==tDutType_8)                                                //GATE 1 (banner A): golden `else if(tRotate.DutNum==tDutType_8)` //kevin 20130517 add 8 Site
    {
        if(iInArmType==e9045_1x2_2_14 || iInArmType==e9045_2x2_4_14 || iInArmType==e9045_2x4_4_14)    //Steven 20201028 : for rotator
        {
            if(TestIF.iTestMode==QualSite2X2 && DeviceForm.XDimension>=5000)    //wei 20151229  Xilinx 52.5x52.5 放一個Rotate  //Alick 20160105 (wei) modify IC>=50x50 放一個Rotate
            {
                for(int j=iKit; j<MAX_ARM_Col; j+=4)
                {
                    int i=iRotato_Out_Row;
                    int j2=j*2;
                    if(OutArmSuck.Item[i][j2]!=NULL_IC && OutArmSuck.Item[i][j2]!=HAS_NULL_IC)
                    {
                        OutArmSuck.Suck[i][j2].SetNeedDestroy(true);               //AI(W906-RKOUT) 20260807: A-1
                        flag[i][j2]=MOT[OutArmZIndex[i][j2]].MotorMove2SpeedForPicker(Prod.iOutArm_RotatePlace[i][j2], &ArmSpeed[OutArm]);
                    }
                    else
                    {
                        flag[i][j2]=true;
                    }
                }
            }
            else
            {
                for(int j=iKit; j<MAX_ARM_Col; j+=4)
                {
                    int i=iRotato_Out_Row;
                    int j2=j*2;
                    if(OutArmSuck.Item[i][j2]!=NULL_IC && OutArmSuck.Item[i][j2]!=HAS_NULL_IC)
                    {
                        OutArmSuck.Suck[i][j2].SetNeedDestroy(true);               //AI(W906-RKOUT) 20260807: A-1
                        flag[i][j2]=MOT[OutArmZIndex[i][j2]].MotorMove2SpeedForPicker(Prod.iOutArm_RotatePlace[i][j2], &ArmSpeed[OutArm]);
                    }
                    else
                    {
                        flag[i][j2]=true;
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
                    if(OutArmSuck.Item[i][j]!=NULL_IC && OutArmSuck.Item[i][j]!=HAS_NULL_IC)
                    {
                        OutArmSuck.Suck[i][j].SetNeedDestroy(true);                //AI(W906-RKOUT) 20260807: A-1
                        flag[i][j]=MOT[OutArmSuck.Suck[i][j].iMotNo].MotorMove2SpeedForPicker(Prod.iOutArm_RotatePlace[i][j], &ArmSpeed[OutArm]);
                    }
                    else
                    {
                        flag[i][j]=true;
                    }
                }
            }
        }
    }
    else
    {                                                                           //Ifor 20180110 (Steven) : add 僅有一個Rotate sensor
        for(int i=0; i<MAX_ARM_Row; i++)
        {
            for(int j=iKit; j<MAX_ARM_Col; j+=i2x2Suck_Out)                 //(0, 2) & (1, 3)
            {
                if(OutArmSuck.Item[i][j]!=NULL_IC && OutArmSuck.Item[i][j]!=HAS_NULL_IC)
                {
                    OutArmSuck.Suck[i][j].SetNeedDestroy(true);                    //AI(W906-RKOUT) 20260807: A-1
                    flag[i][j]=MOT[OutArmSuck.Suck[i][j].iMotNo].MotorMove2SpeedForPicker(Prod.iOutArm_RotatePlace[i][j], &ArmSpeed[OutArm]);   //Sam 20250428 : 修正 1X2 OutRotate 旋轉異常
                }
                else
                {
                    flag[i][j]=true;
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
bool M_MoveOutArmZ_ToRotateKIT_Pick(int iKit, int PickAngle)
{
    bool flag[2][4]={{true, true, true, true}, {true, true, true, true}};
    int  iSuckR, iSuckC, iTrayY, iTrayX, iRealR, iRealC;
    int iMAX_ARM_Col=0;
    if(W906RKO_DUTNUM==tDutType_8)                                                     //GATE 1 (banner A): golden `if(tRotate.DutNum == tDutType_8)` //kevin 20130517 add 8 Site
        iMAX_ARM_Col=MAX_ARM_Col;
    else
        iMAX_ARM_Col=MAX_ARM_Col/2;

    if(iRotate_Type==e1MotRotate1Dut)                                           //Frank 20180512 (Steven) : modify 1 Dut 1 Motor
    {
        for(int j=iKit; j<OutArmSuck.iMaxCol; j+=4)                             //AI(W906-RKOUT) 20260807: E-9 (flag[2][4] vs iMaxCol) -- golden's own risk, kept
        {
            iTrayY=0;
            iTrayX=0;
            iSuckR=iRotato_Out_Row;
            iSuckC=j;
            iRealR=OutArmSuck.Suck[iSuckR][iSuckC].iMyRow;
            iRealC=OutArmSuck.Suck[iSuckR][iSuckC].iMyCol;
            if(MOT[MOutRotateKit].Tray.Data[iTrayX][iTrayY]!=NULL_IC &&
               MOT[MOutRotateKit].Tray.Data[iTrayX][iTrayY]!=HAS_NULL_IC)
            {
                if(CheckOutRotateAnglePostion(iRealR, iRealC, iTrayX, iTrayY)==true)
                {
                    OutArmSuck.Suck[iSuckR][iSuckC].SetNeedSuck(true);
                    flag[iSuckR][iSuckC]=MOT[OutArmSuck.Suck[iSuckR][iSuckC].iMotNo].MotorMove2SpeedForPicker(Prod.iOutArm_RotatePick[iSuckR][iSuckC], &ArmSpeed[OutArm]);  //Ifor 20250701 fix Rotate Suck Error
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
        if(OutArmSuck.iPickCol==1)
        {
            for(int j=iKit; j<OutArmSuck.iMaxCol; j+=4)
            {
                iTrayY=0;
                iTrayX=0;
                iSuckR=iRotato_Out_Row;
                iSuckC=j;
                iRealR=OutArmSuck.Suck[iSuckR][iSuckC].iMyRow;
                iRealC=OutArmSuck.Suck[iSuckR][iSuckC].iMyCol;
                if(MOT[MOutRotateKit].Tray.Data[iTrayX][iTrayY]!=NULL_IC &&
                   MOT[MOutRotateKit].Tray.Data[iTrayX][iTrayY]!=HAS_NULL_IC)
                {
                    if(CheckOutRotateAnglePostion(iRealR, iRealC, iTrayX, iTrayY)==true)
                    {
                        OutArmSuck.Suck[iSuckR][iSuckC].SetNeedSuck(true);
                        flag[iSuckR][iSuckC]=MOT[OutArmSuck.Suck[iSuckR][iSuckC].iMotNo].MotorMove2SpeedForPicker(Prod.iOutArm_RotatePick[iSuckR][iSuckC], &ArmSpeed[OutArm]);  //Ifor 20250701 fix Rotate Suck Error
                    }
                }
                else
                {
                    flag[iSuckR][iSuckC]=true;
                }
            }
        }
        else if(OutArmSuck.iPickCol==2)
        {
            for(int j=0; j<2; j++)
            {
                iTrayY=0;
                iTrayX=j;
                iSuckR=iRotato_Out_Row;
                iSuckC=j;
                iRealR=OutArmSuck.Suck[iSuckR][iSuckC].iMyRow;
                iRealC=OutArmSuck.Suck[iSuckR][iSuckC].iMyCol;
                if(MOT[MOutRotateKit].Tray.Data[iTrayX][iTrayY]!=NULL_IC &&
                   MOT[MOutRotateKit].Tray.Data[iTrayX][iTrayY]!=HAS_NULL_IC)
                {
                    if(CheckOutRotateAnglePostion(iRealR, iRealC, iTrayX, iTrayY)==true)
                    {
                        OutArmSuck.Suck[iSuckR][iSuckC].SetNeedSuck(true);
                        flag[iSuckR][iSuckC]=MOT[OutArmSuck.Suck[iSuckR][iSuckC].iMotNo].MotorMove2SpeedForPicker(Prod.iOutArm_RotatePick[iSuckR][iSuckC], &ArmSpeed[OutArm]);  //Ifor 20250701 fix Rotate Suck Error
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
                iSuckR=iRotato_Out_Row;
                iSuckC=j*2+iKit;
                iRealR=OutArmSuck.Suck[iSuckR][iSuckC].iMyRow;
                iRealC=OutArmSuck.Suck[iSuckR][iSuckC].iMyCol;
                if(MOT[MOutRotateKit].Tray.Data[iTrayX][iTrayY]!=NULL_IC &&
                   MOT[MOutRotateKit].Tray.Data[iTrayX][iTrayY]!=HAS_NULL_IC)
                {
                    if(CheckOutRotateAnglePostion(iRealR, iRealC, iTrayX, iTrayY)==true)
                    {
                        OutArmSuck.Suck[iSuckR][iSuckC].SetNeedSuck(true);
                        flag[iSuckR][iSuckC]=MOT[OutArmSuck.Suck[iSuckR][iSuckC].iMotNo].MotorMove2SpeedForPicker(Prod.iOutArm_RotatePick[iSuckR][iSuckC], &ArmSpeed[OutArm]);  //Ifor 20250701 fix Rotate Suck Error
                    }
                }
                else
                {
                    flag[iSuckR][iSuckC]=true;
                }
            }
        }
    }
    else if((TestIF.iTestMode==QualSite1X4 && TestIF.iUseSuckMode==4) || TestIF.iTestMode==_8Site1X4)  //ChungHung 20150528 add for 海思 _8Site1x4
    {
        for(int j=0; j<iMAX_ARM_Col; j++)                                       //(0, 2) & (1, 3)
        {                                                                       //Kit(0, 1) -> Suck(0, 2) or Suck(1, 3)
            iSuckR=0;
            if(W906RKO_DUTNUM==tDutType_8)                                             //GATE 1 (banner A): golden `if(tRotate.DutNum==tDutType_8)` //jou 2015-10-14 fix 1x4 rotate dut8 hang up
                iSuckC=j;
            else
                iSuckC=j*i2x2Suck_Out+iKit;

            iTrayX=j;
            iTrayY=0;

            if(MOT[MOutRotateKit].Tray.Data[iTrayX][iTrayY]!=NULL_IC && MOT[MOutRotateKit].Tray.Data[iTrayX][iTrayY]!=HAS_NULL_IC)
            {
                if(CheckRotateOutAnglePostion(iTrayY, iTrayX, PickAngle, 1)==true)    //Ifor 20170412 (Steven) Modify 角度已算過不需在減掉原本角度
                {
                    OutArmSuck.Suck[iSuckR][iSuckC].SetNeedSuck(true);
                    flag[iSuckR][iSuckC]=MOT[OutArmZIndex[iSuckR][iSuckC]].MotorMove2SpeedForPicker(Prod.iOutArm_RotatePick[iSuckR][iSuckC], &ArmSpeed[OutArm]);
                }
            }
            else
            {
                flag[iSuckR][iSuckC]=true;
            }
        }
    }
    else if(iInArmType==e9045_1x2_2_14 || iInArmType==e9045_2x2_4_14) //|| iInArmType==e9045_2x4_4_14)   //Sam 20211224 : 修正為使用 1 4 吸嘴  //Steven 20201028 : for rotator
    {
        if(TestIF.iTestMode==QualSite2X2 && DeviceForm.XDimension>=5000)        //wei 20151229  Xilinx 52.5x52.5 放一個Rotate  //Alick 20160105 (wei) modify IC>=50x50 放一個Rotate
        {
            for(int j=iKit; j<iMAX_ARM_Col; j+=4)                               //(0, 2) & (1, 3)
            {
                iSuckR=iRotato_Out_Row;
                iSuckC=j*2;
                iTrayX=1;
                iTrayY=0;
                if(MOT[MOutRotateKit].Tray.Data[iTrayX][iTrayY]!=NULL_IC && MOT[MOutRotateKit].Tray.Data[iTrayX][iTrayY]!=HAS_NULL_IC)
                {
                    if(CheckRotateOutAnglePostion(iTrayY, iTrayX, PickAngle, 1)==true)    //Ifor 20170412 (Steven) Modify 角度已算過不需在減掉原本角度
                    {
                        OutArmSuck.Suck[iSuckR][iSuckC].SetNeedSuck(true);
                        flag[iSuckR][iSuckC]=MOT[OutArmZIndex[iSuckR][iSuckC]].MotorMove2SpeedForPicker(Prod.iOutArm_RotatePick[iSuckR][iSuckC], &ArmSpeed[OutArm]);
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
            if(iRotate_Type==e2MotRotate2Dut && TestIF.iTestMode==DualSite)
            {
                for(int j=iKit; j<iMAX_ARM_Col; j++)                            //(0, 2) & (1, 3)
                {
                    iSuckR=iRotato_Out_Row;
                    iSuckC=j*2;
                    iTrayX=j;
                    iTrayY=0;
                    if(MOT[MOutRotateKit].Tray.Data[iTrayX][iTrayY]!=NULL_IC && MOT[MOutRotateKit].Tray.Data[iTrayX][iTrayY]!=HAS_NULL_IC)
                    {
                        if(CheckRotateOutAnglePostion(iTrayY, iTrayX, PickAngle, 1)==true)   //Ifor 20170412 (Steven) Modify 角度已算過不需在減掉原本角度
                        {
                            OutArmSuck.Suck[iSuckR][iSuckC].SetNeedSuck(true);
                            flag[iSuckR][iSuckC]=MOT[OutArmZIndex[iSuckR][iSuckC]].MotorMove2SpeedForPicker(Prod.iOutArm_RotatePick[iSuckR][iSuckC], &ArmSpeed[OutArm]);
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
                for(int j=iKit; j<iMAX_ARM_Col; j+=4)                           //(0, 2) & (1, 3)
                {
                    iSuckR=iRotato_Out_Row;
                    iSuckC=j*2;
                    iTrayX=j+1;
                    iTrayY=iKit;
                    if(MOT[MOutRotateKit].Tray.Data[iTrayX][iTrayY]!=NULL_IC && MOT[MOutRotateKit].Tray.Data[iTrayX][iTrayY]!=HAS_NULL_IC)
                    {
                        if(CheckRotateOutAnglePostion(iTrayY, iTrayX, PickAngle, 1)==true)   //Ifor 20170412 (Steven) Modify 角度已算過不需在減掉原本角度
                        {
                            OutArmSuck.Suck[iSuckR][iSuckC].SetNeedSuck(true);
                            flag[iSuckR][iSuckC]=MOT[OutArmZIndex[iSuckR][iSuckC]].MotorMove2SpeedForPicker(Prod.iOutArm_RotatePick[iSuckR][iSuckC], &ArmSpeed[OutArm]);
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
        for(int i=0; i<MAX_ARM_Row; i++)
        {
            for(int j=0; j<iMAX_ARM_Col; j++)                                   //(0, 2) & (1, 3)
            {
                if(iRotate_Type==e2MotRotate2Dut && TestIF.iTestMode==SingleSite)
                {
                    iSuckR=0;
                    //iSuckC=3;
                    iSuckC=0;                                                   //Sam 20250618 : 修正 Single Rotate 錯誤
                    iTrayX=1;
                    iTrayY=0;
                }
               /* else if(bSingleDut&& TestIF.iTestMode==SingleSite)              //Ifor 20180110 (Steven) : add 僅有一個Rotate sensor
                {
                    iSuckR=0;
                    //iSuckC=3;
                    iSuckC=0;                                                   //Sam 20250618 : 修正 Single Rotate 錯誤
                    iTrayX=0;
                    iTrayY=0;
                    if(i!=0 || j!=0)                                            //Steven 20190809 : Fixed for hang up
                        continue;
                }    */
                else
                {                                                               //Kit(0, 1) -> Suck(0, 2) or Suck(1, 3)
                    iSuckR=i;
                    if(W906RKO_DUTNUM==tDutType_8)                                     //GATE 1 (banner A): golden `if(tRotate.DutNum==tDutType_8)` //kevin 20130517 add 8 Site
                        iSuckC=j;
                    else
                        iSuckC=j*i2x2Suck_Out+iKit;

                    iTrayX=j;
                    iTrayY=i;
                }

                if(MOT[MOutRotateKit].Tray.Data[iTrayX][iTrayY]!=NULL_IC &&
                   MOT[MOutRotateKit].Tray.Data[iTrayX][iTrayY]!=HAS_NULL_IC)
                {
                    if(CheckRotateOutAnglePostion(iTrayY, iTrayX, PickAngle, 1)==true)    //Ifor 20170412 (Steven) Modify 角度已算過不需在減掉原本角度
                    {
                        OutArmSuck.Suck[iSuckR][iSuckC].SetNeedSuck(true);
                        flag[iSuckR][iSuckC]=MOT[OutArmSuck.Suck[iSuckR][iSuckC].iMotNo].MotorMove2SpeedForPicker(Prod.iOutArm_RotatePick[iSuckR][iSuckC], &ArmSpeed[OutArm]);  //Sam 20250428 : 修正 1X2 OutRotate 旋轉異常
                    }
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
//---------------------------------------------------------------------------
bool M_DoOutArmRotate_InOutArm()                                                //add One sucker with rotate
{
    int &Task=iOutArmRotateKit;
    int ret;
    static int rCount;

    switch(Task)
    {
        case 1:
            rCount=1;
//            DoRotate_Out(rCount, true);
            ret=SetMotorResolution(OutAngle45, OutAngle90, false);
            Task=100;
            break;
        case 100:
            ret=DoRotate_Out(rCount);

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
            ret=ShowErrorMessage("JAM0553", K_RETRY, MInRotateKit);             //kevin 20130524

            if(ret==K_RETRY)
            {
                Task=2100;
            }
            break;
        case 2100:
            FrmRotate->bRotateOutHome=true;                                     //kevin 20130706
            FrmRotate->InitialOutRotateHome();                                  //Steven 20170329 (wei) : Add individual rotate motor
            Task=2200;
            break;
        case 2200:
            if(FrmRotate->bRotateOutHome==false)                                //kevin 20130706
            {
//AI(W906-RKOUT) 20260807: GATE 8 (banner A).  golden :1421
//  `FrmRotate->SetOutRotateSpeed(ArmSpeed[OutRotate].iACDCBodySP, ArmSpeed[OutRotate].iBodySP);`
//  forms/fRotate.h's TFrmRotate stand-in deliberately did NOT land
//  SetInRotateSpeed/SetOutRotateSpeed (its own banner says so).  Only the CALL is
//  gated; the guard above and the Task transition below stay ACTIVE so the
//  alarm-recovery SM still converges.
#if 0 // TODO(W7-form): golden :1421 FrmRotate->SetOutRotateSpeed (not on the forms/fRotate.h stand-in)
                FrmRotate->SetOutRotateSpeed(ArmSpeed[OutRotate].iACDCBodySP, ArmSpeed[OutRotate].iBodySP);    //Steven 20170329 (wei) : Add individual rotate motor
#endif
                Task=100;
            }
            break;
    }
    return false;
}
//------------------------------------------------------------------------------
int GotOutRotateKitForSingle(int &iKit, int &iCount, bool bRelease)
{
    int Task=1000;
    if(bRelease)
    {
        Task=1000;
        if(OutArmSuck.Item[iRotato_Out_Row][0]!=NULL_IC && OutArmSuck.Item[iRotato_Out_Row][0]!=HAS_NULL_IC && iCount==0)
        {
            iKit=0;
            Task=1100;
        }
        else if((OutArmSuck.Item[iRotato_Out_Row][0]==NULL_IC || OutArmSuck.Item[iRotato_Out_Row][0]==HAS_NULL_IC) && iCount==0)
        {
            iCount=1;
            Task=1000;
        }
        else if(OutArmSuck.Item[iRotato_Out_Row][1]!=NULL_IC && OutArmSuck.Item[iRotato_Out_Row][1]!=HAS_NULL_IC && iCount==1)
        {
            iKit=1;
            Task=1100;
        }
        else if((OutArmSuck.Item[iRotato_Out_Row][1]==NULL_IC || OutArmSuck.Item[iRotato_Out_Row][1]==HAS_NULL_IC) && iCount==1)
        {
            iCount=2;
            Task=1000;
        }
        else if(OutArmSuck.Item[iRotato_Out_Row][2]!=NULL_IC && OutArmSuck.Item[iRotato_Out_Row][2]!=HAS_NULL_IC && iCount==2)
        {
            iKit=2;
            Task=1100;
        }
        else if((OutArmSuck.Item[iRotato_Out_Row][2]==NULL_IC || OutArmSuck.Item[iRotato_Out_Row][2]==HAS_NULL_IC) && iCount==2)
        {
            iCount=3;
            Task=1000;
        }
        else if(OutArmSuck.Item[iRotato_Out_Row][3]!=NULL_IC && OutArmSuck.Item[iRotato_Out_Row][3]!=HAS_NULL_IC && iCount==3)
        {
            iKit=3;
            Task=1100;
        }
        else if(iRotato_Out_Row==1)
        {
            Task=6000;
        }
        else
        {
            iRotato_Out_Row=1;
            Task=1000;
        }
    }
    else
    {
        Task=1;
        if(iRotato_Out_Row==0)
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
                iRotato_Out_Row=1;
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
                iRotato_Out_Row=0;
                iKit=0;
                MOT[MOutRotateKit].SetHasNullIcToNullIc();                      //Frank 20170822 (wei): 清空Rotate資料
                Task=1;
            }
        }
    }
    return Task;
}
//------------------------------------------------------------------------------
int GotOutRotateKitForAxxG(int &iKit, int &iCount, bool bRelease)
{
    int Task=1000;
    if(bRelease)
    {
        if(((OutArmSuck.Item[iRotato_Out_Row][0]!=NULL_IC && OutArmSuck.Item[iRotato_Out_Row][0]!=HAS_NULL_IC) ||
            (OutArmSuck.Item[iRotato_Out_Row][1]!=NULL_IC && OutArmSuck.Item[iRotato_Out_Row][1]!=HAS_NULL_IC)) &&
           iCount==0)
        {
            iKit=0;
            Task=1100;
        }
        else if((OutArmSuck.Item[iRotato_Out_Row][0]==NULL_IC || OutArmSuck.Item[iRotato_Out_Row][0]==HAS_NULL_IC ||
                 OutArmSuck.Item[iRotato_Out_Row][1]==NULL_IC || OutArmSuck.Item[iRotato_Out_Row][1]==HAS_NULL_IC) &&
                iCount==0)
        {
            iCount=1;
            Task=1000;
        }
        else if(iRotato_Out_Row==1)
        {
            Task=6000;
        }
        else
        {
            iRotato_Out_Row=1;
            Task=1000;
        }
    }
    else
    {
        Task=10;
        if(iRotato_Out_Row==0)
        {
            iCount=0;
            iRotato_Out_Row=1;
            iKit=0;
            Task=10;
        }
        else
        {
            iCount=0;
            iRotato_Out_Row=0;
            iKit=0;
            MOT[MOutRotateKit].SetHasNullIcToNullIc();                          //Frank 20170822 (wei): 清空Rotate資料
            Task=1;
        }
    }
    return Task;
}
//------------------------------------------------------------------------------
int GotOutRotateKitForAxEx_2RotMot(int &iKit, int &iCount, bool bRelease)
{
    int Task=1000;
    if(bRelease)
    {
        Task=1000;
        if(((OutArmSuck.Item[iRotato_Out_Row][0]!=NULL_IC && OutArmSuck.Item[iRotato_Out_Row][0]!=HAS_NULL_IC) ||
            (OutArmSuck.Item[iRotato_Out_Row][1]!=NULL_IC && OutArmSuck.Item[iRotato_Out_Row][1]!=HAS_NULL_IC)) &&
           iCount==0)
        {
            iKit=0;
            Task=1100;
        }
        else if((OutArmSuck.Item[iRotato_Out_Row][0]==NULL_IC || OutArmSuck.Item[iRotato_Out_Row][0]==HAS_NULL_IC ||
                 OutArmSuck.Item[iRotato_Out_Row][1]==NULL_IC || OutArmSuck.Item[iRotato_Out_Row][1]==HAS_NULL_IC) &&
                iCount==0)
        {
            iCount=1;
            Task=1000;
        }
        else if(((OutArmSuck.Item[iRotato_Out_Row][2]!=NULL_IC && OutArmSuck.Item[iRotato_Out_Row][2]!=HAS_NULL_IC) ||
                 (OutArmSuck.Item[iRotato_Out_Row][3]!=NULL_IC && OutArmSuck.Item[iRotato_Out_Row][3]!=HAS_NULL_IC)) &&
                iCount==1)
        {
            iKit=1;
            Task=1100;
        }
        //AI(W906-RKOUT) 20260807: E-4 GOLDEN BUG preserved VERBATIM (golden :1619-1621) --
        //  this arm tests `iCount==0`, which the 2nd `else if` above already consumed,
        //  so the branch is UNREACHABLE and iCount never becomes 2 here.  Not fixed.
        else if((OutArmSuck.Item[iRotato_Out_Row][2]==NULL_IC || OutArmSuck.Item[iRotato_Out_Row][2]==HAS_NULL_IC ||
                 OutArmSuck.Item[iRotato_Out_Row][3]==NULL_IC || OutArmSuck.Item[iRotato_Out_Row][3]==HAS_NULL_IC) &&
                iCount==0)
        {
            iCount=2;
            Task=1000;
        }
        else if(iRotato_Out_Row==1)
        {
            Task=6000;
        }
        else
        {
            iRotato_Out_Row=1;
            Task=1000;
        }
    }
    else
    {
        Task=1;
        if(iRotato_Out_Row==0)
        {
            if(iCount==0)
            {
                iKit=1;
                iCount=1;
            }
            else
            {
                iCount=0;
                iRotato_Out_Row=1;
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
                iRotato_Out_Row=0;
                iKit=0;
                MOT[MOutRotateKit].SetHasNullIcToNullIc();                       //Frank 20170822 (wei): 清空Rotate資料
                Task=1;
            }
        }
    }
    return Task;
}
//---------------------------------------------------------------------------
bool M_DoOutArmRotateKIT_Motor()
{
    int &Task=iOutArmRotateKit;
    int j2;
    int iSuckR, iSuckC, iTrayY, iTrayX, iRealR, iRealC;

    static int iRetryCT=0;
    static int iKit=0;
    static int rCount=0;
    bool flag=true;
    int iRotateCount=0;                                                         //wei 20141230 Rotate旋轉問題 for 大IC單獨吸
    int ret;
    static int iOutCount=0;
    bool bHasError=false;
    bool bHasDuplicateErr=false;
    static bool bSuckDuplicateErr[MAX_ARM_Row][MAX_ARM_Col]={false};            //Steven 20091218 : Avoid duplicate message
    static bool bRotateMoveOk=false, bOutXYMoveOk=false;
    AnsiString ErrPart="";

    int iMAX_ARM_Row, iMAX_ARM_Col;
    if(W906RKO_DUTNUM==tDutType_8)                                                     //GATE 1 (banner A): golden `if(tRotate.DutNum==tDutType_8)` //kevin 20130517 add 8 site
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
            iKit=0;
            Task=10;
            iOutCount=0;
            MoveOutRotateToDegreeAtSameTime(0, true);                           //Steven 20170504 (wei) : 針對獨立馬達旋轉站, 一起旋轉到特定角度
            // NOTE: faithful fall-through to case 10 (no break in golden :1712)
        case 10:                                                                //jou 2013-05-08 不等旋轉完成
            ret=SetMotorResolution(OutAngle45, OutAngle90, false);              //kevin 20131003
            Task=100;
            // NOTE: faithful fall-through to case 100 (no break in golden :1715)
        case 100:
            MOT[MOutRotateKit].SetTray(NULL_IC, __FUNC__);
            ZeroMemory(bSuckDuplicateErr, sizeof(bSuckDuplicateErr));
            Task=1000;
            // NOTE: faithful fall-through to case 1000 (no break in golden :1719)
        case 1000:
            if(iRotate_Type==e1MotRotate1Dut)                                   //Frank 20180512 (Steven) : modify 1 Dut 1 Motor
            {
                Task=GotOutRotateKitForSingle(iKit, iOutCount, true);
            }
            else if(iRotate_Type==e2MotRotate2Dut)
            {
                if(OutArmSuck.iPickCol==1)
                {
                    Task=GotOutRotateKitForSingle(iKit, iOutCount, true);
                }
                else if(bUseAxxGPicker() ||
                        iCloseSiteModeFor1x4==e1x4CloseAbAc)                    //Steven 20241111 : for 1x4 close 2 site
                {
                    Task=GotOutRotateKitForAxxG(iKit, iOutCount, true);
                }
                else
                {
                    Task=GotOutRotateKitForAxEx_2RotMot(iKit, iOutCount, true);
                }
            }
            else if(W906RKO_DUTNUM==tDutType_8)                                        //GATE 1 (banner A): golden `else if(tRotate.DutNum==tDutType_8)` //kevin 20130517 add 8 site
            {
                if(iInArmType==e9045_1x2_2_14 || iInArmType==e9045_2x2_4_14 || iInArmType==e9045_2x4_4_14)    //Steven 20201028 : for rotator
                {
                    if(TestIF.iTestMode==QualSite2X2 && DeviceForm.XDimension>=5000)    //wei 20151229  Xilinx 52.5x52.5 放一個Rotate  //Alick 20160105 (wei) modify IC>=50x50 放一個Rotate
                    {
                        if(OutArmSuck.Item[iRotato_Out_Row][0]!=NULL_IC && OutArmSuck.Item[iRotato_Out_Row][0]!=HAS_NULL_IC && iOutCount==0)
                        {
                            iKit=0;
                            Task=1100;
                        }
                        else if((OutArmSuck.Item[iRotato_Out_Row][0]==NULL_IC || OutArmSuck.Item[iRotato_Out_Row][0]==HAS_NULL_IC) && iOutCount==0)
                        {
                            iOutCount=1;
                            Task=1000;
                        }
                        else if(OutArmSuck.Item[iRotato_Out_Row][2]!=NULL_IC && OutArmSuck.Item[iRotato_Out_Row][2]!=HAS_NULL_IC && iOutCount==1)
                        {
                            iKit=1;
                            Task=1100;
                        }
                        else if(iRotato_Out_Row==1)
                        {
                            Task=6000;
                        }
                        else
                        {
                            iRotato_Out_Row=1;
                            Task=1000;
                        }
                    }
                    else
                    {
                        if(OutArmSuck.Item[iRotato_Out_Row][0]!=NULL_IC && OutArmSuck.Item[iRotato_Out_Row][0]!=HAS_NULL_IC)
                        {
                            iKit=0;
                            Task=1100;
                        }
                        else if(OutArmSuck.Item[iRotato_Out_Row][2]!=NULL_IC && OutArmSuck.Item[iRotato_Out_Row][2]!=HAS_NULL_IC)
                        {
                            iKit=1;
                            Task=1100;
                        }
                        else if(iRotato_Out_Row==1)
                        {
                            Task=6000;
                        }
                        else
                        {
                            iRotato_Out_Row=1;
                            Task=1000;
                        }
                    }
                }
                else
                {
                    Task=1100;
                }
            }
            else
            {
                if(iRotate_Type==e2MotRotate2Dut)                               //wei 20190515 modify 2 Dut 2 Motor
                {
                    if(TestIF.iTestMode==SingleSite)
                    {
                        if(OutArmSuck.Item[0][0]!=NULL_IC && OutArmSuck.Item[0][0]!=HAS_NULL_IC)
                        {
                            iKit=1;
                            Task=1100;
                        }
                        else
                        {
                            Task=2000;
                        }
                    }
                    else if(TestIF.iTestMode==DualSite)
                    {
                        if((OutArmSuck.Item[0][0]!=NULL_IC && OutArmSuck.Item[0][0]!=HAS_NULL_IC) ||
                           (OutArmSuck.Item[0][2]!=NULL_IC && OutArmSuck.Item[0][2]!=HAS_NULL_IC) )
                        {
                            Task=1100;
                        }
                        else
                        {
                            Task=2000;
                        }
                    }
                    else
                    {
                        Task=2000;
                    }
                }
                /*else if(bSingleDut)                                             //Ifor 20180110 (Steven) : add 僅有一個Rotate sensor
                {
                     if(OutArmSuck.Item[0][3]!=NULL_IC && OutArmSuck.Item[0][3]!=HAS_NULL_IC)
                     {
                        Task=1100;
                     }
                     else
                     {
                        Task=2000;
                     }
                }  */
                else
                {
                    if((OutArmSuck.Item[0][0+iKit]!=NULL_IC             && OutArmSuck.Item[0][0+iKit]!=HAS_NULL_IC) ||
                       (OutArmSuck.Item[0][i2x2Suck_Out+iKit]!=NULL_IC  && OutArmSuck.Item[0][i2x2Suck_Out+iKit]!=HAS_NULL_IC) ||
                       (OutArmSuck.Item[1][0+iKit]!=NULL_IC             && OutArmSuck.Item[1][0+iKit]!=HAS_NULL_IC) ||
                       (OutArmSuck.Item[1][i2x2Suck_Out+iKit]!=NULL_IC  && OutArmSuck.Item[1][i2x2Suck_Out+iKit]!=HAS_NULL_IC))
                    {
                        Task=1100;
                    }
                    else
                    {
                        Task=2000;
                    }
                }
            }
            break;
        case 1100:                                                              //wei 20160106 增加放Rotate前偵測
            if(CheckRotateSensor(false))
            {
                Task=1150;
            }
            else
            {
                Task=1120;
            }
            break;
        case 1120:
            if(MoveOutArmXY_ToShuttleAlarmArea()==false)                        //如果要移開,而且還沒移到定位
            {
                return false;                                                   //先離開等In Arm
            }
            Task=1130;
            break;
        case 1130:                                                              //out Rotate 疊料
            ShowErrorMessage("JAM0552", K_RETRY, MOutRotateKit);
            Task=1100;
            break;
        case 1150:
        //Steven 20170505 (wei) : for 提升Rotate的UPH
        //==>
            MoveOutRotateToDegreeAtSameTime(0, true);
            bRotateMoveOk=false;
            bOutXYMoveOk=false;
            Task=1160;
            // NOTE: faithful fall-through to case 1160 (no break in golden :1887)
        case 1160:
            if(bRotateMoveOk==false)                                            //Steven 20170504 (wei) : 針對獨立馬達旋轉站, 一起旋轉到特定角度
                bRotateMoveOk=MoveOutRotateToDegreeAtSameTime(0);

            if(bOutXYMoveOk==false)
                bOutXYMoveOk=M_MoveOutArmXY_ToRotateKIT(iKit);
        //<==
        //Steven 20170505 (wei) : for 提升Rotate的UPH

            if(bRotateMoveOk && bOutXYMoveOk)
            {
                CheckOutArmSuckICFallDown();

                for(int i=0; i<MAX_ARM_Row; i++)
                {
                    for(int j=0; j<MAX_ARM_Col; j++)
                    {
                        OutArmSuck.Suck[i][j].SetNeedDestroy(false);               //AI(W906-RKOUT) 20260807: A-1 (banner C) -- SetNeedDestroy is the TU-local macro (golden MyKitSuck.h:122) //kevin 20130524   //jou 20231012 : 修正rotate預設值true->false
                    }
                }
                Task=1200;
            }
            else
            {
                CheckOutArmSuckICFallDown();
                break;
            }
            // NOTE: faithful fall-through to case 1200 (golden :1914 -- only the else breaks)
        case 1200:
            if(M_MoveOutArmZ_ToRotateKIT_Place(iKit)==true)
            {
                if((AccessLevel>=1 || CosFunction.bOPCanPressStepAndTStart) &&  //JerryYang 20170417 (wei) OP權限也能按Step跟T.Start
                    Sen[SnRKManualStep].IsOn())
                {
                    bEnterOffset=false;
                    fMain->Pause("M_DoOutArmRotateKIT_Motor 1200");
                    Task=1500;
                    break;
                }

                OutArmSuck.ResetAll();                                          //Steven 20160323 : 避免未開啟真空
                Task=1300;
            }
            else
            {
                break;
            }
            // NOTE: faithful fall-through to case 1300 (golden :1933)
        case 1300:
            flag=true;
            if(iRotate_Type==e1MotRotate1Dut)                                   //Frank 20180512 (Steven) : modify 1 Dut 1 Motor
            {
                for(int j=iKit; j<OutArmSuck.iMaxCol; j+=4)
                {
                    iTrayY=0;
                    iTrayX=0;
                    iSuckR=iRotato_Out_Row;
                    iSuckC=j;
                    if(CosFunction.bPassBinNoRotate==true && W906RKO_TROT_PASSBINNOROTATE==true && OutArmSuck.bPass[iSuckR][iSuckC]==true) //GATE 2 (banner A) //jou 20231020 : Pass bin no rotate
                    {
                    }
                    else if(OutArmSuck.Item[iSuckR][iSuckC] && OutArmSuck.Suck[iSuckR][iSuckC].GetNeedDestroyStatus() || OutArmSuck.Item[iSuckR][iSuckC]==HAS_NULL_IC)   //AI(W906-RKOUT) 20260807: E-5 (unparenthesised &&/||) kept verbatim
                    {
                        if(OutArmSuck.Item[iSuckR][iSuckC]==HAS_NULL_IC || OutArmSuck.Suck[iSuckR][iSuckC].Destroy())
                        {
                            bSuckDuplicateErr[iSuckR][iSuckC]=false;
                            OutArmSuck.CopyToTray(iSuckR, iSuckC, NULL_IC, MOT[MOutRotateKit], iTrayY, iTrayX, OutArmSuck.Item[iSuckR][iSuckC]);
                            if(OutArmSuck.Item[iSuckR][iSuckC]!=HAS_NULL_IC)    //Steven 20170109 (Jou) : 沒IC的地方不檢查吹氣
                                bOutArmCheckDestroyACT[iSuckR][iSuckC]=true;    //kevin 20140220 回吸檢測 jou 981130 確認device確實destroy完成
                        }
                        else if(OutArmSuck.Suck[iSuckR][iSuckC].Error==false)
                        {
                            flag=false;
                        }
                        else if(OutArmSuck.Suck[iSuckR][iSuckC].Error)
                        {
                            bHasError=true;
                        }
                    }
                }
            }
            else if(iRotate_Type==e2MotRotate2Dut)
            {
                if(OutArmSuck.iPickCol==1)
                {
                    for(int j=iKit; j<OutArmSuck.iMaxCol; j+=4)
                    {
                        iTrayY=0;
                        iTrayX=0;
                        iSuckR=iRotato_Out_Row;
                        iSuckC=0;
                        if(CosFunction.bPassBinNoRotate==true && W906RKO_TROT_PASSBINNOROTATE==true && OutArmSuck.bPass[iSuckR][iSuckC]==true) //GATE 2 (banner A) //jou 20231020 : Pass bin no rotate
                        {
                        }
                        else if(OutArmSuck.Item[iSuckR][iSuckC] && OutArmSuck.Suck[iSuckR][iSuckC].GetNeedDestroyStatus() || OutArmSuck.Item[iSuckR][iSuckC]==HAS_NULL_IC)
                        {
                            if(OutArmSuck.Item[iSuckR][iSuckC]==HAS_NULL_IC || OutArmSuck.Suck[iSuckR][iSuckC].Destroy())
                            {
                                bSuckDuplicateErr[iSuckR][iSuckC]=false;
                                OutArmSuck.CopyToTray(iSuckR, iSuckC, NULL_IC, MOT[MOutRotateKit], iTrayY, iTrayX, OutArmSuck.Item[iSuckR][iSuckC]);
                                if(OutArmSuck.Item[iSuckR][iSuckC]!=HAS_NULL_IC)    //Steven 20170109 (Jou) : 沒IC的地方不檢查吹氣
                                    bOutArmCheckDestroyACT[iSuckR][iSuckC]=true;    //kevin 20140220 回吸檢測 jou 981130 確認device確實destroy完成
                            }
                            else if(OutArmSuck.Suck[iSuckR][iSuckC].Error==false)
                            {
                                flag=false;
                            }
                            else if(OutArmSuck.Suck[iSuckR][iSuckC].Error)
                            {
                                bHasError=true;
                            }
                        }
                    }
                }
                else if(OutArmSuck.iPickCol==2)
                {
                    for(int j=0; j<2; j++)
                    {
                        iTrayY=0;
                        iTrayX=j;
                        iSuckR=iRotato_Out_Row;
                        iSuckC=j;
                        if(CosFunction.bPassBinNoRotate==true && W906RKO_TROT_PASSBINNOROTATE==true && OutArmSuck.bPass[iSuckR][iSuckC]==true) //GATE 2 (banner A) //jou 20231020 : Pass bin no rotate
                        {
                        }
                        else if(OutArmSuck.Item[iSuckR][iSuckC] && OutArmSuck.Suck[iSuckR][iSuckC].GetNeedDestroyStatus() || OutArmSuck.Item[iSuckR][iSuckC]==HAS_NULL_IC)
                        {
                            if(OutArmSuck.Item[iSuckR][iSuckC]==HAS_NULL_IC || OutArmSuck.Suck[iSuckR][iSuckC].Destroy())
                            {
                                bSuckDuplicateErr[iSuckR][iSuckC]=false;
                                OutArmSuck.CopyToTray(iSuckR, iSuckC, NULL_IC, MOT[MOutRotateKit], iTrayY, iTrayX, OutArmSuck.Item[iSuckR][iSuckC]);
                                if(OutArmSuck.Item[iSuckR][iSuckC]!=HAS_NULL_IC)    //Steven 20170109 (Jou) : 沒IC的地方不檢查吹氣
                                    bOutArmCheckDestroyACT[iSuckR][iSuckC]=true;    //kevin 20140220 回吸檢測 jou 981130 確認device確實destroy完成
                            }
                            else if(OutArmSuck.Suck[iSuckR][iSuckC].Error==false)
                            {
                                flag=false;
                            }
                            else if(OutArmSuck.Suck[iSuckR][iSuckC].Error)
                            {
                                bHasError=true;
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
                        iSuckR=iRotato_Out_Row;
                        iSuckC=j*2+iKit;

                        if(CosFunction.bPassBinNoRotate==true && W906RKO_TROT_PASSBINNOROTATE==true && OutArmSuck.bPass[iSuckR][iSuckC]==true) //GATE 2 (banner A) //jou 20231020 : Pass bin no rotate
                        {
                        }
                        else if(OutArmSuck.Item[iSuckR][iSuckC] && OutArmSuck.Suck[iSuckR][iSuckC].GetNeedDestroyStatus() || OutArmSuck.Item[iSuckR][iSuckC]==HAS_NULL_IC)
                        {
                            if(OutArmSuck.Item[iSuckR][iSuckC]==HAS_NULL_IC || OutArmSuck.Suck[iSuckR][iSuckC].Destroy())
                            {
                                bSuckDuplicateErr[iSuckR][iSuckC]=false;
                                OutArmSuck.CopyToTray(iSuckR, iSuckC, NULL_IC, MOT[MOutRotateKit], iTrayY, iTrayX, OutArmSuck.Item[iSuckR][iSuckC]);
                                if(OutArmSuck.Item[iSuckR][iSuckC]!=HAS_NULL_IC)    //Steven 20170109 (Jou) : 沒IC的地方不檢查吹氣
                                    bOutArmCheckDestroyACT[iSuckR][iSuckC]=true;    //kevin 20140220 回吸檢測 jou 981130 確認device確實destroy完成
                            }
                            else if(OutArmSuck.Suck[iSuckR][iSuckC].Error==false)
                            {
                                flag=false;
                            }
                            else if(OutArmSuck.Suck[iSuckR][iSuckC].Error)
                            {
                                bHasError=true;
                            }
                        }
                    }
                }
            }
            else if(W906RKO_DUTNUM==tDutType_8)                                        //GATE 1 (banner A): golden `else if(tRotate.DutNum==tDutType_8)` //Steven 20160615 : fixed for 2x4_4 wirh rotate
            {
                if(iInArmType==e9045_1x2_2_14 || iInArmType==e9045_2x2_4_14 || iInArmType==e9045_2x4_4_14)    //Steven 20201028 : for rotator
                {
                    for(int j=iKit; j<iMAX_ARM_Col; j+=4)
                    {
                        //Steven 20170504 (wei) : 修改Rotate資料交換方式
                        //==>
                        iSuckR=iRotato_Out_Row;
                        iSuckC=j*2;
                        if(TestIF.iTestMode==QualSite2X2 && DeviceForm.XDimension>=5000)
                        {                                                       //wei 20151229  Xilinx 52.5x52.5 放一個Rotate     //wei 20170822 (Steven): ==5000 -> >=5000
                            iTrayX=1;
                            iTrayY=0;
                        }
                        else
                        {
                            iTrayX=1+iKit;
                            iTrayY=iKit;
                        }

                        if(CosFunction.bPassBinNoRotate==true && W906RKO_TROT_PASSBINNOROTATE==true && OutArmSuck.bPass[iSuckR][iSuckC]==true) //GATE 2 (banner A) //jou 20231020 : Pass bin no rotate
                        {
                        }
                        else if(OutArmSuck.Item[iSuckR][iSuckC] && OutArmSuck.Suck[iSuckR][iSuckC].GetNeedDestroyStatus() || OutArmSuck.Item[iSuckR][iSuckC]==HAS_NULL_IC)
                        {
                            if(OutArmSuck.Item[iSuckR][iSuckC]==HAS_NULL_IC || OutArmSuck.Suck[iSuckR][iSuckC].Destroy())
                            {
                                bSuckDuplicateErr[iSuckR][iSuckC]=false;
                                OutArmSuck.CopyToTray(iSuckR, iSuckC, NULL_IC, MOT[MOutRotateKit], iTrayY, iTrayX, OutArmSuck.Item[iSuckR][iSuckC]);
                                if(OutArmSuck.Item[iSuckR][iSuckC]!=HAS_NULL_IC) //Steven 20170109 (Jou) : 沒IC的地方不檢查吹氣
                                    bOutArmCheckDestroyACT[iSuckR][iSuckC]=true; //kevin 20140220 回吸檢測 jou 981130 確認device確實destroy完成
                            }
                            else if(OutArmSuck.Suck[iSuckR][iSuckC].Error==false)
                            {
                                flag=false;
                            }
                            else if(OutArmSuck.Suck[iSuckR][iSuckC].Error)
                            {
                                bHasError=true;
                            }
                        }
                        //<==
                        //Steven 20170504 (wei) : 修改Rotate資料交換方式
                    }
                }
                else
                {
                    for(int i=0; i<iMAX_ARM_Row; i++)
                    {
                        for(int j=0; j<iMAX_ARM_Col; j++)
                        {
                            //Steven 20170504 (wei) : 修改Rotate資料交換方式
                            //==>
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

                            if(CosFunction.bPassBinNoRotate==true && W906RKO_TROT_PASSBINNOROTATE==true && OutArmSuck.bPass[iSuckR][iSuckC]==true) //GATE 2 (banner A) //jou 20231020 : Pass bin no rotate
                            {
                            }
                            else if(OutArmSuck.Item[iSuckR][iSuckC] && OutArmSuck.Suck[iSuckR][iSuckC].GetNeedDestroyStatus() || OutArmSuck.Item[iSuckR][iSuckC] == HAS_NULL_IC)
                            {
                                if(OutArmSuck.Item[iSuckR][iSuckC]==HAS_NULL_IC || OutArmSuck.Suck[iSuckR][iSuckC].Destroy())
                                {
                                    bSuckDuplicateErr[iSuckR][iSuckC]=false;
                                    OutArmSuck.CopyToTray(iSuckR, iSuckC, NULL_IC, MOT[MOutRotateKit], iTrayY, iTrayX, OutArmSuck.Item[iSuckR][iSuckC]);
                                    if(OutArmSuck.Item[iSuckR][iSuckC]!=HAS_NULL_IC) //Steven 20170109 (Jou) : 沒IC的地方不檢查吹氣
                                        bOutArmCheckDestroyACT[iSuckR][iSuckC]=true; //kevin 20140220 回吸檢測 jou 981130 確認device確實destroy完成
                                }
                                else if(OutArmSuck.Suck[iSuckR][iSuckC].Error==false)
                                {
                                    flag=false;
                                }
                                else if(OutArmSuck.Suck[iSuckR][iSuckC].Error)
                                {
                                    bHasError=true;
                                }
                            }
                            //<==
                            //Steven 20170504 (wei) : 修改Rotate資料交換方式
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
                        if(iRotate_Type==e2MotRotate2Dut && TestIF.iTestMode==SingleSite)   //wei 20190515 modify 2 Dut 2 Motor
                        {
                            iSuckR=0;
                            //iSuckC=3;
                            iSuckC=0;   //Sam 20250618 : 修正 Single Rotate 錯誤
                            iTrayX=1;
                            iTrayY=0;
                        }
                        else
                        {
                            //Steven 20170504 (wei) : 修改Rotate資料交換方式
                            //==>
                            iSuckR=i;
                            iSuckC=j*i2x2Suck_Out+iKit;

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
                        }

                        if(CosFunction.bPassBinNoRotate==true && W906RKO_TROT_PASSBINNOROTATE==true && OutArmSuck.bPass[iSuckR][iSuckC]==true) //GATE 2 (banner A) //jou 20231020 : Pass bin no rotate
                        {
                        }
                        else if(OutArmSuck.Item[iSuckR][iSuckC] && OutArmSuck.Suck[iSuckR][iSuckC].GetNeedDestroyStatus() || OutArmSuck.Item[iSuckR][iSuckC] == HAS_NULL_IC)
                        {
                            if(OutArmSuck.Item[iSuckR][iSuckC]==HAS_NULL_IC || OutArmSuck.Suck[iSuckR][iSuckC].Destroy())
                            {
                                bSuckDuplicateErr[iSuckR][iSuckC]=false;
                                OutArmSuck.CopyToTray(iSuckR, iSuckC, NULL_IC, MOT[MOutRotateKit], iTrayY, iTrayX, OutArmSuck.Item[iSuckR][iSuckC]);
                                if(OutArmSuck.Item[iSuckR][iSuckC]!=HAS_NULL_IC) //Steven 20170109 (Jou) : 沒IC的地方不檢查吹氣
                                    bOutArmCheckDestroyACT[iSuckR][iSuckC]=true; //kevin 20140220 回吸檢測 jou 981130 確認device確實destroy完成
                            }
                            else if(OutArmSuck.Suck[iSuckR][iSuckC].Error==false)
                            {
                                flag=false;
                            }
                            else if(OutArmSuck.Suck[iSuckR][iSuckC].Error)
                            {
                                bHasError=true;
                            }
                        }
                        //<==
                        //Steven 20170504 (wei) : 修改Rotate資料交換方式
                    }
                }
            }

            if(flag==false)                                                     //kevin 20130517
                break;

            if(bHasError)
            {
                Task=1600;
                break;
            }

            ZeroMemory(bSuckDuplicateErr, sizeof(bSuckDuplicateErr));
            Task=2000;
            break;
         // 例外處理 -----------------------------
        case 1500:      //(0, 2)調整Offset
            if(MoveOutArmToAutoSafe())
            {
                bEnterOffset=false;
                Task=1100;
            }
            break;
        case 1600:      //(0, 2)吹氣異常
            if(MoveOutArmToAutoSafe())
            {
                Task=1601;
            }
            break;
        case 1601:
            ErrPart=" ";
            for(int i=0; i<MAX_ARM_Row; i++)
            {
                for(int j=0; j<MAX_ARM_Col; j++)
                {
                    if(bSuckDuplicateErr[i][j])
                        bHasDuplicateErr=true;
                    if(OutArmSuck.Suck[i][j].Error)
                    {
                        ErrPart+=OutArmSuck.Suck[i][j].sName;
                        bSuckDuplicateErr[i][j]=true;
                        OutArmSuck.Suck[i][j].Error=false;
                    }
                    else
                    {
                        bSuckDuplicateErr[i][j]=false;
                    }
                }
            }
            ShowErrorMessage("JAM0217", K_RETRY, MOutArmX, bHasDuplicateErr, ErrPart);
            Task=1200;
            break;
        case 2000:
            if(MoveOutArmToAutoSafe())
            {
                if(iRotate_Type==e1MotRotate1Dut ||                             //Frank 20180512 (Steven) : modify 1 Dut 1 Motor
                   iRotate_Type==e2MotRotate2Dut)
                {
                    Task=2500;
                }
                else if(W906RKO_DUTNUM==tDutType_8)                                    //GATE 1 (banner A): golden `else if(tRotate.DutNum==tDutType_8)` //kevin 20130517 add 8 site
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
                                if(OutArmSuck.Item[iRotato_Out_Row][2]!=NULL_IC && OutArmSuck.Item[iRotato_Out_Row][2]!=HAS_NULL_IC)
                                {
                                    iKit=1;
                                    Task=1100;
                                }
                                else
                                {
                                    iKit=0;
                                    Task=2500;
                                }
                                break;
                            }
                            else
                            {
                                iKit=0;
                                Task=2500;
                            }
                        }
                        break;
                    }
                    else if(OutArmSuck.FindNoIC())                              //kevkin 20130519 IC已放下
                    {
                        iKit=0;
                    }
                }
                else
                {
                }
                Task=2500;
                break;
            }
            break;
        case 2500:                                                              //偵測IC 是否疊料
            if(CheckRotateSensor(false))
            {
                Task=2510;
            }
            else
            {
                if(IniConfig.bA21RotateDetectErrNeedShake)
                {
                    InitialShakeOutArmRotateKIT();
                    Task=2501;
                }
                else
                {
                    Task=2502;
                }
            }
            break;
        case 2501:
            if(M_DoShakeOutArmRotateKIT()==true)                                //JerryYang 20160804 偵測到異常要轉三下
            {
                if(CheckRotateSensor(false))                                    //JerryYang 20160804 轉完再偵測一次是否疊料
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
            if(MoveOutArmXY_ToShuttleAlarmArea()==false)                        //如果要移開,而且還沒移到定位
            {
                return false;                                                   //先離開等In Arm
            }
            Task=2503;
            break;
        case 2503:                                                              //out Rotate 疊料
            ShowErrorMessage("JAM0552", K_RETRY, MOutRotateKit);
            Task=2500;
            break;
        case 2510:
            DoRotate_Out(rCount, true);
            if(W906RKO_SHOWROTATEBYSITE==true)                                //GATE 6 (banner A): golden `if(FrmRotate->bShowRotateBySite==true)` //Steven 20170329 (wei) : Add individual rotate motor
            {
                Task=2600;
            }
            else if(W906RKO_DUTNUM==tDutType_8)                                        //GATE 1 (banner A): golden `else if(tRotate.DutNum==tDutType_8)`
            {
                if(iInArmType==e9045_1x2_2_14 || iInArmType==e9045_2x2_4_14 || iInArmType==e9045_2x4_4_14)    //Steven 20201028 : for rotator
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
            DoRotate_Out(rCount, true);
            if(iInArmType==e9045_1x2_2_14 ||                                    //Steven 20201028 : for rotator
               iInArmType==e9045_2x2_4_14 ||
               iInArmType==e9045_2x4_4_14  ||
               iRotate_Type==eInOutArm1Motor)
            {
                for(iRotateCount=4; iRotateCount>=0; iRotateCount--)
                {
                    if(TestIF.iTestMode==QualSite2X2 && DeviceForm.XDimension>=5000)       //wei 20151229  Xilinx 52.5x52.5 放一個Rotate  //Alick 20160105 (wei) modify IC>=50x50 放一個Rotate
                    {
                        if(CheckRotateOutAnglePostion(0, 1, iRotateCount-1, 0)==true)       //Ifor 20170412 (Steven) Modify 角度已算過不需在減掉原本角度
                        {
                            rCount=iRotateCount;
                            Task=2600;
                            break;
                        }
                    }
                    else
                    {
                        if(CheckRotateOutAnglePostion(iKit, iKit+1, iRotateCount-1, 0)==true)   //Ifor 20170412 (Steven) Modify 角度已算過不需在減掉原本角度
                        {
                            rCount=iRotateCount;
                            Task=2600;
                            break;
                        }
                    }
                }
            }
            break;
        case 2600:
            if(iRotate_Type==e1MotRotate1Dut)                                   //Frank 20180512 (Steven) : modify 1 Dut 1 Motor
            {
            }
            else if(iRotate_Type==e2MotRotate2Dut)
            {
                for(int j=0; j<2; j++)
                {
                    if(MOT[MOutRotateKit].Tray.Data[j][0]!=NULL_IC &&
                       MOT[MOutRotateKit].Tray.Data[j][0]!=HAS_NULL_IC &&
                       MOT[MOutRotateKit].Tray.iNeedRotAng[j][0]!=MOT[MOutRotateKit].Tray.iCurrRotAng[j][0])
                    {
                        DoRotate_Out(rCount, true);
                    }
                }
            }
            else if(W906RKO_SHOWROTATEBYSITE==true)                           //GATE 6 (banner A): golden `else if(FrmRotate->bShowRotateBySite==true)` //Sam 20240110 : 檢查這次的選轉位置若都為 Has_Null_IC 都不要轉了
            {
                if(rCount-1>0)
                {
                    bool bNeedR=false;
                    for(int i=0; i<iMAX_ARM_Row; i++)
                    {
                        for(int j=0; j<iMAX_ARM_Col; j++)
                        {
                            iSuckR=i;
                            iSuckC=j;
                            iTrayX=j;
                            iTrayY=i;
                            if(MOT[MOutRotateKit].Tray.Data[iTrayX][iTrayY]!=NULL_IC &&
                               MOT[MOutRotateKit].Tray.Data[iTrayX][iTrayY]!=HAS_NULL_IC &&
                               CheckRotateOutAnglePostion(iTrayY, iTrayX, rCount-1, 0)==true)
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

            ret=DoRotate_Out(rCount);

            if(ret==1)
            {
                bOut_ICRotationCompleteOnKit=true;                              //Sam 20210623 : 修正 IC 轉完後被 Home，導致 IC 轉向異常。
                Task=3000;
            }
            else if(ret==-999 || ret==-100)                                     //Error
            {
                Task=4800;
            }
            break;
        case 3000:
            Task=3100;
            // NOTE: faithful fall-through to case 3100 (no break in golden :2473)
        case 3100:                                                              //吸取IC
            if(iRotate_Type==e1MotRotate1Dut ||                                 //Frank 20180512 (Steven) : modify 1 Dut 1 Motor
               iRotate_Type==e2MotRotate2Dut)
            {
                if(MOT[MOutRotateKit].Tray.HasIC())
                {
                    iRetryCT=0;
                    Task=4000;
                }
                else
                {
                    Task=4400;
                }
            }
            else if(W906RKO_DUTNUM==tDutType_8)                                        //GATE 1 (banner A): golden `else if(tRotate.DutNum==tDutType_8)` //kevin 20130517 add 8 site
            {
                if(iInArmType==e9045_1x2_2_14 || iInArmType==e9045_2x2_4_14 || iInArmType==e9045_2x4_4_14)    //Steven 20201028 : for rotator
                {
                    if(TestIF.iTestMode==QualSite2X2 && DeviceForm.XDimension>=5000)            //wei 20151229  Xilinx 52.5x52.5 放一個Rotate  //Alick 20160105 (wei) modify IC>=50x50 放一個Rotate
                    {
                        if(MOT[MOutRotateKit].Tray.Data[1][0]!=NULL_IC)
                        {
                            Task=4000;
                        }
                    }
                    else
                    {
                        if(MOT[MOutRotateKit].Tray.Data[1][0]!=NULL_IC)
                        {
                            iKit=0;
                            Task=4000;
                        }
                        else if(MOT[MOutRotateKit].Tray.Data[2][1]!=NULL_IC)
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
                    if(MOT[MOutRotateKit].Tray.HasRealIC())
                    {
                        iRetryCT=0;
                        Task=4000;
                    }
                    else
                    {
                        Task=5000;
                    }
                }
            }
            else
            {
                if(MOT[MOutRotateKit].Tray.Data[0][0]!=NULL_IC ||
                   MOT[MOutRotateKit].Tray.Data[1][0]!=NULL_IC ||
                   MOT[MOutRotateKit].Tray.Data[0][1]!=NULL_IC ||
                   MOT[MOutRotateKit].Tray.Data[1][1]!=NULL_IC)
                {
                    iRetryCT=0;
                    Task=4000;
                }
                else
                {
                    // Task=5000; //jou 2014-03-18 Task=5000->4400; kit=0 關site不需要旋轉,kit=1沒有檢查就return true
                    Task=4400;
                    break;
                }
            }
            break;
        case 4000:
            if(M_MoveOutArmXY_ToRotateKIT(iKit)==true)
            {
                CheckOutArmSuckICFallDown();                                    //Steven 20110516 : 修改成整合式Alarm

                for(int i=0; i<MAX_ARM_Row; i++)
                {
                    for(int j=0; j<MAX_ARM_Col; j++)
                    {
                        OutArmSuck.Suck[i][j].SetNeedSuck(false);
                    }
                }
                Task=4100;
            }
            else
            {
                CheckOutArmSuckICFallDown();                                    //Steven 20110516 : 修改成整合式Alarm
                break;
            }
            // NOTE: faithful fall-through to case 4100 (golden :2566)
        case 4100:
            if(bOut_ICRotationCompleteOnKit==false)                             //Sam 20210623 : 修正 IC 轉完後被 Home，導致 IC 轉向異常。
            {
                DoRotate_Out(rCount, true);
                Task=2600;
                break;
            }

            if(M_MoveOutArmZ_ToRotateKIT_Pick(iKit, Prod.OutRotationCount[rCount-1])==true)  //kevin 20131003  更改OUTARM相對位置
            {
                if((AccessLevel>=1 || CosFunction.bOPCanPressStepAndTStart) &&  //JerryYang 20170417 (wei) OP權限也能按Step跟T.Start
                    Sen[SnRKManualStep].IsOn())
                {
                    bEnterOffset=false;
                    fMain->Pause("M_DoOutArmRotateKIT_Motor 4100");
                    Task=4500;
                    break;
                }

                OutArmSuck.ResetAll();                                          //Steven 20160323 : 避免未開啟真空
                Task=4200;
            }
            else
            {
                break;
            }
            // NOTE: faithful fall-through to case 4200 (golden :2592)
        case 4200:
            flag=true;

            if(iRotate_Type==e1MotRotate1Dut)                               //Frank 20180512 (Steven) : modify 1 Dut 1 Motor
            {
                for(int j=iKit; j<OutArmSuck.iMaxCol; j+=4)
                {
                    iTrayY=0;
                    iTrayX=0;
                    iSuckR=iRotato_Out_Row;
                    iSuckC=j;
                    iRealR=OutArmSuck.Suck[iSuckR][iSuckC].iMyRow;
                    iRealC=OutArmSuck.Suck[iSuckR][iSuckC].iMyCol;
                    if(MOT[MOutRotateKit].Tray.Data[iTrayX][iTrayY]==HAS_NULL_IC)
                    {
                        bSuckDuplicateErr[iSuckR][iSuckC]=false;
                        OutArmSuck.CopyFromTray(iSuckR, iSuckC, MOT[MOutRotateKit].Tray.Data[iTrayX][iTrayY], MOT[MOutRotateKit], iTrayY, iTrayX, NULL_IC);
                    }
                    else if(MOT[MOutRotateKit].Tray.Data[iTrayX][iTrayY]!=NULL_IC &&
                            CheckOutRotateAnglePostion(iRealR, iRealC, iTrayX, iTrayY)==true)
                    {
                        if(OutArmSuck.Suck[iSuckR][iSuckC].Suck())
                        {
                            bSuckDuplicateErr[iSuckR][iSuckC]=false;
                            OutArmSuck.CopyFromTray(iSuckR, iSuckC, MOT[MOutRotateKit].Tray.Data[iTrayX][iTrayY], MOT[MOutRotateKit], iTrayY, iTrayX, NULL_IC);
                        }
                        else if(OutArmSuck.Suck[iSuckR][iSuckC].Error==false)
                        {
                            flag=false;
                        }
                    }
                }
            }
            else if(iRotate_Type==e2MotRotate2Dut)                              //wei 20190515 modify 2 Dut 2 Motor
            {
                //AI(W906-RKOUT) 20260807: E-8 -- golden :2628/:2658 read InArmSuck.iPickCol
                //  here (the OUT-side place code at golden :954/:974 reads OutArmSuck.iPickCol).
                //  Golden's asymmetry, kept verbatim.
                if(InArmSuck.iPickCol==1)
                {
                    for(int j=iKit; j<OutArmSuck.iMaxCol; j+=4)
                    {
                        iTrayY=0;
                        iTrayX=0;
                        iSuckR=iRotato_Out_Row;
                        iSuckC=j;
                        iRealR=OutArmSuck.Suck[iSuckR][iSuckC].iMyRow;
                        iRealC=OutArmSuck.Suck[iSuckR][iSuckC].iMyCol;
                        if(MOT[MOutRotateKit].Tray.Data[iTrayX][iTrayY]==HAS_NULL_IC)
                        {
                            bSuckDuplicateErr[iSuckR][iSuckC]=false;
                            OutArmSuck.CopyFromTray(iSuckR, iSuckC, MOT[MOutRotateKit].Tray.Data[iTrayX][iTrayY], MOT[MOutRotateKit], iTrayY, iTrayX, NULL_IC);
                        }
                        else if(MOT[MOutRotateKit].Tray.Data[iTrayX][iTrayY]!=NULL_IC &&
                                CheckOutRotateAnglePostion(iRealR, iRealC, iTrayX, iTrayY)==true)
                        {
                            if(OutArmSuck.Suck[iSuckR][iSuckC].Suck())
                            {
                                bSuckDuplicateErr[iSuckR][iSuckC]=false;
                                OutArmSuck.CopyFromTray(iSuckR, iSuckC, MOT[MOutRotateKit].Tray.Data[iTrayX][iTrayY], MOT[MOutRotateKit], iTrayY, iTrayX, NULL_IC);
                            }
                            else if(OutArmSuck.Suck[iSuckR][iSuckC].Error==false)
                            {
                                flag=false;
                            }
                        }
                    }
                }
                else if(InArmSuck.iPickCol==2)
                {
                    for(int j=0; j<2; j++)
                    {
                        iTrayY=0;
                        iTrayX=j;
                        iSuckR=iRotato_Out_Row;
                        iSuckC=j;
                        iRealR=OutArmSuck.Suck[iSuckR][iSuckC].iMyRow;
                        iRealC=OutArmSuck.Suck[iSuckR][iSuckC].iMyCol;
                        if(MOT[MOutRotateKit].Tray.Data[iTrayX][iTrayY]==HAS_NULL_IC)
                        {
                            bSuckDuplicateErr[iSuckR][iSuckC]=false;
                            OutArmSuck.CopyFromTray(iSuckR, iSuckC, MOT[MOutRotateKit].Tray.Data[iTrayX][iTrayY], MOT[MOutRotateKit], iTrayY, iTrayX, NULL_IC);
                        }
                        else if(MOT[MOutRotateKit].Tray.Data[iTrayX][iTrayY]!=NULL_IC &&
                                CheckOutRotateAnglePostion(iRealR, iRealC, iTrayX, iTrayY)==true)
                        {
                            if(OutArmSuck.Suck[iSuckR][iSuckC].Suck())
                            {
                                bSuckDuplicateErr[iSuckR][iSuckC]=false;
                                OutArmSuck.CopyFromTray(iSuckR, iSuckC, MOT[MOutRotateKit].Tray.Data[iTrayX][iTrayY], MOT[MOutRotateKit], iTrayY, iTrayX, NULL_IC);
                            }
                            else if(OutArmSuck.Suck[iSuckR][iSuckC].Error==false)
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
                        iSuckR=iRotato_Out_Row;
                        iSuckC=j*2+iKit;
                        iRealR=OutArmSuck.Suck[iSuckR][iSuckC].iMyRow;
                        iRealC=OutArmSuck.Suck[iSuckR][iSuckC].iMyCol;
                        if(MOT[MOutRotateKit].Tray.Data[iTrayX][iTrayY]==HAS_NULL_IC)
                        {
                            bSuckDuplicateErr[iSuckR][iSuckC]=false;
                            OutArmSuck.CopyFromTray(iSuckR, iSuckC, MOT[MOutRotateKit].Tray.Data[iTrayX][iTrayY], MOT[MOutRotateKit], iTrayY, iTrayX, NULL_IC);
                        }
                        else if(MOT[MOutRotateKit].Tray.Data[iTrayX][iTrayY]!=NULL_IC &&
                                CheckOutRotateAnglePostion(iRealR, iRealC, iTrayX, iTrayY)==true)
                        {
                            if(OutArmSuck.Suck[iSuckR][iSuckC].Suck())
                            {
                                bSuckDuplicateErr[iSuckR][iSuckC]=false;
                                OutArmSuck.CopyFromTray(iSuckR, iSuckC, MOT[MOutRotateKit].Tray.Data[iTrayX][iTrayY], MOT[MOutRotateKit], iTrayY, iTrayX, NULL_IC);
                            }
                            else if(OutArmSuck.Suck[iSuckR][iSuckC].Error==false)
                            {
                                flag=false;
                            }
                        }
                    }
                }
            }
            else if(W906RKO_DUTNUM==tDutType_8)                                        //GATE 1 (banner A): golden `else if(tRotate.DutNum==tDutType_8)` //Steven 20160615 : fixed for 2x4_4 wirh rotate
            {
                if((TestIF.iTestMode==QualSite1X4 && TestIF.iUseSuckMode==4) || TestIF.iTestMode==_8Site1X4)  //ChungHung 20150528 add for 海思 _8Site1x4
                {                                                               //jou 2015-10-28 新增 1x4 8吸嘴 Rotate 模式
                    for(int j=0; j<iMAX_ARM_Col; j++)
                    {
                        //Steven 20170504 (wei) : 修改Rotate資料交換方式
                        //==>
                        iSuckR=0;
                        if(W906RKO_DUTNUM==tDutType_8)                                 //GATE 1 (banner A): golden `if(tRotate.DutNum==tDutType_8)` //jou 2015-10-14 fix 1x4 rotate dut8 hang up
                            iSuckC=j;
                        else
                            iSuckC=j*i2x2Suck_Out+iKit;

                        iTrayX=j;
                        iTrayY=iKit;

                        if(MOT[MOutRotateKit].Tray.Data[iTrayX][iTrayY]==HAS_NULL_IC)
                        {
                            bSuckDuplicateErr[iSuckR][iSuckC]=false;
                            OutArmSuck.CopyFromTray(iSuckR, iSuckC, MOT[MOutRotateKit].Tray.Data[iTrayX][iTrayY], MOT[MOutRotateKit], iTrayY, iTrayX, NULL_IC);
                        }
                        else if(MOT[MOutRotateKit].Tray.Data[iTrayX][iTrayY]!=NULL_IC)
                        {
                            if(OutArmSuck.Suck[iSuckR][iSuckC].Suck())
                            {
                                bSuckDuplicateErr[iSuckR][iSuckC]=false;
                                OutArmSuck.CopyFromTray(iSuckR, iSuckC, MOT[MOutRotateKit].Tray.Data[iTrayX][iTrayY], MOT[MOutRotateKit], iTrayY, iTrayX, NULL_IC);
                            }
                            else if(OutArmSuck.Suck[iSuckR][iSuckC].Error==false)
                            {
                                flag=false;
                            }
                        }
                        //<==
                        //Steven 20170504 (wei) : 修改Rotate資料交換方式
                    }
                }
                else if(iInArmType==e9045_1x2_2_14 || iInArmType==e9045_2x2_4_14 || iInArmType==e9045_2x4_4_14)    //Steven 20201028 : for rotator
                {
                    for(int j=iKit; j<MAX_ARM_Col; j+=4)
                    {
                        //Steven 20170504 (wei) : 修改Rotate資料交換方式
                        //==>
                        iSuckR=iRotato_Out_Row;
                        iSuckC=j*2;

                        if(TestIF.iTestMode==QualSite2X2 && DeviceForm.XDimension>=5000)
                        {   //wei 20151229  Xilinx 52.5x52.5 放一個Rotate  //Alick 20160105 (wei) modify IC>=50x50 放一個Rotate
                            iTrayX=1;
                            iTrayY=0;
                        }
                        else
                        {
                            iTrayX=j+1;
                            iTrayY=iKit;
                        }

                        if(MOT[MOutRotateKit].Tray.Data[iTrayX][iTrayY]==HAS_NULL_IC)
                        {
                            bSuckDuplicateErr[iSuckR][iSuckC]=false;
                            OutArmSuck.CopyFromTray(iSuckR, iSuckC, MOT[MOutRotateKit].Tray.Data[iTrayX][iTrayY], MOT[MOutRotateKit], iTrayY, iTrayX, NULL_IC);
                        }
                        else if(MOT[MOutRotateKit].Tray.Data[iTrayX][iTrayY]!=NULL_IC &&
                                CheckRotateOutAnglePostion(iTrayY, iTrayX, rCount-1, 0)==true)        //Ifor 20170412 (Steven) Modify 角度已算過不需在減掉原本角度
                        {
                            if(OutArmSuck.Suck[iSuckR][iSuckC].Suck())
                            {
                                bSuckDuplicateErr[iSuckR][iSuckC]=false;
                                OutArmSuck.CopyFromTray(iSuckR, iSuckC, MOT[MOutRotateKit].Tray.Data[iTrayX][iTrayY], MOT[MOutRotateKit], iTrayY, iTrayX, NULL_IC);
                            }
                            else if(OutArmSuck.Suck[iSuckR][iSuckC].Error==false)
                            {
                                flag=false;
                            }
                        }
                        //<==
                        //Steven 20170504 (wei) : 修改Rotate資料交換方式
                    }
                }
                else
                {
                    for(int i=0; i<iMAX_ARM_Row; i++)
                    {
                        for(int j=0; j<iMAX_ARM_Col; j++)
                        {
                            //Steven 20170504 (wei) : 修改Rotate資料交換方式
                            //==>
                            iSuckR=i;
                            iSuckC=j;

                            iTrayX=j;
                            iTrayY=i;

                            if(MOT[MOutRotateKit].Tray.Data[iTrayX][iTrayY]==HAS_NULL_IC)
                            {
                                bSuckDuplicateErr[iSuckR][iSuckC]=false;
                                OutArmSuck.CopyFromTray(iSuckR, iSuckC, MOT[MOutRotateKit].Tray.Data[iTrayX][iTrayY], MOT[MOutRotateKit], iTrayY, iTrayX, NULL_IC);
                            }
                            else if(MOT[MOutRotateKit].Tray.Data[iTrayX][iTrayY]!=NULL_IC &&
                                    CheckRotateOutAnglePostion(iTrayY, iTrayX, rCount-1, 0)==true)        //Ifor 20170412 (Steven) Modify 角度已算過不需在減掉原本角度
                            {
                                if(OutArmSuck.Suck[iSuckR][iSuckC].Suck())
                                {
                                    bSuckDuplicateErr[iSuckR][iSuckC]=false;
                                    OutArmSuck.CopyFromTray(iSuckR, iSuckC, MOT[MOutRotateKit].Tray.Data[iTrayX][iTrayY], MOT[MOutRotateKit], iTrayY, iTrayX, NULL_IC);
                                }
                                else if(OutArmSuck.Suck[iSuckR][iSuckC].Error==false)
                                {
                                    flag=false;
                                }
                            }
                            //<==
                            //Steven 20170504 (wei) : 修改Rotate資料交換方式
                        }
                    }
                }
            }
            else
            {
                //jou 2015-10-28 新增 1x4 8吸嘴 Rotate 模式
                if((TestIF.iTestMode == QualSite1X4 && TestIF.iUseSuckMode==4) || TestIF.iTestMode==_8Site1X4)  //ChungHung 20150528 add for 海思 _8Site1x4
                {
                    for(int j=0; j<iMAX_ARM_Col; j++)
                    {
                        //Steven 20170504 (wei) : 修改Rotate資料交換方式
                        //==>
                        iSuckR=0;
                        iSuckC=j*i2x2Suck_Out+iKit;

                        iTrayX=j;
                        iTrayY=iKit;
                        if(MOT[MOutRotateKit].Tray.Data[iTrayX][iTrayY]==HAS_NULL_IC)
                        {
                            bSuckDuplicateErr[iSuckR][iSuckC]=false;
                            OutArmSuck.CopyFromTray(iSuckR, iSuckC, MOT[MOutRotateKit].Tray.Data[iTrayX][iTrayY], MOT[MOutRotateKit], iTrayY, iTrayX, NULL_IC);
                        }
                        else if(MOT[MOutRotateKit].Tray.Data[iTrayX][iTrayY]!=NULL_IC &&
                                CheckRotateOutAnglePostion(iTrayY, iTrayX, rCount-1, 0)==true)        //Ifor 20170412 (Steven) Modify 角度已算過不需在減掉原本角度
                        {
                            if(OutArmSuck.Suck[iSuckR][iSuckC].Suck())
                            {
                                bSuckDuplicateErr[iSuckR][iSuckC]=false;
                                OutArmSuck.CopyFromTray(iSuckR, iSuckC, MOT[MOutRotateKit].Tray.Data[iTrayX][iTrayY], MOT[MOutRotateKit], iTrayY, iTrayX, NULL_IC);
                            }
                            else if(OutArmSuck.Suck[iSuckR][iSuckC].Error==false)
                            {
                                flag=false;
                            }
                        }
                        //<==
                        //Steven 20170504 (wei) : 修改Rotate資料交換方式
                    }
                }
                else
                {
                    //Steven 20170329 (wei) : Add individual rotate motor
                    //==>
                    if(USE_ROTATE_KIT==1 && (iRotate_Type==e4MotRotate || iRotate_Type==e8MotRotate || iRotate_Type==e2MotRotate2Dut))  //wei 20190515 modify 2 Dut 2 Motor
                    {
                        for(int i=0; i<2; i++)
                        {
                            for(int j=0; j<2; j++)
                            {
                                iSuckR=i;
                                iSuckC=j*i2x2Suck_Out+iKit;

                                iTrayX=j;
                                iTrayY=i;
                                if(iRotate_Type==e2MotRotate2Dut && TestIF.iTestMode==SingleSite)   //wei 20190515 modify 2 Dut 2 Motor
                                {
                                    iSuckR=0;
                                    //iSuckC=3;
                                    iSuckC=0;   //Sam 20250618 : 修正 Single Rotate 錯誤
                                    iTrayX=1;
                                    iTrayY=0;
                                }

                                if(MOT[MOutRotateKit].Tray.Data[iTrayX][iTrayY]==HAS_NULL_IC)
                                {
                                    bSuckDuplicateErr[iSuckR][iSuckC]=false;
                                    OutArmSuck.CopyFromTray(iSuckR, iSuckC, MOT[MOutRotateKit].Tray.Data[iTrayX][iTrayY], MOT[MOutRotateKit], iTrayY, iTrayX, NULL_IC);
                                }
                                else if(MOT[MOutRotateKit].Tray.Data[iTrayX][iTrayY]!=NULL_IC &&
                                        CheckRotateOutAnglePostion(iTrayY, iTrayX, rCount-1, 0)==true)        //Ifor 20170412 (Steven) Modify 角度已算過不需在減掉原本角度
                                {
                                    if(OutArmSuck.Suck[iSuckR][iSuckC].Suck())
                                    {
                                        bSuckDuplicateErr[iSuckR][iSuckC]=false;
                                        OutArmSuck.CopyFromTray(iSuckR, iSuckC, MOT[MOutRotateKit].Tray.Data[iTrayX][iTrayY], MOT[MOutRotateKit], iTrayY, iTrayX, NULL_IC);
                                    }
                                    else if(OutArmSuck.Suck[iSuckR][iSuckC].Error==false)
                                    {
                                        flag=false;
                                    }
                                }
                            }
                        }
                    }
                    //<==
                    //Steven 20170329 (wei) : Add individual rotate motor
                    else
                    {
                        for(int i=0; i<iMAX_ARM_Row; i++)
                        {
                            for(int j=0; j<iMAX_ARM_Col; j++)
                            {
                                //Ifor 20180110 (Steven) : add 僅有一個Rotate sensor
                              /*  if(bSingleDut || TestIF.iTestMode==SingleSite)  //Ifor 20201028 add: 避免Single Site 吸Rotate位置異常
                                {
                                    iSuckR=0;
                                    //iSuckC=3;
                                    iSuckC=0;   //Sam 20250618 : 修正 Single Rotate 錯誤
                                    iTrayX=0;
                                    iTrayY=0;
                                    if(i!=0 || j!=0)                            //Steven 20190809 : Fixed for hang up
                                        continue;
                                }
                                else     */
                                {
                                    //Steven 20170504 : 修改Rotate資料交換方式
                                    //<==
                                    iSuckR=i;
                                    iSuckC=j*i2x2Suck_Out+iKit;

                                    iTrayX=j;
                                    iTrayY=i;
                                }

                                if(MOT[MOutRotateKit].Tray.Data[iTrayX][iTrayY]==HAS_NULL_IC)
                                {
                                    bSuckDuplicateErr[iSuckR][iSuckC]=false;
                                    OutArmSuck.CopyFromTray(iSuckR, iSuckC, MOT[MOutRotateKit].Tray.Data[iTrayX][iTrayY], MOT[MOutRotateKit], iTrayY, iTrayX, NULL_IC);
                                }
                                else if(MOT[MOutRotateKit].Tray.Data[iTrayX][iTrayY]!=NULL_IC &&
                                        CheckRotateOutAnglePostion(iTrayY, iTrayX, rCount-1, 0)==true)        //Ifor 20170412 (Steven) Modify 角度已算過不需在減掉原本角度
                                {
                                    if(OutArmSuck.Suck[iSuckR][iSuckC].Suck())
                                    {
                                        bSuckDuplicateErr[iSuckR][iSuckC]=false;
                                        OutArmSuck.CopyFromTray(iSuckR, iSuckC, MOT[MOutRotateKit].Tray.Data[iTrayX][iTrayY], MOT[MOutRotateKit], iTrayY, iTrayX, NULL_IC);
                                    }
                                    else if(OutArmSuck.Suck[iSuckR][iSuckC].Error==false)
                                    {
                                        flag=false;
                                    }
                                }
                                //<==
                                //Steven 20170504 : 修改Rotate資料交換方式
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
                    if(OutArmSuck.Suck[i][j].Error)
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
            if(MoveOutArmToAutoSafe())
            {
                if(W906RKO_SHOWROTATEBYSITE==true)                            //GATE 6 (banner A): golden `if(FrmRotate->bShowRotateBySite==true)` //wei 20190515 modify 2 Dut 2 Motor
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
                if(MOT[MOutRotateKit].Tray.Data[0][0]==NULL_IC)
                    Task=5000;
                else
                    Task=2600;
            }
            else if(iRotate_Type==e2MotRotate2Dut)                              //wei 20190515 modify 2 Dut 2 Motor
            {
                if(MOT[MOutRotateKit].Tray.Data[0][0]==NULL_IC &&
                   MOT[MOutRotateKit].Tray.Data[1][0]==NULL_IC)
                    Task=5000;
                else
                    Task=2600;
            }
            else if(iInArmType==e9045_1x2_2_14 || iInArmType==e9045_2x2_4_14 || iInArmType==e9045_2x4_4_14)    //Steven 20201028 : for rotator
            {
                if(W906RKO_DUTNUM==tDutType_8)                                         //GATE 1 (banner A): golden `if(tRotate.DutNum==tDutType_8)`
                {
                    if(MOT[MOutRotateKit].Tray.Data[1][0]==NULL_IC && MOT[MOutRotateKit].Tray.Data[2][1]==NULL_IC)
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
                    if(MOT[MOutRotateKit].Tray.Data[1][0]==NULL_IC && MOT[MOutRotateKit].Tray.Data[1][1]==NULL_IC)
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
                if((TestIF.iTestMode==_8Site2X4 || TestIF.iTestMode==_16Site4X4 || TestIF.iTestMode==_16Site2X8 || (TestIF.iTestMode==QualSite2X2 && TestIF.iUseSuckMode==8)) && W906RKO_ARMCANSUCK4IC(0, true)==true)  //Sam 20190226 : 16Site4X4 //JerryYang 20170606 (Steven) 獨立rotate 支援2x2 8吸嘴模式
                {
                    if(W906RKO_DUTNUM==tDutType_4)                                     //GATE 1 (banner A): golden `if(tRotate.DutNum==tDutType_4)` //Steven 20140207 : 修正Rotate吸八顆放四顆
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
            Task=6000;
            // NOTE: faithful fall-through to case 6000 (no break in golden :3075)
        case 6000:
            if(iRotate_Type==e1MotRotate1Dut)                                   //Frank 20180512 (Steven) : modify 1 Dut 1 Motor
            {
                Task=GotOutRotateKitForSingle(iKit, iOutCount, false);
                if(Task==1)
                    return true;
            }
            else if(iRotate_Type==e2MotRotate2Dut)                              //wei 20190515 modify 2 Dut 2 Motor
            {
                if(InArmSuck.iPickCol==1)                                       //AI(W906-RKOUT) 20260807: E-8 -- golden :3085 reads InArmSuck here; kept
                {
                    Task=GotOutRotateKitForSingle(iKit, iOutCount, false);
                }
                else if(bUseAxxGPicker() ||
                        iCloseSiteModeFor1x4==e1x4CloseAbAc)                    //Steven 20241111 : for 1x4 close 2 site
                {
                    Task=GotOutRotateKitForAxxG(iKit, iOutCount, false);
                }
                else
                {
                    Task=GotOutRotateKitForAxEx_2RotMot(iKit, iOutCount, false);
                }

                if(Task==1)
                    return true;
            }
            else if(W906RKO_DUTNUM==tDutType_4)                                        //GATE 1 (banner A): golden `else if(tRotate.DutNum==tDutType_4)`
            {
                iKit=0;
                MOT[MOutRotateKit].SetHasNullIcToNullIc();                      //Frank 20170822 (wei): 清空Rotate資料
                return true;
            }
            else if(iInArmType==e9045_1x2_2_14 || iInArmType==e9045_2x2_4_14 || iInArmType==e9045_2x4_4_14)    //Steven 20201028 : for rotator
            {
                if(TestIF.iTestMode==QualSite2X2 && DeviceForm.XDimension>=5000)     //wei 20151229  Xilinx 52.5x52.5 放一個Rotate  //Alick 20160105 (wei) modify IC>=50x50 放一個Rotate
                {
                    if(iRotato_Out_Row==0)
                    {
                        if(iOutCount==0)
                        {
                            iKit=1;
                            iOutCount=1;
                        }
                        else
                        {
                            iOutCount=0;
                            iRotato_Out_Row=1;
                            iKit=0;
                        }
                        Task=10;
                    }
                    else
                    {
                        if(iOutCount==0)
                        {
                            iKit=1;
                            iOutCount=1;
                            Task=10;
                        }
                        else
                        {
                            iOutCount=0;
                            iRotato_Out_Row=0;
                            iKit=0;
                            MOT[MOutRotateKit].SetHasNullIcToNullIc();          //Frank 20170822 (wei): 清空Rotate資料
                            return true;
                        }
                    }
                }
                else
                {
                    if(iRotato_Out_Row==0)
                    {
                        iRotato_Out_Row=1;
                        iKit=0;
                        Task=10;
                    }
                    else
                    {
                        iRotato_Out_Row=0;
                        iKit=0;
                        MOT[MOutRotateKit].SetHasNullIcToNullIc();              //Frank 20170822 (wei): 清空Rotate資料
                        return true;
                    }
                }
            }
            else
            {
                MOT[MOutRotateKit].SetHasNullIcToNullIc();                      //Frank 20170822 (wei): 清空Rotate資料
                return true;
            }
            iKit=0;
            break;
        case 4500:                                                              //(1, 3)調整Offset
            if(MoveOutArmToAutoSafe())
            {
                bEnterOffset=false;
                Task=4000;
            }
            break;
        case 4600:
            if(MoveOutArmToAutoSafe())
            {
                if(iRetryCT>ArmSpeed[OutArm].iRetryCT)                    //Sam 20250206 : Rotate pick error retry for setting
                {
                    ErrPart=" ";
                    for(int i=0; i<MAX_ARM_Row; i++)
                    {
                        for(int j=0; j<MAX_ARM_Col; j++)
                        {
                            if(bSuckDuplicateErr[i][j])
                                bHasDuplicateErr=true;
                            if(OutArmSuck.Suck[i][j].Error)
                            {
                                ErrPart+=OutArmSuck.Suck[i][j].sName;
                                bSuckDuplicateErr[i][j]=true;
                                OutArmSuck.Suck[i][j].Error=false;
                            }
                            else
                            {
                                bSuckDuplicateErr[i][j]=false;
                            }
                        }
                    }
                    ret=ShowErrorMessage("JAM0210", K_RETRY|K_SKIP, MOutArmX, bHasDuplicateErr, ErrPart);     //Sam 20250204 : Add Skip
                }
                else
                {
                    ret=K_RETRY;
                }

                if(ret==K_SKIP)
                {
                    if(iRotate_Type==e1MotRotate1Dut)                                   //Frank 20180512 (Steven) : modify 1 Dut 1 Motor
                    {
                        for(int j=iKit; j<OutArmSuck.iMaxCol; j+=4)
                        {
                            iTrayY=0;
                            iTrayX=0;
                            iSuckR=iRotato_Out_Row;
                            iSuckC=j;
                            if(bSuckDuplicateErr[iSuckR][iSuckC])
                            {
                                OutArmSuck.PordRec[iSuckR][iSuckC].AddErrorRecord("JAM0210");
                                MOT[MOutRotateKit].SetTraySingleData(iTrayX, iTrayY, NULL_IC);
                                bSuckDuplicateErr[iSuckR][iSuckC]=false;
                                OutArmSuck.SetItemData(iSuckR, iSuckC, HAS_NULL_IC);
                            }
                        }
                    }
                    else if(iRotate_Type==e2MotRotate2Dut)                              //wei 20190515 modify 2 Dut 2 Motor
                    {
                        if(OutArmSuck.iPickCol==1)
                        {
                            for(int j=iKit; j<OutArmSuck.iMaxCol; j+=4)
                            {
                                iTrayY=0;
                                iTrayX=0;
                                iSuckR=iRotato_Out_Row;
                                iSuckC=j;
                                if(bSuckDuplicateErr[iSuckR][iSuckC])
                                {
                                    OutArmSuck.PordRec[iSuckR][iSuckC].AddErrorRecord("JAM0210");
                                    MOT[MOutRotateKit].SetTraySingleData(iTrayX, iTrayY, NULL_IC);
                                    bSuckDuplicateErr[iSuckR][iSuckC]=false;
                                    OutArmSuck.SetItemData(iSuckR, iSuckC, HAS_NULL_IC);
                                }
                            }
                        }
                        else if(OutArmSuck.iPickCol==2)
                        {
                            for(int j=0; j<2; j++)
                            {
                                iTrayY=0;
                                iTrayX=0;
                                iSuckR=iRotato_Out_Row;
                                iSuckC=j;
                                if(bSuckDuplicateErr[iSuckR][iSuckC])
                                {
                                    OutArmSuck.PordRec[iSuckR][iSuckC].AddErrorRecord("JAM0210");
                                    MOT[MOutRotateKit].SetTraySingleData(iTrayX, iTrayY, NULL_IC);
                                    bSuckDuplicateErr[iSuckR][iSuckC]=false;
                                    OutArmSuck.SetItemData(iSuckR, iSuckC, HAS_NULL_IC);
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
                                    OutArmSuck.PordRec[iSuckR][iSuckC].AddErrorRecord("JAM0210");
                                    MOT[MOutRotateKit].SetTraySingleData(iTrayX, iTrayY, NULL_IC);
                                    bSuckDuplicateErr[iSuckR][iSuckC]=false;
                                    OutArmSuck.SetItemData(iSuckR, iSuckC, HAS_NULL_IC);
                                }
                            }
                        }
                    }
                    else if(iInArmType==e9045_1x2_2_14 || iInArmType==e9045_2x2_4_14 || iInArmType==e9045_2x4_4_14)     //Steven 20201028 : for rotator
                    {
                        for(int j=iKit; j<MAX_ARM_Col; j+=4)                    //jou 2015-07-03 修正 Rotate 2x2 Pitch>3750 Pick up error SKIP NG
                        {
                            int i=iRotato_Out_Row;
                            j2=j*2;
                            if(bSuckDuplicateErr[i][j2])
                            {
                                OutArmSuck.PordRec[i][j2].AddErrorRecord("JAM0210");     //Steven 20161214 : Add Jam Record
                                MOT[MOutRotateKit].SetTraySingleData(j+1, iKit, NULL_IC);    //kevin 20130524
                                bSuckDuplicateErr[i][j2]=false;                 //kevin 20130524
                                OutArmSuck.SetItemData(i, j2, HAS_NULL_IC);     //Steven 20161116 : NULL_IC --> HAS_NULL_IC
                            }
                        }
                    }
                    else
                    {
                        for(int i=0; i<iMAX_ARM_Row; i++)                       //kevin 20130517
                        {
                            for(int j=0; j<iMAX_ARM_Col; j++)                   //kevin 20130517
                            {
                                //AI(W906-RKOUT) 20260807: E-10 -- golden :3304 uses the IN-side
                                //  stride i2x2Suck here while every other OUT-side site uses
                                //  i2x2Suck_Out.  Golden's inconsistency, kept verbatim.
                                if(W906RKO_DUTNUM==tDutType_8)                         //GATE 1 (banner A): golden `if(tRotate.DutNum == tDutType_8)` //kevin 20130517 add 8 site
                                    j2=j;
                                else
                                    j2=j*i2x2Suck+iKit;

                                if(bSuckDuplicateErr[i][j2])                    //kevin 20140220
                                {
                                    OutArmSuck.PordRec[i][j2].AddErrorRecord("JAM0210");     //Steven 20161214 : Add Jam Record
                                    MOT[MOutRotateKit].SetTraySingleData(j, i, NULL_IC);    //kevin 20130524
                                    bSuckDuplicateErr[i][j2]=false;             //kevin 20130524
                                    OutArmSuck.SetItemData(i, j2, HAS_NULL_IC); //Steven 20161116 : NULL_IC --> HAS_NULL_IC
                                }
                            }
                        }
                    }

                    for(int i=0; i<MAX_ARM_Row; i++)
                    {
                        for(int j=0; j<MAX_ARM_Col; j++)
                        {
                            OutArmSuck.Suck[i][j].Error=false;
                            bSuckDuplicateErr[i][j]=false;
                        }
                    }
                    Task=4300;
                    break;
                }
                Task=4000;
                if(ret==K_RETRY && iRetryCT>ArmSpeed[OutArm].iRetryCT)   //Sam 20250206 : Rotate pick error retry for setting
                {
                    Task=4700;
                }
            }
            break;
        case 4700:
            SetOutArmHome();
            iRetryCT=0;
            Task =4000;
            break;
        // --------------------------------------
        case 4800:
            if(MoveOutArmXY_ToShuttleAlarmArea()==false)                        //如果要移開,而且還沒移到定位
            {
                return false;                                                   //先離開等In Arm
            }
            Task=4801;
            break;
        case 4801:                                                              //旋轉失敗
            ret=ShowErrorMessage("JAM0553", K_RETRY, MOutRotateKit);            //kevin 20130524
            if(ret==K_RETRY)
            {
                Task=4810;
            }
            break;
        case 4810:
            FrmRotate->bRotateOutHome=true;                                     //kevin 20130706
            FrmRotate->InitialOutRotateHome();                                  //Steven 20170329 (wei) : Add individual rotate motor
            Task=4820;
            break;
        case 4820:
            if(FrmRotate->bRotateOutHome==false)                                //kevin 20130706
            {
//AI(W906-RKOUT) 20260807: GATE 8 (banner A) -- second of the two SetOutRotateSpeed
//  call sites (golden :3363).  Same reason as golden :1421: forms/fRotate.h's
//  TFrmRotate stand-in does not declare SetOutRotateSpeed.  Only the CALL is
//  gated; the Task transition below stays ACTIVE.
#if 0 // TODO(W7-form): golden :3363 FrmRotate->SetOutRotateSpeed (not on the forms/fRotate.h stand-in)
                FrmRotate->SetOutRotateSpeed(ArmSpeed[OutRotate].iACDCBodySP, ArmSpeed[OutRotate].iBodySP);    //Steven 20170329 (wei) : Add individual rotate motor
#endif
                Task=2600;
            }
            break;
    }
    return false;
}
//---------------------------------------------------------------------------
bool M_DoShakeOutArmRotateKIT()                                                 //JerryYang 20160825 Rotate sensor偵測異常,要先試著旋轉三次再跳alarm
{
    int &iTask=iShakeOutArmRotateKit;
    int ret, iShakeLimitNum=3;

    if(USE_ROTATE_KIT==1 && iRotate_Type==e1MotRotate)
    {
        switch(iTask)
        {
            case 1:
                ret=SetMotorResolution(OutAngle45, OutAngle90, false);          //Rotate旋轉功能,反轉4.5度   //Ifor 20170629 (Steven) fix Out Rotate Err
                ret=ret-100;
                if(MOT[MOutRotateKit].MotorMove(ret))
                    iTask=10;
                break;
            case 10:
                ret=SetMotorResolution(OutAngle45, OutAngle90, false);          //Rotate旋轉功能,正轉4.5度    //Ifor 20170629 (Steven) fix Out Rotate Err
                ret=ret+100;
                if(MOT[MOutRotateKit].MotorMove(ret))
                {
                    iOutArmShakeCnt++;
                    if(iOutArmShakeCnt>=iShakeLimitNum)                         //完成旋轉三次
                    {
                        iOutArmShakeCnt=0;
                        iTask=20;
                    }
                    else
                    {
                        iTask=1;                                                //還沒轉完要繼續轉
                    }
                }
                break;
            case 20:
                ret=SetMotorResolution(OutAngle45, OutAngle90, false);          //回到原本的角度     //Ifor 20170629 (Steven) fix Out Rotate Err
                if(MOT[MOutRotateKit].MotorMove(ret))
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
                MoveOutRotateToDegreeAtSameTime(0, true);
                iTask=10;
                break;
            case 10:
                ret=MoveOutRotateToDegreeAtSameTime(0);
                if(ret)
                {
                    iOutArmShakeCnt++;
                    if(iOutArmShakeCnt>=iShakeLimitNum)                         //完成旋轉三次
                    {
                        iOutArmShakeCnt=0;
                        return true;
                    }
                    else
                    {
                        iTask=20;                                               //還沒轉完要繼續轉
                    }
                }
                break;
            case 20:
                MoveOutRotateToDegreeAtSameTime(0, true);
                iTask=30;
                // NOTE: faithful fall-through to case 30 (no break in golden :3437)
            case 30:
                ret=MoveOutRotateToDegreeAtSameTime(90);
                if(ret)
                    iTask=1;
                break;
        }
    }
    return false;
}
//---------------------------------------------------------------------------
int DoRotate_Out(int &rCount, bool bReset)
{
    int iRet=0;

    if(USE_ROTATE_KIT==1)
    {
        if(iRotate_Type==e1MotRotate ||
           iRotate_Type==eInOutArm1Motor)
        {
            iRet=M1_DoOutRotateMove(rCount, bReset);
        }
        else if(iRotate_Type==e1MotRotate1Dut)                                  //Frank 20180512 (Steven) : modify 1 Dut 1 Motor
        {
            iRet=M1_DoOutRotateMove1Mot(rCount, bReset);
        }
        else if(iRotate_Type==e2MotRotate2Dut)
        {
            iRet=M2_DoOutRotateMove(rCount, bReset);
        }
        else if(iRotate_Type==e4MotRotate)
        {
            iRet=M4_DoOutRotateMove(rCount, bReset);
        }
        else if(iRotate_Type==e8MotRotate)
        {
            iRet=M8_DoOutRotateMove(rCount, bReset);
        }
    }
    else
    {
        iRet=1;
    }

    return iRet;
}
//---------------------------------------------------------------------------
int M1_DoOutRotateMove(int &rCount, bool bReset)
{
    int iResult         =0;
    int iAngle          =0;
    int iPos            =0;
    int iBacklash       =0;
//    int iNowPos         =0;
    int iAOIAngle       =0;
//    static bool bDoHomeFlag=false;
//    int iCount          =0;
//    static bool QQ      =false;

    if(bReset)
    {
        rCount=Prod.RotationTimeOut;                                            //Ifor 20170412 (Steven) RotationTime ==> RotationTimeOut   //jou 20231122 : Rotate Use RT mode
        CheckOutRotateSensor(true);
//        QQ=false;
        return iResult;
    }

//    iCount=CheckOutRotateSensor();
//    if(iCount==-100)
//    {
//        QQ=true;
//    }

    //jou 20231122 : Rotate Use RT mode
    if(Prod.OutRotationCount[rCount-1]!=0)                                      //20140318    如果設為大於0才進入的話-90度會不會旋轉
    {
        iAngle=(int)Prod.OutRotationCount[rCount-1]/90;                         //Ifor 20170407 已換算好角度直接/90即可
    }
    //Ifor 20211220 add:旋轉角度270度時改反轉減少旋轉時間
    if(iAngle==3)
        iAngle=-1;
    else if(iAngle==-3)
        iAngle=1;

    //AI(W906-RKOUT) 20260807: GATE 9 (banner A).  golden :3521-3522 is
    //    if(USE_Scanner_AOI_Inspection==(int)eBtnAOI_TopBottomInstall)
    //        iAOIAngle=FrmAOI->ttbInsp->iRotateKitAngOffset_Out;
    //  forms/fAOI.h's TFrmAOI stand-in carries exactly one member (bSimulateTopBtm);
    //  golden's ttbInsp inspection sub-object has no home anywhere in this tree.
    //  Only the ASSIGNMENT is gated -- the `if` guard stays ACTIVE, so iAOIAngle
    //  keeps its initialiser 0, which is its value when the Top/Bottom AOI angle
    //  offset is not configured.  Braces added around golden's single statement
    //  purely so the gate has somewhere to live; no control-flow change.
    if(USE_Scanner_AOI_Inspection==(int)eBtnAOI_TopBottomInstall)
    {
#if 0 // TODO(W7-form): golden :3522 FrmAOI->ttbInsp->iRotateKitAngOffset_Out (no ttbInsp on the forms/fAOI.h stand-in)
        iAOIAngle=FrmAOI->ttbInsp->iRotateKitAngOffset_Out;
#endif
    }

    iPos=(OutAngle90*iAngle)+Prod.iOut_iRotateA+iAOIAngle;  //kevin 20130415
    iBacklash=MOT[MOutRotateKit].GetRotatorBacklash(iPos, false);               //RogerYang 20260113 : Rotator新增背隙補償
    iPos+=iBacklash;

    if(iRotate_Type==eInOutArm1Motor && iAngle!=0)                                           //Ifor 20251204 add:rotate Offset
    {
        iPos=iPos+W906RKO_TROT_IROTATEOFFSET(1);                                      //GATE 5 (banner A): golden `iPos=iPos+tRotate.iRotateOffset[1];`
    }

    if(MOT[MOutRotateKit].MotorMove(iPos))
    {
        int btest=MOT[MInRotateKit].iLastRotatorDirP;                           //RogerYang 20260113 : Rotator新增背隙補償
        (void)btest;                                                            //AI(W906-RKOUT) 20260807: E-7 -- golden :3535 declares btest, reads the IN rotate kit inside the OUT mover and never uses it.  Retained per house rule instead of deleted.
        if(iRotate_Type==eInOutArm1Motor)
            iResult=1;
        else
            iResult=CheckOutRotateSensor();
    }

//    if(iResult==1 && QQ==true)
//    {
//        iResult=-100;
//    }
    return iResult;
}
//---------------------------------------------------------------------------
int M1_DoOutRotateMove1Mot(int &rCount, bool bReset)
{
    int iResult =0;
    int iAngle  =0;
    int iPos    =0;

    if(bReset)
    {
        return iResult;
    }

    if(MOT[MOutRotateKit].Tray.Data[0][0]!=NULL_IC &&
       MOT[MOutRotateKit].Tray.Data[0][0]!=HAS_NULL_IC &&
       MOT[MOutRotateKit].Tray.iNeedRotAng[0][0]!=MOT[MOutRotateKit].Tray.iCurrRotAng[0][0])
        iAngle=MOT[MOutRotateKit].Tray.iNeedRotAng[0][0]/90;
    else
        iAngle=MOT[MOutRotateKit].Tray.iCurrRotAng[0][0]/90;

    iPos=(OutAngle90*iAngle)+Prod.iOut_iRotateA;
    rCount=iAngle;
    iResult=MOT[MOutRotate[0][0]].MotorMove(iPos);
    if(iResult!=0)
    {
        MOT[MOutRotateKit].Tray.iCurrRotAng[0][0]=MOT[MOutRotateKit].Tray.iNeedRotAng[0][0];
        iResult=CheckOutRotateSensor();                                         //Steven 20170504 (wei) : 針對入料旋轉站的Sensor檢查
    }
    return iResult;
}
//---------------------------------------------------------------------------
//V3.27C Steven 20170504 (wei) : For 2顆獨立旋轉馬達同時轉動
//---------------------------------------------------------------------------
int M2_DoOutRotateMove(int &rCount, bool bReset)
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
        return iResult;
    }

    //AI(W906-RKOUT) 20260807: E-8 -- golden :3601 steers the OUT rotate movers on
    //  InArmSuck.iPickCol.  Golden's own choice; kept verbatim.
    if(InArmSuck.iPickCol==1)
    {
        int i=0;
        int j=0;
        j2=2;

        if(bM8Result[i][j2]==false)
        {
            if(MOT[MOutRotateKit].Tray.Data[j][i]!=NULL_IC &&
               MOT[MOutRotateKit].Tray.Data[j][i]!=HAS_NULL_IC &&
               MOT[MOutRotateKit].Tray.iNeedRotAng[j][i]!=MOT[MOutRotateKit].Tray.iCurrRotAng[j][i])
                iAngle=MOT[MOutRotateKit].Tray.iNeedRotAng[j][i]/90;
            else
                iAngle=MOT[MOutRotateKit].Tray.iCurrRotAng[j][i]/90;

            iPos=(OutAngle90*iAngle)+Tech.iOutRotate[i][j2+1];
            bM8Result[i][j2]=MOT[MOutRotate[i][j2]].MotorMove(iPos);
            if(bM8Result[i][j2]==false)
            {
                iResult=0;
            }
            else
            {
                MOT[MOutRotateKit].Tray.iCurrRotAng[j][i]=MOT[MOutRotateKit].Tray.iNeedRotAng[j][i];
            }
        }
    }
    else
    {
        for(int i=0; i<1; i++)
        {
            for(int j=0; j<iCol; j++)
            {
                j2=j*2;

                if(bM8Result[i][j2]==false)
                {
                    if(MOT[MOutRotateKit].Tray.Data[j][i]!=NULL_IC &&
                       MOT[MOutRotateKit].Tray.Data[j][i]!=HAS_NULL_IC &&
                       MOT[MOutRotateKit].Tray.iNeedRotAng[j][i]!=MOT[MOutRotateKit].Tray.iCurrRotAng[j][i])
                        iAngle=MOT[MOutRotateKit].Tray.iNeedRotAng[j][i]/90;
                    else
                        iAngle=MOT[MOutRotateKit].Tray.iCurrRotAng[j][i]/90;

                    iPos=(OutAngle90*iAngle)+Tech.iOutRotate[i][j2+1];
                    bM8Result[i][j2]=MOT[MOutRotate[i][j2]].MotorMove(iPos);
                    if(bM8Result[i][j2]==false)
                    {
                        iResult=0;
                    }
                    else
                    {
                        MOT[MOutRotateKit].Tray.iCurrRotAng[j][i]=MOT[MOutRotateKit].Tray.iNeedRotAng[j][i];
                    }
                }
            }
        }
    }

    return iResult;
}
//---------------------------------------------------------------------------
//V3.27C Steven 20170504 (wei) : For 4顆獨立旋轉馬達同時轉動
//---------------------------------------------------------------------------
int M4_DoOutRotateMove(int &rCount, bool bReset)
{
    static bool bM8Result[2][4]={false};
    int j2;
    int iResult =1;
    int iAngle  =0;
    int iPos    =0;
//    static bool QQ      =false;
//    int iCount          =0;

    if(bReset)
    {
        for(int i=0; i<2; i++)
        {
            for(int j=0; j<4; j++)
            {
                bM8Result[i][j]=false;
            }
        }
        CheckOutRotateSensor(true);
//        QQ=false;
        return iResult;
    }

//    iCount=CheckOutRotateSensor();
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
                if(MOT[MOutRotateKit].Tray.Data[j][i]!=NULL_IC &&
                   MOT[MOutRotateKit].Tray.Data[j][i]!=HAS_NULL_IC &&
                   MOT[MOutRotateKit].Tray.iNeedRotAng[j][i]!=MOT[MOutRotateKit].Tray.iCurrRotAng[j][i])
                    iAngle=MOT[MOutRotateKit].Tray.iNeedRotAng[j][i]/90;
                else
                    iAngle=MOT[MOutRotateKit].Tray.iCurrRotAng[j][i]/90;

                iPos=(OutAngle90*iAngle)+Tech.iOutRotate[i][j2];

                bM8Result[i][j2]=MOT[MOutRotate[i][j2]].MotorMove(iPos);
                if(bM8Result[i][j2]==false)
                {
                    iResult=0;
                }
                else
                {
                    MOT[MOutRotateKit].Tray.iCurrRotAng[j][i]=MOT[MOutRotateKit].Tray.iNeedRotAng[j][i];
                }
            }
        }
    }

    if(iResult!=0)
    {
        iResult=CheckOutRotateSensor();                                         //Steven 20170504 (wei) : 針對入料旋轉站的Sensor檢查
    }

//    if(iResult==1 && QQ==true)
//    {
//        iResult=-100;
//    }

    return iResult;
}
//---------------------------------------------------------------------------
//V3.27C Steven 20170504 (wei) : For 8顆獨立旋轉馬達同時轉動
//---------------------------------------------------------------------------
int M8_DoOutRotateMove(int &rCount, bool bReset)
{
    static bool bM8Result[2][4]={false};
    int j2;
    int iResult =1;
    int iAngle  =0;
    int iPos    =0;
//    static bool QQ      =false;
//    int iCount          =0;

    if(bReset)
    {
        for(int i=0; i<2; i++)
        {
            for(int j=0; j<4; j++)
            {
                bM8Result[i][j]=false;
            }
        }
        CheckOutRotateSensor(true);
//        QQ=false;
        return iResult;
    }

//    iCount=CheckOutRotateSensor();
//    if(iCount==-100)
//    {
//        QQ=true;
//    }

    for(int i=0; i<2; i++)
    {
        for(int j=0; j<2; j++)
        {
            j2=j;
            if(bM8Result[i][j2]==false)
            {
                if(MOT[MOutRotateKit].Tray.Data[j][i]!=NULL_IC &&
                   MOT[MOutRotateKit].Tray.Data[j][i]!=HAS_NULL_IC &&
                   MOT[MOutRotateKit].Tray.iNeedRotAng[j][i]!=MOT[MOutRotateKit].Tray.iCurrRotAng[j][i])
                    iAngle=MOT[MOutRotateKit].Tray.iNeedRotAng[j][i]/90;
                else
                    iAngle=MOT[MOutRotateKit].Tray.iCurrRotAng[j][i]/90;

                iPos=(OutAngle90*iAngle)+Tech.iOutRotate[i][j2];

                bM8Result[i][j2]=MOT[MOutRotate[i][j2]].MotorMove(iPos);
                if(bM8Result[i][j2]==false)
                {
                    iResult=0;
                }
                else
                {
                    MOT[MOutRotateKit].Tray.iCurrRotAng[j][i]=MOT[MOutRotateKit].Tray.iNeedRotAng[j][i];
                }
            }
        }
    }

    if(iResult!=0)
    {
        iResult=CheckOutRotateSensor();  //Steven 20170504 (wei) : 針對入料旋轉站的Sensor檢查
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
bool MoveOutRotateToDegreeAtSameTime(int iDegree, bool bInitial)
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
        iRotate_Type==eInOutArm1Motor))                                         //add One sucker with rotate
    {
//        buffer=SetMotorResolution(OutAngle90, OutAngle90, true);
        buffer=SetMotorResolution(OutAngle90, OutAngle90, false);               //Ifor 20170629 (Steven) fix Out Rotate Err
        if(MOT[MOutRotateKit].MotorMove(buffer))                                //旋轉成和shuttle一樣方向
        {
            for(int i=0; i<2; i++)
            {
                for(int j=0; j<4; j++)
                {
                    MOT[MOutRotateKit].Tray.iCurrRotAng[j][i]=iDegree;
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
                    j2=j*2;
                    if(j>=2)
                    {
                        continue;
                    }
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
                    if(USE_ROTATE_KIT==1 && iRotate_Type==e1MotRotate)
                        buffer=Tech.iOutRotate[i][j2]+2000*iDegree;
                    else if(USE_ROTATE_KIT==1 && iRotate_Type==e2MotRotate2Dut)
                        buffer=Tech.iOutRotate[i][j2+1]+800*iDegree;
                    else
                        buffer=Tech.iOutRotate[i][j2]+800*iDegree;
                    if(MOT[MOutRotate[i][j2]].ReadPos()!=buffer)
                    {
                        bResult=false;
                        bMoveFinish[i][j]=MOT[MOutRotate[i][j2]].MotorMove(buffer);
                    }
                    else
                    {
                        bMoveFinish[i][j]=true;
                    }
                }

                if(bMoveFinish[i][j]==true)
                {
                    MOT[MOutRotateKit].Tray.iCurrRotAng[j][i]=iDegree;
                }
            }
        }
    }
    return bResult;
}
//---------------------------------------------------------------------------
bool CheckOutRotateAnglePostion(int iSuckR, int iSuckC, int iTrayX, int iTrayY)
{
    bool bResult=false;
    if(Prod.RotateDutDate[1][iSuckR][iSuckC]==MOT[MOutRotateKit].Tray.iCurrRotAng[iTrayX][iTrayY])
        bResult=true;

    return bResult;
}
//---------------------------------------------------------------------------
//Ifor 20170412 (Steven) add Check Angle 與各Site位置是否相同
//==>
bool CheckRotateOutAnglePostion(int iTrayY, int iTrayX, int iTemp, int iMode)
{
    bool bResult=false;
    int dbTempAngle=0;
    int iRealR, iRealC;

    if(W906RKO_SHOWROTATEBYSITE==true)                                        //GATE 6 (banner A): golden `if(FrmRotate->bShowRotateBySite==true)` //Steven 20170329 (wei) : Add individual rotate motor  //wei 20190515 modify 2 Dut 2 Motor
    {
        if(iRotate_Type==e1MotRotate1Dut)
        {
            iRealR=iTrayY;
            iRealC=OutArmSuck.Suck[iTrayY][iTrayX].iMyCol;
            if(Prod.RotateDutDate[1][iRealR][iRealC]==MOT[MOutRotateKit].Tray.iCurrRotAng[0][0])
                bResult=true;
        }
        else if(iRotate_Type==e2MotRotate2Dut && TestIF.iTestMode==SingleSite)
        {
            if(Prod.RotateDutDate[1][0][0]==MOT[MOutRotateKit].Tray.iCurrRotAng[1][0])
                bResult=true;
        }
        else
        {
            if(Prod.RotateDutDate[1][iTrayY][iTrayX]==MOT[MOutRotateKit].Tray.iCurrRotAng[iTrayX][iTrayY])
                bResult=true;
        }
    }
    else
    {
        if(W906RKO_TROT_USEDIFFERENTANGLE==true)                                   //GATE 4 (banner A): golden `if(tRotate.bUseDifferentAngle==true)`
        {
            dbTempAngle=360-Prod.RotateDutDate[0][iTrayY][iTrayX]+Prod.RotateDutDate[1][iTrayY][iTrayX]-360;
        }
        else
        {
            dbTempAngle=0-Prod.RotateDutDate[0][iTrayY][iTrayX];
        }

        if(iMode==0)
        {
            if(dbTempAngle==Prod.OutRotationCount[iTemp])
                bResult=true;
        }
        else
        {
            if(dbTempAngle==iTemp)
                bResult=true;
        }
    }

    return bResult;
}
//<==
//Ifor 20170412 (Steven) add Check Angle 與各Site位置是否相同
//---------------------------------------------------------------------------
int CheckOutRotateSensor(bool bReset)
{
    static int iCount=0;
    int iResult=0;

    if(bReset)
    {
        iCount=0;
        return iResult;
    }

    iCount++;
    if(Sen[SnRotateRowOut1].Enable==true &&
           Sen[SnRotateRowOut2].Enable==true &&
           Sen[SnRotateRowOut1].IsOn()       &&
           Sen[SnRotateRowOut2].IsOn()       )
    {
        iResult=1;
        iCount=0;
    }
    else if(Sen[SnRotateRowOut1].Enable==true    &&
            Sen[SnRotateRowOut2].Enable==false   &&
            Sen[SnRotateRowOut1].IsOn()          )
    {
        iResult=1;
        iCount=0;
    }
    else if(Sen[SnRotateRowOut1].Enable==false)
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

//        if(iCount>30)
//        {
//            iResult=-100;     //Alarm
//        }
    }
    return iResult;
}
//---------------------------------------------------------------------------
bool CheckRotateOutNotFinish()
{
    if(USE_ROTATE_KIT==1 && tRotate.ActiveRotate && TrayForm.iRotateKIT_InputType!=0 && (iOutRotateFinish==1)) //kevin 20130524 發生alarm inarm到等待點
    {
        return true;
    }
    else
    {
        return false;
    }
}
