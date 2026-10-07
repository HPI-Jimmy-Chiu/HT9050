// =============================================================================
//  forms/fContact_IndexPickPlace.cpp -- E-042 = E-038 Phase B, step B3 (SAFETY): golden's Index arm pick-from-shuttle
//  and place-to-shuttle state machines.  golden 0618 cContact.cpp:
//    span A :2384-5252   CheckIndexArmStatus (free function) + timers DoZ1/Z2PickFromShuttleDelay +
//                        TfContact::DoZ1PickFromShuttle + TfContact::DoZ2PickFromShuttle
//    span B :11354-11739 timer htPlaceToShuttleDelay + TfContact::DoZPlaceToShuttle
//    span C :19277-19501 TfContact::DoArm1PlaceToShuttle + TfContact::DoArm2PlaceToShuttle
//
//  AI(W906-E042) 20261005 (B3, St01): NEW FILE. Golden source of truth:
//  D:\HT9045\backup\HT9011UC_Code_V3.33.906.0_20260618\cContact.cpp (cp950, decoded strictly, 0 U+FFFD).
//  Plan: D:\AI_TempFile\st01e-e042-plan-20261004.md (s1.1 rows 7-11, s7 B3, s14).  Generator (re-run byte-identical):
//  D:\AI_TempFile\st01e-e042-run\gen_b3.py (+ b3_edits.py: every changed golden line, asserted).
//
//  *** LIVE ON HT9050 SINCE W-152 (main b4639372, package 176): THESE BODIES MOVE INDEX Z1 AND THE SHUTTLES *** -- AI(W906-W152-BANNER)
//    golden's callers are DoTestContactFunction cases 300 / 330 / 900 (:12771 / :12805 / :13350), translated in forms/fContact_ContactSM.cpp
//    (step B4); MainProc calls DoTestContactFunction on the HT9050 PCI1203 Index Z1 only (csystem.cpp:31466; was #if 0).
//    tests/test_indexz_autoheight_1203.cpp [B9] / [B14] pin that ONE live caller (was: 0).  P4 refuses unless EastSun hand-sets
//    [IndexDriver] HT9050_INDEXZ_TORQUE_CONFIRMED=1 (RULINGS_20261007 #9; machine preconditions: TO_ES02 W-160).
//
//  LINE MAP: each span is VERBATIM and CONTIGUOUS:
//    IndexPickPlace span A: port line = golden line + (-2300)
//    IndexPickPlace span B: port line = golden line + (-8399)
//    IndexPickPlace span C: port line = golden line + (-15934)
//  Every changed golden line carries AI(W906-E042) on that same line (inserts sit before the line's trailing //).
//
//  PORT PROTECTIONS (all AI(W906-E042) NOT GOLDEN; live PCI1203 Index Z1 SHIP rows only; RS-232 / Panasonic machines
//  and SOFT_SIMULTE run golden byte for byte; a stop is never gated):
//    P7  W906_IndexZDriveFaultStop on each state machine's `switch(Task)` line = every tick, before golden's dispatch
//        (Steven 1004 07:5x).  Trip -> "ST" + message + golden case 536's exit
//        (fAllMotorHome=false; CarlibrationTask=1; return false) -> the contact SM's case 1 then says "Must home first".
//    P5  W906_IndexZTorqueWaitStop on the pick torque waits (DoZ1PickFromShuttle 560 golden :3012, Z2 twin :4382).
//    P9  W906_IndexZCommandFloorStop on the pick descent step lines (550, golden :2992 / Z2 twin :4363): never step from
//        a commanded position at / below fIndexDownPos (golden walks on while bNeedSuck is set, :2933-2950).
//    W-44 (socket press only, ST01-M 1005 03:4x) is NOT applied here: a pick / place needs the shuttle parked under
//        the index.  It sits on Do_Z1 / Do_Z2_AutoGetHeight (forms/fContact_AutoHeight.cpp).
//  #20 RULE (V912 better): DoArm1PlaceToShuttle case 300 starts hContactDeley before case 400 waits on it (D1).
// =============================================================================
#include "forms/fContact.h"
#include "forms/fContact_AutoHeight.h"
#include "cContact.h"
#include "MachineDefine.h"
#include "atester.h"
#include "atester_shims.h"
#include "aTester_Front.h"
#include "aArmHeader.h"             // __FUNC__
#include "MachineType.h"
#include "Motor/mymotor.h"          // MOT[]
#include "Motor/HTMotor.h"
#include "mysensor.h"               // Sen[]
#include "myswitch.h"               // SW[]
#include "csystem.h"
#include "cprod.h"
#include "cmydef.h"
#include "cpublic.h"                // Get0_01MMType / ConvertTouMType
#include "cUnitConvert.h"           // iUnitMultiply100
#include "common.h"
#include "Config.h"
#include "CosFunction.h"
#include "LastSet.h"
#include "aHotPlateSubstrate.h"     // FTestSuck / BTestSuck / TestSocket -- THE aHotPlateSubstrate.h TMyKitSuck
#include "FormsFacade.h"            // fMain
#include "canary_support.h"         // ShowMyMessage
#include "mycylin.h"                // Cylinder[]
#include "ckernel.h"
#include "cinitial.h"               // ShowMainScreenPresure
#include "MessageDef.h"
#include "Motor/myGALILmotor.h"
#include "acarry_shims.h"
#include "acarry.h"                 // DoInOutARM_SHT_MoveSafe
#include "IndexZTorque1203.h"       // P5 / P7 / P9 (AI(W906-E042))
#include "atester_ProcessCount.h"   // ProcessPiggyBackFunction
#include <cstdlib>
#include <cmath>

const int iSuckDelay                        =10;   // golden cContact.cpp:73
//  golden cContact.cpp:87-88 (file scope, internal linkage; the port's cContact.h does not carry them).
const int PICK_UP_OFFSET                    =0;
const int CONTACT_UP_OFFSET                 =90;
//  AI(W906-E042) DEVIATION, same as forms/fContact_AutoHeight.cpp / forms/fContact.cpp D-8: golden passes
//  `X->Text.c_str()` to Get0_01MMType(char*); the body only reads it.
static inline int Get0_01MMType(const char* s) { return Get0_01MMType(const_cast<char*>(s)); }

// ===== golden 0618 cContact.cpp:2384-5252 (span A) =====
bool CheckIndexArmStatus(int whicheArm)                                         //Steven 20120214 Start: 避免吸嘴上有IC
{
    int ret1[MAX_SOCKET_ROW][MAX_SOCKET_COL]={{0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0}};
    int ret2[MAX_SOCKET_ROW][MAX_SOCKET_COL]={{0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0}};
    int iNN=IsNNMode();
    bool bArm1SiteUse=false, bArm2SiteUse=false;
    bool bHasFail1=false, bHasFail2=false;
    AnsiString errPart1="at Index Arm 1";
    AnsiString errPart2="at Index Arm 2";

    for(int i=0; i<FTestSuck.iShtRow; i++)
    {
        for(int j=0; j<FTestSuck.iShtCol; j++)
        {
            bArm1SiteUse=false;
            bArm2SiteUse=false;
            if(bUseTwoArm32Site==true)
            {
                if(LastSet.bUseTestSocket[0][i+iNN][j])
                    bArm1SiteUse=true;

                if(LastSet.bUseTestSocket[0][i][j])
                    bArm2SiteUse=true;
            }
            else
            {
                if(LastSet.bUseTestSocket[0][i][j])                             //ChungHung 20130910 alter for SCK can close site by Index
                    bArm1SiteUse=true;

                if(LastSet.bUseTestSocket[1][i][j])                             //Steven 20100824 : 關Site的位置不做檢查
                    bArm2SiteUse=true;
            }

            if(bArm1SiteUse)
            {
                if(IniConfig.bSIGURDFunction)                                   //Sam 20221019 : 矽格北興 Contact 模式初始檢查吸嘴上有 IC 就報警。
                {
                    ret1[i][j]=CheckTestSuckStatus(FTestSuck, i, j);
                }
                else
                {
                    ret1[i][j]=CheckSuckInitialStatus(FTestSuck, i, j);
                }
            }
            else
            {
                ret1[i][j]=0;
            }

            if(bArm2SiteUse)
            {
                if(IniConfig.bSIGURDFunction)                                   //Sam 20221019 : 矽格北興 Contact 模式初始檢查吸嘴上有 IC 就報警。
                {
                    ret2[i][j]=CheckTestSuckStatus(BTestSuck, i, j);
                }
                else
                {
                    ret2[i][j]=CheckSuckInitialStatus(BTestSuck, i, j);
                }
            }
            else
            {
                ret2[i][j]=0;
            }

            if(whicheArm==1)
            {
                ret1[i][j]=0;
            }

            if(ret1[i][j]!=0)
            {
                bHasFail1=true;
                errPart1+=IndexSuckName[i+iNN][j];
            }

            if(ret2[i][j]!=0)
            {
                bHasFail2=true;
                errPart2+=IndexSuckName[i][j];
            }
        }
    }

    if((IniConfig.bD58UseArm1PickPlaceArm2Test==true &&
        TestIF_File.bArm1PickPlaceArm2Test==true) &&                            //Steven 20150129 : 需要確認Arm2有沒有粘料
       TestIF_File.bCheckArm2Vacuum==false)                                     //Ifor 20200811 Fix: Arm1 Pick Place Arm2Test 需卡兩個條件
        bHasFail2=false;

    if(bHasFail1)                                                               //jou 2011-11-08 retry -> skip字義上比較恰當
    {
        ShowErrorMessage("WAR0320", K_RETRY, MTestZ1, false, errPart1);
    }

    if(bHasFail2)
    {
         ShowErrorMessage("WAR0320", K_RETRY, MTestZ2, false, errPart2);
    }
    return (bHasFail1 || bHasFail2);
}
TQPF_Timer DoZ1PickFromShuttleDelay;
TQPF_Timer DoZ2PickFromShuttleDelay;
//---------------------------------------------------------------------------
bool TfContact::DoZ1PickFromShuttle()   //AI(W906-E042) 20261005 (B3): golden `bool __fastcall` -- __fastcall dropped as in forms/fContact.cpp (the declaration has none)
{
//    #ifdef SOFT_SIMULTE
//        return true;
//    #else
    {
        static int Counter=0, Pos=0, iStepSpeed=0;
        static int  iSuckX1=-1, iSuckY1=-1, iSuckX2=-1, iSuckY2=-1;
        static bool flag1=false, bNeedSuck=false, bAutoShuttle=false, suckjam=true;
        static bool bFlagSh1;
        static bool bSuckFlag1=false, bSuckFlag2=false;
        static bool bProcessIndexSuckDestroy=false;

        int iContactKG=CONTECT_SHUTTLE_KG;
        int &Task=Z_Height_Task, ret=0, iTorque=0, iPosCheck=0, ct=0, PosY1=0;
        int iRow=0, iCol=0;
        int iNN=IsNNMode();                                                     //Steven 20241015 : fixed for NN mode
        bool bf1, bf2, bf3, bf4, bf5, bAlarm=true, bCheckSuck=false;

        if(INDEX_SUCKER_TYPE==1)                                                //jou 2010-05-19 start : 負壓
        {
            if(bProcessIndexSuckDestroy==false)
                bProcessIndexSuckDestroy=fiosetview->ProcessIndexSuckDestroy1();
        }
        AnsiString ErrPart="";                                                  //KaiHuang 20200724 : For ASE-CL Daily Correlation, Index 沒吸到料需 Alarm
        bool bHasErr=false;                                                     //KaiHuang 20200724 : For ASE-CL Daily Correlation, Index 沒吸到料需 Alarm
        bool bHasDuplicateErr=false;

        if(W906_IndexZDriveFaultStop("DoZ1PickFromShuttle")) { fAllMotorHome=false; CarlibrationTask=1; return false; }   switch(Task)   //AI(W906-E042) 20261005 (B3) P7 NOT GOLDEN: M14 drive alarm / ERROR_STOP / servo off / monitor lost or frozen / route fault -> ST + message + golden 536 exit; PCI1203 Z1 SHIP only, no-op elsewhere
        {
            case 1:
                if(MOT[MTestZ1].Gali_Two_ZAxis_Move(Prod.TestZ1_Safe, GotIndexZSpeed(iSpeedZ*1000), "DoZ1PickFromShuttle 1"))                                   //JimmyChiu 20211028 : All speed can set by speed setting.
                {
                    if(iContactMode==CONTACT_AUTO_GET_HEIGHT &&
                       IniConfig.bD12UseDeviceFormPressDoShtHeight==false)                                                                                      //Steven 20140220 : 用生產ep去做蝦頭auto high
                    {
                        ADAM_WriteMaxData(true);                                                                                                                //Steven 20241014 : 整合auto height輸出壓力
                    }
                    else
                    {
                        ADAM_WriteVoltage(DeviceForm.dPress);
                    }

                    if(chk_K_Temperature->Checked==true)                                                                                                        //Ztex 2024.03.26 Add Contact Mode K Temperature
                        return true;

                    Task=100;
                }
                break;
            case 100:  //Steven 20120214 Start: 避免吸嘴上有IC
                for(int i=0; i<FTestSuck.iShtRow; i++)
                {
                    for(int j=0; j<FTestSuck.iShtCol; j++)
                    {
                        if(INDEX_SUCKER_TYPE==0)
                        {
                            FTestSuck.Suck[i][j].On();

                            if(TestIF_File.bArm1PickPlaceArm2Test==false ||
                               ((IniConfig.bD58UseArm1PickPlaceArm2Test==true && TestIF_File.bArm1PickPlaceArm2Test==true) && TestIF_File.bCheckArm2Vacuum==true))  //Steven 20150129 : 需要確認Arm2有沒有粘料    //Ifor 20200811 Fix: Arm1 Pick Place Arm2Test 需卡兩個條件
                            {
                                BTestSuck.Suck[i][j].On();
                            }
                        }
                        else
                        {
                            if(IniConfig.bSIGURDFunction)  //Sam 20221019 : 矽格北興 Contact 模式初始檢查吸嘴上有 IC 就報警。
                            {
                                                                                //不要將 bIndexSuck = true 不然 Alarm 後上面若有 IC 會掉下來
                            }
                            else
                            {
                                fiosetview->bIndexSuck[0][i][j]=true;
                                if(TestIF_File.bArm1PickPlaceArm2Test==false ||
                                   ((IniConfig.bD58UseArm1PickPlaceArm2Test==true && TestIF_File.bArm1PickPlaceArm2Test==true) && TestIF_File.bCheckArm2Vacuum==true))  //Steven 20150129 : 需要確認Arm2有沒有粘料    //Ifor 20200811 Fix: Arm1 Pick Place Arm2Test 需卡兩個條件
                                {
                                    fiosetview->bIndexSuck[1][i][j]=true;
                                }
                            }
                        }
                    }
                }
                DoZ1PickFromShuttleDelay.SetSecAndOn(1);
                Task=110;
                break;
            case 110:
                if(DoZ1PickFromShuttleDelay.Off())
                {
                    if(EP_Install==3)                                                                                                                           //JerryYang 20240111 : add
                    {
                        if(IniConfig.bD24EnableEPCheckFuntion==true)
                        {
                            ADAM_Rang(IniConfig.iD26EPEncoderRange);
                            if(ADAM_Alarm()==true)
                            {
                                ret=ShowErrorMessage("WAR1605", K_RETRY|K_SKIP, 0, MMSystem, "fContact::DoZ1PickFromShuttle");                                  //"請檢查EP是否漏氣!"
                                if(ret==K_RETRY)
                                {
                                    return false;
                                }
                            }
                        }
                    }

                    if(CheckIndexArmStatus(0))
                    {
                        Task=100;
                    }
                    else
                    {
                        Task=120;
                    }
                }
                break;
            case 120:
                if(INDEX_SUCKER_TYPE==0)  //wei 20160318 判斷完需要關真空
                {
                    for(int i=0; i<FTestSuck.iShtRow; i++)
                    {
                        for(int j=0; j<FTestSuck.iShtCol; j++)
                        {
                            FTestSuck.Suck[i][j].Normal();

                            if(TestIF_File.bArm1PickPlaceArm2Test==false ||
                               ((IniConfig.bD58UseArm1PickPlaceArm2Test==true && TestIF_File.bArm1PickPlaceArm2Test==true) && TestIF_File.bCheckArm2Vacuum==true))  //Steven 20150129 : 需要確認Arm2有沒有粘料    //Ifor 20200811 Fix: Arm1 Pick Place Arm2Test 需卡兩個條件
                            {
                                BTestSuck.Suck[i][j].Normal();
                            }
                        }
                    }
                }
                DoZ1PickFromShuttleDelay.SetSecAndOn(1);
                Task=130;
                break;
            case 130:
                if(DoZ1PickFromShuttleDelay.Off())
                {
                    if(BAR_CODE_INSTALL!=ebctUninstall &&
                        TestIF_File.bEnableBarCode &&
                        chkDailyCorrelation->Checked==false)                    //Steven 20121023 : BarCode_2D
                    {
                        if(BOTTOM_2DID && TestIF_File.bEnableBottom2D)          //Steven 20190308 : Bottom 2D
                        {
                            if(iContactMode!=CONTACT_DEVICE_MAP_CHECK)          //Steven 20190910 : Qualcomm功能
                            {
                                for(int i=0; i<FLCarryKit.iShtRow; i++)
                                {
                                    for(int j=0; j<FLCarryKit.iShtCol; j++)
                                    {
                                        if(TestIF_File.iSiteMap[i+iNN][j]>0)    //Steven 20170302 (wei) : 確認哪個Site有開, 從1開始~32
                                            FLCarryKit.SetItemData(i, j, HAS_IC);
                                    }
                                }
                                bSetHasIC=true;                                 //Steven 20190626 : Fixed 2DID error in contact test
                            }
                            Task=150;
                        }
                        else
                        {
                            if(chk2DID->Checked)                                //Steven 20210310 : contact test可以跳過讀取2DID
                            {
                                Task=140;
                            }
                            else
                            {
                                Task=150;
                            }
                        }
                    }
                    else
                    {
                        Task=150;
                    }
                }
                break;
            case 140:
                if(iContactMode!=CONTACT_DEVICE_MAP_CHECK)                      //Steven 20190910 : Qualcomm功能
                {
                    bSetHasIC=true;                                             //Steven 20190626 : Fixed 2DID error in contact test
                    for(int i=0; i<FLCarryKit.iShtRow; i++)
                    {
                        for(int j=0; j<FLCarryKit.iShtCol; j++)
                        {
                            if(TestIF_File.iSiteMap[i+iNN][j]>0)                //Steven 20170302 (wei) : 確認哪個Site有開, 從1開始~32
                            {
                                FLCarryKit.SetItemData(i, j, HAS_IC);
                            }

                            if(TestIF_File.iSiteMap[i][j]>0)                    //Steven 20170302 (wei) : 確認哪個Site有開, 從1開始~32
                            {
                                BLCarryKit.SetItemData(i, j, HAS_IC);
                            }
                        }
                    }
                }

                if(false /*GATE(W906-E042-2DM) AI(W906-E042) 20261005: TfBarCode (aHotPlateSubstrate.h:984) has no DoBarcodeCCDAutoTeach (golden fBarCode 2D-matrix auto learn); until it is ported cb2DMatrix behaves as unchecked (golden without the option)*/ && cb2DMatrix->Checked)                                         //Steven 20160118 : 自動學習2D code Matrix
                {
                    {} /*GATE(W906-E042-2DM) fBarCode->DoBarcodeCCDAutoTeach(true); -- unreachable (condition above)*/
                    Task=141;
                }
                else
                {
                    Task=142;
                }
                break;
            case 141:
                bFlagSh1=true; /*GATE(W906-E042-2DM) bFlagSh1=fBarCode->DoBarcodeCCDAutoTeach(); -- unreachable: only case 140's gated 2D-matrix branch sets Task=141*/
                if(bFlagSh1)
                    Task=142;
                break;
            case 142:
                bFlagSh1=false;
                fBarCode->InitialBarcodeScanInShuttle1();
                {} /*GATE(W906-E042-BCE) fBarCode->CleanBarcodeError(1); -- AI(W906-E042) 20261005: TfBarCode has no CleanBarcodeError (golden Steven 20260421, clears barcode error flags after the shuttle scan); not ported -- must be before B6 on barcode machines*/             //Steven 20260421 : Clean error flags to prevent false alarm in Contact/AutoHeight flow
//                fBarCode->InitialBarcodeScanInShuttle2();
                Task=143;
                break;
            case 143:
                if(bFlagSh1==false)
                {
                    if(TestIF_File.iShuttleMode==1 && TestIF_File.iShuttle_Sel==1)
                    {
                        bFlagSh1=true;
                    }
                    else if(BAR_CODE_INSTALL==ebctUseCCDMode &&                 //Steven 20160106 : 改用CCD拍完就跑的方式
                            TestIF_File.bEnableBottom2D==false)                 //Steven 20190308 : Bottom 2D
                    {
                        if(CosFunction.b2DUseSubJobFunction==true && TestIF_File.b2DUseSubJob==true|| //Ifor 20200807 add:In House 2D Use Sub Job Function
                           (CosFunction.b2DUsePinInspection==true && TestIF_File.b2DUsePinInspection==true)) //Eastsun 20260515 F022: D2 add Pin Inspection
                        {
                            bFlagSh1=fBarCode->DoBarcodeScanInShuttle_1(true);  //Eastsun 20260327 KYEC pin1要求Contact 可以skip 正常做不能skip
                        }
                        else
                        {
                            bFlagSh1=fBarCode->DoBarcodeCCDInShuttle_1();
                        }
                    }
                    else if(TestIF_File.b2DTriggerMode)                         //Steven 20151225 : 改用拍完就跑的方式
                    {
                        bFlagSh1=fBarCode->DoBarcodeTriggerInShuttle_1();
                    }
                    else
                    {
                        bFlagSh1=fBarCode->DoBarcodeScanInShuttle_1(true);
                    }
                }

                if(bFlagSh1)
                {
                    Task=150;
                }
                break;
            case 150:                                                                                                                                           //Steven 20120214 End: 避免吸嘴上有IC
                if(IniConfig.bF21InOutArmZMotorPrivate)
                {
                    if(DoInOutARM_SHT_MoveSafe(0))                                                                                                              //kevin 20161005 SHUTTLE 1 移動安全保護
                        return false;
                    if(DoInOutARM_SHT_MoveSafe(1))                                                                                                              //kevin 20161005 SHUTTLE 1 移動安全保護
                        return false;
                }
                bf1=MOT[MInShuttle1].MotorMove(Prod.InSHT[0].iRight);
                if(TestIF_File.iShuttleMode==0)
                    bf2=MOT[MInShuttle2].MotorMove(Prod.InSHT[1].iRight);                                                                                       //kevin 20220816 unmark
                else
                    bf2=true;

                bf4=true;
                bf5=true;

                bf3=MOT[MTestY1].GalilTwoY_Move(Prod.TestY1_Front, Prod.TestY2_Middle, GotIndexYSpeed(iSpeed*3000), "DoZ1PickFromShuttle 150");                 //JimmyChiu 20211028 : All speed can set by speed setting.
                if(bf1 && bf2 && bf3 && bf4 && bf5)
                    Task=200;
                break;
            case 200:
                if(INDEX_SUCKER_TYPE==1)                                                                                //jou 2010-05-19 start : 負壓
                    fiosetview->ResetIndexSuck();

                for(int i=0; i<FTestSuck.iShtRow; i++)
                {
                    for(int j=0; j<FTestSuck.iShtCol; j++)
                    {
                        if(INDEX_SUCKER_TYPE==0)
                            FTestSuck.Suck[i][j].On();
                        else
                            fiosetview->bIndexSuck[0][i][j]=true;
                    }
                }

                if(INDEX_SUCKER_TYPE==1)
                    bProcessIndexSuckDestroy=false;
                else
                    bProcessIndexSuckDestroy=true;

                bAutoShuttle=false;
                if(iContactMode==CONTACT_AUTO_GET_HEIGHT && chkShuttle->Checked)                                        //auto hight
                {
                    bAutoShuttle=true;
                    InitWriteAndCheckMotorTorqueTask();
                    bNeedSuck=false;
                    Task=300;
                }
                else
                {
                    if(iContactMode==CONTACT_TEST ||
                       iContactMode==CONTACT_DEVICE_MAP_CHECK)                                                          //Steven 20191009 : Device Map for Qualcomm
                    {
                        Task=250;                                                                                       //jou 2010-05-19 start : 負壓
                        if(IniConfig.bD73ContactModeFast)                                                               //kevin 20220817 : Conttact mode 加速
                        {
                            hContactDeley.SetSecAndOn(1);
                        }
                        else if(INDEX_SUCKER_TYPE==1)
                        {
                            hContactDeley.SetSecAndOn(3);
                        }
                        else                                                                                            //kevin 20220817
                        {
                            hContactDeley.SetSecAndOn(1);
                        }
                    }
                    else
                    {
                        Task=3000;
                    }
                }
                break;
            case 250:
                if(hContactDeley.Off() && bProcessIndexSuckDestroy==true)
                {
                    Task=400;
                    suckjam=true;                                               //JerryYang 20231002 : 修正index arm上有IC沒有跳ALARM就把Device吹掉的問題
                }
                break;
            case 300:
                if(USE_IO_CHANGE_TOQUE==true)                                   //jou 2012-06-21 Enable index I/O Change Toque
                {
                    SW[SwIndexChangeToque1].Off();
                    SW[SwIndexChangeToque2].Off();
                }

                ret=COM2->iWriteAndCheckMotorTorque(0, iContactKG);

                if(ret==1)
                {
                    Task=400;                                                   //Eliot 2007_0907
                    Counter=0;
                    iStepSpeed=30000;
                    suckjam=true;
                }
                else if(ret==2)
                {
                    ShowMyMessage("Motor torque set error", "馬達扭力設定錯誤", "DoZ1PickFromShuttle 300");
                    fAllMotorHome=false;
                    CarlibrationTask=1;
                    bNeedSuck=false;
                    return false;
                }
                break;
            case 400:                                                           //Eliot 2007_0907
                flag1=false;
                bUniversalkitflag[0]=false;

                for(int i=0; i<FTestSuck.iShtRow; i++)
                {
                    for(int j=0; j<FTestSuck.iShtCol; j++)
                    {
                        if(FTestSuck.Suck[i][j].GetStatus())
                            flag1=true;
                    }
                }

                if(suckjam==true && flag1==true)                                // sucker already on at high position
                {
                    ShowMyMessage("Maybe Z1 sucker sensor initial on error", "Index Z1的真空產生器的Sensor在初始化時被開啟!", "DoZ1PickFromShuttle 400");
                    fAllMotorHome=false;
                    CarlibrationTask=1;
                    bNeedSuck=false;
                    return false;
                }
                suckjam=false;

                for(int i=0; i<FTestSuck.iShtRow; i++)
                {
                    for(int j=0; j<FTestSuck.iShtCol; j++)
                    {
                        FTestSuck.Suck[i][j].Normal();
                    }
                }
                Pos=0;
                Counter=0;

                if(iContactMode==CONTACT_TEST ||
                   iContactMode==CONTACT_DEVICE_MAP_CHECK)                      //Steven 20191009 : Device Map for Qualcomm
                {
                    if(USE_IO_CHANGE_TOQUE==true)                               //jou 2012-06-21 Enable index I/O Change Toque
                    {
                        SW[SwIndexChangeToque1].On();
                        SW[SwIndexChangeToque2].Off();
                    }
                    Task=3000;
                }
                else
                {
                    if(IniConfig.bChangeKitNoHardStop==true)                    //jou 2015-12-08 Xilinx 驗證用
                    {
                        bSuckFlag1=false;
                        bSuckFlag2=false;
                        for(int i=0; i<FTestSuck.iShtRow; i++)
                        {
                            for(int j=1; j<FTestSuck.iShtCol; j++)
                            {
                                if(bSuckFlag1==false)
                                {
                                    if(LastSet.bUseTestSocket[0][i+iNN][j]==true)
                                    {
                                        FTestSuck.Suck[i][j].On();
                                        bSuckFlag1=true;
                                        iSuckX1=i;
                                        iSuckY1=j;
                                    }
                                }

                                if(bSuckFlag2==false)
                                {
                                    iRow=FTestSuck.iShtRow-i-1;
                                    iCol=FTestSuck.iShtCol-j-1;

                                    if(iRow>=0 &&
                                       iCol>=0 &&
                                       LastSet.bUseTestSocket[0][iRow+iNN][iCol]==true)
                                    {
                                        FTestSuck.Suck[iRow][iCol].On();
                                        bSuckFlag2=true;
                                        iSuckX2=iRow;
                                        iSuckY2=iCol;
                                    }
                                }
                            }
                        }
                        bSuckFlag1=false;
                        bSuckFlag2=false;
                    }
                    Task=500;
                }
                break;
            case 500:
                if(CUSTOMER_CODE==CC_AMKOR_China ||
                   CUSTOMER_CODE==CC_QUALCOMM)                                  //JerryYang 20170412 (Steven) add QUALCOMM
                    Pos-=100;
                else
                    Pos-=300;
                Task=550;

                iPosCheck=MOT[MTestZ1].Gali_ReadEncoderPos();                   //Eliot 2010_01_12 start
                if(iPosCheck<(Tech.iTestZ1ShutlePick-1000))
                {
                    if(bNeedSuck==false)
                    {
                        ShowMyMessage("Index arm Z1 to shuttle height error!", "Please check teaching about index arm 1 to shuttle Z position.", "DoZ1PickFromShuttle 500");
                        Task=570;
                        break;
                    }
                }
                break;
            case 550:
                if(IniConfig.bChangeKitNoHardStop==true)                                                                //jou 2015-12-08 Xilinx 驗證用
                {
                    if(bSuckFlag1==false)
                    {
                        if(iSuckX1<0 || iSuckY1<0)
                            bSuckFlag1=false;
                        else if(FTestSuck.Suck[iSuckX1][iSuckY1].GetStatus())
                            bSuckFlag1=true;
                    }

                    if(bSuckFlag2==false)
                    {
                        if(iSuckX2<0 || iSuckY2<0)
                            bSuckFlag2=false;
                        else if(FTestSuck.Suck[iSuckX2][iSuckY2].GetStatus())
                            bSuckFlag2=true;
                    }

                    if(bSuckFlag1==true || bSuckFlag2==true)
                    {
                        Task=555;
                        StopAllMotor(true);   //AI(W906-E042) 20261005: golden `StopAllMotor();` = the default argument true (Motor/myGALILmotor.h:78); spelled out because aHotPlateSubstrate.h:980 also declares a no-arg StopAllMotor (forwards to (true), aHotPlateSubstrate.cpp:1236) and the call would be ambiguous
                        MOT[MTestZ1].Gali_Command("ST", __FUNC__);                                                      //馬達停止
                        fMain->chkReadTorque1->Checked=false;                                                           //wei 20160504 不顯示扭力
                        fMain->chkReadTorque2->Checked=false;                                                           //wei 20160504 不顯示扭力
                        Pos=MOT[MTestZ1].Gali_ReadEncoderPos();
//                        Pos=Pos-30;
                        break;
                    }
                }

                PosY1=MOT[MTestY1].Gali_ReadPos();                                                                      //JerryYang 20160824 Start:Z1下降前增加防護
                PosY1-=Prod.TestY1_Front;
                if(abs(PosY1)>10)
                {
                    ShowMyMessage("Index Y1 Motor Position error", "馬達Y1位置錯誤!", "Do_Z1_AutoGetHeight 560");
                    return false;
                }

                if(W906_IndexZCommandFloorStop(Pos, fIndexDownPos, "DoZ1PickFromShuttle 550")) { fAllMotorHome=false; CarlibrationTask=1; return false; }                   if(MOT[MTestZ1].Gali_MotMoveNoWait(Pos, GotIndexZSpeed(iSpeedZ*100), 0, false))                         //JimmyChiu 20211028 : All speed can set by speed setting.
                {
                    COM2->InitReadTorueTask();
                    fMain->chkReadTorque1->Checked=true;
                    fMain->chkReadTorque2->Checked=false;
                    fMain->edTorue0->Text="";
                    bReadMCU2=true;                                                                                     //kevin 20220225 read MCU DATA
                    Task=560;
                }
                break;
            case 555:
                if(MOT[MTestZ1].Gali_MotMoveNoWait(Pos, GotIndexZSpeed(iSpeedZ*100), 0, false))                         //JimmyChiu 20211028 : All speed can set by speed setting.
                {
                    Task=570;
                }
                break;
            case 560:
                #ifdef SOFT_SIMULTE
                    Task=570;
                #endif
                if(W906_IndexZTorqueWaitStop(0, fMain->edTorue0->Text=="", "DoZ1PickFromShuttle 560")) { fAllMotorHome=false; CarlibrationTask=1; return false; }   if(fMain->edTorue0->Text=="")   //AI(W906-E042) 20261005 (B3) P5 NOT GOLDEN: 5 s without a torque value -> ST + message + golden 536 exit (golden waits for ever)
                    break;

                iTorque=atoi(fMain->edTorue0->Text.c_str());

                if(INDEX_DRIVER_TYPE==Panasonic_DRIVER)                         //Panasonic     //Steven 20140530
                    iTorque=abs(iTorque);

                ShowMainScreenPresure(0);                                       //kevin 20130605 read Torque send gpib use
                if(iTorque>=iContactKG)
                {
                    COM2->InitReadTorueTask();
                    fMain->chkReadTorque1->Checked=true;
                    fMain->chkReadTorque2->Checked=false;
                    fMain->edTorue0->Text="";
                    bReadMCU2=true;                                             //kevin 20220225 read MCU DATA
                    Counter++;
                    if(Counter>5)
                    {
                        Counter=0;
                        if(bNeedSuck==false)
                        {
                            Task=570;
                            break;
                        }
                    }
                }
                else
                {
                    Counter=0;
                    Task=500;
                }
                break;
            case 570:
                bNeedSuck=true;
                MOT[MTestZ1].Gali_Command("ST", __FUNC__);  //馬達停止
                if(DeviceForm_File.dKitDiameter<=2.5 &&  //JerryYang 20201224 修正20mm SLK auto height高度太低的問題
                   INDEX_PRESS_TYPE!=e85KG)  //Frank 20250214 add
                {
                    Pos=MOT[MTestZ1].Gali_ReadEncoderPos()+PICK_UP_OFFSET+200;
                }
                else
                {
                    Pos=MOT[MTestZ1].Gali_ReadEncoderPos()+PICK_UP_OFFSET;
                }

                if(CUSTOMER_CODE==CC_ASE_KaohSiung)
                    edPickUp1->Text=(Pos-Tech.iTestZ1ShutlePick-200)/100.0;  //kevin 20220831 shuttle pick high -0.2mm
                else
                    edPickUp1->Text=(Pos-Tech.iTestZ1ShutlePick)/100.0;

                if(CUSTOMER_CODE==CC_SIGURD_PeiXing)  //KaiChen 20191111 ：矽格-北興 黃建榮 要求 Contact 頁面中，IC 大小任一邊小於3.5mm，Release Height 設為 1mm
                {
                    if(atoi(edYDimension->Text.c_str())<=3.5 || atoi(edXDimension->Text.c_str())<=3.5)  //jou 2012-10-18 因為常常飛料，&& -> ||，< -> <=
                        edReleaseHeight1->Text=(Pos-Tech.iTestZ1ShutlePick+RELEASE_UP_SMALL)/100.0;
                    else
                        edReleaseHeight1->Text=(Pos-Tech.iTestZ1ShutlePick+RELEASE_UP_BIG)/100.0;
                }
                else if(CUSTOMER_CODE==CC_ASE_KaohSiung)  //kevin 20220831 autohigh IC大小任一邊小於 release high
                {
                    if(atoi(edYDimension->Text.c_str())<5 || atoi(edXDimension->Text.c_str())<5)  //IC <5 up +1 mm
                        edReleaseHeight1->Text=(Pos-Tech.iTestZ1ShutlePick+100)/100.0;
                    else if((atoi(edYDimension->Text.c_str())>=5 && atoi(edYDimension->Text.c_str())<=6.99)||(atoi(edXDimension->Text.c_str())>=5 &&(atoi(edXDimension->Text.c_str())<=6.99)))  //IC <5 up +1 mm
                        edReleaseHeight1->Text=(Pos-Tech.iTestZ1ShutlePick+150)/100.0;
                    else
                        edReleaseHeight1->Text=(Pos-Tech.iTestZ1ShutlePick+RELEASE_UP_BIG)/100.0;
                }
                else if(CosFunction.bAutoHeightSHTReleaseByFile)  //Sam 20200217 : K高後 Shuuttle Release Height offset By SetupFile
                {
                    edReleaseHeight1->Text=((Pos-Tech.iTestZ1ShutlePick)/100.0)+DeviceForm.iAutoHeightSHTReleaseOfs;
                }
                else
                {
                    if(atoi(edYDimension->Text.c_str())<=7 || atoi(edXDimension->Text.c_str())<=7)  //jou 2012-10-18 因為常常飛料，&& -> ||，< -> <=
                        edReleaseHeight1->Text=(Pos-Tech.iTestZ1ShutlePick+RELEASE_UP_SMALL)/100.0;
                    else
                        edReleaseHeight1->Text=(Pos-Tech.iTestZ1ShutlePick+RELEASE_UP_BIG)/100.0;
                }

                InitWriteAndCheckMotorTorqueTask();
                hContactDeley.SetSecAndOn(3);
                ADAM_WriteVoltage(0);

                if(INDEX_SUCKER_TYPE==1)  //jou 2010-05-19 start : 負壓
                    fiosetview->ResetIndexSuck();

                for(int i=0; i<FTestSuck.iShtRow; i++)
                {
                    for(int j=0; j<FTestSuck.iShtCol; j++)
                    {
                        if((CosFunction.bContactTestVacOffByCloseSite==true && LastSet.bUseTestSocket[0][i+iNN][j]) ||  //JerryYang 20160328 for 矽格北興,auto height時,關site的部分不吸取IC
                            CosFunction.bContactTestVacOffByCloseSite==false)
                        {
                            if(INDEX_SUCKER_TYPE==0)
                            {
                                FTestSuck.Suck[i][j].On();
                            }
                            else
                            {
                                fiosetview->bIndexSuck[0][i][j]=true;
                                ct=0;
                                if(IniConfig.bKoreaFunction==true)  //Steven 20150824 : 韓國要求用原本的方式
                                {
                                    do
                                    {
                                        fiosetview->ProcessIndexSuckDestroy1();
                                        MySleepEx(1, true);
                                        ct++;
                                    }while(ct<iSuckDelay);
                                }
                                else
                                {
                                    do
                                    {
                                        bCheckSuck=fiosetview->ProcessIndexSuckDestroy1();
                                        MySleepEx(1, true);
                                        ct++;
                                    }while(bCheckSuck==false);  //jou 2015-05-18 修正 contect mode 中途暫停會發生掉料
                                }
                            }
                        }
                    }
                }
                bProcessIndexSuckDestroy=false;

                Task=600;
                break;
            case 600:
                if(hContactDeley.Off() || bProcessIndexSuckDestroy==true)                                               //jou 2010-05-19 start : 負壓
                {
                    for(int i=0; i<FTestSuck.iShtRow; i++)
                    {
                        for(int j=0; j<FTestSuck.iShtCol; j++)
                        {
                            if(FTestSuck.Suck[i][j].GetStatus())
                            {
                                if(BAR_CODE_INSTALL!=ebctUninstall &&
                                   (TestIF_File.bEnableBarCode ||                                                       //Ifor 20190129 : add Cognex EtherNet 通訊
                                   (BOTTOM_2DID && TestIF_File.bEnableBottom2D)))                                       //Steven 20190422 : Bottom 2DID
                                {
                                    if(chk2DID->Checked==false)                                                         //Steven 20210310 : contact test可以跳過讀取2DID
                                    {
                                        if(iContactMode==CONTACT_DEVICE_MAP_CHECK)                                      //Steven 20210609 : 修正有IC才吸取
                                        {
                                            if(FLCarryKit.Item[i][j]!=NULL_IC)                                          //Steven 20170111 : 避免Auto Height時重複進來被改成NULL_IC
                                                FTestSuck.MoveSuckData(FLCarryKit, i, j);
                                        }
                                        else
                                        {
                                            FTestSuck.SetItemData(i, j, HAS_IC);
                                        }
                                    }
                                    else
                                    {
                                        if(FLCarryKit.Item[i][j]!=NULL_IC)                                              //Steven 20170111 : 避免Auto Height時重複進來被改成NULL_IC
                                            FTestSuck.MoveSuckData(FLCarryKit, i, j);
                                    }
                                }
                                else
                                {
                                    if(iContactMode==CONTACT_DEVICE_MAP_CHECK)                                          //Steven 20210609 : 修正有IC才吸取
                                    {
                                        if(FLCarryKit.Item[i][j]!=NULL_IC)                                              //Steven 20170111 : 避免Auto Height時重複進來被改成NULL_IC
                                            FTestSuck.MoveSuckData(FLCarryKit, i, j);
                                    }
                                    else
                                    {
                                        FTestSuck.SetItemData(i, j, HAS_IC);
                                    }
                                }
                            }

                            if(IniConfig.bChangeKitNoHardStop==true)                                                    //jou 2015-12-08 Xilinx 驗證用    //Frank 20171030 (Steven) add Floating Shuttle Read Torque
                            {
                                if(bSuckFlag1==false)
                                    bSuckFlag1=FTestSuck.Suck[iSuckX1][iSuckY1].GetStatus();
                                if(bSuckFlag2==false)
                                    bSuckFlag2=FTestSuck.Suck[iSuckX2][iSuckY2].GetStatus();
                                if(bSuckFlag1==false && bSuckFlag2==false)
                                {
                                    bUniversalkitflag[0]=true;
                                }
                            }
                        }
                    }

                    if(USE_IO_CHANGE_TOQUE==true)                                                                       //jou 2012-06-21 Enable index I/O Change Toque
                    {
                        SW[SwIndexChangeToque1].Off();
                        SW[SwIndexChangeToque2].Off();
                    }

                    if(DeviceForm_File.dKitDiameter<=2.5 && IniConfig.bKoreaFunction)
                    {
                        Pos=MOT[MTestZ1].Gali_ReadEncoderPos();
                        Pos=Pos+3000;
                        Task=610;
                    }
                    else
                    {
                        ret=COM2->iWriteAndCheckMotorTorque(0, 80);
                        if(ret==1)
                        {
                            MOT[MTestZ1].iGali_SingalHomeTask=1;
                            Task=650;
                        }
                        else if(ret==2)
                        {
                            ShowMyMessage("Motor torque set error", "馬達扭力設定錯誤!", "DoZ1PickFromShuttle 600");
                            fAllMotorHome=false;
                            CarlibrationTask=1;
                            bNeedSuck=false;
                            return false;
                        }
                    }
                }
                break;
            case 610:
                if(MOT[MTestZ1].Gali_MotMoveSkipEncoder(Pos, GotIndexZSpeed(iSpeedZ*1000)))                             //JimmyChiu 20211028 : All speed can set by speed setting.
                {
                    ret=COM2->iWriteAndCheckMotorTorque(0, 80);
                    if(ret==1)
                    {
                        MOT[MTestZ1].iGali_SingalHomeTask=1;
                        Task=650;
                    }
                    else if(ret==2)
                    {
                        ShowMyMessage("Motor torque set error", "馬達扭力設定錯誤!", "DoZ1PickFromShuttle 600");
                        fAllMotorHome=false;
                        CarlibrationTask=1;
                        bNeedSuck=false;
                        return false;
                    }
                }
                break;
            case 650:
                if(MOT[MTestZ1].Gali_SingalHome())
                {
                    Task=700;
                }
                break;
            case 3000:
                if(MOT[MTestZ1].Gali_MotMove(Tech.iTestZ1ShutlePick+iUnitMultiply100(atof(edPickUp1->Text.c_str()))+Offset.iIndexArmPickUp[0], GotIndexZSpeed(iSpeedZ*1000)))  //JimmyChiu 20211028 : All speed can set by speed setting.
                {
                    if(chkDailyCorrelation->Checked)  //KaiHuang 20200724 : For ASE-CL Daily Correlation, Index 沒吸到料需 Alarm
                    {
                        for(int i=0; i<MAX_Index_Row; i++)
                        {
                            for(int j=0; j<NEW_MAX_Index_Col; j++)
                            {
                                bArm1SuckFinish[i][j]=false;  //Steven 20110301 : 初始化，都當作還沒做完
                                bArm1DuplicateErr[i][j]=false;
                                FTestSuck.Suck[i][j].Reset();  //Steven 20140213 : Jordan說Index下去不吸直接Alarm
                            }
                        }
                        Task=660;
                        break;
                    }
                    //if(iContactMode==CONTACT_TEST)                            //jou 2012-05-07 make code"if(iContactMode==CONTACT_TEST)",Auto High unclick "Auto High include Shuttle",index arm不做吸取動作.
                    {
                        if(INDEX_SUCKER_TYPE==1)  //jou 2010-05-19 start : 負壓
                        {
                            fiosetview->ResetIndexSuck();
                            for(int i=0; i<FTestSuck.iShtRow; i++)
                            {
                                for(int j=0; j<FTestSuck.iShtCol; j++)
                                {
                                    if((CosFunction.bContactTestVacOffByCloseSite==true && LastSet.bUseTestSocket[0][i+iNN][j]) ||  //JerryYang 20160328 for 矽格北興,Contact test時關site的部分不吸取IC
                                        CosFunction.bContactTestVacOffByCloseSite==false)
                                    {
                                        fiosetview->bIndexSuck[0][i][j]=true;
                                        ct=0;
                                        if(IniConfig.bKoreaFunction==true)  //Steven 20150824 : 韓國要求用原本的方式
                                        {
                                            do
                                            {
                                                fiosetview->ProcessIndexSuckDestroy1();
                                                MySleepEx(1, true);
                                                ct++;
                                            }while(ct<iSuckDelay);
                                        }
                                        else
                                        {
                                            do
                                            {
                                                if(false /*GATE(W906-E042-D73) AI(W906-E042) 20261005: TfiosetviewShim has no ContactIndexSuckDestroy1 (golden fIoSetView, KYEC fast contact mode); until it is ported the option behaves as OFF = golden's own else branch*/ && IniConfig.bD73ContactModeFast)  //kevin 20220817 : Conttact mode 加速
                                                {
                                                    bCheckSuck=true; /*GATE(W906-E042-D73) bCheckSuck=fiosetview->ContactIndexSuckDestroy1(0, i, j); -- AI(W906-E042) 20261005: unreachable (condition above)*/
                                                }
                                                else
                                                {
                                                    bCheckSuck=fiosetview->ProcessIndexSuckDestroy1();
                                                    MySleepEx(1, true);  //kevin 20220816 mark
                                                }
                                                ct++;
                                            }while(bCheckSuck==false);  //jou 2015-05-18 修正 contect mode 中途暫停會發生掉料
                                        }
                                    }
                                }
                            }
                            hContactDeley.SetSecAndOn(1);  //jou 3 -> 1
                        }
                        else
                        {
                            for(int i=0; i<FTestSuck.iShtRow; i++)
                            {
                                for(int j=0; j<FTestSuck.iShtCol; j++)
                                {
                                    if((CosFunction.bContactTestVacOffByCloseSite==true && LastSet.bUseTestSocket[0][i+iNN][j]) ||  //JerryYang 20160328 for 矽格北興,Contact test時關site的部分不吸取IC
                                        CosFunction.bContactTestVacOffByCloseSite==false)
                                    {
                                        FTestSuck.Suck[i][j].On();
                                    }
                                }
                            }
                            hContactDeley.SetSecAndOn(1);
                        }
                    }
                    bProcessIndexSuckDestroy=false;
                    Task=3050;
                }
                break;
            case 3050:
                if(hContactDeley.Off() || bProcessIndexSuckDestroy==true)
                {
                    //if(iContactMode==CONTACT_TEST)                            //jou 2012-05-07 make code"if(iContactMode==CONTACT_TEST)",Auto High unclick "Auto High include Shuttle",index arm不做吸取動作.
                    {
                        for(int i=0; i<FTestSuck.iShtRow; i++)
                        {
                            for(int j=0; j<FTestSuck.iShtCol; j++)
                            {
                                #ifndef SOFT_SIMULTE
                                if(LastSet.iRealDummy==REALLY && !FTestSuck.Suck[i][j].GetStatus())
                                {
                                    FTestSuck.Suck[i][j].Off();                                                         //Normal -> Off 避免ic被沾黏上來
                                }
                                else
                                #endif
                                {
                                    if(BAR_CODE_INSTALL!=ebctUninstall &&
                                       (TestIF_File.bEnableBarCode ||                                                   //Ifor 20190129 : add Cognex EtherNet 通訊
                                       (BOTTOM_2DID && TestIF_File.bEnableBottom2D)))                                   //Steven 20190422 : Bottom 2DID
                                    {
                                        if(chk2DID->Checked==false)                                                     //Steven 20210310 : contact test可以跳過讀取2DID
                                        {
                                            if(iContactMode==CONTACT_DEVICE_MAP_CHECK)                                  //Steven 20210609 : 修正有IC才吸取
                                            {
                                                if(FLCarryKit.Item[i][j]!=NULL_IC)                                      //Steven 20170111 : 避免Auto Height時重複進來被改成NULL_IC
                                                    FTestSuck.MoveSuckData(FLCarryKit, i, j);
                                            }
                                            else
                                            {
                                                FTestSuck.SetItemData(i, j, HAS_IC);
                                            }
                                        }
                                        else
                                        {
                                            if(FLCarryKit.Item[i][j]!=NULL_IC)                                          //Steven 20170111 : 避免Auto Height時重複進來被改成NULL_IC
                                                FTestSuck.MoveSuckData(FLCarryKit, i, j);
                                        }
                                    }
                                    else
                                    {
                                        if(iContactMode==CONTACT_DEVICE_MAP_CHECK)                                      //Steven 20210609 : 修正有IC才吸取
                                        {
                                            if(FLCarryKit.Item[i][j]!=NULL_IC)                                          //Steven 20170111 : 避免Auto Height時重複進來被改成NULL_IC
                                                FTestSuck.MoveSuckData(FLCarryKit, i, j);
                                        }
                                        else
                                        {
                                            FTestSuck.SetItemData(i, j, HAS_IC);
                                        }
                                    }
                                }
                            }
                        }
                    }
                    //AI(ht9045-v899) 20260421: 全智 Contact/Auto Height 吸取後立即驗證每個應吸取 site 的真空 sensor，掉料/沒吸到時直接停止下壓避免下一刀壓壞 IC 與配件
                    if(CosFunction.bContactTestICDropGuard==true &&
                       (iContactMode==CONTACT_TEST ||
                        iContactMode==AUTO_CONTACT_TEST ||
                        iContactMode==CONTACT_AUTO_GET_HEIGHT))
                    {
                        bool bDropDetected=false;
                        bool bGigasContactTestPartialPick=false;
                        bool bHasGuardSite=false;

                        //AI(ht9045-v899) 20260608: 將全智 partial pick 豁免由僅 CONTACT_TEST 放寬至 AUTO_CONTACT_TEST/CONTACT_AUTO_GET_HEIGHT，保留 CUSTOMER_CODE==CC_GIGAS 前置條件確保只改變全智行為(全智雙Arm Contact掉料防護放寬, CASE-20260608-001)
                        bGigasContactTestPartialPick=(CUSTOMER_CODE==CC_GIGAS &&
                                                      (iContactMode==CONTACT_TEST ||
                                                       iContactMode==AUTO_CONTACT_TEST ||
                                                       iContactMode==CONTACT_AUTO_GET_HEIGHT));
                        //AI(ht9045-v899) 20260608: 全智改用該site IC資料是否==HAS_IC(實際吸取成功)當守護門檻，不再用recipe site開關LastSet.bUseTestSocket；HAS_IC於case3050僅在REALLY且真空sensor ON時SetItemData，代表實際吸到的site，故只檢查這些site有無掉料，其餘site一律不檢查不報錯(全智改用HAS_IC判斷掉料, CASE-20260608-001)
                        for(int i=0; i<FTestSuck.iShtRow; i++)
                        {
                            for(int j=0; j<FTestSuck.iShtCol; j++)
                            {
                                bool bNeedGuard;

                                if(bGigasContactTestPartialPick)
                                    bNeedGuard=(FTestSuck.Item[i][j]==HAS_IC);
                                else
                                    bNeedGuard=(LastSet.bUseTestSocket[0][i+iNN][j]==true);   //AI(ht9045-v899) 20260608: 非全智維持原依recipe site開關檢查行為不變(全智改用HAS_IC判斷掉料, CASE-20260608-001)

                                if(bNeedGuard)
                                {
                                    bHasGuardSite=true;
                                    if(FTestSuck.Suck[i][j].GetStatus()==false)
                                        bDropDetected=true;
                                }
                            }
                        }

                        //AI(ht9045-v899) 20260608: 移除整臂無守護site即強制報掉料的致命fallback，全智允許整臂本輪不上料(部分手動上料)時跳過防護不報掉料，非全智不受影響(全智雙Arm Contact掉料防護放寬, CASE-20260608-001)
                        //(原: if(bGigasContactTestPartialPick && bHasGuardSite==false) bDropDetected=true;)

                        if(bDropDetected==true)
                        {
                            //AI(ht9045-v899) 20260608: 真實掉料中止前將全智本臂(Z1=Front)本次pick暫存資料還原回進站前狀態(iFTestBackItem)，避免殘留IC觸發IndexHasIC()互鎖鎖死contact模式，比照Exit還原寫法且僅全智路徑生效(全智雙Arm Contact掉料防護放寬, CASE-20260608-001)
                            if(bGigasContactTestPartialPick)
                            {
                                for(int ci=0; ci<FTestSuck.iShtRow; ci++)
                                {
                                    for(int cj=0; cj<FTestSuck.iShtCol; cj++)
                                    {
                                        FTestSuck.SetItemData(ci, cj, iFTestBackItem[ci][cj]);
                                    }
                                }
                            }
                            ShowMyMessage("Z1 Contact/Auto Height pick fail or IC drop, stop pressing",
                                          "Z1 Contact/Auto Height 吸取失敗或掉料，已停止下壓以保護 IC 與配件",
                                          "DoZ1PickFromShuttle 3050");
                            fAllMotorHome=false;
                            CarlibrationTask=1;
                            bNeedSuck=false;
                            return false;
                        }
                    }
                    Task=3100;
                }
                break;
            case 3100:
                if(MOT[MTestZ1].Gali_MotMove(Tech.iTestZ1ShutlePick+iUnitMultiply100(atof(edPickUp1->Text.c_str()))+Offset.iIndexArmPickUp[0]+200, GotIndexZSpeed(iSpeedZ*1000)))  //JimmyChiu 20211028 : All speed can set by speed setting.
                {
                    bAlarm=CheckIndexAllSuckICFallDown(true, false);  //Steven 20110725 : 修改負壓檢查方式
                    if(bAlarm==true)
                    {
                        ShowMyMessage("Z1 Pick From Shuttle drop IC ", "Z1吸取Shuttle ic掉落");
                        //AI(ht9045-v899) 20260421: 全智要求掉料時直接中止 Contact/Auto Height，避免下一刀下壓壓壞 IC/配件
                        if(CosFunction.bContactTestICDropGuard==true &&
                           (iContactMode==CONTACT_TEST ||
                            iContactMode==AUTO_CONTACT_TEST ||
                            iContactMode==CONTACT_AUTO_GET_HEIGHT))
                        {
                            fAllMotorHome=false;
                            CarlibrationTask=1;
                            bNeedSuck=false;
                            return false;
                        }
                    }
                    Task=700;
                }
                break;

            case 660:                                                           //KaiHuang 20200724 : For ASE-CL Daily Correlation, Index 沒吸到料需 Alarm
                bArm1NeedSuck=true;                                             //JerryYang 20190123 新增保護避免真空持續on會造成all site掉料
                bArm1SuckComplete=true;

                #ifdef SOFT_SIMULTE
//                    if(CheckBox_Index1PickError->Checked==true)
//                    {
//                        bArm1SuckFinish[0][0]=true;
//                        FTestSuck.Suck[0][0].Error=true;
//                    }
                #endif
                    DoArm1Suck();                                               //JerryYang 20190123 把index arm吸真空&交換狀態包成函式

                //Steven 20110301 : Start
                for(int i=0; i<MAX_Index_Row; i++)
                {
                    for(int j=0; j<NEW_MAX_Index_Col; j++)
                    {
                        if(bArm1SuckFinish[i][j]==false)                        //只要有未完成的就繼續等
                            bArm1SuckComplete=false;
                    }
                }
                //Steven 20110301 : End

                if(bArm1SuckComplete==true)                                     //Steven 20110301 : 所有吸嘴都做完
                {
                    bArm1NeedSuck=false;
                    bHasErr=false;
                    for(int i=0; i<MAX_Index_Row; i++)                          //若有吸取錯誤
                    {
                        for(int j=0; j<NEW_MAX_Index_Col; j++)
                        {
                            if(FTestSuck.Suck[i][j].Error)
                            {
                                bHasErr=true;
                                //jou 2012-06-12 吸shuttle時，需檢測Torque，過大需alarm
//                                if(USE_IO_CHANGE_TOQUE==true)  //jou 2012-06-12 即時更新扭力值不能開，不然會衝突
//                                {
//                                    fMain->CheckBox10->Checked=true;
//                                    fMain->CheckBox11->Checked=false;
//                                    fMain->edTorue0->Text="";
//                                    bOverHappen=false;
//                                    hDoFrontTestSuckIC.SetSecAndOn(1);
//                                }

//                                if(IniConfig.bD62PickUpErrorNeedPurge)                  //Steveb 20161024 : 吸取異常需要吹氣一次
//                                {
//                                    FTestSuck.Suck[i][j].Off();
//                                }
                            }
                        }
                    }

                    if(bHasErr)
                    {
                        bIndexPickUpErrMoveSht1=true;                           //Steven 20171221 (Wei) : 修正[D43]當蝦頭退出來要回去前,如果In Arm補了HAS_NULL_IC在蝦頭上會造Hang up
                        Task=665;
                        return false;
                    }

                    if(FLCarryKit.HasRealIC())
                        break;

//                    if(bAutoSiteMapWaitTestResult==true && FTestSuck.HasRealIC()==false)    //Steven 20200326 : 修正JCET Auto site map發生inarm掉料會hang up
//                    {
//                        bAutoSiteMapWaitTestResult=false;
//                    }

                    ZeroMemory(bArm1DuplicateErr, sizeof(bArm1DuplicateErr));
                    if(FLCarryKit.HasIC())
                        break;

                    bResetIndexArm1Pick=false;
//                    if(CUSTOMER_CODE==CC_TSMC_TAINAN && (Prod.bWhenNoFullSiteUseInitialDelay && IniConfig.bL18NofullsiteaddTemperatureoffset))     //wei 20151228 No FullSite delay    //wei 20161102 No FullSite delay修改||->&&
//                        CheckFTFullSite();
                    Task=3100;
                    iD43AutoRetryWhenIndexPickErrCnt[0]=0;                      //Steven 20170105 : Index吸取異常,要退出來用Shuttle Sensor檢查後, 再進去吸一次
                }
                break;
            case 665:                                                           //KaiHuang 20200724 : For ASE-CL Daily Correlation, Index 沒吸到料需 Alarm
                CheckIndexAllSuckICFallDown(true, false);                       //Steven 20110725 : 修改負壓檢查方式

                if(MOT[MTestZ1].Gali_ReadPos()>(Prod.TestZ1_Pick+1000))         //Steven 20150407 : 修正[D45] Out Arm等Index Z功能, 避免Auto Homing
                {
                    bZ1PickShuttle=false;
                }

                #ifdef DEBUG_INDEX_UPH
                if(MOT[MTestZ1].Gali_MotMove2(Prod.TestZ1_Safe, iIndexSpeed, iIndexAcc))
                #else
                if(MOT[MTestZ1].Gali_MotMoveNoWait(Prod.TestZ1_Safe, MOT[MTestZ1].GailSpeed, 0))
                #endif
                {
                    Task=670;                                                   //Steven 20160718 : Index pick up error with [D43]
                }
                break;
            case 670:
                bZ1PickShuttle=false;                                           //Steven 20150407 : 修正[D45] Out Arm等Index Z功能, 避免Auto Homing
//                if(IniConfig.bD43IndexDropErrorCanRetryandSkip)
//                {
//                    if(FRCarryKit.NoIC()) //ChungHung 20120717 add Index Drop Error Can Retry and Start
//                    {
//                        MOT[MInShuttle1].fCanMoveM=true;
//                        bShuttle1MoveToLeft=true;
//                        bIndexPickErrShtStayRight1=false;   //JerryYang 20181206 (Steven) : fix 啟用D43功能時,index arm pick up error後按retry可能發生hang up
//                        bCheckNullIC1=false;      //JerryYang 20170623 (wei) 修正有裝out shuttle 前後對照的機台發生index arm吸取異常無法跳出alram造成hang up
//                        Task=320;
//                    }
//                }
//                else
                {
                    if(bPlaceToShuttle2Step)                                    //Steven 20160718 : 避免放蝦頭放到一半讓位會死雞
                        return false;
                    Task=675;
                }
                break;
            case 675:                                                                                                   //等Shuttle移出來
                //ChungHung 20120717 add Index Drop Error Can Retry and Start
                if(IniConfig.bD43IndexDropErrorCanRetryandSkip && bShuttle1MoveToLeft)                                  //ChungHung 20131015 fix hangup
                {
                    if(InSHT1InLF()!=true)
                    {
                        return false;
                    }
                }
                bShuttle1MoveToLeft=false;
//                MOT[MInShuttle1].fCanMoveM=false;
                Task=680;

                if(TestIF.iTestMode==_32Site4X8N || TestIF.iTestMode==_16Site4X4)                                       //Sam 20190226 : 16Site4X4 //Steven 20150901
                    break;
            case 680:                                                                                                                                           //ChungHung 20130924 add
                //Steven 20110131 Start
                ErrPart=" ";
                bHasErr=false;
                bHasDuplicateErr=false;
                for(int i=0; i<MAX_Index_Row; i++)
                {
                    for(int j=0; j<NEW_MAX_Index_Col; j++)
                    {
                        if(bArm1DuplicateErr[i][j])
                            bHasDuplicateErr=true;

                        if(FTestSuck.Suck[i][j].Error)
                        {
                            bHasErr=true;
                            if(TestIF.iTestMode==_32Site4X8N || TestIF.iTestMode==_16Site4X4)                                                                   //Sam 20190226 : 16Site4X4 ///kevin 20180504 add  error pos
                                ErrPart+=IndexSuckName[i+2][j];
                            else
                                ErrPart+=IndexSuckName[i][j];
                            if(IniConfig.bD62PickUpErrorNeedPurge)                                                                                              //Steveb 20161024 : 吸取異常需要吹氣一次
                            {
                                FTestSuck.Suck[i][j].Normal();
                            }
                            FLCarryKit.PordRec[i][j].AddErrorRecordNoSave("JAM0301");
                        }
                        else
                        {
                            FTestSuck.Suck[i][j].Error=false;
                        }
                    }
                }

                if(bHasErr)
                {
                    if(IndexAlarmInArmAway()==false)                                                                                                            //Steven 20130613 : Index異常時, In Arm要先讓位功能
                    {
                        return false;
                    }

                    bHasErr=false;
                    if(IniConfig.bD43AutoRetryWhenIndexPickErr &&                                                                                               //Steven 20170105 : Index吸取異常,要退出來用Shuttle Sensor檢查後, 再進去吸一次
                       iD43AutoRetryWhenIndexPickErrCnt[0]==0)
                    {
                        ret=K_RETRY;
                    }
                    else if(IniConfig.bNewResetFunction==true && bResetIndexArm1Pick==true)
                    {
                        ret=K_SKIP;
                    }
                    else
                    {
                        if(CosFunction.bJAM0301NeedOpenChamberDoor)                                                                                             //wei : JAM0301 & JAM0302需要開啟Chamber門10秒
                        {
                            bIsTestSitICFallDown=true;
                        }

//                        if(IniConfig.bIndexPickErrOnlySKIP==true || IniConfig.bD64IndexPickErrOnlySKIP)//kevin 20171103 (wei) add retry function     //jou 2012-02-13 index pick-up error only skip
//                            ret=ShowErrorMessage("JAM0301", K_SKIP, MTestZ1, bHasDuplicateErr, ErrPart); //Devicr Pick-Up Error
//                        else
//                            ret=ShowErrorMessage("JAM0301", K_SKIP|K_RETRY, MTestZ1, bHasDuplicateErr, ErrPart); //Devicr Pick-Up Error
                        ret=ShowErrorMessage("JAM0301", K_RETRY, MTestZ1, bHasDuplicateErr, ErrPart);                                                           //Devicr Pick-Up Error
                    }

                    if(ret==K_SKIP)
                    {
                        bAutoSiteMapWaitTestResult=false;                                                                                                       //Ifor 20180115 (Steven) : add Site Mapping SKIP 需清除旗標
                        bIndexPickUpErrMoveSht1=false;                                                                                                          //Steven 20171221 (Wei) : 修正[D43]當蝦頭退出來要回去前,如果In Arm補了HAS_NULL_IC在蝦頭上會造Hang up
                        iD43AutoRetryWhenIndexPickErrCnt[0]=0;                                                                                                  //Steven 20170105 : Index吸取異常,要退出來用Shuttle Sensor檢查後, 再進去吸一次

                        for(int i=0; i<FTestSuck.iShtRow; i++)
                        {
                            for(int j=0; j<FTestSuck.iShtCol; j++)
                            {
    //                            FLCarryKit.PordRec[i][j].AddErrorRecord("JAM0301");
                                //jou 2011-12-27 有發生過Skip又重吸一次,改成下面的方式
                                if(FTestSuck.Item[i][j]==NULL_IC)
                                {
                                    if(FTestSuck.Suck[i][j].Error)                                                                                              //JerryYang 20200303 fix 沒IC的地方jam count被++
                                    {
//                                        if(CosFunction.bUseSCKART)                              //Steven 20161214 (wei) : For SCK ART
//                                            fSCKART->iOutputJamCnt++;
                                    }
                                    FLCarryKit.PordRec[i][j].AddErrorRecord("JAM0301");                                                                         //Steven 20161214 : 加上Index異常Skip的ErrorLog
                                    FTestSuck.SetItemData(i, j, HAS_NULL_IC);
                                    FTestSuck.Suck[i][j].Normal();
                                    if(CosFunction.bIndexPickErrSkipNeedCheckVac==true && IniConfig.bD50IndexPickErrSkipNeedCheckVac)                           //JerryYang 20170610 (wei) JSCC要求index pick up error 需再慢速下降吸一次
                                    {
                                        bSkipNeedCheckVac[0][i][j]=true;
                                        bArm1PressSkipNeedDownCheckVac=true;
                                    }
                                }
                                FLCarryKit.SetItemData(i, j, NULL_IC);

                                bArm1DuplicateErr[i][j]=false;
                            }
                        }

                        if(IniConfig.bNewResetFunction==true && bResetIndexArm1Pick==true)
                        {
                        }
                        else
                        {
                            //ChungHung 20110302 start
                            if(IniConfig.bD42IndexPickICShuttlePause)
                            {
                                bInArmNeedToSafePos=true;
                                bShuttle1Pause=true;
                                bIndexArm1PickupErrStop=true;                                                                                                   //jou 2012-02-29 index pick up error,index arm move to center & alarm
                                bShowShuttle1Device=true;                                                                                                       //kevin 20180504 index pick up error
                            }
                            //ChungHung 20110302 end
                        }
                        bIndexPickUpErrorWaitRetry=false;                                                                                                       //Ifor 20171119 (Steven) : add 避免Index Pick Up Err Inarm 偷跑造成資料異常導致Hangup
                        if(IniConfig.bD43IndexDropErrorCanRetryandSkip)                                                                                         //JerryYang 20181206 (Steven) : fix 啟用D43功能時,index arm pick up error後按retry可能發生hang up
                        {
                            bIndexPickErrShtStayRight1=false;
                        }

                        if(IniConfig.bD43IndexPickErrCheckSocket==true)                                                                                         //Steven 20190115 : SCC要求吸取異常要檢查Socket
                        {
                            bIndexArm1PickUpErrNeedPiggyback=true;
                            fMain->ResetRecordforPiggyBack("RESET_ForIndexPickUpErr");
                            iWhoTriggerPiggyBack=pbtIndexArmPickUpErr;
                            ProcessPiggyBackFunction();
                        }
                    }
                    else
                    {
                        if(IniConfig.bD43AutoRetryWhenIndexPickErr &&                                                                                           //Steven 20170105 : Index吸取異常,要退出來用Shuttle Sensor檢查後, 再進去吸一次
                           iD43AutoRetryWhenIndexPickErrCnt[0]==0)
                        {
                            iD43AutoRetryWhenIndexPickErrCnt[0]++;
                        }
                        else
                        {
                            iD43AutoRetryWhenIndexPickErrCnt[0]=0;                                                                                              //Steven 20170105 : Index吸取異常,要退出來用Shuttle Sensor檢查後, 再進去吸一次
                            for(int i=0; i<MAX_Index_Row; i++)
                                for(int j=0; j<NEW_MAX_Index_Col; j++)
                                    if(FTestSuck.Suck[i][j].Error)
                                        bArm1DuplicateErr[i][j]=true;
                        }
                        bIndexPickUpErrorWaitRetry=true;                                                                                                        //Ifor 20171119 (Steven) : add 避免Index Pick Up Err Inarm 偷跑造成資料異常導致Hangup
                        if(IniConfig.bD43IndexDropErrorCanRetryandSkip)                                                                                         //JerryYang 20181206 (Steven) : fix 啟用D43功能時,index arm pick up error後按retry可能發生hang up
                        {
                            bIndexPickErrShtStayRight1=true;
                        }
                        bIndexPickUpErrorWaitRetry=true;                                                                                                        //Ifor 20171119 : add 避免Index Pick Up Err Inarm 偷跑造成資料異常導致Hangup
                    }

                    FTestSuck.ResetAll();                                                                                                                       //Steven 20160323 : 避免未開啟真空
                }
                //Steven 20110131 End

                if(IniConfig.bNewResetFunction==true && bResetIndexArm1Pick==true)
                {
                    bResetIndexArm1Pick=false;
                }
                else
                {
                    //ChungHung 20120717 add Index Drop Error Can Retry and Start
                    if(IniConfig.bD43IndexDropErrorCanRetryandSkip)
                    {
                        //等待Shuttle 移至右邊
                        MOT[MInShuttle1].fCanMoveM=true;
                        bShuttle1MoveToRight=true;
                    }
                }

                if(bShuttle1Pause)
                    MOT[MInShuttle1].SetSpeed(10);                                                                                                              //kevin 20180226 (Steven) add pick up error  shuttle down speed

                if(FLCarryKit.HasRealIC())
                {
                    Task=3000;
                }
                else if(CosFunction.bIndexPickErrSkipNeedCheckVac==true && IniConfig.bD50IndexPickErrSkipNeedCheckVac && bArm1PressSkipNeedDownCheckVac==true)  //JerryYang 20170610 (wei) JSCC要求index pick up error 需再慢速下降吸一次
                {
                    Task=3000;
                }
                else
                {
                    for(int i=0; i<FTestSuck.iShtRow; i++)
                    {
                        for(int j=0; j<FTestSuck.iShtCol; j++)
                        {
                            if(FLCarryKit.Item[i][j])
                            {
                                //ChungHung 20120911 add
                                if(FLCarryKit.Item[i][j]==HAS_NULL_IC && FTestSuck.Item[i][j]!=NULL_IC)
                                    FLCarryKit.SetItemData(i, j, NULL_IC);
                                if(FLCarryKit.Item[i][j]==HAS_NULL_IC)
                                {
                                    FTestSuck.Suck[i][j].Normal();
                                    FTestSuck.MoveSuckData(FLCarryKit, i, j);
                                    bArm1DuplicateErr[i][j]=false;
                                }
                            }
                        }
                    }

                    if(FLCarryKit.HasIC())
                    {
                        Task=3000;
                        break;
                    }
                    Task=700;
                }
                break;
            case 700:
                if(MOT[MTestZ1].Gali_Two_ZAxis_Move(Prod.TestZ1_Safe, GotIndexZSpeed(iSpeedZ*1000), "DoZ1PickFromShuttle 700"))                                 //JimmyChiu 20211028 : All speed can set by speed setting.
                {
                    ADAM_WriteVoltage(DeviceForm.dPress);

                    #ifndef SOFT_SIMULTE
                    if(LastSet.iRealDummy==REALLY)                                                                                                              //jou 2012-05-07 Auto High unclick "Auto High include Shuttle",index arm不做吸取動作.
                    {
                        for(int i=0; i<FTestSuck.iShtRow; i++)
                        {
                            for(int j=0; j<FTestSuck.iShtCol; j++)
                            {
                                if(!FTestSuck.Suck[i][j].GetStatus() && FTestSuck.Suck[i][j].Status==false)
                                    FTestSuck.Suck[i][j].Normal();
                            }
                        }
                    }
                    #endif

                    if(iContactMode==CONTACT_AUTO_GET_HEIGHT && bAutoShuttle==false)                                                                            // 自動 K
                    {
                        InitWriteAndCheckMotorTorqueTask();
                        Task=800;
                        break;
                    }
                    else
                    {
                        if(USE_IO_CHANGE_TOQUE==true)                                                                                                           //jou 2012-06-21 Enable index I/O Change Toque
                        {
                            SW[SwIndexChangeToque1].Off();
                            SW[SwIndexChangeToque2].Off();
                        }
                        Task=1;
                        return true;
                    }
                }
                break;
            case 800:
                if(USE_IO_CHANGE_TOQUE==true)                                   //jou 2012-06-21 Enable index I/O Change Toque
                {
                    SW[SwIndexChangeToque1].Off();
                    SW[SwIndexChangeToque2].Off();
                }

                ret=COM2->iWriteAndCheckMotorTorque(0, 300);
                if(ret==1)
                {
                    Task=1;
                    return true;
                }
                else if(ret==2)
                {
                    ShowMyMessage("Index Z1 Motor torque set error", "馬達扭力設定錯誤!", "DoZ1PickFromShuttle 800");
                    fAllMotorHome=false;
                    CarlibrationTask=1;
                    bNeedSuck=false;
                    return false;
                }
                break;
        }
        return false;
    }
}
//---------------------------------------------------------------------------
bool TfContact::DoZ2PickFromShuttle()   //AI(W906-E042) 20261005 (B3): __fastcall dropped
{
//    #ifdef SOFT_SIMULTE
//        return true;
//    #else
    {
        static int Counter=0, Pos=0, iStepSpeed=0;
        static int  iSuckX1=-1, iSuckY1=-1, iSuckX2=-1, iSuckY2=-1;
        static bool flag1=false, bNeedSuck=false, bAutoShuttle=false, suckjam=true;
        static bool bFlagSh2;
        static bool bSuckFlag1=false, bSuckFlag2=false;
        static bool bProcessIndexSuckDestroy=false;                             //jou 2010-05-19 start : 負壓

        int iContactKG=CONTECT_SHUTTLE_KG;
        int &Task=Z_Height_Task, ret=0, iTorque=0, iPosCheck=0, ct=0, PosY2=0;
        int iRow=0, iCol=0;
        bool bf1, bf2, bf3, bf4, bf5, bAlarm=true, bCheckSuck=false;

        AnsiString ErrPart="";                                                  //KaiHuang 20200724 : For ASE-CL Daily Correlation, Index 沒吸到料需 Alarm
        bool bHasErr=false;                                                     //KaiHuang 20200724 : For ASE-CL Daily Correlation, Index 沒吸到料需 Alarm
        bool bHasDuplicateErr=false;

        if(INDEX_SUCKER_TYPE==1)
        {
            if(bProcessIndexSuckDestroy==false)
                bProcessIndexSuckDestroy=fiosetview->ProcessIndexSuckDestroy2();
        }

        if(W906_IndexZDriveFaultStop("DoZ2PickFromShuttle")) { fAllMotorHome=false; CarlibrationTask=1; return false; }   switch(Task)   //AI(W906-E042) 20261005 (B3) P7 NOT GOLDEN: M14 drive alarm / ERROR_STOP / servo off / monitor lost or frozen / route fault -> ST + message + golden 536 exit; PCI1203 Z1 SHIP only, no-op elsewhere
        {
            case 1:
                if(MOT[MTestZ1].Gali_Two_ZAxis_Move(Prod.TestZ1_Safe, GotIndexZSpeed(iSpeedZ*1000), "DoZ2PickFromShuttle 1"))                                   //一次移動二軸     //JimmyChiu 20211028 : All speed can set by speed setting.
                {
                    if(iContactMode==CONTACT_AUTO_GET_HEIGHT &&                                                                                                 //ChungHung 20130829 解mark
                       IniConfig.bD12UseDeviceFormPressDoShtHeight==false)                                                                                      //Steven 20140220 : 用生產ep去做蝦頭auto high
                    {
                        ADAM_WriteMaxData(true);                                                                                                                //Steven 20241014 : 整合auto height輸出壓力
                    }
                    else
                    {
                        ADAM_WriteVoltage(DeviceForm.dPress);
                    }

                    if(chk_K_Temperature->Checked==true)                                                                                                        //Ztex 2024.03.26 Add Contact Mode K Temperature
                        return true;
                    Task=100;
                }
                break;
            case 100:  //Steven 20120214 Start: 避免吸嘴上有IC
                for(int i=0; i<BTestSuck.iShtRow; i++)
                {
                    for(int j=0; j<BTestSuck.iShtCol; j++)
                    {
                        if(INDEX_SUCKER_TYPE==0)
                        {
                            if(TestIF_File.bArm1PickPlaceArm2Test==false ||
                               ((IniConfig.bD58UseArm1PickPlaceArm2Test==true && TestIF_File.bArm1PickPlaceArm2Test==true) && TestIF_File.bCheckArm2Vacuum==true))  //Steven 20150129 : 需要確認Arm2有沒有粘料    //Ifor 20200811 Fix: Arm1 Pick Place Arm2Test 需卡兩個條件
                            {
                                BTestSuck.Suck[i][j].On();
                            }
                        }
                        else
                        {
                            if(IniConfig.bSIGURDFunction)  //Sam 20221019 : 矽格北興 Contact 模式初始檢查吸嘴上有 IC 就報警。
                            {
                                                                                //不要將 bIndexSuck = true 不然 Alarm 後上面若有 IC 會掉下來
                            }
                            else
                            {
                                if(TestIF_File.bArm1PickPlaceArm2Test==false ||
                                   ((IniConfig.bD58UseArm1PickPlaceArm2Test==true && TestIF_File.bArm1PickPlaceArm2Test==true) && TestIF_File.bCheckArm2Vacuum==true))  //Steven 20150129 : 需要確認Arm2有沒有粘料    //Ifor 20200811 Fix: Arm1 Pick Place Arm2Test 需卡兩個條件
                                {
                                    fiosetview->bIndexSuck[1][i][j]=true;
                                }
                            }
                        }
                    }
                }
                DoZ2PickFromShuttleDelay.SetSecAndOn(1);
                Task=110;
                break;
            case 110:
                if(DoZ2PickFromShuttleDelay.Off())
                {
                    if(EP_Install==3)                                                                                                                           //JerryYang 20240111 : add
                    {
                        if(IniConfig.bD24EnableEPCheckFuntion==true)
                        {
                            ADAM_Rang(IniConfig.iD26EPEncoderRange);
                            if(ADAM_Alarm()==true)
                            {
                                ret=ShowErrorMessage("WAR1605", K_RETRY|K_SKIP, 0, MMSystem, "fContact::DoZ2PickFromShuttle");                                  //"請檢查EP是否漏氣!"
                                if(ret==K_RETRY)
                                {
                                    return false;
                                }
                            }
                        }
                    }

                    if(CheckIndexArmStatus(1))
                    {
                        Task=100;
                    }
                    else
                    {
                        Task=120;
                    }
                }
                break;
            case 120:
                if(INDEX_SUCKER_TYPE==0)
                {
                    for(int i=0; i<BTestSuck.iShtRow; i++)
                    {
                        for(int j=0; j<BTestSuck.iShtCol; j++)
                        {
                            if(TestIF_File.bArm1PickPlaceArm2Test==false ||
                               ((IniConfig.bD58UseArm1PickPlaceArm2Test==true && TestIF_File.bArm1PickPlaceArm2Test==true) && TestIF_File.bCheckArm2Vacuum==true))  //Steven 20150129 : 需要確認Arm2有沒有粘料    //Ifor 20200811 Fix: Arm1 Pick Place Arm2Test 需卡兩個條件
                            {
                                BTestSuck.Suck[i][j].Normal();
                            }
                        }
                    }
                }
                DoZ2PickFromShuttleDelay.SetSecAndOn(1);
                Task=130;
                break;
            case 130:
                if(DoZ2PickFromShuttleDelay.Off())
                {
                    if(BAR_CODE_INSTALL!=ebctUninstall &&
                       TestIF_File.bEnableBarCode &&
                       chkDailyCorrelation->Checked==false)                     //Steven 20121023 : BarCode_2D
                    {
                        if(BOTTOM_2DID && TestIF_File.bEnableBottom2D)          //Steven 20190308 : Bottom 2D
                        {
                            if(iContactMode!=CONTACT_DEVICE_MAP_CHECK)          //Steven 20190910 : Qualcomm功能
                            {
                                for(int i=0; i<BTestSuck.iShtRow; i++)
                                {
                                    for(int j=0; j<BTestSuck.iShtCol; j++)
                                    {
                                        if(TestIF_File.iSiteMap[i][j]>0)        //Steven 20170302 (wei) : 確認哪個Site有開, 從1開始~32
                                            BLCarryKit.SetItemData(i, j, HAS_IC);
                                    }
                                }
                                bSetHasIC=true;                                 //Steven 20190626 : Fixed 2DID error in contact test
                            }
                            Task=150;
                        }
                        else
                        {
                            if(chk2DID->Checked)                                //Steven 20210310 : contact test可以跳過讀取2DID
                            {
                                Task=140;
                            }
                            else
                            {
                                Task=150;
                            }
                        }
                    }
                    else
                    {
                        Task=150;
                    }
                }
                break;
            case 140:
                if(iContactMode!=CONTACT_DEVICE_MAP_CHECK)                      //Steven 20190910 : Qualcomm功能
                {
                    bSetHasIC=true;                                             //Steven 20190626 : Fixed 2DID error in contact test
                    for(int i=0; i<BTestSuck.iShtRow; i++)
                    {
                        for(int j=0; j<BTestSuck.iShtCol; j++)
                        {
                            if(TestIF_File.iSiteMap[i][j]>0)                    //Steven 20170302 (wei) : 確認哪個Site有開, 從1開始~32
                                BLCarryKit.SetItemData(i, j, HAS_IC);
                        }
                    }
                }
                Task=142;
                break;
            case 142:
                bFlagSh2=false;
                fBarCode->InitialBarcodeScanInShuttle2();
                {} /*GATE(W906-E042-BCE) fBarCode->CleanBarcodeError(2); -- AI(W906-E042) 20261005: TfBarCode has no CleanBarcodeError (golden Steven 20260421, clears barcode error flags after the shuttle scan); not ported -- must be before B6 on barcode machines*/             //Steven 20260421 : Clean error flags to prevent false alarm in Contact/AutoHeight flow
                Task=143;
                break;
            case 143:
                if(bFlagSh2==false)
                {
                    if(TestIF_File.iShuttleMode==1 && TestIF_File.iShuttle_Sel==0)
                    {
                        bFlagSh2=true;
                    }
                    else if(BAR_CODE_INSTALL==ebctUseCCDMode &&                 //Steven 20160106 : 改用CCD拍完就跑的方式
                            TestIF_File.bEnableBottom2D==false)                 //Steven 20190308 : Bottom 2D
                    {
                        if(CosFunction.b2DUseSubJobFunction==true && TestIF_File.b2DUseSubJob==true|| //Ifor 20200807 add:In House 2D Use Sub Job Function
                           (CosFunction.b2DUsePinInspection==true && TestIF_File.b2DUsePinInspection==true)) //Eastsun 20260515 F022: D2 add Pin Inspection
                        {
                            bFlagSh2=fBarCode->DoBarcodeScanInShuttle_2(true);  //Eastsun 20260327 KYEC pin1要求Contact 可以skip 正常做不能skip
                        }
                        else
                        {
                            bFlagSh2=fBarCode->DoBarcodeCCDInShuttle_2();
                        }
                    }
                    else if(TestIF_File.b2DTriggerMode)                         //Steven 20151225 : 改用拍完就跑的方式
                    {
                        bFlagSh2=fBarCode->DoBarcodeTriggerInShuttle_2();
                    }
                    else
                    {
                        bFlagSh2=fBarCode->DoBarcodeScanInShuttle_2(true);      //Eastsun 20260327 KYEC pin1要求Contact 可以skip 正常做不能skip
                    }
                }

                if(bFlagSh2)
                {
                    Task=150;
                }
                break;
            case 150:                                                                                                                                           //Steven 20120214 End: 避免吸嘴上有IC
                if(IniConfig.bF21InOutArmZMotorPrivate)
                {
                    if(DoInOutARM_SHT_MoveSafe(0))                                                                                                              //kevin 20161005 SHUTTLE 1 移動安全保護
                        return false;
                    if(DoInOutARM_SHT_MoveSafe(1))                                                                                                              //kevin 20161005 SHUTTLE 1 移動安全保護
                        return false;
                }
                bf1=MOT[MInShuttle1].MotorMove(Prod.InSHT[0].iRight);
                bf2=MOT[MInShuttle2].MotorMove(Prod.InSHT[1].iRight);
                bf4=true;
                bf5=true;
                bf3=MOT[MTestY1].GalilTwoY_Move(Prod.TestY1_Middle, Prod.TestY2_Rear, GotIndexYSpeed(iSpeed*3000), "DoZ2PickFromShuttle 150");                  //JimmyChiu 20211028 : All speed can set by speed setting.

                if(bf1 && bf2 && bf3 && bf4 && bf5)
                    Task=200;
                break;
            case 200:
                if(INDEX_SUCKER_TYPE==1)                                                                                //jou 2010-05-19 start : 負壓
                    fiosetview->ResetIndexSuck();

                for(int i=0; i<BTestSuck.iShtRow; i++)
                {
                    for(int j=0; j<BTestSuck.iShtCol; j++)
                    {
                        if(INDEX_SUCKER_TYPE==0)
                            BTestSuck.Suck[i][j].On();
                        else
                            fiosetview->bIndexSuck[1][i][j]=true;
                    }
                }

                if(INDEX_SUCKER_TYPE==1)
                    bProcessIndexSuckDestroy=false;
                else
                    bProcessIndexSuckDestroy=true;

                bAutoShuttle=false;
                if(iContactMode==CONTACT_AUTO_GET_HEIGHT && chkShuttle->Checked)                                        //auto hight
                {
                    bAutoShuttle=true;
                    InitWriteAndCheckMotorTorqueTask();
                    bNeedSuck=false;
                    Task=300;
                }
                else
                {
                    if(iContactMode==CONTACT_TEST ||
                       iContactMode==CONTACT_DEVICE_MAP_CHECK)                                                          //Steven 20191009 : Device Map for Qualcomm
                    {
                        Task=250;
                        if(IniConfig.bD73ContactModeFast)                                                               //kevin 20220817 : Conttact mode 加速
                        {
                           hContactDeley.SetSecAndOn(1);
                        }
                        else if(INDEX_SUCKER_TYPE==1)
                        {
                           hContactDeley.SetSecAndOn(3);
                        }
                        else
                        {
                            hContactDeley.SetSecAndOn(1);
                        }
                    }
                    else
                    {
                        Task=3000;
                    }
                }
                break;
            case 250:
                if(hContactDeley.Off() && bProcessIndexSuckDestroy==true)
                {
                    Task=400;
                    suckjam=true;                                               //JerryYang 20231002 : 修正index arm上有IC沒有跳ALARM就把Device吹掉的問題
                }
                break;
            case 300:
                if(USE_IO_CHANGE_TOQUE==true)                                   //jou 2012-06-21 Enable index I/O Change Toque
                {
                    SW[SwIndexChangeToque1].Off();
                    SW[SwIndexChangeToque2].Off();
                }

                ret=COM2->iWriteAndCheckMotorTorque(1, iContactKG);
                if(ret==1)
                {
                    Task=400;
                    Counter=0;
                    iStepSpeed=30000;
                    suckjam=true;
                }
                else if(ret==2)
                {
                    ShowMyMessage("Motor torque set error", "馬達扭力設定錯誤!", "DoZ2PickFromShuttle 300");
                    fAllMotorHome=false;
                    CarlibrationTask=1;
                    bNeedSuck=false;
                    return false;
                }
                break;
            case 400:
                flag1=false;
                bUniversalkitflag[1]=false;

                for(int i=0; i<BTestSuck.iShtRow; i++)
                {
                    for(int j=0; j<BTestSuck.iShtCol; j++)
                    {
                        if(BTestSuck.Suck[i][j].GetStatus())
                            flag1=true;
                    }
                }

                if(suckjam==true && flag1==true)                                // sucker already on at high position
                {
                    ShowMyMessage("Maybe Z2 sucker sensor initial on error", "Index Z2的真空產生器的Sensor在初始化時被開啟!", "DoZ2PickFromShuttle 400");
                    fAllMotorHome=false;
                    CarlibrationTask=1;
                    bNeedSuck=false;
                    return false;
                }
                suckjam=false;

                for(int i=0; i<BTestSuck.iShtRow; i++)
                {
                    for(int j=0; j<BTestSuck.iShtCol; j++)
                    {
                        BTestSuck.Suck[i][j].Normal();
                    }
                }
                Pos=0;
                Counter=0;

                if(iContactMode==CONTACT_TEST ||
                   iContactMode==CONTACT_DEVICE_MAP_CHECK)                      //Steven 20191009 : Device Map for Qualcomm
                {
                    if(USE_IO_CHANGE_TOQUE==true)                               //jou 2012-06-21 Enable index I/O Change Toque
                    {
                        SW[SwIndexChangeToque1].Off();
                        SW[SwIndexChangeToque2].On();
                    }
                    Task=3000;
                }
                else
                {
                    if(IniConfig.bChangeKitNoHardStop==true)                    //jou 2015-12-08 Xilinx 驗證用
                    {
                        bSuckFlag1=false;
                        bSuckFlag2=false;

                        for(int i=0; i<BTestSuck.iShtRow; i++)
                        {
                            for(int j=1; j<BTestSuck.iShtCol; j++)
                            {
                                if(bSuckFlag1==false)
                                {
                                    if(LastSet.bUseTestSocket[1][i][j]==true)
                                    {
                                        BTestSuck.Suck[i][j].On();
                                        bSuckFlag1=true;
                                        iSuckX1=i;
                                        iSuckY1=j;
                                    }
                                }

                                if(bSuckFlag2==false)
                                {
                                    iRow=BTestSuck.iShtRow-i-1;
                                    iCol=BTestSuck.iShtCol-j-1;
                                    if(iRow>=0 &&
                                       iCol>=0 &&
                                       LastSet.bUseTestSocket[1][iRow][iCol]==true)
                                    {
                                        BTestSuck.Suck[iRow][iCol].On();
                                        bSuckFlag2=true;
                                        iSuckX2=iRow;
                                        iSuckY2=iCol;
                                    }
                                }
                            }
                        }
                        bSuckFlag1=false;
                        bSuckFlag2=false;
                    }
                    Task=500;
                }
                break;
            case 500:
                if(CUSTOMER_CODE==CC_AMKOR_China ||
                   CUSTOMER_CODE==CC_QUALCOMM)                                  //JerryYang 20170412 (Steven) add QUALCOMM
                    Pos-=100;
                else
                    Pos-=300;
                Task=550;

                iPosCheck=MOT[MTestZ2].Gali_ReadEncoderPos();
                if(iPosCheck<(Tech.iTestZ2ShutlePick-1000))
                {
                    if(bNeedSuck==false)
                    {
                        ShowMyMessage("Index arm Z2 to shuttle height error!", "Please check teaching about index arm 2 to shuttle Z position.", "DoZ2PickFromShuttle 500");
                        Task=570;
                        break;
                    }
                }
                break;
            case 550:
                if(IniConfig.bChangeKitNoHardStop==true)                                                                //jou 2015-12-08 Xilinx 驗證用
                {
                    if(bSuckFlag1==false)
                    {
                        if(iSuckX1<0 || iSuckY1<0)
                            bSuckFlag1=false;
                        else if(BTestSuck.Suck[iSuckX1][iSuckY1].GetStatus())
                            bSuckFlag1=true;
                    }

                    if(bSuckFlag2==false)
                    {
                        if(iSuckX2<0 || iSuckY2<0)
                            bSuckFlag2=false;
                        else if(BTestSuck.Suck[iSuckX2][iSuckY2].GetStatus())
                            bSuckFlag2=true;
                    }

                    if(bSuckFlag1==true || bSuckFlag2==true)
                    {
                        Task=555;
                        StopAllMotor(true);   //AI(W906-E042) 20261005: golden `StopAllMotor();` = the default argument true (Motor/myGALILmotor.h:78); spelled out because aHotPlateSubstrate.h:980 also declares a no-arg StopAllMotor (forwards to (true), aHotPlateSubstrate.cpp:1236) and the call would be ambiguous
                        MOT[MTestZ2].Gali_Command("ST", __FUNC__);                                                      //馬達停止
                        fMain->chkReadTorque1->Checked=false;                                                           //wei 20160504 不顯示扭力
                        fMain->chkReadTorque2->Checked=false;                                                           //wei 20160504 不顯示扭力
                        Pos=MOT[MTestZ2].Gali_ReadEncoderPos();
                        break;
                    }
                }

                if(USE_INDEX_ARM_AXES==IndexArm_3_Axis)                                                                 //JimmyChiu 20220708 : add Index Arm Axis
                {
                    PosY2=MOT[MTestY1].Gali_ReadPos();
                    if(abs(PosY2-Prod.TestY1_Middle)>10)
                    {
                        ShowMyMessage("Index Y1 Motor Position error", "馬達Y1位置錯誤!", "::DoZ2PickFromShuttle 550");
                        return false;
                    }
                }
                else
                {
                    PosY2=MOT[MTestY2].Gali_ReadPos();
                    PosY2-=Prod.TestY2_Rear;
                    if(abs(PosY2)>10)                                                                                   //JerryYang 20160824 Start:Z2下降前增加防護
                    {
                        ShowMyMessage("Index Y2 Motor Position error", "馬達Y2位置錯誤!", "::DoZ2PickFromShuttle 550");
                        return false;
                    }
                }

                if(W906_IndexZCommandFloorStop(Pos, fIndexDownPos, "DoZ2PickFromShuttle 550")) { fAllMotorHome=false; CarlibrationTask=1; return false; }                   if(MOT[MTestZ2].Gali_MotMoveNoWait(Pos, GotIndexZSpeed(iSpeedZ*100), 0, false))                         //JimmyChiu 20211028 : All speed can set by speed setting.
                {
                    COM2->InitReadTorueTask();
                    fMain->chkReadTorque1->Checked=false;
                    fMain->chkReadTorque2->Checked=true;
                    fMain->edTorue1->Text="";
                    Task=560;
                }
                break;
            case 555:
                if(MOT[MTestZ2].Gali_MotMoveNoWait(Pos, GotIndexZSpeed(iSpeedZ*100), 0, false))                         //JimmyChiu 20211028 : All speed can set by speed setting.
                {
                    Task=570;
                }
                break;
            case 560:
                #ifdef SOFT_SIMULTE
                    Task=570;
                #endif
                if(W906_IndexZTorqueWaitStop(1, fMain->edTorue1->Text=="", "DoZ2PickFromShuttle 560")) { fAllMotorHome=false; CarlibrationTask=1; return false; }   if(fMain->edTorue1->Text=="")   //AI(W906-E042) 20261005 (B3) P5 NOT GOLDEN: 5 s without a torque value -> ST Z2 + message + golden 536 exit
                    break;

                iTorque=atoi(fMain->edTorue1->Text.c_str());

                if(INDEX_DRIVER_TYPE==Panasonic_DRIVER)                         //Panasonic     //Steven 20140530
                    iTorque=abs(iTorque);

                if(iTorque>=iContactKG)
                {
                    COM2->InitReadTorueTask();
                    fMain->chkReadTorque1->Checked=false;
                    fMain->chkReadTorque2->Checked=true;
                    fMain->edTorue1->Text="";

                    Counter++;
                    if(Counter>5)
                    {
                        Counter=0;
                        if(bNeedSuck==false)
                        {
                            Task=570;
                            break;
                        }
                    }
                }
                else
                {
                    Counter=0;
                    Task=500;
                }
                break;
            case 570:
                bNeedSuck=true;
                MOT[MTestZ2].Gali_Command("ST", __FUNC__);                                                              //馬達停止
                if(DeviceForm_File.dKitDiameter<=2.5 &&                                                                 //JerryYang 20201224 修正20mm SLK auto height高度太低的問題
                   INDEX_PRESS_TYPE!=e85KG)                                                                             //Frank 20250214 add
                {
                    Pos=MOT[MTestZ2].Gali_ReadEncoderPos()+PICK_UP_OFFSET+200;
                }
                else
                {
                    Pos=MOT[MTestZ2].Gali_ReadEncoderPos()+PICK_UP_OFFSET;
                }

                if(CUSTOMER_CODE==CC_ASE_KaohSiung)
                    edPickUp2->Text=(Pos-Tech.iTestZ2ShutlePick-20)/100.0;                                              //kevin 20220831 shuttle pick high -0.2mm
                else
                    edPickUp2->Text=(Pos-Tech.iTestZ2ShutlePick)/100.0;

                if(CUSTOMER_CODE==CC_SIGURD_PeiXing)                                                                    //KaiChen 20191111 ：矽格-北興 黃建榮 要求 Contact 頁面中，IC 大小任一邊小於3.5mm，Release Height 設為 1mm
                {
                    if(atoi(edYDimension->Text.c_str())<=3.5 ||
                       atoi(edXDimension->Text.c_str())<=3.5)                                                           //jou 2012-10-18 因為常常飛料，&& -> ||，< -> <=
                        edReleaseHeight2->Text=(Pos-Tech.iTestZ2ShutlePick+RELEASE_UP_SMALL)/100.0;
                    else
                        edReleaseHeight2->Text=(Pos-Tech.iTestZ2ShutlePick+RELEASE_UP_BIG)/100.0;
                }
                else if(CUSTOMER_CODE==CC_ASE_KaohSiung)                                                                //kevin 20220831 autohigh IC大小任一邊小於 release high
                {
                    if(atoi(edYDimension->Text.c_str())<5 ||
                       atoi(edXDimension->Text.c_str())<5)                                                              //IC <5 up +1 mm
                        edReleaseHeight2->Text=(Pos-Tech.iTestZ2ShutlePick+100)/100.0;
                    else if((atoi(edYDimension->Text.c_str())>=5 && atoi(edYDimension->Text.c_str())<=6.99) ||
                            (atoi(edXDimension->Text.c_str())>=5 && (atoi(edXDimension->Text.c_str())<=6.99)))          //IC <5 up +1 mm
                        edReleaseHeight2->Text=(Pos-Tech.iTestZ2ShutlePick+150)/100.0;
                    else
                        edReleaseHeight2->Text=(Pos-Tech.iTestZ2ShutlePick+RELEASE_UP_BIG)/100.0;
                }
                else if(CosFunction.bAutoHeightSHTReleaseByFile)                                                        //Sam 20200217 : K高後 Shuuttle Release Height offset By SetupFile
                {
                    edReleaseHeight2->Text=((Pos-Tech.iTestZ2ShutlePick)/100.0)+DeviceForm.iAutoHeightSHTReleaseOfs;
                }
                else
                {
                    if(atoi(edYDimension->Text.c_str())<=7 ||
                       atoi(edXDimension->Text.c_str())<=7)                                                             //jou 2012-10-18 因為常常飛料，&& -> ||，< -> <=
                        edReleaseHeight2->Text=(Pos-Tech.iTestZ2ShutlePick+RELEASE_UP_SMALL)/100.0;
                    else
                        edReleaseHeight2->Text=(Pos-Tech.iTestZ2ShutlePick+RELEASE_UP_BIG)/100.0;
                }

                InitWriteAndCheckMotorTorqueTask();
                hContactDeley.SetSecAndOn(3);
                ADAM_WriteVoltage(0);

                if(INDEX_SUCKER_TYPE==1)                                                                                //jou 2010-05-19 start : 負壓
                    fiosetview->ResetIndexSuck();

                for(int i=0; i<BTestSuck.iShtRow; i++)
                {
                    for(int j=0; j<BTestSuck.iShtCol; j++)
                    {
                        if((CosFunction.bContactTestVacOffByCloseSite==true && LastSet.bUseTestSocket[1][i][j]) ||      //JerryYang 20160328 for 矽格北興,auto height時,關site的部分不吸取IC
                            CosFunction.bContactTestVacOffByCloseSite==false)
                        {
                            if(INDEX_SUCKER_TYPE==0)
                            {
                                BTestSuck.Suck[i][j].On();
                            }
                            else
                            {
                                fiosetview->bIndexSuck[1][i][j]=true;
                                ct=0;
                                if(IniConfig.bKoreaFunction==true)                                                      //Steven 20150824 : 韓國要求用原本的方式
                                {
                                    do
                                    {
                                        fiosetview->ProcessIndexSuckDestroy2();
                                        MySleepEx(1, true);
                                        ct++;
                                    }while(ct<iSuckDelay);
                                }
                                else
                                {
                                    do
                                    {
                                        if(false /*GATE(W906-E042-D73) AI(W906-E042) 20261005: TfiosetviewShim has no ContactIndexSuckDestroy1 (golden fIoSetView, KYEC fast contact mode); until it is ported the option behaves as OFF = golden's own else branch*/ && IniConfig.bD73ContactModeFast)                                               //kevin 20220817 : Conttact mode 加速
                                        {
                                            bCheckSuck=true; /*GATE(W906-E042-D73) bCheckSuck=fiosetview->ContactIndexSuckDestroy1(1, i, j); -- AI(W906-E042) 20261005: unreachable (condition above)*/
                                        }
                                        else
                                        {
                                            bCheckSuck=fiosetview->ProcessIndexSuckDestroy2();
                                            MySleepEx(1, true);
                                        }
                                        ct++;
                                    }while(bCheckSuck==false);                                                          //jou 2015-05-18 修正 contect mode 中途暫停會發生掉料
                                }
                            }
                        }
                    }
                }
                bProcessIndexSuckDestroy=false;
                Task=600;
                break;
            case 600:
                if(hContactDeley.Off() || bProcessIndexSuckDestroy==true)
                {
                    for(int i=0; i<BTestSuck.iShtRow; i++)
                    {
                        for(int j=0; j<BTestSuck.iShtCol; j++)
                        {
                            if(BTestSuck.Suck[i][j].GetStatus())
                            {
                                if(BAR_CODE_INSTALL!=ebctUninstall &&
                                   (TestIF_File.bEnableBarCode ||                                                       //Ifor 20190129 : add Cognex EtherNet 通訊
                                   (BOTTOM_2DID && TestIF_File.bEnableBottom2D)))                                       //Steven 20190422 : Bottom 2DID
                                {
                                    if(chk2DID->Checked==false)                                                         //Steven 20210310 : contact test可以跳過讀取2DID
                                    {
                                        if(iContactMode==CONTACT_DEVICE_MAP_CHECK)                                      //Steven 20210609 : 修正有IC才吸取
                                        {
                                            if(BLCarryKit.Item[i][j]!=NULL_IC)                                          //Steven 20170111 : 避免Auto Height時重複進來被改成NULL_IC
                                                BTestSuck.MoveSuckData(BLCarryKit, i, j);                               //Steven 20210806 : Fixed for Device Mapping
                                        }
                                        else
                                        {
                                            BTestSuck.SetItemData(i, j, HAS_IC);
                                        }
                                    }
                                    else
                                    {
                                        if(BLCarryKit.Item[i][j]!=NULL_IC)                                              //Steven 20170111 : 避免Auto Height時重複進來被改成NULL_IC
                                            BTestSuck.MoveSuckData(BLCarryKit, i, j);
                                    }
                                }
                                else
                                {
                                    if(iContactMode==CONTACT_DEVICE_MAP_CHECK)                                          //Steven 20210609 : 修正有IC才吸取
                                    {
                                        if(BLCarryKit.Item[i][j]!=NULL_IC)                                              //Steven 20170111 : 避免Auto Height時重複進來被改成NULL_IC
                                            BTestSuck.MoveSuckData(BLCarryKit, i, j);                                   //Steven 20210806 : Fixed for Device Mapping
                                    }
                                    else
                                    {
                                        BTestSuck.SetItemData(i, j, HAS_IC);
                                    }
                                }
                            }

                            if(IniConfig.bChangeKitNoHardStop==true)                                                    //jou 2015-12-08 Xilinx 驗證用    //Frank 20171030 add Floating Shuttle Read Torque
                            {
                                if(bSuckFlag1==false)
                                    bSuckFlag1=BTestSuck.Suck[iSuckX1][iSuckY1].GetStatus();
                                if(bSuckFlag2==false)
                                    bSuckFlag2=BTestSuck.Suck[iSuckX2][iSuckY2].GetStatus();
                                if(bSuckFlag1==false && bSuckFlag2==false)
                                {
                                    bUniversalkitflag[1]=true;
                                }
                            }
                        }
                    }

                    if(USE_IO_CHANGE_TOQUE==true)                                                                       //jou 2012-06-21 Enable index I/O Change Toque
                    {
                        SW[SwIndexChangeToque1].Off();
                        SW[SwIndexChangeToque2].Off();
                    }

                    if(DeviceForm_File.dKitDiameter<=2.5 && IniConfig.bKoreaFunction)
                    {
                        Pos=MOT[MTestZ2].Gali_ReadEncoderPos();
                        Pos=Pos+3000;
                        Task=610;
                    }
                    else
                    {
                        ret=COM2->iWriteAndCheckMotorTorque(1,80);
                        if(ret==1)
                        {
                            MOT[MTestZ2].iGali_SingalHomeTask=1;
                            Task=650;
                        }
                        else if(ret==2)
                        {
                            ShowMyMessage("Motor torque set error", "馬達扭力設定錯誤!", "DoZ2PickFromShuttle 600");
                            fAllMotorHome=false;
                            CarlibrationTask=1;
                            bNeedSuck=false;
                            return false;
                        }
                    }
                }
                break;
            case 610:
                if(MOT[MTestZ2].Gali_MotMoveSkipEncoder(Pos, GotIndexZSpeed(iSpeedZ*1000)))                             //JimmyChiu 20211028 : All speed can set by speed setting.
                {
                    ret=COM2->iWriteAndCheckMotorTorque(1,80);
                    if(ret==1)
                    {
                        MOT[MTestZ2].iGali_SingalHomeTask=1;
                        Task=650;
                    }
                    else if(ret==2)
                    {
                        ShowMyMessage("Motor torque set error", "馬達扭力設定錯誤!", "DoZ2PickFromShuttle 600");
                        fAllMotorHome=false;
                        CarlibrationTask=1;
                        bNeedSuck=false;
                        return false;
                    }
                }
                break;
            case 650:
                if(MOT[MTestZ2].Gali_SingalHome())
                {
                    Task=700;
                }
                break;
            case 3000:
                if(MOT[MTestZ2].Gali_MotMove(Tech.iTestZ2ShutlePick+iUnitMultiply100(atof(edPickUp2->Text.c_str()))+Offset.iIndexArmPickUp[1], GotIndexZSpeed(iSpeedZ*1000)))  //JimmyChiu 20211028 : All speed can set by speed setting.
                {
                    if(chkDailyCorrelation->Checked)  //KaiHuang 20200724 : For ASE-CL Daily Correlation, Index 沒吸到料需 Alarm
                    {
                        for(int i=0; i<FTestSuck.iShtRow; i++)
                        {
                            for(int j=0; j<FTestSuck.iShtCol; j++)
                            {
                                bArm2SuckFinish[i][j]=false;  //Steven 20110301 : 初始化，都當作還沒做完
                                bArm2DuplicateErr[i][j]=false;
                                BTestSuck.Suck[i][j].Reset();  //Steven 20140213 : Jordan說Index下去不吸直接Alarm
                            }
                        }
                        Task=660;
                        break;
                    }
                    //if(iContactMode==CONTACT_TEST)                            //jou 2012-05-07 make code"if(iContactMode==CONTACT_TEST)",Auto High unclick "Auto High include Shuttle",index arm不做吸取動作.
                    {
                        if(INDEX_SUCKER_TYPE==1)  //jou 2010-05-19 start : 負壓
                        {
                            fiosetview->ResetIndexSuck();
                            for(int i=0; i<BTestSuck.iShtRow; i++)
                            {
                                for(int j=0; j<BTestSuck.iShtCol; j++)
                                {
                                    if((CosFunction.bContactTestVacOffByCloseSite==true && LastSet.bUseTestSocket[1][i][j]) ||  //JerryYang 20160328 for 矽格北興,Contact test時關site的部分不吸取IC
                                        CosFunction.bContactTestVacOffByCloseSite==false)
                                    {
                                        fiosetview->bIndexSuck[1][i][j]=true;
                                        ct=0;
                                        if(IniConfig.bKoreaFunction==true)  //Steven 20150824 : 韓國要求用原本的方式
                                        {
                                            do
                                            {
                                                fiosetview->ProcessIndexSuckDestroy2();
                                                MySleepEx(1, true);
                                                ct++;
                                            }while(ct<iSuckDelay);
                                        }
                                        else
                                        {
                                            do
                                            {
                                                if(false /*GATE(W906-E042-D73) AI(W906-E042) 20261005: TfiosetviewShim has no ContactIndexSuckDestroy1 (golden fIoSetView, KYEC fast contact mode); until it is ported the option behaves as OFF = golden's own else branch*/ && IniConfig.bD73ContactModeFast)  //kevin 20220817 : Conttact mode 加速
                                                {
                                                    bCheckSuck=true; /*GATE(W906-E042-D73) bCheckSuck=fiosetview->ContactIndexSuckDestroy1(1,i,j); -- AI(W906-E042) 20261005: unreachable (condition above)*/
                                                }
                                                else
                                                {
                                                    bCheckSuck=fiosetview->ProcessIndexSuckDestroy2();
                                                    MySleepEx(1, true);  //kevin 20220816
                                                }
                                                ct++;
                                            }while(bCheckSuck==false);  //jou 2015-05-18 修正 contect mode 中途暫停會發生掉料
                                        }
                                    }
                                }
                            }
                            hContactDeley.SetSecAndOn(1);  //jou 3 -> 1
                        }
                        else
                        {
                            for(int i=0; i<BTestSuck.iShtRow; i++)
                            {
                                for(int j=0; j<BTestSuck.iShtCol; j++)
                                {
                                    if((CosFunction.bContactTestVacOffByCloseSite==true && LastSet.bUseTestSocket[1][i][j]) ||  //JerryYang 20160328 for 矽格北興,Contact test時關site的部分不吸取IC
                                        CosFunction.bContactTestVacOffByCloseSite==false)
                                    {
                                        BTestSuck.Suck[i][j].On();
                                    }
                                }
                            }
                            hContactDeley.SetSecAndOn(1);
                        }
                    }
                    bProcessIndexSuckDestroy=false;
                    Task=3050;
                }
                break;
            case 3050:
                if(hContactDeley.Off() || bProcessIndexSuckDestroy==true)
                {
                    //if(iContactMode==CONTACT_TEST)                            //jou 2012-05-07 make code"if(iContactMode==CONTACT_TEST)",Auto High unclick "Auto High include Shuttle",index arm不做吸取動作.
                    {
                        for(int i=0; i<BTestSuck.iShtRow; i++)
                        {
                            for(int j=0; j<BTestSuck.iShtCol; j++)
                            {
                                #ifndef SOFT_SIMULTE
                                if(LastSet.iRealDummy==REALLY && !BTestSuck.Suck[i][j].GetStatus())
                                {
                                    BTestSuck.Suck[i][j].Off();                                                         //Normal -> Off 避免ic被沾黏上來
                                }
                                else
                                #endif
                                {
                                    if(BAR_CODE_INSTALL!=ebctUninstall &&
                                       (TestIF_File.bEnableBarCode ||
                                       (BOTTOM_2DID && TestIF_File.bEnableBottom2D)))                                   //Steven 20190422 : Bottom 2DID
                                    {
                                        if(chk2DID->Checked==false)                                                     //Steven 20210310 : contact test可以跳過讀取2DID
                                        {
                                            if(iContactMode==CONTACT_DEVICE_MAP_CHECK)                                  //Steven 20210609 : 修正有IC才吸取
                                            {
                                                if(BLCarryKit.Item[i][j]!=NULL_IC)                                      //Steven 20170111 : 避免Auto Height時重複進來被改成NULL_IC
                                                    BTestSuck.MoveSuckData(BLCarryKit, i, j);                           //Steven 20210806 : Fixed for Device Mapping
                                            }
                                            else
                                            {
                                                BTestSuck.SetItemData(i, j, HAS_IC);
                                            }
                                        }
                                        else
                                        {
                                            if(BLCarryKit.Item[i][j]!=NULL_IC)                                          //Steven 20170111 : 避免Auto Height時重複進來被改成NULL_IC
                                                BTestSuck.MoveSuckData(BLCarryKit, i, j);
                                        }
                                    }
                                    else
                                    {
                                        if(iContactMode==CONTACT_DEVICE_MAP_CHECK)                                      //Steven 20210609 : 修正有IC才吸取
                                        {
                                            if(BLCarryKit.Item[i][j]!=NULL_IC)                                          //Steven 20170111 : 避免Auto Height時重複進來被改成NULL_IC
                                                BTestSuck.MoveSuckData(BLCarryKit, i, j);                               //Steven 20210806 : Fixed for Device Mapping
                                        }
                                        else
                                        {
                                            BTestSuck.SetItemData(i, j, HAS_IC);
                                        }
                                    }
                                }
                            }
                        }
                    }
                    //AI(ht9045-v899) 20260421: 全智 Contact/Auto Height 吸取後立即驗證每個應吸取 site 的真空 sensor，掉料/沒吸到時直接停止下壓避免下一刀壓壞 IC 與配件
                    if(CosFunction.bContactTestICDropGuard==true &&
                       (iContactMode==CONTACT_TEST ||
                        iContactMode==AUTO_CONTACT_TEST ||
                        iContactMode==CONTACT_AUTO_GET_HEIGHT))
                    {
                        bool bDropDetected=false;
                        bool bGigasContactTestPartialPick=false;
                        bool bHasGuardSite=false;

                        //AI(ht9045-v899) 20260608: 將全智 partial pick 豁免由僅 CONTACT_TEST 放寬至 AUTO_CONTACT_TEST/CONTACT_AUTO_GET_HEIGHT，保留 CUSTOMER_CODE==CC_GIGAS 前置條件確保只改變全智行為(全智雙Arm Contact掉料防護放寬, CASE-20260608-001)
                        bGigasContactTestPartialPick=(CUSTOMER_CODE==CC_GIGAS &&
                                                      (iContactMode==CONTACT_TEST ||
                                                       iContactMode==AUTO_CONTACT_TEST ||
                                                       iContactMode==CONTACT_AUTO_GET_HEIGHT));
                        //AI(ht9045-v899) 20260608: 全智改用該site IC資料是否==HAS_IC(實際吸取成功)當守護門檻，不再用recipe site開關LastSet.bUseTestSocket；HAS_IC於case3050僅在REALLY且真空sensor ON時SetItemData，代表實際吸到的site，故只檢查這些site有無掉料，其餘site一律不檢查不報錯(全智改用HAS_IC判斷掉料, CASE-20260608-001)
                        for(int i=0; i<BTestSuck.iShtRow; i++)
                        {
                            for(int j=0; j<BTestSuck.iShtCol; j++)
                            {
                                bool bNeedGuard;

                                if(bGigasContactTestPartialPick)
                                    bNeedGuard=(BTestSuck.Item[i][j]==HAS_IC);
                                else
                                    bNeedGuard=(LastSet.bUseTestSocket[1][i][j]==true);   //AI(ht9045-v899) 20260608: 非全智維持原依recipe site開關檢查行為不變(全智改用HAS_IC判斷掉料, CASE-20260608-001)

                                if(bNeedGuard)
                                {
                                    bHasGuardSite=true;
                                    if(BTestSuck.Suck[i][j].GetStatus()==false)
                                        bDropDetected=true;
                                }
                            }
                        }

                        //AI(ht9045-v899) 20260608: 移除整臂無守護site即強制報掉料的致命fallback，全智允許整臂本輪不上料(部分手動上料)時跳過防護不報掉料，非全智不受影響(全智雙Arm Contact掉料防護放寬, CASE-20260608-001)
                        //(原: if(bGigasContactTestPartialPick && bHasGuardSite==false) bDropDetected=true;)

                        if(bDropDetected==true)
                        {
                            //AI(ht9045-v899) 20260608: 真實掉料中止前將全智本臂(Z2=Rear)本次pick暫存資料還原回進站前狀態(iBTestBackItem)，避免殘留IC觸發IndexHasIC()互鎖鎖死contact模式，比照Exit還原寫法且僅全智路徑生效(全智雙Arm Contact掉料防護放寬, CASE-20260608-001)
                            if(bGigasContactTestPartialPick)
                            {
                                for(int ci=0; ci<BTestSuck.iShtRow; ci++)
                                {
                                    for(int cj=0; cj<BTestSuck.iShtCol; cj++)
                                    {
                                        BTestSuck.SetItemData(ci, cj, iBTestBackItem[ci][cj]);
                                    }
                                }
                            }
                            ShowMyMessage("Z2 Contact/Auto Height pick fail or IC drop, stop pressing",
                                          "Z2 Contact/Auto Height 吸取失敗或掉料，已停止下壓以保護 IC 與配件",
                                          "DoZ2PickFromShuttle 3050");
                            fAllMotorHome=false;
                            CarlibrationTask=1;
                            bNeedSuck=false;
                            return false;
                        }
                    }
                    Task=3100;
                }
                break;
            case 3100:
                if(MOT[MTestZ2].Gali_MotMove(Tech.iTestZ2ShutlePick+iUnitMultiply100(atof(edPickUp2->Text.c_str()))+Offset.iIndexArmPickUp[1]+200, GotIndexZSpeed(iSpeedZ*1000)))  //JimmyChiu 20211028 : All speed can set by speed setting.
                {
                    bAlarm=CheckIndexAllSuckICFallDown(false, true);  //Steven 20110725 : 修改負壓檢查方式
                    if(bAlarm==true)
                    {
                        ShowMyMessage("Z2 Pick From Shuttle drop IC ","Z2吸取Shuttle ic掉落");
                        //AI(ht9045-v899) 20260421: 全智要求掉料時直接中止 Contact/Auto Height，避免下一刀下壓壓壞 IC/配件
                        if(CosFunction.bContactTestICDropGuard==true &&
                           (iContactMode==CONTACT_TEST ||
                            iContactMode==AUTO_CONTACT_TEST ||
                            iContactMode==CONTACT_AUTO_GET_HEIGHT))
                        {
                            fAllMotorHome=false;
                            CarlibrationTask=1;
                            bNeedSuck=false;
                            return false;
                        }
                    }
                    Task=700;
                }
                break;
            case 660:
                bArm2NeedSuck=true;                                             //JerryYang 20190123 新增保護避免真空持續on會造成all site掉料
                bArm2SuckComplete=true;

                #ifdef SOFT_SIMULTE
//                    if(CheckBox_Index2PickError->Checked==true)
//                    {
//                        bArm2SuckFinish[0][2]=true;
//                        BTestSuck.Suck[0][2].Error=true;
//                    }
                #endif
                    DoArm2Suck();                                               //JerryYang 20190123 把index arm吸真空&交換狀態包成函式
                //Steven 20110301 : Start
                for(int i=0; i<BTestSuck.iShtRow; i++)
                {
                    for(int j=0; j<BTestSuck.iShtCol; j++)
                    {
                        if(bArm2SuckFinish[i][j]==false)                        //只要有未完成的就繼續等
                            bArm2SuckComplete=false;
                    }
                }
                //Steven 20110301 : End

                if(bArm2SuckComplete==true)                                     //Steven 20110301 : 所有吸嘴都做完
                {
                    bHasErr=false;
                    bArm2NeedSuck=false;
                    for(int i=0; i<BTestSuck.iShtRow; i++)
                    {
                        for(int j=0; j<BTestSuck.iShtCol; j++)
                        {
                            if(BTestSuck.Suck[i][j].Error)
                            {
                                bHasErr=true;
                                //jou 2012-06-12 吸shuttle時，需檢測Torque，過大需alarm
//                                if(USE_IO_CHANGE_TOQUE==true)  //jou 2012-06-12 即時更新扭力值不能開，不然會衝突
//                                {
//                                    fMain->CheckBox10->Checked=false;
//                                    fMain->CheckBox11->Checked=true;
//                                    fMain->edTorue1->Text="";
//                                    bOverHappen=false;
//                                    hDoRearTestSuckIC.SetSecAndOn(1);
//                                }
//
//                                if(IniConfig.bD62PickUpErrorNeedPurge)                  //Steveb 20161024 : 吸取異常需要吹氣一次
//                                {
//                                    BTestSuck.Suck[i][j].Off();
//                                }
                            }
                        }
                    }

                    if(bHasErr)
                    {
                        Task=665;
                        return false;
                    }

                    if(BLCarryKit.HasRealIC())
                        break;

//                    if(bAutoSiteMapWaitTestResult==true && BTestSuck.HasRealIC()==false)    //Steven 20200326 : 修正JCET Auto site map發生inarm掉料會hang up
//                    {
//                        bAutoSiteMapWaitTestResult=false;
//                    }

                    ZeroMemory(bArm2DuplicateErr, sizeof(bArm2DuplicateErr));
                    if(BLCarryKit.HasIC())
                        break;

                    bResetIndexArm2Pick=false;
//                    if(CUSTOMER_CODE==CC_TSMC_TAINAN && (Prod.bWhenNoFullSiteUseInitialDelay && IniConfig.bL18NofullsiteaddTemperatureoffset))     //wei 20151228 No FullSite delay    //wei 20161102 No FullSite delay修改||->&&
//                        CheckBTFullSite();
                    Task=3100;
                    iD43AutoRetryWhenIndexPickErrCnt[1]=0;                      //Steven 20170105 : Index吸取異常,要退出來用Shuttle Sensor檢查後, 再進去吸一次
                }
                break;
            case 665:
                CheckIndexAllSuckICFallDown(false, true);                       //Steven 20110725 : 修改負壓檢查方式

                if(MOT[MTestZ2].Gali_ReadPos()>(Prod.TestZ2_Pick+1000))         //Steven 20150407 : 修正[D45] Out Arm等Index Z功能, 避免Auto Homing
                {
                    bZ2PickShuttle=false;
                }

                #ifdef DEBUG_INDEX_UPH
                if(MOT[MTestZ2].Gali_MotMove2(Prod.TestZ2_Safe, iIndexSpeed, iIndexAcc))
                #else
                if(MOT[MTestZ2].Gali_MotMoveNoWait(Prod.TestZ2_Safe, MOT[MTestZ2].GailSpeed, 0))
                #endif
                {
                    Task=670;                                                   //Steven 20160718 : Index pick up error with [D43]
                }
                break;
            case 670:
//                bZ2PickShuttle=false;   //Steven 20150407 : 修正[D45] Out Arm等Index Z功能, 避免Auto Homing
//
//                if(IniConfig.bD43IndexDropErrorCanRetryandSkip)
//                {
//                    if(BRCarryKit.NoIC()) //ChungHung 20120717 add Index Drop Error Can Retry and Start
//                    {
//                        MOT[MInShuttle2].fCanMoveM=true;
//                        bShuttle2MoveToLeft=true;
//                        bIndexPickErrShtStayRight2=false;       //JerryYang 20181206 (Steven) : fix 啟用D43功能時,index arm pick up error後按retry可能發生hang up
//                        bCheckNullIC2=false;   //JerryYang 20170623 (wei) 修正有裝out shuttle 前後對照的機台發生index arm吸取異常無法跳出alram造成hang up
//                        Task=320;
//                    }
//                }
//                else
                {
                    if(bPlaceToShuttle2Step)                                    //Steven 20160718 : 避免放蝦頭放到一半讓位會死雞
                        return false;
                    Task=675;
                }
                break;
            case 675:
                //ChungHung 20120717 add Index Drop Error Can Retry and Start
                if(IniConfig.bD43IndexDropErrorCanRetryandSkip && bShuttle2MoveToLeft)                                  //ChungHung 20131015 fix hangup
                {
                    if(InSHT2InLF()!=true)
                    {
                        return false;
                    }
                }
                bShuttle2MoveToLeft=false;
//                MOT[MInShuttle2].fCanMoveM=false;
                Task=680;

                if(TestIF.iTestMode==_32Site4X8N || TestIF.iTestMode==_16Site4X4)                                       //Sam 20190226 : 16Site4X4 //Steven 20150901
                    break;
            case 680:
                //Steven 20110131 Start
                ErrPart=" ";
                bHasErr=false;
                bHasDuplicateErr=false;
                for(int i=0; i<BTestSuck.iShtRow; i++)
                {
                    for(int j=0; j<BTestSuck.iShtCol; j++)
                    {
                        if(bArm2DuplicateErr[i][j])
                            bHasDuplicateErr=true;

                        if(BTestSuck.Suck[i][j].Error)
                        {
                            bHasErr=true;
                            ErrPart+=IndexSuckName[i][j];
                            if(IniConfig.bD62PickUpErrorNeedPurge)                                                                                              //Steveb 20161024 : 吸取異常需要吹氣一次
                            {
                                BTestSuck.Suck[i][j].Normal();
                            }
                            BLCarryKit.PordRec[i][j].AddErrorRecordNoSave("JAM0302");
                        }
                        else
                        {
                            BTestSuck.Suck[i][j].Error=false;
                        }
                    }
                }

                if(bHasErr)
                {
                    if(IndexAlarmInArmAway()==false)                                                                                                            //Steven 20130613 : Index異常時, In Arm要先讓位功能
                    {
                        return false;
                    }

                    bHasErr=false;
                    if(IniConfig.bD43AutoRetryWhenIndexPickErr &&                                                                                               //Steven 20170105 : Index吸取異常,要退出來用Shuttle Sensor檢查後, 再進去吸一次
                       iD43AutoRetryWhenIndexPickErrCnt[1]==0)
                    {
                        ret=K_RETRY;
                    }
                    else if(IniConfig.bNewResetFunction==true && bResetIndexArm2Pick==true)
                    {
                        ret=K_SKIP;
                    }
                    else
                    {
                        if(CosFunction.bJAM0301NeedOpenChamberDoor)                                                                                             //wei : JAM0301 & JAM0302需要開啟Chamber門10秒
                        {
                            bIsTestSitICFallDown=true;
                        }

//                        if(IniConfig.bIndexPickErrOnlySKIP==true || IniConfig.bD64IndexPickErrOnlySKIP)//kevin 20171103 (wei) add retry function     //jou 2012-02-13 index pick-up error only skip
//                            ret=ShowErrorMessage("JAM0302", K_SKIP, MTestZ2, bHasDuplicateErr, ErrPart); //Devicr Pick-Up Error
//                        else
//                            ret=ShowErrorMessage("JAM0302", K_SKIP|K_RETRY, MTestZ2, bHasDuplicateErr, ErrPart); //Devicr Pick-Up Error     .
                        ret=ShowErrorMessage("JAM0302", K_RETRY, MTestZ2, bHasDuplicateErr, ErrPart);                                                           //Devicr Pick-Up Error
                    }

                    if(ret==K_SKIP)
                    {
                        bAutoSiteMapWaitTestResult=false;                                                                                                       //Ifor 20180115 (Steven) : add Site Mapping SKIP 需清除旗標
                        bIndexPickUpErrMoveSht2=false;                                                                                                          //Steven 20171221 (Wei) : 修正[D43]當蝦頭退出來要回去前,如果In Arm補了HAS_NULL_IC在蝦頭上會造Hang up
                        iD43AutoRetryWhenIndexPickErrCnt[1]=0;                                                                                                  //Steven 20170105 : Index吸取異常,要退出來用Shuttle Sensor檢查後, 再進去吸一次
                        for(int i=0; i<BTestSuck.iShtRow; i++)
                        {
                            for(int j=0; j<BTestSuck.iShtCol; j++)
                            {
    //                            BLCarryKit.PordRec[i][j].AddErrorRecord("JAM0301");
                                //jou 2011-12-27 有發生過Skip又重吸一次,改成下面的方式
                                if(BTestSuck.Item[i][j]==NULL_IC)
                                {
                                    if(BTestSuck.Suck[i][j].Error)                                                                                              //JerryYang 20200303 fix 沒IC的地方jam count被++
                                    {
//                                        if(CosFunction.bUseSCKART)                              //Steven 20161214 (wei) : For SCK ART
//                                            fSCKART->iOutputJamCnt++;
                                    }
                                    BLCarryKit.PordRec[i][j].AddErrorRecord("JAM0302");                                                                         //Steven 20161214 : 加上Index異常Skip的ErrorLog
                                    BTestSuck.SetItemData(i, j, HAS_NULL_IC);
                                    BTestSuck.Suck[i][j].Normal();
                                    if(CosFunction.bIndexPickErrSkipNeedCheckVac==true && IniConfig.bD50IndexPickErrSkipNeedCheckVac)                           //JerryYang 20170610 (wei) JSCC要求index pick up error 需再慢速下降吸一次
                                    {
                                        bSkipNeedCheckVac[1][i][j]=true;
                                        bArm2PressSkipNeedDownCheckVac=true;
                                    }
                                }
                                BLCarryKit.SetItemData(i, j, NULL_IC);

                                bArm2DuplicateErr[i][j]=false;
                            }
                        }

                        if(IniConfig.bNewResetFunction==true && bResetIndexArm2Pick==true)
                        {
                        }
                        else
                        {
                            //ChungHung 20110302 start
                            if(IniConfig.bD42IndexPickICShuttlePause)
                            {
                                bInArmNeedToSafePos=true;
                                bShuttle2Pause=true;
                                bIndexArm2PickupErrStop=true;                                                                                                   //jou 2012-02-29 index pick up error,index arm move to center & alarm
                                bShowShuttle2Device=true;                                                                                                       //kevin 20180504 index pick up error
                            }
                            //ChungHung 20110302 end
                        }
                        bIndexPickUpErrorWaitRetry=false;                                                                                                       //Ifor 20171119 (Steven) : add 避免Index Pick Up Err Inarm 偷跑造成資料異常導致Hangup
                        if(IniConfig.bD43IndexDropErrorCanRetryandSkip)                                                                                         //JerryYang 20181206 (Steven) : fix 啟用D43功能時,index arm pick up error後按retry可能發生hang up
                        {
                            bIndexPickErrShtStayRight2=false;
                        }

                        if(IniConfig.bD43IndexPickErrCheckSocket==true)                                                                                         //Steven 20190115 : SCC要求吸取異常要檢查Socket
                        {
                            bIndexArm2PickUpErrNeedPiggyback=true;
                            fMain->ResetRecordforPiggyBack("RESET_ForIndexPickUpErr");
                            iWhoTriggerPiggyBack=pbtIndexArmPickUpErr;
                            ProcessPiggyBackFunction();
                        }
                    }
                    else
                    {
                        if(IniConfig.bD43AutoRetryWhenIndexPickErr &&                                                                                           //Steven 20170105 : Index吸取異常,要退出來用Shuttle Sensor檢查後, 再進去吸一次
                           iD43AutoRetryWhenIndexPickErrCnt[1]==0)
                        {
                            iD43AutoRetryWhenIndexPickErrCnt[1]++;
                        }
                        else
                        {
                            iD43AutoRetryWhenIndexPickErrCnt[1]=0;                                                                                              //Steven 20170105 : Index吸取異常,要退出來用Shuttle Sensor檢查後, 再進去吸一次
                            for(int i=0; i<BTestSuck.iShtRow; i++)
                                for(int j=0; j<BTestSuck.iShtCol; j++)
                                    if(BTestSuck.Suck[i][j].Error)
                                        bArm2DuplicateErr[i][j]=true;
                        }
                        bIndexPickUpErrorWaitRetry=true;                                                                                                        //Ifor 20171119 (Steven) : add 避免Index Pick Up Err Inarm 偷跑造成資料異常導致Hangup
                        if(IniConfig.bD43IndexDropErrorCanRetryandSkip)                                                                                         //JerryYang 20181206 (Steven) : fix 啟用D43功能時,index arm pick up error後按retry可能發生hang up
                        {
                            bIndexPickErrShtStayRight2=true;
                        }
                        bIndexPickUpErrorWaitRetry=true;                                                                                                        //Ifor 20171119 : add 避免Index Pick Up Err Inarm 偷跑造成資料異常導致Hangup
                    }

                    BTestSuck.ResetAll();                                                                                                                       //Steven 20160323 : 避免未開啟真空
                }
                //Steven 20110131 End
                if(bShuttle2Pause)
                    MOT[MInShuttle2].SetSpeed(10);                                                                                                              //kevin 20180226 (Steven) add pick up error  shuttle down speed

                if(IniConfig.bNewResetFunction==true && bResetIndexArm2Pick==true)
                {
                    bResetIndexArm2Pick=false;
                }
                else
                {
                    //ChungHung 20120717 add Index Drop Error Can Retry and Start
                    if(IniConfig.bD43IndexDropErrorCanRetryandSkip)
                    {
                        //等待Shuttle 移至右邊
                        MOT[MInShuttle2].fCanMoveM=true;
                        bShuttle2MoveToRight=true;
                    }
                }

                if(BLCarryKit.HasRealIC())
                {
                    Task=3000;
                }
                else if(CosFunction.bIndexPickErrSkipNeedCheckVac==true && IniConfig.bD50IndexPickErrSkipNeedCheckVac && bArm2PressSkipNeedDownCheckVac==true)  //JerryYang 20170610 (wei) JSCC要求index pick up error 需再慢速下降吸一次
                {
                    Task=3000;
                }
                else
                {
                    for(int i=0; i<BTestSuck.iShtRow; i++)
                    {
                        for(int j=0; j<BTestSuck.iShtCol; j++)
                        {
                            if(BLCarryKit.Item[i][j])
                            {
                                //ChungHung 20120911 add   kevin 20120911
                                if(BLCarryKit.Item[i][j]==HAS_NULL_IC && BTestSuck.Item[i][j]!=NULL_IC)
                                    BLCarryKit.SetItemData(i, j, NULL_IC);
                                if(BLCarryKit.Item[i][j]==HAS_NULL_IC)
                                {
                                    BTestSuck.Suck[i][j].Normal();
                                    BTestSuck.MoveSuckData(BLCarryKit, i, j);
                                    bArm2DuplicateErr[i][j]=false;
                                }
                            }
                        }
                    }

                    if(BLCarryKit.HasIC())
                    {
                        Task=3000;
                        break;
                    }
                    Task=700;
                }
                break;
            case 700:
                if(MOT[MTestZ2].Gali_Two_ZAxis_Move(Prod.TestZ1_Safe, GotIndexZSpeed(iSpeedZ*1000), "DoZ2PickFromShuttle 700"))                                 //JimmyChiu 20211028 : All speed can set by speed setting.
                {
                    ADAM_WriteVoltage(DeviceForm.dPress);

                    MySleep(500);

                    #ifndef SOFT_SIMULTE
                    if(LastSet.iRealDummy==REALLY)                                                                                                              //jou 2012-05-07 Auto High unclick "Auto High include Shuttle",index arm不做吸取動作.
                    {
                        for(int i=0; i<BTestSuck.iShtRow; i++)
                        {
                            for(int j=0; j<BTestSuck.iShtCol; j++)
                            {
                                if(!BTestSuck.Suck[i][j].GetStatus() && BTestSuck.Suck[i][j].Status==false)
                                    BTestSuck.Suck[i][j].Normal();
                            }
                        }
                    }
                    #endif

                    if(iContactMode==CONTACT_AUTO_GET_HEIGHT && bAutoShuttle==false)                                                                            // 自動 K
                    {
                        InitWriteAndCheckMotorTorqueTask();
                        Task=800;
                        break;
                    }
                    else
                    {
                        if(USE_IO_CHANGE_TOQUE==true)                                                                                                           //jou 2012-06-21 Enable index I/O Change Toque
                        {
                            SW[SwIndexChangeToque1].Off();
                            SW[SwIndexChangeToque2].Off();
                        }
                        Task=1;
                        return true;
                    }
                }
                break;
            case 800:
                if(USE_IO_CHANGE_TOQUE==true)                                   //jou 2012-06-21 Enable index I/O Change Toque
                {
                    SW[SwIndexChangeToque1].Off();
                    SW[SwIndexChangeToque2].Off();
                }

                ret=COM2->iWriteAndCheckMotorTorque(1, 300);
                if(ret==1)
                {
                    Task=1;
                    return true;
                }
                else if(ret==2)
                {
                    ShowMyMessage("Motor torque set error", "馬達扭力設定錯誤!", "DoZ2PickFromShuttle 800");
                    fAllMotorHome=false;
                    CarlibrationTask=1;
                    bNeedSuck=false;
                    return false;
                }
                break;
        }
        return false;
    }
}

// ===== golden 0618 cContact.cpp:11354-11739 (span B) =====
TQPF_Timer htPlaceToShuttleDelay;
bool TfContact::DoZPlaceToShuttle()   //AI(W906-E042) 20261005 (B3): __fastcall dropped
{
    EPSwitchOnOff(eEPSwBoth);

    static bool bFlag[2]={false, false};

    #ifndef SOFT_SIMULTE
    int iNN=IsNNMode();
    bool bHasErr=false;
    #endif

    int &Task=Z_Height_Task;
    int pos1=0, pos2=0;
    bool bCheckDestroy=false;
    AnsiString ErrPart="";

    #ifndef SOFT_SIMULTE
    //AI(ht9045-v899) 20260518: reuse index drop handling during Gigas auto height place-to-shuttle
    //AI(ht9045-v899) 20260522: extend Gigas place-to-shuttle drop guard to Contact Test before release
    bool bCheckGigasPlaceToShuttleDrop=(CUSTOMER_CODE==CC_GIGAS &&
                                        (iContactMode==CONTACT_AUTO_GET_HEIGHT ||
                                         iContactMode==CONTACT_TEST) &&
                                        chk_K_Temperature->Checked==false);
    if(bCheckGigasPlaceToShuttleDrop)
    {
        if(Task==100 || Task==120 || Task==150 || Task==160 || Task==200)
        {
            if(CheckIndexAllSuckICFallDown(true, false))
            {
                CheckIndexSuckICFallDownSetToHasNullIC(0);
                return false;
            }

            if(CheckIndexAllSuckICFallDown(false, true))
            {
                CheckIndexSuckICFallDownSetToHasNullIC(1);
                return false;
            }
        }
        else if(Task==600 || Task==700)
        {
            if(CheckIndexAllSuckICFallDown(false, true))
            {
                CheckIndexSuckICFallDownSetToHasNullIC(1);
                return false;
            }
        }
    }
    #endif

    if(W906_IndexZDriveFaultStop("DoZPlaceToShuttle")) { fAllMotorHome=false; CarlibrationTask=1; return false; }   switch(Task)   //AI(W906-E042) 20261005 (B3) P7 NOT GOLDEN: M14 drive alarm / ERROR_STOP / servo off / monitor lost or frozen / route fault -> ST + message + golden 536 exit; PCI1203 Z1 SHIP only, no-op elsewhere
    {
        case 1:
            #ifndef SOFT_SIMULTE
            if(chk_K_Temperature->Checked==true)                                //Ztex 2024.03.26 Add Contact Mode K Temperature
                return true;
            bHasErr|=CheckIndexAllSuckICFallDown(true, false);                  //Steven 20131128 : 換位置

            for(int i=0; i<FTestSuck.iShtRow; i++)                              //Steven 20110516 Start: IC掉落要Alarm,並整合成一次
            {
                for(int j=0; j<FTestSuck.iShtCol; j++)
                {
                    if(FTestSuck.Suck[i][j].Enable       &&
                       FTestSuck.Suck[i][j].SenUsing!="" &&
                       FTestSuck.Item[i][j]!=HAS_NULL_IC &&
                       FTestSuck.Item[i][j]!=NULL_IC)
                    {
                        if(FTestSuck.Suck[i][j].GetStatus()==false)
                        {
                            ErrPart+=IndexSuckName[i+iNN][j];
                            bHasErr=true;
                        }
                    }
                }
            }

            if(LastSet.iRealDummy==REALLY && bHasErr)
            {
                ShowErrorMessage("JAM0303", K_SKIP, MTestZ1, false, ErrPart);   //Steven 20100129 : Device Drop Error
                if(CosFunction.bJAM0303NeedOpenChamberDoor)                     //Steven : JAM0303 & JAM0403需要開啟Chamber門10秒
                    bIsTestSitICFallDown=true;

                for(int i=0; i<FTestSuck.iShtRow; i++)
                {
                    for(int j=0; j<FTestSuck.iShtCol; j++)
                    {
                        if(FTestSuck.Suck[i][j].Error ||
                           (FTestSuck.Item[i][j]!=HAS_NULL_IC &&
                            FTestSuck.Item[i][j]!=NULL_IC &&
                            FTestSuck.Suck[i][j].GetStatus()==false))           //有用到且有吸到IC的卻掉了
                        {
                            FTestSuck.SetItemData(i, j, HAS_NULL_IC);           //Steven 20110829 : 把有IC掉料的位置改成Has Null IC
                            FTestSuck.Suck[i][j].Normal();                      //Steven 20110829 : 把真空關掉
                        }
                    }
                }
                break;
            }

            bHasErr|=CheckIndexAllSuckICFallDown(false, true);

            for(int i=0; i<BTestSuck.iShtRow; i++)                              //Steven 20110516 Start: IC掉落要Alarm,並整合成一次
            {
                for(int j=0; j<BTestSuck.iShtCol; j++)
                {
                    if(BTestSuck.Suck[i][j].Enable       &&
                       BTestSuck.Suck[i][j].SenUsing!="" &&
                       BTestSuck.Item[i][j]!=HAS_NULL_IC &&
                       BTestSuck.Item[i][j]!=NULL_IC)
                    {
                        if(BTestSuck.Suck[i][j].GetStatus()==false)
                        {
                            ErrPart+=IndexSuckName[i][j];
                            bHasErr=true;
                        }
                    }
                }
            }

            if(LastSet.iRealDummy==REALLY && bHasErr)
            {
                if(CosFunction.bJAM0303NeedOpenChamberDoor)                     //Steven : JAM0303 & JAM0403需要開啟Chamber門10秒
                    bIsTestSitICFallDown=true;
                ShowErrorMessage("JAM0304", K_SKIP, MTestZ2, false, ErrPart);   //Steven 20100129 : Device Drop Error
                for(int i=0; i<FTestSuck.iShtRow; i++)
                {
                    for(int j=0; j<FTestSuck.iShtCol; j++)
                    {
                        if(BTestSuck.Suck[i][j].Error ||
                           (BTestSuck.Item[i][j]!=HAS_NULL_IC &&
                            BTestSuck.Item[i][j]!=NULL_IC &&
                            BTestSuck.Suck[i][j].GetStatus()==false))           //有用到且有吸到IC的卻掉了
                        {
                            BTestSuck.SetItemData(i, j, HAS_NULL_IC);           //Steven 20110829 : 把有IC掉料的位置改成Has Null IC
                            BTestSuck.Suck[i][j].Normal();                      //Steven 20110829 : 把真空關掉
                        }
                    }
                }
                break;
            }
            #endif
            Task=100;
        case 100:
            if(MOT[MTestZ1].Gali_Two_ZAxis_Move(Prod.TestZ1_Safe, GotIndexZSpeed(iSpeedZ*1000)))                        //Steven 20200121 : ATK小鄭說吸放料速度要一樣 5000 --> 1000    //JimmyChiu 20211028 : All speed can set by speed setting.
                Task=110;
            break;
        case 110:
            if(USE_INDEX_ARM_AXES==IndexArm_3_Axis)                             //JimmyChiu 20220708 : add Index Arm Axis
            {
                Task=1000;                                                      //單軸作業
            }
            else
            {
                Task=120;                                                       //兩軸同時作業
            }
            break;

        case 1000:                                                              //Initial   //JimmyChiu 20220826 : place to shuttle by single arm start
            DoArm1PlaceToShuttle(true);
            DoArm2PlaceToShuttle(true);
            Task=1100;
        case 1100:                                                              //arm1
            if(DoArm1PlaceToShuttle(false))
            {
                Task=1200;
            }
            break;
        case 1200:                                                              //arm1
            if(DoArm2PlaceToShuttle(false))
            {
                Task=400;
            }
            break;
        case 120:
            if(MOT[MTestY1].GalilTwoY_Move(Prod.TestY1_Front, Prod.TestY2_Rear, GotIndexYSpeed(iSpeed*3000), "DoZPlaceToShuttle 100"))                          //JimmyChiu 20211028 : All speed can set by speed setting.
            {
                htPlaceToShuttleDelay.SetSecAndOn(0.3);
                Task=150;
            }
            break;
        case 150:                                                               //Steven 20221024 : 提示先存檔, 避免發生Alarm
            pos1 =Get0_01MMType(edReleaseHeight1->Text.c_str());
            pos1+=Tech.iTestZ1ShutlePick;
            pos2 =Get0_01MMType(edReleaseHeight2->Text.c_str());
            pos2+=Tech.iTestZ2ShutlePick;

            if((pos1<-9000 && abs(pos1-Prod.TestZ1_Pick)>1000) ||
               (pos2<-9000 && abs(pos2-Prod.TestZ2_Pick)>1000))
            {
                ErrPart.sprintf("Z1 Org=%d, New=%d; Z2 Org=%d, New=%d", Prod.TestZ1_Pick, pos1, Prod.TestZ2_Pick, pos2);
                ShowMyMessage("The height difference is too big, please make sure and save data first.", ErrPart, ErrPart);
                Prod.TestZ1_Pick=pos1;
                Prod.TestZ2_Pick=pos2;
            }
            Task=160;
            break;
        case 160:
            if(htPlaceToShuttleDelay.Off())
            {
                if(USE_IO_CHANGE_TOQUE==true)                                   //jou 2012-06-21 Enable index I/O Change Toque
                {
                    SW[SwIndexChangeToque1].On();
                    SW[SwIndexChangeToque2].On();
                }
                bFlag[0]=false;
                bFlag[1]=false;

                if(IniConfig.bIndexArm2SupplyLight==true ||                     //jou 2012-10-19 Index Arm 2 供應光源 for CMOS
                   (IniConfig.bD58UseArm1PickPlaceArm2Test==true &&             //kevin 20150127 Arm1 下壓 arm2 測試
                    TestIF_File.bArm1PickPlaceArm2Test==true))                  //Ifor 20200811 Fix: Arm1 Pick Place Arm2Test 需卡兩個條件
                {
                    bFlag[1]=true;
                }

                Task=200;
            }
            break;
        case 200:
            pos1=Get0_01MMType(edReleaseHeight1->Text.c_str());
            pos1+=Tech.iTestZ1ShutlePick;
            pos2=Get0_01MMType(edReleaseHeight2->Text.c_str());
            pos2+=Tech.iTestZ2ShutlePick;

            if(TestIF.iShuttleMode==1 || bUseTwoArm)                                                                    //Use Signal shuttle
            {                                                                                                           //ChungHung 20140522 add disable Arm no move
                if(TestIF.iShuttle_Sel==0 || (bUseTwoArm && iRunWhichArm==0))                                           //shuttle 1
                {
                    bFlag[1]=true;
                }
                else if(TestIF.iShuttle_Sel==1 || ((bUseTwoArm && iRunWhichArm==1)))
                {
                    bFlag[0]=true;
                }
            }

            if(bFlag[0]==false)
                bFlag[0]=MOT[MTestZ1].Gali_MotMove(pos1, GotIndexZSpeed(iSpeedZ*1000));                                 //Steven 20200121 : ATK小鄭說吸放料速度要一樣 3000 --> 1000   //JimmyChiu 20211028 : All speed can set by speed setting.
            if(bFlag[1]==false)
                bFlag[1]=MOT[MTestZ2].Gali_MotMove(pos2, GotIndexZSpeed(iSpeedZ*1000));                                 //Steven 20200121 : ATK小鄭說吸放料速度要一樣 3000 --> 1000    //JimmyChiu 20211028 : All speed can set by speed setting.

            if(bFlag[0] && bFlag[1])
            {
                #ifndef SOFT_SIMULTE
                //AI(ht9045-v899) 20260518: recheck Gigas auto height drop before vacuum release
                //AI(ht9045-v899) 20260522: keep Contact Test drop guard active until place-to-shuttle vacuum release
                if(bCheckGigasPlaceToShuttleDrop)
                {
                    if(CheckIndexAllSuckICFallDown(true, false))
                    {
                        CheckIndexSuckICFallDownSetToHasNullIC(0);
                        break;
                    }

                    if(CheckIndexAllSuckICFallDown(false, true))
                    {
                        CheckIndexSuckICFallDownSetToHasNullIC(1);
                        break;
                    }
                }
                #endif

                for(int i=0; i<FTestSuck.iShtRow; i++)
                {
                    for(int j=0; j<FTestSuck.iShtCol; j++)
                    {
                        FTestSuck.Suck[i][j].Off();

                        if(FTestSuck.iShtCol<=4)                                                                        //Steven 20241016 : 改用Site數判斷
                            BTestSuck.Suck[i][j].Off();
                    }
                }

                hContactDeley.SetSecAndOn(2);                                                                           //ChungHung 20140527 fix can not release 0.7--->2
                if(IniConfig.bD73ContactModeFast)                                                                       //kevin 20220817 : Conttact mode 加速
                {
                    Task=350;                                                                                           //kevin 20220817
                }
                else
                {
                    if(FTestSuck.iShtCol<=4)                                                                            //Steven 20241016 : 改用Site數判斷
                        Task=300;
                    else
                        Task=600;                                                                                       //Steven 20100313 : 因為氣壓不足,所以得分兩次放下IC
                }
            }
            break;
        case 300:
            if(hContactDeley.Off())
                Task=350;
            break;
        case 350:
            for(int i=0; i<FTestSuck.iShtRow; i++)
            {
                for(int j=0; j<FTestSuck.iShtCol; j++)
                {
                    FTestSuck.Suck[i][j].Normal();
                    BTestSuck.Suck[i][j].Normal();
                }
            }
            MySleep(500);
            Task=400;
            break;
        case 400:
            if(MOT[MTestZ1].Gali_Two_ZAxis_Move(Prod.TestZ1_Safe, GotIndexZSpeed(iSpeedZ*1000), "DoZPlaceToShuttle 400"))                                       //Steven 20200121 : ATK小鄭說吸放料速度要一樣 2000 --> 1000    //JimmyChiu 20211028 : All speed can set by speed setting.
            {
                if(USE_IO_CHANGE_TOQUE==true)                                                                                                                   //jou 2012-06-21 Enable index I/O Change Toque
                {
                    SW[SwIndexChangeToque1].Off();
                    SW[SwIndexChangeToque2].Off();
                }

                Task=1;
                SW[SwTesterPower].Off();                                                                                                                        //jou 2012-07-24 Tester Power
                if(iContactMode==CONTACT_DEVICE_MAP_CHECK)                                                                                                      //Steven 20190910 : Qualcomm功能
                {
                    for(int i=0; i<FTestSuck.iShtRow; i++)
                    {
                        for(int j=0; j<FTestSuck.iShtCol; j++)
                        {
                            if(FTestSuck.Item[i][j]!=NULL_IC)
                                FLCarryKit.MoveSuckData(FTestSuck, i, j);
                            if(BTestSuck.Item[i][j]!=NULL_IC)
                                BLCarryKit.MoveSuckData(BTestSuck, i, j);
                        }
                    }
                }
                else
                {
                    if(BAR_CODE_INSTALL!=ebctUninstall &&
                       TestIF_File.bEnableBarCode)                                                                                                              //Steven 20121023 : BarCode_2D
                    {
                        FLCarryKit.ClearAll();
                        BLCarryKit.ClearAll();
                    }
                }

                if(chkDailyCorrelation->Checked)                                                                                                                //KaiHuang  20200619 : 修正 DC Bug
                {
                    for(int i=0; i<BTestSuck.iShtRow; i++)
                    {
                        for(int j=0; j<BTestSuck.iShtCol; j++)
                        {
                            if(FTestSuck.Item[i][j]!=NULL_IC)
                                FLCarryKit.MoveSuckData(FTestSuck, i, j);
                            if(BTestSuck.Item[i][j]!=NULL_IC)
                                BLCarryKit.MoveSuckData(BTestSuck, i, j);
                        }
                    }
                }

                FTestSuck.ClearAll();                                                                                                                           //jou 2010-05-19 start : 負壓
                BTestSuck.ClearAll();
                TestSocket.ClearAll();                                                                                                                          //Steven 20160123 : For 2D function do contact test
                return true;
            }
            break;
        case 600:                                                               //Steven 20100313
            if(hContactDeley.Off()==false)                                      //ChungHung 20140527 fix can not release 0.7--->2
                break;

            bCheckDestroy=true;
            if(bCheckDestroy==true && hContactDeley.Off())
            {
                Task=700;
            }
            break;
        case 700:
            for(int i=0; i<FTestSuck.iShtRow; i++)
            {
                for(int j=0; j<FTestSuck.iShtCol; j++)
                {
                    FTestSuck.Suck[i][j].Normal();
                    BTestSuck.Suck[i][j].Off();
                }
            }
            hContactDeley.SetSecAndOn(0.7);
            Task=800;
            break;
        case 800:
            if(hContactDeley.Off())
                Task=350;
            break;
    }
    return false;
}

// ===== golden 0618 cContact.cpp:19277-19501 (span C) =====
bool TfContact::DoArm1PlaceToShuttle(bool bIsFirst)
{
    if(TestIF.iShuttleMode==1)                                                  //Use Signal shuttle
    {
        if(TestIF.iShuttle_Sel==0)                                              //shuttle 1
        {
            //pass
        }
        else                                                                    //shuttle 2
        {
            return true;
        }
    }

    int &Task=iDoArm1PlaceToShuttleTask, pos=0;
    AnsiString ErrPart="";

    if(bIsFirst)
    {
        Task=1;
        return true;
    }
    else
    {
        if(W906_IndexZDriveFaultStop("DoArm1PlaceToShuttle")) { fAllMotorHome=false; CarlibrationTask=1; return false; }   switch(Task)   //AI(W906-E042) 20261005 (B3) P7 NOT GOLDEN: M14 drive alarm / ERROR_STOP / servo off / monitor lost or frozen / route fault -> ST + message + golden 536 exit; PCI1203 Z1 SHIP only, no-op elsewhere
        {
            case 1:
                Task=100;
                break;
            case 100:
                if(MOT[MTestY1].GalilTwoY_Move(Prod.TestY1_Front, Prod.TestY2_Rear, GotIndexYSpeed(iSpeed*3000), AnsiString(__FUNC__)+" 100"))
                {
                    Task=200;
                    htPlaceToShuttleDelay.SetSecAndOn(0.3);
                }
                break;
            case 200:
                if(htPlaceToShuttleDelay.Off())
                {
                    if(USE_IO_CHANGE_TOQUE==true)                               //jou 2012-06-21 Enable index I/O Change Toque
                    {
                        SW[SwIndexChangeToque1].On();
                    }
                    pos=Get0_01MMType(edReleaseHeight1->Text.c_str())+Tech.iTestZ1ShutlePick;
                    if(pos<-9000 && abs(pos-Prod.TestZ1_Pick)>1000)
                    {
                        ErrPart.sprintf("Z1 Org=%d, New=%d", Prod.TestZ1_Pick, pos);
                        ShowMyMessage("The height difference is too big, please make sure and save data first.", ErrPart, ErrPart);
                        Prod.TestZ1_Pick=pos;
                    }
                    Task=300;
                }
                break;
            case 300:
                pos=Get0_01MMType(edReleaseHeight1->Text.c_str())+Tech.iTestZ1ShutlePick;
                if(MOT[MTestZ1].Gali_MotMove(pos, GotIndexZSpeed(iSpeedZ*1000)))
                {
                    #ifndef SOFT_SIMULTE
                    //AI(ht9045-v899) 20260522: check Gigas Contact Test Z1 drop before 3-axis place-to-shuttle release
                    if(CUSTOMER_CODE==CC_GIGAS &&
                       (iContactMode==CONTACT_AUTO_GET_HEIGHT ||
                        iContactMode==CONTACT_TEST) &&
                       chk_K_Temperature->Checked==false)
                    {
                        if(CheckIndexAllSuckICFallDown(true, false))
                        {
                            CheckIndexSuckICFallDownSetToHasNullIC(0);
                            break;
                        }
                    }
                    #endif

                    for(int i=0; i<FTestSuck.iShtRow; i++)
                    {
                        for(int j=0; j<FTestSuck.iShtCol; j++)
                        {
                            FTestSuck.Suck[i][j].Off();
                        }
                    }
                    hContactDeley.SetSecAndOn(2); Task=400;   //AI(W906-E042) 20261005 (B3) #20 RULE, V912 better: [906] golden 0618 :19356 went to case 400, which waits hContactDeley.Off() (:19360), WITHOUT starting the delay, so the blow-off length was whatever delay was armed last; [V912] cContact.cpp :19648 adds `hContactDeley.SetSecAndOn(2);` here (AI(ht9045-v912) 20260910); [why] the Arm2 twin already does it (golden 0618 :19474, ChungHung 20140527 'fix can not release 0.7--->2')
                }
                break;
            case 400:
                if(hContactDeley.Off())
                    Task=500;
                break;
            case 500:
                for(int i=0; i<FTestSuck.iShtRow; i++)
                {
                    for(int j=0; j<FTestSuck.iShtCol; j++)
                    {
                        FTestSuck.Suck[i][j].Normal();
                    }
                }
                Task=600;
                break;
            case 600:
                if(MOT[MTestZ1].Gali_Two_ZAxis_Move(Prod.TestZ1_Safe, GotIndexZSpeed(iSpeedZ*1000), AnsiString(__FUNC__)+" 600"))
                    Task=9999;
                break;
            case 9999:                                                          // finish
                return true;
        }
    }
    return false;
}
//---------------------------------------------------------------------------
bool TfContact::DoArm2PlaceToShuttle(bool bIsFirst)
{
    if(TestIF.iShuttleMode==1)                                                  //Use Signal shuttle
    {
        if(TestIF.iShuttle_Sel==0)                                              //shuttle 1
        {
            return true;
        }
        else                                                                    //shuttle 2
        {
                                                                                //pass
        }
    }

    int &Task=iDoArm2PlaceToShuttleTask, pos=0;
    AnsiString ErrPart="";

    if(bIsFirst)
    {
        Task=1;
        return true;
    }
    else
    {
        if(W906_IndexZDriveFaultStop("DoArm2PlaceToShuttle")) { fAllMotorHome=false; CarlibrationTask=1; return false; }   switch(Task)   //AI(W906-E042) 20261005 (B3) P7 NOT GOLDEN: M14 drive alarm / ERROR_STOP / servo off / monitor lost or frozen / route fault -> ST + message + golden 536 exit; PCI1203 Z1 SHIP only, no-op elsewhere
        {
            case 1:
                Task=100;
                break;
            case 100:
                if(USE_INDEX_ARM_AXES==IndexArm_3_Axis)                         //JimmyChiu 20220708 : add Index Arm Axis
                {
                    if(MOT[MTestY1].Gali_MotMove(Prod.TestY1_Middle,GotIndexYSpeed(iSpeed*3000)))
                    {
                        Task=200;
                        htPlaceToShuttleDelay.SetSecAndOn(0.3);
                    }
                }
                else
                {
                    if(MOT[MTestY1].GalilTwoY_Move(Prod.TestY1_Front, Prod.TestY2_Rear, GotIndexYSpeed(iSpeed*3000), AnsiString(__FUNC__)+" 100"))
                    {
                        Task=200;
                        htPlaceToShuttleDelay.SetSecAndOn(0.3);
                    }
                }
                break;
            case 200:
                if(htPlaceToShuttleDelay.Off())
                {
                    if(USE_IO_CHANGE_TOQUE==true)                               //jou 2012-06-21 Enable index I/O Change Toque
                    {
                        SW[SwIndexChangeToque2].On();
                    }
                    pos=Get0_01MMType(edReleaseHeight2->Text.c_str())+Tech.iTestZ2ShutlePick;
                    if(pos<-9000 && abs(pos-Prod.TestZ2_Pick)>1000)
                    {
                        ErrPart.sprintf("Z2 Org=%d, New=%d", Prod.TestZ2_Pick, pos);
                        ShowMyMessage("The height difference is too big, please make sure and save data first.", ErrPart, ErrPart);
                        Prod.TestZ2_Pick=pos;
                    }
                    Task=300;
                }
                break;
            case 300:
                pos=Get0_01MMType(edReleaseHeight2->Text.c_str())+Tech.iTestZ2ShutlePick;
                if(MOT[MTestZ2].Gali_MotMove(pos, GotIndexZSpeed(iSpeedZ*1000)))
                {
                    #ifndef SOFT_SIMULTE
                    //AI(ht9045-v899) 20260522: check Gigas Contact Test Z2 drop before 3-axis place-to-shuttle release
                    if(CUSTOMER_CODE==CC_GIGAS &&
                       (iContactMode==CONTACT_AUTO_GET_HEIGHT ||
                        iContactMode==CONTACT_TEST) &&
                       chk_K_Temperature->Checked==false)
                    {
                        if(CheckIndexAllSuckICFallDown(false, true))
                        {
                            CheckIndexSuckICFallDownSetToHasNullIC(1);
                            break;
                        }
                    }
                    #endif

                    for(int i=0; i<BTestSuck.iShtRow; i++)
                    {
                        for(int j=0; j<BTestSuck.iShtCol; j++)
                        {
                            BTestSuck.Suck[i][j].Off();
                        }
                    }
                    hContactDeley.SetSecAndOn(2);                               //ChungHung 20140527 fix can not release 0.7--->2
                    Task=400;
                }
                break;
            case 400:
                if(hContactDeley.Off())
                    Task=500;
                break;
            case 500:
                for(int i=0; i<BTestSuck.iShtRow; i++)
                {
                    for(int j=0; j<BTestSuck.iShtCol; j++)
                    {
                        BTestSuck.Suck[i][j].Normal();
                    }
                }
                Task=600;
                break;
            case 600:
                if(MOT[MTestZ1].Gali_Two_ZAxis_Move(Prod.TestZ1_Safe, GotIndexZSpeed(iSpeedZ*1000), AnsiString(__FUNC__)+" 600"))
                    Task=9999;
                break;
            case 9999:
                return true;
        }
    }
    return false;
}
// ===== end of span C (golden :19501) =====
