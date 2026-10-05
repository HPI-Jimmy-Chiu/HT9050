// =============================================================================
//  ainarm_SearchPickPlate.cpp  --  shared InArm HotPlate-PICK geometry + SM
//
//  Translated from: HT9011UC_Code_V3.33.906.0_20260618/ainarm_SearchPickPlate.cpp (1399 ln)
//  Translation wave: W6.2 (IN-ARM foundation -- shared HP-pick geometry leaf)
//  Translator: AI(W6.2-INARM) 20260626
//
//  FAITHFUL TRANSLATION
//  --------------------
//  Shared HP-pick sub-task machine the ~30 in-arm site-variants call.  All names,
//  1-based numeric Task states, switch(Task) semantics and the intentional case
//  fall-throughs (1->50, 190->200, 350->400; no break) are preserved VERBATIM.
//  The HP-pick geometry math (MoveInArmXYPickHotPlate_9045 basis-shift: float
//  divisors /7.0 and /3.0 -- NOT int-truncated) is kept EXACTLY.
//
//  TASK CURSOR (W6 strategy: each SM owns its extern cursor): the only SM here is
//  DoInArmPickFromHotPlate_9045(); its cursor is iInArmPickFromHotPlateTask
//  (golden ainarm2.h:114).  This .cpp OWNS its definition below.
//
//  GATED / SWAPPED (see include block): heavy UI/SECS/cMyDB/OCR/rotate headers
//  swapped for the W6.2 substrate + FormsFacade + canary_support; aArmHeader.h
//  stays the W6.0 god-header SHIM.  iInArmType dispatch stays in the (later)
//  ainarm core -- these are the shared geometry leaves.
//
//  SOFT_SIMULTE is NOT defined: the #ifdef SOFT_SIMULTE blocks (fMain->cb1,
//  PickFromHPList->SaveFile(sHPPickRec)) compile out NATURALLY and are kept
//  verbatim (inert).  The #ifndef SOFT_SIMULTE CheckSafeDoorIsClosed lines are
//  ACTIVE (compiled in).
//
//  In-arm engine helpers not yet translated (DoInArm_9045_SuckerMap,
//  MoveInArmZToPlateSafe, IsCheckInArmDestroyActiveFinish,
//  SetShuttleToHasNullIC_9045, InArmSub/AddSpeed, AdjustShuttlePlaceOrder,
//  SetInArmHome, AddInArmPickerCount, InspectInArmPosition, TransferHotPlateRatio,
//  GetInArmPitchX/Y_9045) resolve via extern decls in aHotPlateSubstrate.h
//  (offline sim bodies in aHotPlateSubstrate.cpp); the Get*/Search*/Has*/Move*
//  geometry leaves in this file are the live linkable surface.
//
//  Big5: every Chinese comment decoded cleanly via cp950 and is preserved as
//  UTF-8.  NO U+FFFD is emitted.
// =============================================================================
// BCB6 ORIGINAL include block (mirrored as a comment for provenance):
//   #include "MachineDefine.h"   ; #pragma hdrstop
//   #include "ainarm_SearchPickPlate.h"
//   #include "aArmHeader.h"  (god-header -- kept as the W6.0 SHIM, #if 0)
//   #include "note.h"        (ShowErrorMessage)            -> canary_support.h
//   #include "csystem.h"     (predicate API + CheckSafeDoorIsClosed) -> kept
//   #include "cinitial.h"    (InitialOK etc. via cmydef)   -> cmydef.h
//   #include "mymessbox.h"   (ShowMyMessage VCL)           -> canary_support.h
//   #include "mymotor.h"     (MOT[]/InArmContinuousMove_9045) -> Motor/mymotor.h
//   #include "cmydef.h" / "atester.h" / "common.h" / "mycylin.h" / "cprod.h"
//   #include "MyKitSuck.h"   (InArmSuck/FLCarryKit/...)     -> aHotPlateSubstrate.h
//   #include "OCR.h" / "cMyDB.h" (MyDBIProcess)            -> aHotPlateSubstrate.h
//   #include "aRotateKIT.h" / "fRotate.h" (tRotate)        -> aHotPlateSubstrate.h
//   #include "cpublic.h" / "mysensor.h"
//   #include "main.h" (fMain) / "cSortCT.h" (fSortCT) / "uLotInfo.h" (fLotInfo)
//   #include "SCK_ART.h" (fSCKART)                          -> FormsFacade.h
//   ainarm2.h cursors (iPickPlate/iInArmPickFromHotPlateTask/iHotWhich*...) -> aHotPlateSubstrate.h
//   #pragma package(smart_init)
#include "ainarm_SearchPickPlate.h"

#include "aArmHeader.h"             // W6.0 SHIM (god-header gated #if 0; do NOT un-gate)
#include "ainarm_SearchPlacePlate.h" // shared HP-place geometry externs (bZFlgToHP*/iZPosToHP/GetHotPlateYHalfPos/GetInArmZToHPPos/...) -- golden pulled these via aArmHeader god-header
#include "csystem.h"                // predicate API (InSHT1InLF/ShuttleHasIC/IndexHasIC/CheckSafeDoorIsClosed/DoAutoSiteMappingDropError)
#include "Motor/mymotor.h"          // MOT[], InArmContinuousMove_9045 (W4 sim stub)
#include "cmydef.h"                 // global type universe + IC consts + most scalars
#include "common.h"
#include "mycylin.h"
#include "cprod.h"                  // Prod / TestIF / TestIF_File / HotPlateForm / ArmSpeed
#include "cpublic.h"                // CosFunction
#include "mysensor.h"               // Sen[]
#include "aHotPlateSubstrate.h"     // InArmSuck/FLCarryKit/.. + PickFromHPList + ainarm2 cursors + engine externs + tRotate/Laser
#include "FormsFacade.h"            // fMain/fSortCT/fLotInfo/fOffSet/fSCKART (non-VCL stubs)
#include "canary_support.h"         // ShowErrorMessage / ShowMyMessage / RecordProcess / LastSet / __FUNC__

// ainarm2.h forward decls the golden pulled via aArmHeader (functions defined
// later in THIS file / the per-site core).  Declare the ones PickPlate uses
// before their definition point.  AI(W6.2-INARM) 20260626.
extern bool SearchPlateToPick();                    // defined below (golden ainarm2.h:108)
bool SearchPickPlateXItem_AutoSiteMap();            // defined below in this TU

//------------------------------------------------------------------------------
//  W6 task-cursor (this SM owns its extern; golden decl ainarm2.h:114)
//  AI(W6.2-INARM) 20260626: own the definition here, like the W6.1 canary owns
//  iLoadNewEmptyTrayToCarTask.
//------------------------------------------------------------------------------
int iInArmPickFromHotPlateTask = 1;

//==============================================================================
int GetHeaterWaitTime()
{
    int &ix=iPickPlateX[0], &iy=iPickPlateY[0];
    int ret=0;
    if(MOT[MMPlate1+iPickPlate[0]].Tray.Data[ix][iy]!=NULL_IC)
    {
        ret=Prod.iHotTime-HotTime[iPickPlate[0]][ix][iy];
    }
    else
    {
        ret=0;
    }

    if(ret<0)
        return 0;
    return ret;
}
//==============================================================================
bool ArmRow1AllHasIC()
{
//    int iStepX=0, iInArmXPos=0;
//    iStepX=GetHotPlateColStep(iPickHP);
//
//    for(int i=0; i<iStepX; i++)
//    {
//        if(HotPlateForm.XDivision==6  &&
//           HotPlateForm.YDivision==11 &&
//           InArmSuck.iPickRow==2 &&
//           iYHalf==2 &&
//           iPickPlateX[0]==1 && iPickPlateY[0]==9 &&
//           i8PickerHPMode==iHPNoUse8suck)
//        {
//            iPickHPOrder=1;
//            return false;
//        }
//        else
//        {
//            if(InArmSuck.Item[0][iInArmXPos]==NULL_IC)
//            {
//                iPickHPOrder=0;
//                OnlyRow2=false;
//                return false;
//            }
//        }
//    }
//    iPickHPOrder=1;
    return true;
}
//==============================================================================
//V3.27N.548 Ifor 20171006 (Steven) : add 備份Hotplate取料位置
//==============================================================================
void BackupHotplatelocation()
{
//    if(IniConfig.bI21AutoSiteMappingUseHotplate==true   &&
//       bSiteMappingCHKOK==false                         &&
//       bAutoSiteMapHotplateSave==false                  )                     //Ifor 20171006 : 備份Hotplate取料位置
//    {
//        iAutoSiteMapHotplatePickPlate   =iPickPlate[0];
//        iAutoSiteMapHotplatePickPlateX  =iPickPlateX[0];
//        iAutoSiteMapHotplatePickPlateY  =iPickPlateY[0];
//    }
}
//==============================================================================
void TryHotClearRecBufMMPlate()                                                 //ChungHung 20120206 Hotplate check
{
    for(int k=0; k<2; k++)
    {
        for(int i=0; i<HotPlateForm.XDivision; i++)
        {
            for(int j=0; j<HotPlateForm.YDivision; j++)
            {
                if(MOT[MMPlate1+k].Tray.Data[i][j]==HAS_TRY_SUCK_IC)
                {
                    MOT[MMPlate1+k].SetTraySingleData(i, j, NULL_IC);
                }
            }
        }
    }
}
//------------------------------------------------------------------------------
void PorcessJAM0109HotPlatePickUpErrorSkip(int iSuckR, int iSuckC, int iP, int iPlateR, int iPlateC)                    //Steven 20161214 : 將HP吸取異常SKIP統一成Function   //Steven 20240130 : iC / iR 對調
{
    int iXPos=MOT[MInArmX].ReadPos();
    int iYPos=MOT[MInArmY].ReadPos();
    if(CosFunction.bUseSCKART)                                                  //Steven 20161214 (wei) : For SCK ART
        fSCKART->iInputJamCnt++;

    InArmSuck.PordRec[iSuckR][iSuckC].AddErrorRecord("JAM0109", true, iXPos, iYPos);                                    //Steven 20161214 : Add Jam Record
    bZFlgToHPPick[iSuckR][iSuckC]       =false;
    bPickHPDuplicateErr[iSuckR][iSuckC] =false;
    InArmSuck.HotCount                  =iHotPlateCount[iP][iPlateC][iPlateR];
    InArmSuck.iWhichKitBackup           =iHotWhichKit[iP][iPlateC][iPlateR];    //wei 20171211 (jou) InArmSuck.iWhichKit-->InArmSuck.iWhichKitBackup
    InArmSuck.iWhichShuttleBackup       =iHotWhichShuttle[iP][iPlateC][iPlateR];                                        //Ifor 20170927 (Steven) : add 備份Hotplate WhichShuttle
    iAutoSiteMapHotplateSource          =iP;                                    //Ifor 20170928 (Steven) : 避免Auto Site Mapping 補錯Hotplate資料
    InArmSuck.CopyFromTray(iSuckR, iSuckC, HAS_NULL_IC, MOT[MMPlate1+iP], iPlateR, iPlateC, NULL_IC, -1, true);         //Steven 20250226 : HP Pick Err SKIP後也要交換資料
    InspectInArmPosition(MMPlate1+iP, iSuckR, iSuckC, iPlateR, iPlateC, false);
    InArmSuck.PordRec[iSuckR][iSuckC].AddInArmHotplatePickRecord(iSuckR, iSuckC);                                       //Sam 20200716 : Add record

//    InArmSuck.Suck[iSuckR][iSuckC].Error=false;
//    InArmSuck.SetItemData(iSuckR, iSuckC, HAS_NULL_IC);                       //Steven 20150921 : For HP Skip Hang Up
//    MOT[MMPlate1+iP].SetTraySingleData(iPlateC, iPlateR, NULL_IC);            //Steven 20150921 : HAS_NULL_IC --> NULL_IC
    InArmSuckUse[iSuckR][iSuckC]=false;                                         //Steven 20160606 : Fixed for 2x2 hang up

    if(bRunAutoSiteMapping==true)                                               //Ifor 20210524 Fix: Auto Sitemapping Pick up Error 資料異常
    {
        MOT[MMPlate1+iP].SetTraySingleData(iPlateC, iPlateR, HAS_NULL_IC);
        MOT[MMPlate1+iP].Tray.SiteMapData[iPlateC][iPlateR]=1;
        bAutoSiteMapHotplateSave=true;
        DoAutoSiteMappingDropError();
    }
    else
    {
        iHotWhichShuttle[iP][iPlateC][iPlateR]  =-1;
        iHotWhichKit[iP][iPlateC][iPlateR]      =-1;
        iHotPlateCount[iP][iPlateC][iPlateR]    =-1;
    }

    if(CosFunction.bShowHPICCount)                                              //Steven 20221228 : 計算加熱盤IC數量
    {
        fSortCT->pnlHP1->Caption=MOT[MMPlate1].Tray.HowManyIC();
        fSortCT->pnlHP2->Caption=MOT[MMPlate2].Tray.HowManyIC();
    }
}
//==============================================================================
//V3.27Z.568 Ifor 20180518 : add 整合Hot plate 資料轉換
//==============================================================================
void HotplateDataConversion(int iSuckR, int iSuckC, int iP, int iPlateR, int iPlateC, bool *flag1)
{
    AnsiString Str;
    if(InArmSuck.Item[iSuckR][iSuckC]!=NULL_IC)
        return;

    if(iP<0 || iPlateC<0 || iPlateR<0)                                          //Steven 20250227 : add protection
        return;

    if(InArmSuck.Item[iSuckR][iSuckC]==NULL_IC &&
       MOT[MMPlate1+iP].Tray.Data[iPlateC][iPlateR]==HAS_NULL_IC)               //Steven 20250401 : add protection
    {
        bZFlgToHPPick[iSuckR][iSuckC]       =false;
        bPickHPDuplicateErr[iSuckR][iSuckC] =false;
        InArmSuck.HotCount                  =iHotPlateCount[iP][iPlateC][iPlateR];
        InArmSuck.iWhichKitBackup           =iHotWhichKit[iP][iPlateC][iPlateR];                                        //wei 20171211 (jou) InArmSuck.iWhichKit-->InArmSuck.iWhichKitBackup
        InArmSuck.iWhichShuttleBackup       =iHotWhichShuttle[iP][iPlateC][iPlateR];                                    //Ifor 20170927 (Steven) : add 備份Hotplate WhichShuttle
        iAutoSiteMapHotplateSource          =iP;                                //Ifor 20170928 (Steven) : 避免Auto Site Mapping 補錯Hotplate資料
        InArmSuck.CopyFromTray(iSuckR, iSuckC, MOT[MMPlate1+iP].Tray.Data[iPlateC][iPlateR], MOT[MMPlate1+iP], iPlateR, iPlateC, NULL_IC, -1, true);            //Frank 20160505 add

        InspectInArmPosition(MMPlate1+iP, iSuckR, iSuckC, iPlateR, iPlateC, false);

        InArmSuck.PordRec[iSuckR][iSuckC].AddInArmHotplatePickRecord(iSuckR, iSuckC);                                   //Sam 20200716 : Add record
        if(bRunAutoSiteMapping==false)                                          //Ifor 20210426 add:執行Auto Site Mapping 保留原本資料避免無IC補回Hot plate發生Hang up
        {
            iHotWhichShuttle[iP][iPlateC][iPlateR]  =-1;
            iHotWhichKit[iP][iPlateC][iPlateR]      =-1;
            iHotPlateCount[iP][iPlateC][iPlateR]    =-1;
        }
        else                                                                    //Steven 20211209 : 紀錄Auto Site Map動作
        {
            Str.sprintf("%d, %d, %d, %s, Pick", iP, iPlateR, iPlateC, InArmSuck.Suck[iSuckR][iSuckC].sName);
            fMain->slAutoSiteMapLog->AddTextWithDateTime(Str);
            InArmSiteMapData.UpdateData(iP, iPlateR, iPlateC, iSuckR, iSuckC);  //Steven 20211209 : 紀錄Site map資料
        }
    }
    else if(iInArmType==e9045_2x4_8 ||
       iInArmType==e9045_2x8_8 ||
       iInArmType==e9045_2x6_8 ||
       iInArmType==e9045_2x8_32)
    {
        if(InArmSuck.Item[iSuckR][iSuckC]==NULL_IC &&
            bZFlgToHPPick[iSuckR][iSuckC])                                      //Steven 20160719 : 修改判斷方式
        {
            if(bRunAutoSiteMapping==true && MOT[MMPlate1+iP].Tray.Data[iPlateC][iPlateR]==HAS_NULL_IC)
            {
            }
            else
            {
                if(ArmSpeed_File[InArm].bUseHPVacuum)                           //Steven 20180125 (Jou) : 加熱盤的真空等待時間
                    InArmSuck.Suck[iSuckR][iSuckC].OnDelayTime=ArmSpeed[InArm].dHPVacuumTI*100;

                #ifdef SOFT_SIMULTE
                if(fMain->cb1->Checked==true)
                {
                    if(MOT[MMPlate1+iP].Tray.Data[iPlateC][iPlateR]!=NULL_IC)
                    {
                        InArmSuck.Suck[iSuckR][iSuckC].Error=true;
                    }
                }
                #endif

                if(InArmSuck.Suck[iSuckR][iSuckC].Error==true)                  //Sam 20220125 : fixed for in arm pick HP hang up
                {
                }
                else if(InArmSuck.Suck[iSuckR][iSuckC].Suck())
                {
                    AddInArmPickerCount(iSuckR, iSuckC);
                    bZFlgToHPPick[iSuckR][iSuckC]       =false;
                    bPickHPDuplicateErr[iSuckR][iSuckC] =false;
                    InArmSuck.HotCount                  =iHotPlateCount[iP][iPlateC][iPlateR];
                    InArmSuck.iWhichKitBackup           =iHotWhichKit[iP][iPlateC][iPlateR];                            //wei 20171211 (jou) InArmSuck.iWhichKit-->InArmSuck.iWhichKitBackup
                    InArmSuck.iWhichShuttleBackup       =iHotWhichShuttle[iP][iPlateC][iPlateR];                        //Ifor 20170927 (Steven) : add 備份Hotplate WhichShuttle
                    iAutoSiteMapHotplateSource          =iP;                    //Ifor 20170928 (Steven) : 避免Auto Site Mapping 補錯Hotplate資料
                    InArmSuck.CopyFromTray(iSuckR, iSuckC, MOT[MMPlate1+iP].Tray.Data[iPlateC][iPlateR], MOT[MMPlate1+iP], iPlateR, iPlateC, NULL_IC, -1, true);  //Frank 20160505 add

                    InspectInArmPosition(MMPlate1+iP, iSuckR, iSuckC, iPlateR, iPlateC, false);

                    InArmSuck.PordRec[iSuckR][iSuckC].AddInArmHotplatePickRecord(iSuckR, iSuckC);                       //Sam 20200716 : Add record
                    if(bRunAutoSiteMapping==false)                              //Ifor 20210426 add:執行Auto Site Mapping 保留原本資料避免無IC補回Hot plate發生Hang up
                    {
                        iHotWhichShuttle[iP][iPlateC][iPlateR]  =-1;
                        iHotWhichKit[iP][iPlateC][iPlateR]      =-1;
                        iHotPlateCount[iP][iPlateC][iPlateR]    =-1;
                    }
                    else                                                        //Steven 20211209 : 紀錄Auto Site Map動作
                    {
                        Str.sprintf("%d, %d, %d, %s, Pick", iP, iPlateR, iPlateC, InArmSuck.Suck[iSuckR][iSuckC].sName);
                        fMain->slAutoSiteMapLog->AddTextWithDateTime(Str);
                        InArmSiteMapData.UpdateData(iP, iPlateR, iPlateC, iSuckR, iSuckC);                              //Steven 20211209 : 紀錄Site map資料
                    }
                }
                else if(InArmSuck.Suck[iSuckR][iSuckC].Error==false)
                {
                    *flag1=false;
                }
            }
        }
        else
        {
            if(bZFlgToHPPick[iSuckR][iSuckC]==false &&
               InArmSuck.Item[iSuckR][iSuckC]==NULL_IC)                         //Ifor 20190507 : add 未使用的吸嘴設定HasNull IC
                InArmSuck.SetItemData(iSuckR, iSuckC, HAS_NULL_IC);
        }
    }
    else
    {
        if(bZFlgToHPPick[iSuckR][iSuckC] &&
           InArmSuck.Item[iSuckR][iSuckC]==NULL_IC)                             //Ifor 20190507 : add 避免重複吸取IC
        {
            if(ArmSpeed_File[InArm].bUseHPVacuum)                               //Steven 20180125 (Jou) : 加熱盤的真空等待時間
                InArmSuck.Suck[iSuckR][iSuckC].OnDelayTime=ArmSpeed[InArm].dHPVacuumTI*100;

            if(bRunAutoSiteMapping==true && MOT[MMPlate1+iP].Tray.Data[iPlateC][iPlateR]==HAS_NULL_IC)
            {
            }
            else
            {
                #ifdef SOFT_SIMULTE
                if(fMain->cb1->Checked==true)
                {
                    if(MOT[MMPlate1+iP].Tray.Data[iPlateC][iPlateR]!=HAS_NULL_IC ||
                       MOT[MMPlate1+iP].Tray.Data[iPlateC][iPlateR]!=NULL_IC)
                    {
                        InArmSuck.Suck[iSuckR][iSuckC].Error=true;
                    }
                }
                #endif

                if(InArmSuck.Suck[iSuckR][iSuckC].Error==true)                  //Sam 20220125 : fixed for in arm pick HP hang up
                {
                }
                else if(InArmSuck.Suck[iSuckR][iSuckC].Suck())
                {
                    if(MOT[MMPlate1+iP].Tray.Data[iPlateC][iPlateR]!=HAS_NULL_IC)                                       //JerryYang 20220331 : add 吸真空次數計數
                    {
                        AddInArmPickerCount(iSuckR, iSuckC);
                    }
                    bZFlgToHPPick[iSuckR][iSuckC]       =false;
                    bPickHPDuplicateErr[iSuckR][iSuckC] =false;
                    InArmSuck.HotCount                  =iHotPlateCount[iP][iPlateC][iPlateR];
                    InArmSuck.iWhichKitBackup           =iHotWhichKit[iP][iPlateC][iPlateR];                            //wei 20171211 (jou) InArmSuck.iWhichKit-->InArmSuck.iWhichKitBackup
                    InArmSuck.iWhichShuttleBackup       =iHotWhichShuttle[iP][iPlateC][iPlateR];                        //Ifor 20170927 (Steven) : add 備份Hotplate WhichShuttle
                    iAutoSiteMapHotplateSource          =iP;                    //Ifor 20170928 (Steven) : 避免Auto Site Mapping 補錯Hotplate資料
                    InArmSuck.CopyFromTray(iSuckR, iSuckC, MOT[MMPlate1+iP].Tray.Data[iPlateC][iPlateR], MOT[MMPlate1+iP], iPlateR, iPlateC, NULL_IC, -1, true);  //Frank 20160505 add

                    InspectInArmPosition(MMPlate1+iP, iSuckR, iSuckC, iPlateR, iPlateC, false);

                    InArmSuck.PordRec[iSuckR][iSuckC].AddInArmHotplatePickRecord(iSuckR, iSuckC);                       //Sam 20200716 : Add record
                    if(iInArmType==e9045_1x2_4_Hot  ||
                       iInArmType==e9045_1x4_8_Hot  ||
                       iInArmType==e9045_2x1_2_13   ||
                       iInArmType==e9045_2x2_8_Hot)
                    {
                        AdjustShuttlePlaceOrder(0);
                    }

                    if(bRunAutoSiteMapping==false)                              //Ifor 20210426 add:執行Auto Site Mapping 保留原本資料避免無IC補回Hot plate發生Hang up
                    {
                        iHotWhichShuttle[iP][iPlateC][iPlateR]  =-1;
                        iHotWhichKit[iP][iPlateC][iPlateR]      =-1;
                        iHotPlateCount[iP][iPlateC][iPlateR]    =-1;
                    }
                    else                                                        //Steven 20211209 : 紀錄Auto Site Map動作
                    {
                        Str.sprintf("%d, %d, %d, %s, Pick", iP, iPlateR, iPlateC, InArmSuck.Suck[iSuckR][iSuckC].sName);
                        fMain->slAutoSiteMapLog->AddTextWithDateTime(Str);
                        InArmSiteMapData.UpdateData(iP, iPlateR, iPlateC, iSuckR, iSuckC);                              //Steven 20211209 : 紀錄Site map資料
                    }
                }
                else if(InArmSuck.Suck[iSuckR][iSuckC].Error==false)
                {
                    *flag1=false;
                }
            }
        }
        else
        {
            if(bZFlgToHPPick[iSuckR][iSuckC]==false &&
               InArmSuck.Item[iSuckR][iSuckC]==NULL_IC)                         //Ifor 20190507 : add 未使用的吸嘴設定HasNull IC
                InArmSuck.SetItemData(iSuckR, iSuckC, HAS_NULL_IC);
        }
    }

    if(LastSet.iRunStartMode==rsmAutoSiteMap &&
       CosFunction.bUSEJCETSiteMapMode==true && *flag1==true)                   //Steven 20220527 : for JCET Auto Site Map
    {
        bAutoSiteMapHasPickHP=true;
        PickFromHPList->SetPlateSuck(iP, iPlateR, iPlateC, false);
    }

    if(CosFunction.bShowHPICCount)                                              //Steven 20221228 : 計算加熱盤IC數量
    {
        fSortCT->pnlHP1->Caption=MOT[MMPlate1].Tray.HowManyIC();
        fSortCT->pnlHP2->Caption=MOT[MMPlate2].Tray.HowManyIC();
    }
}
//==============================================================================
bool InArmNeedCheckHotPlateOffset(bool bPlace)                                  //Steven 20230531 : 簡化判斷式
{
    bool bResult=false;
    InArmSuck.ResetAll();                                                       //Steven 20160323 : 避免未開啟真空
    if(bPlace)
    {
        if((AccessLevel>=1 || CosFunction.bOPCanPressStepAndTStart) &&          //JerryYang 20170417 (wei) OP權限也能按Step跟T.Start
           (Sen[SnRKManualTStart].IsOn() ||
            fOffSet->UseAutoOffsetFunction("Hot Plate"+AnsiString(2-iPlacePlate[0])) ||                                 //jou 2013-08-29 Use Auto Offset Funtion
            fOffSet->UseInArmSetupTeach(InOfsHP1+iPlacePlate[0])))              //JerryYang 20180921 Setup Teach功能
            bResult=true;
    }
    else
    {
        if((AccessLevel>=1 || CosFunction.bOPCanPressStepAndTStart) &&          //JerryYang 20170417 (wei) OP權限也能按Step跟T.Start
           (Sen[SnRKManualTStart].IsOn() ||
            fOffSet->UseAutoOffsetFunction("Hot Plate"+AnsiString(2-iPickPlate[0])) ||                                  //jou 2013-08-29 Use Auto Offset Funtion
            fOffSet->UseInArmSetupTeach(InOfsHP1+iPickPlate[0])))               //JerryYang 20180921 Setup Teach功能
            bResult=true;
    }

    if(bResult)
    {
        bEnterOffset=false;
        fMain->Pause("InArmNeedCheckHotPlateOffset");
    }
    return bResult;
}
//-----------------------------------------------------------------------------
int GetInArmToHPYPitch_9045()
{
    if(USE_IN_Y_IS_AUTO_PITCH==true)                                            //Steven for HT1032  //JerryYang 20251218 : IN/OUT ARM支援不同模組
    {
        if(InArmSuck.iPickRow==1)
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
        else
        {
            return HotPlateForm.YPitch*iYHalf;
        }
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
//==============================================================================
bool MoveInArmZToHotPlatePick(int iCT)
{
    bool flag[2][4]={{false, false, false, false}, {false, false, false, false}};

    GetInArmZToHPPos(iPickHP, iCT);

    for(int i=0; i<2; i++)
    {
        for(int j=0; j<4; j++)
        {
            if(bZFlgToHP[i][j])                                                 //Steven 20140217 : 兩段速移動
            {
                flag[i][j]=MOT[InArmZIndex[i][j]].MotorMove2SpeedForPicker(iZPosToHP[i][j], &ArmSpeed[InArm]);
            }
            else
            {
                flag[i][j]=true;
            }
        }
    }

    for(int i=0; i<2; i++)
    {
        for(int j=0; j<4; j++)
        {
            if(bZFlgToHP[i][j])
            {
                if(flag[i][j]==false)
                    return false;
            }
        }
    }
    return true;
}
//-----------------------------------------------------------------------------
bool MoveInArmXYPickHotPlate_9045(bool bNeedDown)
{
    int iXVariable[X_PITCH_COUNT];
    int iYVariable      =0;
    int iMovePitchX     =GetVariableXInHotPlateData(iPickHP);
    int iMovePitchY     =GetInArmToHPYPitch_9045();
    int iOffsetPos      =GetInArmToHPOffset_9045(iPickHP);
    int iP              =0;
    int iR              =iPickPlateY[0];
    int iC              =iPickPlateX[0];
    int iPlateR[MAX_ARM_Row][MAX_ARM_Col];                                      //Jimmychiu 20230417 : Record the position after placing the IC
    int iPlateC[MAX_ARM_Row][MAX_ARM_Col];
    double dMovePitchX  =0;
    ZeroMemory(iXVariable, sizeof(iXVariable));
    ZeroMemory(iPlateR, sizeof(iPlateR));
    ZeroMemory(iPlateC, sizeof(iPlateC));

    for(int i=0; i<X_PITCH_COUNT; i++)
        iXVariable[i]=GetInArmPitchX_9045(iMovePitchX, i, iOffsetPos);
    iYVariable=GetInArmPitchY_9045(iMovePitchY, iOffsetPos);

    if(LastSet.iRunStartMode==rsmAutoSiteMap &&
       CosFunction.bUSEJCETSiteMapMode==true)                                   //Steven 20220527 : for JCET Auto Site Map
    {
        bZFlgToHP    [iAutoSiteMapInArmRow][iAutoSiteMapInArmCol]=true;
        bZFlgToHPPick[iAutoSiteMapInArmRow][iAutoSiteMapInArmCol]=true;
        iP=iAutoSiteMapHPNo;
        iR=iAutoSiteMapHPR;
        iC=iAutoSiteMapHPC;

        iPickPlateX[0]=0;                                                       //RogerYang 20250709 重新計算pitch，避免用錯col位置造成Pitch錯誤
        iMovePitchX=GetVariableXInHotPlateData(iPickHP);
    }
    else
    {
        PickFromHPList->GetHPFirstTeam(&iP, iPlateR, iPlateC, bZFlgToHPPick);
        if(PickFromHPList->GetHPFirstTeamMotUse(bZFlgToHP)==false)              //RogerYang 20250624 Add HotPlate ErrMessage
        {
            ShowMyMessage(AnsiString().sprintf("%s %s Task=%d", __FUNC__, "No GetHPFirstTeamMotUse", 0));
            return false;
        }
    }

    if(bZFlgToHP[0][0]==false && bZFlgToHP[0][1]==false && bZFlgToHP[0][2]==false && bZFlgToHP[0][3]==false &&
       bZFlgToHP[1][0]==false && bZFlgToHP[1][1]==false && bZFlgToHP[1][2]==false && bZFlgToHP[1][3]==false)
        return true;

    if(iP==0)                                                                   // iPlate=0 先到2號加熱盤也就上上加熱盤
    {
        iHPXPos=Prod.XInArm_Plate2_Pick[iInArmYBase][iInArmXBase]+Prod.HotPlateForm[0].iXPitch*iC;
        iHPYPos=Prod.YInArm_Plate2_Pick[iInArmYBase][iInArmXBase]-Prod.HotPlateForm[0].iYPitch*iR;
    }
    else
    {
        iHPXPos=Prod.XInArm_Plate1_Pick[iInArmYBase][iInArmXBase]+Prod.HotPlateForm[0].iXPitch*iC;
        iHPYPos=Prod.YInArm_Plate1_Pick[iInArmYBase][iInArmXBase]-Prod.HotPlateForm[0].iYPitch*iR;
    }

    OnlyRow2=true;
    for(int j=0; j<4; j++)
    {
        if(bZFlgToHPPick[0][j]==true)
            OnlyRow2=false;
    }

    if(IniConfig.bA09_ByArmCloseSite)                                           //ChungHung 20130910 alter for SCK can close site by Index
    {
        if(USE_IN_Y_IS_AUTO_PITCH==true)                                        //JerryYang (Steven) 20170912 fix Y-Pitch機台 2x2 hot plate點位錯誤  //JerryYang 20251218 : IN/OUT ARM支援不同模組
        {
            if(OnlyRow2==false)
            {
                iHPYPos=iHPYPos-iMovePitchY;
            }
        }
        else
        {
            if(iCloseSiteState==0)                                              //ChungHung 20140610 fix CloseSitebyIndexArm 上排全關 動作異常
            {
                if(OnlyRow2)
                {
                    iHPYPos=iHPYPos+iMovePitchY;
                }
            }
            else
            {
                if(OnlyRow2)
                {
                    iHPYPos=iHPYPos+iMovePitchY;
                }
            }
        }
    }
    else
    {
        if(USE_IN_Y_IS_AUTO_PITCH==true)                                        //ChungHung 20140304 add for AutoYPitch  //JerryYang 20251218 : IN/OUT ARM支援不同模組
        {
            if(OnlyRow2==false)
            {
                iHPYPos=iHPYPos-iMovePitchY;
            }
        }
        else
        {
            if(OnlyRow2==true)
            {
                iHPYPos=iHPYPos+iMovePitchY;
            }
        }
    }

    if(USE_PICKER_COUNT==ep1Picker)
    {
    }
    else
    {
        if(bZFlgToHPPick[0][0] || bZFlgToHPPick[1][0])                          //RogerYang 20250819 改回來 bZFlgToHP->bZFlgToHPPick
        {
            if(USE_PICKER_COUNT==ep16Picker)                                    //基準為Ad
            {
                dMovePitchX =double(iMovePitchX)/7.0;
                iHPXPos=iHPXPos+dMovePitchX*3.0;                                //基準在X-Start位置, Aa 移動到Ad 的位置
            }
            else if(USE_IN_OUT_ARM_Y_PITCH==iXYPitchBb ||                       //基準為Ab //Steven for HT7080
                    USE_IN_OUT_ARM_Y_PITCH==iXYPitchIn_Bb_Out_Bc)               //Ztex 2024.02.24 Add HT-1132
            {
                dMovePitchX =double(iMovePitchX)/3.0;
                iHPXPos=iHPXPos+dMovePitchX*1.0;                                //基準在X-Start位置, Aa 移動到Ab 的位置
            }
            else                                                                //基準為Ac
            {
                dMovePitchX =double(iMovePitchX)/3.0;
                iHPXPos=iHPXPos+dMovePitchX*2.0;                                //基準在X-Start位置, Aa 移動到Ac 的位置
            }
        }
        else if(bZFlgToHPPick[0][1] || bZFlgToHPPick[1][1])                     //RogerYang 20250819 改回來 bZFlgToHP->bZFlgToHPPick
        {
            if(USE_PICKER_COUNT==ep16Picker)                                    //基準為Ad
            {
                dMovePitchX =double(iMovePitchX)/7.0;
                iHPXPos=iHPXPos+dMovePitchX*1.0;                                //基準在X-Start位置, Ac 移動到Ad 的位置
            }
            else if(USE_IN_OUT_ARM_Y_PITCH==iXYPitchBb ||                       //基準為Ab //Steven for HT7080
                    USE_IN_OUT_ARM_Y_PITCH==iXYPitchIn_Bb_Out_Bc)               //Ztex 2024.02.24 Add HT-1132
            {
                dMovePitchX =double(iMovePitchX)/3.0;
                iHPXPos=iHPXPos+dMovePitchX*0.0;                                //基準在X-Start位置, Ab 移動到Ab 的位置
            }
            else                                                                //基準為Ac
            {
                dMovePitchX =double(iMovePitchX)/3.0;
                iHPXPos=iHPXPos+dMovePitchX*1.0;                                //基準在X-Start位置, Ab 移動到Ac 的位置
            }
        }
        else if(bZFlgToHPPick[0][2] || bZFlgToHPPick[1][2])                     //RogerYang 20250819 改回來 bZFlgToHP->bZFlgToHPPick
        {
            if(USE_PICKER_COUNT==ep16Picker)                                    //基準為Ad
            {
                dMovePitchX =double(iMovePitchX)/7.0;
                iHPXPos=iHPXPos-dMovePitchX*1.0;                                //基準在X-Start位置, Ae 移動到Ad 的位置
            }
            else if(USE_IN_OUT_ARM_Y_PITCH==iXYPitchBb ||                       //基準為Ab //Steven for HT7080
                    USE_IN_OUT_ARM_Y_PITCH==iXYPitchIn_Bb_Out_Bc)               //Ztex 2024.02.24 Add HT-1132
            {
                dMovePitchX =double(iMovePitchX)/3.0;
                iHPXPos=iHPXPos-dMovePitchX*1.0;                                //基準在X-Start位置, Ac 移動到Ab 的位置
            }
            else                                                                //基準為Ac
            {
                dMovePitchX =double(iMovePitchX)/3.0;
                iHPXPos=iHPXPos+dMovePitchX*0.0;                                //基準在X-Start位置, Ac 移動到Ac 的位置
            }
        }
        else if(bZFlgToHPPick[0][3] || bZFlgToHPPick[1][3])                     //RogerYang 20250819 改回來 bZFlgToHP->bZFlgToHPPick
        {
            if(USE_PICKER_COUNT==ep16Picker)                                    //基準為Ad
            {
                dMovePitchX =double(iMovePitchX)/7.0;
                iHPXPos=iHPXPos-dMovePitchX*3.0;                                //基準在X-Start位置, Ag移動到Ad 的位置
            }
            else if(USE_IN_OUT_ARM_Y_PITCH==iXYPitchBb ||                       //基準為Ab //Steven for HT7080
                    USE_IN_OUT_ARM_Y_PITCH==iXYPitchIn_Bb_Out_Bc)               //Ztex 2024.02.24 Add HT-1132
            {
                dMovePitchX =double(iMovePitchX)/3.0;
                iHPXPos=iHPXPos-dMovePitchX*2.0;                                //基準在X-Start位置, Ad 移動到Ab 的位置
            }
            else                                                                //基準為Ac
            {
                dMovePitchX =double(iMovePitchX)/3.0;
                iHPXPos=iHPXPos-dMovePitchX*1.0;                                //基準在X-Start位置, Ad 移動到Ac 的位置
            }
        }
    }

    if(bNeedDown)
    {
        GetInArmZToHPPos(iPickHP, 0);
    }
    else
    {
        for(int i=0; i<2; i++)
        {
            for(int j=0; j<4; j++)
            {
                iZPosToHP[i][j]=ZSafePos;
            }
        }
    }

    TransferHotPlateRatio(true, &iHPXPos, &iHPYPos);                            //Steven 20110324
    if(InArmContinuousMove_9045(iHPXPos, iHPYPos, iXVariable, iYVariable, bZFlgToHP, iZPosToHP, ZAxisDown, bNeedDown))  //RogerYang 20250819 改回來 bZFlgToHPPick->bZFlgToHP
    {
        return true;
    }
    return false;
}
//==============================================================================
bool DoInArmPickFromHotPlate_9045()
{
    int &Task=iInArmPickFromHotPlateTask, ret;
    static int iRetryCT=0;
    int &iP=iPickPlate[0];
    bool flag1=false;
    static bool bCheckSpeed=false;

    int iPlateR[MAX_ARM_Row][MAX_ARM_Col];                                      //Jimmychiu 20230417 : Record the position after placing the IC
    int iPlateC[MAX_ARM_Row][MAX_ARM_Col];
    bool bSuck[MAX_ARM_Row][MAX_ARM_Col];
    ZeroMemory(iPlateR, sizeof(iPlateR));
    ZeroMemory(iPlateC, sizeof(iPlateC));
    ZeroMemory(bSuck, sizeof(bSuck));

    AnsiString ErrPart="";                                                      //Steven 20100104 : 整合AlarmCodde
    bool bHasDuplicateErr=false;                                                //Steven 20100104 : 整合AlarmCodde

    GetHotPlateYHalfPos();

    switch(Task)
    {
        case 1:
            DoInArm_9045_SuckerMap();                                           //Steven 20220531 : for NN mode Auto Site Map
            if(SearchPlateToPick()==false)
            {
                ShowMyMessage(AnsiString().sprintf("%s %s", __FUNC__, "No PickFromHPList"));
                return true;
            }
            bPickFormHotplatePartOK=false;                                      //ChungHung 20120112 Hang Up 解除
            bPickFormHotplateRetry=false;                                       //Ifor 20160616 add Hot plate Pickup error Retry Hang Up 解除
            Task=50;
        case 50:
            if(MoveInArmZToPlateSafe(Task))
            {
                Task=100;
            }
            break;
        case 100:
            iRetryCT=0;
            if(LastSet.iRunStartMode==rsmAutoSiteMap &&
               CosFunction.bUSEJCETSiteMapMode==true)                                                                   //Steven 20220527 : for JCET Auto Site Map
            {
                ZeroMemory(bZFlgToHP, sizeof(bZFlgToHP));
                ZeroMemory(bZFlgToHPPick, sizeof(bZFlgToHPPick));
                bZFlgToHP[iAutoSiteMapInArmRow][iAutoSiteMapInArmCol]=true;
                bZFlgToHPPick[iAutoSiteMapInArmRow][iAutoSiteMapInArmCol]=true;
            }
            else
            {
                if(PickFromHPList->GetHPFirstTeamMotUse(bZFlgToHP)==false)                                              //RogerYang 20250624 Add HotPlate ErrMessage
                {
                    ShowMyMessage(AnsiString().sprintf("%s %s Task=%d", __FUNC__, "No GetHPFirstTeamMotUse", Task));
                    break;
                }

                if(PickFromHPList->GetHPFirstTeamSuckUse(bZFlgToHPPick)==false)                                         //RogerYang 20250624 Add HotPlate ErrMessage
                {
                    ShowMyMessage(AnsiString().sprintf("%s %s Task=%d", __FUNC__, "No GetHPFirstTeamSuckUse", Task));
                    break;
                }
            }
            Task=110;
            break;
        case 105:
            if(MoveInArmZToPlateSafe(Task))
            {
                Task=110;
            }
            break;
        case 110:
            if(MoveInArmXYPickHotPlate_9045(false))
            {
                if(IsCheckInArmDestroyActiveFinish())                           //ChungHung 201112296 add Pick 前先確認CheckInArmDestroyActive 已完成
                {
                    Task=150;
                }
            }
            break;
        case 150:
            if(InArmSuck.iWhichShtPickFor32==0)                                 //Steven 20160201 : 修正Shuttle放料順序
            {
                if(bCheckSpeed)                                                 //jou 2012-07-16 Auto Speed Hot Mode Fix
                {
                    if(FLCarryKit.UseSiteHasIC())
                        InArmSubSpeed();
                    else
                        InArmAddSpeed();

                    bCheckSpeed=false;
                }

                if(InSHT1InLF())
                {
                    if(USE_PICKER_COUNT==ep1Picker)                             //20250901 : todo
                    {
                        if(iOneCycle==1 &&                                      //Steven 20240605 : Fix for one cycle hang up
                           InArmSuck.HasIC()==false &&
                           (IsFLCarrKitAllHasIC()))                             //Steven 20250424 : 修正要吸多次加熱盤的時候按了one cycle hang up
                        {
                            Task=1;
                            return true;
                        }
                        Task=190;                                               //ChungHung 20120724 在Task=150~Task=200時 進入Contact 做動作InArm位置會被修改造成HangUp
                    }
                    else
                    {
                        if(InArmSuck.iWhichKitPickFor32==0)
                        {
                            if(FLCarryKit.LeftSideNoIC(InArmSuck.iShtKitStep)==false)
                            {
                                MOT[MInShuttle1].fCanMoveL=true;
                                SetShuttleToHasNullIC_9045(0, 0);
                            }
                            else
                            {
                                if(iOneCycle==1 &&                              //Steven 20240605 : Fix for one cycle hang up
                                   InArmSuck.HasIC()==false)                    //Steven 20250424 : 修正要吸多次加熱盤的時候按了one cycle hang up
                                {
                                    Task=1;
                                    return true;
                                }

                                Task=190;                                       //ChungHung 20120724 在Task=150~Task=200時 進入Contact 做動作InArm位置會被修改造成HangUp
                            }
                        }
                        else
                        {
                            if(FLCarryKit.RightSideNoIC(InArmSuck.iShtKitStep)==false)
                            {
                                MOT[MInShuttle1].fCanMoveL=true;
                                SetShuttleToHasNullIC_9045(0, 1);
                            }
                            else
                            {
                                Task=190;                                       //ChungHung 20120724 在Task=150~Task=200時 進入Contact 做動作InArm位置會被修改造成HangUp
                            }
                        }
                    }
                }
                else if((IniConfig.bP56TrayArmWaitAtColorTrack ||               //jou 20240910 : 修正tray arm 讓位 hang up
                         (INSTALL_OCR!=eocrUninstal && CosFunction.bTrayOCR)) &&
                        NeedWaitTrayArm==true)                                  //Ifor 20180510 : Fix OCR機型IniConfig.bCloseSiteByIndexArm未開啟時會丟同一個蝦頭導致Hang up //jou 20240910 : 修正tray arm 讓位 hang up
                {
                    return true;
                }
                else if(BLCarryKit.UseSiteNoIC() && InSHT2InLF())
                {
                    if(LastSet.iTemperature==Tempture_Hot &&                    //jou 2010-08-05 start : 當加熱模式下in arm pick up hotplate & out shuttle 1 has ic & arm 2 has ic & 其他沒有ic時 hang up
                       iOneCycle==1 &&
                       IndexStatus==Z1Down_Z2Up)
                    {
                        if(InArmSuck.HasType(HAS_HOT_IC)==true &&
                           ShuttleHasIC()==false &&
                           IndexHasIC()==false &&
                           MOT[MInShuttle1].fCanMoveM==false &&
                           OutArmSuck.HasIC()==false)
                        {
                            MOT[MInShuttle1].fCanMoveM=true;
                        }
                    }
                }
            }
            else
            {
                if(bCheckSpeed)                                                 //jou 2012-07-16 Auto Speed Hot Mode Fix
                {
                    if(BLCarryKit.UseSiteHasIC())
                        InArmSubSpeed();
                    else
                        InArmAddSpeed();

                    bCheckSpeed=false;
                }

                if(InSHT2InLF())
                {
                    if(USE_PICKER_COUNT==ep1Picker)
                    {
                        Task=190;                                               //ChungHung 20120724 在Task=150~Task=200時 進入Contact 做動作InArm位置會被修改造成HangUp
                    }
                    else if(InArmSuck.iWhichKitPickFor32==0)
                    {
                        if(BLCarryKit.LeftSideNoIC(InArmSuck.iShtKitStep)==false)
                        {
                            MOT[MInShuttle2].fCanMoveL=true;
                            SetShuttleToHasNullIC_9045(0, 0);
                        }
                        else
                        {
                            Task=190;                                           //ChungHung 20120724 在Task=150~Task=200時 進入Contact 做動作InArm位置會被修改造成HangUp
                        }
                    }
                    else
                    {
                        if(BLCarryKit.RightSideNoIC(InArmSuck.iShtKitStep)==false)
                        {
                            MOT[MInShuttle2].fCanMoveL=true;
                            SetShuttleToHasNullIC_9045(0, 1);
                        }
                        else
                        {
                            Task=190;                                           //ChungHung 20120724 在Task=150~Task=200時 進入Contact 做動作InArm位置會被修改造成HangUp
                        }
                    }
                }
                else if((IniConfig.bP56TrayArmWaitAtColorTrack ||               //jou 20240910 : 修正tray arm 讓位 hang up
                         (INSTALL_OCR!=eocrUninstal && CosFunction.bTrayOCR)) &&
                        NeedWaitTrayArm==true)                                  //Ifor 20180510 : Fix OCR機型IniConfig.bCloseSiteByIndexArm未開啟時會丟同一個蝦頭導致Hang up //jou 20240910 : 修正tray arm 讓位 hang up
                {
                    return true;
                }
            }
            break;
        case 190:                                                               //ChungHung 20120724 在Task=150~Task=200時 進入Contact 做動作InArm位置會被修改造成HangUp
            if(MoveInArmXYPickHotPlate_9045(false)==false)
            {
                return false;
            }

//            if(iOneCycle==1)                                                  //Steven 20240701 : Mark
//            {
//                Task=1;
//                return true;
//            }
//            else
            {
                Task=200;
            }
        case 200:
            if(LastSet.iRealDummy==REALLY &&
               ArmSpeed[InArm].bSuckOnDown &&
               Sen[SnRKManualTStart].IsOn()==false)
            {
                bPickFromHotplate=true;
                for(int i=0; i<2; i++)
                    for(int j=0; j<4; j++)
                    {
                        if(bZFlgToHPPick[i][j]==true)
                        {
                            InArmSuck.Suck[i][j].On();
                        }
                    }
            }

            for(int i=0; i<2; i++)
            {
                for(int j=0; j<4; j++)
                {
                    if(bZFlgToHP[i][j]==true &&                                 //Jimmychiu 20250529 : fixed for Retry pick ic from plate
                       InArmSuck.Item[i][j]!=NULL_IC)
                    {
                        bZFlgToHP[i][j]=false;
                    }
                }
            }

            if(MoveInArmZToHotPlatePick(iRetryCT))
            {
                if(InArmNeedCheckHotPlateOffset(false))                         //Steven 20230531 : 簡化判斷式
                {
                    Task=210;
                    break;
                }

                InArmSuck.ResetAll();                                           //Steven 20160323 : 避免未開啟真空
                Task=300;
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
                Task=200;
            }
            break;
        case 220:
            if(MoveInArmZToPlateSafe(Task))
            {
                Task=110;
            }
            break;
        case 300:
            bPickFromHotplate=true;
            flag1=true;
            BackupHotplatelocation();                                                                                   //Ifor 20171006 (Steven) : add 備份Hotplate取料位置
            if(LastSet.iRunStartMode==rsmAutoSiteMap &&
               CosFunction.bUSEJCETSiteMapMode==true)                                                                   //Steven 20220527 : for JCET Auto Site Map
            {
                HotplateDataConversion(iAutoSiteMapInArmRow, iAutoSiteMapInArmCol, iAutoSiteMapHPNo, iAutoSiteMapHPR, iAutoSiteMapHPC, &flag1);
                if(flag1==true)
                    bAutoSiteMapHasPickHP=true;
            }
            else
            {
                if(PickFromHPList->GetHPFirstTeam(&iP, iPlateR, iPlateC, bSuck))                                        //RogerYang 20250819 改回來
                {
                    //PickFromHPList->GetHPFirstTeamMotUse(bSuck);               //RogerYang 20250613 Z軸真的有下去吸
                    for(int i=0; i<2; i++)
                    {
                        for(int j=0; j<4; j++)
                        {
                            if(bSuck[i][j]==true)
                            {
                                HotplateDataConversion(i, j, iP, iPlateR[i][j], iPlateC[i][j], &flag1);                 //Ifor 20180518 : add 整合Hot plate 資料轉換
                            }
                        }
                    }
                }
            }

            if(flag1==false)                                                                                            //KEVIN 20130421 按RETRY資料尚未交換會將其他IC丟掉
                break;

            for(int i=0; i<2; i++)
            {
                for(int j=0; j<4; j++)
                {
                    if(InArmSuck.Suck[i][j].Error)
                    {
                        Task=320;
                        return false;
                    }
                    else
                    {
                        bPickHPDuplicateErr[i][j]=false;
                    }
                }
            }

            #ifdef SOFT_SIMULTE
            PickFromHPList->SaveFile(sHPPickRec);
            #endif
            bPickFormHotplatePartOK=true;                                                                               //ChungHung 20120112 Hang Up 解除
            Task=340;
            break;
        case 320:
            if(MoveInArmZToPlateSafe(Task))
            {
                iRetryCT++;
                Task=330;
            }
            break;
        case 330:
            if(iRetryCT>ArmSpeed[InArm].iRetryCT)                                                                       //Steven 20120109 : 整合成一次Skip多顆
            {
                ErrPart=" ";
                bHasDuplicateErr=false;
                for(int i=0; i<2; i++)
                {
                    for(int j=0; j<4; j++)
                    {
                        if(bPickHPDuplicateErr[i][j])
                            bHasDuplicateErr=true;

                        if(InArmSuck.Suck[i][j].Error)
                        {
                            bPickHPDuplicateErr[i][j]=true;
                            ErrPart+=InArmSuck.Suck[i][j].sName;
                        }
                    }
                }

                iRetryCT=0;
                ret=0;
                if(IniConfig.bInOutArmCanPushHome)
                {
                    ret=ShowErrorMessage("JAM0109", K_RETRY|K_SKIP|K_HOME, MInArmX, bHasDuplicateErr, ErrPart);         //ChungHung HT9045 2011/12/13 //Input pickup device error時,按"retry"鍵,機台都會自動home
                }
                else
                {
                    ret=ShowErrorMessage("JAM0109", K_RETRY|K_SKIP, MInArmX, bHasDuplicateErr, ErrPart);                //Steven 20100104
                }

                if(ret==K_SKIP)
                {
                    bAutoSiteMapWaitTestResult=false;                                                                   //Ifor 20180115 (Steven) : add Site Mapping SKIP 需清除旗標
                    if(LastSet.iRunStartMode==rsmAutoSiteMap &&
                       CosFunction.bUSEJCETSiteMapMode==true)                                                           //Steven 20220527 : for JCET Auto Site Map
                    {
                        PorcessJAM0109HotPlatePickUpErrorSkip(iAutoSiteMapInArmRow, iAutoSiteMapInArmCol, iAutoSiteMapHPNo, iAutoSiteMapHPR, iAutoSiteMapHPC);
                        bPickHPDuplicateErr[iAutoSiteMapInArmRow][iAutoSiteMapInArmCol]=false;
                    }
                    else
                    {
                        PickFromHPList->GetHPFirstTeam(&iP, iPlateR, iPlateC, bSuck);
                        for(int i=0; i<2; i++)
                        {
                            for(int j=0; j<4; j++)
                            {
                                if(InArmSuck.Suck[i][j].Error)
                                {
                                    PorcessJAM0109HotPlatePickUpErrorSkip(i, j, iP, iPlateR[i][j], iPlateC[i][j]);      //Steven 20161214 : 將HP吸取異常SKIP統一成Function
                                    bPickHPDuplicateErr[i][j]=false;
                                }
                            }
                        }
                    }
                    Task=340;                                                                                           //kevin 20150514   HOTPLATE不要再下去吸
                    return false;                                                                                       //kevin 20150514
                }
                else
                {
                    InArmSuck.ResetAll();                                                                               //Steven 20250227 : 避免未開啟真空
                    bPickFormHotplateRetry=true;                                                                        //Ifor 20160616 add Hot plate Pickup error Retry Hang Up 解除
                    if(IniConfig.bInOutArmCanPushHome)
                    {
                        if(ret==K_HOME)                                                                                 //ChungHung HT9045 2011/12/13 //Input pickup device error時,按"retry"鍵,機台都會自動home start
                        {
                            Task=335;
                        }
                        else
                        {
                            Task=336;
                        }
                    }
                    else
                    {
                        Task=335;
                    }
                }
                return false;
            }
            Task=190;                                                                                                   //JerryYang 20250612 : fix offset當下無作用
            break;
        case 335:
            SetInArmHome();
            iRetryCT=0;
            Task=190;                                                           //JerryYang 20250612 : fix offset當下無作用
            break;
        case 336:                                                               //ChungHung HT9045 2011/12/13 //Input pickup device error時,按"retry"鍵,機台都會自動home start
            iRetryCT=0;
            Task=190;                                                           //JerryYang 20250612 : fix offset當下無作用
            break;
        case 340:
            if(MoveInArmZToPlateSafe(Task))
            {
                Task=350;
            }
            break;
        case 350:
            if(LastSet.iRunStartMode==rsmAutoSiteMap &&
               CosFunction.bUSEJCETSiteMapMode==true)                           //Steven 20220527 : for JCET Auto Site Map
            {
                Task=400;
            }
            else
            {
                if(PickFromHPList->DataForwardAndNextTeam())
                {
                    #ifdef SOFT_SIMULTE
                    PickFromHPList->SaveFile(sHPPickRec);
                    #endif
                    Task=1;                                                     //Steven 20150921 : For HP Skip Hang Up
                    break;
                }
                else
                {
                    Task=400;
                }
            }
        case 400:
            if(ArmSpeed[InArm].bAutoSpeed &&                                    //jou 2012-07-16 Auto Speed Hot Mode Fix
               LastSet.iTemperature==Tempture_Hot &&
               OnlyRow2==false)
                bCheckSpeed=true;
            else
                bCheckSpeed=false;

            if(iPickPlate[0]==0)                                                //JerryYang 20180921 Setup Teach功能
            {
                iInArmPickPlaceCnt[InOfsHP1]++;
            }
            else
            {
                iInArmPickPlaceCnt[InOfsHP2]++;
            }

            Task=1;
            return true;
    }
    return false;
}
//==============================================================================
bool SearchPickPlateXItem_AutoSiteMap()                                         //Steven 20220527 : for JCET Auto Site Map
{
    bool bHasIC=false;

    for(int iP=0; iP<2; iP++)
    {
        for(int iR=0; iR<HotPlateForm.YDivision; iR++)
        {
            for(int iC=0; iC<HotPlateForm.XDivision; iC++)
            {
                if(MOT[MMPlate1+iP].Tray.iWhichSite[iC][iR]==iAutoSiteMapSiteNo)
                {
                    bHasIC=true;
                    if(bUseTwoArm32Site==true)
                    {
                        if(MOT[MMPlate1+iP].Tray.Data[iC][iR]==HAS_HOT_IC)
                        {
                            iAutoSiteMapHPC=iC;
                            iAutoSiteMapHPR=iR;
                            iAutoSiteMapHPNo=iP;
                            InArmSuck.iWhichSht         =iHotWhichShuttle[iP][iC][iR];                                  //Steven 20220913 : Fixed for ASM function
                            InArmSuck.iWhichKit         =iHotWhichKit[iP][iC][iR];
                            InArmSuck.iWhichShtPickFor32=iHotWhichShuttle[iP][iC][iR];
                            InArmSuck.iWhichKitPickFor32=iHotWhichKit[iP][iC][iR];
                            return true;
                        }
                    }
                    else
                    {
                        if(MOT[MMPlate1+iP].Tray.Data[iC][iR]==HAS_HOT_IC)
                        {
                            iAutoSiteMapHPC=iC;
                            iAutoSiteMapHPR=iR;
                            iAutoSiteMapHPNo=iP;
                            InArmSuck.iWhichSht         =iAutoSiteMapHPToSht;   //Steven 20220913 : Fixed for ASM function
                            InArmSuck.iWhichKit         =iHotWhichKit[iP][iC][iR];
                            InArmSuck.iWhichShtPickFor32=iAutoSiteMapHPToSht;   //Steven 20220913 : Fixed for ASM function
                            InArmSuck.iWhichKitPickFor32=iHotWhichKit[iP][iC][iR];
                            return true;
                        }
                    }
                }
            }
        }
    }

    if(bHasIC==false)
    {
        iDoSiteMappingStep++;
    }
    return false;
}
//------------------------------------------------------------------------------
bool SearchPlateToPick()                                                        //Steven 20240311 : 重新整理SearchPlateToPick()
{
    if(LastSet.iRunStartMode==rsmAutoSiteMap &&
       CosFunction.bUSEJCETSiteMapMode==true)                                   //Steven 20220527 : for JCET Auto Site Map
    {
        SearchPickPlateXItem_AutoSiteMap();
    }
    else
    {                                                                           //Jimmychiu 20230417 : Record the position after placing the IC
        if(PickFromHPList->GetHPFirstTeamPlate(iPickPlate[0], iPickPlateY[0], iPickPlateX[0], InArmSuck.iWhichShtPickFor32, InArmSuck.iWhichKitPickFor32)==false)
        {
            return false;
        }
    }

    if(TestIF.iShuttleMode==1)
    {
        if(TestIF.iShuttle_Sel==0)
            InArmSuck.iWhichShtPickFor32=0;
        else if(TestIF.iShuttle_Sel==1)
            InArmSuck.iWhichShtPickFor32=1;
    }

    #ifndef SOFT_SIMULTE                                                        //kevin 20220914 add  SOFT_SIMULTE 不檢查安全門
    CheckSafeDoorIsClosed();                                                    //Sam 20220829 : 發生 HotPlateHangUp 時必須要先檢查安全門
    #endif
    return true;
}
//------------------------------------------------------------------------------
bool HasHotReadyIC_9045()
{
    bool bHasIC=false, bNeedSuck=false;
    int iP=0, iR=0, iC=0, iSht=0, iKit=0;

    if(iPickPlate[0]<0  || iPickPlateY[0]<0   || iPickPlateX[0]<0 ||
       iPickPlate[0]>=2 || iPickPlateY[0]>=50 || iPickPlateX[0]>=50)
    {
        return false;
    }

    if(bRunAutoSiteMapping==true)                                               //Ifor 20180518 : add 簡化Site Mapping 旗標
    {
        if(CosFunction.bUSEJCETSiteMapMode==true)                               //Steven 20220527 : for JCET Auto Site Map
        {
            if(bAutoSiteMapHasPickHP==false)
                return SearchPickPlateXItem_AutoSiteMap();
            else
                return false;
        }
        else if(bAutoSiteMapHotplateReady==false && iCleanOut==0)               //Ifor 20180212 (Steven) : add 避免Clean Out 時 Hang up
        {
            return false;
        }
    }
    else
    {
        if(LastSet.iRunStartMode==rsmAutoSiteMap)                               //jou 2016-11-07 JCET fix Auto Site mapping Hot mode hangup
        {
            return false;
        }
    }

    if(MOT[MMPlate2].HasIC() || MOT[MMPlate1].HasIC())
        SearchPlateToPick();
    else
        return false;

    if(CUSTOMER_CODE==CC_ASE_KaohSiung &&                                       //kevin 20201015 add ASE off line no check Temp
       LastSet.iTester==OFF_LINE && LastSet.iTemperature==Tempture_Hot)
    {
        return true;
    }
    else
    {
        bool bReady=true;
        //不指定shuttle吸取
        TList* lsPoint2D=new TList();
        if(PickFromHPList->GetHPFirstTeamToList(&iP, lsPoint2D))
        {
            for(int i=0; i<lsPoint2D->Count; i++)
            {
                uPoint2D* p2d=(uPoint2D*)lsPoint2D->Items[i];
                iR=p2d->Y;
                iC=p2d->X;
                if(iP>=0 && iR>=0 && iC>=0 &&
                   iP<2  && iR<50 && iC<50)
                {
                    if(MOT[MMPlate1+iP].Tray.Data[iC][iR]!=NULL_IC)
                    {
                        bHasIC=true;
                    }

                    if(bHasIC &&
                       HotTime[iP][iC][iR]>=Prod.iHotTime)
                    {
                        //pass
                    }
                    else
                    {
                        bReady=false;
                    }
                }
                else
                {
                    bReady=false;
                }
            }

            if(bReady==false && bHasIC==true)                                   //JimmyChiu 20250707 : try to fix hang up.
            {
                return bReady;
            }

            if(bHasIC==false)                                                   //Steven 20250226 : 發現多餘的組別,填入HAS_NULL_IC
            {
                PickFromHPList->SaveFile(sHPPickRecException);
                PickFromHPList->SaveFile(sHPPickRec);
                if(PickFromHPList->GetHPFirstTeamMotUse(bZFlgToHP)==false)      //RogerYang 20250624 Add HotPlate ErrMessage
                {
                    ShowMyMessage(AnsiString().sprintf("%s %s Task=%d", __FUNC__, "No GetHPFirstTeamMotUse", 0));
                    return false;
                }
                bNeedSuck=false;
                for(int i=0; i<MAX_ARM_Row; i++)
                {
                    for(int j=0; j<MAX_ARM_Col; j++)
                    {
                        if(bZFlgToHP[i][j])
                        {
                            bNeedSuck=true;
                        }
                    }
                }

                if(bNeedSuck==false)                                            //Jimmychiu 20250220 : 將不執行的資料刪除並搜尋下一筆
                {
                    PickFromHPList->DataForwardAndNextTeam();
                    MyDBIProcess("Exception", "HasHotReadyIC : Clear First team, all Site=-1."); //AI(general) 20260408 (RogerYang) : fix all Site=-1 return true bug
                    bReady=false;                                               //AI(general) 20260408 (RogerYang) : Site=-1 team skip, return false
                }
                else if(PickFromHPList->GetHPFirstTeamPlate(iP, iR, iC, iSht, iKit))
                {
                    for(int i=0; i<lsPoint2D->Count; i++)
                    {
                        uPoint2D* p2d=(uPoint2D*)lsPoint2D->Items[i];
                        iR=p2d->Y;
                        iC=p2d->X;
                        if(iP>=0 && iR>=0 && iC>=0 &&
                           iP<2  && iR<50 && iC<50)
                        {
                            if(MOT[MMPlate1+iP].Tray.Data[iC][iR]==NULL_IC)
                            {
                                iHotWhichShuttle[iP][iC][iR]  =iSht;
                                iHotWhichKit[iP][iC][iR]      =iKit;
                                iHotPlateCount[iP][iC][iR]    =0;
                                MOT[MMPlate1+iP].SetTraySingleData(iC, iR, HAS_NULL_IC);
                            }
                        }
                    }
                    MyDBIProcess("Exception", "HasHotReadyIC : Reset First team to HAS_NULL_IC.");
                }
                else
                {
                    PickFromHPList->DataForwardAndNextTeam();
                    MyDBIProcess("Exception", "HasHotReadyIC : Clear First team with bNeedSuck==true.");
                }
            }
        }
        else
        {
            ShowMyMessage(AnsiString().sprintf("%s %s", __FUNC__, "No PickFromHPList"));
            bReady=false;
        }
        lsPoint2D->Clear();
        delete lsPoint2D;
        return bReady;
    }
//    return false;
}
//------------------------------------------------------------------------------

