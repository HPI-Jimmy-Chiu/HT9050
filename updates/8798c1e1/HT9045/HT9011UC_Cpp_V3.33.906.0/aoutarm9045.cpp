// =============================================================================
//  aoutarm9045.cpp  --  Out-Arm ENGINE  (W6.2c translation)
//
//  Faithful translation of golden  aoutarm9045.cpp (4753 lines, BCB6, Big5/cp950).
//  Translator: AI(W6.2c-OUTARM) 20260626
//
//  ROLE: this is the out-arm ENGINE -- the DIRECT mirror of the in-arm engine
//  (ainarm9045.cpp).  It owns the central place-IC-to-Unloader/Auto cycle SM, the
//  iInArmType dispatch ladders into 26 site-variant modules, the pure pitch/offset
//  geometry, and several auxiliary task SMs.
//
//  WAVE SCOPE (W6.2c) -- what is ACTIVE vs GATED (see each gate's golden cite):
//
//   ACTIVE (faithful, verbatim formulas / branches / cursor flow / fall-throughs):
//     * pure geometry/pitch helpers: GetOutArmPitch_9045, GetOutArmPitch2_9045,
//       GetOutArmPitchY_9045, GetOutArmPitchX_9045, GetOutArmPitchXMM_9045,
//       GetOutArmPitchYMM_9045, BinBoxShiftY.  All use ChangeToFloatNonPcnt
//       (MachineType.h, zero-guarded) -> EXACT numeric semantics incl final
//       (int)r truncation.
//     * the 2 iInArmType dispatch ladders DoOutArm_9045 / DoPickFromShuttle_9045
//       (pre-dispatch guards + if-ladder STRUCTURE; the DoPickFromShuttle final
//       else Program-Error path stays the live offline behavior; DoOutArm_9045
//       has NO final else in golden -> reproduced verbatim).
//     * the 4 pumpable task SMs (switch skeleton + cursor flow + fall-throughs
//       verbatim): DoOutArmAdditionalFunction, DoOutArmPlaceToAuto_9045,
//       DoOutArmAfterPlaceToAuto, DoOutArmIonFanGiveWay.  Inner hardware /
//       cross-module / Magazine / AOI / ATK bodies are gated behind active stubs.
//
//   GATED (compiling ACTIVE stub right after; documented at each site):
//     * the ~26 per-variant DoOutArm_9045_<layout>() / DoPickFromShuttle_9045_
//       <layout>(iSht) / _All_1Picker call-arms (`#if 0 // TODO(W6.2b variants)`)
//       -- variant bodies live in aoutarm9045_<layout>.cpp, out of scope; active
//       stubs (void no-op / return false) declared right after so the ladders link.
//     * the hardware/cross-module/UI surface (MOT[]/sensor/Magazine/AOI/Rotate/
//       SortingBinTray/SetTrayBinData/...) the SMs call -- `// TODO(W7)` ACTIVE
//       stubs (return false/0/no-op/sane default) so the engine LINKS and the SMs
//       pump.  Each stub cites golden file:line.
//
//  CORRECTION to the original task brief: the engine dispatches on the SHARED
//  global `int iInArmType` (golden cmydef.h), NOT an `iOutArmType` (no such
//  symbol exists in this file) -- exactly like the in-arm engine.
//
//  VCL/Borland conversions: #pragma hdrstop / #pragma package(smart_init) dropped
//  (MinGW); AnsiString (1-based) + AnsiString::sprintf via vclcompat (kept
//  verbatim); __FUNC__ via aArmHeader.h W6.0 shim; TQPF_Timer via myTimer.h.
//  SOFT_SIMULTE is NOT defined -> #ifndef SOFT_SIMULTE paths stay active;
//  #ifdef SOFT_SIMULTE paths are kept VERBATIM but inert.  The TMyQASample type
//  (golden aoutarm.h:155, __fastcall ctor) is out of scope -> NOT reproduced.
//
//  Big5: every Chinese comment decoded via cp950 and preserved as UTF-8.
//  Final gate: ZERO U+FFFD.
// =============================================================================
#include "MachineDefine.h"

#include "aoutarm9045.h"
#include "aArmHeader.h"             // __FUNC__ shim, RecordProcess, ShowErrorMessage, etc.

#include "cprod.h"                 // Prod / TestIF / TestIF_File / OutArmOffSet / ArmSpeed
#include "Motor/mymotor.h"         // MOT[], TMyMotor, TTrayMotor
#include "csystem.h"               // OutSHT1InRT / OutSHT2InRT / ShuttleHasIC / IndexHasIC
#include "cmydef.h"                // global scalar universe + IC consts + iInArmType
#include "cpublic.h"               // CosFunction
#include "common.h"
#include "aHotPlateSubstrate.h"    // OutArmSuck/FRCarryKit/BRCarryKit/OutArm2Suck, cursors, tRotate
#include "FormsFacade.h"           // fMain/fAGV offline stand-ins
#include "canary_support.h"        // LastSet / ShowMyMessage / ShowErrorMessage / RecordProcess / WhichAutoNeedTray
// AI(W906-PT-W4-integrate) 20260809: aoutarm.h is now included DIRECTLY, because the
//   eight file-scope stand-in DEFINITIONS this TU used to carry (iWhichAuto /
//   bOutArmXOverLimit / bOverTray / DoPlaceToAutoDelay / CheckOutSuckICFallDown /
//   SendDataToASE / ErrorBinBoxDetect / PorcessJAM0201OutArmPickUpErrorSkip) were also
//   serving as this TU's DECLARATIONS -- retiring them (aoutarm.cpp landed in PT-W4 with
//   golden's real bodies) left the call sites with no declaration in scope.
//   aoutarm.h declares every one of them (:81 / :87 / :95 / :91 / :153 / :166 / :187 /
//   :170) and that header shipped with aoutarm.cpp in the same wave.
//   WHY DIRECTLY, and not by ungating: golden reaches aoutarm.h through aArmHeader.h,
//   but this tree's aArmHeader.h:61 sits inside the `#if 0` block opened at :23 that
//   also pulls ~40 per-site arm headers -- ungating it is a W7 decision, not this
//   integrate step's.  Same include, narrowest possible route.
#include "aoutarm.h"               // golden reaches this via aArmHeader.h:61 (still gated here)
#include "mysensor.h"              // Sen[] (SnRKManualStep) for DoOutArmSuckPreOn below   //AI(W906-AMB-L2) 20260930: INBOX 111b item 1 -- the part include at :2401 comes too late for :1033; on the blank line so no line number moves
//==============================================================================
//  golden file globals (verbatim)  -- golden aoutarm9045.cpp:44/46/251/285/2133-2136/3862/4607
//==============================================================================
bool bOutArmCheckDestroyACT[MAX_ARM_Row][MAX_ARM_Col]={{false, false, false, false}, {false, false, false, false}};   // golden :44

int iBinBoxShiftY=0;                                                            //jou 2012-12-11 support Bin Box  -- golden :46

bool bCheckOutArmDestroyActiveFinish[MAX_ARM_Row][MAX_ARM_Col]={{true, true, true, true}, {true, true, true, true}};  // golden :251
bool bCheckOutArmDestroyActive[MAX_ARM_Row][MAX_ARM_Col]={{false,false,false,false},{false,false,false,false}};       // golden :285

bool bOutRotator;                                                               // golden :2133
bool bDoAOI;                                                                    // golden :2134
bool bDoFixAI;                                                                  //Sam 20211220 : 整合 Fix AI AOI  -- golden :2135
int  iOutArmAdditionalFunctionTask;                                             // golden :2136

int  iOutPickFromSht=0;                                                         // golden :3862
int  iOutPlaceToAuto=1;   int  iOutPlaceToRotate=2;   // golden :3863 //AI(W906-W188) 20261010 (NB2-1): golden 913 aoutarm9045.cpp:4169 (ht9045-v899 20260811: place-to-rotator; iOutPlaceToAuto would index s6TrayName[MOutRotateKit=42] past eTrayCount)

int  iOutArmIonFanGiveWayTask;                                                  // golden :4607

//==============================================================================
//==  GATED externs: the ~26 per-layout dispatch callees + the hardware /
//==  cross-module surface the ladders + SMs call.  Golden DECLARES these via the
//==  per-layout headers (aoutarm9045_<layout>.h) / app headers (main.h / acatchtray.h
//==  / Magazine.h / fAOI.h / aRotateKIT_Out.h / cSortCT.h / ...) -- none in scope
//==  this wave.  Below: the real golden prototypes behind `#if 0`, then an ACTIVE
//==  compiling stub block (mirror ainarm9045.cpp:887).
//==============================================================================
// --- W6.2c-OUT batch: 6 site variants made ACTIVE -- their real per-layout
//     externs un-gated here (translated bodies live in aoutarm9045_<layout>.cpp,
//     registered in CMake lib ht9045_sm); their static no-op stubs removed below.
void DoOutArm_9045_1x1_1();      void DoOutArm_9045_1x2_2();     void DoOutArm_9045_1x2_4();
void DoOutArm_9045_1x3_2_14();   void DoOutArm_9045_1x3_4();     void DoOutArm_9045_1x4_2();
bool DoPickFromShuttle_9045_1x1_1(int);     bool DoPickFromShuttle_9045_1x2_2(int);
bool DoPickFromShuttle_9045_1x2_4(int);     bool DoPickFromShuttle_9045_1x3_2_14(int);
bool DoPickFromShuttle_9045_1x3_4(int);     bool DoPickFromShuttle_9045_1x4_2(int);
// --- W6.2c-OUT batch-2: 7 more variants made ACTIVE (LIVE) -- externs un-gated here.
void DoOutArm_9045_1x4_4S();  void DoOutArm_9045_1x4_4();  void DoOutArm_9045_1x4_8();
void DoOutArm_9045_2x1_2();   void DoOutArm_9045_2x2_4();  void DoOutArm_9045_2x2_8();
void DoOutArm_9045_2x3_6_14();
bool DoPickFromShuttle_9045_1x4_4S(int); bool DoPickFromShuttle_9045_1x4_4(int);
bool DoPickFromShuttle_9045_1x4_8(int);  bool DoPickFromShuttle_9045_2x1_2(int);
bool DoPickFromShuttle_9045_2x2_4(int);  bool DoPickFromShuttle_9045_2x2_8(int);
bool DoPickFromShuttle_9045_2x3_6_14(int);

// --- W6.2c-OUT batch-3 (FINAL): 7 last variants made ACTIVE (LIVE) -- externs
//     un-gated here (translated bodies in aoutarm9045_<layout>.cpp). This empties
//     both the gated-extern and static-stub blocks; all out-arm variants now real.
//     NOTE: All_1Picker is DoOutArm-ONLY (picker-gated dispatch); it has NO
//     DoPickFromShuttle_9045_All_1Picker extern -- matches the dispatch ladder.
void DoOutArm_9045_All_1Picker();
void DoOutArm_9045_2x3_6();      void DoOutArm_9045_2x4_4();
void DoOutArm_9045_2x4_8();      void DoOutArm_9045_2x5_8();     void DoOutArm_9045_2x6_8();
void DoOutArm_9045_2x8_8();
bool DoPickFromShuttle_9045_2x3_6(int);
bool DoPickFromShuttle_9045_2x4_4(int);     bool DoPickFromShuttle_9045_2x4_8(int);
bool DoPickFromShuttle_9045_2x5_8(int);     bool DoPickFromShuttle_9045_2x6_8(int);
bool DoPickFromShuttle_9045_2x8_8(int);

//==============================================================================
//==  GATED hardware / cross-module / UI surface (// TODO(W7)).  ACTIVE stubs so
//==  the kept SMs LINK and pump.  Each cites golden file:line + the offline
//==  rationale.  These have no translated home this wave (MOT[]/sensor/Magazine/
//==  AOI/Rotate/SortingBinTray/SetTrayBinData/cinitial/fProductionInfo/...).
//==============================================================================
//  (i) functions ALREADY declared in shared headers (csystem.h / cprod.h) but
//      with NO translated body anywhere -> DEFINE them here (non-static, matching
//      the header prototype) as offline-safe stubs.
//      CORRECTION (W6.2c verify): ReversionEmptyPoint / EnableFix3UseCylinder are
//      DECLARED in cprod.h but their cprod.cpp bodies (golden cprod.cpp:1452/1484)
//      sit inside cprod.cpp's `#if 0 // TODO(W6)` block (opened cprod.cpp:184) --
//      so they are NOT in ht9045_globals.  DEFINE offline-safe stubs here matching
//      the cprod.h prototype, faithful to the golden offline end-state.  When
//      cprod.cpp later ungates them, remove these two (will become duplicates).
// AI(W906-GA1-B2-integrate) 20260804: the two offline stand-ins that lived here
// (EnableFix3UseCylinder / ReversionEmptyPoint) are RETIRED -- cprod.cpp's real
// bodies were ungated by GA-1-B2, exactly the removal this file's own
// CORRECTION note anticipated. Declarations still come from cprod.h.
#if 0   // PT-W5c RETIRED (CheckOutArmSuckICFallDown)
//AI(ht9045-v906) 20260809: PT-W5c -- RETIRED. The real faithful body now lives in csystem.cpp (its golden home); keeping this stand-in is a multiple-definition error, measured in build_0809_w5c. Same convention as csystem_shims.cpp:165.
bool CheckOutArmSuckICFallDown()                   { return false; }  // golden csystem.h:84 -- offline: no fall-down
#endif
// AI(W906-PT-W4-integrate) 20260809: 8 STAND-IN DEFINITION(S) RETIRED FROM HERE.
//   aoutarm.cpp (golden's own home for all of them) landed in wave PT-W4 and is
//   registered in ht9045_sm, so both definitions were in libht9045_sm.a and every
//   executable linking it failed with `multiple definition of ...`. The linker named
//   each one, which is also the proof the signatures match exactly -- a decorated-name
//   collision cannot happen otherwise.
//   Retired here: CheckOutSuckICFallDown, SendDataToASE, ErrorBinBoxDetect, PorcessJAM0201OutArmPickUpErrorSkip, iWhichAuto, bOutArmXOverLimit, bOverTray, DoPlaceToAutoDelay
//   BEHAVIOUR: these were offline defaults (return true/false/0/no-op); the real bodies
//   run golden's actual logic, so out-arm paths that used to short-circuit now execute.
//   That is the point of the wave, and it is why this wave was measured on its own.
#if 0   // PT-W5c RETIRED (DetectAutoTray)
//AI(ht9045-v906) 20260809: PT-W5c -- RETIRED. The real faithful body now lives in csystem.cpp (its golden home); keeping this stand-in is a multiple-definition error, measured in build_0809_w5c. Same convention as csystem_shims.cpp:165.
bool DetectAutoTray(int,int*)                      { return true;  }  // golden csystem.h:232 -- offline: tray detected
#endif
#if 0   // PT-W5f RETIRED (MagazineBufferIsEmpty)
//AI(ht9045-v906) 20260810: PT-W5f -- RETIRED. csystem.cpp wave 2 landed the real faithful body in its golden home; keeping this stand-in is a multiple-definition error. Same convention as csystem_shims.cpp:165. *** BEHAVIOUR DELTA, NOT NEUTRAL: the stub answered "buffer empty" unconditionally, so retiring it lets the real check actually run and it can now REFUSE. ***
bool MagazineBufferIsEmpty(int)                    { return true;  }  // golden csystem.h:290 -- no magazine present
#endif
#if 0   // PT-W5f RETIRED (WhichMagazineBufferIsFull)
//AI(ht9045-v906) 20260810: PT-W5f -- RETIRED. csystem.cpp wave 2 landed the real faithful body in its golden home; keeping this stand-in is a multiple-definition error. Same convention as csystem_shims.cpp:165.
int  WhichMagazineBufferIsFull()                   { return -1;    }  // golden csystem.h:289 -- no magazine full
#endif
// -- W6.2c-OUT ADD: out-arm engine cross-module surface the 6 site-variant
//    DoOutArm/DoPickFromShuttle SMs call (golden aoutarm.h:124 / aoutarm9045.cpp
//    Steven 20161214).  No translated home yet -> offline-safe non-static defs so
//    the variant TUs link.  TODO(W7): wire to the real bin-box / JAM0201 skip SMs.
//  (ii) functions with NO target home at all -> file-local static stubs.
//
//  AI(W906-PT-W4-integrate) 20260809: NINE OF THESE STATICS ARE RETIRED, and the
//  compiler is what forced the issue rather than a judgement call. `static` gave them
//  internal linkage, so while aoutarm.cpp's real bodies were absent they linked
//  cleanly AND SILENTLY SHADOWED anything with the same name -- the same invisible
//  defect class as a test defining its own empty copy. The moment this TU started
//  including aoutarm.h (see the include block above, needed because eight retired
//  stand-in definitions were also serving as declarations), every one of them became
//  `declared 'extern' and later 'static'`, a hard error, plus one ambiguous overload.
//  Retired, with the aoutarm.h declaration that now governs each:
//      MoveOutArmToAutoSafe          aoutarm.h:115
//      MoveOutArm2XYToDecayTeach     aoutarm.h:106
//      MoveOutArmXY_ToFix_Tray_Full  aoutarm.h:104  (no-arg static made the calls at
//                                    :1253/:1289 ambiguous against the real
//                                    `bool bMoveY=false` form -- they now reach it)
//      IfUseOnebyOne                 aoutarm.h:164
//      InitOutArmTask                aoutarm.h:97
//      IsCatchTrayReadySupplyNewTray aoutarm.h:127
//      CheckOutArmCleanOut           aoutarm.h:121
//      SetOutArmHome                 aoutarm.h:160
//      CheckUseFixBinBoxFunction     aoutarm.h:183
//  BEHAVIOUR: all nine were offline defaults (true / false / no-op / echo the Task
//  argument back). This TU's out-arm SMs now call golden's real bodies in aoutarm.cpp,
//  so paths that used to short-circuit now execute. That is the wave's whole point.
//  KEPT (aoutarm.h does not declare them, so no clash and no real body to reach):
//  MoveOutArmZToPlateSafe / DoOutArmPlaceToAuto / DoFix3FullTray /
//  InitialDoPickFromMagazineBuffer -- checked name by name, not assumed.
bool MoveOutArmZToPlateSafe();   // AI(W906-ARM1) 20260927: 以前是本 TU 的 static 樁（回 true），把 asortarm.cpp 已照翻的真本體（宣告 asortarm.h:179）遮住了 —— NB2 R105 ⑧
//  OutArmContinuousMove_9045 is REAL-declared in Motor/mymotor.h (8 params,
//  bLoader=false default) + sim-defined in mymotor.cpp (returns false) -> use it.
bool DoOutArmPlaceToAuto(int iToWhichAuto);   // AI(W906-ARM3) 20260927: 以前是 static 樁（回 true ⇒ 放料立即回成功）—— 真本體照 golden :2525-3075 翻在本檔尾端（NB2 R105 ⑥）
bool DoFix3FullTray();   // AI(W906-ARM4) 20260927: 以前是本 TU 的 static 樁（回 true），把 SortingBinTray/SortingBinTray.cpp:753 已照翻的真本體（宣告 SortingBinTray.h:79）遮住了
static void InitialDoPickFromMagazineBuffer()      {}                 // golden Magazine.h -- no magazine present
//AI(W906-PT-W1-integrate) 20260807: TODO(W7) -- the REAL engine behind the next
//  five stubs now exists: SortingBinTray/SortingBinTray.cpp landed in wave PT-W1
//  and is compiled into ht9045_sm.  These file-local statics still shadow it, so
//  that unit currently has zero callers.  Retiring them means deleting these five
//  lines AND ungating aArmHeader.h:23-98's `#include "SortingBinTray.h"`, which
//  pulls the whole arm god-header -- a W7 decision, deliberately not taken in the
//  wave's integrate step.  The same note is in SortingBinTray.cpp's banner.
static void DoSortingBinTray(int)                  {}                 // golden cSortCT.h -- nothing to sort
static bool DoSortingBinTray()                     { return true;  }  // golden cSortCT.h -- sort done
static void SortingBinTray_SetMotorPosData()       {}                 // golden cSortCT.h
static void SortingBinTray_SetTrayData(int)        {}                 // golden cSortCT.h

//  (iii) golden file globals with NO target home -> offline definitions here.
int        iOutRotateFinish=0;          //kevin 20130524        -- golden (out-rotate handshake)

//==============================================================================
//==  ACTIVE: BinBoxShiftY  -- golden aoutarm9045.cpp:47 (trivial)
//==============================================================================
void BinBoxShiftY()                                                             //jou 2012-12-11 support Bin Box
{
    iBinBoxShiftY=0;
}

//==============================================================================
//==  ACTIVE: pure geometry / pitch helpers (faithful, verbatim formulas)
//==============================================================================
//------------------------------------------------------------------------------
int GetOutArmPitch_9045(int w)                                                  // golden :157
{
    double m, r;
    m=(double)ChangeToFloatNonPcnt((double)((Prod.iOutArmX120Pitch-Prod.iOutArmX40Pitch)), (double)(iPitch_Max_minus_Min));                                     //Isaac 20171204 (Steven) : Xpitch40->50mm, 新增150mm的Teaching點位，內差法公式計算
    r=(double)Prod.iOutArmX40Pitch+m*((double)w-double(iXpitchMinX3));
    return (int)r;
}
//------------------------------------------------------------------------------
int GetOutArmPitch2_9045(int w)                                                 //Steven 20131002 : XY變距  -- golden :165
{
    double m, r;                                                                //Jerryyang 20180205 float -> double
    m=(double)ChangeToFloatNonPcnt((double)((Prod.iOutArmX120Pitch2-Prod.iOutArmX40Pitch2)), (double)(iPitch_Max_minus_Min));                                   //Isaac 20171204 (Steven) : Xpitch40->50mm     //Steven 20200327 : 修正out arm x-pitch的點位錯誤
    r=(double)Prod.iOutArmX40Pitch2+m*((double)w-double(iXpitchMinX3));
    return (int)r;
}
//------------------------------------------------------------------------------
int GetOutArmPitchY_9045(int w, int iOffsetPos)                                 //Steven 20131002 : XY變距  -- golden :173
{
    if(iOffsetPos>=0)                                                           //Jimmychiu 20240731 : for out arm offset value
        w+=OutArmOffSet[iOffsetPos]->GetVariableY();
    double m=0.0, r=0.0;
    m=ChangeToFloatNonPcnt((double)(Prod.iOutArmY60Pitch-Prod.iOutArmY15Pitch), (double)(6000-IN_OUT_ARM_Y_PITCH_MIN)); //Steven 20260505 : add zero-guard
    r=(double)Prod.iOutArmY15Pitch+m*((double)w-(double)IN_OUT_ARM_Y_PITCH_MIN);
    return r;
}
//------------------------------------------------------------------------------
int GetOutArmPitchX_9045(int w, int iX, int iOffsetPos)                         //Steven for HT1032  -- golden :183
{
    if(iOffsetPos>=0)                                                           //Jimmychiu 20240731 : for out arm offset value
        w+=OutArmOffSet[iOffsetPos]->GetXPitch(iX);
    double m=0.0, r=0.0;
    double dX120[4]={Prod.iOutArmX120Pitch, Prod.iOutArmX120Pitch2, Prod.iOutArmX120Pitch3, Prod.iOutArmX120Pitch4};
    double dX_40[4]={Prod.iOutArmX40Pitch,  Prod.iOutArmX40Pitch2,  Prod.iOutArmX40Pitch3,  Prod.iOutArmX40Pitch4};
    m=ChangeToFloatNonPcnt((double)(((double)(dX120[iX]-dX_40[iX]))), (double)(iPitch_Max_minus_Min));
    if(USE_PICKER_COUNT==ep16Picker)
        r=(double)dX_40[iX]+m*((double)w-double(iXpitchMinX7));
    else
        r=(double)dX_40[iX]+m*((double)w-double(iXpitchMinX3));
    return r;
}
//==============================================================================
int GetOutArmPitchXMM_9045(int iMotPulse)                                       // golden :3808
{
    double m=0.0, r=0.0;
    m=ChangeToFloatNonPcnt((double)(((double)(Prod.iOutArmX120Pitch-Prod.iOutArmX40Pitch))), (double)(iPitch_Max_minus_Min));
    r=(double)ChangeToFloatNonPcnt((double)((iMotPulse-Prod.iOutArmX40Pitch)), (double)(m));
    r+=double(iXpitchMinX3);
    return (int)r;
}
//==============================================================================
int GetOutArmPitchYMM_9045(int iMotPulse)                                       // golden :3817
{
    double m=0.0,r=0.0;
    if(USE_OUT_Y_IS_AUTO_PITCH==true)                                           //JerryYang 20251218 : IN/OUT ARM支援不同模組
    {
        m=ChangeToFloatNonPcnt((double)(Prod.iOutArmY60Pitch-Prod.iOutArmY15Pitch), (double)(6000-IN_OUT_ARM_Y_PITCH_MIN)); //Steven 20260505 : add zero-guard
        r=(double)ChangeToFloatNonPcnt((double)((iMotPulse-Prod.iOutArmY15Pitch)), (double)(m));
        r+=double(IN_OUT_ARM_Y_PITCH_MIN);
    }
    else
    {
        r=TestIF.iARM_Y_PITCH;
    }

    return (int)r;
}
//==============================================================================
//==  GetOutArmCellPos / GetOutArmToShtCellPos -- golden :3834 / :3848.  Keep the
//==  pitch-math STRUCTURE active; gate the MOT[].ReadPos() encoder reads (mirror
//==  in-arm GetInArmCellPos -- those reads have no offline home here).  Offline
//==  the encoders read 0 so the cell positions are pure pitch deltas.
//==============================================================================
void GetOutArmCellPos(int iRow, int iCol, int &Ypos, int &Xpos)                 // golden :3834
{
    int iPitchPulse=0, iPitchPulseY=0, iBaseX=0, iBaseY=0;
#if 1 // was: #if 0 -- opened AI(W906-POOL2) 20261008 (Ifor01): TODO(W7) GetOutArmCellPos encoders reason expired -- MOT[].ReadPos() is the live encoder/command read (the in-arm twin GetInArmCellPos reads it ungated, ainarm9045.cpp); offline the sim drivers return their position; golden :3836-3845
    iPitchPulse =MOT[MOutArmPitch ].ReadPos();
    iPitchPulseY=MOT[MOutArmPitchY].ReadPos();
    iBaseX      =MOT[MOutArmX].ReadPos();
    iBaseY      =MOT[MOutArmY].ReadPos();
#endif
    int iMovePitchX=GetOutArmPitchXMM_9045(iPitchPulse);
    int iMovePitchY=GetOutArmPitchYMM_9045(iPitchPulseY);

    if(USE_PICKER_COUNT==ep1Picker)
        iMovePitchX=0;
    else
        iMovePitchX/=3;

    Xpos=iBaseX+(iMovePitchX*(iCol-iOutArmXBase));
    Ypos=iBaseY-(iMovePitchY*(iRow-iOutArmYBase));
}
//==============================================================================
void GetOutArmToShtCellPos(int iRow, int iCol, int &Ypos, int &Xpos)            // golden :3848
{
    int iPitchPulse=0, iPitchPulseY=0, iBaseX=0, iBaseY=0;
#if 1 // was: #if 0 -- opened AI(W906-POOL2) 20261008 (Ifor01): TODO(W7) GetOutArmToShtCellPos encoders reason expired -- MOT[].ReadPos() is the live encoder/command read (the in-arm twin GetInArmCellPos reads it ungated, ainarm9045.cpp); offline the sim drivers return their position; golden :3850-3859
    iPitchPulse =MOT[MOutArmPitch ].ReadPos();
    iPitchPulseY=MOT[MOutArmPitchY].ReadPos();
    iBaseX      =MOT[MOutArmX].ReadPos();
    iBaseY      =MOT[MOutArmY].ReadPos();
#endif
    int iMovePitchX=GetOutArmPitchXMM_9045(iPitchPulse);
    int iMovePitchY=GetOutArmPitchYMM_9045(iPitchPulseY);

    if(USE_PICKER_COUNT==ep1Picker)
        iMovePitchX=0;
    else
        iMovePitchX/=3;

    Xpos=iBaseX+(iMovePitchX*(iCol*OutArmSuck.iPickStep-iOutArmXBase));
    Ypos=iBaseY-(iMovePitchY*(iRow-iOutArmYBase));
}

//==============================================================================
//==  GATED hardware helpers kept as ACTIVE compiling stubs (// TODO(W7)).  [AI(W906-FLOW-2) 20260928: superseded -- body live since 216f7bd8: the golden bodies below are compiled]
//==  Golden bodies are MOT[]/OutArmZSafe/ScanMotorStatus/sensor-bound.
//==============================================================================
// ===========================================================================
//  GOLDEN VERBATIM PAIR -- MoveOutArmToAutoSafe_9045()
//  GATED : golden aoutarm9045.cpp:55-155 (101 lines), inert reference text.
//  LIVE  : the MoveOutArmToAutoSafe_9045() body immediately AFTER the #endif below.  It is UNCHANGED by
//          this wave -- net behaviour change is ZERO.  [AI(W906-FLOW-2) 20260928: superseded -- body live since 216f7bd8: this banner describes the retired gate; the golden body is compiled (see :384)]
//  WHY   : the census scored this "translated" because a same-named LIVE body
//          existed, without comparing SIZE.  Golden's 101 lines were NOWHERE in
//          the tree -- lost text, not deferred behaviour.  Now the text EXISTS
//          and is auditable, so a later un-gate is mechanical.
//  NOTE  : golden's body is MOT[]/OutArmZSafe/SetOutArmHome/WAR0257 bound.
//  RULES : nothing inside the gate is fixed, renamed, reflowed or reindented;
//          it is golden's bytes.  Nothing it references had to be made to
//          exist -- no stub, declaration or header was added for it.
//  SHAPE : same pair shape as csystem.cpp MainProc / atester.cpp (PT-W6a/W6b)
//          golden-verbatim gates.
// ===========================================================================
// AI(W906-FLOW-2) 20260928: GATE G-PTW6c-MoveOutArmToAutoSafe_9045 RETIRED (was `#if 0 // GOLDEN VERBATIM`): the golden body below (golden aoutarm9045.cpp:55-155, re-verified identical) is compiled now; every dependency is live: OutArmZSafe (Motor/mymotor.cpp:6205), SetOutArmHome (aoutarm.cpp:3194), Prod.ZOutArmSafe (written cinitial.cpp:11554), OutArmSuck.Suck[][].iMotNo (cinitial.cpp:947-). The `return true` stand-in at :487-493 is commented out.
bool MoveOutArmToAutoSafe_9045()
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
                            MyDBIProcess("Motion", sData+AnsiString(" [MoveOutArmToAutoSafe_9045]"));   //AI(W906-FLOW-2) 20260928: golden :86 is the 3-arg MyDBIProcess("Motion", sData, "MoveOutArmToAutoSafe_9045"); this TU sees only the 2-arg sink (aHotPlateSubstrate.h:981) -- 3rd arg (DB function-name column) folded into the message, idiom of asortarm.cpp:2479 / ainarm2.cpp:4669
                        else
                            ShowMyMessage(sData, "", "MoveOutArmToAutoSafe_9045");
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
                if(iRetryFail<3)
                {
                    iRetryFail++;
                    SetOutArmHome();
                }
                else
                {
                    ShowErrorMessage("WAR0257", 0, MOutArmX, false, MOT[iAlarmSuck].NumberAlias);                       //出料手臂的吸嘴 %s 沒有在安全位置
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
//#endif // AI(W906-FLOW-2) 20260928: end of the now-compiled golden body (golden aoutarm9045.cpp:55-155)
//bool MoveOutArmToAutoSafe_9045()                                                // golden :55  AI(W906-FLOW-2) 20260928: retired stand-in
//{
//    // golden :57-154 walks OutArmSuck.iMotRow/iMotCol driving MOT[].MotorMove /
//    // OutArmZSafe / SetOutArmHome / ShowErrorMessage("WAR0257").  All MOT[]/
//    // alarm-bound with no offline home -> gated; offline the arm is "at safe".
//    return true;                                                                // TODO(W7) -- golden aoutarm9045.cpp:55 (MOT[]/OutArmZSafe motion)
//}
//-----------------------------------------------------------------------------
// ===========================================================================
//  GOLDEN VERBATIM PAIR -- GetOutArmZDownPos_9045()
//  GATED : golden aoutarm9045.cpp:198-249 (52 lines), inert reference text.
//  LIVE  : the GetOutArmZDownPos_9045() body immediately AFTER the #endif below.  It is UNCHANGED by
//          this wave -- net behaviour change is ZERO.  [AI(W906-FLOW-2) 20260928: superseded -- body live since 216f7bd8: this banner describes the retired gate; the golden body is compiled (see :511)]
//  WHY   : the census scored this "translated" because a same-named LIVE body
//          existed, without comparing SIZE.  Golden's 52 lines were NOWHERE in
//          the tree -- lost text, not deferred behaviour.  Now the text EXISTS
//          and is auditable, so a later un-gate is mechanical.
//  NOTE  : golden's body reads OutArmSuck.Suck[][].iMyRow/iMyCol + Prod.ZPlace[][].
//  RULES : nothing inside the gate is fixed, renamed, reflowed or reindented;
//          it is golden's bytes.  Nothing it references had to be made to
//          exist -- no stub, declaration or header was added for it.
//  SHAPE : same pair shape as csystem.cpp MainProc / atester.cpp (PT-W6a/W6b)
//          golden-verbatim gates.
// ===========================================================================
// AI(W906-FLOW-2) 20260928: GATE G-PTW6c-GetOutArmZDownPos_9045 RETIRED (was `#if 0 // GOLDEN VERBATIM`): the golden body below (golden aoutarm9045.cpp:198-249, re-verified identical) is compiled now; the stand-in reason ("bOutArmSuckActive has no offline home") is false: bOutArmSuckActive is defined aoutarm.cpp:597 and written live (aoutarm.cpp:2239-2309), iOutArmXPosition aoutarm.h:150, Prod.ZPlace written cinitial.cpp:11924-. The all-"no down" stand-in at :565-580 is commented out.
void GetOutArmZDownPos_9045(int iWhichAuto, bool bNeedDown, bool ZDownSel[MAX_ARM_Row][MAX_ARM_Col], int iZPos[MAX_ARM_Row][MAX_ARM_Col])                       //Steven 20230323 : For HT1032
{
    int iR, iC;
    for(int i=0; i<OutArmSuck.iMaxRow; i++)
    {
        for(int j=0; j<OutArmSuck.iMaxCol; j++)
        {
            iR=OutArmSuck.Suck[i][j].iMyRow;                                    //實體Row   //Steven 20250826 : fixed for out arm offset
            iC=OutArmSuck.Suck[i][j].iMyCol;                                    //實體Col

            if(i<OutArmSuck.iPickRow && j<OutArmSuck.iPickCol)
            {
                if(bNeedDown==true)
                {
                    ZDownSel[i][j]=bOutArmSuckActive[i][j];
                }
                else
                {
                    iOutArmXPosition[i][j]=j;
                    bOutArmSuckActive[i][j]=true;
                    ZDownSel[i][j]=false;
                }

                if(ZDownSel[i][j])
                {
                    iZPos[i][j]=Prod.ZPlace[iWhichAuto][iR][iC];
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

    if(InOutArmPickerUseMotor==eptUseMotCyn)
    {
        if(bNeedDown==true)
        {
            iZPos[0][0]=Prod.ZPlace[iWhichAuto][0][0];
        }
        else
        {
            iZPos[0][0]=ZSafePos;
        }
    }
}
//#endif // AI(W906-FLOW-2) 20260928: end of the now-compiled golden body (golden aoutarm9045.cpp:198-249)
//void GetOutArmZDownPos_9045(int iWhichAuto, bool bNeedDown, bool ZDownSel[MAX_ARM_Row][MAX_ARM_Col], int iZPos[MAX_ARM_Row][MAX_ARM_Col])                       //Steven 20230323 : For HT1032  -- golden :198  AI(W906-FLOW-2) 20260928: retired stand-in
//{
//    // golden :200-248 fills ZDownSel/iZPos from OutArmSuck.Suck[][].iMyRow/iMyCol
//    // + Prod.ZPlace[][]/bOutArmSuckActive[][]/ZSafePos.  bOutArmSuckActive[][]
//    // has no offline home (per-layout module) -> gate; offline mark every cell
//    // "no down" at ZSafePos so the place SM does not drive a phantom Z move.
//#if 0 // TODO(W7) -- golden aoutarm9045.cpp:200-248 (OutArmSuck.Suck iMyRow/iMyCol + bOutArmSuckActive[][] + Prod.ZPlace[][])
//#endif
//    (void)iWhichAuto; (void)bNeedDown;
//    for(int i=0; i<MAX_ARM_Row; i++)
//        for(int j=0; j<MAX_ARM_Col; j++)
//        {
//            ZDownSel[i][j]=false;
//            iZPos[i][j]=ZSafePos;
//        }
//}
//-----------------------------------------------------------------------------
bool IsCheckOutArmDestroyActiveFinish(int iXPos, int iYPos);                    //2013-08-02 Dell  -- golden :252  AI(W906-FLOW-2) 20260928: `return true` stand-in retired -- golden :252-283 is translated VERBATIM at the END of this file; its partner CheckOutArmDestroyActive (:621) is live since 216f7bd8
//{
//    #ifdef SOFT_SIMULTE
//    if(CUSTOMER_CODE==CC_AMKOR_Japan)                                           //RogerYang 20250813 add  (flag undefined -> inert)
//        return true;
//    #endif
//    // golden :258-282 reads MOT[MOutArmX/Y].ReadPos() within iPreRange + walks
//    // bCheckOutArmDestroyActiveFinish[][], then MoveOutArmToAutoSafe() if not
//    // finished.  MOT[] encoder reads gated; offline destroy is finished.
//#if 0 // TODO(W7) -- golden aoutarm9045.cpp:258-282 (MOT[MOutArmX/Y].ReadPos() proximity check)
//#endif
//    (void)iXPos; (void)iYPos;
//    return true;                                                                // offline: destroy-active finished
//}
//------------------------------------------------------------------------------
//  CheckOutArmDestroyActive -- golden :287.  Per-nozzle destroy-confirm SM
//  (static iTask[Row][Col]; cases 1/200/300).  The whole body derefs MOT[]/
//  OutArmSuck.Suck[].OnDestroy/OffDestroy/OnSuck/GetStatus + OutArmPordRec[][] +
//  iMMAuto[]/SetTrayBinData -- hardware-bound, no offline home.  GATED whole-body;
//  ACTIVE no-op so DoOutArm_9045's bCheckOutArmDestroyActive[][] guard reads false  [AI(W906-FLOW-2) 20260928: superseded -- body live since 216f7bd8 (:621); the guard reads the real flags, and IsCheckOutArmDestroyActiveFinish (EOF) waits on it]
//  and dispatch proceeds (mirror in-arm CheckInArmDestroyActive gate).
//------------------------------------------------------------------------------
// ===========================================================================
//  GOLDEN VERBATIM PAIR -- CheckOutArmDestroyActive()
//  GATED : golden aoutarm9045.cpp:287-400 (114 lines), inert reference text.
//  LIVE  : the CheckOutArmDestroyActive() body immediately AFTER the #endif below.  It is UNCHANGED by
//          this wave -- net behaviour change is ZERO.  [AI(W906-FLOW-2) 20260928: superseded -- body live since 216f7bd8: this banner describes the retired gate; the golden body is compiled (see :620)]
//  WHY   : the census scored this "translated" because a same-named LIVE body
//          existed, without comparing SIZE.  Golden's 114 lines were NOWHERE in
//          the tree -- lost text, not deferred behaviour.  Now the text EXISTS
//          and is auditable, so a later un-gate is mechanical.
//  NOTE  : golden's body is the per-nozzle destroy-confirm SM (iTask[Row][Col], cases 1/200/300).
//  RULES : nothing inside the gate is fixed, renamed, reflowed or reindented;
//          it is golden's bytes.  Nothing it references had to be made to
//          exist -- no stub, declaration or header was added for it.
//  SHAPE : same pair shape as csystem.cpp MainProc / atester.cpp (PT-W6a/W6b)
//          golden-verbatim gates.
// ===========================================================================
TQPF_Timer CheckOutArmDestroyActiveDelay[MAX_ARM_Row][MAX_ARM_Col];         // AI(W906-FLOW-2) 20260928: golden aoutarm9045.cpp:286 -- the one global the golden body below needs. GATE G-PTW6c-CheckOutArmDestroyActive RETIRED (was `#if 0 // GOLDEN VERBATIM`): golden :287-400 (re-verified identical) is compiled now; the `{}` stand-in at :736-740 is commented out.
void CheckOutArmDestroyActive()                                                 //jou 981130 start : 確認device確實destroy完成
{
    int iPos[MAX_ARM_Row][MAX_ARM_Col];
    static int iTask[MAX_ARM_Row][MAX_ARM_Col]={{0, 0, 0, 0}, {0, 0, 0, 0}};
    static int iXPos=0, iYPos=0;                                                //kevin 20120514 開吹氣時xy 馬達移動就關吹氣
    int iXpos;
    int iYpos;
    int iMotZ;
    int iWhichAuto, iCol, iRow, iMotor;
    AnsiString Msg;

    for(int i=0; i<OutArmSuck.iPickRow; i++)
    {
        for(int j=0; j<OutArmSuck.iPickCol; j++)
        {
            if(bOutArmCheckDestroyACT[i][j]==false)
            {
                iTask[i][j]=1;
                continue;
            }

            switch(iTask[i][j])
            {
                case 1:
                    bCheckOutArmDestroyActiveFinish[i][j]=false;                //ChungHung 201112296 add Pick 前先確認CheckInArmDestroyActive 已完成
                    iMotZ=(InOutArmPickerUseMotor==eptUseMotCyn)?MOutArmZA:OutArmSuck.Suck[i][j].iMotNo;
                    iPos[i][j]=MOT[iMotZ].ReadPos();
                    if(iXPos!=MOT[MOutArmX].ReadPos() &&
                       iYPos!=MOT[MOutArmY].ReadPos())                          //kevin 20120514 Xy 移動關吹氣
                    {
                        if(OutArmSuck.Item[i][j]==NULL_IC)                      //Steven 20120507 : 要持續吹氣
                            OutArmSuck.Suck[i][j].OffDestroy();
                    }

                    if(iPos[i][j]>-500)
                    {
                        iTask[i][j]=200;
                        OutArmSuck.Suck[i][j].OffDestroy();
                    }
                    else
                    {
                        if(OutArmSuck.Item[i][j]==NULL_IC)                      //Steven 20120507 : 要持續吹氣
                        {
                            if(ArmSpeed[InArm].bSuckOnDown)                     //jou 2012-10-29 大IC才持續吹氣
                                OutArmSuck.Suck[i][j].OnDestroy();
                            iXPos=MOT[MOutArmX].ReadPos();
                            iYPos=MOT[MOutArmY].ReadPos();
                        }
                    }
                    break;
                case 200:
                    OutArmSuck.Suck[i][j].OnSuck();
                    CheckOutArmDestroyActiveDelay[i][j].SetSecAndOn(ArmSpeed[OutArm].dDestroyCheckTime);                //2013-08-01    Dell    modify  公司統一回吸功能
                    if(ArmSpeed[OutArm].bSuckOnDown==false ||
                       ArmSpeed[OutArm].bDestroyPauseCheck)                                                             //2013-08-01    Dell    modify  公司統一回吸功能
                    {
                        bCheckOutArmDestroyActive[i][j]=true;                                                           //Steven 20120507 : 要偵測完才可以動，會影響UPH
                    }

                    iTask[i][j]=300;
                    break;
                case 300:
                    if(CheckOutArmDestroyActiveDelay[i][j].Off())
                    {
                        bResetOutArm=false;
                        if(OutArmSuck.Suck[i][j].W906_GetStatusAnyOn())  // AI(W906-F03-QUADVAC) 20261004: 群組：任 1 個有真空就算（回吸／殘料）
                        {
                            iXpos=MOT[MOutArmX].ReadPos();
                            iYpos=MOT[MOutArmY].ReadPos();
                            ShowErrorMessage("WAR0258", 0, MOutArmX, false, OutArmSuck.Suck[i][j].sName);               //破壞錯誤, 請將IC取下

                            iWhichAuto=OutArmPordRec[i][j].GetWhcihAuto();
                            iCol=OutArmPordRec[i][j].GetAutoX();
                            iRow=OutArmPordRec[i][j].GetAutoY();
                            if(iWhichAuto>=0)                                                                           //Steven 20210316 : 掉料的時候, 清除Unloader tray上的資料
                            {
                                iMotor=iMMAuto[iWhichAuto];

                                if(IniConfig.bE65_ClearTrayDataWhenOutArmDestoryErr)
                                {
                                    MOT[iMotor].SetTrayBinData(iCol, iRow, NULL_IC, AnsiString(""));

                                    if(iWhichAuto<0)
                                        Msg.sprintf("Clear unloader data error");
                                    else if(iWhichAuto<iAutoCnt)
                                        Msg.sprintf("Clear data of Auto%d, X:%d, Y%d", iWhichAuto+1, iCol, iRow);
                                    else
                                        Msg.sprintf("Clear data of Fix%d, X:%d, Y%d", iWhichAuto+1, iCol, iRow);

                                    MyDBIProcess("Process", Msg);
                                }
                                OutArmPordRec[i][j].bUse=true;
                                OutArmPordRec[i][j].AddErrorRecord("WAR0258", true, iXpos, iYpos);
                            }
                            bCheckOutArmDestroyActive[i][j]=true;
                            iTask[i][j]=200;                                                                            //Steven 20120507 : 有出錯就繼續吸吸看，看IC有沒有被拿掉。
                        }
                        else
                        {
                            bCheckOutArmDestroyActiveFinish[i][j]=true;                                                 //ChungHung 201112296 add Pick 前先確認CheckInArmDestroyActive 已完成
                            OutArmSuck.Suck[i][j].Normal();
                            OutArmPordRec[i][j].InitialRecord();                                                        //Steven 20210316 : 掉料的時候, 清除Unloader tray上的資料
                            bOutArmCheckDestroyACT[i][j]=false;
                            bCheckOutArmDestroyActive[i][j]=false;
                            iTask[i][j]=1;                                                                              //ChungHung 20111229 add 防止跳出後bInArmCheckDestroyACT馬上被設為true 會照成有一次無效
                        }
                    }
                    break;
                default :
                    break;
            }
        }
    }
}
//#endif // AI(W906-FLOW-2) 20260928: end of the now-compiled golden body (golden aoutarm9045.cpp:287-400)
//void CheckOutArmDestroyActive()                                                 //jou 981130 : 確認device確實destroy完成  AI(W906-FLOW-2) 20260928: retired stand-in
//{
//#if 0 // TODO(W7) -- golden aoutarm9045.cpp:287-400 (MOT[]/OutArmSuck destroy SM + OutArmPordRec[][] + SetTrayBinData)
//#endif
//}

//==============================================================================
//==  ACTIVE: iInArmType dispatch ladder #1  DoOutArm_9045   -- golden :402
//==  Pre-dispatch guards + if-ladder STRUCTURE kept ACTIVE & verbatim; the
//==  cross-module guard bodies (fNote / AOA CCD block) are gated with active
//==  fallbacks; the ~21 per-variant call-arms dispatch to the ACTIVE stubs above.
//==  golden DoOutArm_9045 has NO final else -> unmatched iInArmType is a silent
//==  no-op.  Reproduced verbatim.
//==============================================================================
void DoOutArm_9045()                                                            // golden :402
{
    if(IniConfig.bEnableTestingNeedStopAllMotor==true &&                        //jou 2013-09-25 Testing Need Stop All Motor
       IniConfig.bI24TestingNeedStopAllMotor==true &&
       bTestingStopAllMotor==true)
    {
        return;
    }

    if(IniConfig.bAlarmNeedServoOff==true)                                      //JerryYang 20161227 (Steven) 修正發生JAM,servo off後推動out arm, 再重新start會發生點位錯誤
    {
//#if 0 // AI(W906-FLOW-2) 20260928: gate RETIRED -- golden aoutarm9045.cpp:413. TfNote::bMyServoOffOutArm exists (forms/fNote.h:305, init false fNote.cpp:42) on the single live fNote; already read live at csystem.cpp:21903 and cleared at uhome.cpp:1074. Nothing sets it true yet (golden writers main.cpp:7182/7215, note.cpp:2074 untranslated) -> no effect until they land.
        if(fNote->bMyServoOffOutArm==true)
        {
            return;
        }
//#endif // AI(W906-FLOW-2) 20260928: end of retired gate (:761)
    }

    if(CUSTOMER_CODE==CC_ASE_KaohSiung)
    {
        if(ArmSpeed_File[OutArm].bDevicConfirm)                                 //kevin 20170510 (wei) 回吸功能開關
            CheckOutArmDestroyActive();                                         //jou 981130 start : 確認device確實destroy完成
    }
    else
    {
        CheckOutArmDestroyActive();                                             //jou 981130 start : 確認device確實destroy完成
    }

    for(int i=0; i<OutArmSuck.iPickRow; i++)
        for(int j=0; j<OutArmSuck.iPickCol; j++)
            if(bCheckOutArmDestroyActive[i][j]==true)
                return;

    if(IniConfig.bQAMode==true &&
       LastSet.iRunStartMode==rsmQAMode &&
       CosFunction.bQAModeUseUnloadCnt)                                         //JerryYang 20221004 : Maxim版本QA mode
    {
        Check_QA_ModeUnloadCount();
    }

    if(iPauseBackUp!=-1 &&
       OutArmSuck.IsPickSuckFinish()==true &&
       OutArmSuck.IsPickDestroyFinish()==true)
    {
        return;
    }

    if(bResetOutArmTask==true)
    {
        bResetOutArmTask=false;
        InitOutArmTask();
        return;
    }

    // golden :455-532 MACHINE_HAS_AUTO_ALIGNMENT_CCD AOA block.  Its body derefs
    // MOT[MManualTray*].fHasTray / TestIF_File.iAutoAlignment_UseFix[] /
    // CheckOutArmAutoAlignmentTrayModeBeUse / IsCatchTrayReadySupplyNewTray -- all
    // cross-module with no offline home.  Keep the OUTER guard ACTIVE (faithful:
    // offline bEnableAutoAlignment is false -> takes the else, which sets the two
    // flags false); GATE the inner true-branch body.
    if(MACHINE_HAS_AUTO_ALIGNMENT_CCD &&
       TestIF.bEnableAutoAlignment==true &&
       (LastSet.iRealDummy==HAS_TRAY || LastSet.iRealDummy==REALLY))            //KenHsieh 20210813 : add CCD AUTO ALIGNMENT
    {
#if 0 // TODO(W7) -- golden aoutarm9045.cpp:460-526 (MOT[MManualTray*]/AOA tray-mode block)  -- STILL HOLDS, re-checked AI(W906-POOL2) 20261008 (Ifor01): fHasTray / iAutoAlignment_UseFix / iAutoIndex / WhichAutoNeedTray / IsCatchTrayReadySupplyNewTray exist now, but golden :507-508 also needs CheckOutArmAutoAlignmentCKModeBeUse (absent tree-wide) and CheckOutArmAutoAlignmentTrayModeBeUse(long&, bool) (only a `bool (int) { return false; }` stand-in, acatchtray_shims.cpp:132); the body here is a placeholder, translate golden :460-526 when those two land
        bool bDoOutArmAutoAlignment=false;
        if(bAutoNeedTrayMustFinish) { /* ... golden :462-522 verbatim ... */ }
        else if(bRunOutArmAutoAlignment) { return; }
#endif
    }
    else
    {
        bAutoNeedTrayMustFinish=false;
        bRunOutArmAutoAlignment=false;
    }

    if(bOutArmLaserActionflag[0]==true || bOutArmLaserActionflag[1]==true)
    {
        return;
    }

    // --- iInArmType dispatch ladder (golden :539-628) ----------------------
    // STRUCTURE kept ACTIVE & verbatim.  The ~21 per-variant callees dispatch to
    // the ACTIVE stubs declared above (variant bodies live in aoutarm9045_<layout>.cpp,
    // W6.2b variants).  The e9045_1x4_4_Back arm is an EMPTY `//` body in golden --
    // kept empty.  golden has NO final else -> silent no-op on unmatched type.
    if(USE_PICKER_COUNT==ep1Picker)
    {
        DoOutArm_9045_All_1Picker();
    }
    //AI(W906-TIF912) 20260925: golden 912 :629-634 -- DualSite-Use-One-Suck rung.
    //  GATE G-TIF912-1x2_1: DoOutArm_9045_1x2_1() is golden aoutarm9045_1x2_1.cpp
    //  (2215 L), NOT translated.  Condition live (ladder order = golden); the
    //  body is a LOUD no-op instead of golden's real flow -- not a silent one,
    //  because this rung is a matched arm in golden, not the unmatched fallthrough
    //  the banner above describes.  DoInArm_9045 raises the same Program Error
    //  on its own rung, so no IC is picked into this flow in the first place.
    else if(TestIF_File.bDualSiteUseOneSuck &&                                  //wei 20251218 DualSite Use One Suck (port from HT9046LS V3.34)
            (TestIF.iTestMode==DualSite || TestIF.iTestMode==QualSite2X2N) &&
            DeviceForm.XDimension>=7000 && DeviceForm.YDimension>=7000)
    {
#if 0   // GATE G-TIF912-1x2_1 -- golden :633 (aoutarm9045_1x2_1.cpp not translated)
        DoOutArm_9045_1x2_1();
#endif
        ShowMyMessage("Program Error in DoOutArm_9045()", "DualSiteUseOneSuck (DoOutArm_9045_1x2_1 not translated)");
    }
    else if(iInArmType==e9045_1x1_1 ||
       iInArmType==e9045_1x4_1_Ac)
    {
        DoOutArm_9045_1x1_1();
    }
    else if(iInArmType==e9045_1x2_2_14 ||
            iInArmType==e9045_1x2_2_13)
    {
        DoOutArm_9045_1x2_2();
    }
    else if(iInArmType==e9045_1x2_4_Hot)
    {
        DoOutArm_9045_1x2_4();
    }
    else if(iInArmType==e9045_1x3_2_14)
    {
        DoOutArm_9045_1x3_2_14();
    }
    else if(iInArmType==e9045_1x3_4)                                            //Steven 20220425 : 1x3_14
    {
        DoOutArm_9045_1x3_4();
    }
    else if(iInArmType==e9045_1x4_2_14)
    {
        DoOutArm_9045_1x4_2();
    }
    else if(iInArmType==e9045_1x4_4_13)
    {
        DoOutArm_9045_1x4_4S();
    }
    else if(iInArmType==e9045_1x4_4_Back)
    {
//
    }
    else if(iInArmType==e9045_1x4_4)
    {
        DoOutArm_9045_1x4_4();
    }
    else if(iInArmType==e9045_1x4_8_Hot)
    {
        DoOutArm_9045_1x4_8();
    }
    else if(iInArmType==e9045_2x1_2_13)
    {
        DoOutArm_9045_2x1_2();
    }
    else if(iInArmType==e9045_2x2_4_12 ||
            iInArmType==e9045_2x2_4_13 ||
            iInArmType==e9045_2x2_4_14)
    {
        DoOutArm_9045_2x2_4();
    }
    else if(iInArmType==e9045_2x2_8_Hot)
    {
        DoOutArm_9045_2x2_8();
    }
    else if(iInArmType==e9045_2x3_6_14)
    {
        DoOutArm_9045_2x3_6_14();
    }
    else if(iInArmType==e9045_2x3_6)
    {
        DoOutArm_9045_2x3_6();
    }
    else if(iInArmType==e9045_2x4_4_13 ||
            iInArmType==e9045_2x4_4_14)
    {
        DoOutArm_9045_2x4_4();
    }
    else if(iInArmType==e9045_2x4_8)
    {
        DoOutArm_9045_2x4_8();
    }
    else if(iInArmType==e9045_2x5_8)                                            //wei 20190614 10 site
    {
        DoOutArm_9045_2x5_8();
    }
    else if(iInArmType==e9045_2x6_8)
    {
        DoOutArm_9045_2x6_8();
    }
    else if(iInArmType==e9045_2x8_8 ||
            iInArmType==e9045_2x8_32)
    {
        DoOutArm_9045_2x8_8();
    }
}
//-----------------------------------------------------------------------------
//  CheckOutArmInitState_9045 -- golden :632.  Walks OutArmSuck + CheckSuckInitialStatus
//  + ShowErrorMessage("WAR0226"); MOT[]/CheckSuckInitialStatus hardware-bound.
//  GATED whole-body; offline init-state is OK (no fail) -> true.  [AI(W906-FLOW-2) 20260928: superseded -- body live since 216f7bd8 (:966)]
//-----------------------------------------------------------------------------
// ===========================================================================
//  GOLDEN VERBATIM PAIR -- CheckOutArmInitState_9045()
//  GATED : golden aoutarm9045.cpp:632-671 (40 lines), inert reference text.
//  LIVE  : the CheckOutArmInitState_9045() body immediately AFTER the #endif below.  It is UNCHANGED by
//          this wave -- net behaviour change is ZERO.  [AI(W906-FLOW-2) 20260928: superseded -- body live since 216f7bd8: this banner describes the retired gate; the golden body is compiled (see :965)]
//  WHY   : the census scored this "translated" because a same-named LIVE body
//          existed, without comparing SIZE.  Golden's 40 lines were NOWHERE in
//          the tree -- lost text, not deferred behaviour.  Now the text EXISTS
//          and is auditable, so a later un-gate is mechanical.
//  NOTE  : golden's body walks OutArmSuck + CheckSuckInitialStatus + WAR0226.
//  RULES : nothing inside the gate is fixed, renamed, reflowed or reindented;
//          it is golden's bytes.  Nothing it references had to be made to
//          exist -- no stub, declaration or header was added for it.
//  SHAPE : same pair shape as csystem.cpp MainProc / atester.cpp (PT-W6a/W6b)
//          golden-verbatim gates.
// ===========================================================================
// AI(W906-FLOW-2) 20260928: GATE G-PTW6c-CheckOutArmInitState_9045 RETIRED (was `#if 0 // GOLDEN VERBATIM`): the golden body below (golden aoutarm9045.cpp:632-671, re-verified identical) is compiled now: CheckSuckInitialStatus is live (csystem.cpp:20910), TMyProductionRecord::AddErrorRecord is real (W2-PRODLOG), same shape as the live in-arm twin CheckInArmSuckInitial (ainarm2.cpp:4815-4866). Golden reads `ret` uninitialised when bHasFail==false -- kept as golden (then every flag[][] is 0, so the K_SKIP loop body cannot fire for i<2,j<4). The `return true` stand-in at :1007-1012 is commented out.
bool CheckOutArmInitState_9045()                                                //Steven 20100104 : 將所有錯誤整合只顯示一次
{
    bool bHasFail=false;
    int ret;
    int flag[MAX_ARM_Row][MAX_ARM_Col]={{0, 0, 0, 0}, {0, 0, 0, 0}};
    AnsiString errSuck="at Out Arm Suck";
    int iXpos=MOT[MOutArmX].ReadPos();
    int iYpos=MOT[MOutArmY].ReadPos();
    for(int i=0; i<OutArmSuck.iPickRow; i++)
    {
        for(int j=0; j<OutArmSuck.iPickCol; j++)
        {
            flag[i][j]=CheckSuckInitialStatus(OutArmSuck, i, j);
            if(flag[i][j]!=0)
            {
                bHasFail=true;
                errSuck+=OutArmSuck.Suck[i][j].sName;
            }
        }
    }

    if(bHasFail)
        ret=ShowErrorMessage("WAR0226", K_RETRY|K_SKIP, MOutArmX, false, errSuck);

    if(ret==K_SKIP)
    {
        for(int i=0; i<OutArmSuck.iMaxRow; i++)
        {
            for(int j=0; j<OutArmSuck.iMaxCol; j++)
            {
                if(flag[i][j]>=Vaccum_Initial_Off)
                {
                    OutArmSuck.PordRec[i][j].AddErrorRecord("WAR0226", true, iXpos, iYpos);                             //Steven 20160114 (jou) : Add Production Error Log
                    OutArmSuck.SetItemData(i, j, HAS_NULL_IC);
                }
            }
        }
    }
    return !bHasFail;
}
//#endif // AI(W906-FLOW-2) 20260928: end of the now-compiled golden body (golden aoutarm9045.cpp:632-671)
//bool CheckOutArmInitState_9045()                                                //Steven 20100104 : 將所有錯誤整合只顯示一次  AI(W906-FLOW-2) 20260928: retired stand-in
//{
//#if 0 // TODO(W7) -- golden aoutarm9045.cpp:632-671 (CheckSuckInitialStatus/MOT[]/ShowErrorMessage WAR0226)
//#endif
//    return true;                                                                // offline: no suck-init failure
//}
//------------------------------------------------------------------------------
void DoOutArmSuckPreOn(int iSht, int iKit, int iXPos, int iYPos, bool bZDown)   // golden :673
{
    // golden :675-709 reads MOT[MOutArmX/Y].ReadPos() proximity + OutArmSuck.Suck
    // pre-on solenoid drive.  Keep the early-return guard ACTIVE & faithful; gate
    // the MOT[]/solenoid body.   //AI(W906-AMB-L2) 20260930: INBOX 111b item 1 -- no longer gated, the golden body is compiled below
    if(bZDown==false || iSht==2)                                                //RogerYang 20250513 Add for 9046AU  //Steven 20230601 : 加入iKit跟bZDown判斷
        return;
//#if 0 // TODO(W7) -- golden aoutarm9045.cpp:678-709 (MOT[MOutArmX/Y].ReadPos() proximity + OutArmSuck.Suck pre-on)   //AI(W906-AMB-L2) 20260930: INBOX 111b item 1 -- gate RETIRED, golden aoutarm9045.cpp:678-706 compiled below word for word. Every dependency is present: MOT[] / Sen[SnRKManualStep] / LastSet / ArmSpeed[OutArm].bSuckOnDown are used live in this file, ptrOutSHT is golden mykitsuck.h:476 (DEF mykitsuck.cpp) and FRCarryKit / BRCarryKit / OutArmSuck.iPickKitStep are the golden TMyKitSuck layout (mykitsuck.h, reached via aHotPlateSubstrate.h:97).  Effect = golden: with REALLY + bSuckOnDown and no manual step, the out-arm vacuum opens once the arm is within 50 pulses of the target during the Z descent (before: only after Z reached position).
    int iPosX=0, iPosY=0;
    int iPreRange=50;
    iKit=(iKit==0)?0:OutArmSuck.iPickKitStep;

    if(iSht==0)
        ptrOutSHT=&FRCarryKit;
    else
        ptrOutSHT=&BRCarryKit;

    if(LastSet.iRealDummy==REALLY &&
       ArmSpeed[OutArm].bSuckOnDown &&
       Sen[SnRKManualStep].IsOn()==false)
    {
        iPosX=MOT[MOutArmX].ReadPos();
        iPosY=MOT[MOutArmY].ReadPos();                                          //2013-08-02    Dell
        if(iPosX>=iXPos-iPreRange && iPosX<=iXPos+iPreRange &&
           iPosY>=iYPos-iPreRange && iPosY<=iYPos+iPreRange)
        {
            for(int i=0; i<OutArmSuck.iPickRow; i++)
            {
                for(int j=0; j<OutArmSuck.iPickCol; j++)                        //少了kit
                {
                    if(ptrOutSHT->Item[i][j+iKit] &&                            //有問題!!
                       OutArmSuck.Suck[i][j].GetNeedSuckStatus())
                        OutArmSuck.Suck[i][j].On();
                }
            }
        }
    }
//#endif   //AI(W906-AMB-L2) 20260930: end of the golden body (golden :706); the offline "(void)iKit; (void)iXPos; (void)iYPos;" stand-in line is gone with the gate
}
//------------------------------------------------------------------------------
// AI(W906-AMB-L2) 20260929: golden aoutarm9045.cpp:709 DEFINES `int iOutShtRetryCount=0;` right here and golden aoutarm9045.h:9 declares it.  In the port the definition lives in aoutarm_shims.cpp:39 (left there: a second definition here would collide at link) and the port aoutarm9045.h has no extern for it, so it is declared locally for the golden body below.  golden :710 bOutSuckShtDupErr (DEF aoutarm_shims.cpp:41; the port declares it per variant file, e.g. aoutarm9045_1x1_1.cpp:218 -- NOT in aoutarm9045.h, review 20260929 F1) is not read by this function.
extern int  iOutShtRetryCount;                                                  // golden aoutarm9045.h:9 -- DEF aoutarm_shims.cpp:39
// AI(W906-AMB-L2) 20260929: GATE `#if 0 // TODO(W7) -- golden aoutarm9045.cpp:711+ (OutArmSuck/Prod/MOT[] out-shuttle Z status)` RETIRED: the golden body below (golden aoutarm9045.cpp:711-747, word for word) is compiled now.  Every dependency was re-measured and exists: OutArmSuck (TMyKitSuck, DECL aHotPlateSubstrate.h:651) .iPickRow/.iPickCol (MyKitSuck.h:290-291), .Suck[][].iMyRow/.iMyCol (MyKitSuck.h:268-269), .Suck[][].GetNeedSuckStatus() (MyKitSuck.h:252, public); Prod.ZOutArm_Shuttle1_Pick/_Shuttle2_Pick (cprod.h:448-449, written ungated cinitial.cpp:11464-11490), Prod.ZOutArm_SortShuttle_Place (cprod.h:410, written cinitial.cpp:11697-11713); ArmSpeed[OutArm].dRetryDown (cprod.h:2836 / extern cprod.h:2879, loaded cSpeed.cpp:712); iOutShtRetryCount (above).  golden reads NO MOT[] here -- the "MOT[]" in the old gate text was wrong.  The all-"no down" stand-in is commented out after the body; with it OutArmZMoveDown never lowered a picker at the shuttle.
// AI(W906-AMB-L2) 20260929: GOLDEN BEHAVIOUR KEPT: the loops cover OutArmSuck.iPickRow x iPickCol only, so cells outside the pick grid are NOT written (the stand-in wrote all MAX_ARM_Row x MAX_ARM_Col); callers zero-fill bZFlag/iZPos first (e.g. aoutarm9045_1x1_1.cpp:391-392), so those cells stay false/0 exactly as in golden.
void GetOutShuttleStatus_9045(int iSht, int iZPos[MAX_ARM_Row][MAX_ARM_Col], bool bZDown, bool bZFlag[MAX_ARM_Row][MAX_ARM_Col])
{
    int iR, iC;
    for(int i=0; i<OutArmSuck.iPickRow; i++)
    {
        for(int j=0; j<OutArmSuck.iPickCol; j++)
        {
            iR=OutArmSuck.Suck[i][j].iMyRow;                                    //實體Row   //Steven 20250826 : fixed for out arm offset
            iC=OutArmSuck.Suck[i][j].iMyCol;                                    //實體Col

            if(bZDown && OutArmSuck.Suck[i][j].GetNeedSuckStatus())
                bZFlag[i][j]=true;
            else
                bZFlag[i][j]=false;

            if(bZDown)
            {
                if(iSht==0)
                {
                    iZPos[i][j]=Prod.ZOutArm_Shuttle1_Pick[iR][iC]+iOutShtRetryCount*ArmSpeed[OutArm].dRetryDown;
                }
                else if(iSht==2)                                                //RogerYang 20250513 Add for 9046AU Sort Shuttle
                {
                    iZPos[i][j]=Prod.ZOutArm_SortShuttle_Place[iR][iC]+iOutShtRetryCount*ArmSpeed[OutArm].dRetryDown;
                }
                else
                {
                    iZPos[i][j]=Prod.ZOutArm_Shuttle2_Pick[iR][iC]+iOutShtRetryCount*ArmSpeed[OutArm].dRetryDown;
                }
            }
            else
            {
                iZPos[i][j]=ZSafePos;
            }
        }
    }
}
// AI(W906-AMB-L2) 20260929: end of the now-compiled golden body (golden aoutarm9045.cpp:711-747)
//void GetOutShuttleStatus_9045(int iSht, int iZPos[MAX_ARM_Row][MAX_ARM_Col], bool bZDown, bool bZFlag[MAX_ARM_Row][MAX_ARM_Col])   // golden :711  AI(W906-AMB-L2) 20260929: retired stand-in
//{
//    // golden :713+ builds the out-shuttle Z status grid from OutArmSuck/Prod/MOT[].
//    // GATED whole-body; offline default the grid to "no down" at ZSafePos.
//#if 0 // TODO(W7) -- golden aoutarm9045.cpp:711+ (OutArmSuck/Prod/MOT[] out-shuttle Z status)
//#endif
//    (void)iSht; (void)bZDown;
//    for(int i=0; i<MAX_ARM_Row; i++)
//        for(int j=0; j<MAX_ARM_Col; j++)
//        {
//            bZFlag[i][j]=false;
//            iZPos[i][j]=ZSafePos;
//        }
//}

//==============================================================================
//==  ACTIVE: iInArmType dispatch ladder #2  DoPickFromShuttle_9045  -- golden :2038
//==  Pre-dispatch is none; the if-ladder STRUCTURE + the final else Program-Error
//==  are kept ACTIVE & verbatim (the final else is the live offline behavior).
//==  The ~19 per-layout callees dispatch to the ACTIVE stubs above.
//==============================================================================
bool DoPickFromShuttle_9045(int iSht)                                           // golden :2038
{
    AnsiString Str;
    bool bRet=false;
    if(iInArmType==e9045_1x1_1 ||
       iInArmType==e9045_1x4_1_Ac)
    {
        bRet=DoPickFromShuttle_9045_1x1_1(iSht);
    }
    else if(iInArmType==e9045_1x2_2_13 ||
            iInArmType==e9045_1x2_2_14)
    {
        bRet=DoPickFromShuttle_9045_1x2_2(iSht);
    }
    else if(iInArmType==e9045_1x2_4_Hot)
    {
        bRet=DoPickFromShuttle_9045_1x2_4(iSht);
    }
    else if(iInArmType==e9045_1x3_2_14)
    {
        bRet=DoPickFromShuttle_9045_1x3_2_14(iSht);
    }
    else if(iInArmType==e9045_1x3_4)
    {
        bRet=DoPickFromShuttle_9045_1x3_4(iSht);
    }
    else if(iInArmType==e9045_1x4_4_13)
    {
        bRet=DoPickFromShuttle_9045_1x4_4S(iSht);
    }
    else if(iInArmType==e9045_1x4_2_14)
    {
        bRet=DoPickFromShuttle_9045_1x4_2(iSht);
    }
    else if(iInArmType==e9045_1x4_4_Back ||
            iInArmType==e9045_1x4_4)
    {
        bRet=DoPickFromShuttle_9045_1x4_4(iSht);
    }
    else if(iInArmType==e9045_1x4_8_Hot)
    {
        bRet=DoPickFromShuttle_9045_1x4_8(iSht);
    }
    else if(iInArmType==e9045_2x1_2_13)
    {
        bRet=DoPickFromShuttle_9045_2x1_2(iSht);
    }
    else if(iInArmType==e9045_2x2_4_12 ||
            iInArmType==e9045_2x2_4_13 ||
            iInArmType==e9045_2x2_4_14)
    {
        bRet=DoPickFromShuttle_9045_2x2_4(iSht);
    }
    else if(iInArmType==e9045_2x2_8_Hot)
    {
        bRet=DoPickFromShuttle_9045_2x2_8(iSht);
    }
    else if(iInArmType==e9045_2x3_6_14)
    {
        bRet=DoPickFromShuttle_9045_2x3_6_14(iSht);
    }
    else if(iInArmType==e9045_2x3_6)
    {
        bRet=DoPickFromShuttle_9045_2x3_6(iSht);
    }
    else if(iInArmType==e9045_2x4_4_13 ||
            iInArmType==e9045_2x4_4_14)
    {
        bRet=DoPickFromShuttle_9045_2x4_4(iSht);
    }
    else if(iInArmType==e9045_2x4_8)
    {
        bRet=DoPickFromShuttle_9045_2x4_8(iSht);
    }
    else if(iInArmType==e9045_2x5_8)
    {
        bRet=DoPickFromShuttle_9045_2x5_8(iSht);
    }
    else if(iInArmType==e9045_2x6_8)
    {
        bRet=DoPickFromShuttle_9045_2x6_8(iSht);
    }
    else if(iInArmType==e9045_2x8_8 ||
            iInArmType==e9045_2x8_32)
    {
        bRet=DoPickFromShuttle_9045_2x8_8(iSht);
    }
    else
    {
        Str.sprintf("iInArmType=%d", iInArmType);                               //Steven 20220620 : add log message
        ShowMyMessage("Program Error in DoPickFromShuttle_9045()", Str);
    }
    return bRet;
}

//==============================================================================
//==  ACTIVE SM #1: DoOutArmAdditionalFunction  -- golden :2246
//==  DIRECT mirror of DoInArmAdditionalFunction.  Cursor flow (Task transitions)
//==  + the case 1->100 fall-through kept ACTIVE & verbatim.  The cross-module
//==  action bodies (MoveOutArmToAutoSafe/InitialOutArmRotateKIT/InitAOIFunction/
//==  fFixAICCD-> + predicate CheekNeedToDoOutArmAdditionalFunction) are gated; the
//==  offline "nothing to do" path drives Task 1->100->bResult=true (SM converges).
//==============================================================================
void InitDoOutArmAdditionalFunction()                                           // golden :2138
{
    iOutArmAdditionalFunctionTask=1;
}
//------------------------------------------------------------------------------
#if 0 // AI(W906-OUTADD) 20260927: 出料臂附加功能三支（CheekNeed… / DoOutArmAdditionalFunction / PreSetOutAdditionalFlag）的離線舊版與 golden 原文對照退休 —— golden 照翻在本檔檔尾
//  GOLDEN VERBATIM PAIR -- CheekNeedToDoOutArmAdditionalFunction()
//  GATED : golden aoutarm9045.cpp:2143-2244 (102 lines), inert reference text.
//  LIVE  : the CheekNeedToDoOutArmAdditionalFunction() body immediately AFTER the #endif below.  It is UNCHANGED by
//          this wave -- net behaviour change is ZERO.
//  WHY   : the census scored this "translated" because a same-named LIVE body
//          existed, without comparing SIZE.  Golden's 102 lines were NOWHERE in
//          the tree -- lost text, not deferred behaviour.  Now the text EXISTS
//          and is auditable, so a later un-gate is mechanical.
//  NOTE  : golden's body is the Rotate/AOI/FixAI predicate (tRotate/tAOISetup/FrmAOI/fFixAICCD).
//  RULES : nothing inside the gate is fixed, renamed, reflowed or reindented;
//          it is golden's bytes.  Nothing it references had to be made to
//          exist -- no stub, declaration or header was added for it.
//  SHAPE : same pair shape as csystem.cpp MainProc / atester.cpp (PT-W6a/W6b)
//          golden-verbatim gates.
// ===========================================================================
#if 0 // GOLDEN VERBATIM -- golden aoutarm9045.cpp:2143-2244 (102 lines).  GATE G-PTW6c-CheekNeedToDoOutArmAdditionalFunction.  NOT COMPILED: the ACTIVE CheekNeedToDoOutArmAdditionalFunction() is the body immediately after this #endif.
bool CheekNeedToDoOutArmAdditionalFunction()                                    //Steven 20210609 : 整合Precisor, Rotator, Bottom CCD, Die Clean
{
    bool bResult=false;
    bool bSupportFixAI=false;
    if(OutArmSuck.bAlreadyRotate==false)
    {
        if(CheckRotateOutNotFinish())
        {
            bOutRotator=true;
            bResult=true;
        }
        else if(USE_ROTATE_KIT==1 &&
                tRotate.ActiveRotate &&
                TrayForm.iRotateKIT_InputType!=0 &&
                (iOutRotateFinish==0 || iOutRotateFinish==1))
        {
            bOutRotator=true;
            bResult=true;
        }
    }

    if(CosFunction.bART_RT_NoRotate && tRotate.bART_RT_NoRotate && bCanRunSCKART)                                       //Sam 20240809 : ART RT No Rotate
        bOutRotator=false;
    if(OutArmSuck.bAlreadyAOI==false)
    {
//        if(tAOISetup.bEnabledAOI ||                                             //2014-03-04    Dell    for SPIL WLP Add 5S Inspection
//           (USE_Scanner_AOI_Inspection==true && ScannerAOIIF.iEnableScannerMode!=0) ||
//           (USE_Top_Scanner_AOI_Inspection==true && ScannerAOIIF.iEnableTopScannerMode!=0))
        if(tAOISetup.bEnabledAOI ||  //2014-03-04    Dell    for SPIL WLP Add 5S Inspection
           (USE_Scanner_AOI_Inspection==(int)eBtnAOI_BottomInstall && ScannerAOIIF.iEnableScannerMode!=0) || //AI(general) 20260411 (RogerYang) : fix eBtnAOI_Uninstall typo, was always false
           (USE_Top_Scanner_AOI_Inspection==(int)eBtnAOI_BottomInstall && ScannerAOIIF.iEnableTopScannerMode!=0) ||
           FrmAOI->RunTopBottomInspect()==true)  //Jimmychiu 20240322 : Top & Bottom Inspect
        {
            bDoAOI=true;
            bResult=true;
        }
    }

    if(OutArmSuck.bAlreadyFixAI==false)                                         //Sam 20211220 : 整合 Fix AI AOI
    {
        if(TestIF.iTestMode==SingleSite                         ||
           TestIF.iTestMode==QualSite2X2N                       ||
           TestIF.iTestMode==DualSite)
        {
            bSupportFixAI=true;
        }
        else if(TestIF.iTestMode==QualSite2X2)
        {
            if(USE_PICKER_COUNT==0)
            {
                bSupportFixAI=true;
            }
            else if(TestIF.iUseSuckMode==4 && TestIF.dSiteXPitch>iXpitchMinX2)
            {
                bSupportFixAI=true;
            }
            else
            {
                if(TestIF.iUseSuckMode==8 &&
                   TestIF.dSiteXPitch>iXpitchMinX2)                             //Sam 20231107 : 補 Fix AOI 模式
                {
                    bSupportFixAI=true;
                }
            }
        }
       else if(TestIF.iTestMode==QualSite1X4 ||                                 //Sam 20220613: 修正 FixAOI 未動作問題
               TestIF.iTestMode==_12Site2X6  ||                                 //Sam 20240329 : 新增 FixAOI 模式
               TestIF.iTestMode==_16Site2X8)
        {
            //if(TestIF.iUseSuckMode==4)                                        //Sam 20231107 : 補 Fix AOI 模式 Mark
            {
                bSupportFixAI=true;
            }
        }
        else if(TestIF.iTestMode==_8Site2X4 ||
                TestIF.iTestMode==_16Site4X4)
        {
            if(ArmCanSuck4IC(0, true)==false || USE_PICKER_COUNT==0)
            {
            }
            else
            {
               bSupportFixAI=true;
            }
        }

        if(bSupportFixAI &&
           USE_Fix_AI_CCD &&
           TestIF_File.bEnableFix2BGAAICCD &&
           fFixAICCD->NeedToGrabImage())                                        //KaiChen 20190508 ：矽格湖口 AI CCD Function
        {
            bDoFixAI=true;
            bResult=true;
        }
    }

    if(bResult)
    {
        InitDoOutArmAdditionalFunction();
    }
    return bResult;
}
#endif // GOLDEN VERBATIM -- golden aoutarm9045.cpp:2143-2244  (GATE G-PTW6c-CheekNeedToDoOutArmAdditionalFunction, end)
bool CheekNeedToDoOutArmAdditionalFunction()                                    //Steven 20210609 : 整合Precisor, Rotator, Bottom CCD, Die Clean  -- golden :2143
{
    bool bResult=false;
    // golden :2145-2243 reads OutArmSuck.bAlready{Rotate,AOI,FixAI} + CheckRotateOutNotFinish
    // + USE_ROTATE_KIT/tRotate + tAOISetup/ScannerAOIIF/FrmAOI-> + fFixAICCD-> and
    // sets bOutRotator/bDoAOI/bDoFixAI.  All cross-module (Rotate/AOI/FixAI), no
    // offline home.  Offline: nothing to do -> bResult stays false.
#if 0 // TODO(W7) -- golden aoutarm9045.cpp:2145-2238 (Rotate/AOI/FixAI predicate: tRotate/tAOISetup/ScannerAOIIF/FrmAOI/fFixAICCD)
#endif
    if(bResult)
    {
        InitDoOutArmAdditionalFunction();
    }
    return bResult;
}
//------------------------------------------------------------------------------
bool DoOutArmAdditionalFunction()                                               //Steven 20210609 : 整合Precisor, Rotator, Bottom CCD, Die Clean  -- golden :2246
{
    bool bResult=false;
    int &Task=iOutArmAdditionalFunctionTask;

    switch(iOutArmAdditionalFunctionTask)
    {
        case 1:
            CheekNeedToDoOutArmAdditionalFunction();
            Task=100;
            // fall-through to case 100 (golden :2255 -- intentional, no break)
        case 100:
            if(MoveOutArmToAutoSafe())
            {
                CheckOutArmSuckICFallDown();
#if 0 // TODO(W7) -- golden aoutarm9045.cpp:2260-2280 (MOT[MInShuttle1/2].fCanMoveR + Rotate/AOI/FixAI Init* dispatch)
                MOT[MInShuttle1].fCanMoveR=true;
                MOT[MInShuttle2].fCanMoveR=true;
                if(OutArmSuck.bAlreadyRotate==false && bOutRotator)
                {
                    if(iOutRotateFinish==0)
                    {
                        InitialOutArmRotateKIT();
                        iOutRotateFinish=1;
                    }
                    Task=10000;
                }
                else if(OutArmSuck.bAlreadyAOI==false && bDoAOI)
                {
                    InitAOIFunction();
                    Task=20000;
                }
                else if(OutArmSuck.bAlreadyFixAI==false && bDoFixAI)
                {
                    fFixAICCD->Fix2AICCDFunction();
                    Task=30000;
                }
                else
#endif
                {
                    // offline: Rotate/AOI/FixAI all "already done" -> nothing to do.
                    bResult=true;
                }
            }
            break;
#if 0 // TODO(W7) -- golden aoutarm9045.cpp:2287-2311 (Rotate/AOI/FixAI action SMs: DoOutArmRotateKIT/DoAOIFunction/fFixAICCD->DoFix2AICCDFunction)
        case 10000:
            if(DoOutArmRotateKIT()==true)
            {
                OutArmSuck.bAlreadyRotate=true;                                 //Steven 20260612 : Fix == to = (was comparison, not assignment)
                bOutRotator=false;
                iOutRotateFinish=2;                                             //kevin 20130524 目前放到 auto tray
                Task=100;
            }
            break;
       case 20000:
            if(DoAOIFunction()==true)
            {
                OutArmSuck.bAlreadyAOI=true;
                bDoAOI=false;
                Task=100;
            }
            break;
       case  30000:
            if(fFixAICCD->DoFix2AICCDFunction()==true)
            {
                OutArmSuck.bAlreadyFixAI=true;
                bDoFixAI=false;
                Task=100;
            }
            break;
#endif
    }
    return bResult;
}
//------------------------------------------------------------------------------
void PreSetOutAdditionalFlag()                                                  //Steven 20210609 : 整合Precisor, Rotator, Bottom CCD, Die Clean  -- golden :2316
{
    // golden :2318-2330 presets OutArmSuck.bAlready{Rotate,AOI,FixAI} from
    // USE_ROTATE_KIT/tRotate/MOT[MOutRotateKit]/tAOISetup/ScannerAOIIF/FrmAOI->/
    // fFixAICCD->.  Offline the additional functions are all "already done" so the
    // additional-fn SM converges immediately.  Keep that faithful end-state.
    OutArmSuck.bAlreadyRotate=true;
    OutArmSuck.bAlreadyAOI=true;
    OutArmSuck.bAlreadyFixAI=true;
}
#endif // AI(W906-OUTADD) 20260927: 離線舊版退休到這裡
//==============================================================================
//==  ACTIVE SM #2: DoOutArmPlaceToAuto_9045  -- golden :3077
//==  CENTRAL pumpable out-cycle SM (mirror of in-arm central pick SM).  switch
//==  skeleton + cursor flow + the case 1->10 and case 50->100 FALL-THROUGHS kept
//==  ACTIVE & verbatim.  Inner hardware / cross-module call bodies (CheckOutSuckICFallDown
//==  / SearchTrayToPlace_9045 / SetOutArm_9045 / MoveOutArmToAutoSafe / DoOutArmPlaceToAuto
//==  / DetectAutoTray / MOT[]) dispatch to the ACTIVE stubs above (// TODO(W7)).
//==============================================================================
bool DoOutArmPlaceToAuto_9045()                                                 // golden :3077
{
    int ct;
    bool flag;
    static int iRetry=0;
    int &Task=iPlaceToAutoTask;

    switch(Task)
    {
        case 1:
            bOutArmXOverLimit=false;                                            //Steven 20171206 (Wei) : In ATK, 修正Out Arm同時放料但是超出軟體極限縮Pitch導致疊料的問題
            InitialFix3CanFullTask();                                           //ChungHung 20140313 add Fix3 can Full Tray
            Task=10;
            // fall-through to case 10 (golden :3090 -- intentional, no break)
        case 10:
            if(CheckOutSuckICFallDown(false)==true)                             //Steven 20210916 : 全時檢查out arm掉料
            {
                Task=220;
                return false;
            }

            iWhichAuto=SearchTrayToPlace_9045();                                //Steven 20160524 : 簡化Out Arm程式碼
            IfUseOnebyOne(iWhichAuto);                                          //Steven 20111101 : 整合成function  //Steven 20210608 : 加入iWhichAuto判斷

            if(AUTO3_IS_MAGAZINE==1 &&                                          //JerryYang 20221215 : Magazine把fix區當buffer區功能
               iWhichAuto>=iMagMin  &&
               TestIF_File.iMagFixTrayType==1)
            {
                ct=iWhichBuff;
            }
            else
            {
                ct=iWhichAuto;
            }

            if(AUTO3_IS_MAGAZINE==1 && iMagazineStatus!=0)                      //JerryYang 20250920 : fixed for Multi bin
            {
                return false;
            }

            if(TRAY_ARM_MODE==eAboveCoveyor &&
               (MOT[iMMAuto[ct]].fHasTray==false ||
                (CheckUseFixBinBoxFunction()==false &&                          //kevin 20160912
                 MOT[iMMAuto[ct]].FullIC())))                                   //Steven 20091002 : 當要放IC時，編輯成滿盤會Hang Up，修正
            {
                InitOutArmTask();
                break;
            }

            if(iWhichAuto!=eBulkBox && (EnableFix3UseCylinder()))
            {
                if(UseFix3Cylinder(ct)==false)
                    break;
            }

            flag=SetOutArm_9045();
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
                if(bOverTray==true)
                {
                    MOT[iMMAuto[ct]].SetTray(HAS_IC, __FUNC__);
                    return true;
                }
            }
            break;
        case 30:
            IsCatchTrayReadySupplyNewTray();
            if(AUTO3_IS_MAGAZINE==1 &&                                          //JerryYang 20221215 : Magazine把fix區當buffer區功能
               iWhichAuto>=iMagMin  &&
               TestIF_File.iMagFixTrayType==1)
            {
                ct=iWhichBuff;
            }
            else
            {
                ct=iWhichAuto;
            }

            if(MOT[iMMAuto[ct]].fHasTray &&
               MOT[iMMAuto[ct]].FullIC()==false)
                Task=10;
            break;
        case 50:
            if(CheckOutSuckICFallDown(false)==false)                            //Steven 20110216 Start : 整合Alarm Code
            {
                iRetry=0;
                Task=100;
            }
            else
            {
                Task=220;
                return false;
            }
            // fall-through to case 100 (golden :3178 -- intentional, no break)
        case 100:
            if(OutArmNeedCheckOffset(true, iWhichAuto))                         //Steven 20230531 : 簡化判斷式
            {
                Task=200;
                break;
            }

            if(ArmSpeed[OutArm].bSuckOnDown==false &&                           //jou 2010-12-01 start : 小IC時,會偶發性置偏
               ArmSpeed[OutArm].dDestroyPauseTime!=0)
            {
                DoPlaceToAutoDelay.SetSecAndOn(ArmSpeed[OutArm].dDestroyPauseTime);
                Task=110;
            }
            else
            {
                Task=300;
            }
            break;
        case 110:
            if(DoPlaceToAutoDelay.Off()==true)
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
            if(MoveOutArmToAutoSafe())
            {
                Task=1;
                if(OutArmSuck.HasIC()==false)                                   //Steven 20220812 : 避免IC掉光後卡死
                    return true;
            }
            break;
        case 300:
#if 0 // TODO(W7) -- golden aoutarm9045.cpp:3222-3227 fAutoTeach->IsRun() (TfAutoTeach VCL form, no home)
            if(CosFunction.bManualSteplAutoTeach &&
               IniConfig.bA56EnableAutoTeachFunciton &&
               fAutoTeach->IsRun())                                             //Jimmychiu 20211020 : Auto alignment mode
            {
                iWhichAuto=iFixRight;
            }
#endif

            if(DoOutArmPlaceToAuto(iWhichAuto)==true)                           //Steven 20110216 : 整合AlarmCode
            {
                Task=400;
            }
            break;
        case 400:
            if(USE_ROTATE_KIT==1 &&
               iRotate_Type==eInOutArm1Motor &&
               USE_PICKER_COUNT==ep1Picker)
            {
                if(MoveOutArmToAutoSafe()==false)
                    break;
            }

            if(CosFunction.bNewAutoTrayDetect==true &&                          //JerryYang 20170822 (Steven) 更改Auto區偵測tray盤時機 避免out arm吸嘴下降時造成誤判
               iWhichAuto<iAutoRight)
            {
                if(MOT[iMMAuto[iWhichAuto]].Tray.FullIC())                      //JerryYang 20170919 (Steven) Auto擺滿的時候會自動退tray,不要再偵測auto tray
                {
                    ReversionEmptyPoint();
                    Task=1;
                    return true;
                }
                else
                {
                    if(MoveOutArmToAutoSafe())
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
            if(DetectAutoTray(iWhichAuto, &iRetry)==true)                       //JerryYang 20170822 (Steven) 更改Auto區偵測tray盤時機 避免out arm吸嘴下降時造成誤判
            {
                ReversionEmptyPoint();
                Task=1;
                return true;
            }
    }
    return false;
}

//==============================================================================
//==  ACTIVE SM #3: DoOutArmAfterPlaceToAuto  -- golden :3281
//==  post-place after-action SM (Fix3 full-tray, magazine buffer, ATK-AMR,
//==  clean-out sort, ion-fan give-way handoff, AOI consecutive-fail).  Returns int
//==  iResult.  switch skeleton + cursor flow + the case 1000->2000 (DoFix3FullTray
//==  true) + case 2000->3000 (commented //break) FALL-THROUGHS kept VERBATIM.
//==  Hardware/Magazine/AOI/ATK bodies dispatch to the ACTIVE stubs above.
//==============================================================================
int DoOutArmAfterPlaceToAuto(bool &bCheckSpeed, bool bInitial)                  //Steven 20220526 : 針對放下IC到Unloader後的動作做整合  -- golden :3281
{
    int iResult=0;
    int &Task=iDoOutArmAfterPlaceToAutoTask;
    if(bInitial)
    {
        Task=1;
        InitialFix3CanFullTask();
        return iResult;
    }

    switch(Task)
    {
        case 1:
            if(bAOIConsecutiveFailFlag || bAOIConsecutiveFailPictureFlag || bAOIFailCountFlag)
            {
                Task=6000;
            }
            else if(OutArmSuck.HasRealIC()==false)
            {
                if(IniConfig.bASE_Report)                                       //kevin 20150121 (jou) 高雄日月光IC履歷記錄
                    SendDataToASE(AseIcRecord);                                 //kevin 20161228

                BinBoxShiftY();                                                 //jou 2012-12-11 support Bin Box

                // golden :3306-3323 ATK-AMR branch.  The condition derefs
                // LastSet.iUnloadFixTray (==eAtkTfMoveFixIC) and the body the
                // eAtkTf* enum + MOT[iMMAuto[]].Tray -- none translated this wave.
                // Offline fAGV->IsATK_AMR() is false, so the live path is the
                // else below; GATE the whole ATK-AMR condition+body.
#if 0 // TODO(W7) -- golden aoutarm9045.cpp:3306-3323 (fAGV ATK-AMR: LastSet.iUnloadFixTray/eAtkTf*/MOT[iMMAuto[]].Tray)
                if(fAGV->IsATK_AMR() &&
                        LastSet.iUnloadFixTray==eAtkTfMoveFixIC)                //Steven 20260202 : for ATK AMR
                {
                    for(int i=iFixMin; i<=iFixMax; i++)                         //Fix Tray
                    {
                        if(MOT[iMMAuto[i]].HasRealIC())
                        {
                            InitialDoPickFromMagazineBuffer();
                            iResult=11100;
                            return iResult;
                        }
                    }
                    for(int i=iFixMin; i<=iFixMax; i++)                         //AI(ht9045-atk-amr-flow) 20260427 (RogerYang) : clear null IC after sorting
                        MOT[iMMAuto[i]].Tray.ClearNullIC();
                    LastSet.iUnloadFixTray=eAtkTfFixToAMR;                      //AI(ht9045-atk-amr-flow) 20260421 (RogerYang) : activate FixToAMR
                    Task=1;                                                     //EventReport moved to main loop FixToAMR branch
                    iResult=1;
                }
                else
#endif
                {
                    if(AUTO3_IS_MAGAZINE==1 && TestIF_File.iMagFixTrayType==1)  //JerryYang 20221215 : Magazine把fix區當buffer區功能
                    {
                        if((MagazineBufferIsEmpty(iPickWhichMag)==false &&
                            iWhichMag==iAuto3MagazineIndex && iAuto3MagazineIndex!=-1) ||
                           WhichMagazineBufferIsFull()!=-1)                     //要把buffer區清完
                        {
                            InitialDoPickFromMagazineBuffer();
                            iResult=11100;
                            return iResult;
                        }

                        if(iAuto3MagazineIndex!=-1 &&
                           MagazineBufferIsEmpty(iAuto3MagazineIndex)==false)   //如果buffer區還有AUTO3的Magazine bin別, 要放到AUTO3去
                        {
                            iPickWhichMag=iAuto3MagazineIndex;
                            InitialDoPickFromMagazineBuffer();
                            iResult=11100;
                            return iResult;
                        }
                    }

                    if(CosFunction.bUseFix3FullTray &&                          //Ifor 20161122 add Fix3 Full Tray 流程
                       FIX3_FULL_PLACE==Fix3K_Uninstall &&                      //Steven 20250911 : for 硬體 fix3 full 可以關閉
                       IniConfig.bE55UseFix3FullTray==true &&                   //Ifor 20161122 add 判斷是否有開啟 Fix3 Full Tray 功能
                       MOT[iMMAuto[iSortingTrayFix3]].Tray.HasRealIC()==true)
                    {
                        SortingBinTray_Data.iMAXSUCK_OutArmSuckX=4;
                        SortingBinTray_SetMotorPosData();                       //設定馬達、及點位資料
                        SortingBinTray_SetTrayData(iSortingTrayFix3);           //設定Tray盤資料
                        bFix3HasNullICSet=true;
                        Task=1000;
                    }
                    else
                    {
                        InitialFix3CanFullTask();
                        Task=2000;
                    }
                }
            }
            else
            {
                if(AUTO3_IS_MAGAZINE==1 && TestIF_File.iMagFixTrayType==1)      //JerryYang 20221215 : Magazine把fix區當buffer區功能
                {
                    Task=1;
                    iResult=3010;
                }
                else
                {
                    if(EnableFix3UseCylinder())                                 //JimmyChiu 20220927 : Stepper Motor Control in Fix3
                    {
                        iWhichAuto=SearchTrayToPlace_9045();                    //Steven 20160524 : 簡化Out Arm程式碼
                        if(iMMAuto[iWhichAuto]!=MManualTray3)                   //吸嘴還有料要繼續做, 而且不是去Fix3的, 先把氣缸縮回來
                        {
                            InitialFix3CanFullTask();                           //ChungHung 20140313 add Fix3 can Full Tray
                            Task=500;
                        }
                        else
                        {
                            Task=1;
                            iResult=3010;
                        }
                    }
                    else
                    {
                        Task=1;
                        iResult=3010;
                    }
                }
            }
            break;
        case 500:
            if(UseFix3Cylinder(0)==true)                                        //吸嘴還有料要繼續做
            {
                Task=1;
                iResult=3010;
            }
            break;
        case 1000:
            if(DoFix3FullTray())                                                //Ifor 20161122 執行Fix3 Full Tray 整盤功能
            {
                InitialFix3CanFullTask();
                Task=2000;
            }
            else
            {
                break;
            }
            // fall-through to case 2000 (golden :3413 -- only when DoFix3FullTray() true)
        case 2000:
            if(EnableFix3UseCylinder())                                         //JimmyChiu 20220927 : Stepper Motor Control in Fix3
            {
                if(UseFix3Cylinder(0)==true)                                    //吸嘴已經放完料
                {
                    Task=3000;
                }
                else
                {
                    break;
                }
            }
            else
            {
                Task=3000;
            }
//            break;
        case 3000:
            Task=CheckOutArmCleanOut(3100);
            if(ArmSpeed[InArm].bAutoSpeed)                                      //Steven 20110525 : Auto Speed
                bCheckSpeed=true;
            else
                bCheckSpeed=false;
            break;
        case 3099:                                                              //==> Eastsun 20260521 整合
            if(DoOutArmIonFanGiveWay())                                         //Ifor 20241127 add:
            {
                Task=1;
                iResult=100;
            }
            break;
        case 3100:
            if(IniConfig.bA15_1ESDGiveWayFunction==true)
            {
                InitDoOutArmIonFanGiveWayTask();
                Task=3099;
            }
            else
            {
            Task=1;
            iResult=100;
            }

            if(tRotate.ActiveRotate==1 &&
               iRotate_Type==eInOutArm1Motor)                                   //add One sucker with rotate
            {
                iResult+=10;
            }
            break;
        case 5000:
            if(IniConfig.bP27AutoSortingBinTrayByOutArmwhenCleanOut==true &&    //JerryYang 20170911 (Steven) Clean Out時整盤
               CosFunction.bSortingBinTraywhenCleanOut==true &&                 //是否使用Auto Sorting AutoTray Auto Sorting AutoTray的功能
               bSortingAllBinTrayFinish==false)                                 //是否所有的Bin Tray IC都整盤完成
            {
                DoSortingBinTray(0);                                            //Bin IC排序主流程
                Task=5100;
                break;
            }

            if(MoveOutArmXY_ToFix_Tray_Full())
            {
                if(iCleanOut==1)
                {
                    if(InArmSuck.HasIC()    ||
                       OutArmSuck.HasIC()   ||
                       ShuttleHasIC()       ||
                       IndexHasIC())
                    {
                        if(USE_OUT_SORT_ARM!=eartUninstall)                     //RogerYang 20250516 add for 9046AU
                        {
                            if(OutArm2Suck.HasIC())
                            {
                                Task=1;
                                iResult=1;
                            }
                        }
                        else
                        {
                            Task=1;
                            iResult=1;
                        }
                    }
                }
            }
            break;
        case 5100:                                                              //JerryYang 20150910 Auto Sorting BinTray by Out Arm when Clean Out
            if(DoSortingBinTray()==true)                                        //Bin IC排序主流程
            {
                if(bSortingAllBinTrayFinish==true)                              //是否所有的Bin Tray IC都整盤完成
                {
                    Task=5000;
                }
            }
            break;
        case 6000:
            if(MoveOutArmXY_ToFix_Tray_Full())
            {
                if(bAOIConsecutiveFailFlag)
                {
                    ShowMyMessage("AOI Consecutive Fail");
                    bAOIConsecutiveFailFlag=false;
                }

                if(bAOIConsecutiveFailPictureFlag)
                {
                    ShowMyMessage("AOI Consecutive Picture Fail");
                    bAOIConsecutiveFailPictureFlag=false;
                }

                if(bAOIFailCountFlag)
                {
                    ShowMyMessage("AOI Accumulated Count Fail");
                    bAOIFailCountFlag=false;
                }
                Task=1;
            }
            break;
    }

    return iResult;
}

//==============================================================================
//==  ACTIVE SM #4: DoOutArmIonFanGiveWay  -- golden :4613 (Eastsun 20260521)
//==  ESD ion-fan give-way SM.  Substrate-pumpable: gating predicates are
//==  OutArmSuck.HasRealIC()/HasIC + FRCarryKit/BRCarryKit.HasIC + OutSHT1InRT/
//==  OutSHT2InRT (csystem predicates) + InArmSuck/ShuttleHasIC/IndexHasIC +
//==  WhichAutoNeedTray + MOT[MMPlate1/2].HasIC.  Cursor flow + early-out kept
//==  ACTIVE & verbatim (no fall-throughs); only the 2 motion bodies
//==  (MoveOutArmZToPlateSafe / MoveOutArm2XYToDecayTeach) dispatch to the ACTIVE
//==  stubs above, and the MOT[MMPlate1/2].HasIC reads in case 5 are gated.
//==============================================================================
void InitDoOutArmIonFanGiveWayTask()                                            //Eastsun 20260521 整合  -- golden :4608
{
    iOutArmIonFanGiveWayTask=1;
}
//------------------------------------------------------------------------------
bool DoOutArmIonFanGiveWay()                                                    //Eastsun 20260521 整合  -- golden :4613
{
    bool bResult=false;
    int &Task=iOutArmIonFanGiveWayTask;

    if(IniConfig.bA15_1ESDGiveWayFunction==false)
    {
        iOutArmIonFanGiveWayTask=1;
        return true;
    }

    switch(Task)
    {
        case 1:
            if(IniConfig.bA15_1ESDGiveWayFunction==true && OutArmSuck.HasRealIC()==false)
            {
                if(OutArmSuck.HasRealIC()==false)
                {
                    if((FRCarryKit.HasIC() && OutSHT1InRT()) ||
                       (BRCarryKit.HasIC() && OutSHT2InRT()))
                    {
                        Task=5;
                    }
                    else
                    {
                        Task=2;
                    }
                }
                else
                {
                    Task=5;
                }
            }
            else
            {
                Task=5;
            }
            break;
        case 2:
            if(MoveOutArmZToPlateSafe()==true)
            {
                Task=3;
            }
            break;
        case 3:
            if(MoveOutArm2XYToDecayTeach())
            {
                RecordProcess("Out Arm Ion Fan Give Way");
                Task=5;
            }
            break;
        case 5:
            if(IniConfig.bA15_1ESDGiveWayFunction==true)
            {
                if((FRCarryKit.HasIC() && OutSHT1InRT()) ||
                   (BRCarryKit.HasIC() && OutSHT2InRT()))
                {
                    bResult=true;
                    bOutArmIonFanGiveWay=false;
                }
                else if(WhichAutoNeedTray()!=0)
                {
                     bResult=true;
                     bOutArmIonFanGiveWay=false;
                }
                else  if(InArmSuck.HasIC()==false   &&
                         OutArmSuck.HasIC()==false  &&
                         ShuttleHasIC()==false      &&
                         IndexHasIC()==false        )
//                         bOutArmIonFanGiveWay==false )
                {
                    if(bOutArmIonFanGiveWay==false)
                    {
                        bOutArmIonFanGiveWay=false;
                        bResult=true;
                    }
                    else
                    {
                        bool bPlate1HasIC=false, bPlate2HasIC=false;
#if 1 // was: #if 0 -- opened AI(W906-POOL2) 20261008 (Ifor01): TODO(W7) MMPlate HasIC reason expired -- TTrayMotor::HasIC exists (Motor/mymotor.h:364) and MMPlate1/2 are bound; golden :4692-4693
                        bPlate1HasIC=MOT[MMPlate1].HasIC();
                        bPlate2HasIC=MOT[MMPlate2].HasIC();
#endif
                        if(iCleanOut==1                 &&
                           bPlate1HasIC==false          &&
                           bPlate2HasIC==false          )
                        {
                            bOutArmIonFanGiveWay=true;
                            bResult=true;
                        }
                    }
                }
            }
            else
            {
                bResult=true;
            }
            break;
    }

    return bResult;
}

//==============================================================================
//==  ACTIVE: InspectOutArmPosition  -- golden :3865.  IniConfig.bE74 early-return
//==  is the live offline behavior (debug flag off by default); the bX/bY compare
//==  body (MOT[] encoders + ShowErrorMessage alarm sink) is gated.
//==============================================================================
// ===========================================================================
//  GOLDEN VERBATIM PAIR -- InspectOutArmPosition()
//  GATED : golden aoutarm9045.cpp:3865-3993 (129 lines), inert reference text.
//  LIVE  : the InspectOutArmPosition() body immediately AFTER the #endif below.  It is UNCHANGED by
//          this wave -- net behaviour change is ZERO.
//  WHY   : the census scored this "translated" because a same-named LIVE body
//          existed, without comparing SIZE.  Golden's 129 lines were NOWHERE in
//          the tree -- lost text, not deferred behaviour.  Now the text EXISTS
//          and is auditable, so a later un-gate is mechanical.
//  NOTE  : golden's body is the bX/bY encoder compare + WAR0263/WAR0264 alarm sink.
//  RULES : nothing inside the gate is fixed, renamed, reflowed or reindented;
//          it is golden's bytes.  Nothing it references had to be made to
//          exist -- no stub, declaration or header was added for it.
//  SHAPE : same pair shape as csystem.cpp MainProc / atester.cpp (PT-W6a/W6b)
//          golden-verbatim gates.
// ===========================================================================
#if 0 // GOLDEN VERBATIM -- golden aoutarm9045.cpp:3865-3993 (129 lines).  GATE G-PTW6c-InspectOutArmPosition.  NOT COMPILED: the ACTIVE InspectOutArmPosition() is the body immediately after this #endif.
void InspectOutArmPosition(int iTarget, int iSuckRow, int iSuckCol, int iTargetRow, int iTargetCol, int iAction)
{
    if(IniConfig.bE74_InspectArmPosition==false)                                //Jimmychiu 20240408 : debug for inarm position
        return;

    bool bX, bY;
    int iOutPutTray;
    AnsiString sError, sPos="";
    AnsiString sPlace;

    if(iAction==iOutPlaceToAuto)
    {
        sPlace="Place to";
        sPos=s6TrayName[iTarget];

        if(AUTO3_IS_MAGAZINE==1             &&                                  //JerryYang 20221215 : Magazine把fix區當buffer區功能
           TestIF_File.iMagFixTrayType==1   &&
           iTarget>=iMagMin)
        {
            iOutPutTray=iWhichBuff;
        }
        else
        {
            iOutPutTray=iTarget;
        }
    }
    else
    {
        return;                                                                 //Steven 20241220 : 暫時跳過
        sPlace="Pick from";
        if(iTarget==MOutShuttle1)
        {
            sPos="Shuttle 1";
        }
        else if(iTarget==MOutShuttle2)
        {
            sPos="Shuttle 2";
        }
        else if(iTarget==MOutRotateKit)
        {
            sPos="Out Rotator";
        }
    }

    int XEncoder=0, YEncoder=0, HardwarePosX=0, HardwarePosY=0;

    if(OutArmSuck.Item[iSuckRow][iSuckCol]!=HAS_NULL_IC &&
       OutArmSuck.Item[iSuckRow][iSuckCol]!=HAS_NULL_CLEAN_IC)
    {
        if(iTarget==MOutShuttle1 ||
           iTarget==MOutShuttle2)
            GetOutArmToShtCellPos(iSuckRow, iSuckCol, YEncoder, XEncoder);
        else
            GetOutArmCellPos(iSuckRow, iSuckCol, YEncoder, XEncoder);

        if(iAction==iOutPlaceToAuto)
        {
            if(iTarget==eBulkBox)
            {
                HardwarePosX=Prod.iOutArmBinBoxX+dOutArmXPitch_1Step*iTargetCol;
                HardwarePosY=Prod.iOutArmBinBoxY-Prod.iOutArmBinBoxY*200;       //待確認
            }
            else                                                                //Jimmychiu 20240731 : remove offset value
            {
                HardwarePosX=Prod.XStart[iOutPutTray][iOutArmYBase][iOutArmXBase]+AutoForm[iOutPutTray]->XPitch*iTargetCol;                                     //Jimmy 20240826 : fixed for E74
                HardwarePosY=Prod.YStart[iOutPutTray][iOutArmYBase][iOutArmXBase]-AutoForm[iOutPutTray]->YPitch*iTargetRow;

                if(USE_PICKER_COUNT==ep1Picker)
                    HardwarePosX=HardwarePosX;
                else
                    HardwarePosX+=((OutArmOffSet[GetOutOffsetFromWhichAuto(iOutPutTray)]->GetVariable())/3)*(iSuckCol-iOutArmXBase);

                HardwarePosY-=OutArmOffSet[GetOutOffsetFromWhichAuto(iOutPutTray)]->GetVariableY()*(iSuckRow-iOutArmYBase);
            }
        }
        else
        {
            if(iTarget==MOutShuttle1 ||
               iTarget==MOutShuttle2)
            {
                GetShtRowColStartPos(iTarget,HardwarePosX,HardwarePosY);
                HardwarePosX+=TestIF.dSiteXPitch*iTargetCol;
                HardwarePosY-=TestIF.dSiteYPitch*iTargetRow;
                int iXoffset=0, iYoffset=0;
                if(iTarget==MOutShuttle1)
                {
                    iXoffset=OutArmOffSet[OutOfsOutSh1]->GetVariable();
                    iYoffset=OutArmOffSet[OutOfsOutSh1]->GetVariableY();
                }
                else                                                            //MOutShuttle2
                {
                    iXoffset=OutArmOffSet[OutOfsOutSh2]->GetVariable();
                    iYoffset=OutArmOffSet[OutOfsOutSh2]->GetVariableY();
                }

                if(USE_PICKER_COUNT==ep1Picker)
                    HardwarePosX=HardwarePosX;
                else
                    HardwarePosX+=(iXoffset/3)*(iSuckCol-iOutArmXBase);

                HardwarePosY-=iYoffset*(iSuckRow-iOutArmYBase);
                if(TestIF.bNS7000kit)                                           //jou 981208 start : NS7000 bias kit
                {
                    int iShiftY=(TestIF.dSiteYPitch==0)?(6000/2):(TestIF.dSiteYPitch/2);
                    if(iTarget==MOutShuttle1)
                        HardwarePosY+=iShiftY;
                    else
                        HardwarePosY-=iShiftY;
                }
            }
            else if(iTarget==MOutRotateKit)                                     //待確認
            {
                HardwarePosX=Prod.iOutArmRotateToUnloaderX;
                HardwarePosY=Prod.iOutArmRotateToUnloaderY;
            }
        }

        int iLimit=100*InputLimit.iOffsetXYHigh;                                //Jimmychiu 20240731 : remove offset value
        bX=MOT[MOutArmX].CheckArmPosArrival(HardwarePosX, XEncoder, iLimit);    //Steven 20240719 : 放寬檢查的範圍
        bY=MOT[MOutArmY].CheckArmPosArrival(HardwarePosY, YEncoder, iLimit);
        if(bX==false || bY==false)
        {
            sError.sprintf("OutArm Suck[%d, %d] Pos(Y=%d, X=%d), \r%s %s [%d, %d] Pos(Y=%d, X=%d)",
                           iSuckRow, iSuckCol, YEncoder, XEncoder, sPlace, sPos, iTargetRow, iTargetCol, HardwarePosY, HardwarePosX);

            ShowMyMessage(sError);
        }
    }
}
#endif // GOLDEN VERBATIM -- golden aoutarm9045.cpp:3865-3993  (GATE G-PTW6c-InspectOutArmPosition, end)
void GetShtRowColStartPos(int iTarget,int &iColStart,int &iRowStart);           // ainarm9045.h:148 (body ainarm9045.cpp) -- InspectOutArmPosition below; the whole header clashes with this file's own InArmLeftSide* declarations, golden reaches it through aArmHeader.h  //AI(W906-POOL2) 20261008 (Ifor01)
void InspectOutArmPosition(int iTarget, int iSuckRow, int iSuckCol, int iTargetRow, int iTargetCol, int iAction)   // golden :3865
{
    if(IniConfig.bE74_InspectArmPosition==false)                                //Jimmychiu 20240408 : debug for inarm position
        return;

#if 1 // was: #if 0 -- opened AI(W906-POOL2) 20261008 (Ifor01): TODO(W7) InspectOutArmPosition translated -- the gate held only a comment; golden :3870-3992 below verbatim (cp950 -> UTF-8). Every dependency exists: GetOutArm(ToSht)CellPos read the encoders (opened above), OutArmOffSet[], GetOutOffsetFromWhichAuto (declared in aoutarm9045.h as golden :98), GetShtRowColStartPos, AutoForm[], s6TrayName.  E74 debug option only (early return above)
    bool bX, bY;
    int iOutPutTray;
    AnsiString sError, sPos="";
    AnsiString sPlace;

    if(iAction==iOutPlaceToAuto)
    {
        sPlace="Place to";
        sPos=s6TrayName[iTarget];

        if(AUTO3_IS_MAGAZINE==1             &&                                  //JerryYang 20221215 : Magazine把fix區當buffer區功能
           TestIF_File.iMagFixTrayType==1   &&
           iTarget>=iMagMin)
        {
            iOutPutTray=iWhichBuff;
        }
        else
        {
            iOutPutTray=iTarget;
        }
    }
    else
    {
//        return;                                                                 //Steven 20241220 : 暫時跳過   //AI(W906-W188) 20261010 (NB2-1): golden 913 aoutarm9045.cpp:4199 (commented out)
        sPlace=(iAction==iOutPlaceToRotate)?"Place to":"Pick from";   //AI(W906-W188) 20261010 (NB2-1): golden 913 aoutarm9045.cpp:4200-4201
        if(iTarget==MOutShuttle1)
        {
            sPos="Shuttle 1";
        }
        else if(iTarget==MOutShuttle2)
        {
            sPos="Shuttle 2";
        }
        else if(iTarget==MOutRotateKit)
        {
            sPos="Out Rotator";
        }
    }
    if(iAction==iOutPlaceToAuto && iTarget==eBulkBox) return;   //AI(W906-W188) 20261010 (NB2-1): golden 913 aoutarm9045.cpp:4216-4224 (eFix12==MOutShuttle1==17, eBulkBox==MOutShuttle2==18: a place to BulkBox is not checked)
    int XEncoder=0, YEncoder=0, HardwarePosX=0, HardwarePosY=0;

    if(OutArmSuck.Item[iSuckRow][iSuckCol]!=HAS_NULL_IC &&
       OutArmSuck.Item[iSuckRow][iSuckCol]!=HAS_NULL_CLEAN_IC)
    {
        if(iAction!=iOutPlaceToAuto && (iTarget==MOutShuttle1 ||   //AI(W906-W188) 20261010 (NB2-1): golden 913 aoutarm9045.cpp:4231-4233 (a place to Fix12 / BulkBox must not take the shuttle formula)
           iTarget==MOutShuttle2))
            GetOutArmToShtCellPos(iSuckRow, iSuckCol, YEncoder, XEncoder);
        else
            GetOutArmCellPos(iSuckRow, iSuckCol, YEncoder, XEncoder);

        if(iAction==iOutPlaceToAuto)
        {
//            if(iTarget==eBulkBox)   //AI(W906-W188) 20261010 (NB2-1): golden 913 aoutarm9045.cpp:4240-4248 (BulkBox returned above; block commented out)
//            {
//                HardwarePosX=Prod.iOutArmBinBoxX+dOutArmXPitch_1Step*iTargetCol;
//                HardwarePosY=Prod.iOutArmBinBoxY-Prod.iOutArmBinBoxY*200;       //待確認
//            }
//            else                                                                //Jimmychiu 20240731 : remove offset value
            {
                HardwarePosX=Prod.XStart[iOutPutTray][iOutArmYBase][iOutArmXBase]+AutoForm[iOutPutTray]->XPitch*iTargetCol;                                     //Jimmy 20240826 : fixed for E74
                HardwarePosY=Prod.YStart[iOutPutTray][iOutArmYBase][iOutArmXBase]-AutoForm[iOutPutTray]->YPitch*iTargetRow;

                if(USE_PICKER_COUNT==ep1Picker)
                    HardwarePosX=HardwarePosX;
                else
                    HardwarePosX+=((OutArmOffSet[GetOutOffsetFromWhichAuto(iOutPutTray)]->GetVariable())/3)*(iSuckCol-iOutArmXBase);

                HardwarePosY-=OutArmOffSet[GetOutOffsetFromWhichAuto(iOutPutTray)]->GetVariableY()*(iSuckRow-iOutArmYBase);
            }
        }
        else
        {
            if(iTarget==MOutShuttle1 ||
               iTarget==MOutShuttle2)
            {
                GetShtRowColStartPos(iTarget,HardwarePosX,HardwarePosY);
                HardwarePosX+=TestIF.dSiteXPitch*iTargetCol;
                HardwarePosY-=TestIF.dSiteYPitch*iTargetRow;
                int iXoffset=0, iYoffset=0;
                if(iTarget==MOutShuttle1)
                {
                    iXoffset=OutArmOffSet[OutOfsOutSh1]->GetVariable();
                    iYoffset=OutArmOffSet[OutOfsOutSh1]->GetVariableY();
                }
                else                                                            //MOutShuttle2
                {
                    iXoffset=OutArmOffSet[OutOfsOutSh2]->GetVariable();
                    iYoffset=OutArmOffSet[OutOfsOutSh2]->GetVariableY();
                }

                if(USE_PICKER_COUNT==ep1Picker)
                    HardwarePosX=HardwarePosX;
                else
                    HardwarePosX+=(iXoffset/3)*(iSuckCol*OutArmSuck.iPickStep-iOutArmXBase);   //AI(W906-W188) 20261010 (NB2-1): golden 913 aoutarm9045.cpp:4284-4288 (same column step as GetOutArmToShtCellPos)

                HardwarePosY-=iYoffset*(iSuckRow-iOutArmYBase);
                if(TestIF.bNS7000kit)                                           //jou 981208 start : NS7000 bias kit
                {
                    int iShiftY=(TestIF.dSiteYPitch==0)?(6000/2):(TestIF.dSiteYPitch/2);
                    if(iTarget==MOutShuttle1)
                        HardwarePosY+=iShiftY;
                    else
                        HardwarePosY-=iShiftY;
                }
            }
            else if(iTarget==MOutRotateKit)                                     //待確認
            {   extern int iRotateKIT_Start_X_H; extern int iRotateKIT_Start_Y_H; extern int iRotateKIT_Pitch_X_H; extern int iRotateKIT_Pitch_Y_H;   // golden reaches these through aRotateKIT.h:89-92
                int iRotColMax=(tRotate.ColCount>1)?(tRotate.ColCount-1):1; HardwarePosX=Prod.iOutArm_RotateX+iRotateKIT_Start_X_H-iRotateKIT_Pitch_X_H*(iRotColMax-iTargetCol);   //AI(W906-W188) 20261010 (NB2-1): golden 913 aoutarm9045.cpp:4300-4313 (each rotator hole, not one fixed point)
                HardwarePosY=Prod.iOutArm_RotateY-iRotateKIT_Start_Y_H-iRotateKIT_Pitch_Y_H*(iTargetRow-iOutArmYBase);
            }
        }

        int iLimit=100*InputLimit.iOffsetXYHigh;                                //Jimmychiu 20240731 : remove offset value
        bX=MOT[MOutArmX].CheckArmPosArrival(HardwarePosX, XEncoder, iLimit);    //Steven 20240719 : 放寬檢查的範圍
        bY=MOT[MOutArmY].CheckArmPosArrival(HardwarePosY, YEncoder, iLimit);
        if(bX==false || bY==false)
        {
            sError.sprintf("OutArm Suck[%d, %d] Pos(Y=%d, X=%d), \r%s %s [%d, %d] Pos(Y=%d, X=%d)",
                           iSuckRow, iSuckCol, YEncoder, XEncoder, sPlace, sPos, iTargetRow, iTargetCol, HardwarePosY, HardwarePosX);

            ShowMyMessage(sError);
        }
    }
#else
    (void)iTarget; (void)iSuckRow; (void)iSuckCol; (void)iTargetRow; (void)iTargetCol; (void)iAction;
#endif
}

//==============================================================================
//==  ACTIVE: IsMotorArrival -- golden :3802.  Pure arithmetic via TMyMotor
//==  CheckArmPosArrival; gate the MOT[].ReadPos() encoder read (offline 0).
//==============================================================================
bool IsMotorArrival(int iMot,int iTargetPos)                                    //Jimmychiu 20240726 : Fill The Tray After Out Arm Place  -- golden :3802
{
    int iReadPos=0;
#if 1 // was: #if 0 -- opened AI(W906-POOL2) 20261008 (Ifor01): TODO(W7) IsMotorArrival reason expired -- MOT[].ReadPos() is the live encoder/command read (the in-arm twin GetInArmCellPos reads it ungated, ainarm9045.cpp); offline the sim drivers return their position; golden :3804
    iReadPos=MOT[iMot].ReadPos();
#endif
    (void)iMot;
    const int iArrivalGap=10;                                                   // golden :3800
    bool b=TMyMotor().CheckArmPosArrival(iTargetPos, iReadPos, iArrivalGap);
    return b;
}

//==============================================================================
//==  ACTIVE: MoveOutArmXY_To_ESDSafePos -- golden :4711.  Keep the bAlarmNeedServoOff
//==  guard + the position math STRUCTURE; gate the fNote / MOT[].ReadPos() reads   //AI(W906-FLOW-5) 20260929: the fNote read is no longer gated (:2174 retired); the MOT[].ReadPos() reads and the stub move still are (other gates, not this item)
//==  and route the final move through the OutArmContinuousMove_9045 stub.
//==============================================================================
bool MoveOutArmXY_To_ESDSafePos()                                               //Eastsun 20260521 整合  -- golden :4711
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
//#if 0 // TODO(W7) -- golden aoutarm9045.cpp:4724 fNote->bMyServoOffOutArm (TfNote VCL form, no home)   //AI(W906-FLOW-5) 20260929: gate RETIRED -- golden aoutarm9045.cpp:4722-4728 (MoveOutArmXY_To_ESDSafePos) restored; the stated reason is false since AI(W906-HOME-C2) 20260920: TfNote::bMyServoOffOutArm exists (forms/fNote.h:305, ctor false forms/fNote.cpp:42) on the one live fNote (forms/fNote.cpp:48) and is already read live at aoutarm9045.cpp:762 (DoOutArm_9045, FLOW-2); the only reason a gate may stay is a missing dependency, and there is none. Inert until a writer sets it (golden main.cpp:7182/:7215, note.cpp:2074 -- not translated yet); when one lands this refuses the move exactly as golden does
        if(fNote->bMyServoOffOutArm)
        {
            return false;
        }
//#endif   //AI(W906-FLOW-5) 20260929: end of the retired TODO(W7) gate (:2174)
    }

    iXPos=Prod.XOutArm_Shuttle1_Pick[iOutArmYBase][iOutArmXBase];
    iYPos=((Prod.YOutArm_Shuttle1_Pick[iOutArmYBase][iOutArmXBase]+Prod.YOutArm_Shuttle2_Pick[iOutArmYBase][iOutArmXBase])/2);

#if 1 // was: #if 0 -- opened AI(W906-POOL2) 20261008 (Ifor01): TODO(W7) ESD safe-pos pitch reads reason expired -- MOT[].ReadPos() is the live encoder/command read (the in-arm twin GetInArmCellPos reads it ungated, ainarm9045.cpp); offline the sim drivers return their position; golden :4733-4747
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
        iYVariable   =MOT[MOutArmPitchY].ReadPos();
    }
#endif

    if(OutArmContinuousMove_9045(iXPos, iYPos, iXVariable, iYVariable, bZFlag, iZPos, false))
        return true;
    return false;
}

//==============================================================================
//==  ACTIVE offline stubs for the remaining golden out-arm engine surface.
//==  Each is declared in aoutarm9045.h; the golden body is MOT[]/cross-module/
//==  UI-bound with no translated home this wave.  Offline-safe defaults keep the
//==  engine linkable.  Golden file:line cited per symbol.  TODO(W7).
//==============================================================================
// AI(W906-ARM2) 20260927: RETIRED stub DoMoveOutArmXYToPlace_9045（以前回常數）—— real body at end of file (golden :2333-2449)
// AI(W906-ARM2) 20260927: RETIRED stub SetOutArm_9045（以前回常數）—— real body at end of file (golden :2493-2523)
// AI(W906-ARM2) 20260927: RETIRED stub SearchTrayToPlace_9045（以前回常數）—— real body at end of file (golden :1316-1418)
// AI(W906-ARM2) 20260927: RETIRED stub SearchUnLoadTrayUpDown_9045（以前回常數）—— real body at end of file (golden :2451-2491)
// AI(W906-ARM1) 20260927: RETIRED stub VerifyFixTrayLink（以前回 0）—— real body at end of file (golden :1600-1672)
// AI(W906-ARM1) 20260927: RETIRED stub InitialOutArmNeedSuck（以前回 false ⇒ 出料臂永遠不下去吸）—— real body at end of file (golden :3535-3590)
// AI(W906-ARM4) 20260927: RETIRED stub UseFix3Cylinder（以前回 true）—— real body at end of file (golden :794-1186)
// AI(W906-ARM1) 20260927: RETIRED stub OutArmNeedCheckOffset（以前回 false）—— real body at end of file (golden :3592-3627)
// AI(W906-ARM4) 20260927: RETIRED stub Check_QA_ModeUnloadCount（以前回 false）—— real body at end of file (golden :3766-3798)
// AI(W906-ARM4) 20260927: RETIRED stub InitialFix3CanFullTask（以前什麼都不做）—— real body at end of file (golden :749-752)

//=============================================================================
// BANNER EXTENSION -- PT-W8 (aoutarm9045.cpp, 29 golden functions + 3 globals)
// Translator: AI(ht9045-v906) 20260811.  Append-only; nothing above this line
// was rewritten or reordered.
//
// ROLE (unchanged): out-arm ENGINE.  This batch adds the Fix-tray full/alarm
//   ladder, the tray/magazine SEARCH helpers, the shuttle-offset pickers, the
//   out-arm-to-task50 predicate, the tray-status verifier, the whole
//   "Fill The Tray After Out Arm Place" family (Jimmychiu 20240726), and the
//   9046AU sort-shuttle offset/place helpers (RogerYang 20250510/13).
//   Pumped by: DoOutArm_9045 / DoOutArmPlaceToAuto_9045 / DoOutArmAfterPlaceToAuto
//   (this file) and by the 27 aoutarm9045_<layout>.cpp variant SMs, which all
//   already forward-declare these symbols locally.
//
// WAVE SCOPE -- one line per assigned golden function (golden aoutarm9045.cpp):
//   :754   Fix3MoveToLeft                         ACTIVE
//   :793   TQPF_Timer Fix3CylinderDelay           ACTIVE (file-scope global)
//   :1188  GetVariableYOutShuttleData             ACTIVE
//   :1211  GetOutArmToShuttleOffset_9045          ACTIVE
//   :1290  GetOutArmXToShuttleOffset_9045         ACTIVE
//   :1303  GetOutArmYToShuttleOffset_9045         ACTIVE
//   :1420  SearchTrayToPlace_Magazine             ACTIVE (one gated callee: G1)
//   :1561  SearchTrayToPick_Buffer                ACTIVE (one supplied constant: N1)
//   :1674  DoFixTrayFullAlarm                     ACTIVE
//   :3629  CheckOutArmToTask50                    ACTIVE
//   :3675  VerifyTrayStatus                       ACTIVE
//   :3995  GetOutOffsetFromWhichAuto              ACTIVE
//   :4081  MyFillTheTrayAfterOutArmPlace mFillTray ACTIVE (file-scope global)
//   :4082  IsEnableFillTheTrayAfterOutArmPlace    ACTIVE
//   :4092  IsRunWhichAtuoFillTray                 ACTIVE
//   :4110  RecTrayICStatus                        ACTIVE
//   :4127  HasGapsInTheTray                       ACTIVE
//   :4167  DetermineFeasibilityOfSuck             ACTIVE
//   :4193  GetWhichAutoStart                      ACTIVE
//   :4208  GetWhichAutoPickZ                      ACTIVE
//   :4221  GetWhichAutoPlaceZ                     ACTIVE
//   :4234  IsOutArmPosOutLimit                    ACTIVE
//   :4248  FindArmSuck                            ACTIVE
//   :4270  MoveOutArmXYAndSuck(SingleSuckPosOnTray,uPoint2D)  ACTIVE
//   :4325  IsOutArmArrival                        ACTIVE
//   :4332  int iOutArmFillTheTrayAfterPlaceAllICTask ACTIVE (file-scope global)
//   :4333  DoOutArmFillTheTrayAfterPlaceAllIC     ACTIVE
//   :4464  GetOutArmXToSortShtOffset              ACTIVE
//   :4474  GetOutArmYToSortShtOffset              ACTIVE
//   :4484  NeedPlaceToSort                        ACTIVE
//   :4508  SetSortShuttleStatus_Place             ACTIVE
//   :4536  InitNewFixTrayForUnloaderClipRead      GATED body (G2), ACTIVE shell
//
//   NOT IN SCOPE (deliberately): the SECOND MoveOutArmXYAndSuck overload,
//   golden :4305 `bool MoveOutArmXYAndSuck(ArmAndSuckInfo asInfo)` -- it is a
//   different function at a different golden line and was not in this batch.
//   No call site of it is translated in this tree today (checked: `grep -rn
//   "MoveOutArmXYAndSuck" D:/HT9045/HT9011UC_Cpp_V3.33.906.0` -> 0 hits before
//   this file, 20260811), so leaving it out breaks nothing; the Auto-Cal-Suck-Z
//   family (golden :4614+/MoveOutArmXYAndSuckUp) that consumes it is also
//   untranslated.  Listed here so the next wave does not think it is done.
//
// GATE REGISTER -- every `#if 0` / gated callee this block introduces:
//
//  (G1) CheckPlaceToBufferTray(int)   -- golden Magazine.h:149, body golden
//       Magazine.cpp:376-413.  Call site kept BYTE-IDENTICAL to golden
//       (:1532).  Shape (b): file-local `static` stand-in.
//       ABSENCE CLAIM + COMMAND + TIME: Magazine.cpp / Magazine.h have no port
//       anywhere in this tree.  Verified with
//         rg -n "CheckPlaceToBufferTray" D:/HT9045/HT9011UC_Cpp_V3.33.906.0
//       -> the ONLY hits are golden-citation comments; no declaration and no
//       definition.  Run 20260811 at the START of this wave and RE-RUN at the
//       END (both runs clean; see report).
//       WHY `false` IS FAITHFUL -- AI(pt-wave) 20260811 PT-W8 integrate, this
//       entry REPLACES an earlier one that argued for `true`.  The struck text
//       read: "Offline the buffer tray is freshly initialised (every Tray.Data
//       cell NULL_IC) while iNeedPlace is at most the nozzle count, so golden's
//       OWN body evaluates to true."  That derivation reaches the opposite
//       conclusion from the code it cites.  Golden's body counts NULL_IC cells
//       only INSIDE the band `j>=k*iYRegNum && j<(k+1)*iYRegNum` (golden
//       Magazine.cpp:385-387), and iYRegNum is 0 in both trees (golden
//       cmydef.cpp:5523, port cmydef.cpp:5541) -- so the band is `j>=0 && j<0`,
//       empty for every j, and iCnt is 0 no matter how empty the tray is.
//       iNeedPlace is likewise 0 offline (OutArmSuck.Item zero-init, NULL_IC==0
//       at cmydef.cpp:153).  `if(iCnt<=iNeedPlace) return false;` => 0<=0 =>
//       GOLDEN RETURNS false.  `false` is therefore the derived value, and it is
//       also the cautious one for a capacity predicate.
//       REAL-MACHINE DELTA: on a Magazine machine with a configured iYRegNum and
//       genuine room in the band, golden returns true and the IC is placed in the
//       buffer.  This stand-in always reports "no room", so SearchTrayToPlace_
//       Magazine takes golden's divert path instead (`iOutArmWhichAuto=
//       iMagAtAuto; return 2;`) and sends the IC to Auto3.  Retire it together
//       with Magazine.cpp -- pure deletion of one line.
//       REACHABILITY TODAY: the enclosing branch needs
//       `AUTO3_IS_MAGAZINE==1 && TestIF_File.iMagFixTrayType==1 &&
//       OutArmSuck.iWhichAuto[i][j]>=iMagMin`; none of those is true in the
//       offline recipe, so the stand-in is currently unreached.
//
//  (G2) InitNewFixTrayForUnloaderClipRead -- golden :4536-4605.  The BODY is
//       `#if 0`-gated; the shell (`bool bret=false;` ... `return bret;`) is
//       golden's own first and last statement and stays ACTIVE.
//       WHY: every line of the body derefs `fBarCode->eUnloaderClipAuto1/…` and
//       `fBarCode->ccdUnloader[…]`.  In this tree `fBarCode` is NOT golden's
//       TfBarCode: it is `TfBarCode *fBarCode` (aHotPlateSubstrate.h:984-
//       1026), a 25-method offline facade that carries NEITHER the
//       `enum eMulti2DType` (golden BarCode/BarCode.h:966-975) NOR the
//       `uCCDUnloaderClip ccdUnloader[eUnloaderClipTotal]` array (golden
//       BarCode/BarCode.h:976).  Extending that shim is an edit to
//       aHotPlateSubstrate.h, which is NOT this wave's file.
//       NOTE, and it matters: `uCCDUnloaderClip` ITSELF *is* translated
//       (BarCode/BarCode_Helpers.h:109 / .cpp:57-265) -- so this is trap 3,
//       not a dead premise.  The missing piece is only the OWNER (the
//       `ccdUnloader[]` member on the form facade), not the class.
//       WHY `false` IS FAITHFUL: with no CCD wired,
//       `ccdUnloader[i].bGetResult` is false (ctor, BarCode_Helpers.cpp:57) and
//       `DoGetPhotoCmd(false,sErr)` returns "" with sErr set only when a socket
//       connect is attempted -- in this tree the socket is gated, so both sGet
//       and sErr come back empty, neither inner branch runs, and golden's own
//       `bret` is still false at `return bret;`.  Identical value, same path.
//       REAL-MACHINE DELTA: on a machine with 海康 unloader-clip readers the
//       body reads the clip code, calls InitNewFixTray(...) and stores the code
//       into MOT[iMMAuto[]].Tray.cCassetteID, returning true.  Gated, it never
//       initialises the new Fix tray and DoFixTrayFullAlarm therefore returns
//       false on that path.  Both call-site guards
//       (`CosFunction.bReadClipCodeFromUnloader && IniConfig.bP60ReadClipCode
//       FromUnloader`) are false offline, so nothing reaches it today.
//       RETIRE WHEN: BarCode/BarCode.cpp (TfBarCode) lands, or the shim gains
//       `eMulti2DType` + `ccdUnloader[]`.  Retirement is deleting two lines.
//
//  (N1) eAtkTfMoveFixIC = 6  -- golden Automation/AGV.h:218, `enum eATkTrayFeed`.
//       This is NOT a gate: it is golden's VERBATIM value, read out of golden,
//       supplied file-locally because AGV.h has no port.  Exactly the shape
//       aoutarm.cpp:527-530 and aoutarm9045_2x8_8.cpp:320-326 already use for
//       this same enumerator.  ABSENCE CLAIM + COMMAND + TIME:
//         rg -n "eAtkTfMoveFixIC" D:/HT9045/HT9011UC_Cpp_V3.33.906.0 --glob *.h
//       -> 0 hits (run 20260811, start AND end of wave).  The only definitions
//       in the tree are the two file-local ones cited above, both `static`, so
//       adding a third file-local one cannot collide.
//=============================================================================
#include "mysensor.h"              // Sen[] (SnFixedTrayDetect / SnAutoTrayDetect / SnFix3FullPlace)
#include "mycylin.h"               // Cylinder[] (C_FixTray_FullPlace / C_AutoSide_Fixer / C_AutoEdgePush / C_AutoUpPress)
#include "SECSGEM/SecsEventType.h" // SECS_EVENT.Fix1..6Full / Fix1PortStatusChanged
#include "SECSGEM/SecsEventReport.h" // EventReport(unsigned)
#include "myTimer.h"               // TQPF_Timer (golden's Fix3CylinderDelay type)

//-----------------------------------------------------------------------------
//  (T5) SingleSuckPosOnTray / MyFillTheTrayAfterOutArmPlace -- golden
//  Public/HTEditList.h:314-333 and :335-346, reproduced VERBATIM as a TU-local
//  definition instead of `#include "Public/HTEditList.h"`.
//  WHY NOT THE INCLUDE (this is trap 5, hit and measured, not theorised):
//    `#include "Public/HTEditList.h"` in THIS TU is a hard compile error --
//      Public/HTEditList.h:128 `redefinition of class TList`  (previous:
//        aHotPlateSubstrate.h:80)
//      Public/HTEditList.h:321 `redefinition of class uPlateInfo` (previous:
//        aHotPlateSubstrate.h:673)
//    and those two are NOT the same class in the two headers -- the substrate
//    carries deliberately NARROWER mirrors.  Public/HTEditList.h documents the
//    collision itself, at its own :50-79 "INTEGRATION HAZARDS (A)/(B)", and
//    states that reconciling it (deleting the substrate copies and repointing
//    ~30 consumers) is explicitly out of scope.  This TU already includes
//    aHotPlateSubstrate.h at :64 and cannot drop it (OutArmSuck / TMyKitSuck /
//    TMySucker / fBarCode all come from there), so the two headers are mutually
//    exclusive here.
//  WHY A LOCAL COPY IS SAFE: the text below is byte-identical to golden, and
//  Public/HTEditList.h:437-469 is itself a byte-identical mirror of the same
//  golden lines -- so both definitions have the SAME LAYOUT.  That is the exact
//  property trap 5 says to verify rather than assume; it was verified by diffing
//  golden Public/HTEditList.h:314-346 against port Public/HTEditList.h:437-469.
//  If anyone ever changes one, they MUST change the other: the mangled name of
//  MoveOutArmXYAndSuck(SingleSuckPosOnTray, uPoint2D) does not encode where the
//  class was defined, so divergent layouts would link cleanly and read every
//  field at the wrong offset.  Guarded so a future header include wins instead
//  of colliding.  ArmAndSuckInfo (golden :292-312) is NOT copied -- this batch
//  does not translate the overload that uses it.
//-----------------------------------------------------------------------------
#ifndef HT9045_FILLTRAY_TYPES_LOCAL
#define HT9045_FILLTRAY_TYPES_LOCAL
typedef struct                                                                  //Jimmychiu 20240726 : Fill The Tray After Out Arm Place
{
    int iWhichAuto;
    uPoint2D pTrayRowCol;
    uPoint2D pArmXYPos;
    int iArmXV1;
    int iArmXV2;
    int iArmYV1;
    bool bPlace;
    void Clear()
    {
        iWhichAuto=-1;
        pTrayRowCol.Clear();
        pArmXYPos.Clear();
        iArmXV1=0;
        iArmXV2=0;
        iArmYV1=0;
        bPlace=false;
    }
}SingleSuckPosOnTray;
//---------------------------------------------------------------------------
typedef struct                                                                  //Jimmychiu 20240726 : Fill The Tray After Out Arm Place
{
    SingleSuckPosOnTray ssTarget;
    SingleSuckPosOnTray ssSource;
    uPoint2D pArmSuckActive;
    void Clear()
    {
        ssTarget.Clear();
        ssSource.Clear();
        pArmSuckActive.Clear();
    }
}MyFillTheTrayAfterOutArmPlace;
#endif // HT9045_FILLTRAY_TYPES_LOCAL

//-----------------------------------------------------------------------------
//  Declarations this block's OWN bodies need.  Golden gets all of them from
//  aoutarm9045.h; this tree's aoutarm9045.h deliberately does NOT carry them
//  and this wave does not extend it -- see the report's "for the main loop"
//  section for WHY (every one of the 27 aoutarm9045_<layout>.cpp variants
//  already forward-declares GetOutArmToShuttleOffset_9045 locally, several of
//  them WITH `bool bPitch=true`; publishing the same default from the header
//  would be a hard "default argument given ... after previous specification"
//  error in each of those TUs).  Signatures verbatim from golden aoutarm9045.h.
//-----------------------------------------------------------------------------
void GetWhichAutoStart(int iWhichAuto, uPoint2D &pStartPos);                     // golden aoutarm9045.h (def below, golden :4193)
bool IsOutArmPosOutLimit(uPoint2D pArmPos);                                      // golden aoutarm9045.h (def below, golden :4234)
bool IsOutArmArrival(uPoint2D pTargetPos);                                       // golden aoutarm9045.h (def below, golden :4325)
bool InitNewFixTrayForUnloaderClipRead(int iwhichauto, bool bOutputReport=false);// golden aoutarm9045.h (def below, golden :4536) -- default arg is golden's
extern int iPlaceTrayToAutoTask;                                                 // golden acatchtray.h; port DECL acatchtray.h:50 -- declared locally rather than pulling the whole tray-arm header into the out-arm engine
int  OutArmPickTrayAlarm(int iwhichTray, AnsiString ErrPart);                    //JerryYang 20151013 整盤OutArm吸取異常 -- REAL def SortingBinTray/SortingBinTray.cpp:2518 (declared SortingBinTray/SortingBinTray.h:77; declared LOCALLY here instead of including that header, because this TU already owns file-local `static` DoSortingBinTray/SortingBinTray_Set* stand-ins at :230-233 and pulling the header would turn them into "static declaration follows non-static declaration" hard errors)

// (N1) golden Automation/AGV.h:218 -- see GATE REGISTER.
#ifndef AOUT9045_EATKTFMOVEFIXIC_FWD
#define AOUT9045_EATKTFMOVEFIXIC_FWD
static const int eAtkTfMoveFixIC = 6;                                           // golden Automation/AGV.h:218 (enum eATkTrayFeed)
#endif

// (G1) golden Magazine.h:149 -- see GATE REGISTER.
// AI(pt-wave) 20260811 PT-W8 integrate: stand-in corrected true -> false.  The
// original comment claimed "golden's own arithmetic yields true on an empty
// buffer band"; that derivation runs backwards.  Golden Magazine.cpp:376-413:
//   iCnt counts Tray.Data[i][j]==NULL_IC cells inside the band
//        j >= k*iYRegNum && j < (k+1)*iYRegNum      (golden :385-387)
//   iNeedPlace counts nozzles already routed to this magazine (golden :395-401)
//   if(iCnt<=iNeedPlace) return false;   else return true;  (golden :407-412)
// iYRegNum is 0 in BOTH trees (golden cmydef.cpp:5523, port cmydef.cpp:5541), so
// the band test is `j>=0 && j<0` -- false for every j, whatever the tray holds --
// hence iCnt==0.  Offline OutArmSuck.Item is zero-initialised and NULL_IC==0
// (cmydef.cpp:153), so the !=NULL_IC guard never passes and iNeedPlace==0.
// 0<=0 => GOLDEN RETURNS false.  `true` was the permissive value ("buffer has
// room, route the IC there") for a capacity predicate, licensed by a derivation
// that reached the opposite conclusion from the code it cited.
// Unreachable offline either way (needs AUTO3_IS_MAGAZINE==1 &&
// TestIF_File.iMagFixTrayType==1 && OutArmSuck.iWhichAuto[i][j]>=iMagMin), so
// this changes no current behaviour -- it stops the stand-in from lying.
static bool CheckPlaceToBufferTray(int /*iWhichMagazine*/) { return false; }     // golden Magazine.h:149 / Magazine.cpp:376 -- see derivation above

//------------------------------------------------------------------------------
//  Fix3MoveToLeft -- golden aoutarm9045.cpp:754-791
//------------------------------------------------------------------------------
bool Fix3MoveToLeft(bool bLeft)                                                 //JimmyChiu 20220927 : Stepper Motor Control in Fix3
{
    int iXPos=0;
    bool bBack=false;
    if(FIX3_FULL_PLACE==Fix3K_UseStepperMotor)
    {
        iXPos=(bLeft)?Tech.iFix3PosL:Tech.iFix3PosR;
        if(MOT[MFix3Full].CompareCommandPos(iXPos, 10)!=1)
            bBack=MOT[MFix3Full].MotorMove(iXPos);
        else
            bBack=true;

        if(bBack)
        {
            if(bLeft)
            {
                if(Sen[SnFix3FullPlace].Enable &&                               //Steven 20250814 : Fix3滿盤功能-馬達版+Sensor
                   Sen[SnFix3FullPlace].IsOn()==false)
                {
                    bBack=false;
                    ShowMyMessage("The fix3 motor does not move to left!!", "Please check the motor or the sensor (SnFix3FullPlace) should be on");
                }
            }
            else
            {
                MOT[MFix3Full].ScanMotorStatus();
                int iPos=MOT[MFix3Full].ReadPos();
                if(iPos>=0 &&
                   MOT[MFix3Full].Led[iHomeLed]==false)
                {
                    bBack=false;
                    ShowMyMessage("The fix3 motor does not move to right!!", "Please check the motor or the motor home sensor should be on");
                }
            }
        }
    }
    return bBack;
}
//------------------------------------------------------------------------------
//  Fix3CylinderDelay -- golden aoutarm9045.cpp:793 (file-scope global).
//  Golden's type IS TQPF_Timer (myTimer.h), not HTimer -- no substitution made.
//  Its only golden reader is UseFix3Cylinder (golden :794), which is still an
//  offline stand-in at aoutarm9045.cpp:2204 in this tree, so the object has no
//  reader yet.  Static-init safety (trap 4): TQPF_Timer::TQPF_Timer()
//  (myTimer.cpp:14-17) calls ONLY CalibratePerformanceCounterOverhead(), which
//  is QueryPerformanceCounter x1001 -- it touches none of the 18 NULL globals in
//  docs/PT_CAMPAIGN_PLAN.md §8.  Checked, not assumed.
//------------------------------------------------------------------------------
TQPF_Timer Fix3CylinderDelay;

//------------------------------------------------------------------------------
//  GetVariableYOutShuttleData -- golden aoutarm9045.cpp:1188-1209
//  REPLACES the one-line offline stub aoutarm_shims.cpp:50 (`return 0;`).
//------------------------------------------------------------------------------
int GetVariableYOutShuttleData()                                                //ChungHung 20131231 alter AutoYPitch
{
    if(USE_OUT_Y_IS_AUTO_PITCH==true)                                           //JerryYang 20251218 : IN/OUT ARM支援不同模組
    {
        if(OutArmSuck.iPickRow==1 ||
           iInArmType==e9045_1x4_8_Hot ||
           iInArmType==e9045_1x2_4_Hot ||
           iInArmType==e9045_2x2_8_Hot)                                         //JerryYang 20231003
            return 6000;

        if(TestIF.dSiteYPitch>=IN_OUT_ARM_Y_PITCH_MAX)                          //kevin 20190901 : 修正Y-Pitch
             return IN_OUT_ARM_Y_PITCH_MAX;
        else if(TestIF.dSiteYPitch<=IN_OUT_ARM_Y_PITCH_MIN)                     //kevin 20190901
            return IN_OUT_ARM_Y_PITCH_MIN;

        return TestIF.dSiteYPitch;
    }
    else
    {
        return TestIF_File.iARM_Y_PITCH;
    }
}
//------------------------------------------------------------------------------
//  GetOutArmToShuttleOffset_9045 -- golden aoutarm9045.cpp:1211-1288
//  REPLACES the one-line offline stub aoutarm_shims.cpp:47 (`return 0;`).
//  NOTE the behaviour delta the stub was hiding: golden's NOT-FOUND value is
//  -1 (it is what `iOffsetPos` is initialised to and what comes back when
//  TestIF.bEnableAutoAlignment is true), and every caller tests `iOffsetPos>=0`.
//  The stub answered 0, i.e. "OutOfsOutSh1", not "no offset".
//  The default argument (`bool bPitch=true`) is NOT repeated here -- golden
//  states it in aoutarm9045.h and every variant TU restates it locally.
//------------------------------------------------------------------------------
int GetOutArmToShuttleOffset_9045(int iSht, int iModeRow, int iModeCol, bool bPitch)
{
    int iOffsetPos=-1;
    if(TestIF.bEnableAutoAlignment==false)
    {
        if(IniConfig.bE34InOutArmPitchZOffsetSameOne==true && bPitch)
        {
            iOffsetPos=OutOfsOutSh1;
        }
        else
        {
            if(IniConfig.bE47_ShuttleUse4Offset)
            {
                if(iModeRow==1)
                {
                    if(iModeCol%100<10)
                    {
                        if(iSht==0)
                            iOffsetPos=OutOfsOutSh1LB;
                        else
                            iOffsetPos=OutOfsOutSh2LB;
                    }
                    else
                    {
                        if(iSht==0)
                            iOffsetPos=OutOfsOutSh1RB;
                        else
                            iOffsetPos=OutOfsOutSh2RB;
                    }
                }
                else if(iModeRow==0)
                {
                    if(iModeCol%100<10)
                    {
                        if(iSht==0)
                            iOffsetPos=OutOfsOutSh1;
                        else
                            iOffsetPos=OutOfsOutSh2;
                    }
                    else
                    {
                        if(iSht==0)
                            iOffsetPos=OutOfsOutSh1RA;
                        else
                            iOffsetPos=OutOfsOutSh2RA;
                    }
                }
                else
                {
                    if(iModeCol%100<10)                                         //Steven 20240826 : 補上Offset
                    {
                        if(iSht==0)
                            iOffsetPos=OutOfsOutSh1;
                        else
                            iOffsetPos=OutOfsOutSh2;
                    }
                    else
                    {
                        if(iSht==0)
                            iOffsetPos=OutOfsOutSh1RA;
                        else
                            iOffsetPos=OutOfsOutSh2RA;
                    }
                }
            }
            else
            {
                if(iSht==0)
                    iOffsetPos=OutOfsOutSh1;
                else if(iSht==1)                                                //RogerYang 20250510 Add for 9046AU
                    iOffsetPos=OutOfsOutSh2;
                else
                    iOffsetPos=OutOfsSortSht;
            }
        }
    }
    return iOffsetPos;
}
//------------------------------------------------------------------------------
//  GetOutArmXToShuttleOffset_9045 -- golden aoutarm9045.cpp:1290-1301
//  REPLACES the one-line offline stub aoutarm_shims.cpp:49 (`return 0;`).
//  GOLDEN QUIRK PRESERVED, NOT FIXED: both arms of `if(iSht==0)` are identical
//  (`OutArmOffSet[iOffsetPos]->GetX()`), so the branch is a no-op.  Golden has
//  it that way (:1295-1298); reproduced verbatim.
//------------------------------------------------------------------------------
int GetOutArmXToShuttleOffset_9045(int iSht, int iOffsetPos)                    //Steven 20240309 : fixed for out arm to sht offset
{
    int iPos=0;
    if(iOffsetPos>=0)
    {
        if(iSht==0)
            iPos=OutArmOffSet[iOffsetPos]->GetX();
        else
            iPos=OutArmOffSet[iOffsetPos]->GetX();
    }
    return iPos;
}
//------------------------------------------------------------------------------
//  GetOutArmYToShuttleOffset_9045 -- golden aoutarm9045.cpp:1303-1314
//  REPLACES the one-line offline stub aoutarm_shims.cpp:48 (`return 0;`).
//  Same golden quirk as the X twin: both arms of `if(iSht==0)` are identical.
//------------------------------------------------------------------------------
int GetOutArmYToShuttleOffset_9045(int iSht, int iOffsetPos)
{
    int iPos=0;
    if(iOffsetPos>=0)
    {
        if(iSht==0)
            iPos=OutArmOffSet[iOffsetPos]->GetY();
        else
            iPos=OutArmOffSet[iOffsetPos]->GetY();
    }
    return iPos;
}

//------------------------------------------------------------------------------
//  SearchTrayToPlace_Magazine -- golden aoutarm9045.cpp:1420-1559
//  REPLACES the one-line offline stub aoutarm_shims.cpp:83 (`return 0;`).
//  That stub's own comment already recorded that golden's real not-found value
//  is `Prod.iIfErrorT6` (golden :1558) and never a literal 0 -- this body
//  restores exactly that.  Gated callee: (G1) CheckPlaceToBufferTray at :1532.
//------------------------------------------------------------------------------
int SearchTrayToPlace_Magazine()                                                //JerryYang 20221215 : add Magazine
{
    int iTempWhichMag=-1;
    iOutArmPlaceOrder=0;
    bool bHasChang=false;

    for(int i=0; i<OutArmSuck.iMaxRow; i++)                                     //Sam 20221121 : 先檢查吸嘴上有沒有可以直接放 Auto3 Magazine 的 Tray
    {
        for(int j=0; j<OutArmSuck.iMaxCol; j++)
        {
            for(int k=0; k<MAX_MGZ_TRAY; k++)
            {
                if(OutArmSuck.Item[i][j]!=NULL_IC &&
                    OutArmSuck.iWhichAuto[i][j]==iSortTrayIndex[k])
                {
                    if(i==0)
                        iOutArmPlaceOrder=0;
                    else
                        iOutArmPlaceOrder=1;

                    iTempWhichMag=OutArmSuck.iWhichAuto[i][j]-iMagMin;          //JerryYang 20221121 : 避免iWhichMag一直被改變, 有找到當下要放auto3的再改
                    if(MOT[MMMagazineTary1+iTempWhichMag].Tray.FullIC()==true)  //JerryYang 20250920 : fixed for Multi bin
                    {
                        for(int l=1; l<14; l++)
                        {
                            if(iTempWhichMag+l<14)
                            {
                                if(BinSelect[iTestRunMode].bMagazineLink[iTempWhichMag+l]==true)
                                {
                                    if(MOT[MMMagazineTary1+iTempWhichMag+l].Tray.FullIC()==false && iTempWhichMag+l==iAuto3MagazineIndex)
                                    {
                                        iWhichMag=iTempWhichMag+l;
                                        iOutArmWhichAuto=iMagAtAuto;
                                        return 2;
                                    }
                                }
                                else
                                {
                                    break;
                                }
                            }
                        }
                    }
                    else
                    {
                        if(iTempWhichMag==iAuto3MagazineIndex)
                        {
                            iWhichMag=iTempWhichMag;
                            iOutArmWhichAuto=iMagAtAuto;
                            return 2;
                        }
                    }
                }
            }
        }
    }

    for(int k=0; k<eTrayCount; k++)
    {
        if(Prod.iTrayType[iSortTrayIndex[k]]==tNotUse)
            continue;

        for(int i=0; i<OutArmSuck.iMaxRow; i++)
        {
            for(int j=0; j<OutArmSuck.iMaxCol; j++)
            {
                if(OutArmSuck.Item[i][j]!=NULL_IC &&
                   OutArmSuck.iWhichAuto[i][j]==iSortTrayIndex[k])
                {
                    if(i==0)
                        iOutArmPlaceOrder=0;
                    else
                        iOutArmPlaceOrder=1;
                    if(OutArmSuck.iWhichAuto[i][j]>=iMagMin)
                    {
                        iWhichBuff=(OutArmSuck.iWhichAuto[i][j]-iMagMin)%iFixCnt+iFixCnt;                               //QQQ
                        iTempWhichMag=OutArmSuck.iWhichAuto[i][j]-iMagMin;
                        bHasChang=false;
                        if(MOT[MMMagazineTary1+iTempWhichMag].Tray.FullIC()==true)                                      //JerryYang 20250920 : fixed for Multi bin
                        {
                            for(int l=1; l<14; l++)
                            {
                                if(iTempWhichMag+l<14)
                                {
                                    if(BinSelect[iTestRunMode].bMagazineLink[iTempWhichMag+l]==true)
                                    {
                                        if(MOT[MMMagazineTary1+iTempWhichMag+l].Tray.FullIC()==false)
                                        {
                                            iWhichMag=iTempWhichMag+l;
                                            bHasChang=true;
                                             break;
                                        }
                                    }
                                    else
                                    {
                                        iWhichMag=iTempWhichMag;
                                        break;
                                    }
                                }
                            }
                        }

                        if(bHasChang==false)
                            iWhichMag=iTempWhichMag;

                        if(TestIF_File.iMagFixTrayType==1)                      //buffer tray
                        {
                            if(iAuto3MagazineIndex==iWhichMag)
                            {
                                iOutArmWhichAuto=iMagAtAuto;
                                return 2;
                            }
                            else if(CheckPlaceToBufferTray(OutArmSuck.iWhichAuto[i][j]))                                //buffer區有位置, 先放buffer區
                            {
                                iOutArmWhichAuto=OutArmSuck.iWhichAuto[i][j];
                                return OutArmSuck.iWhichAuto[i][j];
                            }
                            else                                                //buffer區不夠放, 放AUTO3
                            {
                                iOutArmWhichAuto=iMagAtAuto;
                                return 2;
                            }
                        }
                        else
                        {
                            iOutArmWhichAuto=iMagAtAuto;
                            return 2;
                        }
                    }
                    else
                    {
                        iOutArmWhichAuto=iSortTrayIndex[k];
                        return iSortTrayIndex[k];
                    }
                }
            }
        }
    }
    return Prod.iIfErrorT6;
}
//------------------------------------------------------------------------------
//  SearchTrayToPick_Buffer -- golden aoutarm9045.cpp:1561-1598
//  REPLACES the file-local `static` offline stub aoutarm.cpp:541 (`return -1;`).
//  That one is `static`, so it does NOT collide at link time -- it SHADOWS,
//  which is trap 4 shape (d): aoutarm.cpp:814 will keep calling its own copy
//  until the main loop deletes that line.  Supplied constant: (N1).
//------------------------------------------------------------------------------
int SearchTrayToPick_Buffer()                                                   //JerryYang 20221215 : add Magazine
{
    iOutArmPickOrder=0;
    int iMag=iPickWhichMag-eMag1;
    int iRx=iMag%iFixCnt+eMag1;
    bool bCheck=false;

    if(fAGV->IsATK_AMR() &&
       LastSet.iUnloadFixTray==eAtkTfMoveFixIC)                                 //Steven 20260202 : for ATK AMR
    {
        for(int i=iFixMin; i<=iFixMax; i++)                                     //Fix Tray
        {
            if(bCheck==false &&
               MOT[iMMAuto[i]].HasRealIC())
            {
                iRx=i;
                iWhichAuto=i-iFixMin;
                bCheck=true;
            }
        }
    }

    for(int i=0; i<OutArmSuck.iPickRow; i++)
    {
        for(int j=0; j<OutArmSuck.iPickCol; j++)
        {
            if(OutArmSuck.Item[i][j]==NULL_IC)
            {
                if(i==0)
                    iOutArmPickOrder=0;
                else
                    iOutArmPickOrder=1;
                return iRx;
            }
        }
    }
    return -1;
}

//------------------------------------------------------------------------------
//  DoFixTrayFullAlarm -- golden aoutarm9045.cpp:1674-2016
//  REPLACES the one-line offline stub aoutarm_shims.cpp:81 (`return true;`).
//  BEHAVIOUR DELTA, NOT NEUTRAL: the stub said "alarm handled, carry on"
//  unconditionally.  The real body can now return false (Fix tray full / tray
//  missing / Fix3 stepper not at the right side) and will raise MES1710/1711/
//  1811/sMES1720[]/sMES1721[] through ShowErrorMessage.
//  The `#ifdef SOFT_SIMULTE` arms are kept VERBATIM but are inert (SOFT_SIMULTE
//  is not defined in this tree) -- that is why SaveProductionRecord and the
//  simulation-only InitNewFixTrayForUnloaderClipRead calls compile away.
//------------------------------------------------------------------------------

// AI(W906-BA-SIM0) 20260911: declaration for the SOFT_SIMULTE arm below, migrated
// from the D:\HT9050 tree (its AI(W906-SIM-0) 20260825).
//
// WHY IT IS NEEDED EVEN THOUGH THE CALL COMPILES AWAY TODAY. The banner above is
// right that the arm is inert while SOFT_SIMULTE is undefined -- but "inert" and
// "compilable" are different properties. That tree turned MachineType.h:48 on for
// one wave, the arm became live, and this was the last compile error in
// ht9045_sm. So the file as it stands cannot be built in the simulation
// configuration at all, and nothing says so until someone tries.
//
// This is a DECLARATION ONLY. The definition already exists and is already
// reachable in this tree (SortingBinTray/SortingBinTray.cpp:2699), and csystem.cpp
// already declares it exactly this way at :12042 and calls it from four live
// sites -- so the writing behaviour it performs (a CSV under
// D:\HT9045_Log\UnloadTrayLog\, gated on CosFunction.bSaveProductionLogByUnloaderTray
// and TrayData->HasRealIC()) is unchanged and already live. Nothing new becomes
// reachable here; only the dead arm becomes compilable.
extern void SaveProductionRecord(class TMyTray *TrayData, AnsiString Name);     // golden SortingBinTray.h:94 -- def SortingBinTray/SortingBinTray.cpp:2699

bool DoFixTrayFullAlarm()                                                       //Steven 20181113 : 修正Fix Link Alarm的問題
{
    bool bResult=true;
    bool bScan1By1=false;
    int iAuto, iFix=iAutoIndex[iWhichAuto];
    AnsiString str, s;

    if(FIX3_FULL_PLACE==Fix3K_UseStepperMotor &&
       iWhichAuto==eFix3 &&                                                     //Jimmychiu 20250306 : iFix --> iWhichAuto
       Fix3MoveToLeft(false)==false)                                            //Jimmychiu 20230628 : FIX3到達左側不檢查盤是否存在
    {
        bResult=false;
        return bResult;
    }

    if(AUTO3_IS_MAGAZINE==1             &&                                      //JerryYang 20221215 : Magazine把fix區當buffer區功能
       TestIF_File.iMagFixTrayType==1   &&
       iWhichAuto>=iMagMin)
    {
    }
    else
    {
        if(iWhichAuto==eFix1 &&                                                 //Steven 20181113 : 修正Fix Link顯示問題
           Prod.bLinkTo6Tray[eFix2]==true &&
           Prod.bLinkTo6Tray[eFix3]==true)
        {
            if(MOT[MManualTray1].FullIC() &&
               MOT[MManualTray2].FullIC() &&
               MOT[MManualTray3].FullIC())
            {
                #ifdef SOFT_SIMULTE
                    if(CosFunction.bReadClipCodeFromUnloader==true &&
                       IniConfig.bP60ReadClipCodeFromUnloader==true &&
                       LastSet.iRealDummy!=DUMMY)
                    {
                        InitNewFixTrayForUnloaderClipRead(eFix1);
                        InitNewFixTrayForUnloaderClipRead(eFix2);
                        InitNewFixTrayForUnloaderClipRead(eFix3);
                    }
                    else
                    {
                        InitNewFixTray(eFix1, "AOutArm_FullIC");                //Steven 20160414 : 整合Fix盤設定
                        InitNewFixTray(eFix2, "AOutArm_FullIC");
                        InitNewFixTray(eFix3, "AOutArm_FullIC");
                    }
                #else
                    if(LastSet.iRealDummy!=DUMMY)
                    {
                        if(IniConfig.bEnable_SECS_GEM==true)                    //Steven 20140528 : Secs Gem
                        {
                            EventReport(SECS_EVENT.Fix1Full);                   //38     Fix 1 Full
                            EventReport(SECS_EVENT.Fix2Full);
                            EventReport(SECS_EVENT.Fix3Full);
                        }

                        ShowErrorMessage("MES1710", K_RETRY, MManualTray1);     //Fix Tray 1, 2 and 3 is filled with devices.
                    }
                    else if(CosFunction.bReadClipCodeFromUnloader==true &&
                            IniConfig.bP60ReadClipCodeFromUnloader==true &&
                            LastSet.iRealDummy!=DUMMY)
                    {
                        InitNewFixTrayForUnloaderClipRead(eFix1);
                        InitNewFixTrayForUnloaderClipRead(eFix2);
                        InitNewFixTrayForUnloaderClipRead(eFix3);
                    }
                    else
                    {
                        InitNewFixTray(eFix1, "AOutArm_FullIC");                //Steven 20160414 : 整合Fix盤設定
                        InitNewFixTray(eFix2, "AOutArm_FullIC");
                        InitNewFixTray(eFix3, "AOutArm_FullIC");
                    }
                #endif
                bResult=false;
            }
            else
            {
                bScan1By1=true;
            }
        }
        else if(iWhichAuto==eFix2 &&
                Prod.bLinkTo6Tray[eFix2]==true &&
                Prod.bLinkTo6Tray[eFix3]==false)
        {
            if(MOT[MManualTray1].FullIC() &&
               MOT[MManualTray2].FullIC())
            {
                #ifdef SOFT_SIMULTE
                    if(CosFunction.bReadClipCodeFromUnloader==true &&
                       IniConfig.bP60ReadClipCodeFromUnloader==true &&
                       LastSet.iRealDummy!=DUMMY)
                    {
                        InitNewFixTrayForUnloaderClipRead(eFix1);
                        InitNewFixTrayForUnloaderClipRead(eFix2);
                    }
                    else
                    {
                        InitNewFixTray(eFix1, "AOutArm_FullIC");                //Steven 20160414 : 整合Fix盤設定
                        InitNewFixTray(eFix2, "AOutArm_FullIC");
                    }
                #else
                    if(LastSet.iRealDummy!=DUMMY)
                    {
                        if(IniConfig.bEnable_SECS_GEM==true)                    //Steven 20140528 : Secs Gem
                        {
                            EventReport(SECS_EVENT.Fix1Full);                   //38     Fix 1 Full
                            EventReport(SECS_EVENT.Fix2Full);
                        }

                        ShowErrorMessage("MES1711", K_RETRY, MManualTray1);     //Fix Tray 1 and 2 is filled with devices.
                    }
                    else if(CosFunction.bReadClipCodeFromUnloader==true &&
                            IniConfig.bP60ReadClipCodeFromUnloader==true &&
                            LastSet.iRealDummy!=DUMMY)
                    {
                        InitNewFixTrayForUnloaderClipRead(eFix1);
                        InitNewFixTrayForUnloaderClipRead(eFix2);
                    }
                    else
                    {
                        InitNewFixTray(eFix1, "AOutArm_FullIC");                //Steven 20160414 : 整合Fix盤設定
                        InitNewFixTray(eFix2, "AOutArm_FullIC");
                    }
                #endif
                bResult=false;
            }
            else
            {
                bScan1By1=true;
            }
        }
        else if(iWhichAuto==eFix3 &&
                Prod.bLinkTo6Tray[eAuto2]==false &&
                Prod.bLinkTo6Tray[eAuto3]==true)
        {
            if(MOT[MManualTray2].FullIC() &&
               MOT[MManualTray3].FullIC())
            {
                #ifdef SOFT_SIMULTE
                    if(CosFunction.bReadClipCodeFromUnloader==true &&
                       IniConfig.bP60ReadClipCodeFromUnloader==true &&
                       LastSet.iRealDummy!=DUMMY)
                    {
                        InitNewFixTrayForUnloaderClipRead(eFix2);
                        InitNewFixTrayForUnloaderClipRead(eFix3);
                    }
                    else
                    {
                        InitNewFixTray(eFix2, "AOutArm_FullIC");                //Steven 20160414 : 整合Fix盤設定
                        InitNewFixTray(eFix3, "AOutArm_FullIC");
                    }
                #else
                    if(LastSet.iRealDummy!=DUMMY)
                    {
                        if(IniConfig.bEnable_SECS_GEM==true)                    //Steven 20140528 : Secs Gem
                        {
                            EventReport(SECS_EVENT.Fix2Full);                   //38     Fix 1 Full
                            EventReport(SECS_EVENT.Fix3Full);
                        }
                        ShowErrorMessage("MES1811", K_RETRY, MManualTray1);     //Fix Tray 2 and 3 is filled with devices.
                    }
                    else if(CosFunction.bReadClipCodeFromUnloader==true &&
                            IniConfig.bP60ReadClipCodeFromUnloader==true &&
                            LastSet.iRealDummy!=DUMMY)
                    {
                        InitNewFixTrayForUnloaderClipRead(eFix2);
                        InitNewFixTrayForUnloaderClipRead(eFix3);
                    }
                    else
                    {
                        InitNewFixTray(eFix2, "AOutArm_FullIC");                //Steven 20160414 : 整合Fix盤設定
                        InitNewFixTray(eFix3, "AOutArm_FullIC");
                    }
                #endif
                bResult=false;
            }
            else
            {
                bScan1By1=true;
            }
        }
        //<==
        //Steven 20181113 : 修正Fix Link顯示問題
        else
        {
            bScan1By1=true;
        }

        if(bScan1By1)
        {
            for(int i=iFixMin; i<=iFixMax; i++)
            {
                iFix=iAutoIndex[i];
                if(MOT[iMMAuto[i]].FullIC())
                {
                    #ifdef SOFT_SIMULTE
                        if(CosFunction.bReadClipCodeFromUnloader==true &&
                           IniConfig.bP60ReadClipCodeFromUnloader==true &&
                           LastSet.iRealDummy!=DUMMY)
                        {
                            InitNewFixTrayForUnloaderClipRead(i);
                        }
                        else
                        {
                            if(MOT[iMMAuto[i]].Tray.HasRealIC())
                            {
                                SaveProductionRecord(&MOT[iMMAuto[i]].Tray, s6TrayName[i]);                             //Steven 20200330 : production log by unloader tray存檔
                            }
                            InitNewFixTray(i, "AOutArm_FullIC");                //Steven 20160414 : 整合Fix盤設定
                        }
                    #else
                        if(LastSet.iRealDummy!=DUMMY)
                        {
                            if(IniConfig.bEnable_SECS_GEM==true)                //Steven 20140528 : Secs Gem
                            {
                                if(i==eFix1)
                                    EventReport(SECS_EVENT.Fix1Full);           //38     Fix 1 Full
                                else if(i==eFix2)
                                    EventReport(SECS_EVENT.Fix2Full);
                                else if(i==eFix3)
                                    EventReport(SECS_EVENT.Fix3Full);
                                else if(i==eFix4)
                                    EventReport(SECS_EVENT.Fix4Full);           //Steven 20230907 : For HT-9011UC
                                else if(i==eFix5)
                                    EventReport(SECS_EVENT.Fix5Full);
                                else if(i==eFix6)
                                    EventReport(SECS_EVENT.Fix6Full);
                            }

                            if(IniConfig.bA68_AutoLoadUnload)                   //JerryYang 20250521 : For AMR
                            {
                                iPortStatus[ePortFix1+i-iFixMin]=eFixFullTray;
                                iLastPortStatus[ePortFix1+i-iFixMin]=eFixFullTray;
                                EventReport(SECS_EVENT.Fix1PortStatusChanged+i-iFixMin);
                            }

                            ShowErrorMessage(sMES1720[iFix], K_RETRY, iMMAuto[i]);
                        }
                        else if(CosFunction.bReadClipCodeFromUnloader==true &&
                                IniConfig.bP60ReadClipCodeFromUnloader==true &&
                                LastSet.iRealDummy!=DUMMY)
                        {
                            InitNewFixTrayForUnloaderClipRead(i);
                        }
                        else
                        {
                            InitNewFixTray(i, "AOutArm_FullIC");                //Steven 20160414 : 整合Fix盤設定
                        }
                    #endif
                    bResult=false;
                }
            }
        }
    }

    if(AUTO3_IS_MAGAZINE==1             &&                                      //JerryYang 20221215 : Magazine把fix區當buffer區功能
       TestIF_File.iMagFixTrayType==1   &&
       iWhichAuto>=iMagMin &&
       iWhichBuff>=iFixMin)
    {
        iAuto=iWhichBuff;
        iFix=iAutoIndex[iWhichBuff];
    }
    else
    {
        iAuto=iWhichAuto;
        iFix=iAutoIndex[iWhichAuto];
    }

    //AI(W906-TIF912) 20260925: golden 912 :2058-2082 (Prevent Drop: pick an empty
    //  manual Fix tray).  UNREACHABLE IN GOLDEN, so recorded, not compiled:
    //  912 changed the signature to DoFixTrayFullAlarm(bool bCheckFixStatus)
    //  with default false (golden aoutarm9045.h:49) and NO caller passes an
    //  argument (grep `DoFixTrayFullAlarm\s*\(\s*[^\s)]` over the whole 912
    //  tree, 20260925: only the definition :1776 and the header :49 match), so
    //  `bCheckFixStatus==true` is never true.  The port keeps the 906 signature
    //  DoFixTrayFullAlarm() because 27 variant files forward-declare exactly
    //  that (aoutarm9045_*.cpp, 20260925); adding a defaulted parameter here would split it into a second
    //  overload and break their link.  Behaviour is identical either way.
#if 0   // golden 912 :2058-2082 -- dead in golden (bCheckFixStatus always false); needs the 912 locals iData/iFixData (:1782)
    if(TestIF_File.bPreventDropfunction && bCheckFixStatus==true)          //Frank 20260710 ADD     //Eastsun 20260710整合
    {
        iFixData=-1;
        iData=BinSelect[iTestRunMode].IfErrorT3 ;
        if(MOT[MManualTray1+iData-3].fHasTray==false)
        {
            iFixData=(iData-3);
        }

        for(int j=0; j<iTestBinCount; j++)
        {
            iFix=BinSelect[iTestRunMode].iCatDataT3Pos[j]-1;
            if(MOT[MManualTray1+iFix-3].fHasTray==false)
            {
                if(iFix==3 || iFix==4 || iFix==5)
                    iFixData=(iFix-3);
            }
        }

        if(iFixData>0)
        {
            iAuto=iFixData;
            iWhichAuto=iFixData;
        }
    }
#endif

    if(Sen[SnFixedTrayDetect[iFix]].IsOff() && LastSet.iRealDummy!=DUMMY)
    {
        if(FIX3_FULL_PLACE==Fix3K_UseCylinder46LA &&                            //ChungHung 20140730 fix LA fix3 function 中途按Stop 會秀Alarm
           iWhichAuto==eFix2 &&                                                 //Jimmychiu 20250306 : iFix --> iWhichAuto
           Cylinder[C_FixTray_FullPlace].OffSensor()==false)
        {
            ;
        }
        else
        {
            ShowErrorMessage(sMES1721[iFix], K_RETRY, iMMAuto[iAuto]);
            bResult=false;
        }
    }

    for(int i=iFixMin; i<=iFixMax; i++)
    {
        iFix=iAutoIndex[i];
        if(LastSet.iRealDummy!=DUMMY)
        {
            if(MOT[iMMAuto[i]].fHasTray==false &&
               Sen[SnFixedTrayDetect[iFix]].IsOff()==false)
            {
                if(CosFunction.bReadClipCodeFromUnloader==true &&
                   IniConfig.bP60ReadClipCodeFromUnloader==true &&
                   LastSet.iRealDummy!=DUMMY)
                {
                    if(InitNewFixTrayForUnloaderClipRead(i)==true)
                    {
                        if(IniConfig.bA68_AutoLoadUnload)                       //JerryYang 20250521 : For AMR
                        {
                            iPortStatus[ePortFix1+i-iFixMin]=eFixFTrayArrived;
                            iLastPortStatus[ePortFix1+i-iFixMin]=eFixFTrayArrived;
                            EventReport(SECS_EVENT.Fix1PortStatusChanged+i-iFixMin);
                        }
                    }
                    else
                    {
                        bResult=false;
                    }
                }
                else
                {
                    InitNewFixTray(i, "AOutArm");                               //Steven 20160414 : 整合Fix盤設定
                    if(IniConfig.bA68_AutoLoadUnload)                           //JerryYang 20250521 : For AMR
                    {
                        iPortStatus[ePortFix1+i-iFixMin]=eFixFTrayArrived;
                        iLastPortStatus[ePortFix1+i-iFixMin]=eFixFTrayArrived;
                        EventReport(SECS_EVENT.Fix1PortStatusChanged+i-iFixMin);
                    }
                }
            }
        }
        else
        {
            if(MOT[iMMAuto[i]].fHasTray==false)
            {
                if(CosFunction.bReadClipCodeFromUnloader==true &&
                   IniConfig.bP60ReadClipCodeFromUnloader==true &&
                   LastSet.iRealDummy!=DUMMY)
                {
                    if(InitNewFixTrayForUnloaderClipRead(i)==false)
                    {
                        bResult=false;
                    }
                }
                else
                {
                    InitNewFixTray(i, "AOutArm");                               //Steven 20160414 : 整合Fix盤設定
                }
            }
        }
    }
    return bResult;
}

//------------------------------------------------------------------------------
//  CheckOutArmToTask50 -- golden aoutarm9045.cpp:3629-3673
//  REPLACES the one-line offline stub aoutarm_shims.cpp:66 (`return true;`).
//  BEHAVIOUR DELTA, NOT NEUTRAL: the stub said "always jump to task 50".  The
//  real body is a 4-way carry-kit / test-suck / shuttle-at-right interlock and
//  returns false for most states.
//------------------------------------------------------------------------------
bool CheckOutArmToTask50(int iSht)                                              //Steven 20241019 : 整合out arm判斷式
{
    if(iSht==0)
    {
        if((iOneCycle==1 || iCleanOut==1) &&
            FTestSuck.UseSiteNoIC() &&
            FRCarryKit.UseSiteNoIC() &&
            (BRCarryKit.UseSiteHasIC() ||
             BTestSuck.UseSiteHasIC()))
        {
            return true;
        }

        if(FRCarryKit.UseSiteNoIC() &&
           OutSHT1InRT() &&
           BRCarryKit.UseSiteHasIC() &&
           OutSHT2InRT())
        {
            return true;
        }
    }
    else
    {
        if((iOneCycle==1 || iCleanOut==1) &&
            BTestSuck.UseSiteNoIC() &&
            BRCarryKit.UseSiteNoIC() &&
            (FRCarryKit.UseSiteHasIC() ||
             FTestSuck.UseSiteHasIC()))
        {
            return true;
        }

        if(BRCarryKit.UseSiteNoIC() &&
           OutSHT2InRT())
        {
            if(FRCarryKit.UseSiteHasIC() &&
               OutSHT1InRT())
            {
                return true;
            }
        }
    }

    return false;
}
//------------------------------------------------------------------------------
//  VerifyTrayStatus -- golden aoutarm9045.cpp:3675-3764
//  REPLACES the one-line offline stub aoutarm_shims.cpp:82 (`return 3300;`).
//  3300 happens to be golden's fall-through value, so the stub was numerically
//  right for the common case -- but it could never produce 3100 / 3020 / 3050,
//  i.e. the JAM1102/JAM1103/WAR1130 retry-skip ladder was unreachable.
//  The `#ifndef SOFT_SIMULTE` arm is the LIVE one here (flag2/flag3 read real
//  sensors); the `#else` arm is kept verbatim and inert.
//------------------------------------------------------------------------------
int VerifyTrayStatus()
{
    static int iRetry=0;
    bool flag1=false, flag2=false, flag3=false, flag4=false;
    AnsiString str, Str2;
    int Task=3300, ret;
    if(TRAY_ARM_MODE==eAboveCoveyor &&
       MOT[iMMAuto[iWhichAuto]].fHasTray==true)
    {
        flag1=Sen[SnAutoTrayDetect[iWhichAuto]].IsOff();
        #ifndef SOFT_SIMULTE
        flag2=Sen[SnAutoFixCyPush[iWhichAuto]].IsOn();
        flag3=Cylinder[C_AutoSide_Fixer[iWhichAuto]].OffSensor();               //wei 20241011 新增後勾氣缸off sensor,避免氣缸沒作動
        if(AUTO3_IS_MAGAZINE==1 &&  iWhichAuto==iMagAtAuto)
        {
            flag3=false;
        }
        #else
        flag2=false;
        flag3=false;
        #endif
        flag4=Sen[SnAutoEdgePush[iWhichAuto]].IsOn();

        if((flag1 || flag2 || flag3 || flag4) &&
           LastSet.iRealDummy!=DUMMY)
        {
            if(iPlaceTrayToAutoTask!=1)                                         //Steven 20241125 : fixed for place tray to auto alarm.
            {
                Task=3100;
                return Task;
            }

            Cylinder[C_AutoSide_Fixer[iWhichAuto]].Off();
            Cylinder[C_AutoEdgePush  [iWhichAuto]].Off();
            Cylinder[C_AutoUpPress   [iWhichAuto]].Off();                       //JerryYang 20190423 新增unloader壓tray

            if(flag2)
            {
                Str2.sprintf("%s must off.", Sen[SnAutoFixCyPush[iWhichAuto]].Name);
                str=sJAM1103[iWhichAuto];
            }
            else if(flag3)                                                      //Steven 20241108 : 增加顯示sensor狀態
            {
                Str2.sprintf("%s must off.", Cylinder[C_AutoSide_Fixer[iWhichAuto]].OffSensorName);
                str=sJAM1103[iWhichAuto];
            }
            else if(flag4)
            {
                Str2.sprintf("%s must off.", Sen[SnAutoEdgePush[iWhichAuto]].Name);
                str=sJAM1102[iWhichAuto];
            }
            else
            {
                Str2.sprintf("%s must on.", Sen[SnAutoTrayDetect[iWhichAuto]].Name);
                str=sWAR1130[iWhichAuto];
            }

            if(iRetry<=5)                                                       //Steven 20241122 : retry 5次
            {
                iRetry++;
//                RecordProcess(Str2, "Auto Retry");
                ret=K_RETRY;
            }
            else
            {
                ret=ShowErrorMessage(str, K_RETRY|K_SKIP, iMMAuto[iWhichAuto], bOutArmTrayDuplicateErr[iWhichAuto], Str2);
                iRetry=0;
            }
            bOutArmTrayDuplicateErr[iWhichAuto]=true;

            if(ret==K_SKIP)
            {
                fProductionInfo->CalTrayICCount(iWhichAuto);
                MOT[iMMAuto[iWhichAuto]].ClearTray(__FUNC__);
                bOutArmTrayDuplicateErr[iWhichAuto]=false;
                Task=3020;
            }
            else
            {
                Task=3050;                                                      //Ifor 20171031 add Auto Tray ReCheck
            }
            return Task;
        }
        else
        {
            bOutArmTrayDuplicateErr[iWhichAuto]=false;
        }
    }
    return Task;
}

//------------------------------------------------------------------------------
//  GetOutOffsetFromWhichAuto -- golden aoutarm9045.cpp:3995-4079
//  GOLDEN BUG PRESERVED, NOT FIXED (:3999): the diagnostic uses "%s" for the
//  INT iWhichAuto (`sprintf("Error Func:%s iWhichAuto:%s",__FUNC__,iWhichAuto)`).
//  On a real BCB6 build that prints garbage / faults.  Reproduced verbatim --
//  it only fires on the already-invalid `iWhichAuto>=eTrayCount` path.
//  NOTE: this file already CALLS this function at :2054/:2056, inside the
//  `#if 0` twin of InspectOutArmPosition -- those call sites were waiting for
//  exactly this body.
//------------------------------------------------------------------------------
int GetOutOffsetFromWhichAuto(int iWhichAuto)                                   //Jimmychiu 20240731 : for out arm offset value
{
    if(iWhichAuto>=eTrayCount)
    {
        ShowMyMessage(AnsiString().sprintf("Error Func:%s iWhichAuto:%s",__FUNC__,iWhichAuto));
        return 0;
    }

    if(AUTO_EMPTY_COLOR>=3)
    {
        int iWhichAutoToOutOffset[eTrayCount]={OutOfsAuto1,                     //eAuto1
                                               OutOfsAuto2,                     //eAuto2
                                               OutOfsAuto3,                     //eAuto3
                                               OutOfsAuto4,                     //eAuto4
                                               OutOfsAuto5,                     //eAuto5
                                               OutOfsAuto6,                     //eAuto6
                                               OutOfsFix1,                      //eFix1
                                               OutOfsFix2,                      //eFix2
                                               OutOfsFix3,                      //eFix3
                                               OutOfsFix4,                      //eFix4
                                               OutOfsFix5,                      //eFix5
                                               OutOfsFix6,                      //eFix6
                                               OutOfsFix1,                      //eFix7
                                               OutOfsFix2,                      //eFix8
                                               OutOfsFix3,                      //eFix9
                                               OutOfsFix4,                      //eFix10
                                               OutOfsFix5,                      //eFix11
                                               OutOfsFix6,                      //eFix12
                                               OutOfsFix6,                      //eBulkBox
                                               OutOfsAuto6,                     //eMag1
                                               OutOfsAuto6,                     //eMag2
                                               OutOfsAuto6,                     //eMag3
                                               OutOfsAuto6,                     //eMag4
                                               OutOfsAuto6,                     //eMag5
                                               OutOfsAuto6,                     //eMag6
                                               OutOfsAuto6,                     //eMag7
                                               OutOfsAuto6,                     //eMag8
                                               OutOfsAuto6,                     //eMag9
                                               OutOfsAuto6,                     //eMag10
                                               OutOfsAuto6,                     //eMag11
                                               OutOfsAuto6,                     //eMag12
                                               OutOfsAuto6,                     //eMag13
                                               OutOfsAuto6,                     //eMag14
                                              };
        return iWhichAutoToOutOffset[iWhichAuto];
    }
    else
    {
        int iWhichAutoToOutOffset[eTrayCount]={OutOfsAuto1,                     //eAuto1
                                               OutOfsAuto2,                     //eAuto2
                                               OutOfsAuto3,                     //eAuto3
                                               OutOfsAuto4,                     //eAuto4
                                               OutOfsAuto5,                     //eAuto5
                                               OutOfsAuto6,                     //eAuto6
                                               OutOfsFix1,                      //eFix1
                                               OutOfsFix2,                      //eFix2
                                               OutOfsFix3,                      //eFix3
                                               OutOfsFix1,                      //eFix4
                                               OutOfsFix2,                      //eFix5
                                               OutOfsFix3,                      //eFix6
                                               OutOfsFix1,                      //eFix7
                                               OutOfsFix2,                      //eFix8
                                               OutOfsFix3,                      //eFix9
                                               OutOfsFix4,                      //eFix10
                                               OutOfsFix5,                      //eFix11
                                               OutOfsFix6,                      //eFix12
                                               OutOfsFix3,                      //eBulkBox
                                               OutOfsAuto3,                     //eMag1
                                               OutOfsAuto3,                     //eMag2
                                               OutOfsAuto3,                     //eMag3
                                               OutOfsAuto3,                     //eMag4
                                               OutOfsAuto3,                     //eMag5
                                               OutOfsAuto3,                     //eMag6
                                               OutOfsAuto3,                     //eMag7
                                               OutOfsAuto3,                     //eMag8
                                               OutOfsAuto3,                     //eMag9
                                               OutOfsAuto3,                     //eMag10
                                               OutOfsAuto3,                     //eMag11
                                               OutOfsAuto3,                     //eMag12
                                               OutOfsAuto3,                     //eMag13
                                               OutOfsAuto3,                     //eMag14
                                              };
        return iWhichAutoToOutOffset[iWhichAuto];
    }
}

//==============================================================================
//  "Fill The Tray After Out Arm Place" family -- Jimmychiu 20240726.
//  golden aoutarm9045.cpp:4081-4462, translated in golden order.
//  Static-init safety (trap 4): `mFillTray` is a plain aggregate
//  (Public/HTEditList.h:458-469) whose only non-trivial members are uPoint2D,
//  and uPoint2D::uPoint2D() (Public/HTEditList.cpp:2712) assigns X/Y only.
//  `iOutArmFillTheTrayAfterPlaceAllICTask` is a plain int.  Neither ctor
//  touches any of the 18 NULL globals in docs/PT_CAMPAIGN_PLAN.md §8 -- read
//  the two ctor bodies to confirm, did not assume.
//==============================================================================
//  mFillTray + IsEnableFillTheTrayAfterOutArmPlace -- golden :4081-4090
MyFillTheTrayAfterOutArmPlace mFillTray;
bool IsEnableFillTheTrayAfterOutArmPlace()                                      //Jimmychiu 20240726 : Fill The Tray After Out Arm Place
{
    if(CosFunction.bFillTheTrayAfterOutArmPlace==true &&
       IniConfig.bE85_FillTray_Enable==true)
    {
        return true;
    }
    return false;
}
//------------------------------------------------------------------------------
//  IsRunWhichAtuoFillTray -- golden aoutarm9045.cpp:4092-4108  (golden spells
//  it "Atuo"; the typo is part of the symbol name and is preserved)
bool IsRunWhichAtuoFillTray(int iWhichAuto)                                     //Jimmychiu 20240726 : Fill The Tray After Out Arm Place
{
    if(IsEnableFillTheTrayAfterOutArmPlace())
    {
        if(iWhichAuto==iSortingTrayAuto3 &&
           IniConfig.bE85_FillTray_Auto3==true)
        {
            return true;
        }
        else if(iWhichAuto==iSortingTrayAuto1 &&
                IniConfig.bE85_FillTray_Auto1==true)
        {
            return true;
        }
    }
    return false;
}
//------------------------------------------------------------------------------
//  RecTrayICStatus -- golden aoutarm9045.cpp:4110-4125  (file-scope, golden
//  gives it no header declaration; only HasGapsInTheTray calls it)
//  NOTE the deliberate argument order at the call sites (:4142/:4155): golden
//  passes (irow, icol) and the body indexes Tray.Data[icol][irow].
bool RecTrayICStatus(TTrayMotor *myMotor, int irow, int icol, bool &bHasMidEmpty, uPoint2D &pTargetPos, uPoint2D &pSourcePos)                                   //Jimmychiu 20240726 : Fill The Tray After Out Arm Place
{
    if(bHasMidEmpty==false && myMotor->Tray.Data[icol][irow]==HAS_NULL_IC)
    {
        pTargetPos.X=icol;
        pTargetPos.Y=irow;
        bHasMidEmpty=true;
    }
    else if(bHasMidEmpty==true && myMotor->Tray.Data[icol][irow]==HAS_IC)
    {
        pSourcePos.X=icol;
        pSourcePos.Y=irow;
        return true;
    }
    return false;
}
//------------------------------------------------------------------------------
//  HasGapsInTheTray -- golden aoutarm9045.cpp:4127-4165
//  GOLDEN QUIRK PRESERVED (:4134-4135): GetTrayDirection() writes `Direction`,
//  then the very next line overwrites it with AutoForm[iWhichAuto]->Direction.
//  The GetTrayDirection call is still load-bearing -- it also fills the six
//  ix/iy Start/End/Step loop bounds.  Reproduced verbatim.
bool HasGapsInTheTray(int iWhichAuto, uPoint2D &pTargetPos, uPoint2D &pSourcePos)                                       //Jimmychiu 20240726 : Fill The Tray After Out Arm Place
{
    bool bHasMidEmpty=false;
    TTrayMotor *myMotor=&MOT[iMMAuto[iWhichAuto]];
    if(myMotor->fHasTray==false || myMotor->HasIC()==false)
        return false;
    int Direction=0, ixStart=0, ixEnd=0, ixStep=0, iyStart=0, iyEnd=0, iyStep=0;
    GetTrayDirection(iWhichAuto, Direction, ixStart, ixEnd, ixStep, iyStart, iyEnd, iyStep);
    Direction=AutoForm[iWhichAuto]->Direction;
    if(Direction<4)
    {
        for(int irow=iyStart; irow!=iyEnd; irow+=iyStep)
        {
            for(int icol=ixStart; icol!=ixEnd; icol+=ixStep)
            {
                if(RecTrayICStatus(myMotor, irow, icol, bHasMidEmpty, pTargetPos, pSourcePos))
                {
                    return true;
                }
            }
        }
    }
    else
    {
        for(int icol=ixStart; icol!=ixEnd; icol+=ixStep)
        {
            for(int irow=iyStart; irow!=iyEnd; irow+=iyStep)
            {
                if(RecTrayICStatus(myMotor, irow, icol, bHasMidEmpty, pTargetPos, pSourcePos))
                {
                    return true;
                }
            }
        }
    }
    pTargetPos.Clear();
    pSourcePos.Clear();
    return false;
}
//------------------------------------------------------------------------------
//  DetermineFeasibilityOfSuck -- golden aoutarm9045.cpp:4167-4191
bool DetermineFeasibilityOfSuck(SingleSuckPosOnTray &ssp, uPoint2D pArmSuckActive)                                      //Jimmychiu 20240726 : Fill The Tray After Out Arm Place
{
    ssp.iArmXV1=iXpitchMaxX3;
    ssp.iArmXV2=iXpitchMaxX3;
    ssp.iArmYV1=TestIF.iARM_Y_PITCH;
    uPoint2D pStartPos;
    GetWhichAutoStart(ssp.iWhichAuto,pStartPos);
    //to tray cell(0, 0)
    ssp.pArmXYPos.X=pStartPos.X;
    ssp.pArmXYPos.Y=pStartPos.Y;
    //to tray cell(iy, ix)
    ssp.pArmXYPos.X+=(ssp.pTrayRowCol.X*AutoForm[ssp.iWhichAuto]->XPitch);
    ssp.pArmXYPos.Y-=(ssp.pTrayRowCol.Y*AutoForm[ssp.iWhichAuto]->YPitch);
    //suck xy to cell(iy, ix)
    ssp.pArmXYPos.X-=(pArmSuckActive.X-iOutArmXBase)*iXpitchMax;
    ssp.pArmXYPos.Y+=(pArmSuckActive.Y-iOutArmYBase)*ssp.iArmYV1;
    if(IsOutArmPosOutLimit(ssp.pArmXYPos)==true)
    {
        return false;
    }
    else
    {
        return true;
    }
}
//------------------------------------------------------------------------------
//  GetWhichAutoStart -- golden aoutarm9045.cpp:4193-4206
void GetWhichAutoStart(int iWhichAuto, uPoint2D &pStartPos)                     //Jimmychiu 20240726 : Fill The Tray After Out Arm Place
{
    if(iWhichAuto<3)
    {
        pStartPos.X=Prod.XOutArm_Auto_Place[iWhichAuto][iOutArmYBase][iOutArmXBase];
        pStartPos.Y=Prod.YOutArm_Auto_Place[iWhichAuto][iOutArmYBase][iOutArmXBase];
    }
    else
    {
        iWhichAuto%=3;
        pStartPos.X=Prod.XOutArm_Fix_Place[iWhichAuto][iOutArmYBase][iOutArmXBase];
        pStartPos.Y=Prod.YOutArm_Fix_Place[iWhichAuto][iOutArmYBase][iOutArmXBase];
    }
}
//------------------------------------------------------------------------------
//  GetWhichAutoPickZ -- golden aoutarm9045.cpp:4208-4219
//  GOLDEN QUIRK PRESERVED (:4217): the Fix arm subtracts the AUTO place-offset
//  (`OutArmOffSet[OutOfsAuto1+iWhichAuto]->GetPlace()`) but adds the FIX
//  pick-up offset (`OutArmOffSet[OutOfsFix1+iWhichAuto]->GetPickUp()`).  The
//  mismatched offset bases are golden's; NOT "corrected".
int GetWhichAutoPickZ(int iWhichAuto, int irow, int icol)                       //Jimmychiu 20240726 : Fill The Tray After Out Arm Place
{
    if(iWhichAuto<3)
    {
        return Prod.ZOutArm_Auto_Place[iWhichAuto][irow][icol]-OutArmOffSet[OutOfsAuto1+iWhichAuto]->GetPlace()+OutArmOffSet[OutOfsAuto1+iWhichAuto]->GetPickUp();
    }
    else
    {
        iWhichAuto%=3;
        return Prod.ZOutArm_Fix_Place[iWhichAuto][irow][icol]-OutArmOffSet[OutOfsAuto1+iWhichAuto]->GetPlace()+OutArmOffSet[OutOfsFix1+iWhichAuto]->GetPickUp();
    }
}
//------------------------------------------------------------------------------
//  GetWhichAutoPlaceZ -- golden aoutarm9045.cpp:4221-4232
int GetWhichAutoPlaceZ(int iWhichAuto, int irow, int icol)                      //Jimmychiu 20240726 : Fill The Tray After Out Arm Place
{
    if(iWhichAuto<3)
    {
        return Prod.ZOutArm_Auto_Place[iWhichAuto][irow][icol];
    }
    else
    {
        iWhichAuto%=3;
        return Prod.ZOutArm_Fix_Place[iWhichAuto][irow][icol];
    }
}
//------------------------------------------------------------------------------
//  IsOutArmPosOutLimit -- golden aoutarm9045.cpp:4234-4246
//  The asymmetric margins (+10 on the negative soft limit, -100 on the
//  positive) are golden's magic numbers, kept exactly.
bool IsOutArmPosOutLimit(uPoint2D pArmPos)                                      //Jimmychiu 20240726 : Fill The Tray After Out Arm Place
{
    if(pArmPos.X<(MOT[MOutArmX].Motor->PSoftLimitN+10) || pArmPos.X>(MOT[MOutArmX].Motor->PSoftLimitP-100))
    {
        return true;
    }

    if(pArmPos.Y<(MOT[MOutArmY].Motor->PSoftLimitN+10) || pArmPos.Y>(MOT[MOutArmY].Motor->PSoftLimitP-100))
    {
        return true;
    }
    return false;
}
//------------------------------------------------------------------------------
//  FindArmSuck -- golden aoutarm9045.cpp:4248-4268
bool FindArmSuck(int iWhichAuto, MyFillTheTrayAfterOutArmPlace &mfillT)         //Jimmychiu 20240726 : Fill The Tray After Out Arm Place
{
    mfillT.ssTarget.iWhichAuto=iWhichAuto;
    mfillT.ssTarget.bPlace=true;
    mfillT.ssSource.iWhichAuto=iWhichAuto;
    mfillT.ssSource.bPlace=false;
    for(int irow=0; irow<MAX_ARM_Row; irow++)
    {
        for(int icol=0; icol<MAX_ARM_Col; icol++)
        {
            mfillT.pArmSuckActive.X=icol;
            mfillT.pArmSuckActive.Y=irow;
            if(DetermineFeasibilityOfSuck(mfillT.ssTarget, mfillT.pArmSuckActive)==true &&
               DetermineFeasibilityOfSuck(mfillT.ssSource, mfillT.pArmSuckActive)==true)
            {
                return true;
            }
        }
    }
    return false;
}
//------------------------------------------------------------------------------
//  MoveOutArmXYAndSuck(SingleSuckPosOnTray, uPoint2D) -- golden :4270-4303
//  (the ArmAndSuckInfo overload at golden :4305 is out of this batch's scope --
//   see WAVE SCOPE above)
bool MoveOutArmXYAndSuck(SingleSuckPosOnTray ssTarget, uPoint2D pActiveSuck)    //Jimmychiu 20240726 : Fill The Tray After Out Arm Place
{
    bool bDoSuck=IsOutArmArrival(ssTarget.pArmXYPos);
    if(bDoSuck==false && MoveOutArmToAutoSafe()==false)
    {
        return false;
    }
    bool ZDownFlag[MAX_ARM_Row][MAX_ARM_Col];
    int ZDownPos[MAX_ARM_Row][MAX_ARM_Col];
    int iXVariable[X_PITCH_COUNT];
    int iYVariable=0;
    ZeroMemory(iXVariable,  sizeof(iXVariable));
    ZeroMemory(ZDownFlag,   sizeof(ZDownFlag));
    ZeroMemory(ZDownPos,    sizeof(ZDownPos));
    iXVariable[0]=GetOutArmPitch_9045(ssTarget.iArmXV1);
    iXVariable[1]=GetOutArmPitch2_9045(ssTarget.iArmXV2);
    iYVariable   =GetOutArmPitchY_9045(ssTarget.iArmYV1);
    int irow     =pActiveSuck.Y, icol=pActiveSuck.X;
    ZDownFlag[irow][icol]=true;
    if(ssTarget.bPlace)
    {
        ZDownPos[irow][icol]=GetWhichAutoPlaceZ(ssTarget.iWhichAuto, irow, icol);
    }
    else
    {
        ZDownPos[irow][icol]=GetWhichAutoPickZ(ssTarget.iWhichAuto, irow, icol);
    }
    bool bZDown=true;
    if(OutArmContinuousMove_9045(ssTarget.pArmXYPos.X, ssTarget.pArmXYPos.Y, iXVariable, iYVariable, ZDownFlag, ZDownPos, bZDown))
    {
        return true;
    }
    return false;
}
//------------------------------------------------------------------------------
//  IsOutArmArrival -- golden aoutarm9045.cpp:4325-4330
//  `TMyMotor()` is golden's own temporary (CheckArmPosArrival needs no state);
//  TMyMotor has a public default ctor (Motor/mymotor.h:130), so this is a
//  1:1 translation, not a workaround.
bool IsOutArmArrival(uPoint2D pTargetPos)                                       //Jimmychiu 20240726 : Fill The Tray After Out Arm Place
{
    bool bX=TMyMotor().CheckArmPosArrival(pTargetPos.X, MOT[MOutArmX].ReadPos(), 10);
    bool bY=TMyMotor().CheckArmPosArrival(pTargetPos.Y, MOT[MOutArmY].ReadPos(), 10);
    return (bX && bY);
}
//------------------------------------------------------------------------------
//  iOutArmFillTheTrayAfterPlaceAllICTask + DoOutArmFillTheTrayAfterPlaceAllIC
//  -- golden aoutarm9045.cpp:4332-4462
//  `int &Task=iOutArmFillTheTrayAfterPlaceAllICTask;` is golden's reference-to-
//  cursor idiom, kept.  `bResult` is written once (false) and never set true --
//  golden's, not a translation slip: the SM signals completion through Task,
//  and the `case 1` "nothing to fill" exit is the only `return true`.
//  The `#ifdef SOFT_SIMULTE` fMain->chkInPickLoadError arms are verbatim/inert.
//------------------------------------------------------------------------------
int iOutArmFillTheTrayAfterPlaceAllICTask;
bool DoOutArmFillTheTrayAfterPlaceAllIC(bool bIsFirst, int iWhichAuto)          //Jimmychiu 20240726 : Fill The Tray After Out Arm Place
{
    bool bResult=false;
    int &Task=iOutArmFillTheTrayAfterPlaceAllICTask;
    if(bIsFirst)
    {
        Task=1;
        mFillTray.Clear();
        return true;
    }
    int ret=0;
    switch(Task)
    {
        case 1:
            if(HasGapsInTheTray(iWhichAuto, mFillTray.ssTarget.pTrayRowCol, mFillTray.ssSource.pTrayRowCol)==true &&
               FindArmSuck(iWhichAuto, mFillTray)==true)
            {
                Task=100;
            }
            else
            {
                return true;
            }
            break;
        case 100:                                                               //move to source tray
            if(MoveOutArmXYAndSuck(mFillTray.ssSource, mFillTray.pArmSuckActive))
            {
                if(OutArmNeedCheckOffset(false, 0))                             //Steven 20230531 : 簡化判斷式
                {
                    bEnterOffset=false;
                    fMain->Pause("DoOutArmFillTheTrayAfterPlaceAllIC");
                    Task=100;
                    break;
                }
                OutArmSuck.ResetAll();                                          //Steven 20160323 : 避免未開啟真空
                Task=200;
            }
            break;
        case 200:
            if(OutArmSuck.Suck[mFillTray.pArmSuckActive.Y][mFillTray.pArmSuckActive.X].Suck())
            {
                #ifdef SOFT_SIMULTE                                             //模擬ic不見了
                if(fMain->chkInPickLoadError->Checked==true)
                {
                    OutArmSuck.Suck[mFillTray.pArmSuckActive.Y][mFillTray.pArmSuckActive.X].Error=true;
                }
                #endif                                                          //模擬ic不見了 end
                MOT[iMMAuto[iWhichAuto]].SetTrayBinData(mFillTray.ssSource.pTrayRowCol.X, mFillTray.ssSource.pTrayRowCol.Y, NULL_IC, "");
                OutArmSuck.SetItemData(mFillTray.pArmSuckActive.Y, mFillTray.pArmSuckActive.X, HAS_IC);
                InspectOutArmPosition(iWhichAuto, mFillTray.pArmSuckActive.Y, mFillTray.pArmSuckActive.X, mFillTray.ssSource.pTrayRowCol.Y, mFillTray.ssSource.pTrayRowCol.X, iOutPlaceToAuto);
                Task=300;
            }
            else
            {
                if(OutArmSuck.Suck[mFillTray.pArmSuckActive.Y][mFillTray.pArmSuckActive.X].Error==false)
                    break;
            }

            if(OutArmSuck.Suck[mFillTray.pArmSuckActive.Y][mFillTray.pArmSuckActive.X].Error)
            {
                AnsiString ErrPart=OutArmSuck.Suck[mFillTray.pArmSuckActive.Y][mFillTray.pArmSuckActive.X].sName;
                ret=OutArmPickTrayAlarm(iWhichAuto, ErrPart);
                if(ret==K_RETRY)
                {
                    Task=100;
                }
                else                                                            //K_SKIP
                {
                    MOT[iMMAuto[iWhichAuto]].SetTrayBinData(mFillTray.ssSource.pTrayRowCol.X, mFillTray.ssSource.pTrayRowCol.Y, NULL_IC, "");
                    Task=500;
                }
            }
            break;
        case 300:                                                               //move to target tray
            if(MoveOutArmXYAndSuck(mFillTray.ssTarget,mFillTray.pArmSuckActive))
            {
                if(OutArmNeedCheckOffset(false, 0))                             //Steven 20230531 : 簡化判斷式
                {
                    bEnterOffset=false;
                    fMain->Pause("DoOutArmFillTheTrayAfterPlaceAllIC");
                    Task=300;
                    break;
                }
                OutArmSuck.ResetAll();                                          //Steven 20160323 : 避免未開啟真空
                Task=400;
            }
            break;
        case 400:
            if(OutArmSuck.Suck[mFillTray.pArmSuckActive.Y][mFillTray.pArmSuckActive.X].Destroy())
            {
                #ifdef SOFT_SIMULTE                                             //模擬ic不見了
                if(fMain->chkInPickLoadError->Checked==true)
                {
                    OutArmSuck.Suck[mFillTray.pArmSuckActive.Y][mFillTray.pArmSuckActive.X].Error=true;
                }
                #endif                                                          //模擬ic不見了 end
                MOT[iMMAuto[iWhichAuto]].SetTrayBinData(mFillTray.ssTarget.pTrayRowCol.X, mFillTray.ssTarget.pTrayRowCol.Y, HAS_IC , "");
                OutArmSuck.SetItemData(mFillTray.pArmSuckActive.Y, mFillTray.pArmSuckActive.X, NULL_IC);
                Task=500;
            }
            else
            {
                if(OutArmSuck.Suck[mFillTray.pArmSuckActive.Y][mFillTray.pArmSuckActive.X].Error==false)
                    break;
            }

            if(OutArmSuck.Suck[mFillTray.pArmSuckActive.Y][mFillTray.pArmSuckActive.X].Error)
            {
                AnsiString ErrPart=OutArmSuck.Suck[mFillTray.pArmSuckActive.Y][mFillTray.pArmSuckActive.X].sName;
                ret=ShowErrorMessage("JAM0217", K_RETRY|K_SKIP, MOutArmX, false, ErrPart);
                if(ret==K_RETRY)
                {
                    Task=300;
                }
                else                                                            //K_SKIP
                {
                    OutArmSuck.SetItemData(mFillTray.pArmSuckActive.Y, mFillTray.pArmSuckActive.X, NULL_IC);
                    Task=500;
                }
            }
            break;
        case 500:
            if(MoveOutArmToAutoSafe()==true)
            {
                Task=1;
            }
            break;
    }
    return bResult;
}

//------------------------------------------------------------------------------
//  GetOutArmXToSortShtOffset -- golden aoutarm9045.cpp:4464-4472
//  9046AU sort-shuttle offset.  No stub existed for this pair.
//------------------------------------------------------------------------------
int GetOutArmXToSortShtOffset(int iOffsetPos)                                   //RogerYang 20250510 Add for 9046AU
{
    int iPos=0;
    if(iOffsetPos>=0)
    {
        iPos=SortArmOffSet[iOffsetPos]->GetX();
    }
    return iPos;
}
//------------------------------------------------------------------------------
//  GetOutArmYToSortShtOffset -- golden aoutarm9045.cpp:4474-4482
//------------------------------------------------------------------------------
int GetOutArmYToSortShtOffset(int iOffsetPos)                                   //RogerYang 20250510 Add for 9046AU
{
    int iPos=0;
    if(iOffsetPos>=0)
    {
        iPos=SortArmOffSet[iOffsetPos]->GetY();
    }
    return iPos;
}
//------------------------------------------------------------------------------
//  NeedPlaceToSort -- golden aoutarm9045.cpp:4484-4506
//  REPLACES the one-line offline stub aoutarm_shims.cpp:121 (`return false;`).
//  Offline the answer is STILL false, because USE_OUT_SORT_ARM==eartUninstall
//  short-circuits at :4486 -- so this retirement is behaviour-neutral on a
//  non-9046AU machine and correct on a 9046AU one.
//------------------------------------------------------------------------------
bool NeedPlaceToSort()                                                          //RogerYang 20250513 Add for 9046AU
{
    if(USE_OUT_SORT_ARM==eartUninstall)
    {
        return false;
    }

    bool bIsNeedSort=false;
    for(int i=0; i<OutArmSuck.iMaxRow; i++)
    {
        for(int j=0; j<OutArmSuck.iMaxCol; j++)
        {
            if(OutArmSuck.Item[i][j]!=NULL_IC &&
               (OutArmSuck.iWhichAuto[i][j]==eAuto4 ||
                OutArmSuck.iWhichAuto[i][j]==eAuto5 ||
                OutArmSuck.iWhichAuto[i][j]==eAuto6))
            {
                bIsNeedSort=true;
            }
        }
    }
    return bIsNeedSort;
}
//------------------------------------------------------------------------------
//  SetSortShuttleStatus_Place -- golden aoutarm9045.cpp:4508-4534
//  REPLACES the no-op offline stub aoutarm_shims.cpp:146.
//  `OutSht3Kit` is the 9046AU sort kit: declared acarry_shims.h:101, DEFINED
//  acarry_shims.cpp:67, and it is a `TMyKitSuck` from aHotPlateSubstrate.h:365
//  -- the SAME header this TU already includes, so the layout the linker binds
//  is the layout this code compiled against (trap 5 checked, see report).
//  Golden's commented-out InspectOutArmPosition line at :4520 is kept as a
//  comment, verbatim, including its Chinese note.
//------------------------------------------------------------------------------
extern TMyKitSuck OutSht3Kit;                                                   // DECL acarry_shims.h:101 / DEF acarry_shims.cpp:67 -- golden MyKitSuck.h (9046AU sort kit)
void SetSortShuttleStatus_Place(int iShtRow, int iShtCol, int iSuckRow, int iSuckCol)
{
    AnsiString Str;

    if(OutArmSuck.Item[iSuckRow][iSuckCol]!=HAS_NULL_IC)                        //Steven 20170109 : 沒IC的地方不檢查吹氣
    {
        bOutArmCheckDestroyACT[iSuckRow][iSuckCol]=true;                        //jou 981130 確認device確實destroy完成
    }

    if(OutSht3Kit.Item[iShtRow][iShtCol]==NULL_IC)
    {
        OutArmSuck.PordRec[iSuckRow][iSuckCol].AddArmSiteRecord(3, OutArmSuck.iWhichSite[iSuckRow][iSuckCol]);          //Steven 20240508 : fixed for iWhichSite flag
        //InspectOutArmPosition(MOutSortSht, iSuckRow, iSuckCol, iShtRow, iShtCol, true);   //裡面還沒寫 針對outarm放到shuttle的功能
        OutSht3Kit.MoveSuckDataDiff(OutArmSuck, iSuckRow, iSuckCol, iShtRow, iShtCol);
        OutSht3Kit.iWhichSite[iShtRow][iShtCol]=OutSht3Kit.iWhichSite[iShtRow][iShtCol]-1;                              //Steven 20240320 : 變更iWhcihSite填入位置
        OutSht3Kit.iWhichIndex[iShtRow][iShtCol]=0;                             //Steven 20240320 : 變更iWhichIndex填入位置
    }
    else if(OutArmSuck.Item[iSuckRow][iSuckCol]==HAS_NULL_IC)
    {
        OutArmSuck.SetItemData(iSuckRow, iSuckCol, NULL_IC);
    }
    else
    {
        Str.sprintf("SortShuttleKit.Item[%d][%d]=%d, OutArmSuck.Item[%d][%d]=%d", iShtRow, iShtCol, OutSht3Kit.Item[iShtRow][iShtCol], iSuckRow, iSuckCol, OutArmSuck.Item[iSuckRow][iSuckCol]);  //Steven 20220620 : add log message
        ShowMyMessage("Program Error in SetSortShuttleStatus", Str);
    }
}

//------------------------------------------------------------------------------
//  InitNewFixTrayForUnloaderClipRead -- golden aoutarm9045.cpp:4536-4605
//  GATE (G2) -- see GATE REGISTER at the top of this block.  Golden's first and
//  last statements stay ACTIVE; the fBarCode-dependent middle is `#if 0`.
//------------------------------------------------------------------------------
bool InitNewFixTrayForUnloaderClipRead(int iwhichauto,bool bOutputReport)       //Jimmychiu 20250818 : 海康智能讀碼
{
    bool bret=false;
#if 0 // GATE (G2) -- golden aoutarm9045.cpp:4539-4603: needs fBarCode->eUnloaderClipAuto1/… (golden BarCode/BarCode.h:966-975) and fBarCode->ccdUnloader[] (golden BarCode/BarCode.h:976); this tree's TfBarCode (BarCode/BarCode.h, partial) carries neither.  Offline result is golden's own: bret stays false.
    AnsiString sErr="";
    AnsiString sGet="";
    AnsiString sAreaName="";
    //
    int iiccdUnloaderID=0;
    if(iwhichauto==(int)eAuto1)
    {
        iiccdUnloaderID=(int)(fBarCode->eUnloaderClipAuto1);
        sAreaName="Auto1";
    }
    else if(iwhichauto==(int)eAuto2)
    {
        iiccdUnloaderID=(int)(fBarCode->eUnloaderClipAuto2);
        sAreaName="Auto2";
    }
    else if(iwhichauto==(int)eAuto3)
    {
        iiccdUnloaderID=(int)(fBarCode->eUnloaderClipAuto3);
        sAreaName="Auto3";
    }
    else if(iwhichauto==(int)eFix1)
    {
        iiccdUnloaderID=(int)(fBarCode->eUnloaderClipFix1);
        sAreaName="Fix1";
    }
    else if(iwhichauto==(int)eFix2)
    {
        iiccdUnloaderID=(int)(fBarCode->eUnloaderClipFix2);
        sAreaName="Fix2";
    }
    else if(iwhichauto==(int)eFix3)
    {
        iiccdUnloaderID=(int)(fBarCode->eUnloaderClipFix3);
        sAreaName="Fix3";
    }
    else
    {
        iiccdUnloaderID=(int)(fBarCode->eUnloaderClipAuto1);
        sAreaName="Unknown Area";
    }
    //
    if(fBarCode->ccdUnloader[iiccdUnloaderID].bGetResult==false)
    {
        sErr="";
        sGet=fBarCode->ccdUnloader[iiccdUnloaderID].DoGetPhotoCmd(false,sErr);
        if(sGet.IsEmpty()==false)
        {
            if(bOutputReport==true)
            {
                //out put report
            }
            InitNewFixTray(iwhichauto, "AOutArm_FullIC");                       //Steven 20160414 : 整合Fix盤設定
            MOT[iMMAuto[iwhichauto]].Tray.cCassetteID=sGet;
            bret=true;
        }
        else if(sErr.IsEmpty()==false)
        {
            ShowMyMessage(AnsiString().sprintf("%s CCD Clip ID:%s",sAreaName,sGet));
            fBarCode->ccdUnloader[iiccdUnloaderID].DoGetPhotoCmd(true,sErr);
        }
    }
    else
    {
        bret=true;
    }
#endif
    return bret;
}

//=============================================================================
//  END PT-W8 append block.
//=============================================================================

// ===========================================================================
//  AI(W906-ARM1) 20260927: golden 出料臂吸料那一側（NB2 R105 ②⑧）逐行照翻：
//  InitialOutArmNeedSuck（:3535-3590）：清吸嘴的 NeedSuck／NeedDestroy，入料飛梭還在動或還沒到右邊就回 false（kevin 20120612 防重吸），
//    再設 ptrOutSHT。以前是回 false 的樁 ⇒ 27 支出料臂 variant 的 OutArmZNeedDown_* 第一行就回 false，出料臂停在飛梭上方不會下去吸。
//  VerifyFixTrayLink（:1600-1672）：Fix 盤滿／Auto 盤連結的換盤；以前回 0（跳過）。
//  OutArmNeedCheckOffset（:3592-3627）：操作員開手動 Step／Auto Offset／Setup Teach 時暫停；以前回 false（不停）。
//  移植樹沒有的相依才閘（GATE(W906-ARM1)）。
// ===========================================================================
// ---- golden aoutarm9045.cpp:1600-1672 VerifyFixTrayLink ----
int VerifyFixTrayLink()
{
    if(AUTO3_IS_MAGAZINE==1 &&                                                  //JerryYang 20221215 : add Magazine
       iWhichAuto==iMagAtAuto)
    {
        return 10000;
    }
    else if(AUTO3_IS_MAGAZINE==1             &&                                 //JerryYang 20221215 : Magazine把fix區當buffer區功能
            TestIF_File.iMagFixTrayType==1   &&
            iWhichAuto>=iMagMin)
    {
        return 0;
    }
    else
    {
        SetFixTrayFullIC();
        if(MOT[iMMAuto[iWhichAuto]].fHasTray==false ||
           (MOT[iMMAuto[iWhichAuto]].FullIC() &&
            CheckUseFixBinBoxFunction()==false))                                //kevin 20160822
        {
            if(iWhichAuto>=iFixMin)
            {
                if(TrayForm.iFixTrayMode==0)                                    // full tray
                {
                    if(iWhichAuto==iFixMin && Prod.bLinkTo6Tray[eFix2])
                    {
                        for(int j=0; j<OutArmSuck.iPickCol; j++)
                        {
                            if(OutArmSuck.Item[iOutArmPlaceOrder][j]!=NULL_IC &&
                               OutArmSuck.iWhichAuto[iOutArmPlaceOrder][j]==iWhichAuto)
                            {
                                OutArmSuck.iWhichAuto[iOutArmPlaceOrder][j]=iWhichAuto+1;
                                return 1;
                            }
                        }
                    }

                    if(iWhichAuto==iFixMin+1 && TrayForm.bFixTrayLink[eFix3])   // fix 2 full link fix3
                    {
                        for(int j=0; j<OutArmSuck.iPickCol; j++)
                        {
                            if(OutArmSuck.Item[iOutArmPlaceOrder][j]!=NULL_IC &&
                               OutArmSuck.iWhichAuto[iOutArmPlaceOrder][j]==iWhichAuto)
                            {
                                OutArmSuck.iWhichAuto[iOutArmPlaceOrder][j]=iWhichAuto+1;
                                return 1;
                            }
                        }
                    }
                }
                return 4000;
            }
            else if(IniConfig.bAutoTrayLink==true)                              //jou 2012-06-14 Auto Tray Link
            {
                if((iWhichAuto==eAuto2 && Prod.bLinkTo6Tray[eAuto3]==true) ||
                   (iWhichAuto==eAuto1 && Prod.bLinkTo6Tray[eAuto2]==true))
                {
                    for(int j=0; j<OutArmSuck.iPickCol; j++)
                    {
                        if(OutArmSuck.Item[iOutArmPlaceOrder][j]!=NULL_IC &&
                           OutArmSuck.iWhichAuto[iOutArmPlaceOrder][j]==iWhichAuto)
                        {
                            OutArmSuck.iWhichAuto[iOutArmPlaceOrder][j]=iWhichAuto+1;
                        }
                    }
                    return 1;
                }
            }
            MoveOutArmXY_ToFix_Tray_Full();
        }
    }
    return 0;
}
// ---- golden aoutarm9045.cpp:3535-3590 InitialOutArmNeedSuck ----
bool InitialOutArmNeedSuck(int iShuttle)                                        //Steven 20220927 : 重新整理In / Out Arm Need Suck
{
    int iPos1=0, iPos2=0;
    bool bLedFlag1=false, bLedFlag2=false;

    for(int i=0; i<OutArmSuck.iMaxRow; i++)
    {
        for(int j=0; j<OutArmSuck.iMaxCol; j++)
        {
            if(iShuttle==2)                                                     //RogerYang 20250513 Add for 9046AU
                OutArmSuck.Suck[i][j].SetNeedDestroy(false);
            else
                OutArmSuck.Suck[i][j].SetNeedSuck(false);                       //inital
        }
    }

    //AI(W906-FLOW9050-A) 20261001: H074 —— 照 910 aoutarm9045.cpp:3581-3591：Type_HT9050 只有一組出料 Shuttle（MOutShuttle1），不論 iShuttle 都看
    //  它的到位燈與 Prod.OutSHT[0].iRight（容差同下面各臂）；其他機種照舊依 iShuttle 走 MInShuttle1／MInShuttle2／MOutSortSht。
    //  今天 9050GPIB 解成 Type_HT9046_LS（database.cpp:517）⇒ 永遠走 else if，行為零變化。原本這一行是 `if(iShuttle==0)`。
    if(MachineTypeChoice==Type_HT9050)
    {
        iPos1=MOT[MOutShuttle1].ReadPos();
        MOT[MOutShuttle1].ScanMotorStatus();
        bLedFlag1=MOT[MOutShuttle1].Led[iInposLed];
        if(bLedFlag1 || (iPos1<(Prod.OutSHT[0].iRight+InputLimit.iOffsetXYLow*100)))
        {
            return false;                                                       //kevin 20120612 ourarm double pick
        }
    }
    else if(iShuttle==0)                                                             //jou 980916 start : offset shuttle right hang up
    {
        iPos1=MOT[MInShuttle1].ReadPos();
        MOT[MInShuttle1].ScanMotorStatus();
        bLedFlag1=MOT[MInShuttle1].Led[iInposLed];
        if(bLedFlag1 || (iPos1<(Prod.InSHT[0].iRight+InputLimit.iOffsetXYLow*100)))
        {
            return false;                                                       //kevin 20120612 ourarm double pick
        }
    }
    else if(iShuttle==1)
    {
        iPos2=MOT[MInShuttle2].ReadPos();
        MOT[MInShuttle2].ScanMotorStatus();
        bLedFlag2=MOT[MInShuttle2].Led[iInposLed];
        if(bLedFlag2 || (iPos2<(Prod.InSHT[1].iRight+InputLimit.iOffsetXYLow*100)))
        {
            return false;                                                       //kevin 20120612 ourarm double pick
        }
    }
    else if(iShuttle==2)
    {                                                                           //RogerYang 20250513 Add for 9046AU
        iPos2=MOT[MOutSortSht].ReadPos();
        MOT[MOutSortSht].ScanMotorStatus();
        bLedFlag2=MOT[MOutSortSht].Led[iInposLed];
        if(bLedFlag2 || (iPos2<(Prod.SortSHT.iRight+InputLimit.iOffsetXYLow*100)))
        {
            return false;                                                       //kevin 20120612 ourarm double pick
        }
    }

    if(iShuttle==0)
        ptrOutSHT=&FRCarryKit;
    else if(iShuttle==1)                                                        //RogerYang 20250513 Add for 9046AU
        ptrOutSHT=&BRCarryKit;
    else
        ptrOutSHT=&OutSht3Kit;

    return true;
}
// ---- golden aoutarm9045.cpp:3592-3627 OutArmNeedCheckOffset ----
bool OutArmNeedCheckOffset(bool bPlace, int iPos)                               //Steven 20230531 : 簡化判斷式
{
    bool bresult=false;
    eOutOffset eOutOffsetPos;
    AnsiString sOutOffsetPos;
    if(bPlace)
    {
        int ct=GetOutOffsetFromWhichAuto(iPos);                                 //Jimmychiu 20240731 : for out arm offset value
        sOutOffsetPos=CapStrOutput[ct];                                         //Steven 20230907 : For HT-9011UC
        eOutOffsetPos=eOutOffset(ct);

        if((AccessLevel>=1 || CosFunction.bOPCanPressStepAndTStart) &&          //JerryYang 20170417 (wei) OP權限也能按Step跟T.Start
           (Sen[SnRKManualStep].IsOn() ||
            fOffSet->UseAutoOffsetFunction(sOutOffsetPos) ||                    //jou 2013-08-29 Use Auto Offset Funtion
            fOffSet->UseOutArmSetupTeach(eOutOffsetPos)))                       //JerryYang 20180921 Setup Teach功能 //jou 2013-08-29 Use Auto Offset Funtion
            bresult=true;
    }
    else
    {
        eOutOffsetPos=(iPos==0)?OutOfsOutSh1:OutOfsOutSh2;
        sOutOffsetPos=(iPos==0)?"Output Shuttle1":"Output Shuttle2";

        if((AccessLevel>=1 || CosFunction.bOPCanPressStepAndTStart) &&          //JerryYang 20170417 (wei) OP權限也能按Step跟T.Start
           (Sen[SnRKManualStep].IsOn() ||
            fOffSet->UseAutoOffsetFunction(sOutOffsetPos) ||                    //jou 2013-08-29 Use Auto Offset Funtion
            fOffSet->UseOutArmSetupTeach(eOutOffsetPos)))                       //JerryYang 20180921 Setup Teach功能
            bresult=true;
    }

    if(bresult)
    {
        bEnterOffset=false;
        fMain->Pause("OutArmNeedCheckOffset");
    }
    return bresult;
}

// ===========================================================================
//  AI(W906-ARM2) 20260927: golden 出料臂放料那一側（NB2 R105 ④⑤）逐行照翻：
//  SearchTrayToPlace_9045（:1316-1418）：依吸嘴上 IC 的 iWhichAuto 找要放的盤、設 iOutArmPlaceOrder（含 FIFO、ATK AMR、Magazine 分支）；
//    以前回 0 ⇒ 永遠放第一個 Auto 盤、不分 bin。
//  SetOutArm_9045（:2493-2523）→ SearchUnLoadTrayUpDown_9045（:2451-2491）→ DoMoveOutArmXYToPlace_9045（:2333-2449）：
//    出料臂移到放料位置；以前三支都是回 false 的樁 ⇒ DoOutArmPlaceToAuto_9045（本檔 :1441）永遠移不到放料位置。
//  移植樹沒有的相依才閘（GATE(W906-ARM2)）。
// ===========================================================================
// AI(W906-ARM2) 20260927: golden 經 asortarm.h／acatchtray.h 取得；本 TU 沒有 include 它們，只補宣告（簽名逐字抄移植樹的標頭）。
extern int iSortArmPlaceOrder;                 // asortarm.cpp:573 / asortarm.h:63（golden asortarm.cpp:41 / asortarm.h:13）—— NB2 R105 說「全樹沒有」是過期的
bool IsTrayArmMoveAvoidOutArmCrash();          // acatchtray.cpp:8144 / acatchtray.h:32
// ---- golden aoutarm9045.cpp:1316-1418 SearchTrayToPlace_9045 ----
int SearchTrayToPlace_9045(bool bIsSort)                                        //Steven 20160524 : 簡化Out Arm程式碼
{
//    int iSuckX=0, iSuckY=3;
//    if(Prod.bSingleUseOtherSuck)                                                //wei 20220823 Single Site使用C吸嘴判斷
//    {
//        iSuckY=2;
//    }

    TMyKitSuck *ptrOutArmSuck;
    if(bIsSort==true)                                                           //RogerYang 20250521 Add for 9046AU
        ptrOutArmSuck=&OutArm2Suck;
    else
        ptrOutArmSuck=&OutArmSuck;

    if(IniConfig.bI37_EnableFIFOMode &&
       LastSet.iRunStartMode==rsmFIFOMode)                                      //Ifor 20181127 : Find_OutArm_Single 搬至 SearchTrayToPlace_9045()判斷避免進入DoMoveOutArmXYToPlace_9045
    {
        for(int i=0; i<ptrOutArmSuck->iMaxRow; i++)
        {
            for(int j=0; j<ptrOutArmSuck->iMaxCol; j++)
            {
                if(ptrOutArmSuck->Item[i][j]!=NULL_IC)
                {
                    if(iLoaderNum==0)                                           //Frank 20181121 add iLoaderNum
                        iLoaderNum=ptrOutArmSuck->PordRec[i][j].GetLoaderNum();
                    if(iLoaderNum!=ptrOutArmSuck->PordRec[i][j].GetLoaderNum())
                    {
                        if(MOT[MMAuto2].HasIC())                                //Ifor 20181127 : add 避免滿Tray後換Tray會再次換Tray
                        {
                            MOT[MMAuto2].InitNewTray(HAS_IC, false, __FUNC__);
                        }
                        iLoaderNum=ptrOutArmSuck->PordRec[i][j].GetLoaderNum();
                    }
                }
            }
        }

        if(CUSTOMER_CODE==CC_KYEC_LEE)                                          //Ifor 20181107 : add KYEC FIFO 強制放至Auto 2
            return 1;
    }

    if(fAGV->IsATK_AMR() &&
       LastSet.iUnloadFixTray==eAtkTfMoveFixIC)                                 //Steven 20260202 : for ATK AMR
    {
        if(OutArmSuck.ArmRow0HaveRealIC())
            iOutArmPlaceOrder=0;
        else
            iOutArmPlaceOrder=1;
        return iFixToWhichAuto-eFix1;
    }

    if(AUTO3_IS_MAGAZINE==1 && bIsSort==false)                                  //RogerYang 20250521 Add for 9046AU    //JerryYang 20221215 : add Magazine
    {
        return SearchTrayToPlace_Magazine();
    }

    iOutArmPlaceOrder=0;
    iSortArmPlaceOrder=0;                                                       //RogerYang 20250521 Add for 9046AU
    int iPlaceOrder=0;
//    if(TestIF.iTestMode==SingleSite)
//    {
//        for(int k=0; k<eTrayCount; k++)
//        {
////            if(Prod.iTrayType[k]==tNotUse)
////                continue;
//
//            if(OutArmSuck.Item[iSuckX][iSuckY]!=NULL_IC &&
//               OutArmSuck.iWhichAuto[iSuckX][iSuckY]==iSortTrayIndex[k])
//            {
//                iOutArmPlaceOrder=0;
//                return iSortTrayIndex[k];
//            }
//        }
//    }
//    else
//    {
        for(int k=0; k<eTrayCount; k++)                                         //JerryYang 20230820
        {
            for(int i=0; i<ptrOutArmSuck->iMaxRow; i++)
            {
                for(int j=0; j<ptrOutArmSuck->iMaxCol; j++)
                {
                    if(ptrOutArmSuck->Item[i][j]!=NULL_IC &&
                       ptrOutArmSuck->iWhichAuto[i][j]==iSortTrayIndex[k])
                    {
                        if(i==0)
                            iPlaceOrder=0;
                        else
                            iPlaceOrder=1;

                        if(bIsSort==true)                                       //RogerYang 20250521 Add for 9046AU
                            iSortArmPlaceOrder=iPlaceOrder;
                        else
                            iOutArmPlaceOrder=iPlaceOrder;
                        return iSortTrayIndex[k];
                    }
                }
            }
        }
//    }

    return 4;
}
// ---- golden aoutarm9045.cpp:2333-2449 DoMoveOutArmXYToPlace_9045 ----
bool DoMoveOutArmXYToPlace_9045(bool &bCanPick2ICAtOnceTime, int iRow, int iCol, bool RealMove)
{
    int iOutPutTray=0;
    int iXPos=0, iYPos=0;
    int iXVariable[X_PITCH_COUNT];
    int iYVariable =0;
    int iMovePitchX=AutoCalculateOutArmXClosePitch(iWhichAuto+bOutArmXOverLimit);                                       //Steven 20251101 : 修正超出極限後, 要重新算一次X-Pitch
    int iMovePitchY=AutoCalculateOutArmYClosePitch(iWhichAuto);
    bool bZFlag[MAX_ARM_Row][MAX_ARM_Col];
    int iZPos[MAX_ARM_Row][MAX_ARM_Col];
    int iOffsetPos=-1;
    bool bNeedDown=true;
    bool bFlag=false;
    ZeroMemory(iXVariable, sizeof(iXVariable));
    ZeroMemory(bZFlag, sizeof(bZFlag));
    ZeroMemory(iZPos, sizeof(iZPos));

    if(AUTO3_IS_MAGAZINE==1             &&                                      //JerryYang 20221215 : Magazine把fix區當buffer區功能
       TestIF_File.iMagFixTrayType==1   &&
       iWhichAuto>=iMagMin)
    {
        iOutPutTray=iWhichBuff;
    }
    else
    {
        iOutPutTray=iWhichAuto;
    }

    if(IsTrayArmMoveAvoidOutArmCrash())
    {
        MOT[MOutArmX].PCIL132_StopMotor();
        MOT[MOutArmY].PCIL132_StopMotor();
        return false;
    }

    bFlag=GetOutArmToUnLoaderPosition(bCanPick2ICAtOnceTime, iOutPutTray, iXPos, iYPos, iRow, iCol, RealMove);

    if(bCanPick2ICAtOnceTime==false)
        iMovePitchX=AutoCalculateOutArmXClosePitch(iWhichAuto, bOutArmXOverLimit);                                      //Steven 20240501 : 修正超出極限後, 要重新算一次X-Pitch

    if(USE_OUT_Y_IS_AUTO_PITCH==true)                                           //Steven 20190520 : Fixed Y-Pitch position error  //JerryYang 20251218 : IN/OUT ARM支援不同模組
    {
        if(iOutArmPlaceOrder==0)
            iYPos=iYPos-iMovePitchY;
    }
    else
    {
        if(iOutArmPlaceOrder!=0)
            iYPos+=Prod.iOutARM_Y_PITCH;
    }

    if(iYPos<(MOT[MOutArmY].Motor->PSoftLimitN+10))                             //JerryYang 20180221 (Steven) : fix y-pitch 最小25mm會有行程的問題, y-pitch要縮到最小
    {
        if(USE_OUT_Y_IS_AUTO_PITCH==true)                                       //JerryYang 20251218 : IN/OUT ARM支援不同模組
        {
            if(iOutArmPlaceOrder==0)
            {
                iYPos=iYPos+iMovePitchY;
                iMovePitchY=IN_OUT_ARM_Y_PITCH_MIN;
                iYPos=iYPos-iMovePitchY;
                iYVariable=GetOutArmPitchY_9045(iMovePitchY);
            }
        }
    }

    if(RealMove==false)
        return bFlag;

    if(iXPos<(MOT[MOutArmX].Motor->PSoftLimitN+10) ||                           //jou 2010-02-01 fix 3 吸嘴超過 軟體極限時 , 設成 has null ic
       iXPos>(MOT[MOutArmX].Motor->PSoftLimitP-10))
    {
        if(MOT[iMMAuto[iOutPutTray]].Tray.Data[iCol][iRow]==NULL_IC)            //ChungHung 20111215 嘗試將Fix3放滿
        {
            MOT[iMMAuto[iOutPutTray]].SetTraySingleData(iCol, iRow, HAS_NULL_IC);
            ReserveEmptyPoint(iMMAuto[iOutPutTray], iCol, iRow);
        }

        if(iHWFix_BinBox==1 && iWhichAuto==eBulkBox)
        {
            ShowErrorMessage("WAR16318", 0, MMSystem, 0, "623939");             //kevin 20210401 超出極限秀ALARM 避免HANG UP
        }
        return false;
    }

    if(IniConfig.bFix3PutAllFullIC)                                             //ChungHung 20111215 嘗試將Fix3放滿
    {
        if(iWhichAuto==iFixRight)
            ReversionEmptyPoint();
    }

    if(IniConfig.bE34InOutArmPitchZOffsetSameOne==true)
    {
        if(iWhichAuto>=0 && iWhichAuto<=iAutoRight)
            iOffsetPos=OutOfsAuto1;
        else
            iOffsetPos=OutOfsFix1;
    }
    else
    {
        iOffsetPos=OutOfsAuto1+iOutPutTray;
    }

    for(int i=0; i<X_PITCH_COUNT; i++)
        iXVariable[i]=GetOutArmPitchX_9045(iMovePitchX, i, iOffsetPos);
    iYVariable=GetOutArmPitchY_9045(iMovePitchY, iOffsetPos);

    if(MOT[iMMAuto[iOutPutTray]].fHasTray==false ||
       MOT[iMMAuto[iOutPutTray]].FullIC())
        bNeedDown=false;
    GetOutArmZDownPos_9045(iWhichAuto, bNeedDown, bZFlag, iZPos);

    //AI(W906-HT9050-RULE10) 20261004: EastSun rule 10 -- on HT9050 the XY move to the Auto1 tray goes Y first (with C_OutArmSmallY On), then X
    //  (Motor/mymotor.cpp PCIL112_OutArmXYMove case 500). The flag is only up during this one call. HT9050 = the 9050GPIB flag
    //  (W906_Ht9050OrgHome answers), as acarry.cpp / uhome.cpp. Other machines / other trays: golden.
    { extern int g_W906OutArmRule10; g_W906OutArmRule10=(W906_Ht9050OrgHome(MTrayX)!=-2 && iOutPutTray==eAuto1) ? 1 : 0; }   //AI(W906-HT9050-RULE10-2) 20261004: C_OutArmSmallY On with Y (EastSun)
    bool w906Done=OutArmContinuousMove_9045(iXPos, iYPos, iXVariable, iYVariable, bZFlag, iZPos, true);
    { extern int g_W906OutArmRule10; g_W906OutArmRule10=0; }
    if(w906Done)
    {
        return true;
    }
    return false;
}
// ---- golden aoutarm9045.cpp:2451-2491 SearchUnLoadTrayUpDown_9045 ----
bool SearchUnLoadTrayUpDown_9045(bool &bCanPick2ICAtOnceTime, int &iRow, int &iCol)
{
    if(AUTO3_IS_MAGAZINE==1             &&                                      //JerryYang 20221215 : Magazine把fix區當buffer區功能
       TestIF_File.iMagFixTrayType==1   &&
       iWhichAuto>=iMagMin)
    {
        AdjustOutArmClosePitchCondition(iWhichBuff, bCanPick2ICAtOnceTime);
    }
    else
    {
        AdjustOutArmClosePitchCondition(iWhichAuto, bCanPick2ICAtOnceTime);
    }

    if(bOutArmXOverLimit)                                                       //Steven 20171206 (Wei) : In ATK, 修正Out Arm同時放料但是超出軟體極限縮Pitch導致疊料的問題
        bCanPick2ICAtOnceTime=false;

    if(bCanPick2ICAtOnceTime==true)
    {
        int ct=Find_OutArm_PickerMaxUseCountOnTime(OutArmSuck.iPickCol, iRow, iCol);
        if(ct<=1)
        {
            bCanPick2ICAtOnceTime=false;
        }
        else if(iWhichAuto==iAutoRight)
        {
            if(DoMoveOutArmXYToPlace_9045(bCanPick2ICAtOnceTime, iRow, iCol, false)==true)
                return true;
            bCanPick2ICAtOnceTime=false;
        }
        else
        {
            SearchBinBoxUpDown(bCanPick2ICAtOnceTime, iRow, iCol);              //Steven 20230601 : 重新整理Bin Box
            return true;
        }
    }
//    else                                                                      //JerryYang 20250917 : Mark掉
//    {
        Find_OutArm_Single(OutArmSuck.iPickCol, iRow, iCol);
//    }
    return true;
}
// ---- golden aoutarm9045.cpp:2493-2523 SetOutArm_9045 ----
bool SetOutArm_9045()
{
    int iCol=0, iRow=0;
    bool bCanPick2ICAtOnceTime;
    if(IsTrayArmMoveAvoidOutArmCrash())
    {
        MOT[MOutArmX].PCIL132_StopMotor();
        MOT[MOutArmY].PCIL132_StopMotor();
        return false;
    }

    if(SearchUnLoadTrayUpDown_9045(bCanPick2ICAtOnceTime, iRow, iCol)==false)
        return false;

    bOverTray=false;
    if(iCol<0 || iRow<0)                                                        //jou 2012-09-04 修正極限無法到達時,回填Null_IC會造成找不到地方放,會放在 x=-1,y=-1的位置上
    {
        bOverTray=true;
        return false;
    }

    if(TRAY_ARM_MODE==eAboveCoveyor &&
       MOT[MTrayX].ReadPos()>Prod.iXTrayColor)
    {
        MOT[MOutArmX].PCIL132_StopMotor();
        MOT[MOutArmY].PCIL132_StopMotor();
        return false;
    }

    return DoMoveOutArmXYToPlace_9045(bCanPick2ICAtOnceTime, iRow, iCol, true);
}
#include "Interface/TesterTCP_OSReport.h"   // TesterTCP_PlaceOSTestResultToTray (:4744)   //AI(W906-G023) 20261001 (St02-E): on the blank line, nothing below moves
// ===========================================================================
//  AI(W906-ARM3) 20260927: golden DoOutArmPlaceToAuto（aoutarm9045.cpp:2525-3075）逐行照翻（NB2 R105 ⑥）：
//  出料臂把 IC 放進 Auto／Fix 盤的整段動作 —— Z 下降、破真空、檢查、寫料況、各種客戶／選配分支。
//  以前是 static 樁回 true（本檔 :220）⇒ DoOutArmPlaceToAuto_9045（:1541）呼叫它時「放料」立刻成功，沒有 Z 動作、沒有破真空、沒有警報。
//  移植樹沒有的相依才閘（GATE(W906-ARM3)），都在客戶／選配分支。
// ===========================================================================
#include "cSocket.h"            // LotSummary（TLotSummary::AddCount，golden :2590）—— 同 asortarm.cpp:521
#include "Public/MyStringList.h" // TMyStringList（sl2DMappingLog／slHanaTrayMap，cmydef.h:118／:140 只有前向宣告）
#include "BarCode/BarCode_Bottom2DID.h" // list2DByLot／map2DList（golden fBarCode 的成員，移植樹是全域）—— 同 aoutarm.cpp:501 A5
#include "mymessbox_shim.h"             // MyMessageBox (golden mymessbox.h) -- DoOutArmPlaceToAuto, golden :3029-3030  //AI(W906-POOL2) 20261008 (Ifor01)
bool DoOutArmPlaceToAuto(int iToWhichAuto)                                      //Steven 20110216
{
    static bool bSuckDuplicateErr[MAX_ARM_Row][MAX_ARM_Col]={{false, false, false, false}, {false, false, false, false}};                                       //Steven 20091218 : Avoid duplicate message
    int Motor=iMMAuto[iToWhichAuto];
    int iXPos=0, iYPos=0;
    int iXPitchPos=0, iXPitch2Pos=0, iYPitchPos=0;                              //JerryYang 20210119 : add x,y pitch位置
    int iTrayRow=0, iTrayCol=0, iTemp=0;
    bool flag=true;
    bool bHasDuplicateErr=false;
    bool bHasErr=false;
    AnsiString ErrPart=" ", str="";
    AnsiString asCheck="";                                                      //2012.06.01 Q_Q
    AnsiString asOCRCode="";                                                    //wei 20161005 紀錄OCR Code
    AnsiString asInfo="";

    iOutArmYStep=ChangeToFloatNonPcnt((double)(AutoCalculateOutArmYClosePitch(iToWhichAuto)), (double)(AutoForm[iWhichAuto]->YPitch));                          //ChungHung 20131231 alter AutoYPitch
    if((USE_OUT_Y_IS_AUTO_PITCH==false && TestIF.bSearchLastMode==false) ||     //JerryYang 20251218 : IN/OUT ARM支援不同模組
       UnloaderTrayYDirForArmYPitch(iToWhichAuto)==false ||                     //ChungHung 20131231 alter AutoYPitch
       iOutArmPlaceOrder==1)
    {
        iOutArmYStep=0;                                                         //kevin 20171120 (wei) 判斷LOAD TRAY Y PITCH是否是 ARM Y PITCH可以一次8吸嘴吸IC
    }

    if(AUTO3_IS_MAGAZINE==1             &&                                      //JerryYang 20221215 : Magazine把fix區當buffer區功能
       TestIF_File.iMagFixTrayType==1   &&
       iToWhichAuto>=iMagAtAuto)
    {
        Motor=iMMAuto[iWhichBuff];
    }
    else
    {
        Motor=iMMAuto[iToWhichAuto];
    }

    for(int i=0; i<OutArmSuck.iPickRow; i++)
    {
        for(int j=0; j<OutArmSuck.iPickCol; j++)
        {
            if(bOutArmSuckActive[i][j])
            {
                if(OutArmSuck.Item[i][j]==HAS_NULL_IC)                          //Steven 20170113 : 沒IC的地方不檢查吹氣
                {
                    OutArmSuck.SetItemData(i, j, NULL_IC);
                }
                else
                {
                    flag=OutArmSuck.Suck[i][j].Destroy();
                    if(flag)
                    {
                        iTrayCol=iOutArmXPosition[i][j];
                        iTrayRow=iOutArmYPosition+iOutArmYStep*i;
                        MOT[Motor].Tray.iWhichSite [iTrayCol][iTrayRow]=OutArmSuck.iWhichSite[i][j];
                        MOT[Motor].Tray.iWhichIndex[iTrayCol][iTrayRow]=OutArmSuck.iWhichIndex[i][j];
                        MOT[Motor].Tray.iBinCode   [iTrayCol][iTrayRow]=OutArmSuck.Item[i][j];
                        MOT[Motor].Tray.iWhichAuto [iTrayCol][iTrayRow]=OutArmSuck.iWhichAuto[i][j];                    //JerryYang 20221215 : Magazine把fix區當buffer區功能
                        MOT[Motor].Tray.iBinData   [iTrayCol][iTrayRow]=OutArmSuck.iBinData[i][j];
                        MOT[Motor].Tray.iAOIResult [iTrayCol][iTrayRow]=OutArmSuck.iAOIResult[i][j];                    //Sam 20240325 : 新增 DamageTrayMapping 功能

#if 1 // GATE(W906-ARM3) golden :2583-2583 —— fTesterTCP（golden TfTesterTCP 表單的 Open/Short 結果放盤）移植樹沒有 TfTesterTCP 類別與 fTesterTCP 全域 —— 同 aoutarm.cpp GATE G11   //AI(W906-G023) 20261001 (St02-E): LIFTED -- body = TesterTCP_PlaceOSTestResultToTray (Interface/TesterTCP_OSReport.cpp, ht9045_sm; golden Interface/TesterTCP.cpp:699-739)
                        TesterTCP_PlaceOSTestResultToTray(i, j, iTrayRow, iTrayCol, OutArmSuck.iWhichAuto[i][j]);     //Steven 20250515 : 整合Open Short測試報表   //AI(W906-G023) 20261001 (St02-E): golden fTesterTCP->PlaceOSTestResultToTray -- free function in ht9045_sm (forms -> sm back-edge), like TesterTCP_CopyOSTestResult at atester_ProcessCount.cpp:2108
#endif // GATE(W906-ARM3)
                        InspectOutArmPosition(iToWhichAuto, OutArmSuck.Suck[i][j].iMyRow, OutArmSuck.Suck[i][j].iMyCol, iTrayRow, iTrayCol, iOutPlaceToAuto);

                        if(OutArmSuck.Item[i][j]>=START_TEST)
                        {
                            if(OutArmSuck.iWhichSite[i][j]>=0 &&
                               OutArmSuck.iWhichSite[i][j]<32)                  //Steven 20240320 : 避免溢位
                                LotSummary.AddCount(OutArmSuck.iWhichSite[i][j], OutArmSuck.Item[i][j]-TEST_PASS);

#if 0 // GATE(W906-ARM3) golden :2592-2595 —— fSCKART->iInfo_MultiLotCnt（golden SCK_ART.h:323）移植樹的 TfSCKART 沒有這個成員 —— 同 asortarm.cpp GATE(5)／aTester_Front.cpp:2427；單批次時 golden 也不進這個 if
                            if(fSCKART->iInfo_MultiLotCnt>1)
                            {
                                LotSummary.AddByLotCount(OutArmSuck.iWhichSite[i][j], OutArmSuck.Item[i][j]-TEST_PASS, OutArmSuck.cDeviceInf[i][j]);
                            }
#endif // GATE(W906-ARM3)
                        }

                        if(LastSet.iTester==_2D_SORT &&
                           CosFunction.bSortingBy2DList &&
                           TestIF_File.bSortingBy2DIDList)                      //JerryYang 20230814 : 有開2D SORT才存
                        {
                            if(CUSTOMER_CODE==CC_QUALCOMM)                      //RogerYang 20260121 : Add for RF360 2DID sorting 塞bin結果
                            {
#if 0 // GATE(W906-ARM3) golden :2604-2604 —— 型別轉接：vclcompat 的 Strings[] 回 StringsProxy（有 operator AnsiString、沒有 .ToIntDef）；#else 只把主詞包成 AnsiString(...)，字面與參數不變 —— 同 asortarm.cpp GATE(14)
                                sl2DMappingLog->MyInsert2DMappingToFile(OutArmSuck.cDeviceInf[i][j], OutArmSuck.PordRec[i][j].asBuffer->Strings[eTestCate].ToIntDef(-1));
#else // GATE(W906-ARM3) 型別轉接版（只有主詞包成 AnsiString(...)）
                                sl2DMappingLog->MyInsert2DMappingToFile(OutArmSuck.cDeviceInf[i][j], AnsiString(OutArmSuck.PordRec[i][j].asBuffer->Strings[eTestCate]).ToIntDef(-1));
#endif // GATE(W906-ARM3)
                            }
                            else
                            {
                                sl2DMappingLog->MyInsert2DMappingToFile(OutArmSuck.cDeviceInf[i][j], 1);                //JerryYang 20230322 : add 2D mapping result
                            }
                        }

                        if(OutArmSuck.Item[i][j]>=START_TEST)                   //Steven 20171031 (wei) : Add TTMyTray滑鼠滑過去顯示Hint
                        {
                            if(TestIF_File.bEnableBarCode)
                            {
                                asCheck.sprintf("%s", OutArmSuck.cReDeviceInf[i][j]);
                                if(asCheck=="0" || asCheck=="")  //AI(W906-NUMCMP) 20260927: golden compares with AnsiString(0) = "0" in BCB6 (NB2 R89, bcc32 5.6.4 measured); vclcompat would take 0 as a null const char* = "" -- written as "0" (census tools/nb2_assist/numcmp_census.py flagged this after ARM3 15df16ab)
                                    asInfo.sprintf("BIN: %d, 2DID: ERROR %s", OutArmSuck.Item[i][j]-TEST_PASS, OutArmSuck.cDeviceInf[i][j]);
                                else
                                    asInfo.sprintf("BIN: %d, 2DID: %s", OutArmSuck.Item[i][j]-TEST_PASS, OutArmSuck.cDeviceInf[i][j]);
                            }
                            else
                            {
                                asInfo.sprintf("BIN: %d", OutArmSuck.Item[i][j]-TEST_PASS);
                            }
                        }
                        else
                        {
                            if(TestIF_File.bEnableBarCode)
                            {
                                asCheck.sprintf("%s", OutArmSuck.cReDeviceInf[i][j]);
                                if(asCheck=="0" || asCheck=="")  //AI(W906-NUMCMP) 20260927: golden compares with AnsiString(0) = "0" in BCB6 (NB2 R89, bcc32 5.6.4 measured); vclcompat would take 0 as a null const char* = "" -- written as "0" (census tools/nb2_assist/numcmp_census.py flagged this after ARM3 15df16ab)
                                    asInfo.sprintf("BIN: %d, 2DID: ERROR %s", OutArmSuck.Item[i][j]-TEST_PASS, OutArmSuck.cDeviceInf[i][j]);
                                else
                                    asInfo.sprintf("BIN: %d, 2DID: %s", OutArmSuck.Item[i][j], OutArmSuck.cDeviceInf[i][j]);
                            }
                            else
                            {
                                asInfo.sprintf("BIN: %d", OutArmSuck.Item[i][j]);
                            }
                        }

                        if((OutArmSuck.Item[i][j]-TEST_PASS)>iTestBinCount)     //error bin
                            iByBinTotal[iTestBinCount]++;                       //kevin 20180705 (wei) bin 數量 Bin total[0]
                        else
                            iByBinTotal[OutArmSuck.Item[i][j]-TEST_PASS]++;     //kevin 20180705 (wei) bin 數量 Bin total[0]

                        if(CosFunction.bTrayOCR && TestIF.bOcrFunction)         //wei 20161005 紀錄OCR Code
                            asOCRCode=OutArmSuck.cDeviceInf[i][j];
                        else
                            asOCRCode="na";

//                        if(IniConfig.bN14_9_ULQtyReport==true)
//                        {
//                            if(iToWhichAuto>=iFixRight)
//                                LastSet.iN14_9_TrayICCnt[iToWhichAuto-3]++;     //Sam 20190801 : Bug Fix
//                            else
//                                LastSet.iN14_9_TrayICCnt[iToWhichAuto]++;       //Sam 20190801 : Bug Fix
//                        }

                        iXPos=MOT[MOutArmX].ReadPos();                          //Frank 20160727 add iXPos iYPos
                        iYPos=MOT[MOutArmY].ReadPos();                          //Frank 20160727 add iXPos iYPos
                        iXPitchPos=MOT[MOutArmPitch].ReadPos();                 //JerryYang 20210119 : add x,y pitch位置
                        if(USE_OUT_ARM_Y_PITCH==iXYPitchVariable ||             //JerryYang 20251218 : IN/OUT ARM支援不同模組
                           USE_OUT_ARM_Y_PITCH==iXYPitchIn_Bb_Out_Bc)           //Ztex 2024.02.24 Add HT-1132
                        {
                            iXPitch2Pos=MOT[MOutArmPitchX2].ReadPos();
                            iYPitchPos=MOT[MOutArmPitchY].ReadPos();
                        }
                        else if(USE_OUT_ARM_Y_PITCH==iXYPitch16Picker ||        //QQ  //JerryYang 20251218 : IN/OUT ARM支援不同模組
                                USE_OUT_ARM_Y_PITCH==iXYPitch16Bd_Be)           //Ztex 2023.12.06 Add HT-1032
                        {
                            iXPitch2Pos=MOT[MOutArmPitchX2].ReadPos();
                            iYPitchPos =MOT[MOutArmPitchY].ReadPos();
                        }

                        bOutArmCheckDestroyACT[i][j]=true;                      //kevin 20140220 回吸檢測 jou 981130 確認device確實destroy完成

                        if(TestIF_File.bEnableBarCode)
                        {
                            asCheck.sprintf("%s", OutArmSuck.cReDeviceInf[i][j]);
                            if(asCheck=="0" || asCheck=="")  //AI(W906-NUMCMP) 20260927: golden compares with AnsiString(0) = "0" in BCB6 (NB2 R89, bcc32 5.6.4 measured); vclcompat would take 0 as a null const char* = "" -- written as "0" (census tools/nb2_assist/numcmp_census.py flagged this after ARM3 15df16ab)
                                MOT[Motor].SetTrayBinData(iTrayCol, iTrayRow, HAS_IC, asInfo);                          //ChungHung 20131231 alter AutoYPitch
                            else
                                MOT[Motor].SetTrayBinData(iTrayCol, iTrayRow, HAS_BARCODEERROR_IC, asInfo);             //ChungHung 20131231 alter AutoYPitch

                            if(LastSet.iRealDummy==REALLY  &&                   //jou 20190930 : Barcode Tray record file
                               TestIF_File.bEnableBarCode==true)                //jou 20191008 : Barcode duplicate file by out arm
                            {
                                if(CosFunction.bBarcodeTrayRecFile==true)
                                    MOT[Motor].Tray.cDeviceInf[iTrayCol][iTrayRow]=OutArmSuck.cDeviceInf[i][j];

                                if(CosFunction.bBarcodeDuplicateFileByOutArm==true && TestIF_File.bCheckCodeByLot==true &&
                                   OutArmSuck.Item[i][j]!=HAS_NULL_IC && OutArmSuck.Item[i][j]!=NULL_IC &&
                                   OutArmSuck.cDeviceInf[i][j]!=asBarCodeErrorSend && OutArmSuck.cDeviceInf[i][j]!="")
                                {
#if 0 // GATE(W906-ARM3) golden :2697-2700 —— 轉接：golden fBarCode->list2DByLot／map2DList 是 TfBarCode 成員，移植樹的同一個物件是全域（BarCode/BarCode_Bottom2DID.h:144、BarCode_Shuttle2_Scan.h:141）；#else 只拿掉 `fBarCode->` —— 同 aoutarm.cpp ADAPTATION A5
                                    fBarCode->list2DByLot->Clear();
                                    fBarCode->map2DList[OutArmSuck.cDeviceInf[i][j]]=i;
                                    fBarCode->list2DByLot->Add(OutArmSuck.cDeviceInf[i][j]);
                                    WriteDataToFile(asBarCodeLot, fBarCode->list2DByLot->Text);                         //Steven 20160429 : 開程式要把2D List讀回來
#else // GATE(W906-ARM3) 型別轉接版（只有主詞包成 AnsiString(...)）
                                    list2DByLot->Clear();
                                    map2DList[OutArmSuck.cDeviceInf[i][j]]=i;
                                    list2DByLot->Add(OutArmSuck.cDeviceInf[i][j]);
                                    WriteDataToFile(asBarCodeLot, list2DByLot->Text);                         //Steven 20160429 : 開程式要把2D List讀回來
#endif // GATE(W906-ARM3)
                                }
                            }
                        }
                        else
                        {
                            if(IniConfig.bBinBox==true &&                       //jou 2012-12-11 support Bin Box
                               iToWhichAuto==iBinBoxAtFix &&
                               AutoForm[iBinBoxAtFix]->iTrayType==iBinBoxType)
                            {
                                LastSet.iBinBoxCount++;
                            }
                            else
                            {
                                MOT[Motor].SetTrayBinData(iTrayCol, iTrayRow, HAS_IC, asInfo);                          //ChungHung 20131231 alter AutoYPitch
                            }
                        }

                        bOutArmSuckActive[i][j]=false;

                        if(OutArmSuck.iBinData[i][j]<0 ||
                           OutArmSuck.iBinData[i][j]>=iTestBinCount)            //jou 20170606 (wei) : 修正iBinData陣列被填入-1
                        {
                            iSV_ErrBinCnt++;
                        }
                        else
                        {
                            iSVByBinCount[OutArmSuck.iBinData[i][j]]++;
                        }

                        if(AUTO3_IS_MAGAZINE==1             &&                  //JerryYang 20221215 : Magazine把fix區當buffer區功能
                           TestIF_File.iMagFixTrayType==1   &&
                           iToWhichAuto>=iMagAtAuto)
                        {
                        }
                        else
                        {
                            if(TrayForm.bEnableAMR)                            //==> Eastsun 20260512 : Phase4-F6T1 KYEC AMR Magazine Count (a-side aoutarm.cpp L2959-L3000)
                            {
                                iMagBin = OutArmSuck.iWhichAuto[i][j];
                                if(iMagBin>=eMag1 && eMag14>=iMagBin)
                                {
                                    int AMRMagazineIndex = iMagBin-eMag1;
                                    for(int k=AMRMagazineIndex; k<(14-AMRMagazineIndex); k++) //Eastsun 20260116 : maga full
                                    {
                                        if(!MOT[MMMagazineTary1+k].Tray.FullIC() && k==AMRMagazineIndex)
                                        {
                                            TestIF_File.iAMRMagzineDeviceCount[k]++;
                                            break;
                                        }
                                        else if(BinSelect[iTestRunMode].bMagazineLink[k]==true && !MOT[MMMagazineTary1+k].Tray.FullIC())
                                        {
                                            TestIF_File.iAMRMagzineDeviceCount[k]++;
                                            break;
                                        }
                                        else if(k!=AMRMagazineIndex && BinSelect[iTestRunMode].bMagazineLink[k]==false)
                                        {
                                            break;
                                        }
                                    }
                                }
                                else
                                {
                                    TestIF_File.iAMRDeviceCount[OutArmSuck.iWhichAuto[i][j]+3]++;
                                }
                            }

                            for(int k=0; k<4; k++)
                            {
                                LastSet.BinCT[k][iTo3Unload[OutArmSuck.iWhichAuto[i][j]]]++;

                                if(IniConfig.bVTESTFunction==true &&            //marvin 20200424 (Kirin) Added always record report by time.
                                   IniConfig.bCheckFile==true)
                                {
                                    #ifdef SOFT_SIMULTE
                                    if(OutArmSuck.iWhichAuto[i][j]<=iFixRightHalf)
                                        LastSet.iBinCTForAlways[k][OutArmSuck.iWhichAuto[i][j]]++;
                                    #else
                                    if(LastSet.iTester==ON_LINE &&
                                       OutArmSuck.iWhichAuto[i][j]<=iFixRightHalf)
                                        LastSet.iBinCTForAlways[k][OutArmSuck.iWhichAuto[i][j]]++;
                                    #endif
                                }

                                if(CosFunction.bSpecailLowYeild &&
                                   Prod.bFailAlarmLowYieldSpecial)              //Sam 20210505 : PTI 要求的兩段 Low Yeild
                                    LastSet.BinCT_PTI[k][OutArmSuck.iWhichAuto[i][j]]++;

                                if(CUSTOMER_CODE==CC_TERAPOWER &&
                                   Prod.bD22SupportMultiDoubleContact)          //Sam 20180612 : 有開啟 D22 Double Contact Contact， 以第一次的測試結果來做 ProcessCount
                                {
                                    if(OutArmSuck.iBinDataBackUp[i][j]<0 ||
                                       OutArmSuck.iBinDataBackUp[i][j]>iTestBinCount)                                   //Steven 20260316 : 修正 bin code 超出範圍問題
                                        LastSet.iBinData32[k][iTestBinCount]++;
                                    else
                                        LastSet.iBinData32[k][OutArmSuck.iBinDataBackUp[i][j]]++;
                                }
                                else
                                {
                                    if(OutArmSuck.iBinData[i][j]<0)             //jou 20170606 (wei) : 修正iBinData陣列被填入-1
                                        LastSet.iBinData32[k][iTestBinCount]++;
                                    else
                                        LastSet.iBinData32[k][OutArmSuck.iBinData[i][j]]++;
                                }

                                if((USE_AUTO_RETEST==eartInstall && (bAutoReTest_ART || IniConfig.bA10_AutoReTest)) || CosFunction.bUseARTSortCount)            //kevin 20150615   //Ifor 20170315 (wei) add 新增使用ART Sort Count 計數功能
                                {
                                    if(CUSTOMER_CODE==CC_TERAPOWER &&
                                       Prod.bD22SupportMultiDoubleContact)      //Sam 20180612 : 有開啟 D22 Double Contact Contact， 以第一次的測試結果來做 ProcessCount
                                    {
                                        if(OutArmSuck.iBinDataBackUp[i][j]<0)
                                            LastSet.iBinData32_ART[k][iTestBinCount]++;
                                        else
                                            LastSet.iBinData32_ART[k][OutArmSuck.iBinDataBackUp[i][j]]++;
                                    }
                                    else
                                    {
                                        LastSet.BinCT_ART[k][iTo3Unload[OutArmSuck.iWhichAuto[i][j]]]++;
                                        if(OutArmSuck.iBinData[i][j]<0)         //jou 20170606 (wei) : 修正iBinData陣列被填入-1
                                            LastSet.iBinData32_ART[k][iTestBinCount]++;
                                        else
                                            LastSet.iBinData32_ART[k][OutArmSuck.iBinData[i][j]]++;
                                    }
                                }
                            }

                            if(bCanRunSCKART==true)                             //Steven 20161201 (wei) : For SCK 93K ART
                            {
                                LastSet.lSCKARTBinCT[OutArmSuck.iWhichAuto[i][j]]++;                                    //Steven 20170113 (wei) : For count unload count
                                if(TestIF_File.iSCKART_SortMode==1)
                                {
                                    if(Prod.bCateRTo6Tray[OutArmSuck.iWhichAuto[i][j]])
                                        LastSet.iSCKART_RTUnitCount++;
                                    else if(Prod.bART6Tray[OutArmSuck.iWhichAuto[i][j]])                                //Steven 20190605 : 可能沒選CateR
                                        LastSet.iSCKART_RTUnitCount++;
                                }
                                else
                                {
                                    if(Prod.bART6Tray[OutArmSuck.iWhichAuto[i][j]])
                                        LastSet.iSCKART_RTUnitCount++;
                                }
                            }
                        }

                        fSortCT->ShowSortIC();

                        if(AUTO3_IS_MAGAZINE==1             &&                  //JerryYang 20221215 : Magazine把fix區當buffer區功能
                           TestIF_File.iMagFixTrayType==1   &&
                           iToWhichAuto>=iMagAtAuto)
                        {
                            MOT[Motor].Tray.PordRec[iTrayCol][iTrayRow]->asBuffer->CommaText=OutArmSuck.PordRec[i][j].asBuffer->CommaText;                      //把production log資料過到fix buffer區
                        }
                        else
                        {
                            if(AUTO3_IS_MAGAZINE==1 &&
                               iToWhichAuto==iMagAtAuto)                        //JerryYang 20230509 : Magazine production log
                            {
                                OutArmSuck.PordRec[i][j].AddUnloadRecord(OutArmSuck.iWhichAuto[i][j],
                                                                         LastSet.iUnloaderTrayCount_ART[OutArmSuck.iWhichAuto[i][j]],
                                                                         iTrayRow, iTrayCol,                            //Frank 20160505 add
                                                                         iXPos, iYPos,                                  //Frank 20160727 add iXPos iYPos
                                                                         asOCRCode,                                     //wei 20161005 紀錄OCR Code
                                                                         iXPitchPos, iXPitch2Pos, iYPitchPos,           //JerryYang 20210119 : add x,y pitch位置
                                                                         "");   //Steven 20220120 : Amlogic需要收SBIN
                            }
                            else
                            {
                                OutArmSuck.PordRec[i][j].AddUnloadRecord(iToWhichAuto,
                                                                         LastSet.iUnloaderTrayCount_ART[iToWhichAuto],  //Frank 20160505 add
                                                                         iTrayRow, iTrayCol,                            //Frank 20160505 add
                                                                         iXPos, iYPos,                                  //Frank 20160727 add iXPos iYPos
                                                                         asOCRCode,                                     //wei 20161005 紀錄OCR Code
                                                                         iXPitchPos, iXPitch2Pos, iYPitchPos,           //JerryYang 20210119 : add x,y pitch位置
                                                                         MOT[MMAuto1+iToWhichAuto].Tray.cTrayID);       //Steven 20220120 : Amlogic需要收SBIN
                            }
                            iWhichMag=-1;                                       //JerryYang 20250920 : fixed for Multi bin

                            if(USE_TRAY_MAPPING==1 && USE_KEYENCE_EMPTY==3 && TestIF_File.bEnableTrayID2==true)         //JerryYang 20250113 : Color上方TrayID
                            {
                                OutArmPordRec[i][j].asBuffer->CommaText=OutArmSuck.PordRec[i][j].asBuffer->CommaText;

                                iTemp=OutArmSuck.Item[i][j]-TEST_PASS;

                                if(MOT[MMAuto1+iToWhichAuto].Tray.cTrayID=="" && iToWhichAuto<3 && Prod.bIsPassBin[iTemp]==1)
                                {
                                    ShowMyMessage("Tray ID is NULL!!!");
                                }

                                if(Prod.bIsPassBin[iTemp]==1 && iTemp<iTestBinCount)
                                {
                                    OutArmSuck.PordRec[i][j].SaveJHT_Record();  //Frank 20160505 add
                                }
                            }

                            if(fMain->hanaART->IsHanaArtAvailable())            //Steven 20250414 : HANA ART Function
                            {
                                str.sprintf("%s,%s,%s - (%d,%d)", OutArmSuck.PordRec[i][j].asBuffer->Strings[eOrderTest],
                                                                  OutArmSuck.PordRec[i][j].asBuffer->Strings[eSiteNO],
                                                                  OutArmSuck.PordRec[i][j].asBuffer->Strings[eTestCate],
                                                                  iTrayRow+1, iTrayCol+1);

//AI(W906-LOGOBJ-W7) 20260927 (St02-E): GATE(W906-ARM3) opened -- slHanaTrayMap is defined now (cmydef.cpp:6151) and W906_CreateLogObjects news it (LogObjects.cpp); golden 906_0625_Steven aoutarm9045.cpp:2901.  The enclosing fMain->hanaART->IsHanaArtAvailable() (:4999) still returns false in V906 (forms/fMain.cpp:29)
                                slHanaTrayMap[iWhichAuto]->AddText(str);
//AI(W906-LOGOBJ-W7) 20260927 (St02-E): (the old #endif of GATE(W906-ARM3) golden :2901)
                            }

                            MOT[Motor].Tray.PordRec[iTrayCol][iTrayRow]->asBuffer->CommaText=OutArmSuck.PordRec[i][j].asBuffer->CommaText;                      //Steven 20221005 : Production Log減少記憶體使用量

                            if(OutArmSuck.PordRec[i][j].asBuffer->Strings[eTestCate]=="E")                              //JerryYang 20230322 : Lot summary要計算各類型ERR的數量
                            {
#if 0 // GATE(W906-ARM3) golden :2908-2920 —— 型別轉接：同上，StringsProxy 沒有 .Pos；#else 只把三個主詞包成 AnsiString(...)，字面、>0 門檻、|| 與分支順序不變；:2921-2924 的 else 留在閘外 —— 同 asortarm.cpp GATE(14)
                                if(OutArmSuck.PordRec[i][j].asBuffer->Strings[eErrorCode].Pos("Duplicate 2DID error")>0)
                                {
                                    iE2Count++;
                                }
                                else if(OutArmSuck.PordRec[i][j].asBuffer->Strings[eErrorCode].Pos("NonTestToRBin")>0 ||
                                        OutArmSuck.PordRec[i][j].asBuffer->Strings[eErrorCode].Pos("No2DCodeDevice")>0)
                                {
                                    iE1Count++;
                                }
                                else if(OutArmSuck.PordRec[i][j].asBuffer->Strings[eErrorCode].Pos("2DID is not in sorting list")>0)
                                {
                                    iE3Count++;
                                }
#else // GATE(W906-ARM3) 型別轉接版（只有主詞包成 AnsiString(...)）
                                if(AnsiString(OutArmSuck.PordRec[i][j].asBuffer->Strings[eErrorCode]).Pos("Duplicate 2DID error")>0)
                                {
                                    iE2Count++;
                                }
                                else if(AnsiString(OutArmSuck.PordRec[i][j].asBuffer->Strings[eErrorCode]).Pos("NonTestToRBin")>0 ||
                                        AnsiString(OutArmSuck.PordRec[i][j].asBuffer->Strings[eErrorCode]).Pos("No2DCodeDevice")>0)
                                {
                                    iE1Count++;
                                }
                                else if(AnsiString(OutArmSuck.PordRec[i][j].asBuffer->Strings[eErrorCode]).Pos("2DID is not in sorting list")>0)
                                {
                                    iE3Count++;
                                }
#endif // GATE(W906-ARM3)
                                else
                                {
                                    iENotDefinedCount++;
                                }
                            }

                            if(IniConfig.bOCRAndBinLog &&
                               TestIF.bOcrFunction &&
                               CosFunction.bTrayOCR)                            //KenHsieh 20230406 : 新增OCR Data + Bin Log功能
                            {
#if 0 // GATE(W906-ARM3) golden :2931-2931 —— TfOCR::SaveOCRAndBinData（OCR＋Bin log，選配 bOCRAndBinLog＆bOcrFunction＆bTrayOCR）移植樹的 TfOCR 沒有這個方法
                                fOCR->SaveOCRAndBinData(asOCRCode, OutArmSuck.PordRec[i][j].asBuffer->Strings[eTestCate], Prod.iIsPassT6[iToWhichAuto]);        //Steven 20240105 : Prod.bIsPass --> Prod.iIsPassT6
#endif // GATE(W906-ARM3)
                            }
                            OutArmSuck.PordRec[i][j].SaveRecord();              //Frank 20160505 add
                            if(IniConfig.bQAMode==true &&
                               LastSet.iRunStartMode==rsmQAMode &&
                               CosFunction.bQAModeUseUnloadCnt)                 //JerryYang 20221004 : Maxim版本QA mode
                            {
                                if((OutArmSuck.Item[i][j]-TEST_PASS)==1)
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
                        OutArmSuck.SetItemData(i, j, NULL_IC);

                        if(CosFunction.bTraySortCntFunc &&                      //Steven 20210608 : Tray指定數量退盤功能
                           iToWhichAuto<iAutoCnt &&
                           (LastSet.iRunStartMode==rsmContinuStart ||
                            LastSet.iRunStartMode==rsmContinuStart_ART))        //Steven 20251023 : fixed for sort mode
                        {
                            if(TrayForm.bTraySortCntFunc[iToWhichAuto]==true &&
                               LastSet.iTrayCntForSortFunc[iToWhichAuto]<6)
                            {
                                LastSet.iTrayICCntForSortFunc[iToWhichAuto]++;
                            }
                        }
                    }
                    else if(OutArmSuck.Suck[i][j].Error)
                    {
                        ErrPart+=OutArmSuck.Suck[i][j].sName;
                        bHasErr=true;
                    }
                    else
                    {
                        bSuckDuplicateErr[i][j]=false;
                    }
                }
            }

            if(bOutArmSuckActive[i][j])
                flag=false;
        }
    }

    if(bHasErr)
    {
        ShowErrorMessage("JAM0217", K_RETRY, MOutArmX, bHasDuplicateErr, ErrPart);                                      //jou 2015-03-27 JAM0203->JAM0217 Device drop error -> Vacuum sensor OFF error
        for(int i=0; i<OutArmSuck.iMaxRow; i++)
        {
            for(int j=0; j<OutArmSuck.iMaxCol; j++)
            {
                if(OutArmSuck.Suck[i][j].Error)
                {
                    bSuckDuplicateErr[i][j]=true;
                    iXPos=MOT[MOutArmX].ReadPos();
                    iYPos=MOT[MOutArmY].ReadPos();
                    OutArmSuck.PordRec[i][j].AddErrorRecordNoSave("JAM0217", true, iXPos, iYPos);                       //Steven 20160114 (jou) : Add Production Error Log
                }
                else
                {
                    bSuckDuplicateErr[i][j]=false;
                }
            }
        }
    }

    for(int i=0; i<OutArmSuck.iMaxRow; i++)
        for(int j=0; j<OutArmSuck.iMaxCol; j++)
            if(bOutArmSuckActive[i][j])
                return false;

    if(IniConfig.bI52_bAQLSortMode==true &&             //==> //Eastsun 20260520 整合//Ifor 20210326 add:KYEC 要求新增AQL Sortting Mode 到達設定數量退Tray
       bStartAQLSortMode==true &&
       iToWhichAuto==iAQLBin)
    {
        AnsiString str1, str2;
        int iNewEQCCount=LastSet.BinCT_ART[0][iAQLBin];
        if(iNewEQCCount>=iAQLCount)
        {
            if(iToWhichAuto>=3)
            {
                MOT[MManualTray1+(iAQLBin-3)].SetNullIcToHasNullIc();
            }
            else
            {
                MOT[MMAuto1+iToWhichAuto].SetNullIcToHasNullIc();
            }

            bStartAQLSortMode=false;
            iAQLBin=-1;
            iAQLCount=0;
            RecordProcess("AQL Sorting Finish");
#if 1 // was: #if 0 -- opened AI(W906-POOL2) 20261008 (Ifor01): GATE(W906-ARM3) MyMessageBox reason expired -- read through W906_FormShowing("MyMessageBox", ...) like acatchtray.cpp:6086/:6138 (member || the web host's page table, so the web message box counts); golden :3029-3030
            if(W906_FormShowing("MyMessageBox", MyMessageBox->Visible)==true)             //Ifor 20160901 Alarm 前若form有開啟先關閉
                MyMessageBox->Close();
#endif // GATE(W906-ARM3)
            iUnLoaderCount=8;                           //Ifor 20160829 必須不為0 Handler才不停機
            str1.sprintf("EQC Remain Quantity Finished (%dEA), Please Remove the tray",iNewEQCCount);
            str2.sprintf("EQC 數量預留完成 (%dEA), 請取下Tray盤",iNewEQCCount);
            RunInfo.iPreAlarmPosition=9;                //Ifor 20171024 (wei) : add Pre Alarm Position 1:LD 2:Auto1 3:Auto2 4:Auto3 5:Fix1 6:Fix2 7:Fix3 8:Empty 9:Color
            bAQL_Sorting_Finish=true;                   //Ifor 20231124 add:避免AQL Mode 誤報空穴問題
            ShowUnloaderTrayMessage(str1, str2);
        }
    }

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
           (LastSet.iRunStartMode==rsmContinuStart ||
            LastSet.iRunStartMode==rsmContinuStart_ART))                        //Steven 20251023 : fixed for sort mode
        {
            int iTrayCnt=LastSet.iTrayCntForSortFunc[iToWhichAuto];
            if(LastSet.iTrayICCntForSortFunc[iToWhichAuto]>=TrayForm.iTraySortCntFunc[iToWhichAuto][iTrayCnt])          //當目前IC大於設定值,就要退盤
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

#if 0 // GATE(W906-ARM3) golden :3072-3072 —— fSortCT->CheckTheYieldAfterPlaceAuto（golden cSortCT.cpp:1794-1810，良率警報 Alarm4Yield／Alarm5Yield）cSortCT.cpp 沒翻、TfSortCT 沒有這個方法 —— 同 asortarm.cpp GATE(11)
    fSortCT->CheckTheYieldAfterPlaceAuto();                                     //Sam 20230306 : 修正 Initial Start 誤報警問題
#endif // GATE(W906-ARM3)

    return true;
}

// ===========================================================================
//  AI(W906-ARM4) 20260927: golden Fix3 盤滿與 QA 模式出料計數（NB2 R103「出料臂核心流程」那一列）逐行照翻：
//  InitialFix3CanFullTask（:749-752）、UseFix3Cylinder（:794-1186，Fix3 盤滿時用氣缸把盤推走／換盤的狀態機）、Check_QA_ModeUnloadCount（:3766-3798）。
//  以前：UseFix3Cylinder 一律回 true（「氣缸已到位」）、InitialFix3CanFullTask 什麼都不做、Check_QA_ModeUnloadCount 一律回 false；
//  DoFix3FullTray 是 static 樁回 true，遮住 SortingBinTray.cpp 的真本體 ⇒ Fix3 盤滿的整段流程被跳過（cStateRecord 的 Fix3CanFullTask 永遠不動）。
//  移植樹沒有的相依才閘（GATE(W906-ARM4)）。
// ===========================================================================
// golden 經 uhome.h 取得；本 TU 沒有 include，只補宣告（簽名照 acatchtray_shims.h:423-424；本體 uhome.cpp 檔尾，AI(W906-SMHOME)）。
void InitProcessSingleMotorTask(int iMot);
bool ProcessSingleMotorHome(int iMot);
// ---- golden aoutarm9045.cpp:749-752 InitialFix3CanFullTask ----
void InitialFix3CanFullTask()                                                   //ChungHung 20140313 add Fix3 can Full Tray
{
    iFix3CanFullTask=1;
}
// ---- golden aoutarm9045.cpp:794-1186 UseFix3Cylinder ----
bool UseFix3Cylinder(int iWhichFix)                                             //ChungHung 20140313 add Fix3 can Full Tray
{
    bool bResult=false;
    static int iCount=0;
    static bool bDuplicate=false;
    bool bByPass=false;
    int &Task=iFix3CanFullTask;                                                 //ChungHung 20140313 add Fix3 can Full Tray
    #ifdef SOFT_SIMULTE
        bByPass=true;                                                           //Steven 20230516 : 為了讓軟體模擬看起來Fix3會動
    #endif

    if(AUTO_EMPTY_COLOR>=3)
        return true;

    if(FIX3_FULL_PLACE==Fix3K_UseCylinder)
    {
        switch(Task)                                                            //ChungHung 20140313 add Fix3 can Full Tray
        {
            case 1:
                if(MoveOutArmToAutoSafe_9045()==false)                          //ChungHung 20140313 add Fix3 can Full Tray
                    return bResult;

                iCount=0;
                bDuplicate=false;
                if(iMMAuto[iWhichFix]==MManualTray3)
                {
                    bShuttleMoveToLeftforFix3=true;                             //Shuttle1 and Shuttle2 go to left side
                }
                else
                {
                    bShuttleMoveToLeftforFix3=false;
                }
                bUseFix3CylinderActive=true;                                    //ChungHung 20150520 add fix Fix3 full 和 ShuttleShanke 衝突
                Cylinder[C_FixTray_FullPlace].Reset();
                Task=20;
            case 20:
                if(bShuttleMoveToLeftforFix3)                                   //ChungHung 20140313 add Fix3 can Full Tray
                {
                    if(InSHT1InLF()!=true || InSHT2InLF()!=true)                //wait shuttle1 and shuttle2 already in left side
                    {
                        if(bSuckingFlagZ1==false &&                             //Steven 20240916 : index下降到shuttle吸放料
                           InSHT1InLF()==false)                                 //Steven 20210802 : 修正index吸取異常跑[D43]流程, 同時out arm又要放Fix3
                        {
                            MOT[MInShuttle1].fCanMoveR=true;
                            if(InArmZSafe(DETECT_ALL_FLAG)==-1)                 //Steven 20211122 : 修正Fix3使用氣缸滿盤, 如果Z軸在上才可以推
                            {
                                MOT[MInShuttle1].fCanMoveM=true;                //Steven 20230118 : 修正index吸取異常跑[D43]流程, 同時out arm又要放Fix3
                                MOT[MInShuttle1].fCanMoveL=true;
                            }
                        }

                        if(bSuckingFlagZ2==false &&                             //Steven 20240916 : index下降到shuttle吸放料
                           InSHT2InLF()==false)
                        {
                            MOT[MInShuttle2].fCanMoveR=true;
                            if(InArmZSafe(DETECT_ALL_FLAG)==-1)                 //Steven 20211122 : 修正Fix3使用氣缸滿盤, 如果Z軸在上才可以推
                            {
                                MOT[MInShuttle2].fCanMoveM=true;                //Steven 20230118 : 修正index吸取異常跑[D43]流程, 同時out arm又要放Fix3
                                MOT[MInShuttle2].fCanMoveL=true;
                            }
                        }
                        return bResult;
                    }
                    MOT[MInShuttle1].fCanMoveR=false;
                    MOT[MInShuttle2].fCanMoveR=false;
                    if(Cylinder[C_FixTray_FullPlace].OnStatus()==false)         //ChungHung 20140609 add Use Fix3 Cylinder時 不要每次等Delay time
                    {
                        Cylinder[C_FixTray_FullPlace].On();
                        Fix3CylinderDelay.SetSecAndOn(10);                      //Steven 20230517 : 增加Fix 3氣缸10秒內沒到位的Alarm
                        Task=30;
                    }
                    else
                    {
                        Task=40;
                    }
                }
                else
                {
                    Cylinder[C_FixTray_FullPlace].Reset();
                    if(Cylinder[C_FixTray_FullPlace].OffStatus()==false)        //ChungHung 20140609 add Use Fix3 Cylinder時 不要每次等Delay time
                    {
                        Fix3CylinderDelay.SetSecAndOn(10);                      //Steven 20230517 : 增加Fix 3氣缸10秒內沒到位的Alarm
                        Cylinder[C_FixTray_FullPlace].Off();
                        Task=50;
                    }
                    else
                    {
                        Task=60;
                    }
                }

                if(Task!=30)
                {
                    break;
                }
            case 30:
                if(Cylinder[C_FixTray_FullPlace].Push() || bByPass)
                {
                    Fix3CylinderDelay.SetMSAndOn((bByPass)?1:300);              //wei 20150901 減少Delay時間 1000--->300
                    Task=31;
                }
                else if(Fix3CylinderDelay.Off())                                //Steven 20230517 : 增加Fix 3氣缸10秒內沒到位的Alarm
                {
                    if(bHangTimePause==true)                                    //Rogeryang 20251231 : Add HangTimePause need to reset
                        Fix3CylinderDelay.SetSecAndOn(10);
                    else
                        Task=45;
                    break;
                }
                else
                {
                    break;
                }
            case 31:
                if(Fix3CylinderDelay.Off())
                {
                    if(Cylinder[C_FixTray_FullPlace].OnSensor())                //wei 20150424 到位檢查
                    {
                        Task=40;
                    }
                    else
                    {
                        if(bHangTimePause==false)                               //Rogeryang 20251231 : Add HangTimePause need to reset
                            iCount++;
                        Task=30;
                        Fix3CylinderDelay.SetSecAndOn(10);                      //Steven 20230517 : 增加Fix 3氣缸10秒內沒到位的Alarm
                        Cylinder[C_FixTray_FullPlace].Reset();
                        if(iCount>=3)                                           //Steven 20190619 : Fix3氣缸動作增加異常的例外處理
                        {
                            Task=45;
                        }
                        break;
                    }
                }
                else
                {
                    break;
                }
            case 40:
                bResult=true;
                break;
            case 45:
                ShowErrorMessage("JAM1940", K_RETRY, MManualTray3, bDuplicate);
                Fix3CylinderDelay.SetSecAndOn(10);                              //Steven 20230517 : 增加Fix 3氣缸10秒內沒到位的Alarm
                iCount=0;
                bDuplicate=true;
                Task=30;
                break;
            case 50:
                if(Cylinder[C_FixTray_FullPlace].Pop() || bByPass)
                {
                    Fix3CylinderDelay.SetMSAndOn((bByPass)?1:300);              //wei 20150901 減少Delay時間 1000--->300
                    Task=51;
                }
                else if(Fix3CylinderDelay.Off())                                //Steven 20230517 : 增加Fix 3氣缸10秒內沒到位的Alarm
                {
                    if(bHangTimePause==true)                                    //Rogeryang 20251231 : Add HangTimePause need to reset
                        Fix3CylinderDelay.SetSecAndOn(10);
                    else
                        Task=65;
                    //break;                                                    //RogerYang 20251230 : remove break
                }
                else
                {
                    Fix3CylinderDelay.SetSecAndOn(10);
                    break;
                }
            case 51:
                if(Fix3CylinderDelay.Off())
                {
                    if(Cylinder[C_FixTray_FullPlace].OffSensor())               //wei 20150424 到位檢查
                    {
                        Task=60;
                    }
                    else
                    {
                        Task=50;
                        if(bHangTimePause==false)                               //Rogeryang 20251231 : Add HangTimePause need to reset
                            iCount++;
                        Fix3CylinderDelay.SetSecAndOn(10);                      //Steven 20230517 : 增加Fix 3氣缸10秒內沒到位的Alarm
                        Cylinder[C_FixTray_FullPlace].Reset();
                        if(iCount>=3)                                           //Steven 20190619 : Fix3氣缸動作增加異常的例外處理
                        {
                            Task=65;
                        }
                    }
                }

                if(Task!=60)
                {
                    break;
                }
            case 60:
                bUseFix3CylinderActive=false;
                MOT[MInShuttle1].fCanMoveR=true;
                MOT[MInShuttle2].fCanMoveR=true;
                bResult=true;
                break;
            case 65:
                ShowErrorMessage("JAM1941", K_RETRY, MManualTray3, bDuplicate);
                Fix3CylinderDelay.SetSecAndOn(10);                              //Steven 20230517 : 增加Fix 3氣缸10秒內沒到位的Alarm
                iCount=0;
                bDuplicate=true;
                Task=50;
                break;
        }
    }
    else if(FIX3_FULL_PLACE==Fix3K_UseCylinder46LA)                             //ChungHung 20140722 add for HT9046LA //蓋二樓的版本
    {
        if(Sen[SnFix3Lock].IsOn()==true)
        {
            ShowMyMessage("Please UnLock Fix3","");
            Task=1;
            return bResult;
        }

        switch(Task)
        {
            case 1:
                if(MoveOutArmToAutoSafe_9045()==false)                          //ChungHung 20140313 add Fix3 can Full Tray
                    return bResult;

                iCount=0;
                bDuplicate=false;
                if(iMMAuto[iWhichFix]==MManualTray3)
                {
                    if(Cylinder[C_FixTray_FullPlace].OnStatus()==false)         //ChungHung 20140609 add Use Fix3 Cylinder時 不要每次等Delay time
                    {
                        Cylinder[C_FixTray_FullPlace].On();
                        Fix3CylinderDelay.SetSecAndOn(10);                      //Steven 20230517 : 增加Fix 3氣缸10秒內沒到位的Alarm
                        Task=30;
                    }
                    else
                    {
                        Task=50;
                        bResult=true;
                    }
                }
                else
                {
                    if(Cylinder[C_FixTray_FullPlace].OnStatus()==true)          //ChungHung 20140609 add Use Fix3 Cylinder時 不要每次等Delay time
                    {
                        Cylinder[C_FixTray_FullPlace].Off();
                        Fix3CylinderDelay.SetSecAndOn(10);                      //Steven 20230517 : 增加Fix 3氣缸10秒內沒到位的Alarm
                        Task=40;
                    }
                    else
                    {
                        Task=50;
                        bResult=true;
                    }
                }
                break;
            case 30:
                if(Cylinder[C_FixTray_FullPlace].Push() || bByPass)
                {
                    if(Cylinder[C_FixTray_FullPlace].OnSensor())                //wei 20150424 到位檢查
                    {
                        Task=50;
                        bResult=true;
                    }
                    else
                    {
                        Cylinder[C_FixTray_FullPlace].Reset();
                        Fix3CylinderDelay.SetSecAndOn(10);                      //Steven 20230517 : 增加Fix 3氣缸10秒內沒到位的Alarm
                        if(bHangTimePause==false)                               //Rogeryang 20251231 : Add HangTimePause need to reset
                            iCount++;
                        if(iCount>=3)                                           //Steven 20190619 : Fix3氣缸動作增加異常的例外處理
                            Task=35;
                        else
                            Task=30;
                    }
                }
                else if(Fix3CylinderDelay.Off())
                {
                    if(bHangTimePause==true)                                    //Rogeryang 20251231 : Add HangTimePause need to reset
                        Fix3CylinderDelay.SetSecAndOn(10);
                    else
                        Task=35;
                }
                break;
            case 35:
                ShowErrorMessage("JAM1940", K_RETRY, MManualTray3, bDuplicate);
                Fix3CylinderDelay.SetSecAndOn(10);                              //Steven 20230517 : 增加Fix 3氣缸10秒內沒到位的Alarm
                iCount=0;
                bDuplicate=true;
                Task=30;
                break;
            case 40:
                if(Cylinder[C_FixTray_FullPlace].Pop() || bByPass)
                {
                    if(Cylinder[C_FixTray_FullPlace].OffSensor())               //wei 20150424 到位檢查
                    {
                        Task=50;
                        bResult=true;
                    }
                    else
                    {
                        Cylinder[C_FixTray_FullPlace].Reset();
                        Fix3CylinderDelay.SetSecAndOn(10);                      //Steven 20230517 : 增加Fix 3氣缸10秒內沒到位的Alarm
                        if(bHangTimePause==false)                               //Rogeryang 20251231 : Add HangTimePause need to reset
                            iCount++;
                        if(iCount>=3)                                           //Steven 20190619 : Fix3氣缸動作增加異常的例外處理
                            Task=45;
                        else
                            Task=40;
                    }
                }
                else if(Fix3CylinderDelay.Off())
                {
                    if(bHangTimePause==true)                                    //Rogeryang 20251231 : Add HangTimePause need to reset
                        Fix3CylinderDelay.SetSecAndOn(10);
                    else
                        Task=45;
                }
                break;
            case 45:
                ShowErrorMessage("JAM1941", K_RETRY, MManualTray3, bDuplicate);
                Fix3CylinderDelay.SetSecAndOn(10);                              //Steven 20230517 : 增加Fix 3氣缸10秒內沒到位的Alarm
                iCount=0;
                bDuplicate=true;
                Task=40;
                break;
            case 50:
                bResult=true;
                break;
        }
    }
    else if(FIX3_FULL_PLACE==Fix3K_UseStepperMotor)                             //JimmyChiu 20220927 : Stepper Motor Control in Fix3
    {
        switch(Task)
        {
            case 1:
                if(iMMAuto[iWhichFix]==MManualTray3 ||
                   iMMAuto[iWhichFix]==MManualTray2)
                {
                    if(MoveOutArmToAutoSafe_9045()==false)
                        return false;
                    if(iMMAuto[iWhichFix]==MManualTray3)
                        Task=30;
                    else
                        Task=40;
                }
                else
                {
                    return true;
                }
                break;
            case 25:                                                            //Initial home
                InitProcessSingleMotorTask(MFix3Full);
                Task=27;
            case 27:
                if(ProcessSingleMotorHome(MFix3Full)==true)
                {
                    Task=30;
                }
                break;
            case 30:
                if(bHandlerPause==true)                                         //JimmyChiu 20230808 : Fixed for Pause to lost position when step motor move
                {
                    Task=25;
                }
                else if(Fix3MoveToLeft(true))
                {
                    Task=50;
                }
                break;
            case 35:                                                            //Initial home
                InitProcessSingleMotorTask(MFix3Full);
                Task=37;
            case 37:
                if(ProcessSingleMotorHome(MFix3Full)==true)
                {
                    Task=40;
                }
                break;
            case 40:
                if(bHandlerPause==true)                                         //JimmyChiu 20230808 : Fixed for Pause to lost position when step motor move
                {
                    Task=35;
                }
                else if(Fix3MoveToLeft(false))
                {
                    Task=50;
                }
                break;
            case 50:
                bResult=true;
                break;
        }
    }
    return bResult;
}
// ---- golden aoutarm9045.cpp:3766-3798 Check_QA_ModeUnloadCount ----
bool Check_QA_ModeUnloadCount()                                                 //JerryYang 20221004 : Maxim版本QA mode
{
    if(IniConfig.bQAMode==true && LastSet.iRunStartMode==rsmQAMode)             //Steven 20111005 : QA Mode
    {                                                                           //jou 2012-05-03 增加QA mode使用獨立的Loader Count
        if(iQAModePassCT>=Prod.iQAModeCount)                                    //數量到的話要先快速CleanOut
        {
            if(bQAModeQuickCleanOut==false && bQAModeFinishCleanOut==false)
            {
                TrayForm.bAutoFeed=false;
                bQAModeQuickCleanOut=true;
                if(fMain->BtnOneCycle->Down==false)
                {
                    InitOneCycle("QA Mode");
                    fMain->BtnOneCycle->Down=true;
                }
                return true;
            }

            if(bQAModeQuickCleanOut==true && bQAModeFinishCleanOut==false)
            {
                ArmSpeed[InArm].bVariModeFIX=IniConfig.bBackUpInArmMode;
                bQAModeFinishCleanOut=true;
                bQAModeQuickCleanOut=false;
            }
        }
        else if(iQAModePassCT>Prod.iQAModeCount-20 &&
                iQAModePassCT<Prod.iQAModeCount)                                //數量接近時，要改用Fix方式生產
        {
            ArmSpeed[OutArm].bVariModeFIX=true;
        }
    }
    return false;
}

// ===========================================================================
//  AI(W906-OUTADD) 20260927: golden 出料臂附加功能（旋轉站／AOI／Fix AI CCD）逐行照翻：
//  CheekNeedToDoOutArmAdditionalFunction 依機種設定與吸嘴上 IC 決定要做哪幾項；DoOutArmAdditionalFunction 依序做；
//  PreSetOutAdditionalFlag 依設定預設「已做／未做」。以前：判斷式一律「不需要」、動作整段 #if 0、預設一律「全部已做完」
//  ⇒ 不管機台有沒有裝出料旋轉站／AOI／Fix AI CCD，出料臂都不會做。入料臂那一側是 AI(W906-INADD)（ainarm9045.cpp 檔尾）。
//  移植樹沒有的相依才閘（GATE(W906-OUTADD)）。
// ===========================================================================
// golden 經 aRotateKIT_Out.h 取得；本 TU 沒有 include，只補宣告（本體 RotateKit/aRotateKIT_Out.cpp:437／:1103／:4522）。
void InitialOutArmRotateKIT();
bool DoOutArmRotateKIT();
bool CheckRotateOutNotFinish();
// ---- golden aoutarm9045.cpp:2143-2244 CheekNeedToDoOutArmAdditionalFunction ----
bool CheekNeedToDoOutArmAdditionalFunction()                                    //Steven 20210609 : 整合Precisor, Rotator, Bottom CCD, Die Clean
{
    bool bResult=false;
    bool bSupportFixAI=false;
    if(OutArmSuck.bAlreadyRotate==false)
    {
        if(CheckRotateOutNotFinish())
        {
            bOutRotator=true;
            bResult=true;
        }
        else if(USE_ROTATE_KIT==1 &&
                tRotate.ActiveRotate &&
                TrayForm.iRotateKIT_InputType!=0 &&
                (iOutRotateFinish==0 || iOutRotateFinish==1))
        {
            bOutRotator=true;
            bResult=true;
        }
    }

#if 1 // was: #if 0 -- opened AI(W906-POOL2) 20261008 (Ifor01): GATE(W906-OUTADD) bART_RT_NoRotate reason expired -- tRotateShim is the full TRotate since 0926 (forms/fRotate.h:95 bART_RT_NoRotate), FileRW/Rotate.gen.inc:260-261 reads it from Rotate.Data as golden RotateKit/fRotate.cpp:288-289; golden :2164-2165
    if(CosFunction.bART_RT_NoRotate && tRotate.bART_RT_NoRotate && bCanRunSCKART)                                       //Sam 20240809 : ART RT No Rotate
        bOutRotator=false;
#endif // GATE(W906-OUTADD)
    if(OutArmSuck.bAlreadyAOI==false)
    {
//        if(tAOISetup.bEnabledAOI ||                                             //2014-03-04    Dell    for SPIL WLP Add 5S Inspection
//           (USE_Scanner_AOI_Inspection==true && ScannerAOIIF.iEnableScannerMode!=0) ||
//           (USE_Top_Scanner_AOI_Inspection==true && ScannerAOIIF.iEnableTopScannerMode!=0))
        if(tAOISetup.bEnabledAOI ||  //2014-03-04    Dell    for SPIL WLP Add 5S Inspection
           (USE_Scanner_AOI_Inspection==(int)eBtnAOI_BottomInstall && ScannerAOIIF.iEnableScannerMode!=0) || //AI(general) 20260411 (RogerYang) : fix eBtnAOI_Uninstall typo, was always false
           (USE_Top_Scanner_AOI_Inspection==(int)eBtnAOI_BottomInstall && ScannerAOIIF.iEnableTopScannerMode!=0) ||
#if 0 // GATE(W906-OUTADD) golden :2174-2174 —— 轉接：TFrmAOI（golden fAOI.cpp:43 FrmAOI）與 InitAOIFunction／DoAOIFunction（fAOI.cpp:2739／:2753）移植樹沒有；條件式裡只把 `FrmAOI->RunTopBottomInspect()==true` 換成 false，tAOISetup／ScannerAOIIF 那幾項照 golden
           FrmAOI->RunTopBottomInspect()==true)  //Jimmychiu 20240322 : Top & Bottom Inspect
#else // GATE(W906-OUTADD) 轉接版（只把移植樹沒有的那一項換成 false，其他照 golden）
           false)  //Jimmychiu 20240322 : Top & Bottom Inspect
#endif // GATE(W906-OUTADD)
        {
            bDoAOI=true;
            bResult=true;
        }
    }

    if(OutArmSuck.bAlreadyFixAI==false)                                         //Sam 20211220 : 整合 Fix AI AOI
    {
        if(TestIF.iTestMode==SingleSite                         ||
           TestIF.iTestMode==QualSite2X2N                       ||
           TestIF.iTestMode==DualSite)
        {
            bSupportFixAI=true;
        }
        else if(TestIF.iTestMode==QualSite2X2)
        {
            if(USE_PICKER_COUNT==0)
            {
                bSupportFixAI=true;
            }
            else if(TestIF.iUseSuckMode==4 && TestIF.dSiteXPitch>iXpitchMinX2)
            {
                bSupportFixAI=true;
            }
            else
            {
                if(TestIF.iUseSuckMode==8 &&
                   TestIF.dSiteXPitch>iXpitchMinX2)                             //Sam 20231107 : 補 Fix AOI 模式
                {
                    bSupportFixAI=true;
                }
            }
        }
       else if(TestIF.iTestMode==QualSite1X4 ||                                 //Sam 20220613: 修正 FixAOI 未動作問題
               TestIF.iTestMode==_12Site2X6  ||                                 //Sam 20240329 : 新增 FixAOI 模式
               TestIF.iTestMode==_16Site2X8)
        {
            //if(TestIF.iUseSuckMode==4)                                        //Sam 20231107 : 補 Fix AOI 模式 Mark
            {
                bSupportFixAI=true;
            }
        }
        else if(TestIF.iTestMode==_8Site2X4 ||
                TestIF.iTestMode==_16Site4X4)
        {
            if(ArmCanSuck4IC(0, true)==false || USE_PICKER_COUNT==0)
            {
            }
            else
            {
               bSupportFixAI=true;
            }
        }

#if 0 // GATE(W906-OUTADD) golden :2229-2236 —— TfFixAICCD 的 NeedToGrabImage／Fix2AICCDFunction／DoFix2AICCDFunction（矽格湖口 AI CCD）移植樹的 facade 沒有
        if(bSupportFixAI &&
           USE_Fix_AI_CCD &&
           TestIF_File.bEnableFix2BGAAICCD &&
           fFixAICCD->NeedToGrabImage())                                        //KaiChen 20190508 ：矽格湖口 AI CCD Function
        {
            bDoFixAI=true;
            bResult=true;
        }
#endif // GATE(W906-OUTADD)
    }

    if(bResult)
    {
        InitDoOutArmAdditionalFunction();
    }
    return bResult;
}
// ---- golden aoutarm9045.cpp:2246-2314 DoOutArmAdditionalFunction ----
bool DoOutArmAdditionalFunction()                                               //Steven 20210609 : 整合Precisor, Rotator, Bottom CCD, Die Clean
{
    bool bResult=false;
    int &Task=iOutArmAdditionalFunctionTask;

    switch(iOutArmAdditionalFunctionTask)
    {
        case 1:
            CheekNeedToDoOutArmAdditionalFunction();
            Task=100;
        case 100:
            if(MoveOutArmToAutoSafe())
            {
                CheckOutArmSuckICFallDown();
                MOT[MInShuttle1].fCanMoveR=true;
                MOT[MInShuttle2].fCanMoveR=true;
                if(OutArmSuck.bAlreadyRotate==false && bOutRotator)
                {
                    if(iOutRotateFinish==0)
                    {
                        InitialOutArmRotateKIT();
                        iOutRotateFinish=1;
                    }
                    Task=10000;
                }
#if 0 // GATE(W906-OUTADD) golden :2271-2280 —— AOI 與 Fix AI 兩條 else-if：TFrmAOI（golden fAOI.cpp:43 FrmAOI）與 InitAOIFunction／DoAOIFunction（fAOI.cpp:2739／:2753）移植樹沒有；TfFixAICCD 的 NeedToGrabImage／Fix2AICCDFunction／DoFix2AICCDFunction（矽格湖口 AI CCD）移植樹的 facade 沒有（閘掉這兩臂後 else-if 鏈照樣成立：旋轉站那一臂＋最後的 else）
                else if(OutArmSuck.bAlreadyAOI==false && bDoAOI)
                {
                    InitAOIFunction();
                    Task=20000;
                }
                else if(OutArmSuck.bAlreadyFixAI==false && bDoFixAI)
                {
                    fFixAICCD->Fix2AICCDFunction();
                    Task=30000;
                }
#endif // GATE(W906-OUTADD)
                else
                {
                    bResult=true;
                }
            }
            break;
        case 10000:
            if(DoOutArmRotateKIT()==true)
            {
                OutArmSuck.bAlreadyRotate=true;                                 //Steven 20260612 : Fix == to = (was comparison, not assignment)
                bOutRotator=false;
                iOutRotateFinish=2;                                             //kevin 20130524 目前放到 auto tray
                Task=100;
            }
            break;
#if 0 // GATE(W906-OUTADD) golden :2296-2311 —— 動作 case 20000（DoAOIFunction）與 30000（fFixAICCD->DoFix2AICCDFunction）：TFrmAOI（golden fAOI.cpp:43 FrmAOI）與 InitAOIFunction／DoAOIFunction（fAOI.cpp:2739／:2753）移植樹沒有；TfFixAICCD 的 NeedToGrabImage／Fix2AICCDFunction／DoFix2AICCDFunction（矽格湖口 AI CCD）移植樹的 facade 沒有
       case 20000:
            if(DoAOIFunction()==true)
            {
                OutArmSuck.bAlreadyAOI=true;
                bDoAOI=false;
                Task=100;
            }
            break;
       case  30000:
            if(fFixAICCD->DoFix2AICCDFunction()==true)
            {
                OutArmSuck.bAlreadyFixAI=true;
                bDoFixAI=false;
                Task=100;
            }
            break;
#endif // GATE(W906-OUTADD)
    }
    return bResult;
}
// ---- golden aoutarm9045.cpp:2316-2331 PreSetOutAdditionalFlag ----
void PreSetOutAdditionalFlag()                                                  //Steven 20210609 : 整合Precisor, Rotator, Bottom CCD, Die Clean
{
    if(USE_ROTATE_KIT==1 && tRotate.ActiveRotate==true && MOT[MOutRotateKit].HasRealIC())
    {
        OutArmSuck.bAlreadyRotate=false;
    }
    else
    {
        OutArmSuck.bAlreadyRotate=(USE_ROTATE_KIT==1 && tRotate.ActiveRotate==true)?false:true;                         //JerryYang 20230503 : 修正out rotate不作動的問題
    }
//    OutArmSuck.bAlreadyAOI  =(tAOISetup.bEnabledAOI || (USE_Scanner_AOI_Inspection==true && ScannerAOIIF.iEnableScannerMode!=0))?false:true;
    OutArmSuck.bAlreadyAOI  =(tAOISetup.bEnabledAOI ||
                             (USE_Scanner_AOI_Inspection==(int)eBtnAOI_BottomInstall && ScannerAOIIF.iEnableScannerMode!=0) ||
#if 0 // GATE(W906-OUTADD) golden :2329-2329 —— 轉接：TFrmAOI（golden fAOI.cpp:43 FrmAOI）與 InitAOIFunction／DoAOIFunction（fAOI.cpp:2739／:2753）移植樹沒有；只把 `FrmAOI->ttbInsp->iEnable==1` 換成 false
                             (USE_Scanner_AOI_Inspection==(int)eBtnAOI_TopBottomInstall && FrmAOI->ttbInsp->iEnable==1))?false:true;
#else // GATE(W906-OUTADD) 轉接版（只把移植樹沒有的那一項換成 false，其他照 golden）
                             (USE_Scanner_AOI_Inspection==(int)eBtnAOI_TopBottomInstall && false))?false:true;
#endif // GATE(W906-OUTADD)
#if 0 // GATE(W906-OUTADD) golden :2330-2330 —— 轉接：TfFixAICCD 的 NeedToGrabImage／Fix2AICCDFunction／DoFix2AICCDFunction（矽格湖口 AI CCD）移植樹的 facade 沒有；只把 `fFixAICCD->NeedToGrabImage()` 換成 false（⇒ bAlreadyFixAI＝true）
    OutArmSuck.bAlreadyFixAI=(USE_Fix_AI_CCD && TestIF_File.bEnableFix2BGAAICCD && fFixAICCD->NeedToGrabImage())?false:true;                                    //Sam 20211220 : 整合 Fix AI AOI
#else // GATE(W906-OUTADD) 轉接版（只把移植樹沒有的那一項換成 false，其他照 golden）
    OutArmSuck.bAlreadyFixAI=(USE_Fix_AI_CCD && TestIF_File.bEnableFix2BGAAICCD && false)?false:true;                                    //Sam 20211220 : 整合 Fix AI AOI
#endif // GATE(W906-OUTADD)
}
//==============================================================================
//AI(W906-FLOW-2) 20260928: golden aoutarm9045.cpp:252-283 IsCheckOutArmDestroyActiveFinish (port `return true` stand-in at
//  :582-595 retired: :582 is now a forward declaration, :583-595 the commented-out stand-in).  Translated VERBATIM
//  (cp950 -> UTF-8, comments kept).  It is the pick-side half of the pair whose other half, CheckOutArmDestroyActive
//  (:621, golden :287-400), 216f7bd8 made live: that SM clears bCheckOutArmDestroyActiveFinish[i][j] (:645) when a place
//  arms the blow-off check and sets it back (:720) once the nozzle reads clear.  This function makes the out-arm pick at
//  the out-shuttle wait (27 per-layout call sites, e.g. aoutarm9045_2x6_8.cpp:989 / golden aoutarm9045_2x2_4.cpp:375)
//  while any nozzle is still being checked and the arm is within 100 counts of the pick XY, lifting the nozzles meanwhile.
//  Dependencies, all live: MOT[MOutArmX/Y].ReadPos (Motor/mymotor.h), OutArmSuck.iPickRow/iPickCol (mykitsuck.h),
//  bCheckOutArmDestroyActiveFinish (:88, golden :251), MoveOutArmToAutoSafe (aoutarm.cpp:858 -> MoveOutArmToAutoSafe_9045 :385).
//==============================================================================
bool IsCheckOutArmDestroyActiveFinish(int iXPos, int iYPos)                     //2013-08-02    Dell add Pick 前先確認CheckInArmDestroyActive 已完成
{
    #ifdef SOFT_SIMULTE
    if(CUSTOMER_CODE==CC_AMKOR_Japan)                                           //RogerYang 20250813 add
        return true;
    #endif
    int iPosX=0, iPosY=0;
    int iPreRange=100;
    bool bRet=true;
    iPosX=MOT[MOutArmX].ReadPos();
    iPosY=MOT[MOutArmY].ReadPos();

    if(iPosX>=iXPos-iPreRange && iPosX<=iXPos+iPreRange &&
       iPosY>=iYPos-iPreRange && iPosY<=iYPos+iPreRange)
    {
        for(int i=0; i<OutArmSuck.iPickRow; i++)
        {
            for(int j=0; j<OutArmSuck.iPickCol; j++)
            {
                if(bCheckOutArmDestroyActiveFinish[i][j]==false)
                    bRet=false;
            }
        }
    }

    if(bRet==false)                                                             //Steven 20241023 : 為了避免卡死, 先把吸嘴抬起來
    {
        MoveOutArmToAutoSafe();
    }

    return bRet;
}
