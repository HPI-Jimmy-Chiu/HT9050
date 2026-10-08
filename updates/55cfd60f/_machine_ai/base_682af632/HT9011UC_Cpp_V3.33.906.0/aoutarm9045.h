// =============================================================================
//  aoutarm9045.h  --  Out-Arm ENGINE public interface  (W6.2c translation)
//
//  Faithful translation of golden  aoutarm9045.h  (BCB6, Big5/cp950).
//  Translator: AI(W6.2c-OUTARM) 20260626
//
//  Names / signatures / default args preserved VERBATIM so the team can
//  reverse-look-up the BCB6 version.  vclcompat AnsiString (1-based) replaces
//  the VCL AnsiString.  No __fastcall / __property / PACKAGE in this header.
//
//  SCOPE (W6.2c) -- this is the DIRECT mirror of the in-arm engine wave (W6.2b).
//  The .cpp keeps ACTIVE only:
//    * pure geometry / pitch helpers (GetOutArmPitch*, GetOutArmPitchXMM/YMM,
//      BinBoxShiftY),
//    * the two iInArmType dispatch ladders DoOutArm_9045 / DoPickFromShuttle_9045
//      (pre-dispatch guards + if-ladder + DoPickFromShuttle final-else Program-
//      Error), and
//    * the 4 pumpable task SMs DoOutArmAdditionalFunction /
//      DoOutArmPlaceToAuto_9045 / DoOutArmAfterPlaceToAuto / DoOutArmIonFanGiveWay
//      (switch skeleton + cursor flow + fall-throughs verbatim).
//  Everything else is an ACTIVE offline stub (return false / 0 / no-op / sane
//  default) gated `#if 0 // TODO(W6.2b variants)` / `// TODO(W7)` so the engine
//  LINKS into lib ht9045_sm and the 4 SMs pump over the Sim HAL.
//
//  CORRECTION to the original brief: the out-arm engine dispatches on the SHARED
//  global `int iInArmType` (golden cmydef.h), NOT an `iOutArmType` (no such
//  symbol exists in this file) -- exactly like the in-arm engine.
//
//  NOTE: golden aoutarm9045.h #includes "HTEditList.h" (uPoint2D / uPlateInfo /
//  MyFillTheTrayAfterOutArmPlace).  Those Fill-The-Tray + Auto-Cal-Suck-Z types
//  belong to gated W7 surface; this header does NOT pull them.  uPoint2D already
//  lives in cmydef.h.  The Fill-Tray / Auto-Cal decls are omitted (their bodies
//  are gated out of the .cpp).
//
//  Big5: every Chinese comment decoded via cp950 and preserved as UTF-8.
//  Final gate: ZERO U+FFFD.
// =============================================================================
//---------------------------------------------------------------------------
#ifndef aoutarm9045H
#define aoutarm9045H
//---------------------------------------------------------------------------
#include "cmydef.h"                 // MAX_ARM_Row/Col, iInArmType, enums
//---------------------------------------------------------------------------
extern bool bOutArmCheckDestroyACT[MAX_ARM_Row][MAX_ARM_Col];                   //jou 981130 確認device確實destroy完成
extern int  iBinBoxShiftY;                                                      //jou 2012-12-11 support Bin Box
void BinBoxShiftY();                                                            //jou 2012-12-11 support Bin Box
extern int iOutArmAdditionalFunctionTask;

const int iOUTARM_SUCK=1000;

bool MoveOutArmToAutoSafe_9045();
bool DoMoveOutArmXYToPlace_9045(bool &bCanPick2ICAtOnceTime, int iRow, int iCol, bool RealMove);

int  GetOutArmPitch_9045(int w);
int  GetOutArmPitch2_9045(int w);                                               //Steven 20131002 : XY變距
int  GetOutArmPitchY_9045(int w, int iOffset=-1);                               //Steven 20131002 : XY變距
int  GetOutArmPitchX_9045(int w, int iX, int iOffset=-1);                       //Steven for HT1032
int  GetOutArmPitchXMM_9045(int iMotPulse);
int  GetOutArmPitchYMM_9045(int iMotPulse);

void GetOutArmZDownPos_9045(int iWhichAuto, bool bNeedDown, bool ZDownSel[MAX_ARM_Row][MAX_ARM_Col], int iZPos[MAX_ARM_Row][MAX_ARM_Col]);                      //Steven 20230323 : For HT1032
void GetOutArmCellPos(int iRow, int iCol, int &Ypos, int &Xpos);
void GetOutArmToShtCellPos(int iRow, int iCol, int &Ypos, int &Xpos);

void GetOutShuttleStatus_9045(int iSht, int iZPos[MAX_ARM_Row][MAX_ARM_Col], bool bZDown, bool bZFlag[MAX_ARM_Row][MAX_ARM_Col]);

void DoOutArm_9045();
bool DoPickFromShuttle_9045(int iSht);
bool DoOutArmPlaceToAuto_9045();
void DoOutArmSuckPreOn(int iSht, int iKit, int iXPos, int iYPos, bool bZDown);
bool DoOutArmAdditionalFunction();                                              //Steven 20210609 : 整合Precisor, Rotator, Bottom CCD, Die Clean
int  DoOutArmAfterPlaceToAuto(bool &bCheckSpeed, bool bInitial=false);          //Steven 20220526 : 針對放下IC到Unloader後的動作做整合
int  VerifyFixTrayLink();

bool CheckOutArmInitState_9045();
bool CheekNeedToDoOutArmAdditionalFunction();

void InitDoOutArmAdditionalFunction();
void InitialFix3CanFullTask();                                                  //ChungHung 20140313 add Fix3 can Full Tray
bool InitialOutArmNeedSuck(int iShuttle);                                       //Steven 20220927 : 重新整理In / Out Arm Need Suck
bool UseFix3Cylinder(int iWhichAuto);                                           //ChungHung 20140313 add Fix3 can Full Tray

bool SetOutArm_9045();

int  SearchTrayToPlace_9045(bool bIsSort=false);                                //RogerYang 20250515 Add for 9046AU   //Steven 20160524 : 簡化Out Arm程式碼
bool SearchUnLoadTrayUpDown_9045(bool &bCanPick2ICAtOnceTime, int &iRow, int &iCol);

bool IsCheckOutArmDestroyActiveFinish(int iXPos, int iYPos);                    //ChungHung 201112296 add Pick 前先確認CheckInArmDestroyActive 已完成
void PreSetOutAdditionalFlag();                                                 //Steven 20210609 : 整合Precisor, Rotator, Bottom CCD, Die Clean
bool OutArmNeedCheckOffset(bool bPlace, int iPos);                             //Steven 20230531 : 簡化判斷式

bool Check_QA_ModeUnloadCount();                                                //JerryYang 20221004 : Maxim版本QA mode

extern int iOutPickFromSht;
extern int iOutPlaceToAuto;
void InspectOutArmPosition(int iTarget, int iSuckRow, int iSuckCol, int iTargetRow, int iTargetCol, int iAction);

bool IsMotorArrival(int iMot,int iTargetPos);
extern bool DoOutArmIonFanGiveWay();                                            //Ifor 20241126 add: OutArm Ion Fan Give Way
extern void InitDoOutArmIonFanGiveWayTask();                                    //Ifor 20241126 add: OutArm Ion Fan Give Way
bool MoveOutArmXY_To_ESDSafePos();

// destroy-confirm SM surface (golden aoutarm9045.cpp:251/285/287)
extern bool bCheckOutArmDestroyActive[MAX_ARM_Row][MAX_ARM_Col];
extern bool bCheckOutArmDestroyActiveFinish[MAX_ARM_Row][MAX_ARM_Col];
void CheckOutArmDestroyActive();                                                //jou 981130 確認device確實destroy完成

// AI(pt-wave) 20260811 PT-W8 integrate: published because retiring the one-line
// stand-in that used to live at aoutarm.cpp:541 also removed the DECLARATION
// aoutarm.cpp:816 was relying on -- the stub sat in the SAME TU as its caller,
// so its retirement is a COMPILE error, not a link error.  (The other 15 stubs
// retired this wave were in *_shims.cpp, i.e. a different TU from every caller,
// and those callers carry their own forward declarations -- which is exactly why
// checking only the cross-TU case missed this one.)  Signature is golden's:
// aoutarm9045.h:64.
int  SearchTrayToPick_Buffer();                                                 // golden aoutarm9045.h:64

#endif
