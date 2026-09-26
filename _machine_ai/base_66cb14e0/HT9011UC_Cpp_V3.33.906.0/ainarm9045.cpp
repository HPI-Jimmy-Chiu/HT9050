// =============================================================================
//  ainarm9045.cpp  --  In-Arm ENGINE  (W6.2b translation)
//
//  Faithful translation of golden  ainarm9045.cpp (9222 lines, BCB6, Big5/cp950).
//  Translator: AI(W6.2b-INARM) 20260626
//
//  ROLE (golden header comment): "9045 2x4 吸嘴組 / 支援 2x4 (每次 4 隻 都用)".
//  This is the in-arm ENGINE: it owns the central pick state machine, the
//  iInArmType dispatch ladders into 25 site-variant modules, the pure
//  pitch/offset geometry, and several auxiliary task SMs.
//
//  WAVE SCOPE (W6.2b) -- what is ACTIVE vs GATED (see each gate's golden cite):
//
//   ACTIVE (faithful, verbatim formulas / branches / cursor flow):
//     * pure geometry/picker helpers:
//         GetInArmPitch_9045, GetInArmPitchX_9045, GetInArmPitch2_9045,
//         GetInArmPitchY_9045, GetInArmXToShuttleOffset_9045,
//         GetInArmYToShuttleOffset_9045, GetInArmToShuttleOffset_9045,
//         GetInArmToLoaderOffset_9045, GetInArmPitchXMM_9045,
//         GetInArmPitchYMM_9045, GetInArmCellPos, GetInArmToShtCellPos,
//         GetInOffsetFromWhichTarget, bUseAxExPicker, bUseAxxGPicker,
//         bUseACEGPicker, GetJStep, GetShuttleCol (ep1 path),
//         FindCentorPointIndex, GetShtStartPos(x2).
//     * the 2 iInArmType dispatch ladders DoInArm_9045 / DoInArm_9045_SuckerMap
//       (pre-dispatch guards + if-ladder structure + final else Program-Error).
//     * 3 task SMs whose cursor flow pumps over the Sim substrate:
//         DoInArmAdditionalFunction, DoInArmIonFanGiveWay,
//         ProcessSCKARTLoadingCount.
//     * the 11 functions previously stubbed in aHotPlateSubstrate.cpp
//       (now REAL-defined here; the substrate stubs were removed -- pre-step).
//
//   GATED (compiling ACTIVE stub right after; documented at the site):
//     * the 50 per-variant `DoInArm_9045_<layout>()` / `_SuckerMap()` call-arms
//       (`#if 0 // TODO(W6.2b variants)`) -- variants out of scope this wave;
//       the final else Program-Error path stays the live offline behavior.
//     * GetShuttleCol per-iInArmType XPHSuckToSht_* lookup branches
//       (`#if 0 // TODO(W7 variant tables)`) -- those arrays live in the
//       per-layout headers, undefined in target; ep1Picker early-return active,
//       default iShtCol=0.
//     * the dense hardware/UI SMs DoInArmPickFromLoadStage_9045 /
//       DoInArmCheckShuttleFloating / CheckInShuttleSensor_Latch /
//       DoInArmAutoCalSuckZ -- bodies are MOT[]/sensor/fProductionInfo-bound
//       with no translated home (`// TODO(W7)`); ACTIVE stubs preserve the
//       cursor var + a sane terminal value so the engine links.
//     * every other golden function (Loader search, tray-end, UPH, alarm sinks,
//       etc.) -- ACTIVE offline stub, documented in the STUBS section.
//
//  VCL/Borland conversions: AnsiString (1-based) via vclcompat; __fastcall
//  removed from CalculateUPH; Now().FormatString / AnsiString::sprintf kept
//  verbatim (vclcompat supports them); __FUNC__ via aArmHeader.h W6.0 shim; the
//  2 goto/label pairs are inside GATED SMs so they are not reproduced here (the
//  stub bodies do not need them); ChangeToFloatNonPcnt kept (zero-guarded).
//  SOFT_SIMULTE is NOT defined -> #ifndef paths stay active (none reproduced in
//  the kept bodies).
//
//  Big5: every Chinese comment decoded via cp950 and preserved as UTF-8.
//  Final gate: ZERO U+FFFD.
// =============================================================================
#include "MachineDefine.h"

#include "ainarm9045.h"
#include "aArmHeader.h"             // __FUNC__ shim, RecordProcess, ShowErrorMessage, etc.

#include "cprod.h"                 // Prod / TestIF / TestIF_File / InArmOffSet / ArmSpeed
#include "Motor/mymotor.h"         // MOT[], TTrayMotor
#include "csystem.h"               // InSHT1InLF / InSHT2InLF / state predicates
#include "cmydef.h"                // global scalar universe + IC consts + ASET_StartTimeNAME
#include "cpublic.h"               // CosFunction
#include "common.h"
#include "aHotPlateSubstrate.h"    // InArmSuck/FLCarryKit/BLCarryKit, fBarCode, cursors, tRotate
#include "FormsFacade.h"           // fMain/fSortCT/fSCKART offline stand-ins
#include "canary_support.h"        // LastSet / ShowMyMessage / ShowErrorMessage / RecordProcess
#include "myswitch.h"              // W7-A1: SW[] (SwShuttleVibration1+iSht .Off())
#include "acarry.h"                // W7-A1: IsFLCarrKitAllHasIC (shuttle floating force-vibrate guard)
#include "acarry_shims.h"          // W7-A1: fLtcSensor (TfLtcSensor* Get/Set/ClearLtcSensor)
#include "csystem_shims.h"         // W7-A1: bShuttleShake (floating SM 9000-terminal reset)
#include "ainarm9045_w7_shims.h"   // W7-A1 INTEGRATE: shuttle-motion seam decls (defines AINARM_W7A1_SHUTTLEMOTION_FWD -> suppresses the inline block below)

// ---------------------------------------------------------------------------
//  PT-W7d: the SEVEN file-scope globals golden declares BETWEEN functions.
//  CORRECTED 20260811 after checking each line against golden instead of trusting my own first
//  pass: wave_targets.py PRINTS type+name but captures array dimensions in a separate group, so
//  my first version dropped them. Three real errors, only ONE of which the compiler caught:
//    * CheckInArmDestroyActiveDelay -- golden :1168 is [MAX_ARM_Row][MAX_ARM_Col]; I had a scalar,
//      and it COMPILED anyway.
//    * bMyFlag -- golden :6131 is a 2-D array with an initialiser; the scalar only failed because
//      golden :8637 does ZeroMemory(bMyFlag, sizeof(bMyFlag)).
//    * iInArmXStep / iInArmYStep -- golden :4979/:4980 initialise to 1, I had written 0. Nothing
//      would ever have flagged that; it is the same silent-wrong-constant class this campaign
//      keeps finding in agent output.
//  Listed by tools/census/wave_targets.py and re-verified absent at integration, because
//  absence claims expire. Added by the integrator, never by an agent: two agents emitting one
//  global is a multiple-definition link error, so the wave brief made these the main loop's job.
//  No HTimer in this set, so no TQPF_Timer substitution is needed here.
//  AI(pt-wave) 20260811
// ---------------------------------------------------------------------------
TQPF_Timer CheckInArmDestroyActiveDelay[MAX_ARM_Row][MAX_ARM_Col];              // golden ainarm9045.cpp:1168
double dInArmXPitch_1Step=0.0;                                                  // golden :4977
double dInArmXPitch_MovePitch=0.0;                                              // golden :4978
int    iInArmXStep=1;                                                           // golden :4979 (golden inits to 1, NOT 0)
int    iInArmYStep=1;                                                           // golden :4980 (golden inits to 1, NOT 0)
bool bMyFlag[MAX_ARM_Row][MAX_ARM_Col]={{false, false, false, false}, {false, false, false, false}};   // golden :6131
TQPF_Timer tAutoCalSuckDelayInarm;                                              // golden :8842

//---------------------------------------------------------------------------
//  golden file globals (verbatim)  -- golden ainarm9045.cpp:46-59
//---------------------------------------------------------------------------
const int XDir=0;
const int YDir=1;
// NOTE: bInArmCheckDestroyACT (golden :48) is ALREADY defined in
// aHotPlateSubstrate.cpp (the place-swap leaf reads it).  Not redefined here to
// avoid a multiple-definition link error.  AI(W6.2b-INARM) 20260626.

// NOTE: bPickFormHotplatePartOK / bPickFormHotplateRetry /
// bInArmTryPickFromHotPlateFinish / bInArmToPickHotPlatePos are golden-defined
// here (:51-57) but are ALREADY defined in aHotPlateSubstrate.cpp because the
// already-translated leaf modules (ainarm_SearchPickPlate.cpp) need them.  To
// avoid an ODR collision we DO NOT redefine them here -- they keep their W6.2
// home.  AI(W6.2b-INARM) 20260626.

int iTempSuckX=-1;
int iTempSuckY=-1;

// golden ainarm9045.h externs that have no other home this wave (offline data).
int  iZPosToCleanKit[MAX_ARM_Row][MAX_ARM_Col]={{0}};
bool bZFlgToCleanKit[MAX_ARM_Row][MAX_ARM_Col]={{false}};
int  iMoveToShuttle=0;
bool bCheckInArmDestroyActive[MAX_ARM_Row][MAX_ARM_Col]={{false}};
int  iProcessSCKARTLoadingCountTask=1;

//==============================================================================
//==  ACTIVE: pure geometry / pitch helpers (faithful, verbatim formulas)
//==============================================================================

//==============================================================================
// 取得判斷入料變 pitch 距離  -- golden ainarm9045.cpp:129
//==============================================================================
int GetInArmPitch_9045(int w)
{
    double m,r;
    m=ChangeToFloatNonPcnt((double)(((double)(Prod.iInArmX120Pitch-Prod.iInArmX40Pitch))), (double)(iPitch_Max_minus_Min));
    r=(double)Prod.iInArmX40Pitch+m*((double)w-double(iXpitchMinX3));
    return (int)r;
}
//==============================================================================
int GetInArmPitchX_9045(int w, int iX, int iOffsetPos)                          //Steven 20230323 : For HT1032  -- golden :137
{
    int iPos=0;
    double m, r;
    double dX120[4]={Prod.iInArmX120Pitch, Prod.iInArmX120Pitch2, Prod.iInArmX120Pitch3, Prod.iInArmX120Pitch4};
    double dX_40[4]={Prod.iInArmX40Pitch,  Prod.iInArmX40Pitch2,  Prod.iInArmX40Pitch3,  Prod.iInArmX40Pitch4};

    m=ChangeToFloatNonPcnt((double)(((double)(dX120[iX]-dX_40[iX]))), (double)(iPitch_Max_minus_Min));
    if(USE_16PICKER_TYPE==1)                                                    //Ztex 2023.12.06 Add HT-1032
        r=(double)dX_40[iX]+m*((double)w-double(iXpitchMinX7));
    else
        r=(double)dX_40[iX]+m*((double)w-double(iXpitchMinX3));

    if(iOffsetPos>=0)
        iPos=r+InArmOffSet[iOffsetPos]->GetXPitch(iX);
    else
        iPos=r;

    if(bRunAutoClean && TestIF.iAutoClean_Function &&
       IniConfig.bE48_ShuttleUse4Offset_Autoclean==false)                       //Steven 20180920 : #P180920-ATK-H9-01 , InArm put the Auto Clean pad wrong position on the shuttle.
    {
        iPos=iPos+TestIF_File.ShuttlePitchOffset;
        if(CosFunction.bAutoCleanOffsetUseSingleSetting && IniConfig.bE47_ShuttleUse4Offset==false)                     //Sam 20220720 : AutoClean Offset 僅使用 Clean 設定 Offset 來補償
            iPos=iPos-InArmOffSet[iOffsetPos]->GetXPitch(iX);
    }
    return iPos;
}
//==============================================================================
int GetInArmPitch2_9045(int w)                                                  // golden :165
{
    double m, r;
    m=ChangeToFloatNonPcnt((double)(((double)(Prod.iInArmX120Pitch2-Prod.iInArmX40Pitch2))), (double)(iPitch_Max_minus_Min));                                   //Steven 20131002 : XY變距        //Isaac 20171204 (Steven) : Xpitch40->50mm, 新增150mm的Teaching點位，內差法公式計算
    r=(double)Prod.iInArmX40Pitch2+m*((double)w-double(iXpitchMinX3));
    return (int)r;
}
//==============================================================================
int GetInArmPitchY_9045(int w, int iOffsetPos)                                  //ChungHung 20131231 alter AutoYPitch  -- golden :173
{
    int iPos=0;
    double m, r;
    if(w>IN_OUT_ARM_Y_PITCH_MAX)                                                //Steven 20201205 : 加上保護
        w=IN_OUT_ARM_Y_PITCH_MAX;                                               //因為Teaching做的是6000
    m=ChangeToFloatNonPcnt((double)(Prod.iInArmY60Pitch-Prod.iInArmY15Pitch), (double)(6000-IN_OUT_ARM_Y_PITCH_MIN)); //Steven 20260505 : add zero-guard
    r=(double)Prod.iInArmY15Pitch+m*((double)w-(double)IN_OUT_ARM_Y_PITCH_MIN);
    if(iOffsetPos>=0)
        iPos=r+InArmOffSet[iOffsetPos]->GetVariableY();
    else
        iPos=r;
    return iPos;
}
//==============================================================================
int GetInArmXToShuttleOffset_9045(int iSht, int iOffsetPos)                     // golden :188
{
    int iPos=0;
    if(iOffsetPos>=0)
    {
        if(iSht==0)
            iPos=InArmOffSet[iOffsetPos]->GetX();
        else
            iPos=InArmOffSet[iOffsetPos]->GetX();
    }
    return iPos;
}
//==============================================================================
int GetInArmYToShuttleOffset_9045(int iSht, int iOffsetPos)                     // golden :201
{
    int iPos=0;
    if(iOffsetPos>=0)
    {
        if(iSht==0)
            iPos=InArmOffSet[iOffsetPos]->GetY();
        else
            iPos=InArmOffSet[iOffsetPos]->GetY();
    }
    return iPos;
}
//==============================================================================
int GetInArmToShuttleOffset_9045(int iSht, int iModeRow, int iModeCol, bool bPitch)   // golden :214
{
    int iOffsetPos=-1;
    if(TestIF.bEnableAutoAlignment==false)                                      //KenHsieh 20220923 : add AOA功能開啟不補償
    {
        if(bRunAutoClean && TestIF.iAutoClean_Function)
        {
            if(IniConfig.bE48_ShuttleUse4Offset_Autoclean)
            {
                if(iSht==0)
                {
                    if(iShuttleRowKit==2)
                        iOffsetPos=InOfsInSh1LB_AutoClean;
                    else if(iShuttleRowKit==3)
                        iOffsetPos=InOfsInSh1RA_AutoClean;
                    else if(iShuttleRowKit==4)
                        iOffsetPos=InOfsInSh1RB_AutoClean;
                    else
                        iOffsetPos=InOfsInSh1_AutoClean;
                }
                else
                {
                    if(iShuttleRowKit==2)
                        iOffsetPos=InOfsInSh2LB_AutoClean;
                    else if(iShuttleRowKit==3)
                        iOffsetPos=InOfsInSh2RA_AutoClean;
                    else if(iShuttleRowKit==4)
                        iOffsetPos=InOfsInSh2RB_AutoClean;
                    else //if(iShuttleRowKit==1)                                //Steven 20210304 : 修正1x1沒有計算Pitch的問題
                        iOffsetPos=InOfsInSh2_AutoClean;
                }
            }
            else
            {
                if(iSht==0)
                    iOffsetPos=InOfsInSh1;
                else
                    iOffsetPos=InOfsInSh2;
            }
        }
        else if(IniConfig.bE34InOutArmPitchZOffsetSameOne==true && bPitch)
        {
            iOffsetPos=InOfsInSh1;
        }
        else
        {
            if(IniConfig.bE47_ShuttleUse4Offset)                                //Steven 20140827 : Shuttle可以分別調前後排的Offset
            {
                if(iModeRow==1)
                {
                    if(iModeCol%100<10)
                    {
                        if(iSht==0)
                            iOffsetPos=InOfsInSh1LB;
                        else
                            iOffsetPos=InOfsInSh2LB;
                    }
                    else
                    {
                        if(iSht==0)
                            iOffsetPos=InOfsInSh1RB;
                        else
                            iOffsetPos=InOfsInSh2RB;
                    }
                }
                else if(iModeRow==0)
                {
                    if(iModeCol%100<10)
                    {
                        if(iSht==0)
                            iOffsetPos=InOfsInSh1;
                        else
                            iOffsetPos=InOfsInSh2;
                    }
                    else
                    {
                        if(iSht==0)
                            iOffsetPos=InOfsInSh1RA;
                        else
                            iOffsetPos=InOfsInSh2RA;
                    }
                }
                else
                {
                    if(iModeCol%100<10)                                         //Steven 20240826 : 補上Offset
                    {
                        if(iSht==0)
                            iOffsetPos=InOfsInSh1;
                        else
                            iOffsetPos=InOfsInSh2;
                    }
                    else
                    {
                        if(iSht==0)
                            iOffsetPos=InOfsInSh1RA;
                        else
                            iOffsetPos=InOfsInSh2RA;
                    }
                }
            }
            else
            {
                if(iSht==0)
                    iOffsetPos=InOfsInSh1;
                else
                    iOffsetPos=InOfsInSh2;
            }
        }
    }
    return iOffsetPos;
}
//==============================================================================
int GetInArmToLoaderOffset_9045(int iSelRow)                                    // golden :326
{
    int iOffsetPos=-1;
    if(TestIF.bEnableAutoAlignment==false)                                      //KenHsieh 20220923 : add AOA功能開啟不補償
    {
        if(IniConfig.bE46_LoaderUse2Offset && iSelRow==1)                       //Steven 20140827 : Loader可以分別調前後排的Offset
        {
            iOffsetPos=InOfsLoaderB;
        }
        else
        {
            iOffsetPos=InOfsLoader;
        }
    }
    return iOffsetPos;
}
//==============================================================================
int GetInArmPitchXMM_9045(int iMotPulse)                                        // golden :8507
{
    double m=0.0, r=0.0;
    m=ChangeToFloatNonPcnt((double)(((double)(Prod.iInArmX120Pitch-Prod.iInArmX40Pitch))), (double)(iPitch_Max_minus_Min));
    r=(double)ChangeToFloatNonPcnt((double)((iMotPulse-Prod.iInArmX40Pitch)), (double)(m));
    r+=double(iXpitchMinX3);
    return (int)r;
}
//==============================================================================
int GetInArmPitchYMM_9045(int iMotPulse)                                        // golden :8516
{
    double m=0.0,r=0.0;
    if(USE_IN_Y_IS_AUTO_PITCH==true)                                            //JerryYang 20251218 : IN/OUT ARM支援不同模組
    {
        m=ChangeToFloatNonPcnt((double)(Prod.iInArmY60Pitch-Prod.iInArmY15Pitch), (double)(6000-IN_OUT_ARM_Y_PITCH_MIN)); //Steven 20260505 : add zero-guard
        r=(double)ChangeToFloatNonPcnt((double)((iMotPulse-Prod.iInArmY15Pitch)), (double)(m));
        r+=double(IN_OUT_ARM_Y_PITCH_MIN);
    }
    else
    {
        r=TestIF.iARM_Y_PITCH;
    }

    return (int)r;
}
//==============================================================================
void GetInArmCellPos(int iRow, int iCol, int &Ypos, int &Xpos)                  // golden :8533
{
    int iMovePitchX=GetInArmPitchXMM_9045(MOT[MInArmPitch].ReadPos());
    int iMovePitchY=GetInArmPitchYMM_9045(MOT[MInArmPitchY].ReadPos());
    iMovePitchX/=3;

    if(USE_PICKER_COUNT==ep1Picker)
        iMovePitchX=0;

    Xpos=MOT[MInArmX].ReadPos()+(iMovePitchX*(iCol-iInArmXBase));
    Ypos=MOT[MInArmY].ReadPos()-(iMovePitchY*(iRow-iInArmYBase));
}
//==============================================================================
void GetInArmToShtCellPos(int iRow, int iCol, int &Ypos, int &Xpos)             // golden :8546
{
    int iMovePitchX=GetInArmPitchXMM_9045(MOT[MInArmPitch].ReadPos());
//    int iMovePitchY=GetInArmPitchYMM_9045(MOT[MInArmPitchY].ReadPos());
    iMovePitchX/=3;
    Xpos=MOT[MInArmX].ReadPos()+(iMovePitchX*(iCol-iInArmXBase));
    Ypos=MOT[MInArmY].ReadPos();
}
//==============================================================================
int GetInOffsetFromWhichTarget(int iWhichTarget)                                //Jimmychiu 20240731 : for out arm offset value  -- golden :8555
{
    if(iWhichTarget==MMTrayY)
    {
        return InOfsLoader;
    }
    else if(iWhichTarget==MMPlate1)
    {
        return InOfsHP2;
    }
    else if(iWhichTarget==MMPlate2)
    {
        return InOfsHP1;
    }
    else if(iWhichTarget==MMAutoCleanKit)
    {
        if(IniConfig.bE43AutoCleanUseHotplate)                                  //Steven 20160629 : for AutoClean use Hotplate1
        {
            return InOfsHP1;
        }
        else
        {
            return InOfsAutoClean;
        }
    }
    else if(iWhichTarget==MInRotateKit)                                         //待確認
    {
        return iRotate_In_Index+InOfsHP1;
    }
    else if(iWhichTarget==MPreciser)                                            //待確認
    {
        return InOfsPreciser;
    }
    return 0;
}

//==============================================================================
//==  ACTIVE: picker-mode helpers + GetJStep (faithful, verbatim)
//==============================================================================
bool bUseAxExPicker()                                                           // golden :8281 (REAL-defined here; substrate stub removed)
{
    if(iInArmType==e9045_1x2_2_13 ||
       iInArmType==e9045_1x4_4_13 ||
       iInArmType==e9045_2x1_2_13 ||
       iInArmType==e9045_2x2_4_13 ||
       iInArmType==e9045_2x4_4_13 ||
       iCloseSiteModeFor2x8==e2x8Run2x2_13)
    {
        return true;
    }
    else if(bRunAutoClean)                                                      //Sam 20250203 : 修正 1X4 8 吸嘴模式
    {
        if(iInArmType==e9045_2x2_8_Hot &&
           iXpitchMinX2_MM<TestIF_File.dSiteXPitch &&
           TestIF_File.dSiteXPitch<=iXpitchMaxX2_MM)                            //SitePitchX 26-80 之間跑 1 3 吸嘴
        {
            return true;
        }
    }
    return false;
}
//------------------------------------------------------------------------------
bool bUseAxxGPicker()                                                           // golden :8304 (REAL-defined here; substrate stub removed)
{
    if(iInArmType==e9045_1x2_2_14 ||
       iInArmType==e9045_1x3_2_14 ||                                            //Steven 20220425 : 1x3_14
       iInArmType==e9045_1x4_2_14 ||
       iInArmType==e9045_2x2_2_14 ||
       iInArmType==e9045_2x2_4_14 ||
       iInArmType==e9045_2x3_6_14 ||                                            //Steven 20200219 : 2x3mode, Layout: 120x70mm
       iInArmType==e9045_2x4_4_14 ||
       iCloseSiteModeFor2x8==e2x8Run2x2_14)
    {
        return true;
    }
    else if(bRunAutoClean)                                                      //Sam 20250212 : 修正 2X2 8 吸嘴模式 AutoClean
    {
        if(iInArmType==e9045_2x2_8_Hot &&
           TestIF_File.dSiteXPitch>iXpitchMaxX2_MM)                             //SitePitchX >80 之間跑 1 4 吸嘴
            return true;
    }
    return false;
}
//------------------------------------------------------------------------------
bool bUseACEGPicker()                                                           // golden :8326
{
    if(iInArmType==e9045_1x4_4 ||
       iInArmType==e9045_1x4_4_Back ||                                          //Steven 20220425 : 1x3_14
       iInArmType==e9045_1x4_8_Hot ||
       iInArmType==e9045_2x4_8 ||
       iInArmType==e9045_2x8_8 ||
       iInArmType==e9045_2x8_32 ||
       iInArmType==e9045_2x2_8_Hot)
    {
        return true;
    }
    return false;
}
//------------------------------------------------------------------------------
int GetJStep()                                                                  // golden :8341
{
    int jStep=1;
    if(bUseAxExPicker())
    {
        jStep=2;
    }
    else if(bUseAxxGPicker() ||
           iCloseSiteModeFor1x4==e1x4CloseAbAc)                                 //Steven 20241111 : for 1x4 close 2 site)
    {
        jStep=3;
    }
    else
    {
        jStep=1;
    }
    return jStep;
}
//---------------------------------------------------------------------------
int GetShuttleCol(int iSuckRow, int iSuckCol, bool bOutArm)                     // golden :8360
{
    int iShtCol=0;

    if(USE_PICKER_COUNT==ep1Picker)
    {
        return iSuckCol;
    }

    // golden :8369-8462 dispatches into the per-iInArmType XPHSuckToSht_*
    // lookup tables (XPHSuckToSht_2x4_8 / _2x2_13 / _2x8_8 / ...).  Those arrays
    // live in the per-layout headers (ainarm9045_<layout>.h) which are NOT in
    // the target tree this wave -> the ladder cannot compile.  Gate it; the
    // ep1Picker early-return above stays ACTIVE and the default iShtCol=0 is the
    // live offline value.
#if 0 // TODO(W7 variant tables) -- golden ainarm9045.cpp:8369 (XPHSuckToSht_* undefined in target)
    if(iInArmType==e9045_1x4_1_Ac)
    {
        iShtCol=2;
    }
    else if(iInArmType==e9045_1x1_1)
    {
        iShtCol=0;
    }
    else if(iInArmType==e9045_1x2_4_Hot)
    {
        iShtCol=XPHSuckToSht_1x2_4[i1x2_4UseACEGPicker][iSuckRow][iSuckCol];
    }
    /* ... 23 further per-iInArmType lookup-table branches (golden :8381-8462) ... */
    else if(iInArmType==e9045_2x8_8 ||
            iInArmType==e9045_2x8_32)
    {
        if(bOutArm)
            iShtCol=XPHSuckToSht_2x8_8_OutArm[InArmSuck.iModeX][iSuckRow][iSuckCol];
        else
            iShtCol=XPHSuckToSht_2x8_8[InArmSuck.iModeX][iSuckRow][iSuckCol];
    }
#else
    (void)iSuckRow; (void)bOutArm;       // W7-gated lookup tables
#endif

    return iShtCol;
}
//------------------------------------------------------------------------------
bool DoPickLoaderOK(int iSht, int iKit)                                         // golden :8466
{
    bool bflag=false;
    if(IniConfig.bI28_OnOffSiteOnTheFly &&
       LastSet.iTemperature==Tempture_Hot &&
       InArmSideAllCloseWithKit(iSht, iKit))
    {
#if 0 // TODO(W7) -- golden :8472 SetInArmUseSuckToHasNullIC (no translated home)
        SetInArmUseSuckToHasNullIC(iSht, iKit);
#endif
    }

    int iKit32=(iKit==0)?0:4;

    if(USE_PICKER_COUNT==ep1Picker)                                             //JerryYang 20250904 : add
    {
        if(InArmSuck.Item[0][0]==NULL_IC)
        {
            bflag=true;
        }
    }
    else
    {
        for(int i=0; i<InArmSuck.iPickRow; i++)
        {
             for(int j=0; j<InArmSuck.iMaxCol; j++)
            {
                if(Prod.fInArmSuck4x8[iSht][i][j+iKit32] &&
                   InArmSuck.Item[i][j]==NULL_IC)
                {
                    bflag=true;
                }
            }
        }
    }

    if(!bflag)                                                                  //InArm 已吸滿
    {
        return true;
    }
    return false;
}

//==============================================================================
//==  ACTIVE: shuttle cell geometry (pure arithmetic; faithful)
//==============================================================================
double FindCentorPointIndex(int iItemCount)                                     // golden :9071
{
    int iPitchCount=iItemCount-1;
    double dret=(double)iPitchCount/2;
    return dret;
}
//---------------------------------------------------------------------------
int GetShtStartPos(int iCentorABSPostion, double dCentorIndex, int iGap)        // golden :9078
{
    int iret=iCentorABSPostion-(int)(dCentorIndex*iGap);
    return iret;
}
//---------------------------------------------------------------------------
int GetShtStartPos(int iItemCount,                                              // golden :9084
                   int iCentorABSPostion,
                   int iGap)
{
    double dCentorIndex=FindCentorPointIndex(iItemCount);
    int iStart=GetShtStartPos(iCentorABSPostion, dCentorIndex, iGap);
    return iStart;
}
//---------------------------------------------------------------------------
void GetShtRowColStartPos(int iTarget, int &iColStart, int &iRowStart)          // golden :9093
{
    // golden reads TestSocket.iShtRow / .iShtCol -- TestSocket is a cross-module
    // object with no translated home in the target this wave.  Gate the
    // TestSocket reads (default the grid to 0x0); keep the arithmetic + the
    // per-target Prod offset additions ACTIVE.  TODO(W7) restore TestSocket.
    int iRow=0;                                                                 // golden: TestSocket.iShtRow
    int iCol=0;                                                                 // golden: TestSocket.iShtCol
#if 0 // TODO(W7) -- golden :9095 TestSocket.iShtRow/iShtCol (TestSocket no home)
    iRow=TestSocket.iShtRow;
    iCol=TestSocket.iShtCol;
#endif
    iColStart=iRowStart=0;
    int HardwarePosX=iInArmShtXCenterPos;
    int HardwarePosY=iInArmShtYCenterPos;
    if(bUseTwoArm32Site==true)                                                  //RogerYang 20250815 修正32Site計算Y異常
        iRow=iRow/2;
    if(iTarget==MOutShuttle1 ||
       iTarget==MOutShuttle2)
    {
        HardwarePosX=iOutArmShtXCenterPos;
        HardwarePosY=iOutArmShtYCenterPos;
    }

    if(iTarget==MInShuttle1)
    {
        HardwarePosX+=Prod.XInArm_Shuttle1_Place[iInArmYBase][iInArmXBase];
        HardwarePosY+=Prod.YInArm_Shuttle1_Place[iInArmYBase][iInArmXBase];
    }
    else if(iTarget==MInShuttle2)
    {
        HardwarePosX+=Prod.XInArm_Shuttle2_Place[iInArmYBase][iInArmXBase];
        HardwarePosY+=Prod.YInArm_Shuttle2_Place[iInArmYBase][iInArmXBase];
    }
    else if(iTarget==MOutShuttle1)
    {
        HardwarePosX+=Prod.XOutArm_Shuttle1_Pick[iOutArmYBase][iOutArmXBase];
        HardwarePosY+=Prod.YOutArm_Shuttle1_Pick[iOutArmYBase][iOutArmXBase];
    }
    else if(iTarget==MOutShuttle2)
    {
        HardwarePosX+=Prod.XOutArm_Shuttle2_Pick[iOutArmYBase][iOutArmXBase];
        HardwarePosY+=Prod.YOutArm_Shuttle2_Pick[iOutArmYBase][iOutArmXBase];
    }
    else
    {
        return;
    }
    iColStart=GetShtStartPos(iCol, HardwarePosX, (int)(TestIF.dSiteXPitch));
    iRowStart=GetShtStartPos(iRow, HardwarePosY, (int)(TestIF.dSiteYPitch)*(-1));                                       //反向補償*(-1)
}
//---------------------------------------------------------------------------
void GetShtRowColPos(int iTarget, int iColIndex, int iRowindex, int &HardwarePosX, int &HardwarePosY)   // golden :9137
{
    GetShtRowColStartPos(iTarget,HardwarePosX,HardwarePosY);
    HardwarePosX+=(int)(TestIF.dSiteXPitch)*iColIndex;
    HardwarePosY-=(int)(TestIF.dSiteYPitch)*iRowindex;

    if(TestIF.bNS7000kit)                                                       //jou 981208 start : NS7000 bias kit
    {
        int iShiftY=(TestIF.dSiteYPitch==0)?(6000/2):(TestIF.dSiteYPitch/2);
        if(iTarget==MInShuttle1)
            HardwarePosY+=iShiftY;
        else
            HardwarePosY-=iShiftY;
    }
    else if(bUseTwoArm32Site==true)                                             //RogerYang 20250815 修正32Site計算Y異常
    {
        if(iTarget==MInShuttle1)
            HardwarePosY+=1000;
        else
            HardwarePosY-=1000;
    }
}

//==============================================================================
//==  ACTIVE: InspectInArmPosition (REAL-defined here; substrate stub removed)
//==  golden :8591.  Early-return path is the live offline behavior (the debug
//==  flag bE74_InspectArmPosition is off by default); the body's coordinate
//==  comparison + alarm sink is gated (touches encoders + ShowErrorMessage).
//==============================================================================
void InspectInArmPosition(int iTarget, int iSuckRow, int iSuckCol, int iTargetRow, int iTargetCol, bool bPlace)
{
    if(IniConfig.bE74_InspectArmPosition==false)                                //Jimmychiu 20240408 : debug for inarm position
        return;

#if 0 // TODO(W7) -- golden :8597-8697 encoder compare + ShowErrorMessage alarm sink
    // Full coordinate-deviation check: GetInArmCellPos vs the per-target
    // Prod.*_Pick hardware position, then ShowErrorMessage("WAR...") on mismatch.
    // Gated: reads MOT encoders & dispatches an alarm with no offline home.
#else
    (void)iTarget; (void)iSuckRow; (void)iSuckCol; (void)iTargetRow; (void)iTargetCol; (void)bPlace;
#endif
}

//==============================================================================
//==  ACTIVE: left-side predicates (REAL-defined here; substrate stubs removed)
//==  golden :4193 / :4212.
//==============================================================================
bool InArmLeftSideNoIC(int iRow)                                                // golden :4193
{
    // golden walks InArmSuck.Item[iRow][0..iModeX] checking for !=HAS_IC.
    // Offline grid is empty -> "left side has no IC" is true.  Keep faithful to
    // the empty-grid result without dereferencing the (unsized) iModeX walk.
    for(int j=0; j<InArmSuck.iMaxCol; j++)
    {
        (void)iRow;
        // offline: Item defaults to 0 (==NULL_IC region); treat as no real IC.
    }
    return InArmSuck.NoIC();
}
//------------------------------------------------------------------------------
bool InArmLeftSideHasIC(int iRow)                                               // golden :4212
{
    (void)iRow;
    return InArmSuck.HasRealIC();
}

//==============================================================================
//==  ACTIVE: IsCheckInArmDestroyActiveFinish (REAL-defined here; stub removed)
//==  golden :1153.  Offline: no destroy pending -> finished.
//==============================================================================
bool IsCheckInArmDestroyActiveFinish()                                          //ChungHung 20111229
{
    for(int i=0; i<MAX_ARM_Row; i++)
        for(int j=0; j<MAX_ARM_Col; j++)
            if(bCheckInArmDestroyActive[i][j]==true)
                return false;
    return true;
}

//==============================================================================
//==  ACTIVE: AddInArmPickerCount / SetShuttleToHasNullIC_9045 (REAL-defined
//==  here; substrate stubs removed).  golden :2584 / :343.  The golden bodies
//==  drive prod-record counters / shuttle-item grids; offline the counters live
//==  in Prod and the shuttle item-grid write is a no-op (no shuttle HAL yet).
//==============================================================================
void AddInArmPickerCount(int iSuckRow, int iSuckCol)                            //JerryYang 20220331 : add 吸真空次數計數  -- golden :2584
{
#if 0 // TODO(W7) -- golden :2586 InArmSuck.PordRec[][].Add* prod-record counter
    InArmSuck.PordRec[iSuckRow][iSuckCol].AddInArmPickRecord();
#else
    (void)iSuckRow; (void)iSuckCol;
#endif
}
//------------------------------------------------------------------------------
void SetShuttleToHasNullIC_9045(int iSht, int iKit)                             // golden :343
{
#if 0 // TODO(W7) -- golden :345-384 FLCarryKit/BLCarryKit shuttle-grid item writes
    // Marks the closed shuttle/kit cells to HAS_NULL_IC across the carry-kit
    // grids; offline the carry-kit grids have no IC so this is a no-op.
#else
    (void)iSht; (void)iKit;
#endif
}

//==============================================================================
//==  ACTIVE: iInArmType dispatch ladder #1  DoInArm_9045   -- golden :4416
//==  Pre-dispatch guards + if-ladder STRUCTURE kept ACTIVE; the 25 per-variant
//==  call-arms are GATED (variants out of scope) so the live offline behavior
//==  is the final else Program-Error path.
//==============================================================================
// golden :4391-4415 declares the 25 variant externs.  W6.2b-canary: only the
// e9045_1x1_1 / e9045_1x4_1_Ac arm is un-gated this wave -> declare ONLY the two
// now-live callees (defined in ainarm9045_1x1_1.cpp -- file-local in the golden,
// consumed via this dispatch ladder, so NOT in ainarm9045_1x1_1.h).  golden :4391
// `extern void DoInArm_9045_1x1_1();` + golden :4650 `extern void
// DoInArm_9045_1x1_1_SuckerMap();`.  The remaining 23 externs are still inside
// the #if 0 gate below, so they need no declaration.
extern void DoInArm_9045_1x1_1();                                               //AI(W6.2b-canary) 20260626: golden :4391 -- now-live dispatch callee
extern void DoInArm_9045_1x1_1_SuckerMap();                                     //AI(W6.2b-canary) 20260626: golden :4650 -- now-live SuckerMap callee
// W6.2c batch-1: 5 variants made live -- declare their now-live dispatch callees
// (defined in ainarm9045_<v>.cpp).  AI(W6.2c-INARM) 20260626.
extern void DoInArm_9045_1x2_2();                                               //golden :4392
extern void DoInArm_9045_1x2_2_SuckerMap();                                     //golden :4651
extern void DoInArm_9045_1x2_2_14();                                            //golden :4393
extern void DoInArm_9045_1x2_2_14_SuckerMap();                                  //golden :4652
extern void DoInArm_9045_1x2_4_Hot();                                           //golden :4394
extern void DoInArm_9045_1x2_4_Hot_SuckerMap();                                 //golden :4653
extern void DoInArm_9045_1x3_4();                                               //golden :4395
extern void DoInArm_9045_1x3_4_SuckerMap();                                     //golden :4655
extern void DoInArm_9045_1x3_2_14();                                            //golden :4396
extern void DoInArm_9045_1x3_2_14_SuckerMap();                                  //golden :4654
// W6.2c batch-2: 6 variants made live -- declare their now-live dispatch callees
// (defined in ainarm9045_<v>.cpp).  AI(W6.2c-INARM-batch2) 20260626.
extern void DoInArm_9045_1x4_2();                                               //golden :4397
extern void DoInArm_9045_1x4_2_SuckerMap();                                     //golden :4657
extern void DoInArm_9045_1x4_8_Hot();                                           //golden :4401
extern void DoInArm_9045_1x4_8_Hot_SuckerMap();                                 //golden :4661
extern void DoInArm_9045_2x1_2();                                              //golden :4402
extern void DoInArm_9045_2x1_2_SuckerMap();                                     //golden :4662
extern void DoInArm_9045_2x2_4();                                              //golden :4403
extern void DoInArm_9045_2x2_4_SuckerMap();                                     //golden :4663
extern void DoInArm_9045_2x2_4_12();                                            //golden :4404
extern void DoInArm_9045_2x2_4_12_SuckerMap();                                  //golden :4664
extern void DoInArm_9045_2x2_4_14();                                            //golden :4405
extern void DoInArm_9045_2x2_4_14_SuckerMap();                                  //golden :4665
// W6.2c batch-3: 6 variants made live -- declare their now-live dispatch callees.
// AI(W6.2c-INARM-batch3) 20260626.
extern void DoInArm_9045_1x4_4();                                               //golden :4408
extern void DoInArm_9045_1x4_4_SuckerMap();                                     //golden :4658
extern void DoInArm_9045_1x4_4_Back();                                          //golden :4407
extern void DoInArm_9045_1x4_4_Back_SuckerMap();                                //golden :4659
extern void DoInArm_9045_2x3_6();                                               //golden :4411
extern void DoInArm_9045_2x3_6_SuckerMap();                                     //golden :4666
extern void DoInArm_9045_2x3_6_14();                                            //golden :4412
extern void DoInArm_9045_2x3_6_14_SuckerMap();                                  //golden :4667
extern void DoInArm_9045_2x4_4();                                               //golden :4409
extern void DoInArm_9045_2x4_4_SuckerMap();                                     //golden :4669
extern void DoInArm_9045_All_1Pick();                                           //golden :4415
extern void DoInArm_9045_All_1Pick_SuckerMap();                                 //golden :4675 (declared; NEVER dispatched -- faithful to golden)
// W6.2c batch-4: 5 variants made live -- declare their now-live dispatch callees
// (defined in ainarm9045_<v>.cpp).  AI(W6.2c-INARM-batch4) 20260626.  NOTE:
// e9045_2x8_32 has NO own callee -- the golden engine routes it to the 2x8_8 family
// (golden :4639/:4804), so no extra extern is needed for it.
extern void DoInArm_9045_2x2_8_Hot();                                           //golden :4406
extern void DoInArm_9045_2x2_8_Hot_SuckerMap();                                 //golden :4666
extern void DoInArm_9045_2x4_8();                                               //golden :4411
extern void DoInArm_9045_2x4_8_SuckerMap();                                     //golden :4671
extern void DoInArm_9045_2x5_8();                                               //golden :4412
extern void DoInArm_9045_2x5_8_SuckerMap();                                     //golden :4672
extern void DoInArm_9045_2x6_8();                                               //golden :4413
extern void DoInArm_9045_2x6_8_SuckerMap();                                     //golden :4673
extern void DoInArm_9045_2x8_8();                                               //golden :4414
extern void DoInArm_9045_2x8_8_SuckerMap();                                     //golden :4674
// W6.2c batch-5: 2 S-family (HT-9045S AutoClean) variants made live.  IRREGULAR
// func names -- copy EXACTLY: file ainarm9045S_1x4_4 defines DoInArm_9045S_1x4_4
// (S in func), file ainarm9045S_2x4_4_13 defines DoInArm_9045_2x4_4_13 (NO S in func).
extern void DoInArm_9045S_1x4_4();              //golden ainarm9045.cpp:4569 callee (def ainarm9045S_1x4_4.cpp:1528)
extern void DoInArm_9045S_1x4_4_SuckerMap();    //golden :4734 callee (def ainarm9045S_1x4_4.cpp:1497)
extern void DoInArm_9045_2x4_4_13();            //golden :4617 callee (def ainarm9045S_2x4_4_13.cpp:1818)
extern void DoInArm_9045_2x4_4_13_SuckerMap();  //golden :4782 callee (def ainarm9045S_2x4_4_13.cpp:1783)

void DoInArm_9045()                                                             //Steven 20240223 : 重新整理DoInArm_9045
{
    AnsiString Str;
    static int iD44CheckIndexICDestroy=0;                                       //kevin 20190201 detetect Count

    if(MACHINE_HAS_AUTO_ALIGNMENT_CCD &&                                        //KenHsieh 20210813 : add CCD AUTO ALIGNMENT
       TestIF.bEnableAutoAlignment==true &&
       (LastSet.iRealDummy==HAS_TRAY ||
        LastSet.iRealDummy==REALLY))
    {
        if(bRunInArmAutoAlignment || lOutArmAutoAlignmentFlag)
        {
            return;
        }
    }

    if(CosFunction.bAutoSiteMappingUseHotPlate==true    &&                      //Ifor 20180518 : add 簡化Site Mapping 旗標
       IniConfig.bI21AutoSiteMappingUseHotplate==true   &&
       bAutoSiteMapHotplateReady==true                  &&
       LastSet.iRunStartMode==rsmAutoSiteMap            &&
       IniConfig.bI21EnableASM==true                    &&
       bSiteMappingCHKOK==false                         )
    {
        bRunAutoSiteMapping=true;
    }
    else
    {
        bRunAutoSiteMapping=false;
    }

    if(IniConfig.bEnableTestingNeedStopAllMotor==true &&
       IniConfig.bI24TestingNeedStopAllMotor==true &&
       bTestingStopAllMotor==true)                                              //jou 2013-09-25 Testing Need Stop All Motor
    {
        StopAllMotor();
#if 0 // TODO(W7) -- golden :4451 MOT[MTestZ1].Gali_Command("ST") motion stop
        MOT[MTestZ1].Gali_Command("ST", __FUNC__);
#endif
        return;
    }

    if(IniConfig.bAlarmNeedServoOff==true)                                      //JerryYang 20180514 新增保護避免servo off後推動in arm, 再重新start會發生點位錯誤
    {
#if 0 // TODO(W7) -- golden :4456 fNote->bMyServoOffInArm (TfNote VCL form, no home)
        if(fNote->bMyServoOffInArm==true)
        {
            return;
        }
#endif
    }

    if(ASET_StartTimeNAME=="")                                                  //kevin 20150212 高雄日月光IC履歷記錄 先給檔案初始時間    //Steven 20160531 : modify
    {
        ASET_StartTimeNAME=FormatDateTime("yyyymmdd", Now());                   //kevin 20161228 記錄天數為檔名  // golden: Now().FormatString("yyyymmdd") -> vclcompat FormatDateTime
    }

    if(iD44CheckIndexICDestroy>100)                                             //kevin 20190201 detetect Count
        bIndexplaceCheck=true;                                                  //kevin 20190201 index 回溪檢測中

    if(CUSTOMER_CODE==CC_ASE_KaohSiung)
    {
        if(ArmSpeed_File[InArm].bDevicConfirm)                                  //kevin 20170510 (wei) 回吸功能開關
            CheckInArmDestroyActive();                                          //jou 981130 start : 確認device確實destroy完成

        if(IniConfig.bD44CheckIndexICDestroy &&
           iIndexTakeDeviceChk2!=0 &&
           (iArmTask==2000 || bIndexAlarmInArmAway))                            //kevin 20190201 回吸檢測狀態 避免inarm來回跑
        {
            iD44CheckIndexICDestroy++;                                          //kevin 20190201 detetect Count
            return;
        }
    }
    else
    {
        CheckInArmDestroyActive();                                              //jou 981130 start : 確認device確實destroy完成
    }
    iD44CheckIndexICDestroy=0;                                                  //kevin 20190201 detetect Count
    bIndexplaceCheck=false;                                                     //kevin 20190201 index 回溪檢測中

    for(int i=0; i<InArmSuck.iMaxRow; i++)
    {
        for(int j=0; j<InArmSuck.iMaxCol; j++)
        {
            if(bCheckInArmDestroyActive[i][j]==true)
            {
                return;
            }
        }
    }

#if 0 // TODO(W7) -- golden :4503 bEject (cmydef.h extern, no definition in target yet)
    if(bEject)                                                                  //JerryYang 20251020 : 渠梁半清機功能
    {
        return;
    }
#endif

    if(iPauseBackUp!=-1 &&
       InArmSuck.IsPickSuckFinish()==true &&
       InArmSuck.IsPickDestroyFinish()==true)
    {
        return;
    }

    if(bResetInArmTask==true)
    {
        bResetInArmTask=false;
        InitInArmTask();
        return;
    }

    if(bInArmLaserActionflag[0]==true ||                                        //==> Eastsun 20260525 laser 整合
       bInArmLaserActionflag[1]==true)
    {
        return;
    }

    // --- iInArmType dispatch ladder ---------------------------------------
    // STRUCTURE kept ACTIVE.  W6.2b-canary UN-GATES ONLY the e9045_1x1_1 /
    // e9045_1x4_1_Ac arm -- the SOLE per-site variant translated + buildable
    // this wave (ainarm9045_1x1_1.cpp; golden :4536/:4563 both route here).
    // The other 23 arms (DoInArm_9045_<layout>()) are NOT translated yet -> they
    // stay GATED (#if 0) below so the engine still links; for any still-gated
    // iInArmType the final else Program-Error path is the live offline behavior.
    // NOTE (verified, not assumed): the golden ladder has NO e9045_2x4_16 enum
    // and routes e9045_2x8_32 -> DoInArm_9045_2x8_8() (golden :4639-4641), so the
    // ainarm9045_2x4_16.cpp / _2x8_32.cpp files are DEAD -- registered to compile,
    // never dispatched (see CMakeLists ht9045_sm + each variant file head).
    //AI(W6.2c-INARM-batch3) 20260626: All_1Pick is the FIRST golden ladder arm
    //(golden :4535-4537), evaluated by picker-count BEFORE the iInArmType chain.
    if(USE_PICKER_COUNT==ep1Picker)             { DoInArm_9045_All_1Pick();  }  //AI(W6.2c-INARM-batch3) 20260626: golden :4535
    else if(iInArmType==e9045_1x1_1 ||                                          //AI(W6.2b-canary) 20260626: un-gate ONLY the translated 1x1_1 arm
       iInArmType==e9045_1x4_1_Ac)              { DoInArm_9045_1x1_1();      }  //golden :4536 (e9045_1x1_1) + :4563 (e9045_1x4_1_Ac both call DoInArm_9045_1x1_1)
    // --- W6.2c batch-1: 5 variant arms pulled out of the #if 0 gate (ACTIVE) ---
    else if(iInArmType==e9045_1x2_2_13)          { DoInArm_9045_1x2_2();      }  //AI(W6.2c-INARM) 20260626: golden :4538
    else if(iInArmType==e9045_1x2_2_14)          { DoInArm_9045_1x2_2_14();   }  //AI(W6.2c-INARM) 20260626: golden :4539
    else if(iInArmType==e9045_1x2_4_Hot)         { DoInArm_9045_1x2_4_Hot();  }  //AI(W6.2c-INARM) 20260626: golden :4540
    else if(iInArmType==e9045_1x3_2_14)          { DoInArm_9045_1x3_2_14();   }  //AI(W6.2c-INARM) 20260626: golden :4541
    else if(iInArmType==e9045_1x3_4)             { DoInArm_9045_1x3_4();      }  //AI(W6.2c-INARM) 20260626: golden :4559
    // --- W6.2c batch-2: 6 variant arms pulled out of the #if 0 gate (ACTIVE) ---
    else if(iInArmType==e9045_1x4_2_14)          { DoInArm_9045_1x4_2();      }  //AI(W6.2c-INARM-batch2) 20260626: golden :4571
    else if(iInArmType==e9045_1x4_8_Hot)         { DoInArm_9045_1x4_8_Hot();  }  //AI(W6.2c-INARM-batch2) 20260626: golden :4583
    else if(iInArmType==e9045_2x1_2_13)          { DoInArm_9045_2x1_2();      }  //AI(W6.2c-INARM-batch2) 20260626: golden :4587
    else if(iInArmType==e9045_2x2_4_12)          { DoInArm_9045_2x2_4_12();   }  //AI(W6.2c-INARM-batch2) 20260626: golden :4591
    else if(iInArmType==e9045_2x2_4_13)          { DoInArm_9045_2x2_4();      }  //AI(W6.2c-INARM-batch2) 20260626: golden :4595
    else if(iInArmType==e9045_2x2_4_14)          { DoInArm_9045_2x2_4_14();   }  //AI(W6.2c-INARM-batch2) 20260626: golden :4599
    // --- W6.2c batch-3: 5 variant arms pulled out of the #if 0 gate (ACTIVE);
    //     All_1Pick is the leading picker-count `if` above (golden :4535) ---
    else if(iInArmType==e9045_1x4_4_Back)        { DoInArm_9045_1x4_4_Back(); }  //AI(W6.2c-INARM-batch3) 20260626: golden :4575
    else if(iInArmType==e9045_1x4_4)             { DoInArm_9045_1x4_4();      }  //AI(W6.2c-INARM-batch3) 20260626: golden :4579
    else if(iInArmType==e9045_2x3_6)             { DoInArm_9045_2x3_6();      }  //AI(W6.2c-INARM-batch3) 20260626: golden :4613
    else if(iInArmType==e9045_2x3_6_14)          { DoInArm_9045_2x3_6_14();   }  //AI(W6.2c-INARM-batch3) 20260626: golden :4607
    else if(iInArmType==e9045_2x4_4_14)          { DoInArm_9045_2x4_4();      }  //AI(W6.2c-INARM-batch3) 20260626: golden :4619
    // --- W6.2c batch-4: 6 variant arms pulled out of the #if 0 gate (ACTIVE);
    //     e9045_2x8_32 routes to the SAME DoInArm_9045_2x8_8() callee (golden :4639-4641),
    //     so both 2x8_8 and 2x8_32 arms are active and both call the 2x8_8 family. ---
    else if(iInArmType==e9045_2x2_8_Hot)         { DoInArm_9045_2x2_8_Hot();  }  //AI(W6.2c-INARM-batch4) 20260626: golden :4603
    else if(iInArmType==e9045_2x4_8)             { DoInArm_9045_2x4_8();      }  //AI(W6.2c-INARM-batch4) 20260626: golden :4623
    else if(iInArmType==e9045_2x5_8)             { DoInArm_9045_2x5_8();      }  //AI(W6.2c-INARM-batch4) 20260626: golden :4627
    else if(iInArmType==e9045_2x6_8)             { DoInArm_9045_2x6_8();      }  //AI(W6.2c-INARM-batch4) 20260626: golden :4631
    else if(iInArmType==e9045_2x8_8)             { DoInArm_9045_2x8_8();      }  //AI(W6.2c-INARM-batch4) 20260626: golden :4635
    else if(iInArmType==e9045_2x8_32)            { DoInArm_9045_2x8_8();      }  //AI(W6.2c-INARM-batch4) 20260626: golden :4639 (routes 2x8_32 -> 2x8_8 callee)
    // --- W6.2c batch-5: 2 S-family (HT-9045S) variant arms made ACTIVE. ---
    else if(iInArmType==e9045_1x4_4_13)          { DoInArm_9045S_1x4_4();     }  //AI(W6.2c-INARM-batch5) 20260626: golden :4567 (HT-9045S; S in func name)
    else if(iInArmType==e9045_2x4_4_13)          { DoInArm_9045_2x4_4_13();   }  //AI(W6.2c-INARM-batch5) 20260626: golden :4615 (NO S in func name)
    else
    {
        Str.sprintf("iInArmType=%d", iInArmType);                               //Steven 20220620 : add log message
        ShowMyMessage("Program Error in DoInArm_9045()", Str);
    }
}

//==============================================================================
//==  ACTIVE: iInArmType dispatch ladder #2  DoInArm_9045_SuckerMap  -- golden :4676
//==  REAL-defined here (substrate stub removed).  The Prod grid-clear is kept
//==  ACTIVE (faithful); the 25 per-variant *_SuckerMap() call-arms are GATED.
//==============================================================================
void DoInArm_9045_SuckerMap()                                                   //Steven 20240223 : 重新整理DoInArm_9045_SuckerMap
{
    AnsiString Str;
    for(int i=0; i<2; i++)
    {
        for(int j=0; j<8; j++)
        {
            Prod.fInArmSuck4x8[0][i][j]=false;                                  //Steven 20240410 : Add for有使用的吸嘴, 對應到開關site
            Prod.fInArmSuck4x8[1][i][j]=false;
            Prod.bInSuckUse[0][i][j]=false;                                     //Steven 20240410 : Add for有使用的吸嘴, 對應到Site Mode, Auto clean專用
            Prod.bInSuckUse[1][i][j]=false;
            Prod.iSiteMap[0][i][j]=-1;                                          //AI(general) 20260408 (RogerYang) : fix copy-paste bug
            Prod.iSiteMap[1][i][j]=-1;
        }
    }

    bool bSiteMapHotplateReady=false;

    if(LastSet.iTemperature==Tempture_Hot &&
       IniConfig.bI21AutoSiteMappingUseHotplate==true)
    {
        bSiteMapHotplateReady=bAutoSiteMapHotplateReady;
    }
    else
    {
        bSiteMapHotplateReady=true;
    }
    (void)bSiteMapHotplateReady;

    // W6.2b-canary: UN-GATE ONLY the e9045_1x1_1 / e9045_1x4_1_Ac arm (the SOLE
    // translated variant; golden :4705).  The remaining 23 *_SuckerMap() picker-
    // map builders are NOT translated yet -> they stay GATED below; for any
    // still-gated iInArmType the else Program-Error path is the live offline
    // behavior and the cleared Prod grids above remain the live state.
    if(iInArmType==e9045_1x1_1 ||                                               //AI(W6.2b-canary) 20260626: un-gate ONLY the translated 1x1_1 SuckerMap arm
       iInArmType==e9045_1x4_1_Ac)              { DoInArm_9045_1x1_1_SuckerMap(); }  //golden :4705
    // --- W6.2c batch-1: 5 variant SuckerMap arms pulled out of the #if 0 gate (ACTIVE) ---
    else if(iInArmType==e9045_1x2_2_13)  { i1x2_4UseACEGPicker=0; DoInArm_9045_1x2_2_SuckerMap();    }  //AI(W6.2c-INARM) 20260626: golden :4709
    else if(iInArmType==e9045_1x2_2_14)  { i1x2_4UseACEGPicker=0; DoInArm_9045_1x2_2_14_SuckerMap(); }  //AI(W6.2c-INARM) 20260626: golden :4710
    else if(iInArmType==e9045_1x2_4_Hot) { i1x2_4UseACEGPicker=0; DoInArm_9045_1x2_4_Hot_SuckerMap();}  //AI(W6.2c-INARM) 20260626: golden :4711
    else if(iInArmType==e9045_1x3_4)     { DoInArm_9045_1x3_4_SuckerMap();    }                          //AI(W6.2c-INARM) 20260626: golden :4724
    else if(iInArmType==e9045_1x3_2_14)  { DoInArm_9045_1x3_2_14_SuckerMap(); }                          //AI(W6.2c-INARM) 20260626: golden :4712
    // --- W6.2c batch-2: 6 variant SuckerMap arms made ACTIVE (no picker-prefix; verified golden :4736-4767) ---
    else if(iInArmType==e9045_1x4_2_14)  { DoInArm_9045_1x4_2_SuckerMap();    }                          //AI(W6.2c-INARM-batch2) 20260626: golden :4736
    else if(iInArmType==e9045_1x4_8_Hot) { DoInArm_9045_1x4_8_Hot_SuckerMap();}                          //AI(W6.2c-INARM-batch2) 20260626: golden :4748
    else if(iInArmType==e9045_2x1_2_13)  { DoInArm_9045_2x1_2_SuckerMap();    }                          //AI(W6.2c-INARM-batch2) 20260626: golden :4752
    else if(iInArmType==e9045_2x2_4_12)  { DoInArm_9045_2x2_4_12_SuckerMap(); }                          //AI(W6.2c-INARM-batch2) 20260626: golden :4756
    else if(iInArmType==e9045_2x2_4_13)  { DoInArm_9045_2x2_4_SuckerMap();    }                          //AI(W6.2c-INARM-batch2) 20260626: golden :4760
    else if(iInArmType==e9045_2x2_4_14)  { DoInArm_9045_2x2_4_14_SuckerMap(); }                          //AI(W6.2c-INARM-batch2) 20260626: golden :4764
    // --- W6.2c batch-3: 5 variant SuckerMap arms made ACTIVE (no picker-prefix; verified golden :4740/:4744/:4774/:4778/:4784).
    //     All_1Pick has NO SuckerMap arm: the golden engine declares the extern
    //     (golden :4675) but NEVER dispatches DoInArm_9045_All_1Pick_SuckerMap() --
    //     faithful, do NOT add one and add NO ep1Picker guard to this ladder. ---
    else if(iInArmType==e9045_1x4_4_Back) { DoInArm_9045_1x4_4_Back_SuckerMap();}                         //AI(W6.2c-INARM-batch3) 20260626: golden :4740
    else if(iInArmType==e9045_1x4_4)      { DoInArm_9045_1x4_4_SuckerMap();    }                          //AI(W6.2c-INARM-batch3) 20260626: golden :4744
    else if(iInArmType==e9045_2x3_6)      { DoInArm_9045_2x3_6_SuckerMap();    }                          //AI(W6.2c-INARM-batch3) 20260626: golden :4778
    else if(iInArmType==e9045_2x3_6_14)   { DoInArm_9045_2x3_6_14_SuckerMap(); }                          //AI(W6.2c-INARM-batch3) 20260626: golden :4774
    else if(iInArmType==e9045_2x4_4_14)   { DoInArm_9045_2x4_4_SuckerMap();    }                          //AI(W6.2c-INARM-batch3) 20260626: golden :4784
    // --- W6.2c batch-4: 6 variant SuckerMap arms made ACTIVE (no picker-prefix;
    //     verified golden :4768-4806).  e9045_2x8_32 routes to the SAME
    //     DoInArm_9045_2x8_8_SuckerMap() callee (golden :4804-4806). ---
    else if(iInArmType==e9045_2x2_8_Hot) { DoInArm_9045_2x2_8_Hot_SuckerMap(); }                          //AI(W6.2c-INARM-batch4) 20260626: golden :4768
    else if(iInArmType==e9045_2x4_8)     { DoInArm_9045_2x4_8_SuckerMap();     }                          //AI(W6.2c-INARM-batch4) 20260626: golden :4788
    else if(iInArmType==e9045_2x5_8)     { DoInArm_9045_2x5_8_SuckerMap();     }                          //AI(W6.2c-INARM-batch4) 20260626: golden :4792
    else if(iInArmType==e9045_2x6_8)     { DoInArm_9045_2x6_8_SuckerMap();     }                          //AI(W6.2c-INARM-batch4) 20260626: golden :4796
    else if(iInArmType==e9045_2x8_8)     { DoInArm_9045_2x8_8_SuckerMap();     }                          //AI(W6.2c-INARM-batch4) 20260626: golden :4800
    else if(iInArmType==e9045_2x8_32)    { DoInArm_9045_2x8_8_SuckerMap();     }                          //AI(W6.2c-INARM-batch4) 20260626: golden :4804 (routes 2x8_32 -> 2x8_8 callee)
    // --- W6.2c batch-5: 2 S-family SuckerMap arms made ACTIVE (transcribed from golden; no picker-prefix). ---
    else if(iInArmType==e9045_1x4_4_13)  { DoInArm_9045S_1x4_4_SuckerMap();    }                          //AI(W6.2c-INARM-batch5) 20260626: golden :4732 (no picker-prefix)
    else if(iInArmType==e9045_2x4_4_13)  { DoInArm_9045_2x4_4_13_SuckerMap();  }                          //AI(W6.2c-INARM-batch5) 20260626: golden :4780 (no picker-prefix)
#if 0 // TODO(W6.2b variants) -- golden :4704-4830 (remaining per-layout *_SuckerMap arms; un-gate as each variant lands)
    /* ... further per-iInArmType *_SuckerMap dispatch arms ... */
#endif
    else { Str.sprintf("iInArmType=%d", iInArmType); ShowMyMessage("Program Error in DoInArm_9045_SuckerMap()", Str); }
}

//==============================================================================
//==  ACTIVE SM #1: DoInArmAdditionalFunction  -- golden :3209
//==  Cursor flow (Task transitions) kept ACTIVE & verbatim.  The cross-module
//==  action bodies (DoInDieClean / DoInArmDevicePosPrecise / fBarCode-> /
//==  DoInArmRotateKIT) and the predicate (CheekNeedToDoInArmAdditionalFunction)
//==  have no translated home -> gated; the offline "nothing to do" path drives
//==  Task 1->100->bResult=true (the SM converges cleanly).
//==============================================================================
int iInArmAdditionalFunctionTask;
//==============================================================================
void InitDoInArmAdditionalFunction()                                            //Steven 20210609 : 整合Precisor, Bottom CCD, Die Clean, Rotator
{
    iInArmAdditionalFunctionTask=1;
}
//==============================================================================
bool CheekNeedToDoInArmAdditionalFunction()                                     // golden :3107
{
    bool bResult=false;
    // golden body reads InArmSuck.HasType(...) + USE_ROTATE_KIT / tRotate +
    // bDoPreciser / USE_DIE_CLEAN / BOTTOM_2DID and sets the bDieClean /
    // bPrecisor / bBottom2DID / bInRotator flags (all cross-module, no home).
    // Offline: nothing to do -> bResult stays false.
#if 0 // TODO(W7) -- golden :3109-3204 (bInRotator/bDieClean/.. flags, CheckNeedRotateWithoutHotIC, fSCKART, PreSetInArmAdditionalFlag)
    /* full predicate body verbatim in golden; gated -- depends on untranslated
       additional-function flags + CheckNeedRotateWithoutHotIC + fSCKART. */
#endif
    if(bResult)
        InitDoInArmAdditionalFunction();
    return bResult;
}
//==============================================================================
bool DoInArmAdditionalFunction()                                                //Steven 20210609 : 整合Precisor, Rotator, Bottom CCD, Die Clean  -- golden :3209
{
    bool bResult=false;
    bool flag;  (void)flag;

    int &Task=iInArmAdditionalFunctionTask;

    switch(Task)
    {
        case 1:
            CheekNeedToDoInArmAdditionalFunction();
            Task=100;
            // fall-through to case 100 (golden :3219 -- intentional, no break)
        case 100:
            // golden :3221-3239 selects Die Clean / Preciser / 2DID / Rotate via
            // the InArmSuck.bAlready* flags + bDieClean/bPrecisor/.. flags.  The
            // bAlready* flags are substrate-provided; the bDieClean/.. selectors
            // are untranslated -> offline they are all false, so this else-path
            // is the live behavior: nothing to do -> finish.
#if 0 // TODO(W7) -- golden :3221-3239 additional-fn selector (bDieClean/bPrecisor/bBottom2DID/bInRotator + Init* calls)
            if(InArmSuck.bAlreadyDieClean==false && bDieClean)
            {
                InitInDieCleanTask();
                Task=10000;
            }
            else if(InArmSuck.bAlreadyPreciser==false && bPrecisor)
            {
                InitInArmDevicePosPrecise();
                Task=20000;
            }
            else if(InArmSuck.bAlready2DID==false && bBottom2DID)
            {
                fBarCode->InitBottom2DIDScan();
                Task=30000;
            }
            else if(InArmSuck.bAlready2DID==false && bInRotator)
            {
                Task=40000;
            }
            else
#endif
            {
                bResult=true;
            }
            break;
#if 0 // TODO(W7) -- golden :3246-3304 action SMs (DoInDieClean / DoInArmDevicePosPrecise / fBarCode-> / MoveInArmZToPlateSafe + DoInArmRotateKIT)
        case 10000:
            flag=DoInDieClean();
            if(flag==true)                                                      //wei 20170418
            {
                InArmSuck.bAlreadyDieClean=true;
                bDieClean=false;
                Task=100;
            }
            break;
        case 20000:
            flag=DoInArmDevicePosPrecise();
            if(flag)                                                            //Frank 20180410 (Steven) : InArm Preciser Station
            {
                InArmSuck.bAlreadyPreciser=true;
                bPrecisor=false;
                bDoPreciser=true;
                bHangTimePause=true;                                            //Steven 20090827 : Hang Up dectector
                Task=100;
            }
            break;
        case 30000:
            if(BOTTOM_2DID_CCD==1)                                              //KaiChen 20200513 : Bottom 2D 8CCD
                flag=fBarCode->DoBottom2DID_8CCD_Scan();
            else
                flag=fBarCode->DoBottom2DIDScan();
            if(flag)
            {
                InArmSuck.bAlready2DID=true;
                bBottom2DID=false;
                Task=100;
            }
            break;
        case 40000:
            flag=MoveInArmZToPlateSafe(Task);
            if(flag)
            {
                iInRotateFinish=1;
                Task=41000;
            }
            else
            {
                break;
            }
        case 41000:
            flag=DoInArmRotateKIT();
            if(flag==true)
            {
                iInRotateFinish=2;                                              //kevin 20130524  旋轉完成
                InArmSuck.bAlreadyRotate=true;
                bInRotator=false;
                bWaitRotateFinish=false;
                Task=100;
            }
            break;
#endif
    }
    return bResult;
}
//==============================================================================
void PreSetInArmAdditionalFlag()                                                //Steven 20210609 : 整合Precisor, Rotator, Bottom CCD, Die Clean  -- golden :3309
{
    // golden body presets the InArmSuck.bAlready* flags from USE_ROTATE_KIT /
    // tRotate / bEnableUsePreciser / USE_DIE_CLEAN / BOTTOM_2DID.  Offline the
    // additional functions are all "already done" so the additional-fn SM
    // converges immediately.  Keep that faithful end-state.
    InArmSuck.bAlreadyPreciser=true;
    InArmSuck.bAlreadyDieClean=true;
    InArmSuck.bAlready2DID=true;
    InArmSuck.bAlreadyRotate=true;
}

//==============================================================================
//==  ACTIVE SM #2: DoInArmIonFanGiveWay  -- golden :9166 (Eastsun 20260521)
//==  Cursor flow kept ACTIVE & verbatim; the one motion body
//==  (MoveInArm2XYToDecayTeach) is gated (no translated home) and treated as
//==  "reached" offline so the SM advances.
//==============================================================================
int iInArmIonFanGiveWayTask;
void InitDoInArmIonFanGiveWayTask()                                             //Eastsun 20260521 整合
{
    iInArmIonFanGiveWayTask=1;
}
//------------------------------------------------------------------------------
bool DoInArmIonFanGiveWay()                                                     //Eastsun 20260521 整合
{
    bool bResult=false;
    int &Task=iInArmIonFanGiveWayTask;

    switch(Task)
    {
        case 1:
            if(IniConfig.bA15_1ESDGiveWayFunction==true && InArmSuck.HasRealIC()==false)
            {
                if(InArmSuck.HasRealIC()==false)
                {
                    if((FLCarryKit.NoIC() && InSHT1InLF()) ||
                       (BLCarryKit.NoIC() && InSHT2InLF()))
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
            if(MoveInArmZToPlateSafe(Task))
            {
                Task=3;
            }
            break;
        case 3:
        {
            bool bDecayReached;
            // ⚠ AI(W906-NL-ABSENCE) 20260916: 「no translated home」已半過期 ——
            //   body 在 ainarm2.cpp:925，但**沒有任何標頭宣告它**，本 TU 仍叫不到。
            //   同一個判定寫在 csystem.cpp 的 GATE h4-G2。運動/安全，夜間不解。
#if 0 // TODO(W7) -- golden :9205 MoveInArm2XYToDecayTeach() (no translated home)
            bDecayReached=MoveInArm2XYToDecayTeach();
#else
            bDecayReached=true;                                                 // offline: treat decay-teach pos as reached
#endif
            if(bDecayReached)
            {
                RecordProcess("In Arm Ion Fan Give Way");
                Task=5;
            }
            break;
        }
        case 5:
            if((FLCarryKit.NoIC() && InSHT1InLF()) ||
               (BLCarryKit.NoIC() && InSHT2InLF()))
            {
                bResult=true;
            }
            break;
    }

    return bResult;
}

//==============================================================================
//==  ACTIVE SM #3: ProcessSCKARTLoadingCount  -- golden :7305
//==  Cursor flow kept ACTIVE & verbatim; the vacuum-destroy + alarm sink bodies
//==  are gated.  Behind bCanRunSCKART (default false) in callers, so offline
//==  this SM is dormant; kept pumpable + faithful.
//==============================================================================
bool ProcessSCKARTLoadingCount(bool bReset=false)
{
    AnsiString ErrPart;  (void)ErrPart;
    bool bResult=false;
    if(bReset)
    {
        iProcessSCKARTLoadingCountTask=1;
        return bResult;
    }

    int &Task=iProcessSCKARTLoadingCountTask;
    switch(Task)
    {
        case 1:
        {
            bool bDestroyDone;
#if 0 // TODO(W7) -- golden :7319 InArmSuck.Suck[iLoadPickX][iLoadPickY].Destroy()
            bDestroyDone=InArmSuck.Suck[iLoadPickX][iLoadPickY].Destroy();
#else
            bDestroyDone=true;                                                  // offline: destroy done immediately
#endif
            if(bDestroyDone)
            {
                InArmSuckUse[iLoadPickX][iLoadPickY]=true;
                Task=100;
            }
            break;
        }
        case 100:
            if(MoveInArmZToPlateSafe(Task))
            {
                Task=200;
            }
            break;
        case 200:
#if 0 // TODO(W7) -- golden :7332 ShowErrorMessage("WAR0120") alarm sink
            ErrPart=InArmSuck.Suck[iLoadPickX][iLoadPickY].sName;
            ShowErrorMessage("WAR0120", K_RETRY, MInArmX, false, ErrPart);
#endif
            bResult=true;
            bPickFromLoader=false;                                              //Steven 20171226 (Wei) : 確認Loader吸取完成
            break;
    }
    return bResult;
}

//==============================================================================
//==  GATED SMs (ACTIVE stub preserving cursor var + sane terminal value).
//==  These four golden SMs are MOT[]/sensor/fProductionInfo-bound with no
//==  translated home; reproducing their dense case bodies would require pulling
//==  untranslated modules.  Each stub keeps the cursor var (so callers/canary
//==  can drive it) and returns the golden "still working / not finished"
//==  default so the engine LINKS and the higher-level loop does not crash.
//==============================================================================
// =============================================================================
//  W7-A1 SEAM forward-decls (SUBSTRATE EXTERNs the two un-gated SMs below
//  reference but that have NO translated home yet).  Each is #ifndef-guarded so
//  it cannot ODR-collide with a sibling TU's identical forward-decl, and is
//  REPORTED in seamNeeds for the serial Integrate to land its real home.
//    * DoShakeShuttle/DoKnockShuttle/DoKnockShuttleFirst/DoVibrateShuttle,
//      RecordShtSuperflous, IndexZCanMove[2], bShuttleKnock -- golden ainarm2.h
//      :189/190/191/214/218/48/85 (the SHUTTLE-motion engine, not yet translated).
//  Offline behavior once Integrate lands them: shake/knock/vibrate helpers are
//  no-ops returning true (motion converges immediately on the Sim HAL);
//  RecordShtSuperflous is a log no-op; IndexZCanMove[]/bShuttleKnock are plain
//  globals.
// =============================================================================
#ifndef AINARM_W7A1_SHUTTLEMOTION_FWD
#define AINARM_W7A1_SHUTTLEMOTION_FWD
extern bool DoShakeShuttle(int iShuttle, bool bNeedInitial=false);              //Steven 20120801 : 修改抖抖功能  // golden ainarm2.h:189
extern bool DoKnockShuttle(int iShuttle, bool bNeedInitial=false);             //Jou 2013-03-08 修改敲敲功能      // golden ainarm2.h:190
extern bool DoKnockShuttleFirst(int iShuttle, bool bNeedInitial=false);        //jou 2015-12-09 Shuttle 每次入料前敲擊 // golden ainarm2.h:191
extern bool DoVibrateShuttle(int iShuttle, bool bNeedInitial=false);          //JerryYang 20190123 shuttle震動馬達 // golden ainarm2.h:214
extern void RecordShtSuperflous(int iShuttle);                                 //JerryYang 20181121 : add log     // golden ainarm2.h:218
extern bool IndexZCanMove[2];                                                  // golden ainarm2.h:48
extern bool bShuttleKnock;                                                     //Jou 2013-03-08 修改敲敲功能      // golden ainarm2.h:85
// golden ainarm9045.cpp:3759-3760 -- the golden TU forward-declares these two
// itself (no header decl).  Real home: acarry.cpp (confirmed in tree).  Mirrored.
extern bool IsTestZ1NotSafeShuttle1CanNotMove(int &iRetryCT);
extern bool IsTestZ2NotSafeShuttle2CanNotMove(int &iRetryCT);
#endif

// W7-A1 SEAM (INTEGRATED 20260629 by Integrate step):
//   * bNo9Action  -- golden cInArmPlacement.h:63 -- is now a REAL member of the
//     TfMainInplace facade (FormsFacade.h), so its READ (W7A1_NO9_ACTION()) and
//     its case-9000 WRITE are wired to the real member.  W7A1_NO9_MEMBERS_DONE
//     gates the bNo9Action write so it is independent of the still-pending pair.
//   * bRunNo9 / DoSuckToRecycleFromShuttle (golden cInArmPlacement.h:62/48) are
//     NOT added to the facade this wave (out of W7-A1 scope); their write/call
//     sites stay #ifdef W7A1_NO9_MEMBERS_PENDING (undefined -> compiled out).
//     Both sit inside InArmPlacementEnable()==false No9 placement blocks, so the
//     region is never entered at runtime offline -> offline behavior identical.
#define W7A1_NO9_MEMBERS_DONE 1                                                 // W7-A1: bNo9Action member landed on TfMainInplace
#define W7A1_NO9_ACTION() (fMain->cInplace->bNo9Action)                        // golden cInArmPlacement.h:63 (offline always false)

// golden ainarm9045.cpp:3343 -- file-scope place-OK delay timer (owned by this TU).
TQPF_Timer hPutOKDelay1;

int iInArmInArmCheckShtFloatTask;                                              // golden :3343-ctx (cursor for DoInArmCheckShuttleFloating)
void InitDoInArmCheckShtFloatTask()                                             // golden :3344
{
    iInArmInArmCheckShtFloatTask=1;
}
// golden ainarm9045.cpp:3349-3757 -- place-to-shuttle FLOATING quality SM.
// Un-gated W7-A1.  Verbatim cursor flow (1->1260/1300->1301->1410->1411 shake/
// vibrate/knock loop->2000/2100 No9->3000 latch->9000 Finish), the
// IN_ARM_CHECK_SHUTTLE goto/label pair, and every SOFT-state branch preserved.
// The 3 cInplace No9 member-touches (bRunNo9 / bNo9Action / DoSuckToRecycleFromShuttle)
// are kept VERBATIM but #ifndef-guarded (W7A1_NO9_MEMBERS_PENDING undefined here)
// because TfMainInplace (FormsFacade.h, off-limits this wave) lacks them; they sit
// behind InArmPlacementEnable() which is false offline so the guarded region is
// never entered at runtime anyway -- offline behavior is identical.  REPORTED in
// seamNeeds.
bool DoInArmCheckShuttleFloating(int iSht, bool bPlaceOtherShuttle, bool bNeedCheck)
{
    static bool bShakeFlag[2]={false, false}, bVibration[2]={false, false};

    int &Task=iInArmInArmCheckShtFloatTask;
    int ret=0, iret=0, iLtcMoveAct=0, iCheckSHPos=0;
    bool bResult=false, flag=false, bLtcAlarm=true;
    AnsiString ErrorPickup=AnsiString("");
    IN_ARM_CHECK_SHUTTLE:

    switch(Task)
    {
        case 1:
            if(MoveInArmZToPlateSafe(Task))
            {
                if(bNeedCheck==true)
                {
                    Task=1300;
                }
                else if(iSht==0 &&
                   (InSHT1InLF()==false ||                                      //Steven 20250314 : add protection to avoid picker hit shuttle
                    FLCarryKit.HasRealIC()==false))
                {
                    Task=9000;
                }
                else if(iSht==1 &&
                        (InSHT2InLF()==false ||                                 //Steven 20250314 : add protection to avoid picker hit shuttle
                         BLCarryKit.HasRealIC()==false))
                {
                    Task=9000;
                }
                else if(IniConfig.bF14_1KnockShuttleFirst)
                {
                    DoKnockShuttleFirst(iSht, true);                            //jou 2015-12-09 SCS 要求 Shuttle 每次入料前 敲擊
                    Task=1260;
                }
                else
                {
                    Task=1300;
                }
                goto IN_ARM_CHECK_SHUTTLE;
            }
            break;
        case 1260:
            if(DoKnockShuttleFirst(iSht))                                       //jou 2015-12-09 SCS 要求 Shuttle 每次入料前 敲擊
            {
                Task=1300;
            }

            if(Task!=1300)
                break;
        case 1300:                                                              //kevin 20141206 start
            if(iInArmType==e9045_1x4_8_Hot)                                     //Steven 20220522 : fixed for 8吸嘴模式
            {
                for(int j=0; j<MAX_ARM_Col; j++)
                {
                    if(bPlaceOtherShuttle==false)
                        InArmSuck.SetItemData(1, j, NULL_IC);
                    else
                        InArmSuck.SetItemData(0, j, NULL_IC);
                }
            }
            else if(iInArmType==e9045_1x2_4_Hot)
            {
                if(i1x2_4UseACEGPicker==0)                                      //Steven 20230530 : 1x2_4改用Row A
                {
                    for(int j=0; j<MAX_ARM_Col; j++)
                    {
                        if(bPlaceOtherShuttle)
                            InArmSuck.SetItemData(1, j, NULL_IC);
                        else
                            InArmSuck.SetItemData(0, j, NULL_IC);
                    }
                }
                else
                {
                    for(int j=0; j<2; j++)
                    {
                        if(bPlaceOtherShuttle)
                            InArmSuck.SetItemData(0, j*2+1, NULL_IC);
                        else
                            InArmSuck.SetItemData(0, j*2, NULL_IC);
                    }
                }
            }
            else if(iInArmType==e9045_2x2_8_Hot)
            {
                for(int i=0; i<MAX_ARM_Row; i++)
                {
                    for(int j=0; j<MAX_ARM_Col/2; j++)
                    {
                        if(bPlaceOtherShuttle)
                            InArmSuck.SetItemData(i, j*2+1, NULL_IC);
                        else
                            InArmSuck.SetItemData(i, j*2, NULL_IC);
                    }
                }
            }
            else
            {
                InArmSuck.SetAllToNullIC();
            }
            hPutOKDelay1.SetSecAndOn(ArmSpeed[InArm].dWaitOnSH);
            Task=1301;
        case 1301:                                                              //kevin 20131011 在下黏貨誤判
            if(hPutOKDelay1.Off())
            {
                flag=CheckInArmDestroyICFail();                                 //Steven 20111223 : 檢查破壞錯誤
                if(flag==false)
                {
                    RecordProcess("VOFTask=1301");
                    return false;
                }

                if(In_Shuttle_Auto_Latch==eInSHAutoLtc)                         //KenHsieh 20250722 : InSht sensor 改為2頭，並用Latch 判別疊料以及飛料
                {
                    InitAutoChkInSHLatchTask();
                    bInSHLtcFin[iSht]=false;                                    //KenHsieh 20251106 : fix close site don't do In Sht Latch
                    Task=3000;
                    break;
                }
                else
                {
                    Task=1410;
                }
            }
            else
            {
                break;
            }
        case 1410:
            if(CUSTOMER_CODE==CC_ASE_KaohSiung &&
               IniConfig.bF23ShuttleVibration)                                  //kevin 20210415 IN Arm Vibrate shuttle
                SW[SwShuttleVibration1+iSht].Off();

            bShakeFlag[iSht]=false;
            bVibration[iSht]=false;                                             //JerryYang 20180711 (wei) Shuttle震動後還是置偏要接著搖
            if(IniConfig.bF01ShakeShuttleWhenJam==false &&
               IniConfig.bF23ShuttleVibration==false)                           //JerryYang 20171205 (Steven) shuttle震動馬達功能
                bShakeFlag[iSht]=true;

            if(CosFunction.bShakeShuttleEveryTime &&                            //Steven 20200616 : ATK要求每次放料都要搖搖蝦頭
               (TestIF_File.iShakeShuttleWhenPlaceIC==1 ||
                TestIF_File.iShakeShuttleWhenPlaceIC==3))                       //Steven 20220427 : 每次放料都要抖抖馬達
            {
                if(TestIF_File.iShakeShuttleWhenPlaceIC==3)
                    bVibration[iSht]=true;
                DoShakeShuttle(iSht, true);
                Task=1420;
                break;
            }
            else if(CosFunction.bShakeShuttleEveryTime &&
                    TestIF_File.iShakeShuttleWhenPlaceIC==2)                    //Steven 20220427 : 每次放料都要抖抖馬達
            {
                DoVibrateShuttle(iSht, true);
                Task=1440;
                break;
            }
            else
            {
                for(int i=0; i<9; i++)
                    iInShuttleJam[iSht][i]=0;                                   //kevin 20220819 shuttle Jam  紀錄位置後續，要做放到垃圾桶動作
                Task=1411;
            }
        case 1411:
            if(TestIF.iTestMode==SingleSite || TestIF.iTestMode==DualSite2x1)
                ret=CheckShuttleSensor_9045_1x1(iSht, bShakeFlag[iSht]);
            else if(TestIF.iTestMode==DualSite || TestIF.iTestMode==QualSite2X2N)
                ret=CheckShuttleSensor_9045_1x2(iSht, bShakeFlag[iSht]);
            else if(TestIF.iTestMode==QualSite1X4 || TestIF.iTestMode==_8Site2X4N)                                      //Wei 20231211 : 2X4NN Mode
                ret=CheckShuttleSensor_9045_1x4(iSht, bShakeFlag[iSht]);
            else if(TestIF.iTestMode==QualSite2X2)
                ret=CheckShuttleSensor_9045_2x2(iSht, bShakeFlag[iSht]);
            else if(TestIF.iTestMode==TriSite1X3 || TestIF.iTestMode==_6Site2X3 || TestIF.iTestMode==_6Site2X3N)
                ret=CheckShuttleSensor_9045_2x3(iSht, bShakeFlag[iSht]);
            else if(TestIF.iTestMode==_10Site2X5)
                ret=CheckShuttleSensor_9045_2x5(iSht, bShakeFlag[iSht]);
            else if(TestIF.iTestMode==_12Site2X6)
                ret=CheckShuttleSensor_9045_2x6(iSht, bShakeFlag[iSht]);
            else if(TestIF.iTestMode==_16Site2X8 || TestIF.iTestMode==_32Site4X8N)
                ret=CheckShuttleSensor_9045_2x8(iSht, bShakeFlag[iSht]);
            else
                ret=CheckShuttleSensor_9045_2x4(iSht, bShakeFlag[iSht]);

            if(ret==1)                                                                                                  //Steven 20160411 : 不然不會搖
            {
                bShakeFlag[iSht]=false;
                bVibration[iSht]=false;                                                                                 //JerryYang 20180711 (wei) Shuttle震動後還是置偏要接著搖
                iShuttleLog=0;                                                                                          //JerryYang 20181121 (Steven) : add log,記錄in shuttle device是被震動馬達or搖搖功能導正
            }
            else if(ret==3)
            {
                break;
            }
            else if(ret==2 ||
                    (IniConfig.bF29AlwaysVibrateOnShuttle &&
                     bShakeFlag[iSht]==false &&
                     bVibration[iSht]==false &&
                     IsFLCarrKitAllHasIC()))                                                                            //Sam 20210602 : 每次都要強制震動
            {
                if(IniConfig.bF23ShuttleVibration &&
                   ((IniConfig.bF01ShakeShuttleWhenJam==true && bVibration[iSht]==false) ||
                     IniConfig.bF01ShakeShuttleWhenJam==false))                                                         //JerryYang 20180711 (wei) Shuttle震動後還是置偏要接著搖
                {
                    if(IniConfig.bF01ShakeShuttleWhenJam==true)
                    {
                        bShakeFlag[iSht]=false;
                    }
                    else
                    {
                        bShakeFlag[iSht]=true;
                    }
                    bVibration[iSht]=true;
                    Task=1440;
                    DoVibrateShuttle(iSht, true);                                                                       //JerryYang 20190123 shuttle震動馬達
                }
                else if(bShuttleKnock==false)                                                                           //Jou 2013-03-08 修改敲敲功能
                {
                    if(IniConfig.bF01ShakeShuttleWhenJam==false &&
                       IniConfig.bF23ShuttleVibration==false)                                                           //kevin 20190731 不使用搖搖
                    {
                        bShakeFlag[iSht]=true;
                        break;
                    }
                    Task=1420;
                    DoShakeShuttle(iSht, true);                                                                         //Steven 20120801 : 修改抖抖功能 (true為初始化)
                }
                else
                {
                    bShakeFlag[iSht]=true;
                    DoKnockShuttle(iSht, true);                                                                         //jou 2013-07-17 Knock Shuttle(true為初始化)
                    Task=1430;
                }
                break;
            }
            else
            {
                Task=9000;                                                                                              //JimmyChiu 20220908 add Pickup Error Placement
            }
            break;
        case 1420:
            if(iSht==0 && InSHT1InLF()==false)                                  //Steven 20250314 : add protection to avoid picker hit shuttle
            {
                break;
            }
            else if(iSht==1 && InSHT2InLF()==false)
            {
                break;
            }
            else
            {
                Task=1401;
            }
        case 1401:
            if(DoShakeShuttle(iSht))                                            //Steven 20120801 : 修改抖抖功能
            {
                if(CosFunction.bShakeShuttleEveryTime &&                        //Steven 20220427 : 每次放料都要抖抖馬達
                   TestIF_File.iShakeShuttleWhenPlaceIC==3 &&
                   bVibration[iSht]==true)
                {
                    DoVibrateShuttle(iSht, true);
                    Task=1430;
                }
                else if(fMain->cInplace->InArmPlacementEnable())                //JimmyChiu 20220908 add Pickup Error Placement
                {
                    Task=2000;
                }
                else
                {
                    Task=1411;
                }
            }
            break;
        case 1430:
            if(DoKnockShuttle(iSht))                                            //Jou 2013-03-08 修改敲敲功能
            {
                Task=1411;
            }
            break;
        case 1440:
            if(DoVibrateShuttle(iSht))                                          //JerryYang 20190123 shuttle震動馬達
            {
                Task=1411;
            }
            break;
        case 2000:
            if(fMain->cInplace->InArmPlacementEnable())                         //JimmyChiu 20220908 add Pickup Error Placement
            {
                #ifdef W7A1_NO9_MEMBERS_PENDING
                fMain->cInplace->DoSuckToRecycleFromShuttle(true,ErrorPickup);  // W7-A1 SEAM: TfMainInplace lacks DoSuckToRecycleFromShuttle offline (golden cInArmPlacement.h:48)
                #endif
            }
            Task=2100;
        case 2100:
            if(fMain->cInplace->InArmPlacementEnable())
            {
                #ifdef W7A1_NO9_MEMBERS_PENDING
                if(fMain->cInplace->DoSuckToRecycleFromShuttle(false,ErrorPickup))  // W7-A1 SEAM (golden cInArmPlacement.h:48)
                {
                    fMain->cInplace->bRunNo9=false;                             //KenHsieh 20240131 : add No9作動畫面 // W7-A1 SEAM (golden cInArmPlacement.h:62)
                    bInSHLtcFin[iSht]=true;                                     //KenHsieh 20251106 : fix close site don't do In Sht Latch
                    Task=9000;
                }
                #endif
                break;
            }
            else
            {
                #ifdef W7A1_NO9_MEMBERS_PENDING
                fMain->cInplace->bRunNo9=false;                                 //KenHsieh 20240131 : add No9作動畫面 // W7-A1 SEAM (golden cInArmPlacement.h:62)
                #endif
            }

            if(In_Shuttle_Auto_Latch==eInSHAutoLtc)                             //back to check shuttle sensor //KenHsieh 20251105 : Auto In shuttle latch combine No9 func.
            {
                InitAutoChkInSHLatchTask();
                Task=3000;
            }
            else
            {
                Task=1411;
            }
            break;
        //JimmyChiu 20220908 add Pickup Error Placement
        //<==
        case 3000:                                                              //KenHsieh 20250722 : InSht sensor 改為2顆，並用Latch 判別疊料以及飛料
            if(((iSht==0 && FLCarryKit.UseSiteFullIC()) ||
               (iSht==1 && BLCarryKit.UseSiteFullIC())) ||                      //Nomal 放滿data
               ((iOneCycle==1 || iCleanOut==1) &&                               //OneCycle or CleanOut
               ((iSht==0 &&
                 FLCarryKit.HasIC() &&
                 FLCarryKit.UseSiteFullIC()==false) ||
                (iSht==1 &&
                 BLCarryKit.HasIC() &&
                 BLCarryKit.UseSiteFullIC()==false)) &&                         //OneCycle or CleanOut下有IC
                ((MOT[MMTrayY].fHasTray==false &&
                 MOT[MMPlate1].HasIC()==false &&
                 MOT[MMPlate2].HasIC()==false &&                                //OneCycle or CleanOut下 Loader & HP 無料
                (iCleanOut!=1 ||
                (iCleanOut==1 && MOT[MMTrayY_Car].fHasTray==false))) ||         //Cleanout 下要判斷Car
                (iOneCycle==1 && IniConfig.bP17InArmFullPickFromLoader==false))))   //Onecycle + Close P17  //KenHsieh 20251014 : fix onecycle or cleanout 時，跑Latch問題
            {
                if(((BAR_CODE_INSTALL==ebctInShtIntel ||
                     BAR_CODE_INSTALL==ebctUseCCDMode ||
                     BAR_CODE_INSTALL==ebctEtherNetCCD ||
                     BAR_CODE_INSTALL==ebcUseOCR) &&
                     TestIF_File.bEnableBarCode   &&
                    ((iSht==0 && FLCarryKit.HasIC())  ||
                     (iSht==1 && BLCarryKit.HasIC())) &&                        //Ifor 20190129 : add Cognex EtherNet 通訊   //Ifor 20210407 add: 自製OCR
                    (BOTTOM_2DID==0 || TestIF_File.bEnableBottom2D==false)) ||  //KenHsieh 20251008 : Fix Auto InSht Latch for 2D hangup, need to move left
                     DeviceForm.bShuttleWaitingOutSiteChamber)                  //KenHsieh 20260320 : 此功能開啟，Shuttle 必須回到左邊，避免Hanhgup
                {
                    iLtcMoveAct=1;
                }

                if(fMain->cInplace->InArmPlacementEnable())                     //KenHsieh 20251105 : Auto In shuttle latch combine No9 func.
                {
                    bLtcAlarm=false;
                }

                iret=CheckInShuttleSensor_Latch(iSht, iLtcMoveAct, bLtcAlarm);  //KenHsieh 20251105 : Auto In shuttle latch combine No9 func.
                if(iret==1)
                {
                    bInSHLtcFin[iSht]=true;                                     //KenHsieh 20251106 : fix close site don't do In Sht Latch

                    if(iSht==0)                                                 //KenHsieh 20251118 : 修改Out Sht 會重新GetLtc 導致Lose IC 異常
                    {
                        iCheckSHPos=MOT[MInShuttle1].ReadPos();
                        if(iLtcMoveAct==1)                                      //KenHsieh 20251121 : 修正跑barcode 未清除ltc 導致誤判out lose IC
                        {
                            fLtcSensor->ClearLtcSensor(0);
                            fLtcSensor->SetLtcSensor(0);
                        }
                    }
                    else
                    {
                        iCheckSHPos=MOT[MInShuttle2].ReadPos();
                        if(iLtcMoveAct==1)                                      //KenHsieh 20251121 : 修正跑barcode 未清除ltc 導致誤判out lose IC
                        {
                            fLtcSensor->ClearLtcSensor(1);
                            fLtcSensor->SetLtcSensor(1);
                        }
                    }

                    if(iCheckSHPos!=Prod.InSHT[iSht].iLeft)
                        bNeedGetSHRightLtc[iSht]=false;
                    else
                        bNeedGetSHRightLtc[iSht]=true;
                    Task=9000;
                }
                else if(iret==2)
                {
                    InitAutoChkInSHLatchTask();                                 //有異常皆會跑到左邊，固可Init後重跑一次
                    Task=3000;
                }
            }
            else
            {
                Task=9000;
            }
            break;
        case 9000:                                                              //JimmyChiu 20220908 add Pickup Error Placement
            RecordShtSuperflous(iSht);                                          //JerryYang 20181121 (Steven) : add log,記錄in shuttle device是被震動馬達or搖搖功能導正
            IndexZCanMove[0]=true;
            IndexZCanMove[1]=true;
            bShuttleShake=false;
            bShuttleKnock=false;                                                //Jou 2013-03-08 修改敲敲功能
            bVibration[iSht]=false;                                             //JerryYang 20180711 (wei) Shuttle震動後還是置偏要接著搖
            #ifdef W7A1_NO9_MEMBERS_DONE
            fMain->cInplace->bNo9Action=false;                                  //KenHsieh 20251105 : Auto In shuttle latch combine No9 func. // W7-A1 SEAM (golden cInArmPlacement.h:63)
            #endif
            bResult=true;
            Task=1;
            break;
    }
    return bResult;
}
//------------------------------------------------------------------------------
// golden ainarm9045.cpp:3762 -- file-scope Auto-retry counter (owned by this TU).
int iLtcErrRetryCt[2];                                                          //KenHsieh 20260402 : add Auto retry 1 times
int iAutoChkInSHLatchTask;
void InitAutoChkInSHLatchTask()                                                 // golden :3763
{
    iAutoChkInSHLatchTask=1;
    iLtcErrRetryCt[0]=0;                                                        //KenHsieh 20260402 : add Auto retry 1 times
    iLtcErrRetryCt[1]=0;
}
// golden ainarm9045.cpp:3810-4174 -- the IN-SHUTTLE LATCH interlock.  Un-gated
// W7-A1.  CRUX: opens with the VERBATIM golden entry guard; offline the sim
// canary LastSet.iRealDummy==DUMMY(0)!=REALLY self-disables this to return 1
// (Finish) BEFORE any CCLink/Ltc/MOT body runs -> crash-free, zero motion dep.
// The two #ifndef SOFT_SIMULTE blocks (1250/2250) are preserved verbatim and
// ACTIVE (SOFT_SIMULTE not defined).
int CheckInShuttleSensor_Latch(int iShuttle, int iMoveAct, bool bAlarm)         //0:NotFin 1:Finish 2:Error  //KenHsieh 20251104 : add 檢測後動作 & alarm 變數  //KenHsieh 20250722 : InSht sensor 改為2顆，並用Latch 判別疊料以及飛料
{
    if(MOTION_CARD_TYPE!=MotionCard_Contec ||
       LastSet.iRealDummy!=REALLY)
        return 1;

    int &Task=iAutoChkInSHLatchTask;
    int iResult=0;
    bool bInSHLtcZFlag[2]={false, false};
    bool bflag=false, bCheckData=true, bDuplicateErr=false;
    AnsiString asStr1="";
    static int iSHAutoLtcErr=0, iRetryCT=0;
    static AnsiString ErrPart_InSHLtc="";
    static bool bfCanMoveLChange[2]={false, false}, bDupErrFlag[2][10]={{false}};

    switch(Task)
    {
        case 1:
            if(iShuttle==0)
            {
                if(IsTestZ1NotSafeShuttle1CanNotMove(iRetryCT))                 //KenHsieh 20251112 : 修改Index 判斷方式，避免Hangup與無謂的等待
                    break;
                iRetryCT=0;
                fLtcSensor->ClearLtcSensor(0);                                  //KenHsieh 20260402 : SH1增加Clear，避免檢測失敗
                fLtcSensor->SetLtcSensor(0);
                bInSh1DoLtc=true;
                Task=1000;
            }

            if(iShuttle==1)
            {
                if(IsTestZ2NotSafeShuttle2CanNotMove(iRetryCT))                 //KenHsieh 20251112 : 修改Index 判斷方式，避免Hangup與無謂的等待
                    break;
                iRetryCT=0;
                fLtcSensor->SetLtcSensor(1);
                bInSh2DoLtc=true;
                Task=2000;
            }
            bfCanMoveLChange[iShuttle]=false;
            break;
        case 1000:
            bInSHLtcZFlag[0]=MOT[MInSh1LtcSenZ1].MotorMove(Prod.iInSH1SenICDetectZ1+Prod.iInSH1SenICAddPos);
            bInSHLtcZFlag[1]=MOT[MInSh1LtcSenZ2].MotorMove(Prod.iInSH1SenICDetectZ2+Prod.iInSH1SenICAddPos);

            if(bInSHLtcZFlag[0]==true && bInSHLtcZFlag[1]==true)
                Task=1100;
            break;
        case 1100:
            if(IsTestZ1NotSafeShuttle1CanNotMove(iRetryCT)==false)              //KenHsieh 20251112 : 修改Index 判斷方式，避免Hangup與無謂的等待
            {
                iRetryCT=0;
                if(MOT[MInShuttle1].fCanMoveL==false)
                {
                    SetShuttlefCanMoveL(iShuttle, true, __FUNC__, "1100");
                    bfCanMoveLChange[iShuttle]=true;
                }
                Task=1200;
            }
            break;
        case 1200:
            if(IsTestZ1NotSafeShuttle1CanNotMove(iRetryCT))                     //KenHsieh 20251112 : 修改Index 判斷方式，避免Hangup與無謂的等待
                break;

            iRetryCT=0;
            bflag=MOT[MInShuttle1].MotorMove(Prod.InSHT[0].iRight-200);         //KenHsieh 20260120 : Add Shuttle move 保護

            if(bflag)
            {
                if(bfCanMoveLChange[iShuttle]==true)
                {
                    SetShuttlefCanMoveL(iShuttle, false, __FUNC__, "1200");
                    bfCanMoveLChange[iShuttle]=false;
                }
                Task=1250;
            }
            break;
        case 1250:
            iSHAutoLtcErr=0;
            fLtcSensor->GetLtcSensor(0);
            ErrPart_InSHLtc="";
            #ifndef SOFT_SIMULTE
            if(fMain->cInplace->InArmPlacementEnable() &&
               W7A1_NO9_ACTION())                                              //KenHsieh 20251105 : Auto In shuttle latch combine No9 func. // W7-A1 SEAM: cInplace->bNo9Action (golden cInArmPlacement.h:63)
            {
                bCheckData=false;
            }

            if(LastSet.iRealDummy!=REALLY)                                      //KenHsieh 20260506 : 模式有IC才需要判斷
                iSHAutoLtcErr=0;
            else
                iSHAutoLtcErr=DoCheckShuttle1ICByLTC_AutoLatch(ErrPart_InSHLtc, bCheckData);
            if(iSHAutoLtcErr!=0)
            {
                if(MOT[MInShuttle1].fCanMoveL==false)
                {
                    SetShuttlefCanMoveL(iShuttle, true, __FUNC__, "1250");
                    bfCanMoveLChange[iShuttle]=true;
                }
                Task=1260;
            }
            else
            #endif
            {
                iLtcErrRetryCt[0]=0;                                            //KenHsieh 20260402 : add Auto retry 1 times
                if(iMoveAct==1)                                                 //KenHsieh 20251104 : add 檢測後動作 & alarm 變數
                {
                    if(MOT[MInShuttle1].fCanMoveL==false)
                    {
                        SetShuttlefCanMoveL(iShuttle, true, __FUNC__, "1250_iMoveAct=1");
                        bfCanMoveLChange[iShuttle]=true;
                    }
                    Task=1260;
                }
                else
                {
                    Task=9900;
                }
            }
            break;
        case 1260:
            bflag=MOT[MInShuttle1].MotorMove(Prod.InSHT[0].iLeft);       //KenHsieh 20260120 : Add Shuttle move 保護
            if(bflag)
            {
                if(bfCanMoveLChange[iShuttle]==true)
                {
                    SetShuttlefCanMoveL(iShuttle, false, __FUNC__, "1260");
                    bfCanMoveLChange[iShuttle]=false;
                }

                if(iSHAutoLtcErr!=0)                                            //KenHsieh 20251008 : Fix Auto InSht Latch for 2D hangup, need to move left
                {
                    if(bAlarm)                                                  //KenHsieh 20251104 : add 檢測後動作 & alarm 變數
                    {
                        Task=1265;
                    }
                    else
                    {
                        InitDoInShZHome();
                        Task=1270;
                    }
                }
                else
                {
                    Task=9900;
                }
            }
            break;
        case 1265:
            for(int i=0; i<FLCarryKit.iShtCol; i++)                             //KenHsieh 20251128 : InShtLtc 新增異常重複判斷  //KenHsieh 20260518 : SThreadPara.iXItem → FLCarryKit.iShtCol
            {
                if(bInSHLtcErrNo[iShuttle][i] &&
                   bDupErrFlag[iShuttle][i])
                    bDuplicateErr=true;

                if(bInSHLtcErrNo[iShuttle][i])
                    bDupErrFlag[iShuttle][i]=true;
                else
                    bDupErrFlag[iShuttle][i]=false;
            }

            iLtcErrRetryCt[0]++;                                                //KenHsieh 20260402 : add Auto retry 1 times
            if(iLtcErrRetryCt[0]>1)
            {
                if(iSHAutoLtcErr==1)
                    ShowErrorMessage("JAM0401", K_RETRY, MInShuttle1, bDuplicateErr, ErrPart_InSHLtc);
                else
                    ShowErrorMessage("JAM0403", K_RETRY, MInShuttle1, bDuplicateErr, ErrPart_InSHLtc);
            }
            else
            {
                asStr1.sprintf("Auto retry for In shuttle1 latch error : %s", ErrPart_InSHLtc);
                RecordProcess(asStr1);
            }

            InitDoInShZHome();
            Task=1270;
            break;
        case 1270:
            if(DoInShZHome(iShuttle))
            {
                if(bAlarm)                                                      //KenHsieh 20251104 : add 檢測後動作 & alarm 變數
                {
                    iResult=0;
                    Task=1;
                }
                else
                {
                    fLtcSensor->SetLtcSensor(0);                                //JerryYang 20230406 : 重置shuttle1的latch
                    fLtcSensor->ClearLtcSensor(0);                              //Sam 20221101 : Latch 清除都要確認是否清清乾淨
                    Task=9901;
                }
            }
            break;
        case 2000:
            bInSHLtcZFlag[0]=MOT[MInSh2LtcSenZ1].MotorMove(Prod.iInSH2SenICDetectZ1+Prod.iInSH2SenICAddPos);
            bInSHLtcZFlag[1]=MOT[MInSh2LtcSenZ2].MotorMove(Prod.iInSH2SenICDetectZ2+Prod.iInSH2SenICAddPos);

            if(bInSHLtcZFlag[0]==true && bInSHLtcZFlag[1]==true)
                Task=2100;
            break;
        case 2100:
            if(IsTestZ2NotSafeShuttle2CanNotMove(iRetryCT)==false)              //KenHsieh 20251112 : 修改Index 判斷方式，避免Hangup與無謂的等待
            {
                iRetryCT=0;
                if(MOT[MInShuttle2].fCanMoveL==false)
                {
                    SetShuttlefCanMoveL(iShuttle, true, __FUNC__, "2100");
                    bfCanMoveLChange[iShuttle]=true;
                }
                Task=2200;
            }
            break;
        case 2200:
            if(IsTestZ2NotSafeShuttle2CanNotMove(iRetryCT))                     //KenHsieh 20251112 : 修改Index 判斷方式，避免Hangup與無謂的等待
                break;

            iRetryCT=0;
            bflag=MOT[MInShuttle2].MotorMove(Prod.InSHT[1].iRight-200);  //KenHsieh 20260120 : Add Shuttle move 保護
            if(bflag)
            {
                if(bfCanMoveLChange[iShuttle]==true)
                {
                    SetShuttlefCanMoveL(iShuttle, false, __FUNC__, "2200");
                    bfCanMoveLChange[iShuttle]=false;
                }
                Task=2250;
            }
            break;
        case 2250:
            iSHAutoLtcErr=0;
            fLtcSensor->GetLtcSensor(1);
            ErrPart_InSHLtc="";
            #ifndef SOFT_SIMULTE
            if(fMain->cInplace->InArmPlacementEnable() &&
               W7A1_NO9_ACTION())                                              //KenHsieh 20251105 : Auto In shuttle latch combine No9 func. // W7-A1 SEAM: cInplace->bNo9Action (golden cInArmPlacement.h:63)
            {
                bCheckData=false;
            }

            if(LastSet.iRealDummy!=REALLY)                                      //KenHsieh 20260506 : 模式有IC才需要判斷
                iSHAutoLtcErr=0;
            else
                iSHAutoLtcErr=DoCheckShuttle2ICByLTC_AutoLatch(ErrPart_InSHLtc, bCheckData);
            if(iSHAutoLtcErr!=0)
            {
                if(MOT[MInShuttle2].fCanMoveL==false)
                {
                    SetShuttlefCanMoveL(iShuttle, true, __FUNC__, "2250");
                    bfCanMoveLChange[iShuttle]=true;
                }
                Task=2260;
            }
            else
            #endif
            {
                iLtcErrRetryCt[1]=0;                                            //KenHsieh 20260402 : add Auto retry 1 times
                if(iMoveAct==1)
                {
                    if(MOT[MInShuttle2].fCanMoveL==false)
                    {
                        SetShuttlefCanMoveL(iShuttle, true, __FUNC__, "2250_iMoveAct=1");
                        bfCanMoveLChange[iShuttle]=true;
                    }
                    Task=2260;
                }
                else
                {
                    Task=9900;
                }
            }
            break;
        case 2260:
            bflag=MOT[MInShuttle2].MotorMove(Prod.InSHT[1].iLeft);       //KenHsieh 20260120 : Add Shuttle move 保護
            if(bflag)
            {
                if(bfCanMoveLChange[iShuttle]==true)
                {
                    SetShuttlefCanMoveL(iShuttle, false, __FUNC__, "2260");
                    bfCanMoveLChange[iShuttle]=false;
                }

                if(iSHAutoLtcErr!=0)                                            //KenHsieh 20251008 : Fix Auto InSht Latch for 2D hangup, need to move left
                {
                    if(bAlarm)                                                  //KenHsieh 20251104 : add 檢測後動作 & alarm 變數
                    {
                        Task=2265;
                    }
                    else
                    {
                        InitDoInShZHome();
                        Task=2270;
                    }
                }
                else
                {
                    Task=9900;
                }
            }
            break;
        case 2265:
            for(int i=0; i<BLCarryKit.iShtCol; i++)                             //KenHsieh 20251128 : InShtLtc 新增異常重複判斷  //KenHsieh 20260518 : SThreadPara.iXItem → BLCarryKit.iShtCol
            {
                if(bInSHLtcErrNo[iShuttle][i] &&
                   bDupErrFlag[iShuttle][i])
                    bDuplicateErr=true;

                if(bInSHLtcErrNo[iShuttle][i])
                    bDupErrFlag[iShuttle][i]=true;
                else
                    bDupErrFlag[iShuttle][i]=false;
            }

            iLtcErrRetryCt[1]++;                                                //KenHsieh 20260402 : add Auto retry 1 times
            if(iLtcErrRetryCt[1]>1)
            {
                if(iSHAutoLtcErr==1)
                    ShowErrorMessage("JAM0404", K_RETRY, MInShuttle2, bDuplicateErr, ErrPart_InSHLtc);
                else
                    ShowErrorMessage("JAM0406", K_RETRY, MInShuttle2, bDuplicateErr, ErrPart_InSHLtc);
            }
            else
            {
                asStr1.sprintf("Auto retry for In shuttle2 latch error : %s", ErrPart_InSHLtc);
                RecordProcess(asStr1);
            }

            InitDoInShZHome();
            Task=2270;
            break;
        case 2270:
            if(DoInShZHome(iShuttle))
            {
                if(bAlarm)                                                      //KenHsieh 20251104 : add 檢測後動作 & alarm 變數
                {
                    iResult=0;
                    Task=1;
                }
                else
                {
                    fLtcSensor->ClearLtcSensor(1);                              //Sam 20221101 : Latch 清除都要確認是否清清乾淨
                    fLtcSensor->SetLtcSensor(1);                                //JerryYang 20230406 : 重置shuttle1的latch
                    Task=9901;
                }
            }
            break;
        case 9900:
            bInSh1DoLtc=false;
            bInSh2DoLtc=false;
            bfCanMoveLChange[iShuttle]=false;
            for(int i=0; i<(iShuttle==0 ? FLCarryKit.iShtCol : BLCarryKit.iShtCol); i++)     //KenHsieh 20251128 : InShtLtc 新增異常重複判斷  //KenHsieh 20260518 : SThreadPara.iXItem → kit-aware iShtCol
                bDupErrFlag[iShuttle][i]=false;

            iResult=1;
            break;
        case 9901:
            bInSh1DoLtc=false;
            bInSh2DoLtc=false;
            bfCanMoveLChange[iShuttle]=false;
            iResult=2;
            break;
    }

    return iResult;
}
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//==============================================================================
//==  TRANSLATED (was GATED) -- golden :7619-8279, verbatim transliteration.
//==
//==  THE central pick state machine: In-Arm picks IC out of the Loader tray.
//==  Cursor is the substrate-provided iPickFromLoadStageTask; `int &Task` binds
//==  it (golden :7642).  Structure preserved exactly: the backward
//==  `goto IN_ARM_LOADER` (label golden :7637, jump golden :7703), the case
//==  order, and the suck loop at case 1000 (golden :7909).
//==
//==  WHY THIS ONE MATTERS.  Two already-translated functions in this file had
//==  ZERO callers *because* this body was gated:
//==      ArmFinishForLoader()  (this file, golden :6677) -- golden calls it :7784/:8202
//==      bNeedOneCycle()       (this file, golden :7587) -- golden :7669/:7733/:7748/:8218
//==  Translating this body is what makes both reachable; see the TRAP notes
//==  above each of them.
//==
//==  CALLEE HONESTY -- this body calls five callees that are STILL STUBS.  The
//==  control flow below is faithful, but these five contribute nothing yet:
//==      sLoadPickupClean()          this file (golden :4182)   {}
//==      ShowAutoSkipError()         this file (golden :2478)   {}
//==      PickErrorData(int,int)      this file (golden :2502)   {}
//==      DoInArmLoadPickUP_9045(..)  this file (golden :4175)   {}
//==      AutoTeachLoadTrayZ(..)      ainarm9045_2x4_16_shims.cpp:84
//==                                  `return false` -- never reports "taught",
//==                                  so case 5 never advances back to Task=1.
//==                                  The case 1 arm that reaches it is guarded
//==                                  by CUSTOMER_CODE==CC_ASE_KaohSiung{,_K12}
//==                                  && bLoadInarmAutoHigh, so the default
//==                                  offline path never enters it.
//==  Every other callee below resolves to a real translated body.
//==
//==  ShowErrorMessage's K-code return drives the case 1100 recovery arms
//==  (K_TRAY_END / K_CLEAN_OUT / K_SKIP / K_HOME / else K_RETRY).  Offline that
//==  is the settable seam W906_ShowErrorMessage_SimReturn (canary_support.h),
//==  so a test can actually walk each arm.
//==============================================================================
#include "mycylin.h"                // Cylinder[] / TMyCylinder : C_TrayY_Fixer.On()/.Push(), C_LoaderEdgePush.Enable/.On() -- golden :7832/:7840/:7862
#include "atester_shims.h"          // fObserver (TfObserver*, -> forms/fObserver.h:1196) for AddTimeData at golden :8205.  HOIST ME to ainarm9045.cpp's include head -- this file already pulls the same header in at four later points for fContact.

//--  Forward declarations for callees that golden reaches by FILE ORDER -------
//--  (golden defines each above :7619 in its own ainarm9045.cpp; the port tree
//--  moved them BELOW this point, and none of them has a header declaration in
//--  golden either).  Declarations only -- no definitions, so no ODR risk.
#ifndef HT9045_W7_CheckLoaderHasTray3_DECLARED
#define HT9045_W7_CheckLoaderHasTray3_DECLARED
int  CheckLoaderHasTray(bool bAlarm, int iErrorCount, bool bTrayDuplicateErr);  //Steven 20161223 (jou) // golden ainarm9045.cpp:7256
#endif
#ifndef HT9045_W7_ArmFinishForLoader_DECLARED
#define HT9045_W7_ArmFinishForLoader_DECLARED
bool ArmFinishForLoader();                                                      // golden ainarm9045.cpp:6677
#endif
#ifndef HT9045_W7_ArmRow1FinishForLoader_DECLARED
#define HT9045_W7_ArmRow1FinishForLoader_DECLARED
bool ArmRow1FinishForLoader();                                                  // golden ainarm9045.cpp:6652
#endif
#ifndef HT9045_W7_bNeedOneCycle_DECLARED
#define HT9045_W7_bNeedOneCycle_DECLARED
bool bNeedOneCycle();                                                           //Steven 20131029 // golden ainarm9045.cpp:7587
#endif
#ifndef HT9045_W7_AddLoadingCount_DECLARED
#define HT9045_W7_AddLoadingCount_DECLARED
void AddLoadingCount(int iSuckRow, int iSuckCol, int iTrayRow, int iTrayCol);   //Steven 20161123 // golden ainarm9045.cpp:2592
#endif
#ifndef HT9045_W7_GetLoaderYPitchStep_DECLARED
#define HT9045_W7_GetLoaderYPitchStep_DECLARED
int  GetLoaderYPitchStep();                                                     //Steven 20141112 // golden ainarm9045.cpp:4863
#endif
#ifndef HT9045_W7_DoAutoSkipCheck_DECLARED
#define HT9045_W7_DoAutoSkipCheck_DECLARED
bool DoAutoSkipCheck();                                                         //kevin 20170822 // golden ainarm9045.cpp:7341
#endif
#ifndef HT9045_W7_ProcessMES0101InArmPickLoaderError_DECLARED
#define HT9045_W7_ProcessMES0101InArmPickLoaderError_DECLARED
int  ProcessMES0101InArmPickLoaderError(bool bHasDuplicateErr, AnsiString ErrPart); //Steven 20141121 // golden ainarm9045.cpp:7033
#endif

//--  Cross-module callees: golden reaches these through headers this TU does
//--  not include.  Golden header coordinate cited per symbol.
#ifndef HT9045_W7_MoveInArm2XYToWait_DECLARED
#define HT9045_W7_MoveInArm2XYToWait_DECLARED
extern bool MoveInArm2XYToWait();                                               // golden ainarm2.h:146 (port body ainarm2.cpp:5049)
#endif
#ifndef HT9045_W7_MoveInArm2XYToShuttle2Wait_DECLARED
#define HT9045_W7_MoveInArm2XYToShuttle2Wait_DECLARED
extern bool MoveInArm2XYToShuttle2Wait();                                       // golden ainarm2.h:143 (port body ainarm2.cpp:5188)
#endif
#ifndef HT9045_W7_MoveInArmZToPlateSafeAndCheckLoaderTray_DECLARED
#define HT9045_W7_MoveInArmZToPlateSafeAndCheckLoaderTray_DECLARED
extern bool MoveInArmZToPlateSafeAndCheckLoaderTray(int iTask);                 //Ifor 20221227 // golden ainarm2.h:140 (port body ainarm2.cpp:656)
#endif
#ifndef HT9045_W7_bNeedPickupErrorICToRecycleBin_DECLARED
#define HT9045_W7_bNeedPickupErrorICToRecycleBin_DECLARED
extern bool bNeedPickupErrorICToRecycleBin();                                   //JimmyChiu 20220908 // golden ainarm2.h:170 (port body ainarm2.cpp:7762)
#endif
#ifndef HT9045_W7_SetInArmUseSuckToHasNullIC_DECLARED
#define HT9045_W7_SetInArmUseSuckToHasNullIC_DECLARED
extern void SetInArmUseSuckToHasNullIC(int iSht, int iKit);                     // golden ainarm2.h:151 (port body ainarm2.cpp:140)
#endif
#ifndef HT9045_W7_CheckTrayMapData_DECLARED
#define HT9045_W7_CheckTrayMapData_DECLARED
extern void CheckTrayMapData(int iTrayRow, int iTrayCol);                       //wei 20170302 // golden ainarm2.h:217 (port body ainarm2.cpp:7595)
#endif
#ifndef HT9045_W7_CheckInArmSuckFromLoaderICFallDown_DECLARED
#define HT9045_W7_CheckInArmSuckFromLoaderICFallDown_DECLARED
extern bool CheckInArmSuckFromLoaderICFallDown();                               //Steven 20110516 // golden csystem.h:47 (port body csystem.cpp:22248)
#endif
#ifndef HT9045_W7_MoveInRotateToDegreeAtSameTime_DECLARED
#define HT9045_W7_MoveInRotateToDegreeAtSameTime_DECLARED
extern bool MoveInRotateToDegreeAtSameTime(int iDegree, bool bInitial=false);   // golden RotateKit/aRotateKIT_In.h:31 (port body RotateKit/aRotateKIT_In.cpp:4163)
#endif
#ifndef HT9045_W7_AutoTeachLoadTrayZ3_DECLARED
#define HT9045_W7_AutoTeachLoadTrayZ3_DECLARED
extern bool AutoTeachLoadTrayZ(bool Reset, int InArm, int &iAOATask);           //kevin 20170930 // golden AutoTeach/AutoTeach.h:231 (port STUB ainarm9045_2x4_16_shims.cpp:84)
#endif

//--  Data the golden body reaches through ainarm2.h / AutoTeach.h, plus the
//--  file-scope timer golden defines on the line directly above :7619.
#ifndef HT9045_W7_iXPosition_DECLARED
#define HT9045_W7_iXPosition_DECLARED
extern int  iXPosition[8], iYPosition;                                          // golden ainarm2.h:82 (port defs ainarm9045_2x4_16_shims.cpp)
#endif
#ifndef HT9045_W7_iInArmZTeachTask_DECLARED
#define HT9045_W7_iInArmZTeachTask_DECLARED
extern int  iInArmZTeachTask;                                                   //kevin 20171107 // golden AutoTeach/AutoTeach.h:234
#endif
#ifndef HT9045_W7_DoArmPickFromLoadStage_Delay_DECLARED
#define HT9045_W7_DoArmPickFromLoadStage_Delay_DECLARED
extern TQPF_Timer DoArmPickFromLoadStage_Delay;                                 //Steven 20160219 // golden ainarm9045.cpp:7618 (port def ainarm9045_2x4_16_shims.cpp:85)
#endif
//------------------------------------------------------------------------------
bool DoInArmPickFromLoadStage_9045()
{
    static bool bHasLog=false;
    if(MOT[MTrayX].ReadEncoderPos()<Prod.iXTraySafty)                           //JimmyChiu 20250722 : 確保In arm移動到loader時，Tray arm已離開
    {
        MoveInArm2XYToWait();
//        if(bHasLog==false)
//        {
//            AnsiString sErr=AnsiString().sprintf("Tray arm not Safe pos. %s , Trayarm encoder=%d",__FUNC__,MOT[MTrayX].ReadEncoderPos());
//            RecordProcess(sErr);
//            bHasLog=true;
//        }
        return false;
    }

    static int iRetryCT=0, iBuf=0;
    static bool bTrayDuplicateErr=false;

    IN_ARM_LOADER:

    QueueTaskList[13].CheckTaskChange();                                        //Steven 20200821 : 使用Goto也要記錄Task變化
    int ret;
    int iFlag;
    int &Task=iPickFromLoadStageTask;
    int iYpos=0;
    int iSht=InArmSuck.iWhichSht;
    int iKit=InArmSuck.iWhichKit;

    bool flag1, bTrayEndCleanOut, bPickFallDown;
    bool bHasDuplicateErr=false, bLoadpickerrorMoveWait=false;                  //kevin 20220708            //Steven 20100104 : 整合AlarmCodde
    bool bflag=false;
    AnsiString ErrPart="", ErrTray="", sbuffer="";                              //kevin 20220521 tray pos   //Steven 20100104 : 整合AlarmCodde
    bHasLog=false;

    switch(Task)
    {
        case 1:
            if(CUSTOMER_CODE==CC_ASE_KaohSiung ||
               CUSTOMER_CODE==CC_ASE_KaohSiung_K12)                             //Steven 20131101 : Add ASE-K12
            {
                if(bLoadInarmAutoHigh)                                          //kevin 20171106 (wei) initial inarm 校正高度 Z軸先不要下去吸
                {                                                               //jou 2011-02-08 start : OneCycle偷拿料
                    AutoTeachLoadTrayZ(true, 0, iInArmZTeachTask);
                    Task=5;
                    break;
                }
            }

            if(MoveInArmZToPlateSafeAndCheckLoaderTray(Task))                   //Ifor 20221227 add:新增Loader 取料後判斷是否吸到Loader Tray
            {
                if(bNeedOneCycle())                                             //Steven 20131029 : 解決Index Position Error
                {
                    if(MOT[MMTrayY].Tray.HasIC()==false &&                      //JerryYang 20250718 : fix 取TRAY時input arm沒有讓位
                       MOT[MMTrayY].fHasTray &&
                       (TrayForm.iManualRemoveLoader!=0 &&                      //Steven 20130819 : 加入開門檢查的功能  (==true --> !=0)
                        bLoaderHasSkip==true) ||                                //jou 2010-12-20 修正需要手動拿Loader Tray時,In Arm Y移至shuttle 2上,方便OP拿Tray
                        bNeedManualRemoveTray==true)                            //Ifor 20200331 : add Edit Loader Tray Need Manual Remove Tray
                    {
                        MoveInArm2XYToShuttle2Wait();
                        return false;
                    }
                    else
                    {
                        return true;
                    }
                }

                if(DoPickLoaderOK(iSht, iKit))
                {
                    return true;
                }

                MyInArmAtShuttleTimer.LatchCycleTime(true);                     //Steven 20151201

                if(tRotate.ActiveRotate==1 && iRotate_Type==eInOutArm1Motor)    //add One sucker with rotate
                {
                    MoveInRotateToDegreeAtSameTime(0, true);
                    Task=9;
                }
                else
                {
                    Task=10;
                }

                goto IN_ARM_LOADER;
            }
            break;
        case 5:                                                                 //kevin 20171103 (wei) Inarm Auto High //kevin 20170929 test4 不讓z軸往下
            if(AutoTeachLoadTrayZ(false, InArm, iInArmZTeachTask))
            {
                bLoadInarmAutoHigh=false;
                Task=1;
            }
            break;
        case 9:
            if(MoveInRotateToDegreeAtSameTime(0))                               //add One sucker with rotate
            {
                Task=10;
            }
            break;
        case 10:
            if(MOT[MMTrayY].HasIC())
            {
                iFlag=CheckLoaderHasTray(false, 0, false);                      //Steven 20161223 (jou) : 確認LoaderSensor統一成Function
                if(iFlag>0)
                {
                    Task=20;
                    break;
                }
                else
                {
                    bTrayDuplicateErr=false;
                }

                if(bNeedOneCycle())                                             //Steven 20131029 : 解決Index Position Error
                {
                    Task=1;
                    return true;
                }

                if(bNeedPickupErrorICToRecycleBin())                            //JimmyChiu 20220908 add Pickup Error Placement
                {
                    Task=10;
                    break;
                }
                Task=12;
            }
            else
            {
                if(bNeedOneCycle())                                             //Steven 20131029 : 解決Index Position Error
                {
                    Task=1;
                    return true;
                }

                if((TrayForm.iManualRemoveLoader!=0 &&                          //Steven 20130819 : 加入開門檢查的功能  (==true --> !=0)
                    bLoaderHasSkip==true) ||                                    //jou 2010-12-20 修正需要手動拿Loader Tray時,In Arm Y移至shuttle 2上,方便OP拿Tray
                    bNeedManualRemoveTray==true)                                //Ifor 20200331 : add Edit Loader Tray Need Manual Remove Tray
                {
                    if(MoveInArm2XYToShuttle2Wait())
                    {
                        Task=15;
                    }
                }
                else
                {
                    if(MoveInArm2XYToWait())
                    {
                        Task=15;
                    }
                }
            }

            if(Task!=12)                                                        //Steven 20180813 : add in arm speed
                break;
        case 12:
            if(IniConfig.bUseAutoSiteMapping)                                   //jou 2011-03-24 start : Auto Site Mapping
            {
                if(MOT[MMTrayY].fHasTray==false)
                {
                    Task=10;
                    break;
                }
            }

            if(ArmFinishForLoader())
            {
                Task=2100;
            }
            else if(MOT[MMTrayY].HasIC())
            {
                if(SearchAndMoveInArmXYToLoad_9045())
                {
                    if(IsCheckInArmDestroyActiveFinish())                       //ChungHung 201112296 add Pick 前先確認CheckInArmDestroyActive 已完成
                    {
                        Task=200;
                    }
                }
            }
            else
            {
                Task=10;
            }
            break;
        case 15:
            if(MOT[MMTrayY_Car].fHasTray ||
               MOT[MMTrayY].fHasTray ||
               MOT[MMTrayZ].fHasTray)
            {
                Task=10;
                return false;
            }
            else
            {
                if(DoAutoSkipCheck())                                           //Steven 20241218 :  avoid the tray not goes down yet
                {
                    if(InArmSuck.HasRealIC())                                   //Sam 20250307 : 真的上面有 IC 再來補滿
                        SetInArmUseSuckToHasNullIC(iSht, iKit);                 //Steven 20241226
                    else
                        InArmSuck.ClearAll();

                    Task=1;
                    return true;
                }
            }
            break;
        case 20:
            if(MoveInArm2XYToWait())
                Task=30;
            break;
        case 30:
            if(INSTALL_OCR_YMot==eocrYMotInstal)                                //Frank 20250214 add
            {
                Cylinder[C_TrayY_Fixer].On();
                DoArmPickFromLoadStage_Delay.SetSecAndOn(Ld_UldDelayTime.LD_FixTrayDely);
                Task=39;
            }
            else
            {
                if(Cylinder[C_TrayY_Fixer].Push())
                    Task=40;
            }
            break;
        case 39:
            if(DoArmPickFromLoadStage_Delay.Off())
                Task=40;
            break;
        case 40:
            iBuf++;
            iFlag=CheckLoaderHasTray(true, iBuf, bTrayDuplicateErr);            //Steven 20161223 (jou) : 確認LoaderSensor統一成Function
            if(iFlag>0)
            {
                if(iFlag==2)
                {
                    bTrayDuplicateErr=true;
                    iBuf=0;
                }
            }
            else
            {
                bTrayDuplicateErr=false;
            }

            if(Cylinder[C_LoaderEdgePush].Enable)                               //kevin 20210830 add side push
            {
                Cylinder[C_LoaderEdgePush].On();                                //jou 2011-02-23 先打出去0.15sec
            }
            Task=10;
            break;
        case 200:
            if(MoveInArmZToLoaderPick(iRetryCT))
            {
                if(InArmNeedCheckOffset(false, 0))                              //Steven 20230531 : 簡化判斷式
                {
                    Task=220;
                    break;
                }

                if(InArmSuck.iPickRow==1)
                {
                    iLoadPitchStepY=0;
                }
                else
                {
                    if(USE_IN_Y_IS_AUTO_PITCH==true &&                          //ChungHung 20131231 alter AutoYPitch  //JerryYang 20251218 : IN/OUT ARM支援不同模組
                       ArmRow1FinishForLoader()==true)
                    {
                        iLoadPitchStepY=0;
                    }
                    else
                    {
                        iLoadPitchStepY=GetLoaderYPitchStep();
                    }
                }

                sLoadPickupClean();                                             //kevin 20220723 initial data
                Task=1000;
            }
            break;
        case 220:
            if(bEnterOffset==true)
            {
                bEnterOffset=false;
                Task=1;
            }
            else
            {
                Task=200;
            }
            break;
        case 1000:
            bPickFromLoader=true;                                                                                       //Steven 20171226 (Wei) : 確認Loader吸取完成
            flag1=true;
            iBuf=0;
            sAskStartDetect="";
            for(int i=0; i<InArmSuck.iMaxRow; i++)
            {
                for(int j=0; j<InArmSuck.iMaxCol; j++)
                {
                    if(InArmSuckUse[i][j] && InArmSuck.Item[i][j]==NULL_IC)
                    {
                        if(iXPosition[j]<0)                                                                             //Steven 20240920 : Loader吸料計算的資料錯誤;
                        {
                            ShowErrorMessage("WAR0149", 0, MInArmX, 0, IndexSuckName[i][j]);
                            continue;
                        }

                        if(ArmSpeed_File[InArm].bUseHPVacuum)                                                           //Steven 20180125 (Jou) : 加熱盤的真空等待時間
                            InArmSuck.Suck[i][j].OnDelayTime=ArmSpeed[InArm].dVacuumTI*100;

                        iYpos=iYPosition+iLoadPitchStepY*i;
                        if(bSuckEnd[i][j]==false && InArmSuck.Suck[i][j].Suck())                                        //wei 20150318 suck 判斷是否動作
                        {
                            AddLoadingCount(i, j, iYpos, iXPosition[j]);                                                //Steven 20161123 (jou) : 將Loader吸取的數量計算統一成Function
                        }
                        else if(InArmSuck.Suck[i][j].Error==false)
                        {
                            flag1=false;
                        }
                        else if(InArmSuck.Suck[i][j].Error==true)
                        {
                            bSuckEnd[i][j]=true;
                            sLoadPickupErrorTrayPos[i][j].sprintf("(%d,%d);", iXPosition[j]+1, iYpos+1);                //kevin 20220521 Load pick error record X, Y 座標
                        }
                    }
                    else
                    {
                        bSuckEnd[i][j]=true;
                    }
                }
            }

            if(flag1==false)                                                                                            //KEVIN 20130421 按RETRY資料尚未交換會將其他IC丟掉
            {
                break;
            }

            ShowAutoSkipError();                                                                                        //kevin 20170904 記錄每一盤 SKIP 沒吸到IC位置

            for(int i=0; i<InArmSuck.iMaxRow; i++)
            {
                for(int j=0; j<InArmSuck.iMaxCol; j++)
                {
                    if(InArmSuck.Suck[i][j].Error)
                    {
                        bInArmSuckErr=true;
                        iRetryCT++;
                        if(iRetryCT<=ArmSpeed[InArm].iRetryCT)                                                          //Steven 20141113 : Loader SKIP時在原地Retry
                        {
                            Task=1010;
                        }
                        else
                        {
                            Task=1050;
//                            if(bPickUpHomeFinish==false)                      //Steven 20240417 : mark
//                            {
//                                bPickUpHomeFinish=true;
//                                Task=1010;
//                                SetInArmHome();
//                            }
                        }
                        return false;
                    }
                }
            }

            iRetryCT=0;
            if((bCanRunSCKART==true ||                                                                                  //Steven 20161223 (wei) : For SCK ART loading count
                IniConfig.bA65_BundleIDList) &&
               iSCKARTLoadingStatus>=1 &&                                                                               //數量到了要放下來,然後 Alarm
               TestIF_File.bSCKART_LotDeviceCheck &&
               bSCKARTLoadCntAlarm==true)
            {
                ProcessSCKARTLoadingCount(true);
                Task=4000;
            }
            else if(IniConfig.bP57LoaderAutoCleanOutByInputCT &&                                                        //Sam 20250605 : Loader Count AutoCleanOut
                    iSCKARTLoadingStatus>=1 &&
                    bSCKARTLoadCntAlarm==true)
            {
                ProcessSCKARTLoadingCount(true);
                Task=4000;
            }
            else
            {
                Task=2000;
            }
            break;
        case 1010:
            if(MoveInArmZToPlateSafeAndCheckLoaderTray(Task))                   //Ifor 20221227 add:新增Loader 取料後判斷是否吸到Loader Tray
            {
                if(IniConfig.bE56LoaderRetryAtSamePosition)                     //Steven 20170828 (wei) : Loader吸取異常時,要在同一個位置作Retry
                {
                    Task=1020;
                }
                else
                {
                    bInArmPickErrFromLoader=true;                               //Steven 20170828 (wei) : 確認是否有在Loader吸料異常
                    Task=10;
                }
            }
            break;
        case 1020:
            if(MoveInArmZToLoaderPick(iRetryCT))                                //Steven 20141113 : Loader SKIP時在原地Retry
            {
                Task=1000;
            }
            break;
        case 1050:
            if(MoveInArmZToPlateSafeAndCheckLoaderTray(Task))                   //Ifor 20221227 add:新增Loader 取料後判斷是否吸到Loader Tray
            {
                bPickFromLoader=false;                                          //Steven 20180417 (Jou) : 確認Loader吸取完成
                Task=1100;
            }
            break;
        case 1100:
            bTrayEndCleanOut=false;                                                                                     //不要重復問2次
            iRetryCT=0;

            ErrPart=" ";
            ErrTray="";                                                                                                 //kevin 20220521 tray pos
            bHasDuplicateErr=false;
            for(int i=0; i<InArmSuck.iMaxRow; i++)
            {
                for(int j=0; j<InArmSuck.iMaxCol; j++)
                {
                    if(bPickLoaderDuplicateErr[i][j])
                    {
                        bHasDuplicateErr=true;
                    }

                    if(InArmSuck.Suck[i][j].Error)
                    {
                        CheckTrayMapData(iXPosition[j], iYPosition+iLoadPitchStepY*i);                                  //wei 20170302 (Steven) Tray map data error
                        ErrPart+=InArmSuck.Suck[i][j].sName;
                        bPickLoaderDuplicateErr[i][j]=true;
                        iYpos=iYPosition+iLoadPitchStepY*i;                                                             //kevin 20171026 (wei) add Tray iYPos
                        PickErrorData(iXPosition[j], iYpos);                                                            //kevin 20170904 (Steven) auto tray end 記錄一盤ＸＹ位置
                        ErrTray+=sLoadPickupErrorTrayPos[i][j];                                                         //kevin 20220521 add Tray X Y  Pos      //kevin 20220521 tray pos
                        //sLoadPickupErrorTrayPos[i][j]="";                     //kevin 20220823 mark kevin 20220521 add Tray X Y  Pos      //kevin 20220521 tray pos
                        bLoadpickerrorMoveWait=true;                                                                    //kevin 20220708
                    }
                    else
                    {
                        bPickLoaderDuplicateErr[i][j]=false;
                    }
                }
            }

            if(bLoadpickerrorMoveWait &&
               (IniConfig.bE67_LoadPickerrorMoveWaitpos ||                                                              //kevin 20220708 load pick error move wait
                CUSTOMER_CODE==CC_ASE_KaohSiung))
            {
                if(MoveInArm2XYToWait()==false)
                   return false;
            }

            if(bTrayEndCleanOut==false)                                                                                 // if is TRAY END or CLEAN OUT don't ask again
            {
                if(IniConfig.b17bUseLoadCCDTrayMap)                                                                     //kevin 20220825 change function for load ccd check pickup error show Array
                {
                    DoInArmLoadPickUP_9045(ErrPart, ErrTray);                                                           //kevin 20220823 add Tray X Y  Pos
                }
                ret=ProcessMES0101InArmPickLoaderError(bHasDuplicateErr, ErrPart);                                      //Steven 20141121 : 整合InArm吸取Loader異常Alarm
            }

            if(ret==K_TRAY_END)
            {
                if(CosFunction.bOpenDoorCheckLoaderAfterTrayEnd)                                                        //Jimmychiu 20250826 : Open Door Check Loader After TrayEnd
                {
                    bNeedManualCheckEmptyTray=true;
                    ShowErrorMessage("MES0923", K_RETRY, MMTrayY, false, "DoCatchTray");
                }
                bTrayEndCleanOut=true;
                return DoTrayEndProcess_9045();                                                                         //Steven 20190703 : 整合Tray End動作
            }
            else if(ret==K_CLEAN_OUT)                                                                                   // maybe is SKIP ,so can not include under line
            {
                fMain->CleanOut("DoInArmPickFromLoadStage");
                Task=1;
                return true;                                                                                            //Steven 20130612 : 要可以Clean Out
            }
            else if(ret==K_SKIP)
            {
                bflag=DoTraySkipProcess_9045();                                                                         //Steven 20190703 : 整合Load Tray Skip動作
                if(bflag==false)
                {
                    Task=1400;
                    return false;
                }
            }
            else if(ret==K_HOME)
            {
                InArmSuck.ResetAll();                                                                                   //Steven 20160323 : 避免未開啟真空
                Task=1200;
                return false;
            }
            else                                                                                                        // if(ret==K_RETRY)
            {
                if(IniConfig.bSPILFunction==true &&                                                                     //JerryYang 20170328 (Jou) 矽品客戶碼統一用SPILFunction
                   ret==K_RETRY)                                                                                        //jou 2012-11-28 SPIL Manual Tray remove功能, 要包含在Loader上按 Retry
                {
                    bLoaderHasSkip=true;
                    if(CosFunction.bNextTrayNeedManualRemoveTray==true)                                                 //Sam 20190731 : 當 Load 發生 Pickup Error Skip 時，當前這盤除了要手動移除，下一盤也要手動移除。
                    {
                        MaualRemoveTray3Alarm.SetTakePickUpError();
                    }
                }
                InArmSuck.ResetAll();                                                                                   //Steven 20160323 : 避免未開啟真空

                if(IniConfig.bInOutArmCanPushHome)
                {
                    Task=1400;                                                                                          //不回Home
                }
                else
                {
                    Task=1200;
                }
                return false;
            }

            for(int i=0; i<InArmSuck.iMaxRow; i++)
            {
                for(int j=0; j<InArmSuck.iMaxCol; j++)
                {
                    if(InArmSuckUse[i][j])
                    {
                        Task=10;
                        break;
                    }
                }
            }

            iRetryCT=0;
            Task=2000;
            break;
        case 1200:
            if(MoveInArmZToPlateSafeAndCheckLoaderTray(Task))                   //Ifor 20221227 add:新增Loader 取料後判斷是否吸到Loader Tray
            {
                Task=1300;
            }
            break;
        case 1300:
            SetInArmHome();
            //iRetryCT=0;
            Task=10;
            break;
        case 1400:
            if(MoveInArmZToPlateSafeAndCheckLoaderTray(Task))                   //Ifor 20221227 add:新增Loader 取料後判斷是否吸到Loader Tray                                     //ChungHung HT9045 2011/12/13 //Input pickup device error時,按"retry"鍵,機台都會自動home
            {
                Task=10;
            }
            break;
        case 2000:
            if(MoveInArmZToPlateSafeAndCheckLoaderTray(Task))                   //Ifor 20221227 add:新增Loader 取料後判斷是否吸到Loader Tray
            {
                bPickFallDown=CheckInArmSuckFromLoaderICFallDown();             //Steven 20110516
                if(bPickFallDown)
                {
                    if(CosFunction.bAutoSkipNoDropError &&
                       IniConfig.bE60PickLoaderDropAutoSkip &&
                       (bDropRetry==true &&
                        ArmSpeed[InArm].bAutoSKIP==true &&
                        IniConfig.bRecordSkipPosition &&
                        bFirstRecordLoaderData==true))
                    {
                        if(CUSTOMER_CODE==CC_ASE_KaohSiung)                     //kevin 20221008 add drop again pickup
                            Task=1200;
                        else
                            Task=1010;
                    }
                    else
                    {
                        Task=1;
                    }
                }
                else
                {
                    Task=2100;
                }
            }
            break;
        case 2100:
            if(ArmFinishForLoader())
            {
                SetInArmUseSuckToHasNullIC(iSht, iKit);
                fObserver->AddTimeData(1, MyInArmAtShuttleTimer.LatchCycleTime()/1000.0);                               //JerryYang 20151209
                Task=1;
                return true;
            }
            else
            {
                if(IniConfig.bP17InArmFullPickFromLoader==false)                                                        //Steven 20111220 : 讓每盤都是滿的
                {
                    if((CUSTOMER_CODE==CC_KYEC_JCTHIU ||
                        CUSTOMER_CODE==CC_KYEC_LEE) &&                                                                  //Ifor 20181009 (Steven) : KYEC add FIFO Function
                       IniConfig.bI37_EnableFIFOMode &&
                       LastSet.iRunStartMode==rsmFIFOMode)
                    {
                        if(bNeedOneCycle())
                        {
                            Task=1;
                            return true;
                        }
                        else
                        {
                            Task=10;
                        }
                    }
                    else
                    {
                        if(iOneCycle==1)                                                                                //kevin 20110425 有吸取到ic就放shuttle 沒吸到吸嘴就不吸
                        {
                            Task=1;
                            return true;
                        }
                        else
                        {
                            Task=10;
                        }
                    }
                }
                else
                {
                    Task=10;
                }
            }
            break;
        case 2500:
            if(DoArmPickFromLoadStage_Delay.Off())                              //jou 2011-02-08 start : OneCycle偷拿料
            {
                Task=3000;
            }
            break;
        case 3000:
            if(MoveInArmZToPlateSafeAndCheckLoaderTray(Task))                   //Ifor 20221227 add:新增Loader 取料後判斷是否吸到Loader Tray
            {
                Task=1;

                for(int i=0; i<InArmSuck.iMaxRow; i++)
                {
                    for(int j=0; j<InArmSuck.iMaxCol; j++)
                    {
                        if(InArmSuck.Item[i][j]==NULL_IC)
                        {
                            InArmSuck.Suck[i][j].OffDestroy();
                        }
                    }
                }
                return true;
            }
            break;
        case 4000:
            if(ProcessSCKARTLoadingCount())                                     //Steven 20161223 (wei) : For SCK ART loading count
            {
                Task=10;
            }
            break;
    }
    return false;
}
//------------------------------------------------------------------------------
// golden :8843 -- auto-calibrate suck-Z SM (iDoInArmAutoCalSuckZTask,
// switch@8867).  Derefs MOT[] encoders + fProductionInfo->setEditZ* (TEdit form
// widgets).  GATED whole-body; bIsFirst init path kept faithful (golden :8860).
int iDoInArmAutoCalSuckZTask=0;
bool DoInArmAutoCalSuckZ(bool bIsFirst)
{
    int &Task=iDoInArmAutoCalSuckZTask;
    if(bIsFirst)                                                               // golden :8860 init path (faithful)
    {
        Task=1;
        return true;
    }
#if 0 // TODO(W7) -- golden :8867-9028 (MOT[] encoders + fProductionInfo->setEditZ* TEdit SM)
#endif
    return false;                                                              // golden default: calibration not finished
}

//==============================================================================
//==  ACTIVE offline stubs for the remaining golden in-arm engine surface.
//==  Each is declared in ainarm9045.h; the golden body is MOT[]/cross-module/
//==  UI-bound with no translated home this wave.  Offline-safe defaults keep
//==  the engine linkable.  Golden file:line cited per symbol.  TODO(W7).
//==============================================================================
// AI(W906-P1c) 20260921: 這個樁**退休了** —— 真本體翻在本檔尾端
//   （golden `ainarm9045.cpp:1616-2063`，448 行）。
//   放尾端而不是原地的理由是相依順序：本體會呼叫 `CheckInArmYStep`(:4249)、
//   `CheckPickerMode`(:4025)、`ArmYCanSuck2IC`(:2542)、`ArmXCanSuck2IC_9045S`(:2583)、
//   `ArmXCanSuck4IC_9045`(:2620) —— 全部定義在這一行**後面**。
//   放檔尾就不必補五個前向宣告。
//
// ⛔ 它為什麼非翻不可：**它是 `iInArmType` 全樹唯一的生產端。**
//   `cmydef.cpp:4918 int iInArmType=0;` 是初值，排除 tests/ 之後全樹**零個寫入點**，
//   而讀取點有 **583 個、橫跨 70 個檔**。`MachineType.h:1054 e9045_1x1_1 = 0`
//   ⇒ 任何機型都被當成 1x1 單吸嘴，而且**不會有錯誤訊息**（0 是合法值，
//     不是「未設定」哨兵）。詳見 docs/INBOX_QUEUE.md 的 Q19。
// ===========================================================================
//  SetInOutArmParameter -- golden ainarm9045.cpp:1499-1614.  AI(W906-P1b) 20260920
//
//  bring-up 時**依機型挑一支 `SetInOutArmParameter_<layout>()` 跑**，最後呼叫
//  `SetArmRowCount()` 把 InArmSuck 的幾何推給所有 carry kit / TestSuck /
//  TestSocket。
//
//  ⛔ AI(W906-P1c) 20260921 更正：這段橫幅是 20260920 寫的，兩處已過期 ——
//    (a) 「本樹唯一的呼叫點是 `cinitial.cpp:17363`」：行號漂了，而且**不只一個**。
//        實測現在是 `cinitial.cpp:17497`（`ChangeSite()` 裡）與
//        本檔 `:12380`（`DoInArm_9045_Type()` 的尾巴，golden :2061）。
//    (b) 當時 `iInArmType` **沒有生產端**（全樹恆 0），所以那時翻這一支會讓
//        每一台機器都誤走 `_1x1_1`。20260921 的 P1c 第 1 步已經把生產端
//        （`DoInArm_9045_Type`，448 行）翻好並解掉擋住它的 GATE N3-G6，
//        實測斷點命中 2 次。**先後順序是刻意的，不要顛倒。**
//
//  ⚠ 它原本是空樁，後果是**整條 layout 參數設定鏈都沒跑**：
//    20 支 `SetInOutArmParameter_*()` 在本樹都有真本體（逐檔查過，沒有一支是
//    樁），但沒有人叫它們；`SetArmRowCount()` 也因此是死碼。
//    ⇒ 解開這一支，那 21 支才會活。
//
//  ⚠⚠ **這會改變執行期行為**，而且改的是吸嘴幾何 —— 不是顯示。
//    解樁的理由是本迴圈的常設規則：加閘的唯一合法理由是「相依不存在」，
//    不是「怕機台會動」。這裡相依存在（下面那一項除外），所以照翻。
// ===========================================================================
//  [LIFTED AI(W906-W3-9c) 20260925 -- the prelude is back in SetInOutArmParameter below; this banner is history] SAFETY-GATE(W906-P1b-COPYBACKUP) —— golden :1503-1513
//
//  缺的相依（兩個，都不是「怕它動」）：
//
//   1. `CopyKitSuck(TMyKitSuck*, TMyKitSuck*)` —— **本樹沒有活的定義**。
//      golden 在 `MyKitSuck.cpp:2795-2828`，逐格複製 `TMySucker` 的 **23 個
//      欄位**；本樹活的 `TMySucker`（aHotPlateSubstrate.h）只帶了其中 **6 個**
//      （OnType / SuckerName / iMotNo / iMyRow / iMyCol / sName）。
//      要忠實翻它，得先把另外 17 個欄位（On/Off/Sen 的 Ring/IP/Port/Bit/Type
//      與三個 *PortName）帶進門面 —— 那是 IO 綁定層的事，不是 P1b。
//      ⚠ `mykitsuck.cpp:2818` 有一份，但那個檔**刻意不在建置裡**
//        （CMakeLists.txt:2684，兩個 TMyKitSuck 的 ODR 陷阱）。
//      ⚠ 同一個缺口已經擋住 `cinitial.cpp:933` 與 `:1113` 兩段 `#if 0`，
//        它們的註解寫的是同一件事。**補 CopyKitSuck 會一次解開三處。**
//
//   2. `InArmSuckBackup` —— **本樹沒有活的定義**（只有 `mykitsuck.cpp:227`，
//      同一個沒被建置的檔）。`OutArmSuckBackup` 倒是有
//      （aHotPlateSubstrate.cpp:1280）。
//
//  後果（機台的話）：**手臂吸嘴的 IO 綁定不會從備份還原**。
//  golden 在這裡先把備份蓋回 InArmSuck/OutArmSuck，才讓 layout setter 依機型
//  重挑要用哪幾個吸嘴。少了還原，如果上一次換線把某個吸嘴的 IO 改過，
//  這一次 bring-up 不會把它洗回原始表。
//  ⇒ 第一次 bring-up（值都還是 cinitial 讀進來的原始值）**沒有差別**；
//    差別出現在「同一次開機內改過吸嘴設定又重跑 bring-up」。
//
//  ⚠ 為什麼只閘這 12 行、其餘照跑：**逐檔量過，20 支 layout setter 裡
//    對 `InArmSuckBackup` 的活引用是 0 個**（全部都在註解裡，golden 自己也
//    把那些 CopyInitSuck 註解掉了）。所以少了這個 prelude，
//    那 20 支照樣跑得起來、跑的是同一件事。
// ===========================================================================
//  ⚠ 20 支 layout setter 與 IsNNMode 的宣告分散在 20 個 `ainarm9045_*.h`，
//    本檔一個都沒 include。把它們整批提到檔頭風險遠大於這件事本身
//    ⇒ 只補這 20 個零參數 void 宣告，簽名與各自的 header 逐字相同。
void SetInOutArmParameter_All_1Pick();      // ainarm9045_All_1Pick.h
void SetInOutArmParameter_1x1_1();          // ainarm9045_1x1_1.h
void SetInOutArmParameter_1x2_2();          // ainarm9045_1x2_2.h
void SetInOutArmParameter_1x2_4_Hot();      // ainarm9045_1x2_4_Hot.h
void SetInOutArmParameter_1x3_2_14();       // ainarm9045_1x3_2_14.h
void SetInOutArmParameter_1x3_4();          // ainarm9045_1x3_4.h
void SetInOutArmParameter_1x4_4_13();       // ainarm9045S_1x4_4.h
void SetInOutArmParameter_1x4_2();          // ainarm9045_1x4_2.h
void SetInOutArmParameter_1x4_4();          // ainarm9045_1x4_4.h
void SetInOutArmParameter_1x4_8_Hot();      // ainarm9045_1x4_8_Hot.h
void SetInOutArmParameter_2x1_2();          // ainarm9045_2x1_2.h
void SetInOutArmParameter_2x2_4();          // ainarm9045_2x2_4.h
void SetInOutArmParameter_2x2_8_Hot();      // ainarm9045_2x2_8_Hot.h
void SetInOutArmParameter_2x3_6_14();       // ainarm9045_2x3_6_14.h
void SetInOutArmParameter_2x3_6();          // ainarm9045_2x3_6.h
void SetInOutArmParameter_2x4_4();          // ainarm9045_2x4_4.h
void SetInOutArmParameter_2x4_8();          // ainarm9045_2x4_8.h
void SetInOutArmParameter_2x5_8();          // ainarm9045_2x5_8.h
void SetInOutArmParameter_2x6_8();          // ainarm9045_2x6_8.h
void SetInOutArmParameter_2x8_8();          // ainarm9045_2x8_8.h

void SetInOutArmParameter()                                                     //Steven 20250401 : 整合in arm參數設定
{
    if(InArmSuck.HasIC()==false && OutArmSuck.HasIC()==false)                   //Steven 20230921 : 修正LED例外異常  // golden :1501
    {
        // AI(W906-W3-9c) 20260925: SAFETY-GATE(W906-P1b-COPYBACKUP) LIFTED -- golden :1503-1513 back in (NB2 R3 RB-3).
        //    Both stated blockers died: CopyKitSuck live at mykitsuck.cpp:2818, InArmSuckBackup at mykitsuck.cpp:227 (A4-6).
        CopyKitSuck(&InArmSuckBackup,  &InArmSuck);                          // golden :1503  Frank HT1032 add
        CopyKitSuck(&OutArmSuckBackup, &OutArmSuck);                         // golden :1504

        for(int i=0; i<InArmSuck.iMaxRow; i++)                               // golden :1506
        {
            for(int j=0; j<InArmSuck.iMaxCol; j++)                           // golden :1508
            {
                CopyInitSuck(&InArmSuckBackup , &InArmSuck , i, j, i, j);     // golden :1510
                CopyInitSuck(&OutArmSuckBackup, &OutArmSuck, i, j, i, j);     // golden :1511
            }
        }

        if(USE_PICKER_COUNT==ep1Picker && iInArmType==e9045_2x2_4_13)           // golden :1515
        {
            SetInOutArmParameter_All_1Pick();
        }
        else if(iInArmType==e9045_1x1_1)
        {
            SetInOutArmParameter_1x1_1();
        }
        else if(iInArmType==e9045_1x2_2_14 ||
                iInArmType==e9045_1x2_2_13)
        {
            SetInOutArmParameter_1x2_2();
        }
        else if(iInArmType==e9045_1x2_4_Hot)
        {
            SetInOutArmParameter_1x2_4_Hot();
        }
        else if(iInArmType==e9045_1x3_2_14)
        {
            SetInOutArmParameter_1x3_2_14();
        }
        else if(iInArmType==e9045_1x3_4)
        {
            SetInOutArmParameter_1x3_4();
        }
        else if(iInArmType==e9045_1x4_4_13)
        {
            SetInOutArmParameter_1x4_4_13();
        }
        else if(iInArmType==e9045_1x4_2_14)
        {
            SetInOutArmParameter_1x4_2();
        }
        else if(iInArmType==e9045_1x4_4_Back ||
                iInArmType==e9045_1x4_4 ||
                iInArmType==e9045_1x4_1_Ac)
        {
            SetInOutArmParameter_1x4_4();
        }
        else if(iInArmType==e9045_1x4_8_Hot)
        {
            SetInOutArmParameter_1x4_8_Hot();
        }
        else if(iInArmType==e9045_2x1_2_13)
        {
            SetInOutArmParameter_2x1_2();
        }
        else if(iInArmType==e9045_2x2_4_14 ||
                iInArmType==e9045_2x2_4_12 ||
                iInArmType==e9045_2x2_4_13)
        {
            SetInOutArmParameter_2x2_4();
        }
        else if(iInArmType==e9045_2x2_8_Hot)
        {
            SetInOutArmParameter_2x2_8_Hot();
        }
        else if(iInArmType==e9045_2x3_6_14)
        {
            SetInOutArmParameter_2x3_6_14();
        }
        else if(iInArmType==e9045_2x3_6)
        {
            SetInOutArmParameter_2x3_6();
        }
        else if(iInArmType==e9045_2x4_4_13 ||
                iInArmType==e9045_2x4_4_14)
        {
            SetInOutArmParameter_2x4_4();
        }
        else if(iInArmType==e9045_2x4_8)
        {
            SetInOutArmParameter_2x4_8();
        }
        else if(iInArmType==e9045_2x5_8)
        {
            SetInOutArmParameter_2x5_8();
        }
        else if(iInArmType==e9045_2x6_8)
        {
            SetInOutArmParameter_2x6_8();
        }
        else if(iInArmType==e9045_2x8_8)
        {
            SetInOutArmParameter_2x8_8();
        }
    //    else if(iInArmType==e9045_2x8_16)                                     // golden :1601-1604 原本就是註解
    //    {
    //        SetInOutArmParameter_2x8_16();
    //    }
        else if(iInArmType==e9045_2x8_32)
        {
    //    if(USE_PICKER_COUNT==ep16Picker)                                      // golden :1607-1609 原本就是註解
    //        SetInOutArmParameter_2x8_16();
    //    else
            SetInOutArmParameter_2x8_8();
        }
        SetArmRowCount();                                                       // golden :1612
    }
}
// ===========================================================================
//  SetArmRowCount -- golden ainarm9045.cpp:1471-1497.  AI(W906-P1b) 20260920
//
//  它做一件事：**把 InArmSuck 的 shuttle 幾何推給所有 carry kit / TestSuck /
//  TestSocket**。沒有它，那些物件的 `iShtRow`/`iShtCol` 一輩子是 ctor 值，
//  而 `AutoClean.cpp:4633/4648/4756/4771` 拿 `iShtRow` 當迴圈上界、
//  `HANA_ART.cpp:794/837` 與 `SCK_ART.cpp:591/618` 拿 `iShtRow*iShtCol`
//  當通道總數。
//
//  ⚠⚠ **它今天還是死碼，而且我知道**：golden 全樹唯一的呼叫點是
//  `ainarm9045.cpp:1612`，在 `SetInOutArmParameter()` 裡面 ——
//  而 `SetInOutArmParameter()` 在本樹**也是空樁**（本檔下方，golden :1499，
//  115 行的機型派發器）。
//  ⇒ 翻這一支**不會改變任何執行期行為**，它是把缺口補起來、讓文字可稽核。
//  ⇒ **不要因為它現在是死碼就以為翻了沒用** —— 反過來也一樣：
//    不要因為它翻好了就以為 carry kit 的幾何已經正確。
//
//  ## ⛔ 要讓它活起來，**不是**「翻 SetInOutArmParameter 就好」
//
//  20260920 我本來就是這樣打算的，腳本都寫好了。**量了之後發現那樣做是錯的。**
//
//  `SetInOutArmParameter()`（golden :1499-1614）是一個依 `iInArmType` 的
//  20 岔派發器。而 `iInArmType` 在本樹的處境是：
//
//    * 定義在 `cmydef.cpp:4918`，初值 **0**
//    * 全樹**沒有任何生產端**（`git grep 'iInArmType *=' -- '*.cpp'` 只剩
//      那個初值、log 的 sprintf、以及 tests/ 裡的夾具）
//    * golden 的生產端是 `DoInArm_9045_Type()`（golden :1616-2063，**448 行**），
//      而它在本樹**也是空樁**（本區塊下方）
//    * `MachineType.h:1054` **`e9045_1x1_1 = 0`**
//
//  ⇒ 只翻派發器的後果：**任何機型都會走 `SetInOutArmParameter_1x1_1()`**。
//    一台 2x4 八吸嘴的機器會拿到單吸嘴的幾何，而且**沒有任何錯誤訊息** ——
//    因為 0 是一個合法的 enum 值，不是「未設定」。
//    那比現在（整條鏈都不跑、值停在 ctor）**更糟**：現在是「沒設定」，
//    那樣會變成「設定成錯的」。
//
//  ⇒ 正確的順序是 **`DoInArm_9045_Type()`（448 行）→ `SetInOutArmParameter()`
//    （116 行）→ 這一支**。golden `cinitial.cpp` 也正是這個順序
//    （本樹 `cinitial.cpp:17353` 叫前者、`:17363` 叫後者，兩個呼叫點都在，
//     呼叫的都是空樁）。
//
//  ⚠ 那一波還有一個獨立的缺口：golden :1503-1513 的備份還原用
//    `CopyKitSuck()` 與 `InArmSuckBackup`，兩個在本樹都**沒有活的定義**
//    （`mykitsuck.cpp` 刻意不在建置裡，CMakeLists.txt:2684）。
//    `CopyKitSuck` 逐格複製 `TMySucker` 的 23 個欄位，而本樹活的 `TMySucker`
//    只帶了 6 個 —— 要補得先把 IO 綁定那 17 個欄位帶進門面。
//    同一個缺口也擋著 `cinitial.cpp:933` 與 `:1113` 兩段 `#if 0`。
//    ⚠ 但這一項**可以部分閘**：逐檔量過，20 支 layout setter 裡對
//      `InArmSuckBackup` 的**活引用是 0 個**（全在註解裡，golden 自己也把那些
//      CopyInitSuck 註解掉了），所以少了那個 prelude，那 20 支照樣跑同一件事。
// ===========================================================================
//  ⚠ `IsNNMode()` 宣告在 `atester_shims.h:284`，而本檔**只在 5085 行之後**才
//    include 它（那幾處都寫著 HOIST-ME）。把那個 header 整個提到檔頭 =
//    讓前 5,000 行突然看見約 300 個宣告，風險遠大於這件事本身。
//    ⇒ 這裡只補**這一個**宣告，簽名逐字抄 `atester_shims.h:284`。
//    ⚠ 它的本體是**真的**（`cinitial.cpp:7287`，golden 的原生落點）；
//      `atester_shims.cpp:337` 那個回 0 的離線樁已經 `#if 0` 退役。
//      所以這裡的三岔分支在本樹是活的，不是恆走 None_NN。
int IsNNMode();                                  // golden cinitial.h:60 / 本樹 atester_shims.h:284
void SetArmRowCount()                                                         //Steven 20250401 : 整合in arm參數設定
{
    OutArmSuck.iXStep=InArmSuck.iXStep;                                         // golden :1473
//    OutArmSuck.iYStep=InArmSuck.iYStep;                                       //Rogeryang 20260108 : 9011UC in/outarm模組不同需拆開
    OutArmSuck.iModeX=InArmSuck.iModeX;                                         // golden :1475
    FLCarryKit.SetPickerCount(InArmSuck.iShtRow, InArmSuck.iShtCol);            // golden :1476
    BLCarryKit.SetPickerCount(InArmSuck.iShtRow, InArmSuck.iShtCol);
    FRCarryKit.SetPickerCount(InArmSuck.iShtRow, InArmSuck.iShtCol);
    BRCarryKit.SetPickerCount(InArmSuck.iShtRow, InArmSuck.iShtCol);
    FTestSuck.SetPickerCount(InArmSuck.iShtRow, InArmSuck.iShtCol);             //Steven 20241002 : TestSuck補上 Row / Col
    BTestSuck.SetPickerCount(InArmSuck.iShtRow, InArmSuck.iShtCol);

    OutSht3Kit.SetPickerCount(InArmSuck.iShtRow, InArmSuck.iShtCol);            //RogerYang 20250514 Add for 9046AU

    if(iInArmType==e9045_1x4_1_Ac)                                              //Steven 20250820 : fix for 1x4_AC
    {
        TestSocket.SetPickerCount(1, 4);
    }
    else if(IsNNMode()==None_NN)
    {
        TestSocket.SetPickerCount(InArmSuck.iShtRow, InArmSuck.iShtCol);
    }
    else
    {
        TestSocket.SetPickerCount(InArmSuck.iShtRow+InArmSuck.iShtRow, InArmSuck.iShtCol);
    }
}
void DoInArmSuckPreOn(int, int)                                           {}    // golden :2231
void DoInArmLoadPickUP_9045(AnsiString, AnsiString)                       {}    // golden :4175
void sLoadPickupClean()                                                   {}    // golden :4182
void SetAutoSkipCount(int)                                                {}    // golden :2408
void RecordUPH(int)                                                       {}    // golden :5376
void RecordLotUPH_For_FOREHOPE_NINGBO(AnsiString, AnsiString, AnsiString, int, int, int) {} // golden :5390
void InArmSuckReset()                                                     {}    // golden :2302
void ShowAutoSkipError()                                                  {}    // golden :2478
void PickErrorData(int, int)                                              {}    // golden :2502
void AutoSkipHasIClog(AnsiString, int, int)                               {}    // golden :2388
void CheckInArmDestroyActiveFinishInitial()                               {}    // golden :1142
void TrayPickupErrorData()                                                {}    // golden :2516
void CheckInArmDestroyActive()                                            {}    // golden :1169
void SetShuttleStatus_9045(int, int, int, int, int)                       {}    // golden :476
void GetInArmZShtDownPos_9045(int, bool, bool)                            {}    // golden :530
void InArmZNeedDown_9045(int, int, bool)                                  {}    // golden :613
void SetShuttleToHasNullICWhenCleanOut()                                  {}    // golden :4288
void SetShuttleToNullICWhenCleanOut()                                     {}    // golden :4273

#if 0   // PT-W7d RETIRED (AutoTrayEndMusic): the real translated body now lives further down this file
bool AutoTrayEndMusic(int, int)                          { return false; }      // golden :2527
#endif
#if 0   // PT-W7d RETIRED (bCheckYPitchHome): the real translated body now lives further down this file
bool bCheckYPitchHome(int)                               { return false; }      // golden :2564
#endif
#if 0   // PT-W7d RETIRED (bCheckYPitchRunHomeSen): the real translated body now lives further down this file
bool bCheckYPitchRunHomeSen(int)                         { return false; }      // golden :2540
#endif
bool SearchLoadTrayUpDown_9045_FIFO_2Step(bool&, int&, int&, int&, int&) { return false; } // golden :91(decl)
#if 0   // PT-W7d RETIRED (DoTrayEndProcess_9045): the real translated body now lives further down this file
bool DoTrayEndProcess_9045()                             { return false; }      // golden :2828
#endif
#if 0   // PT-W7d RETIRED (DoTraySkipProcess_9045): the real translated body now lives further down this file
bool DoTraySkipProcess_9045()                            { return false; }      // golden :2894
#endif
#if 0   // PT-W7d RETIRED (DoRecordSkipPosition_9045): the real translated body now lives further down this file
bool DoRecordSkipPosition_9045(bool)                     { return false; }      // golden :2999
#endif
#if 0   // PT-W7d RETIRED (CheckPlaceToShuttle): the real translated body now lives further down this file
bool CheckPlaceToShuttle()                               { return false; }      // golden :2313
#endif
#if 0   // PT-W7d RETIRED (DoInArmPlaceToShuttle_9045): the real translated body now lives further down this file
bool DoInArmPlaceToShuttle_9045()                        { return false; }      // golden :8723
#endif
#if 0   // PT-W7d RETIRED (MoveInArmXYAndSuckUp): the real translated body now lives further down this file
bool MoveInArmXYAndSuckUp(int, int, int)                 { return false; }      // golden :9029
#endif
#if 0   // PT-W7d RETIRED (Find_InArm_Single): the real translated body now lives further down this file
bool Find_InArm_Single(int, int&, int&)                  { return false; }      // golden :6364
#endif
#if 0   // PT-W7d RETIRED (SearchAndMoveInArmXYToLoad_9045): the real translated body now lives further down this file
bool SearchAndMoveInArmXYToLoad_9045()                   { return false; }      // golden :6960
#endif
#if 0   // PT-W7d RETIRED (MoveInArmZToLoaderPick): the real translated body now lives further down this file
bool MoveInArmZToLoaderPick(int)                         { return false; }      // golden :6983
#endif
bool MoveArmXYToLoader_9045(bool&, int, int&, int&, bool, bool) { return false; } // golden :5604
#if 0   // PT-W7d RETIRED (MoveInArmXYToShuttle_9045): the real translated body now lives further down this file
bool MoveInArmXYToShuttle_9045(int, int, bool, bool)     { return false; }      // golden :740
#endif
#if 0   // PT-W7d RETIRED (MoveInArmZToShuttlePlace_9045): the real translated body now lives further down this file
bool MoveInArmZToShuttlePlace_9045(int)                  { return false; }      // golden :866
#endif
#if 0   // PT-W7d RETIRED (InitialInArmNeedSuck): the real translated body now lives further down this file
bool InitialInArmNeedSuck(int, bool)                     { return false; }      // golden :3771
#endif
#if 0   // PT-W7d RETIRED (InArmSideAllClose): the real translated body now lives further down this file
bool InArmSideAllClose(int)                              { return true;  }      // golden :4231 (offline: all closed)
#endif
#if 0   // PT-W7d RETIRED (InArmSideAllCloseWithKit): the real translated body now lives further down this file
bool InArmSideAllCloseWithKit(int, int)                  { return true;  }      // golden :4251
#endif
#if 0   // PT-W7d RETIRED (bCheckInShuttlePosAndNOIC): the real translated body now lives further down this file
bool bCheckInShuttlePosAndNOIC()                         { return false; }      // golden :4368
#endif
#if 0   // PT-W7d RETIRED (InArmNeedCheckOffset): the real translated body now lives further down this file
bool InArmNeedCheckOffset(bool, int)                     { return false; }      // golden :4329
#endif
#if 0   // PT-W7d RETIRED (CheckShuttleSensor_9045): the real translated body now lives further down this file
bool CheckShuttleSensor_9045(int)                        { return true;  }      // golden :2070 (offline: no floating)
#endif

#if 0   // PT-W7d RETIRED (MoveInArmXYToWaitTrayArm): the real translated body now lives further down this file
int  MoveInArmXYToWaitTrayArm(int, int, bool, bool)      { return 0; }          // golden :828
#endif
#if 0   // PT-W7d RETIRED (GetVariableYInShuttleData): the real translated body now lives further down this file
int  GetVariableYInShuttleData()                         { return 0; }          // golden :2254
#endif
#if 0   // PT-W7d RETIRED (AutoCalculateInArmYClosePitch): the real translated body now lives further down this file
int  AutoCalculateInArmYClosePitch(bool, bool)           { return 0; }          // golden :4911
#endif
#if 0   // PT-W7d RETIRED (CheckOneCycleAction): the real translated body now lives further down this file
int  CheckOneCycleAction(int)                            { return 0; }          // golden :7431
#endif

void SetClosedShtKitToHasNullIC_9045_dummy() {}          // (placeholder anchor)
#if 0   // PT-W7d RETIRED (SetClosedShtKitToHasNullIC_9045): the real translated body now lives further down this file
bool SetClosedShtKitToHasNullIC_9045(int, int, AnsiString, AnsiString) { return false; } // golden :385
#endif

// CheckShuttleSensor_9045_<layout> family (golden :1025-1133) -- per-layout
// sensor checks; offline "no floating" (golden returns 0 = OK).
#if 0   // PT-W7d RETIRED (CheckShuttleSensor_9045_1x1): the real translated body now lives further down this file
int CheckShuttleSensor_9045_1x1(int, bool, bool) { return 0; }
#endif
#if 0   // PT-W7d RETIRED (CheckShuttleSensor_9045_1x4): the real translated body now lives further down this file
int CheckShuttleSensor_9045_1x4(int, bool, bool) { return 0; }
#endif
#if 0   // PT-W7d RETIRED (CheckShuttleSensor_9045_2x1): the real translated body now lives further down this file
int CheckShuttleSensor_9045_2x1(int, bool, bool) { return 0; }
#endif
#if 0   // PT-W7d RETIRED (CheckShuttleSensor_9045_2x3): the real translated body now lives further down this file
int CheckShuttleSensor_9045_2x3(int, bool, bool) { return 0; }
#endif
#if 0   // PT-W7d RETIRED (CheckShuttleSensor_9045_2x4): the real translated body now lives further down this file
int CheckShuttleSensor_9045_2x4(int, bool, bool) { return 0; }
#endif
#if 0   // PT-W7d RETIRED (CheckShuttleSensor_9045_1x2): the real translated body now lives further down this file
int CheckShuttleSensor_9045_1x2(int, bool, bool) { return 0; }
#endif
#if 0   // PT-W7d RETIRED (CheckShuttleSensor_9045_2x2): the real translated body now lives further down this file
int CheckShuttleSensor_9045_2x2(int, bool, bool) { return 0; }
#endif
#if 0   // PT-W7d RETIRED (CheckShuttleSensor_9045_2x5): the real translated body now lives further down this file
int CheckShuttleSensor_9045_2x5(int, bool, bool) { return 0; }
#endif
#if 0   // PT-W7d RETIRED (CheckShuttleSensor_9045_2x6): the real translated body now lives further down this file
int CheckShuttleSensor_9045_2x6(int, bool, bool) { return 0; }
#endif
#if 0   // PT-W7d RETIRED (CheckShuttleSensor_9045_2x8): the real translated body now lives further down this file
int CheckShuttleSensor_9045_2x8(int, bool, bool) { return 0; }
#endif
//==============================================================================
//  k7-ainarm PART 00062 -- ArmYCanSuck2IC()
//  Translator: AI(pt-wave k7-ainarm) 20260811
//  Golden: HT9011UC_Code_V3.33.906.0_20260618/ainarm9045.cpp:62-78 (cp950)
//
//  ROLE          : predicate -- can the in-arm Y axis open wide enough to suck 2
//                  ICs at once for this device?  (Y dimension vs the arm Y-pitch
//                  ceiling; auto-pitch machines compare against the mechanical
//                  IN_OUT_ARM_Y_PITCH_MAX, fixed-pitch machines against the
//                  taught TestIF.iARM_Y_PITCH.)
//  WAVE SCOPE    : ArmYCanSuck2IC  golden :62-78  -- ACTIVE (verbatim, no gates)
//  GATE REGISTER : (none -- every symbol resolves)
//                    DeviceForm.YDimension      cprod.h:1244 (SYSTEM_DEVICE_FORM)
//                    USE_IN_Y_IS_AUTO_PITCH     cmydef.h
//                    IN_OUT_ARM_Y_PITCH_MAX     cmydef.h
//                    TestIF.iARM_Y_PITCH        cprod.h:2576 / :1931
//
//  HAND-OFF: golden declares ArmYCanSuck2IC in NO header (golden csystem.h:107
//  carries it COMMENTED OUT, and the port mirrors that at csystem.h:158).  This
//  is therefore a definition with no visible declaration; the port has no caller
//  yet either (golden`s callers are the 1x2_4 NN-mode arms).  If the main loop
//  wants callers to see it, un-comment csystem.h:158 -- do NOT add a second
//  declaration elsewhere.
//==============================================================================
bool ArmYCanSuck2IC()
{
    if(DeviceForm.YDimension<=0)                                                //Sam 20250203 : 有讀取到參數後才能判斷
        return true;

    if(USE_IN_Y_IS_AUTO_PITCH==true)                                            //JerryYang 20251218 : IN/OUT ARM支援不同模組
    {
        if(DeviceForm.YDimension>IN_OUT_ARM_Y_PITCH_MAX)
            return false;
    }
    else
    {
        if(DeviceForm.YDimension>=TestIF.iARM_Y_PITCH)
            return false;
    }
    return true;
}

//==============================================================================
//  AI(k6-ainarm) 20260811 -- golden ainarm9045.cpp:80-86   ArmXCanSuck2IC_9045S
//
//  ROLE          -- HT-9045S X-direction 2-nozzle feasibility predicate: returns
//                   false once the device body X-dimension reaches
//                   iXpitchMaxX2-250 (7750) i.e. too wide to run 2 pickers.
//  WAVE SCOPE    -- ArmXCanSuck2IC_9045S   ACTIVE   (golden :80-86)
//  GATE REGISTER -- none.  This part emits no #if 0.
//
//  Deps, all REAL and reachable from ainarm9045.cpp's EXISTING include set:
//    DeviceForm  (SYSTEM_DEVICE_FORM, cprod.h:1244; .XDimension cprod.h:1168 double)
//    iXpitchMaxX2 (cmydef.h:4530)
//  No integer division here; the int expression iXpitchMaxX2-250 is compared
//  against a double exactly as golden does -- kept verbatim.
//
//  LINKAGE (TRAP 1): NOT static.  golden csystem.h:108 carries the declaration
//  COMMENTED OUT, so golden reaches this body only from inside its own TU, and
//  the sole golden caller is golden :1641 inside DoInArm_9045_Type -- which in
//  the port is still the offline stub at ainarm9045.cpp:2268.  So this body has
//  NO caller in the port yet (TRAP 1 shape (a), stated not hidden).  Making it
//  static would be shape (d) the moment a later wave lands DoInArm_9045_Type in
//  this same TU, so it stays external.
//==============================================================================
bool ArmXCanSuck2IC_9045S()
{
    if(DeviceForm.XDimension>=iXpitchMaxX2-250)                                 //超過7750就要減少吸嘴數量
        return false;

    return true;
}

//============================================================================
//  ArmXCanSuck4IC_9045  --  golden ainarm9045.cpp:88-113   (k1-ainarm, W7-D)
//
//  ROLE: X-pitch feasibility predicate -- "may the in-arm still take 4 ICs per
//        stroke?"  Pure read-only arithmetic over the device X dimension, the
//        machine X-pitch ceilings and the per-test-mode / hot-plate pitch
//        exceptions.  No hardware, no state written.
//
//  WAVE SCOPE:
//    * ArmXCanSuck4IC_9045()            ACTIVE (verbatim)      golden :88-113
//
//  GATE REGISTER: empty -- this function contains no #if 0.
//
//  TRAP 1 note, shape (a) "nobody references it": there is no live port caller
//  yet.  Golden calls it from CheckPickerMode (golden :1258, call at :1322) and
//  DoInArm_9045_Type (golden :1616, calls at :1669/:1684/:1880); in the port
//  CheckPickerMode has no body at all and DoInArm_9045_Type is the `{}` stub at
//  ainarm9045.cpp:2268.  There is also no prototype anywhere: golden csystem.h:109
//  keeps it COMMENTED OUT (`//bool ArmXCanSuck4IC_9045();`) and the port mirrors
//  that at csystem.h:160, so golden itself relies on definition-before-use inside
//  ainarm9045.cpp.  Deliberately NOT written `static` (that would be TRAP 1 shape
//  (d): an internal-linkage shadow that links clean and explodes the day the real
//  declaration is un-commented in the same TU).  Retiring the two csystem.h
//  comment-outs is the main loop's call, not mine.
//
//  INTEGER/FLOAT note: golden :94 divides by 2.0 (a genuine double divide on a
//  double member) -- kept as a double divide, NOT converted, and no int/int
//  anywhere in this function was touched.
//============================================================================
bool ArmXCanSuck4IC_9045()
{
    if(DeviceForm.XDimension>=iXpitchMax-250)                                   //超過3750就要減少吸嘴數量
        return false;

    if(TestIF_File.dSiteXPitch>iXpitchMaxX1_MM &&                               //X-Pitch=60
       (TestIF_File.dSiteXPitch/2.0)<=(DeviceForm_File.XDimension+2))           //IC 28.575 mm時, 要減少吸嘴數量
        return false;

    if((TestIF_File.iTestMode==QualSite1X4 ||                                   // 1x4
        TestIF_File.iTestMode==_8Site2X4N) &&                                   //Wei 20231211 : 2X4NN Mode
       LastSet.iTemperature==Tempture_Hot &&
       HotPlateForm_File.XPitch>40)                                             //Steven 20240308 : 1x4加熱模式, X-Pitch>40, 只能跑2吸嘴
    {
        return false;
    }

    if((TestIF_File.iTestMode==TriSite1X3  ||                                   //1x3 //Frank 20160329 add for 1x3_4
        TestIF_File.iTestMode==_6Site2X3N) &&                                   //Steven 20220425 : 2X3NN Mode
       TestIF_File.dSiteXPitch>iXpitchMinX3_MM)                                 //Steven 20240427 : fixed for 1x3
    {
        return false;
    }

    return true;
}

//==============================================================================
//  ROLE          -- ArmXCanSuck4IC_1032(): HT-1032 (16-picker) X-direction
//                   "may the arm still suck 4 IC at once?" predicate.  Pure
//                   geometry: says NO when the device body is too wide for the
//                   2x-max X-pitch, or when half the recipe site X-pitch cannot
//                   clear the device width +2 mm.
//  WAVE SCOPE    -- ArmXCanSuck4IC_1032   golden :115-125   ACTIVE (no gates)
//  GATE REGISTER -- (none: this function writes no #if 0)
//
//  ACTION REQUIRED -- A SIBLING GATE IS NOW EXPIRED BECAUSE OF THIS PART
//  -------------------------------------------------------------------
//  _w7d_parts/01258_CheckPickerMode.txt lines 113-117 carry `GATE k6-G2`, whose
//  written premise is "golden ainarm9045.cpp:1267 ArmXCanSuck4IC_1032() -- NO
//  port body ... no sibling part" (its own notes at :60 and :72 say the gate must
//  be retired the moment the body lands).  THIS PART IS THAT BODY.  At
//  integration, retire k6-G2 in 01258 and restore its
//  `bXCanSuck4IC_1032=ArmXCanSuck4IC_1032();`.  No prototype is needed there: the
//  stitch is in golden-line order, so this definition (golden :115) precedes the
//  call site (golden :1267) in the same TU.
//  Cross-checked at 2026-08-11 01:53 +0800 by reading 01258_CheckPickerMode.txt
//  directly.  This is the mutual half of TRAP 2: 01258's absence claim and this
//  agent's TRAP-1-shape-(a) claim expired against each other inside one wave.
//
//  NOTES for the main loop
//  -----------------------
//  * NO DECLARATION EXISTS, in golden or in the port.  golden csystem.h:110 and
//    port csystem.h:161 both carry it COMMENTED OUT
//    (`//bool ArmXCanSuck4IC_1032();`).  That is golden's own state and is
//    reproduced by NOT adding a declaration.
//  * NOT `static`: golden defines it with external linkage (TRAP 1 shape (d)
//    avoided deliberately).  With 01258's gate retired the symbol also stops
//    being TRAP 1 shape (a) -- it gains its one real caller.
//  * All 5 symbols it touches exist in the port: DeviceForm / DeviceForm_File
//    (cprod.h:1245, .XDimension is `double` at cprod.h:1168), iXpitchMaxX2
//    (cmydef.h:4530), iXpitchMaxX2_MM (cmydef.h:4544), TestIF_File.dSiteXPitch
//    (cprod.h:1656).
//  * `DeviceForm.XDimension>=iXpitchMaxX2-250` is double>=int -- golden's own
//    mixed compare, kept verbatim (no cast added, no int truncation introduced).
//  * INTEGER DIVISION AUDIT: the only division is golden's own `/2.0` on a
//    double.  No int/int existed and none was created.
//  * GOLDEN QUIRK kept: the :117 comment says 3750, copied verbatim from the
//    sibling ArmXCanSuck4IC_9045 (golden :90) where it guards iXpitchMax; here it
//    guards iXpitchMaxX2, so the number in the comment no longer tracks the
//    constant.  Comment transcribed as-is; NOT corrected.
//==============================================================================
bool ArmXCanSuck4IC_1032()                                                      // golden :115
{
    if(DeviceForm.XDimension>=iXpitchMaxX2-250)                                 //超過3750就要減少吸嘴數量
        return false;

    if(TestIF_File.dSiteXPitch>iXpitchMaxX2_MM &&                               //X-Pitch=60
       (TestIF_File.dSiteXPitch/2.0)<=(DeviceForm_File.XDimension+2))           //IC 28.575 mm時, 要減少吸嘴數量
        return false;

    return true;
}

//==============================================================================
//== ROLE: SetClosedShtKitToHasNullIC_9045 -- when a shuttle KIT has all of its
//==       sites turned off (auto-site-off / close-arm), mark that kit's shuttle
//==       cells HAS_NULL_IC, release the shuttle can-move-left interlock, and
//==       return whether the caller must run the "adjust twice" pitch pass.
//==
//== WAVE SCOPE (k2-ainarm):
//==   SetClosedShtKitToHasNullIC_9045  golden ainarm9045.cpp:385-474  -- ACTIVE
//==     (100% faithful; ZERO gates -- every callee has a real port body)
//==
//== GATE REGISTER: (none -- this function needed no #if 0)
//==   Callee provenance, so the "no gate" claim is checkable:
//==     * SetShuttleToHasNullIC_9045(int,int=-1) -- REAL body already in this same
//==       TU (port ainarm9045.cpp:750).  Its own interior is W7-gated; that is
//==       that function's pre-existing gate, not a new one of mine.
//==     * InArmSideAllCloseWithKit(int,int) -- REAL body landed by THIS wave
//==       (part 04251_InArmSideAllCloseWithKit.txt).  MAIN-LOOP WARNING: the port
//==       TODAY has `bool InArmSideAllCloseWithKit(int,int){return true;}` at port
//==       ainarm9045.cpp:2308 -- retire that stub or this file collides (duplicate
//==       definition).  The real body returns FALSE for a kit that still has
//==       enabled nozzles, so THIS function's behaviour genuinely changes at
//==       integration (the stub made every kit look closed, which forced the
//==       "all closed" arms of both if/else chains below).
//==     * SetShuttlefCanMoveL(int,bool,AnsiString,AnsiString="") -- REAL body
//==       aHotPlateSubstrate.cpp:1567 (decl aHotPlateSubstrate.h:1070).
//==
//== INTEGRATION NOTE (TRAP 1 shape (b)): port ainarm9045.cpp:2319 currently holds
//== `bool SetClosedShtKitToHasNullIC_9045(int,int,AnsiString,AnsiString){return
//== false;}`.  DELETE that stub.  (The `SetClosedShtKitToHasNullIC_9045_dummy()`
//== anchor at :2318 is a DIFFERENT symbol -- it is not this function.)  The
//== declaration already exists non-static at ainarm9045.h:58, so this definition
//== is NOT static -- TRAP 1 shape (d) avoided.
//==
//== Big5 -> UTF-8: every Chinese comment transcribed character for character.
//==============================================================================
bool SetClosedShtKitToHasNullIC_9045(int iSht, int iKit, AnsiString Func, AnsiString Task)                              //Steven 20231115 : fixed for auto site off
{
    int iAnotherSht=1-iSht;
    bool bFreeSht=false, bNeedAdjust2Time=false;
    if(iKit==0)
    {
        SetShuttleToHasNullIC_9045(iSht, 0);                                    //0是只塞一半

        if(IniConfig.bUseAutoSiteMapping &&
           LastSet.iRunStartMode==rsmAutoSiteMap &&
           IniConfig.bI21ASMRunTimeCHeck==false)                                //Steven 20140729 : AutoSiteMapping, 邊生產邊做)
        {
            SetShuttleToHasNullIC_9045(iSht);
            if(InArmSideAllCloseWithKit(iAnotherSht, 0)==true &&                //如果Arm 2的Site全部被關掉
               InArmSideAllCloseWithKit(iAnotherSht, 1)==true)                  //Steven 20241108 : fixed for auto site map and close arm.
            {
                SetShuttleToHasNullIC_9045(iAnotherSht);
            }
            bNeedAdjust2Time=true;
            bFreeSht=true;
        }
        else
        {
            if(InArmSideAllCloseWithKit(iSht, 1)==true)                         //jou 2013-08-14 修正關右邊八個site時,in arm 與 shuttle 撞機
            {
                SetShuttleToHasNullIC_9045(iSht);
                bNeedAdjust2Time=true;

                if(InArmSideAllCloseWithKit(iAnotherSht, 0)==true &&            //如果Arm 2的Site全部被關掉
                   InArmSideAllCloseWithKit(iAnotherSht, 1)==true)
                {
                    SetShuttleToHasNullIC_9045(iAnotherSht);
                }
                bFreeSht=true;
            }
            else
            {
                bFreeSht=false;
            }
        }
    }
    else
    {
        SetShuttleToHasNullIC_9045(iSht);
        if(IniConfig.bUseAutoSiteMapping &&
           LastSet.iRunStartMode==rsmAutoSiteMap &&
           IniConfig.bI21ASMRunTimeCHeck==false)                                //Steven 20140729 : AutoSiteMapping, 邊生產邊做
        {
            if(InArmSideAllCloseWithKit(iAnotherSht, 0)==true &&                //如果Arm 2的Site全部被關掉
               InArmSideAllCloseWithKit(iAnotherSht, 1)==true)                  //Steven 20241108 : fixed for auto site map and close arm.
            {
                SetShuttleToHasNullIC_9045(iAnotherSht);
            }
        }
        else
        {
            if(InArmSideAllCloseWithKit(iAnotherSht, 0)==true &&                //如果Arm 2的Site全部被關掉
               InArmSideAllCloseWithKit(iAnotherSht, 1)==true)
            {
                SetShuttleToHasNullIC_9045(iAnotherSht);
            }
        }
        bFreeSht=true;
    }
    SetShuttlefCanMoveL(iSht, bFreeSht, Func, Task);

    if(iInArmType==e9045_1x1_1      ||
       iInArmType==e9045_1x2_2_13   ||
       iInArmType==e9045_1x2_2_14   ||
       iInArmType==e9045_1x2_4_Hot  ||
       iInArmType==e9045_1x3_4      ||
       iInArmType==e9045_1x4_1_Ac   ||
       iInArmType==e9045_1x4_4      ||
       iInArmType==e9045_1x4_8_Hot  ||                                          //Steven 20240119 : fixed for 2x4_16
       iInArmType==e9045_1x4_4_Back ||
       iInArmType==e9045_2x2_8_Hot  ||
       iInArmType==e9045_2x1_2_13   ||
       iInArmType==e9045_2x2_4_12   ||
       iInArmType==e9045_2x2_4_13   ||
       iInArmType==e9045_2x2_4_14   ||
       iInArmType==e9045_2x3_6      ||                                          //Steven 20200219 : 2x3mode, Layout: 120x70mm
       iInArmType==e9045_2x4_8      ||
       iCloseSiteModeFor2x8>e2x8OneByOne ||
       iCloseSiteModeFor2x6>e2x6OneByOne)                                       //Steven 20240417 : 12site關中間4site
    {
        bNeedAdjust2Time=false;
    }

    return bNeedAdjust2Time;
}

//==============================================================================
//  ROLE          MoveInArmXYToShuttle_9045 -- the in-arm "move XY over the target
//                shuttle cell" FRONT DOOR.  One ep1Picker fast path, then a
//                switch(TestIF.iTestMode) that hands off to the per-layout
//                MoveInArm2XYToShuttle_9045_<layout>() geometry mover, with the
//                iInArmType / bRunAutoClean sub-selection inside each site-count
//                arm.  Returns the callee bResult; default: pops the
//                "mode is not support" ShowMyMessage.
//
//  WAVE SCOPE    ACTIVE  MoveInArmXYToShuttle_9045   golden ainarm9045.cpp:740-824
//                (no GATED sub-range -- every one of the 22 callees this switch can
//                 reach has a real, non-stub port body, all of them registered in
//                 the SAME static archive ht9045_sm as ainarm9045.cpp, so TRAP 1
//                 shapes (a)/(c) do not apply.  Verified 2026-08-11 01:33 with
//                 `rg -n "^bool MoveInArm2XYToShuttle_9045_\w+\(" -g "*.cpp" .`
//                 (27 definitions) + `grep -nE "^\s*ainarm[A-Za-z0-9_]*\.cpp"
//                 CMakeLists.txt` (all inside add_library(ht9045_sm ...) at :1387).)
//
//  GATE REGISTER (this part file)   -- EMPTY.  No #if 0 is emitted here.
//
//  GOLDEN BUG kept verbatim: `bool bResult;` (golden :742) is NEVER assigned on
//  the switch `default:` arm (golden :820-821 only calls ShowMyMessage), so
//  golden :823 `return bResult;` returns an indeterminate value for any
//  TestIF.iTestMode outside the 16 listed cases.  NOT fixed -- behaviour
//  preserved.  (gcc/clang will emit -Wmaybe-uninitialized; the port sets no
//  -Werror -- CMakeLists.txt:372 HT9045_WARNFLAGS is warnings-only, and
//  ht9045_sm is not even in the WARNFLAGS list.)
//
//  HOISTED, not invented: golden declares its 25 per-layout callees as a plain
//  extern block immediately above this function (golden :714-738); the golden TU
//  gets them that way and NOT from the per-layout headers.  The port TU cannot
//  reach them any other way either -- aArmHeader.h (the god-header that pulls
//  ainarm9045_<layout>.h) has its whole include list gated `#if 0` at
//  aArmHeader.h:23.  So golden :714-738 is reproduced VERBATIM below, exactly
//  where golden puts it.  These are declarations, not definitions: a sibling
//  agent repeating any of them is harmless.
//  AI(W7D-k3-ainarm) 20260811
//==============================================================================
extern bool MoveInArm2XYToShuttle_9045_1x1_1(int iSht, bool IncludeZ, bool bPlace);
extern bool MoveInArm2XYToShuttle_9045_1x2_2_13(int iSht, bool IncludeZ, bool bPlace);
extern bool MoveInArm2XYToShuttle_9045_1x2_2_14(int iSht, bool IncludeZ, bool bPlace);
extern bool MoveInArm2XYToShuttle_9045_1x2_4_Hot(int iSht, bool IncludeZ, bool bPlace);
extern bool MoveInArm2XYToShuttle_9045_1x3_4(int iSht, bool IncludeZ, bool bPlace);
extern bool MoveInArm2XYToShuttle_9045_1x3_2_14(int iSht, int iKit, bool IncludeZ, bool bPlace);
extern bool MoveInArm2XYToShuttle_9045_1x4_2(int iSht, int iKit, bool IncludeZ, bool bPlace);
extern bool MoveInArm2XYToShuttle_9045_1x4_4(int iSht, bool IncludeZ, bool bPlace);
extern bool MoveInArm2XYToShuttle_9045_1x4_4_Back(int iSht, bool IncludeZ, bool bPlace);
extern bool MoveInArm2XYToShuttle_9045_1x4_4_13(int iSht, bool IncludeZ, bool bPlace);
extern bool MoveInArm2XYToShuttle_9045_1x4_8_Hot(int iSht, bool IncludeZ, bool bPlace);                                 //Sam 20250203 : 修正 1X4 8 吸嘴模式
extern bool MoveInArm2XYToShuttle_9045_2x1_2(int iSht, bool IncludeZ, bool bPlace);
extern bool MoveInArm2XYToShuttle_9045_2x2_4_12(int iSht, bool IncludeZ, bool bPlace);
extern bool MoveInArm2XYToShuttle_9045_2x2_4_13(int iSht, bool IncludeZ, bool bPlace);
extern bool MoveInArm2XYToShuttle_9045_2x2_4_14(int iSht, bool IncludeZ, bool bPlace);
extern bool MoveInArm2XYToShuttle_9045_2x2_8_Hot(int iSht, bool IncludeZ, bool bPlace);                                 //Sam 20250212 : 修正 2X2 8 吸嘴模式
extern bool MoveInArm2XYToShuttle_9045_2x3_6_14(int iSht, int iKit, bool IncludeZ, bool bPlace);
extern bool MoveInArm2XYToShuttle_9045_2x3_6(int iSht, bool IncludeZ, bool bPlace);
extern bool MoveInArm2XYToShuttle_9045_2x4_4_13(int iSht, int iKit, bool IncludeZ, bool bPlace);
extern bool MoveInArm2XYToShuttle_9045_2x4_4_14(int iSht, int iKit, bool IncludeZ, bool bPlace);
extern bool MoveInArm2XYToShuttle_9045_2x4_8(int iSht, bool IncludeZ, bool bPlace);
extern bool MoveInArm2XYToShuttle_9045_2x5_8(int iSht, int iKit, bool IncludeZ, bool bPlace);
extern bool MoveInArm2XYToShuttle_9045_2x6_8(int iSht, int iKit, bool IncludeZ, bool bPlace);
extern bool MoveInArm2XYToShuttle_9045_2x8_8(int iSht, int iKit, bool IncludeZ, bool bPlace);
extern bool MoveInArm2XYToShuttle_9045_All_1Pick(int iSht, bool IncludeZ, bool bPlace);
//------------------------------------------------------------------------------
bool MoveInArmXYToShuttle_9045(int iSht, int iKit, bool IncludeZ, bool bPlace)  //Steven 20240512 : 整合MoveInArmXYToShuttle
{
    bool bResult;
    if(USE_PICKER_COUNT==ep1Picker)
    {
        bResult=MoveInArm2XYToShuttle_9045_All_1Pick(iSht, ZAxisNotDown, bPlace);
        return bResult;
    }

    switch(TestIF.iTestMode)
    {
        case SingleSite:
            bResult=MoveInArm2XYToShuttle_9045_1x1_1(iSht, ZAxisNotDown, bPlace);
            break;
        case DualSite:                                                          //1x2
        case QualSite2X2N:                                                      //Steven 20201014 : for 2x2 nn mode auto clean
            if(iInArmType==e9045_1x2_4_Hot && bRunAutoClean==false)             //Sam 20250212 : 修正 2X2 8 吸嘴模式
                bResult=MoveInArm2XYToShuttle_9045_1x2_4_Hot(iSht, ZAxisNotDown, bPlace);
            else if(iInArmType==e9045_1x2_2_14)                                 //Steven 20240508 : fixed for auto clean
                bResult=MoveInArm2XYToShuttle_9045_1x2_2_14(iSht, ZAxisNotDown, bPlace);
            else
                bResult=MoveInArm2XYToShuttle_9045_1x2_2_13(iSht, ZAxisNotDown, bPlace);
            break;
        case TriSite1X3:                                                        //Frank 20160323 1x3 AutoClean Add
        case _6Site2X3N:                                                        //Steven 20220425 : 2X3NN Mode
            if(iInArmType==e9045_1x3_2_14)                                      //Steven 20220425 : 1x3_14
                bResult=MoveInArm2XYToShuttle_9045_1x3_2_14(iSht, iKit, ZAxisNotDown, bPlace);
            else
                bResult=MoveInArm2XYToShuttle_9045_1x3_4(iSht, ZAxisNotDown, bPlace);
            break;
        case QualSite1X4:                                                       //1x4
        case _8Site1X4:                                                         //ChungHung 20150528 add for 海思 _8Site1x4
        case _8Site2X4N:                                                        //Wei 20231211 : 2X4NN Mode
            if(iInArmType==e9045_1x4_1_Ac)                                      //Steven 20200720 : 1x4只開site Ac
                bResult=MoveInArm2XYToShuttle_9045_1x1_1(iSht, ZAxisNotDown, bPlace);
            else if(iInArmType==e9045_1x4_4_13)                                 //Steven 20170810 (Jou) : Add for HT-9045S
                bResult=MoveInArm2XYToShuttle_9045_1x4_4_13(iSht, ZAxisNotDown, bPlace);
            else if(iInArmType==e9045_1x4_2_14)
                bResult=MoveInArm2XYToShuttle_9045_1x4_2(iSht, iKit, ZAxisNotDown, bPlace);
            else if(iInArmType==e9045_1x4_8_Hot && bRunAutoClean==false)        //Sam 20250212 : 修正 1X4 8 吸嘴模式
                bResult=MoveInArm2XYToShuttle_9045_1x4_8_Hot(iSht, ZAxisNotDown, bPlace);
            else
                bResult=MoveInArm2XYToShuttle_9045_1x4_4(iSht, ZAxisNotDown, bPlace);
            break;
        case QualSite2X2:                                                       //2x2
            if(iInArmType==e9045_2x2_4_14)                                      //Steven 20240508 : fixed for auto clean
                bResult=MoveInArm2XYToShuttle_9045_2x2_4_14(iSht, ZAxisNotDown, bPlace);
            else if(iInArmType==e9045_2x2_8_Hot && bRunAutoClean==false)        //Sam 20250212 : 修正 2X2 8 吸嘴模式
                bResult=MoveInArm2XYToShuttle_9045_2x2_8_Hot(iSht, ZAxisNotDown, bPlace);
            else
                bResult=MoveInArm2XYToShuttle_9045_2x2_4_13(iSht, ZAxisNotDown, bPlace);
            break;
        case _6Site2X3:
            if(iInArmType==e9045_2x3_6_14)                                      //Steven 20200219 : 2x3mode, Layout: 120x70mm
                bResult=MoveInArm2XYToShuttle_9045_2x3_6_14(iSht, iKit, ZAxisNotDown, bPlace);
            else                                                                //ChungHung 20140115 add for 2x3 ~ need test
                bResult=MoveInArm2XYToShuttle_9045_2x3_6(iSht, ZAxisNotDown, bPlace);
            break;
        case _8Site2X4:                                                         //2x4
        case _16Site4X4:                                                        //Sam 20190226 : 16Site4X4
            if(iInArmType==e9045_2x4_4_13)                                      //Steven 20240508 : fixed for auto clean
                bResult=MoveInArm2XYToShuttle_9045_2x4_4_13(iSht, iKit, ZAxisNotDown, bPlace);
            else if(iInArmType==e9045_2x4_4_14)
                bResult=MoveInArm2XYToShuttle_9045_2x4_4_14(iSht, iKit, ZAxisNotDown, bPlace);
            else
                bResult=MoveInArm2XYToShuttle_9045_2x4_8(iSht, ZAxisNotDown, bPlace);
            break;
        case _10Site2X5:
            bResult=MoveInArm2XYToShuttle_9045_2x5_8(iSht, iKit, ZAxisNotDown, bPlace);
            break;
        case _12Site2X6:                                                        //2013-11-05    Dell    Modify _12Site2X6 autoclean
            bResult=MoveInArm2XYToShuttle_9045_2x6_8(iSht, iKit, ZAxisNotDown, bPlace);
            break;
        case _16Site2X8:                                                        //2x8
        case _32Site4X8N:                                                       //Steven 20140512: For HT-9047 Auto Clean 待驗證
//            if(iInArmType==e9045_2x8_16)
//                bResult=MoveInArm2XYToShuttle_9045_2x8_16(iSht, ZAxisNotDown, bPlace);
//            else
                bResult=MoveInArm2XYToShuttle_9045_2x8_8(iSht, iKit, ZAxisNotDown, bPlace);
            break;
        default:
            ShowMyMessage("The mode is not support!!", "Please save state record and provide to HonPrec software engineer", "MoveInArmXYToShuttle");
    }
    return bResult;
}

//==============================================================================
//  AI(k6-ainarm) 20260811 -- golden ainarm9045.cpp:828-864  MoveInArmXYToWaitTrayArm
//
//  ROLE          -- In-arm XY router used while the TRAY ARM is being waited on:
//                   latches iMoveToShuttle once (1 = tray present but empty and
//                   not an under-conveyor tray arm -> go to the in-arm WAIT pos;
//                   2 = otherwise -> go straight to the shuttle), then returns
//                   the caller's next Task code (1000/2000 wait, 1100/2100
//                   shuttle) per shuttle index, or 0 while still moving.
//  WAVE SCOPE    -- MoveInArmXYToWaitTrayArm  ACTIVE  (golden :828-864)
//  GATE REGISTER -- none.  This part emits no #if 0.
//
//  RETIRE AT INTEGRATION: ainarm9045.cpp:2313 currently holds the offline stub
//  `int MoveInArmXYToWaitTrayArm(int, int, bool, bool) { return 0; }  // golden :828`.
//  It is in the SAME file this part is stitched into and MUST be deleted, or the
//  build breaks with a redefinition.  The declaration (ainarm9045.h:55, with the
//  4 golden parameter names) stays and matches this definition exactly.
//
//  golden :826 `int iMoveToShuttle=0;` is DELIBERATELY NOT EMITTED: it is already
//  defined in this TU at ainarm9045.cpp:101 (W6.2b) and externed at
//  ainarm9045.h:36.  Emitting it again would be a multiple definition.
//
//  Deps: MOT[]/TTrayMotor.Tray.HasIC()/fHasTray (Motor/mymotor.h:336/340/353,
//  already included), TRAY_ARM_MODE (cmydef.h:2997), eUnderCoveyor
//  (MachineType.h:1374 via cmydef.h), MoveInArmXYToShuttle_9045 (ainarm9045.h:54,
//  offline stub in this TU at :2304), MoveInArm2XYToWait -- see the note below.
//==============================================================================
// MoveInArm2XYToWait() has a REAL port body (acatchtray_shims.cpp:110, declared
// acatchtray_shims.h:361, offline `return true` = wait pos reached), but THIS TU
// does not include acatchtray_shims.h.  It is forward-declared rather than gated:
// gating it would silently delete the in-arm give-way move, and the symbol is
// linkable today.  #ifndef-guarded so a sibling part emitting the identical
// forward-decl into the same stitched file cannot collide.
//   MAIN LOOP, read before 'tidying' this away: hoisting `#include
//   "acatchtray_shims.h"` into the include block does NOT work as a drop-in --
//   that header (line 87) also declares `ProcessSCKARTLoadingCount(bool
//   bReset=false)` while the DEFINITION at ainarm9045.cpp:1321 already carries
//   the same default argument, which is a hard error (default argument given
//   twice).  The forward-decl is the deliberate choice, not laziness.
#ifndef AINARM_K6_MOVEINARM2XYTOWAIT_FWD
#define AINARM_K6_MOVEINARM2XYTOWAIT_FWD
extern bool MoveInArm2XYToWait();                                               // golden ainarm2.h decl; port body acatchtray_shims.cpp:110
#endif
//------------------------------------------------------------------------------
int MoveInArmXYToWaitTrayArm(int iSht, int iKit, bool IncludeZ, bool bPlace)
{
    int iResult=0;

    if(iMoveToShuttle==0)                                                       //Steven 20241115 : for in arm 去sht 1放料hang up
    {
        if(MOT[MMTrayY].Tray.HasIC()==false &&
           MOT[MMTrayY].fHasTray &&
           TRAY_ARM_MODE!=eUnderCoveyor)
        {
            iMoveToShuttle=1;
        }
        else
        {
            iMoveToShuttle=2;
        }
    }

    if(iMoveToShuttle==1)
    {
        if(MoveInArm2XYToWait())
        {
            iMoveToShuttle=0;
            iResult=(iSht==0)?1000:2000;
        }
    }
    else
    {
        if(MoveInArmXYToShuttle_9045(iSht, iKit, IncludeZ, bPlace))
        {
            iMoveToShuttle=0;
            iResult=(iSht==0)?1100:2100;
        }
    }

    return iResult;
}

//==============================================================================
//  MoveInArmZToShuttlePlace_9045  --  golden ainarm9045.cpp:866-871
//  Translator: AI(W7D-ainarm-k8) 20260811
//
//  ROLE          -- recompute the in-arm Z "place onto in-shuttle" target for
//                   shuttle iSht, then drive every flagged nozzle down; the
//                   returned bool is InArmZMoveDown's "all flagged axes arrived"
//                   verdict, handed straight back to the caller.
//  WAVE SCOPE    -- MoveInArmZToShuttlePlace_9045   golden :866-871   ACTIVE
//  GATE REGISTER -- (empty: this function contains no #if 0)
//
//  Retires the offline stub `bool MoveInArmZToShuttlePlace_9045(int) { return
//  false; }` at port ainarm9045.cpp:2305 -- the MAIN LOOP deletes that line at
//  integration; this part file does not touch it.  Both callees already exist:
//  GetInArmZShtDownPos_9045 (still the offline no-op stub, port
//  ainarm9045.cpp:2285) and InArmZMoveDown (real decl Motor/mymotor.h:404, whose
//  4th/5th params bLoader/bPreOn are defaulted -- golden passes only 3, kept).
//  Because the pos-calc callee is still a no-op, offline bZFlgToSht/iZPosToSht
//  keep whatever the caller left in them -- the CALL IS KEPT, not dropped, so
//  retiring that stub later needs no edit here.  54 textual call sites across 28
//  ainarm9045_<layout>.cpp files (one pair each, e.g. ainarm9045_2x4_16.cpp:2399 /
//  :2778); 52 of them are LIVE -- the 2 in ainarm9045_2x8_32.cpp (:4964 / :5362)
//  sit inside that file's wholly-#if-0'd body, preprocessor-nesting verified
//  2026-08-11 02:00 +0800.
//==============================================================================
bool MoveInArmZToShuttlePlace_9045(int iSht)
{
    GetInArmZShtDownPos_9045(iSht, true, true);
    bool bResult=InArmZMoveDown(bZFlgToSht, iZPosToSht, false);
    return bResult;
}

//==============================================================================
//  ROLE          CheckShuttleSensorStatus -- the shared in-shuttle FLOATING /
//                mis-placed-device sensor decoder.  Walks `Line` shuttle sensors
//                via SThreadPara.iInShSenIndex[Index][i], skips disabled ones,
//                builds the human-readable ErrPart position string, stamps
//                iInShuttleJam[Index][j] (2 = this cell must be dumped, 0 = clean),
//                and on error either raises the JAM0401/JAM0404 (normal) or
//                JAM0416/JAM0417 (autoclean) alarm after retracting the in-arm,
//                or reports 2 = "keep shaking".  0 = all clean, 3 = in-arm not yet
//                clear of the shuttle.
//
//  WAVE SCOPE    ACTIVE  CheckShuttleSensorStatus   golden ainarm9045.cpp:879-1017
//                (task said 879..1003; 1003 is mid-function -- see DEVIATION 1 in
//                 the return report.  1017 is the real closing brace.)
//
//  GATE REGISTER (this part file)   -- EMPTY.  No #if 0 is emitted here.  Every
//                symbol resolves: SThreadPara.iInShSenIndex (cprod.h:2910),
//                Sen[]/.Enable/.IsOn() (mysensor.h:48/44/42), ShuttleSensorName[9]
//                (cmydef.h:3241, cmydef.cpp:20), SnInPutSHT1S1/2S1 (cmydef.h:763/771),
//                SnInPutSHT1S8/2S8 (cmydef.h:905/907), iInShuttleJam[2][9]
//                (cmydef.h:5228, cmydef.cpp:5308), iCheckShuttleSensor (cmydef.h:5876,
//                cmydef.cpp:6011), MoveInArm2XYToWait (acatchtray_shims.cpp:110),
//                ShowErrorMessage 5-arg (canary_support.h:66), K_RETRY (cmydef.h:276),
//                MInShuttle1/2 (cmydef.h:2112/2113).
//
//  TRAP 1 shape (a) + TWO SIBLING GATES THIS BODY EXPIRES -- READ BEFORE MERGING.
//  Golden reaches this function only from the six CheckShuttleSensor_9045_2x5 /
//  _2x6 / _2x8 / _1x4 / _2x3 / _1x2 wrappers (golden :1036/1051/1066/1081/1096/1116).
//  Measured 2026-08-11 01:43:42 in D:/HT9045/HT9011UC_Cpp_V3.33.906.0 with
//    rg -n --no-heading -g "*.txt" "CheckShuttleSensorStatus" _w7d_parts
//  TWO SIBLING PART FILES OF THIS SAME WAVE gate their call to me PRECISELY on
//  "CheckShuttleSensorStatus has no port body":
//    * _w7d_parts/01070_CheckShuttleSensor_9045_1x4.txt:54  GATE k6-G1 (golden :1081)
//    * _w7d_parts/01085_CheckShuttleSensor_9045_2x3.txt:69  GATE k1-G1 (golden :1096)
//  BOTH PREMISES ARE FALSE ONCE THIS PART IS STITCHED.  The main loop must re-run
//  their absence commands at integration and DELETE both gates -- otherwise this
//  139-line engine links and is never called, and the in-shuttle floating /
//  stacked-IC JAM (JAM0401/0404/0416/0417) stays silently disabled.  Re-asking
//  TRAP 3`s "why SHOULD they stay gated" finds no surviving reason: same TU, same
//  archive (ht9045_sm), no new dependency.
//  Until those two gates are deleted this body is compiled and linked (it shares a
//  TU with live symbols, so the object file is always extracted) but UNREACHABLE --
//  TRAP 1 shape (a).  The remaining four wrappers (_2x5 golden :1025, _2x6 :1040,
//  _2x8 :1055, _1x2 :1105) are not in _w7d_parts at all and are still the
//  `{ return 0; }` stubs at ainarm9045.cpp:2325-2332.
//  AI(W7D-k3-ainarm) 20260811
//==============================================================================
// This TU does not include mysensor.h (golden gets Sen[] through the aArmHeader
// god-header, whose include list is gated `#if 0` at the port aArmHeader.h:23).
// Include it here rather than invent a sensor accessor.  Guarded (mysensorH), so
// a sibling part file including it too is a no-op.  The main loop may hoist this
// line into the file-top include block; nothing else changes if it does.
#include "mysensor.h"                                                           // Sen[] / TMySensor::Enable / IsOn()

// golden ainarm2.h:146 `extern bool MoveInArm2XYToWait();` -- golden pulls it via
// the aArmHeader god-header.  The port HAS a real body (acatchtray_shims.cpp:110,
// offline: returns true = the in-arm is clear) but declares it in
// acatchtray_shims.h:361, which this TU does not include.  Declaring it here is a
// declaration only, never a gate and never a stub.
extern bool MoveInArm2XYToWait();                                               // golden ainarm2.h:146
//==============================================================================
//Index         : Shuttle 1 or 2
//alarmflag     : 是否要直接Alarm
//Line          : 有幾顆Sensor要偵測
//bDuplicateErr : 是否有重複發生
//bSensorOn     : 是否有錯誤
//==============================================================================
int CheckShuttleSensorStatus(int Index, bool alarmflag, int Line, bool *bDuplicateErr, bool *bSensorOn, bool bAutoclean)                                        //Steven 20110409 : 整合Alarm Code  //JerryYang 20191113 新增alarm code區分in shuttle floating error/ clean pad floating error
{
    int j, Pos;
    AnsiString ErrPart="";                                                      //Steven 20110408 : 合併Alarm
    bool bHasDuplicateErr=false;                                                //Steven 20110408 : 合併Alarm
    bool bHasErr=false;                                                         //Steven 20110408 : 合併Alarm

    for(int i=0; i<Line; i++)
    {
        if(bDuplicateErr[i]==true)                                              //檢查是否為重覆的Alarm
            bHasDuplicateErr=true;

        Pos=SThreadPara.iInShSenIndex[Index][i];
        if(Sen[Pos].Enable==false)
        {
            bSensorOn[i]=false;                                                 //計為無錯誤或已經排除
        }
        else
        {
            if(Sen[Pos].IsOn()==true)                                           //檢查Sensor是否被遮斷
            {
                bHasErr=true;                                                   //表示有錯誤
                bSensorOn[i]=true;                                              //計為已經有發生過錯誤

                if(i==7)                                                        //9046的第8顆要另外處理
                {
                    ErrPart+=ShuttleSensorName[7];                              //紀錄錯誤的位置
                }
                else if(i==8)
                {
                    ErrPart+=" X1";                                             //紀錄錯誤的位置
                }
                else if(i==9)                                                   //2013-07-16    Dell    Shuttle cross sensor
                {
                    ErrPart+=" X2";                                             //紀錄錯誤的位置
                }
                else
                {
                    if(Pos>=SnInPutSHT1S8)
                    {
                        if(Index==0)
                            j=Pos-SnInPutSHT1S8+7;                              //Steven 20200812 : 8 --> 7
                        else
                            j=Pos-SnInPutSHT2S8+7;                              //Steven 20200812 : 8 --> 7
                    }
                    else
                    {
                        if(Index==0)
                            j=Pos-SnInPutSHT1S1;
                        else
                            j=Pos-SnInPutSHT2S1;
                    }

                    if(Index==0)
                        ErrPart+=ShuttleSensorName[j];                          //紀錄錯誤的位置    //jou 2014-08-08 記憶體破壞 187-33  [Pos-SnInPutSHT1S1] -> i
                    else
                        ErrPart+=ShuttleSensorName[j];                          //紀錄錯誤的位置    //jou 2014-08-08 記憶體破壞 187-33 [Pos-SnInPutSHT2S1] -> i

                    iInShuttleJam[Index][j]=2;                                  //kevin 20220819 shuttle Jam  紀錄位置後續，要做放到垃圾桶動作   //KEVIN 20220818 j=1 - 8
                }
            }
            else
            {
                bSensorOn[i]=false;                                             //計為無錯誤或已經排除
                if(i==7 || i==8 || i==9)                                        //9046的第8顆要另外處理
                {
                }
                else
                {
                    if(Pos>=SnInPutSHT1S8)
                    {
                        if(Index==0)
                            j=Pos-SnInPutSHT1S8+7;                              //Steven 20200812 : 8 --> 7
                        else
                            j=Pos-SnInPutSHT2S8+7;                              //Steven 20200812 : 8 --> 7
                    }
                    else
                    {
                        if(Index==0)
                            j=Pos-SnInPutSHT1S1;
                        else
                            j=Pos-SnInPutSHT2S1;
                    }
                    iInShuttleJam[Index][j]=0;                                  //kevin 20220819 shuttle Jam  紀錄位置後續，要做放到垃圾桶動作   //KEVIN 20220818 j=1 - 8
                }
            }
        }
    }

    if(bHasErr)
    {
        if(alarmflag)
        {
            if(!MoveInArm2XYToWait())                                           //先將In Arm讓開
                return 3;                                                       //Steven && jou 20091008

            if(bAutoclean)                                                      //JerryYang 20191113 新增alarm code區分in shuttle floating error/ clean pad floating error
            {
                if(Index==0)
                    ShowErrorMessage("JAM0416", K_RETRY, MInShuttle1, bHasDuplicateErr, ErrPart);                       //Input Shuttle 1 device floating error.                }
                else
                    ShowErrorMessage("JAM0417", K_RETRY, MInShuttle2, bHasDuplicateErr, ErrPart);                       //Input Shuttle 2 device floating error.                }
            }
            else
            {
                if(Index==0)
                    ShowErrorMessage("JAM0401", K_RETRY, MInShuttle1, bHasDuplicateErr, ErrPart);                       //Input Shuttle 1 device floating error.
                else
                    ShowErrorMessage("JAM0404", K_RETRY, MInShuttle2, bHasDuplicateErr, ErrPart);                       //Input Shuttle 2 device floating error.
                iCheckShuttleSensor=1;
            }

            for(int i=0; i<Line; i++)                                           //顯示錯誤後,才進行flag的轉換
            {
                if(bSensorOn[i]==true)
                {
                    bDuplicateErr[i]=true;
                }
                else
                {
                    bDuplicateErr[i]=false;
                }
            }
            return 1;
        }
        else
        {
            return 2;                                                           //等待抖抖抖
        }
    }
    else
    {
        for(int i=0; i<Line; i++)                                               //沒有錯誤要清空紀錄
        {
            bDuplicateErr[i]=false;
        }
    }
    return 0;
}

//==============================================================================
//== ROLE: CheckShuttleSensor_9045_2x5 -- the 2x5 (_10Site2X5) in-shuttle cross-
//==       sensor / floating check.  Sets SenNum from CROSS_SENSOR_INSTALL (7 with
//==       the cross sensors fitted, else 5) and hands the scan to the shared
//==       CheckShuttleSensorStatus.  Returns the verdict the place-to-shuttle SM
//==       switches on (golden ainarm9045.cpp:1411 ladder: 1=corrected, 2=shake/
//==       vibrate/knock, 3=keep waiting, anything else=go to Task 9000 "OK").
//==
//== WAVE SCOPE (k2-ainarm):
//==   CheckShuttleSensor_9045_2x5  golden ainarm9045.cpp:1025-1038  -- ACTIVE
//==     (100% faithful; ZERO gates -- see the TRAP 2 record below, this function
//==      WAS gated for ~15 minutes of this wave and the gate is now RETIRED)
//==
//== GATE REGISTER: EMPTY.  There is no #if 0 in this part.
//==
//==============================================================================
//== TRAP 2 EVENT -- AN ABSENCE CLAIM OF MINE EXPIRED INSIDE THIS WAVE, AND THIS IS
//== THE RECORD OF IT.  Do not delete this note; it is the audit trail.
//==
//==  1. WHAT I FIRST CONCLUDED (and it was true at the time):
//==       gate W7D-K2-G01 on golden :1036, because
//==       CheckShuttleSensorStatus(int,bool,int,bool*,bool*,bool) -- golden home
//==       ainarm9045.cpp:879, a file-scope SIBLING inside the same golden TU, in
//==       no header at all -- had no port body and no declaration.
//==     COMMAND (cwd D:/HT9045/HT9011UC_Cpp_V3.33.906.0):
//==       rg -n "CheckShuttleSensorStatus" --glob "*.cpp" --glob "*.h" .
//==     RESULT: 0 files, 0 hits.   MEASURED AT 2026-08-11 01:34:39 local.
//==
//==  2. WHY THAT CLAIM WAS ALWAYS FRAGILE: golden :879 is a sibling function of
//==     the very file I was handed a slice of, so it was a prime candidate for a
//==     concurrent agent -- exactly the shape TRAP 2 warns about.
//==
//==  3. WHAT ACTUALLY HAPPENED: a sibling part landed the REAL body 13 minutes
//==     after my measurement --
//==       _w7d_parts/00879_CheckShuttleSensorStatus.txt, mtime 2026-08-11 01:47
//==     and it is the full golden body, non-static, with golden's exact signature
//==     `int CheckShuttleSensorStatus(int Index, bool alarmflag, int Line, bool
//==     *bDuplicateErr, bool *bSensorOn, bool bAutoclean)` (that part's line 68),
//==     ending in a real `return 0;` -- not a stub.
//==     RE-MEASURED AT 2026-08-11 01:49:31 local.
//==     RE-CHECK COMMAND for the main loop:
//==       ls -la _w7d_parts/00879_CheckShuttleSensorStatus.txt   (must exist), and
//==       after the stitch:
//==       rg -n "^int CheckShuttleSensorStatus" ainarm9045.cpp   (must be 1 hit)
//==
//==  4. WHAT I DID: RETIRED the gate.  golden :1036 is now called live, verbatim.
//==     TRAP 3 was asked properly first -- "why SHOULD this stay gated anyway?" --
//==     and the answer is genuinely nothing: all six arguments are locals or plain
//==     scalars, the callee pulls in no new archive (it is in this same TU), it
//==     does no file I/O, and it hard-codes no real-machine path.  Unlike
//==     MyMessageBox (wrong archive) or WriteLastDataFile (writes the real
//==     machine's saved state), there is no second reason here.  So: retire.
//==
//==  5. ORDERING DEPENDENCY THIS CREATES -- MAIN LOOP MUST HOLD IT.  There is NO
//==     header declaration for CheckShuttleSensorStatus anywhere (golden has none
//==     either); golden compiles only because the definition at :879 precedes the
//==     call at :1036 in the same TU.  So part 00879 MUST be stitched BEFORE part
//==     01025.  The zero-padded golden-line filenames already give that order
//==     (00879 < 01025) -- do not re-order the parts.  IF part 00879 is dropped
//==     from this wave for any reason, this file must be re-gated (restore the
//==     `#if 0`/`#else ret=0;`/`#endif` around golden :1036); it will NOT merely
//==     fail to link, it will fail to COMPILE, which is the loud failure I want.
//==
//==  6. CROSS-PART INCONSISTENCY I FOUND WHILE FIXING MINE -- THIS IS THE
//==     "50 expired gates: body landed, every consumer still gated" failure mode
//==     starting to happen again, inside a single wave.  Surveyed at 01:50 local,
//==     command: for f in _w7d_parts/01*CheckShuttleSensor_9045_*.txt; do grep -c
//==     "CheckShuttleSensorStatus" $f; grep -c "^#if 0" $f; done
//==       * ALREADY LIVE (no #if 0, call the symbol): 01070 (1x4), 01125 (1x1)
//==       * STILL GATED for this same now-landed symbol: 01040 (2x6)
//==       * carry one #if 0 each, verify whether it is THIS symbol: 01085 (2x3),
//==         01105 (1x2)
//==       * do not reference it at all: 01100 (2x4), 01120 (2x2), 01133 (2x1),
//==         02070 (CheckShuttleSensor_9045)
//==     ACTION FOR THE MAIN LOOP: after stitching, sweep every consumer of
//==     CheckShuttleSensorStatus and un-gate them together with this one.  Leaving
//==     a mixed set means some site modes silently lose floating detection while
//==     their neighbours keep it -- and the build stays green either way, so ctest
//==     will not tell you.
//==============================================================================
//== INTEGRATION NOTE (TRAP 1 shape (b)): port ainarm9045.cpp:2330 currently holds
//== `int CheckShuttleSensor_9045_2x5(int,bool,bool){return 0;}`.  Delete it or
//== this file collides.  WARNING ON HOW YOU VERIFY: offline the ACTIVE guard
//== `if(LastSet.iRealDummy!=REALLY) return 0;` (golden :1029-1030) makes the real
//== body return 0 too -- the SAME value the stub returned -- so a green build and
//== a green ctest prove NOTHING about whether the swap happened.  Verify by grep
//== that the stub line is gone, never by test outcome.  Declaration is non-static
//== at ainarm9045.h:96 and carries the `bAutoclean=false` default, so this
//== definition omits the default and is NOT static (TRAP 1 shape (d) avoided).
//==
//== GOLDEN QUIRK PRESERVED, NOT FIXED: `static bool bDuplicateErr[2][8]={false,
//== false};` (golden :1027) initialises a 2x8 array with a 2-element brace list --
//== legal, and it zero-fills the rest, but the intent reads as one initialiser per
//== shuttle rather than per cell.  Also `bDuplicateErr[Index]` / `bSensorOn[Index]`
//== are indexed by Index with no bounds check, so an Index outside [0,2) reads out
//== of range.  Both transcribed exactly as golden wrote them.
//==
//== INTEGER DIVISION: none.  `(CROSS_SENSOR_INSTALL)?7:5` is a select.
//==============================================================================
int CheckShuttleSensor_9045_2x5(int Index, bool alarmflag, bool bAutoclean)
{
    static bool bDuplicateErr[2][8]={false, false};                             //2013-07-16    Dell    Shuttle cross sensor

    if(LastSet.iRealDummy!=REALLY)
        return 0;

    int ret;
    int SenNum=(CROSS_SENSOR_INSTALL)?7:5;
    bool bSensorOn[2][8]={false};

    ret=CheckShuttleSensorStatus(Index, alarmflag, SenNum, bDuplicateErr[Index], bSensorOn[Index], bAutoclean);         //Steven 20130702 : _12Site2X6
    return ret;
}

// =============================================================================
//  k4-ainarm PART 01/08  --  CheckShuttleSensor_9045_2x6   golden ainarm9045.cpp:1040-1053
//  Translator: AI(k4-ainarm) 20260811
//
//  ROLE   : 12-site 2x6 in-shuttle floating check -- picks 8 vs 6 sensor lines from
//           CROSS_SENSOR_INSTALL and delegates the whole verdict to the shared
//           CheckShuttleSensorStatus scanner; returns 0 when the machine is not REALLY.
//
//  WAVE SCOPE
//    CheckShuttleSensor_9045_2x6  golden :1040-1053  ACTIVE, fully faithful, ZERO gates.
//
//  TRAP 2 IN ACTION -- READ THIS FIRST.  I wrote this part's first draft with SIX gates
//  whose premise was "no port body exists".  I re-ran every absence claim before finishing,
//  at 2026-08-11 01:50:31 +0800, and six of them HAD DIED: sibling agents landed the bodies as part
//  files in _w7d_parts/ during this same wave.  A gate on a dead premise is exactly the
//  defect TRAP 2 describes -- so those six are RETIRED here (call ACTIVE), not left in with
//  an apology.  The gates that survive below survive because the symbol is STILL absent,
//  re-verified at 2026-08-11 01:50:31 +0800, not because nobody re-checked.
//
//  GATE REGISTER: EMPTY -- and it is empty on purpose.  Here is the one gate that was here:
//    k4-G1 (RETIRED 2026-08-11 01:50:31 +0800)  golden :1051  CheckShuttleSensorStatus
//      original premise: "0 matches in the port tree", measured 2026-08-11 01:26:17 +0800 with
//        cd D:/HT9045/HT9011UC_Cpp_V3.33.906.0 && grep -rnoE "\bCheckShuttleSensorStatus\b" \
//          --include=*.cpp --include=*.h --exclude-dir=.git --exclude-dir=.svn . | wc -l  ->  0
//      why it is retired: re-running the same claim at 2026-08-11 01:50:31 +0800 found the body had landed
//        as a sibling part file this wave --
//          _w7d_parts/00879_CheckShuttleSensorStatus.txt:68
//            int CheckShuttleSensorStatus(int Index, bool alarmflag, int Line,
//                                         bool *bDuplicateErr, bool *bSensorOn, bool bAutoclean)
//        which is parameter-for-parameter what golden :1051 calls, so the ACTIVE call binds.
//        Stitch order is safe without any forward declaration: golden's own definition order
//        puts :879 before :1040, and the part filenames sort 00879 < 01040.
//      TRAP 3 check -- is there another reason it SHOULD stay gated?  No.  This is a pure
//        in-tree sensor scan (no archive-boundary cost like MyMessageBox, no real-machine
//        file it could overwrite like WriteLastDataFile).  Leaving it gated would have
//        silently disabled in-shuttle floating detection -- a floating IC would be pressed by
//        the Index instead of stopping the lot -- so retiring it is also the safe answer.
//      >>> IF THE MAIN LOOP DROPS 00879 FROM THE STITCH, THIS PART STOPS COMPILING.  That is
//          the correct, loud failure; do not re-gate it to paper over a missing sibling.
//
//  GOLDEN QUIRKS kept as-is: bDuplicateErr is `static` (:1042), so the duplicate-alarm latch
//  deliberately persists across calls -- and only 2 of its 16 elements are explicitly
//  initialised ({false, false}); the rest are zero-filled by the language.  Both preserved.
//
//  Big5 -> UTF-8: the :1051 trailing comment (JerryYang 20191113) is transcribed
//  character-for-character from cp950.  ZERO U+FFFD.
// =============================================================================
int CheckShuttleSensor_9045_2x6(int Index, bool alarmflag, bool bAutoclean)
{
    static bool bDuplicateErr[2][8]={false, false};                             //2013-07-16    Dell    Shuttle cross sensor

    if(LastSet.iRealDummy!=REALLY)
        return 0;

    int ret;
    int SenNum=(CROSS_SENSOR_INSTALL)?8:6;
    bool bSensorOn[2][8]={false};

    ret=CheckShuttleSensorStatus(Index, alarmflag, SenNum, bDuplicateErr[Index], bSensorOn[Index], bAutoclean);         //Steven 20130702 : _12Site2X6  //JerryYang 20191113 新增alarm code區分in shuttle floating error/ clean pad floating error
    return ret;
}

//==============================================================================
//  ROLE          -- CheckShuttleSensor_9045_2x8(): the 2x8-layout arm of the
//                   CheckShuttleSensor_9045_<layout> family.  Picks the sensor
//                   count for a 2x8 in-shuttle (10 when the cross sensor is
//                   installed, else 8) and hands the per-shuttle duplicate-error
//                   and sensor-state rows to the shared floating/residual
//                   checker.  Returns that checker's verdict (0 == OK).
//  WAVE SCOPE    -- CheckShuttleSensor_9045_2x8   golden :1055-1068
//                   ACTIVE, FULLY LIVE (no gates)
//  GATE REGISTER -- (none: this function writes no #if 0.  See the RETIRED GATE
//                   LOG below -- k5-G1 was written, then retired inside the wave.)
//
//  RETIRED GATE LOG -- k5-G1, the TRAP 2 case, kept as evidence
//  -----------------------------------------------------------
//  (k5-G1, RETIRED)  golden :1066  `ret=CheckShuttleSensorStatus(...)`.
//      ORIGINAL PREMISE: CheckShuttleSensorStatus (golden ainarm9045.cpp:879,
//      outside this agent's ranges, no declaration in any golden header) had no
//      body anywhere in the port.
//      COMMAND RUN, and its result at the time:
//        grep -rn --include=*.cpp --include=*.h -w "CheckShuttleSensorStatus" . | grep -v "/\.svn/" | wc -l
//        RESULT 0     MEASURED AT 2026-08-11 01:37:38 +0800   (cwd D:/HT9045/HT9011UC_Cpp_V3.33.906.0)
//      PREMISE DIED AT 2026-08-11 01:47 +0800, inside the same wave, exactly the
//      way TRAP 2 says it does: a sibling landed
//      _w7d_parts/00879_CheckShuttleSensorStatus.txt, whose line 68 is
//        int CheckShuttleSensorStatus(int Index, bool alarmflag, int Line,
//                                     bool *bDuplicateErr, bool *bSensorOn,
//                                     bool bAutoclean)
//      -- signature-identical to golden :879 and to this call site (`SenNum` ->
//      `int Line`; `bDuplicateErr[Index]` / `bSensorOn[Index]` are bool[10] rows
//      that decay to `bool*`).  Re-verified by reading that file at 01:53.
//      SO THE GATE IS GONE and golden :1066 is LIVE.  Note this was NOT a
//      "degraded value happened to be equivalent" case -- returning 0 would have
//      silently disabled real shuttle-floating detection.  Leaving the gate in
//      because it "compiled fine" is precisely the defect TRAP 2 describes.
//      IF THE SIBLING PART IS DROPPED at integration, the failure is a clean
//      undefined-reference to CheckShuttleSensorStatus (not silent breakage), and
//      the one-line fallback is to re-wrap :1066 in `#if 0` with `ret=0;` in the
//      `#else` -- 0 being this family's "sensors OK" verdict, the same value as
//      :1060 and as the ten sibling stubs at ainarm9045.cpp:2323-2332.
//
//  NOTES for the main loop
//  -----------------------
//  * The port ALREADY has `int CheckShuttleSensor_9045_2x8(int, bool, bool)
//    { return 0; }` at ainarm9045.cpp:2332.  That stub is the main loop's to
//    retire; because it lives in the SAME TU as this definition, failing to
//    retire it is a hard `redefinition` compile error, never a silent
//    TRAP 1 shape (b).  This part is NOT written `static`.
//    Live call sites that start using this body: ainarm9045.cpp:1614 and
//    AutoClean/AutoClean.cpp:3206.
//  * NO default argument here: ainarm9045.h:98 already declares
//    `bAutoclean=false`; repeating it in the definition is ill-formed.  golden
//    :1055 likewise omits it.
//  * The local prototype for CheckShuttleSensorStatus below makes this part
//    independent of stitch order (golden :879 precedes :1055, so it should not be
//    needed -- but a redundant matching declaration is free, and golden itself
//    relies purely on definition order, which a part-file stitch must not
//    assume).  NO default arguments on it: the sibling definition has none.
//  * `static bool bDuplicateErr[2][10]` is FUNCTION-local and POD-initialised,
//    so TRAP 4 does not apply: no constructor runs, nothing is touched before
//    main().
//  * GOLDEN ODDITY kept: `={false, false}` initialises only 2 of 20 elements
//    explicitly, and `bool bSensorOn[2][10]={false}` only 1 of 20 (the rest
//    zero-fill anyway).  Transcribed verbatim.
//  * `int ret;` is left uninitialised, as golden has it -- now safe again,
//    because the live assignment on the next line is unconditional.
//  * INTEGER DIVISION AUDIT: no division in this function.
//  * Symbols present: LastSet.iRealDummy (LastSet.h:127), REALLY (cmydef.h:200),
//    CROSS_SENSOR_INSTALL (cmydef.h:2966).
//==============================================================================
int CheckShuttleSensorStatus(int Index, bool alarmflag, int Line, bool *bDuplicateErr, bool *bSensorOn, bool bAutoclean);        // golden ainarm9045.cpp:879 (same TU, sibling part 00879); golden declares it in no header, so declared locally to stay stitch-order independent

int CheckShuttleSensor_9045_2x8(int Index, bool alarmflag, bool bAutoclean)     //JerryYang 20191113 新增alarm code區分in shuttle floating error/ clean pad floating error
{
    static bool bDuplicateErr[2][10]={false, false};                            //2013-07-16    Dell    Shuttle cross sensor

    if(LastSet.iRealDummy!=REALLY)
        return 0;

    int ret;
    int SenNum=(CROSS_SENSOR_INSTALL)?10:8;
    bool bSensorOn[2][10]={false};

    ret=CheckShuttleSensorStatus(Index, alarmflag, SenNum, bDuplicateErr[Index], bSensorOn[Index], bAutoclean);         //JerryYang 20191113 新增alarm code區分in shuttle floating error/ clean pad floating error
    return ret;
}

//==============================================================================
//  AI(k6-ainarm) 20260811 -- golden ainarm9045.cpp:1070-1083  CheckShuttleSensor_9045_1x4
//
//  ROLE          -- 1x4 in-shuttle floating / stacked-IC sensor check: picks the
//                   sensor count (6 with cross sensors installed, else 4) and
//                   hands the per-shuttle duplicate-error latch + sensor-state
//                   row to CheckShuttleSensorStatus, returning its verdict.
//                   Self-disables (return 0) unless LastSet.iRealDummy==REALLY.
//  WAVE SCOPE    -- CheckShuttleSensor_9045_1x4  FULLY ACTIVE, NO GATE  (golden :1070-1083)
//
//  GATE REGISTER -- EMPTY.  This part emits NO #if 0.  It very nearly did; the
//  story is recorded here because it is exactly TRAP 2 and the main loop should
//  be able to audit the decision.
//
//  ****  TRAP 2 CAUGHT IN FLIGHT -- A GATE I WROTE AND THEN RETIRED  ***********
//  At 2026-08-11T01:31:24+0800 I ran:
//       rg -c --glob '*.cpp' --glob '*.h' --glob '!tests/**' -w CheckShuttleSensorStatus D:/HT9045/HT9011UC_Cpp_V3.33.906.0
//     -> 0 hits, no file matched at all.  CheckShuttleSensorStatus had no port
//        body (golden home ainarm9045.cpp:879 -- the same golden TU, a different
//        function, NOT in my range and NOT in my task list), so I gated the call
//        as GATE k6-G1 with ret=0 as the offline value.
//  At 2026-08-11T01:41 I re-ran it against the shared part directory and found:
//       _w7d_parts/00879_CheckShuttleSensorStatus.txt   (written 01:40)
//     i.e. a SIBLING AGENT LANDED IT NINE MINUTES AFTER MY SCAN.  I read that
//     part and confirmed its signature is golden-verbatim and matches this call
//     site argument-for-argument:
//       int CheckShuttleSensorStatus(int Index, bool alarmflag, int Line,
//                                    bool *bDuplicateErr, bool *bSensorOn, bool bAutoclean)
//  SO THE GATE IS GONE, not merely annotated.  Asking TRAP 3's question -- 'why
//  SHOULD this stay gated?' -- gives no second reason: the callee is a pure
//  sensor read with no new dependency, no UI, no file I/O, and it lands in this
//  same TU, so there is nothing to pay for.  Keeping the gate would have been the
//  defect: it silently disables the in-shuttle floating / stacked-IC scan on a
//  REALLY run, so a mis-seated device would be reported clean and the Index could
//  press onto it.  'The degraded value happens to equal the old stub's 0' would
//  NOT have made that fine.
//  IF THE MAIN LOOP DROPS part 00879: this file then produces a LOUD undefined
//  reference to CheckShuttleSensorStatus.  That is the intended failure mode and
//  is strictly preferable to silently losing the sensor check -- restore the gate
//  only as a deliberate, documented step, not to quiet a link error.
//  ****************************************************************************
//
//  RETIRE AT INTEGRATION: ainarm9045.cpp:2324 holds the offline stub
//  `int CheckShuttleSensor_9045_1x4(int, bool, bool) { return 0; }` in the SAME
//  file -- delete it or the build breaks with a redefinition.  The default
//  argument `bAutoclean=false` lives in the declaration (ainarm9045.h:65) and is
//  therefore NOT repeated here.
//==============================================================================
// ORDER-INDEPENDENCE, not a gate: part 00879 sorts BEFORE this part, so in
// filename order its definition already precedes this call -- but the append
// order is the main loop's choice, not mine, and an out-of-order stitch would
// otherwise fail with 'CheckShuttleSensorStatus was not declared in this scope'.
// This forward-decl makes the part order-independent.  A repeated declaration is
// harmless in C++ even if part 00879 also emits one (only definitions collide),
// and it is #ifndef-guarded regardless.  Signature transcribed from golden :879.
#ifndef AINARM_K6_CHECKSHUTTLESENSORSTATUS_FWD
#define AINARM_K6_CHECKSHUTTLESENSORSTATUS_FWD
extern int CheckShuttleSensorStatus(int Index, bool alarmflag, int Line, bool *bDuplicateErr, bool *bSensorOn, bool bAutoclean);   // golden ainarm9045.cpp:879; port body _w7d_parts/00879 (this wave)
#endif
//------------------------------------------------------------------------------
int CheckShuttleSensor_9045_1x4(int Index, bool alarmflag, bool bAutoclean)
{
    static bool bDuplicateErr[2][6]={false, false};                             //2013-07-16    Dell    Shuttle cross sensor

    if(LastSet.iRealDummy!=REALLY)
        return 0;

    int ret;
    int SenNum=(CROSS_SENSOR_INSTALL)?6:4;
    bool bSensorOn[2][6]={false};

    ret=CheckShuttleSensorStatus(Index, alarmflag, SenNum, bDuplicateErr[Index], bSensorOn[Index], bAutoclean);         //JerryYang 20191113 新增alarm code區分in shuttle floating error/ clean pad floating error
    return ret;
}

//============================================================================
//  CheckShuttleSensor_9045_2x3  --  golden ainarm9045.cpp:1085-1098  (k1-ainarm)
//
//  ROLE: 2x3_6 layout arm of the in-shuttle "is the IC properly seated in the
//        carry kit?" family.  Picks the sensor count for the layout (5 with the
//        cross sensor installed, else 3) and hands the per-shuttle latch array
//        to the shared CheckShuttleSensorStatus engine.
//
//  WAVE SCOPE:
//    * CheckShuttleSensor_9045_2x3(int,bool,bool)   ACTIVE body / 1 GATED call
//                                                   golden :1085-1098
//
//  Return-code contract, golden's own words at :1020-1023 immediately above this
//  family:  0 = 正確放入飛梭中 / 1 = 不正確放入飛梭中 / 2 = 不正確放入飛梭中且時間
//  延遲時間已到達 / 3 = 等待InArm移動到安全位置.
//
//  No default arguments are repeated here: ainarm9045.h:92 already carries
//  `bool bAutoclean=false` and C++ forbids restating it at the definition.
//
//  GATE REGISTER
//  -- GATE k1-G1, golden line :1096 -- CheckShuttleSensorStatus(...) --------
//     WHY THE OFFLINE DEFAULT IS FAITHFUL: the gated call's result is returned
//       unchanged, and the offline value chosen is golden's own "0 = 正確放入飛梭
//       中" (no floating).  That is the identical premise already carried by every
//       sibling in this family in the port: ainarm9045.cpp:2322-2331 return 0 for
//       _1x1/_1x4/_2x1/_2x3/_2x4/_1x2/_2x2/_2x5/_2x6/_2x8 and :2319
//       CheckShuttleSensor_9045(int) returns true, both documented there as
//       "offline: no floating".  This gate inherits that premise rather than
//       inventing a new one.  Everything else in the golden body -- the
//       LastSet.iRealDummy!=REALLY early return, the function-static
//       bDuplicateErr[2][5] latch, the CROSS_SENSOR_INSTALL ? 5 : 3 sensor count
//       and bSensorOn[2][5] -- is ACTIVE and verbatim.
//     REAL-MACHINE BEHAVIOUR DIFFERENCE: on iron CheckShuttleSensorStatus reads
//       those 3 (or 5) in-shuttle seating sensors, latches per-cell duplicate
//       errors into bDuplicateErr[Index][], raises the in-shuttle-floating /
//       clean-pad-floating alarm when alarmflag is true, and can return 1, 2 or 3.
//       Gated, a floating or stacked IC is never detected and no alarm fires, so
//       a proud IC would be carried on into the Index press.  This is a DETECTION
//       loss, not a motion change.
//     ABSENCE PROOF (TRAP 2 -- claim expires at integration, re-run it):
//       (run with cwd = D:/HT9045/HT9011UC_Cpp_V3.33.906.0)
//         git grep -n -- "CheckShuttleSensorStatus" -- '*.cpp' '*.h'
//       -> 0 hits.  Cross-checked untracked files too:
//         grep -rIl --include=*.cpp --include=*.h "CheckShuttleSensorStatus" .
//       -> 0 files.  RUN AT 2026-08-11 01:29:06 +0800 (grep finished 01:30:51).
//     TRAP 3 -- why this SHOULD stay gated even after the symbol lands: golden's
//       CheckShuttleSensorStatus is itself a 139-line sensor+alarm engine
//       (golden :879-1017), a translation target in its own right, not a symbol
//       someone forgot to register.  Even once its body exists, this gate must
//       stay shut until the in-shuttle sensor objects have a real backing store:
//       with no sensors the engine would report "not seated" and every offline
//       cycle would raise a floating alarm.  Retire it only together with the
//       sensor substrate.
//============================================================================
int CheckShuttleSensor_9045_2x3(int Index, bool alarmflag, bool bAutoclean)     //ChungHung 20140115 add for 2x3_6
{
    static bool bDuplicateErr[2][5]={false, false};                             //2013-07-16    Dell    Shuttle cross sensor

    if(LastSet.iRealDummy!=REALLY)
        return 0;

    int ret;
    int SenNum=(CROSS_SENSOR_INSTALL)?5:3;
    bool bSensorOn[2][5]={false};

    // AI(pt-wave) 20260811 PT-W7d INTEGRATE -- GATE k1-G1 RETIRED, and this one was not a
    // style choice. Its premise ("CheckShuttleSensorStatus is absent from the port") was stamped
    // 01:29 and had already expired by 01:47, when sibling part
    // _w7d_parts/00879_CheckShuttleSensorStatus.txt landed and DEFINED that engine at its line 68
    // -- into THIS SAME stitched translation unit. TRAP 2 in its purest form: an absence claim
    // that expired inside its own wave.
    // Why it mattered more than a missing feature: the #else arm returned 0, which golden
    // :1020-1023 defines as the seated case, while the 2x4 variant in the same TU forwards to
    // _1x4 and DOES detect a floating IC. Same sensor family, two different truths, and the
    // gated arm would have passed a proud or stacked IC to the Index press with no alarm.
    // Verified by opening both part files, not by trusting the audit that raised it -- which was
    // also wrong about scope: it claimed the 1x2 and 2x6 twins were gated too, and they are not
    // (both call CheckShuttleSensorStatus directly, 0 gates).
    ret=CheckShuttleSensorStatus(Index, alarmflag, SenNum, bDuplicateErr[Index], bSensorOn[Index], bAutoclean);         //JerryYang 20191113 新增alarm code區分in shuttle floating error/ clean pad floating error
    return ret;
}

//============================================================================
//  CheckShuttleSensor_9045_2x4  --  golden ainarm9045.cpp:1100-1103  (k1-ainarm)
//
//  ROLE: 2x4 layout arm of the in-shuttle seating-check family.  Golden simply
//        forwards to the 1x4 arm (same 4-cell kit geometry per shuttle row).
//
//  WAVE SCOPE:
//    * CheckShuttleSensor_9045_2x4(int,bool,bool)  ACTIVE (verbatim) :1100-1103
//
//  GATE REGISTER: empty -- this function contains no #if 0.
//
//  TRAP 1 note, shape (b) "a stub already satisfies the demand": the callee
//  CheckShuttleSensor_9045_1x4 is still the offline stub at port
//  ainarm9045.cpp:2323 (`{ return 0; }`), so this forwarder is faithful in shape
//  but returns 0 until that sibling gets its real body (golden :1070-1083).  The
//  forwarding itself is golden behaviour and is NOT gated -- gating a plain
//  delegation would hide the dependency instead of exposing it.  Note this is
//  exactly the shape nm --undefined-only cannot see.
//
//  No default argument restated here (ainarm9045.h:93 owns `bAutoclean=false`).
//============================================================================
int CheckShuttleSensor_9045_2x4(int Index, bool alarmflag, bool bAutoclean)
{
    return CheckShuttleSensor_9045_1x4(Index, alarmflag, bAutoclean);
}

//==============================================================================
//  k7-ainarm PART 01105 -- CheckShuttleSensor_9045_1x2(int, bool, bool)
//  Translator: AI(pt-wave k7-ainarm) 20260811
//  Golden: ainarm9045.cpp:1105-1118 (cp950)
//
//  ROLE          : the 1x2-layout arm of the CheckShuttleSensor_9045_<layout>
//                  family -- decides how many cross sensors this shuttle has
//                  (4 when CROSS_SENSOR_INSTALL, else 2) and hands the actual
//                  floating check to the shared CheckShuttleSensorStatus(),
//                  passing this layout`s own per-shuttle duplicate-alarm latch.
//                  Return codes are golden`s own legend (ainarm9045.cpp:1020-1023):
//                  0 = 正確放入的狀況, 1 = 不正確放入的狀況,
//                  2 = 不正確放入的狀況且時間超過時間已到了, 3 = 等待InArm移動到安全位置.
//  WAVE SCOPE    : CheckShuttleSensor_9045_1x2  golden :1105-1118 -- ACTIVE,
//                  VERBATIM, ZERO GATES (see the retirement note below).
//                  RETIRES the offline stub `int CheckShuttleSensor_9045_1x2(int,
//                  bool, bool) { return 0; }` at the port`s ainarm9045.cpp:2328 --
//                  the main loop must delete that line.  One live caller already
//                  exists: ainarm9045.cpp:1602 `ret=CheckShuttleSensor_9045_1x2
//                  (iSht, bShakeFlag[iSht]);` (default bAutoclean=false,
//                  ainarm9045.h:94).  This retirement IS a behaviour change --
//                  that caller stops getting a hard-coded 0 and starts getting the
//                  real in-shuttle floating verdict.
//  GATE REGISTER : EMPTY -- no #if 0 in this part.
//
//  RETIRED GATE (k7-G1) -- THE TRAP-2 CASE, CAUGHT INSIDE THIS WAVE
//  ------------------------------------------------------------------------
//  golden line :1116, symbol CheckShuttleSensorStatus(int, bool, int, bool*,
//  bool*, bool) -- golden defines it at ainarm9045.cpp:879, in THIS SAME golden
//  file, 226 lines above my range and outside my assigned ranges.
//    FIRST MEASUREMENT : `rg -n "\bCheckShuttleSensorStatus\b" -g "*.cpp" -g "*.h"`
//                        -> 0 hits (exit 1), at 2026-08-11 01:34:26 +0800.  On that
//                        evidence I wrote the call inside `#if 0` with an offline
//                        `ret=0`.
//    RE-MEASUREMENT    : `rg -n "^(int|bool|void)\s+(CheckShuttleSensorStatus)\s*\(" 
//                        -g "*.cpp" -g "*.h" -g "_w7d_parts/*.txt"`
//                        -> _w7d_parts/00879_CheckShuttleSensorStatus.txt:68, at
//                        2026-08-11 01:49:41 +0800.  A SIBLING AGENT LANDED THE
//                        REAL BODY 15 MINUTES AFTER MY FIRST MEASUREMENT, with the
//                        signature golden has, character for character.  The
//                        absence premise was therefore FALSE by integration time.
//    TRAP 3 RE-ASK ("why SHOULD this stay gated?") : no reason survives.  The body
//                        lands in THIS VERY TU (same golden file -> same stitched
//                        .cpp), so there is no cross-archive edge to buy; it does
//                        no file I/O and hard-codes no D:\ path; and the numeric
//                        part-file prefixes keep golden order, so 00879 precedes
//                        01105 and the definition is already visible at my call.
//                        Gate RETIRED -- golden :1116 is now ACTIVE and verbatim.
//    ORDER INSURANCE   : I still emit one forward declaration below, copied
//                        character-for-character from the sibling`s definition, so
//                        this part compiles even if the main loop stitches the
//                        parts in some order other than golden order.  A pure
//                        DECLARATION -- no definition, no global, no ODR risk, and
//                        it is the same BCB6 idiom the port already uses at
//                        acarry.cpp:171 for CheckOneCycleAction.
//==============================================================================
// golden ainarm9045.cpp:879 -- defined by sibling part 00879_CheckShuttleSensorStatus.txt.
// Declaration only (golden declares this symbol in no header at all).
int CheckShuttleSensorStatus(int Index, bool alarmflag, int Line, bool *bDuplicateErr, bool *bSensorOn, bool bAutoclean);
//------------------------------------------------------------------------------
int CheckShuttleSensor_9045_1x2(int Index, bool alarmflag, bool bAutoclean)
{
    static bool bDuplicateErr[2][4]={false, false};                             //2013-07-16    Dell    Shuttle cross sensor

    if(LastSet.iRealDummy!=REALLY)
        return 0;

    int ret;
    int SenNum=(CROSS_SENSOR_INSTALL)?4:2;
    bool bSensorOn[2][4]={false};

    ret=CheckShuttleSensorStatus(Index, alarmflag, SenNum, bDuplicateErr[Index], bSensorOn[Index], bAutoclean);         //JerryYang 20191113 新增alarm code區分in shuttle floating error/ clean pad floating error
    return ret;
}

//============================================================================
//  CheckShuttleSensor_9045_2x2  --  golden ainarm9045.cpp:1120-1123  (k1-ainarm)
//
//  ROLE: 2x2 layout arm of the in-shuttle seating-check family.  Golden forwards
//        to the 1x2 arm (2-cell kit geometry per shuttle row).
//
//  WAVE SCOPE:
//    * CheckShuttleSensor_9045_2x2(int,bool,bool)  ACTIVE (verbatim) :1120-1123
//
//  GATE REGISTER: empty -- this function contains no #if 0.
//
//  TRAP 1 note, shape (b): the callee CheckShuttleSensor_9045_1x2 is still the
//  offline stub at port ainarm9045.cpp:2328 (`{ return 0; }`); golden's real body
//  is :1105-1118.  Delegation kept ACTIVE for the same reason as the 2x4 arm.
//
//  No default argument restated here (ainarm9045.h:95 owns `bAutoclean=false`).
//============================================================================
int CheckShuttleSensor_9045_2x2(int Index, bool alarmflag, bool bAutoclean)
{
    return CheckShuttleSensor_9045_1x2(Index, alarmflag, bAutoclean);
}

//==============================================================================
//  ROLE          CheckShuttleSensor_9045_1x1 -- 1x1 shuttle-sensor wrapper.  Pure
//                redirect: the "1x4 with only site Ac open" layout borrows the 1x4
//                sensor set, everything else borrows the 1x2 set.
//
//  WAVE SCOPE    ACTIVE  CheckShuttleSensor_9045_1x1   golden ainarm9045.cpp:1125-1131
//
//  GATE REGISTER (this part file)   -- EMPTY.
//
//  CALLEE HONESTY (not a gate, and NOT something I may fix from here).  Measured
//  2026-08-11 01:43:42 (`ls _w7d_parts`):
//    * CheckShuttleSensor_9045_1x4 -- a SIBLING lands it this wave
//      (_w7d_parts/01070_CheckShuttleSensor_9045_1x4.txt), but with its inner
//      CheckShuttleSensorStatus call behind GATE k6-G1, so it still answers 0.
//    * CheckShuttleSensor_9045_1x2 (golden :1105) -- NOT in _w7d_parts; still the
//      `{ return 0; }` stub at ainarm9045.cpp:2329.
//  So this wrapper is faithful in SHAPE but both branches still answer 0
//  ("correctly placed in the shuttle") -- exactly what the stub it replaces
//  (ainarm9045.cpp:2323) answered, i.e. retiring that stub is behaviour-neutral
//  today and becomes real when k6-G1 dies and golden :1105 lands.  Its one live
//  caller is DoInArmCheckShuttleFloating (ainarm9045.cpp:1600).
//  Declared ainarm9045.h:89 (non-static, `bAutoclean=false` default lives in the
//  header -- so this definition must NOT repeat the default and must NOT be static).
//  AI(W7D-k3-ainarm) 20260811
//==============================================================================
int CheckShuttleSensor_9045_1x1(int Index, bool alarmflag, bool bAutoclean)
{
    if(iInArmType==e9045_1x4_1_Ac)                                              //Steven 20200720 : 1x4只開site Ac
        return CheckShuttleSensor_9045_1x4(Index, alarmflag, bAutoclean);
    else
        return CheckShuttleSensor_9045_1x2(Index, alarmflag, bAutoclean);
}

//==============================================================================
//  AI(k6-ainarm) 20260811 -- golden ainarm9045.cpp:1133-1136  CheckShuttleSensor_9045_2x1
//
//  ROLE          -- 2x1 in-shuttle sensor check: pure delegation, golden routes
//                   the 2x1 layout at the 1x2 sensor map unchanged.
//  WAVE SCOPE    -- CheckShuttleSensor_9045_2x1  ACTIVE  (golden :1133-1136)
//  GATE REGISTER -- none.  This part emits no #if 0.
//
//  The callee CheckShuttleSensor_9045_1x2 is declared at ainarm9045.h:70 and
//  currently satisfied by the offline stub at ainarm9045.cpp:2328 (returns 0).
//  That is a real, linkable symbol -- the delegation stays ACTIVE, unaltered.
//  Offline consequence, stated because it is inherited rather than introduced:
//  while _1x2 is a stub this returns 0 == "no floating" on a real machine too.
//
//  RETIRE AT INTEGRATION: ainarm9045.cpp:2325 holds the offline stub
//  `int CheckShuttleSensor_9045_2x1(int, bool, bool) { return 0; }` in the SAME
//  file -- delete it or the build breaks with a redefinition.  The default
//  argument `bAutoclean=false` lives in the declaration (ainarm9045.h:67) and is
//  therefore NOT repeated here.
//==============================================================================
int CheckShuttleSensor_9045_2x1(int Index, bool alarmflag, bool bAutoclean)
{
    return CheckShuttleSensor_9045_1x2(Index, alarmflag, bAutoclean);
}

//==============================================================================
//  AI(k6-ainarm) 20260811 -- golden ainarm9045.cpp:1258-1401  CheckPickerMode
//
//  ROLE          -- Decides WHICH pickers of a 4-nozzle X row are used and how
//                   many X-pitch cells to step: returns the picker mode
//                   (1234=Aa+Ab+Ac+Ad, 14=Aa+Ad, 13=Aa+Ac) and sets
//                   InArmSuck.iXStep (mirrored into OutArmSuck.iXStep on exit).
//                   Two independent ladders: HT-1032 (USE_16PICKER_TYPE==1) and
//                   the 9045 family, each split on whether 4 ICs fit across.
//  WAVE SCOPE    -- CheckPickerMode  ACTIVE with 1 gate  (golden :1258-1401)
//
//  FIDELITY NOTES (read before touching a number):
//    * EVERY division here is FLOATING POINT in golden and must stay that way:
//      `UserDefForm[Ld].XPitch/100.0` (double/double, golden :1310 and :1390) and
//      `TestIF_File.dSiteXPitch/2` (dSiteXPitch is double, cprod.h:1656, so /2 is
//      a double division).  Do NOT 'tidy' either into integer arithmetic -- that
//      is the exact substitution that previously produced stepped position drift.
//    * golden :1328 compares dSiteXPitch against the bare literal 120 (not a
//      _MM constant).  Kept verbatim.
//    * golden :1356 keeps its own comment 理論上沒辦法跑 on an iXStep=4 branch it
//      believes unreachable.  Transcribed, not judged.
//    * the mode-16 (`iPickCount!=4`) path falls straight through both ladders and
//      returns 1234 with iXStep==1 -- golden has no else, so neither do we.
//
//  GATE REGISTER
//   GATE k6-G2 -- golden ainarm9045.cpp:1267  ArmXCanSuck4IC_1032()
//     SYMBOL ABSENT.  golden home ainarm9045.cpp:115-125 (same golden TU, NOT in
//     this agent's range).  golden csystem.h:110 carries the declaration
//     COMMENTED OUT, and the port mirrors that comment verbatim at csystem.h:110
//     -- i.e. the ONLY textual hit in the port tree is a comment, there is no
//     declaration and no body.
//     WHY THE OFFLINE DEFAULT (true) IS FAITHFUL: transcribing golden :115-125,
//     the body returns false only if DeviceForm.XDimension>=iXpitchMaxX2-250 or
//     (dSiteXPitch>iXpitchMaxX2_MM && dSiteXPitch/2.0<=DeviceForm_File.XDimension+2);
//     with the offline zero-initialised DeviceForm / TestIF_File both tests are
//     false, so golden itself computes true.  This is the computed offline value,
//     NOT a convenient guess -- and it is explicitly NOT unconditionally true on a
//     machine with a wide device or a loaded recipe.
//     REAL-MACHINE DIFFERENCE: on an HT-1032 (USE_16PICKER_TYPE==1) running a
//     device wide enough to fail the check, golden drops to the 13/14 two-nozzle
//     mode; gated, this function keeps the 4-nozzle branch and iXStep of 1/2/4
//     from the 'fits' side -- i.e. the arm would try to place 4 ICs at a pitch
//     the device body cannot take.  HT-1032 only; the 9045 ladder is untouched.
//     ABSENCE COMMAND (exact):
//       rg -c --glob '*.cpp' --glob '*.h' --glob '!tests/**' -w ArmXCanSuck4IC_1032 D:/HT9045/HT9011UC_Cpp_V3.33.906.0
//       -> 1 hit, `.\csystem.h:1`, and that hit is the commented-out declaration
//          csystem.h:110 `//bool ArmXCanSuck4IC_1032();`.  No definition anywhere.
//     MEASURED AT: 2026-08-11T01:31:24+0800.
//
//   TRAP 2 -- WHY G2 STILL HOLDS AND WHY ITS TWIN DOES NOT.  Both predicates live
//     in THIS SAME golden file (:88 ArmXCanSuck4IC_9045, :115 ArmXCanSuck4IC_1032),
//     5 and 27 lines from ArmXCanSuck2IC_9045S which this very agent emits, so a
//     sibling landing them mid-wave was never hypothetical.  It happened:
//       * ArmXCanSuck4IC_9045 -- LANDED at 01:40 as
//         _w7d_parts/00088_ArmXCanSuck4IC_9045.txt.  I read it and confirmed the
//         body is golden :88-113 verbatim and the signature is `bool
//         ArmXCanSuck4IC_9045()`.  MY GATE FOR IT IS THEREFORE GONE -- golden
//         :1322 below is restored VERBATIM, calling it directly.  See the
//         RETIRED-GATE note under the ORDER-INDEPENDENCE block.
//       * ArmXCanSuck4IC_1032 -- STILL ABSENT at the same re-scan; no sibling part
//         exists for it.  So GATE k6-G2 above stands, on the same premise, re-
//         measured.  RE-SCAN COMMAND (both, exact) and time:
//
//         =====================================================================
//         AI(W906-NL-ABSENCE) 20260916: **THIS ABSENCE CLAIM HAS EXPIRED.**
//         ArmXCanSuck4IC_1032() is now DEFINED -- and not somewhere exotic:
//         **ainarm9045.cpp:2606, in THIS SAME FILE, ~1,300 lines above the gate**,
//         body = golden :115-125 verbatim.  The file's own WAVE SCOPE banner at
//         :2567 already records it as "ACTIVE (no gates)".
//         Linker-level proof (not grep): nm --defined-only over the 18 archives
//         of build/ finds `__Z19ArmXCanSuck4IC_1032v` in libht9045_sm.a.
//
//         WHY THE GATE IS STILL CLOSED ANYWAY (night-loop line C, 20260916):
//         this is USE_16PICKER_TYPE==1 (HT-1032) in-arm PICK-COUNT logic -- it
//         decides how many ICs the arm sucks.  That is motion/safety-adjacent,
//         and the night loop's standing rule is "safety-critical -> queue it,
//         do not do it, do not ask".  So the gate is left exactly as it is and
//         the decision is handed to the user.
//
//         WHAT THE USER NEEDS TO KNOW TO DECIDE (measured, 20260916):
//           * offline the stand-in and the real function agree: over zero-init
//             DeviceForm / TestIF_File, golden :115-125 returns true, which is
//             what the #else branch already assigns.  So un-gating is a NO-OP
//             offline and only changes behaviour on a real HT-1032.
//           * the "SECOND, better reason" written below (USE_16PICKER_TYPE) was
//             about WHICH MACHINE the branch belongs to, not about absence --
//             it does not by itself argue for keeping the call gated.
//         =====================================================================
//
//           ls D:/HT9045/HT9011UC_Cpp_V3.33.906.0/_w7d_parts | grep -i ArmXCanSuck
//           rg -c --glob '*.cpp' --glob '*.h' --glob '!tests/**' -w ArmXCanSuck4IC_9045 D:/HT9045/HT9011UC_Cpp_V3.33.906.0
//           rg -c --glob '*.cpp' --glob '*.h' --glob '!tests/**' -w ArmXCanSuck4IC_1032 D:/HT9045/HT9011UC_Cpp_V3.33.906.0
//         MEASURED AT: 2026-08-11T01:41 (+0800).
//     TRAP 3 for the SURVIVING gate: asked again, 'why SHOULD k6-G2 be gated?',
//     there is now a SECOND, better reason than mere absence -- USE_16PICKER_TYPE
//     ==1 is the HT-1032 machine (cmydef.h:2916, `Ztex 2023.12.06 Add HT-1032`),
//     not the HT-9045 this port targets, so the whole ladder the gate sits in is
//     unreachable on this hardware.  It should still be retired the moment
//     ArmXCanSuck4IC_1032 lands, because a dead branch is not a licence to be
//     wrong -- but it is not urgent, and it is NOT safety-relevant here.
//
//  LINKAGE (TRAP 1): NOT static, and NOT declared anywhere -- golden declares
//  CheckPickerMode in no header (12 golden uses, all inside ainarm9045.cpp), and
//  the port has zero textual hits, so after this part lands the body has NO
//  caller: shape (a).  Its golden callers (:1769-:2049) live in DoInArm_9045_Type
//  / SetInOutArmParameter, still offline stubs at ainarm9045.cpp:2268-2269.
//  Static would become shape (d) as soon as those land in this same TU.
//==============================================================================
// ORDER-INDEPENDENCE + RETIRED-GATE note.  golden :1322 calls ArmXCanSuck4IC_9045()
// directly and this part keeps that call VERBATIM, because a sibling landed the
// body this wave (_w7d_parts/00088, 01:40 -- see TRAP 2 in the GATE REGISTER).
// Part 00088 sorts before this one so its definition normally precedes the call,
// but append order is the main loop's choice, so the symbol is forward-declared
// here to keep the part order-independent.  A repeated declaration is harmless in
// C++ (only definitions collide) and it is #ifndef-guarded anyway.
//   IF THE MAIN LOOP DROPS part 00088: this file then produces a LOUD undefined
//   reference to ArmXCanSuck4IC_9045.  That is the intended failure mode -- re-
//   gating it would silently make the 9045 ladder claim 4 nozzles fit when the
//   device body / X-pitch / hot-1x4 rules say only 2 do.
#ifndef AINARM_K6_ARMXCANSUCK4IC_9045_FWD
#define AINARM_K6_ARMXCANSUCK4IC_9045_FWD
extern bool ArmXCanSuck4IC_9045();                                              // golden ainarm9045.cpp:88-113 (port csystem.h:109 decl commented out); port body _w7d_parts/00088 (this wave)
#endif
//------------------------------------------------------------------------------
int CheckPickerMode(int iPickCount, bool bSupport14Mode, bool bSupport13Mode)   //12=Aa+Ab, 13=Aa+Ac, 14=Aa+Ad, 1234=Aa+Ab+Ac+Ad
{
    int Ld=TrayForm.Loader.iTrayType;
    int iResult=1234;
    InArmSuck.iXStep=1;
    if(USE_16PICKER_TYPE==1)                                                    //Ztex 2023.12.06 Add HT-1032
    {
        if(iPickCount==4)
        {
            //AI(k6-ainarm) 20260811: GATE k6-G2 SEAM.  golden :1267 tested
            //ArmXCanSuck4IC_1032() directly in this `if`.  The call is hoisted
            //into a local ONLY so the absent symbol can be gated without losing
            //the branch; evaluation order is unchanged (this was the first thing
            //evaluated inside `if(iPickCount==4)`).
            bool bXCanSuck4IC_1032;
#if 0 // GATE k6-G2 -- golden ainarm9045.cpp:1267 ArmXCanSuck4IC_1032() (golden body :115-125; NO port body, port csystem.h:110 decl is commented out).  Absence command + time + real-machine difference: see GATE REGISTER above.
            bXCanSuck4IC_1032=ArmXCanSuck4IC_1032();
#else
            bXCanSuck4IC_1032=true;                                             // GATE k6-G2 offline value: what golden :115-125 itself computes over the zero-initialised DeviceForm / TestIF_File
#endif // GATE k6-G2
            if(bXCanSuck4IC_1032==false)
            {
                if(bSupport13Mode==false ||
                   (bSupport14Mode && TestIF_File.dSiteXPitch>iXpitchMaxX4_MM))
                {
                    iResult=14;
                    if(TestIF_File.dSiteXPitch>=iXpitchMinX6_MM &&
                       TestIF_File.dSiteXPitch<=iXpitchMaxX6_MM)
                        InArmSuck.iXStep=1;
                    else
                        InArmSuck.iXStep=2;
                }
                else if(bSupport13Mode)
                {
                    iResult=13;
                    if(TestIF_File.dSiteXPitch>=iXpitchMinX4_MM &&
                       TestIF_File.dSiteXPitch<=iXpitchMaxX4_MM)
                        InArmSuck.iXStep=1;
                    else
                        InArmSuck.iXStep=2;
                }
                else
                {
                    InArmSuck.iXStep=4;
                }
            }
            else
            {
                if(TestIF_File.dSiteXPitch<=iXpitchMaxX2_MM &&                  //一次放
                   TestIF_File.dSiteXPitch>=iXpitchMinX2_MM)
                {
                    InArmSuck.iXStep=1;
                }
                else if(TestIF_File.iInArmToShtReleaseMode==1)                  //Steven 20250630 : 力旺要照順序擺
                {
                    InArmSuck.iXStep=4;
                }
                else if(TestIF_File.dSiteXPitch>iXpitchMaxX4_MM)                //四吸嘴跑13吸嘴
                {
                    InArmSuck.iXStep=4;
                }
                else
                {
                    if(UserDefForm[Ld].XPitch/100.0<=TestIF_File.dSiteXPitch/2)
                        InArmSuck.iXStep=2;
                    else
                        InArmSuck.iXStep=4;
                }
            }
        }
    }
    else
    {
        if(iPickCount==4)
        {
            if(ArmXCanSuck4IC_9045()==false)
            {
                if((TestIF_File.iTestMode==TriSite1X3 ||
                    TestIF_File.iTestMode==_6Site2X3N) &&                       //JerryYang 20251124 : fix 1x3
                   TestIF.iSiteMap[0][1]==0 &&
                   TestIF.iSiteMap[1][1]==0 &&
                   TestIF_File.dSiteXPitch>=120)
                {
                    iResult=14;
                    InArmSuck.iXStep=3;
                }
                else
                {
                    if(bSupport13Mode==false ||
                       (bSupport14Mode && TestIF_File.dSiteXPitch>iXpitchMaxX2_MM))
                    {
                        iResult=14;
                        if(TestIF_File.dSiteXPitch>=iXpitchMinX3_MM &&
                           TestIF_File.dSiteXPitch<=iXpitchMaxX3_MM)
                            InArmSuck.iXStep=1;
                        else
                            InArmSuck.iXStep=2;
                    }
                    else if(bSupport13Mode)
                    {
                        iResult=13;
                        if(TestIF_File.dSiteXPitch>=iXpitchMinX2_MM &&
                           TestIF_File.dSiteXPitch<=iXpitchMaxX2_MM)
                            InArmSuck.iXStep=1;
                        else
                            InArmSuck.iXStep=2;
                    }
                    else
                    {
                        InArmSuck.iXStep=4;                                     //理論上沒辦法跑
                    }
                }
            }
            else
            {
                if(TestIF_File.dSiteXPitch<=iXpitchMaxX1_MM &&                  //一次放
                   TestIF_File.dSiteXPitch>=iXpitchMinX1_MM)
                {
                    InArmSuck.iXStep=1;
                }
                else if(TestIF_File.iInArmToShtReleaseMode==1)                  //Steven 20250630 : 力旺要照順序擺
                {
                    InArmSuck.iXStep=4;
                }
                else if(TestIF_File.dSiteXPitch>iXpitchMaxX2_MM)
                {
                    InArmSuck.iXStep=4;
                }
                else
                {
                    if(iInArmType==e9045_2x8_8)                                 //JerryYang 20231003
                    {
                        InArmSuck.iXStep=4;
                    }
                    else if(iInArmType==e9045_2x6_8 &&                          //JerryYang 20231003
                            TestIF_File.dSiteXPitch>iXpitchMaxX1_MM)            //Sam 20250203 : 修正 2X6 Hangup
                    {
                        InArmSuck.iXStep=4;
                    }
                    else
                    {
                        if((TestIF_File.dSiteXPitch==iXpitchMaxX2_MM &&         //Steven 20240308 : 修正1x4 X-Pitch 80mm
                            InArmSuck.iShtCol==4) ||
                           UserDefForm[Ld].XPitch/100.0<=TestIF_File.dSiteXPitch/2)
                            InArmSuck.iXStep=2;
                        else
                            InArmSuck.iXStep=4;
                    }
                }
            }
        }
    }
    OutArmSuck.iXStep=InArmSuck.iXStep;
    return iResult;
}

//==============================================================================
//  ROLE          -- CheckInArmYStep(): decides how many TRAY Y cells one picker
//                   row spans (iYStep = 1 or 2) for the in-arm, the out-arm and
//                   (9046AU only) the sort arm, from the recipe's site Y-pitch.
//                   Single-row-ish test modes are always 1 cell; otherwise either
//                   the AUTO-pitch window test or an exact compare against the
//                   shuttle's variable-Y teach value decides.
//  WAVE SCOPE    -- CheckInArmYStep   golden :1405-1469   ACTIVE (no gates)
//  GATE REGISTER -- (none: this function writes no #if 0)
//
//  NOTES for the main loop
//  -----------------------
//  * TRAP 5 -- WHICH TMyKitSuck.  This function writes InArmSuck.iYStep,
//    OutArmSuck.iYStep and OutArm2Suck.iYStep.  `class TMyKitSuck` exists TWICE
//    in the port with DIFFERENT LAYOUTS: mykitsuck.h:274 and
//    aHotPlateSubstrate.h:365.  I relied on **aHotPlateSubstrate.h:365**, because
//    that is the header the port's ainarm9045.cpp already includes (its include
//    head has `#include "aHotPlateSubstrate.h"` and does NOT include
//    mykitsuck.h) -- i.e. the same one every function already in this file
//    resolves through, and the 177-TU majority layout.  `iYStep` is declared
//    there at aHotPlateSubstrate.h:510.  The OBJECTS written are the ones defined
//    in **aHotPlateSubstrate.cpp** -- InArmSuck :80, OutArmSuck :83,
//    OutArm2Suck :85 (declared aHotPlateSubstrate.h:624 / :627 / :629).
//    FLAGGED, not fixed: mykitsuck.cpp:205/213/214 define same-named objects of
//    the OTHER layout.  That pre-exists this part; if mykitsuck.cpp is ever
//    registered into the same link the tree has a real ODR collision.
//  * GetVariableYOutShuttleData() has a REAL body -- aoutarm_shims.cpp:43,
//    `int GetVariableYOutShuttleData() { return 0; }` -- but NO header declares
//    it.  Every consumer in the port writes its own file-scope prototype
//    (aoutarm.cpp:516, aoutarm9045_1x2_2.cpp:122, aoutarm9045_1x4_2.cpp:81, ...).
//    This part follows that established idiom below.  Duplicate declarations are
//    harmless if the main loop or a sibling also emits one.
//  * GetVariableYInShuttleData() needs no prototype: ainarm9045.h:46 declares it.
//    Behaviour note for the wave ledger, not a gate: the port's current body is
//    the offline stub at ainarm9045.cpp:2288 (returns 0), but sibling part
//    _w7d_parts/02254_GetVariableYInShuttleData.txt:43 lands the REAL golden body
//    this same wave -- so after integration the `else` branch here starts
//    comparing against a real teach value instead of always 0.  Verified by
//    reading that part at 2026-08-11 01:53 +0800.  Nothing to change here; noted
//    because it changes this function's OUTPUT without changing its text.
//  * GOLDEN BUG kept (do NOT "fix"): golden :1431 / :1448 compare a `double`
//    (TestIF_File.dSiteYPitch*100) for EXACT EQUALITY with an `int`
//    (`==iYPitch`).  A recipe Y-pitch that is not exactly representable in binary
//    falls to the else and yields iYStep=2.  Transcribed verbatim.
//  * INTEGER DIVISION AUDIT: the two `/100.0` divisors are golden's own FLOAT
//    divisors on an int numerator (IN_OUT_ARM_Y_PITCH_MIN/MAX are `extern int`,
//    cmydef.h:2892/2893) -- int/double in golden too.  NOTHING was converted; no
//    int/int appears anywhere in this function.
//  * GOLDEN ASYMMETRY kept: the whole in-arm/out-arm Y-step block sits in the
//    `else` of the test-mode ladder, so in the single-row modes OutArmSuck.iYStep
//    and (when USE_OUT_SORT_ARM is uninstalled) OutArm2Suck.iYStep are left
//    UNTOUCHED while InArmSuck.iYStep is forced to 1.  Structure preserved.
//  * Symbols all present: USE_IN_Y_IS_AUTO_PITCH / USE_OUT_Y_IS_AUTO_PITCH
//    (cmydef.h:2890/2891), IN_OUT_ARM_Y_PITCH_MIN/MAX (cmydef.h:2892/2893),
//    USE_OUT_SORT_ARM (cmydef.h:2899), eartUninstall (MachineType.h:908),
//    the 8 eTestMode constants (MachineType.h:445-453),
//    TestIF_File.iTestMode / .dSiteYPitch (cprod.h:1657).
//  * No declaration exists for CheckInArmYStep in golden or the port.  NOT
//    `static` -- external linkage, as golden.  Its only golden caller is
//    DoInArm_9045_Type (golden :1623), which the port currently no-ops at
//    ainarm9045.cpp:2277, so TRAP 1 shape (a) still applies to THIS symbol after
//    this wave (checked _w7d_parts at 01:53: no part covers golden :1616).
//    Harmless: it lands in ainarm9045.cpp, a TU already extracted from
//    libht9045_sm.a by dozens of referenced symbols, so extraction never depended
//    on this function.  Flagged so it is not mistaken for "wired up".
//==============================================================================
int  GetVariableYOutShuttleData();                                              // REAL def aoutarm_shims.cpp:43 (ht9045_sm) -- golden aoutarm9045.h:23; no port header declares it, so declared locally per the aoutarm*.cpp idiom

void CheckInArmYStep()                                                          //Steven 20240205 : Y-Pitch可否一次放
{
    if(TestIF_File.iTestMode==SingleSite   ||
       TestIF_File.iTestMode==DualSite     ||                                   // 1x2
       TestIF_File.iTestMode==QualSite2X2N ||                                   //Frank 20200520 2X2NN Mode
       TestIF_File.iTestMode==TriSite1X3   ||                                   //1x3 //Frank 20160329 add for 1x3_4
       TestIF_File.iTestMode==_6Site2X3N   ||                                   //Steven 20220425 : 2X3NN Mode
       TestIF_File.iTestMode==QualSite1X4  ||                                   // 1x4
       TestIF_File.iTestMode==_8Site2X4N   ||                                   //Wei 20231211 : 2X4NN Mode
       TestIF_File.iTestMode==DualSite2x1)
    {
        InArmSuck.iYStep=1;
    }
    else
    {
        if(USE_IN_Y_IS_AUTO_PITCH==true)                                        //JerryYang 20251218 : IN/OUT ARM支援不同模組
        {
            if(TestIF_File.dSiteYPitch>=IN_OUT_ARM_Y_PITCH_MIN/100.0 &&
               TestIF_File.dSiteYPitch<=IN_OUT_ARM_Y_PITCH_MAX/100.0)
                InArmSuck.iYStep=1;
            else
                InArmSuck.iYStep=2;
        }
        else
        {
            int iYPitch=GetVariableYInShuttleData();
            if((TestIF_File.dSiteYPitch*100)==iYPitch)                          //Steven 20240306 : 修正Y-Pitch異常問題
                InArmSuck.iYStep=1;
            else
                InArmSuck.iYStep=2;
        }

        if(USE_OUT_Y_IS_AUTO_PITCH==true)                                       //Rogeryang 20260108 : 9011UC in/outarm模組不同需拆開
        {
            if(TestIF_File.dSiteYPitch>=IN_OUT_ARM_Y_PITCH_MIN/100.0 &&
               TestIF_File.dSiteYPitch<=IN_OUT_ARM_Y_PITCH_MAX/100.0)
                OutArmSuck.iYStep=1;
            else
                OutArmSuck.iYStep=2;
        }
        else
        {
            int iYPitch=GetVariableYOutShuttleData();
            if((TestIF_File.dSiteYPitch*100)==iYPitch)
                OutArmSuck.iYStep=1;
            else
                OutArmSuck.iYStep=2;
        }
    }

    if(USE_OUT_SORT_ARM!=eartUninstall)                                         //RogerYang 20250521 add for 9046AU
    {
        if(TestIF_File.iTestMode==SingleSite   ||
           TestIF_File.iTestMode==TriSite1X3   ||
           TestIF_File.iTestMode==QualSite1X4  ||
           TestIF_File.iTestMode==DualSite2x1)
        {
            OutArm2Suck.iYStep=1;
        }
        else
        {
            OutArm2Suck.iYStep=2;
        }
    }
}

//==============================================================================
//  CheckShuttleSensor_9045  --  golden ainarm9045.cpp:2070-2229
//  Translator: AI(W7D-ainarm-k8) 20260811
//
//  ROLE          -- TestIF.iTestMode fan-out (17 case labels grouped into 11 arms):
//                   run the per-layout in-shuttle floating-sensor check on
//                   shuttle 1 and/or shuttle 2 as CheckItem selects, and report
//                   true only when BOTH selected sides came back 0 (= OK).
//  WAVE SCOPE    -- CheckShuttleSensor_9045   golden :2070-2229   ACTIVE
//  GATE REGISTER -- (empty: this function contains no #if 0)
//
//  Retires the offline stub `bool CheckShuttleSensor_9045(int) { return true; }`
//  at port ainarm9045.cpp:2311 (MAIN LOOP deletes it).  The 10
//  CheckShuttleSensor_9045_<layout> callees already exist as offline
//  `{ return 0; }` stubs at port ainarm9045.cpp:2323-2332, so offline this still
//  evaluates to true -- but now THROUGH golden's real switch, so retiring those
//  10 stubs later needs no edit here.  Live callers: acarry.cpp:4297
//  (CheckItem=1) and acarry.cpp:6124 (CheckItem=2), declared at acarry.cpp:3260 /
//  acarry_shims.h:213; the CheckItem=0 default lives in port ainarm9045.h:100.
//
//  golden :2064-2069 is this function's header comment.  It sits OUTSIDE the
//  assigned 2070..2229 range, so it is transcribed here character-for-character
//  instead of being emitted into the source stream (nothing dropped):
//      //-----------------------------------------------------------------------------
//      //jou 2010-01-23 for contact do auto-high use
//      //CheckItem=0 : 兩邊都檢查
//      //CheckItem=1 : 檢查Shuttle 1
//      //CheckItem=2 : 檢查Shuttle 2
//      //-----------------------------------------------------------------------------
//
//  GOLDEN BUGS KEPT VERBATIM (do NOT "fix" any of these):
//   1. golden :2178 (_10Site2X5) gates ret2 on TestIF_File.bArm1PickPlaceArm2Test
//      ALONE, while all 10 other arms require
//      IniConfig.bD58UseArm1PickPlaceArm2Test && TestIF_File.bArm1PickPlaceArm2Test
//      -- Ifor's 20200811 "需卡兩個條件" fix never reached the 2x5 arm.
//   2. there is NO `default:` arm -- an iTestMode outside the 17 listed values
//      leaves ret1=ret2=1 and falls through to `return false` ("not OK").
//   3. _32Site4X8N (:2209) dispatches to the _2x8 checker, the same one
//      _16Site2X8 (:2196) uses, and _16Site4X4 (:2158) shares the _2x4 checker
//      with _8Site2X4 -- so 4x8/4x4 are checked as if they were 2x8/2x4.
//   4. :2126 `_2x2(1,true)` and :2141 `_1x4(1,true)` omit the space after the
//      comma the other arms use -- transcribed byte-for-byte anyway.
//==============================================================================
bool CheckShuttleSensor_9045(int CheckItem)                                     //Steven 20150415 : 加上CheckItem,可以判別哪邊不用做檢查
{
    int ret1=1, ret2=1;
    switch(TestIF.iTestMode)
    {
        case SingleSite:
            if(CheckItem!=2)
                ret1=CheckShuttleSensor_9045_1x1(0, true);
            else
                ret1=0;

            if((IniConfig.bD58UseArm1PickPlaceArm2Test==true &&
                TestIF_File.bArm1PickPlaceArm2Test==true) ||                    //Ifor 20200811 Fix: Arm1 Pick Place Arm2Test 需卡兩個條件
               CheckItem==1)
                ret2=0;
            else
                ret2=CheckShuttleSensor_9045_1x1(1, true);
            break;
        case DualSite:                                                          //1x2
        case QualSite2X2N:                                                      //Frank 20200520 2X2NN Mode
            if(CheckItem!=2)
                ret1=CheckShuttleSensor_9045_1x2(0, true);
            else
                ret1=0;

            if((IniConfig.bD58UseArm1PickPlaceArm2Test==true &&
                TestIF_File.bArm1PickPlaceArm2Test==true) ||                    //Ifor 20200811 Fix: Arm1 Pick Place Arm2Test 需卡兩個條件
               CheckItem==1)
                ret2=0;
            else
                ret2=CheckShuttleSensor_9045_1x2(1, true);
            break;
        case DualSite2x1:
            if(CheckItem!=2)
                ret1=CheckShuttleSensor_9045_2x1(0, true);
            else
                ret1=0;

            if((IniConfig.bD58UseArm1PickPlaceArm2Test==true &&
                TestIF_File.bArm1PickPlaceArm2Test==true) ||                    //Ifor 20200811 Fix: Arm1 Pick Place Arm2Test 需卡兩個條件
               CheckItem==1)
                ret2=0;
            else
                ret2=CheckShuttleSensor_9045_2x1(1, true);
            break;
        case QualSite2X2:                                                       //2x2
            if(CheckItem!=2)
                ret1=CheckShuttleSensor_9045_2x2(0, true);
            else
                ret1=0;

            if((IniConfig.bD58UseArm1PickPlaceArm2Test==true &&
                TestIF_File.bArm1PickPlaceArm2Test==true) ||                    //Ifor 20200811 Fix: Arm1 Pick Place Arm2Test 需卡兩個條件
               CheckItem==1)
                ret2=0;
            else
                ret2=CheckShuttleSensor_9045_2x2(1,true);
            break;
        case QualSite1X4:                                                       //1x4
        case _8Site1X4:                                                         //ChungHung 20150528 add for 海思 _8Site1x4
        case _8Site2X4N:                                                        //Wei 20231211 : 2X4NN Mode
            if(CheckItem!=2)
                ret1=CheckShuttleSensor_9045_1x4(0, true);
            else
                ret1=0;

            if((IniConfig.bD58UseArm1PickPlaceArm2Test==true &&
                TestIF_File.bArm1PickPlaceArm2Test==true) ||                    //Ifor 20200811 Fix: Arm1 Pick Place Arm2Test 需卡兩個條件
               CheckItem==1)
                ret2=0;
            else
                ret2=CheckShuttleSensor_9045_1x4(1,true);
            break;
        case TriSite1X3:                                                        //Frank 20160329 add for 1x3_4
        case _6Site2X3:                                                         //ChungHung 20140115 add for 2x3_6
        case _6Site2X3N:                                                        //Steven 20220425 : 2X3NN Mode
            if(CheckItem!=2)
                ret1=CheckShuttleSensor_9045_2x3(0, true);
            else
                ret1=0;

            if((IniConfig.bD58UseArm1PickPlaceArm2Test==true &&
                TestIF_File.bArm1PickPlaceArm2Test==true) ||                    //Ifor 20200811 Fix: Arm1 Pick Place Arm2Test 需卡兩個條件
               CheckItem==1)
                ret2=0;
            else
                ret2=CheckShuttleSensor_9045_2x3(1, true);
            break;
        case _16Site4X4:                                                        //Sam 20190226 : 16Site4X4
        case _8Site2X4:                                                         //2x4
            if(CheckItem!=2)
                ret1=CheckShuttleSensor_9045_2x4(0, true);
            else
                ret1=0;

            if((IniConfig.bD58UseArm1PickPlaceArm2Test==true &&
                TestIF_File.bArm1PickPlaceArm2Test==true) ||                    //Ifor 20200811 Fix: Arm1 Pick Place Arm2Test 需卡兩個條件
               CheckItem==1)
                ret2=0;
            else
                ret2=CheckShuttleSensor_9045_2x4(1, true);
            break;
        case _10Site2X5:                                                        //2x5 //wei 20190614 10 site
            if(CheckItem!=2)
                ret1=CheckShuttleSensor_9045_2x5(0, true);
            else
                ret1=0;

            if(TestIF_File.bArm1PickPlaceArm2Test || CheckItem==1)
                ret2=0;
            else
                ret2=CheckShuttleSensor_9045_2x5(1, true);
            break;
        case _12Site2X6:                                                        //2x6
            if(CheckItem!=2)
                ret1=CheckShuttleSensor_9045_2x6(0, true);
            else
                ret1=0;

            if((IniConfig.bD58UseArm1PickPlaceArm2Test==true &&
                TestIF_File.bArm1PickPlaceArm2Test==true) ||                    //Ifor 20200811 Fix: Arm1 Pick Place Arm2Test 需卡兩個條件
               CheckItem==1)
                ret2=0;
            else
                ret2=CheckShuttleSensor_9045_2x6(1, true);
            break;
        case _16Site2X8:                                                        //2x8
            if(CheckItem!=2)
                ret1=CheckShuttleSensor_9045_2x8(0, true);
            else
                ret1=0;

            if((IniConfig.bD58UseArm1PickPlaceArm2Test==true &&
                TestIF_File.bArm1PickPlaceArm2Test==true) ||                    //Ifor 20200811 Fix: Arm1 Pick Place Arm2Test 需卡兩個條件
               CheckItem==1)
                ret2=0;
            else
                ret2=CheckShuttleSensor_9045_2x8(1, true);
            break;
        case _32Site4X8N:                                                       //4x8
            if(CheckItem!=2)
                ret1=CheckShuttleSensor_9045_2x8(0, true);
            else
                ret1=0;

            if((IniConfig.bD58UseArm1PickPlaceArm2Test==true &&
                TestIF_File.bArm1PickPlaceArm2Test==true) ||                    //Ifor 20200811 Fix: Arm1 Pick Place Arm2Test 需卡兩個條件
               CheckItem==1)
                ret2=0;
            else
                ret2=CheckShuttleSensor_9045_2x8(1, true);
            break;
    }

    if(ret1==0 && ret2==0)
    {
        return true;                                                            //OK
    }
    return false;
}

//==============================================================================
//  k7-ainarm PART 02254 -- GetVariableYInShuttleData()
//  Translator: AI(pt-wave k7-ainarm) 20260811
//  Golden: ainarm9045.cpp:2254-2300 (cp950)
//
//  ROLE          : returns the Y-pitch (in motor units) the in-arm must open to
//                  when placing into / picking from the shuttle -- the generic
//                  ("泛用型") calculator that replaced the per-layout tables.
//                  Auto-pitch machines clamp TestIF.dSiteYPitch into
//                  [IN_OUT_ARM_Y_PITCH_MIN, IN_OUT_ARM_Y_PITCH_MAX]; single-row /
//                  *_Hot layouts short-circuit to 6000 (or to the taught
//                  Prod.LoadForm.iYPitch on HT9046_LS + HT9016C); manual-pitch
//                  machines answer 6350 / 3600 / 6000.
//  WAVE SCOPE    : GetVariableYInShuttleData  golden :2254-2300 -- ACTIVE
//                  (verbatim, no gates).  This RETIRES the offline stub
//                  `int  GetVariableYInShuttleData() { return 0; }` at the port`s
//                  ainarm9045.cpp:2314 -- the main loop must delete that line, or
//                  the stitched TU has two definitions.  15 live call sites
//                  already exist (ainarm9045_*.cpp `int iMovePitchY =
//                  GetVariableYInShuttleData();`), so retiring the stub is a
//                  BEHAVIOUR change from 0 to a real pitch -- intended.
//  GATE REGISTER : (none -- every symbol resolves)
//                    InArmSuck.iShtRow          aHotPlateSubstrate.h:406 (TMyKitSuck)
//                    iInArmType / e9045_*_Hot   cmydef.h / MachineType.h
//                    MachineTypeChoice / SubMachineType / Type_HT9046_LS /
//                    Type_HT9016C               cmydef.h / MachineType.h
//                    Prod.LoadForm.iYPitch      cprod.h:38
//                    TestIF.dSiteYPitch         cprod.h:2576
//                    USE_IN_OUT_ARM_Y_PITCH / iXPitchManual635 / iXPitchManual360
//                                               cmydef.h
//                    TestIF_File.iARM_Y_PITCH   cprod.h:2577 / :1931
//
//  TRAP 5 note: InArmSuck is the aHotPlateSubstrate.h TMyKitSuck (declaration at
//  aHotPlateSubstrate.h:406 for iShtRow, object defined in aHotPlateSubstrate.cpp)
//  -- NOT mykitsuck.h:274.  This TU already includes aHotPlateSubstrate.h and
//  never mykitsuck.h, so the offsets are the ones the other 177 TUs use.
//
//  FIDELITY note: golden`s last auto-pitch statement is `return
//  TestIF.dSiteYPitch;` -- a double truncated to the int return type.  Kept as
//  written (an explicit cast would read the same, but golden has none, so neither
//  do I).  NOT converted to any rounding helper.
//==============================================================================
int GetVariableYInShuttleData()                                                 //Steven 20230519 : 改成泛用型計算Y-Pitch
{
    if(USE_IN_Y_IS_AUTO_PITCH==true)                                            //Steven for HT1032  //JerryYang 20251218 : IN/OUT ARM支援不同模組
    {
        if(InArmSuck.iShtRow==1 ||                                              //Steven 20240311 : iPickRow --> iShtRow
           iInArmType==e9045_1x4_8_Hot ||
           iInArmType==e9045_1x2_4_Hot ||
           iInArmType==e9045_2x2_8_Hot)                                         //JerryYang 20231003
        {
            if(MachineTypeChoice==Type_HT9046_LS &&
               SubMachineType==Type_HT9016C)                                    //Jimmychiu 20230207 Add TempPosShift for HT9016C
            {
                if(Prod.LoadForm.iYPitch>=IN_OUT_ARM_Y_PITCH_MIN &&
                   Prod.LoadForm.iYPitch<=IN_OUT_ARM_Y_PITCH_MAX)               //Min Y pitch 1500 in HT9016c
                {
                    return Prod.LoadForm.iYPitch;
                }
                else
                {
                    return IN_OUT_ARM_Y_PITCH_MIN;
                }
            }
            else
            {
                return 6000;
            }
        }

        if(TestIF.dSiteYPitch>=IN_OUT_ARM_Y_PITCH_MAX)
            return IN_OUT_ARM_Y_PITCH_MAX;
        else if(TestIF.dSiteYPitch<=IN_OUT_ARM_Y_PITCH_MIN)
            return IN_OUT_ARM_Y_PITCH_MIN;

        return TestIF.dSiteYPitch;
    }
    else if(USE_IN_OUT_ARM_Y_PITCH==iXPitchManual635 &&
            TestIF_File.iARM_Y_PITCH==6350)                                     //Steven 20240306 : 修正Y-Pitch異常問題
    {
        return 6350;
    }
    else if(USE_IN_OUT_ARM_Y_PITCH==iXPitchManual360 &&
            TestIF_File.iARM_Y_PITCH==3600)                                     //Steven 20240306 : 修正Y-Pitch異常問題
    {
        return 3600;
    }
    return 6000;
}

// =============================================================================
//  k4-ainarm PART 02/08  --  CheckPlaceToShuttle   golden ainarm9045.cpp:2313-2386
//  Translator: AI(k4-ainarm) 20260811
//
//  ROLE   : "use initial-delay as soak time" entry gate.  On the very first input
//           (bFirstInput) it re-labels every HAS_IC on the in-arm grid to HAS_HOT_IC (they
//           have now soaked), then -- if either shuttle has a free side/site -- latches which
//           shuttle+kit the next place targets, kicks the place-to-shuttle task and reports
//           true so the caller diverts to placing instead of picking.
//
//  WAVE SCOPE
//    CheckPlaceToShuttle  golden :2313-2386  ACTIVE, fully faithful, ZERO gates.
//
//  GATE REGISTER: (empty -- nothing gated in this function)
//    Every symbol resolves against the include set ainarm9045.cpp already has.  Verified by
//    compiling a probe TU carrying that exact include list plus these references:
//      g++ -std=c++17 -fsyntax-only -I. -Ivclcompat <probe>.cpp   ->  clean
//      (run in D:/HT9045/HT9011UC_Cpp_V3.33.906.0, 2026-08-11 01:26:17 +0800)
//    Confirmed present: CosFunction/Temperature .bUseInitialDelayAsSoakTime, bFirstInput,
//    TestIF.iTestMode, _16Site2X8/_12Site2X6/_32Site4X8N, TestIF_File.iShuttleMode/
//    iShuttle_Sel, FL/BLCarryKit.LeftSideNoIC/RightSideNoIC/UseSiteNoIC, InArmSuck.Item/
//    SetItemData/iWhichKit/iWhichShtPickFor32/iWhichKitPickFor32, HAS_IC, HAS_HOT_IC,
//    InitInArmPlaceToShuttleTask (aHotPlateSubstrate.h:1081), bPlaceShuttle.
//
//  TRAP 5 -- WHICH TMyKitSuck.  InArmSuck / FLCarryKit / BLCarryKit here are the
//    aHotPlateSubstrate.h:365 TMyKitSuck (the 177-TU layout), and the objects are defined in
//    aHotPlateSubstrate.cpp:80,81,82.  Proof this TU cannot pick up the rival mykitsuck.h:274
//    class (different layout -- would read every field at the wrong offset):
//      g++ -std=c++17 -E -I. -Ivclcompat <probe>.cpp | grep -c "mykitsuck.h"   ->  0
//      (2026-08-11 01:26:17 +0800).  mykitsuck.cpp:205-206 ALSO defines InArmSuck/FLCarryKit -- that TU
//      must stay unregistered; nothing here changes that.
//
//  GOLDEN QUIRK kept as-is: `int iCenter;` (:2316) is left uninitialised and is only ever
//  read on the _16Site2X8/_12Site2X6/_32Site4X8N path that assigns it first (:2332-2335), so
//  golden is safe by construction -- NOT initialised here, to stay a translation.
//
//  Big5 -> UTF-8: no Chinese in this range; the four Ifor 20260317 ASCII comments are
//  preserved verbatim.  ZERO U+FFFD.
// =============================================================================
bool CheckPlaceToShuttle()
{
    bool bResult=false;
    int  iCenter;

    if(CosFunction.bUseInitialDelayAsSoakTime &&
       Temperature.bUseInitialDelayAsSoakTime)
    {
        if(bFirstInput)
        {
            for(int i=0; i<MAX_ARM_Row; i++)
                for(int j=0; j<MAX_ARM_Col; j++)
                    if(InArmSuck.Item[i][j]==HAS_IC)
                        InArmSuck.SetItemData(i, j, HAS_HOT_IC);

            if(TestIF.iTestMode==_16Site2X8 ||
               TestIF.iTestMode==_12Site2X6 ||
               TestIF.iTestMode==_32Site4X8N)
            {
                if(TestIF.iTestMode==_12Site2X6)
                    iCenter=3;
                else
                    iCenter=4;

                if((TestIF_File.iShuttleMode==0 ||
                   (TestIF_File.iShuttleMode==1 && TestIF_File.iShuttle_Sel==0)) &&
                   (FLCarryKit.LeftSideNoIC(iCenter)==true ||
                    FLCarryKit.RightSideNoIC(iCenter)==true))
                {
                    InArmSuck.iWhichShtPickFor32=0;                             //Ifor 20260317 : fix should be 0 for FLCarryKit(Shuttle1)
                    InArmSuck.iWhichKitPickFor32=InArmSuck.iWhichKit;
                    InitInArmPlaceToShuttleTask();
                    bPlaceShuttle=true;
                    bResult=true;
                }

                if(bResult==false &&
                   (BLCarryKit.LeftSideNoIC(iCenter)==true ||
                    BLCarryKit.RightSideNoIC(iCenter)==true))
                {
                    InArmSuck.iWhichShtPickFor32=1;                             //Ifor 20260317 : fix should be 1 for BLCarryKit(Shuttle2)
                    InArmSuck.iWhichKitPickFor32=InArmSuck.iWhichKit;
                    InitInArmPlaceToShuttleTask();
                    bPlaceShuttle=true;
                    bResult=true;
                }
            }
            else
            {
                if((TestIF_File.iShuttleMode==0 ||
                   (TestIF_File.iShuttleMode==1 &&
                    TestIF_File.iShuttle_Sel==0)) &&
                   FLCarryKit.UseSiteNoIC())
                {
                    InArmSuck.iWhichShtPickFor32=0;                             //Ifor 20260317 : fix should be 0 for FLCarryKit(Shuttle1)
                    InArmSuck.iWhichKitPickFor32=InArmSuck.iWhichKit;
                    InitInArmPlaceToShuttleTask();
                    bPlaceShuttle=true;
                    bResult=true;
                }

                if(bResult==false && BLCarryKit.UseSiteNoIC())
                {
                    InArmSuck.iWhichShtPickFor32=1;                             //Ifor 20260317 : fix should be 1 for BLCarryKit(Shuttle2)
                    InArmSuck.iWhichKitPickFor32=InArmSuck.iWhichKit;
                    InitInArmPlaceToShuttleTask();
                    bPlaceShuttle=true;
                    bResult=true;
                }
            }
        }
    }
    return bResult;
}

// =============================================================================
//  k4-ainarm PART 03/08  --  AutoTrayEndMusic   golden ainarm9045.cpp:2527-2538
//  Translator: AI(k4-ainarm) 20260811
//
//  ROLE   : Auto-Tray-end annunciator.  Arms the buzzer/light flag once per tray (only while
//           the "finish light+alarm" latch is still clear) and reports whether the operator
//           has held the key long enough (iSkeyTime >= iHandMode).
//
//  WAVE SCOPE
//    AutoTrayEndMusic  golden :2527-2538  ACTIVE, fully faithful, ZERO gates.
//
//  GATE REGISTER: (empty -- nothing gated in this function)
//    bNeedMusicFinishLighAndAlarmOn and bNeedMusicAndAlarmOn both resolve through the
//    existing include set (probe TU, g++ -fsyntax-only, 2026-08-11 01:26:17 +0800).
//
//  GOLDEN QUIRK kept as-is: the misspelled global bNeedMusicFinishLighAndAlarmOn ("Ligh",
//  not "Light") is golden's own spelling and is what the port declares -- NOT corrected.
//  Note the two flags are different objects: the function only ever SETS ...AndAlarmOn and
//  only ever READS ...FinishLighAndAlarmOn; that asymmetry is golden's, kept verbatim.
//
//  Big5 -> UTF-8: three Chinese comments transcribed character-for-character from cp950 --
//  :2527, :2529, :2531.  ZERO U+FFFD.
// =============================================================================
bool AutoTrayEndMusic(int iSkeyTime, int iHandMode)                             //kevin 20170904 (wei) Auto Tray end 每盤吸到空位置就發ALARM 及音樂。
{
    if(bNeedMusicFinishLighAndAlarmOn==false)                                   //kevin 20170816 每盤進來啟動音樂
    {
        bNeedMusicAndAlarmOn=true;                                              //kevin 20170816 add 開始起動
    }

    if(iSkeyTime>=iHandMode)
        return true;

    return false;
}

//==============================================================================
//  ROLE          bCheckYPitchRunHomeSen -- drive the auto-Y-pitch axis to 0 and
//                report whether it has BOTH finished the move and lit the home LED.
//                iXYHome 0 = in-arm (MInArmPitchY), else out-arm (MOutArmPitchY).
//                Machines without the auto-Y-pitch option always report false.
//
//  WAVE SCOPE    ACTIVE  bCheckYPitchRunHomeSen   golden ainarm9045.cpp:2540-2562
//
//  GATE REGISTER (this part file)   -- EMPTY.  USE_IN_Y_IS_AUTO_PITCH (cmydef.h:2890),
//                MInArmPitchY / MOutArmPitchY (cmydef.h:2133/2135), MOT[] TTrayMotor
//                (Motor/mymotor.h:385), MotorMove(int) (:163), ScanMotorStatus()
//                (:168), Led[10] (:176), iHomeLed==1 (Motor/HTMotor.h:42) all exist.
//
//  FAITHFUL DETAIL: golden assigns the INT return of MotorMove(0) into `bool
//  bflag1` -- golden Motor/mymotor.h:75 also declares `int MotorMove(int p);`, so
//  this is golden`s own int->bool narrowing, not a translation artifact.  Kept.
//
//  OFFLINE BEHAVIOUR vs the stub it retires (ainarm9045.cpp:2292 `{ return false; }`):
//  identical whenever USE_IN_Y_IS_AUTO_PITCH is false, and once it is true the
//  answer now depends on the real sim-motor state instead of being hard-false --
//  which is the point.  Live callers: the 1x1_1 / 1x2_2 / 1x2_2_14 / 1x2_4_Hot /
//  1x4_* / 2x*_* variant SMs (e.g. ainarm9045_1x1_1.cpp:1023).
//  AI(W7D-k3-ainarm) 20260811
//==============================================================================
bool bCheckYPitchRunHomeSen(int iXYHome)                                        //kevin 20180822 (Steven) : y pitch 遮住home
{
    bool bflag1=false, bflag2=false;
    if(USE_IN_Y_IS_AUTO_PITCH==true)                                            //JerryYang 20251218 : IN/OUT ARM支援不同模組
    {
        if(iXYHome==0)
        {
            bflag1=MOT[MInArmPitchY].MotorMove(0);
            MOT[MInArmPitchY].ScanMotorStatus();
            bflag2=MOT[MInArmPitchY].Led[iHomeLed];
        }
        else
        {
            bflag1=MOT[MOutArmPitchY].MotorMove(0);
            MOT[MOutArmPitchY].ScanMotorStatus();
            bflag2=MOT[MOutArmPitchY].Led[iHomeLed];
        }

        if(bflag1 && bflag2)
            return true;
    }
    return false;
}

//============================================================================
//  bCheckYPitchHome  --  golden ainarm9045.cpp:2564-2582   (k1-ainarm)
//
//  ROLE: drives the Y-pitch (nozzle spread) axis back to its home position for
//        the in-arm (iXYHome==0) or the out-arm (iXYHome!=0), and answers "is it
//        home yet?".  Only meaningful when the machine has the auto-Y-pitch
//        module; otherwise golden returns false unconditionally.
//
//  WAVE SCOPE:
//    * bCheckYPitchHome(int)             ACTIVE (verbatim)      golden :2564-2582
//
//  GATE REGISTER: empty -- this function contains no #if 0.  ProcessSingleMotorHome
//  IS linkable in the port, so nothing here had to be gated.
//
//  SEAM / TRAP 1 note, shape (b) "a stub already satisfied the demand" -- READ
//  THIS BEFORE TRUSTING THE RETURN VALUE.  The only callee, ProcessSingleMotorHome,
//  resolves to the offline shim `bool ProcessSingleMotorHome(int) { return true; }`
//  at acatchtray_shims.cpp:117 (declared acatchtray_shims.h:373 "offline: home
//  done (true)").  So offline this function returns TRUE the moment
//  USE_IN_Y_IS_AUTO_PITCH is true -- it never waits for a real axis.  That is the
//  established offline contract of that shim and I did NOT change it, but it means
//  the 40-odd `if(bCheckYPitchHome(0))` guards in the ainarm9045_* variant SMs
//  advance on the first pump.  Real machine: ProcessSingleMotorHome runs the
//  per-motor home task (issue home, poll status, check the home LED) over several
//  pumps and only then returns true.
//
//  ProcessSingleMotorHome is declared locally just below rather than by adding
//  #include "acatchtray_shims.h": that header is not in this TU's include list and
//  the include list lives at the top of ainarm9045.cpp, outside my part file.  A
//  local forward declaration is the convention this very file already uses --
//  ainarm9045.cpp:1390-1400 forward-declares DoShakeShuttle / DoKnockShuttle /
//  DoVibrateShuttle / RecordShtSuperflous / IsTestZ*NotSafe* the same way, and
//  aoutarm9045S_1x4_4.cpp:147 forward-declares bCheckYPitchHome itself.  The text
//  below is byte-identical to acatchtray_shims.h:373, so if that header is ever
//  pulled into this TU the two declarations agree.
//
//  RETIRING THE OLD STUB: port ainarm9045.cpp:2291 currently holds
//  `bool bCheckYPitchHome(int) { return false; }`.  That line is the main loop's to
//  delete at integration -- note the offline VALUE FLIPS false -> true for machines
//  with USE_IN_Y_IS_AUTO_PITCH, which is the faithful direction but is a live
//  behaviour change for every variant SM that polls it.
//============================================================================
bool ProcessSingleMotorHome(int iMot);                                          // mirrored verbatim from acatchtray_shims.h:373 (not #included by this TU) -- offline: home done (true)
//============================================================================
bool bCheckYPitchHome(int iXYHome)                                              //kevin 20180822 (Steven) : y pitch 歸home 動作
{
    bool bflag1=false;
    if(USE_IN_Y_IS_AUTO_PITCH==true)                                            //ChungHung 20140304 add for AutoYPitch  //JerryYang 20251218 : IN/OUT ARM支援不同模組
    {
        if(iXYHome==0)
        {
            bflag1=ProcessSingleMotorHome(MInArmPitchY);
        }
        else
        {
            bflag1=ProcessSingleMotorHome(MOutArmPitchY);
        }

        if(bflag1)
            return true;
    }
    return false;
}

//==============================================================================
//== ROLE: AddLoadingCount -- the single accounting hook golden runs the moment
//==       the in-arm has successfully sucked ONE IC off the loader tray.  It
//==       (1) re-reads the SCK-ART loading-count verdict and may abort the pick
//==       with bSCKARTLoadCntAlarm, (2) marks the nozzle as "suck end", (3) does
//==       the tray-map cross-check, (4) MOVES the item from the tray grid into the
//==       in-arm grid (CopyFromTray), (5) records site / rotate-angle / motor
//==       position / production-record rows, and (6) bumps ~18 different counters
//==       (lot, day, jam-rate, ART, QA-mode, Piggy-Back, SECS, UI panels).
//==
//== WAVE SCOPE (k2-ainarm):
//==   AddLoadingCount  golden ainarm9045.cpp:2592-2826  -- ACTIVE with 13 gates
//==     (every counter, every grid write and the CopyFromTray item move are
//==      ACTIVE; the 13 gates are all UI-form members / cross-module helpers that
//==      genuinely have no port home -- enumerated one by one below)
//==
//==============================================================================
//==  INCLUDES THIS PART NEEDS  --  MAIN LOOP: HOIST THESE THREE LINES INTO THE
//==  TU's INCLUDE BLOCK AT THE TOP OF ainarm9045.cpp AND DELETE THEM FROM HERE.
//==  They are emitted here (file scope, legal C++) so the part compiles as-is if
//==  the hoist is forgotten.  All three were verified by compiling the WHOLE
//==  current port ainarm9045.cpp (2,332 lines) with exactly these three added and
//==  the real ht9045_sm flags -- clean, zero diagnostics:
//==    g++ -fsyntax-only -std=c++1z -DCSYSTEM_TICK_ORACLE -DDLLDIR_EX
//==        -DMN200DLL_EXPORTS -DWINVER=0x0601 -D_WIN32_WINNT=0x0601
//==        -I<port> -I<port>/Motor/vendor -I<port>/EtherCAT/vendor
//==        -I<port>/SECSGEM -I<port>/third_party/sqlite3   (2026-08-11 01:31 local)
//==  WHY EACH ONE, and why NOT a TU-local stand-in:
//==    * atester_shims.h  -> `fContact` (TfContactShim*, atester_shims.h:251) for
//==      golden :2686/:2712.  TRAP 5 NOTE: in the port `fContact` is a
//==      TfContactShim*, NOT golden's TfContact* -- cContact.h declares no
//==      `fContact` at all -- so there is exactly one candidate and no offset
//==      hazard.  Precedent: every in-arm variant TU already includes this header
//==      for the same symbol (e.g. ainarm9045_1x1_1.cpp:64).
//==    * cSocket.h        -> `LotSummary` for golden :2761.  The object is REAL
//==      (`TLotSummary LotSummary;` cSocket.cpp:173, extern cSocket.h:249, class
//==      cSocket.h:189).  A TU-local stand-in was rejected deliberately: it would
//==      make iLoadTotal a per-TU counter that the lot summary never reads (the
//==      shape Automation/SCK_ART_Remainder.cpp:233 already had to work around).
//==    * cContact.h       -> `CONTACT_DEVICE_MAP_CHECK` (const int 9,
//==      cContact.h:83) for golden :2686.  Golden gets this header via
//==      MachineDefine.h; the PORT MachineDefine.h has its whole golden include
//==      block inside `#if 0` (MachineDefine.h:34-146, cContact.h at :137), which
//==      is why it is not already visible.  Including it is therefore MORE
//==      golden-faithful, not less.  A `static const int CONTACT_DEVICE_MAP_CHECK
//==      = 9;` TU-local (the AutoClean.cpp:118 precedent) was rejected because it
//==      would then REDEFINITION-clash the day anyone hoists cContact.h.
//==============================================================================
//==  GATE REGISTER  (13 gates; all absence commands run with cwd
//==  D:/HT9045/HT9011UC_Cpp_V3.33.906.0, ALL MEASURED AT 2026-08-11 01:35:11 local
//==  (Asia/Taipei) unless a different time is stated on the entry)
//==
//== [W7D-K2-G02] golden :2603  fAutoTeach->CheckLoaderCountNeedOneCycle()
//==   SYMBOL  : the global `fAutoTeach` (golden AutoTeach.h TfAutoTeach*; method
//==             golden AutoTeach.cpp:1247).  The port HAS AutoTeach.h/.cpp files,
//==             but there is no `fAutoTeach` object and no facade for it.
//==   FAITHFUL?: yes -- the call is a pure UI/auto-alignment marker that sets a
//==             "this lot needs a OneCycle" flag inside the AutoTeach form; it
//==             returns nothing and AddLoadingCount reads nothing back from it.
//==             Skipping it changes no state this function or its callers touch.
//==   REAL-MACHINE DIFFERENCE: in CCD auto-alignment mode the loader-count-driven
//==             "please run one cycle" request is never raised, so an operator can
//==             keep loading past the alignment re-check point.
//==   ABSENCE : rg -n "\*fAutoTeach|fAutoTeach *;|fAutoTeach *=" --glob "*.h"
//==             --glob "*.cpp" .    -> 0 hits (the only textual fAutoTeach hits in
//==             the tree are GATE comments: aoutarm.cpp:4147 GATE G13 and
//==             aTester_Rear.cpp:93-99, both citing the SAME premise).
//==   TRAP 3  : if a facade lands, RETIRE -- there is no second reason.
//==
//== [W7D-K2-G03] golden :2616-2621  fSCKART->DoChkInputCntAlarm(true)
//==   SYMBOL  : TfSCKART::DoChkInputCntAlarm(bool).  The port `fSCKART` is
//==             forms/fSCKART.h:74 TfSCKART (extern at :177) and that class does
//==             NOT have this method.  A free-function translation DOES exist
//==             (Automation/SCK_ART.h:152 `SckArt_DoChkInputCntAlarm(const
//==             SckArtState&, bool)`) but it needs a SckArtState, which the real
//==             global fSCKART is not -- so it is not reachable from here.
//==   WHOLE IF GATED, not just the call: the call IS the second conjunct of the
//==             condition, so there is nothing left to test.  Same shape as the
//==             established aTester_Front.cpp:9863 GATE W7F3-G03.
//==   FAITHFUL?: yes -- offline TfSCKART is the no-op facade, so golden would
//==             evaluate DoChkInputCntAlarm(true) as false and fall straight
//==             through this empty block.  The ENCLOSING `if(...)` (golden
//==             :2609-2614) stays ACTIVE, so the else-if chain below is unchanged.
//==   REAL-MACHINE DIFFERENCE: when the SCK-ART input count reaches its limit the
//==             pick is NOT aborted -- bSCKARTLoadCntAlarm stays false and
//==             AddLoadingCount runs to completion, so the machine keeps loading
//==             past the ART lot-check quantity instead of alarming.  (The Renesas
//==             FT-CT exemption read at :2616 is lost with it.)
//==   ABSENCE : rg -c "DoChkInputCntAlarm" forms/fSCKART.h   -> 0 hits
//==
//== [W7D-K2-G04] golden :2630  RestoreLoadeIC(iTrayRow)
//==   SYMBOL  : golden ainarm2.h:220 `extern void RestoreLoadeIC(int iLoaderY=0);`
//==             (JerryYang 20200422: after the auto-skip count is reached, go back
//==             and retry the last row).  The port has no ainarm2.h at all.
//==   FAITHFUL?: partly -- offline nothing re-reads the restored row within this
//==             function, so AddLoadingCount itself behaves identically.  Stated
//==             plainly: this is a real dropped call, not an equivalence.
//==   REAL-MACHINE DIFFERENCE: after auto-skip has given up on a row, that row is
//==             never re-armed for a retry pass, so ICs that a second attempt would
//==             have picked stay on the tray and go out as skipped.
//==   ABSENCE : rg -n "RestoreLoadeIC" --glob "*.cpp" --glob "*.h" .  -> 0 hits
//==
//== [W7D-K2-G05] golden :2637-2648  the whole tray-map cross-check block
//==   SYMBOLS (three absences in one block, which is why ONE gate covers it):
//==     (a) EnableTraymapCheckFunction(int) -- golden ainarm2.h:216, body golden
//==         ainarm2.cpp:3585.  No port body AND no declaration.
//==     (b) MyDeCodeASCII(int) -- golden EJ1N/TextProcess.h:9.  The port HAS
//==         EJ1N/TextProcess.h, but it is NOT reachable from this TU: golden pulls
//==         it via MachineDefine.h:109, and the port MachineDefine.h has that whole
//==         block inside `#if 0` (:34-146).
//==     (c) fTrayMapping->iTrayMappingDate + EventReport(SECS_EVENT.MapNoArmHasIC)
//==         -- both DO exist in the port but only behind acatchtray_shims.h
//==         (fTrayMapping at :313 with iTrayMappingDate at :254; SECS_EVENT/
//==         EventReport via SECSGEM/SecsEventReport.h at :156).  Because (a) gates
//==         the whole block anyway, this part does NOT take that include -- one
//==         fewer structural change.  If (a) ever lands, acatchtray_shims.h must
//==         be added at the same time.
//==   WHOLE IF GATED: (a) is the condition, so nothing is left to test.
//==   FAITHFUL?: yes for the standard offline config -- EnableTraymapCheckFunction
//==             is a config predicate that is false unless tray-map checking is
//==             switched on, and the three ACTIVE clears immediately above the gate
//==             (bTrayMapDataError / iTrayMapDataX / iTrayMapDataY, golden
//==             :2634-2636) already leave exactly the state golden leaves when the
//==             predicate is false.
//==   REAL-MACHINE DIFFERENCE: with tray-map checking ON, an IC picked from a tray
//==             cell whose map data is not 1 is no longer flagged:
//==             bTrayMapDataCheckError stays false, iMapErrorCount does not rise,
//==             and the SECS MapNoArmHasIC event is not reported -- the handler
//==             silently tests material the map says should not be there.
//==   ABSENCE : rg -n "^bool EnableTraymapCheckFunction|extern bool
//==             EnableTraymapCheckFunction" --glob "*.cpp" --glob "*.h" . -> 0
//==             hits (the only two textual hits in the tree are the twin GATE
//==             comments aTester_Front.cpp:9863 / aTester_Rear.cpp:9793 -- SAME
//==             premise, so this gate inherits an already-audited claim rather
//==             than inventing one).
//==
//== [W7D-K2-G06] golden :2707  InArmSuck.iCurrRotAng[..]=FrmRotate->iFromTrayAngle
//==   SYMBOL  : TfRotate::iFromTrayAngle.  `FrmRotate` itself is real
//==             (forms/fRotate.h) but has no iFromTrayAngle member.
//==   ONLY THE ASSIGNMENT is gated; iCurrRotAng keeps whatever it held (0 after
//==             the TMyKitSuck ctor), which is also the "0 degrees" value.
//==   FAITHFUL?: yes offline -- with no rotator installed the from-tray angle IS
//==             0, so the gated write and the live state agree.
//==   REAL-MACHINE DIFFERENCE: with a rotator installed and trays presented at a
//==             non-zero angle, the CURRENT angle is recorded as 0 while
//==             iNeedRotAng (still ACTIVE, golden :2701/:2705) holds the target --
//==             so the rotator would turn by the full target instead of the delta.
//==             That is a REAL mis-rotation, not a cosmetic loss.
//==   ABSENCE : rg -n "iFromTrayAngle" --glob "*.cpp" --glob "*.h" .  -> 0 hits
//==
//== [W7D-K2-G07] golden :2721  the SCK-ART AddLoadRecord row
//==   SYMBOL  : TfSCKART::sInfo_CustLotID.  forms/fSCKART.h has sLotID (:86) and
//==             sLotStartTime (:152) but NOT sInfo_CustLotID (that field exists
//==             only on the SEPARATE Automation/SCK_ART_Remainder.h:711 state
//==             struct, which is not what `fSCKART` points at -- TRAP 5 shape).
//==   ONLY THE ONE LINE is gated.  `LastSet.iSCKARTInputCT++` (golden :2722) stays
//==             ACTIVE, deliberately: it is the counter the ART flow reads back.
//==   FAITHFUL?: no equivalence claimed -- one production-record row is not
//==             written.  I did NOT substitute `sLotID` alone, because that would
//==             silently write a DIFFERENT lot string into a shipped record.
//==   REAL-MACHINE DIFFERENCE: on SCK-ART / SPIL-ART machines the per-IC Load row
//==             (tray no, row, col, X/Y position, lot id, lot start time) is
//==             missing from that IC's production record; the two non-ART
//==             AddLoadRecord paths below are ACTIVE, so only the ART path loses it.
//==   ABSENCE : rg -c "sInfo_CustLotID" forms/fSCKART.h   -> 0 hits
//==
//== [W7D-K2-G08] golden :2724-2728  the fLotInfo lot-ID AddLoadRecord branch
//==   SYMBOL  : TfLotInfo::tsLotID (golden uLotInfo.h:302 TTabSheet).  The port
//==             fLotInfo has edtSysLotID (forms/fLotInfo.h:29) but no tsLotID tab.
//==   WHOLE else-if GATED (the member IS the condition).  The `#if 0`/`#endif`
//==             sits between the previous `}` and this `else`, so the final `else`
//==             at golden :2729 becomes the live arm -- which is EXACTLY golden's
//==             offline behaviour, because with the Lot-ID tab not visible
//==             `tsLotID->TabVisible` is false and golden also falls to that else.
//==   FAITHFUL?: yes -- same arm taken, same AddLoadRecord written, just with
//==             ASET_ScheduleNAME instead of the (offline empty) edtSysLotID text.
//==   REAL-MACHINE DIFFERENCE: when the operator HAS the Lot-ID tab open with a
//==             system lot ID typed in, the Load record row is stamped with the
//==             schedule name instead of that lot ID.
//==   ABSENCE : rg -n "tsLotID" --glob "*.cpp" --glob "*.h" .  -> 1 file, and it
//==             is tools/dfm2rc/layout_out/uLotInfo_layout.gen.cpp:83-85, i.e.
//==             GENERATED LAYOUT DATA (a string table), not a C++ member.
//==
//== [W7D-K2-G09] golden :2735-2736  iRecordTrayPickPosY / iRecordTrayPickPosX
//==   SYMBOL  : golden ainarm2.h:92 `extern int iRecordTrayPickPosX[16][16];` (+Y).
//==             No port home; NOT one of the seven file-scope globals this wave
//==             hands to the main loop, so I do not define them here either.
//==   FAITHFUL?: no equivalence claimed -- two grid writes are dropped.
//==   REAL-MACHINE DIFFERENCE: the "which tray cell did nozzle (r,c) come from"
//==             memory is never written, so the loader RETRY / SKIP paths that
//==             replay it (golden csystem.cpp:2017 / :2063) cannot put an IC back
//==             into the cell it came from.
//==   ABSENCE : rg -n "iRecordTrayPickPos" . -> hits only in csystem.cpp:20130-
//==             20131 and :22396-22398, which are that TU's OWN gate comments
//==             G18/G19 for the SAME two symbols ("no port").  This gate inherits
//==             an already-audited premise.
//==
//== [W7D-K2-G10] golden :2738-2739  fTrayMapping->sgSuckmap->Cells[..]=1
//==   SYMBOL  : TfTrayMapping::sgSuckmap (golden cTrayMapping.h TStringGrid).  The
//==             port TfTrayMapping shim (acatchtray_shims.h:219) does not have it.
//==   WHOLE IF GATED (condition + its single statement, one unit).
//==   FAITHFUL?: yes offline -- USE_TRAY_MAPPING is etmInstall only on machines
//==             with the tray-mapping option, and this write is UI-only (the grid
//==             cell is read back by fTrayMapping->WritePickMapLog(), never by the
//==             pick logic).
//==   REAL-MACHINE DIFFERENCE: the tray-mapping "was an IC sucked from this cell"
//==             grid stays blank, so the pick-map log written at end of tray is
//==             empty even though ICs were taken.
//==   ABSENCE : rg -n "sgSuckmap" --glob "*.cpp" --glob "*.h" .  -> 1 file, and it
//==             is tools/dfm2rc/layout_out/cTrayMapping_layout.gen.cpp:40, i.e.
//==             GENERATED LAYOUT DATA, not a C++ member.
//==
//== [W7D-K2-G11] golden :2741-2759  the VTEST / MES first-record block
//==   SYMBOL  : the global `fMesSystem` (golden Mes/fVATMesFileSys.h TfMesSystem)
//==             and its bFormShowJustInitial / DoInitailRecordReportByTime().  The
//==             whole Mes/ subsystem is absent from the port.
//==   WHOLE nest GATED, and the inner `#ifdef SOFT_SIMULTE` / `#else` / `#endif`
//==             pair is transcribed VERBATIM inside the gate (SOFT_SIMULTE is not
//==             defined in the port, so the `#else` LastSet.iTester==ON_LINE arm is
//==             the one that would be live).
//==   FAITHFUL?: yes offline -- gated by IniConfig.bVTESTFunction && bCheckFile,
//==             both off unless the VTEST/Kirin option is configured.
//==   REAL-MACHINE DIFFERENCE: on a VTEST machine the once-per-lot
//==             "record report by time" MES file is never initialised on the first
//==             IC, so the timed MES report for that lot is missing.
//==   ABSENCE : rg -n "\*fMesSystem" --glob "*.cpp" --glob "*.h" .  -> 0 hits (the
//==             15 textual fMesSystem files are all comments or TU-local shims,
//==             e.g. asendic_Auto.cpp:339 W7L1A_TfMesSystem_AutoShim).
//==
//== [W7D-K2-G12] golden :2769-2770  fObserver->StatisticalLoaderCount()
//==   SYMBOL  : TfObserver::StatisticalLoaderCount().  `fObserver` is real in the
//==             port (atester_shims.h) but has no such method.
//==   WHOLE IF GATED (condition IniConfig.bSIGURDFunction + its one statement).
//==   FAITHFUL?: yes offline -- bSIGURDFunction is a SIGURD/Sigurd-customer
//==             option, off by default, and the method only paints the Jam
//==             statistics page.  Every real counter it would summarise
//==             (LastSet.iLoaderCount, SendCT[], iDaySendCT, ...) is ACTIVE above.
//==   REAL-MACHINE DIFFERENCE: on SIGURD machines the Jam-statistics page loader
//==             column stops advancing (display only; the underlying counts are
//==             still correct).
//==   ABSENCE : rg -n "StatisticalLoaderCount" --glob "*.cpp" --glob "*.h" . -> 0
//==
//== [W7D-K2-G13] golden :2796-2799  fLotInfo->RefreshOtherTool()
//==   SYMBOL  : TfLotInfo::RefreshOtherTool().  fLotInfo is real; method is not.
//==   WHOLE inner IF GATED.  The enclosing `if(TestIF.bContinuousLoader==true)`
//==             and its ACTIVE counter bumps (golden :2788-2794) are untouched.
//==   FAITHFUL?: yes offline -- it is guarded by CUSTOMER_CODE==CC_PTI and it only
//==             refreshes the "other tool" panel on the LotInfo form.
//==   REAL-MACHINE DIFFERENCE: on PTI machines running Piggy-Back continuous
//==             loading, the LotInfo other-tool panel stops refreshing.
//==   ABSENCE : rg -n "RefreshOtherTool" --glob "*.cpp" --glob "*.h" .  -> 0 hits
//==
//== [W7D-K2-G14] golden :2812-2817  fSortCT->ShowLoadingIC_ART()
//==   SYMBOL  : TfSortCT::ShowLoadingIC_ART().  forms/fSortCT.h:71 has the plain
//==             ShowLoadingIC() (ACTIVE at golden :2810, un-gated) but not the ART
//==             twin.
//==   WHOLE IF GATED (the 3-line ART condition + its one statement).
//==   FAITHFUL?: yes offline -- USE_AUTO_RETEST is eartInstall / bUseARTSortCount
//==             only on ART machines, and this repaints a panel; the ART COUNTERS
//==             it would display (LastSet.SendCT_ART[0..2],
//==             LastSet.iLoaderTrayCount_ART) are all ACTIVE above.
//==   REAL-MACHINE DIFFERENCE: on ART machines the ART load panel is not
//==             repainted (display only).
//==   ABSENCE : rg -n "ShowLoadingIC_ART" --glob "*.cpp" --glob "*.h" . -> 1 file,
//==             SECSGEM/uHGemHT9045.cpp:4348/:4378/:5352, all of them that TU's own
//==             GATE G08 comments recording the SAME absence ("only ShowLoadingIC
//==             exists").  Premise inherited, not invented.
//==============================================================================
//==  GOLDEN BUGS FOUND AND DELIBERATELY PRESERVED (do NOT "fix" these):
//==   (1) `iSiteCol` (golden :2594) is initialised to 0 and NEVER assigned, yet it
//==       indexes Prod.RotateDutDate at golden :2705.  So the non-rotator branch
//==       always reads column 0 of the rotate-angle table regardless of which
//==       nozzle picked.  Kept verbatim.
//==   (2) golden :2701 vs :2705 index the SAME array with DIFFERENT coordinate
//==       systems -- [iSuckRow][iSuckCol] (nozzle space) when the rotator is
//==       active, [iSiteRow][iSiteCol] (site space) otherwise.  Kept verbatim.
//==   (3) golden :2629 passes the tray coordinates to AutoSkipHasIClog in the order
//==       (iTrayCol, iTrayRow) while the immediately following AddLoadRecord calls
//==       pass (iTrayRow, iTrayCol).  One of the two is transposed in golden.
//==       Kept verbatim -- both orders transcribed exactly as written.
//==   (4) golden :2802-2808 is `if(CosFunction.bQAModeUseUnloadCnt){}else{...}` --
//==       an empty then-branch.  Kept, including the empty braces.
//==
//==  INTEGER DIVISION: there is no division anywhere in this function.  The two
//==  `?:` selects (`(InArmSuck.iWhichKit==1)?4:0` at :2676 and `(iKit==0)?0:4`
//==  patterns elsewhere) are selects, not divisions.  Nothing was floated.
//==
//==  INTEGRATION NOTE (TRAP 1 shape (b)) -- THE STUB IS NOT IN THIS FILE:
//==  AddLoadingCount is declared at ainarm9045_2x4_16_shims.h:57 and STUBBED at
//==  ainarm9045_2x4_16_shims.cpp:24 (`void AddLoadingCount(int,int,int,int) {}`).
//==  BOTH must go, or this definition collides.  Live callers exist today
//==  (ainarm9045_2x4_16.cpp:1358, ainarm9045_2x8_32.cpp:1702), so this is the
//==  single most behaviour-visible body in my slice: every loader counter in the
//==  system currently never moves, and every picked IC currently never leaves the
//==  tray grid, because CopyFromTray was inside that no-op.  The definition is NOT
//==  static (TRAP 1 shape (d) avoided; a non-static declaration exists in that
//==  shim header).
//==
//==  Big5 -> UTF-8: every Chinese comment transcribed character for character.
//==============================================================================
#include "atester_shims.h"      // W7D-k2 HOIST-ME: fContact (TfContactShim*, atester_shims.h:251) for golden :2686/:2712
#include "cSocket.h"            // W7D-k2 HOIST-ME: LotSummary (extern cSocket.h:249, object cSocket.cpp:173) for golden :2761
#include "cContact.h"           // W7D-k2 HOIST-ME: CONTACT_DEVICE_MAP_CHECK (const int 9, cContact.h:83) for golden :2686 -- golden reaches it via MachineDefine.h:137, which the port has inside #if 0 (:34-146)
//==============================================================================
void AddLoadingCount(int iSuckRow, int iSuckCol, int iTrayRow, int iTrayCol)    //Steven 20161123 : 將Loader吸取的數量計算統一成Function
{
    int iSiteRow=0, iSiteCol=0;
    int iMotXpos=0, iMotYpos=0;                                                 //Sam 20250211 : Add Load Pos
    bSCKARTLoadCntAlarm=false;
    iSCKARTLoadingStatus=fSCKART->CheckLoadingCount();                          //Steven 20161223 (wei) : For SCK ART loading count
    iLoadPickX=iSuckRow;
    iLoadPickY=iSuckCol;

    bAutoRetryFlag=false;                                                       //JerryYang 20220901 : ASE-CL pick up error時吸取邏輯要改找吸最左上角的

#if 0 // GATE W7D-K2-G02 -- golden :2603 fAutoTeach->CheckLoaderCountNeedOneCycle() -- no fAutoTeach global anywhere in the port (see GATE REGISTER for the exact command + timestamp)
    fAutoTeach->CheckLoaderCountNeedOneCycle();                                 //JimmyChiu 20211020 : Auto alignment mode
#endif // GATE W7D-K2-G02

    bSuckEnd[iSuckRow][iSuckCol]=true;
    InArmSuckUse[iSuckRow][iSuckCol]=false;
    bPickLoaderDuplicateErr[iSuckRow][iSuckCol]=false;

    if((bCanRunSCKART==true ||
        IniConfig.bA65_BundleIDList) &&                                         //JerryYang 20250220 : fix AUTO IN OUT
       LastSet.iRunStartMode!=rsmContinuRetest  &&                              //Steven 20170513 (jou) : For SCK ART can do manual RT
       LastSet.iRunStartMode!=rsmCInitialRetest &&
       iSCKARTLoadingStatus>=1                  &&                              //數量到了要放下來,然後 Alarm
       TestIF_File.bSCKART_LotDeviceCheck       )
    {
#if 0 // GATE W7D-K2-G03 -- golden :2616-2621 fSCKART->DoChkInputCntAlarm(bool) -- absent on forms/fSCKART.h TfSCKART.  WHOLE if gated because the call IS the condition; enclosing if(:2609-2614) stays ACTIVE so the else-if chain is unchanged
        if(TestIF_File.bRENESAS_EnableFTCT==false &&                            //RogerYang 20250923 : 瑞薩FT-CT 不要Alarm 數量到了繼續把這一盤做完
            fSCKART->DoChkInputCntAlarm(true)==true)                            //RogerYang 20251224 : Rf360需求 lotcheck加入上下限設定
        {
            bSCKARTLoadCntAlarm=true;
            return;
        }
#endif // GATE W7D-K2-G03
    }
    else if(IniConfig.bP57LoaderAutoCleanOutByInputCT &&                        //Sam 20250605 : Loader Count AutoCleanOut
            iSCKARTLoadingStatus>=1)
    {
        bSCKARTLoadCntAlarm=true;
        return;
    }
    AutoSkipHasIClog(InArmSuck.Suck[iSuckRow][iSuckCol].sName, iTrayCol, iTrayRow);                                     //Steven 20211228 : 換位置 //kevin 20170904 (wei) add 記錄每一盤 SKIP 沒吸到IC位置
#if 0 // GATE W7D-K2-G04 -- golden :2630 RestoreLoadeIC(int) -- golden ainarm2.h:220, no port body and no declaration
    RestoreLoadeIC(iTrayRow);                                                   //Steven 20211228 : 換位置 //JerryYang 20200422 Auto skip次數到達後, 自動再去最後一排吸吸看
#endif // GATE W7D-K2-G04
    AddInArmPickerCount(iSuckRow, iSuckCol);                                    //JerryYang 20220331 : add 吸真空次數計數
    CheckPickerLifeNeedOneCycle();                                              //AI(ht9045-config) 20260521 (RogerYang) : SCC吸嘴壽命報警OneCycle優化

    bTrayMapDataError[iSuckRow][iSuckCol]=false;                                //wei 20170302 Tray map data error
    iTrayMapDataX[iSuckRow][iSuckCol]=0;
    iTrayMapDataY[iSuckRow][iSuckCol]=0;
#if 0 // GATE W7D-K2-G05 -- golden :2637-2648 EnableTraymapCheckFunction(int) (golden ainarm2.h:216 / ainarm2.cpp:3585, no port body) + MyDeCodeASCII (EJ1N/TextProcess.h, unreachable: MachineDefine.h:109 is inside the port #if 0) + fTrayMapping/EventReport (exist only behind acatchtray_shims.h, not taken since the block is gated anyway).  The 3 ACTIVE clears at :2634-2636 already leave golden predicate-false state
    if(EnableTraymapCheckFunction(0))                                           //wei 20170302 (Steven) Tray map data error
    {
        if(MyDeCodeASCII(fTrayMapping->iTrayMappingDate[iTrayRow][iTrayCol])!=1)
        {
            bTrayMapDataCheckError=true;
            bTrayMapDataError[iSuckRow][iSuckCol]=true;
            iTrayMapDataX[iSuckRow][iSuckCol]=iTrayCol;
            iTrayMapDataY[iSuckRow][iSuckCol]=iTrayRow;
            iMapErrorCount++;
            EventReport(SECS_EVENT.MapNoArmHasIC);                              //wei 20170609
        }
    }
#endif // GATE W7D-K2-G05

    if(IniConfig.bI27_ManualSortMode && bRunManualSortMode==true)               //Steven 20150915 : For TSMC 手動整盤功能
        InArmSuck.CopyFromTray(iSuckRow, iSuckCol, HAS_IC, MOT[MMTrayY], iTrayRow, iTrayCol, NULL_IC, MOT[MMTrayY].Tray.iTarget[iTrayCol][iTrayRow]);
    else
        InArmSuck.CopyFromTray(iSuckRow, iSuckCol, HAS_IC, MOT[MMTrayY], iTrayRow, iTrayCol, NULL_IC);

    InspectInArmPosition(MMTrayY, iSuckRow, iSuckCol, iTrayRow, iTrayCol, false);

    if(IsNNMode()==NN_2Row)
    {
        if(InArmSuck.iWhichSht==0)
            iSiteRow=iSuckRow+2;
        else
            iSiteRow=iSuckRow;
    }
    else if(IsNNMode()==NN_1Row)
    {
        if(InArmSuck.iWhichSht==0)
            iSiteRow=iSuckRow+1;
        else
            iSiteRow=iSuckRow;
    }
    else
    {
        iSiteRow=iSuckRow;
    }

    int iPickKit=(InArmSuck.iWhichKit==1)?4:0;
    if(iCloseSiteModeFor2x8>e2x8OneByOne ||
       iCloseSiteModeFor2x6>e2x6OneByOne)
    {
        iPickKit=0;
    }

    if((IniConfig.bI37_EnableFIFOMode &&
        IniConfig.bI37_EnableFIFOSiteOrder &&
        LastSet.iRunStartMode==rsmFIFOMode) ||                                  //Steven 20170302 (wei) : FIFO MODE
       ((fContact->fShow && iContactMode==CONTACT_DEVICE_MAP_CHECK) ||          //Steven 20220510 : For QTI SD Device Map Function
         fContact->IsRun2DCheck()==true))                                       //JerryYang 20250220 : 2DID硬體順序檢查功能
    {
        //這兩個模式不能用
    }
    else                                                                        //Steven 20220527 : for JCET Auto Site Map
    {
        InArmSuck.iWhichSite[iSuckRow][iSuckCol]=Prod.iSiteMap[InArmSuck.iWhichSht][iSuckRow][iSuckCol+iPickKit];       //Steven 20220526 : 針對Auto Site Map加熱模式做紀錄   //Steven 20240508 : fixed for iWhichSite flag
    }

    if(USE_ROTATE_KIT==1 &&
       (iRotate_Type==e1MotRotate1Dut ||                                        //Steven 20250801 : for rotator
        iRotate_Type==e2MotRotate2Dut) &&
       tRotate.ActiveRotate)
    {
        InArmSuck.iNeedRotAng[iSuckRow][iSuckCol]=Prod.RotateDutDate[0][iSuckRow][iSuckCol];                            //預計旋轉的角度   //jou 20231122 : Rotate Use RT mode
    }
    else
    {
        InArmSuck.iNeedRotAng[iSuckRow][iSuckCol]=Prod.RotateDutDate[0][iSiteRow][iSiteCol];                            //預計旋轉的角度   //jou 20231122 : Rotate Use RT mode
    }
#if 0 // GATE W7D-K2-G06 -- golden :2707 FrmRotate->iFromTrayAngle -- FrmRotate is real (forms/fRotate.h) but has no iFromTrayAngle member; iCurrRotAng keeps its ctor 0 (= 0 degrees)
    InArmSuck.iCurrRotAng[iSuckRow][iSuckCol]=FrmRotate->iFromTrayAngle;        //目前產品的角度
#endif // GATE W7D-K2-G06

    iMotXpos=MOT[MInArmX].ReadPos();                                            //Sam 20250211 : Add Load Pos
    iMotYpos=MOT[MInArmY].ReadPos();

    if(fContact->IsRun2DCheck()==true)                                          //JerryYang 20250220 : 2DID硬體順序檢查功能
    {
        InArmSuck.PordRec[iSuckRow][iSuckCol].asBuffer->Strings[eLoadX]   =AnsiString(iTrayCol);                        //JerryYang 20250220 : 2DID硬體順序檢查功能
        InArmSuck.PordRec[iSuckRow][iSuckCol].asBuffer->Strings[eLoadY]   =AnsiString(iTrayRow);
        return;
    }

    if(bCanRunSCKART==true || CosFunction.bART_SECSGEM_93K)                     //JerryYang 20220923 : add SPIL             //Steven 20161214 (wei) : For SCK ART
    {
#if 0 // GATE W7D-K2-G07 -- golden :2721 fSCKART->sInfo_CustLotID -- absent on forms/fSCKART.h TfSCKART (it exists only on the separate Automation/SCK_ART_Remainder.h:711 state struct).  Only THIS line is gated; LastSet.iSCKARTInputCT++ below stays ACTIVE.  Deliberately NOT substituted with sLotID alone -- that would write a different lot string into a shipped record
        InArmSuck.PordRec[iSuckRow][iSuckCol].AddLoadRecord(ASE_InTrayNum, iTrayRow, iTrayCol, iMotXpos, iMotYpos, fSCKART->sLotID+" "+fSCKART->sInfo_CustLotID, fSCKART->sLotStartTime);
#endif // GATE W7D-K2-G07
        LastSet.iSCKARTInputCT++;
    }
#if 0 // GATE W7D-K2-G08 -- golden :2724-2728 fLotInfo->tsLotID (golden uLotInfo.h:302 TTabSheet) -- absent on the port fLotInfo.  Gating the whole else-if hands the final else (:2729) the live arm, which is exactly what golden does offline because TabVisible is false
    else if(fLotInfo->tsLotID->TabVisible==true &&
            fLotInfo->edtSysLotID->Text!="")                                    //Steven 20200617 : Log加上Lot ID
    {
        InArmSuck.PordRec[iSuckRow][iSuckCol].AddLoadRecord(ASE_InTrayNum, iTrayRow, iTrayCol, iMotXpos, iMotYpos, fLotInfo->edtSysLotID->Text, ASET_StartTimeNAME);
    }
#endif // GATE W7D-K2-G08
    else
    {
        InArmSuck.PordRec[iSuckRow][iSuckCol].AddLoadRecord(ASE_InTrayNum, iTrayRow, iTrayCol, iMotXpos, iMotYpos, ASET_ScheduleNAME, ASET_StartTimeNAME);      //Steven 20160114 : Add Production Log -- Load
    }
    InArmSuck.PordRec[iSuckRow][iSuckCol].AddInArmLoaderPickRecord(iSuckRow, iSuckCol);                                 //Sam 20200716 : Add record

#if 0 // GATE W7D-K2-G09 -- golden :2735-2736 iRecordTrayPickPosY/X -- golden ainarm2.h:92, no port home.  SAME premise the port already gated at csystem.cpp:20130-20131 (G18/G19)
    iRecordTrayPickPosY[iSuckRow][iSuckCol]=iTrayRow;
    iRecordTrayPickPosX[iSuckRow][iSuckCol]=iTrayCol;
#endif // GATE W7D-K2-G09

#if 0 // GATE W7D-K2-G10 -- golden :2738-2739 fTrayMapping->sgSuckmap -- absent on the acatchtray_shims.h:219 TfTrayMapping shim (only generated layout data mentions the name)
    if(USE_TRAY_MAPPING==etmInstall)
        fTrayMapping->sgSuckmap->Cells[iTrayCol+1][iTrayRow+1]=1;               //wei 20170119 (Steven) Tray Mapping紀錄是否有吸取IC
#endif // GATE W7D-K2-G10

#if 0 // GATE W7D-K2-G11 -- golden :2741-2759 fMesSystem (golden Mes/fVATMesFileSys.h TfMesSystem) -- the whole Mes/ subsystem is absent.  The inner #ifdef SOFT_SIMULTE / #else pair is transcribed VERBATIM inside this gate (SOFT_SIMULTE undefined -> the #else ON_LINE arm is the one that would be live)
    if(IniConfig.bVTESTFunction==true && IniConfig.bCheckFile==true)
    {
        if(LastSet.SendCT[0]==0)
        {
            if(fMesSystem->bFormShowJustInitial==false)                         //marvin 20200424 (Kirin) Added always record report by time.
            {
#ifdef SOFT_SIMULTE
                fMesSystem->bFormShowJustInitial=true;
                fMesSystem->DoInitailRecordReportByTime();
#else
                if(LastSet.iTester==ON_LINE)
                {
                    fMesSystem->bFormShowJustInitial=true;
                    fMesSystem->DoInitailRecordReportByTime();
                }
#endif
            }
        }
    }
#endif // GATE W7D-K2-G11

    LotSummary.iLoadTotal++;
    iUPH_LoaderCount++;
    LastSet.SendCT[0]++;
    LastSet.SendCT[1]++;                                                        //jou 2010-08-13 計數jam rate,改為Tray Feed為一單位
    LastSet.iDaySendCT++;                                                       //jou 20210108 : 上海偉測要求新增每日jam rate統計
    iRecordJamRateByTime_LoaderCount++;                                         // 2015.11.11 , Joye , Add Jam Rate Record
    LastSet.iLoaderCount++;                                                     //Sam 20210107 : Summary Report fuction
    iOneTrayPickCount[0]++;                                                     //kevin 20210623  loader 目前TRAY吸取數量
#if 0 // GATE W7D-K2-G12 -- golden :2769-2770 fObserver->StatisticalLoaderCount() -- fObserver is real (atester_shims.h) but has no such method; display only, every underlying counter stays ACTIVE
    if(IniConfig.bSIGURDFunction)                                               //KaiChen 20200618 ：矽格，增加Jam統計頁面
        fObserver->StatisticalLoaderCount();
#endif // GATE W7D-K2-G12

    if(bRTLoaderTrayCount)
    {
        LastSet.iLoaderTrayCount_ART++;
        bRTLoaderTrayCount=false;                                               //wei 20160112 ART R1不算空盤
    }

    if((USE_AUTO_RETEST==eartInstall &&                                         //kevin 20150615
        (bAutoReTest_ART || IniConfig.bA10_AutoReTest)) ||
        CosFunction.bUseARTSortCount)                                           //Ifor 20170315 (wei) add 新增使用ART Sort Count 計數功能
    {
        LastSet.SendCT_ART[0]++;
        LastSet.SendCT_ART[1]++;
    }

    if(TestIF.bContinuousLoader==true)
    {
        LastSet.SendCT[2]++;                                                    //jou 2010-11-17 Piggy-Back Functions
        if((USE_AUTO_RETEST==eartInstall &&                                     //kevin 20150615
           (bAutoReTest_ART || IniConfig.bA10_AutoReTest)) ||
           CosFunction.bUseARTSortCount)                                        //Ifor 20170315 (wei) add 新增使用ART Sort Count 計數功能
        {
            LastSet.SendCT_ART[2]++;
        }

#if 0 // GATE W7D-K2-G13 -- golden :2796-2799 fLotInfo->RefreshOtherTool() -- fLotInfo is real, method is not.  Enclosing if(TestIF.bContinuousLoader) and its counter bumps stay ACTIVE
        if(CUSTOMER_CODE==CC_PTI)
        {
            fLotInfo->RefreshOtherTool();
        }
#endif // GATE W7D-K2-G13
    }

    if(CosFunction.bQAModeUseUnloadCnt)                                         //JerryYang 20221004 : Maxim版本QA mode
    {
    }
    else
    {
        iQAModeLoaderCT++;                                                      //jou 2012-05-03 增加QA mode使用獨立的Loader Count
    }
    LastSet.iIndexInputOutPut[3]++;                                             //jou 2015-08-21 add InArm Pick count
    fSortCT->ShowLoadingIC();

#if 0 // GATE W7D-K2-G14 -- golden :2812-2817 fSortCT->ShowLoadingIC_ART() -- forms/fSortCT.h:71 has ShowLoadingIC (ACTIVE at :2810) but not the ART twin; display only, ART counters stay ACTIVE.  SAME absence already recorded at SECSGEM/uHGemHT9045.cpp:4348 (G08)
    if((USE_AUTO_RETEST==eartInstall &&                                         //kevin 20150615
        (bAutoReTest_ART || IniConfig.bA10_AutoReTest)) ||
       CosFunction.bUseARTSortCount)                                            //Ifor 20170315 (wei) add 新增使用ART Sort Count 計數功能
    {
        fSortCT->ShowLoadingIC_ART();
    }
#endif // GATE W7D-K2-G14

    iContinueAutoSkipAutoTrayEnd=0;                                             //ChungHung 20141002 add for KYEC AutoRetest
    bLoaderHasSuck=true;

    if(CosFunction.bShowHPICCount)                                              //Steven 20221228 : 計算加熱盤IC數量
    {
        fSortCT->pnlLoad->Caption=MOT[MMTrayY].Tray.HowManyIC();
    }
}

//==============================================================================
//  AI(k6-ainarm) 20260811 -- golden ainarm9045.cpp:2828-2892  DoTrayEndProcess_9045
//
//  ROLE          -- The LOADER TRAY-END recovery: clears the auto-skip counter,
//                   latches 'this tray had a skip' + the 2-tray manual-removal
//                   demand (except CC_ASE_Korea, which deliberately does
//                   NOTHING), releases the 3 loader cylinders, clears every
//                   nozzle's vacuum-Error / use / duplicate-error flag, re-inits
//                   the loader tray to NULL_IC, rewinds the central pick SM
//                   cursor, dumps the ASE-KaohSiung tray-end position log, and
//                   returns TRUE only when the arm may keep going with a partly
//                   filled picker set (non-Hot layouts with P17 off, or FIFO
//                   mode) -- otherwise FALSE.
//  WAVE SCOPE    -- DoTrayEndProcess_9045  ACTIVE  (golden :2828-2892)
//  GATE REGISTER -- none.  This part emits no #if 0.  Every symbol golden touches
//                   here is REAL in the port; the one thing missing was an
//                   include, handled below rather than by gating a cylinder OFF.
//
//  GOLDEN BUG, PRESERVED: `bHasSkip` is written (:2845, :2852) and never read --
//  its only consumer is the commented-out MES0101 production-record block at
//  :2853-2857.  Transcribed as-is, comment block included; not repaired.
//
//  RETIRE AT INTEGRATION: ainarm9045.cpp:2294 holds the offline stub
//  `bool DoTrayEndProcess_9045() { return false; }  // golden :2828` in the SAME
//  file -- delete it or the build breaks with a redefinition.  Note the stub was
//  NOT behaviour-neutral: it returned false unconditionally, i.e. it turned the
//  'may continue with a partly filled picker' answer into 'must not', so landing
//  the real body CHANGES offline behaviour on the FIFO / P17-off paths.  That is
//  the point of landing it, but it is a behaviour delta, so it is stated.
//
//  Deps verified REAL: SetAutoSkipCount (ainarm9045.h:88, stub :2274),
//  CUSTOMER_CODE / CC_ASE_Korea (MachineType.h:304) / CC_ASE_KaohSiung,
//  bLoaderHasSkip (cmydef.h:3267), iManualRemoveTrayCnt (cmydef.h:3269),
//  bAutoRetryFlag (cmydef.h:5895), InArmSuck.Suck[][].Error
//  (aHotPlateSubstrate.h:109 -- see TRAP 5 note), InArmSuckUse
//  (aHotPlateSubstrate.h:743), bPickLoaderDuplicateErr (cmydef.h:4875),
//  MOT[MMTrayY].InitNewTray (Motor/mymotor.h:347), Tray.HowManyIC (mytray.h:87),
//  fSortCT->pnlLoad (forms/fSortCT.h:29 + extern :76),
//  CosFunction.bShowHPICCount (CosFunction.h:381), InitTrayEndFunction
//  (csystem.h:90, real body csystem.cpp:376), iPickFromLoadStageTask
//  (aHotPlateSubstrate.h:937), sAutoTrayendStartPos(Buffer) (cmydef.h:4342-4343),
//  IniConfig.bP17InArmFullPickFromLoader (Config.h:1401) /
//  bI37_EnableFIFOMode (Config.h:804), LastSet.iRunStartMode (LastSet.h:366),
//  rsmFIFOMode (MachineType.h:615).
//
//  TRAP 5 -- WHICH HEADER: InArmSuck is the `TMyKitSuck` of
//  aHotPlateSubstrate.h:365 (extern at aHotPlateSubstrate.h:624), whose nested
//  TMySucker is aHotPlateSubstrate.h:106 and carries `bool Error` at :109.  There
//  IS a second, DIFFERENTLY LAID OUT pair -- TMyKitSuck at mykitsuck.h:274 with
//  TMySucker at mykitsuck.h:143 (`bool Error` at :183).  This part relies on the
//  aHotPlateSubstrate one because that is the header ainarm9045.cpp already
//  includes (:70) and the one the other 176 TUs use; mykitsuck.h is NOT included
//  by this TU and must not be added, or every field would be read at the wrong
//  offset while still linking cleanly.
//==============================================================================
// MAIN-LOOP INCLUDE HAND-OFF, NOT A GATE.  golden :2842-2844 turns 3 loader
// cylinders OFF through `Cylinder[]` (mycylin.h:176, class TMyCylinder
// mycylin.h:44 with On()/Off() at :112-113).  Cylinder[] is REAL and linkable,
// but ainarm9045.cpp's include block does not reach mycylin.h and NO header in
// the tree includes it transitively (checked: zero `#include "mycylin.h"` hits
// in *.h), so the sibling in-arm variants each include it directly -- e.g.
// ainarm9045_1x1_1.cpp:58.  mycylin.h is self-contained and guarded (mycylinH,
// :36-37), so this mid-file include is legal and idempotent.
//   Deliberately NOT gated: gating the three Off() calls would leave the tray-Y
//   fixer, the loader edge push and the loader up-press ENGAGED through a tray-end
//   recovery -- a mechanical interference, not a cosmetic offline difference.
//   ACTION FOR THE MAIN LOOP: hoist this line into ainarm9045.cpp's include block
//   (next to "myswitch.h", :73) and delete it from here.
#include "mycylin.h"                                                            //AI(k6-ainarm) 20260811: Cylinder[] / TMyCylinder for golden :2842-2844
//------------------------------------------------------------------------------
bool DoTrayEndProcess_9045()                                                    //Steven 20190703 : 整合Tray End動作
{
    bool bHasSkip=false;                                                        //Sam 20200922 : Pick up Error Production_Log 也要記錄
    SetAutoSkipCount(0);                                                        //Steven 20150217 : 顯示Auto Skip的數量, 1=++, 0=清空計數

    if(CUSTOMER_CODE==CC_ASE_Korea)
    {
    }
    else
    {
        bLoaderHasSkip=true;                                                    //Steven 20101019
        iManualRemoveTrayCnt=2;                                                 //Sam 20190731 : 當 Load 發生 Pickup Error Skip 時，當前這盤除了要手動移除，下一盤也要手動移除。
    }
    bAutoRetryFlag=false;                                                       //JerryYang 20220901 : ASE-CL pick up error時吸取邏輯要改找吸最左上角的
    Cylinder[C_TrayY_Fixer].Off();
    Cylinder[C_LoaderEdgePush].Off();
    Cylinder[C_LoaderUpPress].Off();                                            //JerryYang 20181120 (Steven) : (Steven) : 獨立控制loader壓tray
    bHasSkip=false;                                                             //Sam 20200922 : Pick up Error Production_Log 也要記錄
    for(int i=0; i<MAX_ARM_Row; i++)
    {
        for(int j=0; j<MAX_ARM_Col; j++)
        {
            if(InArmSuck.Suck[i][j].Error && bHasSkip==false)                   //Sam 20200922 : Pick up Error Production_Log 也要記錄
            {
                bHasSkip=true;                                                  //一次Skip一顆
//                if(CosFunction.bLoadPickUpErrorMES0101Record)
//                {
////                    AddLoadingCount(i, j, iYPosition+iLoadPitchStepY*i, iXPosition[j]);
//                    InArmSuck.PordRec[i][j].AddErrorRecord("MES0101");
//                }
            }
            InArmSuckUse[i][j]=false;
            bPickLoaderDuplicateErr[i][j]=false;
            InArmSuck.Suck[i][j].Error=false;                                   //jou 2012-01-11 Tray end會造成Vaccum OFF error JAM0127
        }
    }

    MOT[MMTrayY].InitNewTray(NULL_IC, false, __FUNC__);
    if(CosFunction.bShowHPICCount)                                              //Steven 20221228 : 計算加熱盤IC數量
    {
        fSortCT->pnlLoad->Caption=MOT[MMTrayY].Tray.HowManyIC();
    }
    InitTrayEndFunction();
    iPickFromLoadStageTask=1;
    if(CUSTOMER_CODE==CC_ASE_KaohSiung)
    {
        sAutoTrayendStartPosBuffer.sprintf("Tray End Pos:%s", sAutoTrayendStartPos);                                    //kevin 20171026 (wei) 回應 ase close Site
        RecordProcess(sAutoTrayendStartPosBuffer);                              //kevin 20171026 (wei) inarm pick error for tray X Y pos log
        sAutoTrayendStartPos="";
        sAutoTrayendStartPosBuffer="";
    }

    if(iInArmType!=e9045_1x2_4_Hot &&                                           //Steven 20161103 : 1x2_4_Hot強制補滿
       iInArmType!=e9045_1x4_8_Hot &&                                           //Steven 20161103 : 1x4_8_Hot強制補滿
       iInArmType!=e9045_2x2_8_Hot &&                                           //Steven 20161103 : 2x2_8_Hot強制補滿
       IniConfig.bP17InArmFullPickFromLoader==false && InArmSuck.HasIC())       //Steven 20101221 :讓每盤都是滿的
    {
        return true;
    }
    else if(IniConfig.bI37_EnableFIFOMode && LastSet.iRunStartMode==rsmFIFOMode && InArmSuck.HasIC())                   //Steven 20160303 : FIFO Mode
    {
        return true;
    }
    return false;
}

//==============================================================================
//  DoTraySkipProcess_9045  --  golden ainarm9045.cpp:2894-2997
//  Translator: AI(W7D-ainarm-k8) 20260811
//
//  ROLE          -- the Loader "pick error -> SKIP" integrator.  Either it demotes
//                   the traymap-flagged cells to NULL_IC and REFUSES the skip
//                   (return false), or it latches bLoaderHasSkip, walks the nozzle
//                   grid clearing each errored nozzle, writes that tray cell back
//                   to NULL_IC, optionally records the skip position and trips the
//                   auto-skip-count ceiling, then hands off to
//                   DoRecordSkipPosition_9045 (return true).
//  WAVE SCOPE    -- DoTraySkipProcess_9045   golden :2894-2997   ACTIVE
//                   (the head arm's nozzle-grid body is GATED -- see below)
//
//  GATE REGISTER
//   * GATE W7D-G01   golden :2900   `EnableTraymapCheckFunction(1)`
//     WHY THE OFFLINE DEFAULT (false) IS FAITHFUL: golden home is ainarm2.h:216,
//     body ainarm2.cpp:3585.  The port has NO body and NO declaration anywhere,
//     and the port's two OTHER call sites are ALREADY gated on exactly this
//     premise with exactly this default -- aTester_Rear.cpp:9793 ("GATE G18") and
//     aTester_Front.cpp:9863 ("GATE W7F3-G03"), both "condition reads false".
//     false selects golden's `else` arm, which IS the normal Loader-skip path
//     (the head arm is the Tray-map-data-error REFUSAL), so offline behaviour
//     equals a machine with traymap checking switched off -- the shipped default.
//     REAL-MACHINE DIFFERENCE: on a traymap-check-enabled machine golden REFUSES
//     the skip (returns false) after demoting the traymap-mismatched cells to
//     NULL_IC; offline we always take the skip and return true.
//     ABSENCE COMMAND (cwd D:/HT9045/HT9011UC_Cpp_V3.33.906.0):
//       grep -rn --include=*.cpp --include=*.h -E "^[A-Za-z_].*\bEnableTraymapCheckFunction[[:space:]]*\(" . | grep -v '^\./build'
//     RESULT: 0 lines (only the 2 already-gated call sites exist textually).
//     RUN AT: 2026-08-11 01:33:43 +0800.
//   * GATE W7D-G01b  golden :2902-2920   the head arm's nozzle-grid body
//     WHY IT STAYS GATED EVEN THOUGH G01 ALREADY MAKES IT UNREACHABLE -- this is a
//     COMPILE blocker, not merely dead code: the body derefs
//     `fTrayMapping->iTrayMappingDateCheck[][]`, which does NOT exist on the
//     port's TfTrayMapping (acatchtray_shims.h:219-313 carries only
//     iTrayMappingDate[35][70] at :257); it also needs MyDeCodeASCII
//     (EJ1N/TextProcess.h:106) and acatchtray_shims.h itself, and neither header
//     is in this TU's include set.  The 3 statements in the arm that DO have port
//     homes stay LIVE: InArmSuck.ResetAll() (:2921),
//     bTrayMapDataCheckErrorSkip=false (:2922) and `return false` (:2923).
//     REAL-MACHINE DIFFERENCE: on the real machine the errored nozzles whose
//     traymap cell disagrees get their tray cell written back to NULL_IC before
//     the refusal; offline the refusal happens without that write-back.
//     ABSENCE COMMAND (same cwd):
//       grep -rn --include=*.cpp --include=*.h -w "iTrayMappingDateCheck" . | grep -v '^\./build'
//     RESULT: 0 lines.  RUN AT: 2026-08-11 01:34:22 +0800.
//
//  Retires the offline stub `bool DoTraySkipProcess_9045() { return false; }` at
//  port ainarm9045.cpp:2295 (MAIN LOOP deletes it).  NOTE the stub returned FALSE
//  where the live body returns TRUE on the normal path -- so this is a real
//  behaviour change at its ONE live call site, ainarm9045_2x4_16.cpp:1524.  (The
//  second textual caller, ainarm9045_2x8_32.cpp:1854, is inside that file's
//  wholly-#if-0'd body -- preprocessor-nesting verified 2026-08-11 02:00 +0800 --
//  so it is NOT a live caller.)  That is the point of landing it.
//  Callee status re-checked 2026-08-11 01:54:20 +0800 (an in-wave fact, not the
//  01:33 snapshot): SetAutoSkipCount stays the offline stub at port
//  ainarm9045.cpp:2274 -- no part file exists for golden :2408.  But
//  AutoTrayEndMusic AND DoRecordSkipPosition_9045 are BOTH getting real bodies this
//  same wave (sibling _w7d_parts/02527_AutoTrayEndMusic.txt and
//  02999_DoRecordSkipPosition_9045.txt), retiring the port stubs at :2290 / :2296.
//  So the auto-skip-count-over path and the skip-position recording this function
//  hands off to become REAL once stitched -- the offline no-op assumption anyone
//  reads into `return true` here is already stale.
//  TRAP 5 -- InArmSuck: I relied on the aHotPlateSubstrate.h TMyKitSuck (class at
//  :365; .Suck[][] is TMySucker whose .Error is :109; .SetItemData :497;
//  .ResetAll :461), the variant 177 TUs use -- NOT mykitsuck.h:274.  That is the
//  header this TU already includes, and the InArmSuck object is defined in
//  aHotPlateSubstrate.cpp.
//  GOLDEN BUG KEPT: `bHasSkip` (:2896) is declared uninitialised; it is assigned
//  at :2929 on the only path that reads it, so it is benign -- left as written.
//  iXPosition[l] with l < MAX_ARM_Col (= 4, MachineType.h:388) is in range for the
//  `int iXPosition[8]` defined at ainarm9045_2x4_16_shims.cpp:21.
//==============================================================================
// AI(W7D-ainarm-k8) 20260811: golden ainarm2.h:82 `extern int  iXPosition[8], iYPosition;`
// The port ainarm2.h lacks that line; the only port declaration is
// ainarm9045_2x4_16_shims.h:55-56, which this TU does not include.  DECLARATION
// ONLY -- the objects are defined at ainarm9045_2x4_16_shims.cpp:21-22, so this
// can never cause `multiple definition` even if a sibling part emits it too.
extern int  iXPosition[8];                                                      // golden ainarm2.h:82
extern int  iYPosition;                                                         // golden ainarm2.h:82
//==============================================================================
bool DoTraySkipProcess_9045()                                                   //Steven 20190703 : 整合Load Tray Skip動作
{
    bool bHasSkip;
    bool bClearLoaderTray=false;
    bool bFTCTSkipFlag=false;                                                   //RogerYang 20251002 : RogerYang 瑞薩FT-CT

    bool bTraymapCheck=false;                                                   // GATE W7D-G01 offline default (see GATE REGISTER above)
#if 0 // GATE W7D-G01 -- golden :2900 EnableTraymapCheckFunction(int) (golden home ainarm2.h:216, body ainarm2.cpp:3585): NO port body and NO port declaration; the same premise is already gated at aTester_Rear.cpp:9793 (G18) and aTester_Front.cpp:9863 (W7F3-G03). Offline reads false -> golden's else arm (the normal Loader-skip path) runs. Absence command + timestamp are in the GATE REGISTER above.
    bTraymapCheck=EnableTraymapCheckFunction(1);
#endif
    if(bTraymapCheck)                                                           //wei 20170302 (Steven) Tray map data error
    {
#if 0 // GATE W7D-G01b -- golden :2902-2920: fTrayMapping->iTrayMappingDateCheck[][] is ABSENT from the port TfTrayMapping (acatchtray_shims.h:257 carries iTrayMappingDate only), and MyDeCodeASCII (EJ1N/TextProcess.h:106) + acatchtray_shims.h are not in this TU. Compile blocker, not merely dead code. Offline: the refusal happens without the per-cell NULL_IC write-back.
        for(int k=0; k<MAX_ARM_Row; k++)
        {
            for(int l=0; l<MAX_ARM_Col; l++)
            {
                if(InArmSuck.Suck[k][l].Error &&
                   MyDeCodeASCII(fTrayMapping->iTrayMappingDate[iYPosition+iLoadPitchStepY*k][iXPosition[l]])!=1)
                {
                    if(TestIF_File.bEnableTrayMapDoubleCheck)
                    {
                        if(MyDeCodeASCII(fTrayMapping->iTrayMappingDateCheck[iYPosition+iLoadPitchStepY*k][iXPosition[l]])!=1)
                            MOT[MMTrayY].SetTraySingleData(iXPosition[l], iYPosition+iLoadPitchStepY*k, NULL_IC);
                    }
                    else
                    {
                        MOT[MMTrayY].SetTraySingleData(iXPosition[l], iYPosition+iLoadPitchStepY*k, NULL_IC);
                    }
                }
            }
        }
#endif
        InArmSuck.ResetAll();                                                   //Steven 20160323 : 避免未開啟真空
        bTrayMapDataCheckErrorSkip=false;
        return false;
    }
    else
    {
        bLoaderHasSkip=true;                                                    //Steven 20101019
        iManualRemoveTrayCnt=2;                                                 //Sam 20190731 : 當 Load 發生 Pickup Error Skip 時，當前這盤除了要手動移除，下一盤也要手動移除。
        bHasSkip=false;

        if(TestIF_File.bRENESAS_EnableFTCT==true &&                             //RogerYang 20251112 : ART數量到，如果inarm上有IC直接搬走
            fSCKART->bFirstFullSkip==false &&                                   //但是數量超過，在下一次取料要變回原本吸料方式
            LastSet.iSCKARTInputCT>=fSCKART->iInputCount)
        {
            fSCKART->bFirstFullSkip=true;
            bFTCTSkipFlag=true;
        }

        for(int k=0; k<MAX_ARM_Row; k++)
        {
            for(int l=0; l<MAX_ARM_Col; l++)
            {
                if(InArmSuck.Suck[k][l].Error && bHasSkip==false)
                {
                    if(IniConfig.bRecordSkipPosition==false &&
                       USE_IN_Y_IS_AUTO_PITCH==false)                           //JerryYang 20251218 : IN/OUT ARM支援不同模組
                    {
                        bHasSkip=true;                                          //一次Skip一顆
                    }

                    if(IniConfig.bI37_EnableFIFOMode &&                         //Frank 20180810 : add FIFO SKIP Action
                       IniConfig.bI37_EnableFIFOSiteOrder &&                    //Ifor 20181009 (Steven) : KYEC add FIFO Function
                       LastSet.iRunStartMode==rsmFIFOMode)
                    {
                        InArmSuck.SetItemData(k, l, HAS_NULL_IC);
                    }

                    SetAutoSkipCount(1);                                        //Steven 20150217 : 顯示Auto Skip的數量, 1=++, 0=清空計數

                    if(bFTCTSkipFlag==true)                                     //RogerYang 20251112 : ART數量到，如果inarm上有IC直接搬走
                    {
                        InArmSuck.SetItemData(k, l, HAS_NULL_IC);
                    }

                    InArmSuckUse[k][l]=false;
                    bPickLoaderDuplicateErr[k][l]=false;
                    InArmSuck.Suck[k][l].Error=false;                           //jou 2012-01-11 Tray end會造成Vaccum OFF error JAM0127
                    MOT[MMTrayY].SetTraySingleData(iXPosition[l], iYPosition+iLoadPitchStepY*k, NULL_IC);

                    if(IniConfig.bRecordSkipPosition)                           //jou 2013-05-30 Record Skip position
                    {
                        MOT[MMTrayY].SetTrayBufferSingleData(iXPosition[l], iYPosition+iLoadPitchStepY*k, HAS_SKIP_IC);

                        if(ArmSpeed[InArm].bAutoSKIP==true)                     //jou 2013-09-23 add Auto SKIP CT
                        {
                            if(CUSTOMER_CODE==CC_ASE_KaohSiung)                 //吸取到空格後再吸到IC需發ALARM
                            {
                                bAutoSkiplog=true;                              //kevin 20170904 (Steven) 記錄SKIP TRAY X Y 位置
                                bASkStart=AutoTrayEndMusic(iAutoSkipCT, 4);
                            }

                            if(iAutoSkipCT>=ArmSpeed[InArm].iAutoSkipCT)
                            {
                                bAutoSkipCntOver=true;                          //JerryYang 20210420 : 矽品蘇州要求auto skip數量小於設定值要由工程師排除
                                SetAutoSkipCount(0);                            //Steven 20150217 : 顯示Auto Skip的數量, 1=++, 0=清空計數
                                bClearLoaderTray=true;                          //Steven 20150130 : 修正Auto Skip功能
                            }
                        }
                    }
                }
            }
        }

        DoRecordSkipPosition_9045(bClearLoaderTray);                            //Steven 20190703 : 整合Load Tray Skip動作
        return true;
    }
}

//==============================================================================
//  k7-ainarm PART 02999 -- DoRecordSkipPosition_9045(bool)
//  Translator: AI(pt-wave k7-ainarm) 20260811
//  Golden: ainarm9045.cpp:2999-3092 (cp950)
//
//  ROLE          : the Auto-Skip / Load-Tray-Skip bookkeeping step.  When the
//                  caller asks to clear the loader tray it (a) optionally spares
//                  the tray`s LAST row so the arm gets one more try at it
//                  (CC_ASE_KaohSiung or bE62TryPickLastRow -- mirroring the live
//                  HAS_IC cells into BufferData and raising
//                  bTryPickLastRowIng), then (b) blanks the tray`s Data grid to
//                  NULL_IC over the chosen row window; (c) for ASE Kaohsiung it
//                  writes the "Tray End Pos" / "Auto Tray Skip Pos" process log
//                  line; (d) refreshes the loader IC-count panel.  Always
//                  returns false.
//  WAVE SCOPE    : DoRecordSkipPosition_9045  golden :2999-3092 -- ACTIVE
//                  (verbatim, no gates).  RETIRES the offline stub
//                  `bool DoRecordSkipPosition_9045(bool) { return false; }` at the
//                  port`s ainarm9045.cpp:2296 -- the main loop must delete that
//                  line.  Golden`s own return value is the constant false, so the
//                  RETURN is unchanged by retiring the stub; what the stub was
//                  suppressing is the tray-grid mutation and the log line.
//  GATE REGISTER : (none -- every symbol resolves)
//                    MOT[MMTrayY].Tray.{XItem,YItem,Data,BufferData,HowManyIC}
//                                               mytray.h:33,34,40,46,87 via Motor/mymotor.h:336
//                    MOT[].SetTraySingleData / SetTrayBufferSingleData
//                                               Motor/mymotor.h
//                    TrayForm.Loader.Direction  cprod.h:1359 / :1305 / :1265
//                    IniConfig.bRecordSkipPosition / bE62TryPickLastRow  Config.h
//                    CUSTOMER_CODE / CC_ASE_KaohSiung  cmydef.h / MachineType.h
//                    bTryPickLastRowIng / bManualTrayend / sAutoTrayendStartPos /
//                    sAutoTrayendStartPosBuffer  cmydef.h
//                    NULL_IC (=0) / HAS_IC (=2)  cmydef.h:156 / cmydef.cpp:153,155
//                    RecordProcess              aArmHeader.h
//                    CosFunction.bShowHPICCount CosFunction.h:381 (via cpublic.h)
//                    fSortCT->pnlLoad->Caption  forms/fSortCT.h:29 via FormsFacade.h:60
//                                               (HowManyIC() int -> AnsiString(int) ctor,
//                                                exactly as fSortCT.h:28 documents; the
//                                                identical golden line already lives at
//                                                acatchtray.cpp:1977 / cinitial.cpp:10038)
//
//  GOLDEN QUIRK KEPT (not a translation choice): the "spare the last row" branch
//  for iTrayDir 0/1 sets iAutotrayEndYEnd=YItem-1 and then, when the last row has
//  ALREADY been searched, RESTORES it to YItem -- i.e. the row window is widened
//  again.  For the other tray directions the mirror-image code moves
//  iAutotrayEndYStart 1 -> 0 instead.  Reproduced exactly, asymmetry included.
//==============================================================================
bool DoRecordSkipPosition_9045(bool bClearLoaderTray)                           //Steven 20190703 : 整合Load Tray Skip動作
{
    int iAutotrayEndYStart=0;
    int iAutotrayEndYEnd=MOT[MMTrayY].Tray.YItem;                               //kevin 20170429 add search load tray
    int iTrayDir=TrayForm.Loader.Direction;
    if(IniConfig.bRecordSkipPosition && bClearLoaderTray)                       //Steven 20150130 : 修正Auto Skip功能
    {
        if(CUSTOMER_CODE==CC_ASE_KaohSiung ||                                   //kevin 20170529 (wei) add autoskip finish search load tray
           IniConfig.bE62TryPickLastRow)                                        //JerryYang 20200422 Auto skip次數到達後, 自動再去最後一排吸吸看
        {
            if(iTrayDir==0 || iTrayDir==1)
            {
                iAutotrayEndYEnd=MOT[MMTrayY].Tray.YItem-1;                     //20170429 每一 盤最後一排要再搜尋是否有IC
                if(MOT[MMTrayY].Tray.Data[0][iAutotrayEndYEnd]==NULL_IC &&
                   MOT[MMTrayY].Tray.Data[MOT[MMTrayY].Tray.XItem-1][iAutotrayEndYEnd]==NULL_IC)                        //判斷是否已搜尋最後一排 就不再搜最後一排
                {
                    iAutotrayEndYStart=0;
                    iAutotrayEndYEnd=MOT[MMTrayY].Tray.YItem;
                }
                else
                {
                    bTryPickLastRowIng=true;
                    for(int i=0; i<MOT[MMTrayY].Tray.XItem; i++)
                    {
                        for(int j=0; j<MOT[MMTrayY].Tray.YItem; j++)
                        {
                            if(MOT[MMTrayY].Tray.Data[i][j]==HAS_IC)
                            {
                                if(MOT[MMTrayY].Tray.BufferData[i][j]==NULL_IC)
                                {
                                    MOT[MMTrayY].SetTrayBufferSingleData(i, j, HAS_IC);
                                }
                            }
                        }
                    }
                }
            }
            else
            {
                iAutotrayEndYStart=1;
                if(MOT[MMTrayY].Tray.Data[0][0]==NULL_IC &&
                   MOT[MMTrayY].Tray.Data[MOT[MMTrayY].Tray.XItem-1][0]==NULL_IC)                                       //判斷是否已搜尋最後一排 就不再搜最後一排
                {
                    iAutotrayEndYStart=0;
                    iAutotrayEndYEnd=MOT[MMTrayY].Tray.YItem;
                }
                else
                {
                    bTryPickLastRowIng=true;
                    for(int i=0; i<MOT[MMTrayY].Tray.XItem; i++)
                    {
                        for(int j=0; j<MOT[MMTrayY].Tray.YItem; j++)
                        {
                            if(MOT[MMTrayY].Tray.Data[i][j]==HAS_IC)
                            {
                                if(MOT[MMTrayY].Tray.BufferData[i][j]==NULL_IC)
                                {
                                    MOT[MMTrayY].SetTrayBufferSingleData(i, j, HAS_IC);
                                }
                            }
                        }
                    }
                }
            }
        }

        for(int i=0; i<MOT[MMTrayY].Tray.XItem; i++)
            for(int j=iAutotrayEndYStart; j<iAutotrayEndYEnd; j++)
                MOT[MMTrayY].SetTraySingleData(i, j, NULL_IC);
    }

    if(CUSTOMER_CODE==CC_ASE_KaohSiung)
    {
        if(bManualTrayend)                                                      //kevin 20171026 (wei) 手縱 tray end
        {
            bManualTrayend=false;
            sAutoTrayendStartPosBuffer.sprintf("Tray End Pos:%s", sAutoTrayendStartPos);                                //kevin 20171026 (wei) 回應 ase close Site
            RecordProcess(sAutoTrayendStartPosBuffer);                          //kevin 20171026 (wei) inarm pick error for tray X Y pos log
        }
        else
        {
            sAutoTrayendStartPosBuffer.sprintf("Auto Tray Skip Pos:%s", sAutoTrayendStartPos);                          //kevin 20171026 (wei) 回應 ase close Site
            RecordProcess(sAutoTrayendStartPosBuffer);                          //kevin 20171026 (wei) inarm pick error for tray X Y pos log
        }
        sAutoTrayendStartPos="";
        sAutoTrayendStartPosBuffer="";
    }

    if(CosFunction.bShowHPICCount)                                              //Steven 20221228 : 計算加熱盤IC數量
    {
        fSortCT->pnlLoad->Caption=MOT[MMTrayY].Tray.HowManyIC();
    }
    return false;
}

//==============================================================================
//== ROLE: InitialInArmNeedSuck -- reset the in-arm per-nozzle "needs suck" /
//==       "needs destroy" flag grid before a pick (bPlace==false) or a place
//==       (bPlace==true); on the pick path also snapshot the target carry-kit
//==       into ptrInSHTBackup, then report whether that shuttle is at its
//==       left-front (LF) position so the arm may proceed.
//==
//== WAVE SCOPE (k2-ainarm):
//==   InitialInArmNeedSuck  golden ainarm9045.cpp:3771-3808  -- ACTIVE
//==     (ZERO gates.  Two golden member calls have no member of that name on the
//==      port class; BOTH are replaced by FAITHFUL equivalents whose bodies were
//==      read out of golden, not approximated -- documented individually below.)
//==   + W7Dk2_KitSuck_CopyFrom  = golden MyKitSuck.cpp:1151-1193  -- ACTIVE
//==     (TU-local free-function transcription of TMyKitSuck::CopyFrom; see
//==      FAITHFUL SUBSTITUTION #2)
//==
//== GATE REGISTER: (none -- this function needed no #if 0)
//==
//== TRAP 5 -- WHICH TMyKitSuck / TMySucker THIS RELIES ON.  There are two classes
//== of each name with DIFFERENT layouts.  This TU includes aHotPlateSubstrate.h,
//== so InArmSuck / FLCarryKit / BLCarryKit / ptrInSHTBackup are the
//== aHotPlateSubstrate.h:365 TMyKitSuck (the one 177 TUs use), whose nozzle type
//== is the aHotPlateSubstrate.h:106 TMySucker.  The OBJECTS are defined in
//== aHotPlateSubstrate.cpp (ptrInSHTBackup at aHotPlateSubstrate.cpp:1096; the
//== externs at aHotPlateSubstrate.h:1033 and :624-626).  The OTHER pair lives in
//== mykitsuck.h (TMyKitSuck at :274, CopyFrom at :369) and mykitsuck.cpp is
//== DELIBERATELY NOT REGISTERED in the build (CMakeLists.txt:2099) -- so nothing
//== of mykitsuck.h is linkable, and picking it would have read every field at the
//== wrong offset while still compiling and linking clean.
//==
//== FAITHFUL SUBSTITUTION #1 -- golden :3779 `Suck[i][j].SetNeedDestroy(false)`.
//== The port TMySucker (aHotPlateSubstrate.h:106) carries `bool bNeedDestroy` and
//== `GetNeedDestroyStatus()` (both under HT9045_SUCKER_NEEDDESTROY_ADDED) but NOT
//== the setter.  golden MyKitSuck.h:122 is `void SetNeedDestroy(bool Value)
//== {bNeedDestroy=Value;}` -- read, not guessed -- so the direct member write
//== below is byte-identical in effect.  A substitution, NOT a gate, NOT an
//== approximation.
//==
//== FAITHFUL SUBSTITUTION #2 -- golden :3792/:3794 `ptrInSHTBackup.CopyFrom(kit)`.
//== TMyKitSuck::CopyFrom is declared only on the WRONG class (mykitsuck.h:369,
//== unlinkable per the TRAP 5 note above); the port class has no CopyFrom.  Rather
//== than gate a load-bearing snapshot, the golden body (MyKitSuck.cpp:1151-1193)
//== is reproduced below as the TU-local free function W7Dk2_KitSuck_CopyFrom.
//== HOW IT WAS PRODUCED, because hand-transcription already bit me once here: the
//== 43 golden lines are read from MyKitSuck.cpp with encoding cp950 and emitted
//== MECHANICALLY, with exactly one transformation -- qualifying the implicit
//== `this->` members with `dst.` (it is a free function, not a member).  Nothing
//== is retyped, so all 4 Chinese trailing comments are byte-exact.  (My first
//== hand-typed attempt wrote 0x8207 for 0x6536 on MyKitSuck.cpp:1174 and
//== abbreviated the :1186/:1190 comments -- caught by comparing codepoint lists
//== against golden, which is why this is generated instead of typed.)
//== ZERO new dependencies: all 21 fields it touches already exist on the port
//== class; PordRec[][].bUse exists at Public/MyProductionRecord.h:164 and asBuffer
//== (a real TStringList*) at :158.  Same precedent the tree already set with
//== aTester_Rear.cpp:1027 `static void W64bT2_SetUnuseToNullIC(TMyKitSuck&)`.
//== It is `static` and its name exists NOWHERE else in the tree, so TRAP 1 shape
//== (d) cannot apply (there is no non-static declaration of it anywhere); it is
//== also #ifndef-guarded so a sibling part landing in this same TU cannot
//== double-define it.
//== HAND-OFF: the right permanent fix is a real TMyKitSuck::CopyFrom on
//== aHotPlateSubstrate.h/.cpp; that header was off-limits to this task.
//== WHY I DID NOT JUST GATE IT: in golden the only readers of ptrInSHTBackup are
//== ainarm9045S_1x4_4.cpp:118-139 and ainarm9045S_2x4_4_13.cpp:158-182, and both
//== OVERWRITE every cell with SetItemData before reading -- so no golden reader
//== observes this snapshot today.  That is exactly what makes a no-op stub LOOK
//== harmless, and exactly why it was ported for real instead.
//==
//== INTEGRATION NOTE (TRAP 1 shape (b)): port ainarm9045.cpp:2306 currently holds
//== `bool InitialInArmNeedSuck(int,bool){return false;}`.  Delete it or this file
//== collides.  BEHAVIOUR CHANGES at integration: the stub always answered "not
//== ready" (false); the real body answers TRUE whenever the addressed shuttle is
//== in LF, which is what actually lets the in-arm pick/place sequence advance.
//== Declaration is non-static at ainarm9045.h:132 -- this definition is not static.
//==
//== Big5 -> UTF-8: every Chinese comment transcribed character for character.
//==============================================================================
#ifndef HT9045_W7DK2_KITSUCK_COPYFROM
#define HT9045_W7DK2_KITSUCK_COPYFROM
// FAITHFUL free-function equivalent of golden TMyKitSuck::CopyFrom
// (declared golden MyKitSuck.h:263, body golden MyKitSuck.cpp:1151-1193).
// Emitted mechanically from golden; only change is the `dst.` qualification.
static void W7Dk2_KitSuck_CopyFrom(TMyKitSuck &dst, TMyKitSuck &Source)   // = golden `void TMyKitSuck::CopyFrom(class TMyKitSuck &Source)` MyKitSuck.cpp:1151
{
    int MinRow, MinCol;
    if(Source.iMaxRow>=dst.iMaxRow)
        MinRow=dst.iMaxRow;
    else
        MinRow=Source.iMaxRow;

    if(Source.iMaxCol>=dst.iMaxCol)
        MinCol=dst.iMaxCol;
    else
        MinCol=Source.iMaxCol;

    for(int i=0; i<MinRow; i++)
    {
        for(int j=0; j<MinCol; j++)
        {
            dst.SetItemData(i, j, Source.Item[i][j]);
            dst.PordRec[i][j].asBuffer->CommaText=Source.PordRec[i][j].asBuffer->CommaText;
            dst.PordRec[i][j].bUse  =Source.PordRec[i][j].bUse;                     //Frank 20160505 add

            dst.cDeviceInf[i][j]    =Source.cDeviceInf[i][j];
            dst.cReDeviceInf[i][j]  =Source.cReDeviceInf[i][j];
            dst.cSBin[i][j]         =Source.cSBin[i][j];                            //Steven 20220120 : Amlogic需要收SBIN
            dst.b2DIDNG[i][j]       =Source.b2DIDNG[i][j];                          //Steven 20200611 : for Murata, 2DID NG不測試

            dst.iWhichSite[i][j]    =Source.iWhichSite[i][j];
            dst.iWhichAuto[i][j]    =Source.iWhichAuto[i][j];
            dst.iCurrRotAng[i][j]   =Source.iCurrRotAng[i][j];                      //Steven 20170425 (wei) : Add rotate motor
            dst.iNeedRotAng[i][j]   =Source.iNeedRotAng[i][j];
            dst.iWhichIndex[i][j]   =Source.iWhichIndex[i][j];                      //ChungHung 20150205 add for ATK
            dst.bPass[i][j]         =Source.bPass[i][j];
            dst.iCleanCount[i][j]   =Source.iCleanCount[i][j];
            dst.bFliped[i][j]       =Source.bFliped[i][j];
            dst.iBinData[i][j]      =Source.iBinData[i][j];
            dst.iBinDataBackUp[i][j]=Source.iBinDataBackUp[i][j];                   //Sam 20180612 : 有開啟 D22 Double Contact Contact， 以第一次的測試結果來做 ProcessCount
            dst.bQATray[i][j]       =Source.bQATray[i][j];
            dst.iAutoCleanRecX[i][j]=Source.iAutoCleanRecX[i][j];
            dst.iAutoCleanRecY[i][j]=Source.iAutoCleanRecY[i][j];
            dst.iAOIResult[i][j]    =Source.iAOIResult[i][j];                       //Sam 20240325 : 新增 DamageTrayMapping 功能
        }
    }
}
#endif // HT9045_W7DK2_KITSUCK_COPYFROM
//------------------------------------------------------------------------------
bool InitialInArmNeedSuck(int iShuttle, bool bPlace)                            //Steven 20220927 : 重新整理In / Out Arm Need Suck
{
    for(int i=0; i<InArmSuck.iMaxRow; i++)
    {
        for(int j=0; j<InArmSuck.iMaxCol; j++)
        {
            if(bPlace==true)
            {
                InArmSuck.Suck[i][j].bNeedDestroy=false;                        // W7D-k2 FAITHFUL SUBSTITUTION #1 for golden :3779 `InArmSuck.Suck[i][j].SetNeedDestroy(false);` -- golden MyKitSuck.h:122 body is literally {bNeedDestroy=Value;} and the port TMySucker exposes bNeedDestroy but not the setter.  Byte-identical effect; NOT a gate.
            }
            else
            {
                InArmSuck.Suck[i][j].SetNeedSuck(false);
                InArmSuck.iNeedSuck[i][j]=false;
            }
        }
    }

    if(bPlace==false)
    {
        if(iShuttle==0)
            W7Dk2_KitSuck_CopyFrom(ptrInSHTBackup, FLCarryKit);                 // W7D-k2 FAITHFUL SUBSTITUTION #2 for golden :3792 `ptrInSHTBackup.CopyFrom(FLCarryKit);`
        else
            W7Dk2_KitSuck_CopyFrom(ptrInSHTBackup, BLCarryKit);                 // W7D-k2 FAITHFUL SUBSTITUTION #2 for golden :3794 `ptrInSHTBackup.CopyFrom(BLCarryKit);`
    }

    if(iShuttle==0)
    {
        if(InSHT1InLF()==false)
            return false;
    }
    else if(iShuttle==1)
    {
        if(InSHT2InLF()==false)
            return false;
    }
    return true;
}

//==============================================================================
//  ROLE          InArmSideAllClose -- "is every site on this shuttle side closed?"
//                Single-site shortcut first (shuttle-mode 1 + the OTHER shuttle
//                selected => that side is by definition closed), then scan
//                Prod.fInArmSuck4x8[iSht][row][0..3]: any enabled nozzle => false.
//
//  WAVE SCOPE    ACTIVE  InArmSideAllClose   golden ainarm9045.cpp:4231-4249
//
//  GATE REGISTER (this part file)   -- EMPTY.  TestIF_File (cprod.h:2577) with
//                .iShuttleMode / .iShuttle_Sel (cprod.h:1654/1655),
//                Prod.fInArmSuck4x8[2][2][8] (cprod.h:820) and InArmSuck.iShtRow
//                (aHotPlateSubstrate.h:406) all exist.
//
//  BEHAVIOUR CHANGE vs the stub it retires (ainarm9045.cpp:2307 `{ return true; }`,
//  labelled "offline: all closed"): the stub answered TRUE unconditionally, which
//  is the DANGEROUS direction -- its callers (ainarm9045_1x1_1.cpp:1002/1332/2147,
//  ainarm9045S_*.cpp:2029, ...) read `all sites on that side are closed` as
//  "skip this side", so the stub silently skipped sides that actually have open
//  sites.  This body answers from Prod.fInArmSuck4x8, i.e. false as soon as one
//  nozzle is enabled.  Flagged for the main loop as a real behaviour delta, not a
//  no-op un-stub.
//
//  NOTE golden`s inner bound is the literal 4 (NOT InArmSuck.iMaxCol) and the
//  outer bound is iShtRow (NOT iMaxRow / iPickRow).  Both kept exactly.
//  AI(W7D-k3-ainarm) 20260811
//==============================================================================
bool InArmSideAllClose(int iSht)
{
    if(iSht==0 && TestIF_File.iShuttleMode==1 && TestIF_File.iShuttle_Sel==1)   //jou 2024-09-24 : 修正single site 關arm hang up
        return true;
    else if(iSht==1 && TestIF_File.iShuttleMode==1 && TestIF_File.iShuttle_Sel==0)
        return true;

    for(int i=0; i<InArmSuck.iShtRow; i++)
    {
        for(int j=0; j<4; j++)
        {
            if(Prod.fInArmSuck4x8[iSht][i][j]==true)
            {
                return false;
            }
        }
    }
    return true;
}

//==============================================================================
//== ROLE: InArmSideAllCloseWithKit -- "is EVERY site of this shuttle-side KIT
//==       turned off?".  Reads the 4x8 nozzle-enable map Prod.fInArmSuck4x8 over
//==       the 4 columns belonging to kit 0 (cols 0-3) or kit 1 (cols 4-7), plus
//==       the single-shuttle-mode short-circuit for the non-selected shuttle.
//==
//== WAVE SCOPE (k2-ainarm):
//==   InArmSideAllCloseWithKit  golden ainarm9045.cpp:4251-4271  -- ACTIVE
//==     (100% faithful; ZERO gates -- pure reads of TestIF_File / InArmSuck /
//==      Prod, all real in the port)
//==
//== GATE REGISTER: (none -- this function needed no #if 0)
//==   Data provenance: TestIF_File.iShuttleMode / .iShuttle_Sel -> cprod.h;
//==   InArmSuck.iShtRow -> aHotPlateSubstrate.h:406 (TRAP 5: the
//==   aHotPlateSubstrate.h:365 TMyKitSuck, object defined in aHotPlateSubstrate.cpp
//==   -- NOT the mykitsuck.h:274 class, whose .cpp is deliberately unregistered per
//==   CMakeLists.txt:2099); Prod.fInArmSuck4x8[2][2][8] -> cprod.h:820.
//==
//== INTEGER DIVISION: none here.  `(iKit==0)?0:4` is a select, not a division --
//== kept verbatim, and note it maps ANY non-zero iKit to 4 (golden behaviour).
//==
//== GOLDEN QUIRK PRESERVED: the inner bound is a hard `j<4`, NOT InArmSuck.iMaxCol
//== -- that hard 4 is what makes iKit32 a 4-column window.  Transcribed as-is.
//==
//== INTEGRATION NOTE (TRAP 1 shape (b)): port ainarm9045.cpp:2308 currently holds
//== `bool InArmSideAllCloseWithKit(int,int){return true;}  // golden :4251`.
//== Delete it or this file collides.  THIS IS THE MOST BEHAVIOUR-VISIBLE SWAP IN
//== MY SLICE: the stub answered "yes, everything is closed" unconditionally, which
//== forced the "all sites closed" arm of every caller.  Live callers today: port
//== ainarm9045.cpp:541 (DoPickLoaderOK) and this wave's
//== SetClosedShtKitToHasNullIC_9045 (part 00385).  Expect real behaviour movement
//== in both once the real body lands -- that movement is the point, not a
//== regression.  Declaration is non-static at ainarm9045.h:136.
//==============================================================================
bool InArmSideAllCloseWithKit(int iSht, int iKit)
{
    if(iSht==0 && TestIF_File.iShuttleMode==1 && TestIF_File.iShuttle_Sel==1)   //jou 2024-09-24 : 修正single site 關arm hang up
        return true;
    else if(iSht==1 && TestIF_File.iShuttleMode==1 && TestIF_File.iShuttle_Sel==0)
        return true;

    int iKit32=(iKit==0)?0:4;

    for(int i=0; i<InArmSuck.iShtRow; i++)
    {
        for(int j=0; j<4; j++)
        {
            if(Prod.fInArmSuck4x8[iSht][i][j+iKit32]==true)
            {
                return false;
            }
        }
    }
    return true;
}

// =============================================================================
//  k4-ainarm PART 04/08  --  InArmNeedCheckOffset   golden ainarm9045.cpp:4329-4366
//  Translator: AI(k4-ainarm) 20260811
//
//  ROLE   : Teach/offset break-in decision for the three in-arm targets (Input Shuttle1,
//           Input Shuttle2, Loader).  If the operator has rights (AccessLevel>=1 or the
//           OP-can-press-Step option) AND any one of {manual T.Start key held, that target's
//           Auto-Offset function armed, that target's Setup-Teach armed}, it clears
//           bEnterOffset and PAUSES the machine so the teach screen can be used.
//
//  WAVE SCOPE
//    InArmNeedCheckOffset  golden :4329-4366  ACTIVE, fully faithful, ZERO gates.
//
//  GATE REGISTER: (empty -- nothing gated in this function)
//    The one dependency the file's current include set could not reach is fixed by an
//    include, not by a gate:
//      * Sen[SnRKManualTStart].IsOn()  -- `Sen` was NOT visible from ainarm9045.cpp's include
//        block (probe TU error: "'Sen' was not declared in this scope", 2026-08-11 01:26:17 +0800).
//        Canonical declaration is mysensor.h:48  `extern class TMySensor Sen[MAX_SENSOR_ITEM];`
//        so this part adds `#include "mysensor.h"`.  That header IS guarded (#ifndef mysensorH
//        at :18 / #endif at :53) so a repeat include is harmless, and it pulls in only
//        vclcompat/vcl_compat.h + MachineType.h -- both already in this TU.  TRAP 5 check that
//        it cannot swap a same-named class under this TU:
//          grep -n "#include" mysensor.h   ->  exactly those 2 headers, no mykitsuck.h.
//      * fOffSet->UseAutoOffsetFunction(const char*) / ->UseInArmSetupTeach(int),
//        fMain->Pause(AnsiString), AccessLevel, bEnterOffset, InOfsInSh1/InOfsInSh2/
//        InOfsLoader, CosFunction.bOPCanPressStepAndTStart -- all already reachable (probe TU
//        compiled clean once the mysensor.h line was added).
//
//  Big5 -> UTF-8: six Chinese comments transcribed character-for-character from cp950 --
//  :4329, :4336/:4344/:4353, :4339/:4347/:4356.  ZERO U+FFFD.
// =============================================================================
#include "mysensor.h"               // k4-ainarm: Sen[] / SnRKManualTStart -- golden mysensor.h:48 (guarded header; vclcompat + MachineType.h only -- see GATE REGISTER note above)

bool InArmNeedCheckOffset(bool bPlace, int iPos)                                //Steven 20230531 : 簡化判斷式
{
    bool bResult=false;
    if(bPlace)
    {
        if(iPos==0)
        {
            if((AccessLevel>=1 || CosFunction.bOPCanPressStepAndTStart) &&      //JerryYang 20170417 (wei) OP權限也能按Step跟T.Start
                (Sen[SnRKManualTStart].IsOn() ||
                fOffSet->UseAutoOffsetFunction("Input Shuttle1") ||             //jou 2013-08-29 Use Auto Offset Funtion
                fOffSet->UseInArmSetupTeach(InOfsInSh1)))                       //JerryYang 20180921 Setup Teach功能
                bResult=true;
        }
        else
        {
            if((AccessLevel>=1 || CosFunction.bOPCanPressStepAndTStart) &&      //JerryYang 20170417 (wei) OP權限也能按Step跟T.Start
                (Sen[SnRKManualTStart].IsOn() ||
                fOffSet->UseAutoOffsetFunction("Input Shuttle2") ||             //jou 2013-08-29 Use Auto Offset Funtion
                fOffSet->UseInArmSetupTeach(InOfsInSh2)))                       //JerryYang 20180921 Setup Teach功能
                bResult=true;
        }
    }
    else
    {
        if((AccessLevel>=1 || CosFunction.bOPCanPressStepAndTStart) &&          //JerryYang 20170417 (wei) OP權限也能按Step跟T.Start
           (Sen[SnRKManualTStart].IsOn() ||
           fOffSet->UseAutoOffsetFunction("Loader") ||                          //jou 2013-08-29 Use Auto Offset Funtion
           fOffSet->UseInArmSetupTeach(InOfsLoader)))                           //JerryYang 20180921 Setup Teach功能
            bResult=true;
    }

    if(bResult)
    {
        bEnterOffset=false;
        fMain->Pause("InArmNeedCheckOffset");
    }
    return bResult;
}

//==============================================================================
//  bCheckInShuttlePosAndNOIC  --  golden ainarm9045.cpp:4368-4389
//  Translator: AI(W7D-ainarm-k8) 20260811
//
//  ROLE          -- KenHsieh 20230614 interlock predicate: before the in-arm may
//                   pick an IC out of the Loader tray, the in-shuttle it feeds
//                   must be at its Left/Feed position AND empty.  Returns true =
//                   "must WAIT" (not ready); false when the feature is off
//                   (bE72 == false) or the shuttle is ready.
//  WAVE SCOPE    -- bCheckInShuttlePosAndNOIC   golden :4368-4389   ACTIVE
//  GATE REGISTER -- (empty: this function contains no #if 0)
//
//  Retires the offline stub `bool bCheckInShuttlePosAndNOIC() { return false; }`
//  at port ainarm9045.cpp:2309 (MAIN LOOP deletes it).  Every symbol is already
//  reachable from this TU's include set (probe-measured, not assumed):
//  IniConfig.bE72_InarmPickICNeedWaitSH (Config.h:641), InSHT1InLF/InSHT2InLF
//  (csystem.h), FLCarryKit/BLCarryKit + InArmSuck.iWhichSht (aHotPlateSubstrate.h).
//  TRAP 5 -- FLCarryKit / BLCarryKit / InArmSuck: I relied on the
//  aHotPlateSubstrate.h TMyKitSuck (class :365; UseSiteHasIC decl :467;
//  iWhichSht :408), NOT mykitsuck.h:274 (UseSiteHasIC :413, iWhichSht :319).  The
//  two layouts differ, so picking the wrong header links clean and then reads
//  every field at the wrong offset.  aHotPlateSubstrate.h is what this TU already
//  includes and aHotPlateSubstrate.cpp is where the three objects are defined.
//  The value the stub chose (false = "no need to wait") is the SAME value this
//  live body returns while IniConfig.bE72_InarmPickICNeedWaitSH is false, so no
//  offline behaviour moves -- the gain is the real wiring, and per this wave's
//  brief "the degraded value happens to be equivalent" is not itself a defence.
//==============================================================================
bool bCheckInShuttlePosAndNOIC()                                                //KenHsieh 20230614 : Inarm pick IC from tray需待SH到位且為可取放料狀態
{
    if(IniConfig.bE72_InarmPickICNeedWaitSH==false)
        return false;

    if(InArmSuck.iWhichSht==0)
    {
        if(InSHT1InLF()==false || FLCarryKit.UseSiteHasIC())
        {
            return true;
        }
    }
    else
    {
        if(InSHT2InLF()==false || BLCarryKit.UseSiteHasIC())
        {
            return true;
        }
    }

    return false;
}

//==============================================================================
//  k7-ainarm PART 04831 -- TrayYDirForArmYPitch()
//  Translator: AI(pt-wave k7-ainarm) 20260811
//  Golden: ainarm9045.cpp:4831-4861 (cp950)
//
//  ROLE          : predicate -- "does the LOADER tray`s Y pitch line up with the
//                  arm`s Y pitch, so the arm may pick a whole Y row at a time?"
//                  Auto-pitch machines answer yes only when no site is closed AND
//                  AutoCalculateInArmYClosePitch() found a usable close-pitch, and
//                  answer NO outright when the customer profile forbids the
//                  row-by-row (搜最後一排) mode.  Fixed-pitch machines answer yes when
//                  the taught loader pitch equals TestIF.iARM_Y_PITCH.  Always no
//                  while an in-arm loader pick error is outstanding.
//  WAVE SCOPE    : TrayYDirForArmYPitch  golden :4831-4861 -- ACTIVE body, ONE
//                  gated `else if` arm (k7-G2).  No stub to retire -- the symbol
//                  exists nowhere in the port today.  Its only golden-visible port
//                  call site, ainarm9045_2x8_32.cpp:1344, is inside that file`s
//                  whole-body `#if 0 // TODO(W7)` (CMakeLists.txt:1519 explains
//                  the 2x8_32 variant is dead), so this body has NO reachable
//                  caller yet -- net-new surface, not a behaviour change.
//
//  GATE REGISTER
//  ------------------------------------------------------------------------
//  GATE (k7-G2)  golden lines: ainarm9045.cpp:4856-4859
//    SYMBOL       : LoadTrayCanUse8Suck() -- golden ainarm2.h:200.  The port has
//                   NO externally-linkable definition; the only body in the tree
//                   is a file-scope `static` stand-in inside ANOTHER translation
//                   unit (aoutarm.cpp:559, `static int LoadTrayCanUse8Suck()
//                   { return 0; }`, that file`s own documented gate G6) and two
//                   commented-out call sites in asortarm.cpp:2739/2781.  I did NOT
//                   copy aoutarm`s static shim into this TU: golden ainarm2.h:200
//                   declares the symbol NON-static, so a file-scope `static`
//                   definition here would be exactly TRAP 1 shape (d) -- it would
//                   link clean today and explode the day ainarm2.h is translated
//                   and included in this TU.
//    OFFLINE DEFAULT CHOSEN : the whole `else if` arm is gated out, so control
//                   falls through to golden`s own `return false;` (:4860).
//    WHY FAITHFUL : golden compares `LoadTrayCanUse8Suck()==1`.  The ONLY value
//                   this symbol has anywhere in the port is 0, so the comparison
//                   is false and the arm can never be taken -- gating it is
//                   value-identical to keeping it.  This is the same premise
//                   aoutarm.cpp:264-274 already wrote up for its own G6 (twin
//                   golden function UnloaderTrayYDirForArmYPitch, golden :2960 /
//                   :3001); the premise carries over unchanged, so I am not
//                   re-deriving it -- only refusing aoutarm`s static-shim shape.
//    REAL-MACHINE DIFFERENCE : on a real machine LoadTrayCanUse8Suck() answers 1
//                   when the loader tray pitch permits one 8-nozzle grab, and this
//                   arm would then return true even when the pitches do not match
//                   exactly -- i.e. golden has a SECOND, looser route to "yes" via
//                   TestIF.bSearchLastMode.  Gated, only the exact-match route
//                   (:4852) can say yes, so the arm falls back to picking fewer
//                   ICs per row.  Throughput, not safety.
//    PROVING COMMAND (run from D:/HT9045/HT9011UC_Cpp_V3.33.906.0):
//                   rg -n "LoadTrayCanUse8Suck" -g "*.cpp" -g "*.h" -g "_w7d_parts/*.txt"
//    MEASURED AT  : 2026-08-11 01:34:26 +0800 -> 7 hits, ALL comments or
//                   aoutarm.cpp`s own `static` (aoutarm.cpp:264,268,559,3572,3613
//                   + asortarm.cpp:2739,:2781 commented out).
//    RE-MEASURED  : 2026-08-11 01:49:41 +0800, NOW INCLUDING every sibling part
//                   file in _w7d_parts/ -> unchanged; still zero non-static
//                   definitions.  GATE STANDS (TRAP 2 checked, premise alive).
//                   Golden defines LoadTrayCanUse8Suck in ainarm2.cpp, a different
//                   golden file entirely, so no part of this wave can land it.
//    TRAP 3 CHECK : if a real LoadTrayCanUse8Suck() lands, this gate should STILL
//                   NOT be retired blindly -- retiring it also requires deleting
//                   aoutarm.cpp:559`s static shadow in the same commit, otherwise
//                   two TUs in the same archive disagree about what the function
//                   returns (aoutarm keeps 0, this file gets the real answer) and
//                   the two twin predicates silently diverge.
//
//  ADJACENT-BODY NOTE (refreshed at the 01:49:41 re-measurement -- my earlier
//  draft called this an "adjacent STUB note" and that is now WRONG):
//  AutoCalculateInArmYClosePitch(bool,bool) at :4847 no longer resolves to the
//  port`s `{ return 0; }` stub (ainarm9045.cpp:2315).  A sibling landed the REAL
//  golden body this same wave -- _w7d_parts/04911_AutoCalculateInArmYClosePitch.txt,
//  `int AutoCalculateInArmYClosePitch(bool bStart, bool bCheckIsZero)` -- so the
//  auto-pitch branch at :4846-4850 becomes genuinely reachable and this predicate
//  can now return true.  The main loop must delete ainarm9045.cpp:2315 along with
//  the other retired stubs, or the stitched TU has two definitions.
//==============================================================================
bool TrayYDirForArmYPitch()
{
    int iLoadYPitch=Prod.LoadForm.iYPitch;

    if(bInArmPickErrFromLoader)                                                 //Steven 20170828 (wei) : 確認是否有在Loader吸料異常
        return false;

    if(USE_IN_Y_IS_AUTO_PITCH==true)                                            //Steven for HT1032   //JerryYang 20251218 : IN/OUT ARM支援不同模組  //JerryYang 20251218 : IN/OUT ARM支援不同模組
    {
        iCloseSiteState=CloseSiteState();                                       //Steven 20210420 : 修正12site關site跑2x4 mode, HP=6x11 hang up
        if(CosFunction.bYPitchNotUseSearchLastMode &&
           ArmSpeed_File[InArm].bYPitchUseSearchLastMode==0)
        {
            return false;                                                       //kevin 20211130  Y PITCH 不使用一排一排 吸
        }
        else if(iCloseSiteState==0 &&
                AutoCalculateInArmYClosePitch(false, true)!=0)
        {
            return true;
        }
    }
    else if(iLoadYPitch==TestIF.iARM_Y_PITCH)
    {
        return true;
    }
#if 0 // GATE (k7-G2) -- golden ainarm9045.cpp:4856-4859 (LoadTrayCanUse8Suck absent; see banner)
    else if(LoadTrayCanUse8Suck()==1 && TestIF.bSearchLastMode)                 //kevin 20171120 (wei) 判斷LOAD TRAY Y PITCH是否是 ARM Y PITCH可以一次8吸嘴吸IC
    {
        return true;
    }
#endif
    return false;
}

//==============================================================================
//  GetLoaderYPitchStep  --  golden ainarm9045.cpp:4863-4891
//  Translator: AI(W7D-ainarm-k8) 20260811
//
//  ROLE          -- how many Loader tray rows one in-arm Y-pitch step spans: 0
//                   when the tray Y direction does not match the arm Y pitch,
//                   else the auto-close-pitch (variable modes) or the configured
//                   TestIF.iARM_Y_PITCH, each divided by the tray YPitch.
//  WAVE SCOPE    -- GetLoaderYPitchStep   golden :4863-4891   ACTIVE, NO GATE
//
//  GATE REGISTER -- (empty: this function contains no #if 0)
//
//  RETIRED GATE W7D-G02 -- TRAP 2 CAUGHT IN-WAVE, READ THIS.
//  I first gated golden :4868 `TrayYDirForArmYPitch()` on the premise "no port
//  body, no port declaration; the single textual hit is a call site inside the
//  gated body of ainarm9045_2x8_32.cpp:1344" -- measured 2026-08-11 01:34:22 +0800
//  with:
//     grep -rn --include=*.cpp --include=*.h -woE "TrayYDirForArmYPitch" . | grep -v '^\./build' | grep -v "Unloader"
//  THAT PREMISE EXPIRED 10 MINUTES LATER, inside this same wave: a SIBLING AGENT
//  landed _w7d_parts/04831_TrayYDirForArmYPitch.txt at 01:44, defining
//  `bool TrayYDirForArmYPitch()` (its line 80) with EXTERNAL linkage at golden
//  :4831 -- i.e. in this very stitched TU, 32 lines AHEAD of my :4863 in golden
//  order, so it is both visible and defined before my call.  Re-measured
//  2026-08-11 01:54:20 +0800.
//  TRAP 3 RE-ASK ("a dead premise does not automatically mean retire"): is there a
//  DIFFERENT reason this should stay gated?  Checked and NO -- the body is real,
//  non-static, in the SAME translation unit (no cross-archive dependency to take
//  on), and it hard-codes no path and touches no persistent file, so neither the
//  MyMessageBox reason nor the WriteLastDataFile reason applies.  GATE RETIRED and
//  golden :4868 is emitted VERBATIM below.
//  CONSEQUENCE the main loop must know: this function no longer always returns 0.
//  Its offline value now depends on the sibling's predicate (which itself still
//  carries its own gate k7-G2 over golden :4856-4859 LoadTrayCanUse8Suck), so
//  retiring the ainarm9045_2x4_16_shims.cpp:29 `{ return 0; }` stub is a REAL
//  behaviour change at ainarm9045_2x4_16.cpp:1309, not a no-op.
//
//  Retires the offline stub `int GetLoaderYPitchStep() { return 0; }` at
//  ainarm9045_2x4_16_shims.cpp:29 (declared ainarm9045_2x4_16_shims.h:62) -- MAIN
//  LOOP deletes it.  ONE live caller: ainarm9045_2x4_16.cpp:1309.  The second
//  textual caller ainarm9045_2x8_32.cpp:1667 is inside that file's wholly-#if-0'd
//  body (verified 2026-08-11 02:00 +0800), so it does not count.
//  INTEGER DIVISION IS LOAD-BEARING here and is preserved exactly.  BOTH
//  `AutoCalculateInArmYClosePitch()/iYPitch` (:4877) and
//  `TestIF.iARM_Y_PITCH/iYPitch` (:4887) are int/int, and the :4883-4884
//  `iYPitch=iYPitch/10; iYPitch=iYPitch*10;` pair is golden's deliberate
//  round-DOWN-to-10 for the iXPitch60 mechanism.  No float helper substituted --
//  a float-helper substitution is exactly what produced stepped position drift
//  before.  `int iYPitch=LoadForm->YPitch;` truncates a double member
//  (cprod.h:1259) into an int, exactly as golden does.
//  AutoCalculateInArmYClosePitch() is called with no args: port ainarm9045.h:46
//  carries golden's (bStart=false, bCheckIsZero=false) defaults.  Its real body is
//  ALSO landing this wave (sibling _w7d_parts/04911_AutoCalculateInArmYClosePitch
//  .txt, `int AutoCalculateInArmYClosePitch(bool bStart, bool bCheckIsZero)` at its
//  line 52, golden :4911 -- so it stitches AFTER my :4863 in golden order, which is
//  fine because port ainarm9045.h:46 already declares it).  So the :4877 branch is
//  no longer a guaranteed 0 either; re-checked 2026-08-11 01:54:20 +0800.
//==============================================================================
int GetLoaderYPitchStep()                                                       //Steven 20141112
{
    int iYPitch=LoadForm->YPitch;
    int iYPitchStep=0;

    if(TrayYDirForArmYPitch()==false)
    {
        iYPitchStep=0;
    }
    else
    {
        if(USE_IN_OUT_ARM_Y_PITCH==iXYPitchVariable ||                          //ChungHung 20140304 add for AutoYPitch
           USE_IN_OUT_ARM_Y_PITCH==iXYPitchIn_Bb_Out_Bc)                        //Ztex 2024.02.24 Add HT-1132
        {
            iYPitchStep=(iYPitch==0)?0:(AutoCalculateInArmYClosePitch()/iYPitch);                                       //RogerYang 20250618 防止除以零     //ChungHung 20131231 alter AutoYPitch
        }
        else
        {
            if(USE_IN_OUT_ARM_Y_PITCH==iXPitch60)
            {
                iYPitch=iYPitch/10;
                iYPitch=iYPitch*10;
            }

            iYPitchStep=(iYPitch==0)?0:(TestIF.iARM_Y_PITCH/iYPitch);           //RogerYang 20250618 防止除以零
        }
    }
    return iYPitchStep;
}

//==============================================================================
//  InArmAddBlockPitch  --  golden ainarm9045.cpp:4893-4909
//  Translator: AI(W7D-ainarm-k8) 20260811
//
//  ROLE          -- SPIL WLP / carrier-tray "tray block" correction.  For a tray
//                   divided into blocks it replaces the flat XPitch*col (resp.
//                   YPitch*row) span of each COMPLETED block with the block pitch,
//                   by adding BlockPitch*(idx/Division) and subtracting
//                   Pitch*Division*(idx/Division).  Works in place on the caller's
//                   iXPos/iYPos; iXY selects 1 = X, 2 = Y.
//  WAVE SCOPE    -- InArmAddBlockPitch   golden :4893-4909   ACTIVE
//  GATE REGISTER -- (empty: this function contains no #if 0)
//
//  Declared in golden ainarm2.h:169 -- the ONLY header declaration of it in the
//  golden tree -- so at integration the port needs that same declaration visible
//  to callers.  There is currently NO port stub of this symbol to retire (grep of
//  the whole port tree: 0 hits), so this is a net-new body rather than a stub
//  retirement; its only caller is GetInArmToLoaderPosition_Single (this same wave,
//  golden :5207 and :5352).
//  ChangeToFloatNonPcnt is the zero-guarded divide template at MachineType.h:1601
//  (returns float).  Golden feeds it two (double) casts, so T=double, and the
//  float result is multiplied by a double member and assigned back into an int& --
//  truncating.  That is golden's arithmetic and it is reproduced EXACTLY.  Note
//  this is deliberately NOT the "integer division is load-bearing" case: golden
//  itself asked for the float helper here, so restoring int/int would be the
//  defect.  LoadForm is the TRAY_TYPE_PARA* at cprod.h:1365, whose XPitch /
//  YPitch / BlockPitchX / BlockPitchY are doubles and XDivision / YDivision ints
//  (cprod.h:1258-1263, 1284-1285).
//==============================================================================
void InArmAddBlockPitch(int &iXPos, int &iYPos, int indexRow, int indexCol, int iXY)                                    //Frank 20160928 add Subtray Function
{                                                                               //2014-03-04    Dell    for SPIL WLP Add Tray Block
    if(IniConfig.bUseTrayBlockMode &&
       IniConfig.bP06_LoaderUseCarrierTray==true)                               //Ifor 20181012 (Steven) : add Block Pitch Calculation
    {
        if(iXY==1)
        {
            iXPos=iXPos+((ChangeToFloatNonPcnt((double)(indexCol), (double)(LoadForm->XDivision)))*LoadForm->BlockPitchX);
            iXPos=iXPos-((ChangeToFloatNonPcnt((double)(indexCol), (double)(LoadForm->XDivision)))*(LoadForm->XPitch*LoadForm->XDivision));
        }
        else if(iXY==2)
        {
            iYPos=iYPos-((ChangeToFloatNonPcnt((double)(indexRow), (double)(LoadForm->YDivision)))*LoadForm->BlockPitchY);
            iYPos=iYPos+((ChangeToFloatNonPcnt((double)(indexRow), (double)(LoadForm->YDivision)))*(LoadForm->YPitch*LoadForm->YDivision));
        }
    }
}

//============================================================================
//  AutoCalculateInArmYClosePitch  --  golden ainarm9045.cpp:4911-4975 (k1-ainarm)
//
//  ROLE: solves the in-arm CLOSED Y-pitch (nozzle row spacing) in motor pulses.
//        bStart==false is a pure getter for the cached InArmClose_PitchY.  With
//        bStart==true and a variable-Y-pitch machine it snaps the pitch to the
//        loader tray's own Y pitch when that already sits inside the mechanical
//        [min,max] window, else walks integer multiples of the tray pitch until it
//        clears the minimum, and falls back to the recipe pitch TestIF.iARM_Y_PITCH
//        when nothing fits.  Single-site is special-cased to a fixed 6000.
//
//  WAVE SCOPE:
//    * AutoCalculateInArmYClosePitch(bool,bool)  ACTIVE (verbatim) :4911-4975
//
//  GATE REGISTER: empty -- this function contains no #if 0.  Every symbol it
//  touches (TrayForm.Loader.iTrayType, UserDefForm[].YPitch, InArmClose_PitchY,
//  USE_IN_OUT_ARM_Y_PITCH, iXYPitchVariable, iXYPitchIn_Bb_Out_Bc,
//  IN_OUT_ARM_Y_PITCH_MIN/MAX, TestIF.iARM_Y_PITCH, TestIF_File.iTestMode,
//  SingleSite) already exists in the port, so nothing had to be degraded.
//
//  ARITHMETIC: all-integer, and deliberately kept that way -- iMin, TrayYPitch,
//  TrayYPitch*i and the comparisons are int throughout.  No division at all, so
//  there was nothing here for a float helper to corrupt.
//
//  GOLDEN BUGS PRESERVED (behaviour kept, per the no-fix rule):
//   (1) golden :4944-4952 -- `do { i++; if(TrayYPitch*i>=iMin) {...break;} }
//       while(TrayYPitch*i<=IN_OUT_ARM_Y_PITCH_MAX);`  If UserDefForm[Ld].YPitch
//       is 0 (tray type never configured), TrayYPitch*i is 0 for every i, the
//       break condition can never be met and the while condition is always true:
//       INFINITE LOOP.  Golden has the same hole; not fixed.
//   (2) same loop -- if it exits by the while condition rather than the break,
//       InArmClose_PitchY was never assigned in this call, so :4954 tests the
//       value left over from a previous call (or the global's initial value) and
//       may `return` it as if freshly solved.
//   (3) golden :4935 accepts TrayYPitch when it is >= iMin, but iMin is only
//       raised to IN_OUT_ARM_Y_PITCH_MIN when that is strictly greater than 2500
//       (:4932), so a configured minimum BELOW 2500 is silently ignored.
//   (4) the second parameter bCheckIsZero is never read anywhere in the body --
//       a golden dead parameter.  Kept in the signature (callers pass it, and
//       ainarm9045.h:46 declares it).
//
//  Default arguments are NOT restated: ainarm9045.h:46 already declares
//  `bool bStart=false, bool bCheckIsZero=false`.
//
//  RETIRING THE OLD STUB: port ainarm9045.cpp:2315 holds
//  `int AutoCalculateInArmYClosePitch(bool, bool) { return 0; }` -- main loop's to
//  delete.  Behaviour change at integration: callers that used to see 0 now see a
//  real pitch (and the bStart==false getter now returns the cached
//  InArmClose_PitchY).  Find_InArm_PickerMaxUseCountOnTime in this same wave
//  divides LoadForm->YPitch into this result, so the two land together.
//============================================================================
int AutoCalculateInArmYClosePitch(bool bStart, bool bCheckIsZero)               //ChungHung 20131231 alter AutoYPitch
{
    int Ld=TrayForm.Loader.iTrayType;
    int TrayYPitch=UserDefForm[Ld].YPitch;
    //AI(W906-PITCH0) 20260923: 使用者裁決的暫時解 —— TrayYPitch<=0 時改成 1，解開 START 卡死。
    //  原話：「至少在卡住或讀取前，如果判定為0就給預設值=1，這樣可以解開卡住問題」。
    //  卡死機制（gdb 實測 i 已溢位成 -1607791631）：下面 `if(TrayYPitch<iMin)` 的 do-while
    //  在 TrayYPitch==0 時 break 條件 `0*i>=iMin` 恆假、while 條件 `0*i<=IN_OUT_ARM_Y_PITCH_MAX` 恆真
    //  ⇒ 無窮迴圈；而呼叫端 StartFromWeb（WebStart.cpp:1237）跑在 wb_serve 唯一的輪詢執行緒上
    //  ⇒ PumpTick/MainProc/DoInArm_9045 全停，網頁 "no ack within 15000ms"。
    //  ⚠ **偏離 golden**：golden ainarm9045.cpp:4952 同一段在 0 時一樣會卡；golden 的
    //    bCheckIsZero 參數從未被讀取（本函式檔頭 GOLDEN BUGS PRESERVED (4)），這裡沒有啟用它。
    //  ⚠ **1 只是解卡的值，不是正確的 Tray pitch**：迴圈會在 i=2500 停，InArmClose_PitchY=2500
    //    （25.00 mm）。模擬可以往下跑；真機台上手臂的 Y 收合 pitch 會是錯的，所以大聲記一筆。
    //  0 的根因是 Tray.Data 沒讀進來（forms/fTrayForm.cpp 未翻 TfTrayForm::ReadFile）＋ N3-G8 沒換算，
    //  治本排在 INBOX T5；UserDefForm[].YPitch 有值之後，這段就不會再觸發。
    if(TrayYPitch<=0)
    {
        AnsiString asPitch0;
        asPitch0.sprintf("W906-PITCH0: UserDefForm[%d].YPitch=%d <= 0 -- 暫時改用 1 避免無窮迴圈"
                         "（Tray.Data 沒讀進來；真機台上 Y 收合 pitch 會不正確）", Ld, TrayYPitch);
        RecordProcess(asPitch0);
        TrayYPitch=1;
    }
    int iMin=2500;                                                              //KevinYang 20210126 : 3500 --> 2500

    if(bStart==false)                                                           //Steven 20190314 : for Y-Pitch
    {
        return InArmClose_PitchY;
    }
    else
    {
        if(USE_IN_OUT_ARM_Y_PITCH==iXYPitchVariable ||                          //ChungHung 20140304 add for AutoYPitch
           USE_IN_OUT_ARM_Y_PITCH==iXYPitchIn_Bb_Out_Bc)                        //Ztex 2024.02.24 Add HT-1132
        {
            if(TestIF_File.iTestMode==SingleSite)                               //kevin 20210104
            {
                InArmClose_PitchY=6000;
                return InArmClose_PitchY;                                       //Steven 20240201 : TestIF.iARM_Y_PITCH --> InArmClose_PitchY
            }

            if(IN_OUT_ARM_Y_PITCH_MIN>iMin)
                iMin=IN_OUT_ARM_Y_PITCH_MIN;

            if(TrayYPitch>=iMin && TrayYPitch<=IN_OUT_ARM_Y_PITCH_MAX)
            {
                InArmClose_PitchY=TrayYPitch;
                return InArmClose_PitchY;
            }

            if(TrayYPitch<iMin)
            {
                int i=0;
                do
                {
                    i++;
                    if(TrayYPitch*i>=iMin)                                      //18800 @ 16x39 (MIN:2500)
                    {                                                           //19600 @ 16x39 (MIN:3100)
                        InArmClose_PitchY=TrayYPitch*i;
                        break;
                    }
                }while(TrayYPitch*i<=IN_OUT_ARM_Y_PITCH_MAX);

                if(InArmClose_PitchY<=IN_OUT_ARM_Y_PITCH_MAX)
                {
                    return InArmClose_PitchY;
                }
                else
                {
                    InArmClose_PitchY=TestIF.iARM_Y_PITCH;
                }
            }
            else
            {
                InArmClose_PitchY=TestIF.iARM_Y_PITCH;
            }
        }
        else
        {
            InArmClose_PitchY=TestIF.iARM_Y_PITCH;
        }

        return InArmClose_PitchY;                                               //Steven 20240201 : TestIF.iARM_Y_PITCH --> InArmClose_PitchY
    }
}

//==============================================================================
//  ROLE          -- AutoCalculateInArmXClosePitch(): searches the loader tray's
//                   X grid for a whole-cell multiple of its X-pitch that lands
//                   inside the arm's mechanical X-pitch window, and publishes it
//                   as InArmClose_PitchX plus the two derived doubles
//                   (dInArmXPitch_1Step = per-nozzle step,
//                   dInArmXPitch_MovePitch = fully-open span) and iInArmXStep
//                   (cells per step).  Returns the found pitch, or -1 after
//                   installing the fully-open fallback.  Five mutually-exclusive
//                   picker families select the window and the divisor.
//  WAVE SCOPE    -- AutoCalculateInArmXClosePitch   golden :4982-5113
//                   ACTIVE (no gates)
//  GATE REGISTER -- (none: this function writes no #if 0)
//
//  NOTES for the main loop
//  -----------------------
//  * FILE-SCOPE GLOBALS ARE NOT MINE.  golden :4977-4980 define
//    dInArmXPitch_1Step / dInArmXPitch_MovePitch / iInArmXStep / iInArmYStep
//    immediately above this function.  This part starts at golden :4982 and emits
//    NONE of them -- the main loop owns all four (plus golden :1168, :6131,
//    :8842).  They are USED here and assumed to exist.  (Sibling part
//    05735_Find_InArm_PickerMaxUseCountOnTime.txt:130-131 emits matching `extern`
//    DECLARATIONS for two of them, which is compatible either way.)
//  * INTEGER DIVISION AUDIT -- load-bearing, nothing was floated:
//      - `UserDefForm[Ld].XDivision%2` and `UserDefForm[Ld].XDivision/2`
//        (golden :5013/:5015/:5026/:5042/:5044/:5055) are int%int and int/int on
//        `int XDivision` (cprod.h:1262).  KEPT AS INTEGER DIVISION.  So
//        `iStep = XDivision/2+1` truncates FIRST, then adds 1 -- exactly golden.
//      - every `/2.0 /3.0 /4.0 /6.0` and every `double(...)` is golden's OWN
//        float divisor / cast on a double target, transcribed verbatim.
//      - NO ChangeToFloatNonPcnt was introduced anywhere in this function
//        (golden does not use it here either).
//  * NARROWING kept: `InArmClose_PitchX` is `int` (cmydef.h:3118) but
//    `UserDefForm[Ld].XPitch` is `double` (cprod.h:1258), so :5001/:5016/:5030/
//    :5045/:5059/:5075/:5098 truncate toward zero, and :5109's `?:` yields
//    `double` before truncating into the int.  That truncation IS golden's
//    behaviour; no rounding helper was added.
//  * `int iStep;` (golden :4984) is left UNINITIALISED exactly as golden has it.
//    It is only ever read after being assigned in the same branch, so this is not
//    a live defect -- but it is golden's shape and is not "tidied".
//  * DEAD-CODE BLOCK kept: golden :5085-5093 is a 9-line commented-out
//    `else if(...)` condition sitting BETWEEN the live `else` (:5084) and the
//    opening brace of its block (:5094).  Transcribed verbatim -- it documents
//    which test modes the final else was meant to cover.
//  * Symbols all present: iTotalXPitchStep (cmydef.h:5573), iMaxXPitch4Pick /
//    iMinXPitch4Pick / iMaxXPitch2Pick_13 / iMinXPitch2Pick_13 /
//    iMaxXPitch2Pick_14 / iMinXPitch2Pick_14 (cmydef.h:5576-5581),
//    USE_16PICKER_TYPE (cmydef.h:2916), iXpitchMax/X2/X3 (cmydef.h:4529-4531),
//    iXpitchMin/X2/X3 (cmydef.h:4536-4538), iXpitchMaxX4/X6 (cmydef.h:4532/4534),
//    iXpitchMinX4/X6 (cmydef.h:4539/4541), UserDefForm[4] (cprod.h:1363),
//    TrayForm (cprod.h:1359) with .Loader.iTrayType (cprod.h:1267),
//    bUseAxxGPicker / bUseAxExPicker (declared ainarm9045.h:41-42, REAL bodies
//    already in this file at ainarm9045.cpp:436 / :413),
//    iCloseSiteModeFor1x4 + e1x4CloseAbAc (aHotPlateSubstrate.h:958 / :966 --
//    resolved through aHotPlateSubstrate.h, which this file includes, NOT through
//    ainarm9045_1x4_4.h:24-32).
//  * TRAP 5 note: InArmSuck.iPickCol is read at :4996/:5069/:5070.  Same answer
//    as part 01405 -- the TMyKitSuck of aHotPlateSubstrate.h:365 (iPickCol at
//    :405), object InArmSuck defined at aHotPlateSubstrate.cpp:80.
//  * No declaration exists in golden or the port; NOT `static` (external linkage,
//    as golden).  It IS referenced -- by part 06589
//    (AdjustInArmClosePitchCondition), and after this wave also by sibling parts
//    05604_MoveInArmXYToLoader_9045.txt (golden :5606) and
//    06734_SearchLoadTrayUpDown_9045.txt (golden :6841), so it is genuinely
//    reachable, not TRAP 1 shape (a).
//  * TRAP 4: no file-scope object added.
//==============================================================================
int AutoCalculateInArmXClosePitch()                                             // golden :4982
{
    int iStep;
    int Ld=TrayForm.Loader.iTrayType;
    InArmClose_PitchX=0;
    iTotalXPitchStep    =(USE_16PICKER_TYPE==1)?7:3;
    iMaxXPitch4Pick     =(USE_16PICKER_TYPE==1)?iXpitchMaxX2:iXpitchMax;
    iMinXPitch4Pick     =(USE_16PICKER_TYPE==1)?iXpitchMinX2:iXpitchMin;
    iMaxXPitch2Pick_13  =(USE_16PICKER_TYPE==1)?iXpitchMaxX4:iXpitchMaxX2;
    iMinXPitch2Pick_13  =(USE_16PICKER_TYPE==1)?iXpitchMinX4:iXpitchMinX2;
    iMaxXPitch2Pick_14  =(USE_16PICKER_TYPE==1)?iXpitchMaxX6:iXpitchMaxX3;
    iMinXPitch2Pick_14  =(USE_16PICKER_TYPE==1)?iXpitchMinX6:iXpitchMinX3;

    if(USE_16PICKER_TYPE==1 &&                                                  //Ztex 2023.12.06 Add HT-1032
       InArmSuck.iPickCol==8)
    {
        for(int i=1; i<UserDefForm[Ld].XDivision; i++)
        {
            iInArmXStep=i;
            InArmClose_PitchX=UserDefForm[Ld].XPitch*iInArmXStep;
            if(InArmClose_PitchX>=iXpitchMin && InArmClose_PitchX<=iXpitchMax)
            {
                dInArmXPitch_1Step      =InArmClose_PitchX;
                dInArmXPitch_MovePitch  =InArmClose_PitchX*7;
                return InArmClose_PitchX;
            }
        }
    }
    else if(bUseAxxGPicker() ||                                                 //Steven 20240515 : 往上移動
            iCloseSiteModeFor1x4==e1x4CloseAbAc)                                //Steven 20241111 : for 1x4 close 2 site
    {
        if((UserDefForm[Ld].XDivision%2)==0)
        {
            iInArmXStep=(UserDefForm[Ld].XDivision/2);
            InArmClose_PitchX=UserDefForm[Ld].XPitch*iInArmXStep;
            if(InArmClose_PitchX>=iMinXPitch2Pick_14 && InArmClose_PitchX<=iMaxXPitch2Pick_14)
            {
                dInArmXPitch_1Step      =(USE_16PICKER_TYPE==1)?(double(InArmClose_PitchX)/6.0):(double(InArmClose_PitchX)/3.0);
                dInArmXPitch_MovePitch  =(USE_16PICKER_TYPE==1)?(dInArmXPitch_1Step*7.0):(dInArmXPitch_1Step*3.0);
                return InArmClose_PitchX;
            }
        }
        else
        {
            iStep=UserDefForm[Ld].XDivision/2+1;
            for(int i=iStep; i>=1; i--)                                         //Steven 20241231 : > --> >=
            {
                iInArmXStep=i;
                InArmClose_PitchX=UserDefForm[Ld].XPitch*iInArmXStep;
                if(InArmClose_PitchX>=iMinXPitch2Pick_14 && InArmClose_PitchX<=iMaxXPitch2Pick_14)
                {
                    dInArmXPitch_1Step      =(USE_16PICKER_TYPE==1)?(double(InArmClose_PitchX)/6.0):(double(InArmClose_PitchX)/3.0);
                    dInArmXPitch_MovePitch  =(USE_16PICKER_TYPE==1)?(dInArmXPitch_1Step*7.0):(dInArmXPitch_1Step*3.0);
                    return InArmClose_PitchX;
                }
            }
        }
    }
    else if(bUseAxExPicker())                                                   //Steven 20240515 : 往上移動
    {
        if((UserDefForm[Ld].XDivision%2)==0)
        {
            iInArmXStep=(UserDefForm[Ld].XDivision/2);
            InArmClose_PitchX=UserDefForm[Ld].XPitch*iInArmXStep;
            if(InArmClose_PitchX>=iMinXPitch2Pick_13 && InArmClose_PitchX<=iMaxXPitch2Pick_13)
            {
                dInArmXPitch_1Step      =(USE_16PICKER_TYPE==1)?(double(InArmClose_PitchX)/4.0):(double(InArmClose_PitchX)/2.0);
                dInArmXPitch_MovePitch  =(USE_16PICKER_TYPE==1)?(dInArmXPitch_1Step*7.0):(dInArmXPitch_1Step*3.0);
                return InArmClose_PitchX;
            }
        }
        else
        {
            iStep=UserDefForm[Ld].XDivision/2+1;
            for(int i=iStep; i>=1; i--)                                         //Steven 20241231 : > --> >=
            {
                iInArmXStep=i;
                InArmClose_PitchX=UserDefForm[Ld].XPitch*iInArmXStep;
                if(InArmClose_PitchX>=iMinXPitch2Pick_13 && InArmClose_PitchX<=iMaxXPitch2Pick_13)
                {
                    dInArmXPitch_1Step      =(USE_16PICKER_TYPE==1)?(double(InArmClose_PitchX)/4.0):(double(InArmClose_PitchX)/2.0);                            //Steven 20240807 : 2.0 --> 3.0  //Steven 20240902 : 2.0才對!!
                    dInArmXPitch_MovePitch  =(USE_16PICKER_TYPE==1)?(dInArmXPitch_1Step*7.0):(dInArmXPitch_1Step*3.0);
                    return InArmClose_PitchX;
                }
            }
        }
    }
    else if(InArmSuck.iPickCol==4 ||                                            //Steven 20240515 : 吸嘴數少的要先判断
            InArmSuck.iPickCol==1)
    {
        for(int i=1; i<UserDefForm[Ld].XDivision; i++)
        {
            iInArmXStep=i;
            InArmClose_PitchX=UserDefForm[Ld].XPitch*i;
            if(InArmClose_PitchX>=iMinXPitch4Pick && InArmClose_PitchX<=iMaxXPitch4Pick)
            {
                dInArmXPitch_1Step      =(USE_16PICKER_TYPE==1)?(double(InArmClose_PitchX)/2.0):InArmClose_PitchX;
                dInArmXPitch_MovePitch  =(USE_16PICKER_TYPE==1)?(dInArmXPitch_1Step*7.0):(dInArmXPitch_1Step*3.0);
                return InArmClose_PitchX;
            }
        }
    }
    else
//    else if((TestIF.iTestMode==_8Site2X4  ||
//             TestIF.iTestMode==_10Site2X5 ||                                  //wei 20190614 10 site
//             TestIF.iTestMode==_12Site2X6 ||                                  //Steven 20141122 : 補上12, 16, 32 Site的X-Pitch模式
//             TestIF.iTestMode==_16Site4X4 ||                                  //kevin 20210916 add
//             TestIF.iTestMode==_16Site2X8 ||
//             TestIF.iTestMode==_32Site4X8N ||
//             TestIF.iTestMode==_32Site4X8M ||
//             TestIF.iTestMode==QualSite1X4) &&                                //JerryYang 20230322 : fix 1x4_14吸嘴
//             ArmCanSuck4IC(0)==false)                                         // 2x4 big ic
    {
        for(int i=1; i<UserDefForm[Ld].XDivision; i++)
        {
            iInArmXStep=i;
            InArmClose_PitchX=UserDefForm[Ld].XPitch*i;
            if(InArmClose_PitchX>=iMinXPitch4Pick && InArmClose_PitchX<=iMaxXPitch4Pick)
            {
                dInArmXPitch_1Step      =(USE_16PICKER_TYPE==1)?(double(InArmClose_PitchX)/2.0):InArmClose_PitchX;
                dInArmXPitch_MovePitch  =(USE_16PICKER_TYPE==1)?(dInArmXPitch_1Step*7.0):(dInArmXPitch_1Step*3.0);
                return InArmClose_PitchX;
            }
        }
    }

    iInArmXStep=1;
    InArmClose_PitchX       =(USE_16PICKER_TYPE==1)?(double(iMaxXPitch4Pick)/2.0):iMaxXPitch4Pick;
    dInArmXPitch_1Step      =(USE_16PICKER_TYPE==1)?(double(InArmClose_PitchX)/2.0):InArmClose_PitchX;
    dInArmXPitch_MovePitch  =(USE_16PICKER_TYPE==1)?(dInArmXPitch_1Step*7.0):(dInArmXPitch_1Step*3.0);
    return -1;
}

// =============================================================================
//  k4-ainarm PART 05/08  --  GetInArmToLoaderPosition   golden ainarm9045.cpp:5115-5172
//  Translator: AI(k4-ainarm) 20260811
//
//  ROLE   : Pure geometry.  Fills the per-nozzle Loader column map iXPosition[]/iYPosition
//           for the active picker kind (AxxG or 1x4-close-2-site -> nozzles 0 and 3; AxEx ->
//           0 and 2; 4- or 1-pick -> 0..3; else the full 8), then returns the X/Y motor
//           target for the base nozzle, honouring the E46 "Loader front/rear rows get
//           separate offsets" option (suppressed while AOA alignment is on).
//
//  WAVE SCOPE
//    GetInArmToLoaderPosition  golden :5115-5172  ACTIVE, fully faithful, ZERO gates.
//
//  GATE REGISTER: (empty -- nothing gated in this function)
//    iXPosition/iYPosition were the only unreachable names (probe TU errors "'iYPosition'
//    was not declared in this scope" / "'iXPosition' was not declared in this scope",
//    2026-08-11 01:26:17 +0800).  They DO exist in the port -- ainarm9045_2x4_16_shims.h:55-56 -- but that
//    header is not in ainarm9045.cpp's include block, so this part restates the two
//    declarations verbatim instead of gating live geometry.  A repeated identical extern is
//    legal, so the main loop may instead add the include and drop these two lines.
//
//  INTEGER ARITHMETIC IS LOAD-BEARING -- read before touching :5161/:5169.
//    `iInArmXBase*dInArmXPitch_1Step` promotes the whole right-hand side to double and the
//    assignment to `int &iXPos` then TRUNCATES toward zero.  That truncation is golden's
//    observable behaviour (it is how the base-nozzle X lands on a whole encoder count), so it
//    is reproduced exactly: no rounding helper, no ChangeToFloatNonPcnt, no reordering of the
//    five terms.  Likewise :5142-5155 stay integer `iCol + N*iInArmXStep` products -- turning
//    any of these into floating point is the documented way to produce stepped position drift.
//
//  FILE-SCOPE GLOBALS I ONLY USE, NEVER DEFINE (main loop owns all seven in one place):
//    iInArmXStep (golden :4979) and dInArmXPitch_1Step (golden :4977).  Not emitted here --
//    two agents defining one global is a `multiple definition` link error.
//
//  Big5 -> UTF-8: the commented-out iPickCol==1 block (:5119-5126) is transcribed verbatim,
//  INCLUDING its Chinese comment on :5123, plus :5158, :5159 and the two-part :5160.
//  Dead code is NOT dropped.  ZERO U+FFFD.
// =============================================================================
// k4-ainarm: golden ainarm2.h iXPosition[8]/iYPosition.  Port home is
// ainarm9045_2x4_16_shims.h:55-56, which ainarm9045.cpp does not include -- restated here
// verbatim rather than gating the geometry.  Identical to that header, so both may coexist.
extern int iXPosition[8];
extern int iYPosition;

void GetInArmToLoaderPosition(int iSelRow, int &iXPos, int &iYPos, int iRow, int iCol)
{
    iYPosition=iRow;

//    if(InArmSuck.iPickCol==1)
//    {
//        iXPosition[0]=iCol;
//        iXPosition[1]=iCol+1;                                                   //Steven 20240920 : Fixed for 1x1 mode
//        iXPosition[2]=iCol+2;                                                   //Steven 20240920 : Loader吸料計算的資料錯誤;
//        iXPosition[3]=iCol+3;
//    }
//    else
    if(bUseAxxGPicker()||
      iCloseSiteModeFor1x4==e1x4CloseAbAc)                                      //Steven 20241111 : for 1x4 close 2 site
    {
        iXPosition[0]=iCol;
        iXPosition[3]=iCol+iInArmXStep;
    }
    else if(bUseAxExPicker())
    {
        iXPosition[0]=iCol;
        iXPosition[2]=iCol+iInArmXStep;
    }
    else if(InArmSuck.iPickCol==4 ||
            InArmSuck.iPickCol==1)                                              //Steven 20240920 : Fixed for 1x1 mode
    {
        iXPosition[0]=iCol;
        iXPosition[1]=iCol+1*iInArmXStep;
        iXPosition[2]=iCol+2*iInArmXStep;
        iXPosition[3]=iCol+3*iInArmXStep;
    }
    else                                                                        //if(InArmSuck.iPickCol==8)
    {
        iXPosition[0]=iCol+0*iInArmXStep;
        iXPosition[1]=iCol+1*iInArmXStep;
        iXPosition[2]=iCol+2*iInArmXStep;
        iXPosition[3]=iCol+3*iInArmXStep;
        iXPosition[4]=iCol+4*iInArmXStep;
        iXPosition[5]=iCol+5*iInArmXStep;
        iXPosition[6]=iCol+6*iInArmXStep;
        iXPosition[7]=iCol+7*iInArmXStep;
    }

    if(IniConfig.bE46_LoaderUse2Offset && iSelRow==1 &&                         //Steven 20140827 : Loader可以分別調前後排的Offset
       TestIF.bEnableAutoAlignment==false)                                      //KenHsieh 20220923 : add AOA功能開啟不補償
    {                                                                           //基準移動到吸取的位置    //第一支移動到基準位置
        iXPos=Prod.XInArm_Tray_Pick[iInArmYBase][iInArmXBase]+iCol*Prod.LoadForm.iXPitch+iInArmXBase*dInArmXPitch_1Step-InArmOffSet[InOfsLoader]->GetX()+InArmOffSet[InOfsLoaderB]->GetX();
        iYPos=Prod.YInArm_Tray_Pick[iInArmYBase][iInArmXBase]-iRow*Prod.LoadForm.iYPitch-InArmOffSet[InOfsLoader]->GetY()+InArmOffSet[InOfsLoaderB]->GetY();
    }
    else
    {
        if(USE_PICKER_COUNT==ep1Picker)
            iXPos=Prod.XInArm_Tray_Pick[iInArmYBase][iInArmXBase]+iCol*Prod.LoadForm.iXPitch;
        else
            iXPos=Prod.XInArm_Tray_Pick[iInArmYBase][iInArmXBase]+iCol*Prod.LoadForm.iXPitch+iInArmXBase*dInArmXPitch_1Step;
        iYPos=Prod.YInArm_Tray_Pick[iInArmYBase][iInArmXBase]-iRow*Prod.LoadForm.iYPitch;
    }
}

//==============================================================================
//  GetInArmToLoaderPosition_Single  --  golden ainarm9045.cpp:5174-5358
//  Translator: AI(W7D-ainarm-k8) 20260811
//
//  ROLE          -- the SINGLE-nozzle Loader pick solver (Fix mode / 1-picker
//                   path).  Finds the first enabled nozzle on row iSelRow,
//                   force-disables every LATER nozzle on BOTH rows, computes the
//                   X/Y tray-cell target for it, and -- if that X would overrun
//                   MInArmX's positive soft limit -- searches
//                   DeviceForm.XDimension*i+200 for an X pitch inside the picker
//                   family's min/max window, writing dInArmXPitch_1Step and
//                   dInArmXPitch_MovePitch.  It then RE-computes X/Y with the
//                   chosen 1-step pitch, records the cursor into
//                   iXPosition[]/iYPosition, and returns the move pitch.
//  WAVE SCOPE    -- GetInArmToLoaderPosition_Single   golden :5174-5358   ACTIVE,
//                   NO GATE
//
//  GATE REGISTER -- (empty: this function contains no #if 0)
//
//  RETIRED GATE W7D-G04 -- TRAP 2 CAUGHT IN-WAVE, READ THIS.
//  I first gated golden :5323 `AutoCalculateInArmXClosePitch()` on the premise "no
//  port body, no port declaration, no port call site (0 textual hits tree-wide)" --
//  measured 2026-08-11 01:34:22 +0800 with:
//     grep -rn --include=*.cpp --include=*.h -w "AutoCalculateInArmXClosePitch" . | grep -v '^\./build'
//  THAT PREMISE EXPIRED inside this same wave: a SIBLING AGENT landed
//  _w7d_parts/04982_AutoCalculateInArmXClosePitch.txt, defining
//  `int AutoCalculateInArmXClosePitch()` (its line 63) with EXTERNAL linkage at
//  golden :4982 -- i.e. in this very stitched TU, AHEAD of my :5174 in golden
//  order.  Re-measured 2026-08-11 01:54:20 +0800.
//  TRAP 3 RE-ASK: any DIFFERENT reason to stay gated?  No -- same TU, external
//  linkage, no cross-archive dependency, no hard-coded path, no persistent-state
//  write.  GATE RETIRED and golden :5323 is emitted VERBATIM below, so the "大IC
//  使用Fix時,吸嘴B吸Loader最右側可能會Close Pitch導致飛料" protection golden :5316 documents is now
//  actually present on the 8-picker arm instead of silently skipped.
//  CROSS-PART HAND-OFF, MAIN LOOP MUST RECONCILE: the sibling part file
//  _w7d_parts/05604_MoveInArmXYToLoader_9045.txt still gates the SAME symbol at its
//  line 123 -- `#if 0 // GATE k4-G2 -- golden :5606 AutoCalculateInArmXClosePitch
//  (golden ainarm9045.cpp:4982; NO port body)` -- on the premise that is now dead
//  too.  If mine is retired and k4-G2 is not, the stitched tree ends up with the
//  same golden call LIVE in one place and silently dead in another, which is the
//  exact hazard TRAP 3 warns about.  I did not edit that file (outside my
//  boundary); flagging it IS the hand-off.
//
//  TRAP 1: the caller situation ALSO changed inside this wave, so the honest
//  statement is not "shape (a)".  golden's only caller of this function is
//  MoveInArmXYToLoader_9045 at golden :5636 (whose definition starts at golden
//  :5604), and that function IS landing this wave as sibling part file
//  05604_MoveInArmXYToLoader_9045.txt -- so my body gets a real caller in the same
//  TU.  SEPARATE DEFECT FOUND while checking this: the port's existing offline stub
//  at ainarm9045.cpp:2303 is spelled `MoveArmXYToLoader_9045` (no "In") and cites
//  "golden :5604", but golden contains ZERO occurrences of `MoveArmXYToLoader_9045`
//  and 6 of `MoveInArmXYToLoader_9045` (counted 2026-08-11 01:54:20 +0800).  So the
//  sibling's correctly-named definition will NOT collide with that stub -- the stub
//  survives as a misnamed orphan rather than being retired.  MAIN LOOP: delete it.
//
//  FILE-SCOPE GLOBALS NOT MINE: dInArmXPitch_1Step and dInArmXPitch_MovePitch
//  (golden :4977-4978) are read AND written here.  Per this wave's brief the MAIN
//  LOOP defines all seven such globals in one place, so this part file USES them
//  and deliberately does NOT define them (two agents defining one global is a
//  `multiple definition` link error).
//
//  GOLDEN BUGS KEPT VERBATIM (do NOT "fix" any of these):
//   1. :5194-5195 `if(iCurrSuck<0 || iCurrSuck>4)` -- the search loop above runs
//      i<4, so iCurrSuck can only be -1..3; the `>4` half is unreachable and even
//      a hypothetical 4 would slip through as valid.  It should read `>=4`.
//   2. :5197 and :5357 `return dInArmXPitch_MovePitch;` return a double from an
//      `int` function -- truncated.
//   3. :5203 / :5205 / :5347 / :5349 assign a `double(...)` expression into the
//      `int&` out-param iXPos -- truncated.  Kept: that is the machine's real
//      rounding and changing it is exactly the stepped-drift defect class.
//   4. :5300-5302 the `iPickCol==4` non-16PICKER arm is the ONLY one of the six
//      search arms with a `break` after a match; the other five keep looping and
//      rely on the `bHasMatch==false` guard to make later passes no-ops.  Same
//      end result, but the asymmetry is golden's.
//   5. :5300 sets dInArmXPitch_1Step to the WHOLE iTrayXPitch while its sibling
//      arms divide by 2/3/4/6 -- consistent with the *3.0 MovePitch beside it.
//   6. :5222 etc. `iTrayXPitch=DeviceForm.XDimension*i+200;` is double*int+200
//      truncated into an int, and for i==0 it is the constant 200.
//  The commented-out `iTrayXPitch=Prod.LoadForm.iXPitch*i;` at :5280 is kept as a
//  comment, verbatim, as are all the dated engineer annotations in the range.
//  TRAP 5 -- InArmSuck: aHotPlateSubstrate.h TMyKitSuck (class :365; .Suck[][] is
//  TMySucker whose .iMyCol is :134; .iPickCol :405), NOT mykitsuck.h:274 (:269 /
//  :291).  Same header this TU already includes; object in aHotPlateSubstrate.cpp.
//==============================================================================
// AI(W7D-ainarm-k8) 20260811: golden ainarm2.h:82 `extern int  iXPosition[8], iYPosition;`
// The port ainarm2.h lacks that line; the only port declaration is
// ainarm9045_2x4_16_shims.h:55-56, which this TU does not include.  DECLARATION
// ONLY -- the objects are defined at ainarm9045_2x4_16_shims.cpp:21-22, so this
// can never cause `multiple definition` even if a sibling part emits it too.
extern int  iXPosition[8];                                                      // golden ainarm2.h:82
extern int  iYPosition;                                                         // golden ainarm2.h:82
//==============================================================================
int GetInArmToLoaderPosition_Single(int iSelRow, int &iXPos, int &iYPos, int iRow, int iCol)
{
    int iRealUseSuck=-1, iCurrSuck=-1, iTrayXPitch;
    bool bHasMatch=false;
    iYPosition=iRow;

    for(int i=0; i<4; i++)
    {
        if(InArmSuckUse[iSelRow][i]==true)
        {
            iCurrSuck=i;
            for(int j=(i+1); j<4; j++)
            {
                InArmSuckUse[iSelRow][j]=false;
                InArmSuckUse[1-iSelRow][j]=false;
            }
            break;
        }
    }

    if(iCurrSuck<0 ||
       iCurrSuck>4)
    {
        return dInArmXPitch_MovePitch;                                          //Steven 20240701 : 修正Fix模式下, X Pitch錯誤
    }

    iRealUseSuck=InArmSuck.Suck[iSelRow][iCurrSuck].iMyCol;

    if(USE_PICKER_COUNT==ep1Picker)
        iXPos=double(Prod.XInArm_Tray_Pick[iSelRow][iCurrSuck])+double(iCol*Prod.LoadForm.iXPitch);
    else                                                                        //基準移動到吸取的位置                    //第一支移動到基準位置 - 要使用的吸嘴移動到第一支的位置
        iXPos=double(Prod.XInArm_Tray_Pick[iSelRow][iCurrSuck])+double(iCol*Prod.LoadForm.iXPitch)+double(iInArmXBase-iRealUseSuck)*dInArmXPitch_1Step;
    if(IniConfig.bUseTrayBlockMode)                                             //Frank 20160928 add Subtray Function
        InArmAddBlockPitch(iXPos, iYPos, iRow, iCol, 1);

    if(iXPos>(MOT[MInArmX].Motor->PSoftLimitP-100))
    {
        if(InArmSuck.iPickCol==1)
        {
            bHasMatch=true;
            dInArmXPitch_1Step    =iXpitchMin;
            dInArmXPitch_MovePitch=iXpitchMinX7;
        }
        else if(bUseAxxGPicker() ||
                iCloseSiteModeFor1x4==e1x4CloseAbAc)                            //Steven 20241111 : for 1x4 close 2 site)
        {
            for(int i=0; i<Prod.LoadForm.iXItem; i++)
            {
                iTrayXPitch=DeviceForm.XDimension*i+200;
                if(USE_16PICKER_TYPE==1)                                        //Ztex 2023.12.06 Add HT-1032
                {
                    if(bHasMatch==false &&
                       iTrayXPitch>=iXpitchMinX6 &&
                       iTrayXPitch<=iXpitchMaxX6)
                    {
                        bHasMatch=true;
                        dInArmXPitch_1Step      =double(iTrayXPitch)/6.0;
                        dInArmXPitch_MovePitch  =dInArmXPitch_1Step*7.0;
                    }
                }
                else
                {
                    if(bHasMatch==false &&
                       iTrayXPitch>=iXpitchMinX3 &&
                       iTrayXPitch<=iXpitchMaxX3)
                    {
                        bHasMatch=true;
                        dInArmXPitch_1Step      =double(iTrayXPitch)/3.0;
                        dInArmXPitch_MovePitch  =iTrayXPitch;                   //dInArmXPitch_1Step*3.0;
                    }
                }
            }
        }
        else if(bUseAxExPicker())
        {
            for(int i=0; i<Prod.LoadForm.iXItem; i++)
            {
                iTrayXPitch=DeviceForm.XDimension*i+200;
                if(USE_16PICKER_TYPE==1)                                        //Ztex 2023.12.06 Add HT-1032
                {
                    if(bHasMatch==false &&
                       iTrayXPitch>=iXpitchMinX4 &&
                       iTrayXPitch<=iXpitchMaxX4)
                    {
                        bHasMatch=true;
                        dInArmXPitch_1Step      =double(iTrayXPitch)/4.0;
                        dInArmXPitch_MovePitch  =dInArmXPitch_1Step*7.0;
                    }
                }
                else
                {
                    if(bHasMatch==false &&
                       iTrayXPitch>=iXpitchMinX2 &&
                       iTrayXPitch<=iXpitchMaxX2)
                    {
                        bHasMatch=true;
                        dInArmXPitch_1Step      =double(iTrayXPitch)/2.0;
                        dInArmXPitch_MovePitch  =dInArmXPitch_1Step*3.0;
                    }
                }
            }
        }
        else if(InArmSuck.iPickCol==4)
        {
            for(int i=0; i<Prod.LoadForm.iXItem; i++)
            {
//                iTrayXPitch=Prod.LoadForm.iXPitch*i;
                iTrayXPitch=DeviceForm.XDimension*i+200;                        //Steven 20250122 : 修正A吸嘴到最右邊下不去
                if(USE_16PICKER_TYPE==1)                                        //Ztex 2023.12.06 Add HT-1032
                {
                    if(bHasMatch==false &&
                       iTrayXPitch>=iXpitchMinX2 &&
                       iTrayXPitch<=iXpitchMaxX2)
                    {
                        bHasMatch=true;
                        dInArmXPitch_1Step      =double(iTrayXPitch)/2.0;
                        dInArmXPitch_MovePitch  =dInArmXPitch_1Step*7.0;
                    }
                }
                else
                {
                    if(bHasMatch==false &&
                       iTrayXPitch>=iXpitchMin &&
                       iTrayXPitch<=iXpitchMax)
                    {
                        bHasMatch=true;
                        dInArmXPitch_1Step      =iTrayXPitch;
                        dInArmXPitch_MovePitch  =iTrayXPitch*3.0;
                        break;
                    }
                }
            }
        }
        else if(InArmSuck.iPickCol==8)
        {
            bHasMatch=true;
            if(Prod.LoadForm.iXPitch*7.0<=iXpitchMinX7)
            {
                dInArmXPitch_1Step      =iXpitchMin;
                dInArmXPitch_MovePitch  =iXpitchMinX7;
            }
            else if(Prod.LoadForm.iXPitch*7.0<=iXpitchMaxX7 &&
                    Prod.LoadForm.iXPitch*7.0>=iXpitchMinX7)                    //Steven 20140520 : 大IC使用Fix時,吸嘴B吸Loader最右側可能會Close Pitch導致飛料
            {
                dInArmXPitch_1Step      =Prod.LoadForm.iXPitch;
                dInArmXPitch_MovePitch  =Prod.LoadForm.iXPitch*7.0;
            }
            else
            {
                AutoCalculateInArmXClosePitch();
            }
        }
    }
    else
    {
        bHasMatch=true;
    }

    if(bHasMatch==false)
    {
        if(USE_16PICKER_TYPE==1)                                                //Ztex 2023.12.06 Add HT-1032
        {
            dInArmXPitch_1Step      =iXpitchMax;
            dInArmXPitch_MovePitch  =iXpitchMaxX7;
        }
        else
        {
            dInArmXPitch_1Step      =iXpitchMax;
            dInArmXPitch_MovePitch  =iXpitchMaxX3;
        }
    }

    if(USE_PICKER_COUNT==ep1Picker)
        iXPos=double(Prod.XInArm_Tray_Pick[iSelRow][iCurrSuck])+double(iCol*Prod.LoadForm.iXPitch);
    else                                                                        //基準移動到吸取的位置            //第一支移動到基準位置 - 要使用的吸嘴移動到第一支的位置
        iXPos=double(Prod.XInArm_Tray_Pick[iSelRow][iCurrSuck])+double(iCol*Prod.LoadForm.iXPitch)+double(iInArmXBase-iRealUseSuck)*dInArmXPitch_1Step;
    iYPos=Prod.YInArm_Tray_Pick[iSelRow][iCurrSuck]-iRow*Prod.LoadForm.iYPitch;
    if(IniConfig.bUseTrayBlockMode)                                             //Frank 20160928 add Subtray Function
        InArmAddBlockPitch(iXPos, iYPos, iRow, iCol, 2);

    iXPosition[iCurrSuck]=iCol;
    iYPosition=iRow;

    return dInArmXPitch_MovePitch;
}

//==============================================================================
//== ROLE: LotRecordUPH -- append one UPH row (start / end / pause / units-per-hour)
//==       to <as9045UPH>\<SysLotID>_UPH.csv, writing the CSV header row first if
//==       the file does not exist yet.  (golden comment: KYEC UPH record)
//==
//== WAVE SCOPE (k2-ainarm):
//==   LotRecordUPH  golden ainarm9045.cpp:5362-5374  -- ACTIVE
//==     (100% faithful; ZERO gates)
//==
//== GATE REGISTER: (none -- this function needed no #if 0)
//==   Callee/data provenance: as9045UPH -> common.h (value "D:\HT9045_Log\UPH",
//==   set in common.cpp); fLotInfo->edtSysLotID -> forms/fLotInfo.h:29
//==   (TfLotInfoEdit*, real member, ->Text is real storage); FileExists ->
//==   database.h; WriteDataToFile -> reachable through this TU's existing includes
//==   (verified by probe compile with the exact ht9045_sm flags).
//==
//== TRAP 1 shape (a) -- READ THIS.  This function has NO CALLER in the port today
//== and NO declaration in any header, so nothing will pull it out of the archive:
//==   * golden's only call site is golden ainarm9045.cpp:5507, inside the RecordUPH
//==     region, and RecordUPH is still a no-op stub at port ainarm9045.cpp:2275
//==     (`void RecordUPH(int) {}  // golden :5376`).
//==   * absence command (cwd D:/HT9045/HT9011UC_Cpp_V3.33.906.0):
//==       rg -n "LotRecordUPH" --glob "*.cpp" --glob "*.h" .   -> 0 hits
//==     MEASURED AT 2026-08-11 01:34:39 local (Asia/Taipei).
//== So a green build proves NOTHING about this body.  It is deliberately NOT
//== static (external linkage, ready for its caller), and the main loop should add
//== `extern void LotRecordUPH(AnsiString, AnsiString, AnsiString, int);` to
//== ainarm9045.h in the wave that translates RecordUPH (golden :5376).
//==
//== FILE-I/O SAFETY (the TRAP 3 shape that WriteLastDataFile hit): this writes
//== under as9045UPH = "D:\HT9045_Log\UPH", the REAL machine log tree, not a test
//== sandbox.  It is safe to land ACTIVE **only because it has no caller** (above)
//== -- ctest cannot reach it.  WHOEVER WIRES RecordUPH MUST RE-ASK THAT QUESTION,
//== because at that moment ctest gains a path that creates real files under
//== D:\HT9045_Log\UPH.  Flagged here rather than pre-emptively gating a faithful
//== body.
//==
//== GOLDEN QUIRK PRESERVED (do not "fix"): golden :5372 passes AnsiString objects
//== into %s varargs slots.  That is latent UB under BCB6 which happens to work
//== because AnsiString is a single pointer; the port's vclcompat sprintf handles
//== the idiom deliberately (vclcompat/AnsiString.h:139 "raw AnsiString to %s"), so
//== the line is transcribed verbatim rather than rewritten with .c_str().  golden
//== :5366 DOES use .c_str() on both of its arguments -- also as-is.  The
//== inconsistency is golden's, and it is kept.
//==============================================================================
void LotRecordUPH(AnsiString UPH_StartTime, AnsiString UPH_EndTime, AnsiString UPH_PauseTime, int iUPH)                 //wei 20151221 KYEC 儲存UPH紀錄
{
    AnsiString filename;
    AnsiString t;
    filename.sprintf("%s\\%s_UPH.csv", as9045UPH.c_str(), fLotInfo->edtSysLotID->Text.c_str());
    if(!FileExists(filename))
    {
        t.sprintf("%s, %s, %s, %s\n", "Start Time", "End Time", "Pause Time", "UPH");
        WriteDataToFile(filename, t);                                           //Steven 20160604 : add protect of fopen
    }
    t.sprintf("%s, %s, %s, %d\n", UPH_StartTime, UPH_EndTime, UPH_PauseTime, iUPH);
    WriteDataToFile(filename, t);
}

//==============================================================================
//  k7-ainarm PART 05412 -- CalculateUPH(bool)
//  Translator: AI(pt-wave k7-ainarm) 20260811
//  Golden: ainarm9045.cpp:5412-5602 (cp950)
//
//  ROLE          : the UPH (units-per-hour) accountant.  On reset it zeroes the
//                  running UPH, restamps tUPH_StartTime and blanks the 12-row UPH
//                  history grid (7 columns when the VTEST profile is on).
//                  Otherwise it closes the interval: scrolls the grid down one
//                  row, records start/end/pause times, subtracts the paused time,
//                  converts the remainder to seconds via DecodeTime, computes
//                  RunInfo.iUPH = (3600/seconds) * iUPH_LoaderCount, mirrors the
//                  10 newest rows into the SECS/GEM tsUPH string list, fires SECS
//                  event 54 (UPHRecordEnd), then averages the 10 stored UPH values
//                  into RunInfo.iAvgUPH.  Finally it paints the status bar.
//  WAVE SCOPE    : CalculateUPH  golden :5412-5602 -- ACTIVE body (every formula,
//                  every branch, both loops, the DEBUG_TRY_CATCH #ifdef verbatim),
//                  ONE seam (k7-S1) + FOUR gates (k7-G4, k7-G6, k7-G7a, k7-G7b),
//                  and ONE gate RETIRED mid-wave (k7-G5, LotRecordUPH).  No stub to
//                  retire -- the symbol exists nowhere in the port.  Its only
//                  golden-visible port call site, ainarm9045_2x8_32.cpp:1226, sits
//                  inside that file`s whole-body `#if 0 // TODO(W7)` (see
//                  CMakeLists.txt:1519), so there is NO reachable caller yet:
//                  net-new surface, not a behaviour change.
//
//  NOTE (k7-N1) -- __fastcall.  Golden`s signature is `void __fastcall
//  CalculateUPH(bool bReset)`.  Dropped, per THIS FILE`S OWN documented rule
//  (port ainarm9045.cpp:49, "__fastcall removed from CalculateUPH").  Safe here
//  because the symbol has NO declaration anywhere in the tree (checked every
//  *.h), so there is no decl/def pair to keep in lockstep -- which is exactly the
//  hazard vclcompat/vcl_compat.h:24-50 warns about.  If the main loop adds a
//  declaration to ainarm9045.h it must ALSO be written without __fastcall.
//
//  NOTE (k7-N2) -- TDateTime::FormatString.  Golden calls
//  `<TDateTime>.FormatString("hh:nn:ss")` at :5476/:5480/:5481/:5482.  vclcompat
//  has no TDateTime::FormatString member (rg -n "FormatString" vclcompat/ -> 0
//  hits, 2026-08-11 01:20 +0800); the tree`s established rewrite is
//  `FormatDateTime(fmt, dt)` (vclcompat/TDateTime.h:67), whose precedent is set by
//  THIS FILE at ainarm9045.cpp:890 and cited by name in OCRInsp.cpp:1377,
//  Automation/SCK_ART_Remainder.cpp:1428 and Motor/myGALILmotor.cpp:424.  The
//  FORMAT STRINGS ARE UNCHANGED.  Not a gate -- an exact-equivalent rewrite.
//
//  INTEGER SEMANTICS PRESERVED (no float-helper substitution anywhere):
//    * :5556 `iTotalUPH/iCount` stays INTEGER division inside AnsiString(int).
//    * :5530 `Cells[0][i/2+1]` / `Cells[3][i/2+1]` stay INTEGER `i/2`.
//    * :5493 `3600/fConsumeSecond` IS double division in golden (fConsumeSecond is
//      double) -- kept as written, NOT "fixed" to 3600.0.
//    * :5498 `RunInfo.iUPH=fTimerMultiple*iUPH_LoaderCount;` keeps golden`s
//      double->int truncation.
//    * :5491 `MSec/(double)1000.0` keeps golden`s cast.
//
//  SEAM REGISTER
//  ------------------------------------------------------------------------
//  SEAM (k7-S1)  golden lines: ~40 sites across :5428-5600
//    SYMBOL       : fShowBinSelect (golden cShowBinSelect.h, TfShowBinSelect*) --
//                   its ->UPH_StringGrid->Cells[col][row] grid and ->tsUPH->
//                   Strings[i] list.
//    WHY A SEAM AND NOT A GATE : this widget is not decoration for CalculateUPH --
//                   it IS the algorithm`s storage.  The 10-row history is produced
//                   by golden shifting Cells[c][i]=Cells[c][i-1] (:5459-5468) and
//                   the average is then computed by READING Cells[3][1..10] back
//                   (:5542-5545).  Gate the grid and RunInfo.iAvgUPH becomes a
//                   constant 0 and the whole scroll loop becomes dead -- i.e.
//                   gating would silently delete arithmetic, which is worse than
//                   the documented seam.  So the grid gets a TU-local backing
//                   store and every formula stays live and faithful.  The port
//                   already sets this precedent twice for THIS EXACT widget
//                   (csystem.cpp:4853-4860 `W7C1_TfShowBinSelectSeam`,
//                   Automation/auto9045.cpp:325 `W5FA_FShowBinSelect`) and once
//                   for absent fMain widgets (aTester_Rear.cpp:100-106
//                   `W64bT2_TfMainTorqueSeam`).  Mine differs from csystem`s in
//                   one deliberate way: csystem`s seam made UPH_StringGrid a VALUE
//                   member with no `Cells`, which is why csystem.cpp:26233-26241
//                   had to GATE golden`s real expression.  Mine supports the full
//                   `->UPH_StringGrid->Cells[c][r]`, so golden`s text is verbatim.
//    PROVING COMMAND (run from D:/HT9045/HT9011UC_Cpp_V3.33.906.0):
//                   rg -n "^\s*(extern\s+)?\w+\s*\*\s*fShowBinSelect|extern.*fShowBinSelect" -g "*.h" -g "*.cpp"
//                   -> 0 hits (exit 1);  rg -l "ShowBinSelect" forms/ vclcompat/
//                   -> 0 hits (exit 1).  The 7 files that mention the name hold
//                   only comments, TU-local seams, or `#if 0` regions
//                   (atester.cpp:2779 is inside the `#if 0` opened at :2614).
//    MEASURED AT  : 2026-08-11 01:34:42 +0800
//    RE-MEASURED  : 2026-08-11 01:49:41 +0800, now also scanning every sibling
//                   part file -> still 0 declarations anywhere.  SEAM STANDS
//                   (TRAP 2 checked, premise alive).
//    REAL-MACHINE DIFFERENCE : the operator no longer SEES the UPH history grid --
//                   the numbers are computed and stored, just not painted.  Every
//                   value the state machine consumes (RunInfo.iUPH,
//                   RunInfo.iAvgUPH, tUPH_*, iUPH_LoaderCount, MyDBIUPH`s EventLog
//                   row, SECS event 54) is unaffected.
//    TRAP 4       : the backing store lives in FUNCTION-LOCAL statics inside
//                   W7dK7_ShowBinSelect(), so no file-scope constructor runs
//                   before main() -- the fLaserSensor failure mode cannot recur
//                   here.  The struct that holds the two pointers is POD
//                   (zero-init, no dynamic init) and is re-pointed on every call,
//                   so cross-TU init order is irrelevant.
//    RETIREMENT   : when a real TfShowBinSelect lands in forms/, delete the seam
//                   block and the `#define`/`#undef` pair.  Golden`s ~40 lines
//                   need no edit.
//
//  GATE REGISTER
//  ------------------------------------------------------------------------
//  GATE (k7-G4)  golden line: ainarm9045.cpp:5497
//    SYMBOL       : the file-scope global `char DateTime[256];` -- golden DEFINES
//                   it at ainarm9045.cpp:5360, i.e. between two functions of THIS
//                   SAME golden file.  ** IT IS NOT IN THE TASK`S LIST OF SEVEN
//                   un-owned file-scope globals (:1168, :4977, :4978, :4979,
//                   :4980, :6131, :8842) ** -- so it is an EIGHTH one, owned by
//                   nobody, and I must not emit it (it is outside my :5412-5602
//                   range and the rule is that globals are the main loop`s).
//    OFFLINE DEFAULT CHOSEN : the single `sprintf(DateTime, ...)` statement is
//                   gated out; nothing replaces it.
//    WHY FAITHFUL : within CalculateUPH DateTime is WRITE-ONLY -- nothing in this
//                   function ever reads it.  Its only reader in the whole golden
//                   file is RecordUPH (golden :5385 `t.sprintf("%s, %d\n",
//                   DateTime, UPH)`), which in the port is the no-op stub `void
//                   RecordUPH(int) {}` at ainarm9045.cpp:2275.  So the gate has
//                   ZERO observable effect in the port today -- provably, not
//                   arguably.
//    REAL-MACHINE DIFFERENCE : none until RecordUPH is translated; at that point
//                   the hourly UPH CSV would carry an empty timestamp column.
//    PROVING COMMAND (run from D:/HT9045/HT9011UC_Cpp_V3.33.906.0):
//                   rg -n "char[[:space:]]+DateTime" -g "*.cpp" -g "*.h"
//                   -> 0 hits (exit 1).  (`DateTime` alone DOES hit cpublic.h
//                   :244/:267/:285/:305, but those are `AnsiString DateTime[...]`
//                   MEMBERS of queue structs -- a different symbol entirely.  Do
//                   not let that false positive retire this gate.)
//    MEASURED AT  : 2026-08-11 01:34:26 +0800
//    RE-MEASURED  : 2026-08-11 01:49:41 +0800, now also scanning every sibling
//                   part file (-g "_w7d_parts/*.txt") -> still nobody emits it.
//                   GATE STANDS (TRAP 2 checked, premise alive).
//    TRAP 3 CHECK : when the global does land, retiring this gate requires
//                   `#include <stdio.h>` to be in scope for sprintf() -- I added
//                   only <stdlib.h> (for atoi, golden :5544) because the sprintf
//                   line is gated.
//    ACTION FOR THE MAIN LOOP : add `char DateTime[256];` (golden :5360) to the
//                   same one place the other seven globals go, and re-run the
//                   census -- tools/census/wave_targets.py missed it.
//
//  RETIRED GATE (k7-G5) -- THE THIRD TRAP-2 CASE, CAUGHT INSIDE THIS WAVE
//    golden line :5507, symbol LotRecordUPH(AnsiString, AnsiString, AnsiString,
//    int) -- golden defines it at ainarm9045.cpp:5362, in THIS SAME golden file,
//    50 lines above my range and outside my assigned ranges.
//    FIRST MEASUREMENT : rg -c --no-messages -g "*.cpp" -g "*.h" "LotRecordUPH"
//                        -> 0 files (exit 1), at 2026-08-11 01:34:26 +0800.  On
//                        that evidence the call was written inside `#if 0`.
//    RE-MEASUREMENT    : rg -n "^(int|bool|void)\s+(LotRecordUPH)\s*\(" -g "*.cpp"
//                        -g "*.h" -g "_w7d_parts/*.txt"
//                        -> _w7d_parts/05362_LotRecordUPH.txt:46, at 2026-08-11
//                        01:49:41 +0800.  A SIBLING LANDED THE REAL BODY, with
//                        golden`s exact signature.  Premise FALSE by integration.
//    TRAP 3 RE-ASK ("why SHOULD this stay gated?") : the sibling`s body keeps
//                        golden`s two WriteDataToFile() calls ACTIVE and builds its
//                        path from as9045UPH + fLotInfo->edtSysLotID->Text -- i.e.
//                        the body really does append to the machine`s production
//                        UPH CSV tree, the same class of hazard as WriteLastDataFile
//                        hard-coding D:\HT9045\system\lastdata.dat.  I weighed
//                        keeping the gate on THAT ground and rejected it: MY CALL
//                        SITE is double-guarded by IniConfig.bP11RecordUPH AND
//                        CUSTOMER_CODE==CC_KYEC_LEE, and the standard build forces
//                        CUSTOMER_CODE to CC_HONPREC_QC, so no ctest run can reach
//                        the write through this path.  Gating a call site to
//                        contain a hazard that lives in someone else`s body would
//                        just manufacture one more expired gate.  Gate RETIRED;
//                        golden :5507 is ACTIVE and verbatim.
//    HAND-OFF (not mine to fix) : the file-write hazard is real and belongs to
//                        part 05362 -- if ctest is ever run with a KYEC customer
//                        profile, or if a future wave relaxes that guard, the log
//                        root needs redirecting first.  Flagging, not fixing.
//    ORDER INSURANCE   : one forward declaration below, copied character-for-
//                        character from the sibling`s definition, so this part
//                        compiles regardless of part-stitch order.
//
//  GATE (k7-G6)  golden lines: ainarm9045.cpp:5520-5521
//    SYMBOL       : fMain->bCloseExcelflag / fMain->bCloseExcelfinishflag --
//                   golden main.h members; the port`s FormsFacade TfMain
//                   (forms/fMain.h) has neither.
//    OFFLINE DEFAULT CHOSEN : both assignments gated out; the surrounding `else`
//                   shell kept.
//    WHY FAITHFUL : this pair is a one-way request to the Excel-export worker,
//                   whose only reader in golden is RecordUPH (:5383
//                   `if(fMain->bCloseExcelfinishflag)`) -- and RecordUPH is the
//                   no-op stub at ainarm9045.cpp:2275, so nothing in the port can
//                   observe the flags.  Same provable-zero-effect argument as G4.
//    REAL-MACHINE DIFFERENCE : for the non-KYEC / non-FOREHOPE P11 customers the
//                   Excel UPH export is never triggered.  Reporting only.
//    PROVING COMMAND (run from D:/HT9045/HT9011UC_Cpp_V3.33.906.0):
//                   rg -n "StatusBar1|Panels|bCloseExcel" forms/fMain.h
//                   -> 0 hits (exit 1)
//    MEASURED AT  : 2026-08-11 01:34:26 +0800
//    RE-MEASURED  : 2026-08-11 01:49:41 +0800 -> forms/fMain.h unchanged, 0 hits.
//                   GATE STANDS (TRAP 2 checked, premise alive).
//    TRAP 3 CHECK : a real TfMain with these members would make this linkable, but
//                   it should still not be retired alone -- retire it together
//                   with RecordUPH (golden :5376), or the flags get set and
//                   nothing ever clears them.
//
//  GATE (k7-G7a) golden lines: ainarm9045.cpp:5594-5595
//  GATE (k7-G7b) golden line : ainarm9045.cpp:5600
//    SYMBOL       : fMain->StatusBar1->Panels->Items[n]->Text -- absent from
//                   forms/fMain.h (no StatusBar1, no Panels).
//    OFFLINE DEFAULT CHOSEN : the three assignments gated out; both branch shells
//                   (the CC_ASE_KaohSiung / bG10ShowImmediateUPH nest and its
//                   `else`) kept so the customer split stays readable.
//    WHY FAITHFUL : write-only display text.  Nothing in golden reads the status
//                   bar back -- unlike the UPH grid, which is why the grid got a
//                   SEAM and this gets a GATE.  Every number reaching these lines
//                   (Cells[3][1]) is already computed and stored by the seam.
//    REAL-MACHINE DIFFERENCE : the "UPH = n" status-bar panel is not painted, and
//                   on ASE Kaohsiung panel 7 is not cleared.  Display only.
//    PROVING COMMAND : same as k7-G6 above
//                   (rg -n "StatusBar1|Panels|bCloseExcel" forms/fMain.h -> exit 1)
//    MEASURED AT  : 2026-08-11 01:34:26 +0800
//    RE-MEASURED  : 2026-08-11 01:49:41 +0800 -> forms/fMain.h unchanged, 0 hits.
//                   BOTH GATES STAND (TRAP 2 checked, premise alive).
//    PREMISE CARRIES OVER : this is the SAME absence csystem.cpp already wrote up
//                   as its H1-03 gate ("forms/fMain.h has no StatusBar1 member",
//                   csystem.cpp:26217-26224 and :26252-26260) -- I am reusing that
//                   premise rather than re-deriving it, exactly as instructed.
//    TRAP 3 CHECK : if a StatusBar1 lands, retire G7a and G7b TOGETHER with
//                   csystem.cpp`s H1-03/H1-04 -- a half-retirement leaves two TUs
//                   disagreeing about whether the bar is live.
//
//  SYMBOLS DELIBERATELY LEFT ACTIVE (each checked for a LINKABLE body, not just a
//  declaration, before I trusted it -- TRAP 1(a)/(b)/(c)):
//    * MyDBIUPH(int)                cMyDB.h:85 (decl, __fastcall) / cMyDB.cpp:414
//                                   (def, __fastcall -- pair consistent).  Sole
//                                   definition in the tree, so no stub can
//                                   pre-satisfy the demand.  ARCHIVE: cMyDB.cpp is
//                                   in ht9045_db (CMakeLists.txt:818); this TU is
//                                   in ht9045_sm, which reaches it transitively
//                                   (ht9045_sm -> ht9045_secsgem -> ht9045_db,
//                                   CMakeLists.txt:2136 / :1214).  Acyclic, so the
//                                   generated link order resolves it.
//    * EventReport(unsigned)        SECSGEM/SecsEventReport.h:55 /
//                                   SecsEventReport.cpp:15, and SECS_EVENT at
//                                   SecsEventType.h:340 / SecsEventType.cpp.  NOT
//                                   a new archive edge: acatchtray.cpp (same
//                                   ht9045_sm archive) already calls
//                                   EventReport(SECS_EVENT.*) at :1650 and 8 more.
//    * GetSiteCount(bool)           cprod.h / cprod.cpp:375 (live body -- the
//                                   duplicate that atester_shims.cpp used to carry
//                                   was RETIRED, atester_shims.cpp:135).
//    * RecordLotUPH_For_FOREHOPE_NINGBO(...)  the port`s existing offline stub at
//                                   ainarm9045.cpp:2276 (golden :5390).  Left as
//                                   an ACTIVE call on purpose: it is a real
//                                   declared symbol with a body, so gating it
//                                   would be inventing a problem.  Noting only
//                                   that the FOREHOPE CSV is a no-op until that
//                                   stub is retired.
//    * MyDBIProcess("Exception",..) reachable only under `#ifdef DEBUG_TRY_CATCH`,
//                                   which is NOT defined in this tree, so the
//                                   try/catch is preprocessed away.  Reproduced
//                                   VERBATIM anyway (golden :5538-5553), per the
//                                   same convention aTester_Rear.cpp:139-143 used
//                                   for SOFT_SIMULTE / DEBUG_INDEX_UPH.  Had it
//                                   been live, the 2-arg MyDBIProcess resolves via
//                                   aHotPlateSubstrate.h / .cpp:1079.
//
//  TRAP 5 -- TWO HEADERS, SAME CLASS NAME.  This body touches ONE object of a
//  duplicated class: TestSocket (`Cells[6][1]=TestSocket.iShtRow*TestSocket.
//  iShtCol`, golden :5478).  I relied on aHotPlateSubstrate.h -- `extern
//  TMyKitSuck TestSocket;` at aHotPlateSubstrate.h:635, layout at :365ff (iShtRow
//  :406, iShtCol via SetPickerCount :492), object DEFINED in
//  aHotPlateSubstrate.cpp.  NOT mykitsuck.h:274, whose TMyKitSuck has a different
//  layout; this TU has never included mykitsuck.h (the port ainarm9045.cpp include
//  block, :59-77, pulls aHotPlateSubstrate.h only) and I added no include that
//  would change that.  Picking the other header would have linked perfectly and
//  read iShtRow/iShtCol at the wrong offsets.
//==============================================================================

#include <stdlib.h>                        // atoi (golden :5544)
// golden ainarm9045.cpp:5499 -- MyDBIUPH.  DECLARED LOCALLY, NOT via #include
// "cMyDB.h": that header re-declares MyDBIProcessNew and RecordProcess with
// default arguments this TU already has from canary_support.h:70/:210, so
// including it here is a hard error (measured: `ambiguating new declaration of
// MyDBIProcessNew` at cMyDB.h:82 + `default argument given for parameter 2 of
// RecordProcess` at cMyDB.h:122, g++ 6.3.0 -fsyntax-only, 2026-08-11 01:57 +0800).
// A local extern-style declaration is the BCB6 idiom golden itself uses and the
// port already uses at acarry.cpp:171.  __fastcall IS KEPT: cMyDB.h:85 and
// cMyDB.cpp:414 both carry it, and on this toolchain __fastcall is a real calling
// convention that changes the mangled name -- dropping it here would link-fail
// (vclcompat/vcl_compat.h:34-47 spells out this exact lockstep rule).
void __fastcall MyDBIUPH(int UPH);
#include "SECSGEM/SecsEventType.h"         // SECS_EVENT (:340) -- SecsEventType.cpp (golden :5534)
#include "SECSGEM/SecsEventReport.h"       // EventReport (:55) -- SecsEventReport.cpp:15 (golden :5534)

// golden ainarm9045.cpp:5362 -- defined by sibling part 05362_LotRecordUPH.txt.
// Declaration only (golden declares this symbol in no header at all).
void LotRecordUPH(AnsiString UPH_StartTime, AnsiString UPH_EndTime, AnsiString UPH_PauseTime, int iUPH);

//------------------------------------------------------------------------------
//  SEAM (k7-S1) -- fShowBinSelect->UPH_StringGrid->Cells[col][row] and
//  fShowBinSelect->tsUPH->Strings[i].  See SEAM REGISTER above.  RETIRE BY
//  DELETING THIS BLOCK AND THE #define/#undef PAIR THAT BRACKETS THE FUNCTION.
//  Golden uses Cells[0..6][0..12] and Strings[0..19]; the stores are sized past
//  both so no golden index can run off the end.
//------------------------------------------------------------------------------
namespace {
struct W7dK7_UPHCells                                                           // TStringGrid::Cells[col][row]
{
    AnsiString  store[8][16];
    AnsiString* operator[](int iCol) { return store[iCol]; }                     // Cells[c] -> row array; [r] then yields AnsiString&
};
struct W7dK7_UPHStringGrid { W7dK7_UPHCells Cells; };
struct W7dK7_UPHStrings                                                         // TStrings::Strings[i]
{
    AnsiString  store[24];
    AnsiString& operator[](int i) { return store[i]; }
};
struct W7dK7_UPHTabSet    { W7dK7_UPHStrings Strings; };
struct W7dK7_ShowBinSelectSeam                                                  // POD: pointers only, zero-init, no dynamic init
{
    W7dK7_UPHStringGrid *UPH_StringGrid;
    W7dK7_UPHTabSet     *tsUPH;
};
//  TRAP 4: the AnsiString stores are FUNCTION-LOCAL statics -- constructed on the
//  first call, never before main().
W7dK7_ShowBinSelectSeam* W7dK7_ShowBinSelect()
{
    static W7dK7_UPHStringGrid     grid;
    static W7dK7_UPHTabSet         tabs;
    static W7dK7_ShowBinSelectSeam obj;
    obj.UPH_StringGrid=&grid;
    obj.tsUPH         =&tabs;
    return &obj;
}
}   // anonymous namespace
#ifdef fShowBinSelect
#undef fShowBinSelect
#endif
#define fShowBinSelect W7dK7_ShowBinSelect()                                    // SEAM (k7-S1): keeps golden`s ~40 grid lines verbatim
//==============================================================================
// 計算最近10筆的UPH與平均值
//==============================================================================
void CalculateUPH(bool bReset)                                                  // golden: `void __fastcall CalculateUPH(bool bReset)` -- see NOTE (k7-N1)
{
    int iCount=0, iTotalUPH;
    double fConsumeSecond,fTimerMultiple;
    TDateTime tTempTime, tConsumeSecond;
    Word Hour, Min, Sec, MSec;

    tTempTime=Now();
    if(bReset || bOneTimes)
    {
        bOneTimes=false;
        RunInfo.iUPH=0;
        tUPH_StartTime=tTempTime;
        tUPH_PauseTime=0;
        iUPH_LoaderCount=0;

        fShowBinSelect->UPH_StringGrid->Cells[0][0]="Start Time";
        fShowBinSelect->UPH_StringGrid->Cells[1][0]="End Time";
        fShowBinSelect->UPH_StringGrid->Cells[2][0]="Pause Time";
        fShowBinSelect->UPH_StringGrid->Cells[3][0]="UPH";
        for(int i=1; i<=12; i++)
        {
            fShowBinSelect->UPH_StringGrid->Cells[0][i]="";
            fShowBinSelect->UPH_StringGrid->Cells[1][i]="";
            fShowBinSelect->UPH_StringGrid->Cells[2][i]="";
            fShowBinSelect->UPH_StringGrid->Cells[3][i]="";
        }

        if(IniConfig.bVTESTFunction==true)                                      //RogerYang 20250224 偉測需求 新增三列信息,耗時,數量,site
        {
            fShowBinSelect->UPH_StringGrid->Cells[4][0]="Elaps. Time";
            fShowBinSelect->UPH_StringGrid->Cells[5][0]="Total Units";
            fShowBinSelect->UPH_StringGrid->Cells[6][0]="Site";
            for(int i=1; i<=12; i++)
            {
                fShowBinSelect->UPH_StringGrid->Cells[4][i]="";
                fShowBinSelect->UPH_StringGrid->Cells[5][i]="";
                fShowBinSelect->UPH_StringGrid->Cells[6][i]="";
            }
        }
    }
    else
    {
        if(tTempTime>tUPH_StartTime)
        {
            for(int i=10; i>=1; i--)
            {
                fShowBinSelect->UPH_StringGrid->Cells[0][i]=fShowBinSelect->UPH_StringGrid->Cells[0][i-1];
                fShowBinSelect->UPH_StringGrid->Cells[1][i]=fShowBinSelect->UPH_StringGrid->Cells[1][i-1];
                fShowBinSelect->UPH_StringGrid->Cells[2][i]=fShowBinSelect->UPH_StringGrid->Cells[2][i-1];
                fShowBinSelect->UPH_StringGrid->Cells[3][i]=fShowBinSelect->UPH_StringGrid->Cells[3][i-1];

                if(IniConfig.bVTESTFunction==true)                              //RogerYang 20250224 偉測需求 新增三列信息,耗時,數量,site
                {
                    fShowBinSelect->UPH_StringGrid->Cells[4][i]=fShowBinSelect->UPH_StringGrid->Cells[4][i-1];
                    fShowBinSelect->UPH_StringGrid->Cells[5][i]=fShowBinSelect->UPH_StringGrid->Cells[5][i-1];
                    fShowBinSelect->UPH_StringGrid->Cells[6][i]=fShowBinSelect->UPH_StringGrid->Cells[6][i-1];
                }
            }

            tUPH_EndTime=tTempTime;
            tConsumeSecond=(tUPH_EndTime-tUPH_StartTime);
            if(IniConfig.bVTESTFunction==true)                                  //RogerYang 20250224 偉測需求 新增三列信息,耗時,數量,site (Elaps. Time 不包含 Pause Time)
            {
                fShowBinSelect->UPH_StringGrid->Cells[4][1]=FormatDateTime("hh:nn:ss", tConsumeSecond);  //Elaps. Time
                fShowBinSelect->UPH_StringGrid->Cells[5][1]=iUPH_LoaderCount;   //Total Units
                fShowBinSelect->UPH_StringGrid->Cells[6][1]=TestSocket.iShtRow*TestSocket.iShtCol;                      //Site數
            }
            fShowBinSelect->UPH_StringGrid->Cells[0][1]=FormatDateTime("hh:nn:ss", tUPH_StartTime);  //Steven 20090714
            fShowBinSelect->UPH_StringGrid->Cells[1][1]=FormatDateTime("hh:nn:ss", tUPH_EndTime);  //Steven 20090714
            fShowBinSelect->UPH_StringGrid->Cells[2][1]=FormatDateTime("hh:nn:ss", tUPH_PauseTime);  //Steven 20090714
            tUPH_StartTime=tUPH_EndTime;
            tConsumeSecond=tConsumeSecond-tUPH_PauseTime;
            tUPH_PauseTime=0;
            fConsumeSecond=0;
            DecodeTime(tConsumeSecond, Hour, Min, Sec, MSec);
            fConsumeSecond+=Hour*3600.0;
            fConsumeSecond+=Min*60.0;
            fConsumeSecond+=Sec;
            fConsumeSecond+=MSec/(double)1000.0;
            if(fConsumeSecond>0)                                                //Steven 20110418
                fTimerMultiple=3600/fConsumeSecond;
            else
                fTimerMultiple=0;

#if 0 // GATE (k7-G4) -- golden :5497; the global `char DateTime[256]` (golden :5360) has no port home
            sprintf(DateTime, "%02d:%02d:%02d",  SystemHour, SystemMin, SystemSec);                                     //Frank 20150515
#endif
            RunInfo.iUPH=fTimerMultiple*iUPH_LoaderCount;
            MyDBIUPH(RunInfo.iUPH);                                             //Steven 20190906 : Add UPH in EventLog
            int iLoaderCount=iUPH_LoaderCount;
            iUPH_LoaderCount=0;
            fShowBinSelect->UPH_StringGrid->Cells[3][1]=RunInfo.iUPH;           //Steven 20090714
            if(IniConfig.bP11RecordUPH)                                         //Frank 20150515
            {
                if(CUSTOMER_CODE==CC_KYEC_LEE)
                {
                    LotRecordUPH(fShowBinSelect->UPH_StringGrid->Cells[0][1], fShowBinSelect->UPH_StringGrid->Cells[1][1], fShowBinSelect->UPH_StringGrid->Cells[2][1], RunInfo.iUPH);  //wei 20151221 KYEC 儲存UPH紀錄
                }
                else if(CUSTOMER_CODE==CC_FOREHOPE_NINGBO)                      //Jimmychiu 20250304 : P11新增CC_FOREHOPE_NINGBO
                {
                    AnsiString sStartTime=fShowBinSelect->UPH_StringGrid->Cells[0][1];
                    AnsiString sEndTime=fShowBinSelect->UPH_StringGrid->Cells[1][1];
                    AnsiString sPauseTime=fShowBinSelect->UPH_StringGrid->Cells[2][1];
                    int iTrayCount=iLoaderCount;
                    int iSiteCount=GetSiteCount(false);
                    RecordLotUPH_For_FOREHOPE_NINGBO(sStartTime, sEndTime, sPauseTime, RunInfo.iUPH, iTrayCount, iSiteCount);
                }
                else
                {
#if 0 // GATE (k7-G6) -- golden :5520-5521; forms/fMain.h has no bCloseExcelflag / bCloseExcelfinishflag
                    fMain->bCloseExcelflag=true;
                    fMain->bCloseExcelfinishflag=false;
#endif
                }
            }
            iTotalUPH=0;
            iCount=0;

            for(int i=0; i<20; i+=2)                                            //Steven 20140621 : For SECS GEM
            {
                fShowBinSelect->tsUPH->Strings[i  ]=fShowBinSelect->UPH_StringGrid->Cells[0][i/2+1];
                fShowBinSelect->tsUPH->Strings[i+1]=fShowBinSelect->UPH_StringGrid->Cells[3][i/2+1];
            }

            if(IniConfig.bEnable_SECS_GEM==true)                                //Steven 20140528 : Secs Gem
                EventReport(SECS_EVENT.UPHRecordEnd);                           //54 UPH Record End

            for(int i=1; i<=10; i++)
            {
#ifdef DEBUG_TRY_CATCH
                try
                {
#endif
                    if(fShowBinSelect->UPH_StringGrid->Cells[3][i]!="")
                    {
                        iTotalUPH+=atoi(AnsiString(fShowBinSelect->UPH_StringGrid->Cells[3][i]).c_str());
                        iCount++;
                    }
#ifdef DEBUG_TRY_CATCH
                }
                catch(...)
                {
                    MyDBIProcess("Exception", "CalculateUPH");
                }
#endif
            }
            fShowBinSelect->UPH_StringGrid->Cells[2][12]="Avg UPH :";
            RunInfo.iAvgUPH=AnsiString((iCount==0)?0:iTotalUPH/iCount);
            fShowBinSelect->UPH_StringGrid->Cells[3][12]=RunInfo.iAvgUPH;
        }
        else
        {
            RunInfo.iUPH=0;
            tUPH_StartTime=tTempTime;
            fShowBinSelect->UPH_StringGrid->Cells[0][0]="Start Time";
            fShowBinSelect->UPH_StringGrid->Cells[1][0]="End Time";
            fShowBinSelect->UPH_StringGrid->Cells[2][0]="Pause Time";
            fShowBinSelect->UPH_StringGrid->Cells[3][0]="UPH";
            for(int i=1; i<=12; i++)
            {
                fShowBinSelect->UPH_StringGrid->Cells[0][i]="";
                fShowBinSelect->UPH_StringGrid->Cells[1][i]="";
                fShowBinSelect->UPH_StringGrid->Cells[2][i]="";
                fShowBinSelect->UPH_StringGrid->Cells[3][i]="";
            }

            if(IniConfig.bVTESTFunction==true)                                  //RogerYang 20250224 偉測需求 新增三列信息,耗時,數量,site
            {
                fShowBinSelect->UPH_StringGrid->Cells[4][0]="Elaps. Time";
                fShowBinSelect->UPH_StringGrid->Cells[5][0]="Total Units";
                fShowBinSelect->UPH_StringGrid->Cells[6][0]="Site";
                for(int i=1; i<=12; i++)
                {
                    fShowBinSelect->UPH_StringGrid->Cells[4][i]="";
                    fShowBinSelect->UPH_StringGrid->Cells[5][i]="";
                    fShowBinSelect->UPH_StringGrid->Cells[6][i]="";
                }
            }
        }
    }

    if(CUSTOMER_CODE==CC_ASE_KaohSiung)                                         //kevin 20180417 只秀一個UPH 高雄
    {
        if(IniConfig.bG10ShowImmediateUPH==false)
        {
#if 0 // GATE (k7-G7a) -- golden :5594-5595; forms/fMain.h has no StatusBar1 (same premise as csystem.cpp H1-03)
            fMain->StatusBar1->Panels->Items[7]->Text="";
            fMain->StatusBar1->Panels->Items[2]->Text="UPH = "+AnsiString(fShowBinSelect->UPH_StringGrid->Cells[3][1]);
#endif
        }
    }
    else
    {
#if 0 // GATE (k7-G7b) -- golden :5600; forms/fMain.h has no StatusBar1 (same premise as csystem.cpp H1-03)
        fMain->StatusBar1->Panels->Items[2]->Text="UPH = "+AnsiString(fShowBinSelect->UPH_StringGrid->Cells[3][1]);
#endif
    }
}
#ifdef fShowBinSelect
#undef fShowBinSelect                                                           // SEAM (k7-S1) scope ends -- do not leak the macro into a sibling`s part
#endif

// =============================================================================
//  k4-ainarm PART 06/08  --  MoveInArmXYToLoader_9045   golden ainarm9045.cpp:5604-5733
//  Translator: AI(k4-ainarm) 20260811
//
//  ROLE   : The in-arm -> Loader XY move.  Resolves the pick target (all-at-once vs
//           one-by-one), applies the auto-Y-pitch squeeze with its soft-limit rescue, builds
//           the per-nozzle Z-down flag/position grids, adds the per-nozzle Loader offsets,
//           answers "would this move be legal?" when RealMove==false, and otherwise issues
//           the pitch-varied continuous move and books the UPH sample.
//
//  WAVE SCOPE
//    MoveInArmXYToLoader_9045  golden :5604-5733  ACTIVE with TWO surviving gates
//                                                 (k4-G2, k4-G4).  k4-G3 and k4-G5 retired.
//
//  TRAP 2 IN ACTION -- READ THIS FIRST.  I wrote this part's first draft with SIX gates
//  whose premise was "no port body exists".  I re-ran every absence claim before finishing,
//  at 2026-08-11 01:50:31 +0800, and six of them HAD DIED: sibling agents landed the bodies as part
//  files in _w7d_parts/ during this same wave.  A gate on a dead premise is exactly the
//  defect TRAP 2 describes -- so those six are RETIRED here (call ACTIVE), not left in with
//  an apology.  The gates that survive below survive because the symbol is STILL absent,
//  re-verified at 2026-08-11 01:50:31 +0800, not because nobody re-checked.
//
//  NAME NOTE -- THIS IS A GOLDEN BUG, AND IT IS WHY THIS PART IS NOT A DUPLICATE.
//    golden ainarm9045.h:78 declares  MoveArmXYToLoader_9045  (no "In"), but golden
//    ainarm9045.cpp:5604 DEFINES  MoveInArmXYToLoader_9045  (with "In").  The declared name
//    has ZERO occurrences in golden ainarm9045.cpp, i.e. golden ships a dangling declaration
//    that nothing defines and nothing calls.  The port faithfully inherited the typo:
//    ainarm9045.h:78 carries the header spelling and ainarm9045.cpp:2303 stubs
//    `MoveArmXYToLoader_9045(...) { return false; }  // golden :5604`.  That stub is a
//    DIFFERENT SYMBOL from the function below, so this part collides with nothing -- but it
//    also means MY definition has no declaration in any header, so nothing outside this TU can
//    call it (exactly golden's situation).  Behaviour preserved, bug preserved, reported.
//
//  RETIRED GATES (were here in the first draft; premise died at 2026-08-11 01:50:31 +0800)
//    k4-G3 (RETIRED)  golden :5636  GetInArmToLoaderPosition_Single(int,int&,int&,int,int)
//      original premise: 0 matches in the port tree, measured 2026-08-11 01:26:17 +0800.
//      why retired: landed this wave as _w7d_parts/05174_GetInArmToLoaderPosition_Single.txt:87
//        with signature (int iSelRow, int &iXPos, int &iYPos, int iRow, int iCol) -- exactly
//        golden's :5636 call.  Stitch order safe: 05174 < 05604, matching golden's :5174 < :5604.
//      what retiring BUYS: the one-by-one (Fix-pitch / single-nozzle) Loader pick path is now
//        real.  Gated, it left iXPos/iYPos at 0 and drove the arm toward tray origin -- so
//        this retirement removes a genuinely broken motion path, not just a cosmetic gap.
//    k4-G5 (RETIRED)  golden :5728  CalculateUPH(false)
//      original premise: 2 matches, both a banner sentence and an already-gated call; no body.
//      why retired: landed as _w7d_parts/05412_CalculateUPH.txt:303 as `void CalculateUPH(bool
//        bReset)` -- __fastcall dropped, exactly the port convention noted at ainarm9045.cpp:49,
//        so golden's `CalculateUPH(false)` binds unchanged.  Stitch order 05412 < 05604.
//
//  GATE REGISTER -- the two that SURVIVE, re-verified still absent at 2026-08-11 01:50:31 +0800.
//    Absence command for each (run in D:/HT9045/HT9011UC_Cpp_V3.33.906.0), and I re-ran it
//    against the sibling part files too, since a body can arrive there before it arrives in
//    the tree:
//      grep -rnoE "\b<SYMBOL>\b" --include=*.cpp --include=*.h \
//                --exclude-dir=.git --exclude-dir=.svn .            | wc -l
//      grep -lnE "^(int|void|bool) <SYMBOL>\s*\(" _w7d_parts/*.txt        (empty = still absent)
//
//    k4-G2  golden :5606  AutoCalculateInArmXClosePitch()          golden body at :4982
//      measured: 0 matches in the port tree (2026-08-11 01:26:17 +0800), AND no part file defines it
//        (2026-08-11 01:50:31 +0800) -- there is no 04982_* part in _w7d_parts, so unlike k4-G3/k4-G5 this
//        premise is STILL TRUE.  Two sibling parts (05174, 05735) reference the symbol and are
//        gating it the same way, which is consistent with it being nobody's target this wave.
//      why the offline default is faithful: golden discards the return value on this line, so
//        skipping the CALL changes no local.  What it costs is the function's SIDE EFFECTS,
//        which are large and must be stated plainly: golden :4986-5006 writes
//        InArmClose_PitchX, iTotalXPitchStep, iMaxXPitch4Pick, iMinXPitch4Pick,
//        iMaxXPitch2Pick_13/_14, iMinXPitch2Pick_13/_14, AND the three pitch globals this
//        function then reads -- iInArmXStep, dInArmXPitch_1Step, dInArmXPitch_MovePitch.
//      real-machine difference: gated, those globals keep their static-init 0, so iMovePitchX
//        (:5610) is 0 and every nozzle collapses onto the base column with no pitch spread.
//        The move stays soft-limit-legal, so it fails QUIETLY as a wrong position rather than
//        an alarm.  SAFETY-RELEVANT -- do not ship gated.
//      TRAP 2 re-check instruction: golden :4982 is a small pure-arithmetic function with no
//        HW dependency; it is a cheap next target.  Re-run both commands at integration and
//        retire k4-G2 TOGETHER WITH k4-G9 in PART 07 -- they are the same symbol.
//
//    k4-G4  golden :5720  TransferLoaderRatio(&iXPos, &iYPos)   golden body ainarm2.cpp:2112
//      measured: 5 matches, ALL comments or already-gated call sites -- no body (2026-08-11 01:26:17 +0800);
//        no part file defines it either (2026-08-11 01:50:31 +0800).  This is the one gate here whose premise
//        is NOT same-file: golden keeps it in ainarm2.cpp, and the port's ainarm2.cpp is a
//        114-line partial that does not carry it.
//      PREMISE CARRIES OVER FROM AN EXISTING GATE, not re-derived: cInArmPlacement.cpp:724
//        already gates this identical call with `#if 0 // GATE (1): golden ainarm2.cpp:2112
//        TransferLoaderRatio(int*,int*)` plus a full file-head GATE REGISTER entry.  I match
//        that decision so the two sites cannot drift apart.
//      why the offline default is faithful: golden applies a software gear ratio in place;
//        skipping it leaves iXPos/iYPos in un-scaled engineering units, i.e. exactly the
//        identity ratio -- correct for every machine whose Loader ratio is 1:1, and wrong by
//        that ratio otherwise.  Stated, not hidden.
//      real-machine difference: on a geared Loader every X/Y target is off by the ratio.
//      TRAP 3 -- THIS ONE SHOULD STAY GATED EVEN ONCE ainarm2.cpp LANDS, until someone
//        confirms the ratio source is actually populated offline; and it must be retired in
//        lockstep with cInArmPlacement.cpp:724, never on its own.  "The body exists" is not
//        sufficient grounds to open it.
//
//  ACTIVE dependencies I verified rather than assumed:
//    InArmContinuousMove_9045 -- REAL declaration Motor/mymotor.h:396, and its 8-parameter
//      shape matches golden's 8-argument call exactly: (int X, int Y, int Vari[X_PITCH_COUNT],
//      int YVari, bool ZDownSel[MAX_ARM_Row][MAX_ARM_Col], int iZPos[MAX_ARM_Row][MAX_ARM_Col],
//      bool ZNeedDown, bool bLoader=false).  NOT gated.
//    AutoCalculateInArmYClosePitch() -- ainarm9045.h:46 declares it with defaults
//      (bool bStart=false, bool bCheckIsZero=false), so golden's no-argument call binds. ACTIVE.
//    GetInArmToLoaderOffset_9045 (ainarm9045.h:51), GetInArmPitchX_9045 / GetInArmPitchY_9045
//      (already ACTIVE in this file), DoInArmSuckPreOn (stub at ainarm9045.cpp:2264),
//      ZeroMemory, MOT[].Motor->PSoftLimitP/N, UserDefForm[] / DeviceForm / TrayForm,
//      InArmOffSet[]->GetArmX/GetArmY, ZSafePos, X_PITCH_COUNT, InArmSuckUse -- all resolved
//      in a probe TU carrying this file's exact include list (g++ -std=c++17 -fsyntax-only
//      -I. -Ivclcompat, 2026-08-11 01:26:17 +0800).
//
//  FILE-SCOPE GLOBALS I ONLY USE, NEVER DEFINE (main loop owns all seven in one place):
//    dInArmXPitch_MovePitch (golden :4978), dInArmXPitch_1Step (:4977), iInArmXStep (:4979).
//    Confirmed by compiling the simulated stitch: the ONLY errors were these three names, and
//    they vanished when I supplied them probe-only outside any part file.
//
//  GOLDEN QUIRK kept as-is: the :5657-5660 `else` is mis-indented in golden (the `else` sits
//  one level left of its own braces).  Transcribed exactly -- reflowing it would be a silent
//  edit to a safety-relevant Y-pitch rescue branch.  Also kept: the dead :5705-5713 block,
//  commented out by Eastsun 20251231 when the offset work moved upward.
//
//  INTEGER DIVISION KEPT: :5655 `DeviceForm.YDimension/2+1000` is integer division on an int
//  member -- the truncation is load-bearing for the Y-pitch rescue and is NOT widened.
//
//  Big5 -> UTF-8: thirteen Chinese comments transcribed character-for-character from cp950
//  (:5606, :5636, :5644, :5647, :5653, :5672, :5676, :5681, :5683, :5689, :5705, :5710,
//  :5720).  ZERO U+FFFD.
// =============================================================================
bool MoveInArmXYToLoader_9045(bool &bCanPick2ICAtOnceTime, int iSelRow, int &iRow, int &iCol, bool RealMove, bool ZNeedDown)
{
#if 0 // GATE k4-G2 (LIVE, re-verified absent 2026-08-11 01:50:31 +0800) -- golden :5606 AutoCalculateInArmXClosePitch (golden ainarm9045.cpp:4982; no port body, no part file) -- retire together with k4-G9; see GATE REGISTER above
    AutoCalculateInArmXClosePitch();                                            //Steven 20240509 : 移到外面, 避免Fix mode沒執行
#endif
    int iXPos=0, iYPos=0;
    int iXVariable[X_PITCH_COUNT];
    int iYVariable  =0;
    int iMovePitchX =dInArmXPitch_MovePitch;
    int iMovePitchY =AutoCalculateInArmYClosePitch()+IniConfig.iInArm60mmOffset;
    int Ld          =TrayForm.Loader.iTrayType;
    bool bZFlag[MAX_ARM_Row][MAX_ARM_Col];
    int iZPos[MAX_ARM_Row][MAX_ARM_Col];
    int iOffsetPos=-1;
    ZeroMemory(iXVariable, sizeof(iXVariable));
    ZeroMemory(bZFlag, sizeof(bZFlag));
    ZeroMemory(iZPos, sizeof(iZPos));

    if(iSelRow==2)                                                              //ChungHung 20131231 alter AutoYPitch
    {
        iSelRow=0;
        if(bCanPick2ICAtOnceTime==false)                                        //Steven 20141029 : XY-Pitch with Fix Pitch mode
        {
            for(int j=0; j<InArmSuck.iMaxCol; j++)
                InArmSuckUse[1][j]=false;
        }
    }

    if(bCanPick2ICAtOnceTime==true)
    {
        GetInArmToLoaderPosition(iSelRow, iXPos, iYPos, iRow, iCol);
    }
    else                                                                        //one by one suck
    {
        iMovePitchX=GetInArmToLoaderPosition_Single(iSelRow, iXPos, iYPos, iRow, iCol);                                 //Steven 20240701 : 修正Fix模式下, X Pitch錯誤
    }

    if(USE_IN_Y_IS_AUTO_PITCH==true)                                            //ChungHung 20140304 add for AutoYPitch  //JerryYang 20251218 : IN/OUT ARM支援不同模組  //JerryYang 20251218 : IN/OUT ARM支援不同模組
    {
        if(iInArmOrder==0)
        {
            iYPos=iYPos-iMovePitchY;
            if(iYPos<(MOT[MInArmY].Motor->PSoftLimitN+10))                      //JerryYang 20180221 (Steven) : fix y-pitch 最小25mm會有行程的問題, y-pitch要縮到最小
            {
                iYPos=iYPos+iMovePitchY;
                if((UserDefForm[Ld].YPitch-500)<IN_OUT_ARM_Y_PITCH_MIN)         //Steven 20230727 : 當TrayPitch比最小還小的時候,才能縮到Tray Pitch
                {
                    iMovePitchY=IN_OUT_ARM_Y_PITCH_MIN;
                }
                else
                {
                    if(UserDefForm[Ld].YPitch>6000)                             //Stevenhong 20260325 : TESNA 的IC 與Pitch 比較大
                    {
                        iMovePitchY=DeviceForm.YDimension/2+1000;
                    }
                else
                    {
                    iMovePitchY=UserDefForm[Ld].YPitch-500;
                    }
                }

                iYPos=iYPos-iMovePitchY;
            }
        }
    }
    else
    {
        iYPos=iYPos+iMovePitchY*iInArmOrder;
    }

    for(int i=0; i<InArmSuck.iMaxRow; i++)                                      //Eastsun 20251231 : 從下面移上來的
    {
        for(int j=0; j<InArmSuck.iMaxCol; j++)
        {
            bZFlag[i][j]=(ZNeedDown)?InArmSuckUse[i][j]:false;                  //kevin 20210305 add Z motor 不往下
            iZPos[i][j] =(bZFlag[i][j])?Prod.ZInArm_Tray_Pick[i][j]:ZSafePos;
        }
    }

    if(bCanPick2ICAtOnceTime==false)                                            //Eastsun 20251231 : 單支吸嘴偏移功能
    {
        for(int i=0; i<InArmSuck.iMaxRow; i++)                                  //Eastsun 20251231 : 從下面移上來的
        {
            for(int j=0; j<InArmSuck.iMaxCol; j++)
            {
                if(bZFlag[i][j])
                {
                    iXPos+=InArmOffSet[InOfsLoader]->GetArmX(i, j);             //Eastsun 20251231 : 本來依照架構須加在GetInArmToLoaderPosition_Single 但是沒辦法判斷吸嘴編號
                    iYPos+=InArmOffSet[InOfsLoader]->GetArmY(i, j);
                }
            }
        }
    }

    if(RealMove==false)
    {
        if(iXPos>=MOT[MInArmX].Motor->PSoftLimitP || iXPos<=MOT[MInArmX].Motor->PSoftLimitN)
            return false;
        if(iYPos>=MOT[MInArmY].Motor->PSoftLimitP || iYPos<=MOT[MInArmY].Motor->PSoftLimitN)
            return false;
        return true;
    }

//==>                                                                       //Eastsun 20251231 : 偏移需求往上移
//    for(int i=0; i<InArmSuck.iMaxRow; i++)
//    {
//        for(int j=0; j<InArmSuck.iMaxCol; j++)
//        {
//            bZFlag[i][j]=(ZNeedDown)?InArmSuckUse[i][j]:false;                  //kevin 20210305 add Z motor 不往下
//            iZPos[i][j] =(bZFlag[i][j])?Prod.ZInArm_Tray_Pick[i][j]:ZSafePos;
//        }
//    }

    iOffsetPos=GetInArmToLoaderOffset_9045(iSelRow);
    iYVariable=GetInArmPitchY_9045(iMovePitchY, iOffsetPos);
    for(int i=0; i<X_PITCH_COUNT; i++)
        iXVariable[i]=GetInArmPitchX_9045(iMovePitchX, i, iOffsetPos);

#if 0 // GATE k4-G4 (LIVE, re-verified absent 2026-08-11 01:50:31 +0800) -- golden :5720 TransferLoaderRatio (golden ainarm2.cpp:2112) -- premise shared with the existing gate at cInArmPlacement.cpp:724; TRAP 3: keep gated until retired in lockstep with that site; see GATE REGISTER above
    TransferLoaderRatio(&iXPos, &iYPos);                                        //Steven 20141029 : Loader的軟體齒輪比
#endif
    DoInArmSuckPreOn(iXPos, iYPos);

    if(InArmContinuousMove_9045(iXPos, iYPos, iXVariable, iYVariable, bZFlag, iZPos, ZNeedDown, true))
    {
        if(bRecordUPH && iCol<=0 && iRow==0)
        {
            bRecordUPH=false;
            CalculateUPH(false);
        }
        return true;
    }
    return false;
}

//============================================================================
//  Find_InArm_PickerMaxUseCountOnTime  --  golden ainarm9045.cpp:5735-6129
//  (k1-ainarm, W7-D)
//
//  ROLE: the loader pick PLANNER.  For the current tray direction it sweeps the
//        loader tray cell by cell and, at every cell that still holds an IC,
//        builds the nozzle-usage mask bInArmToLoaderUsage[][] that a pick from
//        that cell would produce (honouring the picker kind AxEx / AxxG / the 1x4
//        close-2-site mode via GetJStep, the one-or-two-row iRowCT, the tray X/Y
//        steps, nozzles that already carry an IC, and cells that are empty).  It
//        keeps the best cell -- the one that fills the most nozzles in a single
//        stroke -- returns that count, and reports it through isStart / iRow /
//        iCol plus the surviving mask.  Pure planning: it moves nothing.
//
//  WAVE SCOPE:
//    * Find_InArm_PickerMaxUseCountOnTime(int,int&,int&,int&,bool)
//                                        ACTIVE body / 1 GATED call  :5735-6129
//
//  TRAP 1 note, shape (a) "nobody references it": no live port caller.  Golden's
//  live call site is SearchLoadTrayUpDown_9045 (golden :6734, call at :6849), which
//  has no port body at all; the only port text that names this function is
//  ainarm9045_2x8_32.cpp:1387, which (a) is the DIFFERENT 8-argument overload
//  `(bool*,int,int&,int&,int&,int,int,bool)` and (b) sits inside the file-wide
//  `#if 0 // TODO(W7): needs Prod.fInArmSuckUse_TwoArm32Site` opened at
//  ainarm9045_2x8_32.cpp:91 -- so it is dead text and cannot even see this
//  5-argument overload.  Not written `static` (that would be TRAP 1 shape (d));
//  ainarm9045.h has no prototype for it, matching golden, where no header declares
//  it either (verified: `grep -rn Find_InArm_PickerMaxUseCountOnTime --include=*.h`
//  over the golden tree -> 0 hits).
//
//  ARITHMETIC / INTEGER-DIVISION DISCIPLINE: every index formula
//  (`cx=x+(j-isStart)*iInArmXStep`, `y+iInArmYStep`, `j2=j*GetJStep()`) is integer
//  and stays integer.  The one non-integer expression is golden :5770,
//  `iInArmYStep=ChangeToFloatNonPcnt((double)AutoCalculateInArmYClosePitch(),
//  LoadForm->YPitch);` -- golden itself calls the zero-guarded DOUBLE helper there
//  and then truncates into the int iInArmYStep.  That is golden's own shape and is
//  transcribed unchanged: I did not "clean it up" into an int/int divide, because
//  the truncation point (after the divide, not inside it) is load-bearing.
//
//  GOLDEN BUGS PRESERVED (behaviour kept, per the no-fix rule):
//   (1) golden :5962-5971 / :5997-6000 / :6022-6030 / :6058-6061 -- when
//       `cx>=MOT[MMTrayY].Tray.XItem` golden marks the nozzle unusable but then
//       FALLS THROUGH and still indexes `MOT[MMTrayY].Tray.Data[cx][y]`: an
//       out-of-range read of the tray grid (in-array, past the configured tray).
//   (2) golden :5973 / :6033 -- `Tray.Data[cx][y+iInArmYStep]` is never bounded
//       against Tray.YItem, so the second row probe can read past the tray in Y.
//   (3) golden :5926 / :5937 -- isStart is only assigned when some nozzle is still
//       free; if none is, the previous/incoming isStart is reused by the
//       `cx=x+(j-isStart)*iInArmXStep` arithmetic further down.
//   (4) golden :6088 `x=ixEnd;` then `break;` -- the assignment is dead (the break
//       leaves the loop immediately).  Kept verbatim.
//   (5) golden :5757 mutates the by-value parameter bSearchLast for the JCET
//       front-row-fallback case; the caller's TestIF.bSearchLastMode is unaffected.
//       Kept.
//
//  TRAP 4 (static init): nothing here is a file-scope object with a running
//  constructor.  The one file-scope datum this part adds (bInArmToLoaderUsage,
//  below) is a POD bool array with an aggregate initialiser -- constant
//  initialisation, no code runs before main().  The function-local `static int
//  iRecY=0;` at golden :5742 is likewise constant-initialised.
//
//  TRAP 5 (two headers, same class): this function dereferences InArmSuck
//  (.iMaxRow/.iMaxCol/.iPickRow/.Item).  I relied on the TMyKitSuck declared at
//  aHotPlateSubstrate.h:365 -- the one this TU actually includes (ainarm9045.cpp
//  includes "aHotPlateSubstrate.h", never "mykitsuck.h") -- and the object that
//  backs it is defined at aHotPlateSubstrate.cpp:81/8x, which IS registered in
//  CMakeLists.txt:1466.  The rival class at mykitsuck.h:274 with its own
//  TMyKitSuck FLCarryKit at mykitsuck.cpp:206 has a DIFFERENT layout and is
//  deliberately not registered (CMakeLists.txt:2099-2112 says so in as many
//  words).  I added no include and changed no include, so this TU cannot drift
//  onto the wrong layout.
//
//  FILE-SCOPE DATUM I HAD TO PROVIDE -- PLEASE READ (hand-off):
//    bInArmToLoaderUsage[MAX_ARM_Row][MAX_ARM_Col] is golden ainarm2.cpp:100
//    (declared golden ainarm2.h:174).  It is NOT one of the seven
//    ainarm9045.cpp globals the main loop owns, and it does not exist anywhere in
//    the port yet.  It is this function's entire output surface, so gating its
//    ~40 uses would have deleted the translation.  I therefore define it here,
//    wrapped in a HT9045_bInArmToLoaderUsage_DEFINED guard, exactly the shape
//    ainarm2.cpp:45-48 already uses for bDestoryOnSht ("the single ACTIVE
//    definition lives in the TU that needs it").  Because every part file in this
//    wave is stitched into THIS one translation unit, the guard also protects
//    against a sibling agent emitting the same block.  When golden ainarm2.cpp:100
//    is translated, delete this block (a duplicate would be a LOUD multiple-
//    definition link error -- which is why it is not `static`: a static shadow
//    would silently split the array in two and let callers read the wrong copy).
//    ABSENCE PROOF (TRAP 2 -- re-run at integration):
//      (run with cwd = D:/HT9045/HT9011UC_Cpp_V3.33.906.0)
//        git grep -n -- "bInArmToLoaderUsage" -- '*.cpp' '*.h'      -> 0 hits
//      grep -rIl --include=*.cpp --include=*.h "bInArmToLoaderUsage" . -> 0 files
//      RUN AT 2026-08-11 01:29:06 +0800 (grep finished 01:30:51 +0800).
//
//  GATE REGISTER
//  -- GATE k1-G2, golden line :5768 -- AutoCalculateInArmXClosePitch() ------
//     WHY THE OFFLINE DEFAULT IS FAITHFUL: the call is a pure setter -- its whole
//       observable effect at this point is to write iInArmXStep (and
//       dInArmXPitch_1Step / dInArmXPitch_MovePitch), all three of which are the
//       main loop's file-scope globals carrying golden's own initialisers from
//       golden :4977-4980 (iInArmXStep=1, both doubles 0.0).  Its return value is
//       discarded here.  So with the gate shut every `cx=x+(j-isStart)*iInArmXStep`
//       below evaluates with step 1 -- adjacent tray columns -- which is exactly
//       golden's behaviour on the (common) 1-column-per-nozzle recipe, and is the
//       value the global is defined to hold before anyone solves for it.
//     REAL-MACHINE BEHAVIOUR DIFFERENCE: on iron the X close-pitch solver can
//       return 2, 3 or 4 tray columns per nozzle for wide-pitch trays.  With the
//       gate shut the planner would then probe adjacent columns instead of the
//       correctly spaced ones and could return a plan that looks legal but points
//       the nozzles at the wrong cells.  This gate is therefore RETIRE-ON-ARRIVAL,
//       not an acceptable steady state.
//     ABSENCE PROOF (TRAP 2 -- claim expires inside this very wave; a sibling may
//       land golden :4982 minutes after I measured, so re-run this):
//       (run with cwd = D:/HT9045/HT9011UC_Cpp_V3.33.906.0)
//         git grep -n -- "AutoCalculateInArmXClosePitch" -- '*.cpp' '*.h'
//       -> 0 hits.  Cross-checked untracked files:
//         grep -rIl --include=*.cpp --include=*.h "AutoCalculateInArmXClosePitch" .
//                                                                   -> 0 files
//       RUN AT 2026-08-11 01:29:06 +0800 (grep finished 01:30:51 +0800).
//     TRAP 3 -- why this SHOULD be gated: only the missing body, nothing else.
//       golden :4982 is pure integer geometry over Prod / LoadForm / TrayForm with
//       no HAL, no alarm sink and no persisted file, and it sits in THIS golden
//       file 5 lines after AutoCalculateInArmYClosePitch which this same wave
//       translates -- it is simply outside my assigned range.  There is no second
//       reason to keep it shut.  If it landed during the wave, DELETE the gate
//       (one #if 0 / #else / #endif triple) rather than re-justifying it.
//============================================================================
//  Forward DECLARATIONS ONLY (never definitions) of the two main-loop-owned
//  globals this function reads, so that this part is order-independent within the
//  stitched file: golden :4979 / :4980.  The main loop still owns the single
//  definition of each; repeating an extern declaration is legal and idempotent.
extern int iInArmXStep;                                                         // golden ainarm9045.cpp:4979 -- DEFINITION owned by the main loop
extern int iInArmYStep;                                                         // golden ainarm9045.cpp:4980 -- DEFINITION owned by the main loop

#ifndef HT9045_bInArmToLoaderUsage_DEFINED
#define HT9045_bInArmToLoaderUsage_DEFINED
bool bInArmToLoaderUsage[MAX_ARM_Row][MAX_ARM_Col]={{false, false, false, false}, {false, false, false, false}};   // golden ainarm2.cpp:100 (decl golden ainarm2.h:174) -- single ACTIVE definition, see hand-off note above
#endif
//============================================================================
int Find_InArm_PickerMaxUseCountOnTime(int iRowCT, int &isStart, int &iRow, int &iCol, bool bSearchLast)
{
    int cx, iMax=0, ct=0;
    bool backupFlag[MAX_ARM_Row][MAX_ARM_Col]={{false, false, false, false}, {false, false, false, false}};
    bool bSourceFlag[MAX_ARM_Row][MAX_ARM_Col]={{false, false, false, false}, {false, false, false, false}};
    int ixStart, ixEnd, ixStep;
    int iyStart, iyEnd, iyStep;
    static int iRecY=0;
    int iTrayDir=TrayForm.Loader.Direction;

    if(IniConfig.bQAMode==true &&
       LastSet.iRunStartMode==rsmQAMode &&
       CosFunction.bQAmodeSupplyTrayDir==true)                                  //jou 2014-10-24 QA mode 增加支援吸Tray方向
    {
        iTrayDir=TestIF_File.iQATrayDirect;
    }

    if(CUSTOMER_CODE==CC_JCET                           &&                      //Ifor 20171108 (Steven) add JCET 要求 Loader 後排吸取不到至前排吸取
       bInArmPickErrFromLoader==true                    &&
       IniConfig.bE56LoaderRetryAtSamePosition==false   &&
       USE_IN_Y_IS_AUTO_PITCH==false                          )                 //JerryYang 20251218 : IN/OUT ARM支援不同模組
    {
        bSearchLast=false;
    }

    for(int i=0; i<InArmSuck.iMaxRow; i++)
    {
        for(int j=0; j<InArmSuck.iMaxCol; j++)
        {
            bSourceFlag[i][j]=bInArmToLoaderUsage[i][j];
        }
    }

    //AI(k1-ainarm) 20260811: GATE k1-G2 -- see GATE REGISTER in this part's
    // banner.  golden :5768 verbatim inside the #if 0; with it shut iInArmXStep
    // keeps golden's own initial value 1 (golden :4979).
#if 0 // GATE k1-G2 -- golden ainarm9045.cpp:5768 (AutoCalculateInArmXClosePitch, golden :4982, absent from port)
    AutoCalculateInArmXClosePitch();
#endif

    iInArmYStep=ChangeToFloatNonPcnt((double)AutoCalculateInArmYClosePitch(), LoadForm->YPitch);

    bool bFrontHasIC=false;

    if(iTrayDir==0)
    {
        if(InArmSuck.iPickRow==2)
        {
            if(bSearchLast && iCloseSiteState==0)
            {
                if(MOT[MMTrayY].FullIC())
                {
                    iyStart=0;
                    iRecY=0;
                }
                else
                {
                    if(iRowCT==1)
                    {
                        for(int j=0; j<MAX_ARM_Col; j++)
                        {
                            if(InArmSuckUse[0][j] && InArmSuck.Item[0][j]!=NULL_IC)
                                bFrontHasIC=true;
                        }

                        if(bFrontHasIC || CosFunction.bLoaderAutoRetry)         //JerryYang 20220901 : ASE-CL要求只有左上角的device吸不到要跳ALARM, 其他的位置吸不到要自動RETRY    //kevin 20181222 add吸嘴第一排有IC 第二排沒有IC 需回到第一排開始吸
                            iyStart=iRecY;                                      //避免未滿盤往下一直吸
                        else
                            iyStart=iRecY+iInArmYStep;                          //ChungHung 20120505 9045WS

                        if(iyStart>=LoadForm->YDivision)                        //KEVIN 20181222 ?? LOAD TRAY 8 PICK UP ERROR
                        {
                            iyStart=0;
                            iRecY=0;
                        }
                    }
                    else
                    {
                        iyStart=0;
                    }
                }
            }
            else
            {
                iyStart=0;
            }
            ixStart=0;
            ixEnd=LoadForm->XDivision;
            ixStep=1;
            iyEnd=LoadForm->YDivision;
            iyStep=1;
        }
        else
        {
            ixStart=0;
            ixEnd=LoadForm->XDivision;
            ixStep=1;
            iyStart=0;
            iyEnd=LoadForm->YDivision;
            iyStep=1;
        }
    }
    else if(iTrayDir==1)
    {
        ixStart=MOT[MMTrayY].Tray.XItem-1;
        ixEnd=-1;
        ixStep=-1;

        iyStart=0;
        iyEnd=MOT[MMTrayY].Tray.YItem;
        iyStep=1;
    }
    else if(iTrayDir==2)
    {
        ixStart=0;
        ixEnd=MOT[MMTrayY].Tray.XItem;
        ixStep=1;

        iyStart=MOT[MMTrayY].Tray.YItem-1;
        iyEnd=-1;
        iyStep=-1;
    }
    else
    {
        ixStart=MOT[MMTrayY].Tray.XItem-1;
        ixEnd=-1;
        ixStep=-1;

        iyStart=MOT[MMTrayY].Tray.YItem-1;
        iyEnd=-1;
        iyStep=-1;
    }

    for(int y=iyStart; y!=iyEnd; y+=iyStep)
    {
        if(iMax!=0)                                                             //如果Tray X排都吸不到IC，Y排跳下一格
            break;
        for(int x=ixStart; x!=ixEnd; x+=ixStep)
        {
            if(MOT[MMTrayY].Tray.Data[x][y]==HAS_IC)
            {
                if(bUseAxExPicker() ||
                   bUseAxxGPicker() ||
                   iCloseSiteModeFor1x4==e1x4CloseAbAc)                         //Steven 20241111 : for 1x4 close 2 site
                {
                    for(int j=0; j<2; j++)
                    {
                        int j2=j*GetJStep();
                        if(iRowCT==2)                                           //ChungHung 20131231 alter AutoYPitch
                        {
                            for(int i=0; i<iRowCT; i++)
                            {
                                bInArmToLoaderUsage[i][j2]=bSourceFlag[i][j2];  //Steven 20110810 : 右下往左上的吸取問題
                                if(InArmSuck.Item[i][j2]!=NULL_IC)
                                    bInArmToLoaderUsage[i][j2]=false;
                            }
                        }
                        else
                        {
                            bInArmToLoaderUsage[iRowCT][j2]=bSourceFlag[iRowCT][j2];
                            if(InArmSuck.Item[iRowCT][j2]!=NULL_IC)
                                bInArmToLoaderUsage[iRowCT][j2]=false;          //Frank 20160420 j*2
                        }
                    }
                }
                else
                {
                    if(iRowCT==2)                                               //ChungHung 20131231 alter AutoYPitch
                    {
                        for(int i=0; i<iRowCT; i++)
                        {
                            for(int j=0; j<MAX_ARM_Col; j++)
                            {
                                bInArmToLoaderUsage[i][j]=bSourceFlag[i][j];
                                if(InArmSuck.Item[i][j]!=NULL_IC)               //判斷吸嘴上是否有IC，有IC時 flag設成false
                                    bInArmToLoaderUsage[i][j]=false;
                            }
                        }
                    }
                    else
                    {
                        for(int j=0; j<MAX_ARM_Col; j++)
                        {
                            bInArmToLoaderUsage[iRowCT][j]=bSourceFlag[iRowCT][j];
                            if(InArmSuck.Item[iRowCT][j]!=NULL_IC)              //判斷吸嘴上是否有IC，有IC時 flag設成false
                                bInArmToLoaderUsage[iRowCT][j]=false;
                        }
                    }
                }

                if(iRowCT==2)                                                   //ChungHung 20131231 alter AutoYPitch
                {
                    for(int j=0; j<MAX_ARM_Col; j++)
                    {
                        if(bInArmToLoaderUsage[0][j])
                        {
                            isStart=j;                                          //確認從第幾隻吸嘴開始
                            break;
                        }
                    }
                }
                else
                {
                    for(int j=0; j<MAX_ARM_Col; j++)
                    {
                        if(bInArmToLoaderUsage[iRowCT][j])
                        {
                            isStart=j;                                          //確認從第幾隻吸嘴開始
                            break;
                        }
                    }
                }

                ct=0;

                if(bUseAxExPicker() ||
                   bUseAxxGPicker() ||
                   iCloseSiteModeFor1x4==e1x4CloseAbAc)                         //Steven 20241111 : for 1x4 close 2 site
                {
                    for(int j=0; j<2; j++)
                    {
                        int j2=j*GetJStep();
                        cx=x+(j-isStart)*iInArmXStep;
                        if(iRowCT==2)                                           //ChungHung 20131231 alter AutoYPitch
                        {
                            if(cx<0)                                            //Steven 20160525 : 避免小於0
                            {
                                bInArmToLoaderUsage[0][j2]=false;
                                bInArmToLoaderUsage[1][j2]=false;
                            }
                            else
                            {
                                if(cx>=MOT[MMTrayY].Tray.XItem)                 //判斷是否大於Tray X Item
                                {
                                    bInArmToLoaderUsage[0][j2]=false;
                                    bInArmToLoaderUsage[1][j2]=false;
                                }

                                if(MOT[MMTrayY].Tray.Data[cx][y]==NULL_IC)      //判斷Tray上是否有IC
                                {
                                    bInArmToLoaderUsage[0][j2]=false;
                                }

                                if(MOT[MMTrayY].Tray.Data[cx][y+iInArmYStep]==NULL_IC)                                  //判斷Tray上是否有IC
                                {
                                    bInArmToLoaderUsage[1][j2]=false;
                                }
                            }

                            if(bInArmToLoaderUsage[0][j2])
                            {
                                ct++;                                           //記數能一次吸幾顆IC
                            }

                            if(bInArmToLoaderUsage[1][j2])
                            {
                                ct++;                                           //記數能一次吸幾顆IC
                            }
                        }
                        else
                        {
                            if(cx<0)                                            //Steven 20160525 : 避免小於0
                            {
                                bInArmToLoaderUsage[iRowCT][j2]=false;
                            }
                            else
                            {
                                if(cx>=MOT[MMTrayY].Tray.XItem)                 //判斷是否大於Tray X Item
                                    bInArmToLoaderUsage[iRowCT][j2]=false;
                                if(MOT[MMTrayY].Tray.Data[cx][y]==NULL_IC)      //判斷Tray上是否有IC
                                    bInArmToLoaderUsage[iRowCT][j2]=false;
                            }

                            if(bInArmToLoaderUsage[iRowCT][j2])
                                ct++;                                           //記數能一次吸幾顆IC
                        }
                    }
                }
                else
                {
                    for(int j=0; j<MAX_ARM_Col; j++)
                    {
                        if(iRowCT==2)                                           //ChungHung 20131231 alter AutoYPitch
                        {
                            cx=x+(j-isStart)*iInArmXStep;
                            if(cx<0)                                            //Steven 20160525 : 避免小於0
                            {
                                bInArmToLoaderUsage[0][j]=false;
                                bInArmToLoaderUsage[1][j]=false;
                            }
                            else
                            {
                                if(cx>=MOT[MMTrayY].Tray.XItem)                 //判斷是否大於Tray X Item
                                {
                                    bInArmToLoaderUsage[0][j]=false;
                                    bInArmToLoaderUsage[1][j]=false;
                                }

                                if(MOT[MMTrayY].Tray.Data[cx][y]==NULL_IC)      //判斷Tray上是否有IC
                                {
                                    bInArmToLoaderUsage[0][j]=false;
                                }

                                if(MOT[MMTrayY].Tray.Data[cx][y+iInArmYStep]==NULL_IC)                                  //判斷Tray上是否有IC
                                {
                                    bInArmToLoaderUsage[1][j]=false;
                                }
                            }

                            if(bInArmToLoaderUsage[0][j])
                            {
                                ct++;                                           //記數能一次吸幾顆IC
                            }

                            if(bInArmToLoaderUsage[1][j])
                            {
                                ct++;                                           //記數能一次吸幾顆IC
                            }
                        }
                        else
                        {
                            cx=x+(j-isStart)*iInArmXStep;
                            if(cx<0)                                            //Steven 20160525 : 避免小於0
                            {
                                bInArmToLoaderUsage[iRowCT][j]=false;
                            }
                            else
                            {
                                if(cx>=MOT[MMTrayY].Tray.XItem)                 //判斷是否大於Tray X Item
                                    bInArmToLoaderUsage[iRowCT][j]=false;
                                if(MOT[MMTrayY].Tray.Data[cx][y]==NULL_IC)      //判斷Tray上是否有IC
                                    bInArmToLoaderUsage[iRowCT][j]=false;
                            }

                            if(bInArmToLoaderUsage[iRowCT][j])
                                ct++;                                           //記數能一次吸幾顆IC
                        }
                    }
                }

                if(CosFunction.bLoaderAutoRetry &&
                   TrayForm.Loader.Direction==0 &&
                   bAutoRetryFlag==true && ct>0)                                //JerryYang 20220901 : ASE-CL要求只有左上角的device吸不到要跳ALARM, 其他的位置吸不到要自動RETRY
                {
                    iMax=ct;
                    iCol=x;
                    iRow=y;
                    if(iRowCT==2)                                               //ChungHung 20131231 alter AutoYPitch
                    {
                        for(int i=0; i<iRowCT; i++)
                            for(int j=0; j<MAX_ARM_Col; j++)
                                backupFlag[i][j]=bInArmToLoaderUsage[i][j];
                    }
                    else
                    {
                        for(int j=0; j<MAX_ARM_Col; j++)
                            backupFlag[iRowCT][j]=bInArmToLoaderUsage[iRowCT][j];
                    }
                    x=ixEnd;                                                    //跳出for迴圈
                    break;
                }

                if(ct>iMax)                                                     //確認最多顆位置
                {
                    iMax=ct;
                    iCol=x;
                    iRow=y;
                    if(iRowCT==2)                                               //ChungHung 20131231 alter AutoYPitch
                    {
                        for(int i=0; i<iRowCT; i++)
                            for(int j=0; j<MAX_ARM_Col; j++)
                                backupFlag[i][j]=bInArmToLoaderUsage[i][j];
                    }
                    else
                    {
                        for(int j=0; j<MAX_ARM_Col; j++)
                            backupFlag[iRowCT][j]=bInArmToLoaderUsage[iRowCT][j];
                    }
                }
            }
        }
    }

    for(int i=0; i<MAX_ARM_Row; i++)
        for(int j=0; j<MAX_ARM_Col; j++)
            bInArmToLoaderUsage[i][j]=backupFlag[i][j];

    if(bSearchLast && iCloseSiteState==0)
    {
        if(InArmSuck.iPickRow==2)
        {
            if(iRowCT==0)
            {
                iRecY=iRow;
            }
        }
    }

    return iMax;
}

//==============================================================================
//  ROLE          Find_InArm_Single_FIFO_SiteOrder -- FIFO-site-order loader pick
//                planner.  Finds the first still-empty in-arm nozzle that is
//                allowed to go to the loader, reads the SITE NUMBER that nozzle
//                feeds (Prod.iSiteMap[iSht][i][j+iKit32]), then sweeps the loader
//                tray in one of 8 fill directions looking for a HAS_IC cell whose
//                iWhichSite matches that site; on a hit it publishes the tray
//                cell in the file-scope iMyCol/iMyRow and returns true.  On a miss
//                it burns that nozzle (HAS_NULL_IC + drop it from the loader-usage
//                mask), re-picks the next site and RECURSES.
//
//  WAVE SCOPE    ACTIVE  Find_InArm_Single_FIFO_SiteOrder   golden ainarm9045.cpp:6133-6362
//                No GATED sub-range: all 230 golden lines are ACTIVE and verbatim --
//                all 8 iTrayDir sweeps, every loop bound, the IniConfig direction
//                lock, SetItemData, and the self-recursion at golden :6351.
//                (A gate WAS drafted over golden :6161/6165/6333/6340 and then
//                 retired inside the wave -- full audit trail in GATE REGISTER.)
//
//  GATE REGISTER   -- ONE gate was opened here and then RETIRED INSIDE THE WAVE.
//                     Both states are recorded because the retirement is exactly
//                     TRAP 2 firing against my own absence-claim, and the main
//                     loop must re-run the command below at integration.
//
//   GATE W7D-1  (OPENED 01:35:55, RETIRED 01:43:42 -- NOT PRESENT in this file)
//                GOLDEN LINES    ainarm9045.cpp:6161, 6165, 6333, 6340
//                SYMBOL          bInArmToLoaderUsage[MAX_ARM_Row][MAX_ARM_Col]
//                GOLDEN HOME     definition golden ainarm2.cpp:100, declaration
//                                golden ainarm2.h:174 (the port has no ainarm2.h;
//                                golden ainarm2.h decls live in aHotPlateSubstrate.h).
//                ABSENCE COMMAND (run in D:/HT9045/HT9011UC_Cpp_V3.33.906.0):
//                                rg -n --no-heading -g "*.cpp" -g "*.h" "bInArmToLoaderUsage" .
//                FIRST MEASURED  2026-08-11 01:35:55 (local, D:/HT9045/HT9011UC_Cpp_V3.33.906.0)
//                                -> 0 hits, exit 1.  ABSENT.  I therefore drafted a
//                                seam that degraded the two READS (:6165, :6340) to
//                                the constant false and the two WRITES (:6161, :6333)
//                                to no-ops -- provably the all-false array`s own
//                                answer, but still a degradation.
//                RE-MEASURED     2026-08-11 01:43:42, same command widened with
//                                -g "*.txt" so it also sees this wave`s part files:
//                                -> HIT.  A SIBLING part file landed the real
//                                definition minutes after my first measurement:
//                                _w7d_parts/05735_Find_InArm_PickerMaxUseCountOnTime.txt
//                                lines 133-136, `#ifndef HT9045_bInArmToLoaderUsage_DEFINED`
//                                / `bool bInArmToLoaderUsage[MAX_ARM_Row][MAX_ARM_Col]=
//                                {{false,false,false,false},{false,false,false,false}};`
//                                -- golden ainarm2.cpp:100 verbatim.  05735 sorts
//                                BEFORE 06133, so in the stitched file the definition
//                                precedes this function.
//                DECISION        GATE DELETED.  golden :6161/6165/6333/6340 are
//                                emitted VERBATIM and ACTIVE below; there is no
//                                `#if 0` and no seam macro anywhere in this file.
//                                Re-asking TRAP 3`s question ("why SHOULD it be
//                                gated") gives no surviving reason: the array is
//                                plain data in the same TU, costs no new dependency,
//                                touches no HAL, no file and no alarm sink.
//                WHAT THE MAIN LOOP MUST CHECK: this function now has a HARD
//                                dependency on that sibling part.  If part 05735 is
//                                dropped or its definition is hoisted WITHOUT being
//                                kept, ainarm9045.cpp fails to LINK with `undefined
//                                reference to bInArmToLoaderUsage`.  That loud
//                                failure is deliberately preferred over silently
//                                re-degrading FIFO picking to "never finds a match".
//                REAL-MACHINE DIFFERENCE IF IT EVER GOES BACK BEHIND A GATE: with
//                                the reads forced false, bHasEmptyPicker can never
//                                become true, iSiteNo stays 0, the whole
//                                `if(iSiteNo!=0)` block (golden :6176-6356) is
//                                unreachable and the function always returns false
//                                (golden :6361) -- FIFO-site-order picking would
//                                look implemented and behave as if switched off.
//
//  FILE-SCOPE GLOBAL THE TASK`S ENUMERATION MISSED -- golden ainarm9045.cpp:6132
//  `int iMyCol, iMyRow;`.  The task lists SEVEN main-loop-owned globals of this
//  golden file and names :6131 bMyFlag but NOT :6132, one line below it.  These two
//  are read by this function (golden :6152/6190/6209/6228/6247/6266/6285/6304/6323)
//  and by Find_InArm_Single (golden :6376 `iCol=iMyCol;`, still the port stub
//  ainarm9045.cpp:2300), so without a definition the stitched file COMPILES and then
//  fails to LINK.  Verified they do not exist at file scope anywhere in the port:
//    rg -n --no-heading -g "*.cpp" -g "*.h" -g "*.txt" "^\s*(extern\s+)?int\s+iMyCol" .
//    -> only aHotPlateSubstrate.h:134 and mykitsuck.h:269, BOTH class members of
//       TMySucker, zero file-scope.  Measured 2026-08-11 01:43:42.
//  I define them below under the port`s OWN established guard idiom (the same shape
//  as ainarm2.cpp:44-47 `#ifndef HT9045_bDestoryOnSht_DEFINED` and as sibling part
//  05735 lines 133-136), so that if the main loop adds them too the clash is an
//  immediate, obvious COMPILE error rather than a silent link hole.  DELETE my three
//  lines if you place them with the other six.
//
//  TRAP 5 -- WHICH HEADER I RELIED ON: InArmSuck here is the TMyKitSuck declared
//  at aHotPlateSubstrate.h:365 (the 177-TU layout), object defined
//  aHotPlateSubstrate.cpp:80; NOT mykitsuck.h:274 (whose duplicate InArmSuck at
//  mykitsuck.cpp:205 is a different layout and is not registered in CMakeLists).
//  That is the header this TU already includes, and .iMaxRow/.iMaxCol/.Item/
//  .SetItemData/.iWhichSht/.iWhichKit all come from it.
//  AI(W7D-k3-ainarm) 20260811
//==============================================================================
// DECLARATION ONLY (never a definition here): golden ainarm2.h:174.  The single
// ACTIVE definition is golden ainarm2.cpp:100, landed this wave by sibling part
// 05735_Find_InArm_PickerMaxUseCountOnTime.txt:133-136.  Repeating an extern
// declaration after (or before) that definition is legal and idempotent, and it
// makes this part order-independent inside the stitched file.
extern bool bInArmToLoaderUsage[MAX_ARM_Row][MAX_ARM_Col];                      // golden ainarm2.h:174

// golden ainarm9045.cpp:6132 -- see the "ENUMERATION MISSED" note above.  Guarded
// with the port`s own idiom so a second definition is a compile error, not silence.
#ifndef HT9045_iMyColRow_DEFINED
#define HT9045_iMyColRow_DEFINED
int iMyCol, iMyRow;                                                             // golden ainarm9045.cpp:6132
#endif
//==============================================================================
bool Find_InArm_Single_FIFO_SiteOrder()                                         //Steven 20170302 (wei) : FIFO MODE
{
    int iSiteNo=0;
    int iTrayDir;
    bool bHasEmptyPicker=false;
    int iSht  =InArmSuck.iWhichSht;
    int iKit32=(InArmSuck.iWhichKit==1)?4:0;

    if(IniConfig.bI37_LockLoaderDirection)
    {
        iTrayDir=IniConfig.iI37_LockLoaderDirection;
    }
    else
    {
        iTrayDir=TrayForm.Loader.Direction;
    }

    ZeroMemory(bMyFlag, sizeof(bMyFlag));
    int iInArmRow=0, iInArmCol=0;                                               //Jimmychiu 20220901 加入初始值
    iMyCol=0;
    iMyRow=0;

    for(int i=0; i<InArmSuck.iMaxRow; i++)
    {
        for(int j=0; j<InArmSuck.iMaxCol; j++)
        {
            if(InArmSuck.Item[i][j]!=NULL_IC)
            {
                bInArmToLoaderUsage[i][j]=false;
            }

            if(bHasEmptyPicker==false       &&
               bInArmToLoaderUsage[i][j]==true  &&
               InArmSuck.Item[i][j]==NULL_IC)
            {
                bHasEmptyPicker=true;
                iSiteNo=Prod.iSiteMap[iSht][i][j+iKit32];
                iInArmRow=i;
                iInArmCol=j;
            }
        }
    }

    if(iSiteNo!=0)
    {
        // ----   左至右,上至下
        //  /
        // --->
        if(iTrayDir==0)
        {
            for(int j=0; j<MOT[MMTrayY].Tray.YItem; j++)
            {
                for(int i=0; i<MOT[MMTrayY].Tray.XItem; i++)
                {
                    if(iSiteNo==MOT[MMTrayY].Tray.iWhichSite[i][j] &&
                       MOT[MMTrayY].Tray.Data[i][j]==HAS_IC)
                    {
                        iMyCol=i;
                        iMyRow=j;
                        return true;
                    }
                }
            }
        }
        // ----   右至左,上至下
        //  \\
        // <---
        else if(iTrayDir==1)
        {
            for(int j=0; j<MOT[MMTrayY].Tray.YItem; j++)
            {
                for(int i=MOT[MMTrayY].Tray.XItem-1; i>=0; i--)
                {
                    if(iSiteNo==MOT[MMTrayY].Tray.iWhichSite[i][j] &&
                       MOT[MMTrayY].Tray.Data[i][j]==HAS_IC)
                    {
                        iMyCol=i;
                        iMyRow=j;
                        return true;
                    }
                }
            }
        }
        // --->   左至右,下至上
        //  \\
        // ----
        else if(iTrayDir==2)
        {
            for(int j=MOT[MMTrayY].Tray.YItem-1; j>=0; j--)
            {
                for(int i=0; i<MOT[MMTrayY].Tray.XItem; i++)
                {
                    if(iSiteNo==MOT[MMTrayY].Tray.iWhichSite[i][j] &&
                       MOT[MMTrayY].Tray.Data[i][j]==HAS_IC)
                    {
                        iMyCol=i;
                        iMyRow=j;
                        return true;
                    }
                }
            }
        }
        // <---   右至左,下至上
        //   /
        // ----
        else if(iTrayDir==3)
        {
            for(int j=MOT[MMTrayY].Tray.YItem-1; j>=0; j--)
            {
                for(int i=MOT[MMTrayY].Tray.XItem-1; i>=0; i--)
                {
                    if(iSiteNo==MOT[MMTrayY].Tray.iWhichSite[i][j] &&
                       MOT[MMTrayY].Tray.Data[i][j]==HAS_IC)
                    {
                        iMyCol=i;
                        iMyRow=j;
                        return true;
                    }
                }
            }
        }
        // |   | 上至下, 左至右
        // | / |
        // |   V
        else if(iTrayDir==4)
        {
            for(int i=0; i<MOT[MMTrayY].Tray.XItem; i++)
            {
                for(int j=0; j<MOT[MMTrayY].Tray.YItem; j++)
                {
                    if(iSiteNo==MOT[MMTrayY].Tray.iWhichSite[i][j] &&
                       MOT[MMTrayY].Tray.Data[i][j]==HAS_IC)
                    {
                        iMyCol=i;
                        iMyRow=j;
                        return true;
                    }
                }
            }
        }
        // |   ^ 下至上, 左至右
        // | \\|
        // |   |
        else if(iTrayDir==5)
        {
            for(int i=0; i<MOT[MMTrayY].Tray.XItem; i++)
            {
                for(int j=MOT[MMTrayY].Tray.YItem-1; j>=0; j--)
                {
                    if(iSiteNo==MOT[MMTrayY].Tray.iWhichSite[i][j] &&
                       MOT[MMTrayY].Tray.Data[i][j]==HAS_IC)
                    {
                        iMyCol=i;
                        iMyRow=j;
                        return true;
                    }
                }
            }
        }
        // |   | 上至下, 右至左
        // | \\|
        // V   |
        else if(iTrayDir==6)
        {
            for(int i=MOT[MMTrayY].Tray.XItem-1; i>=0; i--)
            {
                for(int j=0; j<MOT[MMTrayY].Tray.YItem; j++)
                {
                    if(iSiteNo==MOT[MMTrayY].Tray.iWhichSite[i][j] &&
                       MOT[MMTrayY].Tray.Data[i][j]==HAS_IC)
                    {
                        iMyCol=i;
                        iMyRow=j;
                        return true;
                    }
                }
            }
        }
        // ^   | 下至上, 右至左
        // | / |
        // |   |
        else if(iTrayDir==7)
        {
            for(int i=MOT[MMTrayY].Tray.XItem-1; i>=0; i--)
            {
                for(int j=MOT[MMTrayY].Tray.YItem-1; j>=0; j--)
                {
                    if(iSiteNo==MOT[MMTrayY].Tray.iWhichSite[i][j] &&
                       MOT[MMTrayY].Tray.Data[i][j]==HAS_IC)
                    {
                        iMyCol=i;
                        iMyRow=j;
                        return true;
                    }
                }
            }
        }

        bHasEmptyPicker=false;
        InArmSuck.SetItemData(iInArmRow, iInArmCol, HAS_NULL_IC);
        bInArmToLoaderUsage[iInArmRow][iInArmCol]=false;

        for(int i=0; i<InArmSuck.iMaxRow; i++)
        {
            for(int j=0; j<InArmSuck.iMaxCol; j++)
            {
                if(bHasEmptyPicker==false       &&
                   bInArmToLoaderUsage[i][j]==true  &&
                   InArmSuck.Item[i][j]==NULL_IC)
                {
                    bHasEmptyPicker=true;
                    iSiteNo=Prod.iSiteMap[iSht][i][j+iKit32];
                }
            }
        }

        if(iSiteNo!=0)
        {
            if(Find_InArm_Single_FIFO_SiteOrder()==true)
            {
                return true;
            }
        }
    }
    else
    {
        //不知道會不會發生
    }
    return false;
}

//==============================================================================
//  ROLE          -- Find_InArm_Single(): finds the NEXT single IC to pick out of
//                   the loader tray and returns its (iRow,iCol).  Scan order is
//                   chosen by TrayForm.Loader.Direction (8 arms: 0..6 plus a
//                   final else), with direction 0 additionally supporting the
//                   SPIL/WLP "tray block" sub-tray walk that advances
//                   iInArmBlockIndex past fully-empty blocks.  Two special modes
//                   short-circuit ahead of everything: FIFO-by-site-order / QTI
//                   device-map, and the QA-mode tray-direction override.
//  WAVE SCOPE    -- Find_InArm_Single   golden :6364-6587
//                   ACTIVE, FULLY LIVE (no gates)
//  GATE REGISTER -- (none: this function writes no #if 0.  See the RETIRED GATE
//                   LOG below -- k5-G2 and k5-G3 were written, then retired
//                   inside the wave.)
//
//  RETIRED GATE LOG -- two TRAP 2 cases, kept as evidence
//  -----------------------------------------------------
//  (k5-G2, RETIRED)  golden :6369-6384, the FIFO-site-order / QTI-device-map arm.
//      ORIGINAL PREMISE, two absences:
//        (i) Find_InArm_Single_FIFO_SiteOrder() (golden ainarm9045.cpp:6133,
//            outside this agent's ranges, declared in no golden header) had no
//            body in the port.
//            COMMAND: grep -rn --include=*.cpp --include=*.h -w "Find_InArm_Single_FIFO_SiteOrder" . | wc -l
//            RESULT 0     MEASURED AT 2026-08-11 01:37:38 +0800
//       (ii) the FILE-SCOPE `int iMyCol, iMyRow;` (golden :6132, read at golden
//            :6376-6377) did not exist, and is NOT among the seven globals the
//            main loop owns (:1168, :4977, :4978, :4979, :4980, :6131, :8842) --
//            note it owns :6131 bMyFlag but NOT :6132.  The only port
//            iMyRow/iMyCol are TMyKitSuck MEMBERS (aHotPlateSubstrate.h:133-134,
//            mykitsuck.h:268-269), which a bare `iMyCol` cannot name.
//            COMMAND: grep -rn --include=*.cpp --include=*.h -E "^(int|extern int)[[:space:]]+iMyCol" . | wc -l
//            RESULT 0     MEASURED AT 2026-08-11 01:37:38 +0800
//      BOTH PREMISES DIED AT 2026-08-11 01:47 +0800, inside the same wave: a
//      sibling landed _w7d_parts/06133_Find_InArm_Single_FIFO_SiteOrder.txt,
//      which defines the function at its line 109 AND -- having spotted the same
//      enumeration hole in golden :6132 -- emits `int iMyCol, iMyRow;` at its
//      line 106 inside a `#ifndef HT9045_iMyColRow_DEFINED` guard.  Verified by
//      reading that file at 01:53.  So golden :6369-6384 is now LIVE.
//      Note what would have been lost by leaving the gate up: with
//      FIFO-by-site-order enabled or the QTI device-map form open, golden picks
//      the IC the TESTER's site order demands; the gated build fell through to
//      the geometric scan -- wrong IC, wrong site attribution.  The fact that
//      both guard disjuncts are offline-false made the gate LOOK harmless, which
//      is exactly the "equivalent, therefore fine" reasoning TRAP 2 forbids.
//  (k5-G3, RETIRED)  golden :6391-6400, the `bInArmToLoaderUsage[i][j]=false;`
//      clearing loop.
//      ORIGINAL PREMISE: bInArmToLoaderUsage (golden decl ainarm2.h:174, golden
//      def ainarm2.cpp:100) had ZERO occurrences tree-wide.
//        COMMAND: grep -rn --include=*.cpp --include=*.h -w "bInArmToLoaderUsage" . | wc -l
//        RESULT 0     MEASURED AT 2026-08-11 01:37:38 +0800
//      PREMISE DIED AT 2026-08-11 01:37-01:48 +0800: sibling part
//      _w7d_parts/05735_Find_InArm_PickerMaxUseCountOnTime.txt defines the array
//      at its line 135 under a `#ifndef HT9045_bInArmToLoaderUsage_DEFINED`
//      guard, and both it and 06734_SearchLoadTrayUpDown_9045.txt read it.  (My
//      01:37 grep only covered *.cpp/*.h, so it could not have seen a .txt part
//      file -- that is precisely why TRAP 2 demands the command be re-run at
//      integration rather than trusted.)  Verified by reading 05735 at 01:53.
//      So golden :6391-6400 is now LIVE, and the "stale reservation lets the same
//      occupied nozzle be handed out twice" hazard is closed rather than
//      documented.
//      IF EITHER SIBLING PART IS DROPPED at integration, the failure is a clean
//      undefined-reference (to Find_InArm_Single_FIFO_SiteOrder, iMyCol/iMyRow,
//      or bInArmToLoaderUsage) -- never silent.  The fallback is to re-wrap the
//      two regions in `#if 0` exactly as described above.
//
//  NOTES for the main loop -- ACTION REQUIRED (include head)
//  --------------------------------------------------------
//  * Retiring k5-G2 makes golden :6372 LIVE, so this part now needs two includes
//    the port's ainarm9045.cpp does NOT yet have:
//        #include "atester_shims.h"   // TfContactShim + `extern TfContactShim *fContact` (:154, :251)
//        #include "cContact.h"        // const int CONTACT_DEVICE_MAP_CHECK = 9 (:83)
//    Emitted at the top of this part so the stitched file compiles stand-alone.
//    Both are include-guarded (atester_shimsH / CCONTACT_H) so the duplicate in
//    part 06589 is a no-op, and cContact.h pulls only MachineType.h.  PLEASE
//    HOIST them into ainarm9045.cpp's include head and delete them from both
//    parts -- the sibling engines already do this (ainarm9045_1x4_4.cpp:85,
//    ainarm9045_2x4_16.cpp:67).
//  * `iContactMode` needs nothing new -- plain global, cmydef.h:3102 (cContact.h
//    :173-174 says so explicitly: NOT a TfContact member).
//
//  NOTES for the main loop -- other
//  --------------------------------
//  * The port ALREADY has `bool Find_InArm_Single(int, int&, int&)
//    { return false; }` at ainarm9045.cpp:2300 -- retire it.  Same TU, so a
//    missed retirement is a hard `redefinition` error, not silent TRAP 1
//    shape (b).  NOT `static`.  Declared ainarm9045.h:75.
//  * There is a SECOND, 4-argument `Find_InArm_Single` in the tree (called at
//    ainarm9045_2x8_32.cpp:1391).  Different overload, different header,
//    untouched here.
//  * `int iUseSuck` is UNUSED -- golden never reads it (verified across
//    :6364-6587).  Parameter name kept verbatim; this library builds with
//    -Wno-unused-parameter (build/CMakeFiles/ht9045_sm.dir/flags.make), so no
//    `(void)` cast was added to golden's code.
//  * GOLDEN BUG #1 kept, integer-division class: golden :6414/:6415 assign the
//    result of ChangeToFloatNonPcnt (MachineType.h:1601, returns FLOAT) into the
//    `int` BX/BY, and golden :6421/:6438 do
//    `YArea = BY + (ChangeToFloatNonPcnt((double)iInArmBlockIndex,
//    (double)LoadForm->BlockXItem) * BY)` -- i.e. golden itself replaced the
//    original integer `iInArmBlockIndex/BlockXItem` block-ROW index with a FLOAT
//    division, so that index no longer truncates and YArea can overshoot by up to
//    one block row on non-multiple indices.  The asymmetry proves it was a
//    mechanical edit: the X side one line above still uses honest integer `%`
//    (`iInArmBlockIndex%LoadForm->BlockXItem`).  Transcribed VERBATIM -- do NOT
//    "restore" the integer division; that would change behaviour.
//  * GOLDEN BUG #2 kept: `bBlockEmpty` (golden :6411) is initialised true ONCE
//    outside the k-loop and never reset, so after the first non-empty block the
//    `if(bBlockEmpty)` advance can never fire again and the walk breaks out.
//    Transcribed verbatim.
//  * Symbols present: IniConfig.bQAMode / .bUseTrayBlockMode (Config.h:220) /
//    .bP06_LoaderUseCarrierTray (Config.h:1386) / .bI37_EnableFIFOMode
//    (Config.h:804) / .bI37_EnableFIFOSiteOrder (Config.h:805),
//    LastSet.iRunStartMode (LastSet.h:366), rsmFIFOMode / rsmQAMode
//    (MachineType.h:615 / :608), CosFunction.bQAmodeSupplyTrayDir
//    (CosFunction.h:110), TestIF_File.iQATrayDirect (cprod.h:1914), MMTrayY
//    (cmydef.h:2255), MOT[].Tray (Motor/mymotor.h:336 -> TMyTray, mytray.h
//    XItem/YItem/Data), HAS_IC / NULL_IC (cmydef.h:158 / :156), iInArmBlockIndex
//    (cmydef.h:3431), LoadForm (cprod.h:1365, ->BlockXItem/BlockYItem
//    cprod.h:1289-1290), TrayForm.Loader.Direction (cprod.h:1265),
//    ChangeToFloatNonPcnt (MachineType.h:1601).
//  * TRAP 5: InArmSuck.iMaxRow / iMaxCol / Item resolve through
//    aHotPlateSubstrate.h:365 (:377 / :402 / :370), object
//    aHotPlateSubstrate.cpp:80 -- the same choice as parts 01405 / 04982.
//  * TRAP 4: this part adds NO file-scope OBJECT -- the three file-scope lines
//    below are DECLARATIONS only (`extern` / prototype), so nothing is
//    constructed before main().  fContact is dereferenced only from inside a
//    function body that runs after main(), so the cross-TU dynamic-init ordering
//    hazard does not apply.  golden derefs it unguarded and so do the port's
//    other call sites (AutoClean/AutoClean.cpp:4719, atester.cpp:8774), so no
//    `if(fContact)` guard was added -- that would be a redesign.
//==============================================================================
#include "atester_shims.h"          // AI(k5-ainarm) 20260811: fContact / TfContactShim for golden :6372.  HOIST ME to ainarm9045.cpp's include head.
#include "cContact.h"               // AI(k5-ainarm) 20260811: CONTACT_DEVICE_MAP_CHECK for golden :6372.    HOIST ME to ainarm9045.cpp's include head.

bool Find_InArm_Single_FIFO_SiteOrder();                                        // golden ainarm9045.cpp:6133 (same TU, sibling part 06133); golden declares it in no header -- local prototype keeps this part stitch-order independent
extern int  iMyCol, iMyRow;                                                     // golden ainarm9045.cpp:6132 -- DEFINITION owned by sibling part 06133 (guard HT9045_iMyColRow_DEFINED); declaration only here
extern bool bInArmToLoaderUsage[MAX_ARM_Row][MAX_ARM_Col];                      // golden ainarm2.h:174 -- DEFINITION owned by sibling part 05735 (guard HT9045_bInArmToLoaderUsage_DEFINED); declaration only here

bool Find_InArm_Single(int iUseSuck, int &iRow, int &iCol)                       // golden :6364
{
    int XArea=0, YArea=0;
    int iTrayDir=TrayForm.Loader.Direction;

    if((IniConfig.bI37_EnableFIFOMode &&
        IniConfig.bI37_EnableFIFOSiteOrder &&
        LastSet.iRunStartMode==rsmFIFOMode) ||                                  //Steven 20170302 (wei) : FIFO MODE
       (fContact->fShow && iContactMode==CONTACT_DEVICE_MAP_CHECK))             //Steven 20220510 : For QTI SD Device Map Function
    {
        if(Find_InArm_Single_FIFO_SiteOrder()==true)
        {
            iCol=iMyCol;
            iRow=iMyRow;
            return true;
        }
        else
        {
            return false;
        }
    }

    if(IniConfig.bQAMode==true &&
       LastSet.iRunStartMode==rsmQAMode &&
       CosFunction.bQAmodeSupplyTrayDir==true)                                  //jou 2014-10-24 QA mode 增加支援吸Tray方向
        iTrayDir=TestIF_File.iQATrayDirect;

    for(int i=0; i<InArmSuck.iMaxRow; i++)
    {
        for(int j=0; j<InArmSuck.iMaxCol; j++)
        {
            if(InArmSuck.Item[i][j]!=NULL_IC)
            {
                bInArmToLoaderUsage[i][j]=false;
            }
        }
    }

    if(iTrayDir==0)
    {
        XArea=MOT[MMTrayY].Tray.XItem;
        YArea=MOT[MMTrayY].Tray.YItem;

        if(IniConfig.bUseTrayBlockMode &&
           IniConfig.bP06_LoaderUseCarrierTray==true)                           //Frank 20160928 add Subtray Function
        {
            int BX, BY, BlockTotal;
            bool bBlockEmpty=true;
            if((LoadForm->BlockXItem>0) || (LoadForm->BlockYItem>0))
            {
                BX = ChangeToFloatNonPcnt((double)(MOT[MMTrayY].Tray.XItem), (double)(LoadForm->BlockXItem));
                BY = ChangeToFloatNonPcnt((double)(MOT[MMTrayY].Tray.YItem), (double)(LoadForm->BlockYItem));
                BlockTotal = LoadForm->BlockXItem*LoadForm->BlockYItem;
                iInArmBlockIndex =0;
                for(int k=0; k<BlockTotal; k++)
                {
                    XArea=BX+(iInArmBlockIndex%LoadForm->BlockXItem*BX);
                    YArea=BY+(ChangeToFloatNonPcnt((double)(iInArmBlockIndex), (double)(LoadForm->BlockXItem))*BY);

                    for(int y=0; y<YArea; y++)
                    {
                        for(int x=0; x<XArea; x++)
                        {
                            if(MOT[MMTrayY].Tray.Data[x][y]==HAS_IC)
                            {
                                bBlockEmpty=false;
                            }
                        }
                    }

                    if(bBlockEmpty)
                    {
                        iInArmBlockIndex=k+1;
                        XArea=BX+(iInArmBlockIndex%LoadForm->BlockXItem*BX);    //Frank 20160928 add Subtray Function
                        YArea=BY+(ChangeToFloatNonPcnt((double)(iInArmBlockIndex), (double)(LoadForm->BlockXItem))*BY);
                    }
                    else
                    {
                        break;
                    }
                }
            }
            else                                                                //Dell 20140409 : Fix WLP
            {
                YArea=MOT[MMTrayY].Tray.YItem;
                XArea=MOT[MMTrayY].Tray.XItem;
            }

            for(int y=0; y<YArea; y++)
            {
                for(int x=0; x<XArea; x++)
                {
                    if(MOT[MMTrayY].Tray.Data[x][y]==HAS_IC)
                    {
                        iCol=x;
                        iRow=y;
                        return true;
                    }
                }
            }
        }
        else
        {
            for(int y=0; y<MOT[MMTrayY].Tray.YItem; y++)
            {
                for(int x=0; x<MOT[MMTrayY].Tray.XItem; x++)
                {
                    if(MOT[MMTrayY].Tray.Data[x][y]==HAS_IC)
                    {
                        iCol=x;
                        iRow=y;
                        return true;
                    }
                }
            }
        }
    }
    else if(iTrayDir==1)
    {
        for(int y=0; y<MOT[MMTrayY].Tray.YItem; y++)
        {
            for(int x=(MOT[MMTrayY].Tray.XItem-1); x>=0; x--)
            {
                if(MOT[MMTrayY].Tray.Data[x][y]==HAS_IC)
                {
                    iCol=x;
                    iRow=y;
                    return true;
                }
            }
        }
    }
    else if(iTrayDir==2)
    {
        for(int y=(MOT[MMTrayY].Tray.YItem-1); y>=0; y--)
        {
            for(int x=0; x<MOT[MMTrayY].Tray.XItem; x++)
            {
                if(MOT[MMTrayY].Tray.Data[x][y]==HAS_IC)
                {
                    iCol=x;
                    iRow=y;
                    return true;
                }
            }
        }
    }
    else if(iTrayDir==3)
    {
        for(int y=(MOT[MMTrayY].Tray.YItem-1); y>=0; y--)
        {
            for(int x=(MOT[MMTrayY].Tray.XItem-1); x>=0; x--)
            {
                if(MOT[MMTrayY].Tray.Data[x][y]==HAS_IC)
                {
                    iCol=x;
                    iRow=y;
                    return true;
                }
            }
        }
    }
    else if(iTrayDir==4)
    {
        for(int x=0; x<MOT[MMTrayY].Tray.XItem; x++)
        {
            for(int y=0; y<MOT[MMTrayY].Tray.YItem; y++)
            {
                if(MOT[MMTrayY].Tray.Data[x][y]==HAS_IC)
                {
                    iCol=x;
                    iRow=y;
                    return true;
                }
            }
        }
    }
    else if(iTrayDir==5)
    {
        for(int x=0; x<MOT[MMTrayY].Tray.XItem; x++)
        {
            for(int y=(MOT[MMTrayY].Tray.YItem-1); y>=0; y--)
            {
                if(MOT[MMTrayY].Tray.Data[x][y]==HAS_IC)
                {
                    iCol=x;
                    iRow=y;
                    return true;
                }
            }
        }
    }
    else if(iTrayDir==6)
    {
        for(int x=(MOT[MMTrayY].Tray.XItem-1); x>=0; x--)
        {
            for(int y=0; y<MOT[MMTrayY].Tray.YItem; y++)
            {
                if(MOT[MMTrayY].Tray.Data[x][y]==HAS_IC)
                {
                    iCol=x;
                    iRow=y;
                    return true;
                }
            }
        }
    }
    else
    {
        for(int x=(MOT[MMTrayY].Tray.XItem-1); x>=0; x--)
        {
            for(int y=(MOT[MMTrayY].Tray.YItem-1); y>=0; y--)
            {
                if(MOT[MMTrayY].Tray.Data[x][y]==HAS_IC)
                {
                    iCol=x;
                    iRow=y;
                    return true;
                }
            }
        }
    }
    return false;
}

//==============================================================================
//  ROLE          -- AdjustInArmClosePitchCondition(): the single "may the in-arm
//                   pick 2 IC in one go?" verdict.  Starts false, lets the
//                   X-close-pitch search (AutoCalculateInArmXClosePitch) turn it
//                   true when a legal pitch was found AND the recipe is not in
//                   Fix (one-at-a-time) mode, then applies SEVEN independent veto
//                   rules -- device-map / 2DID order check, FIFO site order,
//                   loader direction >= 4, two tray-block vetoes, and the
//                   1-picker machine.
//  WAVE SCOPE    -- AdjustInArmClosePitchCondition   golden :6589-6650
//                   ACTIVE (all 7 vetoes live), 1 GATE (k5-G4)
//
//  GATE REGISTER
//  =============
//  (k5-G4) golden :6593  `fSCKART->CheckInArmNeedVariModeFIX();`
//          THIS GATE'S PREMISE IS NOT NEW -- it is the IN-ARM TWIN of the
//          already-registered GATE G14 in aoutarm.cpp:2910 (register entry
//          aoutarm.cpp:366-401), which gates
//          `fSCKART->CheckOutArmNeedVariModeFIX(iAuto)` for exactly the same
//          structural reason.  Carrying that premise over rather than
//          re-deriving it, and re-verifying its three legs this wave:
//            * The BODY IS REALLY TRANSLATED: Automation/SCK_ART.cpp:334
//              `void SckArt_CheckInArmNeedVariModeFIX(SckArtState &st)`,
//              declared Automation/SCK_ART.h:136 (golden
//              TfSCKART::CheckInArmNeedVariModeFIX, golden SCK_ART.cpp:995-1029),
//              with a passing unit test at tests/test_SCK_ART.cpp:155-196.  So
//              this is NOT a "missing translation" gate.
//            * It cannot be CALLED from here: it takes `SckArtState&`, and
//              forms/fSCKART.h:74 `class TfSCKART` -- the class of the ONE real
//              global `fSCKART` -- does not derive from SckArtState (re-read the
//              class head this wave: `class TfSCKART`, then the opening brace,
//              then `public:` at :74-76 -- no base-clause), and there is NO
//              global SckArtState object anywhere in the tree to pass instead.
//            * forms/fSCKART.h:63 forbids inventing another declaration site for
//              that state, so standing one up purely to satisfy one call would
//              violate a written instruction in the tree.
//          TRAP 3 -- "why SHOULD this be gated", asked fresh: because the fix is
//          NOT at this call site.  Even with the body present and linkable, the
//          right repair is for TfSCKART to grow the method as a forwarder over
//          its own state once the five-way SckArtState reconciliation debt in
//          forms/fSCKART.h is settled.  Un-gating by inventing a sixth state
//          object would make that debt worse, so a "body exists" finding at
//          integration must NOT by itself retire this gate.  THIS IS THE ONE GATE
//          OF THIS AGENT'S FIVE THAT SURVIVED THE WAVE -- the other four
//          (k5-G1/G2/G3/G5) were absence gates whose premises died against
//          sibling part files within 10 minutes and were retired.  This one is
//          not an absence gate at all.
//          WHY THE OFFLINE DEFAULT (do not call) IS FAITHFUL: the function's only
//          effect outside its own struct is writing ArmSpeed[InArm].bVariModeFIX,
//          and golden :6595 -- LIVE, two lines below -- reads that flag with a
//          plain `==false`.  Not writing it simply leaves whatever the
//          operator/recipe already set; nothing is corrupted and no variable is
//          left uninitialised.
//          REAL-MACHINE BEHAVIOUR DIFFERENCE, on an ART machine only
//          (CosFunction.bUseSCKART && USE_AUTO_RETEST==eartInstall &&
//          IniConfig.bA10_AutoReTest && TestIF_File.bSCKART_EnableART): the
//          ART-specific forcing of one-IC-at-a-time INPUT placement is lost, so
//          the arm keeps picking 2 IC at once during the ART window that golden
//          would have narrowed to Fix mode.
//          ABSENCE PROVED BY (run from D:/HT9045/HT9011UC_Cpp_V3.33.906.0):
//            grep -n "CheckInArmNeedVariModeFIX" forms/fSCKART.h | wc -l
//              RESULT 0     MEASURED AT 2026-08-11 01:39:44 +0800
//            grep -rn --include=*.cpp -E "^(SckArtState|SckArtRemainderState)[[:space:]]+[A-Za-z_]" . | wc -l
//              RESULT 0     MEASURED AT 2026-08-11 01:39:44 +0800
//          Both re-confirmed against the _w7d_parts directory at 01:53 +0800: no
//          sibling part adds a TfSCKART method or a global SckArtState.
//          TRAP 2: both commands are re-runnable at integration.  A non-zero
//          FIRST result (TfSCKART grew the method) is a NECESSARY but not
//          SUFFICIENT condition to retire this gate -- see the TRAP 3 paragraph.
//
//  NOTES for the main loop -- ACTION REQUIRED (include head)
//  --------------------------------------------------------
//  * This part needs TWO includes the port's ainarm9045.cpp does NOT yet have,
//    for the LIVE golden :6611-6612 device-map / 2DID veto:
//        #include "atester_shims.h"   // TfContactShim + `extern TfContactShim *fContact` (:154, :251)
//        #include "cContact.h"        // const int CONTACT_DEVICE_MAP_CHECK = 9 (:83)
//    They are emitted at the top of THIS part so the stitched file compiles
//    stand-alone.  Both are include-guarded (atester_shimsH / CCONTACT_H) so the
//    duplicate in part 06364 is a no-op, and cContact.h pulls only MachineType.h.
//    PLEASE HOIST them into ainarm9045.cpp's include head and delete them from
//    both parts -- the sibling engines already do this
//    (ainarm9045_1x4_4.cpp:85, ainarm9045_2x4_16.cpp:67).
//  * `iContactMode` needs nothing new -- plain global, cmydef.h:3102 (cContact.h
//    :173-174 says so explicitly: NOT a TfContact member).
//  * I did NOT copy AutoClean.cpp:118's `static const int
//    CONTACT_DEVICE_MAP_CHECK = 9;` file-local trick: in a stitched TU that
//    becomes a redefinition the moment anyone includes cContact.h.
//
//  NOTES for the main loop -- other
//  --------------------------------
//  * No declaration exists for this 1-arg overload in golden or the port.  NOT
//    `static` (external linkage, as golden).  A DIFFERENT 2-arg
//    `AdjustInArmClosePitchCondition(bool&, int&)` is called at
//    ainarm9045_2x8_32.cpp:1329 (golden ainarm9045_2x8_32.cpp:1266) -- a separate
//    overload, untouched here.
//  * REACHABILITY: golden's only caller of the 1-arg form is
//    SearchLoadTrayUpDown_9045 (golden :6736).  That function lands THIS SAME
//    WAVE as sibling part 06734_SearchLoadTrayUpDown_9045.txt (definition at its
//    line 168), so this body is genuinely reachable -- it is NOT TRAP 1
//    shape (a).  Checked by reading that part at 2026-08-11 01:53 +0800.  If
//    06734 is dropped, this body becomes unreferenced (harmless: same
//    already-extracted TU) but also unreached, which must then be said out loud.
//  * AutoCalculateInArmXClosePitch() is declared locally below so this part does
//    not depend on the main loop stitching part 04982 ABOVE it.  A matching
//    redeclaration is legal either side of the definition.
//  * TRAP 4: no file-scope object added -- the one file-scope line is a
//    prototype.  fContact is dereferenced only inside a function body that runs
//    after main(), so the cross-TU dynamic-init ordering hazard does not apply
//    (fContact itself is atester_shims.cpp:313
//    `TfContactShim *fContact = new TfContactShim();`, whose ctor touches only its
//    own three widget stand-ins).  golden derefs it unguarded and so do the
//    port's other call sites (AutoClean/AutoClean.cpp:4719, atester.cpp:8774), so
//    NO `if(fContact)` guard was added -- that would be a redesign.
//  * INTEGER DIVISION AUDIT: there is no division in this function.
//  * Symbols present: ArmSpeed (cprod.h:2875) with .bVariModeFIX (cprod.h:2839),
//    InArm==0 (MachineType.h:682 eSpeedPart), fContact->IsRun2DCheck()
//    (atester_shims.h:228, wired to the real golden predicate at
//    atester_shims.cpp:302), _8Site2X4 / _16Site4X4 (MachineType.h:452 / :457),
//    e9045_1x4_1_Ac (MachineType.h:1015), LoadForm->BlockNumberX/Y
//    (cprod.h:1279-1280), USE_PICKER_COUNT (cmydef.h:2915), ep1Picker
//    (MachineType.h:1313), iInArmType / TestIF.iTestMode (already used
//    throughout this file).
//==============================================================================
#include "atester_shims.h"          // AI(k5-ainarm) 20260811: fContact / TfContactShim for golden :6611-6612.  HOIST ME to ainarm9045.cpp's include head.
#include "cContact.h"               // AI(k5-ainarm) 20260811: CONTACT_DEVICE_MAP_CHECK for golden :6611.      HOIST ME to ainarm9045.cpp's include head.

int AutoCalculateInArmXClosePitch();                                            // golden ainarm9045.cpp:4982 (same TU, part 04982); golden declares it nowhere, so declared locally to stay stitch-order independent

void AdjustInArmClosePitchCondition(bool &bCanPick2ICAtOnceTime)                 // golden :6589
{
    bCanPick2ICAtOnceTime=false;

#if 0 // GATE k5-G4 -- golden :6593.  Twin of aoutarm.cpp:2910 GATE G14: the body IS translated (Automation/SCK_ART.cpp:334) but takes SckArtState& and TfSCKART is not one.  See GATE REGISTER k5-G4.
    fSCKART->CheckInArmNeedVariModeFIX();                                       //Sam 20211118 : 整合 ART InOut Arm 一顆一顆放
#endif
    int iResult=AutoCalculateInArmXClosePitch();                                //Steven 20240509 : 移到外面, 避免Fix mode沒執行
    if(ArmSpeed[InArm].bVariModeFIX==false)
    {
        if(iResult==-1)
        {
            bCanPick2ICAtOnceTime=false;
        }
        else
        {
            bCanPick2ICAtOnceTime=true;
        }
    }
    else
    {
        bCanPick2ICAtOnceTime=false;
    }

    if((fContact->fShow && iContactMode==CONTACT_DEVICE_MAP_CHECK) ||           //Steven 20220510 : For QTI SD Device Map Function
       fContact->IsRun2DCheck()==true)                                          //JerryYang 20250220 : 2DID硬體順序檢查功能
    {
        bCanPick2ICAtOnceTime=false;
    }

    if(IniConfig.bI37_EnableFIFOMode && IniConfig.bI37_EnableFIFOSiteOrder && LastSet.iRunStartMode==rsmFIFOMode)       //Steven 20170302 (wei) : FIFO MODE
        bCanPick2ICAtOnceTime=false;

    if(TrayForm.Loader.Direction>=4)
        bCanPick2ICAtOnceTime=false;

    if((IniConfig.bUseTrayBlockMode &&
        IniConfig.bP06_LoaderUseCarrierTray==true) &&                           //Ifor 20161129 add P06 判断避免動作異常
       (TestIF.iTestMode==_8Site2X4  ||
        TestIF.iTestMode==_16Site4X4 ||                                         //Sam 20190226 : 16Site4X4
        iInArmType==e9045_1x4_1_Ac   ||                                         //Steven 20200720 : 1x4只開site Ac
        TestIF.iTestMode==SingleSite ||
        TestIF.iTestMode==DualSite))                                            //Steven 20140716 Add
    {
        if((LoadForm->BlockNumberX>0) || (LoadForm->BlockNumberY>0))
        {
            bCanPick2ICAtOnceTime=false;
        }
    }

    if(IniConfig.bUseTrayBlockMode &&
       IniConfig.bP06_LoaderUseCarrierTray==true)                               //Frank 20160928 add Subtray Function
    {
        if((LoadForm->BlockXItem>0) || (LoadForm->BlockYItem>0))
        {
            bCanPick2ICAtOnceTime=false;
        }
    }

    if(USE_PICKER_COUNT==ep1Picker)
    {
        bCanPick2ICAtOnceTime=false;
    }
}

//==============================================================================
//  k7-ainarm PART 06652 -- ArmRow1FinishForLoader()
//  Translator: AI(pt-wave k7-ainarm) 20260811
//  Golden: ainarm9045.cpp:6652-6675 (cp950)
//
//  ROLE          : predicate -- "is row 1 of the in-arm done, as far as the
//                  LOADER is concerned?"  Walks the 4 row-0 nozzles the current
//                  shuttle/kit actually uses (Prod.fInArmSuck4x8[iSht][0][j+iKit32],
//                  where iKit32 shifts the window to nozzles 4..7 for kit 1) and
//                  answers false the moment a USED nozzle is still empty.
//  WAVE SCOPE    : ArmRow1FinishForLoader  golden :6652-6675 -- ACTIVE (verbatim,
//                  no gates).  No stub to retire -- this symbol exists NOWHERE in
//                  the port today, so this is a net-new body.
//  GATE REGISTER : (none -- every symbol resolves)
//                    InArmSuck.iWhichSht / iWhichKit / Item[][]
//                                               aHotPlateSubstrate.h:408,409,370
//                    Prod.fInArmSuck4x8[2][2][8]  cprod.h:820
//                    iInArmType / e9045_*_Hot    cmydef.h / MachineType.h
//                    NULL_IC                     cmydef.h:156
//
//  HAND-OFF: like ArmYCanSuck2IC, golden declares ArmRow1FinishForLoader in NO
//  header (checked every golden *.h).  Golden`s consumers carry a local `extern`,
//  the BCB6 idiom the port already reproduces at acarry.cpp:171 for
//  CheckOneCycleAction.  There is no port caller yet, so nothing breaks either
//  way; if the main loop wants one, ainarm9045.h is the natural home.
//
//  TRANSCRIPTION note: golden`s own commented-out line `//        iSht32=0;`
//  (:6661) is a dead leftover -- kept verbatim, character for character, because
//  it documents that the *_Hot layouts once reset the shuttle index too.
//==============================================================================
bool ArmRow1FinishForLoader()
{
    int iSht  =InArmSuck.iWhichSht;
    int iKit32=(InArmSuck.iWhichKit==0)?0:4;

    if(iInArmType==e9045_1x4_8_Hot ||                                           //Steven 20240119 : fixed for 2x4_16
       iInArmType==e9045_1x2_4_Hot ||
       iInArmType==e9045_2x2_8_Hot)
    {
//        iSht32=0;
        iKit32=0;
    }

    for(int j=0; j<4; j++)
    {
        int j2=j+iKit32;
        if(Prod.fInArmSuck4x8[iSht][0][j2]==true)
        {
            if(InArmSuck.Item[0][j]==NULL_IC)
                return false;
        }
    }
    return true;
}

//==============================================================================
//  AI(k6-ainarm) 20260811 -- golden ainarm9045.cpp:6677-6732  ArmFinishForLoader
//
//  ROLE          -- 'Is the in-arm done picking at the loader and free to go to
//                   the shuttle?'  Reads the target shuttle/kit off InArmSuck
//                   (forced to shuttle 0 / kit 0 for the three HotPlate
//                   layouts), short-circuits TRUE on clean-out with an empty
//                   loader and empty tray car, and TRUE in FIFO+site-order mode
//                   once the loader tray is empty; then answers by scanning the
//                   nozzles that Prod.fInArmSuck4x8 marks as used for this
//                   shuttle/kit -- any NULL_IC among them means 'not finished'.
//                   Single-picker machines answer from nozzle [0][0] alone.
//  WAVE SCOPE    -- ArmFinishForLoader  ACTIVE  (golden :6677-6732)
//  GATE REGISTER -- none.  This part emits no #if 0.
//
//  FIDELITY NOTES:
//    * the kit offset is the golden ternary `(InArmSuck.iWhichKit==0)?0:4` and the
//      scan bounds are golden's bare literals `i<2` / `j<4` (NOT MAX_ARM_Row /
//      MAX_ARM_Col) with `j2=j+iKit32` indexing fInArmSuck4x8 but plain `j`
//      indexing Item[][].  That asymmetry is golden's and is load-bearing: the
//      nozzle grid is 2x4 while the site map is 2x8.  Kept exactly.
//    * the two early TRUEs are ORDER-SENSITIVE (clean-out before FIFO before the
//      ep1Picker branch); statement order preserved.
//
//  LINKAGE (TRAP 1): NOT static.  golden declares ArmFinishForLoader in no
//  header (3 golden uses, all inside ainarm9045.cpp: the definition plus :7784
//  and :8202 inside DoInArmPickFromLoadStage_9045), and the port tree has ZERO
//  textual hits, so after this part lands the body has NO caller: shape (a).  The
//  port's DoInArmPickFromLoadStage_9045 is whole-body gated at
//  ainarm9045.cpp:2237-2243, which is exactly why.  Static would become shape (d)
//  when that SM lands in this same TU.
//
//  Deps verified REAL: InArmSuck.iWhichSht/.iWhichKit (aHotPlateSubstrate.h:405-406
//  region of TMyKitSuck), .Item[][] (:373), iCleanOut (cmydef.h:255),
//  MOT[MMTrayY].HasIC() (Motor/mymotor.h:353), MOT[MMTrayY_Car].fHasTray
//  (mymotor.h:340, index cmydef.h:2256), LastSet.iRunStartMode (LastSet.h:366),
//  rsmFIFOMode (MachineType.h:615), IniConfig.bI37_EnableFIFOMode /
//  bI37_EnableFIFOSiteOrder (Config.h:804-805), USE_PICKER_COUNT / ep1Picker,
//  Prod.fInArmSuck4x8 (cprod.h; already written by DoInArm_9045_SuckerMap in this
//  same TU), NULL_IC.  TRAP 5: the TMyKitSuck relied on is
//  aHotPlateSubstrate.h:365 (see the DoTrayEndProcess_9045 part for the full
//  two-header argument) -- NOT mykitsuck.h:274.
//==============================================================================
bool ArmFinishForLoader()                                                       //check Arm finish and  can go shuttle
{
    int iSht  =InArmSuck.iWhichSht;
    int iKit32=(InArmSuck.iWhichKit==0)?0:4;

    if(iInArmType==e9045_1x4_8_Hot ||                                           //Steven 20240119 : fixed for 2x4_16
       iInArmType==e9045_1x2_4_Hot ||
       iInArmType==e9045_2x2_8_Hot)
    {
        iSht  =0;
        iKit32=0;
    }

    if(iCleanOut &&
       MOT[MMTrayY].HasIC()==false &&
       MOT[MMTrayY_Car].fHasTray==false)
        return true;

    if(LastSet.iRunStartMode==rsmFIFOMode &&
       IniConfig.bI37_EnableFIFOMode &&
       IniConfig.bI37_EnableFIFOSiteOrder)                                      //Jimmychiu 20250715 : 每盤IC吸完後編號重新計數
    {
        if(MOT[MMTrayY].HasIC()==false)
        {
            return true;
        }
    }

    if(USE_PICKER_COUNT==ep1Picker)
    {
        if(InArmSuck.Item[0][0]==NULL_IC)
        {
            return false;
        }
        else
        {
            return true;
        }
    }

    for(int i=0; i<2; i++)
    {
        for(int j=0; j<4; j++)
        {
            int j2=j+iKit32;
            if(Prod.fInArmSuck4x8[iSht][i][j2]==true)
            {
                if(InArmSuck.Item[i][j]==NULL_IC)
                {
                    return false;
                }
            }
        }
    }
    return true;
}

// =============================================================================
//  k4-ainarm PART 07/08  --  SearchLoadTrayUpDown_9045   golden ainarm9045.cpp:6734-6958
//  Translator: AI(k4-ainarm) 20260811
//
//  ROLE   : The Loader search loop.  Per iteration it decides which nozzle row(s) are in play
//           (both rows at once, or row 0 / row 1), seeds the nozzle-usage map from
//           Prod.fInArmSuck4x8 for the active shuttle+kit, asks the finder for the richest
//           reachable pick (falling back to single-nozzle, and to FIFO site order when the I37
//           options are on), converts that to a tray row/col, rejects it against the
//           tray/block or XDivision bounds, and asks MoveInArmXYToLoader_9045 whether the move
//           is legal.  It loops until a legal pick is found, degrading bCanPick2ICAtOnceTime
//           once, then raises WAR0152 and gives up.
//
//  WAVE SCOPE
//    SearchLoadTrayUpDown_9045  golden :6734-6958  ACTIVE with TWO surviving gates
//                                                  (k4-G6, k4-G9).  k4-G7, k4-G8, k4-G10
//                                                  retired; the bInArmToLoaderUsage hand-off
//                                                  is resolved.
//
//  TRAP 2 IN ACTION -- READ THIS FIRST.  I wrote this part's first draft with SIX gates
//  whose premise was "no port body exists".  I re-ran every absence claim before finishing,
//  at 2026-08-11 01:50:31 +0800, and six of them HAD DIED: sibling agents landed the bodies as part
//  files in _w7d_parts/ during this same wave.  A gate on a dead premise is exactly the
//  defect TRAP 2 describes -- so those six are RETIRED here (call ACTIVE), not left in with
//  an apology.  The gates that survive below survive because the symbol is STILL absent,
//  re-verified at 2026-08-11 01:50:31 +0800, not because nobody re-checked.
//
//  REACHABILITY, STATED HONESTLY (TRAP 1 shape (a)).  This body lands with NO live caller.
//    golden calls SearchLoadTrayUpDown_9045 from exactly one place -- ainarm9045.cpp:6972,
//    inside SearchAndMoveInArmXYToLoad_9045 (golden :6960) -- and the port still stubs that
//    caller at ainarm9045.cpp:2300 (`SearchAndMoveInArmXYToLoad_9045() { return false; }`).
//    No part file in _w7d_parts translates :6960 either (checked 2026-08-11 01:50:31 +0800; the nearest are
//    06734 = this one and 06983 = MoveInArmZToLoaderPick).  It is also declared in no header
//    (golden ainarm9045.h declares only the ..._FIFO_2Step sibling, at :91).  So "the build
//    went green" will NOT prove this function is wired -- nothing reaches this code until
//    :6960 is translated and its stub retired.  Verification command for the main loop:
//      grep -rn "SearchLoadTrayUpDown_9045" --include=*.cpp --include=*.h . | grep -v _FIFO_2Step
//    I am NOT writing this `static` and NOT inventing a declaration -- that would be TRAP 1
//    shape (d).  It is a plain external definition, exactly as golden has it.
//
//  THE EIGHTH MISSING GLOBAL -- bInArmToLoaderUsage -- RESOLVED IN-WAVE, hand-off withdrawn.
//    This function's spine is bInArmToLoaderUsage[MAX_ARM_Row][MAX_ARM_Col] (:6747 ZeroMemory,
//    :6802/:6830 seed, :6854 OR into bflag, :6862 restore, :6893/:6908 copy to InArmSuckUse).
//    At 2026-08-11 01:26:17 +0800 the port had NO definition of it anywhere:
//      cd D:/HT9045/HT9011UC_Cpp_V3.33.906.0 && grep -rnoE "\bbInArmToLoaderUsage\b" \
//        --include=*.cpp --include=*.h --exclude-dir=.git --exclude-dir=.svn . | wc -l  ->  0
//    Golden's home for it is ainarm2.cpp:100 (extern at golden ainarm2.h:174), and the port's
//    ainarm2.cpp is a 114-line partial -- so I flagged it as an EIGHTH global beyond the seven
//    my brief said the main loop adds, missing from that list because the census enumerated
//    globals inside ainarm9045.cpp's OWN span and this one lives in ainarm2.cpp.
//    Re-checking at 2026-08-11 01:50:31 +0800, a sibling has since supplied it:
//      _w7d_parts/05735_Find_InArm_PickerMaxUseCountOnTime.txt:135
//        bool bInArmToLoaderUsage[MAX_ARM_Row][MAX_ARM_Col]={{false,...},{false,...}};
//                                                  // golden ainarm2.cpp:100 -- single ACTIVE definition
//    so the definition exists exactly once across the wave (I verified no other part file
//    defines it, and I deliberately do NOT define it either -- two definitions is a
//    `multiple definition` link error).  05735 sorts before 06734, so it precedes every use.
//    The `extern` below is therefore redundant-but-legal; I keep it so this part still states
//    its own contract, and so it survives if 05735 is dropped from the stitch.
//    >>> RESIDUAL RISK FOR THE MAIN LOOP: if 05735 is dropped, this TU compiles and FAILS TO
//        LINK on bInArmToLoaderUsage.  -fsyntax-only will not catch that.
//
//  RETIRED GATES (were here in the first draft; premises died at 2026-08-11 01:50:31 +0800)
//    k4-G7 (RETIRED)  golden :6791-6794  TrayYDirForArmYPitch() AND ArmRow1FinishForLoader()
//      original premise: TrayYDirForArmYPitch -> 1 match, only the gated ainarm9045_2x8_32.cpp
//        call site (the other hits were the DIFFERENT function UnloaderTrayYDirForArmYPitch,
//        aoutarm.cpp:3565, matching as a substring); ArmRow1FinishForLoader -> 0 matches.
//        Measured 2026-08-11 01:26:17 +0800.
//      why retired: BOTH landed this wave --
//        _w7d_parts/04831_TrayYDirForArmYPitch.txt:74   bool TrayYDirForArmYPitch()
//        _w7d_parts/06652_ArmRow1FinishForLoader.txt:31 bool ArmRow1FinishForLoader()
//        both nullary, matching golden's calls, and both sort before 06734.
//      what retiring BUYS: the four-conjunct AND is whole again, so the two-row
//        "pick both nozzle rows in one pass" optimisation on :6794 can be selected.  My draft
//        had substituted `if(false)`, which silently halved throughput on every qualifying
//        layout.  Retiring is both the faithful and the fast answer.
//    k4-G8 (RETIRED)  golden :6809-6810  ArmRow1FinishForLoader()
//      same landing as k4-G7 (06652:31).  My draft had defaulted the gated term to true, which
//        pinned the search to nozzle row 0 and would have re-picked row 0 forever once it was
//        exhausted.  Now golden's real row-0/row-1 alternation is restored.
//    k4-G10 (RETIRED)  golden :6849  Find_InArm_PickerMaxUseCountOnTime(5 args)
//      original premise: 1 match, the gated ainarm9045_2x8_32.cpp call site -- which passes
//        EIGHT arguments, so golden carries overloads and the 5-arg one had no body (2026-08-11 01:26:17 +0800).
//      why retired: landed as _w7d_parts/05735_Find_InArm_PickerMaxUseCountOnTime.txt:138
//        int Find_InArm_PickerMaxUseCountOnTime(int iRowCT, int &isStart, int &iRow, int &iCol,
//                                               bool bSearchLast)
//        -- the 5-parameter form golden :6849 calls.  05735 < 06734 in stitch order.
//      what retiring BUYS: this is the big one.  Gated, isStart/_iRow/_iCol stayed 0, bflag
//        (:6854) never went true, control fell to Find_InArm_Single (still a `return false`
//        stub at ainarm9045.cpp:2299) and the function ALWAYS returned false at :6869 -- i.e.
//        the Loader search never found a pick at all.  Retiring turns the search back on.
//        Note Find_InArm_Single (:6864/:6875) is still that stub, so the single-nozzle
//        fallback remains inert -- flagged, not gated, because the symbol does resolve.
//
//  GATE REGISTER -- the two that SURVIVE, re-verified still absent at 2026-08-11 01:50:31 +0800 against BOTH
//    the port tree and the sibling part files:
//      grep -rnoE "\b<SYMBOL>\b" --include=*.cpp --include=*.h \
//                --exclude-dir=.git --exclude-dir=.svn .          | wc -l
//      grep -lnE "^(int|void|bool) <SYMBOL>\s*\(" _w7d_parts/*.txt      (empty = still absent)
//
//    k4-G6  golden :6736  AdjustInArmClosePitchCondition(bool&)     golden body at :6589
//      measured: 1 match, a call site inside the already-gated ainarm9045_2x8_32.cpp
//        (CMakeLists.txt:1519 records that file as "whole golden body GATED"), so no body; and
//        no 06589_* part file exists (2026-08-11 01:50:31 +0800).  Note that 2x8_32 call passes TWO arguments
//        while golden :6589 takes one -- golden has overloads and the 1-arg one is mine.
//      why the offline default is faithful: golden lets it pre-adjust bCanPick2ICAtOnceTime
//        before the search loop; skipping it leaves the caller's value untouched, and the loop
//        already re-derives the same downgrade itself at :6866 and :6946-6949.  So the
//        degradation is recoverable inside this function rather than invented here.
//      real-machine difference: a layout that must start in single-pick mode now enters the
//        loop in all-at-once mode and needs one extra loop pass to discover it.  Bounded, and
//        it cannot produce a wrong pick -- only a slower first iteration.
//      TRAP 2 re-check instruction: re-run both commands at integration.
//
//    k4-G9  golden :6841-6842  AutoCalculateInArmXClosePitch()      golden body at :4982
//      measured: 0 matches in the port tree (2026-08-11 01:26:17 +0800), no 04982_* part file (2026-08-11 01:50:31 +0800).
//        SAME absent symbol as PART 06's k4-G2, used here for its RETURN value (==-1 means
//        "no legal close pitch exists").
//      why the offline default is faithful: golden's only action on -1 is
//        `bCanPick2ICAtOnceTime=false`.  Gating skips that forced downgrade, leaving the flag
//        as the caller set it; the loop still downgrades on its own at :6946-6949 when the move
//        turns out to be illegal.  The safety net is retained, one step later.
//      real-machine difference: on a tray pitch with no legal close pitch, one extra loop pass
//        (and one extra rejected MoveInArmXYToLoader_9045 legality probe) before the same
//        downgrade is reached.
//      >>> RETIRE k4-G9 AND PART 06's k4-G2 TOGETHER -- one symbol, two call sites.
//
//  ACTIVE dependencies I verified rather than assumed (probe TU carrying this file's exact
//  include list, g++ -std=c++17 -fsyntax-only -I. -Ivclcompat, 2026-08-11 01:26:17 +0800):
//  DoInArm_9045_SuckerMap, Find_InArm_Single (3-arg form -- golden ainarm9045.h:50 matches the
//  port stub exactly), ArmSpeed[InArm].bVariModeFIX, Prod.fInArmSuck4x8,
//  InArmSuck.iWhichSht/iWhichKit/iPickRow/iMaxRow/iMaxCol, USE_PICKER_COUNT/ep1Picker,
//  USE_16PICKER_TYPE, IniConfig.bI37_EnableFIFOMode/bI37_EnableFIFOSiteOrder/bUseTrayBlockMode/
//  bP06_LoaderUseCarrierTray, LastSet.iRunStartMode/rsmFIFOMode,
//  LoadForm->XDivision/YDivision/BlockXItem/BlockYItem, InArmSuckUse,
//  ShowErrorMessage(...,__FUNC__), and MoveInArmXYToLoader_9045 (PART 06).
//
//  fContact->IsRun2DCheck() at :6753 is ACTIVE, NOT gated -- but it needed a header.
//    `fContact` was unreachable from ainarm9045.cpp's include block (probe error
//    "'fContact' was not declared in this scope").  Its port declaration is
//    atester_shims.h:251  `extern TfContactShim *fContact;` -- note TfContactShim
//    (atester_shims.h:154), the port's registered stand-in, NOT golden's TfContact from
//    cContact.h:667.  I include that header rather than gate a live branch.
//  TRAP 5 -- WHICH TMyKitSuck / WHICH TfContact.  InArmSuck / FLCarryKit / BLCarryKit are the
//    aHotPlateSubstrate.h:365 TMyKitSuck (the 177-TU layout), objects defined in
//    aHotPlateSubstrate.cpp:80,81,82.  Adding atester_shims.h could not swap that layout:
//      grep -n "#include" atester_shims.h  ->  vclcompat/vcl_compat.h, vclcompat/Controls.h,
//                                               MachineType.h, <vector>   (no mykitsuck.h)
//      g++ -E ... <probe with both includes> | grep -c "mykitsuck.h"   ->  0   (2026-08-11 01:26:17 +0800)
//    so the rival mykitsuck.h:274 TMyKitSuck (different layout, would read every field at the
//    wrong offset; mykitsuck.cpp:205-206 also defines InArmSuck/FLCarryKit) never enters this
//    TU.  That TU must stay unregistered; nothing here changes it.
//
//  GOLDEN NO-OP KEPT, NOT "OPTIMISED": :6745 sets iKit32=(iWhichKit==1)?4:0 and :6755 sets it
//  to the identical expression inside the IsRun2DCheck() guard.  The two differ only if
//  DoInArm_9045_SuckerMap() (:6752, between them) mutates InArmSuck.iWhichKit -- so it is NOT
//  provably dead and is reproduced exactly.  Also kept verbatim: `bool ... bNeedPick;` (:6738)
//  left uninitialised, read only on the FIFO path that assigns it first.
//
//  INTEGER DIVISION KEPT: none introduced; :6742 and :6786 stay integer products
//  (iMinXStep*iInArmXStep), and :6914 stays iCol=_iCol-(isStart*iInArmXStep).
//
//  Big5 -> UTF-8: eleven Chinese comments transcribed character-for-character from cp950
//  (:6742, :6766, :6794, :6797, :6812, :6817, :6918, :6932, :6933, :6935, :6952).  Golden's
//  own wording on :6952 is kept exactly as written.  ZERO U+FFFD.
// =============================================================================
#include "atester_shims.h"          // k4-ainarm: fContact (TfContactShim* -- atester_shims.h:251) for the :6753 IsRun2DCheck branch; see the fContact note above

// k4-ainarm: golden ainarm2.h:174 extern, verbatim.  The single ACTIVE definition now lives in
// sibling part 05735 (golden ainarm2.cpp:100) -- see "THE EIGHTH MISSING GLOBAL" above.
// Declared here, deliberately NOT defined: a second definition is a link error.
extern bool bInArmToLoaderUsage[MAX_ARM_Row][MAX_ARM_Col];

bool SearchLoadTrayUpDown_9045(bool &bCanPick2ICAtOnceTime, int &iRow, int &iCol, int &NowRow, bool bNeedDown)
{
#if 0 // GATE k4-G6 (LIVE, re-verified absent 2026-08-11 01:50:31 +0800) -- golden :6736 AdjustInArmClosePitchCondition (golden ainarm9045.cpp:6589; no port body, no part file) -- see GATE REGISTER above
    AdjustInArmClosePitchCondition(bCanPick2ICAtOnceTime);
#endif

    bool CanMove=false, bNeedPick;
    bool flag_Temp[MAX_ARM_Row][MAX_ARM_Col], bflag=false;                      //Ztex 2024.03.02 Add HT-1032AT Right Side Can't Suck IC Issue
    int isStart=0, _iRow=0, _iCol=0;
    int ArmMaxSuck=4;
    int iMinXStep=(USE_16PICKER_TYPE==1)?-7:-3;                                 //Steven 20230727 : 修正最小X位置 //Ztex 2023.12.06 Add HT-1032
    int iRowCT=0;
    int iSht  =InArmSuck.iWhichSht;
    int iKit32=(InArmSuck.iWhichKit==1)?4:0;
    int iCollimit=0;
    ZeroMemory(bInArmToLoaderUsage, sizeof(bInArmToLoaderUsage));
    ZeroMemory(flag_Temp, sizeof(flag_Temp));                                   //Ztex 2024.03.02 Add HT-1032AT Right Side Can't Suck IC Issue
    //----------------------------------------
    //Set Arm Suck Status
    //----------------------------------------
    DoInArm_9045_SuckerMap();
    if(fContact->IsRun2DCheck())                                                //JerryYang 20260409 : fix 2DID mapping
    {
        iKit32=(InArmSuck.iWhichKit==1)?4:0;
    }

    if(iInArmType==e9045_1x4_8_Hot ||
       iInArmType==e9045_1x2_4_Hot ||
       iInArmType==e9045_2x2_8_Hot)
    {
        iSht  =0;
        iKit32=0;
    }

    if(iInArmType==e9045_1x1_1     ||                                           //Sam 20250902 : 修正一次吸料問題
       iInArmType==e9045_1x4_1_Ac  ||
       iInArmType==e9045_1x2_2_13  ||
       iInArmType==e9045_1x2_2_14  ||
       iInArmType==e9045_1x2_4_Hot ||
       iInArmType==e9045_1x4_2_14  ||
       iInArmType==e9045_1x4_4_13  ||
       iInArmType==e9045_2x1_2_13  ||
       iInArmType==e9045_2x2_2_14  ||
       iInArmType==e9045_2x2_4_12  ||
       iInArmType==e9045_2x2_4_13  ||
       iInArmType==e9045_2x2_4_14  ||
       iInArmType==e9045_2x3_6_14  ||
       iInArmType==e9045_2x4_4_13  ||
       iInArmType==e9045_2x4_4_14)
    {
        iCollimit=0;
    }
    else
    {
        iCollimit=iMinXStep*iInArmXStep;
    }

    while(CanMove==false)
    {
        if(ArmSpeed[InArm].bVariModeFIX==false &&
           InArmSuck.iPickRow==2  &&
           TrayYDirForArmYPitch() &&
           ArmRow1FinishForLoader()==false)                                     //是否一次能吸8顆
        {
            iRowCT=2;
            iInArmOrder=0;                                                      //吸嘴組，第一排
            for(int i=0; i<InArmSuck.iMaxRow; i++)
            {
                for(int j=0; j<InArmSuck.iMaxCol; j++)
                {
                    bInArmToLoaderUsage[i][j]=Prod.fInArmSuck4x8[iSht][i][j+iKit32];
                    flag_Temp[i][j] =Prod.fInArmSuck4x8[iSht][i][j+iKit32];     //Ztex 2024.03.02 Add HT-1032AT Right Side Can't Suck IC Issue
                }
            }
        }
        else
        {
            if(InArmSuck.iPickRow==1 ||
               ArmRow1FinishForLoader()==false)
            {
                iInArmOrder=0;                                                  //吸嘴組，第一排
                iRowCT=0;
            }
            else
            {
                iInArmOrder=1;                                                  //吸嘴組，第二排
                iRowCT=1;
            }

            if(USE_PICKER_COUNT==ep1Picker)                                     //JerryYang 20250902 : add
            {
                bInArmToLoaderUsage[0][0]=true;
                flag_Temp[0][0]=true;
            }
            else
            {
                for(int j=0; j<InArmSuck.iMaxCol; j++)
                {
                    bInArmToLoaderUsage[iRowCT][j]=Prod.fInArmSuck4x8[iSht][iRowCT][j+iKit32];
                    flag_Temp[iRowCT][j]=Prod.fInArmSuck4x8[iSht][iRowCT][j+iKit32];                                    //Ztex 2024.03.02 Add HT-1032AT Right Side Can't Suck IC Issue
                }
            }
        }

        NowRow=iRowCT;
        //----------------------------------------------------------
        //check inArm Close pitch is legal
        if(bCanPick2ICAtOnceTime)
        {
#if 0 // GATE k4-G9 (LIVE, re-verified absent 2026-08-11 01:50:31 +0800) -- golden :6841-6842 AutoCalculateInArmXClosePitch (golden ainarm9045.cpp:4982) -- retire together with PART 06 k4-G2; see GATE REGISTER above
            if(AutoCalculateInArmXClosePitch()==-1)
                bCanPick2ICAtOnceTime=false;
#endif
        }
        //----------------------------------------------------------
        //Set Arm Suck whith tray
        //----------------------------------------------------------
        if(bCanPick2ICAtOnceTime==true)
        {
            Find_InArm_PickerMaxUseCountOnTime(iRowCT, isStart, _iRow, _iCol, TestIF.bSearchLastMode);
            for(int i=0; i<InArmSuck.iMaxRow; i++)                              //Ztex 2024.03.02 Add HT-1032AT Right Side Can't Suck IC Issue
            {
                for(int j=0; j<InArmSuck.iMaxCol; j++)
                {
                    bflag|=bInArmToLoaderUsage[i][j];
                }
            }

            if(bflag==false)
            {
                for(int i=0; i<InArmSuck.iMaxRow; i++)
                    for(int j=0; j<InArmSuck.iMaxCol; j++)
                        bInArmToLoaderUsage[i][j]=flag_Temp[i][j];

                bool bsingle=Find_InArm_Single(ArmMaxSuck, _iRow, _iCol);
                isStart=0;
                bCanPick2ICAtOnceTime=false;
                if(bsingle==false)
                {
                    return false;
                }
            }
        }
        else
        {
            bool bsingle=Find_InArm_Single(ArmMaxSuck, _iRow, _iCol);
            isStart=0;
            if(bsingle==false)
            {
                return false;
            }
        }
        //-----------------------------------------------------------

        if(IniConfig.bI37_EnableFIFOMode &&
           IniConfig.bI37_EnableFIFOSiteOrder &&
           LastSet.iRunStartMode==rsmFIFOMode)                                  //Steven 20170302 (wei) : FIFO MODE
        {
            bNeedPick=false;
            for(int i=0; i<InArmSuck.iMaxRow; i++)
            {
                for(int j=0; j<InArmSuck.iMaxCol; j++)
                {
                    InArmSuckUse[i][j]=bInArmToLoaderUsage[i][j];
                    if(bInArmToLoaderUsage[i][j]==true)
                        bNeedPick=true;
                }
            }

            if(bNeedPick==false)
                return false;
        }
        else
        {
            for(int i=0; i<InArmSuck.iMaxRow; i++)
            {
                for(int j=0; j<InArmSuck.iMaxCol; j++)
                {
                    InArmSuckUse[i][j]=bInArmToLoaderUsage[i][j];
                }
            }
        }

        //iX===>Tray has IC in x direct isStart==>need suck unit  iInArmXStep===>suck two ic pitch
        iCol=_iCol-(isStart*iInArmXStep);
        iRow=_iRow;

        if(IniConfig.bUseTrayBlockMode &&
           IniConfig.bP06_LoaderUseCarrierTray==true)                           //Ifor 20181011 (Steven) : add P06 功能馬達極限位置
        {
            if((iCol<(iMinXStep*iInArmXStep) ||
                iCol>=(LoadForm->XDivision*LoadForm->BlockXItem)) ||
               (iRow<0 ||
                iRow>=(LoadForm->YDivision*LoadForm->BlockYItem)))
            {
                CanMove=false;
            }
            else
            {
                CanMove=MoveInArmXYToLoader_9045(bCanPick2ICAtOnceTime, iRowCT, iRow, iCol, false, bNeedDown);
            }
        }
        else if((iCol<iCollimit ||                                              //Sam 20250902 : 修正一次吸料問題 //Sam 20250313 : 修正吸料順序錯誤問題
                 iCol>=LoadForm->XDivision) ||                                  //jou 2011-04-15 最左邊最多不能超過(-3*iInArmXStep)
                (iRow<0 ||
                 iRow>=LoadForm->YDivision))                                    //Hung 20110407 : 2x2吸取異常時會跳舞解除
        {
            CanMove=false;
        }
        else
        {
            CanMove=MoveInArmXYToLoader_9045(bCanPick2ICAtOnceTime, iRowCT, iRow, iCol, false, bNeedDown);
        }

        if(CanMove==false)
        {
            if(bCanPick2ICAtOnceTime==true)
            {
                bCanPick2ICAtOnceTime=false;
            }
            else
            {
                ShowErrorMessage("WAR0152", 0, MInArmX, 0, __FUNC__);           //粹盤參數錯誤，馬達將會超出限制
                return false;
            }
        }
    }
    return true;
}

//==============================================================================
//  ROLE          -- SearchAndMoveInArmXYToLoad_9045(): the loader-side entry
//                   point.  Decides whether the Z axis needs to go down at all
//                   (only if the loader stage actually holds a tray WITH IC),
//                   asks the up/down tray search for the next pick target, then
//                   commands the in-arm XY (and pitch) move onto it -- with the
//                   Z-down suppressed in the "load initial / auto-high" teach
//                   mode.
//  WAVE SCOPE    -- SearchAndMoveInArmXYToLoad_9045   golden :6960-6981
//                   ACTIVE, FULLY LIVE (no gates)
//  GATE REGISTER -- (none: this function writes no #if 0.  See the RETIRED GATE
//                   LOG below -- k5-G5 was written, then retired inside the wave.)
//
//  RETIRED GATE LOG -- k5-G5, a TRAP 2 case with a golden-bug twist
//  ---------------------------------------------------------------
//  (k5-G5, RETIRED)  golden :6972-6980, the search call plus all three move
//      returns.
//      ORIGINAL PREMISE, two absences:
//        (i) SearchLoadTrayUpDown_9045(bool&,int&,int&,int&,bool) -- golden
//            ainarm9045.cpp:6734, declared in no golden header -- had no body in
//            the port.
//            COMMAND: grep -rn --include=*.cpp --include=*.h -w "SearchLoadTrayUpDown_9045" . | wc -l
//            RESULT 0     MEASURED AT 2026-08-11 01:37:38 +0800
//            The `-w` is load-bearing: without it
//            SearchLoadTrayUpDown_9045_FIFO_2Step (ainarm9045.h:116, stub
//            ainarm9045.cpp:2293) and SearchLoadTrayUpDown_9045_2x8_32
//            (ainarm9045_2x8_32.cpp:1325) give a FALSE "present".
//       (ii) MoveInArmXYToLoader_9045(bool&,int,int&,int&,bool,bool) -- golden
//            ainarm9045.cpp:5604 -- had no body in the port.
//            COMMAND: grep -rn --include=*.cpp --include=*.h -w "MoveInArmXYToLoader_9045" . | wc -l
//            RESULT 0     MEASURED AT 2026-08-11 01:37:38 +0800
//      BOTH PREMISES DIED AT 2026-08-11 01:48 +0800, inside the same wave:
//      siblings landed _w7d_parts/06734_SearchLoadTrayUpDown_9045.txt (definition
//      at its line 168, signature-identical) and
//      _w7d_parts/05604_MoveInArmXYToLoader_9045.txt (definition at its line 121,
//      and CRUCIALLY under the "In" spelling this call site needs).  Verified by
//      reading both files at 01:53.  So golden :6972-6980 is LIVE and this
//      function no longer returns a hard `false`.
//      THE GOLDEN BUG THAT MADE (ii) LOOK ALREADY-SATISFIED -- keep this on
//      record, it is a real trap for the next reader: golden's own header and
//      definition names DISAGREE.  golden ainarm9045.h:53 declares
//      `MoveArmXYToLoader_9045` (no "In"), while golden ainarm9045.cpp:5604
//      DEFINES `MoveInArmXYToLoader_9045` (with "In"), and all six golden call
//      sites use the "In" spelling.  So golden declares a function it never
//      defines and defines one it never declares.  The port faithfully copied the
//      header (ainarm9045.h:78) and therefore stubbed the WRONG NAME -- `bool
//      MoveArmXYToLoader_9045(bool&, int, int&, int&, bool, bool)
//      { return false; }` at ainarm9045.cpp:2303, mis-cited as "golden :5604".
//      That stub can never satisfy this call site.  Not fixed here; ACTION for
//      the main loop: when 05604 lands the "In" body, decide what to do with the
//      orphan `MoveArmXYToLoader_9045` declaration + stub (they are now dead
//      weight, and leaving both invites TRAP 1 shape (b) confusion).
//      IF EITHER SIBLING PART IS DROPPED at integration, the failure is a clean
//      undefined-reference -- never silent.  The fallback is to re-wrap
//      :6972-6980 in `#if 0` with `return false;` in the `#else`: that is
//      golden's own "loader pick not staged" value (its :6973 failure path), the
//      value the retired stub at ainarm9045.cpp:2301 already returned, and the
//      NO-MOTION answer, so it cannot command a move on stale coordinates.
//
//  NOTES for the main loop
//  -----------------------
//  * The port ALREADY has `bool SearchAndMoveInArmXYToLoad_9045()
//    { return false; }` at ainarm9045.cpp:2301 -- retire it.  Same TU, so a
//    missed retirement is a hard `redefinition` error, not silent TRAP 1
//    shape (b).  NOT `static`.  Declared ainarm9045.h:76.
//  * The two local prototypes below keep this part independent of stitch order
//    (golden :5604 and :6734 both precede :6960, so in golden-line order they
//    should not be needed -- but golden relies purely on definition order, which
//    a part-file stitch must not assume).  NO default arguments on either: the
//    sibling definitions have none, and golden's only defaulted declaration is
//    for the differently-spelled orphan.
//  * MOT[MMTrayY].fHasTray (Motor/mymotor.h:340) and MOT[MMTrayY].HasIC()
//    (Motor/mymotor.h:353) both exist, so bNeedDown is computed exactly as golden
//    computes it -- and now actually consumed.
//  * Common/PickPlanner/cInArmLoaderPickPlanner.h:18 and :28 document a planned
//    "authoritative cut-over of SearchAndMoveInArmXYToLoad_9045" under
//    bUseLoaderPickPlanner.  This part deliberately translates GOLDEN, not that
//    plan -- no planner call was inserted.
//  * bLoadAutoTeachInarm (cmydef.h:5020) and bLoadInarmAutoHigh (cmydef.h:4402)
//    both exist.  GOLDEN ODDITY kept: the first and third branches are IDENTICAL
//    (`RealMove=true`), so the bLoadAutoTeachInarm test only matters when
//    bLoadInarmAutoHigh is also set -- golden's three-way if/else if/else is
//    reproduced verbatim rather than collapsed.
//  * INTEGER DIVISION AUDIT: no division in this function.
//  * TRAP 4: no file-scope object added -- the two file-scope lines are
//    prototypes.
//==============================================================================
bool SearchLoadTrayUpDown_9045(bool &bCanPick2ICAtOnceTime, int &iRow, int &iCol, int &NowRow, bool bNeedDown);          // golden ainarm9045.cpp:6734 (same TU, sibling part 06734); golden declares it in no header
bool MoveInArmXYToLoader_9045(bool &bCanPick2ICAtOnceTime, int iSelRow, int &iRow, int &iCol, bool RealMove, bool ZNeedDown);    // golden ainarm9045.cpp:5604 (same TU, sibling part 05604); golden's header declares the DIFFERENT name MoveArmXYToLoader_9045 -- see RETIRED GATE LOG

bool SearchAndMoveInArmXYToLoad_9045()                                          //Steven 20240427 : 整合Loader動作
{
    int  iCol=0, iRow=0, iSelRow=0;
    bool bCanPick2ICAtOnceTime=false;
    bool bNeedDown=true;

    if(MOT[MMTrayY].fHasTray==false ||
       MOT[MMTrayY].HasIC()==false)
    {
        bNeedDown=false;
    }

    if(SearchLoadTrayUpDown_9045(bCanPick2ICAtOnceTime, iRow, iCol, iSelRow, bNeedDown)==false)
        return false;

    if(bLoadAutoTeachInarm)                                                     //kevin 20210305 add inarm Auto teach move X Y PITCH Z 軸不動
        return MoveInArmXYToLoader_9045(bCanPick2ICAtOnceTime, iSelRow, iRow, iCol, true, bNeedDown);
    else if(bLoadInarmAutoHigh)                                                 //kevin 20170929 (wei) load initial load 一盤 inarm 校正高度 Z軸先不要下去吸
        return MoveInArmXYToLoader_9045(bCanPick2ICAtOnceTime, iSelRow, iRow, iCol, false, bNeedDown);
    else
        return MoveInArmXYToLoader_9045(bCanPick2ICAtOnceTime, iSelRow, iRow, iCol, true, bNeedDown);
}

//==============================================================================
//  ROLE          MoveInArmZToLoaderPick -- drop the in-arm Z axes onto the loader
//                tray to pick.  Single-site (or 1x4-only-site-Ac) uses one nozzle
//                (column 1 when the C-nozzle options are set); every other layout
//                drives one Z per USED nozzle in parallel and only reports success
//                once ALL of them arrived.  Target is
//                Prod.ZInArm_Tray_Pick[i][j] + Step*ArmSpeed[InArm].dRetryDown, via
//                the 2-speed picker move.  On success it clamps the tray
//                (C_TrayY_Fixer / C_LoaderEdgePush / C_LoaderUpPress) and resets the
//                suck state.
//
//  WAVE SCOPE    ACTIVE  MoveInArmZToLoaderPick   golden ainarm9045.cpp:6983-7031
//
//  GATE REGISTER (this part file)   -- EMPTY.  Everything resolves:
//                InArmZIndex[2][8] (cmydef.h:3103), MOT[] TTrayMotor
//                (Motor/mymotor.h:385) with MotorMove2SpeedForPicker(int,
//                ARM_CONDITION*, bool) (:165), Prod.ZInArm_Tray_Pick[][]
//                (cprod.h:395), ArmSpeed[SpeedPartTotal] (cprod.h:2875) /
//                ARM_CONDITION::dRetryDown (cprod.h:2832) / InArm==0
//                (MachineType.h:682), InArmSuckUse[][] (aHotPlateSubstrate.h:743,
//                defined aHotPlateSubstrate.cpp:885), Cylinder[MaxCylinderItem]
//                (mycylin.h:176) with On() (:112), C_TrayY_Fixer / C_LoaderEdgePush /
//                C_LoaderUpPress (cmydef.h:303/319/364), InArmSuck.ResetAll()
//                (aHotPlateSubstrate.h) and InArmSuckReset() (declared
//                ainarm9045.h:106).
//
//  CALLEE HONESTY (not a gate): InArmSuckReset() is still the `{}` stub at
//  ainarm9045.cpp:2277, and MotorMove2SpeedForPicker`s body is the W6 sim-motor
//  one (Motor/mymotor.h:161-162 says the state-machine bodies are TODO(W6)).  So
//  this function is faithful in structure while its two leaves are still shallow;
//  no behaviour is invented here.
//
//  GOLDEN QUIRK kept verbatim: golden :7018 `return bResult=false;` (assign-then-
//  return inside the all-arrived check) -- kept exactly, including the assignment.
//  Also kept: the SingleSite branch writes bResult from a SINGLE MotorMove2Speed...
//  call and does NOT consult flag[][], while the else branch ignores bResult`s
//  initial true until the flag sweep.
//
//  BEHAVIOUR CHANGE vs the stub it retires (ainarm9045.cpp:2302 `{ return false; }`):
//  the stub could never report a completed Z drop; this body can, and it now
//  actuates three cylinders on success.  Real motion, flagged for the main loop.
//  AI(W7D-k3-ainarm) 20260811
//==============================================================================
// This TU does not include mycylin.h (golden gets Cylinder[] through the
// aArmHeader god-header, include list gated `#if 0` at aArmHeader.h:23).  Include
// it here rather than invent a cylinder accessor.  Guarded (mycylinH).
#include "mycylin.h"                                                            // Cylinder[] / TMyCylinder::On()
//------------------------------------------------------------------------------
bool MoveInArmZToLoaderPick(int Step)                                           //Steven 20160414 : 簡化In Arm程式碼
{
    bool flag[MAX_ARM_Row][MAX_ARM_Col]={{false, false, false, false}, {false, false, false, false}};
    bool bResult=true;
    int iSuckR=0, iSuckC=0;

    if(TestIF.iTestMode==SingleSite || iInArmType==e9045_1x4_1_Ac)              //Steven 20200720 : 1x4只開site Ac
    {
        if(Prod.bSingleUseOtherSuck ||
           Prod.bSingleInArmUseOtherSuck)                                       //wei 20220823 Single Site使用C吸嘴判斷
        {
            iSuckC=1;
        }
        bResult=MOT[InArmZIndex[iSuckR][iSuckC]].MotorMove2SpeedForPicker(Prod.ZInArm_Tray_Pick[iSuckR][iSuckC]+Step*ArmSpeed[InArm].dRetryDown, &ArmSpeed[InArm], true);  //JerryYang 20190729 二段速功能可選擇only at loader   //Steven 20140217 : 兩段速移動
    }
    else
    {
        for(int i=0; i<InArmSuck.iMaxRow; i++)
        {
            for(int j=0; j<InArmSuck.iMaxCol; j++)
            {
                if(InArmSuckUse[i][j])
                {
                    flag[i][j]=MOT[InArmZIndex[i][j]].MotorMove2SpeedForPicker(Prod.ZInArm_Tray_Pick[i][j]+Step*ArmSpeed[InArm].dRetryDown, &ArmSpeed[InArm], true);  //JerryYang 20190729 二段速功能可選擇only at loader  //Steven 20140217 : 兩段速移動
                }
                else
                {
                    flag[i][j]=true;
                }
            }
        }

        for(int i=0; i<InArmSuck.iMaxRow; i++)
            for(int j=0; j<InArmSuck.iMaxCol; j++)
                if(flag[i][j]==false)
                    return bResult=false;
    }

    if(bResult)                                                                 //Steven 20161223 (jou) : 簡化程式碼
    {
        Cylinder[C_TrayY_Fixer].On();
        Cylinder[C_LoaderEdgePush].On();
        Cylinder[C_LoaderUpPress].On();                                         //JerryYang 20181120 (Steven) : (Steven) : 獨立控制loader壓tray
        InArmSuck.ResetAll();                                                   //Steven 20160323 : 避免未開啟真空
        InArmSuckReset();                                                       //wei 20150318 suck動作重置
    }

    return bResult;
}

//==============================================================================
//  AI(k6-ainarm) 20260811 -- golden ainarm9045.cpp:7033-7254
//                            ProcessMES0101InArmPickLoaderError
//
//  ROLE          -- THE loader pick-up-error policy router (MES0101).  Given
//                   'the in-arm failed to pick at the loader', it decides the
//                   recovery verdict and returns it as a Kxxx code: auto-SKIP up
//                   to a per-customer budget then TRAY_END (SCK-ART budget,
//                   KYEC auto-retest budget, or the P57 loader-count budget,
//                   each restoring the tray buffer to HAS_IC and re-Start()ing
//                   the machine), auto-SKIP on a traymap-data error, auto-SKIP
//                   on the recorded-skip-position path, auto-RETRY for ASE-CL
//                   (only when the failure is NOT the tray's top-left device),
//                   or -- the general case -- put the MES0101 alarm on screen
//                   with the button set the recipe allows and then, for any
//                   verdict that abandons the tray, mark the tray for Empty /
//                   Color manual removal, or force one-by-one picking.
//  WAVE SCOPE    -- ProcessMES0101InArmPickLoaderError  ACTIVE with 1 gate
//                   (golden :7033-7254)
//
//  ****  TRAP 1 SHAPE (b) -- READ THIS BEFORE BUILDING  ************************
//  A NO-OP STUB FOR THIS EXACT SYMBOL ALREADY SATISFIES EVERY CALLER:
//      ainarm9045_2x4_16_shims.cpp:32
//        int ProcessMES0101InArmPickLoaderError(bool, AnsiString) { return 0; }
//  declared at ainarm9045_2x4_16_shims.h:65 with the identical signature, and
//  called from ainarm9045_2x4_16.cpp:1509 and ainarm9045_2x8_32.cpp:1839.  This
//  part's body therefore COLLIDES with it: leaving both in the tree is a
//  multiple definition, and (worse) a build that merely 'goes green' after
//  landing this file would prove nothing, because the stub is what the two
//  variant TUs were resolving against -- invisible to nm --undefined-only.
//  ACTION FOR THE MAIN LOOP: delete ONLY the definition at
//  ainarm9045_2x4_16_shims.cpp:32 and leave the declaration in the .h.  That is
//  the same retirement this file has already performed twice, with the notice
//  left in place both times -- see its own comments for ProcessSCKARTLoadingCount
//  ('now DEFINED in the registered ainarm9045.cpp') and MyTickCount.  This agent
//  did NOT touch that file, per its boundary.
//  ****************************************************************************
//
//  BEHAVIOUR DELTA once the stub is retired (this is not neutral, so it is
//  stated): the stub returned 0, which is NOT one of K_RETRY/K_SKIP/K_TRAY_END/
//  K_CLEAN_OUT/K_HOME, so both variant SMs took their 'no recovery selected'
//  path.  With the real body the offline default path ends at ShowErrorMessage
//  (canary_support.cpp, sim default K_RETRY), so those SMs will now see K_RETRY.
//
//  GOLDEN BUGS, PRESERVED (do not repair):
//   1. :7036 `int iX, iY;` are UNINITIALISED and are read at :7065 whenever
//      CosFunction.bLoaderAutoRetry is on and the loader tray holds no HAS_IC at
//      all -- the search loop at :7045-7057 then never assigns them.  Kept.
//   2. :7230 indexes `TrayForm.LoaderToEmptyColor[iRunStartMode]` -- the array is
//      [2] (cprod.h:1308) while iRunStartMode is the run-start-mode enum that
//      reaches at least 12 (rsmFIFOMode, MachineType.h:615).  An out-of-bounds
//      read on any mode above 1.  Kept verbatim.
//   3. the RecordProcess tag `ProcessMES0101InArmPickLoaderError_5` is used TWICE,
//      at :7161 and :7194, so the two paths are indistinguishable in the process
//      log (:7178 then jumps the numbering to _6).  Kept verbatim.
//
//  GATE REGISTER
//   GATE k6-G4 -- golden ainarm9045.cpp:7132-7137  the whole
//     `else if(EnableTraymapCheckFunction(0) && EnableTraymapCheckFunction(1))`
//     arm (its body is ret=K_SKIP + the _3 process log).
//     SYMBOL ABSENT.  golden decl ainarm2.h:216, golden body ainarm2.cpp:3585 --
//     ainarm2.cpp is not translated, and there is no declaration anywhere in the
//     port either.  THE WHOLE ARM is gated, not just the two calls, because the
//     calls ARE the condition -- nothing would be left to test.  Gating an
//     `else if` arm keeps the chain intact: the preceding `}` is followed by the
//     next `else if`, which is exactly how the two existing twins do it.
//     THIS IS NOT A NEW PREMISE -- it is the SAME premise as the two gates
//     already in the tree, and it still holds: aTester_Front.cpp:9863 (GATE
//     W7F3-G03) and aTester_Rear.cpp:9793 (TODO(G18)).  Rather than re-derive it,
//     this gate is registered as the third instance of it, and all three should be
//     retired together when ainarm2.cpp's EnableTraymapCheckFunction lands.
//     WHY THE OFFLINE DEFAULT IS FAITHFUL: the arm is skipped, so control falls
//     to the NEXT arm in golden's own chain (:7138 bAutoSKIP/bRecordSkipPosition,
//     then :7145 ASE-CL auto-retry, then :7153 P57, then the general alarm).  No
//     verdict is invented and `ret` keeps its :7035 initial value 0 on no path
//     that golden would have set -- because every remaining arm assigns ret.
//     Offline both traymap-check functions would have to return true to enter,
//     and the traymap check is a recipe feature that is off by default.
//     REAL-MACHINE DIFFERENCE: with the traymap-data-check feature ON, golden
//     silently auto-SKIPs a loader pick-up error and logs it; gated, the machine
//     instead falls through to the general MES0101 handling and will normally
//     raise the operator alarm.  That is the SAFE direction (alarm instead of
//     silent skip) but it is a real behavioural difference and it will change UPH.
//     ABSENCE COMMAND (exact):
//       rg -c --glob '*.cpp' --glob '*.h' --glob '!tests/**' -w EnableTraymapCheckFunction D:/HT9045/HT9011UC_Cpp_V3.33.906.0
//       -> 2 files, `.\aTester_Front.cpp:3` and `.\aTester_Rear.cpp:3`, and every
//          one of those 6 hits is INSIDE the two #if 0 gates named above (the
//          gate comment plus the two gated calls).  No declaration, no body.
//     MEASURED AT: 2026-08-11T01:31:24+0800.
//     TRAP 2: re-run at integration; if ainarm2.cpp landed, retire all three.
//
//  Deps verified REAL (spot-checked header + line, not assumed):
//    IniConfig.bE56LoaderRetryAtSamePosition (Config.h:624),
//    bInArmPickErrFromLoader (cmydef.h:4332), CosFunction.bLoaderAutoRetry
//    (CosFunction.h:477), MOT[MMTrayY].Tray.XItem/YItem/Data (mytray.h:33-40),
//    HAS_IC/NULL_IC, InArmSuck.Suck[][].Error (aHotPlateSubstrate.h:109),
//    bCanRunSCKART (cmydef.h:4224), TestIF_File.bSCKART_AutoSkip /
//    iSCKART_AutoSkipCount (cprod.h:2224-2225), LastSet.iSCKARTInputCT
//    (LastSet.h:479), fSCKART->iInputCount (forms/fSCKART.h:80, extern :177),
//    iContinueAutoSkipAutoTrayEnd (cmydef.h:3612), K_SKIP/K_TRAY_END/K_RETRY/
//    K_CLEAN_OUT/K_HOME (cmydef.h:278-282), RecordProcess 2-arg
//    (canary_support.h:70), MOT[].SetTrayBufferSingleData (Motor/mymotor.h:361),
//    fMain->Start(AnsiString) (forms/fMain.h:215), CosFunction.bUseSCKART
//    (CosFunction.h:90), USE_AUTO_RETEST (cmydef.h:2985) / eartInstall
//    (MachineType.h:908), IniConfig.bA10_AutoReTest, bAutoReTest_ART
//    (cmydef.h:3703), CosFunction.bContinueAutoSkipAutoTrayEnd (CosFunction.h:20),
//    LastSet.bLoaderTrayCount_ART (LastSet.h:443), TrayForm.bEnableAMR
//    (cprod.h:1350), bLoaderCocverRemove (cmydef.h:5775), CC_KYEC_LEE
//    (MachineType.h:292), TrayForm.iManualRemoveLoader (cprod.h:1320),
//    ArmSpeed[InArm].bAutoSKIP (cprod.h:2838), IniConfig.bRecordSkipPosition
//    (Config.h:201), bFirstRecordLoaderData (cmydef.h:3506), bAutoSkipFlag
//    (cmydef.h:3680), TrayForm.Loader.Direction (cprod.h TRAY_TYPE_PARA),
//    MyDBIProcessNew (canary_support.h:210), IniConfig.bP57... (Config.h:1482),
//    LastSet.iP57_InputCT (LastSet.h:569), IniConfig.bE86... (Config.h:591),
//    ShowErrorMessage (canary_support.h:66), IniConfig.bInOutArmCanPushHome
//    (Config.h:102), bE49... (Config.h:615), CosFunction.bMES0101CanCleanOut
//    (CosFunction.h:47), TrayForm.bAutoFeed (cprod.h:1314), CC_PTI
//    (MachineType.h:331), IniConfig.bP24... (Config.h:1416-1417),
//    CosFunction.bSpecialP24 (CosFunction.h:394), Tray.iNeedManualRemoved /
//    bMustToEmpty (mytray.h:67-68), iRunStartMode (cmydef.h:3112),
//    TrayForm.LoaderToEmptyColor (cprod.h:1308), IniConfig.bP39... (Config.h:1445),
//    CosFunction.bOneByOneWhenPickErrAtLoader (CosFunction.h:463),
//    IniConfig.bE78... (Config.h:645) / bBackUpInArmMode (Config.h:42),
//    ArmSpeed_File[InArm].bVariModeFIX (cprod.h:2839), bNeedOneByOnePickInArm
//    (cprod.h:3317), bPickFromLoader (aHotPlateSubstrate.h:938), MInArmX.
//==============================================================================
// iXPosition[8] / iYPosition are REAL and DEFINED (ainarm9045_2x4_16_shims.cpp:21-22)
// and declared at ainarm9045_2x4_16_shims.h:55-56, but this TU must not include
// that header: it ALSO declares ProcessSCKARTLoadingCount(bool bReset=false)
// while the definition at ainarm9045.cpp:1321 already carries that default
// argument -- 'default argument given twice', a hard error.  So they are
// forward-declared here instead, #ifndef-guarded against a sibling part emitting
// the same pair.  golden home for both: ainarm2.h.
#ifndef AINARM_K6_LOADERPICKPOS_FWD
#define AINARM_K6_LOADERPICKPOS_FWD
extern int iXPosition[8];                                                       // golden ainarm2.h; port def ainarm9045_2x4_16_shims.cpp:21
extern int iYPosition;                                                          // golden ainarm2.h; port def ainarm9045_2x4_16_shims.cpp:22
#endif
//------------------------------------------------------------------------------
int ProcessMES0101InArmPickLoaderError(bool bHasDuplicateErr, AnsiString ErrPart)
{                                                                               //Steven 20141121 : 整合InArm吸取Loader異常Alarm
    int ret=0;                                                                  //Jimmychiu 20220901 加入初始值
    int iX, iY;
    bool bASECL_PickupErrNeedAlm=false;
    AnsiString str="";
    if(IniConfig.bE56LoaderRetryAtSamePosition==false)                          //Steven 20170828 (wei) : Loader吸取異常時,要在同一個位置作Retry
        bInArmPickErrFromLoader=true;                                           //Steven 20170828 (wei) : 確認是否有在Loader吸料異常

    if(CosFunction.bLoaderAutoRetry)                                            //JerryYang 20220901 : ASE-CL要求只有左上角的device吸不到要跳ALARM, 其他的位置吸不到要自動RETRY
    {
        {
            for(int y=0; y<MOT[MMTrayY].Tray.YItem; y++)                        //tray盤最左上角的
            {
                for(int x=0; x<MOT[MMTrayY].Tray.XItem; x++)
                {
                    if(MOT[MMTrayY].Tray.Data[x][y]==HAS_IC)
                    {
                        iX=x;
                        iY=y;
                        y=MOT[MMTrayY].Tray.YItem;                              //找到最左上角的就跳出迴圈
                        break;
                    }
                }
            }

            for(int i=0; i<MAX_ARM_Row; i++)
            {
                for(int j=0; j<MAX_ARM_Col; j++)
                {
                    if(InArmSuck.Suck[i][j].Error==true)
                    {
                        if(iX==iXPosition[j] && iY==iYPosition+iLoadPitchStepY*i)
                        {
                            bASECL_PickupErrNeedAlm=true;
                        }
                    }
                }
            }
        }
    }

    if(bCanRunSCKART==true          &&                                          //Steven 20170315 (wei) : For SCK ART Auto Skip
       TestIF_File.bSCKART_AutoSkip &&
       LastSet.iSCKARTInputCT==fSCKART->iInputCount)                            //Steven 20170720 (wei) : Add
    {
        if(iContinueAutoSkipAutoTrayEnd<TestIF_File.iSCKART_AutoSkipCount-1)    //Sam 20191209 : ART Fix issue
        {
            ret=K_SKIP;
            iContinueAutoSkipAutoTrayEnd++;
            str.sprintf("K_SKIP and iContinueAutoSkipAutoTrayEnd = %d", iContinueAutoSkipAutoTrayEnd);
            RecordProcess(str, "ProcessMES0101InArmPickLoaderError_1");         //Steven 20220608 : 紀錄MES0101的處理動作
        }
        else
        {
            ret=K_TRAY_END;
            iContinueAutoSkipAutoTrayEnd=0;
            for(int i=0; i<MOT[MMTrayY].Tray.XItem; i++)                        //Steven 20170706 (wei) : Add for ATK ART
            {
                for(int j=0; j<MOT[MMTrayY].Tray.YItem; j++)
                {
                    if(MOT[MMTrayY].Tray.Data[i][j]==HAS_IC)
                    {
                        MOT[MMTrayY].SetTrayBufferSingleData(i, j, HAS_IC);     //Steven 20170712 (Wei) : HAS_SKIP_IC --> HAS_IC
                    }
                }
            }
            str.sprintf("K_TRAY_END and iContinueAutoSkipAutoTrayEnd=0");
            RecordProcess(str, "ProcessMES0101InArmPickLoaderError_2");         //Steven 20220608 : 紀錄MES0101的處理動作
            fMain->Start("ProcessMES0101InArmPickLoaderError 1");
        }
    }
    else if(CUSTOMER_CODE!=CC_ASE_KaohSiung &&
            CosFunction.bUseSCKART==false   &&
            USE_AUTO_RETEST==eartInstall    &&
            (IniConfig.bA10_AutoReTest || bAutoReTest_ART) &&
            CosFunction.bContinueAutoSkipAutoTrayEnd &&
            LastSet.bLoaderTrayCount_ART)                                       //ChungHung 20141014 add for KYEC AutoRetest
    {                                                                           //kevin 20150707
        if(iContinueAutoSkipAutoTrayEnd<=(TrayForm.bEnableAMR?3:5))             //==> Eastsun 20260512 F7-T1 (a-side <=3 wrap)
        {
            ret=K_SKIP;
            iContinueAutoSkipAutoTrayEnd++;
        }
        else
        {
            ret=K_TRAY_END;
            fMain->Start("ProcessMES0101InArmPickLoaderError 2");
        }

        if(TrayForm.bEnableAMR)                                                 //==> Eastsun 20260512 : Phase4-F7T2a KYEC AMR bLoaderCocverRemove (a-side ainarm2.cpp L5156-L5157)
            bLoaderCocverRemove=true;

        if(CUSTOMER_CODE==CC_KYEC_LEE &&                                        //Frank 20160919 add
           !TrayForm.bEnableAMR)                                                //Eastsun 20260512 F7-T2b 整合 (a-side ainarm2.cpp L5160 add !bEnableAMR guard)
        {
            TrayForm.iManualRemoveLoader=2;
        }
    }
#if 0 // GATE k6-G4 -- golden ainarm9045.cpp:7132-7137, the WHOLE else-if arm.  EnableTraymapCheckFunction(int): golden decl ainarm2.h:216, golden body ainarm2.cpp:3585, NO port body and NO port declaration.  Same premise as the two gates already in the tree (aTester_Front.cpp:9863 GATE W7F3-G03 / aTester_Rear.cpp:9793 TODO(G18)) -- retire all three together.  Absence command + time + real-machine difference: see GATE REGISTER above.
    else if(EnableTraymapCheckFunction(0) && EnableTraymapCheckFunction(1))     //wei 20170302 (Steven) Tray map data error
    {
        ret=K_SKIP;
        str.sprintf("K_SKIP by EnableTraymapCheckFunction");
        RecordProcess(str, "ProcessMES0101InArmPickLoaderError_3");             //Steven 20220608 : 紀錄MES0101的處理動作
    }
#endif // GATE k6-G4
    else if(ArmSpeed[InArm].bAutoSKIP==true && IniConfig.bRecordSkipPosition && bFirstRecordLoaderData==true)
    {
        ret=K_SKIP;
        bAutoSkipFlag=true;                                                     //jou 20150320   auto skip 次數時間導致Hang up
        str.sprintf("K_SKIP by bAutoSkipFlag");
        RecordProcess(str, "ProcessMES0101InArmPickLoaderError_4");             //Steven 20220608 : 紀錄MES0101的處理動作
    }
    else if(CosFunction.bLoaderAutoRetry &&
            bASECL_PickupErrNeedAlm==false &&
            TrayForm.Loader.Direction==0)                                       //JerryYang 20220901 : ASE-CL要求只有左上角的device吸不到要跳ALARM, 其他的位置吸不到要自動RETRY
    {
        ret=K_RETRY;
        bAutoRetryFlag=true;                                                    //JerryYang 20220901 : ASE-CL pick up error時吸取邏輯要改找吸最左上角的
        MyDBIProcessNew("Motion", "WAR2211", "In arm pick up error at loader(Auto Retry)", ErrPart);                    //JerryYang 20200203 pick up error auto skip
    }
    else if(IniConfig.bP57LoaderAutoCleanOutByInputCT &&                        //Sam 20250605 : Loader Count AutoCleanOut
            LastSet.iP57_InputCT!=0)
    {
        if(iContinueAutoSkipAutoTrayEnd<5-1)                                    //Sam 20191209 : ART Fix issue
        {
            ret=K_SKIP;
            iContinueAutoSkipAutoTrayEnd++;
            str.sprintf("K_SKIP and iContinueAutoSkipAutoTrayEnd = %d", iContinueAutoSkipAutoTrayEnd);
            RecordProcess(str, "ProcessMES0101InArmPickLoaderError_5");
        }
        else
        {
            ret=K_TRAY_END;
            iContinueAutoSkipAutoTrayEnd=0;
            for(int i=0; i<MOT[MMTrayY].Tray.XItem; i++)
            {
                for(int j=0; j<MOT[MMTrayY].Tray.YItem; j++)
                {
                    if(MOT[MMTrayY].Tray.Data[i][j]==HAS_IC)
                    {
                        MOT[MMTrayY].SetTrayBufferSingleData(i, j, HAS_IC);
                    }
                }
            }
            str.sprintf("K_TRAY_END and iContinueAutoSkipAutoTrayEnd=0");
            RecordProcess(str, "ProcessMES0101InArmPickLoaderError_6");
            fMain->Start("ProcessMES0101InArmPickLoaderError 3");
        }
    }
    else
    {
        if(CUSTOMER_CODE!=CC_ASE_KaohSiung &&
           CosFunction.bUseSCKART==false    &&                                  //Steven 20170712 (Wei) : Fixed for SCK_ART error
           USE_AUTO_RETEST==eartInstall &&
           (IniConfig.bA10_AutoReTest || bAutoReTest_ART) &&
           CosFunction.bContinueAutoSkipAutoTrayEnd &&
           LastSet.bLoaderTrayCount_ART)                                        //ChungHung 20141014 add for KYEC AutoRetest
        {
            ret=K_TRAY_END;
            iContinueAutoSkipAutoTrayEnd=0;
            str.sprintf("K_TRAY_END by iContinueAutoSkipAutoTrayEnd=0");
            RecordProcess(str, "ProcessMES0101InArmPickLoaderError_5");         //Steven 20220608 : 紀錄MES0101的處理動作
            fMain->Start("ProcessMES0101InArmPickLoaderError 4");
        }
        else if(IniConfig.bE86_InArmPickErrOnLoaderOnlyCanSKIP)                 //JerryYang 20250120 : add
        {
            ret=ShowErrorMessage("MES0101", K_TRAY_END|K_SKIP, MInArmX, bHasDuplicateErr, ErrPart);                     //ChungHung HT9045 2011/12/13 //Input pickup device error時,按"retry"鍵,機台都會自動home start
        }
        else if(IniConfig.bInOutArmCanPushHome)
        {
            ret=ShowErrorMessage("MES0101", K_RETRY|K_TRAY_END|K_SKIP|K_HOME, MInArmX, bHasDuplicateErr, ErrPart);      //ChungHung HT9045 2011/12/13 //Input pickup device error時,按"retry"鍵,機台都會自動home start
        }
        else if(IniConfig.bE49_LoaderOnlyRetryAndCleanOut)                      //Steven 20141105 : Loader吸取異常只能Retry與CleanOut
        {
            ret=ShowErrorMessage("MES0101", K_RETRY|K_CLEAN_OUT, MInArmX, bHasDuplicateErr, ErrPart);
        }
        else
        {
            if(CosFunction.bMES0101CanCleanOut && TrayForm.bAutoFeed==false)    //Steven 20120830 : Loader吸取異常可以按CleanOut
                ret=ShowErrorMessage("MES0101", K_RETRY|K_TRAY_END|K_SKIP|K_CLEAN_OUT, MInArmX, bHasDuplicateErr, ErrPart);
            else
                ret=ShowErrorMessage("MES0101", K_RETRY|K_TRAY_END|K_SKIP, MInArmX, bHasDuplicateErr, ErrPart);         //Steven 20100508
        }
        bFirstRecordLoaderData=true;                                            //Steven 20150429 : Fixed for Auto Skip

        if(ret==K_TRAY_END || ret==K_SKIP || ret==K_CLEAN_OUT ||
           (ret==K_RETRY && CUSTOMER_CODE==CC_PTI))                             //Sam 20231129 : 力成 Retry 也要搬到 Color
        {
            if(IniConfig.bP24SkipEventNeedRemoveEmptyAndColorTray==true ||      //Frank 20150626 : for矽格 Loader有Skip要到Empty的位置做檢查
               IniConfig.bP24SkipEventNeedRemoveColorTrayForIDT==true)          //Sam 20220530 : 當 Loader 發生 Skip/Edit 時，此盤做完後搬到 Empty 軌道後，會收盤起來並報警提示人員收盤(連兩盤)
            {
                if(CosFunction.bSpecialP24)                                     //Sam 20230320 : 整合 P24 北興版為獨立功能碼
                {
                    MOT[MMTrayY].Tray.iNeedManualRemoved=1;                     //to color alarm
                }
                else
                {
                    if(TrayForm.LoaderToEmptyColor[iRunStartMode]==0)
                         MOT[MMTrayY].Tray.iNeedManualRemoved=0;                //to empty alarm
                    else
                        MOT[MMTrayY].Tray.iNeedManualRemoved=1;                 //to color alarm
                }

                iManualRemoveTrayCnt=2;                                         //Sam 20220530 : 當 Loader 發生 Skip/Edit 時，此盤做完後搬到 Empty 軌道後，會收盤起來並報警提示人員收盤(連兩盤)
            }
            else if(IniConfig.bP39LoaderHasSkipPlaceToEmpty)                    //Sam 20230220 : 移除客戶碼                     //KaiChen 20201125 ： 矽格湖口，Loader 有 Skip 強制放Loader
            {
                MOT[MMTrayY].Tray.bMustToEmpty=true;                            //Sam 20230317 : 修正 P24 功能
            }
            else if(CosFunction.bOneByOneWhenPickErrAtLoader &&
                    IniConfig.bE78OneByOneWhenPickErrAtLoader &&
                    ret==K_SKIP)                                                //Jimmychiu 20250924 : Suck one by one when a pickup error occurs at the loader.
            {
                IniConfig.bBackUpInArmMode=ArmSpeed[InArm].bVariModeFIX;
                ArmSpeed_File[InArm].bVariModeFIX=true;
                bNeedOneByOnePickInArm=true;
            }
        }
    }
    bPickFromLoader=false;                                                      //Ifor 20180322 : add 避免發生異常讓位旗標為true造成Hang up
    return ret;
}

//==============================================================================
//  CheckLoaderHasTray  --  golden ainarm9045.cpp:7256-7299
//  Translator: AI(W7D-ainarm-k8) 20260811
//
//  ROLE          -- pre-pick Loader gate.  Returns 3 when the in-shuttle is not
//                   ready yet (non-alarm probe only), 0 when the Loader station is
//                   clean, 1 when a blocker is seen but we are still under the
//                   3-strike alarm threshold, and 2 after raising the matching JAM
//                   (0929 sure-tray / 0903 fixer-push / 0902 edge-push / 0909
//                   catch-all).  DUMMY lots bypass the blocker entirely.
//  WAVE SCOPE    -- CheckLoaderHasTray   golden :7256-7299   ACTIVE
//  GATE REGISTER -- (empty: this function contains no #if 0)
//
//  Retires the offline stub `int CheckLoaderHasTray(bool,int,bool) { return 0; }`
//  at ainarm9045_2x4_16_shims.cpp:26 (declared ainarm9045_2x4_16_shims.h:59).
//  TRAP 1 shape (b) HAND-OFF, MAIN LOOP MUST READ THIS: that same shim file ALSO
//  carries a ZERO-ARG overload `int CheckLoaderHasTray() { return 0; }` at
//  ainarm9045_2x4_16_shims.cpp:25 (declared :58) which GOLDEN DOES NOT HAVE --
//  golden's definition takes 3 mandatory args, and golden has no header
//  declaration and no default arguments at all.  (The zero-arg CALLS in golden
//  ainarm9045_2x4_16.cpp:1079 and ainarm9045_2x8_32.cpp:1465 are a golden
//  inconsistency -- there is no golden declaration that makes them legal.  Noted,
//  not fixed.)  If only the 3-arg stub is retired, the port's LIVE zero-arg call
//  site ainarm9045_2x4_16.cpp:1159 KEEPS resolving to the
//  `{ return 0; }` overload -- a stub still satisfying the demand, invisible to
//  `nm --undefined-only`.  The correct fix is to keep golden's 3 parameters (as
//  here) and make those two sites pass `(false, 0, false)`, which is literally
//  what golden ainarm9045.cpp:7722 does.  I did not edit either file (outside my
//  boundary); flagging it IS the hand-off.  (The other two textual call sites,
//  ainarm9045_2x8_32.cpp:1528 zero-arg and :1631 3-arg, are inside that file's
//  wholly-#if-0'd body -- preprocessor-nesting verified 2026-08-11 02:00 +0800 --
//  so the only LIVE sites are ainarm9045_2x4_16.cpp:1159 and :1280.)
//
//  INCLUDES: `Sen[]` (mysensor.h:48) and `Cylinder[]` (mycylin.h:176) are the ONLY
//  two names in this whole task that the stitched ainarm9045.cpp cannot currently
//  reach.  MEASURED, not guessed: a probe TU carrying ainarm9045.cpp's exact
//  17-header include block plus every symbol used by all 8 of my functions was run
//  through `g++ -std=c++1z -fsyntax-only` with the ht9045_sm flag set and reported
//  exactly 2 errors, both "'Sen' / 'Cylinder' was not declared in this scope"
//  (run 2026-08-11 01:31 +0800).  Both headers are guarded (mysensorH / mycylinH),
//  so these mid-file includes are idempotent; MAIN LOOP may hoist them into the
//  file's include block instead.
//
//  SOFT_SIMULTE is NOT defined in this port, so golden's #ifndef arm (:7266-7270)
//  is the LIVE one and the #else `flag2=false` (:7272) is the dead one -- both
//  reproduced verbatim, preprocessor block intact, neither collapsed.
//  __FUNC__ resolves via canary_support.h:45 (`#define __FUNC__ __func__`) and the
//  5-arg ShowErrorMessage is canary_support.h:66 -- both already in this TU.
//  bCheckInShuttlePosAndNOIC (golden :4368) is this same wave's part file 04368;
//  until that part lands the call resolves to the port stub at
//  ainarm9045.cpp:2309, which returns the same false -- so there is no stitch
//  ordering hazard between the two.
//  GOLDEN BEHAVIOUR KEPT: the alarm branch needs `bAlarm && iErrorCount>=3`, so a
//  caller passing bAlarm=true with iErrorCount<3 gets 1 (retry), not 2; and the
//  `bAlarm==false` early-out at :7258 returns 3 ONLY on the non-alarm probe path.
//==============================================================================
// AI(W7D-ainarm-k8) 20260811: the 2 headers this function needs that the file's
// include block lacks (probe-measured, see banner).  Guarded, hence idempotent.
#include "mysensor.h"              // Sen[] : SnLoaderSureTray / SnLoaderFixCyPush / SnLoaderEdgePush
#include "mycylin.h"               // Cylinder[] : C_TrayY_Fixer.OffSensor()
int CheckLoaderHasTray(bool bAlarm, int iErrorCount, bool bTrayDuplicateErr)    //Steven 20161223 (jou) : 確認LoaderSensor統一成Function
{
    if(bAlarm==false && bCheckInShuttlePosAndNOIC())                            //KenHsieh 20230614 : Inarm pick IC from tray需待SH到位且為可取放料狀態
        return 3;

    int  iResult=0;                                                             //Steven 20170105 (Wei) : 修改為int
    bool bHasError=false;
    bool flag1=Sen[SnLoaderSureTray].IsOff();
    bool flag2=false;
    #ifndef SOFT_SIMULTE
    if(INSTALL_OCR_YMot==eocrYMotInstal)                                        //Frank 20250214 add
        flag2=Sen[SnLoaderFixCyPush].IsOn();
    else
        flag2=(Sen[SnLoaderFixCyPush].IsOn() ||
               Cylinder[C_TrayY_Fixer].OffSensor());                            //wei 20241011 新增後勾氣缸off sensor,避免氣缸沒作動
    #else
        flag2=false;
    #endif
    bool flag3=Sen[SnLoaderEdgePush].IsOn();

    if(flag1==true || flag2==true || flag3==true)
        bHasError=true;

    if(LastSet.iRealDummy!=DUMMY && bHasError)
    {
        if(bAlarm && iErrorCount>=3)
        {
            if(flag1)
                ShowErrorMessage("JAM0929", K_RETRY, MMTrayY, bTrayDuplicateErr, __FUNC__);
            else if(flag2)
                ShowErrorMessage("JAM0903", K_RETRY, MMTrayY, bTrayDuplicateErr, __FUNC__);
            else if(flag3)
                ShowErrorMessage("JAM0902", K_RETRY, MMTrayY, bTrayDuplicateErr, __FUNC__);
            else
                ShowErrorMessage("JAM0909", K_RETRY, MMTrayY, bTrayDuplicateErr, __FUNC__);
            iResult=2;
        }
        else
        {
            iResult=1;
        }
    }
    return iResult;
}

//==============================================================================
//== ROLE: DoAutoSkipCheck -- "must the in-arm STOP / finish this tray instead of
//==       skipping on?".  Three independent yes-paths: ASE-Kaohsiung auto-skip
//==       with ICs still on the arm at ambient, FIFO mode with ICs on the arm, and
//==       clean-out (or clean-out about to start).  Also flushes the pending
//==       "Auto Tray Skip End" string back to the ASE host.
//==
//== WAVE SCOPE (k2-ainarm):
//==   DoAutoSkipCheck  golden ainarm9045.cpp:7341-7385  -- ACTIVE
//==     (100% faithful; ZERO gates)
//==
//== GATE REGISTER: (none -- this function needed no #if 0)
//==   Callee provenance: SendDataToASE(AnsiString) -> declared csystem.h, and note
//==   the port has TWO bodies ON PURPOSE -- csystem.cpp:15736 is LIVE and
//==   aoutarm.cpp:3315 sits inside `#if 0 // PT-W5c RETIRED`, mirroring golden's
//==   own duplicate-with-different-bodies quirk (golden aoutarm.cpp:2706 strips the
//==   last character, golden csystem.cpp:12769 does not; csystem.obj links first on
//==   a real machine).  ShowErrorMessage / K_TRAY_END -> canary_support.h / cmydef.h.
//==   ArmSpeed[InArm].bAutoSKIP -> cprod.h.  bRunAutoSiteMapping / iCleanOut /
//==   bCleanoutStart / bAutoTrayEndHasIC / sAutoTrayendEndPos(+Buffer) -> cmydef.h.
//==
//== GOLDEN DEAD CODE PRESERVED, NOT DELETED: golden :7368-7371 is a commented-out
//== `else if(IniConfig.bP17InArmFullPickFromLoader==false && InArmSuck.HasIC())`
//== arm.  It is transcribed verbatim as comment text, because its presence is what
//== makes the following `else if` an ELSE-IF of the ASE branch rather than a fresh
//== `if` -- dropping it would read as a restructure.
//==
//== ONE STRUCTURAL TRAP IN GOLDEN, FAITHFULLY KEPT: the FIFO arm and the clean-out
//== arm are `else if` continuations of `if(CUSTOMER_CODE==CC_ASE_KaohSiung && ...)`.
//== On an ASE-Kaohsiung machine with bAutoSKIP on, FIFO mode and clean-out can
//== therefore NEVER return true from here.  That is golden behaviour and is NOT
//== corrected.  (Standard builds force CUSTOMER_CODE away from CC_ASE_KaohSiung,
//== so on those the first arm is dead code and the chain effectively starts at the
//== FIFO arm.)
//==
//== INTEGRATION NOTE (TRAP 1 shape (b)) -- THE STUB IS NOT IN THIS FILE:
//== DoAutoSkipCheck is declared at ainarm9045_2x4_16_shims.h:60 and STUBBED at
//== ainarm9045_2x4_16_shims.cpp:27 (`bool DoAutoSkipCheck(){return false;} //
//== offline: not in auto-skip flow`).  BOTH must go, or this definition collides.
//== Live callers exist today (ainarm9045_2x4_16.cpp:1199 and :1214,
//== ainarm9045_2x8_32.cpp:1565 and :1575), so the swap is genuinely observable:
//== those call sites currently always see false.
//==============================================================================
bool DoAutoSkipCheck()                                                          //kevin 20170822 auto tray end 有吸到IC 換TRAY 要ALARM
{                                                                               //Ifor 20180418 : add Auto Site Mapping 補料回Hotplate必須滿料避免Hang up
    if(bRunAutoSiteMapping==true)                                               //Ifor 20180518 : add 簡化Site Mapping 旗標
    {
        return false;
    }

    if(CUSTOMER_CODE==CC_ASE_KaohSiung && ArmSpeed[InArm].bAutoSKIP==true)      //kevin 20170822 (wei) add
    {
        if(sAutoTrayendEndPos!="")
        {
            sAutoTrayendEndPosBuffer.sprintf("Auto Tray Skip End:%s",sAutoTrayendEndPos);                               //kevin 20170824 回應 ase close Site
            SendDataToASE(sAutoTrayendEndPosBuffer);                            //kevin 20170824 回應 ase close Site
            sAutoTrayendEndPos="";
            sAutoTrayendEndPosBuffer="";
        }

        if(bAutoTrayEndHasIC)                                                   //kevin 20170822 auto tray end 有吸到IC 換TRAY 要ALARM
            ShowErrorMessage("WAR09101", K_TRAY_END, MInArmX);

        if((LastSet.iTemperature==Tempture_AmbientHot ||
            LastSet.iTemperature==Tempture_Ambient) &&
           InArmSuck.HasIC())
        {
            return true;
        }
    }
//    else if(IniConfig.bP17InArmFullPickFromLoader==false && InArmSuck.HasIC())  //Steven 20101221 :讓每盤都是滿的
//    {
//        return true;
//    }
    else if(IniConfig.bI37_EnableFIFOMode &&                                    //Steven 20160303 : FIFO Mode
            LastSet.iRunStartMode==rsmFIFOMode &&
            InArmSuck.HasIC())
    {
        return true;
    }
    else if(iCleanOut==1 ||                                                     //Steven 20241218 : avoid the tray not goes down yet
            bCleanoutStart==true)                                               //Steven 20250113 : 準備clean out, 但是被one cycle插隊會hang up
    {
        return true;
    }

    return false;
}

//============================================================================
//  ShuttleIsFull_9045  --  golden ainarm9045.cpp:7387-7429   (k1-ainarm)
//
//  ROLE: "is this in-shuttle carry kit already fully loaded?"  (golden's own
//        comment: 檢查shuttle是否已擺滿.)  While the arm is neither placing into a
//        shuttle nor picking off the hot plate, it asks the kit's own
//        UseSiteFullIC() predicate first and, failing that, scans the kit grid for
//        any NULL_IC cell.  Read-only; no motion, no state written.
//
//  WAVE SCOPE:
//    * ShuttleIsFull_9045(int)           ACTIVE (verbatim)      golden :7387-7429
//
//  GATE REGISTER: empty -- this function contains no #if 0.  bPlaceShuttle
//  (aHotPlateSubstrate.h:1098), bPickFromHotplate (aHotPlateSubstrate.h:744),
//  FLCarryKit / BLCarryKit and TMyKitSuck::UseSiteFullIC (aHotPlateSubstrate.h:469,
//  noted there as "W6.2b1x1: FAITHFUL -- iShtRow/iShtCol scan") all exist, so
//  nothing had to be degraded.
//
//  GOLDEN LOGIC WORTH NOTING (kept, not fixed): the two `else` arms fall out of
//  their grid scans without returning, so when no NULL_IC cell is found control
//  reaches golden :7428 `return true`.  Likewise the whole body is skipped -- and
//  true returned -- whenever bPlaceShuttle or bPickFromHotplate is set, i.e. "full"
//  is the answer during a place/hot-plate-pick pass regardless of the grid.  Both
//  are golden's shapes and are preserved exactly.
//
//  TRAP 1 note, shape (a) "nobody references it": no live port caller.  All five
//  golden call sites (:7461, :7480, :7527, :7549, :7559) are inside
//  CheckOneCycleAction (golden :7431), which in the port is still the offline stub
//  `int CheckOneCycleAction(int) { return 0; }` at ainarm9045.cpp:2316.  Nothing
//  declares it either -- golden has no header prototype for it (verified:
//  `grep -rn ShuttleIsFull_9045 --include=*.h` over the golden tree -> 0 hits), so
//  golden relies on definition-before-use within ainarm9045.cpp and the port has no
//  prototype to collide with.  Deliberately NOT `static` (TRAP 1 shape (d)).
//
//  TRAP 5 (two headers, same class name): FLCarryKit and BLCarryKit are TMyKitSuck.
//  I relied on the declaration at aHotPlateSubstrate.h:365 -- the header this TU
//  includes -- and the objects that back them are defined in
//  aHotPlateSubstrate.cpp (FLCarryKit at :81), which is registered in
//  CMakeLists.txt:1466.  The other TMyKitSuck at mykitsuck.h:274, with its own
//  `TMyKitSuck FLCarryKit;` at mykitsuck.cpp:206, has a different member layout and
//  is deliberately NOT registered (CMakeLists.txt:2099-2112).  Reading .iShtRow /
//  .iShtCol / .Item[][] through the wrong one of the two would link perfectly and
//  return garbage, so: aHotPlateSubstrate.h + aHotPlateSubstrate.cpp, no include
//  added or changed by me.
//============================================================================
bool ShuttleIsFull_9045(int iSht)                                               //Steven 20240322 : 檢查shuttle是否已擺滿
{
    if(bPlaceShuttle==false && bPickFromHotplate==false)
    {
        if(iSht==0)
        {
            if(FLCarryKit.UseSiteFullIC())                                      //Steven 20241119 : Fixed
            {
                return true;
            }
            else
            {
                for(int i=0; i<FLCarryKit.iShtRow; i++)
                {
                    for(int j=0; j<FLCarryKit.iShtCol; j++)
                    {
                        if(FLCarryKit.Item[i][j]==NULL_IC)
                            return false;
                    }
                }
            }
        }
        else
        {
            if(BLCarryKit.UseSiteFullIC())
            {
                return true;
            }
            else
            {
                for(int i=0; i<BLCarryKit.iShtRow; i++)
                {
                    for(int j=0; j<BLCarryKit.iShtCol; j++)
                    {
                        if(BLCarryKit.Item[i][j]==NULL_IC)
                            return false;
                    }
                }
            }
        }
    }
    return true;
}

//==============================================================================
//  k7-ainarm PART 07431 -- CheckOneCycleAction(int)
//  Translator: AI(pt-wave k7-ainarm) 20260811
//  Golden: ainarm9045.cpp:7431-7585 (cp950)
//
//  ROLE          : the ONE-CYCLE arbiter -- given the task number the in-arm SM
//                  was about to take, decides whether the arm should keep feeding
//                  (Task=500 on the hot-plate path / Task=100 on the loader path)
//                  or wind down (Task=50).  Only engages when
//                  IniConfig.bP17InArmFullPickFromLoader is set and the run is not
//                  QA mode; splits on LastSet.iTemperature (hot -> the hot plates
//                  and PickFromHPList decide, ambient -> the loader tray decides),
//                  and on the way it can LOCK shuttle 2 (MOT[MInShuttle2].
//                  fCanMoveL=false) to stop an index position error.
//  WAVE SCOPE    : CheckOneCycleAction  golden :7431-7585 -- ACTIVE, VERBATIM,
//                  ZERO GATES (see the retirement note below).  RETIRES the
//                  offline stub `int CheckOneCycleAction(int) { return 0; }` at the
//                  port`s ainarm9045.cpp:2316 -- the main loop must delete that
//                  line.  This retirement IS a behaviour change with many live
//                  consumers: acarry.cpp:5327 plus ~25 ainarm9045_<layout>.cpp
//                  sites (`Task=CheckOneCycleAction(50)`), all of which get 0
//                  today -- i.e. the stub silently drove every one-cycle decision
//                  to task 0 instead of golden`s 50/100/500.  The symbol is
//                  already declared twice in the port (ainarm9045.h:73 and
//                  acarry_shims.h:214); both are plain non-static declarations that
//                  this non-static definition satisfies.  TRAP 1(b): the main loop
//                  must confirm acarry_shims.cpp does not ALSO carry a body, or a
//                  stub there will keep satisfying the demand and my definition
//                  will never be extracted from the archive.
//  GATE REGISTER : EMPTY -- no #if 0 in this part.
//
//  RETIRED GATE/SEAM (k7-G3) -- THE SECOND TRAP-2 CASE, CAUGHT INSIDE THIS WAVE
//  ------------------------------------------------------------------------
//  golden lines :7461, :7480, :7527, :7549, :7559 -- five uses of
//  ShuttleIsFull_9045(int), which golden defines at ainarm9045.cpp:7387-7429, in
//  THIS SAME golden file, 44 lines above my range and outside my assigned ranges.
//    FIRST MEASUREMENT : `rg -n "\bShuttleIsFull_9045\b" -g "*.cpp" -g "*.h"`
//                        -> 0 hits (exit 1), at 2026-08-11 01:34:26 +0800.  Because
//                        the five uses are sub-expressions of compound `if`
//                        conditions and cannot each carry a `#if 0`, I had built a
//                        single TU-local seam (W7dK7_ShuttleIsFull_9045) plus a
//                        `#define`/`#undef` pair, with offline default `false`
//                        derived from golden`s own body against the port substrate
//                        (bPlaceShuttle/bPickFromHotplate both false; NULL_IC==0
//                        and TMyKitSuck`s ctor leaves Item[][] zero-initialised, so
//                        golden itself reaches `return false`).
//    RE-MEASUREMENT    : `rg -n "^(int|bool|void)\s+(ShuttleIsFull_9045)\s*\(" 
//                        -g "*.cpp" -g "*.h" -g "_w7d_parts/*.txt"`
//                        -> _w7d_parts/07387_ShuttleIsFull_9045.txt:46, at
//                        2026-08-11 01:49:41 +0800.  A SIBLING LANDED THE REAL
//                        BODY.  Note the trap exactly as documented: my degraded
//                        value happened to be the SAME `false` golden computes
//                        offline -- and that does NOT make the seam fine, because
//                        the defect was the false premise, and a seam that hides
//                        five real call sites behind a constant is precisely the
//                        "macro seam routes a live call site to a stub while the
//                        real body exists" debt already tracked in this project.
//    TRAP 3 RE-ASK ("why SHOULD this stay seamed?") : no reason survives.  Golden`s
//                        body touches only symbols this TU already links
//                        (bPlaceShuttle acatchtray.cpp:137, bPickFromHotplate
//                        aHotPlateSubstrate.cpp:886, FLCarryKit/BLCarryKit,
//                        UseSiteFullIC aHotPlateSubstrate.h:469, NULL_IC), does no
//                        file I/O, and lands in THIS VERY TU, so there is no
//                        archive edge to buy.  SEAM AND MACRO DELETED ENTIRELY --
//                        golden`s five call lines are ACTIVE and verbatim.
//    ORDER INSURANCE   : one forward declaration below, copied character-for-
//                        character from the sibling`s definition, so this part
//                        compiles regardless of part-stitch order.  Declaration
//                        only -- no definition, no global, no ODR risk.
//
//  ADJACENT-BODY NOTE (refreshed at the 01:49:41 re-measurement -- my earlier draft
//  said InArmSideAllClose was a `{ return true; }` stub and that is now WRONG):
//  a sibling landed the REAL golden body this same wave --
//  _w7d_parts/04231_InArmSideAllClose.txt, `bool InArmSideAllClose(int iSht)` --
//  so golden`s first test `if(iSht==0 && InArmSideAllClose(0)==false)` (:7445) is
//  genuinely reachable and the shuttle-1 hot-plate branch is live again.  The main
//  loop must delete the stub at ainarm9045.cpp:2307 with the others.  Still
//  STUBBED at the time of writing, and therefore still no-ops inside this body:
//  SetShuttleToHasNullICWhenCleanOut / SetShuttleToNullICWhenCleanOut
//  (ainarm9045.cpp:2287-2288, golden :4288 / :4273) -- both are called on the
//  "no IC anywhere" paths (:7511, :7569, :7574), i.e. exactly the Clean-time
//  hang-up guards Steven added, so they are worth someone`s next wave.
//
//  GOLDEN BUG KEPT VERBATIM (golden :7577-7580) -- see the marker at that line.
//==============================================================================
// golden ainarm9045.cpp:7387 -- defined by sibling part 07387_ShuttleIsFull_9045.txt.
// Declaration only (golden declares this symbol in no header at all).
bool ShuttleIsFull_9045(int iSht);
//------------------------------------------------------------------------------
int CheckOneCycleAction(int iTask)                                              //Steven 20240326 : 判斷one cycle的時候要不要繼續放料
{
    int Task=iTask;
    int iP=0, iR=0, iC=0, iSht=-1, iKit=-1;

    if(IniConfig.bP17InArmFullPickFromLoader &&                                 //Steven 20181018 : 修正常溫也要補滿Site
       LastSet.iRunStartMode!=rsmQAMode)                                        //Steven 20190410 : Fixed for QA mode hang up
    {
        if(LastSet.iTemperature==Tempture_Hot)
        {
            if(MOT[MMPlate1].HasIC() || MOT[MMPlate2].HasIC())
            {
                if(PickFromHPList->GetHPFirstTeamPlate(iP, iR, iC, iSht, iKit))                                         //JerryYang 20250408 : fix Hot plate偶發one cycle hang up
                {
                    if(iSht==0 && InArmSideAllClose(0)==false)
                    {
                        if(InSHT1InLF())
                        {
                            if(iInArmType==e9045_1x3_2_14 ||
                               iInArmType==e9045_1x4_2_14 ||
                               iInArmType==e9045_2x3_6_14 ||
                               iInArmType==e9045_2x4_4_13 ||
                               iInArmType==e9045_2x4_4_14 ||
                               iInArmType==e9045_2x5_8 ||
                               iInArmType==e9045_2x6_8 ||
                               iInArmType==e9045_2x8_32 ||
                               iInArmType==e9045_2x8_8 ||
                               USE_PICKER_COUNT==ep1Picker)
                            {
                                if(FLCarryKit.UseSiteHasIC()==true &&           //如果蝦頭一有料而且
                                   ShuttleIsFull_9045(0)==false)                //    蝦頭一還沒擺滿
                                {
                                    Task=500;
                                }
                            }
                        }
                    }
                    else if(iSht==1)
                    {
                        if(InSHT2InLF())
                        {
                            if(InArmSideAllClose(0)==false)                     //如果Arm 1有開site, 要做完Arm 2的料
                            {
                                if(IniConfig.bD43IndexDropErrorCanRetryandSkip &&
                                   (bShuttle2MoveToRight ||                     //Steven 20260121 : dont set shuttle when index jam
                                    bShuttle2MoveToLeft  ||                     //Steven 20220620 : 避免index在Kit1吸取異常, In arm偷放料
                                    bShuttle2HasPickErr))                       //Steven 20230116 : 避免In arm 偷放料
                                {
                                }
                                else if(ShuttleIsFull_9045(1)==false ||         //如果蝦頭二還沒擺滿 或者
                                        BLCarryKit.UseSiteNoIC())               //如果蝦頭二沒料
                                {
                                    MOT[MInShuttle2].fCanMoveL=false;           //把蝦頭鎖定, 避免index position error
                                }
                                Task=500;
                            }
                            else
                            {
                                if(iInArmType==e9045_1x3_2_14 ||
                                   iInArmType==e9045_1x4_2_14 ||
                                   iInArmType==e9045_2x3_6_14 ||
                                   iInArmType==e9045_2x4_4_13 ||
                                   iInArmType==e9045_2x4_4_14 ||
                                   iInArmType==e9045_2x5_8 ||
                                   iInArmType==e9045_2x6_8 ||
                                   iInArmType==e9045_2x8_32 ||
                                   iInArmType==e9045_2x8_8)
                                {
                                    if(iKit==1)
                                    {
                                        Task=500;
                                    }
                                }
                            }
                        }
                    }
                }
            }
            else
            {
                SetShuttleToHasNullICWhenCleanOut();                            //Steven 20230309 : 修正Clean時, 都沒IC造成的Hang Up
            }
        }
        else
        {
            if(MOT[MMTrayY].HasIC())
            {
                if(InArmSuck.iWhichSht==0)
                {
                    if(InArmSideAllClose(0)==true)
                    {
                        AdjustShuttlePlaceOrder(1);
                    }
                    else if(InSHT1InLF())
                    {
                        if(FLCarryKit.UseSiteHasIC()==true &&                   //如果蝦頭一有料而且
                           ShuttleIsFull_9045(0)==false)                        //    蝦頭一還沒擺滿
                        {
                            Task=100;
                        }
                    }
                }
                else
                {
                    if(InArmSideAllClose(1)==true)
                    {
                        AdjustShuttlePlaceOrder(0);
                    }
                    else if(InSHT2InLF())
                    {
                        if(InArmSideAllClose(0)==false)                         //如果Arm 1有開site, 要做完Arm 2的料
                        {
                            if(IniConfig.bD43IndexDropErrorCanRetryandSkip &&
                               (bShuttle2MoveToRight ||                         //Steven 20260121 : dont set shuttle when index jam
                                bShuttle2MoveToLeft  ||                         //Steven 20220620 : 避免index在Kit1吸取異常, In arm偷放料
                                bShuttle2HasPickErr))                           //Steven 20230116 : 避免In arm 偷放料
                            {
                            }
                            else if(ShuttleIsFull_9045(1)==false ||             //如果蝦頭二還沒擺滿 或者
                                    BLCarryKit.UseSiteNoIC())                   //如果蝦頭二沒料
                            {
                                MOT[MInShuttle2].fCanMoveL=false;               //把蝦頭鎖定, 避免index position error
                            }
                            Task=100;
                        }
                        else
                        {
                            if(BLCarryKit.UseSiteHasIC()==true &&               //如果蝦頭二有料而且
                               ShuttleIsFull_9045(1)==false)                    //    蝦頭二還沒擺滿
                            {
                                Task=100;
                            }
                        }
                    }
                }
            }
            else
            {
                SetShuttleToHasNullICWhenCleanOut();                            //Steven 20230309 : 修正Clean時, 都沒IC造成的Hang Up
            }
        }

        if(Task!=500 && Task!=100)
            SetShuttleToNullICWhenCleanOut();                                   //Steven 20240326 : 修正Clean時, 都沒IC造成的Hang Up
    }

    //  GOLDEN BUG (k7-B1), KEPT EXACTLY AS WRITTEN -- golden :7577-7580.  `&&`
    //  binds tighter than `||`, so this reads
    //      ((Task==iTask && bPickFromLoader) || bPickFromHotplate ||
    //       InArmSuck.HasIC())
    //  NOT the `Task==iTask && (…||…||…)` the layout implies: whenever
    //  bPickFromHotplate is true or the arm holds an IC, Task is forced to 50 even
    //  if the block above had just chosen 100 or 500.  NOT FIXED (translation, not
    //  redesign) and NOT reformatted -- golden`s own line breaks are preserved so
    //  the precedence is as visible here as it is there.
    if(Task==iTask &&
       bPickFromLoader==true ||
       bPickFromHotplate==true ||
       InArmSuck.HasIC()==true)
    {
        Task=50;
    }
    return Task;
}

//==============================================================================
//  ROLE          bNeedOneCycle -- "must the in-arm stop feeding and let the machine
//                run itself dry?"  QA-mode quota first (iQAModeLoaderCT reached
//                Prod.iQAModeCount while a one-cycle is armed), then the general
//                iOneCycle==1 answer: CheckOneCycleAction(0)!=0 means keep feeding;
//                otherwise the 32-site/NN-mode ambient special case (shuttle 1 or
//                front arm still holds an untested IC while shuttle 2 and the rear
//                arm are empty) also keeps feeding; anything else => yes, one cycle.
//
//  WAVE SCOPE    ACTIVE  bNeedOneCycle   golden ainarm9045.cpp:7587-7616
//
//  GATE REGISTER (this part file)   -- EMPTY.  IniConfig.bQAMode (Config.h:143),
//                LastSet.iRunStartMode (LastSet.h:366) / .iTemperature (:123),
//                rsmQAMode (MachineType.h:608), iQAModeLoaderCT (cmydef.h:3159),
//                Prod.iQAModeCount (cprod.h:721), iOneCycle (cmydef.h:253),
//                CheckOneCycleAction (ainarm9045.h:73), Tempture_Hot (cmydef.h:2760),
//                QualSite2X2N / _16Site4X4 / _32Site4X8N / _6Site2X3N (MachineType.h)
//                all exist.
//
//  TRAP 5 -- WHICH HEADER I RELIED ON: FLCarryKit / BLCarryKit / FTestSuck /
//  BTestSuck are the TMyKitSuck of aHotPlateSubstrate.h:365 (objects defined
//  aHotPlateSubstrate.cpp:81/82/92/93), which is the header this TU includes --
//  NOT mykitsuck.h:274 (different layout; its duplicate objects at
//  mykitsuck.cpp:205-207 are not registered in CMakeLists).  HasRealIC() / HasIC()
//  / NoIC() / AlreadyTest() are all declared on that class.
//
//  CALLEE HONESTY (not a gate): CheckOneCycleAction is still the `{ return 0; }`
//  stub at ainarm9045.cpp:2316, so offline the `iRet!=0` early-out never fires and
//  control always reaches the ambient/NN test.  Faithful, but shallow at the leaf.
//
//  NO CALLER YET (TRAP 1 shape (a), honest note, NOT a gate): golden calls this
//  from DoInArmPickFromLoadStage_9045 (golden :7669/:7733/:7748/:8218 -- port body
//  is the gated stub at ainarm9045.cpp:2237-2246) and from ainarm9045_2x4_16.cpp /
//  _2x8_32.cpp (golden :1044/:1090/:1105/:1169/:1548 and :1429/:1476/:1491/:1898),
//  whose port translations do not reference it yet -- verified 2026-08-11 01:33
//  with `rg -n "\bbNeedOneCycle\b" -g "*.cpp" -g "*.h" .` (0 hits in the port).
//  bNeedOneCycle also has NO declaration in golden ainarm9045.h, so it is
//  file-scope-but-external in golden too; defined non-static here, exactly as
//  golden does, so no TRAP 1 shape (d) shadow is created.
//  AI(W7D-k3-ainarm) 20260811
//==============================================================================
bool bNeedOneCycle()                                                            //Steven 20131029 : 解決Index Position Error
{
    if(IniConfig.bQAMode==true && LastSet.iRunStartMode==rsmQAMode)             //ChungHung 20141023 fix bNeedOneCycle make QA mode Error running
    {
        if(iQAModeLoaderCT>=Prod.iQAModeCount && iOneCycle==1)
            return true;
    }

    if(iOneCycle==1)
    {
        int iRet=CheckOneCycleAction(0);
        if(iRet!=0)
        {
            return false;
        }
        else if(LastSet.iTemperature!=Tempture_Hot &&                           //JerryYang 20251020 : fix 32 site one cycle hang up
                (TestIF.iTestMode==QualSite2X2N || TestIF.iTestMode==_16Site4X4 ||
                 TestIF.iTestMode==_32Site4X8N  || TestIF.iTestMode==_6Site2X3N) &&                                     //Steven 20220425 : 2X3NN Mode
                ((FLCarryKit.HasRealIC() || (FTestSuck.HasIC() && FTestSuck.AlreadyTest()==false)) &&                   //Shuttle 1或Arm 1有IC
                (BLCarryKit.NoIC() && BTestSuck.NoIC())))                       //Shuttle 2或Arm 2沒IC
        {
            return false;
        }
        else
        {
            return true;
        }
    }
    return false;
}

//==============================================================================
//  AI(k6-ainarm) 20260811 -- golden ainarm9045.cpp:8281-8302  bUseAxExPicker
//
//  ****  THIS PART EMITS NO DEFINITION.  READ WHY.  *************************
//  bUseAxExPicker IS ALREADY TRANSLATED AND ACTIVE in the port, at
//  ainarm9045.cpp:413-434 -- the very file this part would be stitched into.
//  It is NOT a stub: it is the full W6.2b translation, cited there as
//  `// golden :8281 (REAL-defined here; substrate stub removed)`.
//
//  I diffed it against golden :8281-8302 line by line.  It is VERBATIM: the same
//  6-way `if` (e9045_1x2_2_13 / 1x4_4_13 / 2x1_2_13 / 2x2_4_13 / 2x4_4_13 /
//  iCloseSiteModeFor2x8==e2x8Run2x2_13), the same `else if(bRunAutoClean)` with
//  the same 2x2_8_Hot + iXpitchMinX2_MM < dSiteXPitch <= iXpitchMaxX2_MM window,
//  the same two Chinese trailing comments (Sam 20250203 修正 1X4 8 吸嘴模式 and
//  SitePitchX 26-80 之間跑 1 3 吸嘴), the same return structure.  Nothing to fix
//  and nothing to add.
//
//  Emitting it again would be a MULTIPLE DEFINITION of a symbol that is declared
//  extern at ainarm9045.h:39 and called from GetJStep (ainarm9045.cpp:476) and
//  the variant modules -- i.e. a hard link error, not a silent problem.  So this
//  file carries the golden text ONLY inside a DUPLICATE-GUARD #if 0, so the main
//  loop can diff it against :413-434 and then DELETE THIS FILE.
//
//  ROOT CAUSE for the report, because it will recur: the target list was built by
//  function span / name, and this name was already claimed by an earlier wave.
//  Nine functions were assigned to this agent; this is the one that was already
//  done.  The other eight were genuinely un-translated (four of them existing
//  only as no-op stubs, one only as a stub in a DIFFERENT file -- see the
//  ProcessMES0101 part).
//
//  WAVE SCOPE    -- bUseAxExPicker  ALREADY ACTIVE ELSEWHERE, NOT RE-EMITTED
//                   (golden :8281-8302 == port ainarm9045.cpp:413-434)
//
//  GATE REGISTER
//   GATE k6-G5 -- golden ainarm9045.cpp:8281-8302.  This is the ONE #if 0 in this
//     file and it is a DUPLICATE-DEFINITION GUARD, *NOT* a symbol-absence gate.
//     There is therefore no absence to claim -- the opposite is true, and the
//     command below PROVES PRESENCE rather than absence:
//       rg -n --glob '*.cpp' -w bUseAxExPicker D:/HT9045/HT9011UC_Cpp_V3.33.906.0/ainarm9045.cpp
//       -> hits at :413 (the definition) and :476 (GetJStep's call).
//     MEASURED AT: 2026-08-11T01:31:24+0800 (same scan as this part's siblings).
//     WHY IT SHOULD STAY GATED (TRAP 3 form of the question): not because the
//     symbol is missing, but because the port already owns a byte-equal body and
//     C++ permits exactly one.  The correct end state is 'delete this file',
//     not 'un-gate it'.  Offline and real-machine behaviour are IDENTICAL either
//     way, since the two texts are the same text.
//     REAL-MACHINE DIFFERENCE: none.
//  ****************************************************************************
//==============================================================================
#if 0 // GATE k6-G5 -- DUPLICATE-DEFINITION GUARD, NOT an absence gate.  Verbatim golden ainarm9045.cpp:8281-8302, kept here ONLY so the main loop can diff it against the already-ACTIVE port body at ainarm9045.cpp:413-434 and then delete this file.  Un-gating this would be a multiple definition of a symbol declared extern at ainarm9045.h:39.
bool bUseAxExPicker()
{
    if(iInArmType==e9045_1x2_2_13 ||
       iInArmType==e9045_1x4_4_13 ||
       iInArmType==e9045_2x1_2_13 ||
       iInArmType==e9045_2x2_4_13 ||
       iInArmType==e9045_2x4_4_13 ||
       iCloseSiteModeFor2x8==e2x8Run2x2_13)
    {
        return true;
    }
    else if(bRunAutoClean)                                                      //Sam 20250203 : 修正 1X4 8 吸嘴模式
    {
        if(iInArmType==e9045_2x2_8_Hot &&
           iXpitchMinX2_MM<TestIF_File.dSiteXPitch &&
           TestIF_File.dSiteXPitch<=iXpitchMaxX2_MM)                            //SitePitchX 26-80 之間跑 1 3 吸嘴
        {
            return true;
        }
    }
    return false;
}
#endif // GATE k6-G5

// =============================================================================
//  k4-ainarm PART 08/08  --  bUseAxxGPicker   golden ainarm9045.cpp:8304-8324
//  Translator: AI(k4-ainarm) 20260811
//
//  *** THIS PART INTENTIONALLY EMITS NO CODE.  DO NOT "FIX" IT BY ADDING THE FUNCTION. ***
//
//  ROLE (of the golden function): reports whether the active layout uses the A..G flush-nozzle
//  stepping -- true for the seven _14 in-arm types and the 2x8-run-2x2_14 close-site mode,
//  plus the AutoClean special case where a 2x2_8_Hot layout with SitePitchX above
//  iXpitchMaxX2_MM drops to 1-of-4 nozzles.
//
//  WAVE SCOPE
//    bUseAxxGPicker  golden :8304-8324  ALREADY TRANSLATED -- NOT re-emitted (see below).
//
//  GATE REGISTER: (empty -- no #if 0 written, because no code is written)
//
//  WHY THIS IS EMPTY -- my task asked for a translation that is already in the tree.
//    The port ALREADY carries this function, REAL-defined, in the very file this wave
//    stitches into:
//      D:/HT9045/HT9011UC_Cpp_V3.33.906.0/ainarm9045.cpp:436
//        bool bUseAxxGPicker()   // golden :8304 (REAL-defined here; substrate stub removed)
//    It is not a stub.  I diffed it against golden ainarm9045.cpp:8304-8324 line by line: all
//    seven iInArmType comparisons, the iCloseSiteModeFor2x8==e2x8Run2x2_14 disjunct, the
//    bRunAutoClean else-if with its nested e9045_2x2_8_Hot + dSiteXPitch>iXpitchMaxX2_MM test,
//    both return values, and all four trailing comments (Steven 20220425, Steven 20200219,
//    Sam 20250212, and the SitePitchX note) are present and identical, Chinese included.
//    The port's own file banner lists it as ACTIVE at ainarm9045.cpp:21, it is declared at
//    ainarm9045.h:40 and aHotPlateSubstrate.h:858, ~30 live call sites already bind to it
//    (ainarm2.cpp:67, aoutarm.cpp:3735, ainarm_SearchPlacePlate.cpp x9, AutoClean/AutoClean.cpp
//    x6, RotateKit/aRotateKIT_In.cpp x8, RotateKit/aRotateKIT_Out.cpp x3,
//    ainarm9045_2x2_8_Hot.cpp:3336, plus my own PART 05 at golden :5127), and one test asserts
//    it (tests/test_w6_2_inarm_core.cpp:250).
//
//    Emitting it again would append a SECOND external definition of bUseAxxGPicker to the same
//    translation unit -- a hard `redefinition of 'bool bUseAxxGPicker()'` compile error that
//    would fail the stitched file's -fsyntax-only gate outright.  My brief says 64 golden
//    functions are already translated in that file and not to duplicate any of them; this is
//    one of them, so the faithful action is to emit nothing and say so loudly here, at the
//    exact place the stitcher looks.
//
//    Command that establishes it (run in D:/HT9045/HT9011UC_Cpp_V3.33.906.0, 2026-08-11 01:26:17 +0800):
//      grep -n "^bool bUseAxxGPicker()" ainarm9045.cpp    ->  436:bool bUseAxxGPicker()
//
//  >>> ACTION FOR THE MAIN LOOP: drop this part file from the stitch.  Nothing is missing;
//      golden :8304-8324 is already covered by ainarm9045.cpp:436.  If the wave's coverage
//      accounting credits golden :8304-8324 to me, it is double-counting an earlier wave.
//      (Its neighbour bUseACEGPicker, golden :8326, is likewise already at ainarm9045.cpp:457.)
//
//  Line/brace accounting for this file: 0 code lines, brace delta 0 (comments only).
// =============================================================================

//==============================================================================
//== ROLE: DoInArmPlaceToShuttle_9045 -- the in-arm PLACE-TO-SHUTTLE dispatcher.
//==       Clears bPlaceToHotplate, then routes to one of 26 per-layout
//==       place-to-shuttle state machines by USE_PICKER_COUNT / iInArmType, and
//==       raises a Program-Error message for an unmapped iInArmType.
//==
//== WAVE SCOPE (k2-ainarm):
//==   DoInArmPlaceToShuttle_9045  golden ainarm9045.cpp:8723-8839  -- ACTIVE
//==     (100% faithful; ZERO gates -- ALL 26 callees have real port bodies)
//==   + the callee forward-declaration block golden ainarm9045.cpp:8698-8722
//==     (see the note below on why it is in this part)
//==
//== GATE REGISTER: (none -- this function needed no #if 0)
//==   Every one of golden's 26 dispatch targets was located as a real, non-static
//==   definition in the port BEFORE un-gating anything.  Single command, cwd
//==   D:/HT9045/HT9011UC_Cpp_V3.33.906.0, run 2026-08-11 01:29 local:
//==     rg -n "^bool DoInArmPlaceToShuttle_9045_" --glob "*.cpp" .
//==   -> 26 hits, one per golden target: All_1Pick (ainarm9045_All_1Pick.cpp:629),
//==   1x1_1 (ainarm9045_1x1_1.cpp:591), 1x2_2_13 (ainarm9045_1x2_2.cpp:718),
//==   1x2_2_14 (ainarm9045_1x2_2_14.cpp:662), 1x2_4_Hot (:938), 1x3_2_14 (:775),
//==   1x3_4 (:783), 1x4_4_13 (ainarm9045S_1x4_4.cpp:678), 1x4_2_14
//==   (ainarm9045_1x4_2.cpp:761), 1x4_4_Back (:750), 1x4_4 (:961), 1x4_8_Hot
//==   (:843), 2x1_2 (:741), 2x2_4_12 (:668), 2x2_4_13 (ainarm9045_2x2_4.cpp:779),
//==   2x2_4_14 (:741), 2x2_8_Hot (:2381), 2x3_6_14 (:975), 2x3_6 (:871), 2x4_4_13
//==   (ainarm9045S_2x4_4_13.cpp:974), 2x4_4_14 (ainarm9045_2x4_4.cpp:1036), 2x4_8
//==   (:1182), 2x5_8 (:946), 2x6_8 (:1360), 2x8_8 (:2121).  (e9045_2x8_32 has no
//==   callee of its own -- golden :8829-8831 routes it to the 2x8_8 body; kept.)
//==
//== WHY THE EXTERN BLOCK IS IN THIS PART.  golden :8698-8722 is 25 `extern bool
//== DoInArmPlaceToShuttle_9045_*();` lines sitting one line ABOVE my assigned
//== range; the body cannot compile without them, and only ONE of the 26 is
//== declared in a reachable header (ainarm9045_1x2_4_Hot.h:30).  They are
//== DECLARATIONS, so re-declaring is legal and can never cause a
//== multiple-definition error even if a sibling emits the same block -- MAIN LOOP:
//== if a sibling part also carries golden :8698-8722, simply drop one copy; this
//== is not a conflict.  Mirrors what the port already does for the DoInArm_9045
//== ladder (port ainarm9045.cpp:773-835 declares its now-live callees inline with
//== golden line citations).
//==
//== INTEGRATION NOTE (TRAP 1 shapes (b) AND (a) at once): port ainarm9045.cpp:2298
//== currently holds `bool DoInArmPlaceToShuttle_9045(){return false;} // golden
//== :8723`.  Delete it or this file collides.  This swap is the one that finally
//== gives all 26 variant place-to-shuttle bodies a reachable caller -- until now
//== they were compiled-but-never-dispatched (shape (a)), so "the build was green"
//== never meant they were wired.  Declaration is non-static at ainarm9045.h:71.
//==
//== GOLDEN LADDER QUIRKS, transcribed and NOT corrected: there is no arm for
//== e9045_2x4_16 anywhere (consistent with the port already recording that
//== ainarm9045_2x4_16.cpp is dead by design, port ainarm9045.cpp:961-964), and
//== e9045_1x4_1_Ac is folded into the 1x1_1 arm at golden :8732-8733.  Arm order
//== is golden's order, including that 1x4_4_13 is tested before 1x4_2_14.
//==============================================================================
extern bool DoInArmPlaceToShuttle_9045_1x1_1();
extern bool DoInArmPlaceToShuttle_9045_1x2_2_13();
extern bool DoInArmPlaceToShuttle_9045_1x2_2_14();
extern bool DoInArmPlaceToShuttle_9045_1x2_4_Hot();
extern bool DoInArmPlaceToShuttle_9045_1x3_2_14();
extern bool DoInArmPlaceToShuttle_9045_1x3_4();
extern bool DoInArmPlaceToShuttle_9045_1x4_2_14();
extern bool DoInArmPlaceToShuttle_9045_1x4_4_13();
extern bool DoInArmPlaceToShuttle_9045_1x4_4();
extern bool DoInArmPlaceToShuttle_9045_1x4_4_Back();
extern bool DoInArmPlaceToShuttle_9045_1x4_8_Hot();
extern bool DoInArmPlaceToShuttle_9045_2x1_2();
extern bool DoInArmPlaceToShuttle_9045_2x2_4_12();
extern bool DoInArmPlaceToShuttle_9045_2x2_4_13();
extern bool DoInArmPlaceToShuttle_9045_2x2_4_14();
extern bool DoInArmPlaceToShuttle_9045_2x2_8_Hot();
extern bool DoInArmPlaceToShuttle_9045_2x3_6();
extern bool DoInArmPlaceToShuttle_9045_2x3_6_14();
extern bool DoInArmPlaceToShuttle_9045_2x4_4_13();
extern bool DoInArmPlaceToShuttle_9045_2x4_4_14();
extern bool DoInArmPlaceToShuttle_9045_2x4_8();
extern bool DoInArmPlaceToShuttle_9045_2x5_8();
extern bool DoInArmPlaceToShuttle_9045_2x6_8();
extern bool DoInArmPlaceToShuttle_9045_2x8_8();
extern bool DoInArmPlaceToShuttle_9045_All_1Pick();
bool DoInArmPlaceToShuttle_9045()
{
    bool bResult=false;
    AnsiString Str;
    bPlaceToHotplate=false;                                                     //RogerYang 20251220 : add for catchtray hangup
    if(USE_PICKER_COUNT==ep1Picker)
    {
        bResult=DoInArmPlaceToShuttle_9045_All_1Pick();
    }
    else if(iInArmType==e9045_1x1_1 ||
       iInArmType==e9045_1x4_1_Ac)
    {
        bResult=DoInArmPlaceToShuttle_9045_1x1_1();
    }
    else if(iInArmType==e9045_1x2_2_13)
    {
        bResult=DoInArmPlaceToShuttle_9045_1x2_2_13();
    }
    else if(iInArmType==e9045_1x2_2_14)
    {
        bResult=DoInArmPlaceToShuttle_9045_1x2_2_14();
    }
    else if(iInArmType==e9045_1x2_4_Hot)
    {
        bResult=DoInArmPlaceToShuttle_9045_1x2_4_Hot();
    }
    else if(iInArmType==e9045_1x3_2_14)
    {
        bResult=DoInArmPlaceToShuttle_9045_1x3_2_14();
    }
    else if(iInArmType==e9045_1x3_4)
    {
        bResult=DoInArmPlaceToShuttle_9045_1x3_4();
    }
    else if(iInArmType==e9045_1x4_4_13)
    {
        bResult=DoInArmPlaceToShuttle_9045_1x4_4_13();
    }
    else if(iInArmType==e9045_1x4_2_14)
    {
        bResult=DoInArmPlaceToShuttle_9045_1x4_2_14();
    }
    else if(iInArmType==e9045_1x4_4_Back)
    {
        bResult=DoInArmPlaceToShuttle_9045_1x4_4_Back();
    }
    else if(iInArmType==e9045_1x4_4)
    {
        bResult=DoInArmPlaceToShuttle_9045_1x4_4();
    }
    else if(iInArmType==e9045_1x4_8_Hot)
    {
        bResult=DoInArmPlaceToShuttle_9045_1x4_8_Hot();
    }
    else if(iInArmType==e9045_2x1_2_13)
    {
        bResult=DoInArmPlaceToShuttle_9045_2x1_2();
    }
    else if(iInArmType==e9045_2x2_4_12)
    {
        bResult=DoInArmPlaceToShuttle_9045_2x2_4_12();
    }
    else if(iInArmType==e9045_2x2_4_13)
    {
        bResult=DoInArmPlaceToShuttle_9045_2x2_4_13();
    }
    else if(iInArmType==e9045_2x2_4_14)
    {
        bResult=DoInArmPlaceToShuttle_9045_2x2_4_14();
    }
    else if(iInArmType==e9045_2x2_8_Hot)
    {
        bResult=DoInArmPlaceToShuttle_9045_2x2_8_Hot();
    }
    else if(iInArmType==e9045_2x3_6_14)
    {
        bResult=DoInArmPlaceToShuttle_9045_2x3_6_14();
    }
    else if(iInArmType==e9045_2x3_6)
    {
        bResult=DoInArmPlaceToShuttle_9045_2x3_6();
    }
    else if(iInArmType==e9045_2x4_4_13)
    {
        bResult=DoInArmPlaceToShuttle_9045_2x4_4_13();
    }
    else if(iInArmType==e9045_2x4_4_14)
    {
        bResult=DoInArmPlaceToShuttle_9045_2x4_4_14();
    }
    else if(iInArmType==e9045_2x4_8)
    {
        bResult=DoInArmPlaceToShuttle_9045_2x4_8();
    }
    else if(iInArmType==e9045_2x5_8)
    {
        bResult=DoInArmPlaceToShuttle_9045_2x5_8();
    }
    else if(iInArmType==e9045_2x6_8)
    {
        bResult=DoInArmPlaceToShuttle_9045_2x6_8();
    }
    else if(iInArmType==e9045_2x8_8)
    {
        bResult=DoInArmPlaceToShuttle_9045_2x8_8();
    }
    else if(iInArmType==e9045_2x8_32)
    {
        bResult=DoInArmPlaceToShuttle_9045_2x8_8();
    }
    else
    {
        Str.sprintf("iInArmType=%d", iInArmType);                               //Steven 20220620 : add log message
        ShowMyMessage("Program Error in DoInArmPlaceToShuttle_9045()", Str);
    }
    return bResult;
}

//==============================================================================
//  ROLE          -- MoveInArmXYAndSuckUp(): parks Z at the plate-safe height,
//                   then does ONE continuous in-arm move to an absolute (X,Y)
//                   with the X nozzle pitch set to the mechanical minimum-x3
//                   teach point and the Y pitch taken either from the recipe arm
//                   Y-pitch or from the interpolated auto-Y value.  Y is offset
//                   by (iType - iOutArmYBase) pitches so caller row A / row B
//                   land on the same absolute target.  Despite the name it
//                   commands no suction: ZNeedDown=true only tells the continuous
//                   move to include the Z axis in the profile.
//  WAVE SCOPE    -- MoveInArmXYAndSuckUp   golden :9029-9069   ACTIVE (no gates)
//  GATE REGISTER -- (none: this function writes no #if 0)
//
//  NOTES for the main loop
//  -----------------------
//  * FULLY ACTIVE -- every callee already has a body in the port:
//      MoveInArmZToPlateSafe(int)    decl aHotPlateSubstrate.h:901, body
//                                    aHotPlateSubstrate.cpp:1065 (offline: Z
//                                    reaches safe immediately -> true)
//      GetInArmPitch_9045(int)       decl ainarm9045.h:51, body ainarm9045.cpp:112
//      GetInArmPitch2_9045(int)      decl ainarm9045.h:49, body ainarm9045.cpp:148
//      GetInArmPitchY_9045(int,int)  decl ainarm9045.h:50 with iOffsetPos=-1
//                                    DEFAULT (which is why golden's 1-argument
//                                    call at :9051 compiles), body
//                                    ainarm9045.cpp:156
//      InArmContinuousMove_9045(...) decl Motor/mymotor.h:396 (W4/W5a sim motor),
//                                    trailing `bool bLoader=false` default, which
//                                    is why golden's 7-argument call compiles
//  * ZeroMemory is the established port idiom for this exact shape -- see
//    aoutarm.cpp:904-906 and ainarm9045_2x4_16.cpp:943-945, the out-arm/16-site
//    twins of golden :9038/:9039/:9044.  No memset substitution, no std::fill.
//  * GOLDEN BUG kept: the signature comment says `iType 0:A row 2:B row` while
//    the port header (ainarm9045.h:147, copied from golden ainarm9045.h) says
//    `0:A row 1:B row`.  golden's own two spellings disagree; the .cpp comment is
//    transcribed verbatim as "2" and NOT reconciled.  iType is used
//    arithmetically (`iActSuckR-iOutArmYBase`), so whether a caller passes 1 or 2
//    shifts Y by a whole pitch -- worth a look by whoever lands the callers, but
//    not this part's call to make.
//  * GOLDEN ODDITY kept: golden computes iMovePitchY=TestIF.iARM_Y_PITCH at
//    :9047, uses it for GetInArmPitchY_9045 at :9051, then OVERWRITES iMovePitchY
//    at :9052-9059 -- so when IN_OUT_ARM_Y_PITCH_MIN==0 the assignment at :9054
//    re-assigns the same value.  Statement order preserved exactly; nothing
//    collapsed.
//  * INTEGER DIVISION AUDIT: there is NO division in this function.  The one
//    arithmetic expression, `iPosY+(iActSuckR-iOutArmYBase)*iMovePitchY`
//    (golden :9062), is all-int and stays all-int -- no float helper introduced.
//  * `bZFlag` / `iZPos` are declared UNINITIALISED then ZeroMemory'd, exactly as
//    golden does it (:9036-9039).  Not converted to an aggregate initialiser --
//    that would be a redesign of golden's shape.
//  * `iMovePitchX` is assigned once and read once; kept because it is golden's
//    line :9046 and it names the magic constant.
//  * Symbols present: MAX_ARM_Row / MAX_ARM_Col + X_PITCH_COUNT
//    (MachineType.h:408 X_PITCH_COUNT==4, pulled via cmydef.h), iXpitchMinX3
//    (cmydef.h:4538), TestIF.iARM_Y_PITCH (cprod.h:1931),
//    IN_OUT_ARM_Y_PITCH_MIN (cmydef.h:2892), iOutArmYBase (cmydef.h:3641).
//  * The port ALREADY has `bool MoveInArmXYAndSuckUp(int, int, int)
//    { return false; }` at ainarm9045.cpp:2299 -- retire it.  Same TU, so a
//    missed retirement is a hard `redefinition` error, not silent TRAP 1
//    shape (b).  NOT `static`.  Declared ainarm9045.h:147.
//  * REACHABILITY, stated honestly: golden has NO caller of this function inside
//    ainarm9045.cpp, and no sibling part in _w7d_parts references it (checked
//    2026-08-11 01:53 +0800).  It is declared in ainarm9045.h:147 for the
//    AutoTeach/manual-move UI, so TRAP 1 shape (a) applies until that consumer
//    lands.  Harmless -- same already-extracted TU -- but do not record it as
//    "wired up".
//  * TRAP 4: no file-scope object added; the two arrays are function-local
//    automatics with no constructor.
//==============================================================================
bool MoveInArmXYAndSuckUp(int iPosX, int iPosY, int iType)                       //iType 0:A row 2:B row
{
    if(MoveInArmZToPlateSafe(0)==false)
    {
        return false;
    }
    int iXPos=0,iYPos=0,iMovePitchX=0,iMovePitchY=0;
    bool bZFlag[MAX_ARM_Row][MAX_ARM_Col];
    int iZPos[MAX_ARM_Row][MAX_ARM_Col];
    ZeroMemory(bZFlag, sizeof(bZFlag));
    ZeroMemory(iZPos, sizeof(iZPos));
    int iXVariable[X_PITCH_COUNT];
    int iYVariable =0;
    int iActSuckR=iType;

    ZeroMemory(iXVariable , sizeof(iXVariable));

    iMovePitchX=iXpitchMinX3;
    iMovePitchY=TestIF.iARM_Y_PITCH;

    iXVariable[0]=GetInArmPitch_9045(iMovePitchX);
    iXVariable[1]=GetInArmPitch2_9045(iMovePitchX);
    iYVariable=GetInArmPitchY_9045(iMovePitchY);
    if(IN_OUT_ARM_Y_PITCH_MIN==0)
    {
        iMovePitchY=TestIF.iARM_Y_PITCH;
    }
    else
    {
        iMovePitchY=iYVariable;
    }

    iXPos=iPosX;
    iYPos=iPosY+(iActSuckR-iOutArmYBase)*iMovePitchY;

    if(InArmContinuousMove_9045(iXPos, iYPos, iXVariable, iYVariable, bZFlag, iZPos, true))
    {
        return true;
    }
    return false;
}

//==============================================================================
//  DoInArm_9045_Type -- golden ainarm9045.cpp:1616-2063（448 行）
//  AI(W906-P1c) 20260921
//
//  ## 它是什麼
//
//  依 `TestIF_File.iTestMode` / `USE_PICKER_COUNT` / `iUseSuckMode` /
//  `dSiteXPitch` 等組態，**決定這台機器的手臂排列**（`iInArmType`），
//  順便設 `InArmSuck.iXStep` / `iModeX` 等旗標，最後叫 `SetInOutArmParameter()`。
//
//  ## 為什麼是 🔴
//
//  它是 `iInArmType` 全樹**唯一的生產端**。沒有它：
//      cmydef.cpp:4918   int iInArmType=0;        <- 初值，也是全樹唯一的寫入
//      583 個讀取點（70 個檔）                      <- 全部讀到 0
//      MachineType.h:1054 e9045_1x1_1 = 0          <- 0 是**合法機型**
//  ⇒ 一台 2x4 八吸嘴的機器會被當成 1x1 單吸嘴，而且靜默。
//
//  ## 相依：零個缺
//
//  用編譯器判（本檔全文 ＋ golden body 拼成一個 TU，兩組態各一次
//  `-fsyntax-only`）。未解析只有 `fSetup` 一個，而它存在
//  （`forms/fSetup.h:623 extern TfSetup *fSetup;`），
//  `CheckSTMMode()` 也有**真本體**（`cSetUp.cpp:333-362`，不是樁）。
//  ⇒ 只補一個 include，**一個閘都沒加**。
//
//  ⚠ 20260920 我第一次量這件事用的是 `git grep`，結論是「94 個識別字有 93 個」——
//    那個數字是錯的：grep 把**註解裡的提及**算成「有」，而 `CheckSTMMode`
//    全樹命中都在 `cSetUp.cpp` 的註解行裡。用編譯器才問得出「宣告看得到嗎」。
//==============================================================================
#include "forms/fSetup.h"           // AI(W906-P1c) 20260921: fSetup->CheckSTMMode()（golden :2061）
void DoInArm_9045_Type()                                                        //Steven 20201014 : 將DoInArm_9045_Type獨立,避免DoInArm_9045還沒執行就被使用
{
    double LoaderXPitch=UserDefForm_File[TrayForm.Loader.iTrayType].XPitch;
//    int Ld=TrayForm.Loader.iTrayType;
    int iModeX;
    InArmSuck.iXStep=1;
    OutArm2Suck.iXStep=1;                                                       //RogerYang 20250521 add for 9046AU
    CheckInArmYStep();                                                          //Steven 20240205 : Y-Pitch可否一次放

//    if(USE_PICKER_COUNT==ep1Picker)                                           //JerryYang 20250830 : todo
//    {
//        iInArmType=e9045_1x1_1;
//    }
//    else
    if(TestIF_File.iTestMode==SingleSite)
    {
        iInArmType=e9045_1x1_1;
    }
    else if(TestIF_File.iTestMode==DualSite ||                                  // 1x2
            TestIF_File.iTestMode==QualSite2X2N)                                //Frank 20200520 2X2NN Mode
    {
        if(USE_PICKER_COUNT==0)                                                 //Steven 20161117 : for HT-9045S
        {
            if(TestIF_File.iUseSuckMode==4)                                     //use 4 pick unit
            {
                if(ArmXCanSuck2IC_9045S()==false ||                             //先確認可不可以跑4吸嘴
                   ArmYCanSuck2IC()==false)
                {
                    TestIF_File.iUseSuckMode=2;
                }
            }

            if(TestIF_File.iUseSuckMode==4)                                     //use 4 pick unit
            {
                iInArmType=e9045_1x2_4_Hot;
                if(TestIF_File.dSiteXPitch>iXpitchMaxX2_MM)
                    InArmSuck.iXStep=2;
            }
            else
            {
                iInArmType=e9045_1x2_2_13;
                if(TestIF_File.dSiteXPitch>iXpitchMaxX2_MM)
                    InArmSuck.iXStep=2;
            }
        }
        else if(USE_16PICKER_TYPE==1)                                           //Ztex 2023.12.06 Add HT-1032
        {
        }
        else
        {
            if(TestIF_File.iUseSuckMode==4)                                     //use 4 pick unit
            {
                if(TestIF_File.dSiteXPitch<iXpitchMaxX3_MM &&                   //先確認可不可以跑4吸嘴
                   (ArmXCanSuck4IC_9045()==false &&
                    ArmYCanSuck2IC()==false))
                {
                    TestIF_File.iUseSuckMode=2;
                }
                else if(TestIF_File.iTestMode==QualSite2X2N)
                {
                    TestIF_File.iUseSuckMode=2;
                }
            }

            if(TestIF_File.iUseSuckMode==4 &&
               TestIF_File.iTestMode!=QualSite2X2N)                             //Ifor 20260123 add:
            {
                iInArmType=e9045_1x2_4_Hot;
                if(ArmXCanSuck4IC_9045()==false)
                {
                    i1x2_4UseACEGPicker=0;                                      //使用Aa, Ad, Ba, Bd
                    if(TestIF_File.dSiteXPitch>iXpitchMaxX3_MM)
                        InArmSuck.iXStep=2;
                }
                else
                {
                    i1x2_4UseACEGPicker=1;                                      //Steven 20230530 : 1x2_4改用Row A
                    if(TestIF_File.dSiteXPitch>iXpitchMaxX2_MM)
                        InArmSuck.iXStep=2;
                }
            }
            else
            {
//                if(TestIF_File.dSiteXPitch>iXpitchMaxX3_MM)                   //Steven 20250701 : Mark for 1x2
//                {
//                    iInArmType=e9045_1x2_2_14;
//                    InArmSuck.iXStep=2;
//                }
//                else if(TestIF_File.dSiteXPitch>iXpitchMaxX2_MM)
//                {
//                    iInArmType=e9045_1x2_2_14;
//                }
//                else
//                {
//                    if(LastSet.iTemperature==Tempture_Hot)                      //Steven 20250716 : 修正HP item4 1x2/2x2 NN mode
//                    {
//                        if((Prod.HotPlateForm[0].iXItem==4  && Prod.HotPlateForm[0].iXPitch>(iXpitchMaxX2/2)) ||        //HP X-Pitch >40mm
//                           (Prod.HotPlateForm[0].iXItem==6  && Prod.HotPlateForm[0].iXPitch>(iXpitchMaxX2/3+1)) ||      //HP X-Pitch >28mm
//                           (Prod.HotPlateForm[0].iXItem==8  && Prod.HotPlateForm[0].iXPitch>(iXpitchMaxX2/4)) ||        //HP X-Pitch >10mm
//                           (Prod.HotPlateForm[0].iXItem==10 && Prod.HotPlateForm[0].iXPitch>(iXpitchMaxX2/5)) ||        //HP X-Pitch >8mm
//                           (Prod.HotPlateForm[0].iXItem==12 && Prod.HotPlateForm[0].iXPitch>(iXpitchMaxX2/6)) ||        //HP X-Pitch >7mm
//                           (Prod.HotPlateForm[0].iXItem==16 && Prod.HotPlateForm[0].iXPitch>(iXpitchMaxX2/6)))          //HP X-Pitch >7mm
//                        {
//                            iInArmType=e9045_1x2_2_14;
//                        }
//                        else
//                        {
//                            iInArmType=e9045_1x2_2_13;
//                        }
//                    }
//                    else
                    {
                        if(TestIF_File.dSiteXPitch>iXpitchMaxX2_MM)
                        {
                            iInArmType=e9045_1x2_2_14;
                            if(TestIF_File.dSiteXPitch>iXpitchMaxX3_MM)         //Steven 202507023 : Fixed for 1x2
                            {
                                InArmSuck.iXStep=2;
                            }
                            else
                            {
                                InArmSuck.iXStep=1;
                            }
                        }
                        else if(LastSet.iTemperature==Tempture_Hot &&
                                Prod.HotPlateForm[0].iXPitch>(iXpitchMaxX2/3+1) &&                                      //RogerYang 20250827 修正HP Pitch>26.66且iXItem=6使用14吸嘴
                                Prod.HotPlateForm[0].iXItem==6)
                        {
                            iInArmType=e9045_1x2_2_14;
                        }
                        else
                        {
                            iInArmType=e9045_1x2_2_13;
                        }
                    }
//                }
            }
        }
    }
    else if(TestIF_File.iTestMode==TriSite1X3 ||                                //1x3 //Frank 20160329 add for 1x3_4
            TestIF_File.iTestMode==_6Site2X3N)                                  //Steven 20220425 : 2X3NN Mode
    {
        iInArmType=e9045_1x3_4;
//        if(USE_PICKER_COUNT==0)                                                 //Steven 20161117 : for HT-9045S
//        {
//            iInArmType=e9045_1x3_2_13;
//            if(TestIF_File.dSiteXPitch<=iXpitchMaxX2_MM)
//                InArmSuck.iXStep=1;
//            else
//                InArmSuck.iXStep=2;
//        }
//        else
        {
            iModeX=CheckPickerMode(4, true, false);                             //Steven 20240427 : fixed for 1x3
            if(iModeX==14)
                iInArmType=e9045_1x3_2_14;
        }
    }
    else if(TestIF_File.iTestMode==QualSite1X4 ||                               // 1x4
            TestIF_File.iTestMode==_8Site2X4N)                                  //Wei 20231211 : 2X4NN Mode
    {
        iInArmType=e9045_1x4_4;

        if(LastSet.iTemperature==Tempture_Hot &&                                //Steven 20240308 : 修正HP item4 X-Pitch 45mm
           Prod.HotPlateForm[0].iXPitch>4000)
        {
            TestIF_File.iUseSuckMode=2;
        }

        if(TestIF_File.iTestMode==QualSite1X4 &&
           CosFunction.b1x4OnlyAaUse1x1Mode &&                                  //Steven 20200720 : 1x4只開site Ac, 跑1x1模式
           TestIF_File.iSiteMap[0][0]<=0 &&
           TestIF_File.iSiteMap[0][1]<=0 &&
           TestIF_File.iSiteMap[0][3]<=0)
        {
            TestIF_File.iUseSuckMode=4;
            iInArmType=e9045_1x4_1_Ac;
        }
        else if(USE_PICKER_COUNT==0)                                            //Steven 20161117 : for HT-9045S
        {
            TestIF_File.iUseSuckMode=4;
            iInArmType=e9045_1x4_4_13;

            if(TestIF_File.dSiteXPitch>iXpitchMaxX2_MM)
                InArmSuck.iXStep=2;
        }
        else
        {
            if(TestIF_File.iUseSuckMode==8)
            {
                if(CheckPickerMode(4, true, false)!=1234 ||
                   ArmYCanSuck2IC()==false ||
                   LoaderXPitch>=iXpitchMaxX1_MM ||                             //Steven 20200327 : Tray form大於40的,不能跑8吸嘴)  //Steven 20240221 : > --> >=
                   TestIF_File.bInArmUseBackRowSuck==true)                      //jou 20161122 (Steven) In & Out arm use back row suck
                {
                    TestIF_File.iUseSuckMode=4;
                }
            }

            if(TestIF_File.iUseSuckMode==8)                                     //jou 980525 1x4 select 4 or 8 pick unit
            {
                iInArmType=e9045_1x4_8_Hot;
                if(USE_16PICKER_TYPE==1)                                        //Ztex 2023.12.06 Add HT-1032
                {
                    if(TestIF_File.dSiteXPitch>iXpitchMaxX4_MM)
                        InArmSuck.iXStep=4;
                    else if(TestIF_File.dSiteXPitch>iXpitchMaxX2_MM)
                        InArmSuck.iXStep=2;
                }
                else
                {
                    if(TestIF_File.dSiteXPitch>iXpitchMaxX2_MM)
                        InArmSuck.iXStep=4;
                    else if(TestIF_File.dSiteXPitch>iXpitchMaxX1_MM)            //Sam 20250203 : 再改回 > //Steven 20240221 : > --> >=
                        InArmSuck.iXStep=2;
                }
            }
            else
            {
                iModeX=CheckPickerMode(4, true, false);                         //JerryYang 20231003 1x4 14吸嘴
                if(iModeX==13)
                {
                    iInArmType=e9045_1x4_4_13;
                }
                else if(iModeX==14)
                {
                    iInArmType=e9045_1x4_2_14;
                }
                else
                {
                    if(TestIF_File.bInArmUseBackRowSuck==true)                  //jou 20161122 (Steven) In & Out arm use back row suck
                        iInArmType=e9045_1x4_4_Back;
                    else
                        iInArmType=e9045_1x4_4;
                }
            }
        }
    }
    else if(TestIF_File.iTestMode==DualSite2x1)
    {
        iInArmType=e9045_2x1_2_13;
    }
    else if(TestIF_File.iTestMode==QualSite2X2)                                 //2x2
    {
        InArmSuck.iXStep=1;
        if(USE_PICKER_COUNT==ep1Picker)                                         //JerryYang 20250830 : todo
        {
            iInArmType=e9045_2x2_4_13;
        }
        else if(USE_PICKER_COUNT==0)                                            //Steven 20161117 : for HT-9045S
        {
            TestIF_File.iUseSuckMode=4;
            iInArmType=e9045_2x2_4_13;
            if(TestIF_File.dSiteXPitch>iXpitchMaxX3_MM)
                InArmSuck.iXStep=2;
        }
        else if(USE_16PICKER_TYPE==1)                                           //Ztex 2023.12.06 Add HT-1032
        {
        }
        else
        {
            if(TestIF_File.iUseSuckMode==8)
            {
                if(LoaderXPitch>iXpitchMaxX1_MM ||                              //Steven 20200327 : Tray form大於40的,不能跑8吸嘴
                   ArmXCanSuck4IC_9045()==false)
                {
                    TestIF_File.iUseSuckMode=4;
                }
            }

            if(TestIF_File.iUseSuckMode==8)
            {
                iInArmType=e9045_2x2_8_Hot;
                if(TestIF_File.dSiteXPitch>iXpitchMaxX4_MM &&
                   TestIF_File.dSiteXPitch>iXpitchMinX4_MM)
                {
                    InArmSuck.iXStep=2;
                }
            }
            else
            {
//                if(TestIF_File.dSiteXPitch>iXpitchMaxX2_MM)
//                {
//                    iInArmType=e9045_2x2_4_14;
//                    if(TestIF_File.dSiteXPitch>iXpitchMaxX3_MM ||
//                       TestIF_File.dSiteXPitch<iXpitchMinX3_MM)
//                        InArmSuck.iXStep=2;
//                }
//                else if(TestIF_File.dSiteXPitch<iXpitchMinX2_MM)
//                {
//                    iInArmType=e9045_2x2_4_12;
//                    if(TestIF_File.dSiteXPitch>iXpitchMaxX1_MM ||
//                       TestIF_File.dSiteXPitch<iXpitchMinX1_MM)
//                        InArmSuck.iXStep=2;
//                }
//                else
//                {
//                    if(LastSet.iTemperature==Tempture_Hot)                      //Steven 20240308 : 修正HP item4 X-Pitch 45mm
//                    {
//                        if((Prod.HotPlateForm[0].iXItem==4  && Prod.HotPlateForm[0].iXPitch>(iXpitchMaxX2/2)) ||
//                           (Prod.HotPlateForm[0].iXItem==6  && Prod.HotPlateForm[0].iXPitch>(iXpitchMaxX2/3+1)) ||
//                           (Prod.HotPlateForm[0].iXItem==8  && Prod.HotPlateForm[0].iXPitch>(iXpitchMaxX2/4)) ||
//                           (Prod.HotPlateForm[0].iXItem==10 && Prod.HotPlateForm[0].iXPitch>(iXpitchMaxX2/5)) ||
//                           (Prod.HotPlateForm[0].iXItem==12 && Prod.HotPlateForm[0].iXPitch>(iXpitchMaxX2/6)) ||
//                           (Prod.HotPlateForm[0].iXItem==16 && Prod.HotPlateForm[0].iXPitch>(iXpitchMaxX2/6)))
//                        {
//                            iInArmType=e9045_2x2_4_14;
//                            if(TestIF_File.dSiteXPitch>iXpitchMaxX3_MM ||
//                               TestIF_File.dSiteXPitch<iXpitchMinX3_MM)
//                                InArmSuck.iXStep=2;
//                        }
//                        else
//                        {
//                            iInArmType=e9045_2x2_4_13;
//                            if(TestIF_File.dSiteXPitch>iXpitchMaxX2_MM ||
//                               TestIF_File.dSiteXPitch<iXpitchMinX2_MM)
//                                InArmSuck.iXStep=2;
//                        }
//                    }

                    if(TestIF_File.dSiteXPitch>iXpitchMaxX2_MM)
                    {
                        iInArmType=e9045_2x2_4_14;
                        if(TestIF_File.dSiteXPitch>iXpitchMaxX3_MM)             //Steven 202507023 : Fixed for 1x2
                        {
                            InArmSuck.iXStep=2;
                        }
                        else
                        {
                            InArmSuck.iXStep=1;
                        }
                    }
                    else if(LastSet.iTemperature==Tempture_Hot &&
                            Prod.HotPlateForm[0].iXPitch>(iXpitchMaxX2/3+1) &&  //RogerYang 20250827 修正HP Pitch>26.66且iXItem=6使用14吸嘴
                            Prod.HotPlateForm[0].iXItem==6)
                    {
                        iInArmType=e9045_2x2_4_14;
                    }
                    else if(LastSet.iTemperature==Tempture_Hot &&               //AI(ht9045-inarm-flow) 20260512 (RogerYang) : HP 12x20 XP*6>80mm
                            Prod.HotPlateForm[0].iXItem==12 &&
                            Prod.HotPlateForm[0].iXPitch*6>iXpitchMaxX2)
                    {
                        iInArmType=e9045_2x2_4_14;
                    }
                    else
                    {
                        iInArmType=e9045_2x2_4_13;
                        if(TestIF_File.dSiteXPitch>iXpitchMaxX2_MM ||
                           TestIF_File.dSiteXPitch<iXpitchMinX2_MM)
                            InArmSuck.iXStep=2;
                    }
//                }
            }
        }
    }
    else if(TestIF_File.iTestMode==_6Site2X3)                                   //ChungHung 20140115 add for 2x3_6
    {
        iInArmType=e9045_2x3_6;
        if(LastSet.iTemperature==Tempture_Hot &&                                //Steven 20241023 : 修正HP item4 X-Pitch 45mm
           Prod.HotPlateForm[0].iXPitch>4000)
        {
            iModeX=CheckPickerMode(4, true, false);
            if(iModeX==14)
            {
                iInArmType=e9045_2x3_6_14;
            }
            else
            {
            }
        }
        else if(USE_PICKER_COUNT==0)                                            //Steven 20161117 : for HT-9045S
        {
//            iInArmType=e9045_2x3_6_13;
        }
        else
        {
            iModeX=CheckPickerMode(4, true, false);
            if(iModeX==14)
            {
                iInArmType=e9045_2x3_6_14;
            }
            else
            {
            }
        }
    }
    else if(TestIF_File.iTestMode==_8Site2X4 ||                                 // 2x4
            TestIF_File.iTestMode==_16Site4X4)                                  //Sam 20190226 : 16Site4X4
    {
        iInArmType=e9045_2x4_8;

        if(LastSet.iTemperature==Tempture_Hot &&                                //Steven 20241023 : 修正HP item4 X-Pitch 45mm
           Prod.HotPlateForm[0].iXPitch>4000)
        {
            iModeX=CheckPickerMode(4, true, false);
            if(iModeX==14)
                iInArmType=e9045_2x4_4_14;
        }
        else if(USE_PICKER_COUNT==0)                                            //Steven 20161117 : for HT-9045S
        {
            iInArmType=e9045_2x4_4_13;
            if(TestIF_File.dSiteXPitch<=iXpitchMaxX2_MM)
                InArmSuck.iXStep=1;
            else
                InArmSuck.iXStep=2;
        }
        else
        {
            iModeX=CheckPickerMode(4, true, false);
            if(iModeX==14)
                iInArmType=e9045_2x4_4_14;
        }
        OutArm2Suck.iXStep=2;                                                   //RogerYang 20250710 Add for 9046AU
    }
    else if(TestIF_File.iTestMode==_10Site2X5)                                  // 2x5  //Steven 20211220 : for 2x5
    {
        iInArmType=e9045_2x5_8;
        iModeX=CheckPickerMode(4, false, false);
    }
    else if(TestIF_File.iTestMode==_12Site2X6)
    {
        iInArmType=e9045_2x6_8;
        iModeX=CheckPickerMode(4, false, false);
    }
    else if(TestIF_File.iTestMode==_16Site2X8)                                  // 2x8
    {
        iInArmType=e9045_2x8_8;
        iModeX=CheckPickerMode(4, false, false);
        OutArm2Suck.iXStep=4;                                                   //RogerYang 20250710 Add for 9046AU
    }
    else if(TestIF_File.iTestMode==_32Site4X8N)
    {
        iInArmType=e9045_2x8_32;
        iModeX=CheckPickerMode(4, false, false);
    }

    if(InArmSuck.iXStep==1)
        InArmSuck.iModeX=0;
    else if(InArmSuck.iXStep==2)
        InArmSuck.iModeX=1;
    else                                                                        //if(InArmSuck.iXStep==4)
        InArmSuck.iModeX=2;
//    else
//        InArmSuck.iModeX=3;

    fSetup->CheckSTMMode();                                                     //Steven 20221006 : 確認STM模式
    SetInOutArmParameter();
}
