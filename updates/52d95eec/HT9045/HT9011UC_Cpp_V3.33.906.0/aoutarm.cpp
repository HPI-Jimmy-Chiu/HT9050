// =============================================================================
//  aoutarm.cpp  --  Out-arm ENGINE: state-machine entry, bin routing, tray
//                   search / close-pitch geometry, pick-error alarm surface.
//
//  Faithful translation of golden aoutarm.cpp (3985 lines / 3619 code lines,
//  BCB6, Big5/cp950).  Golden body :36-:3985 is transcribed VERBATIM; only the
//  include head (:1-:35) is rewritten and the point edits in the GATE REGISTER
//  below are applied.
//  Translator: AI(W906-PT-W4-aoutarm) 20260808
//  Translation wave: W906-PT-W4 (last tier of genuinely-missing non-form units).
//
//  ROLE
//  ----
//  This is the CONFIG-INDEPENDENT half of the out-arm.  The 27 aoutarm9045_<cfg>
//  files already in this tree are the per-site-layout state machines; the engine
//  aoutarm9045.cpp is the dispatcher; THIS file is the shared library underneath
//  both.  It owns, in golden's own order:
//    * the file-scope SM cursors and flags the whole out-arm reads
//      (OutArmTask / iPickFromShuttle1/2Task / iPlaceToAuto/Fix/SortShtTask /
//      iOutArmInspectTask / iOutArmPickFromAuto1Task / iPlaceToOutRotateTask /
//      iFix3CanFullTask / the 13 Magazine + catch-tray cursors / bOverTray /
//      bUseOnebyOne / bPickShuttleError / bOutArmXOverLimit / bCarryControlOutarm1,2),
//    * the per-nozzle placement grids (bOutArmSuckActive / bOutArmSuckActiveBack /
//      iOutArmXPosition / iOutArmYPosition) and the close-pitch geometry
//      (dOutArmXPitch_1Step / dOutArmXPitch_MovePitch / iOutArmXStep / iOutArmYStep),
//    * 5 QPF software timers (HInspectWait / DoPickFromShuttle1Delay /
//      DoPickFromShuttle2Delay / DoPlaceToAutoDelay / DoSHTWaitTime),
//    * CheckBin -- the BIN -> target-tray router (golden's own comment at :600
//      says "不可以亂改" / do not casually change it),
//    * the two tray-search cores Find_OutArm_PickerMaxUseCountOnTime (multi-IC
//      at once) and Find_OutArm_Single (one IC at a time), plus GetTrayDirection,
//    * the X/Y close-pitch calculators AutoCalculateOutArmXClosePitch /
//      AutoCalculateOutArmYClosePitch and GetOutArmToUnLoaderPosition,
//    * the pick/drop alarm surface (CheckOutSuckICFallDown / OutArmPickShuttleAlarm
//      / PorcessJAM0201OutArmPickUpErrorSkip / SwapShuttleDataToOutArm), and
//    * class TMyQASample + its global QASam.
//
//  WAVE SCOPE -- 63 golden free functions + 3 TMyQASample methods, ALL ACTIVE
//  (no function is gated out whole; the only gates are the 17 point-gates in the
//  GATE REGISTER, all inside otherwise-live bodies):
//    CheckOutArmZ                                golden :95    ACTIVE
//    CheckTesterZ                                golden :119   ACTIVE
//    bOutShtwaitPick                             golden :137   ACTIVE
//    CheckShuttleICPos                           golden :147   ACTIVE
//    CheckOutArmCleanOut                         golden :217   ACTIVE (gates G1,G2,G16)
//    MoveOutArmToAutoSafe                        golden :300   ACTIVE
//    MoveOutArmXY_ToFix_Tray_Full                golden :311   ACTIVE (gates G7,G8)
//    MoveOutArmXY_ToShuttleAlarmArea             golden :386   ACTIVE (gate  G7)
//    MoveOutArm2XYToDecayTeach                   golden :431   ACTIVE (gate  G7)
//    MoveOutArmXY_ToShtMidArea                   golden :488   ACTIVE (gate  G7)
//    IsCatchTrayReadySupplyNewTray               golden :522   ACTIVE
//    CheckOutArmInitState                        golden :595   ACTIVE
//    CheckBin                                    golden :602   ACTIVE
//    InitPickFromShuttle1Task                    golden :743   ACTIVE
//    PickFromShuttle                             golden :751   ACTIVE
//    InitPickFromShuttle2Task                    golden :759   ACTIVE
//    InitPlaceToSortShtTask                      golden :767   ACTIVE
//    InitPlaceToAutoTask                         golden :774   ACTIVE
//    InitPlaceToFixTask                          golden :781   ACTIVE
//    InitOutArmInspectTask                       golden :786   ACTIVE
//    InitOutArmTask                              golden :793   ACTIVE (gate  G3)
//    InitOutArmPickFromAuto1Task                 golden :800   ACTIVE
//    DoOutArm_Scanner                            golden :805   ACTIVE (empty in golden)
//    SetFixTrayFullIC                            golden :809   ACTIVE
//    SetFixTrayMiddleDtata                       golden :842   ACTIVE (gate  G9)
//    OutArmAddSpeed                              golden :1126  ACTIVE (gate  G4)
//    OutArmSubSpeed                              golden :1155  ACTIVE (gate  G4)
//    initDoPlaceOutRotateTask                    golden :1169  ACTIVE
//    DoPlaceOutRotate                            golden :1174  ACTIVE (golden body = `return true;`)
//    NeedScan                                    golden :1179  ACTIVE (golden body = commented-out + `return false;`)
//    DoOutArm                                    golden :1195  ACTIVE (gate  G10)
//    SearchBinBoxUpDown                          golden :1215  ACTIVE
//    Find_OutArm_PickerBufferMaxUseCountOnTime   golden :1247  ACTIVE (golden body is ENTIRELY commented out + `return 0;` -- kept verbatim, see NOTE 1)
//    Find_OutArm_PickerMaxUseCountOnTime         golden :1270  ACTIVE
//    Find_OutArm_Single                          golden :1804  ACTIVE
//    AdjustOutArmClosePitchCondition             golden :2305  ACTIVE (gates G14,G11)
//    CheckOutSuckICFallDown                      golden :2369  ACTIVE (#ifdef SOFT_SIMULTE kept verbatim; SOFT_SIMULTE is NOT defined -> the real body compiles)
//    SetOutArmHome                               golden :2583  ACTIVE
//    ShowOutputShuttleDataMiss                   golden :2603  ACTIVE
//    CheckOutputShuttleDataMiss                  golden :2608  ACTIVE
//    IfUseOnebyOne                               golden :2621  ACTIVE
//    OutArmAddBlockPitch                         golden :2694  ACTIVE
//    SendDataToASE                               golden :2706  ACTIVE (gate  G12)
//    TransferAutoRatio                           golden :2712  ACTIVE (gate  G5)
//    TransferOutShuttleRatio                     golden :2758  ACTIVE (gate  G5)
//    OutArmPickShuttleAlarm                      golden :2818  ACTIVE
//    PorcessJAM0201OutArmPickUpErrorSkip         golden :2926  ACTIVE
//    UnloaderTrayYDirForArmYPitch                golden :2953  ACTIVE (gate  G6)
//    AutoCalculateOutArmYClosePitch              golden :2967  ACTIVE (gate  G6)
//    CheckOutArmSuckOnlyGOrH                     golden :3013  ACTIVE
//    AutoCalculateOutArmXClosePitch              golden :3037  ACTIVE
//    GetOutArmToUnLoaderPosition                 golden :3283  ACTIVE
//    CheckUseFixBinBoxFunction                   golden :3440  ACTIVE
//    CheckBinBoxIsFull                           golden :3447  ACTIVE
//    ErrorBinBoxDetect                           golden :3468  ACTIVE
//    SwapShuttleDataToOutArm                     golden :3485  ACTIVE (gates G13,G17)
//    OutArmAddSpeedDisplay                       golden :3566  ACTIVE
//    OutArmSubSpeedDisplay                       golden :3571  ACTIVE
//    TMyQASample::TMyQASample                    golden :3580  ACTIVE (adapt A2)
//    TMyQASample::ClearCount                     golden :3585  ACTIVE
//    TMyQASample::FindDeviceToQA                 golden :3593  ACTIVE (empty in golden)
//    CheckDuplicateBarCode                       golden :3597  ACTIVE (adapt A5; #ifdef SOFT_SIMULTE kept verbatim)
//    SetOutArmNeedDestory                        golden :3665  ACTIVE (adapt A3)
//    MoveOutArm2XYToShuttle2Wait                 golden :3679  ACTIVE
//    GetTrayDirection                            golden :3711  ACTIVE
//    CalOneByOneCount                            golden :3926  ACTIVE (gate  G6 via AutoCalculateOutArmYClosePitch)
//   Plus all 52 golden file-scope globals (counted, not estimated: 48 at golden
//   :38-:93 and 4 at golden :3032-:3035; the list is in this wave's report).
//
//  SATISFIED-BY-SUBSTRATE (already real elsewhere in this tree -- NOT redefined
//  here; each was checked to a concrete .cpp that is registered in CMakeLists.txt,
//  see the LINK-CLOSURE section of this wave's report):
//    MOT[] / TMyMotor (Motor/mymotor.h, Motor/mymotor.cpp), OutArmContinuousMove_9045
//    / GetOutArmPitchX_9045 / GetOutArmPitchY_9045 / MoveOutArmToAutoSafe_9045 /
//    CheckOutArmInitState_9045 / DoOutArm_9045 / DoOutArmAfterPlaceToAuto /
//    PreSetOutAdditionalFlag (aoutarm9045.h / aoutarm9045.cpp),
//    TMyKitSuck + TMySucker + TMyProductionRecord + OutArmSuck / InArmSuck /
//    F,BLCarryKit / F,BRCarryKit / F,BTestSuck (aHotPlateSubstrate.h/.cpp -- see
//    NOTE 2 for why this header and not mykitsuck.h),
//    ptrOutSHT (acarry.cpp:99), GetOutArmToShuttleOffset_9045 /
//    GetVariableYOutShuttleData / iOutArmZTeachTask (aoutarm_shims.cpp:42/43/33),
//    fSCKART (forms/fSCKART.cpp -- but see GATE G14: its class is NOT a SckArtState),
//    list2DByLot (BarCode/BarCode_Bottom2DID.cpp:45),
//    AutoTeachLoadTrayZ (ainarm9045_2x4_16_shims.h), WhichAutoNeedTray
//    (acatchtray.cpp), IsTrayArmMoveAvoidOutArmCrash (acatchtray.cpp),
//    ShowErrorMessage / ShowMyMessage / RecordProcess / NewRecordProcess /
//    MyDBIProcess / LastSet / K_RETRY / K_SKIP / K_HOME / DUMMY / REALLY
//    (canary_support.h), Prod / TestIF / TestIF_File / ArmSpeed / ArmSpeed_File /
//    AutoArmSpeed / AutoForm / LoadForm / TrayForm / DeviceForm / Tech /
//    Temperature / BinSelect / LotSummary / OutArmOffSet / tRotate (cprod.h),
//    CosFunction / IniConfig (cpublic.h / cmydef.h), Sen[] (mysensor.h),
//    Cylinder[] (mycylin.h), fNote / fBarCode / fAGV / fSCKART (FormsFacade.h),
//    ChangeToFloatNonPcnt / MAX_ARM_Row / MAX_ARM_Col / X_PITCH_COUNT /
//    CUSTOMER_CODE + every CC_* / USE_* / e* enum (MachineType.h, cmydef.h).
//
//  NOTE 1 -- Find_OutArm_PickerBufferMaxUseCountOnTime (golden :1247-1268) is a
//  DEAD FUNCTION IN GOLDEN ITSELF: its entire 19-line body is commented out and
//  it unconditionally `return 0;`.  That is golden's text and it is reproduced
//  verbatim, comments included.  It is NOT a gate this port introduced.
//
//  NOTE 2 -- WHY aHotPlateSubstrate.h AND NOT mykitsuck.h.  This tree has TWO
//  classes named TMyKitSuck with DIFFERENT layouts (mykitsuck.h:274 and
//  aHotPlateSubstrate.h:365) and 14 colliding global names.  CMakeLists.txt:2011
//  says in as many words "*** DELIBERATELY NOT REGISTERED IN THIS WAVE:
//  mykitsuck.cpp ***", so aHotPlateSubstrate.cpp is the only compiled home for
//  OutArmSuck / F,BRCarryKit / F,BTestSuck.  Verified for THIS file the way trap
//  5 demands -- not by "it compiles": `g++ -E aoutarm.cpp | grep -cE "^class
//  TMyKitSuck$"` == 1 (a second hit for `^class TMyKitSuck;$` is this file's own
//  forward declaration out of aoutarm.h gate (H1), not a second definition), and
//  the same command on acarry.cpp -- which defines ptrOutSHT, the one
//  TMyKitSuck* this file dereferences across a TU boundary -- also == 1, and in
//  BOTH cases the preprocessor line markers put that one definition at
//  aHotPlateSubstrate.h:365.  Both TUs therefore agree on the layout.
//
//  GATE REGISTER
//  =============
//  Two kinds of entry.  ADAPTATIONS (A*) change the spelling of a golden
//  expression but NOT its behaviour -- each names the port symbol that IS the
//  same object/body.  GATES (G*) default a behaviour because the golden symbol
//  has no compiled body anywhere in this tree.
//
//  Every G* entry uses one of two shapes, both of which retire by DELETION:
//   (a) `#if 0 <verbatim golden> #else <default> #endif` at the call site, or
//   (b) a file-local `static` stand-in with golden's EXACT signature, so the
//       golden call sites stay byte-identical and retirement = delete the static.
//       Shape (b) is this tree's own registered idiom for exactly this situation
//       -- see aoutarm9045.cpp:147-174 (a block of 12), csystem.cpp:1266-1270,
//       AutoClean/AutoClean.cpp:177-181.
//
//  ---- ADAPTATIONS (no behaviour delta) ----
//  (A1) aoutarm.h:5 `#include "mykitsuck.h"` -- gated in the HEADER; see the
//       aoutarm.h banner, gate (H1).  BEHAVIOUR DELTA: none.
//  (A2) golden :3580 `__fastcall TMyQASample::TMyQASample()` -> `TMyQASample::
//       TMyQASample()`, in lockstep with aoutarm.h:160 (this wave's contract
//       drops __fastcall).  Kept in lockstep deliberately: vclcompat/vcl_compat.h
//       :24-50 records that __fastcall is a REAL MinGW keyword here, so a decl/def
//       mismatch would be a silent ABI bug.  BEHAVIOUR DELTA: none -- the ctor is
//       only ever invoked by this file's own `TMyQASample QASam;` static-init and
//       no TU takes its address.
//  (A3) golden :3670 `OutArmSuck.Suck[r][c].SetNeedDestroy(true);` -- the substrate
//       TMySucker (aHotPlateSubstrate.h:148) landed the STORAGE `bNeedDestroy` and
//       the getter but not golden mykitsuck.h:122's one-line setter, and that
//       header is not this wave's file.  A function-like macro reproduces golden's
//       inline body EXACTLY: `#define SetNeedDestroy(v) bNeedDestroy = (v)`, so the
//       call expands to `...bNeedDestroy = (true);` -- the same write to the same
//       public member.  Identical precedent, same reasoning, in RotateKit/
//       aRotateKIT_In.cpp:189-204.  BEHAVIOUR DELTA: none.  Retire by adding the
//       real one-line setter to aHotPlateSubstrate.h.
//  (A4) -- RETIRED LABEL, deliberately left as a hole so the numbering in this
//       banner matches this wave's report.  It was going to be
//       `fSCKART->CheckOutArmNeedVariModeFIX` re-spelled as the real free
//       function; that turned out to be impossible (its state object does not
//       exist in this tree) and it became GATE G14 instead.  See G14.
//  (A5) golden :3610,:3611,:3626,:3632 `fBarCode->list2DByLot->...` ->
//       `list2DByLot->...`.  BarCode/BarCode_Bottom2DID.h:144-148 declares
//       `extern TStringList *list2DByLot;` and its own comment says it IS golden's
//       `fBarCode->list2DByLot`, "exposed here as a bare global instead" because
//       TfBarCode has no such member; the object is really allocated at
//       BarCode/BarCode_Bottom2DID.cpp:45 (`new TStringList()`), and that .cpp is
//       registered (CMakeLists.txt:1856).  Same object, same 4 operations
//       (Clear/LoadFromFile/Sort/Find), all present on vclcompat TStringList
//       (:166/:221/:198/:182).  BEHAVIOUR DELTA: none.
//
//  ---- GATES (behaviour defaulted) ----
//  (G1) golden :245 and :261 `InitialDoPickFromMagazineBuffer();` -- golden
//       Magazine.h:151; Magazine.cpp is NOT translated in this tree and the only
//       thing spelling this name is a file-LOCAL `static` no-op in aoutarm9045.cpp
//       :174, which is not link-visible here.  Shape (b), no-op.
//       WHY FAITHFUL: the function only re-arms the magazine-buffer pick cursor
//       (`iPickFromMagazineBufferTask=1`-class bookkeeping) for a Magazine unit
//       this tree cannot drive at all; both call sites are already inside
//       `AUTO3_IS_MAGAZINE==1` / `fAGV->IsATK_AMR()` branches that are false on a
//       non-Magazine, non-AMR machine.  BEHAVIOUR DELTA ON A REAL MACHINE with a
//       Magazine: CheckOutArmCleanOut still returns 11100 (it does -- golden :247
//       and :263 are LIVE), so the caller still enters the magazine-buffer
//       clean-out branch, but with a stale cursor -- i.e. the clean-out would
//       resume mid-sequence instead of at step 1.  Retire with Magazine.cpp.
//  (G2) golden :258 `iPickWhichBuff=SearchTrayToPick_Buffer();` -- golden
//       aoutarm9045.h:64; NO body anywhere in this tree (not even a stub).
//       Shape (b), `return -1;`.
//       WHY -1 IS FAITHFUL: golden's very next line is `if(iPickWhichBuff!=-1)`,
//       so -1 is golden's own documented "no buffer tray found" sentinel and is
//       the only value that leaves the surrounding block a no-op instead of
//       driving the arm to a fabricated tray index.  BEHAVIOUR DELTA on an ATK-AMR
//       machine: the eAtkTfMoveFixIC clean-out never hands IC off to the buffer
//       (the branch is skipped); on every other machine the enclosing
//       `fAGV->IsATK_AMR()` is false and there is no delta at all.
//  (G3) golden :797 `InitAOIFunction();` -- golden fAOI.h:418/:494; fAOI.cpp is a
//       VCL form unit, untranslated, and the only other mention in this tree is
//       inside aoutarm9045.cpp's own `#if 0` island (:723-:769).  Shape (b), no-op.
//       WHY FAITHFUL: it resets the 5S/AOI inspection sub-SM at the start of a new
//       out-arm cycle.  With no AOI form there is no sub-SM to reset, and the two
//       flags the AOI path keys off (USE_Scanner_AOI_Inspection /
//       ScannerAOIIF.iEnableScannerMode) are read from a machine that does not
//       have the option.  BEHAVIOUR DELTA on an AOI machine: the AOI cursor is not
//       re-armed on InitOutArmTask, so a mid-inspection abort would resume rather
//       than restart.  Retire with fAOI.cpp.
//  (G4) golden :1137, :1150, :1165 `SetOutArmSpeed(true);` -- golden cinitial.h:47
//       / cinitial.cpp:5300, untranslated.  This tree already answers this exact
//       symbol twice with shape (b): csystem.cpp:1268 and AutoClean/AutoClean.cpp
//       :177.  Same shape here, no-op.
//       WHY FAITHFUL: OutArmAddSpeed/OutArmSubSpeed compute the new
//       AutoArmSpeed[OutArm] values and then ask cinitial to PUSH them to the
//       servo.  The computation -- which is what every other reader of
//       AutoArmSpeed sees -- is fully live; only the push is missing, and there is
//       no servo attached offline.  BEHAVIOUR DELTA on a real machine: the
//       adaptive out-arm speed ramp would compute correct targets but never apply
//       them, so the arm would stay at whatever speed the last real
//       SetOutArmSpeed left it.  THIS ONE IS SAFETY-ADJACENT (motion speed) and
//       must be retired before any real-machine run.
//  (G5) golden :2716 and :2762 `CheckOutArmXYScaleByAutoTeach(*iXPos,*iYPos,...)`
//       -- golden AutoAlignment/AutoAlignment.h:254, AutoAlignment.cpp
//       untranslated; grepped whole tree, zero occurrences of the name outside
//       this file.  Shape (b), no-op (signature `(int&,int&,int)` verbatim from
//       golden AutoAlignment.h:254, so the two out-parameters are simply left at
//       the caller's already-computed values).
//       WHY FAITHFUL: golden calls it only under
//       `MACHINE_HAS_AUTO_ALIGNMENT_CCD && TestIF.bEnableAutoAlignment==true`;
//       both branches' `else` arm (the ordinary scale compensation, golden
//       :2718-2755 / :2764-2815) is fully live.  A machine without the alignment
//       CCD never reaches the gated line.  BEHAVIOUR DELTA on an AOA machine: no
//       CCD-measured scale correction is applied to the Auto-tray / out-shuttle
//       target, i.e. it falls back to the un-corrected taught position.
//  (G6) golden :2960 and :3001 `LoadTrayCanUse8Suck()` -- golden ainarm2.h:200;
//       ainarm2.cpp is one of the 26 half-translated units and this function is
//       not among the translated ones (grepped whole tree, zero occurrences of
//       the name outside this file).  Shape (b), `return 0;`.
//       WHY 0 IS FAITHFUL: golden compares `LoadTrayCanUse8Suck()==1`, so 0 is
//       "the loader tray Y-pitch is NOT an exact multiple of the arm Y-pitch",
//       which is the general case; 1 is the special 8-nozzle-at-once optimisation.
//       Returning 0 keeps the ORDINARY path, never the optimisation -- the safe
//       direction (it can only cost throughput, never mis-place an IC).
//       BEHAVIOUR DELTA: on a machine where the pitches DO match,
//       UnloaderTrayYDirForArmYPitch stops reporting true via this second route
//       (the first route, USE_OUT_Y_IS_AUTO_PITCH, is live and unaffected), and
//       AutoCalculateOutArmYClosePitch's small-pitch branch returns the computed
//       OutArmClose_PitchY instead of the short-circuit TestIF.iARM_Y_PITCH.
//  (G7) golden :325, :399, :444, :503 `if(fNote->bMyServoOffOutArm)` -- golden   //AI(W906-FLOW-5) 20260929: G7 RETIRED at all four sites (:914 :992 :1039 :1100) -- bMyServoOffOutArm is on TfNote (forms/fNote.h:305); the text of this (G7) note is history
//       note.h:417.  The port's TfNote (FormsFacade.h) carries bMyServoOffInArm
//       but NOT bMyServoOffOutArm.  Shape (a), `#if 0` with no `#else` (the whole
//       `if(...){return false;}` block is elided), which is BYTE-FOR-BYTE the
//       shape aoutarm9045.cpp:396-401 and :1468-1473 already use for these exact
//       golden lines.
//       WHY FAITHFUL: the flag means "the operator servo-off'd the out arm from
//       the alarm dialog (咖啡杯解除)"; offline there is no such dialog and no
//       servo to release, so false -- i.e. do not refuse the move -- is the
//       correct default.  BEHAVIOUR DELTA on a real machine: after an operator
//       servo-off, these four move helpers would return true (move accepted)
//       instead of refusing, so the position-loss protection that golden added
//       (JerryYang 20161227) is absent.  SAFETY-ADJACENT; retire with note.cpp.
//  (G8) golden :342 `fBarCode->InitialAllCCDUnloaderClip();` -- golden BarCode/
//       BarCode.h:983; no body anywhere in this tree (grepped: zero occurrences
//       of the name).  Shape (a), `#if 0`, no `#else`.
//       WHY FAITHFUL: the call is already inside
//       `CosFunction.bReadClipCodeFromUnloader && IniConfig.bP60ReadClipCodeFromUnloader`
//       -- the P60 unloader-clip-code option -- and only clears the CCD's
//       per-clip result buffers.  BEHAVIOUR DELTA on a P60 machine: stale clip
//       codes from the previous tray are not cleared before the Fix-tray-full
//       retreat.
//  (G9) golden :1067 `FrmAOI->RunTopBottomInspect()==true)` -- the THIRD disjunct
//       of the `if` at golden :1065-:1067.  golden fAOI.h:437; no port body (the
//       only other mentions are inside aoutarm9045.cpp's `#if 0` islands).
//       Shape (a) with `#else` supplying `false)` so the `||` chain stays
//       syntactically whole and the first two disjuncts stay LIVE.
//       WHY false IS FAITHFUL: the disjunct asks "is the Top&Bottom inspection
//       recipe running"; with no AOI form nothing can be running.  BEHAVIOUR
//       DELTA on a Top&Bottom-inspect machine (Jimmychiu 20240322): the AOI
//       break-hole map is not painted into the two iMMAoi trays, so the out arm
//       would place IC into positions the inspection recipe wants left empty.
//  (G10) golden :1197, :1198 `FR/BRCarryKit.SetHasNullIcToNullIc();` -- golden   [AI(W906-FLOW-2) 20260928: G10 RETIRED at :1800 -- the premise in this note is stale]
//       mykitsuck.h:281.  The substrate TMyKitSuck (aHotPlateSubstrate.h:365) has
//       SetNullIcToHasNullIc but NOT the inverse SetHasNullIcToNullIc, and
//       mykitsuck.cpp is deliberately unregistered (NOTE 2).  Shape (a), `#if 0`,
//       no `#else`.  Identical precedent: atester_32Site.cpp:218-220's
//       `W5_32S_SETHASNULLIC` no-op seam and csystem.cpp's
//       `W7C2_BLCARRYKIT_SETHASNULLIC`, both for these same two kits.
//       WHY FAITHFUL OFFLINE: HAS_NULL_IC on an out-shuttle carry kit marks "a
//       cell deliberately left empty for this site map"; converting it back to
//       NULL_IC at the top of DoOutArm re-opens those cells for the next cycle.
//       With no shuttle hardware the grids are all NULL_IC already, so the call
//       is a no-op in fact as well as in code.  BEHAVIOUR DELTA on a real
//       machine: cells marked HAS_NULL_IC by a previous site map stay marked, so
//       the out arm would treat them as permanently unusable -- golden's own
//       comment (Steven 20160524, and jou 2011-06-29 "out shuttle has_null_ic
//       hang up fix") says this is precisely the hang-up this call prevents.
//       Retire by adding the 3-line method to aHotPlateSubstrate.h.
//  (G11) golden :2348 `bCanPick2ICAtOnceTime=fTesterTCP->bOutArmVariAuto_OS[iAuto];`
//       -- golden Interface/TesterTCP.h:261 (a member of the golden TfTesterTCP
//       FORM).  This port's Interface/TesterTCP.h says of itself (:59) "no
//       TfTesterTCP class or facade at all, only free functions", and there is no
//       `fTesterTCP` global anywhere (Automation/SCK_ART_Remainder.cpp:242 gates
//       the same pointer for the same reason).  Shape (a) with `#else` -> `false`.
//       WHY false IS FAITHFUL: bOutArmVariAuto_OS[3] is a per-Auto-tray operator
//       checkbox on the OS-tester page, default-cleared -- golden's own comment
//       (RogerYang 20260210) says 田揚志's requirement is that ONLY the trays that
//       were explicitly ticked may be filled all-at-once, so the un-ticked/absent
//       state is `false`.  Note the whole branch is reached only when
//       `CUSTOMER_CODE==CC_JSCC_OS`; on any other customer code there is no delta.
//       BEHAVIOUR DELTA on a CC_JSCC_OS machine: the out arm places one IC at a
//       time into every Auto tray -- slower, never wrong.
//  (G12) golden :2709 `ASESendMessage->SendToASEData(Data.SubString(1,iLength));`
//       -- golden "ASE_K Socket/aseTest.h":57, a TASESendMessage VCL form;
//       untranslated (its only other appearance in this tree is inside the
//       dfm2rc layout fixtures under build/).  Shape (a), `#if 0`, no `#else`.
//       NOTE: `int iLength=Data.Length()-1;` (golden :2708) stays LIVE, so the
//       function keeps golden's shape and the variable does not vanish.
//       WHY FAITHFUL: this is a pure outbound telemetry hop to the ASE
//       host socket; nothing in the out-arm SM reads a reply.  BEHAVIOUR DELTA on
//       an ASE machine: the per-site bin string is computed but not transmitted,
//       so ASE's host-side record would be missing.  Retire with aseTest.cpp.
//       CROSS-CHECK: aoutarm9045.cpp:149 currently DEFINES a whole-body no-op
//       `void SendDataToASE(AnsiString) {}` -- this file's real body supersedes
//       it; see the report's retirement list.
//  (G13) golden :3535 `if(fAutoTeach->IsRun())` -- golden AutoTeach.h TfAutoTeach,
//       a VCL form with no port body or facade (aoutarm9045.cpp:973 and
//       aoutarm9045_2x4_4.cpp:725 gate the same pointer for the same reason).
//       Shape (a) with `#else` -> `if(false)`, which keeps golden's whole
//       if / else-if / else-if / else-if / else CHAIN intact and LIVE from the
//       second arm onward (this is why the CONDITION is gated and not the block:
//       eliding the block would break the chain).
//       WHY false IS FAITHFUL: the arm asks "is the auto-alignment teach routine
//       running", which offline it never is.  BEHAVIOUR DELTA during a real
//       auto-alignment teach: the IC would be routed by its real bin instead of
//       being forced to eFix3 (the teach collection tray), i.e. teach samples
//       would scatter into production trays.
//  (G14) golden :2309 `fSCKART->CheckOutArmNeedVariModeFIX(iAuto);` -- READ THIS
//       ONE CAREFULLY, it is the only gate here whose golden BODY is already
//       fully translated.  Automation/SCK_ART.cpp:373 is the real, complete
//       translation of golden TfSCKART::CheckOutArmNeedVariModeFIX(int) (golden
//       Automation/SCK_ART.cpp:1031-1070, declared golden Automation/SCK_ART.h
//       :294).  It cannot be called from here anyway, because its signature is
//       `SckArt_CheckOutArmNeedVariModeFIX(SckArtState &st, int iAuto)` and:
//         * forms/fSCKART.h:74 `class TfSCKART` -- the class of the ONE real
//           global `fSCKART` -- does NOT derive from SckArtState (checked this
//           wave by reading the class head; note that forms/fSCKART.h:20-25's
//           ": public SckArtState" remark belongs to list item 3,
//           SckArtRemainderState, NOT to TfSCKART -- easy to misread),
//         * there is NO global SckArtState (or SckArtRemainderState) object
//           anywhere in the tree -- grepped, the only instances are the two
//           file-local `SckArtState core` members inside csystem.cpp's
//           W7C1_/W7C2_TfSCKARTSeam structs, and
//         * forms/fSCKART.h:63 says in as many words "Do NOT invent a SIXTH"
//           declaration site for this state, so standing up a seventh
//           SckArtState here purely to satisfy one call would be a direct
//           violation of a written instruction in the tree.
//       Shape (a), `#if 0`, no `#else`.  WHY THAT IS THE RIGHT DEFAULT: the
//       function's only out-of-struct effect is writing ArmSpeed[OutArm].
//       bVariModeFIX, and golden :2340 (LIVE, 2 lines below) already reads that
//       flag with a plain `==false` test, so not writing it simply leaves
//       whatever the operator/recipe set -- it does not corrupt anything.
//       BEHAVIOUR DELTA on an ART machine (CosFunction.bUseSCKART &&
//       USE_AUTO_RETEST==eartInstall && IniConfig.bA10_AutoReTest &&
//       TestIF_File.bSCKART_EnableART): the two ART-specific forcings of
//       one-IC-at-a-time placement are lost -- (i) Fail-bin IC
//       (Prod.iIsFailT6[iAuto]==1) is no longer forced to Fix mode, and (ii) the
//       end-of-lot guard that switches to Fix mode when the remaining count
//       drops within 20 (CC_TERAPOWER) / 30 devices no longer fires.
//       RETIREMENT: the correct fix is NOT to gate-flip here -- it is for
//       TfSCKART to grow the method as a forwarder over its own state once the
//       five-way SckArtState reconciliation debt in forms/fSCKART.h is settled.
//  ---- GATES FOUND BY THE LINK-CLOSURE AUDIT, NOT BY THE COMPILER ----
//  The three below are DECLARED in headers this file includes, so
//  `-fsyntax-only` was perfectly happy with them.  They were caught by
//  `nm --undefined-only aoutarm.o` diffed against every libht9045_*.a of the
//  most recent full build (build_0808_w3_final, archives timestamped 15:57-16:00
//  on 20260808; diff run 23:40).  All three golden bodies live in csystem.cpp,
//  which is one of the 26 half-translated units (docs/PT_CAMPAIGN_PLAN.md SS3:
//  142 functions / 8,169 lines still missing there), and none of the three is
//  among the translated ones -- so this file would have been the FIRST caller
//  and would have produced three undefined references at integrate time.
//  Because each is declared by an included header, a file-local `static` of the
//  same name is ill-formed ("static declaration follows non-static"); these
//  therefore use the tree's OTHER registered shape for that situation --
//  `static <ret> PTW4_X(...) {} ` + `#define X PTW4_X` -- which is exactly what
//  csystem.cpp:1266-1270 does for cinitial.h-declared symbols.  Golden call
//  sites stay byte-identical; retirement = delete the stub and its #define.
//  (G15) golden :825, :830, :837 `ClearFixTray(i, "SetFixTrayFullIC", 3);` --   [AI(W906-I129-8) 20261002: RETIRED, see the stand-in's old place below]
//       declared port csystem.h:247, golden body csystem.cpp:15853.
//       ** THIS IS THE MOST CONSEQUENTIAL GATE IN THE FILE. **  golden's
//       SetFixTrayFullIC detects a full (or full-half) Fix tray and then calls
//       ClearFixTray to retire it; with the call gated, the DETECTION is fully
//       live but the RETIREMENT never happens.  WHY IT IS STILL THE RIGHT
//       DEFAULT for this tree: there is no alternative -- the only faithful
//       action is csystem.cpp's own 500-line body, and inventing a partial
//       version of it here would be a fabrication that later silently diverges.
//       BEHAVIOUR DELTA ON A REAL MACHINE: a Fix tray that fills up is never
//       marked for change, so the operator is never prompted and the out arm
//       keeps trying to place into a full tray.  DO NOT SHIP WITH THIS GATE OPEN.
//       Retire with csystem.cpp's ClearFixTray (PT-W5).
//  (G16) golden :246 and :262 `CheckMagazineBufferNeedClear();` -- declared port
//       csystem.h:305, golden body csystem.cpp:24055 ("clean out的時候用的").
//       Same Magazine clean-out branch as G1 and inert for the same reason on a
//       non-Magazine machine.  BEHAVIOUR DELTA on a Magazine machine: the
//       buffer-needs-clearing evaluation is skipped, so clean-out proceeds
//       without knowing whether the Magazine buffer still holds IC.
//  (G17) golden :3494 `CheckPickerLifeNeedOneCycle();` -- declared port
//       cmydef.h:5824, golden body csystem.cpp:12774 (AI(ht9045-config) 20260521
//       (RogerYang), "SCC吸嘴壽命報警OneCycle優化").  Already inside
//       `if(IniConfig.bO20InOutArmPickerLifeTimeCount==true)`, and the LINE
//       BEFORE it -- `TestIF_File.OutArmPickerLifeCnt[r][c]++` (golden :3493) --
//       is LIVE, so the counter still accumulates.  BEHAVIOUR DELTA on a machine
//       with O20 picker-life counting on: the count is kept but the
//       "picker life reached -> request OneCycle stop" escalation never fires.
//
//  VENDOR APIs: NONE.  This file calls no dmc_*/Acm_*/mn_*/smc_*/_mnet_*/_l1xx_*
//  entry point, directly or through a macro; all motion goes through MOT[] /
//  OutArmContinuousMove_9045.
//
//  STATIC-INIT: this file defines 6 objects with non-trivial ctors -- QASam
//  (TMyQASample, golden :92) and the 5 TQPF_Timer globals HInspectWait /
//  DoPickFromShuttle1Delay / DoPickFromShuttle2Delay / DoPlaceToAutoDelay
//  (golden :59-:62) and DoSHTWaitTime (golden :93).  Checked against
//  docs/PT_CAMPAIGN_PLAN.md section 8's 18 NULL globals: TMyQASample::TMyQASample
//  calls only ClearCount(), whose body is ZeroMemory over its OWN iCount plus
//  three of its own ints; TQPF_Timer::TQPF_Timer (myTimer.cpp:14) calls only
//  CalibratePerformanceCounterOverhead(), which is QueryPerformanceCounter over
//  its OWN members.  Neither touches ANY global, so neither can be order-
//  dependent.  This is the same argument aHotPlateSubstrate.cpp made for
//  uPlateInfo, and it was verified by reading both ctor bodies, not assumed.
//
//  Big5: every Chinese comment decoded via cp950 and preserved as UTF-8.
//  Final gate: UTF-8, ZERO U+FFFD, ZERO CRLF.
// =============================================================================
//  INCLUDE REWRITE (golden :1-:35 -> the translated subset):
//    golden: MachineDefine.h ; #pragma hdrstop ; aoutarm.h ; aArmHeader.h ;
//            MyKitSuck.h ; cprod.h ; mymotor.h ; mySensor.h ; note.h ;
//            csystem.h ; mymessbox.h ; cinitial.h ; cSortCT.h ; fAOI.h ;
//            aseTest.h ; main.h ; mycylin.h ; cmydef.h ; common.h ; SCK_ART.h ;
//            aRotateKIT.h ; fRotate.h ; InOutArmZteach.h ; cSocket.h ;
//            BarCode.h ; Magazine.h ; TesterTCP.h ; AGV.h ; acatchtray.h ;
//            #pragma package(smart_init)
//    Untranslated-in-this-tree golden headers dropped (their symbols are either
//    gated above or forward-declared below): note.h, mymessbox.h, cSortCT.h,
//    fAOI.h, aseTest.h, main.h, fRotate.h, InOutArmZteach.h, BarCode.h,
//    Magazine.h, AGV.h.  #pragma hdrstop / #pragma package(smart_init) dropped
//    per this wave's contract.
// =============================================================================
#include "MachineDefine.h"
#include "MachineType.h"            // MAX_ARM_Row / MAX_ARM_Col / X_PITCH_COUNT / ChangeToFloatNonPcnt / CUSTOMER_CODE + CC_* + e* enums
//---------------------------------------------------------------------------
#include "aoutarm.h"                // this unit's own public surface (golden aoutarm.h)
#include "aArmHeader.h"             // golden god-header (guard-only shim in this tree)
#include "aoutarm9045.h"            // engine: MoveOutArmToAutoSafe_9045 / CheckOutArmInitState_9045 / DoOutArm_9045 / GetOutArmPitchX,Y_9045 / OutArmContinuousMove_9045 / UseFix3Cylinder
#include "csystem.h"                // InSHT1InRT / InSHT2InRT / ShuttleHasIC / IndexHasIC / IsInArmCleanOutFinish / IsOutArmCleanOutFinish / DoOutArmAfterPlaceToAuto / SendDataToASE decl / EnableFix3UseCylinder
#include "Motor/mymotor.h"          // MOT[] / TMyMotor / OutArmContinuousMove_9045
#include "cmydef.h"                 // global scalar universe + IC consts + iInArmType + iMMAuto / iFixMin.. / iMagMin.. / iOutArmYBase / iXpitch* / M* motor ids / iCleanOut / iOneCycle
#include "cprod.h"                  // Prod / TestIF / TestIF_File / ArmSpeed / ArmSpeed_File / AutoArmSpeed / AutoForm / LoadForm / TrayForm / DeviceForm / Tech / Temperature / BinSelect / LotSummary / OutArmOffSet / tRotate
#include "cpublic.h"                // CosFunction / IniConfig
#include "common.h"                 // FileExists / asBarCodeLot / asBarCodeErrorSend
#include "mysensor.h"               // Sen[] (SnFix3FullPlace)
#include "mycylin.h"                // Cylinder[] (C_TrayCover)
#include "cSocket.h"                // TMySocket surface used via the shuttle kits
#include "cinitial.h"               // ClearFixTray / machine-config readers
#include "acatchtray.h"             // WhichAutoNeedTray / IsTrayArmMoveAvoidOutArmCrash
#include "aHotPlateSubstrate.h"     // TMyKitSuck (SEE NOTE 2) + OutArmSuck / InArmSuck / F,BLCarryKit / F,BRCarryKit / F,BTestSuck + TMySucker + TMyProductionRecord
#include "acarry_shims.h"           // MoveOutArmToAutoSafe / MoveOutArmXY_ToShuttleAlarmArea decls
#include "acatchtray_shims.h"       // MoveOutArmXY_ToFix_Tray_Full decl / iWhichAuto
#include "csystem_shims.h"          // DoOutArm / InitOutArmTask / SetFixTrayMiddleDtata decls (their stub BODIES are retired by this file -- see report)
#include "ainarm9045_2x4_16_shims.h"// AutoTeachLoadTrayZ
#include "RotateKit/aRotateKIT.h"   // rotate-kit surface (tRotate consumers)
#include "BarCode/BarCode_Bottom2DID.h" // list2DByLot  (adaptation A5)
#include "FormsFacade.h"            // fNote / fBarCode / fAGV / fSCKART / fFixAICCD offline stand-ins
#include "canary_support.h"          // ShowErrorMessage / ShowMyMessage / RecordProcess / NewRecordProcess / MyDBIProcess / LastSet / K_RETRY / K_SKIP / K_HOME / DUMMY / REALLY
//---------------------------------------------------------------------------

//==============================================================================
//==  Cross-module forward declarations -- REAL bodies, no gate.
//==  Guarded with #ifndef so the serial Integrate agent can move any of these
//==  into a shared header without clashing.  Each names the .cpp that defines it
//==  and that .cpp's CMakeLists.txt registration was checked this wave.
//==============================================================================
#ifndef AOUTARM_ENGINE_XMOD_FWD
#define AOUTARM_ENGINE_XMOD_FWD
extern TMyKitSuck *ptrOutSHT;                                                   // REAL def acarry.cpp:99 (ht9045_sm) -- golden mykitsuck.cpp:43 / mykitsuck.h:381; same TMyKitSuck layout, verified (NOTE 2)
int  GetOutArmToShuttleOffset_9045(int iSht, int iModeRow, int iModeCol, bool bPitch=true);                               // REAL def aoutarm_shims.cpp:42 (ht9045_sm) -- golden aoutarm9045.cpp:1211
int  GetVariableYOutShuttleData();                                              // REAL def aoutarm_shims.cpp:43 (ht9045_sm) -- golden aoutarm9045.cpp (ChungHung 20131231)
extern int iOutArmZTeachTask;                                                   // REAL def aoutarm_shims.cpp:33 (ht9045_sm) -- golden AutoTeach.h:235
#endif

// eAtkTfMoveFixIC -- golden Automation/AGV.h:218, `enum eATkTrayFeed{...
// eAtkTfMoveFixIC=6, ...}` (value read out of golden this wave, not guessed).
// AGV.h is not translated in this tree, so the enumerator has no header home;
// the VERBATIM value is provided behind a guard exactly as aoutarm9045_2x8_8.cpp
// :320-326 already does for this same enumerator.  Offline fAGV->IsATK_AMR() is
// false, so the only branch that reads it is inert -- the value merely has to
// exist for the comparison to compile.  NOT a gate: the value is golden's.
#ifndef AOUT_EATKTFMOVEFIXIC_FWD
#define AOUT_EATKTFMOVEFIXIC_FWD
static const int eAtkTfMoveFixIC = 6;                                           // golden Automation/AGV.h:218
#endif

//==============================================================================
//==  GATE shape (b) -- file-local `static` stand-ins with golden's EXACT
//==  signature, so every golden call site below stays BYTE-IDENTICAL to golden
//==  and retirement is a pure deletion of these five lines.  Rationale, faithful-
//==  default argument and real-machine behaviour delta for each: GATE REGISTER
//==  entries G1-G6 in the banner above.  Precedent for the shape:
//==  aoutarm9045.cpp:147-174, csystem.cpp:1266-1270, AutoClean/AutoClean.cpp:177.
//==============================================================================
static void InitialDoPickFromMagazineBuffer()                        {}         // (G1) golden Magazine.h:151      -- offline: magazine-buffer pick cursor not re-armed
#if 0   // PT-W8 RETIRED (SearchTrayToPick_Buffer): real translated body now live
static int  SearchTrayToPick_Buffer()                                { return -1; } // (G2) golden aoutarm9045.h:64 -- offline: golden's own "not found" sentinel
#endif
static void InitAOIFunction()                                        {}         // (G3) golden fAOI.h:418          -- offline: no AOI sub-SM to reset
//AI(ht9045-v906) 20260810: PT-W7a integrate -- RENAMED, not retired, and the reason matters.
// cinitial.cpp now holds golden's REAL SetOutArmSpeed (cinitial.cpp:10554), and cinitial.h's
// declaration now reaches this TU, which made `extern` followed by `static` a hard compile
// error here -- trap 4 (the static shadow) finally surfacing loudly instead of silently.
// Deleting the shadow would bind the three call sites below (:1722/:1735/:1750) to the real
// body, and that body calls SetMotorAccelSpeed/SetMotorScaleSpeed on MOutArmX/MOutArmY/
// MOutArmPitch. This file's OWN note at :249 already calls that "SAFETY-ADJACENT (motion
// speed)". Per the campaign policy, a safety-critical behaviour change queues for the user
// rather than landing unattended, so behaviour is preserved EXACTLY as it was: the call sites
// keep calling a private no-op, now under a name that cannot collide and cannot be mistaken
// for the real thing. No macro, no shadow -- the code says what it does.
// UN-GATE TASK: delete this stub, restore the three call sites to SetOutArmSpeed(true), and
// measure on its own fresh Debug+Release. Same for AutoClean/AutoClean.cpp:177, which holds
// an identical shadow that did not error only because cinitial.h does not reach it.
static void W7A_OutArmSpeed_NoOp_PendingSafetyReview(bool /*bShow*/) {}          // (G4) golden cinitial.h:47       -- offline: computed speed not pushed to servo  ** SAFETY-ADJACENT **
static void CheckOutArmXYScaleByAutoTeach(int & /*iXPos*/, int & /*iYPos*/, int /*iArea*/) {} // (G5) golden AutoAlignment/AutoAlignment.h:254 -- offline: leave caller's taught position uncorrected
#if 0   // PT-W7e-part2 RETIRED (LoadTrayCanUse8Suck): real translated body now in ainarm2.cpp
static int  LoadTrayCanUse8Suck()                                    { return 0; }  // (G6) golden ainarm2.h:200    -- offline: 0 (!=1) keeps the ORDINARY, non-8-nozzle path
#endif

//==============================================================================
//==  GATE shape (b'), for the three symbols that ARE declared by an included
//==  header but have NO body anywhere (found by the nm link-closure audit, not
//==  by the compiler -- see GATE REGISTER G15/G16/G17).  A plain file-local
//==  `static` of the same name would be ill-formed after those declarations, so
//==  these use stub + #define, the shape csystem.cpp:1266-1270 already uses for
//==  exactly this situation.  Placed AFTER every #include on purpose, so the real
//==  declarations above are NOT macro-mangled.  Retirement = delete these 6 lines.
//==============================================================================
// AI(W906-I129-8) 20261002: (G15) RETIRED (INBOX 129 (a)(b) #8, RULINGS_20261001 #0) -- the empty PTW4_ClearFixTray stand-in and its #define are gone; csystem.cpp:13586 ClearFixTray is live (= golden csystem.cpp:15853; only its iAction==2 barcode-record block is still gated, G1 there),
//   so SetFixTrayFullIC's three ClearFixTray(i, "SetFixTrayFullIC", 3) calls (golden :825 / :830 / :837) now mark the full Fix tray's empty cells HAS_IC (MOT[iMMAuto[i]].SetNullIcToHasIc()) and the tray gets changed, as golden.
static void PTW4_CheckMagazineBufferNeedClear()                              {} // (G16) decl csystem.h:305    / golden body csystem.cpp:24055
#define CheckMagazineBufferNeedClear    PTW4_CheckMagazineBufferNeedClear
static void PTW4_CheckPickerLifeNeedOneCycle()                               {} // (G17) decl cmydef.h:5824    / golden body csystem.cpp:12774
#define CheckPickerLifeNeedOneCycle     PTW4_CheckPickerLifeNeedOneCycle

//==============================================================================
//==  ADAPTATION A3 -- golden mykitsuck.h:122's one-line inline setter, reproduced
//==  as a function-like macro over the storage the substrate TMySucker DID land
//==  (aHotPlateSubstrate.h:148).  `X.SetNeedDestroy(true);` expands to
//==  `X.bNeedDestroy = (true);` -- the same write to the same public member, so
//==  this is NOT a gate.  Defined AFTER all #includes because no included header
//==  spells this token.  Identical precedent: RotateKit/aRotateKIT_In.cpp:200.
//==  Retire by adding the real setter to aHotPlateSubstrate.h.
//==============================================================================
#ifndef SetNeedDestroy
#define SetNeedDestroy(v)   bNeedDestroy = (v)                                  // golden mykitsuck.h:122  void SetNeedDestroy(bool Value) {bNeedDestroy=Value;}
#endif
//==============================================================================
//==============================================================================
extern bool IsTrayArmMoveAvoidOutArmCrash();                                    //Steven 20260428 [E90]: forward decl from acatchtray.h
bool bOutArmXOverLimit=false;                                                   //Steven 20171206 (Wei) : In ATK, 修正Out Arm同時放料但是超出軟體極限縮Pitch導致疊料的問題
bool bOutArmSuckActive[MAX_ARM_Row][MAX_ARM_Col]={{false, false, false, false}, {false, false, false, false}};
bool bOutArmSuckActiveBack[MAX_ARM_Row][MAX_ARM_Col]={{false, false, false, false}, {false, false, false, false}};
bool bOutArmManualStepPress=false;
int OutArmTask=1;
int iDoOutArmAfterPlaceToAutoTask=1;                                            //Steven 20220526 : 針對放下IC到Unloader後的動作做整合
int iOutArmPlaceOrder=0;
int iOutArmPickOrder=0;                                                         //JerryYang 20221215 : Magazine把fix區當buffer區功能
int iWhichAuto;
int iScannerFailCount[4]={0, 0, 0, 0};
int iScannerContinueFail=0;
int iScanCount=0;

int iPickFromShuttle1Task=1;
int iPickFromShuttle2Task=1;
int iPlaceToAutoTask=1;
int iPlaceToFixTask=1;
int iOutArmInspectTask=1;
int iOutArmPickFromAuto1Task=1;
int iPlaceToOutRotateTask=1;
int iPlaceToSortShtTask=1;                                                      //RogerYang 20250513 Add for 9046AU
TQPF_Timer HInspectWait;
TQPF_Timer DoPickFromShuttle1Delay;
TQPF_Timer DoPickFromShuttle2Delay;
TQPF_Timer DoPlaceToAutoDelay;

int iOutArmXPosition[MAX_ARM_Row][MAX_ARM_Col]={{0, 0, 0, 0}, {0, 0, 0, 0}};
int iOutArmYPosition=0;
bool bPlaceToAuto1IsFull=false;
bool bCarryControlOutarm1=false;                                                //jou 2011-11-21 bCarryControlOutarm -> bCarryControlOutarm1 不應該共用一個flag
bool bCarryControlOutarm2=false;

bool bPickShuttleError=false;

bool bUseOnebyOne=false;                                                        //2010-12-07

bool bOverTray=false;                                                           //jou 2012-09-04 修正極限無法到達時,回填Null_IC會造成找不到地方放,會放在 x=-1,y=-1的位置上

int iFix3CanFullTask=1;                                                         //ChungHung 20140313 add Fix3 can Full Tray

int iPlaceToMagazineTrayTask    =1;                                             //JerryYang 20221215 : add Magazine
int iPlaceToMagazineBufferTask  =1;
int iPickFromMagazineBufferTask =1;
int iAuto3MagazineTask          =1;
int iMagazineUpDownTask         =1;
int iCatchTrayGetTrayTask       =1;
int iCatchTrayReceiveTrayTask   =1;
int iMagazineTrayFeedTask       =1;
int iCatchTrayGetNewTrayTask    =1;                                             // 2012.11.05 , Joye , HT-7046
int iCatchTrayChangeTrayTask    =1;                                             // 2012.11.05 , Joye , HT-7046
int iCatchMagazineTrayOut       =1;
int iDoMagazineScanHasTrayTask  =1;
int iMagazineInitCheckTask      =1;

TMyQASample QASam;
TQPF_Timer DoSHTWaitTime;                                                       //kevin 20210525 out arm 在shuttle 上等待
//==============================================================================
bool CheckOutArmZ(bool bMessage)
{
    AnsiString S1, S2;
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
                    ShowErrorMessage("WAR0257", 0, MOutArmX, false, MOT[iMotZ].Alias);                                  //出料手臂的吸嘴 %s 沒有在安全位置
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
bool CheckTesterZ(int WhichShuttle)
{
    if(IniConfig.bD45UseOutArmCheckIndex==false)
      return false;

    if(WhichShuttle==0)                                                         //Steven 20150407 : 修正[D45] Out Arm等Index Z功能, 避免Auto Homing
    {
        if(bZ1PickShuttle==true)
            return true;
    }
    else if(WhichShuttle==1)
    {
        if(bZ2PickShuttle==true)
            return true;
    }
    return false;
}
//==============================================================================
bool bOutShtwaitPick()                                                          //kevin 20210602 Out arm sht on wait
{
    if(ArmSpeed[OutArm].dWaitOnSH!=0)                                           //kevin 20210525 out arm 起動在shuttle 上等待 時間
    {
        if(DoSHTWaitTime.Off())
              return true;
    }
    return false;
}
//==============================================================================
bool CheckShuttleICPos(int WhichShuttle)
{
    if(iOneCycle || iCleanOut || IniConfig.bD45UseOutArmCheckIndex==false)
    {
        return false;
    }
    else
    {
        if(WhichShuttle==1)
        {
//#ifdef Carry4
//            if((TestIF.iTestMode==DualSiteBS || TestIF.iTestMode==QualSite2X2BS) && OutSHT1InRT())
//                return false;
//            if(FLCarryKit.UseSiteHasIC() && OutSHT1InRT())
//#else
//            if((TestIF.iTestMode==DualSiteBS || TestIF.iTestMode==QualSite2X2BS) && InSHT1InRT())
//                return false;
            if(FLCarryKit.UseSiteHasIC() && InSHT1InRT())
//#endif
            {
                if(FRCarryKit.UseSiteNoIC())
                {
                    return false;
                }
                else if(FRCarryKit.UseSiteNoIC())
                {
                    if(FTestSuck.UseSiteHasIC())
                        return false;
                    else
                        return true;
                }
                else
                {
                    return true;
                }
            }
        }
        else
        {
//#ifdef Carry4
//            if((TestIF.iTestMode==DualSiteBS || TestIF.iTestMode==QualSite2X2BS) && OutSHT2InRT())
//                return false;
//            if(BLCarryKit.UseSiteHasIC() && OutSHT2InRT())
//#else
//            if((TestIF.iTestMode==DualSiteBS || TestIF.iTestMode==QualSite2X2BS) && InSHT2InRT())
//                return false;
            if(BLCarryKit.UseSiteHasIC() && InSHT2InRT())
//#endif
            {
                if(BRCarryKit.UseSiteNoIC())
                {
                    return false;
                }
                else if(BRCarryKit.UseSiteNoIC())
                {
                    if(BTestSuck.UseSiteHasIC())
                        return false;
                    else
                        return true;
                }
                else
                {
                    return true;
                }
            }
        }
        return false;
    }
}
//==============================================================================
int CheckOutArmCleanOut(int Task)
{
    //JerryYang 20150910 START:Auto Sorting BinTray by Out Arm when Clean Out
    bool bCheckSpeed;

    if(iCleanOut==1 &&
       IniConfig.bP27AutoSortingBinTrayByOutArmwhenCleanOut==true &&            //JerryYang 20150910 START:Auto Sorting BinTray by Out Arm when Clean Out
       CosFunction.bSortingBinTraywhenCleanOut==true)                           //JerryYang 20170911 (Steven) Clean Out時整盤
    {
        if(InArmSuck.HasIC()==false   &&
           OutArmSuck.HasIC()==false  &&
           ShuttleHasIC()==false      &&
           IndexHasIC()==false        &&
           IsInArmCleanOutFinish()==true)                                       //RogerYang 20170609 (wei) 修正 bP27AutoSortingBinTrayByOutArmwhenCleanOut==false 時, Auto Sorting HangUp 問題
        {
            return 5000;
        }
    }
    else if(iCleanOut==1 &&
            AUTO3_IS_MAGAZINE==1 &&
            TestIF_File.iMagFixTrayType==1)                                     //JerryYang 20221215 : Magazine把fix區當buffer區功能
    {
        if(InArmSuck.HasIC()==false   &&
           OutArmSuck.HasIC()==false  &&
           ShuttleHasIC()==false      &&
           IndexHasIC()==false        &&
           IsInArmCleanOutFinish()==true)
        {
            InitialDoPickFromMagazineBuffer();
            CheckMagazineBufferNeedClear();
            return 11100;
        }
    }
    else if(fAGV->IsATK_AMR() &&
            LastSet.iUnloadFixTray==eAtkTfMoveFixIC)                            //Steven 20260202 : for ATK AMR
    {
        if(InArmSuck.HasIC()==false   &&
           OutArmSuck.HasIC()==false  &&
           ShuttleHasIC()==false      &&
           IndexHasIC()==false)
        {
            iPickWhichBuff=SearchTrayToPick_Buffer();
            if(iPickWhichBuff!=-1)
            {
                InitialDoPickFromMagazineBuffer();
                CheckMagazineBufferNeedClear();
                return 11100;
            }
        }
    }

    if(bOutarmAutoHigh)                                                         //kevin 20171115 (wei) load initial load 一盤 Outarm 校正高度Z軸先不要下去吸
    {
        AutoTeachLoadTrayZ(true, OutArm, iOutArmZTeachTask);
        return 310;
    }

    if(IniConfig. bA15_1ESDGiveWayFunction==true && WhichAutoNeedTray()==0)     //Eastsun 20260521 整合
    {
        if(bOutArmIonFanGiveWay==false)
        {
            DoOutArmAfterPlaceToAuto(bCheckSpeed, true);
            bOutArmIonFanGiveWay=true;
            //return 3500;
        }

        if(IsOutArmCleanOutFinish())
        {
            //if(iTrayFeed==false)
                bOutArmIonFanGiveWay=false;
        }

        if(bOutArmIonFanGiveWay)                                                //Eastsun 20260226 : 修正clean out OutArm來回跑問題
        {
            return 3500;
        }
    }
    return Task;
}
//==============================================================================
//  Ver : 2003_07
// 輸出臂移到自動 TRAY安全高度
//==============================================================================
bool MoveOutArmToAutoSafe()                                                     //只上升吸嘴到安全高度
{
    return MoveOutArmToAutoSafe_9045();
}
//==============================================================================
// 輸出臂 x y 軸移到 shuttle 1
//  Ver : 2003_07
//==============================================================================
//固定粹盤滿了輸出臂xy移動到(-38000, Prod.iOutArmSafeY)
//  Ver : 2003_07
//==============================================================================
bool MoveOutArmXY_ToFix_Tray_Full(bool bMoveY)                                  //Ifor 20250926 add:Fix Tray 取放時移開Out Arm
{
    // AI(W906-PT-W4-integrate) 20260809: GUARD ADDED -- golden has none here, and this
    //   is the only edit to this unit's translated logic.  It is the direct analogue of
    //   the `if(elLaser)` guard PT-W3 added in OmronLaser/LaserSensor.cpp, for the same
    //   plan-section-8 reason: a faithful body reaching a global that a NOT-YET-PORTED
    //   initialisation would have filled.
    //   MEASURED, not predicted: retiring acatchtray_shims.cpp:214's `return true` stub
    //   made this the live body, and W6_6_Hub + W6_6_CSystemCycle went SEGFAULT with the
    //   backtrace  main -> DoAllProcess -> DoOutArm -> DoOutArm_9045 ->
    //   DoOutArm_9045_1x1_1 -> MoveOutArmXY_ToFix_Tray_Full, faulting on the very next
    //   line: `MOT[MOutArmX].Motor` is NULL, so `->PSoftLimitN` is a null deref.
    //   WHY IT IS NULL: golden attaches a driver per axis in cinitial.cpp:3482-3542
    //   (`MOT[i].Motor = new TMyGALILMotor/TMyMN200Motor/...` by MOTOR_DRIVER_TYPE); the
    //   port's cinitial.cpp has never reached that region, and several test banners
    //   (e.g. tests/test_w6_4_tester.cpp:133) document "offline MOT[].Motor==NULL for
    //   every axis" as the tree's standing convention.
    //   RETURNING false IS GOLDEN-REACHABLE, not invented: this function already returns
    //   false from two of its own early-outs below (the G7 servo-off gate and the E90
    //   crash-avoid gate), and the callers treat false as "not moved yet, retry next
    //   cycle" -- which is exactly the state a machine whose X axis has no driver is in.
    //   ⚠ HAND-OFF -- DO NOT just delete this guard.  There are 16 more unguarded
    //   `MOT[].Motor->` derefs in this file and asortarm.cpp (PSoftLimitN/P, Enable,
    //   ReadPos), on paths these tests do not currently reach, and 292 tree-wide.  They
    //   cannot be fixed one guard at a time: the real fix is to attach a driver offline
    //   (Motor/mySimMotor.cpp's TMySimMotor exists for it), and that CANNOT be done in
    //   isolation either, because Motor/mymotor.cpp's Gali_Two_ZAxis_Move / ISNormal /
    //   GalilTwoY_Move stubs currently return `(Motor==NULL)` as their offline
    //   "completed" answer -- attaching a driver flips those three from true to false.
    //   The NULL-ness and those fast paths are two halves of ONE convention; they move
    //   together, in their own wave, with their own measurement.  See docs/DEVLOG.md.
    if(MOT[MOutArmX].Motor==NULL)
        return false;
    int iXPos=MOT[MOutArmX].Motor->PSoftLimitN+iOutArmXBase*2000+100;           //Steven 20211005 : 針對46LS且大IC於Fix換盤讓位要換位置 //Steven 20260331 : 要小心Aa的IC會撞到Sensor //Steven 20260423 : ATK HT-9046LS Hybrid by Matthew Han
    int iYPos=Prod.iOutArmSafeY;
    int iXVariable[X_PITCH_COUNT];
    int iYVariable=0;
    bool bZFlag[MAX_ARM_Row][MAX_ARM_Col];
    int iZPos[MAX_ARM_Row][MAX_ARM_Col];
    ZeroMemory(iXVariable, sizeof(iXVariable));
    ZeroMemory(bZFlag, sizeof(bZFlag));
    ZeroMemory(iZPos, sizeof(iZPos));

    if(IniConfig.bAlarmNeedServoOff)                                            //Steven 20111226 : Out Arm咖啡杯解除
    {
//#if 0   // GATE G7 -- golden `fNote->bMyServoOffOutArm` (note.h:417); the port TfNote carries bMyServoOffInArm only.   //AI(W906-FLOW-5) 20260929: gate RETIRED -- golden aoutarm.cpp:323-329 (MoveOutArmXY_ToFix_Tray_Full) restored; the stated reason is false since AI(W906-HOME-C2) 20260920: TfNote::bMyServoOffOutArm exists (forms/fNote.h:305, ctor false forms/fNote.cpp:42) on the one live fNote (forms/fNote.cpp:48) and is already read live at aoutarm9045.cpp:762 (DoOutArm_9045, FLOW-2); the only reason a gate may stay is a missing dependency, and there is none. Inert until a writer sets it (golden main.cpp:7182/:7215, note.cpp:2074 -- not translated yet); when one lands this refuses the move exactly as golden does
        if(fNote->bMyServoOffOutArm)
        {
            return false;
        }
//#endif   //AI(W906-FLOW-5) 20260929: end of the retired GATE G7 (:914)
    }

    //Steven 20260428 [E90]: When bE90 ON and bMoveY=true, OutArm Y will retreat to iOutArmSafeY_FixFull (= SafeY-15000).
    //If TrayX is in danger zone (>= Empty 31600), abort and let case 4000 retry next cycle.
    if(bMoveY==true && IniConfig.bE90_OutArmFixFullExtraY==true)
    {
        if(IsTrayArmMoveAvoidOutArmCrash()==true)
            return false;
    }

    if(CosFunction.bReadClipCodeFromUnloader==true &&
       IniConfig.bP60ReadClipCodeFromUnloader==true)
    {
#if 0   // GATE G8 -- golden `fBarCode->InitialAllCCDUnloaderClip()` (BarCode/BarCode.h:983); no body anywhere in this tree.
        fBarCode->InitialAllCCDUnloaderClip();
#endif
    }

    if(CUSTOMER_CODE==CC_ASE_SG)
    {
        if(bMoveY==true)                                                        //Ifor 20250926 add:Fix Tray 取放時移開Out Arm
            iYPos=Prod.iOutArmSafeY-15000;                                      //Steven 20260428 [E90]: ASE_SG keeps original behavior; bE90 forced ON makes iOutArmSafeY_FixFull == SafeY-15000 (equivalent)
        else
            iYPos=Prod.iOutArmSafeY;
        iXPos=Tech.iOutArmAuto2X;
    }
    else
    {
        //Steven 20260428 [E90]: bMoveY=true uses iOutArmSafeY_FixFull (SafeY-15000 if bE90 ON, else SafeY)
        if(bMoveY==true)
            iYPos=Prod.iOutArmSafeY_FixFull;
        else
            iYPos=Prod.iOutArmSafeY;
    }

    iXVariable[0]=MOT[MOutArmPitch].ReadPos();
    if(USE_OUT_ARM_Y_PITCH==iXYPitchVariable ||                                 //JerryYang 20251218 : IN/OUT ARM支援不同模組
       USE_OUT_ARM_Y_PITCH==iXYPitchIn_Bb_Out_Bc)                               //Ztex 2024.02.24 Add HT-1132
    {
        iXVariable[1]=MOT[MOutArmPitchX2].ReadPos();
        iYVariable   =MOT[MOutArmPitchY].ReadPos();
    }
    else if(USE_OUT_ARM_Y_PITCH==iXYPitch16Picker ||                            //Steven for HT1032    //JerryYang 20251218 : IN/OUT ARM支援不同模組
            USE_OUT_ARM_Y_PITCH==iXYPitch16Bd_Be)                               //Ztex 2023.12.06 Add HT-1032
    {
        iXVariable[1]=MOT[MOutArmPitchX2].ReadPos();
        iXVariable[2]=MOT[MOutArmPitchX3].ReadPos();
        iXVariable[3]=MOT[MOutArmPitchX4].ReadPos();
        iYVariable   =MOT[MOutArmPitchY].ReadPos();                             //Steven 20131002 : XY變距
    }

    if(OutArmContinuousMove_9045(iXPos, iYPos, iXVariable, iYVariable, bZFlag, iZPos, false))                           //Steven for HT1032
        return true;
    return false;
}
//==============================================================================
//固定Shuttle Alarm輸出臂xy移動到(Tech.iOutArmAuto2X,, Tech.iOutArmAuto2Y)
//  Ver : 2003_07
//==============================================================================
bool MoveOutArmXY_ToShuttleAlarmArea()
{
    int iXPos=0, iYPos=0;
    int iXVariable[X_PITCH_COUNT];
    int iYVariable=0;
    bool bZFlag[MAX_ARM_Row][MAX_ARM_Col];
    int iZPos[MAX_ARM_Row][MAX_ARM_Col];
    ZeroMemory(iXVariable, sizeof(iXVariable));
    ZeroMemory(bZFlag, sizeof(bZFlag));
    ZeroMemory(iZPos, sizeof(iZPos));

    if(IniConfig.bAlarmNeedServoOff)                                            //Steven 20111226 : Out Arm咖啡杯解除
    {
//#if 0   // GATE G7 -- golden `fNote->bMyServoOffOutArm` (note.h:417); the port TfNote carries bMyServoOffInArm only.   //AI(W906-FLOW-5) 20260929: gate RETIRED -- golden aoutarm.cpp:397-403 (MoveOutArmXY_ToShuttleAlarmArea) restored; same reason as :914
        if(fNote->bMyServoOffOutArm)
        {
            return false;
        }
//#endif   //AI(W906-FLOW-5) 20260929: end of the retired GATE G7 (:992)
    }

    iXPos=Tech.iOutArmAuto2X;
    iYPos=Tech.iOutArmAuto2Y+6500;
    iXVariable[0]=MOT[MOutArmPitch].ReadPos();
    if(USE_OUT_ARM_Y_PITCH==iXYPitchVariable ||                                 //JerryYang 20251218 : IN/OUT ARM支援不同模組
       USE_OUT_ARM_Y_PITCH==iXYPitchIn_Bb_Out_Bc)                               //Ztex 2024.02.24 Add HT-1132
    {
        iXVariable[1]=MOT[MOutArmPitchX2].ReadPos();
        iYVariable   =MOT[MOutArmPitchY].ReadPos();
    }
    else if(USE_OUT_ARM_Y_PITCH==iXYPitch16Picker ||                            //Steven for HT1032    //JerryYang 20251218 : IN/OUT ARM支援不同模組
            USE_OUT_ARM_Y_PITCH==iXYPitch16Bd_Be)                               //Ztex 2023.12.06 Add HT-1032
    {
        iXVariable[1]=MOT[MOutArmPitchX2].ReadPos();
        iXVariable[2]=MOT[MOutArmPitchX3].ReadPos();
        iXVariable[3]=MOT[MOutArmPitchX4].ReadPos();
        iYVariable   =MOT[MOutArmPitchY].ReadPos();                             //Steven 20131002 : XY變距
    }

    if(OutArmContinuousMove_9045(iXPos, iYPos, iXVariable, iYVariable, bZFlag, iZPos, false))                           //Steven for HT1032
    {
        bPickShuttleError=true;
        return true;
    }
    return false;
}
//-----------------------------------------------------------------------------
bool MoveOutArm2XYToDecayTeach()                                                //Ifor 20151210 :新增OutArm Move to Decay Teach 點位
{
    int iXPos=0, iYPos=0;
    int iXVariable[X_PITCH_COUNT];
    int iYVariable=0;
    bool bZFlag[MAX_ARM_Row][MAX_ARM_Col];
    int iZPos[MAX_ARM_Row][MAX_ARM_Col];
    ZeroMemory(iXVariable, sizeof(iXVariable));
    ZeroMemory(bZFlag, sizeof(bZFlag));
    ZeroMemory(iZPos, sizeof(iZPos));

    if(IniConfig.bAlarmNeedServoOff)                                            //Steven 20111226 : Out Arm咖啡杯解除
    {
//#if 0   // GATE G7 -- golden `fNote->bMyServoOffOutArm` (note.h:417). The WHOLE servo-off arm (DEBUG_AutoHomeLog block, fAllMotorHome=false, CC_SIGURD_PeiXing ShowMyMessage, return false) is kept golden-verbatim inside the gate.   //AI(W906-FLOW-5) 20260929: gate RETIRED -- golden aoutarm.cpp:442-460 (MoveOutArm2XYToDecayTeach, incl. fAllMotorHome=false and the CC_SIGURD_PeiXing message) restored; same reason as :914
        if(fNote->bMyServoOffOutArm)
        {
            #ifdef DEBUG_AutoHomeLog
            if(fAllMotorHome==true)
            {
                NewRecordProcess("", "fAllMotorHome", "MoveOutArm2XYToDecayTeach");
            }
            #endif

            fAllMotorHome=false;                                                //Ifor 20151211 須重新回home
            if(CUSTOMER_CODE==CC_SIGURD_PeiXing)
            {
                ShowMyMessage("Output Arm Servo Off, need home");               //JerryYang 20160328 for 矽格北興,觸發回home的地方補上log
            }
            return false;
        }
//#endif   //AI(W906-FLOW-5) 20260929: end of the retired GATE G7 (:1039)
    }

    iXPos=Prod.iOutArmDecay_X;
    iYPos=Prod.iOutArmDecay_Y;
    iXVariable[0]=MOT[MOutArmPitch].ReadPos();
    if(USE_OUT_ARM_Y_PITCH==iXYPitchVariable ||                                 //JerryYang 20251218 : IN/OUT ARM支援不同模組
       USE_OUT_ARM_Y_PITCH==iXYPitchIn_Bb_Out_Bc)                               //Ztex 2024.02.24 Add HT-1132
    {
        iXVariable[1]=MOT[MOutArmPitchX2].ReadPos();
        iYVariable   =MOT[MOutArmPitchY].ReadPos();
    }
    else if(USE_OUT_ARM_Y_PITCH==iXYPitch16Picker ||                            //Steven for HT1032  //JerryYang 20251218 : IN/OUT ARM支援不同模組
            USE_OUT_ARM_Y_PITCH==iXYPitch16Bd_Be)                               //Ztex 2023.12.06 Add HT-1032
    {
        iXVariable[1]=MOT[MOutArmPitchX2].ReadPos();
        iXVariable[2]=MOT[MOutArmPitchX3].ReadPos();
        iXVariable[3]=MOT[MOutArmPitchX4].ReadPos();
        iYVariable   =MOT[MOutArmPitchY].ReadPos();                             //Steven 20131002 : XY變距
    }

    if(OutArmContinuousMove_9045(iXPos, iYPos, iXVariable, iYVariable, bZFlag, iZPos, false))                           //Steven for HT1032
    {
        bPickShuttleError=true;                                                 //Sam 20240904 : 移到裡面
        return true;
    }
    return false;
}
//==============================================================================
bool MoveOutArmXY_ToShtMidArea()
{
    bool bZFlag[MAX_ARM_Row][MAX_ARM_Col];
    int iZPos[MAX_ARM_Row][MAX_ARM_Col];
    int iXVariable[X_PITCH_COUNT];
    int iXPos               =Prod.XOutArm_Shuttle2_Pick[iOutArmYBase][iOutArmXBase];
    int iYPos               =Prod.YOutArm_Shuttle2_Pick[iOutArmYBase][iOutArmXBase];
//    int iMovePitchX         =iXpitchMaxX3;
    int iMovePitchY         =GetVariableYOutShuttleData();
    int iOffsetPos          =0;
    int iYVariable          =GetOutArmPitchY_9045(iMovePitchY, iOffsetPos);
    ZeroMemory(iXVariable,  sizeof(iXVariable));
    ZeroMemory(bZFlag,      sizeof(bZFlag));
    ZeroMemory(iZPos,       sizeof(iZPos));
    {
//#if 0   // GATE G7 -- golden `fNote->bMyServoOffOutArm` (note.h:417); the port TfNote carries bMyServoOffInArm only. Same #if 0-no-#else shape aoutarm9045.cpp:396-401 uses for these exact golden lines.   //AI(W906-FLOW-5) 20260929: gate RETIRED -- golden aoutarm.cpp:502-507 (MoveOutArmXY_ToShtMidArea, golden has no bAlarmNeedServoOff guard here either) restored; same reason as :914
        if(fNote->bMyServoOffOutArm)
        {
            return false;
        }
//#endif   //AI(W906-FLOW-5) 20260929: end of the retired GATE G7 (:1100)
    }

    iXPos=Tech.iOutArmShuttle1X;
    iYPos=Tech.iOutArmShuttle1Y+6500;

    bPickShuttleError=true;

    if(OutArmContinuousMove_9045(iXPos, iYPos, iXVariable, iYVariable, bZFlag, iZPos, false))
        return true;
    return false;
}
//==============================================================================
// 當抓TRAY臂通知其準備去移動手動平台,輸出臂回應其是否允許移動手動平台
//  Ver : 2003_07
//==============================================================================
bool IsCatchTrayReadySupplyNewTray()
{
    int iPosX, iPosY;
    if(iCatchTrayControlManual==0 &&                                            //When Now CatchTray can't Move
       bAtuoTrayICDetectErr==false)                                             //Isaac 20180109 (Steven) : auto123可前進後退
    {
        iPosX=MOT[MOutArmX].ReadPos();
        iPosY=MOT[MOutArmY].ReadPos();

        if(TRAY_ARM_MODE==eUnderCoveyor)                                        //Steven 20230611 : QQ 是否需要確認吸嘴位置?
        {
            iCatchTrayControlManual=1;
            return true;
        }

        if(bInstallRotate && bQualSiteQuickMode &&
           iPosY>=Prod.YOutArm_Shuttle1_Pick[iOutArmYBase][iOutArmXBase] &&
           iPosX<=(Prod.XOutArm_Shuttle1_Pick[iOutArmYBase][iOutArmXBase]+QUAD_SITE_X_PITCH+500))
        {
            iCatchTrayControlManual=1;
            return true;
        }

        if(iPosY>=Prod.YOutArm_Shuttle1_Pick[iOutArmYBase][iOutArmXBase] &&
           iPosX<=(Prod.XOutArm_Shuttle1_Pick[iOutArmYBase][iOutArmXBase]+QUAD_SITE_X_PITCH))
        {
            iCatchTrayControlManual=1;
            return true;
        }

        if(iPosY>=Prod.iOutArmSafeY &&
           MOT[MOutArmX].fCMD==false &&
           MOT[MOutArmY].fCMD==false)                                           //kevin 20180712 (wei) shuttle 1 位置 tray arm move
        {
            iCatchTrayControlManual=1;
            return true;
        }

        if(MOT[MOutArmY].CompareCommandPos(Prod.iOutArmSafeY, 2)==1 &&
           iPosX==-35000)
        {
            iCatchTrayControlManual=1;
            return true;
        }

        if(MOT[MOutArmY].CompareCommandPos(Prod.iOutArmSafeY, 2)==1 &&
           iPosX==-38000)
        {
            iCatchTrayControlManual=1;
            return true;
        }

        if(MOT[MOutArmY].CompareCommandPos(Prod.iOutArmSafeY, 2)==1 &&
           MOT[MOutArmX].CompareCommandPos(Prod.XOutArm_Shuttle1_Pick[iOutArmYBase][iOutArmXBase], 2)==1)
        {
            iCatchTrayControlManual=1;
            return true;
        }

        if(Cylinder[C_TrayCover].Enable==false                                 &&                                       //ChungHung 20150303 add for AutoRetest
            iPosY>=Prod.YOutArm_Shuttle1_Pick[iOutArmYBase][iOutArmXBase]-5000 &&
            MOT[MOutArmX].fCMD==false && MOT[MOutArmY].fCMD==false)
        {
            iCatchTrayControlManual=1;
            return true;
        }
    }
    return false;
}
//==============================================================================
// 檢查輸出臂啟始狀態
//  Ver : 2003_07
//==============================================================================
bool CheckOutArmInitState()
{
    return CheckOutArmInitState_9045();
}
//==============================================================================
//CheckBin用來把IC根據BIN放到對應的Tray盤，不可以亂改
//==============================================================================
int CheckBin(int &ct, int iShuttle)                                             //kevin 20160826 add shuttle //Steven 20110527 : 整合成只叫一次
{
    bool bShowMess=false;
    bool bAlarm=true;                                                           //kevin 20160906

    if(LastSet.iRealDummy==REALLY && LastSet.iTester==ON_LINE)                  //Steven 20150713 : 整理LastSet.iTester
    {
        if(iHWFix_BinBox==1)
        {
            if(Prod.iIfErrorT6!=eBulkBox)
            {
                bShowMess=true;
            }
        }
        else
        {
            if(Prod.iIfErrorT6==eBulkBox)
            {
                bShowMess=true;
            }
            else if(AUTO3_IS_MAGAZINE==1)                                       //JerryYang 20221215 : add Magazine
            {
                if(Prod.iIfErrorT6==iMagMin ||
                   Prod.iIfErrorT6>iMagMax)
                {
                    bShowMess=true;
                }
                else if(TestIF_File.iMagFixTrayType==1)                         //Fix盤當Magazine buffer模式
                {
                    if(Prod.iIfErrorT6>=iFixMin && Prod.iIfErrorT6<=iFixRightHalf)
                    {
                        bShowMess=true;
                    }
                }
                else
                {
                    if(TrayForm.iFixTrayMode==0)                                //Fix盤不分上下盤模式
                    {
                        if(Prod.iIfErrorT6>iFixRight && Prod.iIfErrorT6<=iFixRightHalf)
                        {
                            bShowMess=true;
                        }
                    }
                }
            }
            else if(Prod.iTrayType[Prod.iIfErrorT6]==tNotUse)                   //Steven 20240415 : 修正Error Bin設定到Fix6會跳alarm的問題
            {
                bShowMess=true;
            }
        }
    }

    ct=(ct>iTestBinCount-1)?Prod.iIfErrorT6:Prod.iT6CatData[ct];                //Steven 20121112 : RS232支援32Bin 14->iTestBinCount-1

    if(ct!=Prod.iIfErrorT6)                                                     //從不是Error Bin變成Error Bin就是式有問題
    {
        if(ct<eAuto1)
        {
            ct=Prod.iIfErrorT6;
        }
        else if(AUTO3_IS_MAGAZINE==1)                                           //JerryYang 20221215 : add Magazine
        {
            if(ct>iMagMax ||
               ct==iMagAtAuto)
            {
                ct=Prod.iIfErrorT6;
            }
            else if(TestIF_File.iMagFixTrayType==1)                             //Fix盤當Magazine buffer模式
            {
                if(ct>=iFixMin && ct<=iFixRightHalf)
                {
                    bShowMess=true;
                }
            }
            else if(TrayForm.iFixTrayMode==0)                                   //Fix盤不分上下盤模式
            {
                if(ct>iFixRight && ct<=iFixRightHalf)
                {
                    ct=Prod.iIfErrorT6;
                }
            }
            else
            {
            }
        }
        else if(TrayForm.iFixTrayMode==0)
        {
            if(ct>iFixRight)
            {
                ct=Prod.iIfErrorT6;
            }
        }
        else                                                                    //Fix tray上下盤
        {
            if(ct>iFixRightHalf)
            {
                ct=Prod.iIfErrorT6;
            }
        }

        if(LastSet.iRealDummy==REALLY &&
           LastSet.iTester==ON_LINE &&
           ct==Prod.iIfErrorT6)
        {
            bShowMess=true;
        }

        if(iHWFix_BinBox==1)                                                    //kevin 20160906
             bAlarm=false;
    }

    if(bShowMess && bAlarm)                                                     //kevin 20160906
    {
        if(bCancelErrorBin==1)                                                  //kevin 20160826
        {
            if(iShuttle==0)
                ShowErrorMessage("WAR07356", K_SKIP, MMInterface, false, sArm1BinError);
            else
                ShowErrorMessage("WAR07356", K_SKIP, MMInterface, false, sArm2BinError);
        }
        else
        {
            ShowErrorMessage("WAR07356", K_SKIP, MMInterface);
        }
    }

    if(USE_AUTO_RETEST==eartInstall     &&                                      //Steven 20161221 (wei) : Cate R for ART
       CosFunction.bUseSCKART           &&
       TestIF_File.iSCKART_SortMode==1  &&
       LastSet.iRunStartMode==rsmContinuStart_ART)
    {
        if(Prod.bCateRTo6Tray[ct])
            ct=BinSelect[iTestRunMode].iARTTrack;
    }

    return ct;
}
//==============================================================================
// 輸出臂從 Shuttle 1 吸取 IC 流程動作
//  Ver : 2003_07
//==============================================================================
void  InitPickFromShuttle1Task()
{
    bOutArmPickShtHasRetry=false;
    iPickFromShuttle1Task=1;
    iPickFromShuttle2Task=1;
    PreSetOutAdditionalFlag();                                                  //Steven 20220614 : Fixed for out arm additional function
}
//==============================================================================
void PickFromShuttle(int ibwhichKit)                                            //kevin 20210612 add int iwhichKit=0
{
    if(ArmSpeed[OutArm].dWaitOnSH!=0 && ibwhichKit==0)                          //kevin 20210525 out arm 起動在shuttle 上等待 時間
    {
        DoSHTWaitTime.SetSecAndOn(ArmSpeed[OutArm].dWaitOnSH);                  //kevin 20210525 out arm 在shuttle 上等待
    }
}
//==============================================================================
void  InitPickFromShuttle2Task()
{
    bOutArmPickShtHasRetry=false;
    iPickFromShuttle1Task=1;
    iPickFromShuttle2Task=1;
    PreSetOutAdditionalFlag();                                                  //Steven 20220614 : Fixed for out arm additional function
}
//==============================================================================
void InitPlaceToSortShtTask()                                                   //RogerYang 20250609 Add for 9046AU
{
    iPlaceToSortShtTask=1;
}
//==============================================================================
// 設定輸出臂放到 Auto Tray 流程動作
//==============================================================================
void InitPlaceToAutoTask()
{
    iPlaceToAutoTask=1;
}
//==============================================================================
// 設定輸出臂放到 Fix Tray 流程動作
//==============================================================================
void InitPlaceToFixTask()
{
    iPlaceToFixTask=1;
}
//------------------------------------------------------------------------------
void InitOutArmInspectTask()
{
    iOutArmInspectTask=1;
}
//==============================================================================
// 設定輸出臂主流程動作
//==============================================================================
void InitOutArmTask()
{
    OutArmTask=1;

    InitAOIFunction();                                                          //2014-03-04    Dell    for SPIL WLP Add 5S Inspection
}
//------------------------------------------------------------------------------
void InitOutArmPickFromAuto1Task()
{
    iOutArmPickFromAuto1Task=1;
}
//------------------------------------------------------------------------------
void DoOutArm_Scanner()
{
}
//------------------------------------------------------------------------------
void SetFixTrayFullIC()
{
    if(TrayForm.iFixTrayMode==0)
        return;
    for(int i=iFixMin; i<=iFixMax; i++)
    {
        if(TrayForm.bFixTrayLink[i])
            continue;
        if(MOT[iMMAuto[i]].HowManyDevice(NULL_IC)==0)                           //AI(general) 20260413 (RogerYang) : 已無空位不重複設置，避免 EventLog 爆量
            continue;
        if(CosFunction.bUseTrayUpDownSet)                                       //wei 20160224 TSMC FIX UPDOWN
        {
            if(TrayForm.bTrayUpDownSet[i])
            {
                if(MOT[iMMAuto[i]].UpHalfIsFull() ||
                   MOT[iMMAuto[i]].DownHalfIsFull())
                    ClearFixTray(i, "SetFixTrayFullIC", 3);                     //Steven 20160414 : 整合Fix盤設定
            }
            else                                                                //AI(general) 20260412 (RogerYang) : bTrayUpDownSet=false時用FullIC判斷整盤滿
            {
                if(MOT[iMMAuto[i]].FullIC())
                    ClearFixTray(i, "SetFixTrayFullIC", 3);
            }
        }
        else
        {
            if(MOT[iMMAuto[i]].UpHalfIsFull() ||
               MOT[iMMAuto[i]].DownHalfIsFull())
                ClearFixTray(i, "SetFixTrayFullIC", 3);                         //Steven 20160414 : 整合Fix盤設定
        }
    }
}
//-----------------------------------------------------------------------------
void SetFixTrayMiddleDtata()
{
    bool bFlag=true;
    int ct;                                                                     //JerryYang 20151119
    static bool bHasFixMode=false;
    int iRegion=5;

    if(AUTO3_IS_MAGAZINE==1 &&
       TestIF_File.iMagFixTrayType==1)                                          //JerryYang 20221215 : Magazine把fix區當buffer區功能
    {
        for(int i=2; i<10; i++)                                                 //QQQ
        {
            if(MOT[MManualTray1].Tray.YItem>=(i*iRegion)-1)
            {
                iYRegNum=i;                                                     //多少Y為一個區間
            }
            else
            {
                break;
            }
        }
    }

    for(int i=iFixMin; i<=iFixMax; i++)
    {
        if(AUTO3_IS_MAGAZINE==1 &&
           TestIF_File.iMagFixTrayType==1)                                      //JerryYang 20221215 : Magazine把fix區當buffer區功能
        {
            for(int iR=1; iR<MOT[iMMAuto[i]].Tray.YItem; iR++)
            {
                if(iR%iYRegNum==0)
                {
                    for(int iC=0; iC<MOT[iMMAuto[i]].Tray.XItem; iC++)
                    {
                        MOT[iMMAuto[i]].SetTraySingleData(iC, iR-1, HAS_NULL_IC);                                       //畫分隔線
                    }
                }
            }
        }
        else
        {
            if(TrayForm.iFixTrayMode!=0)
            {
                if(TrayForm.bFixTrayLink[i])
                    continue;

                if(AutoForm[i]->Direction<4)
                {
                    if(CosFunction.bUseTrayUpDownSet)                           //wei 20160224 TSMC FIX UPDOWN
                    {
                        if(TrayForm.bTrayUpDownSet[i])
                        {
                            if((MOT[iMMAuto[i]].Tray.YItem%2)==0)
                            {
                                ct=MOT[iMMAuto[i]].Tray.YItem-2;
                                ct/=2;
                                for(int iR=ct; iR<(ct+2); iR++)
                                {
                                    for(int iC=0; iC<MOT[iMMAuto[i]].Tray.XItem; iC++)
                                        MOT[iMMAuto[i]].SetTraySingleData(iC, iR, HAS_NULL_IC);
                                }
                            }
                            else
                            {
                                ct=MOT[iMMAuto[i]].Tray.YItem-1;
                                ct/=2;
                                for(int iR=ct; iR<(ct+1); iR++)
                                {
                                    for(int iC=0; iC<MOT[iMMAuto[i]].Tray.XItem; iC++)
                                        MOT[iMMAuto[i]].SetTraySingleData(iC, iR, HAS_NULL_IC);
                                }
                            }
                        }
                    }
                    else
                    {
                        if((MOT[iMMAuto[i]].Tray.YItem%2)==0)
                        {
                            ct=MOT[iMMAuto[i]].Tray.YItem-2;
                            ct/=2;
                            for(int iR=ct; iR<(ct+2); iR++)
                            {
                                for(int iC=0; iC<MOT[iMMAuto[i]].Tray.XItem; iC++)
                                    MOT[iMMAuto[i]].SetTraySingleData(iC, iR, HAS_NULL_IC);
                            }
                        }
                        else
                        {
                            ct=MOT[iMMAuto[i]].Tray.YItem-1;
                            ct/=2;
                            for(int iR=ct; iR<(ct+1); iR++)
                            {
                                for(int iC=0; iC<MOT[iMMAuto[i]].Tray.XItem; iC++)
                                    MOT[iMMAuto[i]].SetTraySingleData(iC, iR, HAS_NULL_IC);
                            }
                        }
                    }
                }
                else
                {
                    if((MOT[iMMAuto[i]].Tray.XItem%2)==0)
                    {
                        ct=MOT[iMMAuto[i]].Tray.XItem-2;
                        ct/=2;
                        for(int iC=ct; iC<(ct+2); iC++)
                        {
                            for(int iR=0; iR<MOT[iMMAuto[i]].Tray.YItem; iR++)
                                MOT[iMMAuto[i]].SetTraySingleData(iC, iR, HAS_NULL_IC);
                        }
                    }
                    else
                    {
                        ct=MOT[iMMAuto[i]].Tray.XItem-1;
                        ct/=2;
                        for(int iC=ct; iC<(ct+1); iC++)
                        {
                            for(int iR=0; iR<MOT[iMMAuto[i]].Tray.YItem; iR++)
                                MOT[iMMAuto[i]].SetTraySingleData(iC, iR, HAS_NULL_IC);
                        }
                    }
                }
                bHasFixMode=true;
            }
            else
            {
                if(bHasFixMode==true)
                {
                    bHasFixMode=false;
                    for(int iC=0; iC<MOT[iMMAuto[i]].Tray.XItem; iC++)
                    {
                        for(int iR=0; iR<MOT[iMMAuto[i]].Tray.YItem; iR++)
                        {
                            if(MOT[iMMAuto[i]].Tray.Data[iC][iR]==HAS_NULL_IC)
                                MOT[iMMAuto[i]].SetTraySingleData(iC, iR, NULL_IC);
                        }
                    }
                }
            }
        }
    }

    int iCol=MOT[iMMAuto[iFixRight]].Tray.XItem;                                //Steven 20091120 : For XItem is odd need -1
    if(USE_PICKER_COUNT==0)                                                     //Ifor 20170309 (wei) add HT9045S Short Shuttle IC 小於20mm Fix3 無法放滿
    {
        if(AutoForm[iFixRight]->XPitch<20.0)
        {
            iCol=iCol-1;
        }
    }
    else
    {
        if(iCol%2==0)
            iCol/=2;
        else
            iCol=(iCol-1)/2;
    }

    if(CosFunction.bUseFix3FullTray &&                                          //Ifor 20161208 add Fix3 Full Tray 放料順序 Direction>=4 有機會超出極限導致未滿盤被退盤
       FIX3_FULL_PLACE==Fix3K_Uninstall &&                                      //Steven 20250911 : for 硬體 fix3 full 可以關閉
       IniConfig.bE55UseFix3FullTray==true)
    {
        if(AutoForm[iFixRight]->Direction>=4 || iCol==2)                        //Ifor 20170112 修正 Fix3 滿盤功能 Aa 吸嘴無法放置X=2 導致退盤問題
            iCol=iCol-1;
    }

    bFlag=true;

    if(IniConfig.bFix3PutAllFullIC)                                             //ChungHung 20111215 嘗試將Fix3放滿
    {
        bFlag=false;
    }

    if(FIX3_FULL_PLACE==Fix3K_ShortShuttle &&                                   //Steven 20250814 : Fix3滿盤功能-馬達版+Sensor
       (Sen[SnFix3FullPlace].Enable &&
        Sen[SnFix3FullPlace].IsOn()))                                           //Steven 20121020 : Fix3滿盤
    {
        bFlag=false;
    }

    if(IniConfig.bBinBox==true &&                                               //jou 2012-12-11 support Bin Box
       AutoForm[iBinBoxAtFix]->iTrayType==iBinBoxType)
    {
        bFlag=false;
    }

    if(TestIF.iTestMode==SingleSite)                                            //2013-05-28    Dell    for Single site
    {
        bFlag=false;
    }

    if(EnableFix3UseCylinder())                                                 //ChungHung 20140313 add Fix3 can Full Tray   //ChungHung 20140722 add for HT9046LA
    {
        bFlag=false;
    }

    if(MachineTypeChoice==Type_HT9046_LS && FIX3_INSTALL==1)                    //Steven 20140626 : LS要Fix3滿盤
    {
        bFlag=false;
    }

    if(bFlag)
    {
        for(int iC=iCol; iC<MOT[MManualTray3].Tray.XItem; iC++)
        {
            for(int iR=0; iR<MOT[MManualTray3].Tray.YItem; iR++)
            {
                if(CosFunction.bUseFix3FullTray &&                              //Ifor 20161122 add 判斷是否有開啟 Fix3 Full Tray 功能
                   FIX3_FULL_PLACE==Fix3K_Uninstall &&                          //Steven 20250911 : for 硬體 fix3 full 可以關閉
                   IniConfig.bE55UseFix3FullTray==true)                         //Ifor 20161122 add Fix3 Full Tray 功能開啟，若有IC在預設放不到的位置上，不強制寫成HAS_NULL_IC
                {
                    if(MOT[MManualTray3].Tray.Data[iC][iR]!=HAS_IC)
                        MOT[MManualTray3].SetTraySingleData(iC, iR, HAS_NULL_IC);
                }
                else
                {
                    MOT[MManualTray3].SetTraySingleData(iC, iR, HAS_NULL_IC);
                }
            }
        }
    }

//    if((USE_Scanner_AOI_Inspection==true && ScannerAOIIF.iEnableScannerMode!=0) ||      //Ifor 20200108 add: Use Scan AOI
//       (USE_Top_Scanner_AOI_Inspection==true && ScannerAOIIF.iEnableTopScannerMode!=0)) //Ifor 20200902 add: TFAMD Top AOI
    if((USE_Scanner_AOI_Inspection==(int)eBtnAOI_BottomInstall && ScannerAOIIF.iEnableScannerMode!=0) ||      //Ifor 20200108 add: Use Scan AOI 需自動跳過破孔位置放IC
       (USE_Top_Scanner_AOI_Inspection==(int)eBtnAOI_BottomInstall && ScannerAOIIF.iEnableTopScannerMode!=0) ||                //Ifor 20200902 add: TFAMD Top AOI
#if 0   // GATE G9 -- golden `FrmAOI->RunTopBottomInspect()` (fAOI.h:437, TFrmAOI form, untranslated). THIRD disjunct only; the two Scanner-AOI disjuncts at golden :1065-:1066 stay LIVE.
        FrmAOI->RunTopBottomInspect()==true)                                                             //Jimmychiu 20240322 : Top & Bottom Inspect
#else
        false)                                                              //AI(W906-PT-W4-aoutarm) 20260808: G9 -- no AOI form => Top&Bottom inspect can never be running
#endif
    {
        for(int i=0; i<2; i++)
        {
            for(int iC=0; iC<MOT[iMMAuto[iMMAoi+i]].Tray.XItem; iC++)
            {
                for(int iR=0; iR<MOT[iMMAuto[iMMAoi+i]].Tray.YItem; iR++)
                {
                    if(MOT[iMMAuto[iMMAoi+i]].Tray.Data[iC][iR]==CLEAN_FINISH_IC)
                        MOT[iMMAuto[iMMAoi+i]].SetTraySingleData(iC, iR, NULL_IC);
                }
            }
        }

        for(int iC=0; iC<MOT[iMMAuto[iMMAoi]].Tray.XItem; iC++)
        {
            for(int iR=0; iR<MOT[iMMAuto[iMMAoi]].Tray.YItem; iR++)
            {
                if(MOT[MMScanAOI].Tray.Data[iC][iR]!=0)
                {
                    MOT[iMMAuto[iMMAoi]].SetTraySingleData(iC, iR, CLEAN_FINISH_IC);
                    MOT[iMMAuto[iMMAoi+1]].SetTraySingleData((MOT[iMMAuto[iMMAoi]].Tray.XItem-1)-iC, iR, CLEAN_FINISH_IC);
                }
            }
        }
    }

    if(CosFunction.bForceSetAuto3Fix3SideHasNullIC)                             //JerryYang 20151120 add for 京元Xilinx, IC 大於 40x40mm時會敲到,Auto3和Fix3最右邊要設為 HasNullIC
    {
        for(int iR=0; iR<MOT[iMMAuto[iFixMax]].Tray.YItem; iR++)
        {
            int iC=MOT[iMMAuto[iFixMax]].Tray.XItem-1;
            MOT[iMMAuto[iFixMax]].SetTraySingleData(iC, iR, HAS_NULL_IC);
        }

        if(AutoForm[iFixMax]->XPitch>=4000)                                     //Jou 20170505 (Steven) : 補卡40mm    //JerryYang 20180522 : 這個是device大小 40 mm
        {
            for(int iR=0; iR<MOT[iMMAuto[iMMAutoRight]].Tray.YItem; iR++)
            {
                int iC=MOT[iMMAuto[iMMAutoRight]].Tray.XItem-1;
                MOT[iMMAuto[iMMAutoRight]].SetTraySingleData(iC, iR, HAS_NULL_IC);
            }
        }
    }

    if(CUSTOMER_CODE==CC_KYEC_XILINX &&
       USE_ROTATE_KIT==1 &&
       BAR_CODE_INSTALL==ebctInShtIntel)                                        //wei 20161005
    {
        for(int iC=0; iC<MOT[MManualTray1].Tray.XItem; iC++)
        {
            for(int iR=0; iR<MOT[MManualTray1].Tray.YItem-1; iR++)
            {
                MOT[MManualTray1].SetTraySingleData(iC, iR, HAS_NULL_IC);
            }
        }
    }
}
//-----------------------------------------------------------------------------
void OutArmAddSpeed()
{
    if(AutoArmSpeed[OutArm].iBodySP<=(ArmSpeed[OutArm].iBodySP-5) ||
       AutoArmSpeed[OutArm].iACDCBodySP<=(ArmSpeed[OutArm].iACDCBodySP-5))
    {
        if(AutoArmSpeed[OutArm].iBodySP<=(ArmSpeed[OutArm].iBodySP-5))
            AutoArmSpeed[OutArm].iBodySP+=5;

        if(AutoArmSpeed[OutArm].iACDCBodySP<=(ArmSpeed[OutArm].iACDCBodySP-5))
            AutoArmSpeed[OutArm].iACDCBodySP+=5;

        W7A_OutArmSpeed_NoOp_PendingSafetyReview(true);           //AI(ht9045-v906) 20260810: PT-W7a -- was SetOutArmSpeed(true); see the stub note. Behaviour unchanged (no-op); restore this call when the safety review lands.
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

            W7A_OutArmSpeed_NoOp_PendingSafetyReview(true);           //AI(ht9045-v906) 20260810: PT-W7a -- was SetOutArmSpeed(true); see the stub note. Behaviour unchanged (no-op); restore this call when the safety review lands.
        }
    }
}
//-----------------------------------------------------------------------------
void OutArmSubSpeed()
{
    if(AutoArmSpeed[OutArm].iBodySP>25 ||
       AutoArmSpeed[OutArm].iACDCBodySP>25)
    {
        if(AutoArmSpeed[OutArm].iBodySP>25)
            AutoArmSpeed[OutArm].iBodySP-=2;

        if(AutoArmSpeed[OutArm].iACDCBodySP>25)
            AutoArmSpeed[OutArm].iACDCBodySP-=2;
        W7A_OutArmSpeed_NoOp_PendingSafetyReview(true);           //AI(ht9045-v906) 20260810: PT-W7a -- was SetOutArmSpeed(true); see the stub note. Behaviour unchanged (no-op); restore this call when the safety review lands.
    }
}
//-----------------------------------------------------------------------------
void initDoPlaceOutRotateTask()
{
    iPlaceToOutRotateTask=1;
}
//-----------------------------------------------------------------------------
bool DoPlaceOutRotate()
{
    return true;
}
//-----------------------------------------------------------------------------
bool NeedScan()
{
    /*if(BinSelect.iAllScan==0)
        return true;
    if(BinSelect.iScanInterval<=0)
        return true;
    iScanCount++;
    if(iScanCount>=BinSelect.iScanInterval )
    {
        iScanCount=0;
        return true;
    }
    return false; */
    return false;
}
//---------------------------------------------------------------------------
void DoOutArm()
{
//#if 0 // AI(W906-FLOW-2) 20260928: GATE G10 RETIRED -- golden aoutarm.cpp:1197-1198 runs again every DoOutArm tick. Old premise false: mykitsuck.cpp is registered (CMakeLists.txt:2003) and TMyKitSuck::SetHasNullIcToNullIc is defined at mykitsuck.cpp:574 (golden MyKitSuck.cpp:385, identical) on the ONE TMyKitSuck class (mykitsuck.h:274; FR/BRCarryKit defined aHotPlateSubstrate.cpp:86 with it; the aHotPlateSubstrate.h:371 mirror is #if 0 since A4-6).
    FRCarryKit.SetHasNullIcToNullIc();                                          //Steven 20160524 : ADD SetHasNullIcToNullIc
    BRCarryKit.SetHasNullIcToNullIc();                                          //Steven 20160524 : ADD SetHasNullIcToNullIc
//#endif // AI(W906-FLOW-2) 20260928: end of retired GATE G10 (:1800)

    for(int i=0; i<OutArmSuck.iShtRow; i++)
    {
        for(int j=0; j<OutArmSuck.iShtCol; j++)
        {
            if(FRCarryKit.Item[i][j]==HAS_IC || FRCarryKit.Item[i][j]==HAS_HOT_IC)
                FRCarryKit.Item[i][j]=(TEST_PASS+iTestBinCount);
            if(BRCarryKit.Item[i][j]==HAS_IC || BRCarryKit.Item[i][j]==HAS_HOT_IC)
                BRCarryKit.Item[i][j]=(TEST_PASS+iTestBinCount);
        }
    }
    SetFixTrayMiddleDtata();
    DoOutArm_9045();
    return;
}
//---------------------------------------------------------------------------
void SearchBinBoxUpDown(bool &bCanPick2ICAtOnceTime, int &iRow, int &iCol)      //Steven 20230601 : 重新整理Bin Box
{
    int iBackCol, iBackRow, iBackS;
    if(IniConfig.bBinBox)                                                       //jou 2012-12-11 support Bin Box
    {
        if(iWhichAuto==iBinBoxAtFix &&
           AutoForm[iBinBoxAtFix]->iTrayType==3 &&
           iOutArmPlaceOrder==0)
        {
            iBackCol=iCol;
            iBackRow=iRow;
            iBackS   =iOutArmXStep;

            for(int i=0; i<OutArmSuck.iPickRow; i++)
                for(int j=0; j<OutArmSuck.iPickCol; j++)
                    bOutArmSuckActiveBack[i][j]=bOutArmSuckActive[i][j];

            iOutArmPlaceOrder=1;
            Find_OutArm_PickerMaxUseCountOnTime(4, iCol, iRow);

            iCol=iBackCol;
            iRow=iBackRow;
            iOutArmXStep=iBackS;
            iOutArmPlaceOrder=0;

            for(int i=0; i<OutArmSuck.iPickRow; i++)
                for(int j=0; j<OutArmSuck.iPickCol; j++)
                    bOutArmSuckActive[i][j]=(bOutArmSuckActive[i][j] | bOutArmSuckActiveBack[i][j]);
        }
    }
}
//---------------------------------------------------------------------------
int Find_OutArm_PickerBufferMaxUseCountOnTime(int iUseSuck, int &iStep, int &iRow, int &iCol, bool bSearchLast)         //JerryYang 20221215 : Magazine把fix區當buffer區功能
{
//    int iTrayCol;
//    bool bBackupOutArmSuckActive[MAX_ARM_Row][MAX_ARM_Col]={{false,false,false,false},{false,false,false,false}};
//    int iBackupOutArmXPosition[MAX_ARM_Row][MAX_ARM_Col]={{0,0,0,0},{0,0,0,0}};
//    int iBackupOutArmYPosition;
//    int iCT,itStartCol, itStartRow;
//    int iMaxCT=0,isStartX=0;
//    int Motor,x,y;
//    int iBackix=0,iBackiy=0;
//    int ixStart,ixEnd,ixStep;
//    int iyStart,iyEnd,iyStep;
//    int Direction=0;
//    int i, j, i2;//, j2;
//    static int iRecY=0,iRecWhichAuto;
//    int isStartXTran[4]={0,1,1,1};
//
//    int Yitem=0;
//    int iRegion=5;  //Magazine 14bin, 1個fix盤5個bin
//    int iRx=0, iRy=0;
    return 0;
}
//---------------------------------------------------------------------------
int Find_OutArm_PickerMaxUseCountOnTime(int iUseSuck, int &iRow, int &iCol)
{
    int  iTrayCol;
    bool bBackupOutArmSuckActive[MAX_ARM_Row][MAX_ARM_Col]={{false, false, false, false}, {false, false, false, false}};
    int iBackupOutArmXPosition[MAX_ARM_Row][MAX_ARM_Col]={{0, 0, 0, 0}, {0, 0, 0, 0}};
    int iBackupOutArmYPosition;
    int iCT=0, itStartCol=0, itStartRow=0;
    int iMaxCT=0, isStartX=0;
    int Motor;
    int iBackix=0, iBackiy=0;
    int ixStart=0, ixEnd, ixStep;
    int iyStart=0, iyEnd, iyStep;
    int Direction;
    static int iRecY=0, iRecWhichAuto;
    int Yitem=0;
    int iRegion=5;                                                              //Magazine 14bin, 1個fix盤5個bin
    int iRy=0;
    bool bSearchLast=(OutArmSuck.iPickRow==2)?TestIF.bSearchLastMode:false;

    if(IniConfig.bBinBox)
    {
        if(iWhichAuto==iBinBoxAtFix &&
           AutoForm[iBinBoxAtFix]->iTrayType==3 &&
           iOutArmPlaceOrder==0)
        {
            bSearchLast=false;
        }
    }

    int iOutArmWhichAuto=iWhichAuto;                                            //JerryYang 20221215 : 改用iOutArmWhichAuto
    if(AUTO3_IS_MAGAZINE==1 && iWhichAuto==iMagAtAuto)                          //JerryYang 20221215 : iOutArmWhichAuto用來記錄要放到哪個Magazine
    {
        iOutArmWhichAuto=iMagMin+iWhichMag;
    }

    for(int i=0; i<OutArmSuck.iPickRow; i++)
    {
        for(int j=0; j<OutArmSuck.iPickCol; j++)
        {
            if(OutArmSuck.Item[i][j] &&
               OutArmSuck.iWhichAuto[i][j]==iOutArmWhichAuto)                   //JerryYang 20221215 : 改用iOutArmWhichAuto
            {
                isStartX=j;
                i=OutArmSuck.iMaxRow;
                j=OutArmSuck.iMaxCol;
                break;
            }
        }
    }

    if(AUTO3_IS_MAGAZINE==1             &&                                      //JerryYang 20221215 : Magazine把fix區當buffer區功能
       TestIF_File.iMagFixTrayType==1   &&
       iWhichAuto>=iMagMin)
    {
        Motor=iMMAuto[iWhichBuff];
        Direction=0;                                                            //Magazine buffer區不給設定方向
    }
    else
    {
        Motor=iMMAuto[iWhichAuto];
        Direction=AutoForm[iWhichAuto]->Direction;
    }

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
        else if(AUTO3_IS_MAGAZINE==1            &&                              //JerryYang 20221215 : Magazine把fix區當buffer區功能
                TestIF_File.iMagFixTrayType==1  &&
                iWhichAuto>=iMagMin             &&
                iWhichMag>=0                    &&
                MOT[Motor].Tray.YItem>2*iRegion-1)
        {
            iyStep=1;
            Yitem=MOT[Motor].Tray.YItem;

            for(int i=2; i<10; i++)
            {
                if(Yitem>=(i*iRegion)-1)
                {
                    iYRegNum=i;                                                 //多少Y為一個區間
                }
                else
                {
                    break;
                }
            }
            iRy=iWhichMag/3;
            iyStart=iYRegNum*iRy;
            iyEnd=iYRegNum*iRy+(iYRegNum-1);
        }
        else
        {
            if(TrayForm.iFixTrayMode==0 || iWhichAuto<iAutoCnt)
            {
                if(bSearchLast && iCloseSiteState==0)
                {
                    if(OutArmSuck.iPickRow==2)
                    {
                        if(MOT[Motor].HasIC()==false)
                        {
                            iyStart=0;
                            iRecY=0;
                        }
                        else
                        {
                            if(iOutArmPlaceOrder==1 && iRecWhichAuto==iWhichAuto)
                            {
                                iyStart=iRecY+(ChangeToFloatNonPcnt((double)(TestIF.iARM_Y_PITCH), (double)(AutoForm[iWhichAuto]->YPitch)));                    //ChungHung 20120505 9045WS
                                if(iyStart>=AutoForm[iWhichAuto]->YDivision)
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
            else
            {
                iyStep=1;
                if(iWhichAuto>iFixRight)
                {
                    if(Prod.bLinkTo6Tray[iWhichAuto])
                        iyStart=0;
                    else
                        iyStart=MOT[Motor].Tray.YItem/2+1;
                    iyEnd=MOT[Motor].Tray.YItem;
                }
                else
                {
                    iyStart=0;
                    if(Prod.bLinkTo6Tray[iWhichAuto])                           //QQQ
                    {
                        iyEnd=MOT[Motor].Tray.YItem;
                    }
                    else
                    {
                        if(CosFunction.bUseTrayUpDownSet)                       //wei 20160224 TSMC FIX UPDOWN
                        {
                            if(TrayForm.bTrayUpDownSet[iWhichAuto])
                                iyEnd=(MOT[Motor].Tray.YItem-1)/2;
                            else
                                iyEnd=MOT[Motor].Tray.YItem;
                        }
                        else
                        {
                            iyEnd=(MOT[Motor].Tray.YItem-1)/2;
                        }
                    }
                }
            }
        }
    }
    else if(Direction==1)
    {
        ixStart=MOT[Motor].Tray.XItem-1;
        ixEnd=-1;
        ixStep=-1;
        if(TrayForm.iFixTrayMode==0 || iWhichAuto<iAutoCnt)
        {
            iyStart=0;
            iyEnd=MOT[Motor].Tray.YItem;
            iyStep=1;
        }
        else
        {
            iyStep=1;
            if(iWhichAuto>iFixRight)
            {
                if(Prod.bLinkTo6Tray[iWhichAuto])
                    iyStart=0;
                else
                    iyStart=MOT[Motor].Tray.YItem/2+1;
                iyEnd=MOT[Motor].Tray.YItem;
            }
            else
            {
                iyStart=0;
                if(Prod.bLinkTo6Tray[iWhichAuto])
                {
                    iyEnd=MOT[Motor].Tray.YItem;
                }
                else
                {
                    if(CosFunction.bUseTrayUpDownSet)                           //wei 20160224 TSMC FIX UPDOWN
                    {
                        if(TrayForm.bTrayUpDownSet[iWhichAuto])
                            iyEnd=(MOT[Motor].Tray.YItem-1)/2;
                        else
                            iyEnd=MOT[Motor].Tray.YItem;
                    }
                    else
                    {
                        iyEnd=(MOT[Motor].Tray.YItem-1)/2;
                    }
                }
            }
        }
    }
    else if(Direction==2)
    {
        ixStart=0;
        ixEnd=MOT[Motor].Tray.XItem;
        ixStep=1;
        if(TrayForm.iFixTrayMode==0 || iWhichAuto<iAutoCnt)
        {
            iyStart=MOT[Motor].Tray.YItem-1;
            iyEnd=-1;
            iyStep=-1;
        }
        else
        {
            iyStep=-1;
            if(iWhichAuto>iFixRight)
            {
                if(Prod.bLinkTo6Tray[iWhichAuto])
                {
                    iyStart=MOT[Motor].Tray.YItem-1;
                    iyEnd=-1;
                }
                else
                {
                    iyEnd=MOT[Motor].Tray.YItem/2;
                    iyStart=MOT[Motor].Tray.YItem-1;
                }
            }
            else
            {
                iyStart=MOT[Motor].Tray.YItem-1;
                if(Prod.bLinkTo6Tray[iWhichAuto])
                {
                    iyStart=MOT[Motor].Tray.YItem-1;
                    iyEnd=-1;
                }
                else
                {
                    iyEnd=-1;
                    if(CosFunction.bUseTrayUpDownSet)                           //wei 20160224 TSMC FIX UPDOWN
                    {
                        if(TrayForm.bTrayUpDownSet[iWhichAuto])
                            iyStart=MOT[Motor].Tray.YItem/2-1;
                        else
                            iyStart=MOT[Motor].Tray.YItem-1;
                    }
                    else
                    {
                        iyStart=MOT[Motor].Tray.YItem/2-1;
                    }
                }
            }
        }
    }
    else
    {
        ixStart=MOT[Motor].Tray.XItem-1;
        ixEnd=-1;
        ixStep=-1;
        if(TrayForm.iFixTrayMode==0 || iWhichAuto<iAutoCnt)
        {
            iyStart=MOT[Motor].Tray.YItem-1;
            iyEnd=-1;
            iyStep=-1;
        }
        else
        {
            iyStep=-1;
            if(iWhichAuto>iFixRight)
            {
                if(Prod.bLinkTo6Tray[iWhichAuto-iFixRight])
                {
                    iyStart=MOT[Motor].Tray.YItem-1;
                    iyEnd=-1;
                }
                else
                {
                    iyEnd=MOT[Motor].Tray.YItem/2;
                    iyStart=MOT[Motor].Tray.YItem-1;
                }
            }
            else
            {
                iyStart=MOT[Motor].Tray.YItem-1;
                if(Prod.bLinkTo6Tray[iWhichAuto-iFixRight])
                {
                    iyStart=MOT[Motor].Tray.YItem-1;
                    iyEnd=-1;
                }
                else
                {
                    iyEnd=-1;
                    if(CosFunction.bUseTrayUpDownSet)                           //wei 20160224 TSMC FIX UPDOWN
                    {
                        if(TrayForm.bTrayUpDownSet[iWhichAuto])
                            iyStart=MOT[Motor].Tray.YItem/2-1;
                        else
                            iyStart=MOT[Motor].Tray.YItem-1;
                    }
                    else
                    {
                        iyStart=MOT[Motor].Tray.YItem/2-1;
                    }
                }
            }
        }
    }

    iOutArmYStep=ChangeToFloatNonPcnt((double)(AutoCalculateOutArmYClosePitch(iWhichAuto)), (double)(AutoForm[iWhichAuto]->YPitch));                            //Steven 20230523 : LoadForm->YPitch --> AutoForm[iWhichAuto]->YPitch

    if(TRAY_ARM_MODE==eUnderCoveyor &&
       (MOT[Motor].fHasTray==false ||
        MOT[Motor].FullIC()==true))
    {
        for(int iC=isStartX; iC<OutArmSuck.iPickCol; iC++)
        {
            iTrayCol=iC-isStartX;
            iTrayCol=itStartCol+iTrayCol*iOutArmXStep;
            if(ArmSpeed_File[OutArm].bYPitchUseSearchLastMode && iOutArmPlaceOrder==0 &&
               UnloaderTrayYDirForArmYPitch(iWhichAuto) &&
               (itStartRow+iOutArmYStep<AutoForm[iWhichAuto]->YDivision) &&     //JerryYang 20221128 : 避免Y超出Tray盤
               (itStartRow+iOutArmYStep<iyEnd))                                 //JerryYang 20181112 (Steven) : ATP要求修改unloader擺放方式，盡量不要有空格
            {
                if(OutArmSuck.Item[0][iC]!=NULL_IC &&
                   OutArmSuck.iWhichAuto[0][iC]==iOutArmWhichAuto &&            //JerryYang 20221215 : 改用iOutArmWhichAuto
                   iTrayCol<AutoForm[iWhichAuto]->XDivision)
                {
                    iCT++;
                    bOutArmSuckActive[0][iC]=true;
                    iOutArmXPosition[0][iC]=iTrayCol;
                    bBackupOutArmSuckActive[0][iC]=true;
                    iBackupOutArmXPosition[0][iC]=iTrayCol;
                    iOutArmYPosition=iRow;
                }

                if(OutArmSuck.Item[1][iC]!=NULL_IC &&
                   OutArmSuck.iWhichAuto[1][iC]==iOutArmWhichAuto &&            //JerryYang 20221215 : 改用iOutArmWhichAuto
                   iTrayCol<AutoForm[iWhichAuto]->XDivision)
                {
                    iCT++;
                    bOutArmSuckActive[1][iC]=true;
                    iOutArmXPosition[1][iC]=iTrayCol;
                    bBackupOutArmSuckActive[1][iC]=true;
                    iBackupOutArmXPosition[1][iC]=iTrayCol;
                    iOutArmYPosition=iRow;
                }
            }
            else
            {
                if(OutArmSuck.Item[iOutArmPlaceOrder][iC]!=NULL_IC &&
                   OutArmSuck.iWhichAuto[iOutArmPlaceOrder][iC]==iOutArmWhichAuto &&
                   iTrayCol<AutoForm[iWhichAuto]->XDivision)
                {
                    iCT++;
                    bOutArmSuckActive[iOutArmPlaceOrder][iC]=true;
                    iOutArmXPosition[iOutArmPlaceOrder][iC]=iTrayCol;
                    bBackupOutArmSuckActive[iOutArmPlaceOrder][iC]=true;
                    iBackupOutArmXPosition[iOutArmPlaceOrder][iC]=iTrayCol;
                    iOutArmYPosition=iRow;
                }
            }
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
                    for(int i=0; i<OutArmSuck.iMaxRow; i++)
                        for(int j=0; j<OutArmSuck.iMaxCol; j++)
                            bOutArmSuckActive[i][j]=false;
                    iCT=0;

                    for(int j=isStartX; j<OutArmSuck.iPickCol; j++)
                    {
                        iTrayCol=j-isStartX;
                        iTrayCol=itStartCol+iTrayCol*iOutArmXStep;
                        if(ArmSpeed_File[OutArm].bYPitchUseSearchLastMode && iOutArmPlaceOrder==0 &&
                           UnloaderTrayYDirForArmYPitch(iWhichAuto) &&
                           (itStartRow+iOutArmYStep<MOT[Motor].Tray.YItem) &&   //JerryYang 20221128 : 避免Y超出Tray盤
                           (itStartRow+iOutArmYStep<iyEnd))                     //JerryYang 20181112 (Steven) : ATP要求修改unloader擺放方式，盡量不要有空格
                        {
                            if(OutArmSuck.Item[0][j]!=NULL_IC &&
                               OutArmSuck.iWhichAuto[0][j]==iOutArmWhichAuto &&                                         //JerryYang 20221215 : 改用iOutArmWhichAuto
                               iTrayCol<MOT[Motor].Tray.XItem &&
                               MOT[Motor].Tray.Data[iTrayCol][itStartRow]==NULL_IC)
                            {
                                iCT++;
                                bOutArmSuckActive[0][j]=true;
                                iOutArmXPosition[0][j] =iTrayCol;
                                iOutArmYPosition=iRow;
                            }

                            if(OutArmSuck.Item[1][j]!=NULL_IC &&
                               OutArmSuck.iWhichAuto[1][j]==iOutArmWhichAuto &&                                         //JerryYang 20221215 : 改用iOutArmWhichAuto
                               iTrayCol<MOT[Motor].Tray.XItem &&
                               MOT[Motor].Tray.Data[iTrayCol][itStartRow+iOutArmYStep]==NULL_IC)
                            {
                                iCT++;
                                bOutArmSuckActive[1][j]=true;
                                iOutArmXPosition[1][j] =iTrayCol;
                                iOutArmYPosition=iRow;
                            }
                        }
                        else
                        {
                            if(OutArmSuck.Item[iOutArmPlaceOrder][j]!=NULL_IC &&
                               OutArmSuck.iWhichAuto[iOutArmPlaceOrder][j]==iOutArmWhichAuto &&
                               iTrayCol<MOT[Motor].Tray.XItem &&
                               MOT[Motor].Tray.Data[iTrayCol][itStartRow]==NULL_IC)
                            {
                                iCT++;
                                bOutArmSuckActive[iOutArmPlaceOrder][j]=true;
                                iOutArmXPosition[iOutArmPlaceOrder][j] =iTrayCol;
                                iOutArmYPosition=iRow;
                            }
                        }
                    }

                    if(iCT>iMaxCT)
                    {
                        iMaxCT=iCT;
                        if(iOutArmPlaceOrder==0 &&
                           UnloaderTrayYDirForArmYPitch(iWhichAuto))            //ChungHung 20131231 alter AutoYPitch
                        {
                            for(int i=0; i<OutArmSuck.iPickRow; i++)
                            {
                                for(int j=0; j<OutArmSuck.iPickCol; j++)
                                {
                                    bBackupOutArmSuckActive[i][j]=bOutArmSuckActive[i][j];
                                    iBackupOutArmXPosition[i][j] =iOutArmXPosition[i][j];
                                }
                                iBackupOutArmYPosition=iOutArmYPosition;
                                iBackix=iCol;
                                iBackiy=iRow;
                            }
                        }
                        else
                        {
                            for(int j=0; j<OutArmSuck.iPickCol; j++)
                            {
                                bBackupOutArmSuckActive[iOutArmPlaceOrder][j]=bOutArmSuckActive[iOutArmPlaceOrder][j];
                                iBackupOutArmXPosition[iOutArmPlaceOrder][j] =iOutArmXPosition[iOutArmPlaceOrder][j];
                            }
                            iBackupOutArmYPosition=iOutArmYPosition;
                            iBackix=iCol;
                            iBackiy=iRow;
                        }
                    }
                }
            }
        }
    }

    for(int i=0; i<OutArmSuck.iPickCol; i++)
    {
        if(iOutArmPlaceOrder==0 && UnloaderTrayYDirForArmYPitch(iWhichAuto))    //ChungHung 20131231 alter AutoYPitch
        {
            bOutArmSuckActive[0][i]=bBackupOutArmSuckActive[0][i];
            iOutArmXPosition[0][i] =iBackupOutArmXPosition[0][i];
            bOutArmSuckActive[1][i]=bBackupOutArmSuckActive[1][i];
            iOutArmXPosition[1][i] =iBackupOutArmXPosition[1][i];
        }
        else
        {
            bOutArmSuckActive[iOutArmPlaceOrder][i]=bBackupOutArmSuckActive[iOutArmPlaceOrder][i];
            iOutArmXPosition[iOutArmPlaceOrder][i] =iBackupOutArmXPosition[iOutArmPlaceOrder][i];
        }
    }
    iOutArmYPosition=iBackupOutArmYPosition;
    iCol=iBackix;
    iRow=iBackiy;

    if(bSearchLast && iCloseSiteState==0)
    {
        if(OutArmSuck.iPickRow==2)
        {
            if(iOutArmPlaceOrder==0)
            {
                iRecY=iRow;
                iRecWhichAuto=iWhichAuto;
            }
        }
    }

    return iMaxCT;
}
//------------------------------------------------------------------------------
void Find_OutArm_Single(int iUseSuck, int &iRow, int &iCol)
{
    int itStartCol, itStartRow;
    int isStartCol;
    int Motor;
    int ixStart, ixEnd, ixStep;
    int iyStart, iyEnd, iyStep;
    int Direction=0;
    int serchtable[4]={0, 1, 2, 3};
    int Yitem=0;
    int iRegion=5;                                                              //Magazine 14bin, 1個fix盤5個bin
    int iRy=0;

    int iOutArmWhichAuto=iWhichAuto;                                            //JerryYang 20221215 : 改用iOutArmWhichAuto

    if(AUTO3_IS_MAGAZINE==1 && iWhichAuto==iMagAtAuto)                          //JerryYang 20221215 : 改用iOutArmWhichAuto
    {
        iOutArmWhichAuto=iMagMin+iWhichMag;
        if(TestIF_File.iMagFixTrayType==1)                                      //JerryYang 20221215 : Magazine把fix區當buffer區功能
            Motor=iMMAuto[iWhichBuff];
        else
            Motor=iMMAuto[iWhichAuto];
    }
    else
    {
        Motor=iMMAuto[iWhichAuto];
    }

    itStartCol=-1;
    itStartRow=-1;

    for(int i=0; i<OutArmSuck.iMaxRow; i++)
        for(int j=0; j<OutArmSuck.iMaxCol; j++)
            bOutArmSuckActive[i][j]=false;

    if(IniConfig.bI37_EnableFIFOMode && LastSet.iRunStartMode==rsmFIFOMode)     //Steven 20160303 : FIFO Mode
    {
        for(int i=0; i<OutArmSuck.iMaxRow; i++)
        {
            for(int j=0; j<OutArmSuck.iMaxCol; j++)
            {
                if(OutArmSuck.Item[i][j]!=NULL_IC)
                {
                    bOutArmSuckActive[i][j]=true;
                    iCol=OutArmSuck.PordRec[i][j].GetLoaderX();
                    iRow=OutArmSuck.PordRec[i][j].GetLoaderY();
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

    if(AUTO3_IS_MAGAZINE==1             &&                                      //JerryYang 20221215 : Magazine把fix區當buffer區功能
       TestIF_File.iMagFixTrayType==1   &&
       iWhichAuto>=iMagMin)
    {
        Direction=0;                                                            //Magazine buffer不給設定
    }
    else
    {
        if(iHWFix_BinBox==1 && iWhichAuto==eBulkBox)                            //kevin 20160819
            Direction=1;
        else
            Direction=AutoForm[iWhichAuto]->Direction;
    }

    if(IniConfig.bFix3PutAllFullIC)                                             //ChungHung 20111215 嘗試將Fix3放滿
    {
        if(iWhichAuto==iFixRight)
        {
            Direction=1;
        }
    }

    if(IniConfig.bUseTrayBlockMode   &&                                         //2014-03-04    Dell    for SPIL WLP Add Tray Block
       ((AutoForm[0]->BlockNumberX>0) ||
        (AutoForm[0]->BlockNumberY>0)))                                         //Steven 20150702 : Fixed 分Tray問題
        TrayForm.iFixTrayMode=0;

    if(IniConfig.bUseTrayBlockMode &&
       IniConfig.bP06_LoaderUseCarrierTray==true &&
       ((AutoForm[0]->BlockXItem>0) || (AutoForm[0]->BlockYItem>0)))            //Frank 20160928 add Subtray Function
        TrayForm.iFixTrayMode=0;

    if(Direction==0 || Direction==4)                                            //左上到右下                            //Steven 20141118 : Add Out Arm Direction 4~7
    {
        ixStart=0;
        ixEnd=MOT[Motor].Tray.XItem;
        ixStep=1;
        if(AUTO3_IS_MAGAZINE==1             &&                                  //JerryYang 20221215 : Magazine把fix區當buffer區功能
           TestIF_File.iMagFixTrayType==1   &&
           iWhichAuto>=iMagAtAuto           &&
           iWhichMag>=0                     &&
           MOT[Motor].Tray.YItem>2*iRegion-1)
        {
            iyStep=1;
            Yitem=MOT[Motor].Tray.YItem;

            for(int i=2; i<10; i++)
            {
                if(Yitem>=(i*iRegion)-1)
                {
                    iYRegNum=i;                                                 //多少Y為一個區間
                }
                else
                {
                    break;
                }
            }
            iRy=iWhichMag/3;
            iyStart=iYRegNum*iRy;
            iyEnd=iYRegNum*iRy+(iYRegNum-1);
        }
        else if(TrayForm.iFixTrayMode==0  || iWhichAuto<iAutoCnt)
        {
            iyStart=0;
            iyEnd=MOT[Motor].Tray.YItem;
            iyStep=1;
            if(IniConfig.bUseTrayBlockMode &&                                   //Frank 20160928 add Subtray Function
               IniConfig.bP06_LoaderUseCarrierTray==true)                       //Ifor 20161129 add P06 判斷避免OutArm 一顆一顆放料
            {
                int BX, BY, BlockTotal;
                bool bBlockEmpty = true;
                if((AutoForm[iWhichAuto]->BlockXItem>0) ||
                   (AutoForm[iWhichAuto]->BlockYItem>0))
                {
                    BX=ChangeToFloatNonPcnt((double)(MOT[Motor].Tray.XItem), (double)(AutoForm[iWhichAuto]->BlockXItem));
                    BY=ChangeToFloatNonPcnt((double)(MOT[Motor].Tray.YItem), (double)(AutoForm[iWhichAuto]->BlockYItem));
                    BlockTotal = AutoForm[iWhichAuto]->BlockXItem*AutoForm[iWhichAuto]->BlockYItem;
                    iOutArmBlockIndex =0;
                    for(int k=0; k<BlockTotal; k++)
                    {
                        ixEnd=BX+(iOutArmBlockIndex%AutoForm[iWhichAuto]->BlockXItem*BX);
                        iyEnd=BY+(ChangeToFloatNonPcnt((double)(iOutArmBlockIndex), (double)(AutoForm[iWhichAuto]->BlockXItem))*BY);

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
                            iOutArmBlockIndex=k+1;
                            ixEnd=BX+(iOutArmBlockIndex%AutoForm[iWhichAuto]->BlockXItem*BX);
                            iyEnd=BY+(ChangeToFloatNonPcnt((double)(iOutArmBlockIndex), (double)(AutoForm[iWhichAuto]->BlockXItem))*BY);
                        }
                        else
                        {
                            break;
                        }
                    }
                }
            }
        }
        else
        {
            iyStep=1;
            if(iWhichAuto>iFixRight)
            {
                if(TrayForm.bFixTrayLink[iWhichAuto])
                    iyStart=0;
                else
                    iyStart=MOT[Motor].Tray.YItem/2+1;
                iyEnd=MOT[Motor].Tray.YItem;
            }
            else
            {
                iyStart=0;
                if(TrayForm.bFixTrayLink[iWhichAuto])
                {
                    iyEnd=MOT[Motor].Tray.YItem;
                }
                else
                {
                    if(CosFunction.bUseTrayUpDownSet)                           //wei 20160224 TSMC FIX UPDOWN
                    {
                        if(TrayForm.bTrayUpDownSet[iWhichAuto])
                            iyEnd=(MOT[Motor].Tray.YItem-1)/2;
                        else
                            iyEnd=MOT[Motor].Tray.YItem;
                    }
                    else
                    {
                        iyEnd=(MOT[Motor].Tray.YItem-1)/2;
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
        if(TrayForm.iFixTrayMode==0 || iWhichAuto<iAutoCnt)
        {
            iyStart=0;
            iyEnd=MOT[Motor].Tray.YItem;
            iyStep=1;
        }
        else
        {
            iyStep=1;
            if(iWhichAuto>iFixRight)
            {
                if(TrayForm.bFixTrayLink[iWhichAuto])
                    iyStart=0;
                else
                    iyStart=MOT[Motor].Tray.YItem/2+1;
                iyEnd=MOT[Motor].Tray.YItem;
            }
            else
            {
                iyStart=0;
                if(TrayForm.bFixTrayLink[iWhichAuto])
                {
                    iyEnd=MOT[Motor].Tray.YItem;
                }
                else
                {
                    if(CosFunction.bUseTrayUpDownSet)                           //wei 20160224 TSMC FIX UPDOWN
                    {
                        if(TrayForm.bTrayUpDownSet[iWhichAuto])
                            iyEnd=(MOT[Motor].Tray.YItem-1)/2;
                        else
                            iyEnd=MOT[Motor].Tray.YItem;
                    }
                    else
                    {
                        iyEnd=(MOT[Motor].Tray.YItem-1)/2;
                    }
                }
            }
        }
    }
    else if(Direction==2 || Direction==5)                                       //左下到右上
    {
        ixStart=0;
        ixEnd=MOT[Motor].Tray.XItem;
        ixStep=1;
        if(TrayForm.iFixTrayMode==0 || iWhichAuto<iAutoCnt)
        {
            iyStart=MOT[Motor].Tray.YItem-1;
            iyEnd=-1;
            iyStep=-1;
        }
        else
        {
            iyStep=-1;
            if(iWhichAuto>iFixRight)
            {
                if(TrayForm.bFixTrayLink[iWhichAuto])
                {
                    iyStart=MOT[Motor].Tray.YItem-1;
                    iyEnd=-1;
                }
                else
                {
                    iyEnd=MOT[Motor].Tray.YItem/2;
                    iyStart=MOT[Motor].Tray.YItem-1;
                }
            }
            else
            {
                iyStart=MOT[Motor].Tray.YItem-1;
                if(TrayForm.bFixTrayLink[iWhichAuto])
                {
                    iyStart=MOT[Motor].Tray.YItem-1;
                    iyEnd=-1;
                }
                else
                {
                    iyEnd=-1;
                    if(CosFunction.bUseTrayUpDownSet)                           //wei 20160224 TSMC FIX UPDOWN
                    {
                        if(TrayForm.bTrayUpDownSet[iWhichAuto])
                            iyStart=MOT[Motor].Tray.YItem/2-1;
                        else
                            iyStart=MOT[Motor].Tray.YItem-1;
                    }
                    else
                    {
                        iyStart=MOT[Motor].Tray.YItem/2-1;
                    }
                }
            }
        }
    }
    else                                                                        //右下到左上
    {
        ixStart=MOT[Motor].Tray.XItem-1;
        ixEnd=-1;
        ixStep=-1;
        if(TrayForm.iFixTrayMode==0 || iWhichAuto<iAutoCnt)
        {
            iyStart=MOT[Motor].Tray.YItem-1;
            iyEnd=-1;
            iyStep=-1;
        }
        else
        {
            iyStep=-1;
            if(iWhichAuto>iFixRight)
            {
                if(TrayForm.bFixTrayLink[iWhichAuto])
                {
                    iyStart=MOT[Motor].Tray.YItem-1;
                    iyEnd=-1;
                }
                else
                {
                    iyEnd=MOT[Motor].Tray.YItem/2;
                    iyStart=MOT[Motor].Tray.YItem-1;
                }
            }
            else
            {
                iyStart=MOT[Motor].Tray.YItem-1;
                if(TrayForm.bFixTrayLink[iWhichAuto])
                {
                    iyStart=MOT[Motor].Tray.YItem-1;
                    iyEnd=-1;
                }
                else
                {
                    iyEnd=-1;

                    if(CosFunction.bUseTrayUpDownSet)                           //wei 20160224 TSMC FIX UPDOWN
                    {
                        if(TrayForm.bTrayUpDownSet[iWhichAuto])
                            iyStart=MOT[Motor].Tray.YItem/2-1;
                        else
                            iyStart=MOT[Motor].Tray.YItem-1;
                    }
                    else
                    {
                        iyStart=MOT[Motor].Tray.YItem/2-1;
                    }
                }
            }
        }
    }

    if(FIX3_FULL_PLACE==Fix3K_Uninstall &&
       Motor==MManualTray3)                                                     //kevin 20150713 fix3 由右到左放
    {
        ixStart=MOT[Motor].Tray.XItem-1;
        ixEnd=0;
        ixStep=1;

        for(int iR=iyStart; iR!=iyEnd; iR+=iyStep)
        {
            for(int iC=ixStart; iC>=ixEnd; iC-=ixStep)
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
    else if(Direction<4)                                                        //橫排先放
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

    if(iHWFix_BinBox==1 && iWhichAuto==eBulkBox)                                //kevin 20160826
    {
        for(int i=0; i<OutArmSuck.iPickRow; i++)
        {
            for(int j=0; j<OutArmSuck.iPickCol; j++)
            {
                isStartCol=serchtable[j];
                if(OutArmSuck.Item[i][j]!=NULL_IC &&
                   OutArmSuck.iWhichAuto[i][j]==iWhichAuto)
                {
                    bOutArmSuckActive[i][j]=true;
                    iCol=0;
                    iRow=0;
                    return ;
                }
            }
        }
    }
    else if(iWhichAuto==iAutoRight ||
            iWhichAuto==iFixRight  ||
            iWhichAuto==iFixRightHalf)                                          //near right side
    {
        for(int i=0; i<OutArmSuck.iPickRow; i++)
        {
            for(int j=OutArmSuck.iPickCol-1; j>=0; j--)
            {
                isStartCol=serchtable[j];
                if(OutArmSuck.Item[i][isStartCol]!=NULL_IC &&
                   OutArmSuck.iWhichAuto[i][isStartCol]==iOutArmWhichAuto)      //JerryYang 20221215 : 改用iOutArmWhichAuto
                {
                    bOutArmSuckActive[i][isStartCol]=true;
                    iCol=itStartCol;
                    iRow=itStartRow;
                    return ;
                }
                else if(AUTO3_IS_MAGAZINE==1)                                   //JerryYang 20250920 : fixed for Multi bin
                {
                    int iTempWhichMag=-1;
                    int iMagLink=-1;
                    iTempWhichMag=OutArmSuck.iWhichAuto[i][isStartCol]-eMag1;
                    if(MOT[MMMagazineTary1+iTempWhichMag].Tray.FullIC()==true)
                    {
                        for(int l=1; l<14; l++)
                        {
                            if(BinSelect[iTestRunMode].bMagazineLink[iTempWhichMag+l]==true)
                            {
                                if(MOT[MMMagazineTary1+iTempWhichMag+l].Tray.FullIC()==false)
                                {
//                                    iMagLink=iTempWhichMag+l;
                                    iMagLink=l;                                 //Ifor 20240222 fix Hang up 問題
                                    break;
                                }
                            }
                        }

                        if(OutArmSuck.Item[i][isStartCol]!=NULL_IC && (OutArmSuck.iWhichAuto[i][isStartCol]+iMagLink)==iOutArmWhichAuto)
                        {
                            bOutArmSuckActive[i][isStartCol]=true;
                            iCol=itStartCol;
                            iRow=itStartRow;
                            return ;
                        }
                    }
                }
            }
        }
    }
    else
    {
        for(int i=0; i<OutArmSuck.iPickRow; i++)
        {
            for(int j=0; j<OutArmSuck.iPickCol; j++)
            {
                isStartCol=serchtable[j];
                if(OutArmSuck.Item[i][j]!=NULL_IC &&
                   OutArmSuck.iWhichAuto[i][j]==iOutArmWhichAuto)               //JerryYang 20221215 : 改用iOutArmWhichAuto
                {
                    bOutArmSuckActive[i][j]=true;
                    iCol=itStartCol;
                    iRow=itStartRow;
                    return ;
                }
            }
        }
    }
}
//------------------------------------------------------------------------------
void AdjustOutArmClosePitchCondition(int iAuto, bool &bCanPick2ICAtOnceTime)
{
    int iResult=0;
    bCanPick2ICAtOnceTime=false;
#if 0   // GATE G14 -- golden `fSCKART->CheckOutArmNeedVariModeFIX(iAuto)`. The BODY is really translated (Automation/SCK_ART.cpp:373) but it takes `SckArtState&`, and forms/fSCKART.h:74 `class TfSCKART` does NOT derive from SckArtState -- there is no SckArtState object anywhere in this tree to pass, and forms/fSCKART.h:63 explicitly forbids inventing another declaration site. See GATE REGISTER G14.
    fSCKART->CheckOutArmNeedVariModeFIX(iAuto);                                 //Sam 20211108 : TPW OutArm 也要一顆一顆放
#endif

    if(IniConfig.bI37_EnableFIFOMode &&
       IniConfig.bI37_EnableFIFOSiteOrder &&
       LastSet.iRunStartMode==rsmFIFOMode)                                      //Frank 20180810 (Steven) : add FIFO By Tester SiteMap
    {
        bCanPick2ICAtOnceTime=false;
        iOutArmXStep=1;
        return;
    }

    if(IniConfig.bI33ErrorBinBox &&
       iHWFix_BinBox==1 &&
       iAuto==eBulkBox)
    {
        bCanPick2ICAtOnceTime=false;
        iOutArmXStep=1;
        return;
    }

    if(IniConfig.bFix3PutAllFullIC)                                             //ChungHung 20111215 嘗試將Fix3放滿
    {
        if(iAuto==iFixRight || iAuto==iFixRightHalf)                            //KaiChen 20200116 ：修正 Fix3切上下盤時擺放異常
        {
            bCanPick2ICAtOnceTime=false;
            iOutArmXStep=1;
            return;
        }
    }

    iResult=AutoCalculateOutArmXClosePitch(iAuto);
    if(ArmSpeed[OutArm].bVariModeFIX==false && !bUseOnebyOne)                   //jou 2010-09-07 當clean out時,out arm排列自動改為One by one排列
    {
        if(iResult==-1)
        {
            bCanPick2ICAtOnceTime=false;
        }
        else if(CUSTOMER_CODE==CC_JSCC_OS)                                      //RogerYang 20260210 : 田揚志需求，有設定的才可以一次放全部，OS報表要照Tray盤放料順序顯示
        {
#if 0   // GATE G11 -- golden `fTesterTCP->bOutArmVariAuto_OS[]` (Interface/TesterTCP.h:261, a TfTesterTCP form member; this port has no TfTesterTCP class and no fTesterTCP global).
            bCanPick2ICAtOnceTime=fTesterTCP->bOutArmVariAuto_OS[iAuto];
#else
            bCanPick2ICAtOnceTime=false;                                    //AI(W906-PT-W4-aoutarm) 20260808: G11 -- default-cleared per-Auto OS checkbox => place one IC at a time (CC_JSCC_OS branch only)
#endif
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
bool CheckOutSuckICFallDown(bool AutoClean)
{
    #ifdef SOFT_SIMULTE
        return false;
    #else
    {
        int ret, iMotZ;
        AnsiString ErrPart="", Str;                                             //Steven 20110216 : 合併Alarm
        bool bHasDuplicateErr=false;                                            //Steven 20110216 : 合併Alarm
        bool bHasErr=false;                                                     //Steven 20110216 : 合併Alarm
        static bool bSuckErr[MAX_ARM_Row][MAX_ARM_Col]={{false, false, false, false}, {false, false, false, false}};    //Steven 20110216 : Avoid duplicate message
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
                                OutArmSuck.PordRec[i][j].AddErrorRecord("JAM0203", true, iXPos, iYPos);                 //Steven 20160114 (jou) : Add Production Error Log
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
                                OutArmSuck.PordRec[i][j].AddErrorRecord("JAM0203", true, iXPos, iYPos);                 //Steven 20160114 (jou) : Add Production Error Log
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
                                OutArmSuck.PordRec[i][j].AddErrorRecord("JAM0203", true, iXPos, iYPos);                 //Steven 20160114 (jou) : Add Production Error Log
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
                                OutArmSuck.PordRec[i][j].AddErrorRecordNoSave("JAM0203", false, iXPos, iYPos);          //Steven 20160114 (jou) : Add Production Error Log //Steven 20190110 : Error Log Add Motor Position.
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
                RecordProcess("CheckOutSuckICFallDown JAM0203 - Picker Position", Str);                                 //Steven 20210929 : 紀錄out arm掉料發生時, 吸嘴的位置

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
                                OutArmSuck.PordRec[i][j].AddErrorRecord("JAM0203", true, iXPos, iYPos);                 //Steven 20160114 (jou) : Add Production Error Log //Steven 20190110 : Error Log Add Motor Position.
                                OutArmSuck.SetItemData(i, j, NULL_IC);
                                OutArmSuck.Suck[i][j].Normal();
                                bOutArmSuckActive[i][j]=false;
                                bSuckErr[i][j]=false;
                                if(CosFunction.bUseSCKART && AutoClean==false)  //Steven 20161214 (wei) : For SCK ART
                                    fSCKART->AddOutputJamCnt(i, j, ret);        //RogerYang 20250923 : 整合ART OutArm JamCount
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
void SetOutArmHome()
{
    AnsiString sHomeState="";                                                   //Sam 20230707 : 新增 InOutArm Z Home前Home sensor 狀態
    sHomeState=sOutArmZHomeState();
    NewRecordProcess("","OutArm home sensor status",sHomeState);
    InitDoArmZHome();
    bNeedArmZHome=true;
    bIsOutArmHome=true;

    bOutArmPitchNeedHomeCheck=true;

    for(int i=0; i<OutArmSuck.iPickRow; i++)
    {
        for(int j=0; j<OutArmSuck.iPickCol; j++)
        {
            bOutArmZNeedHome[i][j]=true;
        }
    }
}
//------------------------------------------------------------------------------
void ShowOutputShuttleDataMiss()                                                //Steven 20110527
{
    ShowErrorMessage("WAR0256", 0, MOutArmX);                                   //Outut shuttle偵測bin資料不正確,將自動放置於interface error bin
}
//------------------------------------------------------------------------------
bool CheckOutputShuttleDataMiss(TMyKitSuck *P, int iR, int iC)
{
    if(P->Item[iR][iC]==(TEST_PASS+iTestBinCount))
        return false;
    if(P->Item[iR][iC]!=P->iBinData[iR][iC]+TEST_PASS)
    {
        P->Item[iR][iC]=TEST_PASS+iTestBinCount;
        return true;
    }

    return false;
}
//------------------------------------------------------------------------------
void IfUseOnebyOne(int iAuto)
{
    bool bFlag=false;

    if(iAuto<iAutoCnt)                                                          //Steven 20210608 : Tray指定數量退盤功能
    {
        if(CosFunction.bTraySortCntFunc &&
           TrayForm.bTraySortCntFunc[iAuto]==true  &&
           LastSet.iTrayCntForSortFunc[iAuto]<6 &&
           (LastSet.iRunStartMode==rsmContinuStart ||
            LastSet.iRunStartMode==rsmContinuStart_ART))                        //Steven 20251023 : fixed for sort mode
        {
            int iCurrentTrayCnt     =LastSet.iTrayCntForSortFunc[iAuto];        //目前退了幾盤
            int iCurrentTrayICCnt   =LastSet.iTrayICCntForSortFunc[iAuto];      //目前該盤上的IC數量
            while(iCurrentTrayCnt<6 &&                                          //先判斷陣列內的值,要小於6
                  TrayForm.iTraySortCntFunc[iAuto][iCurrentTrayCnt]==0)
            {
                LastSet.iTrayCntForSortFunc[iAuto]++;
                iCurrentTrayCnt=LastSet.iTrayCntForSortFunc[iAuto];
            };

            if(iCurrentTrayCnt<6)
            {
                iCurrentTrayICCnt=LastSet.iTrayICCntForSortFunc[iAuto];
                if(iCurrentTrayICCnt<TrayForm.iTraySortCntFunc[iAuto][iCurrentTrayCnt])
                {
                    bUseOnebyOne=true;
                }
                else
                {
                    bFlag=true;
                }
            }
            else
            {
                bFlag=true;
            }
        }
        else
        {
            bFlag=true;
        }
    }

    if(bFlag)
    {
        if(LastSet.iTester==_2D_SORT &&
           CosFunction.bSortingBy2DList &&
           TestIF_File.bSortingBy2DIDList &&                                    //JerryYang 20240927 : 2D SORT模式Output arm放料避免空洞
           CalOneByOneCount(iAuto)==true)
        {
            bUseOnebyOne=true;
        }
        else if(iCleanOut==1 &&
                TrayForm.bAutoFeed==false)
        {
            if(LastSet.iTemperature==Tempture_Hot &&
               (MOT[MMPlate1].HasIC() || MOT[MMPlate2].Tray.HasIC()))           //kevin 20130509 autoclean
            {
                bUseOnebyOne=false;
            }
            else
            {
                bUseOnebyOne=true;
            }
        }
        else
        {
            bUseOnebyOne=false;
        }
    }
}
//------------------------------------------------------------------------------
void OutArmAddBlockPitch(int &nowX, int &nowY, int iRow, int iCol)              //Frank 20160928 add Subtray Function
{
    if(IniConfig.bUseTrayBlockMode &&                                           //2014-03-04    Dell    for SPIL WLP Add Tray Block
       IniConfig.bP06_LoaderUseCarrierTray==true)                               //Ifor 20181012 (Steven) : add Block Pitch Calculation
    {
        nowX=nowX+((ChangeToFloatNonPcnt((double)(iCol), (double)(AutoForm[0]->XDivision)))* AutoForm[0]->BlockPitchX);
        nowX=nowX-((ChangeToFloatNonPcnt((double)(iCol), (double)(AutoForm[0]->XDivision)))*(AutoForm[0]->XPitch*AutoForm[0]->XDivision));
        nowY=nowY-((ChangeToFloatNonPcnt((double)(iRow), (double)(AutoForm[0]->YDivision)))* AutoForm[0]->BlockPitchY);
        nowY=nowY+((ChangeToFloatNonPcnt((double)(iRow), (double)(AutoForm[0]->YDivision)))*(AutoForm[0]->YPitch*AutoForm[0]->YDivision));
    }
}
//------------------------------------------------------------------------------
#if 0   // PT-W5c RETIRED (SendDataToASE)
//AI(ht9045-v906) 20260809: PT-W5c -- RETIRED, and note WHY this one is odd: GOLDEN ITSELF defines SendDataToASE twice, live and non-static, with DIFFERENT bodies -- aoutarm.cpp:2706 strips the last character via SubString(1,iLength), csystem.cpp:12769 sends Data whole. Both units are in golden <OBJFILES> (aoutarm.obj 129, csystem.obj 58 of 289), so csystem.obj links first and ITS body is the one reachable on a real machine. That is why csystem.cpp keeps the live copy and this -- the golden-faithful aoutarm variant -- is the one gated. Currently moot: ASESendMessage is untranslated so both gate to a no-op.
void SendDataToASE(AnsiString Data)                                             //kevin 20161228 add
{
    int iLength=Data.Length()-1;                                                //kevin 20141023 最後一個site資料不要加 :
#if 0   // GATE G12 -- golden `ASESendMessage->SendToASEData()` ("ASE_K Socket/aseTest.h":57, TASESendMessage form, untranslated). golden :2708 iLength stays LIVE.
    ASESendMessage->SendToASEData(Data.SubString(1, iLength));
#endif
}
#endif
//------------------------------------------------------------------------------
void TransferAutoRatio(int iWhichAuto, int *iXPos, int *iYPos)                  //Steven 20141029 : Auto的軟體齒輪比
{
    if(MACHINE_HAS_AUTO_ALIGNMENT_CCD && TestIF.bEnableAutoAlignment==true)     //KenHsieh 20210813 : add CCD AUTO ALIGNMENT
    {
        CheckOutArmXYScaleByAutoTeach(*iXPos, *iYPos, iTo3Unload[iWhichAuto]);  //JerryYang 20240306 : fix AOA異常, 改用iTo3Unload
    }
    else                                                                        //KenHsieh 20210824 : add AOA功能開啟不補償
    {
        double fi;                                                              //Ztex 2024.11.18 Add In\Out\Sht Different Scale By Temperature -->
        if(Tri_Temp_Machine==1 && LastSet.iTemperature==Tempture_Hot)
        {
            if(IniConfig.bE31_1OutArmUseDifferentScale_Hot==true && Temperature.fWorkTemperBase>=26)
            {
                fi=*iXPos-Prod.XStart[iWhichAuto][iOutArmYBase][iOutArmXBase];
                fi*=IniConfig.dTrayXScale_Hot[iWhichAuto];
                *iXPos=fi+Prod.XStart[iWhichAuto][iOutArmYBase][iOutArmXBase];

                fi=*iYPos-Prod.YStart[iWhichAuto][iOutArmYBase][iOutArmXBase];
                fi*=IniConfig.dTrayYScale_Hot[iWhichAuto];
                *iYPos=fi+Prod.YStart[iWhichAuto][iOutArmYBase][iOutArmXBase];
            }

            if(IniConfig.bE31_2OutArmUseDifferentScale_Cold==true && Temperature.fWorkTemperBase<26)
            {
                fi=*iXPos-Prod.XStart[iWhichAuto][iOutArmYBase][iOutArmXBase];
                fi*=IniConfig.dTrayXScale_Cold[iWhichAuto];
                *iXPos=fi+Prod.XStart[iWhichAuto][iOutArmYBase][iOutArmXBase];

                fi=*iYPos-Prod.YStart[iWhichAuto][iOutArmYBase][iOutArmXBase];
                fi*=IniConfig.dTrayYScale_Cold[iWhichAuto];
                *iYPos=fi+Prod.YStart[iWhichAuto][iOutArmYBase][iOutArmXBase];
            }
        }                                                                       //Ztex 2024.11.18 Add In\Out\Sht Different Scale By Temperature <--
        else if(IniConfig.bE31OutArmUseDifferentScale)                          //Steven 20090710 Start
        {
            fi=*iXPos-Prod.XStart[iWhichAuto][iOutArmYBase][iOutArmXBase];
            fi*=IniConfig.dTrayXScale[iWhichAuto];
            *iXPos=fi+Prod.XStart[iWhichAuto][iOutArmYBase][iOutArmXBase];

            fi=*iYPos-Prod.YStart[iWhichAuto][iOutArmYBase][iOutArmXBase];
            fi*=IniConfig.dTrayYScale[iWhichAuto];
            *iYPos=fi+Prod.YStart[iWhichAuto][iOutArmYBase][iOutArmXBase];
        }
    }
}
//------------------------------------------------------------------------------
void TransferOutShuttleRatio(int iSht, int *iXPos, int *iYPos, int iRow, int iCol)                                      //Steven 20141029 : Out Shuttle的軟體齒輪比
{
    if(MACHINE_HAS_AUTO_ALIGNMENT_CCD && TestIF.bEnableAutoAlignment==true)     //KenHsieh 20210813 : add CCD AUTO ALIGNMENT
    {
        CheckOutArmXYScaleByAutoTeach(*iXPos, *iYPos, iSht+AOA_OutSht1);        //JerryYang 20241119 : fix AOA
    }
    else                                                                        //KenHsieh 20210824 : add AOA功能開啟不補償
    {
        if(TestIF_File.bInArmUseDifferentScaleBySetupFile)                      //pig 20220706 AddOutputScaleBySetup
        {
            double fi;

            if(iSht==0)
            {
                fi=*iXPos-Prod.XOutArm_Shuttle1_Pick[iRow][iCol];
                fi*=TestIF_File.fOutShuttleXScaleBySetupFile[0];
                *iXPos=fi+Prod.XOutArm_Shuttle1_Pick[iRow][iCol];

                fi=*iYPos-Prod.YOutArm_Shuttle1_Pick[iRow][iCol];
                fi*=TestIF_File.fOutShuttleXScaleBySetupFile[0];
                *iYPos=fi+Prod.YOutArm_Shuttle1_Pick[iRow][iCol];
            }
            else
            {
                fi=*iXPos-Prod.XOutArm_Shuttle2_Pick[iRow][iCol];
                fi*=TestIF_File.fOutShuttleXScaleBySetupFile[1];
                *iXPos=fi+Prod.XOutArm_Shuttle2_Pick[iRow][iCol];

                fi=*iYPos-Prod.YOutArm_Shuttle2_Pick[iRow][iCol];
                fi*=TestIF_File.fOutShuttleXScaleBySetupFile[1];
                *iYPos=fi+Prod.YOutArm_Shuttle2_Pick[iRow][iCol];
            }
        }
        else if(IniConfig.bE32ShuttleUseDifferentScale)                         //jou 980716 start : add Scale
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
    }
}
//------------------------------------------------------------------------------
int OutArmPickShuttleAlarm(int iSht, bool bHasDuplicateErr, AnsiString ErrPart)                                         //Steven 20141121 : 整合OutArm吸取異常
{
    int ret=0, iCT=0;                                                           //Jimmychiu 20220901 加入初始值
    AnsiString sBuffer="";                                                      //Jimmychiu 20220901 加入初始值

    if(CUSTOMER_CODE==CC_GT)                                                    //jou 20220922 : 寰邦設備經理Chris 要求 Jam0201,Jam0202,Jam0203 只有Skip選項
    {
        if(iSht==0)
            ret=ShowErrorMessage("JAM0201", K_SKIP, MOutArmX, bHasDuplicateErr, ErrPart);
        else
            ret=ShowErrorMessage("JAM0202", K_SKIP, MOutArmX, bHasDuplicateErr, ErrPart);
    }
    else if(IniConfig.iE50_OutArmPickUpErrorOption==0)                          //RETRY & SKIP  //JerryYang 20210813 : RETRY+SKIP
    {
        if(IniConfig.bInOutArmCanPushHome)
        {
            if(iSht==0)
                ret=ShowErrorMessage("JAM0201", K_RETRY|K_SKIP|K_HOME, MOutArmX, bHasDuplicateErr, ErrPart);            //ChungHung HT9045 2011/12/13 //Input pickup device error時,按"retry"鍵,機台都會自動home
            else
                ret=ShowErrorMessage("JAM0202", K_RETRY|K_SKIP|K_HOME, MOutArmX, bHasDuplicateErr, ErrPart);            //ChungHung HT9045 2011/12/13 //Input pickup device error時,按"retry"鍵,機台都會自動home
        }
        else
        {
            if(iSht==0)
                ret=ShowErrorMessage("JAM0201", K_RETRY|K_SKIP, MOutArmX, bHasDuplicateErr, ErrPart);
            else
                ret=ShowErrorMessage("JAM0202", K_RETRY|K_SKIP, MOutArmX, bHasDuplicateErr, ErrPart);
        }
    }
    else if(IniConfig.iE50_OutArmPickUpErrorOption==1)                          //JerryYang 20210813 : RETRY  //Steven 20141121 : OutArm吸取異常只能Retry
    {
        if(IniConfig.bInOutArmCanPushHome)
        {
            if(iSht==0)
                ret=ShowErrorMessage("JAM0201", K_RETRY|K_HOME, MOutArmX, bHasDuplicateErr, ErrPart);                   //ChungHung HT9045 2011/12/13 //Input pickup device error時,按"retry"鍵,機台都會自動home
            else
                ret=ShowErrorMessage("JAM0202", K_RETRY|K_HOME, MOutArmX, bHasDuplicateErr, ErrPart);                   //ChungHung HT9045 2011/12/13 //Input pickup device error時,按"retry"鍵,機台都會自動home
        }
        else
        {
            if(iSht==0)
                ret=ShowErrorMessage("JAM0201", K_RETRY, MOutArmX, bHasDuplicateErr, ErrPart);
            else
                ret=ShowErrorMessage("JAM0202", K_RETRY, MOutArmX, bHasDuplicateErr, ErrPart);
        }
    }
    else                                                                        //JerryYang 20210813 : SKIP
    {
        if(IniConfig.bInOutArmCanPushHome)
        {
            if(iSht==0)
                ret=ShowErrorMessage("JAM0201", K_SKIP|K_HOME, MOutArmX, bHasDuplicateErr, ErrPart);                    //ChungHung HT9045 2011/12/13 //Input pickup device error時,按"retry"鍵,機台都會自動home
            else
                ret=ShowErrorMessage("JAM0202", K_SKIP|K_HOME, MOutArmX, bHasDuplicateErr, ErrPart);                    //ChungHung HT9045 2011/12/13 //Input pickup device error時,按"retry"鍵,機台都會自動home
        }
        else
        {
            if(iSht==0)
                ret=ShowErrorMessage("JAM0201", K_SKIP, MOutArmX, bHasDuplicateErr, ErrPart);
            else
                ret=ShowErrorMessage("JAM0202", K_SKIP, MOutArmX, bHasDuplicateErr, ErrPart);
        }
    }

    if(ret!=K_SKIP)
    {
        bOutArmPickShtHasRetry=true;
    }

    if(IniConfig.bA35SetErrBinWhenOutShtLoseAndPickupErr)                       //JerryYang 20220215 : 松諭要求的功能Out shuttle lose IC 以及 out arm pick up error set to error bin
    {
//        if(ret!=K_SKIP)
        {
            if(iSht==0)
            {
                FRCarryKit.SetAllRealIC2InterfaceBin();
                iCT=FRCarryKit.CountRealIC();                                   //JerryYang 20190226 : FR -> BR
                sBuffer.printf("Output arm pick up error set to error bin : Out Shttle1 Device=%d;", iCT);
                MyDBIProcess("Message", sBuffer);
            }
            else
            {
                BRCarryKit.SetAllRealIC2InterfaceBin();
                iCT=BRCarryKit.CountRealIC();                                   //JerryYang 20190226 : FR -> BR
                sBuffer.printf("Output arm pick up error set to error bin : Out Shttle2 Device=%d;", iCT);
                MyDBIProcess("Message", sBuffer);
            }
        }
        for(int i=0; i<OutArmSuck.iPickRow; i++)
        {
            for(int j=0; j<OutArmSuck.iPickCol; j++)
            {
                if(OutArmSuck.Item[i][j]!=NULL_IC && OutArmSuck.Item[i][j]!=HAS_NULL_IC)
                {
                    OutArmSuck.SetItemData(i, j, TEST_PASS+iTestBinCount);
                    OutArmSuck.PordRec[i][j].AddTestResultRecord(iTestBinCount, OutArmSuck.cSBin[i][j]);                //Frank 20160505 add
                    OutArmSuck.iWhichAuto[i][j]=Prod.iIfErrorT6;
                }
            }
        }
        iCT=OutArmSuck.CountRealIC();
        sBuffer.printf("Output arm pick up error set to error bin : Out arm Device=%d;", iCT);
        MyDBIProcess("Message", sBuffer);
    }

    return ret;
}
//------------------------------------------------------------------------------
void PorcessJAM0201OutArmPickUpErrorSkip(int iSht, int iShtRow, int iShtCol, int iSuckRow, int iSuckCol)                //Steven 20161214 : 加上OutArm吸取異常Skip的ErrorLog
{
    int iXPos=MOT[MOutArmX].ReadPos();
    int iYPos=MOT[MOutArmY].ReadPos();
    bool bIsPassBinOnCarryKit=false;                                            //RogerYang 20251014 : 取得IC在Shuttle Kit 在PickUp Error的時候，當下是否為passbin
    if(iSht==0)                                                                 //Steven 20161214 : Add Jam Record
    {
        FRCarryKit.PordRec[iShtRow][iShtCol].AddErrorRecord("JAM0201", true, iXPos, iYPos);
        FRCarryKit.SetItemData(iShtRow, iShtCol, NULL_IC);
        bIsPassBinOnCarryKit=Prod.bIsPassBin[FRCarryKit.iBinData[iShtRow][iShtCol]];                                    //RogerYang 20251014 : 取得IC在Shuttle Kit 在PickUp Error的時候，當下是否為passbin
    }
    else
    {
        BRCarryKit.PordRec[iShtRow][iShtCol].AddErrorRecord("JAM0202", true, iXPos, iYPos);
        BRCarryKit.SetItemData(iShtRow, iShtCol, NULL_IC);
        bIsPassBinOnCarryKit=Prod.bIsPassBin[BRCarryKit.iBinData[iShtRow][iShtCol]];                                    //RogerYang 20251014 : 取得IC在Shuttle Kit 在PickUp Error的時候，當下是否為passbin
    }

    OutArmSuck.Suck[iSuckRow][iSuckCol].Error=false;

    if(CosFunction.bUseSCKART)                                                  //Steven 20161214 (wei) : For SCK ART
    {
        int iBinOnCarryKit=(bIsPassBinOnCarryKit==true)?1:2;                    //RogerYang 20260226 : 修正計數
        fSCKART->AddOutputJamCnt(iShtRow, iShtCol, K_SKIP, iBinOnCarryKit);     //RogerYang 20250923 : 整合ART OutArm JamCount
    }
}
//==============================================================================
bool UnloaderTrayYDirForArmYPitch(int iAuto)
{
    if(USE_OUT_Y_IS_AUTO_PITCH==true)                                           //ChungHung 20131231 alter AutoYPitch  //JerryYang 20251218 : IN/OUT ARM支援不同模組
    {
        AutoCalculateOutArmYClosePitch(iAuto);
        return true;
    }
    else if(LoadTrayCanUse8Suck()==1 && TestIF.bSearchLastMode)                 //kevin 20171120 (wei) 判斷LOAD TRAY Y PITCH是否是 ARM Y PITCH可以一次8吸嘴吸IC
    {
        return true;
    }
    return false;
}
//==============================================================================
int AutoCalculateOutArmYClosePitch(int iAuto)                                   //ChungHung 20131231 alter AutoYPitch
{
    int iMin=1500;                                                              //kevin 20190518 : OutArm 放置AUTO最後一排 合到最小PITCH IC 掉落
    OutArmClose_PitchY=0;

    if(USE_OUT_Y_IS_AUTO_PITCH==false)                                          //JerryYang 20251218 : IN/OUT ARM支援不同模組
    {
        iOutArmYStep=1;
        return TestIF.iARM_Y_PITCH;
    }

    if(IN_OUT_ARM_Y_PITCH_MIN>iMin)
        iMin=IN_OUT_ARM_Y_PITCH_MIN;

    if(AutoForm[iAuto]->YPitch>=iMin &&
       AutoForm[iAuto]->YPitch<=IN_OUT_ARM_Y_PITCH_MAX)
    {
        OutArmClose_PitchY=AutoForm[iAuto]->YPitch;
        return OutArmClose_PitchY;
    }

    if(AutoForm[iAuto]->YPitch<iMin)
    {
        int i=0;
        do
        {
            i++;
            if(AutoForm[iAuto]->YPitch*i>=iMin)
            {
                OutArmClose_PitchY=AutoForm[iAuto]->YPitch*i;
                break;
            }
        }while(AutoForm[iAuto]->YPitch*i<=IN_OUT_ARM_Y_PITCH_MAX);

        if(LoadTrayCanUse8Suck()==1 && TestIF.bSearchLastMode)                  //kevin 20171120 (wei) 判斷LOAD TRAY Y PITCH是否是 ARM Y PITCH可以一次8吸嘴吸IC
            return TestIF.iARM_Y_PITCH;

        if(OutArmClose_PitchY<=IN_OUT_ARM_Y_PITCH_MAX)
            return OutArmClose_PitchY;
        else
            return TestIF.iARM_Y_PITCH;                                         //Steven 20160627 : Add for Y-Pitch  -1 --> 6000
    }

    return TestIF.iARM_Y_PITCH;                                                 //Steven 20160627 : Add for Y-Pitch  -1 --> 6000
}
//==============================================================================
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
//==============================================================================
double dOutArmXPitch_1Step=0.0;                                                 //1吸嘴之間的X-Pitch
double dOutArmXPitch_MovePitch=0.0;                                             //張到最開的X-Pitch
int    iOutArmXStep=1;                                                          //Out Arm在Tray X一次要跳幾格
int    iOutArmYStep=1;                                                          //Out Arm在Tray Y一次要跳幾格
//==============================================================================
int AutoCalculateOutArmXClosePitch(int iAuto, bool bUseDeviceDinemsion)
{
    int iStep;
    OutArmClose_PitchX=0;
    int iMaxXPitch4Pick   =(USE_16PICKER_TYPE==1)?iXpitchMaxX2:iXpitchMax;
    int iMinXPitch4Pick   =(USE_16PICKER_TYPE==1)?iXpitchMinX2:iXpitchMin;
    int iMaxXPitch2Pick_13=(USE_16PICKER_TYPE==1)?iXpitchMaxX4:iXpitchMaxX2;
    int iMinXPitch2Pick_13=(USE_16PICKER_TYPE==1)?iXpitchMinX4:iXpitchMinX2;
    int iMaxXPitch2Pick_14=(USE_16PICKER_TYPE==1)?iXpitchMaxX6:iXpitchMaxX3;
    int iMinXPitch2Pick_14=(USE_16PICKER_TYPE==1)?iXpitchMinX6:iXpitchMinX3;

    if(USE_16PICKER_TYPE==1 &&
       OutArmSuck.iPickCol==8)
    {
        for(int i=1; i<AutoForm[iAuto]->XDivision; i++)
        {
            iOutArmXStep=i;
            if(bUseDeviceDinemsion==true)
                OutArmClose_PitchX=DeviceForm.XDimension*iOutArmXStep+100.0;    //要比IC大一點, 避免撞到
            else
                OutArmClose_PitchX=AutoForm[iAuto]->XPitch*iOutArmXStep;
            if(OutArmClose_PitchX>=iXpitchMin && OutArmClose_PitchX<=iXpitchMax)
            {
                dOutArmXPitch_1Step     =OutArmClose_PitchX;
                dOutArmXPitch_MovePitch =OutArmClose_PitchX*7;
                return dOutArmXPitch_MovePitch;
            }
        }
    }
    else if(OutArmSuck.iPickCol==1)                                             //Steven 20241012 : 1x1 mode直接使用最小X-Pitch
    {
        OutArmClose_PitchX      =iMinXPitch2Pick_14;
        dOutArmXPitch_1Step     =(USE_16PICKER_TYPE==1)?(double(OutArmClose_PitchX)/6.0):(double(OutArmClose_PitchX)/3.0);
        dOutArmXPitch_MovePitch =(USE_16PICKER_TYPE==1)?(dOutArmXPitch_1Step*7.0):(dOutArmXPitch_1Step*3.0);
        return dOutArmXPitch_MovePitch;
    }
    else if(iInArmType==e9045_1x4_2_14 &&                                       //Jimmychiu 20241217 : for out arm 1x4_2
            TestIF_File.dSiteXPitch<=iXpitchMaxX2_MM)
    {
        if(bUseDeviceDinemsion==true)
        {
            for(int i=1; i<AutoForm[iAuto]->XDivision; i++)
            {
                iOutArmXStep=i;
                OutArmClose_PitchX=DeviceForm.XDimension*iOutArmXStep+100.0;
                if(OutArmClose_PitchX>=iMinXPitch2Pick_13 && OutArmClose_PitchX<=iMaxXPitch2Pick_13)
                {
                    dOutArmXPitch_1Step     =(USE_16PICKER_TYPE==1)?(double(OutArmClose_PitchX)/4.0):(double(OutArmClose_PitchX)/2.0);
                    dOutArmXPitch_MovePitch =(USE_16PICKER_TYPE==1)?(dOutArmXPitch_1Step*7.0):(dOutArmXPitch_1Step*3.0);
                    return dOutArmXPitch_MovePitch;
                }
            }
        }
        else if((AutoForm[iAuto]->XDivision%2)==0)
        {
            iOutArmXStep=AutoForm[iAuto]->XDivision/2;
            if(bUseDeviceDinemsion==true)
                    OutArmClose_PitchX=DeviceForm.XDimension*iOutArmXStep;
                else
                    OutArmClose_PitchX=AutoForm[iAuto]->XPitch*iOutArmXStep;
            if(OutArmClose_PitchX>=iMinXPitch2Pick_13 && OutArmClose_PitchX<=iMaxXPitch2Pick_13)
            {
                dOutArmXPitch_1Step     =(USE_16PICKER_TYPE==1)?(double(OutArmClose_PitchX)/4.0):(double(OutArmClose_PitchX)/2.0);
                dOutArmXPitch_MovePitch =(USE_16PICKER_TYPE==1)?(dOutArmXPitch_1Step*7.0):(dOutArmXPitch_1Step*3.0);
                return dOutArmXPitch_MovePitch;
            }
        }
        else
        {
            iStep=AutoForm[iAuto]->XDivision/2+1;                               //RogerYang 20260326 : Fix for 1x4
            for(int i=iStep; i>=1; i--)
            {
                iOutArmXStep=i;
                if(bUseDeviceDinemsion==true)
                    OutArmClose_PitchX=DeviceForm.XDimension*iOutArmXStep;
                else
                    OutArmClose_PitchX=AutoForm[iAuto]->XPitch*iOutArmXStep;
                if(OutArmClose_PitchX>=iMinXPitch2Pick_14 && OutArmClose_PitchX<=iMaxXPitch2Pick_14)
                {
                    dOutArmXPitch_1Step     =(USE_16PICKER_TYPE==1)?(double(OutArmClose_PitchX)/6.0):(double(OutArmClose_PitchX)/3.0);
                    dOutArmXPitch_MovePitch =(USE_16PICKER_TYPE==1)?(dOutArmXPitch_1Step*7.0):(dOutArmXPitch_1Step*3.0);
                    return dOutArmXPitch_MovePitch;
                }
            }
        }
    }
    else if(bUseAxxGPicker() ||
            (iInArmType==e9045_1x4_4 &&                                         //Steven 20241111 : for 1x4 close 2 site
             iCloseSiteModeFor1x4==e1x4CloseAbAc))
    {
        if(bUseDeviceDinemsion==true)
        {
            for(int i=1; i<AutoForm[iAuto]->XDivision; i++)
            {
                iOutArmXStep=i;
                OutArmClose_PitchX=DeviceForm.XDimension*iOutArmXStep+100.0;
                if(OutArmClose_PitchX>=iMinXPitch2Pick_14 && OutArmClose_PitchX<=iMaxXPitch2Pick_14)
                {
                    dOutArmXPitch_1Step     =(USE_16PICKER_TYPE==1)?(double(OutArmClose_PitchX)/6.0):(double(OutArmClose_PitchX)/3.0);
                    dOutArmXPitch_MovePitch =(USE_16PICKER_TYPE==1)?(dOutArmXPitch_1Step*7.0):(dOutArmXPitch_1Step*3.0);
                    return dOutArmXPitch_MovePitch;
                }
            }
        }
        else if((AutoForm[iAuto]->XDivision%2)==0)
        {
            iOutArmXStep=AutoForm[iAuto]->XDivision/2;
            OutArmClose_PitchX=AutoForm[iAuto]->XPitch*iOutArmXStep;
            if(OutArmClose_PitchX>=iMinXPitch2Pick_14 && OutArmClose_PitchX<=iMaxXPitch2Pick_14)
            {
                dOutArmXPitch_1Step     =(USE_16PICKER_TYPE==1)?(double(OutArmClose_PitchX)/6.0):(double(OutArmClose_PitchX)/3.0);
                dOutArmXPitch_MovePitch =(USE_16PICKER_TYPE==1)?(dOutArmXPitch_1Step*7.0):(dOutArmXPitch_1Step*3.0);
                return dOutArmXPitch_MovePitch;
            }
        }
        else
        {
            iStep=AutoForm[iAuto]->XDivision/2+1;
            for(int i=iStep; i>=1; i--)                                         //Steven 20241231 : > --> >=
            {
                iOutArmXStep=i;
                if(bUseDeviceDinemsion==true)
                    OutArmClose_PitchX=DeviceForm.XDimension*iOutArmXStep;
                else
                    OutArmClose_PitchX=AutoForm[iAuto]->XPitch*iOutArmXStep;
                if(OutArmClose_PitchX>=iMinXPitch2Pick_14 && OutArmClose_PitchX<=iMaxXPitch2Pick_14)
                {
                    dOutArmXPitch_1Step     =(USE_16PICKER_TYPE==1)?(double(OutArmClose_PitchX)/6.0):(double(OutArmClose_PitchX)/3.0);
                    dOutArmXPitch_MovePitch =(USE_16PICKER_TYPE==1)?(dOutArmXPitch_1Step*7.0):(dOutArmXPitch_1Step*3.0);
                    return dOutArmXPitch_MovePitch;
                }
            }
        }
    }
    else if(bUseAxExPicker() ||
            iInArmType==e9045_1x2_4_Hot)                                        //RogerYang 20260102(JimmyChiu) : 修正dual site模式下，outarm放料位置錯誤
    {
        if(bUseDeviceDinemsion==true)
        {
            for(int i=1; i<AutoForm[iAuto]->XDivision; i++)
            {
                iOutArmXStep=i;
                OutArmClose_PitchX=DeviceForm.XDimension*iOutArmXStep+100.0;
                if(OutArmClose_PitchX>=iMinXPitch2Pick_13 && OutArmClose_PitchX<=iMaxXPitch2Pick_13)
                {
                    dOutArmXPitch_1Step     =(USE_16PICKER_TYPE==1)?(double(OutArmClose_PitchX)/4.0):(double(OutArmClose_PitchX)/2.0);
                    dOutArmXPitch_MovePitch =(USE_16PICKER_TYPE==1)?(dOutArmXPitch_1Step*7.0):(dOutArmXPitch_1Step*3.0);
                    return dOutArmXPitch_MovePitch;
                }
            }
        }
        else if((AutoForm[iAuto]->XDivision%2)==0)
        {
            iOutArmXStep=AutoForm[iAuto]->XDivision/2;
            if(bUseDeviceDinemsion==true)
                    OutArmClose_PitchX=DeviceForm.XDimension*iOutArmXStep;
                else
                    OutArmClose_PitchX=AutoForm[iAuto]->XPitch*iOutArmXStep;
            if(OutArmClose_PitchX>=iMinXPitch2Pick_13 && OutArmClose_PitchX<=iMaxXPitch2Pick_13)
            {
                dOutArmXPitch_1Step     =(USE_16PICKER_TYPE==1)?(double(OutArmClose_PitchX)/4.0):(double(OutArmClose_PitchX)/2.0);
                dOutArmXPitch_MovePitch =(USE_16PICKER_TYPE==1)?(dOutArmXPitch_1Step*7.0):(dOutArmXPitch_1Step*3.0);
                return dOutArmXPitch_MovePitch;
            }
        }
        else
        {
            iStep=AutoForm[iAuto]->XDivision/2+1;
            for(int i=iStep; i>=1; i--)                                         //Steven 20241231 : > --> >=
            {
                iOutArmXStep=i;
                if(bUseDeviceDinemsion==true)
                    OutArmClose_PitchX=DeviceForm.XDimension*iOutArmXStep;
                else
                    OutArmClose_PitchX=AutoForm[iAuto]->XPitch*iOutArmXStep;
                if(OutArmClose_PitchX>=iMinXPitch2Pick_13 && OutArmClose_PitchX<=iMaxXPitch2Pick_13)
                {
                    dOutArmXPitch_1Step     =(USE_16PICKER_TYPE==1)?(double(OutArmClose_PitchX)/4.0):(double(OutArmClose_PitchX)/2.0);
                    dOutArmXPitch_MovePitch =(USE_16PICKER_TYPE==1)?(dOutArmXPitch_1Step*7.0):(dOutArmXPitch_1Step*3.0);
                    return dOutArmXPitch_MovePitch;
                }
            }
        }
    }
    else                                                                        //if(OutArmSuck.iPickCol==4 ||
         //   OutArmSuck.iPickCol==3)
    {
        if(bUseDeviceDinemsion==true)
        {
            for(int i=1; i<AutoForm[iAuto]->XDivision; i++)
            {
                iOutArmXStep=i;
                OutArmClose_PitchX=DeviceForm.XDimension*iOutArmXStep+100.0;

                if(OutArmClose_PitchX>=iMinXPitch4Pick && OutArmClose_PitchX<=iMaxXPitch4Pick)
                {
                    dOutArmXPitch_1Step     =(USE_16PICKER_TYPE==1)?(double(OutArmClose_PitchX)/2.0):OutArmClose_PitchX;
                    dOutArmXPitch_MovePitch =(USE_16PICKER_TYPE==1)?(dOutArmXPitch_1Step*7.0):(dOutArmXPitch_1Step*3.0);
                    return dOutArmXPitch_MovePitch;
                }
            }
        }
        else
        {
            int iStart=AutoForm[iAuto]->XDivision-1;
            if((AutoForm[iAuto]->XDivision%4)==0)
            {
                iStart=(AutoForm[iAuto]->XDivision/4);
            }

            if(iStart<1)
                iStart=1;                                                       //Jimmychiu 20231215 : add Fix3 change to 2 bin box

            if(AutoForm[iAuto]->XDivision==1)                                   //Steven 20240801 : for tray x item=1
            {
                OutArmClose_PitchX=iMaxXPitch4Pick;
                dOutArmXPitch_1Step=iXpitchMax;
                dOutArmXPitch_MovePitch=(USE_16PICKER_TYPE==1)?iXpitchMaxX7:iXpitchMaxX3;
                return dOutArmXPitch_MovePitch;
            }
            else
            {
                for(int i=iStart; i>0; i--)
                {
                    iOutArmXStep=i;
                    OutArmClose_PitchX=AutoForm[iAuto]->XPitch*iOutArmXStep;
                    if(OutArmClose_PitchX>=iMinXPitch4Pick && OutArmClose_PitchX<=iMaxXPitch4Pick)
                    {
                        dOutArmXPitch_1Step     =(USE_16PICKER_TYPE==1)?(double(OutArmClose_PitchX)/2.0):OutArmClose_PitchX;
                        dOutArmXPitch_MovePitch =(USE_16PICKER_TYPE==1)?(dOutArmXPitch_1Step*7.0):(dOutArmXPitch_1Step*3.0);
                        return dOutArmXPitch_MovePitch;
                    }
                }
            }
        }
    }

    iOutArmXStep=1;
    if(OutArmClose_PitchX==0)                                                   //Steven 20240205 : 修正有可能Pitch是0
        OutArmClose_PitchX=iMinXPitch2Pick_14;
    dOutArmXPitch_1Step     =(USE_16PICKER_TYPE==1)?(double(OutArmClose_PitchX)/2.0):iMaxXPitch4Pick;                   //Jimmychiu 20241217 : OutArmClose_PitchX->iMaxXPitch4Pick
    dOutArmXPitch_MovePitch =(USE_16PICKER_TYPE==1)?(dOutArmXPitch_1Step*7.0):(dOutArmXPitch_1Step*3.0);
    OutArmClose_PitchX=iMaxXPitch4Pick;
    return dOutArmXPitch_MovePitch;
}
//------------------------------------------------------------------------------
bool GetOutArmToUnLoaderPosition(int bCanPick2ICAtOnceTime, int iOutPutTray, int &iXPos, int &iYPos, int iR, int iC, bool RealMove)
{
    int iVariablePara[]={2, 1, 0, -1, -1, -1, -1, -1};                          //實體吸嘴相對於基準軸的Pitch數量
    int iRealUseSuck=0;

    if(USE_OUT_ARM_Y_PITCH==iXYPitch16Bd_Be)                                    //Ztex 2023.12.06 Add HT-1032  //JerryYang 20251218 : IN/OUT ARM支援不同模組
    {
        iVariablePara[0]=4;
        iVariablePara[1]=3;
        iVariablePara[2]=2;
        iVariablePara[3]=1;
        iVariablePara[4]=0;
        iVariablePara[5]=-1;
        iVariablePara[6]=-2;
        iVariablePara[7]=-3;
    }
    else if(USE_PICKER_COUNT==ep16Picker)
    {
        iVariablePara[0]=3;
        iVariablePara[1]=2;
        iVariablePara[2]=1;
        iVariablePara[3]=0;
        iVariablePara[4]=-1;
        iVariablePara[5]=-2;
        iVariablePara[6]=-3;
        iVariablePara[7]=-4;
    }
    else if(USE_OUT_ARM_Y_PITCH==iXYPitchIn_Bb_Out_Bc)                          //Ztex 2024.02.24 Add HT-1132  //JerryYang 20251218 : IN/OUT ARM支援不同模組
    {
        iVariablePara[0]=2;
        iVariablePara[1]=1;
        iVariablePara[2]=0;
        iVariablePara[3]=-1;
    }
    else
    {
        if(USE_OUT_ARM_Y_PITCH==iXYPitchVariable)                               //ChungHung 20131231 alter AutoYPitch  //JerryYang 20251218 : IN/OUT ARM支援不同模組
        {
            iVariablePara[0]=1;
            iVariablePara[1]=0;
            iVariablePara[2]=-1;
            iVariablePara[3]=-2;
        }
        else
        {
            iVariablePara[0]=2;
            iVariablePara[1]=1;
            iVariablePara[2]=0;
            iVariablePara[3]=-1;
        }
    }

    if((iHWFix_BinBox==1 && iWhichAuto==eBulkBox) ||
       (IniConfig.bBinBox && iWhichAuto==iBinBoxAtFix &&
        AutoForm[iBinBoxAtFix]->iTrayType==3))                                  //jou 2012-12-11 support Bin Box
    {
        iYPos=Prod.iOutArmBinBoxY-iBinBoxShiftY*200;
        for(int i=0; i<OutArmSuck.iPickRow; i++)
        {
            for(int j=0; j<OutArmSuck.iPickCol; j++)
            {
                if(bOutArmSuckActive[i][j])
                {
                    iRealUseSuck=OutArmSuck.Suck[i][j].iMyCol;                  //移動到實體吸嘴相對於基準軸的Pitch數量
                    iXPos=Prod.iOutArmBinBoxX+dOutArmXPitch_1Step*iVariablePara[iRealUseSuck];
                    break;
                }
            }
        }
    }
    else if(bCanPick2ICAtOnceTime==true)
    {
        iYPos=Prod.YStart[iOutPutTray][iOutArmYBase][iOutArmXBase]-iR*AutoForm[iOutPutTray]->YPitch;
        for(int j=0; j<OutArmSuck.iPickCol; j++)
        {
            if(bOutArmSuckActive[iOutArmPlaceOrder][j]==true)
            {
                iRealUseSuck=OutArmSuck.Suck[iOutArmPlaceOrder][j].iMyCol;
                break;
            }
        }

        iXPos=Prod.XStart[iOutPutTray][iOutArmYBase][iOutArmXBase]+iC*AutoForm[iOutPutTray]->XPitch+dOutArmXPitch_1Step*iVariablePara[iRealUseSuck];
        TransferAutoRatio(iOutPutTray, &iXPos, &iYPos);                         //Steven 20141029 : Auto的軟體齒輪比
        if(iXPos<(MOT[MOutArmX].Motor->PSoftLimitN+10) ||
           iXPos>(MOT[MOutArmX].Motor->PSoftLimitP-50))                         //Steven 20141217 : For Auto 1使用G & H吸嘴
        {
            bOutArmXOverLimit=true;                                             //Steven 20171206 (Wei) : In ATK, 修正Out Arm同時放料但是超出軟體極限縮Pitch導致疊料的問題
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
        for(int i=0; i<OutArmSuck.iPickRow; i++)
        {
            for(int j=0; j<OutArmSuck.iPickCol; j++)
            {
                if(bOutArmSuckActive[i][j])
                {
                    iYPos=Prod.YStart[iOutPutTray][i][j]-iR*AutoForm[iOutPutTray]->YPitch;
                    iRealUseSuck=OutArmSuck.Suck[iOutArmPlaceOrder][j].iMyCol;
                    if(USE_PICKER_COUNT==ep1Picker)
                        iXPos=Prod.XStart[iOutPutTray][iOutArmYBase][iOutArmXBase]+iC*AutoForm[iOutPutTray]->XPitch;
                    else
                        iXPos=Prod.XStart[iOutPutTray][iOutArmYBase][iOutArmXBase]+iC*AutoForm[iOutPutTray]->XPitch+dOutArmXPitch_1Step*iVariablePara[iRealUseSuck];

                    if(IniConfig.bUseTrayBlockMode)                             //2014-03-04    Dell    for SPIL WLP Add Tray Block
                        OutArmAddBlockPitch(iXPos, iYPos, iR, iC);

//                    TransferAutoRatio(iOutPutTray, &iXPos, &iYPos);           //RogerYang 20260123 : Mark  //Steven 20141029 : Auto的軟體齒輪比
                    if(iXPos<(MOT[MOutArmX].Motor->PSoftLimitN+10) ||           //Steven 20141217 : For Auto 1使用G & H吸嘴
                       iXPos>(MOT[MOutArmX].Motor->PSoftLimitP-50))             //jou 2014-11-20 100->50 fix out arm device > 25mm place to Fix3 hang up
                    {
                        bOutArmXOverLimit=true;                                 //Steven 20240501 : 修正超出極限後, 要重新算一次X-Pitch
                    }

                    if(bOutArmXOverLimit==true)                                 //Steven 20240719 : 修正超出極限後, 要重新算一次X-Pitch
                        AutoCalculateOutArmXClosePitch(iWhichAuto, true);

                    if(USE_PICKER_COUNT==ep1Picker)
                        iXPos=Prod.XStart[iOutPutTray][iOutArmYBase][iOutArmXBase]+iC*AutoForm[iOutPutTray]->XPitch;
                    else
                        iXPos=Prod.XStart[iOutPutTray][iOutArmYBase][iOutArmXBase]+iC*AutoForm[iOutPutTray]->XPitch+dOutArmXPitch_1Step*iVariablePara[iRealUseSuck];

                    if(iOutPutTray>=eAuto1 && iOutPutTray<=eAuto6)              //Eastsun 20251231 : Auto tray偏移
                    {
                        iXPos+=OutArmOffSet[OutOfsAuto1+iOutPutTray-eAuto1]->GetArmX(i, j);                             //Eastsun 20251231 : 單支吸嘴偏移功能
                        iYPos+=OutArmOffSet[OutOfsAuto1+iOutPutTray-eAuto1]->GetArmY(i, j);
                    }
                    else if(iOutPutTray>=eFix1 && iOutPutTray<=eFix6)           //Eastsun 20251231 : Fix tray偏移
                    {
                        iXPos+=OutArmOffSet[OutOfsFix1+iOutPutTray-eFix1]->GetArmX(i, j);                               //Eastsun 20251231 : 單支吸嘴偏移功能
                        iYPos+=OutArmOffSet[OutOfsFix1+iOutPutTray-eFix1]->GetArmY(i, j);
                    }

                    if(IniConfig.bUseTrayBlockMode)                             //2014-03-04    Dell    for SPIL WLP Add Tray Block
                        OutArmAddBlockPitch(iXPos, iYPos, iR, iC);

                    TransferAutoRatio(iOutPutTray, &iXPos, &iYPos);             //Steven 20141029 : Auto的軟體齒輪比
                    iOutArmXPosition[i][j]=iC;
                    iOutArmYPosition=iR;
                    for(int k=i; k<OutArmSuck.iPickRow; k++)
                        for(int l=j+1; l<OutArmSuck.iPickCol; l++)
                            bOutArmSuckActive[k][l]=false;
                    break;
                }
            }
        }
    }
    return true;
}
//------------------------------------------------------------------------------
bool CheckUseFixBinBoxFunction()                                                //pig 2013.04.08 FixBinBox start
{
    if(iHWFix_BinBox==1 && iWhichAuto==eBulkBox && iErrorBinBoxAlarm!=0)
        return true;
    return false;
}
//------------------------------------------------------------------------------
bool CheckBinBoxIsFull(bool Alarm, int iAuto)                                   //判斷BOX是否滿了
{
    if(iHWFix_BinBox==0)
        return false;

    if(iHWFix_BinBox==1 && iAuto==eBulkBox && iErrorBinBoxAlarm!=0 && LastSet.iBinBoxCount>=iErrorBinBoxAlarm)
    {
        bFixBinBoxIsFull=true;
        if(Alarm==true)
        {
            if(MoveOutArmToAutoSafe())
                ShowErrorMessage("MES1970", K_RETRY, MMBulkboxKit);
        }
    }

    if(bFixBinBoxIsFull)
        return true;

    return false;
}
//------------------------------------------------------------------------------
bool ErrorBinBoxDetect(int iError)
{
    if(iHWFix_BinBox==1 && CheckBinBoxIsFull(true, iError))
        return false;

    if(iHWFix_BinBox==1 && iError==eBulkBox)                                    //kevin 20160906 add Bluke Box
    {
        if(iHWFix_BinBox==1)
        {
            LastSet.iBinBoxCount++;                                             //kevin 20160827 error bin 計數判斷方式 0:每顆IC  1:一次測試有任一顆就算一次
        }
    }
    return true;
}
//------------------------------------------------------------------------------
//V3.27C.523 Steven 20170428 (wei) : 統一Out Arm從Shuttle吸料後的資料交換
//------------------------------------------------------------------------------
bool SwapShuttleDataToOutArm(int iSht, int iShtRow, int iShtCol, int iSuckRow, int iSuckCol, int *iWitchErrBin)
{
    bool bHasErr=false;
    int iT6;

    LastSet.iIndexInputOutPut[2]++;                                             //kevin 20130125 計數吸取ic
    if(IniConfig.bO20InOutArmPickerLifeTimeCount==true)                         //JerryYang 20220331 : add 吸真空次數計數
    {
        TestIF_File.OutArmPickerLifeCnt[iSuckRow][iSuckCol]++;
        CheckPickerLifeNeedOneCycle();                                          //AI(ht9045-config) 20260521 (RogerYang) : SCC吸嘴壽命報警OneCycle優化
    }
    OutArmSuck.Suck[iSuckRow][iSuckCol].SetNeedSuck(false);

    if(iSht==0)
    {
        if(CheckOutputShuttleDataMiss(&FRCarryKit, iShtRow, iShtCol))
            bHasErr=true;

        iT6=FRCarryKit.Item[iShtRow][iShtCol]-TEST_PASS;
        OutArmSuck.MoveSuckDataDiff(FRCarryKit, iShtRow, iShtCol, iSuckRow, iSuckCol);
    }
    else
    {
        if(CheckOutputShuttleDataMiss(&BRCarryKit, iShtRow, iShtCol))
            bHasErr=true;

        iT6=BRCarryKit.Item[iShtRow][iShtCol]-TEST_PASS;
        OutArmSuck.MoveSuckDataDiff(BRCarryKit, iShtRow, iShtCol, iSuckRow, iSuckCol);
    }

    OutArmSuck.PordRec[iSuckRow][iSuckCol].AddOutArmShuttlePickRecord(iSuckRow, iSuckCol);
    if(iSht==0 &&
       IsNNMode()==NN_2Row)
    {
        OutArmSuck.PordRec[iSuckRow][iSuckCol].AddOutShuttleDetectRecord(TestIF.iSiteMap[iShtRow+2][iShtCol], iShtRow+2, iShtCol);                              //Sam 20201216 : Add record
    }
    else if(IsNNMode()==NN_1Row)
    {
        OutArmSuck.PordRec[iSuckRow][iSuckCol].AddOutShuttleDetectRecord(TestIF.iSiteMap[iShtRow+1][iShtCol], iShtRow+1, iShtCol);                              //Sam 20201216 : Add record
    }
    else
    {
        OutArmSuck.PordRec[iSuckRow][iSuckCol].AddOutShuttleDetectRecord(TestIF.iSiteMap[iShtRow][iShtCol], iShtRow, iShtCol);                                  //Sam 20201216 : Add record
    }

    OutArmSuck.iNeedRotAng[iSuckRow][iSuckCol]=tRotate.RotateDutDate[1][iShtRow][iShtCol];                              //預計旋轉的角度

    if(CheckBin(iT6, iSht)==eBulkBox)                                           //kevin 20160906 by arm error bin check
        *iWitchErrBin=eBulkBox;

#if 0   // GATE G13 -- golden `fAutoTeach->IsRun()` (AutoTeach.h TfAutoTeach; no port body or facade). CONDITION gated so golden’s whole if/else-if chain stays live.
    if(fAutoTeach->IsRun())                                                     //JimmyChiu 20211020 : Auto alignment mode
#else
    if(false)                                                                //AI(W906-PT-W4-aoutarm) 20260808: G13 -- offline the auto-alignment teach routine is never running
#endif
    {
        OutArmSuck.iWhichAuto[iSuckRow][iSuckCol]=eFix3;
    }
    else if(IniConfig.bUseAutoSiteMapping && LastSet.iRunStartMode==rsmAutoSiteMap && IniConfig.bASMAutoSiteMapBinComBine)                                      //kevin 20150115  Auto Site map 所有bin 別 放在盤FIX 2 //kevin 20150115
    {
        OutArmSuck.iWhichAuto[iSuckRow][iSuckCol]=eFix2;                        //AutoSiteMap 所有ic放到fix 2
    }
    else if(CosFunction.bHaveFIFOMode && LastSet.iRunStartMode==rsmFIFOMode)    //Steven 20160303 : FIFO Mode
    {
        OutArmSuck.iWhichAuto[iSuckRow][iSuckCol]=eAuto2;
    }
    else if(TestIF_File.bEnableQASampling && OutArmSuck.bQATray[iSuckRow][iSuckCol]==true && Prod.iQASamplingT6Pos!=ePosNoUse &&                                //Steven 20190326 : QA Sampling
            TestIF_File.bSCKART_EnableART && fSCKART->iLotCount!=0)
    {
        OutArmSuck.iWhichAuto[iSuckRow][iSuckCol]=Prod.iQASamplingT6;
    }
    else if(IniConfig.bA30SetupTeachFunction && LastSet.iTester==OFF_LINE && LastSet.bNeedSetupTeach && iSortUnloadT6>=0)                                       //JerryYang 20180921 Setup Teach功能
    {
        OutArmSuck.iWhichAuto[iSuckRow][iSuckCol]=iSortUnloadT6;
    }
    else
    {
        OutArmSuck.iWhichAuto[iSuckRow][iSuckCol]=iT6;
    }

    InspectOutArmPosition(MOutShuttle1+iSht, iSuckRow, iSuckCol, iShtRow, iShtCol, iOutPickFromSht);

    return bHasErr;
}
//------------------------------------------------------------------------------
void OutArmAddSpeedDisplay()                                                    //KaiChen 20171225 (Steven)：Add Speed Display
{
    iOutArmSpeed1++;
}
//------------------------------------------------------------------------------
void OutArmSubSpeedDisplay()                                                    //KaiChen 20171225 (Steven)：Add Speed Display
{
    iOutArmSpeed1--;
    if(iOutArmSpeed1>99)
        iOutArmSpeed1=99;
    if(iOutArmSpeed1<-99)
        iOutArmSpeed1=-99;
}
//------------------------------------------------------------------------------
//AI(W906-PT-W4-aoutarm) 20260808: ADAPTATION A2 -- golden `__fastcall TMyQASample::TMyQASample()`; __fastcall dropped in lockstep with aoutarm.h:160 (this wave’s contract).
TMyQASample::TMyQASample()
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
//------------------------------------------------------------------------------
bool CheckDuplicateBarCode()                                                    //jou 20191008 : Barcode duplicate file by out arm
{
    if(CosFunction.bBarcodeDuplicateFileByOutArm==false)
        return false;
    #ifdef SOFT_SIMULTE
        return false;
    #else
        int iPos=0, iIndex, ret;
        AnsiString Str, Str1, ErrPart;
        bool bDuplicateFlag[4][8];

        if(FileExists(asBarCodeLot))                                            //Steven 20160505 : 加上保護, 不然開程式會跳Error
        {
//AI(W906-PT-W4-aoutarm) 20260808: ADAPTATION A5 -- golden `fBarCode->list2DByLot`; the port home of that exact object is the bare global (BarCode/BarCode_Bottom2DID.h:144, real def BarCode/BarCode_Bottom2DID.cpp:45).
            list2DByLot->Clear();
//AI(W906-PT-W4-aoutarm) 20260808: ADAPTATION A5 -- golden `fBarCode->list2DByLot`; the port home of that exact object is the bare global (BarCode/BarCode_Bottom2DID.h:144, real def BarCode/BarCode_Bottom2DID.cpp:45).
            list2DByLot->LoadFromFile(asBarCodeLot);
            ErrPart="";
            ZeroMemory(bDuplicateFlag, sizeof(bDuplicateFlag));
        }

        for(int i=0; i<OutArmSuck.iMaxRow; i++)
        {
            for(int j=0; j<OutArmSuck.iMaxCol; j++)
            {
                if(OutArmSuck.Item[i][j]!=HAS_NULL_IC &&
                   OutArmSuck.Item[i][j]!=NULL_IC &&
                   OutArmSuck.cDeviceInf[i][j]!=asBarCodeErrorSend &&
                   OutArmSuck.cDeviceInf[i][j]!="")
                {
                    Str=OutArmSuck.cDeviceInf[i][j];
//AI(W906-PT-W4-aoutarm) 20260808: ADAPTATION A5 -- golden `fBarCode->list2DByLot`; the port home of that exact object is the bare global (BarCode/BarCode_Bottom2DID.h:144, real def BarCode/BarCode_Bottom2DID.cpp:45).
                    list2DByLot->Sort();
                    iPos=Str.Pos("\r\n");
                    if(iPos>0)
                        Str1=Str.SubString(1, iPos-1);
                    else
                        Str1=Str;
//AI(W906-PT-W4-aoutarm) 20260808: ADAPTATION A5 -- golden `fBarCode->list2DByLot`; the port home of that exact object is the bare global (BarCode/BarCode_Bottom2DID.h:144, real def BarCode/BarCode_Bottom2DID.cpp:45).
                    if(list2DByLot->Find(Str1, iIndex))
                    {
                        ErrPart+=OutArmSuck.Suck[i][j].sName;
                        bDuplicateFlag[i][j]=true;
                    }
                }
            }
        }

        if(ErrPart!="")
            ret=ShowErrorMessage("WAR02350", K_SKIP, MOutArmX, false, ErrPart);                                         //ChungHung HT9045 2011/12/13 //Input pickup device
        if(ret==K_SKIP)
        {
            for(int i=0; i<OutArmSuck.iMaxRow; i++)
            {
                for(int j=0; j<OutArmSuck.iMaxCol; j++)
                {
                    if(bDuplicateFlag[i][j]==true)
                    {
                        OutArmSuck.SetItemData(i, j, NULL_IC);
                        OutArmSuck.Suck[i][j].Off();
                    }
                }
            }
            return false;
        }
        else
        {
            return true;
        }
    #endif
}
//------------------------------------------------------------------------------
void SetOutArmNeedDestory(int iSuckRow, int iSuckCol, int iShtRow, int iShtCol, bool bPlace)                            //RogerYang 20250514 Add for 9046AU
{
    if(bPlace==true)
    {
        //if(ptrOutSHT->Item[iShtRow][iShtCol]==NULL_IC)
        OutArmSuck.Suck[iSuckRow][iSuckCol].SetNeedDestroy(true);               //Sort Shuttle可能還有料
    }
    else
    {
        if(ptrOutSHT->Item[iShtRow][iShtCol]>NULL_IC)
            OutArmSuck.Suck[iSuckRow][iSuckCol].SetNeedSuck(true);
    }
}
//------------------------------------------------------------------------------
bool MoveOutArm2XYToShuttle2Wait()
{
    bool bZFlag[MAX_ARM_Row][MAX_ARM_Col];
    int iZPos[MAX_ARM_Row][MAX_ARM_Col];
    int iXVariable[X_PITCH_COUNT];
    int iXPos               =Prod.XOutArm_Shuttle2_Pick[iOutArmYBase][iOutArmXBase];
    int iYPos               =Prod.YOutArm_Shuttle2_Pick[iOutArmYBase][iOutArmXBase];
    int iMovePitchX         =iXpitchMaxX3;
    int iMovePitchY         =GetVariableYOutShuttleData();
    int iOffsetPos          =GetOutArmToShuttleOffset_9045(1, 2, 0, true);
    int iYVariable          =GetOutArmPitchY_9045(iMovePitchY, iOffsetPos);
    ZeroMemory(iXVariable,  sizeof(iXVariable));
    ZeroMemory(bZFlag,      sizeof(bZFlag));
    ZeroMemory(iZPos,       sizeof(iZPos));

    if(IniConfig.bAlarmNeedServoOff)                                            //Steven 20111226 : In Arm 咖啡杯解除
    {
        if(fNote->bMyServoOffInArm)
        {
            return false;
        }
    }

    for(int i=0; i<X_PITCH_COUNT; i++)
        iXVariable[i]=GetOutArmPitchX_9045(iMovePitchX, i, iOffsetPos);

    if(OutArmContinuousMove_9045(iXPos, iYPos, iXVariable, iYVariable, bZFlag, iZPos, false))
        return true;

    return false;
}
//-----------------------------------------------------------------------------
void GetTrayDirection(int iWhichAuto, int &Direction, int &ixStart, int &ixEnd, int &ixStep, int &iyStart, int &iyEnd, int &iyStep)
{
    int Motor=iMMAuto[iWhichAuto%3];
    if(AUTO3_IS_MAGAZINE && iWhichAuto>=eMag1 &&
       TestIF_File.iMagFixTrayType==1)                                          //JerryYang 20221215 : Magazine把fix區當buffer區功能
    {
        Direction=0;                                                            //Magazine buffer不給設定
    }
    else if(iWhichAuto==5 && IniConfig.bFix3PutAllFullIC)                       //ChungHung 20111215 嘗試將Fix3放滿
    {
        Direction=1;
    }
    else if(iWhichAuto==9 && iHWFix_BinBox==1)                                  //kevin 20160819
    {
        Direction=1;
    }
    else
    {
        Direction=AutoForm[iWhichAuto]->Direction;
    }

    if(Direction==0 || Direction==4)                                            //左上到右下    //Steven 20141118 : Add Out Arm Direction 4~7
    {
        ixStart=0;
        ixEnd=MOT[Motor].Tray.XItem;
        ixStep=1;
        iyStep=1;
        if(iWhichAuto>=6)
        {
            if(TrayForm.bFixTrayLink[iWhichAuto%3])
                iyStart=0;
            else
                iyStart=MOT[Motor].Tray.YItem/2+1;
            iyEnd=MOT[Motor].Tray.YItem;
        }
        else
        {
            iyStart=0;
            if(TrayForm.bFixTrayLink[iWhichAuto-3])
            {
                iyEnd=MOT[Motor].Tray.YItem;
            }
            else if(TrayForm.Auto[iWhichAuto].iTrayType==iBinBoxType &&
                    TrayForm.bTrayUpDownSet[iWhichAuto]==true)                  //Jimmychiu 20231215 : add Fix3 change to 2 bin box
            {
                iyEnd=MOT[Motor].Tray.YItem;
            }
            else if(CosFunction.bUseTrayUpDownSet)                              //wei 20160224 TSMC FIX UPDOWN
            {
                if(TrayForm.bTrayUpDownSet[iWhichAuto-3])
                    iyEnd=(MOT[Motor].Tray.YItem-1)/2;
                else
                    iyEnd=MOT[Motor].Tray.YItem;
            }
            else
            {
                iyEnd=(MOT[Motor].Tray.YItem-1)/2;
            }
        }
    }
    else if(Direction==1 || Direction==6)                                       //右上到左下
    {
        ixStart=MOT[Motor].Tray.XItem-1;
        ixEnd=-1;
        ixStep=-1;
        if(TrayForm.iFixTrayMode==0 || iWhichAuto<3)
        {
            iyStart=0;
            iyEnd=MOT[Motor].Tray.YItem;
            iyStep=1;
        }
        else
        {
            iyStep=1;
            if(iWhichAuto>=6)
            {
                if(TrayForm.bFixTrayLink[iWhichAuto%3])
                    iyStart=0;
                else
                    iyStart=MOT[Motor].Tray.YItem/2+1;
                iyEnd=MOT[Motor].Tray.YItem;
            }
            else
            {
                iyStart=0;
                if(TrayForm.bFixTrayLink[iWhichAuto-3])
                {
                    iyEnd=MOT[Motor].Tray.YItem;
                }
                else
                {
                    if(CosFunction.bUseTrayUpDownSet)                           //wei 20160224 TSMC FIX UPDOWN
                    {
                        if(TrayForm.bTrayUpDownSet[iWhichAuto-3])
                            iyEnd=(MOT[Motor].Tray.YItem-1)/2;
                        else
                            iyEnd=MOT[Motor].Tray.YItem;
                    }
                    else
                    {
                        iyEnd=(MOT[Motor].Tray.YItem-1)/2;
                    }
                }
            }
        }
    }
    else if(Direction==2 || Direction==5)                                       //左下到右上
    {
        ixStart=0;
        ixEnd=MOT[Motor].Tray.XItem;
        ixStep=1;
        if(TrayForm.iFixTrayMode==0 || iWhichAuto<3)
        {
            iyStart=MOT[Motor].Tray.YItem-1;
            iyEnd=-1;
            iyStep=-1;
        }
        else
        {
            iyStep=-1;
            if(iWhichAuto>=6)
            {
                if(TrayForm.bFixTrayLink[iWhichAuto%3])
                {
                    iyStart=MOT[Motor].Tray.YItem-1;
                    iyEnd=-1;
                }
                else
                {
                    iyEnd=MOT[Motor].Tray.YItem/2;
                    iyStart=MOT[Motor].Tray.YItem-1;
                }
            }
            else
            {
                iyStart=MOT[Motor].Tray.YItem-1;
                if(TrayForm.bFixTrayLink[iWhichAuto-3])
                {
                    iyStart=MOT[Motor].Tray.YItem-1;
                    iyEnd=-1;
                }
                else
                {
                    iyEnd=-1;
                    if(CosFunction.bUseTrayUpDownSet)                           //wei 20160224 TSMC FIX UPDOWN
                    {
                        if(TrayForm.bTrayUpDownSet[iWhichAuto-3])
                            iyStart=MOT[Motor].Tray.YItem/2-1;
                        else
                            iyStart=MOT[Motor].Tray.YItem-1;
                    }
                    else
                    {
                        iyStart=MOT[Motor].Tray.YItem/2-1;
                    }
                }
            }
        }
    }
    else                                                                        //右下到左上
    {
        ixStart=MOT[Motor].Tray.XItem-1;
        ixEnd=-1;
        ixStep=-1;
        if(TrayForm.iFixTrayMode==0 || iWhichAuto<3)
        {
            iyStart=MOT[Motor].Tray.YItem-1;
            iyEnd=-1;
            iyStep=-1;
        }
        else
        {
            iyStep=-1;
            if(iWhichAuto>=6)
            {
                if(TrayForm.bFixTrayLink[iWhichAuto%3])
                {
                    iyStart=MOT[Motor].Tray.YItem-1;
                    iyEnd=-1;
                }
                else
                {
                    iyEnd=MOT[Motor].Tray.YItem/2;
                    iyStart=MOT[Motor].Tray.YItem-1;
                }
            }
            else
            {
                iyStart=MOT[Motor].Tray.YItem-1;
                if(TrayForm.bFixTrayLink[iWhichAuto-3])
                {
                    iyStart=MOT[Motor].Tray.YItem-1;
                    iyEnd=-1;
                }
                else
                {
                    iyEnd=-1;

                    if(CosFunction.bUseTrayUpDownSet)                           //wei 20160224 TSMC FIX UPDOWN
                    {
                        if(TrayForm.bTrayUpDownSet[iWhichAuto-3])
                            iyStart=MOT[Motor].Tray.YItem/2-1;
                        else
                            iyStart=MOT[Motor].Tray.YItem-1;
                    }
                    else
                    {
                        iyStart=MOT[Motor].Tray.YItem/2-1;
                    }
                }
            }
        }
    }
}
//------------------------------------------------------------------------------
bool CalOneByOneCount(int iAuto)                                                //JerryYang 20240927 : 2D SORT模式Output arm放料避免空洞
{
    int iCount=0, iBinSelCT=0, pitchstepY=0, temp=0, iUnloadCnt[eTrayCount];

    if(TestIF_File.iTestMode==DualSite          ||                              //1x2
       TestIF_File.iTestMode==QualSite2X2N      ||                              //Frank 20200520 2X2NN Mode
       TestIF_File.iTestMode==_6Site2X3N        ||                              //Steven 20220425 : 2X3NN Mode
       TestIF_File.iTestMode==QualSite1X4       ||                              //1x4
       TestIF_File.iTestMode==TriSite1X3        ||                              //1x3   //Frank 20160329 add for 1x3_4
       (TestIF_File.iTestMode==SingleSite)      ||                              //1x1
       TestIF.iTestMode==_8Site1X4)                                             //Isaac 20180402 (Steven) 2x3 mode 用Y sensor 偵測Outshuttle殘料
    {
        iCount=LoadForm->XDivision*3;
    }
    else
    {
        pitchstepY=ChangeToFloatNonPcnt((double)(AutoCalculateOutArmYClosePitch(iAuto)), (double)(LoadForm->YPitch));
        iCount=(pitchstepY+2)*LoadForm->XDivision;
    }

    if(TrayForm.iFixTrayMode)
    {
        iBinSelCT=9;                                                            //use up down
    }
    else
    {
        iBinSelCT=6;
    }

    for(int i=0; i<10; i++)
    {
        iUnloadCnt[i]=0;
    }

    for(int i=0; i<iBinSelCT; i++)
    {
        for(int j=0; j<iTestBinCount; j++)
        {
            temp=Prod.iT6CatData[j];
            if(temp<=0)
                continue;

            if(i==temp-1)
            {
                iUnloadCnt[i]+=LotSummary.iTotalCategory[j];
            }
        }
    }

    if(iExceptAutoCnt[iAuto]-iUnloadCnt[iAuto]<iCount)
    {
        return true;
    }
    else
    {
        return false;
    }
}
//------------------------------------------------------------------------------
