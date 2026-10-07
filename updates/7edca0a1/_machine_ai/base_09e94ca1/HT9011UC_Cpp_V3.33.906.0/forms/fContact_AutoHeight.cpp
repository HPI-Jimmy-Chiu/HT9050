// =============================================================================
//  forms/fContact_AutoHeight.cpp -- E-042 = E-038 Phase B, step B1 (SAFETY): golden's Index Z1 Auto Height
//  state machine TfContact::Do_Z1_AutoGetHeight (golden 0618 cContact.cpp:5389-8168) + its file-scope timers
//  (:5384-5388) + the GetAutoHeightMaxKGTorque member (:5273-5382, delegating to the calc core).
//
//  AI(W906-E042) 20261004 (St01 / ST01-E): NEW FILE. Golden source of truth:
//  D:\HT9045\backup\HT9011UC_Code_V3.33.906.0_20260618\cContact.cpp (cp950, decoded strictly, 0 U+FFFD).
//  Plan: D:\AI_TempFile\st01e-e042-plan-20261004.md (Steven 1004 07:2x Q91 = A: St01 takes Phase B;
//  Q90 = A: no gravity baseline; 07:5x: 「io 馬達的裝置有異常error的時候，機台就不可以動」).
//
//  *** NO CALLER -- THIS BODY CANNOT MOVE ANYTHING TODAY ***
//    golden's callers are DoTestContactFunction case 400 (cContact.cpp:13088), itself not translated, and DoDeviceMapCheck :18168 (S-24)
//    (forms/fContact.h GATE (S-11)) and called from MainProc only inside `#if 0` (csystem.cpp:31320-31322).
//    tests/test_indexz_autoheight_1203.cpp [B9] pins both (census: 0 live callers outside the test).
//    Lifting that #if 0 is step B6: after 10/05 18:00, EastSun E-10, Gerneral.ini [IndexDriver]
//    HT9050_INDEXZ_TORQUE_CONFIRMED=1 (EastSun, by hand), Jimmy NIGHT_REPORT s0 #86, S-26 R4.
//
//  LINE MAP: the golden span below is VERBATIM and CONTIGUOUS -- port line = golden line + (-5284).
//  Every changed golden line is listed in the generator (scratch: D:\AI_TempFile\st01e-e042-run\gen_autoheight.py + autoheight_edits.py) and carries
//  an AI(W906-E042) tag ON THAT SAME LINE (inserts sit before the line's trailing //), so the offset never
//  drifts; [B10] pins it.
//
//  PORT PROTECTIONS ON THIS BODY (all AI(W906-E042) NOT GOLDEN; PCI1203 Z1 only via IndexZTorque1203.cpp,
//  SHIP builds only; RS-232 / Panasonic machines and SOFT_SIMULTE see golden byte for byte):
//    P7  W906_IndexZDriveFaultStop -- first statement of EVERY tick (the `switch(Task)` line): M14 drive
//        alarm / ERROR_STOP / servo off / monitor lost or frozen / route-latched failure -> "ST" (never
//        gated) + message + golden case 536's exit (fAllMotorHome=false; CarlibrationTask=1; return false).
//    P4+P8 W906_IndexZRunRefused -- case 1 entry: CONFIRMED != 1, BASELINE=1 (Q90), Mitsubishi, Direction 1, no hook, mode not 1 / 3 or not single arm (Steven 1004 08:4x) -> message, SystemStart=false, CarlibrationTask=1; nothing moved.
//        Its drive-health clause cannot fire HERE: P7 on the switch line runs first and takes the mid-run exit (golden reaches this case 1 only after DoZ1PickFromShuttle moved Z1); that clause is for DoTestContactFunction case 1 (B4).
//    P5  W906_IndexZTorqueWaitStop -- on the golden wait lines (case 555 golden :5767, case 7170 golden :6410): no
//        torque value for 5 s -> "ST" + message + the same exit.
//    P9  W906_IndexZCommandFloorStop -- on golden's Panasonic step line (case 560, golden :6069): the next step
//        would start from a COMMANDED position already at / below fIndexDownPos -> "ST" + message + exit.
//    P6  fIndexDownPos (golden encoder floor) stays verbatim.
//  Golden fContact -> port fContactForm is not needed here: the body is a TfContact member.
// =============================================================================
#include "forms/fContact.h"
#include "forms/fContact_AutoHeight.h"
#include "cContact.h"               // CONTACT_* (CONTACT_TEST since W906-E042), kgTranToMitsubishikg, ComputeAutoHeightMaxKGTorque
#include "MachineDefine.h"
#include "atester.h"                // DoSetupTest / DoTesterSidePush / iSetupTask / IsTest ...
#include "atester_shims.h"          // COM2 / ADAM_* / fiosetview / ATC_InterfaceForm shims (as atester.cpp)
#include "aTester_Front.h"
#include "aArmHeader.h"             // __FUNC__
#include "MachineType.h"
#include "Motor/mymotor.h"          // MOT[]
#include "Motor/HTMotor.h"          // iInposLed / iHomeLed
#include "mysensor.h"               // Sen[]
#include "myswitch.h"               // SW[]
#include "csystem.h"
#include "cprod.h"                  // Prod / Tech / TestIF / TestIF_File / DeviceForm / Temperature
#include "cmydef.h"
#include "cpublic.h"                // Get0_01MMType / ConvertTouMType
#include "common.h"
#include "Config.h"                 // IniConfig
#include "CosFunction.h"
#include "LastSet.h"
#include "aHotPlateSubstrate.h"     // FTestSuck / TestSocket -- THE aHotPlateSubstrate.h TMyKitSuck (as forms/fContact.cpp)
#include "FormsFacade.h"            // fMain
#include "canary_support.h"         // ShowMyMessage / ShowErrorMessage
#include "mycylin.h"                // Cylinder[]
#include "ckernel.h"                // WaitManualStepKey / WaitManualStartKey
#include "cinitial.h"               // ShowMainScreenPresure
#include "MessageDef.h"             // MSG_CMD_* / HHandler2Gpib
#include "Motor/myGALILmotor.h"     // ScanIndexMotorCanMove
#include "Interface/InterfaceSYS.h" // _RunTestProgram / _RunTestProgram_BarMess
#include "acarry_shims.h"           // ATC_InterfaceForm: TATC_InterfaceFormShim (:115, iATC_MODE_TYPE only) -- as atester.cpp
#include "forms/fATCHandlerSide.h"  // ATC_TYPE_70 (:697)
#include "ATC/ATCInterface.h"       // ATCInterfaceForm (the old ATC system)
#include "forms/fHome.h"            // fHome->InitDoTestZHome
#include "forms/fTestCategory.h"    // fTestCategory
#include "forms/fDynamicTemp.h"     // fDynamicTemp
#include "IndexZTorque1203.h"       // P4 / P5 / P7 / P8 / P9 (AI(W906-E042))
#include <cstdlib>
#include <cmath>

//  golden cContact.cpp:93-98 -- file-scope globals the Auto Height / contact state machines share (bDoRTCLearning,
//  golden :96, already lives in cprod.cpp:129).  AI(W906-E042): the port had none of the others (0 references);
//  exported through forms/fContact_AutoHeight.h so the laptop's CT-3c Index jog (golden :16950-17052) can read
//  iIndexStatus.
int iIndexStatus;                                                               //Ifor 20170105 (Steven) add Index 到位後顯示 Index Arm Jog Move 工具
int iContactJogMove, iContactJogSpeed;                                          //Alick 20170116 add
bool bContactJogFlag;                                                           //Alick 20170116 add
double iTotalOffset_1=0;                                                        //Ifor 20200318 : add 鋼徑80mmn 因為機台Hold不住變形需補下壓高度
double iTotalOffset_2=0;                                                        //Ifor 20200318 : add 鋼徑80mmn 因為機台Hold不住變形需補下壓高度

//  golden cContact.cpp:5259 -- file scope, right before GetAutoHeightMaxKGTorque (internal linkage).
const int SafeTestZContactHeight=-8000;

//  AI(W906-E042) DEVIATION (forms/fContact.cpp D-8, same reason): golden passes `X->Text.c_str()` to
//  Get0_01MMType(char*) (cpublic.h:19), which BCB6 allowed because its c_str() returned char*.  The body only
//  reads the text (`double i=atof(str); ...`, cpublic.cpp:411-418), so this TU-local overload forwards a
//  const_cast instead of touching the golden lines.
static inline int Get0_01MMType(const char* s) { return Get0_01MMType(const_cast<char*>(s)); }
//  Same for the SPEA arm's `_RunTestProgram_BarMess(sizeof(flag), flag, 0, "")` (golden passes "" as Byte*; the body
//  never reads it when iMessageSize is 0 -- Interface/InterfaceSYS.cpp:224).  SPEA only (TestIF.iGpibMode).
static inline bool _RunTestProgram_BarMess(int n, bool* s, int m, const char* msg) { return _RunTestProgram_BarMess(n, s, m, (Byte*)const_cast<char*>(msg)); }

// ===== golden 0618 cContact.cpp:5384-8168, verbatim and contiguous: port line = golden line + (-5284) =====
TQPF_Timer Do_Z1_AutoGetHeightDelay;
TQPF_Timer Z1ClampCloseDelay;                                                   //JerryYang 20160524
TQPF_Timer Z1ClampOpenDelay;                                                    //JerryYang 20160524
TQPF_Timer Z1ClampTimeOutDelay;                                                 //JerryYang 20160524
TQPF_Timer Z2_Down_Delay;
bool TfContact::Do_Z1_AutoGetHeight()   //AI(W906-E042) 20261004: golden `bool __fastcall` -- __fastcall dropped as in forms/fContact.cpp (the declaration has none)
{
    static int iBackTask=1;
    static int iTestSec=0;                                                      //KevinCheng 20260115 : NV Contact Test
    static int iTestSidePushcount=0;                                            //Richard 20220321
    static int iHeightStep=0;
    static int kg=0, vkg=0;                                                     //Steven 20100604 : For Mitsubishi
    static int Counter=0, Pos=0, PosY1=0, iStepSpeed=0, iManualSpeed=100;
    static int iRecordZ1Pos=0, iRecordZ1Pos1=0, iRecordZ1Pos2=0;
    static int iPosZ[2]={0, 0};
    static bool bZMotMove=false;
    static bool bArm2Test=false;                                                //Ifor 20200708 add: Arm1 Pick up Arm2 Test 模式下 Arm2 在Socket裡面
    static bool bZ1Z2Press, bEPLeakage=false;
    static bool bRecordZ1Pos=false;
    static bool bAlarmGoTask560=true;
    static bool bSHMotMove[2]={false, false};
    static bool bCheckICFall[4][8]={{false, false, false, false, false, false, false, false},
                                    {false, false, false, false, false, false, false, false},
                                    {false, false, false, false, false, false, false, false},
                                    {false, false, false, false, false, false, false, false}};                          //Steven 20100129

    static double dHeight[2]={0.0, 0.0};

    int &Task=Z_Height_Task, ret=0, iZPos=0, iNN=IsNNMode();
    int iSiteOn[4]={0, 0, 0, 0};
    bool bCheckDestroy=false;
    bool flag=false, bflag=false;                                               //kevin 20140612
    bool bHasErr=false;
    double fDropPos=0.0;
    TEdit *tempEdit[]={edPinCount, edForcePerPinN, edForcePerPinG};
    AnsiString ErrPart="";
    ShowMainScreenPresure(0);                                                   //kevin 20130605 read Torque send gpib use
    AnsiString str, str1;

    if(W906_IndexZDriveFaultStop("Do_Z1_AutoGetHeight")) { fAllMotorHome=false; CarlibrationTask=1; return false; }   if(W906_IndexZShuttleHomeStop(Task, "Do_Z1_AutoGetHeight")) { fAllMotorHome=false; CarlibrationTask=1; return false; }   switch(Task)   //AI(W906-E042) 20261004 P7 NOT GOLDEN: M14 drive alarm / ERROR_STOP / servo off / monitor lost or frozen / route fault -> ST + message + golden 536 exit; PCI1203 Z1 SHIP only, no-op elsewhere.  AI(W906-E042) 20261005 (B3) W-44 NOT GOLDEN (Steven; ST01-M 1005 03:4x socket press only): a press task with the In / Out shuttle away from home -> ST + message + golden 536 exit
    {
        case 1: if(W906_IndexZRunRefused(iContactMode, 0, "Do_Z1_AutoGetHeight 1")) { SystemStart=false; CarlibrationTask=1; return false; }   /*AI(W906-E042) 20261004 P4+P8+P7 NOT GOLDEN: CONFIRMED / BASELINE (Q90) / mode whitelist / drive health*/                                                                                                                                                  //Index1 確認Index Device 是否掉料 Z1回安全位置
            dHeight[0]=0.0;
            dHeight[1]=0.0;
            iHeightStep=0;

            bHasErr|=CheckIndexAllSuckICFallDown(true, false);                                                                                                  //Steven 20131128 : 換位置
            bArm2Test=false;                                                                                                                                    //Ifor 20200708 add: Arm1 Pick up Arm2 Test 模式下 Arm2 在Socket裡面

            for(int i=0; i<FTestSuck.iShtRow; i++)                                                                                                              //Steven 20110516 Start: IC掉落要Alarm,並整合成一次
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
                if(CosFunction.bJAM0303NeedOpenChamberDoor)                                                                                                     //Steven : JAM0303 & JAM0403需要開啟Chamber門10秒
                    bIsTestSitICFallDown=true;

                ShowErrorMessage("JAM0303", K_SKIP, MTestZ1, false, ErrPart);                                                                                   //Steven 20100129 : Device Drop Error
                for(int i=0; i<FTestSuck.iShtRow; i++)
                {
                    for(int j=0; j<FTestSuck.iShtCol; j++)
                    {
                        if(FTestSuck.Suck[i][j].Error ||
                           (FTestSuck.Item[i][j]!=HAS_NULL_IC &&
                            FTestSuck.Item[i][j]!=NULL_IC &&
                            FTestSuck.Suck[i][j].GetStatus()==false))                                                                                           //有用到且有吸到IC的卻掉了
                        {
                            FTestSuck.SetItemData(i, j, HAS_NULL_IC);                                                                                           //Steven 20110829 : 把有IC掉料的位置改成Has Null IC
                            FTestSuck.Suck[i][j].Normal();                                                                                                      //Steven 20110829 : 把真空關掉
                            bCheckICFall[i][j]=false;
                        }
                    }
                }
                break;
            }

            for(int i=0; i<FTestSuck.iShtRow; i++)                                                                                                              //Steven 20100129 Start: IC掉落要Alarm
            {
                for(int j=0; j<FTestSuck.iShtCol; j++)
                {
                    if(FTestSuck.Suck[i][j].GetStatus()==true)                                                                                                  //有用到且有吸到IC的
                    {
                        bCheckICFall[i][j]=true;
                    }
                    else
                    {
                        FTestSuck.Suck[i][j].Normal();
                        bCheckICFall[i][j]=false;
                    }
                }
            }

            if(ATC_SYSTEM==eATCHonPrecType && ATC_SYSTEM==eATCHonPrecType)                                                                                      //Dell 20140509
                ATCInterfaceForm->ATC_SYS.SetNowArm(0);                                                                                                         //ATC

            if(ATC_SYSTEM==eATC60 || ATC_SYSTEM==eATC30)                                                                                                        //20141204 ChungHung add for ATC3.0 //2014-05-30    Dell    for ATC6.0
            {
                ATCInterfaceForm->ATC_60_SYS.SetHandlerNowArm(0);
            }
            else if(ATC_SYSTEM==eNewATCSystem)                                                                                                                  //Ifor 20151230 :add New ATC Interface HandlerArm
            {
                {} /*GATE(W906-E042-ATC) ATC_InterfaceForm->HandlerArm(0); -- AI(W906-E042): as above (the ATC 'which arm' notice)*/
            }

            if(TestIF.bUseSLKClamp==true && TestIF_File.iSeparabilityTest==1)                                                                                   //JerryYang 20160616 分離流程
            {
                if(Cylinder[C_SLK1_Clamp].OnSensor()==false)
                {
                    ShowMyMessage("Please check the clamp I/O of arm1 layout kit", "請確認Arm1 layout kit clamp汽缸的I/O是否正常");
                    return false;
                }
            }

            if(INSTALL_SOCKET_CLAMP)                                                                                                                            //JerryYang 20160607 機台選用分離機構 需偵測socket sensor
            {
                if(Sen[SnSocketHasClamp1].IsOn() || Sen[SnSocketHasClamp2].IsOn())
                {
                    ShowMyMessage("Please check the socket sensor","socket sensor偵測異常");
                    return false;
                }
            }

            EPSwitchOnOff(eEPSwBoth);
            bZMotMove=MOT[MTestZ1].Gali_Two_ZAxis_Move(Prod.TestZ1_Safe, GotIndexZSpeed(iSpeedZ*1000), "Do_Z1_AutoGetHeight 1");                                //JimmyChiu 20211028 : All speed can set by speed setting.

            if(bZMotMove)
            {
                palArm1Height->Caption=0;
                bRecordZ1Pos=false;
                if(USE_OUT_SHT_MOT==1 || MachineTypeChoice==Type_HT502 || W906_IndexZLive1203())   //AI(W906-E042) 20261005 (B3b) NOT GOLDEN: a live PCI-1203 Index Z (HT9050: no Index Y, USE_OUT_SHT_MOT=0 on the machine) takes golden's own shuttle branch -- case 150/151 move In shuttle 1 to InSHT[0].iLeft and Out shuttle 1 to OutSHT[0].iRight before the socket press (W-44 checks exactly these) -- instead of case 100, the Index-Y path; same test as the Do_Z2 arm-2 refusal; USE_OUT_SHT_MOT itself is unchanged; 910 HT9050 cContact.cpp:5539 has the same golden line
                    Task=150;
                else
                    Task=100;
            }
            break;
        case 100:                                                                                                                                               // IndexY1 移至Socket IndexY2 移至Shuttle2上方
            if(MOT[MTestY1].GalilTwoY_Move(Prod.TestY1_Middle, Prod.TestY2_Rear, GotIndexYSpeed(iSpeed*3000), "Do_Z1_AutoGetHeight 100"))                       //JimmyChiu 20211028 : All speed can set by speed setting.
                Task=110;
            break;
        case 110:                                                               //Index 確認是否有Device 掉落
            bHasErr=false;
            for(int i=0; i<FTestSuck.iShtRow; i++)
            {
                for(int j=0; j<FTestSuck.iShtCol; j++)
                {
                    if(bCheckICFall[i][j]==true &&
                       FTestSuck.Suck[i][j].GetStatus()==false)                 //有用到且有吸到IC的卻掉了
                    {
                        ErrPart+=IndexSuckName[i+iNN][j];
                        bHasErr=true;
                    }
                }
            }

            if(LastSet.iRealDummy==REALLY && bHasErr)
                Task=2200;
            else
                Task=200;
            break;
        case 150:
            iPosZ[0]=MOT[MInShuttle1].ReadPos();
            iPosZ[1]=MOT[MOutShuttle1].ReadPos();
            if(iPosZ[0]==Prod.InSHT[0].iLeft && iPosZ[1]==Prod.OutSHT[0].iRight)
            {
                Task=110;
            }
            else
            {
                bSHMotMove[0]=false;
                bSHMotMove[1]=false;
                Task=151;
            }
            break;
        case 151:
            if(bSHMotMove[0]==false)
            {
                bSHMotMove[0]=MOT[MInShuttle1].MotorMove(Prod.InSHT[0].iLeft);
            }
            else
            {
                bSHMotMove[0]=true;
            }

            if(bSHMotMove[1]==false)
            {
                bSHMotMove[1]=MOT[MOutShuttle1].MotorMove(Prod.OutSHT[0].iRight);
            }
            else
            {
                bSHMotMove[1]=true;
            }

            if(bSHMotMove[0] && bSHMotMove[1])
            {
                Task=110;
            }
            break;
        case 200:                                                               //IndexZ 回Home
            fHome->InitDoTestZHome();
            Task=520;
            break;
        case 520:                                                               //依照個模式執行動作 CONTACT_TEST--->2900   CONTACT_AUTO_GET_HEIGHT--->530
            if(iContactMode==CONTACT_MANUAL_GET_HEIGHT)                         // 手動  K
            {
                ADAM_WriteVoltage(DeviceForm.dPress);
                SW[SwManualZ1].On();
                if(IniConfig.bD10ManualHeightComptibleWithNS)
                {
                    SW[SwManualZ2].Off();
                    Task=530;
                }
                else
                {
                    MotorStatus=false;
                    Task=950;
                }
            }
            else if(iContactMode==CONTACT_TEST ||                               // contact test
                    iContactMode==STEP_CONTACT_TEST ||                          //Steven 20150811 : Step by Step Contact Test
                    iContactMode==CONTACT_DEVICE_MAP_CHECK)                     //Steven 20191009 : Device Map for Qualcomm
            {
                ADAM_WriteVoltage(DeviceForm.dPress);
                if(iContactMode==CONTACT_TEST &&
                   (cbContactMode->ItemIndex==DirectContactSoftEP ||
                    cbContactMode->ItemIndex==DropContactSoftEP))
                {                                                               //kevin 20130608 馬達到定點 ep 再充氣
//                    bContSoftEpSwitch(0, false);                                //ARM1 浮動頭不充氣    //JerryYang 20151202 true->flase
                    EPSwitchOnOff(eEPSwArm2);
                }

                if(EP_Install==3 && IniConfig.bD24EnableEPCheckFuntion==true)   //JerryYang 20240111 : add
                {
                    Do_Z1_AutoGetHeightDelay.SetSecAndOn(2);
                    Task=2800;
                }
                else
                {
                    Task=2900;
                }
            }
            else                                                                // CONTACT_AUTO_GET_HEIGHT
            {
                if(IniConfig.iD17_UseHardwareHeightToContact==3)                //Steven 20231025 : K兩次作比對
                {
                    if(iHeightStep==0)
                        ADAM_WriteMaxData(true);                                //Steven 20241014 : 整合auto height輸出壓力
                    else
                        ADAM_WriteMaxData(false);
                }
                else if(IniConfig.iD17_UseHardwareHeightToContact==2)           //Steven 20231025 : 不充氣K高
                {
                    ADAM_WriteVoltage(dMinForce/2.0);
                }
                else
                {
                    ADAM_WriteMaxData(true);                                    //Steven 20241014 : 整合auto height輸出壓力
                }

                for(int i=0; i<3; i++)                                          //Steven 20100624 : K高過程不可以改變
                   tempEdit[i]->Enabled=false;

                Task=530;

                if(CUSTOMER_CODE==CC_AMKOR_China ||                             //jou 2013-04-25 因為 HT9045W 增加鋼瓶,充飽氣需要5 sec的時間,所以增加等待時間
                   CUSTOMER_CODE==CC_QUALCOMM)                                  //JerryYang 20170412 (Steven) add QUALCOMM
                {
                    if(INDEX_PRESS_TYPE==e85KG && INDEX_SUCKER_TYPE==0)
                    {
                        Do_Z1_AutoGetHeightDelay.SetSecAndOn(3);
                        Task=525;
                    }
                }
            }
            break;
        case 525:
            if(Do_Z1_AutoGetHeightDelay.Off())
            {
                Task=530;
            }
            break;
        case 530:                                                                                                       //IndexZ1移至StandBy位置後設定扭力值
            if(IniConfig.iD17_UseHardwareHeightToContact==3 && iHeightStep==1)                                          //Steven 20231025 : 不充氣K高
            {
                bZMotMove=true;
            }
            else
            {
                bZMotMove=(MOT[MTestZ1].Gali_MotMove(Tech.iTestZDown-5000, GotIndexZSpeed(iSpeedZ*1000)));              //JimmyChiu 20211028 : All speed can set by speed setting.
            }

            if(bZMotMove==true)
            {
                if(iContactMode==CONTACT_MANUAL_GET_HEIGHT)                                                             // 手動  K
                    ADAM_WriteVoltage(DeviceForm.dPress);

                InitWriteAndCheckMotorTorqueTask();
                kg=GetAutoHeightMaxKGTorque();                                                                          //Steven 20170720 (wei) : for low contact force
                Task=535;
                iManualSpeed=100;                                                                                       //Steven 20100208 :10->100 加快速度
            }
            break;
        case 535:
            bZ1Z2Press=false;
            Task=536;
            break;
        case 536:                                                               //確認扭力值設定完成
            if(USE_IO_CHANGE_TOQUE==true)                                       //jou 2012-06-21 Enable index I/O Change Toque
            {
                SW[SwIndexChangeToque1].Off();
                SW[SwIndexChangeToque2].Off();
            }

            if(INDEX_DRIVER_TYPE==Panasonic_DRIVER)                             //Panasonic
            {
                ret=COM2->iWriteAndCheckMotorTorque(0, kg);
            }
            else
            {
                vkg=kgTranToMitsubishikg(kg);
                ret=COM2->iWriteAndCheckMotorTorque(0, vkg);
            }

            if(ret==1)
            {
                Task=550;
                Counter=0;

                if(IniConfig.iD17_UseHardwareHeightToContact==3 && iHeightStep==1)
                    iStepSpeed=100;
                else
                    iStepSpeed=300;

                if(ATC_InterfaceForm->iATC_MODE_TYPE==ATC_TYPE_70 &&
                   Temperature.bATC7TSDFunction==true)                          //Steven 20161201 : by site TSD for Contact Test
                {
                    iSiteOn[0]=(FTestSuck.Item[0][0]==HAS_IC || FTestSuck.Item[0][0]==HAS_HOT_IC)?1:0;
                    iSiteOn[1]=(FTestSuck.Item[0][1]==HAS_IC || FTestSuck.Item[0][1]==HAS_HOT_IC)?1:0;
                    iSiteOn[2]=0;
                    iSiteOn[3]=0;
                    { (void)iSiteOn; } /*GATE(W906-E042-ATC) ATC_InterfaceForm->UseTSD_Function(4, iSiteOn); -- AI(W906-E042): ATC_InterfaceForm is TATC_InterfaceFormShim (acarry_shims.h:115, iATC_MODE_TYPE only); the real TATC_InterfaceForm (forms/fATCHandlerSide.h) has no global -- same gate as atester.cpp R04 / T04 (W906-GB-P2b); HT9050 USE_ATC_MODE=5 = no ATC*/
                }
                ATC_SwitchTjSignal(1);                                          //Ifor 20210622 add: ATC Switch TJ
                fMain->SendMSG_CMD(MSG_CMD_Arm1Down);                           //Ifor 20210331 :add Contact Mode GPIB需要知道哪隻Arm在Socker
                if(ATC_SYSTEM==eNewATCSystem &&
                   Temperature.bATCActiveCooling==true)                         //JerryYang 20220815 : send ATC which ARM
                {
                    {} /*GATE(W906-E042-ATC) ATC_InterfaceForm->HandlerArm(0); -- AI(W906-E042): as above (the ATC 'which arm' notice)*/
                }
                iWhichArmDown=1;
            }
            else if(ret==2)
            {
                ShowMyMessage("Index Z1 Motor torque set error", "馬達扭力設定錯誤!", "Do_Z1_AutoGetHeight 536");
                fAllMotorHome=false;
                CarlibrationTask=1;
                return false;
            }
            break;
        case 550:                                                               //讀取扭力值並顯示
            COM2->InitReadTorueTask();
            fMain->chkReadTorque1->Checked=true;
            fMain->chkReadTorque2->Checked=false;
            fMain->edTorue0->Text="";
            bReadMCU2=true;                                                     //kevin 20220225 read MCU DATA
            Task=555;
            break;
        case 555:                                                                                                       //讀取扭力值與設定值比較後EP洩氣並取得IndexZ 高度
            #ifndef SOFT_SIMULTE
                if(W906_IndexZTorqueWaitStop(0, fMain->edTorue0->Text=="", "Do_Z1_AutoGetHeight 555")) { fAllMotorHome=false; CarlibrationTask=1; return false; }   if(fMain->edTorue0->Text=="")   //AI(W906-E042) 20261004 P5 NOT GOLDEN: 5 s without a torque value -> ST + message + golden 536 exit (golden waits for ever)
                    break;
            #else
                fMain->edTorue0->Text=30;
            #endif

            Pos=MOT[MTestZ1].Gali_ReadPos();
            edContactHeight1->Text=ConvertTouMType(MOT[MTestZ1].Gali_ReadEncoderPos());
            TorqueData=atoi(fMain->edTorue0->Text.c_str());

            if(INDEX_DRIVER_TYPE==Mitsubishi_DRIVER)
            {
                PnlTorue0->Caption="";
                if(TorqueData>=kg-6)                                                                                    // 2010.05.19 , Joye
                {
                    TorqueData=kg;
                }
            }

            if(INDEX_DRIVER_TYPE==Panasonic_DRIVER)                                                                     //Panasonic
                TorqueData=abs(TorqueData);

            if(TorqueData>=kg ||
               atof(edContactHeight1->Text.c_str())<=fIndexDownPos)                                                     //jou 2010-06-29 start : auto high 如不裝 socket base 需可執行結束,直接設入最大容許值，並show出alarm!
            {
                if(IniConfig.bD10ManualHeightComptibleWithNS &&
                   iContactMode==CONTACT_MANUAL_GET_HEIGHT)
                {
                    MOT[MTestZ1].Gali_Command("ST", __FUNC__);
                    fMain->SendMSG_CMD(MSG_CMD_Arm1Down);                                                               //Ifor 20210331 :add Contact Mode GPIB需要知道哪隻Arm在Socker
                    if(ATC_SYSTEM==eNewATCSystem &&
                       Temperature.bATCActiveCooling==true)                                                             //JerryYang 20220815 : send ATC which ARM
                    {
                        {} /*GATE(W906-E042-ATC) ATC_InterfaceForm->HandlerArm(0); -- AI(W906-E042): as above (the ATC 'which arm' notice)*/
                    }
                    iWhichArmDown=1;
                    Task=560;
                }
                else
                {
                    Counter++;
                    if(INDEX_DRIVER_TYPE==Mitsubishi_DRIVER)
                        PnlTorue0->Caption=TorqueData;

                    if(Counter>=10)                                                                                     // avoid torque on only short time
                    {
                        Counter=0;
                        Pos=MOT[MTestZ1].Gali_ReadPos();
                        MOT[MTestZ1].Gali_Command("ST", __FUNC__);

                        if(bRecordZ1Pos==false)                                                                         //取得充飽氣的高度
                        {
                            iRecordZ1Pos1=MOT[MTestZ1].Gali_ReadEncoderPos();
                            edContactHeight1->Text=ConvertTouMType(iRecordZ1Pos1);

                            if(atof(edContactHeight1->Text.c_str())<=fIndexDownPos)                                     //jou 2010-06-29 start : auto high 如不裝 socket base 需可執行結束,直接設入最大容許值，並showt出alarm!
                            {
                                iRecordZ1Pos1=fIndexDownPos*100;
                                edContactHeight1->Text=fIndexDownPos;
                                ShowMyMessage("Attention!! Over Z1 contact high! Be sure!","注意!!超過Z1 contact高度!需確認!");
                            }

                            if(EP_Install==3 || EP_Install==5)
                            {
                                if(IniConfig.bD26EnableEPEncoderRange==true)                                            //ChungHung 20111217
                                {
                                    ADAM_Rang(IniConfig.iD26EPEncoderRange);
                                    if(EP_Install==5)
                                        bEPLeakage=ADAM_Alarm(0);
                                    else
                                        bEPLeakage=ADAM_Alarm();
                                }
                                else
                                {
                                    bEPLeakage=false;
                                }
                            }
                            else
                            {
                                if(Sen[SnEPAlarm].Enable==true)
                                {
                                    bEPLeakage=Sen[SnEPAlarm].IsOff();                                                  //Steven 20110622 : 檢查EP有沒有漏
                                }
                                else
                                {
                                    bEPLeakage=false;
                                }
                            }

                            ADAM_WriteVoltage(0);
                            MySleep(3000);
                            ADAM_WriteVoltage(0);
                            bRecordZ1Pos=true;
                            Task=700;
                        }
                        else                                                                                            //取得洩氣的高度
                        {
                            iRecordZ1Pos2=MOT[MTestZ1].Gali_ReadEncoderPos();
                            edContactHeight1->Text=ConvertTouMType(iRecordZ1Pos2);
                            Task=700;
                        }
                    }
                    else
                    {
                        Task=550;
                    }
                }
            }
            else
            {
                Counter=0;
                fMain->SendMSG_CMD(MSG_CMD_Arm1Down);                                                                   //Ifor 20210331 :add Contact Mode GPIB需要知道哪隻Arm在Socker
                if(ATC_SYSTEM==eNewATCSystem &&
                   Temperature.bATCActiveCooling==true)                                                                 //JerryYang 20220815 : send ATC which ARM
                {
                    {} /*GATE(W906-E042-ATC) ATC_InterfaceForm->HandlerArm(0); -- AI(W906-E042): as above (the ATC 'which arm' notice)*/
                }
                iWhichArmDown=1;
                Task=560;
            }
            break;
        case 560:
            bAlarmGoTask560=true;
            if(MachineTypeChoice!=Type_HT502)
            {
                PosY1=MOT[MTestY1].Gali_ReadPos();
                PosY1-=Prod.TestY1_Middle;
                if(abs(PosY1)>10)                                                                                                                               //JerryYang 20160824 Start:Z1下降前增加防護
                {
                    ShowMyMessage("Index Y1 Motor Position error", "馬達Y1位置錯誤!", "Do_Z1_AutoGetHeight 560");
                    return false;
                }
            }

            iZPos=MOT[MTestZ1].Gali_ReadPos();                                                                                                                  //kevin 20140612 add start
            if(iZPos>(Prod.TestZ1_Test-Prod.TestZ1_Drop_Offset+750))
            {
                if(ATC_InterfaceForm->iATC_MODE_TYPE==ATC_TYPE_70 &&
                   Temperature.bATC7TSDFunction==true)                                                                                                          //Steven 20161201 : by site TSD for Contact Test
                {
                    iSiteOn[0]=(FTestSuck.Item[0][0]==HAS_IC || FTestSuck.Item[0][0]==HAS_HOT_IC)?1:0;
                    iSiteOn[1]=(FTestSuck.Item[0][1]==HAS_IC || FTestSuck.Item[0][1]==HAS_HOT_IC)?1:0;
                    iSiteOn[2]=0;
                    iSiteOn[3]=0;
                    { (void)iSiteOn; } /*GATE(W906-E042-ATC) ATC_InterfaceForm->UseTSD_Function(4, iSiteOn); -- AI(W906-E042): ATC_InterfaceForm is TATC_InterfaceFormShim (acarry_shims.h:115, iATC_MODE_TYPE only); the real TATC_InterfaceForm (forms/fATCHandlerSide.h) has no global -- same gate as atester.cpp R04 / T04 (W906-GB-P2b); HT9050 USE_ATC_MODE=5 = no ATC*/
                }
                ATC_SwitchTjSignal(1, false);                                                                                                                   //Ifor 20210622 add: ATC Switch TJ
                flag=false;
                if(LastSet.iRealDummy==REALLY)
                {
                    for(int i=0; i<FTestSuck.iShtRow; i++)
                    {
                        for(int j=0; j<FTestSuck.iShtCol; j++)
                        {
                            if(FTestSuck.Suck[i][j].Enable       &&                                                                                             //Steven 20110725 : 不再使用IsSuckICFallDown
                               FTestSuck.Suck[i][j].SenUsing!="" &&
                               FTestSuck.Item[i][j]!=HAS_NULL_IC &&
                               FTestSuck.Item[i][j]!=NULL_IC)
                            {
                                if(FTestSuck.Suck[i][j].GetStatus()==false)
                                {
                                    FTestSuck.Suck[i][j].Normal();                                                                                              //jou 2012-01-17 直接關掉，避免掉到shuttle去，也避免要掉不掉Hang up
                                    flag=true;
                                    bRearHeadICFallDown=true;                                                                                                   //kevin 20131120 發ALARM 開CHAMBO門 按Z1
                                }
                            }
                        }
                    }
                }

                if(flag)
                {
                    MOT[MTestZ1].Gali_Command("ST", __FUNC__);
                    Task=564;
                    return false;
                }
            }

            if(IniConfig.bD10ManualHeightComptibleWithNS &&
               iContactMode==CONTACT_MANUAL_GET_HEIGHT)
            {
                if(Sen[SnFMotorDown].IsOff()==false && TorqueData<kg)                                                                                           //下降
                {
                    if(INDEX_MOTION_CARD==0)
                        MOT[MTestZ1].Gali_JogNSetup(iManualSpeed);
                    else
                        MOT[MTestZ1].Motor->JogN();
                    bZ1Z2Press=true;
                    if(iManualSpeed<5000)
                        iManualSpeed+=100;
                }
                else if(Sen[SnBMotorDown].IsOff()==false)                                                                                                       //上升
                {
                    Pos=MOT[MTestZ1].Gali_ReadPos();
                    if(Pos<Tech.iTestZDown)
                    {
                        bZ1Z2Press=true;
                        if(INDEX_MOTION_CARD==0)
                            MOT[MTestZ1].Gali_JogPSetup(iManualSpeed);
                        else
                            MOT[MTestZ1].Motor->JogP();
                        if(iManualSpeed<5000)
                            iManualSpeed+=100;
                    }
                }
                else
                {
                    iManualSpeed=100;                                                                                                                           //Steven 20100208 :10->100 加快速度
                    MOT[MTestZ1].Gali_Command("ST", __FUNC__);
                    if(WaitManualStartKey())                                                                                                                    //Test IC
                    {
                        if(ATC_InterfaceForm->iATC_MODE_TYPE==ATC_TYPE_70 &&
                           Temperature.bATC7TSDFunction==true)                                                                                                  //Steven 20161201 : by site TSD for Contact Test
                        {
                            iSiteOn[0]=(FTestSuck.Item[0][0]==HAS_IC || FTestSuck.Item[0][0]==HAS_HOT_IC)?1:0;
                            iSiteOn[1]=(FTestSuck.Item[0][1]==HAS_IC || FTestSuck.Item[0][1]==HAS_HOT_IC)?1:0;
                            iSiteOn[2]=0;
                            iSiteOn[3]=0;
                            { (void)iSiteOn; } /*GATE(W906-E042-ATC) ATC_InterfaceForm->UseTSD_Function(4, iSiteOn); -- AI(W906-E042): ATC_InterfaceForm is TATC_InterfaceFormShim (acarry_shims.h:115, iATC_MODE_TYPE only); the real TATC_InterfaceForm (forms/fATCHandlerSide.h) has no global -- same gate as atester.cpp R04 / T04 (W906-GB-P2b); HT9050 USE_ATC_MODE=5 = no ATC*/
                        }
                        ATC_SwitchTjSignal(1);                                                                                                                  //Ifor 20210622 add: ATC Switch TJ
                        fMain->SendMSG_CMD(MSG_CMD_ContactTestArm1);                                                                                            //Steven 20150304 : Add GPIB LOG
                        {} /*GATE(W906-E042-UI) fTestCategory->BringToFront(); -- AI(W906-E042): window z-order only; TfTestCategory has no base class / BringToFront (forms/fTestCategory.h:257 D-2)*/                                                                                                          //Steven 20100823
                        if(IniConfig.bC04EnableTestTempIC)
                            {} /*GATE(W906-E042-UI) fDynamicTemp->BringToFront(); -- AI(W906-E042): window z-order only; fDynamicTemp has no global (forms/fContact.h:458-461 X-13)*/                                                                                                       //Steven 20120810
                        iSetupTask=1;
                        IsTest=true;                                                                                                                            //Steven 20110920
                        Task=600;
                        labDelayStatus->Caption="Waiting Test Result";
                        break;
                    }

                    if(WaitManualStepKey())
                    {
                        if(ATC_InterfaceForm->iATC_MODE_TYPE==ATC_TYPE_70 &&
                           Temperature.bATC7TSDFunction==true)                                                                                                  //Steven 20161201 : by site TSD for Contact Test
                        {
                            iSiteOn[0]=0;
                            iSiteOn[1]=0;
                            iSiteOn[2]=0;
                            iSiteOn[3]=0;
                            { (void)iSiteOn; } /*GATE(W906-E042-ATC) ATC_InterfaceForm->UseTSD_Function(4, iSiteOn); -- AI(W906-E042): ATC_InterfaceForm is TATC_InterfaceFormShim (acarry_shims.h:115, iATC_MODE_TYPE only); the real TATC_InterfaceForm (forms/fATCHandlerSide.h) has no global -- same gate as atester.cpp R04 / T04 (W906-GB-P2b); HT9050 USE_ATC_MODE=5 = no ATC*/
                        }

                        iTriggerBoostFunction=-1;
                        iTriggerBoostFuncBack=-1;
                        iBoostFuncStep=5;
                        edContactHeight1->Text=ConvertTouMType(MOT[MTestZ1].Gali_ReadEncoderPos());

                        InitWriteAndCheckMotorTorqueTask();
                        SW[SwRKManualTStart].Off();
                        IsTest=false;                                                                                                                           //Steven 20120505 Start: 離開時,要通知GPIB重置測試狀態
                        TestSocket.ClearAll();
                        if(TestIF.iGpibMode==InterfaceType_SPEA_Type)
                        {
                            bool flag[32]={false};
                            if(BAR_CODE_INSTALL!=ebctUninstall &&
                               TestIF_File.bEnableBarCode)
                                _RunTestProgram_BarMess(sizeof(flag), flag, 0, "");
                            else
                                _RunTestProgram(sizeof(flag), flag);
                        }
                        else
                        {
                            fMain->RunTestProgram(false);
                        }

                        fMain->SendMSG_CMD(MSG_CMD_ContactTestAbort);                                                                                           //Steven 20150304 : Add GPIB LOG
                        labDelayStatus->Caption="";
                        Task=900;
                        break;
                    }
                }
                Task=550;
            }
            else
            {
                if(INDEX_DRIVER_TYPE==Mitsubishi_DRIVER)
                {
                    if(TorqueData>3)
                    {
                        if(INDEX_MOTION_CARD==0)
                            bZMotMove=MOT[MTestZ1].Gali_MotMoveSkipEncoder(Pos-iStepSpeed/10, GotIndexZSpeed(iSpeedZ*1000));                                    //JimmyChiu 20211028 : All speed can set by speed setting.
                        else
                            bZMotMove=MOT[MTestZ1].MotorMove(Pos-iStepSpeed/10);

                        if(bZMotMove)
                            Task=550;
                    }
                    else
                    {
                        if(INDEX_MOTION_CARD==0)
                            bZMotMove=MOT[MTestZ1].Gali_MotMoveSkipEncoder(Pos-iStepSpeed, GotIndexZSpeed(iSpeedZ*1000));                                       //JimmyChiu 20211028 : All speed can set by speed setting.
                        else
                            bZMotMove=MOT[MTestZ1].MotorMove(Pos-iStepSpeed);

                        if(bZMotMove)
                            Task=550;
                    }
                }
                else
                {
                    if(W906_IndexZCommandFloorStop(Pos, fIndexDownPos, "Do_Z1_AutoGetHeight 560")) { fAllMotorHome=false; CarlibrationTask=1; return false; }   if(INDEX_MOTION_CARD==0)   //AI(W906-E042) 20261004 P9 NOT GOLDEN: golden stops on the ENCODER floor only (:5790); a stalled Z would be walked down for ever
                        bZMotMove=MOT[MTestZ1].Gali_MotMoveSkipEncoder(Pos-iStepSpeed, GotIndexZSpeed(iSpeedZ*1000));                                           //JimmyChiu 20211028 : All speed can set by speed setting.
                    else
                        bZMotMove=MOT[MTestZ1].MotorMove(Pos-iStepSpeed);

                    if(bZMotMove)
                    {
                        MOT[MTestZ1].Gali_ScanMotStatus();
                        if(MOT[MTestZ1].Led[iInposLed]==false)
                        {
                            palArm1Height->Caption=Pos-iStepSpeed;
                            Task=550;
                        }
                    }
                }
            }
            break;
        case 564:                                                                                                       //kevin 20140612  上升方便取料
            bZMotMove=MOT[MTestZ1].Gali_MotMove(-1000, GotIndexZSpeed(iSpeedZ*1000));                                   //JimmyChiu 20211028 : All speed can set by speed setting.

            if(bZMotMove)
            {
                Task=565;
            }
            break;
        case 565:                                                               //kevin 20140612 add
            bHasErr|=CheckIndexAllSuckICFallDown(true, false);                  //Steven 20131128 : 換位置
            for(int i=0; i<FTestSuck.iShtRow; i++)
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
                bIsTestSitICFallDown=true;
                ShowErrorMessage("JAM0303", K_SKIP, MTestZ1, false, ErrPart);   //Steven 20100129 : Device Drop Error

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
                            bCheckICFall[i][j]=false;
                        }
                    }
                }
                break;
            }

            if(bIsTestSitICFallDown)                                            //Steven 20100406
            {
                for(int i=0; i<FTestSuck.iShtRow; i++)
                    for(int j=0; j<FTestSuck.iShtCol; j++)
                        bCheckICFall[i][j]=false;
            }

            if(bIsTestSitICFallDown==false)
            {
                if(bAlarmGoTask560)                                             //Steven 20190822 : Add alarm when auto height
                {
                    Task=560;
                }
                else
                {
                    Task=710;
                }
                return false;
            }
            break;
        case 600:
            if(DoSetupTest(0))
            {
                Task=560;
                break;
            }

            if(WaitManualStepKey())
            {
                if(ATC_InterfaceForm->iATC_MODE_TYPE==ATC_TYPE_70 &&
                   Temperature.bATC7TSDFunction==true)                          //Steven 20161201 : by site TSD for Contact Test
                {
                    iSiteOn[0]=0;
                    iSiteOn[1]=0;
                    iSiteOn[2]=0;
                    iSiteOn[3]=0;
                    { (void)iSiteOn; } /*GATE(W906-E042-ATC) ATC_InterfaceForm->UseTSD_Function(4, iSiteOn); -- AI(W906-E042): ATC_InterfaceForm is TATC_InterfaceFormShim (acarry_shims.h:115, iATC_MODE_TYPE only); the real TATC_InterfaceForm (forms/fATCHandlerSide.h) has no global -- same gate as atester.cpp R04 / T04 (W906-GB-P2b); HT9050 USE_ATC_MODE=5 = no ATC*/
                }

                edContactHeight1->Text=ConvertTouMType(MOT[MTestZ1].Gali_ReadEncoderPos());
                InitWriteAndCheckMotorTorqueTask();
                SW[SwRKManualTStart].Off();
                IsTest=false;                                                   //Steven 20120505 Start: 離開時,要通知GPIB重置測試狀態
                TestSocket.ClearAll();

                if(TestIF.iTestType==TTL_MODE && (TTL_CARD_TYPE<2))
                {
                    DoSetupTest();                                              //Steven 20230731 : Contact Test結束要清除TTL訊號
                }
                else if(TestIF.iGpibMode==InterfaceType_SPEA_Type)
                {
                    bool flag[32]={false};
                    if(BAR_CODE_INSTALL!=ebctUninstall && TestIF_File.bEnableBarCode)
                        _RunTestProgram_BarMess(sizeof(flag), flag, 0, "");
                    else
                        _RunTestProgram(sizeof(flag), flag);
                }
                else
                {
                    fMain->RunTestProgram(false);
                }
                labDelayStatus->Caption="";
                Task=900;
            }
            break;
        case 700:                                                               //取得IndexZ1目前位置往上加3000
            Pos=MOT[MTestZ1].Gali_ReadEncoderPos();
            Pos=Pos+3000;

            if(IniConfig.iD17_UseHardwareHeightToContact==3)                    //Steven 20231025 : 不充氣K高
            {
                dHeight[iHeightStep]=iRecordZ1Pos1;
                iHeightStep++;
                if(iHeightStep==1)
                    Pos=MOT[MTestZ1].Gali_ReadEncoderPos();
            }
            else if(IniConfig.iD17_UseHardwareHeightToContact==2)               //Steven 20231025 : 不充氣K高
            {
                iRecordZ1Pos=iRecordZ1Pos1+100;
            }
            else if(DeviceForm_File.dKitDiameter<=2.5)                          //kevin 20170804 (Steven) 20mm Auto Height
            {
                if(INDEX_PRESS_TYPE==e85KG)                                     //Frank 20250214 add
                    iRecordZ1Pos=iRecordZ1Pos1;
                else
                    iRecordZ1Pos=iRecordZ1Pos1+100;
            }
            else
            {
                if(IniConfig.iD17_UseHardwareHeightToContact==1)                //Steven 20170411 (wei) : SCK的SIP怕刮傷,所以Contact Height使用硬體高度
                {
                    iRecordZ1Pos=iRecordZ1Pos1;
                }
                else if(bEPLeakage==true)                                       //jou 2011-09-29 如果沒有設定alarm值,高度都不減會太高,所以改-20
                {
                    iRecordZ1Pos=iRecordZ1Pos1-20;
                }
                else
                {
                    iRecordZ1Pos=iRecordZ1Pos1-100;
                }
            }

            if(cbContactMode->ItemIndex==DropPlaceShiftContact)                 //ChungHung 20150528 add for 海思 _8Site1x4 in contact
                Task=701;
            else
                Task=710;
            break;
        case 701:                                                                                                       //ChungHung 20150528 add for 海思 _8Site1x4 in contact
            if(INDEX_MOTION_CARD==0)
                bZMotMove=MOT[MTestZ1].Gali_MotMove(Prod.TestZ1_Safe, GotIndexZSpeed(iSpeedZ*1000));                    //Z2 上升至安全位置 //JimmyChiu 20211028 : All speed can set by speed setting.
            else
                bZMotMove=MOT[MTestZ1].MotorMove(Prod.TestZ1_Safe);

            if(bZMotMove)
            {
                Task=702;
            }
            break;
        case 702:                                                                                                                                               //ChungHung 20150528 add for 海思 _8Site1x4 in contact
            if(MachineTypeChoice==Type_HT502 ||
               MOT[MTestY1].GalilTwoY_Move(Prod.TestY1_Middle-TestIF.dSiteYPitch, Prod.TestY2_Rear, GotIndexYSpeed(iSpeed*3000), "Do_Z1_AutoGetHeight 702"))    //JimmyChiu 20211028 : All speed can set by speed setting.
            {
                Task=710;
            }
            break;
        case 710:                                                                                                       //Lee 2007_0208  //IndexZ1往上移動至目標位置
            if(INDEX_MOTION_CARD==0)
                bZMotMove=MOT[MTestZ1].Gali_MotMoveSkipEncoder(Pos, GotIndexZSpeed(iSpeedZ*1000));                      //Z2 上升至安全位置              //JimmyChiu 20211028 : All speed can set by speed setting.
            else
                bZMotMove=MOT[MTestZ1].MotorMove(Pos);

            if(bZMotMove)
            {
                palArm1Height->Caption=IntToStr(Pos);
                InitWriteAndCheckMotorTorqueTask();
                edContactHeight1->Text=ConvertTouMType(Pos);

                if(IniConfig.iD17_UseHardwareHeightToContact==3)                                                        //Steven 20231025 : 不充氣K高
                {
                    if(iHeightStep<2)
                    {
                        Task=520;
                        bRecordZ1Pos=false;
                    }
                    else
                    {
                        Task=7110;
                    }
                }
                else
                {
                    ADAM_WriteVoltage(DeviceForm.dPress);
                    ADAM_WriteVoltage(0);
                    MySleepEx(1000, false);
                    ADAM_WriteVoltage(DeviceForm.dPress);
                    Task=7150;
                }
            }
            break;
        case 7110:
            if(abs(dHeight[0]-dHeight[1])<(IniConfig.dD17_3CheckEPLeakage*100.0))
            {
                Task=7120;
            }
            else
            {
                iRecordZ1Pos=dHeight[0]-100;
                ADAM_WriteVoltage(DeviceForm.dPress);
                ADAM_WriteVoltage(0);
                MySleepEx(1000, false);
                ADAM_WriteVoltage(DeviceForm.dPress);

                Task=7150;
            }
            break;
        case 7120:
            bZMotMove=MOT[MTestZ1].Gali_MotMove(0, GotIndexZSpeed(iSpeedZ*1000));
            if(bZMotMove)
            {
                Task=7125;
            }
            break;
        case 7125:
            ShowErrorMessage("WAR0377", 0, MTestY1);
            iRecordZ1Pos=dHeight[1]+100;
            ADAM_WriteVoltage(DeviceForm.dPress);
            ADAM_WriteVoltage(0);
            MySleepEx(1000, false);
            ADAM_WriteVoltage(DeviceForm.dPress);
            Task=7150;
            break;
        case 7150:                                                              //寫入為大容許扭力值 120%
            flag=false;                                                         //jou 2010-06-21 start : auto high加一段程式驗證是否有沒有掉O-Ring
            if(LastSet.iRealDummy==REALLY)                                      //Steven 20190822 : Add alarm when auto height
            {
                for(int i=0; i<FTestSuck.iShtRow; i++)
                {
                    for(int j=0; j<FTestSuck.iShtCol; j++)
                    {
                        if(FTestSuck.Suck[i][j].Enable       &&                 //Steven 20110725 : 不再使用IsSuckICFallDown
                           FTestSuck.Suck[i][j].SenUsing!="" &&
                           FTestSuck.Item[i][j]!=HAS_NULL_IC &&
                           FTestSuck.Item[i][j]!=NULL_IC)
                        {
                            if(FTestSuck.Suck[i][j].GetStatus()==false)
                            {
                                FTestSuck.Suck[i][j].Normal();                  //jou 2012-01-17 直接關掉，避免掉到shuttle去，也避免要掉不掉Hang up
                                flag=true;
                                bRearHeadICFallDown=true;                       //kevin 20131120 發ALARM 開CHAMBO門 按Z1
                            }
                        }
                    }
                }
            }

            if(flag)
            {
                bAlarmGoTask560=false;
                MOT[MTestZ1].Gali_Command("ST", __FUNC__);
                Task=564;
                return false;
            }

            if(USE_IO_CHANGE_TOQUE==true)                                       //jou 2012-06-21 Enable index I/O Change Toque
            {
                SW[SwIndexChangeToque1].Off();
                SW[SwIndexChangeToque2].Off();
            }

            ret=COM2->iWriteAndCheckMotorTorque(0, 120);                        //jou 981128
            if(ret==1)
            {
                dHeight[0]=0.0;
                dHeight[1]=0.0;
                iHeightStep=0;
                bZMotMove=false;
                Task=7160;
            }
            else if(ret==2)
            {
                ShowMyMessage("Index Z1 Motor torque set error", "Z1馬達扭力設定錯誤!", "Do_Z1_AutoGetHeight 7150");
                fAllMotorHome=false;
                CarlibrationTask=1;
                return false;
            }
            break;
        case 7160:                                                                                                      //IndexZ1 往下移至目標位置
            bZMotMove=MOT[MTestZ1].Gali_MotMove(iRecordZ1Pos, GotIndexZSpeed(iSpeedZ*1000));                            //JimmyChiu 20211028 : All speed can set by speed setting.

            if(bZMotMove)
            {
                palArm1Height->Caption = IntToStr(iRecordZ1Pos);
                edContactHeight1->Text=ConvertTouMType(iRecordZ1Pos);

                UpdateContactRelative();                                                                                //JerryYang 20250120 : add

                InitWriteAndCheckMotorTorqueTask();

                COM2->InitReadTorueTask();
                fMain->chkReadTorque1->Checked=true;
                fMain->chkReadTorque2->Checked=false;
                fMain->edTorue0->Text="";
                Counter=0;
                bReadMCU2=true;                                                                                         //kevin 20220225 read MCU DATA
                Task=7170;
            }
            break;
        case 7170:                                                              //確認扭力是否超過 125%
            #ifdef SOFT_SIMULTE
                fMain->edTorue0->Text=30;
            #endif

            if(W906_IndexZTorqueWaitStop(0, fMain->edTorue0->Text=="", "Do_Z1_AutoGetHeight 7170")) { fAllMotorHome=false; CarlibrationTask=1; return false; }   if(fMain->edTorue0->Text!="")   //AI(W906-E042) 20261004 P5 NOT GOLDEN: 5 s without a torque value -> ST + message + golden 536 exit (golden waits for ever)
            {
                MySleep(500);
                if(abs(atoi(fMain->edTorue0->Text.c_str()))>=125)               //Frank 20150317 120->125
                {
                    if(Counter>=5)
                    {
                        ShowMyMessage("Z1 Motor Auto Height error, check EP Value!", "Z1馬達自動取得高度錯誤,請檢查EP是否漏氣!", "Do_Z1_AutoGetHeight 7170");
                        InitWriteAndCheckMotorTorqueTask();

                        if(INDEX_MOTION_CARD==0)
                            MOT[MTestZ1].iGali_SingalHomeTask=1;
                        else
                            MOT[MTestZ1].HomeReset();
                        Task=905;
                        break;
                    }
                    Counter++;
                    COM2->InitReadTorueTask();
                    fMain->chkReadTorque1->Checked=true;
                    fMain->chkReadTorque2->Checked=false;
                    fMain->edTorue0->Text="";
                    bReadMCU2=true;                                             //kevin 20220225 read MCU DATA
                }
                else
                {
                    InitWriteAndCheckMotorTorqueTask();
                    Task=720;
                }
            }
            break;
        case 720:                                                               //Lee 2007_0208    //寫入扭力值300%
            if(USE_IO_CHANGE_TOQUE==true)                                       //jou 2012-06-21 Enable index I/O Change Toque
            {
                SW[SwIndexChangeToque1].Off();
                SW[SwIndexChangeToque2].Off();
            }

            ret=COM2->iWriteAndCheckMotorTorque(0, 300);                        //jou 981128
            if(ret==1)
            {
                bZMotMove=false;
                Task=730;
            }
            else if(ret==2)
            {
                ShowMyMessage("Index Z1 Motor torque set error", "Z1馬達扭力設定錯誤!", "Do_Z1_AutoGetHeight 720");
                fAllMotorHome=false;
                CarlibrationTask=1;
                return false;
            }
            break;
        case 730:                                                                                                       //jou 981128  IndexZ1往下移至目標位置
            bZMotMove=MOT[MTestZ1].Gali_MotMove(iRecordZ1Pos, GotIndexZSpeed(iSpeedZ*1000));                            //JimmyChiu 20211028 : All speed can set by speed setting.

            if(bZMotMove)
            {
                palArm1Height->Caption=IntToStr(iRecordZ1Pos);
                InitWriteAndCheckMotorTorqueTask();
                edContactHeight1->Text=ConvertTouMType(iRecordZ1Pos);

                if(ATC_InterfaceForm->iATC_MODE_TYPE==ATC_TYPE_70 &&
                   Temperature.bATC7TSDFunction==true)                                                                  //Steven 20161201 : by site TSD for Contact Test
                {
                    iSiteOn[0]=(FTestSuck.Item[0][0]==HAS_IC || FTestSuck.Item[0][0]==HAS_HOT_IC)?1:0;
                    iSiteOn[1]=(FTestSuck.Item[0][1]==HAS_IC || FTestSuck.Item[0][1]==HAS_HOT_IC)?1:0;
                    iSiteOn[2]=0;
                    iSiteOn[3]=0;
                    { (void)iSiteOn; } /*GATE(W906-E042-ATC) ATC_InterfaceForm->UseTSD_Function(4, iSiteOn); -- AI(W906-E042): ATC_InterfaceForm is TATC_InterfaceFormShim (acarry_shims.h:115, iATC_MODE_TYPE only); the real TATC_InterfaceForm (forms/fATCHandlerSide.h) has no global -- same gate as atester.cpp R04 / T04 (W906-GB-P2b); HT9050 USE_ATC_MODE=5 = no ATC*/
                }
                ATC_SwitchTjSignal(1);                                                                                  //Ifor 20210622 add: ATC Switch TJ
                fMain->edTorue0->Text="999";
                fMain->SendMSG_CMD(MSG_CMD_Arm1Down);                                                                   //Ifor 20210331 :add Contact Mode GPIB需要知道哪隻Arm在Socker
                if(ATC_SYSTEM==eNewATCSystem &&
                   Temperature.bATCActiveCooling==true)                                                                 //JerryYang 20220815 : send ATC which ARM
                {
                    {} /*GATE(W906-E042-ATC) ATC_InterfaceForm->HandlerArm(0); -- AI(W906-E042): as above (the ATC 'which arm' notice)*/
                }
                iWhichArmDown=1;
                Task=800;

                for(int i=0; i<3; i++)                                                                                  //Steven 20100624 : K高過程不可以改變
                    tempEdit[i]->Enabled=true;
            }
            break;
        case 800:
//            bCheckDeviceDrop=false;                                           //Steven 20131128 : 檢查掉料的時間點 Do_Z1_AutoGetHeight case 800
            if(WaitManualStartKey())                                            //Steven 20100225
            {
                labDelayStatus->Caption="Waiting Test Result";
                fMain->SendMSG_CMD(MSG_CMD_ContactTestArm1);                    //Steven 20150304 : Add GPIB LOG
                {} /*GATE(W906-E042-UI) fTestCategory->BringToFront(); -- AI(W906-E042): window z-order only; TfTestCategory has no base class / BringToFront (forms/fTestCategory.h:257 D-2)*/                                  //Steven 20100823
                if(IniConfig.bC04EnableTestTempIC)
                    {} /*GATE(W906-E042-UI) fDynamicTemp->BringToFront(); -- AI(W906-E042): window z-order only; fDynamicTemp has no global (forms/fContact.h:458-461 X-13)*/                               //Steven 20120810
                iSetupTask=1;
                IsTest=true;                                                    //Steven 20110920
                Task=810;
                break;
            }

            if(WaitManualStepKey())
            {
                if(ATC_InterfaceForm->iATC_MODE_TYPE==ATC_TYPE_70 &&
                   Temperature.bATC7TSDFunction==true)                          //Steven 20161201 : by site TSD for Contact Test
                {
                    iSiteOn[0]=0;
                    iSiteOn[1]=0;
                    iSiteOn[2]=0;
                    iSiteOn[3]=0;
                    { (void)iSiteOn; } /*GATE(W906-E042-ATC) ATC_InterfaceForm->UseTSD_Function(4, iSiteOn); -- AI(W906-E042): ATC_InterfaceForm is TATC_InterfaceFormShim (acarry_shims.h:115, iATC_MODE_TYPE only); the real TATC_InterfaceForm (forms/fATCHandlerSide.h) has no global -- same gate as atester.cpp R04 / T04 (W906-GB-P2b); HT9050 USE_ATC_MODE=5 = no ATC*/
                }

                iTriggerBoostFunction=-1;
                iTriggerBoostFuncBack=-1;
                iBoostFuncStep=5;
                InitWriteAndCheckMotorTorqueTask();

                if(INDEX_MOTION_CARD==0)
                    MOT[MTestZ1].iGali_SingalHomeTask=1;
                else
                    MOT[MTestZ1].HomeReset();

                SW[SwRKManualTStart].Off();
                IsTest=false;                                                   //Steven 20120505 Start: 離開時,要通知GPIB重置測試狀態
                TestSocket.ClearAll();
                if(TestIF.iGpibMode==InterfaceType_SPEA_Type)
                {
                    bool flag[32]={false};
                    if(BAR_CODE_INSTALL!=ebctUninstall && TestIF_File.bEnableBarCode)
                        _RunTestProgram_BarMess(sizeof(flag), flag, 0, "");
                    else
                        _RunTestProgram(sizeof(flag), flag);
                }
                else
                {
                    fMain->RunTestProgram(false);
                }
                labDelayStatus->Caption="";
                fMain->SendMSG_CMD(MSG_CMD_ContactTestAbort);                   //Steven 20150304 : Add GPIB LOG

                if(IniConfig.bD58UseArm1PickPlaceArm2Test==true &&              //JerryYang 20250515 : 一丟一測Auto height把IC丟到SOCKET
                   TestIF_File.bArm1PickPlaceArm2Test==true)
                {
                    Task=8005;
                }
                else
                {
                    Task=905;
                }
            }

            if(fMain->edTorue0->Text!="")
            {
                COM2->InitReadTorueTask();
                fMain->chkReadTorque1->Checked=true;
                fMain->chkReadTorque2->Checked=false;
                fMain->edTorue0->Text="";
            }
            break;
        //JerryYang 20250515 : 一丟一測Auto height把IC丟到SOCKET
        //==>
        case 8005:
            if(CosFunction.bFixedDropSpeed)                                                                             //JerryYang 20250515 : 一丟一測Auto height把IC丟到SOCKET
            {
                fDropPos=atof(edContactHeight1->Text.c_str())+(double)CosFunction.dLimitMinDropOffset;
            }
            else
            {
                fDropPos=atof(edContactHeight1->Text.c_str())+atof(edDropOffset1->Text.c_str());
            }

            Pos=Get0_01MMType(FloatToStr(fDropPos).c_str());
            if(IniConfig.bChangeKitNoHardStop ||
               CosFunction.bContactShowOffset)                                                                          //JerryYang 20171201 (Steven) 修正 Contact height offset無效的問題
            {
                Pos=Pos+Get0_01MMType(edContactOffsetArm1->Text.c_str());
            }

            if(DeviceForm_File.dKitDiameter==8 ||
               DeviceForm_File.dKitDiameter==40.2)
            {
                Pos=Pos-iTotalOffset_1;
            }

            bZMotMove=MOT[MTestZ1].Gali_MotMove(Pos, GotIndexZSpeed(iSpeedZ*1000));                                     //JimmyChiu 20211028 : All speed can set by speed setting.

            if(bZMotMove)
            {
                palArm1Height->Caption=IntToStr(Pos);
                Task=8006;
            }
            break;
        case 8006:                                                              //丟測,放下IC
//            bCheckDeviceDrop=false;                                           //Steven 20131128 : 檢查掉料的時間點 Do_Z1_AutoGetHeight case 2910 : 丟測放下IC
            for(int i=0; i<FTestSuck.iShtRow; i++)
            {
                for(int j=0; j<FTestSuck.iShtCol; j++)
                {
                    if(INDEX_SUCKER_TYPE==1)
                    {
                        fiosetview->bIndexDestroy[0][i][j]=true;
                        bIndexCheckNoStopVaccum=true;
                    }
                    else
                    {
                         FTestSuck.Suck[i][j].Off();
                    }
                }
            }
            Do_Z1_AutoGetHeightDelay.SetSecAndOn(Prod.TestZ_Drop_Wait);         // delay 0.3 sec for ic down    //Steven 20140909 : 換到迴圈外面
            Task=8007;
            break;
        case 8007:                                                              //等待丟IC的Delay
            if(INDEX_SUCKER_TYPE==1)
            {
                bCheckDestroy=fiosetview->ProcessIndexSuckDestroy1();
            }
            else
            {
                bCheckDestroy=true;
            }

            if(bCheckDestroy==true && Do_Z1_AutoGetHeightDelay.Off())
            {
                for(int i=0; i<FTestSuck.iShtRow; i++)
                    for(int j=0; j<FTestSuck.iShtCol; j++)
                        FTestSuck.Suck[i][j].Normal();

                bZMotMove=false;

                Task=905;
            }
            break;
        //<==
        //JerryYang 20250515 : 一丟一測Auto height把IC丟到SOCKET
        case 810:
            if(DoSetupTest(0))
            {
                Task=800;
                break;
            }

            if(WaitManualStepKey())
            {
                if(ATC_InterfaceForm->iATC_MODE_TYPE==ATC_TYPE_70 &&
                   Temperature.bATC7TSDFunction==true)                          //Steven 20161201 : by site TSD for Contact Test
                {
                    iSiteOn[0]=0;
                    iSiteOn[1]=0;
                    iSiteOn[2]=0;
                    iSiteOn[3]=0;
                    { (void)iSiteOn; } /*GATE(W906-E042-ATC) ATC_InterfaceForm->UseTSD_Function(4, iSiteOn); -- AI(W906-E042): ATC_InterfaceForm is TATC_InterfaceFormShim (acarry_shims.h:115, iATC_MODE_TYPE only); the real TATC_InterfaceForm (forms/fATCHandlerSide.h) has no global -- same gate as atester.cpp R04 / T04 (W906-GB-P2b); HT9050 USE_ATC_MODE=5 = no ATC*/
                }

                InitWriteAndCheckMotorTorqueTask();

                if(INDEX_MOTION_CARD==0)
                    MOT[MTestZ1].iGali_SingalHomeTask=1;
                else
                    MOT[MTestZ1].HomeReset();

                SW[SwRKManualTStart].Off();
                IsTest=false;                                                   //Steven 20120505 Start: 離開時,要通知GPIB重置測試狀態
                TestSocket.ClearAll();
                if(TestIF.iTestType==TTL_MODE && (TTL_CARD_TYPE<2))
                {
                    DoSetupTest();                                              //Steven 20230731 : Contact Test結束要清除TTL訊號
                }
                else if(TestIF.iGpibMode==InterfaceType_SPEA_Type)
                {
                    bool flag[32]={false};
                    if(BAR_CODE_INSTALL!=ebctUninstall && TestIF_File.bEnableBarCode)
                        _RunTestProgram_BarMess(sizeof(flag), flag, 0, "");
                    else
                        _RunTestProgram(sizeof(flag), flag);
                }
                else
                {
                    fMain->RunTestProgram(false);
                }
                labDelayStatus->Caption="";
                Task=905;                                                       //jou 981128
            }
            break;
        case 900:
            Pos=MOT[MTestZ1].Gali_ReadEncoderPos();
            Pos+=500*MOT[MTestZ1].Motor->GearRatio;                             //+100條=10mm
            if(IniConfig.bD10ManualHeightComptibleWithNS &&
               iContactMode==CONTACT_MANUAL_GET_HEIGHT)                         //Steven 20100225
            {
                if(Pos>SafeTestZContactHeight)
                {
                    ShowMyMessage("Index Z1 test height too high error", "警告，Index Z1的測試高度太高!!", "Do_Z1_AutoGetHeight 900");
                    Task=200;
                    break;
                }
            }
            Task=9000;
            break;
        case 9000:
            if(MOT[MTestZ1].Gali_MotMoveSkipEncoder(Pos, GotIndexZSpeed(iSpeedZ*1000)))                                 // 以 40 kg 以上上升  5mm                   //JimmyChiu 20211028 : All speed can set by speed setting.
            {
                palArm1Height->Caption=Pos;
                InitWriteAndCheckMotorTorqueTask();
                Task=9010;
            }
            break;
        case 9010:
            if(USE_IO_CHANGE_TOQUE==true)                                       //jou 2012-06-21 Enable index I/O Change Toque
            {
                SW[SwIndexChangeToque1].Off();
                SW[SwIndexChangeToque2].Off();
            }

            if(IniConfig.bD10ManualHeightComptibleWithNS)
            {
                ret=COM2->iWriteAndCheckMotorTorque(0, kg);
            }
            else
            {
                if(kg>50)
                    ret=COM2->iWriteAndCheckMotorTorque(0, 90);
                else
                    ret=COM2->iWriteAndCheckMotorTorque(0, kg+40);
            }

            if(ret==1)
            {
                Task=901;
            }
            else if(ret==2)
            {
                ShowMyMessage("Index Z1 Motor torque set error", "Z1馬達扭力設定錯誤!", "Do_Z1_AutoGetHeight 9010");
                fAllMotorHome=false;
                CarlibrationTask=1;
                return false;
            }
            break;
        case 901:
            Pos=MOT[MTestZ1].Gali_ReadEncoderPos();
            Task=90100;
            Pos+=1000*MOT[MTestZ1].Motor->GearRatio;                            //+200條 = 20mm
            break;
        case 90100:
            if(MOT[MTestZ1].Gali_MotMoveSkipEncoder(Pos, GotIndexZSpeed(iSpeedZ*1000)))                                 // 以 40 kg 以上上升  5mm                   //JimmyChiu 20211028 : All speed can set by speed setting.
            {
                palArm1Height->Caption=Pos;
                InitWriteAndCheckMotorTorqueTask();
                Task=90110;
            }
            break;
        case 90110:
            if(USE_IO_CHANGE_TOQUE==true)                                       //jou 2012-06-21 Enable index I/O Change Toque
            {
                SW[SwIndexChangeToque1].Off();
                SW[SwIndexChangeToque2].Off();
            }

            ret=COM2->iWriteAndCheckMotorTorque(0, 99);
            if(ret==1)
            {
                Task=90120;
            }
            else if(ret==2)
            {
                ShowMyMessage("Index Z1 Motor torque set error", "Z1馬達扭力設定錯誤!", "Do_Z1_AutoGetHeight 90110");
                fAllMotorHome=false;
                CarlibrationTask=1;
                return false;
            }
            break;
        case 90120:
            if(MOT[MTestZ1].Gali_MotMoveSkipEncoder(Tech.iTestZDown, GotIndexZSpeed(iSpeedZ*1000)))                     // 以 40 kg 以上上升  5mm       //JimmyChiu 20211028 : All speed can set by speed setting.
            {
                InitWriteAndCheckMotorTorqueTask();
                palArm1Height->Caption=Tech.iTestZDown;
                Task=902;
            }
            break;
        case 902:
            if(USE_IO_CHANGE_TOQUE==true)                                       //jou 2012-06-21 Enable index I/O Change Toque
            {
                SW[SwIndexChangeToque1].Off();
                SW[SwIndexChangeToque2].Off();
            }

            ret=COM2->iWriteAndCheckMotorTorque(0, 300);
            if(ret==1)
            {
                Task=905;
                MOT[MTestZ1].iGali_SingalHomeTask=1;                            //Ifor 20161114 Fix AutoHigh 時 Hangup
            }
            else if(ret==2)
            {
                ShowMyMessage("Index Z1 Motor torque set error", "Z1馬達扭力設定錯誤!", "Do_Z2_AutoGetHeight 902");
                fAllMotorHome=false;
                CarlibrationTask=1;
                return false;
            }
            break;
        case 905:
            Task=930;
            break;
        case 930:
            if(INDEX_MOTION_CARD==0)
            {
                if(MOT[MTestZ1].Gali_SingalHome())
                {
                    bHasErr=false;                                              //Steven 20110829
                    palArm1Height->Caption=0;
                    Task=2100;
                }
                else                                                            //Steven 20100208
                {
                    if(MOT[MTestZ1].iGali_SingalHomeTask==900)
                        MOT[MTestZ1].iGali_SingalHomeTask=500;
                }
            }
            else
            {
                if(MOT[MTestZ1].MotorHome(true))
                {
                    bHasErr=false;                                              //Steven 20110829
                    palArm1Height->Caption=0;
                    Task=2100;
                }
            }
            break;
        //==============
        case 950:
            InitWriteAndCheckMotorTorqueTask();
            Task=960;
        case 960:
            if(USE_IO_CHANGE_TOQUE==true)                                       //jou 2012-06-21 Enable index I/O Change Toque
            {
                SW[SwIndexChangeToque1].Off();
                SW[SwIndexChangeToque2].Off();
            }

            ret=COM2->iWriteAndCheckMotorTorque(0, Prod.iMaxPreasure);

            if(ret==1)
            {
                Task=1000;
            }
            else if(ret==2)
            {
                ShowMyMessage("Index Z1 Motor torque set error", "Z1馬達扭力設定錯誤!", "Do_Z1_AutoGetHeight 960");
                fAllMotorHome=false;
                CarlibrationTask=1;
                return false;
            }
            break;
        case 1000:
            if(Sen[SnFMotorDown].IsOff()==false)
            {
                MOT[MTestZ1].Gali_Command("MOY", __FUNC__);                     // servo off
                MotorStatus=true;
            }
            else
            {
                if(MotorStatus)
                {
                    MotorStatus=false;
                    MOT[MTestZ1].Gali_Command("SHY", __FUNC__);

                    COM2->InitReadTorueTask();
                    fMain->chkReadTorque1->Checked=true;
                    fMain->chkReadTorque2->Checked=false;
                    fMain->edTorue0->Text="";
                }
            }

            if(MotorStatus==true)
                break;

            if(WaitManualStepKey())
            {
                labDelayStatus->Caption="";
                SW[SwRKManualTStart].Off();
                DriverDelay.SetMSAndOn(40);
                Task=2000;
            }
            break;
        case 1100:
            if(DoSetupTest(0))
            {
                Task=1000;
                break;
            }

            if(WaitManualStepKey())
            {
                if(ATC_InterfaceForm->iATC_MODE_TYPE==ATC_TYPE_70 &&
                   Temperature.bATC7TSDFunction==true)                          //Steven 20161201 : by site TSD for Contact Test
                {
                    iSiteOn[0]=0;
                    iSiteOn[1]=0;
                    iSiteOn[2]=0;
                    iSiteOn[3]=0;
                    { (void)iSiteOn; } /*GATE(W906-E042-ATC) ATC_InterfaceForm->UseTSD_Function(4, iSiteOn); -- AI(W906-E042): ATC_InterfaceForm is TATC_InterfaceFormShim (acarry_shims.h:115, iATC_MODE_TYPE only); the real TATC_InterfaceForm (forms/fATCHandlerSide.h) has no global -- same gate as atester.cpp R04 / T04 (W906-GB-P2b); HT9050 USE_ATC_MODE=5 = no ATC*/
                }

                DriverDelay.SetMSAndOn(4000);

                SW[SwRKManualTStart].Off();
                IsTest=false;                                                   //Steven 20120505 Start: 離開時,要通知GPIB重置測試狀態
                TestSocket.ClearAll();
                if(TestIF.iTestType==TTL_MODE && (TTL_CARD_TYPE<2))
                {
                    DoSetupTest();                                              //Steven 20230731 : Contact Test結束要清除TTL訊號
                }
                else if(TestIF.iGpibMode==InterfaceType_SPEA_Type)
                {
                    bool flag[32]={false};
                    if(BAR_CODE_INSTALL!=ebctUninstall && TestIF_File.bEnableBarCode)
                        _RunTestProgram_BarMess(sizeof(flag), flag, 0, "");
                    else
                        _RunTestProgram(sizeof(flag), flag);
                }
                else
                {
                    fMain->RunTestProgram(false);
                }
                labDelayStatus->Caption="";
                Task=2000;
            }
            break;
        case 2000:
            if(DriverDelay.Off())
            {
                Task=2050;
                bLampManualSetp=false;
                bLampManualStart=false;
                SW[SwManualZ1].Off();
                MOT[MTestZ1].iGali_SingalHomeTask=1;
            }
            break;
        case 2050:
            if(ScanIndexMotorCanMove()==false)
                return false;

            if(MOT[MTestZ1].Gali_SingalHome())
            {
                Offset.iIndexArmContact[0]=0;
                Offset_File.iIndexArmContact[0]=0;                              //Steven 20150512 : Offset --> Offset_File
                Pos=-MOT[MTestZ1].Motor->LastHomePos;

                if(Pos>SafeTestZContactHeight)
                {
                    ShowMyMessage("Index Z1 test height too high error", "警告，Index Z1的測試高度太高!!", "Do_Z1_AutoGetHeight 2050");
                    Task=950;                                                   //Steven 20100225 : 失敗後再繼續時不用歸零
                    break;
                }
                edContactHeight1->Text=ConvertTouMType(Pos);
                Task=2060;
            }
            break;
        case 2060:
            InitWriteAndCheckMotorTorqueTask();
            Task=2070;
        case 2070:
            if(USE_IO_CHANGE_TOQUE==true)                                       //jou 2012-06-21 Enable index I/O Change Toque
            {
                SW[SwIndexChangeToque1].Off();
                SW[SwIndexChangeToque2].Off();
            }

            ret=COM2->iWriteAndCheckMotorTorque(0, 300);
            if(ret==1)
            {
                bHasErr=false;                                                  //Steven 20110829
                Task=2100;
            }
            else if(ret==2)
            {
                ShowMyMessage("Index Z1 Motor torque set error", "Z1馬達扭力設定錯誤!", "Do_Z1_AutoGetHeight 2070");
                fAllMotorHome=false;
                CarlibrationTask=1;
                return false;
            }
            break;
        case 2100:
            fMain->chkReadTorque1->Checked=false;
            fMain->chkReadTorque2->Checked=false;

            bZMotMove=MOT[MTestZ1].Gali_MotMove(0, GotIndexZSpeed(iSpeedZ*1000));                                       //JimmyChiu 20211028 : All speed can set by speed setting.

            if(bZMotMove)
            {
                Task=2200;                                                                                              //Steven 20110829 : 升上來後才檢查IC掉料
            }
            break;
        case 2200:                                                              //Steven 20110829 : 升上來後才檢查IC掉料
            if(IniConfig.bD58UseArm1PickPlaceArm2Test==true &&                  //JerryYang 20250515 : 一丟一測Auto height把IC丟到SOCKET
               TestIF_File.bArm1PickPlaceArm2Test==true)
            {
                if(IniConfig.bC08_SocketSensor &&
                   TestIF_File.bEnSocketSensor)
                {
                    bflag=false;
                    str="";
                    for(int i=0; i<TestIF_File.iSocketCount; i++)
                    {
                        if(TestIF_File.iSensorCheckType[i]==2 &&                //Steven 20200420 : Socket Sensor功能可以選
                           Sen[SThreadPara.iSocketSensor[i]].IsOn())
                        {
                            bflag=true;
                            str+=IntToStr(i+1)+",";
                        }
                    }

                    if(bflag)
                    {
                        ShowErrorMessage("WAR0323", K_RETRY, MTestZ1, false, str);
                        return false;
                    }
                }
            }
            else
            {
                bHasErr|=CheckIndexAllSuckICFallDown(true, false);              //Steven 20131128 : 換位置

                for(int i=0; i<FTestSuck.iShtRow; i++)                          //Steven 20110516 Start: IC掉落要Alarm,並整合成一次
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
                            bCheckICFall[i][j]=false;
                        }
                    }
                }
                break;
            }

            palArm1Height->Caption=0;
            if(!bContinueContact)                                               //Steven 20100406
            {
                for(int i=0; i<FTestSuck.iShtRow; i++)
                    for(int j=0; j<FTestSuck.iShtCol; j++)
                        bCheckICFall[i][j]=false;
            }

            if(bContinueContact)
            {
                Task=1;
                return false;
            }
            return true;                                                        //AutoHeight 完成
        case 2800:
            if(Do_Z1_AutoGetHeightDelay.Off())                                                                          //JerryYang 20240111 : add
            {
                ADAM_Rang(IniConfig.iD26EPEncoderRange);
                if(ADAM_Alarm()==true)
                {
                    ret=ShowErrorMessage("WAR1605", K_RETRY|K_SKIP, 0, MMSystem, "TfContact::Do_Z1_AutoGetHeight");     //"請檢查EP是否漏氣!"
                    if(ret==K_RETRY)
                    {
                        return false;
                    }
                    else
                    {
                        Task=2900;
                    }
                }
                else
                {
                    Task=2900;
                }
            }
            break;
        case 2900:                                                                                                      //移動Arm 1到Socket
            if(MOT[MTestZ1].Gali_ReadPos()>(Prod.TestZ1_Test-Prod.TestZ1_Drop_Offset+750))                              //kevin 20140612 add start
            {
                flag=false;
                if(LastSet.iRealDummy==REALLY)
                {
                    for(int i=0; i<FTestSuck.iShtRow; i++)
                    {
                        for(int j=0; j<FTestSuck.iShtCol; j++)
                        {
                            if(FTestSuck.Suck[i][j].Enable       &&                                                     //Steven 20110725 : 不再使用IsSuckICFallDown
                               FTestSuck.Suck[i][j].SenUsing!="" &&
                               FTestSuck.Item[i][j]!=HAS_NULL_IC &&
                               FTestSuck.Item[i][j]!=NULL_IC)
                            {
                                if(FTestSuck.Suck[i][j].GetStatus()==false)
                                {
                                    FTestSuck.Suck[i][j].Normal();                                                      //jou 2012-01-17 直接關掉，避免掉到shuttle去，也避免要掉不掉Hang up
                                    flag=true;
                                    bRearHeadICFallDown=true;                                                           //kevin 20131120 發ALARM 開CHAMBO門 按Z1
                                }
                            }
                        }
                    }
                }

                if(flag)
                {
                    MOT[MTestZ1].Gali_Command("ST", __FUNC__);
                    bZMotMove=false;
                    Task=2904;
                    return false;
                }
            }

            if(DeviceForm_File.dKitDiameter==8 ||                                                                       //Ifor 20200318 : add 鋼徑80mmn 因為機台Hold不住變形需補下壓高度
               DeviceForm_File.dKitDiameter==40.2)
            {
                TestZ_CompensationHight();
            }

            if(cbContactMode->ItemIndex==DirectContactMode)                                                             //Direct Test Mode
            {
                Pos=Get0_01MMType(edContactHeight1->Text.c_str());
                if(CosFunction.bContactShowOffset || IniConfig.bChangeKitNoHardStop)                                    //2014-03-14    Dell      //Steven 20140409 : 矽品要求Contact畫面顯示Offset     //wei 20160603 驗證用
                    Pos=Pos+Get0_01MMType(edContactOffsetArm1->Text.c_str());

                if(DeviceForm_File.dKitDiameter==8 ||
                   DeviceForm_File.dKitDiameter==40.2)
                {
                    Pos=Pos-iTotalOffset_1;
                }

                bZMotMove=MOT[MTestZ1].Gali_MotMove(Pos, GotIndexZSpeed(iSpeedZ*1000));                                 //JimmyChiu 20211028 : All speed can set by speed setting.

                if(bZMotMove)
                {
                    palArm1Height->Caption=IntToStr(Pos);
                    COM2->InitReadTorueTask();
                    fMain->chkReadTorque1->Checked=true;
                    fMain->chkReadTorque2->Checked=false;
                    fMain->edTorue0->Text="";
                    Task=3000;
                }
            }
            else                                                                                                        //Drop Test Mode
            {
                if(CosFunction.bFixedDropSpeed)                                                                         //Steven 20131101 : 使用固定的Drop速度
                {
                    fDropPos=atof(edContactHeight1->Text.c_str())+(double)CosFunction.dLimitMinDropOffset;
                }
                else
                {
                    fDropPos=atof(edContactHeight1->Text.c_str())+atof(edDropOffset1->Text.c_str());
                }

                Pos=Get0_01MMType(FloatToStr(fDropPos).c_str());
                if(IniConfig.bChangeKitNoHardStop ||
                   CosFunction.bContactShowOffset)                                                                      //JerryYang 20171201 (Steven) 修正 Contact height offset無效的問題
                {
                    Pos=Pos+Get0_01MMType(edContactOffsetArm1->Text.c_str());
                }

                if(DeviceForm_File.dKitDiameter==8 ||
                   DeviceForm_File.dKitDiameter==40.2)
                {
                    Pos=Pos-iTotalOffset_1;
                }

                bZMotMove=MOT[MTestZ1].Gali_MotMove(Pos, GotIndexZSpeed(iSpeedZ*1000));                                 //JimmyChiu 20211028 : All speed can set by speed setting.

                if(bZMotMove)
                {
                    palArm1Height->Caption=IntToStr(Pos);
                    COM2->InitReadTorueTask();
                    fMain->chkReadTorque1->Checked=true;
                    fMain->chkReadTorque2->Checked=false;
                    fMain->edTorue0->Text="";
                    if(cbContactMode->ItemIndex==DropContact ||
                       cbContactMode->ItemIndex==DropContactModeDiffentSpeed ||
                       cbContactMode->ItemIndex==TMoveDrop   ||
                       cbContactMode->ItemIndex==TMoveDropSlowContact ||
                       cbContactMode->ItemIndex==DropPlaceShiftContact)                                                 //ChungHung 20150528 add for 海思 _8Site1x4
                    {
                        Task=2910;
                    }
                    else
                    {
                        Task=2930;
                    }
                }
            }
            break;
        case 2904:                                                                                                      //kevin 20140612  上升方便取料
            bZMotMove=MOT[MTestZ1].Gali_MotMove(-1000, GotIndexZSpeed(iSpeedZ*1000));                                   //JimmyChiu 20211028 : All speed can set by speed setting.

            if(bZMotMove)
            {
                Task=2905;
            }
            break;
        case 2905:                                                              //kevin 20140612 add
            bHasErr|=CheckIndexAllSuckICFallDown(true, false);                  //Steven 20131128 : 換位置
            for(int i=0; i<FTestSuck.iShtRow; i++)
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
                bIsTestSitICFallDown=true;
                ShowErrorMessage("JAM0303", K_SKIP, MTestZ1, false, ErrPart);   //Steven 20100129 : Device Drop Error

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
                            bCheckICFall[i][j]=false;
                        }
                    }
                }
                break;
            }

            if(bIsTestSitICFallDown)                                            //Steven 20100406
            {
                for(int i=0; i<FTestSuck.iShtRow; i++)
                    for(int j=0; j<FTestSuck.iShtCol; j++)
                        bCheckICFall[i][j]=false;
            }

            if(bIsTestSitICFallDown==false)
            {
                Task=2900;
                return false;
            }
            break;
        case 2910:                                                              //丟測,放下IC
//            bCheckDeviceDrop=false;                                           //Steven 20131128 : 檢查掉料的時間點 Do_Z1_AutoGetHeight case 2910 : 丟測放下IC
            for(int i=0; i<FTestSuck.iShtRow; i++)
            {
                for(int j=0; j<FTestSuck.iShtCol; j++)
                {
                    if(INDEX_SUCKER_TYPE==1)
                    {
                        fiosetview->bIndexDestroy[0][i][j]=true;
                        bIndexCheckNoStopVaccum=true;
                    }
                    else
                    {
                         FTestSuck.Suck[i][j].Off();
                    }
                }
            }

            if(IniConfig.bD58UseArm1PickPlaceArm2Test==true &&
               TestIF_File.bArm1PickPlaceArm2Test==true)                        //JerryYang 20241122 : 啟用D44功能時，INDEX ARM丟料後會持續吹氣，一直到INDEX ARM上抬到丟料高度
            {
                if(IniConfig.bD44CheckIndexICDestroy)
                {
                    for(int i=0; i<MAX_Index_Row; i++)
                    {
                        for(int j=0; j<NEW_MAX_Index_Col; j++)
                        {
                            if(FTestSuck.Item[i][j]==HAS_HOT_IC || FTestSuck.Item[i][j]==HAS_IC)
                            {
                                FTestSuck.Suck[i][j].Destroy();
                            }
                        }
                    }
                }
            }
            Do_Z1_AutoGetHeightDelay.SetSecAndOn(Prod.TestZ_Drop_Wait);         // delay 0.3 sec for ic down    //Steven 20140909 : 換到迴圈外面
            Task=2920;
        case 2920:                                                                                                      //等待丟IC的Delay
            if(INDEX_SUCKER_TYPE==1)
            {
                bCheckDestroy=fiosetview->ProcessIndexSuckDestroy1();
            }
            else
            {
                bCheckDestroy=true;
            }

            if(bCheckDestroy==true && Do_Z1_AutoGetHeightDelay.Off())
            {
                for(int i=0; i<FTestSuck.iShtRow; i++)
                    for(int j=0; j<FTestSuck.iShtCol; j++)
                        FTestSuck.Suck[i][j].Normal();

                bZMotMove=false;
                if(MachineTypeChoice==Type_HT502)
                {
                    Task=2930;
                }
                else
                {
                    if(CosFunction.bTesterSidePushFunction==true  &&                                                    //Richard 20220321 : 渠梁Side Push
                       DeviceForm.bTesterSidePush         ==true  &&
                       iTestSidePushcount                 ==0   )
                    {
                        if(Cylinder[C_TesterSidePush].Enable==true)
                        {
                            Do_Z1_AutoGetHeightDelay.SetSecAndOn(DeviceForm_File.dSitePushWaitTime);                    //Richard 20220321 : 渠梁Side Push
                            Task=5900;
                            iTestSidePushcount++;
                            break;
                        }
                    }

                    if(cbContactMode->ItemIndex==DropPlaceShiftContact)                                                 //ChungHung 20150528 add for 海思 _8Site1x4 in Contact
                        Task=2921;
                    else
                        Task=2930;
                }

                if(IniConfig.bIndexArm2SupplyLight==true ||                                                             //jou 2012-10-19 Index Arm 2 供應光源 for CMOS
                   (IniConfig.bD58UseArm1PickPlaceArm2Test==true && TestIF_File.bArm1PickPlaceArm2Test==true))          //kevin 20150127 Arm1 下壓 arm2 測試  //Ifor 20200811 Fix: Arm1 Pick Place Arm2Test 需卡兩個條件
                {
                    Task=4000;
                    if(IniConfig.bD58UseArm1PickPlaceArm2Test==true &&
                       TestIF_File.bArm1PickPlaceArm2Test==true)                                                        //JerryYang 20241122 : 啟用D44功能時，INDEX ARM丟料後會持續吹氣，一直到INDEX ARM上抬到丟料高度
                    {
                        if(IniConfig.bD44CheckIndexICDestroy)
                        {
                            for(int i=0; i<MAX_Index_Row; i++)
                            {
                                for(int j=0; j<NEW_MAX_Index_Col; j++)
                                {
                                    if(FTestSuck.Item[i][j]==HAS_HOT_IC || FTestSuck.Item[i][j]==HAS_IC)
                                    {
                                        FTestSuck.Suck[i][j].Destroy();
                                    }
                                }
                            }
                        }
                    }
                    break;
                }
            }
            iTestSidePushcount=0;
            break;
        case 2921:                                                                                                      //ChungHung 20150528 add for 海思 _8Site1x4 in Contact
            if(MOT[MTestZ1].Gali_MotMove(Prod.TestZ1_Safe, GotIndexZSpeed(iSpeedZ*1000)))                               //Z2 上升至安全位置                       //JimmyChiu 20211028 : All speed can set by speed setting.
            {
                Task=2922;
            }
            break;
        case 2922:                                                                                                                                              //ChungHung 20150528 add for 海思 _8Site1x4 in Contact
            if(MOT[MTestY1].GalilTwoY_Move(Prod.TestY1_Middle-TestIF.dSiteYPitch, Prod.TestY2_Rear, GotIndexYSpeed(iSpeed*3000), "Do_Z1_AutoGetHeight 2923"))   //JimmyChiu 20211028 : All speed can set by speed setting.
            {
                Task=2930;
            }
            break;
        case 2930:                                                                                                                                              //移動Arm 1 到測區
            if(CosFunction.bTesterSidePushFunction==true && DeviceForm.bTesterSidePush==true && Do_Z1_AutoGetHeightDelay.Off()==false)                          //Richard 20220321 : 渠梁Side Push
            {
                break;
            }
            Pos=Get0_01MMType(edContactHeight1->Text.c_str());

            if(IniConfig.bChangeKitNoHardStop || CosFunction.bContactShowOffset)                                                                                //JerryYang 20171201 (wei) 修正 Contact height offset無效的問題    //wei 20160603 驗證用
                Pos=Pos+Get0_01MMType(edContactOffsetArm1->Text.c_str());

            if(iContactMode==CONTACT_TEST &&
               (cbContactMode->ItemIndex==DirectContactSoftEP ||
                cbContactMode->ItemIndex==DropContactSoftEP))
            {
                if(MOT[MTestZ1].Gali_ReadPos()<(Pos+1500))
                {                                                                                                                                               //kevin 20130608 馬達到定點 ep 再充氣
                    EPSwitchOnOff(eEPSwBoth);
//                    bContSoftEpSwitch(0, true);                                 //ARM1 浮動頭充氣   //JerryYang 20151202 flase->true
                }
            }

            if(DeviceForm_File.dKitDiameter==8 ||                                                                                                               //Ifor 20230214 add: Contact 頁面 缸徑80 不同Contact Mode 高度補償
               DeviceForm_File.dKitDiameter==40.2)
            {
                TestZ_CompensationHight();
                Pos=Pos-iTotalOffset_1;
            }

            bZMotMove=MOT[MTestZ1].Gali_MotMove(Pos, GotIndexZSpeed(iSpeedZ*1000));                                                                             //JimmyChiu 20211028 : All speed can set by speed setting.

            if(bZMotMove)
            {
                palArm1Height->Caption=IntToStr(Pos);
                COM2->InitReadTorueTask();
                fMain->chkReadTorque1->Checked=true;
                fMain->chkReadTorque2->Checked=false;
                fMain->edTorue0->Text="";
                if(TestIF.bUseSLKClamp==true &&
                   TestIF_File.iSeparabilityTest==1)                                                                                                            //JerryYang 20160616 分離流程
                {
                    Task=2952;
                }
                else if((cbContactMode->ItemIndex==DropContact ||
                         cbContactMode->ItemIndex==DropContactModeDiffentSpeed ||
                         cbContactMode->ItemIndex==TMoveDrop   ||                                                                                               //Steven 20160530
                         cbContactMode->ItemIndex==TMoveDropSlowContact) &&
                         cbVacuumMode->ItemIndex==VacuumONMode)                                                                                                 //kevin 20110824 丟測掉料 DirectContactMode
                {
                    Task=2940;
                }
                else
                {
                    Task=3000;
                }
            }
            break;
        case 2940:                                                              // Vacumm On Mode
            for(int i=0; i<FTestSuck.iShtRow; i++)
            {
                for(int j=0; j<FTestSuck.iShtCol; j++)
                {
                    if(bCheckICFall[i][j]==true)                                //有用到且有吸到IC的
                        FTestSuck.Suck[i][j].On();
                }
            }

            Do_Z1_AutoGetHeightDelay.SetMSAndOn(100);                           // delay 0.1 sec for ic down
            Task=2950;
            break;
        case 2950:                                                              //等待Vacumm的Delay
            if(Do_Z1_AutoGetHeightDelay.Off())
                Task=3000;
            break;
        case 2952:                                                              //JerryYang 20160616 分離流程 START
            Cylinder[C_Socket_Unclamp].Off();
            Cylinder[C_Socket_Clamp].On();                                      //Z1已經下降到測試高度,Socket clamp夾持
            Z1ClampCloseDelay.SetMSAndOn(300);
            Z1ClampTimeOutDelay.SetSec(2.5);
            Z1ClampTimeOutDelay.On();
            Task=2953;
            break;
        case 2953:
            if(Z1ClampCloseDelay.Off())
            {
                #ifndef SOFT_SIMULTE
                if(Sen[SnSocketClampPush1].IsOn()==true && Sen[SnSocketClampPush2].IsOn()==true &&
                   Sen[SnSocketHasClamp1].IsOn()==true && Sen[SnSocketHasClamp2].IsOn()==true)
                {                                                               //破真空將IC放到SOCKET上
                    for(int i=0; i<FTestSuck.iShtRow; i++)
                    {
                        for(int j=0; j<FTestSuck.iShtCol; j++)
                        {
                            FTestSuck.Suck[i][j].Normal();
                        }
                    }
                    Do_Z1_AutoGetHeightDelay.SetMSAndOn(100);                   // delay 0.1 sec for ic down
                    Task=2954;
                }
                else
                {
                    if(Z1ClampTimeOutDelay.Off())
                    {
                        ret=ShowErrorMessage("JAM0375", K_RETRY, MMIndex);      //socket clamp異常
                        if(ret==K_RETRY)
                        {
                            Task=2952;
                        }
                    }
                }
                #else
                    Do_Z1_AutoGetHeightDelay.SetMSAndOn(100);                   // delay 0.1 sec for ic down
                    Task=2954;
                #endif
            }
            break;
        case 2954:
            if(Do_Z1_AutoGetHeightDelay.Off())                                  // Socket clamp夾持後,SLK1 放開clamp
            {
                Cylinder[C_SLK1_Clamp].Off();
                Cylinder[C_SLK1_Unclamp].On();
                Z1ClampCloseDelay.SetMSAndOn(500);
                Z1ClampTimeOutDelay.SetSec(2.5);
                Z1ClampTimeOutDelay.On();
                Task=2955;
            }
            break;
        case 2955:
            #ifdef SOFT_SIMULTE
            Task=2956;
            #else
            if(Z1ClampCloseDelay.Off())
            {
                if(Cylinder[C_SLK1_Unclamp].OnStatus())
                    Task=2956;
                if(Z1ClampTimeOutDelay.Off())
                {
                    ret=ShowErrorMessage("JAM0373", K_RETRY, MTestY1);          //ARM1 SLK放開clamp異常
                    if(ret==K_RETRY)
                    {
                        Task=2954;
                    }
                }
            }
            #endif
            break;
        case 2956:
            Pos=Get0_01MMType(edContactHeight1->Text.c_str());
            if(MOT[MTestZ1].Gali_MotMove(Pos+1500, GotIndexZSpeed(iSpeedZ*1000)))                                       //JerryYang 20160621 Arm 1上升5mm再測試        //JimmyChiu 20211028 : All speed can set by speed setting.
                Task=3010;
            break;
        case 3000:
            if(ATC_InterfaceForm->iATC_MODE_TYPE==ATC_TYPE_70 &&
               Temperature.bATC7TSDFunction==true)                                                                                                              //Steven 20161201 : by site TSD for Contact Test
            {
                iSiteOn[0]=(FTestSuck.Item[0][0]==HAS_IC || FTestSuck.Item[0][0]==HAS_HOT_IC)?1:0;
                iSiteOn[1]=(FTestSuck.Item[0][1]==HAS_IC || FTestSuck.Item[0][1]==HAS_HOT_IC)?1:0;
                iSiteOn[2]=0;
                iSiteOn[3]=0;
                { (void)iSiteOn; } /*GATE(W906-E042-ATC) ATC_InterfaceForm->UseTSD_Function(4, iSiteOn); -- AI(W906-E042): ATC_InterfaceForm is TATC_InterfaceFormShim (acarry_shims.h:115, iATC_MODE_TYPE only); the real TATC_InterfaceForm (forms/fATCHandlerSide.h) has no global -- same gate as atester.cpp R04 / T04 (W906-GB-P2b); HT9050 USE_ATC_MODE=5 = no ATC*/
            }

            if(ATC_SYSTEM==eNewATCSystem &&                                                                                                                     //Ifor 20190321 : add Test Mode & ATC Enables Site Send to GPIB
               Temperature.bATCActiveCooling==true &&
               TestIF_File.i2DIDFormat==eAMD)                                                                                                                   //JerryYang 20200422 2DID format選項改用下拉選單
            {
                for(int i=0; i<MAX_SOCKET_TOTAL; i++)
                {
                    HHandler2Gpib.Site[i]=bATC_EnablesChannel[i];
                }
            }
            HHandler2Gpib.iLotStatus=TestIF_File.iTestMode;

            if(IniConfig.bD58UseArm1PickPlaceArm2Test==true && TestIF_File.bArm1PickPlaceArm2Test==true && bArm2Test==true)
            {
                fMain->SendMSG_CMD(MSG_CMD_Arm2Down);                                                                                                           //Ifor 20180622 (wei) : add ContactTestArm
                if(ATC_SYSTEM==eNewATCSystem &&
                   Temperature.bATCActiveCooling==true)                                                                                                         //JerryYang 20220815 : send ATC which ARM
                {
                    {} /*GATE(W906-E042-ATC) ATC_InterfaceForm->HandlerArm(1); -- AI(W906-E042): as above (the ATC 'which arm' notice)*/
                }
                iWhichArmDown=2;                                                                                                                                //JerryYang 20200316 add SVID 哪支arm下壓在測區
                ATC_SwitchTjSignal(2);                                                                                                                          //Ifor 20210622 add: ATC Switch TJ
            }
            else
            {
                fMain->SendMSG_CMD(MSG_CMD_Arm1Down);                                                                                                           //Ifor 20180622 (wei) : add ContactTestArm
                if(ATC_SYSTEM==eNewATCSystem &&
                   Temperature.bATCActiveCooling==true)                                                                                                         //JerryYang 20220815 : send ATC which ARM
                {
                    {} /*GATE(W906-E042-ATC) ATC_InterfaceForm->HandlerArm(0); -- AI(W906-E042): as above (the ATC 'which arm' notice)*/
                }
                iWhichArmDown=1;                                                                                                                                //JerryYang 20200316 add SVID 哪支arm下壓在測區
                ATC_SwitchTjSignal(1);                                                                                                                          //Ifor 20210622 add: ATC Switch TJ
            }

            if(cbContactMode->ItemIndex==DirectContactSoftEP ||
               cbContactMode->ItemIndex==DropContactSoftEP)
            {                                                                                                                                                   //kevin 20130608 馬達到定點 ep 在充氣
                Do_Z1_AutoGetHeightDelay.SetMSAndOn(500);                                                                                                       // delay 0.1 sec for ic down
                Task=3001;
            }
            else
            {
                if(CUSTOMER_CODE==CC_KYEC_CHEN &&
                   cbVacuumMode->ItemIndex==VacuumOFFMode)                                                                                                      //jou 2015-08-31 新增Index Vacuum Off mode
                {
                    for(int i=0; i<FTestSuck.iShtRow; i++)
                    {
                        for(int j=0; j<FTestSuck.iShtCol; j++)
                        {
                            FTestSuck.Suck[i][j].Normal();
                        }
                    }
                }

                if(CosFunction.bTesterSidePushFunction==true && DeviceForm.bTesterSidePush==true && DeviceForm_File.iSidePushMode==0)                           //Richard 20230417 : 渠梁Side Push fix延遲
                    Task=3002;
                else
                    Task=3010;
            }
            iGPIBIndexStatus=Z1Down_Z2Up;
            break;
        case 3001:                                                              //等待Vacumm的Delay
            if(Do_Z1_AutoGetHeightDelay.Off())
            {
                Task=3010;
            }
            break;
        case 3002:                                                              //Richard 20230417 : 渠梁Side Push fix延遲
            if(CosFunction.bTesterSidePushFunction==true &&
               DeviceForm.bTesterSidePush==true)
            {
                if(Cylinder[C_TesterSidePush].Enable==true)
                {
                    Task=6100;
                    Do_Z1_AutoGetHeightDelay.SetSecAndOn(DeviceForm_File.dSitePushWaitTime);
                    break;
                }
                else                                                            //JerryYang 20250228 : fix hang up
                {
                    if(IniConfig.bSPILFunction &&
                        cbTestContactMode->Checked==true)                       //KevinCheng 20260115 : NV Contact Test
                    {
                        iTestSec=atoi(edTestSec->Text.c_str());
                        Task=3003;
                        Z2_Down_Delay.SetSecAndOn(iTestSec);
                    }
                    else
                    {
                        Task=3010;
                        break;
                    }
                }
            }
            break;
        case 3003:                                                              //KevinCheng 20260115 : NV Contact Test
            if(Z2_Down_Delay.Off())
            {
                Task=3010;
            }
            break;
        case 3010:                                                                                                      //Ifor 20170105 (Steven) add Index 到位後顯示 Index Arm Jog Move 工具
            if(CUSTOMER_CODE==CC_KYEC_LEE ||
               CUSTOMER_CODE==CC_HONPREC_QC)
            {
                edContactOffsetArm1->Enabled=false;                                                                     //Alick 20170116 add 當JOG視窗顯示時，對應Z的輸入框不可KEYIN
                plIndexArmJogMove->Visible=true;
                iIndexStatus=Z1Down_Z2Up;
                if(bContactJogFlag==true)                                                                               //Ifor 20170125 (Steven) add 避免尚未壓下Jog Move 按鈕 iContactJogMove=0 會跑回原點
                {
                    if(MOT[MTestZ1].Gali_MotMoveNoWait(iContactJogMove, GotIndexZSpeed(iSpeedZ*1000), 0))               //Alick 20170116 add   //JimmyChiu 20211028 : All speed can set by speed setting.
                    {
                        bContactJogFlag=false;
                        btnIndexArmJogMove_Up->Enabled=true;
                        btnIndexArmJogMove_Down->Enabled=true;
                    }
                }
            }

            if(iContactMode==CONTACT_DEVICE_MAP_CHECK ||                                                                //Steven 20191009 : Device Map for Qualcomm
               WaitManualStartKey())
            {
                labDelayStatus->Caption="Waiting Test Result";
                if(IniConfig.bD58UseArm1PickPlaceArm2Test==true && TestIF_File.bArm1PickPlaceArm2Test==true && bArm2Test==true)
                    fMain->SendMSG_CMD(MSG_CMD_ContactTestArm2);                                                        //Steven 20150304 : Add GPIB LOG
                else
                    fMain->SendMSG_CMD(MSG_CMD_ContactTestArm1);                                                        //Steven 20150304 : Add GPIB LOG

                {} /*GATE(W906-E042-UI) fTestCategory->BringToFront(); -- AI(W906-E042): window z-order only; TfTestCategory has no base class / BringToFront (forms/fTestCategory.h:257 D-2)*/                                                                          //Steven 20100823
                if(IniConfig.bC04EnableTestTempIC)
                    {} /*GATE(W906-E042-UI) fDynamicTemp->BringToFront(); -- AI(W906-E042): window z-order only; fDynamicTemp has no global (forms/fContact.h:458-461 X-13)*/                                                                       //Steven 20120810
                iSetupTask=1;
                IsTest=true;                                                                                            //Steven 20110920
                Task=3100;
                break;
            }

            if(WaitManualStepKey() ||
                (IniConfig.bSPILFunction && cbTestContactMode->Checked==true))                                          //KevinCheng 20260115 : NV Contact Test
            {
                Task=3110;
                if(((DeviceForm.iSocketInitialICCheckPosition==1 && IniConfig.bTestIcCheckInContact==true) ||           //JerryYang 20250120 : add
                   (LastSet.iD41SocketInitialICCheckPosition==1 && IniConfig.bTestIcCheckInContact==false)) &&
                    cbCalibrateAboveHeight->Checked)                                                                    //Above Socket
                {
                    Task=3105;
                    DoCalibrateAboveHeightZ1(true);
                }

                if(ATC_SYSTEM==eNewATCSystem)                                                                           //Ifor 20151230 :add New ATC Interface HandlerArm
                {
                    {} /*GATE(W906-E042-ATC) ATC_InterfaceForm->HandlerArm(-1); -- AI(W906-E042): as above (the ATC 'which arm' notice)*/                                                                  //Ifor 20220107 add:通知ATC目前哪隻Arm再Socket
                }
            }

            if(fMain->edTorue0->Text!="")
            {
                COM2->InitReadTorueTask();
                fMain->chkReadTorque1->Checked=true;
                fMain->chkReadTorque2->Checked=false;
                fMain->edTorue0->Text="";
            }
            break;
        case 3105:
            if(DoCalibrateAboveHeightZ1()==true)                                //JerryYang 20250120 : add
            {
                Memo1->Lines->Add("Z1 Above Socket height calibration is finished!");
                Task=3110;
            }
            break;
        case 3100:
            if(DoSetupTest(0))
            {
                if(iContactMode==CONTACT_DEVICE_MAP_CHECK)                      //Steven 20191009 : Device Map for Qualcomm
                    Task=3110;
                else
                    Task=3000;

                if(iContactMode==STEP_CONTACT_TEST && chkDailyCorrelation->Checked)
                    Task=3110;
                break;
            }

            if(WaitManualStepKey())
            {
                Task=3110;
            }
            break;
        case 3110:                                                                                                                                              //Ifor 20170105 (Steven) add Index 到位後顯示 Index Arm Jog Move 工具
            if(CUSTOMER_CODE==CC_KYEC_LEE)
            {
                plIndexArmJogMove->Visible=false;
                edContactOffsetArm1->Enabled=true;                                                                                                              //Ifor 20170125 (Steven) add 按下Step要解除對應Z的輸入框鎖定
            }

            if(ATC_InterfaceForm->iATC_MODE_TYPE==ATC_TYPE_70 &&
               Temperature.bATC7TSDFunction==true)                                                                                                              //Steven 20161201 : by site TSD for Contact Test
            {
                iSiteOn[0]=0;
                iSiteOn[1]=0;
                iSiteOn[2]=0;
                iSiteOn[3]=0;
                { (void)iSiteOn; } /*GATE(W906-E042-ATC) ATC_InterfaceForm->UseTSD_Function(4, iSiteOn); -- AI(W906-E042): ATC_InterfaceForm is TATC_InterfaceFormShim (acarry_shims.h:115, iATC_MODE_TYPE only); the real TATC_InterfaceForm (forms/fATCHandlerSide.h) has no global -- same gate as atester.cpp R04 / T04 (W906-GB-P2b); HT9050 USE_ATC_MODE=5 = no ATC*/
            }

            iTriggerBoostFunction=-1;
            iTriggerBoostFuncBack=-1;
            iBoostFuncStep=5;

            SW[SwRKManualTStart].Off();
            IsTest=false;                                                                                                                                       //Steven 20120505 Start: 離開時,要通知GPIB重置測試狀態
            TestSocket.ClearAll();
            if(TestIF.iTestType==TTL_MODE && (TTL_CARD_TYPE<2))
            {
                DoSetupTest();                                                                                                                                  //Steven 20230731 : Contact Test結束要清除TTL訊號
            }
            else if(TestIF.iGpibMode==InterfaceType_SPEA_Type)
            {
                bool flag[32]={false};
                if(BAR_CODE_INSTALL!=ebctUninstall && TestIF_File.bEnableBarCode)
                    _RunTestProgram_BarMess(sizeof(flag), flag, 0, "");
                else
                    _RunTestProgram(sizeof(flag), flag);
            }
            else
            {
                fMain->RunTestProgram(false);
            }
            labDelayStatus->Caption="";

            fMain->SendMSG_CMD(MSG_CMD_ContactTestAbort);                                                                                                       //Steven 20150304 : Add GPIB LOG
            if((IniConfig.bD58UseArm1PickPlaceArm2Test==true && TestIF_File.bArm1PickPlaceArm2Test==true) && bContinueContact)                                  //JerryYang 20160108 使用arm1丟arm2測模式,contact test 按one cycle後Arm2要上升再重新contact   //Ifor 20200811 Fix: Arm1 Pick Place Arm2Test 需卡兩個條件
            {
                Task=4060;
                break;
            }

            if(cbContactMode->ItemIndex==DropPlaceShiftContact)                                                                                                 //ChungHung 20150528 add for 海思 _8Site1x4
            {
                Task=3150;
            }
            else if(TestIF_File.bUseSLKClamp &&
                    TestIF_File.iSeparabilityTest==1)                                                                                                           //JerryYang 20160617 夾起來
            {
                Task=3180;
                break;
            }
            else if(CosFunction.bTesterSidePushFunction==true && DeviceForm.bTesterSidePush==true && DeviceForm_File.iSidePushMode==1)                          //Richard 20220321 : 渠梁Side Push
            {
                Task=6200;
                break;
            }
            else
            {
                Task=3200;
            }

            if(IniConfig.bIndexArm2SupplyLight==true ||                                                                                                         //jou 2012-10-19 Index Arm 2 供應光源 for CMOS
               (IniConfig.bD58UseArm1PickPlaceArm2Test==true && TestIF_File.bArm1PickPlaceArm2Test==true))                                                      //kevin 20150127 Arm1 下壓 arm2 測試   //Ifor 20200811 Fix: Arm1 Pick Place Arm2Test 需卡兩個條件
            {
                Task=4030;
                break;
            }
            break;
        case 3150:                                                                                                      //ChungHung 20150528 add for 海思 _8Site1x4
            if(MOT[MTestZ1].Gali_MotMove(Prod.TestZ1_Safe, GotIndexZSpeed(iSpeedZ*1000)))                               //Z1 上升至安全位置                       //JimmyChiu 20211028 : All speed can set by speed setting.
            {
                Task=3160;
            }
            break;
        case 3160:                                                                                                                                              //ChungHung 20150528 add for 海思 _8Site1x4
            if(MOT[MTestY1].GalilTwoY_Move(Prod.TestY1_Middle-TestIF.dSiteYPitch, Prod.TestY2_Rear, GotIndexYSpeed(iSpeed*3000), "Do_Z1_AutoGetHeight 3160"))   //JimmyChiu 20211028 : All speed can set by speed setting.
            {
                Task=3170;
            }
            break;
        case 3170:                                                              //ChungHung 20150528 add for 海思 _8Site1x4
            Pos=Get0_01MMType(edContactHeight1->Text.c_str());
            if(MOT[MTestZ1].Gali_MotMove(Pos, GotIndexZSpeed(iSpeedZ*1000)))    //JimmyChiu 20211028 : All speed can set by speed setting.
            {
                Task=3200;
            }
            break;
        case 3180:                                                              //JerryYang 20160621 分離機構流程Start
            if(Cylinder[C_SLK1_Unclamp].OnSensor())
            {
                Task=3182;
            }
            break;
        case 3182:
            Pos=Get0_01MMType(edContactHeight1->Text.c_str());
            if(MOT[MTestZ1].Gali_MotMove(Pos, GotIndexZSpeed(iSpeedZ*1000)))    //JimmyChiu 20211028 : All speed can set by speed setting.
            {
                Task=3184;
            }
            break;
        case 3184:
            Cylinder[C_SLK1_Unclamp].Off();
            Cylinder[C_SLK1_Clamp].On();
            Z1ClampOpenDelay.SetMSAndOn(500);
            Z1ClampTimeOutDelay.SetSec(2.5);
            Z1ClampTimeOutDelay.On();
            Task=3186;
            break;
        case 3186:
            if(Z1ClampOpenDelay.Off())
            {
                if(Cylinder[C_SLK1_Clamp].OnSensor())
                {
                    Task=3188;
                }
                else
                {
                    if(Z1ClampTimeOutDelay.Off())
                    {
                        ret=ShowErrorMessage("JAM0371", K_RETRY, MTestY1);      //ARM2 SLK clamp未到位
                        if(ret==K_RETRY)
                        {
                            Task=3184;
                        }
                    }
                }
            }
            break;
        case 3188:
            Cylinder[C_Socket_Clamp].Off();                                     //Socket Clamp 打開
            Cylinder[C_Socket_Unclamp].On();
            Z1ClampOpenDelay.SetMSAndOn(500);
            Z1ClampTimeOutDelay.SetSec(2.5);
            Z1ClampTimeOutDelay.On();
            Task=3190;
            break;
        case 3190:
            if(Z1ClampOpenDelay.Off())
            {
                #ifndef SOFT_SIMULTE
                if(Sen[SnSocketClampPull1].IsOn()==true && Sen[SnSocketClampPull2].IsOn()==true &&
                   Sen[SnSocketClampPush1].IsOff()==true && Sen[SnSocketClampPush2].IsOff()==true)
                {
                    Task=3200;
                }
                else
                {
                    if(Z1ClampTimeOutDelay.Off())
                    {
                        ret=ShowErrorMessage("JAM0376", K_RETRY, MMIndex);      //放開clamp異常
                        if(ret==K_RETRY)
                        {
                            Task=3188;
                        }
                    }
                }
                #else
                    Task=3200;
                #endif
            }
            break;
        case 3200:
            for(int i=0; i<FTestSuck.iShtRow; i++)
            {
                for(int j=0; j<FTestSuck.iShtCol; j++)
                {
                    if(bCheckICFall[i][j]==true)                                //有用到的
                        FTestSuck.Suck[i][j].On();
                }
            }
            Do_Z1_AutoGetHeightDelay.SetSecAndOn(2);                            // delay 3 sec for ic down
            Task=3210;
        case 3210:
            if(Do_Z1_AutoGetHeightDelay.Off())
            {
                Task=3300;
            }
            break;
        case 3300:
            Pos=Get0_01MMType(edContactHeight1->Text.c_str());
            if(MOT[MTestZ1].Gali_MotMove(Pos+200, GotIndexZSpeed(iSpeedZ*100)))                                         //JimmyChiu 20211028 : All speed can set by speed setting.
            {
                bHasErr=false;                                                                                          //Steven 20110829
                Task=2100;
            }
            break;
        case 4000:                                                              //jou 2012-10-19 Index Arm 2 供應光源 for CMOS start
            if(CosFunction.bFixedDropSpeed)                                     //Steven 20131101 : 使用固定的Drop速度
            {
                fDropPos=atof(edContactHeight1->Text.c_str())+(double)CosFunction.dLimitMinDropOffset+10.0;
            }
            else
            {
                fDropPos=atof(edContactHeight1->Text.c_str())+atof(edDropOffset1->Text.c_str())+10.0;
            }
            Pos=Get0_01MMType(FloatToStr(fDropPos).c_str());
            if(MOT[MTestZ1].Gali_MotMove(Pos, GotIndexZSpeed(iSpeedZ*100)))     //JimmyChiu 20211028 : All speed can set by speed setting.
            {
                if(IniConfig.bD58UseArm1PickPlaceArm2Test==true &&
                   TestIF_File.bArm1PickPlaceArm2Test==true)                    //JerryYang 20241122 : 啟用D44功能時，INDEX ARM丟料後會持續吹氣，一直到INDEX ARM上抬到丟料高度
                {
                    if(IniConfig.bD44CheckIndexICDestroy)
                    {
                        for(int i=0; i<MAX_Index_Row; i++)
                        {
                            for(int j=0; j<NEW_MAX_Index_Col; j++)
                            {
                                if(FTestSuck.Item[i][j]==HAS_HOT_IC || FTestSuck.Item[i][j]==HAS_IC)
                                {
                                    FTestSuck.Suck[i][j].Normal();
                                }
                            }
                        }
                    }
                }
                Task=4005;
            }
            break;
        case 4005:
            if(MOT[MTestZ1].Gali_MotMove(0, GotIndexZSpeed(iSpeedZ*1000)))                                              //JimmyChiu 20211028 : All speed can set by speed setting.
            {
                if(IniConfig.bC08_SocketSensor && TestIF_File.bEnSocketSensor && TestIF_File.bCheckSocketFloating)      //Steven 2020604 : 使用Socket Sensor驗證置偏
                {
                    Task=5000;
                    iBackTask=4010;
                }
                else
                {
                    Task=4010;
                }
            }
            break;
        case 4010:                                                                                                                                              //JimmyChiu 20220708 : add Index Arm Axis
            if(MOT[MTestY1].GalilTwoY_Move(Prod.TestY1_Front, Prod.TestY2_Middle, GotIndexYSpeed(iSpeed*3000), "Do_Z1_AutoGetHeight 4010"))                     //JimmyChiu 20211028 : All speed can set by speed setting.
            {
                Task=4020;
            }
            break;
        case 4020:                                                              //Arm2準備下去壓
            Pos=Get0_01MMType(edContactHeight2->Text.c_str());
            if(MOT[MTestZ2].Gali_MotMove(Pos, GotIndexZSpeed(iSpeedZ*1000)))    //JimmyChiu 20211028 : All speed can set by speed setting.
            {
                Task=3000;
                bArm2Test=true;                                                 //Ifor 20200708 add: Arm1 Pick up Arm2 Test 模式下 Arm2 在Socket裡面
            }
            break;
        case 4030:
            Pos=Get0_01MMType(edContactHeight2->Text.c_str())+1000;
            if(MOT[MTestZ2].Gali_MotMove(Pos, GotIndexZSpeed(iSpeedZ*100)))     //JimmyChiu 20211028 : All speed can set by speed setting.
            {
                Task=4035;
            }
            break;
        case 4035:                                                                                                      //Arm2測試完成, 上升
            if(MOT[MTestZ2].Gali_MotMove(0, GotIndexZSpeed(iSpeedZ*1000)))                                              //JimmyChiu 20211028 : All speed can set by speed setting.
            {
                if(IniConfig.bC08_SocketSensor && TestIF_File.bEnSocketSensor && TestIF_File.bCheckSocketFloating)      //Steven 2020604 : 使用Socket Sensor驗證置偏
                {
                    Task=5000;
                    iBackTask=4040;
                }
                else
                {
                    Task=4040;
                }
            }
            break;
        case 4040:
            if(MOT[MTestY1].GalilTwoY_Move(Prod.TestY1_Middle, Prod.TestY2_Rear, GotIndexYSpeed(iSpeed*3000), "Do_Z1_AutoGetHeight 4040"))                      //JimmyChiu 20211028 : All speed can set by speed setting.
            {
                Task=4050;
            }
            break;
        case 4050:
            Pos=Get0_01MMType(edContactHeight1->Text.c_str());
            if(MOT[MTestZ1].Gali_MotMove(Pos, GotIndexZSpeed(iSpeedZ*1000)))    //JimmyChiu 20211028 : All speed can set by speed setting.
            {
                Task=3200;
                bArm2Test=false;                                                //Ifor 20200708 add: Arm1 Pick up Arm2 Test 模式下 Arm2 在Socket裡面
            }
            break;
        case 4060:                                                              //JerryYang 20160108 使用arm1丟arm2測模式,contact test 按one cycle後Arm2要上升再重新contact
            Pos=Get0_01MMType(edContactHeight2->Text.c_str())+1000;
            if(MOT[MTestZ2].Gali_MotMove(Pos, GotIndexZSpeed(iSpeedZ*100)))     //JimmyChiu 20211028 : All speed can set by speed setting.
            {
                Task=4065;
            }
            break;
        case 4065:                                                                                                      //JerryYang 20160108 使用arm1丟arm2測模式,contact test 按one cycle後Arm2要上升再重新contact
            if(MOT[MTestZ2].Gali_MotMove(0, GotIndexZSpeed(iSpeedZ*1000)))                                              //JimmyChiu 20211028 : All speed can set by speed setting.
            {
                if(IniConfig.bC08_SocketSensor && TestIF_File.bEnSocketSensor && TestIF_File.bCheckSocketFloating)      //Steven 2020604 : 使用Socket Sensor驗證置偏
                {
                    Task=5000;
                    iBackTask=4020;
                }
                else
                {
                    Task=4020;
                }
            }
            break;                                                                                                      //jou 2012-10-19 Index Arm 2 供應光源 for CMOS end
        case 5000:                                                              //Steven 200604 : 使用Socket Sensor驗證置偏
            flag=false;
            for(int i=0; i<TestIF_File.iSocketCount; i++)
            {
                if(TestIF_File.iSensorCheckType[i]==2 &&                        //Steven 20200420 : Socket Sensor功能可以選
                   Sen[SThreadPara.iSocketSensor[i]].IsOn())                    //Steven 20200420 : Arm 1在上, 檢查置偏遮斷
                {
                    flag=true;
                }
            }

            if(flag)
            {
                Task=5100;
            }
            else
            {
                Task=iBackTask;
            }
            break;
        case 5100:
            if(MOT[MTestY1].GalilTwoY_Move(Prod.TestY1_Front, Prod.TestY2_Rear, GotIndexYSpeed(iSpeed*3000), "Do_Z1_AutoGetHeight 5100"))                       //JimmyChiu 20211028 : All speed can set by speed setting.
            {
                Task=5200;
            }
            break;
        case 5200:
            ErrPart="";
            for(int i=0; i<TestIF_File.iSocketCount; i++)
            {
                if(TestIF_File.iSensorCheckType[i]==2 &&                        //Steven 20200420 : Socket Sensor功能可以選
                   Sen[SThreadPara.iSocketSensor[i]].IsOn())                    //Steven 20200420 : Arm 1在上, 檢查置偏遮斷
                {
                    ErrPart+=IntToStr(i+1)+",";
                }
            }
            bIsSocketSensor=true;
            ShowErrorMessage("WAR0323", K_RETRY, MTestZ1, false, ErrPart);      //Socket detect device floting error
            Task=5000;
            break;                                                              //Steven 200604 : 使用Socket Sensor驗證置偏
        case 5900:                                                              //Richard 20220321 : 渠梁Side Push
            if(Do_Z1_AutoGetHeightDelay.Off()==false)
            {
                break;
            }

            if(CosFunction.bFixedDropSpeed)                                     //Steven 20131101 : 使用固定的Drop速度
            {
                fDropPos=atof(edContactHeight1->Text.c_str())+(double)CosFunction.dLimitMinDropOffset;
            }
            else
            {
                fDropPos=atof(edContactHeight1->Text.c_str())+atof(edDropOffset1->Text.c_str());
            }
            Pos=Get0_01MMType(FloatToStr(fDropPos).c_str());
            if(IniConfig.bChangeKitNoHardStop ||
               CosFunction.bContactShowOffset)                                  //JerryYang 20171201 (Steven) 修正 Contact height offset無效的問題
                Pos=Pos+Get0_01MMType(edContactOffsetArm1->Text.c_str());
            if(MOT[MTestZ1].Gali_MotMove(Pos-100, GotIndexZSpeed(iSpeedZ*1000)))
            {
                Task=6000;
                Do_Z1_AutoGetHeightDelay.SetSecAndOn(DeviceForm_File.dSitePushWaitTime);
                break;
            }
            break;
        case 6000:
            if(Do_Z1_AutoGetHeightDelay.Off()==false)
            {
                break;
            }

            if(DoTesterSidePush(true)==1)
            {
                Do_Z1_AutoGetHeightDelay.SetSecAndOn(DeviceForm_File.dSitePushWaitTime);
                Task=6010;                                                      //Richard 20230301 : 渠梁Side Push 新模式更改6100->2920
            }
            break;
        case 6010:
            if(Do_Z1_AutoGetHeightDelay.Off()==false)
            {
                break;
            }
            Task=2920;
            break;
        case 6100:
            if(Do_Z1_AutoGetHeightDelay.Off()==false)
            {
                break;
            }

            if(DoTesterSidePush(false)==10)
            {
                Task=3010;                                                      //Richard 20230301 : 渠梁Side Push 新模式更改2920->3010
                Do_Z1_AutoGetHeightDelay.SetSecAndOn(DeviceForm_File.dSitePushWaitTime);
            }
            break;
        case 6200:
            if(DoTesterSidePush(false)==10)
            {
                Task=3200;
            }
            break;
    }
    return false;
}
//------------------------------------------------------------------------------
TQPF_Timer Do_Z2_AutoGetHeightDelay;
TQPF_Timer Z2ClampCloseDelay;                                                   //JerryYang 20160621
TQPF_Timer Z2ClampTimeOutDelay;                                                 //JerryYang 20160621
TQPF_Timer Z2ClampOpenDelay;                                                    //JerryYang 20160621
bool TfContact::Do_Z2_AutoGetHeight()   //AI(W906-E042) 20261005 (B3): golden `bool __fastcall` -- __fastcall dropped as in forms/fContact.cpp (the declaration has none)
{
    static int iTestSidePushcount=0;                                            //Richard 20220321 : 渠梁Side Push
    static int iHeightStep=0;
    static int kg=0, vkg=0;                                                     //Steven 20100604 : For Mitsubishi
    static int Counter=0, Pos=0, iStepSpeed=0, iManualSpeed=100;
    static int iRecordZ2Pos=0, iRecordZ2Pos1=0, iRecordZ2Pos2=0;

    static bool bZ1Z2Press, bEPLeakage=false;
    static bool bRecordZ2Pos=false;
    static bool bAlarmGoTask560=true;
    static bool bCheckICFall[4][8]={{false, false, false, false, false, false, false, false},
                                    {false, false, false, false, false, false, false, false},
                                    {false, false, false, false, false, false, false, false},
                                    {false, false, false, false, false, false, false, false}};                          //Steven 20100129

    static double dHeight[2]={0.0, 0.0};

    int &Task=Z_Height_Task, ret=0, PosY2=0;
    int iSiteOn[4]={0, 0, 0, 0};
    bool bCheckDestroy=false;
    bool flag=false, bflag=false;                                               //kevin 20140612
    bool bHasErr=false, bZMotMove=false;
    double fDropPos=0.0;                                                        //JerryYang 20160701 int->double
    TEdit *tempEdit[]={edPinCount, edForcePerPinN, edForcePerPinG};
    AnsiString ErrPart="", str="";
    ShowMainScreenPresure(1);                                                   //kevin 20130605 read Torque send gpib use

    if(W906_IndexZDriveFaultStop("Do_Z2_AutoGetHeight")) { fAllMotorHome=false; CarlibrationTask=1; return false; }   if(W906_IndexZShuttleHomeStop(Task, "Do_Z2_AutoGetHeight")) { fAllMotorHome=false; CarlibrationTask=1; return false; }   switch(Task)   //AI(W906-E042) 20261005 (B3) P7 + W-44 NOT GOLDEN (Steven; ST01-M 1005 03:4x socket press only): a press task with the In / Out shuttle away from home -> ST + message + golden 536 exit; PCI1203 Z1 SHIP only
    {
        case 1: if(W906_IndexZRunRefused(iContactMode, 1, "Do_Z2_AutoGetHeight 1")) { SystemStart=false; CarlibrationTask=1; return false; }   /*AI(W906-E042) 20261005 (B3) P4+P8+P7 NOT GOLDEN: arm 2 is refused on a PCI-1203 Index Z (HT9050 has one index arm)*/ 
            dHeight[0]=0.0;
            dHeight[1]=0.0;
            iHeightStep=0;

            bHasErr|=CheckIndexAllSuckICFallDown(false, true);
            for(int i=0; i<BTestSuck.iShtRow; i++)                                                                                                              //Steven 20110516 Start: IC掉落要Alarm,並整合成一次
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
                if(CosFunction.bJAM0303NeedOpenChamberDoor)                                                                                                     //Steven : JAM0303 & JAM0403需要開啟Chamber門10秒
                    bIsTestSitICFallDown=true;
                ShowErrorMessage("JAM0304", K_SKIP, MTestZ2, false, ErrPart);                                                                                   //Steven 20100129 : Device Drop Error
                for(int i=0; i<BTestSuck.iShtRow; i++)
                {
                    for(int j=0; j<BTestSuck.iShtCol; j++)
                    {
                        if(BTestSuck.Suck[i][j].Error ||
                           (BTestSuck.Item[i][j]!=HAS_NULL_IC &&
                            BTestSuck.Item[i][j]!=NULL_IC &&
                            BTestSuck.Suck[i][j].GetStatus()==false))                                                                                           //有用到且有吸到IC的卻掉了
                        {
                            BTestSuck.SetItemData(i, j, HAS_NULL_IC);                                                                                           //Steven 20110829 : 把有IC掉料的位置改成Has Null IC
                            BTestSuck.Suck[i][j].Normal();                                                                                                      //Steven 20110829 : 把真空關掉
                            bCheckICFall[i][j]=false;
                        }
                    }
                }
                break;
            }

            bEPLeakage=false;
            for(int i=0; i<BTestSuck.iShtRow; i++)                                                                                                              //Steven 20100129 Start: IC掉落要Alarm
            {
                for(int j=0; j<BTestSuck.iShtCol; j++)
                {
                    if(BTestSuck.Suck[i][j].GetStatus()==true)                                                                                                  //有用到且有吸到IC的
                    {
                        bCheckICFall[i][j]=true;
                    }
                    else
                    {
                        BTestSuck.Suck[i][j].Normal();
                        bCheckICFall[i][j]=false;
                    }
                }
            }

            if(ATC_SYSTEM==eATCHonPrecType && ATC_SYSTEM==eATCHonPrecType)                                                                                      //Dell 20140509
                ATCInterfaceForm->ATC_SYS.SetNowArm(1);                                                                                                         //ATC

            if(ATC_SYSTEM==eATC60 ||                                                                                                                            //20141204 ChungHung add for ATC3.0
               ATC_SYSTEM==eATC30)                                                                                                                              //2014-05-30    Dell    for ATC6.0
            {
                ATCInterfaceForm->ATC_60_SYS.SetHandlerNowArm(1);
            }
            else if(ATC_SYSTEM==eNewATCSystem)                                                                                                                  //Ifor 20151230 :add New ATC Interface HandlerArm
            {
                {} /*GATE(W906-E042-ATC) ATC_InterfaceForm->HandlerArm(1); -- AI(W906-E042): as above (the ATC 'which arm' notice)*/
            }

            if(TestIF.bUseSLKClamp==true &&
               TestIF_File.iSeparabilityTest==1)                                                                                                                //JerryYang 20160616 分離流程
            {
                if(Cylinder[C_SLK2_Clamp].OnSensor()==false)
                {
                    ShowMyMessage("Please check the clamp I/O of arm2 layout kit", "請確認Arm2 layout kit clamp汽缸的I/O是否正常");
                    return false;
                }
            }

            if(INSTALL_SOCKET_CLAMP)                                                                                                                            //JerryYang 20160607 機台選用分離機構 需偵測socket sensor
            {
                if(Sen[SnSocketHasClamp1].IsOn() ||
                   Sen[SnSocketHasClamp2].IsOn())
                {
                    ShowMyMessage("Please check the socket sensor","socket sensor偵測異常");
                    return false;
                }
            }

            EPSwitchOnOff(eEPSwBoth);
            if(MOT[MTestZ1].Gali_Two_ZAxis_Move(Prod.TestZ1_Safe, GotIndexZSpeed(iSpeedZ*1000), "Do_Z2_AutoGetHeight 1"))                                       //JimmyChiu 20211028 : All speed can set by speed setting.
            {
                palArm2Height->Caption=0;
                bRecordZ2Pos=false;
                Task=100;
            }
            break;
        case 100:                                                                                                                                               //JimmyChiu 20220708 : add Index Arm Axis
            if(MOT[MTestY1].GalilTwoY_Move(Prod.TestY1_Front, Prod.TestY2_Middle, GotIndexYSpeed(iSpeed*3000), "Do_Z2_AutoGetHeight 100"))                      //JimmyChiu 20211028 : All speed can set by speed setting.
                Task=110;
            break;
        case 110:
            if(IniConfig.bD58UseArm1PickPlaceArm2Test==true &&                  //JerryYang 20250515
               TestIF_File.bArm1PickPlaceArm2Test==true)
            {
                if(IniConfig.bC08_SocketSensor &&
                   TestIF_File.bEnSocketSensor)
                {
                    bflag=false;
                    str="";
                    for(int i=0; i<TestIF_File.iSocketCount; i++)
                    {
                        if(TestIF_File.iSensorCheckType[i]==2 &&                //Steven 20200420 : Socket Sensor功能可以選
                           Sen[SThreadPara.iSocketSensor[i]].IsOn())
                        {
                            bflag=true;
                            str+=IntToStr(i+1)+",";
                        }
                    }

                    if(bflag)
                    {
                        ShowErrorMessage("WAR0323", K_RETRY, MTestZ1, false, str);
                        break;
                    }
                }
            }

            bHasErr=false;
            for(int i=0; i<BTestSuck.iShtRow; i++)
            {
                for(int j=0; j<BTestSuck.iShtCol; j++)
                {
                    if(bCheckICFall[i][j]==true &&
                       BTestSuck.Suck[i][j].GetStatus()==false)                 //有用到且有吸到IC的卻掉了
                    {
                        ErrPart+=IndexSuckName[i][j];
                        bHasErr=true;
                    }
                }
            }

            if(LastSet.iRealDummy==REALLY && bHasErr)
                Task=2100;
            else
                Task=200;
            break;
        case 200:
            fHome->InitDoTestZHome();
            Task=520;
            break;
        case 520:
            if(iContactMode==CONTACT_MANUAL_GET_HEIGHT)                         // 手動K
            {
                ADAM_WriteVoltage(DeviceForm.dPress);
                SW[SwManualZ2].On();
                if(IniConfig.bD10ManualHeightComptibleWithNS)
                {
                    SW[SwManualZ1].Off();
                    Task=530;
                }
                else
                {
                    SW[SwManualZ2].On();
                    MotorStatus=false;
                    Task=950;
                }
            }
            else if(iContactMode==CONTACT_TEST ||                               // contact test
                    iContactMode==STEP_CONTACT_TEST ||                          //Steven 20150811 : Step by Step Contact Test
                    iContactMode==CONTACT_DEVICE_MAP_CHECK)                     //Steven 20191009 : Device Map for Qualcomm
            {
                ADAM_WriteVoltage(DeviceForm.dPress);
                if(iContactMode==CONTACT_TEST &&
                   (cbContactMode->ItemIndex==DirectContactSoftEP ||
                    cbContactMode->ItemIndex==DropContactSoftEP))
                {                                                               //kevin 20130608 馬達到定點 ep 再充氣
//                    bContSoftEpSwitch(1,false);                                 //ARM2 浮動頭不充氣   //JerryYang 20151202 true->flase
                    EPSwitchOnOff(eEPSwArm1);
                }

                if(EP_Install==3 && IniConfig.bD24EnableEPCheckFuntion==true)   //JerryYang 20240111 : add
                {
                    Do_Z2_AutoGetHeightDelay.SetSecAndOn(2);
                    Task=2800;
                }
                else
                {
                    Task=2900;
                }
            }
            else                                                                // CONTACT_AUTO_GET_HEIGHT
            {
                if(IniConfig.iD17_UseHardwareHeightToContact==3)                //Steven 20231025 : K兩次作比對
                {
                    if(iHeightStep==0)
                        ADAM_WriteMaxData(true);                                //Steven 20241014 : 整合auto height輸出壓力
                    else
                        ADAM_WriteMaxData(false);
                }
                else if(IniConfig.iD17_UseHardwareHeightToContact==2)           //Steven 20231025 : 不充氣K高
                {
                    ADAM_WriteVoltage(dMinForce/2.0);
                }
                else
                {
                    ADAM_WriteMaxData(true);                                    //Steven 20241014 : 整合auto height輸出壓力
                }

                for(int i=0; i<3; i++)                                          //Steven 20100624 : K高過程不可以改變
                   tempEdit[i]->Enabled=false;

                Task=530;

                if(CUSTOMER_CODE==CC_AMKOR_China ||                             //jou 2013-04-25 因為 HT9045W 增加鋼瓶,充飽氣需要5 sec的時間,所以增加等待時間
                   CUSTOMER_CODE==CC_QUALCOMM)                                  //JerryYang 20170412 (Steven) add QUALCOMM
                {
                    if(INDEX_PRESS_TYPE==e85KG && INDEX_SUCKER_TYPE==0)
                    {
                        Do_Z2_AutoGetHeightDelay.SetSecAndOn(3);
                        Task=525;
                    }
                }
            }
            break;
        case 525:
            if(Do_Z2_AutoGetHeightDelay.Off())
            {
                Task=530;
            }
            break;
        case 530:
            if(IniConfig.iD17_UseHardwareHeightToContact==3 && iHeightStep==1)                                          //Steven 20231025 : 不充氣K高
            {
                bZMotMove=true;
            }
            else
            {
                bZMotMove=MOT[MTestZ2].Gali_MotMove(Tech.iTestZDown-5000, GotIndexZSpeed(iSpeedZ*1000));                //JimmyChiu 20211028 : All speed can set by speed setting.
            }

            if(bZMotMove)
            {
                InitWriteAndCheckMotorTorqueTask();
                kg=GetAutoHeightMaxKGTorque();                                                                          //Steven 20170720 (wei) : for low contact force
                Task=535;
                iManualSpeed=100;                                                                                       //Steven 20100223
            }
            break;
        case 535:
            bZ1Z2Press=false;
            Task=536;
            break;
        case 536:
            if(USE_IO_CHANGE_TOQUE==true)                                       //jou 2012-06-21 Enable index I/O Change Toque
            {
                SW[SwIndexChangeToque1].Off();
                SW[SwIndexChangeToque2].Off();
            }

            if(INDEX_DRIVER_TYPE==Panasonic_DRIVER)                             //Panasonic
            {
                ret=COM2->iWriteAndCheckMotorTorque(1, kg);
            }
            else
            {
                vkg=kgTranToMitsubishikg(kg);
                ret=COM2->iWriteAndCheckMotorTorque(1, vkg);
            }

            if(ret==1)
            {
                Task=550;
                Counter=0;
                if(IniConfig.iD17_UseHardwareHeightToContact==3 && iHeightStep==1)
                    iStepSpeed=100;
                else
                    iStepSpeed=300;
            }
            else if(ret==2)
            {
                ShowMyMessage("Index Z2 Motor torque set error", "Z2馬達扭力設定錯誤!", "Do_Z2_AutoGetHeight 536");
                fAllMotorHome=false;
                CarlibrationTask=1;
                return false;
            }
            break;
        case 550:
            COM2->InitReadTorueTask();
            fMain->chkReadTorque1->Checked=false;
            fMain->chkReadTorque2->Checked=true;
            fMain->edTorue1->Text="";
            Task=555;
        case 555:
            #ifndef SOFT_SIMULTE
                if(W906_IndexZTorqueWaitStop(1, fMain->edTorue1->Text=="", "Do_Z2_AutoGetHeight 555")) { fAllMotorHome=false; CarlibrationTask=1; return false; }   if(fMain->edTorue1->Text=="")   //AI(W906-E042) 20261005 (B3) P5 NOT GOLDEN: 5 s without a torque value -> ST Z2 + message + golden 536 exit (golden waits for ever)
                    break;
            #else
                fMain->edTorue1->Text=30;
            #endif
            Pos=MOT[MTestZ2].Gali_ReadPos();
            edContactHeight2->Text=ConvertTouMType(MOT[MTestZ2].Gali_ReadEncoderPos());
            TorqueData=atoi(fMain->edTorue1->Text.c_str());

            if(INDEX_DRIVER_TYPE==Mitsubishi_DRIVER)
            {
                PnlTorue0->Caption="";
                if(TorqueData>=kg-6)                                            // 2010.05.19 , Joye
                {
                    TorqueData=kg;
                }
            }

            if(INDEX_DRIVER_TYPE==Panasonic_DRIVER)                             //Panasonic
                TorqueData=abs(TorqueData);

            if(TorqueData>=kg ||
               atof(edContactHeight2->Text.c_str())<=fIndexDownPos)             //jou 2010-06-29 start : auto high 如不裝 socket base 需可執行結束,直接設入最大容許值，並showt出alarm!
            {
                if(IniConfig.bD10ManualHeightComptibleWithNS &&
                   iContactMode==CONTACT_MANUAL_GET_HEIGHT)
                {
                    MOT[MTestZ2].Gali_Command("ST", __FUNC__);
                    fMain->SendMSG_CMD(MSG_CMD_Arm2Down);                       //Ifor 20210331 :add Contact Mode GPIB需要知道哪隻Arm在Socker
                    if(ATC_SYSTEM==eNewATCSystem &&
                       Temperature.bATCActiveCooling==true)                     //JerryYang 20220815 : send ATC which ARM
                    {
                        {} /*GATE(W906-E042-ATC) ATC_InterfaceForm->HandlerArm(1); -- AI(W906-E042): as above (the ATC 'which arm' notice)*/
                    }
                    iWhichArmDown=2;
                    Task=560;
                }
                else
                {
                    Counter++;
                    if(INDEX_DRIVER_TYPE==Mitsubishi_DRIVER)
                        PnlTorue0->Caption=TorqueData;

                    if(Counter>=10)                                             // avoid torque on only short time
                    {
                        Counter=0;

                        Pos=MOT[MTestZ2].Gali_ReadPos();
                        MOT[MTestZ2].Gali_Command("ST", __FUNC__);
                        if(bRecordZ2Pos==false)                                 //取得充飽氣的高度
                        {
                            iRecordZ2Pos1=MOT[MTestZ2].Gali_ReadEncoderPos();
                            edContactHeight2->Text=ConvertTouMType(iRecordZ2Pos1);

                            if(atof(edContactHeight2->Text.c_str())<=fIndexDownPos)
                            {                                                   //jou 2010-06-29 start : auto high 如不裝 socket base 需可執行結束,直接設入最大容許值，並show出alarm!
                                iRecordZ2Pos1=fIndexDownPos*100;
                                edContactHeight2->Text=fIndexDownPos;
                                ShowMyMessage("Attention!! Over Z2 contact high! Be sure!","注意!!超過Z2 contact高度!需確認!");
                            }

                            if(EP_Install==3 || EP_Install==5)
                            {
                                if(IniConfig.bD26EnableEPEncoderRange==true)    //ChungHung 20111217
                                {
                                    ADAM_Rang(IniConfig.iD26EPEncoderRange);
                                    if(EP_Install==5)
                                        bEPLeakage=ADAM_Alarm(1);
                                    else
                                        bEPLeakage=ADAM_Alarm();
                                }
                                else
                                {
                                    bEPLeakage=false;
                                }
                            }
                            else
                            {
                                if(Sen[SnEPAlarm].Enable==true)
                                {
                                    bEPLeakage=Sen[SnEPAlarm].IsOff();          //Steven 20110622 : 檢查EP有沒有漏
                                }
                                else
                                {
                                    bEPLeakage=false;
                                }
                            }

                            ADAM_WriteVoltage(0);
                            MySleep(3000);
                            ADAM_WriteVoltage(0);
                            bRecordZ2Pos=true;
                            Task=700;
                        }
                        else                                                    //取得洩氣的高度
                        {
                            iRecordZ2Pos2=MOT[MTestZ2].Gali_ReadEncoderPos();
                            edContactHeight2->Text=ConvertTouMType(iRecordZ2Pos2);
                            Task=700;
                        }
                    }
                    else
                    {
                        Task=550;
                    }
                }
            }
            else
            {
                Counter=0;
                fMain->SendMSG_CMD(MSG_CMD_Arm2Down);                           //Ifor 20210331 :add Contact Mode GPIB需要知道哪隻Arm在Socker
                if(ATC_SYSTEM==eNewATCSystem &&
                   Temperature.bATCActiveCooling==true)                         //JerryYang 20220815 : send ATC which ARM
                {
                    {} /*GATE(W906-E042-ATC) ATC_InterfaceForm->HandlerArm(1); -- AI(W906-E042): as above (the ATC 'which arm' notice)*/
                }
                iWhichArmDown=2;
                Task=560;
            }
            break;
        case 560:
            bAlarmGoTask560=true;
            if(USE_INDEX_ARM_AXES==IndexArm_3_Axis)                                                                     //JimmyChiu 20220708 : add Index Arm Axis
            {
                PosY2=MOT[MTestY1].Gali_ReadPos();
                if(abs(PosY2-Prod.TestY1_Front)>10)                                                                     //JerryYang 20160824 Start:Z2下降前增加防護
                {
                    ShowMyMessage("Index Y1 Motor Position error", "馬達Y1位置錯誤!", "Do_Z2_AutoGetHeight 560");
                    return false;
                }
            }
            else
            {
                PosY2=MOT[MTestY2].Gali_ReadPos();
                if(abs(PosY2-Prod.TestY2_Middle)>10)
                {
                    ShowMyMessage("Index Y2 Motor Position error", "馬達Y2位置錯誤!", "Do_Z2_AutoGetHeight 560");
                    return false;
                }
            }

            if(MOT[MTestZ2].Gali_ReadPos()>(Prod.TestZ2_Test-Prod.TestZ2_Drop_Offset+750))                              //kevin 20140612 add start
            {
                if(ATC_InterfaceForm->iATC_MODE_TYPE==ATC_TYPE_70 &&
                   Temperature.bATC7TSDFunction==true)                                                                  //Steven 20161201 : by site TSD for Contact Test
                {
                    iSiteOn[0]=0;
                    iSiteOn[1]=0;
                    iSiteOn[2]=(BTestSuck.Item[0][0]==HAS_IC || BTestSuck.Item[0][0]==HAS_HOT_IC)?1:0;
                    iSiteOn[3]=(BTestSuck.Item[0][1]==HAS_IC || BTestSuck.Item[0][1]==HAS_HOT_IC)?1:0;
                    { (void)iSiteOn; } /*GATE(W906-E042-ATC) ATC_InterfaceForm->UseTSD_Function(4, iSiteOn); -- AI(W906-E042): ATC_InterfaceForm is TATC_InterfaceFormShim (acarry_shims.h:115, iATC_MODE_TYPE only); the real TATC_InterfaceForm (forms/fATCHandlerSide.h) has no global -- same gate as atester.cpp R04 / T04 (W906-GB-P2b); HT9050 USE_ATC_MODE=5 = no ATC*/
                }
                ATC_SwitchTjSignal(2, false);                                                                           //Ifor 20210622 add: ATC Switch TJ
                flag=false;
                if(LastSet.iRealDummy==REALLY)
                {
                    for(int i=0; i<BTestSuck.iShtRow; i++)
                    {
                        for(int j=0; j<BTestSuck.iShtCol; j++)
                        {
                            if(BTestSuck.Suck[i][j].Enable       &&                                                     //Steven 20110725 : 不再使用IsSuckICFallDown
                               BTestSuck.Suck[i][j].SenUsing!="" &&
                               BTestSuck.Item[i][j]!=HAS_NULL_IC &&
                               BTestSuck.Item[i][j]!=NULL_IC)
                            {
                                if(BTestSuck.Suck[i][j].GetStatus()==false)
                                {
                                    BTestSuck.Suck[i][j].Normal();                                                      //jou 2012-01-17 直接關掉，避免掉到shuttle去，也避免要掉不掉Hang up
                                    flag=true;
                                    bRecIndexDropAlarm2=true;                                                           //jou 2012-01-17 紀錄index Drop alarm
                                }
                            }
                        }
                    }
                }

                if(flag)
                {
                    MOT[MTestZ1].Gali_Command("ST", __FUNC__);
                    Task=564;
                    return false;
                }
            }                                                                                                           //----------kevin 20140612  掉料

            if(IniConfig.bD10ManualHeightComptibleWithNS &&
               iContactMode==CONTACT_MANUAL_GET_HEIGHT)
            {
                if(Sen[SnBMotorDown].IsOff()==false &&
                   TorqueData<kg)
                {
                    bZ1Z2Press=true;
                    MOT[MTestZ2].Gali_JogNSetup(iManualSpeed);
                    if(iManualSpeed<5000)
                        iManualSpeed+=100;
                }
                else if(Sen[SnFMotorDown].IsOff()==false)
                {
                    Pos=MOT[MTestZ2].Gali_ReadPos();
                    if(Pos<Tech.iTestZDown)
                    {
                        bZ1Z2Press=true;
                        MOT[MTestZ2].Gali_JogPSetup(iManualSpeed);

                        if(iManualSpeed<5000)
                            iManualSpeed+=100;
                    }
                }
                else
                {
                    iManualSpeed=100;                                                                                   //Steven 20100223
                    MOT[MTestZ2].Gali_Command("ST", __FUNC__);
                    if(WaitManualStartKey())                                                                            //Test IC
                    {
                        if(ATC_InterfaceForm->iATC_MODE_TYPE==ATC_TYPE_70 &&
                           Temperature.bATC7TSDFunction==true)                                                          //Steven 20161201 : by site TSD for Contact Test
                        {
                            iSiteOn[0]=0;
                            iSiteOn[1]=0;
                            iSiteOn[2]=(BTestSuck.Item[0][0]==HAS_IC || BTestSuck.Item[0][0]==HAS_HOT_IC)?1:0;
                            iSiteOn[3]=(BTestSuck.Item[0][1]==HAS_IC || BTestSuck.Item[0][1]==HAS_HOT_IC)?1:0;
                            { (void)iSiteOn; } /*GATE(W906-E042-ATC) ATC_InterfaceForm->UseTSD_Function(4, iSiteOn); -- AI(W906-E042): ATC_InterfaceForm is TATC_InterfaceFormShim (acarry_shims.h:115, iATC_MODE_TYPE only); the real TATC_InterfaceForm (forms/fATCHandlerSide.h) has no global -- same gate as atester.cpp R04 / T04 (W906-GB-P2b); HT9050 USE_ATC_MODE=5 = no ATC*/
                        }
                        ATC_SwitchTjSignal(2);                                                                          //Ifor 20210622 add: ATC Switch TJ
                        fMain->SendMSG_CMD(MSG_CMD_Arm2Down);                                                           //Steven 20150304 : Add GPIB LOG
                        if(ATC_SYSTEM==eNewATCSystem &&
                           Temperature.bATCActiveCooling==true)                                                         //JerryYang 20220815 : send ATC which ARM
                        {
                            {} /*GATE(W906-E042-ATC) ATC_InterfaceForm->HandlerArm(1); -- AI(W906-E042): as above (the ATC 'which arm' notice)*/
                        }
                        iWhichArmDown=2;
                        {} /*GATE(W906-E042-UI) fTestCategory->BringToFront(); -- AI(W906-E042): window z-order only; TfTestCategory has no base class / BringToFront (forms/fTestCategory.h:257 D-2)*/                                                                  //Steven 20100823
                        if(IniConfig.bC04EnableTestTempIC)
                            {} /*GATE(W906-E042-UI) fDynamicTemp->BringToFront(); -- AI(W906-E042): window z-order only; fDynamicTemp has no global (forms/fContact.h:458-461 X-13)*/                                                               //Steven 20120810
                        iSetupTask=1;
                        IsTest=true;                                                                                    //Steven 20110920
                        Task=600;
                        labDelayStatus->Caption="Waiting Test Result";
                        break;
                    }

                    if(WaitManualStepKey())
                    {
                        if(ATC_InterfaceForm->iATC_MODE_TYPE==ATC_TYPE_70 &&
                           Temperature.bATC7TSDFunction==true)                                                          //Steven 20161201 : by site TSD for Contact Test
                        {
                            iSiteOn[0]=0;
                            iSiteOn[1]=0;
                            iSiteOn[2]=0;
                            iSiteOn[3]=0;
                            { (void)iSiteOn; } /*GATE(W906-E042-ATC) ATC_InterfaceForm->UseTSD_Function(4, iSiteOn); -- AI(W906-E042): ATC_InterfaceForm is TATC_InterfaceFormShim (acarry_shims.h:115, iATC_MODE_TYPE only); the real TATC_InterfaceForm (forms/fATCHandlerSide.h) has no global -- same gate as atester.cpp R04 / T04 (W906-GB-P2b); HT9050 USE_ATC_MODE=5 = no ATC*/
                        }

                        iTriggerBoostFunction=-1;
                        iTriggerBoostFuncBack=-1;
                        iBoostFuncStep=5;
                        edContactHeight2->Text=ConvertTouMType(MOT[MTestZ2].Gali_ReadEncoderPos());
                        InitWriteAndCheckMotorTorqueTask();
                        SW[SwRKManualTStart].Off();
                        IsTest=false;                                                                                   //Steven 20120505 Start: 離開時,要通知GPIB重置測試狀態
                        TestSocket.ClearAll();
                        if(TestIF.iGpibMode==InterfaceType_SPEA_Type)
                        {
                            bool flag[32]={false};
                            if(BAR_CODE_INSTALL!=ebctUninstall && TestIF_File.bEnableBarCode)
                                _RunTestProgram_BarMess(sizeof(flag), flag, 0, "");
                            else
                                _RunTestProgram(sizeof(flag), flag);
                        }
                        else
                        {
                            fMain->RunTestProgram(false);
                        }

                        fMain->SendMSG_CMD(MSG_CMD_ContactTestAbort);                                                   //Steven 20150304 : Add GPIB LOG
                        labDelayStatus->Caption="";
                        Task=900;
                        break;
                    }
                }
                Task=550;
            }
            else
            {
                if(W906_IndexZCommandFloorStop(Pos, fIndexDownPos, "Do_Z2_AutoGetHeight 560")) { fAllMotorHome=false; CarlibrationTask=1; return false; }                   if(MOT[MTestZ2].Gali_MotMoveSkipEncoder(Pos-iStepSpeed, GotIndexZSpeed(iSpeedZ*1000)))                  //JimmyChiu 20211028 : All speed can set by speed setting.
                {
                    MOT[MTestZ2].Gali_ScanMotStatus();
                    if(MOT[MTestZ2].Led[iInposLed]==false)
                    {
                        palArm2Height->Caption = Pos-iStepSpeed;
                        Task=550;
                    }
                }
            }
            break;
        case 564:                                                               //kevin 20140612  上升方便取料
            if(MOT[MTestZ2].Gali_MotMove(-1000, GotIndexZSpeed(iSpeedZ*1000)))  //JimmyChiu 20211028 : All speed can set by speed setting.
            {
                Task=565;
            }
            break;
        case 565:
            bHasErr|=CheckIndexAllSuckICFallDown(false, true);
            for(int i=0; i<BTestSuck.iShtRow; i++)
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
                for(int i=0; i<BTestSuck.iShtRow; i++)
                {
                    for(int j=0; j<BTestSuck.iShtCol; j++)
                    {
                        if(BTestSuck.Suck[i][j].Error ||
                           (BTestSuck.Item[i][j]!=HAS_NULL_IC &&
                            BTestSuck.Item[i][j]!=NULL_IC &&
                            BTestSuck.Suck[i][j].GetStatus()==false))           //有用到且有吸到IC的卻掉了
                        {
                            BTestSuck.SetItemData(i, j, HAS_NULL_IC);           //Steven 20110829 : 把有IC掉料的位置改成Has Null IC
                            BTestSuck.Suck[i][j].Normal();                      //Steven 20110829 : 把真空關掉
                            bCheckICFall[i][j]=false;
                        }
                    }
                }
                break;
            }

            if(bIsTestSitICFallDown)
            {
                for(int i=0; i<BTestSuck.iShtRow; i++)
                    for(int j=0; j<BTestSuck.iShtCol; j++)
                        bCheckICFall[i][j]=false;                               //恢復原狀
            }

            if(bIsTestSitICFallDown==false)
            {
                if(bAlarmGoTask560)                                             //Steven 20190822 : Add alarm when auto height
                {
                    Task=560;
                }
                else
                {
                    Task=710;
                }
                return false;
            }
            break;
        case 600:
            if(DoSetupTest(1))
            {
                Task=560;
                break;
            }

            if(WaitManualStepKey())
            {
                if(ATC_InterfaceForm->iATC_MODE_TYPE==ATC_TYPE_70 &&
                   Temperature.bATC7TSDFunction==true)                          //Steven 20161201 : by site TSD for Contact Test
                {
                    iSiteOn[0]=0;
                    iSiteOn[1]=0;
                    iSiteOn[2]=0;
                    iSiteOn[3]=0;
                    { (void)iSiteOn; } /*GATE(W906-E042-ATC) ATC_InterfaceForm->UseTSD_Function(4, iSiteOn); -- AI(W906-E042): ATC_InterfaceForm is TATC_InterfaceFormShim (acarry_shims.h:115, iATC_MODE_TYPE only); the real TATC_InterfaceForm (forms/fATCHandlerSide.h) has no global -- same gate as atester.cpp R04 / T04 (W906-GB-P2b); HT9050 USE_ATC_MODE=5 = no ATC*/
                }

                edContactHeight2->Text=ConvertTouMType(MOT[MTestZ2].Gali_ReadEncoderPos());
                InitWriteAndCheckMotorTorqueTask();

                SW[SwRKManualTStart].Off();
                IsTest=false;                                                   //Steven 20120505 Start: 離開時,要通知GPIB重置測試狀態
                TestSocket.ClearAll();
                if(TestIF.iTestType==TTL_MODE && (TTL_CARD_TYPE<2))
                {
                    DoSetupTest();                                              //Steven 20230731 : Contact Test結束要清除TTL訊號
                }
                else if(TestIF.iGpibMode==InterfaceType_SPEA_Type)
                {
                    bool flag[32]={false};
                    if(BAR_CODE_INSTALL!=ebctUninstall && TestIF_File.bEnableBarCode)
                        _RunTestProgram_BarMess(sizeof(flag), flag, 0, "");
                    else
                        _RunTestProgram(sizeof(flag), flag);
                }
                else
                {
                    fMain->RunTestProgram(false);
                }
                InitWriteAndCheckMotorTorqueTask();
                labDelayStatus->Caption="";
                Task=900;
            }
            break;
        case 700:                                                               //Lee 2007_0208
            Pos=MOT[MTestZ2].Gali_ReadEncoderPos();
            Pos=Pos+3000;

            if(IniConfig.iD17_UseHardwareHeightToContact==3)                    //Steven 20231025 : 不充氣K高
            {
                dHeight[iHeightStep]=iRecordZ2Pos1;
                iHeightStep++;
                if(iHeightStep==1)
                    Pos=MOT[MTestZ2].Gali_ReadEncoderPos();
            }
            else if(IniConfig.iD17_UseHardwareHeightToContact==2)               //Steven 20231025 : 不充氣K高
            {
                iRecordZ2Pos=iRecordZ2Pos1+100;
            }
            else if(DeviceForm_File.dKitDiameter<=2.5)                          //kevin 20170804 (Steven) 20mm Auto Height
            {
                if(INDEX_PRESS_TYPE==e85KG)                                     //Frank 20250214 add
                    iRecordZ2Pos=iRecordZ2Pos1;
                else
                    iRecordZ2Pos=iRecordZ2Pos1+100;
            }
            else
            {
                if(IniConfig.iD17_UseHardwareHeightToContact==1)                //Steven 20170411 (wei) : SCK的SIP怕刮傷,所以Contact Height使用硬體高度
                {
                    iRecordZ2Pos=iRecordZ2Pos1;
                }
                else if(bEPLeakage==true)                                       //jou 2011-09-29 如果沒有設定alarm值,高度都不減會太高,所以改-20
                {
                    iRecordZ2Pos=iRecordZ2Pos1-20;
                }
                else
                {
                    iRecordZ2Pos=iRecordZ2Pos1-100;
                }
            }

            if(cbContactMode->ItemIndex==DropPlaceShiftContact)                 //ChungHung 20150528 add for 海思 _8Site1x4 in contact
                Task=701;
            else
                Task=710;
            break;
        case 701:                                                                                                       //ChungHung 20150528 add for 海思 _8Site1x4 in contact
            if(MOT[MTestZ2].Gali_MotMove(Prod.TestZ1_Safe, GotIndexZSpeed(iSpeedZ*1000)))                               //Z2 上升至安全位置   //JimmyChiu 20211028 : All speed can set by speed setting.
            {
                Task=702;
            }
            break;
        case 702:                                                                                                                                               //ChungHung 20150528 add for 海思 _8Site1x4 in contact
            if(MOT[MTestY1].GalilTwoY_Move(Prod.TestY2_Middle-TestIF.dSiteYPitch, Prod.TestY1_Front, GotIndexYSpeed(iSpeed*3000), "Do_Z2_AutoGetHeight 702"))   //JimmyChiu 20211028 : All speed can set by speed setting.
            {
                Task=710;
            }
            break;
        case 710:                                                                                                       //Lee 2007_0208
            if(MOT[MTestZ2].Gali_MotMoveSkipEncoder(Pos, GotIndexZSpeed(iSpeedZ*1000)))                                 //JimmyChiu 20211028 : All speed can set by speed setting.
            {
                palArm2Height->Caption=Pos;
                edContactHeight2->Text=ConvertTouMType(Pos);                                                            //contect high

                InitWriteAndCheckMotorTorqueTask();

                if(IniConfig.bD58UseArm1PickPlaceArm2Test==true &&                                                      //JerryYang 20250515
                   TestIF_File.bArm1PickPlaceArm2Test==true)
                {
                    if(IniConfig.bC08_SocketSensor &&
                       TestIF_File.bEnSocketSensor)
                    {
                        bflag=false;
                        str="";
                        for(int i=0; i<TestIF_File.iSocketCount; i++)
                        {
                            if(TestIF_File.iSensorCheckType[i]==2 &&                                                    //Steven 20200420 : Socket Sensor功能可以選
                               Sen[SThreadPara.iSocketSensor[i]].IsOn())
                            {
                                bflag=true;
                                str+=IntToStr(i+1)+",";
                            }
                        }

                        if(bflag)
                        {
                            ShowErrorMessage("WAR0323", K_RETRY, MTestZ1, false, str);
                            break;
                        }
                    }
                }

                if(IniConfig.iD17_UseHardwareHeightToContact==3)                                                        //Steven 20231025 : 不充氣K高
                {
                    if(iHeightStep<2)
                    {
                        Task=520;
                        bRecordZ2Pos=false;
                    }
                    else
                    {
                        Task=7110;
                    }
                }
                else
                {
                    ADAM_WriteVoltage(DeviceForm.dPress);
                    ADAM_WriteVoltage(0);
                    MySleepEx(1000, false);
                    ADAM_WriteVoltage(DeviceForm.dPress);
                    Task=7150;
                }
            }
            break;
        case 7110:
            if(abs(dHeight[0]-dHeight[1])<(IniConfig.dD17_3CheckEPLeakage*100.0))
            {
                Task=7120;
            }
            else
            {
                iRecordZ2Pos=dHeight[0]-100;
                ADAM_WriteVoltage(DeviceForm.dPress);
                ADAM_WriteVoltage(0);
                MySleepEx(1000, false);
                ADAM_WriteVoltage(DeviceForm.dPress);

                Task=7150;
            }
            break;
        case 7120:
            bZMotMove=MOT[MTestZ2].Gali_MotMove(0, GotIndexZSpeed(iSpeedZ*1000));
            if(bZMotMove)
            {
                Task=7125;
            }
            break;
        case 7125:
            ShowErrorMessage("WAR0378", 0, MTestY2);
            iRecordZ2Pos=dHeight[1]+100;
            ADAM_WriteVoltage(DeviceForm.dPress);
            ADAM_WriteVoltage(0);
            MySleepEx(1000, false);
            ADAM_WriteVoltage(DeviceForm.dPress);

            Task=7150;
            break;
        case 7150:                                                              //jou 2010-06-21 start : auto high加一段程式驗證是否有沒有掉O-Ring
            flag=false;
            if(LastSet.iRealDummy==REALLY)                                      //Steven 20190822 : Add alarm when auto height
            {
                for(int i=0; i<BTestSuck.iShtRow; i++)
                {
                    for(int j=0; j<BTestSuck.iShtCol; j++)
                    {
                        if(BTestSuck.Suck[i][j].Enable       &&                 //Steven 20110725 : 不再使用IsSuckICFallDown
                           BTestSuck.Suck[i][j].SenUsing!="" &&
                           BTestSuck.Item[i][j]!=HAS_NULL_IC &&
                           BTestSuck.Item[i][j]!=NULL_IC)
                        {
                            if(BTestSuck.Suck[i][j].GetStatus()==false)
                            {
                                BTestSuck.Suck[i][j].Normal();                  //jou 2012-01-17 直接關掉，避免掉到shuttle去，也避免要掉不掉Hang up
                                flag=true;
                                bRecIndexDropAlarm2=true;                       //jou 2012-01-17 紀錄index Drop alarm
                            }
                        }
                    }
                }
            }

            if(flag)
            {
                bAlarmGoTask560=false;
                MOT[MTestZ1].Gali_Command("ST", __FUNC__);
                Task=564;
                return false;
            }

            if(USE_IO_CHANGE_TOQUE==true)                                       //jou 2012-06-21 Enable index I/O Change Toque
            {
                SW[SwIndexChangeToque1].Off();
                SW[SwIndexChangeToque2].Off();
            }

            ret=COM2->iWriteAndCheckMotorTorque(1, 120);                        //jou 981128
            if(ret==1)
            {
                Task=7160;
            }
            else if(ret==2)
            {
                ShowMyMessage("Index Z2 Motor torque set error", "Z2馬達扭力設定錯誤!", "Do_Z2_AutoGetHeight 7150");
                fAllMotorHome=false;
                CarlibrationTask=1;
                return false;
            }
            break;
        case 7160:
            if(MOT[MTestZ2].Gali_MotMove(iRecordZ2Pos, GotIndexZSpeed(iSpeedZ*1000)))                                   //JimmyChiu 20211028 : All speed can set by speed setting.
            {
                palArm2Height->Caption = iRecordZ2Pos;
                edContactHeight2->Text=ConvertTouMType(iRecordZ2Pos);                                                   //contect high

                UpdateContactRelative();                                                                                //JerryYang 20250120 : add

                COM2->InitReadTorueTask();
                fMain->chkReadTorque1->Checked=false;
                fMain->chkReadTorque2->Checked=true;
                fMain->edTorue1->Text="";
                Counter=0;
                Task=7170;
            }
            break;
        case 7170:
            #ifdef SOFT_SIMULTE
                fMain->edTorue1->Text=30;
            #endif

            if(W906_IndexZTorqueWaitStop(1, fMain->edTorue1->Text=="", "Do_Z2_AutoGetHeight 7170")) { fAllMotorHome=false; CarlibrationTask=1; return false; }   if(fMain->edTorue1->Text!="")   //AI(W906-E042) 20261005 (B3) P5 NOT GOLDEN: 5 s without a torque value -> ST Z2 + message + golden 536 exit (golden waits for ever)
            {
                MySleep(500);
                if(abs(atoi(fMain->edTorue1->Text.c_str()))>=125)               //Frank 20150317 120->125
                {
                    if(Counter>=5)
                    {
                        ShowMyMessage("Z2 Motor Auto Height error,check EP Value!", "Z2馬達自動取得高度錯誤,請檢查EP是否漏氣!", "Do_Z2_AutoGetHeight 7170");
                        InitWriteAndCheckMotorTorqueTask();
                        MOT[MTestZ2].iGali_SingalHomeTask=1;
                        Task=905;
                        break;
                    }
                    Counter++;
                    COM2->InitReadTorueTask();
                    fMain->chkReadTorque1->Checked=false;
                    fMain->chkReadTorque2->Checked=true;
                    fMain->edTorue1->Text="";
                }
                else
                {
                    InitWriteAndCheckMotorTorqueTask();
                    Task=720;
                }
            }
            break;
        case 720:                                                               //Lee 2007_0208
            if(USE_IO_CHANGE_TOQUE==true)                                       //jou 2012-06-21 Enable index I/O Change Toque
            {
                SW[SwIndexChangeToque1].Off();
                SW[SwIndexChangeToque2].Off();
            }

            ret=COM2->iWriteAndCheckMotorTorque(1, 300);                        //jou 981128
            if(ret==1)
            {
                Task=730;
            }
            else if(ret==2)
            {
                ShowMyMessage("Index Z2 Motor torque set error", "Z2馬達扭力設定錯誤!", "Do_Z2_AutoGetHeight 720");
                fAllMotorHome=false;
                CarlibrationTask=1;
                return false;
            }
            break;
        case 730:                                                                                                       //jou 981128
            if(MOT[MTestZ2].Gali_MotMove(iRecordZ2Pos, GotIndexZSpeed(iSpeedZ*1000)))                                   //JimmyChiu 20211028 : All speed can set by speed setting.
            {
                palArm2Height->Caption = iRecordZ2Pos;
                InitWriteAndCheckMotorTorqueTask();
                edContactHeight2->Text=ConvertTouMType(iRecordZ2Pos);                                                   //contect high

                fMain->edTorue1->Text="999";
                Task=800;

                if(ATC_InterfaceForm->iATC_MODE_TYPE==ATC_TYPE_70 &&
                   Temperature.bATC7TSDFunction==true)                                                                  //Steven 20161201 : by site TSD for Contact Test
                {
                    iSiteOn[0]=0;
                    iSiteOn[1]=0;
                    iSiteOn[2]=(BTestSuck.Item[0][0]==HAS_IC || BTestSuck.Item[0][0]==HAS_HOT_IC)?1:0;
                    iSiteOn[3]=(BTestSuck.Item[0][1]==HAS_IC || BTestSuck.Item[0][1]==HAS_HOT_IC)?1:0;
                    { (void)iSiteOn; } /*GATE(W906-E042-ATC) ATC_InterfaceForm->UseTSD_Function(4, iSiteOn); -- AI(W906-E042): ATC_InterfaceForm is TATC_InterfaceFormShim (acarry_shims.h:115, iATC_MODE_TYPE only); the real TATC_InterfaceForm (forms/fATCHandlerSide.h) has no global -- same gate as atester.cpp R04 / T04 (W906-GB-P2b); HT9050 USE_ATC_MODE=5 = no ATC*/
                }
                ATC_SwitchTjSignal(2);                                                                                  //Ifor 20210622 add: ATC Switch TJ
                fMain->SendMSG_CMD(MSG_CMD_Arm2Down);                                                                   //Steven 20150304 : Add GPIB LOG
                if(ATC_SYSTEM==eNewATCSystem &&
                   Temperature.bATCActiveCooling==true)                                                                 //JerryYang 20220815 : send ATC which ARM
                {
                    {} /*GATE(W906-E042-ATC) ATC_InterfaceForm->HandlerArm(1); -- AI(W906-E042): as above (the ATC 'which arm' notice)*/
                }
                iWhichArmDown=2;
                for(int i=0; i<3; i++)                                                                                  //Steven 20100624 : K高過程不可以改變
                    tempEdit[i]->Enabled=true;
            }
            break;
        case 800:
            if(WaitManualStartKey())                                            //Steven 20100225
            {
                fMain->SendMSG_CMD(MSG_CMD_ContactTestArm2);                    //Steven 20150304 : Add GPIB LOG
                {} /*GATE(W906-E042-UI) fTestCategory->BringToFront(); -- AI(W906-E042): window z-order only; TfTestCategory has no base class / BringToFront (forms/fTestCategory.h:257 D-2)*/                                  //Steven 20100823
                if(IniConfig.bC04EnableTestTempIC)
                    {} /*GATE(W906-E042-UI) fDynamicTemp->BringToFront(); -- AI(W906-E042): window z-order only; fDynamicTemp has no global (forms/fContact.h:458-461 X-13)*/                               //Steven 20120810
                IsTest=true;                                                    //Steven 20110920
                iSetupTask=1;
                Task=810;
                labDelayStatus->Caption="Waiting Test Result";
                break;
            }

            if(WaitManualStepKey())
            {
                if(ATC_InterfaceForm->iATC_MODE_TYPE==ATC_TYPE_70 &&
                   Temperature.bATC7TSDFunction==true)                          //Steven 20161201 : by site TSD for Contact Test
                {
                    iSiteOn[0]=0;
                    iSiteOn[1]=0;
                    iSiteOn[2]=0;
                    iSiteOn[3]=0;
                    { (void)iSiteOn; } /*GATE(W906-E042-ATC) ATC_InterfaceForm->UseTSD_Function(4, iSiteOn); -- AI(W906-E042): ATC_InterfaceForm is TATC_InterfaceFormShim (acarry_shims.h:115, iATC_MODE_TYPE only); the real TATC_InterfaceForm (forms/fATCHandlerSide.h) has no global -- same gate as atester.cpp R04 / T04 (W906-GB-P2b); HT9050 USE_ATC_MODE=5 = no ATC*/
                }

                iTriggerBoostFunction=-1;
                iTriggerBoostFuncBack=-1;
                iBoostFuncStep=5;
                SW[SwRKManualTStart].Off();
                IsTest=false;                                                   //Steven 20120505 Start: 離開時,要通知GPIB重置測試狀態
                TestSocket.ClearAll();
                if(TestIF.iGpibMode==InterfaceType_SPEA_Type)
                {
                    bool flag[32]={false};
                    if(BAR_CODE_INSTALL!=ebctUninstall && TestIF_File.bEnableBarCode)
                        _RunTestProgram_BarMess(sizeof(flag), flag, 0, "");
                    else
                        _RunTestProgram(sizeof(flag), flag);
                }
                else
                {
                    fMain->RunTestProgram(false);
                }

                fMain->SendMSG_CMD(MSG_CMD_ContactTestAbort);                   //Steven 20150304 : Add GPIB LOG
                InitWriteAndCheckMotorTorqueTask();
                MOT[MTestZ2].iGali_SingalHomeTask=1;
                labDelayStatus->Caption="";
                Task=905;
            }

            if(fMain->edTorue1->Text!="")
            {
                COM2->InitReadTorueTask();
                fMain->chkReadTorque1->Checked=false;
                fMain->chkReadTorque2->Checked=true;
                fMain->edTorue1->Text="";
                bReadMCU2=true;                                                 //kevin 20220225 read MCU DATA
            }
            break;
        case 810:
            if(DoSetupTest(1))
            {
                Task=800;
                break;
            }

            if(WaitManualStepKey())
            {
                if(ATC_InterfaceForm->iATC_MODE_TYPE==ATC_TYPE_70 &&
                  Temperature.bATC7TSDFunction==true)                           //Steven 20161201 : by site TSD for Contact Test
                {
                    iSiteOn[0]=0;
                    iSiteOn[1]=0;
                    iSiteOn[2]=0;
                    iSiteOn[3]=0;
                    { (void)iSiteOn; } /*GATE(W906-E042-ATC) ATC_InterfaceForm->UseTSD_Function(4, iSiteOn); -- AI(W906-E042): ATC_InterfaceForm is TATC_InterfaceFormShim (acarry_shims.h:115, iATC_MODE_TYPE only); the real TATC_InterfaceForm (forms/fATCHandlerSide.h) has no global -- same gate as atester.cpp R04 / T04 (W906-GB-P2b); HT9050 USE_ATC_MODE=5 = no ATC*/
                }

                InitWriteAndCheckMotorTorqueTask();
                MOT[MTestZ2].iGali_SingalHomeTask=1;

                SW[SwRKManualTStart].Off();
                IsTest=false;                                                   //Steven 20120505 Start: 離開時,要通知GPIB重置測試狀態
                TestSocket.ClearAll();
                if(TestIF.iTestType==TTL_MODE && (TTL_CARD_TYPE<2))
                {
                    DoSetupTest();                                              //Steven 20230731 : Contact Test結束要清除TTL訊號
                }
                else if(TestIF.iGpibMode==InterfaceType_SPEA_Type)
                {
                    bool flag[32]={false};
                    if(BAR_CODE_INSTALL!=ebctUninstall && TestIF_File.bEnableBarCode)
                        _RunTestProgram_BarMess(sizeof(flag), flag, 0, "");
                    else
                        _RunTestProgram(sizeof(flag), flag);
                }
                else
                {
                    fMain->RunTestProgram(false);
                }
                labDelayStatus->Caption="";
                Task=905;                                                       //jou 981128
            }
            break;
        case 900:
            Pos=MOT[MTestZ2].Gali_ReadEncoderPos();
            Pos+=500*MOT[MTestZ2].Motor->GearRatio;                             //+100條=10mm Steven 20100225
            if(IniConfig.bD10ManualHeightComptibleWithNS &&
               iContactMode==CONTACT_MANUAL_GET_HEIGHT)                         //Steven 20100225
            {
                if(Pos>SafeTestZContactHeight)
                {
                    ShowMyMessage("Index Z2 test height too high error", "警告，Index Z2的測試高度太高!!", "Do_Z2_AutoGetHeight 900");
                    Task=200;
                    break;
                }
            }
            Task=9000;
            break;
        case 9000:
            if(MOT[MTestZ2].Gali_MotMoveSkipEncoder(Pos, GotIndexZSpeed(iSpeedZ*1000)))                                 // 以 40 kg 以上上升  5mm    //JimmyChiu 20211028 : All speed can set by speed setting.
            {
                palArm2Height->Caption=Pos;
                InitWriteAndCheckMotorTorqueTask();
                Task=9010;
            }
            break;
        case 9010:
            if(USE_IO_CHANGE_TOQUE==true)                                       //jou 2012-06-21 Enable index I/O Change Toque
            {
                SW[SwIndexChangeToque1].Off();
                SW[SwIndexChangeToque2].Off();
            }

            if(IniConfig.bD10ManualHeightComptibleWithNS)
            {
                ret=COM2->iWriteAndCheckMotorTorque(1, kg);
            }
            else
            {
                if(kg>50)
                    ret=COM2->iWriteAndCheckMotorTorque(1, 90);
                else
                    ret=COM2->iWriteAndCheckMotorTorque(1, kg+40);
            }

            if(ret==1)
            {
                Task=901;
            }
            else if(ret==2)
            {
                ShowMyMessage("Index Z2 Motor torque set error", "Z2馬達扭力設定錯誤!", "Do_Z2_AutoGetHeight 9010");
                fAllMotorHome=false;
                CarlibrationTask=1;
                return false;
            }
            break;
        case 901:
            Pos=MOT[MTestZ2].Gali_ReadPos();
            Task=90100;
            Pos+=1000;                                                          //+1cm
            break;
        case 90100:
            if(MOT[MTestZ2].Gali_MotMoveSkipEncoder(Pos, GotIndexZSpeed(iSpeedZ*1000)))                                 //JimmyChiu 20211028 : All speed can set by speed setting.
            {
                palArm2Height->Caption = Pos;
                InitWriteAndCheckMotorTorqueTask();
                Task=90110;
            }
            break;
        case 90110:
            if(USE_IO_CHANGE_TOQUE==true)                                       //jou 2012-06-21 Enable index I/O Change Toque
            {
                SW[SwIndexChangeToque1].Off();
                SW[SwIndexChangeToque2].Off();
            }

            ret=COM2->iWriteAndCheckMotorTorque(1, 99);
            if(ret==1)
                Task=90120;
            else if(ret==2)
            {
                ShowMyMessage("Index Z2 Motor torque set error", "Z2馬達扭力設定錯誤!", "Do_Z2_AutoGetHeight 90110");
                fAllMotorHome=false;
                CarlibrationTask=1;
                return false;
            }
            break;
        case 90120:
            if(MOT[MTestZ2].Gali_MotMoveSkipEncoder(Tech.iTestZDown, GotIndexZSpeed(iSpeedZ*1000)))                     // 以 40 kg 以上上升  5mm   //JimmyChiu 20211028 : All speed can set by speed setting.
            {
                palArm2Height->Caption=Tech.iTestZDown;
                InitWriteAndCheckMotorTorqueTask();
                Task=902;
            }
            break;
        case 902:
            if(USE_IO_CHANGE_TOQUE==true)                                       //jou 2012-06-21 Enable index I/O Change Toque
            {
                SW[SwIndexChangeToque1].Off();
                SW[SwIndexChangeToque2].Off();
            }

            ret=COM2->iWriteAndCheckMotorTorque(1, 300);
            if(ret==1)
            {
                Task=905;
                MOT[MTestZ2].iGali_SingalHomeTask=1;                            //Ifor 20161114 Fix AutoHigh 時 Hangup
            }
            else if(ret==2)
            {
                ShowMyMessage("Index Z2 Motor torque set error", "Z2馬達扭力設定錯誤!", "Do_Z2_AutoGetHeight 902");
                fAllMotorHome=false;
                CarlibrationTask=1;
                return false;
            }
            break;
        case 905:
            if(MOT[MTestZ2].Gali_SingalHome())
            {
                palArm2Height->Caption = 0;
                Task=2100;
            }
            else                                                                //Steven 20100208
            {
                if(MOT[MTestZ2].iGali_SingalHomeTask==900)
                    MOT[MTestZ2].iGali_SingalHomeTask=500;
            }
            break;
        case 950:
            InitWriteAndCheckMotorTorqueTask();
            Task=960;
        case 960:
            if(USE_IO_CHANGE_TOQUE==true)                                       //jou 2012-06-21 Enable index I/O Change Toque
            {
                SW[SwIndexChangeToque1].Off();
                SW[SwIndexChangeToque2].Off();
            }

            ret=COM2->iWriteAndCheckMotorTorque(1, Prod.iMaxPreasure);
            if(ret==1)
            {
                Task=1000;
            }
            else if(ret==2)
            {
                ShowMyMessage("Index Z2 Motor torque set error", "Z2馬達扭力設定錯誤!", "Do_Z2_AutoGetHeight 960");
                fAllMotorHome=false;
                CarlibrationTask=1;
                return false;
            }
            break;
        case 1000:
            if(Sen[SnBMotorDown].IsOff()==false)
            {
                MOT[MTestZ2].Gali_Command("MOZ", __FUNC__);                     // servo off
                MotorStatus=true;
            }
            else
            {
                if(MotorStatus)
                {
                    MotorStatus=false;
                    MOT[MTestZ2].Gali_Command("SHZ", __FUNC__);
                    COM2->InitReadTorueTask();
                    fMain->chkReadTorque1->Checked=false;
                    fMain->chkReadTorque2->Checked=true;
                    fMain->edTorue1->Text="";
                    bReadMCU2=true;                                             //kevin 20220225 read MCU DATA
                }
            }

            if(MotorStatus==true)
                break;

            if(WaitManualStepKey())
            {
                SW[SwRKManualTStart].Off();
                DriverDelay.SetMSAndOn(4000);
                labDelayStatus->Caption="";
                Task=2000;
            }
            break;
        case 1100:
            if(DoSetupTest(1))
            {
                Task=1000;
                break;
            }

            if(WaitManualStepKey())
            {
                if(ATC_InterfaceForm->iATC_MODE_TYPE==ATC_TYPE_70 &&
                   Temperature.bATC7TSDFunction==true)                          //Steven 20161201 : by site TSD for Contact Test
                {
                    iSiteOn[0]=0;
                    iSiteOn[1]=0;
                    iSiteOn[2]=0;
                    iSiteOn[3]=0;
                    { (void)iSiteOn; } /*GATE(W906-E042-ATC) ATC_InterfaceForm->UseTSD_Function(4, iSiteOn); -- AI(W906-E042): ATC_InterfaceForm is TATC_InterfaceFormShim (acarry_shims.h:115, iATC_MODE_TYPE only); the real TATC_InterfaceForm (forms/fATCHandlerSide.h) has no global -- same gate as atester.cpp R04 / T04 (W906-GB-P2b); HT9050 USE_ATC_MODE=5 = no ATC*/
                }

                DriverDelay.SetMSAndOn(4000);

                SW[SwRKManualTStart].Off();
                IsTest=false;                                                   //Steven 20120505 Start: 離開時,要通知GPIB重置測試狀態
                TestSocket.ClearAll();
                if(TestIF.iTestType==TTL_MODE && (TTL_CARD_TYPE<2))
                {
                    DoSetupTest();                                              //Steven 20230731 : Contact Test結束要清除TTL訊號
                }
                else if(TestIF.iGpibMode==InterfaceType_SPEA_Type)
                {
                    bool flag[32]={false};
                    if(BAR_CODE_INSTALL!=ebctUninstall && TestIF_File.bEnableBarCode)
                        _RunTestProgram_BarMess(sizeof(flag), flag, 0, "");
                    else
                        _RunTestProgram(sizeof(flag), flag);
                }
                else
                {
                    fMain->RunTestProgram(false);
                }

                labDelayStatus->Caption="";
                Task=2000;
            }
            break;
        case 2000:
            if(DriverDelay.Off())
            {
                Task=2050;
                bLampManualSetp=false;
                bLampManualStart=false;
                SW[SwManualZ2].Off();
                MOT[MTestZ2].iGali_SingalHomeTask=1;
            }
            break;
        case 2050:
            if(ScanIndexMotorCanMove() ==false)
                return false;
            if(MOT[MTestZ2].Gali_SingalHome())
            {
                Offset.iIndexArmContact[1]=0;
                Offset_File.iIndexArmContact[1]=0;                              //Steven 20150512 : Offset --> Offset_File

                Pos=-MOT[MTestZ2].Motor->LastHomePos;

                if(Pos>SafeTestZContactHeight)
                {
                    ShowMyMessage("Index Z2 test height too high error", "警告，Index Z2的測試高度太高!!", "Do_Z2_AutoGetHeight 2050");
                    Task=950;                                                   //Steven 20100225
                    break;
                }
                edContactHeight2->Text=ConvertTouMType(Pos);
                Task=2060;
            }
            break;
        case 2060:
            InitWriteAndCheckMotorTorqueTask();
            Task=2070;
        case 2070:
            if(USE_IO_CHANGE_TOQUE==true)                                       //jou 2012-06-21 Enable index I/O Change Toque
            {
                SW[SwIndexChangeToque1].Off();
                SW[SwIndexChangeToque2].Off();
            }

            ret=COM2->iWriteAndCheckMotorTorque(1, 300);
            if(ret==1)
                Task=2100;
            else if(ret==2)
            {
                ShowMyMessage("Index Z2 Motor torque set error", "Z2馬達扭力設定錯誤!", "Do_Z2_AutoGetHeight 2070");
                fAllMotorHome=false;
                CarlibrationTask=1;
                return false;
            }
            break;
        case 2100:
            fMain->chkReadTorque1->Checked=false;
            fMain->chkReadTorque2->Checked=false;
            if(iContactMode==CONTACT_AUTO_GET_HEIGHT ||
               iContactMode==CONTACT_TEST)                                                                              // auto K, must return 0 and check encoder
            {
                if(MOT[MTestZ2].Gali_MotMove(0, GotIndexZSpeed(iSpeedZ*1000)))                                          //JimmyChiu 20211028 : All speed can set by speed setting.
                {
                    if(IniConfig.bD58UseArm1PickPlaceArm2Test==true &&                                                  //JerryYang 20250515
                       TestIF_File.bArm1PickPlaceArm2Test==true)
                    {
                        if(IniConfig.bC08_SocketSensor &&
                           TestIF_File.bEnSocketSensor)
                        {
                            bflag=false;
                            str="";
                            for(int i=0; i<TestIF_File.iSocketCount; i++)
                            {
                                if(TestIF_File.iSensorCheckType[i]==2 &&                                                //Steven 20200420 : Socket Sensor功能可以選
                                   Sen[SThreadPara.iSocketSensor[i]].IsOn())
                                {
                                    bflag=true;
                                    str+=IntToStr(i+1)+",";
                                }
                            }

                            if(bflag)
                            {
                                ShowErrorMessage("WAR0323", K_RETRY, MTestZ1, false, str);
                                break;
                            }
                        }
                    }
                    palArm2Height->Caption=0;
                    Task=2200;                                                                                          //Steven 20131128 : 升上來後才檢查IC掉料
                }
            }
            else
            {
                if(MOT[MTestZ2].Gali_MotMoveSkipEncoder(0, GotIndexZSpeed(iSpeedZ*1000)))                               //JimmyChiu 20211028 : All speed can set by speed setting.
                {
                    Task=2200;                                                                                          //Steven 20131128 : 升上來後才檢查IC掉料
                }
            }
            break;
        case 2200:
            bHasErr|=CheckIndexAllSuckICFallDown(false, true);
            for(int i=0; i<BTestSuck.iShtRow; i++)
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
                for(int i=0; i<BTestSuck.iShtRow; i++)
                {
                    for(int j=0; j<BTestSuck.iShtCol; j++)
                    {
                        if(BTestSuck.Suck[i][j].Error ||
                           (BTestSuck.Item[i][j]!=HAS_NULL_IC &&
                            BTestSuck.Item[i][j]!=NULL_IC &&
                            BTestSuck.Suck[i][j].GetStatus()==false))           //有用到且有吸到IC的卻掉了
                        {
                            BTestSuck.SetItemData(i, j, HAS_NULL_IC);           //Steven 20110829 : 把有IC掉料的位置改成Has Null IC
                            BTestSuck.Suck[i][j].Normal();                      //Steven 20110829 : 把真空關掉
                            bCheckICFall[i][j]=false;
                        }
                    }
                }
                break;
            }

            palArm2Height->Caption=0;
            if(!bContinueContact)
            {
                for(int i=0; i<BTestSuck.iShtRow; i++)
                    for(int j=0; j<BTestSuck.iShtCol; j++)
                        bCheckICFall[i][j]=false;                               //恢復原狀
            }

            if(bContinueContact)
            {
                Task=1;
                return false;
            }
            return true;
        case 2800:
            if(Do_Z2_AutoGetHeightDelay.Off())                                                                          //JerryYang 20240111 : add
            {
                ADAM_Rang(IniConfig.iD26EPEncoderRange);
                if(ADAM_Alarm()==true)
                {
                    ret=ShowErrorMessage("WAR1605", K_RETRY|K_SKIP, 0, MMSystem, "TfContact::Do_Z2_AutoGetHeight");     //"請檢查EP是否漏氣!"
                    if(ret==K_RETRY)
                    {
                        return false;
                    }
                    else
                    {
                        Task=2900;
                    }
                }
                else
                {
                    Task=2900;
                }
            }
            break;
        case 2900:                                                                                                      //移動Arm 2到Socket
            if(MOT[MTestZ2].Gali_ReadPos()>(Prod.TestZ2_Test-Prod.TestZ2_Drop_Offset+750))                              //kevin 20140612 add start
            {
                flag=false;
                if(LastSet.iRealDummy==REALLY)
                {
                    for(int i=0; i<BTestSuck.iShtRow; i++)
                    {
                        for(int j=0; j<BTestSuck.iShtCol; j++)
                        {
                            if(BTestSuck.Suck[i][j].Enable       &&                                                     //Steven 20110725 : 不再使用IsSuckICFallDown
                               BTestSuck.Suck[i][j].SenUsing!="" &&
                               BTestSuck.Item[i][j]!=HAS_NULL_IC &&
                               BTestSuck.Item[i][j]!=NULL_IC)
                            {
                                if(BTestSuck.Suck[i][j].GetStatus()==false)
                                {
                                    BTestSuck.Suck[i][j].Normal();                                                      //jou 2012-01-17 直接關掉，避免掉到shuttle去，也避免要掉不掉Hang up
                                    flag=true;
                                    bRecIndexDropAlarm2=true;                                                           //jou 2012-01-17 紀錄index Drop alarm
                                }
                            }
                        }
                    }
                }

                if(flag)
                {
                    MOT[MTestZ1].Gali_Command("ST", __FUNC__);
                    Task=2904;
                    return false;
                }
            }

            if(DeviceForm_File.dKitDiameter==8 ||                                                                       //Ifor 20200318 : add 鋼徑80mmn 因為機台Hold不住變形需補下壓高度
               DeviceForm_File.dKitDiameter==40.2)
            {
                TestZ_CompensationHight();
            }

            if(cbContactMode->ItemIndex==DirectContactMode)                                                             //Direct Test Mode
            {
                Pos=Get0_01MMType(edContactHeight2->Text.c_str());
                if(CosFunction.bContactShowOffset ||                                                                    //Steven 20140409 : 矽品要求Contact畫面顯示Offset     //wei 20160603 驗證用
                   IniConfig.bChangeKitNoHardStop)                                                                      //2014-03-14    Dell
                {
                    Pos=Pos+Get0_01MMType(edContactOffsetArm2->Text.c_str());
                }

                if(DeviceForm_File.dKitDiameter==8 ||
                   DeviceForm_File.dKitDiameter==40.2)
                {
                    Pos=Pos-iTotalOffset_2;
                }

                if(MOT[MTestZ2].Gali_MotMove(Pos, GotIndexZSpeed(iSpeedZ*1000)))                                        //JimmyChiu 20211028 : All speed can set by speed setting.
                {
                    palArm2Height->Caption=IntToStr(Pos);
                    COM2->InitReadTorueTask();
                    fMain->chkReadTorque1->Checked=false;
                    fMain->chkReadTorque2->Checked=true;
                    fMain->edTorue1->Text="";
                    bReadMCU2=true;                                                                                     //kevin 20220225 read MCU DATA
                    Task=3000;
                }
            }
            else                                                                                                        //Drop Test Mode
            {
                if(CosFunction.bFixedDropSpeed)                                                                         //Steven 20131101 : 使用固定的Drop速度
                {
                    fDropPos=atof(edContactHeight2->Text.c_str())+(double)CosFunction.dLimitMinDropOffset;
                }
                else
                {
                    fDropPos=atof(edContactHeight2->Text.c_str())+atof(edDropOffset2->Text.c_str());
                }
                Pos=Get0_01MMType(FloatToStr(fDropPos).c_str());
                if(IniConfig.bChangeKitNoHardStop ||
                   CosFunction.bContactShowOffset)                                                                      //JerryYang 20171201 (Steven) 修正 Contact height offset無效的問題
                {
                    Pos=Pos+Get0_01MMType(edContactOffsetArm2->Text.c_str());
                }

                if(DeviceForm_File.dKitDiameter==8 ||
                   DeviceForm_File.dKitDiameter==40.2)
                {
                    Pos=Pos-iTotalOffset_2;
                }

                if(MOT[MTestZ2].Gali_MotMove(Pos, GotIndexZSpeed(iSpeedZ*1000)))                                        //JimmyChiu 20211028 : All speed can set by speed setting.
                {
                    palArm2Height->Caption=IntToStr(Pos);
                    COM2->InitReadTorueTask();
                    fMain->chkReadTorque1->Checked=false;
                    fMain->chkReadTorque2->Checked=true;
                    fMain->edTorue1->Text="";
                    bReadMCU2=true;                                                                                     //kevin 20220225 read MCU DATA
                    if(cbContactMode->ItemIndex==DropContact ||
                       cbContactMode->ItemIndex==DropContactModeDiffentSpeed ||
                       cbContactMode->ItemIndex==TMoveDrop ||
                       cbContactMode->ItemIndex==TMoveDropSlowContact ||
                       cbContactMode->ItemIndex==DropPlaceShiftContact)                                                 //ChungHung 20150528 add for 海思 _8Site1x4
                        Task=2910;
                    else
                        Task=2930;
                }
            }
            break;
        case 2904:                                                              //kevin 20140612  上升方便取料
            if(MOT[MTestZ2].Gali_MotMove(-1000, GotIndexZSpeed(iSpeedZ*1000)))  //JimmyChiu 20211028 : All speed can set by speed setting.
            {
                Task=2905;
            }
            break;
        case 2905:
            bHasErr|=CheckIndexAllSuckICFallDown(false, true);
            for(int i=0; i<BTestSuck.iShtRow; i++)
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
                for(int i=0; i<BTestSuck.iShtRow; i++)
                {
                    for(int j=0; j<BTestSuck.iShtCol; j++)
                    {
                        if(BTestSuck.Suck[i][j].Error ||
                           (BTestSuck.Item[i][j]!=HAS_NULL_IC &&
                            BTestSuck.Item[i][j]!=NULL_IC &&
                            BTestSuck.Suck[i][j].GetStatus()==false))           //有用到且有吸到IC的卻掉了
                        {
                            BTestSuck.SetItemData(i, j, HAS_NULL_IC);           //Steven 20110829 : 把有IC掉料的位置改成Has Null IC
                            BTestSuck.Suck[i][j].Normal();                      //Steven 20110829 : 把真空關掉
                            bCheckICFall[i][j]=false;
                        }
                    }
                }
                break;
            }

            if(bIsTestSitICFallDown)
            {
                for(int i=0; i<BTestSuck.iShtRow; i++)
                    for(int j=0; j<BTestSuck.iShtCol; j++)
                        bCheckICFall[i][j]=false;                               //恢復原狀
            }

            if(bIsTestSitICFallDown==false)
            {
                Task=2900;
                return false;
            }
            break;
        case 2910:                                                              //丟測,放下IC
            for(int i=0; i<BTestSuck.iShtRow; i++)
            {
                for(int j=0; j<BTestSuck.iShtCol; j++)
                {
                    if(INDEX_SUCKER_TYPE==1)                                    //jou 2015-07-31 修正 contect test drop mode arm1 & 2 不一樣寫法
                    {
                        fiosetview->bIndexDestroy[1][i][j]=true;
                        bIndexCheckNoStopVaccum=true;
                    }
                    else
                    {
                        BTestSuck.Suck[i][j].Off();
                    }
                }
            }
            Do_Z2_AutoGetHeightDelay.SetSecAndOn(Prod.TestZ_Drop_Wait);         // delay 0.3 sec for ic down      //Steven 20140909 : 換到迴圈外面
            Task=2920;
        case 2920:                                                                                                      //等待丟IC的Delay
            if(INDEX_SUCKER_TYPE==1)                                                                                    //jou 2015-07-31 修正 contect test drop mode arm1 & 2 不一樣寫法
            {
                bCheckDestroy=fiosetview->ProcessIndexSuckDestroy2();
            }
            else
            {
                bCheckDestroy=true;
            }

            if(bCheckDestroy==true && Do_Z2_AutoGetHeightDelay.Off())
            {
                if(CosFunction.bTesterSidePushFunction==true  &&                                                        //Richard 20220321 : 渠梁Side Push
                   DeviceForm.bTesterSidePush         ==true  &&
                   iTestSidePushcount                 ==0   )
                {
                    if(Cylinder[C_TesterSidePush].Enable==true)
                    {
                        Do_Z2_AutoGetHeightDelay.SetSecAndOn(DeviceForm_File.dSitePushWaitTime);                        //Richard 20220321 : 渠梁Side Push
                        Task=5900;
                        iTestSidePushcount++;
                        break;
                    }
                }

                for(int i=0; i<BTestSuck.iShtRow; i++)
                    for(int j=0; j<BTestSuck.iShtCol; j++)
                        BTestSuck.Suck[i][j].Normal();

                if(cbContactMode->ItemIndex==DropPlaceShiftContact )                                                    //ChungHung 20150528 add for 海思 _8Site1x4 in Contact
                    Task=2921;
                else
                    Task=2930;
            }
            iTestSidePushcount=0;
            break;
        case 2921:                                                                                                      //ChungHung 20150528 add for 海思 _8Site1x4 in Contact
            if(MOT[MTestZ2].Gali_MotMove(Prod.TestZ1_Safe, GotIndexZSpeed(iSpeedZ*1000)))                               //Z2 上升至安全位置   //JimmyChiu 20211028 : All speed can set by speed setting.
            {
                Task=2922;
            }
            break;
        case 2923:                                                                                                                                              //ChungHung 20150528 add for 海思 _8Site1x4 in Contact
            if(MOT[MTestY1].GalilTwoY_Move(Prod.TestY2_Middle-TestIF.dSiteYPitch, Prod.TestY1_Front, GotIndexYSpeed(iSpeed*3000), "Do_Z2_AutoGetHeight 2923"))  //JimmyChiu 20211028 : All speed can set by speed setting.
            {
                Task=2930;
            }
            break;
        case 2930:                                                                                                                                              //移動Arm 2 到測區
            if(CosFunction.bTesterSidePushFunction==true && DeviceForm.bTesterSidePush==true && Do_Z2_AutoGetHeightDelay.Off()==false)                          //Richard 20220321 : 渠梁Side Push
            {
                break;
            }
            Pos=Get0_01MMType(edContactHeight2->Text.c_str());
            if(IniConfig.bChangeKitNoHardStop ||
               CosFunction.bContactShowOffset)                                                                                                                  //JerryYang 20171201 (Steven) 修正 Contact height offset無效的問題
            {
                Pos=Pos+Get0_01MMType(edContactOffsetArm2->Text.c_str());
            }

            if(iContactMode==CONTACT_TEST &&                                                                                                                    //kevin 20130608 馬達到定點 ep 再充氣
               (cbContactMode->ItemIndex==DirectContactSoftEP ||
                cbContactMode->ItemIndex==DropContactSoftEP))
            {
                if(MOT[MTestZ2].Gali_ReadPos()<(Pos+1500))                                                                                                      //JerryYang 20231205 : SoftEP高度由+1000改為+1500避免撞到guide pin
                    EPSwitchOnOff(eEPSwBoth);
//                    bContSoftEpSwitch(1, true);                                 //ARM2 浮動頭充氣       //JerryYang 20151202 flase->true
            }

            if(DeviceForm_File.dKitDiameter==8 ||                                                                                                               //Ifor 20230214 add: Contact 頁面 缸徑80 不同Contact Mode 高度補償
               DeviceForm_File.dKitDiameter==40.2)
            {
                TestZ_CompensationHight();
                Pos=Pos-iTotalOffset_2;
            }

            if(MOT[MTestZ2].Gali_MotMove(Pos, GotIndexZSpeed(iSpeedZ*1000)))                                                                                    //JimmyChiu 20211028 : All speed can set by speed setting.
            {
                palArm2Height->Caption=IntToStr(Pos);
                COM2->InitReadTorueTask();
                fMain->chkReadTorque1->Checked=false;
                fMain->chkReadTorque2->Checked=true;
                fMain->edTorue1->Text="";
                bReadMCU2=true;                                                                                                                                 //kevin 20220225 read MCU DATA
                if(TestIF.bUseSLKClamp==true &&
                   TestIF_File.iSeparabilityTest==1)                                                                                                            //JerryYang 20160616 分離流程
                {
                    Task=2952;
                }
                else if((cbContactMode->ItemIndex==DropContact ||
                         cbContactMode->ItemIndex==DropContactModeDiffentSpeed ||
                         cbContactMode->ItemIndex==TMoveDrop   ||                                                                                               //Steven 20160530
                         cbContactMode->ItemIndex==TMoveDropSlowContact) &&
                         cbVacuumMode->ItemIndex==VacuumONMode)                                                                                                 //kevin 20110824 丟測掉料 DirectContactMode
                {
                    Task=2940;
                }
                else
                {
                    Task=3000;
                }
            }
            break;
        case 2940:                                                              // Vacumm On Mode
            for(int i=0; i<BTestSuck.iShtRow; i++)
            {
                for(int j=0; j<BTestSuck.iShtCol; j++)
                {
                    if(bCheckICFall[i][j]==true)                                //有用到的
                        BTestSuck.Suck[i][j].On();
                }
            }

            Do_Z2_AutoGetHeightDelay.SetMSAndOn(100);                           // delay 0.1 sec for ic down
            Task=2950;
            break;
        case 2950:                                                              //等待Vacumm的Delay
            if(Do_Z2_AutoGetHeightDelay.Off())
            {
                Task=3000;
            }
            break;
        case 2952:                                                              //JerryYang 20160621 分離流程START
            Cylinder[C_Socket_Unclamp].Off();
            Cylinder[C_Socket_Clamp].On();                                      //Z2已經下降到測試高度,Socket clamp夾持
            Z2ClampCloseDelay.SetMSAndOn(300);
            Z2ClampTimeOutDelay.SetSec(2.5);
            Z2ClampTimeOutDelay.On();
            Task=2953;
            break;
        case 2953:
            if(Z2ClampCloseDelay.Off())
            {
                #ifndef SOFT_SIMULTE
                if(Sen[SnSocketClampPush1].IsOn()==true && Sen[SnSocketClampPush2].IsOn()==true &&
                   Sen[SnSocketHasClamp1].IsOn()==true && Sen[SnSocketHasClamp2].IsOn()==true)
                {
                    for(int i=0; i<BTestSuck.iShtRow; i++)                      //破真空將IC放到SOCKET上
                    {
                        for(int j=0; j<BTestSuck.iShtCol; j++)
                        {
                            BTestSuck.Suck[i][j].Normal();
                        }
                    }
                    Task=2954;
                }
                else
                {
                    if(Z2ClampTimeOutDelay.Off())
                    {
                        ret=ShowErrorMessage("JAM0375", K_RETRY, MMIndex);      //socket clamp異常
                        if(ret==K_RETRY)
                        {
                            Task=2952;
                        }
                    }
                }
                #else
                    Task=2954;
                #endif
            }
            break;
        case 2954:                                                              // Socket clamp夾持後,SLK2 放開clamp
            Cylinder[C_SLK2_Clamp].Off();
            Cylinder[C_SLK2_Unclamp].On();
            Z2ClampCloseDelay.SetMSAndOn(500);
            Z2ClampTimeOutDelay.SetSec(2.5);
            Z2ClampTimeOutDelay.On();
            Task=2955;
            break;
        case 2955:
            if(Z2ClampCloseDelay.Off())
            {
               #ifndef SOFT_SIMULTE
                if(Cylinder[C_SLK2_Unclamp].OnStatus())
                  Task=2956;
                if(Z2ClampTimeOutDelay.Off())
                {
                    ret=ShowErrorMessage("JAM0374", K_RETRY, MTestY2);          //ARM2 SLK放開clamp異常
                    if(ret==K_RETRY)
                    {
                        Task=2954;
                    }
                }
                #else
                    Task=2956;
                #endif
            }
            break;
        case 2956:                                                                                                      //Arm 2上升5mm再測試
            Pos=Get0_01MMType(edContactHeight2->Text.c_str());
            if(MOT[MTestZ2].Gali_MotMove(Pos+1500, GotIndexZSpeed(iSpeedZ*1000)))                                       //JimmyChiu 20211028 : All speed can set by speed setting.
                Task=3010;
            break;                                                                                                      //JerryYang 20160621 分離流程END
        case 3000:
            if(ATC_InterfaceForm->iATC_MODE_TYPE==ATC_TYPE_70 &&
               Temperature.bATC7TSDFunction==true)                                                                                                              //Steven 20161201 : by site TSD for Contact Test
            {
                iSiteOn[0]=0;
                iSiteOn[1]=0;
                iSiteOn[2]=(BTestSuck.Item[0][0]==HAS_IC || BTestSuck.Item[0][0]==HAS_HOT_IC)?1:0;
                iSiteOn[3]=(BTestSuck.Item[0][1]==HAS_IC || BTestSuck.Item[0][1]==HAS_HOT_IC)?1:0;
                { (void)iSiteOn; } /*GATE(W906-E042-ATC) ATC_InterfaceForm->UseTSD_Function(4, iSiteOn); -- AI(W906-E042): ATC_InterfaceForm is TATC_InterfaceFormShim (acarry_shims.h:115, iATC_MODE_TYPE only); the real TATC_InterfaceForm (forms/fATCHandlerSide.h) has no global -- same gate as atester.cpp R04 / T04 (W906-GB-P2b); HT9050 USE_ATC_MODE=5 = no ATC*/
            }

            if(ATC_SYSTEM==eNewATCSystem &&                                                                                                                     //Ifor 20190321 : add Test Mode & ATC Enables Site Send to GPIB
               Temperature.bATCActiveCooling==true &&
               TestIF_File.i2DIDFormat==eAMD)                                                                                                                   //JerryYang 20200422 2DID format選項改用下拉選單
            {
                for(int i=0; i<MAX_SOCKET_TOTAL; i++)
                {
                    HHandler2Gpib.Site[i]=bATC_EnablesChannel[i];
                }
            }
            HHandler2Gpib.iLotStatus=TestIF_File.iTestMode;

            fMain->SendMSG_CMD(MSG_CMD_Arm2Down);                                                                                                               //Ifor 20180622 (wei) : add ContactTestArm
            if(ATC_SYSTEM==eNewATCSystem &&                                                                                                                     //Ifor 20151230 :add New ATC Interface HandlerArm
               Temperature.bATCActiveCooling==true)                                                                                                             //JerryYang 20220815 : send ATC start testing
            {
                {} /*GATE(W906-E042-ATC) ATC_InterfaceForm->HandlerArm(1); -- AI(W906-E042): as above (the ATC 'which arm' notice)*/
            }
            iWhichArmDown=2;                                                                                                                                    //JerryYang 20200316 add SVID 哪支arm下壓在測區
            ATC_SwitchTjSignal(2);                                                                                                                              //Ifor 20210622 add: ATC Switch TJ
            if(cbContactMode->ItemIndex==DirectContactSoftEP ||
               cbContactMode->ItemIndex==DropContactSoftEP)
            {                                                                                                                                                   //kevin 20130608 馬達到定點 ep 在充氣
//                bContSoftEpSwitch(1, true);                                     //JerryYang 20240111 : add  //ARM2 浮動頭先充氣  //JerryYang 20151202 Arm2 false->true
                EPSwitchOnOff(eEPSwBoth);
                Do_Z2_AutoGetHeightDelay.SetMSAndOn(1000);                                                                                                      // delay 0.1 sec for ic down
                Task=3001;
            }
            else
            {
                if(CUSTOMER_CODE==CC_KYEC_CHEN &&                                                                                                               //jou 2015-08-31 新增Index Vacuum Off mode
                   cbVacuumMode->ItemIndex==VacuumOFFMode)
                {
                    for(int i=0; i<BTestSuck.iShtRow; i++)
                    {
                        for(int j=0; j<BTestSuck.iShtCol; j++)
                        {
                            BTestSuck.Suck[i][j].Normal();
                        }
                    }
                }

                if(CosFunction.bTesterSidePushFunction==true && DeviceForm.bTesterSidePush==true && DeviceForm_File.iSidePushMode==0)                           //Richard 20230417 : 渠梁Side Push fix延遲
                    Task=3002;
                else
                    Task=3010;
            }

            iGPIBIndexStatus=Z1Up_Z2Down;
            break;
        case 3001:                                                              //等待Vacumm的Delay
            if(Do_Z2_AutoGetHeightDelay.Off())
            {
                Task=3010;
            }
            break;
        case 3002:                                                              //Richard 20230417 : 渠梁Side Push fix延遲
            if(CosFunction.bTesterSidePushFunction==true && DeviceForm.bTesterSidePush==true)
            {
                if(Cylinder[C_TesterSidePush].Enable==true)
                {
                    Task=6100;
                    Do_Z2_AutoGetHeightDelay.SetSecAndOn(DeviceForm_File.dSitePushWaitTime);
                    break;
                }
                else                                                            //JerryYang 20250228 : fix hang up
                {
                    Task=3010;
                    break;
                }
            }
        case 3010:                                                                                                      //Ifor 20170105 (Steven) add Index 到位後顯示 Index Arm Jog Move 工具
            if(CUSTOMER_CODE==CC_KYEC_LEE ||
               CUSTOMER_CODE==CC_HONPREC_QC)
            {
                edContactOffsetArm2->Enabled=false;                                                                     //Alick 20170116 add 當JOG視窗顯示時，對應Z的輸入框不可KEYIN
                plIndexArmJogMove->Visible=true;
                iIndexStatus=Z1Up_Z2Down;
                if(bContactJogFlag==true)                                                                               //Ifor 20170125 (Steven) add 避免尚未壓下Jog Move 按鈕 iContactJogMove=0 會跑回原點
                {
                    if(MOT[MTestZ2].Gali_MotMoveNoWait(iContactJogMove, GotIndexZSpeed(iSpeedZ*1000), 0))               //Alick 20170116 add   //JimmyChiu 20211028 : All speed can set by speed setting.
                    {
                        bContactJogFlag=false;
                        btnIndexArmJogMove_Up->Enabled=true;
                        btnIndexArmJogMove_Down->Enabled=true;
                    }
                }
            }

            if(iContactMode==CONTACT_DEVICE_MAP_CHECK ||                                                                //Steven 20191009 : Device Map for Qualcomm
               WaitManualStartKey())
            {
                fMain->SendMSG_CMD(MSG_CMD_ContactTestArm2);                                                            //Steven 20150304 : Add GPIB LOG
                {} /*GATE(W906-E042-UI) fTestCategory->BringToFront(); -- AI(W906-E042): window z-order only; TfTestCategory has no base class / BringToFront (forms/fTestCategory.h:257 D-2)*/                                                                          //Steven 20100823
                if(IniConfig.bC04EnableTestTempIC)
                    {} /*GATE(W906-E042-UI) fDynamicTemp->BringToFront(); -- AI(W906-E042): window z-order only; fDynamicTemp has no global (forms/fContact.h:458-461 X-13)*/                                                                       //Steven 20120810
                iSetupTask=1;
                IsTest=true;                                                                                            //Steven 20110920
                Task=3100;
                labDelayStatus->Caption="Waiting Test Result";
                break;
            }

            if(WaitManualStepKey())
            {
                Task=3110;
                if(((DeviceForm.iSocketInitialICCheckPosition==1 && IniConfig.bTestIcCheckInContact==true) ||           //JerryYang 20250120 : add
                   (LastSet.iD41SocketInitialICCheckPosition==1 && IniConfig.bTestIcCheckInContact==false)) &&
                    cbCalibrateAboveHeight->Checked)                                                                    //Above Socket
                {
                    Task=3105;
                    DoCalibrateAboveHeightZ2(true);
                }

                if(ATC_SYSTEM==eNewATCSystem)                                                                           //Ifor 20151230 :add New ATC Interface HandlerArm
                {
                    {} /*GATE(W906-E042-ATC) ATC_InterfaceForm->HandlerArm(-1); -- AI(W906-E042): as above (the ATC 'which arm' notice)*/                                                                  //Ifor 20220107 add:通知ATC目前哪隻Arm再Socket
                }
            }

            if(fMain->edTorue1->Text!="")
            {
                COM2->InitReadTorueTask();
                fMain->chkReadTorque1->Checked=false;
                fMain->chkReadTorque2->Checked=true;
                fMain->edTorue1->Text="";
                bReadMCU2=true;                                                                                         //kevin 20220225 read MCU DATA
            }
            break;
        case 3105:
            if(DoCalibrateAboveHeightZ2()==true)                                //JerryYang 20250120 : add
            {
                Memo1->Lines->Add("Z2 Above Socket height calibration is finished!");
                Task=3110;
            }
            break;
        case 3100:
            if(DoSetupTest(1))
            {
                if(iContactMode==CONTACT_DEVICE_MAP_CHECK)                      //Steven 20191009 : Device Map for Qualcomm
                    Task=3110;
                else
                    Task=3000;

                if(iContactMode==STEP_CONTACT_TEST && chkDailyCorrelation->Checked)
                    Task=3110;
                break;
            }

            if(WaitManualStepKey())
            {
                Task=3110;
            }
            break;
        case 3110:
            if(CUSTOMER_CODE==CC_KYEC_LEE)                                                                                                                      //Ifor 20170105 (Steven) add Index 到位後顯示 Index Arm Jog Move 工具
            {
                plIndexArmJogMove->Visible=false;
                edContactOffsetArm2->Enabled=true;                                                                                                              //Ifor 20170125 (Steven) add 按下Step要解除對應Z的輸入框鎖定
            }

            if(ATC_InterfaceForm->iATC_MODE_TYPE==ATC_TYPE_70 &&
               Temperature.bATC7TSDFunction==true)                                                                                                              //Steven 20161201 : by site TSD for Contact Test
            {
                iSiteOn[0]=0;
                iSiteOn[1]=0;
                iSiteOn[2]=0;
                iSiteOn[3]=0;
                { (void)iSiteOn; } /*GATE(W906-E042-ATC) ATC_InterfaceForm->UseTSD_Function(4, iSiteOn); -- AI(W906-E042): ATC_InterfaceForm is TATC_InterfaceFormShim (acarry_shims.h:115, iATC_MODE_TYPE only); the real TATC_InterfaceForm (forms/fATCHandlerSide.h) has no global -- same gate as atester.cpp R04 / T04 (W906-GB-P2b); HT9050 USE_ATC_MODE=5 = no ATC*/
            }

            iTriggerBoostFunction=-1;
            iTriggerBoostFuncBack=-1;
            iBoostFuncStep=5;
            SW[SwRKManualTStart].Off();                                                                                                                         //Steven 20120505 Start: 離開時,要通知GPIB重置測試狀態
            IsTest=false;
            TestSocket.ClearAll();
            if(TestIF.iTestType==TTL_MODE && (TTL_CARD_TYPE<2))
            {
                DoSetupTest();                                                                                                                                  //Steven 20230731 : Contact Test結束要清除TTL訊號
            }
            else if(TestIF.iGpibMode==InterfaceType_SPEA_Type)
            {
                bool flag[32]={false};
                if(BAR_CODE_INSTALL!=ebctUninstall &&
                   TestIF_File.bEnableBarCode)
                    _RunTestProgram_BarMess(sizeof(flag), flag, 0, "");
                else
                    _RunTestProgram(sizeof(flag), flag);
            }
            else
            {
                fMain->RunTestProgram(false);                                                                                                                   //Steven 20120505 End : 離開時, 要通知GPIB重置測試狀態
            }

            fMain->SendMSG_CMD(MSG_CMD_ContactTestAbort);                                                                                                       //Steven 20150304 : Add GPIB LOG
            if(cbContactMode->ItemIndex==DropPlaceShiftContact)                                                                                                 //ChungHung 20150528 add for 海思 _8Site1x4
            {
                Task=3150;
            }
            else if(TestIF_File.bUseSLKClamp &&
                    TestIF_File.iSeparabilityTest==1)                                                                                                           //JerryYang 20160617 夾起來
            {
                Task=3180;
            }
            else if(CosFunction.bTesterSidePushFunction==true && DeviceForm.bTesterSidePush==true && DeviceForm_File.iSidePushMode==1)                          //Richard 20220321 : 渠梁Side Push
            {
                Task=6200;
                break;
            }
            else
            {
                Task=3200;
            }
            labDelayStatus->Caption="";
            break;
        case 3150:                                                                                                      //ChungHung 20150528 add for 海思 _8Site1x4
            if(MOT[MTestZ2].Gali_MotMove(Prod.TestZ1_Safe, GotIndexZSpeed(iSpeedZ*1000)))                               //Z1 上升至安全位置   //JimmyChiu 20211028 : All speed can set by speed setting.
            {
                Task=3160;
            }
            break;
        case 3160:                                                                                                                                              //ChungHung 20150528 add for 海思 _8Site1x4
            if(MOT[MTestY1].GalilTwoY_Move(Prod.TestY2_Middle-TestIF.dSiteYPitch, Prod.TestY1_Front, GotIndexYSpeed(iSpeed*3000), "Do_Z2_AutoGetHeight 3160"))  //JimmyChiu 20211028 : All speed can set by speed setting.
            {
                Task=3170;
            }
            break;
        case 3170:                                                              //ChungHung 20150528 add for 海思 _8Site1x4
            Pos=Get0_01MMType(edContactHeight2->Text.c_str());
            if(MOT[MTestZ2].Gali_MotMove(Pos, GotIndexZSpeed(iSpeedZ*1000)))    //JimmyChiu 20211028 : All speed can set by speed setting.
            {
                Task=3200;
            }
            break;
        case 3180:                                                              //JerryYang 20160621 分離機構流程 Start
            if(Cylinder[C_SLK2_Unclamp].OnSensor())
            {
                Task=3182;
            }
            break;
        case 3182:                                                                                                      //測試完,Arm2要下降
            if(MOT[MTestZ2].Gali_MotMove(Prod.TestZ2_Test, GotIndexZSpeed(iSpeedZ*1000)))                               //JimmyChiu 20211028 : All speed can set by speed setting.
            {
                Task=3184;
            }
            break;
        case 3184:                                                              //SLK Clamp 夾持
            {
                Cylinder[C_SLK2_Unclamp].Off();
                Cylinder[C_SLK2_Clamp].On();
                Z2ClampOpenDelay.SetMSAndOn(500);
                Z2ClampTimeOutDelay.SetSec(2.5);
                Z2ClampTimeOutDelay.On();
                Task=3186;
            }
            break;
        case 3186:
            if(Z2ClampOpenDelay.Off())
            {
                if(Cylinder[C_SLK2_Clamp].OnSensor())
                {
                    Task=3188;
                }
                else
                {
                    if(Z2ClampTimeOutDelay.Off())
                    {
                        ret=ShowErrorMessage("JAM0372", K_RETRY, MTestY2);      //Socket clamp未到位
                        if(ret==K_RETRY)
                        {
                            Task=3184;
                        }
                    }
                }
            }
            break;
        case 3188:
            Cylinder[C_Socket_Clamp].Off();                                     //Socket Clamp 不夾
            Cylinder[C_Socket_Unclamp].On();
            Z2ClampOpenDelay.SetMSAndOn(500);
            Z2ClampTimeOutDelay.SetSec(2.5);
            Z2ClampTimeOutDelay.On();
            Task=3190;
            break;
        case 3190:
            if(Z2ClampOpenDelay.Off())
            {
                #ifndef SOFT_SIMULTE
                if(Sen[SnSocketClampPull1].IsOn()==true && Sen[SnSocketClampPull2].IsOn()==true &&
                   Sen[SnSocketClampPush1].IsOff()==true && Sen[SnSocketClampPush2].IsOff()==true)
                {
                    Task=3200;
                }
                else
                {
                    if(Z2ClampTimeOutDelay.Off())
                    {
                        ret=ShowErrorMessage("JAM0376", K_RETRY, MMIndex);      //放開clamp異常
                        if(ret==K_RETRY)
                        {
                            Task=3188;
                        }
                    }
                }
                #else
                    Task=3200;
                #endif
            }
            break;                                                              //JerryYang 20160621 分離機構流程 End
        case 3200:
            for(int i=0; i<BTestSuck.iShtRow; i++)
            {
                for(int j=0; j<BTestSuck.iShtCol; j++)
                {
                    if(bCheckICFall[i][j]==true)                                //有用到的
                        BTestSuck.Suck[i][j].On();
                }
            }

            Do_Z2_AutoGetHeightDelay.SetSecAndOn(2);                            // delay 2 sec for ic down
            Task=3210;
        case 3210:
            if(Do_Z2_AutoGetHeightDelay.Off())
            {
                Task=3300;
            }
            break;
        case 3300:
            Pos=Get0_01MMType(edContactHeight2->Text.c_str());
            if(MOT[MTestZ2].Gali_MotMove(Pos+200, GotIndexZSpeed(iSpeedZ*100)))                                         //JimmyChiu 20211028 : All speed can set by speed setting.
            {
                Task=2100;
            }
            break;
        case 5900:
            if(Do_Z2_AutoGetHeightDelay.Off()==false)                           //Richard 20220321 : 渠梁Side Push
            {
                break;
            }

            if(CosFunction.bFixedDropSpeed)                                     //Steven 20131101 : 使用固定的Drop速度
            {
                fDropPos=atof(edContactHeight2->Text.c_str())+(double)CosFunction.dLimitMinDropOffset;
            }
            else
            {
                fDropPos=atof(edContactHeight2->Text.c_str())+atof(edDropOffset2->Text.c_str());
            }
            Pos=Get0_01MMType(FloatToStr(fDropPos).c_str());

            if(IniConfig.bChangeKitNoHardStop ||
               CosFunction.bContactShowOffset)                                  //JerryYang 20171201 (Steven) 修正 Contact height offset無效的問題
            {
                Pos=Pos+Get0_01MMType(edContactOffsetArm2->Text.c_str());
            }

            if(MOT[MTestZ2].Gali_MotMove(Pos-100, GotIndexZSpeed(iSpeedZ*1000)))
            {
                Task=6000;
                Do_Z2_AutoGetHeightDelay.SetSecAndOn(DeviceForm_File.dSitePushWaitTime);
                break;
            }
            break;
        case 6000:
            if(Do_Z2_AutoGetHeightDelay.Off()==false)
            {
                break;
            }

            if(DoTesterSidePush(true)==1)
            {
                Do_Z2_AutoGetHeightDelay.SetSecAndOn(DeviceForm_File.dSitePushWaitTime);
                Task=6010;                                                      //Richard 20230301 : 渠梁Side Push 新模式更改6100->2920
            }
            break;
        case 6010:
            if(Do_Z2_AutoGetHeightDelay.Off()==false)
            {
                break;
            }
            Task=2920;
            break;
        case 6100:
            if(Do_Z2_AutoGetHeightDelay.Off()==false)
            {
                break;
            }

            if(DoTesterSidePush(false)==10)
            {
                Task=3010;                                                      //Richard 20230301 : 渠梁Side Push 新模式更改2920->3010
                Do_Z2_AutoGetHeightDelay.SetSecAndOn(DeviceForm_File.dSitePushWaitTime);
            }
            break;
        case 6200:
            if(DoTesterSidePush(false)==10)
            {
                Task=3200;                                                      //Richard 20230301 : 渠梁Side Push 新模式更改2920->3010
            }
            break;
    }
    return false;
}
// ===== end of the golden span (cContact.cpp:10510; AI(W906-E042) 20261005 B3: extended from :8168 by Do_Z2_AutoGetHeight :8169-10510, same offset) =====

//  AI(W906-E042) 20261005 (B3): golden cContact.cpp:102-103 file-scope globals (DoCalibrateAboveHeightZ1 / Z2 write them).
int iZ1_AboveOffset=0;                                                          //JerryYang 20250120 : add
int iZ2_AboveOffset=0;

// ===== AI(W906-E042) 20261005 (B3): golden 0618 cContact.cpp:22325-22606 (tCalibrateAboveHeightZ1Delay + DoCalibrateAboveHeightZ1 + DoCalibrateAboveHeightZ2), verbatim: CalibrateAboveZ1 span: port line = golden line + (-17091) =====
TQPF_Timer tCalibrateAboveHeightZ1Delay;
bool TfContact::DoCalibrateAboveHeightZ1(bool Reset)   //AI(W906-E042) 20261005 (B3): __fastcall dropped (declaration forms/fContact.h:1596 has none)                  //KaiChen 20200525 ：Daily Correlation Function
{
//    int i, j;
    bool bResult=false, flag2=false, flag=false;
    bool bZMotMove=false;

    int &Task=iTaskCaliAboveHeight;
    int Pos=0, iAboveHeightZ=20;
//    double dZoffset=0.0;
    static int iSiteCount=0, iCount=0, iZOffset=0;
    bool bIndexSuckCheck=false;
    AnsiString str="";

    if(Reset)
    {
        Task=1;
        iCount=0;
        iZOffset=0;
        return bResult;
    }

    if(W906_IndexZDriveFaultStop("DoCalibrateAboveHeightZ1")) { fAllMotorHome=false; CarlibrationTask=1; return false; }   switch(Task)   //AI(W906-E042) 20261005 (B3) P7 NOT GOLDEN: M14 drive alarm / ERROR_STOP / servo off / monitor lost or frozen / route fault -> ST + message + golden 536 exit; PCI1203 Z1 SHIP only, no-op elsewhere
    {
        case 1:

            #ifndef SOFT_SIMULTE
            if(FTestSuck.HasRealIC()==true)
            {
                bResult=false;
                return bResult;
            }
            #endif

            if(MOT[MTestZ1].Gali_Two_ZAxis_Move(Prod.TestZ1_Safe, GotIndexZSpeed(iSpeedZ*1000), "DoCalibrateAboveHeightZ1_1"))
            {
                Task=100;
                iCount=0;
                iZOffset=0;
            }
            break;
        case 100:
            if(MOT[MTestY1].GalilTwoY_Move(Prod.TestY1_Middle, Prod.TestY2_Rear, GotIndexYSpeed(iSpeed*3000), "DoCalibrateAboveHeightZ1_100"))
            {
                Task=200;
            }
            break;
        case 200:
            Pos=Get0_01MMType(edContactHeight1->Text.c_str());

            iZOffset=iCount*iAboveHeightZ;

            if(iZOffset>=2000)
            {
                ShowMyMessage("Above height is over limti : +1000 ");
                iCount=0;
                return false;
            }

            bZMotMove=MOT[MTestZ1].Gali_MotMove(Pos+iZOffset, GotIndexZSpeed(iSpeedZ*1000));                            //JimmyChiu 20211028 : All speed can set by speed setting.
            if(bZMotMove)
            {
                iCount++;
                Task=300;
                tCalibrateAboveHeightZ1Delay.SetSecAndOn(1.0);
            }
            break;
        case 300:                                                               //Index 1 至 Sock 吸料

            if(tCalibrateAboveHeightZ1Delay.Off()==false)
                break;

            if(INDEX_SUCKER_TYPE==1)                                            //Ifor 20200617 : add Use One By One Index Check Function 整合
            {
                FTestSuck.ResetAll();
                iSiteCount=0;
                IndexCheckOneByOne(true, 0, iSiteCount, false, true);           //Ifor 20200617 : add Use One By One Index Check Function 整合
                Task=400;
                break;
            }
            break;
        case 400:                                                               //Ifor 20200617 : add Use One By One Index Check Function 整合
            if(IndexCheckOneByOne(false, 0, iSiteCount, false, true))           //Ifor 20200617 : add Use One By One Index Check Function 整合
            {
                iSiteCount++;
                if(iSiteCount>=(FTestSuck.iMaxRow*FTestSuck.iMaxCol))
                {
                    Task=500;
                    tCalibrateAboveHeightZ1Delay.SetSecAndOn(1.0);
                }
            }
            break;
        case 500:
            //jou 2012-01-04 需確認Index suck已經完整做完
            if(INDEX_SUCKER_TYPE==1)
            {
                bIndexSuckCheck=false;
                bIndexSuckCheck=fiosetview->ProcessIndexSuckDestroy1();
            }
            else
            {
                bIndexSuckCheck=true;
            }

            if(tCalibrateAboveHeightZ1Delay.Off() && bIndexSuckCheck==true)
            {
                for(int i=0; i<FTestSuck.iMaxRow; i++)                          //jou 2011-07-01 start : 解決未完成auto high中途退出會一直出現need one cycle error
                {
                    for(int j=0; j<FTestSuck.iMaxCol; j++)
                    {
                        flag2=false;
                        FTestSuck.CheckVaccumIsON_AboveSocket(i, j, flag2);
                        if(flag2==true)
                        {
                            flag=true;
                            FTestSuck.Suck[j][i].Normal();
                        }
                    }
                }

                if(flag)                                                        //有真空, 還要繼續往上拉
                {
                    Task=200;
                }
                else                                                            //finish
                {
                    iZ1_AboveOffset=iZOffset;
                    str.sprintf("Z1 above height : %d", iZ1_AboveOffset);
                    Memo1->Lines->Add(str);
//                    dZoffset=iZOffset/100.0;
//                    edD41->Text=dZoffset;
                    return true;
                }
            }
    }
    return bResult;
}
//------------------------------------------------------------------------------
bool TfContact::DoCalibrateAboveHeightZ2(bool Reset)   //AI(W906-E042) 20261005 (B3): __fastcall dropped (declaration forms/fContact.h:1597 has none)                  //KaiChen 20200525 ：Daily Correlation Function
{
    bool bResult=false, flag2=false, flag=false;
    bool bZMotMove=false;

    int &Task=iTaskCaliAboveHeight;
    int Pos=0, iAboveHeightZ=20;
    double dZoffset=0.0;
    static int iSiteCount=0, iCount=0, iZOffset=0;
    bool bIndexSuckCheck=false;
    AnsiString str="";

    if(Reset)
    {
        Task=1;
        iCount=0;
        iZOffset=0;
        return bResult;
    }

    if(W906_IndexZDriveFaultStop("DoCalibrateAboveHeightZ2")) { fAllMotorHome=false; CarlibrationTask=1; return false; }   switch(Task)   //AI(W906-E042) 20261005 (B3) P7 NOT GOLDEN: M14 drive alarm / ERROR_STOP / servo off / monitor lost or frozen / route fault -> ST + message + golden 536 exit; PCI1203 Z1 SHIP only, no-op elsewhere
    {
        case 1:

            #ifndef SOFT_SIMULTE
            if(BTestSuck.HasRealIC()==true)
            {
                bResult=false;
                return bResult;
            }
            #endif

            if(MOT[MTestZ1].Gali_Two_ZAxis_Move(Prod.TestZ1_Safe, GotIndexZSpeed(iSpeedZ*1000), "DoCalibrateAboveHeightZ2_1"))
            {
                Task=100;
                iCount=0;
                iZOffset=0;
            }
            break;
        case 100:
            if(MOT[MTestY1].GalilTwoY_Move(Prod.TestY1_Front, Prod.TestY2_Middle, GotIndexYSpeed(iSpeed*3000), "DoCalibrateAboveHeightZ2_100"))
            {
                Task=200;
            }
            break;
        case 200:
            Pos=Get0_01MMType(edContactHeight2->Text.c_str());

            iZOffset=iCount*iAboveHeightZ;

            if(iZOffset>=2000)
            {
                ShowMyMessage("Above height is over limti : +2000 ");
                iCount=0;
                return false;
            }

            bZMotMove=MOT[MTestZ2].Gali_MotMove(Pos+iZOffset, GotIndexZSpeed(iSpeedZ*1000));                            //JimmyChiu 20211028 : All speed can set by speed setting.
            if(bZMotMove)
            {
                iCount++;
                Task=300;
                tCalibrateAboveHeightZ1Delay.SetSecAndOn(1.0);
            }
            break;
        case 300:                                                               //Index 1 至 Sock 吸料

            if(tCalibrateAboveHeightZ1Delay.Off()==false)
                break;

            if(INDEX_SUCKER_TYPE==1)                                            //Ifor 20200617 : add Use One By One Index Check Function 整合
            {
                BTestSuck.ResetAll();
                iSiteCount=0;
                IndexCheckOneByOne(true, 1, iSiteCount, false, true);           //Ifor 20200617 : add Use One By One Index Check Function 整合
                Task=400;
                break;
            }
            break;
        case 400:                                                               //Ifor 20200617 : add Use One By One Index Check Function 整合
            if(IndexCheckOneByOne(false, 1, iSiteCount, false, true))           //Ifor 20200617 : add Use One By One Index Check Function 整合
            {
                iSiteCount++;
                if(iSiteCount>=(FTestSuck.iMaxRow*FTestSuck.iMaxCol))
                {
                    Task=500;
                    tCalibrateAboveHeightZ1Delay.SetSecAndOn(1.0);
                }
            }
            break;
        case 500:
            //jou 2012-01-04 需確認Index suck已經完整做完
            if(INDEX_SUCKER_TYPE==1)
            {
                bIndexSuckCheck=false;
                bIndexSuckCheck=fiosetview->ProcessIndexSuckDestroy2();
            }
            else
            {
                bIndexSuckCheck=true;
            }

            if(tCalibrateAboveHeightZ1Delay.Off() && bIndexSuckCheck==true)
            {
                for(int i=0; i<FTestSuck.iMaxRow; i++)                          //jou 2011-07-01 start : 解決未完成auto high中途退出會一直出現need one cycle error
                {
                    for(int j=0; j<FTestSuck.iMaxCol; j++)
                    {
                        flag2=false;
                        BTestSuck.CheckVaccumIsON_AboveSocket(i, j, flag2);
                        if(flag2==true)
                        {
                            flag=true;
                            BTestSuck.Suck[j][i].Normal();
                        }
                    }
                }

                if(flag)                                                        //有真空, 還要繼續往上拉
                {
                    Task=200;
                }
                else                                                            //finish
                {
                    iZ2_AboveOffset=iZOffset;
                    str.sprintf("Z2 above height : %d", iZ2_AboveOffset);
                    Memo1->Lines->Add(str);
                    if(iZ1_AboveOffset>iZ2_AboveOffset)
                    {
                        iZOffset=iZ1_AboveOffset;
                    }
                    else
                    {
                        iZOffset=iZ2_AboveOffset;
                    }

                    dZoffset=iZOffset/100.0;
                    edD41->Text=dZoffset;
                    return true;
                }
            }
    }
    return bResult;
}

// ===== AI(W906-E042) 20261005 (B3): golden 0618 cContact.cpp:18565-18626 (ATC_SwitchTjSignal), verbatim: ATC_SwitchTjSignal span: port line = golden line + (-13044) =====
//  Decision (plan s1.2 S-26): TRANSLATED, not gated per site -- golden's arm-down notice is the tester's own signal (GPIB
//  MSG_CMD_Arm1/2Down, SVID iWhichArmDown, the SwTjSignal01-08 outputs); no motion.  Only the ATC 'which arm' calls stay
//  gated (W906-E042-ATC: ATC_InterfaceForm is a shim without HandlerArm).  HT9050 has no ATC; SwTjSignal* Enable decides.
void TfContact::ATC_SwitchTjSignal(int iIndexArm, bool bSnedArmDown)            //Ifor 20210622 add: ATC Switch TJ
{
    if(iIndexArm==1)
    {
        if(ATC_SYSTEM==eNewATCSystem &&                                         //Ifor 20241009 add: send ATC which ARM
           Temperature.bATCActiveCooling==true)
        {
            {} /*GATE(W906-E042-ATC) ATC_InterfaceForm->HandlerArm(0); -- AI(W906-E042): as above (the ATC 'which arm' notice)*/
        }

        if(bSnedArmDown==true)
            fMain->SendMSG_CMD(MSG_CMD_Arm1Down);                               //Steven 20150304 : Add GPIB LOG
        iWhichArmDown=1;                                                        //JerryYang 20200316 add SVID 哪支arm下壓在測區
        if(SW[SwTjSignal01].Enable==true)
            SW[SwTjSignal01].On();
        if(SW[SwTjSignal02].Enable==true)
            SW[SwTjSignal02].On();
        if(SW[SwTjSignal03].Enable==true)
            SW[SwTjSignal03].On();
        if(SW[SwTjSignal04].Enable==true)
            SW[SwTjSignal04].On();

        if(SW[SwTjSignal05].Enable==true)
            SW[SwTjSignal05].Off();
        if(SW[SwTjSignal06].Enable==true)
            SW[SwTjSignal06].Off();
        if(SW[SwTjSignal07].Enable==true)
            SW[SwTjSignal07].Off();
        if(SW[SwTjSignal08].Enable==true)
            SW[SwTjSignal08].Off();
    }
    else if(iIndexArm==2)
    {
        if(ATC_SYSTEM==eNewATCSystem &&                                         //Ifor 20241009 add: send ATC which ARM
           Temperature.bATCActiveCooling==true)
        {
            {} /*GATE(W906-E042-ATC) ATC_InterfaceForm->HandlerArm(1); -- AI(W906-E042): as above (the ATC 'which arm' notice)*/
        }

        if(bSnedArmDown==true)
            fMain->SendMSG_CMD(MSG_CMD_Arm2Down);                               //Steven 20150304 : Add GPIB LOG
        iWhichArmDown=2;                                                        //JerryYang 20200316 add SVID 哪支arm下壓在測區

        if(SW[SwTjSignal01].Enable==true)
            SW[SwTjSignal01].Off();
        if(SW[SwTjSignal02].Enable==true)
            SW[SwTjSignal02].Off();
        if(SW[SwTjSignal03].Enable==true)
            SW[SwTjSignal03].Off();
        if(SW[SwTjSignal04].Enable==true)
            SW[SwTjSignal04].Off();

        if(SW[SwTjSignal05].Enable==true)
            SW[SwTjSignal05].On();
        if(SW[SwTjSignal06].Enable==true)
            SW[SwTjSignal06].On();
        if(SW[SwTjSignal07].Enable==true)
            SW[SwTjSignal07].On();
        if(SW[SwTjSignal08].Enable==true)
            SW[SwTjSignal08].On();
    }
}


//------------------------------------------------------------------------------
//  golden 0618 cContact.cpp:5273-5382 -- TfContact::GetAutoHeightMaxKGTorque (Steven 20170720 (wei) : for low
//  contact force).  forms/fContact.h GATE (X-29) said "the calc core is ALREADY ported ... the member wrapper is
//  left for the wave that decides whether TfContact should delegate or duplicate".  AI(W906-E042) 20261004:
//  delegate -- the body is cContact.h:544 ComputeAutoHeightMaxKGTorque (golden branch for branch, pinned by
//  tests/test_cContact.cpp); this member passes golden's globals in the core's documented order:
//  iContactMode, IniConfig.bD10 / bD14 / iD14, DeviceForm.dKitDiameter, INDEX_PRESS_TYPE, TestIF.iTestMode,
//  TestSocket.iShtRow / iShtCol, TestIF_File.iSiteMap, IsNNMode() (golden :5276-5381 read exactly these).
//------------------------------------------------------------------------------
int TfContact::GetAutoHeightMaxKGTorque()
{
    return ComputeAutoHeightMaxKGTorque(iContactMode,
                                        IniConfig.bD10ManualHeightComptibleWithNS,
                                        IniConfig.bD14_AutoHeightUseSetTorque,
                                        IniConfig.iD14_AutoHeightUseSetTorque,
                                        DeviceForm.dKitDiameter,
                                        INDEX_PRESS_TYPE,
                                        TestIF.iTestMode,
                                        TestSocket.iShtRow,
                                        TestSocket.iShtCol,
                                        TestIF_File.iSiteMap,
                                        IsNNMode());
}

//------------------------------------------------------------------------------
//  golden 0618 cContact.cpp:18453-18540 -- TfContact::TestZ_CompensationHight (Ifor 20200318 : 80 mm kit
//  down-press compensation).  forms/fContact.h GATE (S-51) "writes iTotalOffset_1/2".  AI(W906-E042) 20261004:
//  Do_Z1_AutoGetHeight calls it (golden :7150 / :7430); the body is already ported as the calc core
//  cContact.h:584 ComputeTestZCompensationHight (golden quirk kept: kits other than 8 / 40.2 leave both offsets
//  untouched); it now writes the real golden globals iTotalOffset_1/2 defined above.
//------------------------------------------------------------------------------
void TfContact::TestZ_CompensationHight()
{
    ComputeTestZCompensationHight(DeviceForm_File.dKitDiameter, DeviceForm_File.dPress, dIndexZOffset,
                                  iTotalOffset_1, iTotalOffset_2);
}
