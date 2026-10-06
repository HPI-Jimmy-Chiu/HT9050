// =============================================================================
//  aoutarm9045_2x4_8.cpp  --  Out-Arm per-site VARIANT (2x4_8, two-row 4-col 8-pick)
//
//  Faithful translation of golden aoutarm9045_2x4_8.cpp (2151 lines, BCB6, Big5/cp950).
//  Translator: AI(W6.2c-2x4_8) 20260627
//  Translation wave: W6.2c-OUT batch-3 final (out-arm per-site-config variant: 2x4_8).
//
//  ROLE: the per-site out-arm shuttle-pick / place-to-auto state machines for the
//  2x4_8 layout.  This file owns (golden names VERBATIM):
//    GetNowShuttleMode_2x4_8, OutArmZNeedDown_2x4_8, CheckOutArmXYPitch_2x4_8,
//    MoveOutArmToShuttleIncludeZ_9045_2x4_8 (2-PARAM), DoPickFromShuttle_9045_2x4_8,
//    DoOutArm_9045_2x4_8,
//    and the file-LOCAL 9046AU extras:
//      DoPlaceToSortShuttle_2x4_8_AU()        (golden :1047)
//      MoveSortArmToShuttleIncludeZ_AU_2x4_8(bool) (golden :2110)
//
//  ENGINE DISPATCH (LIVE variant): the out-arm engine aoutarm9045.cpp routes
//  iInArmType==e9045_2x4_8 -> DoOutArm_9045_2x4_8() and
//  DoPickFromShuttle_9045_2x4_8(int).  The file layout suffix == the callee
//  suffix (ENUM->CALLEE map: 2x4_8 -> 2x4_8).
//
//  FAITHFUL: every fn/var/cursor value, switch(Task) structure, formula, branch,
//  fall-through and commented-out code is transcribed VERBATIM from the golden
//  CP950 source.  The DoOutArm fall-throughs (case 1->5, 5->10, 10->50, 50->100,
//  1150->1100, 3100->3300, 3300->3301, 3301->3500) are preserved EXACTLY, as is
//  the DoPickFromShuttle 10->200 fall-through and the 9046AU
//  DoPlaceToSortShuttle 10->200 fall-through.
//
//  NOTE (signature regularity vs siblings):
//    * MoveOutArmToShuttleIncludeZ_9045_2x4_8 is the 2-PARAM form
//      (int iSht, bool bZDown) -- NO iKit (unlike sibling 2x4_4 / 2x5_8 /
//      2x6_8 / 2x8_8).  Matches golden .h:6.
//    * The three pure helpers carry the _2x4_8 suffix WITHOUT the _9045_ infix
//      (GetNowShuttleMode_2x4_8 / OutArmZNeedDown_2x4_8 / CheckOutArmXYPitch_2x4_8).
//
//  9046AU CONTENT (kept VERBATIM): heavy sort-arm fan-out (RogerYang 20250513..17
//  markers).  The sort-shuttle kit is OutSht3Kit (acarry_shims surface), the sort
//  motor is MOT[MOutSortSht], the sort-shuttle-in-RT predicate is SortSHTInRT().
//  AU mode is reached only under a 9046AU config (USE_OUT_SORT_ARM!=eartUninstall);
//  offline that branch is inert (eartUninstall) but the bodies link.  The AU
//  sort-arm engine functions (CheckSortArmXYPitch_2x4_8 / GetSortArmToSortShuttleOffset /
//  GetSortArmPitchX / GetSortShuttleStatus_Pick / IsCheckSortArmDestroyActiveFinish /
//  DoSortArmSuckPreOn / SetSortShuttleStatus_Place / iPlaceToSortShtTask /
//  InitPlaceToSortShtTask / NeedPlaceToSort) are forward-declared behind the
//  engine-FWD guard and REPORTED for the serial Integrate phase (link externs --
//  genuine engine symbols owned by the out-arm/sort engine, not no-op stubs).
//
//  VCL/Borland conversions: VCL AnsiString (1-based) via vclcompat; this file has
//  no __fastcall/__property/__published/PACKAGE.  SOFT_SIMULTE is NOT defined ->
//  the one #ifdef SOFT_SIMULTE block in DoPickFromShuttle (case iOUTARM_SUCK,
//  fMain->chkInPickLoadError test) is compiled OUT, kept VERBATIM.  The two
//  fFixAICCD->OutArmCycleCounterUpdate() Fix-AI-CCD form calls (golden FixAICCD.h
//  TfFixAICCD; no FormsFacade stand-in) are gated behind #if 0 // TODO(W7) with
//  the guard + control flow kept live (no-op offline) -- same pattern as the
//  committed sibling aoutarm9045_1x1_1.cpp.
//
//  Big5: every Chinese comment decoded via cp950 and preserved as UTF-8.
//  Final gate: ZERO U+FFFD.
//
//  INCLUDE REWRITE (mirror of the golden header set, translated subset):
//    golden: MachineDefine.h ; aoutarm9045_2x4_8.h ; aArmHeader.h ; MyKitSuck.h ;
//            cprod.h ; mymotor.h ; mycylin.h ; cSortCT.h ; mysensor.h ; main.h ;
//            note.h ; csystem.h ; acatchtray.h ; cinitial.h ; atester.h ;
//            aRotateKIT.h ; fRotate.h ; cOffSet.h ; fAOI.h ; cmydef.h ;
//            ProductionInfo.h ; AutoTeach.h ; FixAICCD.h ; Magazine.h ;
//            #pragma package(smart_init)
// =============================================================================
#include "MachineDefine.h"                  // <windows.h> (ZeroMemory), vclcompat
#include "aoutarm9045_2x4_8.h"

#include "aArmHeader.h"             // __FUNC__ shim, RecordProcess, ShowErrorMessage
#include "aoutarm9045.h"           // engine: GetOutArmPitchX/Y_9045 / GetOutShuttleStatus_9045 / DoOutArmSuckPreOn / OutArmNeedCheckOffset / IsCheckOutArmDestroyActiveFinish / DoOutArmPlaceToAuto_9045 / DoOutArmAfterPlaceToAuto / DoOutArmAdditionalFunction / CheekNeedToDoOutArmAdditionalFunction / VerifyFixTrayLink / SearchTrayToPlace_9045 / PreSetOutAdditionalFlag / MoveOutArmXY_To_ESDSafePos / iOUTARM_SUCK / InitialOutArmNeedSuck / GetShuttleCol
#include "ainarm9045.h"            // bCheckYPitchRunHomeSen / bCheckYPitchHome / GetShuttleCol
#include "Motor/mymotor.h"         // MOT[], TMyMotor, OutArmContinuousMove_9045, SortArmContinuousMove
#include "mycylin.h"
#include "mysensor.h"              // Sen[] / SnFixedTrayDetect
#include "cprod.h"                 // Prod / TestIF / TestIF_File / ArmSpeed / LastSet
#include "cpublic.h"               // CosFunction
#include "cmydef.h"                // global scalar universe + IC consts + iInArmType + iOutArmShtX/YCenterPos / iOutArmX/YBase / iSortArm* / bEnterOffset / USE_OUT_SORT_ARM / iXpitchMax*
#include "csystem.h"               // OutSHT1InRT / OutSHT2InRT / HasICUnderMachine / HasICUnderHotPlate / CheckOutArmSuckICFallDown / AutoTrayReCheck / IsOutArmCleanOutFinish / SortSHTInRT
#include "acatchtray.h"            // WhichAutoNeedTray
#include "aHotPlateSubstrate.h"    // OutArmSuck/FRCarryKit/BRCarryKit + MoveOutArmToAutoSafe_9045 + iWhichBuff + X_PITCH_COUNT/MAX_ARM_Row/MAX_ARM_Col
#include "FormsFacade.h"           // TrayForm / fMain offline stand-ins
#include "canary_support.h"        // ShowErrorMessage / ShowMyMessage / RecordProcess / LastSet / IniConfig / K_RETRY / K_SKIP / K_HOME / DUMMY
#include "acarry_shims.h"          // MoveOutArmToAutoSafe / MoveOutArmXY_ToShuttleAlarmArea / NULL_IC / OutSht3Kit / SortSHTInRT
#include "acatchtray_shims.h"      // InitProcessSingleMotorTask / iWhichAuto
#include "ainarm9045_2x4_16_shims.h"// AutoTeachLoadTrayZ

// -----------------------------------------------------------------------------
//  SUBSTRATE / ENGINE EXTERNS not yet provided by the current cpp tree (REPORTED
//  for the serial Integrate phase).  Forward-declared here behind #ifndef guards
//  so this TU compiles standalone and CANNOT ODR-collide once the owning module
//  lands.  Signatures / default args are VERBATIM from golden.  The owning
//  modules are the out-arm engine (golden aoutarm.cpp / aoutarm9045.cpp /
//  acatchtray.cpp / acarry.cpp / Magazine.cpp / mykitsuck.h).
//
//  Offline-safe behavior for each: they are GENUINE engine symbols -- the
//  Integrate phase points these at the real (already-translatable) engine defs;
//  no stub bodies are written here (these are link externs, not no-ops).
// -----------------------------------------------------------------------------
#ifndef AOUTARM9045_2X4_8_ENGINE_FWD
#define AOUTARM9045_2X4_8_ENGINE_FWD
extern TMyKitSuck *ptrOutSHT;                                                   // golden MyKitSuck.h -- offline: points at &FRCarryKit / &BRCarryKit / &OutSht3Kit grids
extern bool bOutSuckShtDupErr[MAX_ARM_Row][MAX_ARM_Col];                        // golden aoutarm9045.h -- offline: zeroed
extern int  iOutShtRetryCount;                                                 // golden aoutarm9045.h -- offline: 0
extern int  iPickFromShuttle1Task;                                             // golden aoutarm.h -- offline: SM cursor
extern int  iPickFromShuttle2Task;                                             // golden aoutarm.h -- offline: SM cursor
extern int  OutArmTask;                                                        // golden aoutarm.h -- offline: engine SM cursor
extern bool bOutArmManualStepPress;                                            // golden aoutarm.h -- offline: false

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
bool CheckOutArmToTask50(int iSht);                                            // golden aoutarm9045.h:49 -- offline: false
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
extern bool CheckRotateOutNotFinish();                                         // golden aRotateKIT.h / kevin 20130524 -- offline: false
extern bool CheckPlaceToMagazineTray(int iWhichTray);                          // golden Magazine.h -- offline: true
extern bool DoPickFromMagazineBuffer();                                        // golden Magazine.h -- offline: true
int  SearchTrayToPlace_Magazine();                                             // golden aoutarm9045.h / Eastsun 20260515 -- offline: 0 (no-op search)

extern bool bCarryControlOutarm1;                                              // golden aoutarm.h / jou 2011-11-21 -- offline: false
extern bool bCarryControlOutarm2;                                              // golden aoutarm.h / jou 2011-11-21 -- offline: false
extern int  iOutRotateFinish;                                                  // golden aRotateKIT.h / kevin 20130524 -- offline: 0
extern int  iOutArmZTeachTask;                                                 // golden AutoTeach.h -- offline: cursor
extern TQPF_Timer hOutArmYpitchHomeTimer;                                      // golden ainarm2.h (HTimer; offline TQPF_Timer -- same Off()/SetSecAndOn() surface, mirrors sibling aoutarm9045_2x3_6_14.cpp)
void InitPlaceToAutoTask();                                                    // golden aoutarm.h -- offline: no-op
extern int  iWhichAuto;                                                        // golden aoutarm.h:18 -- offline: place-target Auto index

// ---- 9046AU sort-arm engine surface (golden acarry.cpp / aoutarm9045.cpp) ----
//  REPORTED: these are genuine sort-engine symbols (RogerYang 20250506..17, for
//  HT-9046AU).  Offline they are never dispatched (USE_OUT_SORT_ARM==eartUninstall)
//  but must LINK.  Integrate points them at the real sort engine defs.
extern int  iPlaceToSortShtTask;                                               // golden aoutarm9045.h (9046AU) -- offline: SM cursor
void InitPlaceToSortShtTask();                                                 // golden aoutarm9045.h (9046AU) -- offline: no-op
bool NeedPlaceToSort();                                                        // golden aoutarm9045.h (9046AU) -- offline: false (no AU dispatch)
int  GetSortArmToSortShuttleOffset();                                          // golden aoutarm9045.cpp (9046AU) -- offline: 0
int  GetSortArmPitchX(int iMovePitchX, int iOffsetPos);                        // golden aoutarm9045.cpp (9046AU) -- offline: 0
void GetSortShuttleStatus_Pick(int iZPos[][MAX_ARM_Col], bool bZDown, bool bZFlag[][MAX_ARM_Col]); // golden aoutarm9045.cpp (9046AU) -- offline: zeroed
bool IsCheckSortArmDestroyActiveFinish(int iXPos, int iYPos);                  // golden aoutarm9045.cpp (9046AU) -- offline: true
void DoSortArmSuckPreOn(int iKit, int iXPos, int iYPos, bool bZDown);          // golden aoutarm9045.cpp (9046AU) -- offline: no-op
bool CheckSortArmXYPitch_2x4_8(int *iX, int *iY, int iMovePitchX);             // golden aoutarm9045_2x4_8.cpp (9046AU; def lives in sort-engine, NOT this file) -- offline: true
void SetSortShuttleStatus_Place(int iShtRow, int iShtCol, int iSuckRow, int iSuckCol); // golden aoutarm9045.cpp (9046AU) -- offline: no-op
#endif // AOUTARM9045_2X4_8_ENGINE_FWD

//---------------------------------------------------------------------------
#pragma package(smart_init)
//==============================================================================
//個位數為Y座標, 2=標準, 0=A排, 1=B排
//百位數以上為X座標, 分配方式如下, 千位數為Offset左右之分
//                    0                     =標準
//         100        |         110         =左右分兩次
//    200       201   |    210       211    =左右分四次
// Provenance: golden aoutarm9045_2x4_8.cpp:36
int GetNowShuttleMode_2x4_8(int iSht)
{
    switch(iSht)                                                                //RogerYang 20250513 Add for 9046AU
    {
        case 0:
            ptrOutSHT=&FRCarryKit;
        break;
        case 1:
            ptrOutSHT=&BRCarryKit;
        break;
        case 2:
            ptrOutSHT=&OutSht3Kit;                                              //放
        break;
//        case 3:
//            ptrOutSHT=&OutSht3Kit;    //吸
//        break;
    }

    if(iSht==3)                                                                 //SortArm動作，只有兩隻吸嘴
    {
        //改到GetNowSortShuttleMode_2x4_8()處理
//        if(ptrOutSHT->Item[0][0]>=HAS_IC || ptrOutSHT->Item[0][1]>=HAS_IC)
//            return 10000;                                               //左上兩顆
//        else if(ptrOutSHT->Item[1][0]>=HAS_IC || ptrOutSHT->Item[1][1]>=HAS_IC)
//            return 10001;                                               //左下兩顆
//        else if(ptrOutSHT->Item[0][2]>=HAS_IC || ptrOutSHT->Item[0][3]>=HAS_IC)
//            return 11000;                                               //右上兩顆
//        else //if(ptrOutSHT->Item[1][2]>=HAS_IC || ptrOutSHT->Item[1][3]>=HAS_IC)
//            return 11001;
    }
    else                                                                        //OutArm動作
    {
        if(OutArmSuck.iXStep==1 && OutArmSuck.iYStep==1)
        {
            return 2;
        }
        else if(ptrOutSHT->UseSiteNoIC())
        {
            if(OutArmSuck.iXStep==1)
            {
                return 0;
            }
            else if(OutArmSuck.iXStep==2)
            {
                if(OutArmSuck.iYStep==1)
                    return 10002;                                               //左邊八顆
                else
                    return 10000;                                               //左上四顆
            }
            else                                                                //if(OutArmSuck.iXStep==4)
            {
                if(OutArmSuck.iYStep==1)
                    return 20002;                                               //左邊四顆
                else
                    return 20000;                                               //左上兩顆
            }
        }
        else
        {
            if(OutArmSuck.iXStep==1)
            {
                for(int j=0; j<OutArmSuck.iPickCol; j++)
                {
                    if(ptrOutSHT->Item[0][j]>=HAS_IC)
                        return 0;                                               //A排四顆
                }
                return 1;                                                       //B排四顆
            }
            else if(OutArmSuck.iXStep==2)
            {
                if(OutArmSuck.iYStep==1)
                {
                    if(ptrOutSHT->Item[0][0]>=HAS_IC || ptrOutSHT->Item[0][1]>=HAS_IC ||
                       ptrOutSHT->Item[1][0]>=HAS_IC || ptrOutSHT->Item[1][1]>=HAS_IC)
                        return 10002;                                           //左邊四顆
                    else                                                        //if(ptrOutSHT->[0][2]>=HAS_IC || ptrOutSHT->[0][3]>=HAS_IC ||
                         //   ptrOutSHT->[1][2]>=HAS_IC || ptrOutSHT->[1][3]>=HAS_IC)
                        return 11002;                                           //右邊四顆
                }
                else
                {
                    if(ptrOutSHT->Item[0][0]>=HAS_IC || ptrOutSHT->Item[0][1]>=HAS_IC)
                        return 10000;                                           //左上兩顆
                    else if(ptrOutSHT->Item[1][0]>=HAS_IC || ptrOutSHT->Item[1][1]>=HAS_IC)
                        return 10001;                                           //左下兩顆
                    else if(ptrOutSHT->Item[0][2]>=HAS_IC || ptrOutSHT->Item[0][3]>=HAS_IC)
                        return 11000;                                           //右上兩顆
                    else                                                        //if(ptrOutSHT->Item[1][2]>=HAS_IC || ptrOutSHT->Item[1][3]>=HAS_IC)
                        return 11001;                                           //右下兩顆
                }
            }
            else                                                                //if(OutArmSuck.iXStep==4)
            {
                if(OutArmSuck.iYStep==1)
                {
                    if(ptrOutSHT->Item[0][0]>=HAS_IC ||
                       ptrOutSHT->Item[1][0]>=HAS_IC)
                        return 20002;
                    else if(ptrOutSHT->Item[0][1]>=HAS_IC ||
                            ptrOutSHT->Item[1][1]>=HAS_IC)
                        return 20102;
                    else if(ptrOutSHT->Item[0][2]>=HAS_IC ||
                            ptrOutSHT->Item[1][2]>=HAS_IC)
                        return 21002;
                    else                                                        //if(ptrOutSHT->Item[0][3]>=HAS_IC ||
                         //   ptrOutSHT->Item[1][3]>=HAS_IC)
                        return 21102;
                }
                else
                {
                    if(ptrOutSHT->Item[0][0]>=HAS_IC)
                        return 20000;
                    else if(ptrOutSHT->Item[1][0]>=HAS_IC)
                        return 20001;
                    else if(ptrOutSHT->Item[0][1]>=HAS_IC)
                        return 20100;
                    else if(ptrOutSHT->Item[1][1]>=HAS_IC)
                        return 20101;
                    else if(ptrOutSHT->Item[0][2]>=HAS_IC)
                        return 21000;
                    else if(ptrOutSHT->Item[1][2]>=HAS_IC)
                        return 21001;
                    else if(ptrOutSHT->Item[0][3]>=HAS_IC)
                        return 21100;
                    else                                                        //if(ptrOutSHT->Item[1][3]>=HAS_IC)
                        return 21101;
                }
            }
        }
    }
    return 0;
}
//==============================================================================
// Provenance: golden aoutarm9045_2x4_8.cpp:169
bool OutArmZNeedDown_2x4_8(int iSht)
{
    if(InitialOutArmNeedSuck(iSht)==false)
        return false;

    switch(iSht)                                                                //RogerYang 20250513 Add for 9046AU
    {
        case 0:
            ptrOutSHT=&FRCarryKit;
        break;
        case 1:
            ptrOutSHT=&BRCarryKit;
        break;
        case 2:
            ptrOutSHT=&OutSht3Kit;
        break;
    }

    int iMode   =GetNowShuttleMode_2x4_8(iSht);
    int iModeRow=iMode%100;
    int iModeCol=iMode/100;
    int iShtJ   =0;
    bool bPlace=false;                                                          //RogerYang 20250513 Add for 9046AU

    for(int i=0; i<OutArmSuck.iPickRow; i++)
    {
        for(int j=0; j<OutArmSuck.iPickCol; j++)
        {
            if(iSht==2                              &&
              (OutArmSuck.iWhichAuto[i][j]==eAuto4  ||
               OutArmSuck.iWhichAuto[i][j]==eAuto5  ||
               OutArmSuck.iWhichAuto[i][j]==eAuto6  ))
            {
                bPlace=true;
            }
            else
            {
                bPlace=false;
            }

            iShtJ=GetShuttleCol(i, j);
            if(iModeCol==0 &&
               (iModeRow==2 || iModeRow==i))                                    //Steven 20240205 : 修正 Out arm 放料
            {
                SetOutArmNeedDestory(i, j, i, iShtJ, bPlace);
            }
            else if(iModeCol==100 && (j==0 || j==2) &&                          //吸嘴       ==>  蝦頭
                    (iModeRow==2 || iModeRow==i))                               // O X O X        O O X X
            {                                                                   // O X O X        O O X X
                SetOutArmNeedDestory(i, j, i, iShtJ, bPlace);
            }
            else if(iModeCol==110 && (j==1 || j==3) &&                          //吸嘴       ==>  蝦頭
                    (iModeRow==2 || iModeRow==i))                               // X O X O        X X O O
            {                                                                   // X O X O        X X O O
                SetOutArmNeedDestory(i, j, i, iShtJ, bPlace);
            }
            else if(iModeCol==200 && j==0 &&                                    //吸嘴       ==>  蝦頭
                    (iModeRow==2 || iModeRow==i))                               // O X X X        O X X X
            {                                                                   // O X X X        O X X X
                SetOutArmNeedDestory(i, j, i, iShtJ, bPlace);
            }
            else if(iModeCol==201 && j==1 &&                                    //吸嘴       ==>  蝦頭
                    (iModeRow==2 || iModeRow==i))                               // X O X X        X O X X
            {                                                                   // X O X X        X O X X
                SetOutArmNeedDestory(i, j, i, iShtJ, bPlace);
            }
            else if(iModeCol==210 && j==2 &&                                    //吸嘴       ==>  蝦頭
                    (iModeRow==2 || iModeRow==i))                               // X X O X        X X O X
            {                                                                   // X X O X        X X O X
                SetOutArmNeedDestory(i, j, i, iShtJ, bPlace);
            }
            else if(iModeCol==211 && j==3 &&                                    //吸嘴       ==>  蝦頭
                    (iModeRow==2 || iModeRow==i))                               // X X X O        X X X O
            {                                                                   // X X X O        X X X O
                SetOutArmNeedDestory(i, j, i, iShtJ, bPlace);
            }
        }
    }
    return true;
}
//------------------------------------------------------------------------------
// Provenance: golden aoutarm9045_2x4_8.cpp:250
bool CheckOutArmXYPitch_2x4_8(int *iX, int *iY, int iSht, int iMovePitchX, int iMovePitchY)
{
    int iMode       =GetNowShuttleMode_2x4_8(iSht);
    int iModeRow    =iMode%100;
    int iModeCol    =iMode/100;
    int iOffsetPos  =GetOutArmToShuttleOffset_9045(iSht, iModeRow, iModeCol, false);
    double dMovePitchX;

    if(iSht==0)
    {
        *iX+=Prod.XOutArm_Shuttle1_Pick[iOutArmYBase][iOutArmXBase];
        *iY+=Prod.YOutArm_Shuttle1_Pick[iOutArmYBase][iOutArmXBase];
    }
    else if(iSht==1)                                                            //RogerYang 20250513 Add for 9046AU
    {
        *iX+=Prod.XOutArm_Shuttle2_Pick[iOutArmYBase][iOutArmXBase];
        *iY+=Prod.YOutArm_Shuttle2_Pick[iOutArmYBase][iOutArmXBase];
    }
    else
    {
        *iX+=Prod.XOutArm_SortShuttle_Place[iSortArmYBase][iSortArmXBase];
        *iY+=Prod.YOutArm_SortShuttle_Place[iSortArmYBase][iSortArmXBase];
    }

    if(iModeRow==0)                                                             //Row A
    {
        if(bUseTwoArm32Site &&
           TestIF_File.iYPitchOffsetMode==0)                                    //向舊版本相容
        {
            if(iOutArmYBase==1)                                                 //RogerYang 20260302 : Fixed for Y Pitch
            {
                if(iSht==0)
                {
                    *iY=*iY-iMovePitchY+                                        //A排到中間
                            7000.0-                                             //A排到Shuttle邊緣 (Socket中線)
                            (TestIF.dSiteYOffset/2.0);                          //A排移動半個Center Pitch
                }
                else
                {
                    *iY=*iY-iMovePitchY-                                        //A排到中間
                            7000.0+                                             //A排到Shuttle邊緣 (Socket中線)
                            (TestIF.dSiteYOffset/2.0)+                          //A排移動半個Center Pitch
                            TestIF.dSiteYPitch;                                 //A排移動一個Y-Pitch
                }
            }
            else                                                                //RogerYang 20260302 : Fixed for Y Pitch
            {
                if(iSht==0)
                {
                    *iY=*iY+7000.0-                                             //A排到Shuttle邊緣 (Socket中線)
                            (TestIF.dSiteYOffset/2.0);                          //A排移動半個Center Pitch
                }
                else
                {
                    *iY=*iY-7000.0+                                             //A排到Shuttle邊緣 (Socket中線)
                            (TestIF.dSiteYOffset/2.0)+                          //A排移動半個Center Pitch
                            TestIF.dSiteYPitch;                                 //A排移動一個Y-Pitch
                }
            }
        }
        else if(bUseTwoArm32Site &&
                TestIF_File.iYPitchOffsetMode==2)                               //Steven 20201013 : NN mode add Y-Pitch offset selection
        {
            if(USE_OUT_Y_IS_AUTO_PITCH==true)                                   //JerryYang 20251218 : IN/OUT ARM支援不同模組
            {
                if(iSht==0)
                {
                    *iY=*iY-iMovePitchY+                                        //A排到中間
                            7000.0-                                             //A排到Shuttle邊緣 (Socket中線)
                            (TestIF.dSiteYOffset/2.0);                          //A排移動半個Center Pitch
                }
                else
                {
                    *iY=*iY-iMovePitchY-                                        //A排到中間
                            7000.0+                                             //A排到Shuttle邊緣 (Socket中線)
                            (TestIF.dSiteYOffset/2.0)+                          //A排移動半個Center Pitch
                            TestIF.dSiteYPitch;                                 //A排移動一個Y-Pitch
                }
            }
            else
            {
                if(iSht==0)
                {
                    *iY=*iY+7000.0-                                             //A排到Shuttle邊緣 (Socket中線)
                            (TestIF.dSiteYOffset/2.0);                          //A排移動半個Center Pitch
                }
                else
                {
                    *iY=*iY-7000.0+                                             //A排到Shuttle邊緣 (Socket中線)
                            (TestIF.dSiteYOffset/2.0)+                          //A排移動半個Center Pitch
                            TestIF.dSiteYPitch;                                 //A排移動一個Y-Pitch
                }
            }
        }
        else
        {
            if(USE_OUT_Y_IS_AUTO_PITCH==true)                                   //JerryYang 20251218 : IN/OUT ARM支援不同模組
                *iY=*iY-iMovePitchY+TestIF.dSiteYPitch/2;
            else
                *iY=*iY+TestIF.dSiteYPitch/2;
        }
    }
    else if(iModeRow==1)                                                        //Row B
    {
        if(bUseTwoArm32Site &&
           TestIF_File.iYPitchOffsetMode==0)                                    //向舊版本相容
        {
            if(iOutArmYBase==1)                                                 //RogerYang 20260302 : Fixed for Y Pitch
            {
                if(iSht==0)
                {
                    *iY=*iY+7000.0-                                             //B排到Shuttle邊緣 (Socket中線)
                            (TestIF.dSiteYOffset/2.0)-                          //B排移動半個Center Pitch
                            TestIF.dSiteYPitch;                                 //B排移動到Row B
                }
                else
                {
                    *iY=*iY-7000.0+                                             //B排到Shuttle邊緣 (Socket中線)
                            (TestIF.dSiteYOffset/2.0);                          //B排移動半個Center Pitch
                }
            }
            else                                                                //RogerYang 20260302 : Fixed for Y Pitch
            {
                if(iSht==0)
                {
                    *iY=*iY+iMovePitchY                                         //B排到中間
                            +7000.0-                                            //B排到Shuttle邊緣 (Socket中線)
                            (TestIF.dSiteYOffset/2.0)-                          //B排移動半個Center Pitch
                            TestIF.dSiteYPitch;                                 //B排移動到Row B
                }
                else
                {
                    *iY=*iY+iMovePitchY                                         //B排到中間
                            -7000.0+                                            //B排到Shuttle邊緣 (Socket中線)
                            (TestIF.dSiteYOffset/2.0);                          //B排移動半個Center Pitch
                }
            }
        }
        else if(bUseTwoArm32Site &&
                TestIF_File.iYPitchOffsetMode==2)                               //Steven 20201013 : NN mode add Y-Pitch offset selection
        {
            if(USE_OUT_Y_IS_AUTO_PITCH==true)                                   //JerryYang 20251218 : IN/OUT ARM支援不同模組
            {
                if(iSht)
                {
                    *iY=*iY+7000.0-                                             //B排到Shuttle邊緣 (Socket中線)
                            (TestIF.dSiteYOffset/2.0)-                          //B排移動半個Center Pitch
                            TestIF.dSiteYPitch;                                 //B排移動到Row B
                }
                else
                {
                    *iY=*iY-7000.0+                                             //B排到Shuttle邊緣 (Socket中線)
                            (TestIF.dSiteYOffset/2.0);                          //B排移動半個Center Pitch
                }
            }
            else
            {
                if(iSht==0)
                {
                    *iY=*iY+iMovePitchY+                                        //B排到中間
                           +7000.0-                                             //B排到Shuttle邊緣 (Socket中線)
                            (TestIF.dSiteYOffset/2.0)-                          //B排移動半個Center Pitch
                            TestIF.dSiteYPitch;                                 //B排移動到Row B
                }
                else
                {
                    *iY=*iY+iMovePitchY-                                        //B排到中間
                            7000.0+                                             //B排到Shuttle邊緣 (Socket中線)
                            (TestIF.dSiteYOffset/2.0);                          //B排移動半個Center Pitch
                }
            }
        }
        else
        {
            if(USE_OUT_Y_IS_AUTO_PITCH==true)                                   //JerryYang 20251218 : IN/OUT ARM支援不同模組
                *iY=*iY-TestIF.dSiteYPitch/2;
            else
                *iY=*iY+iMovePitchY-TestIF.dSiteYPitch/2;
        }
    }
    else                                                                        //Both
    {
        if(bUseTwoArm32Site &&
           TestIF_File.iYPitchOffsetMode==0)                                    //向舊版本相容
        {
            if(USE_OUT_Y_IS_AUTO_PITCH==true)                                   //JerryYang 20251218 : IN/OUT ARM支援不同模組
            {
                if(iSht==0)
                {
                    *iY=*iY-iMovePitchY+                                        //A排到中間
                            7000.0-                                             //A排到Shuttle邊緣 (Socket中線)
                            (TestIF.dSiteYOffset/2.0);                          //A排移動半個Center Pitch
                }
                else
                {
                    *iY=*iY-iMovePitchY-                                        //A排到中間
                            7000.0+                                             //A排到Shuttle邊緣 (Socket中線)
                            (TestIF.dSiteYOffset/2.0)+                          //A排移動半個Center Pitch
                            TestIF.dSiteYPitch;                                 //A排移動一個Y-Pitch
                }
            }
            else
            {
                if(iSht==0)
                {
                    *iY+=TestIF.dSiteYOffset;
                    *iY+=TestIF.dSiteYPitch;
                }
                else
                {
                    *iY-=TestIF.dSiteYOffset;
                }
            }
        }
        else if(bUseTwoArm32Site &&
                TestIF_File.iYPitchOffsetMode==2)                               //Steven 20201013 : NN mode add Y-Pitch offset selection
        {
            if(USE_OUT_Y_IS_AUTO_PITCH==true)                                   //JerryYang 20251218 : IN/OUT ARM支援不同模組
            {
                if(iSht==0)
                {
                    *iY=*iY-iMovePitchY+                                        //A排到中間
                            7000.0-                                             //A排到Shuttle邊緣 (Socket中線)
                            (TestIF.dSiteYOffset/2.0);                          //A排移動半個Center Pitch
                }
                else
                {
                    *iY=*iY-iMovePitchY-                                        //A排到中間
                            7000.0+                                             //A排到Shuttle邊緣 (Socket中線)
                            (TestIF.dSiteYOffset/2.0)+                          //A排移動半個Center Pitch
                            TestIF.dSiteYPitch;                                 //A排移動一個Y-Pitch
                }
            }
            else
            {
                if(iSht==0)
                {
                    *iY=*iY+7000.0-                                             //A排到Shuttle邊緣 (Socket中線)
                            (TestIF.dSiteYOffset/2.0);                          //A排移動半個Center Pitch
                }
                else
                {
                    *iY=*iY-7000.0+                                             //A排到Shuttle邊緣 (Socket中線)
                            (TestIF.dSiteYOffset/2.0)+                          //A排移動半個Center Pitch
                            TestIF.dSiteYPitch;                                 //A排移動一個Y-Pitch
                }
            }
        }
        else
        {
            if(USE_OUT_Y_IS_AUTO_PITCH==true)                                   //JerryYang 20251218 : IN/OUT ARM支援不同模組
                *iY=*iY-TestIF.dSiteYPitch/2;
            else
                *iY=*iY+TestIF.dSiteYPitch/2;
        }
    }

    if(USE_OUT_ARM_Y_PITCH==iXYPitch16Bd_Be)                                    //Ztex 2023.12.06 Add HT-1032  //JerryYang 20251218 : IN/OUT ARM支援不同模組
    {
        dMovePitchX=double(iMovePitchX)/7.0;
        if(iModeCol==0)                                                         //Aa --> Aa
        {
            *iX=*iX+(dMovePitchX*4.0-TestIF.dSiteXPitch*1.5);
        }
        else if(iModeCol==100)                                                  //Aa --> Aa
        {
            if(TestIF_File.bEnableUseXCenterPitch==true)
                *iX=*iX+(dMovePitchX*4.0-TestIF.dSiteXCenterPitch*0.5-TestIF.dSiteXPitch*0.5);
            else
                *iX=*iX+(dMovePitchX*4.0-TestIF.dSiteXPitch*1.5);
        }
        else if(iModeCol==110)                                                  //Ac --> Ac
        {
            if(TestIF_File.bEnableUseXCenterPitch==true)
                *iX=*iX+(dMovePitchX*2.0+TestIF.dSiteXCenterPitch*0.5);
            else
                *iX=*iX+(dMovePitchX*2.0+TestIF.dSiteXPitch*0.5);
        }
        else if(iModeCol==200)                                                  //Aa --> Aa
        {
            if(TestIF_File.bEnableUseXCenterPitch==true)
                *iX=*iX+(dMovePitchX*4.0-TestIF.dSiteXCenterPitch*0.5-TestIF.dSiteXPitch*0.5);
            else
                *iX=*iX+(dMovePitchX*4.0-TestIF.dSiteXPitch*1.5);
        }
        else if(iModeCol==201)                                                  //Ac --> Ab
        {
            if(TestIF_File.bEnableUseXCenterPitch==true)
                *iX=*iX+(dMovePitchX*2.0-TestIF.dSiteXCenterPitch*0.5);
            else
                *iX=*iX+(dMovePitchX*2.0-TestIF.dSiteXPitch*0.5);
        }
        else if(iModeCol==210)                                                  //Ae --> Ac
        {
            if(TestIF_File.bEnableUseXCenterPitch==true)
                *iX=*iX+(-dMovePitchX*0.0+TestIF.dSiteXCenterPitch*0.5);
            else
                *iX=*iX+(-dMovePitchX*0.0+TestIF.dSiteXPitch*0.5);
        }
        else if(iModeCol==211)                                                  //Ag --> Ad
        {
            if(TestIF_File.bEnableUseXCenterPitch==true)
                *iX=*iX+(-dMovePitchX*2.0+TestIF.dSiteXCenterPitch*0.5+TestIF.dSiteXPitch*0.5);
            else
                *iX=*iX+(-dMovePitchX*2.0+TestIF.dSiteXPitch*1.5);
        }
    }
    else if(USE_PICKER_COUNT==ep16Picker)                                       //基準為第四隻吸嘴
    {
        dMovePitchX=double(iMovePitchX)/7.0;
        if(iModeCol==0)                                                         //Aa --> Aa
        {
            *iX=*iX+(dMovePitchX*3.0-TestIF.dSiteXPitch*1.5);
        }
        else if(iModeCol==100)                                                  //Aa --> Aa
        {
            if(TestIF_File.bEnableUseXCenterPitch==true)
                *iX=*iX+(dMovePitchX*3.0-TestIF.dSiteXCenterPitch*0.5-TestIF.dSiteXPitch*0.5);
            else
                *iX=*iX+(dMovePitchX*3.0-TestIF.dSiteXPitch*1.5);
        }
        else if(iModeCol==110)                                                  //Ac --> Ac
        {
            if(TestIF_File.bEnableUseXCenterPitch==true)
                *iX=*iX+(dMovePitchX*1.0+TestIF.dSiteXCenterPitch*0.5);
            else
                *iX=*iX+(dMovePitchX*1.0+TestIF.dSiteXPitch*0.5);
        }
        else if(iModeCol==200)                                                  //Aa --> Aa
        {
            if(TestIF_File.bEnableUseXCenterPitch==true)
                *iX=*iX+(dMovePitchX*3.0-TestIF.dSiteXCenterPitch*0.5-TestIF.dSiteXPitch*0.5);
            else
                *iX=*iX+(dMovePitchX*3.0-TestIF.dSiteXPitch*1.5);
        }
        else if(iModeCol==201)                                                  //Ac --> Ab
        {
            if(TestIF_File.bEnableUseXCenterPitch==true)
                *iX=*iX+(dMovePitchX*1.0-TestIF.dSiteXCenterPitch*0.5);
            else
                *iX=*iX+(dMovePitchX*1.0-TestIF.dSiteXPitch*0.5);
        }
        else if(iModeCol==210)                                                  //Ae --> Ac
        {
            if(TestIF_File.bEnableUseXCenterPitch==true)
                *iX=*iX+(-dMovePitchX*1.0+TestIF.dSiteXCenterPitch*0.5);
            else
                *iX=*iX+(-dMovePitchX*1.0+TestIF.dSiteXPitch*0.5);
        }
        else if(iModeCol==211)                                                  //Ag --> Ad
        {
            if(TestIF_File.bEnableUseXCenterPitch==true)
                *iX=*iX+(-dMovePitchX*3.0+TestIF.dSiteXCenterPitch*0.5+TestIF.dSiteXPitch*0.5);
            else
                *iX=*iX+(-dMovePitchX*3.0+TestIF.dSiteXPitch*1.5);
        }
    }
    else if(USE_OUT_ARM_Y_PITCH==iXYPitchVariable)                              //基準為第二隻吸嘴  //JerryYang 20251218 : IN/OUT ARM支援不同模組
    {
        dMovePitchX=double(iMovePitchX)/3.0;
        if(iModeCol==0)                                                         //Aa --> Aa
        {
            *iX=*iX+(dMovePitchX*1.0-TestIF.dSiteXPitch*1.5);
        }
        else if(iModeCol==100)                                                  //Aa --> Aa
        {
            if(TestIF_File.bEnableUseXCenterPitch==true)
                *iX=*iX+(dMovePitchX*1.0-TestIF.dSiteXCenterPitch*0.5-TestIF.dSiteXPitch*1.0);
            else
                *iX=*iX+(dMovePitchX*1.0-TestIF.dSiteXPitch*1.5);
        }
        else if(iModeCol==110)                                                  //Ab --> Ac
        {
            if(TestIF_File.bEnableUseXCenterPitch==true)
                *iX=*iX+(dMovePitchX*0.0+TestIF.dSiteXCenterPitch*0.5);
            else
                *iX=*iX+(dMovePitchX*0.0+TestIF.dSiteXPitch*0.5);
        }
        else if(iModeCol==200)                                                  //Aa --> Aa
        {
            if(TestIF_File.bEnableUseXCenterPitch==true)
                *iX=*iX+(dMovePitchX*1.0-TestIF.dSiteXCenterPitch*0.5-TestIF.dSiteXPitch*1.0);
            else
                *iX=*iX+(dMovePitchX*1.0-TestIF.dSiteXPitch*1.5);
        }
        else if(iModeCol==201)                                                  //Ab --> Ab
        {
            if(TestIF_File.bEnableUseXCenterPitch==true)
                *iX=*iX+(dMovePitchX*0.0-TestIF.dSiteXCenterPitch*0.5);
            else
                *iX=*iX+(dMovePitchX*0.0-TestIF.dSiteXPitch*0.5);
        }
        else if(iModeCol==210)                                                  //Ac --> Ac
        {
            if(TestIF_File.bEnableUseXCenterPitch==true)
                *iX=*iX+(-dMovePitchX*1.0+TestIF.dSiteXCenterPitch*0.5);
            else
                *iX=*iX+(-dMovePitchX*1.0+TestIF.dSiteXPitch*0.5);
        }
        else if(iModeCol==211)                                                  //Ad --> Ad
        {
            if(TestIF_File.bEnableUseXCenterPitch==true)
                *iX=*iX+(-dMovePitchX*2.0+TestIF.dSiteXCenterPitch*0.5+TestIF.dSiteXPitch*1.0);
            else
                *iX=*iX+(-dMovePitchX*2.0+TestIF.dSiteXPitch*1.5);
        }
    }
    else                                                                        //基準為第三隻吸嘴
    {
        dMovePitchX=double(iMovePitchX)/3.0;
        if(iModeCol==0)                                                         //Aa --> Aa
        {
            *iX=*iX+(dMovePitchX*2.0-TestIF.dSiteXPitch*1.5);
        }
        else if(iModeCol==100)                                                  //Aa --> Aa
        {
            if(TestIF_File.bEnableUseXCenterPitch==true)
            {
                *iX=*iX+(dMovePitchX*2.0-TestIF.dSiteXCenterPitch*1-TestIF.dSiteXPitch*0.5);
            }
            else
            {
                *iX=*iX+(dMovePitchX*2.0-TestIF.dSiteXPitch*1.5);
            }
        }
        else if(iModeCol==110)                                                  //Ab --> Ac
        {
            if(TestIF_File.bEnableUseXCenterPitch==true)
                *iX=*iX+(dMovePitchX*1.0+TestIF.dSiteXCenterPitch*0.5);
            else
                *iX=*iX+(dMovePitchX*1.0+TestIF.dSiteXPitch*0.5);
        }
        else if(iModeCol==200)                                                  //Aa --> Aa
        {
            if(TestIF_File.bEnableUseXCenterPitch==true)
                *iX=*iX+(dMovePitchX*2.0-TestIF.dSiteXCenterPitch*1-TestIF.dSiteXPitch*0.5);
            else
                *iX=*iX+(dMovePitchX*2.0-TestIF.dSiteXPitch*1.5);
        }
        else if(iModeCol==201)                                                  //Ab --> Ab
        {
            if(TestIF_File.bEnableUseXCenterPitch==true)
                *iX=*iX+(dMovePitchX*1.0-TestIF.dSiteXCenterPitch*0.5);
            else
                *iX=*iX+(dMovePitchX*1.0-TestIF.dSiteXPitch*0.5);
        }
        else if(iModeCol==210)                                                  //Ac --> Ac
        {
            if(TestIF_File.bEnableUseXCenterPitch==true)
                *iX=*iX+(dMovePitchX*0.0+TestIF.dSiteXCenterPitch*0.5);
            else
                *iX=*iX+(dMovePitchX*0.0+TestIF.dSiteXPitch*0.5);
        }
        else if(iModeCol==211)                                                  //Ad --> Ad
        {
            if(TestIF_File.bEnableUseXCenterPitch==true)
                *iX=*iX+(-dMovePitchX*1.0+TestIF.dSiteXCenterPitch*0.5+TestIF.dSiteXPitch*1.0);
            else
                *iX=*iX+(-dMovePitchX*1.0+TestIF.dSiteXPitch*1.5);
        }
    }

    *iY=*iY+GetOutArmYToShuttleOffset_9045(iSht, iOffsetPos);
    *iX=*iX+GetOutArmXToShuttleOffset_9045(iSht, iOffsetPos);
    TransferOutShuttleRatio(iSht, iX, iY, iOutArmYBase, iOutArmXBase);
    if(OutArmZNeedDown_2x4_8(iSht)==false)
        return false;
    return true;
}
//==============================================================================
// Provenance: golden aoutarm9045_2x4_8.cpp:720
bool MoveOutArmToShuttleIncludeZ_9045_2x4_8(int iSht, bool bZDown)
{
    bool bZFlag[MAX_ARM_Row][MAX_ARM_Col];
    int iZPos[MAX_ARM_Row][MAX_ARM_Col];
    int iXVariable[X_PITCH_COUNT];
    int iXPos               =iOutArmShtXCenterPos;                              //Steven 20141029 : XY-Pitch for Shuttle Center position for base Suck
    int iYPos               =iOutArmShtYCenterPos;
    int iMode               =GetNowShuttleMode_2x4_8(iSht);
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
            iMovePitchX=TestIF.dSiteXPitch/2*7;
        else if(OutArmSuck.iXStep==2)
            iMovePitchX=TestIF.dSiteXPitch/4*7;
        else
            iMovePitchX=iXpitchMaxX7;
    }
    else
    {
        if(OutArmSuck.iXStep==1)
            iMovePitchX=TestIF.dSiteXPitch*3;
        else if(OutArmSuck.iXStep==2)
            iMovePitchX=TestIF.dSiteXPitch/2*3;
        else
            iMovePitchX=iXpitchMaxX3;
    }

    for(int i=0; i<X_PITCH_COUNT; i++)
        iXVariable[i]=GetOutArmPitchX_9045(iMovePitchX, i, iOffsetPos);

    if(CheckOutArmXYPitch_2x4_8(&iXPos, &iYPos, iSht, iMovePitchX, iMovePitchY)==false && bZDown==true)
        return false;

    GetOutShuttleStatus_9045(iSht, iZPos, bZDown, bZFlag);
    if(iSht!=2 && IsCheckOutArmDestroyActiveFinish(iXPos, iYPos)==false)        //RogerYang 20250513 Add for 9046AU           //2013-08-02    Dell     add Pick 前先確認CheckInArmDestroyActive 已完成
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
// Provenance: golden aoutarm9045_2x4_8.cpp:777
bool DoPickFromShuttle_9045_2x4_8(int iSht)
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
                    if(MoveOutArmToShuttleIncludeZ_9045_2x4_8(iSht, false))     //移到 Shuttle 上等待
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
            if(MoveOutArmToShuttleIncludeZ_9045_2x4_8(iSht, true))              //kevin 20210717 change function
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
            for(int i=0; i<OutArmSuck.iPickRow; i++)
            {
                for(int j=0; j<OutArmSuck.iPickCol; j++)
                {
                    iSuckRow  =i;
                    iSuckCol  =j;
                    iShtRow   =i;
                    iShtCol   =GetShuttleCol(i, j);
                    if(ptrOutSHT->Item[iShtRow][iShtCol] &&
                       OutArmSuck.Suck[iSuckRow][iSuckCol].GetNeedSuckStatus())
                    {
                        if(OutArmSuck.Suck[iSuckRow][iSuckCol].Suck())
                        {
                            //模擬ic不見了
                            #ifdef SOFT_SIMULTE
                            if(fMain->chkInPickLoadError->Checked==true)
                            {
                                if(j==1)
                                {
                                    OutArmSuck.Suck[iSuckRow][iSuckCol].Error=true;
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

            if(flag==false)                                                                                                                                     //KEVIN 20130421 按RETRY資料尚未交換會將其他IC丟掉
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

            if(USE_Fix_AI_CCD==true && TestIF_File.bEnableFix2BGAAICCD==true)                                                                                   //KaiChen 20190124 ：矽格湖口 AI CCD Function
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
                        for(int i=0; i<OutArmSuck.iPickRow; i++)
                        {
                            for(int j=0; j<OutArmSuck.iPickCol; j++)
                            {
                                if(OutArmSuck.Suck[i][j].Error)
                                {
                                    iShtCol=GetShuttleCol(i, j);
                                    PorcessJAM0201OutArmPickUpErrorSkip(iSht, i, iShtCol, i, j);                        //Steven 20161214 : 加上OutArm吸取異常Skip的ErrorLog
                                }
                                bOutSuckShtDupErr[i][j]=false;
                            }
                        }

                        if(USE_Fix_AI_CCD==true &&                                                                      //KaiChen 20190124 ：矽格湖口 AI CCD Function
                           TestIF_File.bEnableFix2BGAAICCD==true)
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
// Provenance: golden aoutarm9045_2x4_8.cpp:1047 (file-local 9046AU helper)
bool DoPlaceToSortShuttle_2x4_8_AU()                                            //RogerYang 20250514 Add for 9046AU
{
//    int ret;
    int &Task=iPlaceToSortShtTask;
//    static int iWitchErrBin=0;
    AnsiString ErrPart="";
//    bool bHasDuplicateErr=false;
//    bool bHasErr=false;
    bool flag=false;
    int iSuckRow, iSuckCol, iShtRow, iShtCol;
    ptrOutSHT=&OutSht3Kit;

    switch(Task)
    {
        case 1:
//            if(ArmSpeed[OutArm].dWaitOnSH!=0)                                   //out arm 起動在shuttle 上等待 時間
//            {
//                if(OutArmSuck.NoIC())                                           //第一次 吸取IC 需等待
//                {
//                    if(MoveOutArmToShuttleIncludeZ_9045_2x4_8(2, false))        //移到 Shuttle 上等待
//                    {
//                        PickFromShuttle(0);                                     //add int iwhichKit=0
//                        Task=2;
//                    }
//                }
//                else
//                {
//                    Task=10;
//                }
//            }
//            else
//            {
                Task=10;
//            }
            break;
//        case 2:
//            if(bOutShtwaitPick())                                               //kevin 20210602 Out arm sht on wait
//                Task=10;
//            break;
        case 10:
//            iWitchErrBin=0;

            Task=200;
        case 200:
            if(MoveOutArmToShuttleIncludeZ_9045_2x4_8(2, true))                 //kevin 20210717 change function
            {
//                if(OutArmNeedCheckOffset(false, iSht))                          //Steven 20230531 : 簡化判斷式
//                {
//                    Task=500;
//                    break;
//                }
//                OutArmSuck.ResetAll();                                          //Steven 20160323 : 避免未開啟真空
                Task=1000;
            }
            break;
//        case 500:
//            if(bEnterOffset)
//            {
//                bEnterOffset=false;
//                bOutArmManualStepPress=true;
//                Task=1;
//                return false;
//            }
//            else
//            {
//                Task=iOUTARM_SUCK;
//                OutArmSuck.ResetAll();                                          //Steven 20160323 : 避免未開啟真空
//            }
//            break;
        case 1000:
            flag=true;
//            bHasErr=false;
            for(int i=0; i<OutArmSuck.iPickRow; i++)
            {
                for(int j=0; j<OutArmSuck.iPickCol; j++)
                {
                    iSuckRow  =i;
                    iSuckCol  =j;
                    iShtRow   =i;
                    iShtCol   =GetShuttleCol(i, j);
                    if(ptrOutSHT->Item[iShtRow][iShtCol]==NULL_IC &&
                       OutArmSuck.Suck[iSuckRow][iSuckCol].GetNeedDestroyStatus())
                    {
                        if(OutArmSuck.Item[iSuckRow][iSuckCol]==HAS_NULL_IC ||
                           OutArmSuck.Suck[iSuckRow][iSuckCol].Destroy())
                        {
                            SetSortShuttleStatus_Place(iShtRow, iShtCol, iSuckRow, iSuckCol);
                        }
                        else if(OutArmSuck.Suck[iSuckRow][iSuckCol].Error==false)
                        {
                            flag=false;
                        }
                    }
                }
            }

//            if(bHasErr)
//            {
//                ShowOutputShuttleDataMiss();
//            }

            if(flag==false)                                                     //KEVIN 20130421 按RETRY資料尚未交換會將其他IC丟掉
                break;

            for(int i=0; i<OutArmSuck.iPickRow; i++)
            {
                for(int j=0; j<OutArmSuck.iPickCol; j++)
                    if(OutArmSuck.Item[i][j]!=NULL_IC && OutArmSuck.Suck[i][j].GetNeedDestroyStatus())
                        return false;
            }
            iOutShtRetryCount=0;
            Task=1;
            return true;
    }
    return false;
}
//==============================================================================
// Provenance: golden aoutarm9045_2x4_8.cpp:1164
void DoOutArm_9045_2x4_8()
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
                if(USE_OUT_SORT_ARM!=eartUninstall)                             //RogerYang 20250513 add for 9046AU
                {
                    MOT[MOutSortSht].fCanMoveR=true;
                }
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
                if(bUseTwoArm32Site==true)
                {
                    if(TestIF_File.iShuttleMode==1)                             //Jou 20150909 : For Add 32Site UPH, 讓Shuttle 2先回去補Input
                    {
                        if(FRCarryKit.UseSiteHasIC() && OutSHT1InRT())
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
                    else
                    {
                        if((BRCarryKit.UseSiteHasIC()) && OutSHT2InRT())
                        {
                            OldPos=0;
                            FirstEnter=false;
                        }
                        else if(FRCarryKit.UseSiteHasIC() && OutSHT1InRT())     //jou 20171227 : 修正32 sites hang up
                        {
                            OldPos=1;
                            FirstEnter=false;
                        }
                    }
                }
                else
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
            if(TestIF.iTestMode==_16Site4X4)                                    //Jou 20150909 : For Add 32Site UPH, 讓Shuttle 2先回去補Input   0-->1
            {
                if(MoveOutArmToShuttleIncludeZ_9045_2x4_8(1, false))            //Steven 20230704 : 修正NN mode動作異常
                    Task=CheckOutArmCleanOut();
            }
            else if(MoveOutArmXY_ToFix_Tray_Full())                             //Steven 20230602 : 統一out arm 讓位位置
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
            if(USE_OUT_Y_IS_AUTO_PITCH==true &&                                 //JerryYang 20251218 : IN/OUT ARM支援不同模組
               IniConfig.bE57YPitchHome)                                        //kevin 20180827 (Steven) : add put shuttle Y pitch home AutoYPitch
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

            if(MoveOutArmToShuttleIncludeZ_9045_2x4_8(iWhichShuttle))
            {
                IsCatchTrayReadySupplyNewTray();

                if(OutArmSuck.iXStep!=1 || OutArmSuck.iYStep!=1)
                {
                    iBackMode1=GetNowShuttleMode_2x4_8(0);
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

                if(OutArmSuck.iXStep!=1 || OutArmSuck.iYStep!=1)
                {
                    iMode1=GetNowShuttleMode_2x4_8(0);
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

            if(DoPickFromShuttle_9045_2x4_8(0))
            {
                PreSetOutAdditionalFlag();                                                                              //Steven 20220614 : Fixed for out arm additional function
                FRCarryKit.SetUnuseAndHasNullICToNullIC();
                if(FRCarryKit.UseSiteHasIC())
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

            if(MoveOutArmToShuttleIncludeZ_9045_2x4_8(iWhichShuttle))
            {
                IsCatchTrayReadySupplyNewTray();

                if(OutArmSuck.iXStep!=1 || OutArmSuck.iYStep!=1)
                {
                    iBackMode2=GetNowShuttleMode_2x4_8(1);
                }
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

                if(OutArmSuck.iXStep!=1 || OutArmSuck.iYStep!=1)
                {
                    iMode2=GetNowShuttleMode_2x4_8(1);
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

            if(DoPickFromShuttle_9045_2x4_8(1))
            {
                PreSetOutAdditionalFlag();                                                                              //Steven 20220614 : Fixed for out arm additional function
                BRCarryKit.SetUnuseAndHasNullICToNullIC();
                if(BRCarryKit.UseSiteHasIC())
                {
                    InitPickFromShuttle2Task();                                                                         //Sam 20230208 : 修正 Fix AOI 未拍照問題
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

                if(TestIF.iTestMode==_16Site4X4)                                                                        //Jou 20150909 : For Add 32Site UPH, 讓Shuttle 2先回去補Input
                {
                    if(TestIF_File.iShuttleMode==0)                                                                     //Jou 20150909 : For Add 32Site UPH, 讓Shuttle 2先回去補Input
                    {
                        if(FRCarryKit.UseSiteNoIC())
                            MOT[MInShuttle1].fCanMoveR=true;

                        if(BRCarryKit.UseSiteNoIC())
                            MOT[MInShuttle2].fCanMoveR=true;
                    }
                    else
                    {
                        MOT[MInShuttle1].fCanMoveR=true;
                        MOT[MInShuttle2].fCanMoveR=true;
                    }
                }
                else
                {
                    MOT[MInShuttle1].fCanMoveR=true;
                    MOT[MInShuttle2].fCanMoveR=true;
                    if(USE_OUT_SORT_ARM!=eartUninstall)                                                                 //RogerYang 20250513 add for 9046AU
                    {
                        MOT[MOutSortSht].fCanMoveR=true;
                    }
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
            if(USE_OUT_SORT_ARM!=eartUninstall && NeedPlaceToSort())                                                    //RogerYang 20250513 add for 9046AU
            {
                Task=14000;
                break;
            }
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
                if(Task==100 && (iInArmType==e9045_2x4_4_13 || iInArmType==e9045_2x4_4_14))                             //KevinCheng 20260118
                {
                    if(iWhichShuttle==0)
                    {
                        if(iOutArmiWhichKit==0 && FRCarryKit.RightSideNoIC(2)==false)
                        {
                            Task=1000;
                            return;
                        }
                    }
                    else
                    {
                        if(iOutArmiWhichKit==0 && BRCarryKit.RightSideNoIC(2)==false)
                        {
                            Task=2000;
                            return;
                        }
                    }
                }
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
        case 14000:                                                                                                     //RogerYang 20250513 add for 9046AU
            if(NeedPlaceToSort() && OutSht3Kit.UseSiteNoIC()==true)                                                     //如果有Auto4 Auto5 Auto6 要先去放SortShuttle
            {
                InitPlaceToSortShtTask();
                Task=14100;
            }
            else
                Task=3010;                                                                                              //如果沒有就回去3100
            break;
        case 14100:                                                                                                     //RogerYang 20250513 add for 9046AU
            if(SortSHTInRT())                                                                                           //等SortShttle在右邊
            {
                MOT[MOutSortSht].fCanMoveL=false;
                if(DoPlaceToSortShuttle_2x4_8_AU())
                {
                    Task=3010;
                    MOT[MOutSortSht].fCanMoveL=true;
                }
            }
            break;
    }
}
//---------------------------------------------------------------------------
// Provenance: golden aoutarm9045_2x4_8.cpp:2110 (file-local 9046AU helper)
bool MoveSortArmToShuttleIncludeZ_AU_2x4_8(bool bZDown)                         //RogerYang 20250517 Add for 9046AU
{
    bool bZFlag[MAX_ARM_Row][MAX_ARM_Col];
    int iZPos[MAX_ARM_Row][MAX_ARM_Col];
    int iXVariable;
    int iXPos               =iSortArmShtXCenterPos;
    int iYPos               =iSortArmShtYCenterPos;
//    int iMode               =GetNowSortShuttleMode_2x4_8();
//    int iModeRow            =iMode%100;
//    int iModeCol            =iMode/100;
    int iMovePitchX         =iXpitchMax;
//    int iMovePitchY         =0;
    int iOffsetPos          =GetSortArmToSortShuttleOffset();
//    int iYVariable          =0;
    ZeroMemory(bZFlag,      sizeof(bZFlag));
    ZeroMemory(iZPos,       sizeof(iZPos));

    iXVariable=GetSortArmPitchX(iMovePitchX, iOffsetPos);

//    if(OutArmSuck.iXStep==1)
//        iMovePitchX=TestIF.dSiteXPitch*3;
//    else if(OutArmSuck.iXStep==2)
//        iMovePitchX=TestIF.dSiteXPitch/2*3;
//    else
//        iMovePitchX=iXpitchMaxX3;

    //if(CheckOutArmXYPitch_2x4_8(&iXPos, &iYPos, iSht, iMovePitchX, iMovePitchY)==false && bZDown==true)
    if(CheckSortArmXYPitch_2x4_8(&iXPos, &iYPos, iMovePitchX)==false && bZDown==true)
        return false;

    GetSortShuttleStatus_Pick(iZPos, bZDown, bZFlag);
    if(IsCheckSortArmDestroyActiveFinish(iXPos, iYPos)==false)
        return false;

    //DoOutArmSuckPreOn(iSht, 0, iXPos, iYPos, bZDown);
    DoSortArmSuckPreOn(0, iXPos, iYPos, bZDown);
    if(SortArmContinuousMove(iXPos, iYPos, iXVariable, bZFlag, iZPos, bZDown))
        return true;

    return false;
}
//---------------------------------------------------------------------------
