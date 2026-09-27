// =============================================================================
//  aoutarm9045_1x4_2.cpp  --  Out-Arm per-site VARIANT (1x4_2, single-row 4-picker 2-pick)
//
//  Faithful translation of golden aoutarm9045_1x4_2.cpp (BCB6, Big5/cp950).
//  Translator: AI(W6.2c-1x4_2) 20260627
//  Translation wave: W6.2c (out-arm per-site-config variant: 1x4_2).
//
//  ROLE: the per-site out-arm shuttle-pick / place-to-auto state machines for the
//  1x4_2 layout.  This file owns (golden names VERBATIM):
//    GetNowShuttleMode_1x4_2, OutArmZNeedDown_1x4_2, CheckOutArmXYPitch_1x4_2,
//    MoveOutArmToShuttleIncludeZ_9045_1x4_2, DoPickFromShuttle_9045_1x4_2,
//    DoOutArm_9045_1x4_2.
//
//  ENGINE DISPATCH (LIVE variant): the out-arm engine aoutarm9045.cpp routes
//  iInArmType==e9045_1x4_2_14 -> DoOutArm_9045_1x4_2() (golden aoutarm9045.cpp
//  dispatch ladder :495/:497) and DoPickFromShuttle_9045_1x4_2(int) (:635/:637).
//  The file layout suffix == the callee suffix (ENUM->CALLEE map: 1x4_2_14 -> 1x4_2).
//
//  FAITHFUL: every fn/var/cursor value, switch(Task) structure, formula, branch,
//  fall-through and commented-out code is transcribed VERBATIM from the golden
//  CP950 source.  The DoOutArm_9045_1x4_2 case-3301 fall-through into case-3500,
//  case-3100 fall-through into case-3300, case-10/case-200, case-1/5/10/50/100,
//  case-1140->1150, case-2040->2050 fall-throughs are preserved EXACTLY.
//
//  VCL/Borland conversions: VCL AnsiString (1-based) via vclcompat; this file has
//  no __fastcall/__property/__published/PACKAGE.  SOFT_SIMULTE is NOT defined ->
//  the one #ifdef SOFT_SIMULTE block in DoPickFromShuttle (case iOUTARM_SUCK,
//  chkInPickLoadError test) is compiled OUT, kept VERBATIM.
//
//  Big5: every Chinese comment decoded via cp950 and preserved as UTF-8.
//  Final gate: ZERO U+FFFD.
//
//  INCLUDE REWRITE (mirror of the golden header set, translated subset):
//    golden: MachineDefine.h ; aoutarm9045_1x4_2.h ; aArmHeader.h ; MyKitSuck.h ;
//            cprod.h ; mymotor.h ; mycylin.h ; cSortCT.h ; mysensor.h ; main.h ;
//            note.h ; csystem.h ; acatchtray.h ; cinitial.h ; atester.h ;
//            aRotateKIT.h ; fRotate.h ; cOffSet.h ; fAOI.h ; cmydef.h ;
//            ProductionInfo.h ; Magazine.h ; #pragma package(smart_init)
// =============================================================================
#include "MachineDefine.h"
#include "MachineType.h"            // e9045_1x4_2_14 / e9045_1x4_4_13 / e9045_1x2_2_14 / ep16Picker / eBulkBox / CC_KYEC_LEE global enums
//---------------------------------------------------------------------------
#include "aoutarm9045_1x4_2.h"
#include "aArmHeader.h"             // __FUNC__ shim, RecordProcess, OutArm, etc.
#include "aoutarm9045.h"            // engine: GetOutArmPitch*_9045 / DoOutArmSuckPreOn / IsCheckOutArmDestroyActiveFinish / GetOutShuttleStatus_9045 / InitialOutArmNeedSuck / OutArmNeedCheckOffset / iOUTARM_SUCK / SearchTrayToPlace_9045 / VerifyFixTrayLink / DoOutArmPlaceToAuto_9045 / CheekNeedToDoOutArmAdditionalFunction / MoveOutArmXY_To_ESDSafePos / DoOutArmAdditionalFunction / DoOutArmAfterPlaceToAuto / InitPlaceToAutoTask
#include "csystem.h"                // OutSHT1InRT / OutSHT2InRT / HasICUnderMachine / HasICUnderHotPlate / IsOutArmCleanOutFinish / AutoTrayReCheck
#include "Motor/mymotor.h"          // MOT[], OutArmContinuousMove_9045
#include "cmydef.h"                 // global scalar universe + IC consts + iInArmType + enums + bEnterOffset / bCheckShuttle1/2Flag / iCleanOut / IniConfig / USE_* / iXpitch* / Mxxx motor ids / iAutoIndex / iMMAuto / ...
#include "cprod.h"                  // Prod / TestIF / TestIF_File / ArmSpeed / OutArmOffSet / iXTrayColor
#include "cpublic.h"                // CosFunction
#include "common.h"
#include "mysensor.h"               // Sen[] (SnFixedTrayDetect)
#include "acatchtray.h"             // WhichAutoNeedTray
#include "aHotPlateSubstrate.h"     // OutArmSuck / FRCarryKit / BRCarryKit + cursors + SetMotorSpeed + DoOutArmPlaceToAuto_9045 / DoOutArmAdditionalFunction / DoOutArmAfterPlaceToAuto / InitPlaceToAutoTask / iWhichBuff / iWhichMag
#include "acarry_shims.h"           // MoveOutArmToAutoSafe / MoveOutArmXY_ToShuttleAlarmArea / LastSet / NULL_IC
#include "acatchtray_shims.h"       // InitProcessSingleMotorTask / iWhichAuto
#include "atester_shims.h"          // fObserver / fContact offline stand-ins
#include "ainarm9045_2x4_16_shims.h"// AutoTeachLoadTrayZ
#include "FormsFacade.h"            // fMain offline stand-in (chkInPickLoadError gated out by SOFT_SIMULTE)
#include "canary_support.h"         // ShowErrorMessage / ShowMyMessage / RecordProcess / K_RETRY / DUMMY / __FUNC__
//---------------------------------------------------------------------------
#pragma package(smart_init)

//==============================================================================
//==  STANDALONE forward-declares for cross-module / engine-file-local symbols
//==  that have NO header home in scope this wave.  Guarded with #ifndef so the
//==  serial Integrate agent can add the real shared decl ONCE without clashing.
//==  Each is REPORTED in substrateNeeds (name + golden home + offline behavior).
//==  NOTE: several of these are file-local `static` in the engine aoutarm9045.cpp
//==  (NOT link-visible to this separate TU) -> they need a shared offline home.
//==============================================================================
#ifndef OUTARM_1x4_2_FWD_GUARD
#define OUTARM_1x4_2_FWD_GUARD
// shuttle pointer used by the geometry helpers + DoPickFromShuttle (golden file-global)
extern TMyKitSuck *ptrOutSHT;                                                   // golden MyKitSuck.h -- offline: points at &FRCarryKit / &BRCarryKit grids
// out-arm <-> shuttle offset / ratio helpers (golden engine helpers, aoutarm9045.cpp)
int  GetOutArmToShuttleOffset_9045(int iSht, int iRow, int iCol, bool bPlace);  // golden aoutarm9045.cpp -- offline: -1 (no offset configured)
int  GetOutArmYToShuttleOffset_9045(int iSht, int iOffsetPos);                  // golden aoutarm9045.cpp -- offline: 0
int  GetOutArmXToShuttleOffset_9045(int iSht, int iOffsetPos);                  // golden aoutarm9045.cpp -- offline: 0
void TransferOutShuttleRatio(int iSht, int *iX, int *iY, int iYBase, int iXBase);// golden aoutarm9045.cpp -- offline: identity (no ratio transform)
int  GetVariableYOutShuttleData();                                              // golden aoutarm9045.cpp -- offline: 0
void SetOutArmNeedDestory(int iRow, int iCol, int iShtRow, int iShtCol, bool bPlace=false);   // AI(W906-ARM1) 20260927: golden aoutarm.h:171 的原型（以前 4 參數 ⇒ 連到 aoutarm_shims.cpp 的空樁，吸料那一步不設 NeedSuck）—— NB2 R105 ③
// out-arm pick/data-swap surface (golden engine helpers)
void PickFromShuttle(int iWhichKit=0);                                          // golden aoutarm9045.cpp / kevin 20210612 -- offline: no-op
bool bOutShtwaitPick();                                                         // golden aoutarm9045.cpp / kevin 20210602 -- offline: false (no wait)
bool SwapShuttleDataToOutArm(int iSht, int iShtRow, int iShtCol, int iSuckRow, int iSuckCol, int *iWitchErrBin); // golden aoutarm9045.cpp / Steven 20170428 -- offline: false (no miss)
void ShowOutputShuttleDataMiss();                                              // golden aoutarm9045.cpp -- offline: no-op
bool ErrorBinBoxDetect(int iWitchErrBin);                                      // golden aoutarm9045.cpp / kevin 20160906 -- offline: true
int  OutArmPickShuttleAlarm(int iSht, bool bHasDuplicateErr, AnsiString ErrPart);// golden aoutarm9045.cpp -- offline: K_SKIP
void PorcessJAM0201OutArmPickUpErrorSkip(int iSht, int i, int iShtCol, int ii, int iSuckCol); // golden aoutarm9045.cpp / Steven 20161214 -- offline: no-op
bool CheckTesterZ(int iSht);                                                    // golden aoutarm9045.cpp -- offline: false (tester Z clear)
bool CheckRotateOutNotFinish();                                                // golden aoutarm9045.cpp / kevin 20130524 -- offline: false
bool CheckOutArmInitState();                                                    // golden aoutarm9045.cpp -- offline: true
bool CheckOutArmToTask50(int iWhichShuttle);                                    // golden aoutarm9045.cpp -- offline: false
bool CheckShuttleICPos(int iSht);                                              // golden aoutarm9045.cpp -- offline: true
void InitPickFromShuttle1Task();                                              // golden aoutarm9045.cpp / Sam 20230208 -- offline: no-op
void InitPickFromShuttle2Task();                                              // golden aoutarm9045.cpp / Sam 20230208 -- offline: no-op
// out-arm speed display helpers (golden engine helpers)
void OutArmSubSpeed();                                                          // golden aoutarm9045.cpp / Steven 20110525 -- offline: no-op
void OutArmAddSpeed();                                                          // golden aoutarm9045.cpp / Steven 20110525 -- offline: no-op
void OutArmSubSpeedDisplay();                                                   // golden aoutarm9045.cpp / KaiChen 20171225 -- offline: no-op
void OutArmAddSpeedDisplay();                                                   // golden aoutarm9045.cpp / KaiChen 20171225 -- offline: no-op
// out-arm move targets / clean-out / home (engine file-local statics -> need shared home)
bool MoveOutArmXY_ToFix_Tray_Full(bool bAlarm=false);                          // golden aoutarm.h:41 -- offline: true (reached)
bool IsCatchTrayReadySupplyNewTray();                                          // golden aoutarm.h:64 -- offline: false (catch-tray not ready)
int  CheckOutArmCleanOut(int Task=50);   // AI(W906-ARM1) 20260927: golden aoutarm.h:58 的原型（以前 0 參數 ⇒ 連到 aoutarm_shims.cpp 回 1140 的樁；golden 一般情況回 Task＝50）—— NB2 R105 ⑦
void SetOutArmHome();                                                           // golden aoutarm.h:97 -- offline: no-op
// out-arm additional-flag / fall-down / fix-tray-full / tray-status surface
void PreSetOutAdditionalFlag();                                                // golden aoutarm9045.cpp / Steven 20220614 -- offline: no-op
// CheckOutArmSuckICFallDown() is declared in csystem.h (bool) -- not re-declared here
bool DoFixTrayFullAlarm();                                                      // golden aoutarm9045.cpp -- offline: true
void SetFixTrayFullIC();                                                        // golden aoutarm9045.cpp -- offline: no-op
int  VerifyTrayStatus();                                                        // golden aoutarm9045.cpp -- offline: 3300 (proceed to place)
// magazine surface (golden Magazine.h)
bool CheckPlaceToMagazineTray(int iWhichMag);                                  // golden Magazine.h -- offline: true
bool DoPickFromMagazineBuffer();                                               // golden Magazine.h -- offline: true
int  SearchTrayToPlace_Magazine();                                             // golden aoutarm9045.h:63 -- AI(W906-W7-A2) 20260728: was void (ODR split vs. golden's int; see aoutarm_shims.cpp)
// out-arm Y-pitch home timer + flag (golden file-globals)
extern TQPF_Timer hOutArmYpitchHomeTimer;                                      // golden aoutarm.h -- offline: software timer
extern bool       bYpitchNeddHome;                                             // golden cmydef.h (also) -- offline flag
// carry-control flags (golden file-globals, jou 2011-11-21)
extern bool bCarryControlOutarm1;                                              // golden -- offline: false
extern bool bCarryControlOutarm2;                                              // golden -- offline: false
// out-arm Z auto-teach task cursor (golden file-global)
extern int  iOutArmZTeachTask;                                                 // golden aoutarm.h -- offline: cursor
// out-shuttle duplicate-error map (golden file-global, Steven 20110216)
extern bool bOutSuckShtDupErr[MAX_ARM_Row][MAX_ARM_Col];                       // golden aoutarm9045.cpp -- offline: zeroed
// out-shuttle retry counter (golden file-global)
extern int  iOutShtRetryCount;                                                 // golden aoutarm9045.cpp -- offline: 0
// out-arm manual single-step press flag (golden file-global)
extern bool bOutArmManualStepPress;                                            // golden aoutarm9045.cpp -- offline: false
// per-shuttle pick task cursors (golden file-globals, declared via aoutarm headers not in scope)
extern int  iPickFromShuttle1Task;                                             // golden aoutarm9045.cpp -- offline: SM cursor
extern int  iPickFromShuttle2Task;                                             // golden aoutarm9045.cpp -- offline: SM cursor
// the out-arm engine task cursor (golden file-global; engine OutArmTask)
extern int  OutArmTask;                                                        // golden aoutarm9045.cpp -- offline: engine SM cursor
// out-rotate handshake + place-target index (golden file-globals; engine defines them non-static)
extern int  iOutRotateFinish;                                                  // golden aoutarm9045.cpp / kevin 20130524 -- offline: 0
extern int  iWhichAuto;                                                        // golden aoutarm.h:18 -- offline: place-target Auto index
// y-pitch home predicates (golden ainarm9045.h:114/115; that header clashes with the substrate, so declared here)
bool bCheckYPitchHome(int iXYHome);                                            // golden ainarm9045.h:114 / kevin 20180822 -- offline: true
bool bCheckYPitchRunHomeSen(int iXYHome);                                      // golden ainarm9045.h:115 / kevin 20180822 -- offline: true
// place-to-auto task init (golden aoutarm9045.cpp helper, no header in scope)
void InitPlaceToAutoTask();                                                    // golden aoutarm9045.cpp -- offline: no-op
#endif // OUTARM_1x4_2_FWD_GUARD

//==============================================================================
//  GetNowShuttleMode_1x4_2  -- golden aoutarm9045_1x4_2.cpp:29
//==============================================================================
int GetNowShuttleMode_1x4_2(int iSht)
{
    ptrOutSHT=(iSht==0)?&FRCarryKit:&BRCarryKit;

    if(ptrOutSHT->UseSiteNoIC())
    {
        return 0;
    }
    else if(OutArmSuck.iXStep==1)
    {
        if(ptrOutSHT->Item[0][0]>=HAS_IC ||
           ptrOutSHT->Item[0][1]>=HAS_IC)
        {
            return 0;                                                           //Kit0
        }
        else
        {
            return 1000;
        }
    }
    else if(OutArmSuck.iXStep==2)
    {
        if(ptrOutSHT->Item[0][0]>=HAS_IC)
        {
            return 10000;                                                       //Kit0
        }
        else if(ptrOutSHT->Item[0][1]>=HAS_IC)
        {
            return 10100;                                                       //Kit0
        }
        else if(ptrOutSHT->Item[0][2]>=HAS_IC)
        {
            return 11000;                                                       //Kit1
        }
        else
        {
            return 11100;                                                       //Kit1
        }
    }
    return 0;
}
//==============================================================================
//  OutArmZNeedDown_1x4_2  -- golden aoutarm9045_1x4_2.cpp:71
//==============================================================================
bool OutArmZNeedDown_1x4_2(int iSht)
{
    if(InitialOutArmNeedSuck(iSht)==false)
        return false;

    ptrOutSHT   =(iSht==0)?&FRCarryKit:&BRCarryKit;
    int iMode   =GetNowShuttleMode_1x4_2(iSht);
//    int iModeRow=iMode%100;
    int iModeCol=iMode/100;

    if(iModeCol==0)                                                             //全部一起
    {
        SetOutArmNeedDestory(0, 0, 0, 0);
        SetOutArmNeedDestory(0, 1, 0, 1);
    }
    else if(iModeCol==10)
    {
        SetOutArmNeedDestory(0, 0, 0, 2);
        SetOutArmNeedDestory(0, 1, 0, 3);
    }
    else if(iModeCol==100)
    {
        SetOutArmNeedDestory(0, 0, 0, 0);
    }
    else if(iModeCol==101)
    {
        SetOutArmNeedDestory(0, 1, 0, 1);
    }
    else if(iModeCol==110)
    {
        SetOutArmNeedDestory(0, 0, 0, 2);
    }
    else if(iModeCol==111)
    {
        SetOutArmNeedDestory(0, 1, 0, 3);
    }
    return true;
}
//------------------------------------------------------------------------------
//  CheckOutArmXYPitch_1x4_2  -- golden aoutarm9045_1x4_2.cpp:110
//------------------------------------------------------------------------------
bool CheckOutArmXYPitch_1x4_2(int *iX, int *iY, int iSht, int iKit, int iMovePitchX, int iMovePitchY)
{
    int iMode       =GetNowShuttleMode_1x4_2(iSht);
    int iModeRow    =iMode%100;
    int iModeCol    =iMode/100;
    int iOffsetPos  =GetOutArmToShuttleOffset_9045(iSht, iModeRow, iModeCol+iKit*10, false);                            //Steven 20240826 : Fixed for offset
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
            *iY=*iY-iMovePitchY;
        else
            *iY=*iY;
    }
    else if(iModeRow==1)                                                        //Row B
    {
        if(USE_OUT_Y_IS_AUTO_PITCH==true)                                       //JerryYang 20251218 : IN/OUT ARM支援不同模組
            *iY=*iY;
        else
            *iY=*iY+iMovePitchY;
    }
    else                                                                        //Both
    {
        if(USE_OUT_Y_IS_AUTO_PITCH==true)                                       //JerryYang 20251218 : IN/OUT ARM支援不同模組
            *iY=*iY;
        else
            *iY=*iY;
    }

    if(USE_OUT_ARM_Y_PITCH==iXYPitch16Bd_Be)                                    //Ztex 2023.12.06 Add HT-1032  //JerryYang 20251218 : IN/OUT ARM支援不同模組
    {
    }
    else if(USE_PICKER_COUNT==ep16Picker)                                       //假裝為採四個吸嘴
    {
        dMovePitchX=double(iMovePitchX)/7.0;
    /*    if(iModeCol==0)
        {
            if(iInArmType==e9045_1x2_2_14)                                      //Aa --> Aa
                *iX=*iX+(dMovePitchX*3.0-TestIF.dSiteXPitch*0.5);
            else                                                                //Aa --> Aa
                *iX=*iX+(dMovePitchX*3.0-TestIF.dSiteXPitch*0.5);
        }
        else if(iModeCol==10)
        {
            if(iInArmType==e9045_1x2_2_14)                                      //Aa --> Ac     //使用2, 3吸嘴
                *iX=*iX+(dMovePitchX*3.0+TestIF.dSiteXPitch*0.5);
            else                                                                //Aa --> Ac
                *iX=*iX+(dMovePitchX*3.0+TestIF.dSiteXPitch*0.5);
        }
        else if(iModeCol==110)
        {
            if(iInArmType==e9045_1x2_2_14)                                      //Ag --> Ab
                *iX=*iX-(dMovePitchX*3.0+TestIF.dSiteXPitch*0.5);
            else                                                                //Ae --> Ab
                *iX=*iX-(dMovePitchX*1.0+TestIF.dSiteXPitch*0.5);
        } */
    }
    else if(USE_OUT_ARM_Y_PITCH==iXYPitchVariable)                              //假裝為採第二個吸嘴  //JerryYang 20251218 : IN/OUT ARM支援不同模組
    {
        dMovePitchX=double(iMovePitchX)/3.0;
        if(iInArmType==e9045_1x4_4_13)                                          //Steven 20251024 : fixes for 1x4_4
        {
            if(iModeCol==0)                                                     //Aa --> Aa
            {
                *iX=*iX+(dMovePitchX*1.0-TestIF.dSiteXPitch*1.5);
            }
            else if(iModeCol==10)                                               //Aa --> Ac
            {
                *iX=*iX+(dMovePitchX*1.0+TestIF.dSiteXPitch*0.5);
            }
            else if(iModeCol==100)                                              //Aa --> Aa
            {
                *iX=*iX+(dMovePitchX*1.0-TestIF.dSiteXPitch*1.5);
            }
            else if(iModeCol==101)                                              //Ac --> Ab
            {
                *iX=*iX+(-dMovePitchX*1.0-TestIF.dSiteXPitch*0.5);
            }
            else if(iModeCol==110)                                              //Aa --> Ac
            {
                *iX=*iX+(dMovePitchX*1.0+TestIF.dSiteXPitch*0.5);
            }
            else if(iModeCol==111)                                              //Ac --> Ad
            {
                *iX=*iX+(-dMovePitchX*1.0+TestIF.dSiteXPitch*1.5);
            }
        }
        else
        {
            if(iModeCol==0)                                                     //Aa --> Aa
            {
                *iX=*iX+(dMovePitchX*1.0-TestIF.dSiteXPitch*1.5);
            }
            else if(iModeCol==10)                                               //Aa --> Ac
            {
                *iX=*iX+(dMovePitchX*1.0+TestIF.dSiteXPitch*0.5);
            }
            else if(iModeCol==100)                                              //Aa --> Aa
            {
                *iX=*iX+(dMovePitchX*1.0-TestIF.dSiteXPitch*1.5);
            }
            else if(iModeCol==101)                                              //Ad --> Ab
            {
                *iX=*iX+(-dMovePitchX*2.0-TestIF.dSiteXPitch*0.5);
            }
            else if(iModeCol==110)                                              //Aa --> Ac
            {
                *iX=*iX+(dMovePitchX*1.0+TestIF.dSiteXPitch*0.5);
            }
            else if(iModeCol==111)                                              //Ad --> Ad
            {
                *iX=*iX+(-dMovePitchX*2.0+TestIF.dSiteXPitch*1.5);
            }
        }
    }
    else                                                                        //假裝為採第三個吸嘴
    {
        dMovePitchX=double(iMovePitchX)/3.0;
        if(iInArmType==e9045_1x4_4_13)                                          //Steven 20251024 : fixes for 1x4_4
        {
            if(iModeCol==0)
            {
                *iX=*iX+(dMovePitchX*2.0-TestIF.dSiteXPitch*1.5);               //Aa --> Aa
            }
            else if(iModeCol==10)
            {
                *iX=*iX+(dMovePitchX*2.0+TestIF.dSiteXPitch*0.5);               //Aa --> Ac
            }
            else if(iModeCol==100)
            {
                *iX=*iX+(dMovePitchX*2.0-TestIF.dSiteXPitch*1.5);               //Aa --> Aa
            }
            else if(iModeCol==101)
            {
                *iX=*iX+(dMovePitchX*0.0-TestIF.dSiteXPitch*0.5);               //Ac --> Ab
            }
            else if(iModeCol==110)
            {
                *iX=*iX+(dMovePitchX*2.0+TestIF.dSiteXPitch*0.5);               //Aa --> Ac
            }
            else if(iModeCol==111)
            {
                *iX=*iX+(dMovePitchX*0.0+TestIF.dSiteXPitch*1.5);               //Ad --> Ad
            }
        }
        else
        {
            if(iModeCol==0)
            {
                *iX=*iX+(dMovePitchX*2.0-TestIF.dSiteXPitch*1.5);               //Aa --> Aa
            }
            else if(iModeCol==10)
            {
                *iX=*iX+(dMovePitchX*2.0+TestIF.dSiteXPitch*0.5);               //Aa --> Ac
            }
            else if(iModeCol==100)
            {
                *iX=*iX+(dMovePitchX*2.0-TestIF.dSiteXPitch*1.5);               //Aa --> Aa
            }
            else if(iModeCol==101)
            {
                *iX=*iX+(-dMovePitchX*1.0-TestIF.dSiteXPitch*0.5);              //Ad --> Ab
            }
            else if(iModeCol==110)
            {
                *iX=*iX+(dMovePitchX*2.0+TestIF.dSiteXPitch*0.5);               //Aa --> Ac
            }
            else if(iModeCol==111)
            {
                *iX=*iX+(-dMovePitchX*1.0+TestIF.dSiteXPitch*1.5);              //Ad --> Ad
            }
        }
    }

    *iY=*iY+GetOutArmYToShuttleOffset_9045(iSht, iOffsetPos);
    *iX=*iX+GetOutArmXToShuttleOffset_9045(iSht, iOffsetPos);
    TransferOutShuttleRatio(iSht, iX, iY, iOutArmYBase, iOutArmXBase);
    if(OutArmZNeedDown_1x4_2(iSht)==false)
        return false;
    return true;
}
//==============================================================================
//  MoveOutArmToShuttleIncludeZ_9045_1x4_2  -- golden aoutarm9045_1x4_2.cpp:304
//==============================================================================
bool MoveOutArmToShuttleIncludeZ_9045_1x4_2(int iSht, int iKit, bool bZDown)
{
    bool bZFlag[MAX_ARM_Row][MAX_ARM_Col];
    int iZPos[MAX_ARM_Row][MAX_ARM_Col];
    int iXVariable[X_PITCH_COUNT];
    int iXPos               =iOutArmShtXCenterPos;                              //Steven 20141029 : XY-Pitch for Shuttle Center position for base Suck
    int iYPos               =iOutArmShtYCenterPos;
    int iMode               =GetNowShuttleMode_1x4_2(iSht);
    int iModeRow            =iMode%100;
    int iModeCol            =iMode/100;
    int iMovePitchX         =iXpitchMaxX3;
    int iMovePitchY         =GetVariableYOutShuttleData();
    int iOffsetPos          =GetOutArmToShuttleOffset_9045(iSht, iModeRow, iModeCol+iKit*10, true);                     //Steven 20240826 : Fixed for offset
    int iYVariable          =GetOutArmPitchY_9045(iMovePitchY, iOffsetPos);
    ZeroMemory(iXVariable,  sizeof(iXVariable));
    ZeroMemory(bZFlag,      sizeof(bZFlag));
    ZeroMemory(iZPos,       sizeof(iZPos));

    if(USE_16PICKER_TYPE==1)
    {
        if(OutArmSuck.iXStep==1)
            iMovePitchX=TestIF.dSiteXPitch/6*7;
        else
            iMovePitchX=iXpitchMaxX7;
    }
    else
    {
        if(OutArmSuck.iXStep==1)
        {
            if(TestIF_File.dSiteXPitch>iXpitchMaxX2_MM)
            {
                iMovePitchX=TestIF.dSiteXPitch;
            }
            else if(iInArmType==e9045_1x4_4_13)                                 //Steven 20251024 : fixes for 1x4_4
            {
                iMovePitchX=TestIF.dSiteXPitch/2*3;
            }
            else if(iInArmType==e9045_1x4_2_14 &&                               //RogerYang 20260326 : for SiteXPitch=80，修正AG吸嘴對kit col1&2、3&4
                    TestIF_File.dSiteXPitch==iXpitchMaxX2_MM)
            {
                iMovePitchX=TestIF.dSiteXPitch;
            }
            else
            {
                iMovePitchX=TestIF.dSiteXPitch/3*2;
            }
        }
        else                                                                    //if(OutArmSuck.iXStep==2)
        {
            iMovePitchX=iXpitchMaxX3;
        }
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

    if(CheckOutArmXYPitch_1x4_2(&iXPos, &iYPos, iSht, iKit, iMovePitchX, iMovePitchY)==false && bZDown==true)
        return false;

    GetOutShuttleStatus_9045(iSht, iZPos, bZDown, bZFlag);
    if(IsCheckOutArmDestroyActiveFinish(iXPos, iYPos)==false)                   //2013-08-02    Dell     add Pick 前先確認CheckInArmDestroyActive 已完成
        return false;

    DoOutArmSuckPreOn(iSht, iKit, iXPos, iYPos, bZDown);
    if(OutArmContinuousMove_9045(iXPos, iYPos, iXVariable, iYVariable, bZFlag, iZPos, bZDown))
        return true;

    return false;
}
//==============================================================================
// 從出料 Shuttle 1 吸取 IC 流程動作
//  Ver : 2003_07
//==============================================================================
//  DoPickFromShuttle_9045_1x4_2  -- golden aoutarm9045_1x4_2.cpp:385
//==============================================================================
bool DoPickFromShuttle_9045_1x4_2(int iSht)
{
    int ret;
    int &Task=(iSht==0)?iPickFromShuttle1Task:iPickFromShuttle2Task;
    static int iWitchErrBin=0;
    AnsiString ErrPart="";                                                      //Steven 20110216 : 出料Alarm
    bool bHasDuplicateErr=false;                                                //Steven 20110216 : 出料Alarm
    bool bHasErr=false;                                                         //Steven 20110216 : 出料Alarm
    bool flag=false;
    int iSuckRow=0, iSuckCol=0, iShtRow=0, iShtCol=0;                           //JerryYang 20231004 : single site suck XY固定00
    static bool bDoLeftShuttle=true;
    ptrOutSHT=(iSht==0)?&FRCarryKit:&BRCarryKit;
    int iPickKit=(iOutArmiWhichKit==0)?0:OutArmSuck.iPickKitStep;

    if(CheckTesterZ(iSht))
    {
        //MoveOutArmToAutoSafe();                                               //Steven 20150408 : Mark,避免吸嘴上上下下
        return false;
    }

    switch(Task)
    {
        case 1:
            if(ArmSpeed[OutArm].dWaitOnSH!=0)                                                                           //kevin 20210525 out arm 起動在shuttle 上等待 時間
            {
                if(OutArmSuck.NoIC())                                                                                   //kevin 20210615 第一次 吸料IC 待續抓
                {
                    if(MoveOutArmToShuttleIncludeZ_9045_1x4_2(iSht, iOutArmiWhichKit, false))                           //移到 Shuttle 上等待
                    {
                        PickFromShuttle(iOutArmiWhichKit);                                                              //kevin 20210612 add int iwhichKit=0
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
            if(MoveOutArmToShuttleIncludeZ_9045_1x4_2(iSht, iOutArmiWhichKit, true))                                    //kevin 20210717 change function
            {
                if(OutArmNeedCheckOffset(false, iSht))                                                                  //Steven 20230531 : 簡化判斷式
                {
                    Task=500;
                    break;
                }
                OutArmSuck.ResetAll();                                                                                  //Steven 20160323 : 避免有殘留真空
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
                OutArmSuck.ResetAll();                                          //Steven 20160323 : 避免有殘留真空
            }
            break;
        case iOUTARM_SUCK:
            flag=true;
            if(iOutArmiWhichKit==1)
            {
                bDoLeftShuttle=false;
            }
            else
            {
                bDoLeftShuttle=true;
            }

            bHasErr=false;
            for(int i=0; i<OutArmSuck.iPickRow; i++)
            {
                for(int j=0; j<OutArmSuck.iPickCol; j++)
                {
                    iSuckRow  =i;
                    iSuckCol  =j;
                    iShtRow   =i;
                    iShtCol   =j+iPickKit;

                    if(ptrOutSHT->Item[iShtRow][iShtCol] &&
                       OutArmSuck.Suck[iSuckRow][iSuckCol].GetNeedSuckStatus())
                    {
                        if(OutArmSuck.Suck[iSuckRow][iSuckCol].Suck())
                        {
                            //那粒ic有吸嘴了
                            #ifdef SOFT_SIMULTE
                            if(fMain->chkInPickLoadError->Checked==true)
                            {
                                if(i==0 && j==0)
                                {
                                    OutArmSuck.Suck[0][0].Error=true;
                                    continue;
                                }
                            }
                            #endif

                            bHasErr=SwapShuttleDataToOutArm(iSht, iShtRow, iShtCol, iSuckRow, iSuckCol, &iWitchErrBin);                                         //Steven 20170428 : 統一Out Arm從Shuttle吸料後的資料交換
                            bOutSuckShtDupErr[iSuckRow][iSuckCol]=false;
                        }
                        else if(OutArmSuck.Suck[iSuckRow][iSuckCol].Error==false)
                        {
                            flag=false;
                        }
                    }
                }
            }

            if(bHasErr)
            {
                ShowOutputShuttleDataMiss();
            }

            if(flag==false)                                                                                                                                     //KEVIN 20130421 有RETRY資料尚未交換會將其他IC丟掉
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
                    if(OutArmSuck.Item[i][j]==NULL_IC && OutArmSuck.Suck[i][j].GetNeedSuckStatus())
                        return false;
            }

            if(iHWFix_BinBox==1 && ErrorBinBoxDetect(iWitchErrBin)==false)
                return false;                                                                                                                                   //kevin 20160906 add error bin box
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
                        for(int i=0; i<OutArmSuck.iPickRow; i++)
                        {
                            for(int j=0; j<OutArmSuck.iPickCol; j++)
                            {
                                if(OutArmSuck.Suck[i][j].Error)
                                {
                                    iShtCol=j+iPickKit;
                                    PorcessJAM0201OutArmPickUpErrorSkip(iSht, i, iShtCol, i, iSuckCol);                 //Steven 20161214 : 加上OutArm吸料異常Skip的ErrorLog
                                }
                            }
                        }
                        iOutShtRetryCount=0;
                        if((bDoLeftShuttle==true  && ptrOutSHT->LeftSideNoIC(OutArmSuck.iShtKitStep)) ||
                           (bDoLeftShuttle==false && ptrOutSHT->RightSideNoIC(OutArmSuck.iShtKitStep)))                 //Steven 20170327 (Jou) : 修改Out資料判斷方式
                        {
                            ZeroMemory(bOutSuckShtDupErr, sizeof(bOutSuckShtDupErr));
                            Task=1;
                            return true;
                        }
                    }
                    else                                                                                                //Steven 20190313 : Fixed for out arm pick error need home
                    {
                        if(iOutShtRetryCount>ArmSpeed[OutArm].iRetryCT)
                        {
                            iOutShtRetryCount=0;
                            if(IniConfig.bInOutArmCanPushHome)                                                          //ChungHung HT9045 2011/12/13 //Input pickup device error後,按"retry"後,手臂頭會自動home
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
//  DoOutArm_9045_1x4_2  -- golden aoutarm9045_1x4_2.cpp:657
//==============================================================================
void DoOutArm_9045_1x4_2()
{
    int ret, ct;
    static int OldPos=0;
    static bool FirstEnter=true, bCheckSpeed=false;
    int &Task=OutArmTask;
    static int iWhichShuttle=0;
    bool flag3;
    static bool bCheckSpeed1=false;                                             //KaiChen 20171225 (Steven)：Add Speed Display

    if(bCarryControlOutarm1 || bCarryControlOutarm2)                            //jou 2011-11-21 bCarryControlOutarm -> bCarryControlOutarm1 視為兩個共用一個flag
    {
        return;
    }

    switch(Task)
    {
        case 1:
            bSortingAllBinTrayFinish=false;                                     //JerryYang 20150910 是否所有的Bin Tray IC都跑過完成
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

            if(FRCarryKit.UseSiteHasIC() && OutSHT1InRT())
            {
                OldPos=1;
            }
            else if(BRCarryKit.UseSiteHasIC() && OutSHT2InRT())
            {
                OldPos=0;
            }

            if(TestIF.iShuttleMode==1)
            {
                if(TestIF.iShuttle_Sel==0)
                    OldPos=1;                                                   // shuttle 1
                else
                    OldPos=0;                                                   // shuttle 2
            }

            if(CheckRotateOutNotFinish())                                       //kevin 20130524 這抓alarm inarm到等待點
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
            if(MoveOutArmXY_ToFix_Tray_Full())                                  //Steven 20230602 : 統一out arm 等待位置
            {
                Task=CheckOutArmCleanOut();
            }

            if(Task==5000)                                                      //Sam 20250204 : 修正 P27 吸嘴功能 Hangeup
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
                        if(FRCarryKit.LeftSideNoIC(2)==false)
                            iOutArmiWhichKit=0;
                        else
                            iOutArmiWhichKit=1;
                        InitPickFromShuttle1Task();                                                                     //Sam 20230208 : 修正 Fix AOI 的兩個問題
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
                if(FRCarryKit.LeftSideNoIC(OutArmSuck.iShtKitStep)==false)
                    iOutArmiWhichKit=0;
                else
                    iOutArmiWhichKit=1;
                InitPickFromShuttle1Task();                                                                             //Sam 20230208 : 修正 Fix AOI 的兩個問題
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

            if(MoveOutArmToShuttleIncludeZ_9045_1x4_2(iWhichShuttle, iOutArmiWhichKit, false))
            {
                IsCatchTrayReadySupplyNewTray();
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

            if(WhichAutoNeedTray()!=0)                                                                                  //JerryYang 20170731 (wei) Out arm要等料才不會hang up
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
                    MoveOutArmXY_ToFix_Tray_Full();                                                                         //Steven 20231027 : 先檢查有沒有要放的Tray
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

                InitPickFromShuttle1Task();                                                                             //Sam 20230208 : 修正 Fix AOI 的兩個問題

                if(FRCarryKit.LeftSideNoIC(OutArmSuck.iShtKitStep))
                {
                    iOutArmiWhichKit=1;
                    Task=1200;
                }
                else
                {
                    iOutArmiWhichKit=0;
                    Task=1200;
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
//                if(WhichAutoNeedTray()==0)                                      //JerryYang 20170731 (wei) Out arm要等料才不會hang up
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

            if(DoPickFromShuttle_9045_1x4_2(0))
            {
                PreSetOutAdditionalFlag();                                                                              //Steven 20220614 : Fixed for out arm additional function
                FRCarryKit.SetUnuseAndHasNullICToNullIC();
                if((iOutArmiWhichKit==0 && FRCarryKit.LeftSideNoIC(OutArmSuck.iShtKitStep)==false) ||
                   (iOutArmiWhichKit==1 && FRCarryKit.RightSideNoIC(OutArmSuck.iShtKitStep)==false))
                {
                    InitPickFromShuttle1Task();
                    return;
                }

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
                        if(BRCarryKit.LeftSideNoIC(OutArmSuck.iShtKitStep)==false)
                            iOutArmiWhichKit=0;
                        else
                            iOutArmiWhichKit=1;
                        InitPickFromShuttle2Task();                                                                     //Sam 20230208 : 修正 Fix AOI 的兩個問題
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
                if(BRCarryKit.LeftSideNoIC(OutArmSuck.iShtKitStep)==false)
                    iOutArmiWhichKit=0;
                else
                    iOutArmiWhichKit=1;
                InitPickFromShuttle2Task();                                                                             //Sam 20230208 : 修正 Fix AOI 的兩個問題
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

            if(MoveOutArmToShuttleIncludeZ_9045_1x4_2(iWhichShuttle, iOutArmiWhichKit))
            {
                IsCatchTrayReadySupplyNewTray();

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

                if(BRCarryKit.LeftSideNoIC(OutArmSuck.iPickKitStep)==false)
                    iOutArmiWhichKit=0;
                else
                    iOutArmiWhichKit=1;
                InitPickFromShuttle2Task();                                                                             //Sam 20230208 : 修正 Fix AOI 的兩個問題
                Task=2200;
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

            if(DoPickFromShuttle_9045_1x4_2(1))
            {
                PreSetOutAdditionalFlag();                                                                              //Steven 20220614 : Fixed for out arm additional function
                BRCarryKit.SetUnuseAndHasNullICToNullIC();
                if((iOutArmiWhichKit==0 && BRCarryKit.LeftSideNoIC(OutArmSuck.iShtKitStep)==false) ||
                   (iOutArmiWhichKit==1 && BRCarryKit.RightSideNoIC(OutArmSuck.iShtKitStep)==false))
                {
                    InitPickFromShuttle2Task();
                    return;
                }

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
                    AseIcRecord="@e02001";                                                                              //kevin 20161228 (jou) 傳送給 ASE 的頭資料
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
                if(AUTO3_IS_MAGAZINE==1             &&                                                                  //JerryYang 20221215 : Magazine跟fix區當buffer使功能
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

            if(iCatchTrayControlManual>=2 || WhichAutoNeedTray()!=0)                                                    //Sam 20180822 : OutArm 是否在安全位置，是否能放，不然會與 TrayArm 搶 Tray 動作卡住 Hange up。
            {
                MoveOutArmXY_ToFix_Tray_Full();
                break;
            }
            Task=3100;
            break;
        case 3020:                                                                                                      //jou 2010-01-25 start : one cycle後，可以將tray過載拉出來
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
            Task=3300;                                                                                                  //kevin 20151203 沒有fixtray的ic
        case 3300:
            if(AUTO3_IS_MAGAZINE==1             &&                                                                      //JerryYang 20221215 : Magazine跟fix區當buffer使功能
               TestIF_File.iMagFixTrayType==1   &&
               iWhichAuto>=iMagMin              &&
               iWhichBuff>=iFixMin              &&
               iWhichBuff<=iFixMax)
            {
                ct=iAutoIndex[iWhichBuff];
                if(iWhichBuff>=iFixMin &&
                   Sen[SnFixedTrayDetect[ct]].IsOff() &&
                   LastSet.iRealDummy!=DUMMY)                                                                           //wei 20151217 add iWhichAuto>=3避免沒要放FIX盤也偵測說沒有FIX盤
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
                   LastSet.iRealDummy!=DUMMY)                                                                           //wei 20151217 add iWhichAuto>=3避免沒要放FIX盤也偵測說沒有FIX盤
                {
                    Task=4000;
                    break;
                }
            }
            Task=3301;
            if(Task!=3301)                                                                                              //Steven 20180813 : add Out arm speed
                break;
        case 3301:
            if(DoOutArmPlaceToAuto_9045())                                                                              // 吸嘴可能還在下放
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
                if(Task==100)
                {
                    if(iWhichShuttle==0)
                    {
                        if(iOutArmiWhichKit==0 && FRCarryKit.RightSideNoIC(OutArmSuck.iShtKitStep)==false)
                        {
                            Task=1000;
                            return;
                        }
                    }
                    else
                    {
                        if(iOutArmiWhichKit==0 && BRCarryKit.RightSideNoIC(OutArmSuck.iShtKitStep)==false)
                        {
                            Task=2000;
                            return;
                        }
                    }
                }
            }
            break;
        case 4000:
            if(MoveOutArmXY_ToFix_Tray_Full(true))                                                                      //Ifor 20250926 add:Fix Tray 滿料時離開Out Arm
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
                AseIcRecord="@e02001";                                                                                  //kevin 20161228 (jou) 傳送給 ASE 的頭資料
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
            if(bMagCatchTrayfalg)                                                                                       //JerryYang 20221121 : AUTOI3 Magazine還在換TRAY不能放
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
