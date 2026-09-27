// =============================================================================
//  aoutarm9045S_1x4_4.cpp  --  Out-Arm S-PREFIX per-site VARIANT (S 1x4_4)
//
//  Faithful translation of golden aoutarm9045S_1x4_4.cpp (2314 lines, BCB6, Big5/cp950).
//  Translator: AI(W6.2c-9045S_1x4_4) 20260627
//  Translation wave: W6.2c batch-5 (out-arm per-site-config variant: S-prefix 1x4_4).
//
//  ROLE: the per-site out-arm shuttle-pick / place-to-auto state machines for the
//  S-prefix 1x4_4 layout.  This file owns (golden names VERBATIM, all with the
//  irregular _9045S_1x4_4 infix):
//    GetNowShuttleMode_9045S_1x4_4, OutArmZNeedDown_9045S_1x4_4,
//    GetOutShuttleStatus_9045S_1x4_4, CheckOutArmXYPitch_9045S_1x4_4,
//    MoveOutArmToShuttleIncludeZ_9045S_1x4_4, DoPickFromShuttle1_9045S_1x4_4,
//    DoPickFromShuttle2_9045S_1x4_4, DoMoveOutArmXYToPlace_9045S_1x4_4,
//    SearchUnLoadTrayUpDown_9045S_1x4_4, SetOutArm_9045S_1x4_4,
//    DoPlaceToAuto_9045S_1x4_4, DoOutArm_9045S_1x4_4.
//
//  DEAD-BUT-COMPILED: the top-level export DoOutArm_9045S_1x4_4() has ZERO external
//  callers in the whole golden tree (out-arm dispatch ladders route to the SUFFIX-S
//  DoOutArm_9045_1x4_4S, defined in the DIFFERENT, already-committed
//  aoutarm9045_1x4_4S.cpp).  This file is translated faithfully + added to CMake so it
//  compiles+links; it is never dispatched and never referenced -- exactly like
//  in-arm 2x8_32.
//
//  FAITHFUL: every fn/var/cursor value, switch(Task) structure, formula, branch,
//  fall-through and commented-out code is transcribed VERBATIM from the golden CP950
//  source.  The DoOutArm_9045S_1x4_4 case-1140->1150, case-1150->1100, case-2050->2100,
//  case-3100->3300, case-3301->3500 fall-throughs are preserved EXACTLY.  The
//  DoPickFromShuttle1/2 case-10->200 fall-throughs are preserved EXACTLY.
//
//  VCL/Borland conversions: VCL AnsiString (1-based) via vclcompat; this file has no
//  __fastcall/__property/__published/PACKAGE.  SOFT_SIMULTE is NOT defined -> the two
//  #ifdef SOFT_SIMULTE blocks (chkInPickLoadError tests in DoPickFromShuttle1/2,
//  case iOUTARM_SUCK) are compiled OUT, kept VERBATIM.
//
//  Big5: every Chinese comment decoded via cp950 and preserved as UTF-8.  Final gate:
//  ZERO U+FFFD.
//
//  INCLUDE REWRITE (mirror of the committed sibling aoutarm9045_1x4_4S.cpp's translated
//  subset of the golden header set):
//    golden: MachineDefine.h ; aoutarm9045S_1x4_4.h ; aArmHeader.h ; MyKitSuck.h ;
//            cprod.h ; mymotor.h ; mycylin.h ; cSortCT.h ; mysensor.h ; main.h ;
//            note.h ; csystem.h ; acatchtray.h ; cinitial.h ; atester.h ;
//            aRotateKIT.h ; fRotate.h ; cOffSet.h ; fAOI.h ; cmydef.h ; aoutarm.h ;
//            ProductionInfo.h ; Magazine.h ; #pragma package(smart_init)
// =============================================================================
#include "MachineDefine.h"
#include "MachineType.h"            // OutOfsOutSh1 / OutOfsAuto1 / OutOfsFix1 / eTrayCount / CC_TSMC_TAINAN / CC_KYEC_LEE enums
//---------------------------------------------------------------------------
#include "aoutarm9045S_1x4_4.h"
#include "aArmHeader.h"             // OutArm, RecordProcess, __FUNC__ shim
#include "aoutarm9045.h"            // engine: GetOutArmPitch*_9045 / DoOutArmSuckPreOn / IsCheckOutArmDestroyActiveFinish / InitialOutArmNeedSuck / iOUTARM_SUCK / SearchTrayToPlace_9045 / VerifyFixTrayLink / DoOutArmPlaceToAuto_9045 / CheekNeedToDoOutArmAdditionalFunction / DoOutArmAdditionalFunction / DoOutArmAfterPlaceToAuto / PreSetOutAdditionalFlag / MoveOutArmXY_To_ESDSafePos / UseFix3Cylinder / InitialFix3CanFullTask / EnableFix3UseCylinder
#include "csystem.h"                // OutSHT1InRT / OutSHT2InRT / HasICUnderMachine / HasICUnderHotPlate / AutoTrayReCheck / CheckOutArmSuckICFallDown / XPitchIsStand / IsOutArmCleanOutFinish / DetectAutoTray
#include "Motor/mymotor.h"          // MOT[], OutArmContinuousMove_9045 (note: golden calls the no-suffix overload -- forward-declared below)
#include "cmydef.h"                 // global scalar universe + IC consts + iInArmType + enums + bEnterOffset / bCheckShuttle1/2Flag / iCleanOut / IniConfig / USE_* / iXpitch* / Mxxx motor ids / iAutoIndex / iMMAuto / iUnloadMotorIndex / iWhichBuff / iWhichMag / ...
#include "cprod.h"                  // Prod / TestIF / TestIF_File / ArmSpeed / OutArmOffSet / ReserveEmptyPoint / ReversionEmptyPoint / EnableFix3UseCylinder
#include "cpublic.h"                // CosFunction
#include "common.h"
#include "mysensor.h"               // Sen[] (SnRKManualStep / SnFixedTrayDetect)
#include "acatchtray.h"             // WhichAutoNeedTray / IsTrayArmMoveAvoidOutArmCrash
#include "aHotPlateSubstrate.h"     // OutArmSuck / FRCarryKit / BRCarryKit + cursors + SetMotorSpeed + iPlaceToAutoTask + InitOutArmTask + GetInArmPitchY_9045 + hOutArmYpitchHomeTimer
#include "acarry_shims.h"           // MoveOutArmToAutoSafe / MoveOutArmXY_ToShuttleAlarmArea / LastSet / NULL_IC / HAS_IC / HAS_NULL_IC
#include "acatchtray_shims.h"       // iWhichAuto offline helpers
#include "atester_shims.h"          // fObserver / fContact offline stand-ins
#include "ainarm9045_2x4_16_shims.h"// AutoTeachLoadTrayZ / InitProcessSingleMotorTask
#include "FormsFacade.h"            // fMain offline stand-in (Pause / chkInPickLoadError gated out by SOFT_SIMULTE)
#include "canary_support.h"         // ShowErrorMessage / K_RETRY / K_SKIP / K_HOME / DUMMY / Tempture_Hot / __FUNC__
#include "aoutarm9045_2x6_8.h"      // DoPickFromShuttle_9045_2x6_8 (sibling export called at golden case-2230)
//---------------------------------------------------------------------------
#pragma package(smart_init)
//==============================================================================
//extern bool  bScannerAlarm;
//static int OutArmPitchCylinderState=0;
//==============================================================================

//==============================================================================
//==  STANDALONE forward-declares for cross-module / engine-file-local symbols
//==  that have NO header home in scope this wave.  Guarded with #ifndef so the
//==  serial Integrate agent can add the real shared decl ONCE without clashing.
//==  Each is REPORTED in substrateNeeds (name + golden home + offline behavior).
//==  NOTE: several of these are file-local `static` in the engine aoutarm9045.cpp
//==  (NOT link-visible to this separate TU) -> they need a shared offline home.
//==============================================================================
#ifndef OUTARM_9045S_1x4_4_FWD_GUARD
#define OUTARM_9045S_1x4_4_FWD_GUARD
// shuttle pointer used by the geometry helpers (golden file-global, MyKitSuck.h)
extern TMyKitSuck *ptrOutSHT;                                                   // golden MyKitSuck.h -- offline: points at &FRCarryKit / &BRCarryKit grids
// out-arm <-> shuttle offset / ratio helpers (golden engine helpers, aoutarm9045.cpp)
void TransferOutShuttleRatio(int iSht, int *iX, int *iY, int iYBase, int iXBase);// golden aoutarm9045.cpp / Steven 20141029 -- offline: identity (no ratio transform)
int  GetVariableYOutShuttleData();                                              // golden aoutarm9045.cpp -- offline: 0
void TransferAutoRatio(int iOutPutTray, int *iX, int *iY);                      // golden aoutarm9045.cpp / Steven 20141029 -- offline: identity
// the no-suffix OutArmContinuousMove overload the golden S-file calls (golden tree has
// only OutArmContinuousMove_9045; this 8-arg overload is undeclared in golden too --
// further proof this file was never compiled in golden).  Forward-declared to the exact
// call shape so MY TU compiles; reported in substrateNeeds.
bool OutArmContinuousMove(int X, int Y, int iXVari, bool ZDownFlag[8], int ZDownPos[8], bool bZDown, int iYVari, int iXVari2); // golden mymotor (S overload) -- offline: false (no move)
// out-arm pick/data-swap surface (golden engine helpers)
void PickFromShuttle(int iWhichKit=0);                                          // golden aoutarm9045.cpp / kevin 20210612 -- offline: no-op
bool bOutShtwaitPick();                                                         // golden aoutarm9045.cpp / kevin 20210602 -- offline: false (no wait)
bool SwapShuttleDataToOutArm(int iSht, int iShtRow, int iShtCol, int iSuckRow, int iSuckCol, int *iWitchErrBin); // golden aoutarm9045.cpp / Steven 20170428 -- offline: false (no miss)
void ShowOutputShuttleDataMiss();                                              // golden aoutarm9045.cpp / Steven 20110527 -- offline: no-op
bool ErrorBinBoxDetect(int iWitchErrBin);                                      // golden aoutarm9045.cpp / kevin 20160906 -- offline: true
int  OutArmPickShuttleAlarm(int iSht, bool bHasDuplicateErr, AnsiString ErrPart);// golden aoutarm9045.cpp / Steven 20141121 -- offline: K_SKIP
void PorcessJAM0201OutArmPickUpErrorSkip(int iSht, int iShtRow, int iShtCol, int iSuckRow, int iSuckCol); // golden aoutarm9045.cpp / Steven 20161214 -- offline: no-op
bool CheckTesterZ(int iSht);                                                    // golden aoutarm9045.cpp -- offline: false (tester Z clear)
bool CheckRotateOutNotFinish();                                                // golden aoutarm9045.cpp / kevin 20130524 -- offline: false
bool CheckOutArmInitState();                                                    // golden aoutarm9045.cpp -- offline: true
bool CheckShuttleICPos(int iSht);                                              // golden aoutarm9045.cpp -- offline: true
void InitPickFromShuttle1Task();                                              // golden aoutarm9045.cpp / Sam 20230208 -- offline: no-op
void InitPickFromShuttle2Task();                                              // golden aoutarm9045.cpp / Sam 20230208 -- offline: no-op
// out-arm speed display helpers (golden engine helpers)
void OutArmSubSpeed();                                                          // golden aoutarm9045.cpp / Steven 20110525 -- offline: no-op
void OutArmAddSpeed();                                                          // golden aoutarm9045.cpp / Steven 20110525 -- offline: no-op
void OutArmSubSpeedDisplay();                                                   // golden aoutarm9045.cpp / KaiChen 20171225 -- offline: no-op
void OutArmAddSpeedDisplay();                                                   // golden aoutarm9045.cpp / KaiChen 20171225 -- offline: no-op
// out-arm move targets / catch-tray / clean-out / home (engine file-local statics)
bool MoveOutArmXY_ToFix_Tray_Full(bool bAlarm=false);                          // golden aoutarm.h:41 -- offline: true (reached)
bool IsCatchTrayReadySupplyNewTray();                                          // golden aoutarm.h:64 -- offline: false (catch-tray not ready)
int  CheckOutArmCleanOut();                                                     // golden aoutarm.h:58 -- offline: 1140 (continue normal pick flow)
void SetOutArmHome();                                                           // golden aoutarm.h:97 -- offline: no-op
// out-arm place / search / pitch / bin-box helpers (golden aoutarm9045.cpp / aoutarm.h)
int  AutoCalculateOutArmYClosePitch(int iOutPutTray);                          // golden aoutarm9045.cpp -- offline: 0
void AdjustOutArmClosePitchCondition(int iWhichTray, bool &bCanPick2ICAtOnceTime, int &iStep); // golden aoutarm9045.cpp -- offline: bCanPick2ICAtOnceTime=false
int  Find_OutArm_PickerMaxUseCountOnTime(int iMax, int &iStep, int &ix, int &iy, bool bLast); // golden aoutarm9045.cpp -- offline: 0
void Find_OutArm_Single(int iMax, int &ix, int &iy);                           // golden aoutarm9045.cpp -- offline: ix=iy=-1
void SearchBinBoxUpDown(bool &bCanPick2ICAtOnceTime, int &iStep, int &ix, int &iy); // golden aoutarm9045.cpp / Steven 20230601 -- offline: no-op
void OutArmAddBlockPitch(int &iXPos, int &iYPos, int ix, int iy);              // golden aoutarm9045.cpp -- offline: no-op
int  iCalculateOutArmXPitch(int iXPos, int iMin);                              // golden aoutarm9045.cpp / Steven 20151112 -- offline: 0
bool CheckUseFixBinBoxFunction();                                              // golden aoutarm9045.cpp / kevin 20160822 -- offline: false
void IfUseOnebyOne(int iWhichAuto);                                            // golden aoutarm9045.cpp / Steven 20111101 -- offline: no-op
bool DetectAutoTray(int iWhichAuto, int *iRetry);                              // golden aoutarm9045.cpp -- offline: true (tray detected)
int  VerifyTrayStatus();                                                        // golden aoutarm9045.cpp -- offline: 3300 (proceed to place)
bool DoFixTrayFullAlarm();                                                      // golden aoutarm9045.cpp / Steven 20181113 -- offline: true
void SetFixTrayFullIC();                                                        // golden aoutarm9045.cpp -- offline: no-op
bool CheckOutSuckICFallDown(bool bAlarm);                                       // golden aoutarm9045.cpp / Steven 20210916 -- offline: false (no fall)
// NOTE: CheckOutArmSuckICFallDown() is declared bool in csystem.h -- NOT re-declared here
//       (golden calls it as a void statement; the bool return is discarded).
void GetRowCol(int &iRow, int &iCol);                                          // golden atester (no header in scope here) -- offline: row/col from TestIF
bool MoveOutArmXY_ToShuttleAlarmArea();                                        // golden acarry_shims.h home -- offline: true
// magazine surface (golden Magazine.h, not in include subset)
bool CheckPlaceToMagazineTray(int iWhichMag);                                  // golden Magazine.h -- offline: true
bool DoPickFromMagazineBuffer();                                               // golden Magazine.h -- offline: true
int  SearchTrayToPlace_Magazine();                                             // golden aoutarm9045.h:63 -- AI(W906-W7-A2) 20260728: was void (ODR split vs. golden's int; see aoutarm_shims.cpp)
// out-arm fall-down / clean-out / fix-tray-full surface (engine helpers)
void InitOutArmTask();                                                          // golden aHotPlateSubstrate.h (also engine) -- offline: no-op
// y-pitch home predicates (golden ainarm9045.h:114/115; that header clashes -> declared here)
bool bCheckYPitchHome(int iXYHome);                                            // golden ainarm9045.h:114 / kevin 20180822 -- offline: true
bool bCheckYPitchRunHomeSen(int iXYHome);                                      // golden ainarm9045.h:115 / kevin 20180822 -- offline: true
// out-arm unload-motor index table (golden cmydef.h:4092 decl is COMMENTED OUT -> declared here)
extern int  iUnloadMotorIndex[eTrayCount];                                      // golden cmydef.h:4092 (commented) -- offline: per-tray unload motor id
// out-arm form pointer tables (golden cprod.h decls are COMMENTED OUT -> declared here)
extern TRAY_TYPE_PARA *OutputForm[eTrayCount];                                  // golden cprod.h:1370 (commented) -- offline: per-tray form geometry (XPitch/YPitch)
extern TRAY_TYPE_PARA *FixForm[eTrayCount];                                     // golden cprod.h:1367 (commented) -- offline: per-fix-tray form geometry (iTrayType)
// out-arm suck-active map + XY position cursors (golden file-globals)
extern bool bOutArmSuckActive[MAX_ARM_Row][MAX_ARM_Col];                        // golden aoutarm9045.cpp -- offline: per-picker active map
extern int  iOutArmXPosition[MAX_ARM_Row][MAX_ARM_Col];                         // golden aoutarm9045.cpp -- offline: per-picker tray X index
extern int  iOutArmYPosition;                                                   // golden aoutarm9045.cpp -- offline: tray Y index
extern int  iOutArmPlaceOrder;                                                  // golden aoutarm9045.cpp -- offline: 0
extern bool bOutArmXOverLimit;                                                  // golden aoutarm9045.cpp / Steven 20171206 -- offline: false
// out-arm Y-pitch home flag (golden file-global)
extern bool bYpitchNeddHome;                                                    // golden cmydef.h (also) -- offline flag
// carry-control flags (golden file-globals, jou 2011-11-21)
extern bool bCarryControlOutarm1;                                              // golden aoutarm9045.cpp -- offline: false
extern bool bCarryControlOutarm2;                                              // golden aoutarm9045.cpp -- offline: false
// out-arm Z auto-teach task cursor (golden file-global)
extern int  iOutArmZTeachTask;                                                 // golden aoutarm.h -- offline: cursor
// out-arm manual single-step press flag (golden file-global)
extern bool bOutArmManualStepPress;                                            // golden aoutarm9045.cpp -- offline: false
// per-shuttle pick task cursors (golden file-globals)
extern int  iPickFromShuttle1Task;                                             // golden aoutarm9045.cpp -- offline: SM cursor
extern int  iPickFromShuttle2Task;                                             // golden aoutarm9045.cpp -- offline: SM cursor
// the out-arm engine task cursor (golden file-global; engine OutArmTask)
extern int  OutArmTask;                                                        // golden aoutarm9045.cpp -- offline: engine SM cursor
// out-rotate handshake + place-target index + clean-out tray-full flags (golden file-globals)
extern int  iOutRotateFinish;                                                  // golden aoutarm9045.cpp / kevin 20130524 -- offline: 0
extern int  iWhichAuto;                                                        // golden aoutarm.h:18 -- offline: place-target Auto index
extern bool bOverTray;                                                         // golden aoutarm9045.cpp / jou 2012-09-04 -- offline: false
extern bool bUseOnebyOne;                                                      // golden aoutarm9045.cpp -- offline: false
extern bool bPickShuttleError;                                                 // golden aoutarm9045.cpp -- offline: false
extern bool bSortingAllBinTrayFinish;                                          // golden cmydef.h (also) / JerryYang 20150910 -- offline: false
// place-to-auto delay timer (golden file-global, jou 2010-12-01)
extern TQPF_Timer DoPlaceToAutoDelay;                                          // golden aoutarm9045.cpp -- offline: software timer
extern TQPF_Timer hOutArmYpitchHomeTimer;                                      // golden aoutarm.h (also substrate) -- offline: software timer
// bin-box geometry indices (golden file-globals / cmydef.h subset not in scope)
extern int  iBinBoxPos;                                                        // golden cmydef.h -- offline: bin-box Auto index
extern int  iMagPos;                                                           // golden cmydef.h -- offline: magazine Auto index
extern int  iBinBoxShiftY;                                                     // golden aoutarm9045.cpp -- offline: 0
// place-to-auto task cursor + InitPlaceToAutoTask (golden engine; here for SM)
void InitPlaceToAutoTask();                                                    // golden aoutarm9045.cpp -- offline: no-op
// per-tray place-to-auto (golden engine; the int-arg overload, distinct from the
// no-arg DoOutArmPlaceToAuto_9045() in aoutarm9045.h:69)
bool DoOutArmPlaceToAuto(int iWhichAuto);                                       // golden aoutarm9045.cpp / Steven 20110216 -- offline: true (placed)
#endif // OUTARM_9045S_1x4_4_FWD_GUARD

//==============================================================================
// 輸出臂 x y 軸移到 shuttle 1
//  Ver : 2003_07
//  -- golden aoutarm9045S_1x4_4.cpp:36
//==============================================================================
int GetNowShuttleMode_9045S_1x4_4(int iShuttle)
{
    if(iShuttle==0)
        ptrOutSHT=&FRCarryKit;
    else
        ptrOutSHT=&BRCarryKit;

    if((iShuttle==0 && ptrOutSHT->HasIC()==false) ||
       (iShuttle==1 && ptrOutSHT->HasIC()==false))
    {
        return 1;
    }
    else
    {
        if(ptrOutSHT->Item[0][0] || ptrOutSHT->Item[0][1])
            return 1;
        else
            return 2;
    }
}
//==============================================================================
//  OutArmZNeedDown_9045S_1x4_4  -- golden aoutarm9045S_1x4_4.cpp:57
//==============================================================================
bool OutArmZNeedDown_9045S_1x4_4(int iShuttle)                                  //kevin 20120612 ourarm double pick
{
    if(InitialOutArmNeedSuck(iShuttle)==false)
        return false;

    int i, j;
    int Mode=GetNowShuttleMode_9045S_1x4_4(iShuttle);

    for(i=0; i<2; i++)
    {
        for(j=0; j<2; j++)
        {
            if(Mode==1 && i==0)                                                 //Aa & Ab on Suck[0][0] & Suck[0][2]
            {
                if(ptrOutSHT->Item[0][j])
                    OutArmSuck.Suck[i][j*2].SetNeedSuck(true);
            }
            else if(Mode==2 && i==1)                                            //Ac & Ad on Suck[1][0] & Suck[1][2]
            {
                if(ptrOutSHT->Item[0][j+2])
                    OutArmSuck.Suck[i][j*2].SetNeedSuck(true);
            }
        }
    }

    return true;
}
//==============================================================================
//  GetOutShuttleStatus_9045S_1x4_4  -- golden aoutarm9045S_1x4_4.cpp:84
//==============================================================================
void GetOutShuttleStatus_9045S_1x4_4(bool *flag)
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
//  CheckOutArmXYPitch_9045S_1x4_4  -- golden aoutarm9045S_1x4_4.cpp:100
//------------------------------------------------------------------------------
bool CheckOutArmXYPitch_9045S_1x4_4(int *iX, int *iY, int iShuttle)             //kevin 20120612 ourarm double pick fix
{
    int iMode=GetNowShuttleMode_9045S_1x4_4(iShuttle);
    int iMovePitchY=GetVariableYOutShuttleData();

//    if(iMode==1)        //Aa & Ab on Suck[0][0] & Suck[0][2]
//    {
        if(USE_IN_OUT_ARM_Y_PITCH==iXYPitchVariable)                            //ChungHung 20140304 add for AutoYPitch
        {
            *iX-=TestIF.dSiteXPitch/2;
            *iY-=iMovePitchY;
        }
        else
        {
            *iX-=TestIF.dSiteXPitch/2;
//            *iY+=3000;
        }
//    }

    if(iMode==2)                                                                //Ac & Ad on Suck[1][0] & Suck[1][2]
    {
        *iX+=TestIF.dSiteXPitch*2;
        *iY+=iMovePitchY;
    }

    if(iMode==0 || iMode==1 || iMode==3 || iMode==5)
    {
        if(iShuttle==0)
        {
            *iX+=Prod.XOutArm_Shuttle1_Pick[iOutArmYBase][iOutArmXBase];
            *iY+=Prod.YOutArm_Shuttle1_Pick[iOutArmYBase][iOutArmXBase];
        }
        else
        {
            *iX+=Prod.XOutArm_Shuttle2_Pick[iOutArmYBase][iOutArmXBase];
            *iY+=Prod.YOutArm_Shuttle2_Pick[iOutArmYBase][iOutArmXBase];
        }

        TransferOutShuttleRatio(iShuttle, iX, iY, iOutArmYBase, iOutArmXBase);  //Steven 20141029 : Out Shuttle的軟體齒輪比
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
//    else if(iMode==4 || iMode==6)
//    {
//        if(iShuttle==0)
//        {
//            *iX+=Prod.XOutArm_Shuttle1_Pick[1][0];
//            *iY+=Prod.YOutArm_Shuttle1_Pick[1][0];
//        }
//        else
//        {
//            *iX+=Prod.XOutArm_Shuttle2_Pick[1][0];
//            *iY+=Prod.YOutArm_Shuttle2_Pick[1][0];
//        }
//
//        TransferOutShuttleRatio(iShuttle, iX, iY, 1, 0);       //Steven 20141029 : Out Shuttle的軟體齒輪比
//    }
//    else
//    {
//        if(iShuttle==0)
//        {
//            *iX+=Prod.XOutArm_Shuttle1_Pick[1][1];
//            *iY+=Prod.YOutArm_Shuttle1_Pick[1][1];
//        }
//        else
//        {
//            *iX+=Prod.XOutArm_Shuttle2_Pick[1][1];
//            *iY+=Prod.YOutArm_Shuttle2_Pick[1][1];
//        }
//
//        TransferOutShuttleRatio(iShuttle, iX, iY, 1, 1);       //Steven 20141029 : Out Shuttle的軟體齒輪比
//    }

    if(OutArmZNeedDown_9045S_1x4_4(iShuttle)==false)                            //kevin 20120612 ourarm double pick  start
       return false;
    return true;                                                                //kevin 20120612 end
}
//==============================================================================
//  MoveOutArmToShuttleIncludeZ_9045S_1x4_4  -- golden aoutarm9045S_1x4_4.cpp:191
//==============================================================================
bool MoveOutArmToShuttleIncludeZ_9045S_1x4_4(int iShuttle, bool bZDown)         //kevin 20210612 Out arm add Z bZDown= true 往下
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

    if(TestIF.dSiteXPitch>iXpitchMaxX2)                                         //X Pitch is 4000mm   //Steven 20150330 : X-Pitch > 80mm
        iMovePitchX=iXpitchMaxX3;                                               //Isaac 20171204 (Steven) : Xpitch40->50mm, 12000->iXpitchMaxX3
    else
        iMovePitchX=TestIF.dSiteXPitch+TestIF.dSiteXPitch/2;

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
            iYVariable+=OutArmOffSet[OutOfsOutSh1]->GetVariableY();
            iXVariable2+=OutArmOffSet[OutOfsOutSh1]->GetVariable2();
        }
    }
    else
    {
        if(iShuttle==0)
        {
            iXVariable+=OutArmOffSet[OutOfsOutSh1]->GetVariable();
            if(USE_IN_OUT_ARM_Y_PITCH==iXYPitchVariable)                        //ChungHung 20131231 alter AutoYPitch
            {
                iYVariable+=OutArmOffSet[OutOfsOutSh1]->GetVariableY();
                iXVariable2+=OutArmOffSet[OutOfsOutSh1]->GetVariable2();
            }
        }
        else
        {
            iXVariable+=OutArmOffSet[OutOfsOutSh2]->GetVariable();
            if(USE_IN_OUT_ARM_Y_PITCH==iXYPitchVariable)                        //ChungHung 20131231 alter AutoYPitch
            {
                iYVariable+=OutArmOffSet[OutOfsOutSh2]->GetVariableY();
                iXVariable2+=OutArmOffSet[OutOfsOutSh2]->GetVariable2();
            }
        }
    }

    iXPos+=iOutArmShtXCenterPos;                                                //Steven 20141029 : XY-Pitch for Shuttle Center position for base Suck
    iYPos+=iOutArmShtYCenterPos;

    if(CheckOutArmXYPitch_9045S_1x4_4(&iXPos, &iYPos, iShuttle)==false && bZDown==true)                                 //kevin 20120612 ourarm double pick
         return false;

    //jou 981208 start : NS7000 bias kit
    if(TestIF.bNS7000kit)
    {
        if(iShuttle==0)                                                         //shuttle 1
            iYPos+=iMovePitchY/2;
        else                                                                    //shuttle 2
            iYPos-=iMovePitchY/2;
    }
    //jou 981208 end

    if(CUSTOMER_CODE==CC_TSMC_TAINAN && TestIF_File.bQualSite2X2Shift)          //wei 20160226 TSMC X Shift
    {
        iXPos+=TestIF.dShiftXPitch;
    }

    GetOutShuttleStatus_9045S_1x4_4(&ZDownFlag[0]);                             //一定要在CheckXYPitch()function後面
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

    DoOutArmSuckPreOn(iShuttle, 0, iXPos, iYPos, bZDown);
    if(OutArmContinuousMove(iXPos, iYPos, iXVariable, ZDownFlag, ZDownPos, bZDown, iYVariable, iXVariable2))            //kevin 20210612 Out arm add Z bZDown= true 往下
        return true;

    return false;
}
//==============================================================================
// 輸出臂從 Shuttle 1 吸取 IC 流程動作
//  Ver : 2003_07
//  -- golden aoutarm9045S_1x4_4.cpp:296
//==============================================================================
bool DoPickFromShuttle1_9045S_1x4_4(bool IncludeSHT)
{
    int &Task=iPickFromShuttle1Task, ret, i, j;
    static bool bSuckDuplicateErr[2][4]={{false, false, false, false}, {false, false, false, false}};                   //Steven 20091218 : Avoid duplicate message
    static int iRetryCT=0, iWitchErrBin=0;                                      //kevin 20160906

    AnsiString ErrPart="";                                                      //Steven 20110216 : 合併Alarm
    bool bHasDuplicateErr=false;                                                //Steven 20110216 : 合併Alarm
    bool bHasErr=false;                                                         //Steven 20110216 : 合併Alarm
    bool flag=false;
    int SHTHasIC[MAX_Index_Row][MAX_Index_Col];
    int iSuckX, iSuckY, iShtX, iShtY;

    if(CheckTesterZ(1))                                                         // && Task!=iOUTARM_SUCK)
    {
        //MoveOutArmToAutoSafe();           //Steven 20150408 : Mark,避免吸嘴上上下下
        return false;
    }

    switch(Task)
    {
        case 1:
            if(ArmSpeed[OutArm].dWaitOnSH!=0)                                   //kevin 20210525 out arm 起動在shuttle 上等待 時間
            {
                if(OutArmSuck.NoIC())                                           //kevin 20210615 第一次 吸取IC 需等待
                {
                    if(MoveOutArmToShuttleIncludeZ_9045S_1x4_4(0, false))       //移到 Shuttle 上等待
                    {
                        PickFromShuttle(0);                                     //kevin 20210612 add int iwhichKit=0
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
            if(bOutShtwaitPick())                                               //kevin 20210602 Out arm sht on wait
                Task=10;
            break;
        case 10:
            iWitchErrBin=0;                                                     //kevin 20160906
            Task=200;
        case 200:
            if(MoveOutArmToShuttleIncludeZ_9045S_1x4_4(0, true))                                                                                                //kevin 20210717 change function
            {
                if((AccessLevel>=1 || CosFunction.bOPCanPressStepAndTStart) &&                                                                                  //JerryYang 20170417 (wei) OP權限也能按Step跟T.Start
                   (Sen[SnRKManualStep].IsOn() || fOffSet->UseAutoOffsetFunction("Output Shuttle1") || /*AI(W6.2c-9045S_1x4_4) 20260627 GATE: offline TfOffSet has UseInArmSetupTeach only, no UseOutArmSetupTeach -> golden term `fOffSet->UseOutArmSetupTeach(OutOfsOutSh1)` gated false*/false))             //JerryYang 20180921 Setup Teach功能 //jou 2013-08-29 Use Auto Offset Funtion
                {
                    bEnterOffset=false;
                    fMain->Pause(__FUNC__);                                    //AI(W6.2c-9045S_1x4_4) 20260627 offline TfMain::Pause takes AnsiString (golden no-arg) -> pass __FUNC__; offline never pauses

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
            for(i=0; i<1; i++)
            {
                for(j=0; j<4; j++)
                {
                    SHTHasIC[i][j]=FRCarryKit.Item[i][j];
                }
            }

            bHasErr=false;
            for(i=0; i<2; i++)
            {
                for(j=0; j<2; j++)
                {
                    if(i==0)
                    {
                        iSuckX  =i;
                        iSuckY  =j*2;
                        iShtX   =0;
                        iShtY   =j;
                    }
                    else
                    {
                        iSuckX  =i;
                        iSuckY  =j*2;
                        iShtX   =0;
                        iShtY   =j+2;
                    }

                    if(SHTHasIC[iShtX][iShtY] && OutArmSuck.Suck[iSuckX][iSuckY].GetNeedSuckStatus())
                    {
                        if(OutArmSuck.Suck[iSuckX][iSuckY].Suck())
                        {
                            //模擬ic不見了
                            #ifdef SOFT_SIMULTE
                            if(fMain->chkInPickLoadError->Checked==true)
                            {
                                if(i==1 && j==0)
                                {
                                    OutArmSuck.Suck[1][0].Error=true;
                                    continue;
                                }
                            }
                            #endif
                            //模擬ic不見了 end

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

            for(i=0; i<2; i++)
            {
                for(j=0; j<2; j++)
                {
                    if(OutArmSuck.Suck[i][j*2].Error)
                    {
                        iRetryCT++;
                        Task=2000;
                        return false;
                    }
                }
            }

            //if(flag==false)
            //    break;

            for(i=0; i<2; i++)
                for(j=0; j<2; j++)
                    if(OutArmSuck.Item[i][j*2]==NULL_IC && OutArmSuck.Suck[i][j*2].GetNeedSuckStatus())
                        return false;

            if(iHWFix_BinBox==1 && ErrorBinBoxDetect(iWitchErrBin)==false)
                return false;                                                                                           //kevin 20160906 add error bin box

            iRetryCT=0;
            Task=1;
            return true;
        case 2000:
            if(MoveOutArmToAutoSafe())
            {
                //Steven 20110307 Start : 整合Alarm
                bHasErr=false;
                ErrPart=" ";
                bHasDuplicateErr=false;
                for(i=0; i<2; i++)
                {
                    for(j=0; j<2; j++)
                    {
                        if(bSuckDuplicateErr[i][j*2])
                            bHasDuplicateErr=true;

                        if(OutArmSuck.Suck[i][j*2].Error)
                        {
                            ErrPart+=OutArmSuck.Suck[i][j*2].sName;
                            bHasErr=true;
                        }
                        else
                        {
                            bSuckDuplicateErr[i][j*2]=false;
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

                        for(i=0; i<2; i++)
                        {
                            for(j=0; j<2; j++)
                            {
                                if(OutArmSuck.Suck[i][j*2].Error)
                                    bSuckDuplicateErr[i][j*2]=true;
                                else
                                    bSuckDuplicateErr[i][j*2]=false;
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
                        for(i=0; i<2; i++)
                        {
                            for(j=0; j<2; j++)
                            {
                                if(i==0)
                                {
                                    iSuckX  =i;
                                    iSuckY  =j*2;
                                    iShtX   =0;
                                    iShtY   =j;
                                }
                                else
                                {
                                    iSuckX  =i;
                                    iSuckY  =j*2;
                                    iShtX   =0;
                                    iShtY   =j+2;
                                }

                                if(OutArmSuck.Suck[iSuckX][iSuckY].Error)
                                {
                                    PorcessJAM0201OutArmPickUpErrorSkip(0, iShtX, iShtY, iSuckX, iSuckY);               //Steven 20161214 : 加上OutArm吸取異常Skip的ErrorLog
                                }
                                bSuckDuplicateErr[iSuckX][iSuckY]=false;
                            }
                        }
                        iRetryCT=0;                                                                                     //Steven 20190313 : Fixed for out arm pick error need home
                        Task=1;
                        return true;                                                                                    //kevin 20170731 (Steven) add 沒將手背上IC 放置UNLOAD 會一直ALARM
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
                //Steven 20110307 End
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
//  DoPickFromShuttle2_9045S_1x4_4  -- golden aoutarm9045S_1x4_4.cpp:588
//------------------------------------------------------------------------------
bool DoPickFromShuttle2_9045S_1x4_4(bool IncludeSHT)
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
    int iSuckX, iSuckY, iShtX, iShtY;

    if(CheckTesterZ(2))                                                         // && Task!=iOUTARM_SUCK)
    {
        //MoveOutArmToAutoSafe();           //Steven 20150408 : Mark,避免吸嘴上上下下
        return false;
    }

    switch(Task)
    {
        case 1:
            if(ArmSpeed[OutArm].dWaitOnSH!=0)                                   //kevin 20210525 out arm 起動在shuttle 上等待 時間
            {
                if(OutArmSuck.NoIC())                                           //kevin 20210615 第一次 吸取IC 需等待
                {
                    if(MoveOutArmToShuttleIncludeZ_9045S_1x4_4(1, false))       //移到 Shuttle 上等待
                    {
                        PickFromShuttle(0);                                     //kevin 20210612 add int iwhichKit=0
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
            if(bOutShtwaitPick())                                               //kevin 20210602 Out arm sht on wait
                Task=10;
            break;
        case 10:
            iWitchErrBin=0;                                                     //kevin 20160906
            Task=200;
        case 200:
            if(MoveOutArmToShuttleIncludeZ_9045S_1x4_4(1, true))                                                                                                //kevin 20210717 change function
            {
                if((AccessLevel>=1 || CosFunction.bOPCanPressStepAndTStart) &&                                                                                  //JerryYang 20170417 (wei) OP權限也能按Step跟T.Start
                   (Sen[SnRKManualStep].IsOn() || fOffSet->UseAutoOffsetFunction("Output Shuttle2") || /*AI(W6.2c-9045S_1x4_4) 20260627 GATE: offline TfOffSet has UseInArmSetupTeach only, no UseOutArmSetupTeach -> golden term `fOffSet->UseOutArmSetupTeach(OutOfsOutSh2)` gated false*/false))             //JerryYang 20180921 Setup Teach功能 //jou 2013-08-29 Use Auto Offset Funtion
                {
                    bEnterOffset=false;
                    fMain->Pause(__FUNC__);                                    //AI(W6.2c-9045S_1x4_4) 20260627 offline TfMain::Pause takes AnsiString (golden no-arg) -> pass __FUNC__; offline never pauses

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
                bOutArmManualStepPress=true;
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
            for(i=0; i<2; i++)
                for(j=0; j<4; j++)
                    SHTHasIC[i][j]=BRCarryKit.Item[i][j];

            bHasErr=false;
            for(i=0; i<2; i++)
            {
                for(j=0; j<2; j++)
                {
                    if(i==0)
                    {
                        iSuckX  =i;
                        iSuckY  =j*2;
                        iShtX   =0;
                        iShtY   =j;
                    }
                    else
                    {
                        iSuckX  =i;
                        iSuckY  =j*2;
                        iShtX   =0;
                        iShtY   =j+2;
                    }

                    if(SHTHasIC[iShtX][iShtY] && OutArmSuck.Suck[iSuckX][iSuckY].GetNeedSuckStatus())
                    {
                        if(OutArmSuck.Suck[iSuckX][iSuckY].Suck())
                        {
                            //模擬ic不見了
                            #ifdef SOFT_SIMULTE
                            if(fMain->chkInPickLoadError->Checked==true)
                            {
                                if(i==1 && j==0)
                                {
                                    OutArmSuck.Suck[1][0].Error=true;
                                    continue;
                                }
                            }
                            #endif
                            //模擬ic不見了 end

                            bHasErr=SwapShuttleDataToOutArm(1, iShtX, iShtY, iSuckX, iSuckY, &iWitchErrBin);            //Steven 20170428 : 統一Out Arm從Shuttle吸料後的資料交換
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

            for(i=0; i<2; i++)
            {
                for(j=0; j<2; j++)
                {
                    if(OutArmSuck.Suck[i][j*2].Error)
                    {
                        iRetryCT++;
                        Task=2000;
                        return false;
                    }
                }
            }

            iRetryCT=0;
            for(i=0; i<2; i++)
                for(j=0; j<2; j++)
                    if(OutArmSuck.Item[i][j*2]==NULL_IC && OutArmSuck.Suck[i][j*2].GetNeedSuckStatus())
                        return false;

            if(iHWFix_BinBox==1 && ErrorBinBoxDetect(iWitchErrBin)==false)
                return false;                                                                                           //kevin 20160906 add error bin box

            Task=1;
            return true;
        case 2000:
            if(MoveOutArmToAutoSafe())
            {
                //Steven 20110307 Start : 整合Alarm
                ErrPart=" ";
                bHasDuplicateErr=false;
                bHasErr=false;
                for(i=0; i<2; i++)
                {
                    for(j=0; j<2; j++)
                    {
                        if(bSuckDuplicateErr[i][j*2])
                            bHasDuplicateErr=true;

                        if(OutArmSuck.Suck[i][j*2].Error)
                        {
                            ErrPart+=OutArmSuck.Suck[i][j*2].sName;
                            bHasErr=true;
                        }
                        else
                        {
                            bSuckDuplicateErr[i][j*2]=false;
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

                        for(i=0; i<2; i++)
                        {
                            for(j=0; j<2; j++)
                            {
                                if(OutArmSuck.Suck[i][j*2].Error)
                                    bSuckDuplicateErr[i][j*2]=true;
                                else
                                    bSuckDuplicateErr[i][j*2]=false;
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
                        for(i=0; i<2; i++)
                        {
                            for(j=0; j<2; j++)
                            {
                                if(i==0)
                                {
                                    iSuckX  =i;
                                    iSuckY  =j*2;
                                    iShtX   =0;
                                    iShtY   =j;
                                }
                                else
                                {
                                    iSuckX  =i;
                                    iSuckY  =j*2;
                                    iShtX   =0;
                                    iShtY   =j+2;
                                }

                                if(OutArmSuck.Suck[iSuckX][iSuckY].Error)
                                {
                                    PorcessJAM0201OutArmPickUpErrorSkip(1, iShtX, iShtY, iSuckX, iSuckY);               //Steven 20161214 : 加上OutArm吸取異常Skip的ErrorLog
                                }
                                bSuckDuplicateErr[iSuckX][iSuckY]=false;
                            }
                        }
                        iRetryCT=0;                                                                                     //Steven 20190313 : Fixed for out arm pick error need home
                        Task=1;
                        return true;                                                                                    //kevin 20170731 (Steven) add 沒將手背上IC 放置UNLOAD 會一直ALARM
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
                //Steven 20110307 End
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
//  DoMoveOutArmXYToPlace_9045S_1x4_4  -- golden aoutarm9045S_1x4_4.cpp:873
//==============================================================================
bool DoMoveOutArmXYToPlace_9045S_1x4_4(bool &bCanPick2ICAtOnceTime, int &ix, int &iy, int &iStep, bool ReadMove)
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
        iMovePitchX=(OutArmClose_Pitch*3)/2;
        if(USE_IN_OUT_ARM_Y_PITCH==iXYPitchVariable)                            //ChungHung 20140304 add for AutoYPitch
        {
            iTempXpos+=iMovePitchX/3;                                           //吸嘴A
        }
        else
        {
            iTempXpos+=OutArmClose_Pitch;                                       //Steven 20141217 : OutArmClose_Pitch*2 --> OutArmClose_Pitch
        }

        if(bOutArmSuckActive[iOutArmPlaceOrder][0]==false)
        {
            iTempXpos-=OutArmClose_Pitch;
        }

        if(iTempXpos<(MOT[MOutArmX].Motor->PSoftLimitN+10) ||
           iTempXpos>(MOT[MOutArmX].Motor->PSoftLimitP-100))                    //Steven 20141217 : For Auto 1使用G & H吸嘴
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
                    if(ArmSpeed[OutArm].bVariModeFIX==true || bUseOnebyOne==true)
                        iMovePitchX=iXpitchMaxX3;
                    else
                        iMovePitchX=(OutArmClose_Pitch*3)/2;

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
                       iXPos>(MOT[MOutArmX].Motor->PSoftLimitP-100))            //Steven 20141217 : For Auto 1使用G & H吸嘴
                    {
                        iMovePitchX=iCalculateOutArmXPitch(iXPos, 1);           //Steven 20151112 : 要判斷IC尺寸, 如果尺寸小於13mm,就不能縮到40mm
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
                //AI(W6.2c-9045S_1x4_4) 20260627 substrate GetInArmPitchY_9045 has a 2nd
                //  iOffsetPos param (default -1 in engine header ainarm9045.h:25); golden
                //  calls 1-arg -> pass the engine default -1 to compile, semantics unchanged.
                iYVariable=GetInArmPitchY_9045(iMovePitchY, -1);
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
//  SearchUnLoadTrayUpDown_9045S_1x4_4  -- golden aoutarm9045S_1x4_4.cpp:1105
//==============================================================================
bool SearchUnLoadTrayUpDown_9045S_1x4_4(bool &bCanPick2ICAtOnceTime, int &iStep, int &ix, int &iy)
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
        if(ct<=1 && TestIF.bSearchLastMode==false)
        {
            bCanPick2ICAtOnceTime=false;
        }
        else if(iWhichAuto==iAutoRight)
        {
            if(DoMoveOutArmXYToPlace_9045S_1x4_4(bCanPick2ICAtOnceTime, ix, iy, iStep, false)==true)
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
//  SetOutArm_9045S_1x4_4  -- golden aoutarm9045S_1x4_4.cpp:1144
//==============================================================================
bool SetOutArm_9045S_1x4_4()
{
    int iStep, ix=0, iy=0;
    bool bCanPick2ICAtOnceTime;
    if(IsTrayArmMoveAvoidOutArmCrash())
    {
        MOT[MOutArmX].PCIL132_StopMotor();
        MOT[MOutArmY].PCIL132_StopMotor();
        return false;
    }

    if(SearchUnLoadTrayUpDown_9045S_1x4_4(bCanPick2ICAtOnceTime, iStep, ix, iy)==false)
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

    return  DoMoveOutArmXYToPlace_9045S_1x4_4(bCanPick2ICAtOnceTime, ix, iy, iStep, true);
}
//------------------------------------------------------------------------------
//  DoPlaceToAuto_9045S_1x4_4  -- golden aoutarm9045S_1x4_4.cpp:1176
//------------------------------------------------------------------------------
bool DoPlaceToAuto_9045S_1x4_4()
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

            flag=SetOutArm_9045S_1x4_4();
            if(flag)
            {
                Task=50;
            }
            else                                                                //jou 2012-09-04 修正極限無法到達時,回填Null_IC會造成找不到地方放,會放在 x=-1,y=-1的位置上
            {
                if(bOverTray==true)
                {
                    //==> AI(W6.2c-9045S_1x4_4) 20260627 GATE: golden calls the 1-arg
                    //    MOT[...].SetTray(HAS_IC) but the translated TTrayMotor::SetTray
                    //    signature is SetTray(int data, AnsiString Func) -- 2 args, no
                    //    default (mymotor.h:350; golden mymotor.h:237 is identical 2-arg).
                    //    The golden 1-arg call therefore would NOT compile in golden either
                    //    -- a further confirmation this file is DEAD-but-compiled (never
                    //    built in golden).  Gated VERBATIM; active stub preserves intent
                    //    (mark the over-limit tray full) via the 2-arg form.
                    #if 0 // TODO(W7): restore golden 1-arg SetTray once the SetTray overload set is reconciled
                    MOT[iUnloadMotorIndex[ct]].SetTray(HAS_IC);
                    #else
                    MOT[iUnloadMotorIndex[ct]].SetTray(HAS_IC, __FUNC__);
                    #endif
                    //<== AI(W6.2c-9045S_1x4_4) 20260627
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
                ct=iWhichAuto;
                if(ct>=iFixRight)
                    ct-=3;                                                                                                                                      //QQQ
                ct-=2;
                asAutoOffset="Fix"+AnsiString(ct);
            }

            if((AccessLevel>=1 || CosFunction.bOPCanPressStepAndTStart) &&                                                                                      //JerryYang 20170417 (wei) OP權限也能按Step跟T.Start
               (Sen[SnRKManualStep].IsOn() || fOffSet->UseAutoOffsetFunction(asAutoOffset) || /*AI(W6.2c-9045S_1x4_4) 20260627 GATE: offline TfOffSet has UseInArmSetupTeach only, no UseOutArmSetupTeach -> golden term `fOffSet->UseOutArmSetupTeach(OutOfsAuto1+iWhichAuto)` gated false*/false))            //JerryYang 20180921 Setup Teach功能 //jou 2013-08-29 Use Auto Offset Funtion
            {
                bEnterOffset=false;
                fMain->Pause(__FUNC__);                                        //AI(W6.2c-9045S_1x4_4) 20260627 offline TfMain::Pause takes AnsiString (golden no-arg) -> pass __FUNC__; offline never pauses
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
//  DoOutArm_9045S_1x4_4  -- golden aoutarm9045S_1x4_4.cpp:1372  (DEAD top-level export)
//==============================================================================
void DoOutArm_9045S_1x4_4()
{
    int iRow, iCol;
    int ret, j, iMode1=0, iMode2=0;
    //AI(W6.2c-9045S_1x4_4) 20260627 golden DoOutArm_9045S_1x4_4 uses `ct` at case-3300
    //  (golden lines 2198/2209) but never declares it -- a latent compile error in the
    //  golden source itself (further proof this file is DEAD/never compiled in golden).
    //  Declared here so MY faithful translation compiles; usage/semantics unchanged.
    int ct;
    static int OldPos=1,i;
    static bool FirstEnter=true,bCheckSpeed=false, bOutArmNeedMoveToSht=false;  //JerryYang 20170828 add bOutArmNeedMoveToSht
    static bool bTrayDuplicateErr[9]={false, false, false, false, false, false, false, false, false};                   //Steven 20120208 : bTrayDuplicateErr_Define
    int &Task=OutArmTask;
    static int iWhichShuttle=0, iBackMode1=0, iBackMode2=0;
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
                if((FRCarryKit.HasIC()) && OutSHT1InRT())
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
                    OldPos=1;
                else
                    OldPos=0;
            }

            if(CheckRotateOutNotFinish())                                       //kevin 20130524 發生alarm inarm到等待點
            {
                Task=7000;
                break;                                                          //kevin 20130524
            }

            if(OldPos==1)
            {
                iWhichShuttle=0;
                Task=1000;
            }
            else
            {
                iWhichShuttle=1;
                Task=2000;
            }
            OldPos=1-OldPos;
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

            if(FRCarryKit.HasIC()==false && OutSHT1InRT())
            {
                if(BRCarryKit.HasIC() && OutSHT2InRT())
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
                        iOutArmiWhichKit=0;
                        InitPickFromShuttle1Task();                                                                                                             //Sam 20230208 : 修正 Fix AOI 未拍照問題
                        //iPickFromShuttle1Task=1;
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

            if(Task!=1150)                                                                                                                                      //Steven 20180813 : add Out arm speed
                break;
        case 1150:
            if(bCheckSpeed)                                                                                                                                     //Steven 20110525 : Auto Speed
            {
                bCheckSpeed=false;
                OutArmSubSpeed();
            }

            if(bCheckSpeed1)
            {
                bCheckSpeed1=false;
                OutArmSubSpeedDisplay();
            }

            if(MoveOutArmToShuttleIncludeZ_9045S_1x4_4(iWhichShuttle, false))
            {
                IsCatchTrayReadySupplyNewTray();

                if(XPitchIsStand()==false)                                                                                                                      //Jou 20110328
                {
                    iBackMode1=GetNowShuttleMode_9045S_1x4_4(0);
                }

                if(IniConfig.bA26MotorSpeedSortDisplay)                                                                                                         //KaiChen 20171225 (Steven)：Add Speed Display
                    bCheckSpeed1=true;
                else
                    bCheckSpeed1=false;
                Task=1100;
            }

            if(Task!=1100)                                                                                                                                      //Steven 20180813 : add Out arm speed
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
                //<== Eastsun 20260521 整合
                else
                {
                    MoveOutArmXY_ToFix_Tray_Full();                                                                                                                 //Steven 20231027 : 先檢查有沒有要換Tray
                    Task=1160;
                }
            }
            else if(FRCarryKit.HasIC() && OutSHT1InRT() && bCheckShuttle1Flag==false)
            {
//#ifdef Carry4
//                MOT[MOutShuttle1].fCanMoveR=false;
//#else
                MOT[MInShuttle1].fCanMoveR=false;
//#endif

                iOutArmiWhichKit=0;
                InitPickFromShuttle1Task();
                Task=1230;

                if(XPitchIsStand()==false)                                                                                                                      //Jou 20110328
                {
                    iMode1=GetNowShuttleMode_9045S_1x4_4(0);
                    if(iMode1!=iBackMode1)
                    {
                        Task=1150;
                        break;
                    }
                    else
                    {
                        iMode1=0;
                        iBackMode1=0;
                    }
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

            IsCatchTrayReadySupplyNewTray();
            if(DoPickFromShuttle1_9045S_1x4_4(true))
            {
                PreSetOutAdditionalFlag();                                                                                                                      //Steven 20220614 : Fixed for out arm additional function
                if(FRCarryKit.HasIC())
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
                    return;
                }

                if(OutArmSuck.HasIC()==false)
                {
                    Task=50;
                }
                else
                {
                    iOutRotateFinish=0;                                                                                                                         //kevin 20130524 目前放到 auto tray
//                    iOutArmPickPlaceCnt[OutOfsOutSh1]++;                      //JerryYang 20230523 : 沒在用, Mark掉  //JerryYang 20180921 Setup Teach功能
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
        case 1230:
            IsCatchTrayReadySupplyNewTray();

            if(bPickShuttleError)
            {
                bPickShuttleError=false;
                InitPickFromShuttle1Task();
                Task=1200;
                break;
            }

            if(DoPickFromShuttle1_9045S_1x4_4(false))
            {
                PreSetOutAdditionalFlag();                                                                                                                      //Steven 20220614 : Fixed for out arm additional function
                if(FRCarryKit.HasIC())
                {
                    Task=1200;
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
                        iOutArmiWhichKit=0;
                        InitPickFromShuttle2Task();                                                                                                             //Sam 20230208 : 修正 Fix AOI 未拍照問題
                        //iPickFromShuttle2Task=1;
                        Task=2200;
//                        #ifdef Carry4
//                            MOT[MOutShuttle2].fCanMoveR=false;
//                        #else
                            MOT[MInShuttle2].fCanMoveR=false;
//                        #endif
                    }
                }
                else
                {
                    Task=2050;
                }
            }
            else
            {
                Task=2050;
            }
            break;
        case 2050:
            if(bCheckSpeed)                                                                                                                                     //Steven 20110525 : Auto Speed
            {
                bCheckSpeed=false;
                OutArmSubSpeed();
            }

            if(bCheckSpeed1)                                                                                                                                    //KaiChen 20171225 (Steven)：Add Speed Display
            {
                bCheckSpeed1=false;
                OutArmSubSpeedDisplay();
            }

            if(MoveOutArmToShuttleIncludeZ_9045S_1x4_4(iWhichShuttle, false))
            {
                IsCatchTrayReadySupplyNewTray();

                if(XPitchIsStand()==false)                                                                                                                      //Jou 20110328
                {
                    iBackMode2=GetNowShuttleMode_9045S_1x4_4(1);
                }
                Task=2100;
            }

            if(Task!=2100)                                                                                                                                      //Steven 20180813 : add Out arm speed
                break;
        case 2100:
            IsCatchTrayReadySupplyNewTray();

            if(WhichAutoNeedTray()!=0)                          //JerryYang 20170731 (wei) Out arm要讓位才不會hang up
            {
                if(IniConfig.bA15_1ESDGiveWayFunction==true && USE_BU5_Function==true)
                {
                    MoveOutArmXY_To_ESDSafePos();                                 //Steven 20231027 : 先檢查有沒有要換Tray
                    Task=2170;
                }
            }
            else if(BRCarryKit.HasIC() && OutSHT2InRT() && bCheckShuttle2Flag==false)
            {
//#ifdef Carry4
//                MOT[MOutShuttle2].fCanMoveR=false;
//#else
                MOT[MInShuttle2].fCanMoveR=false;
//#endif
                iOutArmiWhichKit=0;
                InitPickFromShuttle2Task();
                //iPickFromShuttle2Task=1;
                Task=2230;

                if(XPitchIsStand()==false)                                                                                                                      //Jou 20110328
                {
                    iMode2=GetNowShuttleMode_9045S_1x4_4(1);
                    if(iMode2!=iBackMode2)
                    {
                        Task=2050;
                        break;
                    }
                    else
                    {
                        iMode2=0;
                        iBackMode2=0;
                    }
                }
                break;
            }
            else if(FRCarryKit.HasIC() && OutSHT1InRT() && bCheckShuttle1Flag==false)
            {
                Task=50;
                break;
            }
//            else if(WhichAutoNeedTray()!=0)    //JerryYang 20170731 (wei) Out arm要讓位才不會hang up  //JerryYang 20170828 Mark掉,只有Shuttle1需要讓位
//            {
//                MoveOutArmXY_ToFix_Tray_Full();
//            }
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

            if(DoPickFromShuttle2_9045S_1x4_4(true))
            {
                PreSetOutAdditionalFlag();                                                                                                                      //Steven 20220614 : Fixed for out arm additional function
                if(BRCarryKit.HasIC())
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
//                    iOutArmPickPlaceCnt[OutOfsOutSh2]++;                      //JerryYang 20230523 : 沒在用, Mark掉  //JerryYang 20180921 Setup Teach功能
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

            if(DoPickFromShuttle_9045_2x6_8(1))
            {
                PreSetOutAdditionalFlag();                                                                                                                      //Steven 20220614 : Fixed for out arm additional function
                if(BRCarryKit.RightSideNoIC(3)==false)                                                                                                          //ChungHung 20130507 add HT9045 updata for 12site 517
                {
                    InitPickFromShuttle2Task();                                                                                                                 //Sam 20230208 : 修正 Fix AOI 未拍照問題
                    return;
                }

                if(OutArmSuck.HasIC()==false)
                {
                    Task=50;
                }
                else
                {
                    iOutRotateFinish=0;                                                                                                                         //kevin 20130524 目前放到 auto tray
//                    iOutArmPickPlaceCnt[OutOfsOutSh2]++;                      //JerryYang 20230523 : 沒在用, Mark掉  //JerryYang 20180921 Setup Teach功能
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
                MOT[MInShuttle1].fCanMoveR=true;
                MOT[MInShuttle2].fCanMoveR=true;
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

            ret=VerifyFixTrayLink();
            if(ret==4000 || ret==10000)
            {
                Task=ret;
                break;
            }
            else if(ret==1)
            {
                return;
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
                   iWhichAuto>=iMagMin)
                {
                }
                else
                {
                    SetFixTrayFullIC();
                    if(MOT[iMMAuto[iWhichAuto]].fHasTray==false ||
                       MOT[iMMAuto[iWhichAuto]].FullIC())
                    {
                        if(iHWFix_BinBox==1 && iWhichAuto==iBinBoxPos)
                            DoFixTrayFullAlarm();                                                                                                               //kevin 20201207 check Error bin box Fix 1 put tray
                        break;
                    }
                }
            }

            if(iCatchTrayControlManual>=2 || WhichAutoNeedTray()!=0)                                                                                            //Sam 20180822 : OutArm 是否在安全位置，是否讓位，不然會與 TrayArm 補 Tray 動作互卡 Hange up。
            {
                MoveOutArmXY_ToFix_Tray_Full();
                break;
            }
            Task=3100;
            break;
        case 3020:                                                                                                                                              //jou 2010-01-25 start : one cycle後，可以將tray盤拉出來
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
                Task=VerifyTrayStatus();
                if(Task!=3300)
                    break;
            }
            Task=3300;                                                                                                                                          //kevin 20151203 沒有fixtray放ic
        case 3300:
            if(AUTO3_IS_MAGAZINE==1             &&                                                                                                              //JerryYang 20221215 : Magazine把fix區當buffer區功能
               TestIF_File.iMagFixTrayType==1   &&
               iWhichAuto>=iMagMin              &&
               iWhichBuff>=iFixMin              &&
               iWhichBuff<=iFixMax)
            {
                ct=iAutoIndex[iWhichBuff];
                if(iWhichBuff>=iFixMin &&
                   Sen[SnFixedTrayDetect[ct]].IsOff() &&
                   LastSet.iRealDummy!=DUMMY)                                                                                                                   //wei 20151217 add iWhichAuto>=3避免沒要放FIX盤也偵測有沒有FIX盤
                {
                    Task=4000;
                    break;
                }
            }
            else
            {
                ct=iAutoIndex[iWhichAuto];
                if(iWhichAuto>=iAutoCnt &&
                   Sen[SnFixedTrayDetect[ct]].IsOff() &&
                   LastSet.iRealDummy!=DUMMY)                                                                                                                   //wei 20151217 add iWhichAuto>=3避免沒要放FIX盤也偵測有沒有FIX盤
                {
                    Task=4000;
                    break;
                }
            }
            Task=3301;
            if(Task!=3301)                                                                                                                                      //Steven 20180813 : add Out arm speed
                break;
        case 3301:
            if(DoOutArmPlaceToAuto_9045())                                                                                                                      // 吸嘴可能還在下方
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
            {
                Task=ret;
            }
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
