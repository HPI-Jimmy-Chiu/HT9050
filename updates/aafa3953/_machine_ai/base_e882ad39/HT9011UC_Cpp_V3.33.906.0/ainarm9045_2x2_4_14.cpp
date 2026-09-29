// =============================================================================
//  ainarm9045_2x2_4_14.cpp  --  In-Arm per-site VARIANT (2x2_4_14 dual-row)
//
//  Faithful translation of golden ainarm9045_2x2_4_14.cpp (2927 lines, BCB6, Big5).
//  Translator: AI(W6.2c-INARM-2x2_4_14) 20260626
//  Translation wave: W6.2c (in-arm per-site-config variant: 2x2_4_14).  Sibling of
//  the already-translated leaves ainarm_SearchPickPlate / ainarm_SearchPlacePlate
//  and the engine ainarm9045.cpp.  This file is dispatched by the engine's
//  DoInArm_9045() / DoInArm_9045_SuckerMap() ladder for iInArmType==e9045_2x2_4_14
//  (un-gated at the serial Integrate stage; this Translate agent touches only its
//  own .cpp/.h, never a shared file).
//
//  ROLE: the per-site in-arm pick/place state machines for the 2x2_4_14 layout
//  (a dual-row 2x2 site config).  This file owns (golden file:line):
//    XPHSuckToSht_2x2_14[3][8]                  :32   (file-scope const; .h externs it)
//    bTwoStepPlaceToHP                          :38   (file-scope int; file-local)
//    GetNowSiteKitMode_2x2_4_14                 :40
//    InArmZNeedDown_2x2_4_14                    :141
//    CheckXYPitch_2x2_4_14                      :178
//    MoveInArm2XYToShuttle_9045_2x2_4_14        :280
//    DoPlaceToHotPlate_9045_2x2_4_14            :362
//    DoInArmPlaceToShuttle_9045_2x2_4_14        :676
//    DoInArm_9045_2x2_4_14_SuckerMap            :1562  (engine dispatch symbol)
//    DoInArm_9045_2x2_4_14                      :1591  (engine dispatch symbol)
//    DoInArmTryPickFromHotPlate_9045_2x2_4_14   fwd :37 / def :2501
//    GetShuttleState_2x2_4_14                   :2860
//    SetInArmHasDropToShuttle_2x2_4_14          :2903
//
//  FAITHFUL: every fn/var/cursor value, switch(Task) structure, formula,
//  fall-through and goto/label (IN_ARM_SHUTTLE_2x2_4_14 /
//  IN_ARM_TRY_PICK_HOTPLATE_2x2_4_14) is transcribed VERBATIM from the golden
//  CP950.  The one #ifdef SOFT_SIMULTE block in
//  DoInArmTryPickFromHotPlate_9045_2x2_4_14 (golden :2578) keeps its golden #else
//  (real) path active, reproduced verbatim (SOFT_SIMULTE is NOT defined).  Numeric
//  semantics (int vs double, integer division, double pitch math) kept EXACT --
//  e.g. the GetNowSiteKitMode mode codes (iMode%100 / iMode/100), the
//  dMovePitchX=double(iMovePitchX)/N.0 pitch math, HotPlateForm.XDivision/2 vs /4.
//
//  ONE GATED BODY (the only substrate gap -- see SUBSTRATE NEEDS below):
//    GetShuttleState_2x2_4_14 derefs ptrInSHT->ArmDownSideAllTypeIC(...), a
//    TMyKitSuck method NOT yet present in the offline substrate (the sibling
//    ArmUpSideAllTypeIC IS present at aHotPlateSubstrate.h:263; ArmDownSideAllTypeIC
//    is golden MyKitSuck.h:335, used today only inside the gated ainarm9045_2x8_32
//    body).  Because it is a class member I cannot forward-declare it without
//    editing the shared aHotPlateSubstrate.h.  So the golden GetShuttleState body is
//    preserved VERBATIM under "#if 0 // TODO(W7)" and an ACTIVE stub returns 0
//    (a sane "no shuttle has IC / no place needed" default).  Once Integrate adds
//    ArmDownSideAllTypeIC to TMyKitSuck, flip the gate to make this faithful.
//
//  VCL/Borland conversions: AnsiString (1-based, .sprintf) via vclcompat;
//  __fastcall/__property/__published NOT present in this file; __FUNC__ via the
//  aArmHeader shim.  Big5 comments -> UTF-8, ZERO U+FFFD.
// =============================================================================
// BCB6 ORIGINAL include block (mirrored as a comment for provenance):
//   #include "MachineDefine.h" ; #pragma hdrstop
//   #include "ainarm9045_2x2_4_14.h"
//   #include "aArmHeader.h" (god-header; W6.0 SHIM) / "MyKitSuck.h" / "cprod.h"
//   #include "mymotor.h" / "mycylin.h" / "mysensor.h" / "cSortCT.h" / "note.h"
//   #include "main.h" / "csystem.h" / "uhome.h" / "mymessbox.h" / "cinitial.h"
//   #include "atester.h" / "aRotateKIT.h" / "fRotate.h" / "cOffSet.h" / "cMyDB.h"
//   #include "cObserver.h" / "cmydef.h" / "uYieldMonitoring.h" / "BarCode.h"
//   #include "InOutArmZteach.h" / "acarry.h" ; #pragma package(smart_init)
// =============================================================================
#include "MachineDefine.h"

#include "ainarm9045_2x2_4_14.h"
#include "aArmHeader.h"             // __FUNC__ shim, RecordProcess, etc.
#include "ainarm9045.h"             // engine: GetInArmToShuttleOffset_9045 / InArmNeedCheckOffset / MoveInArmXYToShuttle_9045 / GetShuttleCol / ...
#include "ainarm_SearchPickPlate.h" // shared HP-pick externs (bInArmTryPickFromHotPlateFinish / HasHotReadyIC_9045 / iTryPickHP / bTryPickHPDuplicateErr / ...)
#include "ainarm_SearchPlacePlate.h"// shared HP-place externs (SearchPlateToPlace / MoveInArmXYToHotPlatePlace / DoPlaceToHPSwapData / GetPlaceToHotPlateSuckCol / Row2CanPutHP / GetHotPlateColStep / iPlaceHP / HPPlaceLog)
#include "csystem.h"                // CheckInArmSuckICFallDownToHasNullIC / DoInArmAutoSiteMapping / CheckInArmDestroyICFail / DoSiteMappingCHK predicates
#include "Motor/mymotor.h"          // MOT[], InArmContinuousMove_9045, TTrayMotor SetTraySingleData/InitEmptyTray/InitNewTray
#include "cmydef.h"                 // global scalar universe + IC consts + enums (eInArmType / picker enum / Tempture_* / rsm* / CC_*)
#include "common.h"
#include "mycylin.h"
#include "myswitch.h"               // SW[] (SwShuttleVibration1/2 .On())
#include "cprod.h"                  // Prod / TestIF / TestIF_File / ArmSpeed / ArmSpeed_File / LastSet
#include "cpublic.h"                // CosFunction
#include "atester.h"                // dwStartShuttle1Soak / dwStartShuttle2Soak / bOneTimeWait
#include "aHotPlateSubstrate.h"     // InArmSuck/FLCarryKit/BLCarryKit/OutArmSuck/FTestSuck/BTestSuck + ainarm2 cursors/timers + engine shims + PickFromHPList + ptrInSHT(Backup) + Zteach/DoCheckAutoSiteMappingPosition/CheckClearAllHotICThenPickLoadIC
#include "FormsFacade.h"            // HotPlateForm / TrayForm / fContact / fObserver / fYieldMonitoring offline stand-ins
#include "atester_shims.h"          // fObserver (RecordInArmTime/AddTimeData) / fContact (IsRun2DCheck)
#include "acatchtray_shims.h"        // InitProcessSingleMotorTask / MoveInArm2XYToWait / InShtInLF / InSHT1InLF / InSHT2InLF / CheckOneCycleAction
#include "ainarm9045_2x4_16_shims.h"// InitInArmTryPickFromHotPlateTask100 / AdjustShuttlePlaceOrder_AutoSiteMapping / MyTickCount / fYieldMonitoring / iInposLed
#include "canary_support.h"         // ShowErrorMessage / ShowMyMessage / RecordProcess / LastSet / __FUNC__ / K_RETRY / Tempture_Hot / REALLY

// golden ainarm9045_2x2_4_14.cpp:32 -- file-scope picker->shuttle remap (own TU;
// .h externs it).  Verbatim.
const int XPHSuckToSht_2x2_14[3][8]={{0, 2, 3, 1, 4, 5, 6, 7},
                                     {0, 2, 3, 1, 4, 5, 6, 7},
                                     {0, 2, 3, 1, 4, 5, 6, 7}};

//==============================================================================
// golden ainarm9045_2x2_4_14.cpp:37 -- forward decl (file-local) + golden :38
// file-scope two-step-place-to-HP flag (kept file-local exactly as golden).
bool DoInArmTryPickFromHotPlate_9045_2x2_4_14(bool bOneTimeCheckAll, bool bShowErrorMessage);
int bTwoStepPlaceToHP=0;
//==============================================================================
// golden ainarm9045_2x2_4_14.cpp:40
int GetNowSiteKitMode_2x2_4_14(int iSht, bool bPlace)
{
    int iX=HAS_TESTING_IC;
    if(Zteach->fShow)                                                           //kevin 20210416 add Autoteach Z offset shuttle pick
        iX=NULL_IC;
    else if(bRunAutoClean)                                                      //kevin 20220927  IC >3500 //kevin 20220922//kevin 20121430
        iX=CLEAN_FINISH_IC;

    if(bPlace==true)
    {
        if(TestIF.iAutoClean_Function && bUse8Picker==false && bRunAutoClean)
        {
            if(InArmSuck.iXStep==1)
            {
                return (iShuttleRowKit==1)?0:1;
            }
            else
            {
                if(InArmSuck.Item[1][0]>=HAS_IC)
                    return (iShuttleRowKit==1)?10000:10001;
                else                                                            //if(InArmSuck.Item[1][1]>=HAS_IC)
                    return (iShuttleRowKit==1)?11000:11001;
            }
        }
        else
        {
            if(InArmSuck.iXStep==1)
            {
                if(InArmSuck.iYStep==1 && bRunAutoClean==false)
                {
                    return 2;
                }
                else
                {
                    for(int j=0; j<4; j++)
                    {
                        if(InArmSuck.Item[0][j]>=HAS_IC)
                            return 0;                                           //A排四顆
                    }
                    return 1;                                                   //B排四顆
                }
            }
            else
            {
                if(InArmSuck.iYStep==1 && bRunAutoClean==false)
                {
                    if(InArmSuck.Item[0][0]>=HAS_IC ||
                       InArmSuck.Item[1][0]>=HAS_IC)
                        return 10002;
                    else
                        return 10102;
                }
                else
                {
                    if(InArmSuck.Item[0][0]>=HAS_IC)
                        return 10000;
                    else if(InArmSuck.Item[1][0]>=HAS_IC)
                        return 10001;
                    else if(InArmSuck.Item[0][InArmSuck.iPickStep]>=HAS_IC)
                        return 11000;
                    else
                        return 11001;
                }
            }
        }
    }
    else
    {
        for(int i=0; i<FLCarryKit.iMaxRow; i++)                                 //Steven 20241206 : InArmSuck.iShtRow -->　FLCarryKit.iMaxRow
        {
            for(int j=0; j<FLCarryKit.iMaxCol; j++)                             //Steven 20241206 : InArmSuck.iShtCol -->　FLCarryKit.iMaxCol
            {
                if(iSht==0)
                    ptrInSHTBackup.SetItemData(i, j, (FLCarryKit.Item[i][j] || FTestSuck.Item[i][j])?iX:0);             //kevin 20220928  Autoteach Z offset shuttle pick//ChungHung 20131120 AutoClean use Hotplate1
                else
                    ptrInSHTBackup.SetItemData(i, j, (BLCarryKit.Item[i][j] || BTestSuck.Item[i][j])?iX:0);             //kevin 20220928  Autoteach Z offset shuttle pick //ChungHung 20131120 AutoClean use Hotplate1
            }
        }

        if(InArmSuck.iXStep==1)
        {
            if(ptrInSHTBackup.Item[0][0] || ptrInSHTBackup.Item[0][1])
                return 0;
            else
                return 1;
        }
        else if(InArmSuck.iXStep==2)
        {
            if(ptrInSHTBackup.Item[0][0]==iX)
                return 10000;
            else if(ptrInSHTBackup.Item[0][1]==iX)
                return 11000;
            else if(ptrInSHTBackup.Item[1][0]==iX)
                return 10001;
            else                                                                //if(ptrInSHTBackup.Item[1][1]==iX)
                return 11001;
        }
    }
    return 0;
}
//==============================================================================
// golden ainarm9045_2x2_4_14.cpp:141
void InArmZNeedDown_2x2_4_14(int iSht, bool bPlace)
{
    if(InitialInArmNeedSuck(iSht, bPlace)==false)
        return;

    int iSuckRow, iSuckCol, iShtCol;
    int iMode   =GetNowSiteKitMode_2x2_4_14(iSht, bPlace);
    int iModeRow=iMode%100;
    int iModeCol=iMode/100;

    for(int i=0; i<2; i++)
    {
        for(int j=0; j<4; j++)
        {
            iSuckRow=(TestIF.iAutoClean_Function && bUse8Picker==false && bRunAutoClean)?1:i;
            iSuckCol=j;
            iShtCol =GetShuttleCol(i, j);

            if(iModeCol==0 &&                                                   //全部一起
               (iModeRow==2 || iModeRow==i))
            {
                SetInArmNeedDestory(bPlace, i, iShtCol, iSuckRow, iSuckCol);
            }
            else if(iModeCol==100 && j==0 &&                                    //吸嘴       ==>  蝦頭
                    (iModeRow==2 || iModeRow==i))                               // O X            O X
            {                                                                   // O X            O X
                SetInArmNeedDestory(bPlace, i, iShtCol, iSuckRow, iSuckCol);
            }
            else if(iModeCol==110 && j==3 &&                                    //吸嘴       ==>  蝦頭      //Steven 20240508 : 1 --> 3
                    (iModeRow==2 || iModeRow==i))                               // X O            X O
            {                                                                   // X O            X O
                SetInArmNeedDestory(bPlace, i, iShtCol, iSuckRow, iSuckCol);
            }
        }
    }
}
//==============================================================================
// golden ainarm9045_2x2_4_14.cpp:178
void CheckXYPitch_2x2_4_14(int *iX, int *iY, int iSht, bool bPlace, int iMovePitchX, int iMovePitchY)
{
    int iMode       =GetNowSiteKitMode_2x2_4_14(iSht, bPlace);
    int iModeRow    =iMode%100;
    int iModeCol    =iMode/100;
    int iOffsetPos  =GetInArmToShuttleOffset_9045(iSht, iModeRow, iModeCol, false);
    double dMovePitchX;

    if(iSht==0)
    {
        *iX+=Prod.XInArm_Shuttle1_Place[iInArmYBase][iInArmXBase];
        *iY+=Prod.YInArm_Shuttle1_Place[iInArmYBase][iInArmXBase];
    }
    else
    {
        *iX+=Prod.XInArm_Shuttle2_Place[iInArmYBase][iInArmXBase];
        *iY+=Prod.YInArm_Shuttle2_Place[iInArmYBase][iInArmXBase];
    }

    if(iModeRow==0)                                                             //Aa to Row A
    {
        if(USE_IN_Y_IS_AUTO_PITCH==true)                                        //JerryYang 20251218 : IN/OUT ARM支援不同模組
            *iY=*iY-iMovePitchY+TestIF.dSiteYPitch/2;
        else
            *iY=*iY+TestIF.dSiteYPitch/2;
    }
    else if(iModeRow==1)                                                        //Ba to Row A
    {
        if(USE_IN_Y_IS_AUTO_PITCH==true)                                        //JerryYang 20251218 : IN/OUT ARM支援不同模組
            *iY=*iY-TestIF.dSiteYPitch/2;
        else
            *iY=*iY+iMovePitchY-TestIF.dSiteYPitch/2;
    }
    else                                                                        //Steven 20240806 : fixed for 2x2 mode
    {
        if(USE_IN_Y_IS_AUTO_PITCH==true)                                        //JerryYang 20251218 : IN/OUT ARM支援不同模組
            *iY=*iY-TestIF.dSiteYPitch/2;
        else
            *iY=*iY+TestIF.dSiteYPitch/2;
    }

    if(TestIF_File.bQualSite2X2Shift)                                           //kevin 20170512 (wei) add 使用4 site layoutkit 使用1 3 Site shuttle 1X2 位置偏左2000
        *iX=*iX+TestIF_File.dShiftXPitch;

    if(USE_PICKER_COUNT==ep16Picker)                                            //基準為第四隻吸嘴
    {
        dMovePitchX=double(iMovePitchX)/7.0;
        /*if(iModeCol==0)                                                       //Aa --> Aa
        {
            *iX=*iX+(dMovePitchX*3.0-TestIF.dSiteXPitch*1.5);
        }
        else if(iModeCol==100)                                                  //Aa --> Aa
        {
            *iX=*iX+(dMovePitchX*3.0-TestIF.dSiteXPitch*1.5);
        }
        else if(iModeCol==110)                                                  //Ag --> Ab
        {
            *iX=*iX+(dMovePitchX*1.0+TestIF.dSiteXPitch*0.5);
        } */
    }
    else if(USE_IN_OUT_ARM_Y_PITCH==iXYPitchBb)                                 //基準為第二隻吸嘴 //Steven for HT7080
    {
        dMovePitchX=double(iMovePitchX)/3.0;
        if(iModeCol==0)                                                         //Aa --> Aa
        {
            *iX=*iX+(dMovePitchX*1.0-TestIF.dSiteXPitch*0.5);
        }
        else if(iModeCol==100)                                                  //Aa --> Aa
        {
            *iX=*iX+(dMovePitchX*1.0-TestIF.dSiteXPitch*0.5);
        }
        else if(iModeCol==110)                                                  //Ad --> Ab
        {
            *iX=*iX+(-dMovePitchX*1.0+TestIF.dSiteXPitch*0.5);
        }
    }
    else                                                                        //基準為第三隻吸嘴
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
        else if(iModeCol==110)                                                  //Ad --> Ab
        {
            *iX=*iX+(-dMovePitchX*1.0+TestIF.dSiteXPitch*0.5);
        }
    }

    *iY=*iY+GetInArmYToShuttleOffset_9045(iSht, iOffsetPos);
    *iX=*iX+GetInArmXToShuttleOffset_9045(iSht, iOffsetPos);
    TransferInShuttleRatio(iSht, iX, iY, iInArmYBase, iInArmXBase);

    InArmZNeedDown_9045(iSht, 0, bPlace);
}
//==============================================================================
// in arm x y to shuttle position
//==============================================================================
// golden ainarm9045_2x2_4_14.cpp:280
bool MoveInArm2XYToShuttle_9045_2x2_4_14(int iSht, bool IncludeZ, bool bPlace)
{
    int iXPos               =iInArmShtXCenterPos;                               //Steven 20141029 : XY-Pitch for Shuttle Center position for base Suck
    int iYPos               =iInArmShtYCenterPos;
    int iMode               =GetNowSiteKitMode_2x2_4_14(iSht, bPlace);
    int iModeRow            =iMode%100;
    int iModeCol            =iMode/100;
    int iMovePitchX         =iXpitchMaxX3;
    int iMovePitchY         =GetVariableYInShuttleData();
    int iOffsetPos          =GetInArmToShuttleOffset_9045(iSht, iModeRow, iModeCol, true);
    int iYVariable          =GetInArmPitchY_9045(iMovePitchY, iOffsetPos);
    static bool bCheckZSafe =false;

    if(bCheckZSafe==false)
    {
        if(MoveInArmZToPlateSafe(1111)==false)
            return false;
        else
            bCheckZSafe=true;

        ResetInToShtFlag();
    }

    if(USE_PICKER_COUNT==ep16Picker)                                            //基準為第四隻吸嘴
    {
        if(InArmSuck.iXStep==1)
            iMovePitchX=TestIF.dSiteXPitch/6.0*7.0;
        else
            iMovePitchX=iXpitchMaxX7;
    }
    else
    {
        if(InArmSuck.iXStep==1)
            iMovePitchX=TestIF.dSiteXPitch/3.0*3.0;
        else
            iMovePitchX=iXpitchMaxX3;
    }

    for(int i=0; i<X_PITCH_COUNT; i++)
        iInXPToSht[i]=GetInArmPitchX_9045(iMovePitchX, i, iOffsetPos);

    if(TestIF.bNS7000kit)                                                       //jou 981208 start : NS7000 bias kit
    {
        if(iSht==0)
            iYPos+=iMovePitchY/2;
        else
            iYPos-=iMovePitchY/2;
    }

    CheckXYPitch_2x2_4_14(&iXPos, &iYPos, iSht, bPlace, iMovePitchX, iMovePitchY);
    GetInArmZShtDownPos_9045(iSht, bPlace, IncludeZ);

    if(bRunAutoClean &&
       TestIF.iAutoClean_Function)
    {
        if(bUse8Picker==false)                                                  //Steven 20201014 : 整合8吸嘴auto clean
        {
            if(iShuttleRowKit==1 || iShuttleRowKit==3)
                iYPos+=iMovePitchY;
        }

        if(iSht==0)                                                             //Steven 20241102 : fixed for Auto Clean offset
        {
            iXPos=iXPos+TestIF_File.iAutoClean_Shuttle1XOffset;
            iYPos=iYPos+TestIF_File.iAutoClean_Shuttle1YOffset;
        }
        else
        {
            iXPos=iXPos+TestIF_File.iAutoClean_Shuttle2XOffset;
            iYPos=iYPos+TestIF_File.iAutoClean_Shuttle2YOffset;
        }
    }

    if(InArmContinuousMove_9045(iXPos, iYPos, iInXPToSht, iYVariable, bZFlgToSht, iZPosToSht, IncludeZ))
    {
        ResetInToShtFlag();
        bCheckZSafe=false;
        return true;
    }
    return false;
}
//==============================================================================
// golden ainarm9045_2x2_4_14.cpp:362
bool DoPlaceToHotPlate_9045_2x2_4_14()
{
    int &Task=iInArmPlaceToHotPlateTask;
    int ip, ix, iy, j2;
    int iStepHP=2;
    bool flag=true;

    GetHotPlateYHalfPos();

    switch(Task)
    {
        case 1:
            GetVariableXInHotPlateData(iPlaceHP);
            SearchPlateToPlace();
            bPlaceToHotplatePartOK=false;
            Task=100;                                                           //because Z and X-Y is continue move
//            break;
        case 100:
            if(MoveInArmXYToHotPlatePlace(iPlaceHP, true))
            {
                Task=200;
            }
            else
            {
                if(CheckInArmSuckICFallDownToHasNullIC(false)==true)                                                                                            //Steven 20110516 : 修改成整合式Alarm
                {
                    Task=110;                                                                                                                                   //JerryYang 20200422 修正掉料可能檢查不出來的問題, 檢查到有掉料要重新再確認全部吸嘴
                    break;
                }

                if(bTwoStepPlaceToHP!=0)
                {
                    for(int i=0; i<MAX_ARM_Row; i++)
                    {
                        for(int j=0; j<MAX_ARM_Col/2; j++)
                        {
                            if(i==1 && Row2CanPutHP(PlaceMode)==false && iPlaceHPOrder==0 && bAutoSiteMapHotplateSave==false)                                   //Ifor 20171003 (Steven) : add避免無法放回 Hotplate
                                continue;

                            if(InArmSuck.Item[i][j*3]==HAS_IC || InArmSuck.Item[i][j*3]==HAS_NULL_IC)
                            {
                                ip=iPlacePlate[0];
                                if(bPitchOver12000)
                                    ix=iPlacePlateX[0]+j*(HotPlateForm.XDivision/4);
                                else
                                    ix=iPlacePlateX[0]+j*(HotPlateForm.XDivision/2);

                                //jou 980326 all row1 no use hand
                                if(((Prod.fInArmSuck4x8[0][0][0]==false && Prod.fInArmSuck4x8[1][0][0]==false) &&
                                    (Prod.fInArmSuck4x8[0][0][1]==false && Prod.fInArmSuck4x8[1][0][1]==false))||                                               //ChungHung 20130910 alter for SCK can close site by Index
                                   ((Prod.fInArmSuck4x8[0][1][0]==false && Prod.fInArmSuck4x8[1][1][0]==false) &&
                                    (Prod.fInArmSuck4x8[0][1][1]==false && Prod.fInArmSuck4x8[1][1][1]==false)))
                                {
                                    iy=iPlacePlateY[0];
                                }
                                else
                                {
                                    if(iPlaceHPOrder==0)
                                        iy=iPlacePlateY[0]+iYHalf*i;
                                    else
                                        iy=iPlacePlateY[0];
                                }

                                if(i8PickerHPMode==iHPNoUse8suck)                                                                                               //Steven 20210420 : 修正2x2_14跑6x11加熱盤問題
                                {
                                    if(MOT[MMPlate1+ip].Tray.Data[ix][iy]==NULL_IC)
                                    {
                                        MOT[MMPlate1+ip].SetTraySingleData(ix, iy, HAS_NULL_IC);
                                    }
                                }
                            }
                        }
                    }
                    bTwoStepPlaceToHP=0;
                    Task=1;
                    return false;
                }
            }
            break;
        case 110:
            CheckInArmSuckICFallDownToHasNullIC();                              //JerryYang 20200422 修正掉料可能檢查不出來的問題, 檢查到有掉料要重新再確認全部吸嘴
            Task=100;
            break;
        case 200:
            if(InArmNeedCheckHotPlateOffset(true))                              //Steven 20230531 : 簡化判斷式
            {
                Task=210;
                break;
            }

            if(LastSet.iRunStartMode==rsmAutoSiteMap &&
               bAutoSiteMapHasPickHP==true &&
               CosFunction.bUSEJCETSiteMapMode==true)                           //Steven 20220527 : for JCET Auto Site Map
            {
                //pass
            }
            else                                                                //Jimmychiu 20230417 : Record the position after placing the IC
            {
                PickFromHPList->UpdateHPSuckGroup(iPlacePlate[0], iPlacePlateY[0], iPlacePlateX[0], InArmSuck.iWhichSht, InArmSuck.iWhichKit);
            }

            if(ArmSpeed[InArm].iEnableReleaseDelay==0)                          //JerryYang 20160127 for TSMC inarm release device前delay
            {
                InArmReleaseDelayToHot.SetSecAndOn(ArmSpeed[InArm].dReleaseDelayTime);
                Task=340;
            }
            else
            {
                Task=350;
            }
            break;
        case 210:
            if(bEnterOffset==true)
            {
                bEnterOffset=false;
                Task=220;
            }
            else
            {
                Task=100;
            }
            break;
        case 220:
            if(MoveInArmZToPlateSafe(Task))
            {
                Task=100;
            }
            break;
        case 340:
            if(InArmReleaseDelayToHot.Off())                                    //JerryYang 20160127 for TSMC inarm release device前delay
            {
                Task=350;
            }
            break;
        case 350:
            iStepHP=GetHotPlateColStep(iPlaceHP);

            if(LastSet.iRunStartMode==rsmAutoSiteMap &&
               bAutoSiteMapHasPickHP==true &&
               CosFunction.bUSEJCETSiteMapMode==true)                                                                   //Steven 20220527 : for JCET Auto Site Map
            {
                if(InArmSuck.Suck[iAutoSiteMapInArmRow][iAutoSiteMapInArmCol].Destroy())
                {
                    DoPlaceToHPBackupData(iAutoSiteMapInArmRow, iAutoSiteMapInArmCol, iAutoSiteMapHPNo, iAutoSiteMapHPR, iAutoSiteMapHPC);
                    InArmSuck.SetAllToNullIC();
                }
                else                                                                                                    //if(InArmSuck.Suck[iAutoSiteMapInArmRow][iAutoSiteMapInArmCol].Error==false) // kevin 20141206
                {
                    return false;
                }
            }
            else
            {
                for(int i=0; i<2; i++)
                {
                    for(int j=0; j<iStepHP; j++)
                    {
                        if(i==1 && Row2CanPutHP(PlaceMode)==false &&
                           iPlaceHPOrder==0 &&
                           bAutoSiteMapHotplateSave==false)                                                             //Ifor 20171003 (Steven) : add避免無法放回 Hotplate
                        {
                            continue;
                        }

                        j2=GetPlaceToHotPlateSuckCol(j);

                        if(InArmSuck.Item[i][j2]==HAS_IC ||
                           InArmSuck.Item[i][j2]==HAS_NULL_IC)
                        {
                            ip=iPlacePlate[0];
                            ix=GetPlaceToHotPlateCol(j);

                            if(ix>=HotPlateForm.XDivision &&
                               bAutoSiteMapHotplateSave==false)                                                         //Ifor 20180813 (Steven) : Add Auto Site Mapping 補回IC時不可跳過避免Hangup
                            {
                                continue;
                            }

                            if(iPlaceHPOrder==0)
                            {
                                iy=iPlacePlateY[0]+iYHalf*i;
                            }
                            else
                            {
                                iy=iPlacePlateY[0];
                            }

                            if(InArmSuck.Item[i][j2]==HAS_NULL_IC ||
                               InArmSuck.Suck[i][j2].Destroy())
                            {
                                if(bAutoSiteMapHotplateSave==true)
                                {
                                    DoPlaceToHPBackupData(i, j2, iAutoSiteMapHotplateSource, iy, ix);                   //Ifor 20170928 (Steven) : 將Auto Site Mapping 資料寫回到HP
                                }
                                else
                                {
                                    DoPlaceToHPSwapData(i, j2, ip, iy, ix);                                             //Steven 20170109 : 將放料到HP資料交換改成Function
                                }
                            }
                            else if(InArmSuck.Suck[i][j2].Error==false)
                            {
                                flag=false;
                            }
                        }
                    }
                }

                if(flag==false)
                    return false;

                for(int i=0; i<2; i++)
                {
                    for(int j=0; j<iStepHP; j++)
                    {
                        j2=GetPlaceToHotPlateSuckCol(j);

                        if(iPlaceHPOrder==0)
                        {
                            if(Row2CanPutHP(PlaceMode))
                            {
                                if(InArmSuck.Item[i][j2]!=NULL_IC)
                                    return false;
                            }
                            else
                            {
                                if(InArmSuck.Item[0][j2]!=NULL_IC)
                                    return false;
                            }
                        }
                        else
                        {
                            if(InArmSuck.Item[1][j2]!=NULL_IC)
                                return false;
                        }
                    }
                }
            }

            bPlaceToHotplatePartOK=true;                                                                                //ChungHung 20120412 add
            DoCheckAutoSiteMappingPosition();                                                                           //Ifor 20170928 (Steven) : Check Auto Site Mapping Position
            Task=400;
            break;
        case 400:
            if(MoveInArmZToPlateSafe(Task))
            {
                for(int i=0; i<2; i++)
                {
                    for(int j=0; j<4; j++)
                    {
                        j2=GetShuttleCol(i, j);
                        if(TestIF.iSiteMap[i][j2]!=0 &&                         //KevinCheng 20250912 : 0 --> i
                           InArmSuck.Item[i][j]!=NULL_IC)
                        {
                            Task=1;
                            return false;
                        }
                    }
                }

                Task=500;
            }
            break;
        case 500:                                                               //kevin 20131106 start   黏貨判斷
            flag=CheckInArmDestroyICFail();                                     //Steven 20111223 : 檢查吹氣IC是否粘黏錯誤
            if(flag==false)
            {
                RecordProcess("VOFTask=500");
                return false;
            }
            Task=501;
            break;
        case 501:
            SetInArm_Unuse_SuckToNullICForHP();

            AdjustShuttleWhichKitOrder();                                       //Steven 20160201 : 修正Shuttle放料順序
            iHotCount++;                                                        //jou 2013-02-08 修正等待 Soak 時間時，畫面沒有顯示倒數計時的問題

            if(LastSet.iTemperature==Tempture_Hot &&
               CheckHasSpaceToPlace_9045())
                SearchPlateToPlace();

            if(LastSet.iRunStartMode==rsmAutoSiteMap &&
               bAutoSiteMapHasPickHP==true &&
               CosFunction.bUSEJCETSiteMapMode==true)                           //Steven 20220527 : for JCET Auto Site Map
            {
                bAutoSiteMapHasPickHP=false;
            }
            else
            {
                PickFromHPList->AddHPSuckGroup();                               //放完了就加入一個新的Group
            }

            if(USE_LASER_DISTANCE &&
               TestIF_File.bEnableInArmLaser &&
               LaserCheckPos.size()!=0)                                         //Steven 20140228 : 雷射測距功能
            {
                CheckInArmFloating(true);
                Task=600;
                break;
            }
            else
            {
                return true;
            }
        case 600:
            if(CheckInArmFloating())                                            //Steven 20140228 : 雷射測距功能
            {
                return true;
            }
            break;
    }
    return false;
}
//==============================================================================
// golden ainarm9045_2x2_4_14.cpp:676
bool DoInArmPlaceToShuttle_9045_2x2_4_14()
{
    IN_ARM_SHUTTLE_2x2_4_14:

    QueueTaskList[4].CheckTaskChange();                                         //Steven 20200821 : 使用Goto也要記錄Task變化
    int ret;
    int &Task=iInArmPlaceToShuttleTask, iFlag=0;
    int iSuckRow=0, iSuckCol=0, iShtRow=0, iShtCol=0;
    bool flag, bCanFreeShuttle=false;
    bool bNeedAdjust2Time=false;                                                //Steven 20191112 : fixed for ASM close site hang up
    static bool bCheckSpeed=false;
    AnsiString ErrPart="", str="";
    static bool bCheckSpeed1=false;                                             //KaiChen 20171225 (Steven)：Add Speed Display
    if(bIndexPickUpErrorWaitRetry)                                              //Ifor 20171119 : add 避免Index Pick Up Err Inarm 偷跑造成資料異常導致Hangup
        return false;

    int iSht=0, iKit=0;
    if(LastSet.iRunStartMode==rsmAutoSiteMap &&
       (CosFunction.bUSEJCETSiteMapMode==true ||                                //Steven 20231113 : 修正 auto site mapping 2x6 hangup
        CUSTOMER_CODE==CC_SCS))                                                 //jou 20230626 : 修正JSCS auto site mapping 2x8 hangup
    {
        iSht=iAutoSiteMapHPToSht;
    }
    else if(LastSet.iTemperature==Tempture_Hot &&                               //Steven 20160227 : 改成HOT在上面
            LastSet.iRunStartMode!=rsmAutoSiteMap &&                            //kevin 20161124 add hot AutoSitmap hang up
            fContact->IsRun2DCheck()==false)                                    //JerryYang 20250428 : fix 2DID map
    {
        if(LastSet.iTemperature==Tempture_Hot &&
           LastSet.iRunStartMode==rsmQAMode &&
           bQAModeFinishCleanOut==true)                                         //Steven 20180601 : 修正高溫QA mode放料
        {
            iSht=InArmSuck.iWhichSht;
        }
        else
        {
            iSht=InArmSuck.iWhichShtPickFor32;
        }
    }
    else
    {
        iSht=InArmSuck.iWhichSht;
    }
    int iPickKit32=0;

    AutoSiteMappingCheckShuttle(true);                                          //Ifor 20180116 (Steven) : add Auto Site Mapping Check iWhich Shuttle
    switch(Task)
    {
        case 1:
            bDestoryOnSht=false;                                                //Steven 20170905 (wei) : 在Shuttle吹氣與資料交換的Flag
            InArmXMoveSafe=false;

            if(bPlaceToShuttle2Step==false)                                     //Steven 20160721 (Wei) : 避免放一半歸零,會Hangup
            {
                SetInArmUseSuckToHasNullIC(iSht, iKit);
            }

            /*if(IniConfig.bI37_EnableFIFOMode &&                               //Sam 20220411 : 2X6 補 FIFO
               LastSet.iRunStartMode==rsmFIFOMode)                              //JerryYang 20170729 (Steven) 修正FIFO模式 hang up
            {
            }
            else
            {
                if(InArmSuck.HasRealIC()==false)                                //ChungHung 20120105 如果剛好在Shuttle放完IC時 InArm剛好讓開會HangUp
                {
                    if(MoveInArmZToPlateSafe(Task))
                    {
                        InArmSuck.SetAll(NULL_IC);
                        fObserver->RecordInArmTime();
                        return true;
                    }
                    return false;
                }
            } */                                                                //Steven 20250223 : Mark

            if(ArmSpeed[InArm].bAutoSpeed &&
               LastSet.iTemperature!=Tempture_Hot)
                bCheckSpeed=true;
            else
                bCheckSpeed=false;

            if(IniConfig.bA26MotorSpeedSortDisplay==true &&
               LastSet.iTemperature!=Tempture_Hot)
                bCheckSpeed1=true;
            else
                bCheckSpeed1=false;

            Task=100;
        case 100:
            if(bHPCleanout)                                                     //wei 20160624 Hotplate clean out
            {
                iHPShuttle++;
                if(iHPShuttle>=2)
                    iHPShuttle=0;
            }

            if(iSht==0)
            {
                if(IniConfig.bD43IndexDropErrorCanRetryandSkip &&
                   (bShuttle1MoveToRight ||
                    bShuttle1MoveToLeft  ||                                     //Steven 20220620 : 避免index在Kit1吸取異常, In arm偷放料
                    bShuttle1HasPickErr))                                       //Steven 20230116 : 避免In arm 偷放料
                {
                    Task=1;
                    return false;
                }
                else
                {
                    if(IniConfig.bA26MotorSpeedSortDisplay==true)
                    {
                        if(InShtInLF(0))
                            iInShuttleSpeed1--;
                        else
                            iInShuttleSpeed1++;
                    }
                    Task=900;
                }
            }
            else
            {
                if(IniConfig.bD43IndexDropErrorCanRetryandSkip &&
                   (bShuttle2MoveToRight ||
                    bShuttle2MoveToLeft  ||                                     //Steven 20220620 : 避免index在Kit1吸取異常, In arm偷放料
                    bShuttle2HasPickErr))                                       //Steven 20230116 : 避免In arm 偷放料
                {
                    Task=1;
                    return false;
                }
                else
                {
                    if(IniConfig.bA26MotorSpeedSortDisplay==true)
                    {
                        if(InShtInLF(1))
                            iInShuttleSpeed1--;
                        else
                            iInShuttleSpeed1++;
                    }
                    Task=1900;
                }
            }
            goto IN_ARM_SHUTTLE_2x2_4_14;
        case 900:
            if(InSHT1InLF() &&
               FLCarryKit.UseSiteFullIC()==false)
            {
                if(bCheckSpeed)                                                 //Steven 20110525 : Auto Speed
                {
                    bCheckSpeed=false;
                    InArmAddSpeed();
                }

                if(bCheckSpeed1)
                {
                    bCheckSpeed1=false;
                    InArmAddSpeedDisplay();
                }
                Task=950;
                break;
            }
            else
            {
                if(bCheckSpeed)                                                 //Steven 20110525 : Auto Speed
                {
                    bCheckSpeed=false;
                    InArmSubSpeed();
                }

                if(bCheckSpeed1)
                {
                    bCheckSpeed1=false;
                    InArmSubSpeedDisplay();
                }
                Task=930;
            }
        case 930:
            if(MoveInArmXYToWaitTrayArm(iSht, 0, ZAxisNotDown, true)!=0)
            {
                Task=1000;
            }

            if(CheckInArmSuckICFallDownToHasNullIC(false)==true)                //Steven 20110516 : 修改成整合式Alarm
            {
                Task=935;                                                       //JerryYang 20200422 修正掉料可能檢查不出來的問題, 檢查到有掉料要重新再確認全部吸嘴
            }
            break;
        case 935:
            CheckInArmSuckICFallDownToHasNullIC();                              //JerryYang 20200422 修正掉料可能檢查不出來的問題, 檢查到有掉料要重新再確認全部吸嘴
            Task=930;
            break;
        case 950:
            SetShuttlefCanMoveL(0, false, __FUNC__, "950");
            iFlag=MoveInArmXYToWaitTrayArm(iSht, 0, ZAxisDown, true);
            if(iFlag!=0)
            {
                Task=iFlag;
            }

            if(CheckInArmSuckICFallDownToHasNullIC(false)==true)                //Steven 20110516 : 修改成整合式Alarm
            {
                Task=955;                                                       //JerryYang 20200422 修正掉料可能檢查不出來的問題, 檢查到有掉料要重新再確認全部吸嘴
            }
            break;
        case 955:
            CheckInArmSuckICFallDownToHasNullIC(true);                          //JerryYang 20200422 修正掉料可能檢查不出來的問題, 檢查到有掉料要重新再確認全部吸嘴
            if(MoveInArmZToPlateSafe(Task))
            {
                str.sprintf("Please take out the drop device from Shuttle%d", iSht+1);
                ShowMyMessage(str);
                Task=950;
            }
            break;
        case 1000:
            if(InSHT1InLF())
            {
                bCanFreeShuttle=true;                                                                                                                           //jou 2014-11-07 防止 16 site place to shuttle Z軸未上升 Shuttle 就移動撞機
                for(int i=0; i<InArmSuck.iShtRow; i++)
                {
                    for(int j=0; j<InArmSuck.iShtCol; j++)
                    {
                        if(FLCarryKit.Item[i][j]==NULL_IC)
                        {
                            bCanFreeShuttle=false;                                                                                                              //jou 2014-11-07 防止 16 site place to shuttle Z軸未上升 Shuttle 就移動撞機
                        }
                    }
                }

                if(bPlaceToShuttle2Step)                                                                                                                        //Steven 20160721 (Wei) : 避免放一半歸零,會Hangup
                    bCanFreeShuttle=false;

                if(bCanFreeShuttle==false)
                {
                    if(bShuttle1HasPickErr==false)                                                                                                              //Sam 20230103 : 修正 32 Site Pickup error hange up
                        SetShuttlefCanMoveL(0, false, __FUNC__, "1000");
                    InArmZNeedDown_9045(0, 0, true);
                    Task=900;
                    goto IN_ARM_SHUTTLE_2x2_4_14;
                }
            }
            else if(NeedWaitTrayArm && (IniConfig.bP56TrayArmWaitAtColorTrack || (INSTALL_OCR!=eocrUninstal && CosFunction.bTrayOCR)))                          //Steven 20240516 : Tray Arm等待位置改到Color        //wei 20150925 待機位置改道 Color
            {
                InitInOCRWaitTask();                                                                                                                            //wei 20170901 Place To Shuttle
                Task=1030;
            }

            if(MOT[MInShuttle1].Led[iInposLed]==false &&
               MOT[MInShuttle1].fCanMoveL==false)
            {
                bCanFreeShuttle=true;                                                                                                                           //jou 2014-11-07 防止 16 site place to shuttle Z軸未上升 Shuttle 就移動撞機
            }

            if(bCanFreeShuttle==true)                                                                                                                           //jou 2014-11-07 防止 16 site place to shuttle Z軸未上升 Shuttle 就移動撞機
            {
                Task=1050;
            }
            break;
        case 1030:
            if(OCRMoveInArm2XYToWait())                                         //wei 20170901 Place To Shuttle
            {
                Task=1;
            }
            break;
        case 1050:                                                              //jou 2014-11-07 防止 16 site place to shuttle Z軸未上升 Shuttle 就移動撞機
            if(MoveInArmZToPlateSafe(Task))
            {
                SetShuttlefCanMoveL(0, true, __FUNC__, "1050");
                Task=1000;
            }
            break;
        case 1100:
            if(IniConfig.bD43IndexDropErrorCanRetryandSkip &&
               (bShuttle1MoveToRight ||
                bShuttle1MoveToLeft  ||                                         //Steven 20220620 : 避免index在Kit1吸取異常, In arm偷放料
                bShuttle1HasPickErr))                                           //Steven 20230116 : 避免In arm 偷放料
            {
                Task=1;
                return false;
            }

            SetShuttlefCanMoveL(0, false, __FUNC__, "1100");
            if(InShtInLF(0)==false)
            {
                Task=1110;
                break;
            }

            if(CUSTOMER_CODE==CC_ASE_KaohSiung &&
               IniConfig.bF23ShuttleVibration)                                  //kevin 20210415 IN Arm Vibrate shuttle
                SW[SwShuttleVibration1].On();

            bDestoryOnSht=true;                                                 //Steven 20170905 (wei) : 在Shuttle吹氣與資料交換的Flag
            if(MoveInArmZToShuttlePlace_9045(0))
            {
                if(InArmNeedCheckOffset(true, 0))                               //Steven 20230531 : 簡化判斷式
                {
                    Task=1150;
                    break;
                }
                InArmSuck.ResetAll();                                           //Steven 20160323 : 避免未開啟真空
                Task=1200;
                if(ArmSpeed[InArm].iEnableReleaseDelay==0)                      //JerryYang 20160127 for TSMC inarm release device前delay
                {
                    InArmReleaseDelay.SetSecAndOn(ArmSpeed[InArm].dReleaseDelayTime);
                    Task=1180;
                }
            }
            break;
        case 1110:
            if(MoveInArmZToPlateSafe(Task))
            {
                bDestoryOnSht=false;                                            //Steven 20180419 (Jou) : 在Shuttle吹氣與資料交換的Flag
                SetShuttlefCanMoveL(0, true, __FUNC__, "1110");
                Task=1;
            }
            break;
        case 1150:
            if(bEnterOffset==true)
            {
                bEnterOffset=false;
                Task=1160;
            }
            else
            {
                Task=1100;
            }
            break;
        case 1160:
            if(MoveInArmZToPlateSafe(Task))
            {
                Task=1170;
            }
            break;
        case 1170:
            if(MoveInArmXYToShuttle_9045(iSht, 0, ZAxisDown, true))
            {
                Task=1100;
            }
            break;
        case 1180:
            if(InArmReleaseDelay.Off())                                         //JerryYang 20160127 for TSMC inarm release device前delay
            {
                Task=1200;
            }
            break;
        case 1200:
            flag=true;
            bPlaceToShuttle2Step=true;                                                                                  //Steven 20160721 (Wei) : 避免放一半歸零,會Hangup
            bCheckInarmPlaceShuttle=true;                                                                               //kevin 20180525 20180315 add inarm place shuttle 確認是否有抖動情形
            for(int i=0; i<2; i++)
            {
                for(int j=0; j<4; j++)
                {
                    iSuckRow=i;
                    iSuckCol=j;
                    iShtRow =i;
                    iShtCol =GetShuttleCol(i, j+iPickKit32);

                    if(InArmSuck.Item[iSuckRow][iSuckCol] &&
                       InArmSuck.Suck[iSuckRow][iSuckCol].GetNeedDestroyStatus())
                    {
                        iBackInArmHotCount=InArmSuck.HotCount;
                        if(InArmSuck.Item[iSuckRow][iSuckCol]==HAS_NULL_IC ||
                           InArmSuck.Suck[iSuckRow][iSuckCol].Destroy())
                        {
                            SetShuttleStatus_9045(iSht, iShtRow, iShtCol, iSuckRow, iSuckCol);
                        }
                        else if(InArmSuck.Suck[iSuckRow][iSuckCol].Error==false)
                        {
                            flag=false;
                        }
                    }
                }
            }

            if(flag==false)                                                                                             //kevin 20131011 在下真空誤判, 換到上面來
                break;

            ret=CheckInArmDestroyICFail();                                                                              //Steven 20111223 : 檢查破壞錯誤
            if(ret==false)
                return false;

            for(int i=0; i<2; i++)
            {
                for(int j=0; j<4; j++)
                {
                    if(InArmSuck.Item[i][j]!=NULL_IC &&
                       InArmSuck.Suck[i][j].GetNeedDestroyStatus())
                        return false;
                }
            }

            bCheckInarmPlaceShuttle=false;                                                                              //kevin 20180525 20180315 add inarm place shuttle 確認是否有抖動情形
            if(InArmSuck.HasRealIC()==false)
            {
                Task=1300;
                bPlaceToShuttle2Step=false;                                                                             //Steven 20160721 (Wei) : 避免放一半歸零,會Hangup
                fObserver->AddTimeData(2, MyInArmAtShuttleTimer.LatchCycleTime()/1000.0);                               //JerryYang 20151209
            }
            else
            {
                bPlaceToShuttle2Step=true;                                                                              //Steven 20160721 (Wei) : 避免放一半歸零,會Hangup
                Task=1250;
            }
            break;
        case 1250:
            if(MoveInArmXYToShuttle_9045(iSht, 0, ZAxisDown, true))
            {
                Task=1100;
            }

            if(CheckInArmSuckICFallDownToHasNullIC(false)==true)                //Steven 20110516 : 修改成整合式Alarm
            {
                Task=1255;                                                      //JerryYang 20200422 修正掉料可能檢查不出來的問題, 檢查到有掉料要重新再確認全部吸嘴
            }
            break;
        case 1255:
            CheckInArmSuckICFallDownToHasNullIC(true);                          //JerryYang 20200422 修正掉料可能檢查不出來的問題, 檢查到有掉料要重新再確認全部吸嘴
            if(MoveInArmZToPlateSafe(Task))
            {
                str.sprintf("Please take out the drop device from Shuttle%d", iSht+1);
                ShowMyMessage(str);
                Task=1250;
            }
            break;
        case 1300:                                                              //Steven 20220427 : 整合蝦頭搖搖敲敲功能
            InitDoInArmCheckShtFloatTask();
            Task=1400;
        case 1400:
            flag=DoInArmCheckShuttleFloating(0);
            if(flag==true)
            {
                Task=1500;
            }
            else
            {
                break;
            }
        case 1500:
            bNeedAdjust2Time=SetClosedShtKitToHasNullIC_9045(0, iKit, __FUNC__, "1500");                                //Steven 20231115 : fixed for auto site off

            if(In_Shuttle_Auto_Latch==eInSHAutoLtc &&
               bInSHLtcFin[0]==false               &&
               FLCarryKit.UseSiteFullIC())                                      //KenHsieh 20251106 : fix close site don't do In Sht Latch
            {
                Task=1400;
                break;
            }

            AdjustShtOrderWhenPlaceToSht(1);                                                                            //Steven 20180601 : 整合放完蝦頭後的調整

            if(IniConfig.bUseAutoSiteMapping &&
               LastSet.iRunStartMode==rsmAutoSiteMap &&
               (CosFunction.bUSEJCETSiteMapMode==true ||                                                                //Steven 20220913 : Fixed for ASM function
                CUSTOMER_CODE==CC_SCS))                                                                                 //jou 20230626 : 修正JSCS auto site mapping 2x8 hangup
            {
            }
            else
            {
                if(bNeedAdjust2Time)                                                                                    //Steven 20191112 : fixed for ASM close site hang up
                    AdjustShtOrderWhenPlaceToSht(1);
            }

            if(IniConfig.bIndexPickupWait==true &&
               (LastSet.iTemperature==Tempture_Hot ||                                                                   //jou 2012-06-29 Index Pick up need wait Soak Time
                LastSet.iTemperature==Tempture_AmbientHot))                                                             //kevin 20180903 (Steven) : add 恆溫控制
            {
                if(Temperature.iShuttleSoakTimeMode==1 &&
                   Temperature.iInitialStart2Time!=0)
                {
                    FLCarryKit.TSoakTime.SetSecAndOn(Temperature.iInitialStart2Time);
                    dwStartShuttle1Soak=MyTickCount();                                                                  //JerryYang 20181001 (Steven) : fix Shuttle soak time 倒數秒數異常
                    iInitialStart2Count=Temperature.iInitialStart2Time;
                }
            }
            fObserver->RecordInArmTime();
            bDestoryOnSht=false;                                                                                        //Steven 20170905 (wei) : 在Shuttle吹氣與資料交換的Flag
            iInArmPickPlaceCnt[InOfsInSh1]++;                                                                           //JerryYang 20180921 Setup Teach功能
            if(USE_IN_Y_IS_AUTO_PITCH==true &&                                                                          //JerryYang 20251218 : IN/OUT ARM支援不同模組
               IniConfig.bE57YPitchHome)                                                                                //kevin 20180822 (Steven) : add put shuttle Y pitch home AutoYPitch)
            {
                 hInArmYpitchHomeTimer.SetSecAndOn(10);
                 Task=1600;
                 return false;
            }
            return true;
       case 1600:
            if(bCheckYPitchRunHomeSen(0))
            {
                return true;
            }
            else if(hInArmYpitchHomeTimer.Off())
            {
                InitProcessSingleMotorTask(MInArmPitchY);
                hInArmYpitchHomeTimer.SetSecAndOn(10);
                bYpitchNeddHome=true;
                Task=1610;
            }
            break;
       case 1610:
            if(bCheckYPitchHome(0))
            {
                bYpitchNeddHome=false;
                SetMotorSpeed();
                return true;
            }
            else if(hInArmYpitchHomeTimer.Off())
            {
                ShowErrorMessage("WAR0123", K_RETRY, MInArmPitchY);
                Task=1600;
            }
            break;
        //---------------------
        //place shuttle1 finish
        //---------------------
        case 1900:
            if(InSHT2InLF() &&
               BLCarryKit.UseSiteFullIC()==false)
            {
                if(bCheckSpeed)                                                                                                                                 //Steven 20110525 : Auto Speed
                {
                    bCheckSpeed=false;
                    InArmAddSpeed();
                }

                if(bCheckSpeed1)
                {
                    bCheckSpeed1=false;
                    InArmAddSpeedDisplay();
                }
                Task=1950;
                break;
            }
            else
            {
                Task=1910;
            }
            break;
        case 1910:
            if(bCheckSpeed)                                                                                                                                     //Steven 20110525 : Auto Speed
            {
                bCheckSpeed=false;
                InArmSubSpeed();
            }

            if(bCheckSpeed1)
            {
                bCheckSpeed1=false;
                InArmSubSpeedDisplay();
            }
            Task=1930;
        case 1930:
            if(MoveInArmXYToWaitTrayArm(iSht, 0, ZAxisNotDown, true)!=0)
            {
                Task=2000;
            }

            if(CheckInArmSuckICFallDownToHasNullIC(false)==true)                                                                                                //Steven 20110516 : 修改成整合式Alarm
            {
                Task=1935;                                                                                                                                      //JerryYang 20200422 修正掉料可能檢查不出來的問題, 檢查到有掉料要重新再確認全部吸嘴
            }
            break;
        case 1935:
            CheckInArmSuckICFallDownToHasNullIC();                                                                                                              //JerryYang 20200422 修正掉料可能檢查不出來的問題, 檢查到有掉料要重新再確認全部吸嘴
            Task=1930;
            break;
        case 1950:
            SetShuttlefCanMoveL(1, false, __FUNC__, "1950");
            if(MoveInArmXYToShuttle_9045(iSht, 0, ZAxisDown, true))
            {
                Task=2100;
            }

            if(CheckInArmSuckICFallDownToHasNullIC(false)==true)                                                                                                //Steven 20110516 : 修改成整合式Alarm
            {
                Task=1955;                                                                                                                                      //JerryYang 20200422 修正掉料可能檢查不出來的問題, 檢查到有掉料要重新再確認全部吸嘴
            }
            break;
        case 1955:
            CheckInArmSuckICFallDownToHasNullIC(true);                                                                                                          //JerryYang 20200422 修正掉料可能檢查不出來的問題, 檢查到有掉料要重新再確認全部吸嘴
            if(MoveInArmZToPlateSafe(Task))
            {
                str.sprintf("Please take out the drop device from Shuttle%d", iSht+1);
                ShowMyMessage(str);
                Task=1950;
            }
            break;
        case 2000:
            if(InSHT2InLF())
            {
                bCanFreeShuttle=true;                                                                                                                           //jou 2014-11-07 防止 16 site place to shuttle Z軸未上升 Shuttle 就移動撞機
                for(int i=0; i<InArmSuck.iShtRow; i++)
                {
                    for(int j=0; j<InArmSuck.iShtCol; j++)
                    {
                        if(BLCarryKit.Item[i][j]==NULL_IC)
                        {
                            bCanFreeShuttle=false;                                                                                                              //jou 2014-11-07 防止 16 site place to shuttle Z軸未上升 Shuttle 就移動撞機
                        }
                    }
                }

                if(bPlaceToShuttle2Step)                                                                                                                        //Steven 20160721 (Wei) : 避免放一半歸零,會Hangup
                    bCanFreeShuttle=false;

                if(bCanFreeShuttle==false)
                {
                    if(bShuttle2HasPickErr==false)                                                                                                              //Sam 20230103 : 修正 32 Site Pickup error hange up
                        SetShuttlefCanMoveL(1, false, __FUNC__, "2000");
                    InArmZNeedDown_9045(1, 0, true);
                    Task=1900;
                    goto IN_ARM_SHUTTLE_2x2_4_14;
                }
            }
            else if(NeedWaitTrayArm && (IniConfig.bP56TrayArmWaitAtColorTrack || (INSTALL_OCR!=eocrUninstal && CosFunction.bTrayOCR)))                          //Steven 20240516 : Tray Arm等待位置改到Color        //wei 20150925 待機位置改道 Color
            {
                InitInOCRWaitTask();                                                                                                                            //wei 20170901 Place To Shuttle
                Task=2030;
            }

            if(MOT[MInShuttle2].Led[iInposLed]==false &&
               MOT[MInShuttle2].fCanMoveL==false)
            {
                bCanFreeShuttle=true;
            }

            if(bCanFreeShuttle==true)                                                                                                                           //jou 2014-11-07 防止 16 site place to shuttle Z軸未上升 Shuttle 就移動撞機
            {
                Task=2050;
            }
            break;
        case 2030:
            if(OCRMoveInArm2XYToWait())                                                                                                                         //wei 20170901 Place To Shuttle
            {
                Task=1;
            }
            break;
        case 2050:                                                                                                                                              //jou 2014-11-07 防止 16 site place to shuttle Z軸未上升 Shuttle 就移動撞機
            if(MoveInArmZToPlateSafe(Task))
            {
                SetShuttlefCanMoveL(1, true, __FUNC__, "2050");
                Task=2000;
            }
            break;
        case 2100:
            if(IniConfig.bD43IndexDropErrorCanRetryandSkip &&
               (bShuttle2MoveToRight ||
                bShuttle2MoveToLeft  ||                                                                                                                         //Steven 20220620 : 避免index在Kit1吸取異常, In arm偷放料
                bShuttle2HasPickErr))                                                                                                                           //Steven 20230116 : 避免In arm 偷放料
            {
                Task=1;
                return false;
            }

            SetShuttlefCanMoveL(1, false, __FUNC__, "2100");
            if(InShtInLF(1)==false)
            {
                Task=2110;
                break;
            }

            if(CUSTOMER_CODE==CC_ASE_KaohSiung &&
               IniConfig.bF23ShuttleVibration)                                                                                                                  //kevin 20210415 IN Arm Vibrate shuttle
                SW[SwShuttleVibration2].On();

            bDestoryOnSht=true;                                                                                                                                 //Steven 20170905 (wei) : 在Shuttle吹氣與資料交換的Flag
            if(MoveInArmZToShuttlePlace_9045(1))
            {
                if(InArmNeedCheckOffset(true, 1))                                                                                                               //Steven 20230531 : 簡化判斷式
                {
                    Task=2150;
                    break;
                }

                InArmSuck.ResetAll();                                                                                                                           //Steven 20160323 : 避免未開啟真空
                Task=2200;
                if(ArmSpeed[InArm].iEnableReleaseDelay==0)                                                                                                      //JerryYang 20160127 for TSMC inarm release device前delay
                {
                    InArmReleaseDelay.SetSecAndOn(ArmSpeed[InArm].dReleaseDelayTime);
                    Task=2180;
                }
            }
            break;
        case 2110:
            if(MoveInArmZToPlateSafe(Task))
            {
                bDestoryOnSht=false;                                                                                                                            //Steven 20180419 (Jou) : 在Shuttle吹氣與資料交換的Flag
                SetShuttlefCanMoveL(1, true, __FUNC__, "2110");
                Task=1;
            }
            break;
        case 2150:
            if(bEnterOffset==true)
            {
                bEnterOffset=false;
                Task=2160;
            }
            else
            {
                Task=2100;
            }
            break;
        case 2160:
            if(MoveInArmZToPlateSafe(Task))
            {
                Task=2170;
            }
            break;
        case 2170:
            if(MoveInArmXYToShuttle_9045(iSht, 0, ZAxisDown, true))
            {
                Task=2100;
            }
            break;
        case 2180:
            if(InArmReleaseDelay.Off())                                                                                                                         //JerryYang 20160127 for TSMC inarm release device前delay
            {
                Task=2200;
            }
            break;
        case 2200:
            flag=true;
            bPlaceToShuttle2Step=true;                                                                                                                          //Steven 20160721 (Wei) : 避免放一半歸零,會Hangup
            for(int i=0; i<2; i++)
            {
                for(int j=0; j<4; j++)
                {
                    iSuckRow=i;
                    iSuckCol=j;
                    iShtRow =i;
                    iShtCol =GetShuttleCol(i, j+iPickKit32);

                    if(InArmSuck.Item[iSuckRow][iSuckCol] &&
                       InArmSuck.Suck[iSuckRow][iSuckCol].GetNeedDestroyStatus())
                    {
                        iBackInArmHotCount=InArmSuck.HotCount;
                        if(InArmSuck.Item[iSuckRow][iSuckCol]==HAS_NULL_IC ||
                           InArmSuck.Suck[iSuckRow][iSuckCol].Destroy())
                        {
                            SetShuttleStatus_9045(iSht, iShtRow, iShtCol, iSuckRow, iSuckCol);
                        }
                        else if(InArmSuck.Suck[iSuckRow][iSuckCol].Error==false)
                        {
                            flag=false;
                        }
                    }
                }
            }

            if(flag==false)                                                                                                                                     //kevin 20131011 在下真空誤判, 換到上面來
                break;

            ret=CheckInArmDestroyICFail();                                                                                                                      //Steven 20111223 : 檢查破壞錯誤
            if(ret==false)
                return false;

            for(int i=0; i<2; i++)
            {
                for(int j=0; j<4; j++)
                {
                    if(InArmSuck.Item[i][j]!=NULL_IC &&
                       InArmSuck.Suck[i][j].GetNeedDestroyStatus())
                        return false;
                }
            }

            if(InArmSuck.HasRealIC()==false)
            {
                Task=2300;
                bPlaceToShuttle2Step=false;                                                                                                                     //Steven 20160721 (Wei) : 避免放一半歸零,會Hangup
                fObserver->AddTimeData(2, MyInArmAtShuttleTimer.LatchCycleTime()/1000.0);                                                                       //JerryYang 20151209
            }
            else
            {
                bPlaceToShuttle2Step=true;                                                                                                                      //Steven 20160721 (Wei) : 避免放一半歸零,會Hangup
                Task=2250;
            }
            break;
        case 2250:
            if(MoveInArmXYToShuttle_9045(iSht, 0, ZAxisDown, true))
            {
                Task=2100;
            }

            if(CheckInArmSuckICFallDownToHasNullIC(false)==true)                                                                                                //Steven 20110516 : 修改成整合式Alarm
            {
                Task=2255;                                                                                                                                      //JerryYang 20200422 修正掉料可能檢查不出來的問題, 檢查到有掉料要重新再確認全部吸嘴
            }
            break;
        case 2255:
            CheckInArmSuckICFallDownToHasNullIC(true);                                                                                                          //JerryYang 20200422 修正掉料可能檢查不出來的問題, 檢查到有掉料要重新再確認全部吸嘴
            if(MoveInArmZToPlateSafe(Task))
            {
                str.sprintf("Please take out the drop device from Shuttle%d", iSht+1);
                ShowMyMessage(str);
                Task=2250;
            }
            break;
        case 2300:                                                                                                                                              //Steven 20220427 : 整合蝦頭搖搖敲敲功能
            InitDoInArmCheckShtFloatTask();
            Task=2400;
        case 2400:
            flag=DoInArmCheckShuttleFloating(1);
            if(flag==true)
            {
                Task=2500;
            }
            else
            {
                break;
            }
        case 2500:
            bNeedAdjust2Time=SetClosedShtKitToHasNullIC_9045(1, iKit, __FUNC__, "2500");                                                                        //Steven 20231115 : fixed for auto site off

            if(In_Shuttle_Auto_Latch==eInSHAutoLtc &&
               bInSHLtcFin[1]==false               &&
               BLCarryKit.UseSiteFullIC())                                      //KenHsieh 20251106 : fix close site don't do In Sht Latch
            {
                Task=2400;
                break;
            }

            AdjustShtOrderWhenPlaceToSht(2);                                                                                                                    //Steven 20180601 : 整合放完蝦頭後的調整

            if(IniConfig.bUseAutoSiteMapping &&
               LastSet.iRunStartMode==rsmAutoSiteMap &&
               (CosFunction.bUSEJCETSiteMapMode==true ||                                                                                                        //Steven 20220913 : Fixed for ASM function
                CUSTOMER_CODE==CC_SCS))                                                                                                                         //jou 20230626 : 修正JSCS auto site mapping 2x8 hangup
            {
            }
            else
            {
                if(bNeedAdjust2Time)                                                                                                                            //Steven 20191112 : fixed for ASM close site hang up
                    AdjustShtOrderWhenPlaceToSht(2);
            }

            if(IniConfig.bIndexPickupWait==true &&
               (LastSet.iTemperature==Tempture_Hot ||                                                                                                           //jou 2012-06-29 Index Pick up need wait Soak Time
               LastSet.iTemperature==Tempture_AmbientHot))                                                                                                      //kevin 20180903 (Steven) : add 恆溫控制
            {
                if(Temperature.iShuttleSoakTimeMode==1 &&
                   Temperature.iInitialStart2Time!=0)
                {
                    BLCarryKit.TSoakTime.SetSecAndOn(Temperature.iInitialStart2Time);
                    dwStartShuttle2Soak=MyTickCount();                                                                                                          //JerryYang 20181001 (Steven) : fix Shuttle soak time 倒數秒數異常
                    iInitialStart2Count=Temperature.iInitialStart2Time;
                }
            }
            fObserver->RecordInArmTime();
            bDestoryOnSht=false;                                                                                                                                //Steven 20170905 (wei) : 在Shuttle吹氣與資料交換的Flag
            iInArmPickPlaceCnt[InOfsInSh2]++;                                                                                                                   //JerryYang 20180921 Setup Teach功能
            if(USE_IN_Y_IS_AUTO_PITCH==true &&                                                                                                                  //JerryYang 20251218 : IN/OUT ARM支援不同模組
               IniConfig.bE57YPitchHome)                                                                                                                        //kevin 20180822 (Steven) : add put shuttle Y pitch home AutoYPitch
            {
                 hInArmYpitchHomeTimer.SetSecAndOn(10);
                 Task=2600;
                 return false;
            }
            return true;
       case 2600:
            if(bCheckYPitchRunHomeSen(0))
            {
                return true;
            }
            else if(hInArmYpitchHomeTimer.Off())
            {
                InitProcessSingleMotorTask(MInArmPitchY);
                hInArmYpitchHomeTimer.SetSecAndOn(10);
                bYpitchNeddHome=true;
                Task=2610;
            }
            break;
       case 2610:
            if(bCheckYPitchHome(0))
            {
                bYpitchNeddHome=false;
                SetMotorSpeed();
                return true;
            }
            else if(hInArmYpitchHomeTimer.Off())
            {
                ShowErrorMessage("WAR0123", K_RETRY, MInArmPitchY);
                Task=2600;
            }
            break;
    }
    return false;
}
//==============================================================================
// golden ainarm9045_2x2_4_14.cpp:1562 -- engine dispatch symbol (SuckerMap)
void DoInArm_9045_2x2_4_14_SuckerMap()
{
    for(int i=0; i<2; i++)
    {
        for(int j=0; j<8; j++)
        {
            int iShtCol=GetShuttleCol(i, j);
            if(iShtCol<InArmSuck.iShtCol)
            {
                Prod.fInArmSuck4x8[0][i][j]=LastSet.bUseTestSocket[0][i][iShtCol];
                Prod.fInArmSuck4x8[1][i][j]=LastSet.bUseTestSocket[1][i][iShtCol];
                Prod.iSiteMap[0][i][j]     =TestIF.iSiteMap[i][iShtCol];
                Prod.iSiteMap[1][i][j]     =TestIF.iSiteMap[i][iShtCol];
            }
            else
            {
                Prod.fInArmSuck4x8[0][i][j]=false;
                Prod.fInArmSuck4x8[1][i][j]=false;
            }

            if(iShtCol<InArmSuck.iShtCol)
            {
                Prod.bInSuckUse[0][i][j]=true;
                Prod.bInSuckUse[1][i][j]=true;
            }
        }
    }
}
//==============================================================================
// golden ainarm9045_2x2_4_14.cpp:1591 -- engine dispatch symbol (main SM)
void DoInArm_9045_2x2_4_14()
{
    int &Task=iArmTask;
    static int iAdjustSpeed=0;
    static bool bOne=false;
    static bool bFirstTimeShuttle1States=false;                                 //ChungHung 20130910 alter for SCK can close site by Index process
    bool flag=false;
    static int iDisplaySpeed=0;                                                 //KaiChen 20171225 (Steven)：Add Speed Display

    bool bCheckCloseSiteHaveIC=false;                                           //kevin 20130115
    AnsiString ErrPart="";
    bool bHasDuplicateErr=false;                                                //kevin 20130115
//    DoJudgeInputShuttleNeedChangeToNullIC();

    if(DoInArmAutoSiteMapping() ||                                              //jou 2011-03-24 start : Auto Site Mapping
       bIndexAlarmInArmAway)                                                    //kevin 20181102 (Steven) :  index回吸檢測有IC inarm 讓位 inarm先不要動
    {
        return ;                                                                //jou 2016-11-03 JCET 要求Auto Site Mapping 需等待測試結果在繼續入料
    }

    int iShtHP, iKitHP;
    int iSht  =InArmSuck.iWhichSht;
    int iKit  =0;
    int iKit32=(iKit==1)?4:0;

    if(LastSet.iTemperature==Tempture_Hot)                                      //Steven 20150626 : Fixed 32 site 吸取偵測異常    //Steven 20180525 : 修正一直跑Hotplate iWhichKit Close
    {
        iShtHP=InArmSuck.iWhichShtPickFor32;
        iKitHP=0;
    }
    else
    {
        iShtHP=InArmSuck.iWhichSht;
        iKitHP=0;
    }
    int iKitHP32=(iKitHP==1)?4:0;

    switch(Task)
    {
        case 1:
            if(MoveInArm2XYToWait())
            {
                if((iOneCycle && IsInArmOneCycleFinish()) ||
                   (iCleanOut && IsInArmCleanOutFinish()))
                {
                    if(LastSet.iTemperature==Tempture_Hot &&
                       CosFunction.bUseInitialDelayAsSoakTime &&                //Steven 20170511 (wei) : 使用initial delay當 Soak time
                       Temperature.bUseInitialDelayAsSoakTime &&
                       MOT[MMPlate1].HasIC()==false &&
                       MOT[MMPlate2].HasIC()==false)
                    {
                        bOneTimeWait=false;
                    }

                    SetShuttlefCanMoveL(0, true, __FUNC__, "1");
                    SetShuttlefCanMoveL(1, true, __FUNC__, "1");
                    bInArmWaitOneCycle=true;                                    //JerryYang 20220324 : Inarm是否完成ONE CYCLE
                }
                else
                {
                    bInArmWaitOneCycle=false;                                   //JerryYang 20220324 : Inarm是否完成ONE CYCLE
                }
                Task=10;
                bOne=true;
                bFirstTimeShuttle1States=true;                                  //ChungHung 20130910 alter for SCK can close site by Index process
            }

            if(Task!=10)                                                        //Steven 20180813 : add in arm speed
                break;
        case 10:
            if(CheckInArmSuckInitial()==false || bIndexDropVacuumError)         //kevin 20190418 避免 inarm 來回跑
                break;

            if(InArmSuck.HasIC() || bPlaceToHotplate==true)                     //ChungHung 20131010 fix Hotplate 跨排Hangup
            {
                if(bPlaceToHotplate==false &&
                   CheekNeedToDoInArmAdditionalFunction())                      //Steven 20210609 : 整合Precisor, Rotator, Bottom CCD, Die Clean
                {
                    Task=200;
                }
                else if(InArmSuck.HasType(HAS_TRY_SUCK_IC) ||                   //ChungHung 20120206 Hotplate check
                        InArmSuck.HasType(HAS_SUCK_IC))
                {
                    if(bInArmTryPickFromHotPlateFinish==false)
                    {
                        InitInArmTryPickFromHotPlateTask100();
                    }
                    else
                    {
                        InitInArmTryPickFromHotPlateTask();
                    }
                    Task=15000;
                }
                else if(InArmSuck.HasType(HAS_NULL_IC) &&
                        InArmSuck.HasType(NULL_IC) &&
                        bPickFromHotplate==true)
                {
                    iHeaterWaitTime=0;
                    if(bPickFormHotplatePartOK==true)
                    {
                        InitInArmPickFromHotPlateTask340();                     //ChungHung 20120502 Hang Up 解除
                    }
                    else
                    {
                        InitInArmPickFromHotPlateTask50();                      //ChungHung 20120112 Hang Up 解除
                    }
                    Task=1500;
                }
                else if(LastSet.iTemperature==Tempture_Ambient ||
                        LastSet.iTemperature==Tempture_AmbientHot ||            //kevin 20141104 恆溫不放hotplate
                        InArmSuck.HasType(HAS_HOT_IC))
                {
                    if(InArmSuck.HasType(HAS_HOT_IC) &&
                       bPickFromHotplate)
                    {
                        iHeaterWaitTime=0;
                        if(bPickFormHotplatePartOK==true)
                        {
                            InitInArmPickFromHotPlateTask340();                 //ChungHung 20120502 Hang Up 解除
                        }
                        else
                        {
                            InitInArmPickFromHotPlateTask50();                  //ChungHung 20120112 Hang Up 解除
                        }
                        Task=1500;
                    }
                    else
                    {
                        InitInArmPlaceToShuttleTask();
                        Task=2000;
                    }
                }
                else
                {
                    if(bPlaceToHotplate==false)
                    {
                        if(HotPlateForm.XDivision==6)
                        {
                            iForPlaceHPX6Step=0;
                        }
                        InitInArmPlaceToHotPlateTask();
                        Task=400;                                               //Steven 20250107 : fixed for hot mode tray end hang up
                    }
                    else
                    {
                        if(bPlaceToHotplatePartOK==true)                        //ChungHung 20120109 add 6x11 HangeUp
                        {
                            InitInArmPlaceToHotPlateTask400();                  //ChungHung 20120502 HangUp 解除
                        }
                        else
                        {
                            InitInArmPlaceToHotPlateTask100();
                        }

                        if(InArmSuck.Item[0][0]==NULL_IC && InArmSuck.Item[0][InArmSuck.iPickStep]==NULL_IC &&
                           InArmSuck.Item[1][0]==NULL_IC && InArmSuck.Item[1][InArmSuck.iPickStep]==NULL_IC)
                            Task=50;                                            //Steven 20250107 : Move up
                        else
                            Task=1100;
                    }
                }
            }
            else
            {
                if(CheekNeedToDoInArmAdditionalFunction())                      //Steven 20210609 : 整合Precisor, Rotator, Bottom CCD, Die Clean
                {
                    Task=200;
                }
                else
                {
                    bPlaceToHotplate=false;
                    bPickFromHotplate=false;
                    Task=50;
                }
            }

            if(Task!=50)
                break;
        case 50:
            if(CheckHeaterOK()==false)                                          //Steven 20250116 : 確認HeaterOK
            {
                Task=1;
                break;
            }

            InitArmPickFromLoadStageTask();
            Task=75;                                                            //ChungHung 20120206 Hotplate check

            if(INSTALL_OCR!=eocrUninstal && CosFunction.bTrayOCR)
            {
                if(NeedWaitTrayArm==true)                                       //wei 20151002
                {
                    Task=1;                                                     //jou 2016-07-14 修正OCR In arm 讓位造成Hotplate疊料issue
                    return;
                }
            }

            if(CosFunction.bRTCAutoModelVerify==true &&                         //jou 2014-06-24 RTC 自動進行Model驗證
               IniConfig.bD36EnableRTCAutoModelVerify==true)
            {
                if(bNeedWaitRTCAutoVerify==true)
                {
                    Task=1;
                    return;
                }
            }

            if(LastSet.iTemperature==Tempture_Hot &&
               (IniConfig.bE39CheckHotPlateAfterCleanOutAndBeforeTrayFeed==true ||
                IniConfig.bO01_ResetNeedClearAndCheckHP) &&                     //ChungHung 20120206 Hotplate check
               bCanUseHotPlateCheck==true &&
               (bNeedTrySuckHotPlate==true || bHPCleanout==true) &&             //wei 20160624 Hotplate clean out
               bOneTimeHotPlateCheckAll==true)                                  //Steven 20120319 : Clean Out後的檢查
            {
                ;
            }
            else
            {
                if(MOT[MMTrayY].HasIC()==false             &&                   // load and load buffer no any tray
                   MOT[MMTrayY].fHasTray==false            &&
                   MOT[MMTrayY_Car].Tray.HasIC()==false    &&
                   MOT[MMTrayY_Car].fHasTray==false)
                {
                    if(iCleanOut && iOneCycle==0 &&                             //ChungHung 20110829 add
                       LastSet.iTemperature==Tempture_Hot &&
                       (MOT[MMPlate1].HasIC() || MOT[MMPlate2].HasIC()))
                    {
                        DisableAutoSiteMapWhenCleanOut();                       //Steven 20220902 : 修正Auto Site Map, Loader沒有IC
                        Task=500;
                        break;
                    }
                }

                if(IniConfig.bI37_EnableFIFOMode &&
                   LastSet.iRunStartMode==rsmFIFOMode)                          //Sam 20230830 : 修正FIFO跨盤時排序問題
                {
                    if(MOT[MMTrayY].HasIC()==false)
                    {
                        if(InArmSuck.HasIC())
                        {
                            if((iSht==0 && FLCarryKit.UseSiteFullIC()==false) ||
                                iSht==1 && BLCarryKit.UseSiteFullIC()==false)
                            {
                                SetInArmUseSuckToHasNullIC(iSht, iKit);
                                Task=1;
                                break;
                            }
                        }
                    }
                }
            }

            if((MOT[MMTrayY].HasIC()==false &&
                MOT[MMTrayY].fHasTray==true) || bNeedSlapTray)
            {
                Task=1;
                break;
            }

            if(iOneCycle &&                                                     //JerryYang 20250726 : fix clean out in arm loop
               IsInArmOneCycleFinish() &&
               InArmSuck.HasIC()==false)
            {
            }
            else if(iCleanOut &&
                    LastSet.iTemperature==Tempture_Hot &&
                    IsInArmCleanOutFinish()==false)                             //Steven 20230309 : fixed for NN mode hang up
            {
                if(bRunAutoSiteMapping==true &&                                 //Ifor 20210426 add: Auto Site Mapping Clean Out 補料回Hot plate
                   CheckHasSpaceToPlace_9045())
                {
                    if(MOT[MMTrayY].HasIC() || MOT[MMTrayY_Car].fHasTray)
                    {
                        Task=75;
                        break;
                    }
                }

                if((TrayForm.bAutoFeed ||
                    bMustCleanAllTray) &&
                   CheckHasSpaceToPlace_9045())
                {
                    if(MOT[MMTrayY].HasIC() || MOT[MMTrayY_Car].fHasTray)
                    {
                        Task=75;                                                //ChungHung 20120206 Hotplate check
                        break;
                    }
                }

                if((MOT[MMPlate1].HasIC() ||
                    MOT[MMPlate2].HasIC()) &&
                   HasHotReadyIC_9045())
                {
                    Task=500;
                    break;
                }

                if(TrayForm.bAutoFeed==false &&                                 //jou 2010-08-31 start : 2x8 剛好只吸到奇數時,hang up
                   MOT[MMPlate1].HasIC()==false &&
                   MOT[MMPlate2].HasIC()==false &&
                   InArmSuck.HasIC()==false &&
                   (MOT[MInShuttle1].fCanMoveL==false ||
                    MOT[MInShuttle2].fCanMoveL==false))
                {
                    Task=1;
                    break;
                }

                if(TrayForm.bAutoFeed ||
                   (MOT[MMTrayY].fHasTray==false &&
                    MOT[MMTrayY_Car].fHasTray==false) ||
                   bMustCleanAllTray)
                {
                    if(CheckHasSpaceToPlace_9045())
                        Task=75;                                                //ChungHung 20120206 Hotplate check
                    else
                        Task=500;
                }
                else
                {
                    Task=500;
                }
                break;
            }

            if(LastSet.iTemperature==Tempture_Hot &&                            //Ifor 20180507 : add 避免Auto Site Mapping 未補料直接取IC造成資料異常
               CheckHasSpaceToPlace_9045()==false)
            {
                if(LastSet.iRunStartMode==rsmAutoSiteMap)
                {
                    if(bAutoSiteMapHotplateReady==true &&
                       bAutoSiteMapHotplateSave==false)
                    {
                        Task=500;
                    }
                }
                else
                {
                    Task=500;
                }
            }

            if(LastSet.iTemperature==Tempture_Hot &&
               iAdjustSpeed==1 &&
               ArmSpeed[InArm].bAutoSpeed && Task!=500)
            {
                iAdjustSpeed=2;
            }

            if(LastSet.iTemperature==Tempture_Hot &&
               iDisplaySpeed==1 &&
               IniConfig.bA26MotorSpeedSortDisplay==true)
            {
                iDisplaySpeed=2;
                InArmAddSpeedDisplay();
            }

            if((iOneCycle && IsInArmOneCycleFinish()) ||
               (iCleanOut && IsInArmCleanOutFinish()) &&
               InArmSuck.HasIC()==false)
            {
                bInArmWaitOneCycle=true;                                        //JerryYang 20220324 : Inarm是否完成ONE CYCLE
                Task=CheckOneCycleAction(50);                                   //Steven 20240326 : 判斷one cycle的時候要不要繼續放料
                if(Task==100 || Task==500)
                    break;

                if(MoveInArm2XYToWait()==false)
                {
                    Task=50;
                    return;
                }

                bInArmWaitOneCycle=true;                                        //JerryYang 20220324 : Inarm是否完成ONE CYCLE
                if(bOne)
                {
                    bOne=false;

                    SetShuttlefCanMoveL(0, true, __FUNC__, "50");
                    SetShuttlefCanMoveL(1, true, __FUNC__, "50");
                }
                Task=50;
                break;                                                          //JerryYang 20220324 : add
            }
            bInArmWaitOneCycle=false;                                           //JerryYang 20220324 : Inarm是否完成ONE CYCLE

            if(Task==75 &&
               LastSet.iTemperature==Tempture_Hot &&
               CheckHasSpaceToPlace_9045()==false &&
               LastSet.iRunStartMode!=rsmAutoSiteMap)                           //jou 2016-11-07 JCET fix Auto Site mapping Hot mode hangup
            {
                Task=50;
            }

            if(bBackupCleanOut==true && iOneCycle==1  &&                        //Steven 20240709 : One Cycle沒tray Hang up
               MOT[MMTrayY].HasIC()==false            &&
               MOT[MMTrayY].fHasTray==false           &&
               MOT[MMTrayY_Car].Tray.HasIC()==false   &&
               MOT[MMTrayY_Car].fHasTray==false)
            {
                if(LastSet.iTemperature==Tempture_Hot)
                {
                    Task=CheckOneCycleAction(50);                               //Steven 20240326 : 判斷one cycle的時候要不要繼續放料
                    if(Task==100 || Task==500)
                        break;
                }
            }

            if(Task!=75)                                                        //Steven 20180813 : add in arm speed
                break;
        case 75:                                                                //ChungHung 20120206 Hotplate check start
            if(LastSet.iTemperature==Tempture_Hot &&                            //Steven 20170718 (wei) : HP Check加上保護
               bNeedTrySuckHotPlate &&
               bAutoSiteMapHasPickHP==false)                                    //jou 20231018 : 修正E38與autositemapping衝突造成hotplate疊料問題
            {
                SetInArmUseSuckToHasTrySuckIC(iSht, iKit);
                InitInArmTryPickFromHotPlateTask();
                Task=15000;
            }
            else
            {
                TryHotClearRecBufMMPlate();                                     //ChungHung 20120206 Hotplate check
                InitArmPickFromLoadStageTask();
                Task=100;
            }

            if(Task!=100)                                                       //Steven 20180813 : add in arm speed
                break;
        case 100:
            if(CheckHeaterOK()==false)                                          //Steven 20250116 : 確認HeaterOK
            {
                if(iPickFromLoadStageTask==1)
                {
                    Task=1;
                    break;
                }
            }

            if(CUSTOMER_CODE==CC_ASE_M &&                                       //Ifor 20190529 : add 避免 ASEM 常溫Site Mapoing 完成後多入料一次
               LastSet.iRunStartMode==rsmAutoSiteMap &&
               LastSet.iTemperature!=Tempture_Hot &&
               bASMFinishOneCycle==true && iOneCycle==1)
            {
                Task=1;
                break;
            }

            if(USE_TRAY_MAPPING==etmInstall &&
               TestIF_File.bEnableDeviceRemain==true &&                         //Sam 20191113 : 防止殘料檢與 PickFormLoad 打架
               bDoTrayDeviceCheck)
            {
                break;
            }

            flag=DoInArmPickFromLoadStage_9045();
            if(flag)
            {
                iInArmPickPlaceCnt[InOfsLoader]++;                              //JerryYang 20180921 Setup Teach功能
                bHangTimePause=true;                                            //Steven 20090827 : Hang Up dectector
                if(InArmSuck.HasIC()==false)
                {
                    if(InArmSideAllClose(iSht))                                 //Steven 20230331 : 避免關Site死雞
                    {
                        AdjustShuttleWhichKitOrder();
                    }
                    Task=50;
                }
                else
                {
                    bLoaderNeedVibrate=true;                                    //JerryYang 20191002 loader震動馬達
                    Task=200;
                }
            }

            if(Task!=200)                                                       //Steven 20180813 : add in arm speed
                break;
        case 200:                                                               //kevin 20120115 判斷關site是否吸ic
            CheekNeedToDoInArmAdditionalFunction();                             //Steven 20210609 : 整合Precisor, Rotator, Bottom CCD, Die Clean

            for(int i=0; i<2; i++)
            {
                for(int j=0; j<4; j++)
                {
                    if(Prod.fInArmSuck4x8[iSht][i][j+iKit32]==false &&
                       InArmSuck.Item[i][j]==HAS_IC)
                    {
                        Task=300;
                        return;
                    }
                }
            }
            Task=400;
            break;
        case 300:
            ErrPart=" ";
            bHasDuplicateErr=false;
            bCheckCloseSiteHaveIC=false;                                        //kevin 20130115

            for(int i=0; i<2; i++)
            {
                for(int j=0; j<4; j++)
                {
                    if(Prod.fInArmSuck4x8[iSht][i][j+iKit32]==false &&
                       InArmSuck.Suck[i][j].GetStatus())
                    {
                        bCheckCloseSiteHaveIC=true;
                        ErrPart+=InArmSuck.Suck[i][j].sName;
                        InArmSuck.SetItemData(i, j, HAS_NULL_IC);
                    }
                }
            }

            if(bCheckCloseSiteHaveIC)
            {
                ShowErrorMessage("JAM0114", K_RETRY, MInArmX, bHasDuplicateErr, ErrPart);
                bHasDuplicateErr=true;
                break;
            }
            else
            {
                Task=400;
            }

            if(Task!=400)
                break;
        case 400:                                                               //kevin 20130115 判斷關site是否吸ic
            flag=DoInArmAdditionalFunction();                                   //Steven 20210609 : 整合Precisor, Rotator, Bottom CCD, Die Clean
            if(flag==true)
            {
                SetInArmUseSuckToHasNullIC(iSht, iKit);
                InitInArmPlaceToHotPlateTask();
                InitInArmPlaceToShuttleTask();
                if(LastSet.iTemperature==Tempture_Hot)
                {
                    Task=1000;                                                  // place to hot plate
                }
                else
                {
                    Task=2000;                                                  // place to shuttle
                }

                if(IniConfig.bUseAutoSiteMapping)                               //jou 2011-03-24 start : Auto Site Mapping
                {
                    if(LastSet.iTemperature==Tempture_Hot &&
                       LastSet.iRunStartMode==rsmAutoSiteMap &&
                       bSiteMappingCHKOK==false)
                    {
                        bOneTimeWait=false;
                        if(IniConfig.bI21AutoSiteMappingUseHotplate==false)     //Ifor 20170919 (Steven) : add Auto Site Mapping Hotplate Mode
                            Task=2000;
                        else
                            Task=1000;
                    }
                }
            }
            break;
        case 500:
            if(iCleanOut)
            {
                if(MOT[MMPlate1].HasRealIC()==false)
                {
                    MOT[MMPlate1].InitEmptyTray(__FUNC__);
                }

                if(MOT[MMPlate2].HasRealIC()==false)
                {
                    MOT[MMPlate2].InitEmptyTray(__FUNC__);
                }
            }

            if((MOT[MMPlate1].HasIC() ||                                        //Steven 20170511 (wei) : 使用initial delay當 Soak time
                MOT[MMPlate2].HasIC()) &&
               HasHotReadyIC_9045())
            {
                if(MOT[MMTrayY].HasIC()==false && MOT[MMTrayY].fHasTray==true)  //spil 2006_0921
                {
                    Task=1;
                    break;
                }
                iHeaterWaitTime=0;                                              //jou 2012-05-18 先預先在Hotplate上面等待Soak time
                InitInArmPickFromHotPlateTask();
                bOneTimeWait=false;

                if(iDisplaySpeed==1 &&                                          //KaiChen 20171225 (Steven)：Add Speed Display
                   IniConfig.bA26MotorSpeedSortDisplay==true)
                {
                    iDisplaySpeed=2;
                    InArmAddSpeedDisplay();
                }
                Task=1500;
            }
            else
            {
                Task=50;
                iHeaterWaitTime=GetHeaterWaitTime();
                bHangTimePause=true;                                            //Steven 20090827 : Hang Up dectector

                if(iDisplaySpeed==1 &&                                          //KaiChen 20171225 (Steven)：Add Speed Display
                   IniConfig.bA26MotorSpeedSortDisplay==true)
                {
                    iDisplaySpeed=2;
                    InArmSubSpeedDisplay();
                }

                if(CheckHasSpaceToPlace_9045()==false)
                    Task=600;

                if((iCleanOut && IsInArmCleanOutFinish()) || iOneCycle)         //JerryYang 20230808 : fix換arm預熱功能clean out hang up
                {
                    bInArmWaitOneCycle=true;
                }
            }
            break;
        case 600:
            if(MoveInArmZToPlateSafe(Task))
            {
                Task=50;
            }
            break;
        case 999:                                                               //Eastsun 20260521 整合
            if(DoInArmIonFanGiveWay())
            {
                Task=1550;
            }
            break;
        case 1000:
            if(LastSet.iTemperature==Tempture_Hot)                                                                      //ChungHung 20141023 add for ATK QA mode after QA Clean out direct send to Shuttle
            {
                if(CheckPlaceToShuttle())                                                                               //Steven 20170511 (wei) : 使用initial delay當 Soak time
                {
                    Task=2000;
                    return;
                }

                if(IniConfig.bQAMode==true && LastSet.iRunStartMode==rsmQAMode)                                         //Steven 20111005 : QA Mode
                {
                    if(iQAModeLoaderCT>Prod.iQAModeCount)                                                               //數量比設定值多，表示已經做完，所以就直接送Bin 1
                    {
                        InitInArmPlaceToShuttleTask();
                        Task=2000;
                        return;
                    }
                }
            }

            Task=1100;
            InitInArmPlaceToHotPlateTask();
//jou 2010-09-20 add in arm speed
//            break;
        case 1100:
            bPlaceToHotplate=true;
            bPickFromHotplate=false;

            if(DoPlaceToHotPlate_9045())
            {
                if(LastSet.iTemperature==Tempture_Hot &&
                   LastSet.iRunStartMode!=rsmAutoSiteMap)                       //Steven 20220527 : for JCET Auto Site Map
                    HPPlaceLog.SetPosition();                                   //Steven 20211110 : 記錄放料到加熱盤的位置
                bPlaceToHotplate=false;
                bHangTimePause=true;                                            //Steven 20090827 : Hang Up dectector

                if(iOneCycle==0 &&                                              //Steven 20240326 : 修正One Cycle後會多丟一次的問題
                   IniConfig.bE40ClearAllHotICThenPickLoadIC &&                 //ChungHung 201205104 Clear HotIC then Pick Load IC
                   CheckClearAllHotICThenPickLoadIC() &&
                   HasHotReadyIC_9045())
                {
                    iHeaterWaitTime=0;
                    InitInArmPickFromHotPlateTask();
                    bOneTimeWait=false;
                    Task=1500;
                }
                else
                {
                    Task=50;
                    iAdjustSpeed=1;
                    iDisplaySpeed=1;                                            //KaiChen 20171225 (Steven)：Add Speed Display
                    iInRotateFinish=0;                                          //Ifor 20180112 (Steven) : add 避免加熱模式不跑Rotate
                }
                bSiteMappingPlaceHotplate=true;                                 //Ifor 20190308 : add 隨時開關 Site Mapping
            }

            if(Task!=1500)                                                      //Steven 20180813 : add in arm speed
                break;
        case 1500:
            iHeaterWaitTime=0;
            InitInArmPickFromHotPlateTask();
            bOneTimeWait=false;

            if(IniConfig.bA15_1ESDGiveWayFunction==true)                        //==> Eastsun 20260521 整合
            {
                InitDoInArmIonFanGiveWayTask();
                Task=999;
            }
            else
            {
            Task=1550;
            }

            if(Task!=1550)                                                      //Eastsun 20260521 整合
                break;
        case 1550:
            bOneTimeWait=false;
            bPlaceToHotplate=false;

            if(IniConfig.bI23HotTestWaitingMode && bChangeToInitStartMode)      //Chunghung 20111230 Hot Test Waiting Mode
            {
                bChangeToInitStartMode=false;
                ShowMyMessage("Tester Ready?", "", "", true);
            }

            if(DoInArmPickFromHotPlate_9045())
            {
                bPickFromHotplate=false;
                if(InArmSuck.HasIC()==false)
                {
                    if(bRunAutoSiteMapping==true)                               //Ifor 20171128 (Steven) add 避免Auto Site Mapping 多跑一次InArm安全位置
                    {
                        SetShuttlefCanMoveL(0, true, __FUNC__, "1500");
                        SetShuttlefCanMoveL(1, true, __FUNC__, "1500");
                        bOne=true;
                        Task=10;
                    }
                    else
                    {
                        Task=1;
                    }
                    break;
                }
                else                                                            //Steven 20190508 : 改到下面, 避免HP異常hang up一直被重置
                {
                    bHangTimePause=true;                                        //Steven 20090827 : Hang Up dectector
                }

                if(IniConfig.bI28_OnOffSiteOnTheFly==false &&                   //Steven 20230131 : I28與E54衝突
                   IniConfig.bE54CheckCloseSiteNoIC)                            //Steven 20160922 : 因為OneCycle永遠先跑蝦頭1, 檢查加熱盤錯誤功能與ByArmCloseSite衝突
                {
                    for(int i=0; i<2; i++)
                    {
                        for(int j=0; j<4; j++)
                        {
                            if(Prod.fInArmSuck4x8[iShtHP][i][j+iKitHP32]==false &&
                               InArmSuck.Item[i][j]==HAS_HOT_IC)
                            {
                                Task=1600;
                                return;
                            }
                        }
                    }
                }

                InitInArmPlaceToShuttleTask();
                Task=2000;
            }
            break;
        case 1600:
            ErrPart=" ";
            bHasDuplicateErr=false;
            bCheckCloseSiteHaveIC=false;

            for(int i=0; i<2; i++)
            {
                for(int j=0; j<4; j++)
                {
                    if(Prod.fInArmSuck4x8[iShtHP][i][j+iKitHP32]==false &&
                       InArmSuck.Suck[i][j].GetStatus())
                    {
                        bCheckCloseSiteHaveIC=true;
                        ErrPart+=InArmSuck.Suck[i][j].sName;
                        InArmSuck.SetItemData(i, j, HAS_NULL_IC);
                    }
                }
            }

            if(bCheckCloseSiteHaveIC)
            {
                ShowErrorMessage("JAM0114", K_RETRY, MInArmX, bHasDuplicateErr, ErrPart);                               //Steven 20100104
                bHasDuplicateErr=true;
            }
            else
            {
                InitInArmPlaceToShuttleTask();
                bPickFromHotplate=false;
                Task=2000;
            }
            break;
        case 2000:
            if(CheekNeedToDoInArmAdditionalFunction())                          //Steven 20210609 : 整合Precisor, Rotator, Bottom CCD, Die Clean
            {
                Task=200;
                break;
            }
            bPlaceShuttle=true;                                                 //ChungHung 20120226 add
            iInRotateFinish=3;                                                  //kevin 20130524 IC已放置SHUTTLE

            if(DoInArmPlaceToShuttle_9045())
            {
                bPlaceShuttle=false;                                            //ChungHung 20120226 add
                fYieldMonitoring->DoAutoCloseSite(0);                           //Steven 20170905 (wei) : Low Yield Auto Site Off for Ambient
                iInRotateFinish=0;                                              //kevin 20130524 放完SHUTTLE
                iSetShuttleToHasNullIC=0;
                bHangTimePause=true;                                            //Steven 20090827 : Hang Up dectector
                if(LastSet.iRealDummy==DUMMY && iCleanOut)
                {
                    MOT[MMTrayY].InitNewTray(NULL_IC, false, __FUNC__);
                }

                if(LastSet.iTemperature==Tempture_Hot)
                {
                    if(!bTrySuckHotPlateOneCycle)
                        HasHotReadyIC_9045();                                   //Steven 20220530 : Add

                    if(iOneCycle ||
                       (IniConfig.bUseAutoSiteMapping==true &&
                        LastSet.iRunStartMode==rsmAutoSiteMap &&
                        bSiteMappingCHKOK==false))
                    {
                        Task=50;
                        AdjustShuttlePlaceOrder_AutoSiteMapping();              //jou 2016-11-07 JCET fix Auto Site mapping Hot mode hangup
                    }
                    else
                    {
                        if(bNeedTrySuckHotPlate)                                //ChungHung 20120206 Hotplate check
                        {
                            if(bOneTimeHotPlateCheckAll)
                                Task=75;
                            else
                                Task=50;
                        }
                        else
                        {
                            Task=50;
                        }
                    }
                }
                else if(IniConfig.bA15_1ESDGiveWayFunction==true)               //==> Eastsun 20260521 整合
                {
                    InitDoInArmIonFanGiveWayTask();
                    Task=2100;
                }
                else
                {
                    Task=50;
                }

                if(IniConfig.bUseAutoSiteMapping)                               //jou 2011-03-24 start : Auto Site Mapping
                {
                    if(bSiteMappingCHKOK==false)
                        DoSiteMappingCHK();
                }
            }
            break;
        case 2100:                                                              //Eastsun 20260521 整合
            if(DoInArmIonFanGiveWay())
            {
                Task=50;
            }
            break;
        case 15000:                                                             //ChungHung 20120206 Hotplate check
            if(CheckHeaterOK()==false)                                          //Steven 20250116 : 確認HeaterOK
            {
                if(iInArmTryPickFromHotPlateTask==1)
                {
                    Task=1;
                    break;
                }
            }

            if(DoInArmTryPickFromHotPlate_9045_2x2_4_14(bOneTimeHotPlateCheckAll, bTryPickFromHotPlateShowError))
            {
                for(int i=0; i<2; i++)
                {
                    for(int j=0; j<4; j++)
                    {
                        if(InArmSuck.Item[i][j]==HAS_TRY_SUCK_IC)
                            InArmSuck.SetItemData(i, j, NULL_IC);
                    }
                }

                bHangTimePause=true;                                            //Steven 20090827 : Hang Up dectector
                if(InArmSuck.HasIC()==false)
                {
                    InitArmPickFromLoadStageTask();

                    if(bOneTimeHotPlateCheckAll)
                    {
                        if(bNeedTrySuckHotPlate)
                        {
                            Task=75;
                        }
                        else
                        {
                            Task=50;
                        }
                    }
                    else
                    {
                        Task=100;
                    }
                    break;
                }

                iHeaterWaitTime=0;
                bOneTimeWait=false;
                InitInArmPlaceToShuttleTask();
                Task=2000;
            }
            break;
    }
}
//==============================================================================
//ChungHung 20120206 Hotplate check start
// golden ainarm9045_2x2_4_14.cpp:2501
bool DoInArmTryPickFromHotPlate_9045_2x2_4_14(bool bOneTimeCheckAll, bool bShowErrorMessage)
{
    IN_ARM_TRY_PICK_HOTPLATE_2x2_4_14:
    int &Task=iInArmTryPickFromHotPlateTask;
    int ip=iPlacePlate[0], ix=iPlacePlateX[0], iy=iPlacePlateY[0];
    int j2;
    AnsiString ErrPart="";
    bool bHasDuplicateErr;
    bool bHasError;
//    static bool bSuckDuplicateErr[2][4]={{false, false, false, false}, {false, false, false, false}};
    static bool bSuckFinish[2][4]={{false, false, false, false}, {false, false, false, false}};

    GetHotPlateYHalfPos();

    switch(Task)
    {
        case 1:
            if(bInArmTryPickFromHotPlateFinish)                                 //搜尋目標位置
            {
                BackupPlacePos();                                               //ChungHung 20120206 Hotplate check
            }
            bInArmTryPickFromHotPlateFinish=false;                              //ChungHung 20120206 Hotplate check
            SearchPlateToPlace();
            if(IsHotPlateCheckFinsih())                                         //ChungHung 20120816 Hotplate check add
            {
                InArmSuck.SetAll(NULL_IC);
                Task=400;
                return false;
            }
            Task=100;
        case 100:
            if(MoveInArmXYToHotPlatePlace(iTryPickHP, true))
            {
                InArmSuck.ResetAll();                                           //Steven 20160323 : 避免未開啟真空
                Task=350;
            }
            break;
        case 350:
            for(int i=0; i<2; i++)
            {
                for(int j=0; j<2; j++)
                {
                    j2=j*InArmSuck.iPickStep;
                    if(i==1 && Row2CanPutHP(PlaceMode)==false && iPlaceHPOrder==0)
                    {
                        continue;
                    }

                    if(InArmSuck.Item[i][j2]==HAS_TRY_SUCK_IC)
                    {
                        ip=iPlacePlate[0];
                        if(bPitchOver12000)
                            ix=iPlacePlateX[0]+j*(HotPlateForm.XDivision/4);
                        else
                            ix=iPlacePlateX[0]+j*(HotPlateForm.XDivision/2);

                        //jou 980326 all row1 no use hand
                        if(((Prod.fInArmSuck4x8[0][0][0]==false && Prod.fInArmSuck4x8[1][0][0]==false) &&
                            (Prod.fInArmSuck4x8[0][0][1]==false && Prod.fInArmSuck4x8[1][0][1]==false))||               //ChungHung 20130910 alter for SCK can close site by Index
                           ((Prod.fInArmSuck4x8[0][1][0]==false && Prod.fInArmSuck4x8[1][1][0]==false) &&
                            (Prod.fInArmSuck4x8[0][1][1]==false && Prod.fInArmSuck4x8[1][1][1]==false)))
                        {
                            iy=iPlacePlateY[0];
                        }
                        else
                        {
                            if(iPlaceHPOrder==0)
                                iy=iPlacePlateY[0]+iYHalf*i;
                            else
                                iy=iPlacePlateY[0];
                        }

                        if(ArmSpeed_File[InArm].bUseHPVacuum)                                                           //Steven 20180125 (Jou) : 加熱盤的真空等待時間
                            InArmSuck.Suck[i][j2].OnDelayTime=ArmSpeed[InArm].dHPVacuumTI*100;
                        if(InArmSuck.Suck[i][j2].Suck() || InArmSuck.Suck[i][j2].Error)
                        {
                            MOT[MMPlate1+ip].SetTraySingleData(ix, iy, HAS_TRY_SUCK_IC);
                            #ifdef SOFT_SIMULTE
                                InArmSuck.SetItemData(i, j2, NULL_IC);
                            #else
                                if(InArmSuck.Suck[i][j2].Error==false && LastSet.iRealDummy==REALLY)
                                {
                                    if(bHPCleanout)                                                                     //wei 20160624 Hotplate clean out
                                    {
                                        if(iHPShuttle==0)
                                            InArmSuck.iWhichShtPickFor32=0;
                                        else
                                            InArmSuck.iWhichShtPickFor32=1;

                                        if(TestIF.iShuttleMode==1)
                                        {
                                            if(TestIF.iShuttle_Sel==0)
                                                InArmSuck.iWhichShtPickFor32=0;
                                            else if(TestIF.iShuttle_Sel==1)
                                                InArmSuck.iWhichShtPickFor32=1;
                                        }
                                    }

                                    InArmSuck.SetItemData(i, j2, HAS_SUCK_IC);
                                }
                                else
                                {
                                    InArmSuck.SetItemData(i, j2, NULL_IC);
                                }
                            #endif
                        }
                    }
                    else if(InArmSuck.Item[i][j2]==HAS_NULL_IC)
                    {
                        ip=iPlacePlate[0];
                        if(bPitchOver12000)
                            ix=iPlacePlateX[0]+j*(HotPlateForm.XDivision/4);
                        else
                            ix=iPlacePlateX[0]+j*(HotPlateForm.XDivision/2);

                        //jou 980326 all row1 no use hand
                        if(((Prod.fInArmSuck4x8[0][0][0]==false && Prod.fInArmSuck4x8[1][0][0]==false) &&
                            (Prod.fInArmSuck4x8[0][0][1]==false && Prod.fInArmSuck4x8[1][0][1]==false))||               //ChungHung 20130910 alter for SCK can close site by Index
                           ((Prod.fInArmSuck4x8[0][1][0]==false && Prod.fInArmSuck4x8[1][1][0]==false) &&
                            (Prod.fInArmSuck4x8[0][1][1]==false && Prod.fInArmSuck4x8[1][1][1]==false)))
                        {
                            iy=iPlacePlateY[0];
                        }
                        else
                        {
                            if(iPlaceHPOrder==0)
                                iy=iPlacePlateY[0]+iYHalf*i;
                            else
                                iy=iPlacePlateY[0];
                        }

                        MOT[MMPlate1+ip].SetTraySingleData(ix, iy, HAS_TRY_SUCK_IC);
                        InArmSuck.SetItemData(i, j2, NULL_IC);
                    }
                }
            }

            for(int i=0; i<2; i++)
            {
                for(int j=0; j<2; j++)
                {
                    j2=j*InArmSuck.iPickStep;
                    if(iPlaceHPOrder==0)
                    {
                        if(Row2CanPutHP(PlaceMode))
                        {
                            if(InArmSuck.Item[i][j2]==HAS_TRY_SUCK_IC)
                                return false;
                        }
                        else
                        {
                            if(InArmSuck.Item[0][j2]==HAS_TRY_SUCK_IC)
                                return false;
                        }
                    }
                    else
                    {
                        if(InArmSuck.Item[1][j2]==HAS_TRY_SUCK_IC)
                            return false;
                    }
                }
            }

            Task=400;
            goto IN_ARM_TRY_PICK_HOTPLATE_2x2_4_14;
        case 400:
            for(int i=0; i<2; i++)
            {
                for(int j=0; j<4; j++)
                {
                    if((Prod.fInArmSuck4x8[0][i][j]  ||
                        Prod.fInArmSuck4x8[1][i][j]) &&
                        InArmSuck.Item[i][j]==HAS_TRY_SUCK_IC)                  //ChungHung 20130910 alter for SCK can close site by Index
                    {
                        Task=1;
                        return false;
                    }
                }
            }

            if(MoveInArmZToPlateSafe(Task))
            {
                //檢查是否有 HAS_SUCK_IC
                //有HAS_SUCK_IC 秀出Error Message
                //將HAS_SUCK_IC 改為HAS_TRY_SUCK_IC
                //Task=100;
                //return false;
                bHasError=false;
                if(bShowErrorMessage)
                {
                    ErrPart=" ";
                    bHasDuplicateErr=false;

                    for(int i=0; i<2; i++)
                    {
                        for(int j=0; j<4; j++)
                        {
                            InArmSuck.Suck[i][j].Error=false;
                            bSuckFinish[i][j]=false;
                            if(InArmSuck.Item[i][j]==HAS_SUCK_IC)
                            {
                                ErrPart+=InArmSuck.Suck[i][j].sName;
                                bHasError=true;
                            }

                            if(bTryPickHPDuplicateErr[i][j])
                            {
                                bHasDuplicateErr=true;
                            }
                        }
                    }

                    if(bHasError)
                    {
                        ShowErrorMessage("WAR0170", K_RETRY, MInArmX, bHasDuplicateErr, ErrPart);
                        for(int i=0; i<2; i++)
                        {
                            for(int j=0; j<4; j++)
                            {
                                bTryPickHPDuplicateErr[i][j]=true;
                            }
                        }

                        Task=450;
                    }
                    else
                    {
                        for(int i=0; i<2; i++)
                        {
                            for(int j=0; j<4; j++)
                            {
                                bTryPickHPDuplicateErr[i][j]=false;
                            }
                        }

                        Task=500;
                    }
                }
                else
                {
                    if(bOneTimeCheckAll)
                    {
                        if(InArmSuck.HasType(HAS_SUCK_IC))
                        {
                            for(int i=0; i<2; i++)
                            {
                                for(int j=0; j<4; j++)
                                {
                                    if(InArmSuck.Item[i][j]==HAS_SUCK_IC)
                                    {
                                        InArmSuck.SetItemData(i, j, HAS_HOT_IC);
                                    }
                                    else
                                    {
                                        InArmSuck.SetItemData(i, j, HAS_NULL_IC);
                                    }
                                }
                            }
                        }
                    }
                    Task=500;
                }
            }
            break;
        case 450:
            for(int i=0; i<2; i++)
            {
                for(int j=0; j<4; j++)
                {
                    if(bSuckFinish[i][j]==true)
                    {
                        continue;
                    }
                    else
                    {
                        if(ArmSpeed_File[InArm].bUseHPVacuum)                   //Steven 20180125 (Jou) : 加熱盤的真空等待時間
                            InArmSuck.Suck[i][j].OnDelayTime=ArmSpeed[InArm].dHPVacuumTI*100;
                        if((InArmSuck.Suck[i][j].Suck() || InArmSuck.Suck[i][j].Error) && bSuckFinish[i][j]==false)
                        {
                            bSuckFinish[i][j]=true;
                            if(InArmSuck.Suck[i][j].Error==false && LastSet.iRealDummy==REALLY)
                            {
                                InArmSuck.SetItemData(i, j, HAS_SUCK_IC);
                            }
                            else
                            {
                                InArmSuck.SetItemData(i, j, NULL_IC);
                            }
                        }
                    }
                }
            }

            if(bSuckFinish[0][0] && bSuckFinish[0][1] && bSuckFinish[0][2] && bSuckFinish[0][3] &&
               bSuckFinish[1][0] && bSuckFinish[1][1] && bSuckFinish[1][2] && bSuckFinish[1][3])
            {
                if(InArmSuck.HasType(HAS_SUCK_IC))
                {
                    Task=400;
                }
                else
                {
                    TryHotClearRecBufMMPlate();                                 //ChungHung 20120206 Hotplate check;  //ChungHung 20120206 Hotplate check
                    RestorePlacePos();                                          //ChungHung 20120206 Hotplate check
                    bInArmTryPickFromHotPlateFinish=true;                       //ChungHung 20120206 Hotplate check
                    for(int i=0; i<2; i++)
                        for(int j=0; j<2; j++)
                            if(InArmSuck.Item[i][j*InArmSuck.iPickStep]==NULL_IC)
                                InArmSuck.SetItemData(i, j*InArmSuck.iPickStep, HAS_TRY_SUCK_IC);
                    Task=1;
                }
            }
            break;
        case 500:
            SetInArm_Unuse_SuckToNullICForHP();

            if(bOneTimeCheckAll==true)
                AdjustShuttleWhichKitOrder();                                   //Steven 20220520 : 修正Try pick HP

            if(IsHotPlateCheckFinsih())                                         //ChungHung 20120206 Hotplate check
            {
                if(bOneTimeCheckAll==true)
                {
                    bNeedTrySuckHotPlate=false;
                    TryHotClearRecBufMMPlate();                                 //ChungHung 20120206 Hotplate check;

                    if(bHotPlateCheckNeedTrayFeed==true)
                    {
                        bTrySuckHotPlateCleanOut=true;
                        iCleanOut=1;
                    }
                    else
                    {
                        bTrySuckHotPlateOneCycle=true;
                        iOneCycle=1;
                    }
                    ResetHotPlateSearchParameter();
                }
                else
                {
                    if(InArmSuck.HasIC()==false)
                    {
                        bNeedTrySuckHotPlate=false;
                    }
                }
            }

            if(bOneTimeCheckAll==false)
                TryHotClearRecBufMMPlate();                                     //ChungHung 20120206 Hotplate check;  //ChungHung 20120206 Hotplate check

            RestorePlacePos();                                                  //ChungHung 20120206 Hotplate check
            bInArmTryPickFromHotPlateFinish=true;                               //ChungHung 20120206 Hotplate check

            return true;
    }
    return false;
}
//ChungHung 20120206 Hotplate check end
//==============================================================================
// golden ainarm9045_2x2_4_14.cpp:2860
//
// W7-GATE: the golden GetShuttleState_2x2_4_14 body derefs
//   ptrInSHT->ArmDownSideAllTypeIC(...) -- a TMyKitSuck method NOT yet present in
//   the offline substrate (golden MyKitSuck.h:335; sibling ArmUpSideAllTypeIC IS
//   present at aHotPlateSubstrate.h:263).  As a class member it cannot be
//   forward-declared without editing the shared aHotPlateSubstrate.h, which this
//   Translate agent must not touch.  REPORTED in substrateNeeds for Integrate to
//   add ArmDownSideAllTypeIC to TMyKitSuck ONCE; then flip this gate to make the
//   body faithful.  Active stub returns 0 (offline-safe "no shuttle has IC / no
//   place needed" default; this geometry query has no hardware side effect).
int GetShuttleState_2x2_4_14(int iSht, bool bPick)
{
#if 1 // AI(W6.2c-INARM-batch2) 20260626: ArmDownSideAllTypeIC now in substrate (aHotPlateSubstrate.h:335) -> golden body ACTIVE
    int s=InArmSuck.iPickStep;
    if(bPick==bAutoPick)                                                        //Pick
    {
        if(ptrInSHT->ArmUpSideAllTypeIC(NULL_IC, 0, 2)==false)                  //左上有料
            return 1;
        else if(ptrInSHT->ArmDownSideAllTypeIC(NULL_IC, 0, 2)==false)           //左下有料
            return 2;
        else
            return 0;
    }
    else                                                                        //Place
    {
        if(bUse8Picker)
        {
            if(ptrInSHT->ArmUpSideAllTypeIC(NULL_IC, 0, 2) &&                   //左上無料
               ((bAutoCleanPlaceToSht==false && (InArmSuck.Item[0][0]==NULL_IC || InArmSuck.Item[0][s]==NULL_IC)) ||
                (bAutoCleanPlaceToSht==true  && (InArmSuck.Item[0][0]!=NULL_IC || InArmSuck.Item[0][s]!=NULL_IC))))
                return 1;
            else if(ptrInSHT->ArmDownSideAllTypeIC(NULL_IC, 0, 2) &&            //左下無料
                    ((bAutoCleanPlaceToSht==false && (InArmSuck.Item[1][0]==NULL_IC || InArmSuck.Item[1][s]==NULL_IC)) ||
                     (bAutoCleanPlaceToSht==true  && (InArmSuck.Item[1][0]!=NULL_IC || InArmSuck.Item[1][s]!=NULL_IC))))
                return 2;
            else
                return 0;
        }
        else
        {
            if(ptrInSHT->ArmUpSideAllTypeIC(NULL_IC, 0, 2) &&                   //左上無料
               ((bAutoCleanPlaceToSht==false && (InArmSuck.Item[1][0]==NULL_IC || InArmSuck.Item[1][s]==NULL_IC)) ||
                (bAutoCleanPlaceToSht==true  && (InArmSuck.Item[1][0]!=NULL_IC || InArmSuck.Item[1][s]!=NULL_IC))))
                return 1;
            else if(ptrInSHT->ArmDownSideAllTypeIC(NULL_IC, 0, 2) &&            //左下無料
                    ((bAutoCleanPlaceToSht==false && (InArmSuck.Item[1][0]==NULL_IC || InArmSuck.Item[1][s]==NULL_IC)) ||
                     (bAutoCleanPlaceToSht==true  && (InArmSuck.Item[1][0]!=NULL_IC || InArmSuck.Item[1][s]!=NULL_IC))))
                return 2;
            else
                return 0;
        }
    }
#endif
    (void)iSht; (void)bPick;
    return 0; // W7 offline stub -- see gate note above
}
//==============================================================================
// golden ainarm9045_2x2_4_14.cpp:2903
void SetInArmHasDropToShuttle_2x2_4_14(int SHT, int Row, int Col)
{
    if(SHT==0)
    {
        if(Row==0 && Col==0)
            bDropAtSht1NeedCheckVac[0][0]=true;
        else if(Row==0 && Col==3)
            bDropAtSht1NeedCheckVac[0][1]=true;
        else if(Row==1 && Col==0)
            bDropAtSht1NeedCheckVac[1][0]=true;
        else if(Row==1 && Col==3)
            bDropAtSht1NeedCheckVac[1][1]=true;
    }
    else
    {
        if(Row==0 && Col==0)
            bDropAtSht2NeedCheckVac[0][0]=true;
        else if(Row==0 && Col==3)
            bDropAtSht2NeedCheckVac[0][1]=true;
        else if(Row==1 && Col==0)
            bDropAtSht2NeedCheckVac[1][0]=true;
        else if(Row==1 && Col==3)
            bDropAtSht2NeedCheckVac[1][1]=true;
    }
}
