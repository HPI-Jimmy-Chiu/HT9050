// =============================================================================
//  aoutarm9045_1x2_4.cpp  --  Out-Arm per-site VARIANT (1x2_4)
//
//  Faithful translation of golden aoutarm9045_1x2_4.cpp (1395 lines, BCB6, Big5).
//  Translator: AI(W6.2c-1x2_4) 20260627
//  Translation wave: W6.2c (out-arm per-site-config variant: 1x2_4 -- one row,
//  2 pickers picking from a 4-column out-shuttle).  Sibling of the already-
//  translated out-arm engine aoutarm9045.cpp and the in-arm variant files.
//
//  ROLE: the per-site out-arm pick state machines for the 1x2_4 layout.  The
//  engine's dispatch ladders DoOutArm_9045() / DoPickFromShuttle_9045(int) route
//  iInArmType==e9045_1x2_4_Hot here (un-gated at the Integrate stage: the engine's
//  in-file `static` no-op stubs DoOutArm_9045_1x2_4()/DoPickFromShuttle_9045_1x2_4(int)
//  are removed and the externs un-gated).  This file owns:
//    GetNowShuttleMode_1x2_4, OutArmZNeedDown_1x2_4, CheckOutArmXYPitch_1x2_4,
//    MoveOutArmToShuttleIncludeZ_9045_1x2_4, DoPickFromShuttle_9045_1x2_4,
//    DoOutArm_9045_1x2_4.
//
//  FAITHFUL: every fn/var/cursor value, switch(Task) structure, formula,
//  fall-through (case 10->200, 1140->1150, 1100/1150 break-guard, 3010->3100,
//  3100->3300, 3301->3500, 5000->5100, ...) is transcribed VERBATIM from the
//  golden CP950.  Numeric semantics (int vs double, integer division/truncation,
//  the double dMovePitchX pitch math, /7.0 vs /3.0 vs /6*7 vs /2*3) kept EXACT.
//  The commented-out `#ifdef Carry4` blocks are reproduced VERBATIM as comments.
//
//  SOFT_SIMULTE is NOT defined -> the single `#ifdef SOFT_SIMULTE` block in
//  DoPickFromShuttle_9045_1x2_4 case iOUTARM_SUCK (golden :365-374, the
//  fMain->chkInPickLoadError->Checked drop-IC simulation) is compiled OUT; it is
//  reproduced verbatim so it re-enables identically under SOFT_SIMULTE.
//
//  VCL/Borland conversions: AnsiString (1-based, vclcompat); __fastcall/__property/
//  __published/PACKAGE NOT present in this file; __FUNC__ via aArmHeader shim.
//
//  Big5: every Chinese comment decoded via cp950 and preserved as UTF-8.
//  Final gate: ZERO U+FFFD.
// =============================================================================
// BCB6 ORIGINAL include block (mirrored as a comment for provenance):
//   #include "MachineDefine.h" ; #pragma hdrstop ; #include "aoutarm9045_1x2_4.h"
//   #include "aArmHeader.h" / "MyKitSuck.h" / "cprod.h" / "mymotor.h" / "mycylin.h"
//   #include "cSortCT.h" / "mysensor.h" / "main.h" / "note.h" / "csystem.h"
//   #include "acatchtray.h" / "cinitial.h" / "atester.h" / "aRotateKIT.h" / "fRotate.h"
//   #include "cOffSet.h" / "fAOI.h" / "cmydef.h" / "ProductionInfo.h" / "FixAICCD.h"
//   #include "Magazine.h" ; #pragma package(smart_init)
// =============================================================================
#include "MachineDefine.h"
#include "MachineType.h"            // iInArmType enum (e9045_1x2_2_14 / e9045_1x2_4_Hot), OutArm arm-index, eBulkBox, CC_KYEC_LEE, IC consts

#include "aoutarm9045_1x2_4.h"
#include "aArmHeader.h"             // __FUNC__ shim, RecordProcess, ShowErrorMessage, etc.
#include "aoutarm9045.h"            // engine: GetOutArmPitchX/Y_9045 / DoOutArmSuckPreOn / IsCheckOutArmDestroyActiveFinish / GetOutShuttleStatus_9045 / DoOutArmPlaceToAuto_9045 / DoOutArmAfterPlaceToAuto / SearchTrayToPlace_9045 / VerifyFixTrayLink / DoOutArmAdditionalFunction / CheekNeedToDoOutArmAdditionalFunction / PreSetOutAdditionalFlag / OutArmNeedCheckOffset / InitialOutArmNeedSuck / MoveOutArmXY_To_ESDSafePos / iOUTARM_SUCK / ...
#include "cprod.h"                 // Prod / TestIF / TestIF_File / ArmSpeed / LastSet
#include "Motor/mymotor.h"         // MOT[], TMyMotor, TTrayMotor, OutArmContinuousMove_9045
#include "mycylin.h"
#include "mysensor.h"              // Sen[]
#include "csystem.h"               // OutSHT1InRT / OutSHT2InRT / WhichAutoNeedTray / AutoTeachLoadTrayZ / IsOutArmCleanOutFinish / AutoTrayReCheck / HasICUnderMachine / HasICUnderHotPlate / CheckOutArmSuckICFallDown
#include "cmydef.h"                // global scalar universe + IC consts + iInArmType + enums + Auto/Mag/Tray index globals + K_RETRY/K_SKIP/K_HOME
#include "cpublic.h"               // CosFunction
#include "common.h"
#include "Config.h"                // IniConfig
#include "atester.h"               // TestIF.dSiteXPitch / iShuttleMode / iShuttle_Sel / bNS7000kit
#include "aHotPlateSubstrate.h"    // OutArmSuck / FRCarryKit / BRCarryKit (TMyKitSuck/TMySucker) + cursors
#include "FormsFacade.h"           // fMain / TrayForm offline stand-ins
#include "acarry_shims.h"          // MoveOutArmToAutoSafe / MoveOutArmXY_ToShuttleAlarmArea
#include "acatchtray_shims.h"      // InitProcessSingleMotorTask
#include "canary_support.h"        // LastSet / ShowErrorMessage / ShowMyMessage / RecordProcess / WhichAutoNeedTray / K_RETRY/K_SKIP/K_HOME / HTimer

// -----------------------------------------------------------------------------
//  SUBSTRATE EXTERNS not yet provided by a translated header (REPORTED for the
//  serial Integrate phase).  Forward-declared here behind #ifndef guards so this
//  TU compiles standalone and CANNOT ODR-collide once the owning module lands.
//  Signatures are VERBATIM from golden.  (Mirrors the committed sibling
//  aoutarm9045_1x2_2.cpp forward-decl block -- same out-arm family substrate set.)
//
//   engine vars  (golden aoutarm9045.h:8/9):
//     * bOutSuckShtDupErr[MAX_ARM_Row][MAX_ARM_Col]    -- defined in aoutarm9045.cpp
//     * iOutShtRetryCount                              -- defined in aoutarm9045.cpp
//   engine geometry funcs (golden aoutarm9045.h:23/26/27/28):
//     * GetVariableYOutShuttleData / GetOutArmToShuttleOffset_9045 /
//       GetOutArmXToShuttleOffset_9045 / GetOutArmYToShuttleOffset_9045
//   out-arm shuttle-pick engine funcs (golden aoutarm.h / aoutarm9045.cpp):
//     * SetOutArmNeedDestory / TransferOutShuttleRatio / SwapShuttleDataToOutArm /
//       ShowOutputShuttleDataMiss / ErrorBinBoxDetect / OutArmPickShuttleAlarm /
//       PorcessJAM0201OutArmPickUpErrorSkip / SetOutArmHome / PickFromShuttle /
//       bOutShtwaitPick / CheckTesterZ / OutArmAddSpeed / OutArmSubSpeed /
//       OutArmAddSpeedDisplay / OutArmSubSpeedDisplay / MoveOutArmXY_ToShuttleAlarmArea
//   out-arm engine SM globals (golden aoutarm.h):
//     * OutArmTask / iPickFromShuttle1Task / iPickFromShuttle2Task /
//       bOutArmManualStepPress / iWhichAuto / iOutRotateFinish / iOutArmZTeachTask /
//       bCarryControlOutarm1 / bCarryControlOutarm2 / ptrOutSHT
//   cross-module catchtray/magazine/AOI/tray surface (golden acatchtray.h /
//   Magazine.h / main.h / fAOI.h / csystem.h):
//     * IsCatchTrayReadySupplyNewTray / CheckRotateOutNotFinish /
//       MoveOutArmXY_ToFix_Tray_Full / CheckOutArmCleanOut / CheckOutArmInitState /
//       CheckOutArmToTask50 / CheckShuttleICPos / InitPickFromShuttle1Task /
//       InitPickFromShuttle2Task / SetFixTrayFullIC / DoFixTrayFullAlarm /
//       VerifyTrayStatus / CheckPlaceToMagazineTray / DoPickFromMagazineBuffer /
//       SearchTrayToPlace_Magazine / HasICUnderMachine / HasICUnderHotPlate /
//       bCheckYPitchRunHomeSen / bCheckYPitchHome / InitPlaceToAutoTask
//   substrate METHOD (CANNOT forward-declare -- Integrate must add to substrate):
//     * TMySucker::GetNeedSuckStatus()  -- golden mykitsuck.h:123 {return bNeedSuck;}
//       a const accessor returning the per-nozzle need-suck flag; add as inline to
//       aHotPlateSubstrate.h TMySucker.  (Syntax-only flags this on the substrate
//       class until added -- expected, REPORTED; same residual as sibling 1x2_2.)
//   substrate TIMER (CANNOT forward-declare standalone -- Integrate adds to substrate):
//     * hOutArmYpitchHomeTimer (golden aoutarm.h:213, HTimer).  Offline TQPF_Timer
//       (same SetSecAndOn()/Off() surface as the in-arm twin hInArmYpitchHomeTimer).
//       Add to aHotPlateSubstrate.h.  (HTimer type not provided offline -> declared
//       extern HTimer like sibling; REPORTED.)
//   substrate FORM (golden FixAICCD.h:152 / :130):
//     * fFixAICCD->OutArmCycleCounterUpdate()  -- single-method stand-in below.
// -----------------------------------------------------------------------------
class TMyKitSuck;  // for ptrOutSHT forward-decl

#ifndef AOUTARM9045_OUTSHTDUPERR_FWD
#define AOUTARM9045_OUTSHTDUPERR_FWD
extern bool bOutSuckShtDupErr[MAX_ARM_Row][MAX_ARM_Col];                        //Steven 20110216 : 出料Alarm
extern int  iOutShtRetryCount;
#endif
#ifndef AOUTARM9045_OUTGEOM_FWD
#define AOUTARM9045_OUTGEOM_FWD
int  GetVariableYOutShuttleData();                                              //ChungHung 20131231 alter AutoYPitch
int  GetOutArmToShuttleOffset_9045(int iSht, int iModeRow, int iModeCol, bool bPitch=true);
int  GetOutArmXToShuttleOffset_9045(int iSht, int iOffsetPos);                  //Steven 20240309 : fixed for out arm to sht offset
int  GetOutArmYToShuttleOffset_9045(int iSht, int iOffsetPos);
#endif
#ifndef AOUTARM9045_OUTPICK_FWD
#define AOUTARM9045_OUTPICK_FWD
void SetOutArmNeedDestory(int iSuckRow, int iSuckCol, int iTargetRow, int iTargetCol);
void TransferOutShuttleRatio(int iSht, int *iXPos, int *iYPos, int iRow, int iCol);
bool SwapShuttleDataToOutArm(int iSht, int iShtRow, int iShtCol, int iSuckRow, int iSuckCol, int *iWitchErrBin);  //Steven 20170428 : 統一Out Arm從Shuttle吸料後的資料交換
void ShowOutputShuttleDataMiss();
bool ErrorBinBoxDetect(int iWitchErrBin);                                       //kevin 20160906 add error bin box
bool MoveOutArmXY_ToShuttleAlarmArea();
int  OutArmPickShuttleAlarm(int iSht, bool bHasDuplicateErr, AnsiString ErrPart);
void PorcessJAM0201OutArmPickUpErrorSkip(int iSht, int iSuckRow, int iSuckCol, int iTargetRow, int iTargetCol);  //Steven 20161214 : 加上OutArm吸取異常Skip的ErrorLog
void SetOutArmHome();
void PickFromShuttle(int iwhichKit=0);                                          //kevin 20210612 add int iwhichKit=0
bool bOutShtwaitPick();                                                         //kevin 20210602 Out arm sht on wait
bool CheckTesterZ(int iSht);
void OutArmAddSpeed();                                                          //Steven 20110525 : Auto Speed
void OutArmSubSpeed();
void OutArmAddSpeedDisplay();                                                   //KaiChen 20171225 (Steven)：Add Speed Display
void OutArmSubSpeedDisplay();
#endif
#ifndef AOUTARM9045_OUTSM_GLOBALS_FWD
#define AOUTARM9045_OUTSM_GLOBALS_FWD
extern int  OutArmTask;
extern int  iPickFromShuttle1Task;
extern int  iPickFromShuttle2Task;
extern bool bOutArmManualStepPress;
extern int  iWhichAuto;
extern int  iOutRotateFinish;                                                  //kevin 20130524 目前找到 auto tray
extern int  iOutArmZTeachTask;
extern bool bCarryControlOutarm1;                                              //jou 2011-11-21 bCarryControlOutarm -> bCarryControlOutarm1 為防止個別一支flag
extern bool bCarryControlOutarm2;
extern TMyKitSuck *ptrOutSHT;
#endif
#ifndef AOUTARM9045_XMODULE_FWD
#define AOUTARM9045_XMODULE_FWD
bool IsCatchTrayReadySupplyNewTray();                                          // AI(W906-GateA-0) 20260804: was `void` -- golden aoutarm.h:64 is `bool`; MinGW mangling hides return type so it linked, MSVC LNK2019'd (same class as W7-A2)
bool CheckRotateOutNotFinish();                                                //kevin 20130524 發出alarm inarm到等待點
bool MoveOutArmXY_ToFix_Tray_Full(bool bOpenOutArm=false);                      //Ifor 20250926 add:Fix Tray 全滿時移開Out Arm
int  CheckOutArmCleanOut();
bool CheckOutArmInitState();
bool CheckOutArmToTask50(int iWhichShuttle);
bool CheckShuttleICPos(int iSht);
void InitPickFromShuttle1Task();
void InitPickFromShuttle2Task();
void SetFixTrayFullIC();                                                       // AI(W906-GateA-0) 20260804: was `bool` -- golden aoutarm.h:73 is `void`; see IsCatchTrayReadySupplyNewTray note above
bool DoFixTrayFullAlarm();                                                      //Steven 20181113 : 修正Fix Link Alarm的問題
int  VerifyTrayStatus();
bool CheckPlaceToMagazineTray(int iWhichMag);                                   //JerryYang 20220909 : add magazine
bool DoPickFromMagazineBuffer();
int  SearchTrayToPlace_Magazine();                                              //Eastsun 20260515 F021: Magazine search hang-up fix -- AI(W906-W7-A2) 20260728: was void (ODR split vs. golden aoutarm9045.h:63 int; see aoutarm_shims.cpp)
bool HasICUnderMachine();
bool HasICUnderHotPlate();
#endif
#ifndef AOUTARM9045_YPITCH_HOME_FWD
#define AOUTARM9045_YPITCH_HOME_FWD
extern TQPF_Timer hOutArmYpitchHomeTimer;                                      //kevin 20180822 (Steven) : Ypitch home -- AI(ht9045-v899) 20260627: HTimer->TQPF_Timer to match single substrate def (same Off()/SetSecAndOn())
bool bCheckYPitchRunHomeSen(int iXYHome);                                       //kevin 20180822 (Steven) : y pitch 遮住home
bool bCheckYPitchHome(int iXYHome);                                             //kevin 20180822 (Steven) : y pitch 歸home 動作
void InitProcessSingleMotorTask(int iMot);
bool AutoTeachLoadTrayZ(bool bInit, int iArm, int &iTask);                      //kevin 20171103 (wei) Outarm Z Auto High
void InitPlaceToAutoTask();
#endif
// -----------------------------------------------------------------------------
//  Fix-AI-CCD form stand-in (golden FixAICCD.h:152 extern PACKAGE TfFixAICCD
//  *fFixAICCD; method OutArmCycleCounterUpdate()).  FixAICCD.cpp is out of scope;
//  forward-declared behind a guard with the single method this TU calls so the
//  二次解料 AI CCD branches compile.  Offline behavior: the real form/pointer is
//  supplied by FormsFacade/FixAICCD once translated.  REPORTED.
// -----------------------------------------------------------------------------
// AI(W906-W7-L1-Wave0) 20260801: TU-local forward declaration RETIRED -- the real
// stand-in now lives at forms/fFixAICCD.h (reached via this TU's existing
// FormsFacade.h include) and its global is a REAL object, not the null pointer
// aoutarm_shims.cpp:112 used to define.  Retiring it here is mandatory, not
// cosmetic: keeping a second `class TfFixAICCD` in a TU that also sees the facade
// header is a redefinition error.

//==============================================================================
//Shuttle 1放B面, Shuttle 2放A面
//  -- golden aoutarm9045_1x2_4.cpp:32
//==============================================================================
int GetNowShuttleMode_1x2_4(int iSht)
{
    ptrOutSHT=(iSht==0)?&FRCarryKit:&BRCarryKit;

    if(OutArmSuck.iXStep==1)
    {
        return (iSht==0)?1:0;
    }
    else if(ptrOutSHT->UseSiteNoIC())
    {
        return (iSht==0)?10001:10000;
    }
    else
    {
        if(ptrOutSHT->Item[0][0]>=HAS_IC)
            return (iSht==0)?10001:10000;                                       //左上
        else                                                                    //if(ptrOutSHT->Item[0][1]>=HAS_IC)
            return (iSht==0)?11001:11000;                                       //右上
    }
}
//==============================================================================
//  -- golden aoutarm9045_1x2_4.cpp:53
//==============================================================================
bool OutArmZNeedDown_1x2_4(int iSht)
{
    if(InitialOutArmNeedSuck(iSht)==false)
        return false;

    ptrOutSHT=(iSht==0)?&FRCarryKit:&BRCarryKit;
    int iMode=GetNowShuttleMode_1x2_4(iSht);
//    int iModeRow=iMode%100;
    int iModeCol=iMode/100;

    if(iModeCol==0)                                                             //左右都起
    {
        if(iSht==0)
        {
            SetOutArmNeedDestory(1, 0, 0, 0);
            SetOutArmNeedDestory(1, 1, 0, 1);
        }
        else
        {
            SetOutArmNeedDestory(0, 0, 0, 0);
            SetOutArmNeedDestory(0, 1, 0, 1);
        }
    }
    else if(iModeCol==100)
    {
        if(iSht==0)
        {
            SetOutArmNeedDestory(1, 0, 0, 0);
            SetOutArmNeedDestory(1, 1, 0, 1);
        }
        else
        {
            SetOutArmNeedDestory(0, 0, 0, 0);
            SetOutArmNeedDestory(0, 1, 0, 1);
        }
    }
    else if(iModeCol==110)
    {
        if(iSht==0)
        {
            SetOutArmNeedDestory(1, 0, 0, 0);
            SetOutArmNeedDestory(1, 1, 0, 1);
        }
        else
        {
            SetOutArmNeedDestory(0, 0, 0, 0);
            SetOutArmNeedDestory(0, 1, 0, 1);
        }
    }
    return true;
}
//------------------------------------------------------------------------------
//  -- golden aoutarm9045_1x2_4.cpp:105
//------------------------------------------------------------------------------
bool CheckOutArmXYPitch_1x2_4(int *iX, int *iY, int iSht, int iMovePitchX, int iMovePitchY)
{
    int iMode=GetNowShuttleMode_1x2_4(iSht);
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

    if(USE_OUT_ARM_Y_PITCH==iXYPitch16Bd_Be)                                    //放料為第六種子尺  //Ztex 2023.12.06 Add HT-1032  //JerryYang 20251218 : IN/OUT ARM支援不同模組
    {
    }
    else if(USE_PICKER_COUNT==ep16Picker)                                       //放料為第四種子尺
    {
        dMovePitchX=double(iMovePitchX)/7.0;
        if(iModeCol==0)
        {
            if(iInArmType==e9045_1x2_2_14)                                      //Aa --> Aa
                *iX=*iX+(dMovePitchX*3.0-TestIF.dSiteXPitch*0.5);
            else                                                                //Aa --> Aa
                *iX=*iX+(dMovePitchX*3.0-TestIF.dSiteXPitch*0.5);
        }
        else if(iModeCol==100)
        {
            if(iInArmType==e9045_1x2_2_14)                                      //Aa --> Aa
                *iX=*iX+(dMovePitchX*3.0-TestIF.dSiteXPitch*0.5);
            else                                                                //Aa --> Aa
                *iX=*iX+(dMovePitchX*3.0-TestIF.dSiteXPitch*0.5);
        }
        else if(iModeCol==110)
        {
            if(iInArmType==e9045_1x2_2_14)                                      //Ag --> Ab
                *iX=*iX-(dMovePitchX*3.0+TestIF.dSiteXPitch*0.5);
            else                                                                //Ae --> Ab
                *iX=*iX-(dMovePitchX*1.0+TestIF.dSiteXPitch*0.5);
        }
    }
    else if(USE_OUT_ARM_Y_PITCH==iXYPitchVariable)                              //放料為第二種子尺  //JerryYang 20251218 : IN/OUT ARM支援不同模組
    {
        dMovePitchX=double(iMovePitchX)/3.0;
        if(iModeCol==0)
        {                                                                       //Aa --> Aa
            *iX=*iX+(dMovePitchX*1.0-TestIF.dSiteXPitch*0.5);
        }
        else if(iModeCol==100)                                                  //Aa --> Aa
        {
            *iX=*iX+(dMovePitchX*1.0-TestIF.dSiteXPitch*0.5);
        }
        else if(iModeCol==110)                                                  //Ac --> Ab
        {
            *iX=*iX+(-dMovePitchX*1.0+TestIF.dSiteXPitch*0.5);
        }
    }
    else                                                                        //放料為第三種子尺
    {
        dMovePitchX=double(iMovePitchX)/3.0;
        if(iModeCol==0)                                                         //Aa --> Aa
        {
            *iX=*iX+(dMovePitchX*2.0-TestIF.dSiteXPitch*0.5);
        }
        else if(iModeCol==100)                                                  //Aa --> Aa
        {
            *iX=*iX+(dMovePitchX*2.0-TestIF.dSiteXPitch*0.5);
        }
        else if(iModeCol==110)                                                  //Ac --> Ab
        {
            *iX=*iX+(dMovePitchX*0.0+TestIF.dSiteXPitch*0.5);
        }
    }

    *iY=*iY+GetOutArmYToShuttleOffset_9045(iSht, iOffsetPos);
    *iX=*iX+GetOutArmXToShuttleOffset_9045(iSht, iOffsetPos);
    TransferOutShuttleRatio(iSht, iX, iY, iOutArmYBase, iOutArmXBase);
    if(OutArmZNeedDown_1x2_4(iSht)==false)
        return false;
    return true;
}
//==============================================================================
//  -- golden aoutarm9045_1x2_4.cpp:215
//==============================================================================
bool MoveOutArmToShuttleIncludeZ_9045_1x2_4(int iSht, bool bZDown)
{
    bool bZFlag[MAX_ARM_Row][MAX_ARM_Col];
    int iZPos[MAX_ARM_Row][MAX_ARM_Col];
    int iXVariable[X_PITCH_COUNT];
    int iXPos               =iOutArmShtXCenterPos;                              //Steven 20141029 : XY-Pitch for Shuttle Center position for base Suck
    int iYPos               =iOutArmShtYCenterPos;
    int iMode               =GetNowShuttleMode_1x2_4(iSht);
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
        if(OutArmSuck.iXStep==1)
            iMovePitchX=TestIF.dSiteXPitch/6*7;
        else
            iMovePitchX=iXpitchMaxX7;
    }
    else
    {
        if(OutArmSuck.iXStep==1)
            iMovePitchX=TestIF.dSiteXPitch/2*3;
        else
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

    if(CheckOutArmXYPitch_1x2_4(&iXPos, &iYPos, iSht, iMovePitchX, iMovePitchY)==false && bZDown==true)
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
// 取出工從 Shuttle 1 吸取 IC 流程動作
//  Ver : 2003_07
//  -- golden aoutarm9045_1x2_4.cpp:276
//==============================================================================
bool DoPickFromShuttle_9045_1x2_4(int iSht)
{
    int ret;
    int &Task=(iSht==0)?iPickFromShuttle1Task:iPickFromShuttle2Task;
    static int iWitchErrBin=0;
    AnsiString ErrPart="";                                                      //Steven 20110216 : 出料Alarm
    bool bHasDuplicateErr=false;                                                //Steven 20110216 : 出料Alarm
    bool bHasErr=false;                                                         //Steven 20110216 : 出料Alarm
    bool flag=false;
    int iSuckRow, iSuckCol, iShtRow, iShtCol;
    ptrOutSHT=(iSht==0)?&FRCarryKit:&BRCarryKit;

    if(CheckTesterZ(iSht))
    {
        //MoveOutArmToAutoSafe();                                               //Steven 20150408 : Mark,避免子尺上上下下
        return false;
    }

    switch(Task)
    {
        case 1:
            if(ArmSpeed[OutArm].dWaitOnSH!=0)                                   //kevin 20210525 out arm 起動在shuttle 上等待 時間
            {
                if(OutArmSuck.NoIC())                                           //kevin 20210615 第一次 吸取IC 需等候
                {
                    if(MoveOutArmToShuttleIncludeZ_9045_1x2_4(iSht, false))     //先到 Shuttle 上等待
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
            if(MoveOutArmToShuttleIncludeZ_9045_1x2_4(iSht, true))              //kevin 20210717 change function
            {
                if(OutArmNeedCheckOffset(false, iSht))                          //Steven 20230531 : 簡化判斷式
                {
                    Task=500;
                    break;
                }
                OutArmSuck.ResetAll();                                          //Steven 20160323 : 避免重打偵真空
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
                OutArmSuck.ResetAll();                                          //Steven 20160323 : 避免重打偵真空
            }
            break;
        case iOUTARM_SUCK:
            flag=true;
            bHasErr=false;
            for(int j=0; j<OutArmSuck.iPickCol; j++)
            {
                iSuckRow  =(iSht==0)?1:0;
                iSuckCol  =j;
                iShtRow   =0;
                iShtCol   =j;
                if(ptrOutSHT->Item[iShtRow][iShtCol] &&
                   OutArmSuck.Suck[iSuckRow][iSuckCol].GetNeedSuckStatus())
                {
                    if(OutArmSuck.Suck[iSuckRow][iSuckCol].Suck())
                    {
                        //掉落ic的偵測了
                        #ifdef SOFT_SIMULTE
                        if(fMain->chkInPickLoadError->Checked==true)
                        {
                            if(j==0)
                            {
                                OutArmSuck.Suck[0][0].Error=true;
                                continue;
                            }
                        }
                        #endif
                        //掉落ic的偵測了 end
                        bHasErr=SwapShuttleDataToOutArm(iSht, iShtRow, iShtCol, iSuckRow, iSuckCol, &iWitchErrBin);     //Steven 20170428 : 統一Out Arm從Shuttle吸料後的資料交換
                        bOutSuckShtDupErr[iSuckRow][iSuckCol]=false;
                    }
                    else if(OutArmSuck.Suck[iSuckRow][iSuckCol].Error==false)
                    {
                        flag=false;
                    }
                }
            }

            if(bHasErr)
            {
                ShowOutputShuttleDataMiss();
            }

            if(flag==false)                                                                                             //KEVIN 20130421 有RETRY資料尚未交換會將其他IC丟掉
                break;

            for(int j=0; j<OutArmSuck.iPickCol; j++)
            {
                iSuckRow=(iSht==0)?1:0;
                if(OutArmSuck.Suck[iSuckRow][j].Error)
                {
                    iOutShtRetryCount++;
                    Task=2000;
                    return false;
                }
            }

            for(int j=0; j<OutArmSuck.iPickCol; j++)
            {
                iSuckRow=(iSht==0)?1:0;
                if(OutArmSuck.Item[iSuckRow][j]==NULL_IC &&
                   OutArmSuck.Suck[iSuckRow][j].GetNeedSuckStatus())
                    return false;
            }

            if(iHWFix_BinBox==1 && ErrorBinBoxDetect(iWitchErrBin)==false)
                return false;                                                                                           //kevin 20160906 add error bin box

            iOutShtRetryCount=0;

            if(USE_Fix_AI_CCD==true && TestIF_File.bEnableFix2BGAAICCD==true)                                           //Sam 20231107 : 走 Fix AOI 模式
            {
                fFixAICCD->OutArmCycleCounterUpdate();
            }
            Task=1;
            return true;
        case 2000:
            if(MoveOutArmToAutoSafe())
            {
                bHasErr=false;
                ErrPart=" ";
                bHasDuplicateErr=false;                                                                                 //Steven 20110216 Start : 出料Alarm
                for(int j=0; j<OutArmSuck.iPickCol; j++)
                {
                    iSuckRow=(iSht==0)?1:0;
                    if(bOutSuckShtDupErr[iSuckRow][j])
                        bHasDuplicateErr=true;
                    if(OutArmSuck.Suck[iSuckRow][j].Error)
                    {
                        ErrPart+=OutArmSuck.Suck[iSuckRow][j].sName;
                        bHasErr=true;
                    }
                    else
                    {
                        bOutSuckShtDupErr[iSuckRow][j]=false;
                    }
                }

                if(bHasErr)
                {
                    if(iOutShtRetryCount>ArmSpeed[OutArm].iRetryCT)
                    {
                        if(!MoveOutArmXY_ToShuttleAlarmArea())
                            return false;

                        ret=OutArmPickShuttleAlarm(iSht, bHasDuplicateErr, ErrPart);

                        for(int j=0; j<OutArmSuck.iPickCol; j++)
                        {
                            iSuckRow=(iSht==0)?1:0;
                            if(OutArmSuck.Suck[iSuckRow][j].Error)
                                bOutSuckShtDupErr[iSuckRow][j]=true;
                            else
                                bOutSuckShtDupErr[iSuckRow][j]=false;
                        }
                    }
                    else
                    {
                        ret=K_RETRY;
                    }

                    if(ret==K_SKIP)
                    {
                        for(int j=0; j<OutArmSuck.iPickCol; j++)
                        {
                            iSuckRow=(iSht==0)?1:0;
                            if(OutArmSuck.Suck[iSuckRow][j].Error)
                            {
                                PorcessJAM0201OutArmPickUpErrorSkip(iSht, 0, j, iSuckRow, j);                           //Steven 20161214 : 加上OutArm吸料異常Skip的ErrorLog
                            }
                        }

                        if(USE_Fix_AI_CCD==true &&
                           TestIF_File.bEnableFix2BGAAICCD==true)                                                       //Sam 20231107 : 走 Fix AOI 模式
                        {
                            fFixAICCD->OutArmCycleCounterUpdate();
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
                            if(IniConfig.bInOutArmCanPushHome)                                                          //ChungHung HT9045 2011/12/13 //Input pickup device error時,按"retry"時,機台不會自動home
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
//  -- golden aoutarm9045_1x2_4.cpp:525
//==============================================================================
void DoOutArm_9045_1x2_4()
{
    int ret, ct, iMode1=0, iMode2=0;
    static int OldPos=1;
    static bool FirstEnter=true, bCheckSpeed=false, bOutArmNeedMoveToSht=false;  //JerryYang 20170828 add bOutArmNeedMoveToSht
    int &Task=OutArmTask;
    static int iWhichShuttle=0, iBackMode1=0, iBackMode2=0;
    bool flag3;
    static bool bCheckSpeed1=false;                                             //KaiChen 20171225 (Steven)：Add Speed Display

    static int iPickFinish=0x00;

    if(bCarryControlOutarm1 || bCarryControlOutarm2)                            //jou 2011-11-21 bCarryControlOutarm -> bCarryControlOutarm1 不能共用同一個flag
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

            if(TestIF.iShuttleMode==1)
            {
                if(TestIF.iShuttle_Sel==0)
                    OldPos=1;
                else
                    OldPos=0;
            }

            if(CheckRotateOutNotFinish())                                       //kevin 20130524 發完alarm inarm到等待點
            {
                Task=7000;
                break;
            }

            if(WhichAutoNeedTray()!=0 &&
               IniConfig.bA15_1ESDGiveWayFunction==true &&
               USE_BU5_Function==true)
            {
                Task=150;
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
            iPickFinish=0x00;
            OldPos=1-OldPos;
            break;
        case 150:
            if(MoveOutArmXY_To_ESDSafePos())                                    //==> Eastsun 20260521 抽出
            {
                if(WhichAutoNeedTray()==0)
                    Task=100;
            }
            break;
        case 200:
            if(MoveOutArmToAutoSafe())
                Task=300;
            break;
        case 300:
            if(MoveOutArmXY_ToFix_Tray_Full())                                  //Steven 20230602 : 統一out arm的位置
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

            if(MoveOutArmToShuttleIncludeZ_9045_1x2_4(iWhichShuttle, false))
            {
                IsCatchTrayReadySupplyNewTray();

                if(OutArmSuck.iXStep!=1)
                {
                    iBackMode1=GetNowShuttleMode_1x2_4(0);
                }

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

            if(WhichAutoNeedTray()!=0)                                                                                  //JerryYang 20170731 (wei) Out arm要拿料才不會hang up
            {
                MoveOutArmXY_ToFix_Tray_Full();                                                                         //Steven 20231027 : 先檢查有沒有要換Tray
                Task=1160;
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

                if(OutArmSuck.iXStep!=1)
                {
                    iMode1=GetNowShuttleMode_1x2_4(0);
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
//                if(WhichAutoNeedTray()==0)                                      //JerryYang 20170731 (wei) Out arm要拿料才不會hang up
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

            if(DoPickFromShuttle_9045_1x2_4(0))
            {
                PreSetOutAdditionalFlag();                                                                              //Steven 20220614 : Fixed for out arm additional function
                FRCarryKit.SetUnuseAndHasNullICToNullIC();
                if(FRCarryKit.UseSiteHasIC())
                {
                    InitPickFromShuttle1Task();                                                                         //Sam 20230208 : 修正 Fix AOI 的兩個問題
                    return;
                }

                OldPos=0;
                iPickFinish=iPickFinish|0x01;
                if(OutArmSuck.HasIC()==false)
                {
                    Task=50;
                }
                else
                {
                    iOutRotateFinish=0;                                                                                 //kevin 20130524 目前抓的 auto tray
//                    iOutArmPickPlaceCnt[OutOfsOutSh1]++;                      //JerryYang 20230523 : 沒在用, Mark掉  //JerryYang 20180921 Setup Teach功能
                    bPickSH1Flag=true;
                    bPickSH2Flag=false;
                    Task=5000;                                                                                          //ChangHung 2011/02/11
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

            if(MoveOutArmToShuttleIncludeZ_9045_1x2_4(iWhichShuttle, false))
            {
                IsCatchTrayReadySupplyNewTray();

                if(OutArmSuck.iXStep!=1)
                {
                    iBackMode2=GetNowShuttleMode_1x2_4(1);
                }
                Task=2100;
            }

            if(Task!=2100)                                                                                              //Steven 20180813 : add Out arm speed
                break;
        case 2100:
            IsCatchTrayReadySupplyNewTray();

            if(BRCarryKit.UseSiteHasIC() && OutSHT2InRT() && bCheckShuttle2Flag==false)
            {
//#ifdef Carry4
//                MOT[MOutShuttle2].fCanMoveR=false;
//#else
                MOT[MInShuttle2].fCanMoveR=false;
//#endif
                InitPickFromShuttle2Task();
                Task=2200;

                if(OutArmSuck.iXStep!=1)
                {
                    iMode2=GetNowShuttleMode_1x2_4(1);
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
            else if(FRCarryKit.UseSiteHasIC() && OutSHT1InRT() && bCheckShuttle1Flag==false)
            {
                Task=50;
                break;
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

            if(DoPickFromShuttle_9045_1x2_4(1))
            {
                PreSetOutAdditionalFlag();                                                                              //Steven 20220614 : Fixed for out arm additional function
                BRCarryKit.SetUnuseAndHasNullICToNullIC();
                if(BRCarryKit.UseSiteHasIC())
                {
                    InitPickFromShuttle2Task();                                                                         //Sam 20230208 : 修正 Fix AOI 的兩個問題
                    return;
                }

                OldPos=1;
                iPickFinish=iPickFinish|0x02;
                if(OutArmSuck.HasIC()==false)
                {
                    Task=50;
                }
                else
                {
                    iOutRotateFinish=0;                                                                                 //kevin 20130524 目前抓的 auto tray
//                    iOutArmPickPlaceCnt[OutOfsOutSh2]++;                      //JerryYang 20230523 : 沒在用, Mark掉  //JerryYang 20180921 Setup Teach功能
                    bPickSH1Flag=false;
                    bPickSH2Flag=true;
                    Task=5000;
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
                    AseIcRecord="@e02001";                                                                              //kevin 20161228 (jou) 傳送給 ASE 的頭尾資料
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
            iOutRotateFinish=3;                                                                                         //kevin 20130524 目前抓的 auto tray
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

            if(iCatchTrayControlManual>=2 || WhichAutoNeedTray()!=0)                                                    //Sam 20180822 : OutArm 是否在安全位置，是否能不再讓他會與 TrayArm 的 Tray 動作有卡 Hange up。
            {
                MoveOutArmXY_ToFix_Tray_Full();
                break;
            }
            Task=3100;
            break;
        case 3020:                                                                                                      //jou 2010-01-25 start : one cycle後再可以將tray擼出來
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
            if(AUTO3_IS_MAGAZINE==1             &&                                                                      //JerryYang 20221215 : Magazine跟fix區當buffer使功能
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
            if(DoOutArmPlaceToAuto_9045())                                                                              // 子尺進度還在下降
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
            if(MoveOutArmXY_ToFix_Tray_Full(true))                                                                      //Ifor 20250926 add:Fix Tray 偵測時移開Out Arm
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
                AseIcRecord="@e02001";                                                                                  //kevin 20161228 (jou) 傳送給 ASE 的頭尾資料
                Task=3010;
            }
            break;
        case 5000:
            if(MoveOutArmToAutoSafe())
            {
                CheckOutArmSuckICFallDown();
                MOT[MInShuttle1].fCanMoveR=true;
                MOT[MInShuttle2].fCanMoveR=true;
                Task=5100;
            }
            else
            {
                break;
            }
        case 5100:                                                                                                      //判斷另一個Shuttle是否有IC
            if(iPickFinish==0x01)
            {
                if(BRCarryKit.UseSiteHasIC() && OutSHT2InRT() && bCheckShuttle2Flag==false)                             // change another shuttle
                {
                    iWhichShuttle=1;
                    Task=2000;
                }
                else
                {
                    iOutRotateFinish=0;                                                                                 //kevin 20130524 準備抓料ROTATE   //JerryYang 20161007 修正Out Sht1 放Device沒有重抓rotate
                    Task=3000;
                }
            }
            else if(iPickFinish==0x02)
            {
                if(FRCarryKit.UseSiteHasIC() && OutSHT1InRT() && bCheckShuttle1Flag==false)
                {
                    iWhichShuttle=0;
                    Task=1000;
                }
                else
                {
                    iOutRotateFinish=0;                                                                                 //kevin 20130524 準備抓料ROTATE
                    Task=3000;
                    break;
                }
            }
            else if(iPickFinish==0x00)
            {
                Task=50;
            }
            else
            {
                iOutRotateFinish=0;                                                                                     //kevin 20130524 準備抓料ROTATE
                Task=3000;
                break;
            }
            break;
        case 10000:
            if(CheckPlaceToMagazineTray(iWhichMag))                                                                     //JerryYang 20220909 : add magazine
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
            if(bMagCatchTrayfalg)                                                                                       //JerryYang 20221121 : AUTOI3 Magazine還在換TRAY不出料
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
