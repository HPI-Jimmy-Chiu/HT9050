// =============================================================================
//  cSpeed.cpp  --  Arm/Shuttle/Index/Magazine speed & timing recipe display:
//                   current-value read + widget fill (batch-5 cSpeed wave,
//                   display-only bucket -- see forms/fSpeed.h banner FIRST,
//                   it documents the full 57-method classification, GATE
//                   register, and design notes; not repeated in full here).
//
//  Faithful translation of golden cSpeed.cpp (2,517 lines, BCB6, cp950).
//  Translator: AI(W906-FW3-Speed-WA) 20260819.
//  Translation wave: FW-3 cSpeed Wave A (batch 5, display-only mandate).
//  Facade: forms/fSpeed.h.
//
//  WAVE SCOPE -- 6 of 57 golden methods (bucket (a), display/read-only; see
//  forms/fSpeed.h banner for the other 51's classification):
//    TfSpeed()        golden :31-39    (ctor)
//    FormShow         golden :41-338   (298 lines)
//    ReadFile         golden :340-783  (444 lines)
//    ReadWriteFile    golden :784-1006 (read arm only, see GATE (S5)/(S6))
//    DoIniDataToForm  golden :1008-1215 (207 lines)
//    FormClose        golden :1272-1280 (GOLDEN ODDITY: discard, not save)
//
//  GATE REGISTER -- full rationale in forms/fSpeed.h, re-cited briefly at
//  each call site below:
//   (S1) ctor -- edtHPVacuumDelay->Hint, no vclcompat port for `Hint`.
//   (S2) FormShow -- PageControl1->ActivePage, vclcompat::TPageControl only
//        carries ActivePageIndex (int).
//   (S3) FormShow -- rbTemp->SetFocus(), vclcompat::TControl carries no
//        SetFocus().
//   (S5) ReadWriteFile -- dmTrayMotor (golden Motor/TrayStepMotor.h/.cpp) has
//        no port anywhere in this tree.
//   (S6) ReadWriteFile -- write arm forced off (bRead pinned true).
//
//  DEPENDENCY AUDIT (this wave, 20260819) -- every non-widget global/struct
//  member the 6 in-scope methods touch was grepped against this port tree
//  before translating. Confirmed present with matching field names:
//    cmydef.h    : CosFunction/IniConfig field flags cited by name below are
//                  actually on CosFunction (CosFunction.h)/IniConfig (Config.h)
//                  -- cmydef.h itself supplies CUSTOMER_CODE/CC_*/
//                  LoaderUnload_StepMotor/LOAD_Z_USE_MOTOR[]/AUTO_EMPTY_COLOR/
//                  USE_ROTATE_KIT/iRotate_Type/AUTO3_IS_MAGAZINE/
//                  USE_CKD_FCM_CleanAir/USE_DIE_CLEAN/USE_OUT_Y_IS_AUTO_PITCH/
//                  USE_IN_Y_IS_AUTO_PITCH/InOutArmPickerUseMotor/
//                  INDEX_SUCKER_TYPE/ArmMaxPitch(_13Suck)/iEPControlValue/
//                  AccessLevel/iDefHonPrecLevel (all verified present,
//                  `grep -n` against cmydef.h, this wave).
//    cprod.h     : ArmSpeed_File[]/AutoArmSpeed[]/SHSpeed_File/MGSpeed_File
//                  (ARM_CONDITION/SHUTTLE_SPEED/MAGAZINE_SPEED struct bodies
//                  read in full this wave -- every field cSpeed.cpp touches
//                  is present with a matching name), TestIF_File
//                  (SYSTEM_TEST_IF), DeviceForm, TrayForm, UserDefForm_File[4],
//                  RunInfo (RUN_INFO, .bLotStart confirmed).
//    Config.h    : IniConfig (HT9045_CONFIG).
//    CosFunction.h: CosFunction (HT9045_COUSTOMER_FUNCTION).
//    LastSet.h   : LastSet (LAST_GENERAL_SET), rsmFIFOMode.
//    MachineType.h: eSpeedPart (IndexArm/InArm/OutArm/TrayArm/InRotate/
//                  OutRotate/SpeedPartTotal), QualSite1X4/_8Site2X4,
//                  eRotateType::eCynRotate, eptUseMot, CheckRange<T>().
//    common.h    : ReadIniData*/CheckAndReadIniData*/ReadWriteIni* (all 4
//                  overload families), MyForceDirectories, GetLastOpenFN,
//                  DataPath, AuthPath, sSaveByMachine,
//                  asTrayStepSpeedByMachinePatch.
//    csystem.h   : ShuttleHasIC(), IndexHasIC().
//    aHotPlateSubstrate.h: InArmSuck/OutArmSuck (TMyKitSuck -- the 177-TU
//                  shape; deliberately NOT mykitsuck.h, see this tree's own
//                  two-TMyKitSuck-headers gotcha).
//    cAuthority.h: authMainForm[12], bAuthCriticalPara[26].
//    forms/fSecurity.h: fSecurity (TfSecurity*), ::Insufficient(int,bool).
//  NOT found / gated: dmTrayMotor (GATE (S5)), fRPDefault (irrelevant to
//  this wave's 6 methods -- only SearchRecipeParameter/DoReplyDefaultToForm
//  touch it, both excluded per forms/fSpeed.h GATE (S4)).
// =============================================================================
#include "MachineDefine.h"     // de-VCL'd include hub: vclcompat umbrella + portable STL
#pragma hdrstop

#include "forms/fSpeed.h"

#include "cmydef.h"             // CUSTOMER_CODE/CC_*/LoaderUnload_StepMotor/LOAD_Z_USE_MOTOR[]/
                                 //   AUTO_EMPTY_COLOR/USE_ROTATE_KIT/iRotate_Type/AUTO3_IS_MAGAZINE/
                                 //   USE_CKD_FCM_CleanAir/USE_DIE_CLEAN/USE_OUT_Y_IS_AUTO_PITCH/
                                 //   USE_IN_Y_IS_AUTO_PITCH/InOutArmPickerUseMotor/INDEX_SUCKER_TYPE/
                                 //   ArmMaxPitch(_13Suck)/iEPControlValue/AccessLevel/iDefHonPrecLevel
#include "cprod.h"               // ArmSpeed_File[]/AutoArmSpeed[]/SHSpeed_File/MGSpeed_File/
                                 //   TestIF_File/DeviceForm/TrayForm/UserDefForm_File[]/RunInfo
#include "Config.h"              // IniConfig
#include "CosFunction.h"         // CosFunction
#include "LastSet.h"             // LastSet, rsmFIFOMode
#include "MachineType.h"         // eSpeedPart/QualSite1X4/_8Site2X4/eCynRotate/eptUseMot/CheckRange
#include "common.h"              // ReadIniData*/CheckAndReadIniData*/ReadWriteIni*/MyForceDirectories/
                                 //   GetLastOpenFN/DataPath/AuthPath/sSaveByMachine/asTrayStepSpeedByMachinePatch
#include "csystem.h"             // ShuttleHasIC()/IndexHasIC()
#include "aHotPlateSubstrate.h"  // InArmSuck/OutArmSuck (TMyKitSuck, 177-TU shape)
#include "cAuthority.h"          // authMainForm[]/bAuthCriticalPara[]
#include "forms/fSecurity.h"     // fSecurity/TfSecurity::Insufficient
// AI(W906-FW-SPEED-W21) 20260826: fQwertyKey/TfQwertyKey::ShowQwertyKey -- the
//   27 MouseDown keypad launchers appended at the end of this file.  Landed
//   20260824 by wave FW-QWKEY1 (globals DEFINED at forms/fQwertyKey.cpp:41-42,
//   ShowQwertyKey body ACTIVE at :166), which is what retired Wave A's
//   "fQwertyKey has no port" blocker.  Link edge sm->forms is DECLARED at
//   CMakeLists.txt:2237-2248, not left to lazy archive extraction.
#include "forms/fQwertyKey.h"    // fQwertyKey, TfQwertyKey::ShowQwertyKey

//---------------------------------------------------------------------------
TfSpeed::TfSpeed()
{
    LastFileName="";
    tbAllSpeed->Enabled=false;
    spbSpeedAdd->Enabled=false;
    spbSpeedDec->Enabled=false;
#if 0
    // GATE (S1): golden :38 -- `Hint` has no vclcompat port (grepped this
    // wave: 0 hits inside namespace vclcompat). Its only consumer,
    // DoSetRPDefault/DoReplyDefaultToForm/SearchRecipeParameter's ->Hint scan,
    // is itself GATE (S4) (forms/fSpeed.h) -- doubly inert. See banner.
    edtHPVacuumDelay->Hint="edtHPVacuumDelay";                                  //Sam 20201209 : Default Recipe ChangeLog
#endif // GATE (S1): Hint
}
//---------------------------------------------------------------------------
void TfSpeed::FormShow()
{
    AnsiString S;
    LastFileName=GetLastOpenFN();
    ReadFile();
    DoIniDataToForm();
    S.sprintf("Speed Condition  ''%s''  ",LastFileName);
    Caption=S;
#if 0
    // GATE (S2): golden :49 -- vclcompat::TPageControl only carries
    // `ActivePageIndex` (int), no pointer-valued `ActivePage`. Pure
    // "which tab shows first" nicety, zero logic effect. See banner.
    PageControl1->ActivePage=tsAllSpeed;
#endif // GATE (S2): ActivePage
    fShow=true;
    Left=125;                                                                   //Steven 20091103
    Top=10;                                                                     //Steven 20091103

    gbIndexRetry->Visible   =false;
    gbAutoSkip->Visible     =IniConfig.bRecordSkipPosition;                     //Steven 20140816 : rgInArmAutoSpeed --> gbAutoSkip
    edIndexVacumCheckTime->Enabled=true;                                        //JerryYang 20220311 : ATP鎖定Critical parameter
    edIndexAirOnTime->Enabled=true;
    edIndexSpeed->Enabled   =true;
    gbInArmCylinderDelay->Visible =(InOutArmPickerUseMotor!=eptUseMot);
    gbOutArmCylinderDelay->Visible=(InOutArmPickerUseMotor!=eptUseMot);

    edIndexRetryCount->Text =0;
    edIndexArmRetryMM->Text =0;
    edIndexAccDec->Text     =100;                                               //Steven 20091214 Add for Index ACC & DEC must be 100%

    palInRotate->Visible    =(USE_ROTATE_KIT && iRotate_Type!=eCynRotate);      //Steven 20170425 (wei) : Add rotate motor
    palOutRotate->Visible   =(USE_ROTATE_KIT && iRotate_Type!=eCynRotate);
    cbInRotate->Visible     =(USE_ROTATE_KIT && iRotate_Type!=eCynRotate);
    cbOutRotate->Visible    =(USE_ROTATE_KIT && iRotate_Type!=eCynRotate);
    tsMagazine->Visible     =(AUTO3_IS_MAGAZINE==1);                            //JerryYang 20220909 : add magazine
    //jou 2010-05-19 start : 負壓
    if(INDEX_SUCKER_TYPE==0)
    {
        grpIndexPreSuck->Visible=true;
    }
    else
    {
        grpIndexPreSuck->Visible=false;
        chkIndexPreSuck->Checked=true;
    }
    //jou 2010-05-19 end

    rgAutoSpeed->Visible    =CosFunction.bEnableAutoSpeed;                      //Steven 20110526
    rgStepShuttle->Visible  =IniConfig.bEnableStepShuttle;                      //jou 2013-07-16 Step Shuttle check Index -> Input
    rgShakeShuttle->Visible =CosFunction.bShakeShuttleEveryTime;                //Steven 20200616 : ATK要求每次放料都要搖搖蝦頭
    rgInShtOrder->Visible   =(TestIF_File.iTestMode==QualSite1X4 ||             //Steven 20250630 : 力旺要照順序擺
                              TestIF_File.iTestMode>=_8Site2X4);

    if(CosFunction.bLimitMaxSpeed==true)                                        //Steven 20131101 : 限制最高速度
    {
        tbAllSpeed->Max     =CosFunction.iLimitMaxSpeed;
        udInXSpd->Max       =CosFunction.iLimitMaxSpeed;
        udInZSpd->Max       =CosFunction.iLimitMaxSpeed;
        udInPitchSpd->Max   =CosFunction.iLimitMaxSpeed;
        udOutXSpd->Max      =CosFunction.iLimitMaxSpeed;
        udOutZSpd->Max      =CosFunction.iLimitMaxSpeed;
        udOutPitchSpd->Max  =CosFunction.iLimitMaxSpeed;
        udTrayXSpd->Max     =CosFunction.iLimitMaxSpeed;
        udIndexSpd->Max     =CosFunction.iLimitMaxSpeed;
        udSht1Spd->Max      =CosFunction.iLimitMaxSpeed;
        udSht2Spd->Max      =CosFunction.iLimitMaxSpeed;
        udInRotSpd->Max     =CosFunction.iLimitMaxSpeed;
        udOutRotSpd->Max    =CosFunction.iLimitMaxSpeed;

        tbAccSpeed->Max     =CosFunction.iLimitMaxSpeed;
        udInXAcc->Max       =CosFunction.iLimitMaxSpeed;
        udInZSpd->Max       =CosFunction.iLimitMaxSpeed;
        udInPitchAcc->Max   =CosFunction.iLimitMaxSpeed;
        udOutXSpd->Max      =CosFunction.iLimitMaxSpeed;
        udOutZAcc->Max      =CosFunction.iLimitMaxSpeed;
        udOutPitchAcc->Max  =CosFunction.iLimitMaxSpeed;
        udIndexAcc->Max     =CosFunction.iLimitMaxSpeed;
        udSht1Acc->Max      =CosFunction.iLimitMaxSpeed;
        udSht2Acc->Max      =CosFunction.iLimitMaxSpeed;
        udTrayXAcc->Max     =CosFunction.iLimitMaxSpeed;
        udInRotAcc->Max     =CosFunction.iLimitMaxSpeed;
        udOutRotAcc->Max    =CosFunction.iLimitMaxSpeed;
        tbEPControl->Max    =CosFunction.iLimitMaxSpeed;                        //Ifor 20150710 :限制最高速度
        udLoaderSpeed1->Max =CosFunction.iLimitMaxSpeed;
        udEmptySpeed1->Max  =CosFunction.iLimitMaxSpeed;
        udColorSpeed1->Max  =CosFunction.iLimitMaxSpeed;
        udAuto1Speed1->Max  =CosFunction.iLimitMaxSpeed;
        udAuto2Speed1->Max  =CosFunction.iLimitMaxSpeed;
        udAuto3Speed1->Max  =CosFunction.iLimitMaxSpeed;
        udAuto4Speed1->Max  =CosFunction.iLimitMaxSpeed;                        //Steven 20230907 : For HT-9011UC
        udAuto5Speed1->Max  =CosFunction.iLimitMaxSpeed;
        udAuto6Speed1->Max  =CosFunction.iLimitMaxSpeed;
    }

    grpHPVacuumDelay->Visible=IniConfig.bE57HPCanUseAnotherVacuumDelay;         //Steven 20180125 (Jou) : 加熱盤的真空等待時間
    rgInArmPitch->Enabled   =true;
    rgOutArmPitch->Enabled  =true;
    if(TestIF_File.iTestMode==QualSite1X4 ||
       TestIF_File.iTestMode>=_8Site2X4)                                        //Steven 20140212 : For Xilinx 31x31生產8Site
    {
        if(DeviceForm.XDimension>=ArmMaxPitch_13Suck &&
           DeviceForm.XDimension<ArmMaxPitch)                                   //Steven 20140212 : IC大於25mm時,一次吸四顆只能使用Fix Mode
        {
            rgInArmPitch->Enabled =false;
            rgOutArmPitch->Enabled=false;
        }
    }

    if(IniConfig.bRecordSkipPosition)                                           //wei 20150320  Onecycle 後才能開關Auto Skip 避免吸嘴錯誤hang up
    {
        if(InArmSuck.HasIC() || OutArmSuck.HasIC() ||
           ShuttleHasIC()    || IndexHasIC())
        {
            rgInArmAutoSpeed->Enabled=false;
        }
        else
        {
            rgInArmAutoSpeed->Enabled=true;
        }
    }
    gbOutPickErrAct->Visible=(CUSTOMER_CODE==CC_AMKOR_Japan);                   //Steven 20170202 : For ATJ Out Arm pickup error put to define tray

    if(CUSTOMER_CODE==CC_ASE_CL)                                                //JerryYang 20170303 (wei) ASE_CL 權限控制
    {
        rgAutoSpeed->Enabled          =fSecurity->Insufficient(133, false);
        gbIndexWaitTime->Enabled      =fSecurity->Insufficient(134, false);
        gbIndexDestoryTime->Enabled   =fSecurity->Insufficient(135, false);
        //Input Arm
        gbInVacuumOnFunc->Enabled     =fSecurity->Insufficient(136, false);
        gbInArmReleaseDelay->Enabled  =fSecurity->Insufficient(137, false);
        gbInArmRetry->Enabled         =fSecurity->Insufficient(138, false);
        gbInArmWaitTime->Enabled      =fSecurity->Insufficient(138, false);
        gbInArmDestoryTime->Enabled   =fSecurity->Insufficient(139, false);
        gbInDestoryCheck->Enabled     =fSecurity->Insufficient(139, false);
        gbTwoSpeed_In->Enabled        =fSecurity->Insufficient(140, false);
        //Output Arm
        gbOutVacuumOnFunc->Enabled    =fSecurity->Insufficient(141, false);
        gbOutArmRetry->Enabled        =fSecurity->Insufficient(142, false);
        gbOutArmWaitTime ->Enabled    =fSecurity->Insufficient(142, false);
        gbOutArmDestoryTime->Enabled  =fSecurity->Insufficient(143, false);
        gbOutDestoryCheck->Enabled    =fSecurity->Insufficient(144, false);
        gbTwoSpeed_Out->Enabled       =fSecurity->Insufficient(145, false);
        gbTrayArmWaitTime->Enabled    =fSecurity->Insufficient(146, false);
    }
    else                                                                        //jou 981207 權限控制
    {
        tsAllSpeed->Enabled =(fSecurity->Insufficient(3, false) && authMainForm[3]);
        tsInArm->Enabled    =(fSecurity->Insufficient(3, false) && authMainForm[3]);
        tsShuttle->Enabled  =(fSecurity->Insufficient(3, false) && authMainForm[3]);
        tsIndex->Enabled    =(fSecurity->Insufficient(3, false) && authMainForm[3]);
        tsOutArm->Enabled   =(fSecurity->Insufficient(3, false) && authMainForm[3]);
        tsTrayArm->Enabled  =(fSecurity->Insufficient(3, false) && authMainForm[3]);
        tsLoader->Enabled   =(fSecurity->Insufficient(3, false) && authMainForm[3]);
    }
    ckb_HeightCheck_In->Visible         =(CUSTOMER_CODE==CC_TSMC_TAINAN);       //wei 20160825 回吸偵測
    Edt_HeightCheck_In->Visible         =(CUSTOMER_CODE==CC_TSMC_TAINAN);       //wei 20160825 回吸偵測
    gbInArmReleaseDelay->Visible        =(CUSTOMER_CODE==CC_TSMC_TAINAN);       //JerryYang 20170406 (Steven) 只有TSMC才使用input arm release IC前delay
    gbInarmDeviveConfirm->Visible       =(CUSTOMER_CODE==CC_ASE_KaohSiung);
    gbOutarmDeviveConfirm->Visible      =(CUSTOMER_CODE==CC_ASE_KaohSiung);
    gbIndexeviveConfirm->Visible        =(CUSTOMER_CODE==CC_ASE_KaohSiung);     //kevin 20190629 add index 4 site conform
    grpIndexCycleTimeMonitoring->Visible=(CUSTOMER_CODE==CC_SCK ||              //Isaac 20180301 (Steven) Index Cycle Time Monitoring function，SCK功能開放
                                          IniConfig.bSPILFunction==true);       //JerryYang 20220824 : SPIL index cycle time monitor

    if(IniConfig.bSPILFunction==true)                                           //JerryYang 20220923 : Index cycle time監控
    {
        chkEnableIndexCycleTimeMonitoring->Enabled=(AccessLevel>=iDefHonPrecLevel);
        edtIndexCycleTimetolerance->Enabled=(AccessLevel>=iDefHonPrecLevel);
    }
//    grpIndexCycleTimeMonitoring->Visible=(!IniConfig.bSPILFunction);          //JerryYang 20220824 : SPIL index cycle time monitor

    rgSearchLastModeForYPitch->Visible=(CosFunction.bYPitchNotUseSearchLastMode==true && USE_OUT_Y_IS_AUTO_PITCH==true);                                        //JerryYang 20181112 (Steven) : ATP要求修改unloader擺放方式，盡量不要有空格  //JerryYang 20251218 : IN/OUT ARM支援不同模組
    rgSearchInarmLastModeForYPitch->Visible=(CosFunction.bYPitchNotUseSearchLastMode==true && USE_IN_Y_IS_AUTO_PITCH==true);                                    //kevin 20211130 for in arm Ypitch  //JerryYang 20181112 (Steven) : ATP要求修改unloader擺放方式，盡量不要有空格
#if 0
    // GATE (S3): golden :208 -- vclcompat::TControl carries no SetFocus().   //AI(W906-POOL2-S3) 20261008 (RogerYang): STALE -- SetFocus() exists since 0824 (vclcompat/Controls.h:279, offline no-op); gate kept: TfSpeed::FormShow has no caller, the web Speed page runs FileRW/ArmSpeed_File.gen.inc:1200 (same line gated there, focus is HTML-side). Golden 0618 cSpeed.cpp:208
    // Pure UI-focus nicety, zero logic effect. See banner.
    rbTemp->SetFocus();
#endif // GATE (S3): SetFocus

    //Steven 20200529 : Loader入Tray改步進
    //==>
    tsLoader->TabVisible=(LoaderUnload_StepMotor ||
                          LOAD_Z_USE_MOTOR[0] || LOAD_Z_USE_MOTOR[1] || LOAD_Z_USE_MOTOR[2] ||
                          LOAD_Z_USE_MOTOR[3] || LOAD_Z_USE_MOTOR[4] || LOAD_Z_USE_MOTOR[5] ||
                          LOAD_Z_USE_MOTOR[6] || LOAD_Z_USE_MOTOR[7] || LOAD_Z_USE_MOTOR[8]);

    if(tsLoader->TabVisible)                                                    //Steven 20200724 : 修改Tray Z馬達速度設定
    {
        pnlLoaderY->Visible=LoaderUnload_StepMotor;
        pnlpnlLoaderZ->Visible=LOAD_Z_USE_MOTOR[0];
        pnEmptyY->Visible =LoaderUnload_StepMotor;
        pnEmptyZ->Visible =(LOAD_Z_USE_MOTOR[1] && AUTO_EMPTY_COLOR!=0);
        pnlColorY->Visible=LoaderUnload_StepMotor;
        pnlColorZ->Visible=(LOAD_Z_USE_MOTOR[2] && AUTO_EMPTY_COLOR!=0);
        pnlAuto1Y->Visible=LoaderUnload_StepMotor;
        pnlAuto1Z->Visible=LOAD_Z_USE_MOTOR[3];
        pnlAuto2Y->Visible=LoaderUnload_StepMotor;
        pnlAuto2Z->Visible=LOAD_Z_USE_MOTOR[4];
        pnlAuto3Y->Visible=LoaderUnload_StepMotor;
        pnlAuto3Z->Visible=LOAD_Z_USE_MOTOR[5];

        grpAuto4->Visible =((LoaderUnload_StepMotor && AUTO_EMPTY_COLOR>=3) || LOAD_Z_USE_MOTOR[6]);
        pnlAuto4Y->Visible= (LoaderUnload_StepMotor && AUTO_EMPTY_COLOR>=3);    //Steven 20230907 : For HT-9011UC
        pnlAuto4Z->Visible=LOAD_Z_USE_MOTOR[6];
        grpAuto5->Visible =((LoaderUnload_StepMotor && AUTO_EMPTY_COLOR>=3) || LOAD_Z_USE_MOTOR[7]);
        pnlAuto5Y->Visible=(LoaderUnload_StepMotor && AUTO_EMPTY_COLOR>=3);
        pnlAuto5Z->Visible=LOAD_Z_USE_MOTOR[7];
        grpAuto6->Visible =((LoaderUnload_StepMotor && AUTO_EMPTY_COLOR>=4) || LOAD_Z_USE_MOTOR[8]);
        pnlAuto6Y->Visible=(LoaderUnload_StepMotor && AUTO_EMPTY_COLOR>=4);
        pnlAuto6Z->Visible=LOAD_Z_USE_MOTOR[8];

        if(IniConfig.bSPILFunction)                                             //JerryYang 20230214 : SPIL佶融 要求鎖定75%不能改
        {
            pnlLoaderY->Enabled=false;
            pnEmptyY->Enabled=false;
            pnlColorY->Enabled=false;
            pnlAuto1Y->Enabled=false;
            pnlAuto2Y->Enabled=false;
            pnlAuto3Y->Enabled=false;
            pnlAuto4Y->Enabled=false;
            pnlAuto5Y->Enabled=false;
            pnlAuto6Y->Enabled=false;
        }
    }

    grpEmpty->Visible=(AUTO_EMPTY_COLOR!=0);
    grpColor->Visible=(AUTO_EMPTY_COLOR!=0);
    //<==
    //Steven 20200529 : Loader入Tray改步進

    lblTrayStepSpeedByMachine->Visible=(LoaderUnload_StepMotor &&
                                        IniConfig.bP40TrayYSpeedByMachine);     //Sam 20201221 : Tray y step motor by machine
    #ifdef DEBUG_INDEX_UPH                                                      //Steven 20170818 : 測試UPH用
        grpIndexUPHTryRun->Visible=true;
    #else
        grpIndexUPHTryRun->Visible=false;
    #endif

    if(CUSTOMER_CODE==CC_ASE_CL)                                                //JerryYang 20250120 : add
    {
        if(AccessLevel<iDefHonPrecLevel)
        {
            grpLoader->Visible=false;
            grpEmpty->Visible=false;
            grpColor->Visible=false;
            grpAuto1->Visible=false;
            grpAuto2->Visible=false;
            grpAuto3->Visible=false;
        }
        else
        {
            grpLoader->Visible=true;
            grpEmpty->Visible=true;
            grpColor->Visible=true;
            grpAuto1->Visible=true;
            grpAuto2->Visible=true;
            grpAuto3->Visible=true;
        }
        grpLoader->Enabled=false;
        grpEmpty->Enabled=false;
        grpColor->Enabled=false;
        grpAuto1->Enabled=false;
        grpAuto2->Enabled=false;
        grpAuto3->Enabled=false;
    }

    edtLoaderSpeed2->Visible=(CUSTOMER_CODE==CC_ASE_KaohSiung);                 //kevin 20201225 2段速設定
    gbInArmDieClean->Visible=(USE_DIE_CLEAN==1);                                //wei 20170418
    gbOutArmDieClean->Visible=false;                                            //wei 20170418
    if(CosFunction.bOutArmPickUpErrBySetup)                                     //JerryYang 20220923 : E50改成選項
    {
        rgOutArmPickupErrMode->Visible=true;
        if(AccessLevel==iDefHonPrecLevel)
        {
            rgOutArmPickupErrMode->Enabled=true;
        }
        else
        {
            rgOutArmPickupErrMode->Enabled=false;
        }
    }
    else
    {
        rgOutArmPickupErrMode->Visible=false;
    }

    if(CosFunction.bLotStartLockCriticalPara && RunInfo.bLotStart)              //JerryYang 20220311 : ATP鎖定Critical parameter
    {
        if(bAuthCriticalPara[6])
        {
            edIndexVacumCheckTime->Enabled=false;
            edIndexAirOnTime->Enabled=false;
        }

        if(bAuthCriticalPara[11])
        {
            edIndexSpeed->Enabled=false;
        }
    }

    if(CUSTOMER_CODE==CC_QUALCOMM || IniConfig.bSPILFunction)                   //JerryYang 20260415 : add
    {
        gbEnableDelayTimeZero->Visible=false;
    }

    //這一行請保持在最下面!!-----------------
//    myLog.Do_Log(Sender, asUser, asLogPath);                                  //Steven 20100629
}
//---------------------------------------------------------------------------
void TfSpeed::ReadFile()
{
    AnsiString szDir="";
    LastFileName=GetLastOpenFN();
    if(CosFunction.bSaveArmSpeedByMachine && IniConfig.bA57_1SaveArmSpeedByMachine)                                     //JimmyChiu 20220618 : save by machine
    {
        szDir=sSaveByMachine;
    }
    else
    {
        szDir.sprintf("%s%s", DataPath, LastFileName);
    }
    MyForceDirectories(szDir);
    szDir+="\\ArmCondition.Data";
    tbAllSpeed->Position                    =ReadIniData(szDir, "All", "Speed", 1);
    tbAccSpeed->Position                    =ReadIniData(szDir, "All", "Accel", 1);
    edAllSpeed->Text                        =tbAllSpeed->Position;
    edAllAccSpeed->Text                     =tbAccSpeed->Position;

    if(USE_CKD_FCM_CleanAir)                                                    //Ifor 20150710 : EP Speed Control
    {
        gbEPControl->Visible                =true;                              //Ifor 20150710 :安裝CKD 才顯示
        tbEPControl->Position               =ReadIniData(szDir, "All", "EPControl", 1);
        edEPControl->Text                   =tbEPControl->Position;
        iEPControlValue                     =atoi(edEPControl->Text.c_str());
    }
    else
    {
        gbEPControl->Visible                =false;                             //Ifor 20150710 :安裝CKD 才顯示
    }
    //--------------------------------------------------------------------------
    //Index Arm
    //--------------------------------------------------------------------------
    ArmSpeed_File[IndexArm].iBodySP         =ReadIniData(szDir, "Index Arm", "X Speed",                  1);
    ArmSpeed_File[IndexArm].iACDCBodySP     =100;                               //ReadIniData(szDir, "Index Arm", "X Accel",                100);
    if(CosFunction.bLimitMaxSpeed==true)                                        //Steven 20131101 : 限制最高速度
    {
        ArmSpeed_File[IndexArm].iBodySP     =CheckRange(ArmSpeed_File[IndexArm].iBodySP, CosFunction.iLimitMaxSpeed, 1);
    }

    if(CUSTOMER_CODE==CC_ASE_SG)                                                //Ifor 20240913 ASE 新加坡客戶要求 X => Z
    {
        ArmSpeed_File[IndexArm].iBodySP     =CheckAndReadIniData(szDir, "Index Arm", "Y Speed",   ArmSpeed_File[IndexArm].iBodySP);
        ArmSpeed_File[IndexArm].iACDCBodySP =CheckAndReadIniData(szDir, "Index Arm", "Y Accel",   ArmSpeed_File[IndexArm].iACDCBodySP);
    }
    ArmSpeed_File[IndexArm].iRetryCT        =ReadIniData(szDir, "Index Arm", "Retry Count",              0);
    ArmSpeed_File[IndexArm].iRetryCT        =0;
    ArmSpeed_File[IndexArm].dRetryDown      =ReadIniData(szDir, "Index Arm", "Retry Down",             0.0);
    ArmSpeed_File[IndexArm].dVacuumTI       =ReadIniData(szDir, "Index Arm", "Vacuum Check Time",      1.0);
    ArmSpeed_File[IndexArm].dCTAirOn        =ReadIniData(szDir, "Index Arm", "Counter Air ON Time",    1.0);
    //ChungHung 20130413 add 針對 阿凡達IC 無法Relase
    ArmSpeed_File[IndexArm].dDestroyAgainTime=ReadIniData(szDir, "Index Arm", "Destroy Again Time", 0.0);
    ArmSpeed_File[IndexArm].iDestroyAgainCount=ReadIniData(szDir, "Index Arm", "Destroy Again Count", 1);

    if(IniConfig.bD32_35TrayPitchIndexDOnMoreThen500MS)                         //Steven 20120727 : Tray Pitch 大於35mm的話, Index吹氣至少要0.5秒
    {
        if(UserDefForm_File[TrayForm.Loader.iTrayType].XDivision==1 ||          //Steven 20241205 : 修正只有一排的會失效
           UserDefForm_File[TrayForm.Loader.iTrayType].XPitch*100>=3500)        //JerryYang 20181228 fix E41功能失效   //Steven 20140221
        {
            ArmSpeed_File[IndexArm].dCTAirOn=CheckRange(ArmSpeed_File[IndexArm].dCTAirOn, 0.5, 10.0);
        }
    }

    ArmSpeed_File[IndexArm].bIndexFloatCHK  =ReadIniData(szDir, "Index Arm", "Socket Check",             0);
    ArmSpeed_File[IndexArm].bSuckOnDown     =ReadIniData(szDir, "Index Arm", "Vacuum Timing",            0);

    if(CUSTOMER_CODE==CC_SINOICTECH || CUSTOMER_CODE==CC_FMSH)                  //Steven 20221207 : 華嶺預設是打開
        TestIF_File.bIndexPickICWhenOutShtNoIC  =ReadIniData(szDir, "Index Arm", "Pick IC when out shuttle no IC", 1);  //Steven 20221207 : Index必須在out shuttle沒料才可以吸
    else
        TestIF_File.bIndexPickICWhenOutShtNoIC  =ReadIniData(szDir, "Index Arm", "Pick IC when out shuttle no IC", 0);

    if(CUSTOMER_CODE==CC_QUALCOMM || IniConfig.bSPILFunction)                                                          //JerryYang 20250826 : QCT不使用 //JerryYang 20260415 : add
    {
        TestIF_File.bEnableDelayTimeZero=false;
    }
    else
    {
        TestIF_File.bEnableDelayTimeZero=(ReadIniData(szDir, "Index Arm", "Enable Delay Time Zero", 0)==1)?true:false;  //Jimmychiu 20230922 : R230824-ATK-H9-01 Request add "Air purge during place device on out-shuttle" function.
    }

    if(CUSTOMER_CODE==CC_ASE_KaohSiung)
        ArmSpeed_File[IndexArm].bDevicConfirm=ReadIniData(szDir, "Index Arm", "DevicConfirm", true);                    //kevin 20190629 add index 4 site conform
    else
        ArmSpeed_File[IndexArm].bDevicConfirm=false;

    if(CUSTOMER_CODE==CC_ASE_KaohSiung && IniConfig.bD44CheckIndexICDestroy && ArmSpeed_File[IndexArm].bDevicConfirm)   //kevin 20190629 add 高雄強制4個 SITE INDEX check
    {
        IniConfig.bD44CheckIndexICDestroy=false;
    }
    //Isaac 20180301 (Steven) Index Cycle Time Monitoring function，SCK功能開放
    //==>
    if(CUSTOMER_CODE==CC_SCK || IniConfig.bSPILFunction)                        //JerryYang 20220824 : SPIL index cycle time monitor
    {
        if(IniConfig.bSPILFunction)
        {
            TestIF_File.bIndexCycleTimeMonitor  =ReadIniData(szDir, "Index Arm", "EnableNewIndexCycleTimeMonitoring", false);                                   //JerryYang 20230131 : 預設先不開啟
        }
        else
        {
            TestIF_File.bIndexCycleTimeMonitor  =ReadIniData(szDir, "Index Arm", "EnableIndexCycleTimeMonitoring", true);
        }

        TestIF_File.dIndexCycletimeMonitor      =ReadIniData(szDir, "Index Arm", "Monitoring_IndexCycletime", 0.0);
        TestIF_File.dMonitorOutlier             =ReadIniData(szDir, "Index Arm", "Monitoring_Outlier", 0.0);
        TestIF_File.iMonitorWindow              =ReadIniData(szDir, "Index Arm", "Monitoring_Window", 0);
        TestIF_File.dICTTolerance               =ReadIniData(szDir, "Index Arm", "IndexCycleTimetolerance", 15);
        TestIF_File.iICTAction                  =ReadIniData(szDir, "Index Arm", "IndexCycleTimeAction", 0);            //Steven 20250521 : 可以選擇要不要停機
    }
    else
    {
        TestIF_File.bIndexCycleTimeMonitor      =false;
    }
    //<==
    //Isaac 20180301 (Steven) Index Cycle Time Monitoring function，SCK功能開放
    if(CosFunction.bOutArmPickUpErrBySetup==true)                               //JerryYang 20220923 : E50改成選項
    {
        IniConfig.iE50_OutArmPickUpErrorOption=ReadIniData(szDir, "Output Arm", "iE50_OutArmPickUpErrorOption", 0);
    }

    //--------------------------------------------------------------------------
    //InPut Arm
    //--------------------------------------------------------------------------
    ArmSpeed_File[InArm].iBodySP            =ReadIniData(szDir, "Input Arm", "XY Speed",                 1);
    if(IniConfig.bE41_35TrayPitchInOutSpeedSmall80Percent)                      //Steven 20120727 : Tray Pitch 大於35mm的話, In Out Arm需要小於80%
    {
        if(UserDefForm_File[TrayForm.Loader.iTrayType].XDivision==1 ||          //Steven 20241205 : 修正只有一排的會失效
           UserDefForm_File[TrayForm.Loader.iTrayType].XPitch*100>=3500)        //JerryYang 20181228 fix E41功能失效    //Steven 20140221
        {
            ArmSpeed_File[InArm].iBodySP    =CheckRange(ArmSpeed_File[InArm].iBodySP, 80, 1);
        }
    }

    if(CosFunction.bLimitMaxSpeed==true)                                        //Steven 20131101 : 限制最高速度
    {
        ArmSpeed_File[InArm].iBodySP        =CheckRange(ArmSpeed_File[InArm].iBodySP, CosFunction.iLimitMaxSpeed, 1);
    }

    AutoArmSpeed[InArm].iBodySP             =ArmSpeed_File[InArm].iBodySP;      //jou 2012-05-16 Auto Speed
    ArmSpeed_File[InArm].iACDCBodySP        =ReadIniData(szDir, "Input Arm", "XY Accel",                 1);
    AutoArmSpeed[InArm].iACDCBodySP         =ArmSpeed_File[InArm].iACDCBodySP;  //jou 2012-05-16 Auto Speed
    ArmSpeed_File[InArm].iZSP               =ReadIniData(szDir, "Input Arm", "Z Up Speed",               1);
    ArmSpeed_File[InArm].iACDCZSP           =ReadIniData(szDir, "Input Arm", "Z Up Accel",               1);
    ArmSpeed_File[InArm].bAutoSpeed         =ReadIniData(szDir, "Input Arm", "Auto Speed Down",          0);
    ArmSpeed_File[InArm].iAutoSpeedLow      =ReadIniData(szDir, "Input Arm", "AutoSpeedLow",            50);            //kevin 20210913 AutoSpeed 最低速
    if(CosFunction.bEnableAutoSpeed==false)                                     //Steven 20110526
        ArmSpeed_File[InArm].bAutoSpeed     =false;
    ArmSpeed_File[InArm].iRetryCT           =ReadIniData(szDir, "Input Arm", "Retry Count",              1);
    ArmSpeed_File[InArm].dRetryDown         =ReadIniData(szDir, "Input Arm", "Retry Down",             1.0);
    ArmSpeed_File[InArm].dVacuumTI          =ReadIniData(szDir, "Input Arm", "Vacuum Check Time",      1.0);
    ArmSpeed_File[InArm].dHPVacuumTI        =ReadIniData(szDir, "Input Arm", "HP Vacuum Check Time",   1.0);            //Steven 20180125 (Jou) : 加熱盤的真空等待時間
    ArmSpeed_File[InArm].bUseHPVacuum       =ReadIniData(szDir, "Input Arm", "Use HP Vacuum Check Time",  false);       //Steven 20180125 (Jou) : 加熱盤的真空等待時間
    if(IniConfig.bE57HPCanUseAnotherVacuumDelay==false)
        ArmSpeed_File[InArm].bUseHPVacuum=false;
    ArmSpeed_File[InArm].dCTAirOn           =ReadIniData(szDir, "Input Arm", "Counter Air ON Time",    1.0);
    ArmSpeed_File[InArm].dWaitOnSH          =ReadIniData(szDir, "Input Arm", "Shuttle Wait Time",      1.0);
    ArmSpeed_File[InArm].bAutoSKIP          =ReadIniData(szDir, "Input Arm", "Auto Skip",                0);
    ArmSpeed_File[InArm].iAutoSkipCT        =ReadIniData(szDir, "Input Arm", "Auto Skip CT",            20);
    ArmSpeed_File[InArm].bVariModeFIX       =ReadIniData(szDir, "Input Arm", "One by one",               0);

    if(IniConfig.bSPILFunction==true && ArmSpeed_File[InArm].bAutoSKIP)         //wei 20150317 新竹矽品 auto skip 強制手動拿Tray  //JerryYang 20170328 (Jou) 矽品客戶碼統一用SPILFunction
    {
        TrayForm.iManualRemoveLoader=1;
    }

    if(TestIF_File.iTestMode==QualSite1X4 ||
       TestIF_File.iTestMode>=_8Site2X4)                                        //Steven 20140212 : For Xilinx 31x31生產8Site
    {
        if(DeviceForm.XDimension>=ArmMaxPitch_13Suck &&
           DeviceForm.XDimension<ArmMaxPitch)                                   //Steven 20140212 : IC大於25mm時,一次吸四顆只能使用Fix Mode
        {
            ArmSpeed_File[InArm].bVariModeFIX=true;
        }

        TestIF_File.iInArmToShtReleaseMode  =ReadIniData(szDir, "Input Arm", "iInArmToShtReleaseMode", 0);              //Steven 20250630 : 力旺要照順序擺
    }
    else
    {
        TestIF_File.iInArmToShtReleaseMode  =0;                                 //Steven 20250630 : 力旺要照順序擺
    }

    if(IniConfig.bI37_EnableFIFOMode && LastSet.iRunStartMode==rsmFIFOMode)     //Steven 20160303 (wei) : FIFO Mode
    {
        ArmSpeed_File[InArm].bVariModeFIX=true;
    }

    ArmSpeed_File[InArm].iTwoSpeedMove     =ReadIniData(szDir, "Input Arm", "Two Speed On Off",    0);                  //Steven 20140217 : 兩段速移動
    ArmSpeed_File[InArm].dTwoSpeedDistance =ReadIniData(szDir, "Input Arm", "Two Speed Distance",  3.0);                //Steven 20140217 : 兩段速移動
    ArmSpeed_File[InArm].iTwoSpeed         =ReadIniData(szDir, "Input Arm", "Two Speed Precent",   10);                 //Steven 20140217 : 兩段速移動
    ArmSpeed_File[InArm].iTwoADC           =ReadIniData(szDir, "Input Arm", "Two Speed ADC",       10);                 //Steven 20140217 : 兩段速移動
    ArmSpeed_File[InArm].bTwoSpeedOnlyLoader=ReadIniData(szDir, "Input Arm", "Two Speed Only Loader", false);           //JerryYang 20190729 二段速功能可選擇only at loader

    ArmSpeed_File[InArm].bSuckOnDown       =ReadIniData(szDir, "Input Arm", "Vacuum Timing",            0);
    ArmSpeed_File[InArm].iVariSP           =ReadIniData(szDir, "Input Arm", "Open/Close Speed",         1);
    ArmSpeed_File[InArm].iACDCVariSP       =ReadIniData(szDir, "Input Arm", "Open/Close Accel",         1);

    //ChungHung 20131231 alter AutoYPitch start
    ArmSpeed_File[InArm].iVariSPY          =ArmSpeed_File[InArm].iVariSP;       //Steven 20140930 : For XY-Pitch
    ArmSpeed_File[InArm].iACDCVariSPY      =ArmSpeed_File[InArm].iACDCVariSP;
    //ChungHung 20131231 alter AutoYPitch end

    //ChungHung 20130413 add 針對 阿凡達IC 無法Relase
    ArmSpeed_File[InArm].dDestroyAgainTime  =ReadIniData(szDir, "Input Arm", "Destroy Again Time", 0.0);
    ArmSpeed_File[InArm].iDestroyAgainCount =ReadIniData(szDir, "Input Arm", "Destroy Again Count", 1);

    //2013-08-01    Dell    modify  公司統一回吸功能
    ArmSpeed_File[InArm].dDestroyCheckTime  =ReadIniData(szDir, "Input Arm", "Destroy Check Time", 0.1);
    ArmSpeed_File[InArm].bDestroyPauseCheck =ReadIniData(szDir, "Input Arm", "Destroy Pause Check", 0);

    ArmSpeed_File[InRotate].iBodySP         =ReadIniData(szDir, "Input Arm", "Rotate Input Speed", 0.1);                //kevin 20130517 rotate add
    ArmSpeed_File[InRotate].iACDCBodySP     =ReadIniData(szDir, "Input Arm", "Rotate Input Accel", 0);

    ArmSpeed_File[InArm].dCylinderDelay     =ReadIniData(szDir, "Input Arm", "Cylinder Delay", 0.1);
    ArmSpeed_File[InArm].PrecisorOpenSp      =ReadIniData(szDir, "Input Arm", "PrecisorOpenSpeed", 1);                  //JerryYang 20250120 : add
    ArmSpeed_File[InArm].PrecisorCloseSp     =ReadIniData(szDir, "Input Arm", "PrecisorCloseSpeed", 1);
    if(CUSTOMER_CODE==CC_TSMC_TAINAN)                                           //JerryYang 20170406 (Steven) 只有TSMC才使用input arm release IC前delay
    {
        ArmSpeed_File[InArm].dReleaseDelayTime  =ReadIniData(szDir, "Input Arm", "Release Delay Time", 0.0);            //JerryYang 20160127 for TSMC inarm release device前delay
        ArmSpeed_File[InArm].iEnableReleaseDelay=ReadIniData(szDir, "Input Arm", "Enable Relase Delay", 1);             //JerryYang 20160127 for TSMC inarm release device前delay
    }
    else
    {
        ArmSpeed_File[InArm].dReleaseDelayTime=0.0;                             //JerryYang 20160127 for TSMC inarm release device前delay
        ArmSpeed_File[InArm].iEnableReleaseDelay=1;                             //JerryYang 20160127 for TSMC inarm release device前delay
    }

    if(USE_DIE_CLEAN)
    {
        ArmSpeed_File[InArm].iEnableDieClean    =ReadIniData(szDir, "Input Arm", "Enable Die Clean",  0);               //wei 20170418
        ArmSpeed_File[InArm].dDieCleanDelay     =ReadIniData(szDir, "Input Arm", "Die Clean Delay",   0.0);             //wei 20170418
        ArmSpeed_File[InArm].dDieCleanHeight    =ReadIniData(szDir, "Input Arm", "Die Clean Height",  0.1);             //wei 20170418
    }
    else
    {
        ArmSpeed_File[InArm].iEnableDieClean    =0;
    }

    ArmSpeed_File[InArm].bHeightCheck  =ReadIniData(szDir, "Input Arm", "Enable Height Check", false);                  //wei 20160825 回吸偵測
    ArmSpeed_File[InArm].iHeightCheck  =ReadIniData(szDir, "Input Arm", "Height Check",         1000);                  //wei 20160825 回吸偵測
    if(CUSTOMER_CODE==CC_ASE_KaohSiung)
        ArmSpeed_File[InArm].bDevicConfirm=ReadIniData(szDir, "Input Arm", "DevicConfirm", true);                       //kevin 20170510 (wei) 回吸偵測開關
    else
        ArmSpeed_File[InArm].bDevicConfirm=false;

    if(CosFunction.bYPitchNotUseSearchLastMode && USE_OUT_Y_IS_AUTO_PITCH==true)                                        //JerryYang 20181112 (Steven) : ATP要求修改unloader擺放方式，盡量不要有空格  //JerryYang 20251218 : IN/OUT ARM支援不同模組
    {
        if(IniConfig.bSPILFunction==true)                                       //JerryYang 20200324 中科志強說預設要false
        {
            ArmSpeed_File[InArm].bYPitchUseSearchLastMode=ReadIniData(szDir, "Input Arm", "bYPitchNotUseSearchLastMode", false);
        }
        else
        {
            ArmSpeed_File[InArm].bYPitchUseSearchLastMode=ReadIniData(szDir, "Input Arm", "bYPitchNotUseSearchLastMode", true);
        }
    }
    else
    {
        ArmSpeed_File[InArm].bYPitchUseSearchLastMode=true;
    }

    ArmSpeed_File[InArm].bTT_SetSpeed         =ReadIniData(szDir, "Input Arm", "Test Time Set Speed",          0);      //Ztex 2024.08.11 Add Test Time Set Speed
    if(ArmSpeed_File[InArm].bTT_SetSpeed==1)                                    //Ztex 2024.08.11 Add Test Time Set Speed
        ArmSpeed_File[InArm].bAutoSpeed=0;
    //--------------------------------------------------------------------------
    //Output Arm
    //--------------------------------------------------------------------------
    ArmSpeed_File[OutArm].iBodySP           =ReadIniData(szDir, "Output Arm", "XY Speed",                1);
    if(IniConfig.bE41_35TrayPitchInOutSpeedSmall80Percent)                      //Steven 20120727 : Tray Pitch 大於35mm的話, In Out Arm需要小於80%
    {
        if(UserDefForm_File[TrayForm.Loader.iTrayType].XDivision==1 ||          //Steven 20241205 : 修正只有一排的會失效
           UserDefForm_File[TrayForm.Loader.iTrayType].XPitch*100>=3500)        //JerryYang 20181228 fix E41功能失效
        {
            ArmSpeed_File[OutArm].iBodySP   =CheckRange(ArmSpeed_File[OutArm].iBodySP, 80, 1);
        }
    }

    if(CosFunction.bLimitMaxSpeed==true)                                        //Steven 20131101 : 限制最高速度
    {
        ArmSpeed_File[OutArm].iBodySP       =CheckRange(ArmSpeed_File[OutArm].iBodySP, CosFunction.iLimitMaxSpeed, 1);
    }

    AutoArmSpeed[OutArm].iBodySP            =ArmSpeed_File[OutArm].iBodySP;     //jou 2012-05-16 Auto Speed
    ArmSpeed_File[OutArm].iACDCBodySP       =ReadIniData(szDir, "Output Arm", "XY Accel",                1);
    AutoArmSpeed[OutArm].iACDCBodySP        =ArmSpeed_File[OutArm].iACDCBodySP;                                         //jou 2012-05-16 Auto Speed
    ArmSpeed_File[OutArm].iZSP              =ReadIniData(szDir, "Output Arm", "Z Up Speed",              1);
    ArmSpeed_File[OutArm].iACDCZSP          =ReadIniData(szDir, "Output Arm", "Z Up Accel",              1);
    ArmSpeed_File[OutArm].iRetryCT          =ReadIniData(szDir, "Output Arm", "Retry Count",             1);
    ArmSpeed_File[OutArm].dRetryDown        =ReadIniData(szDir, "Output Arm", "Retry Down",              1.0);
    ArmSpeed_File[OutArm].dVacuumTI         =ReadIniData(szDir, "Output Arm", "Vacuum Check Time",       1.0);
    ArmSpeed_File[OutArm].dCTAirOn          =ReadIniData(szDir, "Output Arm", "Counter Air ON Time",     1.0);
    ArmSpeed_File[OutArm].dWaitOnSH         =ReadIniData(szDir, "Output Arm", "Shuttle Wait Time",       0.0);          //kevin 20210525 out arm 在 shuttlr 上等待時間到再往下吸取
    ArmSpeed_File[OutArm].bVariModeFIX      =ReadIniData(szDir, "Output Arm", "One by one",              0);

    ArmSpeed_File[OutArm].iAutoSpeedLow     =ReadIniData(szDir, "Input Arm", "AutoSpeedLow",            50);            //kevin 20210913 AutoSpeed 最低速

    if(TestIF_File.iTestMode==QualSite1X4 ||
       TestIF_File.iTestMode>=_8Site2X4)                                        //Steven 20140212 : For Xilinx 31x31生產8Site
    {
        if(DeviceForm.XDimension>=ArmMaxPitch_13Suck &&
           DeviceForm.XDimension<ArmMaxPitch)                                   //Steven 20140212 : IC大於25mm時,一次吸四顆只能使用Fix Mode
        {
            ArmSpeed_File[OutArm].bVariModeFIX=true;
        }
    }

    if(IniConfig.bI37_EnableFIFOMode && LastSet.iRunStartMode==rsmFIFOMode)     //Steven 20160303 (wei) : FIFO Mode
    {
        ArmSpeed_File[OutArm].bVariModeFIX=true;
    }

    ArmSpeed_File[OutArm].bSuckOnDown       =ReadIniData(szDir, "Output Arm", "Vacuum Timing",           0);
    ArmSpeed_File[OutArm].iVariSP           =ReadIniData(szDir, "Output Arm", "Open/Close Speed",        1);
    ArmSpeed_File[OutArm].iACDCVariSP       =ReadIniData(szDir, "Output Arm", "Open/Close Accel",        1);
    ArmSpeed_File[OutArm].dDestroyPauseTime =ReadIniData(szDir, "Output Arm", "Wait before Air On",      0.01);
    ArmSpeed_File[OutArm].iTwoSpeedMove     =ReadIniData(szDir, "Output Arm", "Two Speed On Off",        0);            //Steven 20140217 : 兩段速移動
    ArmSpeed_File[OutArm].dTwoSpeedDistance =ReadIniData(szDir, "Output Arm", "Two Speed Distance",      3.0);          //Steven 20140217 : 兩段速移動
    ArmSpeed_File[OutArm].iTwoSpeed         =ReadIniData(szDir, "Output Arm", "Two Speed Precent",       10);           //Steven 20140217 : 兩段速移動
    ArmSpeed_File[OutArm].iTwoADC           =ReadIniData(szDir, "Output Arm", "Two Speed ADC",           10);           //Steven 20140217 : 兩段速移動

    //ChungHung 20131231 alter AutoYPitch start
    ArmSpeed_File[OutArm].iVariSPY          =ArmSpeed_File[OutArm].iVariSP;     //Steven 20140930 : For XY-Pitch
    ArmSpeed_File[OutArm].iACDCVariSPY      =ArmSpeed_File[OutArm].iACDCVariSP;
    //ChungHung 20131231 alter AutoYPitch end

    //ChungHung 20130413 add 針對 阿凡達IC 無法Relase
    ArmSpeed_File[OutArm].dDestroyAgainTime =ReadIniData(szDir, "Output Arm", "Destroy Again Time", 0.0);
    ArmSpeed_File[OutArm].iDestroyAgainCount=ReadIniData(szDir, "Output Arm", "Destroy Again Count", 1);

    //2013-08-01    Dell    modify  公司統一回吸功能
    ArmSpeed_File[OutArm].dDestroyCheckTime =ReadIniData(szDir, "Output Arm", "Destroy Check Time", 0.1);
    ArmSpeed_File[OutArm].bDestroyPauseCheck=ReadIniData(szDir, "Output Arm", "Destroy Pause Check", 0);

    ArmSpeed_File[OutArm].dCylinderDelay    =ReadIniData(szDir, "Output Arm", "Cylinder Delay", 0.1);
    if(CUSTOMER_CODE==CC_ASE_KaohSiung)
        ArmSpeed_File[OutArm].bDevicConfirm =ReadIniData(szDir, "Output Arm", "DevicConfirm", true);                    //kevin 20170510 (wei) 回吸偵測開關
    else
        ArmSpeed_File[OutArm].bDevicConfirm =false;

    if(CosFunction.bYPitchNotUseSearchLastMode && USE_OUT_Y_IS_AUTO_PITCH==true)                                        //JerryYang 20181112 (Steven) : ATP要求修改unloader擺放方式，盡量不要有空格  //JerryYang 20251218 : IN/OUT ARM支援不同模組
    {
        if(IniConfig.bSPILFunction==true)                                       //JerryYang 20200324 中科志強說預設要false
        {
            ArmSpeed_File[OutArm].bYPitchUseSearchLastMode=ReadIniData(szDir, "Output Arm", "bYPitchNotUseSearchLastMode", false);
        }
        else
        {
            ArmSpeed_File[OutArm].bYPitchUseSearchLastMode=ReadIniData(szDir, "Output Arm", "bYPitchNotUseSearchLastMode", true);
        }
    }
    else
    {
        ArmSpeed_File[OutArm].bYPitchUseSearchLastMode=true;
    }

    if(USE_DIE_CLEAN)
    {
        ArmSpeed_File[OutArm].iEnableDieClean   =ReadIniData(szDir, "Output Arm", "Enable Die Clean",  0);              //wei 20170418
        ArmSpeed_File[OutArm].dDieCleanDelay    =ReadIniData(szDir, "Output Arm", "Die Clean Delay",   0.0);            //wei 20170418
    }
    else
    {
        ArmSpeed_File[OutArm].iEnableDieClean   =0;
    }

    ArmSpeed_File[OutRotate].iBodySP        =ReadIniData(szDir, "Output Arm", "Rotate Output Speed", 0.1);              //Steven 20140520 : Input --> Output
    ArmSpeed_File[OutRotate].iACDCBodySP    =ReadIniData(szDir, "Output Arm", "Rotate Output Accel", 0);                //Steven 20140520 : Input --> Output
    //--------------------------------------------------------------------------
    //Empty Tray Arm
    //--------------------------------------------------------------------------
    ArmSpeed_File[TrayArm].iBodySP          =ReadIniData(szDir, "Empty Tray Arm", "Speed",               1);

    if(CosFunction.bLimitMaxSpeed==true)                                        //Steven 20131101 : 限制最高速度
    {
        ArmSpeed_File[TrayArm].iBodySP      =CheckRange(ArmSpeed_File[TrayArm].iBodySP, CosFunction.iLimitMaxSpeed, 1);
    }

    ArmSpeed_File[TrayArm].iACDCBodySP      =ReadIniData(szDir, "Empty Tray Arm", "Accel",               1);
    ArmSpeed_File[TrayArm].iRetryCT         =ReadIniData(szDir, "Empty Tray Arm", "Retry Count",         1);
    ArmSpeed_File[TrayArm].dVacuumTI        =ReadIniData(szDir, "Empty Tray Arm", "Vacuum Check Time",   1.0);
    ArmSpeed_File[TrayArm].dCTAirOn         =ReadIniData(szDir, "Empty Tray Arm", "Counter Air ON Time", 1.0);
    ArmSpeed_File[TrayArm].dTAHeadDownIT    =ReadIniData(szDir, "Empty Tray Arm", "Hand Down Time",      1.0);
    if(IniConfig.bC03UseCatchTray)                                              //use catch Tray Fix
    {
        ArmSpeed_File[TrayArm].dTAHeadDownIT=CheckRange(ArmSpeed_File[TrayArm].dTAHeadDownIT, 0.3, 10.0);
    }
    //--------------------------------------------------------------------------
    //Shuttle
    //--------------------------------------------------------------------------
    SHSpeed_File.iSH1Sp                    =ReadIniData(szDir, "Shuttle", "Shuttle 1 Speed",    1);
    SHSpeed_File.iShakeCycles              =ReadIniData(szDir, "Shuttle", "Shake Cycles",           1);                 //Sam 20250326 : 新增 Shake 條件設定
    SHSpeed_File.iShakeDistance            =ReadIniData(szDir, "Shuttle", "Shake distance",         5);
    SHSpeed_File.dShakeDelay               =ReadIniData(szDir, "Shuttle", "Delay between shakes",   0.0);
    SHSpeed_File.iShakeAccDec              =ReadIniData(szDir, "Shuttle", "Shake Accel",            100);

    if(CosFunction.bLimitMaxSpeed==true)                                        //Steven 20131101 : 限制最高速度
    {
        SHSpeed_File.iSH1Sp                 =CheckRange(SHSpeed_File.iSH1Sp, CosFunction.iLimitMaxSpeed, 1);
    }

    SHSpeed_File.iSH1ACDCSp                =ReadIniData(szDir, "Shuttle", "Shuttle 1 Accel",    1);
    SHSpeed_File.iSH2Sp                    =ReadIniData(szDir, "Shuttle", "Shuttle 2 Speed",    1);

    if(CosFunction.bLimitMaxSpeed==true)                                        //Steven 20131101 : 限制最高速度
    {
        SHSpeed_File.iSH2Sp                 =CheckRange(SHSpeed_File.iSH2Sp, CosFunction.iLimitMaxSpeed, 1);
    }

    SHSpeed_File.iSH2ACDCSp                 =ReadIniData(szDir, "Shuttle", "Shuttle 2 Accel",    1);
    SHSpeed_File.iStepSH                    =ReadIniData(szDir, "Shuttle", "Step Shuttle",       0);
    TestIF_File.iShakeShuttleWhenPlaceIC    =ReadIniData(szDir, "Shuttle", "Shake Shuttle",      0);                    //Steven 20200616 : ATK要求每次放料都要搖搖蝦頭

    //Steven 20170317 (wei) mark
    //==>
//    if(CUSTOMER_CODE==CC_AMKOR_Japan)                                         //Steven 20170202 : For ATJ Out Arm pickup error put to define tray
//    {
//        TestIF_File.bOutPickErrAct          =ReadIniData(szDir, "Output Arm", "Out Pick Err Action", false);
//        TestIF_File.iOutPickErrAuto         =ReadIniData(szDir, "Output Arm", "Out Pick Err Tray", 6);
//    }
//    else
    //<==
    //Steven 20170317 (wei) mark
    {
        TestIF_File.bOutPickErrAct          =false;
        TestIF_File.iOutPickErrAuto         =6;
    }

    iIndexSpeed=atoi(edtTrySpeed->Text.c_str());                                //Steven 20170818 : 測試UPH用
    iIndexAcc=atoi(edtTryAcc->Text.c_str());

    //--------------------------------------------------------------------------
    //Magazine
    //--------------------------------------------------------------------------
    MGSpeed_File.iCYSp                  =ReadIniData(szDir, "Magazine", "Magazine CatchY Speed",    1);
    MGSpeed_File.iCYACDCSp              =ReadIniData(szDir, "Magazine", "Magazine CatchY Accel",    1);

    MGSpeed_File.iMZSp                  =ReadIniData(szDir, "Magazine", "MagazineZ Speed",    1);
    MGSpeed_File.iMZACDCSp              =ReadIniData(szDir, "Magazine", "MagazineZ Accel",    1);

    MGSpeed_File.dCYDecRange            =ReadIniData(szDir, "Magazine", "Two Speed Distance",  3.0);
    MGSpeed_File.iCY2Sp                 =ReadIniData(szDir, "Magazine", "Two Speed Precent",   10);
    MGSpeed_File.iCY2ACDCSp             =ReadIniData(szDir, "Magazine", "Two Speed ADC",       10);

    ReadWriteFile(true, szDir);
}
//---------------------------------------------------------------------------
void TfSpeed::ReadWriteFile(bool bRead, AnsiString szDir)
{
    // GATE (S6): golden's bRead==false arm is write path (see forms/fSpeed.h
    // banner) -- forced read-only regardless of the caller's argument. The
    // only in-scope caller (ReadFile) already passes true; this is
    // defense-in-depth against a future caller silently reactivating it.
    bRead = true;

    AnsiString szTrayDir="";
    szTrayDir=szDir;
    int idefaultSpeed=0;
    if(IniConfig.bP40TrayYSpeedByMachine)                                       //Sam 20201221 : Tray y step motor by machine
       szTrayDir=asTrayStepSpeedByMachinePatch;

    if(CUSTOMER_CODE==CC_TERAPOWER)                                             //Sam 20210325 : 晶兆成要求預設值為70
        idefaultSpeed=70;
    else
        idefaultSpeed=100;

#if 0
    // GATE (S5): golden :797-879 -- `dmTrayMotor` (golden Motor/
    // TrayStepMotor.h/.cpp, a TDataModule) has no port anywhere in this tree.
    // See forms/fSpeed.h banner. Kept verbatim for when/if a future wave
    // ports TrayStepMotor.
    if(LoaderUnload_StepMotor)                                                  //Steven 20200529 : Loader入Tray改步進
    {
        if(CUSTOMER_CODE==CC_ASE_CL)                                            //JerryYang 20250120 : add
        {
            dmTrayMotor->iStepMotorSpeed[0]=60;
            dmTrayMotor->iStepMotorSpeed[1]=60;
            dmTrayMotor->iStepMotorSpeed[2]=60;
            dmTrayMotor->iStepMotorSpeed[3]=60;
        }
        else if(IniConfig.bSPILFunction)                                        //JerryYang 20230214 : SPIL佶融 要求鎖定75%不能改
        {
            dmTrayMotor->iStepMotorSpeed[0]=75;
            dmTrayMotor->iStepMotorSpeed[1]=75;
            dmTrayMotor->iStepMotorSpeed[2]=75;
            dmTrayMotor->iStepMotorSpeed[3]=75;
            dmTrayMotor->iStepMotorSpeed[0]=75;
            dmTrayMotor->iStepMotorSpeed[6]=75;
            dmTrayMotor->iStepMotorSpeed[7]=75;
            dmTrayMotor->iStepMotorSpeed[8]=75;
        }
        else
        {
            dmTrayMotor->iStepMotorSpeed[0]=ReadWriteIni(szTrayDir, "Tray", "Loader Tray Speed", atoi(edtLoaderSpeed1->Text.c_str()), idefaultSpeed, bRead, true, 1, 100);  //Sam 20201221 : Tray y step motor by machine
            dmTrayMotor->iStepMotorSpeed[1]=ReadWriteIni(szTrayDir, "Tray", "Auto 1 Tray Speed", atoi(edtAuto1Speed1->Text.c_str()),  idefaultSpeed, bRead, true, 1, 100);
            dmTrayMotor->iStepMotorSpeed[2]=ReadWriteIni(szTrayDir, "Tray", "Auto 2 Tray Speed", atoi(edtAuto2Speed1->Text.c_str()),  idefaultSpeed, bRead, true, 1, 100);
            dmTrayMotor->iStepMotorSpeed[3]=ReadWriteIni(szTrayDir, "Tray", "Auto 3 Tray Speed", atoi(edtAuto3Speed1->Text.c_str()),  idefaultSpeed, bRead, true, 1, 100);
            if(AUTO_EMPTY_COLOR>=3)                                             //Steven 20230907 : For HT-9011UC
            {
                dmTrayMotor->iStepMotorSpeed[6]=ReadWriteIni(szTrayDir, "Tray", "Auto 4 Tray Speed", atoi(edtAuto4Speed1->Text.c_str()),  idefaultSpeed, bRead, true, 1, 100);
                dmTrayMotor->iStepMotorSpeed[7]=ReadWriteIni(szTrayDir, "Tray", "Auto 5 Tray Speed", atoi(edtAuto5Speed1->Text.c_str()),  idefaultSpeed, bRead, true, 1, 100);
            }

            if(AUTO_EMPTY_COLOR>=4)                                             //Steven 20230907 : For HT-9011UC
                dmTrayMotor->iStepMotorSpeed[8]=ReadWriteIni(szTrayDir, "Tray", "Auto 6 Tray Speed", atoi(edtAuto6Speed1->Text.c_str()),  idefaultSpeed, bRead, true, 1, 100);
        }

        if(AUTO_EMPTY_COLOR!=0)
        {
            if(CUSTOMER_CODE==CC_ASE_CL)
            {
                dmTrayMotor->iStepMotorSpeed[4]=60;
                dmTrayMotor->iStepMotorSpeed[5]=60;                             //JerryYang 20240930 : 35 -> 60
            }
            else if(IniConfig.bSPILFunction)                                    //JerryYang 20230214 : SPIL佶融 要求鎖定75%不能改
            {
                dmTrayMotor->iStepMotorSpeed[4]=75;
                dmTrayMotor->iStepMotorSpeed[5]=75;
            }
            else
            {
                dmTrayMotor->iStepMotorSpeed[4]=ReadWriteIni(szTrayDir, "Tray", "Empty Tray Speed",      atoi(edtEmptySpeed1->Text.c_str()), idefaultSpeed, bRead, true, 1, 100);
                dmTrayMotor->iStepMotorSpeed[5]=ReadWriteIni(szTrayDir, "Tray", "Color Tray Speed",      atoi(edtColorSpeed1->Text.c_str()), idefaultSpeed, bRead, true, 1, 100);
            }
            dmTrayMotor->iStepMotorSpeed[9]    =ReadWriteIni(szTrayDir, "Tray", "Loader Tray 2nd Speed", atoi(edtLoaderSpeed2->Text.c_str()), 20, bRead, true, 1, 100);
        }

        if(bRead)
        {
            udLoaderSpeed1->Position=dmTrayMotor->iStepMotorSpeed[0];           //Steven 20210625 : 修正軌道速度初始化設定問題
            edtLoaderSpeed1->Text   =dmTrayMotor->iStepMotorSpeed[0];
            udAuto1Speed1->Position =dmTrayMotor->iStepMotorSpeed[1];
            edtAuto1Speed1->Text    =dmTrayMotor->iStepMotorSpeed[1];
            udAuto2Speed1->Position =dmTrayMotor->iStepMotorSpeed[2];
            edtAuto2Speed1->Text    =dmTrayMotor->iStepMotorSpeed[2];
            udAuto3Speed1->Position =dmTrayMotor->iStepMotorSpeed[3];
            edtAuto3Speed1->Text    =dmTrayMotor->iStepMotorSpeed[3];
            udEmptySpeed1->Position =dmTrayMotor->iStepMotorSpeed[4];
            edtEmptySpeed1->Text    =dmTrayMotor->iStepMotorSpeed[4];
            udColorSpeed1->Position =dmTrayMotor->iStepMotorSpeed[5];
            edtColorSpeed1->Text    =dmTrayMotor->iStepMotorSpeed[5];
            udAuto4Speed1->Position =dmTrayMotor->iStepMotorSpeed[6];
            edtAuto4Speed1->Text    =dmTrayMotor->iStepMotorSpeed[6];
            udAuto5Speed1->Position =dmTrayMotor->iStepMotorSpeed[7];
            edtAuto5Speed1->Text    =dmTrayMotor->iStepMotorSpeed[7];
            udAuto6Speed1->Position =dmTrayMotor->iStepMotorSpeed[8];
            edtAuto6Speed1->Text    =dmTrayMotor->iStepMotorSpeed[8];

            edtLoaderSpeed2->Text   =dmTrayMotor->iStepMotorSpeed[9];           //SECOND SPEED   kevin 20201211
        }

        if(CUSTOMER_CODE==CC_ASE_KaohSiung)
            dmTrayMotor->bNeedSetStepMotorSpeed=true;                           //kevin 20210602
    }
#endif // GATE (S5): dmTrayMotor
    (void)idefaultSpeed; (void)szTrayDir;   // GATE (S5) made these write-only while gated

    if(LOAD_Z_USE_MOTOR[0])                                                     //Steven 20200724 : 修改Tray Z馬達速度設定
    {
        TestIF_File.iTrayZStepSpeed[0]=ReadWriteIni(szDir, "Tray", "Loader Tray Z Speed", atoi(edtLoaderSpeedZ->Text.c_str()), 50, bRead, true, 1, 100);
        if(bRead)
        {
            udLoaderSpeedZ->Position=TestIF_File.iTrayZStepSpeed[0];
            edtLoaderSpeedZ->Text=TestIF_File.iTrayZStepSpeed[0];
        }
    }
    else
    {
        TestIF_File.iTrayZStepSpeed[0]=50;
    }

    if(AUTO_EMPTY_COLOR!=0 && LOAD_Z_USE_MOTOR[1])
    {
        TestIF_File.iTrayZStepSpeed[1]=ReadWriteIni(szDir, "Tray", "Empty Z Speed", atoi(edtEmptySpeedZ->Text.c_str()), 50, bRead, true, 1, 100);
        if(bRead)
        {
            udEmptySpeedZ->Position=TestIF_File.iTrayZStepSpeed[1];
            edtEmptySpeedZ->Text=TestIF_File.iTrayZStepSpeed[1];
        }
    }
    else
    {
        TestIF_File.iTrayZStepSpeed[1]=50;
    }

    if(AUTO_EMPTY_COLOR!=0 && LOAD_Z_USE_MOTOR[2])
    {
        TestIF_File.iTrayZStepSpeed[2]=ReadWriteIni(szDir, "Tray", "Color Z Speed", atoi(edtColorSpeedZ->Text.c_str()), 50, bRead, true, 1, 100);
        if(bRead)
        {
            udColorSpeedZ->Position=TestIF_File.iTrayZStepSpeed[2];
            edtColorSpeedZ->Text=TestIF_File.iTrayZStepSpeed[2];
        }
    }
    else
    {
        TestIF_File.iTrayZStepSpeed[2]=50;
    }

    if(LOAD_Z_USE_MOTOR[3])
    {
        TestIF_File.iTrayZStepSpeed[3]=ReadWriteIni(szDir, "Tray", "Auto 1 Z Speed", atoi(edtAuto1SpeedZ->Text.c_str()), 50, bRead, true, 1, 100);
        if(bRead)
        {
            udAuto1SpeedZ->Position=TestIF_File.iTrayZStepSpeed[3];
            edtAuto1SpeedZ->Text=TestIF_File.iTrayZStepSpeed[3];
        }
    }
    else
    {
        TestIF_File.iTrayZStepSpeed[3]=50;
    }

    if(LOAD_Z_USE_MOTOR[4])
    {
        TestIF_File.iTrayZStepSpeed[4]=ReadWriteIni(szDir, "Tray", "Auto 2 Z Speed", atoi(edtAuto2SpeedZ->Text.c_str()), 50, bRead, true, 1, 100);
        if(bRead)
        {
            udAuto2SpeedZ->Position=TestIF_File.iTrayZStepSpeed[4];
            edtAuto2SpeedZ->Text=TestIF_File.iTrayZStepSpeed[4];
        }
    }
    else
    {
        TestIF_File.iTrayZStepSpeed[4]=50;
    }

    if(LOAD_Z_USE_MOTOR[5])
    {
        TestIF_File.iTrayZStepSpeed[5]=ReadWriteIni(szDir, "Tray", "Auto 3 Z Speed", atoi(edtAuto3SpeedZ->Text.c_str()), 50, bRead, true, 1, 100);
        if(bRead)
        {
            udAuto3SpeedZ->Position=TestIF_File.iTrayZStepSpeed[5];
            edtAuto3SpeedZ->Text=TestIF_File.iTrayZStepSpeed[5];
        }
    }
    else
    {
        TestIF_File.iTrayZStepSpeed[5]=50;
    }

    if(LOAD_Z_USE_MOTOR[6])                                                     //Steven 20230907 : For HT-9011UC
    {
        TestIF_File.iTrayZStepSpeed[6]=ReadWriteIni(szDir, "Tray", "Auto 4 Z Speed", atoi(edtAuto4SpeedZ->Text.c_str()), 50, bRead, true, 1, 100);              //RogerYang 20250508 修正
        if(bRead)
        {
            udAuto4SpeedZ->Position=TestIF_File.iTrayZStepSpeed[6];
            edtAuto4SpeedZ->Text=TestIF_File.iTrayZStepSpeed[6];
        }
    }
    else
    {
        TestIF_File.iTrayZStepSpeed[6]=50;
    }

    if(LOAD_Z_USE_MOTOR[7])
    {
        TestIF_File.iTrayZStepSpeed[7]=ReadWriteIni(szDir, "Tray", "Auto 5 Z Speed", atoi(edtAuto5SpeedZ->Text.c_str()), 50, bRead, true, 1, 100);              //RogerYang 20250508 修正
        if(bRead)
        {
            udAuto5SpeedZ->Position=TestIF_File.iTrayZStepSpeed[7];
            edtAuto5SpeedZ->Text=TestIF_File.iTrayZStepSpeed[7];
        }
    }
    else
    {
        TestIF_File.iTrayZStepSpeed[7]=50;
    }

    if(LOAD_Z_USE_MOTOR[8])
    {
        TestIF_File.iTrayZStepSpeed[8]=ReadWriteIni(szDir, "Tray", "Auto 6 Z Speed", atoi(edtAuto6SpeedZ->Text.c_str()), 50, bRead, true, 1, 100);              //RogerYang 20250508 修正
        if(bRead)
        {
            udAuto6SpeedZ->Position=TestIF_File.iTrayZStepSpeed[8];
            edtAuto6SpeedZ->Text=TestIF_File.iTrayZStepSpeed[8];
        }
    }
    else
    {
        TestIF_File.iTrayZStepSpeed[8]=50;
    }
}
//---------------------------------------------------------------------------
void TfSpeed::DoIniDataToForm()
{
    //--------------------------------------------------------------------------
    //Index Arm
    //--------------------------------------------------------------------------
    udIndexSpd->Position            =ArmSpeed_File[IndexArm].iBodySP;
    edIndexSpeed->Text              =udIndexSpd->Position;
    udIndexAcc->Position            =ArmSpeed_File[IndexArm].iACDCBodySP;
    edIndexAccDec->Text             =udIndexAcc->Position;
    edIndexRetryCount->Text         =ArmSpeed_File[IndexArm].iRetryCT;
    edIndexArmRetryMM->Text         =FormatFloat("0.00", ArmSpeed_File[IndexArm].dRetryDown);
    edIndexVacumCheckTime->Text     =FormatFloat("0.00", ArmSpeed_File[IndexArm].dVacuumTI);
    edIndexAirOnTime->Text          =FormatFloat("0.00", ArmSpeed_File[IndexArm].dCTAirOn);
    rgSocketCheck->ItemIndex        =ArmSpeed_File[IndexArm].bIndexFloatCHK;
    chkIndexPreSuck->Checked        =!ArmSpeed_File[IndexArm].bSuckOnDown;
    chkPickAfterOutArm->Checked     =TestIF_File.bIndexPickICWhenOutShtNoIC;    //Steven 20221207 : Index必須在out shuttle沒料才可以吸
    cbEnableDelayTimeZero->Checked  =TestIF_File.bEnableDelayTimeZero;          //Jimmychiu 20230922 : R230824-ATK-H9-01 Request add "Air purge during place device on out-shuttle" function.
    //ChungHung 20130413 add 針對 阿凡達IC 無法Relase
    edIndexDestroyAgainTime->Text   =FormatFloat("0.00", ArmSpeed_File[IndexArm].dDestroyAgainTime);
    edIndexDestroyAgainCount->Text  =ArmSpeed_File[IndexArm].iDestroyAgainCount;

    //Isaac 20180301 (Steven) Index Cycle Time Monitoring function
    //==>
    chkEnableIndexCycleTimeMonitoring->Checked      =TestIF_File.bIndexCycleTimeMonitor;
    edtMonitoringIndexCycletime->Text               =TestIF_File.dIndexCycletimeMonitor;
    edtMonitoringOutlier->Text                      =TestIF_File.dMonitorOutlier;
    edtMonitoringWindow->Text                       =TestIF_File.iMonitorWindow;

    edtIndexCycleTimetolerance->Text                =TestIF_File.dICTTolerance;                                         //JerryYang 20220824 : SPIL index cycle time monitor
    rgIndexCycleTime->ItemIndex                     =TestIF_File.iICTAction;    //Steven 20250521 : 可以選擇要不要停機

    gbIndexeviveConfirm->ItemIndex =(ArmSpeed_File[IndexArm].bDevicConfirm==1)?0:1;                                     //kevin 20190629 add index 4 site conform
    //<==
    //Isaac 20180301 (Steven) Index Cycle Time Monitoring function
    if(CosFunction.bOutArmPickUpErrBySetup)                                     //JerryYang 20220923 : E50改成選項
    {
        rgOutArmPickupErrMode->ItemIndex=IniConfig.iE50_OutArmPickUpErrorOption;
    }

    //--------------------------------------------------------------------------
    //Input Arm
    //--------------------------------------------------------------------------
    udInXSpd->Position              =ArmSpeed_File[InArm].iBodySP;
    edInXYSpd->Text                 =udInXSpd->Position;
    udInXAcc->Position              =ArmSpeed_File[InArm].iACDCBodySP;
    edInXYAcc->Text                 =udInXAcc->Position;
    udInZSpd->Position              =ArmSpeed_File[InArm].iZSP;
    edInZSpd->Text                  =udInZSpd->Position;
    udInZAcc->Position              =ArmSpeed_File[InArm].iACDCZSP;
    edInZAcc->Text                  =udInZAcc->Position;

    //ChungHung 20130413 add 針對 阿凡達IC 無法Relase
    edInArmDestroyAgainTime->Text   =FormatFloat("0.00", ArmSpeed_File[InArm].dDestroyAgainTime);
    edInArmDestroyAgainCount->Text  =ArmSpeed_File[InArm].iDestroyAgainCount;
    rgAutoSpeed->ItemIndex          =(ArmSpeed_File[InArm].bAutoSpeed==false)?1:0;
    edAutoSpeedLow->Text            = ArmSpeed_File[InArm].iAutoSpeedLow;       //kevin 20210913 AutoSpeed 最低速

    edInArmRetryCount->Text         =ArmSpeed_File[InArm].iRetryCT;
    edInArmRetryMM->Text            =FormatFloat("0.00", ArmSpeed_File[InArm].dRetryDown);
    edInVacumCheckTime->Text        =FormatFloat("0.00", ArmSpeed_File[InArm].dVacuumTI);
    edtHPVacuumDelay->Text          =FormatFloat("0.00", ArmSpeed_File[InArm].dHPVacuumTI);                             //Steven 20180125 (Jou) : 加熱盤的真空等待時間
    chkUseHPVacuumDelay->Checked    =ArmSpeed_File[InArm].bUseHPVacuum;         //Steven 20180125 (Jou) : 加熱盤的真空等待時間
    edInArmAirOnTime->Text          =FormatFloat("0.00", ArmSpeed_File[InArm].dCTAirOn);
    edInArmShtWaitTime->Text        =FormatFloat("0.00", ArmSpeed_File[InArm].dWaitOnSH);
    rgTwoSpeedOnOff_In->ItemIndex   =ArmSpeed_File[InArm].iTwoSpeedMove;        //Steven 20140217 : 兩段速移動
    edTwoSpeedDistance_In->Text     =FormatFloat("0.00", ArmSpeed_File[InArm].dTwoSpeedDistance);                       //Steven 20140217 : 兩段速移動
    edSecondSpeedIn->Text           =AnsiString(ArmSpeed_File[InArm].iTwoSpeed);                                        //Steven 20140217 : 兩段速移動
    edSecondADCIn->Text             =AnsiString(ArmSpeed_File[InArm].iTwoADC);  //Steven 20140217 : 兩段速移動
    edtAutoSkipCT->Text             =ArmSpeed_File[InArm].iAutoSkipCT;
    rgInArmAutoSpeed->ItemIndex     =(ArmSpeed_File[InArm].bAutoSKIP==1)?0:1;
    rgInArmPitch->ItemIndex         =(ArmSpeed_File[InArm].bVariModeFIX==true)?1:0;                                     //Jimmychiu 20251924 : bVariModeFIX==1 to bVariModeFIX==true
    rgInShtOrder->ItemIndex         =TestIF_File.iInArmToShtReleaseMode;        //Steven 20250630 : 力旺要照順序擺
    chkTwoSpeedOnlyLoader->Checked  =ArmSpeed_File[InArm].bTwoSpeedOnlyLoader;  //JerryYang 20190729 二段速功能可選擇only at loader

    chkInArmPreSuck->Checked        =!ArmSpeed_File[InArm].bSuckOnDown;
    udInPitchSpd->Position          =ArmSpeed_File[InArm].iVariSP;
    edInPitchSpd->Text              =udInPitchSpd->Position;
    udInPitchAcc->Position          =ArmSpeed_File[InArm].iACDCVariSP;
    edInPitchAcc->Text              =udInPitchAcc->Position;

    //Steven 20140428 : For Rotate
    udInRotSpd->Position            =ArmSpeed_File[InRotate].iBodySP;
    edInRotSpd->Text                =udInRotSpd->Position;
    udInRotAcc->Position            =ArmSpeed_File[InRotate].iACDCBodySP;
    edInRotAcc->Text                =udInRotAcc->Position;

    udPrecisorOpenSp->Position      =ArmSpeed_File[InArm].PrecisorOpenSp;       //JerryYang 20250120 : add
    edPrecisorOpenSp->Text          =udPrecisorOpenSp->Position;
    udPrecisorCloseSp->Position     =ArmSpeed_File[InArm].PrecisorCloseSp;
    edPrecisorCloseSp->Text         =udPrecisorCloseSp->Position;

    //2013-08-01    Dell    modify  公司統一回吸功能
    Edt_CheckTime_In->Text          =FormatFloat("0.0", ArmSpeed_File[InArm].dDestroyCheckTime);
    ckb_PauseCheck_In->Checked      =ArmSpeed_File[InArm].bDestroyPauseCheck;
    edRelaseDelay->Text             =FormatFloat("0.00", ArmSpeed_File[InArm].dReleaseDelayTime);                       //JerryYang 20160127 for TSMC inarm release device前delay
    rgReleaseDelayTime->ItemIndex   =ArmSpeed_File[InArm].iEnableReleaseDelay;  //JerryYang 20160127 for TSMC inarm release device前delay

    edInArmDieCleanDelay->Text      =FormatFloat("0.00", ArmSpeed_File[InArm].dDieCleanDelay);                          //wei 20170418
    rgInArmDieClean->ItemIndex      =(ArmSpeed_File[InArm].iEnableDieClean==1)?0:1;                                     //wei 20170418
    edInArmDieCleanHeight->Text     =FormatFloat("0.00", ArmSpeed_File[InArm].dDieCleanHeight);                         //wei 20170418
    ckb_HeightCheck_In->Checked     =ArmSpeed_File[InArm].bHeightCheck;         //wei 20160825 回吸偵測
    Edt_HeightCheck_In->Text        =ArmSpeed_File[InArm].iHeightCheck;         //wei 20160825 回吸偵測
    gbInarmDeviveConfirm->ItemIndex =(ArmSpeed_File[InArm].bDevicConfirm==1)?0:1;                                       //kevin 20170510 (wei) 回吸功能開關內定開啟
    rgSearchInarmLastModeForYPitch->ItemIndex=(ArmSpeed_File[InArm].bYPitchUseSearchLastMode==0)?1:0;

    edInArmCylinderDelay->Text      =FormatFloat("0.00", ArmSpeed_File[InArm].dCylinderDelay);
    rgTT_SetSpeed->ItemIndex        =(ArmSpeed_File[InArm].bTT_SetSpeed==false)?1:0;                                    //Ztex 2024.08.11 Add Test Time Set Speed
    //--------------------------------------------------------------------------
    //Output Arm
    //--------------------------------------------------------------------------
    udOutXSpd->Position             =ArmSpeed_File[OutArm].iBodySP;
    edOutXSpd->Text                 =udOutXSpd->Position;
    udOutXAcc->Position             =ArmSpeed_File[OutArm].iACDCBodySP;
    edOutXAcc->Text                 =udOutXAcc->Position;
    udOutZSpd->Position             =ArmSpeed_File[OutArm].iZSP;
    edOutZSpd->Text                 =udOutZSpd->Position;
    udOutZAcc->Position             =ArmSpeed_File[OutArm].iACDCZSP;
    edOutZAcc->Text                 =udOutZAcc->Position;

    //ChungHung 20130413 add 針對 阿凡達IC 無法Relase
    edOutArmDestroyAgainTime->Text  =FormatFloat("0.00", ArmSpeed_File[OutArm].dDestroyAgainTime);
    edOutArmDestroyAgainCount->Text =FormatFloat("0", ArmSpeed_File[OutArm].iDestroyAgainCount);
    edOutArmRetryCount->Text        =FormatFloat("0", ArmSpeed_File[OutArm].iRetryCT) ;
    edOutArmRetryMM->Text           =FormatFloat("0.00", ArmSpeed_File[OutArm].dRetryDown);
    edOutVacumCheckTime->Text       =FormatFloat("0.00", ArmSpeed_File[OutArm].dVacuumTI);
    edOutArmShtWaitTime->Text       =FormatFloat("0.00", ArmSpeed_File[OutArm].dWaitOnSH);                              //kevin 20210525 add out arm 等待時間到再往下 Shuttle 吸取 IC
    edOutArmAirOnTime->Text         =FormatFloat("0.00", ArmSpeed_File[OutArm].dCTAirOn);
    rgOutArmPitch->ItemIndex        =(ArmSpeed_File[OutArm].bVariModeFIX==0)?0:1;
    rgTwoSpeedOnOff_Out->ItemIndex  =ArmSpeed_File[OutArm].iTwoSpeedMove;       //Steven 20140217 : 兩段速移動
    edTwoSpeedDistance_Out->Text    =FormatFloat("0.00", ArmSpeed_File[OutArm].dTwoSpeedDistance);                      //Steven 20140217 : 兩段速移動
    edSecondSpeedOut->Text          =AnsiString(ArmSpeed_File[OutArm].iTwoSpeed);                                       //Steven 20140217 : 兩段速移動
    edSecondADCOut->Text            =AnsiString(ArmSpeed_File[OutArm].iTwoADC);                                         //Steven 20140217 : 兩段速移動

    chkOutArmPreSuck->Checked       =!ArmSpeed_File[OutArm].bSuckOnDown;
    udOutPitchSpd->Position         =ArmSpeed_File[OutArm].iVariSP;
    edOutPitchSpd->Text             =udOutPitchSpd->Position;
    udOutPitchAcc->Position         =ArmSpeed_File[OutArm].iACDCVariSP;
    edOutPitchAcc->Text             =udOutPitchAcc->Position;
    edtOutArmPreSuck->Text          =FormatFloat("0.00", ArmSpeed_File[OutArm].dDestroyPauseTime);
    gbOutarmDeviveConfirm->ItemIndex=(ArmSpeed_File[OutArm].bDevicConfirm==1)?0:1;                                      //kevin 20170510 (wei) 回吸功能開關
    rgSearchLastModeForYPitch->ItemIndex=(ArmSpeed_File[OutArm].bYPitchUseSearchLastMode==0)?1:0;                       //JerryYang 20181112 (Steven) : ATP要求修改unloader擺放方式，盡量不要有空格

    //Steven 20140428 : For Rotate
    udOutRotSpd->Position           =ArmSpeed_File[OutRotate].iBodySP;
    edOutRotSpd->Text               =udOutRotSpd->Position;
    udOutRotAcc->Position           =ArmSpeed_File[OutRotate].iACDCBodySP;
    edOutRotAcc->Text               =udOutRotAcc->Position;

    //2013-08-01    Dell    modify  公司統一回吸功能
    Edt_CheckTime_Out->Text         =FormatFloat("0.0", ArmSpeed_File[OutArm].dDestroyCheckTime);
    ckb_PauseCheck_Out->Checked     =ArmSpeed_File[OutArm].bDestroyPauseCheck;

    edOutArmDieCleanDelay->Text      =FormatFloat("0.00", ArmSpeed_File[OutArm].dDieCleanDelay);                        //wei 20170418
    rgOutArmDieClean->ItemIndex      =(ArmSpeed_File[OutArm].iEnableDieClean==0)?1:0;                                   //wei 20170418

    edOutArmCylinderDelay->Text      =FormatFloat("0.00", ArmSpeed_File[OutArm].dCylinderDelay);
    //--------------------------------------------------------------------------
    //Empty Tray Arm
    //--------------------------------------------------------------------------
    udTrayXSpd->Position            =ArmSpeed_File[TrayArm].iBodySP;
    edTrayXSpd->Text                =udTrayXSpd->Position;
    udTrayXAcc->Position            =ArmSpeed_File[TrayArm].iACDCBodySP;
    edTrayXAcc->Text                =udTrayXAcc->Position;
    edTrayArmRetryCount->Text       =ArmSpeed_File[TrayArm].iRetryCT;
    edTrayVacumCheckTime->Text      =FormatFloat("0.00", ArmSpeed_File[TrayArm].dVacuumTI);
    edTrayArmAirOnTime->Text        =FormatFloat("0.00", ArmSpeed_File[TrayArm].dCTAirOn);
    edTrayArmHandDown->Text         =FormatFloat("0.00", ArmSpeed_File[TrayArm].dTAHeadDownIT);
    //--------------------------------------------------------------------------
    //Shuttle
    //--------------------------------------------------------------------------
    udSht1Spd->Position             =SHSpeed_File.iSH1Sp;
    edSht1Spd->Text                 =udSht1Spd->Position;
    udSht1Acc->Position             =SHSpeed_File.iSH1ACDCSp;
    edSht1Acc->Text                 =udSht1Acc->Position;
    udSht2Spd->Position             =SHSpeed_File.iSH2Sp;
    edSht2Spd->Text                 =udSht2Spd->Position;
    udSht2Acc->Position             =SHSpeed_File.iSH2ACDCSp;
    edSht2Acc->Text                 =udSht2Acc->Position;
    rgStepShuttle->ItemIndex        =(SHSpeed_File.iStepSH==1)?0:1;
    rgShakeShuttle->ItemIndex       =TestIF_File.iShakeShuttleWhenPlaceIC;      //Steven 20200616 : ATK要求每次放料都要搖搖蝦頭  //Steven 20220427 : 每次放料都要抖抖馬達
    edShakeCycles->Text             =SHSpeed_File.iShakeCycles;                 //Sam 20250326 : 新增 Shake 條件設定
    edShakeDistance->Text           =SHSpeed_File.iShakeDistance;
    edShakeDelay->Text              =SHSpeed_File.dShakeDelay;
    edShakeAccDec->Text             =SHSpeed_File.iShakeAccDec;
    //--------------------------------------------------------------------------
    //Magazine
    //--------------------------------------------------------------------------
    udMagCatchYSpd->Position        =MGSpeed_File.iCYSp;
    edMagCatchYSpd->Text            =udMagCatchYSpd->Position;
    udMagCatchYAcc->Position        =MGSpeed_File.iCYACDCSp;
    edMagCatchYAcc->Text            =udMagCatchYAcc->Position;

    udMagZSpd->Position              =MGSpeed_File.iMZSp;
    edMagZSpd->Text                  =udMagZSpd->Position;
    udMagZAcc->Position              =MGSpeed_File.iMZACDCSp;
    edMagZAcc->Text                  =udMagZAcc->Position;

    edTwoSpeedDistanceCatchY->Text =FormatFloat("0.00", MGSpeed_File.dCYDecRange);
    edSecondSpeedCatchY->Text        =AnsiString(MGSpeed_File.iCY2Sp);
    edSecondADCCatchY->Text          =AnsiString(MGSpeed_File.iCY2ACDCSp);

    //Steven 20170317 (wei) mark
    //==>
//    cbOutPickErrAct->Checked        =TestIF_File.bOutPickErrAct;              //Steven 20170202 : For ATJ Out Arm pickup error put to define tray
//    cobOutPickErrAct->ItemIndex     =TestIF_File.iOutPickErrAuto;
    //<==
    //Steven 20170317 (wei) mark
}
//---------------------------------------------------------------------------
void TfSpeed::FormClose()
{
    // GOLDEN ODDITY: this is a DISCARD, not a save -- see forms/fSpeed.h
    // banner. `//JerryYang 20250411 : 離開頁面要刷新一次, 避免誤存檔`
    // ("refresh once on leaving the page, to avoid an accidental save").
    fShow=false;
    ReadFile();
    DoIniDataToForm();                                                          //JerryYang 20250411 : 離開頁面要刷新一次, 避免誤存檔

    //這一行請保持在最下面!!-----------------
//    myLog.Do_Log(Sender, asUser, asLogPath);                                  //Steven 20100629
}
//---------------------------------------------------------------------------

// =============================================================================
//  WAVE FW-SPEED-W21 -- AI(W906-FW-SPEED-W21) 20260826
//  42 input-gesture handlers (10 KeyPress + 27 MouseDown + 4 widget-state +
//  1 chrome).  APPEND-ONLY: nothing above this line was edited except the one
//  added `#include "forms/fQwertyKey.h"` in the include block.
//
//  Read forms/fSpeed.h's "WAVE FW-SPEED-W21" banner FIRST -- it carries the
//  selection rationale (why Wave A's bucket-(b) call is superseded for exactly
//  these 42), the denominator (48/57 golden methods after this wave), the GATE
//  register, GOLDEN ODDITIES (W21-a..e) and DEVIATIONS (W21-D1..D4).  Not
//  repeated here; only re-cited per call site.
//
//  MEASURED, not asserted: across all 42 golden bodies the occurrence count of
//  WriteIni* / fopen / CreateFile / SaveSetup* / Motor*Move / ShowMyMessage is
//  0.  These are keystroke filters, keypad launchers and widget-state setters.
//  The persistence path (spbSaveClick golden :1433-1787, SaveSetupFile golden
//  :2250-2447) remains NOT TRANSLATED and NOT DECLARED.
//
//  NOT WIRED (W21-D3): no OnKeyPress/OnMouseDown/OnClick delegate is assigned
//  for any handler below.  Nothing in this tree calls them yet -- which is why
//  the fQwertyKey NULL-pointer exposure (forms/fQwertyKey.h GOLDEN NOTE (G-d),
//  :139-146: both globals stay NULL until a wiring wave CreateForm's them) is
//  unreachable from here.  A future wiring wave MUST construct fQwertyKey AND
//  fQwertyKey2 before enabling the 27 MouseDown bodies.
//
//  GATE REGISTER (this wave): (W21-G1) sbtExitClick only -- see its call site.
// =============================================================================

//---------------------------------------------------------------------------
//  (i) KeyPress numeric filters -- 10 methods.
//      Golden shape: `if(OnlyNumberXxxInPut(Key)==false) Key=NULL;`
//      `TObject *Sender` dropped, unread (W21-D1); `char &Key` kept (read and
//      written).  Same shape as the already-merged forms/fSetup.cpp:67-71
//      TfSetup::XPitchKeyPress, including `Key=NULL` (ht9045_sm builds with
//      -Wno-conversion-null, CMakeLists.txt:2264).
//---------------------------------------------------------------------------
void TfSpeed::edInArmRetryCountKeyPress(char &Key)                              // golden :1217-1221
{
    if(OnlyNumberInPut(Key)==false)                                             // golden :1219 -- the ONLY integer-only filter of the 10 (W21-c)
        Key=NULL;
}
//---------------------------------------------------------------------------
void TfSpeed::edInVacumCheckTimeKeyPress(char &Key)                             // golden :1223-1227
{
    if(OnlyNumberAndDotInPut(Key)==false)
        Key=NULL;
}
//---------------------------------------------------------------------------
void TfSpeed::edIndexArmRetryMMKeyPress(char &Key)                              // golden :1229-1233
{
    if(OnlyNumberAndDotInPut(Key)==false)
        Key=NULL;
}
//---------------------------------------------------------------------------
void TfSpeed::Edt_CheckTime_InKeyPress(char &Key)                               // golden :1935-1939
{
    if(OnlyNumberAndDotInPut(Key)==false)
        Key=NULL;
}
//---------------------------------------------------------------------------
void TfSpeed::edRelaseDelayKeyPress(char &Key)                                  // golden :1983-1987
{
    if(OnlyNumberAndDotInPut(Key)==false)
        Key=NULL;                                                               //JerryYang 20160127 for TSMC inarm release device前delay
}
//---------------------------------------------------------------------------
//Isaac 20180301 (Steven) Index Cycle Time Monitoring function
//==>
void TfSpeed::edtMonitoringIndexCycletimeKeyPress(char &Key)                    // golden :2191-2196
{
    if(OnlyNumberAndDotInPut(Key)==false)
        Key=NULL;
}
//---------------------------------------------------------------------------
void TfSpeed::edtMonitoringOutlierKeyPress(char &Key)                           // golden :2198-2203
{
    if(OnlyNumberAndDotInPut(Key)==false)
        Key=NULL;
}
//---------------------------------------------------------------------------
void TfSpeed::edtMonitoringWindowKeyPress(char &Key)                            // golden :2205-2210
{
    // GOLDEN ODDITY (W21-c): edtMonitoringWindow is opened as N_INTEGER by its
    // MouseDown (golden :2228) yet filtered here with the DOT-permitting
    // OnlyNumberAndDotInPut.  Golden's own inconsistency; kept verbatim.
    if(OnlyNumberAndDotInPut(Key)==false)
        Key=NULL;
}
//---------------------------------------------------------------------------
void TfSpeed::edtIndexCycleTimetoleranceKeyPress(char &Key)                     // golden :2238-2243
{
    // GOLDEN ODDITY (W21-c): same N_INTEGER-vs-dot mismatch as above
    // (MouseDown golden :2235 passes N_INTEGER).  Verbatim.
    if(OnlyNumberAndDotInPut(Key)==false)
        Key=NULL;
}
//<==
//Isaac 20180301 (Steven) Index Cycle Time Monitoring function
//---------------------------------------------------------------------------
void TfSpeed::edShakeDelayKeyPress(char &Key)                                   // golden :2505-2509
{
    if(OnlyNumberAndDotInPut(Key)==false)
        Key=NULL;
}

//---------------------------------------------------------------------------
//  (ii) MouseDown keypad launchers -- 27 methods.
//       Golden shape: one `fQwertyKey->ShowQwertyKey((TEdit*)Sender, iFunction,
//       iDP, bCheckRange, min, max);` call (2 of the 27 branch first).
//       `TMouseButton Button, TShiftState Shift, int X, int Y` dropped -- none
//       is read in any of the 27 bodies, checked one by one (W21-D1).
//       `Sender` kept and typed as golden's cast target, collapsing the
//       C-style cast into the signature (W21-D2).
//       On the min/max argument order see GOLDEN ODDITY (W21-b): golden's house
//       style passes the numerically LARGER bound into `min`; 8 of these 27
//       invert even that.  Both orders clamp identically because CheckRange
//       (MachineType.h:1524-1544) opens with `if(Maximum<Minimum)`.  Every call
//       below is verbatim, argument for argument.
//---------------------------------------------------------------------------
void TfSpeed::edInArmRetryCountMouseDown(TEdit *Sender)                         // golden :1235-1246
{
    if(CosFunction.bUseOneByOneIndexCheck==true)
    {
        fQwertyKey->ShowQwertyKey(Sender, N_INTEGER, 2, true, 3, 0);
    }
    else
    {
        fQwertyKey->ShowQwertyKey(Sender, N_INTEGER, 2, true, 10, 0);
    }
}
//---------------------------------------------------------------------------
void TfSpeed::edInVacumCheckTimeMouseDown(TEdit *Sender)                        // golden :1248-1252
{
    fQwertyKey->ShowQwertyKey(Sender, N_DOUBLE, 2, true, 10.0, 0.01);           //Steven 20150420
}
//---------------------------------------------------------------------------
void TfSpeed::edIndexArmRetryMMMouseDown(TEdit *Sender)                         // golden :1254-1258
{
    fQwertyKey->ShowQwertyKey(Sender, N_DOUBLE, 2, true, -0.01, -0.3);          //Steven 20150420
}
//---------------------------------------------------------------------------
void TfSpeed::edIndexSpeedMouseDown(TEdit *Sender)                              // golden :1260-1264
{
    fQwertyKey->ShowQwertyKey(Sender, N_INTEGER, 0, true, (CosFunction.bLimitMaxSpeed)?CosFunction.iLimitMaxSpeed:100, 1);   //Steven 20150420
}
//---------------------------------------------------------------------------
void TfSpeed::edIndexAccDecMouseDown(TEdit *Sender)                             // golden :1266-1270
{
    fQwertyKey->ShowQwertyKey(Sender, N_INTEGER, 0, true, 100, 1);              //Steven 20150420
}
//---------------------------------------------------------------------------
void TfSpeed::Edt_CheckTime_InMouseDown(TEdit *Sender)                          // golden :1941-1945
{
    fQwertyKey->ShowQwertyKey(Sender, N_DOUBLE, 2, true, 1.0, 0.1);             //Steven 20140420
}
//---------------------------------------------------------------------------
void TfSpeed::edtAutoSkipCTMouseDown(TEdit *Sender)                             // golden :1947-1951
{
    fQwertyKey->ShowQwertyKey(Sender, N_INTEGER, 0, true, 1000, 5);             //Steven 20150205 : KK說要改成最小5次
}
//---------------------------------------------------------------------------
void TfSpeed::edSecondSpeedInMouseDown(TEdit *Sender)                           // golden :1953-1957
{
    // (W21-b) larger bound passed into `max`, inverting golden's own house
    // style -- benign, CheckRange clamps symmetrically.  Verbatim.
    fQwertyKey->ShowQwertyKey(Sender, N_INTEGER, 0, true, 1, atoi(edInZSpd->Text.c_str()));      //Steven 20140217 : 兩段速移動
}
//---------------------------------------------------------------------------
void TfSpeed::edSecondADCInMouseDown(TEdit *Sender)                             // golden :1959-1963
{
    fQwertyKey->ShowQwertyKey(Sender, N_INTEGER, 0, true, 1, atoi(edInZAcc->Text.c_str()));      //Steven 20140217 : 兩段速移動
}
//---------------------------------------------------------------------------
void TfSpeed::edSecondSpeedOutMouseDown(TEdit *Sender)                          // golden :1965-1969
{
    fQwertyKey->ShowQwertyKey(Sender, N_INTEGER, 0, true, 1, atoi(edOutZSpd->Text.c_str()));     //Steven 20140217 : 兩段速移動
}
//---------------------------------------------------------------------------
void TfSpeed::edSecondADCOutMouseDown(TEdit *Sender)                            // golden :1971-1975
{
    fQwertyKey->ShowQwertyKey(Sender, N_INTEGER, 0, true, 1, atoi(edOutZAcc->Text.c_str()));     //Steven 20140217 : 兩段速移動
}
//---------------------------------------------------------------------------
void TfSpeed::edRelaseDelayMouseDown(TEdit *Sender)                             // golden :1977-1981
{
    fQwertyKey->ShowQwertyKey(Sender, N_DOUBLE, 2, true, 3.0, 0.1);             //JerryYang 20160127 for TSMC inarm release device前delay
}
//---------------------------------------------------------------------------
void TfSpeed::Edt_HeightCheck_InMouseDown(TEdit *Sender)                        // golden :1989-1993
{
    fQwertyKey->ShowQwertyKey(Sender, N_DOUBLE, 2, true, 1500, 500);            //Steven 20140420
}
//---------------------------------------------------------------------------
//Isaac 20180301 (Steven) Index Cycle Time Monitoring function
//==>
void TfSpeed::edtMonitoringIndexCycletimeMouseDown(TEdit *Sender)               // golden :2212-2217
{
    // (W21-b) larger bound into `max`.  Verbatim.
    fQwertyKey->ShowQwertyKey(Sender, N_DOUBLE, 2, true, 0.01, 10.0);
}
//---------------------------------------------------------------------------
void TfSpeed::edtMonitoringOutlierMouseDown(TEdit *Sender)                      // golden :2219-2223
{
    fQwertyKey->ShowQwertyKey(Sender, N_DOUBLE, 2, true, 0.01, 30.0);
}
//---------------------------------------------------------------------------
void TfSpeed::edtMonitoringWindowMouseDown(TEdit *Sender)                       // golden :2225-2229
{
    fQwertyKey->ShowQwertyKey(Sender, N_INTEGER, 0, true, 500, 1);
}
//---------------------------------------------------------------------------
void TfSpeed::edtIndexCycleTimetoleranceMouseDown(TEdit *Sender)                // golden :2231-2236
{
    // (W21-d) iDP==15 is not a plausible decimal-place count; (W21-b) larger
    // bound into `max`.  Both verbatim.
    fQwertyKey->ShowQwertyKey(Sender, N_INTEGER, 15, true, 5, 200);             //JerryYang 20220824 : SPIL index cycle time monitor
}
//<==
//Isaac 20180301 (Steven) Index Cycle Time Monitoring function
//---------------------------------------------------------------------------
void TfSpeed::edtLoaderSpeed1MouseDown(TLabeledEdit *Sender)                    // golden :2448-2452
{
    // (W21-D2) golden casts to (TLabeledEdit *), matching this widget's own
    // declared type at forms/fSpeed.h:606.  (W21-d) iDP==1.
    fQwertyKey->ShowQwertyKey(Sender, N_INTEGER, 1, true, (CosFunction.bLimitMaxSpeed)?CosFunction.iLimitMaxSpeed:100, 1);   //Steven 20150420
}
//---------------------------------------------------------------------------
void TfSpeed::edInArmDieCleanDelayMouseDown(TEdit *Sender)                      // golden :2454-2458
{
    fQwertyKey->ShowQwertyKey(Sender, N_DOUBLE, 2, true, 10.0, 0.1);            //JerryYang 20160127 for TSMC inarm release device前delay
}
//---------------------------------------------------------------------------
void TfSpeed::edInArmDieCleanHeightMouseDown(TEdit *Sender)                     // golden :2460-2467
{
    if(CUSTOMER_CODE==CC_TSMC_TAINAN)
        fQwertyKey->ShowQwertyKey(Sender, N_DOUBLE, 2, true, 20.0, 0.1);
    else
        // (W21-b) the else-arm's bounds are (-20.0, 0.1) -- larger value into
        // `max`, unlike the if-arm.  Golden's own asymmetry; verbatim.
        fQwertyKey->ShowQwertyKey(Sender, N_DOUBLE, 2, true, -20.0, 0.1);
}
//---------------------------------------------------------------------------
void TfSpeed::edOutArmShtWaitTimeMouseDown(TEdit *Sender)                       // golden :2469-2473
{
    fQwertyKey->ShowQwertyKey(Sender, N_DOUBLE, 2, true, 50.0,0);               //kevin 20210525 add out arm shuttle wait
}
//---------------------------------------------------------------------------
void TfSpeed::edAutoSpeedLowMouseDown(TEdit *Sender)                            // golden :2475-2479
{
    fQwertyKey->ShowQwertyKey(Sender, N_INTEGER, 0, true, 100, 1);
}
//---------------------------------------------------------------------------
void TfSpeed::edTwoSpeedDistanceCatchYMouseDown(TEdit *Sender)                  // golden :2481-2485
{
    fQwertyKey->ShowQwertyKey(Sender, N_DOUBLE, 2, true, 20.0, 0.01);           //Steven 20150420
}
//---------------------------------------------------------------------------
void TfSpeed::edShakeCyclesMouseDown(TEdit *Sender)                             // golden :2487-2491
{
    fQwertyKey->ShowQwertyKey(Sender, N_INTEGER, 1, true, 100, 1);              // (W21-d) iDP==1
}
//---------------------------------------------------------------------------
void TfSpeed::edShakeDistanceMouseDown(TEdit *Sender)                           // golden :2493-2497
{
    fQwertyKey->ShowQwertyKey(Sender, N_INTEGER, 5, true, 30, 5);               // (W21-d) iDP==5
}
//---------------------------------------------------------------------------
void TfSpeed::edShakeDelayMouseDown(TEdit *Sender)                              // golden :2499-2503
{
    // (W21-e) golden passes the DOUBLE literal `0.0` as the `int iDP`
    // argument -- implicit narrowing to 0.  Written verbatim rather than
    // "corrected" to 0; the resulting argument is identical, and ht9045_sm
    // enables neither -Wconversion/-Wfloat-conversion nor -Werror
    // (CMakeLists.txt:2263-2264), so this is diagnostic-free.
    fQwertyKey->ShowQwertyKey(Sender, N_DOUBLE, 0.0, true, 2.0, 0.0);
}
//---------------------------------------------------------------------------
void TfSpeed::edShakeAccDecMouseDown(TEdit *Sender)                             // golden :2511-2515
{
    fQwertyKey->ShowQwertyKey(Sender, N_INTEGER, 100, true, 100, 1);            // (W21-d) iDP==100
}

//---------------------------------------------------------------------------
//  (iii) Pure widget-state setters -- 4 methods.  No I/O, no dependency, no
//        machine effect.  `TObject *Sender` dropped, unread (W21-D1).
//---------------------------------------------------------------------------
void TfSpeed::spbSpeedAddClick()                                                // golden :1414-1419
{
    // GOLDEN ODDITY (W21-a) -- DO NOT "SIMPLIFY".  This is "+10, then snap
    // DOWN to the nearest multiple of 10" via INTEGER truncation, not a
    // redundant round-trip: Position=95 yields 105/10*10 = 100, not 105.
    // Position is int (forms/fSpeed.h TfSpeedTrackBar), so `/=10; *=10;`
    // truncates.  Making either operand floating point destroys the snap --
    // this repo has already paid for that exact class of edit once
    // (ChangeToFloatNonPcnt int-division regression, cConfiguration.cpp:152).
    // No clamp against tbAllSpeed->Max: golden leans on the real VCL
    // TTrackBar property setter to clamp; the port's plain int does not.
    // Disclosed in the header banner, deliberately NOT "fixed" here.
    tbAllSpeed->Position+=10;
    tbAllSpeed->Position/=10;
    tbAllSpeed->Position*=10;
}
//---------------------------------------------------------------------------
void TfSpeed::spbSpeedDecClick()                                                // golden :1421-1424
{
    // (W21-a) asymmetric with spbSpeedAddClick above: a bare -=10 with NO
    // snap-to-multiple-of-10 and no clamp at 0.  Golden's own asymmetry, kept.
    tbAllSpeed->Position-=10;
}
//---------------------------------------------------------------------------
void TfSpeed::cbIndexArmClick()                                                 // golden :1426-1431
{
    // Golden enables the speed controls unconditionally -- it does NOT read
    // cbIndexArm->Checked, so unchecking the box also enables them.  Verbatim.
    tbAllSpeed->Enabled =true;
    spbSpeedAdd->Enabled=true;
    spbSpeedDec->Enabled=true;
}
//---------------------------------------------------------------------------
void TfSpeed::spbSelectAllClick()                                               // golden :1795-1810
{
    cbIndexArm->Checked =true;
    cbInArm->Checked    =true;
    cbOutArm->Checked   =true;
    cbShuttle->Checked  =true;
    cbTrayArm->Checked  =true;
    cbInArmZ->Checked   =true;
    cbOutArmZ->Checked  =true;
    tbAllSpeed->Enabled =true;
    spbSpeedAdd->Enabled=true;
    spbSpeedDec->Enabled=true;
    spbSelectAll->Down  =false;
    cbInRotate->Checked =true;
    cbOutRotate->Checked=true;
}

//---------------------------------------------------------------------------
//  (iv) Chrome -- 1 method.
//---------------------------------------------------------------------------
void TfSpeed::sbtExitClick()                                                    // golden :1788-1793
{
    Close();                                                                    // offline no-op member (W21-D4), forms/fSpeed.h
#if 0 // GATE (W21-G1): golden :1791.  BOTH halves absent from the port --
      //   TfShowMessage (forms/fShowMessage.h:22-27) declares only
      //   ShowSpeed(bool), there is no `sgdSpeedView`; and `Repaint()` exists
      //   on no vclcompat control (rg over vclcompat/ -> 2 hits, both comments
      //   in LedCore.cpp:132/:160 saying it was dropped as a renderer concern).
      //   Both owning files are outside this wave's write boundary and the
      //   campaign rule is 跨檔缺口 GATE 不自建 shim.  This is the SAME golden
      //   expression already gated identically at cinitial.cpp:13595-13596
      //   (GATE n5-G1); that gate's shape is reused rather than inventing a
      //   second rationale.  Consequence is cosmetic only: a grid on ANOTHER
      //   form is not repainted.  No logic, no data, no machine effect.
    fShowMessage->sgdSpeedView->Repaint();
#endif // GATE (W21-G1): sgdSpeedView / Repaint
    sbtExit->Down=false;
}
//---------------------------------------------------------------------------

// AI(W906-T4-U36) 20260924: 全域 fSpeed —— golden 由 Application->CreateForm 建立（HT9045.cpp 的表單清單）。
//   forms/fSpeed.h 的 INTEGRATION-PENDING 說明寫「唯一消費者 cprod.cpp:2656-2662 被閘住，宣告全域也解不開它」；
//   T4 靜態盤點（U36）指出真正的缺口是 golden TfMain::DoReadLastData main.cpp:8923 開機就叫 fSpeed->ReadFile()，
//   而移植樹沒有物件 ⇒ ArmCondition.Data 從沒讀過 ⇒ ArmSpeed 全 0 ⇒ SetMotorScaleSpeed 夾成 1%。
//   建構子（上方 :103）只碰自己的 NSDMI widget，靜態初始化安全（PT-W2 的教訓是「建構子不可解參考 NULL 全域」）。
//   全樹其他 fSpeed-> 使用點（Command.cpp:10245/:15079、ProductionInfo/uPAT_Function.cpp:2112-2130）全在 #if 0 內（20260924 量過）。
TfSpeed *fSpeed = new TfSpeed();

// AI(W906-T4-U34U36) 20260924: wb_serve 開機的 DoReadLastData 片段 —— golden main.cpp:8919 fHotPlate->ReadFile()、
//   :8923 fSpeed->ReadFile()（中間 :8921 的 fLd_ULd->ReadFile 已由 T4 U35 a9cf183 補上，見下方區塊；原本閘在 GATE (L-5)）。
//   包成函式是為了讓 tools/wb_serve.cpp 用區塊內 extern 宣告呼叫，不必在它的檔頭加 include（會位移全檔行號）。
//   兩者都在第二次 SetWorkParameter() 之前呼叫，DoStructUnitConvert 才換算得到剛讀進來的 HotPlateForm_File／ArmSpeed_File。
//   include 放在這裡而不是檔頭：檔頭加一行會位移本檔全部行號（多處文件以行號引用 cSpeed.cpp）。
#include "forms/fHotPlate.h"   // fHotPlate（forms/fHotPlate.cpp:27）
#include "forms/fLd_ULd.h"     // fLd_ULd（forms/fLd_ULd.cpp:44）—— AI(W906-T4-U35) 20260924
//Steven 20260924 (S12-C)：fLd_ULd->Init()（golden TfLd_ULd 建構子的 elUdUld 註冊）只做一次的旗標抽成共用函式。
//   FileRW/IniConfig.cpp 開機時要照 golden HT9045.cpp 的 CreateForm 順序（TfLd_ULd :191 在 TfConfiguration :207 之前）
//   先完成它，再跑 InitConfigEdtList —— HTEditList 同 (區段,鍵) 第一筆生效，順序不能反。
//Steven 20260924 (S12-C Ld_UldDelayTime)：改由 FileRW/Ld_UldDelayTime.cpp 用 golden TfLd_ULd 建構子（具名替身）註冊 ——
//   原本 fLd_ULd->Init() 以真元件註冊，THTEdit::ControlName 是空的，頁面拿不到名稱。FileRW_LdUld_Boot 自己保證只做一次。
extern void FileRW_LdUld_Boot();
extern void FileRW_LdUld_ReadFile();
void W906_LdUldInitOnce()
{
    FileRW_LdUld_Boot();
}

void W906_BootReadHotPlateAndSpeed()
{
    fHotPlate->ReadFile();                                                      // golden main.cpp:8919
    {   // AI(W906-T4-U35) 20260924: golden main.cpp:8921 fLd_ULd->ReadFile()（UdUld.Data → Ld_UldDelayTime）；Init() 只叫一次（不冪等）
        W906_LdUldInitOnce();
        FileRW_LdUld_ReadFile();   //Steven 20260924 (S12-C)：golden TfLd_ULd::ReadFile（Ld_UldDelayTime.gen.inc）
    }
    fSpeed->ReadFile();                                                         // golden main.cpp:8923
}
