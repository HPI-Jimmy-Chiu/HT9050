//---------------------------------------------------------------------------
//  atester.cpp  --  TESTER / TEST-HEAD (Index down-press) ENGINE (W6.4 translation)
//
//  Translation wave: W6.4 (TESTER/INDEX ENGINE)
//  Translator: AI(W6.4-TESTER) 20260626
//  Golden source: HT9011UC_Code_V3.33.906.0_20260618/atester.cpp (11819 lines, cp950)
//
//  WHAT THIS FILE IS
//  -----------------
//  The tester/test-head engine: it drives the Index down-press test-head Y/Z
//  motors (MTestY1/MTestZ1/MTestY2/MTestZ2), the FTestSuck/BTestSuck nozzles,
//  the Socket test handshake, and owns iTestHeadMotorTask + iTestYTask.
//
//  W6.4 SCOPE (mirrors the W6.2/W6.3 arm + tray engines)
//  -----------------------------------------------------
//  ACTIVE, faithfully transcribed (cursor values + formulas + fall-throughs
//  VERBATIM, pumpable over Sim HAL):
//    * geometry / numeric anchors: GetRowCol (:144), GetRTCSiteMap (:423),
//      ArrayConvertSite (:504), GetOneByOneData (:621), GetBinaryData (:670),
//      Test_GetTestResulSub (:759), Test_GetTestResul (:789),
//      GetSocketCheckPos (:9791), GetIndexZSpeed (:9814).
//    * the test-cycle dispatcher SM DoTestY (:4789, switch(iTestYTask), oracle
//      path 1 -> 50 -> 100 -> 110 fall-through).
//    * the central index/test SM DoTestHeadMotor (:5562, switch(iTestHeadMotor
//      Task), oracle path 4 -> 9 -> 10 -> 15 over Sim HAL; entry preamble made
//      offline-safe; the dense per-case down-press trees (case 30..600000,
//      12000.., 14000.., load-cell) are GATED, the oracle-window cases reproduced
//      verbatim).
//    * the EP-check SM IndexEveryTimeCheckEP (:9158, case 1 early-out).
//    * the small Mode/Init SMs DoStartMode/DoEndMode/InitTest*/Init*Mode and the
//      decode consumers Check_TTL_Status (:2564) / CheckTestSocketIsError (:2584).
//
//  GATED (#if 0 // TODO(...)) with an ACTIVE compiling stub returning the golden
//  default, OR routed to atester_shims.cpp, so the file LINKS + the core SMs run:
//    * (AI(W906-GB-P2b) 20260926: GetTesterResult, ProcessTestResult, ProcessTesterTimeOut and
//      DoIndexSocketCheck are LIVE golden translations now -- tester-comm P2b, user ruling C-1; each
//      function banner registers its own gates.  They were listed here as gated stubs.)
//    * DoCheckSocketHasIC     (:4305 -- MOT[] vacuum-check SM).                W7.
//    * DoTestHeadMotorLoadCell(:10444).                                        W7.
//    * the per-case heavy bodies of DoTestY / DoTestHeadMotor that deref
//      MOT[]/Suck/Socket/EP/tester-comm beyond the oracle window.             W7.
//    * cContact / iIndexTask (the index-press SM is in cContact.cpp, a VCL FORM,
//      22761 lines -- NOT in scope; never derefed from atester.cpp).          W7.
//    * atester_32Site.cpp / atester_ProcessCount.cpp ->
//      routed to atester_shims (Do* report COMPLETE, Init* no-op).       W6.4b.
//    * aTester_Front.cpp / aTester_Rear.cpp (Front/Rear "Destroy IC" SM pair) --
//      AI(W64b-Integrate) 20260706: now translated for real (no longer an
//      atester_shims stub); every OTHER Front/Rear export (DoFrontTestSuckIC,
//      DoTestYFront, TestZ1OutRandge2, ...) still routes through atester_shims.
//    * the contact-mode form fContact + the ADAM_* EP DAQ API -> atester_shims.
//
//  TRANSLATION RULES
//  -----------------
//    * Off Borland: __fastcall removed; AnsiString/TQPF_Timer/TStringList via
//      vclcompat; numeric semantics EXACT (integer division kept -- esp.
//      GetIndexZSpeed *iScale/100 stays INTEGER division per the divide-safety
//      regression note).
//    * SOFT_SIMULTE NOT defined -> #ifndef/#ifdef SOFT_SIMULTE blocks that fall
//      inside ACTIVE functions are reproduced VERBATIM (the #ifndef bodies are
//      live: Test_GetTestResulSub Sen read; IndexEveryTimeCheckEP #else tree).
//      Blocks inside GATED SM bodies are gated away with those bodies (same as
//      the W6.2 arm engines -- a gated SM does not reproduce its inner directives).
//    * Big5 Chinese comments decoded via cp950, preserved as UTF-8.  ZERO U+FFFD.
//---------------------------------------------------------------------------
#include "MachineDefine.h"

#include "atester.h"
#include "atester_shims.h"          // fContact/ADAM_*/CCDInterfaceForm/fAutomation/fObserver/fiosetview/COM2 offline shims (32Site/ProcessCount/Front/Rear now real, see below)
// AI(W64b-Integrate) 20260706: mirrors golden atester.cpp:7-8, which #include
// "aTester_Front.h"/"aTester_Rear.h" directly right after atester.h -- the
// Front/Rear "Destroy IC" state machines are now translated for real (no
// longer routed through atester_shims' offline stubs for these 6 symbols).
#include "aTester_Front.h"
#include "aTester_Rear.h"
// AI(W5-Automation-Integrate) 20260710: atester_32Site.cpp / atester_ProcessCount.cpp
// are now real translations (added to ht9045_sm) -- include their own headers
// directly instead of relying on atester_shims.h's (now-removed) declarations.
#include "atester_32Site.h"
#include "atester_ProcessCount.h"
#include "aArmHeader.h"             // __FUNC__ shim

#include "MachineType.h"
#include "Motor/mymotor.h"          // MOT[], TTrayMotor
#include "mysensor.h"               // Sen[]
#include "myswitch.h"               // AI(W7T1-Integrate) 20260701: SW[] (SwIndexChangeToque1/2 .Off() in case 12000/etc.) -- golden global; un-gated down-press tree needs it
#include "vclcompat/vcl_compat.h"   // AI(W7T1-Integrate) 20260701: AnsiString(int) ctor (labUser Caption from iUseSocketHeating_Time)
#include "csystem.h"                // state predicates (TestHeadHasIC / InputShuttleHasIC / ...)
#include "cprod.h"                  // Prod / TestIF / TestIF_File / CosFunction / DeviceForm / SThreadPara
#include "cmydef.h"                 // global scalar universe + IC consts + enums
#include "cpublic.h"                // COM2
#include "common.h"
#include "aHotPlateSubstrate.h"     // FTestSuck/BTestSuck/FLCarryKit/BLCarryKit/FRCarryKit/BRCarryKit/TestSocket/InArmSuck
#include "FormsFacade.h"            // fMain offline stand-in
#include "canary_support.h"         // LastSet / ShowMyMessage / ShowErrorMessage / RecordProcess
#include "forms/fMesSystem.h"       // fMesSystem->AutoSiteMapPass (P2b gate R06 lifted)   AI(W906-GB-P2b) 20260926
#include "SECSGEM/SecsEventType.h"  // SECS_EVENT (P2b gate R09 lifted)
#include "SECSGEM/SecsEventReport.h"  // EventReport (P2b gate R09 lifted)
//---------------------------------------------------------------------------

const int CCDTimeOutSec=10;                                                     // golden :67
const int TESTZ1UP=0;                                                           // golden :68
const int TESTZ2UP=1;                                                           // golden :69
const int MaxDIO=40;                                                            // golden :70
const int CheckFailDownCT=1;                                                    //kevin 20131121   10 來不及偵測掉料  -- golden :71

FILE *EPOut;                                                                    //Ifor 20150707  -- golden :80

DWORD dwEndShuttle1Soak;                                                        //JerryYang 20181001 (Steven) : fix Shuttle soak time 倒數秒數異常
DWORD dwStartShuttle1Soak;
DWORD dwEndShuttle2Soak;
DWORD dwStartShuttle2Soak;

TQPF_Timer NULL_Delay;
TQPF_Timer DutDelay;                                                            //Steven 20180808 (wei) : TTL的時間單位改成microsecond(μs)
TQPF_Timer SoftContactTim;                                                      //kevin 20130608 soft 模式 需等ep 穩定
TQPF_Timer TSDDelay;
TQPF_Timer DoGiveWayDelay;                                                      //Ifor 20190723 : add
TQPF_Timer Noise_Delay;
TQPF_Timer HangTime;                                                            //Steven 20090827 : Hang Up dectector
TQPF_Timer hIndexSoakTime;                                                      //2013-11-27   Dell    需要做Index soak time
TQPF_Timer HAfterTestedDelay;                                                   //ChungHung 20140730 add for ATK function after tested delay time
TQPF_Timer DropContactTimer1;                                                   //JerryYang 20170503 (wei) drop contact的index cycle time分成三段來計時
TQPF_Timer DropContactTimer2;
TQPF_Timer DropContactTimer3;
TQPF_Timer dwStartInitialCount;
TQPF_Timer dwEndInitialCount;                                                   //kevin 20180926 add
TQPF_Timer hStartModeWaitTime;
TQPF_Timer DoTestHeadMotorDelay;
TQPF_Timer DoTestHeadMotorDelay2;
// AI(W7T1-Integrate) 20260701: RTC-CCD index-check entry case ids restored to their golden
// file-scope home (golden atester.cpp:5551-5555).  W6.4 had temporarily relocated iCASE_REAL_CCD2
// into atester_shims.cpp as =9 (valid only WHILE the RTC case tree was gated); now the tree is
// ACTIVE, so `case iCASE_REAL_CCD2:` (=9) would collide with the active `case 9:`.  These `const int`
// have internal linkage (only atester.cpp references them, verified tree-wide), so no ODR export.
const int iCASE_REAL_CCD2=40200;                                                // golden atester.cpp:5551
const int iCASE_REAL_CCD3=40300;                                                // golden atester.cpp:5552
const int iCASE_REAL_CCD4=40400;                                                // golden atester.cpp:5553
const int iCASE_REAL_CCD5=40500;                                                // golden atester.cpp:5554
const int iCASE_REAL_CCD6=40510;                                                // golden atester.cpp:5555
TQPF_Timer TestTimeOut;
TQPF_Timer HTestDeley;

int iTestYTask=1;                                                               // golden cursor (InitTestYTask :4784)
int iSetupTask=1;                                                               // golden cursor (DoSetupTest)
int iDoInterFaceErrorStepTask=1;                                               // golden cursor (DoInterFaceErrorStep)
int iCheckSocketHasIC=0;                                                        // golden cursor (InitCheckSocketHasIC :4298)  //AI(W906-FLOW-1) 20260927: golden atester.cpp:4297 has no initialiser (=0); InitCheckSocketHasIC is called only inside DoTestY (golden :4817/:4845) -- was 1
int iIndexEveryTimeCheckEPTask=1;                                              // golden cursor (InitIndexEveryTimeCheckEP :9151)
int iIndexYAxisServoOnStateTas=0;
int iIndexYAxisServoOnStateTask=1;                                              //AI(W906-FLOW-1) 20260927: golden atester.cpp:9076 `=1`; its SM (IndexYAxisServoOff / CheckIndexYAxisServoOnState :9079-9146) is commented out in golden -> recorded value only -- was 0
int iDoIndexSocketCheckTask=1;
int iTempICTask=1;
bool bTestDuplicateErr=false;

int iStartModeEvent1=0;
int iStartModeEvent2=1;
int iStartModeEvent3=2;
int iDoEndMode=1;
int iDoStartMode=1;
int iCCDTimeOutCount=0;
int iStartStep=0;
int iWhichAxisTestIC=0;
int iSendSMITimeOutLimit=1;
int iSocketSenSosPos1=0, iSocketSenSosPos2=0;                                   //kevin 20150613 關arm 設定可判斷位置
int iReContactCnt[MAX_SOCKET_ROW][MAX_SOCKET_COL];                              //Steven 20231205 : 計算某site contact 次數
int iTesterBIN[MAX_SOCKET_ROW][MAX_SOCKET_COL]={{-1, -1, -1, -1, -1, -1, -1, -1},
                                                {-1, -1, -1, -1, -1, -1, -1, -1},
                                                {-1, -1, -1, -1, -1, -1, -1, -1},
                                                {-1, -1, -1, -1, -1, -1, -1, -1}};

bool GetIndexTime_flag=false;
bool ScanPort[MaxDIO];
bool bFTestSuckDrop=false;
bool bBTestSuckDrop=false;
bool bZ1NeedTest=false, bZ2NeedTest=false;
bool bDoubleContact=false;
bool bNeedUpDonwOneTome=false;
bool fRearCheckSuckICPass=false;
bool bIndexPickUpErrMoveSht1=false;                                             //Steven 20171221 (Wei) : 修正[D43]當蝦頭退出來要回去前,如果In Arm補了HAS_NULL_IC在蝦頭上會造Hang up
bool bIndexPickUpErrMoveSht2=false;                                             //Steven 20171221 (Wei) : 修正[D43]當蝦頭退出來要回去前,如果In Arm補了HAS_NULL_IC在蝦頭上會造Hang up
bool bIndexWaitingInArmAway=true;                                               //Steven 20171228 (Wei) : Index在等In Arm讓開
bool bNeedCheckRTCReport=false;                                                 //Ifor 20190723 : add
bool SetNoiseDelay=false;
bool bHangTimePause=false;                                                      //Steven 20090827 : Hang Up dectector
bool bInitialSackTime=false;                                                    //kevin 20131112 第一次吸取ic等待時間
bool bFirstZ1UPZ2Down=true;                                                     //kevin 20131112 加熱時z1在shuttle 1上面 z2在下
bool bNeedIndexSoakTime=false;                                                  //2013-11-27   Dell    需要做Index soak time
bool bATCSiteTest[32]={false, false, false, false, false, false, false, false,
                       false, false, false, false, false, false, false, false,
                       false, false, false, false, false, false, false, false,
                       false, false, false, false, false, false, false, false};                                         //Ifor 20160510 add ATC Test Site

//==============================================================================
int GetRowCol(int &iRow, int &iCol)
{
    int iSiteMapRTC=-1;

    if(TestIF_File.iTestMode==DualSite) //1x2
    {
        iRow=1;
        iCol=2;
        iSiteMapRTC=1;
    }
    else if(TestIF_File.iTestMode==SingleSite) //1x1
    {
        iRow=1;
        iCol=1;
        iSiteMapRTC=0;
    }
    else if(TestIF_File.iTestMode==TriSite1X3)
    {
        iRow=1;
        iCol=3;
        iSiteMapRTC=1;
    }
    else if(TestIF_File.iTestMode==QualSite1X4 || TestIF_File.iTestMode==_8Site1X4)
    {
        iRow=1;
        iCol=4;
        iSiteMapRTC=2;
    }
    else if(TestIF_File.iTestMode==QualSite2X2 || TestIF_File.iTestMode==QualSite2X2N) //2x2
    {
        iRow=2;
        iCol=2;
        iSiteMapRTC=3;
    }
    else if(TestIF_File.iTestMode==DualSite2x1) //2x1
    {
        iRow=2;
        iCol=1;
    }
    else if(TestIF_File.iTestMode==_6Site2X3 ||
            TestIF_File.iTestMode==_6Site2X3N)
    {
        iRow=2;
        iCol=3;
        iSiteMapRTC=7;
    }
    else if(TestIF_File.iTestMode==_8Site2X4) //2x4
    {
        iRow=2;
        iCol=4;
        iSiteMapRTC=4;
    }
    else if(TestIF_File.iTestMode==_16Site4X4)
    {
        iRow=4;
        iCol=4;
    }
    else if(TestIF_File.iTestMode==_10Site2X5) //2x5
    {
        iRow=2;
        iCol=5;
        iSiteMapRTC=5;
    }
    else if(TestIF_File.iTestMode==_12Site2X6) //2x6
    {
        iRow=2;
        iCol=6;
        iSiteMapRTC=5;
    }
    else if(TestIF_File.iTestMode==_16Site2X8) //2x8
    {
        iRow=2;
        iCol=8;
        iSiteMapRTC=6;
    }
    else if(TestIF_File.iTestMode==_32Site4X8M || TestIF_File.iTestMode==_32Site4X8N) //4x8
    {
        iRow=4;
        iCol=8;
    }
    return iSiteMapRTC;
}
//==============================================================================
void InitDoStartMode()
{
    iDoStartMode=1;
}
//==============================================================================
void InitDoEndMode()
{
    iDoEndMode=1;
}
//------------------------------------------------------------------------------
bool IndexAlarmInArmAway()                                                      //Steven 20130613 : Index異常時, In Arm要先讓位功能
{
    bool ret=true;
    if(IniConfig.bIndexJamInArmAway==true)                                      //Steven 20110607 : Index Jam, In Arm要移開
    {
        bIndexWaitingInArmAway=true;                                            //Steven 20171228 (Wei) : : Index在等In Arm讓開

        if(CheckInArmFinishAllPickerAction()==false)                            //Steven 20171226 (Wei) : 修改in arm讓開的flag
        {                                                                       //kevin 20131119 Z軸正在吸取不能被中斷
            return false;
        }

        bInArmNeedToSafePos=true;                                               //Steven 20130819
        InitInArmTask();

        if(MoveInArm2XYToWait()==false)                                         //如果要移開,而且還沒移到定位
        {
            ret=false;                                                          //先離開等In Arm
        }
        else
        {
            ret=true;
            bIndexWaitingInArmAway=false;                                       //Steven 20171228 (Wei) : : Index在等In Arm讓開
        }
    }
    return ret;
}
//---------------------------------------------------------------------------
bool DoStartMode(int mode)
{
    int &Task=iDoStartMode;
    if(mode==0)                                                                 //Mode 0 return true
    {
        return true;
    }
    else if(mode==1)                                                            //Mode 1 向host send "PRODUCTION_REQUEST 0003" return true;
    {
        fAutomation->DoCommandBuffer("PRODUCTION_REQUEST", "", "0003", 0, "");
        return true;
    }
    else if(mode==2)                                                            //Mode 2
    {
        switch(Task)
        {
            case 1:                                                                                                     //向host send "PRODUCTION_REQUEST 0003"
                fAutomation->ClearEvent(iStartModeEvent1);                                                              //釋放資源
                fAutomation->GetEventNum(iStartModeEvent1, "PRODUCTION_REPLY",   "0003");                               //取得編號 設定條件字串
                fAutomation->ClearEvent(iStartModeEvent2);                                                              //釋放資源
                fAutomation->GetEventNum(iStartModeEvent2, "PRODUCTION_REQUEST", "0007");                               //取得編號 設定條件字串
                fAutomation->ClearEvent(iStartModeEvent3);                                                              //釋放資源
                fAutomation->GetEventNum(iStartModeEvent3, "PRODUCTION_REQUEST", "0008");                               //取得編號 設定條件字串
                fAutomation->DoCommandBuffer("PRODUCTION_REQUEST", "","0003", 0, "");
                hStartModeWaitTime.SetSecAndOn(10);                                                                     //設定計數
                Task=50;
            case 50:                                                            //等待Host回覆"PRODUCTION_REPLY 0003"
                if(fAutomation->GetEventResult(iStartModeEvent1)==true)         //Host回覆"PRODUCTION_REPLY 0003"
                {
                    fAutomation->ClearEvent(iStartModeEvent1);                  //釋放資源
                    hStartModeWaitTime.SetSecAndOn(10);
                    Task=100;
                }
                else                                                            //Host還未回覆"PRODUCTION_REPLY 0003"
                {
                    if(hStartModeWaitTime.Off())
                    {
                        Task=1;
                        fAutomation->ClearEvent(iStartModeEvent1);              //釋放資源
                        return true;
                    }
                    break;
                }
            case 100:                                                           //Host回應"PRODUCTION_REQUEST 0007" OK or "PRODUCTION_REQUEST 0008" NG
                if(iStartModeEvent1==-1)                                        //Sam 20190429 : Add CC_PTI_NEWWORK
                {
                   iStartModeEvent1=0;
                   Task=1;
                   break;
                }

                if(fAutomation->GetEventResult(iStartModeEvent2)==true)         //Host 有回應
                {
                    fAutomation->ClearEvent(iStartModeEvent2);                  //Sam 20190429 : Add CC_PTI_NEWWORK
                    fAutomation->DoCommandBuffer("PRODUCTION_REPLY", "","0007", 0, "");
                    bStartModeComplete=true;                                    //Sam 20190429 : Add CC_PTI_NEWWORK
                    Task=1;                                                     //Sam 20190429 : Add CC_PTI_NEWWORK
                    return true;
                }
                else if(fAutomation->GetEventResult(iStartModeEvent3)==true)    //NG
                {
                    fAutomation->ClearEvent(iStartModeEvent3);                  //Sam 20190429 : Add CC_PTI_NEWWORK
                    fAutomation->DoCommandBuffer("PRODUCTION_REPLY", "","0008", 0, "");
                    Task=1;                                                     //Sam 20190429 : Add CC_PTI_NEWWORK
                }
                else                                                            //Host 還未回覆
                {
                    if(hStartModeWaitTime.Off())                                //逾時 ShowAlarm Message "Wait PRODUCTION_REPLY 0007 Over Time" Retry or Skip
                    {
                        Task=1;                                                 //Sam 20190429 : Add CC_PTI_NEWWORK
                        return true;
                    }
                }
                break;
        }
    }
    return false;
}
//------------------------------------------------------------------------------
TQPF_Timer hEndModeWaitTime;
int iEndModeEvent1=0;
bool DoEndMode(int mode)
{
    int &Task=iDoEndMode;

    if(bStartModeComplete==false)
    {
        return true;
    }

    if(mode==0)                                                                 //Mode 0 return true
    {
        return true;
    }

    if(mode==1)                                                                 //Mode 1 向host send "TEST_RESULT_REQUEST" return true;
    {
        fAutomation->DoCommandBuffer("TEST_RESULT_REQUEST", "", "", 0, "");
        return true;
    }

    switch(Task)                                                                //Mode 2
    {
        case 1:                                                                 //向host send "TEST_RESULT_REQUEST"
            fAutomation->DoCommandBuffer("TEST_RESULT_REQUEST", "", "", 0, "");
            hEndModeWaitTime.SetSecAndOn(10);                                   //設定計數
            fAutomation->GetEventNum(iEndModeEvent1, "TEST_RESULT_REPLY", "");  //取得編號 設定條件字串
            Task=50;
        case 50:                                                                //等待Host回覆"TEST_RESULT_REPLY"
            if(fAutomation->GetEventStrResult(iEndModeEvent1)==3)               //Host回覆"PRODUCTION_REPLY 0003"
            {
                fAutomation->ClearEvent(iEndModeEvent1);                        //釋放資源
                bStartModeComplete=false;
                Task=1;
                return true;
            }
            else                                                                //Host還未回覆"TEST_RESULT_REPLY"
            {
                if(hEndModeWaitTime.Off())
                {
                    fAutomation->ClearEvent(iEndModeEvent1);
                    Task=1;
                    return true;
                }
                break;
            }
    }
    return false;
}
//==============================================================================
bool ScanCCDProgram()
{
    HWND HCCDWnd=FindWindow(NULL,"Identification");
    if(HCCDWnd==NULL)
    {
        CCDInterfaceForm->bAtestScanCCDProgram=false;                           //kevin 20110811
        return false;
    }
    else
    {
        CCDInterfaceForm->bAtestScanCCDProgram=true;                            //kevin 20110811
        return true;
    }
}
//==============================================================================
const int OverEncoderDelay=50;

int iGetTestDataDelayTask=1;
int iRecordOldBin[MAX_SOCKET_ROW][MAX_SOCKET_COL]={{-1, -1, -1, -1, -1, -1, -1, -1},
                                                   {-1, -1, -1, -1, -1, -1, -1, -1},
                                                   {-1, -1, -1, -1, -1, -1, -1, -1},
                                                   {-1, -1, -1, -1, -1, -1, -1, -1}};

//==============================================================================
int GetRTCSiteMap()
{
//                    ========================================================
//                    第5碼        Site 分佈樣式
//                    ========================================================
//                    asSiteMap    0         1 x 1
//                                 1         1 x 2
//                                 2         1 x 4
//                                 3         2 x 2
//                                 4         2 x 4
//                                 5         2 x 6
//                                 6         2 x 8
//                    ========================================================
    int iSiteMapRTC=-1;

    if(TestIF_File.iTestMode==SingleSite)                                       //1x1
    {
        iSiteMapRTC=0;
    }
    else if(TestIF_File.iTestMode==DualSite)                                    //1x2
    {
        iSiteMapRTC=1;
    }
    else if(TestIF_File.iTestMode==TriSite1X3)                                  //Frank 20160329 add for 1x3_4
    {
        iSiteMapRTC=1;
    }
    else if(TestIF_File.iTestMode==QualSite1X4 ||
            TestIF_File.iTestMode==_8Site1X4)                                   //ChungHung 20150528 add for 海思 _8Site1x4 //1x4
    {
        iSiteMapRTC=2;
    }
    else if(TestIF_File.iTestMode==DualSite2x1)                                 //2x1
    {
        iSiteMapRTC=1;
    }
    else if(TestIF_File.iTestMode==QualSite2X2 ||                               //2x2
            TestIF_File.iTestMode==QualSite2X2N)                                //Frank 20200520 2X2NN Mod
    {
        iSiteMapRTC=3;
    }
    else if(TestIF_File.iTestMode==_6Site2X3 ||                                 //ChungHung 20140115 add for 2x3_6
            TestIF_File.iTestMode==_6Site2X3N)                                  //Steven 20220425 : 2X3NN Mode
    {
        iSiteMapRTC=7;
    }
    else if(TestIF_File.iTestMode==_8Site2X4 ||                                 //2x4
            TestIF_File.iTestMode==_8Site2X4N ||                                //Wei 20231211 : 2X4NN Mode
            TestIF_File.iTestMode==_16Site4X4)                                  //Sam 20190226 : 16Site4X4
    {
        iSiteMapRTC=4;
    }
    else if(TestIF_File.iTestMode==_10Site2X5)                                  //2x5  //wei 20190614 10 site
    {
        iSiteMapRTC=5;
    }
    else if(TestIF_File.iTestMode==_12Site2X6)                                  //2x6
    {
        iSiteMapRTC=5;
    }
    else if(TestIF_File.iTestMode==_16Site2X8)                                  //2x8  //Eliot 2009_12_25
    {
        iSiteMapRTC=6;
    }
    else if(TestIF_File.iTestMode==_32Site4X8M ||                               //4x8
            TestIF_File.iTestMode==_32Site4X8N)                                 //ChungHung 20130627 alter TestIF--->TestIF_File 修正無法跑32Site
    {
        iSiteMapRTC=6;
    }
    return iSiteMapRTC;
}
//==============================================================================
//******************************************************************************
//
//  注意!! ArrayConvertSite內容影響Site Mapping與開關Site, 修改時要小心驗證
//
//******************************************************************************
int ArrayConvertSite(int X, int Y)
{
    int Site=-1;

    if(TestIF.iTestMode>=_6Site2X3)                                             //Alick 20161011 (Steven) : TTL支援8Site
    {
        Site=X*4+Y+1;
    }
    else if(TestIF.iTestMode>=DualSite2x1)
    {
        Site=X*2+Y+1;
    }
    else
    {
        Site=Y+1;
    }
    return Site-1;
}
//==============================================================================
//  CheckBackError (golden :523) -- read 4 index axes vs TestZ1_Safe window.
//  ACTIVE: offline Gali_ReadPos preserves Position (deterministic 0 baseline).
//==============================================================================
bool CheckBackError()
{
    int ArmCmdPos[4], i;
    ArmCmdPos[0]=MOT[MTestY1].Gali_ReadPos();
    ArmCmdPos[1]=MOT[MTestZ1].Gali_ReadPos();
    ArmCmdPos[2]=MOT[MTestZ2].Gali_ReadPos();
    ArmCmdPos[3]=MOT[MTestY2].Gali_ReadPos();
    for(i=0; i<4; i++)
    {
        if(ArmCmdPos[i]>=Prod.TestZ1_Safe+30 ||
           ArmCmdPos[i]<=Prod.TestZ1_Safe-30)
            return true;
    }
    return false;
}
//==============================================================================
//  TestArmBackPos (golden :539) -- check Z1/Z2 at home, then GalilTwoY_Move back.
//  The #ifndef SOFT_SIMULTE Z-position block is reproduced VERBATIM (SOFT_SIMULTE
//  not defined -> live).  Offline Gali_ReadPos returns 0; if TestZ*_Safe is ~0 the
//  error branch is skipped and the GalilTwoY_Move reports complete (Motor==NULL).
//==============================================================================
bool TestArmBackPos()
{
    bool bError=false;
    static int Temp[2]={0, 0};
    #ifndef SOFT_SIMULTE
    int Z1CmdPos=MOT[MTestZ1].Gali_ReadPos();
    int Z2CmdPos=MOT[MTestZ2].Gali_ReadPos();

    if(Z1CmdPos>=Prod.TestZ1_Safe+50 ||
       Z1CmdPos<=Prod.TestZ1_Safe-50)
    {
        Temp[0]++;
        if(Temp[0]>1000)                                                        //Steven 20170104 : 加上Delay, 等機構穩定
        {
            ShowMyMessage("Index Motor Z1 is not at home position!! [D51]", "Z1馬達未在上方位置!!");                            //Steven 20091004
            Temp[0]=0;
            return false;
        }
        else
        {
            bError=true;
        }
    }

    if(Z2CmdPos>=Prod.TestZ2_Safe+50 || Z2CmdPos<=Prod.TestZ2_Safe<=-50)
    {
        Temp[1]++;
        if(Temp[1]>1000)
        {
            ShowMyMessage("Index Motor Z2 is not at home position!! [D51]", "Z2馬達未在上方位置!!");                            //Steven 20091004
            Temp[1]=0;
            return false;
        }
        else
        {
            bError=true;
        }
    }
    #endif

    if(bError==false)
    {
        if(MOT[MTestY1].GalilTwoY_Move(Prod.TestY1_Middle, Prod.TestY2_Rear, 50000, __FUNC__))
        {
            Temp[0]=0;
            Temp[1]=0;
            IndexStatus=IndexIsBack;
            return true;
        }
    }
    return false;
}
//==============================================================================
int iTestTask=1;
void InitTestTask()
{
    iTestTask=1;
}

//******************************************************************************
//
//  注意!! GetOneByOneData為TTL收發BIN相關, 修改時要小心!!
//
//******************************************************************************
extern bool bEcho, bExist, bUnderTest;
extern bool bEchoStop;                                                          //ChungHung 20130326 add
extern unsigned int iBin[4][8];
extern bool bGPIBError;
int HexData[5]={0x01, 0x02, 0x04, 0x08, 0x10};
//------------------------------------------------------------------------------
int IOCT[]={4,                                                                  //_4Bit=0
            8,                                                                  //_8Bit=1
            4,                                                                  //_5BitPE
            9,                                                                  //_10BitPE
            5,                                                                  //_5Bit
            10,                                                                 //_10Bit
            4,                                                                  //_5BitPO
            9};                                                                 //_10BitPO
                    //_3Bit=8;
//------------------------------------------------------------------------------
int GetOneByOneData(int X, int Y)
{
    int iResult=-1;
    int Data;
    int iSite=ArrayConvertSite(X, Y);
    int sum=0;

    if(Prod.DIOCfg.iCateBitLength==_8Bit || Prod.DIOCfg.iCateBitLength==_10Bit ||
       Prod.DIOCfg.iCateBitLength==_10BitPE || Prod.DIOCfg.iCateBitLength==_10BitPO)
    {
        Data=iSite*10;
        if(CosFunction.bTTLCanUse8Site==true)                                   //Alick 20161011 (Steven) : TTL支援8Site
        {
            if(iSite>=4)
                return -1;
        }
        else
        {
            if(iSite>=2)
                return -1;
        }
    }
    else
    {
        Data=iSite*5;
    }

    for(int i=0; i<IOCT[Prod.DIOCfg.iCateBitLength]; i++)
    {
        if(ScanPort[i+Data])                                                    //jou ??? over array
        {
            sum++;
            iResult=i;
        }
    }

    if(iResult!=-1)
    {
        if(sum!=1)
            return iTestBinCount;                                               //Jou 20210126 : 16 --> iTestBinCount
        return iResult+1;
    }
    return -1;
}
//******************************************************************************
//
//  注意!! GetBinaryData為TTL收發BIN相關, 修改時要小心!!
//
//******************************************************************************
int GetBinaryData(int X, int Y)
{
    int Sum=0;
    int Data;
    int iSite=ArrayConvertSite(X, Y);
    if(Prod.DIOCfg.iCateBitLength==_8Bit || Prod.DIOCfg.iCateBitLength==_10Bit ||
       Prod.DIOCfg.iCateBitLength==_10BitPE || Prod.DIOCfg.iCateBitLength==_10BitPO)
    {
        Data=iSite*10;
        if(iSite>=2)
            return -1;
    }
    else
    {
        Data=iSite*5;
    }

    for(int i=0; i<IOCT[Prod.DIOCfg.iCateBitLength]; i++)
        Sum+=ScanPort[i+Data]*HexData[i];
    if(Sum==0)
        return -1;
    else
        return Sum;
}
//******************************************************************************
//
//  注意!! Test_GetTestResulSub為TTL收發BIN相關, 修改時要小心!!
//
//******************************************************************************
int Test_GetTestResulSub(int X, int Y)
{
#ifndef SOFT_SIMULTE
    for(int i=0; i<10; i++)
    {
        ScanPort[i]     =Sen[SenBit0+i].Status();
        ScanPort[i+10]  =Sen[SenBit10+i].Status();
        ScanPort[i+20]  =Sen[SenBit20+i].Status();                              //Alick 20161011 (Steven) : TTL支援8Site
        ScanPort[i+30]  =Sen[SenBit30+i].Status();
    }
#endif
    for(int i=0; i<10; i++)
    {
        if(ScanPort[i] && SetNoiseDelay==false)
        {
            Noise_Delay.SetSecAndOn(0.1);
            SetNoiseDelay=true;
        }
    }

    if(Prod.DIOCfg.iCateDataType==CHOneByOne)
        return GetOneByOneData(X, Y);
    else
        return GetBinaryData(X, Y);
}
//******************************************************************************
//
//  注意!! Test_GetTestResul為TTL收發BIN相關, 修改時要小心!!
//
//******************************************************************************
int Test_GetTestResul(int X, int Y)                                             //990517 保護機制,重複確認50次
{
    int iOld=-1, iBuffer=-1;

    iOld=Test_GetTestResulSub(X, Y);
    if(iOld==-1)
        return -1;

    for(int i=0; i<50; i++)
    {
        iBuffer=Test_GetTestResulSub(X, Y);
        if(iOld!=iBuffer)
            return -1;
    }

    return iOld;
}
//******************************************************************************
//
//  注意!! GetTesterResult為Handler收發BIN相關, 修改時要小心!!
//
//  AI(W906-GB-P2b) 20260926: LIVE golden translation (tester-comm P2b) -- see the
//  GetTesterResult banner below for the gate register.  (Was a gated whole body
//  with a stub returning false.)
//******************************************************************************
TQPF_Timer TestWaitTime;
TQPF_Timer MyTSDTimer;                                                          //Steven 20161208 : Record TSD time data
TQPF_Timer LBBoostTimeOutTimer;
TQPF_Timer TestStopMotorTimer;
HTimer bTestFailNeedWait;                                                       //Eastsun 20260526 #026-1.6 Ifor 20250620 add:測試溫度Fail後延遲20秒
bool bLBBoostTimeOut=false;                                                     //Steven 20181222 : Add LB升溫的Time out
// ===========================================================================
//  GetTesterResult -- LIVE translation (tester-comm plan P2b).
//  AI(W906-GB-P2b) 20260926, St02 (user ruling 20260926: "由 St02 直接進行這一部分的移植工作").
//
//  GOLDEN.  Base = V3.33.906.0 atester.cpp:849-2553, i.e. the GOLDEN VERBATIM block that used to sit here
//  (byte-identical to the cp950-decoded golden, re-checked this wave), plus the three additions of
//  V3.33.912.0 (user ruling "912是新版本，可以拿906的補充912的就好"):
//      912:1048        Murata 2DID-NG skip also writes a PordRec test-result record
//      912:1052-1067   Barcode CSV Compare, pass-bin skip (Ifor 20260511)            -> contains T03
//      912:1396-1398   A76 HANA RMS recipe interlock before the test signal (RogerYang 20260902) -> T08
//  Result = golden 912 atester.cpp:849-2573 line for line.  EVERY "golden atester.cpp:N" in this banner and
//  in the gate headers is V912 numbering.  V906 numbering: N (N<=1047), N-1 (1049-1051), N-17 (1068-1395),
//  N-20 (N>=1399).
//
//  RULES.  Golden text verbatim: golden comments, column layout, golden defects, and every #ifdef SOFT_SIMULTE /
//  #ifndef SOFT_SIMULTE / #ifdef ASE_KaohSiung branch are kept.  NB: this tree's DEFAULT build defines
//  SOFT_SIMULTE (MachineType.h:63-65, user 20260918), so :1208 `iTriggerBoostFunction=-1;` is compiled in the
//  default build exactly as in a golden SIM build; the shipping build (-DW906_NO_SOFT_SIMULTE=ON) drops it.
//  A symbol that atester.cpp's include closure (its #includes, transitively) does not declare is gated at the
//  smallest statement or operand -- golden text inside `#if 0 // TODO(W906-GB-P2b): Tnn`, and an `#else` where
//  an operand or a value is needed.  Nothing is stubbed; no #include was added.
//
//  GATE REGISTER  (id | golden lines | missing symbol, where looked | #else value -> effect)
//   T01 :856 :923      `Byte` not declared in the closure (V906: Interface/InterfaceSYS.h:36, BinDisplay/
//                      MyBinDisp.h:490, neither included).  BarCodeMess is only memset here -> no effect.
//   T02 :882           fStartCondition->SocketIDLog(): no fStartCondition global anywhere (forms/fStartCondition.h
//                      :272).  No socket-ID CSV line per test start (that call is a file write, fStartCondition.h:93).
//   T03 :1057          TfBarCode has no DoBarcodeCSVCompare.  #else false -> the device is tested; with
//                      [bEnableBarcodeCSVCompare] on, an already-passed device is NOT diverted (no WAR04217).
//   T04 :1212 :1435 :1511-1512 :1684 :1691 :2206 :2210 :2319-2320 :2517-2518
//                      ATC_InterfaceForm / ATC_TYPE_70 / ATC_TYPE_60 not in the closure (acarry_shims.h:115 shim
//                      with iATC_MODE_TYPE only; forms/fATCHandlerSide.h:695/697).  Operands -> false.  Exact while
//                      ATC_SYSTEM!=eNewATCSystem (shim stays 0).  On a new-ATC machine W906_ReadATCIni
//                      (forms/fATCHandlerSide.cpp:631) loads the real mode into the shim, so golden would take the
//                      ATC7.0 by-site TSD path (case 20000); here it does not, and StartTesting / SiteTesting /
//                      UseTSD_Function / ATC60 air-flow are not sent (the shim has none of them anyway).
//   T05 17 x `if(fContact->fShow) fContact-><widget>...;` at :1229 :2292 :2348 :2362 :2373 :2388 :2394 :2412
//                      :2422 :2443 :2457 :2480 :2486 :2521 :2531 :2553 :2565 -- TfContactShim has no
//                      labDelayStatus / lblCountDown / pnlHandlerSatus.  fShow is false offline -> exact.
//   T06 :1384          WaitManualStartKey (ckernel.h:96, not included).  #else "not pressed" -> with [G12] the SM
//                      keeps waiting at case 20 (never auto-grants a manual START).
//   T07 :1392          fMesSystem (forms/fMesSystem.h:715, not included).  #else: golden's own early-out
//                      (AutoSiteMap -> OK) kept, otherwise treated as RCS FAIL -> break (VTEST [bGetRcsChecking
//                      Result] machines do not test; golden SOFT_SIMULTE answers FAIL as well).
//   T08 :1396          TfAutomationShim has no HANARMSRunCheckOK (912 addition).  #else: golden's early-out
//                      (automation.cpp:3139-3142) kept -> no break on every non-A77-HANA machine; on an A77 HANA
//                      machine the recipe check cannot run -> break (fail-safe).
//   T09 :1423-1424     TfMain has no pcCommView / pgcTorque.  #else false -> iCheckTorqueCount not counted here
//                      (offline pgMain->ActivePageIndex is 0, so golden is false too).
//   [LIFTED P2b 20260926] T10 :1448 :1484  fMain->bFind -> W906_TesterBridgeFound() (forms/fMain.h seat, reads
//                      THandlerTesterSide::bFind); not installed = false, the golden "bridge not found" path
//                      (TestTimeOut 3 s, Task=9999) -- golden logic unchanged.
//   T11 :1508 :1555 :1681 :1697   ATCInterfaceForm (ATC/ATCInterface.h:528, not included).  The guarding ifs
//                      need ATC_SYSTEM (Gerneral.ini [ATC] USE_ATC_MODE, default 0) == eATC60/eATC30/eATCHonPrecType,
//                      so exact on a machine without those ATCs; with one: no SendTestStart / SendTestEnd.
//   T12 :1541 :1545    _RunTestProgram_BarMess / _RunTestProgram (Interface/InterfaceSYS.h:353-354, not
//                      included); SPEA Interface.exe is out of scope.  A SPEA machine sends no test signal.
//   T13 :1638-1639 :1895-1896   FormHS not in the closure.  HiSilicon arm-torque log not written (file write).
//   T14 :1754 :1779 :1792       TfBarCode has no GotBinFrom2DSortList / b2DIDIsInsideList /
//                      b2DIDIsInsideToErrorBin.  #else: iTempBin=iTestBinCount ("not in sorting list"),
//                      ret=0 ("not found"), ret=1 ("found in ERR list") -> error bin, never a fabricated good bin.
//   T15 :1768          EventReport / SECS_EVENT (SECSGEM/SecsEventReport.h:55, SecsEventType.h:340, not
//                      included).  CEID UnexpectedUNITIDRead not reported.
//   [LIFTED 20260926] T16 :1832  SAFETY  `if(TestTimeOut.Off())`: SetTestTimeOutTimer (below; pre-splice :10663) is still a no-op, so
//                      TestTimeOut is never armed and an unarmed TQPF_Timer is Off() at once (myTimer.cpp:14-17,
//                      :40-44).  Live, this arm would re-send the test signal on the first tick after every SOT
//                      (RS232 TTL path unconditionally, GPIB path whenever bExist is still false).  #else false ->
//                      no time-out re-send until golden :10747-10783 is translated.  LIFTED: SetTestTimeOutTimer is now
//                      golden 912 :10920-10955 verbatim, so the arm is golden again (TTL card 2/3: I12 -> Task 60 else 55;
//                      other interfaces: bExist==false -> Task 55).
//   T17 :1932-2183     user ruling: new machines use RS232Standard for TTL; the TTL_CARD_TYPE<2 direct-card
//                      branch (cases 100/200/250/260/265/270/300, incl. its SOFT_SIMULTE arms) is not ported.
//                      The dispatch arm :1459-1462 stays golden, so such a machine parks at Task=100.
//   T18 :2215          TfMain has no memoTSD (TSD time display only; the PordRec AddTSDTime loop stays live).
//   T19 :2279-2282     fTemp_Set (forms/fTemp_Set.h:1563, not included): four boost display writes.
//   T20 :2284          TfLotInfo has no SetATCOffset (same gate as aTester_Front.cpp G1).
//
//  FORCED NON-GATE EDIT
//   D1  :1720          `AnsiString(...->Strings[eErrorCode]).AnsiPos(...)`: vclcompat StringsProxy has no AnsiPos
//                      (vclcompat/TStringList.h:90-117).  Same bytes compared.
//
//  LIVE CALLS THAT LAND IN EXISTING V906 BODIES (not gates -- listed so nobody mistakes them for machine IO)
//   fMain->RunTestProgram(true, flag2) :1550   forms/fMain.h:261 `virtual bool RunTestProgram(bool bNeedTest,
//        bool *bSiteOnOff=NULL)`, forwarded through W906_TesterForward to THandlerTesterSide; not installed -> false.
//   fMain->BtnOneCycleClick(fMain) :1763       offline no-op (forms/fMain.cpp:404).
//   ADAM_WriteVoltage :1660 :1673 :1918         atester_shims.cpp:327 no-op: EP press-while-testing not applied.
//   bTestFailNeedWait (HTimer) :1147 :1159      atester_shims.h:467 stub (Off() always true).  Unreachable: golden
//        912 never sets bNeedWaitTemp true (golden cmydef.cpp:5725 =false, only `=false` writes), so :1140's arm is dead.
//   SW[SwTesterPower].On() :1138                real output through the IO HAL, as golden (every mode).
//
//  GOLDEN QUIRKS KEPT
//   Q1 :1048 :1064  two-argument AddTestResultRecord(iTestBinCount, "...") binds the text to SBin (golden and
//                   V906 both have only (int iBin, AnsiString SBin, AnsiString ErrorLog="")); ErrorLog stays "".
//   Q2 :1140-1175   dead if-arm (bNeedWaitTemp is never true in golden).
//   Q3 :1310 :1312  TestIF.dInitialStartDelayDec[4] is indexed with iAddInitStartDelayCT, which reaches 4 when
//                   TestIF.iEnStartDelayCount>4 (one past the end).
//   Q4 :1734        iBin is `unsigned int[4][8]` (:652 above); `iBin[i][j]==-1` compares with UINT_MAX.
//
//  RUNTIME REACH: live since P2b(b) 20260926 -- the golden callers ProcessTestResult (golden :2667 :2669 :2724 :2726;
//  called from aTester_Front.cpp / aTester_Rear.cpp / atester_32Site.cpp) and ProcessTesterTimeOut (:3687, and via
//  Check_TTL_Status :3733 :3770) are translated in this file (the old "GOLDEN VERBATIM gate" note was stale).
// ===========================================================================
bool GetTesterResult(int Type)
{
    static int iBarCodeMessSize=0;                                              //kevin 20160401  //JerryYang 20180629 (wei) : Mark掉,移到上一層函式
    static bool flag[MAX_SOCKET_ROW][MAX_SOCKET_COL];
    static bool bEPaddMSec=false;
    static bool bTestNeedWait=false;                                            //Eastsun 20260526 #026-1.6 Ifor 20250620 add:測試溫度Fail後延遲20秒
//    bool bWhitelistAlarm=false;
#if 0 // TODO(W906-GB-P2b): T01 `Byte` (Borland System.hpp typedef) is not declared by atester.cpp's include closure (V906 has it only in Interface/InterfaceSYS.h:36 and BinDisplay/MyBinDisp.h:490, neither included) -- golden atester.cpp:856
    static Byte BarCodeMess[2048];
#endif // T01
    static double dGetSec=0.0;
    static double dTestTime=0.0;
    static AnsiString StrBarCodeAll;

    int &Task=iTestTask, ret, iCH;
    int iEPaddSec=0;
    int iSiteOn[4]={0, 0, 0, 0};
    int iTempBin=0;
    int iSiteNo;                                                                //kevin 20140317
    DWORD dwTestTime=0;                                                         //jou 2013-09-25 Testing Need Stop All Motor
    char str2[256];
    bool bHasError=false;
    bool bGetTestArmTorque=false;
    bool flag2[MAX_SOCKET_TOTAL];
    ZeroMemory(flag2, sizeof(flag2));
    double dEPaddMSec=0.0;
    AnsiString StrBarCode[32];
    AnsiString Str, Str1, sLog;

    switch(Task)
    {
        case 1:
            bTestingStopAllMotor=false;                                         //jou 2013-09-25 Testing Need Stop All Motor
            iGetTestDataDelayTask=1;                                            // 2010/05/15 lee/joye
            bLBBoostTimeOut=false;
            extern void (*W906_SocketIDLogBody)();                              //AI(W906-W11) 20260927 (St02): T02 un-gated, Steven W11 = B -- golden atester.cpp:882 fStartCondition->SocketIDLog(); the body is St01's FileRW/StartCondition.cpp (writes the socket-ID CSV, W906_SOCKETIDLOG_ROOT), installed at wb_serve boot; not installed (ctest, before St01's line) = no call
            if(W906_SocketIDLogBody) W906_SocketIDLogBody();                    //JerryYang 20250120 : add
            // T02 (was #if 0 ... #endif; this line and the one above keep the line count)
            if(TestIF_File.bEnableReadAndCheckTorque)                           //kevin 20210804 change //jou 2012-06-12 即時更新扭力值不能開，不然會衝突
            {
                if(Type==1)                                                     //ARM2
                {
                    bNeedCheckIndexToque=false;
                    bNeedCheckIndexToque1=false;
                    fMain->chkReadTorque1->Checked=false;
                    fMain->chkReadTorque2->Checked=true;
                    fMain->edTorue1->Text="";
                    COM2->InitReadTorueTask();                                  //kevin 20211028 add HPCOM initial
                    bRetryReadToqu = false;                                     //kevin 20210419 重讀扭力
                }
                else
                {
                    bNeedCheckIndexToque=false;
                    bNeedCheckIndexToque1=false;
                    fMain->chkReadTorque1->Checked=true;
                    fMain->chkReadTorque2->Checked=false;
                    fMain->edTorue0->Text="";
                    COM2->InitReadTorueTask();                                  //kevin 20211028 add HPCOM initial
                    bRetryReadToqu = false;                                     //kevin 20210419 重讀扭力
                }
            }                                                                   //kevin 20211109 mark
            Task=10;                                                            //do not add break;
        case 10:
            for(int i=0; i<TestSocket.iMaxRow; i++)
            {
                for(int j=0; j<TestSocket.iMaxCol; j++)
                {
                    flag[i][j]=false;
                    iTesterBIN[i][j]=-1;
                    iRecordOldBin[i][j]=-1;
                    bDutflag[i*MAX_SOCKET_COL+j]=false;                                                                 //ChungHung 20140611 fix 紀錄測試的Dut位置 對應錯誤
                }
            }

            if((BAR_CODE_INSTALL!=ebctUninstall &&
                TestIF_File.bEnableBarCode) ||
               (INSTALL_OCR!=eocrUninstal && TestIF.bOcrFunction))                                                      //wei 20160922 增加OCR Function
            {
#if 0 // TODO(W906-GB-P2b): T01 BarCodeMess (T01) -- its only use -- golden atester.cpp:923
                memset(BarCodeMess, '\0', sizeof(BarCodeMess));
#endif // T01
                iBarCodeMessSize=0;
            }

            for(int i=0; i<MAX_SOCKET_TOTAL; i++)                                                                       //Jimmychiu 20231016 : 32 ->MAX_SOCKET_TOTAL
            {
                if(TestIF.iGpibMode==InterfaceType_SPEA_Type)                                                           //wei 20151120
                {
                    StrBarCode[i].sprintf("0,");
                }
                else
                {
                    if(CUSTOMER_CODE==CC_FMSH && BAR_CODE_INSTALL==ebcUseOCR)                                           //Ifor 20200924 add:AMD Barcode 資料改由GPIB 處理
                    {
                        StrBarCode[i].sprintf("NULL");
                    }
                    else
                    {
                        StrBarCode[i].sprintf("0");
                        fMain->tBarCodeList->Strings[i]="0";
                    }
                }
            }

            for(int i=0; i<TestSocket.iShtRow; i++)
            {
                for(int j=0; j<TestSocket.iShtCol; j++)
                {
                    if(TestSocket.Item[i][j]!=NULL_IC &&
                       TestSocket.Item[i][j]<TEST_PASS &&
                       TestSocket.Item[i][j]!=HAS_NULL_IC)
                    {
                        if(TestIF_File.iCloseSiteOnHPDontTest!=0 &&                                                     //JerryYang 20180726 (wei) 關site的位置有IC不測試送error bin
                           LastSet.bUseTestSocket[Type][i][j]==false)                                                   //Steven 20250604 : 關site的位置有IC不測試送指定 bin
                        {
//                            if(TestIF_File.iCloseSiteOnHPDontTest==2)           //不能在這邊設定bin, 會造成hang up
//                            {
//                                TestSocket.SetItemData(i, j, TEST_PASS+TestIF_File.iCloseSiteBin);
//                                TestSocket.iBinData[i][j]=TestIF_File.iCloseSiteBin;
//                                TestSocket.PordRec[i][j].AddTestResultRecord(TestIF_File.iCloseSiteBin, TestSocket.cSBin[i][j], "NonTestToSettedBin");
//                            }
//                            else
                            {
                                TestSocket.SetItemData(i, j, TEST_PASS+iTestBinCount);
                                TestSocket.iBinData[i][j]=iTestBinCount;
                                if(TestIF_File.iCloseSiteOnHPDontTest==2)
                                    TestSocket.PordRec[i][j].AddTestResultRecord(iTestBinCount, TestSocket.cSBin[i][j], "NonTestToSettedBin");
                                else
                                    TestSocket.PordRec[i][j].AddTestResultRecord(iTestBinCount, TestSocket.cSBin[i][j], "NonTestToRBin");
                            }
                        }
                        //==> Eastsun 20260526 #026-1.5 Ifor 20201022 add: KYEC QA Mode 結束後不把Error Bin 不測試的Bin不上Offline
                        else if(CUSTOMER_CODE==CC_KYEC_LEE && LastSet.iRunStartMode==rsmQAMode && bQAModeFinishCleanOut==true && Prod.iQAModeRunType==3)
                        {
                            TestSocket.SetItemData(i, j, TEST_PASS+iTestBinCount);
                            TestSocket.iBinData[i][j]=iTestBinCount;
                            TestSocket.PordRec[i][j].AddTestResultRecord(iTestBinCount, TestSocket.cSBin[i][j], "NonTestToRBin");
                        }
                        //<== Eastsun 20260526 #026-1.5
                        else
                        {
                            if((CosFunction.bBarcodeErrNoTestAndShowH==true ||                                          //jou 20191007 : Barcode Error No Test & Show "H"
                                TestIF_File.iNoCodeDeviceToErr==2) &&                                                   //Steven 20200909 : 將2DID all site fail變成選項
                               TestIF_File.bEnableBarCode==true &&
                               (TestSocket.cDeviceInf[i][j]==asBarCodeErrorSend ||
                                TestSocket.cDeviceInf[i][j]==""))
                            {
                                TestSocket.SetItemData(i, j, TEST_PASS+iTestBinCount);
                                TestSocket.iBinData[i][j]=iTestBinCount;
                                if(LastSet.iTester==_2D_SORT)                                                           //JerryYang 20230803 : 2D SORT區分ERROR原因
                                {
                                    TestSocket.PordRec[i][j].AddTestResultRecord(iTestBinCount, TestSocket.cSBin[i][j], "BarcodeReadError");
                                }
                                else
                                {
                                    TestSocket.PordRec[i][j].AddTestResultRecord(iTestBinCount, TestSocket.cSBin[i][j], "NonTestToRBin");
                                }
                            }
                            else
                            {
                                flag[i][j]=true;
                            }
                        }

                        if(TestIF.iSiteMap[i][j]!=-1 &&
                           TestIF.iSiteMap[i][j]!=0)                                                                    //kevin 20141013  ChungHung 20140616 沒卡掉會溢位
                        {
                            if((CosFunction.bBarcodeErrNoTestAndShowH==true ||                                          //jou 20191007 : Barcode Error No Test & Show "H"
                                TestIF_File.iNoCodeDeviceToErr==2) &&                                                   //Steven 20200909 : 將2DID all site fail變成選項
                               TestIF_File.bEnableBarCode==true &&
                               (TestSocket.cDeviceInf[i][j]==asBarCodeErrorSend ||
                                TestSocket.cDeviceInf[i][j]==""))
                            {
                                //bDutflag[TestIF.iSiteMap[i][j]-1]=false;
                            }
                            else
                            {
                                bDutflag[TestIF.iSiteMap[i][j]-1]=true;                                                 //ChungHung 20140611 fix Dut 紀錄 對應錯誤
                            }

                            if(LastSet.iRunStartMode==rsmAutoSiteMap && bSiteMappingCHKOK==false)
                            {
                                iAutoSiteRecordIC[0]=i;                                                                 //kevin 20150114   記錄目前有ic位置
                                iAutoSiteRecordIC[1]=j;                                                                 //kevin 20150114
                            }
                        }

                        //gpib 傳過來是  0 2 4 6 8
                        //               1 3 5 7 9
                        if(IniConfig.bNewResetFunction)
                        {
                            TestSocket.SetItemData(i, j, HAS_TESTING_IC);
                        }
                    }

                    if(CUSTOMER_CODE==CC_Murata &&
                       IniConfig.bN23_1_Enable2DIDCompare)                                                              //Steven 20200611 : for Murata, 2DID NG不測試
                    {
                        if(TestSocket.Item[i][j]!=NULL_IC &&
                           TestSocket.Item[i][j]!=HAS_NULL_IC &&
                           TestSocket.b2DIDNG[i][j]==true)                                                              //Steven 20200611 : for Murata, 2DID NG不測試
                        {
                            flag[i][j]=false;
                            TestSocket.SetItemData(i, j, TEST_PASS+iTestBinCount);
                            TestSocket.iBinData[i][j]=iTestBinCount;
                            TestSocket.PordRec[i][j].AddTestResultRecord(iTestBinCount, "NonTestToRBin");  //AI(W906-GB-P2b) 20260926: golden 912 atester.cpp:1048 (added in 912)
                        }
                    }

                    if(TestIF_File.bEnableBarCode==true &&  //AI(W906-GB-P2b) 20260926: golden 912 atester.cpp:1052-1067 (block added in 912)
                       TestIF_File.bEnableBarcodeCSVCompare==true)              //Ifor 20260511 add: Barcode CSV Compare - Pass Bin skip test
                    {
                        if(TestSocket.Item[i][j]!=NULL_IC       &&
                           TestSocket.Item[i][j]!=HAS_NULL_IC   &&
#if 0 // TODO(W906-GB-P2b): T03 TfBarCode (BarCode/BarCode.h:40-88) has no DoBarcodeCSVCompare -- golden atester.cpp:1057
                           (fBarCode->DoBarcodeCSVCompare(TestSocket.cDeviceInf[i][j])==1))
#else
                           false)                                               //AI(W906-GB-P2b) 20260926: T03 golden 912 atester.cpp:1057 -- unknown CSV verdict -> "not in CSV": the device is tested (no WAR04217 skip)
#endif // T03
                        {

                            ShowErrorMessage("WAR04217", K_SKIP, MMSystem, false);
                            flag[i][j]=false;
                            TestSocket.SetItemData(i, j, TEST_PASS+iTestBinCount);
                            TestSocket.iBinData[i][j]=iTestBinCount;
                            TestSocket.PordRec[i][j].AddTestResultRecord(iTestBinCount, "CSVComparePassToErr");

                        }
                    }
                    if((BAR_CODE_INSTALL!=ebctUninstall && TestIF.bEnableBarCode) ||
                       (INSTALL_OCR!=eocrUninstal && TestIF.bOcrFunction))
                    {
                        iSiteNo=TestIF.iSiteMap[i][j]-1;
                        if(iSiteNo>=0)
                        {
                            if(flag[i][j])
                            {
                                if(TestIF.iGpibMode==InterfaceType_SPEA_Type)                                           //wei 20151120
                                {
                                    StrBarCode[iSiteNo].sprintf("%s,", TestSocket.cDeviceInf[i][j]);
                                }
                                else
                                {
                                    if(TestSocket.Item[i][j]!=NULL_IC     &&
                                       TestSocket.Item[i][j]<TEST_PASS    &&
                                       TestSocket.Item[i][j]!=HAS_NULL_IC &&
                                       (TestSocket.cDeviceInf[i][j]=="" ||
                                        TestSocket.cDeviceInf[i][j]=="0"))
                                    {
                                        TestSocket.cDeviceInf[i][j]=asBarCodeErrorSend;                                 //wei 20160318 Barcode Error依客戶設定
                                    }
                                    StrBarCode[iSiteNo].sprintf("%s", TestSocket.cDeviceInf[i][j]);
                                }
                            }
                            else
                            {
                                if(TestIF.iGpibMode==InterfaceType_SPEA_Type)                                           //wei 20151120
                                {
                                    StrBarCode[iSiteNo].sprintf("0,");
                                }
                                else
                                {
                                    if(TestSocket.Item[i][j]!=NULL_IC     &&
                                       TestSocket.Item[i][j]<TEST_PASS    &&
                                       TestSocket.Item[i][j]!=HAS_NULL_IC &&
                                       TestSocket.cDeviceInf[i][j]=="")
                                    {
                                        StrBarCode[iSiteNo].sprintf(asBarCodeErrorSend.c_str());                        //wei 20160318 Barcode Error依客戶設定
                                    }
                                    else
                                    {
                                        if(CUSTOMER_CODE==CC_FMSH &&
                                           BAR_CODE_INSTALL==ebcUseOCR)                                                 //Ifor 20200924 add:AMD Barcode 資料改由GPIB 處理
                                        {
                                            StrBarCode[iSiteNo].sprintf("NULL");                                        //Ifor 20210906 OCR 尾數盤異常 i=>iSiteNo
                                        }
                                        else
                                        {
                                            StrBarCode[iSiteNo].sprintf("0");
                                        }
                                    }
                                }
                            }
                        }
                        else
                        {
                            //StrBarCode[iSiteNo].sprintf("0,");   記憶體溢位
                        }
                    }
                }
            }

            StrBarCodeAll="";
            for(int i=0; i<MAX_SOCKET_TOTAL; i++)                                                                       //Jimmychiu 20231016 : 32 ->MAX_SOCKET_TOTAL
            {
                StrBarCodeAll+=StrBarCode[i];
                fMain->tBarCodeList->Strings[MAX_SOCKET_TOTAL-1-i]=StrBarCode[i];                                       //wei 20151120   //Jimmychiu 20231016 : 31 ->MAX_SOCKET_TOTAL-1
            }

            SW[SwTesterPower].On();
            //==> Eastsun 20260526 #026-1.6 Ifor 20250620 add:測試溫度Fail後延遲N秒避免快速重測
            if(Temperature.bUseTestTimeBelowNeedDelay==true && bNeedWaitTemp==true)
            {
                int WaitTimer_Start=0;
                if(bTestNeedWait==false)
                {
                    bTestNeedWait=true;
                    iATCTempWaitTimer=Temperature.dTestBelowDelayTime;
                    bTestFailNeedWait.SetSecAndOn(iATCTempWaitTimer);
                    iATCTempWaitTimer_Start=MyTickCount();
                    WaitTimer_Start=180000-(iATCTempWaitTimer*1000);
                    iATCTempWaitTimer_Start=iATCTempWaitTimer_Start-WaitTimer_Start;

                    Str.sprintf("Test Fail Need Wait %d Sec!", iATCTempWaitTimer);
                    RecordProcess(Str);
                    bCheckATCTemp=true;
                }

                if(bTestNeedWait==true)
                {
                    if(bTestFailNeedWait.Off()==true)
                    {
                        bTestNeedWait=false;
                        bNeedWaitTemp=false;
                        bCheckATCTemp=false;
                    }
                    else
                    {
                        break;
                    }
                }
            }
            else
            {
                bTestNeedWait=false;
                bNeedWaitTemp=false;
            }
            //<== Eastsun 20260526 #026-1.6
            if(TestSocket.HasRealIC())                                                                                  //Steven 20190313 : No IC no delay
            {
                if(Prod.bUseOtherArmToTestAfterInitialDelay==false)
                    CheckInitialStartDelayInSocket();                                                                   //JerryYang 20180629 (wei) : Initial delay判斷包成函式
            }

            bATC_SITE_2ND_CHECK[Type]=true;                                                                             //Ifor 20160516 add Index 測試前開啟 ATC 第二點溫度偵測

            for(int i=0; i<iATC_Use_Heat_Count; i++)                                                                    //Ifor 20160516 修改ATC Heat 設定數
            {
                if(Type==0)
                {
                    if(i<(iATC_Use_Heat_Count/2))
                        bATCSiteTest[i]=bATC_EnablesChannel[i];
                    else
                        bATCSiteTest[i]=false;
                }
                else
                {
                    if(i>=(iATC_Use_Heat_Count/2))
                        bATCSiteTest[i]=bATC_EnablesChannel[i];
                    else
                        bATCSiteTest[i]=false;
                }
            }

            if(Temperature.bBoostFuncttion || Temperature.bLBTempFunction)
            {
                iTriggerBoostFunction=CheckToBoostIndexTemp();
            }

            #ifdef SOFT_SIMULTE
                iTriggerBoostFunction=-1;
            #endif

#if 0 // TODO(W906-GB-P2b): T04 ATC_InterfaceForm is declared only in acarry_shims.h:115 (TATC_InterfaceFormShim, iATC_MODE_TYPE only, offline 0) and ATC_TYPE_60/70 only in forms/fATCHandlerSide.h:695/697; neither header is included -- golden atester.cpp:1212
            if(ATC_InterfaceForm->iATC_MODE_TYPE==ATC_TYPE_70 &&
#else
            if(false &&                                                         //AI(W906-GB-P2b) 20260926: T04 -- the shim value (0) is not ATC_TYPE_70, so this conjunct is false: the else-if arms stay golden
#endif // T04
               Temperature.bATC7TSDFunction==true &&                                                                    //Steven 20160604 : by site TSD
               LastSet.iRealDummy==REALLY &&
               LastSet.iTester==ON_LINE)                                                                                //Steven 20161025 : On Line && Has IC才跑TSD
            {
                MyTSDTimer.LatchCycleTime(true);                                                                        //Steven 20161208 : Record TSD time data
                Task=20000;
                break;
            }
            else if((Temperature.bBoostFuncttion ||
                     Temperature.bLBTempFunction) &&
                    iTriggerBoostFunction!=-1)                                                                          //Steven 20180817 : Boost Function
            {
                Task=30000;
                iTriggerBoostFuncBack=-1;
                Str.sprintf("Trigger boost duration function! (%d)", iTriggerBoostFunction);
                RecordProcess(Str);
#if 0 // TODO(W906-GB-P2b): T05 fContact is TfContactShim (atester_shims.h:154-250): no labDelayStatus / lblCountDown / pnlHandlerSatus (the real TfContact, forms/fContact.h:1177-1179, has no `fContact` global); brace-less if goes with its statement -- golden atester.cpp:1229-1230
                if(fContact->fShow)
                    fContact->labDelayStatus->Caption="Start boost function";
#endif // T05
                break;
            }
            else
            {
                Task=20;
            }
        case 20:
            HangTime.SetSecAndOn(Prod.iHangupMaxTime);  //JerryYang 20230803 : fix initial start delay 太長誤發hang up

            if(SystemStart==false)  //JerryYang 20180828 (Steven) : fix 按暫停卻送出測試訊號
            {
                SetTestTimeOutTimer(Type);  //JerryYang 20200623 避免誤發timeout
                break;
            }

            if(bUseInitTempOffset &&
               IniConfig.bL28TempOfsUseReadyTempRange &&
               bEnable_KLT_Function==false &&
               fHeaterOK==false &&
               Temperature.iTempReadyRange!=0 &&  //Sam 20231214 : Temp offset use ready temp range
              (iInitContactCount<(Temperature.iCintactCntForTempOffsetAtInitial+Temperature.iCintactDelayCntForInitTempOffset)))
            {
                break;
            }

            if(TestSocket.HasRealIC())  //Steven 20190313 : No IC no delay
            {
                if(bUseInitDelay &&  //Steven 20171219 (Wei) : 只有加熱模式要跑Initial Delay
                   bNeedInitialTestDelay)
                {
                    if(NeedResetInitialDelay()==true)  //JerryYang 20180828 (Steven) : 預熱過程中又觸發預熱的話, 就重新執行預熱
                    {
                        dwStartInitialCount.LatchCycleTime(true);
                        break;
                    }

                    if(iInitialCount>0)  //ChungHung 20140801 add Korea Want to count down in main status
                    {
                        iInitialCount=Prod.iInitialDelay-(dwStartInitialCount.LatchCycleTime()/1000);  //kevin 20181102 (Steven) : HangUp卡在case20 加入絕對值保護
                        break;
                    }
                }
                else
                {
                    iInitialCount=0;
                }
                bNeedInitialTestDelay=false;

                if(bHPCleanout)  //wei 20160624 Hotplate clean out
                {
                    HTestDeley.SetSecAndOn(0.1);
                }
                else if(IniConfig.bInitialStartDelayCount)  //jou 2012-11-30 高溫動作下希望增加顆數記數,在前幾顆下壓到Socket後,都要等待Delay time
                {
                    if(bUseInitDelay)
                    {
                        if(TestIF.iEnStartDelayCount!=0)
                        {
                            iInitStartDelayTimeCT++;

                            if(iInitStartDelayTimeCT>TestIF.iStartDelayCount[iAddInitStartDelayCT])  //kevin 20180307
                            {
                                if(iAddInitStartDelayCT<4)
                                    iAddInitStartDelayCT++;  //kevin 20180307 add delay count InitStartDelayTime
                                iInitStartDelayTimeCT=1;
                                iInitStartDelayDec=0;
                            }

                            if(bFinishInitStartDelay==false &&
                               (iAddInitStartDelayCT<TestIF.iEnStartDelayCount) &&
                               (iInitStartDelayTimeCT<=TestIF.iStartDelayCount[iAddInitStartDelayCT]))  //kevin 20180307 change parm
                            {
                                if(CosFunction.bHaveIndexContactDelay &&
                                   IniConfig.bD29EnableIndexContactDelay)  //Steven 20140519 : [D29] Index下壓後多Delay 0.4秒 (For SPIL Low Yield)
                                {
                                    HTestDeley.SetSecAndOn(TestIF.dInitStartDelayTime+0.4);
                                }
                                else
                                {
                                    if(TestIF.dInitialStartDelayDec[iAddInitStartDelayCT]!=0)  //kevin 20180307 change parm     //kevin 20161214 (Steven) 第幾個 Count開始執行送訊號delay
                                    {
                                        iInitStartDelayDec=TestIF.dInitStartDelayTime+(iInitStartDelayTimeCT*TestIF.dInitialStartDelayDec[iAddInitStartDelayCT]);  //kevin 20180307 change parm
                                        if(iInitStartDelayDec<=0)
                                            iInitStartDelayDec=0;
                                    }
                                    else
                                    {
                                        iInitStartDelayDec=TestIF.dInitStartDelayTime;
                                    }
                                }
                                bInitStartDelayTime=true;  //wei 20171020 (jou) InitStartDelayTime秒數倒數
                            }

                            if(iAddInitStartDelayCT>=4)
                            {
                               bFinishInitStartDelay=true;  //kevin 20180308 add 動作完成
                               iAddInitStartDelayCT=0;
                            }
                            iInitStartDelayCount=iInitStartDelayDec;
                            dwStartInitialCount.LatchCycleTime(true);
                            HTestDeley.SetSecAndOn(iInitStartDelayDec);
                        }
                        else
                        {
                            if(iInitStartDelayTimeCT<TestIF.iInitStartDelayTimeCT)
                            {
                                iInitStartDelayTimeCT++;
                                if(CosFunction.bHaveIndexContactDelay &&
                                   IniConfig.bD29EnableIndexContactDelay)  //Steven 20140519 : [D29] Index下壓後多Delay 0.4秒 (For SPIL Low Yield)
                                {
                                    HTestDeley.SetSecAndOn(TestIF.dInitStartDelayTime+0.4);
                                }
                                else
                                {
                                    HTestDeley.SetSecAndOn(TestIF.dInitStartDelayTime);
                                }
                                bInitStartDelayTime=true;  //wei 20171020 (jou) InitStartDelayTime秒數倒數
                                dwStartInitialCount.LatchCycleTime(true);
                                iInitStartDelayCount=TestIF.dInitStartDelayTime;
                            }
                            else
                            {
                                if(CosFunction.bHaveIndexContactDelay && IniConfig.bD29EnableIndexContactDelay)  //Steven 20140519 : [D29] Index下壓後多Delay 0.4秒 (For SPIL Low Yield)
                                    HTestDeley.SetSecAndOn(Prod.dTesterStartDelayTime+0.4);  //Steven 20140620 : Fix Start Delay Time
                                else
                                    HTestDeley.SetSecAndOn(Prod.dTesterStartDelayTime);
                            }
                        }
                    }
                    else
                    {
                        if(CosFunction.bHaveIndexContactDelay && IniConfig.bD29EnableIndexContactDelay)  //Steven 20140519 : [D29] Index下壓後多Delay 0.4秒 (For SPIL Low Yield)
                            HTestDeley.SetSecAndOn(Prod.dTesterStartDelayTime+0.4);  //Steven 20140620 : Fix Start Delay Time
                        else
                            HTestDeley.SetSecAndOn(Prod.dTesterStartDelayTime);
                    }
                }
                else
                {
                    if(CosFunction.bHaveIndexContactDelay && IniConfig.bD29EnableIndexContactDelay)  //Steven 20140519 : [D29] Index下壓後多Delay 0.4秒 (For SPIL Low Yield)
                        HTestDeley.SetSecAndOn(Prod.dTesterStartDelayTime+0.4);  //Steven 20140620 : Fix Start Delay Time
                    else
                        HTestDeley.SetSecAndOn(Prod.dTesterStartDelayTime);  //等 motor 穩定 時間 , 應該加到別處 ?????????????
                }
            }
            else
            {
                HTestDeley.SetSecAndOn(0.1);
            }

            if(IniConfig.bG12ContractModeManualMessage)  //kevin 20180222 Arm 1 Arm2 吸取IC 做CONTRACT MODE
            {
                bContractModeTest=true;  //kevin 20180222 contract mode 秀手動送測試訊號
#if 0 // TODO(W906-GB-P2b): T06 WaitManualStartKey is declared in ckernel.h:96, which atester.cpp does not include -- golden atester.cpp:1384
                if(WaitManualStartKey()==false)
#else
                if(true)                                                        //AI(W906-GB-P2b) 20260926: T06 -- key never reported pressed: keep waiting (never auto-grant a manual START; TfContactShim bSetupStart precedent, atester_shims.h)
#endif // T06
                    return false;
                bContractModeTest=false;  //kevin 20180222 contract mode 秀手動送測試訊號
            }

            if(IniConfig.bVTESTFunction==true &&  //jou 20230621 : VTEST Handler即時監控 GetRcsCheckingResult
               IniConfig.bGetRcsCheckingResult==true)
            {
#if 0 // TODO(W906-GB-P2b): T07 fMesSystem is declared in forms/fMesSystem.h:715, which atester.cpp does not include -- golden atester.cpp:1392
                if(fMesSystem->GetRcsCheckingResult(false)==false)
#else
                if(LastSet.iRunStartMode!=rsmAutoSiteMap)                       //AI(W906-GB-P2b) 20260926: T07 -- golden early-out (Mes/fVATMesFileSys.cpp:3085-3089: true in AutoSiteMap) kept; otherwise the RCS verdict is unknown -> treated as FAIL (break), which is also golden's own SOFT_SIMULTE answer (asData="FAIL")
#endif // T07
                    break;
            }

#if 0 // TODO(W906-GB-P2b): T08 TfAutomationShim (atester_shims.h:313-333) has no HANARMSRunCheckOK -- golden atester.cpp:1396
            if(fAutomation && fAutomation->HANARMSRunCheckOK(true)==false)      //RogerYang 20260902 : A76 HANA RMS recipe 互鎖
#else
            if(fAutomation && (CUSTOMER_CODE==CC_HANA_MICRON && IniConfig.bA77_EnableHanaRMSInterlock==true && LastSet.iRunStartMode!=rsmAutoSiteMap))   //AI(W906-GB-P2b) 20260926: golden 912 atester.cpp:1396-1398 (added in 912); T08 -- golden early-out (Automation/automation.cpp:3139-3142) kept: true on every non-A77-HANA machine; on an A77 HANA machine the recipe check cannot run -> FAIL (break)
#endif // T08
                break;

            Task=50;
        case 50:
            if(bInitStartDelayTime)                                             //wei 20171020 (jou) InitStartDelayTime秒數倒數
            {
                if(iInitStartDelayCount>0)
                {
                    if(TestIF.iEnStartDelayCount!=0)
                    {
                        if(bFinishInitStartDelay==false)                        //kevin 20180308
                            iInitStartDelayCount=iInitStartDelayDec-dwStartInitialCount.LatchCycleTime()/1000;
                        else
                            iInitStartDelayCount=TestIF.dInitStartDelayTime-dwStartInitialCount.LatchCycleTime()/1000;
                    }
                    else
                    {
                        iInitStartDelayCount=TestIF.dInitStartDelayTime-dwStartInitialCount.LatchCycleTime()/1000;
                    }
                    break;
                }
            }

            if(HTestDeley.Off())
            {
                if(fMain->pgMain->ActivePageIndex==1 &&
#if 0 // TODO(W906-GB-P2b): T09 TfMain (forms/fMain.h) has no pcCommView / pgcTorque -- golden atester.cpp:1423-1424
                   fMain->pcCommView->ActivePageIndex==0 &&
                   fMain->pgcTorque->ActivePageIndex==1)                        //Steven 20210524 : 連續讀取扭力
#else
                   false)                                                       //AI(W906-GB-P2b) 20260926: T09 -- torque tab not shown (offline pgMain->ActivePageIndex is 0, so golden is false here too)
#endif // T09
                {
                    iCheckTorqueCount++;
                }

                if(fContact->fShow==false)
                    bInitStartDelayNotFinish=false;                             //Ifor 20181220 : add Init Start Delay Time Not Finish

                if(ATC_SYSTEM==eNewATCSystem &&
                   Temperature.bATCActiveCooling==true)                         //Ifor 20190130 add 測試前送出Start/End Test 訊息給 ATC
                {
#if 0 // TODO(W906-GB-P2b): T04 StartTesting (T04) -- golden atester.cpp:1435
                    ATC_InterfaceForm->StartTesting();                          //JerryYang 20220815 : send ATC start testing
#endif // T04
                }

                if(CheckAndRecodrEP(Type))                                      //Steven 20190114 : EP Alarm換位置
                {
                    SetTestTimeOutTimer(Type);                                  //Steven 20230216 : 避免誤發timeout
                    break;
                }

                if(TestIF.iTestType==GPIB_MODE ||
                   LastSet.iTester==OFF_LINE ||
                   TestIF.iTestType==TCP_IP_MODE)                               //wei 20211027 open short TCP/IP
                {
                    if(W906_TesterBridgeFound()==true)      //AI(W906-GB-P2b) 20260926: T10 lifted -- golden `fMain->bFind==true`; bFind lives in THandlerTesterSide, read through the fMain.h seat (not installed = false = golden "bridge not found")
                    {
                        Task=55;
                    }
                    else
                    {
                        TestTimeOut.SetSecAndOn(3);
                        Task=9999;
                        break;
                    }
                }
                else if(TestIF.iTestType==TTL_MODE && (TTL_CARD_TYPE<2))        //Isaac 20200903 :TTL RS232通訊  //Isaac 20210309 :TTL RS232兩塊板子
                {
                    Task=100;  //AI(W906-GB-P2b) 20260926: T17 -- cases 100..300 are gated (user ruling): a TTL_CARD_TYPE<2 machine parks here
                }
                else                                                            //if(TestIF.iTestType==RS232_MODE)  //TTL_CARD_TYPE==2     //Isaac 20200903 :TTL RS232通訊
                {
                    if(IniConfig.bC04EnableTestTempIC)
                    {
                        Task=500;
                        iTempICTask=1;
                    }
                    else
                    {
                        Task=55;                                                //Steven 20100721
                    }
                }
            }

            if(Task!=55)
                break;
//GPIB mode start
        case 55:
            if(SoftStop)
                break;

            if(W906_TesterBridgeFound()==false)                                                                                     //kevin 20150428 避免GPIB關掉   //AI(W906-GB-P2b) 20260926: T10 lifted, as above
            {
                iSiteNo=0;
                break;
            }

            if(TestSocket.AlreadyTestNotIncludeErrorBin()==false ||                                                     //Steven 20200612 : 修正避免連續測兩次
               (Prod.bD22SupportMultiDoubleContact &&                                                                   //Steven 20201024 : Fixed for double contact
                 //IniConfig.bD22VerifyMode) &&                                 //Sam 20231117 : 整合到 QA 模式  //Sam 20221012 : 新增 VerifyMode 功能
                bDoubleContact))
            {
                IsTest=true;
                if(IniConfig.bI31_1GPIBLotEnd)                                                                          //wei 20160624 GPIB Lot End Command    //wei 20160726 TSMC GPIB Lot End
                    bGPIBLotEndCommand=true;
                else
                    bGPIBLotEndCommand=false;
                bGPIBLotStartCommand=true;                                                                              //kevin 20190613 add
                if(TestSocket.HasRealIC())                                                                              //Steven 20210218 : 修正測試時間的紀錄
                    RecordStartTestTime();

                if(DeviceForm.ContactMode==DropContact)                                                                 //JerryYang 20170503 (wei) 第三段drop contact計時,到達contact高度後吸真空+start delay
                    fObserver->AddTimeData(20, DropContactTimer3.LatchCycleTime()/1000.0);
                if(ATC_SYSTEM==eATC60 || ATC_SYSTEM==eATC30)                                                            //20141204 ChungHung add for ATC3.0  //2014-05-30    Dell    for ATC6.0
                {
#if 0 // TODO(W906-GB-P2b): T11 ATCInterfaceForm is declared in ATC/ATCInterface.h:528, which atester.cpp does not include (same gate as aTester_Front.cpp G4) -- golden atester.cpp:1508
                    ATCInterfaceForm->ATC_60_SYS.SendTestStart(1);
#endif // T11
                }

#if 0 // TODO(W906-GB-P2b): T04 SiteTesting (T04); the brace-less if goes with its statement -- golden atester.cpp:1511-1512
                if(ATC_SYSTEM==eNewATCSystem)                                                                           //Ifor 20160516 修改ATC Heat 設定數
                    ATC_InterfaceForm->SiteTesting(iATC_Use_Heat_Count, bATCSiteTest);
#endif // T04

                for(int i=0; i<MAX_SOCKET_TOTAL; i++)
                {
                    #ifdef ASE_KaohSiung
                    if(IniConfig.bUseAutoSiteMapping && LastSet.iRunStartMode==rsmAutoSiteMap)                          //kevin 20150113
                    {
                        if(TestIF.iSiteMap[i%MAX_SOCKET_ROW][i/MAX_SOCKET_ROW]-1>=0)
                            *(flag2+(TestIF.iSiteMap[i%MAX_SOCKET_ROW][i/MAX_SOCKET_ROW]-1))=(TestIF_File.iSiteMap[i%MAX_SOCKET_ROW][i/MAX_SOCKET_ROW]>0);
                    }                                                                                                   //送全部測試資料 好判斷 測試機 測試順序
                    else
                    {
                        if(TestIF.iSiteMap[i%MAX_SOCKET_ROW][i/MAX_SOCKET_ROW]-1>=0)
                            *(flag2+(TestIF.iSiteMap[i%MAX_SOCKET_ROW][i/MAX_SOCKET_ROW]-1))=flag[i%MAX_SOCKET_ROW][i/MAX_SOCKET_ROW];
                    }
                    #else
                        if(TestIF.iSiteMap[i%MAX_SOCKET_ROW][i/MAX_SOCKET_ROW]-1>=0)
                            *(flag2+(TestIF.iSiteMap[i%MAX_SOCKET_ROW][i/MAX_SOCKET_ROW]-1))=flag[i%MAX_SOCKET_ROW][i/MAX_SOCKET_ROW];
                    #endif
                }

                if(TestIF.iGpibMode==InterfaceType_SPEA_Type)
                {
                    if((BAR_CODE_INSTALL==ebctInShtIntel ||
                        BAR_CODE_INSTALL==ebctUseCCDMode ||
                        BAR_CODE_INSTALL==ebctEtherNetCCD ||                                                            //Ifor 20190129 : add Cognex EtherNet 通訊
                        BAR_CODE_INSTALL==ebcUseOCR) &&                                                                 //Ifor 20210407 add: 自製OCR
                        TestIF_File.bEnableBarCode)
                    {
#if 0 // TODO(W906-GB-P2b): T12 _RunTestProgram_BarMess / _RunTestProgram are declared in Interface/InterfaceSYS.h:353-354 (not included); SPEA Interface.exe path is out of scope (TesterComm/Handler/TRANSLATION_RULES.md rule 5) -- golden atester.cpp:1541
                        _RunTestProgram_BarMess(sizeof(flag2), flag2, StrBarCodeAll.Length(), StrBarCodeAll.c_str());
#endif // T12
                    }
                    else
                    {
#if 0 // TODO(W906-GB-P2b): T12 _RunTestProgram (T12) -- golden atester.cpp:1545
                        _RunTestProgram(sizeof(flag2), flag2);
#endif // T12
                    }
                }
                else
                {
                    fMain->RunTestProgram(true, flag2);
                }

                if(ATC_SYSTEM==eATCHonPrecType)                                                                         //Steven 20120410 : Hontech ATC
                {
#if 0 // TODO(W906-GB-P2b): T11 ATCInterfaceForm (T11) -- golden atester.cpp:1555
                    ATCInterfaceForm->SendTestStart(Type);
#endif // T11
                }
            }
            else                                                                                                        //Steven 20200618 : 紀錄被連續測兩次的Bin
            {
                Str="";
                for(int i=0; i<TestSocket.iShtRow; i++)
                {
                    for(int j=0; j<TestSocket.iShtCol; j++)
                    {
                        if(TestSocket.Item[i][j]!=NULL_IC &&
                           TestSocket.Item[i][j]!=HAS_NULL_IC)
                        {
                            Str1.sprintf("Site[%d][%d]=%d,", i, j, TestSocket.Item[i][j]);
                            Str+=Str1;
                        }
                    }
                }
                RecordProcess(Str);
            }

            SetTestTimeOutTimer(Type);                                                                                  //JerryYang 20200623 避免誤發timeout
            SOTPauseIntervalsTime.LatchCycleTime(true);                                                                 //kevin 20181102 (Steven) : 取得機台停止時間
            bSOTToLongStopBlowAir=true;                                                                                 //kevin 20181102 (Steven) : add 上次測試訊號太久 需停止吹氣

            bEPaddMSec=false;                                                                                           //jou 20171026 (wei) : 測試中加壓EP
            TestStopMotorTimer.LatchCycleTime(true);
            if(LastSet.iTester==OFF_LINE)
            {
                IsTest=true;                                                                                            //Steven 20130703
                iCurrentTime=0;
                NULL_Delay.SetSecAndOn(Prod.iTesterDummyTime);
                Task=65;
                break;
            }
            else
            {
                Task=60;
            }
        case 60:
            if(bPauseTester)
            {
                bPauseTester=false;
                SetTestTimeOutTimer(Type);                                                                              //Steven 20200407 : 整合Time Out時間設定
            }

            if(IniConfig.bEnableTestingNeedStopAllMotor==true &&
               IniConfig.bI24TestingNeedStopAllMotor==true)                                                             //jou 2013-09-25 Testing Need Stop All Motor
            {
                dwTestTime=TestStopMotorTimer.LatchCycleTime()/100.0;
                if(dwTestTime>=(unsigned int)((TestIF.dInitWaitTime-0.5)*10) && dwTestTime<(unsigned int)((TestIF.dInitWaitTime+TestIF.dTestingWaitTime)*10))
                {
                    bTestingStopAllMotor=true;                                                                          //jou 2013-09-25 Testing Need Stop All Motor
                }
                else
                {
                    bTestingStopAllMotor=false;                                                                         //jou 2013-09-25 Testing Need Stop All Motor
                }
            }

            if(CosFunction.bUploadTestArmTorqueLog)                                                                     //Ifor 20190912 :add 海思 V02.30 版 Record Torque
            {
                bGetTestArmTorque=false;
                if(bFrontTestArmTorqueFinish==false)
                {
                    if(fMain->edTorue0->Text!="")
                    {
                        bFrontTestArmTorqueFinish=true;
                        bGetTestArmTorque=true;
                    }
                }

                if(bRearTestArmTorqueFinish==false)
                {
                    if(fMain->edTorue1->Text!="")
                    {
                        bRearTestArmTorqueFinish=true;
                        bGetTestArmTorque=true;
                    }
                }

                if(bGetTestArmTorque==true)
                {
#if 0 // TODO(W906-GB-P2b): T13 FormHS is not declared by atester.cpp's include closure (only Automation/SCK_ART_Remainder.h:629 / forms/fHS.h); RecordArmTestInfoLog_HS also writes a log file -- golden atester.cpp:1638-1639
                    sArmTestInfoEvenLogFile=FormHS->GetLastFileLogName_HS(5);
                    FormHS->RecordArmTestInfoLog_HS(sArmTestInfoEvenLogFile);
#endif // T13
                }
            }

            if(IniConfig.bIndexAddPressEP==true)                                                                        //jou 20171026 (wei) : 測試中加壓EP
            {
                bTestEPaddKg=true;
                dTestTime=StrToFloat(FormatFloat("0.0", double(TestStopMotorTimer.LatchCycleTime())/1000.0));

                if(dGetSec!=dTestTime && dTestTime<=IniConfig.iIndexAddPressEP_Time && dTestTime>0)
                {
                    if(DeviceForm.fAireForce!=0)
                    {
                        dGetSec=dTestTime;
                        iEPaddSec=dTestTime*10.0/10.0;
                        bEPaddMSec=!bEPaddMSec;
                        if(bEPaddMSec==true)
                            dEPaddMSec=+IniConfig.dIndexVibrateEP_Kg;
                        else
                            dEPaddMSec=-IniConfig.dIndexVibrateEP_Kg;

                        ADAM_WriteVoltage(DeviceForm.fAireForce+iEPaddSec*IniConfig.dIndexAddPressEP_Kg+dEPaddMSec);
                    }
                }
            }

            if(bEcho &&
               bTimeOutForNoFullSite==false)                                                                            //Steven 20141016 : FullSite的Test Time Out
            {
                iContractCount++;                                                                                       //kevin 20180928 add contract count
                if(IniConfig.bIndexAddPressEP==true)                                                                    //jou 20171026 (wei) : 測試中加壓EP
                {
                    bTestEPaddKg=false;
                    if(DeviceForm.fAireForce!=0)
                        ADAM_WriteVoltage(DeviceForm.fAireForce);
                }

                //iSendGpibTestHome=false;                                      //kevin 20150626 送出測試訊號 等收到資料才能歸home
                bTestingStopAllMotor=false;                                                                             //jou 2013-09-25 Testing Need Stop All Motor

                if(ATC_SYSTEM==eATCHonPrecType)                                                                         // 2011.05.24 , Joye , ATC ----------- //Steven 20120410 : Hontech ATC
                {
#if 0 // TODO(W906-GB-P2b): T11 ATCInterfaceForm (T11) -- golden atester.cpp:1681
                    ATCInterfaceForm->SendTestEnd(Type);
#endif // T11
                }

#if 0 // TODO(W906-GB-P2b): T04 iATC_MODE_TYPE (T04) -- golden atester.cpp:1684
                if(ATC_InterfaceForm->iATC_MODE_TYPE==ATC_TYPE_70 &&
#else
                if(false &&                                                     //AI(W906-GB-P2b) 20260926: T04 -- shim value 0 is not ATC_TYPE_70 (golden false)
#endif // T04
                   Temperature.bATC7TSDFunction==true)                                                                  //Steven 20160604 : by site TSD
                {
                    iSiteOn[0]=0;
                    iSiteOn[1]=0;
                    iSiteOn[2]=0;
                    iSiteOn[3]=0;
#if 0 // TODO(W906-GB-P2b): T04 UseTSD_Function (T04) -- golden atester.cpp:1691
                    ATC_InterfaceForm->UseTSD_Function(4, iSiteOn);                                                     //Ifor 20160823 add Site Count
#endif // T04
                }

                if(ATC_SYSTEM==eATC60 ||                                                                                //2014-05-30    Dell    for ATC6.0
                   ATC_SYSTEM==eATC30)                                                                                  //20141204 ChungHung add for ATC3.0
                {
#if 0 // TODO(W906-GB-P2b): T11 ATCInterfaceForm (T11) -- golden atester.cpp:1697
                    ATCInterfaceForm->ATC_60_SYS.SendTestStart(0);
#endif // T11
                }

                bEcho=false;
                if(bGPIBError)
                {
                    ShowMyMessage("GPIB Format Error", "GPIB 格式錯誤");
                    for(int i=0; i<TestSocket.iShtRow; i++)
                    {
                        for(int j=0; j<TestSocket.iShtCol; j++)
                        {
                            iTesterBIN[i][j]=iTestBinCount;                                                             //Steven 20121112 : RS232支援32Bin 15 --> iTestBinCount
                        }
                    }
                    return true;
                }

                for(int i=0; i<TestSocket.iShtRow; i++)
                {
                    for(int j=0; j<TestSocket.iShtCol; j++)
                    {
                        if(TestIF_File.iCloseSiteOnHPDontTest==2 &&                                                     //Steven 20250604 : 關site的位置有IC不測試送指定 bin
                           TestSocket.iBinData[i][j]==iTestBinCount &&
                           AnsiString(TestSocket.PordRec[i][j].asBuffer->Strings[eErrorCode]).AnsiPos("NonTestToSettedBin")!=0)  //AI(W906-GB-P2b) 20260926: D1 AnsiString(...) around the Strings[] proxy (no AnsiPos on vclcompat StringsProxy, TStringList.h:90-117)
                        {
                            iBin[i][j]=TestIF_File.iCloseSiteBin;
                            iTesterBIN[i][j]=TestIF_File.iCloseSiteBin;
                        }
                        else if(iBin[i][j]==257)                                                                        //kevin 20140319  bin0
                        {
                            iTesterBIN[i][j]=0;
//                            bIndexDutflag[Type][i][j]=true;                   //kevin 20140318 紀錄目前index有IC需測試
                        }
                        else if(iBin[i][j]==0)                                                                          //kevin 20140319  bin0
                        {
                            iTesterBIN[i][j]=-1;
                        }
                        else if(iBin[i][j]==-1)                                                                         //Steven 20150408 : 針對-1 Bin進行處理
                        {
                            bHasError=true;
                            iTesterBIN[i][j]=iTestBinCount;
                        }
                        else
                        {
                            if(TestIF_File.bEnableBarCode)
                            {
                                if(LastSet.iTester==_2D_SORT &&
                                   CosFunction.bSortingBy2DList &&
                                   TestIF_File.bSortingBy2DIDList)                                                      //Frank 20221122 : 2DID sorting for ATK
                                {
                                    Str.sprintf("%s", TestSocket.cDeviceInf[i][j]);
                                    if(Str==asBarCodeErrorSend || Str=="")                                              //wei 20160318 Barcode Error依客戶設定
                                    {
                                        iBin[i][j]=iTestBinCount;
                                    }
                                    else
                                    {
#if 0 // TODO(W906-GB-P2b): T14 TfBarCode (BarCode/BarCode.h:40-88) has no GotBinFrom2DSortList / b2DIDIsInsideList / b2DIDIsInsideToErrorBin -- golden atester.cpp:1754
                                        iTempBin=fBarCode->GotBinFrom2DSortList(Str);                                   //Steven 20240515 : modified for 2D sort
#else
                                        iTempBin=iTestBinCount;                 //AI(W906-GB-P2b) 20260926: T14 -- unknown -> golden's own "not in sorting list" answer (error bin, WAR16329 path)
#endif // T14

                                        if(iTempBin==iTestBinCount)
                                        {
                                            TestSocket.PordRec[i][j].AddErrorRecordNoSave("2DID is not in sorting list");

                                            if(TestIF_File.iActionOf2DNotInList==0)                                     //Steven 20250707 : Action Of 2D Not In List
                                            {
                                                ShowErrorMessage("WAR16329", K_SKIP, MMSystem);
                                                fMain->BtnOneCycleClick(fMain);
                                            }

                                            if(IniConfig.bEnable_SECS_GEM==true)
                                            {
#if 0 // TODO(W906-GB-P2b): T15 EventReport / SECS_EVENT are declared in SECSGEM/SecsEventReport.h:55 / SECSGEM/SecsEventType.h:340, neither included -- golden atester.cpp:1768
                                                EventReport(SECS_EVENT.UnexpectedUNITIDRead);
#endif // T15
                                            }
                                        }
                                        iBin[i][j]=iTempBin;
                                    }
                                }
                                else
                                {
                                    if(TestIF_File.bSearch2DIDByLot &&
                                       LastSet.iTester==OFF_LINE)                                                       //Frank 20170316 (wei) add Search 2DID By Lot
                                    {
#if 0 // TODO(W906-GB-P2b): T14 b2DIDIsInsideList (T14) -- golden atester.cpp:1779
                                        ret=fBarCode->b2DIDIsInsideList(i, j);
#else
                                        ret=0;                                  //AI(W906-GB-P2b) 20260926: T14 -- unknown -> "not found" (b2DIDListErrorBin); never a fabricated good bin
#endif // T14
                                        if(ret==0)                                                                      //沒找到
                                        {
                                            iBin[i][j]=TestIF_File.b2DIDListErrorBin;
                                        }
                                        else if(ret==2)                                                                 //BarcodeError
                                        {
                                            iBin[i][j]=iTestBinCount;
                                        }
                                    }
                                    else if(TestIF_File.b2DIDNotExist2Error &&
                                            LastSet.iTester==OFF_LINE)                                                  //JerryYang 20231218 : 2DID黑名單功能
                                    {
#if 0 // TODO(W906-GB-P2b): T14 b2DIDIsInsideToErrorBin (T14) -- golden atester.cpp:1792
                                        ret=fBarCode->b2DIDIsInsideToErrorBin(i, j);
#else
                                        ret=1;                                  //AI(W906-GB-P2b) 20260926: T14 -- unknown -> "found in the ERR list" (error bin); fail-safe
#endif // T14
                                        if(ret==1)                                                                      //有找到
                                        {
                                            iBin[i][j]=iTestBinCount;
                                            TestSocket.PordRec[i][j].AddErrorRecordNoSave("Search 2DID in the ERR list");
                                        }
                                    }

                                    if(TestIF_File.iNoCodeDeviceToErr!=0)                                               //Steven 20151221 : 將讀取異常的IC放到Error Bin
                                    {
                                        Str.sprintf("%s", TestSocket.cDeviceInf[i][j]);
                                        if(Str==asBarCodeErrorSend || Str=="" || Str==asBarCodeErrorCheckSum)           //KaiChen 20191121 ：中壢日月光 2D Check Sum      //wei 20160318 Barcode Error依客戶設定
                                        {
                                            iBin[i][j]=iTestBinCount;
                                        }
                                    }
                                }
                            }

                            iTesterBIN[i][j]=iBin[i][j];                                                                //-1;
//                            bIndexDutflag[Type][i][j]=true;                     //kevin 20140318 紀錄目前index有IC需測試
                        }
                        iBin[i][j]=0;                                                                                   //Steven 20150306 : 使用完畢就重置變數，避免以後又發生分Bin異常
                    }
                }

                if(bHasError)                                                                                           //Steven 20150408 : 針對-1 Bin進行處理
                {
                    ShowMyMessage("GPIB Format Error", "GPIB 格式錯誤");
                    for(int i=0; i<MAX_SOCKET_ROW; i++)
                    {
                        for(int j=0; j<MAX_SOCKET_COL; j++)
                        {
                            iTesterBIN[i][j]=iTestBinCount;                                                             //Steven 20121112 : RS232支援32Bin 15 --> iTestBinCount
                        }
                    }
                }
                return true;
            }

            if(TestTimeOut.Off())                                               //AI(W906-GB-P2b) 20260926: T16 lifted -- SetTestTimeOutTimer is translated (golden 912 :10920-10955), so TestTimeOut is armed to MaxTime+10 at every SOT; golden atester.cpp:1832
            {
                bTestingStopAllMotor=false;                                                                             //jou 2013-09-25 Testing Need Stop All Motor
                if(TestIF.iTestType==TTL_MODE && (TTL_CARD_TYPE==2 || TTL_CARD_TYPE==3))                                //Isaac 20200903 :TTL RS232通訊
                {
                    if(IniConfig.bI12TesterTimerOutNotNeedReTest)                                                       //No resend SOT
                    {
                        Task=60;
                    }
                    else
                    {
                        Task=55;
                    }
                }
                else
                {
                    if(bExist==false)
                    {
    //                    fMain->WakeupGPIB("GetTesterResult");                 //wei 20150703 因為RS232會斷線
                        Task=55;
                    }
                }
            }
            break;
        case 65:
            if(IniConfig.bEnableTestingNeedStopAllMotor==true &&
               IniConfig.bI24TestingNeedStopAllMotor==true)                     //jou 2013-09-25 Testing Need Stop All Motor
            {
                dwTestTime=TestStopMotorTimer.LatchCycleTime()/100.0;
                if(dwTestTime>=(unsigned int)((TestIF.dInitWaitTime-0.5)*10) &&
                   dwTestTime<(unsigned int)((TestIF.dInitWaitTime+TestIF.dTestingWaitTime)*10))
                {
                    bTestingStopAllMotor=true;                                  //jou 2013-09-25 Testing Need Stop All Motor
                }
                else
                {
                    bTestingStopAllMotor=false;                                 //jou 2013-09-25 Testing Need Stop All Motor
                }
            }

            if(CosFunction.bUploadTestArmTorqueLog)                             //Ifor 20190912 :add 海思 V02.30 版 Record Torque
            {
                bGetTestArmTorque=false;
                if(bFrontTestArmTorqueFinish==false)
                {
                    if(fMain->edTorue0->Text!="")
                    {
                        bFrontTestArmTorqueFinish=true;
                        bGetTestArmTorque=true;
                    }
                }

                if(bRearTestArmTorqueFinish==false)
                {
                    if(fMain->edTorue1->Text!="")
                    {
                        bRearTestArmTorqueFinish=true;
                        bGetTestArmTorque=true;
                    }
                }

                if(bGetTestArmTorque==true)
                {
#if 0 // TODO(W906-GB-P2b): T13 FormHS (T13) -- golden atester.cpp:1895-1896
                    sArmTestInfoEvenLogFile = FormHS->GetLastFileLogName_HS(5);
                    FormHS->RecordArmTestInfoLog_HS(sArmTestInfoEvenLogFile);
#endif // T13
                }
            }

            if(IniConfig.bIndexAddPressEP==true)                                //Ifor 20190912 :add 海思 V02.30 版 Record Torque
            {                                                                   //jou 20171026 (wei) : 測試中加壓EP
                bTestEPaddKg=true;
                dTestTime=StrToFloat(FormatFloat("0.0", double(TestStopMotorTimer.LatchCycleTime())/1000.0));

                if(dGetSec!=dTestTime &&
                   dTestTime<=IniConfig.iIndexAddPressEP_Time && dTestTime>0)
                {
                    if(DeviceForm.fAireForce!=0)
                    {
                        dGetSec=dTestTime;
                        iEPaddSec=dTestTime*10.0/10.0;
                        bEPaddMSec=!bEPaddMSec;
                        if(bEPaddMSec==true)
                            dEPaddMSec=+IniConfig.dIndexVibrateEP_Kg;
                        else
                            dEPaddMSec=-IniConfig.dIndexVibrateEP_Kg;

                        ADAM_WriteVoltage(DeviceForm.fAireForce+iEPaddSec*IniConfig.dIndexAddPressEP_Kg+dEPaddMSec);
                    }
                }
            }

            if(NULL_Delay.Off())
            {
                EndTestTimeStamp(1);
                IsTest=false;                                                   //Steven 20130703
                Task=60;
            }
            break;
//GPIB mode end
//TTL mode start
#if 0 // TODO(W906-GB-P2b): T17 user ruling -- new machines use RS232Standard for TTL, direct-card branch not ported (cases 100/200/250/260/265/270/300) -- golden atester.cpp:1932-2183
        case 100:                                                               // TTL use 應該事先清
            if(ATC_SYSTEM==eATC60 || ATC_SYSTEM==eATC30)                        //20141204 ChungHung add for ATC3.0   //2014-05-30    Dell    for ATC6.0
            {
                ATCInterfaceForm->ATC_60_SYS.SendTestStart(1);
            }

            if(LastSet.iTester==OFF_LINE)                                       //Steven 20091031 Add for TTL offline testing
            {
                Task=300;
                break;
            }
            SetNoiseDelay=false;
            SW[SwClear0].On();
            SW[SwClear4].On();                                                  //Alick 20161011 (Steven) : TTL支援8Site

            TTLLog("GetTesterResult 100");                                      //Steven 20151123 : Log for TTL
            Task=200;
            break;
        case 200:
//            SW[TTL_ClearData[0]].Off();
            SW[SwClear0].Off();                                                 //Frank 20170424 (Steven) : 修正TTL Clear訊號 On --> Off
            SW[SwClear4].Off();                                                 //Alick 20161011 (Steven) : TTL支援8Site

            TTLLog("GetTesterResult 200");                                      //Steven 20151123 : Log for TTL
            Task=250;

            for(int i=0; i<1000; i++)                                           //2008/10/02 lee start // only delay
            {
                ;
            }
            //break;                                                            //2008/10/02 lee start
        case 250:
//            SW[TTL_ClearData[0]].On();
            SW[SwClear0].On();
            SW[SwClear4].On();                                                  //Alick 20161011 (Steven) : TTL支援8Site

            TTLLog("GetTesterResult 250");                                      //Steven 20151123 : Log for TTL
#ifndef SOFT_SIMULTE
            for(int i=0; i<TestSocket.iShtRow; i++)                             //Steven 20241015 : InArmSuck --> TestSocket
            {
                for(int j=0; j<TestSocket.iShtCol; j++)
                {
                    if(Test_GetTestResul(i, j)!=-1)
                    {
                        fMain->Pause("TTL Clear");
                        ShowMyMessage("Interface Board can not clear", "TTL介面卡無法清空資料");
                        Task=100;
//                        bPauseTest=true;
                        return false;
                    }
                }
            }
#endif
            if(TestSocket.HasRealIC())                                          //Steven 20210218 : 修正測試時間的紀錄
                RecordStartTestTime();

            bATC_SITE_2ND_CHECK[Type]=true;                                     //Ifor 20160509 add ATC 測試時開啟第二點溫度監控
            if(ATC_SYSTEM==eNewATCSystem)                                       //Ifor 20160516 修改ATC Heat 設定數
            {
                ATC_InterfaceForm->SiteTesting(iATC_Use_Heat_Count, bATCSiteTest);
            }
            else if(ATC_SYSTEM==eATCHonPrecType)
            {
                ATCInterfaceForm->SendTestStart(Type);
            }
            Task=260;
        case 260:
            if(CUSTOMER_CODE==CC_AMKOR_Korea && SystemStart==false)             //Steven 20151214 : 先針對ATK做修改, 避免影響別人
                return false;

            if(Prod.DIOCfg.iDutType!=DUTNONE)                                   //use Dut signal
            {
                iStartStep=0;
                for(int i=0; i<TestSocket.iShtRow; i++)
                {
                    for(int j=0; j<TestSocket.iShtCol; j++)
                    {
                        iCH=j+i*MAX_Index_Row;
                        if(Prod.DIOCfg.iDutType==DUTPosPluse ||
                           Prod.DIOCfg.iDutType==DUTPosLevel)
                        {
                            if(flag[i][j])
                                SW[TTL_Dut[iCH]].Off();                         // start Test
                        }
                        else
                        {
                            if(flag[i][j])
                                SW[TTL_Dut[iCH]].On();                          // start Test
                        }
                    }
                }

                if(CosFunction.bTTLUseUSec)                                     //Steven 20180808 (wei) : TTL的時間單位改成microsecond
                    DutDelay.SetUSAndOn(Prod.DIOCfg.iDutBfOnTime);
                else
                    DutDelay.SetMSAndOn(Prod.DIOCfg.iDutBfOnTime);
                Task=265;
            }
            else
            {
                Task=270;
            }
            TTLLog("GetTesterResult 260");                                      //Steven 20151123 : Log for TTL
        case 265:
            if(DutDelay.Off())
                Task=270;
            break;
        case 270:
            iStartStep=1;
            if(Prod.DIOCfg.iStartType==CHSingle)
            {
                bClearSrtart[0]=true;
                if(CosFunction.bTTLUseUSec)                                                                                                                     //Steven 20180808 (wei) : TTL的時間單位改成microsecond
                {
                    MyTTLSOTTimer.SetUSAndOn(Prod.DIOCfg.iSTPluseWidth);
                }
                else
                {
                    MyTTLSOTTimer.SetMSAndOn(Prod.DIOCfg.iSTPluseWidth);
                }

                SW[TTL_StartData[Prod.DIOCfg.iOneSTChannel]].On();                                                                                              //only this signal if handler request other please change wire in hardware
            }
            else
            {
                fMain->mmo1->Lines->Clear();
                for(int i=0; i<TestSocket.iShtRow; i++)
                {
                    for(int j=0; j<TestSocket.iShtCol; j++)
                    {
                        if(flag[i][j])
                        {
                            if(TestIF.iTestMode>=_6Site2X3)                                                                                                     //Alick 20161011 (Steven) : TTL支援8Site
                                iCH=j+i*4;
                            else
                                iCH=j+i*MAX_Index_Row;

                            bClearSrtart[iCH]=true;

                            if(Prod.DIOCfg.iSTPluseWidth>=500)                                                                                                  //Eliot 2008_06_10  100->3000  //Steven 20170106 (Jou) : 3000 --> 500
                                Prod.DIOCfg.iSTPluseWidth=500;
                            if(Prod.DIOCfg.iSTPluseWidth<10)                                                                                                    //Steven 20170106 (Jou) : 1 --> 10
                                Prod.DIOCfg.iSTPluseWidth=10;

                            if(CosFunction.bTTLUseUSec)                                                                                                         //Steven 20180808 (wei) : TTL的時間單位改成microsecond
                            {
                                MyTTLSOTTimer.SetUSAndOn(Prod.DIOCfg.iSTPluseWidth);
                            }
                            else
                            {
                                MyTTLSOTTimer.SetMSAndOn(Prod.DIOCfg.iSTPluseWidth);
                            }

                            SW[TTL_StartData[iCH]].On();                                                                                                        // start Test
                            SW[TTL_StartData[iCH]].On();                                                                                                        // start Test
                            SW[TTL_StartData[iCH]].On();                                                                                                        // start Test

                            sprintf(str2, "Start[%d][%d]  %x %01d", i, j, SW[TTL_StartData[iCH]].Port, SW[TTL_StartData[iCH]].Bit);
                            fMain->mmo1->Lines->Add(str2);                                                                                                      //Lee 2010-03-23 start : Record TTL data
                            fMain->mmo1->Lines->Add("");
                            for(int k=0; k<5; k++)
                            {
                                sprintf(str2, "Sensor[%d]  %x %01d", k, Sen[TTL_Sensor[iCH][k]].Port, Sen[TTL_Sensor[iCH][k]].Bit);
                                fMain->mmo1->Lines->Add(str2);
                            }
                            fMain->mmo1->Lines->Add( "");
#ifdef SOFT_SIMULTE
                            Sim_TTL_Single(i, j);
#endif
                        }
                    }
                }
            }
            TTLLog("GetTesterResult 270-1");                                                                                                                    //Steven 20151123 : Log for TTL

            while(1)
            {
                for(int i=0; i<8; i++)                                                                                                                          //Alick 20161011 (Steven) : TTL支援8Site
                {
                    if(bClearSrtart[i])
                    {
                        if(MyTTLSOTTimer.Off())                                                                                                                 //Steven 20180808 (wei) : TTL的時間單位改成microsecond
                        {
                            bClearSrtart[i]=false;
                            SW[TTL_StartData[i]].Off();
                            if(Prod.DIOCfg.iDutType==DUTPosPluse || Prod.DIOCfg.iDutType==DUTNegPluse)
                            {
                                if(CosFunction.bTTLUseUSec)                                                                                                     //Steven 20180808 (wei) : TTL的時間單位改成microsecond
                                    DutDelay.SetUSAndOn(Prod.DIOCfg.iDutAfOffTime);
                                else
                                    DutDelay.SetMSAndOn(Prod.DIOCfg.iDutAfOffTime);
                            }
                        }
                    }
                }

                Application->ProcessMessages();
                if(bClearSrtart[0]==false && bClearSrtart[1]==false && bClearSrtart[2]==false && bClearSrtart[3]==false &&                                      //Alick 20161011 (Steven) : TTL支援8Site
                   bClearSrtart[4]==false && bClearSrtart[5]==false && bClearSrtart[6]==false && bClearSrtart[7]==false)
                    break;
            }

            TTLLog("GetTesterResult 270-2");                                                                                                                    //Steven 20151123 : Log for TTL
            for(int i=0; i<8; i++)                                                                                                                              //Steven 20161011 : TTL支援8Site 4 --> 8
            {
                SW[TTL_StartData[i]].Off();
                bClearSrtart[i]=false;
            }
            TTLLog("GetTesterResult 270-3");                                                                                                                    //Steven 20151123 : Log for TTL
            Task=300;
            break;
        case 300:
            if(ATC_SYSTEM==eATC60 || ATC_SYSTEM==eATC30)                        //20141204 ChungHung add for ATC3.0   //2014-05-30    Dell    for ATC6.0
            {
                ATCInterfaceForm->ATC_60_SYS.SendTestStart(0);
            }

            for(int i=0; i<TestSocket.iShtRow; i++)
            {
                for(int j=0; j<TestSocket.iShtCol; j++)
                {
                    if(flag[i][j]==true)
                    {
                        if(Test_GetTestResul(i, j)==-1)                         // test not finish
                            return false;
                    }
                }
            }

            for(int i=0; i<TestSocket.iShtRow; i++)
                for(int j=0; j<TestSocket.iShtCol; j++)
                    if(flag[i][j])
                        iTesterBIN[i][j]=Test_GetTestResul(i, j);               //jou 2014-06-26 修正TTL BIN1,Handler出現BIN0 errir

            for(int i=0; i<8; i++)                                              //Alick 20161011 (Steven) : TTL支援8Site
            {
                SW[TTL_StartData[i]].Off();
                bClearSrtart[i]=false;
            }

            if(ATC_InterfaceForm->iATC_MODE_TYPE==ATC_TYPE_70 &&
               Temperature.bATC7TSDFunction==true)                              //Steven 20160604 : by site TSD
            {
                iSiteOn[0]=0;
                iSiteOn[1]=0;
                iSiteOn[2]=0;
                iSiteOn[3]=0;
                ATC_InterfaceForm->UseTSD_Function(4, iSiteOn);                 //Ifor 20160823 add Site Count
            }

            TTLLog("GetTesterResult 300-3");                                    //Steven 20151123 : Log for TTL
            return true;
#endif // T17
//TTL mode end
//Temp RS232 start
        case 500:
            if(GetTempICResult())
            {
                return true;
            }
            break;
//Temp RS232 end
        case 9999:
            if(TestTimeOut.Off())
            {
                Task=1;
            }
            break;
//Steven 20160604 start: by site TSD
        case 20000:
            TSDDelay.SetSecAndOn(Temperature.dATC7TSDTimeOut);
            iSiteOn[0]=(Type==0 && (FTestSuck.Item[0][0]==HAS_IC || FTestSuck.Item[0][0]==HAS_HOT_IC))?1:0;
            iSiteOn[1]=(Type==0 && (FTestSuck.Item[0][1]==HAS_IC || FTestSuck.Item[0][1]==HAS_HOT_IC))?1:0;             //Ifor 20160804 FTestSuck.Item[0][0] ==> FTestSuck.Item[0][1]
            iSiteOn[2]=(Type==1 && (BTestSuck.Item[0][0]==HAS_IC || BTestSuck.Item[0][0]==HAS_HOT_IC))?1:0;             //Ifor 20160804 BTestSuck.Item[0][1] ==> BTestSuck.Item[0][0]
            iSiteOn[3]=(Type==1 && (BTestSuck.Item[0][1]==HAS_IC || BTestSuck.Item[0][1]==HAS_HOT_IC))?1:0;
#if 0 // TODO(W906-GB-P2b): T04 UseTSD_Function (T04) -- golden atester.cpp:2206
            ATC_InterfaceForm->UseTSD_Function(4, iSiteOn);                                                             //Ifor 20160823 add Site Count
#endif // T04
            Task=20010;
            break;
        case 20010:
#if 0 // TODO(W906-GB-P2b): T04 bGetATC_SEND_TEMP_READY (T04) -- golden atester.cpp:2210
            if(ATC_InterfaceForm->bGetATC_SEND_TEMP_READY==true)
#else
            if(false)                                                           //AI(W906-GB-P2b) 20260926: T04 -- no ATC ready flag; the TSDDelay time-out arm stays golden (case 20000 is only entered from :1212, now false)
#endif // T04
            {
                HTestDeley.SetSecAndOn(0.01);
                Task=50;                                                        //Steven 20161019 : 20 --> 50 for TSD
                Str.sprintf("%6.3f", MyTSDTimer.LatchCycleTime()/1000.0);
#if 0 // TODO(W906-GB-P2b): T18 TfMain (forms/fMain.h) has no memoTSD -- golden atester.cpp:2215
                fMain->memoTSD->AddTextWithDateTime(Str);                       //Steven 20161208 (jou) : Record TSD time data
#endif // T18

                for(int i=0; i<TestSocket.iShtRow; i++)
                {
                    for(int j=0; j<TestSocket.iShtCol; j++)
                    {
                        TestSocket.PordRec[i][j].AddTSDTime(Str);
                    }
                }

                if(bNeedInitialTestDelay)                                       //Steven 20161201 : Fixed for ATC7.0 hangup with TSD function
                {
                    if(iInitialCount>0)
                    {
                        iInitialCount=(Prod.iInitialDelay)-dwStartInitialCount.LatchCycleTime()/1000;
                        break;
                    }
                }
                bNeedInitialTestDelay=false;
            }
            else if(TSDDelay.Off())
            {
                ret=ShowErrorMessage("WAR15183", K_RETRY|K_SKIP, MTestY1+Type);
                if(ret==K_RETRY)
                {
                    Task=20000;
                    SetTestTimeOutTimer(Type);                                  //Steven 20200407 : 整合Time Out時間設定
                    HangTime.SetSecAndOn(Prod.iHangupMaxTime);                  //Steven 20180829 : #P180827-ATK-H9-03 It happened "Tester Time Up Error" after "TSD Wait Time Error".
                }
                else
                {
                    HTestDeley.SetSecAndOn(0.01);

                    for(int i=0; i<TestSocket.iShtRow; i++)
                    {
                        for(int j=0; j<TestSocket.iShtCol; j++)
                        {
                            iTesterBIN[i][j]=iTestBinCount;                     //Steven 20161025 : 無完成TSD的,要送去Err Bin
                        }
                    }
                    return true;
                }
            }
            break;
//Steven 20160604 End: by site TSD
        //Steven 20180817 : Boost Function
        //==>
        case 30000:
            if(bUnderTest)
            {
                Task=10;
                break;
            }

            iBoostFuncStep=0;
            iBoostEotToSotTime=TestIntervalsBoostTime.LatchCycleTime()/1000.0;  //JerryYang 20181122 (Steven) : 將不同function計時器分開
            if(iTriggerBoostFunction==Temperature.eBMid &&
               Temperature.dBoostIdleTime[0]!=Temperature.dBoostIdleTime[2])    //Steven 20260505 : add zero-guard for (dBoostIdleTime[0]-dBoostIdleTime[2])
            {
                Temperature.dBoostOffset[iTriggerBoostFunction]         =((Temperature.dBoostOffset[0]-Temperature.dBoostOffset[2])/(Temperature.dBoostIdleTime[0]-Temperature.dBoostIdleTime[2]))*(iBoostEotToSotTime-Temperature.dBoostIdleTime[2])+Temperature.dBoostOffset[2];
                Temperature.dBoostDuration[iTriggerBoostFunction]       =((Temperature.dBoostDuration[0]-Temperature.dBoostDuration[2])/(Temperature.dBoostIdleTime[0]-Temperature.dBoostIdleTime[2]))*(iBoostEotToSotTime-Temperature.dBoostIdleTime[2])+Temperature.dBoostDuration[2];
                Temperature.dPostBoostDuration[iTriggerBoostFunction]   =((Temperature.dPostBoostDuration[0]-Temperature.dPostBoostDuration[2])/(Temperature.dBoostIdleTime[0]-Temperature.dBoostIdleTime[2]))*(iBoostEotToSotTime-Temperature.dBoostIdleTime[2])+Temperature.dPostBoostDuration[2];
            }

#if 0 // TODO(W906-GB-P2b): T19 fTemp_Set is declared in forms/fTemp_Set.h:1563, which atester.cpp does not include (display writes only) -- golden atester.cpp:2279-2282
            fTemp_Set->edtIdleTime_Mid->Text        =FormatFloat("0.0", iBoostEotToSotTime);
            fTemp_Set->edtBoostOffset_Mid->Text     =FormatFloat("0.0", Temperature.dBoostOffset[iTriggerBoostFunction]);
            fTemp_Set->edtBoostDuration_Mid->Text   =FormatFloat("0.0", Temperature.dBoostDuration[iTriggerBoostFunction]);
            fTemp_Set->edtPostBoost_Mid->Text       =FormatFloat("0.0", Temperature.dPostBoostDuration[iTriggerBoostFunction]);
#endif // T19

#if 0 // TODO(W906-GB-P2b): T20 TfLotInfo (forms/fLotInfo.h:1413-2314) has no SetATCOffset (same gate as aTester_Front.cpp G1) -- golden atester.cpp:2284
            fLotInfo->SetATCOffset(true);                                       //Steven 20180817 : Boost Function
#endif // T20
//            bSetTempChange=true;
            fHeaterOK=false;
            if(Temperature.bATCActiveCooling)
                HTestDeley.SetSecAndOn(5);
            else
                HTestDeley.SetSecAndOn(10);

#if 0 // TODO(W906-GB-P2b): T05 fContact widget (T05) -- golden atester.cpp:2292-2293
            if(fContact->fShow)
                fContact->pnlHandlerSatus->Visible=true;
#endif // T05

            if(iTriggerBoostFunction==Temperature.eBLBL)
            {
                Task=30001;
                LBBoostTimeOutTimer.SetSecAndOn(Temperature.dBoostTimeOut);     //Steven 20181222 : Add LB升溫的Time out
            }
            else if(Temperature.iBoostFunctionMode==0)                          //No heater wait
            {
                Task=30100;
            }
            else if(Temperature.iBoostFunctionMode==1)                          //Wait Heat up and down
            {
                Task=31100;
            }
            else if(Temperature.iBoostFunctionMode==2)                          //Wait Heat up only
            {
                Task=32100;
            }
            else
            {
                Task=32100;
            }

            break;
        case 30001:                                                             //Steven 20181102 : LB Temp Function
#if 0 // TODO(W906-GB-P2b): T04 iATC_MODE_TYPE/ATC_TYPE_60 (T04) + fTemp_Set (T19); brace-less if -- golden atester.cpp:2319-2320
            if(ATC_InterfaceForm->iATC_MODE_TYPE==ATC_TYPE_60)
                fTemp_Set->ControlATC60AirFlow(1);
#endif // T04

            Task=30002;
        case 30002:
            SetTestTimeOutTimer(Type);                                                                                  //Steven 20200407 : 整合Time Out時間設定

            if(UN150Read[tcLB]>=Temperature.dBoostIdleTime[Temperature.eBLBL]+Temperature.dThreshold)                   //Steven 20190927 : 要比L/B溫度多1.5度, 比較不會在一開始溫度比較低的時候降過頭
            {
                iTriggerBoostFunction=Temperature.eBLBI;
                Task=30000;
            }
            else if(LBBoostTimeOutTimer.Off())                                                                          //Steven 20181222 : Add LB升溫的Time out
            {
                bLBBoostTimeOut=true;
                if(IniConfig.bSPILFunction)                                                                             //JerryYang 20220126 : 矽品要求boost timeout要跳異常訊息
                {
                    ShowMyMessage("LB boost time out.","");
                }
                else
                {
                    RecordProcess("LB boost time out.");
                }
                Task=10;
            }
            break;
        case 30100:                                                             //不等Heater OK
            SetTestTimeOutTimer(Type);                                          //Steven 20200407 : 整合Time Out時間設定
            RecordProcess("Start boost duration.");
#if 0 // TODO(W906-GB-P2b): T05 fContact widget (T05) -- golden atester.cpp:2348-2349
            if(fContact->fShow)
                fContact->labDelayStatus->Caption="Start boost duration.";
#endif // T05
            iInitialCount=Temperature.dBoostDuration[iTriggerBoostFunction];
            HTestDeley.SetSecAndOn(Temperature.dBoostDuration[iTriggerBoostFunction]);
            TSDDelay.LatchCycleTime(true);
            iBoostFuncStep=0;                                                   //補Offset
            Task=30200;
            break;
        case 30200:
            SetTestTimeOutTimer(Type);                                          //Steven 20200407 : 整合Time Out時間設定
            if(HTestDeley.Off())
            {
                iBoostFuncStep=3;
                RecordProcess("Start post boost duration.");
#if 0 // TODO(W906-GB-P2b): T05 fContact widget (T05) -- golden atester.cpp:2362-2363
                if(fContact->fShow)
                    fContact->labDelayStatus->Caption="Start post boost duration.";
#endif // T05
                fHeaterOK=false;
                bSetTempChange=true;
                TSDDelay.LatchCycleTime(true);
                HTestDeley.SetSecAndOn(Temperature.dPostBoostDuration[iTriggerBoostFunction]);
                Task=30300;
            }
            else
            {
                iInitialCount=Temperature.dBoostDuration[iTriggerBoostFunction]-TSDDelay.LatchCycleTime()/1000;
#if 0 // TODO(W906-GB-P2b): T05 fContact widget (T05) -- golden atester.cpp:2373-2374
                if(fContact->fShow)
                    fContact->lblCountDown->Caption=iInitialCount;
#endif // T05

                iBoostFuncStep=0;
            }
            break;
        case 30300:
            SetTestTimeOutTimer(Type);                                          //Steven 20200407 : 整合Time Out時間設定
            if(HTestDeley.Off())
            {
                RecordProcess("Boost duration finish.");
                iTriggerBoostFunction=-1;
                iBoostFuncStep=5;
                HTestDeley.SetSecAndOn(0.01);
                Task=50;
#if 0 // TODO(W906-GB-P2b): T05 fContact widget (T05) -- golden atester.cpp:2388-2389
                if(fContact->fShow)
                    fContact->pnlHandlerSatus->Visible=false;
#endif // T05
            }
            else
            {
                iInitialCount=Temperature.dPostBoostDuration[iTriggerBoostFunction]-TSDDelay.LatchCycleTime()/1000;
#if 0 // TODO(W906-GB-P2b): T05 fContact widget (T05) -- golden atester.cpp:2394-2395
                if(fContact->fShow)
                    fContact->lblCountDown->Caption=iInitialCount;
#endif // T05
                iBoostFuncStep=4;
            }
            break;
        case 31100:                                                             //兩個都要等
            SetTestTimeOutTimer(Type);                                          //Steven 20200407 : 整合Time Out時間設定
            fHeaterOK=false;
            if(HTestDeley.Off())
            {
                Task=31150;
            }
            break;
        case 31150:
            SetTestTimeOutTimer(Type);                                          //Steven 20200407 : 整合Time Out時間設定
            if(fHeaterOK && HTestDeley.Off())
            {
                RecordProcess("Start boost duration.");
#if 0 // TODO(W906-GB-P2b): T05 fContact widget (T05) -- golden atester.cpp:2412-2413
                if(fContact->fShow)
                    fContact->labDelayStatus->Caption="Start boost duration.";
#endif // T05
                iInitialCount=Temperature.dBoostDuration[iTriggerBoostFunction];
                HTestDeley.SetSecAndOn(Temperature.dBoostDuration[iTriggerBoostFunction]);
                TSDDelay.LatchCycleTime(true);
                iBoostFuncStep=1;
                Task=31200;
            }
            else
            {
#if 0 // TODO(W906-GB-P2b): T05 fContact widget (T05) -- golden atester.cpp:2422-2423
                if(fContact->fShow)
                    fContact->lblCountDown->Caption="Heater wait.";
#endif // T05
            }
            break;
        case 31200:
            SetTestTimeOutTimer(Type);                                          //Steven 20200407 : 整合Time Out時間設定
            if(fHeaterOK && HTestDeley.Off())
            {
                iBoostFuncStep=3;
                RecordProcess("Finish boost duration.");
                fHeaterOK=false;
                bSetTempChange=true;
                if(Temperature.bATCActiveCooling)
                    HTestDeley.SetSecAndOn(5);
                else
                    HTestDeley.SetSecAndOn(10);
                Task=31300;
            }
            else
            {
                iInitialCount=Temperature.dBoostDuration[iTriggerBoostFunction]-TSDDelay.LatchCycleTime()/1000;
#if 0 // TODO(W906-GB-P2b): T05 fContact widget (T05) -- golden atester.cpp:2443-2444
                if(fContact->fShow)
                    fContact->lblCountDown->Caption=iInitialCount;
#endif // T05

                iBoostFuncStep=1;
            }
            break;
        case 31300:
            SetTestTimeOutTimer(Type);                                          //Steven 20200407 : 整合Time Out時間設定
            fHeaterOK=false;
            if(HTestDeley.Off())
            {
                Task=31350;
            }

#if 0 // TODO(W906-GB-P2b): T05 fContact widget (T05) -- golden atester.cpp:2457-2458
            if(fContact->fShow)
                fContact->labDelayStatus->Caption="Heater Wait";
#endif // T05
            break;
        case 31350:
            SetTestTimeOutTimer(Type);                                          //Steven 20200407 : 整合Time Out時間設定
            if(fHeaterOK)
            {
                iBoostFuncStep=4;
                RecordProcess("Start post boost duration.");
                TSDDelay.LatchCycleTime(true);
                HTestDeley.SetSecAndOn(Temperature.dPostBoostDuration[iTriggerBoostFunction]);
                Task=31400;
            }
            break;
        case 31400:
            SetTestTimeOutTimer(Type);                                          //Steven 20200407 : 整合Time Out時間設定
            if(HTestDeley.Off())
            {
                RecordProcess("Boost duration finish.");
                iTriggerBoostFunction=-1;
                iBoostFuncStep=5;
                HTestDeley.SetSecAndOn(0.01);
                Task=50;
#if 0 // TODO(W906-GB-P2b): T05 fContact widget (T05) -- golden atester.cpp:2480-2481
                if(fContact->fShow)
                    fContact->pnlHandlerSatus->Visible=false;
#endif // T05
            }
            else
            {
                iInitialCount=Temperature.dPostBoostDuration[iTriggerBoostFunction]-TSDDelay.LatchCycleTime()/1000;
#if 0 // TODO(W906-GB-P2b): T05 fContact widget (T05) -- golden atester.cpp:2486-2487
                if(fContact->fShow)
                    fContact->lblCountDown->Caption=iInitialCount;
#endif // T05
                iBoostFuncStep=4;
            }
            break;
        case 32100:                                                             //只等升溫
            fHeaterOK=false;
            SetTestTimeOutTimer(Type);                                          //Steven 20200407 : 整合Time Out時間設定

            if(HTestDeley.Off())
            {
                bFlagBelowTurnOfValve=true;
                iBoostFuncStep=0;
                Task=32150;
            }
            break;
        case 32150:
            SetTestTimeOutTimer(Type);                                          //Steven 20200407 : 整合Time Out時間設定

            ret=CheckToBoostIndexTemp();                                        //Steven 20181222 : Add 當升溫到一半,發現LB溫度不足,要重新補溫度
            if(ret==4)
            {
                Task=10;
                iTriggerBoostFunction=-1;
                iTriggerBoostFuncBack=-1;
                break;
            }

            if(fHeaterOK && HTestDeley.Off())
            {
                bFlagBelowTurnOfValve=false;
#if 0 // TODO(W906-GB-P2b): T04 iATC_MODE_TYPE/ATC_TYPE_60 (T04) + fTemp_Set (T19); brace-less if -- golden atester.cpp:2517-2518
                if(ATC_InterfaceForm->iATC_MODE_TYPE==ATC_TYPE_60)
                    fTemp_Set->ControlATC60AirFlow(0);
#endif // T04

                RecordProcess("Start boost duration.");
#if 0 // TODO(W906-GB-P2b): T05 fContact widget (T05) -- golden atester.cpp:2521-2522
                if(fContact->fShow)
                    fContact->labDelayStatus->Caption="Start boost duration.";
#endif // T05
                iInitialCount=Temperature.dBoostDuration[iTriggerBoostFunction];
                HTestDeley.SetSecAndOn(Temperature.dBoostDuration[iTriggerBoostFunction]);
                TSDDelay.LatchCycleTime(true);
                iBoostFuncStep=1;
                Task=32200;
            }
            else
            {
#if 0 // TODO(W906-GB-P2b): T05 fContact widget (T05) -- golden atester.cpp:2531-2532
                if(fContact->fShow)
                    fContact->lblCountDown->Caption="Heater wait.";
#endif // T05
            }
            break;
        case 32200:
            SetTestTimeOutTimer(Type);                                          //Steven 20200407 : 整合Time Out時間設定

            ret=CheckToBoostIndexTemp();                                        //Steven 20181222 : Add 當升溫到一半,發現LB溫度不足,要重新補溫度
            if(ret==4)
            {
                Task=10;
                iTriggerBoostFunction=-1;
                iTriggerBoostFuncBack=-1;
                break;
            }

            if(HTestDeley.Off())
            {
                iTriggerBoostFuncBack=iTriggerBoostFunction;
                BoostCoolTime.SetSecAndOn(Temperature.dPostBoostDuration[iTriggerBoostFuncBack]);
                iBoostFuncStep=10;
                iTriggerBoostFunction=-1;
#if 0 // TODO(W906-GB-P2b): T05 fContact widget (T05) -- golden atester.cpp:2553-2554
                if(fContact->fShow)
                    fContact->pnlHandlerSatus->Visible=false;
#endif // T05
                HTestDeley.SetSecAndOn(0.01);
                BoostCoolCountDown.LatchCycleTime(true);
                dBoostCoolSec=0.0;
                BoostCoolStepTimer.SetSecAndOn(1);
                iBoostCountDown=Temperature.dPostBoostDuration[iTriggerBoostFuncBack];
                Task=50;
            }
            else
            {
                iInitialCount=Temperature.dBoostDuration[iTriggerBoostFunction]-TSDDelay.LatchCycleTime()/1000;
#if 0 // TODO(W906-GB-P2b): T05 fContact widget (T05) -- golden atester.cpp:2565-2566
                if(fContact->fShow)
                    fContact->lblCountDown->Caption=iInitialCount;
#endif // T05

                iBoostFuncStep=1;
            }
            break;
    }
    return false;
}
//******************************************************************************
bool Check_TTL_Status(int Type)                                                 // golden :2564 (decode consumer)
{
    GetTesterResult(Type);
    for(int i=0; i<TestSocket.iShtRow; i++)
    {
        for(int j=0; j<TestSocket.iShtCol; j++)
        {
            if(TestSocket.Item[i][j]!=NULL_IC &&
               TestSocket.Item[i][j]!=HAS_NULL_IC &&
               TestSocket.Item[i][j]<TEST_PASS)
            {
                if(iTesterBIN[i][j]<0 ||
                   iTesterBIN[i][j]>iTestBinCount)                              //Steven 20121112 : RS232支援32Bin 15 --> iTestBinCount
                    return false;
            }
        }
    }
    return true;
}
//------------------------------------------------------------------------------
bool CheckTestSocketIsError(int i, int j)                                       // golden :2584 (decode consumer)
{
    if(iTesterBIN[i][j]>=iTestBinCount ||                                       //jou 980818 add test result no set,result set to Interface Error
       iTesterBIN[i][j]<0 ||                                                    //Steven 20120821 : 記憶體溢位
       Prod.iT6CatData[iTesterBIN[i][j]]<0 ||                                   //Steven 20121112 : RS232支援32Bin 15 --> iTestBinCount
       Prod.iT6CatData[iTesterBIN[i][j]]>=eTrayCount ||                         //JerryYang 20221215 : Magazine最大到14
       Prod.iTrayType[Prod.iT6CatData[iTesterBIN[i][j]]]==tNotUse)
    {
        if(LastSet.iTester==_2D_SORT &&
           CosFunction.bSortingBy2DList &&
           TestIF_File.bSortingBy2DIDList &&                                    //Frank 20221122 : 2DID sorting for ATK
           TestIF_File.iActionOf2DNotInList==1)                                 //Steven 20250806 : Action Of 2D Not In List
        {
            return false;
        }
        else
        {
            return true;
        }
    }
    else
    {
        return false;
    }
}
//------------------------------------------------------------------------------
//  ProcessTestResult (golden 906 :2621-3579 == golden 912 :2641-3599) -- bin-decode + DB write + UI report.
//------------------------------------------------------------------------------
// ===========================================================================
//  ProcessTestResult -- LIVE golden translation.  AI(W906-GB-P2b) 20260926, St02 tester-comm stage P2b
//  (user ruling 20260926 "由 St02 直接進行這一部分的移植工作").
//  Golden (Big5/cp950): D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\atester.cpp:2629-3604 (target 912).
//    906 -> 912: the whole region -- preamble comment, the global ProcessTestResultDelay, the 959-line body and
//    the trailing banner (906 :2609-3584 / 912 :2629-3604) -- is byte-identical after cp950 decoding, so there
//    is no 912 change to apply.  912 line = 906 line + 20 throughout this function.
//  Replaces the former GOLDEN VERBATIM PAIR (an inert #if 0 copy of these 959 lines + a slim stub returning 0).
//  The body is that golden text with only the edits listed below.  Every golden comment, column layout,
//  #ifdef SOFT_SIMULTE / #ifndef ASE_KaohSiung / #ifndef DEBUG_AutoSiteMap arm and customer branch is kept.
//  ProcessTestResultDelay (golden 912 :2640) had no definition anywhere in V906 (only the two inert uses); it
//  is golden text and is defined here at file scope, exactly as golden.  EOL: CRLF, as the rest of atester.cpp.
//
//  GATE REGISTER  (golden 912 line numbers; every #if 0 keeps the golden text inside it)
//   R01 :2764 :2774 :2781 :2788  fLotInfo->SaveASECLNewTestLogInfo -- not a TfLotInfo member (forms/fLotInfo.h;
//                     its :1070 lists it among the untranslated log writers).  SAFETY too: it is a log-file
//                     writer.  The bin write, PordRec record and yield bookkeeping around it stay live.
//   R02 :2805        fShowBinSelect->IntervalByTotalCount->Caption -- forms/fShowBinSelect.h (global at :1133)
//                     is not reachable from atester.cpp's includes, and TfShowBinSelect has no
//                     IntervalByTotalCount.  UI label only; the iYieldTotalCount bookkeeping stays live.
//   R03 :2850 :2871  fBarCode->b2DIDIsInsideToErrorBin -- not a TfBarCode member (BarCode/BarCode.h:25-88;
//                     recorded unported at BarCode/BarCode_Bottom2DID.h:155-159).  `#else int k=0;`: 0 is the
//                     callee's own "not in listError2DID" answer (golden 912 BarCode.cpp:8338/:8356), so the
//                     2DID-blacklist arm (TestIF_File.b2DIDNotExist2Error, OFF_LINE only) raises the interface
//                     error exactly as golden does for a 2DID that is not on the list (an alarm, never a
//                     silent accept).  A 2DID that IS on the list is also alarmed until the callee is ported.
//   R04 :3031-3032 :3196-3197 :3239-3240 :3471-3472 :3569-3570 :3592-3593  ATC_InterfaceForm->TestFinish() /
//                     ->HandlerArm(-1) -- ATC_InterfaceForm is not reachable from atester.cpp's includes; its
//                     only declaration is acarry_shims.h:115 (TATC_InterfaceFormShim, iATC_MODE_TYPE only); the
//                     real class forms/fATCHandlerSide.h:845/:847 has no global (same as HandlerGpibMsg.cpp G23).
//                     The `if(ATC_SYSTEM==eNewATCSystem && ...)` blocks and TriggerATC_FFC_Function() stay live.
//   R05 :3261        CONTACT_NORMAL -- not reachable from atester.cpp's includes (cContact.h:158,
//                     BarCode/BarCode_Shuttle2_CCDScan.h:187).  Operand gate; `#else iContactMode==0)` is the
//                     exact golden value (golden 912 cContact.cpp:75 `const int CONTACT_NORMAL=0`): no arm changes.
//   [LIFTED 20260926] R06 :3297-3298   fMesSystem->AutoSiteMapPass -- declared at forms/fMesSystem.h:658, but that header is not
//                     reachable from atester.cpp's includes.  Effect: VTEST (bVTESTFunction) + "Auto Site
//                     Mapping Set Pass BIN", FT run: bAutoSiteMapWaitTestPass keeps the false set at :3272, so
//                     the check always reports "Site Mapping Check Fail" (never a false pass).
//   R07 :3465 :3497  fMain->AutoSiteMappignCleanOut(true) -- not a TfMain member (forms/fMain.h; golden 912
//                     main.h:1334, body main.cpp:29258-29278: tray clear + CleanOut, or One Cycle).  The
//                     ReStartAutoSiteMapping / ShowMyMessage / return beside it stay live.
//   R08 :3484        fMain->SetOpenBin() -- not a TfMain member (golden 912 main.cpp:9516-9519, only
//                     edSetOpenBin->Text=Prod.iOpenBin; same gate as cinitial.cpp:9137 n2-5).  Prod.iOpenBin
//                     is still set live on the line before.
//   [LIFTED 20260926] R09 :3565-3566   `if(IniConfig.bEnable_SECS_GEM==true) EventReport(SECS_EVENT.GetTestResult);` -- EventReport /
//                     SECS_EVENT (SECSGEM/SecsEventReport.h, SecsEventType.h) are reachable only through
//                     acatchtray_shims.h, which atester.cpp does not include.  The whole if-statement is gated
//                     (gating only its body would make the next statement its body).
//   R10 :3578 :3580-3581  fLotInfo->WhenTestRecordTemperatureLog_3Sigma -- not a TfLotInfo member
//                     (forms/fLotInfo.h:1073).  SAFETY too: it writes D:\HT9045_Log\Sigma, and the
//                     `#ifdef SOFT_SIMULTE` arm is the one this tree compiles by default (MachineType.h:62-64),
//                     where golden calls it without the iATCOnLine check.  Both arms kept; in the #else arm the
//                     whole if-statement is gated.
//
//  NON-GATE ADAPTATIONS (AI note at each site)
//    * :2769 :2941  `...asBuffer->Strings[eErrorCode].AnsiPos(...)` -> `AnsiString(...Strings[eErrorCode]).AnsiPos(...)`:
//      vclcompat's StringsProxy has no AnsiPos (vclcompat/TStringList.h:89-117).  Same value.
//
//  GOLDEN CALLS THAT COMPILE AS WRITTEN BUT REACH A V906 STAND-IN (no gate)
//    * MyDBIProcess("Message", ...) binds to aHotPlateSubstrate.h:979 (2-arg; body aHotPlateSubstrate.cpp:1244 only
//      counts, no DB/CSV row).  Golden meant cMyDB's MyDBIProcess(asTable, S1, S2="") (V906 cMyDB.h:81), which
//      atester.cpp does not include.
//    * GetTesterResult (atester.cpp:2541) is still the slim stub: returns false and fills no iTesterBIN.
//      DoCheckHasTestTempChange / TriggerATC_FFC_Function are no-op stubs (atester.cpp:11673/:11680).
//      fMain->Pause / ReStartAutoSiteMapping / BtnOneCycleClick are facade no-ops (forms/fMain.cpp:246/:248/:404;
//      golden 912 main.cpp:29168 ReStartAutoSiteMapping is NOT empty).  fMain->CloseGpibProgram forwards to
//      THandlerTesterSide when TesterComm is wired (forms/fMain.cpp:1063).
//
//  GOLDEN QUIRKS KEPT (look wrong; not fixed)
//    Q1 :2666/:2723  bDone is a fresh local (false): GetTesterResult(Index) runs twice per call and the
//       `if(bDone==true)` arms are dead the first time.
//    Q2 :2691-2709  TTL `case 1:` falls through into `case 100:` on purpose.
//    Q3 :2830  fBarCode->iBinReturnMess[i*2+j] uses row stride 2 whatever TestSocket.iShtCol is.
//    Q4 :2799/:2813 the interval-yield flags read Prod.bIsPassBin[Prod.iT6CatData[bin]] (indexed by category),
//       while :2904 indexes Prod.bIsPassBin by bin; :2809/:2813/:2817 `% Prod.iIntervalLowYieldCountByTotal`
//       divides by zero if that count is 0 while "by total" is enabled.
//    Q5 :3474  TriggerATC_FFC_Function() sits outside the ATC if-block (inside it at the other sites);
//       :3028-3035 (format-error SKIP) returns 1 without calling it.
//    Q6 :3388  `j==(iDoSiteMappingStep-TestSocket.iShtCol)` assumes two socket rows.
//    Q7 static iSiteMappingErrorCT / iOldErrorSite survive lot changes.
// ===========================================================================
//==============================================================================
//******************************************************************************
//
//  注意!! ProcessTestResult為Handler收發BIN相關, 修改時要小心!!
//
//******************************************************************************
//==============================================================================
//  Ver : 2003_07
//
// return  0: not ready       1: OK        2: interface error
//==============================================================================
TQPF_Timer ProcessTestResultDelay;
int ProcessTestResult(int Index)
{
    static int iSiteMappingErrorCT=0;                                           //Ifor 20180417 : add ASE_M要求錯誤三次才Alarm
    static int iOldErrorSite=0;                                                 //Ifor 20180417 : add 避免未清除錯誤次數

    int ret;
    bool bCanSetBin=false;
    bool SomeDataEnter=false;
    bool bAddSitemapStep=false;
    bool bHasTestIC=false;
    bool bSiteMappingHasError=false;                                            //Ifor 20180417 : add 目前Site Maping位置
    bool bInterfaceError=false, flag=true;
    bool bInterfacePos0[MAX_SOCKET_ROW][MAX_SOCKET_COL]={{false, false, false, false, false, false, false, false},
                                                         {false, false, false, false, false, false, false, false},
                                                         {false, false, false, false, false, false, false, false},
                                                         {false, false, false, false, false, false, false, false}};

    AnsiString Str;
    bool bDone=false;

    RecordPiggyBackStartEnd(false);

    if(bNeedReplunge_RFMD==true)
        return 1;

    if(bDone==true)
        GetTesterResult(Index);
    else
        bDone=GetTesterResult(Index);

    for(int i=0; i<TestSocket.iShtRow; i++)
    {
        for(int j=0; j<TestSocket.iShtCol; j++)
        {
            if(TestSocket.Item[i][j]!=NULL_IC &&
               TestSocket.Item[i][j]<TEST_PASS &&
               TestSocket.Item[i][j]!=HAS_NULL_IC)
            {
                if(iTesterBIN[i][j]!=-1)
                {
                    SomeDataEnter=true;
                    if(iRecordOldBin[i][j]==-1)
                        iRecordOldBin[i][j]=iTesterBIN[i][j];
                }
            }
        }
    }

    if(TestIF.iTestType==TTL_MODE)
    {
        switch(iGetTestDataDelayTask)
        {
            case 1:
                if(SomeDataEnter==true)
                {
                    ProcessTestResultDelay.SetMSAndOn(1);
                    iGetTestDataDelayTask=100;
                }
                else
                {
                    break;
                }
            case 100:
                if(ProcessTestResultDelay.Off())
                {
                    bCanSetBin=true;
                }
                break;
        }
    }
    else
    {
        if(fContact->fShow && iTestTask>=30000)
        {
            bCanSetBin=false;
        }
        else
        {
            bCanSetBin=true;
        }
    }

    if(bDone==true)
        GetTesterResult(Index);
    else
        bDone=GetTesterResult(Index);
    bInterfaceError=false;
    if(bCanSetBin==true)
    {
        for(int i=0; i<TestSocket.iShtRow; i++)
        {
            for(int j=0; j<TestSocket.iShtCol; j++)
            {                                                                   // ====================================== 2010/05/15 lee/joye
                if(TestSocket.Item[i][j]!=NULL_IC &&
                   TestSocket.Item[i][j]<TEST_PASS &&
                   TestSocket.Item[i][j]!=HAS_NULL_IC)
                {
                    if(iTesterBIN[i][j]!=-1)
                    {
                        if(iRecordOldBin[i][j]!=iTesterBIN[i][j])               // 2010/05/15 lee/joye
                        {
                            iRecordOldBin[i][j]=-1;
                            iGetTestDataDelayTask=1;
                            return 0;
                        }

                        if(CUSTOMER_CODE==CC_ASE_KaohSiung ||                   //jou 2010-11-11 改成只有福雷會出現GPIB format error
                           CUSTOMER_CODE==CC_ASE_KaohSiung_K12)                 //Steven 20131101 : Add ASE-K12
                        {
                            if(iTesterBIN[i][j]>=iTestBinCount)                 //Steven 20121112 : RS232支援32Bin 15 --> iTestBinCount
                                bBin16HangUp=true;
                        }

                        Str.sprintf("%s", TestSocket.cDeviceInf[i][j]);
                        if(TestIF_File.bEnableBarCode &&
                           TestIF_File.iNoCodeDeviceToErr==1 &&                 //Steven 20151221 : 將讀取異常的IC放到Error Bin
                          (Str==asBarCodeErrorSend ||                           //wei 20160318 Barcode Error依客戶設定
                           Str=="" ||
                           Str==asBarCodeErrorCheckSum))                        //KaiChen 20191121 ：中壢日月光 2D Check Sum
                        {
                            TestSocket.SetItemData(i, j, TEST_PASS+iTestBinCount);
                            TestSocket.iBinData[i][j]=iTestBinCount;
                            TestSocket.PordRec[i][j].AddTestResultRecord(iTestBinCount, TestSocket.cSBin[i][j], "No2DCodeDevice");                              //Frank 20160505 add
#if 0 // TODO(W906-GB-P2b): R01 fLotInfo->SaveASECLNewTestLogInfo is not a TfLotInfo member (forms/fLotInfo.h, :1070 lists it untranslated; SAFETY: a log-file writer) -- golden atester.cpp:2764 (912; 906 :2744)
                            fLotInfo->SaveASECLNewTestLogInfo(i, j, TestSocket.iBinData[i][j]);                         //JerryYang 20250120 : add
#endif
                        }
                        else if(TestIF_File.iCloseSiteOnHPDontTest==2 &&        //JerryYang 20180726 (wei) 關site的位置有IC不測試送error bin
                                TestSocket.iBinData[i][j]==iTestBinCount &&     //Steven 20250604 : 關site的位置有IC不測試送指定 bin
//                                LastSet.bUseTestSocket[Type][i][j]==false)
                                AnsiString(TestSocket.PordRec[i][j].asBuffer->Strings[eErrorCode]).AnsiPos("NonTestToSettedBin")!=0)  //AI(W906-GB-P2b) 20260926: AnsiString(...) wrap -- vclcompat StringsProxy has no AnsiPos (vclcompat/TStringList.h:89-117); same value
                        {
                            TestSocket.SetItemData(i, j, TEST_PASS+TestIF_File.iCloseSiteBin);
                            TestSocket.iBinData[i][j]=TestIF_File.iCloseSiteBin;
                            TestSocket.PordRec[i][j].AddTestResultRecord(TestIF_File.iCloseSiteBin, TestSocket.cSBin[i][j], "NonTestToSettedBin");
#if 0 // TODO(W906-GB-P2b): R01 fLotInfo->SaveASECLNewTestLogInfo is not a TfLotInfo member (forms/fLotInfo.h, :1070 lists it untranslated; SAFETY: a log-file writer) -- golden atester.cpp:2774 (912; 906 :2754)
                            fLotInfo->SaveASECLNewTestLogInfo(i, j, TestSocket.iBinData[i][j]);                         //JerryYang 20250120 : add
#endif
                        }
                        else if(myIAR_Test.GetSuckTempErr(i, j))                //Steven 20250102 : fixed for [I54]
                        {
                            TestSocket.SetItemData(i, j, TEST_PASS+iTestBinCount);
                            TestSocket.iBinData[i][j]=iTestBinCount;
                            TestSocket.PordRec[i][j].AddTestResultRecord(iTestBinCount, TestSocket.cSBin[i][j], "Temperature Error");                           //Frank 20160505 add
#if 0 // TODO(W906-GB-P2b): R01 fLotInfo->SaveASECLNewTestLogInfo is not a TfLotInfo member (forms/fLotInfo.h, :1070 lists it untranslated; SAFETY: a log-file writer) -- golden atester.cpp:2781 (912; 906 :2761)
                            fLotInfo->SaveASECLNewTestLogInfo(i, j, TestSocket.iBinData[i][j]);                         //JerryYang 20250120 : add
#endif
                        }
                        else
                        {
                            TestSocket.SetItemData(i, j, TEST_PASS+iTesterBIN[i][j]);
                            TestSocket.iBinData[i][j]=iTesterBIN[i][j];
                            TestSocket.PordRec[i][j].AddTestResultRecord(TestSocket.iBinData[i][j], TestSocket.cSBin[i][j]);                                    //Frank 20160505 add
#if 0 // TODO(W906-GB-P2b): R01 fLotInfo->SaveASECLNewTestLogInfo is not a TfLotInfo member (forms/fLotInfo.h, :1070 lists it untranslated; SAFETY: a log-file writer) -- golden atester.cpp:2788 (912; 906 :2768)
                            fLotInfo->SaveASECLNewTestLogInfo(i, j, TestSocket.iBinData[i][j]);                         //JerryYang 20250120 : add
#endif
                            if(CosFunction.IntervalYieldCount)                  //wei 20180606 Interval Low Yield By Site
                            {
                                if(Prod.bFailAlarmIntervalLowYieldBySite)
                                {
                                    if(CheckTestSocketIsError(i, j)==true)
                                    {
                                        bIntervalYieldIsPass[Index][i][j][iYieldSiteCount[Index]]=false;
                                    }
                                    else
                                    {
                                        bIntervalYieldIsPass[Index][i][j][iYieldSiteCount[Index]]=Prod.bIsPassBin[Prod.iT6CatData[TestSocket.iBinData[i][j]]];
                                    }
                                }

                                if(Prod.bFailAlarmIntervalLowYieldByTotal)
                                {
#if 0 // TODO(W906-GB-P2b): R02 fShowBinSelect is not reachable from atester.cpp's includes (forms/fShowBinSelect.h:1133) and TfShowBinSelect has no IntervalByTotalCount (UI label) -- golden atester.cpp:2805 (912; 906 :2785)
                                    fShowBinSelect->IntervalByTotalCount->Caption=iYieldTotalCount;
#endif

                                    if(CheckTestSocketIsError(i, j)==true)
                                    {
                                        bYieldTotalBinIsPass[iYieldTotalCount%Prod.iIntervalLowYieldCountByTotal]=false;
                                    }
                                    else
                                    {
                                        bYieldTotalBinIsPass[iYieldTotalCount%Prod.iIntervalLowYieldCountByTotal]=Prod.bIsPassBin[Prod.iT6CatData[TestSocket.iBinData[i][j]]];
                                    }

                                    iYieldTotalCount++;
                                    if((iYieldTotalCount>=Prod.iIntervalLowYieldCountByTotal) &&
                                       (iYieldTotalCount%Prod.iIntervalLowYieldCountByTotal==0))
                                    {
                                        bYieldTotalBin=true;
                                    }
                                }
                                bYieldSiteBinCheck=true;
                            }
                        }

                        if(fBarCode->bGPIBTestBarCodeError)
                        {
                            TestSocket.cReDeviceInf[i][j]="";
                            TestSocket.cReDeviceInf[i][j]=fBarCode->iBinReturnMess[i*2+j];
                        }

                        if(CheckTestSocketIsError(i, j)==true)
                        {
                            if((TestIF_File.bEnableBarCode && TestIF_File.iNoCodeDeviceToErr==1) ||                     //Steven 20151221 : 將讀取異常的IC放到Error Bin
                               (TestIF_File.bEnableBarCode && TestIF_File.bSearch2DIDByLot && LastSet.iTester==OFF_LINE))                                       //Frank 20170316 (wei) add Search 2DID By Lot
                            {
                                Str.sprintf("%s", TestSocket.cDeviceInf[i][j]);
                                if(Str==asBarCodeErrorSend ||                   //wei 20160318 Barcode Error依客戶設定
                                   Str=="" ||
                                   Str==asBarCodeErrorCheckSum)                 //KaiChen 20191121 ：中壢日月光 2D Check Sum
                                {
                                }
                                else
                                {
                                    if(TestIF_File.bEnableBarCode &&
                                       TestIF_File.b2DIDNotExist2Error &&
                                       LastSet.iTester==OFF_LINE)               //JerryYang 20231218 : 2DID黑名單功能
                                    {
#if 0 // TODO(W906-GB-P2b): R03 TfBarCode has no b2DIDIsInsideToErrorBin (BarCode/BarCode.h:25-88; unported, BarCode/BarCode_Bottom2DID.h:155-159) -- golden atester.cpp:2850 (912; 906 :2830)
                                        int k=fBarCode->b2DIDIsInsideToErrorBin(i, j);
#else
                                        int k=0;                                //AI(W906-GB-P2b) 20260926: R03 -- 0 = the callee's own "not in listError2DID" answer (golden 912 BarCode.cpp:8338/:8356): the unlisted-2DID arm (interface error), as golden
#endif
                                        if(k==1)                                //有找到
                                        {
                                        }
                                        else
                                        {
                                            if(bBin16HangUp==false)
                                                bInterfaceError=true;
                                        }
                                    }
                                    else
                                    {
                                        if(bBin16HangUp==false)
                                            bInterfaceError=true;
                                    }
                                }
                            }
                            else if(TestIF_File.bEnableBarCode &&
                                    TestIF_File.b2DIDNotExist2Error &&
                                    LastSet.iTester==OFF_LINE)                  //JerryYang 20231218 : 2DID黑名單功能
                            {
#if 0 // TODO(W906-GB-P2b): R03 TfBarCode has no b2DIDIsInsideToErrorBin (BarCode/BarCode.h:25-88; unported, BarCode/BarCode_Bottom2DID.h:155-159) -- golden atester.cpp:2871 (912; 906 :2851)
                                int k=fBarCode->b2DIDIsInsideToErrorBin(i, j);
#else
                                int k=0;                                        //AI(W906-GB-P2b) 20260926: R03 -- 0 = the callee's own "not in listError2DID" answer (golden 912 BarCode.cpp:8338/:8356): the unlisted-2DID arm (interface error), as golden
#endif
                                if(k==1)                                        //有找到
                                {
                                }
                                else
                                {
                                    if(bBin16HangUp==false)
                                        bInterfaceError=true;
                                }
                            }
                            else
                            {
                                if(bBin16HangUp==false)
                                    bInterfaceError=true;
                            }

                            if((iHWFix_BinBox==1 || bCancelErrorBin))           //kevin 20160819 error bin 要放到 Bin Box
                            {
                                if(Index==0)
                                    sArm1BinError=asRecordTestResult;           //kevin 20160725 記錄 Error Bin
                                else
                                    sArm2BinError=asRecordTestResult;           //kevin 20160725 記錄 Error Bin
                                bBinError[Index]=true;                          //kevin 20160725 Error Bin 發生arm
                                bInterfaceError=false;
                            }

                            bInterfacePos0[i][j]=true;
                            TestSocket.bPass[i][j]=false;
                            TestSocket.bNeedReTest[i][j]=false;
                            LastSet.iTestIgnore++;                              //RogerYang 20250923 : 瑞薩FT-CT
                        }
                        else
                        {
                            if(Prod.bIsPassBin[iTesterBIN[i][j]])
                            {
                                TestSocket.bPass[i][j]=true;
                                if(IniConfig.bD22_4_PassBinCanDoubleContact)    //JerryYang 20230909 : pass bin也可以設定Double contact
                                {
                                    if(Prod.DBContact[iTesterBIN[i][j]]!=0)
                                        TestSocket.bNeedReTest[i][j]=true;
                                    else
                                        TestSocket.bNeedReTest[i][j]=false;
                                }
                                else
                                {
                                    TestSocket.bNeedReTest[i][j]=false;
                                }
                            }
                            else
                            {
                                TestSocket.bPass[i][j]=false;
                                if(Prod.DBContact[iTesterBIN[i][j]]!=0)
                                    TestSocket.bNeedReTest[i][j]=true;
                                else
                                    TestSocket.bNeedReTest[i][j]=false;
                            }
                            LastSet.iTesterMatch++;                             //RogerYang 20250923 : 瑞薩FT-CT TesterMatch表示正常測試結束
                        }
                    }
                    else
                    {
                        flag=false;
                    }
                }
                else if(TestIF_File.iCloseSiteOnHPDontTest==2 &&                //JerryYang 20180726 (wei) 關site的位置有IC不測試送error bin
                        iTestTask==60 &&                                        //Steven 20251022 : hang up
                        bDone &&
                        TestSocket.Item[i][j]!=NULL_IC &&
                        TestSocket.Item[i][j]==(TEST_PASS+iTestBinCount) &&
                        TestSocket.iBinData[i][j]==iTestBinCount &&             //Steven 20251022 : 關site的位置有IC不測試送指定 bin
                        AnsiString(TestSocket.PordRec[i][j].asBuffer->Strings[eErrorCode]).AnsiPos("NonTestToSettedBin")!=0)  //AI(W906-GB-P2b) 20260926: AnsiString(...) wrap -- vclcompat StringsProxy has no AnsiPos (vclcompat/TStringList.h:89-117); same value
                {
                    TestSocket.SetItemData(i, j, TEST_PASS+TestIF_File.iCloseSiteBin);
                    TestSocket.iBinData[i][j]=TestIF_File.iCloseSiteBin;
                    TestSocket.PordRec[i][j].AddTestResultRecord(TestIF_File.iCloseSiteBin, TestSocket.cSBin[i][j], "NonTestToSettedBin");
                }
            }
        }

        if(CosFunction.bCheckTempDuringIndexArmTesting==true &&
           IniConfig.bI54_Enable==true &&
           IniConfig.bI54_1_AllICErr==true &&
           myIAR_Test.GetArmTempErr())                                          //Steven 20250102 : fixed for [I54]
        {
            for(int i=0; i<TestSocket.iShtRow; i++)
            {
                for(int j=0; j<TestSocket.iShtCol; j++)
                {
                    if(TestSocket.Item[i][j]!=NULL_IC &&
                       TestSocket.Item[i][j]!=HAS_NULL_IC)
                    {
                        TestSocket.SetItemData(i, j, TEST_PASS+iTestBinCount);
                        TestSocket.iBinData[i][j]=iTestBinCount;
                        TestSocket.PordRec[i][j].AddTestResultRecord(iTestBinCount, TestSocket.cSBin[i][j], "Temperature Error");                               //Frank 20160505 add
                    }
                }
            }
        }
    }
    else
    {
        flag=false;                                                             // 2010/05/15 lee/joye
    }

    if(bBin16HangUp)                                                            //Steven 20100412
    {
        DoCheckHasTestTempChange();                                             //Ifor 20230505 add: 確認是否測試中有切換溫度並切回原生產溫度
        if(TestIF.iTestType==GPIB_MODE || TestIF.iTestType==TCP_IP_MODE)        //wei 20211027 open short TCP/IP
        {
            if(CUSTOMER_CODE==CC_ASE_KaohSiung || CosFunction.bIndexAreaOnlyCanUseSkip==true)                           //kevin 20141008 測試問題不能重測 //Steven 20141105 : Index內的所有異常都只能用Skip
                ret=ShowErrorMessage("WAR07320", K_SKIP, MMInterface, false, asRecordTestResult);
            else
                ret=ShowErrorMessage("WAR07320", K_RESET|K_SKIP, MMInterface, false, asRecordTestResult);
        }
        else if(TestIF.iTestType==TTL_MODE)
        {
            ret=ShowErrorMessage("WAR07319", K_RESET|K_SKIP, MMInterface, false, asRecordTestResult);
        }
        else
        {
            ret=ShowErrorMessage("WAR07318", K_RESET|K_SKIP, MMInterface, false, asRecordTestResult);
        }

        if(ret==K_SKIP)                                                         //kevin 20161228 (jou) add data format err bin
        {
            for(int i=0; i<TestSocket.iShtRow; i++)
            {
                for(int j=0; j<TestSocket.iShtCol; j++)
                {
                    if(TestSocket.Item[i][j]!=HAS_NULL_IC &&                    //kevin 20120618  沒有IC 就不要設定避免畫面被誤解
                       TestSocket.Item[i][j]!=NULL_IC     &&                    //Steven 20121115 : 沒IC的地方,設定數字進去會Hang Up!
                       CheckTestSocketIsError(i, j)==true)
                    {
                        if(IniConfig.bI26TestCloseSiteHaveBin)                  //kevin 20150202
                        {
                            if(Index==0)
                                bTestBinDataError=1;                            //kevin 20150202 測試bin 別沒設定或關site 有bin 資料
                            else
                                bTestBinDataError=2;                            //kevin 20150202 測試bin 別沒設定或關site 有bin 資料
                        }
                        else
                        {
                            iTesterBIN[i][j]=999;
                            TestSocket.SetItemData(i, j, TEST_PASS+iTesterBIN[i][j]);
                            TestSocket.iBinData[i][j]=iTestBinCount;            //Steven 20190116 : 修正顯示錯誤
                            TestSocket.PordRec[i][j].AddTestResultRecord(iTesterBIN[i][j], TestSocket.cSBin[i][j], "TestBinDataError");                         //Frank 20160505 add
                        }
                    }
                }
            }
            bBin16HangUp=false;
            if(iHWFix_BinBox==1 || bCancelErrorBin)                             //kevin 20161228 (jou) error bin 要放到 Bin Box
            {
                RecordProcess("Format error Use iHWFix_BinBox");
            }
            else
            {
                if(ATC_SYSTEM==eNewATCSystem &&
                   Temperature.bATCActiveCooling==true)                         //JerryYang 20220815 : send ATC start testing
                {
#if 0 // TODO(W906-GB-P2b): R04 ATC_InterfaceForm is not reachable from atester.cpp's includes (only acarry_shims.h:115 TATC_InterfaceFormShim, iATC_MODE_TYPE only; real class forms/fATCHandlerSide.h:845/:847 has no global) -- golden atester.cpp:3031 (912; 906 :3011)
                    ATC_InterfaceForm->TestFinish();
                    ATC_InterfaceForm->HandlerArm(-1);                          //Ifor 20220107 add:通知ATC目前哪隻Arm再Socket
#endif
                }
                return 1;
            }
        }
    }

    if(bInterfaceError)
    {
        DoCheckHasTestTempChange();                                             //Ifor 20230505 add: 確認是否測試中有切換溫度並切回原生產溫度
        SetNoiseDelay=false;
        fMain->CloseGpibProgram(__FUNC__);
        if(IniConfig.bA03UseAfterHomeCarryAndSuckIcToRBin && _bHomeNeedOnecycle)
        {
            ret=K_SKIP;
        }
        else
        {
            if(TestIF.iTestType==GPIB_MODE ||
               TestIF.iTestType==TCP_IP_MODE)                                   //wei 20211027 open short TCP/IP
            {
                if(CUSTOMER_CODE==CC_ASE_CL)                                    //jou 2010-11-25 start : 中壢ASE要Bin 0自動skip，不alarm
                {
                    int k=0;
                    int l=0;
                    for(int i=0; i<TestSocket.iShtRow; i++)
                    {
                        for(int j=0; j<TestSocket.iShtCol; j++)
                        {
                            if(TestSocket.Item[i][j]!=NULL_IC &&
                               TestSocket.Item[i][j]!=HAS_NULL_IC)
                            {
                                k++;
                                if(iTesterBIN[i][j]>=iTestBinCount)             //Steven 20230929 : 15 --> iTestBinCount
                                {
                                    l++;
                                }
                            }
                        }
                    }

                    if(k==l)
                    {
                        if(CosFunction.bTestTimeOutShowSkipAndHome && IniConfig.iI22TestTimeOutOption==3)
                        {
                            ret=ShowErrorMessage("WAR07356", K_HOME, MMInterface, false, asRecordTestResult);
                        }
                        else if(CosFunction.bIndexAreaOnlyCanUseSkip)           //Steven 20141105 : Index內的所有異常都只能用Skip
                        {
                            ret=ShowErrorMessage("WAR07356", K_SKIP, MMInterface, false, asRecordTestResult);
                        }
                        else
                        {
                            ret=ShowErrorMessage("WAR07356", K_RETRY|K_SKIP|K_CLEAN_OUT, MMInterface, false, asRecordTestResult);
                        }
                    }
                    else
                    {
                        MyDBIProcess("Message", asRecordTestResult);
                        ret=K_SKIP;
                    }
                }
                else if(CosFunction.bIndexAreaOnlyCanUseSkip ||
                        CUSTOMER_CODE==CC_AMKOR_Korea ||                        //Steven 20141105 : Index內的所有異常都只能用Skip
                        CUSTOMER_CODE==CC_SCC)                                  //Steven 20220309 : Add JSCC
                {
                    if(CosFunction.bTestTimeOutShowSkipAndHome && IniConfig.iI22TestTimeOutOption==3)
                    {
                        ret=ShowErrorMessage("WAR07356", K_HOME, MMInterface, false, asRecordTestResult);
                    }
                    else
                    {
                        ret=ShowErrorMessage("WAR07356", K_SKIP, MMInterface, false, asRecordTestResult);
                    }
                }
                else if(CUSTOMER_CODE==CC_ASE_KaohSiung)                        //kevin 20141008 測試問題不能重測
                {
                    if(IniConfig.bI26TestCloseSiteHaveBin)
                    {
                        ret=ShowErrorMessage("WAR07359",K_SKIP|K_CLEAN_OUT, MMInterface, false, asRecordTestResult);
                    }
                    else
                    {
                        ret=ShowErrorMessage("WAR07356",K_SKIP|K_CLEAN_OUT, MMInterface, false, asRecordTestResult);
                    }
                }
                else if(CUSTOMER_CODE==CC_KYEC_LEE)                             //Ifor 20180920 (Steven) : Add KYEC 要求TACS and ATN Check Time Out 直接分Err Bin
                {
                    if(asRecordTestResult=="Tester time up error")
                    {
                        asRecordTestResult="";
                        ret=ShowErrorMessage("WAR07352", K_SKIP, MMInterface, false);
                    }
                    else
                    {
                        ret=ShowErrorMessage("WAR07356", K_SKIP|K_CLEAN_OUT, MMInterface, false, asRecordTestResult);   //Ifor 20200106 add: Category setting error取消RETRY選項
                    }
                }
                else
                {
                    if(IniConfig.iI22TestTimeOutOption==0)
                    {
                        ret=ShowErrorMessage("WAR07356", K_SKIP|K_CLEAN_OUT, MMInterface, false, asRecordTestResult);
                    }
                    else if(IniConfig.iI22TestTimeOutOption==1)
                    {
                        ret=ShowErrorMessage("WAR07356", K_RETRY|K_CLEAN_OUT, MMInterface, false, asRecordTestResult);
                    }
                    else if(IniConfig.iI22TestTimeOutOption==2)
                    {
                        ret=ShowErrorMessage("WAR07356", K_SKIP|K_RETRY|K_CLEAN_OUT, MMInterface, false, asRecordTestResult);
                    }
                    else
                    {
                        ret=ShowErrorMessage("WAR07356", K_HOME|K_CLEAN_OUT, MMInterface, false, asRecordTestResult);
                    }
                }
            }
            else
            {
                if(CosFunction.bIndexAreaOnlyCanUseSkip ||                      //Steven 20141105 : Index內的所有異常都只能用Skip
                   CUSTOMER_CODE==CC_AMKOR_Korea ||
                   CUSTOMER_CODE==CC_SCC)                                       //Steven 20220309 : Add JSCC
                {
                    ret=ShowErrorMessage("WAR07356", K_SKIP, MMInterface, false, " ");
                }
                else
                {
                    if(IniConfig.iI22TestTimeOutOption==0)
                    {
                        ret=ShowErrorMessage("WAR07356", K_SKIP|K_CLEAN_OUT, MMInterface, false, " ");
                    }
                    else if(IniConfig.iI22TestTimeOutOption==1)
                    {
                        ret=ShowErrorMessage("WAR07356", K_RETRY|K_CLEAN_OUT, MMInterface, false, " ");
                    }
                    else if(IniConfig.iI22TestTimeOutOption==2)
                    {
                        ret=ShowErrorMessage("WAR07356", K_SKIP|K_RETRY|K_CLEAN_OUT, MMInterface, false, " ");
                    }
                    else
                    {
                        ret=ShowErrorMessage("WAR07356", K_HOME|K_CLEAN_OUT, MMInterface, false, " ");
                    }
                }
            }
        }

        if(ret==K_RETRY)
        {
            for(int i=0; i<TestSocket.iShtRow; i++)
            {
                for(int j=0; j<TestSocket.iShtCol; j++)
                {
                    if(bInterfacePos0[i][j]==true && TestSocket.Item[i][j]!=NULL_IC)
                    {
                        TestSocket.Item[i][j]=HAS_IC;
                    }
                }
            }
            InitTestTask();
            if(ATC_SYSTEM==eNewATCSystem &&
               Temperature.bATCActiveCooling==true)                             //JerryYang 20220815 : send ATC start testing
            {
#if 0 // TODO(W906-GB-P2b): R04 ATC_InterfaceForm is not reachable from atester.cpp's includes (only acarry_shims.h:115 TATC_InterfaceFormShim, iATC_MODE_TYPE only; real class forms/fATCHandlerSide.h:845/:847 has no global) -- golden atester.cpp:3196 (912; 906 :3176)
                ATC_InterfaceForm->TestFinish();
                ATC_InterfaceForm->HandlerArm(-1);                              //Ifor 20220107 add:通知ATC目前哪隻Arm再Socket
#endif
                TriggerATC_FFC_Function();                                      //Ifor 20240507 add :FFC Trigger Even Off
            }
            return 2;
        }

        if(ret==K_SKIP || ret==K_HOME)
        {
            if(ret==K_HOME)
            {
                fAllMotorHome=false;
                bI22_NeedHomeDelay=true;
            }
            bAutoSiteMapWaitTestResult=false;                                   //JerryYang 20190419 fix Auto site mapping發生category error後按SKIP會hang up
            for(int i=0; i<TestSocket.iShtRow; i++)
            {
                for(int j=0; j<TestSocket.iShtCol; j++)
                {
                    if(TestSocket.Item[i][j]!=HAS_NULL_IC &&                    //kevin 20120618  沒有IC 就不要設定避免畫面被誤解
                       TestSocket.Item[i][j]!=NULL_IC     &&                    //Steven 20121115 : 沒IC的地方,設定數字進去會Hang Up!
                       CheckTestSocketIsError(i, j)==true)
                    {
                        if(IniConfig.bI26TestCloseSiteHaveBin)                  //kevin 20150202
                        {
                            if(Index==0)
                                bTestBinDataError=1;                            //kevin 20150202 測試bin 別沒設定或關site 有bin 資料
                            else
                                bTestBinDataError=2;                            //kevin 20150202 測試bin 別沒設定或關site 有bin 資料
                        }
                        else
                        {
                            iTesterBIN[i][j]=999;
                            TestSocket.SetItemData(i, j, TEST_PASS+iTesterBIN[i][j]);
                            TestSocket.iBinData[i][j]=iTestBinCount;            //Steven 20190116 : 修正顯示錯誤
                            TestSocket.PordRec[i][j].AddTestResultRecord(iTesterBIN[i][j], TestSocket.cSBin[i][j], "TestBinDataError");                         //Frank 20160505 add
                        }
                    }
                }
            }

            if(ATC_SYSTEM==eNewATCSystem && Temperature.bATCActiveCooling==true)                                        //JerryYang 20220815 : send ATC start testing
            {
#if 0 // TODO(W906-GB-P2b): R04 ATC_InterfaceForm is not reachable from atester.cpp's includes (only acarry_shims.h:115 TATC_InterfaceFormShim, iATC_MODE_TYPE only; real class forms/fATCHandlerSide.h:845/:847 has no global) -- golden atester.cpp:3239 (912; 906 :3219)
                ATC_InterfaceForm->TestFinish();
                ATC_InterfaceForm->HandlerArm(-1);                              //Ifor 20220107 add:通知ATC目前哪隻Arm再Socket
#endif
                TriggerATC_FFC_Function();                                      //Ifor 20240507 add :FFC Trigger Even Off
            }
            return 1;
        }
    }

    if(flag==true)
    {
        bAddSitemapStep=false;                                                  //Ifor 20181112 : add
        bHasTestIC=false;
        if(bChangeTest_TempOffset!=0 && bDoATCTempRise==false)                  //Ifor 20230504 add: ATC 回溫功能需等待回溫結束才可切回生產溫度
        {
            DoCheckHasTestTempChange();                                         //Ifor 20230505 add: 確認是否測試中有切換溫度並切回原生產溫度
        }
        else
        {
            bChangeTest_TempAlarm=false;                                        //Ifor 20210623 add: Test Temp Change
        }

        if(IniConfig.bUseAutoSiteMapping &&                                     //jou 2011-03-24 start : Auto Site Mapping
#if 0 // TODO(W906-GB-P2b): R05 CONTACT_NORMAL is not reachable from atester.cpp's includes (cContact.h:158, BarCode/BarCode_Shuttle2_CCDScan.h:187; no header may be added) -- golden atester.cpp:3261 (912; 906 :3241)
           iContactMode==CONTACT_NORMAL)                                        //JerryYang 20170417 (Steven) contact test時不需要做auto site map check
#else
           iContactMode==0)                                                     //AI(W906-GB-P2b) 20260926: R05 -- 0 == golden 912 cContact.cpp:75 `const int CONTACT_NORMAL=0` (906 :74): the exact golden value, no arm changes
#endif
        {
            if(LastSet.iRunStartMode==rsmAutoSiteMap &&
               bSiteMappingCHKOK==false)
            {
                if(IniConfig.bVTESTFunction==true &&
                   CosFunction.bAutoSiteMappingSetPassBIN==true)                //jou 20230221 : Auto Site Mapping Set Pass BIN
                {
                    if(iAutoSiteMapRunStartMode==1)                             //RT    //jou 20230224 : 修正auto site mapping hang up
                        bAutoSiteMapWaitTestPass=true;
                    else                                                        //FT
                        bAutoSiteMapWaitTestPass=false;
                }

                for(int i=0; i<TestSocket.iShtRow; i++)
                {
                    for(int j=0; j<TestSocket.iShtCol; j++)
                    {
                        if(TestSocket.Item[i][j]!=HAS_NULL_IC  &&
                           TestSocket.Item[i][j]!=NULL_IC      &&               //Steven 20140918 : Fix for Auto Site Mapping
                           TestSocket.Item[i][j]-1000!=NULL_IC)
                        {
                            bHasTestIC=true;
                            if(iTesterBIN[i][j]>=0 &&
                               iTesterBIN[i][j]<=iTestBinCount)                 //Steven 20121112 : RS232支援32Bin 15 --> iTestBinCount
                            {
                                if(IniConfig.bVTESTFunction==true &&            //jou 2016-10-28 JCET 要求Site Mapping 必須測試到pass bin才能通過
                                   CosFunction.bAutoSiteMappingSetPassBIN==true)                                        //jou 20230221 : Auto Site Mapping Set Pass BIN
                                {
                                    if(iAutoSiteMapRunStartMode==1)             //RT
                                    {
                                        if(iTesterBIN[i][j]==Prod.iOpenBin)
                                            bAutoSiteMapWaitTestPass=false;
                                    }
                                    else                                        //FT
                                    {
//AI(W906-GB-P2b) 20260926: R06 lifted -- forms/fMesSystem.h is included now; golden atester.cpp:3297 (912)
                                        if(fMesSystem->AutoSiteMapPass(iTesterBIN[i][j]))
                                            bAutoSiteMapWaitTestPass=true;
                                    }
                                }
                                else if(CosFunction.bUSEJCETSiteMapMode==true)
                                {
                                    iGetTestData[i][j]=iTesterBIN[i][j];
                                    if(IniConfig.bI21AutoSiteMappingFailBinSetting==true)
                                    {                                           //Ifor 20171128 (wei) : add Auto Site Mapping Fail Bin Setting
                                        if(CosFunction.bAutoSiteMappingSetOpenBIN)
                                        {                                       //Steven 20230213 : [I21-9]的OS Bin跟著工作檔
                                            if(iTesterBIN[i][j]==Prod.iOpenBin)
                                                bAutoSiteMapWaitTestPass=false;
                                            else
                                                bAutoSiteMapWaitTestPass=true;
                                        }
                                        else if(iTesterBIN[i][j]==IniConfig.iI21UseFailBinSetting)
                                        {
                                            bAutoSiteMapWaitTestPass=false;
                                        }
                                        else
                                        {
                                            bAutoSiteMapWaitTestPass=true;
                                        }
                                    }
                                    else
                                    {
                                        if(Prod.bIsPassBin[iTesterBIN[i][j]]==false)                                    //RogerYang 20250220 : fixed for auto site map
                                        {
                                            bAutoSiteMapWaitTestPass=false;
                                        }
                                        else
                                        {
                                            bAutoSiteMapWaitTestPass=true;
                                        }
                                    }

                                    if(CUSTOMER_CODE==CC_LEADYO &&
                                       bAutoSiteMapWaitTestPass==false)         //KenHsieh 20251003 : LEADYO 需要設定fail次數報Alarm
                                    {
                                        if(IniConfig.iI21FailRetryCount!=0)
                                        {
                                            if(iOldErrorSite!=iDoSiteMappingStep)
                                            {
                                                iOldErrorSite=iDoSiteMappingStep;
                                                iSiteMappingErrorCT=0;
                                            }
                                            iSiteMappingErrorCT++;
                                        }
                                        else
                                        {
                                            iSiteMappingErrorCT=0;
                                        }
                                    }
                                    else
                                    {
                                        iSiteMappingErrorCT=0;
                                    }

                                    if(bAutoSiteMapWaitTestPass==false)
                                    {
                                        bASMFinishOneCycle=false;
                                        bSiteMappingCHKOK=false;

                                        if(CUSTOMER_CODE!=CC_LEADYO       ||
                                          IniConfig.iI21FailRetryCount==0 ||
                                          (IniConfig.iI21FailRetryCount!=0 &&
                                           iSiteMappingErrorCT>IniConfig.iI21FailRetryCount))                           //KenHsieh 20251003 : LEADYO 需要設定fail次數報Alarm
                                        {
                                            iSiteMappingErrorCT=0;
                                            #ifndef DEBUG_AutoSiteMap
                                            ShowMyMessage("Site Mapping Check Fail! Must Pass Bin,need Do again!",
                                                          "Site Mapping 確認失敗!必須 Pass Bin,需要再做一次!",
                                                          IndexSuckName[i][j]);                                         //Steven 20220526 : 紀錄Auto Site Map Fail的位置
                                            #endif
                                        }
                                    }

                                    if(CUSTOMER_CODE==CC_ASE_M)                 //Ztex 2025.04.30 Add ASE_M USEJCETSiteMapMode
                                        bAutoSiteMapAmbientResultCheck=true;
                                    DoJCETSiteMappingCHK(bAutoSiteMapWaitTestPass);

                                    if(IniConfig.bI19AuToSitMapPauseWaitBin)
                                        fMain->Pause("bI19AuToSitMapPauseWaitBin");
                                }
                                else if(CUSTOMER_CODE==CC_ASE_M && bGetOpenBin==false)
                                {                                               //Ifor 20180417 : add ASE_M 要求Auto Site Mapping 連續三次錯誤才Alarm(僅判斷未完成Site Mapping)
                                    if(i==0 && j==iDoSiteMappingStep)
                                    {
                                        bSiteMappingHasError=true;
                                    }
                                    else if(i==1 && j==(iDoSiteMappingStep-TestSocket.iShtCol))
                                    {
                                        bSiteMappingHasError=true;
                                    }
                                    else
                                    {
                                        bSiteMappingHasError=false;
                                    }

                                    iGetTestData[i][j]=iTesterBIN[i][j];
                                    if(iTesterBIN[i][j]!=1 && bSiteMappingHasError==true)
                                    {
                                        if(iOldErrorSite!=iDoSiteMappingStep)
                                        {
                                            iOldErrorSite=iDoSiteMappingStep;
                                            iSiteMappingErrorCT=0;
                                        }
                                        bAutoSiteMapWaitTestPass=false;
                                        bASMFinishOneCycle=false;
                                        bSiteMappingCHKOK=false;

                                        iSiteMappingErrorCT++;
                                    }
                                    else
                                    {
                                        bAutoSiteMapWaitTestPass=true;
                                    }

                                    if(iTesterBIN[i][j]==1 && bSiteMappingHasError==true)
                                    {
                                        bAutoSiteMapAmbientResultCheck=true;    //Ifor 20190528 : add Site Mapping Ambient Check
                                    }
                                    else
                                    {
                                        bAutoSiteMapAmbientResultCheck=false;   //Ifor 20190528 : add Site Mapping Ambient Check
                                    }

                                    if(iSiteMappingErrorCT==0)                  //Ifor 20181112 避免 iDoSiteMappingStep 一直++
                                    {
                                        bAddSitemapStep=true;
                                    }
                                    else
                                    {
                                        bAddSitemapStep=false;
                                        if(iSiteMappingErrorCT>=3)              //Ifor 20180417 : add ASE_M要求錯誤三次才Alarm
                                        {
                                            iSiteMappingErrorCT=0;
                                            #ifndef DEBUG_AutoSiteMap
                                            ShowMyMessage("Site Mapping Check Fail! Must Pass Bin,need Do again!",
                                                          "Site Mapping 確認失敗!必須 Pass Bin,需要再做一次!",
                                                          IndexSuckName[i][j]);                                         //Steven 20220526 : 紀錄Auto Site Map Fail的位置
                                            #endif
                                        }
                                    }

                                    if(IniConfig.bI19AuToSitMapPauseWaitBin)
                                        fMain->Pause("bI19AuToSitMapPauseWaitBin");                                     //kevin 20150119 buff
                                }
                                else
                                {
                                    if(bGetOpenBin==true)                       //checkautoSitemap
                                    {
                                        if(IniConfig.bI21ASMNeedCheckEachSiteOpen &&                                    //Steven 20120726 : AutoSiteMapping, 當確認Open Bin時,同時也要檢查是不是所有Dut都Open
                                           CUSTOMER_CODE!=CC_SCS)               //jou 20220926 : 修正JSCS auto site mapping hang up
                                        {
                                            if(iOpenBin==-1)
                                            {
                                                if(LastSet.iTester==OFF_LINE)   //Steven 20150713 : 整理LastSet.iTester
                                                    iOpenBin=2;                 //Steven 20110422 : 離線模式強制為2
                                                else
                                                    iOpenBin=iTesterBIN[i][j];
                                            }
                                            else
                                            {
                                                if(LastSet.iTester==ON_LINE && iOpenBin!=iTesterBIN[i][j])
                                                {
                                                    fMain->ReStartAutoSiteMapping(true);
#if 0 // TODO(W906-GB-P2b): R07 fMain->AutoSiteMappignCleanOut is not a TfMain member (forms/fMain.h; golden 912 main.h:1334) -- golden atester.cpp:3465 (912; 906 :3445)
                                                    fMain->AutoSiteMappignCleanOut(true);
#endif
                                                    ShowMyMessage("Site mapping get open bin fail! Need check load board!",
                                                                  "Site Mapping 取得Open Bin失敗, 需要檢查Load board!");

                                                    if(ATC_SYSTEM==eNewATCSystem && Temperature.bATCActiveCooling==true)                                        //JerryYang 20220815 : send ATC start testing
                                                    {
#if 0 // TODO(W906-GB-P2b): R04 ATC_InterfaceForm is not reachable from atester.cpp's includes (only acarry_shims.h:115 TATC_InterfaceFormShim, iATC_MODE_TYPE only; real class forms/fATCHandlerSide.h:845/:847 has no global) -- golden atester.cpp:3471 (912; 906 :3451)
                                                        ATC_InterfaceForm->TestFinish();
                                                        ATC_InterfaceForm->HandlerArm(-1);                              //Ifor 20220107 add:通知ATC目前哪隻Arm再Socket
#endif
                                                    }
                                                    TriggerATC_FFC_Function();  //Ifor 20240507 add :FFC Trigger Even Off
                                                    return 1;
                                                }
                                            }
                                        }

                                        if(LastSet.iTester==OFF_LINE)           //Steven 20150713 : 整理LastSet.iTester
                                            Prod.iOpenBin=2;                    //Steven 20110422 : 離線模式強制為2
                                        else
                                            Prod.iOpenBin=iTesterBIN[i][j];
#if 0 // TODO(W906-GB-P2b): R08 fMain->SetOpenBin is not a TfMain member (forms/fMain.h; golden 912 main.h:1337, body main.cpp:9516-9519) -- golden atester.cpp:3484 (912; 906 :3464)
                                        fMain->SetOpenBin();
#endif

                                        if(IniConfig.bI19AuToSitMapPauseWaitBin)
                                            fMain->Pause("bI19AuToSitMapPauseWaitBin");                                 //kevin 20150119 buff
                                    }
                                    else
                                    {
                                        iGetTestData[i][j]=iTesterBIN[i][j]+1;

                                        if(iTesterBIN[i][j]==Prod.iOpenBin)
                                        {
                                            #ifndef ASE_KaohSiung               //不是高雄的話
                                                fMain->ReStartAutoSiteMapping(true);
#if 0 // TODO(W906-GB-P2b): R07 fMain->AutoSiteMappignCleanOut is not a TfMain member (forms/fMain.h; golden 912 main.h:1334) -- golden atester.cpp:3497 (912; 906 :3477)
                                                fMain->AutoSiteMappignCleanOut(true);
#endif
                                                ShowMyMessage("Site Mapping Check Fail! Have Open Bin,need Do again!",
                                                              "Site Mapping Check 失敗!有Open Bin,需要再做一次!",
                                                              IndexSuckName[i][j]);                                     //Steven 20220526 : 紀錄Auto Site Map Fail的位置
                                            #else                               //kevin 20150113
                                                ShowMyMessage("Site Mapping Check Fail! Have Open Bin,need Do again!",
                                                              "Site Mapping Check 失敗!有Open Bin,需要再做一次!",
                                                              IndexSuckName[i][j]);                                     //Steven 20220526 : 紀錄Auto Site Map Fail的位置
                                                bShowAutoSiteMappingError=true;                                         //20150115
                                                bSiteMappingCHKOK=false;
                                            #endif
                                        }

                                        if(IniConfig.bI19AuToSitMapPauseWaitBin)
                                            fMain->Pause("bI19AuToSitMapPauseWaitBin");                                 //kevin 20150119 buff
                                    }
                                }
                            }
                        }
                    }
                }

                if(CUSTOMER_CODE==CC_ASE_M && bHasTestIC==true)
                {
                    DoJCETSiteMappingCHK(bAddSitemapStep);                      //Ifor 20181106 往下移至迴圈外避免Step錯亂
                }
                else if(IniConfig.bVTESTFunction==true)
                {
                    if(bHasTestIC==true)
                    {
                        bAutoSiteMapWaitTestResult=false;

                        if(bAutoSiteMapWaitTestPass==false)
                        {
                            bASMFinishOneCycle=false;
                            bSiteMappingCHKOK=false;
                            bAddSitemapStep=false;
                            ShowMyMessage("Site Mapping Check Fail! Must Pass Bin,need Do again!",
                                          "Site Mapping 確認失敗!必須 Pass Bin,需要再做一次!");
                            iAutoSiteMappingErrCT++;
                            if(iAutoSiteMappingErrCT>=IniConfig.iI21AutoSiteMappingErrCT)
                            {
                                iAutoSiteMappingErrCT=0;
                                fMain->BtnOneCycleClick(fMain->BtnOneCycle);
                            }
                        }
                        else
                        {
                            iAutoSiteMappingErrCT=0;
                            bAddSitemapStep=true;
                        }
                        DoJCETSiteMappingCHK(bAddSitemapStep);                  //Ifor 20181106 往下移至迴圈外避免Step錯亂
                    }
                    else if(bHasTestIC==false && bAddSitemapStep==false && iAutoSiteMappingErrCT==0 && iAutoSiteMapRunStartMode==0)                             //FT
                    {
                        bAutoSiteMapWaitTestPass=true;
                    }
                }
            }
        }
        else
        {
            if(bInterfaceError==false)                                          //Steven 20190326 : QA Sampling
            {
                ProcessQASampling(Index);
            }
        }

//AI(W906-GB-P2b) 20260926: R09 lifted -- SECSGEM/SecsEventType.h + SecsEventReport.h are included now; golden atester.cpp (912) SECS GetTestResult event
        if(IniConfig.bEnable_SECS_GEM==true)                                    //Steven 20140528 : Secs Gem
            EventReport(SECS_EVENT.GetTestResult);                              //26     Get Test Result
        if(ATC_SYSTEM==eNewATCSystem && Temperature.bATCActiveCooling==true)    //JerryYang 20220815 : send ATC start testing
        {
#if 0 // TODO(W906-GB-P2b): R04 ATC_InterfaceForm is not reachable from atester.cpp's includes (only acarry_shims.h:115 TATC_InterfaceFormShim, iATC_MODE_TYPE only; real class forms/fATCHandlerSide.h:845/:847 has no global) -- golden atester.cpp:3569 (912; 906 :3549)
            ATC_InterfaceForm->TestFinish();
            ATC_InterfaceForm->HandlerArm(-1);                                  //Ifor 20220107 add:通知ATC目前哪隻Arm再Socket
#endif
            TriggerATC_FFC_Function();                                          //Ifor 20240507 add :FFC Trigger Even Off
        }

        if(IniConfig.bL22Enable3SigmaTempMonitor==true &&                       //kevin 20200521 add 3Sigma Copy
           Temperature.b3SigmaTempMonitior_Enable==true)                        //Hmy 20200510 Add Enable 3 Sigma Temp Monitor
        {
            #ifdef SOFT_SIMULTE
#if 0 // TODO(W906-GB-P2b): R10 fLotInfo->WhenTestRecordTemperatureLog_3Sigma is not a TfLotInfo member (forms/fLotInfo.h:1073); SAFETY: it writes D:\HT9045_Log\Sigma -- golden atester.cpp:3578 (912; 906 :3558)
                fLotInfo->WhenTestRecordTemperatureLog_3Sigma("D:\\HT9045_Log\\Sigma",Index);
#endif
            #else
#if 0 // TODO(W906-GB-P2b): R10 fLotInfo->WhenTestRecordTemperatureLog_3Sigma is not a TfLotInfo member (forms/fLotInfo.h:1073); SAFETY: it writes D:\HT9045_Log\Sigma -- golden atester.cpp:3580 (912; 906 :3560)
                if(iATCOnLine==1 && (ATC_SYSTEM==1 || ATC_SYSTEM==eNewATCSystem))                                       //kevin 20200618 mark
                    fLotInfo->WhenTestRecordTemperatureLog_3Sigma("D:\\HT9045_Log\\Sigma", Index);
#endif
            #endif
        }
        return 1;
    }

    if(TestSocket.All_HAS_NULL_IC())                                            //Steven 20180910 : Prevent hang up
    {
        DoCheckHasTestTempChange();                                             //Ifor 20230505 add: 確認是否測試中有切換溫度並切回原生產溫度
        if(ATC_SYSTEM==eNewATCSystem && Temperature.bATCActiveCooling==true)    //JerryYang 20220815 : send ATC start testing
        {
#if 0 // TODO(W906-GB-P2b): R04 ATC_InterfaceForm is not reachable from atester.cpp's includes (only acarry_shims.h:115 TATC_InterfaceFormShim, iATC_MODE_TYPE only; real class forms/fATCHandlerSide.h:845/:847 has no global) -- golden atester.cpp:3592 (912; 906 :3572)
            ATC_InterfaceForm->TestFinish();
            ATC_InterfaceForm->HandlerArm(-1);                                  //Ifor 20220107 add:通知ATC目前哪隻Arm再Socket
#endif
            TriggerATC_FFC_Function();                                          //Ifor 20240507 add :FFC Trigger Even Off
        }
        return 1;
    }
    return 0;
}
//******************************************************************************
//
//  注意!! ProcessTestResult為Handler收發BIN相關, 修改時要小心!!
//
//******************************************************************************
//------------------------------------------------------------------------------
//  ProcessTesterTimeOut (golden 912 atester.cpp:3614-3879 / golden 906 :3594-3849) -- tester timeout handler.
//------------------------------------------------------------------------------
//  AI(W906-GB-P2b) 20260926: LIVE golden body -- tester-comm stage P2b (user ruling 20260926:
//  "由 St02 直接進行這一部分的移植工作").  The former GOLDEN VERBATIM PAIR (inert G-PTk3 block,
//  re-verified byte-identical to golden 906 :3594-3849, + the slim stub that returned 0) is now
//  this ONE function; the slim stub is retired.
//  BASE / TARGET (user ruling 20260926 "912是新版本，可以拿906的補充912的就好"): golden 906 text +
//  912's additions.  906->912 diff of this function = ONE hunk, golden 912 :3826-3835 ([P65] QA
//  flags reset on Test Timeout Skip, Ifor 20260831), inserted after the MSG_CMD_TimeOutSkip send.
//
//  GATE REGISTER (every #if 0 in this function; golden text kept inside; lines = golden 912)
//   O01 :3703-3707  SPEA arm of the Retry path.  _RunTestProgram_BarMess / _RunTestProgram are declared
//                   only in Interface/InterfaceSYS.h:353-354 (not in atester.cpp's include closure) and
//                   golden passes "" to a Byte* parameter (ill-formed in C++17).  They are the SPEA
//                   Interface-program WM_COPYDATA builders (InterfaceSYS.cpp:224/:248), NOT TTL direct-
//                   card entry points; SPEA is outside tester-comm scope (TESTERCOMM_PORT_LEDGER P2a
//                   note 3).  The if/else ladder stays golden: a SPEA machine takes the now-empty arm.
//   O02 :3808-3812  same as O01, Skip path.
//   O03 :3831       bP65CanRunQA -- not declared in V906 (golden 912 cmydef.h:6038 / cmydef.cpp:6018).
//   O04 :3832       bP65QAhasTouchDown -- not declared in V906 (golden 912 cmydef.h:6039 / :6019).
//                   (P2c ported only bP65QAING, cmydef.h:5927; its reset here IS live.)
//   O05 :3844       fMain->cbUserSelectChange -- not a TfMain member (forms/fMain.h :163-1273).
//   O06 :3845-3846  fSecurity -- only in forms/fSecurity.h:609 (not included; incomplete type).
//                   #else ret=0 = permission denied = what V906's TfSecurity::Insufficient(177) answers
//                   today (cSecurity.cpp:650-667) = the fail-closed "forced false" idiom used by the
//                   other Insufficient gates.  Consequence: the I49 off-line clean-out never starts.
//   O07 :3860-3871  SAFETY: production recipe write DataPath+<recipe>\Contact.Data (backup + raise).
//                   Gated from bRestModeBackupParm=true: that flag alone makes csystem.cpp:3106-3117
//                   (live) rewrite Contact.Data at reset end.  Unreachable anyway while O06 denies.
//   O08 :3872       fContact->ReadFile -- TfContactShim (atester_shims.h:154-251) has no ReadFile; the real
//                   TfContact::ReadFile (forms/fContact.h:1495) is only reachable as fContactForm (:1629).
//  NON-GATE ADAPTATION
//   N1  three block-scope `extern const unsigned int MSG_CMD_*` at the top of the body: real constants
//       (MessageDef.h:96-98, defined MessageDef.cpp:64-66 in ht9045_globals) whose header atester.cpp
//       does not reach -- same idiom as aTester_Front.cpp:10255 / aTester_Rear.cpp:2964.  Delete them
//       if `#include "MessageDef.h"` is ever added to this file.
//  RETURN VALUE (live callers aTester_Front.cpp:4276, aTester_Rear.cpp:4128, atester_32Site.cpp:489/:3371)
//   2 = Retry, 1 = a result was taken (caller then runs ProcessTestResult), 0 = Skip / Home.  The retired
//   stub always returned 0 (= Skip).  Unattended, ShowErrorMessage answers K_RETRY (canary_support.cpp
//   :111), so an ON_LINE GPIB/TCP-IP timeout now returns 2 where the stub skipped; with the wb_serve
//   hook installed the operator's button decides.
//  GOLDEN QUIRKS KEPT (look odd; not "fixed")
//   Q1  the Front/Rear/32Site:3371 callers clear bEcho before the call, so the `bEcho && ...` Retry arm
//       (return 1) is taken only if an ECHO arrives while the alarm is up.
//   Q2  asTestTimeOut is never used; `ret` is reused for the YES/NO answer and every Skip path returns 0.
//   Q3  "//ret==2==Skip": the else arm also takes K_HOME (and any other answer).
//   Q4  the last else of the Retry path (TTL_CARD_TYPE<2, RS232_MODE, off-line GPIB) never re-sends SOT.
//------------------------------------------------------------------------------
int ProcessTesterTimeOut(int Index)
{
    extern const unsigned int MSG_CMD_TimeOutSkip;                              //AI(W906-GB-P2b) 20260926: N1 block-scope extern of MessageDef.h:96 (defined MessageDef.cpp:64 =11); atester.cpp does not reach MessageDef.h
    extern const unsigned int MSG_CMD_TimeOutRetryWait;                         //AI(W906-GB-P2b) 20260926: N1 MessageDef.h:97 / MessageDef.cpp:65 (=12)
    extern const unsigned int MSG_CMD_TimeOutRetrySend;                         //AI(W906-GB-P2b) 20260926: N1 MessageDef.h:98 / MessageDef.cpp:66 (=13)
    int ret=0, i=0, j=0;
    AnsiString asTestTimeOut="";
    int iMot=MTestZ1;

    if(Index==1)                                                                //kevin 20161105 add
        iMot=MTestZ2;

    if(CUSTOMER_CODE==CC_KYEC_LEE ||
       CUSTOMER_CODE==CC_KYEC_XILINX ||
       CUSTOMER_CODE==CC_KYEC_CHEN ||
       CUSTOMER_CODE==CC_SIGURD_ChungXing ||                                    //wei 20150226 Time Out 只能SKIP
       CUSTOMER_CODE==CC_ASE_SG ||                                              //Ifor 20250926 add ASESG
       CosFunction.bTempAlarmBinNeedToError)                                    //Steven 20251022 : Temp alarm need put to error bin
    {
//        fMain->RunTestProgram(false);                                         //wei 20150226 Time Out 就停止測試
//        ret=ShowErrorMessage("WAR07352", K_SKIP, MMInterface, bTestDuplicateErr);
        if(bATCHasAlarmBinNeedToError==true)                                    //Ifor 20160726 add 發生 ATC 異常時需將測中IC放至Error Bin
        {
            bATCHasAlarmBinNeedToError=false;
            RecordProcess("During Test,ATC Alarm Set to Error Bin");
            ret=K_SKIP;
        }
        else
        {
            fMain->RunTestProgram(false);                                       //wei 20150226 Time Out 就停止測試
            ret=ShowErrorMessage("WAR07352", K_SKIP, MMInterface, bTestDuplicateErr);
        }
    }
    else if(CosFunction.bIndexAreaOnlyCanUseSkip)                               //Steven 20141105 : Index內的所有異常都只能用Skip
    {
        if(IniConfig.bI36TestTimeOut)                                           //kevin 20161105 沒有收到測試資料強制取出ic
            ret=ShowErrorMessage("WAR0716", K_SKIP, iMot, bTestDuplicateErr);
        else if(CosFunction.bTestTimeOutShowSkipAndHome && IniConfig.iI22TestTimeOutOption==3)
            ret=ShowErrorMessage("WAR07352", K_HOME, MMInterface, bTestDuplicateErr);
        else
            ret=ShowErrorMessage("WAR07352", K_SKIP, MMInterface, bTestDuplicateErr);
    }
    else
    {
        if(IniConfig.iI22TestTimeOutOption==0)
        {
            ret=ShowErrorMessage("WAR07352", K_SKIP, MMInterface, bTestDuplicateErr);
        }
        else if(IniConfig.iI22TestTimeOutOption==1)
        {
            ret=ShowErrorMessage("WAR07352", K_RETRY, MMInterface, bTestDuplicateErr);
        }
        else if(IniConfig.iI22TestTimeOutOption==2)
        {
            ret=ShowErrorMessage("WAR07352", K_SKIP|K_RETRY, MMInterface, bTestDuplicateErr);
        }
        else
        {
            ret=ShowErrorMessage("WAR07352", K_HOME, MMInterface, bTestDuplicateErr);
        }
    }

    bATCHasAlarmBinNeedToError=false;                                           //kevin 20181011  Ifor 20160726 add 發生 ATC 異常時需將測中IC放至Error Bin
    bTestDuplicateErr=true;
    if(ret==K_RETRY)
    {
        if((TestIF.iTestType==GPIB_MODE ||
            TestIF.iTestType==TCP_IP_MODE) &&                                   //wei 20211027 open short TCP/IP
           LastSet.iTester==ON_LINE)
        {
            if(CUSTOMER_CODE!=CC_ASE_KaohSiung &&
               CUSTOMER_CODE!=CC_ASE_KaohSiung_K12)                             //Steven 20131101 : Add ASE-K12
            {
                if(bEcho && bTimeOutForNoFullSite==false)                       //ChungHung 20141017 fix Full Site Test Time Out Problem
                {
                    iTestTask=60;
                    GetTesterResult(Index);
                    ProcessTestResult(Index);
                    bTestDuplicateErr=false;
                    if((CosFunction.bUSEJCETSiteMapMode==true ||
                        CUSTOMER_CODE==CC_ASE_M) &&                             //Ifor 20180417 : add ASE_M
                       LastSet.iRunStartMode==rsmAutoSiteMap)                   //JerryYang 20170502 (Steven) 修正auto site map hang up問題
                        bAvoidAddDoSiteMappingStep=true;                        //JerryYang 20170316 (Steven) 避免tester time out時Retry會重複進入DoJCETSiteMappingCHK(), 造成auto site mapping一次跳兩顆
                    return 1;
                }
                else                                                            //Jou 20101018 Start
                {
                    if(IniConfig.bI12TesterTimerOutNotNeedReTest==false)
                    {
                        IsTest=false;                                           //Steven 20110722 Start : Skip時,要重置GPIB測試狀態
                        if(TestIF.iGpibMode==InterfaceType_SPEA_Type)
                        {
#if 0 // TODO(W906-GB-P2b): O01 SPEA arm: _RunTestProgram_BarMess/_RunTestProgram are declared only in Interface/InterfaceSYS.h:353-354 (not in atester.cpp's include closure; body InterfaceSYS.cpp:224/:248, SPEA Interface-program WM_COPYDATA, NOT the TTL direct card) and golden passes "" to Byte* (ill-formed in C++17); SPEA is out of tester-comm scope (ledger P2a note 3) -- golden atester.cpp:3703-3707 (912; 906 :3683-3687)
                            bool flag[32]={false};
                            if(BAR_CODE_INSTALL!=ebctUninstall && TestIF_File.bEnableBarCode)
                                _RunTestProgram_BarMess(sizeof(flag), flag, 0, "");
                            else
                                _RunTestProgram(sizeof(flag), flag);
#endif
                        }
                        else
                        {
                            fMain->RunTestProgram(false);
                        }

                        if(CUSTOMER_CODE==CC_AMKOR_China ||                     //Steven 20110125
                           CUSTOMER_CODE==CC_QUALCOMM)                          //JerryYang 20170412 (Steven) add QUALCOMM
                        {
                            fMain->CloseGpibProgram(__FUNC__);
                        }
                        InitTestTask();
                        fMain->SendMSG_CMD(MSG_CMD_TimeOutRetrySend);           //Steven 20150304 : Add GPIB LOG
                    }
                    else
                    {
                        fMain->SendMSG_CMD(MSG_CMD_TimeOutRetryWait);           //Steven 20150304 : Add GPIB LOG
                    }
                }
            }
            return 2;
        }
        else if(TestIF_File.iTestType==TTL_MODE &&
                (TTL_CARD_TYPE==2 || TTL_CARD_TYPE==3))                         //Isaac 20210309 :TTL RS232通訊
        {
            if(Check_TTL_Status(Index))                                         //get TTL data
            {                                                                   //Steven 20101228 Start: TTL TimeOut Hang
                for(i=0; i<TestSocket.iShtRow; i++)
                {
                    for(j=0; j<TestSocket.iShtCol; j++)
                    {
                        if(TestSocket.Item[i][j]!=NULL_IC &&
                           TestSocket.Item[i][j]!=HAS_NULL_IC &&
                           iTesterBIN[i][j]!=-1 &&
                           TestSocket.Item[i][j]<TEST_PASS)
                        {
                            TestSocket.SetItemData(i, j, TEST_PASS+iTesterBIN[i][j]);
                            TestSocket.iBinData[i][j]=iTestBinCount;            //Steven 20190116 : 修正顯示錯誤
                            TestSocket.PordRec[i][j].AddTestResultRecord(iTesterBIN[i][j], TestSocket.cSBin[i][j], "TestTimeOut_RETRY");                        //Frank 20160505 add
                        }
                    }
                }
                bTestDuplicateErr=false;
                return 1;
            }

            if(IniConfig.bI12TesterTimerOutNotNeedReTest==false)                //resend SOT
            {
                IsTest=false;                                                   //Steven 20110722 Start : Skip時,要重置GPIB測試狀態
                fMain->RunTestProgram(false);
                InitTestTask();
                SendTTLRS232CSOTsignal();
                fMain->SendMSG_CMD(MSG_CMD_TimeOutRetrySend);                   //Steven 20150304 : Add GPIB LOG
            }
            else
            {
                fMain->SendMSG_CMD(MSG_CMD_TimeOutRetryWait);                   //Steven 20150304 : Add GPIB LOG
            }
            return 2;
        }
        else
        {
            if(Check_TTL_Status(Index))                                         //get TTL data
            {                                                                   //Steven 20101228 Start: TTL TimeOut Hang
                for(i=0; i<TestSocket.iShtRow; i++)
                {
                    for(j=0; j<TestSocket.iShtCol; j++)
                    {
                        if(TestSocket.Item[i][j]!=NULL_IC &&
                           TestSocket.Item[i][j]!=HAS_NULL_IC &&
                           iTesterBIN[i][j]!=-1 &&
                           TestSocket.Item[i][j]<TEST_PASS)
                        {
                            TestSocket.SetItemData(i, j, TEST_PASS+iTesterBIN[i][j]);
                            TestSocket.iBinData[i][j]=iTestBinCount;            //Steven 20190116 : 修正顯示錯誤
                            TestSocket.PordRec[i][j].AddTestResultRecord(iTesterBIN[i][j], TestSocket.cSBin[i][j], "TestTimeOut_RETRY");                        //Frank 20160505 add
                        }
                    }
                }
                bTestDuplicateErr=false;
                return 1;
            }
            else
            {
                return 2;
            }
        }
    }
    else                                                                        //ret==2==Skip
    {
        if(ret==K_HOME)
        {
            fAllMotorHome=false;
            bI22_NeedHomeDelay=true;
        }

        TestProcessSetToErr("Test Time Out Skip All Place to R bin");

        if(TestIF.iGpibMode==InterfaceType_SPEA_Type)                           //Steven 20141209 : Fixed Time Out Skip
        {
#if 0 // TODO(W906-GB-P2b): O02 same as O01 (SPEA _RunTestProgram_BarMess/_RunTestProgram, Interface/InterfaceSYS.h:353-354 not reachable; "" -> Byte*) -- golden atester.cpp:3808-3812 (912; 906 :3788-3792)
            bool flag[32]={false};
            if(BAR_CODE_INSTALL!=ebctUninstall && TestIF_File.bEnableBarCode)   //Ifor 20190129 : add Cognex EtherNet 通訊  //Ifor 20210407 add: 自製OCR
                _RunTestProgram_BarMess(sizeof(flag), flag, 0, "");
            else
                _RunTestProgram(sizeof(flag), flag);
#endif
        }
        else if(TestIF_File.iTestType==TTL_MODE &&
                (TTL_CARD_TYPE==2 || TTL_CARD_TYPE==3))                         //Isaac 20210309 :TTL RS232通訊
        {
            fMain->RunTestProgram(false);
            SendTTLRS232CSOTsignal();
        }
        else
        {
            fMain->RunTestProgram(false);
        }

        fMain->SendMSG_CMD(MSG_CMD_TimeOutSkip);                                //Steven 20150304 : Add GPIB LOG
        //AI(W906-GB-P2b) 20260926: golden 912 atester.cpp:3826-3835 -- the only 906->912 change in this function (absent in 906); O03/O04 gate its two missing flags
        //Ifor 20260831: [P65] QA cycle aborted by timeout skip, reset flags so next SOT sends 0x41 (not 0x42)
        //==>
        if(IniConfig.bP65EnableArmQAMode)
        {
            bP65QAING=false;
#if 0 // TODO(W906-GB-P2b): O03 bP65CanRunQA not declared anywhere in V906 (grep cmydef.h/.cpp + whole tree: 0 hits; golden 912 cmydef.h:6038 / cmydef.cpp:6018; P2c ported only bP65QAING) -- golden atester.cpp:3831 (912)
            bP65CanRunQA=false;
#endif
#if 0 // TODO(W906-GB-P2b): O04 bP65QAhasTouchDown not declared anywhere in V906 (0 hits; golden 912 cmydef.h:6039 / cmydef.cpp:6019) -- golden atester.cpp:3832 (912)
            bP65QAhasTouchDown=false;
#endif
            RecordProcess("P65 QA flags reset by Test Timeout Skip");
        }
        //<==
    }

    if(IniConfig.bI49_TesterTimeOutResetAllIC)                                  //Sam 20240215 : Tester time out show reset all ic
    {
        bResetNotMsg=true;
        ret=ShowMyMessageBox_YES_NO("Do you want to enable off-line clean unit mode.", "你確定要執行離線清料作業。");
        if(ret==1)                                                              //Sam 20240701 : Offline Clean out ic 新增權限設定
        {
#if 0 // TODO(W906-GB-P2b): O05 fMain->cbUserSelectChange is not a TfMain member (forms/fMain.h class TfMain :163-1273; the only other tree calls, cContactCT.cpp:1217 / cCounterClear.cpp:434, are gated too) -- golden atester.cpp:3844 (912; 906 :3814)
            fMain->cbUserSelectChange(NULL);
#endif
#if 0 // TODO(W906-GB-P2b): O06 fSecurity is declared only in forms/fSecurity.h:609 (TfSecurity :468, Insufficient :575), not in atester.cpp's include closure; incomplete type, so no local extern -- golden atester.cpp:3845-3846 (912; 906 :3815-3816)
            if(fSecurity->Insufficient(177)==false)
                ret=0;
#else
            ret=0;                                                              //AI(W906-GB-P2b) 20260926: O06 = permission DENIED: the answer V906's own TfSecurity::Insufficient(177) gives today (cSecurity.cpp:650-667, iMaxLevelItem==0 -> false) and the fail-closed "forced false" idiom of cContactCT.cpp C3 / cShowBinSelect.cpp B5; so the I49 clean-out below never starts
#endif
        }

        if(ret==1)
        {
            if(DeviceForm.ContactMode==DirectContactMode || DeviceForm.ContactMode==DropContact)
            {
                TrayForm.bAutoFeed=true;
                if(IniConfig.bA10_AutoReTest && TestIF_File.bSCKART_EnableART)
                {
                    LastSet.bBreakSCKART=true;
                    TestIF_File.bSCKART_LotDeviceCheck=false;
                }

#if 0 // TODO(W906-GB-P2b): O07 SAFETY -- WriteIniData x3 rewrite the production recipe file DataPath+<recipe>\Contact.Data (DataPath = D:\HT9045\IniData\Data\ unless W906_INIDATA_ROOT is set, common.cpp:204-225); gated as a group from bRestModeBackupParm=true because the flag alone makes csystem.cpp:3106-3117 (live) write Contact.Data at reset end -- golden atester.cpp:3860-3871 (912; 906 :3830-3841)
                bRestModeBackupParm=true;                                       //Sam 20250820 : [I49] 清料時 Contact Heigh 要拉高
                AnsiString S="";
                AnsiString szDir="";
                S=GetLastOpenFN();
                szDir=DataPath+S;
                szDir+="\\Contact.Data";
                iRestModeBackContactMode=ReadIniData(szDir, "Mode", "Contact", 0);
                dRestModeBackContactHeigh1=ReadIniData(szDir, "Test Arm1", "Contact", 0.0);
                dRestModeBackContactHeigh2=ReadIniData(szDir, "Test Arm2", "Contact", 0.0);
                WriteIniData(szDir, "Mode", "Contact",    DirectContactMode);
                WriteIniData(szDir, "Test Arm1", "Contact", dRestModeBackContactHeigh1+IniConfig.fI49_ChangeAboveSocket);
                WriteIniData(szDir, "Test Arm2", "Contact", dRestModeBackContactHeigh2+IniConfig.fI49_ChangeAboveSocket);
#endif
#if 0 // TODO(W906-GB-P2b): O08 fContact is TfContactShim (atester_shims.h:154-251), which has no ReadFile; the real TfContact::ReadFile (forms/fContact.h:1495) is reachable only as fContactForm (forms/fContact.h:1629, header not included here) -- golden atester.cpp:3872 (912; 906 :3842)
                fContact->ReadFile();
#endif
            }
            fMain->Reset("Reset by ProcessTesterTimeOut");
        }
        bResetNotMsg=false;
    }
    return 0;
}
//------------------------------------------------------------------------------
//  DoInterFaceErrorStep (golden :3856-4103) -- interface-error step SM.
//------------------------------------------------------------------------------
// ===========================================================================
//  GOLDEN VERBATIM PAIR -- DoInterFaceErrorStep
//  GATED : golden atester.cpp:3856-4101 (246 lines), inert reference text.
//  LIVE  : the slim DoInterFaceErrorStep() immediately AFTER the #endif below.  It is
//          UNCHANGED by this wave -- net behaviour change is ZERO.
//  WHY   : the census scored this "translated" because a same-named LIVE body
//          existed, without comparing SIZE.  Golden's 246 lines were NOWHERE in
//          the tree -- lost text, not deferred behaviour.  Now the text EXISTS
//          and is auditable, so a later un-gate is mechanical.
//  RULES : nothing inside the gate is fixed, renamed, reflowed or reindented;
//          golden's own defects and misspellings are preserved so a diff
//          against golden stays EMPTY.  Being gated it needs NO callee to
//          exist -- no stub, declaration or header was added for it.
//  SHAPE : same pair shape as csystem.cpp DoTrayFeedProcess / MainProc /
//          DoAllProcess (PT-W6a) golden-verbatim gates.
// ===========================================================================
#if 0 // GOLDEN VERBATIM -- golden atester.cpp:3856-4101 (246 lines).  GATE G-PTk4-DoInterFaceErrorStep.  NOT COMPILED: the ACTIVE DoInterFaceErrorStep() is the slim body immediately after this #endif.
bool DoInterFaceErrorStep(int ZAxisSelect)
{
    AnsiString asChinese=AnsiString("下壓次數已經超過設定值，請打開Chamber側門並清潔Socket");       //Steven 20230104 : 直接寫死
    AnsiString asEnglish=AnsiString("Contact over setting # Please Open Chamber Side Door and clean socket");           //ChungHung 20121029 alter 客戶會誤解。
    AnsiString ErrPart, str, sErrMsg;                                           //kevin 20161105
    bool flag=false, flag1=false;
    int iMot=MTestZ1;

    int &Task=iDoInterFaceErrorStepTask;

    switch(Task)
    {
        case 1:
            if(ZAxisSelect==TESTZ1UP)
            {
                if(MOT[MTestZ2].Gali_ReadPos()==Prod.TestZ2_Safe)
                {
                    bD52IndexArmUp=false;                                       //JerryYang 20200804 : fix D52 & Index arm在shuttle高度預熱功能同時啟用時，發生tester timeout時會誤發handler hang up
                    if(bContactCTOverCHK)
                    {
                        bContactTimeOverStep=true;
                        if(MOT[MTestZ1].Gali_MotMove(Prod.TestZ1_Safe, MOT[MTestZ1].GailSpeed))
                        {
                            Task=100;
                        }
                    }
                    else
                    {
                        bNowDoInterFaceErrorStep=true;
                        if(MOT[MTestZ1].Gali_MotMove(Prod.All_TestZ_Test_Safe, MOT[MTestZ1].GailSpeed))
                            Task=100;
                    }
                }
            }
            else
            {
                if(MOT[MTestZ1].Gali_ReadPos()==Prod.TestZ1_Safe)
                {
                    bD52IndexArmUp=false;                                       //JerryYang 20200804 : fix D52 & Index arm在shuttle高度預熱功能同時啟用時，發生tester timeout時會誤發handler hang up
                    if(bContactCTOverCHK)
                    {
                        bContactTimeOverStep=true;
                        if(MOT[MTestZ2].Gali_MotMove(Prod.TestZ2_Safe, MOT[MTestZ2].GailSpeed))
                        {
                            Task=100;
                        }
                    }
                    else
                    {
                        bNowDoInterFaceErrorStep=true;
                        if(MOT[MTestZ2].Gali_MotMove(Prod.All_TestZ_Test_Safe, MOT[MTestZ2].GailSpeed))
                            Task=100;
                    }
                }
            }
            break;
        case 100:
            if(bContactCTOverCHK)
            {
                if(TestIF_File.iContactAlarmCount[3]>0)                         //JerryYang 20250120 : modify
                {
                    for(int i=0; i<TestSocket.iShtRow; i++)
                    {
                        for(int j=0; j<TestSocket.iShtCol; j++)
                        {
                            if(LastSet.iSocketContactCount[i][j]>TestIF_File.iContactAlarmCount[3])
                            {
                                str.sprintf("%s:%s,", IndexSuckName[i][j], LastSet.strSocketID[i][j]);
                                sErrMsg+=str;
                            }
                        }
                    }
                }
                ShowErrorMessage("WAR0354", K_RETRY, MTestY1, false, sErrMsg);

                Task=150;
            }
            else
            {
                if(bEchoStop==true)                                             //ChungHung 20130326 add
                {
                    bEchoStop=false;
                    ShowMyMessage("Receive ECHOSTOP Form TESTER");
                    Task=200;
                }
                else
                {
                    ProcessTesterTimeOut(ZAxisSelect);
                    SetNoiseDelay=false;
                    TestISTimeOut=false;
                    ProcessCount(ZAxisSelect, FTestSuck.HasRealIC());           //Eastsun 20260515 F022: D7
                    if(ZAxisSelect==TESTZ1UP)
                    {
                        if(TestSocket.UseSiteHasIC())                           //jou 20230828 : 修正index all drop error
                            FTestSuck.MoveAllItem(TestSocket);
                    }
                    else
                    {
                        if(TestSocket.UseSiteHasIC())                           //jou 20230828 : 修正index all drop error
                            BTestSuck.MoveAllItem(TestSocket);
                    }

                    RecordHistroy(ZAxisSelect);
                    bFinshTest=true;
                    bInitStartDelayNotFinish=true;                              //Ifor 20181220 : add Init Start Delay Time Not Finish
                    bTJControlMode=false;                                       //Ifor 20190328 : add TJ Temp Over Range
                    ATC_InterfaceForm->SendHandler2DID(0, false);
                    SW[SwTesterPower].Off();
                    if(IniConfig.bI36TestTimeOut)                               //kevin 20161105 沒有收到測試資料強至取出ic
                        Task=110;
                    else
                        Task=200;
                }
                break;
            }
            break;
        case 110:                                                               //kevin 20161105 確認ic是否強制取走
            ErrPart=" ";
            for(int i=0; i<FTestSuck.iShtRow; i++)
            {
                for(int j=0; j<FTestSuck.iShtCol; j++)
                {
                    flag1=false;
                    if(ZAxisSelect==TESTZ1UP)
                    {
                        flag1=FTestSuck.Suck[i][j].GetStatus();
                        if(flag1==true)
                        {
                            flag=true;
                            ErrPart+=IndexSuckName[i+IsNNMode()][j];            //Steven 20230712 : 修正NN mode alarm顯示
                        }
                    }
                    else
                    {
                        iMot=MTestZ2;
                        flag1=BTestSuck.Suck[i][j].GetStatus();
                        if(flag1==true)
                        {
                            flag=true;
                            ErrPart+=IndexSuckName[i][j];
                        }
                    }
                }
            }

            if(flag==false)
            {
                if(ZAxisSelect==TESTZ1UP)
                {
                    if(TestSocket.UseSiteHasIC())                               //jou 20230828 : 修正index all drop error
                        FTestSuck.MoveAllItem(TestSocket);
                }
                else
                {
                    if(TestSocket.UseSiteHasIC())                               //jou 20230828 : 修正index all drop error
                        BTestSuck.MoveAllItem(TestSocket);
                }
                Task=200;
            }
            else
            {
                if(IndexAlarmInArmAway()==false)                                //Steven 20130613 : Index異常時, In Arm要先讓位功能
                {
                    return false;
                }
                ShowErrorMessage("WAR0715", K_RETRY, iMot, false, ErrPart);
            }
            break;
        case 150:
            if(bContactCTOverCHK==false)                                        //已經開門了
            {
                Task=170;
            }
            else
            {
                if(TestIF_File.iContactAlarmCount[3]>0)                         //JerryYang 20250120 : modify
                {
                    for(int i=0; i<TestSocket.iShtRow; i++)
                    {
                        for(int j=0; j<TestSocket.iShtCol; j++)
                        {
                            if(LastSet.iSocketContactCount[i][j]>TestIF_File.iContactAlarmCount[3])
                            {
                                str.sprintf("%s:%s,", IndexSuckName[i][j], LastSet.strSocketID[i][j]);
                                sErrMsg+=str;
                            }
                        }
                    }
                }
                ShowErrorMessage("WAR0354", K_RETRY, MTestY1, false, sErrMsg);
//                MyMessageBox->lblChineseMsg->Font->Size=10;
//                ShowMyMessage(asEnglish, asChinese); //jou 2012-06-05
//                MyMessageBox->lblChineseMsg->Font->Size=12;
            }
            break;
        case 170:
            if(ZAxisSelect==TESTZ1UP)
            {
                if(MOT[MTestZ2].Gali_ReadPos()==Prod.TestZ2_Safe)
                {
                    if(MOT[MTestZ1].Gali_MotMove(Prod.TestZ1_Test, MOT[MTestZ1].GailSpeed))
                    {
                        bContactTimeOverStep=false;
                        return true;
                    }
                }
            }
            else
            {
                if(MOT[MTestZ1].Gali_ReadPos()==Prod.TestZ1_Safe)
                {
                    if(MOT[MTestZ2].Gali_MotMove(Prod.TestZ2_Test, MOT[MTestZ2].GailSpeed))
                    {
                        bContactTimeOverStep=false;
                        return true;
                    }
                }
            }
            break;
        case 200:
            if(ZAxisSelect==TESTZ1UP)
            {
                if(MOT[MTestZ2].Gali_ReadPos()==Prod.TestZ2_Safe)
                {
                    if(MOT[MTestZ1].Gali_MotMove(Prod.TestZ1_Test, MOT[MTestZ1].GailSpeed))
                    {
                        bNowDoInterFaceErrorStep=false;
                        return true;
                    }
                }
            }
            else
            {
                if(MOT[MTestZ1].Gali_ReadPos()==Prod.TestZ1_Safe)
                {
                    if(MOT[MTestZ2].Gali_MotMove(Prod.TestZ2_Test, MOT[MTestZ2].GailSpeed))
                    {
                        bNowDoInterFaceErrorStep=false;
                        return true;
                    }
                }
            }
            break;
    }
    return false;
}
#endif // GOLDEN VERBATIM -- golden atester.cpp:3856-4101  (GATE G-PTk4-DoInterFaceErrorStep, end)
bool DoInterFaceErrorStep(int ZAxisSelect)
{
#if 0 // TODO(W7) -- golden :3856-4103 (MOT[] error-recovery SM)
#endif
    (void)ZAxisSelect;
    (void)iDoInterFaceErrorStepTask;
    return false;                                                              // golden default: error-step not finished
}
//------------------------------------------------------------------------------
//  CheckIndexStatus (golden :4104-4236).  Under SOFT_SIMULTE the golden returns
//  true immediately; offline (no SOFT_SIMULTE) the body derefs MOT[].ISZ1Up_
//  Z2DownNoWait() etc.  GATED whole-body; ACTIVE stub returns the offline-safe
//  "in position" (true) so the gated test-head SM treats the index as settled.
//------------------------------------------------------------------------------
// ===========================================================================
//  GOLDEN VERBATIM PAIR -- CheckIndexStatus
//  GATED : golden atester.cpp:4104-4235 (132 lines), inert reference text.
//  LIVE  : the slim CheckIndexStatus() immediately AFTER the #endif below.  It is
//          UNCHANGED by this wave -- net behaviour change is ZERO.
//  WHY   : the census scored this "translated" because a same-named LIVE body
//          existed, without comparing SIZE.  Golden's 132 lines were NOWHERE in
//          the tree -- lost text, not deferred behaviour.  Now the text EXISTS
//          and is auditable, so a later un-gate is mechanical.
//  RULES : nothing inside the gate is fixed, renamed, reflowed or reindented;
//          golden's own defects and misspellings are preserved so a diff
//          against golden stays EMPTY.  Being gated it needs NO callee to
//          exist -- no stub, declaration or header was added for it.
//  SHAPE : same pair shape as csystem.cpp DoTrayFeedProcess / MainProc /
//          DoAllProcess (PT-W6a) golden-verbatim gates.
// ===========================================================================
#if 0 // GOLDEN VERBATIM -- golden atester.cpp:4104-4235 (132 lines).  GATE G-PTk2-CheckIndexStatus.  NOT COMPILED: the ACTIVE CheckIndexStatus() is the slim body immediately after this #endif.
bool CheckIndexStatus(AnsiString str)
{
    #ifdef SOFT_SIMULTE
        return true;
    #else
        static int iRetryCT=0;
        long lPos[4]={0, 0, 0, 0};                                              //kevin 20150915
        iRetryCT++;
        if(IndexStatus==Z1Up_Z2Down)
        {
            if(MOT[MTestZ1].ISZ1Up_Z2DownNoWait())
            {
                iRetryCT=0;
                return true;
            }
            else
            {
                if(iRetryCT>20)
                {
                    lPos[0]=IndexStatus;                                        //kevin 20150915
                    RecordIndexPositionError(AnsiString("CheckIndexStatus()Z1Up_Z2Down_")+str,true,false,false,false,&lPos[0]);                                 //kevin 20150915 record
                    ShowIndexMotorError(AnsiString("CheckIndexStatus1"));
                    iRetryCT=0;                                                 //kevin 20110628 發生alarm 需清為0否則要關程式 //Steven 20180730 : 增加保護
                }
                return false;
            }
        }
        else if(IndexStatus==Z1Down_Z2Up)
        {
            if(MOT[MTestZ1].ISZ1Down_Z2UpNoWait())
            {
                iRetryCT=0;
                return true;
            }
            else
            {
                if(iRetryCT>20)
                {
                    lPos[0]=IndexStatus;                                        //kevin 20150915
                    RecordIndexPositionError(AnsiString("CheckIndexStatus()Z1Down_Z2Up_")+str, true,false,false,false, &lPos[0]);                               //kevin 20150915 record
                    ShowIndexMotorError(AnsiString("CheckIndexStatus2"));
                    iRetryCT=0;                                                 //kevin 20110628 發生alarm 需清為0否則要關程式 //Steven 20180730 : 增加保護
                }
                return false;
            }
        }
        else if(IndexStatus==IndexIsBack)
        {
            if(CheckBackError())
            {
                if(iRetryCT>20)
                {
                    lPos[0]=IndexStatus;                                        //kevin 20150915
                    RecordIndexPositionError(AnsiString("CheckIndexStatus()IndexIsBack_")+str,true,false,false,false,&lPos[0]);                                 //kevin 20150915 record
                    ShowIndexMotorError(AnsiString("CheckIndexStatus3"));
                    iRetryCT=0;                                                 //kevin 20110628 發生alarm 需清為0否則要關程式 //Steven 20180730 : 增加保護
                }
                return false;
            }
        }
        else if(bUseTwoArm32Site==true  &&
                IndexStatus==Z1_Z2_Normal)
        {
            if(MOT[MTestZ1].ISZ1Up_Z2Up())
            {
                iRetryCT=0;
                return true;
            }
            else
            {
                if(iRetryCT>20)
                {
                    lPos[0]=IndexStatus;                                        //kevin 20150915
                    RecordIndexPositionError(AnsiString("CheckIndexStatus()_32Site4X8N Z1_Z2_Normal_")+str,true,false,false,false,&lPos[0]);                    //kevin 20150915 record
                    ShowIndexMotorError(AnsiString("CheckIndexStatus4"));
                    iRetryCT=0;                                                 //kevin 20110628 發生alarm 需清為0否則要關程式 //Steven 20180730 : 增加保護
                }
                return false;
            }
        }
        else if(bUseTwoArm32Site==true  &&
                IndexStatus==Z1_Z2_Down)
        {
            if(MOT[MTestZ1].ISZ1Down_Z2Down())
            {
                iRetryCT=0;
                return true;
            }
            else
            {
                if(iRetryCT>20)
                {
                    lPos[0]=IndexStatus;                                        //kevin 20150915
                    RecordIndexPositionError(AnsiString("CheckIndexStatus()_32Site4X8N Z1_Z2_Down_")+str, true, false, false, false, &lPos[0]);                 //kevin 20150915 record
                    ShowIndexMotorError(AnsiString("CheckIndexStatus5"));
                    iRetryCT=0;                                                 //kevin 20110628 發生alarm 需清為0否則要關程式 //Steven 20180730 : 增加保護
                }
                return false;
            }
        }
        else
        {
            if(MOT[MTestZ1].Gali_ReadEncoderInRandge(Prod.TestZ1_Safe)==false)
            {
                if(iRetryCT>20)
                {
                    lPos[0]=Prod.TestZ1_Safe;                                   //kevin 20150915
                    RecordIndexPositionError(AnsiString("CheckIndexStatus()Prod.TestZ1_Safe_")+str, true, false, false, false, &lPos[0]);                       //kevin 20150915 record

                    ShowIndexMotorError(AnsiString("CheckIndexStatus6"));
                    iRetryCT=0;                                                 //kevin 20110628 發生alarm 需清為0否則要關程式 //Steven 20180730 : 增加保護
                }
                return false;
            }

            if(MOT[MTestZ2].Gali_ReadEncoderInRandge(Prod.TestZ2_Safe)==false)
            {
                if(iRetryCT>20)
                {
                    lPos[0]=Prod.TestZ2_Safe;                                   //kevin 20150915
                    RecordIndexPositionError(AnsiString("CheckIndexStatus()Prod.TestZ2_Safe_")+str, true, false, false, false, &lPos[0]);                       //kevin 20150915 record

                    ShowIndexMotorError(AnsiString("CheckIndexStatus7"));
                    iRetryCT=0;                                                 //kevin 20110628 發生alarm 需清為0否則要關程式 //Steven 20180730 : 增加保護
                }
                return false;
            }
            IndexStatus=Z1_Z2_Normal;
        }
        return true;
    #endif
}
#endif // GOLDEN VERBATIM -- golden atester.cpp:4104-4235  (GATE G-PTk2-CheckIndexStatus, end)
bool CheckIndexStatus(AnsiString str)
{
#if 0 // TODO(W7) -- golden :4104-4236 (MOT[] ISZ1Up_Z2Down/.. position verify)
#endif
    (void)str;
    return true;                                                               // offline: index treated as in position
}
//------------------------------------------------------------------------------
bool CheckShuttlePos()                                                          // golden :4237
{
    for(int i=0; i<2; i++)
    {
        if(MOT[MInShuttle1+i].CompareCommandPos(Prod.InSHT[i].iLeft,  2)!=1 &&  //Sam 20230621 : Gap容許誤差改為1>2 //Isaac 20201217 : 若齒輪比大於1，換算有機會和目標位置差1條 //Ifor 20221117 add: 簡化流程
           MOT[MInShuttle1+i].CompareCommandPos(Prod.InSHT[i].iRight, 2)!=1)
        {
            return false;
        }
    }
    return true;
}
//------------------------------------------------------------------------------
//  CheckPlaceOutShuttle (golden :4250-4297) -- MOT[]/shuttle place verify.
//------------------------------------------------------------------------------
// ===========================================================================
//  GOLDEN VERBATIM PAIR -- CheckPlaceOutShuttle
//  GATED : golden atester.cpp:4250-4293 (44 lines), inert reference text.
//  LIVE  : the slim CheckPlaceOutShuttle() immediately AFTER the #endif below.  It is
//          UNCHANGED by this wave -- net behaviour change is ZERO.
//  WHY   : the census scored this "translated" because a same-named LIVE body
//          existed, without comparing SIZE.  Golden's 44 lines were NOWHERE in
//          the tree -- lost text, not deferred behaviour.  Now the text EXISTS
//          and is auditable, so a later un-gate is mechanical.
//  RULES : nothing inside the gate is fixed, renamed, reflowed or reindented;
//          golden's own defects and misspellings are preserved so a diff
//          against golden stays EMPTY.  Being gated it needs NO callee to
//          exist -- no stub, declaration or header was added for it.
//  SHAPE : same pair shape as csystem.cpp DoTrayFeedProcess / MainProc /
//          DoAllProcess (PT-W6a) golden-verbatim gates.
// ===========================================================================
#if 0 // GOLDEN VERBATIM -- golden atester.cpp:4250-4293 (44 lines).  GATE G-PTk2-CheckPlaceOutShuttle.  NOT COMPILED: the ACTIVE CheckPlaceOutShuttle() is the slim body immediately after this #endif.
bool CheckPlaceOutShuttle(int iShuttle)                                         //Steven 20181228 : Add Index Action  //ChungHung 20171116 modify for Index Action
{
    if(iShuttle==0)
    {
        if((FTestSuck.UseSiteHasIC() &&
            FTestSuck.AlreadyTest()) ||
           FTestNeedDestroy())
        {
            if(FRCarryKit.UseSiteHasIC())
                return false;

            MOT[MInShuttle1].ScanMotorStatus();
            if(InShtInLF(0)==false || MOT[MInShuttle1].Led[iInposLed])
                return false;

            MOT[MInShuttle1].fCanMoveM=false;
        }
        else
        {
            return false;
        }
    }
    else
    {
        if((BTestSuck.UseSiteHasIC() &&
            BTestSuck.AlreadyTest()) ||
           BTestNeedDestroy())
        {
            if(BRCarryKit.UseSiteHasIC())
                return false;

            MOT[MInShuttle2].ScanMotorStatus();
            if(InShtInLF(1)==false || MOT[MInShuttle2].Led[iInposLed])
                return false;

            MOT[MInShuttle2].fCanMoveM=false;
        }
        else
        {
            return false;
        }
    }
    return true;
}
#endif // GOLDEN VERBATIM -- golden atester.cpp:4250-4293  (GATE G-PTk2-CheckPlaceOutShuttle, end)
bool CheckPlaceOutShuttle(int iShuttle)                                         //Steven 20181228 : Add Index Action
{
#if 0 // TODO(W7) -- golden :4250-4297 (MOT[]/shuttle place verify)
#endif
    (void)iShuttle;
    return false;                                                              // golden default
}
//------------------------------------------------------------------------------
void InitCheckSocketHasIC()                                                     // golden :4298
{
    iCheckSocketHasIC=1;
}
//------------------------------------------------------------------------------
//  DoCheckSocketHasIC (golden :4305-4783) -- the dense MOT[]/vacuum socket-IC
//  check SM (Gali_Two_ZAxis_Move trees per index pos).  GATED whole-body;
//  ACTIVE stub keeps cursor + returns golden "check not finished" (false).
//------------------------------------------------------------------------------
// ===========================================================================
//  GOLDEN VERBATIM PAIR -- DoCheckSocketHasIC
//  GATED : golden atester.cpp:4305-4781 (477 lines), inert reference text.
//  LIVE  : the slim DoCheckSocketHasIC() immediately AFTER the #endif below.  It is
//          UNCHANGED by this wave -- net behaviour change is ZERO.
//  WHY   : the census scored this "translated" because a same-named LIVE body
//          existed, without comparing SIZE.  Golden's 477 lines were NOWHERE in
//          the tree -- lost text, not deferred behaviour.  Now the text EXISTS
//          and is auditable, so a later un-gate is mechanical.
//  RULES : nothing inside the gate is fixed, renamed, reflowed or reindented;
//          golden's own defects and misspellings are preserved so a diff
//          against golden stays EMPTY.  Being gated it needs NO callee to
//          exist -- no stub, declaration or header was added for it.
//  SHAPE : same pair shape as csystem.cpp DoTrayFeedProcess / MainProc /
//          DoAllProcess (PT-W6a) golden-verbatim gates.
// ===========================================================================
#if 0 // GOLDEN VERBATIM -- golden atester.cpp:4305-4781 (477 lines).  GATE G-PTk3-DoCheckSocketHasIC.  NOT COMPILED: the ACTIVE DoCheckSocketHasIC() is the slim body immediately after this #endif.
bool DoCheckSocketHasIC(int iSelArm)                                            //jou 20180802 : index pick error need index vaccum check
{
    static int iSiteCount=0;                                                    //kevin 20190709  index check one by one
    static AnsiString ErrPart="";

    int iIndexUpPos=0;
    int &Task=iCheckSocketHasIC;
    int iIndexCheckOffSet=IniConfig.fIndexCheckOffset*100;                      //ChungHung 20140807 add for ATK TestZ_Test + fIndexCheckOffset
    bool flag, flag2;
    AnsiString str;

    if(CUSTOMER_CODE==CC_SIGURD_HUKOU)                                          //KaiChen 20200826 ：矽格-湖口，要求IndexCheck使用Contact高度不使用Offset
    {
        iIndexArmCheck_SG_Arm1=Offset.iIndexArmContact[0];
        iIndexArmCheck_SG_Arm2=Offset.iIndexArmContact[1];
    }
    else
    {
        iIndexArmCheck_SG_Arm1=0;
        iIndexArmCheck_SG_Arm2=0;
    }

    if(Task!=1)                                                                 //Steven 20210413 : 死雞保護
    {
        if(bHandlerPause)
            CheckSocketHasICHangUpCheck.SetSecAndOn(300);
        if(CheckSocketHasICHangUpCheck.Off())
        {
            RecordProcess("Auto State Record by DoCheckSocketHasIC");
            if(Task<2000)
                fMain->DoStateRecord(1);                                        //Steven 20220716 : 把State record獨立出來, 避免抓圖的時候被Alarm擋住
            else
                fMain->DoStateRecord(2);                                        //Steven 20220716 : 把State record獨立出來, 避免抓圖的時候被Alarm擋住
        }
    }

    switch(Task)
    {
        case 1:
            if(LastSet.bD41TestSocketICCheckSkip==true || bCheckIndex==true)
                return true;

            CheckSocketHasICHangUpCheck.SetSecAndOn(300);                                                                                                       //Steven 20210413 : 死雞保護
            if(TestIF_File.iShuttleMode==1 && IniConfig.bShuttleMode50==false && TestIF_File.iShuttle_Sel==1 && Z1Safe==0)                                      //jou 2014-12-19 增加index arm轉換保護
                Task=2000;
            else if(iSelArm==2)                                                                                                                                 //jou 20180802 : index pick error need index vaccum check
                Task=2000;
            else
                Task=1000;
            break;
        case 1000:                                                              //Arm 1 start
            if(MOT[MTestZ1].Gali_Two_ZAxis_Move(Prod.TestZ1_Safe, iSpeedSlow, "DoCheckSocketHasIC 319"))
            {
                Task=1001;
            }
            break;
        case 1001:                                                                                                      //Steven 20220721 : Alarm之前, Index要先讓開
            if(MOT[MTestY1].GalilTwoY_Move(Prod.TestY1_Front, Prod.TestY2_Rear, iSpeedY, "DoCheckSocketHasIC 1010"))
            {
                if(IniConfig.bC08_SocketSensor && TestIF_File.bEnSocketSensor &&
                   (TestIF_File.bSocketDisibleinitialcheck==false ||                                                    //JerryYang 20201101 : Index check下壓前, 要先檢查socket Sensor
                    CUSTOMER_CODE==CC_GIGAS))                                                                           //Isaac 20220126 : 全智要求Index arm在上要強制偵測sensor(應該為off)
                {
                    flag=false;
                    str="check Socket Sensor state Up off,sensor : ";
                    for(int i=0; i<TestIF_File.iSocketCount; i++)
                    {
                        if(Sen[SThreadPara.iSocketSensor[i]].Enable && Sen[SThreadPara.iSocketSensor[i]].IsOn())        //kevin 20150429
                        {
                            flag=true;
                            bIsSocketSensor=true;
                            str+=IntToStr(i+1);
                        }
                    }

                    if(flag)
                    {
                        ShowErrorMessage("WAR0322", K_RETRY, MTestZ1,false, str);                                       //kevin 20130504 socket sensor
                        break;                                                                                          //kevin 20150429確認 socket sensor是否正常
                    }
                }
                Task=1010;
            }
            break;
        case 1010:
            if(MOT[MTestY1].GalilTwoY_Move(Prod.TestY1_Middle, Prod.TestY2_Rear, iSpeedY, "DoCheckSocketHasIC 1010"))
            {
                Task=1020;
                if(ArmSpeed_File[IndexArm].bDevicConfirm &&
                   INDEX_SUCKER_TYPE==1)                                        //kevin 20190912 add 一次開4 site check
                {
                    Task=1050;
                }
            }
            break;
        case 1020:
            if(MOT[MTestZ1].Gali_MotMove(Prod.TestZ1_Test-Prod.TestZ1_Drop_Offset+iIndexCheckOffSet-iIndexArmCheck_SG_Arm1, iSpeedSlow))                        //KaiChen 20200826 ：矽格-湖口，要求IndexCheck使用Contact高度不使用Offset
            {
                IndexStatus=Z1Down_Z2Up;
                Task=1030;
            }
            break;
        case 1030:
            for(int i=0; i<FTestSuck.iShtRow; i++)
            {
                for(int j=0; j<FTestSuck.iShtCol; j++)
                {
                    FTestSuck.Suck[i][j].On();
                    if(INDEX_SUCKER_TYPE==1)
                    {
                        fiosetview->bIndexSuck[0][i][j]=true;
                        bIndexCheckNoStopVaccum=true;
                    }
                }
            }

            CheckSocketHasICDelay.SetSecAndOn(0.5);
            Task=1040;
            break;
        case 1040:                                                              //jou 2012-05-09 Index Check時,如果中途按暫停,也要繼續把suck()做完,避免負壓掉ic
            if(INDEX_SUCKER_TYPE==1)
            {
                fiosetview->ProcessIndexSuckDestroy1();
            }

            if(CheckSocketHasICDelay.Off())
            {
                Task=1050;
            }
            break;
        case 1050:
            iIndexUpPos=GetSocketCheckPos(Prod.TestZ1_Test);                                                                                                    //Steven 20140620 : 整合為Function

            if(MOT[MTestZ1].Gali_MotMove(Prod.TestZ1_Test-Prod.TestZ1_Drop_Offset+iIndexUpPos+iIndexCheckOffSet-iIndexArmCheck_SG_Arm1, iSpeedFast))            //KaiChen 20200826 ：矽格-湖口，要求IndexCheck使用Contact高度不使用Offset
            {
                IndexStatus=Z1Down_Z2Up;                                                                                                                        //kevin 20190912 ???
                if(ArmSpeed_File[IndexArm].bDevicConfirm &&                                                                                                     //kevin 20190709 add 20190629 回吸檢測一次 4 個 SITE
                   INDEX_SUCKER_TYPE==1)                                                                                                                        //kevin 20190530 add index check
                {
                    iSiteCount=0;
                    Task=1055;
                }
                else
                {
                    Task=1060;
                }

                if(CUSTOMER_CODE==CC_Greatek)                                                                                                                   //Wei 20160413
                    CheckSocketHasICDelay.SetSecAndOn(5);                                                                                                       //Steven 20110908 : 上來後也要Delay一下
                else
                    CheckSocketHasICDelay.SetSecAndOn(0.5);                                                                                                     //Steven 20110908 : 上來後也要Delay一下
            }
            break;
        case 1055:                                                              //kevin 20190709 add onecycle index check 4 Site
            if(ArmSpeed_File[IndexArm].bDevicConfirm && INDEX_SUCKER_TYPE==1)   //kevin 20190709 add 20190629 回吸檢測一次 4 個 SITE   //kevin 20190530 add index check
            {
                IndexCheck4Site(true, 0, iSiteCount);
                Task=1056;
            }
            break;
        case 1056:                                                              //kevin 20190709 20190531 index check 4 Site
            if(IndexCheck4Site(false, 0, iSiteCount))
            {
                iSiteCount++;

                if(iSiteCount<TestSocket.iShtCol/2)                             //JerryYang 20250120 : modify
                {
                    Task=1055;
                }
                else
                {
                    Task=1060;
                    iSiteCount=0;
                    if(CUSTOMER_CODE==CC_Greatek)                               //Wei 20160413
                        CheckSocketHasICDelay.SetSecAndOn(5);                   //Steven 20110908 : 上來後也要Delay一下
                    else
                        CheckSocketHasICDelay.SetSecAndOn(0.5);                 //Steven 20110908 : 上來後也要Delay一下
                }
            }
            break;
        case 1060:
            if(CheckSocketHasICDelay.Off())
            {
                flag=false;
                ErrPart=" ";
                for(int i=0; i<FTestSuck.iShtRow; i++)
                {
                    for(int j=0; j<FTestSuck.iShtCol; j++)
                    {
                        flag2=false;
                        FTestSuck.CheckVaccumIsIniaialON(i, j, flag2);
                        if(flag2==true)
                        {
                            flag=true;
                            ErrPart+=IndexSuckName[i+IsNNMode()][j];            //Steven 20230712 : 修正NN mode alarm顯示

                            if(CUSTOMER_CODE==CC_SCS)                           //jou 20170516 (Steven) : SCS要求index check偵測到device時需吹氣
                                FTestSuck.Suck[i][j].Off();
                        }
                    }
                }

                if((flag || TotalErrPart !="") && LastSet.iRealDummy==REALLY)   //kevin 20220422 add Cleanout index check (一次開4個SITE) 檢查有IC不會SHOW ALARM
                {
                    Task=1070;                                                  //fail
                }
                else
                {
                    if(TestIF_File.iShuttleMode==1 &&
                       IniConfig.bShuttleMode50==false &&
                       TestIF_File.iShuttle_Sel==0 &&
                       Z2Safe==0)                                               //jou 2014-12-19 增加index arm轉換保護
                        Task=3000;
                    else if(iSelArm==1)                                         //jou 20180802 : index pick error need index vaccum check
                        return true;
                    else
                        Task=2000;                                              //pass

                    if(IniConfig.bIndexArm2SupplyLight==true ||                 //jou 2012-10-19 Index Arm 2 供應光源 for CMOS
                       TestIF_File.bForEgisTecTest==true     ||                 //Steven 20140922 : Arm2當作指紋測試
                       (IniConfig.bD58UseArm1PickPlaceArm2Test==true &&         //kevin 20150127 Arm1 下壓 arm2 測試
                        TestIF_File.bArm1PickPlaceArm2Test==true))              //Ifor 20200811 Fix: Arm1 Pick Place Arm2Test 需卡兩個條件
                    {
                        Task=3000;
                    }
                }
            }
            break;
        case 1070:
            if(MOT[MTestZ1].Gali_Two_ZAxis_Move(Prod.TestZ1_Safe, iSpeedSlow, "DoCheckSocketHasIC 1070"))
            {
                if(CUSTOMER_CODE==CC_SCS)                                       //jou 20170516 (Steven) : SCS要求index check偵測到device時需吹氣
                {
                    for(int i=0; i<FTestSuck.iShtRow; i++)
                    {
                        for(int j=0; j<FTestSuck.iShtCol; j++)
                        {
                            FTestSuck.Suck[i][j].Normal();
                        }
                    }
                }
                Task=1080;
            }
            break;
        case 1080:
            if(MOT[MTestY1].GalilTwoY_Move(Prod.TestY1_Front, Prod.TestY2_Rear, iSpeedY, "DoCheckSocketHasIC 1080"))
            {
                Task=1090;
            }
            break;
        case 1090:
            if(IndexAlarmInArmAway()==true)                                     //Steven 20130613 : Index異常時, In Arm要先讓位功能
            {
                bIsTestSitICFallDown=true;                                      //Steven 20130613
                if(IniConfig.bD40IndexICFallDownMustPressFMotorDown)            //Steven 20130604 : Socket殘料要按Z1
                {
                    ShowErrorMessage("WAR0310", K_RETRY, MTestY1, false, ErrPart);
                }
                else
                {
                    ShowMyMessage("Arm1 detect Test Socket has IC error", "Arm 1偵測到Socket有IC殘留!!", "DoCheckSocketHasIC 1090");
                }
                TotalErrPart="";                                                //kevin 20220422
                ErrPart="";
                Task=1;
            }
            break;
        case 2000:                                                              //Arm 2 start
            if(MOT[MTestZ1].Gali_Two_ZAxis_Move(Prod.TestZ1_Safe, iSpeedSlow, "DoCheckSocketHasIC 2000"))
            {
                Task=2001;
            }
            break;
        case 2001:                                                                                                      //Steven 20220721 : Alarm之前, Index要先讓開
            if(MOT[MTestY1].GalilTwoY_Move(Prod.TestY1_Front, Prod.TestY2_Rear, iSpeedY, "DoCheckSocketHasIC 2010"))
            {
                if(IniConfig.bC08_SocketSensor &&
                   TestIF_File.bEnSocketSensor &&
                   (TestIF_File.bSocketDisibleinitialcheck==false ||                                                    //JerryYang 20201101 : Index check下壓前, 要先檢查socket Sensor
                    CUSTOMER_CODE==CC_GIGAS))                                                                           //Isaac 20220126 : 全智要求Index arm在上要強制偵測sensor(應該為off)
                {
                    flag=false;
                    str="check Socket Sensor state Up off,sensor : ";
                    for(int i=0; i<TestIF_File.iSocketCount; i++)
                    {
                        if(Sen[SThreadPara.iSocketSensor[i]].Enable &&
                           Sen[SThreadPara.iSocketSensor[i]].IsOn())                                                    //kevin 20150429
                        {
                            flag=true;
                            bIsSocketSensor=true;
                            str+=IntToStr(i+1);
                        }
                    }

                    if(flag)
                    {
                        ShowErrorMessage("WAR0322", K_RETRY, MTestZ1, false, str);                                      //kevin 20130504 socket sensor
                        break;                                                                                          //kevin 20150429確認 socket sensor是否正常
                    }
                }
                Task=2010;
            }
            break;
        case 2010:
            if(MOT[MTestY1].GalilTwoY_Move(Prod.TestY1_Front, Prod.TestY2_Middle, iSpeedY, "DoCheckSocketHasIC 2010"))
            {
                Task=2020;
                if(ArmSpeed_File[IndexArm].bDevicConfirm &&
                   INDEX_SUCKER_TYPE==1)                                        //kevin 20190912 add 一次開4 site check
                    Task=2050;
            }
            break;
        case 2020:
            if(MOT[MTestZ2].Gali_MotMove(Prod.TestZ2_Test-Prod.TestZ2_Drop_Offset+iIndexCheckOffSet-iIndexArmCheck_SG_Arm2, iSpeedSlow))                        //KaiChen 20200826 ：矽格-湖口，要求IndexCheck使用Contact高度不使用Offset
            {
                IndexStatus=Z1Up_Z2Down;
                Task=2030;
            }
            break;
        case 2030:
            for(int i=0; i<BTestSuck.iShtRow; i++)
            {
                for(int j=0; j<BTestSuck.iShtCol; j++)
                {
                    BTestSuck.Suck[i][j].On();
                    if(INDEX_SUCKER_TYPE==1)
                    {
                        fiosetview->bIndexSuck[1][i][j]=true;
                        bIndexCheckNoStopVaccum=true;
                    }
                }
            }

            CheckSocketHasICDelay.SetSecAndOn(1);
            Task=2040;
            break;
        case 2040:                                                              //jou 2012-05-09 Index Check時,如果中途按暫停,也要繼續把suck()做完,避免負壓掉ic
            if(INDEX_SUCKER_TYPE==1)
            {
                fiosetview->ProcessIndexSuckDestroy2();
            }

            if(CheckSocketHasICDelay.Off())
            {
                Task=2050;
            }
            break;
        case 2050:
            iIndexUpPos=GetSocketCheckPos(Prod.TestZ2_Test);                                                                                                    //Steven 20140620 : 整合為Function

            if(MOT[MTestZ2].Gali_MotMove(Prod.TestZ2_Test-Prod.TestZ2_Drop_Offset+iIndexUpPos+iIndexCheckOffSet-iIndexArmCheck_SG_Arm2, iSpeedFast))            //KaiChen 20200826 ：矽格-湖口，要求IndexCheck使用Contact高度不使用Offset
            {
                if(CUSTOMER_CODE==CC_Greatek)                                                                                                                   //Wei 20160413
                    CheckSocketHasICDelay.SetSecAndOn(5);                                                                                                       //Steven 20110908 : 上來後也要Delay一下
                else
                    CheckSocketHasICDelay.SetSecAndOn(0.5);                                                                                                     //Steven 20110908 : 上來後也要Delay一下
                IndexStatus=Z1Up_Z2Down;

                if(ArmSpeed_File[IndexArm].bDevicConfirm &&                                                                                                     //kevin 20190709 add 20190629 回吸檢測一次 4 個 SITE
                   INDEX_SUCKER_TYPE==1)                                                                                                                        //kevin 20190530 add index check
                {
                    iSiteCount=0;
                    Task=2055;
                }
                else
                {
                    Task=2060;
                }
            }
            break;
        case 2055:                                                              //kevin 20190709 add onecycle index check 4 Site
            if(ArmSpeed_File[IndexArm].bDevicConfirm && INDEX_SUCKER_TYPE==1)   //kevin 20190709 add 20190629 回吸檢測一次 4 個 SITE   //kevin 20190530 add index check
            {
                IndexCheck4Site(true,1,iSiteCount);
                Task=2056;
            }
            break;
        case 2056:                                                              //kevin 20190709 20190531 index check 4 Site
            if(IndexCheck4Site(false, 1, iSiteCount))
            {
                iSiteCount++;

                if(iSiteCount<TestSocket.iShtCol/2)                             //JerryYang 20250120 : modify
                {
                    Task=2055;
                }
                else
                {
                    Task=2060;
                    iSiteCount=0;
                    if(CUSTOMER_CODE==CC_Greatek)                               //Wei 20160413
                        CheckSocketHasICDelay.SetSecAndOn(5);                   //Steven 20110908 : 上來後也要Delay一下
                    else
                        CheckSocketHasICDelay.SetSecAndOn(0.5);                 //Steven 20110908 : 上來後也要Delay一下
                }
            }
            break;
        case 2060:
            if(CheckSocketHasICDelay.Off())
            {
                flag=false;
                ErrPart=" ";
                for(int i=0; i<BTestSuck.iShtRow; i++)
                {
                    for(int j=0; j<BTestSuck.iShtCol; j++)
                    {
                        flag2=false;
                        BTestSuck.CheckVaccumIsIniaialON(i, j, flag2);
                        if(flag2==true)
                        {
                            flag=true;
                            ErrPart+=IndexSuckName[i][j];

                            if(CUSTOMER_CODE==CC_SCS)                           //jou 20170516 (Steven) : SCS要求index check偵測到device時需吹氣
                                BTestSuck.Suck[i][j].Off();
                        }
                    }
                }

                if((flag || TotalErrPart!="") && LastSet.iRealDummy==REALLY)    //kevin 20220422 add Cleanout index check (一次開4個SITE) 檢查有IC不會SHOW ALARM
                    Task=2070;                                                  //fail
                else if(iSelArm==2)
                    return true;
                else
                    Task=3000;                                                  //pass
            }
            break;
        case 2070:
            if(MOT[MTestZ1].Gali_Two_ZAxis_Move(Prod.TestZ1_Safe, iSpeedSlow, "DoCheckSocketHasIC 2070"))
            {
                if(CUSTOMER_CODE==CC_SCS)                                       //jou 20170516 (Steven) : SCS要求index check偵測到device時需吹氣
                {
                    for(int i=0; i<BTestSuck.iShtRow; i++)
                    {
                        for(int j=0; j<BTestSuck.iShtCol; j++)
                        {
                            BTestSuck.Suck[i][j].Normal();
                        }
                    }
                }
                Task=2080;
            }
            break;
        case 2080:
            if(MOT[MTestY1].GalilTwoY_Move(Prod.TestY1_Front, Prod.TestY2_Rear, iSpeedY, "DoCheckSocketHasIC 2080"))
            {
                Task=2090;
            }
            break;
        case 2090:
            if(IndexAlarmInArmAway()==true)                                     //Steven 20130613 : Index異常時, In Arm要先讓位功能
            {
                bIsTestSitICFallDown=true;                                      //Steven 20130613
                if(IniConfig.bD40IndexICFallDownMustPressFMotorDown)            //Steven 20130604 : Socket殘料要按Z1
                {
                    ShowErrorMessage("WAR0310", K_RETRY, MTestY2, false, ErrPart);
                }
                else
                {
                    ShowMyMessage("Arm2 detect Test Socket has IC error", "Arm 2偵測到Socket有IC殘留!!", "DoCheckSocketHasIC 2090");
                }
                Task=1;
                ErrPart="";
                TotalErrPart="";                                                //kevin 20220422
            }
            break;
        case 3000:                                                              //finish
            if(MOT[MTestZ1].Gali_Two_ZAxis_Move(Prod.TestZ1_Safe, iSpeedSlow, "DoCheckSocketHasIC 3000"))
            {
                bFTestSuckDrop=false;
                bBTestSuckDrop=false;
                return true;
            }
            break;
    }
    return false;
}
#endif // GOLDEN VERBATIM -- golden atester.cpp:4305-4781  (GATE G-PTk3-DoCheckSocketHasIC, end)
bool DoCheckSocketHasIC(int iSelArm)                                            //jou 20180802 : index pick error need index vaccum check
{
#if 0 // TODO(W7) -- golden :4305-4783 (MOT[] vacuum-check SM)
#endif
    (void)iSelArm;
    (void)iCheckSocketHasIC;
    return false;                                                              // golden default: socket check not finished
}
//------------------------------------------------------------------------------
void InitTestYTask()
{
    iTestYTask=1;
}
//------------------------------------------------------------------------------
//  DoTestY (golden :4789-5347) -- the test-cycle dispatcher SM.  ACTIVE: switch
//  + cursor transitions reproduced VERBATIM for the oracle path (case 1 -> 50 ->
//  100 -> 110 fall-through).  The heavy per-case bodies that only touch substrate
//  (FTestSuck/BTestSuck/FLCarryKit/BLCarryKit + MOT[] move calls) are kept
//  faithful; the socket-purge debug-log call (fMain->DebugOneCycleHotPlate) is a
//  FormsFacade no-op; the TSMC/Dell short-test sub-trees off the oracle path are
//  gated; the bUseTwoArm32Site path (case 260/300/310) routes to atester_shims.
//------------------------------------------------------------------------------
void DoTestY()
{
    static bool fFront=false;

    int &Task=iTestYTask;
    int Type1=0, Type2=0;
    int iSpeedZ=0;
    bool flag;

    switch(Task)
    {
        case 1:
            dTestSec=TestIF_File.iInitialMaxTime;                               //2013-11-27    Dell Add Index soak time
            bNeedIndexSoakTime=false;                                           //2013-11-27    Dell Add Index soak time
            if(IsInArmCleanOutFinish() ||
               IsInArmOneCycleFinish() ||                                       //Steven 20131029 : 解決Index Position Error
               bCanNotDisableOneCycle)
            {
                if(CanYieldAlarmRemainInSHT() &&                                //JerryYang 20220923 : yield alarm時觸發half one cycle(shuttle保留IC不測試跳ONE CYCLE FINISH)
                   (bCanNotDisableOneCycle ||
                   (TestHeadHasIC()==false)))
                {
                    bCanNotDisableOneCycle=true;

                    if(IndexStatus!=Z1_Z2_Normal)
                    {
                        if(bCheckIndex==false)                                  //jou 2011-12-08 做piggy-back時，不用做兩次socket check
                        {
                            InitCheckSocketHasIC();
                            Task=20;
                        }
                        else
                        {
                            Task=25;
                        }
//                        break;                                                //Steven 20180813 : add index arm speed
                    }
                    else
                    {
                        Task=25;
//                        break;
                    }
                }

                if(bCanNotDisableOneCycle ||
                   (TestHeadHasIC()==false &&
                    InputShuttleHasIC()==false &&
                    InArmSuck.HasRealIC()==false))
                {
                    bCanNotDisableOneCycle=true;

                    if(IndexStatus!=Z1_Z2_Normal &&
                       IndexStatus!=IndexIsBack)                                //Steven 20221027 : Fixed D51 hang up
                    {
                        if(bCheckIndex==false)                                  //jou 2011-12-08 做piggy-back時，不用做兩次socket check
                        {
                            InitCheckSocketHasIC();
                            Task=20;
                        }
                        else
                        {
                            Task=25;
                        }
//                        break;                                                //Steven 20180813 : add index arm speed
                    }
                    else
                    {
                        if(LastSet.iTemperature==Tempture_Hot      &&           //Steven 20220428 : Fixed for one cycle hang up
                           TestHeadHasIC()==false  &&
                           InputShuttleHasIC()==false &&
                           InArmSuck.HasType(HAS_HOT_IC)==false)
                        {
                            if(IndexStatus==Z1_Z2_Normal ||
                               IndexStatus==IndexIsBack)                        //Steven 20221027 : Fixed D51 hang up
                            {
                                if(FRCarryKit.UseSiteHasIC())
                                {
                                    MOT[MInShuttle1].fCanMoveM=true;
                                    iIndexTakeDeviceChk1=0;                     //kevin 20190103 回吸檢測狀態
                                }

                                if(BRCarryKit.UseSiteHasIC())
                                {
                                    MOT[MInShuttle2].fCanMoveM=true;
                                    iIndexTakeDeviceChk2=0;                     //kevin 20190103 回吸檢測狀態
                                }
                            }
                            Task=1;
                            break;
                        }
                        Task=25;
//                        break;
                    }
                }
            }

            if(bCanNotDisableOneCycle==false)
            {
                if(bUseTwoArm32Site==true)
                    Task=260;                                                   //InitTestYTwoArm32Site()
                else
                    Task=50;
            }

//            if(Task!=20)                                                      //Steven 20180813 : add index arm speed
                break;
        case 20:
            if(CosFunction.bIndexCheckCanTurnOff &&                             //Isaac 20211019 : 可選擇做index check的時機
               ((IniConfig.iD71IndexCheckOnOffMode==0 && bLotStartEndNeedIndexCheck==false) ||
                (IniConfig.iD71IndexCheckOnOffMode==1 && bIndexJamNeedIndexcheck==false) ||
                 IniConfig.iD71IndexCheckOnOffMode==2))
            {
                Task=25;                                                        //不檢查
            }
            else if(IniConfig.bD55DisableIndexCheck &&                          //ChungHung 20120606 DisableIndexCheck
                    REAL_TIME_CCD==true && COM2->bCCDDummyRum==false)           //Steven 20150723 : Fixed for SCK
            {
                Task=25;
            }
            else if(CosFunction.bBeforeAutoCleanOnlyUseRTC==true &&             //JerryYang 20161216 (Steven) auto clean的前後只靠RTC來檢查socket,不做index下壓至socket吸真空
                    bIsAutoOneCycle==true &&
                    REAL_TIME_CCD==true &&
                    COM2->bCCDDummyRum==false)
            {
                Task=25;
            }
            else if(CosFunction.bAfterAutoCleanNoIndexCheck &&                  //Steven 20191212 : 劉仁洲說Auto Clean只要作一次Index Check
                    IniConfig.iD69IndexCheckModeForAutoClean==2 &&
                    bIsAutoOneCycle==true)
            {
                Task=25;
            }
            else
            {
                if(DoCheckSocketHasIC()==true)
                {
                    Task=25;
                }
            }

//            if(Task!=25)                                                      //Steven 20180813 : add index arm speed
                break;
        case 25:
            iSpeedZ=(INDEX_PRESS_TYPE==e240KG || INDEX_PRESS_TYPE==e400KG || INDEX_PRESS_TYPE==e260KG || INDEX_PRESS_TYPE==e360KG)?200000:50000;                //Steven 20110503 : 240KG加速 //Steven 20131007 : Index 1.5KW, 400KG
            if(MOT[MTestZ2].Gali_Two_ZAxis_Move(Prod.TestZ1_Safe, iSpeedZ, "DoTestY 25"))
            {
                Task=30;
                if(CosFunction.bIndexCheckCanTurnOff)                                                                                                           //Isaac 20211019 : 可選擇做index check的時機，旗標重置
                {
                    bLotStartEndNeedIndexCheck=false;
                    bIndexJamNeedIndexcheck=false;
                }
            }
//            if(Task!=30)                                                      //Steven 20180813 : add index arm speed
                break;
        case 30:
            if(IniConfig.bD51UseOnecycleCleanOutFinishTestArmAtRear)
            {
                Task=45;
            }
            else
            {
                if(MOT[MTestY1].GalilTwoY_Move(Prod.TestY1_Front_EndWaitPos, Prod.TestY2_Rear, 50000, "DoTestY 30"))    //981118 jou Y1 +2000 easy change kit
                    Task=40;
            }
//            if(Task!=40)                                                      //Steven 20180813 : add index arm speed
                break;
        case 40:
            IndexStatus=Z1_Z2_Normal;
            bCanNotDisableOneCycle=false;
            Task=1;
            break;
        case 45:
            if(TestArmBackPos())
            {
                IndexStatus=IndexIsBack;
                bCanNotDisableOneCycle=false;
                Task=1;
            }
            break;
        case 50:
            flag=false;
            if(FTestSuck.UseSiteHasIC())
            {
                if(FTestSuck.AlreadyTest())
                    flag=true;
            }

            if(FTestSuck.UseSiteHasIC()==false)
                Type1=0;
            else if(flag==true)
                Type1=2;
            else
                Type1=1;

            flag=false;

            if(BTestSuck.UseSiteHasIC())
            {
                if(BTestSuck.AlreadyTest())
                {
                    flag=true;
                }
            }

            if(BTestSuck.UseSiteHasIC()==false)
                Type2=0;
            else if(flag==true)
                Type2=2;
            else
                Type2=1;

            if(FTestSuck.PartAlreadyTest())
            {
                fFront=false;
            }
            else if(BTestSuck.PartAlreadyTest())
            {
                fFront=true;
            }
            else
            {
                fFront=true;
                if(     Type1==2) fFront=true;
                else if(Type2==2) fFront=false;
                else if(Type1==0) fFront=true;
                else if(Type2==0) fFront=false;
                else if(Type1==1) fFront=false;
                else if(Type2==1) fFront=true;
                if(Type1==0 && Type2==0)
                {
                    if(FLCarryKit.UseSiteHasIC())
                    {
                        fFront=true;
                    }
                    else
                    {
                        if(BLCarryKit.UseSiteHasIC())                           //need check chang else
                            fFront=false;
                    }
                }
            }

            if(fFront)
                Task=100;
            else
                Task=200;
            break;
        case 60:
            if(IniConfig.bD47EnableSocketPurgeFunction &&
               IniConfig.iD47SocketPurgeCount!=0 &&
               iOneCycle==0 &&
               iCleanOut==0)
            {
                if(LastSet.iD47SocketTestedCount>=IniConfig.iD47SocketPurgeCount)
                {
                    LastSet.iD47SocketTestedCount=0;
                    iClearSocketFunction=1;
                    iOneCycle=1;
                    fMain->DebugOneCycleHotPlate("DoTestY_60");                                                         //Sam 20210915 : 增加 OneCycle Hotpalte Debug Log
                }
            }

            if(IsInArmOneCycleFinish() ||
               IsInArmCleanOutFinish())
            {
                if(IniConfig.bIndexArm2SupplyLight==true ||                                                             //jou 2012-10-19 Index Arm 2 供應光源 for CMOS
                   TestIF_File.bForEgisTecTest==true     ||                                                             //Steven 20140922 : Arm2當作指紋測試
                   (IniConfig.bD58UseArm1PickPlaceArm2Test==true &&                                                     //kevin 20150127 Arm1 下壓 arm2 測試
                    TestIF_File.bArm1PickPlaceArm2Test==true))                                                          //Ifor 20200811 Fix: Arm1 Pick Place Arm2Test 需卡兩個條件
                {
                    if(FTestSuck.UseSiteHasIC()==false &&
                       BTestSuck.UseSiteHasIC()==false &&
                       FLCarryKit.UseSiteHasIC()==false &&
                       InArmSuck.HasRealIC()==false)
                    {
                        Task=1;
                        break;
                    }
                }

                if(CanYieldAlarmRemainInSHT()==true)                                                                    //JerryYang 20220923 : yield alarm時觸發half one cycle(shuttle保留IC不測試跳ONE CYCLE FINISH)
                {
                    if((FTestSuck.UseSiteHasIC()==false || FTestSuck.HasRealIC()==false) &&
                       (BTestSuck.UseSiteHasIC()==false || BTestSuck.HasRealIC()==false))
                    {
                        FTestSuck.ClearAll();
                        BTestSuck.ClearAll();
                        Task=1;
                        break;
                    }
                }

                if((FTestSuck.UseSiteHasIC()==false  || FTestSuck.HasRealIC()==false) &&
                   (BTestSuck.UseSiteHasIC()==false  || BTestSuck.HasRealIC()==false) &&
                   (FLCarryKit.UseSiteHasIC()==false || FLCarryKit.HasRealIC()==false) &&                               //Steven 20191213 : 修正One Cycle的動作
                   (BLCarryKit.UseSiteHasIC()==false || BLCarryKit.HasRealIC()==false) &&
                   InArmSuck.HasRealIC()==false)
                {
                    FTestSuck.ClearAll();
                    BTestSuck.ClearAll();
                    FLCarryKit.ClearAll();
                    BLCarryKit.ClearAll();

                    Task=1;
                    break;
                }
            }

            if(iCleanOut!=0 && IndexHasIC()==false && InputShuttleHasIC()==false)
                break;

            // golden :5099-5176 -- TSMC short-test/initial-delay + Dell index-soak
            // branch tree (deref Prod/Temperature/CosFunction flags + fMain timers).
            // Off the oracle path; GATED.  Offline takes the plain Task=70.
#if 0 // TODO(W7) -- golden :5099-5176 (TSMC initial-delay + Dell hIndexSoakTime/tSoakTimer tree)
#endif
            Task=70;
            break;
        case 65:                                                                //2013-11-27    Dell Add Index soak time "當測試秒數太短(ex open/shot)" or "fHeaterOK==false"
            if(hIndexSoakTime.Off())
                Task=70;
            break;
        case 70:
            if(CosFunction.bOEEFunction)                                        //Steven 20180417 (Jou) : OEE功能
            {
                fObserver->bTestIndexZ=true;
            }
            fFront=!fFront;
            if(fFront)
            {
                Task=100;
            }
            else
            {
                Task=200;
            }

            if(Task!=100)                                                       //Steven 20180813 : add index arm speed
                break;
        case 100:
            InitTestYFrontTask();
            //----- by dell ccd realtime-------------
            bRealCCDSendArm=true;
            //---------------------------------------
            Task=110;
        case 110:
            if(CheckHeaterOK()==false)                                          //Steven 20250116 : 確認HeaterOK
            {
                bHangTimePause=true;
                bNeedIndexSoakTime=true;                                        //2013-11-27    Dell Add Index soak time "當測試秒數太短(ex open/shot)" or "fHeaterOK==false"
            }

            if(DoTestYFront())
            {
                if(IsInArmOneCycleFinish() ||
                   IsInArmCleanOutFinish())
                {
                    if(LastSet.iTemperature==Tempture_Hot &&
                       FTestSuck.UseSiteHasIC()==false   &&
                       BTestSuck.UseSiteHasIC()==false   &&
                       FLCarryKit.UseSiteHasIC()==false  &&
                       BLCarryKit.UseSiteHasIC()==false  &&
                       FRCarryKit.UseSiteHasIC()==false  &&                     //Sam 20230809 : OneCycle 最後一次 IndexArm 動作，需要等待 OutShuttle 動作做完才能做 IndexCheck，避免黏料壓壞 IC
                       InArmSuck.HasType(HAS_HOT_IC)==false)
                    {
                        if(IndexStatus==Z1_Z2_Normal)
                        {
                            if(FRCarryKit.UseSiteHasIC())
                            {
                                MOT[MInShuttle1].fCanMoveM=true;
                                iIndexTakeDeviceChk1=0;                         //kevin 20190103 回吸檢測狀態
                            }

                            if(BRCarryKit.UseSiteHasIC())
                            {
                                MOT[MInShuttle2].fCanMoveM=true;
                                iIndexTakeDeviceChk2=0;                         //kevin 20190103 回吸檢測狀態
                            }
                            Task=1;
                            break;
                        }
                    }
                }
                Task=60;
            }
            break;
        case 200:
            InitTestYRearTask();
            //----- by dell ccd realtime-------------
            bRealCCDSendArm=true;
            //---------------------------------------
            Task=210;
        case 210:
            if(CheckHeaterOK()==false)                                          //Steven 20250116 : 確認HeaterOK
            {
                bHangTimePause=true;
                bNeedIndexSoakTime=true;                                        //2013-11-27    Dell Add Index soak time "當測試秒數太短(ex open/shot)" or "fHeaterOK==false"
            }

            if(DoTestYRear())
            {
                if(IsInArmOneCycleFinish() ||
                   IsInArmCleanOutFinish())                                     //Steven 20220426 : 避免One Cycle發生index position error
                {
                    if(LastSet.iTemperature==Tempture_Hot &&
                       FTestSuck.UseSiteHasIC()==false   &&
                       BTestSuck.UseSiteHasIC()==false   &&
                       FLCarryKit.UseSiteHasIC()==false  &&
                       BLCarryKit.UseSiteHasIC()==false  &&
                       BRCarryKit.UseSiteHasIC()==false  &&                     //Sam 20230809 : OneCycle 最後一次 IndexArm 動作，需要等待 OutShuttle 動作做完才能做 IndexCheck，避免黏料壓壞 IC
                       InArmSuck.HasType(HAS_HOT_IC)==false)
                    {
                        if(IndexStatus==Z1_Z2_Normal)
                        {
                            if(FRCarryKit.UseSiteHasIC())
                            {
                                MOT[MInShuttle1].fCanMoveM=true;
                                iIndexTakeDeviceChk1=0;                         //kevin 20190103 回吸檢測狀態
                            }

                            if(BRCarryKit.UseSiteHasIC())
                            {
                                MOT[MInShuttle2].fCanMoveM=true;
                                iIndexTakeDeviceChk2=0;                         //kevin 20190103 回吸檢測狀態
                            }
                        }
                        Task=1;
                        break;
                    }
                }
                Task=60;
            }
            break;
        case 260:
            if(IniConfig.bD47EnableSocketPurgeFunction &&
               IniConfig.iD47SocketPurgeCount!=0 &&
               iOneCycle==0 &&
               iCleanOut==0)
            {
                if(LastSet.iD47SocketTestedCount>=IniConfig.iD47SocketPurgeCount)
                {
                    LastSet.iD47SocketTestedCount=0;
                    iClearSocketFunction=1;
                    iOneCycle=1;
                    fMain->DebugOneCycleHotPlate("DoTestY_260");                //Sam 20210915 : 增加 OneCycle Hotpalte Debug Log
                }
            }

            if(IsInArmOneCycleFinish() ||
               IsInArmCleanOutFinish())                                         //Steven 20220426 : 避免One Cycle發生index position error
            {
                if(FTestSuck.UseSiteHasIC()==false  &&
                   BTestSuck.UseSiteHasIC()==false  &&
                   FLCarryKit.UseSiteHasIC()==false &&
                   BLCarryKit.UseSiteHasIC()==false)
                {
                    if(LastSet.iTemperature==Tempture_Hot)
                    {
                        if(InArmSuck.HasType(HAS_HOT_IC)==false)
                        {
                            Task=1;
                            break;
                        }
                    }
                    else                                                        //Steven 20220514 : 修正Clean Out死雞
                    {
                        if(IsNNMode()!=NN_2Row)
                        {
                            FTestSuck.ClearAll();
                            BTestSuck.ClearAll();
                        }

                        if(InArmSuck.HasIC()==false)
                        {
                            Task=1;
                            break;
                        }
                    }
                }
            }

            if(iCleanOut!=0 && IndexHasIC()==false && InputShuttleHasIC()==false)
                break;

            Task=300;
            break;
        case 300:
            if(CosFunction.bOEEFunction)                                        //Steven 20180417 (Jou) : OEE功能
            {
                fObserver->bTestIndexZ=true;
            }
            InitTestYTwoArm32SiteTask();
            //----- by dell ccd realtime-------------
            bRealCCDSendArm=true;
            //---------------------------------------
            Task=310;
//            break;                                                            //Steven 20180813 : add index arm speed
        case 310:
            if(CheckHeaterOK()==false)                                          //Steven 20250116 : 確認HeaterOK
            {
                bHangTimePause=true;
                bNeedIndexSoakTime=true;                                        //2013-11-27    Dell Add Index soak time "當測試秒數太短(ex open/shot)" or "fHeaterOK==false"
            }

            if(DoTestY_TwoArm32Site())
                Task=260;
            break;
    }
}
//------------------------------------------------------------------------------
int iTestHeadMotorTask=1;
int iProcessIndexSuckDestroyCnt=0;                                              //RogerYang 20250930 : RogerYang 瑞薩FT-CT 真空產生器自檢功能，一段時間後才重置，避免掉壓
void InitialTestHeadMotorTask()
{
    iTestHeadMotorTask=1;
    if(iProcessIndexSuckDestroyCnt>15 || TestIF_File.bRENESAS_EnableFTCT==false)                                        //RogerYang 20250930 : RogerYang 瑞薩FT-CT 真空產生器自檢功能，一段時間後才重置，避免掉壓
        iProcessIndexSuckDestroyCnt=0;

    if(MOT[MTestY1].Motor!=NULL)                                                //Sam 20220718 : 防止 OneCycle and CleanOut IndexArm 偷跑
        MOT[MTestY1].Gali_Command("VS0;SP0,0,0,0;", __FUNC__);
}
//------------------------------------------------------------------------------
bool IsIndexRunCycle()
{
    if(iTestHeadMotorTask==600)
    {
        if(bCanNotDisableOneCycle==true)
            return false;
        return true;
    }
    else
    {
        return false;
    }
}
// ===========================================================================
//  W7-T1 SEAM BLOCK  -- TU-local forward declarations / offline stand-ins for
//  the down-press / test / torque / EP / load-cell case tree (golden atester.cpp:
//  6229-8650, un-gated below) references that are NOT yet present in the
//  translated tree.  Mirrors the W7C1_SEAM / W7C2_SEAM pattern in csystem.cpp:
//  each stand-in is offline-inert (no-op / golden-safe default) so the faithful
//  golden body lands + compiles NOW; the serial Integrate must replace every
//  W7T1_* stand-in with the REAL translated symbol / real facade member.
//
//  NO-REGRESSION KEY: SOFT_SIMULTE is OFF in this build, so the #else torque
//  branch (COM2->iWriteAndCheckMotorTorque) compiles -- seamed to return 1 (OK)   [AI(W906-R28TORQ) 20260925: no longer -- it forwards to the real TCOM2Shim (rs232.cpp); see W7T1_TCOM2Ext below]
//  so the torque SM advances exactly as the SOFT_SIMULTE branch would.  MOT[] Z/Y
//  moves + Suck/Socket bin writes are REAL (S0-converged / grid); NOT seamed.
// ===========================================================================
#ifndef W7T1_SEAM
#define W7T1_SEAM

// --- wave-timing (common.cpp now provides a real MyTickCount, un-gated by
//  AI(W906-CommonCompletion) 20260721 -- this TU-local macro redirect below
//  still intentionally shadows it with the offline stand-in, unchanged by that
//  wave; revisiting whether this seam should switch to the real tick source is
//  a separate translation decision, out of scope here) ------------------------
//  case 20200/20210 idle-drain timer.  Offline: monotonic 0 (no real tick source
//  yet); startTick/endTick/nowTick are function locals below.
static DWORD W7T1_MyTickCount(){ return (DWORD)0; }            // golden common.h:259
#define MyTickCount            W7T1_MyTickCount

// --- RTC-CCD entry case ids: moved to golden file-scope home (atester.cpp:5551-5555,
//  const int iCASE_REAL_CCD2..6 near DoTestHeadMotorDelay above).  The stale
//  atester_shims.cpp iCASE_REAL_CCD2=9 (+ its atester_shims.h extern) is REMOVED
//  by Integrate so the switch labels carry the golden 40200/40300/40400/40500/40510
//  and no longer collide with the active `case 9:`.  Nothing needed here now.

// --- index-check / torque / socket-heating timers (golden atester.cpp file-scope
//  TQPF_Timer objects).  REAL TQPF_Timer instances (NOT stand-ins) -- same class
//  the active DoTestHeadMotorDelay uses -- so their .Off()/.SetSecAndOn()/
//  .LatchCycleTime() behave exactly as golden.  Absent as instances in the tree
//  today; add TU-local.  Integrate: move to the real file-scope home.
static TQPF_Timer W7T1_ReadTorqueDelay;                        // golden atester.cpp (kevin 20210824)
#define ReadTorqueDelay        W7T1_ReadTorqueDelay
static TQPF_Timer W7T1_DoUseSocketHeating;                     // golden atester.cpp (Ztex 2024.09.07)
#define DoUseSocketHeating     W7T1_DoUseSocketHeating
static TQPF_Timer W7T1_CheckSocketHasICDelay;                  // golden atester.cpp (JerryYang 20250807)
#define CheckSocketHasICDelay  W7T1_CheckSocketHasICDelay

// --- absent FREE functions (golden atester.cpp inline / main.h homes) ----------
// AI(W906-R28TORQ) 20260925: the two Index Z torque stand-ins below are RETIRED -- the calls now bind to the
//   REAL bodies (使用者 20260925 裁決第 6 條「出貨組態的 Index Z 扭力上限要對齊原 BCB6 版本做法」):
//     ShowMainScreenPresure            golden cinitial.cpp:13816 -> port cinitial.cpp (ShowMainScreenPresure, N1-G6 un-gated)
//                                      —— 它不只是顯示：它把 lbArm0Torque->Caption 從 "1:Reading" 改成讀值，
//                                      12110 靠這個比較決定要不要回 120 重寫；也寫 asArmForce1/2（GPIB 回報的接觸力）。
//                                      以前的 no-op 讓出貨組態的 12110 永遠回 120（以前還輪不到，因為 edTorue0 也是替身）。
//     InitWriteAndCheckMotorTorqueTask golden rs232.cpp:1838 -> port rs232.cpp（宣告 atester_shims.h）
#if 0
static void W7T1_ShowMainScreenPresure(int /*iArm*/){}         // golden main.h -- offline: main-screen pressure UI no-op
#define ShowMainScreenPresure  W7T1_ShowMainScreenPresure
static void W7T1_InitWriteAndCheckMotorTorqueTask(){}          // golden atester.cpp -- torque-write cursor reset (offline no-op)
#define InitWriteAndCheckMotorTorqueTask  W7T1_InitWriteAndCheckMotorTorqueTask
#endif
void ShowMainScreenPresure(int index);                         // golden cinitial.h:39 -> port cinitial.h:250（body cinitial.cpp）
static void W7T1_RecordIndexPosition(int /*a*/, int /*b*/){}   // golden atester.cpp (Isaac 20200922) -- encoder-vs-command log (offline no-op)
#define RecordIndexPosition    W7T1_RecordIndexPosition
static void W7T1_EncoderTeachingMaxMinCount(int /*a*/){}       // golden atester.cpp (Isaac 20201012) -- encoder/teach delta log (offline no-op)
#define EncoderTeachingMaxMinCount  W7T1_EncoderTeachingMaxMinCount
static void W7T1_TrigerIndexAxisHome(){}                       // golden atester.cpp (Isaac 20201012) -- index Y over-range auto-home (INDEX_PROTECT_TMOVE gated)
#define TrigerIndexAxisHome    W7T1_TrigerIndexAxisHome
//AI(W906-YESNO) 20260925: `W7T1_ShowMyMessageBox_YES_NO`（`return 0;`）＋ `#define` 拿掉 —— 改用
//  canary_support.h 的真 ShowMyMessageBox_YES_NO（golden mymessbox.h:55），由 wb_serve 送網頁問操作員
//  （使用者 20260925 裁決第 10 條）。原註解寫「NO(0)」是錯的：golden 1=Yes／2=No，0 兩者都不是。

// --- fiosetview index-suck self-check methods (golden iosetview.h) -------------
//  The offline TfiosetviewShim (atester_shims.h) exposes bIndexSuck[][][] but NOT
//  the ProcessIndexSuckDestroy1/2 pump methods.  Offline (no DAQ): report the
//  suck-check "complete" (true) so the index-check SM advances.  Integrate: add
//  the real methods to TfiosetviewShim (or the real iosetview form) + drop the
//  W7T1_FIOSET_PISD* call-site macros.
static bool W7T1_ProcessIndexSuckDestroy1(){ return fiosetview->ProcessIndexSuckDestroy1(); }   //AI(W906-IDXSUCK) 20260927: 以前一律回 true，改成轉呼叫 golden 照翻的本體（atester_shims.cpp 檔尾）    // golden iosetview.h -- offline: suck self-check done (true)
static bool W7T1_ProcessIndexSuckDestroy2(){ return fiosetview->ProcessIndexSuckDestroy2(); }   //AI(W906-IDXSUCK) 20260927: 以前一律回 true，改成轉呼叫 golden 照翻的本體（atester_shims.cpp 檔尾）    // golden iosetview.h -- offline: suck self-check done (true)
#define W7T1_FIOSET_PISD1()    W7T1_ProcessIndexSuckDestroy1()  // golden fiosetview->ProcessIndexSuckDestroy1()
#define W7T1_FIOSET_PISD2()    W7T1_ProcessIndexSuckDestroy2()  // golden fiosetview->ProcessIndexSuckDestroy2()

// --- TMyKitSuck::CheckVaccumIsIniaialON (golden MyKitSuck.h) -------------------
//  Absent from the W6 TMyKitSuck mirror (aHotPlateSubstrate.h).  Offline (no
//  vacuum DAQ): the "is-initial-vacuum-on" out-param stays false (no residual IC
//  detected) so the socket-residual alarm branch is NOT taken.  Integrate: add
//  the real member + drop the W7T1_CHECKVACINIT call-site macro.  (Called on
//  FTestSuck/BTestSuck -- REAL objects.)
#define W7T1_CHECKVACINIT(kit,i,j,out)  do { (out)=false; } while(0)   // golden (kit).CheckVaccumIsIniaialON(i,j,flag)

// --- fContact contact-mode index-check form (golden cContact.h) ----------------
//  fContact (TfContactShim*, atester_shims.h) exposes Do_ROILearning() [no-arg]
//  but the golden RTC path calls Do_ROILearning(true) + InitROILearningTask().
//  Offline: ROI learning "done" (true); init no-op.  Integrate: add the bool
//  overload + InitROILearningTask to the real fContact / cContact form + drop the
//  W7T1_FCONTACT_* call-site macros.
#define W7T1_FCONTACT_DOROI(b)      (fContact->Do_ROILearning())   // golden fContact->Do_ROILearning(true); offline done(true)
#define W7T1_FCONTACT_INITROI()     do { } while(0)                // golden fContact->InitROILearningTask(); offline no-op

// --- fMain torque / open-bin UI members (golden main.h) ------------------------
//  The FormsFacade TfMain (FormsFacade.h) lacks the torque-read UI widgets
//  (lbArm0Torque / lbArm1Torque / chkReadTorque1 / chkReadTorque2 / edTorue0 /
//  edTorue1 / labUser) + SetOpenBin().  With the COM2 torque seam returning OK,
//  the torque-read path is offline-inert; but the code that reads/writes these
//  widgets must compile.  A TU-local stand-in mirrors the touched surface
//  (Caption / Checked / Text.c_str()).  Integrate: add the real widgets to TfMain
//  (TPanel/TLabel/TCheckBox/TEdit -- see the per-member golden citations below)
//  + SetOpenBin() + drop the W7T1_FMAIN_* call-site macros.
//
//  AI(W906-W7-F2) 20260729: the three TU-local value-holder types this block used to
//  declare -- W7T1_TLabelSeam / W7T1_TCheckSeam / W7T1_TEditSeam -- are RETIRED.
//  vclcompat/Controls.h is now the single home for stock-widget stand-ins (plan D4),
//  so the members below name those unified types directly.  Each member's golden
//  widget class was re-read from golden main.h for this change rather than inherited
//  from the old comment: lbArm0Torque main.h:798 and lbArm1Torque main.h:797 are
//  TPanel (NOT TLabel -- they do not all collapse onto one type), labUser main.h:649
//  is TLabel, chkReadTorque1/2 main.h:464-465 are TCheckBox, edTorue0/1 main.h:466-467
//  are TEdit.  Zero behaviour change: each retired type held exactly the one member
//  used here with the same default ("" / false) and the unified replacements keep
//  those defaults; the ONLY objects of these types are the members of the single
//  file-scope W7T1_fMainTorque below (no by-value copy, no aggregate initialisation,
//  no sizeof/memset), so gaining a vtable disturbs nothing.
// AI(W906-R28TORQ) 20260925: the SIX torque widgets (lbArm0/1Torque, chkReadTorque1/2, edTorue0/1) LEFT this
//   TU-local struct -- they are now the REAL fMain members (forms/fMain.h, end of class TfMain, golden main.h
//   :464-467 / :797-798).  Why they had to move: they are cross-TU STATE, not display.  rs232.cpp's
//   ReadTorque_Panasonic writes edTorue0/1 and reads+clears chkReadTorque1/2; cinitial.cpp's ShowMainScreenPresure
//   rewrites lbArm0/1Torque->Caption, which case 12110 below compares against "1:Reading".  A copy that only this
//   TU can see meant rs232 could never deliver the torque value here (12110 waited forever on edTorue0=="").
//   The macro names are kept so the ~40 golden-shaped call sites below stay byte-identical.
//   labUser and SetOpenBin stay TU-local: neither is on the torque path (labUser = socket-heating countdown text,
//   SetOpenBin = open-bin UI), unchanged.
struct W7T1_TfMainTorqueSeam {
    TLabel    labUser;                                         // golden main.h:649 (TLabel*)
    void SetOpenBin(){}                                        // golden main.h -- offline: open-bin set no-op
};
static W7T1_TfMainTorqueSeam W7T1_fMainTorque;
// AI(W7T1-Integrate) 20260701: golden derefs these as POINTERS (fMain->lbArm0Torque->Caption,
// lbArm0Torque is TPanel*, labUser TLabel*, chkReadTorque* TCheckBox*, edTorue* TEdit* -- golden
// main.h:797/798/649/464-467).  Macros yield &member so the golden `->` deref compiles unchanged.
#define W7T1_FMAIN_LBARM0TORQUE   (fMain->lbArm0Torque)          // AI(W906-R28TORQ) 20260925: real fMain member (golden main.h:798)
#define W7T1_FMAIN_LBARM1TORQUE   (fMain->lbArm1Torque)          // AI(W906-R28TORQ) 20260925: real fMain member (golden main.h:797)
#define W7T1_FMAIN_LABUSER        (&W7T1_fMainTorque.labUser)
#define W7T1_FMAIN_CHKREADTORQUE1 (fMain->chkReadTorque1)        // AI(W906-R28TORQ) 20260925: real fMain member (golden main.h:464)
#define W7T1_FMAIN_CHKREADTORQUE2 (fMain->chkReadTorque2)        // AI(W906-R28TORQ) 20260925: real fMain member (golden main.h:465)
#define W7T1_FMAIN_EDTORUE0       (fMain->edTorue0)              // AI(W906-R28TORQ) 20260925: real fMain member (golden main.h:466)
#define W7T1_FMAIN_EDTORUE1       (fMain->edTorue1)              // AI(W906-R28TORQ) 20260925: real fMain member (golden main.h:467)
#define W7T1_FMAIN_SETOPENBIN()   W7T1_fMainTorque.SetOpenBin()

// --- COM2 torque / RTC-vision members (golden rs232.h) -------------------------
//  TCOM2Shim (atester_shims.h) offline exposes bCCDDummyRum(true) +
//  DoReleaseAndInspEnd(); the down-press tree also derefs the torque-write /
//  RTC-site-map / full-view-vision handshake members.  This TU-local extended
//  shim is a SUPERSET (keeps bCCDDummyRum=true + DoReleaseAndInspEnd no-op so the
//  ACTIVE preamble / case-600000 behaviour is IDENTICAL) and adds the down-press
//  members offline-inert:
//    iWriteAndCheckMotorTorque -> 1 (OK, == SOFT_SIMULTE),        [AI(W906-R28TORQ) 20260925: RETIRED -- forwards to the real COM2, see the note inside the struct]
//    GetReadTorueTask          -> 0 (NOT the 999 read-error sentinel), [AI(W906-R28TORQ) 20260925: RETIRED -- same]
//    OpenRTCComPortAgain       -> false (no re-open),
//    bRealTimeCom_ReceiveOK[]  -> false (vision never "received" -> RTC branches
//                                 fall to the golden time-out retry; never
//                                 dereference an untranslated RTC path offline),
//    rtSiteMap/rtFullTOK/rtFullTNG index enum ids.
//  #define COM2 redirects EVERY COM2 use in this TU (active + un-gated) to the
//  superset.  Integrate: extend the REAL TCOM2Shim with these members + drop the
//  #define.
enum { W7T1_rtSiteMap=0, W7T1_rtFullTOK=1, W7T1_rtFullTNG=2, W7T1_RT_N=8 };
struct W7T1_TCOM2Ext {
    bool bCCDDummyRum;                                         // golden rs232.h:157 -- offline true (== TCOM2Shim)
    bool bRealTimeCom_ReceiveOK[W7T1_RT_N];                    // golden rs232.h -- offline all false
    int  rtSiteMap, rtFullTOK, rtFullTNG;                      // golden rs232.h -- RT channel index ids
    void DoReleaseAndInspEnd(){}                               // golden rs232.h:160 -- offline no-op (== TCOM2Shim)
    // AI(W906-R28TORQ) 20260925: the three Index Z torque members now FORWARD to the real COM2 (TCOM2Shim,
    //   bodies rs232.cpp = golden rs232.cpp:1847-2009 / :3763 / :803).  Inside this struct `COM2` is still the
    //   real `extern TCOM2Shim *COM2` -- the redirecting #define comes after the struct.  Before: the write
    //   returned 1 ("OK") without ever reaching the drive, so on a real machine Prod.iMaxPreasure was never
    //   written while the SM went on as if it had been (NB2 R14 S1 / R16; 使用者 20260925 裁決第 6 條).
    //   SOFT_SIMULTE is unaffected: the real iWriteAndCheckMotorTorque has golden's own `#ifdef SOFT_SIMULTE
    //   return 1;` arm, and the atester call sites are in the `#else` arms anyway.
    int  iWriteAndCheckMotorTorque(int iArm, int iPre){ return COM2->iWriteAndCheckMotorTorque(iArm, iPre); } // golden rs232.h:109
    int  GetReadTorueTask(){ return COM2->GetReadTorueTask(); }                                           // golden rs232.h:162
    void InitReadTorueTask(){ COM2->InitReadTorueTask(); }                                                 // golden rs232.h:122
    void SendCommToVision(int /*ch*/, bool /*b*/){}            // golden rs232.h -- offline no-op
    bool OpenRTCComPortAgain(){ return false; }                // golden rs232.h -- offline: no re-open
    W7T1_TCOM2Ext():bCCDDummyRum(true),rtSiteMap(W7T1_rtSiteMap),
                    rtFullTOK(W7T1_rtFullTOK),rtFullTNG(W7T1_rtFullTNG)
    { for(int i=0;i<W7T1_RT_N;i++) bRealTimeCom_ReceiveOK[i]=false; }
};
static W7T1_TCOM2Ext W7T1_com2_ext;
#ifdef COM2
#undef COM2
#endif
#define COM2 (&W7T1_com2_ext)

#endif // W7T1_SEAM

//------------------------------------------------------------------------------
//  DoTestHeadMotor (golden :5562-8655) -- the central index/test SM, ~3000
//  lines.  ACTIVE: the entry preamble + the oracle-window cases (1, 200000,
//  300000, 400000, 500000, 600000, 2,3,4,5,6,7,9,10,15,20,21) reproduced with
//  cursor transitions VERBATIM so the test-head walks 4 -> 9 -> 10 -> 15 over
//  Sim HAL (offline Motor==NULL -> Gali Z/Y moves report complete immediately,
//  ISNormal()==true).  W7-T1: the dense down-press/test/torque/EP/load-cell
//  per-case tree (golden :6229-8650 -- case 30..600, 12000.., 14000.., 122100..,
//  142100.., the RTC/load-cell branch, the cContact ROI hand-off) is now
//  TRANSLATED + ACTIVE (below), entered from the active window (case 21 ->
//  30/60/100/115; case 9 -> 30000).  The deep externals (torque(COM2)/EP-DAQ/
//  RTC-vision/cContact-ROI/torque-UI) route through the W7T1_SEAM stand-ins
//  above (offline-inert); MOT[] Z/Y + Suck/Socket are REAL.  The CCD-form
//  accesses on the install path stay gated (offline bC02InstallCCD==false).
//------------------------------------------------------------------------------
//AI(W906-THM-600K) 20260927: golden 的 case 600000／10000～10030 用到、本檔原本看不到的宣告（golden atester.cpp 由 main.h 等一起帶進來）
#include "Interface/InterfaceSYS.h"      // SendCommand_ESD、ESD_DECAY_TEST
bool CheckShuttleSensorBroken_1(bool bRefreshCheck, bool bRight);               // acarry.h:50（本體 acarry.cpp:7322）
bool CheckShuttleSensorBroken_2(bool bRefreshCheck, bool bRight);               // acarry.h
bool CheckIndexArmInitState();                                                  // 本檔檔尾（golden 同檔）
void DoTestHeadMotor()
{
    static int iRetryCount=0;
    int &Task=iTestHeadMotorTask;
    int ret=0;
    bool flag=false;
    AnsiString str;
    // W7-T1: golden DoTestHeadMotor locals (golden atester.cpp:5564-5578) that the
    // now-un-gated down-press/torque/EP tree consumes.  W6.4 originally elided
    // these while the tree was gated; restored here VERBATIM (minus bFlag/ccRet,
    // which the tree never references).  golden atester.cpp:5564-5578.
    static int iSiteCount=0;
    static int iToqureCount=0;
    static bool bOneTimeFlag=true;
    static bool bFirstTime=true;
    static bool bintered1=true;                                                 //Isaac 20200922 : record indexArmY encoder value vs command value  (golden atester.cpp:5570)
    static DWORD startTick=-1, endTick=0, nowTick=0;
    static AnsiString ErrPart="";
    static bool bFlag[2]={false, false};                                        //AI(W906-THM-600K) 20260927: golden :5569 —— case 10000～10030（F16）用到，照 golden 補回
    int ccRet=0;                                                                //AI(W906-THM-600K) 20260927: golden :5575 的 ccRet（case 10010 用到）
    int TorqueData, iIndexUpPos=0, iContactZ=0;
    int sp, iMaxPreasure=0, iIndexArm[3]={0, 0, 0};
    bool flag2, bIndexSuckCheck;
    bool TMode=false;

    if(CUSTOMER_CODE==CC_SIGURD_HUKOU)                                          //KaiChen 20200826 ：矽格-湖口，要求IndexCheck使用Contact高度不使用Offset
    {
        iIndexArmCheck_SG_Arm1=Offset.iIndexArmContact[0];
        iIndexArmCheck_SG_Arm2=Offset.iIndexArmContact[1];
    }
    else
    {
        iIndexArmCheck_SG_Arm1=0;
        iIndexArmCheck_SG_Arm2=0;
    }

    if(IniConfig.bEnableCCDUSETCPIP)                                            //kevin 20110811 start
    {
        if(ScanCCDProgram())
        {
            CCDInterfaceForm->CCDRunExec();
        }
    }                                                                           //kevin 20110811 End

    if(iPauseBackUp!=-1 &&
       FTestSuck.IsShtSuckFinish()==true &&                                     //ChungHung 20110901 add
       FTestSuck.IsShtDestroyFinish()==true   &&
       BTestSuck.IsShtSuckFinish()==true &&
       BTestSuck.IsShtDestroyFinish()==true)
    {
        return;
    }

    if(USE_16_HEATER==eht16Heater     || USE_16_HEATER==eht16HeaterEJ1N ||          //AI(W906-FLOW-2) 20260928: golden :5609-5621 translated in these same 5 lines (golden's 13 lines packed so nothing below moves); was an empty "#if 0 TODO(W7)" whose reason ("no translated home") is stale: CheckIndexConnect is live at csystem.cpp:21807 (= golden csystem.cpp:20571-20594), ShowErrorMessage at canary_support.cpp:92, ccRet/ret are the locals at :6064/:6049
       USE_16_HEATER==eht32HeaterEJ1N ||                                        //Steven 20140923 : Index使用EJ1N版32組加熱器
       USE_16_HEATER==eht32HeaterKT4H ||                                        //Steven 20150211 : Index使用KT4H版32組加熱器
       USE_16_HEATER==eht16HeaterDTME08 || USE_16_HEATER==eht32HeaterDTME08)    //JimmyChiu 20210923 : Index使用DTME08版16組 / 32組加熱器
    { ccRet=CheckIndexConnect(); if(ccRet>0) { ret=ShowErrorMessage("WAR0360", K_RETRY, MTestZ1+(ccRet-1)); } }   //20111130  Dell    Connect Check Start / End

    if(bIndexCheckState)                                                        //wei 20150903 如果有Auto clean 就重頭開始
    {
        bIndexCheckState=false;
        Task=1;
    }

    if(MACHINE_HAS_AUTO_ALIGNMENT_CCD &&                                        //ChungHung 20210113 add for Alignment CCD start
       TestIF.bEnableAutoAlignment==true &&                                     //KenHsieh 20210813 : add CCD AUTO ALIGNMENT
       (LastSet.iRealDummy==HAS_TRAY ||
        LastSet.iRealDummy==REALLY))
    {
        if(lInArmAutoAlignmentCKTimingFlag!=0x00   || bRunInArmAutoAlignment ||
           lOutArmAutoAlignmentCKTimeingFlag!=0x00 || bRunOutArmAutoAlignment)
            return;
    }

    if((bSht1LoseICErr==true ||
        bSht2LoseICErr==true) &&
       LastSet.iRealDummy==REALLY)                                              //Jimmychiu 20230921 : add index arm stop when outshuttle lose ic
    {
        return;
    }

    if(bEject)                                                                  //JerryYang 20251020 : 渠梁半清機功能
    {
        return;
    }

    switch(Task)
    {
        case 1:                                                                 //確認Index Arm 吸嘴狀態
            if(CUSTOMER_CODE==CC_ASE_KaohSiung && IniConfig.bG11ASEReport)
                ReadWriteTrayID(true);                                          // kevin 20220618 read Tray ID

            if(Prod.TestZ1_Test==0)
                iSocketSenSosPos1=-1000;                                        //kevin 20150613 關arm 設定可判斷位置
            else
                iSocketSenSosPos1=Prod.TestZ1_Test-Prod.TestZ1_Drop_Offset+2000;

            if(Prod.TestZ2_Test==0)
                iSocketSenSosPos2=-1000;                                        //kevin 20150613 關arm 設定可判斷位置
            else
                iSocketSenSosPos2=Prod.TestZ2_Test-Prod.TestZ2_Drop_Offset+2000;

            if(bUseInitTempOffset)                                              //Steven 20141117 : 起測時溫度要補Offset
            {
                RecordProcess("After index check trigger initial offset function.(Temp)");
                iInitContactCount=0;                                            //Steven 20141117 : 起測時溫度要補Offset
                fHeaterOK=false;
            }

            if(INSTALL_SOCKET_CLAMP)                                            //JerryYang 20160607 機台選用分離機構 需偵測socket sensor
            {
                if(Sen[SnSocketHasClamp1].IsOn() || Sen[SnSocketHasClamp2].IsOn())
                {
                    ShowMyMessage("Please check the socket sensor","socket sensor偵測異常");
                    break;
                }
            }
            Task=200000;
            break;
        case 200000:                                                            //Steven 20160318 : 真空產生器自檢功能
            FTestSuck.ResetAll();
            BTestSuck.ResetAll();
            for(int i=0; i<FTestSuck.iShtRow; i++)                              //JerryYang 20160727 修正沒使用的真空產生器也吸真空
            {
                for(int j=0; j<FTestSuck.iShtCol; j++)
                {
                    if(INDEX_SUCKER_TYPE==1)                                    //Steven 20111202
                    {
                        fiosetview->bIndexSuck[0][i][j]=true;
                    }
                    else
                    {
                        FTestSuck.Suck[i][j].On();                              //kevin 20110504 check 掉料
                    }
                }
            }

            if(iProcessIndexSuckDestroyCnt==0 ||                                //RogerYang 20250930 : RogerYang 瑞薩FT-CT 真空產生器自檢功能，一段時間後才重置，避免掉壓
                iProcessIndexSuckDestroyCnt>15)
            {
                iProcessIndexSuckDestroyCnt=0;
            bIndexCheck1=true;                                                  //kevin 20170120
            Task=300000;
            }
            else
            {
                Task=600000;
            }

            if(TestIF_File.bRENESAS_EnableFTCT==true)                           //RogerYang 20250930 : RogerYang 瑞薩FT-CT 真空產生器自檢功能，一段時間後才重置，避免掉壓
                iProcessIndexSuckDestroyCnt++;
            break;
        //AI(W906-THM-600K) 20260927: golden atester.cpp:5719-6035 逐行照翻（case 300000～500000 的真空自檢、case 600000 全段、F16 的 case 10000～10030、case 11）。
        //  以前這裡是 W6.4 的簡化版：300000／400000／500000 只剩 Task=下一步（閘註明的 golden 行號也不對），
        //  600000 只留 golden case 11 的尾段（:6006-6035，CCD-TCPIP／REAL_TIME_CCD 選下一步）、直接跳 9 ——
        //  跳過後排 Index 吸嘴開真空（400000）、bIndexCheck1／2、CheckIndexArmInitState、空吸嘴真空 Normal、
        //  F16 飛梭感測器斷線檢查、起測延遲旗標（bNeedInitialTestDelay／bDoEveryFirstDeviceFunctionUseInitialDelay／
        //  bTestFinishToNextTestOver…）、ESD 衰減測試、case 11 的 bPlaceToShuttleFirst→20000 與 D71／D69「這次不做 Index Check」→150／1500。
        //  動作流程對照量到（INBOX 第 49 列：真機 FT005054 的 TestHeadMotorTask 是 …→500000→600000→11→9，移植樹沒有 11）。
        //  golden 400000 沒有 break、直接落到 500000（照翻）。移植樹沒有的相依才閘（GATE(W906-THM-600K)，理由寫在各自那一行）。
        case 300000:
            if(INDEX_SUCKER_TYPE==0 || fiosetview->ProcessIndexSuckDestroy1())
            {
                bIndexCheck1=false;                                             //kevin 20170120
                Task=400000;
            }
            break;
        case 400000:
            for(int i=0; i<BTestSuck.iShtRow; i++)
            {
                for(int j=0; j<BTestSuck.iShtCol; j++)
                {
                    if(INDEX_SUCKER_TYPE==1)                                    //Steven 20111202
                    {
                        fiosetview->bIndexSuck[1][i][j]=true;
                    }
                    else
                    {
                        BTestSuck.Suck[i][j].On();                              //kevin 20110504 check 掉料
                    }
                }
            }
            bIndexCheck2=true;                                                  //kevin 20170123 (Steven) 幫忙關閉真空 AUTOCLEAN 流程不會去關
            Task=500000;
        case 500000:
            if(INDEX_SUCKER_TYPE==0 || fiosetview->ProcessIndexSuckDestroy2())
            {
                bIndexCheck2=false;                                             //kevin 20170123 (Steven) 幫忙關閉真空 AUTOCLEAN 流程不會去關
                Task=600000;
            }
            break;
        case 600000:
            flag=CheckIndexArmInitState();
            if(flag==true)
            {
                Task=1;
                break;
            }
            else                                                                                                        //狀態正常
            {
                for(int i=0; i<FTestSuck.iShtRow; i++)
                {
                    for(int j=0; j<FTestSuck.iShtCol; j++)
                    {
                        if(FTestSuck.Item[i][j]==NULL_IC)                                                               //jou 2016-04-29 修正回Home Index drop error
                            FTestSuck.Suck[i][j].Normal();
                        if(BTestSuck.Item[i][j]==NULL_IC)
                            BTestSuck.Suck[i][j].Normal();
                    }
                }
                bIndexCheck1=false;                                                                                     //kevin 20170123 (Steven) 幫忙關閉真空 AUTOCLEAN 流程不會去關
                bIndexCheck2=false;                                                                                     //kevin 20170123 (Steven) 幫忙關閉真空 AUTOCLEAN 流程不會去關

                if(IniConfig.bF16CheckShuttleSensorBroken)                                                              //Steven 20221213 : 確認shuttle 有沒有斷線
                {
                    bDoingF16=true;
                    Task=10000;
                }
                else
                {
                    Task=11;
                }

                if(IniConfig.bEnableCCDUSETCPIP)
                {
                    if(CCDInterfaceForm->bAtestScanCCDProgram)                                                          //if(ScanCCDProgram())  kevin 20110811
                    {
#if 0 // GATE(W906-THM-600K) golden :5786-5786 —— CCDInterfaceForm 在移植樹是 TCCDInterfaceFormShim（atester_shims.h），沒有 CloseCCDForm（同 aTester_Front.cpp GATE K1F7）
                        CCDInterfaceForm->CloseCCDForm();
#endif // GATE(W906-THM-600K)
                        RecordProcess("Stop CCD check.");
                    }
                }
                else
                {
                    if(REAL_TIME_CCD && !COM2->bCCDDummyRum)
                    {
#if 0 // GATE(W906-THM-600K) golden :5794-5794 —— COM2 在本檔是 W7T1_TCOM2Ext 替身，沒有 rtLightOn（RTC 視覺指令沒翻）
                        COM2->SendCommToVision(COM2->rtLightOn, true);                                                  //jou 2012-03-29 RTC啟動時,自動將燈箱打開
#endif // GATE(W906-THM-600K)
                        DoTestHeadMotorDelay.SetSecAndOn(3);
                    }
                }

                if(CosFunction.bHiSiliconFunction==true)                                                                //kevin 20200110 add initial
                    bHISIInitiayDelay=true;                                                                             //kevin 20200110 add 海司強至initial delay

                if(Prod.bEveryFirstDeviceUseInitialDelay)                                                               //ChungHung 20140425 add for TSMC Device
                {
                    if(bUseInitTempOffset)                                                                              //Steven 20150707 : Add log for initial offset  //Ifor 20180116 (Steven) : add KYEC 常溫使用 Initial start delay
                    {
                        RecordProcess("After first device trigger initial offset function.");
                    }

                    iInitContactCount=0;                                                                                //Steven 20141117 : 起測時溫度要補Offset
                    if(CosFunction.bFuncStateStopFirtDelay==true)                                                       //jou 2014-09-03 Function State Stop Firt Initital Delay Time
                    {
                        if(bPiggyBackIndexCheck==false &&
                           LastSet.iRunStartMode!=rsmAutoSiteMap)
                        {
                            bNeedInitialTestDelay=true;
                            Prod.iInitialDelay=TestIF.iInitialDelay;                                                    //ChungHung 20141210 add for SCK want to every event have delay
                        }
                    }
                    else
                    {
                        if(bUseInitDelay)                                                                               //ChungHung 20141030 add must know Initial start or not //ChungHung 20141027 add 只有加熱模式需要Initial start delay count  //Ifor 20180116 (Steven) : add KYEC 常溫使用 Initial start delay
                        {
//                            if(IniConfig.bSPILFunction && (HasICUnderMachine() || HasAnyICInMachine()))     //JerryYang 20251106 : fix initial start的auto clean預熱秒數短    //JerryYang 20241118 : 矽品明仁 auto clean執行後預熱時間錯誤
//                            {
//                                if(bDoAfterAutoCleanFunctionUseInitialDelay)
//                                {
//                                    bDoEveryFirstDeviceFunctionUseInitialDelay=false;
//                                }
//                                else
//                                {
//                                    bDoEveryFirstDeviceFunctionUseInitialDelay=true;
//                                }
//                            }
//                            else
                            {
                                bDoEveryFirstDeviceFunctionUseInitialDelay=true;                                        //ChungHung 20140105 add for SCK have order
                            }
                        }
                        else
                        {
                            bDoEveryFirstDeviceFunctionUseInitialDelay=false;                                           //ChungHung 20140105 add for SCK have order
                        }
                    }
                }

                if(bUseInitDelay)                                                                                       //kevin 20160311 取得測試機間隔時間    //Ifor 20180116 (Steven) : add KYEC 常溫使用 Initial start delay
                {
                    if(Prod.bTestFinishToNextTestOver)
                    {
                        iInitContactCount=0;                                                                            //Steven 20160519 : 起測時溫度要補Offset
                        bTestFinishToNextTestOver=true;
                        bFirstTest=true;
                        bTestOverTimeTempOffsetF=false;
                    }
                    else
                    {
                        bTestFinishToNextTestOver=false;
                    }

                    if(Prod.bTestStartToNextTestStart)
                    {
                        iInitContactCount=0;                                                                            //Steven 20160519 : 起測時溫度要補Offset
                        bTestStartToNextTestStart =true;                                                                //kevin 20181031 (Steven) : add SOT start SRQ41 send next SRQ 41
                        bFirstTest=true;
                        bTestOverTimeTempOffsetF=false;
                    }
                    else
                    {
                        bTestStartToNextTestStart =false;                                                               //kevin 20181031 (Steven) : add SOT start SRQ41 send next SRQ 41
                    }
                }
                SendCommand_ESD(ESD_DECAY_TEST);                                                                        //Steven 20140722 : For ESD
            }
            break;
        case 10000:                                                             //Steven 20221213 : 確認shuttle 有沒有斷線
            if(MOT[MTestZ1].Gali_Two_ZAxis_Move(Prod.TestZ1_Safe, iSpeedSlow, "DoTestHeadMotor 10000"))
            {
                bFlag[0]=false;
                bFlag[1]=false;
                CheckShuttleSensorBroken_1(true, false);
                CheckShuttleSensorBroken_2(true, false);
                Task=10010;
            }
            break;
        case 10010:
            if(TestIF_File.iTestMode==SingleSite  || TestIF_File.iTestMode==DualSite    ||  TestIF_File.iTestMode==TriSite1X3   ||                              //Steven 20230508 : 針對IC比較大的檢查位置要多動一點
               TestIF_File.iTestMode==DualSite2x1 || TestIF_File.iTestMode==QualSite2X2 ||  TestIF_File.iTestMode==QualSite2X2N ||
               TestIF_File.iTestMode== _6Site2X3  || TestIF_File.iTestMode==_6Site2X3N  ||
               TestIF_File.iTestMode==_8Site2X4N)                                                                                                               //Wei 20231211 : 2X4NN Mode
            {
                ccRet=1000;
            }
            else
            {
                ccRet=500;
            }

            if(bFlag[0]==false)
                bFlag[0]=MOT[MInShuttle1].MotorMove(Prod.InSHT[0].iLeft+ccRet);
            if(bFlag[1]==false)
                bFlag[1]=MOT[MInShuttle2].MotorMove(Prod.InSHT[1].iLeft+ccRet);
            if(bFlag[0] && bFlag[1])
            {
                CheckShuttleSensorBroken_1(false, false);
                CheckShuttleSensorBroken_2(false, false);
                bFlag[0]=false;
                bFlag[1]=false;
                DoTestHeadMotorDelay.SetSecAndOn(1);
                Task=10020;
            }
            break;
        case 10020:
            if(DoTestHeadMotorDelay.Off())
            {
                if(bFlag[0]==false)
                    bFlag[0]=MOT[MInShuttle1].MotorMove(Prod.InSHT[0].iLeft);
                if(bFlag[1]==false)
                    bFlag[1]=MOT[MInShuttle2].MotorMove(Prod.InSHT[1].iLeft);
            }

            if(bFlag[0] && bFlag[1])
            {
                bFlag[0]=false;
                bFlag[1]=false;
                Task=10030;
            }
            break;
        case 10030:
            if(FLCarryKit.HasRealIC()==false)
                bFlag[0]=CheckShuttleSensorBroken_1(false, true);
            if(BLCarryKit.HasRealIC()==false)
                bFlag[1]=CheckShuttleSensorBroken_2(false, true);

            if(bFlag[0] || bFlag[1])
            {
                Task=10000;
            }
            else
            {
                Task=11;
            }
            break;
        case 11:
            bDoingF16=false;                                                    //Steven 20221213 : 確認shuttle 有沒有斷線
            if(CUSTOMER_CODE==CC_SCK &&                                         //ChungHung 20141111 modify for SCK request
               LastSet.iTemperature==Tempture_Hot &&
               iTesterDucking>0)
            {
                return;
            }

#if 0 // GATE(W906-THM-600K) golden :5952-5967 —— RTC 燈箱開啟等待：COM2 替身沒有 rtLightOn／bRealTimeCom_ReceiveOK 的那一格（RTC 視覺沒翻）；REAL_TIME_CCD 關著的機台這段本來就不走
            if(REAL_TIME_CCD==true && !COM2->bCCDDummyRum)                      //jou 2012-03-29 RTC啟動時,自動將燈箱打開
            {
                if(COM2->bRealTimeCom_ReceiveOK[COM2->rtLightOn]==false)
                {
                    if(IniConfig.bSPILFunction==false)                          //Steven 20140516 : 暫時不關RTC的燈   //JerryYang 20170328 (Jou) 矽品客戶碼統一用SPILFunction
                    {
                        if(DoTestHeadMotorDelay.Off())
                        {
                            if(COM2->OpenRTCComPortAgain())                     //ChungHung 20140520 add 不要第一次TimeOut就秀錯誤訊息
                                ShowMyMessage("RTC Light On Fail!!","RTC光源開啟失敗!!");
                            Task=1;
                        }
                        return;
                    }
                }
            }
#endif // GATE(W906-THM-600K)

            if(bPlaceToShuttleFirst)                                            //是否先放置Shuttle
            {
                Task=20000;
                break;
            }

            iRetryCount=0;
            bReadFrontTestArmTorque=false;
            bReadRearTestArmTorque=false;

            if(CosFunction.bIndexCheckCanTurnOff &&                             //Isaac 20211019 : 可選擇做index check的時機
               ((IniConfig.iD71IndexCheckOnOffMode==0 && bLotStartEndNeedIndexCheck==false) ||
                (IniConfig.iD71IndexCheckOnOffMode==1 && bIndexJamNeedIndexcheck==false) ||
                 IniConfig.iD71IndexCheckOnOffMode==2))
            {
                if(IniConfig.bUseAutoSiteMapping &&
                   CosFunction.bUSEJCETSiteMapMode==false &&
                   LastSet.iRunStartMode==rsmAutoSiteMap &&
                   bSiteMappingCHKOK==false)                                    //Steven 20190313 : Fixed for auto site mpa no open bin when enable [D55]
                    Task=150;
                else
                    Task=1500;                                                  //Steven 20231208 : 1550 --> 1500
            }
            else if(CosFunction.bAfterAutoCleanNoIndexCheck &&                  //Steven 20191212 : 劉仁洲說Auto Clean只要作一次Index Check
                    (IniConfig.iD69IndexCheckModeForAutoClean==1 ||
                     IniConfig.iD69IndexCheckModeForAutoClean==2) &&
                    bAutoCleanFinishOnlyUseRTC==true)
            {
                bAutoCleanFinishOnlyUseRTC=false;
                if(IniConfig.bUseAutoSiteMapping &&
                   CosFunction.bUSEJCETSiteMapMode==false &&
                   LastSet.iRunStartMode==rsmAutoSiteMap &&
                   bSiteMappingCHKOK==false)                                    //Steven 20190313 : Fixed for auto site mpa no open bin when enable [D55]
                    Task=150;
                else
                    Task=1500;                                                  //Steven 20231208 : 1550 --> 1500
            }
            else if(IniConfig.bEnableCCDUSETCPIP)
            {
                CCDInterfaceForm->CCDTimerOnOff(IniConfig.bC02InstallCCD);      //Steven 20110809
                if(IniConfig.bC02InstallCCD==true)
                {
                    fMain->LightOn();
                    DoTestHeadMotorDelay.SetSecAndOn(3);
                    iCCDTimeOutCount=0;
                    Task=2;
                    break;
                }
                else
                {
                    Task=9;
                    break;
                }
            }
            else
            {
                if(REAL_TIME_CCD==true && !COM2->bCCDDummyRum)                  //ChungHung 20121011 鉬FullView 謖IndexCheck
                {
                    COM2->DoReleaseAndInspEnd();
                    Task=iCASE_REAL_CCD2;
                }
                else
                {
                    Task=9;
                }
            }
            break;
        case 2:
            if(MOT[MTestZ1].Gali_Two_ZAxis_Move(Prod.TestZ1_Safe, iSpeedSlow, "DoTestHeadMotor 2"))
            {
                Task=3;
            }
            break;
        case 3:
            if(MOT[MTestY1].GalilTwoY_Move(Prod.TestY1_Front, Prod.TestY2_Rear, iSpeedY, "DoTestHeadMotor 3"))
            {
                Task=4;
            }
            break;
        case 4:
            if(IniConfig.bC02InstallCCD==false)
            {
                Task=9;
                break;
            }
            fMain->lbCCDStatus->Visible=true;
            if(DoTestHeadMotorDelay.Off()==false)
                break;
            // golden :6055-6063 -- CCD identification kick-off (CCDInterfaceForm
            // ->CCDIdentificationOpen/CCDIdentification).  Gated CCD form access;
            // offline this point is unreachable (bC02InstallCCD==false above).
#if 0 // TODO(W7) -- golden :6055-6063 (CCDInterfaceForm identification)
#endif
            Task=5;
            DoTestHeadMotorDelay.SetSecAndOn(20);                               //CCDTimeOutSec);  20110810 設定5秒太短會一直取像
            DoTestHeadMotorDelay2.SetMSAndOn(200);
            break;
        case 5:
            // golden :6067-6128 -- CCD result poll (CCDInterfaceForm program /
            // identification status, fShowMessage form click).  Gated CCD form.
#if 0 // TODO(W7) -- golden :6067-6128 (CCDInterfaceForm result poll)
#endif
            Task=9;
            break;
        case 6:
            if(MOT[MTestY1].GalilTwoY_Move(Prod.TestY1_Front, Prod.TestY2_Rear, iSpeedY, "DoTestHeadMotor 6"))
            {
                Task=7;
            }
            break;
        case 7:
            if(IndexAlarmInArmAway()==false)                                    //Steven 20130613 : Index異常時, In Arm要先讓位功能
            {
                return;
            }
            fMain->Pause("DoTestHeadMotor");
            // golden :6166-6167 -- CCDInterfaceForm->CCDIdentification + program
            // existence flag.  Gated CCD form.
#if 0 // TODO(W7) -- golden :6166-6167 (CCDInterfaceForm CCD identification)
#endif
            Task=1;
            break;
        case 9:                                                                 //IndexZ1 and IndexZ2 皆移至安全位置
            if(MOT[MTestZ1].Gali_Two_ZAxis_Move(Prod.TestZ1_Safe, iSpeedSlow, "DoTestHeadMotor 9"))
            {
                if(TestIF_File.iTestType==TTL_MODE &&
                   (TTL_CARD_TYPE==2 || TTL_CARD_TYPE==3))                      //Isaac 20210309 :TTL RS232通訊
                    SendTTLRS232CSOTsignal();

                CCDInterfaceForm->CCDTimerOnOff(false);                         //Steven 20110809
                iCCDTimeOutCount=0;
                Task=10;

                if(IniConfig.bD24EnableEPCheckFuntion==true &&
                   IniConfig.bIndexEveryTimeCheckEP==true)
                {
                    if(CUSTOMER_CODE!=CC_VTEST_Shanghai)                        // &&                   //jou 20210911 : 上海無錫 張冬冬 index check 不檢查EP
//                       CUSTOMER_CODE!=CC_SCC)                                 //Steven 20240926 : JSCC 李小川 index check 不檢查EP
                    {
                        InitIndexEveryTimeCheckEP();
                        Task=30000;
                    }
                }
                else
                {
                    bIndexEveryTimeCheckEPing=false;
                }
            }
            break;
        case 10:                                                                //判斷IndexZ1 and IndexZ2 是否已在安全位置
            if(MOT[MTestZ1].ISNormal()==false)
            {
                iRetryCount++;
                if(iRetryCount>3)
                {
                    ShowIndexMotorError(AnsiString("DoTestHeadMotor10"));
                    iRetryCount=0;
                }
                else
                {
                    Task=9;
                }
                return ;
            }
            Task=15;
            break;
        case 15:                                                                //Steven 20220721 : Alarm之前, Index要先讓開
            if(MOT[MTestY1].GalilTwoY_Move(Prod.TestY1_Front, Prod.TestY2_Rear, iSpeedY, "DoTestHeadMotor 15"))
            {
                Task=20;
            }
            break;
        case 20:
            if(IniConfig.bC08_SocketSensor && TestIF_File.bEnSocketSensor &&    //kevin 20160209 //kevin 20130504 socket sensor detect error
               (TestIF_File.bSocketDisibleinitialcheck==false ||                //JerryYang 20170426 (Steven) 修正Disable socket sensor detect
                CUSTOMER_CODE==CC_GIGAS))                                       //Isaac 20220126 : 全智要求Index arm在上要強制偵測sensor(應該為off)
            {
                str="check socket sensor state must off, sensor: ";
                for(int i=0; i<TestIF_File.iSocketCount; i++)
                {
                    if(Sen[SThreadPara.iSocketSensor[i]].Enable &&
                       Sen[SThreadPara.iSocketSensor[i]].IsOn())                //kevin 20150429 : Arm在上, 檢查無遮斷, 要Off
                    {
                        flag=true;
                        bIsSocketSensor=true;
                        str+=IntToStr(i+1);
                    }
                }

                if(flag)
                {
                    ShowErrorMessage("WAR0322", K_RETRY, MTestZ1, false, str);  //kevin 20130504 socket sensor
                    return;                                                     //kevin 20150429確認 socket sensor是否正常
                }
            }
            Task=21;
            break;
        case 21:                                                                //Index1 or Index2 是否需要拋下IC
            EPSwitchOnOff(eEPSwBoth);                                           //Steven 20250417 : fixed for [D58]

            if(bFTestSuckDrop==true)
            {
                Task=30;
            }
            else if(bBTestSuckDrop==true)
            {
                Task=60;
            }
            else
            {
                if(CosFunction.bSortingBy2DList==true &&
                   LastSet.iTester==_2D_SORT &&
                   TestIF_File.bSortingBy2DIDList==true)                        //Frank 20221122 : 2DID sorting for ATK
                    Task=115;
                else
                    Task=100;
            }
            break;
        // golden atester.cpp:6229-8650 -- the dense down-press / test / torque /
        // EP / load-cell per-case tree, TRANSLATED FAITHFULLY (W7-T1).  Every case
        // id, iTestHeadMotorTask transition, goto/fall-through/early-return and
        // #ifdef SOFT_SIMULTE / INDEX_PROTECT_TMOVE region is reproduced VERBATIM.
        // MOT[] Z/Y moves (S0-converged) + Suck/Socket bin writes are REAL; the
        // torque(COM2)/EP-DAQ/RTC-vision/cContact-ROI/torque-UI derefs are routed
        // through the W7T1_SEAM stand-ins above (offline-inert).  Entered from the
        // ACTIVE window: case 21 -> 30/60/100/115 and case 9 -> 30000.
        // AI(W7T1-Translate) 20260701.
        case 30:                                                                //Index1移至中間 Index2移至後面
            if(MOT[MTestY1].GalilTwoY_Move(Prod.TestY1_Middle, Prod.TestY2_Rear, iSpeedY, "DoTestHeadMotor 30"))
                Task=40;
            break;
        case 40:                                                                //移至下拋高度
            if(MOT[MTestZ1].Gali_MotMove(Prod.TestZ1_Test-Prod.TestZ1_Drop_Offset, iSpeedSlow))
            {
                IndexStatus=Z1Down_Z2Up;
                Task=50;
            }
            break;
        case 50:                                                                //拋下IC
            for(int i=0; i<FTestSuck.iShtRow; i++)
            {
                for(int j=0; j<FTestSuck.iShtCol; j++)
                {
                    if(FTestSuck.Item[i][j]==HAS_IC)
                    {
                        FTestSuck.Suck[i][j].On();
                        if(INDEX_SUCKER_TYPE==1)                                //Steven 20111202
                        {
                            fiosetview->bIndexSuck[0][i][j]=true;
                        }
                    }
                    DoTestHeadMotorDelay.SetSecAndOn(0.3);                      // delay 0.3 sec for ic down
                }
            }
            Task=55;
            break;
        case 55:
            if(DoTestHeadMotorDelay.Off())
            {
                bFTestSuckDrop=false;
                Task=1;
            }
            break;
        case 60:
            if(MOT[MTestY1].GalilTwoY_Move(Prod.TestY1_Front, Prod.TestY2_Middle, iSpeedY, "DoTestHeadMotor 60"))
                Task=70;
            break;
        case 70:
            if(MOT[MTestZ2].Gali_MotMove(Prod.TestZ2_Test-Prod.TestZ2_Drop_Offset, iSpeedSlow))
            {
                IndexStatus=Z1Up_Z2Down;
                Task=80;
            }
            break;
        case 80:
            for(int i=0; i<BTestSuck.iShtRow; i++)
            {
                for(int j=0; j<BTestSuck.iShtCol; j++)
                {
                    if(BTestSuck.Item[i][j]==HAS_IC)
                    {
                        BTestSuck.Suck[i][j].On();
                        if(INDEX_SUCKER_TYPE==1)                                //Steven 20111202
                        {
                            fiosetview->bIndexSuck[1][i][j]=true;
                        }
                    }
                    DoTestHeadMotorDelay.SetMSAndOn(100);                       // delay 0.3 sec for ic down
                }
            }
            Task=90;
            break;
        case 90:
            if(DoTestHeadMotorDelay.Off())
            {
                bBTestSuckDrop=false;
                Task=1;
            }
            break;
        case 100:                                                               //Index2 上IC是否有掉落
            if(bRearHeadICFallDown)
            {
                Task=110;
                break;
            }                                                                   //Index1 移至中間 Index2 移至後面

            if(MOT[MTestY1].GalilTwoY_Move(Prod.TestY1_Middle, Prod.TestY2_Rear, iSpeedY, "DoTestHeadMotor 100"))
            {
                Task=120;
            }
            break;
        case 110:
            if(MOT[MTestY1].GalilTwoY_Move(Prod.TestY1_Front, Prod.TestY2_Middle, iSpeedY, "DoTestHeadMotor 110"))
                Task=112;
            break;
        case 112:
            if(MOT[MTestZ2].Gali_MotMove(Prod.TestZ2_Test-Prod.TestZ2_Drop_Offset, iSpeedSlow))
            {
                IndexStatus=Z1Up_Z2Down;
                Task=141;
            }
            break;
        case 115:                                                               //JerryYang 20230322 : 2D SORT模式index arm不用下壓到socket
            if(MOT[MTestY1].GalilTwoY_Move(Prod.TestY1_Front, Prod.TestY2_Rear, iSpeedY, "DoTestHeadMotor 115"))
                Task=116;
            break;
        case 116:
            if(MOT[MTestZ2].Gali_MotMove(Prod.TestZ1_Safe, iSpeedSlow))
            {
                IndexStatus=Z1_Z2_Normal;
                Task=120;
            }
            break;
        case 120:                                                               //初始化馬達扭力及確認
            InitWriteAndCheckMotorTorqueTask();
            W7T1_FMAIN_LBARM0TORQUE->Caption="1:Writing";
            W7T1_FMAIN_LBARM1TORQUE->Caption="---";
            Task=12000;
        case 12000:
            if(USE_IO_CHANGE_TOQUE==true)                                       //jou 2012-06-21 Enable index I/O Change Toque
            {
                SW[SwIndexChangeToque1].Off();
                SW[SwIndexChangeToque2].Off();
            }

            #ifdef SOFT_SIMULTE
                ret=1;
            #else
                ret=COM2->iWriteAndCheckMotorTorque(0, Prod.iMaxPreasure);
            #endif

            if(ret==1)                                                          //扭力設定OK
            {
                iToqureCount=0;
                Task=12100;
            }
            else if(ret==2)                                                     //扭力設定NG
            {
                ShowMyMessage("Motor torque set error", "馬達扭力設定錯誤", "DoTestHeadMotor 12000");
                fAllMotorHome=false;
                Task=1;
                return ;
            }
            break;
        case 12100:
            Task=12101;
        case 12101:
            iIndexUpPos=0;

            if(CosFunction.bSortingBy2DList==true &&
               LastSet.iTester==_2D_SORT &&
               TestIF_File.bSortingBy2DIDList==true)                                                                                                            //Frank 20221122 : 2DID sorting for ATK
            {
                if(MOT[MTestZ1].Gali_Two_ZAxis_Move(Prod.TestZ1_Safe, iSpeedSlow, "DoTestHeadMotor 12101"))
                {
                    IndexStatus=Z1_Z2_Normal;
                    #ifdef SOFT_SIMULTE
                        ShowMainScreenPresure(0);
                        Task=12300;
                    #else
                        if(CUSTOMER_CODE==CC_KYEC_XILINX &&
                           IniConfig.bD01EnableReadTorque &&
                           IniConfig.bChangeKitNoHardStop==true &&
                           IniConfig.bRemeberAutoHeight==true)
                        {
                            Task=12102;
                            DoTestHeadMotorDelay.SetSecAndOn(IniConfig.dD01ReadTorqueDelayTime);
                        }
                        else
                        {
                            Task=12110;
                            DoTestHeadMotorDelay.SetSecAndOn(1);                                                                                                //Frank 20171030 (Steven) add Floating Shuttle Read Torque Delay
                            bFirstTime=true;
                        }
                    #endif
                }
            }
            else
            {
                if(MOT[MTestZ1].Gali_MotMove(Prod.TestZ1_Test-Prod.TestZ1_Drop_Offset+iIndexUpPos-iIndexArmCheck_SG_Arm1, iSpeedSlow))                          //KaiChen 20200826 ：矽格-湖口，要求IndexCheck使用Contact高度不使用Offset
                {
                    IndexStatus=Z1Down_Z2Up;
                    #ifdef SOFT_SIMULTE
                        ShowMainScreenPresure(0);
                        Task=12300;
                    #else
                        if(CUSTOMER_CODE==CC_KYEC_XILINX &&
                           IniConfig.bD01EnableReadTorque &&
                           IniConfig.bChangeKitNoHardStop==true &&
                           IniConfig.bRemeberAutoHeight==true)
                        {
                            Task=12102;
                            DoTestHeadMotorDelay.SetSecAndOn(IniConfig.dD01ReadTorqueDelayTime);
                            bFirstTime=true;
                        }
                        else
                        {
                            Task=12110;
                            DoTestHeadMotorDelay.SetSecAndOn(1);                                                                                                //Frank 20171030 (Steven) add Floating Shuttle Read Torque Delay
                            bFirstTime=true;
                        }
                    #endif
                    ReadTorqueDelay.SetSecAndOn(10);                                                                                                            //kevin 20210824 read torque wait alarm time
                }
            }
            break;
        case 12102:                                                             //Frank 20171030 (Steven) add Floating Shuttle Read Torque Delay
            if(DoTestHeadMotorDelay.Off())
            {
                Task=12110;
                DoTestHeadMotorDelay.SetSecAndOn(1);
                bFirstTime=true;
            }
            break;
        case 12110:
            W7T1_FMAIN_LBARM0TORQUE->Caption="1:Reading";

            if(DoTestHeadMotorDelay.Off()==false)
            {
                if(bFirstTime)
                {
                    bFirstTime=false;
                    W7T1_FMAIN_CHKREADTORQUE1->Checked=true;
                    W7T1_FMAIN_CHKREADTORQUE2->Checked=false;
                    W7T1_FMAIN_EDTORUE0->Text="";
                    COM2->InitReadTorueTask();
                    return;                                                     //kevin 20210824 clean Tourqe
                }
            }

            if(W7T1_FMAIN_EDTORUE0->Text=="")
            {
                if(ReadTorqueDelay.Off())                                       //kevin 20210824 read torque wait alarm time
                {
                    if(COM2->GetReadTorueTask()==999)                           //jou 2011-11-29防止Read Torue後數值被清掉，還傻傻的在那邊等
                    {
                        ShowMyMessage("The test head 1 Motor torque read error", "馬達扭力讀取錯誤", "DoTestHeadMotor 12110");
                        fAllMotorHome=false;
                        Task=1;
                    }
                }
                return;
            }
            else
            {
                 ShowMainScreenPresure(0);                                      //kevin 20210413
            }

            if(W7T1_FMAIN_LBARM0TORQUE->Caption=="1:Reading")                       //kevin 20210413
            {
                InitWriteAndCheckMotorTorqueTask();
                Task=120;
                RecordProcess("Arm1 Torque Reading.");
                return;
            }

            if(IniConfig.bD30EnableSiteModeSelect &&
               (TestIF.iShuttleMode==0 ||
               (TestIF.iShuttleMode==1 && TestIF.iShuttle_Sel==0)))             //KEVIN 20150613
            {
                if(IniConfig.bC08_SocketSensor &&
                   TestIF_File.bEnSocketSensor &&
                   TestIF_File.bSocketDisibleinitialcheck==false)               //JerryYang 20170426 (Stven) 修正Disable socket sensor detect
                {
                    str="index check socket sensor down sensor must on, ";
                    for(int i=0; i<TestIF_File.iSocketCount; i++)
                    {
                        if(IsNNMode()==NN_2Row)
                        {
                            if(i>=2 &&
                               Sen[SThreadPara.iSocketSensor[i]].Enable &&
                               Sen[SThreadPara.iSocketSensor[i]].IsOff())       //kevin 20150429 : Arm 1在下, 檢查遮斷, 要On
                            {
                                flag=true;
                                bIsSocketSensor=true;
                                str+=IntToStr(i+1);
                            }
                        }
                        else
                        {
//                            if(IniConfig.bD58UseArm1PickPlaceArm2Test==true &&     //Jimmychiu 20250809 : Mark for Gigas反應檢測功能失效
//                               TestIF_File.bArm1PickPlaceArm2Test==true)
//                            {
                                if(Sen[SThreadPara.iSocketSensor[i]].Enable &&
                                   Sen[SThreadPara.iSocketSensor[i]].IsOff())   //kevin 20150429 : Arm 1在下, 檢查遮斷, 要On
                                {
                                    flag=true;
                                    bIsSocketSensor=true;
                                    str+=IntToStr(i+1);
                                }
//                            }
//                            else
//                            {
//                                if(TestIF_File.iSensorCheckType[i]==2 &&
//                                   Sen[SThreadPara.iSocketSensor[i]].Enable &&
//                                   Sen[SThreadPara.iSocketSensor[i]].IsOff())   //Arm 1在下, 手臂上沒IC, 只檢查floating sensor必須遮到
//                                {
//                                    flag=true;
//                                    bIsSocketSensor=true;
//                                    str+=IntToStr(i+1);
//                                }
//                            }
                        }
                    }

                    if(flag)
                    {
                        ErrPart=str;                                            //Steven 20220721 : Alarm之前, Index要先讓開
                        Task=12150;
//                        ShowErrorMessage("WAR0322", K_RETRY, MTestZ1, false, str);//kevin 20130504 socket sensor
                        return;                                                 //kevin 20150429確認 socket sensor是否正常
                    }
                }
            }
            TorqueData=atoi(W7T1_FMAIN_EDTORUE0->Text.c_str());
            TorqueData=abs(TorqueData);

            if(IniConfig.bControlTorque)                                        //jou 2013-11-05 Index Control Torque
                iMaxPreasure=DeviceForm.iIndexTorqueMax;
            else
                iMaxPreasure=Prod.iMaxPreasure;

            if(CUSTOMER_CODE==CC_KYEC_XILINX &&
               IniConfig.bD01EnableReadTorque &&                                //Frank 20171030 (Steven) add Floting Shuttle Read Torque Delay
               IniConfig.bChangeKitNoHardStop==true &&
               IniConfig.bRemeberAutoHeight==true)
            {
                if(TorqueData>=(IniConfig.dD01ReadTorque+DeviceForm_File.dZ1Torue))
                {
                    ShowMyMessage("The test head 1, contact force over error", "Index 1 壓力過重錯誤", "DoTestHeadMotor 14110");
                    Task=12111;                                                 //kevin 20130418
                    return;
                }
            }
            else
            {
                if(TorqueData>=iMaxPreasure)
                {
                    if(iToqureCount>10)                                         //jou 981130 double check Torque
                    {
                        iToqureCount=0;
                        ShowMyMessage("The test head 1, contact force over error", "Index 1 壓力過重錯誤", "DoTestHeadMotor 12110");
                        Task=12111;                                             //kevin 20130418
                        return ;
                    }
                    else
                    {
                        iToqureCount++;
                        Task=12100;
                        break;
                    }
                }
            }

            ShowMainScreenPresure(0);
            InitWriteAndCheckMotorTorqueTask();

            if(IniConfig.bVTESTFunction==true)                                  //jou 20231102 : VTEST 增加 Torque log
                CheckAndRecodrTorque(0);

            Task=12200;
            break;
        case 12111:                                                             //kevin 20130418
            if(MOT[MTestZ1].Gali_Two_ZAxis_Move(Prod.TestZ1_Safe, iSpeedSlow, "DoTestHeadMotor 12111"))
                Task=12112;
            break;
        case 12112:
            ErrPart="The test head 1, contact force over error";                //kevin 20130418
            bIsContactforce=true;                                               //kevin 20130418 contact force over 需開們確認
            if(CosFunction.bIndexAreaOnlyCanUseSkip)                            //Steven 20141105 : Index內的所有異常都只能用Skip
                ShowErrorMessage("WAR0321", K_SKIP, MTestZ1, false, ErrPart);
            else
                ShowErrorMessage("WAR0321", K_RETRY, MTestZ1, false, ErrPart);
            Task=9;
            break;
        case 12150:                                                             //Steven 20220721 : Alarm之前, Index要先讓開
            if(MOT[MTestZ1].Gali_Two_ZAxis_Move(Prod.TestZ1_Safe, iSpeedSlow, "DoTestHeadMotor 12150"))
            {
                Task=12151;
            }
            break;
        case 12151:
            if(MOT[MTestY1].GalilTwoY_Move(Prod.TestY1_Front, Prod.TestY2_Rear, iSpeedY))
            {
                Task=12152;
            }
            break;
        case 12152:
            if(IndexAlarmInArmAway()==true)                                     //Steven 20130613 : Index異常時, In Arm要先讓位功能
            {
                ShowErrorMessage("WAR0322", K_RETRY, MTestZ1, false, ErrPart);  //kevin 20130504 socket sensor
                Task=9;
            }
            break;
        case 12200:
            if(USE_IO_CHANGE_TOQUE==true)                                       //jou 2012-06-21 Enable index I/O Change Toque
            {
                SW[SwIndexChangeToque1].Off();
                SW[SwIndexChangeToque2].Off();
            }

            ret=COM2->iWriteAndCheckMotorTorque(0, 300);
            if(ret==1)
            {
                W7T1_FMAIN_CHKREADTORQUE1->Checked=false;
                W7T1_FMAIN_CHKREADTORQUE2->Checked=false;
                Task=12300;
                iSiteCount=0;                                                   //kevin 20190530
            }
            else if(ret==2)
            {
                ShowMyMessage("The test head 1 Motor torque set error", "馬達扭力設定錯誤", "DoTestHeadMotor 12200");
                fAllMotorHome=false;
                Task=1;
                return ;
            }
            break;
        case 12300:
            if(ArmSpeed_File[IndexArm].bDevicConfirm &&
               INDEX_SUCKER_TYPE==1)                                            //kevin 20190709 add 20190629 回吸檢測一次 4 個 SITE
            {
                 Task=12301;
            }
            else if(LastSet.bD41TestSocketICCheckSkip==false)
            {
                if(IniConfig.bSPILFunction==true)                               //jou 2014-08-18 SPIL 關arm 不檢查真空值 //JerryYang 20170328 (Jou) 矽品客戶碼統一用SPILFunction
                {
                    if(TestIF_File.iShuttleMode==0 ||
                       (TestIF_File.iShuttleMode==1 &&
                        TestIF_File.iShuttle_Sel==0))
                        Task=121;
                    else
                        Task=130;
                }
                else
                {
                    if(CosFunction.bUseOneByOneIndexCheck==true &&              //Ifor 20180322 : add Use One By One Index Check
                       INDEX_SUCKER_TYPE==1)
                    {
                        if((DeviceForm.iSocketInitialICCheckPosition==1 &&
                            IniConfig.bTestIcCheckInContact==true) ||
                           (IniConfig.iD41SocketInitialICCheckPosition==1 &&
                            IniConfig.bTestIcCheckInContact==false))            //Above Socket
                        {
                            FTestSuck.ResetAll();
                            iSiteCount=0;
                            IndexCheckOneByOne(true, 0, iSiteCount);            //Ifor 20200617 : add Use One By One Index Check Function 整合
                            Task=50000;
                        }
                        else
                        {
                            Task=121;
                        }
                    }
                    else
                    {
                        Task=121;
                    }
                }
            }
            else
            {
                Task=130;
            }
            break;
        case 12301:
            if((DeviceForm.iSocketInitialICCheckPosition==1 &&
                IniConfig.bTestIcCheckInContact==true) ||
               (IniConfig.iD41SocketInitialICCheckPosition==1 &&
                IniConfig.bTestIcCheckInContact==false))                                                                                                        //Above Socket
            {
                iIndexUpPos=0;
                if((DeviceForm.iSocketInitialICCheckPosition==1 &&
                    IniConfig.bTestIcCheckInContact==true) ||                                                                                                   //kevin 20190708 add
                    (IniConfig.iD41SocketInitialICCheckPosition==1 &&
                     IniConfig.bTestIcCheckInContact==false))                                                                                                   //Above Socket
                {
                    iIndexUpPos=GetSocketCheckPos(Prod.TestZ1_Test);                                                                                            //kevin yang 20201014 : 2 --> 1
                }

                if(MOT[MTestZ1].Gali_MotMove(Prod.TestZ1_Test-Prod.TestZ1_Drop_Offset+iIndexUpPos-iIndexArmCheck_SG_Arm1, iSpeedSlow))                          //KaiChen 20200826 ：矽格-湖口，要求IndexCheck使用Contact高度不使用Offset
                {
                    IndexStatus=Z1Down_Z2Up;
                    if(ArmSpeed_File[IndexArm].bDevicConfirm &&                                                                                                 //kevin 20190629 回吸檢測一次 4 個 SITE
                       INDEX_SUCKER_TYPE==1)                                                                                                                    //kevin 20190530 add index check
                    {
                        IndexCheck4Site(true, 0, iSiteCount);
                        Task=12305;
                    }
                    else
                    {
                        Task=121;
                    }
                }
                break;
            }
            else
            {
                if(IniConfig.bSPILFunction==true)                                                                                                               //jou 2014-08-18 SPIL 關arm 不檢查真空值 //JerryYang 20170328 (Jou) 矽品客戶碼統一用SPILFunction
                {
                    if(TestIF_File.iShuttleMode==0 ||
                       (TestIF_File.iShuttleMode==1 &&
                        TestIF_File.iShuttle_Sel==0))
                        Task=121;
                    else
                        Task=130;
                }
                else
                {
                    Task=12302;
                }
            }
            break;
        case 12302:
            if(ArmSpeed_File[IndexArm].bDevicConfirm &&                         //kevin 20190629 回吸檢測一次 4 個 SITE
               INDEX_SUCKER_TYPE==1)                                            //kevin 20190530 add index check
            {
                IndexCheck4Site(true,0,iSiteCount);
                Task=12305;
            }
            else
            {
                Task=121;
            }
            break;
        case 12305:                                                             //kevin 20190531 index check 4 Site
            if(IndexCheck4Site(false, 0, iSiteCount))
            {
                iSiteCount++;

                if(iSiteCount<TestSocket.iShtCol/2)                             //JerryYang 20250120 : modify
                {
                    Task=12301;
                }
                else
                {
                    Task=122100;
                    iSiteCount=0;
                }
            }
            break;
        case 121:
            if(CosFunction.bUseOneByOneIndexCheck==true &&
               INDEX_SUCKER_TYPE==1)                                            //Ifor 20200617 : add Use One By One Index Check Function 整合
            {
                FTestSuck.ResetAll();
                iSiteCount=0;
                IndexCheckOneByOne(true, 0, iSiteCount);                        //Ifor 20200617 : add Use One By One Index Check Function 整合
                Task=80000;
                break;
            }

            for(int i=0; i<FTestSuck.iShtRow; i++)
            {
                for(int j=0; j<FTestSuck.iShtCol; j++)
                {
                    if(INDEX_SUCKER_TYPE==1)
                    {
                        if(CosFunction.bInitTestHeadByTestSiteUse)              //JerryYang 20151016 : TestSuck檢查 關Site時就不開真空偵測
                        {
                            if(bTestSiteUse[0][i][j]==true)
                            {
                                fiosetview->bIndexSuck[0][i][j]=true;
                                bIndexCheckNoStopVaccum=true;
                            }
                        }
                        else
                        {
                            fiosetview->bIndexSuck[0][i][j]=true;
                            bIndexCheckNoStopVaccum=true;
                        }
                    }
                    else
                    {
                        if(CosFunction.bInitTestHeadByTestSiteUse)              //JerryYang 20151016 : TestSuck檢查 關Site時就不開真空偵測
                        {
                            if(bTestSiteUse[0][i][j]==true)
                            {
                                FTestSuck.Suck[i][j].On();
                            }
                        }
                        else
                        {
                            FTestSuck.Suck[i][j].On();
                        }
                    }
                }
            }

            DoTestHeadMotorDelay.SetSecAndOn(0.5);
            Task=122100;
            break;
        case 122100:
            if(INDEX_SUCKER_TYPE==1)                                            //jou 2012-05-09 Index Check時,如果中途按暫停,也要繼續把suck()做完,避免負壓掉ic
            {
                W7T1_FIOSET_PISD1();
            }

            if(DoTestHeadMotorDelay.Off())
            {
                Task=122110;
            }
            break;
        case 122110:
            if(INDEX_SUCKER_TYPE==1)                                                                                                                            //jou 2012-05-09 Index Check時,如果中途按暫停,也要繼續把suck()做完,避免負壓掉ic
            {
                W7T1_FIOSET_PISD1();
            }

            iIndexUpPos=GetSocketCheckPos(Prod.TestZ1_Test);                                                                                                    //Steven 20140620 : 整合為Function
            if(CosFunction.bSortingBy2DList==true &&
               LastSet.iTester==_2D_SORT &&
               TestIF_File.bSortingBy2DIDList==true)                                                                                                            //Frank 20221122 : 2DID sorting for ATK
            {
                if(MOT[MTestZ1].Gali_Two_ZAxis_Move(Prod.TestZ1_Safe, iSpeedSlow, "DoTestHeadMotor 12101"))
                {
                    IndexStatus=Z1_Z2_Normal;
                    if(CUSTOMER_CODE==CC_Greatek)                                                                                                               //Wei 20160413
                        DoTestHeadMotorDelay.SetSecAndOn(5);                                                                                                    //Steven 20110908 : 上來後也要Delay一下
                    else
                        DoTestHeadMotorDelay.SetSecAndOn(0.5);                                                                                                  //Steven 20110908 : 上來後也要Delay一下

                   if(ArmSpeed_File[IndexArm].bDevicConfirm &&
                      TotalErrPart!="")                                                                                                                         //kevin 20190629 add error show
                        Task=123;
                    else
                        Task=122;
                }
            }
            else
            {
                if(MOT[MTestZ1].Gali_MotMove(Prod.TestZ1_Test-Prod.TestZ1_Drop_Offset+iIndexUpPos-iIndexArmCheck_SG_Arm1, iSpeedFast))                          //KaiChen 20200826 ：矽格-湖口，要求IndexCheck使用Contact高度不使用Offset
                {
                    if(CUSTOMER_CODE==CC_Greatek)                                                                                                               //Wei 20160413
                        DoTestHeadMotorDelay.SetSecAndOn(5);                                                                                                    //Steven 20110908 : 上來後也要Delay一下
                    else
                        DoTestHeadMotorDelay.SetSecAndOn(0.5);                                                                                                  //Steven 20110908 : 上來後也要Delay一下

                    if(CosFunction.bUseOneByOneIndexCheck==true &&
                       ArmSpeed_File[IndexArm].bDevicConfirm &&
                       TotalErrPart!="")                                                                                                                        //kevin 20190629 add error show
                        Task=123;
                    else if(CosFunction.bUseOneByOneIndexCheck==true &&
                            TotalErrPart!="")                                                                                                                   //KaiChen 20210104:
                        Task=123;
                    else
                        Task=122;
                }
            }
            break;
        case 122:
            if(INDEX_SUCKER_TYPE==1)                                            //jou 2012-01-04 需確認Index suck已經完整做完
            {
                bIndexSuckCheck=false;
                bIndexSuckCheck=W7T1_FIOSET_PISD1();
            }
            else
            {
                bIndexSuckCheck=true;
            }

            if(DoTestHeadMotorDelay.Off() && bIndexSuckCheck==true)
            {
                bIndexCheckNoStopVaccum=false;                                  //jou 2012-05-09 Index Check時,如果中途按暫停,也要繼續把suck()做完,避免負壓掉ic
                flag=false;
                ErrPart=" ";
                for(int i=0; i<FTestSuck.iShtRow; i++)
                {
                    for(int j=0; j<FTestSuck.iShtCol; j++)
                    {
                        flag2=false;
                        W7T1_CHECKVACINIT(FTestSuck, i, j, flag2);
                        if(flag2==true)
                        {
                            flag=true;
                            ErrPart+=IndexSuckName[i+IsNNMode()][j];            //Steven 20230712 : 修正NN mode alarm顯示

                            if(CUSTOMER_CODE==CC_SCS)                           //jou 20170516 (Steven) : SCS要求index check偵測到device時需吹氣
                                FTestSuck.Suck[i][j].Off();
                        }
                    }
                }

                if(flag && LastSet.iRealDummy==REALLY)                          //Steven 20120726 : 有跑IC才檢查Socket
                    Task=123;
                else
                    Task=130;
            }
            break;
        case 123:
            if(MOT[MTestZ1].Gali_Two_ZAxis_Move(Prod.TestZ1_Safe, iSpeedSlow, "DoTestHeadMotor 123"))
            {
                if(CUSTOMER_CODE==CC_SCS)                                       //jou 20170516 (Steven) : SCS要求index check偵測到device時需吹氣
                {
                    for(int i=0; i<FTestSuck.iShtRow; i++)
                    {
                        for(int j=0; j<FTestSuck.iShtCol; j++)
                        {
                            FTestSuck.Suck[i][j].Normal();
                        }
                    }
                }
                Task=124;
            }
            break;
        case 124:
            if(MOT[MTestY1].GalilTwoY_Move(Prod.TestY1_Front, Prod.TestY2_Middle, iSpeedY, "DoTestHeadMotor 124"))
                Task=125;
            break;
        case 125:
            if(IndexAlarmInArmAway()==true)                                     //Steven 20130613 : Index異常時, In Arm要先讓位功能
            {
                bIsTestSitICFallDown=true;                                      //Steven 20130613
                MyDBIProcess("Message", "Piggy Back Fail");                     //JerryYang 20220614 松諭要求新增
                if(CosFunction.bUseOneByOneIndexCheck==true &&
                   ArmSpeed_File[IndexArm].bDevicConfirm)                       //kevin 20190629 add error show
                {
                    ShowErrorMessage("WAR0310", K_RETRY, MTestY1, false, TotalErrPart);
                }
                else if(CosFunction.bUseOneByOneIndexCheck==true &&
                        INDEX_SUCKER_TYPE==1 &&
                        TotalErrPart!="")                                       //KaiChen 20210104:
                {
                    ShowErrorMessage("WAR0310", K_RETRY, MTestY1, false, TotalErrPart);
                }
                else if(IniConfig.bD40IndexICFallDownMustPressFMotorDown)       //Steven 20130604 : Socket殘料要按Z1
                {
                    ShowErrorMessage("WAR0310", K_RETRY, MTestY1, false, ErrPart);
                }
                else
                {
                    ShowMyMessage("Arm1 detect Test Socket has IC error", "Arm 1偵測到Socket有IC殘留!!", "DoTestHeadMotor 125");
                }
                ErrPart="";
                TotalErrPart="";
                Task=1;
            }
            break;
        case 130:
            Task=15000;
            break;
        case 135:
            if(MOT[MTestZ2].Gali_Two_ZAxis_Move(Prod.TestZ1_Safe, iSpeedSlow, "DoTestHeadMotor 135"))
                Task=140;
            break;
        case 140:
            InitWriteAndCheckMotorTorqueTask();
            W7T1_FMAIN_LBARM1TORQUE->Caption="2:Writing";
            Task=14000;
        case 14000:
            if(USE_IO_CHANGE_TOQUE==true)                                       //jou 2012-06-21 Enable index I/O Change Toque
            {
                SW[SwIndexChangeToque1].Off();
                SW[SwIndexChangeToque2].Off();
            }

            #ifdef SOFT_SIMULTE
                ret=1;
            #else
                ret=COM2->iWriteAndCheckMotorTorque(1, Prod.iMaxPreasure);
            #endif

            if(ret==1)
            {
                iToqureCount=0;
                Task=14100;
            }
            else if(ret==2)
            {
                ShowMyMessage("The test head 2 Motor torque set error", "馬達扭力設定錯誤", "DoTestHeadMotor 14000");
                fAllMotorHome=false;
                Task=1;
                return ;
            }
            break;
        case 14100:
            Task=14101;
        case 14101:
            iIndexUpPos=0;

            if(CosFunction.bSortingBy2DList==true &&
               LastSet.iTester==_2D_SORT &&
               TestIF_File.bSortingBy2DIDList==true)                                                                                                            //Frank 20221122 : 2DID sorting for ATK
            {
                if(MOT[MTestZ2].Gali_MotMove(Prod.TestZ1_Safe, iSpeedSlow))
                {
                    IndexStatus=Z1_Z2_Normal;
                    #ifdef SOFT_SIMULTE
                        ShowMainScreenPresure(1);
                        Task=14300;
                    #else
                        if(CUSTOMER_CODE==CC_KYEC_XILINX &&
                           IniConfig.bD01EnableReadTorque &&                                                                                                    //Frank 20171030 (Steven) add Floating Shuttle Read Torque Delay
                           IniConfig.bChangeKitNoHardStop==true &&
                           IniConfig.bRemeberAutoHeight==true)
                        {
                            Task=14102;
                            DoTestHeadMotorDelay.SetSecAndOn(IniConfig.dD01ReadTorqueDelayTime);
                        }
                        else
                        {
                            Task=14110;
                            DoTestHeadMotorDelay.SetSecAndOn(1);
                            bFirstTime=true;
                        }
                    #endif
                }
            }
            else
            {
                if(MOT[MTestZ2].Gali_MotMove(Prod.TestZ2_Test-Prod.TestZ2_Drop_Offset+iIndexUpPos-iIndexArmCheck_SG_Arm2, iSpeedSlow))                          //KaiChen 20200826 ：矽格-湖口，要求IndexCheck使用Contact高度不使用Offset
                {
                    IndexStatus=Z1Up_Z2Down;
                    #ifdef SOFT_SIMULTE
                        ShowMainScreenPresure(1);
                        Task=14300;
                    #else
                        if(CUSTOMER_CODE==CC_KYEC_XILINX &&
                           IniConfig.bD01EnableReadTorque &&                                                                                                    //Frank 20171030 (Steven) add Floating Shuttle Read Torque Delay
                           IniConfig.bChangeKitNoHardStop==true &&
                           IniConfig.bRemeberAutoHeight==true)
                        {
                            Task=14102;
                            DoTestHeadMotorDelay.SetSecAndOn(IniConfig.dD01ReadTorqueDelayTime);
                        }
                        else
                        {
                            Task=14110;
                            DoTestHeadMotorDelay.SetSecAndOn(1);
                            bFirstTime=true;
                        }
                    #endif
                    ReadTorqueDelay.SetSecAndOn(10);                                                                                                            //kevin 20210824 read torque wait alarm time
                }
            }
            break;
        case 14102:                                                             //Frank 20171030 (Steven) add Floating Shuttle Read Torque Delay
            if(DoTestHeadMotorDelay.Off())
            {
                Task=14110;
                DoTestHeadMotorDelay.SetSecAndOn(1);
                bFirstTime=true;
            }
            break;
        case 14110:
            W7T1_FMAIN_LBARM1TORQUE->Caption="2:Reading";

            if(DoTestHeadMotorDelay.Off()==false)
            {
                if(bFirstTime)
                {
                    bFirstTime=false;
                    W7T1_FMAIN_CHKREADTORQUE1->Checked=false;
                    W7T1_FMAIN_CHKREADTORQUE2->Checked=true;
                    W7T1_FMAIN_EDTORUE1->Text="";
                    COM2->InitReadTorueTask();
                    bReadMCU2=true;                                             //kevin 20220225 read MCU DATA
                    return;                                                     //kevin 20210824 clean Tourqe
                }
            }

            if(W7T1_FMAIN_EDTORUE1->Text=="")                                       //jou 2011-11-29防止Read Torue後數值被清掉，還傻傻的在那邊等
            {
                if(ReadTorqueDelay.Off())                                       //kevin 20210824 read torque wait alarm time
                {
                    if(COM2->GetReadTorueTask()==999)
                    {
                        ShowMyMessage("The test head 2 Motor torque read error", "馬達扭力讀取錯誤", "DoTestHeadMotor 14110");
                        fAllMotorHome=false;
                        Task=1;
                    }
                }
                return;
            }
            else
            {
                 ShowMainScreenPresure(1);                                      //kevin 20210413
            }

            if(W7T1_FMAIN_LBARM1TORQUE->Caption=="2:Reading")                       //kevin 20210413
            {
                InitWriteAndCheckMotorTorqueTask();
                Task=140;
                RecordProcess("Arm2 Torque Reading.");
                 return;
            }
            //Rear Arm Only
            if(IniConfig.bD30EnableSiteModeSelect &&
               (TestIF.iShuttleMode==0 ||
               (TestIF.iShuttleMode==1 && TestIF.iShuttle_Sel==1)))             //KEVIN 20150613
            {
                if(IniConfig.bD58UseArm1PickPlaceArm2Test==true &&
                   TestIF_File.bArm1PickPlaceArm2Test==true)                    //Steven 20200604 : Arm1丟 Arm2測的時候, 只要檢查Arm1
                {
                }
                else if(IniConfig.bC08_SocketSensor &&
                        TestIF_File.bEnSocketSensor &&
                        TestIF_File.bSocketDisibleinitialcheck==false)          //JerryYang 20170426 (Steven) 修正Disable socket sensor detect
                {
                    str="check socket sensor down off,";
                    for(int i=0; i<TestIF_File.iSocketCount; i++)
                    {
                        if(IsNNMode()==NN_2Row)
                        {
                            if(i<=1 &&
                               Sen[SThreadPara.iSocketSensor[i]].Enable &&
                               Sen[SThreadPara.iSocketSensor[i]].IsOff())       //kevin 20150429 : Arm 2在下, 檢查遮斷, 要On
                            {
                                flag=true;
                                bIsSocketSensor=true;
                                str+=IntToStr(i+1);
                            }
                        }
                        else
                        {
//                            if(IniConfig.bD58UseArm1PickPlaceArm2Test==true &&        //Jimmychiu 20250809 : Mark for Gigas反應檢測功能失效
//                               TestIF_File.bArm1PickPlaceArm2Test==true)
//                            {
                                if(Sen[SThreadPara.iSocketSensor[i]].Enable &&
                                   Sen[SThreadPara.iSocketSensor[i]].IsOff())   //kevin 20150429 : Arm 2在下, 檢查遮斷, 要On
                                {
                                    flag=true;
                                    bIsSocketSensor=true;
                                    str+=IntToStr(i+1);
                                }
//                            }
//                            else
//                            {
//                                if(TestIF_File.iSensorCheckType[i]==2 &&
//                                   Sen[SThreadPara.iSocketSensor[i]].Enable &&
//                                   Sen[SThreadPara.iSocketSensor[i]].IsOff())   //Arm 2在下, 手臂上沒IC, 只檢查floating sensor必須遮到
//                                {
//                                    flag=true;
//                                    bIsSocketSensor=true;
//                                    str+=IntToStr(i+1);
//                                }
//                            }
                        }
                    }

                    if(flag)
                    {
                        ErrPart=str;                                            //Steven 20220721 : Alarm之前, Index要先讓開
                        Task=14150;
//                        ShowErrorMessage("WAR0322", K_RETRY, MTestZ2, false, str);//kevin 20130504 socket sensor
                        return;                                                 //kevin 20150429確認 socket sensor是否正常
                    }
                }
            }
            TorqueData=atoi(W7T1_FMAIN_EDTORUE1->Text.c_str());
            TorqueData=abs(TorqueData);

            if(IniConfig.bControlTorque)                                        //jou 2013-11-05 Index Control Torque
                iMaxPreasure=DeviceForm.iIndexTorqueMax;
            else
                iMaxPreasure=Prod.iMaxPreasure;

            if(CUSTOMER_CODE==CC_KYEC_XILINX &&
               IniConfig.bD01EnableReadTorque &&                                //Frank 20171030 (Steven) add Floting Shuttle Read Torque Delay
               IniConfig.bChangeKitNoHardStop==true &&
               IniConfig.bRemeberAutoHeight==true)
            {
                if(TorqueData>=(IniConfig.dD01ReadTorque+DeviceForm_File.dZ2Torue))
                {
                    ShowMyMessage("The test head 2, contact force over error", "Index 2 壓力過重錯誤", "DoTestHeadMotor 14110");
                    Task=14111;                                                 //kevin 20130418
                    return;
                }
            }
            else
            {
                if(TorqueData>=iMaxPreasure)
                {
                    if(iToqureCount>10)                                         //jou 981130 double check Torque
                    {
                        iToqureCount=0;
                        ShowMyMessage("The test head 2, contact force over error", "Index 2 壓力過重錯誤", "DoTestHeadMotor 14110");
                        Task=14111;                                             //kevin 20130418
                        return;
                    }
                    else
                    {
                        iToqureCount++;
                        Task=14100;
                        break;
                    }
                }
            }

            if(IniConfig.bControlTorque)                                        //jou 2013-11-05 Index Control Torque
            {
                iIndexArm[0]=atoi(W7T1_FMAIN_EDTORUE0->Text.c_str());
                iIndexArm[1]=atoi(W7T1_FMAIN_EDTORUE1->Text.c_str());
                iIndexArm[2]=abs(iIndexArm[0]-iIndexArm[1]);
                if(TestIF_File.iShuttleMode==0 &&                               //Steven 20240716 : 關arm的時候不檢查兩arm的壓力差
                   iIndexArm[2]>DeviceForm.iIndexTorqueCmp)
                {
                    ShowMyMessage("The test head contact force% difference over error", "Index 壓力%差距過大錯誤", "DoTestHeadMotor 14110");
                }
            }

            ShowMainScreenPresure(1);
            InitWriteAndCheckMotorTorqueTask();

            if(IniConfig.bVTESTFunction==true)                                  //jou 20231102 : VTEST 增加 Torque log
                CheckAndRecodrTorque(1);

            Task=14200;
            break;
        case 14111:                                                             //kevin 20130418
            if(MOT[MTestZ2].Gali_Two_ZAxis_Move(Prod.TestZ2_Safe, iSpeedSlow, "DoTestHeadMotor 14111"))
                Task++;
            break;
        case 14112:
            ErrPart="The test head 2, contact force over error";                //kevin 20130418
            bIsContactforce=true;                                               //kevin 20130418 contact force over 需開們確認

            if(CosFunction.bIndexAreaOnlyCanUseSkip)                            //Steven 20141105 : Index內的所有異常都只能用Skip
                ShowErrorMessage("WAR0321", K_SKIP, MTestZ1, false, ErrPart);
            else
                ShowErrorMessage("WAR0321", K_RETRY, MTestZ2, false, ErrPart);

            Task=135;
            break;
        case 14150:                                                             //Steven 20220721 : Alarm之前, Index要先讓開
            if(MOT[MTestZ2].Gali_Two_ZAxis_Move(Prod.TestZ2_Safe, iSpeedSlow, "DoTestHeadMotor 14150"))
            {
                Task=14151;
            }
            break;
        case 14151:
            if(MOT[MTestY1].GalilTwoY_Move(Prod.TestY1_Front, Prod.TestY2_Rear, iSpeedY))
            {
                Task=14152;
            }
            break;
        case 14152:
            if(IndexAlarmInArmAway()==true)                                     //Steven 20130613 : Index異常時, In Arm要先讓位功能
            {
                ShowErrorMessage("WAR0322", K_RETRY, MTestZ2, false, ErrPart);  //kevin 20130504 socket sensor
                Task=9;
            }
            break;
        case 14200:
            if(USE_IO_CHANGE_TOQUE==true)                                       //jou 2012-06-21 Enable index I/O Change Toque
            {
                SW[SwIndexChangeToque1].Off();
                SW[SwIndexChangeToque2].Off();
            }

            ret=COM2->iWriteAndCheckMotorTorque(1, 300);
            if(ret==1)
            {
                W7T1_FMAIN_CHKREADTORQUE1->Checked=false;
                W7T1_FMAIN_CHKREADTORQUE2->Checked=false;
                Task=14300;
            }
            else if(ret==2)
            {
                ShowMyMessage("Motor torque set error", "馬達扭力設定錯誤", "DoTestHeadMotor 14200");
                fAllMotorHome=false;
                Task=1;
                return ;
            }
            break;
        case 14300:
            if(ArmSpeed_File[IndexArm].bDevicConfirm &&                         //kevin 20190629 回吸檢測一次 4 個 SITE
               INDEX_SUCKER_TYPE==1)                                            //kevin 20190530 add index check
            {
                IndexCheck4Site(true, 1, iSiteCount);
                Task=14301;
            }
            else if(LastSet.bD41TestSocketICCheckSkip==false)                   //jou 2014-08-18 SPIL 關arm 不檢查真空值
            {
                if(IniConfig.bSPILFunction==true)                               //JerryYang 20170328 (Jou) 矽品客戶碼統一用SPILFunction
                {
                    if(TestIF_File.iShuttleMode==0 ||
                       (TestIF_File.iShuttleMode==1 &&
                        TestIF_File.iShuttle_Sel==1))
                        Task=141;
                    else
                        Task=150;
                }
                else
                {
                    if(CosFunction.bUseOneByOneIndexCheck==true &&              //Ifor 20180322 : add Use One By One Index Check
                       INDEX_SUCKER_TYPE==1)
                    {
                        if((DeviceForm.iSocketInitialICCheckPosition==1 &&
                            IniConfig.bTestIcCheckInContact==true) ||
                           (IniConfig.iD41SocketInitialICCheckPosition==1 &&
                            IniConfig.bTestIcCheckInContact==false))            //Above Socket
                        {
                            BTestSuck.ResetAll();
                            iSiteCount=0;
                            IndexCheckOneByOne(true, 1, iSiteCount);            //Ifor 20200617 : add Use One By One Index Check Function 整合
                            Task=60000;
                        }
                        else
                        {
                            Task=141;
                        }
                    }
                    else if(ArmSpeed_File[IndexArm].bDevicConfirm &&            //kevin 20190629 回吸檢測一次 4 個 SITE
                            INDEX_SUCKER_TYPE==1)                               //kevin 20190530 add index check
                    {
                        IndexCheck4Site(true,1,iSiteCount);
                        Task=14305;
                    }
                    else
                    {
                        Task=141;
                    }
                }
            }
            else
            {
                Task=150;
            }

            if(IniConfig.bIndexArm2SupplyLight==true ||                         //jou 2012-10-19 Index Arm 2 供應光源 for CMOS
               TestIF_File.bForEgisTecTest==true     ||                         //Steven 20140922 : Arm2當作指紋測試
               (IniConfig.bD58UseArm1PickPlaceArm2Test==true &&                 //kevin 20150127 Arm1 下壓 arm2 測試
                TestIF_File.bArm1PickPlaceArm2Test==true))                      //Ifor 20200811 Fix: Arm1 Pick Place Arm2Test 需卡兩個條件
            {
                Task=150;
            }
            break;
        case 14301:
            if((DeviceForm.iSocketInitialICCheckPosition==1 &&
                IniConfig.bTestIcCheckInContact==true) ||
               (IniConfig.iD41SocketInitialICCheckPosition==1 &&
                IniConfig.bTestIcCheckInContact==false))                                                                                                        //Above Socket
            {
                iIndexUpPos=0;
                if((DeviceForm.iSocketInitialICCheckPosition==1 &&
                    IniConfig.bTestIcCheckInContact==true) ||                                                                                                   //kevin 20190708 add
                    (IniConfig.iD41SocketInitialICCheckPosition==1 &&
                     IniConfig.bTestIcCheckInContact==false))                                                                                                   //Above Socket
                {
                    iIndexUpPos=GetSocketCheckPos(Prod.TestZ2_Test);
                }

                if(MOT[MTestZ2].Gali_MotMove(Prod.TestZ2_Test-Prod.TestZ2_Drop_Offset+iIndexUpPos-iIndexArmCheck_SG_Arm2, iSpeedSlow))                          //KaiChen 20200826 ：矽格-湖口，要求IndexCheck使用Contact高度不使用Offset
                {
                    IndexStatus=Z1Down_Z2Up;
                    if(ArmSpeed_File[IndexArm].bDevicConfirm &&                                                                                                 //kevin 20190629 回吸檢測一次 4 個 SITE
                       INDEX_SUCKER_TYPE==1)                                                                                                                    //kevin 20190530 add index check
                    {
                        IndexCheck4Site(true,0,iSiteCount);
                        Task=14305;
                    }
                    else
                    {
                        Task=141;
                    }
                }
                break;
            }
            else
            {
                if(IniConfig.bSPILFunction==true)                                                                                                               //JerryYang 20170328 (Jou) 矽品客戶碼統一用SPILFunction
                {
                    if(TestIF_File.iShuttleMode==0 ||                                                                                                           //jou 2014-08-18 SPIL 關arm 不檢查真空值
                       (TestIF_File.iShuttleMode==1  &&
                        TestIF_File.iShuttle_Sel==0))
                        Task=141;
                    else
                        Task=150;
                }
                else
                {
                    Task=14302;
                }
            }
            break;
        case 14302:
            if(ArmSpeed_File[IndexArm].bDevicConfirm &&                         //kevin 20190629 回吸檢測一次 4 個 SITE
               INDEX_SUCKER_TYPE==1)                                            //kevin 20190530 add index check
            {
                IndexCheck4Site(true, 0, iSiteCount);
                Task=14305;
            }
            else
            {
                Task=141;
            }
            break;
        case 14305:                                                             //kevin 20190531 index check 4 Site
            if(IndexCheck4Site(false, 1, iSiteCount))
            {
                iSiteCount++;

                if(iSiteCount<TestSocket.iShtCol/2)                             //JerryYang 20250120 : modify
                    Task=14301;
                else
                    Task=142100;
            }
            break;
        case 141:
            if((IniConfig.bD58UseArm1PickPlaceArm2Test==true &&
                TestIF_File.bArm1PickPlaceArm2Test==true) &&                    //Steven 20150129 : 需要確認Arm2有沒有粘料
                TestIF_File.bCheckArm2Vacuum==false)                            //Ifor 20200811 Fix: Arm1 Pick Place Arm2Test 需卡兩個條件
            {
                Task=150;
                break;
            }

            if(CosFunction.bUseOneByOneIndexCheck==true &&
               INDEX_SUCKER_TYPE==1)                                            //Ifor 20200617 : add Use One By One Index Check Function 整合
            {
                BTestSuck.ResetAll();
                iSiteCount=0;
                IndexCheckOneByOne(true, 1, iSiteCount);                        //Ifor 20200617 : add Use One By One Index Check Function 整合
                Task=80010;
                break;
            }

            for(int i=0; i<BTestSuck.iShtRow; i++)
            {
                for(int j=0; j<BTestSuck.iShtCol; j++)
                {
                    if(INDEX_SUCKER_TYPE==1)
                    {
                        if(CosFunction.bInitTestHeadByTestSiteUse)              //JerryYang 20151016 : TestSuck檢查 關Site時就不開真空偵測
                        {
                            if(bTestSiteUse[1][i][j]==true)
                            {
                                fiosetview->bIndexSuck[1][i][j]=true;
                                bIndexCheckNoStopVaccum=true;
                            }
                        }
                        else
                        {
                            fiosetview->bIndexSuck[1][i][j]=true;
                            bIndexCheckNoStopVaccum=true;
                        }
                    }
                    else
                    {
                        if(CosFunction.bInitTestHeadByTestSiteUse)              //JerryYang 20151016 : TestSuck檢查 關Site時就不開真空偵測
                        {
                            if(bTestSiteUse[1][i][j]==true)
                            {
                                BTestSuck.Suck[i][j].On();
                            }
                        }
                        else
                        {
                            BTestSuck.Suck[i][j].On();
                        }
                    }
                }
            }

            DoTestHeadMotorDelay.SetSecAndOn(0.5);
            Task=142100;
            break;
        case 142100:
            if(INDEX_SUCKER_TYPE==1)                                            //jou 2012-05-09 Index Check時,如果中途按暫停,也要繼續把suck()做完,避免負壓掉ic
            {
                W7T1_FIOSET_PISD2();
            }

            if(DoTestHeadMotorDelay.Off())
                Task=142110;
            break;
        case 142110:
            if(INDEX_SUCKER_TYPE==1)                                                                                                                            //jou 2012-05-09 Index Check時,如果中途按暫停,也要繼續把suck()做完,避免負壓掉ic
            {
                W7T1_FIOSET_PISD2();
            }

            iIndexUpPos=GetSocketCheckPos(Prod.TestZ2_Test);                                                                                                    //Steven 20140620 : 整合為Function
            if(CosFunction.bSortingBy2DList==true &&
               LastSet.iTester==_2D_SORT &&
               TestIF_File.bSortingBy2DIDList==true)                                                                                                            //Frank 20221122 : 2DID sorting for ATK
            {
                if(MOT[MTestZ1].Gali_Two_ZAxis_Move(Prod.TestZ1_Safe, iSpeedSlow, "DoTestHeadMotor 12101"))
                {
                    IndexStatus=Z1_Z2_Normal;
                    if(CUSTOMER_CODE==CC_Greatek)                                                                                                               //Wei 20160413
                        DoTestHeadMotorDelay.SetSecAndOn(5);                                                                                                    //Steven 20110908 : 上來後也要Delay一下
                    else
                        DoTestHeadMotorDelay.SetSecAndOn(0.5);                                                                                                  //Steven 20110908 : 上來後也要Delay一下

                    if(ArmSpeed_File[IndexArm].bDevicConfirm &&
                       TotalErrPart!="")                                                                                                                        //kevin 20190629 add error show
                        Task=143;
                    else
                        Task=142;
                }
            }
            else
            {
                if(MOT[MTestZ2].Gali_MotMove(Prod.TestZ2_Test-Prod.TestZ2_Drop_Offset+iIndexUpPos-iIndexArmCheck_SG_Arm2, iSpeedFast))                          //KaiChen 20200826 ：矽格-湖口，要求IndexCheck使用Contact高度不使用Offset
                {
                    IndexStatus=Z1Up_Z2Down;                                                                                                                    //kevin 20190118 add index z states
                    if(CUSTOMER_CODE==CC_Greatek)                                                                                                               //Wei 20160413
                        DoTestHeadMotorDelay.SetSecAndOn(5);                                                                                                    //Steven 20110908 : 上來後也要Delay一下
                    else
                        DoTestHeadMotorDelay.SetSecAndOn(0.5);                                                                                                  //Steven 20110908 : 上來後也要Delay一下

                    if(CosFunction.bUseOneByOneIndexCheck==true &&
                       ArmSpeed_File[IndexArm].bDevicConfirm &&
                       TotalErrPart!="")                                                                                                                        //kevin 20190629 add error show
                        Task=143;
                    else if(CosFunction.bUseOneByOneIndexCheck==true &&
                            TotalErrPart!="")                                                                                                                   //KaiChen 20210104:
                        Task=143;
                    else
                        Task=142;
                }
            }
            break;
        case 142:
            if(INDEX_SUCKER_TYPE==1)                                            //jou 2012-01-04 需確認Index suck已經完整做完
            {
                bIndexSuckCheck=false;
                bIndexSuckCheck=W7T1_FIOSET_PISD2();
            }
            else
            {
                bIndexSuckCheck=true;
            }

            if(DoTestHeadMotorDelay.Off() && bIndexSuckCheck==true)
            {
                bIndexCheckNoStopVaccum=false;                                  //jou 2012-05-09 Index Check時,如果中途按暫停,也要繼續把suck()做完,避免負壓掉ic
                flag=false;
                ErrPart=" ";
                for(int i=0; i<BTestSuck.iShtRow; i++)
                {
                    for(int j=0; j<BTestSuck.iShtCol; j++)
                    {
                        flag2=false;
                        W7T1_CHECKVACINIT(BTestSuck, i, j, flag2);
                        if(flag2==true)
                        {
                            flag=true;
                            ErrPart+=IndexSuckName[i][j];

                            if(CUSTOMER_CODE==CC_SCS)                           //jou 20170516 (Steven) : SCS要求index check偵測到device時需吹氣
                                BTestSuck.Suck[i][j].Off();
                        }
                    }
                }

                if(flag && LastSet.iRealDummy==REALLY)                          //Steven 20120726 : 有跑IC才檢查Socket
                    Task=143;
                else
                    Task=150;
            }
            break;
        case 143:
            if(MOT[MTestZ1].Gali_Two_ZAxis_Move(Prod.TestZ1_Safe, iSpeedSlow, "DoTestHeadMotor 143"))
            {
                IndexStatus=Z1_Z2_Normal;                                       //kevin 20190118 add index z states
                if(CUSTOMER_CODE==CC_SCS)                                       //jou 20170516 (Steven) : SCS要求index check偵測到device時需吹氣
                {
                    for(int i=0; i<BTestSuck.iShtRow; i++)
                    {
                        for(int j=0; j<BTestSuck.iShtCol; j++)
                        {
                            BTestSuck.Suck[i][j].Normal();
                        }
                    }
                }
                Task=144;
            }
            break;
        case 144:
            if(MOT[MTestY1].GalilTwoY_Move(Prod.TestY1_Front, Prod.TestY2_Middle, iSpeedY, "DoTestHeadMotor 144"))
                Task=145;
            break;
        case 145:
            if(IndexAlarmInArmAway()==true)                                     //Steven 20130613 : Index異常時, In Arm要先讓位功能
            {
                bIsTestSitICFallDown=true;                                      //Steven 20130613
                if(CosFunction.bUseOneByOneIndexCheck==true &&
                   ArmSpeed_File[IndexArm].bDevicConfirm)                       //kevin 20190629 add error show
                {
                    ShowErrorMessage("WAR0310", K_RETRY, MTestY1, false, TotalErrPart);
                }
                else if(CosFunction.bUseOneByOneIndexCheck==true &&
                        INDEX_SUCKER_TYPE==1 &&
                        TotalErrPart!="")                                       //KaiChen 20210104:
                {
                    ShowErrorMessage("WAR0310", K_RETRY, MTestY2, false, TotalErrPart);
                }
                else if(IniConfig.bD40IndexICFallDownMustPressFMotorDown)       //Steven 20130604 : Socket殘料要按Z1
                {
                    ShowErrorMessage("WAR0310", K_RETRY, MTestY2, false, ErrPart);
                }
                else
                {
                    ShowMyMessage("Arm2 detect Test Socket has IC error", "Arm 2偵測到Socket有IC殘留!!", "DoTestHeadMotor 145");
                }
                ErrPart="";

                Task=1;
            }
            break;
        case 150:
            if(MOT[MTestZ1].Gali_Two_ZAxis_Move(Prod.TestZ1_Safe, iSpeedSlow, "DoTestHeadMotor 150"))
            {
                 IndexStatus=Z1_Z2_Normal;                                                                                                                      //kevin 20190118 add index states

                if(IniConfig.bUseAutoSiteMapping &&                                                                                                             //jou 2011-03-24 start : Auto Site Mapping
                   (CosFunction.bUSEJCETSiteMapMode==false ||                                                                                                   //jou 2016-10-28 JCET 要求Site Mapping 必須測試到pass bin才能通過
                    CosFunction.bAutoSiteMappingSetOpenBIN==true))                                                                                              //jou 20230216 : 修正 bAutoSiteMappingSetOpenBIN 失效
                {
                    if(LastSet.iRunStartMode==rsmAutoSiteMap && bSiteMappingCHKOK==false)
                    {
                        if(CosFunction.bAutoSiteMappingSetOpenBIN==true)                                                                                        //jou 20200928 : Auto Site Mapping Set Open BIN
                        {
                            bGetOpenBin=false;
                            Prod.iOpenBin=TestIF_File.iOpenBin;
                            W7T1_FMAIN_SETOPENBIN();
                            Task=1500;
                        }
                        else
                        {
                            flag=true;
                            for(int i=0; i<BTestSuck.iShtRow; i++)
                            {
                                for(int j=0; j<BTestSuck.iShtCol; j++)
                                {
                                    if(flag==true ||
                                       IniConfig.bI21ASMNeedCheckEachSiteOpen)                                                                                  //Steven 20120726 : AutoSiteMapping, 當確認Open Bin時,同時也要檢查是不是所有Dut都Open
                                    {
                                        if(IsNNMode()==NN_1Row)
                                        {
                                            if(LastSet.bUseTestSocket[0][i][j]==true)
                                            {
                                                if(i<1)
                                                {
                                                    BTestSuck.SetItemData(i, j, HAS_IC);
                                                    BTestSuck.cDeviceInf[i][j].sprintf("AutoSiteMap%02d%02d", i+1, j+1);                                        //Steven 20190313 : Add dummy 2DID for Auto Site Map
                                                }
                                                else
                                                {
                                                    FTestSuck.SetItemData(i-1, j, HAS_IC);
                                                    FTestSuck.cDeviceInf[i][j].sprintf("AutoSiteMap%02d%02d", i+1, j+1);                                        //Steven 20190313 : Add dummy 2DID for Auto Site Map
                                                }
                                                flag=false;
                                            }
                                        }
                                        else if(IsNNMode()==NN_2Row)
                                        {
                                            if(LastSet.bUseTestSocket[0][i][j]==true)
                                            {
                                                if(i<2)
                                                {
                                                    BTestSuck.SetItemData(i, j, HAS_IC);
                                                    BTestSuck.cDeviceInf[i][j].sprintf("AutoSiteMap%02d%02d", i+1, j+1);                                        //Steven 20190313 : Add dummy 2DID for Auto Site Map
                                                }
                                                else
                                                {
                                                    FTestSuck.SetItemData(i-2, j, HAS_IC);
                                                    FTestSuck.cDeviceInf[i][j].sprintf("AutoSiteMap%02d%02d", i+1, j+1);                                        //Steven 20190313 : Add dummy 2DID for Auto Site Map
                                                }
                                                flag=false;
                                            }
                                        }
                                        else
                                        {
                                            if(LastSet.bUseTestSocket[0][i][j]==true ||                                                                         //ChungHung 20130910 alter for SCK can close site by Index
                                               LastSet.bUseTestSocket[1][i][j]==true)
                                            {
                                                if(TestIF_File.iShuttleMode==1 &&                                                                               //Ifor 20171005 (Steven) : add 避免Auto Site mapping 關ARM1 導致Hangup
                                                   TestIF_File.iShuttle_Sel==1)
                                                {
                                                    BTestSuck.SetItemData(i, j, HAS_IC);
                                                    BTestSuck.cDeviceInf[i][j].sprintf("AutoSiteMap%02d%02d", i+1, j+1);                                        //Steven 20190313 : Add dummy 2DID for Auto Site Map
                                                }
                                                else
                                                {
                                                    FTestSuck.SetItemData(i, j, HAS_IC);
                                                    FTestSuck.cDeviceInf[i][j].sprintf("AutoSiteMap%02d%02d", i+1, j+1);                                        //Steven 20190313 : Add dummy 2DID for Auto Site Map
                                                }
                                                flag=false;
                                            }
                                        }
                                    }
                                }
                            }

                            if(IsNNMode()==NN_2Row)
                            {
                                InitTestSuckTestIC_TwoArm32Site_Task();
                                Task=170;
                            }
                            else
                            {
                                if(TestIF_File.iShuttleMode==1 &&
                                   TestIF_File.iShuttle_Sel==1)                                                                                                 //jou 20200827 : 修正auto site mapping 只開arm2異常
                                    InitBTestSuckTestICTask();
                                else
                                    InitFTestSuckTestICTask();
                                Task=160;
                            }

                            bGetOpenBin=true;
                            Prod.iOpenBin=0;                                                                                                                    //kevin 20160513
                        }
                        RecordProcess("Auto Site Map Start");                                                                                                   //kevin 20160125
                        break;
                    }
                    else
                    {
                        Task=1500;
                    }
                }
                else
                {
                    Task=1500;
                }
            }
            break;
        case 160:
            if(TestIF_File.iShuttleMode==1 &&
               TestIF_File.iShuttle_Sel==1)                                     //kevin 20160414 add 只使用ARM2 Auto SiteMap
            {
                 if(DoBTestSuckTestIC())
                {
                    for(int i=0; i<BTestSuck.iShtRow; i++)
                    {
                        for(int j=0; j<BTestSuck.iShtCol; j++)
                        {
                            BTestSuck.Item[i][j]=NULL_IC;
                        }
                    }
                    Task=1500;
                    bGetOpenBin=false;
                }
            }
            else
            {
                if(DoFTestSuckTestIC())
                {
                    for(int i=0; i<BTestSuck.iShtRow; i++)
                    {
                        for(int j=0; j<BTestSuck.iShtCol; j++)
                        {
                            FTestSuck.Item[i][j]=NULL_IC;
                        }
                    }
                    Task=1500;
                    bGetOpenBin=false;
                }
            }
            break;
        case 170:                                                               //Steven 20140512: For HT-9047
            if(DoTestSuckTestIC_TwoArm32Site())
            {
                FTestSuck.SetAllToNullIC();
                BTestSuck.SetAllToNullIC();
                Task=1500;
                bGetOpenBin=false;
            }
            break;
        case 1500:
            if(bUseTwoArm32Site==true)
                Task=1650;
            else
                Task=1550;
            break;
        case 1530:                                                              //kevin 20131112
            if(MOT[MTestY1].GalilTwoY_Move(Prod.TestY1_Front, Prod.TestY2_Middle, iSpeedY, "DoTestHeadMotor 1530"))
            {
                if(LastSet.iTemperature==Tempture_Hot)
                {
                    if(DeviceForm.bShuttleWaitingOutSiteChamber)
                    {
                        MOT[MInShuttle1].fCanMoveM=false;
                        MOT[MInShuttle2].fCanMoveM=false;
                        fFrontNeedTest=true;
                    }

                    if(CosFunction.bInitialStartDelayCount_Init)                //JerryYang 20250120 : modify
                    {
                    }
                    else
                    {
                        iInitStartDelayTimeCT=0;                                //jou 2012-11-30 高溫動作下希望增加顆數記數,在前幾顆下壓到Socket後,都要等待Delay time
                    }
                    iInitStartDelayTimeDetCount=0;                              //kevin 20161214 (Steven) 等待Delay time 加減時間
                    Task=1535;
                }
                else
                {
                    if(DeviceForm.bShuttleWaitingOutSiteChamber)
                    {
                        MOT[MInShuttle1].fCanMoveM=false;
                        MOT[MInShuttle2].fCanMoveM=false;
                        fFrontNeedTest=true;
                    }
                    Task=1535;
                }
            }
            break;
        case 1535:                                                              //kevin 20131112 add
            if(bOneTimeFlag)
            {
                bOneTimeFlag=false;
                iBackUpZ1DownPosition=Prod.TestZ1_Test-Prod.TestZ1_Drop_Offset;
            }

            if(MOT[MTestZ2].Gali_MotMove(Prod.TestZ2_Test-Prod.TestZ2_Drop_Offset, iSpeedSlow))
            {
                bOneTimeFlag=true;
                IndexStatus=Z1Up_Z2Down;
                bRearHeadICFallDown=false;
                InitTestYTask();
                bCanNotDisableOneCycle=false;
                W7T1_FMAIN_CHKREADTORQUE1->Checked=false;
                W7T1_FMAIN_CHKREADTORQUE2->Checked=false;
                if(bInitialStartIndexCheckDone==false)                          //Sam 20221214 : 當機台 Initail Start 時需要先做 Index Check
                    bInitialStartIndexCheckDone=true;
                if(LastSet.iTemperature==Tempture_Hot)
                {
                    if(bOneTimeWait==false || iOneCycle)
                    {
                        if(DeviceForm.bSuckShuttleDeviceAfterTested && DeviceForm.bShuttleWaitingOutSiteChamber)
                            Task=1700;                                          //if tested half do home，need check arm is has device
                        else
                            Task=1720;

                        bInitialSackTime=true;                                  //kevin 20131112 第一次吸取ic等待時間
                        DoTestHeadMotorDelay2.SetSecAndOn(1);                   //kevin 20131112 不等待
                        if(CosFunction.bInitialStartDelayCount_Init)            //JerryYang 20250120 : modify
                        {
                        }
                        else
                        {
                            iInitStartDelayTimeCT=0;                            //jou 2012-11-30 高溫動作下希望增加顆數記數,在前幾顆下壓到Socket後,都要等待Delay time
                        }
                        iInitStartDelayTimeDetCount=0;                          //kevin 20161214 (Steven) 等待Delay time 加減時間
                    }
                }
                else
                {
                    Task=600;                                                   // 到此為止 必須 Z1 Down Z2 Up;
                }
            }
            break;
        case 1550:
            if(CosFunction.bSortingBy2DList==true &&
               LastSet.iTester==_2D_SORT &&
               TestIF_File.bSortingBy2DIDList==true)                            //Frank 20221122 : 2DID sorting for ATK
            {
                if(MOT[MTestY1].GalilTwoY_Move(Prod.TestY1_Front, Prod.TestY2_Rear, iSpeedY, "DoTestHeadMotor 1550"))
                {
                    if(bInitialStartIndexCheckDone==false)                      //Sam 20221214 : 當機台 Initail Start 時需要先做 Index Check
                        bInitialStartIndexCheckDone=true;
                    if(LastSet.iTemperature==Tempture_Hot)
                    {
                        if(Temperature.bZ2DownSocket)                           //Steven 20140827 : 第一次吸取ic等待時間
                        {
                            if(DeviceForm.bShuttleWaitingOutSiteChamber)
                            {
                                MOT[MInShuttle1].fCanMoveM=false;
                                MOT[MInShuttle2].fCanMoveM=false;
                                fFrontNeedTest=true;
                            }

                            if(CosFunction.bInitialStartDelayCount_Init)        //JerryYang 20250120 : modify
                            {
                            }
                            else
                            {
                                iInitStartDelayTimeCT=0;                        //jou 2012-11-30 高溫動作下希望增加顆數記數,在前幾顆下壓到Socket後,都要等待Delay time
                            }
                            iInitStartDelayTimeDetCount=0;                      //kevin 20161214 (Steven) 等待Delay time 加減時間
                            Task=1600;
                        }
                        else if(bOneTimeWait==false || iOneCycle)               //kevin 20131112 tem
                        {
                            if(DeviceForm.bShuttleWaitingOutSiteChamber)
                            {
                                MOT[MInShuttle1].fCanMoveM=false;
                                MOT[MInShuttle2].fCanMoveM=false;
                                fFrontNeedTest=true;
                            }
                            iInitStartDelayTimeCT=0;                            //jou 2012-11-30 高溫動作下希望增加顆數記數,在前幾顆下壓到Socket後,都要等待Delay time
                            iInitStartDelayTimeDetCount=0;                      //kevin 20161214 (Steven) 等待Delay time 加減時間
                            Task=1600;
                        }
                    }
                    else
                    {
                        if(DeviceForm.bShuttleWaitingOutSiteChamber)
                        {
                            MOT[MInShuttle1].fCanMoveM=false;
                            MOT[MInShuttle2].fCanMoveM=false;
                            fFrontNeedTest=true;
                        }
                        Task=1600;
                    }
                }
            }
            else
            {
                if(MOT[MTestY1].GalilTwoY_Move(Prod.TestY1_Middle, Prod.TestY2_Rear, iSpeedY, "DoTestHeadMotor 1550"))
                {
                    if(bInitialStartIndexCheckDone==false)                      //Sam 20221214 : 當機台 Initail Start 時需要先做 Index Check
                        bInitialStartIndexCheckDone=true;

                    if(LastSet.iTemperature==Tempture_Hot)
                    {
                        if(Temperature.bZ2DownSocket)                           //Steven 20140827 : 第一次吸取ic等待時間
                        {
                            if(DeviceForm.bShuttleWaitingOutSiteChamber)
                            {
                                MOT[MInShuttle1].fCanMoveM=false;
                                MOT[MInShuttle2].fCanMoveM=false;
                                fFrontNeedTest=true;
                            }
                            iInitStartDelayTimeCT=0;                            //jou 2012-11-30 高溫動作下希望增加顆數記數,在前幾顆下壓到Socket後,都要等待Delay time
                            iInitStartDelayTimeDetCount=0;                      //kevin 20161214 (Steven) 等待Delay time 加減時間
                            Task=1600;
                        }
                        else if(bOneTimeWait==false || iOneCycle)               //kevin 20131112 tem
                        {
                            if(DeviceForm.bShuttleWaitingOutSiteChamber)
                            {
                                MOT[MInShuttle1].fCanMoveM=false;
                                MOT[MInShuttle2].fCanMoveM=false;
                                fFrontNeedTest=true;
                            }
                            iInitStartDelayTimeCT=0;                            //jou 2012-11-30 高溫動作下希望增加顆數記數,在前幾顆下壓到Socket後,都要等待Delay time
                            iInitStartDelayTimeDetCount=0;                      //kevin 20161214 (Steven) 等待Delay time 加減時間
                            Task=1600;
                        }
                    }
                    else
                    {
                        if(DeviceForm.bShuttleWaitingOutSiteChamber)
                        {
                            MOT[MInShuttle1].fCanMoveM=false;
                            MOT[MInShuttle2].fCanMoveM=false;
                            fFrontNeedTest=true;
                        }
                        Task=1600;
                    }
                }
            }
            break;
        case 1600:
            if(bOneTimeFlag)
            {
                bOneTimeFlag=false;
                iBackUpZ1DownPosition=Prod.TestZ1_Test-Prod.TestZ1_Drop_Offset;
            }

            if(CosFunction.bSortingBy2DList==true &&
               LastSet.iTester==_2D_SORT &&
               TestIF_File.bSortingBy2DIDList==true)                                                                    //Frank 20221122 : 2DID sorting for ATK
            {
                if(MOT[MTestZ1].Gali_Two_ZAxis_Move(Prod.TestZ1_Safe, iSpeedSlow, "DoTestHeadMotor 1600"))
                {
                    bOneTimeFlag=true;
                    IndexStatus=Z1_Z2_Normal;
                    bRearHeadICFallDown=false;
                    InitTestYTask();
                    bCanNotDisableOneCycle=false;
                    W7T1_FMAIN_CHKREADTORQUE1->Checked=false;
                    W7T1_FMAIN_CHKREADTORQUE2->Checked=false;
                    if(bInitialStartIndexCheckDone==false)                                                              //Sam 20221214 : 當機台 Initail Start 時需要先做 Index Check
                        bInitialStartIndexCheckDone=true;
                    if(LastSet.iTemperature==Tempture_Hot)
                    {
                        if(bOneTimeWait==false || iOneCycle)
                        {
                            if(DeviceForm.bSuckShuttleDeviceAfterTested && DeviceForm.bShuttleWaitingOutSiteChamber)
                                Task=1700;                                                                              //if tested half do home，need check arm is has device
                            else
                                Task=1720;

                            if(Temperature.iInitialStart1Time!=0)
                                bInitialSackTime=true;                                                                  //kevin 20131112 第一次吸取ic等待時間
                            DoTestHeadMotorDelay2.SetSecAndOn(1);                                                       //kevin 20131112 不等待
                            if(CosFunction.bInitialStartDelayCount_Init)                                                //JerryYang 20250120 : modify
                            {
                            }
                            else
                            {
                                iInitStartDelayTimeCT=0;                                                                //jou 2012-11-30 高溫動作下希望增加顆數記數,在前幾顆下壓到Socket後,都要等待Delay time
                            }

                            iInitStartDelayTimeDetCount=0;                                                              //kevin 20161214 (Steven) 等待Delay time 加減時間
                        }
                    }
                    else
                    {
                        Task=600;                                                                                       // 到此為止 必須 Z1 Down Z2 Up;
                    }
                }
            }
            else
            {
                if(MOT[MTestZ1].Gali_MotMove(Prod.TestZ1_Test-Prod.TestZ1_Drop_Offset, iSpeedSlow))                     //Marc
                {
                    bOneTimeFlag=true;
                    IndexStatus=Z1Down_Z2Up;
                    bRearHeadICFallDown=false;
                    InitTestYTask();
                    bCanNotDisableOneCycle=false;
                    W7T1_FMAIN_CHKREADTORQUE1->Checked=false;
                    W7T1_FMAIN_CHKREADTORQUE2->Checked=false;
                    if(bInitialStartIndexCheckDone==false)                                                              //Sam 20221214 : 當機台 Initail Start 時需要先做 Index Check
                        bInitialStartIndexCheckDone=true;
                    if(LastSet.iTemperature==Tempture_Hot)
                    {
                        if(bOneTimeWait==false || iOneCycle)
                        {
                            if(DeviceForm.bSuckShuttleDeviceAfterTested && DeviceForm.bShuttleWaitingOutSiteChamber)
                                Task=1700;                                                                              //if tested half do home，need check arm is has device
                            else
                                Task=1720;

                            if(Temperature.iInitialStart1Time!=0)
                                bInitialSackTime=true;                                                                  //kevin 20131112 第一次吸取ic等待時間
                            DoTestHeadMotorDelay2.SetSecAndOn(1);                                                       //kevin 20131112 不等待
                            iInitStartDelayTimeCT=0;                                                                    //jou 2012-11-30 高溫動作下希望增加顆數記數,在前幾顆下壓到Socket後,都要等待Delay time
                            iInitStartDelayTimeDetCount=0;                                                              //kevin 20161214 (Steven) 等待Delay time 加減時間
                        }
                    }
                    else
                    {
                        Task=600;                                                                                       // 到此為止 必須 Z1 Down Z2 Up;
                    }
                }
            }
            break;
        case 1650:                                                              //2013-01-15    Dell    Add nn Mode
            if(MOT[MTestY1].GalilTwoY_Move(Prod.TestY1_Front, Prod.TestY2_Rear, iSpeedY, "DoTestHeadMotor 1650"))
            {
                Task=1660;
            }
            break;
        case 1660:
            if(bOneTimeFlag)
            {
                bOneTimeFlag=false;
                iBackUpZ1DownPosition=Prod.TestZ1_Test-Prod.TestZ1_Drop_Offset;
                iBackUpZ2DownPosition=Prod.TestZ2_Test-Prod.TestZ2_Drop_Offset;
            }

            if(DeviceForm.ContactMode==TMove ||
               DeviceForm.ContactMode==TMoveDrop ||                             //jou 2012-02-03 新增T Move Drop
               DeviceForm.ContactMode==TMoveDropSlowContact ||
               DeviceForm.ContactMode==TMoveSlowContact)                        //Steven 20160130 : TMove Soft contact
            {
                TMode=true;
            }
            else
            {
                if(IniConfig.bIndexPickupErrStop==true &&                       //jou 2012-02-29 index pick up error,index arm move to center & alarm
                   bIndexArm2PickupErrStop==true)
                {
                    TMode=true;
                }
                else
                {
                    TMode=false;
                }
            }

            if(bintered1==true)                                                 //Isaac 20200922 : 紀錄indexArmY encoder值和command值
            {
                bintered1=false;
                RecordIndexPosition(1, 1);                                      //Isaac 20200922 : 紀錄indexArmY encoder值和command值Arm1/Socket
                EncoderTeachingMaxMinCount(1);                                  //Isaac 20201012 : 每次完成動作，比較紀錄Encoder和Teaching點的差值
            }

            #ifdef INDEX_PROTECT_TMOVE
            if(bOverRangeDoTMode==true && bTriger4Indexhome==false)             //Isaac 20201012 : index Y超過範圍，做一次Tmode
            {
                bTriger4Indexhome=true;
                bOverRange4Indexhome=true;
                TrigerIndexAxisHome();                                          //Isaac 20201012 : index Y超過範圍，做一次Tmode，初始化，開始自動校正
                return;
            }
            #endif

            if(MOT[MTestY1].Gali_nnMode_Z1Z2_Down(iSpeedY, TMode))
            {
                Task=1670;
            }
            break;
        case 1670:
            if(bInitialStartIndexCheckDone==false)                              //Sam 20221214 : 當機台 Initail Start 時需要先做 Index Check
                bInitialStartIndexCheckDone=true;
            if(LastSet.iTemperature==Tempture_Hot)
            {
                if(bOneTimeWait==false || iOneCycle)
                {
                    if(DeviceForm.bShuttleWaitingOutSiteChamber)
                    {
                        MOT[MInShuttle1].fCanMoveM=false;
                        MOT[MInShuttle2].fCanMoveM=false;
                        fFrontNeedTest=true;
                    }
                    Task=1680;
                }
            }
            else
            {
                if(DeviceForm.bShuttleWaitingOutSiteChamber)
                {
                    MOT[MInShuttle1].fCanMoveM=false;
                    MOT[MInShuttle2].fCanMoveM=false;
                    fFrontNeedTest=true;
                }
                Task=1680;
            }

            if(Prod.bUseSocketHeating==true && bUseSocketHeating_Wait==false)   //Ztex 2024.09.07 Add Use Socket Heating
            {
                iInitStartDelayTimeCT=0;                                        //Ztex 2024.09.06 Add 4x4 Mode Clear iInitStartDelayTimeCT
                iInitStartDelayTimeDetCount=0;                                  //Ztex 2024.09.06 Add 4x4 Mode Clear iInitStartDelayTimeCT
                DoUseSocketHeating.SetSecAndOn(Prod.iUseSocketHeating);
                DoUseSocketHeating.LatchCycleTime(true);
                bUseSocketHeating_Wait=true;
                Task=1675;
            }
            break;
        case 1675:                                                              //Ztex 2024.09.07 Add Use Socket Heating
            iUseSocketHeating_Time=DoUseSocketHeating.LatchCycleTime()/1000;
            W7T1_FMAIN_LABUSER->Caption=AnsiString(iUseSocketHeating_Time);
            if(DoUseSocketHeating.Off()==true)
            {
                W7T1_FMAIN_LABUSER->Caption="User";
                bDoWhenPressStopOverUseInitialDelay=false;
                Task=1680;
            }
            break;
        case 1680:
            bOneTimeFlag=true;
            IndexStatus=Z1_Z2_Down;
            bRearHeadICFallDown=false;
            InitTestYTask();
            bCanNotDisableOneCycle=false;
            W7T1_FMAIN_CHKREADTORQUE1->Checked=false;
            W7T1_FMAIN_CHKREADTORQUE2->Checked=false;
            if(bInitialStartIndexCheckDone==false)                              //Sam 20221214 : 當機台 Initail Start 時需要先做 Index Check
                bInitialStartIndexCheckDone=true;
            if(LastSet.iTemperature==Tempture_Hot)
            {
                if(bOneTimeWait==false || iOneCycle)
                {
                    if(DeviceForm.bSuckShuttleDeviceAfterTested && DeviceForm.bShuttleWaitingOutSiteChamber)
                        Task=1700;                                              //if tested half do home，need check arm is has device
                    else
                        Task=1720;
                    if(Temperature.iInitialStart1Time!=0)
                        bInitialSackTime=true;                                  //kevin 20131112 第一次吸取ic等待時間
                    DoTestHeadMotorDelay2.SetSecAndOn(1);                       //kevin 20131112 不等待
                }
            }
            else
            {
                Task=600;                                                       // 到此為止 必須 Z1 Z2 Up ;
            }
            break;
        case 1700:
            for(int i=0; i<FTestSuck.iShtRow; i++)
            {
                for(int j=0; j<FTestSuck.iShtCol; j++)
                {
                    FTestSuck.Suck[i][j].On();
                    if(INDEX_SUCKER_TYPE==1)                                    //Steven 20111202
                    {
                        fiosetview->bIndexSuck[0][i][j]=true;
                    }
                }
            }

            DoTestHeadMotorDelay.SetMSAndOn(150);
            Task=1710;
            break;
        case 1710:
            if(DoTestHeadMotorDelay.Off())
            {
                for(int i=0; i<FTestSuck.iShtRow; i++)
                {
                    for(int j=0; j<FTestSuck.iShtCol; j++)
                    {
                        W7T1_CHECKVACINIT(FTestSuck, i, j, flag);
                    }
                }
                Task=1720;
            }
            break;
        case 1720:
            if(DoTestHeadMotorDelay2.Off())
            {
                if(DeviceForm.bShuttleWaitingOutSiteChamber)
                    MOT[MInShuttle1].fCanMoveM=true;

                Task=600;
            }
            break;
        case 600:
            DoTestY();
            break;
        case 15000:
            if(MOT[MTestZ1].Gali_Two_ZAxis_Move(Prod.TestZ1_Safe, iSpeedSlow, "DoTestHeadMotor 15000"))
            {
                IndexStatus=Z1_Z2_Normal;                                       //kevin 20190118 add
                Task=15100;
            }
            break;
        case 15100:
            if(CosFunction.bSortingBy2DList==true &&
               LastSet.iTester==_2D_SORT &&
               TestIF_File.bSortingBy2DIDList==true)                            //Frank 20221122 : 2DID sorting for ATK
            {
                Task=140;
            }
            else
            {
                if(MOT[MTestY1].GalilTwoY_Move(Prod.TestY1_Front, Prod.TestY2_Middle, iSpeedY, "DoTestHeadMotor 15100"))
                    Task=140;
            }
            break;
        case 20000:
            Task=20100;
            break;
        case 20100:
            if(MOT[MTestZ1].Gali_Two_ZAxis_Move(Prod.TestZ1_Safe, iSpeedSlow, "DoTestHeadMotor 20100"))
                Task=20200;
            break;
        case 20200:
            if(FrontTestHeadHasIC())
            {
                Task=20300;
            }
            else if(RearTestHeadHasIC())
            {
                Task=21000;
            }
            else
            {
                bPlaceToShuttleFirst=false;
                Task=20210;
                startTick=MyTickCount();
                endTick=MyTickCount();
            }
            break;
        case 20210:
            nowTick=MyTickCount();
            if((nowTick-endTick)>1000)
            {
                startTick=MyTickCount();
                endTick=MyTickCount();
            }
            else
            {
                endTick=MyTickCount();
                if((endTick-startTick)>2000)                                    //Steven 20190304 : 20000 --> 2000
                {
                    Task=1;
                }
            }

            if(ShuttleHasIC()==false &&
               MOT[MInShuttle1].fCanMoveR==true &&
               MOT[MInShuttle2].fCanMoveR==true)
            {
                SetInitialICCheck();
                Task=1;
            }
            break;
        case 20300:
            if(MOT[MTestY1].GalilTwoY_Move(Prod.TestY1_Front, Prod.TestY2_Middle, iSpeedY, "DoTestHeadMotor 20300"))
            {
                Task=20400;
                iFrontTestDestroyICTask=1;
            }
            break;
        case 20400:
            sp=GetIndexZSpeed(1);                                               //Steven 20160524 : Index Z軸速度整合為Function
            if(MOT[MTestZ2].Gali_MotMove(Prod.TestZ2_Safe, sp))
                Task=20500;
            break;
        case 20500:
            if(DoFrontTestDestroyIC(false))
                Task=20100;
            break;
        case 21000:
            if(MOT[MTestY1].GalilTwoY_Move(Prod.TestY1_Middle, Prod.TestY2_Rear, iSpeedY, "DoTestHeadMotor 21000"))
            {
                iRearTestDestroyICTask=1;
                Task=21400;
            }
            break;
        case 21400:
            sp=GetIndexZSpeed(0);                                               //Steven 20160524 : Index Z軸速度整合為Function
            if(MOT[MTestZ1].Gali_MotMove(Prod.TestZ1_Safe,sp))
                Task=21500;
            break;
       case 21500:
            if(DoRearTestDestroyIC(false))
                Task=20100;
            break;
       case 30000:
            if(IndexEveryTimeCheckEP())                                                                                                                         //jou 2011-04-28 start : Index每一次都確認EP是否有充飽氣。
            {
                Task=10;
            }
            break;
        case iCASE_REAL_CCD2:                                                                                                                                   //Steven 20120222 Start: Index Check時,如果有IC,可以Skip重作ROI或是Retry再檢查一次, 以下整段換位置
            if(MOT[MTestZ1].Gali_Two_ZAxis_Move(Prod.TestZ1_Safe, iSpeedSlow, "DoTestHeadMotor iCASE_REAL_CCD2"))
            {
                Task=iCASE_REAL_CCD3;
            }
            break;
        case iCASE_REAL_CCD3:
            if(MOT[MTestY1].GalilTwoY_Move(Prod.TestY1_Front, Prod.TestY2_Rear, iSpeedY, "DoTestHeadMotor iCASE_REAL_CCD3"))
            {
                if((IniConfig.bHaveRTCCheckSiteMap &&
                    IniConfig.bD35RTCCheckSiteMap) ||                                                                                                           //Steven 20140513
                   (CosFunction.bRTCAutoModelVerify==true &&
                    IniConfig.bD36EnableRTCAutoModelVerify==true))                                                                                              //JerryYang 20230324 : 有開Auto verify要傳map給RTC
                {
                    Task=40310;
                }
                else
                {
                    Task=40330;
                }
            }
            break;
        case 40310:
            if(SendSiteMapToRTC(true, 0)!=-1)                                                                                                                   //jou 2014-05-06 add RTC site map
            {
                bDoFRTCAutoModelVerify=false;
                bDoBRTCAutoModelVerify=false;
                DoTestHeadMotorDelay.SetSecAndOn(10);
                Task=40320;
            }
            else
            {
                Task=40330;
            }
            break;
        case 40320:
            if(COM2->bRealTimeCom_ReceiveOK[COM2->rtSiteMap])
            {
                Task=40330;
            }
            else if(DoTestHeadMotorDelay.Off())
            {
                if(COM2->OpenRTCComPortAgain())                                                                                                                 //Steven 20161206 (wei) : Add if
                    ShowMyMessage("RTC Site Map Time out");
                COM2->DoReleaseAndInspEnd();
                Task=1;
            }
            break;
        case 40330:
            if(bTriggerRTC_AutoSTD)                                                                                                                             //JerryYang 20240829 : SPIL訓永 要求手動觸發RTC AUTO STD, 不要做Full view check
            {
                ret=ShowMyMessageBox_YES_NO("即將執行RTC Calibration model流程, 請確認Socket中沒有IC或異物", "Ready to do RTC Calibration model process, please check there is no abnormail object in socket!");   //AI(W906-YESNO) 20260925: 真的問操作員（替身時代回 0 ⇒ ret!=1 ⇒ 每拍 break、默默停在 40330）
                if(ret!=1)
                {
                    break;
                }

                Task=iCASE_REAL_CCD6;
                W7T1_FCONTACT_INITROI();
            }
            else
            {
                COM2->bRealTimeCom_ReceiveOK[COM2->rtFullTNG]=false;
                COM2->SendCommToVision(COM2->rtFullTOK, true);
                DoTestHeadMotorDelay.SetSecAndOn(10);
                Task=iCASE_REAL_CCD4;
            }
            break;
        case iCASE_REAL_CCD4:
            if(COM2->bRealTimeCom_ReceiveOK[COM2->rtFullTOK])
            {
                COM2->DoReleaseAndInspEnd();
                bRTCFullViewError=false;                                                                                                                        //Steven 20120206 : RTC重複錯誤
                if(CosFunction.bIndexCheckCanTurnOff &&                                                                                                         //Isaac 20211019 : 可選擇做index check的時機
                  ((IniConfig.iD71IndexCheckOnOffMode==0 && bLotStartEndNeedIndexCheck==false) ||
                   (IniConfig.iD71IndexCheckOnOffMode==1 && bIndexJamNeedIndexcheck==false) ||
                    IniConfig.iD71IndexCheckOnOffMode==2))
                {
                    if(IniConfig.bUseAutoSiteMapping &&
                       CosFunction.bUSEJCETSiteMapMode==false &&
                       LastSet.iRunStartMode==rsmAutoSiteMap &&
                       bSiteMappingCHKOK==false)                                                                                                                //Steven 20190313 : Fixed for auto site mpa no open bin when enable [D55]
                        Task=150;
                    else
                        Task=1500;                                                                                                                              //Steven 20231208 : 1550 --> 1500
                }
                else if(IniConfig.bD55DisableIndexCheck &&
                        REAL_TIME_CCD==true &&
                        COM2->bCCDDummyRum==false)                                                                                                              //Steven 20150723 : Fixed for SCK
                {
                    if(IniConfig.bUseAutoSiteMapping &&
                       CosFunction.bUSEJCETSiteMapMode==false &&
                       LastSet.iRunStartMode==rsmAutoSiteMap &&
                       bSiteMappingCHKOK==false)                                                                                                                //Steven 20190313 : Fixed for auto site mpa no open bin when enable [D55]
                        Task=150;
                    else
                        Task=1500;                                                                                                                              //Steven 20231208 : 1550 --> 1500
                }
                else if(CosFunction.bAfterAutoCleanNoIndexCheck &&                                                                                              //Steven 20191212 : 劉仁洲說Auto Clean只要作一次Index Check
                        (IniConfig.iD69IndexCheckModeForAutoClean==1 ||
                         IniConfig.iD69IndexCheckModeForAutoClean==2) &&
                        bAutoCleanFinishOnlyUseRTC==true)
                {
                    bAutoCleanFinishOnlyUseRTC=false;
                    if(IniConfig.bUseAutoSiteMapping &&
                       CosFunction.bUSEJCETSiteMapMode==false &&
                       LastSet.iRunStartMode==rsmAutoSiteMap &&
                       bSiteMappingCHKOK==false)                                                                                                                //Steven 20190313 : Fixed for auto site mpa no open bin when enable [D55]
                        Task=150;
                    else
                        Task=1500;                                                                                                                              //Steven 20231208 : 1550 --> 1500
                }
                else if(CosFunction.bBeforeAutoCleanOnlyUseRTC==true &&                                                                                         //JerryYang 20161216 (Steven) auto clean的前後只靠RTC來檢查socket,不做index下壓至socket吸真空
                        bAutoCleanFinishOnlyUseRTC==true &&
                        REAL_TIME_CCD==true &&
                        COM2->bCCDDummyRum==false)
                {
                    bAutoCleanFinishOnlyUseRTC=false;
                    if(IniConfig.bUseAutoSiteMapping &&
                       CosFunction.bUSEJCETSiteMapMode==false &&
                       LastSet.iRunStartMode==rsmAutoSiteMap &&
                       bSiteMappingCHKOK==false)                                                                                                                //Steven 20190313 : Fixed for auto site mpa no open bin when enable [D55]
                        Task=150;
                    else
                        Task=1500;                                                                                                                              //Steven 20231208 : 1550 --> 1500
                }
                else if(IniConfig.bD75OneCycleFinishedAlawayLearnRTCGolden ||                                                                                   //Sam 20240117 : OneCycle 完成做完 Full view check 後都需要做 RTC Learning golden
                        bTriggerRTC_AutoSTD)                                                                                                                    //JerryYang 20240829 : SPIL訓永 要求手動觸發RTC AUTO STD
                {
                    Task=iCASE_REAL_CCD6;
                    W7T1_FCONTACT_INITROI();
                }
                else
                {
                    Task=9;                                                                                                                                     //2013-01-15    Dell
                }
            }
            else if(COM2->bRealTimeCom_ReceiveOK[COM2->rtFullTNG])
            {
                COM2->DoReleaseAndInspEnd();
                Task=iCASE_REAL_CCD5;
            }
            else if(DoTestHeadMotorDelay.Off())
            {
                if(COM2->OpenRTCComPortAgain())                                                                                                                 //ChungHung 20121005 add
                    ShowErrorMessage("WAR0337", 0, MMIndex, 0, __FUNC__);                                                                                       //RTC FullT Time Out Error!
                COM2->DoReleaseAndInspEnd();
                Task=1;
            }
            break;
        case iCASE_REAL_CCD5:
            if(IndexAlarmInArmAway()==true)                                                                                                                     //Steven 20130613 : Index異常時, In Arm要先讓位功能
            {
                if(IniConfig.bD40IndexICFallDownMustPressFMotorDown)                                                                                            //Steven 20151022 : add for MAXIM
                    bIsTestSitICFallDown=true;

                COM2->DoReleaseAndInspEnd();
                if(CosFunction.bRTCFullViewErrorOnlyRetry)                                                                                                      //Steven 20150304 : bRTCFullViewErrorOnlyRetry預設改為True
                    ret=ShowErrorMessage("WAR0343", K_RETRY, MMCCD, bRTCFullViewError, ErrPart);                                                                //RTC FullView Index Check Error!
                else
                    ret=ShowErrorMessage("WAR0343", K_RETRY|K_SKIP, MMCCD, bRTCFullViewError, ErrPart);                                                         //RTC FullView Index Check Error!
                bRTCFullViewError=true;                                                                                                                         //Steven 20120206 : RTC重複錯誤

                if(ret==K_RETRY)
                {
                    Task=1;
                }
                else
                {
                    Task=iCASE_REAL_CCD6;
                    W7T1_FCONTACT_INITROI();
                    bRTCFullViewError=false;
                }
            }
            break;
        case iCASE_REAL_CCD6:
            if(W7T1_FCONTACT_DOROI(true))                                                                                                                  //ChungHung 20121011 鉬FullView 謖IndexCheck
            {
                Task=9;
            }
            break;
        case 50000:                                                                                                                                             //Index 1 至 Sock 吸料
            if(CUSTOMER_CODE==CC_ASE_CL &&                                                                                                                      //JerryYang 20250120 : modify
              ((DeviceForm.iSocketInitialICCheckPosition==1 && IniConfig.bTestIcCheckInContact==true) ||
              (LastSet.iD41SocketInitialICCheckPosition==1 && IniConfig.bTestIcCheckInContact==false)))                                                         //Above Socket
            {
                iContactZ=Prod.TestZ1_Test-Prod.TestZ1_Drop_Offset-iIndexArmCheck_SG_Arm1-Offset.iIndexArmContact[0];                                           //JerryYang 20250822 : Norman要求不要加offset
            }
            else
            {
                iContactZ=Prod.TestZ1_Test-Prod.TestZ1_Drop_Offset-iIndexArmCheck_SG_Arm1;
            }

            if(MOT[MTestZ1].Gali_MotMove(iContactZ, iSpeedSlow))                                                                                                //KaiChen 20200826 ：矽格-湖口，要求IndexCheck使用Contact高度不使用Offset
            {
                if(IndexCheckOneByOne(false, 0, iSiteCount, true))                                                                                              //Ifor 20200617 : add Use One By One Index Check Function 整合
                {
                    Task=50100;
                }
            }
            break;
        case 50100:                                                                                                                                             //Index 1 至 Drop 位置確認真空
            //W7T1_FIOSET_PISD1();

            if(CUSTOMER_CODE==CC_ASE_CL &&                                                                                                                      //JerryYang 20250120 : modify
              ((DeviceForm.iSocketInitialICCheckPosition==1 && IniConfig.bTestIcCheckInContact==true) ||
              (LastSet.iD41SocketInitialICCheckPosition==1 && IniConfig.bTestIcCheckInContact==false)))                                                         //Above Socket
            {
                iIndexUpPos=GetSocketCheckPos(Prod.TestZ1_Test);                                                                                                //Steven 20140620 : 整合為Function
                iIndexUpPos-=Offset.iIndexArmContact[0];
            }
            else
            {
                iIndexUpPos=GetSocketCheckPos(Prod.TestZ1_Test);
            }

             if(MOT[MTestZ1].Gali_MotMove(Prod.TestZ1_Test-Prod.TestZ1_Drop_Offset+iIndexUpPos-iIndexArmCheck_SG_Arm1, iSpeedFast))                             //KaiChen 20200826 ：矽格-湖口，要求IndexCheck使用Contact高度不使用Offset
            {
                Task=50110;
                CheckSocketHasICDelay.SetSecAndOn(1.0);                                                                                                         //JerryYang 20250807 : 0.5-> 1 sec
            }
            break;
        case 50110:
            if(CheckSocketHasICDelay.Off())
            {
                if(IndexCheckOneByOne(false, 0, iSiteCount))                                                                                                    //Ifor 20200617 : add Use One By One Index Check Function 整合
                {
                    Task=50000;
                    iSiteCount++;
                    if(iSiteCount>=(FTestSuck.iShtCnt))
                    {
                        iSiteCount=0;
                        DoTestHeadMotorDelay.SetSecAndOn(0.5);                                                                                                  //Steven 20110908 : 上來後也要Delay一下
                        //Task=122;
                        Task=122100;
                    }
                }
            }
            break;
        case 60000:                                                                                                                                             //Index 2  至 Sock 吸料

            if(CUSTOMER_CODE==CC_ASE_CL &&                                                                                                                      //JerryYang 20250120 : modify
              ((DeviceForm.iSocketInitialICCheckPosition==1 && IniConfig.bTestIcCheckInContact==true) ||
              (LastSet.iD41SocketInitialICCheckPosition==1 && IniConfig.bTestIcCheckInContact==false)))                                                         //Above Socket
            {
                iContactZ=Prod.TestZ2_Test-Prod.TestZ2_Drop_Offset-iIndexArmCheck_SG_Arm2-Offset.iIndexArmContact[1];                                           //JerryYang 20250822 : Norman要求不要加offset
            }
            else
            {
                iContactZ=Prod.TestZ2_Test-Prod.TestZ2_Drop_Offset-iIndexArmCheck_SG_Arm2;
            }

            if(MOT[MTestZ2].Gali_MotMove(iContactZ, iSpeedSlow))                                                                                                //KaiChen 20200826 ：矽格-湖口，要求IndexCheck使用Contact高度不使用Offset
            {
                if(IndexCheckOneByOne(false, 1, iSiteCount, true))                                                                                              //Ifor 20200617 : add Use One By One Index Check Function 整合
                {
                    Task=60100;
                }
            }
            break;
        case 60100:                                                                                                                                             //Index 2  至 Sock 吸料
            if(CUSTOMER_CODE==CC_ASE_CL &&                                                                                                                      //JerryYang 20250120 : modify
              ((DeviceForm.iSocketInitialICCheckPosition==1 && IniConfig.bTestIcCheckInContact==true) ||
              (LastSet.iD41SocketInitialICCheckPosition==1 && IniConfig.bTestIcCheckInContact==false)))                                                         //Above Socket
            {
                iIndexUpPos=GetSocketCheckPos(Prod.TestZ2_Test);                                                                                                //Steven 20140620 : 整合為Function
                iIndexUpPos-=Offset.iIndexArmContact[1];
            }
            else
            {
                iIndexUpPos=GetSocketCheckPos(Prod.TestZ2_Test);                                                                                                //Steven 20140620 : 整合為Function
            }

            if(MOT[MTestZ2].Gali_MotMove(Prod.TestZ2_Test-Prod.TestZ2_Drop_Offset+iIndexUpPos-iIndexArmCheck_SG_Arm2, iSpeedFast))
            {
                Task=60110;
                CheckSocketHasICDelay.SetSecAndOn(1.0);                                                                                                         //JerryYang 20250807 : 0.5-> 1 sec
            }
            break;
        case 60110:
            if(CheckSocketHasICDelay.Off())
            {
                if(IndexCheckOneByOne(false, 1, iSiteCount))                                                                                                    //Ifor 20200617 : add Use One By One Index Check Function 整合
                {
                    Task=60000;
                    iSiteCount++;
                    if(iSiteCount>=FTestSuck.iShtCnt)
                    {
                        iSiteCount=0;
                        Task=142100;
                        DoTestHeadMotorDelay.SetSecAndOn(0.5);                                                                                                  //Steven 20110908 : 上來後也要Delay一下
                    }
                }
            }
            break;
        case 80000:                                                                                                                                             //Ifor 20200617 : add Use One By One Index Check Function 整合
            if(IndexCheckOneByOne(false, 0, iSiteCount))                                                                                                        //Ifor 20200617 : add Use One By One Index Check Function 整合
            {
                iSiteCount++;
                if(iSiteCount>=FTestSuck.iShtCnt)
                {
                    Task=122100;
                }
            }
            break;
        case 80010:                                                                                                                                             //Ifor 20200617 : add Use One By One Index Check Function 整合
            if(IndexCheckOneByOne(false, 1, iSiteCount))                                                                                                        //Ifor 20200617 : add Use One By One Index Check Function 整合
            {
                iSiteCount++;
                if(iSiteCount>=FTestSuck.iShtCnt)
                {
                    Task=142100;
                }
            }
            break;

        default:
            // offline-safe terminal: any gated case id parks here (golden would
            // have run the down-press tree).  Keep the cursor; no hardware act.
            (void)ret;
            break;
    }
}
//------------------------------------------------------------------------------
//  DoTesterSidePush (golden :8656-8684) -- Richard 渠梁 side-push.  MOT[]/cylin.
//------------------------------------------------------------------------------
int DoTesterSidePush(bool bInitial)                                             //Richard 20220321 : 渠梁Side Push
{
#if 0 // TODO(W7) -- golden :8656-8684 (MOT[]/cylinder side-push)
#endif
    (void)bInitial;
    return 0;                                                                  // golden default
}
//------------------------------------------------------------------------------
//  DoAllPassVerifyRTC (golden :8685-8817) -- RTC auto model verify SM.
//------------------------------------------------------------------------------
// ===========================================================================
//  GOLDEN VERBATIM PAIR -- DoAllPassVerifyRTC
//  GATED : golden atester.cpp:8685-8814 (130 lines), inert reference text.
//  LIVE  : the slim DoAllPassVerifyRTC() immediately AFTER the #endif below.  It is
//          UNCHANGED by this wave -- net behaviour change is ZERO.
//  WHY   : the census scored this "translated" because a same-named LIVE body
//          existed, without comparing SIZE.  Golden's 130 lines were NOWHERE in
//          the tree -- lost text, not deferred behaviour.  Now the text EXISTS
//          and is auditable, so a later un-gate is mechanical.
//  RULES : nothing inside the gate is fixed, renamed, reflowed or reindented;
//          golden's own defects and misspellings are preserved so a diff
//          against golden stays EMPTY.  Being gated it needs NO callee to
//          exist -- no stub, declaration or header was added for it.
//  SHAPE : same pair shape as csystem.cpp DoTrayFeedProcess / MainProc /
//          DoAllProcess (PT-W6a) golden-verbatim gates.
// ===========================================================================
#if 0 // GOLDEN VERBATIM -- golden atester.cpp:8685-8814 (130 lines).  GATE G-PTk4-DoAllPassVerifyRTC.  NOT COMPILED: the ACTIVE DoAllPassVerifyRTC() is the slim body immediately after this #endif.
bool DoAllPassVerifyRTC(bool bInitial)
{
    static bool bRTCRetry=false;                                                //Ifor 20251023 add:RTC無回應Retry 一次
    if(bInitial==true)
    {
        iDoAllPassVerifyTask=1;
        return false;
    }

    int &Task=iDoAllPassVerifyTask;
    switch(Task)
    {
        case 1:
            COM2->bRealTimeCom_ReceiveOK[COM2->rtRelease]=false;
            COM2->bRealTimeCom_ReceiveOK[COM2->rtInspEnd]=false;
            COM2->DoReleaseAndInspEnd();
            DoTestHeadMotorDelay.SetSecAndOn(10);
            Task=41010;
            break;
        case 41010:
            if(COM2->bRealTimeCom_ReceiveOK[COM2->rtRelease] && COM2->bRealTimeCom_ReceiveOK[COM2->rtInspEnd] && COM2->bRealTimeCom_ReceiveOK[COM2->rtInspStart])  //JerryYang 20220215 : Release跟InspEnd一起送
            {
                Task=41050;
                bRTCRetry=false;
            }
            else if(DoTestHeadMotorDelay.Off())
            {
                if(bRTCRetry==true)
                {
                     bRTCRetry=false;
                    if(COM2->OpenRTCComPortAgain())
                        ShowMyMessage("RTC Re-start Time out of Verify RTC");  //JerryYang 20220215 : Release跟InspEnd一起送
                    COM2->DoReleaseAndInspEnd();
                }
                else
                {
                    bRTCRetry=true;
                }
                Task=1;
            }
            break;
        case 41050:
            COM2->bRealTimeCom_ReceiveOK[COM2->rtOPENVERIFYNG]=false;
            COM2->SendCommToVision(COM2->rtOPENVERIFYOK, true);
            DoTestHeadMotorDelay.SetSecAndOn(10);
            Task=41100;
            break;
        case 41100:
            if(COM2->bRealTimeCom_ReceiveOK[COM2->rtOPENVERIFYOK])
            {
                Task=41200;
            }
            else if(COM2->bRealTimeCom_ReceiveOK[COM2->rtOPENVERIFYNG])
            {
                ShowMyMessage("RTC Open Verify NG");
                COM2->DoReleaseAndInspEnd();
                Task=1;
            }
            else if(DoTestHeadMotorDelay.Off())
            {
                if(COM2->OpenRTCComPortAgain())
                    ShowMyMessage("RTC Open Verify Time out");
                COM2->DoReleaseAndInspEnd();
                Task=1;
            }
            break;
        case 41200:
            COM2->bRealTimeCom_ReceiveOK[COM2->rtALLPASSNG]=false;
            COM2->SendCommToVision(COM2->rtALLPASSOK, true);
            DoTestHeadMotorDelay.SetSecAndOn(10);
            Task=41300;
            break;
        case 41300:
            if(COM2->bRealTimeCom_ReceiveOK[COM2->rtALLPASSOK])
            {
                Task=42000;                                                     //JerryYang 20220215 : Release跟InspEnd一起送
//                return true;
            }
            else if(COM2->bRealTimeCom_ReceiveOK[COM2->rtALLPASSNG])
            {
                Task=41400;
            }
            else if(DoTestHeadMotorDelay.Off())
            {
                if(COM2->OpenRTCComPortAgain())
                    ShowMyMessage("RTC Verify All Pass Time out");
                COM2->DoReleaseAndInspEnd();
                Task=41200;
            }
            break;
        case 41400:
            if(IndexAlarmInArmAway()==true)
            {
                bIsTestSitICFallDown=true;
                ShowMyMessage("RTC Verify All Pass NG!!");
                Task=1;
            }
            break;
        case 42000:
            COM2->bRealTimeCom_ReceiveOK[COM2->rtRelease]=false;
            COM2->bRealTimeCom_ReceiveOK[COM2->rtInspEnd]=false;
            COM2->DoReleaseAndInspEnd();
            DoTestHeadMotorDelay.SetSecAndOn(10);
            Task=42100;
            break;
        case 42100:
            if(COM2->bRealTimeCom_ReceiveOK[COM2->rtRelease] && COM2->bRealTimeCom_ReceiveOK[COM2->rtInspEnd] && COM2->bRealTimeCom_ReceiveOK[COM2->rtInspStart])
            {
                bRTCRetry=false;
                return true;
            }
            else if(DoTestHeadMotorDelay.Off())
            {
                if(bRTCRetry==true)
                {
                     bRTCRetry=false;
                    if(COM2->OpenRTCComPortAgain())
                        ShowMyMessage("RTC Re-start Time out of Verify RTC");
                    COM2->DoReleaseAndInspEnd();
                }
                else
                {
                    bRTCRetry=true;
                }
                Task=42000;
            }
            break;
    }
    return false;
}
#endif // GOLDEN VERBATIM -- golden atester.cpp:8685-8814  (GATE G-PTk4-DoAllPassVerifyRTC, end)
bool DoAllPassVerifyRTC(bool bInitial)                                          //jou 2014-06-24 RTC 自動進行Model驗證
{
#if 0 // TODO(W7) -- golden :8685-8817 (RTC verify SM, COM2/RTC)
#endif
    (void)bInitial;
    return false;                                                              // golden default
}
//------------------------------------------------------------------------------
//  DoSetupTest (golden :8818-9001) -- setup-test SM.  MOT[]/Suck/Socket.
//------------------------------------------------------------------------------
// ===========================================================================
//  GOLDEN VERBATIM PAIR -- DoSetupTest
//  GATED : golden atester.cpp:8818-8998 (181 lines), inert reference text.
//  LIVE  : the slim DoSetupTest() immediately AFTER the #endif below.  It is
//          UNCHANGED by this wave -- net behaviour change is ZERO.
//  WHY   : the census scored this "translated" because a same-named LIVE body
//          existed, without comparing SIZE.  Golden's 181 lines were NOWHERE in
//          the tree -- lost text, not deferred behaviour.  Now the text EXISTS
//          and is auditable, so a later un-gate is mechanical.
//  RULES : nothing inside the gate is fixed, renamed, reflowed or reindented;
//          golden's own defects and misspellings are preserved so a diff
//          against golden stays EMPTY.  Being gated it needs NO callee to
//          exist -- no stub, declaration or header was added for it.
//  SHAPE : same pair shape as csystem.cpp DoTrayFeedProcess / MainProc /
//          DoAllProcess (PT-W6a) golden-verbatim gates.
// ===========================================================================
#if 0 // GOLDEN VERBATIM -- golden atester.cpp:8818-8998 (181 lines).  GATE G-PTk4-DoSetupTest.  NOT COMPILED: the ACTIVE DoSetupTest() is the slim body immediately after this #endif.
bool DoSetupTest(int iContactArm)
{
    int &Task=iSetupTask, ret;
    int Pos1, Pos2;
    int iNN=IsNNMode();

    switch(Task)
    {
        case 1:
            Pos1=MOT[MTestZ1].Gali_ReadPos();
            Pos2=MOT[MTestZ2].Gali_ReadPos();
            TestSocket.ClearAll();

            if(IniConfig.bIndexArm2SupplyLight==true ||                         //jou 2012-10-19 Index Arm 2 供應光源 for CMOS
               TestIF_File.bForEgisTecTest==true     ||                         //Steven 20140922 : Arm2當作指紋測試
               (IniConfig.bD58UseArm1PickPlaceArm2Test==true &&                 //kevin 20150127 Arm1 下壓 arm2 測試
                TestIF_File.bArm1PickPlaceArm2Test==true))                      //Ifor 20200811 Fix: Arm1 Pick Place Arm2Test 需卡兩個條件
            {
                IndexStatus=Z1Down_Z2Up;
                iIndexArm=0;
                for(int i=0; i<FTestSuck.iShtRow; i++)
                {
                    for(int j=0; j<FTestSuck.iShtCol; j++)
                    {
                        if(FTestSuck.Item[i][j]==HAS_IC)
                        {
                            if(BAR_CODE_INSTALL!=ebctUninstall &&
                               TestIF_File.bEnableBarCode)                      //Steven 20160123 : For 2D function do contact test
                            {
                                TestSocket.SetItemData(i, j, HAS_IC);           //Steven 20160223 : 修正Contact Test時,連續測試的問題
                                TestSocket.cDeviceInf[i][j]=FTestSuck.cDeviceInf[i][j];
                            }
                            else
                            {
                                TestSocket.SetItemData(i, j, HAS_IC);
                            }
                        }
                    }
                }
            }
            else
            {
                if(bUseTwoArm32Site==true)
                {
                    iIndexArm=0;
                    for(int i=0; i<FTestSuck.iShtRow; i++)
                    {
                        for(int j=0; j<FTestSuck.iShtCol; j++)
                        {
                            if(FTestSuck.Item[i][j]==HAS_IC)
                            {
                                if(BAR_CODE_INSTALL!=ebctUninstall &&
                                   TestIF_File.bEnableBarCode)                  //Steven 20160123 : For 2D function do contact test
                                {
                                    TestSocket.SetItemData(i+iNN, j, HAS_IC);   //Steven 20160223 : 修正Contact Test時,連續測試的問題
                                    TestSocket.cDeviceInf[i+iNN][j]=FTestSuck.cDeviceInf[i][j];
                                }
                                else
                                {
                                    TestSocket.SetItemData(i+iNN, j, HAS_IC);
                                }
                            }

                            if(BTestSuck.Item[i][j]==HAS_IC)                    //Steven 20160216 : 修正Contact Test可能不會測試的問題
                            {
                                if(BAR_CODE_INSTALL!=ebctUninstall &&
                                   TestIF_File.bEnableBarCode)                  //Steven 20160123 : For 2D function do contact test
                                {
                                    TestSocket.SetItemData(i, j, HAS_IC);       //Steven 20160223 : 修正Contact Test時,連續測試的問題
                                    TestSocket.cDeviceInf[i][j]=BTestSuck.cDeviceInf[i][j];
                                }
                                else
                                {
                                    TestSocket.SetItemData(i, j, HAS_IC);
                                }
                            }
                        }
                    }
                }
                else
                {
                    if(Pos1>Pos2)
                    {
                        IndexStatus=Z1Up_Z2Down;
                        iIndexArm=iContactArm;
                        for(int i=0; i<BTestSuck.iShtRow; i++)
                        {
                            for(int j=0; j<BTestSuck.iShtCol; j++)
                            {
                                if(BTestSuck.Item[i][j]==HAS_IC)                //Steven 20160216 : 修正Contact Test可能不會測試的問題
                                {
                                    if(BAR_CODE_INSTALL!=ebctUninstall &&
                                       TestIF_File.bEnableBarCode)              //Steven 20160123 : For 2D function do contact test
                                    {
                                        TestSocket.SetItemData(i, j, HAS_IC);   //Steven 20160223 : 修正Contact Test時,連續測試的問題
                                        TestSocket.cDeviceInf[i][j]=BTestSuck.cDeviceInf[i][j];
                                    }
                                    else
                                    {
                                        TestSocket.SetItemData(i, j, HAS_IC);
                                    }
                                }
                            }
                        }
                    }
                    else
                    {
                        IndexStatus=Z1Down_Z2Up;
                        iIndexArm=iContactArm;
                        for(int i=0; i<FTestSuck.iShtRow; i++)
                        {
                            for(int j=0; j<FTestSuck.iShtCol; j++)
                            {
                                if(FTestSuck.Item[i][j]==HAS_IC)                //Steven 20160216 : 修正Contact Test可能不會測試的問題
                                {
                                    if(BAR_CODE_INSTALL!=ebctUninstall &&
                                       TestIF_File.bEnableBarCode)              //Steven 20160123 : For 2D function do contact test
                                    {
                                        TestSocket.SetItemData(i, j, HAS_IC);   //Steven 20160223 : 修正Contact Test時,連續測試的問題
                                        TestSocket.cDeviceInf[i][j]=FTestSuck.cDeviceInf[i][j];
                                    }
                                    else
                                    {
                                        TestSocket.SetItemData(i, j, HAS_IC);
                                    }
                                }
                            }
                        }
                    }
                }
            }
#ifdef SOFT_SIMULTE
  #ifdef DEBUG
            if(fContact->fShow==true)
            {
                for(int i=0; i<TestSocket.iShtRow; i++)
                    for(int j=0; j<TestSocket.iShtCol; j++)
                        TestSocket.SetItemData(i, j, HAS_IC);
            }
  #endif
#endif
            Task=2200;
        case 2200:
            ProcessStartTestData(iIndexArm);
            DoSetupTestDelay.SetSecAndOn(TestIF.iMaxTime);
            InitTestTask();
            Task=2400;
        case 2400:
            ret=ProcessTestResult(iIndexArm);

            if(fContact->fShow==true &&
               iContactMode==CONTACT_DEVICE_MAP_CHECK &&
               fContact->bPlaceLoad==true)
            {
                ret=1;
            }

            if(ret==1)
            {
                bATC_SITE_2ND_CHECK[iIndexArm]=false;                           //Ifor 20160509 add ATC 測試時開啟第二點溫度監控
                if(ATC_SYSTEM==eNewATCSystem)
                {
                    for(int i=0; i<iATC_Use_Heat_Count; i++)                    //Ifor 20160516 修改ATC Heat 設定數
                        bATCSiteTest[i]=false;
                    ATC_InterfaceForm->SiteTesting(iATC_Use_Heat_Count, bATCSiteTest);
                }
                else if(ATC_SYSTEM==eATCHonPrecType)
                {
                    ATCInterfaceForm->SendTestStart(iIndexArm);
                }
                SetNoiseDelay=false;
                TestISTimeOut=false;
                ProcessCount(iIndexArm, TestSocket.HasRealIC());  //Eastsun 20260515 F022: D7
                TestSocket.ClearAll();
                Task=1;
                return true;
            }
            break;
    }
    return false;
}
#endif // GOLDEN VERBATIM -- golden atester.cpp:8818-8998  (GATE G-PTk4-DoSetupTest, end)
bool DoSetupTest(int iContactArm)
{
#if 0 // TODO(W7) -- golden :8818-9001 (MOT[]/Suck setup-test SM)
#endif
    (void)iContactArm;
    (void)iSetupTask;
    return false;                                                              // golden default
}
//------------------------------------------------------------------------------
//  GetTempICResult (golden :9002-9150) -- IC temperature measure SM.
//------------------------------------------------------------------------------
// ===========================================================================
//  GOLDEN VERBATIM PAIR -- GetTempICResult
//  GATED : golden atester.cpp:9002-9068 (67 lines), inert reference text.
//  LIVE  : the slim GetTempICResult() immediately AFTER the #endif below.  It is
//          UNCHANGED by this wave -- net behaviour change is ZERO.
//  WHY   : the census scored this "translated" because a same-named LIVE body
//          existed, without comparing SIZE.  Golden's 67 lines were NOWHERE in
//          the tree -- lost text, not deferred behaviour.  Now the text EXISTS
//          and is auditable, so a later un-gate is mechanical.
//  RULES : nothing inside the gate is fixed, renamed, reflowed or reindented;
//          golden's own defects and misspellings are preserved so a diff
//          against golden stays EMPTY.  Being gated it needs NO callee to
//          exist -- no stub, declaration or header was added for it.
//  SHAPE : same pair shape as csystem.cpp DoTrayFeedProcess / MainProc /
//          DoAllProcess (PT-W6a) golden-verbatim gates.
// ===========================================================================
#if 0 // GOLDEN VERBATIM -- golden atester.cpp:9002-9068 (67 lines).  GATE G-PTk3-GetTempICResult.  NOT COMPILED: the ACTIVE GetTempICResult() is the slim body immediately after this #endif.
bool GetTempICResult()
{
    static bool bContactOK=false;
    int &Task=iTempICTask;
    AnsiString asWriteComm=":StartTest+";
    if(bPauseTester)
    {
        bPauseTester=false;
        TempICTimeOut.SetSecAndOn(Prod.iTesterDummyTime);
    }

    switch(Task)
    {
        case 1:
            if(SoftStop)
                break;
            IsTest=true;
            bContactOK=false;

            bTempComm6ReceiveOK=false;
            TempICTimeOut.SetSecAndOn(Prod.iTesterDummyTime);
            Task=10;
            break;
        case 10:                                                                //Send Rs232 command
            COM2->TempComm6->WriteCommData(asWriteComm.c_str(), asWriteComm.Length());
            bTempComm6ReceiveOK=false;
            Task=60;
            break;
        case 60:
            if(bTempComm6ReceiveOK)
            {
                bContactOK=true;
                TempICDelay.SetSecAndOn(0.2);
                Task=70;
            }

            if(TempICTimeOut.Off())
            {
                if(bContactOK)
                {
                    if(bTempComm6ReceiveOK)
                    {
                        Task=100;
                    }
                }
                else
                {
                    Task=100;
                }
            }
            break;
        case 70:
            if(TempICDelay.Off())
            {
                Task=10;
            }
            break;
        case 100:
            for(int i=0; i<TestSocket.iShtRow; i++)
                for(int j=0; j<TestSocket.iShtCol; j++)
                    iTesterBIN[i][j]=0;
            Task=1;                                                             //kevin 20130808
            return true;
    }

    return false;
}
#endif // GOLDEN VERBATIM -- golden atester.cpp:9002-9068  (GATE G-PTk3-GetTempICResult, end)
bool GetTempICResult()                                                          //kevin 20130808 IC量測溫度
{
#if 0 // TODO(W7) -- golden :9002-9150 (temperature measure SM)
#endif
    (void)iTempICTask;
    return false;                                                              // golden default
}
//------------------------------------------------------------------------------
void InitIndexEveryTimeCheckEP()
{
    iIndexEveryTimeCheckEPTask=1;
}
//------------------------------------------------------------------------------
//  IndexEveryTimeCheckEP (golden :9158-9265) -- EP-balloon pressure check SM.
//  ACTIVE: case 1 early-out (when NOT REALLY or EP not installed -> return true)
//  reproduced VERBATIM; the case 100/200/300 EP-pressure body (Gali Z move +
//  ADAM DAQ) reproduced over Sim HAL + ADAM_* shims; the SOFT_SIMULTE-gated
//  alarm tree is reproduced verbatim (SOFT_SIMULTE not defined -> #else live).
//  AI(W7T1-Integrate) 20260701: restored the golden `bool IndexEveryTimeCheckEP()` signature
//  (golden atester.cpp:9158).  The un-gated down-press tree now calls it on a LIVE path as
//  `if(IndexEveryTimeCheckEP())` (case 30000, golden atester.cpp:8331), so the W6.4 void wrapper
//  (added while the only caller was gated) is dropped and the bool body carries the golden name.
//------------------------------------------------------------------------------
TQPF_Timer CheckEPTimer;
TQPF_Timer ReleaseEPTimer;
bool IndexEveryTimeCheckEP()
{
    #ifndef SOFT_SIMULTE
    int ret=0;
    bool bAdamAlarm[2]={false, false};
    #endif
    int &Task=iIndexEveryTimeCheckEPTask;

    switch(Task)
    {
        case 1:
            if(LastSet.iRealDummy==REALLY && (EP_Install==3 || EP_Install==5))  //JerryYang 20171030 (wei) fix D24 EP check功能無效問題
            {
                bIndexEveryTimeCheckEPing=true;
                Task=100;
            }
            else
            {
                return true;
            }
            break;
        case 100:
            if(MOT[MTestZ1].Gali_Two_ZAxis_Move(Prod.TestZ1_Safe, MOT[MTestZ1].GailSpeed, "IndexEveryTimeCheckEP 100"))
            {
                ADAM_DirectWriteData((EP_MAXKPA<=500)?4095:2275, 0);            //ChungHung 20140508 有些氣球會破
                CheckEPTimer.SetSecAndOn(5);                                    //Steven 20240618 : 3 --> 5
                Task=200;
            }
            break;
        case 200:
            ADAM_DirectWriteData((EP_MAXKPA<=500)?4095:2275, 0);                                                        //ChungHung 20140508 有些氣球會破
            #ifdef SOFT_SIMULTE
                return true;
            #else
                if(CheckEPTimer.Off())
                {
                    if((EP_Install==3 || EP_Install==5))                                                                //20111217 ChungHung
                    {
                        ADAM_Rang(IniConfig.iD26EPEncoderRange);
                        if(EP_Install==5)
                        {
                            bAdamAlarm[0]=ADAM_Alarm(0);
                            bAdamAlarm[1]=ADAM_Alarm(1);
                        }
                        else
                        {
                            bAdamAlarm[0]=ADAM_Alarm();
                            bAdamAlarm[1]=false;
                        }

                        if(bAdamAlarm[0]==false && bAdamAlarm[1]==false)
                        {
                            ADAM_WriteVoltage(DeviceForm.dPress);
                            bIndexEveryTimeCheckEPing=false;
                            ReleaseEPTimer.SetSecAndOn(1);                                                              //JerryYang 20171030 (wei) delay1秒 等待EP洩氣
                            Task=300;
                        }
                        else
                        {
                            ret=ShowErrorMessage("WAR1605", K_RETRY|K_SKIP, 0, MMSystem, "IndexEveryTimeCheckEP_200");  //"請檢查EP是否漏氣!"
                            if(ret==K_RETRY)
                            {
                                Task=1;
                            }
                            else
                            {
                                bIndexEveryTimeCheckEPing=false;
                                return true;
                            }
                        }
                    }
                    else
                    {
                        if(Sen[SnEPAlarm].IsOff()==false)                                                               //9045 lan=0 ip=6 port=3 bit=1
                        {
                            ADAM_WriteVoltage(DeviceForm.dPress);
                            bIndexEveryTimeCheckEPing=false;
                            ReleaseEPTimer.SetSecAndOn(1);                                                              //JerryYang 20171030 (wei) delay1秒 等待EP洩氣
                            Task=300;
                        }
                        else
                        {
                            ret=ShowErrorMessage("WAR1605", K_RETRY|K_SKIP, 0, MMSystem, "IndexEveryTimeCheckEP_200");  //"請檢查EP是否漏氣!"
                            if(ret==K_RETRY)
                            {
                                Task=1;
                            }
                            else
                            {
                                bIndexEveryTimeCheckEPing=false;
                                return true;
                            }
                        }
                    }
                }
            break;
            #endif
        case 300:
            if(ReleaseEPTimer.Off())                                            //JerryYang 20171030 (wei) delay1秒 等待EP洩氣
            {
                return true;
            }
            break;
    }
    return false;
}
//------------------------------------------------------------------------------
//  CheckAndRecodrEP (golden :9266-9412) -- EP pressure record/alarm.
//------------------------------------------------------------------------------
// ===========================================================================
//  GOLDEN VERBATIM PAIR -- CheckAndRecodrEP
//  GATED : golden atester.cpp:9266-9411 (146 lines), inert reference text.
//  LIVE  : the slim CheckAndRecodrEP() immediately AFTER the #endif below.  It is
//          UNCHANGED by this wave -- net behaviour change is ZERO.
//  WHY   : the census scored this "translated" because a same-named LIVE body
//          existed, without comparing SIZE.  Golden's 146 lines were NOWHERE in
//          the tree -- lost text, not deferred behaviour.  Now the text EXISTS
//          and is auditable, so a later un-gate is mechanical.
//  RULES : nothing inside the gate is fixed, renamed, reflowed or reindented;
//          golden's own defects and misspellings are preserved so a diff
//          against golden stays EMPTY.  Being gated it needs NO callee to
//          exist -- no stub, declaration or header was added for it.
//  SHAPE : same pair shape as csystem.cpp DoTrayFeedProcess / MainProc /
//          DoAllProcess (PT-W6a) golden-verbatim gates.
// ===========================================================================
#if 0 // GOLDEN VERBATIM -- golden atester.cpp:9266-9411 (146 lines).  GATE G-PTk3-CheckAndRecodrEP.  NOT COMPILED: the ACTIVE CheckAndRecodrEP() is the slim body immediately after this #endif.
bool CheckAndRecodrEP(int iArm)                                                 //Steven 20190114 : EP Alarm換位置
{
    AnsiString sBufferT;

    if(IniConfig.bD26EnableEPLog==true)                                         //Ifor 20150818 EP Log 程式修改
    {
        MyForceDirectories(asEPLogPath);
        sBufferT.sprintf("%s\\%04d%02d\\", asEPLogPath, SystemYear, SystemMonth);
        MyForceDirectories(sBufferT);
        String EPDate=Now().FormatString("yyyy-mm-dd");
        String EPName=sBufferT+EPDate+".csv";                                   //kevin 20160106
        String NowEPData="";

        String sHisiFileName="";                                                // kevin 20191210 add
        AnsiString sEPCopyPath="",sEPCopyFileName="";                           //kevin 20191210 add
        sEPCopyPath=asEPLogPath+"\\Current\\";
        sEPCopyFileName = sEPCopyPath;
        if(CUSTOMER_CODE==CC_ASE_KaohSiung)                                     //kevin 20191210 add HISI log file name
        {
            beKeepLotNumber();                                                  //kevin 20191018 ASE KH read lot ID LOG
            sHisiFileName   =sBufferT +"ASE_EP_"+sHandleID+"_"+sInsertion+"_"+sDeviceType+"_"+sTestProgram+"_"+sHiLotID+"_"+sOSATLotID+"_"+ GetDateInfoByString() + ".csv";
            sEPCopyFileName =sEPCopyFileName + "ASE_EP_"+sHandleID+"_"+sInsertion+"_"+sDeviceType+"_"+sTestProgram+"_"+sHiLotID+"_"+sOSATLotID+"_"+ GetDateInfoByString() + ".csv";
        }

        if(!FileExists(EPName))
        {
            NowEPData = NowEPData+
            "Time"      +","+
            "Index"     +","+
            "Setting"   +","+
            "Kpa"       +","+
            "Kg"        +","+
            "Alarm";
            WriteDataToFile(EPName, NowEPData);                                 //Steven 20160604 : add protect of fopen
        }

        if(CosFunction.bHiSiliconFunction==true &&                              //kevin 20191212 add HISI LOG
           CUSTOMER_CODE==CC_ASE_KaohSiung)
        {
            if(sHisiFileName!="" && !FileExists(sHisiFileName))
            {
                NowEPData ="Ver:"+asHandlerVersion+"\n";
                NowEPData = NowEPData+
                "Time"      +","+
                "Index"     +","+
                "Setting"   +","+
                "Kpa"       +","+
                "Kg"        +","+
                "Alarm";
                NowEPData=NowEPData+",Lot ID ,OSAT Lot number " ;               //kevin 20191210 add
                if(CUSTOMER_CODE==CC_ASE_KaohSiung)                             //kevin 20191210 add HISI log file name
                {
                    Del_Tree(sEPCopyPath);
                    bBuildFilter(sEPCopyPath,asEPLogPath);
                }
                WriteDataToFile(sHisiFileName, NowEPData);                      //Steven 20160604 : add protect of fopen
            }
        }

        if((TestIF_File.iShuttleMode==1 && TestIF_File.iShuttle_Sel==iArm) || TestIF_File.iShuttleMode==0)
        {
            bool bAdamAlarm=false, bAdamEPAlarm[2]={false, false};              //jou 20170413 (Steven) : Read Adam EP 提升UPH

            ADAM_Rang(IniConfig.iD26EPEncoderRange);
            if(EP_Install==5)
            {
                if(TestIF_File.iShuttleMode==1 && TestIF_File.iShuttle_Sel==iArm)
                {
                    bAdamEPAlarm[0]=ADAM_Alarm(iArm);
                    bAdamEPAlarm[1]=false;
                }
                else
                {
                    bAdamEPAlarm[0]=ADAM_Alarm(0);
                    bAdamEPAlarm[1]=ADAM_Alarm(1);
                }
            }
            else
            {
                bAdamEPAlarm[0]=ADAM_Alarm();                                   //JerryYang 20171024 (wei) add 單顆浮動頭誤差範圍,依照Mars定義給海思的資料
                bAdamEPAlarm[1]=false;
            }

            if(bAdamEPAlarm[0]==false && bAdamEPAlarm[1]==false)
                bAdamAlarm=false;
            else
                bAdamAlarm=true;

            NowEPData=GetDateInfoByString("/")+" "+GetOnlyTimeInfoByString(":");
            if(iArm==0)
            {
                NowEPData +=",1,";                                              //Index 1
            }
            else
            {
                NowEPData +=",2,";                                              //Index 2
            }

            NowEPData +=fContact->edSetKg->Text+",";

            AnsiString StrData = IntToStr(iReadAdamEP);
            fContact->lblEPValueKpa->Caption = StrData;                         //Ifor 20160303 Add Test Arm2 EP pa Value
            NowEPData +=  StrData+",";

            double dbTransferKg=KpaTransferKG(iReadAdamEP)*dfComplianceUnit;    //kevin 20200313 add  dfComplianceUnit
            StrData = FormatFloat("0.0000", dbTransferKg);
            fContact->lblEPValueKg->Caption = StrData;                          //Ifor 20160303 Add Test Arm2 EP Kg Value
            NowEPData +=  StrData+",";

            if(bAdamAlarm==true)
            {
                NowEPData +="Fail";                                             //kevin 20200120
            }
            else
            {
                NowEPData +="Pass";
            }

            if(CUSTOMER_CODE==CC_ASE_KaohSiung)
            {
                beKeepLotNumber();                                              //kevin 20191018 ASE KH lot ID LOG
                NowEPData = NowEPData+","+sHisiAddTempLog(false);               //kevin 20191230 ASE KH HIS add Lot LOG EP
                WriteDataToFile(sHisiFileName.c_str(), NowEPData.c_str());
            }
            WriteDataToFile(EPName, NowEPData);                                 //Steven 20160604 : add protect of fopen
            if(CosFunction.bHiSiliconFunction &&
               CUSTOMER_CODE==CC_ASE_KaohSiung)                                 //kevin 201912010  copy 海思檔案到上傳路徑
            {
                MyForceDirectories(sEPCopyPath);
                CopyFile(sHisiFileName.c_str(), sEPCopyFileName.c_str(), FALSE);                                        //kevin 20191107 add
            }

            if(IniConfig.bD24EnableEPCheckFuntion ||                            //Steven 20240516 : 換位置, 先紀錄再alarm
               IniConfig.bD26EnableEPEncoderRange)                              //kevin 20200803 add EP alarm
            {
                if(bAdamAlarm &&
                   SystemStart==true)                                           //Steven 20220914 : 增加判斷,造免Alarm卡死
                {
                    ShowErrorMessage("WAR1605", K_RETRY, MMSystem, 0, "CheckAndRecodrEP");                              //"請檢查EP是否漏氣!" //jou 20171120 (Steven) : 修正EP alarm位置錯誤
                    return true;
                }
            }
        }
    }
    return false;
}
#endif // GOLDEN VERBATIM -- golden atester.cpp:9266-9411  (GATE G-PTk3-CheckAndRecodrEP, end)
bool CheckAndRecodrEP(int iArm)                                                 //Steven 20190114 : EP Alarm換位置
{
#if 0 // TODO(W7) -- golden :9266-9412 (EP DAQ record/alarm)
#endif
    (void)iArm;
    return false;                                                              // golden default
}
//------------------------------------------------------------------------------
//  CheckAndRecodrTorque (golden :9413-9464) / NewCheckAndRecodeTorque (:9465-
//  9684) -- index arm torque record/alarm.
//------------------------------------------------------------------------------
// ===========================================================================
//  GOLDEN VERBATIM PAIR -- CheckAndRecodrTorque
//  GATED : golden atester.cpp:9413-9463 (51 lines), inert reference text.
//  LIVE  : the slim CheckAndRecodrTorque() immediately AFTER the #endif below.  It is
//          UNCHANGED by this wave -- net behaviour change is ZERO.
//  WHY   : the census scored this "translated" because a same-named LIVE body
//          existed, without comparing SIZE.  Golden's 51 lines were NOWHERE in
//          the tree -- lost text, not deferred behaviour.  Now the text EXISTS
//          and is auditable, so a later un-gate is mechanical.
//  RULES : nothing inside the gate is fixed, renamed, reflowed or reindented;
//          golden's own defects and misspellings are preserved so a diff
//          against golden stays EMPTY.  Being gated it needs NO callee to
//          exist -- no stub, declaration or header was added for it.
//  SHAPE : same pair shape as csystem.cpp DoTrayFeedProcess / MainProc /
//          DoAllProcess (PT-W6a) golden-verbatim gates.
// ===========================================================================
#if 0 // GOLDEN VERBATIM -- golden atester.cpp:9413-9463 (51 lines).  GATE G-PTk3-CheckAndRecodrTorque.  NOT COMPILED: the ACTIVE CheckAndRecodrTorque() is the slim body immediately after this #endif.
bool CheckAndRecodrTorque(int iArm)                                             //kevin 20201027 Arm  扭力log
{
    AnsiString sBufferT;
    //Ifor 20150818 EP Log 程式修改    kevin 20191211
    if(IniConfig.bD26EnableEPLog==true)
    {
        MyForceDirectories(asTorqLogPath);
        sBufferT.sprintf("%s\\%04d%02d\\", asTorqLogPath, SystemYear, SystemMonth);                                     //kevin 20160106 start
        MyForceDirectories(sBufferT);
        String EPDate=Now().FormatString("yyyy-mm-dd");
        String EPName=sBufferT+EPDate+".csv";                                   //kevin 20160106
        String NowEPData="";

        if(!FileExists(EPName))
        {
            NowEPData = NowEPData+
            "Time"      +","+
            "Index"     +","+
            "Torque";
            WriteDataToFile(EPName, NowEPData);                                 //Steven 20160604 : add protect of fopen
        }

        if((TestIF_File.iShuttleMode==1 && TestIF_File.iShuttle_Sel==iArm) || TestIF_File.iShuttleMode==0)
        {
            NowEPData=GetDateInfoByString("/")+" "+GetOnlyTimeInfoByString(":");
            if(iArm==0)
            {
                NowEPData +=",1,";                                              //Index 1
                sBufferT.sprintf("%s%s",NowEPData,fMain->lbArm0Torque->Caption);
            }
            else
            {
                NowEPData +=",2,";                                              //Index 2
                sBufferT.sprintf("%s%s",NowEPData,fMain->lbArm1Torque->Caption);
            }
            /*
            if(bAdamAlarm==true)
            {
                //NowEPData +="Alarm";
                NowEPData +="Fail";                                             //kevin 20200120
            }
            else
            {
                NowEPData +="Pass";
            } */

            WriteDataToFile(EPName, sBufferT);                                  //Steven 20160604 : add protect of fopen
        }
    }
    return false;
}
#endif // GOLDEN VERBATIM -- golden atester.cpp:9413-9463  (GATE G-PTk3-CheckAndRecodrTorque, end)
bool CheckAndRecodrTorque(int iArm)                                             //kevin 20201027 Arm  扭力log
{
#if 0 // TODO(W7) -- golden :9413-9464 (torque record/alarm, COM2)
#endif
    (void)iArm;
    return false;                                                              // golden default
}
//------------------------------------------------------------------------------
// ===========================================================================
//  GOLDEN VERBATIM PAIR -- NewCheckAndRecodeTorque
//  GATED : golden atester.cpp:9465-9683 (219 lines), inert reference text.
//  LIVE  : the slim NewCheckAndRecodeTorque() immediately AFTER the #endif below.  It is
//          UNCHANGED by this wave -- net behaviour change is ZERO.
//  WHY   : the census scored this "translated" because a same-named LIVE body
//          existed, without comparing SIZE.  Golden's 219 lines were NOWHERE in
//          the tree -- lost text, not deferred behaviour.  Now the text EXISTS
//          and is auditable, so a later un-gate is mechanical.
//  RULES : nothing inside the gate is fixed, renamed, reflowed or reindented;
//          golden's own defects and misspellings are preserved so a diff
//          against golden stays EMPTY.  Being gated it needs NO callee to
//          exist -- no stub, declaration or header was added for it.
//  SHAPE : same pair shape as csystem.cpp DoTrayFeedProcess / MainProc /
//          DoAllProcess (PT-W6a) golden-verbatim gates.
// ===========================================================================
#if 0 // GOLDEN VERBATIM -- golden atester.cpp:9465-9683 (219 lines).  GATE G-PTk4-NewCheckAndRecodeTorque.  NOT COMPILED: the ACTIVE NewCheckAndRecodeTorque() is the slim body immediately after this #endif.
bool NewCheckAndRecodeTorque(int iArm)                                          //KaiHuang 20201222
{
    static bool bFirstRun=true;
    static double dDelayTime_Backup;

    //Delay Time 變更需要重設標準值
    if(bFirstRun==true)
    {
        bFirstRun=false;
        dDelayTime_Backup=TestIF_File.dReadTorqueDelayTime;                     //kevin 20210804 等待時間去讀取扭力 change by setup
    }
    else
    {
        if(TestIF_File.dReadTorqueDelayTime!=dDelayTime_Backup)                 //kevin 20210804 等待時間去讀取扭力 change by setup
        {
            dDelayTime_Backup=TestIF_File.dReadTorqueDelayTime;                 //kevin 20210804 等待時間去讀取扭力 change by setup
            bResetArm1Value=true;
            bResetArm2Value=true;
        }
    }

    if(iArm==0)
    {
        if(FTestSuck.UseSiteHasIC()==true)
        {
            if(fMain->edTorue0->Text=="")                                       //還沒收到扭力值  kevin 20210118
            {
                if(fMain->chkReadTorque1->Checked==true)                        //還在讀取中
                {
                    return false;
                }
                else
                {
                    //Torque Delay 還沒跑完
                    #ifdef SOFT_SIMULTE
                        if(bReadArm1_Torque==true)
                        {
                            bReadArm1_Torque=false;
                            fMain->chkReadTorque1->Checked=false;
                            fMain->edTorue0->Text="1.3";
                            return false;
                        }
                    #else
                        if(bReadArm1_Torque==true)
                        {
                            bReadArm1_Torque=false;
                            fMain->chkReadTorque1->Checked=true;
                            return false;
                        }
                    #endif
                }
            }
            else
            {
                if(bResetArm1Value==true)
                {
                    if(iReadTorqueError==0)
                    {
                        bResetArm1Value=false;
                        //dSetArm1TorqueValue=(double)StrToFloat(fMain->edTorue0->Text.c_str());  //kevin 20210526 mark
                    }
                    else                                                        //讀取有錯誤不能當標準值
                    {
                        ;
                    }
                    //SaveTorque(1, "Arm1", fMain->edTorue0->Text.c_str(), IntToStr(iReadTorqueError)); //kevin 20210118
                    bResetRecordSetArm1Value=true;                              //kevin 20210118
                    return true;
                }
                else
                {
                    if(bResetRecordSetArm1Value)                                //kevin 20210118 : Reset 標準值)
                    {
                        dSetArm1TorqueValue=(double)StrToFloat(fMain->edTorue0->Text.c_str());

                        if(dSetArm1TorqueValue==0.0)                            //kevin 20210303 扭力 = 0 不記錄
                        {
                            bResetRecordSetArm1Value=true;                      //kevin 20210223 change SET Value Return
                        }
                        else
                        {
                            bResetRecordSetArm1Value=false;
                            sSetTorquValue[0]=fMain->edTorue0->Text;            //kevin 20210421 扭力設定讀取值
                            SaveTorque(1, "Arm1", fMain->edTorue0->Text.c_str(), IntToStr(iReadTorqueError),sSetTorquValue[0]);                                 //kevin 20210118
                        }
                    }
                    else
                    {
                        SaveTorque(2, "Arm1", fMain->edTorue0->Text.c_str(), IntToStr(iReadTorqueError),sSetTorquValue[0]);
                    }

                    double dValue=fabs(dSetArm1TorqueValue-(double)StrToFloat(fMain->edTorue0->Text.c_str()));
                    AnsiString strError="";
                    if(iReadTorqueError==0)
                    {
                        if(dValue>TestIF_File.dReadTorque)                      //kevin 20210804 等待時間去讀取扭力 change by setup
                        {
                            iSetTorqueAlarm[0]++;                               //kevin 20210505 扭力連續幾次ALARM
                            if(iSetTorqueAlarm[0]>1)
                            {
                                iSetTorqueAlarm[0]=0;
                                strError.sprintf("Now:%.1f > Set:%.1f", dValue, TestIF_File.dReadTorque);               //kevin 20210804 等待時間去讀取扭力 change by setup
                                ShowErrorMessage("WAR0361", K_SKIP, MTestZ1, false, strError);                          //Steven 20211123 : WAR0370 --> WAR0361
                            }
                        }
                        else
                        {
                            iSetTorqueAlarm[0]=0;
                        }
                    }
                    return true;
                }
            }
        }
        else
        {
            return true;
        }
    }
    else
    {
        if(BTestSuck.UseSiteHasIC()==true)
        {
            if(fMain->edTorue1->Text=="")                                       //已經測完了還沒收到扭力值
            {
                if(fMain->chkReadTorque2->Checked==true)                        //還在讀取中
                {
                    return false;
                }
                else
                {
                    //測試結果回傳了, Torque Delay 還沒跑完
                    #ifdef SOFT_SIMULTE
                        if(bReadArm2_Torque==true)
                        {
                            bReadArm2_Torque=false;
                            fMain->chkReadTorque2->Checked=false;
                            fMain->edTorue1->Text="2.3";
                            return false;
                        }
                    #else
                        if(bReadArm2_Torque==true)
                        {
                            bReadArm2_Torque=false;
                            fMain->chkReadTorque2->Checked=true;
                            return false;
                        }
                    #endif
                }
            }
            else
            {
                if(bResetArm2Value==true)
                {
                    if(iReadTorqueError==0)
                    {
                        bResetArm2Value=false;
                        //dSetArm2TorqueValue=(double)StrToFloat(fMain->edTorue1->Text.c_str());  //kevin 20210118 mark
                    }
                    else                                                        //讀取有錯誤不能當標準值
                    {
                        ;
                    }
                    bResetRecordSetArm2Value = true;                            //kevin 20210118 : Reset 標準值
                    //SaveTorque(1, "Arm2", fMain->edTorue1->Text.c_str(), IntToStr(iReadTorqueError));  kevin 20210118
                    return true;
                }
                else
                {
                    if(bResetRecordSetArm2Value)                                //kevin 20210118 : Reset 標準值)
                    {
                        dSetArm2TorqueValue=(double)StrToFloat(fMain->edTorue1->Text.c_str());

                        if(dSetArm2TorqueValue==0.0)                            //kevin 20210303 扭力 = 0 不記錄
                        {
                            bResetRecordSetArm2Value=true;                      //kevin 20210223 change SET Value Return
                        }
                        else
                        {
                            bResetRecordSetArm2Value = false;
                            sSetTorquValue[1]=fMain->edTorue1->Text;            //kevin 20210421 扭力設定讀取值
                            SaveTorque(1, "Arm2", fMain->edTorue1->Text.c_str(), IntToStr(iReadTorqueError),sSetTorquValue[1]);                                 //kevin 20210118
                        }
                    }
                    else
                    {
                        SaveTorque(2, "Arm2", fMain->edTorue1->Text.c_str(), IntToStr(iReadTorqueError),sSetTorquValue[1]);
                    }

                    double dValue=fabs(dSetArm2TorqueValue-(double)StrToFloat(fMain->edTorue1->Text.c_str()));
                    AnsiString strError="";
                    if(iReadTorqueError==0)
                    {
                        if(dValue>TestIF_File.dReadTorque)                      //kevin 20210804 等待時間去讀取扭力 change by setup
                        {
                            iSetTorqueAlarm[1]++;                               //kevin 20210505 扭力連續幾次ALARM
                            if(iSetTorqueAlarm[1]>1)
                            {
                                iSetTorqueAlarm[1]=0;
                                strError.sprintf("Now:%.1f > Set:%.1f", dValue, TestIF_File.dReadTorque);               //kevin 20210804 等待時間去讀取扭力 change by setup
                                ShowErrorMessage("WAR0362", K_SKIP, MTestZ2, false, strError);                          //Steven 20211123 : WAR0371 --> WAR0362
                            }
                        }
                        else
                        {
                            iSetTorqueAlarm[1]=0;                               //kevin 20210505 扭力連續幾次ALARM
                        }
                    }
                    return true;
                }
            }
        }
        else
        {
            return true;
        }
    }
    return false;
}
#endif // GOLDEN VERBATIM -- golden atester.cpp:9465-9683  (GATE G-PTk4-NewCheckAndRecodeTorque, end)
bool NewCheckAndRecodeTorque(int iArm)                                          //KaiHuang 20201222
{
#if 0 // TODO(W7) -- golden :9465-9684 (torque record/alarm SM, COM2)
#endif
    (void)iArm;
    return false;                                                              // golden default
}
//------------------------------------------------------------------------------
//  SaveTorque (golden :9685-9790) -- write torque log file.
//------------------------------------------------------------------------------
void SaveTorque(int iPosition, AnsiString asArm, AnsiString asValue, AnsiString asAlarmCount, AnsiString asSetValue)    //KaiHuang 20201222 Add : For ASE 高雄 扭力值存Log
{
#if 0 // TODO(W7) -- golden :9685-9790 (torque log file write)
#endif
    (void)iPosition; (void)asArm; (void)asValue; (void)asAlarmCount; (void)asSetValue;
}
//------------------------------------------------------------------------------
int GetSocketCheckPos(int IndexPos)                                             //Steven 20140620 : 整合為Function
{
    int iIndexUpPos=0;

    if(IndexPos==0)                                                             //Steven 20160303 : Fixed for close arm offset will crash
    {
        iIndexUpPos=0;
    }
    else if(CosFunction.bIndexZDownToAboveSocket==true)
    {
        iIndexUpPos=IniConfig.dD41SocketInitialCheckOffset*100;
    }
    else if((DeviceForm.iSocketInitialICCheckPosition==1 && IniConfig.bTestIcCheckInContact==true) ||
            (IniConfig.iD41SocketInitialICCheckPosition==1 && IniConfig.bTestIcCheckInContact==false))                  //Above Socket
    {
        if(IniConfig.bTestIcCheckInContact==true)
            iIndexUpPos=DeviceForm.fSocketInitialICCheckPositionOffset*100;     //Steven 210100818
        else
            iIndexUpPos=IniConfig.fIndexCheckOffset*100;                        //Steven 210100818  //Jimmychiu 20230922 : fixed D41 offset
    }
    return iIndexUpPos;
}
//------------------------------------------------------------------------------
int GetIndexZSpeed(int index)                                                   //Steven 20160524 : Index Z軸速度整合為Function
{
    int iScale, sp=1000;
    if(IniConfig.bD54SlowDown)
    {
        if(IniConfig.iD54SlowDownScale<1)
            iScale=1;
        else
            iScale=IniConfig.iD54SlowDownScale;
        sp=MOT[MTestZ1+index].Motor->PJogHighSpeed*iScale/100;
    }
    else
    {
        sp=MOT[MTestZ1+index].GailSpeed;
    }
    return sp;
}
//------------------------------------------------------------------------------
void IndexAddSpeedDisplay()                                                     //KaiChen 20171225 (Steven)：Add Speed Display
{
#if 0 // TODO(W7) -- golden :9834-9839 (fMain speed-display UI)
#endif
}
//------------------------------------------------------------------------------
void IndexSubSpeedDisplay()                                                     //KaiChen 20171225 (Steven)：Add Speed Display
{
#if 0 // TODO(W7) -- golden :9840-9848 (fMain speed-display UI)
#endif
}
//------------------------------------------------------------------------------
//  IndexCheckOneByOne (golden :9849-9943) / IndexCheck4Site (:9944-10061) --
//  one-by-one / 4-site index-check SMs (MOT[] Z/Suck/Socket).
//------------------------------------------------------------------------------
// ===========================================================================
//  GOLDEN VERBATIM PAIR -- IndexCheckOneByOne
//  GATED : golden atester.cpp:9849-9939 (91 lines), inert reference text.
//  LIVE  : the slim IndexCheckOneByOne() immediately AFTER the #endif below.  It is
//          UNCHANGED by this wave -- net behaviour change is ZERO.
//  WHY   : the census scored this "translated" because a same-named LIVE body
//          existed, without comparing SIZE.  Golden's 91 lines were NOWHERE in
//          the tree -- lost text, not deferred behaviour.  Now the text EXISTS
//          and is auditable, so a later un-gate is mechanical.
//  RULES : nothing inside the gate is fixed, renamed, reflowed or reindented;
//          golden's own defects and misspellings are preserved so a diff
//          against golden stays EMPTY.  Being gated it needs NO callee to
//          exist -- no stub, declaration or header was added for it.
//  SHAPE : same pair shape as csystem.cpp DoTrayFeedProcess / MainProc /
//          DoAllProcess (PT-W6a) golden-verbatim gates.
// ===========================================================================
#if 0 // GOLDEN VERBATIM -- golden atester.cpp:9849-9939 (91 lines).  GATE G-PTk4-IndexCheckOneByOne.  NOT COMPILED: the ACTIVE IndexCheckOneByOne() is the slim body immediately after this #endif.
bool IndexCheckOneByOne(bool bReset, int iWhichArm, int iSiteCount, bool bNeedUpCheck, bool bIsAboveCalibrate)          //Ifor 20200617 : add Use One By One Index Check Function 整合
{
    static AnsiString ErrPart="";

    int &iTask=iIndexCheckinitial;
    int iWhichRow=0, iWhichCol=0;
    bool flag2=true;
    bool bSuckOK=false;

    if(bReset)
    {
        iTask=1;
        TotalErrPart="";
        return false;
    }

    if(iSiteCount<TestSocket.iShtCol)
    {
        iWhichRow=0;
        iWhichCol=iSiteCount;
    }
    else
    {
        iWhichRow=1;
        iWhichCol=iSiteCount-TestSocket.iShtCol;
    }

    switch(iTask)
    {
        case 1:
            bIndexCheckNoStopVaccum=true;                                       //KaiChen 20210104:
            fiosetview->bIndexSuck[iWhichArm][iWhichRow][iWhichCol]=true;
            DoTestHeadMotorDelay.SetSecAndOn(0.5);
            iTask=2;
            break;
        case 2:                                                                 //jou 2012-05-09 Index Check時,如果中途按暫停,也要繼續把suck()做完,避免負壓掉ic
            if(INDEX_SUCKER_TYPE==1)
            {
                if(iWhichArm==0)
                    bSuckOK=fiosetview->ProcessIndexSuckDestroy1();
                else
                    bSuckOK=fiosetview->ProcessIndexSuckDestroy2();
            }

            if(DoTestHeadMotorDelay.Off() && bSuckOK==true)
            {
                iTask=3;
                if(bNeedUpCheck==true)                                          //Above 模式檢查直接離開 上升後才檢查
                {
                    return true;
                }
            }
            break;
        case 3:
            ErrPart=" ";
            flag2=false;

            if(bIsAboveCalibrate==true)                                         //JerryYang 20250120 : modify
            {
                if(iWhichArm==0)
                    FTestSuck.CheckVaccumIsON_AboveSocket(iWhichRow, iWhichCol, flag2);
                else
                    BTestSuck.CheckVaccumIsON_AboveSocket(iWhichRow, iWhichCol, flag2);
            }
            else
            {
                if(iWhichArm==0)
                    FTestSuck.CheckVaccumIsIniaialON(iWhichRow, iWhichCol, flag2);
                else
                    BTestSuck.CheckVaccumIsIniaialON(iWhichRow, iWhichCol, flag2);
            }

            bIndexCheckNoStopVaccum=false;                                      //KaiChen 20210104:

            if(flag2==true)
            {
                if(iWhichArm==0)
                {
                    ErrPart+=IndexSuckName[iWhichRow+IsNNMode()][iWhichCol];
                }
                else
                {
                    ErrPart+=IndexSuckName[iWhichRow][iWhichCol];
                }
                TotalErrPart+= ErrPart;
            }
            iTask=1;
            return true;
    }
    return false;
}
#endif // GOLDEN VERBATIM -- golden atester.cpp:9849-9939  (GATE G-PTk4-IndexCheckOneByOne, end)
bool IndexCheckOneByOne(bool bReset, int iWhichArm, int iSiteCount, bool bNeedUpCheck, bool bIsAboveCalibrate)          //Ifor 20200617 : add Use One By One Index Check Function 整合
{
#if 0 // TODO(W7) -- golden :9849-9943 (one-by-one index-check SM)
#endif
    (void)bReset; (void)iWhichArm; (void)iSiteCount; (void)bNeedUpCheck; (void)bIsAboveCalibrate;
    return false;                                                              // golden default
}
//------------------------------------------------------------------------------
// ===========================================================================
//  GOLDEN VERBATIM PAIR -- IndexCheck4Site
//  GATED : golden atester.cpp:9944-10060 (117 lines), inert reference text.
//  LIVE  : the slim IndexCheck4Site() immediately AFTER the #endif below.  It is
//          UNCHANGED by this wave -- net behaviour change is ZERO.
//  WHY   : the census scored this "translated" because a same-named LIVE body
//          existed, without comparing SIZE.  Golden's 117 lines were NOWHERE in
//          the tree -- lost text, not deferred behaviour.  Now the text EXISTS
//          and is auditable, so a later un-gate is mechanical.
//  RULES : nothing inside the gate is fixed, renamed, reflowed or reindented;
//          golden's own defects and misspellings are preserved so a diff
//          against golden stays EMPTY.  Being gated it needs NO callee to
//          exist -- no stub, declaration or header was added for it.
//  SHAPE : same pair shape as csystem.cpp DoTrayFeedProcess / MainProc /
//          DoAllProcess (PT-W6a) golden-verbatim gates.
// ===========================================================================
#if 0 // GOLDEN VERBATIM -- golden atester.cpp:9944-10060 (117 lines).  GATE G-PTk3-IndexCheck4Site.  NOT COMPILED: the ACTIVE IndexCheck4Site() is the slim body immediately after this #endif.
bool IndexCheck4Site(bool bReset,int iWhichArm, int iSiteCount)
{
    int &iTask=Taskinitial;
    static AnsiString ErrPart="";

    int iSuckcount=iSiteCount*2;
    int iTotal=TestSocket.iShtCnt;
    bool flag=false, flag2=true, bNeedSuck=false;                               //JerryYang 20250120 : modify

    if(bReset)
    {
        iTask=1;
        return false;
    }

    if(iSuckcount>=iTotal)
        iSuckcount=iTotal;

    switch(iTask)
    {
        case 1:
            if(iWhichArm==0)
            {
                bNeedSuck=false;
                for(int i=iSuckcount; i<iSuckcount+2; i++)                      //JerryYang 20250120 : modify
                {
                    if(FTestSuck.bNeedCheck[0][i]==true || FTestSuck.bNeedCheck[1][i]==true)
                    {
                        bNeedSuck=true;
                    }
                }

                if(bNeedSuck)
                {
                    for(int i=iSuckcount; i<iSuckcount+2; i++)
                    {
                        fiosetview->bIndexSuck[0][0][i]=true;
                        fiosetview->bIndexSuck[0][1][i]=true;
                    }
                }
                else
                {
                    return true;
                }
            }
            else if(iWhichArm==1)
            {
                bNeedSuck=false;
                for(int i=iSuckcount; i<iSuckcount+2; i++)                      //JerryYang 20250120 : modify
                {
                    if(BTestSuck.bNeedCheck[0][i]==true || BTestSuck.bNeedCheck[1][i]==true)
                    {
                        bNeedSuck=true;
                    }
                }

                if(bNeedSuck)
                {
                    for(int i=iSuckcount; i<iSuckcount+2; i++)
                    {
                        fiosetview->bIndexSuck[1][0][i]=true;
                        fiosetview->bIndexSuck[1][1][i]=true;
                    }
                }
                else
                {
                    return true;
                }
            }
            DoTestHeadMotorDelay.SetMSAndOn(IniConfig.iD44TestHeadCheckVacuumTime);
            iTask=2;
            break;
        case 2:                                                                 //jou 2012-05-09 Index Check時,如果中途按暫停,也要繼續把suck()做完,避免負壓掉ic
            if(INDEX_SUCKER_TYPE==1)
            {
                if(iWhichArm==0)
                    fiosetview->ProcessIndexSuckDestroy1();
                else
                    fiosetview->ProcessIndexSuckDestroy2();
            }

            if(DoTestHeadMotorDelay.Off())
            {
                bIndexCheckNoStopVaccum=false;                                  //jou 2012-05-09 Index Check時,如果中途按暫停,也要繼續把suck()做完,避免負壓掉ic
                flag=false;
                ErrPart=" ";
                for(int i=0; i<FTestSuck.iShtRow; i++)
                {
                    for(int j=0; j<FTestSuck.iShtCol; j++)
                    {
                        flag2=false;
                        if(iWhichArm==0)
                            FTestSuck.CheckVaccumIsIniaialON(i, j, flag2);
                        else
                            BTestSuck.CheckVaccumIsIniaialON(i, j, flag2);

                        if(flag2==true)
                        {
                            flag=true;
                            ErrPart+=IndexSuckName[i][j];
                            TotalErrPart+=ErrPart;
                        }
                    }
                }

                if(flag)                                                        //kevin 20190819 add
                    TotalErrPart+=ErrPart;

                iTask=3;
            }
            break;
        case 3:
            iTask=1;
            return true;
    }
    return false;
}
#endif // GOLDEN VERBATIM -- golden atester.cpp:9944-10060  (GATE G-PTk3-IndexCheck4Site, end)
bool IndexCheck4Site(bool bReset,int iWhichArm, int iSiteCount)                 //kevin 20190530 index check
{
#if 0 // TODO(W7) -- golden :9944-10061 (4-site index-check SM)
#endif
    (void)bReset; (void)iWhichArm; (void)iSiteCount;
    return false;                                                              // golden default
}
//------------------------------------------------------------------------------
//  CheckInitialStartDelayInSocket (golden :10062-10297) / NeedResetInitialDelay
//  (:10298-10376) / CheckToBoostIndexTemp (:10377-10443).
//------------------------------------------------------------------------------
// ===========================================================================
//  GOLDEN VERBATIM PAIR -- CheckInitialStartDelayInSocket
//  GATED : golden atester.cpp:10062-10296 (235 lines), inert reference text.
//  LIVE  : the slim CheckInitialStartDelayInSocket() immediately AFTER the #endif below.  It is
//          UNCHANGED by this wave -- net behaviour change is ZERO.
//  WHY   : the census scored this "translated" because a same-named LIVE body
//          existed, without comparing SIZE.  Golden's 235 lines were NOWHERE in
//          the tree -- lost text, not deferred behaviour.  Now the text EXISTS
//          and is auditable, so a later un-gate is mechanical.
//  RULES : nothing inside the gate is fixed, renamed, reflowed or reindented;
//          golden's own defects and misspellings are preserved so a diff
//          against golden stays EMPTY.  Being gated it needs NO callee to
//          exist -- no stub, declaration or header was added for it.
//  SHAPE : same pair shape as csystem.cpp DoTrayFeedProcess / MainProc /
//          DoAllProcess (PT-W6a) golden-verbatim gates.
// ===========================================================================
#if 0 // GOLDEN VERBATIM -- golden atester.cpp:10062-10296 (235 lines).  GATE G-PTk3-CheckInitialStartDelayInSocket.  NOT COMPILED: the ACTIVE CheckInitialStartDelayInSocket() is the slim body immediately after this #endif.
void CheckInitialStartDelayInSocket()                                           //JerryYang 20180629 (wei) Initial delay判斷包成函式
{
    static int iCountError=0;
    int iTempMaxWaitTime=0;                                                     //kevin 20160906 溫度加熱最大等待時間  //JerryYang 20180828 修正delay time錯誤問題, static int -> int
    double TestIntervals=0;                                                     //kevin 20160401 取得測試機間隔時間
    AnsiString sBuffer="";                                                      //kevin 20160414

    if(bUseInitDelay)                                                           //Steven 20171219 : 只有加熱模式要跑Initial Delay //Ifor 20180109 (Steven) : add KYEC 常高溫都要跑Initial Delay
    {
        if(CosFunction.bHiSiliconFunction==true)                                //kevin 20200110 add initial
        {
            if(bHISIInitiayDelay)                                               //kevin 20200110 add 海司強至initial delay
            {
                Prod.iInitialDelay=TestIF.dInitStartDelayTime;
                bNeedInitialTestDelay=true;
                iTempMaxWaitTime=Prod.iInitialDelay;
                bHISIInitiayDelay=false;
                bInitialTestDelayStatus[11]=true;                               //Steven 20221214 : Add 主畫面顯示
                sBuffer.sprintf("%d sec", iTempMaxWaitTime);
                RecordProcess("When onecycle KL use initial delay", sBuffer);
            }
        }

        if(bDoWhenPressStopOverUseInitialDelay)                                 //ChungHung 20150526 add for ATK want to even stop over will use initial delay
        {
            bDoWhenPressStopOverUseInitialDelay=false;
            bNeedInitialTestDelay=true;
            if(IniConfig.bI13InitStartDelayHasFTandRT && iRunStartMode==RT)     //Steven 20190313 : Initial Start Delay use different setting in FT and RT
                Prod.iInitialDelay=TestIF.dInitialDelay_6_RT;
            else
                Prod.iInitialDelay=TestIF.iInitialDelay_6;

            if(Prod.iInitialDelay>=iTempMaxWaitTime)
                iTempMaxWaitTime=Prod.iInitialDelay;                            //kevin 20160906 溫度加熱最大等待時間
            bInitialTestDelayStatus[6]=true;                                    //wei 20171020 (jou) 延遲狀態顯示

            sBuffer.sprintf("%d sec", iTempMaxWaitTime);
            RecordProcess("When press stop time over use initial delay", sBuffer);
        }

        if(bDoAfterAutoCleanFunctionUseInitialDelay)
        {
            bDoAfterAutoCleanFunctionUseInitialDelay=false;
            bNeedInitialTestDelay=true;
            if(IniConfig.bI13InitStartDelayHasFTandRT && iRunStartMode==RT)     //Steven 20190313 : Initial Start Delay use different setting in FT and RT
                Prod.iInitialDelay=TestIF.dInitialDelay_3_RT;
            else
                Prod.iInitialDelay=TestIF.iInitialDelay_3;                      //ChungHung 20141210 add for SCK want to every event have delay

            if(Prod.iInitialDelay>=iTempMaxWaitTime)
                iTempMaxWaitTime=Prod.iInitialDelay;                            //kevin 20160906 溫度加熱最大等待時間
            bInitialTestDelayStatus[3]=true;                                    //wei 20171020 (jou) 延遲狀態顯示

            sBuffer.sprintf("%d sec", iTempMaxWaitTime);
            RecordProcess("After auto clean use initial delay", sBuffer);
        }

        if(bDoAfterShowAlarmMessageUseInitialDelay)
        {
            bDoAfterShowAlarmMessageUseInitialDelay=false;
            bNeedInitialTestDelay=true;
            if(IniConfig.bI13InitStartDelayHasFTandRT && iRunStartMode==RT)     //Steven 20190313 : Initial Start Delay use different setting in FT and RT
                Prod.iInitialDelay=TestIF.dInitialDelay_2_RT;
            else
                Prod.iInitialDelay=TestIF.iInitialDelay_2;

            if(Prod.iInitialDelay>=iTempMaxWaitTime)
                iTempMaxWaitTime=Prod.iInitialDelay;                            //kevin 20160906 溫度加熱最大等待時間
            bInitialTestDelayStatus[2]=true;                                    //wei 20171020 (jou) 延遲狀態顯示

            sBuffer.sprintf("%d sec", iTempMaxWaitTime);
            RecordProcess("After show alarm message use initial delay", sBuffer);
        }

        if(bDoAfterOpenHeatDoorUseInitialDelay)
        {
            bDoAfterOpenHeatDoorUseInitialDelay=false;
            bNeedInitialTestDelay=true;
            if(IniConfig.bI13InitStartDelayHasFTandRT && iRunStartMode==RT)     //Steven 20190313 : Initial Start Delay use different setting in FT and RT
                Prod.iInitialDelay=TestIF.dInitialDelay_5_RT;
            else
                Prod.iInitialDelay=TestIF.iInitialDelay_5;                      //ChungHung 20141210 add for SCK want to every event have delay

            if(Prod.iInitialDelay>=iTempMaxWaitTime)
                iTempMaxWaitTime=Prod.iInitialDelay;                            //kevin 20160906 溫度加熱最大等待時間
            bInitialTestDelayStatus[4]=true;                                    //wei 20171020 (jou) 延遲狀態顯示

            sBuffer.sprintf("%d sec", iTempMaxWaitTime);
            RecordProcess("After open heat door use initial delay", sBuffer);
        }

        if(bDoWhenHappenTestedTimeBlowUseInitialDelay)
        {
            bDoWhenHappenTestedTimeBlowUseInitialDelay=false;
            bNeedInitialTestDelay=true;
            if(IniConfig.bI13InitStartDelayHasFTandRT && iRunStartMode==RT)     //Steven 20190313 : Initial Start Delay use different setting in FT and RT
                Prod.iInitialDelay=TestIF.dInitialDelay_4_RT;
            else
                Prod.iInitialDelay=TestIF.iInitialDelay_4;                      //ChungHung 20141210 add for SCK want to every event have delay

            if(Prod.iInitialDelay>=iTempMaxWaitTime)
                iTempMaxWaitTime=Prod.iInitialDelay;                            //kevin 20160906 溫度加熱最大等待時間
            bInitialTestDelayStatus[5]=true;                                    //wei 20171020 (jou) 延遲狀態顯示

            sBuffer.sprintf("%d sec", iTempMaxWaitTime);
            RecordProcess("When happen tested time blow use initial delay", sBuffer);
        }

        if(bDoEveryFirstDeviceFunctionUseInitialDelay)
        {
            bDoEveryFirstDeviceFunctionUseInitialDelay=false;
            if(bFirstDeviceInitialTestDelayWhichOutAfterAutoClean==false)
            {
                bNeedInitialTestDelay=true;                                     //ChungHung 20140425 add for TSMC Device
                if(IniConfig.bI13InitStartDelayHasFTandRT && iRunStartMode==RT)                                         //Steven 20190313 : Initial Start Delay use different setting in FT and RT
                    Prod.iInitialDelay=TestIF.dInitialDelay_1_RT;
                else
                    Prod.iInitialDelay=TestIF.iInitialDelay;                    //ChungHung 20141210 add for SCK want to every event have delay

                if(Prod.iInitialDelay>=iTempMaxWaitTime)
                    iTempMaxWaitTime=Prod.iInitialDelay;                        //kevin 20160906 溫度加熱最大等待時間
                bInitialTestDelayStatus[1]=true;                                //wei 20171020 (jou) 延遲狀態顯示

                if(CosFunction.bUseInitialDelayAsSoakTime &&                    //Steven 20170511 (wei) : 使用initial delay當 Soak time
                   Temperature.bUseInitialDelayAsSoakTime &&
                   bFirstInputForIndex && iTempMaxWaitTime<Temperature.fSoakTime)
                {
                    iTempMaxWaitTime=Temperature.fSoakTime;
                    bFirstInputForIndex=false;
                }

                sBuffer.sprintf("%d sec", iTempMaxWaitTime);
                RecordProcess("Every fisrt device delay use initial delay", sBuffer);
            }
            bFirstDeviceInitialTestDelayWhichOutAfterAutoClean=false;
        }

        if(bDoWhenNoFullSiteUseInitialDelay)                                    //wei 20151228 No FullSite delay
        {
            bDoWhenNoFullSiteUseInitialDelay=false;
            bNeedInitialTestDelay=true;
            if(IniConfig.bI13InitStartDelayHasFTandRT && iRunStartMode==RT)     //Steven 20190313 : Initial Start Delay use different setting in FT and RT
                Prod.iInitialDelay=TestIF.dInitialDelay_7_RT;
            else
                Prod.iInitialDelay=TestIF.iInitialDelay_7;

            if(Prod.iInitialDelay>=iTempMaxWaitTime)
                iTempMaxWaitTime=Prod.iInitialDelay;                            //kevin 20160906 溫度加熱最大等待時間
            bInitialTestDelayStatus[7]=true;                                    //wei 20171020 (jou) 延遲狀態顯示

            sBuffer.sprintf("%d sec", iTempMaxWaitTime);
            RecordProcess("When no fullSite use initial delay", sBuffer);
        }

        if(bDoOTDOffUseInitialDelay)                                            //Steven 20160818 : OTD打開Delay
        {
            bDoOTDOffUseInitialDelay=false;
            bNeedInitialTestDelay=true;
            if(IniConfig.bI13InitStartDelayHasFTandRT && iRunStartMode==RT)     //Steven 20190313 : Initial Start Delay use different setting in FT and RT
                Prod.iInitialDelay=TestIF.dInitialDelay_9_RT;
            else
                Prod.iInitialDelay=TestIF.iInitialDelay_9;
            if(Prod.iInitialDelay>=iTempMaxWaitTime)
                iTempMaxWaitTime=Prod.iInitialDelay;                            //kevin 20160906 溫度加熱最大等待時間
            bInitialTestDelayStatus[9]=true;                                    //wei 20171020 (jou) 延遲狀態顯示

            sBuffer.sprintf("%d sec", iTempMaxWaitTime);
            RecordProcess("OTD unlock use initial delay", sBuffer);
        }

        if(bTestFinishToNextTestOver)                                           //JerryYang 20181120 (Steven) : (Steven) : EOT 預熱功能bug,先還原成舊的
        {
            if(bFirstTest==false)                                               //kevin 20160311
            {
                TestIntervals=TestIntervalsTime.LatchCycleTime(false)/1000.0;
            }
            else
            {
                bFirstTest=false;
                iCountError=0;                                                  //kevin 20160310 連續幾次發警告
            }

            if(TestIntervals>=TestIF.iTestFinishToNextTestOver)                 //kevin 20160310
            {
                iInitContactCount=0;
                bNeedInitialTestDelay=true;
                if(IniConfig.bI13InitStartDelayHasFTandRT && iRunStartMode==RT)                                         //Steven 20190313 : Initial Start Delay use different setting in FT and RT
                    Prod.iInitialDelay=TestIF.dInitialDelay_8_RT;
                else
                    Prod.iInitialDelay=TestIF.iInitialDelay_8;
                if(Prod.iInitialDelay>=iTempMaxWaitTime)
                    iTempMaxWaitTime=Prod.iInitialDelay;                        //kevin 20160906 溫度加熱最大等待時間
                bTestOverTimeTempOffsetF=true;
                iCountError++;                                                  //kevin 20160310 連續幾次發警告
                bInitialTestDelayStatus[8]=true;                                //wei 20171020 (jou) 延遲狀態顯示

                sBuffer.sprintf("EOT monitor time over delay %0.2f sec", TestIntervals);                                //kevin 20160414 顯示超過時間
                RecordProcess(sBuffer, AnsiString(iTempMaxWaitTime)+AnsiString(" sec"));
            }
            else
            {
                iCountError=0;                                                  //kevin 20160310 連續幾次發警告
            }

            if(iCountError>5)
            {
                iCountError=0;                                                  //kevin 20160310 連續幾次發警告
                ShowErrorMessage("MES0714", K_RETRY, MMSystem);                 //kevin 20160318 EOT monitor time 時間設太短
            }
        }

        if(bTestStartToNextTestStartDelay)                                      //kevin 20181122 add SOT start SRQ41 send next SRQ 41)
        {
            iCountError++;                                                      //kevin 20160310 連續幾次發警告
            SW[SwPurgeAir].Off();                                               //kevin 20180928 add blower load board
            iContractCount=0;
        }
    }

    if(bNeedInitialTestDelay)                                                   //ChungHung 20140425 add for TSMC Device
    {
        if(bDoubleContact)                                                      //ChungHung 20140801 add Korea Want to count down in main status
        {
            Prod.iInitialDelay=TestIF.iInitialDelay;                            //kevin 20160906 溫度加熱最大等待時間
            iInitialCount=Prod.iInitialDelay;
        }
        else
        {
            Prod.iInitialDelay=iTempMaxWaitTime;                                //kevin 20160906 溫度加熱最大等待時間
            iInitialCount=Prod.iInitialDelay;
        }
        dwStartInitialCount.LatchCycleTime(true);
        iInitContactCount=0;                                                    //Steven 20151123 : 起測時溫度要補Offset
    }
}
#endif // GOLDEN VERBATIM -- golden atester.cpp:10062-10296  (GATE G-PTk3-CheckInitialStartDelayInSocket, end)
void CheckInitialStartDelayInSocket()                                           //JerryYang 20180629 (wei) Initial delay判斷包成函式
{
#if 0 // TODO(W7) -- golden :10062-10297 (initial-delay-in-socket SM)
#endif
}
//------------------------------------------------------------------------------
// ===========================================================================
//  GOLDEN VERBATIM PAIR -- NeedResetInitialDelay
//  GATED : golden atester.cpp:10298-10375 (78 lines), inert reference text.
//  LIVE  : the slim NeedResetInitialDelay() immediately AFTER the #endif below.  It is
//          UNCHANGED by this wave -- net behaviour change is ZERO.
//  WHY   : the census scored this "translated" because a same-named LIVE body
//          existed, without comparing SIZE.  Golden's 78 lines were NOWHERE in
//          the tree -- lost text, not deferred behaviour.  Now the text EXISTS
//          and is auditable, so a later un-gate is mechanical.
//  RULES : nothing inside the gate is fixed, renamed, reflowed or reindented;
//          golden's own defects and misspellings are preserved so a diff
//          against golden stays EMPTY.  Being gated it needs NO callee to
//          exist -- no stub, declaration or header was added for it.
//  SHAPE : same pair shape as csystem.cpp DoTrayFeedProcess / MainProc /
//          DoAllProcess (PT-W6a) golden-verbatim gates.
// ===========================================================================
#if 0 // GOLDEN VERBATIM -- golden atester.cpp:10298-10375 (78 lines).  GATE G-PTk3-NeedResetInitialDelay.  NOT COMPILED: the ACTIVE NeedResetInitialDelay() is the slim body immediately after this #endif.
bool NeedResetInitialDelay()                                                    //JerryYang 20180828 (Steven) : 預熱過程中又觸發預熱的話, 就重新執行預熱
{
    bool bflag=false;

    if(CosFunction.bHiSiliconFunction==true)                                    //kevin 20200110 add initial
    {
        if(bHISIInitiayDelay)                                                   //kevin 20200110 add 海司強至initial delay
        {
            RecordProcess("Reset by When press stop time over use HISI initial delay");
            bHISIInitiayDelay=false;
            bflag=true;
        }
    }

    if(bDoWhenPressStopOverUseInitialDelay)
    {
        RecordProcess("Reset by When press stop time over use initial delay");
        bDoWhenPressStopOverUseInitialDelay=false;
        bflag=true;
    }

    if(bDoAfterAutoCleanFunctionUseInitialDelay)
    {
        RecordProcess("Reset by After auto clean use initial delay");
        bDoAfterAutoCleanFunctionUseInitialDelay=false;
        bflag=true;
    }

    if(bDoAfterShowAlarmMessageUseInitialDelay)
    {
        RecordProcess("Reset by After show alarm message use initial delay");
        bDoAfterShowAlarmMessageUseInitialDelay=false;
        bflag=true;
    }

    if(bDoAfterOpenHeatDoorUseInitialDelay)
    {
        RecordProcess("Reset by After open heat door use initial delay");
        bDoAfterOpenHeatDoorUseInitialDelay=false;
        bflag=true;
    }

    if(bDoWhenHappenTestedTimeBlowUseInitialDelay)
    {
        RecordProcess("Reset by When happen tested time blow use initial delay");
        bDoWhenHappenTestedTimeBlowUseInitialDelay=false;
        bflag=true;
    }

    if(bDoEveryFirstDeviceFunctionUseInitialDelay)
    {
        bDoEveryFirstDeviceFunctionUseInitialDelay=false;
        RecordProcess("Reset by Every fisrt device delay use initial delay");
        bflag=true;
    }

    if(bDoWhenNoFullSiteUseInitialDelay)
    {
        RecordProcess("Reset by When no fullSite use initial delay");
        bDoWhenNoFullSiteUseInitialDelay=false;
        bflag=true;
    }

    if(bDoOTDOffUseInitialDelay)
    {
        RecordProcess("Reset by OTD unlock use initial delay");
        bDoOTDOffUseInitialDelay=false;
        bflag=true;
    }

    if(bTestStartToNextTestStartDelay)                                          //kevin 20181102
    {
        RecordProcess("Reset by SOTk use initial delay");
        bTestStartToNextTestStartDelay=false;
        bflag=true;
    }
    return bflag;
}
#endif // GOLDEN VERBATIM -- golden atester.cpp:10298-10375  (GATE G-PTk3-NeedResetInitialDelay, end)
bool NeedResetInitialDelay()                                                    //JerryYang 20180828 (Steven) : 預熱過程中又觸發預熱的話, 就重新執行預熱
{
#if 0 // TODO(W7) -- golden :10298-10376
#endif
    return false;                                                              // golden default
}
//------------------------------------------------------------------------------
// ===========================================================================
//  GOLDEN VERBATIM PAIR -- CheckToBoostIndexTemp
//  GATED : golden atester.cpp:10377-10440 (64 lines), inert reference text.
//  LIVE  : the slim CheckToBoostIndexTemp() immediately AFTER the #endif below.  It is
//          UNCHANGED by this wave -- net behaviour change is ZERO.
//  WHY   : the census scored this "translated" because a same-named LIVE body
//          existed, without comparing SIZE.  Golden's 64 lines were NOWHERE in
//          the tree -- lost text, not deferred behaviour.  Now the text EXISTS
//          and is auditable, so a later un-gate is mechanical.
//  RULES : nothing inside the gate is fixed, renamed, reflowed or reindented;
//          golden's own defects and misspellings are preserved so a diff
//          against golden stays EMPTY.  Being gated it needs NO callee to
//          exist -- no stub, declaration or header was added for it.
//  SHAPE : same pair shape as csystem.cpp DoTrayFeedProcess / MainProc /
//          DoAllProcess (PT-W6a) golden-verbatim gates.
// ===========================================================================
#if 0 // GOLDEN VERBATIM -- golden atester.cpp:10377-10440 (64 lines).  GATE G-PTk4-CheckToBoostIndexTemp.  NOT COMPILED: the ACTIVE CheckToBoostIndexTemp() is the slim body immediately after this #endif.
int CheckToBoostIndexTemp()                                                     //Steven 20180817 : Boost Function
{
    int QQ=-1;
    double dEotToSotTime=TestIntervalsBoostTime.LatchCycleTime()/1000.0;        //JerryYang 20181122 (Steven) :  (Steven) : 將不同function計時器分開
    if(((ATC_SYSTEM==eNewATCSystem &&
         ATC_InterfaceForm->iATC_MODE_TYPE==ATC_TYPE_60) ||
        CosFunction.bNonATCSupportLBFunction) &&                                //JerryYang 20220126: non ATC也支援LB function
       (Temperature.bBoostFuncttion ||                                          //Steven 20190627 : Boost func加上保護
        Temperature.bLBTempFunction))                                           //Steven 20181102 : LB Temp Function
    {
//        if(bUnderTest)
//        {
//            QQ=-1;
//        }
//        else
//        {
            if(Temperature.bLBTempFunction)
            {
                if(bLBBoostTimeOut==false && UN150Read[tcLB]<Temperature.dBoostIdleTime[Temperature.eBLBL])
                {
                    QQ=4;
                }
//                else if(Temperature.bEnableBoostOffset[Temperature.eBLBB] && UN150Read[tcLB]<Temperature.dBoostIdleTime[Temperature.eBLBB])
//                {
//                    QQ=5;
//                }
                else if(Temperature.bEnableBoostOffset[Temperature.eBLBI] && dEotToSotTime>=Temperature.dBoostIdleTime[Temperature.eBLBI])
                {
                    QQ=3;
                }
            }
            else if(bBoostFirstBoost)
            {
                bBoostFirstBoost=false;
                QQ=0;
            }
            else if(dEotToSotTime>=Temperature.dBoostIdleTime[Temperature.eBMax])
            {
                QQ=0;
            }
            else if(dEotToSotTime>Temperature.dBoostIdleTime[Temperature.eBMin])
            {
                QQ=1;
            }
            else if(dEotToSotTime==Temperature.dBoostIdleTime[Temperature.eBMin])
            {
                QQ=2;
            }
            else
            {
                QQ=-1;
            }
//        }
    }

//    iBoostFuncStep=0;

    if(bUnderTest && QQ!=-1)
    {
        QQ=-1;
    }

    return QQ;
}
#endif // GOLDEN VERBATIM -- golden atester.cpp:10377-10440  (GATE G-PTk4-CheckToBoostIndexTemp, end)
int CheckToBoostIndexTemp()                                                     //Steven 20180817 : Boost Function
{
#if 0 // TODO(W7) -- golden :10377-10443 (boost-temp check)
#endif
    return 0;                                                                  // golden default
}
//------------------------------------------------------------------------------
//  DoTestHeadMotorLoadCell (golden :10444-10746) -- load-cell measure SM.
//------------------------------------------------------------------------------
// ===========================================================================
//  GOLDEN VERBATIM PAIR -- DoTestHeadMotorLoadCell
//  GATED : golden atester.cpp:10444-10745 (302 lines), inert reference text.
//  LIVE  : the slim DoTestHeadMotorLoadCell() immediately AFTER the #endif below.  It is
//          UNCHANGED by this wave -- net behaviour change is ZERO.
//  WHY   : the census scored this "translated" because a same-named LIVE body
//          existed, without comparing SIZE.  Golden's 302 lines were NOWHERE in
//          the tree -- lost text, not deferred behaviour.  Now the text EXISTS
//          and is auditable, so a later un-gate is mechanical.
//  RULES : nothing inside the gate is fixed, renamed, reflowed or reindented;
//          golden's own defects and misspellings are preserved so a diff
//          against golden stays EMPTY.  Being gated it needs NO callee to
//          exist -- no stub, declaration or header was added for it.
//  SHAPE : same pair shape as csystem.cpp DoTrayFeedProcess / MainProc /
//          DoAllProcess (PT-W6a) golden-verbatim gates.
// ===========================================================================
#if 0 // GOLDEN VERBATIM -- golden atester.cpp:10444-10745 (302 lines).  GATE G-PTk4-DoTestHeadMotorLoadCell.  NOT COMPILED: the ACTIVE DoTestHeadMotorLoadCell() is the slim body immediately after this #endif.
void DoTestHeadMotorLoadCell()
{
//    static int iRetryCount=0;
    //static bool bOneTimeFlag=true;
    //static DWORD startTick=-1, endTick, nowTick;
    //static int iToqureCount=0;
    static bool bFirstTime=true;
//    static AnsiString ErrPart="";
    //bool TMode=false;
    //static int iSiteCount=0;
    int &Task=iTestHeadMotorTask;                                               //, iColCounts=0, iRowCounts=0;
//    int i, j;//, ret, TorqueData, iIndexUpPos=0, ccRet;
//    bool flag=false;//, flag2, bIndexSuckCheck;
//    int sp, iMaxPreasure=0, iIndexArm[3]={0, 0, 0};
    AnsiString str;
//    static char  aLoadCell[4][8]={'0'};   //kevin 20190306 add
    AnsiString SData="@e02019Arm1,sideA";

    switch(Task)
    {
        case 1:                                                                 //確認Index Arm 吸嘴狀態
            if(Prod.TestZ1_Test==0)
                iSocketSenSosPos1=-1000;                                        //kevin 20150613 關arm 設定可判斷位置
            else
                iSocketSenSosPos1=Prod.TestZ1_Test-Prod.TestZ1_Drop_Offset+2000;

            if(Prod.TestZ2_Test==0)
                iSocketSenSosPos2=-1000;                                        //kevin 20150613 關arm 設定可判斷位置
            else
                iSocketSenSosPos2=Prod.TestZ2_Test-Prod.TestZ2_Drop_Offset+2000;

            if(bUseInitTempOffset)                                              //Steven 20141117 : 起測時溫度要補Offset
            {
                RecordProcess("After index check trigger initial offset function.(Temp)");
                iInitContactCount=0;                                            //Steven 20141117 : 起測時溫度要補Offset
                fHeaterOK=false;
            }

            if(INSTALL_SOCKET_CLAMP)                                            //JerryYang 20160607 機台選用分離機構 需偵測socket sensor
            {
                if(Sen[SnSocketHasClamp1].IsOn() || Sen[SnSocketHasClamp2].IsOn())
                {
                    ShowMyMessage("Please check the socket sensor","socket sensor偵測異常");
                    break;
                }
            }

            if(TestIF_File.iShuttleMode==1 && TestIF_File.iShuttle_Sel==1)      //kevin 20190318 use arm 2
                Task=400;
            else
                Task=200;
            break;
        case 200:
            if(MOT[MTestZ1].Gali_Two_ZAxis_Move(Prod.TestZ1_Safe, iSpeedSlow, "DoTestHeadMotorLoadCell 200"))
            {
                Task=210;
            }
            break;
        case 210:
            if(MOT[MTestY1].GalilTwoY_Move(Prod.TestY1_Front, Prod.TestY2_Rear, iSpeedY, "DoTestHeadMotorLoadCell 210"))
            {
                Task=215;
            }
            break;
        case 215:                                                               //IndexZ1 and IndexZ2 皆移至安全位置
            if(MOT[MTestZ1].Gali_Two_ZAxis_Move(Prod.TestZ1_Safe, iSpeedSlow, "DoTestHeadMotorLoadCell 215"))
            {
                Task=216;
            }
            break;
        case 216:                                                               //判斷IndexZ1 and IndexZ2 是否已在安全位置
            Task=260;
            break;
        case 260:
            if(MOT[MTestY1].GalilTwoY_Move(Prod.iLoadCellY1, Prod.TestY2_Rear, iSpeedY, "DoTestHeadMotorLoadCell 260"))                                         //ARM 1 on load cell
            {
                Task=290;
            }
            break;
        case 290:                                                               //120: //Arm 1 on load cell DOWN
            if(MOT[MTestZ1].Gali_MotMove(Prod.dLoadCellZ1Down, iSpeedSlow))
            {
                IndexStatus=Z1Down_Z2Up;
                bloadcellRece=false;
                SData="@e02019Arm1,sideA";
                RespondASECom(SData);                                           //kevin 20190906
                Task=312;
                DoTestHeadMotorDelay.SetSecAndOn(5);                            //wait 3 SEC
                bFirstTime=true;
                bloadcellRece=false;
                sLoadCellReceData="";
                iloadcellRece=0;                                                //kevin 20190906
            }
            break;
        case 312:                                                               //12102:
            if(bloadcellRece)                                                   //kevin 20190906
            {
                Task=313;
                if(iloadcellRece==2)                                            //kevin 20190906 add load cell command
                {
                    //kevin 20190906 add load cell NG
                    ShowMessage(sLoadCellReceData);
                    iloadcellRece=0;
                }
            }
            break;
        case 313:                                                               //12110:   kevin 判斷LOAD CEEL 訊號是否 PASS FAIL
            Task=314;
            break;
        case 314:                                                               //12111: //kevin 20130418
            if(MOT[MTestZ1].Gali_Two_ZAxis_Move(Prod.TestZ1_Safe, iSpeedSlow, "DoTestHeadMotorLoadCell 314"))
                Task=315;
            break;
        case 315:                                                                                                                                               //12112:
            if(MOT[MTestY1].GalilTwoY_Move(Prod.iLoadCellY1+TestIF.dSiteYPitch, Prod.TestY2_Rear, iSpeedY, "DoTestHeadMotorLoadCell 315"))                      //ARM 1 on load cell+ ypitch
            {
                Task=316;
            }
            break;
        case 316:                                                               //12200:     //Arm 1 on load cell DOWN
            if(MOT[MTestZ1].Gali_MotMove(Prod.dLoadCellZ1Down, iSpeedSlow))
            {
                IndexStatus=Z1Down_Z2Up;
                Task=317;
                SData="@e02019Arm1,sideB";
                RespondASECom(SData);                                           //kevin 20190906
                bloadcellRece=false;
                sLoadCellReceData="";
                iloadcellRece=0;                                                //kevin 20190906
                DoTestHeadMotorDelay.SetSecAndOn(5);                            //wait 3 SEC
                bFirstTime=true;
                //kevin 送output 訊號給load cell
            }
             //SW[SwLoadCellB].On();     //kevin 送output 訊號給load cell
            break;
        case 317:
            if(bloadcellRece)                                                   //kevin 20190906
            {
                Task=318;

                if(iloadcellRece ==2)                                           //kevin 20190906 add load cell command
                {
                       //kevin 20190906 add load cell NG
                    ShowMessage(sLoadCellReceData);
                    iloadcellRece=0;
                }
            }
            break;
        case 318:                                                               //12110:   kevin 判斷LOAD CEEL 訊號是否 PASS FAIL
            Task=319;
            break;
        case 319:                                                               //12111:
            if(MOT[MTestZ1].Gali_Two_ZAxis_Move(Prod.TestZ1_Safe, iSpeedSlow, "DoTestHeadMotorLoadCell 319"))
                Task=400;
            break;
        case 400:                                                               // arm 2
            if(MOT[MTestZ2].Gali_Two_ZAxis_Move(Prod.TestZ1_Safe, iSpeedSlow, "DoTestHeadMotorLoadCell 400"))
                Task=401;
            break;
        case 401:                                                               //122110:
             SW[SwLoadCellA].Off();
             SW[SwLoadCellB].Off();
            if(MOT[MTestY1].GalilTwoY_Move(Prod.TestY1_Front, Prod.iLoadCellY2, iSpeedY, "DoTestHeadMotorLoadCell 401"))
                Task=402;
            break;
        case 402:                                                               //140: Arm 2 on load cell
            if(MOT[MTestZ2].Gali_MotMove(Prod.dLoadCellZ2Down, iSpeedSlow))
            {
                  IndexStatus=Z1Up_Z2Down;
                  Task= 403;                                                    //14110;
                  SData="@e02019Arm2,sideB";
                  RespondASECom(SData);                                         //kevin 20190906
                  bloadcellRece=false;
                  sLoadCellReceData="";
                  iloadcellRece=0;                                              //kevin 20190906
                  DoTestHeadMotorDelay.SetSecAndOn(5);
                  bFirstTime=true;
                  //kevin 20190306 送訊號給load cell
            }
            //SW[SwLoadCellA].On();
            break;
        case 403:                                                               //14102:                         //Frank 20171030 (Steven) add Floating Shuttle Read Torque Delay
            /*
            if(DoTestHeadMotorDelay.Off())
            {
                Task= 404;
                if(Sen[SnLoadCell1].Status()==true)
                    aLoadCell[2][0]='P';                                        //kevin 20190306 add
                if(Sen[SnLoadCell2].Status()==true)
                    aLoadCell[2][1]='P';                                        //kevin 20190306 add
                if(Sen[SnLoadCell3].Status()==true)
                    aLoadCell[2][2]='P';                                        //kevin 20190306 add
                if(Sen[SnLoadCell4].Status()==true)
                    aLoadCell[2][3]='P';                                        //kevin 20190306 add
                if(Sen[SnLoadCell5].Status()==true)
                    aLoadCell[2][4]='P';                                        //kevin 20190306 add
                if(Sen[SnLoadCell6].Status()==true)
                    aLoadCell[2][5]='P';                                        //kevin 20190306 add
                if(Sen[SnLoadCell7].Status()==true)
                    aLoadCell[2][6]='P';                                        //kevin 20190306 add
                if(Sen[SnLoadCell8].Status()==true)
                    aLoadCell[2][7]='P';                                        //kevin 20190306 add

                SW[SwLoadCellA].Off();                                          //kevin 送output 訊號給load cell
            } */
            if(bloadcellRece)                                                   //kevin 20190906
            {
                Task= 404;

                if(iloadcellRece ==2)                                           //kevin 20190906 add load cell command
                {
                    iloadcellRece=0;
                       //kevin 20190906 add load cell NG
                    ShowMessage(sLoadCellReceData);
                }
            }
            break;
        case 404:                                                               //12110:   kevin 判斷LOAD CEEL 訊號是否 PASS FAIL
            Task=405;
            break;

       case 405:                                                                //12111: //kevin 20130418
            if(MOT[MTestZ2].Gali_Two_ZAxis_Move(Prod.TestZ1_Safe, iSpeedSlow, "DoTestHeadMotorLoadCell 405"))
                Task=406;
            break;
        case 406:                                                               //12112:             //ARM 2 on load cell+ ypitch
            if(MOT[MTestY1].GalilTwoY_Move(Prod.TestY1_Front, Prod.iLoadCellY2-TestIF.dSiteYPitch, iSpeedY, "DoTestHeadMotorLoadCell 406"))
            {
                Task=407;
            }
            break;
        case 407:                                                               //12200:     //Arm 1 on load cell DOWN
            if(MOT[MTestZ2].Gali_MotMove(Prod.dLoadCellZ2Down, iSpeedSlow))
            {
                bloadcellRece=false;                                            //kevin 20190906
                IndexStatus=Z1Up_Z2Down;
                Task=408;
                SData="@e02019Arm1,sideB";
                RespondASECom(SData);                                           //kevin 20190906
                bloadcellRece=false;
                sLoadCellReceData="";
                iloadcellRece=0;                                                //kevin 20190906
                DoTestHeadMotorDelay.SetSecAndOn(5);                            //wait 3 SEC
                //kevin 送output 訊號給load cell
            }
            //SW[SwLoadCellA].Off();     //kevin 送output 訊號給load cell
            break;
        case 408:

           /*
            if(DoTestHeadMotorDelay.Off())
            {
                Task= 409;
                 if(Sen[SnLoadCell1].Status()==true)
                    aLoadCell[3][0]='P';                                        //kevin 20190306 add
                if(Sen[SnLoadCell2].Status()==true)
                    aLoadCell[3][1]='P';                                        //kevin 20190306 add
                if(Sen[SnLoadCell3].Status()==true)
                    aLoadCell[3][2]='P';                                        //kevin 20190306 add
                if(Sen[SnLoadCell4].Status()==true)
                    aLoadCell[3][3]='P';                                        //kevin 20190306 add
                if(Sen[SnLoadCell5].Status()==true)
                    aLoadCell[3][4]='P';                                        //kevin 20190306 add
                if(Sen[SnLoadCell6].Status()==true)
                    aLoadCell[3][5]='P';                                        //kevin 20190306 add
                if(Sen[SnLoadCell7].Status()==true)
                    aLoadCell[3][6]='P';                                        //kevin 20190306 add
                if(Sen[SnLoadCell8].Status()==true)
                    aLoadCell[3][7]='P';                                        //kevin 20190306 add

                SW[SwLoadCellB].Off();                                          //kevin 送output 訊號給load cell
            }*/
            if(bloadcellRece)                                                   //kevin 20190906
            {
                Task=409;

                if(iloadcellRece ==2)                                           //kevin 20190906 add load cell command
                {
                       //kevin 20190906 add load cell NG
                    ShowMessage(sLoadCellReceData);
                }
            }
            break;
        case 409:                                                               //12110:   kevin 判斷LOAD CEEL 訊號是否 PASS FAIL
            Task= 410;
            break;
        case 410:                                                               //12111:
            if(MOT[MTestZ1].Gali_Two_ZAxis_Move(Prod.TestZ1_Safe, iSpeedSlow, "DoTestHeadMotorLoadCell 410"))
                Task=411;
            break;
        case 411:
            if(MOT[MTestY1].GalilTwoY_Move(Prod.TestY1_Front, Prod.TestY2_Rear, iSpeedY, "DoTestHeadMotorLoadCell 411"))
            {
                bLoadCellTest=false;
                bIsAutoOneCycle=true;
                fMain->BtnOneCycleClick(fMain->BtnOneCycle);
                //fMain->Pause();
                Task=1;
            }
            break;
    }
}
#endif // GOLDEN VERBATIM -- golden atester.cpp:10444-10745  (GATE G-PTk4-DoTestHeadMotorLoadCell, end)
void DoTestHeadMotorLoadCell()
{
#if 0 // TODO(W7) -- golden :10444-10746 (load-cell measure SM, MOT[]/load-cell)
#endif
}
//------------------------------------------------------------------------------
//AI(W906-GB-P2b) 20260926: golden reaches the two per-arm test time-out timers through aTester_Front.h:19 /
//  aTester_Rear.h (`extern TQPF_Timer ...`); the V906 narrow headers leave them out (aTester_Rear.h:13), so they are
//  declared here.  Defined at aTester_Front.cpp:3085 / aTester_Rear.cpp:2974.
extern TQPF_Timer hFTestTimeOutDelay;
extern TQPF_Timer hBTestTimeOutDelay;
//AI(W906-GB-P2b) 20260926: translation-gap fix -- this was a no-op (`#if 0 // TODO(W7)`), so in On-Line the live checks
//  `LastSet.iTester==ON_LINE && h?TestTimeOutDelay.Off()` (aTester_Front.cpp:4239/:4268, aTester_Rear.cpp:4090/:4120)
//  saw an unarmed TQPF_Timer (Off() at once, myTimer.cpp:14-17/:40-44) and timed every test out right after SOT
//  (WAR07352).  Golden 912 atester.cpp:10920-10955 verbatim (906 :10747-10782 is the same text).
void SetTestTimeOutTimer(int Index)                                             //Steven 20200407 : 整合Time Out時間設定
{
    if(Index==0)
    {
        if(bInitialMaxTime==true)                                               //jou 2011-11-09 增加initial max time set
        {
            hFTestTimeOutDelay.SetSecAndOn(TestIF.iInitialMaxTime);
        }
        else
        {
            hFTestTimeOutDelay.SetSecAndOn(TestIF.iMaxTime);
        }
    }
    else if(Index==1)
    {
        if(bInitialMaxTime==true)
        {
            hBTestTimeOutDelay.SetSecAndOn(TestIF.iInitialMaxTime);
        }
        else
        {
            hBTestTimeOutDelay.SetSecAndOn(TestIF.iMaxTime);
        }
    }
//    else                                                                      //Steven 20210827 : Test Time Out要跟Index Arm同步設定
//    {
        if(bInitialMaxTime==true)                                               //Steven 20190119 : 避免Time out retry -> pause後,沒送出SOT會再發Time out.
        {
            TestTimeOut.SetSecAndOn(TestIF.iInitialMaxTime+10);
        }
        else
        {
            TestTimeOut.SetSecAndOn(TestIF.iMaxTime+10);
        }
//    }
}
//------------------------------------------------------------------------------
//  CheckSocketSensor (golden :10784-11127) -- socket-sensor verify SM.
//------------------------------------------------------------------------------
// ===========================================================================
//  GOLDEN VERBATIM PAIR -- CheckSocketSensor
//  GATED : golden atester.cpp:10784-11126 (343 lines), inert reference text.
//  LIVE  : the slim CheckSocketSensor() immediately AFTER the #endif below.  It is
//          UNCHANGED by this wave -- net behaviour change is ZERO.
//  WHY   : the census scored this "translated" because a same-named LIVE body
//          existed, without comparing SIZE.  Golden's 343 lines were NOWHERE in
//          the tree -- lost text, not deferred behaviour.  Now the text EXISTS
//          and is auditable, so a later un-gate is mechanical.
//  RULES : nothing inside the gate is fixed, renamed, reflowed or reindented;
//          golden's own defects and misspellings are preserved so a diff
//          against golden stays EMPTY.  Being gated it needs NO callee to
//          exist -- no stub, declaration or header was added for it.
//  SHAPE : same pair shape as csystem.cpp DoTrayFeedProcess / MainProc /
//          DoAllProcess (PT-W6a) golden-verbatim gates.
// ===========================================================================
#if 0 // GOLDEN VERBATIM -- golden atester.cpp:10784-11126 (343 lines).  GATE G-PTk4-CheckSocketSensor.  NOT COMPILED: the ACTIVE CheckSocketSensor() is the slim body immediately after this #endif.
bool CheckSocketSensor(int iArm, AnsiString Func, bool bInit, bool bCheckArmHieght)                                     //Steven 20200615 : Socket Sensor整合成Function
{                                                                               //jimmychiu 20230830 : add switch check arm height in check socket sensor function
    static int iSocketSensorCT[2][16]={{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}};

    int iAZ1=0, iAZ2=0;
    bool bHasErr=false, bArmHeightIsEnough=false;
    AnsiString str1, sBufferT;

    if(bInit)
    {
        iShowSocketSensor=0;
        for(int i=0; i<TestIF_File.iSocketCount; i++)
            iSocketSensorCT[iArm][i]=0;
    }
    sSocketSensorErr="";

    if(IniConfig.bC08_SocketSensor && TestIF_File.bEnSocketSensor)              //kevin 20130504 socket sensor detect error
    {
        if(bCheckArmHieght==true)                                               //jimmychiu 20230830 : add switch check arm height in check socket sensor function
        {
            iAZ1=MOT[MTestZ1].Gali_ReadEncoderPos();                            //JerryYang 20190327 fix socket sensor誤判問題, 要讀Encoder位置
            iAZ2=MOT[MTestZ2].Gali_ReadEncoderPos();

            if(Prod.TestZ1_Test==0)
                iSocketSenSosPos1=-1000;                                        //kevin 20150613 關arm 設定可判斷位置
            else
                iSocketSenSosPos1=Prod.TestZ1_Test-Prod.TestZ1_Drop_Offset+2000;

            if(Prod.TestZ2_Test==0)
                iSocketSenSosPos2=-1000;                                        //kevin 20150613 關arm 設定可判斷位置
            else
                iSocketSenSosPos2=Prod.TestZ2_Test-Prod.TestZ2_Drop_Offset+2000;
                bArmHeightIsEnough=(iAZ1>iSocketSenSosPos1) && (iAZ2>iSocketSenSosPos2);
        }
        else
        {
            bArmHeightIsEnough=true;
        }

        if(bArmHeightIsEnough)                                                  //kevin 20140508 socket sensor
        {
            if(IniConfig.bD58UseArm1PickPlaceArm2Test==true &&
               TestIF_File.bArm1PickPlaceArm2Test==true)                        //Steven 20200604 : Arm1丟 Arm2測的時候, Arm2只檢查置偏
            {
                if(bHasICinSocket)
                {
                    for(int i=0; i<TestIF_File.iSocketCount; i++)
                    {
                        if(TestIF_File.iSensorCheckType[i]==2 &&
                           Sen[SThreadPara.iSocketSensor[i]].Enable &&
                           Sen[SThreadPara.iSocketSensor[i]].IsOn())            //On的有置偏
                        {
                            MOT[MTestZ1+iArm].Gali_Command("ST", Func+AnsiString("1"));
                            bHasErr=true;
                            iShowSocketSensor=2;
                            sSocketSensorErr+=IntToStr(i+1)+",";
                        }
                        else if(TestIF_File.iSensorCheckType[i]==1 &&
                           Sen[SThreadPara.iSocketSensor[i]].Enable &&
                           Sen[SThreadPara.iSocketSensor[i]].IsOff())           //JerryYang 20260223 : Sensor off狀態表示沒IC
                        {
                            MOT[MTestZ1+iArm].Gali_Command("ST", Func+AnsiString("1"));
                            bHasErr=true;
                            iShowSocketSensor=1;
                            sSocketSensorErr+=IntToStr(i+1)+",";
                        }
                    }
                    #ifdef FOR_QLE
                    if(TestIF_File.iTestMode==QualSite2X2)
                    {
                        if(TestIF_File.iSocketCount==8)
                        {
                            for(int i=0; i<TestIF_File.iSocketCount; i++)
                            {
                                if(i>=4 && i<=7)
                                {
                                    if(TestIF_File.iSensorCheckType[i]==1 &&
                                       Sen[SThreadPara.iSocketSensor[i]].Enable &&
                                       Sen[SThreadPara.iSocketSensor[i]].IsOff())           //JerryYang 20260223 : Sensor off狀態表示沒IC
                                    {
                                        if((i==4 && FTestSuck.Item[0][0]!=NULL_IC && FTestSuck.Item[0][0]!=HAS_NULL_IC) ||
                                           (i==5 && FTestSuck.Item[0][1]!=NULL_IC && FTestSuck.Item[0][1]!=HAS_NULL_IC) ||
                                           (i==6 && FTestSuck.Item[1][0]!=NULL_IC && FTestSuck.Item[1][0]!=HAS_NULL_IC) ||
                                           (i==7 && FTestSuck.Item[1][1]!=NULL_IC && FTestSuck.Item[1][1]!=HAS_NULL_IC))
                                        {
                                            MOT[MTestZ1+iArm].Gali_Command("ST", Func+AnsiString("1"));
                                            bHasErr=true;
                                            iShowSocketSensor=3;
                                            sSocketSensorErr+=IntToStr(i+1)+",";
                                        }
                                    }
                    }
                            }
                        }
                    }
                    else if(TestIF_File.iTestMode==DualSite)                    //KevinCheng 20260420 : 一丟一測 DualSite
                    {
                        if(TestIF_File.iSocketCount==8)
                        {
                            for(int i=0; i<TestIF_File.iSocketCount; i++)
                            {
                                if(i>=4 && i<=5)
                                {
                                    if(TestIF_File.iSensorCheckType[i]==1 &&
                                       Sen[SThreadPara.iSocketSensor[i]].Enable &&
                                       Sen[SThreadPara.iSocketSensor[i]].IsOff())           //JerryYang 20260223 : Sensor off狀態表示沒IC
                                    {
                                        if((i==4 && FTestSuck.Item[0][0]!=NULL_IC && FTestSuck.Item[0][0]!=HAS_NULL_IC) ||
                                           (i==5 && FTestSuck.Item[0][1]!=NULL_IC && FTestSuck.Item[0][1]!=HAS_NULL_IC))
                                        {
                                            MOT[MTestZ1+iArm].Gali_Command("ST", Func+AnsiString("1"));
                                            bHasErr=true;
                                            iShowSocketSensor=3;
                                            sSocketSensorErr+=IntToStr(i+1)+",";
                                        }
                                    }
                                }
                            }
                        }
                    }
                    #endif
                }
                else
                {
                    for(int i=0; i<TestIF_File.iSocketCount; i++)
                    {
                        if(TestIF_File.iSensorCheckType[i]==1 &&
                           Sen[SThreadPara.iSocketSensor[i]].Enable &&
                           Sen[SThreadPara.iSocketSensor[i]].IsOn())            //On的有殘料
                        {
                            MOT[MTestZ1+iArm].Gali_Command("ST", Func+AnsiString("2"));
                            bHasErr=true;
                            iShowSocketSensor=1;
                            sSocketSensorErr+=IntToStr(i+1)+",";
                        }
                    }
                }
            }
            else
            {
                for(int i=0; i<TestIF_File.iSocketCount; i++)
                {
                    if(Sen[SThreadPara.iSocketSensor[i]].Enable &&
                       Sen[SThreadPara.iSocketSensor[i]].IsOn())                //On的有殘料
                    {
                        MOT[MTestZ1+iArm].Gali_Command("ST", Func+AnsiString("3"));
                        bHasErr=true;
                        iShowSocketSensor=1;
                        sSocketSensorErr+=IntToStr(i+1)+",";
                    }
                }
            }

            if(bHasErr)
            {
                if(REAL_TIME_CCD==true)
                {
                    ScanBtnThd->Stop();
                }

                str1.sprintf("%s, iShowSocketSensor:%d, %s", Func, iShowSocketSensor, sSocketSensorErr);
                if(iShowSocketSensor<1)
                    iShowSocketSensor=1;

                RecordProcess(str1);
                if(iArm==0)
                    sBufferT="Z1UpZ2Down: Z1Pos socket sensor detect error"+IntToStr(iAZ1)+">"+IntToStr(Prod.TestZ1_Test-Prod.TestZ1_Drop_Offset+2000)+" Z2Pos "+IntToStr(iAZ2)+">"+IntToStr(Prod.TestZ2_Test-Prod.TestZ2_Drop_Offset+2000);
                else
                    sBufferT="Z1DownZ2Up: Z1Pos "+IntToStr(iAZ1)+">"+IntToStr(Prod.TestZ1_Test-Prod.TestZ1_Drop_Offset+2000)+" Z2Pos "+IntToStr(iAZ1)+">"+IntToStr(Prod.TestZ2_Test-Prod.TestZ2_Drop_Offset+2000);

                RecordProcess(sBufferT);                                        //kevin 20150506
            }
        }

//        if(CUSTOMER_CODE==CC_JCET &&                                          //Steven 20230214 : 強制要檢查Socket Sensor有沒有遮斷的效果
        if(bHasErr==false)                                                      //jou 2016-10-03 江陰長電要求Socket sensor要real time檢查遮斷效果,下壓的時候必須on
        {
            if(IniConfig.bD58UseArm1PickPlaceArm2Test==true &&
               TestIF_File.bArm1PickPlaceArm2Test==true)                        //Steven 20200604 : Arm1丟 Arm2測的時候, Arm2只檢查置偏
            {
                if(bCheckArmHieght==true)                                       //jimmychiu 20230830 : add switch check arm height in check socket sensor function
                {
                    iAZ1=MOT[MTestZ1].Gali_ReadEncoderPos();                    //JerryYang 20190327 fix socket sensor誤判問題, 要讀Encoder位置
                    iAZ2=MOT[MTestZ2].Gali_ReadEncoderPos();
                    if(Prod.TestZ1_Test==0)
                        iSocketSenSosPos1=-1000;                                //kevin 20150613 關arm 設定可判斷位置
                    else
                        iSocketSenSosPos1=Prod.TestZ1_Test-Prod.TestZ1_Drop_Offset+2000;

                    if(Prod.TestZ2_Test==0)
                        iSocketSenSosPos2=-1000;                                //kevin 20150613 關arm 設定可判斷位置
                    else
                        iSocketSenSosPos2=Prod.TestZ2_Test-Prod.TestZ2_Drop_Offset+2000;
                    bArmHeightIsEnough=(iAZ1>iSocketSenSosPos1) && (iAZ2>iSocketSenSosPos2);
                }
                else
                {
                    bArmHeightIsEnough=true;
                }

                if(bArmHeightIsEnough)                                          //jimmychiu 20230830 : add switch check arm height in check socket sensor function
                {
                    if(bHasICinSocket)
                    {
                        for(int i=0; i<TestIF_File.iSocketCount; i++)
                        {
                            if(TestIF_File.iSensorCheckType[i]==2 &&
                               Sen[SThreadPara.iSocketSensor[i]].Enable &&
                               Sen[SThreadPara.iSocketSensor[i]].IsOn())        //On的有至偏
                            {
                                iSocketSensorCT[iArm][i]++;
                                if(iSocketSensorCT[iArm][i]>=1)
                                {
                                    iSocketSensorCT[iArm][i]=0;
                                    MOT[MTestZ1+iArm].Gali_Command("ST", Func+AnsiString("4"));
                                    bHasErr=true;
                                    iShowSocketSensor=2;
                                    sSocketSensorErr+=IntToStr(i+1)+",";
                                }
                            }
                        }
                    }
                    else
                    {
                        for(int i=0; i<TestIF_File.iSocketCount; i++)
                        {
                            if(TestIF_File.iSensorCheckType[i]==1 &&
                               Sen[SThreadPara.iSocketSensor[i]].Enable &&
                               Sen[SThreadPara.iSocketSensor[i]].IsOn())        //On的有殘料
                            {
                                iSocketSensorCT[iArm][i]++;
                                if(iSocketSensorCT[iArm][i]>=1)
                                {
                                    iSocketSensorCT[iArm][i]=0;
                                    MOT[MTestZ1+iArm].Gali_Command("ST", Func+AnsiString("5"));
                                    bHasErr=true;
                                    iShowSocketSensor=1;
                                    sSocketSensorErr+=IntToStr(i+1)+",";
                                }
                            }
                        }
                    }
                }
            }
            else
            {
                if(iArm==0)
                {
                    if(IniConfig.bD30EnableSiteModeSelect &&
                       (TestIF.iShuttleMode==0 ||
                        (TestIF.iShuttleMode==1 && TestIF.iShuttle_Sel==0)))
                    {
                        if(bCheckArmHieght==true)
                        {
                            bArmHeightIsEnough=(iAZ1<=(Prod.TestZ1_Test-Prod.TestZ1_Drop_Offset));
                        }
                        else
                        {
                            bArmHeightIsEnough=true;
                        }

                        if(bArmHeightIsEnough)                                  //kevin 20140508 socket sensor
                        {
                            sSocketSensorErr="index check Socket Sensor down sensor off,";
                            for(int i=0; i<TestIF_File.iSocketCount; i++)
                            {
                                if(((i==0 && FTestSuck.ArmRow0HaveRealIC()) ||
                                    (i==1 && FTestSuck.ArmRow1HaveRealIC())) &&                                         //Steven 20230410 : 單排關site, socket sensor會誤判
                                   FTestSuck.HasRealIC() &&
                                   Sen[SThreadPara.iSocketSensor[i]].Enable &&
                                   Sen[SThreadPara.iSocketSensor[i]].IsOff())
                                {
                                    iSocketSensorCT[iArm][i]++;
                                    if(iSocketSensorCT[iArm][i]>=1)
                                    {
                                        iSocketSensorCT[iArm][i]=0;
                                        MOT[MTestZ1+iArm].Gali_Command("ST", Func+AnsiString("6"));
                                        bHasErr=true;
                                        iShowSocketSensor=1;
                                        sSocketSensorErr+=IntToStr(i+1)+",";
                                    }
                                }
                            }
                        }
                    }
                }
                else
                {
                    if(IniConfig.bD30EnableSiteModeSelect &&
                       (TestIF.iShuttleMode==0 ||
                        (TestIF.iShuttleMode==1 && TestIF.iShuttle_Sel==1)))
                    {
                        if(bCheckArmHieght==true)                               //jimmychiu 20230830 : add switch check arm height in check socket sensor function
                        {
                            bArmHeightIsEnough=(iAZ2<=(Prod.TestZ2_Test-Prod.TestZ2_Drop_Offset));
                        }
                        else
                        {
                            bArmHeightIsEnough=true;
                        }

                        if(bArmHeightIsEnough)                                  //kevin 20140508 socket sensor
                        {
                            sSocketSensorErr="index check Socket Sensor down sensor off,";
                            for(int i=0; i<TestIF_File.iSocketCount; i++)
                            {
                                if(((i==0 && BTestSuck.ArmRow0HaveRealIC()) ||  //Richard 20230417 : FTestSuck>BTestSuck
                                    (i==1 && BTestSuck.ArmRow1HaveRealIC())) &&                                         //Steven 20230410 : 單排關site, socket sensor會誤判
                                   BTestSuck.HasRealIC() &&
                                   Sen[SThreadPara.iSocketSensor[i]].Enable &&
                                   Sen[SThreadPara.iSocketSensor[i]].IsOff())
                                {
                                    iSocketSensorCT[iArm][i]++;
                                    if(iSocketSensorCT[iArm][i]>=1)
                                    {
                                        iSocketSensorCT[iArm][i]=0;
                                        MOT[MTestZ2].Gali_Command("ST", Func+AnsiString("7"));
                                        bHasErr=true;
                                        iShowSocketSensor=1;
                                        sSocketSensorErr+=IntToStr(i+1)+",";
                                    }
                                }
                            }
                        }
                    }
                }
            }

            if(bHasErr)
            {
                if(REAL_TIME_CCD==true)
                {
                    ScanBtnThd->Stop();
                }
                str1.sprintf("%s_real time, iShowSocketSensor:%d, %s", Func, iShowSocketSensor, sSocketSensorErr);
                RecordProcess(str1);
                if(iShowSocketSensor<1)
                    iShowSocketSensor=1;
            }
        }
    }
    return bHasErr;
}
#endif // GOLDEN VERBATIM -- golden atester.cpp:10784-11126  (GATE G-PTk4-CheckSocketSensor, end)
bool CheckSocketSensor(int iArm, AnsiString Func, bool bInit, bool bCheckArmHieght)                                     //Steven 20200615 : Socket Sensor整合成Function
{
#if 0 // TODO(W7) -- golden :10784-11127 (socket-sensor verify SM, Sen[]/MOT[])
#endif
    (void)iArm; (void)Func; (void)bInit; (void)bCheckArmHieght;
    return true;                                                               // offline: socket sensor treated OK
}
//------------------------------------------------------------------------------
//  CheckICExistInSocket (golden :11128-11157) -- Gigas IC-exist check.
//------------------------------------------------------------------------------
void CheckICExistInSocket(AnsiString Func)                                      //Jimmychiu 20250827 : Gigas 要求加入Index下壓時確認IC存在
{
#if 0 // TODO(W7) -- golden :11128-11157 (Gigas IC-exist check, Socket)
#endif
    (void)Func;
}
//------------------------------------------------------------------------------
TQPF_Timer ESCDelay;                                                            //AI(W906-GB-P2b) 20260926: N1 golden file-scope peer (golden 912 atester.cpp:11330 / 906 :11155), never ported; DoIndexSocketCheck is its only user
int iESCError=0;                                                                //AI(W906-GB-P2b) 20260926: N1 golden 912 atester.cpp:11331 / 906 :11156 (file-local in golden; no other TU uses it)
//==============================================================================
void InitialIndexSocketCheckTask()                                              //Steven 20201022 : For RFMD Empty Socket Check Funstion.
{
    iDoIndexSocketCheckTask=1;
    iESCError=0;                                                                //AI(W906-GB-P2b) 20260926: N2 golden 912 atester.cpp:11336 / 906 :11161 (lost when the file was first ported)
}
//------------------------------------------------------------------------------
//  DoIndexSocketCheck (golden 912 atester.cpp:11339-11467 / golden 906 :11164-11292) -- RFMD empty-socket check SM.
//------------------------------------------------------------------------------
//  AI(W906-GB-P2b) 20260926: LIVE golden body -- tester-comm stage P2b (user ruling 20260926:
//  "由 St02 直接進行這一部分的移植工作").  The former GOLDEN VERBATIM PAIR (inert G-PTk2 block,
//  re-verified byte-identical to golden 906 :11164-11292, + the slim no-op stub) is now this ONE
//  function; the slim stub is retired.  906->912 diff of this function: NONE (also none in the
//  peers ESCDelay / iESCError / InitialIndexSocketCheckTask).
//
//  GATE REGISTER (every #if 0 in this function; golden text kept inside; lines = golden 912)
//   S01 :11455  case 600  COM2->SendCommToVision(COM2->rtInspEnd, false).  In this TU COM2 is the
//               TU-local `#define COM2 (&W7T1_com2_ext)` (atester.cpp:5642, struct :5617-5637): it has
//               SendCommToVision (offline no-op, :5632) but no rtInspEnd; the global TCOM2Shim
//               (atester_shims.h:373-463) has neither (golden rs232.h:160 / :126).
//   S02 :11456  case 600  COM2->InitRealTimeCCDPara() -- in neither struct (golden rs232.h:159).
//   S03 :11463  case 700  as S01.
//   S04 :11464  case 700  as S02.
//               Effect: the RTC2 "inspection end" is not sent after the check (Jimmychiu 20231130 fix
//               for #P231120-ATK-H9-04).  Adding rtInspEnd + InitRealTimeCCDPara to W7T1_TCOM2Ext would
//               lift S01-S04 with the SAME run-time effect (that seam sends nothing either).
//  NON-GATE ADAPTATIONS
//   N1  ESCDelay / iESCError restored at file scope just above (golden peers, never ported).
//   N2  InitialIndexSocketCheckTask gets back golden's `iESCError=0;` -- without it an error run
//       (case 600 never clears iESCError) makes the NEXT good run alarm again at case 500.
//   N3  block-scope `extern const unsigned int MSG_CMD_ESC;` (MessageDef.h:183 / MessageDef.cpp:153),
//       same idiom and same removal condition as ProcessTesterTimeOut's N1.
//  REACHABILITY TODAY: no live caller.  csystem.cpp:1280 sits inside the inert DoAllProcess block
//  (#if 0 @694) and :1853 inside `#if 0 // TODO(W7)` @1847; the four bDoEmptySocketCheck=true setters
//  (csystem.cpp:4649/5310/5338/5346) are live.  So this un-gate changes no run-time behaviour until the
//  DoAllProcess EmptySocketCheck rung goes live; then it moves MTestY1/MTestZ1/MTestZ2 exactly as golden.
//  GOLDEN QUIRKS KEPT
//   Q1  case 600 leaves Task at 600 and does not clear iESCError (re-entry relies on the Init above).
//   Q2  off-line (LastSet.iTester==0) the Index still goes down to TestZx_Test-TestZx_Drop_Offset and
//       back; only the ESC request to the tester is skipped (200 -> 500).
//------------------------------------------------------------------------------
void DoIndexSocketCheck()                                                       //Steven 20201022 : For RFMD Empty Socket Check Funstion.
{
    extern const unsigned int MSG_CMD_ESC;                                      //AI(W906-GB-P2b) 20260926: N3 block-scope extern of MessageDef.h:183 (defined MessageDef.cpp:153 =96); atester.cpp does not reach MessageDef.h
    int &Task=iDoIndexSocketCheckTask;

    if(iOneCycle==0)
    {
        bDoEmptySocketOneCycle=false;
    }

    switch(Task)
    {
        case 1:
            if(MOT[MTestZ1].Gali_Two_ZAxis_Move(0, iSpeedSlow))
            {
                Task=10;
            }
            break;
        case 10:
            if(IniConfig.bD30EnableSiteModeSelect && TestIF.iShuttleMode==1 && TestIF.iShuttle_Sel==1)
            {
                if(MOT[MTestY1].GalilTwoY_Move(Prod.TestY1_Front, Prod.TestY2_Middle, iSpeedSlow, __FUNC__))
                {
                    ESCDelay.SetSecAndOn(0.5);
                    Task=20;
                }
            }
            else
            {
                if(MOT[MTestY1].GalilTwoY_Move(Prod.TestY1_Middle, Prod.TestY2_Rear, iSpeedSlow, __FUNC__))
                {
                    ESCDelay.SetSecAndOn(0.5);
                    Task=50;
                }
            }
            break;
        case 20:
            if(ESCDelay.Off())
            {
                Task=30;
            }
            break;
        case 30:
            if(MOT[MTestZ2].Gali_MotMove(Prod.TestZ2_Test-Prod.TestZ2_Drop_Offset, iSpeedSlow))
            {
                ESCDelay.SetSecAndOn(0.5);
                Task=200;
            }
            break;
        case 50:
            if(ESCDelay.Off())
            {
                Task=60;
            }
            break;
        case 60:
            if(MOT[MTestZ1].Gali_MotMove(Prod.TestZ1_Test-Prod.TestZ1_Drop_Offset, iSpeedSlow))
            {
                ESCDelay.SetSecAndOn(0.5);
                Task=200;
            }
            break;
        case 200:
            if(ESCDelay.Off())
            {
                if(LastSet.iTester==0)
                {
                    Task=500;
                    break;
                }
//                ESCDelay.SetSecAndOn(iMaxTestWaitTime);
                SetTestTimeOutTimer(0);
                iGetESCResult=0;
                fMain->SendMSG_CMD(MSG_CMD_ESC);
                Task=300;
            }
            break;
        case 300:
            if(iGetESCResult==1)
            {
                Task=400;
            }
            else if(iGetESCResult==2)
            {
                iESCError=1;
                Task=400;
            }
            else if(TestTimeOut.Off())
            {
                iESCError=2;
                Task=400;
            }
            break;
        case 400:
            if(MOT[MTestZ1].Gali_Two_ZAxis_Move(0, iSpeedSlow))
            {
                Task=500;
            }
            break;
        case 500:
            if(MOT[MTestY1].GalilTwoY_Move(Prod.TestY1_Front, Prod.TestY2_Rear, iSpeedSlow, __FUNC__))
            {
                if(iESCError!=0)
                    Task=600;
                else
                    Task=700;
            }
            break;
        case 600:
            if(iESCError==1)
            {
                ShowErrorMessage("WAR0352", K_SKIP, MMIndex);
            }
            else
            {
                ShowErrorMessage("WAR0353", K_SKIP, MMIndex);
            }
#if 0 // TODO(W906-GB-P2b): S01 COM2 in this TU is `#define COM2 (&W7T1_com2_ext)` (atester.cpp:5642; struct W7T1_TCOM2Ext :5617-5637): SendCommToVision is there as an offline no-op (:5632) but rtInspEnd is not (only rtSiteMap/rtFullTOK/rtFullTNG); the global TCOM2Shim (atester_shims.h:373-463) has neither (golden TCOM2 rs232.h:160 / :126) -- golden atester.cpp:11455 (912; 906 :11280)
            COM2->SendCommToVision(COM2->rtInspEnd, false);                     //Jimmychiu 20231130 : Fixed for #P231120-ATK-H9-04 , V3.32.810_Beta & V3.32.811_Beta , RTC2 Communication error
#endif
#if 0 // TODO(W906-GB-P2b): S02 InitRealTimeCCDPara is a member of neither W7T1_TCOM2Ext (atester.cpp:5617-5637, what COM2 means here via the :5642 #define) nor TCOM2Shim (atester_shims.h:373-463) (golden TCOM2 rs232.h:159) -- golden atester.cpp:11456 (912; 906 :11281)
            COM2->InitRealTimeCCDPara();                                        //Jimmychiu 20231130 : Fixed for #P231120-ATK-H9-04 , V3.32.810_Beta & V3.32.811_Beta , RTC2 Communication error
#endif
            bDoEmptySocketCheck=false;
            break;
        case 700:
            iESCError=0;
            bDoEmptySocketCheck=false;
            RecordProcess("EmptySocketCheck Finish");
#if 0 // TODO(W906-GB-P2b): S03 COM2 in this TU is `#define COM2 (&W7T1_com2_ext)` (atester.cpp:5642; struct W7T1_TCOM2Ext :5617-5637): SendCommToVision is there as an offline no-op (:5632) but rtInspEnd is not (only rtSiteMap/rtFullTOK/rtFullTNG); the global TCOM2Shim (atester_shims.h:373-463) has neither (golden TCOM2 rs232.h:160 / :126) -- golden atester.cpp:11463 (912; 906 :11288)
            COM2->SendCommToVision(COM2->rtInspEnd, false);                     //Jimmychiu 20231130 : Fixed for #P231120-ATK-H9-04 , V3.32.810_Beta & V3.32.811_Beta , RTC2 Communication error
#endif
#if 0 // TODO(W906-GB-P2b): S04 InitRealTimeCCDPara is a member of neither W7T1_TCOM2Ext (atester.cpp:5617-5637, what COM2 means here via the :5642 #define) nor TCOM2Shim (atester_shims.h:373-463) (golden TCOM2 rs232.h:159) -- golden atester.cpp:11464 (912; 906 :11289)
            COM2->InitRealTimeCCDPara();                                        //Jimmychiu 20231130 : Fixed for #P231120-ATK-H9-04 , V3.32.810_Beta & V3.32.811_Beta , RTC2 Communication error
#endif
            return;
    }
}
//------------------------------------------------------------------------------
//  DoHalfViewAllPassVerifyRTC (golden :11297-11493) / DoHalfViewAllFailVerifyRTC
//  (:11494-11699) -- RTC half-view verify SMs (COM2/RTC).
//------------------------------------------------------------------------------
// ===========================================================================
//  GOLDEN VERBATIM PAIR -- DoHalfViewAllPassVerifyRTC
//  GATED : golden atester.cpp:11297-11491 (195 lines), inert reference text.
//  LIVE  : the slim DoHalfViewAllPassVerifyRTC() immediately AFTER the #endif below.  It is
//          UNCHANGED by this wave -- net behaviour change is ZERO.
//  WHY   : the census scored this "translated" because a same-named LIVE body
//          existed, without comparing SIZE.  Golden's 195 lines were NOWHERE in
//          the tree -- lost text, not deferred behaviour.  Now the text EXISTS
//          and is auditable, so a later un-gate is mechanical.
//  RULES : nothing inside the gate is fixed, renamed, reflowed or reindented;
//          golden's own defects and misspellings are preserved so a diff
//          against golden stays EMPTY.  Being gated it needs NO callee to
//          exist -- no stub, declaration or header was added for it.
//  SHAPE : same pair shape as csystem.cpp DoTrayFeedProcess / MainProc /
//          DoAllProcess (PT-W6a) golden-verbatim gates.
// ===========================================================================
#if 0 // GOLDEN VERBATIM -- golden atester.cpp:11297-11491 (195 lines).  GATE G-PTk3-DoHalfViewAllPassVerifyRTC.  NOT COMPILED: the ACTIVE DoHalfViewAllPassVerifyRTC() is the slim body immediately after this #endif.
bool DoHalfViewAllPassVerifyRTC(bool bInitial)                                  //JerryYang 20220215 : RTC Auto Verify half view check
{
    if(bInitial==true)
    {
        iDoHalfViewAllPassVerifyTask=1;
        return false;
    }
    bool TMode=false;

    int &Task=iDoHalfViewAllPassVerifyTask;
    switch(Task)
    {
        case 1:
            if(MOT[MTestZ1].Gali_Two_ZAxis_Move(Prod.TestZ1_Safe, iSpeedSlow, "DoHalfViewAllPassVerifyRTC 1"))
            {
                Task=100;
            }
            break;
        case 100:
            if(MOT[MTestY1].GalilTwoY_Move(Prod.TestY1_Front, Prod.TestY2_Middle, iSpeedY, "DoHalfViewAllPassVerifyRTC 1010"))
            {
                if(iContactMode==CONTACT_NORMAL)
                {
                    bHalfViewVerifyNeedAboveSocket=false;
                    DoSetupSystemToProd();
                }
                else
                {
                    Prod.TestZ1_Test=Get0_01MMType(fContact->edContactHeight1->Text.c_str())+Get0_01MMType(fContact->edDropOffset1->Text.c_str());
                    Prod.TestZ2_Test=Get0_01MMType(fContact->edContactHeight2->Text.c_str())+Get0_01MMType(fContact->edDropOffset2->Text.c_str());
                }
                Task=200;
            }
            break;
        case 200:
            if(iContactMode==CONTACT_NORMAL)
            {
                bHalfViewVerifyNeedAboveSocket=false;
                DoSetupSystemToProd();
            }
            else
            {
                Prod.TestZ1_Test=Get0_01MMType(fContact->edContactHeight1->Text.c_str())+Get0_01MMType(fContact->edDropOffset1->Text.c_str());
                Prod.TestZ2_Test=Get0_01MMType(fContact->edContactHeight2->Text.c_str())+Get0_01MMType(fContact->edDropOffset2->Text.c_str());
            }

            if(MOT[MTestZ2].Gali_MotMove(Prod.TestZ2_Test-Prod.TestZ2_Drop_Offset, iSpeedSlow))
            {
                IndexStatus=Z1Up_Z2Down;
                Task=300;
            }
            break;
        case 300:                                                               //通知Half view auto verify : Arm1
            COM2->SendCommToVision(COM2->rtArm1AllPassVerify, true);
            DoTestHeadMotorDelay.SetSecAndOn(10);
            DoTestHeadMotorDelay3.SetSecAndOn(0.5);
            Task=400;
            break;
        case 400:
//            if(COM2->bRealTimeCom_ReceiveOK[COM2->rtArm1AllPassVerify])
            if(Sen[SnRealTimeCCDIndexArm].IsOff())
            {
                if(DoTestHeadMotorDelay3.Off())
                {
                    COM2->bRealTimeCom_ReceiveOK[COM2->rtALLPASSOK]=false;
                    COM2->bRealTimeCom_ReceiveOK[COM2->rtALLPASSNG]=false;
                    Task=500;
                }
            }
            else if(DoTestHeadMotorDelay.Off())
            {
                ShowMyMessage("RTC Arm 1 Half Veiw all pass verify timeout");
                COM2->DoReleaseAndInspEnd();
                Task=1;
            }
            break;
        case 500:
            if(DeviceForm.ContactMode==TMove ||
               DeviceForm.ContactMode==TMoveDrop ||                             //jou 2012-02-03 新增T Move Drop
               DeviceForm.ContactMode==TMoveDropSlowContact ||
               DeviceForm.ContactMode==TMoveSlowContact)                        //Steven 20160130 : TMove Soft contact
            {
                TMode=true;
            }
            else
            {
                //jou 2012-02-29 index pick up error,index arm move to center & alarm
                if(IniConfig.bIndexPickupErrStop==true && bIndexArm2PickupErrStop==true)
                {
                    TMode=true;
                }
                else
                {
                    TMode=false;
                }
            }

            if(MOT[MTestY1].Z1DownZ2Up(MOT[MTestZ1].GailSpeed, TMode, false))
            {
                Task=600;
            }
            break;
        case 600:                                                               //等待RTC回傳verify結果
            if(COM2->bRealTimeCom_ReceiveOK[COM2->rtALLPASSOK])
            {
                Task=700;
                COM2->SendCommToVision(COM2->rtArmFinish, false);
            }
            else if(COM2->bRealTimeCom_ReceiveOK[COM2->rtALLPASSNG])
            {
                Task=41400;
                COM2->SendCommToVision(COM2->rtArmFinish, false);
            }
            else if(DoTestHeadMotorDelay.Off())
            {
                COM2->SendCommToVision(COM2->rtArmFinish, false);
                ShowMyMessage("RTC Verify All Pass Time out");
                COM2->DoReleaseAndInspEnd();
                Task=1;
            }
            break;
        case 700:                                                               //通知Half view auto verify : Arm2
            COM2->SendCommToVision(COM2->rtArm2AllPassVerify, true);
            DoTestHeadMotorDelay.SetSecAndOn(10);
            DoTestHeadMotorDelay3.SetSecAndOn(0.5);
            Task=800;
            break;
        case 800:
//            if(COM2->bRealTimeCom_ReceiveOK[COM2->rtArm2AllPassVerify])
            if(Sen[SnRealTimeCCDIndexArm].IsOn())
            {
                if(DoTestHeadMotorDelay3.Off())
                {
                    COM2->bRealTimeCom_ReceiveOK[COM2->rtALLPASSOK]=false;
                    COM2->bRealTimeCom_ReceiveOK[COM2->rtALLPASSNG]=false;
                    Task=900;
                }
            }
            else if(DoTestHeadMotorDelay.Off())
            {
                ShowMyMessage("RTC Arm 2 Half Veiw all pass verify timeout");
                COM2->DoReleaseAndInspEnd();
                Task=1;
            }
            break;
        case 900:
            if(MOT[MTestY1].Z1UpZ2Down(MOT[MTestZ2].GailSpeed, TMode, false))
            {
                Task=1000;
            }
            break;
        case 1000:                                                              //等待RTC回傳verify結果
            if(COM2->bRealTimeCom_ReceiveOK[COM2->rtALLPASSOK])
            {
                Task=1100;
                COM2->SendCommToVision(COM2->rtArmFinish, false);
            }
            else if(COM2->bRealTimeCom_ReceiveOK[COM2->rtALLPASSNG])
            {
                Task=41400;
                COM2->SendCommToVision(COM2->rtArmFinish, false);
            }
            else if(DoTestHeadMotorDelay.Off())
            {
                COM2->SendCommToVision(COM2->rtArmFinish, false);
                ShowMyMessage("RTC Verify All Pass Time out");
                COM2->DoReleaseAndInspEnd();
                Task=1;
            }
            break;
        case 1100:
            if(MOT[MTestZ1].Gali_Two_ZAxis_Move(Prod.TestZ1_Safe, iSpeedSlow, "DoHalfViewAllPassVerifyRTC 1100"))
            {
                Task=1200;
            }
            break;
        case 1200:
            if(MOT[MTestY1].GalilTwoY_Move(Prod.TestY1_Front, Prod.TestY2_Rear, iSpeedY, "DoHalfViewAllPassVerifyRTC 1200"))
            {
                Task=1;
                return true;
            }
            break;
        case 41400:
            if(IndexAlarmInArmAway()==true)
            {
                bIsTestSitICFallDown=true;
                ShowMyMessage("RTC Half Veiw Verify All Pass NG!!");
                COM2->DoReleaseAndInspEnd();
                Task=1;
            }
            break;
    }
    return false;
}
#endif // GOLDEN VERBATIM -- golden atester.cpp:11297-11491  (GATE G-PTk3-DoHalfViewAllPassVerifyRTC, end)
bool DoHalfViewAllPassVerifyRTC(bool bInitial)                                  //JerryYang 20220215 : RTC Auto Verify half view check
{
#if 0 // TODO(W7) -- golden :11297-11493 (RTC half-view verify SM)
#endif
    (void)bInitial;
    return false;                                                              // golden default
}
//------------------------------------------------------------------------------
// ===========================================================================
//  GOLDEN VERBATIM PAIR -- DoHalfViewAllFailVerifyRTC
//  GATED : golden atester.cpp:11494-11698 (205 lines), inert reference text.
//  LIVE  : the slim DoHalfViewAllFailVerifyRTC() immediately AFTER the #endif below.  It is
//          UNCHANGED by this wave -- net behaviour change is ZERO.
//  WHY   : the census scored this "translated" because a same-named LIVE body
//          existed, without comparing SIZE.  Golden's 205 lines were NOWHERE in
//          the tree -- lost text, not deferred behaviour.  Now the text EXISTS
//          and is auditable, so a later un-gate is mechanical.
//  RULES : nothing inside the gate is fixed, renamed, reflowed or reindented;
//          golden's own defects and misspellings are preserved so a diff
//          against golden stays EMPTY.  Being gated it needs NO callee to
//          exist -- no stub, declaration or header was added for it.
//  SHAPE : same pair shape as csystem.cpp DoTrayFeedProcess / MainProc /
//          DoAllProcess (PT-W6a) golden-verbatim gates.
// ===========================================================================
#if 0 // GOLDEN VERBATIM -- golden atester.cpp:11494-11698 (205 lines).  GATE G-PTk2-DoHalfViewAllFailVerifyRTC.  NOT COMPILED: the ACTIVE DoHalfViewAllFailVerifyRTC() is the slim body immediately after this #endif.
bool DoHalfViewAllFailVerifyRTC(bool bInitial)
{
    if(bInitial==true)
    {
        iDoHalfViewAllFailVerifyTask=1;
        return false;
    }
    bool TMode=false;

    int &Task=iDoHalfViewAllFailVerifyTask;
    switch(Task)
    {
        case 1:
            if(MOT[MTestZ1].Gali_Two_ZAxis_Move(Prod.TestZ1_Safe, iSpeedSlow, "DoHalfViewAllFailVerifyRTC 1"))
            {
                Task=100;
            }
            break;
        case 100:
            if(MOT[MTestY1].GalilTwoY_Move(Prod.TestY1_Front, Prod.TestY2_Middle, iSpeedY, "DoHalfViewAllFailVerifyRTC 100"))
            {
                if(iContactMode==CONTACT_NORMAL)
                {
                    bHalfViewVerifyNeedAboveSocket=true;
                    DoSetupSystemToProd();
                }
                else
                {
                    Prod.TestZ1_Test=Get0_01MMType(fContact->edContactHeight1->Text.c_str())+Get0_01MMType(fContact->edDropOffset1->Text.c_str())+3000;
                    Prod.TestZ2_Test=Get0_01MMType(fContact->edContactHeight2->Text.c_str())+Get0_01MMType(fContact->edDropOffset2->Text.c_str())+3000;
                }
                Task=200;
            }
            break;
        case 200:
            if(iContactMode==CONTACT_NORMAL)
            {
                bHalfViewVerifyNeedAboveSocket=true;
                DoSetupSystemToProd();
            }
            else
            {
                Prod.TestZ1_Test=Get0_01MMType(fContact->edContactHeight1->Text.c_str())+Get0_01MMType(fContact->edDropOffset1->Text.c_str())+3000;
                Prod.TestZ2_Test=Get0_01MMType(fContact->edContactHeight2->Text.c_str())+Get0_01MMType(fContact->edDropOffset2->Text.c_str())+3000;
            }

            if(MOT[MTestZ2].Gali_MotMove(Prod.TestZ2_Test-Prod.TestZ2_Drop_Offset, iSpeedSlow))
            {
                IndexStatus=Z1Up_Z2Down;
                Task=300;
            }
            break;
        case 300:                                                               //通知Half view auto verify : Arm1
            COM2->SendCommToVision(COM2->rtArm1AllFailVerify, true);
            DoTestHeadMotorDelay.SetSecAndOn(10);
            DoTestHeadMotorDelay3.SetSecAndOn(0.5);
            Task=400;
            break;
        case 400:
//            if(COM2->bRealTimeCom_ReceiveOK[COM2->rtArm1AllFailVerify])
            if(Sen[SnRealTimeCCDIndexArm].IsOff())
            {
                if(DoTestHeadMotorDelay3.Off())
                {
                    COM2->bRealTimeCom_ReceiveOK[COM2->rtALLFAILOK]=false;
                    COM2->bRealTimeCom_ReceiveOK[COM2->rtALLFAILNG]=false;
                    Task=500;
                }
            }
            else if(DoTestHeadMotorDelay.Off())
            {
                ShowMyMessage("RTC Arm 1 Half Veiw all fail verify timeout");
                COM2->DoReleaseAndInspEnd();
                Task=1;
            }
            break;
        case 500:
            if(DeviceForm.ContactMode==TMove ||
               DeviceForm.ContactMode==TMoveDrop ||                             //jou 2012-02-03 新增T Move Drop
               DeviceForm.ContactMode==TMoveDropSlowContact ||
               DeviceForm.ContactMode==TMoveSlowContact)                        //Steven 20160130 : TMove Soft contact
            {
                TMode=true;
            }
            else
            {
                //jou 2012-02-29 index pick up error,index arm move to center & alarm
                if(IniConfig.bIndexPickupErrStop==true && bIndexArm2PickupErrStop==true)
                {
                    TMode=true;
                }
                else
                {
                    TMode=false;
                }
            }

            if(MOT[MTestY1].Z1DownZ2Up(MOT[MTestZ1].GailSpeed, TMode, false))
            {
                Task=600;
            }
            break;
        case 600:                                                               //等待RTC回傳verify結果
            if(COM2->bRealTimeCom_ReceiveOK[COM2->rtALLFAILOK])
            {
                Task=700;
                COM2->SendCommToVision(COM2->rtArmFinish, false);
            }
            else if(COM2->bRealTimeCom_ReceiveOK[COM2->rtALLFAILNG])
            {
                Task=41400;
                COM2->SendCommToVision(COM2->rtArmFinish, false);
            }
            else if(DoTestHeadMotorDelay.Off())
            {
                COM2->SendCommToVision(COM2->rtArmFinish, false);
                ShowMyMessage("RTC Verify All Fail Time out");
                COM2->DoReleaseAndInspEnd();
                Task=1;
            }
            break;
        case 700:                                                               //通知Half view auto verify : Arm2
            COM2->SendCommToVision(COM2->rtArm2AllFailVerify, true);
            DoTestHeadMotorDelay.SetSecAndOn(10);
            DoTestHeadMotorDelay3.SetSecAndOn(0.5);
            Task=800;
            break;
        case 800:
//            if(COM2->bRealTimeCom_ReceiveOK[COM2->rtArm2AllFailVerify])
            if(Sen[SnRealTimeCCDIndexArm].IsOn())
            {
                if(DoTestHeadMotorDelay3.Off())
                {
                    COM2->bRealTimeCom_ReceiveOK[COM2->rtALLFAILOK]=false;
                    COM2->bRealTimeCom_ReceiveOK[COM2->rtALLFAILNG]=false;
                    Task=900;
                }
            }
            else if(DoTestHeadMotorDelay.Off())
            {
                ShowMyMessage("RTC Arm 2 Half Veiw all fail verify timeout");
                COM2->DoReleaseAndInspEnd();
                Task=1;
            }
            break;
        case 900:
            if(MOT[MTestY1].Z1UpZ2Down(MOT[MTestZ2].GailSpeed, TMode, false))
            {
                Task=1000;
            }
            break;
        case 1000:                                                              //等待RTC回傳verify結果
            if(COM2->bRealTimeCom_ReceiveOK[COM2->rtALLFAILOK])
            {
                Task=1100;
                COM2->SendCommToVision(COM2->rtArmFinish, false);
            }
            else if(COM2->bRealTimeCom_ReceiveOK[COM2->rtALLFAILNG])
            {
                Task=41400;
                COM2->SendCommToVision(COM2->rtArmFinish, false);
            }
            else if(DoTestHeadMotorDelay.Off())
            {
                COM2->SendCommToVision(COM2->rtArmFinish, false);
                ShowMyMessage("RTC Verify All Fail Time out");
                COM2->DoReleaseAndInspEnd();
                Task=1;
            }
            break;
        case 1100:
            if(MOT[MTestZ1].Gali_Two_ZAxis_Move(Prod.TestZ1_Safe, iSpeedSlow, "DoHalfViewAllFailVerifyRTC 1100"))
            {
                Task=1200;
            }
            break;
        case 1200:
            if(MOT[MTestY1].GalilTwoY_Move(Prod.TestY1_Front, Prod.TestY2_Rear, iSpeedY, "DoHalfViewAllFailVerifyRTC 1200"))
            {
                Task=1;
//                if(iContactMode==CONTACT_NORMAL)
//                {
                    bHalfViewVerifyNeedAboveSocket=false;
                    DoSetupSystemToProd();
//                }
//                else
//                {
//                    Prod.TestZ1_Test=Get0_01MMType(fContact->edContactHeight1->Text.c_str());
//                    Prod.TestZ2_Test=Get0_01MMType(fContact->edContactHeight2->Text.c_str());
//                }
                return true;
            }
            break;
        case 41400:
            if(IndexAlarmInArmAway()==true)
            {
                bIsTestSitICFallDown=true;
                ShowMyMessage("RTC Half Veiw Verify All Fail NG!!");
                COM2->DoReleaseAndInspEnd();
                Task=1;
            }
            break;
    }
    return false;
}
#endif // GOLDEN VERBATIM -- golden atester.cpp:11494-11698  (GATE G-PTk2-DoHalfViewAllFailVerifyRTC, end)
bool DoHalfViewAllFailVerifyRTC(bool bInitial)
{
#if 0 // TODO(W7) -- golden :11494-11699 (RTC half-view verify SM)
#endif
    (void)bInitial;
    return false;                                                              // golden default
}
//------------------------------------------------------------------------------
void DoTemperatureRise(int iArm, bool bTemperatureRise)                         //Ifor 20230418 add
{
#if 0 // TODO(W7) -- golden :11700-11734 (temperature-rise, Temperature/ATC)
#endif
    (void)iArm; (void)bTemperatureRise;
}
//------------------------------------------------------------------------------
void TriggerATC_FFC_Function(bool bEnabled)                                     //Ifor 20240507 add :FFC Trigger Even
{
#if 0 // TODO(W7) -- golden :11735-11749 (ATC FFC trigger)
#endif
    (void)bEnabled;
}
//------------------------------------------------------------------------------
void DoCheckHasTestTempChange()                                                 //Ifor 20230505 add: 確認是否測試中有切換溫度並切回原生產溫度
{
#if 0 // TODO(W7) -- golden :11750-11771 (test-temp-change check, Temperature)
#endif
}
//------------------------------------------------------------------------------
// ===========================================================================
//  GOLDEN VERBATIM PAIR -- TestProcessSetToErr
//  GATED : golden atester.cpp:11772-11818 (47 lines), inert reference text.
//  LIVE  : the slim TestProcessSetToErr() immediately AFTER the #endif below.  It is
//          UNCHANGED by this wave -- net behaviour change is ZERO.
//  WHY   : the census scored this "translated" because a same-named LIVE body
//          existed, without comparing SIZE.  Golden's 47 lines were NOWHERE in
//          the tree -- lost text, not deferred behaviour.  Now the text EXISTS
//          and is auditable, so a later un-gate is mechanical.
//  RULES : nothing inside the gate is fixed, renamed, reflowed or reindented;
//          golden's own defects and misspellings are preserved so a diff
//          against golden stays EMPTY.  Being gated it needs NO callee to
//          exist -- no stub, declaration or header was added for it.
//  SHAPE : same pair shape as csystem.cpp DoTrayFeedProcess / MainProc /
//          DoAllProcess (PT-W6a) golden-verbatim gates.
// ===========================================================================
#if 0 // GOLDEN VERBATIM -- golden atester.cpp:11772-11818 (47 lines).  GATE G-PTk4-TestProcessSetToErr.  NOT COMPILED: the ACTIVE TestProcessSetToErr() is the slim body immediately after this #endif.
void TestProcessSetToErr(AnsiString sErrorLog)                                  //JerryYang 20231208 : 包起來
{
    AnsiString asTestTimeOut="";
    bTestDuplicateErr=false;
    int iTest=0;
    int iUnTest=0;
    int iNullIC=0;
    for(int i=0; i<TestSocket.iShtRow; i++)
    {
        for(int j=0; j<TestSocket.iShtCol; j++)
        {
            if(TestSocket.Item[i][j]!=NULL_IC &&                                //jou 2011-04-19 只要按SKIP就全部IC歸R道
               TestSocket.Item[i][j]!=HAS_NULL_IC)
            {
                if(iTesterBIN[i][j]==-1 && TestSocket.Item[i][j]<TEST_PASS)
                {
                    iUnTest++;
                }
                else
                {
                    iTest++;
                }

                iTesterBIN[i][j]=999;
                TestSocket.SetItemData(i, j, TEST_PASS+iTesterBIN[i][j]);
                TestSocket.iBinData[i][j]=iTestBinCount;                        //Steven 20190116 : 修正顯示錯誤
                TestSocket.PordRec[i][j].AddTestResultRecord(iTesterBIN[i][j], TestSocket.cSBin[i][j], "TestTimeOut_SKIP");                                     //Frank 20160505 add
            }
            else
            {
                iNullIC++;
            }
        }
    }

    IsTest=false;                                                               //Steven 20110722 Start : Skip時,要重置GPIB測試狀態

    asTestTimeOut.printf("%s : Test=%d ; UnTest=%d ; NullIC=%d", sErrorLog, iTest, iUnTest, iNullIC);
    MyDBIProcess("Message", asTestTimeOut);
    MyDBIProcess("Message", asRecordTestResult);

    bTestingStopAllMotor=false;                                                 //jou 2013-09-25 Testing Need Stop All Motor
    if((CosFunction.bUSEJCETSiteMapMode==true || CUSTOMER_CODE==CC_ASE_M) && LastSet.iRunStartMode==rsmAutoSiteMap)     //JerryYang 20170316 (Steven) 避免auto site mapping時發生tester time out後沒有清除旗標,造成in arm沒有重新吸料會hang up//Ifor 20180417 : add ASE_M
    {
        DoJCETSiteMappingCHK(false);
    }
}
#endif // GOLDEN VERBATIM -- golden atester.cpp:11772-11818  (GATE G-PTk4-TestProcessSetToErr, end)
void TestProcessSetToErr(AnsiString sErrorLog)                                  //JerryYang 20231208 : 包起來
{
#if 0 // TODO(W7) -- golden :11772-11819 (set-test-to-error, fMain/Prod)
#endif
    (void)sErrorLog;
}
//------------------------------------------------------------------------------
//==============================================================================
//==  ACTIVE offline stubs for the remaining golden tester-engine surface that
//==  is declared in atester.h but whose golden body is MOT[]/cross-module/UI/
//==  tester-comm-bound with no translated home this wave.  Offline-safe defaults
//==  keep the engine linkable.  Golden file:line cited per symbol.  TODO(W7/W5).
//==============================================================================
bool ProcessPauseTester()                       { return false; }              // golden -- no pause offline (atester.h:57)
// ===========================================================================
//  GOLDEN VERBATIM PAIR -- bContSoftEpSwitch
//  GATED : golden atester.cpp:9714-9789 (76 lines), inert reference text.
//  LIVE  : the slim bContSoftEpSwitch() immediately AFTER the #endif below.  It is
//          UNCHANGED by this wave -- net behaviour change is ZERO.
//  WHY   : the census scored this "translated" because a same-named LIVE body
//          existed, without comparing SIZE.  Golden's 76 lines were NOWHERE in
//          the tree -- lost text, not deferred behaviour.  Now the text EXISTS
//          and is auditable, so a later un-gate is mechanical.
//  RULES : nothing inside the gate is fixed, renamed, reflowed or reindented;
//          golden's own defects and misspellings are preserved so a diff
//          against golden stays EMPTY.  Being gated it needs NO callee to
//          exist -- no stub, declaration or header was added for it.
//  SHAPE : same pair shape as csystem.cpp DoTrayFeedProcess / MainProc /
//          DoAllProcess (PT-W6a) golden-verbatim gates.
// ===========================================================================
#if 0 // GOLDEN VERBATIM -- golden atester.cpp:9714-9789 (76 lines).  GATE G-PTk2-bContSoftEpSwitch.  NOT COMPILED: the ACTIVE bContSoftEpSwitch() is the slim body immediately after this #endif.
void __fastcall bContSoftEpSwitch(int Index, bool Arm)
{
    if(fiosetview->fShow==true)                                                 //IO畫面
    {
        if(Index==0)
        {
            if(SW[SwEpArm1].Enable==true)                                       //Steven 20110708
            {
                if(FrontTestHeadHasIC())                                        //Index 1 有IC的話,狀態不可以改變
                    SW[SwEpArm1].OnOff(Arm);
            }
        }
        else if(SW[SwEpArm2].Enable==true)                                      //Index 2 有IC的話,狀態不可以改變
        {
            if(RearTestHeadHasIC())
                SW[SwEpArm2].OnOff(Arm);
        }
    }
    else
    {
        if(Index==0)
        {
            if(TestIF_File.iShuttleMode==1 &&
               TestIF_File.iShuttle_Sel==1 &&
               MachineTypeChoice==Type_HT9046_LS)
            {
                if(IniConfig.bD58UseArm1PickPlaceArm2Test==true &&              //Steven 20250417 : fixed for [D58]
                   TestIF_File.bArm1PickPlaceArm2Test==true)
                {
                }
                else
                {
                    SW[SwEpArm1].OnOff(false);                                  //kevin 20220312 有電磁閥沒使用關ARM 關電磁閥
                    return;
                }
            }

            if(SW[SwEpArm1].Enable==true)                                       //Steven 20110708
            {
                SW[SwEpArm1].OnOff(Arm);
            }

            if(SW[SwIndEpArm1].Enable==true)                                    //Steven 20110708
            {
                SW[SwIndEpArm1].OnOff(Arm);
            }
        }
        else
        {
            if(TestIF_File.iShuttleMode==1 &&
               TestIF_File.iShuttle_Sel==0 &&
               MachineTypeChoice==Type_HT9046_LS)                               //ARM 2 close Arm1
            {
                if(IniConfig.bD58UseArm1PickPlaceArm2Test==true &&              //Steven 20250417 : fixed for [D58]
                   TestIF_File.bArm1PickPlaceArm2Test==true)
                {
                }
                else
                {
                    SW[SwEpArm2].OnOff(false);                                  //kevin 20220312 有電磁閥沒使用關ARM 關電磁閥
                    return;
                }
            }

            if(SW[SwEpArm2].Enable==true)
            {
                SW[SwEpArm2].OnOff(Arm);
            }

            if(SW[SwIndEpArm2].Enable==true)                                    //Steven 20110708
            {
                SW[SwIndEpArm2].OnOff(Arm);
            }
        }
    }
}
#endif // GOLDEN VERBATIM -- golden atester.cpp:9714-9789  (GATE G-PTk2-bContSoftEpSwitch, end)
void bContSoftEpSwitch(int Index, bool Arm)                                     //kevin 20130608 soft contactg使用 (W6.4: __fastcall removed)
{
#if 0 // TODO(W7) -- golden EPSwitch soft-contact (MOT[]/EP DAQ)
#endif
    (void)Index; (void)Arm;
}
//------------------------------------------------------------------------------
//  SendSiteMapToRTC (golden :5471-5561) / CheckTwoArmSiteMap (:5451) -- RTC
//  sitemap send + two-arm sitemap diff.  CheckTwoArmSiteMap is ACTIVE (substrate
//  only); SendSiteMapToRTC is gated (COM2/RTC).
//------------------------------------------------------------------------------
// ===========================================================================
//  GOLDEN VERBATIM PAIR -- SendSiteMapToRTC
//  GATED : golden atester.cpp:5471-5549 (79 lines), inert reference text.
//  LIVE  : the slim SendSiteMapToRTC() immediately AFTER the #endif below.  It is
//          UNCHANGED by this wave -- net behaviour change is ZERO.
//  WHY   : the census scored this "translated" because a same-named LIVE body
//          existed, without comparing SIZE.  Golden's 79 lines were NOWHERE in
//          the tree -- lost text, not deferred behaviour.  Now the text EXISTS
//          and is auditable, so a later un-gate is mechanical.
//  RULES : nothing inside the gate is fixed, renamed, reflowed or reindented;
//          golden's own defects and misspellings are preserved so a diff
//          against golden stays EMPTY.  Being gated it needs NO callee to
//          exist -- no stub, declaration or header was added for it.
//  SHAPE : same pair shape as csystem.cpp DoTrayFeedProcess / MainProc /
//          DoAllProcess (PT-W6a) golden-verbatim gates.
// ===========================================================================
#if 0 // GOLDEN VERBATIM -- golden atester.cpp:5471-5549 (79 lines).  GATE G-PTk2-SendSiteMapToRTC.  NOT COMPILED: the ACTIVE SendSiteMapToRTC() is the slim body immediately after this #endif.
int SendSiteMapToRTC(bool bSendToRTC, int iSelArm)
{
//    Handler 控制開關Site :
//    1~4 碼   @MAP
//      5 碼   0 ~ 6
//                    ========================================================
//                    第5碼        Site 分佈樣式
//                    ========================================================
//                    asSiteMap    0         1 x 1
//                                 1         1 x 2
//                                 2         1 x 4
//                                 3         2 x 2
//                                 7         2 x 3
//                                 4         2 x 4
//                                 5         2 x 5
//                                 5         2 x 6
//                                 6         2 x 8
//                    ========================================================
//    6~13碼   asRowA       11111111  Aa ~ Ah  0 代表關Site , 1 代表開Site
//   14~21碼   asRowB       11111111  Ba ~ Bh  0 代表關Site , 1 代表開Site
//      22碼   +

    int iSite1=0, iSite2=0, iTotalSite=0;
    AnsiString asRow1="", asRow2="";

    int iMode=GetRTCSiteMap();

    if(iSelArm==0 || IniConfig.bA09_ByArmCloseSite==false)
    {
        for(int j=0; j<MAX_SOCKET_COL; j++)                                     //JerryYang 20250818 : fix 格式錯誤造成timeout
        {
            asRow1+=(LastSet.bUseTestSocket[0][0][j] || LastSet.bUseTestSocket[1][0][j])?"1":"0";
            asRow2+=(LastSet.bUseTestSocket[0][1][j] || LastSet.bUseTestSocket[1][1][j])?"1":"0";

            if(LastSet.bUseTestSocket[0][0][j] || LastSet.bUseTestSocket[1][0][j])
                iSite1++;
            if(LastSet.bUseTestSocket[0][1][j] || LastSet.bUseTestSocket[1][1][j])
                iSite2++;
        }
    }
    else if(iSelArm==1)
    {
        for(int j=0; j<MAX_SOCKET_COL; j++)                                     //JerryYang 20250818 : fix 格式錯誤造成timeout
        {
            asRow1+=(LastSet.bUseTestSocket[0][0][j])?"1":"0";
            asRow2+=(LastSet.bUseTestSocket[0][1][j])?"1":"0";

            if(LastSet.bUseTestSocket[0][0][j])
                iSite1++;
            if(LastSet.bUseTestSocket[0][1][j])
                iSite2++;
        }
    }
    else if(iSelArm==2)
    {
        for(int j=0; j<MAX_SOCKET_COL; j++)                                     //JerryYang 20250818 : fix 格式錯誤造成timeout
        {
            asRow1+=(LastSet.bUseTestSocket[1][0][j])?"1":"0";
            asRow2+=(LastSet.bUseTestSocket[1][1][j])?"1":"0";

            if(LastSet.bUseTestSocket[1][0][j])
                iSite1++;
            if(LastSet.bUseTestSocket[1][1][j])
                iSite2++;
        }
    }

    if(bSendToRTC==true)
    {
        COM2->sRealTimeCom_Send[COM2->rtSiteMap] = "@MAP"+AnsiString(iMode)+asRow1+asRow2+"+";
        COM2->SendCommToVision(COM2->rtSiteMap, true);
        return iMode;
    }
    else
    {
        iTotalSite=iSite1+iSite2;
        return iTotalSite;
    }
}
#endif // GOLDEN VERBATIM -- golden atester.cpp:5471-5549  (GATE G-PTk2-SendSiteMapToRTC, end)
int SendSiteMapToRTC(bool bSendToRTC, int iSelArm)                              //jou 2014-06-24 RTC 自動進行Model驗證
{
#if 0 // TODO(W7) -- golden :5471-5561 (RTC sitemap send, COM2/RTC)
#endif
    (void)bSendToRTC; (void)iSelArm;
    return 0;                                                                  // golden default
}
bool CheckTwoArmSiteMap()                                                       //jou 2014-06-24
{
    bool bFlag=false;
    for(int i=0; i<FTestSuck.iShtRow; i++)
    {
        for(int j=0; j<FTestSuck.iShtCol; j++)
        {
            if(LastSet.bUseTestSocket[0][i][j] ^ LastSet.bUseTestSocket[1][i][j])
            {
                bFlag=true;
            }
        }
    }
    return bFlag;
}
//---------------------------------------------------------------------------
//  WAVE SCOPE EXTENSION -- PT-Wk (APPEND-ONLY; the head banner above is
//  deliberately left byte-for-byte untouched, per this wave's append-only rule)
//  Translator: AI(PT-Wk-atester) 20260811
//  Golden source: HT9011UC_Code_V3.33.906.0_20260618/atester.cpp (cp950)
//
//  ROLE
//  ----
//  Two further golden atester.cpp free functions, appended in GOLDEN ORDER
//  (:699 before :5373):
//    * Sim_TTL_Single(int,int)   golden :699   GATED  -- offline TTL BIN
//      simulator; sources its answer from fMain->cbSimuBinSite0..3, four
//      TComboBox widgets the FormsFacade fMain stand-in does not carry.
//    * CheckIndexArmInitState()  golden :5373  ACTIVE -- index-arm INTERLOCK
//      predicate (were the two index test nozzles in a legal vacuum state at
//      init?).  Every dependency it needs is REAL in this tree; nothing in it
//      is degraded, so it cannot answer a false "state OK".
//
//  WHO PUMPS THEM
//  --------------
//    * Sim_TTL_Single -- golden's ONLY call site is atester.cpp:2079, wrapped in
//      `#ifdef SOFT_SIMULTE`.  This port mirrors that verbatim at port
//      :2064-2066 and SOFT_SIMULTE is NOT defined, so the call is compiled out
//      in BOTH trees.  The function is compiled-but-uncalled here exactly as it
//      is compiled-but-uncalled in a golden release (non-SOFT_SIMULTE) build.
//    * CheckIndexArmInitState -- golden's ONLY call site is atester.cpp:5751,
//      `case 600000:` of DoTestHeadMotor ("flag=CheckIndexArmInitState(); if
//      (flag==true) Task=1;" -- i.e. a TRUE return sends the index SM back to
//      task 1 instead of releasing it).  That case still sits inside this port's
//      GATED down-press tree, so nothing calls it YET.  Because both live in
//      THIS translation unit and the call site is ABOVE this definition, the
//      main loop must publish `bool CheckIndexArmInitState();` (golden needs no
//      such declaration: golden defines it at :5373, above the :5751 use) before
//      un-gating case 600000.  NOT done here -- append-only, atester.h is not
//      one of my targets.
//
//  GATE REGISTER (this extension only)
//  -----------------------------------
//  G-PTk1 -- Sim_TTL_Single body.
//    (a) GOLDEN LINE: atester.cpp:702
//          TComboBox *ComboCH[4]={fMain->cbSimuBinSite0, fMain->cbSimuBinSite1,
//                                 fMain->cbSimuBinSite2, fMain->cbSimuBinSite3};
//        plus the nine `ComboCH[iCH]->Text=="N"` comparisons at :707-748 that
//        read through it.  Whole body :701-752 gated as one unit.
//    (b) WHY THE OFFLINE DEFAULT IS FAITHFUL: golden reaches this function from
//        exactly ONE place, atester.cpp:2079, and that call is inside
//        `#ifdef SOFT_SIMULTE`.  SOFT_SIMULTE is off in a golden release build
//        and is not defined anywhere in this port, so the function is
//        UNREACHABLE in both trees.  A body that does nothing is therefore
//        observationally identical to golden-as-shipped.  The complete golden
//        body is retained VERBATIM inside the `#if 0` below so a later wave has
//        the exact text -- the same "GOLDEN VERBATIM ... (GATE ..., end)" idiom
//        this file already uses for SendSiteMapToRTC (port :11866-11942).
//    (c) HOW REAL-MACHINE BEHAVIOUR DIFFERS: in a SOFT_SIMULTE build golden
//        OVERWRITES ScanPort[iSite*5 .. iSite*5+4] with the bit pattern the
//        operator selected in cbSimuBinSite<n> -- an offline "fake the tester's
//        BIN reply" affordance.  The gated body writes nothing, so those 5 bits
//        keep whatever ScanPortRefresh (port :745-752) last read from the REAL
//        Sen[] TTL inputs.  That is the cautious direction: real sensor data is
//        preserved and no BIN verdict is fabricated.
//    UN-GATE CONDITION: add cbSimuBinSite0..3 (TComboBox*) to the FormsFacade
//        fMain stand-in.  Absence re-verified at delivery -- see report.
//
//  G-PTk2 -- (none for CheckIndexArmInitState).  It is fully ACTIVE.  Noted for
//    the record, and NOT a gate I introduce: `IsNNMode()` (golden cinitial.h:60)
//    resolves to the pre-existing atester_shims.cpp body, which returns 0 (not
//    NN mode) offline.  It appears ONLY in `IndexSuckName[i+IsNNMode()][j]`
//    (golden :5410), i.e. it selects which nozzle NAME is appended to the
//    WAR0320 alarm TEXT.  It can never flip bHasFail1/bHasFail2 or the return
//    value, so the interlock verdict is unaffected; on an NN-mode machine only
//    the nozzle label printed inside the alarm would shift by one row.
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
//  golden atester.cpp:694-698 -- banner comment, transcribed verbatim
//---------------------------------------------------------------------------
//******************************************************************************
//
//  注意!! Sim_TTL_Single為TTL收發BIN相關, 修改時要小心!!
//
//******************************************************************************
// golden atester.cpp:699-753 -- GATED (GATE G-PTk1).  Body kept VERBATIM.
#if 0 // GOLDEN VERBATIM -- golden atester.cpp:699-753  (GATE G-PTk1-Sim_TTL_Single, begin)
void Sim_TTL_Single(int i, int j)                                               //Steven 20091031 Start : for TTL offline testing
{
    int iSenBitStatus[10]={0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    TComboBox *ComboCH[4]={fMain->cbSimuBinSite0, fMain->cbSimuBinSite1, fMain->cbSimuBinSite2, fMain->cbSimuBinSite3};

    int iSite=ArrayConvertSite(i, j);
    int iCH=TestIF.iSiteMap[i][j]-1;

    if(ComboCH[iCH]->Text=="1")
    {
        iSenBitStatus[0]=1;
    }
    else if(ComboCH[iCH]->Text=="2")
    {
        iSenBitStatus[1]=1;
    }
    else if(ComboCH[iCH]->Text=="3")
    {
        iSenBitStatus[0]=1;
        iSenBitStatus[1]=1;
    }
    else if(ComboCH[iCH]->Text=="4")
    {
        iSenBitStatus[2]=1;
    }
    else if(ComboCH[iCH]->Text=="5")
    {
        iSenBitStatus[0]=1;
        iSenBitStatus[2]=1;
    }
    else if(ComboCH[iCH]->Text=="6")
    {
        iSenBitStatus[1]=1;
        iSenBitStatus[2]=1;
    }
    else if(ComboCH[iCH]->Text=="7")
    {
        iSenBitStatus[0]=1;
        iSenBitStatus[1]=1;
        iSenBitStatus[2]=1;
    }
    else if(ComboCH[iCH]->Text=="8")
    {
        iSenBitStatus[3]=1;
    }
    else if(ComboCH[iCH]->Text=="9")
    {
        iSenBitStatus[0]=1;
        iSenBitStatus[3]=1;
    }
    for(int k=0; k<5; k++)
    {
        ScanPort[k+iSite*5]=iSenBitStatus[k];
    }
}
#endif // GOLDEN VERBATIM -- golden atester.cpp:699-753  (GATE G-PTk1-Sim_TTL_Single, end)
void Sim_TTL_Single(int i, int j)                                               //Steven 20091031 Start : for TTL offline testing  -- golden :699
{
#if 0 // TODO(GA-3/UI) -- golden :701-752 (fMain->cbSimuBinSite0..3 absent from the FormsFacade fMain stand-in)
#endif
    // Offline: leave ScanPort untouched (see GATE G-PTk1 (c) above).  Golden's
    // sole call site is `#ifdef SOFT_SIMULTE` (golden :2078-2080 == port
    // :2064-2066), which is not defined, so this body is unreachable in both trees.
    (void)i; (void)j;
}
//------------------------------------------------------------------------------
// golden atester.cpp:5371 -- golden re-declares this here even though csystem.h
// already carries it (csystem.h:390, `int CheckSuckInitialStatus(class
// TMyKitSuck &Ptr, int iR, int iC)`); reproduced verbatim.
// TRAP 5 (two headers, same class name): the `TMyKitSuck` bound here is
// aHotPlateSubstrate.h:365 -- this TU includes "aHotPlateSubstrate.h" (port
// :93) and NOT mykitsuck.h.  The body that actually gets linked is
// csystem.cpp:22953, and csystem.cpp likewise includes ONLY
// "aHotPlateSubstrate.h" (csystem.cpp:103) with no mykitsuck.h -- so both sides
// of this call compile against the SAME layout.  FTestSuck/BTestSuck are
// declared by that same header, so the reference argument cannot straddle the
// two competing definitions.
//------------------------------------------------------------------------------
extern int CheckSuckInitialStatus(TMyKitSuck &Ptr, int iR, int iC);              // golden :5371
//------------------------------------------------------------------------------
// CheckIndexArmInitState -- golden atester.cpp:5373-5446.  ACTIVE, verbatim.
// INTERLOCK PREDICATE.  Returns TRUE when at least one index nozzle failed its
// initial vacuum-vs-Item cross-check; golden's caller (:5751, case 600000) sends
// the index SM back to Task=1 on TRUE.  Nothing in this body is degraded to an
// offline default, so it cannot answer a false "state OK": every input
// (LastSet.bUseTestSocket, IniConfig/TestIF_File flags, FTestSuck/BTestSuck
// geometry + Item[], CheckSuckInitialStatus) is the real tree object.
//------------------------------------------------------------------------------
bool CheckIndexArmInitState()
{
    int ret1[MAX_SOCKET_ROW][MAX_SOCKET_COL]={{0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0}};
    int ret2[MAX_SOCKET_ROW][MAX_SOCKET_COL]={{0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0}};
    bool bHasFail1=false, bHasFail2=false;
    AnsiString errPart1="at Index Arm 1";
    AnsiString errPart2="at Index Arm 2";

    for(int i=0; i<FTestSuck.iShtRow; i++)
    {
        for(int j=0; j<FTestSuck.iShtCol; j++)
        {
            if(LastSet.bUseTestSocket[0][i][j] ||                               //ChungHung 20130910 alter for SCK can close site by Index
               LastSet.bUseTestSocket[1][i][j])                                 //Steven 20100824 : 關Site的位置不做檢查
            {
                ret1[i][j]=CheckSuckInitialStatus(FTestSuck, i, j);

                if((IniConfig.bD58UseArm1PickPlaceArm2Test==true &&
                    TestIF_File.bArm1PickPlaceArm2Test==true) &&
                    TestIF_File.bCheckArm2Vacuum==false)                        //Steven 20150129 : 需要確認Arm2有沒有粘料   //Ifor 20200811 Fix: Arm1 Pick Place Arm2Test 需卡兩個條件
                {
                    ret2[i][j]=0;                                               //kevin 20150128
                }
                else
                {
                    ret2[i][j]=CheckSuckInitialStatus(BTestSuck, i, j);
                }
            }
            else
            {
                ret1[i][j]=0;
                ret2[i][j]=0;
            }

            if(ret1[i][j]!=0)
            {
                bHasFail1=true;
                errPart1+=IndexSuckName[i+IsNNMode()][j];                       //Steven 20230712 : 修正NN mode alarm顯示
            }

            if(ret2[i][j]!=0)
            {
                bHasFail2=true;
                errPart2+=IndexSuckName[i][j];
            }
        }
    }

    if(bHasFail1)
    {
        if(CUSTOMER_CODE==CC_KYEC_LEE && bEnable_KLT_Function==false)  //Ifor 20220701 KYEC 要求WAR0320 需開門確認並按Z1//Eastsun 20260508 合入
            bIsTestSitICFallDown=true;
        ShowErrorMessage("WAR0320", K_SKIP, MTestZ1, false, errPart1);          //jou 2011-11-08 retry -> skip字義上比較恰當
    }

    if(bHasFail2)
    {
        if(CUSTOMER_CODE==CC_KYEC_LEE && bEnable_KLT_Function==false)  //Ifor 20220701 KYEC 要求WAR0320 需開門確認並按Z1//Eastsun 20260508 合入
            bIsTestSitICFallDown=true;
        ShowErrorMessage("WAR0320", K_SKIP, MTestZ2, false, errPart2);
    }

    for(int i=0; i<FTestSuck.iShtRow; i++)
    {
        for(int j=0; j<FTestSuck.iShtCol; j++)
        {
            if(ret1[i][j]==Vaccum_Initial_Off)
                FTestSuck.SetItemData(i, j, HAS_NULL_IC);
            if(ret2[i][j]==Vaccum_Initial_Off)
                BTestSuck.SetItemData(i, j, HAS_NULL_IC);
        }
    }
    return (bHasFail1 || bHasFail2);
}
//------------------------------------------------------------------------------
// AI(W906-W11) 20260927 (St02): Steven W11 = B -- the T02 hook (GetTesterResult case 1, golden atester.cpp:882
//   fStartCondition->SocketIDLog()).  St01 installs FileRW/StartCondition.cpp's SocketIDLog body at wb_serve boot; 0 = not
//   installed, the call is skipped (no socket-ID CSV line), exactly as while T02 was gated.
void (*W906_SocketIDLogBody)() = 0;
