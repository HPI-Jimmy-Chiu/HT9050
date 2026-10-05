// =============================================================================
//  aoutarm9045_All_1Picker.cpp  --  Out-Arm per-config VARIANT (All_1Picker, single-picker)
//
//  Faithful translation of golden aoutarm9045_All_1Picker.cpp (BCB6, Big5/cp950).
//  Translator: AI(W6.2c-All_1Picker) 20260627
//  Translation wave: W6.2c-OUT (out-arm per-config variant: All_1Picker).
//
//  ROLE: the per-config out-arm state machine for the All_1Picker (single
//  picker) layout.  This file owns (golden names VERBATIM):
//    GetNowShuttleRowCol_All_1Picker, OutArmZNeedDown_All_1Picker,
//    CheckOutArmXYPitch_All_1Picker, MoveOutArmToShuttleIncludeZ_9045_All_1Pick,
//    DoPickFromShuttle_9045_All_1Pick, DoOutArm_9045_All_1Picker,
//    CaculateOutShtXYPos.
//
//  ENGINE DISPATCH (LIVE variant, CARVE-OUT = DoOutArm-ONLY):
//    The out-arm engine aoutarm9045.cpp dispatches the FIRST ladder branch
//    `if(USE_PICKER_COUNT==ep1Picker) DoOutArm_9045_All_1Picker();` -- this is
//    PICKER-GATED, NOT an iInArmType enum.  There is NO
//    DoPickFromShuttle_9045_All_1Picker entry in the engine
//    DoPickFromShuttle_9045() ladder.  Accordingly, only
//    DoOutArm_9045_All_1Picker() is exported (see the .h); the file's pick
//    helper is DoPickFromShuttle_9045_All_1Pick(int) (note `_1Pick`, NOT
//    `_1Picker`) and is FILE-LOCAL (called only inside
//    DoOutArm_9045_All_1Picker).  Do NOT confuse the two names.
//
//  FAITHFUL: every fn/var/cursor value, switch(Task) structure, formula, branch,
//  fall-through and commented-out code is transcribed VERBATIM from the golden
//  CP950 source.  The DoOutArm fall-throughs (case 1->5, 5->10, 10->50,
//  50->100, 1140->1150, 1150->1100, 2040->2050, 2050->2100, 3100->3300,
//  3300->3310, 3310->3500) and the DoPickFromShuttle fall-throughs
//  (case 2->10, 10->200) are preserved EXACTLY.
//
//  VCL/Borland conversions: VCL AnsiString (1-based) via vclcompat; this file
//  has no __fastcall/__property/__published/PACKAGE.  SOFT_SIMULTE is NOT
//  defined -> the one #ifdef SOFT_SIMULTE block in DoPickFromShuttle (case
//  iOUTARM_SUCK, chkInPickLoadError test) is compiled OUT, kept VERBATIM.
//
//  W7-gated offline bodies (kept VERBATIM behind `#if 0 // TODO(W7)` with an
//  active control-flow-preserving fallback, mirroring the engine
//  aoutarm9045.cpp pattern):
//    * fAutoTeach->iATOutArmWhichKit / fAutoTeach->IsRun()/DoNext() in
//      DoPickFromShuttle case 200 / case 500 (TfAutoTeach VCL form, offline N/A).
//
//  Big5: every Chinese comment decoded via cp950 and preserved as UTF-8.
//  Final gate: ZERO U+FFFD.
//
//  INCLUDE REWRITE (mirror of the golden header set, translated subset):
//    golden: MachineDefine.h ; aoutarm9045_All_1Picker.h ; aArmHeader.h ;
//            MyKitSuck.h ; cprod.h ; mymotor.h ; mycylin.h ; cSortCT.h ;
//            mysensor.h ; main.h ; note.h ; csystem.h ; acatchtray.h ;
//            cinitial.h ; atester.h ; aRotateKIT.h ; fRotate.h ; cOffSet.h ;
//            fAOI.h ; cmydef.h ; ProductionInfo.h ; AutoTeach.h ; FixAICCD.h ;
//            Magazine.h ; aRotateKIT_Out.h ; fRotate.h ; #pragma package(smart_init)
// =============================================================================
#include "MachineDefine.h"                  // <windows.h> (ZeroMemory), vclcompat
#include "aoutarm9045_All_1Picker.h"

#include "aArmHeader.h"             // RecordProcess, ShowErrorMessage
#include "aoutarm9045.h"           // engine: GetOutArmPitchX/Y_9045 / GetOutShuttleStatus_9045 / IsCheckOutArmDestroyActiveFinish / InitialOutArmNeedSuck / OutArmNeedCheckOffset / iOUTARM_SUCK / DoOutArmPlaceToAuto_9045 / DoOutArmAfterPlaceToAuto / DoOutArmAdditionalFunction / CheekNeedToDoOutArmAdditionalFunction / SearchTrayToPlace_9045 / VerifyFixTrayLink / PreSetOutAdditionalFlag / MoveOutArmXY_To_ESDSafePos
#include "ainarm9045.h"            // GetShtRowColPos / GetShtRowColStartPos / bCheckYPitchRunHomeSen / bCheckYPitchHome
#include "Motor/mymotor.h"         // MOT[], TMyMotor, TTrayMotor, OutArmContinuousMove_9045
#include "mycylin.h"
#include "mysensor.h"              // Sen[] / SnFixedTrayDetect
#include "cprod.h"                 // Prod / TestIF / TestIF_File / ArmSpeed / LastSet
#include "cpublic.h"               // CosFunction
#include "cmydef.h"                // global scalar universe + IC consts + iInArmType + iOutArmX/YBase + iXpitchMaxX3 + iRotate_Type + iOutArmiWhichKit + AseIcRecord + bEnterOffset
#include "csystem.h"               // OutSHT1InRT / OutSHT2InRT / HasICUnderMachine / HasICUnderHotPlate / CheckOutArmSuckICFallDown / AutoTrayReCheck / IsOutArmCleanOutFinish
#include "acatchtray.h"            // WhichAutoNeedTray
#include "aHotPlateSubstrate.h"    // OutArmSuck/FRCarryKit/BRCarryKit + tRotate + DoOutArmPlaceToAuto_9045 / DoOutArmAfterPlaceToAuto / DoOutArmAdditionalFunction / InitPlaceToAutoTask / SetMotorSpeed
#include "FormsFacade.h"           // TrayForm / fMain offline stand-ins
#include "canary_support.h"        // ShowErrorMessage / ShowMyMessage / RecordProcess / LastSet / IniConfig / K_RETRY / K_SKIP / K_HOME / DUMMY
#include "acarry_shims.h"          // MoveOutArmToAutoSafe / MoveOutArmXY_ToShuttleAlarmArea / NULL_IC
#include "acatchtray_shims.h"      // InitProcessSingleMotorTask
#include "ainarm9045_2x4_16_shims.h"// AutoTeachLoadTrayZ

// -----------------------------------------------------------------------------
//  SUBSTRATE / ENGINE EXTERNS not yet provided by headers in the current cpp
//  tree (REPORTED for the serial Integrate phase).  Forward-declared here behind
//  a single #ifndef guard so this TU compiles standalone and CANNOT ODR-collide
//  once the owning module lands.  Signatures / default args are VERBATIM from
//  golden.  Owning modules: the out-arm engine (golden aoutarm.cpp /
//  aoutarm9045.cpp / acatchtray.cpp / Magazine.cpp / MyKitSuck.h / aRotateKIT*.h).
//
//  Offline-safe behavior: these are GENUINE engine symbols -- the Integrate
//  phase points them at the real (already-translatable) engine defs; no stub
//  bodies are written here (link externs, not no-ops).
// -----------------------------------------------------------------------------
#ifndef AOUTARM9045_ALL_1PICKER_ENGINE_FWD
#define AOUTARM9045_ALL_1PICKER_ENGINE_FWD
extern TMyKitSuck *ptrOutSHT;                                                   // golden MyKitSuck.h -- offline: points at &FRCarryKit / &BRCarryKit grids
extern bool bOutSuckShtDupErr[MAX_ARM_Row][MAX_ARM_Col];                        // golden aoutarm9045.h -- offline: zeroed
extern int  iOutShtRetryCount;                                                  // golden aoutarm9045.h -- offline: 0
extern int  iPickFromShuttle1Task;                                             // golden aoutarm.h -- offline: SM cursor
extern int  iPickFromShuttle2Task;                                             // golden aoutarm.h -- offline: SM cursor
extern int  OutArmTask;                                                        // golden aoutarm.h -- offline: engine SM cursor
extern bool bOutArmManualStepPress;                                            // golden aoutarm.h -- offline: false
extern bool bPickShuttleError;                                                 // golden aoutarm.h:30 -- offline: false

void TransferOutShuttleRatio(int iShuttle, int *iXPos, int *iYPos, int Row, int Col); // golden aoutarm.h -- offline: identity
void SetOutArmNeedDestory(int iRow, int iCol, int iShtRow, int iShtCol, bool bPlace=false); // golden aoutarm.h -- offline: no-op
bool SwapShuttleDataToOutArm(int iSht, int iShtRow, int iShtCol, int iSuckRow, int iSuckCol, int *iWitchErrBin); // golden aoutarm.h -- offline: false (no miss)
void ShowOutputShuttleDataMiss();                                              // golden aoutarm.h -- offline: no-op
bool CheckTesterZ(int WhichShuttle);                                           // golden aoutarm.h -- offline: false (tester Z clear)
void PickFromShuttle(int ibwhichKit);                                          // golden aoutarm.h / kevin 20210612 -- offline: no-op
bool bOutShtwaitPick();                                                        // golden aoutarm.h / kevin 20210602 -- offline: false (no wait)
int  OutArmPickShuttleAlarm(int iSht, bool bHasDuplicateErr, AnsiString ErrPart); // golden aoutarm.h -- offline: K_SKIP
void PorcessJAM0201OutArmPickUpErrorSkip(int iSht, int iShtR, int iShtC, int iSuckR, int iSuckC); // golden aoutarm.h / Steven 20161214 -- offline: no-op
bool ErrorBinBoxDetect(int iError);                                            // golden aoutarm.h / kevin 20160906 -- offline: true
void SetOutArmHome();                                                          // golden aoutarm.h -- offline: no-op
int  GetVariableYOutShuttleData();                                             // golden aoutarm9045.h -- offline: 0
int  GetOutArmToShuttleOffset_9045(int iSht, int iModeRow, int iModeCol, bool bPitch); // golden aoutarm9045.cpp -- offline: -1
int  GetOutArmYToShuttleOffset_9045(int iSht, int iOffsetPos);                 // golden aoutarm9045.cpp -- offline: 0
int  GetOutArmXToShuttleOffset_9045(int iSht, int iOffsetPos);                 // golden aoutarm9045.cpp -- offline: 0

bool CheckOutArmInitState();                                                   // golden aoutarm.h -- offline: true
bool IsCatchTrayReadySupplyNewTray();                                          // golden acatchtray.h -- offline: false (catch-tray not ready)
bool MoveOutArmXY_ToFix_Tray_Full(bool bMoveY=false);                          // golden aoutarm9045.h:41 -- offline: true (reached)
int  CheckOutArmCleanOut(int Task=50);                                         // golden aoutarm9045.h:58 -- offline: 1140 (continue normal pick flow)
bool CheckShuttleICPos(int WhichShuttle);                                      // golden aoutarm9045.h:57 -- offline: true
void InitPickFromShuttle1Task();                                               // golden aoutarm.h / Sam 20230208 -- offline: no-op
void InitPickFromShuttle2Task();                                               // golden aoutarm.h / Sam 20230208 -- offline: no-op
void OutArmSubSpeed();                                                         // golden aoutarm9045.h / Steven 20110525 -- offline: no-op
void OutArmSubSpeedDisplay();                                                  // golden aoutarm9045.h / KaiChen 20171225 -- offline: no-op
void OutArmAddSpeed();                                                         // golden aoutarm9045.h / Steven 20110525 -- offline: no-op
void OutArmAddSpeedDisplay();                                                  // golden aoutarm9045.h / KaiChen 20171225 -- offline: no-op
void SetFixTrayFullIC();                                                       // golden aoutarm9045.h -- offline: no-op
bool DoFixTrayFullAlarm();                                                     // golden aoutarm9045.h -- offline: true
int  VerifyTrayStatus();                                                       // golden aoutarm9045.h -- offline: 3300 (proceed to place)
bool MoveOutRotateToDegreeAtSameTime(int iDegree, bool bInit=false);           // golden aRotateKIT_Out.h / Ifor 20251215 -- offline: true (rotate done)
extern bool CheckRotateOutNotFinish();                                         // golden aRotateKIT.h / kevin 20130524 -- offline: false
extern bool CheckPlaceToMagazineTray(int iWhichTray);                          // golden Magazine.h -- offline: true
extern bool DoPickFromMagazineBuffer();                                        // golden Magazine.h -- offline: true
int  SearchTrayToPlace_Magazine();                                             // golden aoutarm9045.h / Eastsun 20260515 -- offline: 0 (no-op search)

extern bool bCarryControlOutarm1;                                              // golden aoutarm.h / jou 2011-11-21 -- offline: false
extern bool bCarryControlOutarm2;                                              // golden aoutarm.h / jou 2011-11-21 -- offline: false
extern int  iWhichAuto;                                                        // golden aoutarm.h:18 -- offline: place-target Auto index
extern int  iOutRotateFinish;                                                  // golden aRotateKIT.h / kevin 20130524 -- offline: 0
extern int  iOutArmZTeachTask;                                                 // golden AutoTeach.h -- offline: cursor
extern TQPF_Timer hOutArmYpitchHomeTimer;                                      // golden ainarm2.h (HTimer; offline TQPF_Timer -- same Off()/SetSecAndOn() surface, mirrors sibling aoutarm9045_1x3_4.cpp)
void InitPlaceToAutoTask();                                                    // golden aoutarm.h -- offline: no-op
#endif // AOUTARM9045_ALL_1PICKER_ENGINE_FWD

//---------------------------------------------------------------------------
#pragma package(smart_init)
//==============================================================================
//extern bool  bScannerAlarm;
//static int OutArmPitchCylinderState=0;
//==============================================================================
// 輸出臂 x y 軸移到 shuttle 1
//  Ver : 2003_07
//==============================================================================
//個位數為Y座標, 2=標準, 0=A排, 1=B排
//百位數以上為X座標, 分配方式如下
//                    0                     =標準
//         100        |         101         =左右分兩次
//    200       201   |    202       203    =左右分四次
// 300  301  302  303 | 304  305  306  307  =左右分八次
//int GetNowShuttleMode_2x8_16(int iSht)
//{
//    ptrOutSHT=(iSht==0)?&FRCarryKit:&BRCarryKit;
//
//    if(OutArmSuck.iXStep==1 && OutArmSuck.iYStep==1)
//    {
//        return 2;
//    }
//    else if(ptrOutSHT->HasIC()==false)
//    {
//        if(OutArmSuck.iXStep==1)
//        {
//            return 0;
//        }
//        else if(OutArmSuck.iXStep==2)
//        {
//            if(OutArmSuck.iYStep==1)
//                return 10002;           //左邊八顆
//            else
//                return 10000;           //左上四顆
//        }
//        else if(OutArmSuck.iXStep==4)
//        {
//            if(OutArmSuck.iYStep==1)
//                return 20002;           //左邊四顆
//            else
//                return 20000;           //左上兩顆
//        }
//        else //if(OutArmSuck.iXStep==8)
//        {
//            if(OutArmSuck.iYStep==1)
//                return 30002;
//            else
//                return 30000;
//        }
//    }
//    else
//    {
//        if(OutArmSuck.iXStep==1)
//        {
//            for(int j=0; j<OutArmSuck.iPickCol; j++)
//            {
//                if(ptrOutSHT->Item[0][j]>=HAS_IC)
//                    return 0;           //A排八顆
//            }
//            return 1;                   //B排八顆
//        }
//        else if(OutArmSuck.iXStep==2)
//        {
//            if(OutArmSuck.iYStep==1)
//            {
//                if(ptrOutSHT->Item[0][0]>=HAS_IC || ptrOutSHT->Item[0][1]>=HAS_IC || ptrOutSHT->Item[0][2]>=HAS_IC || ptrOutSHT->Item[0][3]>=HAS_IC ||
//                   ptrOutSHT->Item[1][0]>=HAS_IC || ptrOutSHT->Item[1][1]>=HAS_IC || ptrOutSHT->Item[1][2]>=HAS_IC || ptrOutSHT->Item[1][3]>=HAS_IC)
//                    return 10002;       //左邊八顆
//                else //if(ptrOutSHT->Item[0][4]>=HAS_IC || ptrOutSHT->Item[0][5]>=HAS_IC || ptrOutSHT->Item[0][6]>=HAS_IC || ptrOutSHT->Item[0][7]>=HAS_IC ||
//                     //   ptrOutSHT->Item[1][4]>=HAS_IC || ptrOutSHT->Item[1][5]>=HAS_IC || ptrOutSHT->Item[1][6]>=HAS_IC || ptrOutSHT->Item[1][7]>=HAS_IC)
//                    return 11002;       //右邊八顆
//            }
//            else
//            {
//                if(ptrOutSHT->Item[0][0]>=HAS_IC || ptrOutSHT->Item[0][1]>=HAS_IC || ptrOutSHT->Item[0][2]>=HAS_IC || ptrOutSHT->Item[0][3]>=HAS_IC)
//                    return 10000;       //左上四顆
//                else if(ptrOutSHT->Item[1][0]>=HAS_IC || ptrOutSHT->Item[1][1]>=HAS_IC || ptrOutSHT->Item[1][2]>=HAS_IC || ptrOutSHT->Item[1][3]>=HAS_IC)
//                    return 10001;       //左下四顆
//                else if(ptrOutSHT->Item[0][4]>=HAS_IC || ptrOutSHT->Item[0][5]>=HAS_IC || ptrOutSHT->Item[0][6]>=HAS_IC || ptrOutSHT->Item[0][7]>=HAS_IC)
//                    return 11000;       //右上四顆
//                else //if(ptrOutSHT->Item[1][4]>=HAS_IC || ptrOutSHT->Item[1][5]>=HAS_IC || ptrOutSHT->Item[1][6]>=HAS_IC || ptrOutSHT->Item[1][7]>=HAS_IC)
//                    return 11001;       //右下四顆
//            }
//        }
//        else if(OutArmSuck.iXStep==4)
//        {
//            if(OutArmSuck.iYStep==1)
//            {
//                if(ptrOutSHT->Item[0][0]>=HAS_IC || ptrOutSHT->Item[0][1]>=HAS_IC ||
//                   ptrOutSHT->Item[1][0]>=HAS_IC || ptrOutSHT->Item[1][1]>=HAS_IC)
//                    return 20002;       //左邊四顆
//                else if(ptrOutSHT->Item[0][2]>=HAS_IC || ptrOutSHT->Item[0][3]>=HAS_IC ||
//                        ptrOutSHT->Item[1][2]>=HAS_IC || ptrOutSHT->Item[1][3]>=HAS_IC)
//                    return 20102;       //左中四顆
//                else if(ptrOutSHT->Item[0][4]>=HAS_IC || ptrOutSHT->Item[0][5]>=HAS_IC ||
//                        ptrOutSHT->Item[1][4]>=HAS_IC || ptrOutSHT->Item[1][5]>=HAS_IC)
//                    return 21002;       //右中四顆
//                else //if(ptrOutSHT->Item[0][6]>=HAS_IC || ptrOutSHT->Item[0][7]>=HAS_IC ||
//                     //   ptrOutSHT->Item[1][6]>=HAS_IC || ptrOutSHT->Item[1][7]>=HAS_IC)
//                    return 21102;       //右邊四顆
//            }
//            else
//            {
//                if(ptrOutSHT->Item[0][0]>=HAS_IC || ptrOutSHT->Item[0][1]>=HAS_IC)
//                    return 20000;       //左上兩顆
//                else if(ptrOutSHT->Item[1][0]>=HAS_IC || ptrOutSHT->Item[1][1]>=HAS_IC)
//                    return 20001;       //左下兩顆
//                else if(ptrOutSHT->Item[0][2]>=HAS_IC || ptrOutSHT->Item[0][3]>=HAS_IC)
//                    return 20100;       //左上中兩顆
//                else if(ptrOutSHT->Item[1][2]>=HAS_IC || ptrOutSHT->Item[1][3]>=HAS_IC)
//                    return 20101;       //左下中兩顆
//                else if(ptrOutSHT->Item[0][4]>=HAS_IC || ptrOutSHT->Item[0][5]>=HAS_IC)
//                    return 21000;       //右上中兩顆
//                else if(ptrOutSHT->Item[1][4]>=HAS_IC || ptrOutSHT->Item[1][5]>=HAS_IC)
//                    return 21001;       //右下中兩顆
//                else if(ptrOutSHT->Item[0][6]>=HAS_IC || ptrOutSHT->Item[0][7]>=HAS_IC)
//                    return 21100;       //右上中兩顆
//                else //if(ptrOutSHT->Item[1][6]>=HAS_IC || ptrOutSHT->Item[1][7]>=HAS_IC)
//                    return 21101;       //右下中兩顆
//            }
//        }
//        else //if(OutArmSuck.iXStep==8)
//        {
//            if(OutArmSuck.iYStep==1)
//            {
//                if(ptrOutSHT->Item[0][0]>=HAS_IC ||
//                   ptrOutSHT->Item[1][0]>=HAS_IC)
//                    return 30002;
//                else if(ptrOutSHT->Item[0][1]>=HAS_IC ||
//                        ptrOutSHT->Item[1][1]>=HAS_IC)
//                    return 30102;
//                else if(ptrOutSHT->Item[0][2]>=HAS_IC ||
//                        ptrOutSHT->Item[1][2]>=HAS_IC)
//                    return 30202;
//                else if(ptrOutSHT->Item[0][3]>=HAS_IC ||
//                        ptrOutSHT->Item[1][3]>=HAS_IC)
//                    return 30302;
//                else if(ptrOutSHT->Item[0][4]>=HAS_IC ||
//                        ptrOutSHT->Item[1][4]>=HAS_IC)
//                    return 31002;
//                else if(ptrOutSHT->Item[0][5]>=HAS_IC ||
//                        ptrOutSHT->Item[1][5]>=HAS_IC)
//                    return 31102;
//                else if(ptrOutSHT->Item[0][6]>=HAS_IC ||
//                        ptrOutSHT->Item[1][6]>=HAS_IC)
//                    return 31202;
//                else //if(ptrOutSHT->Item[0][7]>=HAS_IC ||
//                     //   ptrOutSHT->Item[1][7]>=HAS_IC)
//                    return 31302;
//            }
//            else
//            {
//                if(ptrOutSHT->Item[0][0]>=HAS_IC)
//                    return 30000;
//                else if(ptrOutSHT->Item[1][0]>=HAS_IC)
//                    return 30001;
//                else if(ptrOutSHT->Item[0][1]>=HAS_IC)
//                    return 30100;
//                else if(ptrOutSHT->Item[1][1]>=HAS_IC)
//                    return 30101;
//                else if(ptrOutSHT->Item[0][2]>=HAS_IC)
//                    return 30200;
//                else if(ptrOutSHT->Item[1][2]>=HAS_IC)
//                    return 30201;
//                else if(ptrOutSHT->Item[0][3]>=HAS_IC)
//                    return 30300;
//                else if(ptrOutSHT->Item[1][3]>=HAS_IC)
//                    return 30301;
//                else if(ptrOutSHT->Item[0][4]>=HAS_IC)
//                    return 31000;
//                else if(ptrOutSHT->Item[1][4]>=HAS_IC)
//                    return 31001;
//                else if(ptrOutSHT->Item[0][5]>=HAS_IC)
//                    return 31100;
//                else if(ptrOutSHT->Item[1][5]>=HAS_IC)
//                    return 31101;
//                else if(ptrOutSHT->Item[0][6]>=HAS_IC)
//                    return 31200;
//                else if(ptrOutSHT->Item[1][6]>=HAS_IC)
//                    return 31201;
//                else if(ptrOutSHT->Item[0][7]>=HAS_IC)
//                    return 31300;
//                else //if(ptrOutSHT->Item[1][7]>=HAS_IC)
//                    return 31301;
//            }
//        }
//    }
//}

// Provenance: golden aoutarm9045_All_1Picker.cpp:220
bool GetNowShuttleRowCol_All_1Picker(int iSht, int *iUseRow, int *iUseCol)
{
    ptrOutSHT=(iSht==0)?&FRCarryKit:&BRCarryKit;

    for(int i=0; i<OutArmSuck.iShtRow; i++)
    {
        for(int j=0; j<OutArmSuck.iShtCol; j++)
        {
            if(ptrOutSHT->Item[i][j]>=HAS_IC)
            {
                *iUseRow=i;
                *iUseCol=j;
                return true;
            }
        }
    }

    return false;
}
//==============================================================================
// Provenance: golden aoutarm9045_All_1Picker.cpp:240
bool OutArmZNeedDown_All_1Picker(int iSht)
{
    if(InitialOutArmNeedSuck(iSht)==false)
        return false;
//    bool bHasIC=false;
    bool bNeedPick=false;
    int iUseRow=0, iUseCol=0;

    ptrOutSHT=(iSht==0)?&FRCarryKit:&BRCarryKit;
//    int iMode=GetNowShuttleMode_2x8_16(iSht);
//    int iModeRow=iMode%100;
//    int iModeCol=iMode/100;
//    int iShtJ=0;

    bNeedPick=GetNowShuttleRowCol_All_1Picker(iSht, &iUseRow, &iUseCol);

    if(bNeedPick)
    {
        SetOutArmNeedDestory(0, 0, iUseRow, iUseCol);
    }
    return true;
}
//------------------------------------------------------------------------------
// Provenance: golden aoutarm9045_All_1Picker.cpp:263
bool CheckOutArmXYPitch_All_1Picker(int *iX, int *iY, int iSht, int iMovePitchX, int iMovePitchY)
{
//    CaculateOutShtXYPos(iSht, iX, iY);
    int iTargetRow=0, iTargetCol=0;
//    int iShuttle=(iSht==0)?OutOfsOutSh1:OutOfsOutSh2;
    int iOffsetPos=(iSht==0)?OutOfsOutSh1:OutOfsOutSh2;
    int iTarget=0;

    GetNowShuttleRowCol_All_1Picker(iSht, &iTargetRow, &iTargetCol);
//    GetShtRowColStartPos(iShuttle,iTarX,iTarY);
//    iTarX+=TestIF.dSiteXPitch*iTargetCol;
//    iTarY-=TestIF.dSiteYPitch*iTargetRow;

    if(iSht==0)
    {
        iTarget=MOutShuttle1;
    }
    else
    {
        iTarget=MOutShuttle2;
    }
    GetShtRowColPos(iTarget,iTargetCol,iTargetRow,*iX,*iY);
    *iY=*iY+GetOutArmYToShuttleOffset_9045(iSht, iOffsetPos);
    *iX=*iX+GetOutArmXToShuttleOffset_9045(iSht, iOffsetPos);
    TransferOutShuttleRatio(iSht, iX, iY, iOutArmYBase, iOutArmXBase);

    if(OutArmZNeedDown_All_1Picker(iSht)==false)
        return false;
    return true;
}
//==============================================================================
// Provenance: golden aoutarm9045_All_1Picker.cpp:294
bool MoveOutArmToShuttleIncludeZ_9045_All_1Pick(int iSht, bool bZDown)
{
    bool bZFlag[MAX_ARM_Row][MAX_ARM_Col];
    int iZPos[MAX_ARM_Row][MAX_ARM_Col];
    int iXVariable[X_PITCH_COUNT];
    int iXPos               =0;                                                 //Steven 20141029 : XY-Pitch for Shuttle Center position for base Suck
    int iYPos               =0;
//    int iMode               =GetNowShuttleMode_2x8_16(iSht);
//    int iModeRow            =iMode%100;
//    int iModeCol            =iMode/100;
    int iMovePitchX         =iXpitchMaxX3;
    int iMovePitchY         =GetVariableYOutShuttleData();
    int iOffsetPos          =GetOutArmToShuttleOffset_9045(iSht, 0, 0, true);
    int iYVariable          =GetOutArmPitchY_9045(iMovePitchY, iOffsetPos);
    ZeroMemory(iXVariable,  sizeof(iXVariable));
    ZeroMemory(bZFlag,      sizeof(bZFlag));
    ZeroMemory(iZPos,       sizeof(iZPos));
//    static bool bCheckZSafe =false;

//    if(OutArmSuck.iXStep==1)                                                    //每一隻吸嘴一個Site Pitch
//        iMovePitchX=TestIF.dSiteXPitch*7;
//    else if(OutArmSuck.iXStep==2)                                               //每兩隻吸嘴一個Site Pitch
//        iMovePitchX=TestIF.dSiteXPitch/2*7;
//    else if(OutArmSuck.iXStep==4)                                               //每四隻吸嘴一個Site Pitch
//        iMovePitchX=TestIF.dSiteXPitch/4*7;
//    else                                                                        //每一隻吸嘴一個最大Pitch
//        iMovePitchX=iXpitchMaxX7;
//
//    for(int i=0; i<X_PITCH_COUNT; i++)
//        iXVariable[i]=GetOutArmPitchX_9045(iMovePitchX, i, iOffsetPos);
//
    if(CheckOutArmXYPitch_All_1Picker(&iXPos, &iYPos, iSht, iMovePitchX, iMovePitchY)==false && bZDown==true)
        return false;
//
    GetOutShuttleStatus_9045(iSht, iZPos, bZDown, bZFlag);
    if(IsCheckOutArmDestroyActiveFinish(iXPos, iYPos)==false)                   //2013-08-02    Dell     add Pick 前先確認CheckInArmDestroyActive 已完成
        return false;
//
//    DoOutArmSuckPreOn(iSht, 0, iXPos, iYPos, bZDown);
    if(OutArmContinuousMove_9045(iXPos, iYPos, iXVariable, iYVariable, bZFlag, iZPos, bZDown))
        return true;

    return false;
}
//==============================================================================
// 輸出臂從 Shuttle 1 吸取 IC 流程動作
//  Ver : 2003_07
//==============================================================================
extern bool CheckOutputShuttleDataMiss(TMyKitSuck *P, int i, int j);            //Steven 20110527 : 整合只叫一次
// Provenance: golden aoutarm9045_All_1Picker.cpp:343
bool DoPickFromShuttle_9045_All_1Pick(int iSht)
{
    static int iWitchErrBin=0;

    int ret;
    int iSuckX, iSuckY, iShtX, iShtY;
    int &Task=(iSht==0)?iPickFromShuttle1Task:iPickFromShuttle2Task;
    bool bHasDuplicateErr=false;                                                //Steven 20110216 : 合併Alarm
    bool bHasErr=false;                                                         //Steven 20110216 : 合併Alarm
    bool flag=false;
    AnsiString ErrPart="";                                                      //Steven 20110216 : 合併Alarm

    ptrOutSHT=(iSht==0)?&FRCarryKit:&BRCarryKit;

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
                if(OutArmSuck.NoIC())                                                                                   //kevin 20210615 第一次 吸取IC 需等待
                {
                    if(MoveOutArmToShuttleIncludeZ_9045_All_1Pick(iSht, false))                                         //移到 Shuttle 上等待
                    {
                        PickFromShuttle(0);                                                                             //kevin 20210612 add int iwhichKit=0
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
            {
                Task=10;
            }

            if(Task!=10)
                break;
        case 10:
            iWitchErrBin=0;
            Task=200;
        case 200:
            if(MoveOutArmToShuttleIncludeZ_9045_All_1Pick(iSht, true))          //kevin 20210717 change function
            {
                #if 0 // TODO(W7) -- golden aoutarm9045_All_1Picker.cpp:400 fAutoTeach->iATOutArmWhichKit (TfAutoTeach VCL form, offline N/A)
                fAutoTeach->iATOutArmWhichKit=iOutArmiWhichKit;                 //JimmyChiu 20211020 : Auto alignment mode
                #endif
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
                #if 0 // TODO(W7) -- golden aoutarm9045_All_1Picker.cpp:420 fAutoTeach->IsRun()/DoNext() (TfAutoTeach VCL form, offline N/A)
                if(fAutoTeach->IsRun())                                         //JimmyChiu 20211020 : Auto alignment mode
                    fAutoTeach->DoNext();
                #endif
                Task=iOUTARM_SUCK;
                OutArmSuck.ResetAll();                                          //Steven 20160323 : 避免未開啟真空
            }
            break;
        case iOUTARM_SUCK:
            flag=true;
            bHasErr=false;

            GetNowShuttleRowCol_All_1Picker(iSht, &iShtX, &iShtY);
            iSuckX  =0;
            iSuckY  =0;

            if(ptrOutSHT->Item[iShtX][iShtY] && OutArmSuck.Suck[iSuckX][iSuckY].GetNeedSuckStatus())
            {
                if(OutArmSuck.Suck[iSuckX][iSuckY].Suck())
                {
                    //模擬ic不見了
                    #ifdef SOFT_SIMULTE
                    if(fMain->chkInPickLoadError->Checked==true)
                    {
                        if(iShtY==1)
                        {
                            OutArmSuck.Suck[iSuckX][iSuckY].Error=true;
                        }
                    }
                    #endif

                    bHasErr=SwapShuttleDataToOutArm(iSht, iShtX, iShtY, iSuckX, iSuckY, &iWitchErrBin);                 //Steven 20170428 : 統一Out Arm從Shuttle吸料後的資料交換
                    bOutSuckShtDupErr[iSuckX][iSuckY]=false;
                }
                else if(OutArmSuck.Suck[iSuckX][iSuckY].Error==false)
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
                    if(OutArmSuck.Item[i][j]==NULL_IC && OutArmSuck.Suck[i][j].GetNeedSuckStatus())
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
//                        for(int i=0; i<OutArmSuck.iPickRow; i++)                 //todo
//                        {
//                            for(int j=0; j<OutArmSuck.iPickCol; j++)
//                            {
//                                if(OutArmSuck.Suck[i][j].Error)
//                                {
//                                    iShtY=XPHNotStander_2x8_16[OutArmSuck.iModeX][j];
//                                    PorcessJAM0201OutArmPickUpErrorSkip(iSht, i, iShtY, i, j);      //Steven 20161214 : 加上OutArm吸取異常Skip的ErrorLog
//                                }
//                                bOutSuckShtDupErr[i][j]=false;
//                            }
//                        }

                        if(OutArmSuck.Suck[0][0].Error)
                        {
                            GetNowShuttleRowCol_All_1Picker(iSht, &iShtX, &iShtY);
                            PorcessJAM0201OutArmPickUpErrorSkip(iSht, iShtX, iShtY, 0, 0);                              //Steven 20161214 : 加上OutArm吸取異常Skip的ErrorLog
                        }

                        if(USE_Fix_AI_CCD==true && TestIF_File.bEnableFix2BGAAICCD==true)                               //KaiChen 20190124 ：矽格湖口 AI CCD Function
                        {
                            #if 0 // TODO(W7): fFixAICCD form (golden FixAICCD.h TfFixAICCD) -- no FormsFacade stand-in; guard + flow kept live, form call gated (no-op offline)
                            fFixAICCD->OutArmCycleCounterUpdate();
                            #endif
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
// Provenance: golden aoutarm9045_All_1Picker.cpp:619
void DoOutArm_9045_All_1Picker()
{
    static int OldPos=1;
    static int iWhichShuttle=0;
    static bool FirstEnter=true, bCheckSpeed=false;
    static bool bCheckSpeed1=false;                                             //KaiChen 20171225 (Steven)：Add Speed Display

    int ret, ct;
    int &Task=OutArmTask;
    bool flag3;

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
                if(tRotate.ActiveRotate==1 &&
                   iRotate_Type==eInOutArm1Motor)                               //add One sucker with rotate
                {
                    Task=110;
                }
                else
                {
                    if(OutArmSuck.HasIC())
                        Task=3000;
                    else
                        Task=100;
                }
            }

            if(Task!=100)                                                       //Steven 20180813 : add Out arm speed
                break;
        case 100:
            IsCatchTrayReadySupplyNewTray();
            if(FirstEnter)
            {
                if(TestIF.iTestMode==_32Site4X8N)
                {
                    if(TestIF_File.iShuttleMode==1)                             //Jou 20150909 : For Add 32Site UPH, 讓Shuttle 2先回去補Input
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
                        else if(FRCarryKit.HasIC() && OutSHT1InRT())            //jou 20171227 : 修正32 sites hang up
                        {
                            OldPos=1;
                            FirstEnter=false;
                        }
                    }
                }
                else
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
                break;
            }

            if(OldPos==1)
            {
                if(BRCarryKit.HasIC() && OutSHT2InRT())
                {
                    Task=2000;
                }
                else
                {
                    iWhichShuttle=0;
                    Task=1000;
                    OldPos=1-OldPos;
                }
            }
            else
            {
                if(FRCarryKit.HasIC() && OutSHT1InRT())
                {
                    Task=1000;
                }
                else
                {
                    iWhichShuttle=1;
                    Task=2000;
                    OldPos=1-OldPos;
                }
            }
            break;
        case 110:
            if(tRotate.ActiveRotate==1 &&
               iRotate_Type==eInOutArm1Motor)                                   //add One sucker with rotate
            {
                MoveOutRotateToDegreeAtSameTime(0, true);
                Task=120;
            }
            break;
        case 120:                                                               //Ifor 20251215 add:Pick Out Shuttle
            if(MoveOutRotateToDegreeAtSameTime(0))
            {
                if(OutArmSuck.HasIC())
                    Task=3000;
                else
                    Task=100;
            }
            break;
        case 200:
            if(MoveOutArmToAutoSafe())
                Task=300;
            break;
        case 300:
            if(TestIF.iTestMode==_32Site4X8N)                                   //Jou 20150909 : For Add 32Site UPH, 讓Shuttle 2先回去補Input
            {
                if(MoveOutArmToShuttleIncludeZ_9045_All_1Pick(1, false))
                    Task=CheckOutArmCleanOut();
            }
            else if(MoveOutArmXY_ToFix_Tray_Full())                             //Steven 20230602 : 統一out arm 讓位位置
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
                if(FRCarryKit.HasIC())
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

            if(MoveOutArmToShuttleIncludeZ_9045_All_1Pick(iWhichShuttle))
            {
                IsCatchTrayReadySupplyNewTray();

//                if(OutArmSuck.iXStep!=1 || OutArmSuck.iYStep!=1)
//                {
//                    iBackMode1=GetNowShuttleMode_2x8_16(0);
//                }

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

            if(FRCarryKit.HasIC() && OutSHT1InRT() && bCheckShuttle1Flag==false)
            {
                if(IniConfig.bL42_UseOutShuttleDesoakTime==true)                                                        //Ztex 2023.04.19 Add HT-1032 TriTemp Function
                {
                    if(FTestSuck.TSoakTime.Off()==false)
                        return;
                }

//#ifdef Carry4
//                MOT[MOutShuttle1].fCanMoveR=false;
//#else
                MOT[MInShuttle1].fCanMoveR=false;
//#endif
                InitPickFromShuttle1Task();
                Task=1230;

//                if(OutArmSuck.iXStep!=1 || OutArmSuck.iYStep!=1)
//                {
//                    iMode1=GetNowShuttleMode_2x8_16(0);
//                    if(iMode1!=iBackMode1)
//                    {
//                        Task=1150;
//                        break;
//                    }
//                    else
//                    {
//                        iMode1=0;
//                        iBackMode1=0;
//                    }
//                }
                break;
            }
            else if(BRCarryKit.HasIC() && OutSHT2InRT() && bCheckShuttle2Flag==false)                                   // change another shuttle
            {
                Task=50;
                break;
            }

            if(IsOutArmCleanOutFinish())                                                                                //Steven 20230323 : fixed for out arm hang up
            {
                Task=50;
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

            if(DoPickFromShuttle_9045_All_1Pick(0))
            {
                PreSetOutAdditionalFlag();                                                                              //Steven 20220614 : Fixed for out arm additional function
//                if(FRCarryKit.HasIC())
//                {
//                    for(int i=0; i<OutArmSuck.iMaxRow; i++)                     //jou 2013-01-21 修正 Tester 分類設定錯誤，Hang up
//                    {
//                        for(int j=0; j<OutArmSuck.iMaxCol; j++)
//                        {
//                            if(i>=OutArmSuck.iShtRow || j>=OutArmSuck.iShtCol)
//                                FRCarryKit.SetItemData(i, j, NULL_IC);
//                        }
//                    }
//                    InitPickFromShuttle1Task();                                 //Sam 20230208 : 修正 Fix AOI 未拍照問題
//                    return;
//                }

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
        case 1230:
            if(bPickShuttleError)
            {
                bPickShuttleError=false;
                InitPickFromShuttle1Task();
                Task=1200;
                break;
            }

            if(DoPickFromShuttle_9045_All_1Pick(0))
            {
                PreSetOutAdditionalFlag();                                                                              //Steven 20220614 : Fixed for out arm additional function
//                if(FRCarryKit.HasIC())
//                {
//                    Task=1200;
//                    InitPickFromShuttle1Task();                                 //Sam 20230208 : 修正 Fix AOI 未拍照問題
//                    return;
//                }

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
                if(BRCarryKit.HasIC())
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

            if(MoveOutArmToShuttleIncludeZ_9045_All_1Pick(iWhichShuttle))
            {
                IsCatchTrayReadySupplyNewTray();

//                if(OutArmSuck.iXStep!=1 || OutArmSuck.iYStep!=1)
//                {
//                    iBackMode2=GetNowShuttleMode_2x8_16(1);
//                }
                Task=2100;
            }

            if(Task!=2100)                                                                                              //Steven 20180813 : add Out arm speed
                break;
        case 2100:
            IsCatchTrayReadySupplyNewTray();

            if(BRCarryKit.HasIC() && OutSHT2InRT() && bCheckShuttle2Flag==false)
            {
                if(IniConfig.bL42_UseOutShuttleDesoakTime==true)                                                        //Ztex 2023.04.19 Add HT-1032 TriTemp Function
                {
                    if(BTestSuck.TSoakTime.Off()==false)
                        return;
                }

//#ifdef Carry4
//                MOT[MOutShuttle2].fCanMoveR=false;
//#else
                MOT[MInShuttle2].fCanMoveR=false;
//#endif
                InitPickFromShuttle2Task();
                Task=2230;

//                if(OutArmSuck.iXStep!=1 || OutArmSuck.iYStep!=1)
//                {
//                    iMode2=GetNowShuttleMode_2x8_16(1);
//                    if(iMode2!=iBackMode2)
//                    {
//                        Task=2050;
//                        break;
//                    }
//                    else
//                    {
//                        iMode2=0;
//                        iBackMode2=0;
//                    }
//                }
                break;
            }
            else if(FRCarryKit.HasIC() && OutSHT1InRT() && bCheckShuttle1Flag==false)
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

            if(DoPickFromShuttle_9045_All_1Pick(1))
            {
                PreSetOutAdditionalFlag();                                                                              //Steven 20220614 : Fixed for out arm additional function
//                if(BRCarryKit.HasIC())
//                {
//                    for(int i=0; i<OutArmSuck.iMaxRow; i++)
//                    {
//                        for(int j=0; j<OutArmSuck.iMaxCol; j++)
//                        {
//                            if(i>=OutArmSuck.iShtRow || j>=OutArmSuck.iShtCol)
//                                BRCarryKit.SetItemData(i, j, NULL_IC);
//                        }
//                    }
//                    InitPickFromShuttle2Task();                                 //Sam 20230208 : 修正 Fix AOI 未拍照問題
//                    return;
//                }

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
        case 2230:
            IsCatchTrayReadySupplyNewTray();

            if(bPickShuttleError)
            {
                bPickShuttleError=false;
                InitPickFromShuttle2Task();
                Task=2200;
                break;
            }

            if(DoPickFromShuttle_9045_All_1Pick(1))
            {
                PreSetOutAdditionalFlag();                                                                              //Steven 20220614 : Fixed for out arm additional function
//                if(BRCarryKit.HasIC())
//                {
//                    Task=2200;
//                    InitPickFromShuttle2Task();                                 //Sam 20230208 : 修正 Fix AOI 未拍照問題
//                    return;
//                }

                if(OutArmSuck.HasIC()==false)
                {
                    Task=50;
                }
                else
                {
                    iOutRotateFinish=0;                                                                                 //kevin 20130524 目前放到 auto tray
                    iOutArmPickPlaceCnt[OutOfsOutSh2]++;                                                                //JerryYang 20180921 Setup Teach功能
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

//                if(TestIF.iTestMode==_32Site4X8N)                             //Jou 20150909 : For Add 32Site UPH, 讓Shuttle 2先回去補Input
//                {                                                             //Sam 20190226 : 16Site4X4
//                    if(FRCarryKit.NoIC()==true)
//                        MOT[MInShuttle1].fCanMoveR=true;
//                    if(BRCarryKit.NoIC()==true)
//                        MOT[MInShuttle2].fCanMoveR=true;
//                }
//                else
                {
                    MOT[MInShuttle1].fCanMoveR=true;
                    MOT[MInShuttle2].fCanMoveR=true;
                }

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

            if(TRAY_ARM_MODE==eAboveCoveyor &&
               MOT[MTrayX].ReadPos()>Prod.iXTrayColor)
            {
                MoveOutArmXY_ToFix_Tray_Full();
                break;
            }
            InitPlaceToAutoTask();

            if(iWhichAuto<=iAutoRight)
            {
                if(TRAY_ARM_MODE==eAboveCoveyor &&
                   (MOT[iMMAuto[iWhichAuto]].fHasTray==false ||
                    MOT[iMMAuto[iWhichAuto]].FullIC()))
                {
                    break;
                }
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
                    if(TRAY_ARM_MODE==eAboveCoveyor &&
                       (MOT[iMMAuto[iWhichAuto]].fHasTray==false ||
                        MOT[iMMAuto[iWhichAuto]].FullIC()))
                    {
                        if(iHWFix_BinBox==1 && iWhichAuto==eBulkBox)
                            DoFixTrayFullAlarm();                                                                       //kevin 20201207 check Error bin box Fix 1 put tray
                        break;
                    }
                }
            }

            if(TRAY_ARM_MODE==eAboveCoveyor &&
               (iCatchTrayControlManual>=2 || WhichAutoNeedTray()!=0))                                                  //Sam 20180822 : OutArm 是否在安全位置，是否讓位，不然會與 TrayArm 補 Tray 動作互卡 Hange up。
            {
                MoveOutArmXY_ToFix_Tray_Full();
                break;
            }
            else if(TRAY_ARM_MODE==eUnderCoveyor&&
                   (iCatchTrayControlManual>=2 || WhichAutoNeedTray()!=0))
            {
                MoveOutArmToAutoSafe_9045();
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
//            if(fAutoTeach->IsRun())                                             //JimmyChiu 20211020 : Auto alignment mode
//            {
//                Task=3305;
//                break;
//            }
            Task=3310;
//            break;
//        case 3305:  //JimmyChiu 20211020 : Auto alignment mode
//            ret=DoPlaceToAutoForAutoTeachOffset_9045_2x8_16();
//            /*
//            0:unfinish
//            1:finish without running next step
//            2:finish and running next step
//            */
//            if(ret==1)
//                Task=100;
//            else if(ret==2)
//                Task=3310;
//            if(Task!=3310)                                                      //Steven 20180813 : add Out arm speed
//                break;
        case 3310:
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
// Provenance: golden aoutarm9045_All_1Picker.cpp:1634
void CaculateOutShtXYPos(int iSht, int &iTarX, int &iTarY)
{
    int iTargetRow=0, iTargetCol=0;
    int iShuttle=(iSht==0)?OutOfsOutSh1:OutOfsOutSh2;
    int iOffsetPos=(iSht==0)?OutOfsOutSh1:OutOfsOutSh2;
    GetNowShuttleRowCol_All_1Picker(iSht, &iTargetRow, &iTargetCol);

    GetShtRowColStartPos(iShuttle,iTarX,iTarY);

    iTarX+=TestIF.dSiteXPitch*iTargetCol;
    iTarY-=TestIF.dSiteYPitch*iTargetRow;

    iTarY=iTarY+GetOutArmYToShuttleOffset_9045(iSht, iOffsetPos);
    iTarX=iTarX+GetOutArmXToShuttleOffset_9045(iSht, iOffsetPos);
}
//------------------------------------------------------------------------------
