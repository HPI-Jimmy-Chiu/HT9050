// =============================================================================
//  ainarm_SearchPlacePlate.cpp  --  shared InArm HotPlate-PLACE geometry/search/swap
//
//  Translated from: HT9011UC_Code_V3.33.906.0_20260618/ainarm_SearchPlacePlate.cpp (4949 ln)
//  Translation wave: W6.2 (IN-ARM foundation -- shared HP-place geometry leaf)
//  Translator: AI(W6.2-INARM) 20260626
//
//  FAITHFUL TRANSLATION
//  --------------------
//  Shared HP-place geometry + search-planner + data-swap leaf library that the
//  per-site ainarm9045_* SMs call.  There is NO switch(Task) SM in this file --
//  the task-cursor SM lives in the per-site modules.  All integer pitch math,
//  %/divisor expressions and the golden /0 guards (e.g. GetHotPlateYHalfPos
//  `(iYPitch==0)?0:...`, RogerYang 20250618) are kept EXACTLY.
//
//  class TMyHotPlatePlaceLog is declared in the .h and DEFINED here (ctor +
//  InitialPosition/CheckPosition/SetPosition), self-contained, verbatim.
//  This file DEFINES the exported globals: iPlaceHP/iPickHP/iTryPickHP,
//  iInArmToHPX/Y, iInXPToHP[], iZPosToHP[][], bZFlgToHP/Usage/Pick[][],
//  bPlaceToHotplatePartOK, the file-local b6x20HP/b4x11HP/b4x10HP_2x2/
//  b8x16HP_2x2/b12x16HP_2x6, iHPHangUpCount/sHPHangUpFunc, tHotPlateHangUp,
//  HPPlaceLog.
//
//  GATED #if 0 // TODO(W7: per-site DoPlaceToHotPlate variants):
//    * DoPlaceToHotPlate_9045() iInArmType DISPATCHER body (golden :4839-4948)
//      + the 24 `extern bool DoPlaceToHotPlate_9045_<config>();` decls
//      (golden :4814-4837).  REASON: pure iInArmType dispatch to per-site bodies
//      defined in ainarm9045_*.cpp (W7); the task rule keeps iInArmType dispatch
//      in the later ainarm core.  ACTIVE stub provided: `return false;`.
//      The .h `extern bool DoPlaceToHotPlate_9045();` stays live.
//    * CheckHasSpaceToPlace_9045's `fMain->sbStateRecordClick(fMain->sbStateRecord);`
//      (golden :588 inside TMyHotPlatePlaceLog::SetPosition) -> gated to avoid
//      pulling TSpeedButton/TObject (state-record UI is W7).
//
//  SOFT_SIMULTE is NOT defined: #ifndef SOFT_SIMULTE CheckSafeDoorIsClosed lines
//  are ACTIVE; any #ifdef SOFT_SIMULTE block compiles out naturally (kept verbatim).
//  Engine helpers not yet translated resolve via aHotPlateSubstrate.h externs.
//
//  Big5: every Chinese comment decoded cleanly via cp950 and is preserved as
//  UTF-8.  NO U+FFFD is emitted.
// =============================================================================
// BCB6 ORIGINAL include block (mirrored as a comment for provenance):
//   #include "MachineDefine.h"   ; #pragma hdrstop
//   #include "ainarm_SearchPlacePlate.h"
//   #include "aArmHeader.h"  (god-header -- kept as the W6.0 SHIM, #if 0)
//   #include "note.h"        (ShowErrorMessage)            -> canary_support.h
//   #include "csystem.h"     (predicate API + CheckSafeDoorIsClosed) -> kept
//   #include "cinitial.h"    (InitialOK etc. via cmydef)   -> cmydef.h
//   #include "mymessbox.h"   (ShowMyMessage VCL)           -> canary_support.h
//   #include "mymotor.h"     (MOT[]/InArmContinuousMove_9045) -> Motor/mymotor.h
//   #include "cmydef.h" / "atester.h" / "common.h" / "mycylin.h" / "cprod.h"
//   #include "MyKitSuck.h"   (InArmSuck/FLCarryKit/...)     -> aHotPlateSubstrate.h
//   #include "OCR.h" / "cMyDB.h" (MyDBIProcess)            -> aHotPlateSubstrate.h
//   #include "aRotateKIT.h" / "fRotate.h" (tRotate/USE_ROTATE_KIT) -> aHotPlateSubstrate.h + cmydef.h
//   #include "cpublic.h"
//   #include "main.h" (fMain) / "cSortCT.h" (fSortCT) / "uLotInfo.h" (fLotInfo) -> FormsFacade.h
//   ainarm2.h cursors (iPlacePlate/iHotWhich*/InArmSiteMapData/...) -> aHotPlateSubstrate.h
//   #pragma package(smart_init)
#include "ainarm_SearchPlacePlate.h"

#include "aArmHeader.h"             // W6.0 SHIM (god-header gated #if 0; do NOT un-gate)
#include "ainarm_SearchPickPlate.h" // shared HP-pick externs (bInArmTryPickFromHotPlateFinish/GetHeaterWaitTime/MoveInArmXYPickHotPlate_9045/...) -- golden pulled these via aArmHeader god-header
#include "csystem.h"                // predicate API (CheckSafeDoorIsClosed) + DoAutoSiteMappingDropError
#include "Motor/mymotor.h"          // MOT[], InArmContinuousMove_9045 (W4 sim stub)
#include "cmydef.h"                 // global type universe + IC consts + USE_ROTATE_KIT/iRotate_Type
#include "common.h"
#include "mycylin.h"
#include "cprod.h"                  // Prod / TestIF / TestIF_File / HotPlateForm / ArmSpeed / LastSet enums
#include "cpublic.h"                // CosFunction
#include "aHotPlateSubstrate.h"     // InArmSuck/FLCarryKit/.. + PickFromHPList + ainarm2 cursors + engine externs + tRotate/Laser
#include "FormsFacade.h"            // fMain/fSortCT/fLotInfo/fOffSet/fSCKART (non-VCL stubs)
#include "canary_support.h"         // ShowErrorMessage / ShowMyMessage / RecordProcess / LastSet / __FUNC__

const int iHangUpCount=10000;                                                   //Steven 20120919 : Hang Up 保護

TMyHotPlatePlaceLog HPPlaceLog;                                                 //Steven 20211110 : 記錄放料到加熱盤的位置
int iPlaceHP    =0;
int iPickHP     =1;
int iTryPickHP  =2;
//==============================================================================
int  iInArmToHPX=0;
int  iInArmToHPY=0;
int  iInXPToHP[X_PITCH_COUNT]={0};
int  iZPosToHP[MAX_ARM_Row][MAX_ARM_Col]={0};
bool bZFlgToHP[MAX_ARM_Row][MAX_ARM_Col]={false};
bool bZFlgToHPUsage[MAX_ARM_Row][MAX_ARM_Col]={false};
bool bZFlgToHPPick[MAX_ARM_Row][MAX_ARM_Col]={false};
bool bPlaceToHotplatePartOK=false;                                              //ChungHung 20120109 add
bool b6x20HP=false;                                                             //Steven 20240930 : for 6x20 HP
bool b4x11HP=false;                                                             //Steven 20240930 : for 4x11 HP
bool b4x10HP_2x2=false;                                                         //Steven 20241120 : for 4x10 HP
bool b8x16HP_2x2=false;                                                         //Steven 20250107 : for 2x2 run 8x16 HP
bool b12x16HP_2x6=false;                                                        //Steven 20241203 : for 12x16 HP
//==============================================================================
void ResetInToHPFlag()
{
    ZeroMemory(iInXPToHP, sizeof(iInXPToHP));
    ZeroMemory(iZPosToHP, sizeof(iZPosToHP));
}
//------------------------------------------------------------------------------
int iHPHangUpCount=0;
AnsiString sHPHangUpFunc="";
TQPF_Timer tHotPlateHangUp;
//------------------------------------------------------------------------------
void ClearHotPlateHangUp()                                                      //Steven 20161102 : 針對加熱盤Hang Up進行紀錄
{
    tHotPlateHangUp.SetSecAndOn(3);                                             //Sam 20220829 : 發生 HotPlateHangUp 時必須要先檢查安全門
    iHPHangUpCount=0;
    sHPHangUpFunc="";
}
//------------------------------------------------------------------------------
void DoHotPlateHangUp(AnsiString FuncName)                                      //Steven 20161102 : 針對加熱盤Hang Up進行紀錄
{
    if(CheckSafeDoorIsClosed()==false)                                          //Sam 20220829 : 發生 HotPlateHangUp 時必須要先檢查安全門
    {
        ShowErrorMessage("WAR0150", 0, MInArmX, false, FuncName);
        ClearHotPlateHangUp();
        fAllMotorHome=false;
        //SystemStart=false;
        //StopAllMotor();
    }
    else
    {
        if(iHPHangUpCount==0)
        {
            sHPHangUpFunc=FuncName;
            iHPHangUpCount=1;
        }
    }
}
//==============================================================================
void GetHotPlateYHalfPos()
{
    if(InitialOK==false)
        return;

    b6x20HP=false;
    b4x11HP=false;
    b4x10HP_2x2=false;                                                          //Steven 20241120 : for 4x10 HP
    b8x16HP_2x2=false;                                                          //Steven 20250107 : for 2x2 run 8x16 HP

    int iYPitch=HotPlateForm.YPitch;

    if(InArmSuck.iPickRow==1)
    {
        iYHalf=1;
    }
    else
    {
        if(USE_IN_OUT_ARM_Y_PITCH==iXPitch60)                                   //ChungHung 20140305 AutoYPitch
        {
            iYPitch=iYPitch/10;
            iYPitch=iYPitch*10;
        }

        if(InArmSuck.iPickRow==2 &&
           InArmSuck.iPickCol>=2 &&                                             //KevinCheng 20250912 : 吸嘴改為AxEx後的路徑
           HotPlateForm.XDivision==4 &&
           HotPlateForm.YDivision%2!=0)
        {
            b4x11HP=true;                                                       //Steven 20240930 : for 4x11 HP
            iYHalf=1;
        }
        else if(InArmSuck.iPickRow==2 &&
                InArmSuck.iPickCol==2 &&
                TestIF.iARM_HP_Y_PITCH%iYPitch!=0)
        {
            if(HotPlateForm.XDivision==4)
                b4x10HP_2x2=true;                                               //Steven 20241120 : for 4x10 HP
            iYHalf=1;
        }
        else if(InArmSuck.iPickRow==2 &&
                HotPlateForm.YDivision==20 &&
                iYPitch==1270)                                                  //Steven 20240930 : for 6x20 HP
        {
            iYHalf=5;
            b6x20HP=true;
        }
        else
        {
            iYHalf=(iYPitch==0)?0:(TestIF.iARM_HP_Y_PITCH/iYPitch);             //RogerYang 20250618 防止除以零
        }

        if(HotPlateForm.XDivision==8)
        {
            if(TestIF.iTestMode==QualSite2X2 && TestIF.iUseSuckMode==4)         //Steven 20250107 : for 2x2 run 8x16 HP
            {
                if(USE_IN_OUT_ARM_Y_PITCH==iXPitch60 &&
                   HotPlateForm.YDivision==16 &&
                   iYPitch==2000)
                {
                    b8x16HP_2x2=true;
                }
            }
        }
        else if(HotPlateForm.XDivision==12)
        {
            if(TestIF.iTestMode==QualSite2X2 && TestIF.iUseSuckMode==4)         //Steven 20211227 : 修正2x2關B排Hang up
            {
                iCloseSiteState=CloseSiteState();                               //Steven 20210420 : 修正12site關site跑2x4 mode, HP=6x11 hang up
                if(iCloseSiteState!=0)
                    iYHalf=1;
            }
            else if(HotPlateForm.YDivision==16 &&
                    iInArmType==e9045_2x6_8)
            {
                if(USE_IN_OUT_ARM_Y_PITCH==iXYPitchVariable ||
                   USE_IN_OUT_ARM_Y_PITCH==iXYPitchIn_Bb_Out_Bc)                //Ztex 2024.02.24 Add HT-1132
                {
                    if(iYPitch*2>IN_OUT_ARM_Y_PITCH_MIN &&
                       iYPitch*2<IN_OUT_ARM_Y_PITCH_MAX)
                    {
                        b12x16HP_2x6=true;                                      //Steven 20241203 : for 12x16 HP
                        iYHalf=2;
                    }
                }
                else
                {
                    if(Prod.HotPlateForm[0].iYPitch>=2115 &&
                       Prod.HotPlateForm[0].iYPitch<=2118)                      //Steven 20250102 : fixed for HP Y-Pitch=21.16mm
                    {
                        b12x16HP_2x6=true;                                      //Steven 20250102 : Fixed 2x6 support X=12
                        iYHalf=3;
                    }
                }
            }
        }

        if(iYHalf<=0)                                                           //Steven 20150224 : 修正Y-Pitch36異常
            iYHalf=1;
    }
}
//==============================================================================
bool HotPlateYPitchCanPutAll()
{
    int iYPitch=Prod.HotPlateForm[0].iYPitch;

    if(bRunAutoClean &&
       TestIF_File.iAutoClean_Function &&
       TestIF_File.iAutoClean_Tray==eCKPos_HP2)                                 //kevin 20130226 autoclean
        return false;

    if(iInArmType==e9045_2x3_6_14)                                              //Steven 20200219 : 2x3mode, Layout: 120x70mm
    {
        return false;
    }

//    if(USE_IN_OUT_ARM_Y_PITCH==iXPitch60 || USE_IN_OUT_ARM_Y_PITCH==iXYPitchVariable) //ChungHung 20140304 add for AutoYPitch
//    {
    iYPitch=iYPitch/10;
    iYPitch=iYPitch*10;
//    }

    if(InArmSuck.iPickRow==1)
        return false;

    if(USE_IN_Y_IS_AUTO_PITCH==true)                                            //Frank 20160302 HotPlate With Y_Pitch  //JerryYang 20251218 : IN/OUT ARM支援不同模組
    {
        if(b12x16HP_2x6==true)                                                  //Steven 20241203 : for 12x16 HP
            return true;

        if(HotPlateForm.XDivision==4)
        {
            if(InArmSuck.iPickRow==2 &&                                         //Steven 20240704 : Mark for 4x11 HP with 2x2 mode
               InArmSuck.iPickCol>2)
                return false;
        }

        if(iYPitch>=IN_OUT_ARM_Y_PITCH_MIN &&
           iYPitch<=IN_OUT_ARM_Y_PITCH_MAX)
        {
            return true;
        }

        if(InArmSuck.iPickRow==2 &&
           TestIF.iARM_HP_Y_PITCH%iYPitch==0)                                   //ChungHung 20130411
            return true;
    }
    else if(iYPitch<=TestIF.iARM_HP_Y_PITCH)                                    //Steven 20191024 : 換位置
    {
        if(b12x16HP_2x6==true)                                                  //Steven 20250102 : Fixed 2x6 support X=12
            return true;

        if(HotPlateForm.XDivision==4)                                           //Steven 20140703 : Fix Hot Plate X-Item=4, Y-Pitch=30 吸放錯亂問題 //Steven 20160222 : Mark 2x2 for hang up
        {
            if(InArmSuck.iPickCol<=2 &&
               b4x11HP==false &&                                                //KevinCheng 20250912 : 吸嘴改為AxEx後的路徑
               TestIF.iARM_HP_Y_PITCH%iYPitch==0)
                return true;
            else
                return false;                                                   //kevin 20150723
        }

        if(HotPlateForm.XDivision==6 && HotPlateForm.YDivision==16)             //Steven 20240326 : Fixed for 6x16 HP
        {
            if(InArmSuck.iPickRow==2 && iYPitch==2000)
                return false;
        }

        if((iInArmType==e9045_2x4_8 ||
            iCloseSiteModeFor2x8>e2x8OneByOne ||
            iCloseSiteModeFor2x6>e2x6OneByOne) &&
            HotPlateForm.XDivision==6)                                          //Sam 20190226 : 16Site4X4
        {
            if(TestIF.iARM_HP_Y_PITCH%iYPitch==0)                               //ChungHung 20130411
                return true;
        }
        else
        {
            if(USE_IN_OUT_ARM_Y_PITCH==iXPitch60 ||
               USE_IN_OUT_ARM_Y_PITCH==iXYPitchVariable ||                      //ChungHung 20140304 add for AutoYPitch
               USE_IN_OUT_ARM_Y_PITCH==iXYPitchIn_Bb_Out_Bc)                    //Ztex 2024.02.24 Add HT-1132
            {
                if(TestIF.iARM_HP_Y_PITCH%iYPitch==0)                           //ChungHung 20120505 9045WS
                    return true;
            }
            else
            {
                if(LastSet.iTemperature==Tempture_Hot &&                        //Frank 20160226 add for 2x2_8_Hot 1cm
                  (HotPlateForm.XDivision==6 || HotPlateForm.XDivision==8) &&
                  TestIF.iTestMode==QualSite2X2 && TestIF.iUseSuckMode==8)
                    return true;

                if((TestIF.iTestMode==QualSite1X4 && USE_PICKER_COUNT!=0) ||    //ChungHung 20130603 alter 1x4
                   TestIF.iTestMode==_8Site1X4 || TestIF.iTestMode==TriSite1X3)                                         //ChungHung 20150528 add for 海思 _8Site1x4 //Frank 20160329 add for 1x3_4
                {
                    return true;
                }
                else
                {
                    if(TestIF.iARM_HP_Y_PITCH==6350 &&
                       Prod.HotPlateForm[0].iYPitch>=2115 &&
                       Prod.HotPlateForm[0].iYPitch<=2118)                      //Steven 20250102 : fixed for HP Y-Pitch=21.16mm
                    {
                        return true;
                    }

                    if(TestIF.iARM_HP_Y_PITCH%iYPitch<=2)                       //Jou 20120505 9045WS
                        return true;
                }
            }
        }
    }
    return false;
}
//------------------------------------------------------------------------------
void Row2PlaceHPPos(int iNowRow)
{
    if(iNowRow+iYHalf>=HotPlateForm.YDivision)
        ShowMyMessage("Place Fail");
}
//------------------------------------------------------------------------------
bool Row2CanPutHP(bool bPlaceMode)
{
    int iNowRow=0, iNowCol=0, iYStep=0;
    if(bRunAutoClean &&
       TestIF_File.iAutoClean_Function &&
       TestIF_File.iAutoClean_Tray==eCKPos_HP2)                                 //kevin 20130226 autoclean
        return false;

    if(iInArmType==e9045_2x3_6_14)                                              //Steven 20200219 : 2x3mode, Layout: 120x70mm
    {
        return false;
    }

    GetHotPlateYHalfPos();
    iYStep=iYHalf;                                                              //ChungHung 20120505 9045WS

    if(HotPlateYPitchCanPutAll()==false)
        return false;

    if(bPlaceMode)
    {
        iNowRow=iPlacePlateY[0];
        iNowCol=iPlacePlateX[0];
    }
    else
    {
        iNowRow=iPickPlateY[0];
        iNowCol=iPickPlateX[0];
    }

    if(HotPlateForm.XDivision==6  &&
       HotPlateForm.YDivision==11 &&
       iYHalf==2 &&
       iNowCol==1 && iNowRow==9)
    {
        if(i8PickerHPMode==iHPWideHP)
        {
        }
        else
        {
            iPlaceHPOrder=1;
            return true;
        }
    }

    if(iYStep+iNowRow>=HotPlateForm.YDivision)
        return false;

    if(LastSet.iTemperature==Tempture_Hot &&                                    //ChungHung 20110309 2x2 start
       (HotPlateForm.XDivision==6 || HotPlateForm.XDivision==8) &&
       TestIF.iTestMode==QualSite2X2 && TestIF.iUseSuckMode==8)
        return true;

    if(iCloseSiteState>0)
        return false;
    return true;
}
//==============================================================================
bool AllRow1NotUseForHP(int iAction)
{
    int iSht  =InArmSuck.iWhichSht;
    int iKit32=(InArmSuck.iWhichKit==0)?0:4;

    if(iAction==iPlaceHP)
    {
        iSht  =InArmSuck.iWhichSht;
        iKit32=(InArmSuck.iWhichKit==0)?0:4;
    }
    else
    {
        iSht  =InArmSuck.iWhichShtPickFor32;
        iKit32=(InArmSuck.iWhichKitPickFor32==0)?0:4;
    }

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
            return false;
        }
    }
    return true;
}
//==============================================================================
bool AllRow2NotUseForHP(int iAction)
{
    int iSht  =InArmSuck.iWhichSht;
    int iKit32=(InArmSuck.iWhichKit==0)?0:4;

    if(iAction==iPlaceHP)
    {
        iSht  =InArmSuck.iWhichSht;
        iKit32=(InArmSuck.iWhichKit==0)?0:4;
    }
    else
    {
        iSht  =InArmSuck.iWhichShtPickFor32;
        iKit32=(InArmSuck.iWhichKitPickFor32==0)?0:4;
    }

    if(iInArmType==e9045_1x4_8_Hot ||                                           //Steven 20240119 : fixed for 2x4_16
       iInArmType==e9045_1x2_4_Hot ||
       iInArmType==e9045_2x2_8_Hot)
    {
//        iSht=0;
        iKit32=0;
    }

    for(int j=0; j<4; j++)
    {
        int j2=j+iKit32;
        if(Prod.fInArmSuck4x8[iSht][1][j2]==true)
        {
            return false;
        }
    }
    return true;
}
//==============================================================================
bool ArmRow1NoICForPlaceToPlate_ACEG()                                          //使用4吸嘴
{
    bool bTryPick=!bInArmTryPickFromHotPlateFinish;                             //ChungHung 20120206 Hotplate check
    int iStepX=4;

    for(int i=0; i<iStepX; i++)
    {
        if(i<InArmSuck.iMaxCol &&
           HotPlateYPitchCanPutAll()==false)                                    //第二排不可放置IC
        {
            if((!bTryPick && InArmSuck.Item[0][i]!=NULL_IC) ||
               ( bTryPick && InArmSuck.Item[0][i]==HAS_TRY_SUCK_IC))            //使用第一排 0,1,2,3 吸嘴 確認吸嘴上沒IC   //ChungHung 20120627 add
            {
                iPlaceHPOrder=0;                                                //第一排
                return false;
            }
        }
        else                                                                    //第二排可放置IC
        {
            if(HotPlateForm.XDivision==6  &&
               HotPlateForm.YDivision==11 &&
               InArmSuck.iPickRow==2 &&
               iYHalf==2 &&
               iPlacePlateX[0]==1 && iPlacePlateY[0]==9 &&
               i8PickerHPMode==iHPNoUse8suck)
            {
                iPlaceHPOrder=1;
                return true;
            }
            else
            {
                if(i<InArmSuck.iMaxCol &&
                   (!bTryPick && InArmSuck.Item[0][i]!=NULL_IC) ||
                   ( bTryPick && InArmSuck.Item[0][i]==HAS_TRY_SUCK_IC))
                {
                    iPlaceHPOrder=0;
                    return false;
                }
            }
        }
    }
    iPlaceHPOrder=1;
    return true;
}
//==============================================================================
bool ArmRow1NoICForPlaceToPlate_AxxG()                                          //使用1 & 4吸嘴
{
    bool bTryPick=!bInArmTryPickFromHotPlateFinish;                             //ChungHung 20120206 Hotplate check
    int iStepX=GetHotPlateColStep(iPickHP);

    for(int j=0; j<iStepX; j++)
    {
        int j2=GetPlaceToHotPlateSuckCol(j);
        if(j2<InArmSuck.iMaxCol &&
           (!bTryPick && InArmSuck.Item[0][j2]!=NULL_IC) ||
           ( bTryPick && InArmSuck.Item[0][j2]==HAS_TRY_SUCK_IC))
        {
            iPlaceHPOrder=0;
            return false;
        }
    }
    iPlaceHPOrder=1;
    return true;
}
//==============================================================================
bool ArmRow1NoICForPlaceToPlate_AxEx()                                          //使用1 & 3吸嘴
{
    bool bTryPick=!bInArmTryPickFromHotPlateFinish;                             //ChungHung 20120206 Hotplate check
    int iStepX=GetHotPlateColStep(iPickHP);

    for(int j=0; j<iStepX; j++)
    {
        int j2=GetPlaceToHotPlateSuckCol(j);
        if(j2<InArmSuck.iMaxCol &&
           (!bTryPick && InArmSuck.Item[0][j2]!=NULL_IC) ||                     //Inarm use 1 & 3 pick
           ( bTryPick && InArmSuck.Item[0][j2]==HAS_TRY_SUCK_IC))               //使用第一排 0,2 吸嘴 確認吸嘴上沒IC
        {
            iPlaceHPOrder=0;                                                    //第一排
            return false;
        }
    }
    iPlaceHPOrder=1;
    return true;
}
//==============================================================================
bool ArmRow1NotICForPlaceToPlate()                                              //檢測第一排是否有IC return false:有IC true:沒有IC  //ChungHung 20120206 Hotplate check
{
    if(CosFunction.bUSEJCETSiteMapMode==true &&                                 //Steven 20220527 : for JCET Auto Site Map
       IniConfig.bI21AutoSiteMappingUseHotplate==true &&
       bAutoSiteMapHasPickHP==true)
    {
        if(iAutoSiteMapInArmRow==0)
            return false;
        else
            return true;
    }
    else
    {
        if(bUseAxExPicker())
        {
            return ArmRow1NoICForPlaceToPlate_AxEx();
        }
        else if(bUseAxxGPicker())
        {
            return ArmRow1NoICForPlaceToPlate_AxxG();
        }
        else
        {
            return ArmRow1NoICForPlaceToPlate_ACEG();
        }
    }
}
//------------------------------------------------------------------------------
__fastcall TMyHotPlatePlaceLog::TMyHotPlatePlaceLog()                           //Steven 20211110 : 記錄放料到加熱盤的位置
{
    InitialPosition();
}
//------------------------------------------------------------------------------
void TMyHotPlatePlaceLog::InitialPosition()                                     //Steven 20211110 : 記錄放料到加熱盤的位置
{
    iX=0;
    iY=0;
    iVari=0;
    iYVari=0;
    iVari2=0;
    bIsSamePos=false;
}
//------------------------------------------------------------------------------
void TMyHotPlatePlaceLog::CheckPosition(int XPos, int YPos, int Vari, int YVari, int Vari2)
{
    if(iX==XPos && iY==YPos && iVari==Vari && iYVari==YVari && iVari2==Vari2)
    {
        bIsSamePos=true;
    }
    else
    {
        bIsSamePos  =false;
        iTempX      =XPos;
        iTempY      =YPos;
        iTempVari   =Vari;
        iTempYVari  =YVari;
        iTempVari2  =Vari2;
    }
}
//------------------------------------------------------------------------------
void TMyHotPlatePlaceLog::SetPosition()                                         //Steven 20211110 : 記錄放料到加熱盤的位置
{
    if(bIsSamePos)
    {
        ShowErrorMessage("JAM0159", K_SKIP, MInArmX);
#if 0 // AI(W6.2-INARM) 20260626 TODO(W7: state-record UI): avoid TSpeedButton/TObject
        fMain->sbStateRecordClick(fMain->sbStateRecord);
#endif
        fAllMotorHome=false;
    }
    else
    {
        iX      =iTempX;
        iY      =iTempY;
        iVari   =iTempVari;
        iYVari  =iTempYVari;
        iVari2  =iTempVari2;
    }
}
//------------------------------------------------------------------------------
bool CheckHasSpaceToPlace_9045()
{
    int iCT=0;
    int PlactCT=0;
    for(int i=0; i<2; i++)
    {
        for(int j=0; j<HotPlateForm.XDivision; j++)
        {
            for(int k=0; k<HotPlateForm.YDivision; k++)
            {
                if(HotPlateForm.iPlateSelect & 2-i)                             //MMPlate1+0=NO 2 HP
                {
                    if(MOT[MMPlate1+i].Tray.Data[j][k]==NULL_IC)
                        iCT++;
                }

                if(bRunAutoSiteMapping==true && MOT[MMPlate1+i].Tray.SiteMapData[j][k]==1)                              //Ifor 20210426 add:Site Mapping 取料後需補回IC
                    return true;
            }
        }
    }

    if(USE_PICKER_COUNT==ep1Picker)                                             //Ifor 20260223 add:單一吸嘴模組 Hotplate需放滿
    {
        PlactCT=0;
    }
    else if(iInArmType==e9045_1x1_1 ||
            iInArmType==e9045_1x4_1_Ac)
    {
        if(CUSTOMER_CODE==CC_ASE_SG)                                            //Ifor 20260223 add: ASE SG 要求Single Site Hotplate需放滿
        {
            PlactCT=0;
        }
        else
        {
            PlactCT=1;
        }
    }
    else if(iInArmType==e9045_1x2_2_13 ||
            iInArmType==e9045_1x2_2_14 )                                        //Ifor 20260130 add: Dual Site 加熱盤放滿
    {
        if((HotPlateForm_File.XDivision*HotPlateForm_File.YDivision)%2==0)
            PlactCT=0;
        else
            PlactCT=4;
    }
    else if(bUseAxExPicker())
    {
        if(iInArmType==e9045_2x2_4_13 &&
           HotPlateForm.XDivision==6 &&
           HotPlateForm.YDivision==11)
        {
           if(HotPlateForm.iPlateSelect==3)
           {
               PlactCT=12;                                                      //Sam 20250904 : 修正加熱盤沒有留空格問題。
           }
           else
           {
               PlactCT=6;                                                       //Sam 20250904 : 修正加熱盤沒有留空格問題。
           }
        }
        else
        {
            PlactCT=2*InArmSuck.iPickRow;                                       //Sam 20250904 : 修正加熱盤沒有留空格問題。
        }
    }
    else if(bUseAxxGPicker())
    {
        if(HotPlateForm.XDivision==6)
        {
            PlactCT=6;
        }
        else if(HotPlateForm.XDivision==3 &&
                TestIF.iTestMode==TriSite1X3 &&                                 //Steven 20241126 : fixed for 1x3mode run 3x7HP
                HotPlateForm.XPitch<=(iXpitchMaxX3/2))
        {
            PlactCT=3;
        }
        else if(iInArmType==e9045_1x2_2_14 &&                                   //Jimmychiu 20251208 : fixed for 1x2_14 same position error
                HotPlateForm.XDivision==2)
        {
            PlactCT=2;
        }
        else
        {
            PlactCT=4;
        }
    }
    else
    {
        if(HotPlateForm.XDivision==4 ||
           HotPlateForm.XDivision==8 ||
           HotPlateForm.XDivision==10 ||
           HotPlateForm.XDivision==12 ||
           HotPlateForm.XDivision==16)                                          //Steven 20150826 : 16x24 Hot Plate for 32Site
        {
            if(b12x16HP_2x6==true)                                              //Steven 20241203 : for 12x16 HP
                PlactCT=6;
            else
                PlactCT=8;
        }
        else if(HotPlateForm.XDivision==6)
        {
            if(i8PickerHPMode==iHPWideHP &&
               (iInArmType==e9045_1x3_4 ||                                      //Steven 20240415 : 1x3 3吸嘴跑6x11寬版要擺滿
                iInArmType==e9045_2x3_6 ||
                iInArmType==e9045_2x6_8))
            {
                PlactCT=3*InArmSuck.iPickRow;
            }
            else if(HotPlateForm.iPlateSelect==3)
            {
                if(iCloseSiteState==1 ||
                   iCloseSiteState==2 ||
                   iInArmType==e9045_1x4_4)                                     //jou 20240910 : 修正6x11 Qual site 1x4 hang up
                {
                    PlactCT=8;
                }
                else
                {
                    PlactCT=12;
                }
            }
            else
            {
                PlactCT=10;
            }
        }
    }

    if(iCT>PlactCT)
        return true;

    if(CosFunction.bAutoSiteMappingUseHotPlate==true    &&
       IniConfig.bI21AutoSiteMappingUseHotplate==true   &&
       LastSet.iRunStartMode==rsmAutoSiteMap            &&
       IniConfig.bI21EnableASM==true                    &&
       bAutoSiteMapHotplateReady==false                 )                       //Ifor 20180221 (Steven) : add 再Auto Site Map模式下才執行
    {
        bAutoSiteMapHotplateReady=true;                                         //Ifor 20170920 (Steven) : add Auto Site Mapping Hotplate 旗標
    }
    return false;
}
//==============================================================================
int GetHotPlateColStep(int iAction)
{
    int iStepHP=1;
    int iPlateC=(iAction==iPlaceHP)?iPlacePlateX[0]:iPickPlateX[0];
    int iPlateR=(iAction==iPlaceHP)?iPlacePlateY[0]:iPickPlateY[0];

    if(iInArmType==e9045_1x1_1 ||
       iInArmType==e9045_1x4_1_Ac)
    {
        iStepHP=1;
    }
    else if(InArmSuck.iPickCol==2)
    {
        iStepHP=2;
        if((iInArmType==e9045_1x2_2_14 ||
            (iInArmType==e9045_1x3_2_14 &&
             InArmSuck.iModeX==3)) &&                                           //Steven 20251113 : for 1x3_14 run 3x7 HP
           HotPlateForm.XDivision==3 &&
           iPlateC==2)
        {
            iStepHP=1;
        }
    }
    else
    {
        if(iAction==iPlaceHP)
        {
            if(HotPlateForm.XDivision==6)
            {
                if(i8PickerHPMode==iHPWideHP)
                {
                    if(iInArmType==e9045_1x3_4 ||                               //Steven 20240415 : 1x3 3吸嘴跑6x11寬版要擺滿
                       iInArmType==e9045_2x3_6 ||
                       iInArmType==e9045_2x6_8)
                    {
                        iStepHP=3;
                    }
                    else
                    {
                        if(iPlateC==4)
                            iStepHP=2;
                        else
                            iStepHP=4;
                    }
                }
                else
                {
                    iStepHP=2;
                }
            }
            else if(HotPlateForm.XDivision==10)
            {
                if(iPlateC==8)
                    iStepHP=2;
                else
                    iStepHP=4;
            }
            else
            {
                iStepHP=4;
            }
        }
        else                                                                    //if(iAction==iPickHP)
        {
            if(HotPlateForm.XDivision==6)
            {
                if(i8PickerHPMode==iHPWideHP)
                {
                    if(iInArmType==e9045_1x3_4 ||                               //Steven 20240415 : 1x3 3吸嘴跑6x11寬版要擺滿
                       iInArmType==e9045_2x3_6 ||
                       iInArmType==e9045_2x6_8)
                    {
                        iStepHP=1;
                    }
                    else
                    {
                        if(iPlateC==4)
                            iStepHP=2;
                        else
                            iStepHP=4;
                    }
                }
                else
                {
                    iStepHP=2;
                }
            }
            else if(HotPlateForm.XDivision==10)
            {
                if(iPlateC==8)
                {
                    if(InArmLeftSideHasIC() && iPlateR%2==1)
                    {
                        iStepHP=4;
                    }
                    else
                    {
                        iStepHP=2;
                    }
                }
                else
                {
                    iStepHP=4;
                }
            }
            else
            {
                iStepHP=4;
            }
        }
    }
    return iStepHP;
}
//==============================================================================
int GetPlaceToHotPlateSuckCol(int j)
{
    int j2=j;

    if(iInArmType==e9045_1x1_1 ||
       iInArmType==e9045_1x4_1_Ac)
    {
        if(TestIF_File.bSingleUseOtherSuck || Prod.bSingleInArmUseOtherSuck)    //Steven 20240801 : fixed for 1x1 mode.
            j2=1;
        else
            j2=0;
    }
    else if(bUseAxExPicker())
    {
        j2=j*2;
    }
    else if(bUseAxxGPicker())
    {
        j2=j*3;
        if((iInArmType==e9045_1x2_2_14 ||
            (iInArmType==e9045_1x3_2_14 &&                                      //Steven 20251113 : for 1x3_14 run 3x7 HP
             InArmSuck.iModeX==3)) &&
           HotPlateForm.XDivision==3 &&
           iPlacePlateX[0]==2)
        {
            if(iPlacePlateY[0]%2==0)
                j2=0;
            else
                j2=3;
        }
    }
    else
    {
        if(HotPlateForm.XDivision==6)
        {
            if(i8PickerHPMode==iHPWideHP)
            {
                if(iInArmType==e9045_1x3_4 ||                                   //Steven 20240415 : 1x3 3吸嘴跑6x11寬版要擺滿
                   iInArmType==e9045_2x3_6 ||
                   iInArmType==e9045_2x6_8)
                {
                    j2=j+InArmSuck.iWhichKit;                                   //Steven 20240415 : kit1時,從吸嘴B開始
                }
                else
                {
                    j2=j+iForPlaceHPX6Step*2;
                }
            }
            else
            {
                j2=j*2+iForPlaceHPX6Step;
            }
        }
        else if(HotPlateForm.XDivision==10)
        {
            if(iPlacePlateX[0]==8 && InArmLeftSideNoIC())
                j2=j+2;
            else
                j2=j;
        }
        else
        {
            j2=j;
        }
    }
    return j2;
}
//==============================================================================
int GetPlaceToHotPlateCol(int j)
{
    int ix=0;
    if(iInArmType==e9045_1x1_1 ||
       iInArmType==e9045_1x4_1_Ac)
    {
        ix=j;
    }
    else if(bUseAxExPicker())
    {
        if(iInArmType==e9045_1x2_2_13 &&                                        //Steven 20250109 : modify for 1x2 mode run 3x6HP
           HotPlateForm.XDivision==3 &&
           iPlacePlateX[0]==2 &&
           iPlacePlateY[0]%2==1 &&
           InArmSuck.Item[0][0]==NULL_IC)
        {
            ix=2;
        }
        else if(HotPlateForm.XDivision==4 &&                                    //Steven 20160912 : For 1x2 XPitch>40mm
                bPitchOver12000)
        {
            ix=iPlacePlateX[0]+j;
        }
        else if(HotPlateForm.XDivision==6 &&                                    //jou 2016-06-04 修正1x2 Hotplate 6x11 X Pitch :36 Hang up異常
                bPitchOver12000)
        {
            if(iPlacePlateX[0]!=4)
                ix=iPlacePlateX[0]+j*2;
            else
                ix=iPlacePlateX[0]+j;
        }
        else if(HotPlateForm.XDivision==8 &&
                bPitchOver12000)
        {
            ix=iPlacePlateX[0]+j*(HotPlateForm.XDivision/4);
        }
        else
        {
            ix=iPlacePlateX[0]+j*(HotPlateForm.XDivision/2);
        }
    }
    else if(bUseAxxGPicker())
    {
        if((iInArmType==e9045_1x2_2_14 ||
            (iInArmType==e9045_1x3_2_14 &&
             InArmSuck.iModeX==3)) &&                                           //Steven 20251113 : for 1x3_14 run 3x7 HP
           HotPlateForm.XDivision==3 &&
           iPlacePlateX[0]==2 &&
           iPlacePlateY[0]%2==1)                                               //Steven 20260402 : Fixed for 3x5 HP GetPlaceToHotPlateCol race condition
        {
            fMain->DebugOneCycleHotPlate(AnsiString("GetHPCol:x=")+iPlacePlateX[0]+",y="+iPlacePlateY[0]+",j="+j+",ix=2,Aa="+InArmSuck.Item[0][0]);  //Steven 20260402 : ProposalB log
            ix=2;
        }
        else if(HotPlateForm.XDivision==4 &&                                    //Steven 20160912 : For 1x2 XPitch>40mm
                bPitchOver12000)
        {
            ix=iPlacePlateX[0]+j;
        }
        else if(HotPlateForm.XDivision==6 &&                                    //jou 2016-06-04 修正1x2 Hotplate 6x11 X Pitch :36 Hang up異常
                bPitchOver12000)
        {
            if(iPlacePlateX[0]!=4)
                ix=iPlacePlateX[0]+j*2;
            else
                ix=iPlacePlateX[0]+j;
        }
        else if(HotPlateForm.XDivision==8 &&
                bPitchOver12000)
        {
            ix=iPlacePlateX[0]+j*(HotPlateForm.XDivision/4);
        }
        else if(InArmSuck.iPickCol==2 &&
                HotPlateForm.XDivision==3 &&
                TestIF.iTestMode==TriSite1X3 &&                                 //Steven 20241126 : fixed for 1x3mode run 3x7HP
                InArmSuck.iModeX!=3  &&                                         //Steven 20251113 : for 1x3_14 run 3x7 HP
                HotPlateForm.XPitch<=(iXpitchMaxX3/2))
        {
            if(iPlacePlateX[0]==0)
            {
                if(j==0)
                    ix=0;
                else
                    ix=2;
            }
            else
            {
                ix=1;
            }
        }
        else
        {
            ix=iPlacePlateX[0]+j*(HotPlateForm.XDivision/2);
        }
    }
    else
    {
        if(HotPlateForm.XDivision==4)
        {
            if(iInArmType==e9045_2x8_32 && bPitchOver12000==true)               //JimmyChiu 20250520 : for HP X=4
            {
                ix=iPlacePlateX[0];
            }
            else
            {
                ix=iPlacePlateX[0]+j;
            }
        }
        else if(HotPlateForm.XDivision==6)
        {
            if(i8PickerHPMode==iHPWideHP)
            {
                if(iInArmType==e9045_1x3_4 ||                                   //Steven 20240415 : 1x3 3吸嘴跑6x11寬版要擺滿
                   iInArmType==e9045_2x3_6 ||
                   iInArmType==e9045_2x6_8)
                {
                    ix=iPlacePlateX[0]+j;
                }
                else
                {
                    ix=iPlacePlateX[0]+j;
                }
            }
            else
            {
                ix=iPlacePlateX[0]+j*3;
            }
        }
        else if(HotPlateForm.XDivision==8)
        {
            if(bPitchOver12000)
                ix=iPlacePlateX[0]+j;
            else
                ix=iPlacePlateX[0]+j*2;
        }
        else if(HotPlateForm.XDivision==10)
        {
            if(iPlacePlateX[0]!=8)
                ix=iPlacePlateX[0]+j*2;
            else
                ix=iPlacePlateX[0]+j;
        }
        else if(HotPlateForm.XDivision==12)
        {
            if(b12x16HP_2x6)                                                    //Steven 20241203 : for 12x16 HP
            {
                if(iPlacePlateX[0]==0 || iPlacePlateX[0]==1)
                    ix=iPlacePlateX[0]+j*2;
                else
                    ix=iPlacePlateX[0]-2+j*2;
            }
            else if(bPitchOver12000)
            {
                ix=iPlacePlateX[0]+j;
            }
            else
            {
                ix=iPlacePlateX[0]+j*2;
            }
        }
        else if(HotPlateForm.XDivision==16)                                     //Steven 20150826 : 16x24 Hot Plate for 32Site
        {
            ix=iPlacePlateX[0]+j*4;
        }
    }
    return ix;
}
//==============================================================================
int GetVariableInHotPlateData_AxEx(int iAction)
{
    double dPitch=0.0;
    int iCol  =(iAction==iPlaceHP)?iPlacePlateX[0]:iPickPlateX[0];              //Steven 20241008 : iPlacePlate --> iPlacePlateX
    bPitchOver12000=false;

    if(HotPlateForm.XDivision==2 ||
       HotPlateForm.XDivision==3)
    {
        dPitch=double(HotPlateForm.XPitch)*1.5;                                 //A E之間為1個Pitch, 總共就是1.5個Pitch (XP*1/2*3)
    }
    else if(HotPlateForm.XDivision==4)
    {
        if(HotPlateForm.XPitch>iXpitchMax)
        {
            bPitchOver12000=true;
            dPitch=double(HotPlateForm.XPitch)*1.5;                             //A E之間為1個Pitch, 總共就是1.5個Pitch (XP*1/2*3)
        }
        else
        {
            dPitch=double(HotPlateForm.XPitch)*3.0;                             //A E之間為2個Pitch, 總共就是3.0個Pitch (XP*2/2*3)
        }
    }
    else if(HotPlateForm.XDivision==6)
    {
        dPitch=double(HotPlateForm.XPitch)*4.5;                                 //A E之間為3個Pitch, 總共就是4.5個Pitch (XP*3/2*3)
        if(dPitch>(iXpitchMaxX3))
        {
            if(iCol!=4)
            {
                dPitch=double(HotPlateForm.XPitch)*3.0;                         //A E之間為2個Pitch, 總共就是3.0個Pitch (XP*2/2*3)
            }
            else
            {
                dPitch=double(HotPlateForm.XPitch)*1.5;                         //A E之間為1個Pitch, 總共就是1.5個Pitch (XP*1/2*3)
            }
            bPitchOver12000=true;
        }
    }
    else if(HotPlateForm.XDivision==8)
    {
        dPitch=double(HotPlateForm.XPitch)*6.0;                                 //A E之間為4個Pitch, 總共就是6.0個Pitch (XP*4/2*3)
        if(dPitch>(iXpitchMaxX3))
        {
            dPitch=double(HotPlateForm.XPitch)*3.0;                             //A E之間為2個Pitch, 總共就是3.0個Pitch (XP*2/2*3)
            bPitchOver12000=true;
        }
    }
    else if(HotPlateForm.XDivision==10)
    {
        ShowMyMessage("Function GetVariableInHotPlateData_AxEx XItem 10 is under construction!");
    }
    else if(HotPlateForm.XDivision==12)
    {
        dPitch=double(HotPlateForm.XPitch)*9.0;                                 //A E之間為6個Pitch, 總共就是9個Pitch (XP*6/2*3)
        if(dPitch>(iXpitchMaxX3))
        {
            if(iCol==8 || iCol==9)
                dPitch =double(HotPlateForm.XPitch)*3.0;                        //A E之間為2個Pitch, 總共就是3.0個Pitch (XP*2/2*3)
            else
                dPitch =double(HotPlateForm.XPitch)*6.0;                        //A E之間為4個Pitch, 總共就是6.0個Pitch (XP*4/2*3)
            bPitchOver12000=true;
        }
    }
    else if(HotPlateForm.XDivision==16)
    {
        ShowMyMessage("Function GetVariableInHotPlateData_AxEx XItem 16 is under construction!");
    }
    else
    {
        dPitch=iXpitchMaxX3;
    }
    return int(dPitch);
}
//==============================================================================
int GetVariableInHotPlateData_AxxG(int iAction)
{
    double dPitch=0.0;
    int iCol=(iAction==iPlaceHP)?iPlacePlateX[0]:iPickPlateX[0];                //Steven 20241008 : iPlacePlate --> iPlacePlateX
    bPitchOver12000=false;

    if(HotPlateForm.XDivision==2 ||
       HotPlateForm.XDivision==3)
    {
        if(InArmSuck.iPickCol==2 &&
           HotPlateForm.XDivision==3 &&
           InArmSuck.iModeX!=3 &&                                               //Steven 20251113 : for 1x3_14 run 3x7 HP
           TestIF.iTestMode==TriSite1X3 &&                                      //Steven 20241126 : fixed for 1x3mode run 3x7HP
           HotPlateForm.XPitch<=(iXpitchMaxX3/2))
        {
            if(iCol==0)
                dPitch=double(HotPlateForm.XPitch)*2.0;                         //A G之間為2個Pitch, 總共就是2.0個Pitch (XP*2/3*3)
            else
                dPitch=double(HotPlateForm.XPitch);                             //A G之間為1個Pitch, 總共就是1.0個Pitch (XP*1/3*3)
        }
        else
        {
            dPitch=HotPlateForm.XPitch;                                         //A G之間為1個Pitch, 總共就是1.0個Pitch (XP*1/3*3)
        }
    }
    else if(HotPlateForm.XDivision==4)
    {
        dPitch=double(HotPlateForm.XPitch)*2.0;                                 //A G之間為2個Pitch, 總共就是2.0個Pitch (XP*2/3*3)
        if(dPitch>iXpitchMaxX3)
        {
            dPitch=HotPlateForm.XPitch;                                         //A G之間為1個Pitch, 總共就是1.0個Pitch (XP*1/3*3)
            bPitchOver12000=true;
        }
    }
    else if(HotPlateForm.XDivision==6)
    {
        dPitch=double(HotPlateForm.XPitch)*3.0;                                 //A G之間為3個Pitch, 總共就是3.0個Pitch (XP*2/3*3)
    }
    else if(HotPlateForm.XDivision==8)
    {
        dPitch=double(HotPlateForm.XPitch)*4.0;                                 //A G之間為4個Pitch, 總共就是4.0個Pitch (XP*4/3*3)
        if(dPitch>(iXpitchMaxX3+300))
        {
            dPitch=double(HotPlateForm.XPitch)*2.0;                             //A G之間為2個Pitch, 總共就是2.0個Pitch (XP*2/3*3)
            bPitchOver12000=true;
        }
    }
    else if(HotPlateForm.XDivision==10)
    {
        dPitch=double(HotPlateForm.XPitch)*5.0;                                 //A G之間為5個Pitch, 總共就是5.0個Pitch (XP*4/3*3)
        if(dPitch>(iXpitchMaxX3+300))
        {
            dPitch=double(HotPlateForm.XPitch)*2.0;                             //A G之間為2個Pitch, 總共就是2.0個Pitch (XP*2/3*3)
            bPitchOver12000=true;
        }
    }
    else if(HotPlateForm.XDivision==12)
    {
        dPitch=double(HotPlateForm.XPitch)*6.0;                                 //A G之間為6個Pitch, 總共就是6.0個Pitch (XP*6/3*3)
        if(dPitch>(iXpitchMaxX3+300))
        {
            dPitch=double(HotPlateForm.XPitch)*3.0;                             //A G之間為3個Pitch, 總共就是2.0個Pitch (XP*2/3*3)
            bPitchOver12000=true;
        }
    }
    else if(HotPlateForm.XDivision==16)
    {
        dPitch=double(HotPlateForm.XPitch)*4.0;                                 //A G之間為4個Pitch, 總共就是3.0個Pitch (XP*4/3*3)
        if(dPitch>(iXpitchMaxX3+300))
        {
            dPitch=double(HotPlateForm.XPitch)*2.0;                             //A G之間為2個Pitch, 總共就是2.0個Pitch (XP*2/3*3)
            bPitchOver12000=true;
        }
    }
    else
    {
        dPitch=iXpitchMaxX3;
    }
    return (int)dPitch;
}
//==============================================================================
int GetVariableInHotPlateData_ACEG(int iAction)
{
    double dPitch=0;
    int iCol =(iAction==iPlaceHP)?iPlacePlateX[0]:iPickPlateX[0];               //Steven 20241008 : iPlacePlate --> iPlacePlateX
    bPitchOver12000=false;

    if(HotPlateForm.XDivision==2 ||
       HotPlateForm.XDivision==3)
    {
        dPitch=HotPlateForm.XPitch;                                             //ACEG之間為1個Pitch, 總共就是3.0個Pitch (XP*1/3*3)
        ShowMyMessage("Function GetVariableInHotPlateData_ACEG XItem 2/3 is under construction!");
    }
    else if(HotPlateForm.XDivision==4)
    {
        dPitch=double(HotPlateForm.XPitch)*3.0;                                 //ACEG之間各1個Pitch, 總共就是3.0個Pitch (XP*1/3*3)
        if(dPitch>(iXpitchMaxX3+300))
        {
            dPitch=double(HotPlateForm.XPitch)*1.5;                             //A E 之間為1個Pitch, 總共就是1.5個Pitch (XP/2*3)
            bPitchOver12000=true;
        }
    }
    else if(HotPlateForm.XDivision==6)
    {
        if(i8PickerHPMode==iHPWideHP)
        {
            if(iInArmType==e9045_1x3_4 ||                                       //Steven 20240415 : 1x3 3吸嘴跑6x11寬版要擺滿
               iInArmType==e9045_2x3_6 ||
               iInArmType==e9045_2x6_8)
            {
                dPitch=double(HotPlateForm.XPitch)*3.0;                         //A E之間為2個Pitch, 總共就是4.5個Pitch (XP*3/2*3)
            }
            else
            {
                dPitch=double(HotPlateForm.XPitch)*3;                           //ACEG之間各1個Pitch, 總共就是3.0個Pitch (XP*1/3*3)
            }
        }
        else
        {
            dPitch=double(HotPlateForm.XPitch)*4.5;                             //A E之間為3個Pitch, 總共就是4.5個Pitch (XP*3/2*3)
        }
    }
    else if(HotPlateForm.XDivision==8)
    {
        dPitch=double(HotPlateForm.XPitch)*6.0;                                 //ACEG之間各2個Pitch, 總共就是6.0個Pitch (XP*2*3)
        if(dPitch>(iXpitchMaxX3+300))
        {
            dPitch=double(HotPlateForm.XPitch)*3.0;                             //ACEG之間各1個Pitch, 總共就是3.0個Pitch (XP*1/3*3)
            bPitchOver12000=true;
        }
    }
    else if(HotPlateForm.XDivision==10)
    {
        //if(iCol!=8)
        //    dPitch=HotPlateForm.XPitch*3*2;
        //else
            dPitch=double(HotPlateForm.XPitch)*3;                               //ACEG之間各1個Pitch, 總共就是3.0個Pitch (XP*1/3*3)
    }
    else if(HotPlateForm.XDivision==12)
    {
        if(b12x16HP_2x6)                                                        //Steven 20241203 : for 12x16 HP
        {
            dPitch=double(HotPlateForm.XPitch)*6.0;                             //ACEG之間各2個Pitch, 總共就是6.0個Pitch (XP*2*3)
        }
        else if(iCol!=8)
        {
            dPitch=double(HotPlateForm.XPitch)*6.0;                             //ACEG之間各2個Pitch, 總共就是6.0個Pitch (XP*2*3)
            if(dPitch>(iXpitchMaxX3+300))                                       //Steven 20140624 : HP X-Item=12, X-Pitch=26.66, In Arm Pitch異常修正
            {
                dPitch=double(HotPlateForm.XPitch)*3.0;                         //ACEG之間各1個Pitch, 總共就是3.0個Pitch (XP*1/3*3)
                bPitchOver12000=true;
            }
        }
        else
        {
            bPitchOver12000=true;
            dPitch=double(HotPlateForm.XPitch)*3;                               //ACEG之間各1個Pitch, 總共就是3.0個Pitch (XP*1/3*3)
        }
    }
    else if(HotPlateForm.XDivision==16)                                         //Steven 20150826 : 16x24 Hot Plate for 32Site
    {
        dPitch=double(HotPlateForm.XPitch)*4*3;                                 //ACEG之間各4個Pitch, 總共就是12.0個Pitch (XP*4*3)
        if(dPitch>(iXpitchMaxX3+300))
        {
            dPitch=double(HotPlateForm.XPitch)*3;                               //ACEG之間各1個Pitch, 總共就是3.0個Pitch (XP*1/3*3)
            bPitchOver12000=true;
        }
    }
    else
    {
        dPitch=iXpitchMaxX3;
    }
    return (int)dPitch;
}
//==============================================================================
void GetInArmZPlaceToHotPlateUsage_3_2Pick_3Site(int iAction)                   //Steven 20241126 : fixed for 1x3mode run 3x7HP
{
    int iCol            =0;
    int iPickC          =0;
    int iStepHP         =GetHotPlateColStep(iAction);
    int gHotPlateIndex  =0;

    if(iPlacePlateX[0]==0)
    {
        for(int j=0; j<iStepHP; j++)
        {
            iPickC=GetPlaceToHotPlateSuckCol(j);
            if(iPickC<InArmSuck.iMaxCol &&
               (iAction==iPlaceHP   && InArmSuck.Item[0][iPickC]!=NULL_IC) ||
               (iAction==iTryPickHP && InArmSuck.Item[0][iPickC]==HAS_TRY_SUCK_IC))
            {
                iCol=iPlacePlateX[0]+gHotPlateIndex;

                if((iCol<HotPlateForm.XDivision)==false)
                    continue;

                bZFlgToHPUsage[0][iPickC]=true;
                if(InArmSuck.Item[0][iPickC]!=HAS_NULL_IC)
                    bZFlgToHP[0][iPickC]=true;
                gHotPlateIndex=2;
            }
        }
    }
    else
    {
        bZFlgToHP[0][0]=false;
        if((iAction==iPlaceHP   && InArmSuck.Item[0][3]!=NULL_IC) ||
           (iAction==iTryPickHP && InArmSuck.Item[0][3]==HAS_TRY_SUCK_IC))
        {
            bZFlgToHPUsage[0][3]=true;
            if(InArmSuck.Item[0][3]!=HAS_NULL_IC)
                bZFlgToHP[0][3]=true;
        }
    }
}
//==============================================================================
void GetInArmZPlaceToHotPlateUsage_3_2Pick(int iAction)
{
    int iCol            =0;
    int iPickC          =0;
    int iStepHP         =GetHotPlateColStep(iAction);
    int gHotPlateIndex  =0;

    for(int j=0; j<iStepHP; j++)
    {
        if(iPlacePlateX[0]<2)
        {
            iPickC=GetPlaceToHotPlateSuckCol(j);
            if(iPickC<InArmSuck.iMaxCol &&
               (iAction==iPlaceHP   && InArmSuck.Item[0][iPickC]!=NULL_IC) ||
               (iAction==iTryPickHP && InArmSuck.Item[0][iPickC]==HAS_TRY_SUCK_IC))
            {
                iCol=iPlacePlateX[0]+gHotPlateIndex;

                if((iCol<HotPlateForm.XDivision)==false)
                    continue;

                bZFlgToHPUsage[0][iPickC]=true;
                if(InArmSuck.Item[0][iPickC]!=HAS_NULL_IC)
                    bZFlgToHP[0][iPickC]=true;
                gHotPlateIndex++;
            }
        }
        else
        {
            if(iForPlaceHPX3Step==0)                                            //Steven 20250109 : modify for 1x2 mode run 3x6HP
            {
                iPickC=0;
                if((iAction==iPlaceHP   && InArmSuck.Item[0][iPickC]!=NULL_IC) ||
                   (iAction==iTryPickHP && InArmSuck.Item[0][iPickC]==HAS_TRY_SUCK_IC))
                {
                    bZFlgToHPUsage[0][iPickC]=true;
                    if(InArmSuck.Item[0][iPickC]!=HAS_NULL_IC)
                        bZFlgToHP[0][iPickC]=true;
                }
            }
            else if(iForPlaceHPX3Step==1)
            {
                if(bUseAxExPicker())
                {
                    iPickC=2;
                }
                else                                                            //if(bUseAxxGPicker())
                {
                    iPickC=3;
                }

                if((iAction==iPlaceHP   && InArmSuck.Item[0][iPickC]!=NULL_IC) ||
                   (iAction==iTryPickHP && InArmSuck.Item[0][iPickC]==HAS_TRY_SUCK_IC))
                {
                    bZFlgToHPUsage[0][iPickC]=true;
                    if(InArmSuck.Item[0][iPickC]!=HAS_NULL_IC)
                        bZFlgToHP[0][iPickC]=true;
                }
            }
            else
            {
                bZFlgToHP[0][iPickC]=false;
            }
        }
    }
}
//==============================================================================
void GetInArmZPlaceToHotPlateUsage_3_2Site(int iAction)                         //Steven 20251113 : for 1x3_14 run 3x7 HP
{
    int iCol            =0;
    int iPickC          =0;
    int iStepHP         =GetHotPlateColStep(iAction);
    int gHotPlateIndex  =0;

    if(iPlacePlateX[0]==0)
    {
        for(int j=0; j<iStepHP; j++)
        {
            iPickC=GetPlaceToHotPlateSuckCol(j);
            if(iPickC<InArmSuck.iMaxCol &&
               (iAction==iPlaceHP   && InArmSuck.Item[0][iPickC]!=NULL_IC) ||
               (iAction==iTryPickHP && InArmSuck.Item[0][iPickC]==HAS_TRY_SUCK_IC))
            {
                iCol=iPlacePlateX[0]+gHotPlateIndex;

                if((iCol<HotPlateForm.XDivision)==false)
                    continue;

                bZFlgToHPUsage[0][iPickC]=true;
                if(InArmSuck.Item[0][iPickC]!=HAS_NULL_IC)
                    bZFlgToHP[0][iPickC]=true;
                gHotPlateIndex=2;
            }
        }
    }
    else
    {
        if(iPlacePlateY[0]%2==0)
        {
            bZFlgToHP[0][3]=false;
            if((iAction==iPlaceHP   && InArmSuck.Item[0][0]!=NULL_IC) ||
               (iAction==iTryPickHP && InArmSuck.Item[0][0]==HAS_TRY_SUCK_IC))
            {
                bZFlgToHPUsage[0][0]=true;
                if(InArmSuck.Item[0][0]!=HAS_NULL_IC)
                    bZFlgToHP[0][0]=true;
            }
        }
        else
        {
            bZFlgToHP[0][0]=false;
            if((iAction==iPlaceHP   && InArmSuck.Item[0][3]!=NULL_IC) ||
               (iAction==iTryPickHP && InArmSuck.Item[0][3]==HAS_TRY_SUCK_IC))
            {
                bZFlgToHPUsage[0][3]=true;
                if(InArmSuck.Item[0][3]!=HAS_NULL_IC)
                    bZFlgToHP[0][3]=true;
            }
        }
    }
}
//==============================================================================
void GetInArmZPlaceToHotPlateUsage(int iAction)
{
    int iPickC      =0;
    int iStepHP     =GetHotPlateColStep(iAction);

    for(int j=0; j<iStepHP; j++)
    {
        iPickC=GetPlaceToHotPlateSuckCol(j);

        if(iPlaceHPOrder==0)
        {
            if(iPickC<InArmSuck.iMaxCol &&
               (iAction==iPlaceHP   && InArmSuck.Item[0][iPickC]!=NULL_IC) ||
               (iAction==iTryPickHP && InArmSuck.Item[0][iPickC]==HAS_TRY_SUCK_IC))
            {
                bZFlgToHPUsage[0][iPickC]=true;
                if(InArmSuck.Item[0][iPickC]!=HAS_NULL_IC)
                    bZFlgToHP[0][iPickC]=true;
                if(iInArmType==e9045_2x8_32 && HotPlateForm.XPitch>iXpitchMax)  //JimmyChiu 20250520 : for HP X=4
                {
                    return;
                }
            }

            if(Row2CanPutHP(PlaceMode))
            {
                if((iAction==iPlaceHP   && InArmSuck.Item[1][iPickC]!=NULL_IC) ||
                   (iAction==iTryPickHP && InArmSuck.Item[1][iPickC]==HAS_TRY_SUCK_IC))
                {
                    bZFlgToHPUsage[1][iPickC]=true;
                    if(InArmSuck.Item[1][iPickC]!=HAS_NULL_IC)
                        bZFlgToHP[1][iPickC]=true;
                    if(iInArmType==e9045_2x8_32 && HotPlateForm.XPitch>iXpitchMax)
                    {
                        return;
                    }
                }
            }
        }
        else
        {
            if(iPickC<InArmSuck.iMaxCol &&
               (iAction==iPlaceHP   && InArmSuck.Item[1][iPickC]!=NULL_IC) ||
               (iAction==iTryPickHP && InArmSuck.Item[1][iPickC]==HAS_TRY_SUCK_IC))
            {
                bZFlgToHPUsage[1][iPickC]=true;
                if(InArmSuck.Item[1][iPickC]!=HAS_NULL_IC)
                    bZFlgToHP[1][iPickC]=true;
                if(iInArmType==e9045_2x8_32 && HotPlateForm.XPitch>iXpitchMax)
                {
                    return;
                }
            }
        }
    }
}
//==============================================================================
//void GetInArmZPlaceToHotPlateUsage_1by1(int iAction)
//{
//    int iPickC      =0;
//    int iStepHP     =GetHotPlateColStep(iAction);
//
//    for(int j=0; j<iStepHP; j++)
//    {
//        iPickC=GetPlaceToHotPlateSuckCol(j);
//
//        if(iPlaceHPOrder==0)
//        {
//            if((iAction==iPlaceHP   && InArmSuck.Item[0][iPickC]!=NULL_IC) ||
//               (iAction==iTryPickHP && InArmSuck.Item[0][iPickC]==HAS_TRY_SUCK_IC))
//            {
//                bZFlgToHPUsage[0][iPickC]=true;
//                if(InArmSuck.Item[0][iPickC]!=HAS_NULL_IC)
//                    bZFlgToHP[0][iPickC]=true;
//
//                return;
//            }
//
//            if(Row2CanPutHP(PlaceMode))
//            {
//                if((iAction==iPlaceHP   && InArmSuck.Item[1][iPickC]!=NULL_IC) ||
//                   (iAction==iTryPickHP && InArmSuck.Item[1][iPickC]==HAS_TRY_SUCK_IC))
//                {
//                    bZFlgToHPUsage[1][iPickC]=true;
//                    if(InArmSuck.Item[1][iPickC]!=HAS_NULL_IC)
//                        bZFlgToHP[1][iPickC]=true;
//                }
//
//                return;
//            }
//        }
//        else
//        {
//            if((iAction==iPlaceHP   && InArmSuck.Item[1][iPickC]!=NULL_IC) ||
//               (iAction==iTryPickHP && InArmSuck.Item[1][iPickC]==HAS_TRY_SUCK_IC))
//            {
//                bZFlgToHPUsage[1][iPickC]=true;
//                if(InArmSuck.Item[1][iPickC]!=HAS_NULL_IC)
//                    bZFlgToHP[1][iPickC]=true;
//
//                return;
//            }
//        }
//    }
//}
//==============================================================================
int GetVariableXInHotPlateData(int iAction)
{
    int iPitch=iXpitchMax;
    if(iInArmType==e9045_1x1_1    ||
       iInArmType==e9045_1x4_1_Ac)
    {
        bPitchOver12000=false;
        iPitch=iXpitchMaxX3;
    }
    else if(bUseAxExPicker())
    {
        iPitch=GetVariableInHotPlateData_AxEx(iAction);
    }
    else if(bUseAxxGPicker())
    {
        iPitch=GetVariableInHotPlateData_AxxG(iAction);
    }
    else if(iInArmType==e9045_1x2_4_Hot)                                        // ||
//            iInArmType==e9045_1x4_8_Hot ||                                    //Sam 20250827 : mark
//            iInArmType==e9045_2x2_8_Hot)
    {
    }
    else if(iInArmType==e9045_2x8_32 && HotPlateForm.XPitch>iXpitchMax)
    {
        bPitchOver12000=true;
        iPitch=iXpitchMaxX3;
    }
    else
    {
        iPitch=GetVariableInHotPlateData_ACEG(iAction);
    }
    return iPitch;
}
//==============================================================================
void GetInArmZToHPPos(int iAction, int iCT)
{
    for(int i=0; i<2; i++)
    {
        for(int j=0; j<4; j++)
        {
            if(bZFlgToHP[i][j]==true)
            {
                if(iAction==iPlaceHP)
                {
                    if(iPlacePlate[0]==0)
                        iZPosToHP[i][j]=Prod.ZInArm_Plate2_Place[i][j];
                    else
                        iZPosToHP[i][j]=Prod.ZInArm_Plate1_Place[i][j];
                }
                else
                {
                    if(iPickPlate[0]==0)
                        iZPosToHP[i][j]=Prod.ZInArm_Plate2_Pick[i][j]+iCT*ArmSpeed[InArm].dRetryDown;
                    else
                        iZPosToHP[i][j]=Prod.ZInArm_Plate1_Pick[i][j]+iCT*ArmSpeed[InArm].dRetryDown;
                }
            }
            else
            {
                iZPosToHP[i][j]=ZSafePos;
            }
        }
    }
}
//==============================================================================
void GetInArmZToHPPlaceFlag(int iAction)
{
    ZeroMemory(bZFlgToHP, sizeof(bZFlgToHP));
    ZeroMemory(bZFlgToHPUsage, sizeof(bZFlgToHPUsage));

    if(CosFunction.bUSEJCETSiteMapMode==true &&                                 //Steven 20220527 : for JCET Auto Site Map
       IniConfig.bI21AutoSiteMappingUseHotplate==true &&
       bAutoSiteMapHasPickHP==true)
    {
        bZFlgToHP     [iAutoSiteMapInArmRow][iAutoSiteMapInArmCol]=true;
        bZFlgToHPUsage[iAutoSiteMapInArmRow][iAutoSiteMapInArmCol]=true;
    }
    else
    {
        if(InArmSuck.iPickCol==2 &&
           HotPlateForm.XDivision==3)
        {
            if(TestIF.iTestMode==TriSite1X3 &&                                  //Steven 20241126 : fixed for 1x3mode run 3x7HP
               HotPlateForm.XPitch<=(iXpitchMaxX3/2) &&
               InArmSuck.iModeX!=3)                                             //Steven 20251113 : for 1x3_14 run 3x7 HP
                GetInArmZPlaceToHotPlateUsage_3_2Pick_3Site(iAction);
            else
                GetInArmZPlaceToHotPlateUsage_3_2Pick(iAction);
        }
//        else if(b4x11HP && bPitchOver12000==true)
//        {
//            GetInArmZPlaceToHotPlateUsage_1by1(iAction);
//        }
        else
        {
            GetInArmZPlaceToHotPlateUsage(iAction);
        }
    }

    GetInArmZToHPPos(iAction, 0);
}
//==============================================================================
int GetInArmToHPOffset_9045(int iAction)
{
    int iOffsetPos=-1;
    int iP=(iAction==iPlaceHP)?iPlacePlate[0]:iPickPlate[0];
    if(TestIF.bEnableAutoAlignment==false)                                      //KenHsieh 20220923 : add AOA功能開啟不補償
    {
        if(IniConfig.bE34InOutArmPitchZOffsetSameOne==true)
        {
            iOffsetPos=InOfsInSh1;
        }
        else
        {
            if(iP==0)
                iOffsetPos=InOfsHP2;
            else
                iOffsetPos=InOfsHP1;
        }
    }
    return iOffsetPos;
}
//==============================================================================
int GetVariableYInHotPlateData()                                                //ChungHung 20131231 alter AutoYPitch
{
    int iPitchY=TestIF.iARM_Y_PITCH;
    if(USE_IN_Y_IS_AUTO_PITCH==true)                                            //Steven 20170728 (wei) : Fixed for Y-Pitch handler pick from HP not 60mm  //JerryYang 20251218 : IN/OUT ARM支援不同模組
    {
        if(InArmSuck.iPickRow==1)
        {
            if(MachineTypeChoice==Type_HT9046_LS &&
               SubMachineType==Type_HT9016C)                                    //Jimmychiu 20230207 Add TempPosShift for HT9016C
            {
                if(Prod.LoadForm.iYPitch>IN_OUT_ARM_Y_PITCH_MIN &&
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
                return iPitchY;
            }
        }
        else
        {
            iPitchY=HotPlateForm.YPitch*iYHalf;
            if(iPitchY<IN_OUT_ARM_Y_PITCH_MIN ||
               iPitchY>IN_OUT_ARM_Y_PITCH_MAX)                                  //Min Y pitch 1500 in HT9016c
            {
                iPitchY=TestIF.iARM_Y_PITCH;
            }
            return iPitchY;
        }
    }
    else
    {
        return iPitchY;
    }
}
//==============================================================================
bool MoveInArmXYToHotPlatePlace(int iAction, bool IncludeZ)
{
    int iOffsetPos          =GetInArmToHPOffset_9045(iAction);
    int iMovePitchX         =GetVariableXInHotPlateData(iAction);
    int iMovePitchY         =GetVariableYInHotPlateData();
    int iYVariable          =GetInArmPitchY_9045(iMovePitchY, iOffsetPos);
    int iP                  =(iAction==iPlaceHP || iAction==iTryPickHP)?iPlacePlate[0] :iPickPlate[0];                  //JerryYang 20250328 : fix hot plate check hang up
    int iC                  =(iAction==iPlaceHP || iAction==iTryPickHP)?iPlacePlateX[0]:iPickPlateX[0];
    int iR                  =(iAction==iPlaceHP || iAction==iTryPickHP)?iPlacePlateY[0]:iPickPlateY[0];
    double dMovePitchX      =0;
    static bool bCheckZSafe =false;

    if(bCheckZSafe==false)
    {
        bCheckZSafe=true;
        ResetInToHPFlag();
    }

    if(CosFunction.bUSEJCETSiteMapMode==true &&                                 //Steven 20220527 : for JCET Auto Site Map
       IniConfig.bI21AutoSiteMappingUseHotplate==true &&
       bAutoSiteMapHasPickHP==true)
    {
        iP=iAutoSiteMapHPNo;
        iR=iAutoSiteMapHPR;
        iC=iAutoSiteMapHPC;

        iPlacePlateX[0]=0;                                                      //RogerYang 20250709 重新計算pitch，避免用錯col位置造成Pitch錯誤
        iPlacePlateY[0]=0;                                                      //RogerYang 20250820 : AutoSiteMap完也歸零Y
        iMovePitchX=GetVariableXInHotPlateData(iAction);
    }

    if(iP==0)                                                                   // iPlate=0 先到2號加熱盤也就上上加熱盤
    {
        iInArmToHPX=Prod.XInArm_Plate2_Pick[iInArmYBase][iInArmXBase]+Prod.HotPlateForm[0].iXPitch*iC;
        iInArmToHPY=Prod.YInArm_Plate2_Pick[iInArmYBase][iInArmXBase]-Prod.HotPlateForm[0].iYPitch*iR;
    }
    else
    {
        iInArmToHPX=Prod.XInArm_Plate1_Pick[iInArmYBase][iInArmXBase]+Prod.HotPlateForm[0].iXPitch*iC;
        iInArmToHPY=Prod.YInArm_Plate1_Pick[iInArmYBase][iInArmXBase]-Prod.HotPlateForm[0].iYPitch*iR;
    }

    if(USE_IN_Y_IS_AUTO_PITCH==true)                                            //ChungHung 20140304 add for AutoYPitch  //JerryYang 20251218 : IN/OUT ARM支援不同模組
    {
        if(ArmRow1NotICForPlaceToPlate()==false)
            iInArmToHPY=iInArmToHPY-iMovePitchY;
    }
    else
    {
        if(ArmRow1NotICForPlaceToPlate()==true)
            iInArmToHPY=iInArmToHPY+iMovePitchY;
    }
    GetInArmZToHPPlaceFlag(iAction);

    if(bZFlgToHP[0][0]==false && bZFlgToHP[0][1]==false && bZFlgToHP[0][2]==false && bZFlgToHP[0][3]==false &&
       bZFlgToHP[1][0]==false && bZFlgToHP[1][1]==false && bZFlgToHP[1][2]==false && bZFlgToHP[1][3]==false)
        return true;

    if(USE_PICKER_COUNT==ep1Picker)
    {
    }
    else
    {
        if(bZFlgToHPUsage[0][0] || bZFlgToHPUsage[1][0])
        {
            if(USE_PICKER_COUNT==ep16Picker)                                    //基準為Ad
            {
                dMovePitchX=double(iMovePitchX)/7.0;
                iInArmToHPX=iInArmToHPX+dMovePitchX*3.0;                        //基準在X-Start位置, Aa 移動到Ad 的位置
            }
            else if(USE_IN_OUT_ARM_Y_PITCH==iXYPitchBb ||                       //基準為Ab //Steven for HT7080
                    USE_IN_OUT_ARM_Y_PITCH==iXYPitchIn_Bb_Out_Bc)               //Ztex 2024.02.24 Add HT-1132
            {
                dMovePitchX=double(iMovePitchX)/3.0;
                iInArmToHPX=iInArmToHPX+dMovePitchX*1.0;                        //基準在X-Start位置, Aa 移動到Ab 的位置
            }
            else                                                                //基準為Ac
            {
                dMovePitchX=double(iMovePitchX)/3.0;
                iInArmToHPX=iInArmToHPX+dMovePitchX*2.0;                        //基準在X-Start位置, Aa 移動到Ac 的位置
            }
        }
        else if(bZFlgToHPUsage[0][1] || bZFlgToHPUsage[1][1])
        {
            if(USE_PICKER_COUNT==ep16Picker)                                    //基準為Ad
            {
                dMovePitchX=double(iMovePitchX)/7.0;
                iInArmToHPX=iInArmToHPX+dMovePitchX*1.0;                        //基準在X-Start位置, Ac 移動到Ad 的位置
            }
            else if(USE_IN_OUT_ARM_Y_PITCH==iXYPitchBb ||                       //基準為Ab //Steven for HT7080
                    USE_IN_OUT_ARM_Y_PITCH==iXYPitchIn_Bb_Out_Bc)               //Ztex 2024.02.24 Add HT-1132
            {
                dMovePitchX=double(iMovePitchX)/3.0;
                iInArmToHPX=iInArmToHPX+dMovePitchX*0.0;                        //基準在X-Start位置, Ab 移動到Ab 的位置
            }
            else                                                                //基準為Ac
            {
                dMovePitchX=double(iMovePitchX)/3.0;
                iInArmToHPX=iInArmToHPX+dMovePitchX*1.0;                        //基準在X-Start位置, Ab 移動到Ac 的位置
            }
        }
        else if(bZFlgToHPUsage[0][2] || bZFlgToHPUsage[1][2])
        {
            if(USE_PICKER_COUNT==ep16Picker)                                    //基準為Ad
            {
                dMovePitchX=double(iMovePitchX)/7.0;
                iInArmToHPX=iInArmToHPX-dMovePitchX*1.0;                        //基準在X-Start位置, Ae 移動到Ad 的位置
            }
            else if(USE_IN_OUT_ARM_Y_PITCH==iXYPitchBb ||                       //基準為Ab //Steven for HT7080
                    USE_IN_OUT_ARM_Y_PITCH==iXYPitchIn_Bb_Out_Bc)               //Ztex 2024.02.24 Add HT-1132
            {
                dMovePitchX=double(iMovePitchX)/3.0;
                iInArmToHPX=iInArmToHPX-dMovePitchX*1.0;                        //基準在X-Start位置, Ac 移動到Ab 的位置
            }
            else                                                                //基準為Ac
            {
                dMovePitchX=double(iMovePitchX)/3.0;
                iInArmToHPX=iInArmToHPX+dMovePitchX*0.0;                        //基準在X-Start位置, Ac 移動到Ac 的位置
            }
        }
        else if(bZFlgToHPUsage[0][3] || bZFlgToHPUsage[1][3])
        {
            if(USE_PICKER_COUNT==ep16Picker)                                    //基準為Ad
            {
                dMovePitchX=double(iMovePitchX)/7.0;
                iInArmToHPX=iInArmToHPX-dMovePitchX*3.0;                        //基準在X-Start位置, Ag移動到Ad 的位置
            }
            else if(USE_IN_OUT_ARM_Y_PITCH==iXYPitchBb ||                       //基準為Ab //Steven for HT7080
                    USE_IN_OUT_ARM_Y_PITCH==iXYPitchIn_Bb_Out_Bc)               //Ztex 2024.02.24 Add HT-1132
            {
                dMovePitchX=double(iMovePitchX)/3.0;
                iInArmToHPX=iInArmToHPX-dMovePitchX*2.0;                        //基準在X-Start位置, Ad 移動到Ab 的位置
            }
            else                                                                //基準為Ac
            {
                dMovePitchX=double(iMovePitchX)/3.0;
                iInArmToHPX=iInArmToHPX-dMovePitchX*1.0;                        //基準在X-Start位置, Ad 移動到Ac 的位置
            }
        }
    }

    for(int i=0; i<X_PITCH_COUNT; i++)
        iInXPToHP[i]=GetInArmPitchX_9045(iMovePitchX, i, iOffsetPos);

    TransferHotPlateRatio((iAction!=iPlaceHP), &iInArmToHPX, &iInArmToHPY);

    if(InArmContinuousMove_9045(iInArmToHPX, iInArmToHPY, iInXPToHP, iYVariable, bZFlgToHP, iZPosToHP, IncludeZ))
    {
        ResetInToHPFlag();
        if(CUSTOMER_CODE!=CC_ASE_SG)
            HPPlaceLog.CheckPosition(iInArmToHPX, iInArmToHPY, iInXPToHP[0], iYVariable, iInXPToHP[1]);                 //Steven 20211110 : 記錄放料到加熱盤的位置
        bCheckZSafe=false;
        return true;
    }
    return false;
}
//------------------------------------------------------------------------------
bool PlaceSpecialPos(int iCol, int iRow, int Mode)
{
    int iPlate;
    int SpecY=0;
    if(Mode==PlaceMode)
        iPlate=iPlacePlate[0];
    else
        iPlate=iPickPlate[0];

    if(((TestIF.iTestMode==QualSite1X4 ||
         TestIF.iTestMode==_8Site1X4) && TestIF.iUseSuckMode==4) ||
         iCloseSiteState==1 || iCloseSiteState==2)                              //ChungHung 20150528 add for 海思 _8Site1x4
    {
        if((TestIF.iTestMode==QualSite1X4 || TestIF.iTestMode==_8Site1X4) && TestIF.iUseSuckMode==4)                    //ChungHung 20150528 add for 海思 _8Site1x4  //ChungHung 20120502 add
        {
            if((iRow%2==0 && iRow!=0) && iCol==0)
            {
                return true;
            }
        }
        else
        {
            if(iPlate==0)
            {
                if(iRow%2==0 && iCol==0)
                    return true;
            }
            else
            {
                SpecY=abs(HotPlateForm.YDivision%(1*2)-1);
                if(iRow%(1*2)==SpecY && iCol==0)
                    return true;
            }
        }
    }
    else
    {
        if(HotPlateForm.YDivision%8==0 && iYHalf==4)
        {
            if((iRow%2)==0 && iCol==0)                                          //jou 2010-08-12
                return true;
        }
        else
        {
            if(iPlate==0 || HotPlateForm.iPlateSelect!=3 ||                     //jou 2010-08-03
              (HotPlateForm.YDivision%2==0 && HotPlateYPitchCanPutAll()==false))
            {
                if(iRow%(iYHalf*2)==0 && iCol==0)
                    return true;
            }
            else
            {
                SpecY=abs(HotPlateForm.YDivision%(iYHalf*2)-iYHalf);
                if(iRow%(iYHalf*2)==SpecY && iCol==0)
                    return true;
            }
        }
    }
    return false;
}
//------------------------------------------------------------------------------
bool CheckHotPlateHasSpace_9045_8_New_V(int iP, int iRow, int iCol, int SpacR, int SpacC, int iSuckRow, int iSuckCol, int state)
{
    bool bfail;
    if(state==0)
        bfail=false;
    else
        bfail=true;

    for(int numR=0; numR<iSuckRow; numR++)
    {
        for(int numC=0; numC<iSuckCol; numC++)
        {
            if((iRow+SpacR*numR>=HotPlateForm.YDivision) ||
               (iCol+SpacC*numC>=HotPlateForm.XDivision))
            {
                bfail=true;
            }
            else if(state==0)                                                   //放置
            {
                if(MOT[MMPlate1+iP].Tray.Data[iCol+SpacC*numC][iRow+SpacR*numR]!=NULL_IC)
                    bfail=true;
            }
            else if(state==1)                                                   //吸取
            {
                if(MOT[MMPlate1+iP].Tray.Data[iCol+SpacC*numC][iRow+SpacR*numR]!=NULL_IC)
                    bfail=false;
            }                                                                   //kevin 20130226 for Hong
            else if(state==3)                                                   //AUTOCLEAN
            {
                if(MOT[MMPlate1+iP].Tray.Data[iCol+SpacC*numC][iRow+SpacR*numR]==NULL_IC      ||
                   MOT[MMPlate1+iP].Tray.Data[iCol+SpacC*numC][iRow+SpacR*numR]==HAS_CLEAN_IC ||
                   MOT[MMPlate1+iP].Tray.Data[iCol+SpacC*numC][iRow+SpacR*numR]==HAS_NULL_CLEAN_IC)
                    bfail=true;
            }                                                                   //kevin 20130226 for Hong
        }
    }
    return !bfail;
}
//------------------------------------------------------------------------------
bool CheckHotPlateHasSpace_9045_8_New_V_OnlyTop(int iP, int iRow, int iCol, int SpacR, int SpacC, int iSuckRow, int iSuckCol, int state)
{
    bool bfail;
    if(state==0)
        bfail=false;
    else
        bfail=true;
    int numR=0;
//    for(int numR=0; numR<iSuckRow; numR++)
    {
        for(int numC=0; numC<iSuckCol; numC++)
        {
            if((iRow+SpacR*numR>=HotPlateForm.YDivision) ||
               (iCol+SpacC*numC>=HotPlateForm.XDivision))
            {
                bfail=true;
            }
            else if(state==0)                                                   //放置
            {
                if(MOT[MMPlate1+iP].Tray.Data[iCol+SpacC*numC][iRow+SpacR*numR]!=NULL_IC)
                    bfail=true;
            }
            else if(state==1)                                                   //吸取
            {
                if(MOT[MMPlate1+iP].Tray.Data[iCol+SpacC*numC][iRow+SpacR*numR]!=NULL_IC)
                    bfail=false;
            }                                                                   //kevin 20130226 for Hong
            else if(state==3)                                                   //AUTOCLEAN
            {
                if(MOT[MMPlate1+iP].Tray.Data[iCol+SpacC*numC][iRow+SpacR*numR]==NULL_IC      ||
                   MOT[MMPlate1+iP].Tray.Data[iCol+SpacC*numC][iRow+SpacR*numR]==HAS_CLEAN_IC ||
                   MOT[MMPlate1+iP].Tray.Data[iCol+SpacC*numC][iRow+SpacR*numR]==HAS_NULL_CLEAN_IC)
                    bfail=true;
            }                                                                   //kevin 20130226 for Hong
        }
    }
    return !bfail;
}
//------------------------------------------------------------------------------
bool CheckHotPlateHasSpace_9045_8_New_V_OnlyBtn(int iP, int iRow, int iCol, int SpacR, int SpacC, int iSuckRow, int iSuckCol, int state)
{
    bool bfail;
    if(state==0)
        bfail=false;
    else
        bfail=true;
    int numR=1;
//    for(int numR=0; numR<iSuckRow; numR++)
    {
        for(int numC=0; numC<iSuckCol; numC++)
        {
            if((iRow+SpacR*numR>=HotPlateForm.YDivision) ||
               (iCol+SpacC*numC>=HotPlateForm.XDivision))
            {
                bfail=true;
            }
            else if(state==0)                                                   //放置
            {
                if(MOT[MMPlate1+iP].Tray.Data[iCol+SpacC*numC][iRow+SpacR*numR]!=NULL_IC)
                    bfail=true;
            }
            else if(state==1)                                                   //吸取
            {
                if(MOT[MMPlate1+iP].Tray.Data[iCol+SpacC*numC][iRow+SpacR*numR]!=NULL_IC)
                    bfail=false;
            }                                                                   //kevin 20130226 for Hong
            else if(state==3)                                                   //AUTOCLEAN
            {
                if(MOT[MMPlate1+iP].Tray.Data[iCol+SpacC*numC][iRow+SpacR*numR]==NULL_IC      ||
                   MOT[MMPlate1+iP].Tray.Data[iCol+SpacC*numC][iRow+SpacR*numR]==HAS_CLEAN_IC ||
                   MOT[MMPlate1+iP].Tray.Data[iCol+SpacC*numC][iRow+SpacR*numR]==HAS_NULL_CLEAN_IC)
                    bfail=true;
            }                                                                   //kevin 20130226 for Hong
        }
    }
    return !bfail;
}
//==============================================================================
void CheckPlateInstall()
{
    if(Prod.bPlateSelect[0] && Prod.bPlateSelect[1])
        return;
    if(iPickPlate[0]==0 && Prod.bPlateSelect[0]==false)
    {
        iPickPlate[0]=1;
        iPickPlateX[0]=0;
        iPickPlateY[0]=0;
    }

    if(iPickPlate[0]==1 && Prod.bPlateSelect[1]==false)
    {
        iPickPlate[0]=0;
        iPickPlateX[0]=0;
        if(TestIF_File.iAutoClean_Tray==eCKPos_HP2 &&
           TestIF_File.iAutoClean_Function==1)                                  //kevin 20130226 autoclean 放在hotplate
            iPickPlateY[0]=4;
        else
            iPickPlateY[0]=0;
    }

    if(iPlacePlate[0]==0 && Prod.bPlateSelect[0]==false)
    {
        iPlacePlate[0]=1;
        iPlacePlateX[0]=0;
        iPlacePlateY[0]=0;
    }

    if(iPlacePlate[0]==1 && Prod.bPlateSelect[1]==false)
    {
        iPlacePlate[0]=0;
        iPlacePlateX[0]=0;
        if(TestIF_File.iAutoClean_Tray==eCKPos_HP2 &&
           TestIF_File.iAutoClean_Function==1)                                  //kevin 20130226 autoclean 放在hotplate
        {
            iPlacePlateY[0]=4;
            iTestData=3;
        }
        else
        {
            iPlacePlateY[0]=0;
        }
    }
}
//==============================================================================
void ResetHotPlateSearchParameter()
{
    for(int i=0; i<2; i++)
    {
        iPickPlate[i]=0 ;
        iPickPlateX[i]=0;
        iPickPlateY[i]=0;
        iPlacePlate[i]=0;
        iPlacePlateX[i]=0;
        iPlacePlateY[i]=0;

        if(i==0 && iPlacePlate[0]==0 &&
           TestIF_File.iAutoClean_Tray==eCKPos_HP2 &&
           TestIF_File.iAutoClean_Function==1)                                  //kevin 20130226 autoclean 確保搜尋資料從0-3開始改為第4排開始超過不改避免順序錯亂
        {
           if(iPickPlateY[i]<=3)
              iPickPlateY[i]=4;
           if(iPlacePlateY[i]<=3)
              iPlacePlateY[i]=4;
           iTestData=10;
        }
    }
    OnlyRow2=false;                                                             //ChungHung 20121001 : 移至cmydef Reset後此參數未清為false 一樣HangUp
    iForPlaceHPX6Step=0;
    iForPickHPX6Step=0;

    iForPlaceHPX10Step=0;                                                       //ChungHung 20150127 add for ATK 10X16 Hotplate
    iForPickHPX10Step=0;                                                        //ChungHung 20150127 add for ATK 10X16 Hotplate

    ResetShuttleWhichKit();                                                     //Steven 20161004
    iInArmOrder=0;
    iPickHPOrder=0;                                                             //ChungHung 20140519 fix hangup 2x4 mode 只開Ad Clean Out後 Site全開 參數位重設 HangUp

//    iPickFromPlate=0;                                                         //Steven 20120822 : 沒用到,馬克掉
    CheckPlateInstall();
}
//------------------------------------------------------------------------------
void SearchPlacePlateXItem_1Suck()
{
    int &iC=iPlacePlateX[0], &iR=iPlacePlateY[0], &iPlate=iPlacePlate[0];
    bool flag;

    int iPlateHalf=0;
    iPlateHalf=HotPlateForm.XDivision;

    int count=0;                                                                //Steven 20120919 : Hang Up 保護
    while(1)
    {
        if(count<iHangUpCount)                                                  //Steven 20120919 : Hang Up 保護
        {
            ClearHotPlateHangUp();
            count++;
        }
        else
        {
            if(tHotPlateHangUp.Off())
            {
                DoHotPlateHangUp(__FUNC__);                                     //Steven 20161102 : 針對加熱盤Hang Up進行紀錄
                count=0;
                break;
            }
        }

        if(iPlate==0 &&
           TestIF_File.iAutoClean_Tray==eCKPos_HP2 &&
           TestIF_File.iAutoClean_Function==1 &&
           iR<4)                                                                //kevin 20150515 autoclean 放在hotplate
            iR=4;

        flag=MOT[MMPlate1+iPlate].Tray.Data[iC][iR];
        if(flag)
        {
            iC++;

            if(iC>=iPlateHalf)
            {
                iC=0;
                iR++;
            }

            if(iR>=MOT[MMPlate1+iPlate].Tray.YItem)
            {
                iC=0;
                iR=0;
                iPlate++;
                if(iPlate>=2)
                    iPlate=0;
                CheckPlateInstall();
            }
        }
        else
        {
            if(iC<0 || iR<0)
            {
                ShowErrorMessage("WAR0151", 0, MInArmX, 0, "SearchPlacePlateXItem_1Suck");                              //放置於加熱盤的位置X或Y小於0
            }
            break;
        }
    }
}
//------------------------------------------------------------------------------
void SearchPlacePlateXItem_All_1Suck()
{
    int &iC=iPlacePlateX[0], &iR=iPlacePlateY[0], &iPlate=iPlacePlate[0];
    int iplateTotal=1;
    if(HotPlateForm.iPlateSelect==0x03)
    {
        iplateTotal=2;
    }

    for(iPlate=0;iPlate<iplateTotal;iPlate++)
    {
        for(int irow=0;irow<HotPlateForm.YDivision;irow++)
        {
            for(int icol=0;icol<HotPlateForm.XDivision;icol++)
            {
                if(MOT[MMPlate1+iPlate].Tray.Data[icol][irow]>0)
                {
                    iC=icol;
                    iR=irow;
                }
            }
        }
    }
    iC=0;
    iR=0;
}
//------------------------------------------------------------------------------
void SearchPlacePlateXItem_1x2Suck()
{
    int &ix=iPlacePlateX[0], &iy=iPlacePlateY[0], &iPlate=iPlacePlate[0];
    static int iix=ix, iiy=(iy==0?0:iy-1), iip=iPlate;

    bool bSuccess;
    //int TargetY=-1;
    int spacX=1;
    int spacY=iYHalf;
    int Row=0;
    int Col=0;
    int state=0;

    if(bUseAxxGPicker())
    {
        if(HotPlateForm.XPitch>iXpitchMaxX3)
            bPitchOver12000=true;
        else
            bPitchOver12000=false;
    }
    else if(HotPlateForm.XPitch>iXpitchMaxX2)                                   //JerryYang 20170531 (wei) dual site 4吸嘴跑Hot plate 6x11, X-Pitch 36.6
    {
        bPitchOver12000=true;
    }

    int count=0;                                                                //Steven 20120919 : Hang Up 保護
    do
    {
        if(count<iHangUpCount)                                                  //Steven 20120919 : Hang Up 保護
        {
            ClearHotPlateHangUp();
            count++;
        }
        else
        {
            if(tHotPlateHangUp.Off())
            {
                DoHotPlateHangUp(__FUNC__);                                     //Steven 20161102 : 針對加熱盤Hang Up進行紀錄
                count=0;
                break;
            }
        }

        if(iPlate==0 && TestIF_File.iAutoClean_Tray==eCKPos_HP2 &&
           TestIF_File.iAutoClean_Function==1 && iy<4)                          //kevin 20150515 autoclean 放在hotplate
            iy=4;

        iForPlaceHPX6Step=0;
        if(bPitchOver12000)                                                     //JerryYang 20170531 dual site 4吸嘴跑Hot plate 6x11, X-Pitch 36.6
        {
            spacX=2;
            spacY=1;
            Row=1;
            if(HotPlateForm.XDivision==4 &&                                     //RogerYang 20250820 : 修正HP pitchX=4500 and DivX=4 只能放左半邊的問題
                bUseAxExPicker()==true)
            {
                Col=1;
            }
            else if(HotPlateForm.XDivision==8 &&                                //Steven 20250930 : 修正HP pitchX=2500 and DivX=8 只能放左半邊的問題
                    bUseAxExPicker()==true)
            {
                Col=2;
            }
            else
            {
                Col=2;
                if(ix==2)
                {
                    ix+=2;
                    spacX=5;
                }
            }
        }
        else
        {
            spacX=HotPlateForm.XDivision/2;
            spacY=iYHalf;
            Row=1;
            Col=2;
        }

        bSuccess=CheckHotPlateHasSpace_9045_8_New_V(iPlate, iy, ix, spacY, spacX, Row, Col, state);
        if(bSuccess==false)
        {
            if(bPitchOver12000==true      &&                                    //RogerYang 20250820 : 修正HP pitchX=4500 and DivX=4 只能放左半邊的問題
                HotPlateForm.XDivision==4 &&
                bUseAxExPicker()==true)
            {
                iy++;
                if(iy>=HotPlateForm.YDivision)
                {
                    iy=0;
                    ix+=2;
                    if(ix>spacX)
                    {
                        ix=0;

                        if(HotPlateForm.iPlateSelect==0x03)
                        {
                            if(iPlate==0)
                                iPlate=1;
                            else
                                iPlate=0;
                        }
                    }
                }
            }
            else if(bPitchOver12000==true      &&                               //Steven 20250930 : 修正HP pitchX=2500 and DivX=8 只能放左半邊的問題
                    HotPlateForm.XDivision==8 &&
                    bUseAxExPicker()==true)
            {
                if(ix==0)
                {
                    ix=1;
                }
                else if(ix==1)
                {
                    ix=4;
                }
                else if(ix==4)
                {
                    ix=5;
                }
                else
                {
                    ix=0;
                    iy++;

                    if(iy>=HotPlateForm.YDivision)
                    {
                        iy=0;

                        if(HotPlateForm.iPlateSelect==0x03)
                        {
                            if(iPlate==0)
                                iPlate=1;
                            else
                                iPlate=0;
                        }
                    }
                }
            }
            else
            {
                ix++;
                if(ix>=spacX)
                {
                    ix=0;
                    iy++;

                    if(iy>=HotPlateForm.YDivision)
                    {
                        iy=0;

                        if(HotPlateForm.iPlateSelect==0x03)
                        {
                            if(iPlate==0)
                                iPlate=1;
                            else
                                iPlate=0;
                        }
                    }
                }
            }
        }
    }while(bSuccess==false);

    return;
}
//------------------------------------------------------------------------------
void SearchPlacePlateXItem_2x2Suck()
{
    int &ix=iPlacePlateX[0], &iy=iPlacePlateY[0], &iPlate=iPlacePlate[0];
    bool bSuccess;
    int spacX=1;
    int spacY=iYHalf;
    int Row=0;
    int Col=0;
    int state=0;

    if(bUseAxxGPicker())
    {
        if(HotPlateForm.XPitch>iXpitchMaxX3)
            bPitchOver12000=true;
        else
            bPitchOver12000=false;
    }
    else if(HotPlateForm.XPitch>iXpitchMaxX2)                                   //JerryYang 20170531 (wei) dual site 4吸嘴跑Hot plate 6x11, X-Pitch 36.6
    {
        bPitchOver12000=true;
    }
    bool bSpecialPlace=false;
    int count=0;                                                                //Steven 20120919 : Hang Up 保護
    do
    {
        if(count<iHangUpCount)                                                  //Steven 20120919 : Hang Up 保護
        {
            ClearHotPlateHangUp();
            count++;
        }
        else
        {
            if(tHotPlateHangUp.Off())
            {
                DoHotPlateHangUp(__FUNC__);                                     //Steven 20161102 : 針對加熱盤Hang Up進行紀錄
                count=0;
                break;
            }
        }

        if(iPlate==0 && TestIF_File.iAutoClean_Tray==eCKPos_HP2 &&
           TestIF_File.iAutoClean_Function==1 && iy<4)                          //kevin 20150515 autoclean 放在hotplate
            iy=4;

        iForPlaceHPX6Step=0;

        if(InArmSuck.iPickRow==2 &&
           HotPlateYPitchCanPutAll() &&
           iy+iYHalf<HotPlateForm.YDivision)                                    //檢查Row2是否可放置 YPitch是否符合 && iy+iYHalf<HotPlateForm.YDivision
        {
            if(bPitchOver12000)                                                 //JerryYang 20170531 (wei) dual site 4吸嘴跑Hot plate 6x11, X-Pitch 36.6
            {
                spacX=2;
                if(HotPlateForm.XDivision==4 &&                                 //RogerYang 20250820 : 修正HP pitchX=4500 and DivX=4 只能放左半邊的問題
                    bUseAxExPicker()==true)
                {
                    Col=1;
                    spacX=4;                                                    //Jimmychiu 20251212 : fixed for 2x2 hotplate search error
                }
                else if(bPitchOver12000==true      &&                           //Steven 20250930 : 修正HP pitchX=2500 and DivX=8 只能放左半邊的問題
                        HotPlateForm.XDivision==8 &&
                        bUseAxExPicker()==true)
                {
                    Col=2;
                }
                else
                {
                    Col=2;
                    if(ix==2)
                    {
                        ix+=2;
                        spacX=5;
                    }
                }
            }
            else
            {
                spacX=HotPlateForm.XDivision/2;
                Col=2;                                                          //KevinCheng 20250912 : 吸嘴改為AxEx後的路徑
            }
            spacY=iYHalf;

            if(ArmRow1NotICForPlaceToPlate() &&
               InArmSuck.HasIC())                                               //Only Row2 HAS_IC
            {
                Row=1;
            }
            else
            {
                Row=2;
            }
        }
        else
        {
            bSpecialPlace=true;
            if(bPitchOver12000)                                                 //JerryYang 20170531 dual site 4吸嘴跑Hot plate 6x11, X-Pitch 36.6
            {
                spacX=2;
                if(HotPlateForm.XDivision==4 &&                                 //KevinCheng 20250912 : 吸嘴改為AxEx後的路徑    //RogerYang 20250820 : 修正HP pitchX=4500 and DivX=4 只能放左半邊的問題
                    bUseAxExPicker()==true)
                {
                    Col=1;
                    spacX=4;                                                    //Jimmychiu 20251212 : fixed for 2x2 hotplate search error
                }
                else if(bPitchOver12000==true      &&                           //Steven 20250930 : 修正HP pitchX=2500 and DivX=8 只能放左半邊的問題
                        HotPlateForm.XDivision==8 &&
                        bUseAxExPicker()==true)
                {
                    Col=2;
                }
                else
                {
                    Col=2;
                    if(ix==2)
                    {
                        ix+=2;
                        spacX=5;
                    }
                }
            }
            else
            {
                spacX=HotPlateForm.XDivision/2;
                Col=2;                                                          //KevinCheng 20250912 : 吸嘴改為AxEx後的路徑
            }

            if(b6x20HP==true ||                                                 //Steven 20240930 : for 6x20 HP
               b4x11HP==true)                                                   //KevinCheng 20250912 : 吸嘴改為AxEx後的路徑
            {
                spacY=iYHalf;
            }
            else
            {
                spacY=1;
            }

            if(ArmRow1NotICForPlaceToPlate() &&
               InArmSuck.HasIC())                                               //KevinCheng 20250912 : 吸嘴改為AxEx後的路徑    //Only Row2 HAS_IC
            {
                Row=1;
            }
            else if(b4x11HP==true &&
                    iy==10)
            {
                Row=1;
            }
            else if(HotPlateForm.XDivision==8  &&                               //JerryYang 20251124 : fix 2x2跑HP 8*16
                    HotPlateForm.YDivision==16 &&
                    iYHalf==3 && (iy==13 || iy==14))
            {
                Row=1;
            }
            else
            {
                Row=2;
            }

            if(HotPlateForm.XDivision*HotPlateForm.YDivision%4!=0)
            {
                if(bPitchOver12000)
                {
                    if(iy+iYHalf>HotPlateForm.YDivision && ix+1>=spacX)
                    {
                        iy=0;
                        ix=0;
                        if(HotPlateForm.iPlateSelect==0x03)
                        {
                            if(iPlate==0)
                                iPlate=1;
                            else
                                iPlate=0;
                        }
                    }

                    if(ix==4)
                        spacX=1;
                }
                else
                {
                    if(iy+iYHalf>=HotPlateForm.YDivision && ix+1>=spacX)
                    {
                        iy=0;
                        ix=0;
                        if(HotPlateForm.iPlateSelect==0x03)
                        {
                            if(iPlate==0)   iPlate=1;
                            else            iPlate=0;
                        }

                        if(InArmSuck.iPickRow==2 &&                             //JerryYang 20251124 : fix
                           HotPlateYPitchCanPutAll() &&
                           iy+iYHalf<HotPlateForm.YDivision)
                        {
                            spacY=iYHalf;
                        }
                    }
                }
            }
        }

        bSuccess=CheckHotPlateHasSpace_9045_8_New_V(iPlate, iy, ix, spacY, spacX, Row, Col, state);
        if(bSuccess==false &&
           bSpecialPlace==true)
        {
            bSuccess=CheckHotPlateHasSpace_9045_8_New_V_OnlyTop(iPlate, iy, ix, spacY, spacX, Row, Col, state);
        }

        if(bSuccess==false)
        {
            if(b6x20HP==true &&                                                 //Steven 20240930 : for 6x20 HP
               InArmSuck.ArmUpSideNoIC()==true &&
               InArmSuck.ArmDownSideNoIC()==false)
            {
                iy=iy+iYHalf;
                bSuccess=CheckHotPlateHasSpace_9045_8_New_V(iPlate, iy, ix, spacY, spacX, Row, Col, state);
                if(bSuccess==false)                                             //不應該成立
                {
                    ix++;
                    iy=iy-iYHalf;
                    if(ix>=spacX)
                    {
                        ix=0;
                        iy++;
                        if(iYHalf>1 &&                                          //Steven 20241022 : 避免連續跳兩次
                           iy%spacY==0)
                        {
                            iy=iy+iYHalf;                                       //Steven 20240930 : iYHalf --> spacY
                        }

                        if(iy>=HotPlateForm.YDivision)                          // ||
        //                   iy+iYHalf>=HotPlateForm.YDivision)                 //Steven 20240704 : Mark for 4x11 HP with 2x2 mode
                        {
                            if(iy>=HotPlateForm.YDivision)
                                iy=0;

                            if(HotPlateForm.iPlateSelect==0x03)
                            {
                                if(iPlate==0)
                                    iPlate=1;
                                else
                                    iPlate=0;
                            }
                        }
                    }
                }
            }
            else if(b4x10HP_2x2==true &&                                        //Steven 20241120 : for 4x10 HP
                    InArmSuck.ArmUpSideNoIC()==true &&
                    InArmSuck.ArmDownSideNoIC()==false)
            {
                iy=iy+iYHalf;
                bSuccess=CheckHotPlateHasSpace_9045_8_New_V(iPlate, iy, ix, spacY, spacX, Row, Col, state);
                if(bSuccess==false)                                             //不應該成立
                {
                    ix++;
                    iy=iy-iYHalf;
                    if(ix>=spacX)
                    {
                        ix=0;
                        iy++;
                        if(iYHalf>1 &&                                          //Steven 20241022 : 避免連續跳兩次
                           iy%spacY==0)
                        {
                            iy=iy+iYHalf;                                       //Steven 20240930 : iYHalf --> spacY
                        }

                        if(iy>=HotPlateForm.YDivision)                          // ||
        //                   iy+iYHalf>=HotPlateForm.YDivision)                 //Steven 20240704 : Mark for 4x11 HP with 2x2 mode
                        {
                            if(iy>=HotPlateForm.YDivision)
                                iy=0;

                            if(HotPlateForm.iPlateSelect==0x03)
                            {
                                if(iPlate==0)
                                    iPlate=1;
                                else
                                    iPlate=0;
                            }
                        }
                    }
                }
            }
            else if(b8x16HP_2x2==true)                                          //Steven 20250107 : for 2x2 run 8x16 HP
            {
                iy++;
                if(iy>=HotPlateForm.YDivision)
                {
                    iy=0;
                    ++ix;
                    if(ix>=HotPlateForm.XDivision/2)
                    {
                        ix=0;
                        if(HotPlateForm.iPlateSelect==0x03)
                        {
                            if(iPlate==0)   iPlate=1;
                            else            iPlate=0;
                        }
                    }
                }
            }
            else if(b4x11HP==true)                                              //KevinCheng 20250912 : 吸嘴改為AxEx後的路徑
            {
                if(InArmSuck.ArmUpSideNoIC()==true &&
                   InArmSuck.ArmDownSideNoIC()==false)
                {
                    iy=iy+iYHalf;
                    if(iy>=HotPlateForm.YDivision)
                        bSuccess=false;
                    else
                        bSuccess=CheckHotPlateHasSpace_9045_8_New_V(iPlate, iy, ix, spacY, spacX, Row, Col, state);
                }

                if(bSuccess==false)
                {
                    ix+=2;
                    if(ix>=spacX)                                               //KevinCheng 20260210 : 多加等於 走左右放
                    {
                        ix=0;
                        iy+=iYHalf;

                        if(iy>=HotPlateForm.YDivision)
                        {
                                iy=0;

                            if(HotPlateForm.iPlateSelect==0x03)
                            {
                                if(iPlate==0)
                                    iPlate=1;
                                else
                                    iPlate=0;
                            }
                        }
                    }
                    else
                    {
                        iy=iy-iYHalf;
                    }
                }
            }
            else if(bPitchOver12000==true      &&                               //Steven 20250930 : 修正HP pitchX=2500 and DivX=8 只能放左半邊的問題
                    bUseAxExPicker()==true)
            {
                if(HotPlateForm.XDivision==4)                                   //Steven 20251217 : fixed for 4x8 X-Pitch 45mm
                {
                    iy++;
                    if(iy>=HotPlateForm.YDivision)
                    {
                        iy=0;
                        ix+=2;
                        if(ix>spacX)
                        {
                            ix=0;

                            if(HotPlateForm.iPlateSelect==0x03)
                            {
                                if(iPlate==0)
                                    iPlate=1;
                                else
                                    iPlate=0;
                            }
                        }
                    }
                }
                else
                {
                    if(ix==0)
                    {
                        ix=1;
                    }
                    else if(ix==1)
                    {
                        ix=4;
                    }
                    else if(ix==4)
                    {
                        ix=5;
                    }
                    else
                    {
                        ix=0;
                        iy++;

                        if(iy>=HotPlateForm.YDivision)
                        {
                            iy=0;

                            if(HotPlateForm.iPlateSelect==0x03)
                            {
                                if(iPlate==0)
                                    iPlate=1;
                                else
                                    iPlate=0;
                            }
                        }
                    }
                }
            }
            else if(bPitchOver12000==true      &&                               //KevinCheng 20251201 : 修正HP XDivision=4 只能放左半邊的問題
                    HotPlateForm.XDivision==4  &&
                    bUseAxExPicker()==true)
            {
                if(iy>=HotPlateForm.YDivision)
                {
                    iy=0;
                    ix+=2;
                    if(ix>spacX)
                        ix=0;

                    if(HotPlateForm.iPlateSelect==0x03)
                    {
                        if(iPlate==0)
                            iPlate=1;
                        else
                            iPlate=0;
                    }
                }
                else
                {
                    iy++;
                }
            }
            else
            {
                if(b6x20HP==true)                                               //Steven 20240930 : for 6x20 HP
                {
                    if(iy==5  || iy==6  || iy==7  || iy==8  || iy==9 ||
                       iy==15 || iy==16 || iy==17 || iy==18 || iy==19)
                    {
                        iy=iy-spacY;
                    }
                }
                else if(b4x10HP_2x2==true)                                      //Steven 20241120 : for 4x10 HP
                {
                    if(iy%2==1)
                    {
                        iy=iy-spacY;
                    }
                }

                ix++;
                if(ix>=spacX)
                {
                    ix=0;
                    if(b4x10HP_2x2==true)                                       //Steven 20241120 : for 4x10 HP
                        iy+=2;
                    else
                        iy++;

                    if(HotPlateForm.XDivision==4  &&
                       HotPlateForm.YDivision==10 && iy==9)                     //Steven 20241108 : fixed for 4x10 HP
                    {
                    }
                    else if(HotPlateForm.XDivision==8  &&                       //JerryYang 20251124 : fix 2x2跑HP 8*16
                            HotPlateForm.YDivision==16 &&
                            iYHalf==3 && (iy==13 || iy==14))
                    {
                    }
                    else if(b4x10HP_2x2==false &&                               //Steven 20241120 : for 4x10 HP
                            spacY>1 &&                                          //Steven 20241022 : 避免連續跳兩次
                            iy%spacY==0)
                    {
                        iy=iy+spacY;                                            //Steven 20240930 : iYHalf --> spacY
                    }

                    if(iy>=HotPlateForm.YDivision)                              // ||
    //                   iy+iYHalf>=HotPlateForm.YDivision)                     //Steven 20240704 : Mark for 4x11 HP with 2x2 mode
                    {
                        if(iy>=HotPlateForm.YDivision)
                            iy=0;

                        if(HotPlateForm.iPlateSelect==0x03)
                        {
                            if(iPlate==0)
                                iPlate=1;
                            else
                                iPlate=0;
                        }
                    }
                }
            }
        }
    }while(bSuccess==false);

    return;
}
//------------------------------------------------------------------------------
void SearchPlacePlateXItem3_2Suck_3Site()                                       //Steven 20241126 : fixed for 1x3mode run 3x7HP
{
    int &x=iPlacePlateX[0], &y=iPlacePlateY[0], &iPlate=iPlacePlate[0];

    int count=0;                                                                //Steven 20120919 : Hang Up 保護
    while(1)
    {
        if(count<iHangUpCount)                                                  //Steven 20120919 : Hang Up 保護
        {
            ClearHotPlateHangUp();
            count++;
        }
        else
        {
            if(tHotPlateHangUp.Off())
            {
                DoHotPlateHangUp(__FUNC__);                                     //Steven 20161102 : 針對加熱盤Hang Up進行紀錄
                count=0;
                break;
            }
        }

        if(iPlate==0 && TestIF_File.iAutoClean_Tray==eCKPos_HP2 &&
           TestIF_File.iAutoClean_Function==1 && y<4)                           //kevin 20150515 autoclean 放在hotplate
            y=4;

        if(HotPlateForm.XPitch<=(iXpitchMaxX3/2))
        {
            for(int j=y; j<HotPlateForm.YDivision; j++)
            {
                for(int i=0; i<HotPlateForm.XDivision; i++)
                {
                    if(TestIF.iSiteMap[0][1]==0)
                    {
                        if(MOT[MMPlate1+iPlate].Tray.Data[i][j]==NULL_IC && i!=1)
                        {
                            x=i;
                            y=j;
                            iForPlaceHPX3Step=0;

                            if(x==1)
                            {
                                if(InArmSuck.Item[0][0]==HAS_NULL_IC)
                                    InArmSuck.SetItemData(0, 0, NULL_IC);
                                iForPlaceHPX3Step=1;
                            }
                            return;
                        }
                    }
                    else
                    {
                        if(MOT[MMPlate1+iPlate].Tray.Data[i][j]==NULL_IC)
                        {
                            x=i;
                            y=j;
                            iForPlaceHPX3Step=0;

                            if(x==1)
                            {
                                if(InArmSuck.Item[0][0]==HAS_NULL_IC)
                                    InArmSuck.SetItemData(0, 0, NULL_IC);
                                iForPlaceHPX3Step=1;
                            }
                            return;
                        }
                    }
                }
//                y=0;
            }
        }
        else
        {
            for(int i=x; i<HotPlateForm.XDivision; i++)
            {
                for(int j=y; j<HotPlateForm.YDivision; j++)
                {
                    if(MOT[MMPlate1+iPlate].Tray.Data[i][j]==NULL_IC)
                    {
                        x=i;
                        y=j;
                        iForPlaceHPX3Step=0;

                        if(x==2 && InArmSuck.Item[0][0]==NULL_IC)
                        {
                            iForPlaceHPX3Step=1;
                        }
                        return;
                    }
                }
                y=0;
            }
        }

        if((HotPlateForm.iPlateSelect&0x02) && (HotPlateForm.iPlateSelect&0x01))
            iPlate++;
        else if(HotPlateForm.iPlateSelect&0x02)
            iPlate=0;
        else if(HotPlateForm.iPlateSelect&0x01)
            iPlate=1;
        x=0;
        y=0;
        if(iPlate>=2)
            iPlate=0;
    }
}
//------------------------------------------------------------------------------
void SearchPlacePlateXItem3_1x2Suck()
{
    int &x=iPlacePlateX[0], &y=iPlacePlateY[0], &iPlate=iPlacePlate[0];

    int count=0;                                                                //Steven 20120919 : Hang Up 保護
    while(1)
    {
        if(count<iHangUpCount)                                                  //Steven 20120919 : Hang Up 保護
        {
            ClearHotPlateHangUp();
            count++;
        }
        else
        {
            if(tHotPlateHangUp.Off())
            {
                DoHotPlateHangUp(__FUNC__);                                     //Steven 20161102 : 針對加熱盤Hang Up進行紀錄
                count=0;
                break;
            }
        }

        if(iPlate==0 && TestIF_File.iAutoClean_Tray==eCKPos_HP2 &&
           TestIF_File.iAutoClean_Function==1 && y<4)                           //kevin 20150515 autoclean 放在hotplate
            y=4;

//        if(HotPlateForm.YDivision%2==1)                                       //Steven 20250923 : Mark for 3x5 HP hang up
//            MOT[MMPlate1+iPlate].SetTraySingleData(HotPlateForm.XDivision-1, HotPlateForm.YDivision-1, HAS_NULL_IC);        //Steven 20151005 : 改成SetTraySingleData

        for(int i=x; i<HotPlateForm.XDivision; i++)
        {
            for(int j=y; j<HotPlateForm.YDivision; j++)
            {
                if(HotPlateForm.YDivision%2==1 &&                               //Steven 20250923 : Fixed for 3x5 HP hang up
                   i==HotPlateForm.XDivision-1 &&
                   j==HotPlateForm.YDivision-1)
                {
                    continue;
                }

                if(MOT[MMPlate1+iPlate].Tray.Data[i][j]==NULL_IC)
                {
                    x=i;
                    y=j;
                    iForPlaceHPX3Step=0;

                    if(TestIF.iTestMode==DualSite2x1 &&                         //Steven 20171220 (Wei) : Fixed for 1x2 run 2x3HP hang Up
                       TestIF.iSiteMap[1][0]==0)                                //Steven 20211227 : 修正2x2關B排Hang up
                    {
                    }
//                    else if(TestIF.iTestMode!=DualSite2x1 &&                  //Steven 20251113 : for 1x3_14 run 3x7 HP
//                            TestIF.iSiteMap[0][1]==0)                           //ChungHung 20130910 alter for SCK can close site by Index
//                    {
//                    }
                    else
                    {
                        if(x==2 && y%2==1)                                      //Steven 20260402 : Fixed for 3x5 HP 1x2 iForPlaceHPX3Step race condition
                        {
                            iForPlaceHPX3Step=1;
                        }
                    }
                    fMain->DebugOneCycleHotPlate(AnsiString("SearchHP3:x=")+x+",y="+y+",Step="+iForPlaceHPX3Step+",Aa="+InArmSuck.Item[0][0]);  //Steven 20260402 : ProposalA log
                    return;
                }
            }
            y=0;
        }

        if((HotPlateForm.iPlateSelect&0x02) && (HotPlateForm.iPlateSelect&0x01))
            iPlate++;
        else if(HotPlateForm.iPlateSelect&0x02)
            iPlate=0;
        else if(HotPlateForm.iPlateSelect&0x01)
            iPlate=1;
        x=0;
        y=0;
        if(iPlate>=2)
            iPlate=0;
    }
}
//------------------------------------------------------------------------------
void SearchPlacePlateXItem3_2x2Suck()
{
    int &x=iPlacePlateX[0], &y=iPlacePlateY[0], &iPlate=iPlacePlate[0];

    int count=0;                                                                //Steven 20120919 : Hang Up 保護
    while(1)
    {
        if(count<iHangUpCount)                                                  //Steven 20120919 : Hang Up 保護
        {
            ClearHotPlateHangUp();
            count++;
        }
        else
        {
            if(tHotPlateHangUp.Off())
            {
                DoHotPlateHangUp(__FUNC__);                                     //Steven 20161102 : 針對加熱盤Hang Up進行紀錄
                count=0;
                break;
            }
        }

        if(iPlate==0 && TestIF_File.iAutoClean_Tray==eCKPos_HP2 &&
           TestIF_File.iAutoClean_Function==1 && y<4)                           //kevin 20150515 autoclean 放在hotplate
            y=4;

        if(HotPlateForm.YDivision%2==1)                                         //[處理Y方向為奇數]直接修改Tray資料為不能擺放;修改Tray資料是不想再去修改CheckHasSpaceToPlace_9045_1x2_2()
            MOT[MMPlate1+iPlate].SetTraySingleData(HotPlateForm.XDivision-1, HotPlateForm.YDivision-1, HAS_NULL_IC);    //Steven 20151005 : 改成SetTraySingleData

        for(int i=x; i<HotPlateForm.XDivision; i++)
        {
            for(int j=y; j<HotPlateForm.YDivision; j++)
            {
                if(MOT[MMPlate1+iPlate].Tray.Data[i][j]==NULL_IC)
                {
                    x=i;
                    y=j;
                    iForPlaceHPX3Step=0;

                    if(TestIF.iTestMode==DualSite2x1 &&                         //Steven 20171220 (Wei) : Fixed for 1x2 run 2x3HP hang Up
                       TestIF.iSiteMap[1][0]==0)                                //Steven 20211227 : 修正2x2關B排Hang up
                    {
                    }
                    else if(TestIF.iTestMode!=DualSite2x1 && TestIF.iSiteMap[0][1]==0)                                  //ChungHung 20130910 alter for SCK can close site by Index
                    {
                    }
                    else
                    {
                        if(x==2 && InArmSuck.Item[0][0]==NULL_IC)
                        {
                            iForPlaceHPX3Step=1;
                        }
                    }
                    return;
                }
            }
            y=0;
        }

        if((HotPlateForm.iPlateSelect&0x02) && (HotPlateForm.iPlateSelect&0x01))
            iPlate++;
        else if(HotPlateForm.iPlateSelect&0x02)
            iPlate=0;
        else if(HotPlateForm.iPlateSelect&0x01)
            iPlate=1;
        x=0;
        y=0;
        if(iPlate>=2)
            iPlate=0;
    }
}
//------------------------------------------------------------------------------
void SearchPlacePlateXItem4_8Suck()
{
    int &ix=iPlacePlateX[0], &iy=iPlacePlateY[0], &iPlate=iPlacePlate[0];

    bool bSuccess;
    //int TargetY=-1;
    int spacX=1;
    int spacY=iYHalf;
    int Row=0;
    int Col=0;
    int state=0;

    int count=0;                                                                //Steven 20120919 : Hang Up 保護
    do
    {
        if(count<iHangUpCount)                                                  //Steven 20120919 : Hang Up 保護
        {
            ClearHotPlateHangUp();
            count++;
        }
        else
        {
            if(tHotPlateHangUp.Off())
            {
                DoHotPlateHangUp(__FUNC__);                                     //Steven 20161102 : 針對加熱盤Hang Up進行紀錄
                count=0;
                break;
            }
        }

        if(iPlate==0 && TestIF_File.iAutoClean_Tray==eCKPos_HP2 &&
           TestIF_File.iAutoClean_Function==1 && iy<4)                          //kevin 20150515 autoclean 放在hotplate
            iy=4;

        if(InArmSuck.iShtRow==1)                                                //Steven 20240304 : fixed for 2x4 nn mode
        {
            spacX=1;
            spacY=1;
            Row=1;
            Col=4;
        }
        else if(bPitchOver12000)
        {
            spacX=1;
            spacY=1;
            Row=1;
            Col=1;
        }
        else if(HotPlateYPitchCanPutAll() &&                                    //檢查Row2是否可放置 YPitch是否符合 &&
                iy+iYHalf<HotPlateForm.YDivision)                               // iy+iYHalf<HotPlateForm.YDivision
        {
            if(ArmRow1NotICForPlaceToPlate() &&                                 //Only Row2 HAS_IC
               InArmSuck.HasIC())                                               //Hung HasRealIC -> HasIC HangUp
            {
                spacX=1;
                spacY=iYHalf;
                Row=1;
                Col=4;
            }
            else
            {
                spacX=1;
                spacY=iYHalf;
                Row=2;
                Col=4;
            }
        }
        else
        {
            spacX=1;
            spacY=iYHalf;
            Row=1;
            Col=4;
        }

        bSuccess=CheckHotPlateHasSpace_9045_8_New_V(iPlate, iy, ix, spacY, spacX, Row, Col, state);

        if(bSuccess==false)
        {
            if(bPitchOver12000)
            {
                ++ix;
                if(ix>=HotPlateForm.XDivision)                                  //JimmyChiu 20250520 : for HP X=4
                {
                    ix=0;
                    iy++;
                    if(iy>=HotPlateForm.YDivision)
                    {
                        iy=0;
                        if(HotPlateForm.iPlateSelect==0x03)
                        {
                            if(iPlate==0)   iPlate=1;
                            else            iPlate=0;
                        }
                    }
                }
            }
            else
            {
                iy++;
                if(iy>=HotPlateForm.YDivision)
                {
                    iy=0;
                    ++ix;
                    if(ix>=HotPlateForm.XDivision/4)
                    {
                        ix=0;
                        if(HotPlateForm.iPlateSelect==0x03)
                        {
                            if(iPlate==0)   iPlate=1;
                            else            iPlate=0;
                        }
                    }
                }
            }
        }
    }while(bSuccess==false);

    return;
}
//------------------------------------------------------------------------------
void SearchPlacePlateXItem6_2x2_8Suck()
{
    int &ix=iPlacePlateX[0], &iy=iPlacePlateY[0], &iPlate=iPlacePlate[0];

    bool bSuccess;
    //int TargetY=-1;
    int spacX=1;
    int spacY=iYHalf;
    int Row=0;
    int Col=0;
    int state=0;

    int count=0;                                                                //Steven 20120919 : Hang Up 保護
    do
    {
        if(count<iHangUpCount)                                                  //Steven 20120919 : Hang Up 保護
        {
            ClearHotPlateHangUp();
            count++;
        }
        else
        {
            if(tHotPlateHangUp.Off())
            {
                DoHotPlateHangUp(__FUNC__);                                     //Steven 20161102 : 針對加熱盤Hang Up進行紀錄
                count=0;
                break;
            }
        }

        if(iPlate==0 &&
           TestIF_File.iAutoClean_Tray==eCKPos_HP2 &&
           TestIF_File.iAutoClean_Function==1 && iy<4)                          //kevin 20150515 autoclean 放在hotplate
            iy=4;

        if(InArmSuck.Item[0][0]==NULL_IC && InArmSuck.Item[0][2]==NULL_IC &&
           InArmSuck.Item[1][0]==NULL_IC && InArmSuck.Item[1][2]==NULL_IC)
            iForPlaceHPX6Step=1;
        else
            iForPlaceHPX6Step=0;

        spacX=3;
        spacY=iYHalf;
        Col=2;

        if(ix<2)
        {
            if(HotPlateYPitchCanPutAll() &&                                     //檢查Row2是否可放置 YPitch是否符合
               iy+iYHalf<HotPlateForm.YDivision)                                //iy+iYHalf<HotPlateForm.YDivision
            {
                Row=2;
            }
            else
            {
                Row=1;
            }
        }
        else                                                                    //ix==2
        {
            Row=2;
        }

        bSuccess=CheckHotPlateHasSpace_9045_8_New_V(iPlate, iy, ix, spacY, spacX, Row, Col, state);

        if(bSuccess==false)
        {
            if(ix==2)
            {
                iy++;
                if(iYHalf>1 &&                                                  //Steven 20241022 : 避免連續跳兩次
                   iy%iYHalf==0)
                {
                    if(!(iCloseSiteState==1 ||                                  //jou 2010-07-05 2x8 hotplate只開單排會hang up
                         iCloseSiteState==2) &&
                        Row2CanPutHP(PickMode) &&                               //ChungHung 20140526 add Y-Pitch 63.5 hangUp
                        iy%(iYHalf*2)!=0)                                       //kevin 20150107 ChungHung 20141217 fix hangup
                    {
                        iy+=iYHalf;
                    }
                    ix=0;
                }
            }
            else
            {
                ix++;
                if(ix>=2)
                {
                    ix=0;
                    iy++;

                    if(iYHalf>1 &&                                              //Steven 20241022 : 避免連續跳兩次
                       iy%iYHalf==0)
                    {
                        ix=2;
                        iy=iy-iYHalf;
                    }
                }
            }

            if(iy>=HotPlateForm.YDivision)
            {
                iy=0;
                ix=0;
                if(HotPlateForm.iPlateSelect==0x03)
                {
                    if(iPlate==0)   iPlate=1;
                    else            iPlate=0;
                }
            }
        }
    }while(bSuccess==false);
    return;
}
//------------------------------------------------------------------------------
void SearchPlacePlateXItem6_3Suck_ACEx()                                        //Steven 20240415 : 1x3 3吸嘴跑6x11寬版要擺滿
{
    int &ix=iPlacePlateX[0], &iy=iPlacePlateY[0], &iPlate=iPlacePlate[0];

    bool bSuccess;
    //int TargetY=-1;
    int spacX=1;
    int spacY=iYHalf;
    int Row=0;
    int Col=0;
    int state=0;

    int count=0;                                                                //Steven 20120919 : Hang Up 保護
    do
    {
        if(count<iHangUpCount)                                                  //Steven 20120919 : Hang Up 保護
        {
            ClearHotPlateHangUp();
            count++;
        }
        else
        {
            if(tHotPlateHangUp.Off())
            {
                DoHotPlateHangUp(__FUNC__);                                     //Steven 20161102 : 針對加熱盤Hang Up進行紀錄
                count=0;
                break;
            }
        }

        if(iPlate==0 &&
           TestIF_File.iAutoClean_Tray==eCKPos_HP2 &&
           TestIF_File.iAutoClean_Function==1 && iy<4)                          //kevin 20150515 autoclean 放在hotplate
            iy=4;

        iForPlaceHPX6Step=0;

        spacX=1;
        spacY=iYHalf;
        Col=3;
        if(HotPlateYPitchCanPutAll() &&                                         //檢查Row2是否可放置 YPitch是否符合
           iy+iYHalf<HotPlateForm.YDivision)                                    //iy+iYHalf<HotPlateForm.YDivision
        {
            Row=2;
        }
        else
        {
            Row=1;
        }

        bSuccess=CheckHotPlateHasSpace_9045_8_New_V(iPlate, iy, ix, spacY, spacX, Row, Col, state);

        if(bSuccess==false)
        {
            ix+=3;
            if(ix>=6)
            {
                ix=0;
                iy++;

                if(InArmSuck.iPickRow==2 &&
                   iy%iYHalf==0)
                {
                    if(!(iCloseSiteState==1 ||                                  //jou 2010-07-05 2x8 hotplate只開單排會hang up
                         iCloseSiteState==2) &&
                        Row2CanPutHP(PickMode) &&                               //ChungHung 20140526 add Y-Pitch 63.5 hangUp
                        iy%(iYHalf*2)!=0)                                       //kevin 20150107 ChungHung 20141217 fix hangup
                    {
                        iy+=iYHalf;
                    }
                }
            }

            if(iy>=HotPlateForm.YDivision)
            {
                iy=0;
                ix=0;
                if(HotPlateForm.iPlateSelect==0x03)
                {
                    if(iPlate==0)   iPlate=1;
                    else            iPlate=0;
                }
            }
        }
    }while(bSuccess==false);
    return;
}
//------------------------------------------------------------------------------
void SearchPlacePlateXItem6_8Suck()
{
    int &ix=iPlacePlateX[0], &iy=iPlacePlateY[0], &iPlate=iPlacePlate[0];

    bool bSuccess;
    //int TargetY=-1;
    int spacX=1;
    int spacY=iYHalf;
    int Row=0;
    int Col=0;
    int state=0;

    int count=0;                                                                //Steven 20120919 : Hang Up 保護
    do
    {
        if(count<iHangUpCount)                                                  //Steven 20120919 : Hang Up 保護
        {
            ClearHotPlateHangUp();
            count++;
        }
        else
        {
            if(tHotPlateHangUp.Off())
            {
                DoHotPlateHangUp(__FUNC__);                                     //Steven 20161102 : 針對加熱盤Hang Up進行紀錄
                count=0;
                break;
            }
        }

        if(iPlate==0 &&
           TestIF_File.iAutoClean_Tray==eCKPos_HP2 &&
           TestIF_File.iAutoClean_Function==1 && iy<4)                          //kevin 20150515 autoclean 放在hotplate
            iy=4;

        if(ix==0)
        {
            iForPlaceHPX6Step=0;
            if(InArmSuck.iPickRow==2 &&
               HotPlateYPitchCanPutAll() &&                                     //檢查Row2是否可放置 YPitch是否符合 &&
               iy+iYHalf<HotPlateForm.YDivision)                                // iy+iYHalf<HotPlateForm.YDivision
            {
                if(ArmRow1NotICForPlaceToPlate() &&                             //Only Row2 HAS_IC
                   InArmSuck.HasIC())                                           //Hung HasRealIC -> HasIC HangUp
                {
                    spacX=1;
                    spacY=iYHalf;
                    Row=1;
                    Col=4;
                }
                else
                {
                    spacX=1;
                    spacY=iYHalf;
                    Row=2;
                    Col=4;
                }
            }
            else
            {
                spacX=1;
                spacY=iYHalf;
                Row=1;
                Col=4;
            }
            bSuccess=CheckHotPlateHasSpace_9045_8_New_V(iPlate, iy, ix, spacY, spacX, Row, Col, state);
            if(bSuccess==false)
            {
                iy++;
                if(iy>=HotPlateForm.YDivision)
                {
                    iy=0;
                    ix=4;
                }
            }
        }
        else                                                                    //ix==4
        {
            if(InArmSuck.Item[0][0]==NULL_IC && InArmSuck.Item[0][1]==NULL_IC &&
               InArmSuck.Item[1][0]==NULL_IC && InArmSuck.Item[1][1]==NULL_IC)
                iForPlaceHPX6Step=1;
            else
                iForPlaceHPX6Step=0;

            if(InArmSuck.iPickRow==2 &&
               HotPlateYPitchCanPutAll() &&                                     //檢查Row2是否可放置 YPitch是否符合 &&
               iy+iYHalf<HotPlateForm.YDivision)                                // iy+iYHalf<HotPlateForm.YDivision
            {
                if(ArmRow1NotICForPlaceToPlate() &&                             //Only Row2 HAS_IC
                   InArmSuck.HasIC())                                           //Hung HasRealIC -> HasIC HangUp
                {
                    spacX=1;
                    spacY=1;
                    Row=1;
                    Col=2;
                }
                else
                {
                    spacX=1;
                    spacY=iYHalf;
                    Row=2;
                    Col=2;
                }
            }
            else
            {
                spacX=1;
                spacY=1;
                if(InArmSuck.iPickRow==2)
                    Row=2;
                else
                    Row=1;
                Col=2;
            }
            bSuccess=CheckHotPlateHasSpace_9045_8_New_V(iPlate, iy, ix, spacY, spacX, Row, Col, state);
            if(bSuccess==false)
            {
                iy++;
                if(iy>=HotPlateForm.YDivision ||
                   (iy==HotPlateForm.YDivision-1 &&
                   (HotPlateForm.XDivision*HotPlateForm.YDivision)%4==2))       //RogerYang 20250818 : 修正計算異常    //Jimmychiu 20250319 : fixed for hotplate col=6 hang up
                {
                    iy=0;
                    ix=0;
                    if(HotPlateForm.iPlateSelect==0x03)
                    {
                        if(iPlate==0)   iPlate=1;
                        else            iPlate=0;
                    }
                }
            }
        }
    }while(bSuccess==false);

    return;
}
//------------------------------------------------------------------------------
void SearchPlacePlateXItem6_8Suck_NotStandY()
{
    int &x=iPlacePlateX[0], &y=iPlacePlateY[0], &iPlate=iPlacePlate[0];
    bool flag[2][2];
    int i, j;
    int count=0;                                                                //Steven 20120919 : Hang Up 保護
    while(1)
    {
        for(i=0; i<2; i++)
            for(j=0; j<2; j++)
                flag[i][j]=false;

        if(count<iHangUpCount)                                                  //Steven 20120919 : Hang Up 保護
        {
            ClearHotPlateHangUp();
            count++;
        }
        else
        {
            if(tHotPlateHangUp.Off())
            {
                DoHotPlateHangUp(__FUNC__);                                     //Steven 20161102 : 針對加熱盤Hang Up進行紀錄
                count=0;
                break;
            }
        }

        if(iPlate==0 &&
           TestIF_File.iAutoClean_Tray==eCKPos_HP2 &&
           TestIF_File.iAutoClean_Function==1 && y<4)                           //kevin 20150515 autoclean 放在hotplate
            y=4;

        ArmRow1NotICForPlaceToPlate();
        for(j=0; j<2; j++)
        {
            if((x+j*3)>HotPlateForm.XDivision)
                flag[iPlaceHPOrder][j]=true;
            else if((y)>HotPlateForm.YDivision)
                flag[iPlaceHPOrder][j]=true;
            else
                flag[iPlaceHPOrder][j]=MOT[MMPlate1+iPlate].Tray.Data[x+j*3][y];
        }

        if(flag[0][0] || flag[0][1] || flag[1][0] || flag[1][1])
        {
            if(x==2)
            {
                y++;
                if(y%2==0)                                                      //JerryYang 20220923 : Fix hang up
                    x=0;
            }
            else
            {
                x++;
                if(x>=2)
                {
                    x=0;
                    y++;
                }

                if(PlaceSpecialPos(x, y, PickMode))                             //JerryYang 20220923 : Fix hang up
                {
                    x=2;
                    //iy-=4;
                    y-=2;                                                       //ChungHung 20120502 add

                    if(y<0)
                    {
                        iPlate=0;
                        y=HotPlateForm.YDivision+y;
                    }
                }
            }

            if(y==HotPlateForm.YDivision-1)                                     //JerryYang 20220923 : Fix hang up
            {
            }
            else if((x==0 && (y+1+(HotPlateForm.YDivision%2))>HotPlateForm.YDivision))
            {
                if(iPlate==0)
                    iPlate=1;
                else
                    iPlate=0;
                CheckPlateInstall();

                x=0;
                y=0;
            }
            else if(y>=HotPlateForm.YDivision)
            {
                x=2;
                y=0;
            }
        }
        else
        {
            if(iPlaceHPOrder==0)
            {
                if(Row2CanPutHP(PlaceMode))
                    Row2PlaceHPPos(y);
            }

            if(x==0)
            {
                iForPlaceHPX6Step=0;
            }
            else if(x==2)
            {
                if(y%2==0)
                    iForPlaceHPX6Step=0;
                else
                    iForPlaceHPX6Step=1;
            }
            else
            {
                iForPlaceHPX6Step=1;
            }

            if(x<0 || y<0)
            {
                ShowErrorMessage("WAR0151", 0, MInArmX, 0, "SearchPlacePlateXItem6_8Suck_NotStandY");                   //放置於加熱盤的位置X或Y小於0
            }
            break;
        }
    }
}
//------------------------------------------------------------------------------
void SearchPlacePlateXItem8_8Suck()
{
    int &ix=iPlacePlateX[0], &iy=iPlacePlateY[0], &iPlate=iPlacePlate[0];

    bool bSuccess;
    //int TargetY=-1;
    int spacX=1;
    int spacY=iYHalf;
    int Row=0;
    int Col=0;
    int state=0;

    int count=0;                                                                //Steven 20120919 : Hang Up 保護
    do
    {
        if(count<iHangUpCount)                                                  //Steven 20120919 : Hang Up 保護
        {
            ClearHotPlateHangUp();
            count++;
        }
        else
        {
            if(tHotPlateHangUp.Off())
            {
                DoHotPlateHangUp(__FUNC__);                                     //Steven 20161102 : 針對加熱盤Hang Up進行紀錄
                count=0;
                break;
            }
        }

        if(iPlate==0 &&
           TestIF_File.iAutoClean_Tray==eCKPos_HP2 &&
           TestIF_File.iAutoClean_Function==1 && iy<4)                          //kevin 20150515 autoclean 放在hotplate
            iy=4;

        if(bPitchOver12000==true)
            spacX=1;
        else
            spacX=2;

        if(HotPlateYPitchCanPutAll() &&                                         //檢查Row2是否可放置 YPitch是否符合 &&
           iy+iYHalf<HotPlateForm.YDivision)                                    // iy+iYHalf<HotPlateForm.YDivision
        {
            if(ArmRow1NotICForPlaceToPlate() &&                                 //Only Row2 HAS_IC
               InArmSuck.HasIC())                                               //Hung HasRealIC -> HasIC HangUp
            {
                spacY=iYHalf;
                Row=1;
                Col=4;
            }
            else
            {
                spacY=iYHalf;
                Row=2;
                Col=4;
            }
        }
        else
        {
            spacY=iYHalf;
            Row=1;
            Col=4;
        }

        bSuccess=CheckHotPlateHasSpace_9045_8_New_V(iPlate, iy, ix, spacY, spacX, Row, Col, state);
        if(bSuccess==false)
        {
            iy++;
            if(iy>=HotPlateForm.YDivision)
            {
                iy=0;
                if(bPitchOver12000==true)
                {
                    ix+=HotPlateForm.XDivision/2;
                    if(ix>=HotPlateForm.XDivision)
                    {
                        ix=0;
                        if(HotPlateForm.iPlateSelect==0x03)
                        {
                            if(iPlate==0)   iPlate=1;
                            else            iPlate=0;
                        }
                    }
                }
                else
                {
                    ++ix;
                    if(ix>=HotPlateForm.XDivision/4)
                    {
                        ix=0;
                        if(HotPlateForm.iPlateSelect==0x03)
                        {
                            if(iPlate==0)   iPlate=1;
                            else            iPlate=0;
                        }
                    }
                }
            }
        }
    }while(bSuccess==false);

    return;
}
//------------------------------------------------------------------------------
void SearchPlacePlateXItem10_8Suck()                                            //ChungHung 20150127 add for ATK 10X16 Hotplate
{
    int &ix=iPlacePlateX[0], &iy=iPlacePlateY[0], &iPlate=iPlacePlate[0];

    bool bSuccess;
    //int TargetY=-1;
    int spacX=1;
    int spacY=iYHalf;
    int Row=0;
    int Col=0;
    int state=0;

    iCloseSiteState=CloseSiteState(true);                                       //Steven 20210420 : 修正12site關site跑2x4 mode, HP=6x11 hang up
    if(iCloseSiteState!=0 || HotPlateYPitchCanPutAll()==false)                  //ChungHung 20140429 add   //ChungHung 20140430 add
        iYHalf=1;

    int count=0;                                                                //Steven 20120919 : Hang Up 保護
    do
    {
        if(count<iHangUpCount)                                                  //Steven 20120919 : Hang Up 保護
        {
            ClearHotPlateHangUp();
            count++;
        }
        else
        {
            if(tHotPlateHangUp.Off())
            {
                DoHotPlateHangUp(__FUNC__);                                     //Steven 20161102 : 針對加熱盤Hang Up進行紀錄
                count=0;
                break;
            }
        }

        if(iPlate==0 &&
           TestIF_File.iAutoClean_Tray==eCKPos_HP2 &&
           TestIF_File.iAutoClean_Function==1 && iy<4)                          //kevin 20150515 autoclean 放在hotplate
            iy=4;

        iForPlaceHPX10Step=0;                                                   //ChungHung 20150127 add for ATK 10X16 Hotplate

        if(HotPlateYPitchCanPutAll() &&                                         //檢查Row2是否可放置 YPitch是否符合 &&
           iy+iYHalf<HotPlateForm.YDivision)                                    // iy+iYHalf<HotPlateForm.YDivision
        {
            if(ArmRow1NotICForPlaceToPlate() &&                                 //Only Row2 HAS_IC
               InArmSuck.HasIC())                                               //Hung HasRealIC -> HasIC HangUp
            {
                if(ix==8)
                {
                    spacX=1;
                    spacY=iYHalf;
                    Row=1;
                    Col=2;
                }
                else
                {
                    spacX=1;
                    spacY=iYHalf;
                    Row=1;
                    Col=4;
                }
            }
            else
            {
                if(ix==8)
                {
                    spacX=1;
                    spacY=iYHalf;
                    Row=2;
                    Col=2;
                }
                else
                {
                    spacX=1;
                    spacY=iYHalf;
                    Row=2;
                    Col=4;
                }
            }
        }
        else
        {
            if(ix==8)
            {
                spacX=1;
                spacY=iYHalf;
                Row=1;
                Col=2;
            }
            else
            {
                spacX=1;
                spacY=iYHalf;
                Row=1;
                Col=4;
            }
        }

        bSuccess=CheckHotPlateHasSpace_9045_8_New_V(iPlate, iy, ix, spacY, spacX, Row, Col, state);
        if(bSuccess==false)
        {
            if(ix<5)
            {
                ix++;
                if(ix>=5)
                {
                    ix=0;
                    iy++;
                    if(iy>=HotPlateForm.YDivision)
                    {
                        ix=8;
                        iy=0;
                    }
                }
            }
            else if(ix==8)                                                      //ix=8
            {
                iy++;
                if(iy>=HotPlateForm.YDivision)
                {
                    ix=0;
                    iy=0;
                    if(HotPlateForm.iPlateSelect==0x03)
                    {
                        if(iPlate==0)   iPlate=1;
                        else            iPlate=0;
                    }
                }
            }
        }
    }while(bSuccess==false);

    if((InArmSuck.Item[0][0]==NULL_IC && InArmSuck.Item[0][1]==NULL_IC && InArmSuck.Item[0][2] && InArmSuck.Item[0][3]) ||
       (InArmSuck.Item[1][0]==NULL_IC && InArmSuck.Item[1][1]==NULL_IC && InArmSuck.Item[1][2] && InArmSuck.Item[1][3]))
        iForPlaceHPX10Step=2;                                                   //ChungHung 20150127 add for ATK 10X16 Hotplate
    else
        iForPlaceHPX10Step=0;

    return;
}
//------------------------------------------------------------------------------
void SearchPlacePlateXItem12_2x6()                                              //Steven 20241203 : for 12x16 HP
{
    int &ix=iPlacePlateX[0], &iy=iPlacePlateY[0], &iPlate=iPlacePlate[0];
    bool bSuccess;
    //int TargetY=-1;
    int spacX=2;
    int spacY=iYHalf;
    int Row=2;
    int Col=3;
    int state=0;

    iCloseSiteState=CloseSiteState(true);                                       //Steven 20210420 : 修正12site關site跑2x4 mode, HP=6x11 hang up
    if(iCloseSiteState!=0 || HotPlateYPitchCanPutAll()==false)                  //ChungHung 20140429 add   //ChungHung 20140430 add
        iYHalf=1;

    int count=0;                                                                //Steven 20120919 : Hang Up 保護
    do
    {
        if(iPlate==0 &&
           TestIF_File.iAutoClean_Tray==eCKPos_HP2 &&
           TestIF_File.iAutoClean_Function==1 && iy<4)                          //kevin 20150515 autoclean 放在hotplate
            iy=4;

        if(count<iHangUpCount)                                                  //Steven 20120919 : Hang Up 保護
        {
            ClearHotPlateHangUp();
            count++;
        }
        else
        {
            if(tHotPlateHangUp.Off())
            {
                DoHotPlateHangUp(__FUNC__);                                     //Steven 20161102 : 針對加熱盤Hang Up進行紀錄
                count=0;
                break;
            }
        }

        if(USE_IN_OUT_ARM_Y_PITCH!=iXYPitchVariable &&                          //Steven 20250102 : Fixed 2x6 support X=12
           USE_IN_OUT_ARM_Y_PITCH!=iXYPitchIn_Bb_Out_Bc)                        //Ztex 2024.02.24 Add HT-1132
        {
            if(iy==13 || iy==14)
            {
                Row=1;
                spacY=iYHalf;
            }
        }

        bSuccess=CheckHotPlateHasSpace_9045_8_New_V(iPlate, iy, ix, spacY, spacX, Row, Col, state);
        if(bSuccess==false)
        {
            if(USE_IN_OUT_ARM_Y_PITCH==iXYPitchVariable ||
               USE_IN_OUT_ARM_Y_PITCH==iXYPitchIn_Bb_Out_Bc)                    //Ztex 2024.02.24 Add HT-1132
            {
                if(ix==0)                                                       //Kit 0
                {
                    ix=6;
                }
                else if(ix==6)                                                  //Kit 1
                {
                    ix=1;
                }
                else if(ix==1)                                                  //Kit 0, 跑kit1會行程不足
                {
                    ix=7;
                }
                else                                                            //Kit 1
                {
                    ix=0;
                    iy++;
                    if(iYHalf>1 &&
                       iy%iYHalf==0)                                            //Steven 20220317 : Fixed 2x4 support X=12
                    {
                        iy+=iYHalf;
                    }

                    if(iy>=HotPlateForm.YDivision)
                    {
                        ix=0;
                        iy=0;
                        if(HotPlateForm.iPlateSelect==0x03)
                        {
                            if(iPlate==0)   iPlate=1;
                            else            iPlate=0;
                        }
                    }
                }
            }
            else
            {
                if(ix==0)                                                       //Kit 0
                {
                    if(iy==13)
                    {
                        iy=14;
                    }
                    else if(iy==14)
                    {
                        iy=13;
                        ix=6;
                    }
                    else
                    {
                        ix=6;
                    }
                }
                else if(ix==6)                                                  //Kit 1
                {
                    if(iy==13)
                    {
                        iy=14;
                    }
                    else if(iy==14)
                    {
                        iy=13;
                        ix=1;
                    }
                    else
                    {
                        ix=1;
                    }
                }
                else if(ix==1)                                                  //Kit 0, 跑kit1會行程不足
                {
                    if(iy==13)
                    {
                        iy=14;
                    }
                    else if(iy==14)
                    {
                        iy=13;
                        ix=7;
                    }
                    else
                    {
                        ix=7;
                    }
                }
                else if(ix==7 && iy==13)
                {
                    iy=14;
                }
                else if(ix==7 && iy==14)
                {
                    ix=0;
                    iy=0;
                    if(HotPlateForm.iPlateSelect==0x03)
                    {
                        if(iPlate==0)   iPlate=1;
                        else            iPlate=0;
                    }
                }
                else                                                            //Steven 20250102 : Fixed 2x6 support X=12
                {
                    ix=0;
                    iy++;
                    if(iYHalf>1 &&
                       iy%iYHalf==0)                                            //Steven 20220317 : Fixed 2x4 support X=12
                    {
                        iy+=iYHalf;
                    }

                    if(iy>=HotPlateForm.YDivision)
                    {
                        ix=0;
                        iy=0;
                        if(HotPlateForm.iPlateSelect==0x03)
                        {
                            if(iPlate==0)   iPlate=1;
                            else            iPlate=0;
                        }
                    }
                }
            }
        }
        else
        {
            if(InArmSuck.iWhichKit==0)
            {
                if(InArmSuck.Item[0][3]==HAS_NULL_IC)
                    InArmSuck.SetItemData(0, 3, NULL_IC);

                if(InArmSuck.Item[1][3]==HAS_NULL_IC)
                    InArmSuck.SetItemData(1, 3, NULL_IC);
            }
            else
            {
                if(InArmSuck.Item[0][0]==HAS_NULL_IC)
                    InArmSuck.SetItemData(0, 0, NULL_IC);

                if(InArmSuck.Item[1][0]==HAS_NULL_IC)
                    InArmSuck.SetItemData(1, 0, NULL_IC);
            }
        }
    }while(bSuccess==false);
}
//------------------------------------------------------------------------------
void SearchPlacePlateXItem12_8Suck()                                            //ChunHung 20141209 合code有漏整段換掉有漏掉  //ChungHung 20140419 add for 2x4 12 site
{
    int &ix=iPlacePlateX[0], &iy=iPlacePlateY[0], &iPlate=iPlacePlate[0];
    bool bSuccess;
    //int TargetY=-1;
    int spacX=1;
    int spacY=iYHalf;
    int Row=0;
    int Col=0;
    int state=0;

    iCloseSiteState=CloseSiteState(true);                                       //Steven 20210420 : 修正12site關site跑2x4 mode, HP=6x11 hang up
    if(iCloseSiteState!=0 || HotPlateYPitchCanPutAll()==false)                  //ChungHung 20140429 add   //ChungHung 20140430 add
        iYHalf=1;

    int count=0;                                                                //Steven 20120919 : Hang Up 保護
    do
    {
        if(iPlate==0 &&
           TestIF_File.iAutoClean_Tray==eCKPos_HP2 &&
           TestIF_File.iAutoClean_Function==1 && iy<4)                          //kevin 20150515 autoclean 放在hotplate
            iy=4;

        if(count<iHangUpCount)                                                  //Steven 20120919 : Hang Up 保護
        {
            ClearHotPlateHangUp();
            count++;
        }
        else
        {
            if(tHotPlateHangUp.Off())
            {
                DoHotPlateHangUp(__FUNC__);                                     //Steven 20161102 : 針對加熱盤Hang Up進行紀錄
                count=0;
                break;
            }
        }

        iForPlaceHPX6Step=0;
        if(HotPlateYPitchCanPutAll() &&                                         //檢查Row2是否可放置 YPitch是否符合 &&
           iy+iYHalf<HotPlateForm.YDivision)                                    // iy+iYHalf<HotPlateForm.YDivision
        {
            if(ArmRow1NotICForPlaceToPlate() &&                                 //Only Row2 HAS_IC
               InArmSuck.HasIC())                                               //Hung HasRealIC -> HasIC HangUp
            {
                if(ix==8)
                {
                    spacX=1;
                    spacY=iYHalf;
                    Row=1;
                    Col=4;
                }
                else
                {
                    spacX=2;
                    spacY=iYHalf;
                    Row=1;
                    Col=4;
                }
            }
            else
            {
                if(ix==8)
                {
                    spacX=1;
                    spacY=iYHalf;
                    Row=2;
                    Col=4;
                }
                else
                {
                    spacX=2;
                    spacY=iYHalf;
                    Row=2;
                    Col=4;
                }
            }
        }
        else
        {
            if(ix==8)
            {
                spacX=1;
                spacY=iYHalf;
                Row=1;
                Col=4;
            }
            else
            {
                spacX=2;
                spacY=iYHalf;
                Row=1;
                Col=4;
            }
        }

        bSuccess=CheckHotPlateHasSpace_9045_8_New_V(iPlate, iy, ix, spacY, spacX, Row, Col, state);
        if(bSuccess==false)
        {
            if(ix<2)
            {
                ix++;
                if(ix>=2)
                {
                    ix=0;
                    iy++;
                    if(iYHalf>1 &&
                       iy%iYHalf==0)                                            //Steven 20220317 : Fixed 2x4 support X=12
                    {
                        iy+=iYHalf;
                    }

                    if(iy>=HotPlateForm.YDivision)
                    {
                        ix=8;
                        iy=0;
                    }
                }
            }
            else if(ix==8)                                                      //ix=8
            {
                iy++;
                if(iYHalf>1 &&
                   iy%iYHalf==0)                                                //Steven 20220317 : Fixed 2x4 support X=12
                {
                    iy+=iYHalf;
                }

                if(iy>=HotPlateForm.YDivision)
                {
                    ix=0;
                    iy=0;
                    if(HotPlateForm.iPlateSelect==0x03)
                    {
                        if(iPlate==0)   iPlate=1;
                        else            iPlate=0;
                    }
                }
            }
        }
    }while(bSuccess==false);
}
//------------------------------------------------------------------------------
void SearchPlacePlateXItem16_8Suck()                                            //Steven 20150826 : 16x24 Hot Plate for 32Site
{
    int &x=iPlacePlateX[0], &y=iPlacePlateY[0], &iPlate=iPlacePlate[0];
    int iStep=0;
    bool flag[2][4];
    int iLimit=0;

    int iYPitch=0;
    iYPitch=HotPlateForm.YPitch/10;
    iYPitch=iYPitch*10;
    int iRunRute=(iYPitch==0)?0:(TestIF.iARM_HP_Y_PITCH/iYPitch);               //RogerYang 20250618 防止除以零  //ChungHung 20120505 9045WS
    if(iPlate==1 && y==4)                                                       //kevin 20130226
        y=0;

    iStep=4;
    iLimit=4;

    int count=0;                                                                //Steven 20120919 : Hang Up 保護
    while(1)
    {
        for(int i=0; i<2; i++)
            for(int j=0; j<4; j++)
                flag[i][j]=false;

        if(count<iHangUpCount)                                                  //Steven 20120919 : Hang Up 保護
        {
            ClearHotPlateHangUp();
            count++;
        }
        else
        {
            if(tHotPlateHangUp.Off())
            {
                DoHotPlateHangUp(__FUNC__);                                     //Steven 20161102 : 針對加熱盤Hang Up進行紀錄
                count=0;
                break;
            }
        }

        for(int i=0; i<2; i++)
        {
            for(int j=0; j<iStep; j++)
            {
                if(i==1 && iPlaceHPOrder==0 && AllRow1NotUseForHP(iPlaceHP)==false)
                {
                    if(Row2CanPutHP(PlaceMode))
                        flag[i][j]=MOT[MMPlate1+iPlate].Tray.Data[x+j*iLimit][y+iYHalf];
                }
                else
                {
                    flag[i][j]=MOT[MMPlate1+iPlate].Tray.Data[x+j*iLimit][y];
                }
            }
        }

        if(flag[0][0] || flag[0][1] || flag[0][2] || flag[0][3] ||
           flag[1][0] || flag[1][1] || flag[1][2] || flag[1][3])
        {
            x++;
            if(x>=iLimit)
            {
                x=0;
                y++;

                if(y%iRunRute==0 && Row2CanPutHP(PlaceMode))                    //ChungHung 20140512 add fix hangup 不能一次放不能加
                    y=y+iRunRute;
            }

            if(y>=MOT[MMPlate1+iPlate].Tray.YItem)
            {
                x=0;
                y=0;
                iPlate--;
                if(iPlate<0)
                    iPlate=1;
                CheckPlateInstall();
                y=0;
            }
        }
        else
        {
            if(x<0 || y<0)
            {
                ShowErrorMessage("WAR0151", 0, MInArmX, 0, "SearchPlacePlateXItem16_8Suck");                            //放置於加熱盤的位置X或Y小於0
            }

            if(iPlaceHPOrder==0 && AllRow1NotUseForHP(iPlaceHP)==false)
            {
                if(Row2CanPutHP(PlaceMode))
                    Row2PlaceHPPos(y);
            }
            break;
        }
    }
}
//------------------------------------------------------------------------------
void SearchPlateToPlace()                                                       //Steven 20240311 : 重新整理SearchPlateToPlace()
{
    static bool bHasChangePlate=false;

    if(LastSet.iRunStartMode==rsmAutoSiteMap &&                                 //Steven 20220527 : for JCET Auto Site Map
       CosFunction.bUSEJCETSiteMapMode &&
       bAutoSiteMapHasPickHP)
    {
        return;
    }

    if(IniConfig.bI21AutoSiteMappingUseHotplate==true   &&
       bAutoSiteMapHotplateSave==true)                                          //Ifor 20170929 (Steven) : add Auto Site Mapping Hotplate 哪邊取料放回哪邊
    {
        if(bHasChangePlate==false)
        {
            bHasChangePlate=true;
        }

        if(LastSet.iRunStartMode==rsmAutoSiteMap)
        {
            if(USE_PICKER_COUNT==0)                                             //Ifor 20190628 : Fix Auto Site Mapping Hangup
            {
                iPickPlateX[0]=iPickPlateX[0]+1;                                //Ifor 20180709 : 往下ㄧ格取
            }
            else
            {
                if(HotPlateForm.XDivision==6 || HotPlateForm.XDivision==4)
                {
                    if(iPickPlateX[0]>=3)
                    {
                        iPickPlateY[0]=iPickPlateY[0]+2;                        //Ifor 20180709 : 往下ㄧ格取
                    }
                    else
                    {
                        iPickPlateY[0]=iPickPlateY[0]+1;                        //Ifor 20180709 : 往下ㄧ格取
                    }
                    iPickPlateX[0]=0;
                }
                else
                {
                    iPickPlateX[0]=iPickPlateX[0]+1;                            //Ifor 20180709 : 往下ㄧ格取
                }
            }
        }
        return;
    }

    if(bHasChangePlate==true)
    {
        bHasChangePlate=false;
    }

    if(USE_PICKER_COUNT==ep1Picker)
    {
        SearchPlacePlateXItem_1Suck();
    }
    else if(TestIF.iTestMode==DualSite &&
       IniConfig.bDualSiteCloseAbCanFullHotplate &&
       TestIF.iSiteMap[0][1]==0)                                                //Steven 20161213 : 1x2關Site Ab強制跑1x1模式
    {
        SearchPlacePlateXItem_1Suck();
    }
    else if(TestIF.iTestMode==DualSite2x1 &&
            IniConfig.bDualSiteCloseAbCanFullHotplate &&
            TestIF.iSiteMap[1][0]==0)                                           //Steven 20161213 : 2x1關Site Ba強制跑1x1模式
    {
        SearchPlacePlateXItem_1Suck();
    }
    else if(TestIF.iTestMode==SingleSite ||
            iInArmType==e9045_1x4_1_Ac)                                         //Steven 20200720 : 1x4只開site Ac
    {
        SearchPlacePlateXItem_1Suck();
    }
    else if(InArmSuck.iPickRow==1 &&
            (bUseAxExPicker() ||
             bUseAxxGPicker()))
    {
        if(TestIF.iTestMode==TriSite1X3 &&                                      //Steven 20241126 : fixed for 1x3mode run 3x7HP
           HotPlateForm.XDivision==3 &&
           InArmSuck.iModeX!=3)                                                 //Steven 20251113 : for 1x3_14 run 3x7 HP
        {
            SearchPlacePlateXItem3_2Suck_3Site();
        }
        else if(HotPlateForm.XDivision==3)
        {
            SearchPlacePlateXItem3_1x2Suck();
        }
        else
        {
            SearchPlacePlateXItem_1x2Suck();
        }
    }
    else if(InArmSuck.iPickRow==2 &&
            (bUseAxExPicker() ||
             bUseAxxGPicker()))
    {
        if(HotPlateForm.XDivision==3)
            SearchPlacePlateXItem3_2x2Suck();
        else
            SearchPlacePlateXItem_2x2Suck();
    }
    else if(HotPlateForm.XDivision==2)                                          //jou 2010-08-03 增加支援Dual Site,Hotplate X Item = 2
    {
        SearchPlacePlateXItem_1x2Suck();
    }
    else if(HotPlateForm.XDivision==3)                                          //20111015    Dell
    {
        SearchPlacePlateXItem3_1x2Suck();
    }
    else if(HotPlateForm.XDivision==4)                                          //Steven 20240304 : 重新整理
    {
        SearchPlacePlateXItem4_8Suck();
    }
    else if(HotPlateForm.XDivision==6)
    {
        if(i8PickerHPMode==iHPWideHP)                                           //ChungHung Add 20101025 CC_ASE_CL HotPlate Offset 10mm         //Steven 20151117 : 2x2 8Picker at Hot mode
        {
            if(iInArmType==e9045_1x3_4 ||                                       //Steven 20240415 : 1x3 3吸嘴跑6x11寬版要擺滿
               iInArmType==e9045_2x3_6 ||
               iInArmType==e9045_2x6_8)
            {
                SearchPlacePlateXItem6_3Suck_ACEx();
            }
            else
            {
                SearchPlacePlateXItem6_8Suck();
            }
        }
        else if(HotPlateYPitchCanPutAll()==false)
        {
            SearchPlacePlateXItem6_8Suck_NotStandY();
        }
        else
        {
            SearchPlacePlateXItem6_2x2_8Suck();
        }
    }
    else if(HotPlateForm.XDivision==8)
    {
        SearchPlacePlateXItem8_8Suck();
    }
    else if(HotPlateForm.XDivision==10)
    {
        SearchPlacePlateXItem10_8Suck();                                        //ChungHung 20150127 add for ATK 10X16 Hotplate
    }
    else if(HotPlateForm.XDivision==12)
    {
        if(b12x16HP_2x6==true)
            SearchPlacePlateXItem12_2x6();                                      //Steven 20241203 : for 12x16 HP
        else
            SearchPlacePlateXItem12_8Suck();
    }
    else if(HotPlateForm.XDivision==16)                                         //Steven 20150826 : 16x24 Hot Plate for 32Site
    {
        SearchPlacePlateXItem16_8Suck();
    }
    else
    {
        ShowErrorMessage("MES0156", 0, MInArmX, 0, "SearchPlateToPlace");       //加熱盤的資料錯誤
    }

    #ifndef SOFT_SIMULTE                                                        //kevin 20220914 add  SOFT_SIMULTE 不檢查安全門
    CheckSafeDoorIsClosed();                                                    //Sam 20220829 : 發生 HotPlateHangUp 時必須要先檢查安全門
    #endif
}
//------------------------------------------------------------------------------
//Steven 20170109 : 將放料到HP資料交換改成Function
//------------------------------------------------------------------------------
void DoPlaceToHPSwapData(int iSuckRow, int iSuckCol, int iP, int iPlateR, int iPlateC)                                  //Steven 20170109 : 將放料到HP資料交換改成Function
{
    AnsiString Str;

    if(iSuckRow<0  || iSuckCol<0  ||
       iSuckRow>=2 || iSuckCol>=8 ||
       iP<0  || iPlateC<0   || iPlateR<0 ||
       iP>=2 || iPlateC>=50 || iPlateR>=50)
    {
        return;
    }

    iHotWhichShuttle[iP][iPlateC][iPlateR]  =InArmSuck.iWhichSht;
    iHotWhichKit[iP][iPlateC][iPlateR]      =InArmSuck.iWhichKit;
    iHotPlateCount[iP][iPlateC][iPlateR]    =iHotCount;
    PickFromHPList->SetArrPlateXY(iSuckRow, iSuckCol, iP, iPlateR, iPlateC, InArmSuck.iWhichSite[iSuckRow][iSuckCol]);  //Jimmychiu 20230417 : Record the position after placing the IC  //Steven 20240508 : fixed for iWhichSite flag

    if((CUSTOMER_CODE==CC_AMKOR_China ||
        CUSTOMER_CODE==CC_QUALCOMM) &&                                          //JerryYang 20170412 (Steven) add QUALCOMM
       LastSet.iRunStartMode==rsmQAMode &&                                      //jou 2014-12-08 QA 的產品測試完成以後，剩下的產品在HOTPLATE 裡面不需要等待加熱
       LastSet.iTester==OFF_LINE &&
       bQAModeFinishCleanOut==true)
        HotTime[iP][iPlateC][iPlateR]       =Prod.iHotTime;
    else
        HotTime[iP][iPlateC][iPlateR]       =0;

    if(InArmSuck.Item[iSuckRow][iSuckCol]!=HAS_NULL_IC)                         //Steven 20170109 : 沒IC的地方不檢查吹氣
        bInArmCheckDestroyACT[iSuckRow][iSuckCol]=true;                         //jou 981130 確認device確實destroy完成

    if(USE_LASER_DISTANCE && TestIF_File.bEnableInArmLaser)                     //Steven 20140228 : 雷射測距功能
    {
        if(InArmSuck.Item[iSuckRow][iSuckCol]==HAS_IC)
        {
            LaserCheckPos.push_back(new TInLaserCheck(iP, iPlateC, iPlateR));
        }
    }
    iRowOnHotPlate[iP][iPlateC][iPlateR]=(iSuckRow+1)*10+iSuckCol+1;            //JerryYang 20180718 (wei) : 放料至hot plate記錄吸嘴位置
                                                                                //                   Aa Ab Ac Ad    11 12 13 14
                                                                                //                   Ba Bb Bc Bd    21 22 23 24
    InArmSuck.PordRec[iSuckRow][iSuckCol].AddHPRecord(iP, iPlateR, iPlateC);    //Frank 20160505 add

    if((MOT[MMPlate1+iP].Tray.Data[iPlateC][iPlateR]==HAS_HOT_IC ||             //Sam 20211001 : 修正 2X8 殘料問題
        MOT[MMPlate1+iP].Tray.Data[iPlateC][iPlateR]==HAS_IC) &&
       InArmSuck.Item[iSuckRow][iSuckCol]==HAS_IC)
    {
        fMain->DebugOneCycleHotPlate("DoPlaceToHPSwapData1");
        ShowMyMessage("HotPlate Data Swap error 1. Please call engineer", "加熱盤資料轉移錯誤1，請通知工程師處理");
    }
    else if((MOT[MMPlate1+iP].Tray.Data[iPlateC][iPlateR]==HAS_HOT_IC ||
             MOT[MMPlate1+iP].Tray.Data[iPlateC][iPlateR]==HAS_IC) &&
            InArmSuck.Item[iSuckRow][iSuckCol]==HAS_NULL_IC)
    {
        fMain->DebugOneCycleHotPlate("DoPlaceToHPSwapData2");
        ShowMyMessage("HotPlate Data Swap error 2. Please call engineer", "加熱盤資料轉移錯誤2，請通知工程師處理");
        return;                                                                 //AI(general) 20260408 (RogerYang) : prevent HAS_NULL_IC overwrite existing IC on HP
    }
    else if(TestIF_File.UseRotateForHT7000HPKit==false &&                       //Sam 20250428 : 修正 UseRotateForHT7000HPKit 誤報警
            USE_ROTATE_KIT==1 &&
            tRotate.ActiveRotate &&                                             //Sam 20221103 : 新增In旋轉角度
            InArmSuck.PordRec[iSuckRow][iSuckCol].GetInRotationAngRecord()=="" &&
            InArmSuck.Item[iSuckRow][iSuckCol]!=HAS_NULL_IC &&
            iRotate_Type!=eInOutArm1Motor)
    {
        ShowMyMessage("Rotate Data Swap error . Please call engineer", "旋轉資料轉移錯誤，請通知工程師處理");
    }

    InspectInArmPosition(MMPlate1+iP, iSuckRow, iSuckCol, iPlateR, iPlateC, true);

    InArmSuck.CopyToTray(iSuckRow, iSuckCol, NULL_IC, MOT[MMPlate1+iP], iPlateR, iPlateC, InArmSuck.Item[iSuckRow][iSuckCol]);
    if(CosFunction.bShowHPICCount)                                              //Steven 20221228 : 計算加熱盤IC數量
    {
        fSortCT->pnlHP1->Caption=MOT[MMPlate1].Tray.HowManyIC();
        fSortCT->pnlHP2->Caption=MOT[MMPlate2].Tray.HowManyIC();
    }
    bAutoSiteMapHasPickHP=false;                                                //Steven 20230210 : Fixed for auto site map
}
//------------------------------------------------------------------------------
//Ifor 20170928 (Steven) : 將Auto Site Mapping 資料寫回到HP
//------------------------------------------------------------------------------
void DoPlaceToHPBackupData(int iSuckRow, int iSuckCol, int iP, int iPlateR, int iPlateC)
{
    AnsiString Str;
    bool bHasNullIC=false;
    if(InArmSuck.Item[iSuckRow][iSuckCol]==HAS_NULL_IC)                         //Ifor 20170928 (Steven) : add Has Null IC 無備份資料需另外處理
        bHasNullIC=true;
    InArmSuck.CopyToTray(iSuckRow, iSuckCol, NULL_IC, MOT[MMPlate1+iP], iPlateR, iPlateC, InArmSuck.Item[iSuckRow][iSuckCol]);
    if(bHasNullIC==true)                                                        //Ifor 20170928 (Steven) : add Has Null IC 無備份資料需另外處理
    {
        if(iSuckRow==0 && iSuckCol==0)                                          //Ifor 20180509 : add Auto Site Mapping HotTime 需清除才會計數
        {
            HotTime[iP][iAutoSiteMapHotplatePlateC][iAutoSiteMapHotplatePlateR]=0;
        }
        return;
    }
    iHotWhichKit[iP][iAutoSiteMapHotplatePlateC][iAutoSiteMapHotplatePlateR]      =InArmSuck.iWhichKitBackup;
    iHotWhichShuttle[iP][iAutoSiteMapHotplatePlateC][iAutoSiteMapHotplatePlateR]  =InArmSuck.iWhichShuttleBackup;
    iHotPlateCount[iP][iAutoSiteMapHotplatePlateC][iAutoSiteMapHotplatePlateR]    =InArmSuck.HotCount;

    if((CUSTOMER_CODE==CC_AMKOR_China || CUSTOMER_CODE==CC_QUALCOMM) && LastSet.iRunStartMode==rsmQAMode && LastSet.iTester==OFF_LINE && bQAModeFinishCleanOut==true)  //jou 2014-12-08 QA 的產品測試完成以後，剩下的產品在HOTPLATE 裡面不需要等待加熱  //JerryYang 20170412 (Steven) add QUALCOMM
        HotTime[iP][iAutoSiteMapHotplatePlateC][iAutoSiteMapHotplatePlateR]       =Prod.iHotTime;
    else
        HotTime[iP][iAutoSiteMapHotplatePlateC][iAutoSiteMapHotplatePlateR]       =0;

    if(InArmSuck.Item[iSuckRow][iSuckCol]!=HAS_NULL_IC)                         //Steven 20170109 : 沒IC的地方不檢查吹氣
        bInArmCheckDestroyACT[iSuckRow][iSuckCol]=true;                         //jou 981130 確認device確實destroy完成

    if(USE_LASER_DISTANCE && TestIF_File.bEnableInArmLaser)                     //Steven 20140228 : 雷射測距功能
    {
        if(InArmSuck.Item[iSuckRow][iSuckCol]==HAS_IC)
        {
            LaserCheckPos.push_back(new TInLaserCheck(iP, iAutoSiteMapHotplatePlateC, iAutoSiteMapHotplatePlateR));
        }
    }

    InspectInArmPosition(MMPlate1+iP, iSuckRow, iSuckCol, iPlateR, iPlateC, true);

    InArmSuck.PordRec[iSuckRow][iSuckCol].AddHPRecord(iP, iAutoSiteMapHotplatePlateR, iAutoSiteMapHotplatePlateC);      //Sam 20200716 : Add record //Frank 20160505 add
    Str.sprintf("%d, %d, %d, %s, Place", iP, iAutoSiteMapHotplatePlateR, iAutoSiteMapHotplatePlateC, InArmSuck.Suck[iSuckRow][iSuckCol].sName);
    fMain->slAutoSiteMapLog->AddTextWithDateTime(Str);                          //Steven 20211209 : 紀錄Auto Site Map動作
    InArmSiteMapData.ClearData();                                               //Steven 20211209 : 紀錄Site map資料
    if(iResetSiteMappingStep>0)
    {
        if(iResetSiteMappingStep==2)
        {
            bSiteMappingCHKOK=true;

            if(fLotInfo->cbRunMode->Visible==true &&                            //RogerYang 20250312 補code(from 偉測版本)
               fLotInfo->cbRunMode->Text.Pos("RT")>0)
            {
                SetRunStartMode(rsmContinuRetest);
            }
            else
            {
                SetRunStartMode(rsmContinuStart);
            }

            for(int i=0; i<2; i++)
            {
                for(int j=0; j<HotPlateForm.XDivision; j++)
                {
                    for(int k=0; k<HotPlateForm.YDivision; k++)
                    {
                        MOT[MMPlate1+i].Tray.SiteMapData[j][k]=0;               //Ifor 20210913 add: 清除Auto Site Mapping 補回Has Null IC資料
                    }
                }
            }
            bAutoSiteMapHotplateSave=false;
            InitInArmTask();
            fMain->ShowTestHeadComp(false);
        }
/*        iDoSiteMappingStep=0;
        iAutoSiteMapCount=-1;                                                   //Steven 20220811 : 紀錄目前是哪個Shuttle
        iAutoSiteCurrStep=-1;
        bAutoSiteMapWaitTestPass=false;
        AdjustShuttlePlaceOrderForASM(true);                                    //Steven 20250206 : fixed ASM之中作one cycle會造成關arm 1時異常
        RecordProcess("Reset step of auto site map", "DoPlaceToHotPlate");   */
        iResetSiteMappingStep=0;
        fMain->ReStartAutoSiteMapping(true);
    }

    if(LastSet.iRunStartMode==rsmAutoSiteMap &&
       bAutoSiteMapHasPickHP==true &&
       CosFunction.bUSEJCETSiteMapMode==true)                                   //Steven 20220527 : for JCET Auto Site Map
    {
        PickFromHPList->SetPlateSuck(iP, iPlateR, iPlateC, true);               //Jimmychiu 20230417 : Record the position after placing the IC
    }

    if(CosFunction.bShowHPICCount)                                              //Steven 20221228 : 計算加熱盤IC數量
    {
        fSortCT->pnlHP1->Caption=MOT[MMPlate1].Tray.HowManyIC();
        fSortCT->pnlHP2->Caption=MOT[MMPlate2].Tray.HowManyIC();
    }
}
//------------------------------------------------------------------------------
#if 0 // AI(W6.2-INARM) 20260626 TODO(W7: per-site DoPlaceToHotPlate variants):
      // pure iInArmType dispatch to per-site bodies in ainarm9045_*.cpp (W7).
      // The 24 extern decls + the dispatcher body are gated; an active
      // return-false stub is provided below.  .h decl stays live.
extern bool DoPlaceToHotPlate_9045_1x1_1();
extern bool DoPlaceToHotPlate_9045_1x2_2();
extern bool DoPlaceToHotPlate_9045_1x2_2_14();
extern bool DoPlaceToHotPlate_9045_1x2_4_Hot();
extern bool DoPlaceToHotPlate_9045_1x3_2_14();
extern bool DoPlaceToHotPlate_9045_1x3_4();
extern bool DoPlaceToHotPlate_9045_1x4_2_14();
extern bool DoPlaceToHotPlate_9045_1x4_4_13();
extern bool DoPlaceToHotPlate_9045_1x4_4();
extern bool DoPlaceToHotPlate_9045_1x4_4_Back();
extern bool DoPlaceToHotPlate_9045_1x4_8_Hot();
extern bool DoPlaceToHotPlate_9045_2x1_2();
extern bool DoPlaceToHotPlate_9045_2x2_4_12();
extern bool DoPlaceToHotPlate_9045_2x2_4_13();
extern bool DoPlaceToHotPlate_9045_2x2_4_14();
extern bool DoPlaceToHotPlate_9045_2x2_8_Hot();
extern bool DoPlaceToHotPlate_9045_2x3_6_14();
extern bool DoPlaceToHotPlate_9045_2x3_6();
extern bool DoPlaceToHotPlate_9045_2x4_4_13();
extern bool DoPlaceToHotPlate_9045_2x4_4_14();
extern bool DoPlaceToHotPlate_9045_2x4_8();
extern bool DoPlaceToHotPlate_9045_2x5_8();
extern bool DoPlaceToHotPlate_9045_2x6_8();
extern bool DoPlaceToHotPlate_9045_2x8_8();
//------------------------------------------------------------------------------
bool DoPlaceToHotPlate_9045()
{
    AnsiString Str;
    bool bRet=false;
    if(USE_PICKER_COUNT==ep1Picker ||
       iInArmType==e9045_1x1_1 ||
       iInArmType==e9045_1x4_1_Ac)
    {
        bRet=DoPlaceToHotPlate_9045_1x1_1();
    }
    else if(iInArmType==e9045_1x2_2_13)
    {
        bRet=DoPlaceToHotPlate_9045_1x2_2();
    }
    else if(iInArmType==e9045_1x2_2_14)
    {
        bRet=DoPlaceToHotPlate_9045_1x2_2_14();
    }
    else if(iInArmType==e9045_1x2_4_Hot)
    {
        bRet=DoPlaceToHotPlate_9045_1x2_4_Hot();
    }
    else if(iInArmType==e9045_1x3_2_14)
    {
        bRet=DoPlaceToHotPlate_9045_1x3_2_14();
    }
    else if(iInArmType==e9045_1x3_4)
    {
        bRet=DoPlaceToHotPlate_9045_1x3_4();
    }
    else if(iInArmType==e9045_1x4_4_13)
    {
        bRet=DoPlaceToHotPlate_9045_1x4_4_13();
    }
    else if(iInArmType==e9045_1x4_2_14)
    {
        bRet=DoPlaceToHotPlate_9045_1x4_2_14();
    }
    else if(iInArmType==e9045_1x4_4_Back)
    {
        bRet=DoPlaceToHotPlate_9045_1x4_4_Back();
    }
    else if(iInArmType==e9045_1x4_4)
    {
        bRet=DoPlaceToHotPlate_9045_1x4_4();
    }
    else if(iInArmType==e9045_1x4_8_Hot)
    {
        bRet=DoPlaceToHotPlate_9045_1x4_8_Hot();
    }
    else if(iInArmType==e9045_2x1_2_13)
    {
        bRet=DoPlaceToHotPlate_9045_2x1_2();
    }
    else if(iInArmType==e9045_2x2_4_12)
    {
        bRet=DoPlaceToHotPlate_9045_2x2_4_12();
    }
    else if(iInArmType==e9045_2x2_4_13)
    {
        bRet=DoPlaceToHotPlate_9045_2x2_4_13();
    }
    else if(iInArmType==e9045_2x2_4_14)
    {
        bRet=DoPlaceToHotPlate_9045_2x2_4_14();
    }
    else if(iInArmType==e9045_2x2_8_Hot)
    {
        bRet=DoPlaceToHotPlate_9045_2x2_8_Hot();
    }
    else if(iInArmType==e9045_2x3_6_14)
    {
        bRet=DoPlaceToHotPlate_9045_2x3_6_14();
    }
    else if(iInArmType==e9045_2x3_6)
    {
        bRet=DoPlaceToHotPlate_9045_2x3_6();
    }
    else if(iInArmType==e9045_2x4_4_13)
    {
        bRet=DoPlaceToHotPlate_9045_2x4_4_13();
    }
    else if(iInArmType==e9045_2x4_4_14)
    {
        bRet=DoPlaceToHotPlate_9045_2x4_4_14();
    }
    else if(iInArmType==e9045_2x4_8)
    {
        bRet=DoPlaceToHotPlate_9045_2x4_8();
    }
    else if(iInArmType==e9045_2x5_8)
    {
        bRet=DoPlaceToHotPlate_9045_2x5_8();
    }
    else if(iInArmType==e9045_2x6_8)
    {
        bRet=DoPlaceToHotPlate_9045_2x6_8();
    }
    else if(iInArmType==e9045_2x8_8 ||
            iInArmType==e9045_2x8_32)
    {
        bRet=DoPlaceToHotPlate_9045_2x8_8();
    }
    else
    {
        Str.sprintf("iInArmType=%d", iInArmType);                               //Steven 20220620 : add log message
        ShowMyMessage("Program Error in DoInArm_9045()", Str);
    }
    return bRet;
}
#endif // TODO(W7): per-site DoPlaceToHotPlate dispatch
bool DoPlaceToHotPlate_9045()
{
    return false;   // AI(W6.2-INARM) 20260626: per-site dispatch is W7
}
//------------------------------------------------------------------------------
