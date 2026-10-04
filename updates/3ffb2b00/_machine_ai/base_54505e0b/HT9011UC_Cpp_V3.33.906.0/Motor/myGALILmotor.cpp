// =============================================================================
//  Motor/myGALILmotor.cpp  --  Galil DMC32 motion driver: the TMyGALILMotor
//                              per-axis HTMotor subclass, the TMyMotor Gali_*
//                              command/motion layer that drives the 4-axis
//                              index arm (Y1/Z1/Z2/Y2 == Galil x/y/z/w), and the
//                              card open/close + vector-move gatekeepers.
//
//  Faithful translation of golden Motor/myGALILmotor.cpp (5221 lines, BCB6,
//  Big5/cp950 -- decoded with python encoding='cp950'; every Chinese comment
//  below is TRANSCRIBED, not paraphrased).
//  Translator: AI(W906-PT-W4-galil) 20260808
//  Translation wave: W906-PT-W4 (last tier of genuinely-missing non-form units).
//
//  ROLE
//  ---------------------------------------------------------------------------
//  This is the ONE unit that talks to the Galil DMC-series motion controller.
//  It has three layers stacked in one file, exactly as golden wrote it:
//    (a) TMyGALILMotor -- a thin HTMotor subclass (25 members) that formats a
//        single-axis Galil ASCII command ("SPx=..;ACx=..;DCx=..;PAx=..;BGx;")
//        and hands it to DMCCommand().  The axis letter comes from iPortID via
//        GetGalilAxisChar (0..3 -> x,y,z,w).
//    (b) 48 TMyMotor::Gali_* / Z1Z2 / ISZ* / GalilTwoY_Move / Gali_nnMode_*
//        members -- the INDEX ARM state machines.  These are NOT thin wrappers:
//        Z1UpZ2Down1 / Z1DownZ2Up1 / Gali_nnMode_Z1Z2_Down / _Up build Galil
//        VECTOR (LM/LI/LE/VS/VA/VD/BGS) programs that move all four index axes
//        on one interpolated path, and the *2 variants ("T move") perform the
//        same motion as a sequence of single-axis moves instead.  Gali_Command is
//        the single choke point: it serialises through a CRITICAL_SECTION, retries
//        up to 150 times, logs to QueueGalilCmd, and escalates WAR2201 / WAR2202 /
//        WAR2203 / WAR1635 / WAR0348 on failure.
//    (c) 12 free functions -- Open_GaliCard / Close_GaliCard (DMCOpen / DMCReset /
//        DMCClose), ScanIndexMotorCanMove (per-axis alarm gate ahead of EVERY
//        move), DoIndexVectorMove (the 20260316/20260527 pre-flight crash check
//        that parses the LI deltas back out of the vector string and refuses the
//        move if a Z would land on the socket while its Y is not there), plus
//        StopAllMotor, CheckIndexArmPosInRange and the delay/rate/error helpers.
//
//  CONDITIONAL-COMPILATION STATE OF THIS BUILD (this port's MachineType.h,
//  identical to golden's for all five switches -- checked line by line):
//    DEBUG_GALIL_CHECK_Y   DEFINED   (MachineType.h:29 / golden :24)  -> ACTIVE
//    DEBUG_GALIL           not def   (MachineType.h:28 / golden :23)  -> inert
//    DEBUG_32SITE          not def   (MachineType.h:25 / golden :20)  -> inert
//    INDEX_PROTECT_TMOVE   not def   (MachineType.h:37 / golden :32)  -> inert
//    SOFT_SIMULTE          not def   (MachineType.h:48 / golden :43)  -> the
//                                    `#ifndef SOFT_SIMULTE` arms are LIVE, i.e.
//                                    the raw DMC* vendor calls ARE compiled in.
//                                    See "VENDOR API" below -- that is the one
//                                    thing the main loop must act on to link.
//
//  WAVE SCOPE -- 85 golden functions, ALL ACTIVE (whole bodies, verbatim control
//  flow / formulas / magic numbers / Galil command strings / fall-throughs).
//  Nothing is GATED-OUT-WHOLE.  Golden line numbers:
//    GetGalilAxisChar (static)                   golden :59   ACTIVE
//    SetGaliRate                                 golden :71   ACTIVE
//    InitialSetUpGaliDelayCount                  golden :80   ACTIVE
//    InitialGaliDelayCount                       golden :85   ACTIVE
//    TMyGALILMotor::TMyGALILMotor  (ctor)        golden :102  ACTIVE
//    TMyGALILMotor::~TMyGALILMotor (dtor)        golden :126  ACTIVE
//    TMyGALILMotor::InitMotor                    golden :130  ACTIVE
//    TMyGALILMotor::SetSpeed                     golden :135  ACTIVE
//    TMyGALILMotor::SetInitSpeed                 golden :140  ACTIVE
//    TMyGALILMotor::SetServoAlarmOn              golden :145  ACTIVE
//    TMyGALILMotor::ReadPos                      golden :150  ACTIVE
//    TMyGALILMotor::ScanMotorStatus              golden :155  ACTIVE
//    TMyGALILMotor::MoveToPos                    golden :172  ACTIVE
//    TMyGALILMotor::Stop                         golden :194  ACTIVE
//    TMyGALILMotor::DecStop                      golden :205  ACTIVE
//    TMyGALILMotor::JogP                         golden :210  ACTIVE
//    TMyGALILMotor::JogN                         golden :230  ACTIVE
//    TMyGALILMotor::HomeObject                   golden :250  ACTIVE
//    TMyGALILMotor::SetRange                     golden :264  ACTIVE
//    TMyGALILMotor::SetRate                      golden :269  ACTIVE
//    TMyGALILMotor::GetAlarm                     golden :274  ACTIVE
//    TMyGALILMotor::HomeFlag                     golden :288  ACTIVE
//    TMyGALILMotor::ResetPos                     golden :293  ACTIVE
//    TMyGALILMotor::MotionDone                   golden :300  ACTIVE
//    TMyGALILMotor::ReadRealPos                  golden :315  ACTIVE
//    TMyGALILMotor::ReadEnCoderRealPos           golden :330  ACTIVE
//    TMyGALILMotor::SetCommand                   golden :345  ACTIVE
//    TMyGALILMotor::SetPosition                  golden :356  ACTIVE
//    TMyGALILMotor::SetServoOn                   golden :367  ACTIVE
//    TMyGALILMotor::SetSoftLimit                 golden :382  ACTIVE
//    GetGalilErrString                           golden :388  ACTIVE
//    TMyMotor::Gali_Command                      golden :428  ACTIVE
//    TMyMotor::Gali_ScanMotStatusTIMO            golden :598  ACTIVE
//    TMyMotor::Gali_ScanMotStatus                golden :643  ACTIVE
//    TMyMotor::Gali_ScanAlarmStatus              golden :684  ACTIVE
//    TMyMotor::Gali_GetMOT                       golden :732  ACTIVE
//    TMyMotor::Gali_MotMove2                     golden :744  ACTIVE
//    TMyMotor::Gali_MotMove                      golden :936  ACTIVE
//    TMyMotor::Gali_MotMoveNoWait                golden :1133 ACTIVE
//    TMyMotor::Gali_MotMoveSkipEncoder           golden :1300 ACTIVE
//    TMyMotor::Z1UpZ2Down1                       golden :1396 ACTIVE
//    TMyMotor::Z1DownZ2Up1                       golden :1765 ACTIVE
//    TMyMotor::Z1UpZ2Down2                       golden :2123 ACTIVE
//    TMyMotor::Z1DownZ2Up2                       golden :2326 ACTIVE
//    TMyMotor::Z1UpZ2Down                        golden :2532 ACTIVE
//    TMyMotor::Z1DownZ2Up                        golden :2588 ACTIVE
//    TMyMotor::Gali_ReadPos                      golden :2644 ACTIVE (one 7-line
//                                                     VCL panel-paint block is
//                                                     gated -- GATE(W4G-3))
//    TMyMotor::Gali_MotHome                      golden :2693 ACTIVE
//    TMyMotor::Gali_MotHomeFindZ                 golden :2745 ACTIVE
//    TMyMotor::Gali_Two_ZAxis_Move               golden :2804 ACTIVE
//    TMyMotor::Gali_JogP                         golden :2916 ACTIVE
//    TMyMotor::Gali_JogPSetup                    golden :2938 ACTIVE
//    TMyMotor::Gali_JogPAndCount                 golden :2960 ACTIVE
//    TMyMotor::Gali_JogN                         golden :2983 ACTIVE
//    TMyMotor::Gali_JogNSetup                    golden :3005 ACTIVE
//    TMyMotor::Gali_JogNAndCount                 golden :3029 ACTIVE
//    TMyMotor::Gali_MovePR                       golden :3052 ACTIVE
//    Open_GaliCard                               golden :3079 ACTIVE
//    Close_GaliCard                              golden :3141 ACTIVE
//    TMyMotor::Gali_ReadEncoderPos               golden :3162 ACTIVE
//    TMyMotor::CheckPos                          golden :3190 ACTIVE
//    TMyMotor::CheckYPos                         golden :3219 ACTIVE
//    TMyMotor::CheckPos_nnMode                   golden :3267 ACTIVE
//    TMyMotor::Gali_ReadEncoderInRandge          golden :3297 ACTIVE
//    TMyMotor::Gali_ReadEncoderBelowCheckHeight  golden :3335 ACTIVE
//    TMyMotor::Gali_ReadEncoderInRandgeMinLimit  golden :3353 ACTIVE
//    TMyMotor::Gali_ReadEncoderInRandgeNoWait    golden :3405 ACTIVE
//    TMyMotor::Gali_ReadEncoderMaxRandge         golden :3443 ACTIVE
//    TMyMotor::Gali_ReadEncoderOver              golden :3480 ACTIVE
//    TMyMotor::ISZ1Up_Z2Down                     golden :3490 ACTIVE
//    TMyMotor::ISZ1Up_Z2DownNoWait               golden :3500 ACTIVE
//    TMyMotor::ISZ1Down_Z2Up                     golden :3548 ACTIVE
//    TMyMotor::ISZ1Down_Z2UpNoWait               golden :3558 ACTIVE
//    TMyMotor::ISNormal                          golden :3578 ACTIVE
//    TMyMotor::Gali_SingalHome                   golden :3589 ACTIVE
//    TMyMotor::Gali_FindZPhase                   golden :3852 ACTIVE
//    TMyMotor::GalilTwoY_Move                    golden :4389 ACTIVE
//    ScanIndexMotorCanMove                       golden :4568 ACTIVE
//    DoIndexVectorMove                           golden :4592 ACTIVE
//    ScanIndexOtherMotorCanMove                  golden :4707 ACTIVE
//    StopAllMotor                                golden :4712 ACTIVE
//    TMyMotor::ISZ1Up_Z2Up                       golden :4770 ACTIVE
//    TMyMotor::ISZ1Down_Z2Down                   golden :4779 ACTIVE
//    TMyMotor::Gali_nnMode_Z1Z2_Down             golden :4788 ACTIVE
//    TMyMotor::Gali_nnMode_Z1Z2_Up               golden :5004 ACTIVE
//    CheckIndexArmPosInRange                     golden :5202 ACTIVE
//   Plus golden's file-scope objects (golden :33-50) -- reproduced in the head
//   block below.  IndexZCanMove[2] is the ONE that is NOT re-defined -- GATE(W4G-1).
//
//  SATISFIED-BY-SUBSTRATE (already real in this tree; NOT re-implemented here):
//    MOT[] / TTrayMotor / TMyMotor (Motor/mymotor.h + mymotor.cpp), HTMotor +
//    CheckIsSafeDoorOpen + iCwLed..iServoOn + ALM_MOTOR_MOVE (Motor/HTMotor.h),
//    TMyMotor::GetRealPos / CheckArmPosArrival / CheckArmPosInRange /
//    CheckYPosWhenZDown / CheckY1Y2TargetPos / Check_Y1Y2_TargetPosWillCrash /
//    Check_Y1_TargetPosInTeachPos / Check_Y2_TargetPosInTeachPos /
//    PCIL132_StopMotor (Motor/mymotor.cpp), ShowIndexMotorError +
//    RecordIndexPositionError (Motor/mymotor.cpp:1425 / :1427, no-ops),
//    ShowErrorMessage / ShowMyMessage / ShowMotorErrorMessage /
//    MotorIndexToJamCode / MyDBIProcessNew / RecordProcess / LastSet / __FUNC__
//    (canary_support.h -- substitutes golden note.h + mymessbox.h + cMyDB.h),
//    QueueGalilCmd (TMyStrQueue100, cpublic.h:283 / cmydef.cpp:4782), MySleep
//    (common.cpp:1919), MySleepEx (common.cpp:1912), TrayMoveOut
//    (asendic.cpp:1501), SW[] / Sw* (myswitch.h), Cylinder[] / C_* (mycylin.h),
//    ReadIniData / WriteIniData / CheckIniData / CheckAndReadIniDataGeneral /
//    WriteIniDataGeneral / asGeneralPath / asIndexZphasePath (common.cpp),
//    GetTimeInfo (cpublic.cpp:450), Prod / Tech / TestIF_File / ArmSpeed /
//    IndexStatus (cprod.h), IniConfig (Config.h), CosFunction (CosFunction.h),
//    COM2 (atester_shims.h + atester_shims.cpp:328; bCCDDummyRum==true offline),
//    fMain (forms/fMain.cpp:385), fOffSet (forms/fOffSet.cpp:12), and the cmydef.h
//    global set (iHome, JamCode, MotErrPos, iGali_SpSpeed, iGali_VsSpeed,
//    MyLongMask, Z1Safe / Z1Up / Z1Down / Z1DownSafe / Z2Safe / Z2Up / Z2Down /
//    Z2DownSafe / XShiftF / XShiftR / AxisY_Pre_MovePos / Z1DownToShuttle /
//    Z2DownToShuttle, iBackUpZ1DownPosition / iBackUpZ2DownPosition,
//    bCheckGiveWay, bShowIndexMotorError, bGail_HighSpeedMode,
//    bResetGalilTwoYMove, bZ1UpAndPlaceZ2Down, bZ1DownZ2UpAndPlace, the
//    iIndex{Y,Z}{1,2}HomeTo*Phase* family, i{Y,Z}{1,2}ModifyDistanceRef,
//    b{Y,Z}{1,2}ModifyDistanceRef, iIndexY{1,2}FindPhase*Pos,
//    iM{ax,in}CommandY{1,2}, GaliPosOffSet, USE_INDEX_ARM_AXES, Gali_MaxAxis,
//    GALI_PROTECT_MODE, TOTAL_MOTOR, CUSTOMER_CODE, MachineTypeChoice,
//    REAL_TIME_CCD, T_MODE_SPEED, INSTALL_OCR), plus the READ-ONLY Galil vendor
//    SDK headers under Motor/vendor/ (never edited).
//
//  =========================================================================
//  GATE REGISTER -- 7 gates.  Each keeps golden's text VERBATIM in its `#if 0`
//  arm so retirement is a deletion.  Every absence claim carries the exact grep;
//  all of them were RE-RUN as the final action of this wave (2026-08-08, after
//  six sibling units had been landing files throughout the session), because an
//  absence claim made at the start of a parallel wave is worthless.
//  =========================================================================
//
//  GATE(W4G-1) -- `bool IndexZCanMove[2]={true, true};`          golden :50
//    GATED: golden's DEFINITION of the per-index-Z interlock array.  Golden
//    myGALILmotor.cpp:50 is its SOLE definition in the whole golden tree
//    (ainarm2.h:48, ainarm2.cpp:2312, AutoClean/AutoClean.cpp:42 and uhome.cpp:49
//    each carry only `extern bool IndexZCanMove[2];`).
//    WHY THE #else IS `extern` AND NOT A DEFINITION: this port ALREADY defines the
//    array at ainarm9045_w7_shims.cpp:47; that file is registered
//    (CMakeLists.txt:1442) and is ALSO linked straight into
//    tests/test_w7_a1_inarm_floating_latch.cpp, which reads and writes it as its
//    terminal oracle (:139-140, :162-163).  Re-defining it here would be a
//    duplicate symbol; removing the shim to make room would break that test.  So
//    this file links against the existing definition.
//    ** BEHAVIOUR DELTA -- A REAL PRE-EXISTING PORT DEFECT THIS UNIT EXPOSES **
//    The shim initialises `{ false, false }`; GOLDEN initialises `{ true, true }`.
//    Gali_MotMove / Gali_MotMove2 / Gali_MotMoveNoWait all open with
//    `if(IndexZCanMove[0]==false) return false;` for MTestZ1 (golden :971, :774,
//    :1166) and the mirror test for MTestZ2.  With `false`, the FIRST index-Z move
//    after start-up is refused, and nothing inside this file can clear the flag
//    (only a move that COMPLETES sets it true).  In practice the machine unwedges
//    only once ainarm9045.cpp:1837-1838 / AutoClean.cpp:8659 etc. force both true.
//    This delta is NOT introduced by this file -- it is the shim's initialiser,
//    newly reachable now that the real Gali_MotMove bodies exist.
//    INTEGRATION REQUEST: set ainarm9045_w7_shims.cpp:47 to `{ true, true }`
//    (golden myGALILmotor.cpp:50) and correct that file's comment, which cites
//    "golden ainarm2.h:48" for the initialiser -- ainarm2.h:48 is only the extern.
//
//  GATE(W4G-2) -- `extern int iEncoderTorence; extern int iCheckZ;
//                  extern char bMask[8];`                 golden :45, :46, :48
//    GATED: golden's three extern declarations of globals golden DEFINES in
//    Motor/mymotor.cpp.
//    WHY: this port's Motor/mymotor.cpp defines all three with INTERNAL linkage --
//    `static int iEncoderTorence = 500` (:113), `static int iCheckZ = 4000` (:114),
//    `static unsigned char bMask[8]` (:120).  A `static` definition is invisible
//    to this TU, so golden's extern cannot resolve.  The #else arm defines
//    TU-local copies with the SAME initialisers mymotor.cpp itself uses.
//    ABSENCE CLAIM + GREP (re-run at hand-off, 2026-08-08):
//      grep -rn "static int iEncoderTorence\|static int iCheckZ\|static unsigned char bMask" --include=*.cpp D:/HT9045/HT9011UC_Cpp_V3.33.906.0/
//      -> Motor/mymotor.cpp:113, :114, :120 only, all `static`; no non-static
//         definition of any of the three exists anywhere in the tree.
//    BEHAVIOUR DELTA: bMask is a constant bit table -> zero delta.
//    iEncoderTorence is declared-but-never-used in golden's myGALILmotor.cpp ->
//    zero delta (kept only for fidelity of the head block).  iCheckZ IS a real
//    delta: Motor/mymotor.cpp:185-196 MUTATES its own copy in InitMOTParameter
//    (500->1000 for two customer codes; 4000->11000 for CC_GIGAS, the
//    ht9045-v899 CASE-20260507-001 fix).  This TU's copy stays 4000, so the two
//    GALI_PROTECT_MODE==0 in-motion socket-height checks -- golden :1602
//    `iEncoderZ2<(Prod.All_TestZ_Test_Safe-iCheckZ)` and golden :1966, the Z1
//    mirror -- open their ShowIndexMotorError window 7000 units (0.01mm) closer
//    to the socket than a CC_GIGAS machine intends.  The check therefore becomes
//    TIGHTER (it can report a position error slightly early); it never suppresses
//    one.  INTEGRATION REQUEST: drop `static` from Motor/mymotor.cpp:113/114/120
//    (golden has all three at external linkage) and delete this #else arm.
//
//  GATE(W4G-3) -- the VCL panel-paint block in Gali_ReadPos  golden :2675-2681
//    GATED: `PWinCtrl->Top=ScreenPos;` / `PWinCtrl->Left=ScreenPos;`.
//    WHY: this port declares the member `void *PWinCtrl` (Motor/mymotor.h:104,
//    its own comment: "was TWinControl* (VCL); gated W7-UI").  A void* cannot be
//    dereferenced.  Motor/mymotor.cpp made the identical call in its own ReadPos.
//    BEHAVIOUR DELTA: the on-screen motor-position marker does not move.
//    ScreenPos is still computed with golden's exact formula and no control flow
//    reads it -> zero machine-behaviour delta.  Retire with W7-UI.
//
//  GATE(W4G-4) -- `Alarm->Set(ALM_MOTOR_MOVE);`  golden :632, :651, :703, :721   ** LIFTED 20260926 AI(W906-HALARM)：halarm.h／HAlarm.cpp 已翻，下面的 WHY 已過期 **
//    GATED: golden's `extern HAlarm *Alarm;` (golden :49); the four Alarm->Set
//    CALL SITES stay verbatim and route to the shim.
//    WHY: golden's `HAlarm *Alarm` is DEFINED at golden main.cpp:202 -- an
//    uninitialised global pointer in the one file this port has not translated
//    (exactly a docs/PT_CAMPAIGN_PLAN.md section-8 NULL) -- and class HAlarm
//    itself lives in the sibling BCB6 component package
//    D:\HT9045\elec\Component\halarm.h, outside the translated tree
//    (canary_support.h:242-247 documents that package boundary).
//    ABSENCE CLAIM + GREP (re-run at hand-off, 2026-08-08):
//      grep -rn "HAlarm[ ]*\*Alarm" --include=*.cpp --include=*.h D:/HT9045/HT9011UC_Cpp_V3.33.906.0/
//      -> ZERO definitions and ZERO declarations.  The only hits are prose:
//         canary_support.h:252, mycylin.cpp:15 and mycylin.cpp:122.
//    WHY THE #else DEFAULT IS FAITHFUL: this tree ALREADY made this exact call
//    for this exact god-object -- mycylin.cpp:122-133 turned golden's
//    `Alarm->Set/Clear` into local no-op stubs ("Only reachable via
//    SystemStart==true escalation").  The #else arm follows that REGISTERED
//    idiom.  It deliberately does NOT route into canary_support.h's
//    W906_PopUpAlarm_Push seam, even though that push IS the offline stand-in for
//    HAlarm::Set: the push takes an `HTMotor*` component and golden's `Alarm` is
//    not a motor, so pushing NULL would make ckernel's
//    `if(MOT[i].Motor==Comp)` (golden ckernel.cpp:2440) FALSE-MATCH every
//    unconfigured motor -- whose ->Motor is also NULL -- and mis-attribute the
//    alarm.  A no-op loses information; a NULL push invents wrong information.
//    BEHAVIOUR DELTA: the pop-up/UI escalation of a motor-move alarm is lost.
//    The part every reader in THIS file actually consumes -- `Gali_MotorAlarm=true`
//    on the line immediately before each Alarm->Set -- is still set verbatim, so
//    GetMotorAlarm() and ScanIndexMotorCanMove()'s decisions are unchanged.
//
//  GATE(W4G-5) -- `GetIndexParm();`  called golden :1414, :1784, :2136, :2338,   //AI(W906-I115B) 20260930: LIFTED (RULINGS_20260930 #4, row 33): cinitial.cpp:16185 has the golden body now, :727 declares it
//                                          :4805, :5021   (6 call sites)
//    GATED: the DECLARATION only.  All six CALLS stay verbatim.
//    WHY: golden declares `extern void GetIndexParm();` (golden cinitial.h:30)
//    and defines it at golden cinitial.cpp:13713.  This port's cinitial.cpp is a
//    PARTIAL translation -- CMakeLists.txt:1391, "block 1 (golden :77-3060)" --
//    and :13713 is far outside that block.
//    ABSENCE CLAIM + GREP (re-run at hand-off, 2026-08-08):
//      grep -rn "GetIndexParm" --include=*.cpp --include=*.h D:/HT9045/HT9011UC_Cpp_V3.33.906.0/
//      -> ZERO hits anywhere in the tree: no declaration, no definition, no call.
//    BEHAVIOUR DELTA -- THE LARGEST ONE IN THIS FILE, STATED PLAINLY.
//    GetIndexParm is what loads the index-move geometry (Z1Safe, Z1Up, Z1Down,
//    Z1DownSafe, Z2Safe, Z2Up, Z2Down, Z2DownSafe, XShiftF, XShiftR,
//    AxisY_Pre_MovePos, Z1DownToShuttle, Z2DownToShuttle) out of Prod/Tech before
//    each vector move.  With a no-op those globals keep whatever they already
//    hold -- and cmydef.cpp initialises every one of them to 0.  The four vector
//    builders (Z1UpZ2Down1 / Z1DownZ2Up1 / Gali_nnMode_Z1Z2_Down / _Up) then emit
//    an all-zero LI program: a Galil vector move of zero distance.  The axes are
//    commanded, the move "completes" immediately, and the CheckPos /
//    ISZ1Up_Z2Down encoder verification that follows fails -> golden's own
//    JamCode(13/14/15) + ShowMotorErrorMessage arm.  On a real machine that reads
//    as "index arm does not move and reports a position error" -- NOT a crash and
//    NOT an unguarded motion.  The single-axis paths (Gali_MotMove, Gali_MotMove2,
//    Gali_Two_ZAxis_Move, GalilTwoY_Move) take their targets as ARGUMENTS and do
//    not depend on GetIndexParm, so those are fully faithful.  Retire the moment
//    cinitial.cpp reaches golden :13713.
//
//  GATE(W4G-6) -- `bool CheckTestZ/CheckTestZ1/CheckTestZ2(AnsiString sFunc);`
//                 golden :52, :53, :54 (forward declarations; 14 call sites)
//    GATED: the three forward declarations.  All 14 CALLS stay verbatim.
//    WHY: golden defines these three at golden Motor/mymotor.cpp:67 / :117 / :173.
//    This port's Motor/mymotor.cpp has NOT translated them -- its own banner
//    (Motor/mymotor.cpp:32) lists "CheckTestZ*" among the gated regions.
//    ABSENCE CLAIM + GREP (re-run at hand-off, 2026-08-08):
//      grep -rn "CheckTestZ" --include=*.cpp --include=*.h D:/HT9045/HT9011UC_Cpp_V3.33.906.0/
//      -> ONE hit, the prose line Motor/mymotor.cpp:32.  No declaration, no
//         definition, no call anywhere.
//    WHY THE #else IS NOT AN APPROXIMATION: golden's three bodies are built
//    ENTIRELY from symbols THIS FILE defines (MOT[].Gali_ReadPos and
//    MOT[].Gali_ReadEncoderInRandgeMinLimit) plus four already real here
//    (ShowIndexMotorError / RecordIndexPositionError -- Motor/mymotor.cpp:1425 /
//    :1427 -- MySleepEx, USE_INDEX_ARM_AXES).  So the #else arm reproduces golden
//    Motor/mymotor.cpp:67-226 BYTE-FOR-BYTE (copied programmatically out of the
//    cp950 source, not retyped) with exactly one change: the three names gain
//    `static` + a `W4G_` prefix so they stay TU-local and CANNOT collide when
//    Motor/mymotor.cpp translates its own copies.  Three #defines preserve the
//    golden call spellings.
//    BEHAVIOUR DELTA: none.  Same branch ladder, same retry ceilings (150 / 100 /
//    100), same ShowIndexMotorError strings, same MySleepEx(5,true).
//    (Their `#ifdef DEBUG` / `#ifdef DEGBU` fMain->lbEnCoder0/1 blocks -- note
//    golden's own typo "DEGBU" at golden mymotor.cpp:204 -- are inert in every
//    build and are kept verbatim, typo included.)
//    INTEGRATION: when Motor/mymotor.cpp translates CheckTestZ*, delete this
//    #else arm and restore the #if 0 forward declarations.
//
//  GATE(W4G-7) -- `SaveFile(asIndexZphasePath, sData);`  golden :4071, :4082,
//                 :4085, :4096, :4141, :4152, :4155, :4166, :4210, :4221, :4224,
//                 :4235, :4281, :4293, :4296, :4307 (16 sites, all in
//                 Gali_FindZPhase)
//    GATED: the declaration.  All 16 CALLS stay verbatim.
//    WHY: `void SaveFile(AnsiString sPath, AnsiString sData)` IS declared in this
//    port (csystem.h:259 -- golden's own home for the declaration) but has NO BODY
//    anywhere in the tree; golden defines it at golden csystem.cpp:22179 and this
//    port's csystem.cpp has not reached it.  This unit would be the tree's FIRST
//    consumer, i.e. the first TU to turn that declaration into an unresolved
//    external -- exactly the failure mode that hides for months.
//    ABSENCE CLAIM + GREP (re-run at hand-off, 2026-08-08):
//      grep -rn "^void SaveFile" --include=*.cpp D:/HT9045/HT9011UC_Cpp_V3.33.906.0/
//      -> only three UNRELATED MEMBER functions: cprod.cpp:476
//         ATK_RECIPE_INFO::SaveFile(), ProductionInfo/FileInfo.cpp:545
//         cDatabaseMin::SaveFile(), Public/HTEditList.cpp:2352
//         uPlateInfo::SaveFile(AnsiString).  The free two-argument
//         SaveFile(AnsiString,AnsiString) has no definition.
//    WHY THE #else IS NOT AN APPROXIMATION: it REPRODUCES golden
//    csystem.cpp:22179-22198 verbatim (MyForceDirectories; build
//    "<path>\<YYYY>_<MM>\<YYYYMMDD>.txt"; fopen "a+"; fputs; fclose) as a
//    TU-local static, because every symbol that body needs is ALREADY REAL here:
//      MyForceDirectories -- common.h:341, body common.cpp:1806, and present in
//        the built archives as __Z18MyForceDirectoriesN9vclcompat10AnsiStringES0_
//        (verified with nm over build_0808_w3_final/*.a this wave);
//      SystemYear / SystemMonth / SystemDate -- cmydef.h:227, cmydef.cpp:292.
//    CORRECTION, RECORDED DELIBERATELY: an earlier draft of this gate said
//    "MyForceDirectories has no port body either" and defaulted to a NO-OP.  That
//    claim was WRONG -- the re-run grep at hand-off found ATC/ATCInterface.cpp:866
//    calling it and common.cpp:1806 defining it.  The gate was rewritten to the
//    faithful reproduction.  The lesson is the campaign's own: an absence claim is
//    only worth the grep that was actually run.
//    BEHAVIOUR DELTA: none.  Same directory layout, same filename format
//    (including golden's `%4d%2d%2d` -- note NOT `%04d%02d%02d`, so single-digit
//    months/days come out space-padded, e.g. "2026 8 8.txt"; that quirk is
//    golden's and is preserved), same append semantics, same silent-skip when
//    fopen fails.  ONE mechanical adaptation inside the body: golden's
//    `sPath+"\\"+SystemYear+"_"+SystemMonth` relies on BCB6's implicit
//    Word -> AnsiString conversion in operator+, so the port spells the two
//    conversions explicitly as AnsiString((int)SystemYear) /
//    AnsiString((int)SystemMonth) -- identical decimal text, no format change.
//
//  =========================================================================
//  VENDOR API -- 7 Galil DMC32 entry points, ALL ACTIVE, NONE STUBBED HERE
//  =========================================================================
//  SOFT_SIMULTE is NOT defined in this build, so every `#ifndef SOFT_SIMULTE` arm
//  below is compiled in and these are live external references.  They are LISTED
//  rather than stubbed because the main loop owns the offline vendor layer
//  (Motor/vendor_offline_motionnet.cpp) and because an archive member is extracted
//  only to resolve an ALREADY-undefined symbol -- a stub invented here could sit
//  unnoticed for months.
//    DMCCommand            Motor/vendor/DMCCOM.H:540   ACTIVE  (30 call sites)
//    DMCClear              Motor/vendor/DMCCOM.H:663   ACTIVE  (golden :547)
//    DMCDiagnosticsOff     Motor/vendor/DMCCOM.H:880   ACTIVE  (golden :553, :3147)
//    DMCGetControllerDesc  Motor/vendor/DMCCOM.H:1048  ACTIVE  (golden :3088)
//    DMCOpen               Motor/vendor/DMCCOM.H:444   ACTIVE  (golden :3097)
//    DMCReset              Motor/vendor/DMCCOM.H:668   ACTIVE  (golden :3109)
//    DMCClose              Motor/vendor/DMCCOM.H:535   ACTIVE  (golden :3148)
//  There is NO existing offline layer for these: Motor/vendor_offline_motionnet.cpp
//  covers 173 mn_* / _mnet_* / _l1xx_* entry points and, verified this wave,
//  `grep -n "DMC" Motor/vendor_offline_motionnet.cpp` returns ZERO lines.  The DMC
//  family is in no import library this build can consume either (golden links
//  Motor/Dmc32b.lib, a BCB6 OMF import library MinGW cannot read).
//  INTEGRATION REQUEST: add the 7 above to the offline vendor layer, each
//  returning its vendor FAILURE code so the driver takes golden's own "bring-up
//  failed" path.  DMCCOM.H's convention is 0 == success and negative DMCERROR_*
//  == failure, so DMCERROR_DRIVER is the honest return for all 7.  Do NOT return
//  0: that would tell this driver a Galil card is present and make it believe
//  position readings that never came from hardware.  With a failure return
//  bGali_CardInstall stays false (Open_GaliCard's `rc!=0` arm returns before
//  setting it), which is exactly the state golden reaches on a machine with no
//  card -- and every Gali_* member already has a `bGali_CardInstall==false`
//  simulate arm, so no new behaviour is invented.
//
//  =========================================================================
//  MECHANICAL ADAPTATIONS (no behaviour change).  Each was applied by script
//  with an ASSERTED occurrence count, so none was missed and none over-applied.
//  =========================================================================
//   1. `#pragma hdrstop` / `#pragma package(smart_init)` dropped (BCB6-only).
//   2. `__fastcall` dropped from the ctor definition (1 site).
//   3. FREE-FUNCTION `sprintf(charbuf, fmt, ...)` -> `W4G_sprintf(...)` at all
//      41 sites (counted, not estimated; 12 of the 41 pass a raw AnsiString --
//      SelMot or PRData -- to a `%s`, e.g. golden :814
//      `sprintf(str,"SP%s=%d;...", SelMot, Speed, ...)`).  That idiom works in
//      BCB6 only because its AnsiString is layout-compatible with char*; under
//      standard C++ it is undefined behaviour.  W4G_sprintf is a
//      variadic-template wrapper mapping AnsiString -> c_str() and passing
//      everything else through -- EXACTLY the mechanism this tree's own
//      vclcompat/AnsiString.h:137-179 already uses for the MEMBER `x.sprintf(...)`
//      form.  All 41 argument lists stay byte-for-byte golden, and the rename is
//      applied uniformly (not only to the 12) so no future edit can reintroduce
//      the UB.  The 35 MEMBER `x.sprintf(...)` calls are excluded by lookbehind
//      and are untouched (asserted: no `.W4G_sprintf(` exists in the output).
//   4. `Now().FormatString("hh:nn:ss")` -> `FormatDateTime("hh:nn:ss", Now())` at
//      both sites (golden :3609, :3887).  vclcompat's TDateTime has no
//      FormatString member; FormatDateTime is the same operation with the same
//      token set (vclcompat/TDateTime.h:67).
//   5. DMCCommand's 2nd parameter is `PSZ` == `char* FAR` (DMCCOM.H:43) and golden
//      hands it `AnsiString::c_str()` (const char*) and string literals.  A
//      self-referential function-like macro inserts the one `(PSZ)` cast at every
//      call site, leaving all 30 golden call texts untouched.  (A function-like
//      macro whose replacement list names itself is not re-expanded, so this
//      terminates -- it is not recursion.)
//   6. INCLUDES.  golden note.h + mymessbox.h + cMyDB.h -> canary_support.h,
//      which owns ShowErrorMessage / ShowMyMessage / ShowMotorErrorMessage /
//      MotorIndexToJamCode / MyDBIProcessNew / RecordProcess in this tree.  NOTE:
//      cMyDB.h must NOT also be included -- its MyDBIProcessNew (cMyDB.h:82) and
//      RecordProcess (cMyDB.h:122) AMBIGUATE against canary_support.h's, a hard
//      error (measured, not assumed).  golden main.h -> forms/fMain.h; golden
//      cOffSet.h -> forms/fOffSet.h; golden rs232.h -> atester_shims.h (COM2);
//      golden common.h kept (it declares BOTH MySleep and MySleepEx here,
//      common.h:365-366).  acarry_shims.h was tried for MySleepEx and REMOVED
//      again: it pulls aHotPlateSubstrate.h, whose `extern void StopAllMotor();`
//      (:923) makes golden's own `StopAllMotor();` calls at golden :2212 / :2416
//      AMBIGUOUS against this unit's `StopAllMotor(bool=true)` -- 2 hard errors,
//      measured, then removed by dropping the include.  This is the collision the
//      header banner warns about; see INTEGRATION REQUESTS.
//      DROPPED as dead for this TU -- golden uMotorTest.h, uhome.h, OCR.h,
//      iosetview.h, MyCCLinkSensor.h, myMN200motor.h, AdvMotDrv.h, AdvMotApi.h:
//      no symbol this file names comes from any of them (INSTALL_OCR /
//      eocrUninstal, the only OCR-looking symbols used, live in cmydef.h /
//      MachineType.h in BOTH trees).
//
//  =========================================================================
//  GOLDEN DEFECTS PRESERVED (found while translating; NOT fixed, per contract)
//  =========================================================================
//   (i)  golden :4158 `ShowErrorMessage("WAR0308", K_RETRY|K_SKIP, MMSystem,
//        "Gali_FindZPhase_446")` passes a STRING as argument 4, but parameter 4 is
//        `bool bDuplicateErr` (golden note.h:466, read directly; this port's
//        canary_support.h:66 is byte-identical).  The pointer decays to `true`, so
//        golden marks that alarm a duplicate error and silently DROPS the
//        "Gali_FindZPhase_446" tag.  Sibling calls in the same function pass the
//        tag correctly in slot 5 (e.g. golden :3756).  Reproduced exactly.
//        MEASURED, not assumed: g++ 6.3.0 emits NO diagnostic for this even at
//        -Wall (a string literal -> bool is a standard conversion; only clang's
//        -Wstring-conversion / -Waddress family would flag it).  So nothing in
//        the build will ever point at it -- which is exactly why it is written
//        down here.
//   (ii) golden :1647 `if(lSC==8)` is DEAD: it sits inside `if(lSC==100){...}`
//        opened at golden :1639, so lSC cannot be 8 there.  The "ER+OE Following
//        Error Exceeded" message is unreachable and the else message always wins.
//        Kept verbatim.
//  (iii) golden :755-758 / :951-954 / :1148-1151 / :2835-2838 / :2848-2851
//        `if(Pos==0){ Pos=0; }` -- no-op self-assignments (breakpoint anchors).
//        Kept.
//   (iv) golden :3202-3213 vs :3279-3290 -- CheckPos()'s 3-axis arm sets
//        `b2=true` while CheckPos_nnMode()'s BOTH 3-axis arms set `b2=false`,
//        which makes CheckPos_nnMode ALWAYS return false in 3-axis mode even
//        though the two functions are otherwise structurally identical.  Kept as
//        written.
//    (v) golden :3166 Gali_ReadEncoderPos derefs `Motor->Enable` with NO
//        `Motor!=NULL` guard, unlike the JimmyChiu 20250306 sites that DO guard
//        (golden :462, :658, :710, :2650).  CheckPos / CheckYPos /
//        CheckPos_nnMode and the five Gali_ReadEncoder* range checks share the
//        omission.  Kept; on this port `MOT[i].Motor` is NULL for every
//        unconfigured axis, so a caller reaching these before cinitial has run
//        faults exactly as golden would.
//   (vi) golden :4838-4846 (Gali_nnMode_Z1Z2_Down) reads Z1's real position into
//        FOUR different globals -- `MOT[MTestZ1].GetRealPos(&Z1DownSafe)` then
//        `MOT[MTestZ1].GetRealPos(&Z2DownSafe)`, and later `&Z1Down` then
//        `&Z2Down` -- i.e. the Z2 halves of the vector are computed from Z1's
//        axis, and golden :5047-5048 does the same with MTestZ2 into Z1Safe and
//        Z2Safe.  Looks like copy-paste, is load-bearing for the 9046LS "nn mode"
//        as shipped.  Kept exactly.
//
//  =========================================================================
//  #########################################################################
//  #  THE ONE THING THAT WILL BREAK TESTS WHEN THIS FILE IS REGISTERED:    #
//  #  GOLDEN'S REAL BODIES DEREFERENCE `Motor` WITHOUT A NULL CHECK, AND   #
//  #  THE STUBS THEY REPLACE HAD A `Motor==NULL` FAST PATH THAT TESTS      #
//  #  CURRENTLY ASSERT ON.  READ THIS BEFORE ADDING THE FILE TO CMake.     #
//  #########################################################################
//  `MOT[]` is a file-scope `TTrayMotor MOT[300]` (Motor/mymotor.h:385) and
//  TMyMotor's ctor never assigns `Motor`, so `MOT[i].Motor` is NULL for every
//  axis until cinitial wires a real HTMotor -- which nothing does in the offline
//  build.  The stubs being replaced here turned that into a benign answer:
//      Motor/mymotor.cpp:972  Gali_Two_ZAxis_Move(...) { return (Motor==NULL); }
//      Motor/mymotor.cpp:979  ISNormal()              { return (Motor==NULL); }
//      Motor/mymotor.cpp:989  GalilTwoY_Move(...)     { return (Motor==NULL); }
//      Motor/mymotor.cpp:948  Gali_MotMove(...)       { return false; }
//  GOLDEN DOES NOT.  Its first statement in Gali_MotMove (golden :941),
//  Gali_MotMoveNoWait (:1138), Gali_MotMoveSkipEncoder (:1305), Z1UpZ2Down1
//  (:1401), Z1DownZ2Up1 (:1770), Z1UpZ2Down2 (:2128), Z1DownZ2Up2 (:2331),
//  Gali_MotHome (:2698), Gali_MotHomeFindZ (:2750), Gali_Two_ZAxis_Move (:2809),
//  all six Gali_Jog* (:2921/:2943/:2965/:2988/:3010/:3034), Gali_MovePR (:3057),
//  Gali_SingalHome (:3599), Gali_FindZPhase (:3857), GalilTwoY_Move (:4394),
//  Gali_nnMode_Z1Z2_Down (:4798) and _Up (:5014) is
//  `if(Motor->CheckIsSafeDoorOpen())` -- an UNGUARDED deref.  Gali_ReadEncoderPos
//  (:3166) and the five Gali_ReadEncoder* range checks (:3299, :3341, :3355,
//  :3407, :3445) open with `Motor->Enable`, likewise unguarded, and that is how
//  ISNormal / ISZ1Up_Z2Down / ISZ1Down_Z2Up / ISZ1Up_Z2Up / ISZ1Down_Z2Down /
//  CheckPos / CheckYPos / CheckPos_nnMode reach it (they call
//  MOT[MTestZ1/Z2/Y1/Y2].Gali_ReadEncoder*).  Only Gali_Command (:462),
//  Gali_ScanMotStatus (:658), Gali_ScanAlarmStatus (:710), Gali_ReadPos (:2650)
//  and Gali_MotMove2 (:749) carry the JimmyChiu 20250306 `Motor!=NULL` guard,
//  and StopAllMotor (:4714, :4723) guards its own sweep.
//  SO: replacing the stubs turns "returns true/false offline" into "NULL deref".
//  KNOWN TEST EXPOSURE (grepped this wave; nearly every target in
//  tests/CMakeLists.txt LINK_GROUPs ht9045_motor, so this list is the set that
//  actually EXERCISES these entry points, not the set that links them):
//    tests/test_w7_s0_motor_convergence.cpp:174-181 -- asserts, by direct call,
//      Gali_Two_ZAxis_Move(...)==true, ISNormal()==true, GalilTwoY_Move(...)==true
//      "offline (Motor==NULL)".  All three become NULL derefs.
//    tests/test_w6_4b_front_rear_destroy.cpp:200,:211 -- asserts DoFront/
//      DoRearTestDestroyIC "PARKS at Task=100 (Gali_MotMove stub always false)".
//    tests/test_w6_4_tester.cpp:135,:198 -- relies on "Gali_Two_ZAxis_Move
//      succeeds immediately + ISNormal()==true (Motor==NULL fast-path)".
//    tests/test_atester_32site.cpp:159-161 -- same two fast paths, by comment.
//    tests/test_w906_doindexautoclean.cpp:121-123 -- same, by comment.
//  RECOMMENDED INTEGRATION ORDER (do NOT "fix" golden by adding NULL guards --
//  that would be a behaviour change this port is not allowed to make):
//    give the four index axes a real backend in the affected tests' setUp --
//    `MOT[MTestY1..MTestY2].Motor = new TMySimMotor();` with Enable=false (
//    Motor/mySimMotor.h already exists and is registered).  Enable==false takes
//    golden's OWN simulate arm in every one of these functions, which is the arm
//    the tests were approximating anyway, and it costs no golden edit.
//
//  =========================================================================
//  STATIC-INIT AUDIT (trap 4)
//  =========================================================================
//  This file defines NO object whose constructor touches a global.  Its
//  file-scope objects are: PODs only (bGali_CardInstall, GaliAxisAlarm[4], rc,
//  response[64], szBuffer[128], szBuffer1[128], hDmc, hWnd, TestYBuffer,
//  TestY2Buffer, GailAcSpeed, GailDcSpeed, GailAcSpeed2, GailDcSpeed2,
//  DelayCount, bGalilTwoZMoveFlag, bGalilTwoYMoveFlag, iStartRetry,
//  iPosDeadband, iEncoderTorence, iCheckZ, bMask[8] -- all constant-initialised),
//  one CRITICAL_SECTION (`g_cs`, POD), and three TQPF_Timer objects
//  (tGalilCmdTimer, tGalilTwoZMoveDelay, tGalilTwoYMoveDelay).  TQPF_Timer's ctor
//  (myTimer.h:24) only calls QueryPerformanceFrequency/Counter against its OWN
//  members -- it reads no global pointer -- so it is safe before main().  The
//  three TU-local shim objects (W4G_alarm_obj / W4G_fmain_ext / W4G_foffset_ext)
//  are aggregates whose ctors touch nothing but their own members.  Nothing here
//  dereferences a plan-section-8 NULL at static-init time.
//  NOTE ON g_cs -- a golden latent bug kept as-is: golden initialises the
//  critical section in the TMyGALILMotor CONSTRUCTOR (golden :104), so
//  Gali_Command's EnterCriticalSection (golden :461) would run on an
//  UNINITIALISED CRITICAL_SECTION if it were ever reached before any
//  TMyGALILMotor was constructed.  Not fixed.
// =============================================================================

#include "MachineDefine.h"          // de-VCL'd include hub (vclcompat umbrella + portable STL + using namespace std)
#include "Motor/mymotor.h"          // golden: "MyMotor.h"     -- TMyMotor / TTrayMotor + MOT[]
#include "Motor/myGALILmotor.h"     // this unit's own contract -- TMyGALILMotor + the 12 free functions
#include "Motor/GaliRoute.h"   //AI(W906-INDEXZ-1203) 20260929: the Index Z route seam (POD, no EtherCAT header) -- see the header and this file's EOF
#include "MachineType.h"            // golden: "MachineType.h" -- DEBUG_GALIL_CHECK_Y / USE_INDEX_ARM_AXES / MachineTypeChoice / Type_HT9046_LS
#include "cpublic.h"                // golden: "cpublic.h"     -- TMyStrQueue100 (QueueGalilCmd), GetTimeInfo
#include "myswitch.h"               // golden: "myswitch.h"    -- SW[] / SwAutoCCW / SwAutoCW / SwShuttleVibration* / SwLoaderVibration
#include "MyLaneIo.h"               // golden: "MyLaneIo.h"
#include "canary_support.h"         // golden: "note.h" + "mymessbox.h" + "cMyDB.h" -- ShowErrorMessage / ShowMyMessage / ShowMotorErrorMessage / MotorIndexToJamCode / MyDBIProcessNew / RecordProcess / LastSet / __FUNC__
#include "common.h"                 // golden: "common.h"      -- MySleep (common.cpp:1919), MySleepEx (common.h:365 / common.cpp:1912), ReadIniData / WriteIniData / CheckIniData, CheckAndReadIniDataGeneral, asGeneralPath, asIndexZphasePath
#include "cinitial.h"               // golden: "cinitial.h"    (GetIndexParm is NOT in this port's partial copy -- GATE(W4G-5))
#include "cmydef.h"                 // golden: "cmydef.h"      -- iHome / JamCode / MotErrPos / iGali_SpSpeed / iGali_VsSpeed / MyLongMask / index geometry globals / Gali_MaxAxis / GALI_PROTECT_MODE
#include "mysensor.h"               // golden: "mysensor.h"
#include "csystem.h"                // golden: "csystem.h"     -- SaveFile declaration (GATE(W4G-7))
#include "mycylin.h"                // golden: "mycylin.h"     -- Cylinder[] / C_HotplateVibration / C_TrayVibration
#include "asendic.h"                // golden: "asendic.h"     -- TrayMoveOut
#include "cprod.h"                  // Prod / Tech / TestIF_File / ArmSpeed / IndexStatus
#include "Config.h"                 // IniConfig
#include "CosFunction.h"            // CosFunction
#include "atester_shims.h"          // golden: "rs232.h"       -- COM2 (TCOM2Shim: bCCDDummyRum)
#include "forms/fMain.h"            // golden: "main.h"        -- fMain
#include "forms/fOffSet.h"          // golden: "cOffSet.h"     -- fOffSet

#include <windows.h>                // CRITICAL_SECTION / HWND
#include "DMCCOM.H"                 // Galil DMC32 SDK (READ-ONLY vendor header, Motor/vendor/): HANDLEDMC, DMCCommand, DMCOpen, DMCReset, DMCClose, DMCClear, DMCDiagnosticsOff, DMCGetControllerDesc
#include "dmcdrc.h"                 // Galil DMC32 SDK: the DMCERROR_* codes GetGalilErrString enumerates
#include <cstdio>
#include <cstdlib>
#include <cstring>

// ---------------------------------------------------------------------------
//  MECHANICAL ADAPTATION 3 -- free-function sprintf with raw AnsiString args.
//  Mirrors vclcompat/AnsiString.h's own conv() helper.  See banner item 3.
// ---------------------------------------------------------------------------
static inline const char* W4G_conv(const AnsiString& s) { return s.c_str(); }
template <class T> static inline T W4G_conv(T v) { return v; }
template <class... Args>
static inline int W4G_sprintf(char* buf, const char* fmt, Args... args)
{
    return ::sprintf(buf, fmt, W4G_conv(args)...);
}

// ---------------------------------------------------------------------------
//  MECHANICAL ADAPTATION 5 -- DMCCommand's 2nd parameter is PSZ (char*, NOT
//  const; DMCCOM.H:43) while golden hands it const char*.  Self-referential
//  function-like macro: the inner DMCCommand is not re-expanded, so every golden
//  call text stays verbatim and gains exactly one cast.
// ---------------------------------------------------------------------------
#define DMCCommand(h, cmd, resp, cb) DMCCommand((h), (PSZ)(cmd), (resp), (cb))

// ---------------------------------------------------------------------------
//  GATE(W4G-4) -- Alarm->Set.  See GATE REGISTER.  Follows the registered
#include "halarm.h"   //AI(W906-HALARM) 20260926: GATE(W4G-4) LIFTED -- 前提「golden 的 HAlarm 不在移植樹」已不成立（halarm.h／HAlarm.cpp，RULINGS_20260926 第 16 條）。原註：mycylin.cpp:122-133 idiom (local no-op for the HAlarm god-object).
// ---------------------------------------------------------------------------
#if 1   // GATE(W4G-4) LIFTED 20260926 (AI(W906-HALARM)): golden Motor/myGALILmotor.cpp:49 -- VERBATIM golden text
extern HAlarm *Alarm;
#else
struct W4G_HAlarmShim
{
    void Set(int /*iAlarmCode*/)   {}   // golden HAlarm::Set   -- see GATE(W4G-4)
    void Clear(int /*iAlarmCode*/) {}   // golden HAlarm::Clear -- not used by this unit; kept for shape
};
static W4G_HAlarmShim W4G_alarm_obj;
#define Alarm (&W4G_alarm_obj)
#endif

// ---------------------------------------------------------------------------
//  fMain / fOffSet MEMBERS this port's form facade does not carry.
//  forms/fMain.h's TfMain has no sb_Gali_LogClick and no AddIndexPosLog;
//  forms/fOffSet.h's TfOffSet has no bEnterSpecialOffset.  Both facade OBJECTS
//  are real and non-NULL (forms/fMain.cpp:385, forms/fOffSet.cpp:12), so this is
//  a MEMBER gap, not a plan-section-8 NULL.  Extended TU-local stand-ins +
//  #define -- exactly the atester.cpp:1645-1662 (W7T1_TCOM2Ext) idiom.
//  ABSENCE CLAIMS + GREPS (re-run at hand-off 2026-08-08 23:52; the FIRST claim
//  was CORRECTED by that re-run -- recorded rather than quietly reworded):
//    grep -rn "sb_Gali_LogClick\|AddIndexPosLog" --include=*.h --include=*.cpp D:/HT9045/HT9011UC_Cpp_V3.33.906.0/
//      -> sb_Gali_LogClick : ZERO hits outside this file.
//      -> AddIndexPosLog   : FOUR hits, ALL in cpublic.cpp -- :1809 prose, :1812
//         `#if 0 // TODO(GA1-B3): blocked by fMain->slIndexYMaxMinShift +
//         fMain->AddIndexPosLog (missing facade members)`, and :1822 / :1826
//         INSIDE that #if 0.  The claim "TfMain has no AddIndexPosLog member"
//         therefore STILL HOLDS -- cpublic.cpp independently documents the same
//         absence and gated itself for it -- but the earlier "ZERO hits"
//         phrasing was WRONG.  USEFUL COROLLARY: adding
//         AddIndexPosLog(AnsiString,bool=false) to TfMain also unblocks HALF of
//         cpublic.cpp's GA1-B3 gate (the other half is slIndexYMaxMinShift).
//    grep -rn "bEnterSpecialOffset" --include=*.h --include=*.cpp D:/HT9045/HT9011UC_Cpp_V3.33.906.0/
//      -> ZERO hits outside this file.
//  BEHAVIOUR DELTA: sb_Gali_LogClick() dumps the Galil command queue to a log
//  file from the UI (golden main.h) and is reached ONLY after 5000 consecutive
//  SC==100 command errors; it is a no-op here and the surrounding
//  `iHome=1; ... return false;` control flow is unchanged.  AddIndexPosLog is
//  called only from the INDEX_PROTECT_TMOVE regions, which this build does not
//  compile.  bEnterSpecialOffset is WRITTEN (false) 12 times by this file and
//  never read by it -- golden's readers live in cOffSet.cpp / atester.cpp, both
//  unported.  INTEGRATION: add the two methods to TfMain and the bool to
//  TfOffSet, then delete these two #defines.
// ---------------------------------------------------------------------------
struct W4G_TfMainExt
{
    void sb_Gali_LogClick()                                        {}  // golden main.h -- offline: no Galil-log dump
    void AddIndexPosLog(AnsiString /*s*/, bool /*bSave*/ = false)   {}  // golden main.h -- offline: no index-pos log (INDEX_PROTECT_TMOVE only)
};
static W4G_TfMainExt W4G_fmain_ext;
#ifdef fMain
#undef fMain
#endif
#define fMain (&W4G_fmain_ext)

struct W4G_TfOffSetExt
{
    bool bEnterSpecialOffset;                                          // golden cOffSet.h -- write-only from this unit
    W4G_TfOffSetExt() : bEnterSpecialOffset(false) {}
};
static W4G_TfOffSetExt W4G_foffset_ext;
#ifdef fOffSet
#undef fOffSet
#endif
#define fOffSet (&W4G_foffset_ext)

// ---------------------------------------------------------------------------
//  GATE(W4G-7) -- SaveFile.  See GATE REGISTER.
// ---------------------------------------------------------------------------
#if 0   // GATE(W4G-7): the real declaration is csystem.h:259 (golden's own home, reached through the #include above); the BODY is golden csystem.cpp:22179 and this port's csystem.cpp has not reached it
void SaveFile(AnsiString sPath, AnsiString sData);
#else
// GATE(W4G-7): golden csystem.cpp:22179-22198 reproduced VERBATIM below, TU-local
// so it cannot collide when csystem.cpp translates its own copy.  Every symbol it
// needs is REAL here: MyForceDirectories (common.h:341 / common.cpp:1806) and
// SystemYear/SystemMonth/SystemDate (cmydef.h:227 / cmydef.cpp:292).
static void W4G_SaveFile(AnsiString sPath, AnsiString sData)                    // golden csystem.cpp:22179
{
    FILE *P;
    AnsiString aPath, cPath, sFile;
    MyForceDirectories(sPath, "CSystem::SaveFile_1");
    cPath=sPath+"\\"+AnsiString((int)SystemYear)+"_"+AnsiString((int)SystemMonth);   // golden: sPath+"\\"+SystemYear+"_"+SystemMonth (BCB6 implicit Word->AnsiString)
    MyForceDirectories(cPath, "CSystem::SaveFile_2");

    sFile.sprintf("%4d%2d%2d.txt", SystemYear, SystemMonth, SystemDate);
    aPath=cPath+"\\"+sFile;

    P=fopen(aPath.c_str(), "a+");

    if(P!=NULL)
    {
        fputs(sData.c_str(), P);
        fputs("\n", P);
        fclose(P);
    }
}
#define SaveFile W4G_SaveFile
#endif

// ---------------------------------------------------------------------------
//  GATE(W4G-5) -- GetIndexParm.  See GATE REGISTER.
// ---------------------------------------------------------------------------
#if 1   // GATE(W4G-5): golden cinitial.h:30 -- VERBATIM golden declaration; body golden cinitial.cpp:13713   //AI(W906-I115B) 20260930: LIFTED -- the six calls (:2389 :2761 :3115 :3319 :5858 :6076) reach cinitial.cpp:16185 GetIndexParm (golden :13713); inert while bGali_CardInstall is false
extern void GetIndexParm();
#else
static void GetIndexParm() {}   // body golden cinitial.cpp:13713, outside this port's partial cinitial.cpp -- see GATE(W4G-5)
#endif

// =============================================================================
//  GOLDEN FILE-SCOPE OBJECTS -- golden Motor/myGALILmotor.cpp:33-54
// =============================================================================
bool bGali_CardInstall=false;
bool GaliAxisAlarm[4]={false,false,false,false};
long rc;
char response[64];
char szBuffer[128], szBuffer1[128];
HANDLEDMC hDmc;
HWND hWnd;
int TestYBuffer, TestY2Buffer;
int GailAcSpeed=20000000 ;
int GailDcSpeed=20000000 ;
int GailAcSpeed2=20000000;
int GailDcSpeed2=20000000;
#if 0   // GATE(W4G-2): golden Motor/myGALILmotor.cpp:45,46,48 -- VERBATIM golden text
extern int iEncoderTorence;
extern int iCheckZ;
extern char bMask[8];
#else
// GATE(W4G-2): this port defines all three `static` in Motor/mymotor.cpp
// (:113 iEncoderTorence=500, :114 iCheckZ=4000, :120 bMask[8]) -- invisible to
// this TU.  Same initialisers.  See GATE REGISTER for the iCheckZ delta.
static int iEncoderTorence = 500;                                               // Motor/mymotor.cpp:113 -- declared by golden here, never used by golden here
static int iCheckZ         = 4000;                                              // Motor/mymotor.cpp:114
static unsigned char bMask[8] = {0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80}; // Motor/mymotor.cpp:120
#endif
CRITICAL_SECTION g_cs;                                                          //ChungHung 20141015 add fix Gail command error
// ===========================================================================
//  ★ AI(W906-P0-5) 20260920: `g_cs` 的**初始化接縫** —— 缺的相依是 BDE。
//
//  ## 為什麼需要它（實測，不是預防性）
//
//  golden 只在**一個**地方呼叫 `InitializeCriticalSection(&g_cs)`：
//  `TMyGALILMotor::TMyGALILMotor()`（本檔 :989-991）。
//  也就是「有人 new 過 Galil 馬達」是 `Gali_Command` 可用的前提。
//
//  golden 有**三個** `new TMyGALILMotor(iGalilPort)`（golden cinitial.cpp
//  :3482 / :3719 / :3899），分別落在 `InitialMotorParameter()` 的三個分支。
//  **本樹只有第一個是活的**，另外兩個在 `cinitial.cpp` 的 GATE 2 裡面 ——
//  那個 gate 擋的是 BDE：`TTable` / `DataModule1->MotorTable` /
//  `CheckMotorValue(TTable*,...)`，全樹都不存在。那是**真正的「相依不存在」**。
//
//  而這台機器走的正好是被 GATE 2 擋掉的那一條：
//      IO_CARD_TYPE   == 1   (既不是 NewIO_MN200(2) 也不是 PCI_P64C64(3))
//      MOTION_CARD_TYPE == 1
//  （兩個值都是 20260920 23:2x 用 gdb 從 `wb_serve.exe` 讀出來的，不是推論。）
//  ⇒ 三個站點一個都不會跑 ⇒ **`g_cs` 全是 0**。
//
//  20260920 退掉 `mymotor.cpp` 的 48 個 Gali 樁之後，`Gali_Command` 的真本體
//  活了過來，它 :1350 就 `EnterCriticalSection(&g_cs)`：
//
//      #3 TMyMotor::Gali_Command
//      #4 InitialTestHeadMotorTask      (atester.cpp:5435，golden 自己的
//                                        `if(MOT[MTestY1].Motor!=NULL)` 之下)
//      #5 InitAllProcessTask
//      #6 ht9045::PumpInit
//      #7 main                          -> 0xC0000005 開機就死
//
//  ⚠⚠ **152 支 ctest 完全沒抓到**（失敗集合逐項等於基準 18 項）——
//     沒有任何測試涵蓋 `wb_serve.exe` 的 PumpInit 路徑。
//     連結綠、測試綠、exe 死。這是「build 綠 ≠ 接上了」最乾淨的實例。
//
//  ## 為什麼是這個修法
//
//  三個選項：
//    (a) 把樁放回去             -> 等於放棄使用者核准的 Q18(A)
//    (b) **在本 TU 補一次性初始化**（選這個）
//    (c) 在 `Gali_Command` 裡先判 `bGali_CardInstall` 再進臨界區
//                               -> 改了 golden 的結構，不忠實
//
//  (b) 重現的是 golden**保證**過的狀態（ctor 跑過 = g_cs 可用），
//  而本樹之所以做不到，純粹是因為 ctor 的呼叫端被 BDE 擋著。
//  等 GATE 2 解開（BDE 那層翻進來）之後，這個接縫會變成多餘的 ——
//  但它是**冪等**的，多跑一次 `InitializeCriticalSection` 也不會壞，
//  所以不需要跟著一起拆。
//
//  ⚠ SIOF：它是**本 TU 的靜態初始化器**，而 `Gali_Command` 的所有呼叫端
//    都在 `main()` 之後（上面那條堆疊就是），沒有任何靜態建構子會呼叫它。
//    ⇒ 不需要跟別的 TU 比初始化順序。
// ===========================================================================
//  ⛔ **刻意沒有解構子** —— 第一版我寫了 `~W906_GaliCsInit(){ DeleteCriticalSection(&g_cs); }`，
//     那是錯的方向：
//       * golden 的 `~TMyGALILMotor()`（本檔 :1012-1014）是**空的**，
//         golden 全樹沒有任何 `DeleteCriticalSection(&g_cs)`。
//       * `wb_serve` 是多執行緒的。程式結束時靜態解構先跑，若還有執行緒
//         停在 `Gali_Command` 的 `EnterCriticalSection(&g_cs)`，
//         那就是對**已刪除**的 CRITICAL_SECTION 操作 —— 又是一次 0xC0000005，
//         而且發生在關機路徑上，最難重現。
//       * 不刪的代價只是「行程結束時少釋放一個核心物件」，OS 自己回收。
//     ⇒ 兩個方向的代價不對稱，照 golden 不刪。
namespace {
struct W906_GaliCsInit
{
    W906_GaliCsInit() { InitializeCriticalSection(&g_cs); }
};
W906_GaliCsInit g_w906GaliCsInit;
}   // namespace
#if 0   // GATE(W4G-1): golden Motor/myGALILmotor.cpp:50 -- VERBATIM golden text (golden's SOLE definition; initialiser {true, true})
bool IndexZCanMove[2]={true, true};
#else
extern bool IndexZCanMove[2];   // GATE(W4G-1): already DEFINED -- wrongly, as {false,false} -- at ainarm9045_w7_shims.cpp:47.  See GATE REGISTER.
#endif

#if 0   // GATE(W4G-6): golden Motor/myGALILmotor.cpp:52-54 -- VERBATIM golden text; the bodies are golden Motor/mymotor.cpp:67 / :117 / :173
bool CheckTestZ(AnsiString sFunc);
bool CheckTestZ1(AnsiString sFunc);
bool CheckTestZ2(AnsiString sFunc);
#else
// GATE(W4G-6): golden Motor/mymotor.cpp:67-226 reproduced BYTE-FOR-BYTE below
// (copied programmatically out of the cp950 source), made TU-local so it cannot
// collide when Motor/mymotor.cpp translates its own copies.  See GATE REGISTER.
static bool W4G_CheckTestZ (AnsiString sFunc);
static bool W4G_CheckTestZ1(AnsiString sFunc);
static bool W4G_CheckTestZ2(AnsiString sFunc);
#define CheckTestZ  W4G_CheckTestZ
#define CheckTestZ1 W4G_CheckTestZ1
#define CheckTestZ2 W4G_CheckTestZ2

static bool W4G_CheckTestZ(AnsiString sFunc)                                               //Steven 20141007 : 換位置
{
    AnsiString Str;
    static int iRetryCT=0;
    bool flag1=false, flag2=false, flag3=false, flag4=false;
    long lPos[4]={0, 0, 0, 0};                                                  //kevin 20150915

    lPos[0]=MOT[MTestZ1].Gali_ReadPos();
    if(MOT[MTestZ1].Gali_ReadEncoderInRandgeMinLimit(lPos[0]))
        flag1=true;

    lPos[1]=MOT[MTestZ2].Gali_ReadPos();
    if(MOT[MTestZ2].Gali_ReadEncoderInRandgeMinLimit(lPos[1]))
        flag2=true;

    lPos[2]=MOT[MTestY1].Gali_ReadPos();
    if(MOT[MTestY1].Gali_ReadEncoderInRandgeMinLimit(lPos[2]))
        flag3=true;

    if(USE_INDEX_ARM_AXES==IndexArm_3_Axis)                                     //JimmyChiu 20220708 : add Index Arm Axis
    {
        lPos[3]=0;
        flag4=true;
    }
    else
    {
        lPos[3]=MOT[MTestY2].Gali_ReadPos();
        if(MOT[MTestY2].Gali_ReadEncoderInRandgeMinLimit(lPos[3]))
            flag4=true;
    }

    if(flag1 && flag2 && flag3 && flag4)
    {
        iRetryCT=0;
        return true;
    }
    else
    {
        iRetryCT++;
        if(iRetryCT>150)                                                        //kevin 20130312100
        {
            Str.sprintf("CheckTestZ(%s)", sFunc);
            ShowIndexMotorError(Str);
            iRetryCT=0;                                                         //kevin 20110628 發生alarm 需清為0否則要關程式
            RecordIndexPositionError(Str, flag1, flag2, flag3, flag4, &lPos[0]); //kevin 20150915 record
        }
        return false;
    }
}
//==============================================================================
static bool W4G_CheckTestZ1(AnsiString sFunc)                                              //Steven 20141007 : 換位置
{
    static int iRetryCT=0;
    bool flag1=false, flag2=false, flag3=false, flag4=false;
    long lPos[4]={0, 0, 0, 0};                                                  //kevin 20150915
    AnsiString S;

    lPos[0]=MOT[MTestZ1].Gali_ReadPos();
    if(MOT[MTestZ1].Gali_ReadEncoderInRandgeMinLimit(lPos[0]))
        flag1=true;

    flag2=true;
    lPos[1]=0;

    lPos[2]=MOT[MTestY1].Gali_ReadPos();
    if(MOT[MTestY1].Gali_ReadEncoderInRandgeMinLimit(lPos[2]))
        flag3=true;

    if(USE_INDEX_ARM_AXES==IndexArm_3_Axis)                                     //JimmyChiu 20220708 : add Index Arm Axis
    {
        lPos[3]=0;
        flag4=true;
    }
    else
    {
        lPos[3]=MOT[MTestY2].Gali_ReadPos();
        if(MOT[MTestY2].Gali_ReadEncoderInRandgeMinLimit(lPos[3]))
            flag4=true;
    }

    if(flag1 && flag2 && flag3 && flag4)
    {
#ifdef DEBUG
        long Pos=MOT[MTestZ1].Gali_ReadPos();
        long Pos1=MOT[MTestZ1].Gali_ReadEncoderPos();
        S.sprintf("%d, %d, %d, %d", Pos, Pos1, Pos-Pos1, iRetryCT);
        fMain->lbEnCoder0->Caption=S;
#endif
        iRetryCT=0;
        return true;
    }
    else
    {
        iRetryCT++;
        MySleepEx(5, true);
        if(iRetryCT>100)
        {
            S.sprintf("CheckTestZ1(%s)", sFunc);
            ShowIndexMotorError(S);
            iRetryCT=0;
            RecordIndexPositionError(S, flag1, flag2, flag3, flag4, &lPos[0]); //kevin 20150915 record
        }
        return false;
    }
}
//==============================================================================
static bool W4G_CheckTestZ2(AnsiString sFunc)                                              //Steven 20141007 : 換位置
{
    static int iRetryCT=0;
    bool flag1=false, flag2=false, flag3=false, flag4=false;
    long lPos[4]={0, 0, 0, 0};                                             //kevin 20150915
    AnsiString S;

    flag1=true;

    lPos[1]=MOT[MTestZ2].Gali_ReadPos();
    if(MOT[MTestZ2].Gali_ReadEncoderInRandgeMinLimit(lPos[1]))
        flag2=true;

    lPos[2]=MOT[MTestY1].Gali_ReadPos();
    if(MOT[MTestY1].Gali_ReadEncoderInRandgeMinLimit(lPos[2]))
        flag3=true;

    if(USE_INDEX_ARM_AXES==IndexArm_3_Axis)                                     //JimmyChiu 20220708 : add Index Arm Axis
    {
        lPos[3]=0;
        flag4=true;
    }
    else
    {
        lPos[3]=MOT[MTestY2].Gali_ReadPos();
        if(MOT[MTestY2].Gali_ReadEncoderInRandgeMinLimit(lPos[3]))
            flag4=true;
    }

    if(flag1 && flag2 && flag3 && flag4)
    {
#ifdef DEGBU
        long Pos=MOT[MTestZ2].Gali_ReadPos();
        long Pos1=MOT[MTestZ2].Gali_ReadEncoderPos();
        S.sprintf("%d, %d, %d, %d", Pos, Pos1, Pos-Pos1, iRetryCT);
        fMain->lbEnCoder1->Caption=S;
#endif
        iRetryCT=0;
        return true;
    }
    else
    {
        iRetryCT++;
        MySleepEx(5, true);
        if(iRetryCT>100)
        {
            S.sprintf("CheckTestZ2(%s)", sFunc);
            ShowIndexMotorError(S);
            iRetryCT=0;
            RecordIndexPositionError(S, flag1, flag2, flag3, flag4, &lPos[0]);  //kevin 20150915 record
        }
        return false;
    }
}
#endif


//------------------------------------------------------------------------------
static char GetGalilAxisChar(unsigned int iPort)
{
    switch(iPort)
    {
        case 0: return 'x';
        case 1: return 'y';
        case 2: return 'z';
        case 3: return 'w';
        default:return 'x';
    }
}
//------------------------------------------------------------------------------
void SetGaliRate(int Scale)
{
    GailAcSpeed =450000*Scale;                                                  // 加減速 - 慢
    GailDcSpeed =450000*Scale;
    GailAcSpeed2=550000*Scale;
    GailDcSpeed2=550000*Scale;                                                  // 加減速 - 快
}
//------------------------------------------------------------------------------
static int DelayCount=2;
void InitialSetUpGaliDelayCount(int iDelayCT)
{
    DelayCount=iDelayCT;
}
//---------------------------------------------------------------------------
void InitialGaliDelayCount()
{
    DelayCount=5;
    int ret;
    AnsiString S="GaliDelayCount";
    ret=CheckAndReadIniDataGeneral("VENDER",S.c_str(),-999999);
    if(ret==-999999)
    {
        WriteIniDataGeneral("VENDER", S.c_str(), DelayCount);
    }
    else
    {
        if(ret<20 && ret>=1)
            DelayCount=ret;
    }
}
//==============================================================================
TMyGALILMotor::TMyGALILMotor(int Addr)
{
    InitializeCriticalSection(&g_cs);                                           //ChungHung 20141015 add fix Gail command error
    Address=0;
    iBoardID=0;
    iPortID=0;

    if(Addr>=0)
    {
        if(Addr<10)
        {
            iPortID=Addr;
        }
        else
        {
            iBoardID=Addr/10;
            iPortID=Addr%10;
        }
    }

    MotorType=Servo_Motor;
    EncoderType=0;
}
//==============================================================================
TMyGALILMotor::~TMyGALILMotor()
{
}
//==============================================================================
int TMyGALILMotor::InitMotor(int IoAddress)
{
    return true;
}
//==============================================================================
void TMyGALILMotor::SetSpeed(unsigned int x, bool bSetJog)
{
    iSpeed=x;
}
//==============================================================================
void TMyGALILMotor::SetInitSpeed(unsigned int x)
{
    InitSpeed=x;
}
//==============================================================================
void TMyGALILMotor::SetServoAlarmOn(bool Value)
{
    PServoAlarmOn=Value;
}
//==============================================================================
int TMyGALILMotor::ReadPos()
{
    return ReadRealPos()*GearRatio;
}
//==============================================================================
void TMyGALILMotor::ScanMotorStatus(bool *Led)
{
    if(Led==NULL)
        return;

    Led[iCwLed]=false;
    Led[iHomeLed]=false;
    Led[iCcwLed]=false;
    Led[iEmgLed]=false;
    Led[iSoftcwLed]=false;
    Led[iSoftccwLed]=false;
    Led[iServoalarmLed]=false;
    Led[iServoOn]=Enable;
    Led[iInposLed]=MotionDone();
    Led[iAlarmLed]=GetAlarm();
}
//==============================================================================
bool TMyGALILMotor::MoveToPos(int Tar)
{
    if(!Enable)
        return true;

    if(!bGali_CardInstall)
        return false;

    int Pos=Tar;
    if(Direction==false)
        Pos=-Pos;

    unsigned int Spd=(iSpeed>0)?iSpeed:1;
    int Acc=(dAcc>0)?(int)dAcc:GailAcSpeed2;
    int Dec=(dDec>0)?(int)dDec:GailDcSpeed2;
    char Axis=GetGalilAxisChar(iPortID);

    AnsiString Cmd;
    Cmd.sprintf("SP%c=%u;AC%c=%d;DC%c=%d;PA%c=%d;BG%c;", Axis, Spd, Axis, Acc, Axis, Dec, Axis, Pos, Axis);
    return (DMCCommand(hDmc, Cmd.c_str(), szBuffer1, sizeof(szBuffer1))==0);
}
//==============================================================================
void TMyGALILMotor::Stop()
{
    if(!Enable || !bGali_CardInstall)
        return;

    char Axis=GetGalilAxisChar(iPortID);
    AnsiString Cmd;
    Cmd.sprintf("ST%c;", Axis);
    DMCCommand(hDmc, Cmd.c_str(), szBuffer1, sizeof(szBuffer1));
}
//==============================================================================
void TMyGALILMotor::DecStop()
{
    Stop();
}
//==============================================================================
bool TMyGALILMotor::JogP()
{
    if(!Enable)
        return true;

    if(!bGali_CardInstall)
        return false;

    int Spd=(iSpeed>0)?(int)iSpeed:(int)PJogHighSpeed;
    if(Spd<=0)
        Spd=1;
    if(Direction==false)
        Spd=-Spd;

    char Axis=GetGalilAxisChar(iPortID);
    AnsiString Cmd;
    Cmd.sprintf("SP%c=%d;AC%c=%d;DC%c=%d;JG%c=%d;BG%c;", Axis, abs(Spd), Axis, GailAcSpeed, Axis, GailDcSpeed, Axis, Spd, Axis);
    return (DMCCommand(hDmc, Cmd.c_str(), szBuffer1, sizeof(szBuffer1))==0);
}
//==============================================================================
bool TMyGALILMotor::JogN()
{
    if(!Enable)
        return true;

    if(!bGali_CardInstall)
        return false;

    int Spd=(iSpeed>0)?(int)iSpeed:(int)PJogHighSpeed;
    if(Spd<=0)
        Spd=1;
    if(Direction)
        Spd=-Spd;

    char Axis=GetGalilAxisChar(iPortID);
    AnsiString Cmd;
    Cmd.sprintf("SP%c=%d;AC%c=%d;DC%c=%d;JG%c=%d;BG%c;", Axis, abs(Spd), Axis, GailAcSpeed, Axis, GailDcSpeed, Axis, Spd, Axis);
    return (DMCCommand(hDmc, Cmd.c_str(), szBuffer1, sizeof(szBuffer1))==0);
}
//==============================================================================
bool TMyGALILMotor::HomeObject()
{
    if(!Enable)
        return true;

    if(!bGali_CardInstall)
        return false;

    char Axis=GetGalilAxisChar(iPortID);
    AnsiString Cmd;
    Cmd.sprintf("HM%c;BG%c;", Axis, Axis);
    return (DMCCommand(hDmc, Cmd.c_str(), szBuffer1, sizeof(szBuffer1))==0);
}
//==============================================================================
void TMyGALILMotor::SetRange(unsigned int a)
{
    Range=a;
}
//==============================================================================
void TMyGALILMotor::SetRate(unsigned int a)
{
    Rate=a;
}
//==============================================================================
bool TMyGALILMotor::GetAlarm(void)
{
    if(!Enable || !bGali_CardInstall)
        return false;

    char Axis=GetGalilAxisChar(iPortID);
    AnsiString Cmd;
    Cmd.sprintf("MG_SC%c", Axis);
    if(DMCCommand(hDmc, Cmd.c_str(), szBuffer1, sizeof(szBuffer1))!=0)
        return false;

    return (atoi(szBuffer1)!=0);
}
//==============================================================================
bool TMyGALILMotor::HomeFlag(void)
{
    return MotionDone();
}
//==============================================================================
bool TMyGALILMotor::ResetPos(int Pulse)
{
    SetCommand(Pulse);
    SetPosition(Pulse);
    return true;
}
//==============================================================================
bool TMyGALILMotor::MotionDone()
{
    if(!Enable || !bGali_CardInstall)
        return true;

    AnsiString Cmd;
    char Axis=GetGalilAxisChar(iPortID);
    Cmd.sprintf("MG_BG%c", Axis);

    if(DMCCommand(hDmc, Cmd.c_str(), szBuffer1, sizeof(szBuffer1))!=0)
        return true;

    return (atoi(szBuffer1)==0);
}
//==============================================================================
int TMyGALILMotor::ReadRealPos()
{
    if(!Enable || !bGali_CardInstall)
        return 0;

    AnsiString Cmd;
    char Axis=GetGalilAxisChar(iPortID);
    Cmd.sprintf("MG _TP%c", Axis);

    if(DMCCommand(hDmc, Cmd.c_str(), szBuffer1, sizeof(szBuffer1))!=0)
        return 0;

    return atol(szBuffer1);
}
//==============================================================================
int TMyGALILMotor::ReadEnCoderRealPos()
{
    if(!Enable || !bGali_CardInstall)
        return 0;

    AnsiString Cmd;
    char Axis=GetGalilAxisChar(iPortID);
    Cmd.sprintf("MG _TE%c", Axis);

    if(DMCCommand(hDmc, Cmd.c_str(), szBuffer1, sizeof(szBuffer1))!=0)
        return 0;

    return atol(szBuffer1);
}
//==============================================================================
int TMyGALILMotor::SetCommand(int p)
{
    if(!Enable || !bGali_CardInstall)
        return 0;

    char Axis=GetGalilAxisChar(iPortID);
    AnsiString Cmd;
    Cmd.sprintf("DP%c=%d;", Axis, p);
    return DMCCommand(hDmc, Cmd.c_str(), szBuffer1, sizeof(szBuffer1));
}
//==============================================================================
int TMyGALILMotor::SetPosition(int p)
{
    if(!Enable || !bGali_CardInstall)
        return 0;

    char Axis=GetGalilAxisChar(iPortID);
    AnsiString Cmd;
    Cmd.sprintf("DE%c=%d;", Axis, p);
    return DMCCommand(hDmc, Cmd.c_str(), szBuffer1, sizeof(szBuffer1));
}
//==============================================================================
void TMyGALILMotor::SetServoOn(bool IsOn)
{
    if(!bGali_CardInstall)
        return;

    char Axis=GetGalilAxisChar(iPortID);
    AnsiString Cmd;
    if(IsOn)
        Cmd.sprintf("SH%c;", Axis);
    else
        Cmd.sprintf("MO%c;", Axis);

    DMCCommand(hDmc, Cmd.c_str(), szBuffer1, sizeof(szBuffer1));
}
//==============================================================================
void TMyGALILMotor::SetSoftLimit(int iPLimit, int iNLimit)
{
    PSoftLimitP=iPLimit;
    PSoftLimitN=iNLimit;
}
//==============================================================================
AnsiString GetGalilErrString(long RC, AnsiString FunctionName)                  //Steven 20151117 : Galil Error Message
{
    AnsiString Str, ErrorString;

    if(RC==DMCERROR_TIMEOUT                             ) ErrorString="DMCERROR_TIMEOUT";
    if(RC==DMCERROR_COMMAND                             ) ErrorString="DMCERROR_COMMAND";
    if(RC==DMCERROR_CONTROLLER                          ) ErrorString="DMCERROR_CONTROLLER";
    if(RC==DMCERROR_FILE                                ) ErrorString="DMCERROR_FILE";
    if(RC==DMCERROR_DRIVER                              ) ErrorString="DMCERROR_DRIVER";
    if(RC==DMCERROR_HANDLE                              ) ErrorString="DMCERROR_HANDLE";
    if(RC==DMCERROR_HMODULE                             ) ErrorString="DMCERROR_HMODULE";
    if(RC==DMCERROR_MEMORY                              ) ErrorString="DMCERROR_MEMORY";
    if(RC==DMCERROR_BUFFERFULL                          ) ErrorString="DMCERROR_BUFFERFULL";
    if(RC==DMCERROR_RESPONSEDATA                        ) ErrorString="DMCERROR_RESPONSEDATA";
    if(RC==DMCERROR_DMA                                 ) ErrorString="DMCERROR_DMA";
    if(RC==DMCERROR_ARGUMENT                            ) ErrorString="DMCERROR_ARGUMENT";
    if(RC==DMCERROR_DATARECORD                          ) ErrorString="DMCERROR_DATARECORD";
    if(RC==DMCERROR_DOWNLOAD                            ) ErrorString="DMCERROR_DOWNLOAD";
    if(RC==DMCERROR_FIRMWARE                            ) ErrorString="DMCERROR_FIRMWARE";
    if(RC==DMCERROR_CONVERSION                          ) ErrorString="DMCERROR_CONVERSION";
    if(RC==DMCERROR_RESOURCE                            ) ErrorString="DMCERROR_RESOURCE";
    if(RC==DMCERROR_REGISTRY                            ) ErrorString="DMCERROR_REGISTRY";
    if(RC==DMCERROR_BUSY                                ) ErrorString="DMCERROR_BUSY";
    if(RC==DMCERROR_DEVICE_DISCONNECTED                 ) ErrorString="DMCERROR_DEVICE_DISCONNECTED";
    if(RC==DMCERROR_TIMEING_ERROR                       ) ErrorString="DMCERROR_TIMEING_ERROR";
    if(RC==DMCERROR_WRITEBUFFER_TOO_LARGE               ) ErrorString="DMCERROR_WRITEBUFFER_TOO_LARGE";
    if(RC==DMCERROR_NO_MODIFY_PNP_CONTROLLER            ) ErrorString="DMCERROR_NO_MODIFY_PNP_CONTROLLER";
    if(RC==DMCERROR_FUNCTION_OBSOLETE                   ) ErrorString="DMCERROR_FUNCTION_OBSOLETE";
    if(RC==DMCERROR_STREAMING_COMMAND_IN_PROGRESS       ) ErrorString="DMCERROR_STREAMING_COMMAND_IN_PROGRESS";
    if(RC==DMCERROR_DEVICEDRIVER_VERSION_TOO_OLD        ) ErrorString="DMCERROR_DEVICEDRIVER_VERSION_TOO_OLD";
    if(RC==DMCERROR_STREAMING_COMMAND_MUST_BE_SOLITARY  ) ErrorString="DMCERROR_STREAMING_COMMAND_MUST_BE_SOLITARY";
    if(RC==DMCERROR_FIRMWARE_VERSION_TOO_OLD            ) ErrorString="DMCERROR_FIRMWARE_VERSION_TOO_OLD";
    if(RC==DMCERROR_ETHERNET_NO_MORE_HANDLES            ) ErrorString="DMCERROR_ETHERNET_NO_MORE_HANDLES";
    if(RC==DMCERROR_NETWORK_UNREACHABLE                 ) ErrorString="DMCERROR_NETWORK_UNREACHABLE";

    Str.sprintf("%s Err RC= %d (%s)", FunctionName, RC, ErrorString);
    return Str;
}
//------------------------------------------------------------------------------
TQPF_Timer tGalilCmdTimer;
long TMyMotor::Gali_Command(AnsiString Data, AnsiString sFunc)
{
    // AI(W906-P0-5) 20260920: 離線守衛 —— 值與退休前 mymotor.cpp 的樁**完全相同**，所以 Motor==NULL 的可觀察行為一格沒變。
    if(Motor==NULL) return 0;
    static AnsiString OldData="";
    AnsiString Data1=Data;
    AnsiString Str;
    int iTryCount=0;
    int iErrCnt=0;
    AnsiString sFileName;                                                       //kevin 20150908
    AnsiString sBuffer;
    int iOldrc=0;                                                               //JerryYang 20180411 (jou) : 修改Galil command error log記錄方式, 不要一直重複記錄相同的alarm
    static int iGalilErrCnt=0;

    if(OldData!=Data)                                                           //Steven 20200320 : 紀錄Galil命令
    {
        if(Data=="TSW" || Data=="TSX" || Data=="TSY" || Data=="TSZ" ||          //JerryYang 20241129 : fix Galil log
           Data=="MG_MOW" || Data=="MG_MOX" || Data=="MG_MOY" || Data=="MG_MOZ" ||
           Data=="TI" ||
           Data=="TPW" || Data=="TPX" || Data=="TPY" || Data=="TPZ" ||
           Data=="TDW" || Data=="TDX" || Data=="TDY" || Data=="TDZ" ||
           Data=="SC" ||
           Data=="MG_BGw" || Data=="MG_BGx" || Data=="MG_BGy" || Data=="MG_BGz" ||      //JerryYang 20250826 : fix galil log
           Data=="MG_BGW" || Data=="MG_BGX" || Data=="MG_BGY" || Data=="MG_BGZ" ||
           Data=="MG_SCy" || Data=="MG_SCz")
        {
        }
        else
        {
            QueueGalilCmd.Add(Data, sFunc);                                     //JerryYang 20250826 : fix galil log
        }

        OldData=Data;
    }

    EnterCriticalSection(&g_cs);                                                //ChungHung 20141015 add fix Gail command error
    if(Motor!=NULL &&                                                           //JimmyChiu 20250306 : 避免還沒初始化就操作馬達
       Motor->Enable && bGali_CardInstall==true)
    {
        while(1)
        {
            #ifndef SOFT_SIMULTE
                rc=DMCCommand(hDmc, Data1.c_str(), szBuffer, sizeof(szBuffer));
            #endif
            if(rc!=0)
            {
                iTryCount++;
                MySleepEx(1, true);
                if(rc==iOldrc)                                                  //JerryYang 20180411 (jou) : 修改Galil command error log記錄方式, 不要一直重複記錄相同的alarm
                {
                }
                else
                {
                    if(iOldrc!=0)
                    {
                        Str=GetGalilErrString(iOldrc, "Gail Command");
                        MyDBIProcessNew("Motion", "WAR2201", "Gail Command", Str+","+AnsiString(Data1)+",ErrCnt:"+IntToStr(iErrCnt)+",Func:"+sFunc);
                        DMCCommand(hDmc, "TC1", szBuffer, sizeof(szBuffer));    /// TC :   要查表對GAIL ALARM    TC1:有詳細說明會被客戶追著打
                        MyDBIProcessNew("Motion", "WAR2202", "Gail TC1", AnsiString(szBuffer)+",ErrCnt:"+IntToStr(iErrCnt));
                        DMCCommand(hDmc, "VS0;SP0,0,0,0;", szBuffer1, sizeof(szBuffer1));   //Steven 20220309 : 避免Galil Command Error時, 不能Alarm
                        SystemStart=false;
                        fAllMotorHome=false;
                        ShowErrorMessage("WAR1635", 0, MMSystem, 0, Str);
                        iErrCnt=0;
                        return 0;                                               //JerryYang 20240111 : 異常了要return掉
                    }
                    iOldrc=rc;
                }

                if(rc==-1)                                                      //JerryYang 20181101 10分鐘內發生3次error要跳alarm
                {
                    if(USE_INDEX_ARM_AXES==IndexArm_4_Axis &&
                       Data1.Pos("LMXYZW")==1)                                  //JerryYang 20240111 : 新增Galil四軸保護，四軸同動指令回傳timeout，不要重複送一次指令
                    {
                        MyDBIProcessNew("Motion", "WAR2203", "Gail Command", AnsiString(Data1)+",Func:"+sFunc);
                        DMCCommand(hDmc, "VS0;SP0,0,0,0;", szBuffer1, sizeof(szBuffer1));   //Steven 20220309 : 避免Galil Command Error時, 不能Alarm
                        SystemStart=false;
                        fAllMotorHome=false;
                        ShowErrorMessage("WAR1635", 0, MMSystem, 0, Str);
                        return 0;
                    }

                    iGalilErrCnt++;
                    if(iGalilErrCnt==1)
                    {
                        tGalilCmdTimer.LatchCycleTimeSec(true);
                    }
                    else if(iGalilErrCnt==3)
                    {
                        iGalilErrCnt=0;
                        if(tGalilCmdTimer.LatchCycleTimeSec()<600)
                        {
                            ShowErrorMessage("WAR0348", 0, MMSystem, 0, "");
                        }
                    }
                }

                iErrCnt++;
                QueueGalilCmd.SafeData();                                       //JerryYang 20250826 : fix galil log  //Steven 20200320 : 紀錄Galil命令
                if(iTryCount<150)                                               //JerryYang 20180508 (jou) : 因index撞機所以修改300 -> 150
                    continue;
            }
            else
            {
                if(iOldrc!=0)
                {
                    Str=GetGalilErrString(iOldrc, "Gail Command");
                    MyDBIProcessNew("Motion", "WAR2201", "Gail Command", Str+","+AnsiString(Data1)+",ErrCnt:"+IntToStr(iErrCnt)+",Func:"+sFunc);
                    DMCCommand(hDmc, "TC1", szBuffer, sizeof(szBuffer));        /// TC :   要查表對GAIL ALARM    TC1:有詳細說明會被客戶追著打
                    MyDBIProcessNew("Motion", "WAR2202", "Gail TC1", AnsiString(szBuffer)+",ErrCnt:"+IntToStr(iErrCnt));
                }
            }

            if(rc!=0)
            {
                SoftStop=true;
                SystemStart=false;
                Str=GetGalilErrString(rc, "Gail Command");
                MyDBIProcessNew("Motion", "WAR2203", "Gail Command Err", AnsiString(Data1)+",ErrCnt:"+IntToStr(iTryCount)+",Func:"+sFunc);
                ShowErrorMessage("WAR1635", 0, MMSystem, 0, Str);
                fAllMotorHome=false;
                DMCClear(hDmc);                                                 //kevin 20150909
                LeaveCriticalSection(&g_cs);                                    //ChungHung 20141015 add fix Gail command error
                return 0;
            }
            else
            {
                DMCDiagnosticsOff(hDmc);                                        //kevin 20150909 記錄 GAIL通訊
                if(Data=="ST")
                {
                    MOT[MTestY1].MovFlag=false;
                    if(USE_INDEX_ARM_AXES==IndexArm_4_Axis)
                    {
                        MOT[MTestY2].MovFlag=false;
                    }
                    MOT[MTestZ1].MovFlag=false;
                    MOT[MTestZ2].MovFlag=false;

                    MOT[MTestY1].bZ1Z2Exute=false;                              //kevin 20110630
                    if(USE_INDEX_ARM_AXES==IndexArm_4_Axis)
                    {
                        MOT[MTestY2].bZ1Z2Exute=false;                              //kevin 20110630
                    }
                }
                LeaveCriticalSection(&g_cs);                                    //ChungHung 20141015 add fix Gail command error
                return atol(szBuffer);
            }
        }
    }
    else
    {
        if(Data=="ST")
        {
            MOT[MTestY1].MovFlag=false;
            if(USE_INDEX_ARM_AXES==IndexArm_4_Axis)
            {
                MOT[MTestY2].MovFlag=false;
            }
            MOT[MTestZ1].MovFlag=false;
            MOT[MTestZ2].MovFlag=false;

            MOT[MTestY1].bZ1Z2Exute=false;                                      //kevin 20110630
            if(USE_INDEX_ARM_AXES==IndexArm_4_Axis)
            {
                MOT[MTestY2].bZ1Z2Exute=false;                              //kevin 20110630
            }
        }  { long w906r=0; if(W906_GaliRouteCommand(Mot_Name, Data.c_str(), &w906r)) { LeaveCriticalSection(&g_cs); return w906r; } }   //AI(W906-INDEXZ-1203) 20260929: no-card branch: after golden's own "ST" clearing (and the QueueGalilCmd log above), a routed axis's string goes to the 1203 route, which answers what atol(DMC reply) would be; not claimed / no route = golden 0
    }
    LeaveCriticalSection(&g_cs);                                                //ChungHung 20141015 add fix Gail command error
    return 0;
}
//------------------------------------------------------------------------------
void TMyMotor::Gali_ScanMotStatusTIMO()
{
    // AI(W906-P0-5) 20260920: 離線守衛 —— 值與退休前 mymotor.cpp 的樁**完全相同**，所以 Motor==NULL 的可觀察行為一格沒變。
    if(Motor==NULL) return;
    BYTE TIData;
    char Gali_TiData[]={1, 3, 5, 7};
    int Sel=0;
    AnsiString GetMot;
    GetMot=Gali_GetMOT(Mot_Name);

    if(Mot_Name==MTestY1)
    {
        Sel=0;
    }
    else if(Mot_Name==MTestZ1)
    {
        Sel=1;
    }
    else if(Mot_Name==MTestZ2)
    {
        Sel=2;
    }
    else
    {
        if(USE_INDEX_ARM_AXES==IndexArm_3_Axis)                                 //JimmyChiu 20220708 : add Index Arm Axis
        {
            return;
        }
        Sel=3;
    }

    TIData=Gali_Command("TI", __FUNC__);                                        //回應電腦input 狀態
    Led[iAlarmLed]=TIData&MyLongMask[Gali_TiData[Sel]];                         //Alarm
    if(Led[iAlarmLed] && MovFlag==true)
    {
        Gali_MotorAlarm=true;
        if(Alarm) Alarm->Set(ALM_MOTOR_MOVE);   //AI(W906-HALARM) 20260926: golden 原文 `Alarm->Set(ALM_MOTOR_MOVE);`；if(Alarm) 同 mycylin.cpp SetAlarm（ctest／工具不建 Alarm）
    }
    else
    {
        Gali_MotorAlarm=false;
    }
    Led[iServoalarmLed]=TIData&MyLongMask[Gali_TiData[Sel]];                    //SAlarm
    if(GaliScanServo)
        Led[iServoOn]=!Gali_Command("MG_MO"+GetMot, __FUNC__);
}
//------------------------------------------------------------------------------
void TMyMotor::Gali_ScanMotStatus()
{
    // AI(W906-P0-5) 20260920: 離線守衛 —— 值與退休前 mymotor.cpp 的樁**完全相同**，所以 Motor==NULL 的可觀察行為一格沒變。
    if(Motor==NULL) return;
    BYTE Data;
    AnsiString GetMot;

    if(Led[iAlarmLed] && MovFlag==true)
    {
        Gali_MotorAlarm=true;
        if(Alarm) Alarm->Set(ALM_MOTOR_MOVE);   //AI(W906-HALARM) 20260926: golden 原文 `Alarm->Set(ALM_MOTOR_MOVE);`；if(Alarm) 同 mycylin.cpp SetAlarm（ctest／工具不建 Alarm）
    }
    else
    {
        Gali_MotorAlarm=false;
    }

    if(Motor!=NULL &&                                                           //JimmyChiu 20250306 : 避免還沒初始化就操作馬達
       Motor->Enable && (bGali_CardInstall==true || W906_GaliRouteOwns(Mot_Name)))   //AI(W906-INDEXZ-1203) 20260929: golden card branch also for the routed axis (HT9050 M14 on the 1203, Motor/GaliRoute.h); no route installed = golden
    {
        GetMot=Gali_GetMOT(Mot_Name);
        Data=Gali_Command("TS"+GetMot, __FUNC__);

        //---------------------------------------
        Led[iCwLed]         =!(Data&bMask[3]);      //CW
        Led[iHomeLed]       =!(Data&bMask[1]);      //HOME
        Led[iCcwLed]        =!(Data&bMask[2]);      //CCW
        Led[iEmgLed]        =false;                 //EMG.S
        Led[iSoftcwLed]     =false;                 //Soft Cw
        Led[iSoftccwLed]    =false;                 //SoftCCW
        Led[iInposLed]      =Data&bMask[7];
        //---------------------------------------

        iCheckStatusCT++;
        if(iCheckStatusCT<Mot_Name)
          return;
        iCheckStatusCT=0;

        Gali_ScanMotStatusTIMO();
        //---------------------------------------
    }
}
//------------------------------------------------------------------------------
void TMyMotor::Gali_ScanAlarmStatus()
{
    // AI(W906-P0-5) 20260920: 離線守衛 —— 值與退休前 mymotor.cpp 的樁**完全相同**，所以 Motor==NULL 的可觀察行為一格沒變。
    if(Motor==NULL) return;
    BYTE TIData;
    int Sel=0;
    char Gali_TiData[]={1, 3, 5, 7};
    AnsiString GetMot;

    if(Mot_Name==MTestY1)
        Sel=0;
    else if(Mot_Name==MTestZ1)
        Sel=1;
    else if(Mot_Name==MTestZ2)
        Sel=2;
    else
        Sel=3;

    if(Led[iAlarmLed] && MovFlag==true)
    {
        Gali_MotorAlarm=true;
        if(Alarm) Alarm->Set(ALM_MOTOR_MOVE);   //AI(W906-HALARM) 20260926: golden 原文 `Alarm->Set(ALM_MOTOR_MOVE);`；if(Alarm) 同 mycylin.cpp SetAlarm（ctest／工具不建 Alarm）
    }
    else
    {
        Gali_MotorAlarm=false;
    }

    if(Motor!=NULL &&                                                           //JimmyChiu 20250306 : 避免還沒初始化就操作馬達
       Motor->Enable && (bGali_CardInstall==true || W906_GaliRouteOwns(Mot_Name)))   //AI(W906-INDEXZ-1203) 20260929: golden card branch also for the routed axis (HT9050 M14 on the 1203, Motor/GaliRoute.h); no route installed = golden
    {
        GetMot=Gali_GetMOT(Mot_Name);

        TIData=Gali_Command("TI", __FUNC__);

        Led[iAlarmLed]=TIData&MyLongMask[Gali_TiData[Sel]];                     //Alarm
        if(Led[iAlarmLed] && MovFlag==true)
        {
            Gali_MotorAlarm=true;
            if(Alarm) Alarm->Set(ALM_MOTOR_MOVE);   //AI(W906-HALARM) 20260926: golden 原文 `Alarm->Set(ALM_MOTOR_MOVE);`；if(Alarm) 同 mycylin.cpp SetAlarm（ctest／工具不建 Alarm）
        }
        else
        {
            Gali_MotorAlarm=false;
        }
        Led[iServoalarmLed]=TIData&MyLongMask[Gali_TiData[Sel]];                //SAlarm
        //---------------------------------------
    }
}
//------------------------------------------------------------------------------
AnsiString TMyMotor::Gali_GetMOT(int MOT)
{
    // AI(W906-P0-5) 20260920: 離線守衛 —— 值與退休前 mymotor.cpp 的樁**完全相同**，所以 Motor==NULL 的可觀察行為一格沒變。
    if(Motor==NULL) return "";
    if(MOT==MTestY1)
        return "X";
    else if(MOT==MTestZ1)
        return "Y";
    else if(MOT==MTestZ2)
        return "Z";
    else
        return "W";
}
//------------------------------------------------------------------------------
bool TMyMotor::Gali_MotMove2(int Pos, int Speed, int SpeedDec)                  //Steven 20170818 : 測試UPH用
{
    //******************************************************************************
    //  注意!! CheckIsSafeDoorOpen為Handler 安全門相關, 修改時要小心!!
    //******************************************************************************
    if(Motor==NULL ||                                                           //JimmyChiu 20250306 : 避免還沒初始化就操作馬達
       Motor->CheckIsSafeDoorOpen())                                            //Jimmychiu 20221013 safedoor判斷整合Function
    {
        return false;
    }

    if(W906_GaliRouteAbsentAxis(Mot_Name)) return true;   /* AI(W906-INDEX-ABSENT) 20261004: an absent Index axis is done at once, before the IndexZCanMove lock (as Gali_MotMove) */  if(Pos==0)
    {
        Pos=0;
    }

    bool flag, checkflag=true;
    char str[256]="";
    AnsiString SelMot;

    SelMot=Gali_GetMOT(Mot_Name);
    int iOrgPos=Pos;
    if(Mot_Name!=MTestY1)
        Pos=-Pos;
    int iReatePos=Pos;
    GetRealPos(&iReatePos);
    double sp=Speed;
    Speed=(int)sp;
    if(Mot_Name==MTestZ1)
    {
        if(IndexZCanMove[0]==false)
            return false;
        IndexZCanMove[1]=false;
    }
    else if(Mot_Name==MTestZ2)
    {
        if(IndexZCanMove[1]==false)
            return false;
        IndexZCanMove[0]=false;
    }

    if(CosFunction.bIndexProtect==true)                                         //Steven 20180319 (jou) : 加入Index Y軸移動前確認Z軸位置保護
    {
        if(CheckYPosWhenZDown(Pos, iOrgPos, AnsiString(__FUNC__))==false)       //jimmychiu 20220815 for index arm checking y pos before z down
            return false;
    }

    if(Motor->Enable && (bGali_CardInstall==true || W906_GaliRouteOwns(Mot_Name)))   //AI(W906-INDEXZ-1203) 20260929: golden card branch also for the routed axis (HT9050 M14 on the 1203, Motor/GaliRoute.h); no route installed = golden
    {
        Position=Gali_ReadPos();
        if(MovFlag==false)
        {
            #ifdef DEBUG_GALIL_CHECK_Y
            if(Mot_Name==MTestY1 || Mot_Name==MTestY2)                          //JerryYang 20241129 : 計算Index Y移動後點位是否兩支ARM同時在SOCKET
            {
                if(CheckY1Y2TargetPos(iOrgPos, AnsiString(__FUNC__))==false)
                    return false;
            }
            #endif

            if(Mot_Name==MTestZ1)
                checkflag=CheckTestZ1("Gali_MotMove2");
            else if(Mot_Name==MTestZ2)
                checkflag=CheckTestZ2("Gali_MotMove2");
            else
                checkflag=CheckTestZ("Gali_MotMove2");

            if(checkflag)
            {
                iGali_SpSpeed=Speed;
                W4G_sprintf(str, "SP%s=%d;AC%s=%d;DC%s=%d;PA%s=%d;BG%s;",
                             SelMot, Speed,
                             SelMot, SpeedDec,
                             SelMot, SpeedDec,
                             SelMot, iReatePos,
                             SelMot);

                if(ScanIndexMotorCanMove())
                    Gali_Command(str, __FUNC__);
                else
                    return false;
                MovFlag=true;
                bScanFlag=false;
                Led[iInposLed]=true;
                GaliSofDelayCount=0;
            }
            else
            {
                return false;                                                   //kevin 20141028 add 2014.10.08 , Joye , KYEC Z1 Position
            }
        }
        else
        {
            if(Gali_Command("MG_BG"+SelMot, __FUNC__)==1)                       //馬達目前是否移動 0:移動完成  //ChungHung 20141016 add 確保每次皆到位
                return false;

            if(bScanFlag==false)
                Gali_ScanMotStatus();
            if(Led[iInposLed]==false)
            {
                if(TargetPosition!=Pos)                                         //kevin 20141028 2014.10.08 , Joye , KYEC Z1 Position
                {
                    MovFlag = false;
                    GaliSofDelayCount = 0;
                    return false;
                }
                bScanFlag=true;
                GaliSofDelayCount++;
                if(GaliSofDelayCount>=DelayCount)
                {
                    if(Mot_Name==MTestY1 || Mot_Name==MTestY2 || Pos==0)
                        flag=Gali_ReadEncoderInRandge(Pos);
                    else
                        flag=Gali_ReadEncoderMaxRandge(Pos);

                    if(flag==false)
                    {
                        iHome=1;
                        if(GaliAxisAlarm[0])
                            JamCode = MotorIndexToJamCode(13);
                        else if(GaliAxisAlarm[1])
                            JamCode = MotorIndexToJamCode(14);
                        else if(GaliAxisAlarm[2])
                            JamCode = MotorIndexToJamCode(15);
                        else  //if(GaliAxisAlarm[3])
                            JamCode = MotorIndexToJamCode(16);
                        ShowMotorErrorMessage(JamCode, MotErrPos+1);            //Sam 20200903 : 補 Alarm
                        bScanFlag=false;
                        GaliSofDelayCount=0;
                        MovFlag=false;
                        return false;
                    }

                    bScanFlag=false;
                    GaliSofDelayCount=0;
                    MovFlag=false;
                    if(Mot_Name==MTestZ1)
                        IndexZCanMove[1]=true;
                    else if(Mot_Name==MTestZ2)
                        IndexZCanMove[0]=true;
                    return true;
                }
            }
        }
    }
    else
    {
        if(Mot_Name==MTestY2)
            Pos=-Pos;
        if(Position==Pos)
        {
            if(Mot_Name==MTestZ1)
                IndexZCanMove[1]=true;
            else if(Mot_Name==MTestZ2)
                IndexZCanMove[0]=true;

            return true;
        }
        else
        {
            if(Position>Pos)
            {
                Position-=speed*W906_SimGalilStepScale();  //AI(W906-SIMSTEP) 20260924: 模擬步進 ×W906_SimGalilStepScale()（B13 500 ms tick；出貨組態＝1，見本檔檔尾）
                if(Position<=Pos)
                {
                    Position=Pos;
                    if(Mot_Name==MTestZ1)
                        IndexZCanMove[1]=true;
                    else if(Mot_Name==MTestZ2)
                        IndexZCanMove[0]=true;
                    return true;
                }
            }
            else
            {
                Position+=speed*W906_SimGalilStepScale();  //AI(W906-SIMSTEP) 20260924: 模擬步進 ×W906_SimGalilStepScale()（B13 500 ms tick；出貨組態＝1，見本檔檔尾）
                if(Position>=Pos)
                {
                    Position=Pos;
                    if(Mot_Name==MTestZ1)
                        IndexZCanMove[1]=true;
                    else if(Mot_Name==MTestZ2)
                        IndexZCanMove[0]=true;
                    return true;
                }
            }
        }
    }
    TargetPosition=Pos;
    return false;
}
//------------------------------------------------------------------------------
bool TMyMotor::Gali_MotMove(int Pos, int Speed, AnsiString _Func)
{
    // AI(W906-P0-5) 20260920: 離線守衛 —— 值與退休前 mymotor.cpp 的樁**完全相同**，所以 Motor==NULL 的可觀察行為一格沒變。
    if(Motor==NULL) return false;
    //******************************************************************************
    //  注意!! CheckIsSafeDoorOpen為Handler 安全門相關, 修改時要小心!!
    //******************************************************************************
    if(Motor->CheckIsSafeDoorOpen())                                            //Jimmychiu 20221013 safedoor判斷整合Function
    {
        return false;
    }

    if((USE_INDEX_ARM_AXES==IndexArm_3_Axis && Mot_Name==MTestY2) || W906_GaliRouteAbsentAxis(Mot_Name))   //JimmyChiu 20220708 : add Index Arm Axis  AI(W906-INDEX-ABSENT) 20261004: + an Index axis the Galil route left absent (HT9050: Y1 / Z2 / Y2) -- golden's own 3-axis "absent Y2 = done at once", before the IndexZCanMove lock (dispatch 7 A5-6: a Z2 move froze Z1)
    {
        return true;
    }

    if(Pos==0)
    {
        Pos=0;
    }

    bool flag=false,checkflag=true;
    char str[256]="";
    AnsiString SelMot=AnsiString("");
    AnsiString sFunc=AnsiString(__FUNC__)+AnsiString("_")+_Func;

    SelMot=Gali_GetMOT(Mot_Name);
    int iOrgPos=Pos;
    if(Mot_Name!=MTestY1)
        Pos=-Pos;
    int iReatePos=Pos;
    GetRealPos(&iReatePos);
    double sp=Speed;
    Speed=(int)sp;
    if(Mot_Name==MTestZ1)
    {
        if(IndexZCanMove[0]==false)
            return false;
        IndexZCanMove[1]=false;
    }
    else if(Mot_Name==MTestZ2)
    {
        if(IndexZCanMove[1]==false)
            return false;
        IndexZCanMove[0]=false;
    }

    if(CosFunction.bIndexProtect==true)                                         //Steven 20180319 (jou) : 加入Index Y軸移動前確認Z軸位置保護
    {
        if(CheckYPosWhenZDown(Pos, iOrgPos, sFunc)==false)                      //jimmychiu 20220815 for index arm checking y pos before z down
            return false;
    }

    if(Motor->Enable && (bGali_CardInstall==true || W906_GaliRouteOwns(Mot_Name)))   //AI(W906-INDEXZ-1203) 20260929: golden card branch also for the routed axis (HT9050 M14 on the 1203, Motor/GaliRoute.h); no route installed = golden
    {
        Position=Gali_ReadPos();
        if(MovFlag==false)
        {
            #ifdef DEBUG_GALIL_CHECK_Y
            if(Mot_Name==MTestY1 || Mot_Name==MTestY2)                          //JerryYang 20241129 : 計算Index Y移動後點位是否兩支ARM同時在SOCKET
            {
                if(CheckY1Y2TargetPos(iOrgPos, sFunc)==false)
                    return false;
            }
            #endif

            if(Mot_Name==MTestZ1)
                checkflag=CheckTestZ1(sFunc);
            else if(Mot_Name==MTestZ2)
                checkflag=CheckTestZ2(sFunc);
            else
                checkflag=CheckTestZ(sFunc);

            if(checkflag)
            {
                iGali_SpSpeed=Speed;
                W4G_sprintf(str, "SP%s=%d;AC%s=%d;DC%s=%d;PA%s=%d;BG%s;",
                              SelMot, Speed,
                              SelMot, GailAcSpeed2,
                              SelMot, GailDcSpeed2,
                              SelMot, iReatePos,
                              SelMot);

                if(ScanIndexMotorCanMove())
                    Gali_Command(str, sFunc);
                else
                    return false;
                MovFlag=true;
                bScanFlag=false;
                Led[iInposLed]=true;
                GaliSofDelayCount=0;
            }
            else
            {
                return false;                                                   //kevin 20141028 add 2014.10.08 , Joye , KYEC Z1 Position
            }
        }
        else
        {
            if(Gali_Command("MG_BG"+SelMot, sFunc)==1)                          //馬達目前是否移動 0:移動完成  //ChungHung 20141016 add 確保每次皆到位
                return false;

            if(bScanFlag==false)
                Gali_ScanMotStatus();
            if(Led[iInposLed]==false)
            {
                if(TargetPosition!=Pos)                                         //kevin 20141028 2014.10.08 , Joye , KYEC Z1 Position
                {
                    MovFlag = false;
                    GaliSofDelayCount = 0;
                    return false;
                }
                bScanFlag=true;
                GaliSofDelayCount++;
                if(GaliSofDelayCount>=DelayCount)
                {
                    if(Mot_Name==MTestY1 || Mot_Name==MTestY2 || Pos==0)
                        flag=Gali_ReadEncoderInRandge(Pos);
                    else
                        flag=Gali_ReadEncoderMaxRandge(Pos);

                    if(flag==false)
                    {
                        iHome=1;
                        if(GaliAxisAlarm[0])
                            JamCode = MotorIndexToJamCode(13);
                        else if(GaliAxisAlarm[1])
                            JamCode = MotorIndexToJamCode(14);
                        else if(GaliAxisAlarm[2])
                            JamCode = MotorIndexToJamCode(15);
                        else  //if(GaliAxisAlarm[3])
                            JamCode = MotorIndexToJamCode(16);
                        ShowMotorErrorMessage(JamCode, MotErrPos+1, sFunc);     //Sam 20200903 : 補 Alarm
                        bScanFlag=false;
                        GaliSofDelayCount=0;
                        MovFlag=false;
                        return false;
                    }

                    bScanFlag=false;
                    GaliSofDelayCount=0;
                    MovFlag=false;
                    if(Mot_Name==MTestZ1)
                        IndexZCanMove[1]=true;
                    else if(Mot_Name==MTestZ2)
                        IndexZCanMove[0]=true;
                    return true;
                }
            }
        }
    }
    else
    {
        if(Mot_Name==MTestY2)
            Pos=-Pos;
        if(Position==Pos)
        {
            if(Mot_Name==MTestZ1)
                IndexZCanMove[1]=true;
            else if(Mot_Name==MTestZ2)
                IndexZCanMove[0]=true;

            return true;
        }
        else
        {
            if(Position>Pos)
            {
                Position-=speed*W906_SimGalilStepScale();  //AI(W906-SIMSTEP) 20260924: 模擬步進 ×W906_SimGalilStepScale()（B13 500 ms tick；出貨組態＝1，見本檔檔尾）
                if(Position<=Pos)
                {
                    Position=Pos;
                    if(Mot_Name==MTestZ1)
                        IndexZCanMove[1]=true;
                    else if(Mot_Name==MTestZ2)
                        IndexZCanMove[0]=true;
                    return true;
                }
            }
            else
            {
                Position+=speed*W906_SimGalilStepScale();  //AI(W906-SIMSTEP) 20260924: 模擬步進 ×W906_SimGalilStepScale()（B13 500 ms tick；出貨組態＝1，見本檔檔尾）
                if(Position>=Pos)
                {
                    Position=Pos;
                    if(Mot_Name==MTestZ1)
                        IndexZCanMove[1]=true;
                    else if(Mot_Name==MTestZ2)
                        IndexZCanMove[0]=true;
                    return true;
                }
            }
        }
    }
    TargetPosition=Pos;
    return false;
}
//------------------------------------------------------------------------------
bool TMyMotor::Gali_MotMoveNoWait(int Pos, int Speed, int iNeedDelayTime, bool bCheckZ)
{
    // AI(W906-P0-5) 20260920: 離線守衛 —— 值與退休前 mymotor.cpp 的樁**完全相同**，所以 Motor==NULL 的可觀察行為一格沒變。
    if(Motor==NULL) return false;
    //******************************************************************************
    //  注意!! CheckIsSafeDoorOpen為Handler 安全門相關, 修改時要小心!!
    //******************************************************************************
    if(Motor->CheckIsSafeDoorOpen())                                            //Jimmychiu 20221013 safedoor判斷整合Function
    {
        return false;
    }

    if((USE_INDEX_ARM_AXES==IndexArm_3_Axis && Mot_Name==MTestY2) || W906_GaliRouteAbsentAxis(Mot_Name))   //JimmyChiu 20220708 : add Index Arm Axis  AI(W906-INDEX-ABSENT) 20261004: + an Index axis the Galil route left absent -- see Gali_MotMove
    {
        return true;
    }

    if(Pos==0)
    {
        Pos=0;
    }
    bool checkflag=true;
    char str[256]="";
    AnsiString SelMot=AnsiString("");

    SelMot=Gali_GetMOT(Mot_Name);
    int iOrgPos=Pos;
    if(Mot_Name!=MTestY1)
        Pos=-Pos;
    double sp=Speed;
    int iReatePos=Pos;
    GetRealPos(&iReatePos);
    Speed=(int)sp;
    if(Mot_Name==MTestZ1)
    {
        if(IndexZCanMove[0]==false)
            return false;
        IndexZCanMove[1]=false;
    }
    else if(Mot_Name==MTestZ2)
    {
        if(IndexZCanMove[1]==false)
            return false;
        IndexZCanMove[0]=false;
    }

    if(CheckYPosWhenZDown(Pos, iOrgPos, AnsiString(__FUNC__))==false)           //jimmychiu 20220815 for index arm checking y pos before z down
        return false;

    if(Motor->Enable && (bGali_CardInstall==true || W906_GaliRouteOwns(Mot_Name)))   //AI(W906-INDEXZ-1203) 20260929: golden card branch also for the routed axis (HT9050 M14 on the 1203, Motor/GaliRoute.h); no route installed = golden
    {
        Position=Gali_ReadPos();
        if(MovFlag==false)
        {
            #ifdef DEBUG_GALIL_CHECK_Y
            if(Mot_Name==MTestY1 || Mot_Name==MTestY2)                          //JerryYang 20241129 : 計算Index Y移動後點位是否兩支ARM同時在SOCKET
            {
                if(CheckY1Y2TargetPos(iOrgPos, AnsiString(__FUNC__))==false)
                    return false;
            }
            #endif

            if(Mot_Name==MTestZ1)
                checkflag=CheckTestZ1("Gali_MotMoveNoWait");
            else if(Mot_Name==MTestZ2)
                checkflag=CheckTestZ2("Gali_MotMoveNoWait");
            else
                checkflag=CheckTestZ("Gali_MotMoveNoWait");

            if(checkflag)
            {
                iGali_SpSpeed=Speed;
               if(ScanIndexMotorCanMove())
                {
                    W4G_sprintf(str, "SP%s=%d;AC%s=%d;DC%s=%d;PA%s=%d;BG%s;",
                                  SelMot, Speed,
                                  SelMot, GailAcSpeed2,
                                  SelMot, GailDcSpeed2,
                                  SelMot, iReatePos,
                                  SelMot);
                    Gali_Command(str, __FUNC__);
                }
                else
                {
                    return false;
                }

                bScanFlag=false;
                MovFlag=true;
                Led[iInposLed]=true;
                GaliSofDelayCount=0;
            }
        }
        else
        {
            if(Gali_Command("MG_BG"+SelMot, __FUNC__)==1)                       //馬達目前是否移動 0:移動完成  //ChungHung 20141016 add 確保每次皆到位
                return false;

            if(bScanFlag==false)
                Gali_ScanMotStatus();
            if(Led[iInposLed]==false)
            {
                bScanFlag=true;
                GaliSofDelayCount++;
                if(GaliSofDelayCount>=iNeedDelayTime)
                {
                    bScanFlag=false;
                    GaliSofDelayCount=0;
                    MovFlag=false;
                    if(Mot_Name==MTestZ1)
                        IndexZCanMove[1]=true;
                    else if(Mot_Name==MTestZ2)
                        IndexZCanMove[0]=true;
                    MySleep(100);
                    ScanIndexOtherMotorCanMove(Mot_Name);
                    return true;
                }
            }
        }
    }
    else
    {
        if(Mot_Name==MTestY2)
            Pos=-Pos;
        if(Position==Pos)
        {
            if(Mot_Name==MTestZ1)
                IndexZCanMove[1]=true;
            else if(Mot_Name==MTestZ2)
                IndexZCanMove[0]=true;

            return true;
        }
        else
        {
            if(Position>Pos)
            {
                Position-=speed*W906_SimGalilStepScale();  //AI(W906-SIMSTEP) 20260924: 模擬步進 ×W906_SimGalilStepScale()（B13 500 ms tick；出貨組態＝1，見本檔檔尾）
                if(Position<=Pos)
                {
                    Position=Pos;
                    if(Mot_Name==MTestZ1)
                        IndexZCanMove[1]=true;
                    else if(Mot_Name==MTestZ2)
                        IndexZCanMove[0]=true;

                    return true;
                }
            }
            else
            {
                Position+=speed*W906_SimGalilStepScale();  //AI(W906-SIMSTEP) 20260924: 模擬步進 ×W906_SimGalilStepScale()（B13 500 ms tick；出貨組態＝1，見本檔檔尾）
                if(Position>=Pos)
                {
                    Position=Pos;
                    if(Mot_Name==MTestZ1)
                        IndexZCanMove[1]=true;
                    else if(Mot_Name==MTestZ2)
                        IndexZCanMove[0]=true;

                    return true;
                }
            }
        }
    }
    TargetPosition=Pos;
    return false;
}
//------------------------------------------------------------------------------
bool TMyMotor::Gali_MotMoveSkipEncoder(int Pos, int Speed)
{
    // AI(W906-P0-5) 20260920: 離線守衛 —— 值與退休前 mymotor.cpp 的樁**完全相同**，所以 Motor==NULL 的可觀察行為一格沒變。
    if(Motor==NULL) return false;
    //******************************************************************************
    //  注意!! CheckIsSafeDoorOpen為Handler 安全門相關, 修改時要小心!!
    //******************************************************************************
    if(Motor->CheckIsSafeDoorOpen())                                            //Jimmychiu 20221013 safedoor判斷整合Function
    {
        return false;
    }

    if(W906_GaliRouteAbsentAxis(Mot_Name)) return true;   /* AI(W906-INDEX-ABSENT) 20261004: an absent Index axis is done at once (as Gali_MotMove) */  char str[256]="";
    AnsiString SelMot=AnsiString("");
    SelMot=Gali_GetMOT(Mot_Name);
    if(Mot_Name!=MTestY1)
        Pos=-Pos;
    double sp=Speed;
    int iReatePos=Pos;
    GetRealPos(&iReatePos);
    sp*=0.6;
    Speed=(int)sp;
    if(Motor->Enable && (bGali_CardInstall==true || W906_GaliRouteOwns(Mot_Name)))   //AI(W906-INDEXZ-1203) 20260929: golden card branch also for the routed axis (HT9050 M14 on the 1203, Motor/GaliRoute.h); no route installed = golden
    {
        if(MovFlag==false)
        {
            iGali_SpSpeed=Speed;
            W4G_sprintf(str, "SP%s=%d;AC%s=%d;DC%s=%d;PA%s=%d;BG%s;",
                          SelMot, Speed,
                          SelMot, GailAcSpeed2,
                          SelMot, GailDcSpeed2,
                          SelMot, iReatePos,
                          SelMot);
            if(ScanIndexMotorCanMove())
            {
                Gali_Command(str, __FUNC__);
            }
            else
            {
                return false;
            }

            MovFlag=true;
        }
        else
        {
            if(Gali_Command("MG_BG"+SelMot, __FUNC__)==1)                       //馬達目前是否移動 0:移動完成  //ChungHung 20141016 add 確保每次皆到位
                return false;

            ScanIndexOtherMotorCanMove(Mot_Name);
            if(bScanFlag==false)
                Gali_ScanMotStatus();
            if(Led[iInposLed]==false)
            {
                bScanFlag=true;
                GaliSofDelayCount++;
                if(GaliSofDelayCount>=DelayCount)
                {
                    bScanFlag=false;
                    GaliSofDelayCount=0;
                    MovFlag=false;
                    return true;
                }
            }
        }
    }
    else
    {
        if(Position==Pos)
        {
            return true;
        }
        else
        {
            if(Position>Pos)
            {
                Position-=speed*W906_SimGalilStepScale();  //AI(W906-SIMSTEP) 20260924: 模擬步進 ×W906_SimGalilStepScale()（B13 500 ms tick；出貨組態＝1，見本檔檔尾）
                if(Position<=Pos)
                {
                    Position=Pos;
                    return true;
                }
            }
            else
            {
                Position+=speed*W906_SimGalilStepScale();  //AI(W906-SIMSTEP) 20260924: 模擬步進 ×W906_SimGalilStepScale()（B13 500 ms tick；出貨組態＝1，見本檔檔尾）
                if(Position>=Pos)
                {
                    Position=Pos;
                    return true;
                }
            }
        }
    }
    TargetPosition=Pos;
    return false;
}
//------------------------------------------------------------------------------
bool TMyMotor::Z1UpZ2Down1(int Speed)
{
    // AI(W906-P0-5) 20260920: 離線守衛 —— 值與退休前 mymotor.cpp 的樁**完全相同**，所以 Motor==NULL 的可觀察行為一格沒變。
    if(Motor==NULL) return false;
    //******************************************************************************
    //  注意!! CheckIsSafeDoorOpen為Handler 安全門相關, 修改時要小心!!
    //******************************************************************************
    if(Motor->CheckIsSafeDoorOpen())                                            //Jimmychiu 20221013 safedoor判斷整合Function
    {
        return false;
    }

    char str[512]="";
    char tmpstr[512]="";
    AnsiString str2="";
    static int iCount=0;                                                        //kevin 20220221 add Gali  Command state  Mylin 20211228 Add Gali Command Log
    static bool MOTMovflag[]={true,true,true,true};
    int iEncoderZ1=0, iEncoderY1=0;
    int iEncoderZ2=0, iEncoderY2=0;

    GetIndexParm();
    if(Motor->Enable && bGali_CardInstall==true)
    {
        if(bShowMotorMove==true)
        {
            MOT[MTestY1].Position=MOT[MTestY1].Gali_ReadPos();
            if(USE_INDEX_ARM_AXES==IndexArm_4_Axis)
            {
                MOT[MTestY2].Position=MOT[MTestY2].Gali_ReadPos();
            }
        }

        if(MovFlag==false && bZ1Z2Exute==false)                                 //kevin 20110630
        {
            if(CheckTestZ1("Z1UpZ2Down1"))
            {
                iGali_VsSpeed=Speed;
                TestYBuffer=XShiftF;
                TestY2Buffer=-XShiftR;
                MOT[MTestZ1].GetRealPos(&Z1Safe);

                #ifdef DEBUG_GALIL_CHECK_Y
                if(Check_Y1Y2_TargetPosWillCrash(TestYBuffer, -TestY2Buffer))   //JerryYang 20250822 : fix index y保護  //JerryYang 20241129 : 計算Index Y移動後點位是否兩支ARM同時在SOCKET
                {
                    str2.sprintf("Index Y Position Calculate error, %s",__FUNC__);
                    ShowMyMessage(str2);
                    fAllMotorHome=false;
                    return false;
                }
                #endif

                MOT[MTestY1].GetRealPos(&TestYBuffer);
                MOT[MTestZ1].GetRealPos(&Z1Up);
                MOT[MTestZ2].GetRealPos(&Z2DownSafe);

                if(USE_INDEX_ARM_AXES==IndexArm_4_Axis)                         //JimmyChiu 20220708 : add Index Arm Axis
                {
                    MOT[MTestY2].GetRealPos(&TestY2Buffer);
                    MOT[MTestY2].GetRealPos(&AxisY_Pre_MovePos);                //2013-01-15    Dell   提高Index 運動速度(目前by9046LS專用)
                }
                else//IndexArm_3_Axis
                {
                    //
                }

                MOT[MTestZ2].GetRealPos(&Z2Down);
                #ifdef DEBUG_GALIL
                MOT[MTestZ2].GetRealPos(&Z2Down2Speed);
                #endif

                if(IniConfig.bD38IndexPutICToShtNoWaitMotion &&                 //Steven 20181228 : Add Index Action
                   bZ1UpAndPlaceZ2Down==true                 &&
                   IniConfig.bIndexArm2SupplyLight==false    &&
                   TestIF_File.bForEgisTecTest==false        &&
                   TestIF_File.bArm1PickPlaceArm2Test==false)
                {
                    W4G_sprintf(str,"LMXYZW;"
                                "LI0,%d,0,0;"
                                "LI%d,%d,%d,%d;"
                                "LI0,%d,%d,0;"
                                "LE;"
                                "VS%d;VA%d;VD%d;BGS;",
                                -Z1Safe,
                                TestYBuffer, -Z1Up, -Z2DownSafe, TestY2Buffer,
                                -Z1DownToShuttle, -Z2Down,
                                Speed, GailAcSpeed, GailDcSpeed);
                }
                else                                                            ////20111114  Dell for Disable Index Arm    Start
                {
                    #ifdef DEBUG_GALIL
                    W4G_sprintf(str, "LMXYZW;"
                                 "LI0,%d,0,0;"
                                 "LI%d,%d,%d,%d;"
                                 "LI0,0,%d,0 >5000;"
                                 "LI0,0,%d,0 <5000;"
                                 "LE;"
                                 "VS%d;VA%d;VD%d;BGS;",
                                 -Z1Safe,
                                 TestYBuffer, -Z1Up, -Z2DownSafe, TestY2Buffer,
                                 -Z2Down,
                                 Z2Down2Speed,
                                 Speed, GailAcSpeed, GailDcSpeed);
                    #else
                    if(TestIF_File.iShuttleMode==1 && IniConfig.bShuttleMode50==false &&
                       TestIF_File.iShuttle_Sel==1 && Z1Safe==0)                //jou 2014-10-23 增加index arm轉換保護
                    {
                        if(USE_INDEX_ARM_AXES==IndexArm_3_Axis)                 //JimmyChiu 20220708 : add Index Arm Axis
                        {
                            W4G_sprintf(tmpstr, "LMXYZ;"
                                            "LI%d,%d,%d;",
                                            TestYBuffer, -Z1Up, -Z2DownSafe);
                        }
                        else
                        {
                            W4G_sprintf(tmpstr, "LMXYZW;"
                                            "LI%d,%d,%d,%d;",
                                            TestYBuffer, -Z1Up, -Z2DownSafe, TestY2Buffer);
                        }
                    }
                    else
                    {
                        if(USE_INDEX_ARM_AXES==IndexArm_3_Axis)                 //JimmyChiu 20220708 : add Index Arm Axis
                        {
                            W4G_sprintf(tmpstr, "LMXYZ;"
                                            "LI0,%d,0;"
                                            "LI%d,%d,%d;",
                                            -Z1Safe,
                                            TestYBuffer, -Z1Up, -Z2DownSafe);
                        }
                        else if(MachineTypeChoice==Type_HT9046_LS &&            //KenHsieh 20230508 : 修正關Arm之Arm偷跑問題
                                TestIF_File.iShuttleMode==1 && IniConfig.bShuttleMode50==false)
                        {
                            W4G_sprintf(tmpstr, "LMXYZW;"
                                            "LI0,%d,0,0;"
                                            "LI%d,%d,%d,%d;",
                                            -Z1Safe,
                                            TestYBuffer, -Z1Up, -Z2DownSafe, TestY2Buffer);
                        }
                        else if(REAL_TIME_CCD && COM2->bCCDDummyRum==false && TestIF_File.bUseRTCStepAsideMode) //Ifor 20230908 add:RTC 讓位功能 By Setup File
                        {
                            W4G_sprintf(tmpstr, "LMXYZW;"
                                            "LI0,%d,0,0;"
                                            "LI%d,%d,%d,%d;"
                                            "LI%d,%d,%d,%d;",
                                            -Z1Safe,
                                            TestYBuffer, -Z1Up, 0, 0,
                                            0, 0, -Z2DownSafe, TestY2Buffer);
                        }
                        else if(MachineTypeChoice==Type_HT9046_LS &&            //jou 2014-10-23 增加index arm轉換保護
                                bGail_HighSpeedMode==true)
                        {
                            W4G_sprintf(tmpstr, "LMXYZW;"
                                            "LI0,%d,0,%d;"
                                            "LI%d,%d,%d,%d<%d>%d;",
                                            -Z1Safe, AxisY_Pre_MovePos,
                                            TestYBuffer, -Z1Up, -Z2DownSafe, (TestY2Buffer-AxisY_Pre_MovePos), MOT[MTestY1].GailSpeed, Speed);
                        }
                        else
                        {
                            W4G_sprintf(tmpstr, "LMXYZW;"
                                            "LI0,%d,0,0;"
                                            "LI%d,%d,%d,%d;",
                                            -Z1Safe,
                                            TestYBuffer, -Z1Up, -Z2DownSafe, TestY2Buffer);
                        }
                    }

                    if(TestIF_File.iShuttleMode==1 && IniConfig.bShuttleMode50==false &&
                       Z2Down==0)                                               //jou 2014-10-23 增加index arm轉換保護
                    {
                         W4G_sprintf(str, "%sLE;VS%d;VA%d;VD%d;BGS;", tmpstr, Speed, GailAcSpeed, GailDcSpeed);
                    }
                    else
                    {
                         W4G_sprintf(str, "%sLI0,0,%d,0;LE;VS%d;VA%d;VD%d;BGS;", tmpstr, -Z2Down, Speed, GailAcSpeed, GailDcSpeed);
                    }
                    #endif
                }

                bScanFlag=false;
                if(DoIndexVectorMove(str))
                {
                    bZ1Z2Exute=true;                                            //kevin 20110630
                }
                else
                {
                    return false;
                }

                for(int i=0; i<Gali_MaxAxis; i++)
                {
                    MOTMovflag[i]=true;
                    MOT[MTestY1+i].Led[iInposLed]=true;
                }
                MovFlag=true;
            }
        }
        else
        {
            if(USE_INDEX_ARM_AXES==IndexArm_4_Axis)
            {
                if(GALI_PROTECT_MODE==0)                                        //Steven 20260504 : Mode 0 = Original JerryYang check
                {
                    iEncoderZ2=MOT[MTestZ2].Gali_ReadEncoderPos();              //JerryYang 20251124 : add portect, 避免index arm撞機
                    iEncoderY2=MOT[MTestY2].Gali_ReadEncoderPos();
                    iEncoderZ1=MOT[MTestZ1].Gali_ReadEncoderPos();
                    iEncoderY1=MOT[MTestY1].Gali_ReadEncoderPos();

                    if(iEncoderZ2<(Prod.All_TestZ_Test_Safe-iCheckZ))             //JerryYang 20260114 : 輕壓的速度比較快, 放寬range   //JerryYang 20251224 : 3000->4000
                    {
                        if(CheckArmPosArrival(iEncoderY2, Prod.TestY2_Middle, IniConfig.GaliPosRange)==false)
                        {
                            ShowIndexMotorError(AnsiString("Z1UpZ2Down1"));
                        }
                    }

                    if(iEncoderZ2<-4000 && iEncoderY2>-9000)                    //JerryYang 20260114 : 檢查下降過程是否會尻到SHUTTLE
                    {
                        ShowIndexMotorError(AnsiString("Z1UpZ2Down1_TEST"));
                    }

                    if(iEncoderZ1<-4000 && iEncoderY1<9000)                     //JerryYang 20260114 : 檢查下降過程是否會尻到SHUTTLE
                    {
                        ShowIndexMotorError(AnsiString("Z1DownZ2Up1_TEST2"));
                    }
                }
                else if(GALI_PROTECT_MODE==2)                                   //Steven 20260504 : Mode 2 = TE (Following Error) check
                {
                    int iFollowErrY1 = MOT[MTestY1].Gali_Command("TEX", __FUNC__);
                    int iFollowErrZ1 = MOT[MTestZ1].Gali_Command("TEY", __FUNC__);
                    int iFollowErrZ2 = MOT[MTestZ2].Gali_Command("TEZ", __FUNC__);
                    int iFollowErrY2 = MOT[MTestY2].Gali_Command("TEW", __FUNC__);

                    int iTeThreshold = iGali_VsSpeed / 50 + 1000;

                    if(abs(iFollowErrY1) > iTeThreshold || abs(iFollowErrZ1) > iTeThreshold ||
                       abs(iFollowErrZ2) > iTeThreshold || abs(iFollowErrY2) > iTeThreshold)
                    {
                        ShowIndexMotorError(AnsiString("Z1UpZ2Down1_FE"));
                    }
                }
                //Mode 1 (ER+OE): No host-side in-motion check needed, Galil firmware handles it
            }

            long lSC=Gali_Command("SC");
            if(lSC==100)                                                        //ChungHung 20150831 add 確保每次皆到位
            {
                iCount++;
                if(iCount>5000)
                {
                    fMain->sb_Gali_LogClick();                                  //Mylin 20211228 Add Gali Command Log
                    iCount=0;
                    iHome =1;
                    if(lSC==8)
                        ShowMyMessage("Index Position Error (OE Stop) in Z1UpZ2Down1, Please Home", "ER+OE Following Error Exceeded");
                    else
                        ShowMyMessage("Gali Command Error When Z1UpZ2Down1, Please Home and Start", "請更換Galil 轉板或100P線材");
                }
                return false;
            }
            iCount=0;                                                           //kevin 20220221 add Gali  Command state Mylin 20211228 Add Gali Command Log

            if(bScanFlag==false)
            {
                for(int i=0; i<Gali_MaxAxis; i++)
                {
                    if(USE_INDEX_ARM_AXES==IndexArm_3_Axis &&                   //JimmyChiu 20220708 : add Index Arm Axis
                       (MTestY1+i)==MTestY2)
                    {
                        MOTMovflag[i]=false;
                    }
                    else
                    {
                        if(MOTMovflag[i])
                        {
                            MOT[MTestY1+i].Gali_ScanMotStatus();
                            MOTMovflag[i]=MOT[MTestY1+i].Led[iInposLed];
                        }
                    }
                }
            }

            if(MOTMovflag[0]==false && MOTMovflag[1]==false && MOTMovflag[2]==false && MOTMovflag[3]==false)
            {
                bScanFlag=true;
                GaliSofDelayCount++;
                if(GaliSofDelayCount>=DelayCount)
                {
                    if(CheckPos(true)==false)                                   //now only check TestY
                    {
                        if(GaliSofDelayCount<=(DelayCount*5))
                            return false;
                        iHome=1;
                        JamCode=MotorIndexToJamCode(13);
                        ShowMotorErrorMessage(JamCode, MotErrPos+1);
                        bScanFlag=false;
                        GaliSofDelayCount=0;
                        MovFlag=false;
                        return false;
                    }

                    if(ISZ1Up_Z2Down())
                    {
                        bScanFlag=false;
                        GaliSofDelayCount=0;
                        MovFlag=false;
                        bZ1Z2Exute=false;                                       //kevin 20110630

                        IndexStatus=Z1Up_Z2Down;
                        fOffSet->bEnterSpecialOffset=false;
                        return true;
                    }
                    else
                    {
                        iHome=1;
                        if(GaliAxisAlarm[1])
                            JamCode=MotorIndexToJamCode(14);
                        else
                            JamCode=MotorIndexToJamCode(15);

                        ShowMotorErrorMessage(JamCode, MotErrPos+1);
                        bScanFlag=false;
                        GaliSofDelayCount=0;
                        MovFlag=false;
                        return false;
                    }
                }
            }
        }
    }
    else
    {
        Speed=ArmSpeed[IndexArm].iBodySP*10*W906_SimGalilStepScale();  //AI(W906-SIMSTEP) 20260924: 模擬步進 ×W906_SimGalilStepScale()（B13 500 ms tick；出貨組態＝1，見本檔檔尾）
        if(Speed<=0)
            Speed=10;
        MOT[MTestZ2].Position+=Speed;
        MOT[MTestZ1].Position-=Speed;
        if(MOT[MTestZ2].Position<=0)
            MOT[MTestZ2].Position=0;
        MOT[MTestY1].Position-=Speed;
        if(USE_INDEX_ARM_AXES==IndexArm_4_Axis)
        {
            MOT[MTestY2].Position-=Speed;
        }

        MOT[MTestY1].TargetPosition=Prod.TestY1_Front;                          //Steven 20210827 : 修改顯示Galil的Target / Current
        if(USE_INDEX_ARM_AXES==IndexArm_4_Axis)
        {
            MOT[MTestY2].TargetPosition=Prod.TestY2_Middle;
        }
        MOT[MTestZ1].TargetPosition=0;
        MOT[MTestZ2].TargetPosition=-Prod.TestZ2_Test;

        if(MOT[MTestZ1].Position<=0 || MOT[MTestY1].Position<=Prod.TestY1_Front)
        {
            IndexStatus=Z1Up_Z2Down;
            MOT[MTestY1].Position=Prod.TestY1_Front;
            if(USE_INDEX_ARM_AXES==IndexArm_4_Axis)
            {
                MOT[MTestY2].Position=Prod.TestY2_Middle;
            }
            MOT[MTestZ1].Position=0;
            MOT[MTestZ2].Position=-Prod.TestZ2_Test;
            fOffSet->bEnterSpecialOffset=false;
            return true;
        }
        return false;
    }
    return false;
}
//------------------------------------------------------------------------------
bool TMyMotor::Z1DownZ2Up1(int Speed)
{
    // AI(W906-P0-5) 20260920: 離線守衛 —— 值與退休前 mymotor.cpp 的樁**完全相同**，所以 Motor==NULL 的可觀察行為一格沒變。
    if(Motor==NULL) return false;
    //******************************************************************************
    //  注意!! CheckIsSafeDoorOpen為Handler 安全門相關, 修改時要小心!!
    //******************************************************************************
    if(Motor->CheckIsSafeDoorOpen())                                            //Jimmychiu 20221013 safedoor判斷整合Function
    {
        return false;
    }

    char str[512]="";
    char tmpstr[512]="";
    AnsiString str2="";

    static int iCount=0;                                                        //kevin 20220221 add Gali  Command state  Mylin 20211228 Add Gali Command Log
    static bool MOTMovflag[]={true, true, true, true};
    int iEncoderZ1=0, iEncoderY1=0;
    int iEncoderZ2=0, iEncoderY2=0;

    GetIndexParm();                                                             //取得移動距離

    if(Motor->Enable && bGali_CardInstall==true)
    {
        if(bShowMotorMove==true)
        {
            MOT[MTestY1].Position=MOT[MTestY1].Gali_ReadPos();
            if(USE_INDEX_ARM_AXES==IndexArm_4_Axis)                             //JimmyChiu 20220708 : add Index Arm Axis
            {
                MOT[MTestY2].Position=MOT[MTestY2].Gali_ReadPos();
            }
        }

        if(MovFlag==false && bZ1Z2Exute==false)                                 //kevin 20110630
        {
            if(CheckTestZ2("Z1DownZ2Up1"))                                      //確認四軸command & encoder位置是否不同
            {
                iGali_VsSpeed=Speed;                                            //?與 Z1UpZ2Down 不同?
                MOT[MTestZ2].GetRealPos(&Z2Safe);

                #ifdef DEBUG_GALIL_CHECK_Y
                if(Check_Y1Y2_TargetPosWillCrash(-XShiftF, -XShiftR))       //JerryYang 20250822 : fix index y保護  //JerryYang 20241129 : 計算Index Y移動後點位是否兩支ARM同時在SOCKET
                {
                    str2.sprintf("Index Y Position Calculate error, %s",__FUNC__);
                    ShowMyMessage(str2);
                    fAllMotorHome=false;
                    return false;
                }
                #endif

                MOT[MTestY1].GetRealPos(&XShiftF);
                MOT[MTestZ1].GetRealPos(&Z1DownSafe);
                MOT[MTestZ2].GetRealPos(&Z2Up);
                if(USE_INDEX_ARM_AXES==IndexArm_4_Axis)                         //JimmyChiu 20220708 : add Index Arm Axis
                {
                    MOT[MTestY2].GetRealPos(&XShiftR);
                }
                MOT[MTestZ1].GetRealPos(&Z1Down);
                #ifdef DEBUG_GALIL
                MOT[MTestZ1].GetRealPos(&Z1Down2Speed);
                #endif
                MOT[MTestY1].GetRealPos(&AxisY_Pre_MovePos);                    //2013-01-15    Dell   提高Index 運動速度(目前by9046LS專用)

                if(IniConfig.bD38IndexPutICToShtNoWaitMotion &&
                   bZ1DownZ2UpAndPlace==true                 &&
                   IniConfig.bIndexArm2SupplyLight==false    &&
                   TestIF_File.bForEgisTecTest==false        &&
                   TestIF_File.bArm1PickPlaceArm2Test==false)
                {
                    if(USE_INDEX_ARM_AXES==IndexArm_3_Axis)                     //JimmyChiu 20220708 : add Index Arm Axis
                    {
                        W4G_sprintf(str,"LMXYZ;"
                                "LI0,0,%d;"
                                "LI%d,%d,%d;"
                                "LI0,%d,%d;"
                                "LE;VS%d;VA%d;VD%d;BGS;",
                                -Z2Safe,
                                -XShiftF, -Z1DownSafe, -Z2Up,
                                -Z1Down, -Z2DownToShuttle,
                                Speed, GailAcSpeed, GailDcSpeed);
                    }
                    else
                    {
                        W4G_sprintf(str,"LMXYZW;"
                                "LI0,0,%d,0;"
                                "LI%d,%d,%d,%d;"
                                "LI0,%d,%d,0;"
                                "LE;VS%d;VA%d;VD%d;BGS;",
                                -Z2Safe,
                                -XShiftF, -Z1DownSafe, -Z2Up, XShiftR,
                                -Z1Down, -Z2DownToShuttle,
                                Speed, GailAcSpeed, GailDcSpeed);
                    }
                }
                else                                                            ////20111114  Dell for Disable Index Arm    Start
                {
                    #ifdef DEBUG_GALIL
                    W4G_sprintf(str, "LMXYZW;"
                                 "LI0,0,%d,0;"
                                 "LI%d,%d,%d,%d;"
                                 "LI0,%d,0,0 >5000;"
                                 "LI0,%d,0,0 <5000;"
                                 "LE;"
                                 "VS%d;VA%d;VD%d;BGS;",
                                 -Z2Safe,
                                 -XShiftF, -Z1DownSafe, -Z2Up, XShiftR,
                                 -Z1Down,
                                 Z1Down2Speed,
                                 Speed, GailAcSpeed, GailDcSpeed);
                    #else
                    if(TestIF_File.iShuttleMode==1 && IniConfig.bShuttleMode50==false &&
                       TestIF_File.iShuttle_Sel==0 && Z2Safe==0)                //jou 2014-10-23 增加index arm轉換保護
                    {
                        if(USE_INDEX_ARM_AXES==IndexArm_3_Axis)                 //JimmyChiu 20220708 : add Index Arm Axis
                            W4G_sprintf(tmpstr, "LMXYZ;LI%d,%d,%d;",
                                            -XShiftF, -Z1DownSafe, -Z2Up);
                        else
                            W4G_sprintf(tmpstr, "LMXYZW;LI%d,%d,%d,%d;",
                                            -XShiftF, -Z1DownSafe, -Z2Up, XShiftR);
                    }
                    else
                    {
                        if(USE_INDEX_ARM_AXES==IndexArm_3_Axis)                 //JimmyChiu 20220708 : add Index Arm Axis
                        {
                            W4G_sprintf(tmpstr, "LMXYZ;"
                                            "LI0,0,%d;"
                                            "LI%d,%d,%d;",
                                            -Z2Safe,
                                            (-XShiftF), -Z1DownSafe, -Z2Up);
                        }
                        else if(MachineTypeChoice==Type_HT9046_LS &&            //KenHsieh 20230508 : 修正關Arm之Arm偷跑問題
                                TestIF_File.iShuttleMode==1 && IniConfig.bShuttleMode50==false)
                        {
                            W4G_sprintf(tmpstr, "LMXYZW;LI0,0,%d,0;LI%d,%d,%d,%d;",
                                            -Z2Safe, -XShiftF, -Z1DownSafe, -Z2Up, XShiftR);
                        }
                        else if(REAL_TIME_CCD && COM2->bCCDDummyRum==false && TestIF_File.bUseRTCStepAsideMode) //Ifor 20230908 add:RTC 讓位功能 By Setup File
                        {
                            W4G_sprintf(tmpstr, "LMXYZW;"
                                            "LI0,0,%d,0;"
                                            "LI%d,%d,%d,%d;"
                                            "LI%d,%d,%d,%d;",
                                            -Z2Safe,
                                            0, 0, -Z2Up, XShiftR,
                                            -XShiftF, -Z1DownSafe, 0, 0);
                        }
                        else if(MachineTypeChoice==Type_HT9046_LS &&
                                bGail_HighSpeedMode==true)                      //2013-01-15    Dell   提高Index 運動速度(目前by9046LS專用)    //jou 2014-10-23 增加index arm轉換保護
                        {
                            W4G_sprintf(tmpstr, "LMXYZW;LI%d,0,%d,0;LI%d,%d,%d,%d<%d>%d;",
                                            AxisY_Pre_MovePos, -Z2Safe, (-XShiftF-AxisY_Pre_MovePos), -Z1DownSafe, -Z2Up, XShiftR, MOT[MTestY1].GailSpeed, Speed);
                        }
                        else
                        {
                            W4G_sprintf(tmpstr, "LMXYZW;LI0,0,%d,0;LI%d,%d,%d,%d;",
                                            -Z2Safe, -XShiftF, -Z1DownSafe, -Z2Up, XShiftR);
                        }
                    }

                    if(TestIF_File.iShuttleMode==1 && IniConfig.bShuttleMode50==false &&
                       Z1Down==0)                                               //jou 2014-10-23 增加index arm轉換保護
                    {
                        W4G_sprintf(str, "%sLE;VS%d;VA%d;VD%d;BGS;",
                                     tmpstr, Speed, GailAcSpeed, GailDcSpeed);
                    }
                    else
                    {
                        W4G_sprintf(str, "%sLI0,%d,0,0;LE;VS%d;VA%d;VD%d;BGS;",
                                     tmpstr, -Z1Down, Speed, GailAcSpeed, GailDcSpeed);
                    }
                    #endif
                }

                bScanFlag=false;
                if(DoIndexVectorMove(str))
                {
                    bZ1Z2Exute=true;                                            //kevin 20110630
                }
                else
                {
                    return false;
                }

                MovFlag=true;
                for(int i=0; i<Gali_MaxAxis; i++)
                {
                    MOTMovflag[i]=true;
                    MOT[MTestY1+i].Led[iInposLed]=true;
                }
            }
        }
        else
        {
            if(USE_INDEX_ARM_AXES==IndexArm_4_Axis)
            {
                if(GALI_PROTECT_MODE==0)                                        //Steven 20260504 : Mode 0 = Original JerryYang check
                {
                    iEncoderZ1=MOT[MTestZ1].Gali_ReadEncoderPos();              //JerryYang 20251124 : add portect, 避免index arm撞機
                    iEncoderY1=MOT[MTestY1].Gali_ReadEncoderPos();
                    iEncoderZ2=MOT[MTestZ2].Gali_ReadEncoderPos();
                    iEncoderY2=MOT[MTestY2].Gali_ReadEncoderPos();

                    if(iEncoderZ1<Prod.All_TestZ_Test_Safe-iCheckZ)             //JerryYang 20260114 : 輕壓的速度比較快, 放寬range   //JerryYang 20251224 : 3000->4000
                    {
                        if(CheckArmPosArrival(iEncoderY1, Prod.TestY1_Middle, IniConfig.GaliPosRange)==false)
                        {
                            ShowIndexMotorError(AnsiString("Z1DownZ2Up1"));
                        }
                    }

                    if(iEncoderZ1<-4000 && iEncoderY1<9000)                     //JerryYang 20260114 : 檢查下降過程是否會尻到SHUTTLE
                    {
                        ShowIndexMotorError(AnsiString("Z1DownZ2Up1_TEST"));
                    }

                    if(iEncoderZ2<-4000 && iEncoderY2>-9000)                    //JerryYang 20260114 : 檢查下降過程是否會尻到SHUTTLE
                    {
                        ShowIndexMotorError(AnsiString("Z1UpZ2Down1_TEST2"));
                    }
                }
                else if(GALI_PROTECT_MODE==2)                                   //Steven 20260504 : Mode 2 = TE (Following Error) check
                {
                    int iFollowErrY1 = MOT[MTestY1].Gali_Command("TEX", __FUNC__);
                    int iFollowErrZ1 = MOT[MTestZ1].Gali_Command("TEY", __FUNC__);
                    int iFollowErrZ2 = MOT[MTestZ2].Gali_Command("TEZ", __FUNC__);
                    int iFollowErrY2 = MOT[MTestY2].Gali_Command("TEW", __FUNC__);

                    int iTeThreshold = iGali_VsSpeed / 50 + 1000;

                    if(abs(iFollowErrY1) > iTeThreshold || abs(iFollowErrZ1) > iTeThreshold ||
                       abs(iFollowErrZ2) > iTeThreshold || abs(iFollowErrY2) > iTeThreshold)
                    {
                        ShowIndexMotorError(AnsiString("Z1DownZ2Up1_FE"));
                    }
                }
                //Mode 1 (ER+OE): No host-side in-motion check needed, Galil firmware handles it
            }

            long lSC=Gali_Command("SC");
            if(lSC==100)                                                        //ChungHung 20150831 add 確保每次皆到位
            {
                iCount++;
                if(iCount>5000)
                {
                    fMain->sb_Gali_LogClick();                                  //Mylin 20211228 Add Gali Command Log
                    iCount=0;
                    iHome =1;
                    ShowMyMessage("Gali Command Error When Z1DownZ2Up1, Please Home and Start", "請更換Galil 轉板或100P線材");
                }
                return false;
            }
            iCount=0;                                                           //kevin 20220221 add Gali  Command state Mylin 20211228 Add Gali Command Log

            if(bScanFlag==false)
            {
                for(int i=0; i<Gali_MaxAxis; i++)
                {
                    if(USE_INDEX_ARM_AXES==IndexArm_3_Axis &&                   //JimmyChiu 20220708 : add Index Arm Axis
                       (MTestY1+i)==MTestY2)
                    {
                        MOTMovflag[i]=false;
                    }
                    else
                    {
                        if(MOTMovflag[i])
                        {
                            MOT[MTestY1+i].Gali_ScanMotStatus();
                            MOTMovflag[i]=MOT[MTestY1+i].Led[iInposLed];
                        }
                    }
                }
            }

            if(MOTMovflag[0]==false && MOTMovflag[1]==false && MOTMovflag[2]==false && MOTMovflag[3]==false)
            {
                bScanFlag=true;
                GaliSofDelayCount++;
                if(GaliSofDelayCount>=DelayCount)
                {
                    if(CheckPos(false)==false)                                  //now only check TestY
                    {
                        if(GaliSofDelayCount<=(DelayCount*5))
                            return false;
                        iHome=1;
                        JamCode = MotorIndexToJamCode(13);
                        ShowMotorErrorMessage(JamCode,MotErrPos+1);
                        bScanFlag=false;
                        GaliSofDelayCount=0;
                        MovFlag=false;
                        return false;
                    }

                    if(ISZ1Down_Z2Up())                                         //check z1 & z2 encoder position
                    {
                        bScanFlag=false;
                        GaliSofDelayCount=0;
                        MovFlag=false;
                        bZ1Z2Exute=false;                                       //kevin 20110630

                        IndexStatus=Z1Down_Z2Up;
                        fOffSet->bEnterSpecialOffset=false;
                        return true;
                    }
                    else
                    {
                        iHome=1;
                        if(GaliAxisAlarm[1])
                            JamCode=MotorIndexToJamCode(14);
                        else
                            JamCode=MotorIndexToJamCode(15);

                        ShowMotorErrorMessage(JamCode,MotErrPos+1);
                        bScanFlag=false;
                        GaliSofDelayCount=0;
                        MovFlag=false;
                        return false;
                    }
                }
            }
        }
    }
    else
    {
        Speed=ArmSpeed[IndexArm].iBodySP*10*W906_SimGalilStepScale();  //AI(W906-SIMSTEP) 20260924: 模擬步進 ×W906_SimGalilStepScale()（B13 500 ms tick；出貨組態＝1，見本檔檔尾）
        if(Speed<=0)
            Speed=10;
        MOT[MTestZ2].Position-=Speed;
        MOT[MTestZ1].Position+=Speed;
        if(MOT[MTestZ2].Position<=0)
            MOT[MTestZ2].Position=0;
        MOT[MTestY1].Position+=Speed;
        if(USE_INDEX_ARM_AXES==IndexArm_4_Axis)                                 //JimmyChiu 20220708 : add Index Arm Axis
        {
            MOT[MTestY2].Position+=Speed;
        }

        MOT[MTestY1].TargetPosition=Prod.TestY1_Middle;                         //Steven 20210827 : 修改顯示Galil的Target / Current
        if(USE_INDEX_ARM_AXES==IndexArm_4_Axis)                                 //JimmyChiu 20220708 : add Index Arm Axis
            MOT[MTestY2].TargetPosition=Prod.TestY2_Rear;
        MOT[MTestZ1].TargetPosition=-Prod.TestZ1_Test;
        MOT[MTestZ2].TargetPosition=0;

        bool bY2PosSafe=(USE_INDEX_ARM_AXES==IndexArm_4_Axis)?(MOT[MTestY2].Position>=Prod.TestY2_Rear):true;  //JimmyChiu 20220708 : add Index Arm Axis
        if(MOT[MTestZ2].Position<=0 || bY2PosSafe)
        {
            IndexStatus=Z1Down_Z2Up;
            MOT[MTestY1].Position=Prod.TestY1_Middle;
            if(USE_INDEX_ARM_AXES==IndexArm_4_Axis)                             //JimmyChiu 20220708 : add Index Arm Axis
                MOT[MTestY2].Position=Prod.TestY2_Rear;
            MOT[MTestZ1].Position=-Prod.TestZ1_Test;
            MOT[MTestZ2].Position=0;
            fOffSet->bEnterSpecialOffset=false;
            return true;
        }
        return false;
    }
    return false;
}
//------------------------------------------------------------------------------
bool TMyMotor::Z1UpZ2Down2(int Speed, bool bPickErr)
{
    // AI(W906-P0-5) 20260920: 離線守衛 —— 值與退休前 mymotor.cpp 的樁**完全相同**，所以 Motor==NULL 的可觀察行為一格沒變。
    if(Motor==NULL) return false;
    //******************************************************************************
    //  注意!! CheckIsSafeDoorOpen為Handler 安全門相關, 修改時要小心!!
    //******************************************************************************
    if(Motor->CheckIsSafeDoorOpen())                                            //Jimmychiu 20221013 safedoor判斷整合Function
    {
        return false;
    }

    static bool MOTMovflag[]={true, true, true, true};
    static bool bMOTMoveFinish[]={false, false, false, false};

    GetIndexParm();
    if(Motor->Enable && bGali_CardInstall==true)
    {
        if(bShowMotorMove==true)
        {
            MOT[MTestY1].Position=MOT[MTestY1].Gali_ReadPos();
            if(USE_INDEX_ARM_AXES==IndexArm_4_Axis)                             //JimmyChiu 20220708 : add Index Arm Axis
                MOT[MTestY2].Position=MOT[MTestY2].Gali_ReadPos();
        }

        if(MovFlag==false && bZ1Z2Exute==false)                                 //kevin 20110630
        {
            if(CheckTestZ1("Z1UpZ2Down2"))
            {
                bScanFlag=false;
                if(ScanIndexMotorCanMove())
                {
                    bZ1Z2Exute=true;                                            //kevin 20110630
                }
                else
                {
                    return false;
                }

                for(int i=0; i<Gali_MaxAxis; i++)
                {
                    MOTMovflag[i]=true;
                    MOT[MTestY1+i].Led[iInposLed]=true;
                    bMOTMoveFinish[i]=false;
                }
            }
        }
        else if(!bMOTMoveFinish[0] || !bMOTMoveFinish[1] ||
                !bMOTMoveFinish[2] || !bMOTMoveFinish[3])                       //Sam 202400510 : T Movw 補 RTC 讓位功能
        {
            if(REAL_TIME_CCD && COM2->bCCDDummyRum==false &&
               TestIF_File.bUseRTCStepAsideMode)                                //Sam 202400510 : T Move 補 RTC 讓位功能
            {
                if(!bMOTMoveFinish[0])
                {
                    bMOTMoveFinish[0]=MOT[MTestZ1].Gali_Two_ZAxis_Move(Prod.TestZ1_Safe, Speed*T_MODE_SPEED, "Z1UpZ2Down2_1");
                }
                else if(!bMOTMoveFinish[1])
                {
                    bMOTMoveFinish[1]=MOT[MTestY1].Gali_MotMove(Prod.TestY1_Front, Speed, "Z1UpZ2Down2_2");
                }
                else if(!bMOTMoveFinish[2])
                {
                    if(USE_INDEX_ARM_AXES==IndexArm_3_Axis)                     //JimmyChiu 20220708 : add Index Arm Axis
                    {
                        bMOTMoveFinish[2]=MOT[MTestY1].Gali_MotMove(Prod.TestY1_Front, Speed, "Z1UpZ2Down2_3");
                    }
                    else
                    {
                        bMOTMoveFinish[2]=MOT[MTestY2].Gali_MotMove(Prod.TestY2_Middle, Speed, "Z1UpZ2Down2_4");
                    }
                }
                else if(!bMOTMoveFinish[3])
                {
                    bMOTMoveFinish[3]=MOT[MTestZ2].Gali_MotMove(Prod.TestZ2_Test, Speed*T_MODE_SPEED, "Z1UpZ2Down2_5");
                }
            }
            else
            {
                bMOTMoveFinish[3]=true;                                         //Sam 202400510 : T Move 補 RTC 讓位功能
                if(!bMOTMoveFinish[0])
                {
                    bMOTMoveFinish[0]=MOT[MTestZ1].Gali_Two_ZAxis_Move(Prod.TestZ1_Safe, Speed*T_MODE_SPEED, "Z1UpZ2Down2_6");   //jou 2012-05-21 T mode index會跳電
                }
                else if(!bMOTMoveFinish[1])
                {
                    bMOTMoveFinish[1]=MOT[MTestY1].GalilTwoY_Move(Prod.TestY1_Front, Prod.TestY2_Middle, Speed, "Z1UpZ2Down2_7");
                    if(bMOTMoveFinish[1]==true && bPickErr==true)               //jou 2012-02-29 index pick up error,index arm move to center & alarm
                    {
                        ShowMyMessage("Test Arm 2 Pick up error after Skip, need take away Arm 2 device", "請檢查index arm上是否有殘留Device", "Z1UpZ2Down2_8");
                        SoftStop=true;
                        StopAllMotor();
                    }
                }
                else if(!bMOTMoveFinish[2])
                {
                    bMOTMoveFinish[2]=MOT[MTestZ2].Gali_MotMove(Prod.TestZ2_Test, Speed*T_MODE_SPEED, "Z1UpZ2Down2_9");   //jou 2012-05-21 T mode index會跳電
                }
            }
        }
        else
        {
            if(bScanFlag==false)
            {
                for(int i=0; i<Gali_MaxAxis; i++)
                {
                    if(USE_INDEX_ARM_AXES==IndexArm_3_Axis &&                   //JimmyChiu 20220708 : add Index Arm Axis
                       (MTestY1+i)==MTestY2)
                    {
                        MOTMovflag[i]=false;
                    }
                    else
                    {
                        if(MOTMovflag[i])
                        {
                            MOT[MTestY1+i].Gali_ScanMotStatus();
                            MOTMovflag[i]=MOT[MTestY1+i].Led[iInposLed];
                        }
                    }
                }
            }

            if(MOTMovflag[0]==false && MOTMovflag[1]==false && MOTMovflag[2]==false && MOTMovflag[3]==false)
            {
                bScanFlag=true;
                GaliSofDelayCount++;
                if(GaliSofDelayCount>=DelayCount)
                {
                    if(CheckPos(true)==false)                                   //now only check TestY
                    {
                        if(GaliSofDelayCount<=(DelayCount*5))
                            return false;
                        iHome=1;
                        JamCode = MotorIndexToJamCode(13);
                        ShowMotorErrorMessage(JamCode, MotErrPos+1, "Z1UpZ2Down2_10");
                        bScanFlag=false;
                        GaliSofDelayCount=0;
                        MovFlag=false;
                        return false;
                    }

                    if(ISZ1Up_Z2Down())
                    {
                        bScanFlag=false;
                        GaliSofDelayCount=0;
                        MovFlag=false;
                        bZ1Z2Exute=false;                                       //kevin 20110630

                        IndexStatus=Z1Up_Z2Down;
                        fOffSet->bEnterSpecialOffset=false;
                        return true;
                    }
                    else
                    {
                        iHome=1;
                        if(GaliAxisAlarm[1])
                            JamCode=MotorIndexToJamCode(14);
                        else
                            JamCode=MotorIndexToJamCode(15);

                        ShowMotorErrorMessage(JamCode, MotErrPos+1, "Z1UpZ2Down2_11");
                        bScanFlag=false;
                        GaliSofDelayCount=0;
                        MovFlag=false;
                        return false;
                    }
                }
            }
        }
    }
    else
    {
        Speed=ArmSpeed[IndexArm].iBodySP*10*W906_SimGalilStepScale();  //AI(W906-SIMSTEP) 20260924: 模擬步進 ×W906_SimGalilStepScale()（B13 500 ms tick；出貨組態＝1，見本檔檔尾）
        if(Speed<=0)
            Speed=10;
        MOT[MTestZ2].Position+=Speed;
        MOT[MTestZ1].Position-=Speed;
        if(MOT[MTestZ2].Position<=0)
            MOT[MTestZ2].Position=0;
        MOT[MTestY1].Position-=Speed;
        if(USE_INDEX_ARM_AXES==IndexArm_4_Axis)
            MOT[MTestY2].Position-=Speed;

        MOT[MTestY1].TargetPosition=Prod.TestY1_Front;                          //Steven 20210827 : 修改顯示Galil的Target / Current
        if(USE_INDEX_ARM_AXES==IndexArm_4_Axis)
            MOT[MTestY2].TargetPosition=Prod.TestY2_Middle;
        MOT[MTestZ1].TargetPosition=0;
        MOT[MTestZ2].TargetPosition=-Prod.TestZ2_Test;

        if(MOT[MTestZ1].Position<=0 || MOT[MTestY1].Position<=Prod.TestY1_Front)
        {
            IndexStatus=Z1Up_Z2Down;
            MOT[MTestY1].Position=Prod.TestY1_Front;
            if(USE_INDEX_ARM_AXES==IndexArm_4_Axis)
                MOT[MTestY2].Position=Prod.TestY2_Middle;
            MOT[MTestZ1].Position=0;
            MOT[MTestZ2].Position=-Prod.TestZ2_Test;
            fOffSet->bEnterSpecialOffset=false;
            return true;
        }
        return false;
    }
    return false;
}
//------------------------------------------------------------------------------
bool TMyMotor::Z1DownZ2Up2(int Speed, bool bPickErr)
{
    // AI(W906-P0-5) 20260920: 離線守衛 —— 值與退休前 mymotor.cpp 的樁**完全相同**，所以 Motor==NULL 的可觀察行為一格沒變。
    if(Motor==NULL) return false;
    //******************************************************************************
    //  注意!! CheckIsSafeDoorOpen為Handler 安全門相關, 修改時要小心!!
    //******************************************************************************
    if(Motor->CheckIsSafeDoorOpen())                                            //Jimmychiu 20221013 safedoor判斷整合Function
    {
        return false;
    }

    static bool MOTMovflag[]={true, true, true, true};
    static bool bMOTMoveFinish[]={false, false, false, false};
    GetIndexParm();                                                             //取得移動距離
    if(Motor->Enable && bGali_CardInstall==true)
    {
        if(bShowMotorMove==true)
        {
            MOT[MTestY1].Position=MOT[MTestY1].Gali_ReadPos();
            if(USE_INDEX_ARM_AXES==IndexArm_4_Axis)                                     //JimmyChiu 20220708 : add Index Arm Axis
            {
                MOT[MTestY2].Position=MOT[MTestY2].Gali_ReadPos();
            }
        }

        if(MovFlag==false && bZ1Z2Exute==false)                                 //kevin 20110630
        {
            if(CheckTestZ2("Z1DownZ2Up2"))                                      //確認四軸command & encoder位置是否不同
            {
                bScanFlag=false;
                if(ScanIndexMotorCanMove())                                     //確認四軸是否alarm
                {
                    bZ1Z2Exute=true;                                            //kevin 20110630
                }
                else
                {
                    return false;
                }

                for(int i=0; i<Gali_MaxAxis; i++)
                {
                    MOTMovflag[i]=true;
                    MOT[MTestY1+i].Led[iInposLed]=true;
                    bMOTMoveFinish[i]=false;
                }
            }
        }
        else if(!bMOTMoveFinish[0] || !bMOTMoveFinish[1] ||
                !bMOTMoveFinish[2] || !bMOTMoveFinish[3])                       //Sam 202400510 : T Movw 補 RTC 讓位功能
        {
            if(REAL_TIME_CCD && COM2->bCCDDummyRum==false &&
               TestIF_File.bUseRTCStepAsideMode)                                //Sam 202400510 : T Move 補 RTC 讓位功能
            {
                if(!bMOTMoveFinish[0])
                {
                    bMOTMoveFinish[0]=MOT[MTestZ1].Gali_Two_ZAxis_Move(Prod.TestZ1_Safe, Speed*T_MODE_SPEED, "Z1DownZ2Up2_1");
                }
                else if(!bMOTMoveFinish[1])
                {
                    if(USE_INDEX_ARM_AXES==IndexArm_3_Axis)                     //JimmyChiu 20220708 : add Index Arm Axis
                    {
                        bMOTMoveFinish[1]=MOT[MTestY1].Gali_MotMove(Prod.TestY1_Middle, Speed, "Z1DownZ2Up2_2");
                    }
                    else
                    {
                        bMOTMoveFinish[1]=MOT[MTestY2].Gali_MotMove(Prod.TestY2_Rear, Speed, "Z1DownZ2Up2_3");
                    }
                }
                else if(!bMOTMoveFinish[2])
                {
                    bMOTMoveFinish[2]=MOT[MTestY1].Gali_MotMove(Prod.TestY1_Middle, Speed, "Z1DownZ2Up2_4");
                }
                else if(!bMOTMoveFinish[3])
                {
                    bMOTMoveFinish[3]=MOT[MTestZ1].Gali_MotMove( Prod.TestZ1_Test, Speed*T_MODE_SPEED, "Z1DownZ2Up2_5");
                }
            }
            else
            {
                bMOTMoveFinish[3]=true;                                         //Sam 202400510 : T Move 補 RTC 讓位功能
                if(!bMOTMoveFinish[0])
                {
                    bMOTMoveFinish[0]=MOT[MTestZ1].Gali_Two_ZAxis_Move(Prod.TestZ1_Safe, Speed*T_MODE_SPEED, "Z1DownZ2Up2_6");   //jou 2012-05-21 T mode index會跳電
                }
                else if(!bMOTMoveFinish[1])
                {
                    bMOTMoveFinish[1]=MOT[MTestY1].GalilTwoY_Move(Prod.TestY1_Middle, Prod.TestY2_Rear, Speed, "Z1DownZ2Up2_7");//JimmyChiu 20220708 : add Index Arm Axis
                    if(bMOTMoveFinish[1]==true && bPickErr==true)
                    {
                        ShowMyMessage("Test Arm 1 Pick up error after Skip, need take away Arm 1 device", "請檢查index arm上是否有殘留Device", "Z1DownZ2Up2_8");
                        SoftStop=true;
                        StopAllMotor();
                    }
                }
                else if(!bMOTMoveFinish[2])
                {
                    bMOTMoveFinish[2]=MOT[MTestZ1].Gali_MotMove( Prod.TestZ1_Test, Speed*T_MODE_SPEED, "Z1DownZ2Up2_9");   //jou 2012-05-21 T mode index會跳電
                }
            }
        }
        else
        {
            if(bScanFlag==false)
            {
                for(int i=0; i<Gali_MaxAxis; i++)
                {
                    if(USE_INDEX_ARM_AXES==IndexArm_3_Axis &&                   //JimmyChiu 20220708 : add Index Arm Axis
                       (MTestY1+i)==MTestY2)
                    {
                        MOTMovflag[i]=false;
                    }
                    else
                    {
                        if(MOTMovflag[i])
                        {
                            MOT[MTestY1+i].Gali_ScanMotStatus();
                            MOTMovflag[i]=MOT[MTestY1+i].Led[iInposLed];
                        }
                    }
                }
            }

            if(MOTMovflag[0]==false && MOTMovflag[1]==false && MOTMovflag[2]==false && MOTMovflag[3]==false)
            {
                bScanFlag=true;

                GaliSofDelayCount++;
                if(GaliSofDelayCount>=DelayCount)
                {
                    if(CheckPos(false)==false)                                  //now only check TestY
                    {
                        if(GaliSofDelayCount<=(DelayCount*5))
                            return false;
                        iHome=1;
                        JamCode = MotorIndexToJamCode(13);
                        ShowMotorErrorMessage(JamCode, MotErrPos+1, "Z1DownZ2Up2_10");
                        bScanFlag=false;
                        GaliSofDelayCount=0;
                        MovFlag=false;
                        return false;
                    }

                    if(ISZ1Down_Z2Up())                                         //check z1 & z2 encoder position
                    {
                        bScanFlag=false;
                        GaliSofDelayCount=0;
                        MovFlag=false;
                        bZ1Z2Exute=false;                                       //kevin 20110630

                        IndexStatus=Z1Down_Z2Up;
                        fOffSet->bEnterSpecialOffset=false;
                        return true;
                    }
                    else
                    {
                        iHome=1;
                        if(GaliAxisAlarm[1])
                            JamCode = MotorIndexToJamCode(14);
                        else
                            JamCode = MotorIndexToJamCode(15);

                        ShowMotorErrorMessage(JamCode,MotErrPos+1, "Z1DownZ2Up2_11");
                        bScanFlag=false;
                        GaliSofDelayCount=0;
                        MovFlag=false;
                        return false;
                    }
                }
            }
        }
    }
    else
    {
        Speed=ArmSpeed[IndexArm].iBodySP*10*W906_SimGalilStepScale();  //AI(W906-SIMSTEP) 20260924: 模擬步進 ×W906_SimGalilStepScale()（B13 500 ms tick；出貨組態＝1，見本檔檔尾）
        if(Speed<=0)
            Speed=10;
        MOT[MTestZ2].Position-=Speed;
        MOT[MTestZ1].Position+=Speed;
        if(MOT[MTestZ2].Position<=0)
            MOT[MTestZ2].Position=0;
        MOT[MTestY1].Position+=Speed;
        if(USE_INDEX_ARM_AXES==IndexArm_4_Axis)
            MOT[MTestY2].Position+=Speed;

        MOT[MTestY1].TargetPosition=Prod.TestY1_Middle;                         //Steven 20210827 : 修改顯示Galil的Target / Current
        if(USE_INDEX_ARM_AXES==IndexArm_4_Axis)
            MOT[MTestY2].TargetPosition=Prod.TestY2_Rear;
        MOT[MTestZ1].TargetPosition=-Prod.TestZ1_Test;
        MOT[MTestZ2].TargetPosition=0;

        bool bY2PosSafe=(USE_INDEX_ARM_AXES==IndexArm_4_Axis)?(MOT[MTestY2].Position>=Prod.TestY2_Rear):true;  //JimmyChiu 20220708 : add Index Arm Axis
        if(MOT[MTestZ2].Position<=0 || bY2PosSafe)
        {
            IndexStatus=Z1Down_Z2Up;
            MOT[MTestY1].Position=Prod.TestY1_Middle;
            if(USE_INDEX_ARM_AXES==IndexArm_4_Axis)
                MOT[MTestY2].Position=Prod.TestY2_Rear;
            MOT[MTestZ1].Position=-Prod.TestZ1_Test;
            MOT[MTestZ2].Position=0;
            fOffSet->bEnterSpecialOffset=false;
            return true;
        }
        return false;
    }
    return false;
}
//------------------------------------------------------------------------------
bool TMyMotor::Z1UpZ2Down(int Speed, bool TMode, bool bPickErr)
{
    // AI(W906-P0-5) 20260920: 離線守衛 —— 值與退休前 mymotor.cpp 的樁**完全相同**，所以 Motor==NULL 的可觀察行為一格沒變。
    if(Motor==NULL) return false;
    if(CosFunction.bSortingBy2DList==true &&                                    //JerryYang 20230322 : 增加保護, 2D SORT模式index arm不用下壓到socket
       TestIF_File.bSortingBy2DIDList==true &&
       LastSet.iTester==_2D_SORT)
    {
        ShowMyMessage("Z1UpZ2Down Error");
        return false;
    }
    bool ret=false;

    #ifdef INDEX_PROTECT_TMOVE
    if(bOverRangeDoTMode==true)                                                 //Isaac 20201012 : index Y超過範圍，做一次Tmode
    {
        if(bOverRange4Indexhome==true)
        {
            return false;
        }
        else
        {
            ret=Z1UpZ2Down2(Speed, bPickErr);
            if(ret==true)
            {
                bOverRangeDoTMode=false;
                bTriger4Indexhome=false;
                GetTimeInfo();
                InitialMaxMinValue("Z1UpZ2Down");                               //Isaac 20201012 : 計算Encoder和commandpos/Teaching的差值，歸零
                fMain->AddIndexPosLog("Save Log");
                fMain->AddIndexPosLog("Auto-calibration End.", true);
                RecordProcess("Auto-calibration End.");
            }
        }
    }
    else
    #endif
    {
        if(CosFunction.bSortingBy2DList==true &&                                //Frank 20221122 : 2DID sorting for ATK
           TestIF_File.bSortingBy2DIDList==true &&
           LastSet.iTester==_2D_SORT)
        {
            ShowMyMessage("Z1UpZ2Down Error");
            return false;
        }

        if(!TMode)
        {
            ret=Z1UpZ2Down1(Speed);
        }
        else
        {
            ret=Z1UpZ2Down2(Speed, bPickErr);
        }
    }
    return ret;
}
//------------------------------------------------------------------------------
bool TMyMotor::Z1DownZ2Up(int Speed, bool TMode, bool bPickErr)
{
    // AI(W906-P0-5) 20260920: 離線守衛 —— 值與退休前 mymotor.cpp 的樁**完全相同**，所以 Motor==NULL 的可觀察行為一格沒變。
    if(Motor==NULL) return false;
    if(CosFunction.bSortingBy2DList==true &&                                    //JerryYang 20230322 : 增加保護, 2D SORT模式index arm不用下壓到socket
       TestIF_File.bSortingBy2DIDList==true &&
       LastSet.iTester==_2D_SORT)
    {
        ShowMyMessage("Z1UpZ2Down Error");
        return false;
    }
    bool ret=false;

    #ifdef INDEX_PROTECT_TMOVE
    if(bOverRangeDoTMode==true)                                                 //Isaac 20201012 : index Y超過範圍，做一次Tmode
    {
        if(bOverRange4Indexhome==true)
        {
            return false;
        }
        else
        {
            ret=Z1DownZ2Up2(Speed, bPickErr);
            if(ret==true)
            {
                bOverRangeDoTMode=false;
                bTriger4Indexhome=false;
                GetTimeInfo();
                InitialMaxMinValue("Z1DownZ2Up");                               //Isaac 20201012 : 計算Encoder和commandpos/Teaching的差值，歸零
                fMain->AddIndexPosLog("Save Log");
                fMain->AddIndexPosLog("Auto-calibration End.", true);
                RecordProcess("Auto-calibration End.");
            }
        }
    }
    else
    #endif
    {
        if(CosFunction.bSortingBy2DList==true &&                                //Frank 20221122 : 2DID sorting for ATK
           TestIF_File.bSortingBy2DIDList==true &&
           LastSet.iTester==_2D_SORT)
        {
            ShowMyMessage("Z1UpZ2Down Error");
            return false;
        }

        if(!TMode)
        {
            ret=Z1DownZ2Up1(Speed);
        }
        else
        {
            ret=Z1DownZ2Up2(Speed, bPickErr);
        }
    }
    return ret;
}
//------------------------------------------------------------------------------
long TMyMotor::Gali_ReadPos()
{
    // AI(W906-P0-5) 20260920: 離線守衛 —— 值與退休前 mymotor.cpp 的樁**完全相同**，所以 Motor==NULL 的可觀察行為一格沒變。
    if(Motor==NULL) return 0;
    char str[256]="";
    long L1=0;
    AnsiString SelMot=AnsiString("");
    int Pos=0;
    if(Motor!=NULL &&                                                           //JimmyChiu 20250306 : 避免還沒初始化就操作馬達
       Motor->Enable && (bGali_CardInstall==true || W906_GaliRouteOwns(Mot_Name)))   //AI(W906-INDEXZ-1203) 20260929: golden card branch also for the routed axis (HT9050 M14 on the 1203, Motor/GaliRoute.h); no route installed = golden
    {
        SelMot=Gali_GetMOT(Mot_Name);
        W4G_sprintf(str, "TD%s", SelMot);
        L1=Gali_Command(str, __FUNC__);
        if(Mot_Name==MTestY2)
            L1=-L1;

        if(bShowMotorMove==true)
            ScreenPos=(int)(Scale*(L1-FactStart))+RefStart;

        Pos=Gali_Command(str, __FUNC__);
        if(Mot_Name==MTestY1)
            Position=Pos*Motor->GearRatio;
        else
            Position=-Pos*Motor->GearRatio;

        return Position;
    }
    else
    {
        if(bShowMotorMove==true)
        {
            ScreenPos=(int)(Scale*(Position-FactStart))+RefStart;
#if 0   // GATE(W4G-3) golden Motor/myGALILmotor.cpp:2675-2681 -- VERBATIM golden text
            if(bPanelUse)
            {
                if(bUpDownMove)
                    PWinCtrl->Top=ScreenPos;
                else
                    PWinCtrl->Left=ScreenPos;
            }
#else
            (void)bPanelUse;    // GATE(W4G-3): PWinCtrl is `void*` here (Motor/mymotor.h:104 "was TWinControl* (VCL); gated W7-UI"); ScreenPos above is still computed
#endif
        }

        if(Mot_Name==MTestY2)
            return Position;
        if(Mot_Name==MTestY1)
            return Position;
        else
            return -Position;
    }
}
//------------------------------------------------------------------------------
void TMyMotor::Gali_MotHome(AnsiString HomeAxis)
{
    // AI(W906-P0-5) 20260920: 離線守衛 —— 值與退休前 mymotor.cpp 的樁**完全相同**，所以 Motor==NULL 的可觀察行為一格沒變。
    if(Motor==NULL) return;
    //******************************************************************************
    //  注意!! CheckIsSafeDoorOpen為Handler 安全門相關, 修改時要小心!!
    //******************************************************************************
    if(Motor->CheckIsSafeDoorOpen())                                            //Jimmychiu 20221013 safedoor判斷整合Function
    {
        return;
    }

    AnsiString aTransferData="", Data="", Str="";
    if(HomeAxis=="X" && MovFlag)
        return;
    else if(HomeAxis=="Y" && MovFlag)
        return;
    else if(HomeAxis=="Z" && MovFlag)
        return;
    else if(HomeAxis=="W" && MovFlag)
        return;
    else if(HomeAxis=="YZ" && MovFlag)
        return;

    if(Motor->Enable && bGali_CardInstall==true)
    {
        aTransferData="";
        aTransferData="SP"+HomeAxis+"=2000;"+
                      "JG"+HomeAxis+"=-5000;"+
                      "AC"+HomeAxis+"=20000;"+
                      "DC"+HomeAxis+"=20000;"+
                      "HM"+HomeAxis+";"+
                      "BG"+HomeAxis+";";

        #ifndef SOFT_SIMULTE
            rc = DMCCommand(hDmc, aTransferData.c_str(), szBuffer1, sizeof(szBuffer1));
        #endif
        if(rc!=0)
        {
            SoftStop=true;
            SystemStart=false;
            Str=GetGalilErrString(rc, "Gali_MotHome");
            MyDBIProcessNew("Motion", "WAR2203", "Gail Command Err", AnsiString(aTransferData));
            fAllMotorHome=false;
            ShowErrorMessage("WAR1635", 0, MMSystem, 0, Str);
            return;
        }
        else
        {
            MovFlag=true;
        }
    }
}
//---------------------------------------------------------------------------
void TMyMotor::Gali_MotHomeFindZ(AnsiString HomeAxis)                           //ChungHung 20141016 add find Z相
{
    // AI(W906-P0-5) 20260920: 離線守衛 —— 值與退休前 mymotor.cpp 的樁**完全相同**，所以 Motor==NULL 的可觀察行為一格沒變。
    if(Motor==NULL) return;
    //******************************************************************************
    //  注意!! CheckIsSafeDoorOpen為Handler 安全門相關, 修改時要小心!!
    //******************************************************************************
    if(Motor->CheckIsSafeDoorOpen())                                            //Jimmychiu 20221013 safedoor判斷整合Function
    {
        return;
    }

    AnsiString aTransferData=AnsiString(""), Data=AnsiString(""), Str=AnsiString("");
    if(HomeAxis=="X" && MovFlag)
        return;
    else if(HomeAxis=="Y" && MovFlag)
        return;
    else if(HomeAxis=="Z" && MovFlag)
        return;
    else if(HomeAxis=="W" && MovFlag)
        return;
    else if(HomeAxis=="YZ" && MovFlag)
        return;

    if(Motor->Enable && bGali_CardInstall==true)
    {
        aTransferData="";
        //SP  Speed
        //JG  Jog
        //AC  Acceleration 加速
        //DC  Deceleration 減速
        //FI  Find Index
        //BG  Begin
        aTransferData="SP"+HomeAxis+"=300;"+                                    //Ifor 20170817 (Steven) "JG"+HomeAxis+"=-300;" ==> "JG"+HomeAxis+"=300;"
                          "JG"+HomeAxis+"=300;"+
                          "AC"+HomeAxis+"=500;"+
                          "DC"+HomeAxis+"=500;"+
                          "FI"+HomeAxis+";"+
                          "BG"+HomeAxis+";";
        #ifndef SOFT_SIMULTE
            rc=DMCCommand(hDmc, aTransferData.c_str(), szBuffer1, sizeof(szBuffer1));
        #endif
        if(rc!=0)
        {
            SoftStop=true;
            SystemStart=false;
            MyDBIProcessNew("Motion", "WAR2203", "Gail Command Err", AnsiString(aTransferData));
            Str=GetGalilErrString(rc, "Gali_MotHome");
            ShowErrorMessage("WAR1635", 0, MMSystem, 0, Str);
            fAllMotorHome=false;
            return;
        }
        else
        {
            MovFlag=true;
        }
    }
}
//---------------------------------------------------------------------------
TQPF_Timer tGalilTwoZMoveDelay;
bool bGalilTwoZMoveFlag=false;
bool TMyMotor::Gali_Two_ZAxis_Move(int Pos, int Speed, AnsiString sFunc, bool bTwoPos, int Pos2)  //Steven 20150911 : modify for 2 Index Z move
{
    // AI(W906-P0-5) 20260920: 離線守衛 —— 值與退休前 mymotor.cpp 的樁**完全相同**，所以 Motor==NULL 的可觀察行為一格沒變。
    if(Motor==NULL) return true;
    //******************************************************************************
    //  注意!! CheckIsSafeDoorOpen為Handler 安全門相關, 修改時要小心!!
    //******************************************************************************
    if(Motor->CheckIsSafeDoorOpen())                                            //Jimmychiu 20221013 safedoor判斷整合Function
    {
        return false;
    }

    char str[256]="";
    AnsiString SelMot=AnsiString(""), sErrFunc=__FUNC__+AnsiString(",Func2,")+sFunc;
    int Z1Pos=0, Z2Pos=0;
    bool flag[2]={false,false};

    if(Pos<0)                                                                   //JerryYang 20180411 (jou) : Z軸移動前確認Y軸位置保護
    {
        if(CosFunction.bIndexProtect==true)
        {
            if(CheckYPos()==false)
            {
                return false;
            }
        }
    }

    Z1Pos=0-Pos;
    if(bTwoPos)                                                                 //Steven 20150911 : modify for 2 Index Z move
    {
        Z2Pos=0-Pos2;

        if(Z2Pos==0)
        {
            Z2Pos=0;
        }
    }
    else
    {
        Z2Pos=0-Pos;
    }

    MOT[MTestZ1].GetRealPos(&Z1Pos);
    MOT[MTestZ2].GetRealPos(&Z2Pos);

    if(Pos==0)
    {
        Pos=0;
    }

    if(Motor->Enable && (bGali_CardInstall==true || W906_GaliRouteOwns(Mot_Name)))   //AI(W906-INDEXZ-1203) 20260929: golden card branch also for the routed axis (HT9050 M14 on the 1203, Motor/GaliRoute.h); no route installed = golden
    {
        MOT[MTestZ1].Position=MOT[MTestZ1].Gali_ReadPos();
        if(bGali_CardInstall==true || W906_GaliRouteOwns(MTestZ2)) MOT[MTestZ2].Position=MOT[MTestZ2].Gali_ReadPos();   //AI(W906-INDEXZ) 20260930: review #4 -- in here only through the route (no card), a Z2 that is neither on a card nor routed answers Gali_ReadPos's no-card `-Position` (:3680), so this line flipped Z2's sign on every call; it keeps its value instead. A card installed = golden
        if(MovFlag==false)
        {
            if(CheckTestZ(sErrFunc))
            {
                iGali_SpSpeed=Speed;
                W4G_sprintf(str, "SP,%d,%d;AC,%d,%d;DC,%d,%d;PA,%d,%d;BGYZ;",
                              Speed, Speed, GailAcSpeed, GailAcSpeed, GailDcSpeed, GailDcSpeed, Z1Pos, Z2Pos);

                if(ScanIndexMotorCanMove())
                {
                    Gali_Command(str, sErrFunc);
                }
                else
                {
                    return false;
                }

                MovFlag=true;
            }
        }
        else
        {
            if(Gali_Command("MG_BGy", sErrFunc)==1 ||                           //馬達目前是否移動 0:移動完成
               Gali_Command("MG_BGz", sErrFunc)==1)                             //ChungHung 20141016 add 確保每次皆到位
                return false;

            MOT[MTestZ1].Gali_ScanMotStatus();
            if(MOT[MTestZ1].Led[iInposLed]==false)
                flag[0]=true;
            MOT[MTestZ2].Gali_ScanMotStatus();
            if(MOT[MTestZ2].Led[iInposLed]==false)
                flag[1]=true;
            if(flag[0] && flag[1])
            {
                GaliSofDelayCount++;
                if(GaliSofDelayCount>=DelayCount)
                {
                    GaliSofDelayCount=0;
                    MovFlag=false;
                    IndexZCanMove[0]=true;                                      //jou 20200526 : 解除index hang up
                    IndexZCanMove[1]=true;
                    return true;
                }
            }
        }
    }
    else
    {
        MOT[MTestZ1].Position=-Pos;
        MOT[MTestZ2].Position=-Pos;
        MOT[MTestZ1].TargetPosition=Pos;
        MOT[MTestZ2].TargetPosition=Pos;
        IndexZCanMove[0]=true;                                                  //jou 20200526 : 解除index hang up
        IndexZCanMove[1]=true;
        return true;
    }
    return false;
}
//-----------------------------------------------------------------------------
void TMyMotor::Gali_JogP(int Speed)
{
    // AI(W906-P0-5) 20260920: 離線守衛 —— 值與退休前 mymotor.cpp 的樁**完全相同**，所以 Motor==NULL 的可觀察行為一格沒變。
    if(Motor==NULL) return;
    //******************************************************************************
    //  注意!! CheckIsSafeDoorOpen為Handler 安全門相關, 修改時要小心!!
    //******************************************************************************
    if(Motor->CheckIsSafeDoorOpen())                                            //Jimmychiu 20221013 safedoor判斷整合Function
    {
        return;
    }

    int JogData=Speed;
    AnsiString SelMot=AnsiString("");
    if(Mot_Name!=MTestY1)
        JogData=-JogData;
    char str[256]="";
    SelMot=Gali_GetMOT(Mot_Name);
    W4G_sprintf(str, "SP%s=%d;AC%s=%d;DC%s=%d;JG%s=%d;BG%s;",
                  SelMot, Speed, SelMot, GailAcSpeed, SelMot, GailDcSpeed, SelMot, JogData, SelMot);

    Gali_Command(str, __FUNC__);
}
//-----------------------------------------------------------------------------
void TMyMotor::Gali_JogPSetup(int Speed)
{
    // AI(W906-P0-5) 20260920: 離線守衛 —— 值與退休前 mymotor.cpp 的樁**完全相同**，所以 Motor==NULL 的可觀察行為一格沒變。
    if(Motor==NULL) return;
    //******************************************************************************
    //  注意!! CheckIsSafeDoorOpen為Handler 安全門相關, 修改時要小心!!
    //******************************************************************************
    if(Motor->CheckIsSafeDoorOpen())                                            //Jimmychiu 20221013 safedoor判斷整合Function
    {
        return;
    }
    int JogData=1000;
    AnsiString SelMot=AnsiString("");
    JogData=abs(Speed);
    if(Mot_Name!=MTestY1)
        JogData=-JogData;
    char str[256]="";
    SelMot=Gali_GetMOT(Mot_Name);
    W4G_sprintf(str, "SP%s=%d;AC%s=%d;DC%s=%d;JG%s=%d;BG%s;",
                  SelMot, 100, SelMot, GailAcSpeed, SelMot, GailDcSpeed, SelMot, JogData, SelMot);

    Gali_Command(str, __FUNC__);
}
//-----------------------------------------------------------------------------
void TMyMotor::Gali_JogPAndCount(int Speed, int Count)
{
    // AI(W906-P0-5) 20260920: 離線守衛 —— 值與退休前 mymotor.cpp 的樁**完全相同**，所以 Motor==NULL 的可觀察行為一格沒變。
    if(Motor==NULL) return;
    //******************************************************************************
    //  注意!! CheckIsSafeDoorOpen為Handler 安全門相關, 修改時要小心!!
    //******************************************************************************
    if(Motor->CheckIsSafeDoorOpen())                                            //Jimmychiu 20221013 safedoor判斷整合Function
    {
        return;
    }

    int JogData=9990;
    JogData=Count;
    AnsiString SelMot=AnsiString("");
    if(Mot_Name!=MTestY1)
        JogData=-JogData;
    char str[256]="";
    SelMot=Gali_GetMOT(Mot_Name);
    W4G_sprintf(str, "SP%s=%d;AC%s=%d;DC%s=%d;JG%s=%d;BG%s;",
                 SelMot, Speed, SelMot, GailAcSpeed, SelMot, GailDcSpeed, SelMot, JogData, SelMot);

    Gali_Command(str, __FUNC__);
}
//-----------------------------------------------------------------------------
void TMyMotor::Gali_JogN(int Speed)
{
    // AI(W906-P0-5) 20260920: 離線守衛 —— 值與退休前 mymotor.cpp 的樁**完全相同**，所以 Motor==NULL 的可觀察行為一格沒變。
    if(Motor==NULL) return;
    //******************************************************************************
    //  注意!! CheckIsSafeDoorOpen為Handler 安全門相關, 修改時要小心!!
    //******************************************************************************
    if(Motor->CheckIsSafeDoorOpen())                                            //Jimmychiu 20221013 safedoor判斷整合Function
    {
        return;
    }

    int JogData=-Speed;
    AnsiString SelMot=AnsiString("");
    if(Mot_Name!=MTestY1)
        JogData=-JogData;
    char str[256]="";
    SelMot=Gali_GetMOT(Mot_Name);
    W4G_sprintf(str, "SP%s=%d;AC%s=%d;DC%s=%d;JG%s=%d;BG%s;",
                  SelMot, Speed, SelMot, GailAcSpeed, SelMot, GailDcSpeed, SelMot, JogData, SelMot);

    Gali_Command(str, __FUNC__);
}
//-----------------------------------------------------------------------------
void TMyMotor::Gali_JogNSetup(int Speed)
{
    // AI(W906-P0-5) 20260920: 離線守衛 —— 值與退休前 mymotor.cpp 的樁**完全相同**，所以 Motor==NULL 的可觀察行為一格沒變。
    if(Motor==NULL) return;
    //******************************************************************************
    //  注意!! CheckIsSafeDoorOpen為Handler 安全門相關, 修改時要小心!!
    //******************************************************************************
    if(Motor->CheckIsSafeDoorOpen())                                            //Jimmychiu 20221013 safedoor判斷整合Function
    {
        return;
    }

    int JogData=-1000;
    JogData=abs(Speed);
    JogData=0-JogData;
    AnsiString SelMot=AnsiString("");
    if(Mot_Name!=MTestY1)
        JogData=-JogData;
    char str[256]="";
    SelMot=Gali_GetMOT(Mot_Name);
    W4G_sprintf(str, "SP%s=%d;AC%s=%d;DC%s=%d;JG%s=%d;BG%s;",
                  SelMot, 100, SelMot, GailAcSpeed, SelMot, GailDcSpeed, SelMot, JogData, SelMot);

    Gali_Command(str, __FUNC__);
}
//-----------------------------------------------------------------------------
void TMyMotor::Gali_JogNAndCount(int Speed,int Count)
{
    // AI(W906-P0-5) 20260920: 離線守衛 —— 值與退休前 mymotor.cpp 的樁**完全相同**，所以 Motor==NULL 的可觀察行為一格沒變。
    if(Motor==NULL) return;
    //******************************************************************************
    //  注意!! CheckIsSafeDoorOpen為Handler 安全門相關, 修改時要小心!!
    //******************************************************************************
    if(Motor->CheckIsSafeDoorOpen())                                            //Jimmychiu 20221013 safedoor判斷整合Function
    {
        return;
    }

    int JogData=-19990;
    JogData=-Count;
    AnsiString SelMot=AnsiString("");
    if(Mot_Name!=MTestY1)
        JogData=-JogData;
    char str[256]="";
    SelMot=Gali_GetMOT(Mot_Name);
    W4G_sprintf(str, "SP%s=%d;AC%s=%d;DC%s=%d;JG%s=%d;BG%s;",
                  SelMot, Speed, SelMot, GailAcSpeed, SelMot, GailDcSpeed, SelMot, JogData, SelMot);

    Gali_Command(str, __FUNC__);
}
//------------------------------------------------------------------------------
bool TMyMotor::Gali_MovePR(int Pos, int Speed)
{
    // AI(W906-P0-5) 20260920: 離線守衛 —— 值與退休前 mymotor.cpp 的樁**完全相同**，所以 Motor==NULL 的可觀察行為一格沒變。
    if(Motor==NULL) return false;
    //******************************************************************************
    //  注意!! CheckIsSafeDoorOpen為Handler 安全門相關, 修改時要小心!!
    //******************************************************************************
    if(Motor->CheckIsSafeDoorOpen())                                            //Jimmychiu 20221013 safedoor判斷整合Function
    {
        return false;
    }

    AnsiString SelMot=AnsiString(""), PRData=AnsiString("");
    GetRealPos(&Pos);
    if(Mot_Name!=MTestY1)
        PRData=-Pos;
    else
        PRData=Pos;

    char str[256]="";
    SelMot=Gali_GetMOT(Mot_Name);

    W4G_sprintf(str, "SP%s=%d;AC%s=%d;DC%s=%d;PR%s=%s;BG%s;",
                  SelMot, Speed, SelMot, GailAcSpeed, SelMot, GailDcSpeed, SelMot, PRData, SelMot);

    Gali_Command(str, __FUNC__);
    return true;
}
//------------------------------------------------------------------------------
bool Open_GaliCard()
{
    #ifdef SOFT_SIMULTE
    bGali_CardInstall=true;
    return true;
    #else
    char ControllerDesc[256]="";
    AnsiString  ControllerInfo=AnsiString(""), Str=AnsiString("");
    #ifndef SOFT_SIMULTE
        rc=DMCGetControllerDesc(1, ControllerDesc, sizeof(ControllerDesc));
    #endif
    ControllerInfo=ControllerDesc;
    if(rc!=0)
    {
        ShowErrorMessage("WAR1636", 0, MMSystem);
        return false;
    }
    #ifndef SOFT_SIMULTE
        rc=DMCOpen(1, hWnd, &hDmc);                                             //開卡
    #endif
    if(rc!=0)
    {
        SoftStop=true;
        SystemStart=false;
        Str=GetGalilErrString(rc, "Open_GaliCard");
        ShowErrorMessage("WAR1635", 0, MMSystem, 0, Str);
        fAllMotorHome=false;
        return false;
    }
    #ifndef SOFT_SIMULTE
    rc=DMCReset(hDmc);                                                          //kevin 20150909 add  RESET
    #endif
    if(rc!=0)
    {
        SoftStop=true;
        SystemStart=false;
        Str=GetGalilErrString(rc, "Open_GaliCard Reset");
        ShowErrorMessage("WAR1635", 0, MMSystem, 0, Str);
        fAllMotorHome=false;
        return false;
    }

    #ifndef SOFT_SIMULTE
        rc=DMCCommand(hDmc, "SH", szBuffer, sizeof(szBuffer));
    #endif
    if(rc!=0)
    {
        SoftStop=true;
        SystemStart=false;
        MyDBIProcessNew("Motion", "WAR2203", "Gail Command Err", AnsiString("SH"));
        Str=GetGalilErrString(rc, "Open_GaliCard ServoON");
        ShowErrorMessage("WAR1635", 0, MMSystem, 0, Str);
        fAllMotorHome=false;
    }
    bGali_CardInstall=true;

    return true;
    #endif
}
//------------------------------------------------------------------------------
// kevin 20150909 GAIL CLOSE CARD
//------------------------------------------------------------------------------
bool Close_GaliCard()
{
    #ifdef SOFT_SIMULTE
        return true;
    #else
        AnsiString Str=AnsiString("");
        DMCDiagnosticsOff(hDmc);                                                //kevin 20150909 關閉記錄 GAIL通訊
        rc=DMCClose(hDmc);
        if(rc!=0)
        {
            SoftStop=true;
            SystemStart=false;
            Str=GetGalilErrString(rc, "Close_GaliCard");
            ShowErrorMessage("WAR1635", 0, MMSystem, 0, Str);
            fAllMotorHome=false;
            return false;
        }
        return true;
    #endif
}
//------------------------------------------------------------------------------
long TMyMotor::Gali_ReadEncoderPos()
{
    // AI(W906-P0-5) 20260920: 離線守衛 —— 值與退休前 mymotor.cpp 的樁**完全相同**，所以 Motor==NULL 的可觀察行為一格沒變。
    if(Motor==NULL) return 0;
    AnsiString SelMot=AnsiString("");
    int iPos=0;
    if(Motor->Enable==false)
    {
       return Gali_ReadPos();
    }
    else
    {
        SelMot=Gali_GetMOT(Mot_Name);
        iPos=Gali_Command("TP"+SelMot, __FUNC__);
        if(Mot_Name==MTestY1 && (USE_INDEX_ARM_AXES==IndexArm_4_Axis))          //JimmyChiu 20220708 : add Index Arm Axis
        {
            EncoderPosition=-iPos*Motor->GearRatio;
        }
        else
        {
            if(USE_INDEX_ARM_AXES==IndexArm_3_Axis && Mot_Name==MTestY2)        //JimmyChiu 20220708 : add Index Arm Axis
            {
                return 0;
            }
            EncoderPosition=iPos*Motor->GearRatio;
        }
        return EncoderPosition;
    }
}
//------------------------------------------------------------------------------
bool TMyMotor::CheckPos(bool Front)
{
    // AI(W906-P0-5) 20260920: 離線守衛 —— 值與退休前 mymotor.cpp 的樁**完全相同**，所以 Motor==NULL 的可觀察行為一格沒變。
    if(Motor==NULL) return false;
    int iEncoderPos1=MOT[MTestY1].Gali_ReadEncoderPos();
    int iEncoderPos2=0;
    if(USE_INDEX_ARM_AXES==IndexArm_4_Axis)                 //JimmyChiu 20220708 : add Index Arm Axis
    {
        iEncoderPos2=MOT[MTestY2].Gali_ReadEncoderPos();
    }
    bool b1=false, b2=false, b3=false;
    if(Front)
    {
        b1=CheckArmPosArrival(iEncoderPos1, Prod.TestY1_Front, GaliPosOffSet);
        if(USE_INDEX_ARM_AXES==IndexArm_3_Axis)                                 //JimmyChiu 20220708 : add Index Arm Axis
            b2=true;
        else
            b2=CheckArmPosArrival(iEncoderPos2, Prod.TestY2_Middle, GaliPosOffSet);
    }
    else
    {
        b1=CheckArmPosArrival(iEncoderPos1, Prod.TestY1_Middle, GaliPosOffSet);
        if(USE_INDEX_ARM_AXES==IndexArm_3_Axis)                                 //JimmyChiu 20220708 : add Index Arm Axis
            b2=true;
        else
            b2=CheckArmPosArrival(iEncoderPos2, Prod.TestY2_Rear, GaliPosOffSet);
    }
    b3=(b1 && b2);
    return b3;
}
//------------------------------------------------------------------------------
bool TMyMotor::CheckYPos()                                                      //JerryYang 20180411 (jou) : Z軸移動前確認Y軸位置保護
{
    // AI(W906-P0-5) 20260920: 離線守衛 —— 值與退休前 mymotor.cpp 的樁**完全相同**，所以 Motor==NULL 的可觀察行為一格沒變。
    if(Motor==NULL) return false;
    static int iRetryCnt=0;
    int iEncoderPos1=MOT[MTestY1].Gali_ReadEncoderPos();
    int iEncoderPos2=0;
    if(USE_INDEX_ARM_AXES==IndexArm_4_Axis)                 //JimmyChiu 20220708 : add Index Arm Axis
    {
        iEncoderPos2=MOT[MTestY2].Gali_ReadEncoderPos();
    }
    bool b1=(CheckArmPosArrival(iEncoderPos1, Prod.TestY1_Front,  IniConfig.GaliPosRange) ||
             CheckArmPosArrival(iEncoderPos1, Prod.TestY1_Middle, IniConfig.GaliPosRange));
    bool b2=false;

    if(USE_INDEX_ARM_AXES==IndexArm_3_Axis)                                     //JimmyChiu 20220708 : add Index Arm Axis
        b2=true;
    else
        b2=(CheckArmPosArrival(iEncoderPos2, Prod.TestY2_Rear,   IniConfig.GaliPosRange) ||
            CheckArmPosArrival(iEncoderPos2, Prod.TestY2_Middle, IniConfig.GaliPosRange));

    if(b1 && b2)
    {
        iRetryCnt=0;
        bShowIndexMotorError=false;                                             //Isaac 20201012 : encoder和command/Teaching點比較，show alarm視窗變大
        return true;
    }
    else
    {
        #ifdef INDEX_PROTECT_TMOVE
        if(iRetryCnt>50)
        {
            if(bOverRangeDoTMode==false)
                RecordIndexPosition(3, 3);                                      //Gali_Two_ZAxis_Move
            bOverRangeDoTMode=true;                                             //觸發做Tmode
        }
        #else
        if(iRetryCnt<100)
        {
            iRetryCnt++;
            return false;
        }
        #endif
        iRetryCnt=0;
        bShowIndexMotorError=true;                                              //Isaac 20201012 : encoder和command/Teaching點比較，show alarm視窗變大
        ShowIndexMotorError(AnsiString("Gali_Two_ZAxis_Move"), true);
        return false;
    }
}
//-------------------------------------------------------------------------------
bool TMyMotor::CheckPos_nnMode(bool bOpen)
{
    // AI(W906-P0-5) 20260920: 離線守衛 —— 值與退休前 mymotor.cpp 的樁**完全相同**，所以 Motor==NULL 的可觀察行為一格沒變。
    if(Motor==NULL) return false;
    int iEncoderPos1=MOT[MTestY1].Gali_ReadEncoderPos();
    int iEncoderPos2=0;
    if(USE_INDEX_ARM_AXES==IndexArm_4_Axis)                 //JimmyChiu 20220708 : add Index Arm Axis
    {
        iEncoderPos2=MOT[MTestY2].Gali_ReadEncoderPos();
    }
    bool b1=false, b2=false, b3=false;
    if(bOpen)
    {
        b1=CheckArmPosArrival(iEncoderPos1, Prod.TestY1_Front,  IniConfig.GaliPosRange);
        if(USE_INDEX_ARM_AXES==IndexArm_3_Axis)                                 //JimmyChiu 20220708 : add Index Arm Axis
            b2=false;
        else
            b2=CheckArmPosArrival(iEncoderPos2, Prod.TestY2_Rear,   IniConfig.GaliPosRange);
    }
    else
    {
        b1=CheckArmPosArrival(iEncoderPos1, Prod.TestY1_Middle,  IniConfig.GaliPosRange);
        if(USE_INDEX_ARM_AXES==IndexArm_3_Axis)                                 //JimmyChiu 20220708 : add Index Arm Axis
            b2=false;
        else
            b2=CheckArmPosArrival(iEncoderPos2, Prod.TestY2_Middle,  IniConfig.GaliPosRange);
    }
    b3=(b1 && b2);
    return b3;
}
//-------------------------------------------------------------------------------
// 用於 TestY 及 TestZ1 TestZ2 之 safe point so ,randge is small
bool TMyMotor::Gali_ReadEncoderInRandge(long checkpos)
{
    // AI(W906-P0-5) 20260920: 離線守衛 —— 值與退休前 mymotor.cpp 的樁**完全相同**，所以 Motor==NULL 的可觀察行為一格沒變。
    if(Motor==NULL) return false;
    if(Motor->Enable==false)
        return true;

    long pos=0, check=0, pos1=0;
    pos=Gali_ReadEncoderPos();
    if(pos<0L)
        pos=0L-pos;
    pos1=checkpos;
    if(pos1<0L)
        pos1=0-pos1;
    check=pos-pos1;
    if(check<0L)
        check=0L-check;
    GaliAxisAlarm[0]=false;
    GaliAxisAlarm[1]=false;
    GaliAxisAlarm[2]=false;
    GaliAxisAlarm[3]=false;
    if(check>=GaliPosOffSet)
    {
        if(Mot_Name==MTestY1)
            GaliAxisAlarm[0]=true;
        else if(Mot_Name==MTestZ1)
            GaliAxisAlarm[1]=true;
        else if(Mot_Name==MTestZ2)
            GaliAxisAlarm[2]=true;
        else
            GaliAxisAlarm[3]=true;

        return false;
    }
    else
    {
        return true;
    }
}
//-------------------------------------------------------------------------------
bool TMyMotor::Gali_ReadEncoderBelowCheckHeight(long checkpos)
{
    // AI(W906-P0-5) 20260920: 離線守衛 —— 值與退休前 mymotor.cpp 的樁**完全相同**，所以 Motor==NULL 的可觀察行為一格沒變。
    if(Motor==NULL) return false;
    #ifdef SOFT_SIMULTE
        return false;
    #else
        long pos, pos1;
        if(Motor->Enable==false)
            return false;

        pos=checkpos-GaliPosOffSet;
        pos1=Gali_ReadEncoderPos();
        if(pos1<pos)
            return true;
        else
            return false;
    #endif
}
//-------------------------------------------------------------------------------
bool TMyMotor::Gali_ReadEncoderInRandgeMinLimit(long checkpos)                  //檢查Y軸command position和Encoder值
{
    // AI(W906-P0-5) 20260920: 離線守衛 —— 值與退休前 mymotor.cpp 的樁**完全相同**，所以 Motor==NULL 的可觀察行為一格沒變。
    if(Motor==NULL) return false;
    if(Motor->Enable==false)
        return true;

    long pos=0, check=0, pos1=0;
    pos=Gali_ReadEncoderPos();
    pos=abs(pos);
    pos1=checkpos;
    pos1=abs(pos1);
    check=pos-pos1;
    check=abs(check);

    //Isaac 20201012 : 計算Encoder和commandpos/Teaching的差值
    //=>
    if(Mot_Name==MTestY1)//Arm1
    {
        if(check>iMaxCommandY1)         //找出最大值
        {
            iMaxCommandY1=check;
        }
        else if(check<iMinCommandY1)    //找出最小值
        {
            iMinCommandY1=check;
        }
    }
    else if(Mot_Name==MTestY2)//Arm2
    {
        if(check>iMaxCommandY2)      // iMaxCommand=0,iMinCommand=0,iMaxTeach=0,iMinTeach=0;
        {
            iMaxCommandY2=check;        //找出最大值
        }
        else if(check<iMinCommandY2)    //找出最小值
        {
            iMinCommandY2=check;
        }
    }
    //<=
    //Isaac 20201012 : 計算Encoder和commandpos/Teaching的差值

    if(check>=IniConfig.GaliPosRange)
    {
        bShowIndexMotorError=true;      //Isaac 20201012 : encoder和command/Teaching點比較，show alarm視窗變大
        return false;
    }
    else
    {
        bShowIndexMotorError=false;     //Isaac 20201012 : encoder和command/Teaching點比較，show alarm視窗變大
        return true;
    }
}
//-------------------------------------------------------------------------------
bool TMyMotor::Gali_ReadEncoderInRandgeNoWait(long checkpos)
{
    // AI(W906-P0-5) 20260920: 離線守衛 —— 值與退休前 mymotor.cpp 的樁**完全相同**，所以 Motor==NULL 的可觀察行為一格沒變。
    if(Motor==NULL) return false;
    if(Motor->Enable==false)
        return true;

    long pos=0, check=0, pos1=0;
    pos=Gali_ReadEncoderPos();
    if(pos<0L)
        pos=0L-pos;
    pos1=checkpos;
    if(pos1<0L)
        pos1=0-pos1;
    check=pos-pos1;
    if(check<0L)
        check=0L-check;
    GaliAxisAlarm[0]=false;
    GaliAxisAlarm[1]=false;
    GaliAxisAlarm[2]=false;
    GaliAxisAlarm[3]=false;
    if(check>=(GaliPosOffSet*2))
    {
        if(Mot_Name==MTestY1)
            GaliAxisAlarm[0]=true;
        else if(Mot_Name==MTestZ1)
            GaliAxisAlarm[1]=true;
        else if(Mot_Name==MTestZ2)
            GaliAxisAlarm[2]=true;
        else
            GaliAxisAlarm[3]=true;
        return false;
    }
    else
    {
        return true;
    }
}
//-------------------------------------------------------------------------------
// 用於 TestZ1 TestZ2 之 test point ,so randge is big
bool TMyMotor::Gali_ReadEncoderMaxRandge(long checkpos)
{
    // AI(W906-P0-5) 20260920: 離線守衛 —— 值與退休前 mymotor.cpp 的樁**完全相同**，所以 Motor==NULL 的可觀察行為一格沒變。
    if(Motor==NULL) return false;
    if(Motor->Enable==false)
        return true;

    long pos=0, check=0, pos1=0;
    pos=Gali_ReadEncoderPos();
    if(pos<0L)
        pos=0L-pos;
    pos1=checkpos;
    if(pos1<0L)
        pos1=0-pos1;
    check=pos-pos1;
    GaliAxisAlarm[0]=false;
    GaliAxisAlarm[1]=false;
    GaliAxisAlarm[2]=false;
    GaliAxisAlarm[3]=false;
    if(check<0L)
        check=0L-check;
    if(check>=(GaliPosOffSet*2))
    {
        if(Mot_Name==MTestY1)
            GaliAxisAlarm[0]=true;
        else if(Mot_Name==MTestZ1)
            GaliAxisAlarm[1]=true;
        else if(Mot_Name==MTestZ2)
            GaliAxisAlarm[2]=true;
        else
            GaliAxisAlarm[3]=true;
        return false;
    }
    else
    {
        return true;
    }
}
//------------------------------------------------------------------------------
bool TMyMotor::Gali_ReadEncoderOver(long checkpos)
{
    // AI(W906-P0-5) 20260920: 離線守衛 —— 值與退休前 mymotor.cpp 的樁**完全相同**，所以 Motor==NULL 的可觀察行為一格沒變。
    if(Motor==NULL) return false;
    long pos=Gali_ReadEncoderPos();
    if(pos>checkpos)
        return false;
    else
        return true;
}
//------------------------------------------------------------------------------
// 1112 Start
bool TMyMotor::ISZ1Up_Z2Down()
{
    // AI(W906-P0-5) 20260920: 離線守衛 —— 值與退休前 mymotor.cpp 的樁**完全相同**，所以 Motor==NULL 的可觀察行為一格沒變。
    if(Motor==NULL) return false;
    //*Index更換Offset會hang up?   (atester.cpp #3917的判斷會異常) jou 2010-04-13
    if(MOT[MTestZ1].Gali_ReadEncoderInRandge(Prod.TestZ1_Safe) &&
       MOT[MTestZ2].Gali_ReadEncoderMaxRandge(iBackUpZ2DownPosition))
        return true;
    else
        return false;
}
//------------------------------------------------------------------------------
bool TMyMotor::ISZ1Up_Z2DownNoWait()
{
    // AI(W906-P0-5) 20260920: 離線守衛 —— 值與退休前 mymotor.cpp 的樁**完全相同**，所以 Motor==NULL 的可觀察行為一格沒變。
    if(Motor==NULL) return false;
    if(Prod.TestZ2_Drop_Offset!=0)  //DropContact or DirectContactModeDiffentSpeed
    {
        //Ifor 20191022 : 避免Arm2不動作時更新到TestZ2_Test位置導致發出四軸異常
        if(IniConfig.bD58UseArm1PickPlaceArm2Test==true &&
           TestIF_File.bArm1PickPlaceArm2Test==true     &&
           bCheckGiveWay==true                          )
        {
            if(MOT[MTestZ1].Gali_ReadEncoderInRandgeNoWait(Prod.TestZ1_Safe) &&
               MOT[MTestZ2].Gali_ReadEncoderMaxRandge(Prod.TestZ2_Safe))
                return true;
            else
                return false;
        }
        else
        {
            if(MOT[MTestZ1].Gali_ReadEncoderInRandgeNoWait(Prod.TestZ1_Safe) &&
               MOT[MTestZ2].Gali_ReadEncoderMaxRandge(Prod.TestZ2_Test-Prod.TestZ2_Drop_Offset))
                return true;
            else
                return false;
        }
    }
    else
    {
        //Ifor 20191022 : 避免Arm2不動作時更新到TestZ2_Test位置導致發出四軸異常
        if(IniConfig.bD58UseArm1PickPlaceArm2Test==true &&
           TestIF_File.bArm1PickPlaceArm2Test==true     &&
           bCheckGiveWay==true                          )
        {
            if(MOT[MTestZ1].Gali_ReadEncoderInRandgeNoWait(Prod.TestZ1_Safe) &&
               MOT[MTestZ2].Gali_ReadEncoderMaxRandge(Prod.TestZ2_Safe))
                return true;
            else
                return false;
        }
        else
        {
            if(MOT[MTestZ1].Gali_ReadEncoderInRandgeNoWait(Prod.TestZ1_Safe) &&
               MOT[MTestZ2].Gali_ReadEncoderMaxRandge(Prod.TestZ2_Test))
                return true;
            else
                return false;
        }
    }
}
//------------------------------------------------------------------------------
bool TMyMotor::ISZ1Down_Z2Up()
{
    // AI(W906-P0-5) 20260920: 離線守衛 —— 值與退休前 mymotor.cpp 的樁**完全相同**，所以 Motor==NULL 的可觀察行為一格沒變。
    if(Motor==NULL) return false;
    //*Index更換Offset會hang up?   (atester.cpp #3917的判斷會異常) jou 2010-04-13
    if(MOT[MTestZ2].Gali_ReadEncoderInRandge(Prod.TestZ2_Safe) &&
       MOT[MTestZ1].Gali_ReadEncoderMaxRandge(iBackUpZ1DownPosition))
        return true;
    else
        return false;
}
//------------------------------------------------------------------------------
bool TMyMotor::ISZ1Down_Z2UpNoWait()
{
    // AI(W906-P0-5) 20260920: 離線守衛 —— 值與退休前 mymotor.cpp 的樁**完全相同**，所以 Motor==NULL 的可觀察行為一格沒變。
    if(Motor==NULL) return false;
    if(Prod.TestZ1_Drop_Offset!=0) //DropContact or DirectContactModeDiffentSpeed
    {
        if(MOT[MTestZ2].Gali_ReadEncoderInRandgeNoWait(Prod.TestZ2_Safe) &&
            MOT[MTestZ1].Gali_ReadEncoderMaxRandge(Prod.TestZ1_Test-Prod.TestZ1_Drop_Offset))
            return true;
        else
            return false;
    }
    else
    {
        if(MOT[MTestZ2].Gali_ReadEncoderInRandgeNoWait(Prod.TestZ2_Safe) &&
            MOT[MTestZ1].Gali_ReadEncoderMaxRandge(Prod.TestZ1_Test ))
            return true;
        else
            return false;
    }
}
//------------------------------------------------------------------------------
bool TMyMotor::ISNormal()
{
    // AI(W906-P0-5) 20260920: 離線守衛 —— 值與退休前 mymotor.cpp 的樁**完全相同**，所以 Motor==NULL 的可觀察行為一格沒變。
    if(Motor==NULL) return true;
    if(MOT[MTestZ1].Gali_ReadEncoderInRandge(Prod.TestZ1_Safe) &&
        MOT[MTestZ2].Gali_ReadEncoderInRandge(Prod.TestZ2_Safe))
        return true;
    else
        return false;
}
//------------------------------------------------------------------------------
// 1112 end
//------------------------------------------------------------------------------
bool TMyMotor::Gali_SingalHome(bool IndexZFirstHome)
{
    // AI(W906-P0-5) 20260920: 離線守衛 —— 值與退休前 mymotor.cpp 的樁**完全相同**，所以 Motor==NULL 的可觀察行為一格沒變。
    if(Motor==NULL) return false;
    long lnumber=0;
    bool checkflag=false;
    int  TestZSafePos=Prod.TestZ1_Safe;                                         //kevin 20160905
    int iPos=0;

    //******************************************************************************
    //  注意!! CheckIsSafeDoorOpen為Handler 安全門相關, 修改時要小心!!
    //******************************************************************************
    if(Motor->CheckIsSafeDoorOpen())                                            //Jimmychiu 20221013 safedoor判斷整合Function
    {
        return false;
    }

    if(Motor->Enable==false)                                                    //Jimmychiu 20230201 add
        return true;

    AnsiString tempS="",tempS1="";
    int tempMOT=0;                                                              //kevin 20170607 motor number
    AnsiString sTime=FormatDateTime("hh:nn:ss", Now()), sData="";                  //kevin 20170515 add

    if(Alias=="MTestY1")
    {
        tempS="X";
        tempS1="x";
        tempMOT=MTestY1;                                                        //kevin 20170515 add motor number
    }
    else if(Alias=="MTestZ1")
    {
        tempS="Y";
        tempS1="y";
        tempMOT=MTestZ1;                                                        //kevin 20170515 add motor number
    }
    else if(Alias=="MTestZ2")
    {
        tempS="Z";
        tempS1="z";
        tempMOT=MTestZ2;                                                        //kevin 20170515 add motor number
    }
    else
    {
        tempS="W";
        tempS1="w";
        tempMOT=MTestY2;                                                        //kevin 20170515 add motor number
    }

    if(Motor->Enable==false)
    {
        bScanFlag=false;
        MovFlag=false;
        GaliSofDelayCount=0;
        HomeFlag=1;
        return true;
    }
    Gali_ScanMotStatus();  if(W906_GaliRouteOwns(Mot_Name)) return W906_GaliRoutedSingalHome(*this);   //AI(W906-INDEXZ-1203) 20260929: routed axis: golden preamble above, then the drive's own home with golden's task numbers/flags (EOF); the Galil HM/FI sensor dance below is Galil-only
    int &Task=iGali_SingalHomeTask;
    switch(Task)
    {
        case 1:
            if(Gali_Command("MG_BG"+tempS1, "Gali_SingalHome_1")==0)            //馬達目前是否移動? 0:移動完成
            {
                Gali_Command("DP"+tempS+"=0;DE"+tempS+"=0;", "Gali_SingalHome_1");         //改變軸卡 command pos and Encoder pos set to 0
                Task=5;
            }
            break;
        case 5:
            if(Gali_Command("MG_BG"+tempS1, "Gali_SingalHome_5")==0)            //馬達目前是否移動 0:移動完成
            {
                Task=8;
            }
            break;
        case 8:
            Gali_ScanMotStatusTIMO();                                           //取得目前I/O狀態
            if(Led[iServoOn]==true)                                             //Servo On
            {
                Task=10;
            }
            else                                                                //Servo Off
            {
                Task=1;
                return false;
            }
            break;
        case 10:
            if(Led[iHomeLed] && (Alias=="MTestY1" || Alias=="MTestY2"))         //20140923 wei  Y軸極限回Home
            {
                Task=20;
            }
            else
            {
                Task=30;
            }
            break;
        case 20:
            if(Motor->HomeDirection)
                Gali_JogN(Motor->PHomeHighSpeed);
            else
                Gali_JogP(Motor->PHomeHighSpeed);
            if(Led[iHomeLed]==false)
            {
                Gali_Command("ST"+tempS, "Gali_SingalHome_20");                 //ChungHung 20141016 fix 每次回Home後 位置可能不在Home點
                Task=30;
            }
            break;
        case 30:
            if(Led[iHomeLed])
            {
                Gali_Command("ST"+tempS, "Gali_SingalHome_30");                 //ChungHung 20141016 fix 每次回Home後 位置可能不在Home點
                DelayCT.Set0_1SecAndOn(5);
                Task=100;
            }
            else
            {
                Task=50;
            }
            break;
        case 50:
            if(Motor->HomeDirection)
                Gali_JogP(Motor->PHomeHighSpeed);
            else
                Gali_JogN(Motor->PHomeHighSpeed);
            Task=30;                                                            //20140923 wei  Z軸極限回Home
            break;
        case 100:
            if(DelayCT.Off())
            {
                if(Motor->HomeDirection)
                    Gali_JogN(Motor->PHomeLowSpeed);
                else
                    Gali_JogP(Motor->PHomeLowSpeed);
                Task=150;
            }
            break;
        case 150:
            if(Led[iHomeLed])
            {
                Task=100;
                iPos=MOT[tempMOT].Gali_ReadEncoderPos();
                if((tempMOT==MTestZ1 || tempMOT==MTestZ2) &&
                   iPos<-1000)                                                  //Steven 20191213 : Index Z sensor故障保護
                {
                    Gali_Command("ST"+tempS, "Gali_SingalHome_150");            //ChungHung 20141016 fix 每次回Home後 位置可能不在Home點
                    DelayCT.Set0_1SecAndOn(5);
                    Task=160;
                }
            }
            else
            {
                Gali_Command("ST"+tempS, "Gali_SingalHome_150");                //ChungHung 20141016 fix 每次回Home後 位置可能不在Home點
                DelayCT.Set0_1SecAndOn(5);
                Task=200;
            }
            break;
        //Steven 20191213 : Index Z sensor故障保護
        //==>
        case 160:
            if(DelayCT.Off())
            {
                if(Gali_MotMove(0, Motor->PHomeLowSpeed, "Gali_SingalHome_160"))
                {
                    Task=170;
                }
            }
            break;
        case 170:
            if(tempMOT==MTestZ1)
                ShowErrorMessage("WAR03502", 0, MTestZ1, 0, "Gali_SingalHome_170");
            else
                ShowErrorMessage("WAR03503", 0, MTestZ2, 0, "Gali_SingalHome_170");
            Task=1;
            break;
        //<==
        //Steven 20191213 : Index Z sensor故障保護
        case 200:
            if(DelayCT.Off())
            {
                Gali_MotHome(tempS);
                MovFlag=true;
                Task=250;                                                       //Ifor 20170817 (wei) Mark Z PHASE 移置別處處理
            }
            break;
        case 250:                                                               //jimmychiu 20220815 avoid repeat sent home command
            if(Gali_Command("MG_BG"+tempS1, "Gali_SingalHome_250")==1)
            {
                Task=300;
            }
            break;
        case 300:
            lnumber=Gali_Command("MG_SC"+tempS1, "Gali_SingalHome_300");
            if(lnumber==10)                                                     //ChungHung 20141016 fix 每次回Home後 位置可能不在Home點
            {
                Motor->LastHomePos=Gali_ReadEncoderPos();                       //讀取目前Encoder
                Gali_MotHomeFindZ(tempS);                                       //ChungHung 20141016 add find Z相
                DelayCT.SetMSAndOn(100);                                        //ChungHung 20141016 10--->100
                Task=350;
            }
            break;
        case 350:                                                               //ChungHung 20141016 add find Z相
            if(DelayCT.Off())
            {
                if(Gali_Command("MG_SC"+tempS1, "Gali_SingalHome_350")==10)     //ChungHung 20141016 fix 每次回Home後 位置可能不在Home點
                {
                    DelayCT.SetMSAndOn(100);                                    //ChungHung 20141016 10--->100
                    Task=400;
                }
            }
            break;
        case 400:
            if(DelayCT.Off())
            {
                Motor->LastHomePos=MOT[tempMOT].Gali_ReadEncoderPos();          //讀取目前Encoder
                Gali_Command("DP"+tempS+"=0;DE"+tempS+"=0;", "Gali_SingalHome_400");
                Motor->LastHomePos=Gali_ReadEncoderPos();                       //讀取目前Encoder
                bScanFlag=false;
                MovFlag=false;
                GaliSofDelayCount=0;
                HomeFlag=1;
                if(Alias=="MTestZ1" || Alias=="MTestZ2")                        //Richard 20221206 : 開會討論後決定先回復為原先模式
                {
                    if(Alias=="MTestZ1")
                        checkflag=CheckTestZ1("Gali_SingalHome400");
                    if(Alias=="MTestZ2")
                        checkflag=CheckTestZ2("Gali_SingalHome400");
                    if(checkflag)
                        Task=450;
                    else
                        Task=500;
                }
                else
                {
                    Task=500;
                }
            }
            break;
        case 450:
            TestZSafePos=10;
            if(Alias=="MTestZ1" || Alias=="MTestZ2")
            {
                TestZSafePos=Prod.TestZ1_Safe;                                  //kevin 20160905
            }

            if(Gali_MotMove(TestZSafePos, Motor->PHomeHighSpeed, "Gali_SingalHome_450"))
            {
                Task=500;
            }
            break;
        case 500:
            Task=900;
            if((CUSTOMER_CODE==CC_ASE_KaohSiung ||
                CosFunction.bHiSiliconFunction==true) &&
               IniConfig.bD63CheckIndexZHomeToZPhaseDistanceRange==true &&
               (Alias=="MTestZ1" || Alias=="MTestZ2"))                          //kevin 20170605 (Steven) add
            {
                MovFlag=false;
                GaliSofDelayCount=0;
                HomeFlag=1;
            }
            return true;
    }
    return false;
}
//-----------------------------------------------------------------------------
bool TMyMotor::Gali_FindZPhase()
{
    // AI(W906-P0-5) 20260920: 離線守衛 —— 值與退休前 mymotor.cpp 的樁**完全相同**，所以 Motor==NULL 的可觀察行為一格沒變。
    if(Motor==NULL) return false;
    //******************************************************************************
    //  注意!! CheckIsSafeDoorOpen為Handler 安全門相關, 修改時要小心!!
    //******************************************************************************
    if(Motor->CheckIsSafeDoorOpen())                                            //Jimmychiu 20221013 safedoor判斷整合Function
    {
        return false;
    }

    bool checkflag=false;
    int  TestZSafePos=Prod.TestZ1_Safe;

    static bool bCheckOverRangeZ1=false;                                        //kevin 20170515 add Jeffrey 20170414 Check IndexZ Home to Z Phase Distance Range
    static bool bCheckOverRangeZ2=false;                                        //kevin 20170515 add Jeffrey 20170414 Check IndexZ Home to Z Phase Distance Range
    static int iZ1DistanceRef=0;                                                //kevin 20170515 add Jeffrey 20170414 Check IndexZ Home to Z Phase Distance Range
    static int iZ1DistanceReal=0;                                               //kevin 20170515 add Jeffrey 20170414 Check IndexZ Home to Z Phase Distance Range
    static int iZ2DistanceRef=0;                                                //kevin 20170515 add Jeffrey 20170414 Check IndexZ Home to Z Phase Distance Range
    static int iZ2DistanceReal=0;                                               //kevin 20170515 add Jeffrey 20170414 Check IndexZ Home to Z Phase Distance Range
    int iZData=0,ret=0;                                                         //kevin 20170511 add Jeffrey 20170414 Check IndexZ Home to Z Phase Distance Range

    //Isaac 20201110 : Index Y find motor phase
    //=>
    int iYData=0;
    static bool bCheckOverRangeY1=false;
    static bool bCheckOverRangeY2=false;
    static int iY1DistanceRef=0;
    static int iY1DistanceReal=0;
    static int iY2DistanceRef=0;
    static int iY2DistanceReal=0;
    //<=
    //Isaac 20201110 : Index Y find motor phase

    AnsiString tempS="", tempS1="";
    int tempMOT=0;                                                              //kevin 20170607 motor number
    AnsiString sTime=FormatDateTime("hh:nn:ss", Now()), sData="";                  //kevin 20170515 add
    int iwhichIndexAxis=-1;                                                     //Isaac 20201110 : Index Y find motor phase，x,y,z,w == Y1,Z1,Z2,Y2 ==0,1,2,3

    if(Alias=="MTestY1")                                                        //Isaac 20201110 : Index Y find motor phase
    {
        tempS="X";
        tempS1="x";
        tempMOT=MTestY1;
        iwhichIndexAxis=0;
    }
    else if(Alias=="MTestZ1")
    {
        tempS="Y";
        tempS1="y";
        tempMOT=MTestZ1;                                                        //kevin 20170515 add motor number
        iwhichIndexAxis=1;
    }
    else if(Alias=="MTestZ2")
    {
        tempS="Z";
        tempS1="z";
        tempMOT=MTestZ2;                                                        //kevin 20170515 add motor number
        iwhichIndexAxis=2;
    }
    else// if(Alias=="MTestY2")                                                 //Isaac 20201110 : Index Y find motor phase
    {
        tempS="W";
        tempS1="w";
        tempMOT=MTestY2;
        iwhichIndexAxis=3;
    }

    if(Motor->Enable==false)
    {
        bScanFlag=false;
        MovFlag=false;
        GaliSofDelayCount=0;
        HomeFlag=1;
        return true;
    }  if(W906_GaliRouteOwns(Mot_Name)) return W906_GaliRoutedFindZPhase(*this);   //AI(W906-INDEXZ-1203) 20260929: D63 Z-phase search is Galil-only: the routed axis refuses (false, never a fake success; HT9050 keeps D63=0)
    Gali_ScanMotStatus();
    //int &Task=iGali_FindZPhaseTask;
    int &Task=iGali_FindZPhaseTask[iwhichIndexAxis];                            //Isaac 20201110 : Index Y find motor phase
    switch(Task)
    {
        case 1:
            if(Gali_Command("MG_BG"+tempS1, __FUNC__)==0)                       //馬達目前是否移動? 0:移動完成
            {
                iZ1DistanceRef=ReadIniData(asGeneralPath, "IndexDriver", "Index_Z1_Home_Position", 120);
                iZ2DistanceRef=ReadIniData(asGeneralPath, "IndexDriver", "Index_Z2_Home_Position", 120);
                iY1DistanceRef=ReadIniData(asGeneralPath, "IndexDriver", "Index_Y1_Home_Position", 20);
                iY2DistanceRef=ReadIniData(asGeneralPath, "IndexDriver", "Index_Y2_Home_Position", 20);

                Task=300;
            }
            break;
        case 300:
            Motor->LastHomePos=Gali_ReadEncoderPos();                           //讀取目前Encoder
            if(Alias=="MTestZ1" || Alias=="MTestZ2")                            //kevin 20170605
            {
                MovFlag=false;                                                  //Ifor 20170818 尋找Z相需設定false
                Gali_MotHomeFindZ(tempS);                                       //kevin 20170606 馬達移到離開HOME 找Z相 ChungHung 20141016 add find Z相
            }
            else if(Alias=="MTestY1" || Alias=="MTestY2")                       //Isaac 20201110 : Index Y find motor phase
            {
                MovFlag=false;
                Gali_MotHomeFindZ(tempS);
            }
            DelayCT.SetMSAndOn(100);                                            //ChungHung 20141016 10--->100
            Task=350;
            break;
        case 350:                                                               //ChungHung 20141016 add find Z相
            if(DelayCT.Off())
            {
                if(Gali_Command("MG_SC"+tempS1, "Gali_FindZPhase_1")==10)                  //ChungHung 20141016 fix 每次回Home後 位置可能不在Home點
                {
                    DelayCT.SetMSAndOn(100);                                    //ChungHung 20141016 10--->100
                    Task=400;
                }
            }
            break;
        case 400:
            if(DelayCT.Off())
            {
                Motor->LastHomePos=MOT[tempMOT].Gali_ReadEncoderPos();          //讀取目前Encoder
                bScanFlag=false;
                MovFlag=false;
                Task=410;
            }
        //==========================================kevin 20170515 ZPhase home==============>
        case 410:
            if(tempMOT==MTestZ1)
            {
                iIndexZ1HomeToZPhaseDistanceFirst=Gali_ReadEncoderPos();
                iIndexZ1HomeToZPosFirst=MOT[MTestZ1].Gali_ReadPos();            //kevin 20170605 read pos
            }
            else if(tempMOT==MTestZ2)
            {
                iIndexZ2HomeToZPhaseDistanceFirst=Gali_ReadEncoderPos();
                iIndexZ2HomeToZPosFirst=MOT[MTestZ2].Gali_ReadPos();            //kevin 20170605 read pos
            }
            else if(tempMOT==MTestY1)                                           //Isaac 20201110 : Index Y find motor phase
            {
                iIndexY1HomeToYPhaseDistanceFirst=Gali_ReadEncoderPos();
                iIndexY1HomeToYPosFirst=MOT[MTestY1].Gali_ReadPos();            //kevin 20170605 read pos
            }
            else if(tempMOT==MTestY2)                                           //Isaac 20201110 : Index Y find motor phase
            {
                if(USE_INDEX_ARM_AXES==IndexArm_4_Axis)                 //JimmyChiu 20220708 : add Index Arm Axis
                {
                    iIndexY2HomeToYPhaseDistanceFirst=Gali_ReadEncoderPos();
                    iIndexY2HomeToYPosFirst=MOT[MTestY2].Gali_ReadPos();            //kevin 20170605 read pos
                }
            }
            MovFlag=false;
            Task=420;
           break;
        case 420:                                                               //Jeffrey 20170414 Check IndexZ Home to Z Phase Distance Range
            Gali_MotHome(tempS);                                                //kevin 20170605 add 找 HOME 點
            if(Gali_Command("MG_BG"+tempS1, __FUNC__)==1)                       //是否移動?
            {
                Task=421;
            }
            break;
       case 421:
            if(Gali_Command("MG_SC"+tempS1, "Gali_FindZPhase_421")==10)         //ChungHung 20150831 add for home process SC==9 : Stopped after homing(HM)
            {
                DelayCT.Set0_1SecAndOn(10);
                Task=430;
            }
            break;
        case 430:
            if(DelayCT.Off()==true)
            {
                if(Gali_Command("MG_BG"+tempS1, "Gali_FindZPhase_430")==0)      //移動完成
                {
                    MovFlag=true;
                    Task=436;
                }
            }
            break;
        case 436:                                                               //Jeffrey 20170414 Check IndexZ Home to Z Phase Distance Range
            if(tempMOT==MTestZ1)
            {
                iIndexZ1HomeToZPhaseDistanceSecond=MOT[MTestZ1].Gali_ReadEncoderPos();
                iIndexZ1HomeToZPosSecond=MOT[MTestZ1].Gali_ReadPos();           //kevin 20170605 read pos
            }
            else if(tempMOT==MTestZ2)
            {
                iIndexZ2HomeToZPhaseDistanceSecond=MOT[MTestZ2].Gali_ReadEncoderPos();
                iIndexZ2HomeToZPosSecond=MOT[MTestZ2].Gali_ReadPos();           //kevin 20170605 read pos
            }
            else if(tempMOT==MTestY1)                                           //Isaac 20201110 : Index Y find motor phase
            {
                iIndexY1HomeToYPhaseDistanceSecond=MOT[MTestY1].Gali_ReadEncoderPos();
                iIndexY1HomeToYPosSecond=MOT[MTestY1].Gali_ReadPos();           //kevin 20170605 read pos
            }
            else if(tempMOT==MTestY2)                                           //Isaac 20201110 : Index Y find motor phase
            {
                if(USE_INDEX_ARM_AXES==IndexArm_4_Axis)                         //JimmyChiu 20220708 : add Index Arm Axis
                {
                    iIndexY2HomeToYPhaseDistanceSecond=MOT[MTestY2].Gali_ReadEncoderPos();
                    iIndexY2HomeToYPosSecond=MOT[MTestY2].Gali_ReadPos();       //kevin 20170605 read pos
                }
            }
            MovFlag=false;                                                      //Z motor move
            GaliSofDelayCount=0;
            Task=446;
            break;
        case 446:                                                               //Jeffrey 20170414 Check IndexZ Home to Z Phase Distance Range
            if(IniConfig.iD63IndexZHomeToZPhaseRange<0)
            {
                IniConfig.iD63IndexZHomeToZPhaseRange=-IniConfig.iD63IndexZHomeToZPhaseRange;
            }

            if(tempMOT==MTestZ1)
            {
                if(bCheckOverRangeZ1==false)
                {
                    iZ1DistanceRef=abs(iIndexZ1HomeToZPhaseDistanceFirst-iIndexZ1HomeToZPhaseDistanceSecond);
                    if(CheckIniData(asGeneralPath, "IndexDriver", "Index_Z1_Home_Position")==false)
                    {
                        WriteIniData(asGeneralPath, "IndexDriver", "Index_Z1_Home_Position", iZ1DistanceRef);//  iZ1DistanceRef );
                        sData.sprintf("%s Finding phase no parameter, add Index_Z1_Home_Position=%d", sTime, iZ1DistanceRef);
                        SaveFile(asIndexZphasePath, sData);
                    }
                    else
                    {
                        iZ1DistanceRef=ReadIniData(asGeneralPath, "IndexDriver", "Index_Z1_Home_Position", 120);
                    }
                    bCheckOverRangeZ1=true;
                }
                iZ1DistanceReal=abs(iIndexZ1HomeToZPhaseDistanceFirst-iIndexZ1HomeToZPhaseDistanceSecond);
                iZData=abs(iZ1DistanceReal-iZ1DistanceRef);
                sData.sprintf("%s Arm 1 Zphase:First: %d ,Second: %d ,Real %d,ref %d,Zhase:%d", sTime, iIndexZ1HomeToZPhaseDistanceFirst, iIndexZ1HomeToZPhaseDistanceSecond, iZ1DistanceReal, iZ1DistanceRef, iZData); //kevin 20170511 add log
                SaveFile(asIndexZphasePath, sData);                             //kevin 20170511
                RecordProcess(sData);
                sData.sprintf("%s Arm 1 ZRead Pos First: %d ,Second: %d", sTime, iIndexZ1HomeToZPosFirst, iIndexZ1HomeToZPosSecond); //kevin 20170511 add log
                SaveFile(asIndexZphasePath, sData);                             //kevin 20170511
                if(iZData>(IniConfig.iD63IndexZHomeToZPhaseRange))              //50->0
                {
                    ret=ShowErrorMessage("WAR0307", K_RETRY|K_SKIP, MMSystem);
                    if(ret==K_SKIP)
                    {
                        if(CUSTOMER_CODE==CC_ASE_KaohSiung)
                        {
                            iZ1DistanceRef=iZ1DistanceReal;
                            WriteIniData(asGeneralPath, "IndexDriver", "Index_Z1_Home_Position", iZ1DistanceRef);
                            sData.sprintf("%s Skip press,and save Index_Z1_Home_Position as %d", sTime, iZ1DistanceRef);
                            SaveFile(asIndexZphasePath, sData);

                            iZ1DistanceRef=ReadIniData(asGeneralPath, "IndexDriver", "Index_Z1_Home_Position", 120);
                        }
                        else
                        {
                            bZ1ModifyDistanceRef=true;                          //Ifor 20190530 : add Z Phase 可SKIP 但需重新contact height
                        }
                        Task=447;
                    }
                    else if(ret==K_RETRY)
                    {
                        iZ1DistanceRef=ReadIniData(asGeneralPath, "IndexDriver", "Index_Z1_Home_Position", 120);
                        Task=1;
                        break;
                    }
                }
                else
                {
                    Task=447;
                    bZ1ModifyDistanceRef=false;
                }

                if(bZ1ModifyDistanceRef==true)                                  //Ifor 20190530 : add Z Phase 可SKIP 但需重新contact height
                {
                    iZ1ModifyDistanceRef=iZ1DistanceReal;
                }
                else
                {
                    iZ1ModifyDistanceRef=iZ1DistanceRef;
                }

                iIndexZ1HomeToZPhaseDistanceFirst=0;
                iIndexZ1HomeToZPhaseDistanceSecond=0;
                iZ1DistanceReal=0;                                              //kevin 20170511 add log
            }
            else if(tempMOT==MTestZ2)
            {
                if(bCheckOverRangeZ2==false)
                {
                    iZ2DistanceRef=abs(iIndexZ2HomeToZPhaseDistanceFirst-iIndexZ2HomeToZPhaseDistanceSecond);
                    if(CheckIniData(asGeneralPath, "IndexDriver", "Index_Z2_Home_Position")==false)
                    {
                        WriteIniData(asGeneralPath, "IndexDriver", "Index_Z2_Home_Position", iZ2DistanceRef);
                        sData.sprintf("%s Finding phase no parameter, add Index_Z2_Home_Position=%d", sTime, iZ2DistanceRef);
                        SaveFile(asIndexZphasePath, sData);
                    }
                    else
                    {
                        iZ2DistanceRef=ReadIniData(asGeneralPath, "IndexDriver", "Index_Z2_Home_Position", 120);
                    }
                    bCheckOverRangeZ2=true;
                }
                iZ2DistanceReal=abs(iIndexZ2HomeToZPhaseDistanceFirst-iIndexZ2HomeToZPhaseDistanceSecond);
                iZData=abs(iZ2DistanceReal-iZ2DistanceRef);
                sData.sprintf("%s Arm 2 Zphase:First: %d ,Second: %d ,Real %d,ref %d,Zhase:%d", sTime, iIndexZ2HomeToZPhaseDistanceFirst, iIndexZ2HomeToZPhaseDistanceSecond, iZ2DistanceReal, iZ2DistanceRef, iZData); //kevin 20170511 add log
                SaveFile(asIndexZphasePath, sData);                             //kevin 20170511
                RecordProcess(sData);
                sData.sprintf("%s Arm 2 ZRead Pos First: %d ,Second: %d", sTime, iIndexZ2HomeToZPosFirst, iIndexZ2HomeToZPosSecond); //kevin 20170511 add log
                SaveFile(asIndexZphasePath, sData);                             //kevin 20170511
                if(iZData>(IniConfig.iD63IndexZHomeToZPhaseRange))              //50->0
                {
                    ret=ShowErrorMessage("WAR0308", K_RETRY|K_SKIP, MMSystem, "Gali_FindZPhase_446");
                    if(ret==K_SKIP)
                    {
                        if(CUSTOMER_CODE==CC_ASE_KaohSiung)
                        {
                            iZ2DistanceRef=iZ2DistanceReal;
                            WriteIniData(asGeneralPath, "IndexDriver", "Index_Z2_Home_Position", iZ2DistanceRef);
                            sData.sprintf("%s Skip press,and save Index_Z2_Home_Position as %d", sTime, iZ2DistanceRef);
                            SaveFile(asIndexZphasePath, sData);

                            iZ2DistanceRef=ReadIniData(asGeneralPath, "IndexDriver", "Index_Z2_Home_Position", 120);
                        }
                        else
                        {
                            bZ2ModifyDistanceRef=true;                          //Ifor 20190530 : add Z Phase 可SKIP 但需重新contact height
                        }
                        Task=447;
                    }
                    else if(ret==K_RETRY)
                    {
                        iZ2DistanceRef=ReadIniData(asGeneralPath, "IndexDriver", "Index_Z2_Home_Position", 120);
                        Task=1;
                    }
                }
                else
                {
                    Task=447;
                    bZ2ModifyDistanceRef=false;
                }

                if(bZ2ModifyDistanceRef==true)                                  //Ifor 20190530 : add Z Phase 可SKIP 但需重新contact height
                {
                    iZ2ModifyDistanceRef=iZ2DistanceReal;
                }
                else
                {
                    iZ2ModifyDistanceRef=iZ2DistanceRef;
                }

                iIndexZ2HomeToZPhaseDistanceFirst=0;
                iIndexZ2HomeToZPhaseDistanceSecond=0;
                iZ2DistanceReal=0;                                              //kevin 20170511 add log
            }
            else if(tempMOT==MTestY1)                                           //Isaac 20201110 : Index Y find motor phase
            {
                if(bCheckOverRangeY1==false)
                {
                    iY1DistanceRef=abs(iIndexY1HomeToYPhaseDistanceFirst-iIndexY1HomeToYPhaseDistanceSecond);
                    if(CheckIniData(asGeneralPath, "IndexDriver", "Index_Y1_Home_Position")==false)
                    {
                        WriteIniData(asGeneralPath, "IndexDriver", "Index_Y1_Home_Position", iY1DistanceRef);
                        sData.sprintf("%s Finding phase no parameter, add Index_Y1_Home_Position=%d", sTime, iY1DistanceRef);
                        SaveFile(asIndexZphasePath, sData);
                    }
                    else
                    {
                        iY1DistanceRef=ReadIniData(asGeneralPath, "IndexDriver", "Index_Y1_Home_Position", 20);
                    }
                    bCheckOverRangeY1=true;
                }
                iY1DistanceReal=abs(iIndexY1HomeToYPhaseDistanceFirst-iIndexY1HomeToYPhaseDistanceSecond);
                iYData=abs(iY1DistanceReal-iY1DistanceRef);
                sData.sprintf("%s Arm 1 Yphase:First: %d ,Second: %d ,Real %d,ref %d,Yhase:%d", sTime, iIndexY1HomeToYPhaseDistanceFirst, iIndexY1HomeToYPhaseDistanceSecond, iY1DistanceReal, iY1DistanceRef, iYData); //kevin 20170511 add log
                SaveFile(asIndexZphasePath, sData);                             //kevin 20170511
                RecordProcess(sData);
                sData.sprintf("%s Arm 1 YRead Pos First: %d ,Second: %d", sTime, iIndexY1HomeToYPosFirst, iIndexY1HomeToYPosSecond); //kevin 20170511 add log
                SaveFile(asIndexZphasePath, sData);                             //kevin 20170511

                if(iYData>(IniConfig.iD63IndexZHomeToZPhaseRange))
                {
                    ret=ShowErrorMessage("WAR03314", K_RETRY|K_SKIP, MMSystem);
                    if(ret==K_SKIP)
                    {
                        bY1ModifyDistanceRef=true;                              //Ifor 20190530 : add Z Phase 可SKIP 但需重新contact height
                        iIndexY1FindPhaseFrontPos=Tech.iHT9040TestY1_Front;
                        iIndexY1FindPhaseMiddlePos=Tech.iHT9040TestY1_Middle;
                        sData.sprintf("%s Y1 Error Now Teaching Position Y1:Front:%d,Middle:%d", sTime, Tech.iHT9040TestY1_Front, Tech.iHT9040TestY1_Middle);
                        SaveFile(asIndexZphasePath,sData);                      //kevin 20170511

                        Task=447;
                    }
                    else if(ret==K_RETRY)
                    {
                        iY1DistanceRef=ReadIniData(asGeneralPath, "IndexDriver", "Index_Y1_Home_Position", 20);
                        Task=1;
                        break;
                    }
                }
                else
                {
                    Task=447;
                    bY1ModifyDistanceRef=false;
                }

                if(bY1ModifyDistanceRef==true)                                  //Ifor 20190530 : add Z Phase 可SKIP 但需重新contact height
                {
                    iY1ModifyDistanceRef=iY1DistanceReal;
                }
                else
                {
                    if(iY1DistanceRef<0)
                    {
                        iY1ModifyDistanceRef=-iY1DistanceRef;
                    }
                    else
                    {
                        iY1ModifyDistanceRef=iY1DistanceRef;
                    }
                }

                iIndexY1HomeToYPhaseDistanceFirst=0;
                iIndexY1HomeToYPhaseDistanceSecond=0;
                iY1DistanceReal=0;                                              //kevin 20170511 add log
            }
            else if(tempMOT==MTestY2)
            {
                if(bCheckOverRangeY2==false)
                {
                    iY2DistanceRef = abs(iIndexY2HomeToYPhaseDistanceFirst-iIndexY2HomeToYPhaseDistanceSecond);
                    if(CheckIniData(asGeneralPath, "IndexDriver", "Index_Y2_Home_Position")==false)
                    {
                        WriteIniData(asGeneralPath, "IndexDriver", "Index_Y2_Home_Position", iY2DistanceRef);
                        sData.sprintf("%s Finding phase no parameter, add Index_Y2_Home_Position=%d", sTime, iY2DistanceRef);
                        SaveFile(asIndexZphasePath, sData);
                    }
                    else
                    {
                        iY2DistanceRef=ReadIniData(asGeneralPath, "IndexDriver", "Index_Y2_Home_Position", 20);
                    }
                    bCheckOverRangeY2=true;
                }
                iY2DistanceReal=abs(iIndexY2HomeToYPhaseDistanceFirst-iIndexY2HomeToYPhaseDistanceSecond);
                iY2DistanceRef=abs(iY2DistanceRef);
                iYData=abs(iY2DistanceReal-iY2DistanceRef);
                sData.sprintf("%s Arm 2 Yphase:First: %d ,Second: %d ,Real %d,ref %d,Zhase:%d", sTime, iIndexY2HomeToYPhaseDistanceFirst, iIndexY2HomeToYPhaseDistanceSecond, iY2DistanceReal, iY2DistanceRef, iYData); //kevin 20170511 add log
                SaveFile(asIndexZphasePath,sData);                              //kevin 20170511
                RecordProcess(sData);
                sData.sprintf("%s Arm 2 YRead Pos First: %d ,Second: %d", sTime, iIndexY2HomeToYPosFirst, iIndexY2HomeToYPosSecond); //kevin 20170511 add log
                SaveFile(asIndexZphasePath, sData);                             //kevin 20170511

                if(iYData>(IniConfig.iD63IndexZHomeToZPhaseRange))
                {
                    ret=ShowErrorMessage("WAR03314", K_RETRY|K_SKIP, MMSystem);
                    if(ret==K_SKIP)
                    {
                        bY2ModifyDistanceRef=true;                              //Ifor 20190530 : add Z Phase 可SKIP 但需重新contact height
                        iIndexY2FindPhaseMiddlePos=Tech.iHT9040TestY2_Middle;
                        iIndexY2FindPhaseRearPos=Tech.iHT9040TestY2_Rear;
                        sData.sprintf("%s Y2 Error Now Teaching Position Y2:Middle:%d,Rear:%d",sTime, Tech.iHT9040TestY2_Middle, Tech.iHT9040TestY2_Rear);
                        SaveFile(asIndexZphasePath, sData);                     //kevin 20170511
                        Task=447;
                    }
                    else if(ret==K_RETRY)
                    {
                        iY2DistanceRef=ReadIniData(asGeneralPath, "IndexDriver", "Index_Y2_Home_Position", 20);
                        Task=1;
                    }
                }
                else
                {
                    Task=447;
                    bY2ModifyDistanceRef=false;
                }

                if(bY2ModifyDistanceRef==true)                                  //Ifor 20190530 : add Z Phase 可SKIP 但需重新contact height
                {
                    iY2ModifyDistanceRef=iY2DistanceReal;
                }
                else
                {
                    if(iY2DistanceRef<0)
                    {
                        iY2ModifyDistanceRef=-iY2DistanceRef;
                    }
                    else
                    {
                        iY2ModifyDistanceRef=iY2DistanceRef;
                    }
                }

                iIndexY2HomeToYPhaseDistanceFirst=0;
                iIndexY2HomeToYPhaseDistanceSecond=0;
                iY2DistanceReal=0;                                              //kevin 20170511 add log
            }
            break;
        case 447:
            bScanFlag=false;
            if(Alias=="MTestZ1" || Alias=="MTestZ2")
            {
                if(Alias=="MTestZ1")
                    checkflag=CheckTestZ1("Gali_FindZPhase447");
                if(Alias=="MTestZ2")
                    checkflag=CheckTestZ2("Gali_FindZPhase447");

                if(checkflag)
                    Task=450;
                else
                    Task=500;
            }
            else
            {
                Task=500;
            }
            break;
        case 450:                                                               //kevin 20170515
            TestZSafePos=10;
            if(Alias=="MTestZ1" || Alias=="MTestZ2")
            {
                TestZSafePos=Prod.TestZ1_Safe;                                  //kevin 20160905
            }

            if(Gali_MotMove(TestZSafePos, Motor->PHomeHighSpeed, "Gali_FindZPhase_450"))
            {
                Task=500;
            }
            break;
        case 500:
            Task=900;
            if(IniConfig.bD63CheckIndexZHomeToZPhaseDistanceRange==true)        //kevin 20170605 add
            {
                MovFlag=false;
                GaliSofDelayCount=0;
                HomeFlag=1;
            }
            return true;
    }
    return false;
}
//-----------------------------------------------------------------------------
TQPF_Timer tGalilTwoYMoveDelay;
bool bGalilTwoYMoveFlag=false;
bool TMyMotor::GalilTwoY_Move(int iYPos, int Y2Pos, int Speed, AnsiString sFunc)
{
    // AI(W906-P0-5) 20260920: 離線守衛 —— 值與退休前 mymotor.cpp 的樁**完全相同**，所以 Motor==NULL 的可觀察行為一格沒變。
    if(Motor==NULL) return true;
    //******************************************************************************
    //  注意!! CheckIsSafeDoorOpen為Handler 安全門相關, 修改時要小心!!
    //******************************************************************************
    if(Motor->CheckIsSafeDoorOpen())                                            //Jimmychiu 20221013 safedoor判斷整合Function
    {
        bGalilTwoYMoveFlag=false;
        return false;
    }

    if(USE_INDEX_ARM_AXES==IndexArm_3_Axis)                                     //JimmyChiu 20220708 : add Index Arm Axis
    {
        return Gali_MotMove(iYPos, Speed, "GalilTwoY_Move"+sFunc);
    }

    if(W906_GaliRouteAbsentAxis(MTestY1) && W906_GaliRouteAbsentAxis(MTestY2)) { bGalilTwoYMoveFlag=false; MOT[MTestY1].Position=iYPos; MOT[MTestY2].Position=-Y2Pos; MOT[MTestY1].TargetPosition=iYPos; MOT[MTestY2].TargetPosition=-Y2Pos; return true; }   /* AI(W906-INDEX-ABSENT) 20261004: no Index Y on this machine (HT9050) -- the disabled branch's final state at once, no bIndexProtect retries */  Check_Y1_TargetPosInTeachPos(iYPos, sFunc);
    Check_Y2_TargetPosInTeachPos(Y2Pos, sFunc);

    bool flag[2]={false, false};
    char str[256]="";
    AnsiString SelMot=AnsiString(""), sErrFunc=__FUNC__+AnsiString(",Func2,")+sFunc, str1="";
    double sp=Speed;
    Speed=(int)sp;
    static int iOldY1=0, iOldY2=0;                                              //Sam 20220611 : 避免上一動作  bGalilTwoYMoveFlag 沒有被清除乾淨，導致偵測誤報警

    #ifndef SOFT_SIMULTE
    static int iRetryCnt=0;
    int iIndexZ1Pos=0, iIndexZ2Pos=0;
    if(CosFunction.bIndexProtect==true)                                         //Steven 20180319 (jou) : 加入Index Y軸移動前確認Z軸位置保護
    {
        iIndexZ1Pos=MOT[MTestZ1].Gali_ReadEncoderPos();
        iIndexZ2Pos=MOT[MTestZ2].Gali_ReadEncoderPos();

        if(iIndexZ1Pos<-200 || iIndexZ2Pos<-200)
        {
            if(iRetryCnt<100)
            {
                iRetryCnt++;
                return false;
            }
            bGalilTwoYMoveFlag=false;
            ShowIndexMotorError(AnsiString("GalilTwoY_Move"));
        }
        iRetryCnt=0;
        #ifdef DEBUG_GALIL_CHECK_Y
        if((iYPos>(Tech.iHT9040TestY1_Middle-7000)) &&      //JerryYang 20250822 : fix index y保護
           (Y2Pos<(Tech.iHT9040TestY2_Middle+7000)))
        {
            str1.sprintf("Index Y Position Calculate error. Y1 Target Pos: %d, Y2 Target Pos:%d, %s", iYPos, Y2Pos, __FUNC__);
            ShowMyMessage(str1);
            fAllMotorHome=false;
            return false;
        }
        #endif
    }
    #endif

    Y2Pos=-Y2Pos;
    if(Motor->Enable && bGali_CardInstall==true)
    {
        MOT[MTestY1].GetRealPos(&iYPos);
        if(USE_INDEX_ARM_AXES==IndexArm_4_Axis)                                 //JimmyChiu 20220708 : add Index Arm Axis
        {
            MOT[MTestY2].GetRealPos(&Y2Pos);
        }
        MOT[MTestY1].Position=MOT[MTestY1].Gali_ReadPos();
        if(USE_INDEX_ARM_AXES==IndexArm_4_Axis)                                 //JimmyChiu 20220708 : add Index Arm Axis
        {
            MOT[MTestY2].Position=MOT[MTestY2].Gali_ReadPos();
        }

        if(bGalilTwoYMoveFlag==false || bResetGalilTwoYMove==true)              //Steven 20210819 : Galil移動異常偵測 //Ifor 20220729 add Reset Galil Two Y Move時間計時
        {
            tGalilTwoYMoveDelay.SetSecAndOn(300);
            bGalilTwoYMoveFlag=true;
            bResetGalilTwoYMove=false;
        }

        if(MovFlag==false)
        {
            iGali_SpSpeed=Speed;
            W4G_sprintf(str,"SP%d,,,%d;AC%d,,,%d;DC%d,,,%d;PA%d,,,%d;BGXW;", Speed, Speed, GailAcSpeed2, GailAcSpeed2, GailDcSpeed2, GailDcSpeed2, iYPos, Y2Pos);

            if(ScanIndexMotorCanMove())
            {
                Gali_Command(str, sErrFunc);
            }
            else
            {
                if(tGalilTwoYMoveDelay.Off())                                   //Steven 20210819 : Galil移動異常偵測
                {
                    SelMot.sprintf("GalilTwoY_Move - ScanIndexMotorCanMove fail, iYPos=%d, Y2Pos=%d, Speed=%d, sFunc=%s", iYPos, Y2Pos, Speed, sErrFunc);
                    ShowMyMessage("GalilTwoY_Move fail", SelMot, SelMot);
                    tGalilTwoYMoveDelay.SetSecAndOn(300);
                    bGalilTwoYMoveFlag=true;
                }
                return false;
            }
            GaliSofDelayCount=0;
            bScanFlag=false;
            MovFlag=true;
        }
        else
        {
            if(USE_INDEX_ARM_AXES==IndexArm_4_Axis)                                 //JimmyChiu 20220708 : add Index Arm Axis
            {
                if(iOldY1!=MOT[MTestY1].Position ||
                   iOldY2!=MOT[MTestY2].Position ||
                   bGalilTwoYMoveFlag==false)                                   //Sam 20220720 : 修正 GalilTwoY_Move 誤報警問題。
                {
                    iOldY1=MOT[MTestY1].Position;
                    iOldY2=MOT[MTestY2].Position;
                    bGalilTwoYMoveFlag=true;                                    //Sam 20220720 : 修正 GalilTwoY_Move 誤報警問題。
                    tGalilTwoYMoveDelay.SetSecAndOn(300);
                }
            }

            if(tGalilTwoYMoveDelay.Off())                                       //Steven 20210819 : Galil移動異常偵測
            {
                SelMot.sprintf("GalilTwoY_Move fail, iYPos=%d, Y2Pos=%d, Speed=%d, sFunc=%s", iYPos, Y2Pos, Speed, sErrFunc);
                ShowMyMessage("GalilTwoY_Move fail", SelMot, SelMot);
                tGalilTwoYMoveDelay.SetSecAndOn(300);
                bGalilTwoYMoveFlag=true;
            }

            if(Gali_Command("MG_BGx", sErrFunc)==1 ||                           //ChungHung 20141016 Try fix 點位飄移
               Gali_Command("MG_BGw", sErrFunc)==1)                             //馬達目前是否移動? 0:移動完成
                return false;

            if(bScanFlag==false)
            {
                MOT[MTestY1].Gali_ScanMotStatus();
                flag[0]=MOT[MTestY1].Led[iInposLed];
                if(USE_INDEX_ARM_AXES==IndexArm_3_Axis)
                {
                    flag[1]=true;
                }
                else
                {
                    MOT[MTestY2].Gali_ScanMotStatus();
                    flag[1]=MOT[MTestY2].Led[iInposLed];
                }
            }

            if(flag[0]==false && flag[1]==false)
            {
                bScanFlag=true;
                GaliSofDelayCount++;
                if(GaliSofDelayCount>=DelayCount)
                {
                    bScanFlag=false;
                    GaliSofDelayCount=0;
                    MovFlag=false;
                    bGalilTwoYMoveFlag=false;                                   //Steven 20210819 : Galil移動異常偵測
                    return true;
                }
            }
        }
    }
    else //if(Motor->Enable==false)
    {
        Speed=ArmSpeed[IndexArm].iBodySP*W906_SimGalilStepScale();  //AI(W906-SIMSTEP) 20260924: 模擬步進 ×W906_SimGalilStepScale()（B13 500 ms tick；出貨組態＝1，見本檔檔尾）
        if(Speed<=0)
            Speed=10;
        MOT[MTestY1].Position+=Speed;
        MOT[MTestY2].Position-=Speed;
        if(MOT[MTestY1].Position>=iYPos || MOT[MTestY2].Position<=-Y2Pos)
        {
            MOT[MTestY1].Position=iYPos;
            MOT[MTestY2].Position=-Y2Pos;
            MOT[MTestY1].TargetPosition=iYPos;                                  //Steven 20210827 : 修改顯示Galil的Target / Current
            MOT[MTestY2].TargetPosition=-Y2Pos;
            return true;
        }
    }
    return false;
}
//------------------------------------------------------------------------------
bool ScanIndexMotorCanMove()
{
    int MotorIndex[4]={MTestY1, MTestZ1, MTestZ2, MTestY2};
    for(int j=0; j<4; j++)
    {
        int i=MotorIndex[j];
        if(USE_INDEX_ARM_AXES==IndexArm_3_Axis && i==MTestY2)                   //JimmyChiu 20220708 : add Index Arm Axis
            continue;
        MOT[i].Gali_ScanAlarmStatus();
        if(MOT[i].Led[iAlarmLed] && MOT[i].Motor->Enable && MOT[i].Motor->PServoAlarmOn)
        {
            MOT[i].HomeFlag=0;
            fAllMotorHome=false;
            SystemStart=false;
            JamCode=MotorIndexToJamCode(i);
            ShowMotorErrorMessage(JamCode,1);
            return false;
        }
    }
    return true;
}
const int iStartRetry=50;                                                       //約100ms，足以覆蓋大多數PLC更新周期
const int iPosDeadband=2;                                                       //避免雜訊導致誤判
//------------------------------------------------------------------------------
bool DoIndexVectorMove(const char* vecCmd)                                 //AI(Copilot) 20260316 (RogerYang) : 分離前置放行與向量啟動確認
{
    if(ScanIndexMotorCanMove()==false)
        return false;

    if(vecCmd==NULL)
        return true;

    //JerryYang 20260527 : 解析vecCmd中LI位移量，推算目標位置，進行撞機預檢
    if(USE_INDEX_ARM_AXES==IndexArm_4_Axis && bGali_CardInstall==true)
    {
        // Step 1: 累加vecCmd中所有LI相對位移量 (Galil軸對應: X=Y1, Y=Z1, Z=Z2, W=Y2)，單位為Pulse
        int iDeltaX=0, iDeltaY=0, iDeltaZ=0, iDeltaW=0;
        const char* pLI = vecCmd;
        while((pLI = strstr(pLI, "LI")) != NULL)
        {
            if(*(pLI+2) == 'M') { pLI+=2; continue; }  // 跳過LMXYZW
            int ix=0, iy=0, iz=0, iw=0;
            if(sscanf(pLI+2, "%d,%d,%d,%d", &ix, &iy, &iz, &iw) >= 3)
            {
                iDeltaX += ix;
                iDeltaY += iy;
                iDeltaZ += iz;
                iDeltaW += iw;
            }
            pLI += 2;
        }
        // Step 2: 讀取目前encoder位置 (單位: 0.01mm，Gali_ReadEncoderPos 已乘以GearRatio)
        int iCurY1 = MOT[MTestY1].Gali_ReadEncoderPos();
        int iCurZ1 = MOT[MTestZ1].Gali_ReadEncoderPos();
        int iCurZ2 = MOT[MTestZ2].Gali_ReadEncoderPos();
        int iCurY2 = MOT[MTestY2].Gali_ReadEncoderPos();
        // Step 3: 將LI位移量從Pulse換算為0.01mm (乘以GearRatio)
        //         Y1軸在4-axis模式下 Gali_ReadEncoderPos 使用負號，故delta也需取負
        int iSwDeltaY1 =  (int)(iDeltaX * MOT[MTestY1].Motor->GearRatio);
        int iSwDeltaZ1 =  -(int)(iDeltaY * MOT[MTestZ1].Motor->GearRatio);
        int iSwDeltaZ2 =  -(int)(iDeltaZ * MOT[MTestZ2].Motor->GearRatio);
        int iSwDeltaY2 =  -(int)(iDeltaW * MOT[MTestY2].Motor->GearRatio);
        // Step 4: 推算移動後的目標位置 (單位: 0.01mm)
        int iTarY1 = iCurY1 + iSwDeltaY1;
        int iTarZ1 = iCurZ1 + iSwDeltaZ1;
        int iTarZ2 = iCurZ2 + iSwDeltaZ2;
        int iTarY2 = iCurY2 + iSwDeltaY2;
        // 規則1: 目標Z若下壓到Socket高度(<=All_TestZ_Test_Safe)，對應Y軸必須在Socket附近
        // 修正: 閾值用All_TestZ_Test_Safe本身，不再減iCheckZ (iCheckZ是超射保護用，不是到達socket的判斷)
        if(iTarZ1 <= Prod.All_TestZ_Test_Safe)
        {
            if(CheckIndexArmPosInRange(iTarY1, Prod.TestY1_Middle-IniConfig.GaliPosRange, Prod.TestY1_Middle+IniConfig.GaliPosRange)==false)
            {
                ShowIndexMotorError(AnsiString("DoIndexVectorMove_Z1Down_Y1NotSafe"));
                return false;
            }
        }

        if(iTarZ2 <= Prod.All_TestZ_Test_Safe)
        {
            if(CheckIndexArmPosInRange(iTarY2, Prod.TestY2_Middle-IniConfig.GaliPosRange, Prod.TestY2_Middle+IniConfig.GaliPosRange)==false)
            {
                ShowIndexMotorError(AnsiString("DoIndexVectorMove_Z2Down_Y2NotSafe"));
                return false;
            }
        }
        // 規則2: 目標Y1與Y2同時在Socket附近即為撞機風險（Y軸水平碰撞，與Z是否下壓無關）
        // 參考Check_Y1Y2_TargetPosWillCrash
        bool bTarY1NearSocket = (iTarY1 > (Tech.iHT9040TestY1_Middle - 7000));
        bool bTarY2NearSocket = (iTarY2 < (Tech.iHT9040TestY2_Middle + 7000));
        if(bTarY1NearSocket && bTarY2NearSocket)
        {
            ShowIndexMotorError(AnsiString("DoIndexVectorMove_Y1Y2BothNearSocket"));
            return false;
        }
    }
    //AI(Copilot) 20260316 (RogerYang) : 改為 BG/SC + 位置變化雙條件確認，避免BG延遲回報誤判
    long lStartX=MOT[MTestY1].Gali_ReadPos();
    long lStartY=MOT[MTestZ1].Gali_ReadPos();
    long lStartZ=MOT[MTestZ2].Gali_ReadPos();
    long lStartW=(USE_INDEX_ARM_AXES==IndexArm_4_Axis) ? MOT[MTestY2].Gali_ReadPos() : 0;

    MOT[MTestY1].Gali_Command(vecCmd, __FUNC__);

    for(int iTry=0; iTry<iStartRetry; iTry++)
    {
//        long lSC  = MOT[MTestY1].Gali_Command("SC",     __FUNC__);             //全軸停止碼，100=異常
        long lBGx = MOT[MTestY1].Gali_Command("MG_BGx", __FUNC__);             //Y1軸運動中=1
        long lBGy = MOT[MTestY1].Gali_Command("MG_BGy", __FUNC__);             //Z1軸運動中=1
        long lBGz = MOT[MTestY1].Gali_Command("MG_BGz", __FUNC__);             //Z2軸運動中=1
        long lBGw = (USE_INDEX_ARM_AXES==IndexArm_4_Axis) ?
                    MOT[MTestY1].Gali_Command("MG_BGw", __FUNC__) : 0;         //4軸模式才讀Y2

        long lNowX=MOT[MTestY1].Gali_ReadPos();
        long lNowY=MOT[MTestZ1].Gali_ReadPos();
        long lNowZ=MOT[MTestZ2].Gali_ReadPos();
        long lNowW=(USE_INDEX_ARM_AXES==IndexArm_4_Axis) ? MOT[MTestY2].Gali_ReadPos() : 0;

        long lDiffX=lNowX-lStartX; if(lDiffX<0) lDiffX=-lDiffX;
        long lDiffY=lNowY-lStartY; if(lDiffY<0) lDiffY=-lDiffY;
        long lDiffZ=lNowZ-lStartZ; if(lDiffZ<0) lDiffZ=-lDiffZ;
        long lDiffW=lNowW-lStartW; if(lDiffW<0) lDiffW=-lDiffW;
        bool bPosMoved=(lDiffX>=iPosDeadband || lDiffY>=iPosDeadband || lDiffZ>=iPosDeadband ||
                       (USE_INDEX_ARM_AXES==IndexArm_4_Axis && lDiffW>=iPosDeadband));

        bool bBGAllStop = (lBGx==0 && lBGy==0 && lBGz==0 &&
                          ((USE_INDEX_ARM_AXES==IndexArm_4_Axis) ? (lBGw==0) : true));
        if(!bBGAllStop || bPosMoved)
            return true;

        MySleepEx(2, true);
    }

    iHome=1;
    ShowMyMessage("Vector command start check fail",
                  "向量命令送出後啟動確認失敗(BG/SC/Pos, retry 50次)，請先Home再啟動");
    return false;
}
//------------------------------------------------------------------------------
bool ScanIndexOtherMotorCanMove(int Index)
{
    return ScanIndexMotorCanMove();
}
//------------------------------------------------------------------------------
void StopAllMotor(bool bIndexCanStop)
{
    if(MOT[MTestY1].Motor!=NULL &&
       bIndexCanStop==true)                                                     //Steven 20220309 : 避免Galil Command Error時, 不能Alarm
    {
        MOT[MTestY1].Gali_Command("VS0;SP0,0,0,0;", __FUNC__);
        bGalilTwoYMoveFlag=false;                                               //Sam 20220720 : 修正 GalilTwoY_Move 誤報警問題。
    }

    for(int i=0; i<TOTAL_MOTOR; i++)
    {
        if(MOT[i].Motor==NULL)                                                  //2013-08-30    Dell    沒開24V造成記憶體溢位
            continue;

        if(MOT[i].Motor->Enable==true)
        {
            if(MOT[i].Motor->PServoAlarmOn==1)                                  //jou 2014-07-02 retrun to V2.12R 修正立即停止造成步進馬達失步
            {
                MOT[i].PCIL132_StopMotor();
            }
            else                                                                //Steven 20170504 (wei) : 修正在馬達工具裡面按STOP,步進馬達不會停的問題
            {
                if(SystemStart==false)
                    MOT[i].PCIL132_StopMotor();
            }
        }
    }

    if(INSTALL_OCR!=eocrUninstal && CosFunction.bTrayOCR)                       //wei 20150925 待機位置改道 Color
    {
        MOT[MLoaderY].PCIL132_StopMotor();
    }
    else
    {
        TrayMoveOut(false, 0);                                                  //Sam 20210325 : 修正整合 tray move
    }

    for(int i=eAuto1; i<=iAutoRight; i++)
    {
        int iAuto=iAutoIndex[i];
        SW[SwAutoCCW[iAuto]].Off();
        SW[SwAutoCW[iAuto]].Off();
    }

    for(int i=0; i<MAX_TRACK; i++)
       TrayMoveOut(false, i);

    //******************************************************************************
    //  注意!! 有在本Function使用到氣缸的話, 必須將Cylinder[i].bCheckSafeDoor==false, 否則進入CheckIsSafeDoorOpen會當掉
    //******************************************************************************
    Cylinder[C_HotplateVibration].Off();
    Cylinder[C_TrayVibration].Off();

    SW[SwShuttleVibration1].Off();
    SW[SwShuttleVibration2].Off();
    SW[SwLoaderVibration].Off();                                                //JerryYang 20191001 loader 震動馬達
}
//------------------------------------------------------------------------------
bool TMyMotor::ISZ1Up_Z2Up()
{
    // AI(W906-P0-5) 20260920: 離線守衛 —— 值與退休前 mymotor.cpp 的樁**完全相同**，所以 Motor==NULL 的可觀察行為一格沒變。
    if(Motor==NULL) return false;
    if(MOT[MTestZ1].Gali_ReadEncoderInRandge(Prod.TestZ1_Safe) &&
       MOT[MTestZ2].Gali_ReadEncoderInRandge(Prod.TestZ2_Safe))
        return true;
    else
        return false;
}
//------------------------------------------------------------------------------
bool TMyMotor::ISZ1Down_Z2Down()
{
    // AI(W906-P0-5) 20260920: 離線守衛 —— 值與退休前 mymotor.cpp 的樁**完全相同**，所以 Motor==NULL 的可觀察行為一格沒變。
    if(Motor==NULL) return false;
    if(MOT[MTestZ1].Gali_ReadEncoderMaxRandge(iBackUpZ1DownPosition) &&
       MOT[MTestZ2].Gali_ReadEncoderMaxRandge(iBackUpZ2DownPosition))
        return true;
    else
        return false;
}
//------------------------------------------------------------------------------
bool TMyMotor::Gali_nnMode_Z1Z2_Down(int Speed, bool TMode)                     //2012-10-22  Dell for HT9046LS
{
    // AI(W906-P0-5) 20260920: 離線守衛 —— 值與退休前 mymotor.cpp 的樁**完全相同**，所以 Motor==NULL 的可觀察行為一格沒變。
    if(Motor==NULL) return false;
    char str[512]="";
    char tmpstr[512]="";
    static bool MOTMovflag[]={true, true, true, true};
    static int iCount=0;                                                        //kevin 20220221 add Gali  Command state  Mylin 20211228 Add Gali Command Log

    //******************************************************************************
    //  注意!! CheckIsSafeDoorOpen為Handler 安全門相關, 修改時要小心!!
    //******************************************************************************
    if(Motor->CheckIsSafeDoorOpen())                                            //Jimmychiu 20221013 safedoor判斷整合Function
    {
        return false;
    }

    if(Motor->Enable && bGali_CardInstall==true)
    {
        GetIndexParm();                                                         //取得移動距離

        if(bShowMotorMove==true)
        {
            MOT[MTestY1].Position=MOT[MTestY1].Gali_ReadPos();
            if(USE_INDEX_ARM_AXES==IndexArm_4_Axis)                             //JimmyChiu 20220708 : add Index Arm Axis
            {
                MOT[MTestY2].Position=MOT[MTestY2].Gali_ReadPos();
            }
            #ifdef DEBUG_32SITE
                fMain->StringGrid14->Cells[1][1]=MOT[MTestY1].Position;
                if(USE_INDEX_ARM_AXES==IndexArm_4_Axis)
                {
                    fMain->StringGrid14->Cells[1][2]=MOT[MTestY2].Position;
                }
            #endif
        }

        if(MovFlag==false && bZ1Z2Exute==false)                                 //kevin 20110630
        {
            if(CheckTestZ("Gali_nnMode_Z1Z2_Down"))                             //確認四軸command & encoder位置是否不同
            {
                #ifdef DEBUG_32SITE
                    fMain->StringGrid14->Cells[2][1]=MOT[MTestY1].Position;
                    if(USE_INDEX_ARM_AXES==IndexArm_4_Axis)
                    {
                        fMain->StringGrid14->Cells[2][2]=MOT[MTestY2].Position;
                    }

                    fMain->StringGrid14->Cells[3][1]=MOT[MTestY1].Gali_ReadEncoderPos();
                    if(USE_INDEX_ARM_AXES==IndexArm_4_Axis)
                    {
                        fMain->StringGrid14->Cells[3][2]=MOT[MTestY2].Gali_ReadEncoderPos();
                    }
                #endif

                iGali_VsSpeed=Speed;                                            //?與 Z1UpZ2Down 不同?
                MOT[MTestZ2].GetRealPos(&Z2Safe);

                MOT[MTestY1].GetRealPos(&XShiftF);
                MOT[MTestZ1].GetRealPos(&Z1DownSafe);
                MOT[MTestZ1].GetRealPos(&Z2DownSafe);
                MOT[MTestZ2].GetRealPos(&Z2Up);
                if(USE_INDEX_ARM_AXES==IndexArm_4_Axis)
                {
                    MOT[MTestY2].GetRealPos(&XShiftR);
                }

                MOT[MTestZ1].GetRealPos(&Z1Down);
                MOT[MTestZ1].GetRealPos(&Z2Down);

                W4G_sprintf(tmpstr, "LMXYZW;LI%d,%d,%d,%d;",
                                -XShiftF, -Z1DownSafe, -Z2DownSafe, -XShiftR);
                W4G_sprintf(str, "%s;LI0,%d,%d,0;LE;VS%d;VA%d;VD%d;BGS;",
                                tmpstr, -Z1Down, -Z2Down, Speed, GailAcSpeed, GailDcSpeed);

                bScanFlag=false;
                if(DoIndexVectorMove(str))
                {
                    bZ1Z2Exute=true;                                            //kevin 20110630
                }
                else
                {
                    return false;
                }

                MovFlag=true;
                for(int i=0; i<Gali_MaxAxis; i++)
                {
                    MOTMovflag[i]=true;
                    MOT[MTestY1+i].Led[iInposLed]=true;
                }
            }
        }
        else
        {
            #ifdef DEBUG_32SITE
                fMain->StringGrid14->Cells[4][1]= MOT[MTestY1].Gali_ReadEncoderPos();
                if(USE_INDEX_ARM_AXES==IndexArm_4_Axis)
                {
                    fMain->StringGrid14->Cells[4][2]= MOT[MTestY2].Gali_ReadEncoderPos();
                }
            #endif

            if(Gali_Command("SC")==100)                                         //ChungHung 20150831 add 確保每次皆到位
            {
                iCount++;                                                       //Mylin 20211228 Add Gali Command Log ==>
                if(iCount>5000)
                {
                    fMain->sb_Gali_LogClick();
                    iCount=0;
                    iHome =1;
                    ShowMyMessage("Gali Command Error When Z1UpZ2Down1, Please Home and Start", "請更換Galil 轉板或100P線材");
                }
                return false;
            }
            iCount=0;                                                           //kevin 20220221 add Gali  Command state Mylin 20211228 Add Gali Command Log

            if(bScanFlag==false)
            {
                for(int i=0; i<Gali_MaxAxis; i++)
                {
                    if(USE_INDEX_ARM_AXES==IndexArm_3_Axis &&
                       (MTestY1+i)==MTestY2)                                    //JimmyChiu 20220708 : add Index Arm Axis
                    {
                        MOTMovflag[i]=false;
                    }
                    else
                    {
                        if(MOTMovflag[i])
                        {
                            MOT[MTestY1+i].Gali_ScanMotStatus();
                            MOTMovflag[i]=MOT[MTestY1+i].Led[iInposLed];
                        }
                    }
                }
            }

            if(MOTMovflag[0]==false && MOTMovflag[1]==false &&
               MOTMovflag[2]==false && MOTMovflag[3]==false)
            {
                bScanFlag=true;

                GaliSofDelayCount++;
                if(GaliSofDelayCount>=DelayCount)
                {
                    if(CheckPos_nnMode(false)==false)                           //now only check TestY
                    {
                        if(GaliSofDelayCount<=(DelayCount*5))
                            return false;
                        iHome=1;
                        JamCode = MotorIndexToJamCode(13);
                        ShowMotorErrorMessage(JamCode,MotErrPos+1);
                        bScanFlag=false;
                        GaliSofDelayCount=0;
                        MovFlag=false;
                        return false;
                    }

                    if(ISZ1Down_Z2Down())                                       //check z1 & z2 encoder position
                    {
                        bScanFlag=false;
                        GaliSofDelayCount=0;
                        MovFlag=false;
                        bZ1Z2Exute=false;                                       //kevin 20110630

                        IndexStatus=Z1_Z2_Down;
                        fOffSet->bEnterSpecialOffset=false;
                        return true;
                    }
                    else
                    {
                        iHome=1;
                        if(GaliAxisAlarm[1])
                            JamCode=MotorIndexToJamCode(14);
                        else
                            JamCode=MotorIndexToJamCode(15);

                        ShowMotorErrorMessage(JamCode, MotErrPos+1);
                        bScanFlag=false;
                        GaliSofDelayCount=0;
                        MovFlag=false;
                        return false;
                    }
                }
            }
        }
    }
    else
    {
        Speed=ArmSpeed[IndexArm].iBodySP*10*W906_SimGalilStepScale();  //AI(W906-SIMSTEP) 20260924: 模擬步進 ×W906_SimGalilStepScale()（B13 500 ms tick；出貨組態＝1，見本檔檔尾）
        if(Speed<=0)
            Speed=10;
        MOT[MTestZ1].Position-=Speed;
        MOT[MTestZ2].Position-=Speed;
        MOT[MTestY1].Position+=Speed;
        MOT[MTestY2].Position-=Speed;

        MOT[MTestY1].TargetPosition=Prod.TestY1_Middle;                         //Steven 20210827 : 修改顯示Galil的Target / Current
        MOT[MTestY2].TargetPosition=Prod.TestY2_Middle;
        MOT[MTestZ1].TargetPosition=-Prod.TestZ1_Test;
        MOT[MTestZ2].TargetPosition=Prod.TestZ2_Test;

        if(MOT[MTestZ1].Position<=Prod.TestZ1_Test   || MOT[MTestZ2].Position <=Prod.TestZ2_Test ||
           MOT[MTestY1].Position<=Prod.TestY1_Middle || MOT[MTestY2].Position>=Prod.TestY2_Middle)
        {
            IndexStatus=Z1_Z2_Down;
            MOT[MTestY1].Position=Prod.TestY1_Middle;
            MOT[MTestY2].Position=Prod.TestY2_Middle;
            MOT[MTestZ1].Position=-Prod.TestZ1_Test;
            MOT[MTestZ2].Position=Prod.TestZ2_Test;
            fOffSet->bEnterSpecialOffset=false;
            return true;
        }
        return false;
    }
    return false;
}
//------------------------------------------------------------------------------
bool TMyMotor::Gali_nnMode_Z1Z2_Up(int Speed, bool TMode)                       //2012-10-22  Dell for HT9046LS
{
    // AI(W906-P0-5) 20260920: 離線守衛 —— 值與退休前 mymotor.cpp 的樁**完全相同**，所以 Motor==NULL 的可觀察行為一格沒變。
    if(Motor==NULL) return false;
    char str[512]="";
    char tmpstr[512]="";
    static bool MOTMovflag[]={true, true, true, true};
    static int iCount=0;                                                        //kevin 20220221 add Gali  Command state  Mylin 20211228 Add Gali Command Log

    //******************************************************************************
    //  注意!! CheckIsSafeDoorOpen為Handler 安全門相關, 修改時要小心!!
    //******************************************************************************
    if(Motor->CheckIsSafeDoorOpen())                                            //Jimmychiu 20221013 safedoor判斷整合Function
    {
        return false;
    }

    if(Motor->Enable && bGali_CardInstall==true)
    {
        GetIndexParm();                                                         //取得移動距離

        if(bShowMotorMove==true)
        {
            MOT[MTestY1].Position=MOT[MTestY1].Gali_ReadPos();
            MOT[MTestY2].Position=MOT[MTestY2].Gali_ReadPos();

            #ifdef DEBUG_32SITE
                fMain->StringGrid14->Cells[1][3]=MOT[MTestY1].Position;
                fMain->StringGrid14->Cells[1][4]=MOT[MTestY2].Position;
            #endif
        }

        if(MovFlag==false && bZ1Z2Exute==false)                                 //kevin 20110630
        {
            if(CheckTestZ("Gali_nnMode_Z1Z2_Up"))                               //確認四軸command & encoder位置是否不同
            {
                #ifdef DEBUG_32SITE
                    fMain->StringGrid14->Cells[2][3]=MOT[MTestY1].Gali_ReadPos();
                    fMain->StringGrid14->Cells[2][4]=MOT[MTestY2].Gali_ReadPos();

                    fMain->StringGrid14->Cells[3][3]=MOT[MTestY1].Gali_ReadEncoderPos();
                    fMain->StringGrid14->Cells[3][4]=MOT[MTestY2].Gali_ReadEncoderPos();
                #endif

                iGali_VsSpeed=Speed;                                            //?與 Z1UpZ2Down 不同?
                MOT[MTestZ2].GetRealPos(&Z1Safe);
                MOT[MTestZ2].GetRealPos(&Z2Safe);

                MOT[MTestY1].GetRealPos(&XShiftF);
                MOT[MTestZ1].GetRealPos(&Z1DownSafe);
                MOT[MTestZ1].GetRealPos(&Z2DownSafe);
                MOT[MTestZ2].GetRealPos(&Z2Up);
                MOT[MTestY2].GetRealPos(&XShiftR);

                MOT[MTestZ1].GetRealPos(&Z1Down);
                MOT[MTestZ1].GetRealPos(&Z2Down);

                W4G_sprintf(tmpstr, "LMXYZW;LI0,%d,%d,0;LI%d,%d,%d,%d;",
                                -Z1Safe, -Z2Safe,                               //Steven 20160518 : 修正32site drop mode
                                XShiftF, Z1DownSafe, Z2DownSafe, XShiftR);
                W4G_sprintf(str, "%s;LE;VS%d;VA%d;VD%d;BGS;",
                             tmpstr, Speed, GailAcSpeed, GailDcSpeed);

                bScanFlag=false;
                if(DoIndexVectorMove(str))
                {
                    bZ1Z2Exute=true;                                            //kevin 20110630
                }
                else
                {
                    return false;
                }

                MovFlag=true;
                for(int i=0; i<Gali_MaxAxis; i++)
                {
                    MOTMovflag[i]=true;
                    MOT[MTestY1+i].Led[8]=true;
                }
            }
        }
        else
        {
            #ifdef DEBUG_32SITE
                fMain->StringGrid14->Cells[4][3]=MOT[MTestY1].Gali_ReadEncoderPos();
                fMain->StringGrid14->Cells[4][4]=MOT[MTestY2].Gali_ReadEncoderPos();
            #endif

            if(Gali_Command("SC")==100)                                         //ChungHung 20150831 add 確保每次皆到位
            {
                iCount++;                                                       //Mylin 20211228 Add Gali Command Log ==>
                if(iCount>5000)
                {
                    fMain->sb_Gali_LogClick();
                    iCount=0;
                    iHome =1;
                    ShowMyMessage("Gali Command Error When Z1UpZ2Down1, Please Home and Start", "請更換Galil 轉板或100P線材");
                }
                return false;
            }
            iCount=0;                                                           //kevin 20220221 add Gali  Command state Mylin 20211228 Add Gali Command Log

            if(bScanFlag==false)
            {
                for(int i=0; i<Gali_MaxAxis; i++)
                {
                    if(MOTMovflag[i])
                    {
                        MOT[MTestY1+i].Gali_ScanMotStatus();
                        MOTMovflag[i]=MOT[MTestY1+i].Led[iInposLed];
                    }
                }
            }

            if(MOTMovflag[0]==false && MOTMovflag[1]==false &&
               MOTMovflag[2]==false && MOTMovflag[3]==false)
            {
                bScanFlag=true;
                GaliSofDelayCount++;
                if(GaliSofDelayCount>=DelayCount)
                {
                    if(CheckPos_nnMode(true)==false)                            //now only check TestY
                    {
                        if(GaliSofDelayCount<=(DelayCount*5))
                            return false;
                        iHome=1;
                        JamCode = MotorIndexToJamCode(13);
                        ShowMotorErrorMessage(JamCode, MotErrPos+1);
                        bScanFlag=false;
                        GaliSofDelayCount=0;
                        MovFlag=false;
                        return false;
                    }

                    if(ISZ1Up_Z2Up())                                           //check z1 & z2 encoder position
                    {
                        bScanFlag=false;
                        GaliSofDelayCount=0;
                        MovFlag=false;
                        bZ1Z2Exute=false;                                       //kevin 20110630

                        IndexStatus=Z1_Z2_Normal;
                        fOffSet->bEnterSpecialOffset=false;
                        return true;
                    }
                    else
                    {
                        iHome=1;
                        if(GaliAxisAlarm[1])
                            JamCode=MotorIndexToJamCode(14);
                        else
                            JamCode=MotorIndexToJamCode(15);

                        ShowMotorErrorMessage(JamCode, MotErrPos+1);
                        bScanFlag=false;
                        GaliSofDelayCount=0;
                        MovFlag=false;
                        return false;
                    }
                }
            }
        }
    }
    else
    {
        Speed=ArmSpeed[IndexArm].iBodySP*10*W906_SimGalilStepScale();  //AI(W906-SIMSTEP) 20260924: 模擬步進 ×W906_SimGalilStepScale()（B13 500 ms tick；出貨組態＝1，見本檔檔尾）
        if(Speed<=0)
            Speed=10;
        MOT[MTestZ1].Position+=Speed;
        MOT[MTestZ2].Position+=Speed;
        MOT[MTestY1].Position-=Speed;
        if(USE_INDEX_ARM_AXES==IndexArm_4_Axis)
            MOT[MTestY2].Position+=Speed;

        MOT[MTestY1].TargetPosition=Prod.TestY1_Front;                          //Steven 20210827 : 修改顯示Galil的Target / Current
        if(USE_INDEX_ARM_AXES==IndexArm_4_Axis)
            MOT[MTestY2].TargetPosition=Prod.TestY2_Rear;
        MOT[MTestZ1].TargetPosition=0;
        MOT[MTestZ2].TargetPosition=0;

        bool bY2PosSafe=(USE_INDEX_ARM_AXES==IndexArm_4_Axis)?(MOT[MTestY2].Position>=Prod.TestY2_Rear):true;  //JimmyChiu 20220708 : add Index Arm Axis
        if(MOT[MTestZ1].Position>=0 || MOT[MTestZ2].Position >=0 ||
           MOT[MTestY1].Position<=Prod.TestY1_Front ||
           bY2PosSafe)
        {
            IndexStatus=Z1_Z2_Normal;
            MOT[MTestY1].Position=Prod.TestY1_Front;
            if(USE_INDEX_ARM_AXES==IndexArm_4_Axis)
                MOT[MTestY2].Position=Prod.TestY2_Rear;
            MOT[MTestZ1].Position=0;
            MOT[MTestZ2].Position=0;
            fOffSet->bEnterSpecialOffset=false;
            return true;
        }
        return false;
    }
    return false;
}
//==============================================================================
//----------------------------------------------------------------------------
bool CheckIndexArmPosInRange(int iNowPos, int iMin, int iMax)
{
    int iTemp;
    if(iMin>iMax)
    {
        iTemp=iMax;
        iMax=iMin;
        iMin=iTemp;
    }

    if(iMin<=iNowPos &&                                                         //JerryYang 20230309 : 修正index arm移動保護
       iNowPos<=iMax)
    {
        return true;
    }
    else
    {
        return false;
    }
}

// =============================================================================
//  AI(W906-SIMSTEP) 20260924: 模擬 Galil 分支的步進倍率（分支 t4-c2-sim，隨 T4-C2 一起給使用者裁決）
//
//  上面各個「沒有 Galil 卡」的 else 分支是 golden 自己的模擬：每次呼叫把 Position 推進 Speed（或成員 speed）個 count。
//  golden 的 MainProc 約 1 ms 一次（tools/wb_serve.cpp kServeTickMs 的註解，RULINGS_20260917 B13），所以模擬軸每秒走上萬 count；
//  移植樹 B13 定案 500 ms 一拍，同一段碼每秒只走 1/500 ⇒ T4-C2 把真實教導點位推進 Prod 之後，模擬歸零的
//  GalilTwoY_Move（ProcessMotorHome case 1310）要走幾十分鐘，而 1310 約 30 秒就重頭再來 ⇒ 永遠歸零不完（20260924 gdb 實測）。
//  補償＝把每拍步進乘上 500，回到 golden 的牆鐘速度；每個分支原本就有「超過目標就夾回目標」的比較，不會衝過頭。
//  只在 SOFT_SIMULTE 建置生效：出貨組態回 1 —— 沒有卡的出貨組態（例如這台筆電跑 gate）行為與 golden 相同，
//  有卡的真機根本不走這些 else 分支。
// =============================================================================
int W906_SimGalilStepScale()
{
#ifdef SOFT_SIMULTE
    return 500;
#else
    return 1;
#endif
}

// =============================================================================
//  AI(W906-INDEXZ-1203) 20260929: the Index Z route seam -- Motor/GaliRoute.h.
//
//  RULINGS_20260929 section 5 (D1 = A, D3 = B, D4), RULINGS_20260928
//  section 6, RULINGS_20260926 section 27 R66-GALI = A. Design:
//  D:\HT9045\backup\night_tools_20260928\INDEXZ_1203_DESIGN_20260929.md
//  AI(W906-INDEXZ) 20260930: INBOX 113 redo -- D2 OFF (MachineType.h EOF), D1 applied by the installer
//  (W906_GaliRouteDisableAbsentIndexAxes below, review #7), the foreign-stop bookkeeping below, and
//  Gali_Two_ZAxis_Move's Z2 read (:3853, review #4).
//
//  Uses in this file (all same-line, no line above moved):
//    :574   the include
//    :1551  Gali_Command no-card branch -> W906_GaliRouteCommand (after golden's
//           own "ST" MovFlag/bZ1Z2Exute clearing; a real card never gets there)
//    :1622 :1676 :1758 :1957 :2151 :2293 :3638 :3850
//           the per-axis card tests: `bGali_CardInstall==true` ->
//           `(bGali_CardInstall==true || W906_GaliRouteOwns(Mot_Name))`
//    :4687  Gali_SingalHome -> W906_GaliRoutedSingalHome (below)
//    :4971  Gali_FindZPhase -> W906_GaliRoutedFindZPhase (below)
//  Not touched on purpose: :1422 (Gali_Command's own card branch), the
//  receiver-Y1 vector / pair functions (Y1 is disabled on HT9050, D1), and
//  Gali_MotHome / Gali_MotHomeFindZ (reached only from the delegated home).
//  With no route installed every one of them is golden, byte for byte.
// =============================================================================
static const TGaliRoute* s_w906GaliRoute = 0;   // POD, constant-initialised (no static-init order issue)

void W906_SetGaliRoute(const TGaliRoute* r)
{
    s_w906GaliRoute = r;
}

const TGaliRoute* W906_GaliRoute()
{
    return s_w906GaliRoute;
}

bool W906_GaliRouteOwns(int motName)
{
    return s_w906GaliRoute != 0 && s_w906GaliRoute->owner >= 0 && s_w906GaliRoute->owner == motName;
}

bool W906_GaliRouteCommand(int receiver, const char* data, long* reply)
{
    if (s_w906GaliRoute == 0 || s_w906GaliRoute->command == 0)
        return false;
    return s_w906GaliRoute->command(receiver, data, reply);
}

// AI(W906-INDEXZ) 20260930: D1 = A at INSTALL time (review #7; was cinitial.cpp:3874/:3956 at construction, which
//   changed an HT9050 ship build even when no route got installed). Motor/GaliRoute.h has the contract. The object
//   stays a TMyGALILMotor (cinitial.cpp's Galil arm built it); what construction with Enable=false would have skipped
//   is `InitMotor(iAdder)` (cinitial.cpp:4105-4106), and TMyGALILMotor::InitMotor is `return true;` (:1087-1090).
int W906_GaliRouteDisableAbsentIndexAxes(bool y1InTable, bool z2InTable, bool y2InTable)
{
    const int  axes[3] = { MTestY1, MTestZ2, MTestY2 };
    const bool keep[3] = { y1InTable, z2InTable, y2InTable };
    int n = 0;
    for (int k = 0; k < 3; ++k) {
        TMyMotor& m = MOT[axes[k]];
        if (keep[k] || W906_GaliRouteOwns(axes[k]) || m.Motor == NULL || m.Motor->Enable == false)
            continue;
        m.Motor->Enable = false;
        ++n;
    }
    return n;
}

// AI(W906-INDEXZ) 20260930: golden "ST"'s MovFlag clearing (golden :577-592) for a stop of the routed axis that
//   bypassed Gali_Command -- Motor/GaliRoute.h has the contract (INBOX 112 review HIGH-1 rule).
//AI(W906-INDEXZ-1203) 20260930: review round 2 D2 -- the WHOLE of golden "ST"'s bookkeeping now (golden :577-592, the same
//  lines as Gali_Command's no-card "ST" branch above), not just the routed axis's MovFlag: golden's operator stop on an index
//  axis is Gali_Command("ST") (uMotorTest.cpp:851-857 / 901-907, uteach.cpp:1270 / 2192), which clears all four MovFlag and
//  bZ1Z2Exute. motName only has to be the routed axis (no route = nothing).
bool W906_GaliRouteForeignStopBookkeeping(int motName)
{
    if (!W906_GaliRouteOwns(motName))
        return false;
    MOT[MTestY1].MovFlag=false;                                                 // golden :579-590
    if(USE_INDEX_ARM_AXES==IndexArm_4_Axis)
    {
        MOT[MTestY2].MovFlag=false;
    }
    MOT[MTestZ1].MovFlag=false;
    MOT[MTestZ2].MovFlag=false;

    MOT[MTestY1].bZ1Z2Exute=false;                                              //kevin 20110630
    if(USE_INDEX_ARM_AXES==IndexArm_4_Axis)
    {
        MOT[MTestY2].bZ1Z2Exute=false;                                          //kevin 20110630
    }
    return true;
}

// -----------------------------------------------------------------------------
//  golden TMyMotor::Gali_SingalHome (golden Motor/myGALILmotor.cpp:3589-3850),
//  for the routed axis. The caller already ran golden's preamble (Motor!=NULL,
//  CheckIsSafeDoorOpen, both Enable==false exits, Gali_ScanMotStatus()).
//  Kept: the task numbers (1, 5, 8, 200, 250, 400, 450, 500 -> 900), the MG_BG
//  waits, the servo check at 8 (off -> Task=1, return false), MovFlag=true at
//  200, the 400 flags (bScanFlag / MovFlag / GaliSofDelayCount / HomeFlag=1,
//  LastHomePos) and CheckTestZ1, the move to TestZ1_Safe at PHomeHighSpeed
//  (routed Gali_MotMove) and the D63 flags at 500, `return true` once.
//  Dropped (Galil-only, no 1203 counterpart): DP/DE=0 at 1 and 400 (the DS402
//  drive owns its origin), the sensor-jog dance 10..170 incl. the "Index Z
//  sensor fault" WAR03502 branch, Gali_MotHome (HM) / Gali_MotHomeFindZ (FI),
//  MG_SC==10 and the 100 ms DelayCT waits between them. The drive's home
//  (EastSun's DS402 Acm_AxHome 124/128, or the card-side MODE12 for another
//  drive -- the Motor Test page's two starts) takes their place at 200/250.
// -----------------------------------------------------------------------------
bool W906_GaliRoutedSingalHome(TMyMotor& m)
{
    const TGaliRoute* r = s_w906GaliRoute;
    bool checkflag = false;
    int  TestZSafePos = Prod.TestZ1_Safe;                                       //kevin 20160905
    int& Task = m.iGali_SingalHomeTask;
    switch(Task)
    {
        case 1:                                                                 // golden :3648-3654
            if(m.Gali_Command("MG_BGy", "Gali_SingalHome_1")==0)                //馬達目前是否移動? 0:移動完成
                Task=5;
            break;
        case 5:                                                                 // golden :3655-3660
            if(m.Gali_Command("MG_BGy", "Gali_SingalHome_5")==0)                //馬達目前是否移動 0:移動完成
                Task=8;
            break;
        case 8:                                                                 // golden :3661-3672 (Servo On -> 10; here 200)
            m.Gali_ScanMotStatusTIMO();                                         //取得目前I/O狀態
            if(m.Led[iServoOn]==true)
            {
                Task=200;
            }
            else
            {
                Task=1;
                return false;
            }
            break;
        case 200:                                                               // golden :3763-3770 Gali_MotHome -> the drive's home
            if(r==0 || r->homeStart==0 ||
               r->homeStart(m.Mot_Name, m.Motor->HomeDirection, m.Motor->PHomeHighSpeed, m.Motor->PHomeLowSpeed,
                            m.Motor->GetAccDataBase(), m.Motor->GetDecDataBase())!=0)
            {
                // refused: the route has marked the axis failed; 250 reports it
            }
            m.MovFlag=true;
            Task=250;                                                           // golden sets 250 after Gali_MotHome too
            break;
        case 250:                                                               // golden 250/300/350 -> the route's home state
        {
            const int p=(r==0 || r->homePoll==0) ? -1 : r->homePoll(m.Mot_Name);
            if(p==1)
            {
                Task=400;
            }
            else if(p<0)
            {
                m.Gali_ScanMotStatusTIMO();                                     // "TI" = alarm, MovFlag true -> golden's latch: Gali_MotorAlarm + ALM_MOTOR_MOVE
                Task=1;
            }
            break;
        }
        case 400:                                                               // golden :3797-3823
            m.Motor->LastHomePos=m.Gali_ReadEncoderPos();                       //讀取目前Encoder
            m.bScanFlag=false;
            m.MovFlag=false;
            m.GaliSofDelayCount=0;
            m.HomeFlag=1;
            if(m.Alias=="MTestZ1" || m.Alias=="MTestZ2")                        //Richard 20221206 : 開會討論後決定先回復為原先模式
            {
                if(m.Alias=="MTestZ1")
                    checkflag=CheckTestZ1("Gali_SingalHome400");
                if(m.Alias=="MTestZ2")
                    checkflag=CheckTestZ2("Gali_SingalHome400");
                if(checkflag)
                    Task=450;
                else
                    Task=500;
            }
            else
            {
                Task=500;
            }
            break;
        case 450:                                                               // golden :3824-3835
            TestZSafePos=10;
            if(m.Alias=="MTestZ1" || m.Alias=="MTestZ2")
            {
                TestZSafePos=Prod.TestZ1_Safe;                                  //kevin 20160905
            }

            if(m.Gali_MotMove(TestZSafePos, m.Motor->PHomeHighSpeed, "Gali_SingalHome_450"))
            {
                Task=500;
            }
            break;
        case 500:                                                               // golden :3836-3847
            Task=900;
            if((CUSTOMER_CODE==CC_ASE_KaohSiung ||
                CosFunction.bHiSiliconFunction==true) &&
               IniConfig.bD63CheckIndexZHomeToZPhaseDistanceRange==true &&
               (m.Alias=="MTestZ1" || m.Alias=="MTestZ2"))                      //kevin 20170605 (Steven) add
            {
                m.MovFlag=false;
                m.GaliSofDelayCount=0;
                m.HomeFlag=1;
            }
            return true;
    }
    return false;
}

// golden TMyMotor::Gali_FindZPhase (golden :3852-4385) is the D63 Z-phase
// distance check: Galil HM/FI/MG_SC only. The routed axis refuses -- false,
// forever, never a fake success (HT9050 must keep D63 = 0; README_MACHINE_AI).
bool W906_GaliRoutedFindZPhase(TMyMotor& m)
{
    static bool s_logged=false;
    if(!s_logged)
    {
        s_logged=true;
        std::printf("index Z -> 1203: Gali_FindZPhase(%s) is Galil-only (D63 Z-phase search) -- refused on the 1203 route (returns false; keep D63=0)\n",
                    m.Alias.c_str());
    }
    return false;
}
//AI(W906-INDEX-ABSENT) 20261004: NB2-1 dispatch 7 C1 (Motor/GaliRoute.h). "Absent" is NOT Motor->Enable alone: the Index axes are
//  built Enable=true (cinitial.cpp InitialMotorParameter) and only the Galil route's install turns the absent ones off
//  (W906_GaliRouteDisableAbsentIndexAxes) -- so no route = golden on every machine.
bool W906_GaliRouteAbsentAxis(int motName)
{
    if (W906_GaliRoute() == 0) return false;
    if (motName != MTestY1 && motName != MTestZ2 && motName != MTestY2) return false;
    if (W906_GaliRouteOwns(motName)) return false;
    HTMotor* m = MOT[motName].Motor;
    return m != NULL && m->Enable == false;
}
