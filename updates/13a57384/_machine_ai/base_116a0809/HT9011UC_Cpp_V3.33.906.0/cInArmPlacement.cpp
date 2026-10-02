// =============================================================================
//  cInArmPlacement.cpp  --  in-arm "pickup error IC" auto-recycle/placement
//                           state machine (JimmyChiu 20220908 feature).
//
//  Faithful translation of golden cInArmPlacement.cpp (577 lines, BCB6,
//  Big5/cp950).
//  Translator: AI(W906-PT-W3) 20260807
//  Translation wave: W906-PT-W3, group "misc_a".
//
//  ROLE: cInArmPlacement is a small helper object the in-arm engine holds to
//  automatically recycle an IC that was left behind on the in-shuttle (a
//  detected sensor-error IC) or a Loader cell explicitly marked HAS_SKIP_IC.
//  It picks the offending IC with the in-arm's dedicated placement nozzle
//  (InArmPlaceSuck.Suck[0][0]) and cylinder (Cylinder[C_TrayX_UpDown]) and
//  parks it at a fixed "Placement" position (Prod.iInPlacementX/Y) so the
//  main pick cycle is not blocked.  Two independent state-machine ladders
//  (one for the in-shuttle, one for the Loader) share the same low-level
//  move/suck primitives (MoveInArmXY / DoInArmPlaceDown / DoInArmPlaceSuck /
//  CheckInArmPlaceSuckHasIC / DoPlaceSuckToPlacement).
//
//  WAVE SCOPE -- ACTIVE vs satisfied-by-substrate
//  -----------------------------------------------
//   ACTIVE (faithful, verbatim -- all 21 golden functions, whole file):
//     cInArmPlacement::cInArmPlacement          golden :19-22
//     cInArmPlacement::~cInArmPlacement         golden :24-27
//     InitialDatas                              golden :29-31   (empty in golden -- kept empty)
//     InitialShuttleDatas                       golden :33-44
//     DoSuckToRecycleFromShuttle                golden :46-89
//     DoCheckShuttleSensor                      golden :91-110
//     DoSuckFromShuttleToPlace                  golden :112-173
//     DoSuckFromShuttle                         golden :175-238
//     GetShuttlePos                             golden :240-276
//     GetPlacementPos                           golden :278-282
//     MoveInArmXY                               golden :284-299
//     DoInArmPlaceDown                          golden :301-317
//     DoInArmPlaceSuck                          golden :319-329
//     CheckInArmPlaceSuckHasIC                  golden :331-341
//     InArmPlacementEnable                      golden :343-346
//     DoPlaceSuckToPlacement                    golden :348-394
//     DoCheckLoaderHasErrorIC                   golden :396-410
//     DoSuckToRecycleFromLoader                 golden :412-454
//     DoSuckFromLoaderToPlace                   golden :456-497
//     DoSuckFromLoader                          golden :499-563
//     GetLoaderPos                              golden :565-576
//   SATISFIED-BY-SUBSTRATE (already real in this tree, nothing shimmed here):
//     Sen[]/TMySensor::Enable+IsOn (mysensor.h/.cpp), MOT[]/TTrayMotor
//     (Motor/mymotor.h, incl. InArmContinuousMove_9045 -- a W4 sim stub that
//     always returns false offline, mymotor.cpp:1356), Cylinder[]/TMyCylinder
//     ::On/Off/OnStatus (mycylin.h), TQPF_Timer (myTimer.h), ShowErrorMessage/
//     K_SKIP/K_RETRY (canary_support.h -- golden note.h), Prod/TestIF
//     (cprod.h), NULL_IC/HAS_SKIP_IC/C_TrayX_UpDown/MMTrayY/MInArmX/
//     iInArmXBase/iInArmYBase/iInArmShtXCenterPos/iInArmShtYCenterPos/
//     USE_InPlacement (cmydef.h), eartInstall (MachineType.h),
//     IniConfig.bE69_PickupErrorPlacement (Config.h), SThreadPara
//     (cprod.h, SHUTTLE_THREAD::iInShSenIndex), TransferInShuttleRatio
//     (aHotPlateSubstrate.h/.cpp -- an intentional no-op shim for the SAME
//     golden gear-ratio function family as GATE (1) below; golden
//     ainarm2.cpp:1972-2110, decl aHotPlateSubstrate.h:1065).
//
//  GATE REGISTER -- exactly TWO gates.
//  ---------------------------------------------------------------------------
//   (1) TransferLoaderRatio(int*,int*)  -- golden ainarm2.cpp:2112-2160+
//       (Loader software gear-ratio: applies AOA auto-teach XY scale
//       correction, or -- in Hot mode with IniConfig.bE30_1InArmUseDifferent
//       ScaleHot enabled -- a per-axis differential scale around
//       Prod.XInArm_Tray_Pick/YInArm_Tray_Pick using LastSet.fLoaderTrayX
//       Scale_Hot/fLoaderTrayYScale_Hot).  Grepped the WHOLE port tree
//       (every .cpp/.h outside build*/) on 20260807: NO compiled body
//       anywhere.  The one other call site, ainarm9045_2x8_32.cpp:1217, has
//       the SAME unaddressed gap (an ACTIVE, ungated call to a symbol with no
//       body anywhere -- that file predates this wave and is out of this
//       wave's write scope to fix).
//       Handled with the tree's #if 0/#else macro-pair idiom: golden's call
//       stays VERBATIM in the #if 0 arm inside GetLoaderPos.
//       WHY NO-OP IS THE FAITHFUL DEFAULT: this mirrors the sibling shim
//       TransferInShuttleRatio (aHotPlateSubstrate.cpp:1502), which is
//       likewise an intentional empty body for the exact same class of
//       gear-ratio/scale correction, pending a future wave that ports
//       ainarm2.cpp itself.
//       BEHAVIOUR DELTA, STATED PLAINLY: on a real machine with AOA
//       auto-alignment enabled for the Loader, or in Hot mode with the E30_1
//       differential-scale option enabled, GetLoaderPos returns an
//       uncorrected raw coordinate; the positioning error this introduces is
//       bounded by how far the configured scale factor departs from 1.0
//       (normally a small, sub-pitch correction).  Retire this gate the
//       moment golden ainarm2.cpp's TransferLoaderRatio lands with a real
//       body in this tree.
//   (2) NONE -- no second call-level gate.  (InArmPlaceSuck is not a gate: it
//       is a real, defined TMyKitSuck object -- see the note immediately
//       below.)
//
//  A GLOBAL THIS FILE DEFINES (NOT A GATE): InArmPlaceSuck
//  ---------------------------------------------------------------------------
//  Golden declares `extern TMyKitSuck InArmPlaceSuck;` in MyKitSuck.h and
//  defines the object in MyKitSuck.cpp (golden mykitsuck.cpp is a DIFFERENT
//  golden unit, not part of this wave's group).  This tree's established
//  substrate for TMyKitSuck is aHotPlateSubstrate.h/.cpp (the "MINIMAL
//  mirror" that InArmSuck/OutArmSuck/TestSocket/... already use -- see e.g.
//  aHotPlateSubstrate.h:624/627/635).  A SEPARATE, PARALLEL full translation
//  of mykitsuck.h/.cpp exists in this same wave under a different group
//  ("kitsuck") but is explicitly NOT wired into any translation unit yet
//  (its own banner: "NOT #included by anything yet... would be a hard
//  redefinition error" if combined with aHotPlateSubstrate.h) -- so this file
//  does NOT include it, to avoid exactly that redefinition risk.  Instead,
//  cInArmPlacement.h/.cpp is the sole ported consumer of InArmPlaceSuck this
//  wave, so -- following the same precedent aHotPlateSubstrate.cpp itself set
//  for InArmSuck/OutArmSuck (also golden MyKitSuck.cpp globals, ported into a
//  different TU than their golden home) -- the object is declared `extern`
//  in cInArmPlacement.h and DEFINED here, using the tree's real, substrate
//  TMyKitSuck class (aHotPlateSubstrate.h).  Only Suck[0][0] (On/Off/Normal/
//  GetStatus) is ever touched by this file; TMyKitSuck's own default
//  constructor already default-constructs every TMySucker cell safely.
//
//  VCL/Borland conversions: #pragma hdrstop + #pragma package(smart_init)
//  dropped (golden :2 and :15).  golden :17 `const int iArmDownDelayTime=0.1;`
//  is KEPT VERBATIM, INCLUDING its int-truncates-0.1-to-0 quirk (a real golden
//  bug: the "0.1 second" arm-down delay is actually zero) -- HARD RULE 3
//  forbids "improving" arithmetic, so `.SetSecAndOn(iArmDownDelayTime)` below
//  really does arm a zero-second timer, exactly as golden does.
//  golden :9 `#include "ainarm2.h"` does not exist in this tree (that golden
//  unit is unported); its declarations this file actually needs are already
//  real, scattered across cmydef.h (iInArmXBase/iInArmYBase/
//  iInArmShtXCenterPos/iInArmShtYCenterPos), Motor/mymotor.h
//  (InArmContinuousMove_9045/MOT[]/X_PITCH_COUNT/MAX_ARM_Row/MAX_ARM_Col) and
//  aHotPlateSubstrate.h (TransferInShuttleRatio) -- included individually
//  below instead.  golden :10 "MyMotor.h" (BCB6 case-insensitive path) maps
//  to this tree's Motor/mymotor.h.  golden :13 "note.h" (ShowErrorMessage/
//  K_SKIP/K_RETRY) maps to canary_support.h, matching the SAME 2-arg-
//  MyDBIProcess-avoidance precedent ATC/TCPData.cpp already set (this file
//  does not call MyDBIProcess, but does need canary_support.h's
//  ShowErrorMessage). No __fastcall / __property / __published anywhere in
//  golden's body.  SOFT_SIMULTE is NOT defined: CheckInArmPlaceSuckHasIC's
//  #ifdef SOFT_SIMULTE arm (golden :334-339) is reproduced verbatim but
//  inert, and the #else real-machine arm compiles.
//  Intentional golden fall-throughs (case 1100->1200, 1200->1300 etc., in
//  DoSuckFromShuttleToPlace/DoSuckFromLoaderToPlace/DoSuckToRecycleFrom{
//  Shuttle,Loader}) are kept EXACTLY as golden wrote them, including the
//  commented-out `//                break;` lines -- NOT annotated with
//  `[[fallthrough]]`, matching the precedent asendic_Auto.cpp's banner
//  states explicitly (inserting the attribute would break the zero-diff
//  fidelity proof; this toolchain's -Wimplicit-fallthrough does not exist in
//  MinGW.org g++ 6.3.0 regardless).
//
//  Big5: every Chinese comment decoded via cp950 and preserved as UTF-8.
//  Final gate: ZERO U+FFFD.
// =============================================================================
#include "MachineDefine.h"          // de-VCL'd include hub (vclcompat umbrella + portable STL + using namespace std)

#include "cInArmPlacement.h"        // this unit's own contract
#include "cprod.h"                  // Prod (XInArm_Shuttle1/2_Place, XInArm_Tray_Pick, iInPlacement*, LoadForm), TestIF (dSiteXPitch/dSiteYPitch), SThreadPara (iInShSenIndex)
#include "cmydef.h"                 // NULL_IC, HAS_SKIP_IC, C_TrayX_UpDown, MMTrayY, MInArmX, iInArmXBase/YBase, iInArmShtXCenterPos/YCenterPos, USE_InPlacement, USE_IN_Y_IS_AUTO_PITCH
#include "mysensor.h"                // Sen[] (TMySensor::Enable/IsOn) -- golden DoCheckShuttleSensor
#include "Motor/mymotor.h"           // MOT[] (TTrayMotor.Tray / .SetTraySingleData), InArmContinuousMove_9045 (W4 sim stub, mymotor.cpp:1356), X_PITCH_COUNT, MAX_ARM_Row, MAX_ARM_Col
#include "mycylin.h"                 // Cylinder[] (TMyCylinder::On/Off/OnStatus)
#include "aHotPlateSubstrate.h"      // TMyKitSuck (InArmPlaceSuck's real class, see banner) + TransferInShuttleRatio (golden ainarm2.cpp:1972, already a no-op shim here)
#include "MachineType.h"             // eartInstall (eAutoRetest)
#include "canary_support.h"          // ShowErrorMessage / K_SKIP / K_RETRY -- golden note.h
//---------------------------------------------------------------------------
const int iArmDownDelayTime=0.1;                                                //AI(W906-PT-W3) 20260807: golden :17 verbatim, INCLUDING the int-truncates-0.1-to-0 quirk (see file head) -- NOT "improved" to a double
//---------------------------------------------------------------------------
//AI(W906-PT-W3) 20260807: golden MyKitSuck.h:371 `extern TMyKitSuck InArmPlaceSuck;`
//  (golden home: mykitsuck.cpp, a DIFFERENT, unported-into-this-TU golden
//  unit). This file is the sole ported consumer this wave, so the object is
//  defined here against the tree's real substrate TMyKitSuck (see file head
//  "A GLOBAL THIS FILE DEFINES" note) -- NOT a gate, a real global.
//TMyKitSuck InArmPlaceSuck;   //AI(W906-A4-6) 20260924: retired -- golden home mykitsuck.cpp:219 is now registered (same archive ht9045_sm; both members extracted together = multiple definition)
//---------------------------------------------------------------------------
cInArmPlacement::cInArmPlacement()
{
    InitialDatas();
}
//---------------------------------------------------------------------------
cInArmPlacement::~cInArmPlacement()
{
    InitialDatas();
}
//---------------------------------------------------------------------------
void cInArmPlacement::InitialDatas()
{
}
//---------------------------------------------------------------------------
void cInArmPlacement::InitialShuttleDatas()
{
    iSuckToRecycleFromShuttleTask=0;
    iSuckFromShuttleToPlaceTask=0;
    iSuckFromShuttleTask=0;
    iPlaceSuckToPlacementTask=0;
    asErrorInArmPlacement=AnsiString("");
    iShuttleLine=0;
    iWhichShuttle=0;
    iSuckToRecycleFromLoaderTask=0;
    iSuckFromLoaderToPlaceTask=0;
}
//---------------------------------------------------------------------------
bool cInArmPlacement::DoSuckToRecycleFromShuttle(bool bIsFirst, AnsiString &asError)
{
    int &iTask=iSuckToRecycleFromShuttleTask;
    static int iErrorNo=0;
    if(bIsFirst)
    {
        asError=AnsiString("");
        iTask=1;
        return true;
    }
    else
    {
        switch(iTask)
        {
            case 1://Initial
                iTask=1000;
                break;
            case 1000://check shuttle has error IC
                iErrorNo=DoCheckShuttleSensor();
                if(iErrorNo>=0)
                {
                    iTask=1100;
                }
                else
                {
                    iTask=9900;
                }
                break;
            case 1100://pick
                DoSuckFromShuttleToPlace(true, iWhichShuttle, iErrorNo, asErrorInArmPlacement);
                iTask=1200;
//                break;
            case 1200://pick
                if(DoSuckFromShuttleToPlace(false, iWhichShuttle, iErrorNo, asErrorInArmPlacement))
                {
                    iTask=1000;
                }
                break;
            case 9900://finish
                return true;
        }
        return false;
    }
}
//---------------------------------------------------------------------------
int cInArmPlacement::DoCheckShuttleSensor()
{
    int Pos=-1;
    if(iShuttleLine<=0)
    {
        return -1;
    }
    else
    {
        for(int i=0; i<iShuttleLine; i++)
        {
            Pos=SThreadPara.iInShSenIndex[iWhichShuttle][i];
            if(Sen[Pos].Enable && Sen[Pos].IsOn())
            {
                return i;
            }
        }
    }
    return -1;
}
//---------------------------------------------------------------------------
bool cInArmPlacement::DoSuckFromShuttleToPlace(bool bIsFirst, int iSht, int iCol, AnsiString &asError)
{
    int &iTask=iSuckFromShuttleToPlaceTask;
    if(bIsFirst)
    {
        asError=AnsiString("");
        iTask=1;
        return true;
    }
    else
    {
        switch(iTask)
        {
            case 1:                                                             //Initial
                iTask=1000;
                break;
            case 1000:                                                          //pick shuttle row1
                DoSuckFromShuttle(true, iSht, ShRow1, iCol, asError);
                iTask=1100;
//                break;
            case 1100:
                if(DoSuckFromShuttle(false, iSht, ShRow1, iCol, asError))
                {
                    iTask=1200;
                }
                break;
            case 1200:                                                          //move to placement
                DoPlaceSuckToPlacement(true, asError);
                iTask=1300;
//                break;
            case 1300:                                                          //move to placement
                if(DoPlaceSuckToPlacement(false, asError))
                {
                    iTask=2000;
                }
                break;
            case 2000:                                                          //pick shuttle row2
                DoSuckFromShuttle(true, iSht, ShRow2, iCol, asError);
                iTask=2100;
//                break;
            case 2100:                                                          //pick shuttle row2
                if(DoSuckFromShuttle(false, iSht, ShRow2, iCol, asError))
                {
                    iTask=2200;
                }
                break;
            case 2200:                                                          //pick shuttle row2
                DoPlaceSuckToPlacement(true, asError);
                iTask=2300;
//                break;
            case 2300:                                                          //move to placement
                if(DoPlaceSuckToPlacement(false, asError))
                {
                    iTask=9900;
                }
                break;
            case 9900:                                                          //finish
                return true;
        }
        return false;
    }
}
//---------------------------------------------------------------------------
bool cInArmPlacement::DoSuckFromShuttle(bool bIsFirst, int iSht, int iRow, int iCol, AnsiString &asError)
{
    int &iTask=iSuckFromShuttleTask;
    int iPosX=0, iPosY=0;
    if(bIsFirst)
    {
        asError=AnsiString("");
        iTask=1;
        return true;
    }
    else
    {
        switch(iTask)
        {
            case 1:                                                             //Initial
                iTask=100;
                break;
            case 100:                                                           //move to error ic and suck
                GetShuttlePos(iSht, iRow, iCol, iPosX, iPosY);
                if(MoveInArmXY(iPosX, iPosY))
                {
                    DoInArmPlaceDown(true);
                    DoInArmPlaceSuck(true);
                    timeArmDownDelay.SetSecAndOn(iArmDownDelayTime);
                    iTask=200;
                }
                break;
            case 200:                                                           //wait and check suck
                if(timeArmDownDelay.Off())
                {
                    DoInArmPlaceDown(false);
                    timeArmDownDelay.SetSecAndOn(iArmDownDelayTime);
                    iTask=300;
                }
                break;
            case 300:                                                           //wait and check suck
                if(timeArmDownDelay.Off())
                {
                    if(CheckInArmPlaceSuckHasIC()==false)
                    {
                        asError.sprintf("Auto Pickup Error IC Failed,Shuttle:%d, Row:%d, Col:%d", iSht, iRow, iCol);
                        int ret=ShowErrorMessage("JAM0126", K_SKIP|K_RETRY, MInArmX, false, asError);
                        if(ret==K_RETRY)
                        {
                            asError="";
                            iTask=100;
                        }
                        else                                                    //K_SKIP
                        {
                            iTask=9900;
                        }
                    }
                    else
                    {
                        iTask=9900;
                    }
                }
                break;
            case 9900:                                                          //finish
                return true;
        }
        return false;
    }
}
//---------------------------------------------------------------------------
void cInArmPlacement::GetShuttlePos(int iSht, int iRow, int iCol, int &iPosX, int &iPosY)
{
    iPosX=iInArmShtXCenterPos;                                                  //offset by shuttle center
    iPosY=iInArmShtYCenterPos;
    if(iSht==0)                                                                 //move to shuttle basic point
    {
        iPosX+=Prod.XInArm_Shuttle1_Place[iInArmYBase][iInArmXBase];            //Steven 20141110 : [0][2] --> [iInArmYBase][iInArmXBase]
        iPosY+=Prod.YInArm_Shuttle1_Place[iInArmYBase][iInArmXBase];
    }
    else
    {
        iPosX+=Prod.XInArm_Shuttle2_Place[iInArmYBase][iInArmXBase];
        iPosY+=Prod.YInArm_Shuttle2_Place[iInArmYBase][iInArmXBase];
    }
    //move to basic hole/suck
    //<==
    iPosX+=TestIF.dSiteXPitch/2;
    if(USE_IN_Y_IS_AUTO_PITCH==true)                                                  //ChungHung 20140304 add for AutoYPitch  //JerryYang 20251218 : IN/OUT ARM支援不同模組
        iPosY-=TestIF.dSiteYPitch/2;
    else
        iPosY+=TestIF.dSiteYPitch/2;
    //<==
    //move to basic hole/suck
    //move to Placement Suck
    //<==
    iPosX+=Prod.iInPlacementOffsetByBasicX;
    iPosY+=Prod.iInPlacementOffsetByBasicY;
    //<==
    //move to Placement Suck
    //move to Destination hole 2x4
    //<==
    iPosX=(iPosX-(iInArmXBase*TestIF.dSiteXPitch))+iCol*TestIF.dSiteXPitch;
    iPosY=(iPosY-(iInArmYBase*TestIF.dSiteYPitch))+iRow*TestIF.dSiteYPitch;
    //<==
    //move to Destination hole 2x4
    TransferInShuttleRatio(iSht, &iPosX, &iPosY, iInArmYBase, iInArmXBase);     //Steven 20110324
}
//---------------------------------------------------------------------------
void cInArmPlacement::GetPlacementPos(int &iPosX, int &iPosY)
{
    iPosX=Prod.iInPlacementX;
    iPosY=Prod.iInPlacementY;
}
//---------------------------------------------------------------------------
bool cInArmPlacement::MoveInArmXY(int iPosX, int iPosY)
{
    int iXPos=iPosX, iYPos=iPosY;
    int iXVariable[X_PITCH_COUNT]={0, 0, 0, 0};
    int iYVariable  =0;
    bool bZFlag[MAX_ARM_Row][MAX_ARM_Col];
    int iZPos[MAX_ARM_Row][MAX_ARM_Col];
    ZeroMemory(bZFlag, sizeof(bZFlag));
    ZeroMemory(iZPos, sizeof(iZPos));

    if(InArmContinuousMove_9045(iXPos, iYPos, iXVariable, iYVariable, bZFlag, iZPos, false, false))
    {
        return true;
    }
    return false;
}
//---------------------------------------------------------------------------
void cInArmPlacement::DoInArmPlaceDown(bool bDown)
{
    if(bDown)
    {
        if(Cylinder[C_TrayX_UpDown].OnStatus()==false)
        {
            Cylinder[C_TrayX_UpDown].On();
        }
    }
    else
    {
        if(Cylinder[C_TrayX_UpDown].OnStatus()==true)
        {
            Cylinder[C_TrayX_UpDown].Off();
        }
    }
}
//---------------------------------------------------------------------------
void cInArmPlacement::DoInArmPlaceSuck(bool bSuck)
{
    if(bSuck)
    {
        InArmPlaceSuck.Suck[0][0].On();
    }
    else
    {
        InArmPlaceSuck.Suck[0][0].Normal();
    }
}
//---------------------------------------------------------------------------
bool cInArmPlacement::CheckInArmPlaceSuckHasIC()
{
    bool bflag=false;
    #ifdef SOFT_SIMULTE
    bflag=true;
    #else
    bflag=InArmPlaceSuck.Suck[0][0].GetStatus();

    #endif
    return bflag;
}
//---------------------------------------------------------------------------
bool cInArmPlacement::InArmPlacementEnable()
{
    return (USE_InPlacement==eartInstall && IniConfig.bE69_PickupErrorPlacement);
}
//---------------------------------------------------------------------------
bool cInArmPlacement::DoPlaceSuckToPlacement(bool bIsFirst, AnsiString &asError)
{
    int &iTask=iPlaceSuckToPlacementTask;
    int iPosX=0, iPosY=0;
    if(bIsFirst)
    {
        asError=AnsiString("");
        iTask=1;
        return true;
    }
    else
    {
        switch(iTask)
        {
            case 1:                                                             //Initial
                iTask=100;
                break;
            case 100:                                                           //move to error ic and suck
                GetPlacementPos(iPosX, iPosY);
                if(MoveInArmXY(iPosX, iPosY))
                {
                    DoInArmPlaceDown(true);
                    timeArmDownDelay.SetSecAndOn(iArmDownDelayTime);
                    iTask=200;
                }
                break;
            case 200:                                                           //wait and check suck
                if(timeArmDownDelay.Off())
                {
                    DoInArmPlaceSuck(false);
                    DoInArmPlaceDown(false);
                    timeArmDownDelay.SetSecAndOn(iArmDownDelayTime);
                    iTask=300;
                }
                break;
            case 300:                                                           //wait and check suck
                if(timeArmDownDelay.Off())
                {
                    iTask=9900;
                }
                break;
            case 9900:                                                          //finish
                return true;
        }
        return false;
    }
}
//---------------------------------------------------------------------------
bool cInArmPlacement::DoCheckLoaderHasErrorIC(int &iRow, int &iCol)
{
    for(int j=0; j<MOT[MMTrayY].Tray.YItem; j++)
        for(int i=0; i<MOT[MMTrayY].Tray.XItem; i++)
            if(MOT[MMTrayY].Tray.Data[i][j]>0)
            {
                if(MOT[MMTrayY].Tray.Data[i][j]==HAS_SKIP_IC)
                {
                    iCol=i;
                    iRow=j;
                    return true;
                }
            }
    return false;
}
//---------------------------------------------------------------------------
bool cInArmPlacement::DoSuckToRecycleFromLoader(bool bIsFirst, AnsiString &asError)
{
    int &iTask=iSuckToRecycleFromLoaderTask;
    static int iCol=0, iRow=0;
    if(bIsFirst)
    {
        asError=AnsiString("");
        iTask=1;
        return true;
    }
    else
    {
        switch(iTask)
        {
            case 1://Initial
                iTask=1000;
                break;
            case 1000://check loader has error IC
                if(DoCheckLoaderHasErrorIC(iRow, iCol))
                {
                    iTask=1100;
                }
                else
                {
                    iTask=9900;
                }
                break;
            case 1100://pick
                DoSuckFromLoaderToPlace(true, iRow, iCol, asErrorInArmPlacement);
                iTask=1200;
//                break;
            case 1200://pick
                if(DoSuckFromLoaderToPlace(false, iRow, iCol, asErrorInArmPlacement))
                {
                    iTask=1000;
                }
                break;
            case 9900://finish
                return true;
        }
        return false;
    }
}
//---------------------------------------------------------------------------
bool cInArmPlacement::DoSuckFromLoaderToPlace(bool bIsFirst, int iRow, int iCol, AnsiString &asError)
{
    int &iTask=iSuckFromLoaderToPlaceTask;
    if(bIsFirst)
    {
        asError=AnsiString("");
        iTask=1;
        return true;
    }
    else
    {
        switch(iTask)
        {
            case 1://Initial
                iTask=1000;
                break;
            case 1000://pick shuttle row1
                DoSuckFromLoader(true, iRow, iCol, asError);
                iTask=1100;
//                break;
            case 1100:
                if(DoSuckFromLoader(false, iRow, iCol, asError))
                {
                    iTask=1200;
                }
                break;
            case 1200://move to placement
                DoPlaceSuckToPlacement(true, asError);
                iTask=1300;
//                break;
            case 1300://move to placement
                if(DoPlaceSuckToPlacement(false, asError))
                {
                    iTask=9900;
                }
                break;
            case 9900://finish
                return true;
        }
        return false;
    }
}
//---------------------------------------------------------------------------
bool cInArmPlacement::DoSuckFromLoader(bool bIsFirst, int iRow, int iCol, AnsiString &asError)
{
    int &iTask=iSuckFromLoaderTask;
    int iPosX=0,iPosY=0;
    if(bIsFirst)
    {
        asError=AnsiString("");
        iTask=1;
        return true;
    }
    else
    {
        switch(iTask)
        {
            case 1://Initial
                iTask=100;
                break;
            case 100://move to error ic and suck
                GetLoaderPos(iRow, iCol, iPosX, iPosY);
                if(MoveInArmXY(iPosX, iPosY))
                {
                    DoInArmPlaceDown(true);
                    DoInArmPlaceSuck(true);
                    timeArmDownDelay.SetSecAndOn(iArmDownDelayTime);
                    iTask=200;
                }
                break;
            case 200://wait and check suck
                if(timeArmDownDelay.Off())
                {
                    DoInArmPlaceDown(false);
                    timeArmDownDelay.SetSecAndOn(iArmDownDelayTime);
                    iTask=300;
                }
                break;
            case 300://wait and check suck
                if(timeArmDownDelay.Off())
                {
                    if(CheckInArmPlaceSuckHasIC()==false)
                    {
                        asError.sprintf("Auto Pickup Error IC Failed,Loader, Row:%d, Col:%d, InarmX=%d,InarmY=%d", iCol, iRow, iPosX, iPosY);
                        int ret=ShowErrorMessage("JAM0126", K_SKIP|K_RETRY, MInArmX, false, asError);
                        if(ret==K_RETRY)
                        {
                            asError="";
                            iTask=100;
                        }
                        else//K_SKIP
                        {
                            iTask=9900;
                        }
                    }
                    else
                    {
                        iTask=9900;
                    }
                }
                break;
            case 9900://finish
                MOT[MMTrayY].SetTraySingleData(iCol, iRow, NULL_IC);
                return true;
        }
        return false;
    }
}
//---------------------------------------------------------------------------
void cInArmPlacement::GetLoaderPos(int iRow, int iCol, int &iPosX, int &iPosY)
{
    iPosX=Prod.XInArm_Tray_Pick[iInArmYBase][iInArmXBase]+(Prod.LoadForm.iXPitch*iCol);
    iPosY=Prod.YInArm_Tray_Pick[iInArmYBase][iInArmXBase]+(Prod.LoadForm.iYPitch*iRow);
    //move to Placement Suck
    //<==
    iPosX+=Prod.iInPlacementOffsetByBasicX;
    iPosY+=Prod.iInPlacementOffsetByBasicY;
    //<==
    //move to Placement Suck
#if 0 // GATE (1): golden ainarm2.cpp:2112 TransferLoaderRatio(int*,int*) -- see file-head GATE REGISTER for the full grep/rationale/behaviour-delta.
    TransferLoaderRatio(&iPosX, &iPosY);                                        //Steven 20141029 : Loader的軟體齒輪比
#else
    // GATE (1) faithful default: no-op (mirrors the sibling shim
    // TransferInShuttleRatio, aHotPlateSubstrate.cpp:1502) -- see file head.
#endif
}
