// =============================================================================
//  aoutarm9045_2x1_2.cpp  --  Out-Arm per-site VARIANT (2x1_2, two-row 1-col / 2-site)
//
//  Faithful translation of golden aoutarm9045_2x1_2.cpp (1295 lines, BCB6, Big5).
//  Translator: AI(W6.2c-2x1_2) 20260627
//  Translation wave: W6.2c-OUT (out-arm per-site-config variant: 2x1_2).  This is
//  the LIVE callee for iInArmType==e9045_2x1_2_13: the engine dispatch ladders in
//  aoutarm9045.cpp route that enum to DoOutArm_9045_2x1_2() /
//  DoPickFromShuttle_9045_2x1_2(int) (this file's two engine-dispatched callees --
//  the file's layout suffix == the callee suffix).
//
//  ROLE: the per-site out-arm pick-from-shuttle / place-to-auto state machines for
//  the 2x1_2 layout (two shuttle rows A/B, single picker column, 2-site pick).
//  This file owns: GetNowShuttleMode_2x1_2, OutArmZNeedDown_2x1_2,
//  CheckOutArmXYPitch_2x1_2, MoveOutArmToShuttleIncludeZ_9045_2x1_2,
//  DoPickFromShuttle_9045_2x1_2, DoOutArm_9045_2x1_2.
//
//  FAITHFUL: every fn/var/cursor value, switch(Task) structure, formula, the
//  case-1->case-5->case-10->case-50 fall-throughs in DoOutArm, the
//  case-10->case-200, case-3100->case-3300, case-3301->case-3500 fall-throughs,
//  the case-1140 / case-1100 / case-2040 / case-2100 inner fall-throughs, the
//  integer-vs-double pitch math (double(iMovePitchX)/3.0 etc.), the commented-out
//  USE_PICKER_COUNT==ep16Picker block in CheckOutArmXYPitch_2x1_2 and the
//  commented #ifdef Carry4 / iOutArmPickPlaceCnt blocks are transcribed VERBATIM
//  from the golden CP950.  The SM case bodies are ACTIVE -- they touch
//  MOT[]/Suck/Sensor/predicates/cross-module callees which resolve over the Sim
//  HAL + aHotPlateSubstrate + the aoutarm9045 engine + shims (substrate symbols
//  not yet present are forward-declared behind #ifndef guards and REPORTED).
//
//  VCL/Borland conversions: AnsiString (1-based) via vclcompat (kept verbatim);
//  no __fastcall/__property/__published/PACKAGE in this file; __FUNC__ via the
//  aArmHeader shim; TQPF_Timer via myTimer.h (pulled by aHotPlateSubstrate.h).
//  SOFT_SIMULTE is NOT defined -> the single #ifdef SOFT_SIMULTE block in
//  DoPickFromShuttle_9045_2x1_2 (fMain->chkInPickLoadError simulate-IC-lost) is
//  kept VERBATIM but inert.  Numeric semantics (int vs double, integer division/
//  truncation) kept EXACT.
//
//  GATED (// TODO(W7)): the fFixAICCD->OutArmCycleCounterUpdate() Fix-AI-CCD UI
//  call (golden FixAICCD.h TfFixAICCD form; no FormsFacade stand-in) is gated
//  behind `#if 0 // TODO(W7)` -- the surrounding
//  `if(USE_Fix_AI_CCD && TestIF_File.bEnableFix2BGAAICCD)` guard + control flow
//  stay live; only the form method call is gated (no-op offline).
//
//  Big5: every Chinese comment decoded via cp950 and preserved as UTF-8.
//  Final gate: ZERO U+FFFD.
// =============================================================================
// BCB6 ORIGINAL include block (mirrored as a comment for provenance):
//   #include "MachineDefine.h" ; #pragma hdrstop
//   #include "aoutarm9045_2x1_2.h"
//   #include "aArmHeader.h" / "MyKitSuck.h" / "cprod.h" / "mymotor.h" / "mycylin.h"
//   #include "cSortCT.h" / "mysensor.h" / "main.h" / "note.h" / "csystem.h"
//   #include "acatchtray.h" / "cinitial.h" / "atester.h" / "aRotateKIT.h"
//   #include "fRotate.h" / "cOffSet.h" / "fAOI.h" / "cmydef.h" / "ProductionInfo.h"
//   #include "FixAICCD.h" / "Magazine.h" ; #pragma package(smart_init)
// =============================================================================
#include "MachineDefine.h"

#include "aoutarm9045_2x1_2.h"
#include "aArmHeader.h"             // __FUNC__ shim, RecordProcess, ShowErrorMessage, etc.
#include "aoutarm9045.h"            // engine: GetOutArmPitchX/Y_9045 / GetOutShuttleStatus_9045 / IsCheckOutArmDestroyActiveFinish / DoOutArmSuckPreOn / InitialOutArmNeedSuck / OutArmNeedCheckOffset / iOUTARM_SUCK / SearchTrayToPlace_9045 / VerifyFixTrayLink / DoOutArmPlaceToAuto_9045 / DoOutArmAfterPlaceToAuto / DoOutArmAdditionalFunction / CheekNeedToDoOutArmAdditionalFunction / PreSetOutAdditionalFlag / MoveOutArmXY_To_ESDSafePos
#include "ainarm9045.h"            // bCheckYPitchHome / bCheckYPitchRunHomeSen (Ypitch home sensors, shared)
#include "cprod.h"                 // Prod / TestIF / TestIF_File / ArmSpeed / LastSet
#include "Motor/mymotor.h"         // MOT[], OutArmContinuousMove_9045, OutArm enum, MInShuttle1/2, MTrayX, MOutArmPitchY, iMMAuto
#include "cmydef.h"                // global scalar universe + IC consts + iInArmType + enums + AseIcRecord / bPickSH*Flag / bMagCatchTrayfalg / iWhich* / iAutoIndex / SnFixedTrayDetect / iOutArmYBase / iOutArmXBase / iOutArmShtXCenterPos / iOutArmShtYCenterPos / iXpitchMaxX3 / iXpitchMaxX7 / X_PITCH_COUNT / ...
#include "cpublic.h"               // CosFunction
#include "common.h"
#include "mycylin.h"
#include "mysensor.h"              // Sen[] (SnFixedTrayDetect)
#include "csystem.h"               // OutSHT1InRT / OutSHT2InRT / CheckOutArmSuckICFallDown / IsOutArmCleanOutFinish / AutoTrayReCheck / HasICUnderMachine / HasICUnderHotPlate
#include "aHotPlateSubstrate.h"    // OutArmSuck / FRCarryKit / BRCarryKit (TMyKitSuck) + TQPF_Timer + cursors
#include "FormsFacade.h"           // fMain (SOFT_SIMULTE chkInPickLoadError) / TrayForm offline stand-ins
#include "acatchtray_shims.h"      // InitProcessSingleMotorTask
#include "acarry_shims.h"          // MoveOutArmToAutoSafe / MoveOutArmXY_ToShuttleAlarmArea / NULL_IC
#include "ainarm9045_2x4_16_shims.h"// AutoTeachLoadTrayZ / SetMotorSpeed
#include "canary_support.h"        // ShowErrorMessage / ShowMyMessage / RecordProcess / LastSet / WhichAutoNeedTray / K_RETRY / iOutArmZTeachTask

// -----------------------------------------------------------------------------
//  SUBSTRATE EXTERNS not yet provided by the current cpp tree (REPORTED for the
//  serial Integrate phase).  Forward-declared here behind #ifndef guards so this
//  TU compiles standalone and CANNOT ODR-collide once the owning module lands.
//  Signatures are VERBATIM from golden aoutarm.h / aoutarm9045.h / MyKitSuck.h /
//  ainarm2.h.
//
//   golden home -> aoutarm.h (legacy out-arm engine, not yet translated):
//     ptrOutSHT, PickFromShuttle, bOutShtwaitPick, ShowOutputShuttleDataMiss,
//     ErrorBinBoxDetect, OutArmPickShuttleAlarm, PorcessJAM0201OutArmPickUpErrorSkip,
//     SetOutArmHome, bOutArmManualStepPress, IsCatchTrayReadySupplyNewTray,
//     CheckShuttleICPos, CheckTesterZ, CheckOutArmInitState, TransferOutShuttleRatio,
//     InitPickFromShuttle1Task, InitPickFromShuttle2Task, InitPlaceToAutoTask,
//     SetFixTrayFullIC, OutArmSubSpeed, OutArmAddSpeed, OutArmSubSpeedDisplay,
//     OutArmAddSpeedDisplay, OutArmTask, CheckOutArmCleanOut, SetOutArmNeedDestory,
//     bCarryControlOutarm1, bCarryControlOutarm2, bEnterOffset, CheckRotateOutNotFinish,
//     MoveOutArmXY_ToFix_Tray_Full, iWhichAuto.
//   golden home -> aoutarm9045.h, but OMITTED from this wave's translated subset:
//     GetOutArmToShuttleOffset_9045, GetOutArmYToShuttleOffset_9045,
//     GetOutArmXToShuttleOffset_9045, GetVariableYOutShuttleData,
//     SwapShuttleDataToOutArm, bOutSuckShtDupErr, iOutShtRetryCount,
//     iPickFromShuttle1Task, iPickFromShuttle2Task, CheckOutArmToTask50,
//     DoFixTrayFullAlarm, VerifyTrayStatus, SearchTrayToPlace_Magazine.
//   golden home -> MyKitSuck.h: ptrOutSHT (TMyKitSuck* cursor).
//   golden home -> ainarm2.h: hOutArmYpitchHomeTimer (HTimer; offline TQPF_Timer
//     per the hInArmYpitchHomeTimer precedent in aHotPlateSubstrate.h), bYpitchNeddHome.
//   golden home -> Magazine.h: CheckPlaceToMagazineTray, DoPickFromMagazineBuffer.
//   Offline-safe behavior for all: declared extern (NOT defined) -> resolves at
//   link once the owning module lands; the SM control flow over the Sim HAL is
//   unaffected for this variant.
// -----------------------------------------------------------------------------
#ifndef AOUT_PTROUTSHT_FWD
#define AOUT_PTROUTSHT_FWD
extern TMyKitSuck *ptrOutSHT;
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
void ShowOutputShuttleDataMiss();                                              //Steven 20110527
#endif
#ifndef AOUT_ERRORBINBOXDETECT_FWD
#define AOUT_ERRORBINBOXDETECT_FWD
bool ErrorBinBoxDetect(int iError);                                            //kevin 20160906 error bin box
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
void SetOutArmHome();                                                          //jou 981203 alarm時，out arm z 軸要home
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
void OutArmSubSpeedDisplay();                                                  //KaiChen 20171225 (Steven)：Add Speed Display
#endif
#ifndef AOUT_OUTARMADDSPEEDDISPLAY_FWD
#define AOUT_OUTARMADDSPEEDDISPLAY_FWD
void OutArmAddSpeedDisplay();                                                  //KaiChen 20171225 (Steven)：Add Speed Display
#endif
#ifndef AOUT_OUTARMTASK_FWD
#define AOUT_OUTARMTASK_FWD
extern int OutArmTask;
#endif
#ifndef AOUT_CHECKOUTARMCLEANOUT_FWD
#define AOUT_CHECKOUTARMCLEANOUT_FWD
int CheckOutArmCleanOut(int Task=50);
#endif
#ifndef AOUT_GETOUTARMTOSHUTTLEOFFSET_FWD
#define AOUT_GETOUTARMTOSHUTTLEOFFSET_FWD
int  GetOutArmToShuttleOffset_9045(int iSht, int iModeRow, int iModeCol, bool bPitch=true);
#endif
#ifndef AOUT_GETOUTARMYTOSHUTTLEOFFSET_FWD
#define AOUT_GETOUTARMYTOSHUTTLEOFFSET_FWD
int GetOutArmYToShuttleOffset_9045(int iSht, int iOffsetPos);
#endif
#ifndef AOUT_GETOUTARMXTOSHUTTLEOFFSET_FWD
#define AOUT_GETOUTARMXTOSHUTTLEOFFSET_FWD
int GetOutArmXToShuttleOffset_9045(int iSht, int iOffsetPos);                  //Steven 20240309 : fixed for out arm to sht offset
#endif
#ifndef AOUT_GETVARIABLEYOUTSHUTTLEDATA_FWD
#define AOUT_GETVARIABLEYOUTSHUTTLEDATA_FWD
int GetVariableYOutShuttleData();                                              //ChungHung 20131231 alter AutoYPitch
#endif
#ifndef AOUT_SETOUTARMNEEDDESTORY_FWD
#define AOUT_SETOUTARMNEEDDESTORY_FWD
void SetOutArmNeedDestory(int iRow, int iCol, int iShtRow, int iShtCol, bool bPlace=false);  //RogerYang 20250516 Add for 9046AU
#endif
#ifndef AOUT_SWAPSHUTTLEDATATOOUTARM_FWD
#define AOUT_SWAPSHUTTLEDATATOOUTARM_FWD
extern bool SwapShuttleDataToOutArm(int iSht, int iShtRow, int iShtCol, int iSuckRow, int iSuckCol, int *iWitchErrBin);  //Steven 20170428
#endif
#ifndef AOUT_BOUTSUCKSHTDUPERR_FWD
#define AOUT_BOUTSUCKSHTDUPERR_FWD
extern bool bOutSuckShtDupErr[MAX_ARM_Row][MAX_ARM_Col];
#endif
#ifndef AOUT_IOUTSHTRETRYCOUNT_FWD
#define AOUT_IOUTSHTRETRYCOUNT_FWD
extern int  iOutShtRetryCount;
#endif
#ifndef AOUT_IPICKFROMSHUTTLE1TASK_FWD
#define AOUT_IPICKFROMSHUTTLE1TASK_FWD
extern int iPickFromShuttle1Task;
#endif
#ifndef AOUT_IPICKFROMSHUTTLE2TASK_FWD
#define AOUT_IPICKFROMSHUTTLE2TASK_FWD
extern int iPickFromShuttle2Task;
#endif
#ifndef AOUT_CHECKOUTARMTOTASK50_FWD
#define AOUT_CHECKOUTARMTOTASK50_FWD
bool CheckOutArmToTask50(int iSht);                                            //Steven
#endif
#ifndef AOUT_DOFIXTRAYFULLALARM_FWD
#define AOUT_DOFIXTRAYFULLALARM_FWD
bool DoFixTrayFullAlarm();
#endif
#ifndef AOUT_VERIFYTRAYSTATUS_FWD
#define AOUT_VERIFYTRAYSTATUS_FWD
int  VerifyTrayStatus();
#endif
#ifndef AOUT_SEARCHTRAYTOPLACE_MAGAZINE_FWD
#define AOUT_SEARCHTRAYTOPLACE_MAGAZINE_FWD
int  SearchTrayToPlace_Magazine();                                             //JerryYang
#endif
#ifndef AOUT_HOUTARMYPITCHHOMETIMER_FWD
#define AOUT_HOUTARMYPITCHHOMETIMER_FWD
extern TQPF_Timer hOutArmYpitchHomeTimer;                                      //golden ainarm2.h HTimer; offline TQPF_Timer (same Off()/SetSecAndOn() surface)
#endif
#ifndef AOUT_BYPITCHNEDDHOME_FWD
#define AOUT_BYPITCHNEDDHOME_FWD
extern bool bYpitchNeddHome;                                                   //kevin 20180823
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
extern bool bCarryControlOutarm1;                                              //golden aoutarm.h
#endif
#ifndef AOUT_BCARRYCONTROLOUTARM2_FWD
#define AOUT_BCARRYCONTROLOUTARM2_FWD
extern bool bCarryControlOutarm2;                                              //golden aoutarm.h
#endif
#ifndef AOUT_BENTEROFFSET_FWD
#define AOUT_BENTEROFFSET_FWD
extern bool bEnterOffset;                                                      //golden aoutarm.h
#endif
#ifndef AOUT_CHECKROTATEOUTNOTFINISH_FWD
#define AOUT_CHECKROTATEOUTNOTFINISH_FWD
bool CheckRotateOutNotFinish();                                                //golden RotateKit/aRotateKIT_Out.h
#endif
#ifndef AOUT_MOVEOUTARMXYTOFIXTRAYFULL_FWD
#define AOUT_MOVEOUTARMXYTOFIXTRAYFULL_FWD
bool MoveOutArmXY_ToFix_Tray_Full(bool bMoveY=false);                          //golden aoutarm.h
#endif
#ifndef AOUT_IOUTARMZTEACHTASK_FWD
#define AOUT_IOUTARMZTEACHTASK_FWD
extern int iOutArmZTeachTask;                                                  //golden AutoTeach/AutoTeach.h
#endif
#ifndef AOUT_IOUTROTATEFINISH_FWD
#define AOUT_IOUTROTATEFINISH_FWD
extern int iOutRotateFinish;                                                   //kevin 20130524 golden RotateKit/aRotateKIT.h
#endif
#ifndef AOUT_IWHICHAUTO_FWD
#define AOUT_IWHICHAUTO_FWD
extern int iWhichAuto;                                                         //golden aoutarm.h
#endif
//==============================================================================
// provenance: golden aoutarm9045_2x1_2.cpp:30 (GetNowShuttleMode_2x1_2)
int GetNowShuttleMode_2x1_2(int iSht)
{
    ptrOutSHT=(iSht==0)?&FRCarryKit:&BRCarryKit;

    if(ptrOutSHT->UseSiteNoIC())
    {
        return 0;
    }
    else
    {
        if(ptrOutSHT->Item[0][0]>=HAS_IC)
            return 0;                                                           //上
        else                                                                    //if(ptrOutSHT->Item[1][0]>=HAS_IC)
            return 1;                                                           //下
    }
}
//==============================================================================
// provenance: golden aoutarm9045_2x1_2.cpp:47 (OutArmZNeedDown_2x1_2)
bool OutArmZNeedDown_2x1_2(int iSht)
{
    if(InitialOutArmNeedSuck(iSht)==false)
        return false;

    ptrOutSHT=(iSht==0)?&FRCarryKit:&BRCarryKit;
    int iMode=GetNowShuttleMode_2x1_2(iSht);
    int iModeRow=iMode%100;
//    int iModeCol=iMode/100;

    if(iModeRow==0)
    {
        SetOutArmNeedDestory(0, 0, 0, 0);
    }
    else
    {
        SetOutArmNeedDestory(0, 1, 1, 0);
    }
    return true;
}
//------------------------------------------------------------------------------
// provenance: golden aoutarm9045_2x1_2.cpp:68 (CheckOutArmXYPitch_2x1_2)
bool CheckOutArmXYPitch_2x1_2(int *iX, int *iY, int iSht, int iMovePitchX, int iMovePitchY)
{
    int iMode=GetNowShuttleMode_2x1_2(iSht);
    int iModeRow=iMode%100;
    int iModeCol=iMode/100;
    int iOffsetPos=GetOutArmToShuttleOffset_9045(iSht, iModeRow, iModeCol, false);
    double dMovePitchX;

    if(iSht==0)
    {
        *iX+=Prod.XOutArm_Shuttle1_Pick[iOutArmYBase][iOutArmXBase];
        *iY+=Prod.YOutArm_Shuttle1_Pick[iOutArmYBase][iOutArmXBase];
    }
    else
    {
        *iX+=Prod.XOutArm_Shuttle2_Pick[iOutArmYBase][iOutArmXBase];
        *iY+=Prod.YOutArm_Shuttle2_Pick[iOutArmYBase][iOutArmXBase];
    }

    if(iModeRow==0)                                                             //Row A
    {
        if(USE_OUT_Y_IS_AUTO_PITCH==true)                                       //JerryYang 20251218 : IN/OUT ARM支援不同模組
            *iY=*iY-iMovePitchY+TestIF.dSiteYPitch/2;
        else
            *iY=*iY+TestIF.dSiteYPitch/2;
    }
    else if(iModeRow==1)                                                        //Row B
    {
        if(USE_OUT_Y_IS_AUTO_PITCH==true)                                       //JerryYang 20251218 : IN/OUT ARM支援不同模組
            *iY=*iY-iMovePitchY-TestIF.dSiteYPitch/2;
        else
            *iY=*iY+iMovePitchY-TestIF.dSiteYPitch/2;
    }
    else                                                                        //Both    //目前不會進來
    {
        if(USE_OUT_Y_IS_AUTO_PITCH==true)                                       //JerryYang 20251218 : IN/OUT ARM支援不同模組
            *iY=*iY-TestIF.dSiteYPitch/2;
        else
            *iY=*iY+TestIF.dSiteYPitch/2;
    }

    /*if(USE_IN_OUT_ARM_Y_PITCH==iXYPitch16Bd_Be)                               //Ztex 2023.12.06 Add HT-1032
    {
    }
    else if(USE_PICKER_COUNT==ep16Picker)                                       //基準為第四隻吸嘴
    {
        dMovePitchX=double(iMovePitchX)/7.0;
        if(iModeRow==0)                                                         //Aa --> Aa
        {
            *iX=*iX+(dMovePitchX*3.0-TestIF.dSiteXPitch*0.5);
        }
        else                                                                    //if(iModeRow==1)                                                  //Ac --> Ba
        {
            *iX=*iX+(dMovePitchX*3.0-TestIF.dSiteXPitch*0.5);
        }
    }
    else */if(USE_OUT_ARM_Y_PITCH==iXYPitchVariable)                            //基準為第二隻吸嘴  //JerryYang 20251218 : IN/OUT ARM支援不同模組
    {
        dMovePitchX=double(iMovePitchX)/3.0;
        if(iModeRow==0)
        {
            *iX=*iX+(dMovePitchX*1.0-TestIF.dSiteXPitch*0.0);
        }
        else                                                                    //if(iModeRow==1)
        {
            *iX=*iX+(-dMovePitchX*1.0-TestIF.dSiteXPitch*0.0);
        }
    }
    else                                                                        //基準為第三隻吸嘴
    {
        dMovePitchX=double(iMovePitchX)/3.0;
        if(iModeRow==0)
        {
            *iX=*iX+(dMovePitchX*2.0-TestIF.dSiteXPitch*0.0);
        }
        else if(iModeRow==1)
        {
            *iX=*iX+(dMovePitchX*0.0-TestIF.dSiteXPitch*0.0);
        }
    }

    *iY=*iY+GetOutArmYToShuttleOffset_9045(iSht, iOffsetPos);
    *iX=*iX+GetOutArmXToShuttleOffset_9045(iSht, iOffsetPos);
    TransferOutShuttleRatio(iSht, iX, iY, iOutArmYBase, iOutArmXBase);
    if(OutArmZNeedDown_2x1_2(iSht)==false)
        return false;
    return true;
}
//==============================================================================
// provenance: golden aoutarm9045_2x1_2.cpp:157 (MoveOutArmToShuttleIncludeZ_9045_2x1_2)
bool MoveOutArmToShuttleIncludeZ_9045_2x1_2(int iSht, bool bZDown)
{
    bool bZFlag[MAX_ARM_Row][MAX_ARM_Col];
    int iZPos[MAX_ARM_Row][MAX_ARM_Col];
    int iXVariable[X_PITCH_COUNT];
    int iXPos               =iOutArmShtXCenterPos;                              //Steven 20141029 : XY-Pitch for Shuttle Center position for base Suck
    int iYPos               =iOutArmShtYCenterPos;
    int iMode               =GetNowShuttleMode_2x1_2(iSht);
    int iModeRow            =iMode%100;
    int iModeCol            =iMode/100;
    int iMovePitchX         =iXpitchMaxX3;
    int iMovePitchY         =GetVariableYOutShuttleData();
    int iOffsetPos          =GetOutArmToShuttleOffset_9045(iSht, iModeRow, iModeCol, true);
    int iYVariable          =GetOutArmPitchY_9045(iMovePitchY, iOffsetPos);
    ZeroMemory(iXVariable,  sizeof(iXVariable));
    ZeroMemory(bZFlag,      sizeof(bZFlag));
    ZeroMemory(iZPos,       sizeof(iZPos));

    if(USE_16PICKER_TYPE==1)
    {
        iMovePitchX=iXpitchMaxX7;
    }
    else
    {
        iMovePitchX=iXpitchMaxX3;
    }

    for(int i=0; i<X_PITCH_COUNT; i++)
        iXVariable[i]=GetOutArmPitchX_9045(iMovePitchX, i, iOffsetPos);

    if(TestIF.bNS7000kit)                                                       //jou 981208 start : NS7000 bias kit
    {
        if(iSht==0)
            iYPos+=iMovePitchY/2;
        else
            iYPos-=iMovePitchY/2;
    }

    if(CheckOutArmXYPitch_2x1_2(&iXPos, &iYPos, iSht, iMovePitchX, iMovePitchY)==false && bZDown==true)
        return false;

    GetOutShuttleStatus_9045(iSht, iZPos, bZDown, bZFlag);
    if(IsCheckOutArmDestroyActiveFinish(iXPos, iYPos)==false)                   //2013-08-02    Dell     add Pick 前先確認CheckInArmDestroyActive 已完成
        return false;

    DoOutArmSuckPreOn(iSht, 0, iXPos, iYPos, bZDown);
    if(OutArmContinuousMove_9045(iXPos, iYPos, iXVariable, iYVariable, bZFlag, iZPos, bZDown))
        return true;

    return false;
}
//==============================================================================
// 輸出臂從 Shuttle 1 吸取 IC 流程動作
//  Ver : 2003_07
//==============================================================================
// provenance: golden aoutarm9045_2x1_2.cpp:212 (DoPickFromShuttle_9045_2x1_2)
bool DoPickFromShuttle_9045_2x1_2(int iSht)
{
    int ret;
    int &Task=(iSht==0)?iPickFromShuttle1Task:iPickFromShuttle2Task;
    static int iWitchErrBin=0;
    AnsiString ErrPart="";                                                      //Steven 20110216 : 合併Alarm
    bool bHasDuplicateErr=false;                                                //Steven 20110216 : 合併Alarm
    bool bHasErr=false;                                                         //Steven 20110216 : 合併Alarm
    bool flag=false;
    int iSuckRow, iSuckCol, iShtRow, iShtCol;
    ptrOutSHT=(iSht==0)?&FRCarryKit:&BRCarryKit;

    if(CheckTesterZ(iSht))
    {
        //MoveOutArmToAutoSafe();                                               //Steven 20150408 : Mark,避免吸嘴上上下下
        return false;
    }

    switch(Task)
    {
        case 1:
            if(ArmSpeed[OutArm].dWaitOnSH!=0)                                   //kevin 20210525 out arm 起動在shuttle 上等待 時間
            {
                if(OutArmSuck.NoIC())                                           //kevin 20210615 第一次 吸取IC 需等待
                {
                    if(MoveOutArmToShuttleIncludeZ_9045_2x1_2(iSht))            //移到 Shuttle 上等待
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
            iWitchErrBin=0;
            Task=200;
        case 200:
            if(MoveOutArmToShuttleIncludeZ_9045_2x1_2(iSht, true))              //kevin 20210717 change function
            {
                if(OutArmNeedCheckOffset(false, iSht))                          //Steven 20230531 : 簡化判斷式
                {
                    Task=500;
                    break;
                }
                OutArmSuck.ResetAll();                                          //Steven 20160323 : 避免未開啟真空
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
            bHasErr=false;

            if(ptrOutSHT->Item[0][0] &&
              OutArmSuck.Suck[0][0].GetNeedSuckStatus())
            {
                iSuckRow  =0;
                iSuckCol  =0;
                iShtRow   =0;
                iShtCol   =0;
                if(OutArmSuck.Suck[iSuckRow][iSuckCol].Suck())
                {
                    //模擬ic不見了
                    #ifdef SOFT_SIMULTE
                    if(fMain->chkInPickLoadError->Checked==true)
                    {
                        OutArmSuck.Suck[0][0].Error=true;
                    }
                    #endif
                    //模擬ic不見了 end
                    bHasErr=SwapShuttleDataToOutArm(iSht, iShtRow, iShtCol, iSuckRow, iSuckCol, &iWitchErrBin);         //Steven 20170428 : 統一Out Arm從Shuttle吸料後的資料交換
                    bOutSuckShtDupErr[iSuckRow][iSuckCol]=false;
                }
                else if(OutArmSuck.Suck[iSuckRow][iSuckCol].Error==false)
                {
                    flag=false;
                }
            }
            else if(ptrOutSHT->Item[1][0] && OutArmSuck.Suck[0][1].GetNeedSuckStatus())
            {
                iSuckRow  =0;
                iSuckCol  =1;
                iShtRow   =1;
                iShtCol   =0;
                if(OutArmSuck.Suck[iSuckRow][iSuckCol].Suck())
                {
                    bHasErr=SwapShuttleDataToOutArm(iSht, iShtRow, iShtCol, iSuckRow, iSuckCol, &iWitchErrBin);         //Steven 20170428 : 統一Out Arm從Shuttle吸料後的資料交換
                    bOutSuckShtDupErr[iSuckRow][iSuckCol]=false;
                }
                else if(OutArmSuck.Suck[iSuckRow][iSuckCol].Error==false)
                {
                    flag=false;
                }
            }

            if(bHasErr)
            {
                ShowOutputShuttleDataMiss();
            }

            if(flag==false)                                                                                             //KEVIN 20130421 按RETRY資料尚未交換會將其他IC丟掉
                break;

            for(int i=0; i<OutArmSuck.iPickRow; i++)
            {
                for(int j=0; j<OutArmSuck.iPickCol; j++)
                {
                    if(OutArmSuck.Suck[i][j].Error)
                    {
                        iOutShtRetryCount++;
                        Task=2000;
                        return false;
                    }
                }
            }

            for(int i=0; i<OutArmSuck.iPickRow; i++)
            {
                for(int j=0; j<OutArmSuck.iPickCol; j++)
                    if(OutArmSuck.Item[i][j]==NULL_IC &&
                       OutArmSuck.Suck[i][j].GetNeedSuckStatus())
                        return false;
            }

            if(iHWFix_BinBox==1 && ErrorBinBoxDetect(iWitchErrBin)==false)
                return false;                                                                                           //kevin 20160906 add error bin box

            if(USE_Fix_AI_CCD==true && TestIF_File.bEnableFix2BGAAICCD==true)                                           //KaiChen 20190124 ：矽格湖口 AI CCD Function
            {
                #if 0 // TODO(W7): fFixAICCD form (golden FixAICCD.h TfFixAICCD) -- no FormsFacade stand-in; guard + flow kept live, form call gated (no-op offline)
                fFixAICCD->OutArmCycleCounterUpdate();
                #endif
            }

            iOutShtRetryCount=0;
            Task=1;
            return true;
        case 2000:
            if(MoveOutArmToAutoSafe())
            {
                bHasErr=false;
                ErrPart=" ";
                bHasDuplicateErr=false;                                                                                 //Steven 20110216 Start : 整合Alarm
                for(int i=0; i<OutArmSuck.iPickRow; i++)
                {
                    for(int j=0; j<OutArmSuck.iPickCol; j++)
                    {
                        if(bOutSuckShtDupErr[i][j])
                            bHasDuplicateErr=true;
                        if(OutArmSuck.Suck[i][j].Error)
                        {
                            ErrPart+=OutArmSuck.Suck[i][j].sName;
                            bHasErr=true;
                        }
                        else
                        {
                            bOutSuckShtDupErr[i][j]=false;
                        }
                    }
                }

                if(bHasErr)
                {
                    if(iOutShtRetryCount>ArmSpeed[OutArm].iRetryCT)
                    {
                        if(!MoveOutArmXY_ToShuttleAlarmArea())
                            return false;

                        ret=OutArmPickShuttleAlarm(iSht, bHasDuplicateErr, ErrPart);

                        for(int i=0; i<OutArmSuck.iPickRow; i++)
                        {
                            for(int j=0; j<OutArmSuck.iPickCol; j++)
                            {
                                if(OutArmSuck.Suck[i][j].Error)
                                {
                                    bOutSuckShtDupErr[i][j]=true;
                                }
                                else
                                {
                                    bOutSuckShtDupErr[i][j]=false;
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
                        if(OutArmSuck.Suck[0][0].Error)
                        {
                            PorcessJAM0201OutArmPickUpErrorSkip(iSht, 0, 0, 0, 0);                                      //Steven 20161214 : 加上OutArm吸取異常Skip的ErrorLog
                        }
                        else if(OutArmSuck.Suck[0][1].Error)
                        {
                            PorcessJAM0201OutArmPickUpErrorSkip(iSht, 1, 0, 0, 1);                                      //Steven 20161214 : 加上OutArm吸取異常Skip的ErrorLog
                        }
                        iOutShtRetryCount=0;
                        ZeroMemory(bOutSuckShtDupErr, sizeof(bOutSuckShtDupErr));
                        Task=1;
                        return true;
                    }
                    else                                                                                                //Steven 20190313 : Fixed for out arm pick error need home
                    {
                        if(iOutShtRetryCount>ArmSpeed[OutArm].iRetryCT)
                        {
                            iOutShtRetryCount=0;
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
                    ZeroMemory(bOutSuckShtDupErr, sizeof(bOutSuckShtDupErr));
                }
                Task=1;
            }
            break;
        case 2200:
            SetOutArmHome();
            iOutShtRetryCount=0;
            Task=1;
            break;
    }
    return false;
}
//==============================================================================
// provenance: golden aoutarm9045_2x1_2.cpp:479 (DoOutArm_9045_2x1_2)
void DoOutArm_9045_2x1_2()
{
    int ret, ct, iMode1=0, iMode2=0;
    static int OldPos=1;
    static bool FirstEnter=true, bCheckSpeed=false;
    int &Task=OutArmTask;
    static int iWhichShuttle=0, iBackMode1=0, iBackMode2=0;
    bool flag3;
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
        case 5:
            if(MoveOutArmToAutoSafe())
                Task=10;

            if(Task!=10)                                                        //Steven 20180813 : add Out arm speed
                break;
        case 10:
            if(CheckOutArmInitState())
                Task=50;

            if(Task!=50)                                                        //Steven 20180813 : add Out arm speed
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

            if(Task!=100)                                                       //Steven 20180813 : add Out arm speed
                break;
        case 100:
            IsCatchTrayReadySupplyNewTray();
            if(FirstEnter)
            {
                if((FRCarryKit.UseSiteHasIC()) && OutSHT1InRT())
                {
                    OldPos=1;
                    FirstEnter=false;
                }
                else if((BRCarryKit.UseSiteHasIC()) && OutSHT2InRT())
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

            if(FRCarryKit.UseSiteHasIC() && OutSHT1InRT())
            {
                if(OldPos==0 && BRCarryKit.UseSiteNoIC() && OutSHT2InRT()==false)
                    OldPos=1;
            }
            else if(BRCarryKit.UseSiteHasIC() && OutSHT2InRT())
            {
                if(OldPos==1 && FRCarryKit.UseSiteNoIC() && OutSHT1InRT()==false)
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
                break;
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

            if(Task==5000)                                                      //Sam 20250204 : 修正 P27 整盤功能 Hangeup
            {
                DoOutArmAfterPlaceToAuto(bCheckSpeed, true);
                Task=3500;
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
            if(USE_OUT_Y_IS_AUTO_PITCH==true && IniConfig.bE57YPitchHome)       //kevin 20180827 (Steven) : add put shuttle Y pitch home AutoYPitch  //JerryYang 20251218 : IN/OUT ARM支援不同模組
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
       case 1030:
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
            if(CheckOutArmToTask50(iWhichShuttle))
            {
                Task=50;
                break;
            }

            if((iCleanOut==1 && HasICUnderMachine()==false) ||
               (iCleanOut==1 && LastSet.iTemperature==Tempture_Hot &&
                                (TrayForm.bAutoFeed==false && bMustCleanAllTray==false) &&
                                HasICUnderHotPlate()==false))                                                           //JerryYang 20190807 fix 快速clean out hang up
            {
                FirstEnter=true;
                Task=1;
                break;
            }

            if(OutSHT1InRT() && bCheckShuttle1Flag==false)
            {
                if(FRCarryKit.UseSiteHasIC())
                {
                    if(CheckShuttleICPos(1))
                    {
                        Task=1150;
                    }
                    else
                    {
                        InitPickFromShuttle1Task();                                                                     //Sam 20230208 : 修正 Fix AOI 未拍照問題
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

            if(Task!=1150)                                                                                              //Steven 20180813 : add Out arm speed
                break;
        case 1150:
            if(bCheckSpeed)                                                                                             //Steven 20110525 : Auto Speed
            {
                bCheckSpeed=false;
                OutArmSubSpeed();
            }

            if(bCheckSpeed1)
            {
                bCheckSpeed1=false;
                OutArmSubSpeedDisplay();
            }

            if(MoveOutArmToShuttleIncludeZ_9045_2x1_2(iWhichShuttle, false))
            {
                IsCatchTrayReadySupplyNewTray();

                iBackMode1=GetNowShuttleMode_2x1_2(0);

                if(IniConfig.bA26MotorSpeedSortDisplay)                                                                 //KaiChen 20171225 (Steven)：Add Speed Display
                    bCheckSpeed1=true;
                else
                    bCheckSpeed1=false;
                Task=1100;
            }

            if(Task!=1100)                                                                                              //Steven 20180813 : add Out arm speed
                break;
        case 1100:
            IsCatchTrayReadySupplyNewTray();

            if(WhichAutoNeedTray()!=0)                                                                                  //JerryYang 20170731 (wei) Out arm要讓位才不會hang up
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
                    MoveOutArmXY_ToFix_Tray_Full();                                                                         //Steven 20231027 : 先檢查有沒有要換Tray
                    Task=1160;
                }
            }
            else if(FRCarryKit.UseSiteHasIC() && OutSHT1InRT() && bCheckShuttle1Flag==false)
            {
//#ifdef Carry4
//                MOT[MOutShuttle1].fCanMoveR=false;
//#else
                MOT[MInShuttle1].fCanMoveR=false;
//#endif
                InitPickFromShuttle1Task();
                Task=1200;

                iMode1=GetNowShuttleMode_2x1_2(0);
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
                break;
            }
            else if(BRCarryKit.UseSiteHasIC() && OutSHT2InRT() && bCheckShuttle2Flag==false)                            // change another shuttle
            {
                Task=50;
                break;
            }
            else if(IsOutArmCleanOutFinish())                                                                           //Steven 20230323 : fixed for out arm hang up
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
            if(bCheckSpeed)                                                                                             //Steven 20110525 : Auto Speed
            {
                bCheckSpeed=false;
                OutArmAddSpeed();
            }

            if(bCheckSpeed1)                                                                                            //KaiChen 20171225 (Steven)：Add Speed Display
            {
                bCheckSpeed1=false;
                OutArmAddSpeedDisplay();
            }

            if(DoPickFromShuttle_9045_2x1_2(0))
            {
                PreSetOutAdditionalFlag();                                                                              //Steven 20220614 : Fixed for out arm additional function
                FRCarryKit.SetUnuseAndHasNullICToNullIC();
                if(FRCarryKit.UseSiteHasIC())
                {
                    InitPickFromShuttle1Task();                                                                         //Sam 20230208 : 修正 Fix AOI 未拍照問題
                    return;
                }

                OldPos=0;
                if(OutArmSuck.HasIC()==false)
                {
                    Task=50;
                }
                else
                {
                    iOutRotateFinish=0;                                                                                 //kevin 20130524 目前放到 auto tray
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
        case 1235:
            bOutArmManualStepPress=false;
            if(MoveOutArmToAutoSafe())
                Task=1150;
            break;
        case 2000:
            if(USE_OUT_Y_IS_AUTO_PITCH==true && IniConfig.bE57YPitchHome)                                               //kevin 20180827 (Steven) : add put shuttle Y pitch home AutoYPitch  //JerryYang 20251218 : IN/OUT ARM支援不同模組
            {
                Task=2010;
                return;
            }
            Task=2040;
            break;
        case 2010:                                                                                                      //kevin 20180827
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
        case 2030:
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
            if(CheckOutArmToTask50(iWhichShuttle))
            {
                Task=50;
                break;
            }

            if((iCleanOut==1 && HasICUnderMachine()==false) ||
               (iCleanOut==1 && LastSet.iTemperature==Tempture_Hot &&
                               (TrayForm.bAutoFeed==false && bMustCleanAllTray==false) &&
                               HasICUnderHotPlate()==false))                                                            //JerryYang 20190807 fix 快速clean out hang up
            {
                FirstEnter=true;
                Task=1;
                break;
            }

            if(OutSHT2InRT() && bCheckShuttle2Flag==false)
            {
                if(BRCarryKit.UseSiteHasIC())
                {
                    if(CheckShuttleICPos(2))
                    {
                        Task=2050;
                    }
                    else
                    {
                        InitPickFromShuttle2Task();                                                                     //Sam 20230208 : 修正 Fix AOI 未拍照問題
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

            if(Task!=2050)                                                                                              //Steven 20180813 : add Out arm speed
                break;
        case 2050:
            if(bCheckSpeed)                                                                                             //Steven 20110525 : Auto Speed
            {
                bCheckSpeed=false;
                OutArmSubSpeed();
            }

            if(bCheckSpeed1)                                                                                            //KaiChen 20171225 (Steven)：Add Speed Display
            {
                bCheckSpeed1=false;
                OutArmSubSpeedDisplay();
            }

            if(MoveOutArmToShuttleIncludeZ_9045_2x1_2(iWhichShuttle, false))
            {
                IsCatchTrayReadySupplyNewTray();

                iBackMode2=GetNowShuttleMode_2x1_2(1);
                Task=2100;
            }

            if(Task!=2100)                                                                                              //Steven 20180813 : add Out arm speed
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
            else if(BRCarryKit.UseSiteHasIC() && OutSHT2InRT() && bCheckShuttle2Flag==false)
            {
//#ifdef Carry4
//                MOT[MOutShuttle2].fCanMoveR=false;
//#else
                MOT[MInShuttle2].fCanMoveR=false;
//#endif
                InitPickFromShuttle2Task();
                Task=2200;

                iMode2=GetNowShuttleMode_2x1_2(1);
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
                break;
            }
            else if(FRCarryKit.UseSiteHasIC() && OutSHT1InRT() && bCheckShuttle1Flag==false)
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
            if(bCheckSpeed)                                                                                             //Steven 20110525 : Auto Speed
            {
                bCheckSpeed=false;
                OutArmAddSpeed();
            }

            if(bCheckSpeed1)                                                                                            //KaiChen 20171225 (Steven)：Add Speed Display
            {
                bCheckSpeed1=false;
                OutArmAddSpeedDisplay();
            }

            IsCatchTrayReadySupplyNewTray();

            if(DoPickFromShuttle_9045_2x1_2(1))
            {
                PreSetOutAdditionalFlag();                                                                              //Steven 20220614 : Fixed for out arm additional function
                BRCarryKit.SetUnuseAndHasNullICToNullIC();
                if(BRCarryKit.UseSiteHasIC())
                {
                    InitPickFromShuttle2Task();                                                                         //Sam 20230208 : 修正 Fix AOI 未拍照問題
                    return;
                }
                OldPos=1;
                if(OutArmSuck.HasIC()==false)
                {
                    Task=50;
                }
                else
                {
                    iOutRotateFinish=0;                                                                                 //kevin 20130524 目前放到 auto tray
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
                if(CheekNeedToDoOutArmAdditionalFunction())                                                             //Steven 20210609 : 整合Precisor, Rotator, Bottom CCD, Die Clean
                {
                    Task=7000;
                }
                else
                {
                    Task=3010;
                    AseIcRecord="@e02001";                                                                              //kevin 20161228 (jou) 傳送給 ASE 抬頭資料
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
            iOutRotateFinish=3;                                                                                         //kevin 20130524 目前放到 auto tray
            iWhichAuto=SearchTrayToPlace_9045();                                                                        //Steven 20160524 : 簡化Out Arm程式碼

            if(AUTO3_IS_MAGAZINE==1 &&
               iWhichAuto==2 &&
               (bMagCatchTrayfalg || bChaneMagTrayflag))                                                                //Ifor 20231114 add:
            {
                if(iCatchTrayControlManual>=2 || WhichAutoNeedTray()!=0)                                                // catch tray busy
                {
                    MoveOutArmXY_ToFix_Tray_Full();
                }
                break;
            }

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
                if(AUTO3_IS_MAGAZINE==1             &&                                                                  //JerryYang 20221215 : Magazine把fix區當buffer區功能
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
                        if(iHWFix_BinBox==1 && iWhichAuto==eBulkBox)
                            DoFixTrayFullAlarm();                                                                       //kevin 20201207 check Error bin box Fix 1 put tray
                        break;
                    }
                }
            }

            if(iCatchTrayControlManual>=2 || WhichAutoNeedTray()!=0)                                                    //Sam 20180822 : OutArm 是否在安全位置，是否讓位，不然會與 TrayArm 補 Tray 動作互卡 Hange up。
            {
                MoveOutArmXY_ToFix_Tray_Full();
                break;
            }
            Task=3100;
            break;
        case 3020:                                                                                                      //jou 2010-01-25 start : one cycle後，可以將tray盤拉出來
            IsCatchTrayReadySupplyNewTray();
            if(MoveOutArmXY_ToFix_Tray_Full())
            {
                if(MOT[iMMAuto[iWhichAuto]].fHasTray==true)
                    Task=3100;
            }
            break;
        case 3050:
            if(AutoTrayReCheck(iWhichAuto))                                                                             //Ifor 20171031 add Auto Tray ReCheck
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
            Task=3300;                                                                                                  //kevin 20151203 沒有fixtray放ic
        case 3300:
            if(AUTO3_IS_MAGAZINE==1             &&                                                                      //JerryYang 20221215 : Magazine把fix區當buffer區功能
               TestIF_File.iMagFixTrayType==1   &&
               iWhichAuto>=iMagMin              &&
               iWhichBuff>=iFixMin              &&
               iWhichBuff<=iFixMax)
            {
                ct=iAutoIndex[iWhichBuff];
                if(iWhichBuff>=iFixMin &&
                   Sen[SnFixedTrayDetect[ct]].IsOff() &&
                   LastSet.iRealDummy!=DUMMY)                                                                           //wei 20151217 add iWhichAuto>=3避免沒要放FIX盤也偵測有沒有FIX盤
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
                   LastSet.iRealDummy!=DUMMY)                                                                           //wei 20151217 add iWhichAuto>=3避免沒要放FIX盤也偵測有沒有FIX盤
                {
                    Task=4000;
                    break;
                }
            }
            Task=3301;
            if(Task!=3301)                                                                                              //Steven 20180813 : add Out arm speed
                break;
        case 3301:
            if(DoOutArmPlaceToAuto_9045())                                                                              // 吸嘴可能還在下方
            {
                DoOutArmAfterPlaceToAuto(bCheckSpeed, true);                                                            //Steven 20220526 : 針對放下IC到Unloader後的動作做整合
                Task=3500;
            }
            else
            {
                break;
            }
        case 3500:
            ret=DoOutArmAfterPlaceToAuto(bCheckSpeed);                                                                  //Steven 20220526 : 針對放下IC到Unloader後的動作做整合
            if(ret!=0)
            {
                Task=ret;
            }
            break;
        case 4000:
            if(MoveOutArmXY_ToFix_Tray_Full(true))                                                                      //Ifor 20250926 add:Fix Tray 取放時移開Out Arm
                Task=4100;
            break;
        case 4100:
            if(DoFixTrayFullAlarm())                                                                                    //Steven 20181113 : 修正Fix Link Alarm的問題
            {
                Task=3010;
            }
            break;
        case 7000:
            flag3=DoOutArmAdditionalFunction();                                                                         //Steven 20210609 : 整合Precisor, Rotator, Bottom CCD, Die Clean
            if(flag3)
            {
                AseIcRecord="@e02001";                                                                                  //kevin 20161228 (jou) 傳送給 ASE 抬頭資料
                Task=3010;
            }
            break;
        case 10000:                                                                                                     //JerryYang 20220909 : add magazine
            if(CheckPlaceToMagazineTray(iWhichMag))
            {
                Task=12000;                                                                                             // To Auto 3
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
                MoveOutArmXY_ToFix_Tray_Full();                                                                         // change Magazine Tray
                break;
            }

            bMagNeedChangeTray=false;
            if(bMagCatchTrayfalg)                                                                                       //JerryYang 20221121 : AUTOI3 Magazine還在夾TRAY的旗標
            {
                break;
            }

            if(MOT[MTrayX].ReadPos()>Prod.iXTrayColor)
            {
                MoveOutArmXY_ToFix_Tray_Full();
                break;
            }

            InitPlaceToAutoTask();
            if(iCatchTrayControlManual>=2)                                                                              // catch tray busy
            {
                MoveOutArmXY_ToFix_Tray_Full();
                break;
            }
            Task=3100;
            break;
    }
}
//---------------------------------------------------------------------------
