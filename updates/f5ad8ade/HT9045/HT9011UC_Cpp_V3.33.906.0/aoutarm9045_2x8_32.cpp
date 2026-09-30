// =============================================================================
//  aoutarm9045_2x8_32.cpp  --  Out-Arm per-site VARIANT (2x8_32, two-row 8-picker 32-site)  [DEAD]
//
//  Faithful translation of golden aoutarm9045_2x8_32.cpp (2867 lines, BCB6, Big5/cp950).
//  Translator: AI(W6.2c-OUT-2x8_32) 20260627
//  Translation wave: W6.2c-OUT batch-4 (DEAD).
//
//  DEAD VARIANT: enum e9045_2x8_32 is PRESENT in MachineType.h but BOTH out-arm
//  dispatch ladders in the engine aoutarm9045.cpp route it to the LIVE 2x8_8 callee
//  (DoOutArm: e9045_2x8_8||e9045_2x8_32 -> DoOutArm_9045_2x8_8 ; DoPickFromShuttle:
//  e9045_2x8_8||e9045_2x8_32 -> DoPickFromShuttle_9045_2x8_8).  Therefore
//  DoOutArm_9045_2x8_32 / SetOutArm_9045_2x8_32 / MoveOutArmToShuttleIncludeZ_9045_2x8_32
//  and every _2x8_32-suffixed helper are NEVER dispatched -- they compile and link as
//  UNIQUE dead-weight symbols (exactly like in-arm ainarm9045_2x8_32.cpp).  This file is
//  added to CMake ONLY: NO static-stub removal, NO extern un-gate, NO dispatch-ladder edit.
//
//  IMPORTANT (vs in-arm 2x8_32): the OUT-arm 2x8_32 needs NO whole-body #if 0 gate --
//  it has no absent Prod member (all 13 Prod.<member> it uses exist in translated cprod.h),
//  so it translates faithfully in full like the live siblings.
//
//  ROLE: the per-site out-arm shuttle-pick / place-to-auto state machines for the 2x8_32
//  layout (2 physical rows, 8 picker columns, 32-site via iOutArmWhichKit=0/4).  This file
//  owns (golden names VERBATIM, all carry the _2x8_32 suffix -> UNIQUE):
//    static int iOutArmWhichKit ; GetNowShuttleMode_2x8_32 ; OutArmZNeedDown_2x8_32 ;
//    GetOutShuttleStatus_2x8_32 ; CheckOutArmXYPitch_2x8_32 ;
//    MoveOutArmToShuttleIncludeZ_9045_2x8_32 (3-param iShuttle/iWhichKit/bZDown) ;
//    DoPickFromShuttle1_9045_2x8_32(bool) ; DoPickFromShuttle2_9045_2x8_32(bool) ;
//    DoMoveOutArmXYToPlace_9045_2x8_32 ; SearchUnLoadTrayUpDown_9045_2x8_32 ;
//    SetOutArm_9045_2x8_32 ; DoPlaceToAuto_9045_2x8_32 ;
//    GetTrayPosition_AccordingBaseSuck_9045_2x8_32 ;
//    GetTrayPosition_AccordingTopLeftActiveSuck_9045_2x8_32 ;
//    DoMoveOutArmXYToPlaceForAutoTeachOffset_9045_2x8_32 ;
//    DoPlaceToAutoForAutoTeachOffset_9045_2x8_32 ; DoOutArm_9045_2x8_32.
//
//  FAITHFUL: every fn/var/cursor value, switch(Task) structure, formula, branch,
//  fall-through and commented-out code is transcribed VERBATIM from the golden CP950
//  source.  The DoOutArm fall-throughs (3100->3300, 3300 special, 3305->3310,
//  3310->3500) and the case-50/100/1140/2040 flows are preserved EXACTLY.  Integer
//  division / truncation (iMovePitchX*iVariablePara[j]/3 etc.) kept EXACT.
//
//  W7-GATE (the ONLY non-verbatim deltas): the VCL UI-form singleton fAutoTeach
//  (golden AutoTeach.h TfAutoTeach : public TForm) has NO offline FormsFacade stand-in.
//  Every fAutoTeach-> deref is wrapped in `#if 0 // TODO(W7)` with the surrounding control
//  flow kept LIVE and an offline-sane default (auto-alignment teach never running ->
//  the gated branches are skipped; DoPlaceToAutoForAutoTeachOffset_9045_2x8_32 returns 1
//  = "finish without next step", and DoOutArm case-3300's fAutoTeach->IsRun() reads false).
//  Nothing else is gated -- all SM bodies touch MOT[]/Suck/Sensor/predicates/engine,
//  which resolve over the Sim HAL + aHotPlateSubstrate + the shims + the FWD-declared
//  engine externs.
//
//  VCL/Borland conversions: VCL AnsiString (1-based) via vclcompat; this file has no
//  __fastcall/__property/__published/PACKAGE.  (This golden file has no #ifdef SOFT_SIMULTE
//  block.)
//
//  Big5: every Chinese comment decoded via cp950 and preserved as UTF-8.  ZERO U+FFFD.
//
//  INCLUDE REWRITE (mirror of the golden header set, translated subset):
//    golden: MachineDefine.h ; aoutarm9045_2x8_32.h ; aArmHeader.h ; MyKitSuck.h ;
//            cprod.h ; mymotor.h ; mycylin.h ; cSortCT.h ; mysensor.h ; main.h ; note.h ;
//            csystem.h ; acatchtray.h ; cinitial.h ; atester.h ; aRotateKIT.h ; fRotate.h ;
//            cOffSet.h ; fAOI.h ; cmydef.h ; ProductionInfo.h ; Magazine.h ;
//            #pragma package(smart_init)
// =============================================================================
#include "MachineDefine.h"                  // <windows.h> (ZeroMemory), vclcompat
#include "MachineType.h"                     // e9045_2x8_32 / e9045_2x8_8 / CC_KYEC_LEE global enums
#include "aoutarm9045_2x8_32.h"

#include "aArmHeader.h"             // __FUNC__ shim, RecordProcess, ShowErrorMessage
#include "aoutarm9045.h"           // engine: GetOutArmPitch*_9045 / DoOutArmSuckPreOn / IsCheckOutArmDestroyActiveFinish / InitialOutArmNeedSuck / iOUTARM_SUCK / SearchTrayToPlace_9045 / DoOutArmPlaceToAuto / DoOutArmAfterPlaceToAuto / DoOutArmAdditionalFunction / CheekNeedToDoOutArmAdditionalFunction / MoveOutArmXY_To_ESDSafePos / PreSetOutAdditionalFlag / iBinBoxShiftY
#include "ainarm9045.h"            // GetVariableYInShuttleData / bCheckYPitchRunHomeSen / bCheckYPitchHome (Ypitch home sensors, shared)
#include "Motor/mymotor.h"         // MOT[], OutArmContinuousMove_9045, OutArm enum, MInShuttle1/2, MTrayX, MOutArmX/Y, MOutArmPitchY, iMMAuto
#include "cprod.h"                 // Prod / TestIF / TestIF_File / ArmSpeed / OutArmOffSet / OutputForm / FixForm / ReserveEmptyPoint / iXTrayColor
#include "cpublic.h"               // CosFunction
#include "cmydef.h"                // global scalar universe + IC consts + iInArmType + enums + iXpitchMaxX2/3 + OutArmClose_Pitch + iUnloadMotorIndex + iAutoCnt/iAutoRight/iFixPosMin/iFixPosMax + iFix3Pos + iBinBoxAtFix/iBinBoxType + iFixRight + AseIcRecord / bPickSH*Flag / iWhich* / iAutoIndex
#include "common.h"
#include "mycylin.h"               // Cylinder[] / C_Auto*
#include "mysensor.h"              // Sen[] / SnFixedTrayDetect / SnAutoTrayDetect / SnAutoFixCyPush / SnAutoEdgePush / SnRKManualStep
#include "csystem.h"               // OutSHT1InRT / OutSHT2InRT / CheckOutArmSuckICFallDown / IsOutArmCleanOutFinish / AutoTrayReCheck / HasICUnderMachine / HasICUnderHotPlate
#include "acatchtray.h"            // WhichAutoNeedTray / IsTrayArmMoveAvoidOutArmCrash
#include "aHotPlateSubstrate.h"    // OutArmSuck / FRCarryKit / BRCarryKit (TMyKitSuck) + FTestSuck/BTestSuck + TQPF_Timer + cursors + SetMotorSpeed + GetInArmPitchY_9045 + X_PITCH_COUNT + iWhichBuff / iWhichMag
#include "FormsFacade.h"           // fMain (Pause) / TrayForm / fProductionInfo / fOffSet offline stand-ins
#include "acarry_shims.h"          // MoveOutArmToAutoSafe / MoveOutArmXY_ToShuttleAlarmArea / LastSet / NULL_IC
#include "acatchtray_shims.h"      // InitProcessSingleMotorTask
#include "ainarm9045_2x4_16_shims.h"// AutoTeachLoadTrayZ
#include "canary_support.h"        // ShowErrorMessage / ShowMyMessage / RecordProcess / IniConfig / WhichAutoNeedTray / K_RETRY / K_SKIP / K_HOME / DUMMY / iOutArmZTeachTask

// -----------------------------------------------------------------------------
//  SUBSTRATE / ENGINE EXTERNS not yet provided by the current cpp tree (REPORTED
//  for the serial Integrate phase).  Forward-declared behind #ifndef guards (the
//  SAME guard-macro names used by the committed sibling out-arm variant TUs, so
//  these CANNOT ODR-collide and stay single-source).  Signatures / default args are
//  VERBATIM from golden aoutarm.h / aoutarm9045.h / MyKitSuck.h / Magazine.h, or for
//  the place-path helpers, MATCH the arity used by the 2x8_32 call sites (genuine
//  engine link externs -- no stub bodies here).
// -----------------------------------------------------------------------------
#ifndef AOUT_PTROUTSHT_FWD
#define AOUT_PTROUTSHT_FWD
extern TMyKitSuck *ptrOutSHT;                                                  // golden MyKitSuck.h -- offline: &FRCarryKit / &BRCarryKit
#endif
#ifndef AOUT_PICKFROMSHUTTLE_FWD
#define AOUT_PICKFROMSHUTTLE_FWD
void PickFromShuttle(int ibwhichKit);                                          //kevin 20210612 add int iwhichKit=0
#endif
#ifndef AOUT_BOUTSHTWAITPICK_FWD
#define AOUT_BOUTSHTWAITPICK_FWD
bool bOutShtwaitPick();                                                        //kevin 20210602 Out arm sht on wait
#endif
#ifndef AOUT_SHOWOUTPUTSHUTTLEDATAMISS_FWD
#define AOUT_SHOWOUTPUTSHUTTLEDATAMISS_FWD
void ShowOutputShuttleDataMiss();                                             //Steven 20110527
#endif
#ifndef AOUT_ERRORBINBOXDETECT_FWD
#define AOUT_ERRORBINBOXDETECT_FWD
bool ErrorBinBoxDetect(int iError);                                           //kevin 20160906 error bin box
#endif
#ifndef AOUT_OUTARMPICKSHUTTLEALARM_FWD
#define AOUT_OUTARMPICKSHUTTLEALARM_FWD
int OutArmPickShuttleAlarm(int iSht, bool bHasDuplicateErr, AnsiString ErrPart);
#endif
#ifndef AOUT_PORCESSJAM0201_FWD
#define AOUT_PORCESSJAM0201_FWD
void PorcessJAM0201OutArmPickUpErrorSkip(int iSht, int iShtR, int iShtC, int iSuckR, int iSuckC);
#endif
#ifndef AOUT_SETOUTARMHOME_FWD
#define AOUT_SETOUTARMHOME_FWD
void SetOutArmHome();
#endif
#ifndef AOUT_BOUTARMMANUALSTEPPRESS_FWD
#define AOUT_BOUTARMMANUALSTEPPRESS_FWD
extern bool bOutArmManualStepPress;
#endif
#ifndef AOUT_ISCATCHTRAYREADYSUPPLYNEWTRAY_FWD
#define AOUT_ISCATCHTRAYREADYSUPPLYNEWTRAY_FWD
bool IsCatchTrayReadySupplyNewTray();
#endif
#ifndef AOUT_CHECKSHUTTLEICPOS_FWD
#define AOUT_CHECKSHUTTLEICPOS_FWD
bool CheckShuttleICPos(int WhichShuttle);
#endif
#ifndef AOUT_CHECKTESTERZ_FWD
#define AOUT_CHECKTESTERZ_FWD
bool CheckTesterZ(int WhichShuttle);
#endif
#ifndef AOUT_CHECKOUTARMINITSTATE_FWD
#define AOUT_CHECKOUTARMINITSTATE_FWD
bool CheckOutArmInitState();
#endif
#ifndef AOUT_TRANSFEROUTSHUTTLERATIO_FWD
#define AOUT_TRANSFEROUTSHUTTLERATIO_FWD
void TransferOutShuttleRatio(int iShuttle, int *iXPos, int *iYPos, int Row, int Col);
#endif
#ifndef AOUT_SWAPSHUTTLEDATATOOUTARM_FWD
#define AOUT_SWAPSHUTTLEDATATOOUTARM_FWD
extern bool SwapShuttleDataToOutArm(int iSht, int iShtRow, int iShtCol, int iSuckRow, int iSuckCol, int *iWitchErrBin);  //Steven 20170428
#endif
#ifndef AOUT_INITPICKFROMSHUTTLE1TASK_FWD
#define AOUT_INITPICKFROMSHUTTLE1TASK_FWD
void InitPickFromShuttle1Task();
#endif
#ifndef AOUT_INITPICKFROMSHUTTLE2TASK_FWD
#define AOUT_INITPICKFROMSHUTTLE2TASK_FWD
void InitPickFromShuttle2Task();
#endif
#ifndef AOUT_INITPLACETOAUTOTASK_FWD
#define AOUT_INITPLACETOAUTOTASK_FWD
void InitPlaceToAutoTask();
#endif
#ifndef AOUT_SETFIXTRAYFULLIC_FWD
#define AOUT_SETFIXTRAYFULLIC_FWD
void SetFixTrayFullIC();
#endif
#ifndef AOUT_OUTARMSUBSPEED_FWD
#define AOUT_OUTARMSUBSPEED_FWD
void OutArmSubSpeed();
#endif
#ifndef AOUT_OUTARMADDSPEED_FWD
#define AOUT_OUTARMADDSPEED_FWD
void OutArmAddSpeed();
#endif
#ifndef AOUT_OUTARMSUBSPEEDDISPLAY_FWD
#define AOUT_OUTARMSUBSPEEDDISPLAY_FWD
void OutArmSubSpeedDisplay();                                                  //KaiChen 20171225 (Steven)
#endif
#ifndef AOUT_OUTARMADDSPEEDDISPLAY_FWD
#define AOUT_OUTARMADDSPEEDDISPLAY_FWD
void OutArmAddSpeedDisplay();                                                  //KaiChen 20171225 (Steven)
#endif
#ifndef AOUT_OUTARMTASK_FWD
#define AOUT_OUTARMTASK_FWD
extern int OutArmTask;
#endif
#ifndef AOUT_CHECKOUTARMCLEANOUT_FWD
#define AOUT_CHECKOUTARMCLEANOUT_FWD
int CheckOutArmCleanOut(int Task=50);
#endif
#ifndef AOUT_GETVARIABLEYOUTSHUTTLEDATA_FWD
#define AOUT_GETVARIABLEYOUTSHUTTLEDATA_FWD
int GetVariableYOutShuttleData();                                             //ChungHung 20131231 AutoYPitch
#endif
#ifndef AOUT_IPICKFROMSHUTTLE1TASK_FWD
#define AOUT_IPICKFROMSHUTTLE1TASK_FWD
extern int iPickFromShuttle1Task;
#endif
#ifndef AOUT_IPICKFROMSHUTTLE2TASK_FWD
#define AOUT_IPICKFROMSHUTTLE2TASK_FWD
extern int iPickFromShuttle2Task;
#endif
#ifndef AOUT_DOFIXTRAYFULLALARM_FWD
#define AOUT_DOFIXTRAYFULLALARM_FWD
bool DoFixTrayFullAlarm();
#endif
#ifndef AOUT_SEARCHTRAYTOPLACE_MAGAZINE_FWD
#define AOUT_SEARCHTRAYTOPLACE_MAGAZINE_FWD
int  SearchTrayToPlace_Magazine();                                            //JerryYang Eastsun 20260515
#endif
#ifndef AOUT_HOUTARMYPITCHHOMETIMER_FWD
#define AOUT_HOUTARMYPITCHHOMETIMER_FWD
extern TQPF_Timer hOutArmYpitchHomeTimer;                                     //golden ainarm2.h HTimer; offline TQPF_Timer
#endif
#ifndef AOUT_BYPITCHNEDDHOME_FWD
#define AOUT_BYPITCHNEDDHOME_FWD
extern bool bYpitchNeddHome;                                                  //kevin 20180823
#endif
#ifndef AOUT_CHECKPLACETOMAGAZINETRAY_FWD
#define AOUT_CHECKPLACETOMAGAZINETRAY_FWD
extern bool CheckPlaceToMagazineTray(int iWhichTray);
#endif
#ifndef AOUT_DOPICKFROMMAGAZINEBUFFER_FWD
#define AOUT_DOPICKFROMMAGAZINEBUFFER_FWD
extern bool DoPickFromMagazineBuffer();
#endif
#ifndef AOUT_BCARRYCONTROLOUTARM1_FWD
#define AOUT_BCARRYCONTROLOUTARM1_FWD
extern bool bCarryControlOutarm1;                                             //golden aoutarm.h
#endif
#ifndef AOUT_BCARRYCONTROLOUTARM2_FWD
#define AOUT_BCARRYCONTROLOUTARM2_FWD
extern bool bCarryControlOutarm2;                                             //golden aoutarm.h
#endif
#ifndef AOUT_CHECKROTATEOUTNOTFINISH_FWD
#define AOUT_CHECKROTATEOUTNOTFINISH_FWD
bool CheckRotateOutNotFinish();                                               //golden aRotateKIT_Out.h
#endif
#ifndef AOUT_MOVEOUTARMXYTOFIXTRAYFULL_FWD
#define AOUT_MOVEOUTARMXYTOFIXTRAYFULL_FWD
bool MoveOutArmXY_ToFix_Tray_Full(bool bMoveY=false);                         //golden aoutarm.h
#endif
#ifndef AOUT_IOUTROTATEFINISH_FWD
#define AOUT_IOUTROTATEFINISH_FWD
extern int iOutRotateFinish;                                                  //kevin 20130524 golden aRotateKIT.h
#endif
#ifndef AOUT_IWHICHAUTO_FWD
#define AOUT_IWHICHAUTO_FWD
extern int iWhichAuto;                                                        //golden aoutarm.h
#endif
#ifndef AOUT_IOUTARMZTEACHTASK_FWD
#define AOUT_IOUTARMZTEACHTASK_FWD
extern int iOutArmZTeachTask;                                                 //kevin 20171107 golden AutoTeach.h:235
#endif
// ---- out-arm PLACE-PATH engine surface (golden aoutarm.h / aoutarm9046 16/32-site
//      overloads).  These are the symbols used by SetOutArm / DoMoveOutArmXYToPlace /
//      SearchUnLoadTrayUpDown / DoPlaceToAuto and are NOT yet present in the cpp tree;
//      declared with the EXACT arity used by the 2x8_32 call sites.  Offline-safe:
//      genuine engine link externs / globals (REPORTED for serial Integrate). --------
#ifndef AOUT_BOUTARMSUCKACTIVE_FWD
#define AOUT_BOUTARMSUCKACTIVE_FWD
extern bool bOutArmSuckActive[MAX_ARM_Row][MAX_ARM_Col];                      //golden aoutarm.h:10 -- offline: false grid
#endif
#ifndef AOUT_IOUTARMXPOSITION_FWD
#define AOUT_IOUTARMXPOSITION_FWD
extern int iOutArmXPosition[MAX_ARM_Row][MAX_ARM_Col];                        //golden aoutarm.h:87 -- offline: 0 grid
#endif
#ifndef AOUT_IOUTARMYPOSITION_FWD
#define AOUT_IOUTARMYPOSITION_FWD
extern int iOutArmYPosition;                                                  //golden aoutarm.h:88 -- offline: 0 (aoutarm_shims.cpp)
#endif
#ifndef AOUT_IOUTARMPLACEORDER_FWD
#define AOUT_IOUTARMPLACEORDER_FWD
extern int iOutArmPlaceOrder;                                                 //golden aoutarm.h:16 -- offline: 0
#endif
#ifndef AOUT_BOVERTRAY_FWD
#define AOUT_BOVERTRAY_FWD
extern bool bOverTray;                                                        //golden aoutarm.h:32 / jou 2012-09-04 -- offline: false
#endif
#ifndef AOUT_BUSEONEBYONE_FWD
#define AOUT_BUSEONEBYONE_FWD
extern bool bUseOnebyOne;                                                     //golden aoutarm.h:99 / 2010-12-07 -- offline: false
#endif
#ifndef AOUT_BOUTARMXOVERLIMIT_FWD
#define AOUT_BOUTARMXOVERLIMIT_FWD
extern bool bOutArmXOverLimit;                                                //golden aoutarm.h:24 / Steven 20171206 -- offline: false
#endif
#ifndef AOUT_IBINBOXPOS_FWD
#define AOUT_IBINBOXPOS_FWD
extern int iBinBoxPos;                                                        //golden aoutarm.h / kevin 20160906 -- offline: bin-box place index
#endif
#ifndef AOUT_IMAGPOS_FWD
#define AOUT_IMAGPOS_FWD
extern int iMagPos;                                                           //golden Magazine.h / JerryYang 20221215 -- offline: magazine place index
#endif
#ifndef AOUT_FINDOUTARMPICKERMAXUSECOUNTONTIME_FWD
#define AOUT_FINDOUTARMPICKERMAXUSECOUNTONTIME_FWD
int  Find_OutArm_PickerMaxUseCountOnTime(int iUseSuck, int &iStep, int &iRow, int &iCol, bool bSearchLastMode); //golden aoutarm.h (16/32-site overload) -- offline: pick-count
#endif
#ifndef AOUT_FINDOUTARMSINGLE_FWD
#define AOUT_FINDOUTARMSINGLE_FWD
void Find_OutArm_Single(int iUseSuck, int &iRow, int &iCol);                  //golden aoutarm.h:86 -- offline: single-pick search
#endif
#ifndef AOUT_ADJUSTOUTARMCLOSEPITCHCONDITION_FWD
#define AOUT_ADJUSTOUTARMCLOSEPITCHCONDITION_FWD
void AdjustOutArmClosePitchCondition(int iw, bool &bCanPick2ICAtOnceTime, int &iStep); //golden aoutarm.h (16/32-site overload) -- offline: no-op
#endif
#ifndef AOUT_SEARCHBINBOXUPDOWN_FWD
#define AOUT_SEARCHBINBOXUPDOWN_FWD
void SearchBinBoxUpDown(bool &bCanPick2ICAtOnceTime, int &iStep, int &iRow, int &iCol);  //golden aoutarm.h (16/32-site overload) / Steven 20230601 -- offline: bin-box reorder
#endif
#ifndef AOUT_ICALCULATEOUTARMXPITCH_FWD
#define AOUT_ICALCULATEOUTARMXPITCH_FWD
int  iCalculateOutArmXPitch(int iXPos, int iWhichSuck=-1);                    //golden aoutarm.h / Steven 20151112 / 20160107 -- offline: clamped X-pitch
#endif
#ifndef AOUT_OUTARMADDBLOCKPITCH_FWD
#define AOUT_OUTARMADDBLOCKPITCH_FWD
void OutArmAddBlockPitch(int &nowX, int &nowY, int iRow, int iCol);           //golden aoutarm.h:102 / Frank 20160928 -- offline: no-op
#endif
#ifndef AOUT_TRANSFERAUTORATIO_FWD
#define AOUT_TRANSFERAUTORATIO_FWD
void TransferAutoRatio(int iWhichAuto, int *iXPos, int *iYPos);               //golden aoutarm.h:104 / Steven 20141029 -- offline: identity
#endif
#ifndef AOUT_AUTOCALCOUTARMYCLOSEPITCH_FWD
#define AOUT_AUTOCALCOUTARMYCLOSEPITCH_FWD
int  AutoCalculateOutArmYClosePitch(int iWhichAuto);                          //golden aoutarm.h:111 -- offline: 0 (aoutarm_shims.cpp)
#endif
#ifndef AOUT_AUTOCALCOUTARMCLOSEPITCH_FWD
#define AOUT_AUTOCALCOUTARMCLOSEPITCH_FWD
int  AutoCalculateOutArmClosePitch(int iWhichAuto);                           //golden aoutarm.h -- offline: close-pitch
#endif
// ---- OLD-API cross-module symbols this stale-but-dead golden file references.
//      They are commented-out / absent in the golden 906 headers too (this dead
//      variant pins an old API) and are NOT in the cpp tree -- forward-declared
//      behind #ifndef guards (REPORTED).  Offline-safe: genuine link externs. ----
#ifndef AOUT_XPHNOTSTANDER_FWD
#define AOUT_XPHNOTSTANDER_FWD
extern int XPHNotStander[4];                                                  //golden cmydef (old-API X-pitch-not-standard map {0,2,1,3}) -- offline: column-swap map
#endif
#ifndef AOUT_OUTARMCONTINUOUSMOVE_OLD_FWD
#define AOUT_OUTARMCONTINUOUSMOVE_OLD_FWD
bool OutArmContinuousMove(int X, int Y, int Vari, bool *ZDownSel, int *ZPos, bool ZNeedDown, int YVari, int Vari2); //golden (old 8-arg API, commented out in 906 mymotor) / Steven 20131002 -- offline: continuous-move
#endif
#ifndef AOUT_OUTPUTFORM_FWD
#define AOUT_OUTPUTFORM_FWD
extern TRAY_TYPE_PARA *OutputForm[eTrayCount];                                //golden cprod.h:1369 (commented out) -- offline: per-tray geometry (XPitch/YPitch)
#endif
#ifndef AOUT_FIXFORM_FWD
#define AOUT_FIXFORM_FWD
extern TRAY_TYPE_PARA *FixForm[eTrayCount];                                   //golden cprod.h:1366 (commented out) -- offline: fix-tray geometry (iTrayType)
#endif
// ---- out-arm setup-teach UI predicate (golden cOffSet.h:490 TfOffSet::UseOutArmSetupTeach).
//      The FormsFacade TfOffSet stub exports UseInArmSetupTeach but NOT the OutArm
//      variant; provide a file-local offline-false shim so the golden boolean keeps its
//      VERBATIM shape (offline setup-teach is never engaged -> false), behind a guard. ----
#ifndef AOUT_2x8_32_OUTARMSETUPTEACH_SHIM
#define AOUT_2x8_32_OUTARMSETUPTEACH_SHIM
// AI(W906-ARM1b) 20260927: RETIRED stand-in fOffSet_UseOutArmSetupTeach_2x8_32（回 false）—— TfOffSet::UseOutArmSetupTeach 已照 golden 翻（4b2ec4f3），呼叫點改回 golden 的 fOffSet->UseOutArmSetupTeach
#endif
// ---- place-to-auto / unload-tray index surface (golden cmydef.h:4092 commented out,
//      aoutarm.cpp / atester.cpp / csystem.cpp) -- absent in cpp tree, forward-declared
//      (REPORTED).  Offline-safe: genuine engine link externs / globals. ----
#ifndef AOUT_IUNLOADMOTORINDEX_FWD
#define AOUT_IUNLOADMOTORINDEX_FWD
extern int  iUnloadMotorIndex[eTrayCount];                                    //golden cmydef.h:4092 (commented out) -- offline: per-tray unload motor index
#endif
#ifndef AOUT_CHECKOUTSUCKICFALLDOWN_FWD
#define AOUT_CHECKOUTSUCKICFALLDOWN_FWD
bool CheckOutSuckICFallDown(bool AutoClean=false);                            //golden csystem.h:90 -- offline: false
#endif
#ifndef AOUT_IFUSEONEBYONE_FWD
#define AOUT_IFUSEONEBYONE_FWD
void IfUseOnebyOne(int iAuto);                                                //golden aoutarm.cpp:2621 / Steven 20111101 -- offline: no-op
#endif
#ifndef AOUT_CHECKUSEFIXBINBOXFUNCTION_FWD
#define AOUT_CHECKUSEFIXBINBOXFUNCTION_FWD
bool CheckUseFixBinBoxFunction();                                            //golden aoutarm9045.h:120 / kevin 20160822 -- offline: false
#endif
#ifndef AOUT_INITOUTARMTASK_FWD
#define AOUT_INITOUTARMTASK_FWD
void InitOutArmTask();                                                        //golden aoutarm.h:34 -- offline: reset SM cursor (aHotPlateSubstrate)
#endif
#ifndef AOUT_DOOUTARMPLACETOAUTO_FWD
#define AOUT_DOOUTARMPLACETOAUTO_FWD
bool DoOutArmPlaceToAuto(int iToWhichAuto);                                   //golden aoutarm.cpp:2525 / Steven 20110216 -- offline: place sequence
#endif
#ifndef AOUT_DOPLACETOAUTODELAY_FWD
#define AOUT_DOPLACETOAUTODELAY_FWD
extern TQPF_Timer DoPlaceToAutoDelay;                                        //golden aoutarm.h:28 (HTimer) -- offline: TQPF_Timer
#endif
#ifndef AOUT_GETROWCOL_FWD
#define AOUT_GETROWCOL_FWD
int  GetRowCol(int &iRow, int &iCol);                                         //golden atester.cpp:144 (atester.h:155) -- offline: site row/col
#endif
#ifndef AOUT_BPICKSHUTTLEERROR_FWD
#define AOUT_BPICKSHUTTLEERROR_FWD
extern bool bPickShuttleError;                                                //golden aoutarm.h:30 -- offline: false
#endif
//==============================================================================
#pragma package(smart_init)
//==============================================================================
//extern bool  bScannerAlarm;
//static int OutArmPitchCylinderState=0;
static int iOutArmWhichKit=0;                                                   //golden aoutarm9045_2x8_32.cpp:31 (file-static)
//==============================================================================
// 輸出臂 x y 軸移到 shuttle 1
//  Ver : 2003_07
//==============================================================================
// golden aoutarm9045_2x8_32.cpp:36
int GetNowShuttleMode_2x8_32(int iShuttle)
{
    int i,iKit=0;

    //jou 2012-08-23 增加16 site X Pitch 30 ~ 40 mm,Y Pitch 60 ~ 63.5 mm
    //X 超過 40 mm 會撞機，所以 make 1,2,5,6,7,8

    if(iShuttle==0)
        ptrOutSHT=&FRCarryKit;
    else
        ptrOutSHT=&BRCarryKit;

    if((iShuttle==0 && ptrOutSHT->HasIC()==false) ||
       (iShuttle==1 && ptrOutSHT->HasIC()==false))
    {
        if(XPitchIsStand() && YPitchIsStand())
        {
            return 0;                                                           //X Pitch  = 40, Y Pitch  = 60;
        }
        else if(XPitchIsStand()        && YPitchIsStand()==false)
        {
            return 3;                                                           //X Pitch  = 40, Y Pitch != 60;
        }
    }
    else
    {
        if(ptrOutSHT->LeftSideNoIC(4))
            iKit=4;
        else
            iKit=0;

        if(XPitchIsStand() && YPitchIsStand())
        {
            return 0;
        }
//        else if(XPitchIsStand()==false && YPitchIsStand())
//        {
//            if(ptrOutSHT->Item[0][0+iKit] || ptrOutSHT->Item[0][1+iKit] ||
//               ptrOutSHT->Item[1][0+iKit] || ptrOutSHT->Item[1][1+iKit])
//                return 1;
//            else
//                return 2;
//        }
        else if(XPitchIsStand() && YPitchIsStand()==false)
        {
            for(i=0; i<4; i++)
            {
                if(ptrOutSHT->Item[0][i+iKit])
                    return 3;
            }
            return 4;
        }
//        else
//        {
//            if(ptrOutSHT->Item[0][0+iKit] || ptrOutSHT->Item[0][1+iKit])
//                return 5;          //Suck[0][0] and Suck[0][2] Positon in Shuttle[0][0] Shuttle[0][1]
//            else if(ptrOutSHT->Item[1][0+iKit] || ptrOutSHT->Item[1][1+iKit])
//                return 6;          //Suck[1][0] and Suck[1][2] Positon in Shuttle[1][0] Shuttle[1][1]
//            else if(ptrOutSHT->Item[0][2+iKit] || ptrOutSHT->Item[0][3+iKit])
//                return 7;          //Suck[0][2] and Suck[0][3] Positon in Shuttle[0][2] Shuttle[0][3]
//            else
//                return 8;          //Suck[1][2] and Suck[1][3] Positon in Shuttle[1][2] Shuttle[1][3]
//        }
    }
    return 0;
}
//==============================================================================
// golden aoutarm9045_2x8_32.cpp:103
bool OutArmZNeedDown_2x8_32(int iShuttle)                                       //kevin 20120612 ourarm double pick
{
    if(InitialOutArmNeedSuck(iShuttle)==false)
        return false;

    int i, j, iKit=0;
    int Mode=GetNowShuttleMode_2x8_32(iShuttle);

    for(i=0; i<MAX_ARM_Row; i++)
    {
        for(j=0; j<MAX_ARM_Col; j++)
        {
            if(ptrOutSHT->LeftSideNoIC(4))
                iKit=4;
            else
                iKit=0;

            if(Mode==0)
            {
                if(ptrOutSHT->Item[i][j+iKit])
                    OutArmSuck.Suck[i][j].SetNeedSuck(true);
            }

            if(Mode==1 && (j==0|| j==2))
            {
                if(ptrOutSHT->Item[i][XPHNotStander[j]+iKit])
                    OutArmSuck.Suck[i][j].SetNeedSuck(true);
            }
            else if(Mode==2 && (j==1 || j==3))
            {
                if(ptrOutSHT->Item[i][XPHNotStander[j]+iKit])
                    OutArmSuck.Suck[i][j].SetNeedSuck(true);
            }
            else if(Mode==3 && i==0)
            {
                if(ptrOutSHT->Item[i][j+iKit])
                    OutArmSuck.Suck[i][j].SetNeedSuck(true);
            }
            else if(Mode==4 && i==1)
            {
                if(ptrOutSHT->Item[i][j+iKit])
                    OutArmSuck.Suck[i][j].SetNeedSuck(true);
            }
            else if(Mode==5 && i==0 && (j==0 || j==2))
            {
                if(ptrOutSHT->Item[i][XPHNotStander[j]+iKit])
                    OutArmSuck.Suck[i][j].SetNeedSuck(true);
            }
            else if(Mode==6 && i==1 && (j==0 || j==2))
            {
                if(ptrOutSHT->Item[i][XPHNotStander[j]+iKit])
                    OutArmSuck.Suck[i][j].SetNeedSuck(true);
            }
            else if(Mode==7 && i==0 && (j==1 || j==3))
            {
                if(ptrOutSHT->Item[i][XPHNotStander[j]+iKit])
                    OutArmSuck.Suck[i][j].SetNeedSuck(true);
            }
            else if(Mode==8 && i==1 && (j==1 || j==3))
            {
                if(ptrOutSHT->Item[i][XPHNotStander[j]+iKit])
                    OutArmSuck.Suck[i][j].SetNeedSuck(true);
            }
        }
    }
    return true;                                                                //kevin 20120612 ourarm double pick
}
//==============================================================================
// golden aoutarm9045_2x8_32.cpp:171
void GetOutShuttleStatus_2x8_32(bool *flag)
{
    int i, j;

    for(i=0; i<MAX_ARM_Row; i++)
    {
        for(j=0; j<MAX_ARM_Col; j++)
        {
            if(OutArmSuck.Suck[i][j].GetNeedSuckStatus())
                flag[i+j*2]=true;
            else
                flag[i+j*2]=false;
        }
    }
}
//------------------------------------------------------------------------------
// golden aoutarm9045_2x8_32.cpp:187
bool CheckOutArmXYPitch_2x8_32(int *iX, int *iY, int iShuttle, int iWhichKit)   //kevin 20120612 ourarm double pick fix
{
    int iMode=GetNowShuttleMode_2x8_32(iShuttle);
    int iMovePitchY=GetVariableYOutShuttleData();

    if(iMode==0)
    {
        //jou 2012-08-23 修正16 site 不能設定 X pitch=40
        if(USE_IN_OUT_ARM_Y_PITCH==iXYPitchVariable)                            //ChungHung 20140304 add for AutoYPitch
        {
            *iX-=(TestIF.dSiteXPitch/2+TestIF.dSiteXPitch*2);                   //Steven 20141106 : for AutoYPitch OK
            *iY-=iMovePitchY/2;
        }
        else
        {
            *iX-=TestIF.dSiteXPitch/2+TestIF.dSiteXPitch;
            if(iShuttle==0)
                *iY+=TestIF.dSiteYPitch;
        }
    }
    else if(iMode==1)
    {
        *iX-=TestIF.dSiteXPitch/2;
        if(iShuttle==0)
            *iY+=TestIF.dSiteYPitch;
    }
    else if(iMode==2)
    {
//jou 980723 start : z3(z1~z4) place to shuttle xxx0 error ; inarm x0x0 place to shuttle xx00
//                                              xxxx               x0x0                  xx00
//        *iX+=TestIF.dSiteXPitch+TestIF.dSiteXPitch/2;
        *iX+=TestIF.dSiteXPitch;
//jou 980723 end
        if(iShuttle==0)
            *iY+=TestIF.dSiteYPitch;
    }
    else if(iMode==3)
    {
        if(USE_IN_OUT_ARM_Y_PITCH==iXYPitchVariable)                            //ChungHung 20140304 add for AutoYPitch
        {
            *iX-=TestIF.dSiteXPitch/2-TestIF.dSiteXPitch*2;
            *iY-=iMovePitchY/2;
        }
        else
        {
            *iX-=TestIF.dSiteXPitch/2+TestIF.dSiteXPitch;
            if(iShuttle==0)
                *iY+=TestIF.dSiteYPitch;
        }
    }
    else if(iMode==4)
    {
        if(USE_IN_OUT_ARM_Y_PITCH==iXYPitchVariable)                            //ChungHung 20140304 add for AutoYPitch
        {
            *iX-=TestIF.dSiteXPitch/2-TestIF.dSiteXPitch*2;
            *iY-=iMovePitchY-TestIF.dSiteYPitch/2;
        }
        else
        {
            *iX-=TestIF.dSiteXPitch/2+TestIF.dSiteXPitch;
        }

        if(iShuttle==0)
            *iY+=TestIF.iARM_Y_PITCH;
        else
            *iY+=TestIF.iARM_Y_PITCH-TestIF.dSiteYPitch;
    }
    else if(iMode==5)
    {
        // X pitch & Y pitch no standard
        // use suck 0x0x form shuttle 00xx
        //          xxxx              xxxx

        *iX-=TestIF.dSiteXPitch/2;
        if(iShuttle==0)
            *iY+=TestIF.dSiteYPitch;
    }
    else if(iMode==6)
    {
        // X pitch & Y pitch no standard
        // use suck xxxx form shuttle xxxx
        //          0x0x              00xx

        *iX-=TestIF.dSiteXPitch/2;
        if(iShuttle==0)
            *iY+=TestIF.iARM_Y_PITCH;
        else
            *iY+=TestIF.iARM_Y_PITCH-TestIF.dSiteYPitch;
    }
    else if(iMode==7)
    {
        // X pitch & Y pitch no standard
        // use suck x0x0 form shuttle xx00
        //          xxxx              xxxx

        *iX+=TestIF.dSiteXPitch+TestIF.dSiteXPitch/2;
        *iX-=TestIF.dSiteXPitch/2;
        if(iShuttle==0)
            *iY+=TestIF.dSiteYPitch;
    }
    else
    {
        // X pitch & Y pitch no standard
        // use suck xxxx form shuttle xxxx
        //          x0x0              xx00

        *iX+=TestIF.dSiteXPitch+TestIF.dSiteXPitch/2;
        *iX-=TestIF.dSiteXPitch/2;
        if(iShuttle==0)
            *iY+=TestIF.iARM_Y_PITCH;
        else
            *iY+=TestIF.iARM_Y_PITCH-TestIF.dSiteYPitch;
    }

    if(iMode==0 || iMode==1 || iMode==3 || iMode==5)
    {
        if(iShuttle==0)
        {
            if(IniConfig.bE47_ShuttleUse4Offset &&                              //Steven 20190802 : For 32 Site Out Shuttle Offset
               iWhichKit==1 &&
               TestIF.bEnableAutoAlignment==false)                              //KenHsieh 20220923 : add AOA功能開啟不補償
            {
                *iX+=Prod.XOutArm_Shuttle1_Pick[iOutArmYBase][iOutArmXBase]-OutArmOffSet[OutOfsOutSh1]->GetX()+OutArmOffSet[OutOfsOutSh1RA]->GetX();
                *iY+=Prod.YOutArm_Shuttle1_Pick[iOutArmYBase][iOutArmXBase]-OutArmOffSet[OutOfsOutSh1]->GetY()+OutArmOffSet[OutOfsOutSh1RA]->GetY();
            }
            else
            {
                *iX+=Prod.XOutArm_Shuttle1_Pick[iOutArmYBase][iOutArmXBase];
                *iY+=Prod.YOutArm_Shuttle1_Pick[iOutArmYBase][iOutArmXBase];
            }
        }
        else
        {
            if(IniConfig.bE47_ShuttleUse4Offset &&                              //Steven 20190802 : For 32 Site Out Shuttle Offset
               iWhichKit==1 &&
               TestIF.bEnableAutoAlignment==false)                              //KenHsieh 20220923 : add AOA功能開啟不補償
            {
                *iX+=Prod.XOutArm_Shuttle2_Pick[iOutArmYBase][iOutArmXBase]-OutArmOffSet[OutOfsOutSh2]->GetX()+OutArmOffSet[OutOfsOutSh2RA]->GetX();
                *iY+=Prod.YOutArm_Shuttle2_Pick[iOutArmYBase][iOutArmXBase]-OutArmOffSet[OutOfsOutSh2]->GetY()+OutArmOffSet[OutOfsOutSh2RA]->GetY();
            }
            else
            {
                *iX+=Prod.XOutArm_Shuttle2_Pick[iOutArmYBase][iOutArmXBase];
                *iY+=Prod.YOutArm_Shuttle2_Pick[iOutArmYBase][iOutArmXBase];
            }
        }

        TransferOutShuttleRatio(iShuttle, iX, iY, 0, 2);                        //Steven 20141029 : Out Shuttle的軟體齒輪比
    }
    else if(iMode==2 || iMode==7)
    {
        if(iShuttle==0)
        {
            *iX+=Prod.XOutArm_Shuttle1_Pick[0][1];
            *iY+=Prod.YOutArm_Shuttle1_Pick[0][1];
        }
        else
        {
            *iX+=Prod.XOutArm_Shuttle2_Pick[0][1];
            *iY+=Prod.YOutArm_Shuttle2_Pick[0][1];
        }

        TransferOutShuttleRatio(iShuttle, iX, iY, 0, 1);                        //Steven 20141029 : Out Shuttle的軟體齒輪比
    }
    else if(iMode==4 || iMode==6)
    {
        if(iShuttle==0)
        {
            *iX+=Prod.XOutArm_Shuttle1_Pick[1][0];
            *iY+=Prod.YOutArm_Shuttle1_Pick[1][0];
        }
        else
        {
            *iX+=Prod.XOutArm_Shuttle2_Pick[1][0];
            *iY+=Prod.YOutArm_Shuttle2_Pick[1][0];
        }

        TransferOutShuttleRatio(iShuttle, iX, iY, 1, 0);                        //Steven 20141029 : Out Shuttle的軟體齒輪比
    }
    else
    {
        if(iShuttle==0)
        {
            *iX+=Prod.XOutArm_Shuttle1_Pick[1][1];
            *iY+=Prod.YOutArm_Shuttle1_Pick[1][1];
        }
        else
        {
            *iX+=Prod.XOutArm_Shuttle2_Pick[1][1];
            *iY+=Prod.YOutArm_Shuttle2_Pick[1][1];
        }

        TransferOutShuttleRatio(iShuttle, iX, iY, 1, 1);                        //Steven 20141029 : Out Shuttle的軟體齒輪比
    }

    if(OutArmZNeedDown_2x8_32(iShuttle)==false)                                 //kevin 20120612 ourarm double pick  start
       return false;
    return true;                                                                //kevin 20120612 end
}
//==============================================================================
// golden aoutarm9045_2x8_32.cpp:387
bool MoveOutArmToShuttleIncludeZ_9045_2x8_32(int iShuttle, int iWhichKit, bool bZDown)                                  //kevin 20210602 Out arm add Z bZDown= true 往下
{
    int ZDownPos[8]={0, 0, 0, 0, 0, 0, 0, 0};
    bool ZDownFlag[8]={false, false, false, false, false, false, false, false};
    int i, j;
    int iXPos=0, iYPos=0;
    int iMovePitchX=0;
    int iMovePitchY=0;
    int iXVariable =0;
    int iXVariable2=0;                                                          //ChungHung 20131231 alter AutoYPitch
    int iYVariable =0;                                                          //Steven 20131002 : XY變距

//    if(XPitchIsStand()) //X Pitch is 4000mm
        iMovePitchX=TestIF.dSiteXPitch*3;
//    else
//        iMovePitchX=TestIF.dSiteXPitch+TestIF.dSiteXPitch/2;
    iMovePitchY=GetVariableYOutShuttleData();

    iXVariable=GetOutArmPitch_9045(iMovePitchX);

    if(USE_IN_OUT_ARM_Y_PITCH==iXYPitchVariable)                                //ChungHung 20131231 alter AutoYPitch
    {
        iYVariable =GetOutArmPitchY_9045(iMovePitchY);
        iXVariable2=GetOutArmPitch2_9045(iMovePitchX);
    }

    //jou 2010-12-20 Pitch & Z 縮減為一個
    if(IniConfig.bE34InOutArmPitchZOffsetSameOne==true)
    {
        iXVariable+=OutArmOffSet[OutOfsOutSh1]->GetVariable();
        if(USE_IN_OUT_ARM_Y_PITCH==iXYPitchVariable)                            //ChungHung 20131231 alter AutoYPitch
        {
            iYVariable +=OutArmOffSet[OutOfsOutSh1]->GetVariableY();
            iXVariable2+=OutArmOffSet[OutOfsOutSh1]->GetVariable2();
        }
    }
    else
    {
        if(IniConfig.bE47_ShuttleUse4Offset &&                                  //RA   kevin 20200519 add offset
           iWhichKit==1 &&
           TestIF.bEnableAutoAlignment==false)                                  //KenHsieh 20220923 : add AOA功能開啟不補償
        {
            if(iShuttle==0)
            {
                iXVariable +=OutArmOffSet[OutOfsOutSh1RA]->GetVariable();
                iXVariable2+=OutArmOffSet[OutOfsOutSh1RA]->GetVariable2();
                iYVariable +=OutArmOffSet[OutOfsOutSh1RA]->GetVariableY();
            }
            else
            {
                iXVariable +=OutArmOffSet[OutOfsOutSh2RA]->GetVariable();
                iXVariable2+=OutArmOffSet[OutOfsOutSh2RA]->GetVariable2();
                iYVariable +=OutArmOffSet[OutOfsOutSh2RA]->GetVariableY();
            }
        }
        else
        {
            if(iShuttle==0)
            {
                iXVariable+=OutArmOffSet[OutOfsOutSh1]->GetVariable();
                if(USE_IN_OUT_ARM_Y_PITCH==iXYPitchVariable)                    //ChungHung 20131231 alter AutoYPitch
                {
                    iYVariable+=OutArmOffSet[OutOfsOutSh1]->GetVariableY();
                    iXVariable2+=OutArmOffSet[OutOfsOutSh1]->GetVariable2();
                }
            }
            else
            {
                iXVariable+=OutArmOffSet[OutOfsOutSh2]->GetVariable();
                if(USE_IN_OUT_ARM_Y_PITCH==iXYPitchVariable)                    //ChungHung 20131231 alter AutoYPitch
                {
                    iYVariable+=OutArmOffSet[OutOfsOutSh2]->GetVariableY();
                    iXVariable2+=OutArmOffSet[OutOfsOutSh2]->GetVariable2();
                }
            }
        }
    }

    iXPos+=iOutArmShtXCenterPos;                                                //Steven 20141029 : XY-Pitch for Shuttle Center position for base Suck
    iYPos+=iOutArmShtYCenterPos;

    if(TestIF_File.iYPitchOffsetMode==0)                                        //Steven 20201013 : NN mode add Y-Pitch offset selection
    {
        if(USE_IN_OUT_ARM_Y_PITCH==iXYPitchVariable)                            //ChungHung 20140304 add for AutoYPitch
        {
            //jou 2016-05-05 修正 32 Sites In & Out Arm 吸放點位異常
            if(iShuttle==0)                                                     //Steven 20140512 : For HT-9047
            {
                iYPos+=7000.0;
                iYPos-=((TestIF.dSiteYOffset/2.0)+(GetVariableYInShuttleData()/2.0));
            }
            else
            {
                iYPos-=7000.0;
                iYPos+=((TestIF.dSiteYOffset/2.0)+(GetVariableYInShuttleData()/2.0));
            }
        }
        else
        {
            if(iShuttle==0)                                                     //Steven 20140512 : For HT-9047
            {
                iYPos+=TestIF.dSiteYOffset;
            }
            else
            {
                iYPos-=TestIF.dSiteYOffset;
            }
        }
    }

    if(CheckOutArmXYPitch_2x8_32(&iXPos, &iYPos, iShuttle, iWhichKit)==false && bZDown==true)                           //kevin 20120612 ourarm double pick
         return false;

    //jou 2012-08-23 修正16 site 不能設定 X pitch=40
    if(iWhichKit!=0)
        iXPos+=TestIF.dSiteXPitch*4;

    GetOutShuttleStatus_2x8_32(&ZDownFlag[0]);                                  //一定要在CheckXYPitch()function後面
    for(i=0; i<MAX_ARM_Row; i++)
    {
        for(j=0; j<MAX_ARM_Col; j++)
        {
            if(iShuttle==0)
                ZDownPos[i+j*2]=Prod.ZOutArm_Shuttle1_Pick[i][j];
            else
                ZDownPos[i+j*2]=Prod.ZOutArm_Shuttle2_Pick[i][j];
        }
    }
    //2013-08-02    Dell     add Pick 前先確認CheckInArmDestroyActive 已完成
    if(IsCheckOutArmDestroyActiveFinish(iXPos, iYPos)==false)
        return false;

    DoOutArmSuckPreOn(iShuttle, (iWhichKit==0)?0:4, iXPos, iYPos, bZDown);
    if(OutArmContinuousMove(iXPos, iYPos, iXVariable, ZDownFlag, ZDownPos, bZDown, iYVariable, iXVariable2))            //kevin 20210612 Out arm add Z bZDown= true 往下
        return true;

    return false;
}
//==============================================================================
// 輸出臂從 Shuttle 1 吸取 IC 流程動作
//  Ver : 2003_07
//==============================================================================
// golden aoutarm9045_2x8_32.cpp:529
bool DoPickFromShuttle1_9045_2x8_32(bool IncludeSHT)
{
    int &Task=iPickFromShuttle1Task, ret, i, j;
    static bool bSuckDuplicateErr[2][4]={{false, false, false, false}, {false, false, false, false}};                   //Steven 20091218 : Avoid duplicate message
//    static bool bRecordSkipData[2][4]={{false, false, false, false}, {false, false, false, false}};  //jou 2010-04-29 start : skip兩次會清掉後面八顆
    static int iRetryCT=0,iWitchErrBin=0;                                       //kevin 20160906

    AnsiString ErrPart="";                                                      //Steven 20110216 : 合併Alarm
    bool bHasDuplicateErr=false;                                                //Steven 20110216 : 合併Alarm
    bool bHasErr=false;                                                         //Steven 20110216 : 合併Alarm
    bool flag=false;
    int SHTHasIC[MAX_Index_Row][MAX_Index_Col];
    int iSuckX, iSuckY, iShtX, iShtY;
    static bool bDoLeftShuttle=true;

    if(CheckTesterZ(1))                                                         // && Task!=iOUTARM_SUCK)
    {
        //MoveOutArmToAutoSafe();           //Steven 20150408 : Mark,避免吸嘴上上下下
        return false;
    }

    switch(Task)
    {
        case 1:
            if(ArmSpeed[OutArm].dWaitOnSH!=0)                                                                           //kevin 20210525 out arm 起動在shuttle 上等待 時間
            {
                if(iOutArmWhichKit==0 && OutArmSuck.NoIC())                                                             //kevin 20210615 第一次 吸取IC 需等待
                {
                    if(MoveOutArmToShuttleIncludeZ_9045_2x8_32(0, iOutArmWhichKit, false))                              //移到 Shuttle 上等待
                    {
                        PickFromShuttle(iOutArmWhichKit);                                                               //kevin 20210612 add int iwhichKit=0
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
            if(bOutShtwaitPick())                                               //kevin 20210602 Out arm sht on wait       //kevin 20210515 add out arm shuttle 上等待時間
                Task=10;
            break;
        case 10:
            iWitchErrBin=0;                                                     //kevin 20160906
            Task=200;
        case 200:
            if(MoveOutArmToShuttleIncludeZ_9045_2x8_32(0, iOutArmWhichKit, true))                                                                               //kevin 20210717 change function
            {
                if((AccessLevel>=1 || CosFunction.bOPCanPressStepAndTStart) &&                                                                                  //JerryYang 20170417 (wei) OP權限也能按Step跟T.Start
                   (Sen[SnRKManualStep].IsOn() || fOffSet->UseAutoOffsetFunction("Output Shuttle1") || fOffSet->UseOutArmSetupTeach(OutOfsOutSh1)))             //JerryYang 20180921 Setup Teach功能 //jou 2013-08-29 Use Auto Offset Funtion
                {
                    bEnterOffset=false;
                    fMain->Pause(__FUNC__);                                       // translated: golden no-arg Pause() -> FormsFacade Pause(AnsiString) sibling convention
                    Task=500;
                    break;
                }
                OutArmSuck.ResetAll();                                                                                                                          //Steven 20160323 : 避免未開啟真空
                Task=iOUTARM_SUCK;
            }
            break;
        case 500:
            if(bEnterOffset)
            {
                bEnterOffset=false;
                bOutArmManualStepPress=true;
                Task=1;
                return false;
            }
            else
            {
                Task=iOUTARM_SUCK;
                OutArmSuck.ResetAll();                                          //Steven 20160323 : 避免未開啟真空
            }
            break;
        case iOUTARM_SUCK:
            flag=true;
            if(iOutArmWhichKit==1)
            {
                bDoLeftShuttle=false;
            }
            else
            {
                bDoLeftShuttle=true;
            }

            for(i=0; i<2; i++)
            {
                for(j=0; j<8; j++)
                {
                    SHTHasIC[i][j]=FRCarryKit.Item[i][j];
                }
            }

            bHasErr=false;
            for(i=0; i<MAX_ARM_Row; i++)
            {
                for(j=0; j<MAX_ARM_Col; j++)
                {
                    iSuckX  =i;
                    iSuckY  =j;
                    iShtX   =i;
                    if(iOutArmWhichKit==1)
                        iShtY=j+4;
                    else
                        iShtY=j;

                    if(SHTHasIC[iShtX][iShtY] && OutArmSuck.Suck[iSuckX][iSuckY].GetNeedSuckStatus())
                    {
                        if(OutArmSuck.Suck[iSuckX][iSuckY].Suck())
                        {
                            //模擬ic不見了
//                            if(i==0 && j==2)
//                            {
//                                OutArmSuck.Suck[0][2].Error=true;
//                                continue;
//                            }
                            bHasErr=SwapShuttleDataToOutArm(0, iShtX, iShtY, iSuckX, iSuckY, &iWitchErrBin);            //Steven 20170428 : 統一Out Arm從Shuttle吸料後的資料交換
                            bSuckDuplicateErr[iSuckX][iSuckY]=false;
                        }
                        else
                        {
                            if(OutArmSuck.Suck[iSuckX][iSuckY].Error==false)
                                flag=false;
                        }
                    }
                }
            }

            if(bHasErr)                                                                                                 //Steven 20110527
            {
                ShowOutputShuttleDataMiss();
            }

            if(flag==false)                                                                                             //KEVIN 20130421 按RETRY資料尚未交換會將其他IC丟掉
                break;

            for(i=0; i<MAX_ARM_Row; i++)
            {
                for(j=0; j<MAX_ARM_Col; j++)
                {
                    if(OutArmSuck.Suck[i][j].Error)
                    {
                        iRetryCT++;
                        Task=2000;
                        return false;
                    }
                }
            }

            //if(flag==false)
            //    break;

            for(i=0; i<MAX_ARM_Row; i++)
                for(j=0; j<MAX_ARM_Col; j++)
                    if(OutArmSuck.Item[i][j]==NULL_IC && OutArmSuck.Suck[i][j].GetNeedSuckStatus())
                        return false;

            if(iHWFix_BinBox==1 && ErrorBinBoxDetect(iWitchErrBin)==false)
                return false;                                                                                           //kevin 20160906 add error bin box

            iRetryCT=0;
            Task=1;
            return true;
        case 2000:
            if(MoveOutArmToAutoSafe())
            {
                //Steven 20110216 Start : 整合Alarm
                bHasErr=false;
                ErrPart=" ";
                bHasDuplicateErr=false;
                for(i=0; i<MAX_ARM_Row; i++)
                {
                    for(j=0; j<MAX_ARM_Col; j++)
                    {
                        if(bSuckDuplicateErr[i][j])
                            bHasDuplicateErr=true;
                        if(OutArmSuck.Suck[i][j].Error)
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

                if(bHasErr)
                {
                    if(iRetryCT>ArmSpeed[OutArm].iRetryCT)
                    {
                        if(!MoveOutArmXY_ToShuttleAlarmArea())
                            return false;

                        ret=OutArmPickShuttleAlarm(0, bHasDuplicateErr, ErrPart);                                       //Steven 20141121 : 整合OutArm吸取異常

                        for(i=0; i<MAX_ARM_Row; i++)
                        {
                            for(j=0; j<MAX_ARM_Col; j++)
                            {
                                if(OutArmSuck.Suck[i][j].Error)
                                {
                                    bSuckDuplicateErr[i][j]=true;
                                }
                                else
                                {
                                    bSuckDuplicateErr[i][j]=false;
                                }
                            }
                        }
//                        iRetryCT=0;
                    }
                    else
                    {
                        ret=K_RETRY;
                    }

                    if(ret==K_SKIP)
                    {
                        for(i=0; i<MAX_ARM_Row; i++)
                        {
                            for(j=0; j<MAX_ARM_Col; j++)
                            {
                                if(OutArmSuck.Suck[i][j].Error)
                                {
                                    if(bDoLeftShuttle==true)
                                        iShtY=j;
                                    else
                                        iShtY=j+4;
                                    PorcessJAM0201OutArmPickUpErrorSkip(0, i, iShtY, i, j);                             //Steven 20161214 : 加上OutArm吸取異常Skip的ErrorLog
                                    bSuckDuplicateErr[i][j]=false;
                                }
                            }
                        }
                        iRetryCT=0;
                        if((bDoLeftShuttle==true  && FRCarryKit.LeftSideNoIC(4)) ||
                           (bDoLeftShuttle==false && FRCarryKit.RightSideNoIC(4)))                                      //Steven 20170327 (Jou) : 修改Out放料判斷方式
                        {
                            Task=1;
                            return true;
                        }
                    }
                    else                                                                                                //Steven 20190313 : Fixed for out arm pick error need home
                    {
                        if(iRetryCT>ArmSpeed[OutArm].iRetryCT)
                        {
                            iRetryCT=0;
                            if(IniConfig.bInOutArmCanPushHome)                                                          //ChungHung HT9045 2011/12/13 //Input pickup device error時,按"retry"鍵,機台都會自動home
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
                    for(i=0; i<MAX_ARM_Row; i++)
                        for(j=0; j<MAX_ARM_Col; j++)
                            bSuckDuplicateErr[i][j]=false;
                }
                //Steven 20110216 End
                Task=1;
            }
            break;
        case 2200:
            SetOutArmHome();
            iRetryCT=0;
            Task=1;
            break;
    }
    return false;
}
//------------------------------------------------------------------------------
// golden aoutarm9045_2x8_32.cpp:820
bool DoPickFromShuttle2_9045_2x8_32(bool IncludeSHT)
{
    int &Task=iPickFromShuttle2Task, i, ret;
    static bool bSuckDuplicateErr[2][4]={{false, false, false, false}, {false, false, false, false}};                   //Steven 20091218 : Avoid duplicate message
    static int iRetryCT=0, iWitchErrBin=0;                                      //kevin 20160906
    AnsiString ErrPart="";                                                      //Steven 20110216 : 合併Alarm
    bool bHasDuplicateErr=false;                                                //Steven 20110216 : 合併Alarm
    bool bHasErr=false;                                                         //Steven 20110216 : 合併Alarm
    bool flag=false;
    int j;
    int SHTHasIC[MAX_Index_Row][MAX_Index_Col];
    static bool bDoLeftShuttle=true;
    int iSuckX, iSuckY, iShtX, iShtY;

    if(CheckTesterZ(2))                                                         // && Task!=iOUTARM_SUCK)
    {
        //MoveOutArmToAutoSafe();           //Steven 20150408 : Mark,避免吸嘴上上下下
        return false;
    }

    switch(Task)
    {
        case 1:
            if(ArmSpeed[OutArm].dWaitOnSH!=0)                                                                           //kevin 20210525 out arm 起動在shuttle 上等待 時間
            {
                if(iOutArmWhichKit==0 && OutArmSuck.NoIC())                                                             //kevin 20210615 第一次 吸取IC 需等待
                {
                    if(MoveOutArmToShuttleIncludeZ_9045_2x8_32(1, iOutArmWhichKit,false))                               //移到 Shuttle 上等待
                    {
                        PickFromShuttle(iOutArmWhichKit);                                                               //kevin 20210612 add int iwhichKit=0
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
            if(bOutShtwaitPick())                                               //kevin 20210602 Out arm sht on wait       //kevin 20210515 add out arm shuttle 上等待時間
                Task=10;
            break;
        case 10:
            iWitchErrBin=0;                                                     //kevin 20160906
            Task=200;
        case 200:
            if(MoveOutArmToShuttleIncludeZ_9045_2x8_32(1, iOutArmWhichKit, true))                                                                               //kevin 20210717 change function
            {
                if((AccessLevel>=1 || CosFunction.bOPCanPressStepAndTStart) &&                                                                                  //JerryYang 20170417 (wei) OP權限也能按Step跟T.Start
                   (Sen[SnRKManualStep].IsOn() || fOffSet->UseAutoOffsetFunction("Output Shuttle2") || fOffSet->UseOutArmSetupTeach(OutOfsOutSh2)))             //JerryYang 20180921 Setup Teach功能 //jou 2013-08-29 Use Auto Offset Funtion
                {
                    bEnterOffset=false;
                    fMain->Pause(__FUNC__);                                       // translated: golden no-arg Pause() -> FormsFacade Pause(AnsiString) sibling convention
                    Task=500;
                    break;
                }
                OutArmSuck.ResetAll();                                                                                                                          //Steven 20160323 : 避免未開啟真空
                Task=iOUTARM_SUCK;
            }
            break;
        case 500:
            if(bEnterOffset)
            {
                bOutArmManualStepPress=true;                                    //Eliot 2009_0219
                Task=1;
                bEnterOffset=false;
                return false;
            }
            else
            {
                Task=iOUTARM_SUCK;
                OutArmSuck.ResetAll();                                          //Steven 20160323 : 避免未開啟真空
            }
            break;
        case iOUTARM_SUCK:
            flag=true;
            if(iOutArmWhichKit==1)                                                                                      //Steven 20170327 (Jou) : 修改Out放料判斷方式
            {
                bDoLeftShuttle=false;
            }
            else
            {
                bDoLeftShuttle=true;
            }

            for(i=0; i<2; i++)
            {
                for(j=0; j<8; j++)
                {
                    SHTHasIC[i][j]=BRCarryKit.Item[i][j];
                }
            }

            bHasErr=false;
            for(i=0; i<MAX_ARM_Row; i++)
            {
                for(j=0; j<MAX_ARM_Col; j++)
                {
                    iSuckX  =i;
                    iSuckY  =j;
                    iShtX   =i;
                    if(iOutArmWhichKit==1)                                                                              //Steven 20170327 (Jou) : 修改Out放料判斷方式
                        iShtY=j+4;
                    else
                        iShtY=j;

                    if(SHTHasIC[iShtX][iShtY] && OutArmSuck.Suck[iSuckX][iSuckY].GetNeedSuckStatus())
                    {
                        if(OutArmSuck.Suck[iSuckX][iSuckY].Suck())
                        {
/*
                            //模擬ic不見了 start
                            if(i==0 && j==0)
                            {
                                OutArmSuck.Suck[0][0].Error=true;
                                continue;
                            }
                            //模擬ic不見了 end
*/
                            bHasErr=SwapShuttleDataToOutArm(1, iShtX, iShtY, iSuckX, iSuckY, &iWitchErrBin);            //Steven 20170428 : 統一Out Arm從Shuttle吸料後的資料交換
                            bSuckDuplicateErr[iSuckX][iSuckY]=false;
                        }
                        else if(OutArmSuck.Suck[iSuckX][iSuckY].Error==false)
                        {
                            flag=false;
                        }
                    }
                }
            }

            if(bHasErr)                                                                                                 //Steven 20110527
            {
                ShowOutputShuttleDataMiss();
            }

            if(flag==false)                                                                                             //KEVIN 20130421 按RETRY資料尚未交換會將其他IC丟掉
                break;

            for(i=0; i<MAX_ARM_Row; i++)
            {
                for(j=0; j<MAX_ARM_Col; j++)
                {
                    if(OutArmSuck.Suck[i][j].Error)
                    {
                        iRetryCT++;
                        Task=2000;
                        return false;
                    }
                }
            }

            iRetryCT=0;
            for(i=0; i<MAX_ARM_Row; i++)
                for(j=0; j<MAX_ARM_Col; j++)
                    if(OutArmSuck.Item[i][j]==NULL_IC && OutArmSuck.Suck[i][j].GetNeedSuckStatus())
                        return false;

            if(iHWFix_BinBox==1 && ErrorBinBoxDetect(iWitchErrBin)==false)
                return false;                                                                                           //kevin 20160906 add error bin box
            Task=1;
            return true;
        case 2000:
            if(MoveOutArmToAutoSafe())
            {
                //Steven 20110216 Start : 整合Alarm
                ErrPart=" ";
                bHasDuplicateErr=false;
                bHasErr=false;
                for(i=0; i<MAX_ARM_Row; i++)
                {
                    for(j=0; j<MAX_ARM_Col; j++)
                    {
                        if(bSuckDuplicateErr[i][j])
                            bHasDuplicateErr=true;

                        if(OutArmSuck.Suck[i][j].Error)
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

                if(bHasErr)
                {
                    if(iRetryCT>ArmSpeed[OutArm].iRetryCT)
                    {
                        if(!MoveOutArmXY_ToShuttleAlarmArea())
                            return false;

                        ret=OutArmPickShuttleAlarm(1, bHasDuplicateErr, ErrPart);                                       //Steven 20141121 : 整合OutArm吸取異常

                        for(i=0; i<MAX_ARM_Row; i++)
                        {
                            for(j=0; j<MAX_ARM_Col; j++)
                            {
                                if(OutArmSuck.Suck[i][j].Error)
                                {
                                    bSuckDuplicateErr[i][j]=true;
                                }
                                else
                                {
                                    bSuckDuplicateErr[i][j]=false;
                                }
                            }
                        }
//                        iRetryCT=0;
                    }
                    else
                    {
                        ret=K_RETRY;
                    }

                    if(ret==K_SKIP)
                    {
                        for(i=0; i<MAX_ARM_Row; i++)
                        {
                            for(j=0; j<MAX_ARM_Col; j++)
                            {
                                if(OutArmSuck.Suck[i][j].Error)
                                {
                                    if(bDoLeftShuttle==true)
                                        iShtY=j;
                                    else
                                        iShtY=j+4;
                                    PorcessJAM0201OutArmPickUpErrorSkip(1, i, iShtY, i, j);                             //Steven 20161214 : 加上OutArm吸取異常Skip的ErrorLog
                                    bSuckDuplicateErr[i][j]=false;
                                }
                            }
                        }

                        iRetryCT=0;
                        if((bDoLeftShuttle==true  && BRCarryKit.LeftSideNoIC(4)) ||
                           (bDoLeftShuttle==false && BRCarryKit.RightSideNoIC(4)))                                      //Steven 20170327 (Jou) : 修改Out放料判斷方式
                        {
                            Task=1;
                            return true;
                        }
                    }
                    else                                                                                                //Steven 20190313 : Fixed for out arm pick error need home
                    {
                        if(iRetryCT>ArmSpeed[OutArm].iRetryCT)
                        {
                            iRetryCT=0;
                            if(IniConfig.bInOutArmCanPushHome)                                                          //ChungHung HT9045 2011/12/13 //Input pickup device error時,按"retry"鍵,機台都會自動home
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
                    for(i=0; i<MAX_ARM_Row; i++)
                        for(j=0; j<MAX_ARM_Col; j++)
                            bSuckDuplicateErr[i][j]=false;
                }
                //Steven 20110216 End
                Task=1;
            }
            break;
        case 2200:
            SetOutArmHome();
            iRetryCT=0;
            Task=1;
            break;
    }
    return false;
}
//==============================================================================
// golden aoutarm9045_2x8_32.cpp:1110
bool DoMoveOutArmXYToPlace_9045_2x8_32(bool &bCanPick2ICAtOnceTime, int &ix, int &iy, int &iStep, bool ReadMove)
{
    int iVariablePara[]={2, 1, 0, -1}, iOutPutTray=0;
    if(USE_IN_OUT_ARM_Y_PITCH==iXYPitchVariable)                                //ChungHung 20131231 alter AutoYPitch
    {
        iVariablePara[0]=1;
        iVariablePara[1]=0;
        iVariablePara[2]=-1;
        iVariablePara[3]=-2;
    }

    int iXPos=0, iYPos=0;
    int iTempXpos=0;
    int iXVariable[X_PITCH_COUNT];
    int iYVariable =0;
    int iMovePitchX=iXpitchMaxX3;
    int iMovePitchY=TestIF.iARM_Y_PITCH;
    int ZDownPos[8]={0, 0, 0, 0, 0, 0, 0, 0};
    bool ZDownFlag[8]={false, false, false, false, false, false, false, false};
    int iOffsetPos=-1;
    ZeroMemory(iXVariable, sizeof(iXVariable));
    ZeroMemory(ZDownFlag, sizeof(ZDownFlag));
    ZeroMemory(ZDownPos, sizeof(ZDownPos));

    if(AUTO3_IS_MAGAZINE==1             &&                                      //JerryYang 20221215 : Magazine把fix區當buffer區功能
       TestIF_File.iMagFixTrayType==1   &&
       iWhichAuto>=iMagPos)
    {
        iOutPutTray=iWhichBuff;
    }
    else
    {
        iOutPutTray=iWhichAuto;
    }

    iMovePitchY=AutoCalculateOutArmYClosePitch(iOutPutTray);

    if(IsTrayArmMoveAvoidOutArmCrash())
    {
        MOT[MOutArmX].PCIL132_StopMotor();
        MOT[MOutArmY].PCIL132_StopMotor();
        return false;
    }

    if((iHWFix_BinBox==1 && iWhichAuto==iBinBoxPos) ||
       (IniConfig.bBinBox &&                                                    //jou 2012-12-11 support Bin Box
        iWhichAuto==iFixRight &&
        FixForm[iBinBoxAtFix]->iTrayType==iBinBoxType))
    {
        iXPos=Prod.iOutArmBinBoxX;
        iYPos=Prod.iOutArmBinBoxY-iBinBoxShiftY*200;

        if(ArmSpeed[OutArm].bVariModeFIX==true || bUseOnebyOne==true)
            iMovePitchX=iXpitchMaxX3;
        else
            iMovePitchX=(iXpitchMaxX2*3)/2;

        for(int i=0; i<MAX_ARM_Row; i++)
        {
            for(int j=0; j<MAX_ARM_Col; j++)
            {
                if(bOutArmSuckActive[i][j])
                {
                    iXPos+=iMovePitchX*iVariablePara[j]/3;
                    break;
                }
            }
        }
    }
    else if(bCanPick2ICAtOnceTime==true)
    {
        iTempXpos=Prod.XStart[iOutPutTray][iOutArmYBase][iOutArmXBase]+ix*OutputForm[iOutPutTray]->XPitch;
        iYPos=Prod.YStart[iOutPutTray][iOutArmYBase][iOutArmXBase]-iy*OutputForm[iOutPutTray]->YPitch;
        iMovePitchX=OutArmClose_Pitch*3;
        if(USE_IN_OUT_ARM_Y_PITCH==iXYPitchVariable)                            //Steven 20160107 start: For auto 1 full place with pick G & H
        {
            iTempXpos+=OutArmClose_Pitch;                                       //吸嘴A
        }
        else
        {
            iTempXpos+=OutArmClose_Pitch*2;
        }

        if(bOutArmSuckActive[iOutArmPlaceOrder][0]==false)
        {
            if(bOutArmSuckActive[iOutArmPlaceOrder][1]==false)
            {
                if(bOutArmSuckActive[iOutArmPlaceOrder][2]==false)
                    iTempXpos-=OutArmClose_Pitch*3;
                else
                    iTempXpos-=OutArmClose_Pitch*2;
            }
            else
            {
                iTempXpos-=OutArmClose_Pitch;
            }
        }

        if(iTempXpos<(MOT[MOutArmX].Motor->PSoftLimitN+10) ||
           iTempXpos>(MOT[MOutArmX].Motor->PSoftLimitP-50))                     //Steven 20141217 : For Auto 1使用G & H吸嘴
        {
            bOutArmXOverLimit=true;                                             //Steven 20171206 (Wei) : In ATK, 修正Out Arm同時放料但是超出軟體極限縮Pitch導致疊料的問題
            return false;
        }
        else
        {
            iXPos=iTempXpos;
            if(ReadMove==false)
                return true;
        }
    }
    else
    {
        for(int i=0; i<MAX_ARM_Row; i++)
        {
            for(int j=0; j<MAX_ARM_Col; j++)
            {
                if(bOutArmSuckActive[i][j])
                {
                    iYPos=Prod.YStart[iOutPutTray][i][j]-iy*OutputForm[iOutPutTray]->YPitch;

                    //jou 2010-09-07 當clean out時,out arm排列自動改為One by one排列
                    if(ArmSpeed[OutArm].bVariModeFIX==true || bUseOnebyOne==true && OutArmClose_Pitch<=2000)
                        iMovePitchX=iXpitchMaxX3;
                    else
                        iMovePitchX=OutArmClose_Pitch*3;

                    if(IniConfig.bFix3PutAllFullIC)                             //ChungHung 20111215 嘗試將Fix3放滿
                    {
                        if(iWhichAuto==iFix3Pos)
                            if(j==2 || j==3)
                                iMovePitchX=iXpitchMaxX3;
                    }

                    iXPos=Prod.XStart[iOutPutTray][i][j]+ix*OutputForm[iOutPutTray]->XPitch+iMovePitchX*iVariablePara[j]/3;
                    if(IniConfig.bUseTrayBlockMode)                             //2014-03-04    Dell    for SPIL WLP Add Tray Block
                        OutArmAddBlockPitch(iXPos, iYPos, ix, iy);

                    if(iXPos<(MOT[MOutArmX].Motor->PSoftLimitN+10) ||
                       iXPos>(MOT[MOutArmX].Motor->PSoftLimitP-50))             //Steven 20141217 : For Auto 1使用G & H吸嘴
                    {
                        if(j==3 && iOutPutTray==0)
                            iMovePitchX=iCalculateOutArmXPitch(iXPos, 3);       //Steven 20160107 : 針對Pick G & H 放到Auto 1最左邊
                        else
                            iMovePitchX=iCalculateOutArmXPitch(iXPos);          //Steven 20151112 : 要判斷IC尺寸, 如果尺寸小於13mm,就不能縮到40mm

                        iXPos=Prod.XStart[iOutPutTray][i][j];
                        iXPos+=ix*OutputForm[iOutPutTray]->XPitch+iMovePitchX*iVariablePara[j]/3;
                    }

                    iXPos=Prod.XStart[iOutPutTray][i][j]+ix*OutputForm[iOutPutTray]->XPitch+iMovePitchX*iVariablePara[j]/3;
                    if(IniConfig.bUseTrayBlockMode)                             //2014-03-04    Dell    for SPIL WLP Add Tray Block
                        OutArmAddBlockPitch(iXPos, iYPos, ix, iy);

                    iOutArmXPosition[i][j]=ix;
                    iOutArmYPosition=iy;
                    for(int k=i; k<MAX_ARM_Row; k++)
                        for(int l=j+1; l<MAX_ARM_Col; l++)
                            bOutArmSuckActive[k][l]=false;
                }
            }
        }
    }

    if(USE_IN_OUT_ARM_Y_PITCH==iXYPitchVariable)                                //Steven 20190520 : Fixed Y-Pitch position error
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
        if(USE_IN_OUT_ARM_Y_PITCH==iXYPitchVariable)
        {
            if(iOutArmPlaceOrder==0)
            {
                iYPos=iYPos+iMovePitchY;
                iMovePitchY=IN_OUT_ARM_Y_PITCH_MIN;
                iYPos=iYPos-iMovePitchY;
                iYVariable=GetInArmPitchY_9045(iMovePitchY);
            }
        }
    }

    TransferAutoRatio(iOutPutTray, &iXPos, &iYPos);                             //Steven 20141029 : Auto的軟體齒輪比

    //jou 2010-02-01 fix 3 吸嘴超過 軟體極限時 , 設成 has null ic
    if(iXPos<(MOT[MOutArmX].Motor->PSoftLimitN+10) || iXPos>(MOT[MOutArmX].Motor->PSoftLimitP-10))
    {
        if(MOT[iUnloadMotorIndex[iOutPutTray]].Tray.Data[ix][iy]==NULL_IC)      //ChungHung 20111215 嘗試將Fix3放滿
        {
            MOT[iUnloadMotorIndex[iOutPutTray]].SetTraySingleData(ix, iy, HAS_NULL_IC);
            ReserveEmptyPoint(iUnloadMotorIndex[iOutPutTray], ix, iy);
        }

        if(iHWFix_BinBox==1 && iWhichAuto==iBinBoxPos)
        {
            ShowErrorMessage("WAR16318", 0, MMSystem, 0, "623939");             //kevin 20210401 超出極限秀ALARM 避免HANG UP
        }
        return false;
    }

    if(IniConfig.bFix3PutAllFullIC)                                             //ChungHung 20111215 嘗試將Fix3放滿
    {
        if(iWhichAuto==iFix3Pos)
            ReversionEmptyPoint();
    }

    if(IniConfig.bE34InOutArmPitchZOffsetSameOne==true)
    {
        if(iWhichAuto>=0 && iWhichAuto<iAutoCnt)
            iOffsetPos=OutOfsAuto1;
        else
            iOffsetPos=OutOfsFix1;
    }
    else
    {
        iOffsetPos=OutOfsAuto1+iOutPutTray;
    }

    iXVariable[0]=GetOutArmPitch_9045(iMovePitchX)+OutArmOffSet[iOffsetPos]->GetVariable();
    if(USE_IN_OUT_ARM_Y_PITCH==iXYPitchVariable)
    {
        iYVariable   =GetOutArmPitchY_9045(iMovePitchY)+OutArmOffSet[iOffsetPos]->GetVariableY();
        iXVariable[1]=GetOutArmPitch2_9045(iMovePitchX)+OutArmOffSet[iOffsetPos]->GetVariable2();
    }

    for(int i=0; i<MAX_ARM_Row; i++)
    {
        for(int j=0; j<MAX_ARM_Col; j++)
        {
            ZDownFlag[i+j*2]=bOutArmSuckActive[i][j];
            ZDownPos [i+j*2]=Prod.ZPlace[iWhichAuto][i][j];
        }
    }

    if(OutArmContinuousMove(iXPos, iYPos, iXVariable[0], ZDownFlag, ZDownPos, true, iYVariable, iXVariable[1]))
    {
        return true;
    }
    return false;
}
//==============================================================================
// golden aoutarm9045_2x8_32.cpp:1358
bool SearchUnLoadTrayUpDown_9045_2x8_32(bool &bCanPick2ICAtOnceTime, int &iStep, int &ix, int &iy)
{
    if(AUTO3_IS_MAGAZINE==1             &&                                      //JerryYang 20221215 : Magazine把fix區當buffer區功能
       TestIF_File.iMagFixTrayType==1   &&
       iWhichAuto>=iMagPos)
    {
        AdjustOutArmClosePitchCondition(iWhichBuff, bCanPick2ICAtOnceTime, iStep);
    }
    else
    {
        AdjustOutArmClosePitchCondition(iWhichAuto, bCanPick2ICAtOnceTime, iStep);
    }

    if(bOutArmXOverLimit)                                                       //Steven 20171206 (Wei) : In ATK, 修正Out Arm同時放料但是超出軟體極限縮Pitch導致疊料的問題
        bCanPick2ICAtOnceTime=false;

    if(bCanPick2ICAtOnceTime==true)
    {
        int ct=Find_OutArm_PickerMaxUseCountOnTime(4, iStep, ix, iy, TestIF.bSearchLastMode);
        if(ct<=1)
        {
            bCanPick2ICAtOnceTime=false;
        }
        else if(iWhichAuto==iAutoRight)
        {
            if(DoMoveOutArmXYToPlace_9045_2x8_32(bCanPick2ICAtOnceTime, ix, iy, iStep, false)==true)
                return true;
            bCanPick2ICAtOnceTime=false;
        }
        else
        {
            SearchBinBoxUpDown(bCanPick2ICAtOnceTime, iStep, ix, iy);           //Steven 20230601 : 重新整理Bin Box
            return true;
        }
    }
    Find_OutArm_Single(4, ix, iy);
    return true;
}
//==============================================================================
// golden aoutarm9045_2x8_32.cpp:1397
bool SetOutArm_9045_2x8_32()
{
    int iStep, ix=0, iy=0;
    bool bCanPick2ICAtOnceTime;
    if(IsTrayArmMoveAvoidOutArmCrash())
    {
        MOT[MOutArmX].PCIL132_StopMotor();
        MOT[MOutArmY].PCIL132_StopMotor();
        return false;
    }

    if(SearchUnLoadTrayUpDown_9045_2x8_32(bCanPick2ICAtOnceTime, iStep, ix, iy)==false)
        return false;

    //jou 2012-09-04 修正極限無法到達時,回填Null_IC會造成找不到地方放,會放在 x=-1,y=-1的位置上
    bOverTray=false;
    if(ix<0 || iy<0)
    {
        bOverTray=true;
        return false;
    }

    if(MOT[MTrayX].ReadPos() > Prod.iXTrayColor)
    {
        MOT[MOutArmY].PCIL132_StopMotor();
        MOT[MOutArmY].PCIL132_StopMotor();
        return false;
    }

    return  DoMoveOutArmXYToPlace_9045_2x8_32(bCanPick2ICAtOnceTime, ix, iy, iStep, true);
}
//------------------------------------------------------------------------------
// golden aoutarm9045_2x8_32.cpp:1429
bool DoPlaceToAuto_9045_2x8_32()
{
    int ct;
    bool flag;
    static int iRetry=0;
    int &Task=iPlaceToAutoTask;
    AnsiString asAutoOffset;

    switch(Task)
    {
        case 1:
            bOutArmXOverLimit=false;                                            //Steven 20171206 (Wei) : In ATK, 修正Out Arm同時放料但是超出軟體極限縮Pitch導致疊料的問題
            InitialFix3CanFullTask();                                           //ChungHung 20140313 add Fix3 can Full Tray
            Task=10;
        case 10:
            if(CheckOutSuckICFallDown(false)==true)                             //Steven 20210916 : 全時檢查out arm掉料
            {
                Task=220;
                return false;
            }

            iWhichAuto=SearchTrayToPlace_9045();                                //Steven 20160524 : 簡化Out Arm程式碼
            IfUseOnebyOne(iWhichAuto);                                          //Steven 20111101 : 整合成function     //Steven 20210608 : 加入iWhichAuto判斷

            if(AUTO3_IS_MAGAZINE==1             &&                              //JerryYang 20221215 : Magazine把fix區當buffer區功能
               TestIF_File.iMagFixTrayType==1   &&
               iWhichAuto>=iMagPos)
            {
                ct=iWhichBuff;
            }
            else
            {
                ct=iWhichAuto;
            }

            if(MOT[iUnloadMotorIndex[iWhichAuto]].fHasTray==false ||
               (MOT[iUnloadMotorIndex[iWhichAuto]].FullIC() &&
                CheckUseFixBinBoxFunction()==false))                            //kevin 20160822
            {
                InitOutArmTask();
                break;
            }

            if(iWhichAuto!=iBinBoxPos &&                                        //kevin 20160912 20160822 add error bin box
               (EnableFix3UseCylinder()))                                       //ChungHung 20140722 add for HT9046LA
            {
                if(UseFix3Cylinder(ct)==false)                                  //ChungHung 20140313 add Fix3 can Full Tray
                    break;
            }

            flag=SetOutArm_9045_2x8_32();
            if(flag)
            {
                Task=50;
            }
            else                                                                //jou 2012-09-04 修正極限無法到達時,回填Null_IC會造成找不到地方放,會放在 x=-1,y=-1的位置上
            {
                if(bOverTray==true)
                {
                    MOT[iUnloadMotorIndex[ct]].SetTray(HAS_IC, __FUNC__);       // translated: golden 1-arg SetTray -> SetTray(int,AnsiString) sibling convention
                    return true;
                }
            }
            break;
        case 50:
            //Steven 20110216 Start : 整合Alarm Code
            if(CheckOutSuckICFallDown(false)==false)
            {
                iRetry=0;
                Task=100;
            }
            else
            {
                Task=220;
                return false;
            }
            //Steven 20110216 End

//jou 980424 add out arm speed
//            break;
        case 100:
            if(iWhichAuto<=iAutoRight)
            {
                if(CosFunction.bNewAutoTrayDetect==true)                                                                                                        //JerryYang 20170822 (Steven) 更改Auto區偵測tray盤時機 避免out arm吸嘴下降時造成誤判
                {
                }
                else
                {
                    if(DetectAutoTray(iWhichAuto, &iRetry)==false)
                    {
                        break;
                    }
                }
                asAutoOffset="Auto"+AnsiString(iWhichAuto+1);                                                                                                   //jou 2013-08-29 Use Auto Offset Funtion
            }
            else
            {
                //jou 2013-08-29 Use Auto Offset Funtion
                ct=iWhichAuto;                                                                                                                                  //QQQ
                if(ct>=6)
                    ct-=3;
                ct-=2;
                asAutoOffset="Fix"+AnsiString(ct);
            }

            if((AccessLevel>=1 || CosFunction.bOPCanPressStepAndTStart) &&                                                                                      //JerryYang 20170417 (wei) OP權限也能按Step跟T.Start
               (Sen[SnRKManualStep].IsOn() || fOffSet->UseAutoOffsetFunction(asAutoOffset) || fOffSet->UseOutArmSetupTeach(OutOfsAuto1+iWhichAuto)))            //JerryYang 20180921 Setup Teach功能 //jou 2013-08-29 Use Auto Offset Funtion
            {
                bEnterOffset=false;
                fMain->Pause(__FUNC__);                                       // translated: golden no-arg Pause() -> FormsFacade Pause(AnsiString) sibling convention
                Task=200;
                break;
            }
//jou 2010-12-01 start : 小IC時,會偶發性置偏
            if(ArmSpeed[OutArm].bSuckOnDown==false && ArmSpeed[OutArm].dDestroyPauseTime!=0)
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
//jou 2010-12-01 end
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
            if(DoOutArmPlaceToAuto(iWhichAuto)==true)                           //Steven 20110216 : 整合AlarmCode
            {
//                iOutArmPickPlaceCnt[OutOfsAuto1+iWhichAuto]++;    //JerryYang 20230523 : 沒在用, Mark掉  //JerryYang 20180921 Setup Teach功能
                Task=400;
            }
            break;
        case 400:
            if(CosFunction.bNewAutoTrayDetect==true && iWhichAuto<=iAutoRight)  //JerryYang 20170822 (Steven) 更改Auto區偵測tray盤時機 避免out arm吸嘴下降時造成誤判
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
                ReversionEmptyPoint();                                          //Eliot 2009_03_06
                Task=1;
                return true;
            }
            break;
        //JerryYang 20170822 (Steven) 更改Auto區偵測tray盤時機 避免out arm吸嘴下降時造成誤判
        //==>
        case 500:
            if(DetectAutoTray(iWhichAuto, &iRetry)==true)
            {
                ReversionEmptyPoint();
                Task=1;
                return true;
            }
        //<==
        //JerryYang 20170822 (Steven) 更改Auto區偵測tray盤時機 避免out arm吸嘴下降時造成誤判
    }
    return false;
}
//==============================================================================
// golden aoutarm9045_2x8_32.cpp:1625
void GetTrayPosition_AccordingBaseSuck_9045_2x8_32(int iToWhichAuto, int iTrayX, int iTrayY, int &iPosX, int &iPosY)    //JimmyChiu 20211020 : Auto alignment mode
{
    iPosX=Prod.XStart[iToWhichAuto][iOutArmYBase][iOutArmXBase]+iTrayX*OutputForm[iToWhichAuto]->XPitch;
    iPosY=Prod.YStart[iToWhichAuto][iOutArmYBase][iOutArmXBase]-iTrayY*OutputForm[iToWhichAuto]->YPitch;
}
//---------------------------------------------------------------------------
///iActiveSuckX/iActiveSuckY = left top active suck
// golden aoutarm9045_2x8_32.cpp:1632
void GetTrayPosition_AccordingTopLeftActiveSuck_9045_2x8_32(int iToWhichAuto, int iActiveSuckX, int iActiveSuckY, int &iPosX, int &iPosY, int &iXVariable, int &iYVariable, int &iXVariable2)  //JimmyChiu 20211020 : Auto alignment mode
{
    int iMovePitchX=0;
    int iMovePitchY=0;
    iMovePitchY=AutoCalculateOutArmYClosePitch(iToWhichAuto);
    int OutArmClose_X_Pitch=AutoCalculateOutArmClosePitch(iToWhichAuto);
    iMovePitchX=OutArmClose_X_Pitch*(MAX_ARM_Col-1);
    iXVariable=GetOutArmPitch_9045(iMovePitchX)+OutArmOffSet[OutOfsAuto1+iToWhichAuto]->GetVariable();
    if(USE_IN_OUT_ARM_Y_PITCH==iXYPitchVariable)                                //ChungHung 20131231 alter AutoYPitch
    {
        iYVariable =GetOutArmPitchY_9045(iMovePitchY)+OutArmOffSet[OutOfsAuto1+iToWhichAuto]->GetVariableY();
        iXVariable2=GetOutArmPitch2_9045(iMovePitchX)+OutArmOffSet[OutOfsAuto1+iToWhichAuto]->GetVariable2();
    }
    //Offset according active suck
    iPosX-=((iActiveSuckX-iOutArmXBase)*OutArmClose_X_Pitch);
    iPosY+=((iActiveSuckY-iOutArmYBase)*iMovePitchY);
}
//---------------------------------------------------------------------------
// golden aoutarm9045_2x8_32.cpp:1650
bool DoMoveOutArmXYToPlaceForAutoTeachOffset_9045_2x8_32(int iwhm, int iTrayX, int iTrayY)                              //JimmyChiu 20211020 : Auto alignment mode
{
    int i, j;
    int iPosX, iPosY, ZDownPos[8]={0, 0, 0, 0, 0, 0, 0, 0};
    bool ZDownFlag[8]={false, false, false, false, false, false, false, false};
    int iXVariable =0;
    int iXVariable2=0;                                                          //ChungHung 20131231 alter AutoYPitch
    int iYVariable =0;                                                          //Steven 20131002 : XY變距
    GetTrayPosition_AccordingBaseSuck_9045_2x8_32(iwhm, iTrayX, iTrayY, iPosX, iPosY);
    GetTrayPosition_AccordingTopLeftActiveSuck_9045_2x8_32(iwhm, 0, 0, iPosX, iPosY, iXVariable, iYVariable, iXVariable2);
    if(IsTrayArmMoveAvoidOutArmCrash())
    {
        MOT[MOutArmX].PCIL132_StopMotor();
        MOT[MOutArmY].PCIL132_StopMotor();
        return false;
    }
    for(i=0; i<MAX_ARM_Row; i++)
    {
        for(j=0; j<MAX_ARM_Col; j++)
        {
            bOutArmSuckActive[0][j]=true;
            iOutArmXPosition[i][j]=j;
            iOutArmYPosition=iTrayY;
            ZDownFlag[i+j*2]=true;
            ZDownPos [i+j*2]=Prod.ZPlace[iwhm][i][j];
        }
    }

    if(OutArmContinuousMove(iPosX, iPosY, iXVariable, ZDownFlag, ZDownPos, true, iYVariable, iXVariable2))              //ChungHung 20131231 alter AutoYPitch
    {
        return true;
    }
    return false;
}
//---------------------------------------------------------------------------
/*
0:unfinish
1:finish without running next step
2:finish and running next step
*/
// golden aoutarm9045_2x8_32.cpp:1690
int DoPlaceToAutoForAutoTeachOffset_9045_2x8_32()                               //JimmyChiu 20211020 : Auto alignment mode
{
    // W7-GATE: the VCL UI-form singleton fAutoTeach (golden AutoTeach.h
    // TfAutoTeach : public TForm) has NO offline FormsFacade stand-in.  Its entire
    // body derefs fAutoTeach->; gated behind #if 0 // TODO(W7).  Offline the auto-
    // alignment teach is never running so this returns 1 = "finish without next step"
    // (verbatim golden early-return when fAutoTeach->IsRun()==false).
    #if 0 // TODO(W7): fAutoTeach form (golden AutoTeach.h TfAutoTeach) -- no FormsFacade stand-in; offline IsRun()==false -> returns 1
    if(fAutoTeach->IsRun()==false)
        return true;
    switch(fAutoTeach->iTaskPlaceToAuto)
    {
        case 0:                                                                 //no 0 ->1
            switch(fAutoTeach->GetState())
            {
                case fAutoTeach->eATFix3:
                    fAutoTeach->iTaskPlaceToAuto=100;
                    fAutoTeach->iWhichAuto=eFix3;
                    break;
                case fAutoTeach->eATAuto1:                                      //need move to fix3 again, cuz 16 sites per shuttle
                    fAutoTeach->iTaskPlaceToAuto=100;
                    fAutoTeach->iWhichAuto=eAuto1;
                    break;
                case fAutoTeach->eATAuto2:
                    fAutoTeach->iTaskPlaceToAuto=100;
                    fAutoTeach->iWhichAuto=eAuto2;
                    break;
                case fAutoTeach->eATAuto3:
                    fAutoTeach->iTaskPlaceToAuto=100;
                    fAutoTeach->iWhichAuto=eAuto3;
                    break;
                case fAutoTeach->eATFix1:
                    fAutoTeach->iTaskPlaceToAuto=100;
                    fAutoTeach->iWhichAuto=eFix1;
                    break;
                case fAutoTeach->eATFix2:
                    fAutoTeach->iTaskPlaceToAuto=100;
                    fAutoTeach->iWhichAuto=eFix2;
                    break;
                default:
                    fAutoTeach->iTaskPlaceToAuto=0;
                    return 2;
            }
            break;
        case 100:
            if(MoveOutArmToAutoSafe())
            {
                fAutoTeach->iTaskPlaceToAuto=200;
            }
            break;
        case 200:
            if(DoMoveOutArmXYToPlaceForAutoTeachOffset_9045_2x8_32(fAutoTeach->iWhichAuto,0,0))
                fAutoTeach->iTaskPlaceToAuto=300;
            break;
        case 300:
            bEnterOffset=false;
            fAutoTeach->iTaskPlaceToAuto=400;
            fMain->Pause(__FUNC__);                                       // translated: golden no-arg Pause() -> FormsFacade Pause(AnsiString) sibling convention
            break;
        case 400:
            if(bEnterOffset==false)
            {
                fAutoTeach->iTaskPlaceToAuto=0;
                if(fAutoTeach->GetState()==fAutoTeach->eATFix3 ||
                   fAutoTeach->GetState()==fAutoTeach->eATFix2)
                    fAutoTeach->iTaskPlaceToAuto=500;
                else
                    fAutoTeach->DoNext();
            }
            else
            {
                bEnterOffset=false;
                fAutoTeach->iTaskPlaceToAuto=100;
            }
            break;
        case 500:
            fAutoTeach->DoNext();
            return 2;
        default:
            return 1;
    }
    return 0;
    #else
    return 1;                                                                   // W7 offline: fAutoTeach not running -> "finish without next step"
    #endif
}
//---------------------------------------------------------------------------
// golden aoutarm9045_2x8_32.cpp:1768
void DoOutArm_9045_2x8_32()
{
    int iRow, iCol;
    int ret, j;
    static int OldPos=0, i;
    static bool FirstEnter=true,bCheckSpeed=false;
    static bool bTrayDuplicateErr[9]={false, false, false, false, false, false, false, false, false};                   //Steven 20120208 : bTrayDuplicateErr_Define
    int &Task=OutArmTask;
    static int iWhichShuttle=0;
    bool flag1, flag2, flag3;
    AnsiString str;
    static bool bCheckSpeed1=false;                                             //KaiChen 20171225 (Steven)：Add Speed Display

    if(bCarryControlOutarm1 || bCarryControlOutarm2)                            //jou 2011-11-21 bCarryControlOutarm -> bCarryControlOutarm1 不應該共用一個flag
    {
        return;
    }

    switch(Task)
    {
        case 1:
            bSortingAllBinTrayFinish=false;                                     //JerryYang 20150910 是否所有的Bin Tray IC都整盤完成
            Task=5;
            break;
        case 5:
            if(MoveOutArmToAutoSafe())
                Task=10;
            break;
        case 10:
            if(CheckOutArmInitState())
                Task=50;
            break;
        case 50:
            FirstEnter=true;                                                    // for judge which shuttle to pick
            IsCatchTrayReadySupplyNewTray();
            if(MoveOutArmToAutoSafe())
            {
//#ifdef Carry4
//                MOT[MOutShuttle1].fCanMoveR=true;
//                MOT[MOutShuttle2].fCanMoveR=true;
//#else
                MOT[MInShuttle1].fCanMoveR=true;
                MOT[MInShuttle2].fCanMoveR=true;
//#endif
                if(OutArmSuck.HasIC())
                    Task=3000;
                else
                    Task=100;
            }
            break;
        case 100:
            IsCatchTrayReadySupplyNewTray();
            if(FirstEnter)
            {
                if(TestIF_File.iShuttleMode==1)                                 //Jou 20150909 : For Add 32Site UPH, 讓Shuttle 2先回去補Input
                {
                    if(FRCarryKit.HasIC() && OutSHT1InRT())
                    {
                        OldPos=1;
                        FirstEnter=false;
                    }
                    else if((BRCarryKit.HasIC()) && OutSHT2InRT())
                    {
                        OldPos=0;
                        FirstEnter=false;
                    }
                }
                else
                {
                    if((BRCarryKit.HasIC()) && OutSHT2InRT())
                    {
                        OldPos=0;
                        FirstEnter=false;
                    }
                    else if(FRCarryKit.HasIC() && OutSHT1InRT())                //jou 20171227 : 修正32 sites hang up
                    {
                        OldPos=1;
                        FirstEnter=false;
                    }
                }
            }

            if(FirstEnter)
            {
                Task=200;
                break;
            }

            if(FRCarryKit.HasIC() && OutSHT1InRT())
            {
                if(OldPos==0 && BRCarryKit.HasIC()==false && OutSHT2InRT()==false)
                    OldPos=1;
            }
            else if(BRCarryKit.HasIC() && OutSHT2InRT())
            {
                if(OldPos==1 && FRCarryKit.HasIC()==false && OutSHT1InRT()==false)
                    OldPos=0;
            }

            if(TestIF.iShuttleMode==1)
            {
                if(TestIF.iShuttle_Sel==0)
                    OldPos=1;                                                   // shuttle 1
                else
                    OldPos=0;                                                   // shuttle 2
            }

            if(CheckRotateOutNotFinish())                                       //kevin 20130524 發生alarm inarm到等待點
            {
                Task=7000;
                break;                                                          //kevin 20130524
            }

            if(OldPos==1)
            {
                iWhichShuttle=0;                                                // shuttle 1
                iOutArmWhichKit=0;                                              //Eliot 2009_12_29
                Task=1000;
            }
            else
            {
                iWhichShuttle=1;                                                // shuttle 2
                iOutArmWhichKit=0;                                              //Eliot 2009_12_29
                Task=2000;
            }
            break;
        case 200:
            if(MoveOutArmToAutoSafe())
                Task=300;
            break;
        case 300:
            if(MoveOutArmXY_ToFix_Tray_Full())                                  //Steven 20230602 : 統一out arm 讓位位置
            {
                Task=CheckOutArmCleanOut();
            }
            break;
        case 310:                                                               //kevin 20171103 (wei) Outarm Z Auto High
            if(AutoTeachLoadTrayZ(false, OutArm, iOutArmZTeachTask))
            {
                MoveOutArmXY_ToFix_Tray_Full();
                Task=311;
            }
            break;
        case 311:                                                               //kevin 20171103 (wei) Outarm Z Auto High
            if(MoveOutArmXY_ToFix_Tray_Full())
            {
                bOutarmAutoHigh=false;
                Task=1;
            }
            break;
        case 1000:
            if(USE_IN_OUT_ARM_Y_PITCH==iXYPitchVariable && IniConfig.bE57YPitchHome)                                    //kevin 20180827 (Steven) : add put shuttle Y pitch home AutoYPitch
            {
                Task=1010;
                return;
            }
            Task=1140;
            break;
       case 1010:
            if(MoveOutArmToAutoSafe())
            {
                hOutArmYpitchHomeTimer.SetSecAndOn(10);
                Task=1020;
            }
            break;
       case 1020:
            if(bCheckYPitchRunHomeSen(1))
            {
                 Task=1140;
            }
            else if(hOutArmYpitchHomeTimer.Off())
            {
                InitProcessSingleMotorTask(MOutArmPitchY);
                hOutArmYpitchHomeTimer.SetSecAndOn(30);
                bYpitchNeddHome=true;
                Task=1030;
            }
            break;
       case 1030:                                                               //Ypitch home
            if(bCheckYPitchHome(1))
            {
                SetMotorSpeed();
                bYpitchNeddHome=false;
                 Task=1140;
            }
            else if(hOutArmYpitchHomeTimer.Off())
            {
                ShowErrorMessage("WAR0227", K_RETRY, MInArmPitchY);
                Task=1020;
            }
            break;
       case 1140:
            if((iOneCycle==1 || iCleanOut==1) &&
                FTestSuck.HasIC()==false && FRCarryKit.HasIC()==false &&
                (BRCarryKit.HasIC() || BTestSuck.HasIC()))
            {
                Task=50;
                break;
            }

            if(FRCarryKit.HasIC()==false && OutSHT1InRT() && BRCarryKit.HasIC() && OutSHT2InRT())
            {
                    Task=50;
                    break;
            }

            if((iCleanOut==1 && HasICUnderMachine()==false) ||
               (iCleanOut==1 && LastSet.iTemperature==Tempture_Hot && (TrayForm.bAutoFeed==false && bMustCleanAllTray==false) && HasICUnderHotPlate()==false))  //JerryYang 20190807 fix 快速clean out hang up
            {
                FirstEnter=true;
                Task=1;
                break;
            }
            //Eliot 2009_12_29 start
            if((FRCarryKit.Item[0][0]!=NULL_IC && FRCarryKit.Item[0][0]!=HAS_NULL_IC) ||
                (FRCarryKit.Item[0][1]!=NULL_IC && FRCarryKit.Item[0][1]!=HAS_NULL_IC) ||
                (FRCarryKit.Item[0][2]!=NULL_IC && FRCarryKit.Item[0][2]!=HAS_NULL_IC) ||
                (FRCarryKit.Item[0][3]!=NULL_IC && FRCarryKit.Item[0][3]!=HAS_NULL_IC) ||
                (FRCarryKit.Item[1][0]!=NULL_IC && FRCarryKit.Item[1][0]!=HAS_NULL_IC) ||
                (FRCarryKit.Item[1][1]!=NULL_IC && FRCarryKit.Item[1][1]!=HAS_NULL_IC) ||
                (FRCarryKit.Item[1][2]!=NULL_IC && FRCarryKit.Item[1][2]!=HAS_NULL_IC) ||
                (FRCarryKit.Item[1][3]!=NULL_IC && FRCarryKit.Item[1][3]!=HAS_NULL_IC))
            {
                iOutArmWhichKit=0;
            }
            else if((FRCarryKit.Item[0][4]!=NULL_IC && FRCarryKit.Item[0][4]!=HAS_NULL_IC) ||
                (FRCarryKit.Item[0][5]!=NULL_IC && FRCarryKit.Item[0][5]!=HAS_NULL_IC) ||
                (FRCarryKit.Item[0][6]!=NULL_IC && FRCarryKit.Item[0][6]!=HAS_NULL_IC) ||
                (FRCarryKit.Item[0][7]!=NULL_IC && FRCarryKit.Item[0][7]!=HAS_NULL_IC) ||
                (FRCarryKit.Item[1][4]!=NULL_IC && FRCarryKit.Item[1][4]!=HAS_NULL_IC) ||
                (FRCarryKit.Item[1][5]!=NULL_IC && FRCarryKit.Item[1][5]!=HAS_NULL_IC) ||
                (FRCarryKit.Item[1][6]!=NULL_IC && FRCarryKit.Item[1][6]!=HAS_NULL_IC) ||
                (FRCarryKit.Item[1][7]!=NULL_IC && FRCarryKit.Item[1][7]!=HAS_NULL_IC))
            {
                iOutArmWhichKit=1;
            }
            //Eliot 2009_12_29 end
            if(OutSHT1InRT() && bCheckShuttle1Flag==false)
            {
                if(FRCarryKit.HasIC())
                {
                    if(CheckShuttleICPos(1))
                    {
                        Task=1150;
                    }
                    else
                    {
                        InitPickFromShuttle1Task();                                                                                                             //Sam 20230208 : 修正 Fix AOI 未拍照問題
                        //iPickFromShuttle1Task=1;
                        if(FRCarryKit.LeftSideNoIC(4))
                            Task=1230;
                        else
                            Task=1200;
//                        #ifdef Carry4
//                            MOT[MOutShuttle1].fCanMoveR=false;
//                        #else
                            MOT[MInShuttle1].fCanMoveR=false;
//                        #endif
                    }
                }
                else
                {
                    Task=1150;
                }
            }
            else
            {
                Task=1150;
            }
            break;
        case 1150:
            if(bCheckSpeed)                                                                                                                                     //Steven 20110525 : Auto Speed
            {
                bCheckSpeed=false;
                OutArmSubSpeed();
            }
            //KaiChen 20171225 (Steven)：Add Speed Display
            //==>
            if(bCheckSpeed1)
            {
                bCheckSpeed1=false;
                OutArmSubSpeedDisplay();
            }
            //<==
            //KaiChen 20171225 (Steven)：Add Speed Display

            if(MoveOutArmToShuttleIncludeZ_9045_2x8_32(iWhichShuttle, iOutArmWhichKit, false))
            {
                IsCatchTrayReadySupplyNewTray();
                //KaiChen 20171225 (Steven)：Add Speed Display
                //==>
                if(IniConfig.bA26MotorSpeedSortDisplay)
                    bCheckSpeed1=true;
                else
                    bCheckSpeed1=false;
                //<==
                //KaiChen 20171225 (Steven)：Add Speed Display
                Task=1100;
            }
            break;
        case 1100:
            IsCatchTrayReadySupplyNewTray();

            if(WhichAutoNeedTray()!=0)                                                                                                                          //JerryYang 20170731 (wei) Out arm要讓位才不會hang up
            {
                //==> Eastsun 20260521 整合
                if(IniConfig.bA15_1ESDGiveWayFunction==true && USE_BU5_Function==true)
                {
                    MoveOutArmXY_To_ESDSafePos();
                    Task=1170;
                }
                else
                {
                    MoveOutArmXY_ToFix_Tray_Full();                                                                                                                 //Steven 20231027 : 先檢查有沒有要換Tray
                    Task=1160;
                }
                //<== Eastsun 20260521 整合
                }
            else if(FRCarryKit.HasIC() && OutSHT1InRT() && bCheckShuttle1Flag==false)
            {
//#ifdef Carry4
//                MOT[MOutShuttle1].fCanMoveR=false;
//#else
                MOT[MInShuttle1].fCanMoveR=false;
//#endif
                InitPickFromShuttle1Task();
                //iPickFromShuttle1Task=1;

                if(FRCarryKit.LeftSideNoIC(4))
                {
                    iOutArmWhichKit=1;
                    Task=1230;
                }
                else
                {
                    iOutArmWhichKit=0;
                    Task=1200;
                }
                break;
            }
            else if(BRCarryKit.HasIC() && OutSHT2InRT() && bCheckShuttle2Flag==false)                                                                           // change another shuttle
            {
                Task=50;
                break;
            }
            else if(IsOutArmCleanOutFinish())                                                                                                                   //Steven 20230323 : fixed for out arm hang up
            {
                Task=50;
            }
            break;
        case 1160:
            if(MoveOutArmXY_ToFix_Tray_Full())
            {
//jou 20231106 : 修正out arm 1x1 hang up
//                if(WhichAutoNeedTray()==0)                                      //JerryYang 20170731 (wei) Out arm要讓位才不會hang up
                    Task=1100;
            }
            break;
        case 1170:                                                       //Eastsun 20260521 整合
            if(MoveOutArmXY_To_ESDSafePos())
            {
                Task=1100;
            }
            break;
        case 1200:
            if(bCheckSpeed)                                                                                                                                     //Steven 20110525 : Auto Speed
            {
                bCheckSpeed=false;
                OutArmAddSpeed();
            }

            if(bCheckSpeed1)                                                                                                                                    //KaiChen 20171225 (Steven)：Add Speed Display
            {
                bCheckSpeed1=false;
                OutArmAddSpeedDisplay();
            }

//            IsCatchTrayReadySupplyNewTray();  //JerryYang 20200603 : 吸完shuttle 1再讓tray arm移動, 修正在shuttle 1下排吸料的時候正好tray arm在Auto 1上方造成tray arm position error
            if(DoPickFromShuttle1_9045_2x8_32(true))
            {
                PreSetOutAdditionalFlag();                                                                                                                      //Steven 20220614 : Fixed for out arm additional function
                if(FRCarryKit.LeftSideNoIC(4)==false)
                {
                    //jou 2013-01-21 修正 Tester 分類設定錯誤，Hang up
                    GetRowCol(iRow, iCol);
                    for(i=0; i<MAX_Index_Row; i++)
                    {
                        for(j=0; j<NEW_MAX_Index_Col; j++)
                        {
                            if(i>=iRow || j>=iCol)
                                FRCarryKit.SetItemData(i, j, NULL_IC);
                        }
                    }
                    InitPickFromShuttle1Task();                                                                                                                 //Sam 20230208 : 修正 Fix AOI 未拍照問題
                    //iPickFromShuttle1Task=1;
                    return;
                }

                if(OutArmSuck.HasIC()==false)
                {
                    Task=50;
                }
                else
                {
                    iOutRotateFinish=0;                                                                                                                         //kevin 20130524 準備放到ROTATE
//                    iOutArmPickPlaceCnt[OutOfsOutSh1]++;  //JerryYang 20230523 : 沒在用, Mark掉    //JerryYang 20180921 Setup Teach功能
                    bPickSH1Flag=true;
                    bPickSH2Flag=false;
                    Task=3000;
                    bNeedCheckOurArmDeviceSuck=true;                                                                                                            //jou 2010-09-07 start : 確認Out arm有確實將device吸走
                }
            }
            else
            {
                if(bOutArmManualStepPress)
                    Task=1235;
            }
            break;
        case 1230:
//            IsCatchTrayReadySupplyNewTray();  //JerryYang 20200603 : 吸完shuttle 1再讓tray arm移動, 修正在shuttle 1下排吸料的時候正好tray arm在Auto 1上方造成tray arm position error

            if(bPickShuttleError)
            {
                bPickShuttleError=false;
                InitPickFromShuttle1Task();
                Task=1200;
                break;
            }

            if(DoPickFromShuttle1_9045_2x8_32(true))
            {
                PreSetOutAdditionalFlag();                                                                                                                      //Steven 20220614 : Fixed for out arm additional function
                if(FRCarryKit.RightSideNoIC(4)==false)
                {
                    InitPickFromShuttle1Task();                                                                                                                 //Sam 20230208 : 修正 Fix AOI 未拍照問題
                    //iPickFromShuttle1Task=1;
                    return;
                }

                if(OutArmSuck.HasIC()==false)
                {
                    Task=50;
                }
                else
                {
                    OldPos=1-OldPos;
                    iOutRotateFinish=0;                                                                                                                         //kevin 20130524 目前放到 auto tray
//                    iOutArmPickPlaceCnt[OutOfsOutSh1]++;  //JerryYang 20230523 : 沒在用, Mark掉  //JerryYang 20180921 Setup Teach功能
                    bPickSH1Flag=true;
                    bPickSH2Flag=false;
                    Task=3000;
                }
            }
            else
            {
                if(bOutArmManualStepPress)
                    Task=1235;
            }
            break;
        case 1235:
            bOutArmManualStepPress=false;
            if(MoveOutArmToAutoSafe())
                Task=1150;
            break;
        case 2000:
            if(USE_IN_OUT_ARM_Y_PITCH==iXYPitchVariable &&IniConfig.bE57YPitchHome)                                                                             //kevin 20180827 (Steven) : add put shuttle Y pitch home AutoYPitch
            {
                Task=2010;
                return;
            }
            Task=2040;
            break;
        case 2010:                                                                                                                                              //kevin 20180827
            if(MoveOutArmToAutoSafe())
            {
                hOutArmYpitchHomeTimer.SetSecAndOn(10);
                Task=2020;
            }
            break;
        case 2020:
            if(bCheckYPitchRunHomeSen(1))
            {
                 Task=2040;
            }
            else if(hOutArmYpitchHomeTimer.Off())
            {
                InitProcessSingleMotorTask(MOutArmPitchY);
                hOutArmYpitchHomeTimer.SetSecAndOn(30);
                bYpitchNeddHome=true;
                Task=2030;
            }
            break;
        case 2030:                                                                                                                                              //Ypitch home
            if(bCheckYPitchHome(1))
            {
                SetMotorSpeed();
                bYpitchNeddHome=false;
                 Task=2040;
            }
            else if(hOutArmYpitchHomeTimer.Off())
            {
                ShowErrorMessage("WAR0227", K_RETRY, MInArmPitchY);
                Task=2020;
            }
            break;
        case 2040:
            if((iOneCycle==1 || iCleanOut==1) &&
                BTestSuck.HasIC()==false && BRCarryKit.HasIC()==false &&
                (FRCarryKit.HasIC() || FTestSuck.HasIC()))
            {
                Task=50;
                break;
            }

            if(BRCarryKit.HasIC()==false && OutSHT2InRT())
            {
                if(FRCarryKit.HasIC() && OutSHT1InRT())
                {
                    Task=50;
                    break;
                }
            }

            if((iCleanOut==1 && HasICUnderMachine()==false) ||
               (iCleanOut==1 && LastSet.iTemperature==Tempture_Hot && (TrayForm.bAutoFeed==false && bMustCleanAllTray==false) && HasICUnderHotPlate()==false))  //JerryYang 20190807 fix 快速clean out hang up
            {
                FirstEnter=true;
                Task=1;
                break;
            }
            //Eliot 2009_12_29 start
            if((BRCarryKit.Item[0][0]!=NULL_IC && BRCarryKit.Item[0][0]!=HAS_NULL_IC) ||
                (BRCarryKit.Item[0][1]!=NULL_IC && BRCarryKit.Item[0][1]!=HAS_NULL_IC) ||
                (BRCarryKit.Item[0][2]!=NULL_IC && BRCarryKit.Item[0][2]!=HAS_NULL_IC) ||
                (BRCarryKit.Item[0][3]!=NULL_IC && BRCarryKit.Item[0][3]!=HAS_NULL_IC) ||
                (BRCarryKit.Item[1][0]!=NULL_IC && BRCarryKit.Item[1][0]!=HAS_NULL_IC) ||
                (BRCarryKit.Item[1][1]!=NULL_IC && BRCarryKit.Item[1][1]!=HAS_NULL_IC) ||
                (BRCarryKit.Item[1][2]!=NULL_IC && BRCarryKit.Item[1][2]!=HAS_NULL_IC) ||
                (BRCarryKit.Item[1][3]!=NULL_IC && BRCarryKit.Item[1][3]!=HAS_NULL_IC))
            {
                iOutArmWhichKit=0;
            }
            else if((BRCarryKit.Item[0][4]!=NULL_IC && BRCarryKit.Item[0][4]!=HAS_NULL_IC) ||
                (BRCarryKit.Item[0][5]!=NULL_IC && BRCarryKit.Item[0][5]!=HAS_NULL_IC) ||
                (BRCarryKit.Item[0][6]!=NULL_IC && BRCarryKit.Item[0][6]!=HAS_NULL_IC) ||
                (BRCarryKit.Item[0][7]!=NULL_IC && BRCarryKit.Item[0][7]!=HAS_NULL_IC) ||
                (BRCarryKit.Item[1][4]!=NULL_IC && BRCarryKit.Item[1][4]!=HAS_NULL_IC) ||
                (BRCarryKit.Item[1][5]!=NULL_IC && BRCarryKit.Item[1][5]!=HAS_NULL_IC) ||
                (BRCarryKit.Item[1][6]!=NULL_IC && BRCarryKit.Item[1][6]!=HAS_NULL_IC) ||
                (BRCarryKit.Item[1][7]!=NULL_IC && BRCarryKit.Item[1][7]!=HAS_NULL_IC))
            {
                iOutArmWhichKit=1;
            }
            //Eliot 2009_12_29 end
            if(OutSHT2InRT() && bCheckShuttle2Flag==false)
            {
                if(BRCarryKit.HasIC())
                {
                    if(CheckShuttleICPos(2))
                    {
                        Task=2050;
                    }
                    else
                    {
                        InitPickFromShuttle2Task();                                                                                                             //Sam 20230208 : 修正 Fix AOI 未拍照問題
                        //iPickFromShuttle2Task=1;
                        if(BRCarryKit.LeftSideNoIC(4))
                            Task=2230;
                        else
                            Task=2200;
//                        #ifdef Carry4
//                            MOT[MOutShuttle2].fCanMoveR=false;
//                        #else
                            MOT[MInShuttle2].fCanMoveR=false;
//                        #endif
                    }
                }
                else
                    Task=2050;
            }
            else
                Task=2050;
            break;
        case 2050:
            if(bCheckSpeed)                                                                                                                                     //Steven 20110525 : Auto Speed
            {
                bCheckSpeed=false;
                OutArmSubSpeed();
            }
            //KaiChen 20171225 (Steven)：Add Speed Display
            //==>
            if(bCheckSpeed1)
            {
                bCheckSpeed1=false;
                OutArmSubSpeedDisplay();
            }
            //<==
            //KaiChen 20171225 (Steven)：Add Speed Display

            if(MoveOutArmToShuttleIncludeZ_9045_2x8_32(iWhichShuttle, iOutArmWhichKit, false))
            {
                IsCatchTrayReadySupplyNewTray();

                Task=2100;
            }
            break;
        case 2100:
            IsCatchTrayReadySupplyNewTray();

            //==> Eastsun 20260521 整合
            if(WhichAutoNeedTray()!=0)
            {
                if(IniConfig.bA15_1ESDGiveWayFunction==true && USE_BU5_Function==true)
                {
                    MoveOutArmXY_To_ESDSafePos();
                    Task=2170;
                }
            }
            //<== Eastsun 20260521 整合
            else if(BRCarryKit.HasIC() && OutSHT2InRT() && bCheckShuttle2Flag==false)
            {
//#ifdef Carry4
//                MOT[MOutShuttle2].fCanMoveR=false;
//#else
                MOT[MInShuttle2].fCanMoveR=false;
//#endif
                InitPickFromShuttle2Task();
                //iPickFromShuttle2Task=1;

                if(BRCarryKit.LeftSideNoIC(4))
                {
                    iOutArmWhichKit=1;
                    Task=2230;
                }
                else
                {
                    iOutArmWhichKit=0;
                    Task=2200;
                }
                break;
            }
            else if(FRCarryKit.HasIC() && OutSHT1InRT() && bCheckShuttle1Flag==false)
            {
                Task=50;
                break;
            }
            break;
        case 2170:                                                       //Eastsun 20260521 整合
            if(MoveOutArmXY_To_ESDSafePos())
            {
                Task=2100;
            }
            break;
        case 2200:
            if(bCheckSpeed)                                                                                                                                     //Steven 20110525 : Auto Speed
            {
                bCheckSpeed=false;
                OutArmAddSpeed();
            }
            //KaiChen 20171225 (Steven)：Add Speed Display
            //==>
            if(bCheckSpeed1)
            {
                bCheckSpeed1=false;
                OutArmAddSpeedDisplay();
            }
            //<==
            //KaiChen 20171225 (Steven)：Add Speed Display

            IsCatchTrayReadySupplyNewTray();

            if(DoPickFromShuttle2_9045_2x8_32(true))
            {
                PreSetOutAdditionalFlag();                                                                                                                      //Steven 20220614 : Fixed for out arm additional function
                if(BRCarryKit.LeftSideNoIC(4)==false)
                {
                    //jou 2013-01-21 修正 Tester 分類設定錯誤，Hang up
                    GetRowCol(iRow, iCol);
                    for(i=0; i<MAX_Index_Row; i++)
                    {
                        for(j=0; j<NEW_MAX_Index_Col; j++)
                        {
                            if(i>=iRow || j>=iCol)
                                BRCarryKit.SetItemData(i, j, NULL_IC);
                        }
                    }

                    InitPickFromShuttle2Task();                                                                                                                 //Sam 20230208 : 修正 Fix AOI 未拍照問題
                    //iPickFromShuttle2Task=1;
                    return;
                }

                if(OutArmSuck.HasIC()==false)
                {
                    Task=50;
                }
                else
                {
                    iOutRotateFinish=0;                                                                                                                         //kevin 20130524 目前放到 auto tray
//                    iOutArmPickPlaceCnt[OutOfsOutSh2]++;  //JerryYang 20230523 : 沒在用, Mark掉  //JerryYang 20180921 Setup Teach功能
                    bPickSH1Flag=false;
                    bPickSH2Flag=true;
                    Task=3000;
                    bNeedCheckOurArmDeviceSuck=true;                                                                                                            //jou 2010-09-07 start : 確認Out arm有確實將device吸走
                }
            }
            else
            {
                if(bOutArmManualStepPress)
                    Task=2235;
            }
            break;
        case 2230:
            IsCatchTrayReadySupplyNewTray();

            if(bPickShuttleError)
            {
                bPickShuttleError=false;
                InitPickFromShuttle2Task();
                Task=2200;
                break;
            }

            if(DoPickFromShuttle2_9045_2x8_32(true))
            {
                PreSetOutAdditionalFlag();                                                                                                                      //Steven 20220614 : Fixed for out arm additional function
                if(BRCarryKit.RightSideNoIC(4)==false)
                {
                    InitPickFromShuttle2Task();                                                                                                                 //Sam 20230208 : 修正 Fix AOI 未拍照問題
                    //iPickFromShuttle2Task=1;
                    return;
                }

                if(OutArmSuck.HasIC()==false)
                {
                    Task=50;
                }
                else
                {
                    OldPos=1-OldPos;
                    iOutRotateFinish=0;                                                                                                                         //kevin 20130524 目前放到 auto tray
//                    iOutArmPickPlaceCnt[OutOfsOutSh2]++;  //JerryYang 20230523 : 沒在用, Mark掉  //JerryYang 20180921 Setup Teach功能
                    bPickSH1Flag=false;
                    bPickSH2Flag=true;
                    Task=3000;
                }
            }
            else
            {
                if(bOutArmManualStepPress)
                    Task=2235;
            }
            break;
        case 2235:
            bOutArmManualStepPress=false;
            if(MoveOutArmToAutoSafe())
                Task=2050;
            break;
        case 3000:
            IsCatchTrayReadySupplyNewTray();
            if(MoveOutArmToAutoSafe())
            {
                CheckOutArmSuckICFallDown();
//#ifdef Carry4
//                MOT[MOutShuttle1].fCanMoveR=true;
//                MOT[MOutShuttle2].fCanMoveR=true;
//#else
                if(TestIF_File.iShuttleMode==0)                                                                                                                 //Jou 20150909 : For Add 32Site UPH, 讓Shuttle 2先回去補Input
                {
                    if(FRCarryKit.NoIC()==true)
                        MOT[MInShuttle1].fCanMoveR=true;
                    if(BRCarryKit.NoIC()==true)
                        MOT[MInShuttle2].fCanMoveR=true;
                }
                else
                {
                    MOT[MInShuttle1].fCanMoveR=true;
                    MOT[MInShuttle2].fCanMoveR=true;
                }
//#endif
                if(CheekNeedToDoOutArmAdditionalFunction())                                                                                                     //Steven 20210609 : 整合Precisor, Rotator, Bottom CCD, Die Clean
                {
                    Task=7000;
                }
                else
                {
                    Task=3010;
                    AseIcRecord="@e02001";                                                                                                                      //kevin 20161228 (jou) 傳送給 ASE 抬頭資料
                }
            }
            break;
        case 3010:
            IsCatchTrayReadySupplyNewTray();
            if(OutArmSuck.HasIC()==false)
            {
                Task=100;
                break;
            }
            iOutRotateFinish=3;                                                                                                                                 //kevin 20130524 目前放到 auto tray
            iWhichAuto=SearchTrayToPlace_9045();                                                                                                                //Steven 20160524 : 簡化Out Arm程式碼
            if(AUTO3_IS_MAGAZINE==1 &&                                                                                                                          //JerryYang 20221215 : add Magazine
               iWhichAuto==iMagAtAuto)
            {
                Task=10000;
                break;
            }

            if(AUTO3_IS_MAGAZINE==1             &&                                                                                                              //JerryYang 20221215 : Magazine把fix區當buffer區功能
               TestIF_File.iMagFixTrayType==1   &&
               iWhichAuto>=iMagPos)
            {
            }
            else
            {
                SetFixTrayFullIC();
                if(MOT[iUnloadMotorIndex[iWhichAuto]].fHasTray==false ||
                   (MOT[iUnloadMotorIndex[iWhichAuto]].FullIC() &&
                    CheckUseFixBinBoxFunction()==false))                                                                                                        //kevin 20160822
                {
                    if(iWhichAuto>=iAutoCnt)
                    {
                        if(TrayForm.iFixTrayMode==0)                                                                                                            // full tray
                        {
                            #if 0 // TODO(W7): golden SYSTEM_BIN_SELECT::bFixLink removed in 906 (renamed to bFixTrayLink); this stale-dead golden ref has no member -> gated, control flow falls through to the bFixTrayLink[2] block below (offline-safe; never dispatched)
                            if(iWhichAuto==iFixPosMin && BinSelect[iTestRunMode].bFixLink[1])
                            {
                                for(j=0; j<MAX_ARM_Col; j++)
                                {
                                    if(OutArmSuck.Item[iOutArmPlaceOrder][j]!=NULL_IC &&
                                       OutArmSuck.iWhichAuto[iOutArmPlaceOrder][j]==iWhichAuto)
                                    {
                                        OutArmSuck.iWhichAuto[iOutArmPlaceOrder][j]=iWhichAuto+1;
                                        return;
                                    }
                                }
                            }
                            #endif

                            if(iWhichAuto==iFixPosMin+1 && TrayForm.bFixTrayLink[2])                                                                            // fix 2 full link fix3
                            {
                                for(j=0; j<MAX_ARM_Col; j++)
                                {
                                    if(OutArmSuck.Item[iOutArmPlaceOrder][j]!=NULL_IC &&
                                       OutArmSuck.iWhichAuto[iOutArmPlaceOrder][j]==iWhichAuto)
                                    {
                                        OutArmSuck.iWhichAuto[iOutArmPlaceOrder][j]=iWhichAuto+1;
                                        return;
                                    }
                                }
                            }
                        }
                        Task=4000;
                        break;
                    }
                    else if(IniConfig.bAutoTrayLink==true)                                                                                                      //jou 2012-06-14 Auto Tray Link
                    {
                        if((iWhichAuto==eAuto2 && BinSelect[iTestRunMode].bAutoLink[2]==true) ||
                           (iWhichAuto==eAuto1 && BinSelect[iTestRunMode].bAutoLink[1]==true))
                        {
                            for(j=0; j<MAX_ARM_Col; j++)
                            {
                                if(OutArmSuck.Item[iOutArmPlaceOrder][j] != NULL_IC &&
                                    OutArmSuck.iWhichAuto[iOutArmPlaceOrder][j]==iWhichAuto)
                                {
                                    OutArmSuck.iWhichAuto[iOutArmPlaceOrder][j]=iWhichAuto+1;
                                }
                            }
                            return;
                        }
                    }
                    MoveOutArmXY_ToFix_Tray_Full();
                    break;
                }
            }

            if(MOT[MTrayX].ReadPos()>Prod.iXTrayColor)
            {
                MoveOutArmXY_ToFix_Tray_Full();
                break;
            }
            InitPlaceToAutoTask();
            if(iWhichAuto<=iAutoRight)
            {
                if(MOT[iMMAuto[iWhichAuto]].fHasTray==false ||
                   MOT[iMMAuto[iWhichAuto]].FullIC())
                    break;
            }
            else
            {
                if(AUTO3_IS_MAGAZINE==1             &&                                                                                                          //JerryYang 20221215 : Magazine把fix區當buffer區功能
                   TestIF_File.iMagFixTrayType==1   &&
                   iWhichAuto>=iMagPos)
                {
                }
                else
                {
                    SetFixTrayFullIC();
                    if(MOT[iMFixTray[iWhichAuto%iAutoCnt]].fHasTray==false ||
                       MOT[iMFixTray[iWhichAuto%iAutoCnt]].FullIC())
                    {
                        if(iHWFix_BinBox==1 && iWhichAuto==iBinBoxPos)
                            DoFixTrayFullAlarm();                                                                                                               //kevin 20201207 check Error bin box Fix 1 put tray
                        break;
                    }
                }
            }

            if(iCatchTrayControlManual>=2 || WhichAutoNeedTray()!=0)                                                                                            // catch tray busy
            {
                MoveOutArmXY_ToFix_Tray_Full();
                break;
            }
            Task=3100;
            break;
        case 3020:
            IsCatchTrayReadySupplyNewTray();
            if(MoveOutArmXY_ToFix_Tray_Full())
            {
                if(MOT[iMMAuto[iWhichAuto]].fHasTray==true)
                    Task=3100;
            }
            break;
        case 3050:
            if(AutoTrayReCheck(iWhichAuto))                                                                                                                     //Ifor 20171031 add Auto Tray ReCheck
            {
                Task=3100;
            }
            break;
        case 3100:
            if(iWhichAuto<=iAutoRight)
            {
                if(MOT[iMMAuto[iWhichAuto]].fHasTray==true)
                {
                    flag1=Sen[SnAutoTrayDetect[iWhichAuto]].IsOff();
                    flag2=Sen[SnAutoFixCyPush[iWhichAuto]].IsOn();
                    flag3=Sen[SnAutoEdgePush[iWhichAuto]].IsOn();
                    if((flag1 || flag2 || flag3) && LastSet.iRealDummy!=DUMMY)
                    {
                        Cylinder[C_AutoSide_Fixer[iWhichAuto]].Off();
                        Cylinder[C_AutoEdgePush[iWhichAuto]].Off();
                        Cylinder[C_AutoUpPress[iWhichAuto]].Off();                                                                                              //JerryYang 20190423 新增unloader壓tray

                        if(flag2)
                            str=sJAM1101[iWhichAuto];
                        else if(flag3)
                            str=sJAM1102[iWhichAuto];
                        else
                            str=sWAR1130[iWhichAuto];

                        ret=ShowErrorMessage(str, K_RETRY|K_SKIP, iMMAuto[iWhichAuto], bTrayDuplicateErr[iWhichAuto], __FUNC__);
                        bTrayDuplicateErr[iWhichAuto]=true;
                        if(ret==K_SKIP)
                        {
                            //#if 0 // TODO(W7): fProductionInfo form (golden ProductionInfo.h TfProductionInfo : public TForm) -- no FormsFacade stand-in; gated (offline no IC-count update)   //AI(W906-S09-FREED) 20260930: gate retired -- as :2542 of aoutarm9045_2x2_4_23.cpp (CalTrayICCount, forms/fProductionInfo.cpp no-op); body = golden 906_0625_Steven aoutarm9045_2x8_32.cpp:2719
                            fProductionInfo->CalTrayICCount(iWhichAuto);
                            //#endif   //AI(W906-S09-FREED) 20260930: see :3124
                            MOT[iMMAuto[iWhichAuto]].ClearTray(__FUNC__);        // translated: golden no-arg ClearTray -> ClearTray(AnsiString) sibling convention
                            bTrayDuplicateErr[iWhichAuto]=false;
                            Task=3020;
                        }
                        else
                        {
                            Task=3050;                                                                                                                          //Ifor 20171031 add Auto Tray ReCheck
                        }
                        return;
                    }
                    else
                    {
                        bTrayDuplicateErr[iWhichAuto]=false;
                    }
                }
            }
            Task=3300;                                                                                                                                          //kevin 20151203 沒有fixtray放ic
        case 3300:
            if(AUTO3_IS_MAGAZINE==1             &&                                                                                                              //JerryYang 20221215 : Magazine把fix區當buffer區功能
               TestIF_File.iMagFixTrayType==1   &&
               iWhichAuto>=iMagPos              &&
               iWhichBuff>=iFixPosMin &&
               iWhichBuff<=iFixPosMax)
            {
                i=iWhichBuff%iAutoCnt;
                if(iWhichBuff>=iFixPosMin && Sen[SnFixedTrayDetect[i]].IsOff() && LastSet.iRealDummy!=DUMMY)                                                    //wei 20151217 add iWhichAuto>=3避免沒要放FIX盤也偵測有沒有FIX盤
                {
                    Task=4000;
                    break;
                }
            }
            else
            {
                i=iWhichAuto%iAutoCnt;
                if(iWhichAuto>=iAutoCnt && Sen[SnFixedTrayDetect[i]].IsOff() && LastSet.iRealDummy!=DUMMY)                                                      //wei 20151217 add iWhichAuto>=3避免沒要放FIX盤也偵測有沒有FIX盤
                {
                    Task=4000;
                    break;
                }
            }

            #if 0 // TODO(W7): fAutoTeach form (golden AutoTeach.h TfAutoTeach) -- no FormsFacade stand-in; offline IsRun()==false -> fall through to Task=3310
            if(fAutoTeach->IsRun())                                                                                                                             //JimmyChiu 20211020 : Auto alignment mode
            {
                Task=3305;
                break;
            }
            #endif
            Task=3310;
            break;
        case 3305:                                                                                                                                              //JimmyChiu 20211020 : Auto alignment mode
            ret=DoPlaceToAutoForAutoTeachOffset_9045_2x8_32();
            if(ret==1)
                Task=100;
            else if(ret==2)
                Task=3310;
            if(Task!=3310)                                                                                                                                      //Steven 20180813 : add Out arm speed
                break;
        case 3310:
            if(DoPlaceToAuto_9045_2x8_32())                                                                                                                     // 吸嘴可能還在下方
            {
                DoOutArmAfterPlaceToAuto(bCheckSpeed, true);                                                                                                    //Steven 20220526 : 針對放下IC到Unloader後的動作做整合
                Task=3500;
            }
            else
            {
                break;
            }
        case 3500:
            ret=DoOutArmAfterPlaceToAuto(bCheckSpeed);                                                                                                          //Steven 20220526 : 針對放下IC到Unloader後的動作做整合
            if(ret!=0)
                Task=ret;
            break;
        case 4000:
            if(MoveOutArmXY_ToFix_Tray_Full())
                Task=4100;
            break;
        case 4100:
            if(DoFixTrayFullAlarm())                                                                                                                            //Steven 20181113 : 修正Fix Link Alarm的問題
            {
                Task=3010;
            }
            break;
        case 7000:
            flag3=DoOutArmAdditionalFunction();                                                                                                                 //Steven 20210609 : 整合Precisor, Rotator, Bottom CCD, Die Clean
            if(flag3)
            {
                AseIcRecord="@e02001";                                                                                                                          //kevin 20161228 (jou) 傳送給 ASE 抬頭資料
                Task=3010;
            }
            break;
        //JerryYang 20220909 : add magazine
        //==>
        case 10000:
            if(CheckPlaceToMagazineTray(iWhichMag))
            {
                Task=12000;                                                                                                                                     // To Auto 3
            }
            break;
        case 11100:
            if(DoPickFromMagazineBuffer())
            {
                iWhichAuto=SearchTrayToPlace_9045();
                iOutArmWhichAuto=iMagAtAuto;
                if(iAuto3MagazineIndex!=iWhichMag && iWhichMag!=-1)
                {
                    bMagNeedChangeTray=true;
                }
                Task=12000;
            }
            break;
        case 12000:
            if(CUSTOMER_CODE==CC_KYEC_LEE && AUTO3_IS_MAGAZINE==1) //Eastsun 20260515 F021: Magazine search hang-up fix
            {
                SearchTrayToPlace_Magazine();
            }
            IsCatchTrayReadySupplyNewTray();
            if(iAuto3MagazineIndex!=iWhichMag ||
               MOT[iMMAuto[iMagAtAuto]].fHasTray==false ||
               MOT[iMMAuto[iMagAtAuto]].FullIC())
            {
                MoveOutArmXY_ToFix_Tray_Full();                                                                                                                 // change Magazine Tray
                break;
            }

            bMagNeedChangeTray=false;
            if(bMagCatchTrayfalg)                                                                                                                               //JerryYang 20221121 : AUTOI3 Magazine還在夾TRAY的旗標
            {
                break;
            }

            if(MOT[MTrayX].ReadPos()>Prod.iXTrayColor)
            {
                MoveOutArmXY_ToFix_Tray_Full();
                break;
            }

            InitPlaceToAutoTask();
            if(iCatchTrayControlManual>=2)                                                                                                                      // catch tray busy
            {
                MoveOutArmXY_ToFix_Tray_Full();
                break;
            }
            Task=3100;
            break;
        //<==
        //JerryYang 20220909 : add magazine
    }
}
//---------------------------------------------------------------------------
