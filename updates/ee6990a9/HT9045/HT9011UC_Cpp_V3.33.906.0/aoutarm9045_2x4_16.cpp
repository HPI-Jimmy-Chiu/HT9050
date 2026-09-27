// =============================================================================
//  aoutarm9045_2x4_16.cpp  --  Out-Arm per-site VARIANT (2x4_16, 16-site / 8-col)
//
//  Faithful translation of golden aoutarm9045_2x4_16.cpp (BCB6, Big5/cp950).
//  Translator: AI(W6.2c-OUT-2x4_16) 20260627
//  Translation wave: W6.2c-OUT batch-4 (DEAD out-arm per-site-config variant).
//
//  ROLE: the per-site out-arm shuttle-pick / place-to-auto state machines for the
//  2x4_16 layout.  This file owns (golden names VERBATIM):
//    GetNowShuttleMode_2x4_16, OutArmZNeedDown_2x4_16, CheckOutArmXYPitch_2x4_16,
//    MoveOutArmToShuttleIncludeZ_9045_2x4_16, DoPickFromShuttle_9045_2x4_16,
//    DoOutArm_9045_2x4_16.
//
//  DEAD-BY-DESIGN (MODE=dead): the enum e9045_2x4_16 is ABSENT from MachineType.h,
//  so NEITHER out-arm dispatch ladder (DoOutArm_9045 / DoPickFromShuttle_9045 in
//  the engine aoutarm9045.cpp) has an arm pointing at DoOutArm_9045_2x4_16 /
//  DoPickFromShuttle_9045_2x4_16.  Every _2x4_16-suffixed symbol here is therefore
//  a UNIQUE never-called symbol -- it links as dead weight (exactly like the
//  in-arm dead variant ainarm9045_2x4_16.cpp).  Integration = add this file to the
//  CMake ht9045_sm library ONLY; NO static-stub removal, NO extern un-gate, NO
//  dispatch-ladder edit to aoutarm9045.cpp.  The .h mirrors golden EXACTLY: it
//  declares ONLY DoOutArm_9045_2x4_16() + MoveOutArmToShuttleIncludeZ_9045_2x4_16
//  (NO DoPickFromShuttle_9045_2x4_16 -- that stays file-internal, called only from
//  this file's own DoOutArm).
//
//  FAITHFUL: every fn/var/cursor value, switch(Task) structure, formula, branch,
//  fall-through and commented-out code is transcribed VERBATIM from the golden
//  CP950 source.  The DoOutArm_9045_2x4_16 case-1->5, case-5->10, case-10->50,
//  case-50->100, case-1140->1150, case-2040->2050, case-3100->3300,
//  case-3300->3301, case-3301->3500 fall-throughs are preserved EXACTLY.  The
//  DoPickFromShuttle_9045_2x4_16 case-10->200 fall-through is preserved EXACTLY.
//
//  VCL/Borland conversions: VCL AnsiString (1-based) via vclcompat; this file has
//  no __fastcall/__property/__published/PACKAGE.  SOFT_SIMULTE is NOT defined ->
//  the one #ifdef SOFT_SIMULTE block in DoPickFromShuttle (case iOUTARM_SUCK,
//  chkInPickLoadError test) is compiled OUT, kept VERBATIM.
//
//  DEAD-BY-DESIGN table: golden references XPHSuckToSht_2x4_16[iModeX][j]
//  (suck-col -> shuttle-col map).  In golden it is a COMMENTED-OUT extern at
//  cmydef.h:3123 with NO initializer anywhere -> the variant never compiled/linked
//  as-is in the BCB6 tree.  To stay FAITHFUL (keep the source text + logic) AND
//  honest (do NOT invent it on the shared cmydef), it is given a file-LOCAL identity
//  map of the golden-cited dims [3][8] -- offline-safe & in-bounds.  Mirrors the
//  committed in-arm sibling ainarm9045_2x4_16.cpp:86.
//
//  Big5: every Chinese comment decoded via cp950 and preserved as UTF-8.
//  Final gate: ZERO U+FFFD.
//
//  INCLUDE REWRITE (mirror of the golden header set, translated subset):
//    golden: MachineDefine.h ; aoutarm9045_2x4_16.h ; aArmHeader.h ; MyKitSuck.h ;
//            cprod.h ; mymotor.h ; mycylin.h ; cSortCT.h ; mysensor.h ; main.h ;
//            note.h ; csystem.h ; acatchtray.h ; cinitial.h ; atester.h ;
//            aRotateKIT.h ; fRotate.h ; cOffSet.h ; fAOI.h ; cmydef.h ;
//            ProductionInfo.h ; AutoTeach.h ; FixAICCD.h ; Magazine.h ;
//            #pragma package(smart_init)
// =============================================================================
#include "MachineDefine.h"
#include "MachineType.h"            // e9045_* / ep16Picker / eBulkBox / eAboveCoveyor / eUnderCoveyor / CC_KYEC_LEE / global enums
//---------------------------------------------------------------------------
#include "aoutarm9045_2x4_16.h"
#include "aArmHeader.h"             // __FUNC__ shim, RecordProcess, OutArm, etc.
#include "aoutarm9045.h"            // engine: GetOutArmPitch*_9045 / DoOutArmSuckPreOn / IsCheckOutArmDestroyActiveFinish / GetOutShuttleStatus_9045 / InitialOutArmNeedSuck / OutArmNeedCheckOffset / iOUTARM_SUCK / SearchTrayToPlace_9045 / VerifyFixTrayLink / DoOutArmPlaceToAuto_9045 / CheekNeedToDoOutArmAdditionalFunction / MoveOutArmXY_To_ESDSafePos / DoOutArmAdditionalFunction / DoOutArmAfterPlaceToAuto / InitPlaceToAutoTask / MoveOutArmToAutoSafe_9045
#include "csystem.h"                // OutSHT1InRT / OutSHT2InRT / HasICUnderMachine / HasICUnderHotPlate / IsOutArmCleanOutFinish / AutoTrayReCheck / CheckOutArmSuckICFallDown
#include "Motor/mymotor.h"          // MOT[], OutArmContinuousMove_9045
#include "cmydef.h"                 // global scalar universe + IC consts + iInArmType + enums + bEnterOffset / bCheckShuttle1/2Flag / iCleanOut / IniConfig / USE_* / iXpitch* / Mxxx motor ids / iAutoIndex / iMMAuto / iWhichBuff / iWhichMag / magazine globals / ...
#include "cprod.h"                  // Prod / TestIF / TestIF_File / ArmSpeed / OutArmOffSet / iXTrayColor
#include "cpublic.h"                // CosFunction
#include "common.h"
#include "mysensor.h"               // Sen[] (SnFixedTrayDetect)
#include "acatchtray.h"             // WhichAutoNeedTray
#include "aHotPlateSubstrate.h"     // OutArmSuck / FRCarryKit / BRCarryKit / FTestSuck / BTestSuck / InArmSuck + cursors + SetMotorSpeed + iWhichBuff / iWhichMag
#include "acarry_shims.h"           // MoveOutArmToAutoSafe / MoveOutArmXY_ToShuttleAlarmArea / LastSet / NULL_IC
#include "acatchtray_shims.h"       // InitProcessSingleMotorTask / iWhichAuto
#include "atester_shims.h"          // fObserver / fContact / fFixAICCD offline stand-ins
#include "ainarm9045_2x4_16_shims.h"// AutoTeachLoadTrayZ
#include "FormsFacade.h"            // fMain offline stand-in (chkInPickLoadError gated out by SOFT_SIMULTE)
#include "canary_support.h"         // ShowErrorMessage / ShowMyMessage / RecordProcess / K_RETRY / K_SKIP / K_HOME / DUMMY / __FUNC__
//---------------------------------------------------------------------------
#pragma package(smart_init)

//==============================================================================
//==  STANDALONE forward-declares for cross-module / engine-file-local symbols
//==  that have NO header home in scope this wave.  Guarded with #ifndef so the
//==  serial Integrate agent can add the real shared decl ONCE without clashing.
//==  Each is REPORTED in substrateNeeds (name + golden home + offline behavior).
//==============================================================================
#ifndef OUTARM_2x4_16_FWD_GUARD
#define OUTARM_2x4_16_FWD_GUARD
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
// out-arm pick-shuttle error flag (golden file-global)
extern bool bPickShuttleError;                                                 // golden aoutarm9045.cpp -- offline: false
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
#endif // OUTARM_2x4_16_FWD_GUARD

// =============================================================================
//  File-local DEAD-BY-DESIGN table (golden cmydef.h:3123 commented-out extern
//  `//extern const int XPHSuckToSht_2x4_16[3][8];` -- NO initializer exists in
//  golden anywhere).  Dims are the golden-cited shape [3][8].  The only golden
//  hint is the variant's own usage (XPHSuckToSht_2x4_16[iModeX][j] maps suck-col
//  -> shuttle-col), so we use the identity map for every row (offline-safe,
//  in-bounds).  Variant-LOCAL (NOT added to the shared cmydef) per the wave
//  directive.  Mirrors the committed in-arm sibling ainarm9045_2x4_16.cpp:86.
// =============================================================================
#ifndef HT9045_XPH_OUT_2x4_16_LOCAL
#define HT9045_XPH_OUT_2x4_16_LOCAL
static const int XPHSuckToSht_2x4_16[3][8] =
{
    {0, 1, 2, 3, 4, 5, 6, 7},
    {0, 1, 2, 3, 4, 5, 6, 7},
    {0, 1, 2, 3, 4, 5, 6, 7},
};
#endif

//==============================================================================
//  GetNowShuttleMode_2x4_16  -- golden aoutarm9045_2x4_16.cpp:36
//==============================================================================
//個位數為Y座標, 2=標準, 0=A排, 1=B排
//百位數以上為X座標, 分配方式如下
//                    0                     =標準
//         100        |         101         =左右分兩次
//    200       201   |    202       203    =左右分四次
int GetNowShuttleMode_2x4_16(int iSht)
{
    ptrOutSHT=(iSht==0)?&FRCarryKit:&BRCarryKit;

    if(OutArmSuck.iXStep==1 && OutArmSuck.iYStep==1)
    {
        return (iSht==0)?102:202;
    }
    else if(ptrOutSHT->HasIC()==false)
    {
        if(OutArmSuck.iXStep==1)
        {
            return (iSht==0)?100:200;
        }
        else if(OutArmSuck.iXStep==2)
        {
            if(OutArmSuck.iYStep==1)
                return (iSht==0)?10002:10202;                                   //左邊八顆
            else
                return (iSht==0)?10000:10200;                                   //左上四顆
        }
        else                                                                    //if(OutArmSuck.iXStep==4)
        {
            if(OutArmSuck.iYStep==1)
                return (iSht==0)?20002:20402;                                   //左邊四顆
            else
                return (iSht==0)?20000:20400;                                   //左上兩顆
        }
    }
    else
    {
        if(OutArmSuck.iXStep==1)
        {
            for(int j=0; j<OutArmSuck.iPickCol; j++)
            {
                if(ptrOutSHT->Item[0][j]>=HAS_IC)
                    return (iSht==0)?100:200;                                   //A排四顆
            }
            return (iSht==0)?101:201;                                           //B排四顆
        }
        else if(OutArmSuck.iXStep==2)
        {
            if(OutArmSuck.iYStep==1)
            {
                if(ptrOutSHT->Item[0][0]>=HAS_IC || ptrOutSHT->Item[0][1]>=HAS_IC ||
                   ptrOutSHT->Item[1][0]>=HAS_IC || ptrOutSHT->Item[1][1]>=HAS_IC)
                    return (iSht==0)?10002:10202;                               //左邊四顆
                else                                                            //if(ptrOutSHT->[0][2]>=HAS_IC || ptrOutSHT->[0][3]>=HAS_IC ||
                     //   ptrOutSHT->[1][2]>=HAS_IC || ptrOutSHT->[1][3]>=HAS_IC)
                    return (iSht==0)?11002:11202;                               //右邊四顆
            }
            else
            {
                if(ptrOutSHT->Item[0][0]>=HAS_IC || ptrOutSHT->Item[0][1]>=HAS_IC)
                    return (iSht==0)?10000:10200;                               //左上兩顆
                else if(ptrOutSHT->Item[1][0]>=HAS_IC || ptrOutSHT->Item[1][1]>=HAS_IC)
                    return (iSht==0)?10001:10201;                               //左下兩顆
                else if(ptrOutSHT->Item[0][2]>=HAS_IC || ptrOutSHT->Item[0][2]>=HAS_IC)
                    return (iSht==0)?11000:11200;                               //右上兩顆
                else                                                            //if(ptrOutSHT->Item[1][3]>=HAS_IC || ptrOutSHT->Item[1][3]>=HAS_IC)
                    return (iSht==0)?11001:11201;                               //右下兩顆
            }
        }
        else                                                                    //if(OutArmSuck.iXStep==4)
        {
            if(OutArmSuck.iYStep==1)
            {
                if(ptrOutSHT->Item[0][0]>=HAS_IC ||
                   ptrOutSHT->Item[1][0]>=HAS_IC)
                    return (iSht==0)?20002:20402;
                else if(ptrOutSHT->Item[0][1]>=HAS_IC ||
                        ptrOutSHT->Item[1][1]>=HAS_IC)
                    return (iSht==0)?20102:20502;
                else if(ptrOutSHT->Item[0][2]>=HAS_IC ||
                        ptrOutSHT->Item[1][2]>=HAS_IC)
                    return (iSht==0)?21002:21402;
                else                                                            //if(ptrOutSHT->Item[0][3]>=HAS_IC ||
                     //   ptrOutSHT->Item[1][3]>=HAS_IC)
                    return (iSht==0)?21102:21502;
            }
            else
            {
                if(ptrOutSHT->Item[0][0]>=HAS_IC)
                    return (iSht==0)?20000:20400;
                else if(ptrOutSHT->Item[1][0]>=HAS_IC)
                    return (iSht==0)?20001:20401;
                else if(ptrOutSHT->Item[0][1]>=HAS_IC)
                    return (iSht==0)?20100:20500;
                else if(ptrOutSHT->Item[1][1]>=HAS_IC)
                    return (iSht==0)?20101:20501;
                else if(ptrOutSHT->Item[0][2]>=HAS_IC)
                    return (iSht==0)?21000:21400;
                else if(ptrOutSHT->Item[1][2]>=HAS_IC)
                    return (iSht==0)?21001:21401;
                else if(ptrOutSHT->Item[0][3]>=HAS_IC)
                    return (iSht==0)?21100:21500;
                else                                                            //if(ptrOutSHT->Item[1][3]>=HAS_IC)
                    return (iSht==0)?21101:21501;
            }
        }
    }
}
//==============================================================================
//  OutArmZNeedDown_2x4_16  -- golden aoutarm9045_2x4_16.cpp:139
//==============================================================================
bool OutArmZNeedDown_2x4_16(int iSht)
{
    if(InitialOutArmNeedSuck(iSht)==false)
        return false;

    ptrOutSHT   =(iSht==0)?&FRCarryKit:&BRCarryKit;
    int iMode   =GetNowShuttleMode_2x4_16(iSht);
    int iModeRow=iMode%100;
    int iModeCol=iMode/100;
    int iShtJ=0;

    for(int i=0; i<OutArmSuck.iPickRow; i++)
    {
        for(int j=0; j<OutArmSuck.iPickCol; j++)
        {
            iShtJ=XPHSuckToSht_2x4_16[InArmSuck.iModeX][j];
            if(iModeCol==1 && (j==0 || j==2 || j==4 || j==6) &&                 //Shuttle 1
               (iModeRow==2 || iModeRow==i))
            {
                SetOutArmNeedDestory(i, j, i, iShtJ);
            }
            else if(iModeCol==2 && (j==1 || j==3 || j==5 || j==7) &&            //Shuttle 2
                    (iModeRow==2 || iModeRow==i))
            {
                SetOutArmNeedDestory(i, j, i, iShtJ);
            }
            else if(iModeCol==100 && (j==0 || j==4) &&                          //吸嘴             ==>  蝦頭 1
                    (iModeRow==2 || iModeRow==i))                               // O X X X O X X X      O O X X
            {                                                                   // O X X X O X X X      O O X X
                SetOutArmNeedDestory(i, j, i, iShtJ);
            }
            else if(iModeCol==110 && (j==2 || j==6) &&                          //吸嘴             ==>  蝦頭 1
                    (iModeRow==2 || iModeRow==i))                               // X X O X X X O X      X X O O
            {                                                                   // X X O X X X O X      X X O O
                SetOutArmNeedDestory(i, j, i, iShtJ);
            }
            else if(iModeCol==102 && (j==1 || j==5) &&                          //吸嘴             ==>  蝦頭 2
                    (iModeRow==2 || iModeRow==i))                               // X O X X X O X X      O O X X
            {                                                                   // X O X X X O X X      O O X X
                SetOutArmNeedDestory(i, j, i, iShtJ);
            }
            else if(iModeCol==112 && (j==3 || j==7) &&                          //吸嘴             ==>  蝦頭 2
                    (iModeRow==2 || iModeRow==i))                               // X X X O X X X O      X X O O
            {                                                                   // X X X O X X X O      X X O O
                SetOutArmNeedDestory(i, j, i, iShtJ);
            }
            else if(iModeCol==200 && j==0 &&                                    //吸嘴             ==>  蝦頭 1
                    (iModeRow==2 || iModeRow==i))                               // O X X X X X X X      O X X X
            {                                                                   // O X X X X X X X      O X X X
                SetOutArmNeedDestory(i, j, i, iShtJ);
            }
            else if(iModeCol==201 && j==2 &&                                    //吸嘴             ==>  蝦頭 1
                    (iModeRow==2 || iModeRow==i))                               // X X O X X X X X      X O X X
            {                                                                   // X X O X X X X X      X O X X
                SetOutArmNeedDestory(i, j, i, iShtJ);
            }
            else if(iModeCol==210 && j==4 &&                                    //吸嘴             ==>  蝦頭 1
                    (iModeRow==2 || iModeRow==i))                               // X X X X O X X X      X X O X
            {                                                                   // X X X X O X X X      X X O X
                SetOutArmNeedDestory(i, j, i, iShtJ);
            }
            else if(iModeCol==211 && j==6 &&                                    //吸嘴             ==>  蝦頭 1
                    (iModeRow==2 || iModeRow==i))                               // X X X X X X O X      X X X O
            {                                                                   // X X X X X X O X      X X X O
                SetOutArmNeedDestory(i, j, i, iShtJ);
            }
            else if(iModeCol==204 && j==1 &&                                    //吸嘴             ==>  蝦頭 2
                    (iModeRow==2 || iModeRow==i))                               // X O X X X X X X      O X X X
            {                                                                   // X O X X X X X X      O X X X
                SetOutArmNeedDestory(i, j, i, iShtJ);
            }
            else if(iModeCol==205 && j==3 &&                                    //吸嘴             ==>  蝦頭 2
                    (iModeRow==2 || iModeRow==i))                               // X X X O X X X X      X O X X
            {                                                                   // X X X O X X X X      X O X X
                SetOutArmNeedDestory(i, j, i, iShtJ);
            }
            else if(iModeCol==214 && j==5 &&                                    //吸嘴             ==>  蝦頭 2
                    (iModeRow==2 || iModeRow==i))                               // X X X X X O X X      X X O X
            {                                                                   // X X X X X O X X      X X O X
                SetOutArmNeedDestory(i, j, i, iShtJ);
            }
            else if(iModeCol==215 && j==7 &&                                    //吸嘴             ==>  蝦頭 2
                    (iModeRow==2 || iModeRow==i))                               // X X X X X X X O      X X X O
            {                                                                   // X X X X X X X O      X X X O
                SetOutArmNeedDestory(i, j, i, iShtJ);
            }
        }
    }
    return true;
}
//------------------------------------------------------------------------------
//  CheckOutArmXYPitch_2x4_16  -- golden aoutarm9045_2x4_16.cpp:230
//------------------------------------------------------------------------------
bool CheckOutArmXYPitch_2x4_16(int *iX, int *iY, int iSht, int iMovePitchX, int iMovePitchY)
{
    int iMode       =GetNowShuttleMode_2x4_16(iSht);
    int iModeRow    =iMode%100;
    int iModeCol    =iMode/100;
    int iOffsetPos  =GetOutArmToShuttleOffset_9045(iSht, iModeRow, iModeCol, false);
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
        if(bUseTwoArm32Site &&
           TestIF_File.iYPitchOffsetMode==0)                                    //向舊版本相容
        {
            if(USE_Y_AUTO_PITCH==true)
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
            if(USE_Y_AUTO_PITCH==true)
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
            if(USE_Y_AUTO_PITCH==true)
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
            if(USE_Y_AUTO_PITCH==true)
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
            if(USE_Y_AUTO_PITCH==true)
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
            if(USE_Y_AUTO_PITCH==true)
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
            if(USE_Y_AUTO_PITCH==true)
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
            if(USE_Y_AUTO_PITCH==true)
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
            if(USE_Y_AUTO_PITCH==true)
                *iY=*iY-TestIF.dSiteYPitch/2;
            else
                *iY=*iY+TestIF.dSiteYPitch/2;
        }
    }

    dMovePitchX=double(iMovePitchX)/7.0;
    if(iModeCol==1)                                                             //Aa --> Aa
    {
        *iX=*iX+(dMovePitchX*3.0-TestIF.dSiteXPitch*1.5);
    }
    else if(iModeCol==2)                                                        //Ab --> Aa
    {
        *iX=*iX+(dMovePitchX*2.0-TestIF.dSiteXPitch*1.5);
    }
    else if(iModeCol==100)                                                      //Aa --> Aa
    {
        if(TestIF_File.bEnableUseXCenterPitch==true)
            *iX=*iX+(dMovePitchX*3.0-TestIF.dSiteXCenterPitch*0.5-TestIF.dSiteXPitch*1.0);
        else
            *iX=*iX+(dMovePitchX*3.0-TestIF.dSiteXPitch*1.5);
    }
    else if(iModeCol==110)                                                      //Ac --> Ac
    {
        if(TestIF_File.bEnableUseXCenterPitch==true)
            *iX=*iX+(dMovePitchX*1.0+TestIF.dSiteXCenterPitch*0.5);
        else
            *iX=*iX+(dMovePitchX*1.0+TestIF.dSiteXPitch*0.5);
    }
    else if(iModeCol==102)                                                      //Ab --> Aa
    {
        if(TestIF_File.bEnableUseXCenterPitch==true)
            *iX=*iX+(dMovePitchX*2.0-TestIF.dSiteXCenterPitch*0.5-TestIF.dSiteXPitch*1.0);
        else
            *iX=*iX+(dMovePitchX*2.0-TestIF.dSiteXPitch*1.5);
    }
    else if(iModeCol==112)                                                      //Ad --> Ac
    {
        if(TestIF_File.bEnableUseXCenterPitch==true)
            *iX=*iX+(dMovePitchX*0.0+TestIF.dSiteXCenterPitch*0.5);
        else
            *iX=*iX+(dMovePitchX*0.0+TestIF.dSiteXPitch*0.5);
    }
    else if(iModeCol==200)                                                      //Aa --> Aa
    {
        if(TestIF_File.bEnableUseXCenterPitch==true)
            *iX=*iX+(dMovePitchX*3.0-TestIF.dSiteXCenterPitch*0.5-TestIF.dSiteXPitch*1.0);
        else
            *iX=*iX+(dMovePitchX*3.0-TestIF.dSiteXPitch*1.5);
    }
    else if(iModeCol==201)                                                      //Ac --> Ab
    {
        if(TestIF_File.bEnableUseXCenterPitch==true)
            *iX=*iX+(dMovePitchX*1.0-TestIF.dSiteXCenterPitch*0.5);
        else
            *iX=*iX+(dMovePitchX*1.0-TestIF.dSiteXPitch*0.5);
    }
    else if(iModeCol==210)                                                      //Ae --> Ac
    {
        if(TestIF_File.bEnableUseXCenterPitch==true)
            *iX=*iX+(-dMovePitchX*1.0+TestIF.dSiteXCenterPitch*0.5);
        else
            *iX=*iX+(-dMovePitchX*1.0+TestIF.dSiteXPitch*0.5);
    }
    else if(iModeCol==211)                                                      //Ag --> Ad
    {
        if(TestIF_File.bEnableUseXCenterPitch==true)
            *iX=*iX+(-dMovePitchX*3.0+TestIF.dSiteXCenterPitch*0.5+TestIF.dSiteXPitch*0.5);
        else
            *iX=*iX+(-dMovePitchX*3.0+TestIF.dSiteXPitch*1.5);
    }
    else if(iModeCol==204)                                                      //Ab --> Aa
    {
        if(TestIF_File.bEnableUseXCenterPitch==true)
            *iX=*iX+(dMovePitchX*2.0-TestIF.dSiteXCenterPitch*0.5-TestIF.dSiteXPitch*1.0);
        else
            *iX=*iX+(dMovePitchX*2.0-TestIF.dSiteXPitch*1.5);
    }
    else if(iModeCol==205)                                                      //Ad --> Ab
    {
        if(TestIF_File.bEnableUseXCenterPitch==true)
            *iX=*iX+(dMovePitchX*0.0-TestIF.dSiteXCenterPitch*0.5);
        else
            *iX=*iX+(dMovePitchX*0.0-TestIF.dSiteXPitch*0.5);
    }
    else if(iModeCol==214)                                                      //Af --> Ac
    {
        if(TestIF_File.bEnableUseXCenterPitch==true)
            *iX=*iX+(-dMovePitchX*2.0+TestIF.dSiteXCenterPitch*0.5);
        else
            *iX=*iX+(-dMovePitchX*2.0+TestIF.dSiteXPitch*0.5);
    }
    else if(iModeCol==215)                                                      //Ah --> Ad
    {
        if(TestIF_File.bEnableUseXCenterPitch==true)
            *iX=*iX+(-dMovePitchX*4.0+TestIF.dSiteXCenterPitch*0.5+TestIF.dSiteXPitch*0.5);
        else
            *iX=*iX+(-dMovePitchX*4.0+TestIF.dSiteXPitch*1.5);
    }

    *iY=*iY+GetOutArmYToShuttleOffset_9045(iSht, iOffsetPos);
    *iX=*iX+GetOutArmXToShuttleOffset_9045(iSht, iOffsetPos);
    TransferOutShuttleRatio(iSht, iX, iY, iOutArmYBase, iOutArmXBase);
    if(OutArmZNeedDown_2x4_16(iSht)==false)
        return false;
    return true;
}
//==============================================================================
//  MoveOutArmToShuttleIncludeZ_9045_2x4_16  -- golden aoutarm9045_2x4_16.cpp:578
//==============================================================================
bool MoveOutArmToShuttleIncludeZ_9045_2x4_16(int iSht, bool bZDown)
{
    bool bZFlag[MAX_ARM_Row][MAX_ARM_Col];
    int iZPos[MAX_ARM_Row][MAX_ARM_Col];
    int iXVariable[X_PITCH_COUNT];
    int iXPos               =iOutArmShtXCenterPos;                              //Steven 20141029 : XY-Pitch for Shuttle Center position for base Suck
    int iYPos               =iOutArmShtYCenterPos;
    int iMode               =GetNowShuttleMode_2x4_16(iSht);
    int iModeRow            =iMode%100;
    int iModeCol            =iMode/100;
    int iMovePitchX         =iXpitchMaxX3;
    int iMovePitchY         =GetVariableYOutShuttleData();
    int iOffsetPos          =GetOutArmToShuttleOffset_9045(iSht, iModeRow, iModeCol, true);
    int iYVariable          =GetOutArmPitchY_9045(iMovePitchY, iOffsetPos);
    ZeroMemory(iXVariable,  sizeof(iXVariable));
    ZeroMemory(bZFlag,      sizeof(bZFlag));
    ZeroMemory(iZPos,       sizeof(iZPos));

    if(USE_PICKER_COUNT==ep16Picker)
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

    if(CheckOutArmXYPitch_2x4_16(&iXPos, &iYPos, iSht, iMovePitchX, iMovePitchY)==false && bZDown==true)
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
//  DoPickFromShuttle_9045_2x4_16  -- golden aoutarm9045_2x4_16.cpp:636
//  NOTE (MODE=dead): file-internal; called ONLY from this file's own DoOutArm.
//  NOT in the engine extern block and NOT in any dispatch ladder.
//==============================================================================
extern bool CheckOutputShuttleDataMiss(TMyKitSuck *P, int i, int j);            //Steven 20110527 : 整合只叫一次
bool DoPickFromShuttle_9045_2x4_16(int iSht)
{
    int ret;
    int &Task=(iSht==0)?iPickFromShuttle1Task:iPickFromShuttle2Task;
    static int iWitchErrBin=0;
    AnsiString ErrPart="";                                                      //Steven 20110216 : 合併Alarm
    bool bHasDuplicateErr=false;                                                //Steven 20110216 : 合併Alarm
    bool bHasErr=false;                                                         //Steven 20110216 : 合併Alarm
    bool flag=false;
    int iSuckX=0, iSuckY=0, iShtX=0, iShtY=0;                                   //JerryYang 20231004 : single site suck XY固定00
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
                    if(MoveOutArmToShuttleIncludeZ_9045_2x4_16(iSht, false))    //移到 Shuttle 上等待
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
            if(MoveOutArmToShuttleIncludeZ_9045_2x4_16(iSht, true))             //kevin 20210717 change function
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
                    iSuckX  =i;
                    iSuckY  =j;
                    iShtX   =i;
                    iShtY   =XPHSuckToSht_2x4_16[OutArmSuck.iModeX][j];
                    if(ptrOutSHT->Item[iShtX][iShtY] && OutArmSuck.Suck[iSuckX][iSuckY].GetNeedSuckStatus())
                    {
                        if(OutArmSuck.Suck[iSuckX][iSuckY].Suck())
                        {
                            //模擬ic不見了
                            #ifdef SOFT_SIMULTE
                            if(fMain->chkInPickLoadError->Checked==true)
                            {
                                if(j==1)
                                {
                                    OutArmSuck.Suck[iSuckX][iSuckY].Error=true;
                                    continue;
                                }
                            }
                            #endif

                            bHasErr=SwapShuttleDataToOutArm(iSht, iShtX, iShtY, iSuckX, iSuckY, &iWitchErrBin);         //Steven 20170428 : 統一Out Arm從Shuttle吸料後的資料交換
                            bOutSuckShtDupErr[iSuckX][iSuckY]=false;
                        }
                        else if(OutArmSuck.Suck[iSuckX][iSuckY].Error==false)
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
                #if 0 // TODO(W7): fFixAICCD form (golden FixAICCD.h TfFixAICCD) -- no FormsFacade stand-in; guard kept live, form call gated (no-op offline)
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
                                    iShtY=XPHSuckToSht_2x4_16[OutArmSuck.iModeX][j];
                                    PorcessJAM0201OutArmPickUpErrorSkip(iSht, i, iShtY, i, j);                          //Steven 20161214 : 加上OutArm吸取異常Skip的ErrorLog
                                }
                                bOutSuckShtDupErr[i][j]=false;
                            }
                        }

                        if(USE_Fix_AI_CCD==true && TestIF_File.bEnableFix2BGAAICCD==true)                               //KaiChen 20190124 ：矽格湖口 AI CCD Function
                        {
                            #if 0 // TODO(W7): fFixAICCD form (golden FixAICCD.h TfFixAICCD) -- no FormsFacade stand-in; guard kept live, form call gated (no-op offline)
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
//  DoOutArm_9045_2x4_16  -- golden aoutarm9045_2x4_16.cpp:903
//==============================================================================
void DoOutArm_9045_2x4_16()
{
    int ret, ct, iMode1=0, iMode2=0;
    static int OldPos=1;
    static bool FirstEnter=true, bCheckSpeed=false;
    int &Task=OutArmTask;
    static int iWhichShuttle=0, iBackMode1=0, iBackMode2=0;
    bool flag3;
    static bool bCheckSpeed1=false;                                             //KaiChen 20171225 (Steven)：Add Speed Display

    static int iPickFinish=0x00;

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
            {                                                                   //Steven 20231207 : Mark for 4x4 16 picker
//                if(TestIF.iTestMode==_16Site4X4)                                //Sam 20190226 : 16Site4X4
//                {
//                    if(TestIF_File.iShuttleMode==1)                             //Jou 20150909 : For Add 32Site UPH, 讓Shuttle 2先回去補Input
//                    {
//                        if(FRCarryKit.HasIC() && OutSHT1InRT())
//                        {
//                            OldPos=1;
//                            FirstEnter=false;
//                        }
//                        else if((BRCarryKit.HasIC()) && OutSHT2InRT())
//                        {
//                            OldPos=0;
//                            FirstEnter=false;
//                        }
//                    }
//                    else
//                    {
//                        if((BRCarryKit.HasIC()) && OutSHT2InRT())
//                        {
//                            OldPos=0;
//                            FirstEnter=false;
//                        }
//                        else if(FRCarryKit.HasIC() && OutSHT1InRT())            //jou 20171227 : 修正32 sites hang up
//                        {
//                            OldPos=1;
//                            FirstEnter=false;
//                        }
//                    }
//                }
//                else
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
            iPickFinish=0x00;
            OldPos=1-OldPos;
            break;
        case 200:
            if(MoveOutArmToAutoSafe())
                Task=300;
            break;
        case 300:                                                               //Steven 20231207 : Mark for 4x4 16 picker
//            if(TestIF.iTestMode==_16Site4X4)                                    //Jou 20150909 : For Add 32Site UPH, 讓Shuttle 2先回去補Input   0-->1
//            {
//                if(MoveOutArmToShuttleIncludeZ_9045_2x4_16(1, false))           //Steven 20230704 : 修正NN mode動作異常
//                    Task=CheckOutArmCleanOut();
//            }
//            else
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
            if(USE_Y_AUTO_PITCH==true && IniConfig.bE57YPitchHome)              //kevin 20180827 (Steven) : add put shuttle Y pitch home AutoYPitch
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

            if(MoveOutArmToShuttleIncludeZ_9045_2x4_16(iWhichShuttle))
            {
                IsCatchTrayReadySupplyNewTray();

                if(OutArmSuck.iXStep!=1 || OutArmSuck.iYStep!=1)
                {
                    iBackMode1=GetNowShuttleMode_2x4_16(0);
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
                else
                {
                    MoveOutArmXY_ToFix_Tray_Full();                                                                         //Steven 20231027 : 先檢查有沒有要換Tray
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
                Task=1230;

                if(OutArmSuck.iXStep!=1 || OutArmSuck.iYStep!=1)
                {
                    iMode1=GetNowShuttleMode_2x4_16(0);
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
            else if(BRCarryKit.HasIC() && OutSHT2InRT() && bCheckShuttle2Flag==false)                                   // change another shuttle
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

            if(DoPickFromShuttle_9045_2x4_16(0))
            {
                PreSetOutAdditionalFlag();                                                                              //Steven 20220614 : Fixed for out arm additional function
                if(FRCarryKit.HasIC())
                {
                    for(int i=0; i<OutArmSuck.iMaxRow; i++)                                                             //jou 2013-01-21 修正 Tester 分類設定錯誤，Hang up
                    {
                        for(int j=0; j<OutArmSuck.iMaxCol; j++)
                        {
                            if(i>=OutArmSuck.iShtRow || j>=OutArmSuck.iShtCol)
                                FRCarryKit.SetItemData(i, j, NULL_IC);
                        }
                    }
                    InitPickFromShuttle1Task();                                                                         //Sam 20230208 : 修正 Fix AOI 未拍照問題
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
                    iOutRotateFinish=0;                                                                                 //kevin 20130524 目前放到 auto tray
//                    iOutArmPickPlaceCnt[OutOfsOutSh1]++;                      //JerryYang 20230523 : 沒在用, Mark掉  //JerryYang 20180921 Setup Teach功能
                    bPickSH1Flag=true;
                    bPickSH2Flag=false;
                    Task=5000;
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

            if(DoPickFromShuttle_9045_2x4_16(0))
            {
                PreSetOutAdditionalFlag();                                                                              //Steven 20220614 : Fixed for out arm additional function
                if(FRCarryKit.HasIC())
                {
                    Task=1200;
                    InitPickFromShuttle1Task();                                                                         //Sam 20230208 : 修正 Fix AOI 未拍照問題
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
                    iOutRotateFinish=0;                                                                                 //kevin 20130524 目前放到 auto tray
//                    iOutArmPickPlaceCnt[OutOfsOutSh1]++;                      //JerryYang 20230523 : 沒在用, Mark掉  //JerryYang 20180921 Setup Teach功能
                    bPickSH1Flag=true;
                    bPickSH2Flag=false;
                    Task=5000;
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
            if(USE_Y_AUTO_PITCH==true && IniConfig.bE57YPitchHome)                                                      //kevin 20180827 (Steven) : add put shuttle Y pitch home AutoYPitch
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

            if(MoveOutArmToShuttleIncludeZ_9045_2x4_16(iWhichShuttle))
            {
                IsCatchTrayReadySupplyNewTray();

                if(OutArmSuck.iXStep!=1 || OutArmSuck.iYStep!=1)
                {
                    iBackMode2=GetNowShuttleMode_2x4_16(1);
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
            else if(BRCarryKit.HasIC() && OutSHT2InRT() && bCheckShuttle2Flag==false)
            {
//#ifdef Carry4
//                MOT[MOutShuttle2].fCanMoveR=false;
//#else
                MOT[MInShuttle2].fCanMoveR=false;
//#endif
                InitPickFromShuttle2Task();
                Task=2230;

                if(OutArmSuck.iXStep!=1 || OutArmSuck.iYStep!=1)
                {
                    iMode2=GetNowShuttleMode_2x4_16(1);
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

            if(DoPickFromShuttle_9045_2x4_16(1))
            {
                PreSetOutAdditionalFlag();                                                                              //Steven 20220614 : Fixed for out arm additional function
                if(BRCarryKit.HasIC())
                {
                    for(int i=0; i<OutArmSuck.iMaxRow; i++)
                    {
                        for(int j=0; j<OutArmSuck.iMaxCol; j++)
                        {
                            if(i>=OutArmSuck.iShtRow || j>=OutArmSuck.iShtCol)
                                BRCarryKit.SetItemData(i, j, NULL_IC);
                        }
                    }
                    InitPickFromShuttle2Task();                                                                         //Sam 20230208 : 修正 Fix AOI 未拍照問題
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
                    iOutRotateFinish=0;                                                                                 //kevin 20130524 目前放到 auto tray
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
        case 2230:
            IsCatchTrayReadySupplyNewTray();

            if(bPickShuttleError)
            {
                bPickShuttleError=false;
                InitPickFromShuttle2Task();
                Task=2200;
                break;
            }

            if(DoPickFromShuttle_9045_2x4_16(1))
            {
                PreSetOutAdditionalFlag();                                                                              //Steven 20220614 : Fixed for out arm additional function
                if(BRCarryKit.HasIC())
                {
                    Task=2200;
                    InitPickFromShuttle2Task();                                                                         //Sam 20230208 : 修正 Fix AOI 未拍照問題
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
                    iOutRotateFinish=0;                                                                                 //kevin 20130524 目前放到 auto tray
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
                                                                                //Steven 20231207 : Mark for 4x4 16 picker
//                if(TestIF.iTestMode==_16Site4X4)                                //Jou 20150909 : For Add 32Site UPH, 讓Shuttle 2先回去補Input
//                {
//                    if(TestIF_File.iShuttleMode==0)                             //Jou 20150909 : For Add 32Site UPH, 讓Shuttle 2先回去補Input
//                    {
//                        if(FRCarryKit.NoIC()==true)
//                            MOT[MInShuttle1].fCanMoveR=true;
//                        if(BRCarryKit.NoIC()==true)
//                            MOT[MInShuttle2].fCanMoveR=true;
//                    }
//                    else
//                    {
//                        MOT[MInShuttle1].fCanMoveR=true;
//                        MOT[MInShuttle2].fCanMoveR=true;
//                    }
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
//                if(Task==100)
//                {
//                    if(iWhichShuttle==0)
//                    {
//                        if(iOutArmiWhichKit==0 && FRCarryKit.RightSideNoIC(2)==false)
//                        {
//                            Task=1000;
//                            return;
//                        }
//                    }
//                    else
//                    {
//                        if(iOutArmiWhichKit==0 && BRCarryKit.RightSideNoIC(2)==false)
//                        {
//                            Task=2000;
//                            return;
//                        }
//                    }
//                }
            }
            break;
        case 4000:
            if(MoveOutArmXY_ToFix_Tray_Full())
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
        case 5000:
            if(MoveOutArmToAutoSafe())
            {
                CheckOutArmSuckICFallDown();
                MOT[MInShuttle1].fCanMoveR=true;
                MOT[MInShuttle2].fCanMoveR=true;
                Task=5100;
            }
            break;
        case 5100:                                                                                                      //判斷另一個Shuttle是否有IC
            if(iPickFinish==0x01)
            {
                if(BRCarryKit.HasIC() && OutSHT2InRT() && bCheckShuttle2Flag==false)                                    // change another shuttle
                {
                    iWhichShuttle=1;
                    Task=2000;
                }
                else if(BRCarryKit.HasIC() || BTestSuck.HasIC())                                                        //Steven 20151023 : 強制等待另外一個蝦頭
                {
                    Task=5200;
                }
                else
                {
                    iOutRotateFinish=0;                                                                                 //jou 2015-10-16 修正1x4 8吸嘴 Rotate異常
                    Task=3000;
                }
            }
            else if(iPickFinish==0x02)
            {
                if(FRCarryKit.HasIC() && OutSHT1InRT() && bCheckShuttle1Flag==false)
                {
                    iWhichShuttle=0;
                    Task=1000;
                }
                else if(FRCarryKit.HasIC() || FTestSuck.HasIC())                                                        //Steven 20151023 : 強制等待另外一個蝦頭
                {
                    Task=5200;
                }
                else
                {
                    iOutRotateFinish=0;                                                                                 //kevin 20130524 準備放到ROTATE
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
                iOutRotateFinish=0;                                                                                     //kevin 20130524 準備放到ROTATE
                Task=3000;
                break;
            }
            break;
        case 5200:
            if(MoveOutArmToAutoSafe())                                                                                  //Steven 20151023 : 強制等待另外一個蝦頭
            {
                Task=5300;
            }
            break;
        case 5300:
            if(iPickFinish==0x01)
            {
                flag3=MoveOutArmToShuttleIncludeZ_9045_2x4_16(1, false);
            }
            else
            {
                flag3=MoveOutArmToShuttleIncludeZ_9045_2x4_16(0, false);
            }

            if(flag3)
                Task=5100;
            break;
        //JerryYang 20220909 : add magazine
        //==>
        case 10000:
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
        //<==
        //JerryYang 20220909 : add magazine
    }
}
//---------------------------------------------------------------------------
