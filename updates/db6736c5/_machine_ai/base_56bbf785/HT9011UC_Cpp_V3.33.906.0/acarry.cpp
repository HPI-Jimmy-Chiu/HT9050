// =============================================================================
//  acarry.cpp  --  SHUTTLE / CARRY ENGINE  (W6.5 translation)
//
//  Faithful translation of golden  acarry.cpp (8521 lines, BCB6, Big5/cp950).
//  Translator: AI(W6.5-CARRY) 20260626
//
//  ROLE: the SHUTTLE / CARRY ENGINE.  Owns the two input-shuttle master state
//  machines Do_Auto_SHT1 (AutoSHT1Task) / Do_Auto_SHT2 (AutoSHT2Task) -- left/
//  right movement, rotate-check, 2D-barcode, floating, null/fly/residual-IC
//  checks, retry/skip/home -- the 9046AU sort-shuttle Do_Auto_SHT3 (AutoSHT3Task),
//  the residual/sensor-broken/null-IC detectors keyed on TestIF.iTestMode, the
//  In/Out-arm-vs-shuttle move-safe interlocks, the pure pitch geometry, and the
//  shuttle sensor-poll thread logic.  Dispatched (NOT a thread) from the csystem
//  main process loop: if(bDoProcess) Do_Auto_SHT1(); else Do_Auto_SHT2(); then
//  if(USE_OUT_SORT_ARM!=eartUninstall) Do_Auto_SHT3();
//
//  WAVE SCOPE (W6.5) -- ACTIVE vs satisfied-by-shim:
//   ACTIVE (faithful, verbatim formulas / branches / cursor flow / fall-throughs /
//   `return;` MoveSafe yields / #ifdef SOFT_SIMULTE blocks -- SOFT_SIMULTE is NOT
//   defined so #ifndef paths run, #ifdef paths stay verbatim but inert):
//     * Do_Auto_SHT1 (AutoSHT1Task=1) -- ALL 39 cases ACTIVE.  case 200 FALLS
//         THROUGH to 201 (no break) -- preserved verbatim.  `return;` after
//         DoInOutARM_SHT_MoveSafe (yield-without-advance) -- preserved.
//     * Do_Auto_SHT2 (AutoSHT2Task=1) -- ALL 39 cases ACTIVE (symmetric set).
//     * Do_Auto_SHT3 (AutoSHT3Task=1) -- 6-case sort-shuttle SM ACTIVE; its
//         9046AU sort-arm substrate (OutSht3Kit / MOutSortSht / Prod.SortSHT /
//         MoveSortArmToAutoSafe) is supplied by offline shims.  Never dispatched
//         offline (USE_OUT_SORT_ARM==eartUninstall) but compiles+links ACTIVE.
//     * CheckShuttleOutputHasICError (switch TestIF.iTestMode, 16 site modes),
//         CheckShuttleSensorBroken_1/_2, CheckNullICShuttle1/2_9045,
//         CheckICUnderShuttle1/2_9045 (#ifdef/#ifndef SOFT_SIMULTE VERBATIM),
//         CheckShuttlePos1/2_M204, ShowOutputShuttleICStatus(_M204),
//         CheckShuttle1/2OutputHasICError_M204, IsFLCarrKitAllHasIC /
//         IsBLCarrKitAllHasIC, CheckCFixTrayFullPlace, the *_MoveSafe trio,
//         GetShuttle1/2Pitch_9045 (oracle), CheckShuttlePitch, DoMoveShuttleSensor,
//         CheckShuttle1/2EncoderPos, DoCheckShuttle1/2ICByLTC(_AutoLatch),
//         DoFloodGateClose, the watchdog, StartDetectMotorSensor, the helper
//         trio, and the uShuttleThread sensor-poll LOGIC (ThreadProcess VERBATIM).
//   SATISFIED-BY-SHIM (acarry_shims.{h,cpp} + FormsFacade):
//     * TECH Tech (6 OutSH*/iInSH* fields) ; fLtcSensor (TfLtcSensor) ; OutSht3Kit
//       (TMyKitSuck) ; ATC_InterfaceForm / ATC_Data -- offline-default so the
//       guarded paths take the deterministic no-detect / no-floodgate branch.
//     * the csystem.cpp / LaserSensorShuttle homes: CheckShtFloating /
//       UseInArmCheckShtFloating / UseOutArmCheckShtFloating, InShtInLF/RT(int),
//       SortSHTInLF/RT, DoStepShuttleCheck, DoCheckShuttle1/2EmptyIC / MustHasIC /
//       Prominent (+ Initial*Task), Check32siteOnlyEnabled*,
//       MoveOutArmXY_ToShuttleAlarmArea, SendHTTPRequest, MoveSortArmToAutoSafe,
//       Do_Auto_SHT1AsOutArmIsRotater (golden decl-only).
//     * FormsFacade fMain shuttle members (cbShowShuttleSensor /
//       cbTestOutShuttleSensor / cbShowInShuttleSensor / AddShuttleMessage /
//       htShullte0/1 / meShuttle1/2 / pgMain / emp7TabSheet21 / cInplace /
//       BtnOneCycleClick / BtnResetClick / Reset / JSCC_ResetForShuttleLoseIC /
//       ResetRecordforPiggyBack).
//     * ptrOutSHT (TMyKitSuck*) -- acarry-LOCAL file-scope pointer (owned here).
//   GATE (#if 0): NONE.  Every golden case body translated ACTIVE.
//
//  uSHUTTLE THREAD: the Win32 HThreadCtrlShuttle wrapper is a THIN OFFLINE DRIVER
//  (no OS thread spawned; methods inert).  The ThreadProcess sensor-poll switch
//  LOGIC is kept VERBATIM and callable.  Win32 HANDLE/DWORD/LPVOID/CALLBACK/
//  INFINITE/WAIT_* tokens come from acarry_shims.h (portable no-op typedefs).
//
//  VCL/Borland conversions: #pragma hdrstop + #pragma package(smart_init) dropped.
//  AnsiString (1-based) + AnsiString::sprintf via vclcompat (verbatim).  NO
//  __fastcall / __property / __published in the .cpp.  Big5 comments decoded via
//  cp950, emitted UTF-8, ZERO U+FFFD.
// =============================================================================

#include "MachineDefine.h"

#include "acarry.h"
#include "acarry_shims.h"          // W6.5 offline cross-module shims

#include "aArmHeader.h"
#include "Motor/mymotor.h"
#include "cprod.h"
#include "cpublic.h"
#include "csystem.h"
#include "atester.h"
#include "common.h"
#include "cmydef.h"
#include "aoutarm9045.h"
//  NOTE: ainarm9045.h is NOT included -- it redeclares InArmLeftSideNoIC/HasIC
//  with default args that aHotPlateSubstrate.h (pulled via acarry_shims.h) also
//  declares with defaults (a latent double-default-arg clash exposed only when
//  both headers land in one TU).  The 5 in-arm symbols acarry actually calls are
//  forward-declared in acarry_shims.h instead.
#include "mycylin.h"
#include "myswitch.h"
#include "mysensor.h"
#include "FormsFacade.h"
#include "myTimer.h"
#include "canary_support.h"        // LastSet / ShowMyMessage / ShowErrorMessage / RecordProcess
#include <cstring>
#include <cstdlib>
//------------------------------------------------------------------------------
//  ptrOutSHT -- golden mykitsuck.cpp:43 (extern TMyKitSuck *ptrOutSHT; mykitsuck.h:381).
//  acarry's residual checks assign &FRCarryKit/&BRCarryKit to it; the W6.5 wave
//  OWNS this file-scope pointer (mykitsuck.cpp is not translated yet).
// AI(W906-A4-6) 20260924 退役（mykitsuck.cpp 有 golden 定義，本顆起註冊；兩份會重複定義）：TMyKitSuck *ptrOutSHT = 0;
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//暫時先關閉1x2 , 1x4上下偵測模式 , 因為沒有實際驗證過
//------------------------------------------------------------------------------
bool b1ShuttleMoveToLeft=false;
bool b2ShuttleMoveToLeft=false;
bool b1ShuttleMoveToRight=false;
bool b2ShuttleMoveToRight=false;
bool bSortShtMoveToLeft=false;                                                  //RogerYang 20250417 for HT9046AU add
bool bSortShtMoveToRight=false;
bool bSHTOfsChangeRight[3];                                                     //RogerYang 20250417 for HT9046AU add index //Sam 20221205 : 修正 Shuttle 移動完成後又被修改 Offset 導致 Hang up
bool bSHTOfsChangeLeft[3];                                                      //RogerYang 20250417 for HT9046AU add index //Sam 20230202 : 修正 Shuttle Left 移動完成後又被修改 Offset 導致 Hang up
bool bShuttle1CatchSignalR=false;
bool bShuttle2CatchSignalR=false;
bool bShuttle1CatchSignalL=false;
bool bShuttle2CatchSignalL=false;

bool bSh1HasFloatingError=false;
bool bSh2HasFloatingError=false;

bool bShuttleHasIC[2][4][8];//[Shuttle][siteX][siteY]
bool bEnter[2][4][8];
int motorpos;

int  AutoSHT1Task=1,AutoSHT2Task=1,AutoSHT3Task=1;                              //RogerYang 20250510 Add for 9046AU
TQPF_Timer HWaitForShuttleFix1,HWaitForShuttleFix2;
TQPF_Timer ShuttleDelay1, ShuttleDelay2;
TQPF_Timer RotShuttleDelay1, RotShuttleDelay2;

//AI(ht9045-staterecord-analysis) 20260430 (RogerYang) : Shuttle Move Timeout watchdog
//   case 120 MotorMove(iLeft) return 0 > 30 sec : step lost / sensor false-trigger loop
const double SHUTTLE_MOVE_TIMEOUT_SEC = 30.0;
TQPF_Timer ShtMoveTimeout1, ShtMoveTimeout2;                                    //Steven 20260507 : replace GetTickCount with TQPF_Timer
bool bShtMoveTimerRunning1 = false;                                              //Steven 20260507 : SHT1 watchdog running flag
bool bShtMoveTimerRunning2 = false;                                              //Steven 20260507 : SHT2 watchdog running flag

//const int SHT_LEFT              =100;   //向左移動
//const int SHT_LEFT_CHECK_NULL_IC=105;   //檢查Out Shuttle IC殘料
//const int SHT_LEFT_SPEED_DOWN   =110;   //複檢時要放慢速度
//const int SHT_LEFT_InShtCHECK_NULL_IC=115;  //kevin 20161108 檢查In Shuttle IC殘料 檢測
//const int SHT_LEFT_MOVE         =120;   //直接移到左邊等待區
//const int SHT_LEFT_OUT_RO_DELAY =124;   //轉轉蝦頭要檢查OutShuttle往左時有沒有轉頭的Delay
//const int SHT_LEFT_IN_RO_DELAY  =125;   //轉轉蝦頭要檢查InShuttle往左時有沒有轉頭的Delay
//const int SHT_LEFT_OUT_RO_CHECK =130;   //轉轉蝦頭要檢查OutShuttle往左時有沒有轉頭
//const int SHT_LEFT_IN_RO_CHECK  =135;   //轉轉蝦頭要檢查InShuttle往左時有沒有轉頭
//const int SHT_LEFT_STEP_SHUTTLE =140;   //jou 2013-07-16 Step Shuttle check Index -> Input
//const int SHT_LEFT_2DID_RESCAN  =145;   //Isacc 20180329 (Steven) 2DID All site fail recheck twice and alarm

//const int SHT_RIGHT             =200;
//const int SHT_RIGHT_CHECK_IN_SHT=202;   //Steven 20150414 : Index吸取異常,Shuttle退出後,要檢查置偏
//const int SHT_RIGHT_1           =201;
//const int SHT_RIGHT_MOVE        =210;   //向右移動到定位
//const int SHT_RIGHT_IN_RO_CHECK =220;   //轉轉蝦頭要檢查InShuttle往右時有沒有轉頭
//const int SHT_RIGHT_OUT_RO_CHECK=230;   //轉轉蝦頭要檢查OutShuttle往右時有沒有轉頭
//const int SHT_RIGHT_IN_RO_DELAY =225;   //轉轉蝦頭要檢查InShuttle往右時有沒有轉頭
//const int SHT_RIGHT_OUT_RO_DELAY=235;   //轉轉蝦頭要檢查OutShuttle往右時有沒有轉頭
//const int SHT_RIGHT_LEFT        =300;
//const int SHT_RIGHTWAIT         =400;
//const int SHT_RIGHTWAIT_ALARM   =500;
//const int SHT_RIGHT_IN_SFC      =2300;  //Steven 20160920 : IC置偏檢查
//const int SHT_RIGHT_IN_BARCODE  =2400;
//const int SHT_RIGHT_OUT_BARCODE =2500;
//const int SHT_RIGHT_IN_HTTP_ERR =2600;
//const int SHT_RIGHT_LASER       =3000;

int iShuttleSpeed=10;                                                           //Steven 20110607 : 殘料偵測Retry時的速度
extern bool bPickShuttleError;
extern bool bCarryControlOutarm1;                                               //jou 2011-11-21 bCarryControlOutarm -> bCarryControlOutarm1 不應該共用一個flag
extern bool bCarryControlOutarm2;
extern int iShakeShuttleTask;                                                   //kevin 20190107 add
extern void InitInArmTask();                                                    //KaiChen 20200304 ：矽格-湖口，要求OutShuttle Loss IC 時機台上的IC放到R道
extern int CheckOneCycleAction(int iTask);                                      //Steven 20240326 : 判斷one cycle的時候要不要繼續放料
//------------------------------------------------------------------------------
bool bExit9080Shuttle=false;
//------------------------------------------------------------------------------
bool IsFLCarrKitAllHasIC()                                                      //ChungHung 20110130 Add
{
    bool bHasEmpty=false;
    bool bfinish=true;

    for(int i=0; i<FLCarryKit.iShtRow; i++)
    {
        for(int j=0; j<FLCarryKit.iShtCol; j++)
        {
            if(FLCarryKit.Item[i][j]==NULL_IC)
            {
                bHasEmpty=true;
            }
        }
    }

    if(bHasEmpty==true)
        bfinish=false;

    return bfinish;
}
//------------------------------------------------------------------------------
bool IsBLCarrKitAllHasIC()
{
    bool bHasEmpty=false;
    bool bfinish=true;

    for(int i=0; i<BLCarryKit.iShtRow; i++)
    {
        for(int j=0; j<BLCarryKit.iShtCol; j++)
        {
            if(BLCarryKit.Item[i][j]==NULL_IC)
            {
                bHasEmpty=true;
            }
        }
    }

    if(bHasEmpty==true)
        bfinish=false;

    return bfinish;
}
//==============================================================================
// check suttle ic position
// return 0 : OK
// return 1 : NG
// return 2 : wait outarm move
//==============================================================================
//------------------------------------------------------------------------------
bool CheckCFixTrayFullPlace()
{
    #ifndef SOFT_SIMULTE
    if(FIX3_FULL_PLACE==Fix3K_UseCylinder)                                      //Steven 20220420 : 針對Fix3氣缸加上保護機制
    {
        if(bShuttleMoveToLeftforFix3)
        {
            return true;
        }
        else if(bShuttleMoveToLeftforFix3==false &&
                Cylinder[C_FixTray_FullPlace].GetOutBit()==false)               //JerryYang 20230519 : 修正fix汽缸 hang up
        {
            if(MOT[MInShuttle1].fCanMoveR==true ||
               MOT[MInShuttle2].fCanMoveR==true)
            {
                if(Cylinder[C_FixTray_FullPlace].OnSensor())
                {
                    ShowMyMessage("C_FixTray_FullPlace OnSensor error, must be off");
                    return true;
                }

                if(Cylinder[C_FixTray_FullPlace].OffSensor()==false)
                {
                    ShowMyMessage("C_FixTray_FullPlace OffSensor error, must be on");
                    return true;
                }
            }
        }
        else if(Cylinder[C_FixTray_FullPlace].GetOutBit()==true)                //JerryYang 20230519 : 修正手動在 IO 頁面把fix tray打出來之後會hang up
        {
            ShowMyMessage("Please make the cylinder [C_FixTray_FullPlace] to off.");
            return true;
        }

        if(Cylinder[C_FixTray_FullPlace].OnSensor() ||
           Cylinder[C_FixTray_FullPlace].OffSensor()==false)                    //Steven 20240916 : Fix3滿盤保護
        {
//            if(CheckOutArmZ(false)==false)                                    //Steven 20220517 : Mark
//            {
//                Cylinder[C_FixTray_FullPlace].Off();
//            }
            return true;
        }
    }
    #endif
    return false;
}
//------------------------------------------------------------------------------
int CheckICUnderShuttle1_9045(bool bShowAlarm)                                  //Steven 20110512 : 整合Alarm Code
{
    #ifdef SOFT_SIMULTE
        return 0;
    #else
    {
        if((MachineTypeChoice==Type_HT9046 || MachineTypeChoice==Type_HT9050))                                      //Steven 20110701 : 9046沒有對照Sensor //AI(W906-HT9050-FAM) 20260925: +Type_HT9050（RULINGS_20260925 第 39 條：「機台類型只靠machine type分類，其他都沿用」）
            return 0;

        if(ENABLE_OUT_SHUTTLE_SENEOR==false)                                    //Steven 20120531 : by pass out shuttle sensor
            return 0;

        if(ENABLE_OUT_SHUTTLEY_LATCH==true &&
           IniConfig.iF07OutShuttleSensorMode==2)                               //Steven 20180329 (Jou) : 使用Y-Latch偵測置偏
            return 0;

        AnsiString str="";
        int ret=0;
        //ChungHung 20130507 add HT9045 updata for 12site 517
        static bool bDuplicateErr[6]={false, false, false, false, false, false};//Steven 20091218 : Avoid duplicate message
        bool bSensorError[6]={false}, bSensorStatus[6]={false}, bHasErr=false, bHasDuplicateErr=false;

        AnsiString ErrPart="";
        if(IniConfig.bF03OutputShuttleSkipICMiss ||
           (LastSet.iRealDummy!=REALLY &&
            fMain->cbTestOutShuttleSensor->Checked==false))                     //Steven 20150727 : 增加Out Shuttle Sensor
            return 0;

        //ChungHung 20130507 add HT9045 updata for 12site 517
        int SensorIndex[6]={SnOutPutSHT1S2, SnOutPutSHT1S3, SnOutPutSHT1S4, SnOutPutSHT1S5, SnOutPutSHT1S6, SnOutPutSHT1S7};
        //int SensorIndex[4]={SnOutPutSHT1S2, SnOutPutSHT1S4, SnOutPutSHT1S5, SnOutPutSHT1S7};

//        int iCheckSensorNo=0;                                                 //Steven 20170209 (wei): iCheckSensorNo 改用 SThreadPara.iCHKStep 取代
        if(SThreadPara.iCHKStep==2)                                             //如果只使用兩顆,要重新給定sensor
        {                                                                       //jou 980724 start : shuttle sensor use 7 pcs
            if(TestIF.dSiteXPitch<=3450)                                        //Frank 20170803 (wei) 2666==>3450
            {
                if(Sen[SnOutPutSHT1S4].Enable && Sen[SnOutPutSHT1S5].Enable)
                {
                    SensorIndex[0]=SnOutPutSHT1S4;
                    SensorIndex[1]=SnOutPutSHT1S5;
                }
            }
            else
            {
                if(Sen[SnOutPutSHT1S3].Enable && Sen[SnOutPutSHT1S6].Enable)
                {
                    SensorIndex[0]=SnOutPutSHT1S3;
                    SensorIndex[1]=SnOutPutSHT1S6;
                }
            }
        }
        else if(SThreadPara.iCHKStep==1)                                        //SingleSite
        {
            SensorIndex[0]=SnOutPutSHT1S3;
        }
        else if(SThreadPara.iCHKStep==3)                                        //ChungHung 20140115 add for 2x3_6
        {
            if(Sen[SnOutPutSHT1S2].Enable &&
               Sen[SnOutPutSHT1S4].Enable &&
               Sen[SnOutPutSHT1S5].Enable)
            {
                SensorIndex[0]=SnOutPutSHT1S2;
                SensorIndex[1]=SnOutPutSHT1S4;
                SensorIndex[2]=SnOutPutSHT1S5;
            }
        }
        else if(SThreadPara.iCHKStep==6)                                        //ChungHung 20130507 add HT9045 updata for 12site 517
        {
            if(Sen[SnOutPutSHT1S2].Enable && Sen[SnOutPutSHT1S3].Enable && Sen[SnOutPutSHT1S4].Enable &&
               Sen[SnOutPutSHT1S5].Enable && Sen[SnOutPutSHT1S6].Enable && Sen[SnOutPutSHT1S7].Enable   )
            {
                SensorIndex[0]=SnOutPutSHT1S2;
                SensorIndex[1]=SnOutPutSHT1S3;
                SensorIndex[2]=SnOutPutSHT1S4;
                SensorIndex[3]=SnOutPutSHT1S5;
                SensorIndex[4]=SnOutPutSHT1S6;
                SensorIndex[5]=SnOutPutSHT1S7;
            }
        }
        else if(SThreadPara.iCHKStep==4)                                        //ChungHung 20130507 add HT9045 updata for 12site 517
        {
            if(Sen[SnOutPutSHT1S2].Enable && Sen[SnOutPutSHT1S4].Enable &&
               Sen[SnOutPutSHT1S5].Enable && Sen[SnOutPutSHT1S7].Enable)
            {
                SensorIndex[0]=SnOutPutSHT1S2;
                SensorIndex[1]=SnOutPutSHT1S4;
                SensorIndex[2]=SnOutPutSHT1S5;
                SensorIndex[3]=SnOutPutSHT1S7;
            }
        }

        for(int i=0; i<SThreadPara.iCHKStep; i++)
        {
            if(IniConfig.iF07OutShuttleSensorMode==0)
                bSensorStatus[i]=Sen[SensorIndex[i]].IsOff();
            else
                bSensorStatus[i]=Sen[SensorIndex[i]].IsOn();

            if(bDuplicateErr[i])
                bHasDuplicateErr=true;

            if(FRCarryKit.iShtRow==2)                                           //Steven 20170209 (wei): 修正Out Shuttle前後對照Sensor偵測飛料Skip
            {
                if((FRCarryKit.Item[0][i]!=NULL_IC && FRCarryKit.Item[0][i]!=HAS_NULL_IC ||
                    FRCarryKit.Item[1][i]!=NULL_IC && FRCarryKit.Item[1][i]!=HAS_NULL_IC) &&
                    bSensorStatus[i])
                {
                    bSensorError[i]=true;
                    bHasErr=true;
                    ErrPart+=ShuttleSensorName[SensorIndex[i]-SnOutPutSHT1S1];  //JerryYang 20201111 修正error part顯示錯誤
                }
                else
                {
                    bDuplicateErr[i]=false;
                }
            }
            else
            {
                if(FRCarryKit.Item[0][i]!=NULL_IC &&
                   FRCarryKit.Item[0][i]!=HAS_NULL_IC &&
                   bSensorStatus[i])
                {
                    bSensorError[i]=true;
                    bHasErr=true;
                    ErrPart+=ShuttleSensorName[SensorIndex[i]-SnOutPutSHT1S1];  //ChungHung 20130502 alter 錯誤 (Shuttle1 and Shuttle2 不一致 改成跟Shuttle2一致)
                }
                else
                {
                    bDuplicateErr[i]=false;
                }
            }
        }

        if(bHasErr)
        {
            if(bShowAlarm)
            {
                bCarryControlOutarm1=true;                                      //jou 2011-11-21 bCarryControlOutarm -> bCarryControlOutarm1 不應該共用一個flag

//jou 2012-02-24 統一改arm可推動方式，out arm讓位造成很多地方hang up
//                if(!MoveOutArmXY_ToShuttleAlarmArea()) return 2;

                str=(IniConfig.iF07OutShuttleSensorMode==0)?"JAM0508":"JAM0501";//Device lose & Device floating

                //KaiChen 20181211 ：Out shuttle Jam Select Skip or Retry
                //==>
                if(IniConfig.iF26OutShuttleJamSelectSkipOrRetry==0)
                    ret=ShowErrorMessage(str, K_RETRY|K_SKIP, MOutShuttle1, bHasDuplicateErr, ErrPart);
                else if(IniConfig.iF26OutShuttleJamSelectSkipOrRetry==1)
                    ret=ShowErrorMessage(str, K_SKIP, MOutShuttle1, bHasDuplicateErr, ErrPart);
                else if(IniConfig.iF26OutShuttleJamSelectSkipOrRetry==2)
                    ret=ShowErrorMessage(str, K_RETRY, MOutShuttle1, bHasDuplicateErr, ErrPart);
                else
                    ret=ShowErrorMessage(str, K_RETRY|K_SKIP, MOutShuttle1, bHasDuplicateErr, ErrPart);
                //<==
                //KaiChen 20181211 ：Out shuttle Jam Select Skip or Retry

                for(int i=0; i<SThreadPara.iCHKStep; i++)
                {
                    if(bSensorError[i])
                        bDuplicateErr[i]=true;
                }

                if(ret==K_SKIP)
                {
                    for(int i=0; i<SThreadPara.iCHKStep; i++)
                    {
                        if(bSensorError[i])
                        {
                            bDuplicateErr[i]=false;
                            if(IniConfig.bF27OutShtLoseICNeedToPick==false)     //Steven 20220120 : 吳如春要求out shuttle lose IC時, out arm還是要下去吸料
                            {
                                FRCarryKit.SetItemData(0, i, NULL_IC);
                                if(CosFunction.bUseSCKART)                      //Steven 20180212 (wei) : For SCK ART
                                    fSCKART->AddOutputJamCnt(0, i, ret);        //RogerYang 20250923 : 整合ART OutArm JamCount
                                if(FRCarryKit.iShtRow==2)                       //Steven 20170209 (wei): 修正Out Shuttle前後對照Sensor偵測飛料Skip
                                {
                                    FRCarryKit.SetItemData(1, i, NULL_IC);
                                    if(CosFunction.bUseSCKART)                  //Steven 20180212 (wei) : For SCK ART
                                        fSCKART->AddOutputJamCnt(1, i, ret);    //RogerYang 20250923 : 整合ART OutArm JamCount
                                }
                            }
                        }
                    }

                    if(CUSTOMER_CODE==CC_SCS)                                   //jou 2015-08-21 After conditional error, all units will go to Error bin ,follow by Index check or piggy back check
                    {
                        if(IniConfig.bF19OutShuttleLoseICNeedPiggyback==true)
                        {
                            fMain->ResetRecordforPiggyBack("RESET_ForShuttleLoseIC");
                        }
                    }
                }
                bCarryControlOutarm1=false;                                     //jou 2011-11-21 bCarryControlOutarm -> bCarryControlOutarm1 不應該共用一個flag
            }
            return 1;
        }
        return 0;
    }
    #endif
}
//------------------------------------------------------------------------------
int CheckICUnderShuttle2_9045(bool bShowAlarm)
{
    #ifdef SOFT_SIMULTE
        return 0;
    #else
    {
        if((MachineTypeChoice==Type_HT9046 || MachineTypeChoice==Type_HT9050))                                      //Steven 20110701 //AI(W906-HT9050-FAM) 20260925: +Type_HT9050（RULINGS_20260925 第 39 條：「機台類型只靠machine type分類，其他都沿用」）
            return 0;

        if(ENABLE_OUT_SHUTTLE_SENEOR==false)                                    //Steven 20120531 : by pass out shuttle sensor
            return 0;

        if(ENABLE_OUT_SHUTTLEY_LATCH==true &&
           IniConfig.iF07OutShuttleSensorMode==2)                               //Steven 20180329 (Jou) : 使用Y-Latch偵測置偏
            return 0;

        AnsiString str="";
        int ret=0;
        int i;
        //ChungHung 20130507 add HT9045 updata for 12site 517
        static bool bDuplicateErr[6]={false, false, false, false, false, false};//Steven 20091218 : Avoid duplicate message
        bool bSensorError[6]={false}, bSensorStatus[6]={false}, bHasErr=false, bHasDuplicateErr=false;

        AnsiString ErrPart="";
        if(IniConfig.bF03OutputShuttleSkipICMiss ||
           (LastSet.iRealDummy!=REALLY &&
            fMain->cbTestOutShuttleSensor->Checked==false))                     //Steven 20150727 : 增加Out Shuttle Sensor
            return 0;

        //ChungHung 20130507 add HT9045 updata for 12site 517
        int SensorIndex[6]={SnOutPutSHT2S2, SnOutPutSHT2S3, SnOutPutSHT2S4, SnOutPutSHT2S5, SnOutPutSHT2S6, SnOutPutSHT2S7};

//        int iCheckSensorNo=0;                                                 //Steven 20170209 (wei): iCheckSensorNo 改用 SThreadPara.iCHKStep 取代
        if(SThreadPara.iCHKStep==2)                                             //如果只使用兩顆,要重新給定sensor
        {
            //jou 980724 start : shuttle sensor use 7 pcs
            if(TestIF.dSiteXPitch<=3450)                                        //Frank 20170803 (wei) 2666==>3450
            {
                if(Sen[SnOutPutSHT2S4].Enable && Sen[SnOutPutSHT2S5].Enable)
                {
                    SensorIndex[0]=SnOutPutSHT2S4;
                    SensorIndex[1]=SnOutPutSHT2S5;
                }
            }
            else
            {
                if(Sen[SnOutPutSHT2S3].Enable && Sen[SnOutPutSHT2S6].Enable)
                {
                    SensorIndex[0]=SnOutPutSHT2S3;
                    SensorIndex[1]=SnOutPutSHT2S6;
                }
            }
            //jou 980724 end
        }
        else if(SThreadPara.iCHKStep==1)                                        //SingleSite
        {
            SensorIndex[0]=SnOutPutSHT2S3;                                      //Dell 20140707 modify SnOutPutSHT1S3 --->SnOutPutSHT2S3
        }
        else if(SThreadPara.iCHKStep==3)                                        //ChungHung 20140115 add for 2x3_6
        {
            if(Sen[SnOutPutSHT2S2].Enable && Sen[SnOutPutSHT2S4].Enable &&
               Sen[SnOutPutSHT2S4].Enable                                 )
            {
                SensorIndex[0]=SnOutPutSHT2S2;
                SensorIndex[1]=SnOutPutSHT2S4;
                SensorIndex[2]=SnOutPutSHT2S5;
            }
        }
        else if(SThreadPara.iCHKStep==6)
        {
            if(Sen[SnOutPutSHT2S2].Enable && Sen[SnOutPutSHT2S3].Enable && Sen[SnOutPutSHT2S4].Enable &&
               Sen[SnOutPutSHT2S5].Enable && Sen[SnOutPutSHT2S6].Enable && Sen[SnOutPutSHT2S7].Enable   )
            {
                SensorIndex[0]=SnOutPutSHT2S2;
                SensorIndex[1]=SnOutPutSHT2S3;
                SensorIndex[2]=SnOutPutSHT2S4;
                SensorIndex[3]=SnOutPutSHT2S5;
                SensorIndex[4]=SnOutPutSHT2S6;
                SensorIndex[5]=SnOutPutSHT2S7;
            }
        }
        else if(SThreadPara.iCHKStep==4)
        {
            if(Sen[SnOutPutSHT2S2].Enable && Sen[SnOutPutSHT2S4].Enable &&
               Sen[SnOutPutSHT2S4].Enable && Sen[SnOutPutSHT2S7].Enable   )
            {
                SensorIndex[0]=SnOutPutSHT2S2;
                SensorIndex[1]=SnOutPutSHT2S4;
                SensorIndex[2]=SnOutPutSHT2S5;
                SensorIndex[3]=SnOutPutSHT2S7;
            }
        }

        for(i=0; i<SThreadPara.iCHKStep; i++)
        {
            //jou 980828 start : add Out Shuttle Sensor detect mode
            //0 = Detect Have IC
            //1 = Detect Double Device
            if(IniConfig.iF07OutShuttleSensorMode==0)
                bSensorStatus[i]=Sen[SensorIndex[i]].IsOff();
            else
                bSensorStatus[i]=Sen[SensorIndex[i]].IsOn();

            if(bDuplicateErr[i])
                bHasDuplicateErr=true;

            if(BRCarryKit.iShtRow==2)                                           //Steven 20170209 (wei): 修正Out Shuttle前後對照Sensor偵測飛料Skip
            {
                if((BRCarryKit.Item[0][i]!=NULL_IC && BRCarryKit.Item[0][i]!=HAS_NULL_IC ||
                    BRCarryKit.Item[1][i]!=NULL_IC && BRCarryKit.Item[1][i]!=HAS_NULL_IC) &&
                    bSensorStatus[i])
                {
                    bSensorError[i]=true;
                    bHasErr=true;
                    ErrPart+=ShuttleSensorName[SensorIndex[i]-SnOutPutSHT2S1];
                }
                else
                {
                    bDuplicateErr[i]=false;
                }
            }
            else
            {
                if(BRCarryKit.Item[0][i]!=NULL_IC && BRCarryKit.Item[0][i]!=HAS_NULL_IC && bSensorStatus[i])
                {
                    bSensorError[i]=true;
                    bHasErr=true;
                    ErrPart+=ShuttleSensorName[SensorIndex[i]-SnOutPutSHT2S1];
                }
                else
                {
                    bDuplicateErr[i]=false;
                }
            }
        }

        if(bHasErr)
        {
            if(bShowAlarm)
            {
                bCarryControlOutarm2=true;                                      //jou 2011-11-21 bCarryControlOutarm -> bCarryControlOutarm2 不應該共用一個flag
                str=(IniConfig.iF07OutShuttleSensorMode==0)?"JAM0509":"JAM0504";//Device lose & Device floating

                if(IniConfig.iF26OutShuttleJamSelectSkipOrRetry==0)             //KaiChen 20181211 ：Out shuttle Jam Select Skip or Retry
                    ret=ShowErrorMessage(str, K_RETRY|K_SKIP, MOutShuttle2, bHasDuplicateErr, ErrPart);
                else if(IniConfig.iF26OutShuttleJamSelectSkipOrRetry==1)
                    ret=ShowErrorMessage(str, K_SKIP, MOutShuttle2, bHasDuplicateErr, ErrPart);
                else if(IniConfig.iF26OutShuttleJamSelectSkipOrRetry==2)
                    ret=ShowErrorMessage(str, K_RETRY, MOutShuttle2, bHasDuplicateErr, ErrPart);
                else
                    ret=ShowErrorMessage(str, K_RETRY|K_SKIP, MOutShuttle2, bHasDuplicateErr, ErrPart);

                for(i=0; i<SThreadPara.iCHKStep; i++)
                {
                    if(bSensorError[i])
                        bDuplicateErr[i]=true;
                }

                if(ret==K_SKIP)
                {
                    for(i=0; i<SThreadPara.iCHKStep; i++)
                    {
                        if(bSensorError[i])
                        {
                            bDuplicateErr[i]=false;
                            if(IniConfig.bF27OutShtLoseICNeedToPick==false)     //Steven 20220120 : 吳如春要求out shuttle lose IC時, out arm還是要下去吸料
                            {
                                BRCarryKit.SetItemData(0, i, NULL_IC);
                                if(CosFunction.bUseSCKART)                      //Steven 20180212 (wei) : For SCK ART
                                    fSCKART->AddOutputJamCnt(0, i, ret);        //RogerYang 20250923 : 整合ART OutArm JamCount
                                if(BRCarryKit.iShtRow==2)                       //Steven 20170209 (wei): 修正Out Shuttle前後對照Sensor偵測飛料Skip
                                {
                                    BRCarryKit.SetItemData(1, i, NULL_IC);
                                    if(CosFunction.bUseSCKART)                  //Steven 20180212 (wei) : For SCK ART
                                        fSCKART->AddOutputJamCnt(1, i, ret);    //RogerYang 20250923 : 整合ART OutArm JamCount
                                }
                            }
                        }
                    }
                }
                bCarryControlOutarm2=false;                                     //jou 2011-11-21 bCarryControlOutarm -> bCarryControlOutarm2 不應該共用一個flag
            }
            return 1;
        }
        return 0;
    }
    #endif
}
//------------------------------------------------------------------------------
//jou 981125 start : 防止Index內疊烏龜
// true  : PASS
// false : FAIL
//------------------------------------------------------------------------------
bool CheckNullICShuttle1_9045(bool bShowAlarm)
{
    #ifdef SOFT_SIMULTE
        return true;
    #else
    {
        if((MachineTypeChoice==Type_HT9046 || MachineTypeChoice==Type_HT9050))                                      //Steven 20110701 //AI(W906-HT9050-FAM) 20260925: +Type_HT9050（RULINGS_20260925 第 39 條：「機台類型只靠machine type分類，其他都沿用」）
            return true;                                                        //Steven 20121116

        if(ENABLE_OUT_SHUTTLE_SENEOR==false)                                    //Steven 20120531 : by pass out shuttle sensor
            return true;                                                        //Steven 20121116

        AnsiString str="";
        int i;
        //ChungHung 20130507 add HT9045 updata for 12site 517
        static bool bDuplicateErr[6]={false, false, false, false,false,false};  //Steven 20091218 : Avoid duplicate message
        bool bSensorError[6]={false}, bSensorStatus[6]={false}, bHasErr=false, bHasDuplicateErr=false;

        AnsiString ErrPart="";
        if(LastSet.iRealDummy!=REALLY &&
           fMain->cbTestOutShuttleSensor->Checked==false)                       //Steven 20150727 : 增加Out Shuttle Sensor
            return true;

        //ChungHung 20130507 add HT9045 updata for 12site 517
        int SensorIndex[6]={SnOutPutSHT1S2, SnOutPutSHT1S3 ,SnOutPutSHT1S4, SnOutPutSHT1S5, SnOutPutSHT1S6, SnOutPutSHT1S7};

//        int iCheckSensorNo=0;                                                 //Steven 20170209 (wei): iCheckSensorNo 改用 SThreadPara.iCHKStep 取代
        if(SThreadPara.iCHKStep==2)                                             //如果只使用兩顆,要重新給定sensor
        {
            //jou 980724 start : shuttle sensor use 7 pcs
            if(TestIF.dSiteXPitch<=3450)                                        //Frank 20170803 (wei) 2666==>3450
            {
                if(Sen[SnOutPutSHT1S4].Enable && Sen[SnOutPutSHT1S5].Enable)
                {
                    SensorIndex[0]=SnOutPutSHT1S4;
                    SensorIndex[1]=SnOutPutSHT1S5;
                }
            }
            else
            {
                if(Sen[SnOutPutSHT1S3].Enable && Sen[SnOutPutSHT1S6].Enable)
                {
                    SensorIndex[0]=SnOutPutSHT1S3;
                    SensorIndex[1]=SnOutPutSHT1S6;
                }
            }
            //jou 980724 end

            if(Sen[SensorIndex[0]].Enable==false && Sen[SensorIndex[1]].Enable==false)
            {
                return true;
            }
        }
        else if(SThreadPara.iCHKStep==1)                                        //SingleSite
        {
            SensorIndex[0]=SnOutPutSHT1S3;
        }
        else if(SThreadPara.iCHKStep==3)                                        //ChungHung 20140115 add for 2x3_6
        {
            if(Sen[SensorIndex[0]].Enable && Sen[SensorIndex[1]].Enable &&
               Sen[SensorIndex[2]].Enable && Sen[SensorIndex[3]].Enable)
            {
                SensorIndex[0]=SnOutPutSHT1S2;
                SensorIndex[1]=SnOutPutSHT1S4;
                SensorIndex[2]=SnOutPutSHT1S5;
            }

            if(Sen[SensorIndex[0]].Enable==false &&
               Sen[SensorIndex[1]].Enable==false &&
               Sen[SensorIndex[2]].Enable==false)
            {
                return true;
            }
        }
        else if(SThreadPara.iCHKStep==6)                                        //ChungHung 20130507 add HT9045 updata for 12site 517
        {
            if(Sen[SnOutPutSHT1S2].Enable && Sen[SnOutPutSHT1S3].Enable && Sen[SnOutPutSHT1S4].Enable &&
               Sen[SnOutPutSHT1S5].Enable && Sen[SnOutPutSHT1S6].Enable && Sen[SnOutPutSHT1S7].Enable   )
            {
                SensorIndex[0]=SnOutPutSHT1S2;
                SensorIndex[1]=SnOutPutSHT1S3;
                SensorIndex[2]=SnOutPutSHT1S4;
                SensorIndex[3]=SnOutPutSHT1S5;
                SensorIndex[4]=SnOutPutSHT1S6;
                SensorIndex[5]=SnOutPutSHT1S7;
            }

            if(Sen[SnOutPutSHT1S2].Enable==false && Sen[SnOutPutSHT1S3].Enable==false && Sen[SnOutPutSHT1S4].Enable==false &&
               Sen[SnOutPutSHT1S5].Enable==false && Sen[SnOutPutSHT1S6].Enable==false && Sen[SnOutPutSHT1S7].Enable==false   )
            {
                return true;
            }
        }
        else
        {
            //ChungHung 20130507 add HT9045 updata for 12site 517
            if(Sen[SensorIndex[0]].Enable && Sen[SensorIndex[1]].Enable &&
               Sen[SensorIndex[2]].Enable && Sen[SensorIndex[3]].Enable)
            {
                SensorIndex[0]=SnOutPutSHT1S2;
                SensorIndex[1]=SnOutPutSHT1S4;
                SensorIndex[2]=SnOutPutSHT1S5;
                SensorIndex[3]=SnOutPutSHT1S7;                                  //ChungHung 20140115 modify 4->3
            }

            if(Sen[SensorIndex[0]].Enable==false && Sen[SensorIndex[1]].Enable==false &&
               Sen[SensorIndex[2]].Enable==false && Sen[SensorIndex[3]].Enable==false)
            {
                return true;
            }
        }

        for(i=0; i<SThreadPara.iCHKStep; i++)
        {
            bSensorStatus[i]=Sen[SensorIndex[i]].IsOn();

            if(bDuplicateErr[i])
                bHasDuplicateErr=true;

            if(bSensorStatus[i])
            {
                bSensorError[i]=true;
                bHasErr=true;
                ErrPart+=ShuttleSensorName[SensorIndex[i]-SnOutPutSHT1S1];
            }
            else
            {
                bDuplicateErr[i]=false;
            }
        }

        if(bHasErr)
        {
            if(bShowAlarm)
            {
                ShowErrorMessage("JAM0560", K_RETRY, MOutShuttle1, bHasDuplicateErr, ErrPart);   //Steven 20210302 : JAM0501 --> JAM0560
                bDuplicateErr[i]=true;

                for(i=0; i<SThreadPara.iCHKStep; i++)
                {
                    if(bSensorError[i])
                        bDuplicateErr[i]=true;
                }
            }
            return false;
        }
        else
        {
            for(i=0; i<SThreadPara.iCHKStep; i++)
            {
                if(bSensorError[i])
                    bDuplicateErr[i]=false;
            }
            return true;
        }
    }
    #endif
}
//------------------------------------------------------------------------------
bool CheckNullICShuttle2_9045(bool bShowAlarm)
{
    #ifdef SOFT_SIMULTE
        return true;
    #else
    {
        if((MachineTypeChoice==Type_HT9046 || MachineTypeChoice==Type_HT9050))                                      //Steven 20110701 //AI(W906-HT9050-FAM) 20260925: +Type_HT9050（RULINGS_20260925 第 39 條：「機台類型只靠machine type分類，其他都沿用」）
            return true;                                                        //Steven 20121116

        if(ENABLE_OUT_SHUTTLE_SENEOR==false)                                    //Steven 20120531 : by pass out shuttle sensor
            return true;                                                        //Steven 20121116

        AnsiString str="";
        int i;
        //ChungHung 20130507 add HT9045 updata for 12site 517
        static bool bDuplicateErr[6]={false, false, false, false, false, false};//Steven 20091218 : Avoid duplicate message
        bool bSensorError[6]={false}, bSensorStatus[6]={false}, bHasErr=false, bHasDuplicateErr=false;
        AnsiString ErrPart="";
        if(LastSet.iRealDummy!=REALLY &&
           fMain->cbTestOutShuttleSensor->Checked==false)                       //Steven 20150727 : 增加Out Shuttle Sensor
            return true;

        //ChungHung 20130507 add HT9045 updata for 12site 517
        int SensorIndex[6]={SnOutPutSHT2S2, SnOutPutSHT2S3, SnOutPutSHT2S4, SnOutPutSHT2S5, SnOutPutSHT2S6, SnOutPutSHT2S7};

//        int iCheckSensorNo=0;                                                 //Steven 20170209 (wei): iCheckSensorNo 改用 SThreadPara.iCHKStep 取代
        if(SThreadPara.iCHKStep==2)                                             //如果只使用兩顆,要重新給定sensor
        {
            //jou 980724 start : shuttle sensor use 7 pcs
            if(TestIF.dSiteXPitch<=3450)                                        //Frank 20170803 (wei) 2666==>3450
            {
                if(Sen[SnOutPutSHT2S4].Enable && Sen[SnOutPutSHT2S5].Enable)
                {
                    SensorIndex[0]=SnOutPutSHT2S4;
                    SensorIndex[1]=SnOutPutSHT2S5;
                }
            }
            else
            {
                if(Sen[SnOutPutSHT2S3].Enable && Sen[SnOutPutSHT2S6].Enable)
                {
                    SensorIndex[0]=SnOutPutSHT2S3;
                    SensorIndex[1]=SnOutPutSHT2S6;
                }
            }
            //jou 980724 end

            if(Sen[SensorIndex[0]].Enable==false && Sen[SensorIndex[1]].Enable==false)
            {
                return true;
            }
        }
        else if(SThreadPara.iCHKStep==1)                                        //SingleSite
        {
            SensorIndex[0]=SnOutPutSHT1S3;
        }
        else if(SThreadPara.iCHKStep==6)                                        //ChungHung 20130507 add HT9045 updata for 12site 517
        {
            if(Sen[SnOutPutSHT2S2].Enable && Sen[SnOutPutSHT2S3].Enable && Sen[SnOutPutSHT2S4].Enable &&
               Sen[SnOutPutSHT2S5].Enable && Sen[SnOutPutSHT2S6].Enable && Sen[SnOutPutSHT2S7].Enable   )
            {
                SensorIndex[0]=SnOutPutSHT2S2;
                SensorIndex[1]=SnOutPutSHT2S3;
                SensorIndex[2]=SnOutPutSHT2S4;
                SensorIndex[3]=SnOutPutSHT2S5;
                SensorIndex[4]=SnOutPutSHT2S6;
                SensorIndex[5]=SnOutPutSHT2S7;
            }

            if(Sen[SnOutPutSHT2S2].Enable==false && Sen[SnOutPutSHT2S3].Enable==false && Sen[SnOutPutSHT2S4].Enable==false &&
               Sen[SnOutPutSHT2S5].Enable==false && Sen[SnOutPutSHT2S6].Enable==false && Sen[SnOutPutSHT2S7].Enable==false   )
            {
                return true;
            }
        }
        else
        {
            //ChungHung 20130507 add HT9045 updata for 12site 517
            if(Sen[SensorIndex[0]].Enable && Sen[SensorIndex[1]].Enable &&
               Sen[SensorIndex[2]].Enable && Sen[SensorIndex[3]].Enable   )
            {
                SensorIndex[0]=SnOutPutSHT2S2;
                SensorIndex[1]=SnOutPutSHT2S4;
                SensorIndex[2]=SnOutPutSHT2S5;
                SensorIndex[3]=SnOutPutSHT2S7;
            }

            if(Sen[SensorIndex[0]].Enable==false && Sen[SensorIndex[1]].Enable==false &&
               Sen[SensorIndex[2]].Enable==false && Sen[SensorIndex[3]].Enable==false)
            {
                return true;
            }
        }

        for(i=0; i<SThreadPara.iCHKStep; i++)
        {
            bSensorStatus[i]=Sen[SensorIndex[i]].IsOn();

            if(bDuplicateErr[i])
                bHasDuplicateErr=true;

            if(bSensorStatus[i])
            {
                bSensorError[i]=true;
                bHasErr=true;
                ErrPart+=ShuttleSensorName[SensorIndex[i]-SnOutPutSHT2S1];
            }
            else
            {
                bDuplicateErr[i]=false;
            }
        }

        if(bHasErr)
        {
            if(bShowAlarm)
            {
                ShowErrorMessage("JAM0570", K_RETRY, MOutShuttle2, bHasDuplicateErr, ErrPart);     //Steven 20210302 : JAM0504 --> JAM0570
                bDuplicateErr[i]=true;

                for(i=0; i<SThreadPara.iCHKStep; i++)
                {
                    if(bSensorError[i])
                        bDuplicateErr[i]=true;
                }
            }
            return false;
        }
        else
        {
            for(i=0; i<SThreadPara.iCHKStep; i++)
            {
                if(bSensorError[i])
                    bDuplicateErr[i]=false;
            }
            return true;
        }
    }
    #endif
}
//jou 981125 end
//------------------------------------------------------------------------------
//bCheckHasIC==true :  檢查殘料
//bCheckHasIC==false : 檢查飛料
//------------------------------------------------------------------------------
bool CheckShuttlePos1_M204(bool bCheckHasIC, bool bAlarm)                       //JerryYang 20200610 Out shuttle lose IC需先讓位再發出alarm
{
    int iPos;
    bool bHasError=false;
    bSh1HasFloatingError=false;

    if(IniConfig.bSPILFunction==true)                                           //JerryYang 20181228 矽品新竹守志要求Latch要能bypass
    {
        if(FRCarryKit.iShtRow==1 &&                                             //Steven 20240428 : 整合判斷方式
           TestIF.bNS7000kit==false)
        {
            if(Tech.OutSH1ZOneRowDetectPos==0 || Tech.OutSH2ZOneRowDetectPos==0)
                return false;
        }
        else if(TestIF_File.iTestMode==SingleSite)                              //Steven 20160919 : fix for 1x1偏心要跑Y sensor, 因為舊機台沒打洞
        {
            if(Tech.OutSH1ZOneRowDetectPos==0 || Tech.OutSH2ZOneRowDetectPos==0)
                return false;
        }
        else if(TestIF_File.iTestMode==_6Site2X3 ||
                TestIF_File.iTestMode==_10Site2X5)                              //Steven 20240428 : Site Col為奇數的話, 可以跑Y-Latch
        {
            if(Use_AxisY_Sensor_2x3mode==false || Tech.OutSH1ZOneRowDetectPos==0 || Tech.OutSH2ZOneRowDetectPos==0)
                return false;
        }
        else
        {
            if(Tech.OutSH1ZDetectPos==0 || Tech.OutSH2ZDetectPos==0)
                return false;
        }
    }

    for(int i=0; i<8; i++)
    {
        SThreadPara.bSh1HasIC[0][i]=true;
        SThreadPara.bSh1HasIC[1][i]=true;
        SThreadPara.bShICFloating[0][i]=true;
    }

    SThreadPara.ErrPartSh1="";
    SThreadPara.FloatingErrPartSh1="";

    if(SThreadPara.iScanSensor==1 && SThreadPara.bOutYUseLatch)                 //Steven 20140818 : 康泰克Out Sensor Y改用Latch
    {
        for(int j=0; j<FRCarryKit.iShtCol; j++)
        {
            for(int i=0; i<fLtcSensor->LatchDataCnt5; i++)
            {
                if(fLtcSensor->LatchDataTable5[i]<SThreadPara.iCheckPosMaxY[0][j] &&
                   fLtcSensor->LatchDataTable5[i]>SThreadPara.iCheckPosMinY[0][j])
                {
                    if(IsNNMode()==NN_2Row)
                    {
                        SThreadPara.bSh1HasIC[0][j]=false;
                        SThreadPara.bSh1HasIC[1][j]=false;
                    }
                    else if((TestIF_File.iTestMode==_6Site2X3 ||
                             TestIF_File.iTestMode==_10Site2X5) &&              //Steven 20240428 : Site Col為奇數的話, 可以跑Y-Latch
                            Use_AxisY_Sensor_2x3mode==true)                     //Isaac 20180402 (Steven) 2x3 mode 用Y sensor 偵測Outshuttle殘料
                    {
                        SThreadPara.bSh1HasIC[0][j]=false;
                        SThreadPara.bSh1HasIC[1][j]=false;
                    }
                    else
                    {
                        SThreadPara.bSh1HasIC[0][j]=false;
                    }
                    break;
                }
            }
        }
    }
    else
    {
        for(int j=0; j<FRCarryKit.iShtCol; j++)
        {
            for(int i=0; i<fLtcSensor->LatchDataCnt1; i++)
            {
                //先找第1顆,因為偏心只用這一顆
                if(fLtcSensor->LatchDataTable1[i]<SThreadPara.iCheckPosMax[0][j] &&
                   fLtcSensor->LatchDataTable1[i]>SThreadPara.iCheckPosMin[0][j])
                {
                    SThreadPara.bSh1HasIC[0][j]=false;                          //這是A行
                    break;
                }
            }

            //一次兩顆,所以要找第0顆
            if(FRCarryKit.iShtRow==2)
            {
                for(int i=0; i<fLtcSensor->LatchDataCnt0; i++)
                {
                    if(fLtcSensor->LatchDataTable0[i]<SThreadPara.iCheckPosMax[0][j] &&
                       fLtcSensor->LatchDataTable0[i]>SThreadPara.iCheckPosMin[0][j])
                    {
                        SThreadPara.bSh1HasIC[1][j]=false;                      //這是B行
                        break;
                    }
                }
            }
        }
    }

    //檢查錯誤的位置
    if(SThreadPara.iScanSensor==1 && SThreadPara.bOutYUseLatch)                 //Steven 20140818 : 康泰克Out Sensor Y改用Latch
    {
        if(IsNNMode()==NN_2Row)
        {
            for(int j=0; j<FRCarryKit.iShtCol; j++)
            {
                if(bCheckHasIC==true)                                           //檢查殘料
                {
                    if(SThreadPara.bSh1HasIC[0][j]==bCheckHasIC)                //有錯的位置
                    {
                        SThreadPara.ErrPartSh1+=IndexSuckName[2][j];            //Steven 20151006 : For 32Site Out Shuttle Alarm 0/1改成 2/3
                        SThreadPara.ErrPartSh1+=IndexSuckName[3][j];
                        bHasError=true;
                    }
                }
                else                                                            //檢查飛料
                {
                    if(((FRCarryKit.Item[0][j]!=NULL_IC && FRCarryKit.Item[0][j]!=HAS_NULL_IC) ||
                        (FRCarryKit.Item[1][j]!=NULL_IC && FRCarryKit.Item[1][j]!=HAS_NULL_IC)) &&  //JerryYang 20200824 : 修正out shuttle sensor判斷方式
                       SThreadPara.bSh1HasIC[0][j]==false)
                    {
                        SThreadPara.ErrPartSh1+=IndexSuckName[2][j];            //Steven 20151006 : For 32Site Out Shuttle Alarm 0/1改成 2/3
                        SThreadPara.ErrPartSh1+=IndexSuckName[3][j];
                        bHasError=true;
                    }
                }
            }
        }
        else if((TestIF_File.iTestMode==_6Site2X3 ||
                 TestIF_File.iTestMode==_10Site2X5) &&                          //Steven 20240428 : Site Col為奇數的話, 可以跑Y-Latch
                Use_AxisY_Sensor_2x3mode==true)                                 //Isaac 20180402 (Steven) 2x3 mode 用Y sensor 偵測Outshuttle殘料
        {
            for(int i=0; i<FRCarryKit.iShtRow; i++)
            {
                for(int j=0; j<FRCarryKit.iShtCol; j++)
                {
                    if(bCheckHasIC==true)                                       //檢查殘料
                    {
                        if(SThreadPara.bSh1HasIC[0][j]==bCheckHasIC ||
                           SThreadPara.bSh1HasIC[1][j]==bCheckHasIC)            //有錯的位置
                        {
                            SThreadPara.ErrPartSh1+=IndexSuckName[i][j];
                            bHasError=true;
                        }
                    }
                    else                                                        //檢查飛料
                    {
                        if(((FRCarryKit.Item[0][j]!=NULL_IC && FRCarryKit.Item[0][j]!=HAS_NULL_IC) ||
                            (FRCarryKit.Item[1][j]!=NULL_IC && FRCarryKit.Item[1][j]!=HAS_NULL_IC)) &&  //JerryYang 20200824 : 修正out shuttle sensor判斷方式
                           SThreadPara.bSh1HasIC[0][j]==false)
                        {
                            SThreadPara.ErrPartSh1+=IndexSuckName[i][j];
                            bHasError=true;
                        }
                    }
                }
            }
        }
        else
        {
            for(int j=0; j<FRCarryKit.iShtCol; j++)
            {
                if(bCheckHasIC==true)                                           //檢查殘料
                {
                    if(SThreadPara.bSh1HasIC[0][j]==bCheckHasIC)                //有錯的位置
                    {
                        SThreadPara.ErrPartSh1+=IndexSuckName[0][j];
                        bHasError=true;
                    }
                }
                else                                                            //檢查飛料
                {
                    if(FRCarryKit.Item[0][j]!=NULL_IC &&
                       FRCarryKit.Item[0][j]!=HAS_NULL_IC &&
                       SThreadPara.bSh1HasIC[0][j]==false)
                    {
                        SThreadPara.ErrPartSh1+=IndexSuckName[0][j];
                        bHasError=true;
                    }
                }
            }
        }
    }
    else
    {
        iPos=IsNNMode();
        for(int i=0; i<FRCarryKit.iShtRow; i++)
        {
            for(int j=0; j<FRCarryKit.iShtCol; j++)
            {
                if(bCheckHasIC==true)                                           //檢查殘料
                {
                    if(TestIF_File.b16Direct8Shuttle &&                         //Steven 20210322 : 16 Site Direct Heater use 8 Site shuttle
                       TestIF.iTestMode==_16Site2X8 &&
                       iCloseSiteStep_2x8==2)                                   //Steven 20240428 : 整合判斷方式
                    {
                        if(j!=0 && j!=1 && j!=6 && j!=7)
                        {
                            if(SThreadPara.bSh1HasIC[i][j]==bCheckHasIC)
                            {
                                SThreadPara.ErrPartSh1+=IndexSuckName[i+iPos][j];
                                bHasError=true;
                            }
                        }
                    }
                    else if((CUSTOMER_CODE==CC_KYEC_JCTHIU ||
                             CUSTOMER_CODE==CC_KYEC_LEE ||
                             IniConfig.bSPILFunction ||                         //JerryYang 20180209 (Steven) KYEC MTK 12 site的kit 選16 site模式中間12site
                             TestIF_File.b16Direct12Shuttle) &&                 //Steven 20210112 : Add function
                             TestIF.iTestMode==_16Site2X8 &&
                             bTestSiteUse[0][0][0]==false && bTestSiteUse[0][0][7]==false &&
                             bTestSiteUse[0][1][0]==false && bTestSiteUse[0][1][7]==false &&
                             TestIF_File.iSiteMap[0][0]==0 && TestIF_File.iSiteMap[0][7]==0 &&
                             TestIF_File.iSiteMap[1][0]==0 && TestIF_File.iSiteMap[1][7]==0)
                    {
                        if(j!=0 && j!=7)
                        {
                            if(SThreadPara.bSh1HasIC[i][j]==bCheckHasIC)
                            {
                                SThreadPara.ErrPartSh1+=IndexSuckName[i+iPos][j];
                                bHasError=true;
                            }
                        }
                    }
                    else if(TestIF.iTestMode==_16Site2X8 &&
                            iCloseSiteModeFor2x8==e2x8_STMMode)                 //JerryYang 20190729 STM 8 site交錯模式
                    {
                        if((i==0 && j==0) || (i==0 && j==2) || (i==0 && j==4) || (i==0 && j==6) || (i==1 && j==1) || (i==1 && j==3) || (i==1 && j==5) || (i==1 && j==7))
                        {
                        }
                        else
                        {
                            if(SThreadPara.bSh1HasIC[i][j]==bCheckHasIC)
                            {
                                SThreadPara.ErrPartSh1+=IndexSuckName[i+iPos][j];
                                bHasError=true;
                            }
                        }
                    }
                    else if(TestIF.iTestMode==_16Site2X8 &&
                            iCloseSiteModeFor2x8==e2x8_TW153Mode)               //Steven 20220816 : Add for TW153TK spec
                    {
                        if((i==0 && j==1) || (i==0 && j==3) || (i==0 && j==5) || (i==0 && j==7) || (i==1 && j==0) || (i==1 && j==2) || (i==1 && j==4) || (i==1 && j==6))
                        {
                        }
                        else
                        {
                            if(SThreadPara.bSh1HasIC[i][j]==bCheckHasIC)
                            {
                                SThreadPara.ErrPartSh1+=IndexSuckName[i+iPos][j];
                                bHasError=true;
                            }
                        }
                    }
                    else if(CUSTOMER_CODE==CC_AMKOR_Philippines &&
                            TestIF.iTestMode==_32Site4X8N)                      //JerryYang 20210409 : add for ATP
                    {
                        if(Check32siteOnlyEnabled4x4Site()==true)
                        {
                            if(j==2 || j==3 || j==4 || j==5)
                            {
                                if(SThreadPara.bSh1HasIC[i][j]==bCheckHasIC)
                                {
                                    SThreadPara.ErrPartSh1+=IndexSuckName[i+iPos][j];
                                    bHasError=true;
                                }
                            }
                        }
                        else if(Check32siteOnlyEnabled24Site()==true)
                        {
                            if(j==1 || j==2 || j==3 || j==4 || j==5 ||j==6)
                            {
                                if(SThreadPara.bSh1HasIC[i][j]==bCheckHasIC)
                                {
                                    SThreadPara.ErrPartSh1+=IndexSuckName[i+iPos][j];
                                    bHasError=true;
                                }
                            }
                        }
                        else
                        {
                            if(SThreadPara.bSh1HasIC[i][j]==bCheckHasIC)        //有錯的位置
                            {
                                SThreadPara.ErrPartSh1+=IndexSuckName[i+iPos][j];
                                bHasError=true;
                            }
                        }
                    }
                    else
                    {
                        if(SThreadPara.bSh1HasIC[i][j]==bCheckHasIC)            //有錯的位置
                        {
                            SThreadPara.ErrPartSh1+=IndexSuckName[i+iPos][j];
                            bHasError=true;
                        }
                    }
                }
                else                                                            //檢查飛料
                {
                    if(TestIF_File.b16Direct8Shuttle &&                         //Steven 20210322 : 16 Site Direct Heater use 8 Site shuttle
                       TestIF.iTestMode==_16Site2X8 &&
                       iCloseSiteStep_2x8==2)                                   //Steven 20240428 : 整合判斷方式
                    {
                        if(j!=0 && j!=1 && j!=6 && j!=7)
                        {
                            if(FRCarryKit.Item[i][j]!=NULL_IC &&
                               FRCarryKit.Item[i][j]!=HAS_NULL_IC &&
                               SThreadPara.bSh1HasIC[i][j]==false)
                            {
                                SThreadPara.ErrPartSh1+=IndexSuckName[i+iPos][j];
                                bHasError=true;
                            }
                        }
                    }
                    else if((CUSTOMER_CODE==CC_KYEC_JCTHIU ||
                             CUSTOMER_CODE==CC_KYEC_LEE ||
                             IniConfig.bSPILFunction ||                         //JerryYang 20180209 (Steven) KYEC MTK 12 site的kit 選16 site模式中間12site
                             TestIF_File.b16Direct12Shuttle) &&                 //Steven 20210112 : Add function
                             TestIF.iTestMode==_16Site2X8 &&
                             bTestSiteUse[0][0][0]==false && bTestSiteUse[0][0][7]==false &&
                             bTestSiteUse[0][1][0]==false && bTestSiteUse[0][1][7]==false &&
                             TestIF_File.iSiteMap[0][0]==0 && TestIF_File.iSiteMap[0][7]==0 &&
                             TestIF_File.iSiteMap[1][0]==0 && TestIF_File.iSiteMap[1][7]==0)
                    {
                        if(j!=0 && j!=7)
                        {
                            if(FRCarryKit.Item[i][j]!=NULL_IC && FRCarryKit.Item[i][j]!=HAS_NULL_IC &&
                               SThreadPara.bSh1HasIC[i][j]==false)
                            {
                                SThreadPara.ErrPartSh1+=IndexSuckName[i+iPos][j];
                                bHasError=true;
                            }
                        }
                    }
                    else if(TestIF.iTestMode==_16Site2X8 &&
                            iCloseSiteModeFor2x8==e2x8_STMMode)                 //JerryYang 20190729 STM 8 site交錯模式
                    {
                        if((i==0 && j==0) || (i==0 && j==2) || (i==0 && j==4) || (i==0 && j==6) || (i==1 && j==1) || (i==1 && j==3) || (i==1 && j==5) || (i==1 && j==7))
                        {
                        }
                        else
                        {
                            if(FRCarryKit.Item[i][j]!=NULL_IC &&
                               FRCarryKit.Item[i][j]!=HAS_NULL_IC &&
                               SThreadPara.bSh1HasIC[i][j]==false)
                            {
                                SThreadPara.ErrPartSh1+=IndexSuckName[i+iPos][j];
                                bHasError=true;
                            }
                        }
                    }
                    else if(TestIF.iTestMode==_16Site2X8 &&
                            iCloseSiteModeFor2x8==e2x8_TW153Mode)               //Steven 20220816 : Add for TW153TK spec
                    {
                        if((i==0 && j==1) || (i==0 && j==3) || (i==0 && j==5) || (i==0 && j==7) || (i==1 && j==0) || (i==1 && j==2) || (i==1 && j==4) || (i==1 && j==6))
                        {
                        }
                        else
                        {
                            if(FRCarryKit.Item[i][j]!=NULL_IC &&
                               FRCarryKit.Item[i][j]!=HAS_NULL_IC &&
                               SThreadPara.bSh1HasIC[i][j]==false)
                            {
                                SThreadPara.ErrPartSh1+=IndexSuckName[i+iPos][j];
                                bHasError=true;
                            }
                        }
                    }
                    else if(CUSTOMER_CODE==CC_AMKOR_Philippines &&
                            TestIF.iTestMode==_32Site4X8N)                      //JerryYang 20210409 : add for ATP
                    {
                        if(Check32siteOnlyEnabled4x4Site()==true)
                        {
                            if(j==2 || j==3 || j==4 || j==5)
                            {
                                if(FRCarryKit.Item[i][j]!=NULL_IC &&
                                   FRCarryKit.Item[i][j]!=HAS_NULL_IC &&
                                   SThreadPara.bSh1HasIC[i][j]==false)
                                {
                                    SThreadPara.ErrPartSh1+=IndexSuckName[i+iPos][j];
//                                    FRCarryKit.SetItemData(i, j, NULL_IC);    //JerryYang 20230908 : Mark掉,fix out shuttle lose IC叫不出來
                                    bHasError=true;
                                }
                            }
                        }
                        else if(Check32siteOnlyEnabled24Site()==true)
                        {
                            if(j==1 || j==2 || j==3 || j==4 || j==5 ||j==6)
                            {
                                if(FRCarryKit.Item[i][j]!=NULL_IC &&
                                   FRCarryKit.Item[i][j]!=HAS_NULL_IC &&
                                   SThreadPara.bSh1HasIC[i][j]==false)
                                {
                                    SThreadPara.ErrPartSh1+=IndexSuckName[i+iPos][j];
//                                    FRCarryKit.SetItemData(i, j, NULL_IC);    //JerryYang 20230908 : Mark掉,fix out shuttle lose IC叫不出來
                                    bHasError=true;
                                }
                            }
                        }
                        else
                        {
                            if(FRCarryKit.Item[i][j]!=NULL_IC &&
                               FRCarryKit.Item[i][j]!=HAS_NULL_IC &&
                               SThreadPara.bSh1HasIC[i][j]==false)
                            {
                                SThreadPara.ErrPartSh1+=IndexSuckName[i+iPos][j];
//                                FRCarryKit.SetItemData(i, j, NULL_IC);    //JerryYang 20230908 : Mark掉,fix out shuttle lose IC叫不出來
                                bHasError=true;
                            }
                        }
                    }
                    else if(FRCarryKit.Item[i][j]!=NULL_IC &&
                            FRCarryKit.Item[i][j]!=HAS_NULL_IC &&
                            SThreadPara.bSh1HasIC[i][j]==false)
                    {
                        SThreadPara.ErrPartSh1+=IndexSuckName[i+iPos][j];
                        bHasError=true;
                    }
                }
            }
        }
    }

    bSh1HasFloatingError=false;
    if(ENABLE_OUT_SHUTTLEY_LATCH==true &&                                       //Steven 20180329 (Jou) : 使用Y-Latch偵測置偏
       IniConfig.iF07OutShuttleSensorMode==2 &&
       SThreadPara.iScanSensor==2)
    {
        for(int j=0; j<FRCarryKit.iShtCol; j++)
        {
            for(int i=0; i<fLtcSensor->LatchDataCnt5; i++)
            {
                if(fLtcSensor->LatchDataTable5[i]<SThreadPara.iCheckPosMaxY[0][j] &&
                   fLtcSensor->LatchDataTable5[i]>SThreadPara.iCheckPosMinY[0][j])
                {
                    SThreadPara.bShICFloating[0][j]=false;
                    break;
                }
            }
        }

        for(int j=0; j<FRCarryKit.iShtCol; j++)
        {
            if(SThreadPara.bShICFloating[0][j]==true)                           //有錯的位置
            {
                SThreadPara.FloatingErrPartSh1+=IndexSuckName[0][j];
                bSh1HasFloatingError=true;
            }
        }
    }

    if((bHasError ||                                                            //Steven 20241111 : 紀錄out shuttle error
        fMain->cbShowShuttleSensor->Checked==true) &&
       bAlarm)                                                                  //有出錯就紀錄
    {
        OutShuttleLog();
        AnsiString sFileName;
        sFileName.sprintf("%04d-%02d-%02d %02d:%02d:%02d.%d", SystemYear, SystemMonth, SystemDate, SystemHour, SystemMin, SystemSec, SystemMSec);

        if(bCheckHasIC)
            sFileName+=" 殘料";
        else
            sFileName+=" 飛料";

        if(bHasError)
            sFileName+=" : Error!!";
        AnsiString str1="Sensor 0: ",
                   str2="Sensor 1: ",
                   str3="Sensor 5: ";

        if((SThreadPara.iScanSensor==1 && SThreadPara.bOutYUseLatch) ||         //Steven 20140818 : 康泰克Out Sensor Y改用Latch
           (ENABLE_OUT_SHUTTLEY_LATCH==true &&                                  //Steven 20180329 (Jou) : 使用Y-Latch偵測置偏
            IniConfig.iF07OutShuttleSensorMode==2 &&
            SThreadPara.iScanSensor==2))
        {
            for(int i=0; i<fLtcSensor->LatchDataCnt5; i++)
            {
                str3+=AnsiString(fLtcSensor->LatchDataTable5[i])+", ";
            }
        }
        else
        //if(FRCarryKit.iShtRow==2)                                               //Steven 20180401 : 移除else, 簡化code
        {
            for(int i=0; i<fLtcSensor->LatchDataCnt0; i++)
            {
                str1+=AnsiString(fLtcSensor->LatchDataTable0[i])+", ";
            }

            for(int i=0; i<fLtcSensor->LatchDataCnt1; i++)
            {
                str2+=AnsiString(fLtcSensor->LatchDataTable1[i])+", ";
            }
        }

        if(bHasError)                                                           //Steven 20241111 : 紀錄out shuttle error
        {
            if((SThreadPara.iScanSensor==1 && SThreadPara.bOutYUseLatch) ||     //Steven 20140818 : 康泰克Out Sensor Y改用Latch
               (ENABLE_OUT_SHUTTLEY_LATCH==true &&                              //Steven 20180329 (Jou) : 使用Y-Latch偵測置偏
                IniConfig.iF07OutShuttleSensorMode==2 &&
                SThreadPara.iScanSensor==2))
            {
                if(bCheckHasIC)
                    RecordProcess("Out Shuttle 1 has residual IC", str3);
                else
                    RecordProcess("Out Shuttle 1 has loose IC", str3);
            }

            if(SThreadPara.iScanSensor==2)                                      //Steven 20180401 : 移除else, 簡化code
            {
                if(bCheckHasIC)
                {
                    RecordProcess("Out Shuttle 1 has residual IC", str1);
                    RecordProcess("Out Shuttle 1 has residual IC", str2);
                }
                else
                {
                    RecordProcess("Out Shuttle 1 has loose IC", str1);
                    RecordProcess("Out Shuttle 1 has loose IC", str2);
                }
            }
        }

        if(fMain->cbShowShuttleSensor->Checked==true)
        {
            if(fMain->meShuttle1->Lines->Count>1024)
                fMain->meShuttle1->Clear();
            fMain->AddShuttleMessage(0, sFileName);

            if((SThreadPara.iScanSensor==1 && SThreadPara.bOutYUseLatch) ||     //Steven 20140818 : 康泰克Out Sensor Y改用Latch
               (ENABLE_OUT_SHUTTLEY_LATCH==true &&                              //Steven 20180329 (Jou) : 使用Y-Latch偵測置偏
                IniConfig.iF07OutShuttleSensorMode==2 &&
                SThreadPara.iScanSensor==2))
            {
                fMain->AddShuttleMessage(0, str3);
            }

            if(SThreadPara.iScanSensor==2)                                      //Steven 20180401 : 移除else, 簡化code
            {
                fMain->AddShuttleMessage(0, str1);
                fMain->AddShuttleMessage(0, str2);
            }
        }
    }
    return bHasError;
}
//------------------------------------------------------------------------------
//bCheckHasIC==true  : 檢查殘料
//bCheckHasIC==false : 檢查飛料
//------------------------------------------------------------------------------
bool CheckShuttlePos2_M204(bool bCheckHasIC, bool bAlarm)                       //JerryYang 20200610 Out shuttle lose IC需先讓位再發出alarm
{
    bool bHasError=false;
    if(IniConfig.bSPILFunction==true)                                           //JerryYang 20181228 矽品新竹守志要求Latch要能bypass
    {
        if(BRCarryKit.iShtRow==1 &&                                             //Steven 20240428 : 整合判斷方式
           TestIF.bNS7000kit==false)
        {
            if(Tech.OutSH1ZOneRowDetectPos==0 || Tech.OutSH2ZOneRowDetectPos==0)
                return false;
        }
        else if(TestIF_File.iTestMode==SingleSite)                              //Steven 20160919 : fix for 1x1偏心要跑Y sensor, 因為舊機台沒打洞
        {
            if(Tech.OutSH1ZOneRowDetectPos==0 || Tech.OutSH2ZOneRowDetectPos==0)
                return false;
        }
        else if(TestIF_File.iTestMode==_6Site2X3 ||
                TestIF_File.iTestMode==_10Site2X5)                              //Steven 20240428 : Site Col為奇數的話, 可以跑Y-Latch
        {
            if(Use_AxisY_Sensor_2x3mode==false || Tech.OutSH1ZOneRowDetectPos==0 || Tech.OutSH2ZOneRowDetectPos==0)
                return false;
        }
        else
        {
            if(Tech.OutSH1ZDetectPos==0 || Tech.OutSH2ZDetectPos==0)
                return false;
        }
    }

    for(int i=0; i<8; i++)
    {
        SThreadPara.bSh2HasIC[0][i]=true;
        SThreadPara.bSh2HasIC[1][i]=true;
        SThreadPara.bShICFloating[1][i]=true;
    }

    SThreadPara.ErrPartSh2="";
    if(SThreadPara.iScanSensor==1 && SThreadPara.bOutYUseLatch)                 //Steven 20140818 : 康泰克Out Sensor Y改用Latch
    {
        for(int j=0; j<BRCarryKit.iShtCol; j++)
        {
            for(int i=0; i<fLtcSensor->LatchDataCnt7; i++)
            {
                if(fLtcSensor->LatchDataTable7[i]<SThreadPara.iCheckPosMaxY[1][j] &&    //JerryYang 20181114 (Steven) : (Steven) : 0 -> 1, 修正Shuttle2 誤用 Out Shuttle 1的點位
                   fLtcSensor->LatchDataTable7[i]>SThreadPara.iCheckPosMinY[1][j])
                {
                    if(IsNNMode()==NN_2Row)
                    {
                        SThreadPara.bSh2HasIC[0][j]=false;
                        SThreadPara.bSh2HasIC[1][j]=false;
                    }
                    else
                    {
                        SThreadPara.bSh2HasIC[0][j]=false;                      //這是A行
                    }
                    break;
                }
            }
        }
    }
    else
    {
        for(int j=0; j<BRCarryKit.iShtCol; j++)
        {
            for(int i=0; i<fLtcSensor->LatchDataCnt2; i++)
            {
                //先找第2顆,因為偏心只用這一顆
                if(fLtcSensor->LatchDataTable2[i]<SThreadPara.iCheckPosMax[1][j] &&
                   fLtcSensor->LatchDataTable2[i]>SThreadPara.iCheckPosMin[1][j])
                {
                    if(BRCarryKit.iShtRow==2)
                        SThreadPara.bSh2HasIC[1][j]=false;                      //這是B行
                    else
                        SThreadPara.bSh2HasIC[0][j]=false;                      //這是A行
                    break;
                }
            }
            //一次兩顆,所以要找第3顆
            if(BRCarryKit.iShtRow==2)
            {
                for(int i=0; i<fLtcSensor->LatchDataCnt3; i++)
                {
                    if(fLtcSensor->LatchDataTable3[i]<SThreadPara.iCheckPosMax[1][j] &&
                       fLtcSensor->LatchDataTable3[i]>SThreadPara.iCheckPosMin[1][j])
                    {
                        SThreadPara.bSh2HasIC[0][j]=false;                      //這是A行
                        break;
                    }
                }
            }
        }
    }

    //檢查錯誤的位置
    if(SThreadPara.iScanSensor==1 && SThreadPara.bOutYUseLatch)                 //Steven 20140818 : 康泰克Out Sensor Y改用Latch
    {
        if(IsNNMode()==NN_1Row)
        {
            for(int j=0; j<BRCarryKit.iShtCol; j++)
            {
                if(bCheckHasIC==true)                                           //檢查殘料
                {
                    if(SThreadPara.bSh2HasIC[0][j]==bCheckHasIC)                //有錯的位置
                    {
                        SThreadPara.ErrPartSh2+=IndexSuckName[0][j];
                        bHasError=true;
                    }
                }
                else                                                            //檢查飛料
                {
                    if(((BRCarryKit.Item[0][j]!=NULL_IC && BRCarryKit.Item[0][j]!=HAS_NULL_IC) ||
                        (BRCarryKit.Item[1][j]!=NULL_IC && BRCarryKit.Item[1][j]!=HAS_NULL_IC)) &&  //JerryYang 20200824 : 修正out shuttle sensor判斷方式
                       SThreadPara.bSh2HasIC[0][j]==false)
                    {
                        SThreadPara.ErrPartSh2+=IndexSuckName[0][j];
                        SThreadPara.ErrPartSh2+=IndexSuckName[1][j];
                        bHasError=true;
                    }
                }
            }
        }
        else if((TestIF_File.iTestMode==_6Site2X3 ||
                 TestIF_File.iTestMode==_10Site2X5) &&                          //Steven 20240428 : Site Col為奇數的話, 可以跑Y-Latch
                Use_AxisY_Sensor_2x3mode==true)                                 //Isaac 20180402 (Steven) 2x3 mode 用Y sensor 偵測Outshuttle殘料
        {
            for(int i=0; i<BRCarryKit.iShtRow; i++)
            {
                for(int j=0; j<BRCarryKit.iShtCol; j++)
                {
                    if(bCheckHasIC==true)                                       //檢查殘料
                    {
                        if(SThreadPara.bSh2HasIC[0][j]==bCheckHasIC ||
                           SThreadPara.bSh2HasIC[1][j]==bCheckHasIC)            //有錯的位置
                        {
                            SThreadPara.ErrPartSh2+=IndexSuckName[i][j];
                            bHasError=true;
                        }
                    }
                    else                                                        //檢查飛料
                    {
                        if(((BRCarryKit.Item[0][j]!=NULL_IC && BRCarryKit.Item[0][j]!=HAS_NULL_IC) ||
                            (BRCarryKit.Item[1][j]!=NULL_IC && BRCarryKit.Item[1][j]!=HAS_NULL_IC)) &&  //JerryYang 20200824 : 修正out shuttle sensor判斷方式
                           SThreadPara.bSh2HasIC[0][j]==false)
                        {
                            SThreadPara.ErrPartSh2+=IndexSuckName[i][j];
                            bHasError=true;
                        }
                    }
                }
            }
        }
        else
        {
            for(int j=0; j<BRCarryKit.iShtCol; j++)
            {
                if(bCheckHasIC==true)                                           //檢查殘料
                {
                    if(SThreadPara.bSh2HasIC[0][j]==bCheckHasIC)                //有錯的位置
                    {
                        SThreadPara.ErrPartSh2+=IndexSuckName[0][j];
                        bHasError=true;
                    }
                }
                else                                                            //檢查飛料
                {
                    if(BRCarryKit.Item[0][j]!=NULL_IC &&
                       BRCarryKit.Item[0][j]!=HAS_NULL_IC &&
                       SThreadPara.bSh2HasIC[0][j]==false)
                    {
                        SThreadPara.ErrPartSh2+=IndexSuckName[0][j];
                        bHasError=true;
                    }
                }
            }
        }
    }
    else
    {
        for(int i=0; i<BRCarryKit.iShtRow; i++)
        {
            for(int j=0; j<BRCarryKit.iShtCol; j++)
            {
                if(bCheckHasIC==true)                                           //檢查殘料
                {
                    if(TestIF_File.b16Direct8Shuttle &&                         //Steven 20210322 : 16 Site Direct Heater use 8 Site shuttle
                       TestIF.iTestMode==_16Site2X8 &&
                       iCloseSiteStep_2x8==2)                                   //Steven 20240428 : 整合判斷方式
                    {
                        if(j!=0 && j!=1 && j!=6 && j!=7)
                        {
                            if(SThreadPara.bSh2HasIC[i][j]==bCheckHasIC)
                            {
                                SThreadPara.ErrPartSh2+=IndexSuckName[i][j];
                                bHasError=true;
                            }
                        }
                    }
                    else if((CUSTOMER_CODE==CC_KYEC_JCTHIU ||
                             CUSTOMER_CODE==CC_KYEC_LEE ||
                             IniConfig.bSPILFunction ||                         //JerryYang 20180209 (Steven) KYEC MTK 12 site的kit 選16 site模式中間12site
                             TestIF_File.b16Direct12Shuttle) &&                 //Steven 20210112 : Add function
                             TestIF.iTestMode==_16Site2X8 &&
                             bTestSiteUse[0][0][0]==false && bTestSiteUse[0][0][7]==false &&
                             bTestSiteUse[0][1][0]==false && bTestSiteUse[0][1][7]==false &&
                             TestIF_File.iSiteMap[0][0]==0 && TestIF_File.iSiteMap[0][7]==0 &&
                             TestIF_File.iSiteMap[1][0]==0 && TestIF_File.iSiteMap[1][7]==0)
                    {
                        if(j!=0 && j!=7)
                        {
                            if(SThreadPara.bSh2HasIC[i][j]==bCheckHasIC)
                            {
                                SThreadPara.ErrPartSh2+=IndexSuckName[i][j];
                                bHasError=true;
                            }
                        }
                    }
                    else if(TestIF.iTestMode==_16Site2X8 &&
                            iCloseSiteModeFor2x8==e2x8_STMMode)                 //JerryYang 20190729 STM 8 site交錯模式
                    {
                        if((i==0 && j==0) || (i==0 && j==2) || (i==0 && j==4) || (i==0 && j==6) || (i==1 && j==1) || (i==1 && j==3) || (i==1 && j==5) || (i==1 && j==7))
                        {
                        }
                        else
                        {
                            if(SThreadPara.bSh2HasIC[i][j]==bCheckHasIC)
                            {
                                SThreadPara.ErrPartSh2+=IndexSuckName[i][j];
                                bHasError=true;
                            }
                        }
                    }
                    else if(TestIF.iTestMode==_16Site2X8 &&
                            iCloseSiteModeFor2x8==e2x8_TW153Mode)               //Steven 20220816 : Add for TW153TK spec
                    {
                        if((i==0 && j==1) || (i==0 && j==3) || (i==0 && j==5) || (i==0 && j==7) || (i==1 && j==0) || (i==1 && j==2) || (i==1 && j==4) || (i==1 && j==6))
                        {
                        }
                        else
                        {
                            if(SThreadPara.bSh2HasIC[i][j]==bCheckHasIC)
                            {
                                SThreadPara.ErrPartSh2+=IndexSuckName[i][j];
                                bHasError=true;
                            }
                        }
                    }
                    else if(CUSTOMER_CODE==CC_AMKOR_Philippines &&
                            TestIF.iTestMode==_32Site4X8N)                      //JerryYang 20210409 : add for ATP
                    {
                        if(Check32siteOnlyEnabled4x4Site()==true)
                        {
                            if(j==2 || j==3 || j==4 || j==5)
                            {
                                if(SThreadPara.bSh2HasIC[i][j]==bCheckHasIC)    //有錯的位置
                                {
                                    SThreadPara.ErrPartSh2+=IndexSuckName[i][j];
                                    bHasError=true;
                                }
                            }
                        }
                        else if(Check32siteOnlyEnabled24Site()==true)
                        {
                            if(j==1 || j==2 || j==3 || j==4 || j==5 ||j==6)
                            {
                                if(SThreadPara.bSh2HasIC[i][j]==bCheckHasIC)    //有錯的位置
                                {
                                    SThreadPara.ErrPartSh2+=IndexSuckName[i][j];
                                    bHasError=true;
                                }
                            }
                        }
                        else
                        {
                            if(SThreadPara.bSh2HasIC[i][j]==bCheckHasIC)        //有錯的位置
                            {
                                SThreadPara.ErrPartSh2+=IndexSuckName[i][j];
                                bHasError=true;
                            }
                        }
                    }
                    else if(SThreadPara.bSh2HasIC[i][j]==bCheckHasIC)           //有錯的位置
                    {
                        SThreadPara.ErrPartSh2+=IndexSuckName[i][j];
                        bHasError=true;
                    }
                }
                else                                                            //檢查飛料
                {
                    if(TestIF_File.b16Direct8Shuttle &&                         //Steven 20210322 : 16 Site Direct Heater use 8 Site shuttle
                       TestIF.iTestMode==_16Site2X8 &&
                       iCloseSiteStep_2x8==2)                                   //Steven 20240428 : 整合判斷方式
                    {
                        if(j!=0 && j!=1 && j!=6 && j!=7)
                        {
                            if(BRCarryKit.Item[i][j]!=NULL_IC &&
                               BRCarryKit.Item[i][j]!=HAS_NULL_IC &&            //Sam 20210304 : NULL --> NULL_IC
                               SThreadPara.bSh2HasIC[i][j]==false)
                            {
                                SThreadPara.ErrPartSh2+=IndexSuckName[i][j];
                                bHasError=true;
                            }
                        }
                    }
                    else if((CUSTOMER_CODE==CC_KYEC_JCTHIU ||
                             CUSTOMER_CODE==CC_KYEC_LEE ||
                             IniConfig.bSPILFunction ||                         //JerryYang 20180209 (Steven) KYEC MTK 12 site的kit 選16 site模式中間12site
                             TestIF_File.b16Direct12Shuttle) &&                 //Steven 20210112 : Add function
                             TestIF.iTestMode==_16Site2X8 &&
                             bTestSiteUse[0][0][0]==false && bTestSiteUse[0][0][7]==false &&
                             bTestSiteUse[0][1][0]==false && bTestSiteUse[0][1][7]==false &&
                             TestIF_File.iSiteMap[0][0]==0 && TestIF_File.iSiteMap[0][7]==0 &&
                             TestIF_File.iSiteMap[1][0]==0 && TestIF_File.iSiteMap[1][7]==0)
                    {
                        if(j!=0 && j!=7)
                        {
                            if(BRCarryKit.Item[i][j]!=NULL_IC &&
                               BRCarryKit.Item[i][j]!=HAS_NULL_IC &&            //Sam 20210304 : NULL --> NULL_IC
                               SThreadPara.bSh2HasIC[i][j]==false)
                            {
                                SThreadPara.ErrPartSh2+=IndexSuckName[i][j];
                                bHasError=true;
                            }
                        }
                    }
                    else if(TestIF.iTestMode==_16Site2X8 &&
                            iCloseSiteModeFor2x8==e2x8_STMMode)                 //JerryYang 20190729 STM 8 site交錯模式
                    {
                        if((i==0 && j==0) || (i==0 && j==2) || (i==0 && j==4) || (i==0 && j==6) || (i==1 && j==1) || (i==1 && j==3) || (i==1 && j==5) || (i==1 && j==7))
                        {
                        }
                        else
                        {
                            if(BRCarryKit.Item[i][j]!=NULL_IC &&
                               BRCarryKit.Item[i][j]!=HAS_NULL_IC &&            //Sam 20210304 : NULL --> NULL_IC
                               SThreadPara.bSh2HasIC[i][j]==false)
                            {
                                SThreadPara.ErrPartSh2+=IndexSuckName[i][j];
                                bHasError=true;
                            }
                        }
                    }
                    else if(TestIF.iTestMode==_16Site2X8 &&
                            iCloseSiteModeFor2x8==e2x8_TW153Mode)               //Steven 20220816 : Add for TW153TK spec
                    {
                        if((i==0 && j==1) || (i==0 && j==3) || (i==0 && j==5) || (i==0 && j==7) || (i==1 && j==0) || (i==1 && j==2) || (i==1 && j==4) || (i==1 && j==6))
                        {
                        }
                        else
                        {
                            if(BRCarryKit.Item[i][j]!=NULL_IC &&
                               BRCarryKit.Item[i][j]!=HAS_NULL_IC &&            //Sam 20210304 : NULL --> NULL_IC
                               SThreadPara.bSh2HasIC[i][j]==false)
                            {
                                SThreadPara.ErrPartSh2+=IndexSuckName[i][j];
                                bHasError=true;
                            }
                        }
                    }
                    else if(CUSTOMER_CODE==CC_AMKOR_Philippines &&
                            TestIF.iTestMode==_32Site4X8N)                      //JerryYang 20210409 : add for ATP
                    {
                        if(Check32siteOnlyEnabled4x4Site()==true)
                        {
                            if(j==2 || j==3 || j==4 || j==5)
                            {
                                if(BRCarryKit.Item[i][j]!=NULL &&
                                   BRCarryKit.Item[i][j]!=HAS_NULL_IC &&
                                   SThreadPara.bSh2HasIC[i][j]==false)
                                {
                                    SThreadPara.ErrPartSh2+=IndexSuckName[i][j];
//                                    BRCarryKit.SetItemData(i, j, NULL);       //JerryYang 20230908 : Mark掉,fix out shuttle lose IC叫不出來
                                    bHasError=true;
                                }
                            }
                        }
                        else if(Check32siteOnlyEnabled24Site()==true)
                        {
                            if(j==1 || j==2 || j==3 || j==4 || j==5 ||j==6)
                            {
                                if(BRCarryKit.Item[i][j]!=NULL &&
                                   BRCarryKit.Item[i][j]!=HAS_NULL_IC &&
                                   SThreadPara.bSh2HasIC[i][j]==false)
                                {
                                    SThreadPara.ErrPartSh2+=IndexSuckName[i][j];
//                                    BRCarryKit.SetItemData(i, j, NULL);       //JerryYang 20230908 : Mark掉,fix out shuttle lose IC叫不出來
                                    bHasError=true;
                                }
                            }
                        }
                        else if(BRCarryKit.Item[i][j]!=NULL &&
                                BRCarryKit.Item[i][j]!=HAS_NULL_IC &&
                           SThreadPara.bSh2HasIC[i][j]==false)
                        {
                            SThreadPara.ErrPartSh2+=IndexSuckName[i][j];
//                            BRCarryKit.SetItemData(i, j, NULL);               //JerryYang 20230908 : Mark掉,fix out shuttle lose IC叫不出來
                            bHasError=true;
                        }
                    }
                    else if(BRCarryKit.Item[i][j]!=NULL_IC &&
                            BRCarryKit.Item[i][j]!=HAS_NULL_IC &&               //Sam 20210304 : NULL --> NULL_IC
                       SThreadPara.bSh2HasIC[i][j]==false)
                    {
                        SThreadPara.ErrPartSh2+=IndexSuckName[i][j];
                        bHasError=true;
                    }
                }
            }
        }
    }

    bSh2HasFloatingError=false;
    if(ENABLE_OUT_SHUTTLEY_LATCH==true &&                                       //Steven 20180329 (Jou) : 使用Y-Latch偵測置偏
       IniConfig.iF07OutShuttleSensorMode==2 &&
       SThreadPara.iScanSensor==2)
    {
        for(int j=0; j<BRCarryKit.iShtCol; j++)
        {
            for(int i=0; i<fLtcSensor->LatchDataCnt7; i++)                      //JerryYang 20181114 (Steven) : (Steven) : 5->7
            {
                if(fLtcSensor->LatchDataTable7[i]<SThreadPara.iCheckPosMaxY[1][j] &&
                   fLtcSensor->LatchDataTable7[i]>SThreadPara.iCheckPosMinY[1][j])
                {
                    SThreadPara.bShICFloating[1][j]=false;
                    break;
                }
            }
        }

        for(int j=0; j<BRCarryKit.iShtCol; j++)
        {
            if(SThreadPara.bShICFloating[1][j]==true)                           //有錯的位置
            {
                SThreadPara.FloatingErrPartSh2+=IndexSuckName[0][j];
                bSh2HasFloatingError=true;
            }
        }
    }

    if((bHasError ||                                                            //Steven 20241111 : 紀錄out shuttle error
        fMain->cbShowShuttleSensor->Checked==true) &&
       bAlarm)                                                                  //有出錯就紀錄
    {
        OutShuttleLog();
        AnsiString sFileName;
        sFileName.sprintf("%04d-%02d-%02d %02d:%02d:%02d.%d", SystemYear, SystemMonth, SystemDate, SystemHour, SystemMin, SystemSec, SystemMSec);
        if(bHasError)
            sFileName+=" : Error!!";

        if(bCheckHasIC)
            sFileName+=" 殘料";
        else
            sFileName+=" 飛料";

        AnsiString str1="Sensor 2: ",
                   str2="Sensor 3: ",
                   str3="Sensor 7: ";

        if(SThreadPara.iScanSensor==1 && SThreadPara.bOutYUseLatch)             //Steven 20140818 : 康泰克Out Sensor Y改用Latch
        {
            for(int i=0; i<fLtcSensor->LatchDataCnt7; i++)
            {
                str3+=AnsiString(fLtcSensor->LatchDataTable7[i])+", ";
            }
        }
        else
        {
            if(BRCarryKit.iShtRow==2)
            {
                for(int i=0; i<fLtcSensor->LatchDataCnt3; i++)
                {
                    str1+=AnsiString(fLtcSensor->LatchDataTable3[i])+", ";
                }
            }

            for(int i=0; i<fLtcSensor->LatchDataCnt2; i++)
            {
                str2+=AnsiString(fLtcSensor->LatchDataTable2[i])+", ";
            }
        }

        if(bHasError)                                                           //Steven 20241111 : 紀錄out shuttle error
        {
            if((SThreadPara.iScanSensor==1 && SThreadPara.bOutYUseLatch) ||     //Steven 20140818 : 康泰克Out Sensor Y改用Latch
               (ENABLE_OUT_SHUTTLEY_LATCH==true &&                              //Steven 20180329 (Jou) : 使用Y-Latch偵測置偏
                IniConfig.iF07OutShuttleSensorMode==2 &&
                SThreadPara.iScanSensor==2))
            {
                if(bCheckHasIC)
                    RecordProcess("Out Shuttle 2 has residual IC", str3);
                else
                    RecordProcess("Out Shuttle 2 has loose IC", str3);
            }

            if(SThreadPara.iScanSensor==2)                                      //Steven 20180401 : 移除else, 簡化code
            {
                if(bCheckHasIC)
                {
                    RecordProcess("Out Shuttle 2 has residual IC", str1);
                    RecordProcess("Out Shuttle 2 has residual IC", str2);
                }
                else
                {
                    RecordProcess("Out Shuttle 2 has loose IC", str1);
                    RecordProcess("Out Shuttle 2 has loose IC", str2);
                }
            }
        }

        if(fMain->cbShowShuttleSensor->Checked==true)
        {
            if(fMain->meShuttle2->Lines->Count>1024)
                fMain->meShuttle2->Clear();

            fMain->AddShuttleMessage(1, sFileName);
            if((SThreadPara.iScanSensor==1 && SThreadPara.bOutYUseLatch) ||     //Steven 20140818 : 康泰克Out Sensor Y改用Latch
               (ENABLE_OUT_SHUTTLEY_LATCH==true &&                              //Steven 20180329 (Jou) : 使用Y-Latch偵測置偏
                IniConfig.iF07OutShuttleSensorMode==2 &&
                SThreadPara.iScanSensor==2))
            {
                fMain->AddShuttleMessage(1, str3);
            }
            else
            {
                fMain->AddShuttleMessage(1, str1);
                fMain->AddShuttleMessage(1, str2);
            }
        }
    }

    return bHasError;
}
//------------------------------------------------------------------------------
bool CheckShuttle1OutputHasICError_M204()                                       //檢查Shuttle 1 IC殘留_M204
{
    #ifdef SOFT_SIMULTE
        return false;
    #else
    {
        if(LastSet.iRealDummy!=REALLY &&
           fMain->cbTestOutShuttleSensor->Checked==false)                       //Steven 20150727 : 增加Out Shuttle Sensor確認功能
        {
            return false;
        }

        if(SThreadPara.bUseM204Mode==false)                                     //Steven 20110527
        {
            return false;
        }

        bool bHasIC=false;
        bHasIC=CheckShuttlePos1_M204(!FRCarryKit.UseSiteHasIC());               //Steven 20110701  //jou 20150313 true-->!FRCarryKit.HasIC()

        return bHasIC;
    }
    #endif
}
//------------------------------------------------------------------------------
bool CheckShuttle2OutputHasICError_M204()                                       //檢查Shuttle 2 IC殘留_M204
{
    #ifdef SOFT_SIMULTE
        return false;
    #else
    {
        if(LastSet.iRealDummy!=REALLY &&
           fMain->cbTestOutShuttleSensor->Checked==false)                       //Steven 20150727 : 增加Out Shuttle Sensor
        {
            return false;
        }

        if(SThreadPara.bUseM204Mode==false)                                     //Steven 20110527
            return false;

        bool bHasIC=false;
        bHasIC=CheckShuttlePos2_M204(!BRCarryKit.UseSiteHasIC());               //Steven 20110701   //jou 20150313 true-->!BRCarryKit.HasIC()
        return bHasIC;
    }
    #endif
}
//------------------------------------------------------------------------------
bool CheckShuttleOutputHasICError(int iSelSHT, int &X, int &Y)                  //檢查IC殘留
{
    int iShuttleZPitch=5000;

    if(iSelSHT==0)
        ptrOutSHT=&FRCarryKit;
    else
        ptrOutSHT=&BRCarryKit;

    int iCHKSensor[2]={SnOutPutSHT1S1, SnOutPutSHT2S1};
    int iCHKZSensor[2][2]=                                                      //JerryYang 20151230 矽格湖口 要求關掉Out shuttle Z enable時不檢查
    {
        {SnOutPutSHT1ZS1, SnOutPutSHT1ZS2},
        {SnOutPutSHT2ZS1, SnOutPutSHT2ZS2}
    };

    if(Sen[iCHKSensor[iSelSHT]].Enable==false || Sen[iCHKSensor[iSelSHT]].Enable==false)
    {
        return false;
    }

    if(CUSTOMER_CODE==CC_SIGURD_HUKOU && (Sen[iCHKZSensor[iSelSHT][0]].Enable==false || Sen[iCHKZSensor[iSelSHT][1]].Enable==false))//JerryYang 20151230 矽格湖口 要求關掉Out shuttle Z enable時不檢查
    {
        return false;
    }

    if(ENABLE_OUT_SHUTTLE_SENEOR && (Sen[iCHKZSensor[iSelSHT][0]].Enable==false || Sen[iCHKZSensor[iSelSHT][1]].Enable==false)) //Steven 20160411 Add
    {
        return false;
    }

    if(LastSet.iRealDummy!=REALLY && fMain->cbTestOutShuttleSensor->Checked==false)     //Steven 20150727 : 增加Out Shuttle Sensor
    {
        return false;
    }

    if(BRCarryKit.iShtRow &&                                                    //Steven 20240428 : 整合判斷方式
       TestIF.bNS7000kit==false)
    {
        if(Tech.OutSH1ZOneRowDetectPos==0 || Tech.OutSH2ZOneRowDetectPos==0)
            return false;
    }
    else if(TestIF_File.iTestMode==SingleSite)                                  //Steven 20160919 : fix for 1x1偏心要跑Y sensor, 因為舊機台沒打洞
    {
        if(Tech.OutSH1ZOneRowDetectPos==0 || Tech.OutSH2ZOneRowDetectPos==0)
            return false;
    }
    else if(TestIF_File.iTestMode==_6Site2X3 ||
            TestIF_File.iTestMode==_10Site2X5)                                  //Steven 20240428 : Site Col為奇數的話, 可以跑Y-Latch
    {
        if(Use_AxisY_Sensor_2x3mode==false || Tech.OutSH1ZOneRowDetectPos==0 || Tech.OutSH2ZOneRowDetectPos==0)
            return false;
    }
    else
    {
        if(Tech.OutSH1ZDetectPos==0 || Tech.OutSH2ZDetectPos==0)
            return false;
    }

    if(SHUTTLE_Z_TYPE==0)                                                       //jou 2013-07-26 : 選擇 Shuttle Z Sensor Type
        iShuttleZPitch=5000;
    else                                                                        //jou 2010-09-08 shuttle z 上下對照sensor,機構sensor無法到達
        iShuttleZPitch=2300;

    if((TestIF_File.iTestMode==_6Site2X3 ||
        TestIF_File.iTestMode==_10Site2X5) &&                                   //Steven 20240428 : Site Col為奇數的話, 可以跑Y-Latch
       Use_AxisY_Sensor_2x3mode==true)                                          //Isaac 20180402 2x3 (Steven) mode 用Y sensor 偵測Outshuttle殘料
    {
    }
    else if(BRCarryKit.iShtRow==2 &&
            TestIF.dSiteYPitch<iShuttleZPitch)
    {
        return false;
    }

    switch(TestIF.iTestMode)
    {
        case SingleSite:                                                        //jou 2013-03-21 make code , 1x1 , 1x2 , 1x4 也要用側邊sensor檢查
            if(bShuttleHasIC[iSelSHT][0][0] && ptrOutSHT->Item[0][0]==NULL_IC)  //ChungHung 20150507 fix Device superfluous error
            {
                if(bShuttleHasIC[iSelSHT][0][0]){X=0;Y=0;}
                    return true;
            }
            break;
        case DualSite:                                                          //1X2
        case QualSite2X2N:                                                      //Frank 20200520 2X2NN Mode
            if((bShuttleHasIC[iSelSHT][0][0] && ptrOutSHT->Item[0][0]==NULL_IC) ||
               (bShuttleHasIC[iSelSHT][0][1] && ptrOutSHT->Item[0][1]==NULL_IC))
            {
                     if(bShuttleHasIC[iSelSHT][0][0]){X=0; Y=0;}
                else if(bShuttleHasIC[iSelSHT][0][1]){X=1; Y=0;}
                return true;
            }
            break;
        case QualSite1X4:                                                       //1X4
        case _8Site1X4:                                                         //ChungHung 20150528 add for 海思 _8Site1x4
        case _8Site2X4N:                                                        //Wei 20231211 : 2X4NN Mode
            if((bShuttleHasIC[iSelSHT][0][0] && ptrOutSHT->Item[0][0]==NULL_IC) ||
               (bShuttleHasIC[iSelSHT][0][1] && ptrOutSHT->Item[0][1]==NULL_IC) ||
               (bShuttleHasIC[iSelSHT][0][2] && ptrOutSHT->Item[0][2]==NULL_IC) ||
               (bShuttleHasIC[iSelSHT][0][3] && ptrOutSHT->Item[0][3]==NULL_IC))
            {
                     if(bShuttleHasIC[iSelSHT][0][0]){X=0;Y=0;}
                else if(bShuttleHasIC[iSelSHT][0][1]){X=1;Y=0;}
                else if(bShuttleHasIC[iSelSHT][0][2]){X=2;Y=0;}
                else if(bShuttleHasIC[iSelSHT][0][3]){X=3;Y=0;}
                return true;
            }
            break;
        case TriSite1X3:                                                        //Frank 20160329 add for 1x3_4
        case _6Site2X3N:                                                        //Steven 20220425 : 2X3NN Mode
            if((bShuttleHasIC[iSelSHT][0][0] && ptrOutSHT->Item[0][0]==NULL_IC) ||
               (bShuttleHasIC[iSelSHT][0][1] && ptrOutSHT->Item[0][1]==NULL_IC) ||
               (bShuttleHasIC[iSelSHT][0][2] && ptrOutSHT->Item[0][2]==NULL_IC))
            {
                     if(bShuttleHasIC[iSelSHT][0][0]){X=0;Y=0;}
                else if(bShuttleHasIC[iSelSHT][0][1]){X=1;Y=0;}
                else if(bShuttleHasIC[iSelSHT][0][2]){X=2;Y=0;}
                return true;
            }
            break;
        case QualSite2X2:                                                       //Qual mode 2X2
            if((bShuttleHasIC[iSelSHT][0][0] && ptrOutSHT->Item[0][0]==NULL_IC) ||
               (bShuttleHasIC[iSelSHT][0][1] && ptrOutSHT->Item[0][1]==NULL_IC) ||
               (bShuttleHasIC[iSelSHT][1][0] && ptrOutSHT->Item[1][0]==NULL_IC) ||
               (bShuttleHasIC[iSelSHT][1][1] && ptrOutSHT->Item[1][1]==NULL_IC))
            {
                     if(bShuttleHasIC[iSelSHT][0][0]){X=0;Y=0;}
                else if(bShuttleHasIC[iSelSHT][0][1]){X=1;Y=0;}
                else if(bShuttleHasIC[iSelSHT][1][0]){X=0;Y=1;}
                else if(bShuttleHasIC[iSelSHT][1][1]){X=1;Y=1;}
                return true;
            }
            break;
        case _6Site2X3:                                                         //ChungHung 20140115 add for 2x3_6
            if((bShuttleHasIC[iSelSHT][0][0] && ptrOutSHT->Item[0][0]==NULL_IC) ||
               (bShuttleHasIC[iSelSHT][0][1] && ptrOutSHT->Item[0][1]==NULL_IC) ||
               (bShuttleHasIC[iSelSHT][0][2] && ptrOutSHT->Item[0][2]==NULL_IC) ||
               (bShuttleHasIC[iSelSHT][1][0] && ptrOutSHT->Item[1][0]==NULL_IC) ||
               (bShuttleHasIC[iSelSHT][1][1] && ptrOutSHT->Item[1][1]==NULL_IC) ||
               (bShuttleHasIC[iSelSHT][1][2] && ptrOutSHT->Item[1][2]==NULL_IC))
            {
                     if(bShuttleHasIC[iSelSHT][0][0]){X=0;Y=0;}
                else if(bShuttleHasIC[iSelSHT][0][1]){X=1;Y=0;}
                else if(bShuttleHasIC[iSelSHT][0][2]){X=2;Y=0;}
                else if(bShuttleHasIC[iSelSHT][1][0]){X=0;Y=1;}
                else if(bShuttleHasIC[iSelSHT][1][1]){X=1;Y=1;}
                else if(bShuttleHasIC[iSelSHT][1][2]){X=2;Y=1;}
                return true;
            }
            break;
        case _8Site2X4:                                                         //2x4
        case _16Site4X4:                                                        //Sam 20190226 : 16Site4X4
            if((bShuttleHasIC[iSelSHT][0][0] && ptrOutSHT->Item[0][0]==NULL_IC) ||
               (bShuttleHasIC[iSelSHT][0][1] && ptrOutSHT->Item[0][1]==NULL_IC) ||
               (bShuttleHasIC[iSelSHT][0][2] && ptrOutSHT->Item[0][2]==NULL_IC) ||
               (bShuttleHasIC[iSelSHT][0][3] && ptrOutSHT->Item[0][3]==NULL_IC) ||
               (bShuttleHasIC[iSelSHT][1][0] && ptrOutSHT->Item[1][0]==NULL_IC) ||
               (bShuttleHasIC[iSelSHT][1][1] && ptrOutSHT->Item[1][1]==NULL_IC) ||
               (bShuttleHasIC[iSelSHT][1][2] && ptrOutSHT->Item[1][2]==NULL_IC) ||
               (bShuttleHasIC[iSelSHT][1][3] && ptrOutSHT->Item[1][3]==NULL_IC))
            {
                     if(bShuttleHasIC[iSelSHT][0][0]){X=0;Y=0;}
                else if(bShuttleHasIC[iSelSHT][0][1]){X=1;Y=0;}
                else if(bShuttleHasIC[iSelSHT][0][2]){X=2;Y=0;}
                else if(bShuttleHasIC[iSelSHT][0][3]){X=3;Y=0;}
                else if(bShuttleHasIC[iSelSHT][1][0]){X=0;Y=1;}
                else if(bShuttleHasIC[iSelSHT][1][1]){X=1;Y=1;}
                else if(bShuttleHasIC[iSelSHT][1][2]){X=2;Y=1;}
                else if(bShuttleHasIC[iSelSHT][1][3]){X=3;Y=1;}
                return true;
            }
            break;
        case _10Site2X5:                                                        //wei 20190614 10 site
            if((bShuttleHasIC[iSelSHT][0][0] && ptrOutSHT->Item[0][0]==NULL_IC) ||
               (bShuttleHasIC[iSelSHT][0][1] && ptrOutSHT->Item[0][1]==NULL_IC) ||
               (bShuttleHasIC[iSelSHT][0][2] && ptrOutSHT->Item[0][2]==NULL_IC) ||
               (bShuttleHasIC[iSelSHT][0][3] && ptrOutSHT->Item[0][3]==NULL_IC) ||
               (bShuttleHasIC[iSelSHT][0][4] && ptrOutSHT->Item[0][4]==NULL_IC) ||
               (bShuttleHasIC[iSelSHT][1][0] && ptrOutSHT->Item[1][0]==NULL_IC) ||
               (bShuttleHasIC[iSelSHT][1][1] && ptrOutSHT->Item[1][1]==NULL_IC) ||
               (bShuttleHasIC[iSelSHT][1][2] && ptrOutSHT->Item[1][2]==NULL_IC) ||
               (bShuttleHasIC[iSelSHT][1][3] && ptrOutSHT->Item[1][3]==NULL_IC) ||
               (bShuttleHasIC[iSelSHT][1][4] && ptrOutSHT->Item[1][4]==NULL_IC))
            {
                     if(bShuttleHasIC[iSelSHT][0][0]){X=0;Y=0;}
                else if(bShuttleHasIC[iSelSHT][0][1]){X=1;Y=0;}
                else if(bShuttleHasIC[iSelSHT][0][2]){X=2;Y=0;}
                else if(bShuttleHasIC[iSelSHT][0][3]){X=3;Y=0;}
                else if(bShuttleHasIC[iSelSHT][0][4]){X=4;Y=0;}
                else if(bShuttleHasIC[iSelSHT][1][0]){X=0;Y=1;}
                else if(bShuttleHasIC[iSelSHT][1][1]){X=1;Y=1;}
                else if(bShuttleHasIC[iSelSHT][1][2]){X=2;Y=1;}
                else if(bShuttleHasIC[iSelSHT][1][3]){X=3;Y=1;}
                else if(bShuttleHasIC[iSelSHT][1][4]){X=4;Y=1;}
                return true;
            }
            break;
        case _12Site2X6:                                                        //ChungHung 20130507 add HT9045 updata for 12site 517
            if((bShuttleHasIC[iSelSHT][0][0] && ptrOutSHT->Item[0][0]==NULL_IC) ||
               (bShuttleHasIC[iSelSHT][0][1] && ptrOutSHT->Item[0][1]==NULL_IC) ||
               (bShuttleHasIC[iSelSHT][0][2] && ptrOutSHT->Item[0][2]==NULL_IC) ||
               (bShuttleHasIC[iSelSHT][0][3] && ptrOutSHT->Item[0][3]==NULL_IC) ||
               (bShuttleHasIC[iSelSHT][0][4] && ptrOutSHT->Item[0][4]==NULL_IC) ||
               (bShuttleHasIC[iSelSHT][0][5] && ptrOutSHT->Item[0][5]==NULL_IC) ||
               (bShuttleHasIC[iSelSHT][1][0] && ptrOutSHT->Item[1][0]==NULL_IC) ||
               (bShuttleHasIC[iSelSHT][1][1] && ptrOutSHT->Item[1][1]==NULL_IC) ||
               (bShuttleHasIC[iSelSHT][1][2] && ptrOutSHT->Item[1][2]==NULL_IC) ||
               (bShuttleHasIC[iSelSHT][1][3] && ptrOutSHT->Item[1][3]==NULL_IC) ||
               (bShuttleHasIC[iSelSHT][1][4] && ptrOutSHT->Item[1][4]==NULL_IC) ||
               (bShuttleHasIC[iSelSHT][1][5] && ptrOutSHT->Item[1][5]==NULL_IC))
            {
                     if(bShuttleHasIC[iSelSHT][0][0]){X=0;Y=0;}
                else if(bShuttleHasIC[iSelSHT][0][1]){X=1;Y=0;}
                else if(bShuttleHasIC[iSelSHT][0][2]){X=2;Y=0;}
                else if(bShuttleHasIC[iSelSHT][0][3]){X=3;Y=0;}
                else if(bShuttleHasIC[iSelSHT][0][4]){X=4;Y=0;}
                else if(bShuttleHasIC[iSelSHT][0][5]){X=5;Y=0;}
                else if(bShuttleHasIC[iSelSHT][1][0]){X=0;Y=1;}
                else if(bShuttleHasIC[iSelSHT][1][1]){X=1;Y=1;}
                else if(bShuttleHasIC[iSelSHT][1][2]){X=2;Y=1;}
                else if(bShuttleHasIC[iSelSHT][1][3]){X=3;Y=1;}
                else if(bShuttleHasIC[iSelSHT][1][4]){X=4;Y=1;}
                else if(bShuttleHasIC[iSelSHT][1][5]){X=5;Y=1;}
                return true;
            }
            break;
        case _16Site2X8:                                                        //2x8
        case _32Site4X8N:                                                       //Steven 20140512 : For HT-9047
            if((bShuttleHasIC[iSelSHT][0][0] && ptrOutSHT->Item[0][0]==NULL_IC) ||
               (bShuttleHasIC[iSelSHT][0][1] && ptrOutSHT->Item[0][1]==NULL_IC) ||
               (bShuttleHasIC[iSelSHT][0][2] && ptrOutSHT->Item[0][2]==NULL_IC) ||
               (bShuttleHasIC[iSelSHT][0][3] && ptrOutSHT->Item[0][3]==NULL_IC) ||
               (bShuttleHasIC[iSelSHT][0][4] && ptrOutSHT->Item[0][4]==NULL_IC) ||
               (bShuttleHasIC[iSelSHT][0][5] && ptrOutSHT->Item[0][5]==NULL_IC) ||
               (bShuttleHasIC[iSelSHT][0][6] && ptrOutSHT->Item[0][6]==NULL_IC) ||
               (bShuttleHasIC[iSelSHT][0][7] && ptrOutSHT->Item[0][7]==NULL_IC) ||
               (bShuttleHasIC[iSelSHT][1][0] && ptrOutSHT->Item[1][0]==NULL_IC) ||
               (bShuttleHasIC[iSelSHT][1][1] && ptrOutSHT->Item[1][1]==NULL_IC) ||
               (bShuttleHasIC[iSelSHT][1][2] && ptrOutSHT->Item[1][2]==NULL_IC) ||
               (bShuttleHasIC[iSelSHT][1][3] && ptrOutSHT->Item[1][3]==NULL_IC) ||
               (bShuttleHasIC[iSelSHT][1][4] && ptrOutSHT->Item[1][4]==NULL_IC) ||
               (bShuttleHasIC[iSelSHT][1][5] && ptrOutSHT->Item[1][5]==NULL_IC) ||
               (bShuttleHasIC[iSelSHT][1][6] && ptrOutSHT->Item[1][6]==NULL_IC) ||
               (bShuttleHasIC[iSelSHT][1][7] && ptrOutSHT->Item[1][7]==NULL_IC))
            {
                     if(bShuttleHasIC[iSelSHT][0][0]){X=0;Y=0;}
                else if(bShuttleHasIC[iSelSHT][0][1]){X=1;Y=0;}
                else if(bShuttleHasIC[iSelSHT][0][2]){X=2;Y=0;}
                else if(bShuttleHasIC[iSelSHT][0][3]){X=3;Y=0;}
                else if(bShuttleHasIC[iSelSHT][0][4]){X=4;Y=0;}
                else if(bShuttleHasIC[iSelSHT][0][5]){X=5;Y=0;}
                else if(bShuttleHasIC[iSelSHT][0][6]){X=6;Y=0;}
                else if(bShuttleHasIC[iSelSHT][0][7]){X=7;Y=0;}
                else if(bShuttleHasIC[iSelSHT][1][0]){X=0;Y=1;}
                else if(bShuttleHasIC[iSelSHT][1][1]){X=1;Y=1;}
                else if(bShuttleHasIC[iSelSHT][1][2]){X=2;Y=1;}
                else if(bShuttleHasIC[iSelSHT][1][3]){X=3;Y=1;}
                else if(bShuttleHasIC[iSelSHT][1][4]){X=4;Y=1;}
                else if(bShuttleHasIC[iSelSHT][1][5]){X=5;Y=1;}
                else if(bShuttleHasIC[iSelSHT][1][6]){X=6;Y=1;}
                else if(bShuttleHasIC[iSelSHT][1][7]){X=7;Y=1;}
                return true;
            }
            break;
    }
    return false;
}
//------------------------------------------------------------------------------
bool ShowOutputShuttleICStatus_M204(int iShuttleNo, bool bAlarm)
{
    int ret=0, iCT=0;                                                           //Frank 20171213 (Steven) : 若Shuttle Lose IC 將IC 放Fail Bin
    bool bShuttleLoseIC[2]={false, false};                                      //Frank 20171213 (Steven) : 若Shuttle Lose IC 將IC 放Fail Bin
    bool bHasErr=false, bHasFloatingErr=false;
    AnsiString sBuffer="";
    AnsiString ErrPart=" ";

    #ifndef SOFT_SIMULTE
    if(iShuttleNo==0)
    {
        bHasErr=CheckShuttlePos1_M204(false, bAlarm);
        bHasFloatingErr=bSh1HasFloatingError;
    }
    else
    {
        bHasErr=CheckShuttlePos2_M204(false, bAlarm);
        bHasFloatingErr=bSh2HasFloatingError;
    }
    #endif

    if(bHasErr==true && bAlarm==true)
    {
        if(IniConfig.bOutShLoseNeedOpenChamber==true ||                         //jou 2013-12-12 Out Shuttle Lose Device Need Open Chamber Door and press Z1
           IniConfig.bF24OutShuttleLoseIcOpenIndexDoor)                         //kevin 20180725 add
            bIsTestSitICFallDown=true;

        #ifdef FOR_ASECL_L8             //JerryYang 20250120 : add
        if(CUSTOMER_CODE==CC_KYEC_LEE)  //Ifor 20170803 (wei) add 京元強制按RESET
        #else
        if(CUSTOMER_CODE==CC_KYEC_LEE || IniConfig.bF36OutShtLoseICResetSetAllToErr)  //Ifor 20170803 (wei) add 京元強制按RESET
        #endif
        {
            if(iShuttleNo==0)
                ret=ShowErrorMessage("JAM0508", K_RESET, MOutShuttle1, false, SThreadPara.ErrPartSh1);
            else
                ret=ShowErrorMessage("JAM0509", K_RESET, MOutShuttle2, false, SThreadPara.ErrPartSh2);
        }
        else
        {
            if(iShuttleNo==0)
                ret=ShowErrorMessage("JAM0508", K_SKIP, MOutShuttle1, false, SThreadPara.ErrPartSh1);
            else
                ret=ShowErrorMessage("JAM0509", K_SKIP, MOutShuttle2, false, SThreadPara.ErrPartSh2);
        }

        if(ret==K_SKIP || ret==K_RESET)                                         //Steven 20111222 : 改成可以RETRY
        {
            for(int i=0; i<FLCarryKit.iShtRow; i++)
            {
                for(int j=0; j<FLCarryKit.iShtCol; j++)
                {
                    if(iShuttleNo==0)                                           //Steven 20200701 : 重新整理Out shuttle alarm
                    {
                        if(SThreadPara.iScanSensor==1 &&
                           SThreadPara.bOutYUseLatch)
                        {
                            if(IsNNMode()==NN_2Row ||
                               ((TestIF_File.iTestMode==_6Site2X3 ||
                                 TestIF_File.iTestMode==_10Site2X5) &&          //Steven 20240428 : Site Col為奇數的話, 可以跑Y-Latch
                                 Use_AxisY_Sensor_2x3mode==true))
                            {
                                if(((FRCarryKit.Item[0][j]!=NULL_IC && FRCarryKit.Item[0][j]!=HAS_NULL_IC) ||
                                    (FRCarryKit.Item[1][j]!=NULL_IC && FRCarryKit.Item[1][j]!=HAS_NULL_IC)) &&
                                   SThreadPara.bSh1HasIC[0][j]==false)
                                {
                                    FRCarryKit.PordRec[0][j].AddErrorRecord("JAM0508");     //Steven 20161215 : Add Production Error Log
                                    FRCarryKit.PordRec[1][j].AddErrorRecord("JAM0508");     //Steven 20161215 : Add Production Error Log
                                    if(IniConfig.bF27OutShtLoseICNeedToPick==false)         //Steven 20220120 : 吳如春要求out shuttle lose IC時, out arm還是要下去吸料
                                    {
                                        if(CosFunction.bUseSCKART)                          //Steven 20161214 (wei) : For SCK ART
                                        {
                                            if(FRCarryKit.Item[0][j]!=NULL_IC && FRCarryKit.Item[0][j]!=HAS_NULL_IC)
                                                fSCKART->AddOutputJamCnt(0, j, ret);        //RogerYang 20250923 : 整合ART OutArm JamCount
                                            if(FRCarryKit.Item[1][j]!=NULL_IC && FRCarryKit.Item[1][j]!=HAS_NULL_IC)
                                                fSCKART->AddOutputJamCnt(1, j, ret);        //RogerYang 20250923 : 整合ART OutArm JamCount
                                        }

                                        FRCarryKit.SetItemData(0, j, NULL_IC);
                                        FRCarryKit.SetItemData(1, j, NULL_IC);
                                    }

                                    if(TestIF_File.bOutShtLoseICSetErrUntilOneCycle==true)  //JerryYang 20170609 (wei) 記錄out shuttle lose IC的site  //JerryYang 20220923 : SPIL Jovie要求改為By工作檔
                                    {
                                        bTestSiteNeedSetErrBin[2][j]=true;
                                        bTestSiteNeedSetErrBin[3][j]=true;
                                    }
                                    bShuttleLoseIC[0]=true;                                 //Frank 20171213 (Steven) : 若Shuttle Lose IC 將IC 放Fail Bin
                                }
                            }
                            else if(FRCarryKit.Item[i][j]!=NULL_IC &&
                                    FRCarryKit.Item[i][j]!=HAS_NULL_IC &&                   //JerryYang 20210302 : 修正out shuttle alarm會把shuttle ic清空
                                    SThreadPara.bSh1HasIC[i][j]==false)
                            {
                                FRCarryKit.PordRec[i][j].AddErrorRecord("JAM0508");         //Steven 20161215 : Add Production Error Log
                                if(IniConfig.bF27OutShtLoseICNeedToPick==false)             //Steven 20220120 : 吳如春要求out shuttle lose IC時, out arm還是要下去吸料
                                {
                                    if(CosFunction.bUseSCKART)                              //Steven 20161214 (wei) : For SCK ART
                                        fSCKART->AddOutputJamCnt(i, j, ret);                //RogerYang 20250923 : 整合ART OutArm JamCount
                                    FRCarryKit.SetItemData(i, j, NULL_IC);
                                }

                                if(TestIF_File.bOutShtLoseICSetErrUntilOneCycle==true)      //JerryYang 20170609 (wei) 記錄out shuttle lose IC的site  //JerryYang 20220923 : SPIL Jovie要求改為By工作檔
                                    bTestSiteNeedSetErrBin[i+2][j]=true;
                                bShuttleLoseIC[0]=true;                                     //Frank 20171213 (Steven) : 若Shuttle Lose IC 將IC 放Fail Bin
                            }
                        }
                        else if(FRCarryKit.Item[i][j]!=NULL_IC && FRCarryKit.Item[i][j]!=HAS_NULL_IC &&
                                SThreadPara.bSh1HasIC[i][j]==false)
                        {
                            if(TestIF_File.b16Direct8Shuttle &&                             //Steven 20210322 : 16 Site Direct Heater use 8 Site shuttle
                               TestIF.iTestMode==_16Site2X8 &&
                               bTestSiteUse[0][0][0]==false && bTestSiteUse[0][1][0]==false && bTestSiteUse[0][0][1]==false && bTestSiteUse[0][1][1]==false &&
                               bTestSiteUse[0][0][6]==false && bTestSiteUse[0][1][6]==false && bTestSiteUse[0][0][7]==false && bTestSiteUse[0][1][7]==false &&
                               TestIF_File.iSiteMap[0][0]==0 && TestIF_File.iSiteMap[1][0]==0 && TestIF_File.iSiteMap[0][1]==0 && TestIF_File.iSiteMap[1][1]==0 &&
                               TestIF_File.iSiteMap[0][6]==0 && TestIF_File.iSiteMap[1][6]==0 && TestIF_File.iSiteMap[0][7]==0 && TestIF_File.iSiteMap[1][7]==0)
                            {
                                if(j!=0 && j!=1 && j!=6 && j!=7)
                                {
                                    FRCarryKit.PordRec[i][j].AddErrorRecord("JAM0508");
                                    if(IniConfig.bF27OutShtLoseICNeedToPick==false)         //Steven 20220120 : 吳如春要求out shuttle lose IC時, out arm還是要下去吸料
                                    {
                                        if(CosFunction.bUseSCKART)
                                            fSCKART->AddOutputJamCnt(i, j, ret);            //RogerYang 20250923 : 整合ART OutArm JamCount
                                        FRCarryKit.SetItemData(i, j, NULL_IC);
                                    }

                                    if(TestIF_File.bOutShtLoseICSetErrUntilOneCycle==true)  //JerryYang 20220923 : SPIL Jovie要求改為By工作檔
                                        bTestSiteNeedSetErrBin[i+2][j]=true;
                                    bShuttleLoseIC[0]=true;
                                }
                            }
                            else if((CUSTOMER_CODE==CC_KYEC_JCTHIU ||
                                     CUSTOMER_CODE==CC_KYEC_LEE ||
                                     IniConfig.bSPILFunction ||                             //JerryYang 20180209 (Steven) KYEC MTK 12 site的kit 選16 site模式中間12site
                                     TestIF_File.b16Direct12Shuttle) &&                     //Steven 20210112 : Add function
                                     TestIF.iTestMode==_16Site2X8 &&
                                     bTestSiteUse[0][0][0]==false && bTestSiteUse[0][1][0]==false && bTestSiteUse[0][0][7]==false && bTestSiteUse[0][1][7]==false &&
                                     TestIF_File.iSiteMap[0][0]==0 && TestIF_File.iSiteMap[1][0]==0 && TestIF_File.iSiteMap[0][7]==0 && TestIF_File.iSiteMap[1][7]==0)
                            {
                                if(j!=0 && j!=7)
                                {
                                    FRCarryKit.PordRec[i][j].AddErrorRecord("JAM0508");     //Steven 20161215 : Add Production Error Log
                                    if(IniConfig.bF27OutShtLoseICNeedToPick==false)         //Steven 20220120 : 吳如春要求out shuttle lose IC時, out arm還是要下去吸料
                                    {
                                        if(CosFunction.bUseSCKART)                          //Steven 20161214 (wei) : For SCK ART
                                            fSCKART->AddOutputJamCnt(i, j, ret);            //RogerYang 20250923 : 整合ART OutArm JamCount
                                        FRCarryKit.SetItemData(i, j, NULL_IC);
                                    }

                                    if(TestIF_File.bOutShtLoseICSetErrUntilOneCycle==true)  //JerryYang 20170609 (wei) 記錄out shuttle lose IC的site  //JerryYang 20220923 : SPIL Jovie要求改為By工作檔
                                        bTestSiteNeedSetErrBin[i+2][j]=true;
                                    bShuttleLoseIC[0]=true;                                 //Frank 20171213 (Steven) : 若Shuttle Lose IC 將IC 放Fail Bin
                                }
                            }
                            else if(TestIF.iTestMode==_16Site2X8 &&
                                    iCloseSiteModeFor2x8==e2x8_STMMode)                     //JerryYang 20190729 STM 8 site交錯模式
                            {
                                if((i==0 && j==0) || (i==0 && j==2) || (i==0 && j==4) || (i==0 && j==6) || (i==1 && j==1) || (i==1 && j==3) || (i==1 && j==5) || (i==1 && j==7))
                                {
                                }
                                else
                                {
                                    FRCarryKit.PordRec[i][j].AddErrorRecord("JAM0508");     //Steven 20161215 : Add Production Error Log
                                    if(IniConfig.bF27OutShtLoseICNeedToPick==false)         //Steven 20220120 : 吳如春要求out shuttle lose IC時, out arm還是要下去吸料
                                    {
                                        if(CosFunction.bUseSCKART)                          //Steven 20161214 (wei) : For SCK ART
                                            fSCKART->AddOutputJamCnt(i, j, ret);            //RogerYang 20250923 : 整合ART OutArm JamCount
                                        FRCarryKit.SetItemData(i, j, NULL_IC);
                                    }

                                    if(TestIF_File.bOutShtLoseICSetErrUntilOneCycle==true)  //JerryYang 20170609 (wei) 記錄out shuttle lose IC的site  //JerryYang 20220923 : SPIL Jovie要求改為By工作檔
                                        bTestSiteNeedSetErrBin[i][j]=true;
                                    bShuttleLoseIC[0]=true;                                 //Frank 20171213 (Steven) : 若Shuttle Lose IC 將IC 放Fail Bin
                                }
                            }
                            else if(TestIF.iTestMode==_16Site2X8 &&
                                    iCloseSiteModeFor2x8==e2x8_TW153Mode)                   //Steven 20220816 : Add for TW153TK spec
                            {
                                if((i==0 && j==1) || (i==0 && j==3) || (i==0 && j==5) || (i==0 && j==7) || (i==1 && j==0) || (i==1 && j==2) || (i==1 && j==4) || (i==1 && j==6))
                                {
                                }
                                else
                                {
                                    FRCarryKit.PordRec[i][j].AddErrorRecord("JAM0508");     //Steven 20161215 : Add Production Error Log
                                    if(IniConfig.bF27OutShtLoseICNeedToPick==false)         //Steven 20220120 : 吳如春要求out shuttle lose IC時, out arm還是要下去吸料
                                    {
                                        if(CosFunction.bUseSCKART)                          //Steven 20161214 (wei) : For SCK ART
                                            fSCKART->AddOutputJamCnt(i, j, ret);            //RogerYang 20250923 : 整合ART OutArm JamCount
                                        FRCarryKit.SetItemData(i, j, NULL_IC);
                                    }

                                    if(TestIF_File.bOutShtLoseICSetErrUntilOneCycle==true)  //JerryYang 20170609 (wei) 記錄out shuttle lose IC的site  //JerryYang 20220923 : SPIL Jovie要求改為By工作檔
                                        bTestSiteNeedSetErrBin[i][j]=true;
                                    bShuttleLoseIC[0]=true;                                 //Frank 20171213 (Steven) : 若Shuttle Lose IC 將IC 放Fail Bin
                                }
                            }
                            else
                            {
                                FRCarryKit.PordRec[i][j].AddErrorRecord("JAM0508");         //Steven 20161215 : Add Production Error Log
                                if(IniConfig.bF27OutShtLoseICNeedToPick==false)             //Steven 20220120 : 吳如春要求out shuttle lose IC時, out arm還是要下去吸料
                                {
                                    if(CosFunction.bUseSCKART)                              //Steven 20161214 (wei) : For SCK ART
                                        fSCKART->AddOutputJamCnt(i, j, ret);                //RogerYang 20250923 : 整合ART OutArm JamCount
                                    FRCarryKit.SetItemData(i, j, NULL_IC);
                                }

                                if(TestIF_File.bOutShtLoseICSetErrUntilOneCycle==true)      //JerryYang 20170609 (wei) 記錄out shuttle lose IC的site  //JerryYang 20220923 : SPIL Jovie要求改為By工作檔
                                    bTestSiteNeedSetErrBin[i+2][j]=true;
                                bShuttleLoseIC[0]=true;                                     //Frank 20171213 (Steven) : 若Shuttle Lose IC 將IC 放Fail Bin
                            }
                        }
                    }
                    else
                    {
                        if(SThreadPara.iScanSensor==1 && SThreadPara.bOutYUseLatch)
                        {
                            if(IsNNMode()==NN_2Row ||
                               ((TestIF_File.iTestMode==_6Site2X3 ||
                                 TestIF_File.iTestMode==_10Site2X5) &&                      //Steven 20240428 : Site Col為奇數的話, 可以跑Y-Latch
                                 Use_AxisY_Sensor_2x3mode==true))
                            {
                                if(((BRCarryKit.Item[0][j]!=NULL_IC && BRCarryKit.Item[0][j]!=HAS_NULL_IC) ||
                                    (BRCarryKit.Item[1][j]!=NULL_IC && BRCarryKit.Item[1][j]!=HAS_NULL_IC)) &&
                                   SThreadPara.bSh2HasIC[0][j]==false)
                                {
                                    BRCarryKit.PordRec[0][j].AddErrorRecord("JAM0509");     //Steven 20161215 : Add Production Error Log
                                    BRCarryKit.PordRec[1][j].AddErrorRecord("JAM0509");     //Steven 20161215 : Add Production Error Log
                                    if(IniConfig.bF27OutShtLoseICNeedToPick==false)         //Steven 20220120 : 吳如春要求out shuttle lose IC時, out arm還是要下去吸料
                                    {
                                        if(CosFunction.bUseSCKART)                          //Steven 20161214 (wei) : For SCK ART
                                        {
                                            if(BRCarryKit.Item[0][j]!=NULL_IC && BRCarryKit.Item[0][j]!=HAS_NULL_IC)
                                                fSCKART->AddOutputJamCnt(0, j, ret);        //RogerYang 20250923 : 整合ART OutArm JamCount
                                            if(BRCarryKit.Item[1][j]!=NULL_IC && BRCarryKit.Item[1][j]!=HAS_NULL_IC)
                                                fSCKART->AddOutputJamCnt(1, j, ret);        //RogerYang 20250923 : 整合ART OutArm JamCount
                                        }

                                        BRCarryKit.SetItemData(0, j, NULL_IC);
                                        BRCarryKit.SetItemData(1, j, NULL_IC);
                                    }

                                    if(TestIF_File.bOutShtLoseICSetErrUntilOneCycle==true)  //JerryYang 20170609 (wei) 記錄out shuttle lose IC的site
                                    {
                                        bTestSiteNeedSetErrBin[0][j]=true;
                                        bTestSiteNeedSetErrBin[1][j]=true;
                                    }
                                    bShuttleLoseIC[1]=true;                                 //Frank 20171213 (Steven) : 若Shuttle Lose IC 將IC 放Fail Bin
                                }
                            }
                            else if(BRCarryKit.Item[i][j]!=NULL_IC &&
                                    BRCarryKit.Item[i][j]!=HAS_NULL_IC &&                   //JerryYang 20210302 : 修正out shuttle alarm會把shuttle ic清空
                                    SThreadPara.bSh2HasIC[i][j]==false)
                            {
                                BRCarryKit.PordRec[i][j].AddErrorRecord("JAM0509");         //Steven 20161215 : Add Production Error Log
                                if(IniConfig.bF27OutShtLoseICNeedToPick==false)             //Steven 20220120 : 吳如春要求out shuttle lose IC時, out arm還是要下去吸料
                                {
                                    if(CosFunction.bUseSCKART)                              //Steven 20161214 (wei) : For SCK ART
                                        fSCKART->AddOutputJamCnt(i, j, ret);                //RogerYang 20250923 : 整合ART OutArm JamCount
                                    BRCarryKit.SetItemData(i, j, NULL_IC);
                                }

                                if(TestIF_File.bOutShtLoseICSetErrUntilOneCycle==true)      //JerryYang 20170609 (wei) 記錄out shuttle lose IC的site  //JerryYang 20220923 : SPIL Jovie要求改為By工作檔
                                    bTestSiteNeedSetErrBin[i][j]=true;
                                bShuttleLoseIC[1]=true;                                     //Frank 20171213 (Steven) : 若Shuttle Lose IC 將IC 放Fail Bin
                            }
                        }
                        else if(BRCarryKit.Item[i][j]!=NULL_IC && BRCarryKit.Item[i][j]!=HAS_NULL_IC &&
                                SThreadPara.bSh2HasIC[i][j]==false)
                        {
                            if(TestIF_File.b16Direct8Shuttle &&                             //Steven 20210322 : 16 Site Direct Heater use 8 Site shuttle
                               TestIF.iTestMode==_16Site2X8 &&
                               bTestSiteUse[0][0][0]==false && bTestSiteUse[0][1][0]==false && bTestSiteUse[0][0][1]==false && bTestSiteUse[0][1][1]==false &&
                               bTestSiteUse[0][0][6]==false && bTestSiteUse[0][1][6]==false && bTestSiteUse[0][0][7]==false && bTestSiteUse[0][1][7]==false &&
                               TestIF_File.iSiteMap[0][0]==0 && TestIF_File.iSiteMap[1][0]==0 && TestIF_File.iSiteMap[0][1]==0 && TestIF_File.iSiteMap[1][1]==0 &&
                               TestIF_File.iSiteMap[0][6]==0 && TestIF_File.iSiteMap[1][6]==0 && TestIF_File.iSiteMap[0][7]==0 && TestIF_File.iSiteMap[1][7]==0)
                            {
                                if(j!=0 && j!=1 && j!=6 && j!=7)
                                {
                                    BRCarryKit.PordRec[i][j].AddErrorRecord("JAM0509");
                                    if(IniConfig.bF27OutShtLoseICNeedToPick==false)         //Steven 20220120 : 吳如春要求out shuttle lose IC時, out arm還是要下去吸料
                                    {
                                        if(CosFunction.bUseSCKART)
                                            fSCKART->AddOutputJamCnt(i, j, ret);            //RogerYang 20250923 : 整合ART OutArm JamCount
                                        BRCarryKit.SetItemData(i, j, NULL_IC);
                                    }

                                    if(TestIF_File.bOutShtLoseICSetErrUntilOneCycle==true)  //JerryYang 20220923 : SPIL Jovie要求改為By工作檔
                                        bTestSiteNeedSetErrBin[i][j]=true;
                                    bShuttleLoseIC[1]=true;
                                }
                            }
                            else if((CUSTOMER_CODE==CC_KYEC_JCTHIU ||
                                     CUSTOMER_CODE==CC_KYEC_LEE ||
                                     IniConfig.bSPILFunction ||                             //JerryYang 20180209 (Steven) KYEC MTK 12 site的kit 選16 site模式中間12site
                                     TestIF_File.b16Direct12Shuttle) &&                     //Steven 20210112 : Add function
                                     TestIF.iTestMode==_16Site2X8 &&
                                     bTestSiteUse[0][0][0]==false && bTestSiteUse[0][1][0]==false && bTestSiteUse[0][0][7]==false && bTestSiteUse[0][1][7]==false &&
                                     TestIF_File.iSiteMap[0][0]==0 && TestIF_File.iSiteMap[1][0]==0 && TestIF_File.iSiteMap[0][7]==0 && TestIF_File.iSiteMap[1][7]==0)
                            {
                                if(j!=0 && j!=7)
                                {
                                    BRCarryKit.PordRec[i][j].AddErrorRecord("JAM0509");     //Steven 20161215 : Add Production Error Log
                                    if(IniConfig.bF27OutShtLoseICNeedToPick==false)         //Steven 20220120 : 吳如春要求out shuttle lose IC時, out arm還是要下去吸料
                                    {
                                        if(CosFunction.bUseSCKART)                          //Steven 20161214 (wei) : For SCK ART
                                            fSCKART->AddOutputJamCnt(i, j, ret);            //RogerYang 20250923 : 整合ART OutArm JamCount
                                        BRCarryKit.SetItemData(i, j, NULL_IC);
                                    }

                                    if(TestIF_File.bOutShtLoseICSetErrUntilOneCycle==true)  //JerryYang 20170609 (wei) 記錄out shuttle lose IC的site  //JerryYang 20220923 : SPIL Jovie要求改為By工作檔
                                        bTestSiteNeedSetErrBin[i][j]=true;
                                    bShuttleLoseIC[1]=true;                                 //Frank 20171213 (Steven) : 若Shuttle Lose IC 將IC 放Fail Bin
                                }
                            }
                            else if(TestIF.iTestMode==_16Site2X8 &&
                                    iCloseSiteModeFor2x8==e2x8_STMMode)                     //JerryYang 20190729 STM 8 site交錯模式
                            {
                                if((i==0 && j==0) || (i==0 && j==2) || (i==0 && j==4) || (i==0 && j==6) || (i==1 && j==1) || (i==1 && j==3) || (i==1 && j==5) || (i==1 && j==7))
                                {
                                }
                                else
                                {
                                    BRCarryKit.PordRec[i][j].AddErrorRecord("JAM0509");     //Steven 20161215 : Add Production Error Log
                                    if(IniConfig.bF27OutShtLoseICNeedToPick==false)         //Steven 20220120 : 吳如春要求out shuttle lose IC時, out arm還是要下去吸料
                                    {
                                        if(CosFunction.bUseSCKART)                          //Steven 20161214 (wei) : For SCK ART
                                            fSCKART->AddOutputJamCnt(i, j, ret);            //RogerYang 20250923 : 整合ART OutArm JamCount
                                        BRCarryKit.SetItemData(i, j, NULL_IC);
                                    }

                                    if(TestIF_File.bOutShtLoseICSetErrUntilOneCycle==true)  //JerryYang 20170609 (wei) 記錄out shuttle lose IC的site //JerryYang 20220923 : SPIL Jovie要求改為By工作檔
                                        bTestSiteNeedSetErrBin[i][j]=true;
                                    bShuttleLoseIC[1]=true;                                 //Frank 20171213 (Steven) : 若Shuttle Lose IC 將IC 放Fail Bin
                                }
                            }
                            else if(TestIF.iTestMode==_16Site2X8 &&
                                    iCloseSiteModeFor2x8==e2x8_TW153Mode)                   //Steven 20220816 : Add for TW153TK spec
                            {
                                if((i==0 && j==1) || (i==0 && j==3) || (i==0 && j==5) || (i==0 && j==7) || (i==1 && j==0) || (i==1 && j==2) || (i==1 && j==4) || (i==1 && j==6))
                                {
                                }
                                else
                                {
                                    BRCarryKit.PordRec[i][j].AddErrorRecord("JAM0509");     //Steven 20161215 : Add Production Error Log
                                    if(IniConfig.bF27OutShtLoseICNeedToPick==false)         //Steven 20220120 : 吳如春要求out shuttle lose IC時, out arm還是要下去吸料
                                    {
                                        if(CosFunction.bUseSCKART)                          //Steven 20161214 (wei) : For SCK ART
                                            fSCKART->AddOutputJamCnt(i, j, ret);            //RogerYang 20250923 : 整合ART OutArm JamCount
                                        BRCarryKit.SetItemData(i, j, NULL_IC);
                                    }

                                    if(TestIF_File.bOutShtLoseICSetErrUntilOneCycle==true)  //JerryYang 20170609 (wei) 記錄out shuttle lose IC的site  //JerryYang 20220923 : SPIL Jovie要求改為By工作檔
                                        bTestSiteNeedSetErrBin[i][j]=true;
                                    bShuttleLoseIC[1]=true;
                                }
                            }
                            else
                            {
                                BRCarryKit.PordRec[i][j].AddErrorRecord("JAM0509");         //Steven 20161215 : Add Production Error Log
                                if(IniConfig.bF27OutShtLoseICNeedToPick==false)             //Steven 20220120 : 吳如春要求out shuttle lose IC時, out arm還是要下去吸料
                                {
                                    if(CosFunction.bUseSCKART)                              //Steven 20161214 (wei) : For SCK ART
                                        fSCKART->AddOutputJamCnt(i, j, ret);                //RogerYang 20250923 : 整合ART OutArm JamCount
                                    BRCarryKit.SetItemData(i, j, NULL_IC);
                                }

                                if(TestIF_File.bOutShtLoseICSetErrUntilOneCycle==true)      //JerryYang 20170609 (wei) 記錄out shuttle lose IC的site  //JerryYang 20220923 : SPIL Jovie要求改為By工作檔
                                    bTestSiteNeedSetErrBin[i][j]=true;
                                bShuttleLoseIC[1]=true;                                     //Frank 20171213 (Steven) : 若Shuttle Lose IC 將IC 放Fail Bin
                            }
                        }
                    }
                }
            }

            #ifdef FOR_ASECL_L8             //JerryYang 20250120 : add
            if(CUSTOMER_CODE==CC_KYEC_LEE)  //Ifor 20170803 (wei) add 京元強制按RESET
            #else
            if(CUSTOMER_CODE==CC_KYEC_LEE || IniConfig.bF36OutShtLoseICResetSetAllToErr)  //Ifor 20170803 (wei) add 京元強制按RESET
            #endif
            {
                bOutShuttleMissIC=true;
                fMain->BtnResetClick(fMain);
            }
            else if(CUSTOMER_CODE==CC_SCS)                                      //jou 2015-08-21 After conditional error, all units will go to Error bin ,follow by Index check or piggy back check
            {
                if(IniConfig.bF19OutShuttleLoseICNeedPiggyback==true)
                {
                    fMain->ResetRecordforPiggyBack("RESET_ForShuttleLoseIC");
                }
            }
            else if(CUSTOMER_CODE==CC_AMKOR_Philippines ||                      //Frank 20171213 (Steven) : 若Shuttle Lose IC 將IC 放Fail Bin
                    IniConfig.bA35SetErrBinWhenOutShtLoseAndPickupErr)          //JerryYang 20220215 : 松諭要求的功能Out shuttle lose IC 以及 out arm pick up error set to error bin
            {
                if(bShuttleLoseIC[0]==true)
                {
                    FRCarryKit.SetAllRealIC2InterfaceBin();
                    iCT=FRCarryKit.CountRealIC();
                    sBuffer.printf("Shuttle1 Lose IC Place to Error bin : Device=%d;", iCT);
                    MyDBIProcess("Message", sBuffer);
                }
                else if(bShuttleLoseIC[1]==true)
                {
                    BRCarryKit.SetAllRealIC2InterfaceBin();
                    iCT=BRCarryKit.CountRealIC();                               //JerryYang 20190226 : FR -> BR
                    sBuffer.printf("Shuttle2 Lose IC Place to Error bin : Device=%d;", iCT);
                    MyDBIProcess("Message", sBuffer);
                }

                if(CUSTOMER_CODE==CC_AMKOR_Philippines)
                {
                    fAllMotorHome=false;
                }
            }
            else if(TestIF_File.bOutShtLoseICSetErrUntilOneCycle==true)         //JerryYang 20170610 (wei) JSCC要求Out shuttle lose IC需自動one cycle,並將對應的site設為Error bin  //JerryYang 20220923 : SPIL Jovie要求改為By工作檔
            {
                fMain->BtnOneCycleClick(fMain);
                bOutShtLoseICNeedSetErrBin=true;
                fMain->JSCC_ResetForShuttleLoseIC();
            }

            bOneCycleAfterSHLossIC=false;
            if(CUSTOMER_CODE==CC_SIGURD_HUKOU &&                                //KaiChen 20200304 ：矽格-湖口，要求OutShuttle Loss IC 時機台上的IC放到R道
               IniConfig.bA33SetICToErrBinAfterOutShtLossIC)
            {
                int iCT, iOutArmPosX, iOutArmPosY;
                AnsiString sBuffer;

                InitInArmTask();
                bOneCycleAfterSHLossIC=true;
                fMain->BtnOneCycleClick(fMain);

                iCT=FRCarryKit.CountRealIC();
                if(iCT>0)
                {
                    sBuffer.printf("Out Shuttle Loss IC, Out Shuttle1 Place to Error bin : Device=%d;",iCT);
                    MyDBIProcess("Message", sBuffer);
                }
                FRCarryKit.SetAllRealIC2InterfaceBin();

                iCT=FLCarryKit.CountRealIC();
                if(iCT>0)
                {
                    sBuffer.printf("Out Shuttle Loss IC, In Shuttle1 Place to Error bin : Device=%d;",iCT);
                    MyDBIProcess("Message", sBuffer);
                }
                FLCarryKit.SetAllRealIC2InterfaceBin();

                iCT=BRCarryKit.CountRealIC();
                if(iCT>0)
                {
                    sBuffer.printf("Out Shuttle Loss IC, Out Shuttle2 Place to Error bin : Device=%d;",iCT);
                    MyDBIProcess("Message", sBuffer);
                }
                BRCarryKit.SetAllRealIC2InterfaceBin();

                iCT=BLCarryKit.CountRealIC();
                if(iCT>0)
                {
                    sBuffer.printf("Out Shuttle Loss IC, In Shuttle2 Place to Error bin : Device=%d;",iCT);
                    MyDBIProcess("Message", sBuffer);
                }
                BLCarryKit.SetAllRealIC2InterfaceBin();

                iCT=FTestSuck.CountRealIC();
                if(iCT>0)
                {
                    sBuffer.printf("Out Shuttle Loss IC, Index Arm1 Place to Error bin : Device=%d;",iCT);
                    MyDBIProcess("Message", sBuffer);
                }
                FTestSuck.SetAllRealIC2InterfaceBin();
                iCT=BTestSuck.CountRealIC();
                if(iCT>0)
                {
                    sBuffer.printf("Out Shuttle Loss IC, Index Arm2 Place to Error bin : Device=%d;",iCT);
                    MyDBIProcess("Message", sBuffer);
                }
                BTestSuck.SetAllRealIC2InterfaceBin();
                TestSocket.SetAllRealIC2InterfaceBin();

                iCT=InArmSuck.CountRealIC();
                if(iCT>0)
                {
                    sBuffer.printf("Out Shuttle Loss IC, Input Arm Place to Error bin : Device=%d;",iCT);
                    MyDBIProcess("Message", sBuffer);

                    for(int i=0; i<InArmSuck.iMaxRow; i++)
                    {
                        for(int j=0; j<InArmSuck.iMaxCol; j++)
                        {
                            if(InArmSuck.Item[i][j]!=HAS_IC)
                            {
                                InArmSuck.SetItemData(i, j, HAS_NULL_IC);
//                                InArmSuck.Item[i][j]=HAS_NULL_IC;
                            }
                            else if(InArmSuck.Item[i][j]!=NULL_IC &&
                                    InArmSuck.Item[i][j]!=HAS_NULL_IC)
                            {
                                InArmSuck.iWhichAuto[i][j]=Prod.iIfErrorT6;
                                InArmSuck.Item[i][j]=TEST_PASS+iTestBinCount;
                                InArmSuck.iBinData[i][j]=iTestBinCount;
                                InArmSuck.bPass[i][j]=false;
                                InArmSuck.bNeedReTest[i][j]=false;
                            }
                        }
                    }
                }

                iOutArmPosX=MOT[MOutArmX].ReadEncoderPos();
                iOutArmPosY=MOT[MOutArmY].ReadEncoderPos();

                sBuffer.printf("Out Shuttle Loss IC, OutArm X Pos=%d, OutArm Y Pos=%d;",iOutArmPosX, iOutArmPosY);
                MyDBIProcess("Message", sBuffer);

                if(iOutArmPosX<-41580 && iOutArmPosY>-49000)
                {
                    iCT=OutArmSuck.CountRealIC();
                    if(iCT>0)
                    {
                        sBuffer.printf("Out Shuttle Loss IC, Output Arm Place to Error bin : Device=%d;",iCT);
                        MyDBIProcess("Message", sBuffer);
                    }

                    for(int i=0; i<OutArmSuck.iPickRow; i++)
                    {
                        for(int j=0; j<OutArmSuck.iPickCol; j++)
                        {
                            if(OutArmSuck.Item[i][j]!=NULL_IC &&
                               OutArmSuck.Item[i][j]!=HAS_NULL_IC)
                            {
                                OutArmSuck.iWhichAuto[i][j]=Prod.iIfErrorT6;
                                OutArmSuck.Item[i][j]=TEST_PASS+iTestBinCount;
                                OutArmSuck.iBinData[i][j]=iTestBinCount;
                                OutArmSuck.bPass[i][j]=false;
                                OutArmSuck.bNeedReTest[i][j]=false;
                            }
                        }
                    }
                }
            }
        }
    }

    if(bHasFloatingErr==true)                                                   //Steven 20180329 (Jou) : 使用Y-Latch偵測置偏
    {
        if(iShuttleNo==0)
            ret=ShowErrorMessage("JAM0501", K_RETRY, MOutShuttle1, false, SThreadPara.FloatingErrPartSh1);
        else
            ret=ShowErrorMessage("JAM0504", K_RETRY, MOutShuttle2, false, SThreadPara.FloatingErrPartSh2);
    }
    return bHasErr;
}
//------------------------------------------------------------------------------
bool ShowOutputShuttleICStatus(int iSelSHT, bool bAlarm)                        //檢查IC Lose
{
    static bool bShuttleLoseIC[2]={false, false};                               //Frank 20171213 (Steven) : 若Shuttle Lose IC 將IC 放Fail Bin
    int iCT=0;                                                                  //Frank 20171213 (Steven) : 若Shuttle Lose IC 將IC 放Fail Bin
    AnsiString sBuffer="";
    bool bHasErr=false;

    if(LastSet.iRealDummy!=REALLY &&
       fMain->cbTestOutShuttleSensor->Checked==false)                           //Steven 20150727 : 增加Out Shuttle Sensor
    {
        return false;
    }

    if((TestIF_File.iTestMode==_6Site2X3 ||
       TestIF_File.iTestMode==_10Site2X5) &&                                    //Steven 20240428 : Site Col為奇數的話, 可以跑Y-Latch
       Use_AxisY_Sensor_2x3mode==true)
    {
    }

    if(SThreadPara.bExeShuttleThread==false)                                    //Steven 20110701
    {
        if(SThreadPara.bUseM204Mode==true)
            bHasErr=ShowOutputShuttleICStatus_M204(iSelSHT, bAlarm);
        else
            return false;
    }
    else
    {
        if((TestIF.iTestMode<=QualSite1X4 ||
            IsNNMode()==NN_1Row) &&
           TestIF.bNS7000kit==false)
        {
            if(iSelSHT==0)
            {
                if(Sen[SnOutPutSHT1S1].Enable==false)
                {
                    return false;
                }
            }
            else
            {
                if(Sen[SnOutPutSHT2S1].Enable==false)
                {
                    return false;
                }
            }
        }
        else if(TestIF_File.iTestMode==SingleSite)                              //Steven 20160919 : fix for 1x1偏心要跑Y sensor, 因為舊機台沒打洞
        {
            if(iSelSHT==0)
            {
                if(Sen[SnOutPutSHT1S1].Enable==false)
                {
                    return false;
                }
            }
            else
            {
                if(Sen[SnOutPutSHT2S1].Enable==false)
                {
                    return false;
                }
            }
        }
        else if((TestIF_File.iTestMode==_6Site2X3 ||
                 TestIF_File.iTestMode==_10Site2X5) &&                          //Steven 20240428 : Site Col為奇數的話, 可以跑Y-Latch
                Use_AxisY_Sensor_2x3mode==true)
        {
            if(iSelSHT==0)
            {
                if(Sen[SnOutPutSHT1S1].Enable==false)
                {
                    return false;
                }
            }
            else
            {
                if(Sen[SnOutPutSHT2S1].Enable==false)
                {
                    return false;
                }
            }
        }
        else
        {
            if(iSelSHT==0)
            {
                if(Sen[SnOutPutSHT1ZS1].Enable==false ||
                   Sen[SnOutPutSHT1ZS2].Enable==false)
                {
                    return false;
                }
            }
            else
            {
                if(Sen[SnOutPutSHT2ZS1].Enable==false ||
                   Sen[SnOutPutSHT2ZS2].Enable==false)
                {
                    return false;
                }
            }
        }

        AnsiString ErrPart=" ";

        if(fMain->pgMain->ActivePageIndex==fMain->emp7TabSheet21)               //在MotionView才顯示
        {
            for(int i=0; i<BRCarryKit.iShtRow; i++)
            {
                for(int j=0; j<BRCarryKit.iShtCol; j++)
                {
                    if(bEnter[iSelSHT][i][j])
                    {
                        if(bShuttleHasIC[iSelSHT][i][j])
                        {
                            if(iSelSHT==0)
                                fMain->htShullte0->SetCellColorIndex(j, i, HAS_IC);
                            else
                                fMain->htShullte1->SetCellColorIndex(j, i, HAS_IC);
                        }
                    }
                }
            }
        }

        for(int i=0; i<BRCarryKit.iShtRow; i++)                                 //Steven 20110222 Start : 整合Alarm
        {
            for(int j=0; j<BRCarryKit.iShtCol; j++)
            {
                if(iSelSHT==0)
                {
                    if(FRCarryKit.Item[i][j]!=NULL_IC && FRCarryKit.Item[i][j]!=HAS_NULL_IC &&
                       bShuttleHasIC[iSelSHT][i][j]==false)
                    {
                        bHasErr=true;
                        if(bAlarm==true)
                        {
                            ErrPart+=IndexSuckName[i][j];
                            if(IniConfig.bF27OutShtLoseICNeedToPick==false)         //Steven 20220120 : 吳如春要求out shuttle lose IC時, out arm還是要下去吸料
                            {
                                FRCarryKit.SetItemData(i, j, NULL_IC);
                                if(CosFunction.bUseSCKART)                          //Steven 20181113 : For SCK ART
                                    fSCKART->AddOutputJamCnt(i, j, K_SKIP);         //RogerYang 20250923 : 整合ART OutArm JamCount                       //Steven 20200527 : 修正ART時, out shuttle lose ic不會計算
                            }
                            bShuttleLoseIC[0]=true;                                 //Frank 20171213 (Steven) : 若Shuttle Lose IC 將IC 放Fail Bin
                            if(TestIF_File.bOutShtLoseICSetErrUntilOneCycle==true)  //JerryYang 20170609 (wei) 記錄out shuttle lose IC的site  //JerryYang 20220923 : SPIL Jovie要求改為By工作檔
                                bTestSiteNeedSetErrBin[i][j]=true;
                        }
                    }
                }
                else
                {
                    if(BRCarryKit.Item[i][j]!=NULL_IC && BRCarryKit.Item[i][j]!=HAS_NULL_IC &&
                        bShuttleHasIC[iSelSHT][i][j]==false)
                    {
                        bHasErr=true;
                        if(bAlarm==true)
                        {
                            ErrPart+=IndexSuckName[i][j];
                            if(IniConfig.bF27OutShtLoseICNeedToPick==false)         //Steven 20220120 : 吳如春要求out shuttle lose IC時, out arm還是要下去吸料
                            {
                                BRCarryKit.SetItemData(i, j, NULL_IC);
                                if(CosFunction.bUseSCKART)                          //Steven 20181113 : For SCK ART
                                    fSCKART->AddOutputJamCnt(i, j, K_SKIP);         //RogerYang 20250923 : 整合ART OutArm JamCount                      //Steven 20200527 : 修正ART時, out shuttle lose ic不會計算
                            }
                            bShuttleLoseIC[1]=true;                                 //Frank 20171213 (Steven) : 若Shuttle Lose IC 將IC 放Fail Bin
                            if(TestIF_File.bOutShtLoseICSetErrUntilOneCycle==true)  //JerryYang 20170609 (wei) 記錄out shuttle lose IC的site  //JerryYang 20220923 : SPIL Jovie要求改為By工作檔
                                bTestSiteNeedSetErrBin[i][j]=true;
                        }
                    }
                }
            }
        }

        if(bHasErr==true)
        {
            if(bAlarm==true)
            {
                if(IniConfig.bOutShLoseNeedOpenChamber==true ||                 //jou 2013-12-12 Out Shuttle Lose Device Need Open Chamber Door and press Z1
                   IniConfig.bF24OutShuttleLoseIcOpenIndexDoor)                 //kevin 20180725 add
                    bIsTestSitICFallDown=true;

                #ifdef FOR_ASECL_L8             //JerryYang 20250120 : add
                if(CUSTOMER_CODE==CC_KYEC_LEE)  //Ifor 20170803 (wei) add 京元強制按RESET
                #else
                if(CUSTOMER_CODE==CC_KYEC_LEE || IniConfig.bF36OutShtLoseICResetSetAllToErr)  //Ifor 20170803 (wei) add 京元強制按RESET
                #endif
                {
                    if(iSelSHT==0)
                        ShowErrorMessage("JAM0508", K_RESET, MOutShuttle1, false, ErrPart);
                    else
                        ShowErrorMessage("JAM0509", K_RESET, MOutShuttle2, false, ErrPart);
                }
                else
                {
                    if(iSelSHT==0)
                        ShowErrorMessage("JAM0508", K_SKIP, MOutShuttle1, false, ErrPart);
                    else
                        ShowErrorMessage("JAM0509", K_SKIP, MOutShuttle2, false, ErrPart);
                }

                #ifdef FOR_ASECL_L8             //JerryYang 20250120 : add
                if(CUSTOMER_CODE==CC_KYEC_LEE)  //Ifor 20170803 (wei) add 京元強制按RESET
                #else
                if(CUSTOMER_CODE==CC_KYEC_LEE || IniConfig.bF36OutShtLoseICResetSetAllToErr)  //Ifor 20170803 (wei) add 京元強制按RESET
                #endif
                {
                    bOutShuttleMissIC=true;
                    fMain->BtnResetClick(fMain);
                }
                else if(CUSTOMER_CODE==CC_SCS)                                  //jou 2015-08-21 After conditional error, all units will go to Error bin ,follow by Index check or piggy back check
                {
                    if(IniConfig.bF19OutShuttleLoseICNeedPiggyback==true)
                    {
                        fMain->ResetRecordforPiggyBack("RESET_ForShuttleLoseIC");
                    }
                }
                else if(CUSTOMER_CODE==CC_AMKOR_Philippines ||                  //Frank 20171213 (Steven) : 若Shuttle Lose IC 將IC 放Fail Bin
                        IniConfig.bA35SetErrBinWhenOutShtLoseAndPickupErr)      //JerryYang 20220215 : 松諭要求的功能Out shuttle lose IC 以及 out arm pick up error set to error bin
                {
                    if(bShuttleLoseIC[0]==true)
                    {
                        FRCarryKit.SetAllRealIC2InterfaceBin();
                        bShuttleLoseIC[0]=false;
                        sBuffer.printf("Shuttle1 Lose IC Place to Error bin : Device=%d;",iCT);
                        MyDBIProcess("Message", sBuffer);
                    }
                    else if(bShuttleLoseIC[1]==true)
                    {
                        BRCarryKit.SetAllRealIC2InterfaceBin();
                        bShuttleLoseIC[1]=false;
                        sBuffer.printf("Shuttle2 Lose IC Place to Error bin : Device=%d;",iCT);
                        MyDBIProcess("Message", sBuffer);
                    }

                    if(CUSTOMER_CODE==CC_AMKOR_Philippines)
                    {
                        fAllMotorHome=false;
                    }
                }
                else if(TestIF_File.bOutShtLoseICSetErrUntilOneCycle==true)     //JerryYang 20170610 (wei) JSCC要求Out shuttle lose IC需自動one cycle,並將對應的site設為Error bin  //JerryYang 20220923 : SPIL Jovie要求改為By工作檔
                {
                    fMain->BtnOneCycleClick(fMain);
                    bOutShtLoseICNeedSetErrBin=true;
                    fMain->JSCC_ResetForShuttleLoseIC();
                }
            }
        }
    }
    return bHasErr;
}
//------------------------------------------------------------------------------
void Initial_Auto_SHT1_Task(){AutoSHT1Task=1;}
void Initial_Auto_SHT2_Task(){AutoSHT2Task=1;}
//------------------------------------------------------------------------------
void ShowIndexAndShuttlePositionNotCorrect(int &iRetryCT)
{
    if(IsIndexRunCycle())
    {
        iRetryCT++;
        if(iRetryCT>1000)
        {
            iRetryCT=0;
            ShowIndexMotorError(AnsiString("ShowIndexAndShuttlePositionNotCorrect"));
        }
    }
}
//------------------------------------------------------------------------------
void ShuttleBeforeMoveIndexZHomeNotOn(int &iRetryCT, int indexArm)
{
    iRetryCT++;
    if(iRetryCT>1000)
    {
        iRetryCT=0;
        if(indexArm==1)
        {
            ShowMyMessage("Z1 Home Sensor must on, Please check!!!");
        }
        else
        {
            ShowMyMessage("Z2 Home Sensor must on, Please check!!!");
        }
        fAllMotorHome=false;                                                    //Ifor 20160314 Home Sensor 異常需回Home
    }
}
//------------------------------------------------------------------------------
bool IsTestZ1NotSafeShuttle1CanNotMove(int &iRetryCT)
{
    if(MOT[MTestY1].Gali_ReadEncoderInRandge(Prod.TestY1_Front))
    {
        if(MOT[MTestZ1].Gali_ReadEncoderBelowCheckHeight(Prod.TestZ1_Safe))
        {
            MOT[MInShuttle1].PCIL132_StopMotor();
            if(MOT[MInShuttle1].IsCanMove())
                ShowIndexAndShuttlePositionNotCorrect(iRetryCT);
            return true;
        }

        if(TestIF_File.iShuttleMode==0 ||                                       //Ifor 20160310 add 蝦頭移動前判斷是否回Home
           (TestIF_File.iShuttleMode==1 && TestIF_File.iShuttle_Sel==0))
        {
            #ifndef SOFT_SIMULTE
            MOT[MTestZ1].ScanMotorStatus();
            if(MOT[MTestZ1].Led[iHomeLed]==false)
            {
                MOT[MInShuttle1].PCIL132_StopMotor();
                if(MOT[MInShuttle1].IsCanMove())
                    ShuttleBeforeMoveIndexZHomeNotOn(iRetryCT, 1);
                return true;
            }
            #endif
        }
    }
    return false;
}
//------------------------------------------------------------------------------
bool InArmFullPickforTestMode()
{
    bool bRe=false;

    if(TestIF.iTestMode!=_16Site2X8 &&
       TestIF.iTestMode!=_12Site2X6 &&
       TestIF.iTestMode!=_32Site4X8N)
        bRe=true;
    else if(iCloseSiteModeFor2x6>e2x6OneByOne ||
            iCloseSiteModeFor2x8>e2x8OneByOne)
        bRe=true;
    else
        bRe=false;
    return bRe;
}
//------------------------------------------------------------------------------
extern bool CheckShuttleSensor_9045(int CheckItem);                             //Steven 20150415 : 加上CheckItem,可以判別哪邊不用做檢查
extern bool MoveInArmZToPlateSafe(int iTask);                                   //kevin 20161005 add
//------------------------------------------------------------------------------
extern int SendHTTPRequest(int iSht);
//------------------------------------------------------------------------------
//AI(ht9045-staterecord-analysis) 20260430 (RogerYang) : Shuttle Move Timeout 集中處理
//   觸發條件：case 120 MotorMove(iLeft) 連續 return 0 > SHUTTLE_MOVE_TIMEOUT_SEC
//   動作：(1) Latch buffer dump 至 EventLog 給 RD 追根因
//         (2) ShowMyMessage 阻塞提示操作員 (不計 JAM/WAR KPI)
//         (3) 操作員確認後 → 清 latch / 重置旗標 / 觸發整機 Home
//         (4) Home 完成後 AutoSHTxTask=1, 操作員按 START 即可繼續
void DoShtMoveTimeoutHandle(int iSelSHT)
{
    int    iMot     = (iSelSHT==0) ? MInShuttle1 : MInShuttle2;                 //達索引
    int    iEncoder = MOT[iMot].ReadPos();                                      //結時 Encoder
    int    iTarget  = Prod.InSHT[iSelSHT].iLeft;                                //標位置
    AnsiString sMsg, sLog;

    //AI(ht9045-staterecord-analysis) 20260430 (RogerYang) : Latch buffer dump (僅 timeout 觸發, 頻率極低)
    sLog.sprintf("Shuttle%d Move Timeout, Enc=%d Tar=%d, RetryCnt=%d, LtcCnt0=%d LtcCnt1=%d LtcCnt5=%d",
                 iSelSHT+1, iEncoder, iTarget,
                 (iSelSHT==0) ? iOutShuttle1HasICErrRetryCnt : iOutShuttle2HasICErrRetryCnt,
                 fLtcSensor->LatchDataCnt0,
                 fLtcSensor->LatchDataCnt1,
                 fLtcSensor->LatchDataCnt5);
    MyDBIProcess("Message", sLog);                                              //A寫入 EventLog

    //AI(ht9045-staterecord-analysis) 20260525 (RogerYang) : 操作員提示訊息 (簡化英文)
    sMsg.sprintf("Shuttle %d Move Timeout (>%d sec)\n"
                 "Encoder=%d  Target=%d\n\n"
                 "Press OK to Home, then START to resume.",
                 iSelSHT+1, (int)(SHUTTLE_MOVE_TIMEOUT_SEC),
                 iEncoder, iTarget);

    if(IniConfig.bAlarmNeedServoOff)
        ShowMyMessage(sMsg, sMsg, "", false, true);                             //Servo Off + 阻塞
    else
        ShowMyMessage(sMsg, sMsg);                                              //純阻塞訊息

    //AI(ht9045-staterecord-analysis) 20260430 (RogerYang) : 操作員確認後 → 清狀態 + 觸發 Home
    fLtcSensor->ClearLtcSensor(iSelSHT);                                        //清 Latch buffer
    if(iSelSHT==0)
    {
        b1ShuttleMoveToLeft         = false;                                    //清方向旗標
        b1ShuttleMoveToRight        = false;
        iOutShuttle1HasICErrRetryCnt= 0;                                        //清殘料 retry 計數
        AutoSHT1Task                = 1;                                        //回起點
        bShtMoveTimerRunning1       = false;                                    //Steven 20260507 : clear watchdog timer flag
    }
    else
    {
        b2ShuttleMoveToLeft         = false;
        b2ShuttleMoveToRight        = false;
        iOutShuttle2HasICErrRetryCnt= 0;
        AutoSHT2Task                = 1;
        bShtMoveTimerRunning2       = false;                                    //Steven 20260507 : clear watchdog timer flag
    }
    SetMotorScaleSpeed(iMot, (iSelSHT==0) ? SHSpeed.iSH1Sp : SHSpeed.iSH2Sp);   //還原正常速度
    fAllMotorHome = false;                                                      //觸發整機 Home
    iHome         = 1;
}
//------------------------------------------------------------------------------
//AI(ht9045-staterecord-analysis) 20260525 (RogerYang) : PAUSE/START 邊界重置 Shuttle Move Timeout watchdog， Steven TQPF_Timer 用 wall-clock, PAUSE 期間不會凍結
void ResetShtMoveTimeoutWatchdog()
{
    if(bShtMoveTimerRunning1)
        ShtMoveTimeout1.SetSecAndOn(SHUTTLE_MOVE_TIMEOUT_SEC);
    if(bShtMoveTimerRunning2)
        ShtMoveTimeout2.SetSecAndOn(SHUTTLE_MOVE_TIMEOUT_SEC);
}
//------------------------------------------------------------------------------
void Do_Auto_SHT1()
{
    static int iRetryCT=0;
    static int iHttpErrRetryCT=0;
    static int SH1X=0, SH1Y=0;                                                  //Sam 20210409 : 改為全域
    static int iShuttlePurgeCount=0;
    static int iOldTask=0;
    static bool bCheckRotateSensor[2]={false,false};                            //Steven 20110801
    static bool bNotLeft=false;
    static bool bDuplicateErr=false;
    static bool bMoveLeft=false;                                                //Steven 20220712 : reset shuttle speed for D43
    static bool bSpeedDown=false;
    static bool RunOK=false;                                                    //kevin 20141213 20140206
    static bool RunShuttleCheckIC=false;                                        //kevin 20161108 確認shuttle 移出是否有 IC

    if(IniConfig.bEnableTestingNeedStopAllMotor==true &&
       IniConfig.bI24TestingNeedStopAllMotor==true &&
       bTestingStopAllMotor==true)                                              //jou 2013-09-25 Testing Need Stop All Motor
    {
        return;
    }

    if(MACHINE_HAS_AUTO_ALIGNMENT_CCD && TestIF.bEnableAutoAlignment==true &&
       (LastSet.iRealDummy==HAS_TRAY ||
        LastSet.iRealDummy==REALLY))                                            //KenHsieh 20210813 : add CCD AUTO ALIGNMENT
    {
        if(bRunInArmAutoAlignment ||
           lOutArmAutoAlignmentFlag)
        {
            return;
        }
    }

    if(IniConfig.bF16CheckShuttleSensorBroken && bDoingF16)                     //Steven 20221213 : 確認shuttle 有沒有斷線
        return;

    if(SystemNG)
        return;

    int ret=-1;
    int pos=0;
    int iIndex1YPos=0, iIndex1ZPos=0;                                           //Steven 20120509 : 吸取異常跟Shuttle殘料同時發生會Hang Up
    int &Task=AutoSHT1Task;
    bool &bCheckShuttleFlag=bCheckShuttle1Flag, Flag=false;
    bool bRun2DBarcode=false, bHasICErr=false;                                  //Ifor 20200907 add:Dummy Run 不執行2D Barcode
    AnsiString str, sErrPart="";

    switch(Task)
    {
        case 1:
            if(MOT[MInShuttle1].IsCanMove())
            {
                if(In_Shuttle_Auto_Latch==eInSHAutoLtc)                         //KenHsieh 20250722 : InSht sensor 改為2顆，並用Latch 判別疊料以及飛料
                {
                    if(bInSh1DoLtc==true)
                        break;
                    pos=MOT[MInShuttle1].ReadPos();                             //KenHsieh 20251118 : 修改Out Sht 會重新GetLtc 導致Lose IC 異常
                    if(pos==Prod.InSHT[0].iLeft)
                        bNeedGetSHRightLtc[0]=true;
                }

                if((bShuttle1MoveToLeft &&
                    IniConfig.bD43IndexDropErrorCanRetryandSkip) ||             //ChungHung 20120717 add Index Drop Error Can Retry and Start
                   (bShuttleMoveToLeftforFix3 &&
                    FIX3_FULL_PLACE==Fix3K_UseCylinder))                        //ChungHung 20140313 add Fix3 can Full Tray
                {
                    bMoveLeft=true;                                             //Steven 20220712 : reset shuttle speed for D43
                    Task=100;
                    return;
                }
                else if(bShuttle1MoveToRight &&
                        IniConfig.bD43IndexDropErrorCanRetryandSkip)
                {
                    Task=202;                                                   //Steven 20150414 : Index吸取異常,Shuttle退出後,要檢查置偏
                    return;
                }
                Task=10;
            }
            break;
        case 10:
            if((iOneCycle && IsInArmOneCycleFinish()) ||
               (iCleanOut && IsInArmCleanOutFinish()))
            {
                if(FRCarryKit.UseSiteHasIC())
                {
                    if(IniConfig.bF22InShuttleDetectOutNoIC)                    //kevin 20161108
                    {
                        InitialCheckShuttle1ProminentNoHasICTask();
                        RunShuttleCheckIC=true;                                 //kevin 20161108 In shuttle 有IC進入測區
                    }

                    if(FLCarryKit.UseSiteHasIC())                               //Steven 20220301 : 修正32site One Cycle Hang Up
                    {
                        if(IniConfig.bP17InArmFullPickFromLoader &&
                           IsFLCarrKitAllHasIC()==false)
                        {
                            if(LastSet.iTemperature==Tempture_Hot)
                            {
                                if(MOT[MMPlate1].HasIC() ||
                                   MOT[MMPlate2].HasIC())
                                    break;
                            }
                            else
                            {
                                if(MOT[MMTrayY_Car].HasIC() &&
                                   MOT[MMTrayY].HasIC())                        //Sam 20210601 : 增加保護
                                    break;
                            }
                        }
                    }

                    Task=200;
                }
                else if(FTestSuck.UseSiteHasIC())
                {
                    Task=100;
                }
                else if(FLCarryKit.UseSiteHasIC())
                {
                    if(CanYieldAlarmRemainInSHT()==true)                        //JerryYang 20220923 : yield alarm時觸發half one cycle(shuttle保留IC不測試跳ONE CYCLE FINISH)
                    {
                        Task=100;
                        break;
                    }

                    if(IniConfig.bP17InArmFullPickFromLoader &&
                       IsFLCarrKitAllHasIC()==false)                            //Steven 20220301 : 修正32site One Cycle Hang Up
                    {
                        if(LastSet.iTemperature==Tempture_Hot)
                        {
                            if(MOT[MMPlate1].HasIC() ||
                               MOT[MMPlate2].HasIC())
                                break;
                        }
                        else
                        {
                            if(MOT[MMTrayY_Car].HasIC() &&
                               MOT[MMTrayY].HasIC())                            //Sam 20210601 : 增加保護
                                break;
                        }
                    }
                    Task=200;
                }
                else
                {
                    Task=100;
                }
            }
            else
            {
                if(FTestSuck.UseSiteNoIC())
                {
                    bool bOK=InArmFullPickforTestMode();                        //模式太多,將判斷式包成函式好閱讀
                    if(IniConfig.bP17InArmFullPickFromLoader && bOK)            //ChungHung 20111230 add Shuttle必須放滿
                    {
                        if(IsFLCarrKitAllHasIC())
                        {
                            if(IniConfig.bF22InShuttleDetectOutNoIC)            //kevin 20161108
                            {
                                InitialCheckShuttle1ProminentNoHasICTask();
                                RunShuttleCheckIC=true;                         //kevin 20161108 In shuttle 有IC進入測區
                            }

                            if(bCheckShuttle1MustHasIC &&
                               IniConfig.bF09CheckICWhichFirstTimeLoad)
                            {
                                bCheckShuttle1MustHasIC=false;
                                InitialCheckShuttle1MustHasICTask();
                                InitialCheckShuttle1ProminentTask();
                                Task=5000;
                            }
                            else
                            {
                                if(IniConfig.bUseAutoSiteMapping &&             //jou 2011-03-24 start : Auto Site Mapping
                                   IniConfig.bI21AutoSiteMappingUseHotplate==false)
                                {
                                    if(LastSet.iTemperature==Tempture_Hot &&
                                       LastSet.iRunStartMode==rsmAutoSiteMap &&
                                       bSiteMappingCHKOK==false)
                                    {
                                        if(IniConfig.bI21SkipSoakTime==true)
                                        {
                                            Task=200;
                                        }
                                        else if(IniConfig.bI21UseSameSoakTime==true)
                                        {
                                            if(FLCarryKit.HotCount>=IniConfig.fI21UseSameSoakTime)
                                                Task=200;
                                        }
                                        else if(FLCarryKit.HotCount>=Prod.iHotTime)
                                        {
                                            Task=200;
                                        }
                                    }
                                    else
                                    {
                                        Task=200;
                                    }
                                }
                                else
                                {
                                    Task=200;
                                }
                            }
                        }
                        else
                        {
                            Task=100;
                        }
                    }
                    else
                    {
                        if(FLCarryKit.UseSiteHasIC())                           //Steven 20160205 : 修正32 Site會Hang Up的問題
                        {
                            if(IniConfig.bF22InShuttleDetectOutNoIC)            //kevin 20161108
                            {
                                InitialCheckShuttle1ProminentNoHasICTask();
                                RunShuttleCheckIC=true;                         //kevin 20161108 In shuttle 有IC進入測區
                            }

                            if(bCheckShuttle1MustHasIC &&
                               IniConfig.bF09CheckICWhichFirstTimeLoad)
                            {
                                bCheckShuttle1MustHasIC=false;
                                InitialCheckShuttle1MustHasICTask();
                                InitialCheckShuttle1ProminentTask();
                                Task=5000;
                            }
                            else
                            {
                                if(IniConfig.bUseAutoSiteMapping &&             //jou 2011-03-24 start : Auto Site Mapping
                                   IniConfig.bI21AutoSiteMappingUseHotplate==false)
                                {
                                    if(LastSet.iTemperature==Tempture_Hot &&
                                       LastSet.iRunStartMode==rsmAutoSiteMap &&
                                       bSiteMappingCHKOK==false)
                                    {
                                        if(IniConfig.bI21SkipSoakTime==true)
                                        {
                                            Task=200;
                                        }
                                        else if(IniConfig.bI21UseSameSoakTime==true)
                                        {
                                            if(FLCarryKit.HotCount>=IniConfig.fI21UseSameSoakTime)
                                                Task=200;
                                        }
                                        else if(FLCarryKit.HotCount>=Prod.iHotTime)
                                        {
                                            Task=200;
                                        }
                                    }
                                    else
                                    {
                                        if(IsFLCarrKitAllHasIC()==false &&      //Steven 20160530 : Fix 2D 8 device no trigger  //Jimmychiu 20231130 : 避免跑16sites，shuttle放一半回HOME造成Hang up
                                           TestIF_File.iTestMode!=SingleSite)   //Ifor 20260310 fix:Single Site 2D first device hang up
                                            break;
                                        Task=200;
                                    }
                                }
                                else
                                {
                                    if(IsFLCarrKitAllHasIC()==false &&          //Steven 20160530 : Fix 2D 8 device no trigger
                                       TestIF_File.iTestMode!=SingleSite)       //Ifor 20260310 fix:Single Site 2D first device hang up
                                        break;
                                    Task=200;
                                }
                            }
                        }
                        else
                        {
                            Task=100;
                        }
                    }
                }
                else
                {
                    if(FRCarryKit.UseSiteNoIC())
                    {
                        Task=100;
                    }
                    else if(bUseTwoArm32Site==true    &&
                            MOT[MInShuttle2].fCanMoveR==false)                  //Jou 20150909 : For Add 32Site UPH, 讓Shuttle 1先回去補Input
                    {
                        Task=100;
                    }
                    else if((bShuttle1MoveToLeft &&
                             IniConfig.bD43IndexDropErrorCanRetryandSkip) ||
                            (bShuttleMoveToLeftforFix3 &&
                             FIX3_FULL_PLACE==Fix3K_UseCylinder))               //JerryYang 20160906 修正hang up, shuttle沒有移動到左邊讓fix移動
                    {
                        bMoveLeft=true;                                         //Steven 20220712 : reset shuttle speed for D43
                        Task=100;
                    }
                    else
                    {
                        if(IsFLCarrKitAllHasIC()==false &&                      //Steven 20160530 : Fix 2D 8 device no trigger
                           TestIF_File.iTestMode!=SingleSite)                   //Ifor 20260310 fix:Single Site 2D first device hang up
                            break;
                        Task=200;
                    }
                }
            }
            break;
        case 105:                                                               //jou 981125 start : 防止Index內疊烏龜
            if((bShuttle1MoveToRight ||
                bShuttle1MoveToLeft) ||
               CheckNullICShuttle1_9045(false))
            {
                Task=100;                                                       //PASS
            }
            else
            {
                if(HWaitForShuttleFix1.Off())                                   //FAIL
                {
                    CheckNullICShuttle1_9045(true);
                }
            }
            break;
        case 100:
            if(IniConfig.bHaveRotateShuttle==true &&                            //Steven 20110801 Start :  轉轉蝦頭要檢查有沒有轉頭
               IniConfig.bRotateShNeedCheck &&
               TestIF.bRotateShuttle &&
               bCheckRotateSensor[1]==true)
            {
                bDuplicateErr=false;
                Task=130;
            }
            else
            {
                if(SHSpeed.iStepSH &&
                   FLCarryKit.UseSiteNoIC() &&
                   b1ShuttleMoveToLeft==false)
                {
                    Task=140;
                    iStepShuttleTask[0]=1;
                }
                else
                {
                    Task=120;
                }
            }
            break;
        case 130:
            if(IsTestZ1NotSafeShuttle1CanNotMove(iRetryCT))
                break;
            iRetryCT=0;
            bCheckShuttleFlag=true;

            if(IniConfig.bF05EnableShtPurgeFunction)
            {
                if(InputShuttleFrontHasIC()==false)
                {
                    pos=MOT[MInShuttle1].ReadPos();
                    if(pos!=Prod.InSHT[0].iLeft)
                    {
                        bNotLeft=true;
                        if(iShuttlePurgeCount>=IniConfig.iF05ShuttlePurgeCount)
                            SW[SwShuttleFan].On();
                    }
                }
            }

            if(MachineTypeChoice==Type_HT9045)                                  //9045
            {
                if(IniConfig.bF11OutShtUseFrontRearSensor &&                    //jou 981125 start : 防止Index內疊烏龜 - 使用前後對照
                   bCheckNullIC1 &&
                   MOT[MInShuttle1].fCanMoveR &&
                   MOT[MInShuttle1].fCanMoveM)
                {
                    if(CheckNullICShuttle1_9045(false)==false)
                    {
                        HWaitForShuttleFix1.SetSecAndOn(1);                     //FAIL
                        Task=105;
                        break;
                    }
                    bCheckNullIC1=false;
                }
            }

            if(b1ShuttleMoveToLeft==false)                                      //jou 2010-06-04 start : 防止Index內疊烏龜
                bShuttle1CatchSignalL=true;

            if(IniConfig.bF21InOutArmZMotorPrivate)
            {
                if(DoInOutARM_SHT_MoveSafe(0))                                  //kevin 20161005 SHUTTLE 1 移動安全保護
                    return;
            }

            if(MOT[MInShuttle1].MotorMove(SThreadPara.iOShRotateToLeftCheck[0]))
            {
                RotShuttleDelay1.SetSecAndOn(IniConfig.dRoShCheckDelayTime);
                Task=124;
            }
            break;
        case 124:
            if(RotShuttleDelay1.Off())
            {
                if(Tech.OutSH1ZOneRowDetectPos!=0 &&
                   Sen[SnOutPutSHT1S1].Enable==true &&
                   Sen[SnOutPutSHT1S1].IsOn())
                {
                    ShowErrorMessage("JAM0550", K_RETRY, MOutShuttle1, bDuplicateErr);  //Out Shuttle 1 Rotate Fail!
                    bDuplicateErr=true;
                    Task=130;
                }
                else
                {
                    Task=135;
                    bCheckRotateSensor[1]=false;
                    bDuplicateErr=false;
                }
            }
            break;
        case 135:
            if(IsTestZ1NotSafeShuttle1CanNotMove(iRetryCT))
                break;
            iRetryCT=0;
            bCheckShuttleFlag=true;

            if(IniConfig.bF05EnableShtPurgeFunction)
            {
                if(InputShuttleFrontHasIC()==false)
                {
                    pos=MOT[MInShuttle1].ReadPos();
                    if(pos!=Prod.InSHT[0].iLeft)
                    {
                        bNotLeft=true;
                        if(iShuttlePurgeCount>=LastSet.iShuttlePurgeCount)
                            SW[SwShuttleFan].On();
                    }
                }
            }

            if(MachineTypeChoice==Type_HT9045)                                  //9045
            {
                if(IniConfig.bF11OutShtUseFrontRearSensor &&                    //jou 981125 start : 防止Index內疊烏龜 - 使用前後對照
                   bCheckNullIC1 &&
                   MOT[MInShuttle1].fCanMoveR &&
                   MOT[MInShuttle1].fCanMoveM)
                {
                    if(CheckNullICShuttle1_9045(false)==false)                  //FAIL
                    {
                        HWaitForShuttleFix1.SetSecAndOn(1);
                        Task=105;
                        break;
                    }
                    bCheckNullIC1=false;
                }
            }

            if(b1ShuttleMoveToLeft==false)                                      //jou 2010-06-04 start : 防止Index內疊烏龜
                bShuttle1CatchSignalL=true;

            if(IniConfig.bF21InOutArmZMotorPrivate)
            {
                if(DoInOutARM_SHT_MoveSafe(0))                                  //kevin 20161005 SHUTTLE 移動安全保護
                    return;
            }

            if(MOT[MInShuttle1].MotorMove(Prod.InSHT[0].iLeft)==1)
            {
                if(DoFloodGateClose()==true)                                    //Ifor 20260318 add:Shuttle1 Left Floodgate Close (case 135)
                {
                    Cylinder[C_OutShuttle1Floodgate].On();
                }

                RotShuttleDelay1.SetSecAndOn(IniConfig.dRoShCheckDelayTime);
                Task=125;
            }
            break;
        case 125:
            if(RotShuttleDelay1.Off())
            {
                if(IniConfig.bHaveRotateShuttle==true &&
                   IniConfig.bRotateShNeedCheck   &&
                   TestIF.bRotateShuttle          &&
                   Tech.OutSH1ZOneRowDetectPos!=0 &&
                   Sen[SThreadPara.iInShSenIndex[0][0]].IsOn())
                {
                    ShowErrorMessage("JAM0450", K_RETRY, MInShuttle1, bDuplicateErr);   //In Shuttle 1 Rotate Fail!
                    bDuplicateErr=true;
                    Task=135;
                    break;
                }
                else
                {
                    Task=120;
                    bDuplicateErr=false;
                }
            }
            break;
        case 120:
            if(IniConfig.bF22InShuttleDetectOutNoIC && RunShuttleCheckIC)       //kevin 20161108
            {
                if(FLCarryKit.HasRealIC()==false)                               //Steven 20180402 (Jou) : 修改判斷式
                {
                    Task=115;
                    RunShuttleCheckIC=false;                                    //kevin 20161108
                    return;
                }
            }

            if(IniConfig.bF05EnableShtPurgeFunction)
            {
                if(InputShuttleFrontHasIC()==false)
                {
                    pos=MOT[MInShuttle1].ReadPos();
                    if(pos!=Prod.InSHT[0].iLeft)
                    {
                        bNotLeft=true;
                        if(iShuttlePurgeCount>=IniConfig.iF05ShuttlePurgeCount)
                            SW[SwShuttleFan].On();
                    }
                }
            }

            if(MachineTypeChoice==Type_HT9045)                                  //9045
            {
                if(IniConfig.bF11OutShtUseFrontRearSensor &&                    //jou 981125 start : 防止Index內疊烏龜 - 使用前後對照
                   bCheckNullIC1 &&
                   MOT[MInShuttle1].fCanMoveR &&
                   MOT[MInShuttle1].fCanMoveM)
                {
                    if(CheckNullICShuttle1_9045(false)==false)
                    {
                        HWaitForShuttleFix1.SetSecAndOn(1);                     //FAIL
                        Task=105;
                        break;
                    }
                    bCheckNullIC1=false;
                }
            }

            if(IniConfig.bF21InOutArmZMotorPrivate)
            {
                if(DoInOutARM_SHT_MoveSafe(0))                                  //kevin 20161005 SHUTTLE 1 移動安全保護
                    return;
            }

            if(IsTestZ1NotSafeShuttle1CanNotMove(iRetryCT))                     //kevin 20190614 move
                break;

            iRetryCT=0;

            if(b1ShuttleMoveToLeft==false)                                      //jou 2010-06-04 start : 防止Index內疊烏龜
                bShuttle1CatchSignalL=true;

            if(IniConfig.bD43IndexDropErrorCanRetryandSkip)                     //Steven 20220712 : reset shuttle speed for D43
            {
                if(bShuttle1MoveToLeft && bMoveLeft==true)
                {
                    SetMotorScaleSpeed(MInShuttle1, 10);
                    bMoveLeft=false;
                }
            }

            if(MOT[MInShuttle1].MotorMove(Prod.InSHT[0].iLeft)==1)
            {
                if(DoFloodGateClose()==true)                                    //Ifor 20260318 add:Shuttle1 Left Floodgate Close
                {
                    Cylinder[C_OutShuttle1Floodgate].On();
                }

                if(IniConfig.bD43IndexDropErrorCanRetryandSkip)                 //Steven 20220712 : reset shuttle speed for D43
                {
                    if(bShuttle1MoveToLeft)
                    {
                        SetMotorScaleSpeed(MInShuttle1, SHSpeed.iSH1Sp);
                    }
                }
                bMoveLeft=false;
                RunOK=false;                                                    //JerryYang 20160809 修正開啟F18 hang up問題
                bCheckShuttleFlag=true;                                         //kevin 20190614 move

                if(IniConfig.bHaveRotateShuttle==true &&
                   IniConfig.bRotateShNeedCheck   &&
                   TestIF.bRotateShuttle          &&                            //Steven 20110801 : 轉轉蝦頭要檢查有沒有轉頭
                   Tech.OutSH1ZOneRowDetectPos!=0 &&
                   Sen[SThreadPara.iInShSenIndex[0][0]].IsOn())
                {
                    ShowErrorMessage("JAM0450", K_RETRY, MInShuttle1, bDuplicateErr); //In Shuttle 1 Rotate Fail!
                    bDuplicateErr=true;
                    break;
                }
                else
                {
                    bDuplicateErr=false;
                }

                if(iOldTask!=Task)
                {
                    bShuttle1CatchSignalL=false;                                //執行序結束
                    fLtcSensor->GetLtcSensor(0);                                //Steven 20110526 : 取得往左移動的Latch
                    iOldTask=Task;
                }

                if(bSpeedDown==true)
                    SetMotorScaleSpeed(MInShuttle1, SHSpeed.iSH1Sp);            //Steven 20110607 : 重新檢查時要放慢速度, 檢查完要還原速度

                if(IniConfig.bIndexJamInArmAway)                                //kevin 20180504 add
                {                                                               //kevin 20180504 move up for hang up
                   if(bInArmNeedToSafePos)
                    {
                        break;
                    }
                }

                if(bShuttle1Pause==true)                                        //jou 990104 start : add from ht9080a
                {
                    bShuttle1Pause=false;
                    bShowShuttle1Device=true;                                   //kevin 20180504 index pick up error
//                    bAutoCleanCheckOpenDoor=true;                             //JerryYang 20190625 mark掉       //kevin 20180725 (wei) add 掉料忽視需開門 Alarm 後需要開門確認
                    bHasICErr=false;
                    sErrPart="";
                    if(IniConfig.bD81IndexCheckVacuumOnShuttle)
                    {
                        for(int i=0; i<MAX_Index_Row; i++)
                        {
                            for(int j=0; j<NEW_MAX_Index_Col; j++)
                            {
                                if(bSHT1_DropPosHasIC[i][j]==true)
                                {
                                    bHasICErr=true;
                                    bSHT1_DropPosHasIC[i][j]=false;
                                    if(TestIF_File.iTestMode==_32Site4X8M ||
                                        TestIF_File.iTestMode==_32Site4X8N ||
                                        TestIF_File.iTestMode==_16Site4X4)                        //Sam 20190226 : 16Site4X4 ///kevin 20180504 add  error pos
                                        sErrPart+=IndexSuckName[i+2][j];
                                    else if(TestIF_File.iTestMode==QualSite2X2N ||
                                            TestIF_File.iTestMode==_6Site2X3N)
                                            sErrPart+=IndexSuckName[i+1][j];                     //Steven 20230712 : 修正NN mode alarm顯示
                                    else
                                        sErrPart+=IndexSuckName[i][j];
                                }
                            }
                        }
                    }

                    if(bHasICErr && bSht1OnlyHasICErr)
                    {
                        str.sprintf("Input arm droped device in Shuttle1, need remove device of %s", sErrPart);
                        if(IniConfig.bAlarmNeedServoOff)    //Steven 20110802
                            ShowMyMessage(str, "","", false, true);
                        else
                            ShowMyMessage(str, "");
                    }
                    else
                    {
                        if(bHasICErr)
                        {
                            str.sprintf("Input arm droped device in Shuttle1, need remove device of %s", sErrPart);
                            if(IniConfig.bAlarmNeedServoOff)    //Steven 20110802
                                ShowMyMessage(str, "","", false, true);
                            else
                                ShowMyMessage(str, "");
                        }

                        if(IniConfig.bAlarmNeedServoOff)                            //Steven 20110802
                            ShowMyMessage("Test Arm 1 Pick up error after Skip, need take away shuttle 1 device [D42]", "Test Arm 1 吸取錯誤並且Skip, 需要拿走shuttle 1 device [D42]", "", false, true);
                        else
                            ShowMyMessage("Test Arm 1 Pick up error after Skip, need take away shuttle 1 device [D42]", "Test Arm 1 吸取錯誤並且Skip, 需要拿走shuttle 1 device [D42]");
                    }

                    fMain->Pause("Do_Auto_SHT1");
                }

                if(IniConfig.bF28IndexCheckShuttlePos)                          //kevin 20220512 add  Index check shuttle pos for Sensor
                {
                    if(Sen[SnSht1Left].Enable && Sen[SnSht1Left].IsOff())
                    {
                        ShowMyMessage("Shuttle 1 Left detect sensor on error");
                        break;
                    }
                }

                if(b1ShuttleMoveToLeft==false)
                {
                    if(b1ShuttleMoveToRight==true)
                    {
                        b1ShuttleMoveToRight=false;
                        bSHTOfsChangeRight[0]=false;                            //Sam 20221205 : 修正 Shuttle 移動完成後又被修改 Offset 導致 Hang up
                        if(TestIF.iShuttleMode==0 ||
                           (TestIF.iShuttleMode==1 && TestIF.iShuttle_Sel==0))  //Jou 20110421
                        {
                            if(SThreadPara.bExeShuttleThread==false)
                            {
                                if(FRCarryKit.UseSiteNoIC())                    //jou 2016-04-22 修正Out shuttle殘料異常
                                {
                                    if(CheckShuttle1OutputHasICError_M204())
                                    {
                                        Task=400;
                                        break;
                                    }
                                }
                            }
                            else
                            {
                                if(CheckShuttleOutputHasICError(0, SH1X, SH1Y))
                                {
                                    if(fMain->cbShowShuttleSensor->Checked==true)
                                    {
                                        AnsiString sFileName;
                                        sFileName.sprintf("%04d-%02d-%02d %02d:%02d:%02d.%d has error, retry:%d", SystemYear, SystemMonth, SystemDate, SystemHour, SystemMin, SystemSec, SystemMSec, iOutShuttle1HasICErrRetryCnt);     //Sam 20210409 : 改為全域
                                        fMain->meShuttle1->Lines->Add(sFileName);
                                    }
                                    Task=400;
                                    break;
                                }
                            }
                        }
                    }
                    iOutShuttle1HasICErrRetryCnt=0;                             //Sam 20210409 : 改為全域
                }

                b1ShuttleMoveToLeft=true;                                       //確實移動到左邊
                StartDetectMotorSensor(0);

                if(In_Shuttle_Auto_Latch==eInSHAutoLtc &&
                   bNeedGetSHRightLtc[0]==false)                                //KenHsieh 20251119 : In SHLtc 後，馬上從右到左Latch 須清除，避免會造成Out Lose IC 誤判
                {
                    fLtcSensor->ClearLtcSensor(0);
                    fLtcSensor->SetLtcSensor(0);
                }

                if(InputShuttleFrontHasIC()==false &&
                   IniConfig.bF05EnableShtPurgeFunction && bNotLeft==true)
                {
                    bNotLeft=false;
                    iShuttlePurgeCount++;
                    if(iShuttlePurgeCount>IniConfig.iF05ShuttlePurgeCount)
                        iShuttlePurgeCount=0;
                }
                SW[SwShuttleFan].Off();

                if(IniConfig.bHaveRotateShuttle==true &&
                   IniConfig.bRotateShNeedCheck &&
                   TestIF.bRotateShuttle)                                       //Steven 20110802 : 轉轉蝦頭要檢查有沒有轉頭
                {
                    bCheckRotateSensor[0]=true;

                    if(IniConfig.bRotateShNeedCheck &&                          //ChungHung 20110922 : 轉轉蝦頭要檢查有沒有轉頭 Check Sensor
                       TestIF.bRotateShuttle)
                    {
                        if(Sen[SnRotateCheck].IsOn())                           //確認SnRotateCheck 是否正常
                        {
                            ShowErrorMessage("JAM0450", K_RETRY, MInShuttle1);  //In Shuttle 1 Rotate Fail!
                            fAllMotorHome=false;
                            iHome=1;
                            break;
                        }
                    }
                }
                Task=1;
                fLtcSensor->ClearLtcSensor(0);                                  //Steven 20250728
                bShtMoveTimerRunning1=false;                                    //Steven 20260507 : arrived, clear watchdog
            }
            else
            {
                b1ShuttleMoveToLeft=false;

                if(!bShtMoveTimerRunning1)                                      //Steven 20260507 : MotorMove not arrived, start watchdog
                {
                    ShtMoveTimeout1.SetSecAndOn(SHUTTLE_MOVE_TIMEOUT_SEC);
                    bShtMoveTimerRunning1=true;
                }
                else if(ShtMoveTimeout1.Off())                                 //Steven 20260507 : watchdog expired
                {
                    bShtMoveTimerRunning1=false;
                    DoShtMoveTimeoutHandle(0);                                  //AI(ht9045-staterecord-analysis) 20260430 (RogerYang) : trigger Timeout handler
                }
            }
            break;
        case 115:                                                               //kevin 20161108 in shuttle 退出是否有IC 殘留檢測
            if(DoCheckShuttle1EmptyIC())
            {
                Task=120;
            }
            break;
        case 110:                                                               //Steven 20110603 : 重新檢查前,多等待一下
            if(ShuttleDelay1.Off())
            {
                bSpeedDown=true;
                SetMotorScaleSpeed(MInShuttle1, iShuttleSpeed);                 //Steven 20110607 : 重新檢查時要放慢速度
                Task=100;
            }
            break;
        case 140:
            if(IsTestZ1NotSafeShuttle1CanNotMove(iRetryCT))
                break;
            iRetryCT=0;
            bCheckShuttleFlag=true;

            if(IniConfig.bF05EnableShtPurgeFunction)
            {
                if(InputShuttleFrontHasIC()==false)
                {
                    pos=MOT[MInShuttle1].ReadPos();
                    if(pos!=Prod.InSHT[0].iLeft)
                    {
                        bNotLeft=true;
                        if(iShuttlePurgeCount>=IniConfig.iF05ShuttlePurgeCount)
                            SW[SwShuttleFan].On();
                    }
                }
            }

            if(MachineTypeChoice==Type_HT9045)                                  //9045
            {
                if(IniConfig.bF11OutShtUseFrontRearSensor &&                    //jou 981125 start : 防止Index內疊烏龜 - 使用前後對照
                   bCheckNullIC1 &&
                   MOT[MInShuttle1].fCanMoveR &&
                   MOT[MInShuttle1].fCanMoveM)
                {
                    if(CheckNullICShuttle1_9045(false)==false)                  //FAIL
                    {
                        HWaitForShuttleFix1.SetSecAndOn(1);
                        Task=105;
                        break;
                    }
                    bCheckNullIC1=false;
                }
            }

            if(b1ShuttleMoveToLeft==false)                                      //jou 2010-06-04 start : 防止Index內疊烏龜
                bShuttle1CatchSignalL=true;

            if(DoStepShuttleCheck(0))
            {
                Task=120;
            }
            break;
        case 400:
            if(IniConfig.bF21InOutArmZMotorPrivate)
            {
                if(DoInOutARM_SHT_MoveSafe(0))                                  //kevin 20161005 SHUTTLE 1 移動安全保護
                    return;
            }
            iIndex1YPos=MOT[MTestY1].Gali_ReadPos();                            //Steven 20120509 : 吸取異常跟Shuttle殘料同時發生會Hang Up
            iIndex1ZPos=MOT[MTestZ1].Gali_ReadPos();
            if(MOT[MInShuttle1].IsCanMove() &&
               InArmZSafe(DETECT_SENSOR_FLAG)==-1 &&
               (iIndex1ZPos>=Prod.TestZ1_Safe-3 ||
                iIndex1YPos!=Prod.TestY1_Front))                                //ChungHung 20150520 fix hangup 遇到iIndex2ZPos差一條的情況
            {
                if(MOT[MInShuttle1].MotorMove(Prod.InSHT[0].iRight-1000)==1)    //故意讓位置不到位，所以-1000
                {
                    if(iOldTask!=Task)
                    {
                        fLtcSensor->ClearLtcSensor(0);                          //Sam 20221101 : Latch 清除都要確認是否清清乾淨
                        iOldTask=Task;
                    }

                    iOutShuttle1HasICErrRetryCnt++;
                    if(iOutShuttle1HasICErrRetryCnt>=2)                         //Sam 20210409 : 改為全域
                    {
                        Task=500;
                        break;
                    }
                    else
                    {
                        fLtcSensor->SetLtcSensor(0);                            //JerryYang 20230406 : 重置shuttle1的latch
                        fLtcSensor->ClearLtcSensor(0);                          //Sam 20221101 : Latch 清除都要確認是否清清乾淨
                        ret=K_RETRY;
                    }

                    if(ret==K_RETRY)
                    {
                        b1ShuttleMoveToRight=true;
                        b1ShuttleMoveToLeft=false;
                        StartDetectMotorSensor(0);
                    }
                    Task=110;                                                   //Steven 20110603
                    ShuttleDelay1.SetSecAndOn(0.5);
                }
            }
            else
            {
                MOT[MInShuttle1].PCIL132_StopMotor();
            }
            break;
        case 500:
            if(SThreadPara.bExeShuttleThread==false)
            {
                if(iHWFix_BinBox==1 && OutArmSuck.NoIC())                       //kevin 20170214 (wei) out arm 沒有ic 移到等待點方便取出 error bin
                {
                    while(Flag==false)
                    {
                        Flag=MoveOutArmXY_ToShuttleAlarmArea();                 //kevin 20160725
                        if(MOT[MTrayX].ReadPos()>=Prod.iXTrayAuto[0])
                            Flag=true;
                    }
                }

                if(CUSTOMER_CODE==CC_ASE_KaohSiung  ||
                   CUSTOMER_CODE==CC_ASE_KaohSiung_K12 ||                       //Steven 20131101 : Add ASE-K12
                   CUSTOMER_CODE==CC_SCS            ||                          //Steven 20110209
                   CUSTOMER_CODE==CC_AMKOR_Japan    ||                          //Steven 20160225 : J-Device Out Shuttle殘料只能Retry
                   CUSTOMER_CODE==CC_ASE_M          ||
                   CUSTOMER_CODE==CC_ASE_SG         )                           //Ifor 20200219 :add ASE SG Out Shuttle殘料只能Retry
                {
                    ret=ShowErrorMessage("JAM0560", K_RETRY, MOutShuttle1, false, SThreadPara.ErrPartSh1);    //Jou 2011-02-08
                }
                else if(AccessLevel<iDefHonPrecLevel)                           //Steven 20110210 : Device Superfloat可以Skip //jou 2014-06-19 Security Have 5 Level 3->iDefHonPrecLevel
                {
                    ret=ShowErrorMessage("JAM0560", K_RETRY, MOutShuttle1, false, SThreadPara.ErrPartSh1);    //Jou 2011-02-08
                }
                else
                {
                    ret=ShowErrorMessage("JAM0560", K_RETRY|K_SKIP, MOutShuttle1, false, SThreadPara.ErrPartSh1);
                }
            }
            else
            {
                EndShuttleThread();                                             //jou 2011-02-17
                if(CUSTOMER_CODE==CC_ASE_KaohSiung  ||
                   CUSTOMER_CODE==CC_ASE_KaohSiung_K12 ||                       //Steven 20131101 : Add ASE-K12
                   CUSTOMER_CODE==CC_SCS            ||                          //Steven 20110209
                   CUSTOMER_CODE==CC_AMKOR_Japan    ||                          //Steven 20160225 : J-Device Out Shuttle殘料只能Retry
                   CUSTOMER_CODE==CC_ASE_M          ||
                   CUSTOMER_CODE==CC_ASE_SG         )                           //Ifor 20200219 :add ASE SG Out Shuttle殘料只能Retry
                {
                    if(CUSTOMER_CODE==CC_ASE_KaohSiung && OutArmSuck.NoIC())    //kevin 20160725 out arm 沒有ic
                    {
                        while(Flag==false)
                        {
                            Flag= MoveOutArmXY_ToShuttleAlarmArea();            //kevin 20160725
                            if(MOT[MTrayX].ReadPos()>=Prod.iXTrayAuto[0])
                                Flag=true;
                        }
                    }
                    ret=ShowErrorMessage("JAM0560", K_RETRY, MOutShuttle1, false, IndexSuckName[SH1Y][SH1X]);    //Steven 20111114 改
                }
                else if(TestIF.iTestMode==DualSite2x1)
                {
                    AnsiString ErrPart=" ";
                    ErrPart+=IndexSuckName[0][0];
                    ErrPart+=IndexSuckName[1][0];
                    ret=ShowErrorMessage("JAM0560", K_RETRY, MOutShuttle1, false, ErrPart);    //Steven 20111114 改
                }
                else if(AccessLevel<iDefHonPrecLevel)                           //Steven 20110210 : Device Superfloat可以Skip //jou 2014-06-19 Security Have 5 Level 3->iDefHonPrecLevel
                {
                    ret=ShowErrorMessage("JAM0560", K_RETRY, MOutShuttle1, false, IndexSuckName[SH1Y][SH1X]);    //Steven 20111114 改
                }
                else
                {
                    ret=ShowErrorMessage("JAM0560", K_RETRY|K_SKIP, MOutShuttle1, false, IndexSuckName[SH1Y][SH1X]);    //Steven 20111114 改
                }
                StartShuttleThread();                                           //jou 2011-02-17
            }

            iOutShuttle1HasICErrRetryCnt=0;                                     //Sam 20210409 : 改為全域
            if(ret==K_RETRY)
            {
                b1ShuttleMoveToRight=true;
                b1ShuttleMoveToLeft=false;
                StartDetectMotorSensor(0);
            }
            Task=110;                                                           //Steven 20110603
            ShuttleDelay1.SetSecAndOn(0.5);
            break;
        case 202:                                                               //Steven 20150414 : Index吸取異常,Shuttle退出後,要檢查置偏
            if(In_Shuttle_Auto_Latch==eInSHAutoLtc)                             //KenHsieh 20250722 : InSht sensor 改為2顆，並用Latch 判別疊料以及飛料
            {
                InitAutoChkInSHLatchTask();
                Task=203;
            }
            else if(CheckShuttleSensor_9045(1))
            {
                Task=200;
            }
            break;
        case 203:                                                               //KenHsieh 20251104 : add 檢測後動作 & alarm 變數
            if(CheckInShuttleSensor_Latch(0, 0, true)==1)
                Task=200;
            break;
        case 200:
            if(In_Shuttle_Auto_Latch==eInSHAutoLtc &&
               bInSh1DoLtc==true)                                               //KenHsieh 20250722 : InSht sensor 改為2顆，並用Latch 判別疊料以及飛料
                break;

            if(TestIF.iShuttleMode==1 &&
               TestIF.iShuttle_Sel==1)                                          //只用Shuttle 2
            {
                Task=201;
            }
            else if((USE_LASER_DISTANCE && TestIF_File.bEnableShuttleLaser) ||
                    (USE_LASER_DISTANCE==2 && TestIF_File.bEnableInShuttleFloatingCheck))       //Steven 20140228 : 雷射測距功能
            {
                if(InShtInRT(0))
                {
                    Task=201;
                }
                else
                {
                    if(AutoSHT2Task==3000 ||
                      (USE_LASER_DISTANCE==2 && TestIF_File.bEnableInShuttleFloatingCheck && AutoSHT2Task==3100))   //互卡
                        break;

                    if(USE_LASER_DISTANCE==2 && TestIF_File.bEnableInShuttleFloatingCheck)
                    {
                        Task=3100;
                    }
                    else
                    {
                        CheckShtFloating(0, true);
                        Task=3000;
                    }
                    break;
                }
            }
            else
            {
                Task=201;
            }
        case 201:
            if(SHT_FLOATING_CHK==1 &&
               TestIF_File.bEnableShtFloatChk)                                  //Steven 20160920 : IC置偏檢查
            {
                if(InSHT1InLF()==false)                                         //JerryYang 20200722 : fix index arm下壓讓shuttle偏移INP亮燈造成沒掃2D就進去測試
                {
                    break;
                }
                fBarCode->InitialShuttleFloatCheck1();
                Task=2300;
                break;
            }

            if(CUSTOMER_CODE==CC_AMD_M && LastSet.iRealDummy!=REALLY)
            {
                bRun2DBarcode=false;                                            //Ifor 20200907 add:Dummy Run 不執行2D Barcode
            }
            else
            {
                bRun2DBarcode=true;                                             //Ifor 20200907 add:Dummy Run 不執行2D Barcode
            }

            if(BAR_CODE_INSTALL!=ebctUninstall &&                               //pig 2011.12.05 2DBarcode
               bResetMode==false &&
               bRun2DBarcode==true)                                             //Steven 20201125 : RESET時, 不需要掃2DID
            {
                /*if(bShuttle1MoveToRight && IniConfig.bD43IndexDropErrorCanRetryandSkip)    //Steven 20160802 : fixed for duplicate 2DID after index pick up error
                {
                    Task=210;
                }
                else*/                                                          //Steven 20160804 : Sck Hur說一定要重掃, 所以改方法

                if(BAR_CODE_INSTALL==ebctOutShtAMD && FRCarryKit.UseSiteHasIC())
                {
                    if(InSHT1InLF()==false)                                     //JerryYang 20200722 : fix index arm下壓讓shuttle偏移INP亮燈造成沒掃2D就進去測試
                    {
                        break;
                    }
                    fBarCode->InitialBarcodeScanOutShuttle1();
                    Task=2500;
                    break;
                }
                else if(BAR_CODE_INSTALL!=ebctUninstall &&
                        TestIF_File.bEnableBarCode &&
                        FLCarryKit.UseSiteHasIC())
                {
                    if(BOTTOM_2DID && TestIF_File.bEnableBottom2D)              //Steven 20190308 : Bottom 2D
                    {
                        ret=SendHTTPRequest(0);                                 //Steven 20200804 : 修正bottom 2D不會回傳資料給server的問題
                        if(ret==-1)                                             //Steven 20200909 : 修改Murata XML上傳資料
                        {
                            str.sprintf("Do Shuttle 1 SendHTTPRequest fail retry count: %d", iHttpErrRetryCT);
                            RecordProcess(str);
                            if(iHttpErrRetryCT>3)
                            {
                                ret=ShowErrorMessage("WAR16320", K_RETRY|K_RESET, MInShuttle1, false, "Do_Auto_SHT1");    //Steven 20201113 : 上傳失敗3次要有Alarm
                                iHttpErrRetryCT=0;
                                if(ret==K_RESET)
                                {
                                    fMain->Reset("Do_Auto_SHT1");
                                    Task=210;                                   //Steven 20201125 : RESET時, 不需要掃2DID
                                }
                                else
                                {
                                    Task=2600;
                                    ShuttleDelay1.SetSecAndOn(30);
                                }
                                break;
                            }
                            else
                            {
                                Task=2600;
                                ShuttleDelay1.SetSecAndOn(30);
                                break;
                            }
                        }
                        else
                        {
                            iHttpErrRetryCT=0;
                        }
                    }
                    else
                    {
                        if(fBarCode->IsSHT2DIDScanFinish(0)==false)             //JerryYang 20200916 : fix hang up, 還沒掃2D才要在左邊
                        {
                            if(InSHT1InLF()==false)                             //JerryYang 20200722 : fix index arm下壓讓shuttle偏移INP亮燈造成沒掃2D就進去測試
                            {
                                break;
                            }
                            fBarCode->InitialBarcodeScanInShuttle1();
                            Task=2400;
                            break;
                        }
                    }
                }
            }

            if(IniConfig.bHaveRotateShuttle==true &&                            //Steven 20110801 Start: 轉轉蝦頭要檢查有沒有轉頭
               IniConfig.bRotateShNeedCheck &&
               TestIF.bRotateShuttle &&
               bCheckRotateSensor[0]==true)
            {
                bDuplicateErr=false;
                if(SThreadPara.bChechInShFirst[0])
                    Task=220;
                else
                    Task=230;
            }
            else
            {
                Task=210;
            }

            if(IniConfig.bIndexPickupWait==true &&                              //jou 2012-06-29 Index Pick up need wait Soak Time
               (LastSet.iTemperature==Tempture_Hot ||
                LastSet.iTemperature==Tempture_AmbientHot))                     //kevin 20180903 (Steven) : add 恆溫控制
            {
                if(Temperature.iShuttleSoakTimeMode==0 && Temperature.iInitialStart2Time!=0)
                {
                    FLCarryKit.TSoakTime.SetSecAndOn(Temperature.iInitialStart2Time);
                }
            }
            break;
        case 220:
            if(DeviceForm.bShuttleWaitingOutSiteChamber &&                      //jou 980930 start : Shuttle Waiting Out Site Chamber
               (LastSet.iTemperature==Tempture_Hot ||
                LastSet.iTemperature==Tempture_AmbientHot))                     //kevin 20180903 (Steven) : add 恆溫控制
            {
                if(fRearNeedTest)
                    break;
            }

            SW[SwShuttleFan].Off();
            if(IsTestZ1NotSafeShuttle1CanNotMove(iRetryCT))
                break;

            if(IniConfig.bF21InOutArmZMotorPrivate)
            {
                if(DoInOutARM_SHT_MoveSafe(0))                                  //kevin 20161005 SHUTTLE 1 移動安全保護
                    return;
            }
            iRetryCT=0;

            if(InSHT1InRT())
            {
                bCheckShuttleFlag=true;
                HWaitForShuttleFix1.Set0_1SecAndOn(10);
            }

            if(b1ShuttleMoveToRight==false)
                bShuttle1CatchSignalR=true;

            if(MOT[MInShuttle1].MotorMove(SThreadPara.iInShRotateCheck[0])==1)
            {
                RotShuttleDelay1.SetSecAndOn(IniConfig.dRoShCheckDelayTime);
                Task=225;
            }
            break;
        case 225:
            if(RotShuttleDelay1.Off())
            {
                if(Tech.iInSH1Sen7DetectPos!=0 &&
                   Sen[SThreadPara.iInShuttleSen7[0]].Enable==true &&
                   Sen[SThreadPara.iInShuttleSen7[0]].IsOn())
                {
                    Task=220;
                    ShowErrorMessage("JAM0450", K_RETRY, MInShuttle1, bDuplicateErr);   //In Shuttle 1 Rotate Fail!
                    bDuplicateErr=true;
                }
                else
                {
                    bDuplicateErr=false;
                    if(SThreadPara.bChechInShFirst[0])
                    {
                        Task=230;                                               //In先，所以要到Out
                    }
                    else
                    {
                        bCheckRotateSensor[0]=false;
                        Task=210;
                    }
                }
            }
            break;
        case 230:
            if(DeviceForm.bShuttleWaitingOutSiteChamber &&                      //jou 980930 start : Shuttle Waiting Out Site Chamber
               (LastSet.iTemperature==Tempture_Hot ||
                LastSet.iTemperature==Tempture_AmbientHot))                     //kevin 20180903 (Steven) : add 恆溫控制
            {
                if(fRearNeedTest)
                    break;
            }

            SW[SwShuttleFan].Off();
            if(IsTestZ1NotSafeShuttle1CanNotMove(iRetryCT))
                break;

            iRetryCT=0;

            if(InSHT1InRT())
            {
                bCheckShuttleFlag=true;
                HWaitForShuttleFix1.Set0_1SecAndOn(10);
            }

            if(b1ShuttleMoveToRight==false)
                bShuttle1CatchSignalR=true;

            if(MOT[MInShuttle1].MotorMove(SThreadPara.iOShRotateToRightCheck[0])==1)
            {
                Task=235;
                RotShuttleDelay1.SetSecAndOn(IniConfig.dRoShCheckDelayTime);
            }
            break;
        case 235:
            if(RotShuttleDelay1.Off())
            {
                if(Tech.OutSH1ZOneRowDetectPos!=0 &&
                   Sen[SnOutPutSHT1S1].Enable==true &&
                   Sen[SnOutPutSHT1S1].IsOn())
                {
                    ShowErrorMessage("JAM0550", K_RETRY, MOutShuttle1, bDuplicateErr);  //Out Shuttle 1 Rotate Fail!
                    Task=230;
                    bDuplicateErr=true;
                }
                else
                {
                    bDuplicateErr=false;
                    if(SThreadPara.bChechInShFirst[0])
                    {
                        bCheckRotateSensor[0]=false;
                        Task=210;
                    }
                    else
                    {
                        Task=220;                                               //Out先，所以要到In
                    }
                }
            }
            break;
        case 3000:
            if(CheckShtFloating(0))
            {
                Task=201;
            }
            break;
        case 3100:
            if(InArmSuck.HasIC()==false)
            {
                bInArmLaserActionflag[0]=true;
                Task=3101;
            }
            break;
        case 3101:
            if(MoveInArmZToPlateSafe(3101))
            {
                UseInArmCheckShtFloating(0, true);
                Task=3102;
            }
            break;
        case 3102:
            if(UseInArmCheckShtFloating(0))
            {
                bInArmLaserActionflag[0]=false;
                Task=201;
            }
            break;
        case 210:
            if(CheckCFixTrayFullPlace())                                        //Steven 20220420 : 針對Fix3氣缸加上保護機制
            {
                if(bShuttleMoveToLeftforFix3 &&
                   FIX3_FULL_PLACE==Fix3K_UseCylinder)                          //Steven 20220517 : Add for Fix 3 Cyn
                {
                    Task=100;
                }
                return;
            }

            if(IniConfig.bD72NNModeMoveShtAfterContact)                         //Steven 20220531 : index下壓之後才能移動shuttle
            {
                if(bUseTwoArm32Site==true)
                {
                    if(IndexStatus!=Z1_Z2_Down &&
                       FTestSuck.UseSiteHasIC() &&
                       FTestSuck.AlreadyTest()==false)
                    {
                        return;
                    }
                }
            }

            if(FLCarryKit.HasDefineIC(HAS_NULL_CLEAN_IC) ||                     //JerryYang 20220923 : 增加保護, 一般流程時shuttle資料不該有auto clean data
               FLCarryKit.HasDefineIC(HAS_CLEAN_IC) ||
               FLCarryKit.HasDefineIC(CLEAN_FINISH_IC))
            {
                ShowMyMessage("Shuttle 1 has clean pad data, please re-start the progarm and clear all device in machince.", "Shuttle 1殘留clean pad資料, 請重開程式並清機");
                break;
            }

            if((Prod.bF18InshuttleDetect ||
                IniConfig.bF20InShuttleProminentDetect) &&                      //jou 20170418 (Steven) : 矽品-世明要求修改config F18至Recipe設定
               (RunOK==false && LastSet.iRealDummy==REALLY) &&                  //kevin 20141213   20140206 input SHUTTLE 第9顆sensor 進入偵測是否有ic)
                CosFunction.bInShuttleDetectByLatch==false)                     //Isaac 20170418 (Steven) 用Y Latch檢測Inshuttle有無IC
            {
                for(int i=0; i<FLCarryKit.iShtRow; i++)
                {
                    for(int j=0; j<FLCarryKit.iShtCol; j++)
                    {
                        if(FLCarryKit.Item[i][j]!=NULL_IC)                      //JerryYang 20241104 : HAS NULL IC也要檢查確實無IC殘留
                        {
                            Task=5000;
                            bF18CheckShuttle1MustHasIC=true;                    //JerryYang 20170515 (Steven) 修正in shuttle detect異常後關閉F18會hang up
                            return;
                        }
                    }
                }
            }

            if(DeviceForm.bShuttleWaitingOutSiteChamber &&                      //jou 980930 start : Shuttle Waiting Out Site Chamber
               (LastSet.iTemperature==Tempture_Hot ||
                LastSet.iTemperature==Tempture_AmbientHot))                     //kevin 20180903 (Steven) : add 恆溫控制
            {
                if(fRearNeedTest)
                    break;
            }

            if(IniConfig.bD44CheckIndexICDestroy && iIndexTakeDeviceChk1!=0)    //kevin 20190103 add 確認回吸check ok
                break;

//            if(ishuttleShake==0 && iShakeShuttleTask==1)                      //JerryYang 20190421 Mark掉, shuttle shake的時候這裡不會進來      //kevin 20190107 shuttle error
//                break;
            SW[SwShuttleFan].Off();
            if(IsTestZ1NotSafeShuttle1CanNotMove(iRetryCT))
                break;
            iRetryCT=0;

            if(InShtInRT(0)==false)                                             //Isaac 20201217 : 若齒輪比大於1，換算有機會和目標位置差1條
            {
                bCheckShuttleFlag=true;
                HWaitForShuttleFix1.SetSecAndOn(1);
            }

//            if(IniConfig.bF16CheckShuttleSensorBroken)                        //2014-01-06    Dell    for TSMC 確認shuttle 有沒有斷線
//                CheckShuttleSensorBroken_1();

            if(b1ShuttleMoveToRight==false)
                bShuttle1CatchSignalR=true;

            if(IniConfig.bF21InOutArmZMotorPrivate)
            {
                if(DoInOutARM_SHT_MoveSafe(0))                                  //kevin 20161005 SHUTTLE 1 移動安全保護
                    return;
            }

            if(bReadAndCheckCPUName==true &&
               CosFunction.bInShuttleDetectByLatch &&                           //Isaac 20170418 (Steven) 用Y Latch檢測Inshuttle有無IC
               Prod.bF18InshuttleDetect &&
               MOTION_CARD_TYPE==MotionCard_Contec &&
               LastSet.iRealDummy==REALLY)                                      //jou 20170418 (Steven) : 矽品-世明要求修改config F18至Recipe設定
            {
                bInSht1LtcDetectTesterCanMove=false;
            }
            else
            {
                bInSht1LtcDetectTesterCanMove=true;
            }

            if(CosFunction.bUseAfterHomeShtChkLoseICNeedSlow &&                 //Sam 20201020 : 回 Home 第一次的 Shuttle Check IC lose 需要變慢。
               bAfterHomeShtChkLoseICNeedSlow[0] &&
               FRCarryKit.UseSiteHasIC())                                       //確認 OutShuttle 有 IC 才需要變慢
            {
                SetMotorScaleSpeed(MInShuttle1, iShuttleSpeed);
            }

            if(MOT[MInShuttle1].MotorMove(Prod.InSHT[0].iRight)==1)
            {
                if(DoFloodGateClose()==true)                                    //Ifor 20260318 add:Shuttle1 Right Floodgate Close
                {
                    Cylinder[C_Shuttle1Floodgate].On();
                }

                if(IniConfig.bHaveRotateShuttle==true &&
                   IniConfig.bRotateShNeedCheck &&
                   TestIF.bRotateShuttle)                                       //Steven 20110801 : 轉轉蝦頭要檢查有沒有轉頭
                {
                    bCheckRotateSensor[1]=true;
                }

                if(iOldTask!=Task)
                {
                    if(In_Shuttle_Auto_Latch!=eInSHAutoLtc ||
                      (In_Shuttle_Auto_Latch==eInSHAutoLtc &&
                       bNeedGetSHRightLtc[0]))                                  //KenHsieh 20260225 : 更改判斷位置，避免iOldTask 沒更新，導致到左邊沒判斷 //KenHsieh 20251118 : 修改Out Sht 會重新GetLtc 導致Lose IC 異常
                        fLtcSensor->GetLtcSensor(0);                            //Steven 20110526 : 取得往右移動的Latch

                    iOldTask=Task;
                }

                if(IniConfig.bF28IndexCheckShuttlePos)                          //kevin 20220512 add  Index check shuttle pos for Sensor
                {
                    if(Sen[SnSht1Right].Enable && Sen[SnSht1Right].IsOff())
                    {
                        ShowMyMessage("Shuttle 1 right detect sensor on error");
                        break;
                    }
                }

                bShuttle1CatchSignalR=false;

                bCheckNullIC1=true;                                             //jou 981125 start : 防止Index內疊烏龜

                if(bReadAndCheckCPUName==true &&
                   CosFunction.bInShuttleDetectByLatch &&                       //Isaac 20170418 (Steven) 用Y Latch檢測Inshuttle有無IC
                   Prod.bF18InshuttleDetect &&
                   MOTION_CARD_TYPE==MotionCard_Contec &&
                   LastSet.iRealDummy==REALLY)                                  //jou 20170418 (Steven) : 矽品-世明要求修改config F18至Recipe設定
                {
                    if(b1ShuttleMoveToLeft==false &&
                       b1ShuttleMoveToRight==true)
                        bInSht1LtcDetectTesterCanMove=true;
                }

                if(b1ShuttleMoveToRight==false)
                {
                    b1ShuttleMoveToRight=true;                                  //確實移動到右邊
                    if(CosFunction.bUseAfterHomeShtChkLoseICNeedSlow &&         //Sam 20201020 : 回 Home 第一次的 Shuttle Check IC lose 需要變慢。
                       bAfterHomeShtChkLoseICNeedSlow[0])
                    {
                        SetMotorScaleSpeed(MInShuttle1, SHSpeed.iSH1Sp);
                        bAfterHomeShtChkLoseICNeedSlow[0]=false;
                    }

                    if(b1ShuttleMoveToLeft==true)                               //確實的從左邊移到右邊去
                    {
                        if(bReadAndCheckCPUName==true &&                        //Isaac 20170418 (Steven) 用Y Latch檢測Inshuttle有無IC
                           CosFunction.bInShuttleDetectByLatch &&
                           Prod.bF18InshuttleDetect &&
                           MOTION_CARD_TYPE==MotionCard_Contec &&
                           LastSet.iRealDummy==REALLY)                          //jou 20170418 (Steven) : 矽品-世明要求修改config F18至Recipe設定
                        {
                            ret=DoCheckShuttle1ICByLTC(true);
                            if(ret==K_RETRY)                                    //check by step
                            {
                                b1ShuttleMoveToLeft=false;
                                b1ShuttleMoveToRight=false;
                                Task=5000;
                                break;
                            }
                            bInSht1LtcDetectTesterCanMove=true;
                        }

                        if(CosFunction.bOutShtLoseICInArmAway)
                        {
                            bSht1LoseICErr=ShowOutputShuttleICStatus(0, false); //show IC 遺失
                            if(bSht1LoseICErr==false)
                            {
                                StartDetectMotorSensor(0);                      //清除
                            }
                        }
                        else
                        {
                            ShowOutputShuttleICStatus(0);
                            StartDetectMotorSensor(0);                          //清除
                        }

                        b1ShuttleMoveToLeft=false;
                        bSHTOfsChangeLeft[0]=false;                             //Sam 20230202 : 修正 Shuttle Left 移動完成後又被修改 Offset 導致 Hang up
                    }
                }

                if(CosFunction.bOutShtLoseICInArmAway)                          //JerryYang 20200519 out shuttle lose IC, in arm要讓位
                {
                    if(bSht1LoseICErr)
                    {
                        if(IndexAlarmInArmAway()==false)
                        {
                            break;
                        }
                        //bSht1LoseICErr=false;
                        ShowOutputShuttleICStatus(0);
                        StartDetectMotorSensor(0);                              //清除
                        b1ShuttleMoveToLeft=false;
                        bSht1LoseICErr=false;                                   //Jimmychiu 20230922 : stop index process when out shuttle alarm
                    }
                }

                if(bCheckShuttleFlag)
                {
                    MOT[MInShuttle1].ScanMotorStatus();
                    if(MOT[MInShuttle1].Led[iInposLed])
                        break;                                                  //還沒穩定

                    //ret=0 OK
                    //ret=1 NG
                    //ret=2 Wait outarm
                    ret=CheckICUnderShuttle1_9045(false);                       //Steven 20110314
                    if(ret==1)                                                  //判斷 IC 有沒放好 不要馬上Alarm
                    {
                        if(HWaitForShuttleFix1.Off())
                        {
                            ret=CheckICUnderShuttle1_9045(true);                //Steven 20110314
                            if(ret==2)
                                break;

                            bPickShuttleError=true;
                            break;
                        }
                        break;
                    }
                    else
                    {
                        bCheckShuttleFlag=false;
                    }
//                    break;
                }

                if(IniConfig.bHaveRotateShuttle==true &&                        //ChungHung 20110922 : 轉轉蝦頭要檢查有沒有轉頭 Check Sensor
                   IniConfig.bRotateShNeedCheck &&
                   TestIF.bRotateShuttle)
                {
                    if(Sen[SnRotateCheck].IsOn())                               //確認SnRotateCheck 是否正常
                    {
                        ShowErrorMessage("JAM0450", K_RETRY, MInShuttle1);      //In Shuttle 1 Rotate Fail!
                        fAllMotorHome=false;
                        iHome=1;
                        break;
                    }
                }

                if(bCancelErrorBin && bBinError[0])                             //kevin 20160725 error bin 發生
                {
                    for(int i=0; i<FLCarryKit.iMaxRow; i++)
                    {
                        for(int j=0; j<FLCarryKit.iMaxCol; j++)
                        {
                            FRCarryKit.SetItemData(i, j, NULL_IC);
                        }
                    }

                    Task=211;
                    break;
                }

                bCheckShuttleFlag=false;

                if(IniConfig.bD43IndexDropErrorCanRetryandSkip &&
                   bIndexPickErrShtStayRight1==true)                            //JerryYang 20181206 (Steven) : fix 啟用D43功能時,index arm pick up error後按retry可能發生hang up
                    break;

                if(USE_LASER_DISTANCE==2 && TestIF_File.bEnableOutShuttleFloatingCheck && FRCarryKit.HasIC())
                {
                    Task=250;
                }
                else
                {
                    Task=1;
                    fLtcSensor->ClearLtcSensor(0);                                  //Steven 20250728
                }
            }
            else
            {
                b1ShuttleMoveToRight=false;
            }
            break;
        case 211:
            if(OutArmSuck.NoIC())                                               //kevin 20160725 out arm 沒有ic
            {
                Task=212;
            }
            break;
        case 212:                                                               //kevin 20160725 記錄 Error Bin
            while(Flag==false)
            {
                Flag=MoveOutArmXY_ToShuttleAlarmArea();                         //JerryYang 20170214 (Steven) 修正多一個分號  //kevin 20160725
                if(MOT[MTrayX].ReadPos()>=Prod.iXTrayAuto[0])
                    Flag=true;
            }

            ret=ShowErrorMessage("WAR07356", K_SKIP, MMInterface, false, sArm1BinError);
            if(ret==K_SKIP)
            {
                bBinError[0]=false;
                bCheckShuttleFlag=false;
                Task=1;
            }
            break;
        case 250:
            if(OutArmSuck.HasIC()==false)
            {
                bOutArmLaserActionflag[0]=true;
                Task=251;
            }
            break;
        case 251:
            if(MoveOutArmToAutoSafe())
            {
                UseOutArmCheckShtFloating(0, true);
                Task=252;
            }
            break;
        case 252:
            if(UseOutArmCheckShtFloating(0, false))
            {
                bOutArmLaserActionflag[0]=false;
                Task=1;
            }
            break;
        case 300:
            if(IsTestZ1NotSafeShuttle1CanNotMove(iRetryCT))
                break;

            if(IniConfig.bF21InOutArmZMotorPrivate)
            {
                if(DoInOutARM_SHT_MoveSafe(0))                                  //kevin 20161005 SHUTTLE 1 移動安全保護
                    return;
            }
            iRetryCT=0;
            bCheckShuttleFlag=true;
            if(MOT[MInShuttle1].MotorMove(Prod.iBS1Right_Half)==1)
            {
                if(iOldTask!=Task)
                {
                    fLtcSensor->ClearLtcSensor(0);                              //Sam 20221101 : Latch 清除都要確認是否清清乾淨
                    iOldTask=Task;
                }

                if(bCheckShuttleFlag)
                {
                    MOT[MInShuttle1].ScanMotorStatus();
                    if(MOT[MInShuttle1].Led[iInposLed])
                        break;                                                  //還沒穩定
                    bCheckShuttleFlag=false;
                    Task=1;
                }
            }
            break;
        case 5000:
            if((Prod.bF18InshuttleDetect==true &&
                IniConfig.bF20InShuttleProminentDetect==false) ||               //jou 20170418 (Steven) : 矽品-世明要求修改config F18至Recipe設定
               (Prod.bF18InshuttleDetect==true &&
                CosFunction.bInShuttleDetectByLatch))                           //Isaac 20170418 (Steven) 用Y Latch檢測Inshuttle有無IC
            {
                if(DoCheckShuttle1MustHasIC())
                {
                    Task=200;
                    bF18CheckShuttle1MustHasIC=false;                           //JerryYang 20170515 (Steven) 修正in shuttle detect異常後關閉F18會hang up
                    RunOK=true;                                                 //JerryYang 20160809 修正開啟F18 hang up問題
                }
            }
            else if(IniConfig.bF20InShuttleProminentDetect==true)               //Alick 20160815 add for 力成
            {
                if(DoCheckShuttle1Prominent())
                {
                    Task=200;
                    bF18CheckShuttle1MustHasIC=false;                           //JerryYang 20170515 (Steven) 修正in shuttle detect異常後關閉F18會hang up
                    RunOK=true;
                }
            }
            else if(bF18CheckShuttle1MustHasIC==true &&                         //JerryYang 20170515 (Steven) 修正in shuttle detect異常後關閉F18會hang up
                    Prod.bF18InshuttleDetect==false)                            //jou 20170418 (Steven) : 矽品-世明要求修改config F18至Recipe設定
            {
                Task=200;
                bF18CheckShuttle1MustHasIC=false;
                RunOK=true;
            }
            else if(IniConfig.bF09CheckICWhichFirstTimeLoad)                    //Steven 20191205 : 修正 [F09]
            {
                if(DoCheckShuttle1MustHasIC())
                {
                    Task=200;
                }
            }

            if((BAR_CODE_INSTALL==ebctInShtIntel ||                             //Steven 20220803 : Fixed Shuttle Hang up
                BAR_CODE_INSTALL==ebctUseCCDMode ||
                BAR_CODE_INSTALL==ebctEtherNetCCD) &&
               TestIF_File.bEnableBarCode &&
               FLCarryKit.UseSiteHasIC())                                       //Ifor 20190129 : add Cognex EtherNet 通訊
            {
                if(BOTTOM_2DID && TestIF_File.bEnableBottom2D)                  //Steven 20190308 : Bottom 2D
                {
                }
                else
                {
                    if(Task==200)                                               //Already finish
                    {
//                        if(InSHT1InLF()==false)                               //JerryYang 20200722 : fix index arm下壓讓shuttle偏移INP亮燈造成沒掃2D就進去測試
                        if(fBarCode->IsSHT2DIDScanFinish(0)==false)
                        {
                            Task=5100;
                            break;
                        }
                    }
                }
            }
            break;
        case 5100:
            if(MOT[MInShuttle1].MotorMove(Prod.InSHT[0].iLeft)==1)
            {
                if(DoFloodGateClose()==true)                                    //Ifor 20260318 add:Shuttle1 Left Floodgate Close (case 5100)
                {
                    Cylinder[C_OutShuttle1Floodgate].On();
                }

                Task=200;
            }
            break;
        case 2300:                                                              //Steven 20160920 : IC置偏檢查
            if(CheckCFixTrayFullPlace())                                        //Steven 20220420 : 針對Fix3氣缸加上保護機制
            {
                if(bShuttleMoveToLeftforFix3 &&
                   FIX3_FULL_PLACE==Fix3K_UseCylinder)                          //Steven 20220517 : Add for Fix 3 Cyn
                {
                    Task=100;
                }
                return;
            }

            if(fBarCode->DoShuttleFloatCheck_1())
            {
                if(BAR_CODE_INSTALL==ebctOutShtAMD && FRCarryKit.UseSiteHasIC())
                {
                    if(InShtInLF(0))
                    {
                        fBarCode->InitialBarcodeScanOutShuttle1();
                        Task=2500;
                        break;
                    }
                }
                else if(BAR_CODE_INSTALL!=ebctUninstall &&
                        TestIF_File.bEnableBarCode &&
                        TestIF_File.bEnableBottom2D==false &&                   //Steven 20190308 : Bottom 2D
                        FLCarryKit.UseSiteHasIC())                              //Ifor 20190129 : add Cognex EtherNet 通訊
                {
                    if(BOTTOM_2DID && TestIF_File.bEnableBottom2D)              //Steven 20190308 : Bottom 2D
                    {
                    }
                    else
                    {
                        if(InShtInLF(0))
                        {
                            fBarCode->InitialBarcodeScanInShuttle1();
                            Task=2400;
                            break;
                        }
                    }
                }

                if(IniConfig.bHaveRotateShuttle==true &&                        //Steven 20110801 Start: 轉轉蝦頭要檢查有沒有轉頭
                   IniConfig.bRotateShNeedCheck &&
                   TestIF.bRotateShuttle &&
                   bCheckRotateSensor[0]==true)
                {
                    bDuplicateErr=false;
                    if(SThreadPara.bChechInShFirst[0])
                        Task=220;
                    else
                        Task=230;
                }
                else
                {
                    Task=210;
                }

                if(IniConfig.bIndexPickupWait==true &&                          //jou 2012-06-29 Index Pick up need wait Soak Time
                   (LastSet.iTemperature==Tempture_Hot ||
                    LastSet.iTemperature==Tempture_AmbientHot))                 //kevin 20180903 (Steven) : add 恆溫控制
                {
                    if(Temperature.iShuttleSoakTimeMode==0 &&
                       Temperature.iInitialStart2Time!=0)
                    {
                        FLCarryKit.TSoakTime.SetSecAndOn(Temperature.iInitialStart2Time);
                    }
                }
            }
            break;
        case 2500:
            if(CheckCFixTrayFullPlace())                                        //Steven 20220420 : 針對Fix3氣缸加上保護機制
            {
                if(bShuttleMoveToLeftforFix3 &&
                   FIX3_FULL_PLACE==Fix3K_UseCylinder)                          //Steven 20220517 : Add for Fix 3 Cyn
                {
                    Task=100;
                }
                return;
            }

            if(fBarCode->DoBarcodeScanOutShuttle_1()==true)                     //pig 2011.12.05 2DBarcode
            {
                Task=210;
            }
            break;
        case 2400:
            if(CheckCFixTrayFullPlace())                                        //Steven 20220420 : 針對Fix3氣缸加上保護機制
            {
                if(bShuttleMoveToLeftforFix3 &&
                   FIX3_FULL_PLACE==Fix3K_UseCylinder)                          //Steven 20220517 : Add for Fix 3 Cyn
                {
                    Task=100;
                }
                return;
            }

            if(BAR_CODE_INSTALL==ebctUseCCDMode)                                //Steven 20160106 : 改用CCD拍完就跑的方式
            {
                //==> Eastsun 20260526 #026-4.PinN.P-NN7 Pin1 OR :KYEC
                if((CosFunction.b2DUseSubJobFunction==true &&
                    TestIF_File.b2DUseSubJob==true) ||                          //Ifor 20200807 add:In House 2D Use Sub Job Function
                   (CosFunction.b2DUsePinInspection==true &&
                    TestIF_File.b2DUsePinInspection==true))                     //Ifor 20230207 add:In House 2D Use Pin1 Inspection Function
                //<== Eastsun 20260526 #026-4.PinN.P-NN7
                {
                    if(fBarCode->DoBarcodeScanInShuttle_1()==true)
                    {
                        Task=210;
                    }
                }
                else
                {
                    if(fBarCode->DoBarcodeCCDInShuttle_1()==true)
                    {
                        Task=210;
                    }
                }
            }
            else if(TestIF_File.b2DTriggerMode)                                 //Steven 20151225 : 改用拍完就跑的方式
            {
                if(fBarCode->DoBarcodeTriggerInShuttle_1()==true)
                {
                    Task=210;
                }
            }
            else
            {
                if(fBarCode->DoBarcodeScanInShuttle_1()==true)
                {
                    Task=210;
                }
            }
            break;
        case 2600:                                                              //Steven 20200909 : 修改Murata XML上傳資料
            if(ShuttleDelay1.Off())
            {
                Task=201;
                iHttpErrRetryCT++;
            }
            break;
    }
}
//------------------------------------------------------------------------------
bool IsTestZ2NotSafeShuttle2CanNotMove(int &iRetryCT)
{
    bool bflag=false;
    if(USE_INDEX_ARM_AXES==IndexArm_3_Axis)                                     //JimmyChiu 20220708 : add Index Arm Axis
    {
        bflag=MOT[MTestY1].Gali_ReadEncoderInRandge(Prod.TestY1_Middle);
    }
    else
    {
        bflag=MOT[MTestY2].Gali_ReadEncoderInRandge(Prod.TestY2_Rear);
    }

    if(bflag)
    {
        if(MOT[MTestZ2].Gali_ReadEncoderBelowCheckHeight(Prod.TestZ2_Safe))
        {
            MOT[MInShuttle2].PCIL132_StopMotor();
            if(MOT[MInShuttle2].IsCanMove())
                ShowIndexAndShuttlePositionNotCorrect(iRetryCT);
            return true;
        }

        if(TestIF_File.iShuttleMode==0 ||                                       //Ifor 20160310 add 蝦頭移動前判斷是否回Home
           (TestIF_File.iShuttleMode==1 && TestIF_File.iShuttle_Sel==1))
        {
            #ifndef SOFT_SIMULTE
            MOT[MTestZ2].ScanMotorStatus();
            if(MOT[MTestZ2].Led[iHomeLed]==false)
            {
                MOT[MInShuttle2].PCIL132_StopMotor();
                if(MOT[MInShuttle2].IsCanMove())
                    ShuttleBeforeMoveIndexZHomeNotOn(iRetryCT, 2);
                return true;
            }
            #endif
        }
    }
    return false;
}
//------------------------------------------------------------------------------
void Do_Auto_SHT2()
{
    static int iHttpErrRetryCT=0;
    static int iRetryCT=0;
    static int SH2X=0, SH2Y=0;                                                  //Sam 20210409 : 改為全域
    static int iOldTask=0;
    static bool RunOK2=false;                                                   //kevin 20141213 20140206
    static bool bMoveLeft=false;                                                //Steven 20220712 : reset shuttle speed for D43
    static bool bSpeedDown=false;
    static bool bDuplicateErr=false;
    static bool RunShuttleCheckIC2=false;                                       //kevin 20161108 確認shuttle 移出是否有 IC
    static bool bCheckRotateSensor[2]={false, false};                           //Steven 20110801

    bool bflag;

    if(IniConfig.bEnableTestingNeedStopAllMotor==true &&
       IniConfig.bI24TestingNeedStopAllMotor==true &&
       bTestingStopAllMotor==true)                                              //jou 2013-09-25 Testing Need Stop All Motor
    {
        return;
    }

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

    if(IniConfig.bF16CheckShuttleSensorBroken && bDoingF16)                     //Steven 20221213 : 確認shuttle 有沒有斷線
        return;

    if(SystemNG)
        return;

    int ret=-1;
    int pos=0;                                                                  //KenHsieh 20251118 : 修改Out Sht 會重新GetLtc 導致Lose IC 異常
    int &Task=AutoSHT2Task;
    int iIndex2YPos=0, iIndex2ZPos=0;                                           //Steven 20120509 : 吸取異常跟Shuttle殘料同時發生會Hang Up
    bool &bCheckShuttleFlag=bCheckShuttle2Flag, Flag=false;
    bool bRun2DBarcode=false, bHasICErr=false;                                  //Ifor 20200907 add:Dummy Run 不執行2D Barcode
    AnsiString str;
    AnsiString sErrPart="";

    switch(Task)
    {
        case 1:
            if(MOT[MInShuttle2].IsCanMove())
            {
                if(In_Shuttle_Auto_Latch==eInSHAutoLtc)                         //KenHsieh 20250722 : InSht sensor 改為2顆，並用Latch 判別疊料以及飛料
                {
                    if(bInSh2DoLtc==true)
                        break;
                    pos=MOT[MInShuttle2].ReadPos();                             //KenHsieh 20251118 : 修改Out Sht 會重新GetLtc 導致Lose IC 異常
                    if(pos==Prod.InSHT[1].iLeft)
                        bNeedGetSHRightLtc[1]=true;
                }

                if((bShuttle2MoveToLeft &&                                      //ChungHung 20120717 add Index Drop Error Can Retry and Start
                    IniConfig.bD43IndexDropErrorCanRetryandSkip) ||
                   (bShuttleMoveToLeftforFix3 &&
                    FIX3_FULL_PLACE==Fix3K_UseCylinder))                        //ChungHung 20140313 add Fix3 can Full Tray
                {
                    bMoveLeft=true;                                             //Steven 20220712 : reset shuttle speed for D43
                    Task=100;
                    return;
                }
                else if(bShuttle2MoveToRight &&
                        IniConfig.bD43IndexDropErrorCanRetryandSkip)
                {
                    Task=202;                                                   //Steven 20150414 : Index吸取異常,Shuttle退出後,要檢查置偏
                    return;
                }
                Task=10;
            }
            break;
        case 10:
            if((iOneCycle && IsInArmOneCycleFinish()) ||
               (iCleanOut && IsInArmCleanOutFinish()))
            {
                if(BRCarryKit.UseSiteHasIC())
                {
                    if(IniConfig.bF22InShuttleDetectOutNoIC)                    //kevin 20161108
                    {
                        InitialCheckShuttle2ProminentNoHasICTask();
                        RunShuttleCheckIC2=true;                                //kevin 20161108 In shuttle 有IC進入測區
                    }

                    ret=CheckOneCycleAction(0);                                 //Steven 20240326 : 判斷one cycle的時候要不要繼續放料
                    if(ret!=0)
                    {
                        break;
                    }
                    Task=200;
                }
                else if(BTestSuck.UseSiteHasIC())
                {
                    Task=100;
                }
                else if(BLCarryKit.UseSiteHasIC())
                {
                    if(CanYieldAlarmRemainInSHT()==true)                        //JerryYang 20220923 : yield alarm時觸發half one cycle(shuttle保留IC不測試跳ONE CYCLE FINISH)
                    {
                        Task=100;
                        break;
                    }

                    if(IniConfig.bP17InArmFullPickFromLoader &&
                       IsBLCarrKitAllHasIC()==false)                            //Steven 20220301 : 修正32site One Cycle Hang Up
                    {
                        if(LastSet.iTemperature==Tempture_Hot)
                        {
                            if(MOT[MMPlate1].HasIC() ||
                               MOT[MMPlate2].HasIC())
                                break;
                        }
                        else
                        {
                            if(MOT[MMTrayY_Car].HasIC() &&
                               MOT[MMTrayY].HasIC())                            //Sam 20210601 : 增加保護
                                break;
                        }
                    }
                    Task=200;
                }
                else
                {
                    Task=100;
                }
            }
            else
            {
                if(BTestSuck.UseSiteNoIC())
                {
                    bool bOK=InArmFullPickforTestMode();                        //模式太多,將判斷式包成函式好問讀
                    if(IniConfig.bP17InArmFullPickFromLoader && bOK)            //ChungHung 20111230 add Shuttle必須放滿
                    {
                        if(IsBLCarrKitAllHasIC())
                        {
                            if(IniConfig.bF22InShuttleDetectOutNoIC)            //kevin 20161108
                            {
                                InitialCheckShuttle2ProminentNoHasICTask();
                                RunShuttleCheckIC2=true;                        //kevin 20161108 In shuttle 有IC進入測區
                            }

                            if(bCheckShuttle2MustHasIC &&
                               IniConfig.bF09CheckICWhichFirstTimeLoad)
                            {
                                bCheckShuttle2MustHasIC=false;
                                InitialCheckShuttle2MustHasICTask();
                                InitialCheckShuttle2ProminentTask();
                                Task=5000;
                            }
                            else
                            {
                                if((CUSTOMER_CODE==CC_SCC ||                    //jou 2011-03-24 start : Auto Site Mapping
                                    CUSTOMER_CODE==CC_SCK ||
                                    CUSTOMER_CODE==CC_AMKOR_Korea) &&
                                   IniConfig.bI21AutoSiteMappingUseHotplate==false)
                                {
                                    if(LastSet.iTemperature==Tempture_Hot &&    //Eliot 2011_0209 start
                                       LastSet.iRunStartMode==rsmAutoSiteMap &&
                                       bSiteMappingCHKOK==false)
                                    {
                                        if(IniConfig.bI21SkipSoakTime==true)
                                        {
                                            Task=200;
                                        }
                                        else if(IniConfig.bI21UseSameSoakTime==true)
                                        {
                                            if(BLCarryKit.HotCount>=IniConfig.fI21UseSameSoakTime)
                                                Task=200;
                                        }
                                        else if(BLCarryKit.HotCount>=Prod.iHotTime)
                                        {
                                            Task=200;
                                        }
                                    }
                                    else
                                    {
                                        Task=200;
                                    }
                                }
                                else
                                {
                                    Task=200;
                                }
                            }
                        }
                        else
                        {
                            Task=100;
                        }
                    }
                    else
                    {
                        if(BLCarryKit.UseSiteHasIC())
                        {
                            if(IniConfig.bF22InShuttleDetectOutNoIC)            //kevin 20161108
                            {
                                InitialCheckShuttle2ProminentNoHasICTask();
                                RunShuttleCheckIC2=true;                        //kevin 20161108 In shuttle 有IC進入測區
                            }

                            if(bCheckShuttle2MustHasIC &&
                               IniConfig.bF09CheckICWhichFirstTimeLoad)
                            {
                                bCheckShuttle2MustHasIC=false;
                                InitialCheckShuttle2MustHasICTask();
                                InitialCheckShuttle2ProminentTask();
                                Task=5000;
                            }
                            else
                            {
                                if(IniConfig.bUseAutoSiteMapping &&             //jou 2011-03-24 start : Auto Site Mapping
                                   IniConfig.bI21AutoSiteMappingUseHotplate==false)
                                {
                                    if(LastSet.iTemperature==Tempture_Hot &&    //Eliot 2011_0209 start
                                       LastSet.iRunStartMode==rsmAutoSiteMap &&
                                       bSiteMappingCHKOK==false)
                                    {
                                        if(IniConfig.bI21SkipSoakTime==true)
                                        {
                                            Task=200;
                                        }
                                        else if(IniConfig.bI21UseSameSoakTime==true)
                                        {
                                            if(BLCarryKit.HotCount>=IniConfig.fI21UseSameSoakTime)
                                                Task=200;
                                        }
                                        else if(BLCarryKit.HotCount>=Prod.iHotTime)
                                        {
                                            Task=200;
                                        }
                                    }
                                    else
                                    {
                                        if(IsBLCarrKitAllHasIC()==false &&      //Steven 20160530 : Fix 2D 8 device no trigger  //Jimmychiu 20231130 : 避免跑16sites，shuttle放一半回HOME造成Hang up
                                           TestIF_File.iTestMode!=SingleSite)   //Ifor 20260310 fix:Single Site 2D first device hang up
                                            break;
                                        Task=200;
                                    }
                                }
                                else
                                {
                                    if(IsBLCarrKitAllHasIC()==false &&          //Steven 20160530 : Fix 2D 8 device no trigger
                                       TestIF_File.iTestMode!=SingleSite)       //Ifor 20260310 fix:Single Site 2D first device hang up
                                        break;
                                    Task=200;
                                }
                            }
                        }
                        else
                        {
                            Task=100;
                        }
                    }
                }
                else
                {
                    if(BRCarryKit.UseSiteNoIC())
                    {
                        Task=100;
                    }
                    else if((bShuttle2MoveToLeft &&
                             IniConfig.bD43IndexDropErrorCanRetryandSkip) ||
                            (bShuttleMoveToLeftforFix3 &&
                             FIX3_FULL_PLACE==Fix3K_UseCylinder))               //JerryYang 20160906 修正hang up, shuttle沒有移動到左邊讓fix移動
                    {
                        bMoveLeft=true;                                         //Steven 20220712 : reset shuttle speed for D43
                        Task=100;
                    }
                    else
                    {
                        if(IsBLCarrKitAllHasIC()==false &&                      //Steven 20160530 : Fix 2D 8 device no trigger
                           TestIF_File.iTestMode!=SingleSite)                   //Ifor 20260310 fix:Single Site 2D first device hang up
                            break;
                        Task=200;
                    }
                }
            }
            break;
        case 105:                                                               //jou 981125 start : 防止Index內疊烏龜
            if(CheckNullICShuttle2_9045(false))
            {
                Task=100;                                                       //PASS
            }
            else
            {
                if(HWaitForShuttleFix2.Off())
                {
                    CheckNullICShuttle2_9045(true);                             //FAIL
                }
            }
            break;
        case 100:                                                               //Steven 20110801 Start: 轉轉蝦頭要檢查有沒有轉頭
            if(IniConfig.bHaveRotateShuttle==true &&
               IniConfig.bRotateShNeedCheck &&
               TestIF.bRotateShuttle &&
               bCheckRotateSensor[1]==true)
            {
                bDuplicateErr=false;
                Task=130;
            }
            else
            {
                if(SHSpeed.iStepSH && BLCarryKit.UseSiteNoIC() && b2ShuttleMoveToLeft==false)
                {
                    Task=140;
                    iStepShuttleTask[1]=1;
                }
                else
                {
                    Task=120;
                }
            }
            break;
        case 130:
            if(IsTestZ2NotSafeShuttle2CanNotMove(iRetryCT))
                break;
            iRetryCT=0;
            bCheckShuttleFlag=true;

            if(MachineTypeChoice==Type_HT9045)
            {
                if(IniConfig.bF11OutShtUseFrontRearSensor &&                    //jou 981125 start : 防止Index內疊烏龜
                   bCheckNullIC2 &&
                   MOT[MInShuttle2].fCanMoveR &&
                   MOT[MInShuttle2].fCanMoveM)
                {
                    if(CheckNullICShuttle2_9045(false)==false)
                    {
                        HWaitForShuttleFix2.SetSecAndOn(1);
                        Task=105;
                        break;
                    }
                    bCheckNullIC2=false;
                }
            }

            if(b2ShuttleMoveToLeft==false)                                      //jou 2010-06-04 start : 防止Index內疊烏龜
                bShuttle2CatchSignalL=true;

            if(IniConfig.bF21InOutArmZMotorPrivate)
            {
                if(DoInOutARM_SHT_MoveSafe(1))                                  //kevin 20161005 SHUTTLE 1 移動安全保護
                    return;
            }

            if(MOT[MInShuttle2].MotorMove(SThreadPara.iOShRotateToLeftCheck[1])==1)
            {
                RotShuttleDelay2.SetSecAndOn(IniConfig.dRoShCheckDelayTime);
                Task=124;
            }
            break;
        case 124:
            if(RotShuttleDelay2.Off())
            {
                if(Tech.OutSH2ZOneRowDetectPos!=0 &&
                   Sen[SnOutPutSHT2S1].Enable==true &&
                   Sen[SnOutPutSHT2S1].IsOn())
                {
                    Task=130;
                    ShowErrorMessage("JAM0551", K_RETRY, MOutShuttle2, bDuplicateErr);  //Out Shuttle 2 Rotate Fail!
                    bDuplicateErr=true;
                }
                else
                {
                    bCheckRotateSensor[1]=false;
                    Task=135;
                    bDuplicateErr=false;
                }
            }
            break;
        case 135:
            if(IsTestZ2NotSafeShuttle2CanNotMove(iRetryCT))
                break;
            iRetryCT=0;
            bCheckShuttleFlag=true;

            if(MachineTypeChoice==Type_HT9045)
            {
                if(IniConfig.bF11OutShtUseFrontRearSensor &&                    //jou 981125 start : 防止Index內疊烏龜
                   bCheckNullIC2 &&
                   MOT[MInShuttle2].fCanMoveR &&
                   MOT[MInShuttle2].fCanMoveM)
                {
                    if(CheckNullICShuttle2_9045(false)==false)
                    {
                        HWaitForShuttleFix2.SetSecAndOn(1);
                        Task=105;
                        break;
                    }
                    bCheckNullIC2=false;
                }
            }

            if(b2ShuttleMoveToLeft==false)                                      //jou 2010-06-04 start : 防止Index內疊烏龜
                bShuttle2CatchSignalL=true;

            if(IniConfig.bF21InOutArmZMotorPrivate)
            {
                if(DoInOutARM_SHT_MoveSafe(1))                                  //kevin 20161005 SHUTTLE 1 移動安全保護
                    return;
            }

            if(MOT[MInShuttle2].MotorMove(Prod.InSHT[1].iLeft)==1)
            {
                if(DoFloodGateClose()==true)                                    //Ifor 20260318 add:Shuttle2 Left Floodgate Close (case 135)
                {
                    Cylinder[C_OutShuttle2Floodgate].On();
                }

                RotShuttleDelay2.SetSecAndOn(IniConfig.dRoShCheckDelayTime);
                Task=125;
            }
            break;
        case 125:
            if(RotShuttleDelay2.Off())
            {
                if(IniConfig.bHaveRotateShuttle==true &&
                   IniConfig.bRotateShNeedCheck   &&
                   TestIF.bRotateShuttle          &&
                   Tech.OutSH2ZOneRowDetectPos!=0 &&
                   Sen[SThreadPara.iInShSenIndex[1][0]].IsOn())
                {
                    ShowErrorMessage("JAM0451", K_RETRY, MInShuttle2, bDuplicateErr);   //In Shuttle 2 Rotate Fail!
                    bDuplicateErr=true;
                    Task=135;
                }
                else
                {
                    Task=120;
                    bDuplicateErr=false;
                }
            }
            break;
        case 120:
            if(IsTestZ2NotSafeShuttle2CanNotMove(iRetryCT))
                break;
            iRetryCT=0;

            if(IniConfig.bF22InShuttleDetectOutNoIC && RunShuttleCheckIC2)      //kevin 20161108
            {
                if(BLCarryKit.HasRealIC()==false)                               //Steven 20180402 (Jou) : 修改判斷式
                {
                    Task=115;
                    RunShuttleCheckIC2=false;                                   //kevin 20161108
                    return;
                }
            }

            if(MachineTypeChoice==Type_HT9045)
            {
                if(IniConfig.bF11OutShtUseFrontRearSensor &&                    //jou 981125 start : 防止Index內疊烏龜
                   bCheckNullIC2 &&
                   MOT[MInShuttle2].fCanMoveR &&
                   MOT[MInShuttle2].fCanMoveM)
                {
                    if(CheckNullICShuttle2_9045(false)==false)
                    {
                        HWaitForShuttleFix2.SetSecAndOn(1);
                        Task=105;
                        break;
                    }
                    bCheckNullIC2=false;
                }
            }

            if(b2ShuttleMoveToLeft==false)                                      //jou 2010-06-04 start : 防止Index內疊烏龜
                bShuttle2CatchSignalL=true;

            if(IniConfig.bF21InOutArmZMotorPrivate)
            {
                if(DoInOutARM_SHT_MoveSafe(1))                                  //kevin 20161005 SHUTTLE 1 移動安全保護
                    return;
            }

            if(IniConfig.bD43IndexDropErrorCanRetryandSkip)                     //Steven 20220712 : reset shuttle speed for D43
            {
                if(bShuttle2MoveToLeft && bMoveLeft==true)
                {
                    SetMotorScaleSpeed(MInShuttle2, 10);
                    bMoveLeft=false;
                }
            }

            if(MOT[MInShuttle2].MotorMove(Prod.InSHT[1].iLeft)==1)
            {
                if(DoFloodGateClose()==true)                                    //Ifor 20260318 add:Shuttle2 Left Floodgate Close
                {
                    Cylinder[C_OutShuttle2Floodgate].On();
                }

                if(IniConfig.bD43IndexDropErrorCanRetryandSkip)                 //Steven 20220712 : reset shuttle speed for D43
                {
                    if(bShuttle2MoveToLeft)
                    {
                        SetMotorScaleSpeed(MInShuttle2, SHSpeed.iSH2Sp);
                    }
                }
                bMoveLeft=false;

                RunOK2=false;                                                   //JerryYang 20160809 修正開啟F18 hang up問題
                bCheckShuttleFlag=true;                                         //Steven 20220301 : 換位置

                if(CosFunction.bInShuttleDetectByLatch &&                       //Isaac 20170418 (Steven) 用Y Latch檢測Inshuttle有無IC
                   Prod.bF18InshuttleDetect &&
                   MOTION_CARD_TYPE==MotionCard_Contec &&
                   LastSet.iRealDummy==REALLY)                                  //jou 20170418 (Steven) : 矽品-世明要求修改config F18至Recipe設定
                {
                    ret=DoCheckShuttle1ICByLTC(false);
                    if(ret==K_RETRY)                                            //check by step
                    {
                        b1ShuttleMoveToLeft=false;
                        b1ShuttleMoveToRight=false;
                        Task=5000;
                        break;
                    }
                    bInSht1LtcDetectTesterCanMove=true;
                }

                if(IniConfig.bHaveRotateShuttle==true &&
                   IniConfig.bRotateShNeedCheck   &&
                   TestIF.bRotateShuttle          &&                            //Steven 20110801 : 轉轉蝦頭要檢查有沒有轉頭
                   Tech.OutSH2ZOneRowDetectPos!=0 &&
                   Sen[SThreadPara.iInShSenIndex[1][0]].IsOn())
                {
                    ShowErrorMessage("JAM0451", K_RETRY, MInShuttle1, bDuplicateErr); //In Shuttle 2 Rotate Fail!
                    bDuplicateErr=true;
                    break;
                }
                else
                {
                    bDuplicateErr=false;
                }

                if(iOldTask!=Task)
                {
                    bShuttle2CatchSignalL=false;
                    fLtcSensor->GetLtcSensor(1);                                //Steven 20110526 : 取得往左移動的Latch
                    iOldTask=Task;
                }

                if(bSpeedDown==true)
                    SetMotorScaleSpeed(MInShuttle2, SHSpeed.iSH2Sp);            //Steven 20110607 : 重新檢查時要放慢速度, 檢查完要還原速度

                if(IniConfig.bIndexJamInArmAway)                                //ChungHung 20110302 start
                {                                                               //kevin 20180504 move up for hang up
                    if(bInArmNeedToSafePos)
                    {
                        break;
                    }
                }

                if(bShuttle2Pause==true)                                        //jou 990104 start : add from ht9080a
                {
                    bShuttle2Pause=false;
                    bShowShuttle2Device=false;                                  //kevin 20180504 index pick up error
//                    bAutoCleanCheckOpenDoor=true;                             //JerryYang 20190625 mark掉  //kevin 20180725 (wei) add 掉料忽視需開門 Alarm 後需要開門確認
                    bHasICErr=false;
                    sErrPart="";
                    if(IniConfig.bD81IndexCheckVacuumOnShuttle)
                    {
                        for(int i=0; i<MAX_Index_Row; i++)
                        {
                            for(int j=0; j<NEW_MAX_Index_Col; j++)
                            {
                                if(bSHT2_DropPosHasIC[i][j]==true)
                                {
                                    bHasICErr=true;
                                    bSHT2_DropPosHasIC[i][j]=false;
                                    if(TestIF_File.iTestMode==_32Site4X8M ||
                                        TestIF_File.iTestMode==_32Site4X8N ||
                                        TestIF_File.iTestMode==_16Site4X4)                        //Sam 20190226 : 16Site4X4 ///kevin 20180504 add  error pos
                                        sErrPart+=IndexSuckName[i+2][j];
                                    else if(TestIF_File.iTestMode==QualSite2X2N ||
                                            TestIF_File.iTestMode==_6Site2X3N)
                                        sErrPart+=IndexSuckName[i+1][j];                     //Steven 20230712 : 修正NN mode alarm顯示
                                    else
                                        sErrPart+=IndexSuckName[i][j];
                                }
                            }
                        }
                    }

                    if(bHasICErr && bSht2OnlyHasICErr)
                    {
                        str.sprintf("Input arm droped device in Shuttle2, need remove device of %s", sErrPart);
                        if(IniConfig.bAlarmNeedServoOff)    //Steven 20110802
                            ShowMyMessage(str, "","", false, true);
                        else
                            ShowMyMessage(str, "");
                    }
                    else
                    {
                        if(bHasICErr)
                        {
                            str.sprintf("Input arm droped device in Shuttle2, need remove device of %s", sErrPart);
                            if(IniConfig.bAlarmNeedServoOff)    //Steven 20110802
                                ShowMyMessage(str, "","", false, true);
                            else
                                ShowMyMessage(str, "");
                        }

                        if(IniConfig.bAlarmNeedServoOff)                            //Steven 20110802
                            ShowMyMessage("Test Arm 2 Pick up error after Skip, need take away shuttle 2 device [D42]", "Test Arm 2 吸取錯誤並且Skip, 需要拿走 shuttle 2 device [D42]", "", false, true);
                        else
                            ShowMyMessage("Test Arm 2 Pick up error after Skip, need take away shuttle 2 device [D42]", "Test Arm 2 吸取錯誤並且Skip, 需要拿走 shuttle 2 device [D42]");
                    }

                    fMain->Pause("Do_Auto_SHT2");
                }

                if(IniConfig.bF28IndexCheckShuttlePos)                          //kevin 20220512 add  Index check shuttle pos for Sensor
                {
                    if(Sen[SnSht2Left].Enable && Sen[SnSht2Left].IsOff())
                    {
                        ShowMyMessage("Shuttle 2 Left detect sensor on error");
                        break;
                    }
                }

                if(b2ShuttleMoveToLeft==false)
                {
                    if(b2ShuttleMoveToRight==true)                              //jou 2010-06-04 start : 防止Index內疊烏龜
                    {
                        b2ShuttleMoveToRight=false;
                        bSHTOfsChangeRight[1]=false;                            //Sam 20221205 : 修正 Shuttle 移動完成後又被修改 Offset 導致 Hang up
                        if(TestIF.iShuttleMode==0 ||
                           (TestIF.iShuttleMode==1 && TestIF.iShuttle_Sel==1))  //Jou 20110421
                        {
                            if(SThreadPara.bExeShuttleThread==false)
                            {
                                if(CheckShuttle2OutputHasICError_M204())
                                {
                                    Task=400;
                                    break;
                                }
                            }
                            else
                            {
                                if(CheckShuttleOutputHasICError(1, SH2X, SH2Y))
                                {
                                    if(fMain->cbShowShuttleSensor->Checked==true)
                                    {
                                        AnsiString sFileName;
                                        sFileName.sprintf("%04d-%02d-%02d %02d:%02d:%02d.%d has error retry:%d", SystemYear, SystemMonth, SystemDate, SystemHour, SystemMin, SystemSec, SystemMSec, iOutShuttle2HasICErrRetryCnt);  //Sam 20210409 : 改為全域
                                        fMain->AddShuttleMessage(1, sFileName);
                                    }
                                    Task=400;
                                    break;
                                }
                            }
                        }
                    }
                    iOutShuttle2HasICErrRetryCnt=0;                             //Sam 20210409 : 改為全域
                }

                b2ShuttleMoveToLeft=true;                                       //確實移動到左邊
                StartDetectMotorSensor(1);

                if(In_Shuttle_Auto_Latch==eInSHAutoLtc &&
                   bNeedGetSHRightLtc[1]==false)                                //KenHsieh 20251119 : In SHLtc 後，馬上從右到左Latch 須清除，避免會造成Out Lose IC 誤判
                {
                    fLtcSensor->ClearLtcSensor(1);
                    fLtcSensor->SetLtcSensor(1);
                }

                if(IniConfig.bHaveRotateShuttle==true && IniConfig.bRotateShNeedCheck && TestIF.bRotateShuttle)   //Steven 20110801 : 轉轉蝦頭要檢查有沒有轉頭
                {
                    bCheckRotateSensor[0]=true;
                    if(IniConfig.bRotateShNeedCheck &&                          //ChungHung 20110922 : 轉轉蝦頭要檢查有沒有轉頭 Check Sensor
                       TestIF.bRotateShuttle)
                    {
                        if(Sen[SnRotateCheck].IsOn())                           //確認SnRotateCheck 是否正常
                        {
                            ShowErrorMessage("JAM0451", K_RETRY, MInShuttle2);  //In Shuttle 2 Rotate Fail!
                            fAllMotorHome=false;
                            iHome=1;
                            break;
                        }
                    }
                }
                Task=1;
                fLtcSensor->ClearLtcSensor(1);                                  //Steven 20250728
                bShtMoveTimerRunning2=false;                                    //Steven 20260507 : arrived, clear watchdog
            }
            else
            {
                b2ShuttleMoveToLeft=false;

                if(!bShtMoveTimerRunning2)                                      //Steven 20260507 : MotorMove not arrived, start watchdog
                {
                    ShtMoveTimeout2.SetSecAndOn(SHUTTLE_MOVE_TIMEOUT_SEC);
                    bShtMoveTimerRunning2=true;
                }
                else if(ShtMoveTimeout2.Off())                                 //Steven 20260507 : watchdog expired
                {
                    bShtMoveTimerRunning2=false;
                    DoShtMoveTimeoutHandle(1);                                  //AI(ht9045-staterecord-analysis) 20260430 (RogerYang) : trigger Timeout handler
                }
            }
            break;
        case 115:                                                               //kevin 20161108 in shuttle 退出是否有IC 殘留檢測
            if(DoCheckShuttle2EmptyIC())
            {
                Task=120;
            }
            break;                                                              //Steven 20090907
        case 110:                                                               //Steven 20110603 : 重新檢查前,多等待一下
            if(ShuttleDelay2.Off())
            {
                bSpeedDown=true;
                SetMotorScaleSpeed(MInShuttle2, iShuttleSpeed);                 //Steven 20110607 : 重新檢查時要放慢速度
                Task=100;
            }
            break;
        case 140:
            if(IsTestZ2NotSafeShuttle2CanNotMove(iRetryCT))
                break;
            iRetryCT=0;
            bCheckShuttleFlag=true;

            if(MachineTypeChoice==Type_HT9045)
            {
                if(IniConfig.bF11OutShtUseFrontRearSensor &&                    //jou 981125 start : 防止Index內疊烏龜
                   bCheckNullIC2 &&
                   MOT[MInShuttle2].fCanMoveR &&
                   MOT[MInShuttle2].fCanMoveM)
                {
                    if(CheckNullICShuttle2_9045(false)==false)
                    {
                        HWaitForShuttleFix2.SetSecAndOn(1);
                        Task=105;
                        break;
                    }
                    bCheckNullIC2=false;
                }
            }

            if(b2ShuttleMoveToLeft==false)                                      //jou 2010-06-04 start : 防止Index內疊烏龜
                bShuttle2CatchSignalL=true;

            if(DoStepShuttleCheck(1))
            {
                Task=120;
            }
            break;
        case 400:
            if(IniConfig.bF21InOutArmZMotorPrivate)
            {
                if(DoInOutARM_SHT_MoveSafe(1))                                  //kevin 20161005 SHUTTLE 1 移動安全保護
                    return;
            }

            if(USE_INDEX_ARM_AXES==IndexArm_4_Axis)                             //JimmyChiu 20251101 : add Index Arm Axis
            {
                iIndex2YPos=MOT[MTestY2].Gali_ReadPos();                        //Steven 20120509 : 吸取異常跟Shuttle殘料同時發生會Hang Up
                bflag=(iIndex2YPos!=Prod.TestY2_Rear);                          //ChungHung 20150520 fix hangup 遇到iIndex2ZPos差一條的情況
            }
            else
            {
                bflag=true;
            }

            iIndex2ZPos=MOT[MTestZ2].Gali_ReadPos();
            if(MOT[MInShuttle2].IsCanMove() &&
               InArmZSafe(DETECT_SENSOR_FLAG)==-1 &&
               (iIndex2ZPos>=Prod.TestZ2_Safe-3 ||
                bflag))
            {
                if(MOT[MInShuttle2].MotorMove(Prod.InSHT[1].iRight-1000)==1)       //故意讓位置不到位，所以-1000
                {
                    if(iOldTask!=Task)
                    {
                        fLtcSensor->ClearLtcSensor(1);                          //Sam 20221101 : Latch 清除都要確認是否清清乾淨
                        iOldTask=Task;
                    }

                    iOutShuttle2HasICErrRetryCnt++;
                    if(iOutShuttle2HasICErrRetryCnt>=2)                         //Sam 20210409 : 改為全域
                    {
                        Task=500;
                        break;
                    }
                    else
                    {
                        fLtcSensor->SetLtcSensor(1);                            //JerryYang 20230406 : 重置shuttle1的latch
                        fLtcSensor->ClearLtcSensor(1);                          //Sam 20221101 : Latch 清除都要確認是否清清乾淨
                        ret=K_RETRY;
                    }

                    if(ret==K_RETRY)
                    {
                        b2ShuttleMoveToRight=true;
                        b2ShuttleMoveToLeft=false;
                        StartDetectMotorSensor(1);
                    }

                    Task=110;                                                   //Steven 20110603
                    ShuttleDelay2.SetSecAndOn(0.5);
                }
            }
            else
            {
                MOT[MInShuttle2].PCIL132_StopMotor();
            }
            break;
        case 500:
            if(SThreadPara.bExeShuttleThread==false)
            {
                if(iHWFix_BinBox==1 && OutArmSuck.NoIC())                       //kevin 20170214 (wei) out arm 沒有ic 移到等待點方便取出 error bin box ic
                {
                    while(Flag==false)
                    {
                        Flag=MoveOutArmXY_ToShuttleAlarmArea();                 //kevin 20160725
                        if(MOT[MTrayX].ReadPos()>=Prod.iXTrayAuto[0])
                            Flag=true;
                    }
                }

                if(CUSTOMER_CODE==CC_ASE_KaohSiung      ||
                   CUSTOMER_CODE==CC_ASE_KaohSiung_K12  ||                      //Steven 20131101 : Add ASE-K12
                   CUSTOMER_CODE==CC_SCS                ||                      //Steven 20110209
                   CUSTOMER_CODE==CC_AMKOR_Japan        ||                      //Steven 20160225 : J-Device Out Shuttle殘料只能Retry
                   CUSTOMER_CODE==CC_ASE_SG)                                    //Ifor 20200219 :add ASE SG Out Shuttle殘料只能Retry
                {
                    ret=ShowErrorMessage("JAM0570", K_RETRY, MOutShuttle2, false, SThreadPara.ErrPartSh2);    //Jou 2011-02-08
                }
                else if(AccessLevel<iDefHonPrecLevel)                           //Steven 20110210 : Device Superfloat可以Skip //jou 2014-06-19 Security Have 5 Level 3->iDefHonPrecLevel
                {
                    ret=ShowErrorMessage("JAM0570", K_RETRY, MOutShuttle2, false, SThreadPara.ErrPartSh2);    //Jou 2011-02-08
                }
                else
                {
                    ret=ShowErrorMessage("JAM0570", K_RETRY|K_SKIP, MOutShuttle2, false, SThreadPara.ErrPartSh2);
                }
            }
            else
            {
                EndShuttleThread();                                             //jou 2011-02-17

                if(CUSTOMER_CODE==CC_ASE_KaohSiung      ||
                   CUSTOMER_CODE==CC_ASE_KaohSiung_K12  ||                      //Steven 20131101 : Add ASE-K12
                   CUSTOMER_CODE==CC_SCS                ||                      //Steven 20110209
                   CUSTOMER_CODE==CC_AMKOR_Japan        ||                      //Steven 20160225 : J-Device Out Shuttle殘料只能Retry
                   CUSTOMER_CODE==CC_ASE_SG)                                    //Ifor 20200219 :add ASE SG Out Shuttle殘料只能Retry
                {
                    ret=ShowErrorMessage("JAM0570", K_RETRY, MOutShuttle2, false, IndexSuckName[SH2Y][SH2X]);    //Steven 20111114 改
                }
                else if(TestIF.iTestMode==DualSite2x1)
                {
                    AnsiString ErrPart=" ";
                    ErrPart+=IndexSuckName[0][0];
                    ErrPart+=IndexSuckName[1][0];
                    ret=ShowErrorMessage("JAM0570", K_RETRY, MOutShuttle2, false, ErrPart);    //Steven 20111114 改
                }
                else if(AccessLevel<iDefHonPrecLevel)                           //Steven 20110210 : Device Superfloat可以Skip //jou 2014-06-19 Security Have 5 Level 3->iDefHonPrecLevel
                {
                    ret=ShowErrorMessage("JAM0570", K_RETRY, MOutShuttle2, false, IndexSuckName[SH2Y][SH2X]);    //Steven 20111114 改
                }
                else
                {
                    ret=ShowErrorMessage("JAM0570", K_RETRY|K_SKIP, MOutShuttle2, false, IndexSuckName[SH2Y][SH2X]);    //Steven 20111114 改
                }
                StartShuttleThread();                                           //jou 2011-02-17
            }

            iOutShuttle2HasICErrRetryCnt=0;                                     //Sam 20210409 : 改為全域
            if(ret==K_RETRY)
            {
                b2ShuttleMoveToRight=true;
                b2ShuttleMoveToLeft=false;
                StartDetectMotorSensor(1);
            }
            Task=110;                                                           //Steven 20110603
            ShuttleDelay2.SetSecAndOn(0.5);
            break;
        case 202:                                                               //Steven 20150414 : Index吸取異常,Shuttle退出後,要檢查置偏
            if(In_Shuttle_Auto_Latch==eInSHAutoLtc)                             //KenHsieh 20250722 : InSht sensor 改為2顆，並用Latch 判別疊料以及飛料
            {
                InitAutoChkInSHLatchTask();
                Task=203;
            }
            else if(CheckShuttleSensor_9045(2))
            {
                Task=200;
            }
            break;
        case 203:                                                               //KenHsieh 20251104 : add 檢測後動作 & alarm 變數
            if(CheckInShuttleSensor_Latch(1, 0, true)==1)
                Task=200;
            break;
        case 200:
            if(In_Shuttle_Auto_Latch==eInSHAutoLtc &&
               bInSh2DoLtc==true)                                               //KenHsieh 20250722 : InSht sensor 改為2顆，並用Latch 判別疊料以及飛料
                break;

            if(TestIF.iShuttleMode==1 && TestIF.iShuttle_Sel==0)                //只用Shuttle 1
            {
                Task=201;
            }
            else if((USE_LASER_DISTANCE && TestIF_File.bEnableShuttleLaser) ||
                    (USE_LASER_DISTANCE==2 && TestIF_File.bEnableInShuttleFloatingCheck))      //Steven 20140228 : 雷射測距功能
            {
                if(InShtInRT(1))                                                //Isaac 20201217 : 若齒輪比大於1，換算有機會和目標位置差1條
                {
                    Task=201;
                }
                else
                {
                    if(AutoSHT1Task==3000 ||
                      (USE_LASER_DISTANCE==2 && TestIF_File.bEnableInShuttleFloatingCheck && AutoSHT1Task==3100))   //互卡
                        break;

                    if(USE_LASER_DISTANCE==2 && TestIF_File.bEnableInShuttleFloatingCheck)
                    {
                        Task=3100;
                    }
                    else
                    {
                        CheckShtFloating(0, true);
                        Task=3000;
                    }
                    break;
                }
            }
            else
            {
                Task=201;
            }
        case 201:
            if(SHT_FLOATING_CHK==1 &&
               TestIF_File.bEnableShtFloatChk)                                  //Steven 20160920 : IC置偏檢查
            {
                if(InSHT2InLF()==false)                                         //JerryYang 20200722 : fix index arm下壓讓shuttle偏移INP亮燈造成沒掃2D就進去測試
                {
                    break;
                }
                fBarCode->InitialShuttleFloatCheck2();
                Task=2300;
                break;
            }

            if(CUSTOMER_CODE==CC_AMD_M &&                                       //pig 2011.12.05 2DBarcode start
               LastSet.iRealDummy!=REALLY)
            {
                bRun2DBarcode=false;                                            //Ifor 20200907 add:Dummy Run 不執行2D Barcode
            }
            else
            {
                bRun2DBarcode=true;                                             //Ifor 20200907 add:Dummy Run 不執行2D Barcode
            }

            if(BAR_CODE_INSTALL!=ebctUninstall && bResetMode==false && bRun2DBarcode==true)//Steven 20201125 : RESET時, 不需要掃2DID
            {
                /*if(bShuttle2MoveToRight &&
                     IniConfig.bD43IndexDropErrorCanRetryandSkip)               //Steven 20160802 : fixed for duplicate 2DID after index pick up error
                {
                    Task=210;
                }
                else*/                                                          //Steven 20160804 : Sck Hur說一定要重掃, 所以改方法
                if(BAR_CODE_INSTALL==ebctOutShtAMD && BRCarryKit.UseSiteHasIC())
                {
                    if(InSHT2InLF()==false)                                     //JerryYang 20200722 : fix index arm下壓讓shuttle偏移INP亮燈造成沒掃2D就進去測試
                    {
                        break;
                    }
                    fBarCode->InitialBarcodeScanOutShuttle2();
                    Task=2500;
                    break;
                }
                else if(BAR_CODE_INSTALL!=ebctUninstall &&
                        TestIF_File.bEnableBarCode &&
                        BLCarryKit.UseSiteHasIC())                              //Ifor 20190129 : add Cognex EtherNet 通訊 //Ifor 20210407 add: 自製OCR
                {
                    if(BOTTOM_2DID && TestIF_File.bEnableBottom2D)              //Steven 20190308 : Bottom 2D
                    {
                        ret=SendHTTPRequest(1);                                 //Steven 20200804 : 修正bottom 2D不會回傳資料給server的問題
                        if(ret==-1)                                             //Steven 20200909 : 修改Murata XML上傳資料
                        {
                            str.sprintf("Do Shuttle 2 SendHTTPRequest fail retry count: %d", iHttpErrRetryCT);
                            RecordProcess(str);
                            if(iHttpErrRetryCT>3)
                            {
                                ret=ShowErrorMessage("WAR16320", K_RETRY|K_RESET, MInShuttle2, "Do_Auto_SHT2");    //Steven 20201113 : 上傳失敗3次要有Alarm
                                iHttpErrRetryCT=0;
                                if(ret==K_RESET)
                                {
                                    fMain->Reset("Do_Auto_SHT2");
                                    Task=210;                                   //Steven 20201125 : RESET時, 不需要掃2DID
                                }
                                else
                                {
                                    Task=2600;
                                    ShuttleDelay2.SetSecAndOn(30);
                                }
                                break;
                            }
                            else
                            {
                                Task=2600;
                                ShuttleDelay2.SetSecAndOn(30);
                                break;
                            }
                        }
                        else
                        {
                            iHttpErrRetryCT=0;
                        }
                    }
                    else
                    {
                        if(fBarCode->IsSHT2DIDScanFinish(1)==false)             //JerryYang 20200916 : fix hang up, 還沒掃2D才要在左邊
                        {
                            if(InSHT2InLF()==false)                             //JerryYang 20200722 : fix index arm下壓讓shuttle偏移INP亮燈造成沒掃2D就進去測試
                            {
                                break;
                            }
                            fBarCode->InitialBarcodeScanInShuttle2();
                            Task=2400;
                            break;
                        }
                    }
                }
            }

            if(IniConfig.bHaveRotateShuttle==true &&                            //Steven 20110801 Start: 轉轉蝦頭要檢查有沒有轉頭
               IniConfig.bRotateShNeedCheck &&
               TestIF.bRotateShuttle &&
               bCheckRotateSensor[0]==true)                                     //Steven 20110801 : 轉轉蝦頭要檢查有沒有轉頭
            {
                bDuplicateErr=false;
                if(SThreadPara.bChechInShFirst[1])
                    Task=220;
                else
                    Task=230;
            }
            else
            {
                Task=210;
            }

            if(IniConfig.bIndexPickupWait==true &&
               LastSet.iTemperature==Tempture_Hot)                              //jou 2012-06-29 Index Pick up need wait Soak Time
            {
                if(Temperature.iShuttleSoakTimeMode==0 &&
                   Temperature.iInitialStart2Time!=0)
                {
                    BLCarryKit.TSoakTime.SetSecAndOn(Temperature.iInitialStart2Time);
                }
            }
            break;
        case 220:
            if(DeviceForm.bShuttleWaitingOutSiteChamber &&                      //jou 980930 start : Shuttle Waiting Out Site Chamber
               LastSet.iTemperature==Tempture_Hot)
            {
                if(fFrontNeedTest)
                    break;
            }

            if(IsTestZ2NotSafeShuttle2CanNotMove(iRetryCT))
                break;
            iRetryCT=0;

            if(InSHT2InRT())
            {
                bCheckShuttleFlag=true;
                HWaitForShuttleFix2.Set0_1SecAndOn(10);
            }

            if(b2ShuttleMoveToRight==false)
                bShuttle2CatchSignalR=true;

            if(IniConfig.bF21InOutArmZMotorPrivate)
            {
                if(DoInOutARM_SHT_MoveSafe(1))                                  //kevin 20161005 SHUTTLE 1 移動安全保護
                    return;
            }

            if(MOT[MInShuttle2].MotorMove(SThreadPara.iInShRotateCheck[1])==1)
            {
                RotShuttleDelay2.SetSecAndOn(IniConfig.dRoShCheckDelayTime);
                Task=225;
            }
            break;
        case 225:
            if(RotShuttleDelay2.Off())
            {
                if(Tech.iInSH2Sen7DetectPos!=0 &&
                   Sen[SThreadPara.iInShuttleSen7[1]].Enable==true &&
                   Sen[SThreadPara.iInShuttleSen7[1]].IsOn())
                {
                    Task=220;
                    ShowErrorMessage("JAM0451", K_RETRY, MInShuttle2, bDuplicateErr);   //In Shuttle 2 Rotate Fail!
                    bDuplicateErr=true;
                }
                else
                {
                    bDuplicateErr=false;
                    if(SThreadPara.bChechInShFirst[1])
                    {
                        Task=230;                                               //In先，所以要到Out
                    }
                    else
                    {
                        bCheckRotateSensor[0]=false;
                        Task=210;
                    }
                }
            }
            break;
        case 230:
            if(DeviceForm.bShuttleWaitingOutSiteChamber &&                      //jou 980930 start : Shuttle Waiting Out Site Chamber
               LastSet.iTemperature==Tempture_Hot)
            {
                if(fFrontNeedTest)
                    break;
            }

            if(IsTestZ2NotSafeShuttle2CanNotMove(iRetryCT))
                break;
            iRetryCT=0;

            if(InSHT2InRT())
            {
                bCheckShuttleFlag=true;
                HWaitForShuttleFix2.Set0_1SecAndOn(10);
            }

            if(b2ShuttleMoveToRight==false)
                bShuttle2CatchSignalR=true;

            if(IniConfig.bF21InOutArmZMotorPrivate)
            {
                if(DoInOutARM_SHT_MoveSafe(1))                                  //kevin 20161005 SHUTTLE 1 移動安全保護
                    return;
            }

            if(MOT[MInShuttle2].MotorMove(SThreadPara.iOShRotateToRightCheck[1])==1)
            {
                RotShuttleDelay2.SetSecAndOn(IniConfig.dRoShCheckDelayTime);
                Task=235;
            }
            break;
        case 235:
            if(RotShuttleDelay2.Off())
            {
                if(Tech.OutSH2ZOneRowDetectPos!=0 &&
                   Sen[SnOutPutSHT2S1].Enable==true &&
                   Sen[SnOutPutSHT2S1].IsOn())
                {
                    Task=230;
                    ShowErrorMessage("JAM0551", K_RETRY, MOutShuttle2, bDuplicateErr);  //Out Shuttle 2 Rotate Fail!
                    bDuplicateErr=true;
                }
                else
                {
                    bDuplicateErr=false;
                    if(SThreadPara.bChechInShFirst[1])
                    {
                        bCheckRotateSensor[0]=false;
                        Task=210;
                    }
                    else
                    {
                        Task=220;                                               //Out先，所以要到In
                    }
                }
            }
            break;
        case 210:
            if(CheckCFixTrayFullPlace())                                        //Steven 20220420 : 針對Fix3氣缸加上保護機制
            {
                if(bShuttleMoveToLeftforFix3 &&
                   FIX3_FULL_PLACE==Fix3K_UseCylinder)                          //Steven 20220517 : Add for Fix 3 Cyn
                {
                    Task=100;
                }
                return;
            }

            if(BLCarryKit.HasDefineIC(HAS_NULL_CLEAN_IC) ||                     //JerryYang 20220923 : 增加保護, 一般流程時shuttle資料不該有auto clean data
               BLCarryKit.HasDefineIC(HAS_CLEAN_IC) ||
               BLCarryKit.HasDefineIC(CLEAN_FINISH_IC))
            {
                ShowMyMessage("Shuttle 2 has clean pad data, please re-start the progarm and clear all device in machince.", "Shuttle 2殘留clean pad資料, 請重開程式並清機");
                break;
            }

            if((Prod.bF18InshuttleDetect ||
                IniConfig.bF20InShuttleProminentDetect) &&                      //jou 20170418 (Steven) : 矽品-世明要求修改config F18至Recipe設定
               (RunOK2==false && LastSet.iRealDummy==REALLY) &&                 //kevin 20141212  20140206 input SHUTTLE 第9顆sensor 進入偵測是否有ic)
               CosFunction.bInShuttleDetectByLatch==false)                      //Isaac 20170418 (Steven) 用Y Latch檢測Inshuttle有無IC
            {
                for(int i=0; i<BLCarryKit.iShtRow; i++)
                {
                    for(int j=0; j<BLCarryKit.iShtCol; j++)
                    {
                        if(BLCarryKit.Item[i][j]!=NULL_IC)                      //JerryYang 20241104 : HAS NULL IC也要檢查確實無IC殘留
                        {
                            Task=5000;
                            bF18CheckShuttle2MustHasIC=true;                    //JerryYang 20170515 (Steven) 修正in shuttle detect異常後關閉F18會hang up
                            return;
                        }
                    }
                }
            }

            if(DeviceForm.bShuttleWaitingOutSiteChamber &&                      //Steven 20110801 End: 轉轉蝦頭要檢查有沒有轉頭
               LastSet.iTemperature==Tempture_Hot)                              //jou 980930 start : Shuttle Waiting Out Site Chamber
            {
                if(fFrontNeedTest)
                    break;
            }

            if(IniConfig.bD44CheckIndexICDestroy &&
               iIndexTakeDeviceChk2!=0)                                         //kevin 20190103 add 確認回吸check ok
                break;
            if(IsTestZ2NotSafeShuttle2CanNotMove(iRetryCT))
                break;

            iRetryCT=0;
//            if(ishuttleShake==1 && iShakeShuttleTask==1)                      //JerryYang 20190421 Mark掉, shuttle shake的時候這裡不會進來      //kevin 20190107 shuttle 2 error
//                break;

            if(InShtInRT(1)==false)                                             //Isaac 20201217 : 若齒輪比大於1，換算有機會和目標位置差1條
            {
                bCheckShuttleFlag=true;
                HWaitForShuttleFix2.SetSecAndOn(1);
            }

            if(b2ShuttleMoveToRight==false)
                bShuttle2CatchSignalR=true;

            if(IniConfig.bF21InOutArmZMotorPrivate)
            {
                if(DoInOutARM_SHT_MoveSafe(1))                                  //kevin 20161005 SHUTTLE 1 移動安全保護
                    return;
            }

            if(bReadAndCheckCPUName==true &&
               CosFunction.bInShuttleDetectByLatch &&                           //Isaac 20170418 (Steven) 用Y Latch檢測Inshuttle有無IC
               Prod.bF18InshuttleDetect &&
               MOTION_CARD_TYPE==MotionCard_Contec &&
               LastSet.iRealDummy==REALLY)                                      //jou 20170418 (Steven) : 矽品-世明要求修改config F18至Recipe設定
            {
                bInSht2LtcDetectTesterCanMove=false;
            }
            else
            {
                bInSht2LtcDetectTesterCanMove=true;
            }

            if(CosFunction.bUseAfterHomeShtChkLoseICNeedSlow &&                 //Sam 20201020 : 回 Home 第一次的 Shuttle Check IC lose 需要變慢。
               bAfterHomeShtChkLoseICNeedSlow[1] &&
               BRCarryKit.UseSiteHasIC())                                       //確認 OutShuttle 有 IC 才需要變慢
            {
                SetMotorScaleSpeed(MInShuttle2, iShuttleSpeed);
            }

            if(MOT[MInShuttle2].MotorMove(Prod.InSHT[1].iRight)==1)
            {
                if(DoFloodGateClose()==true)                                    //Ifor 20260318 add:Shuttle2 Right Floodgate Close
                {
                    Cylinder[C_Shuttle2Floodgate].On();
                }

                if(IniConfig.bHaveRotateShuttle==true &&
                   IniConfig.bRotateShNeedCheck &&
                   TestIF.bRotateShuttle)                                       //Steven 20110801 : 轉轉蝦頭要檢查有沒有轉頭
                {
                    bCheckRotateSensor[1]=true;
                }

                if(iOldTask!=Task)
                {
                    if(In_Shuttle_Auto_Latch!=eInSHAutoLtc ||
                      (In_Shuttle_Auto_Latch==eInSHAutoLtc &&
                       bNeedGetSHRightLtc[1]))                                  //KenHsieh 20260225 : 更改判斷位置，避免iOldTask 沒更新，導致到左邊沒判斷 //KenHsieh 20251118 : 修改Out Sht 會重新GetLtc 導致Lose IC 異常
                        fLtcSensor->GetLtcSensor(1);                            //Sam 20230308 : 修正Latch Sensor

                    iOldTask=Task;
                }

                if(IniConfig.bF28IndexCheckShuttlePos)                          //kevin 20220512 add  Index check shuttle pos for Sensor
                {
                    if(Sen[SnSht2Right].Enable && Sen[SnSht2Right].IsOff())
                    {
                        ShowMyMessage("Shuttle 2 right detect sensor on error");
                        break;
                    }
                }
                bShuttle2CatchSignalR=false;

                bCheckNullIC2=true;                                             //jou 981125 start : 防止Index內疊烏龜

                if(bReadAndCheckCPUName==true &&
                   CosFunction.bInShuttleDetectByLatch &&                       //Isaac 20170418 (Steven) 用Y Latch檢測Inshuttle有無IC
                   Prod.bF18InshuttleDetect &&
                   MOTION_CARD_TYPE==MotionCard_Contec &&
                   LastSet.iRealDummy==REALLY)                                  //jou 20170418 (Steven) : 矽品-世明要求修改config F18至Recipe設定
                {
                    if(b2ShuttleMoveToLeft==false &&
                       b2ShuttleMoveToRight==true)
                        bInSht2LtcDetectTesterCanMove=true;
                }

                if(b2ShuttleMoveToRight==false)
                {
                    b2ShuttleMoveToRight=true;                                  //確實移動到右邊

                    if(CosFunction.bUseAfterHomeShtChkLoseICNeedSlow &&         //Sam 20201020 : 回 Home 第一次的 Shuttle Check IC lose 需要變慢。
                       bAfterHomeShtChkLoseICNeedSlow[1])                       //確認 OutShuttle 有 IC 才需要變慢
                    {
                        SetMotorScaleSpeed(MInShuttle2, SHSpeed.iSH2Sp);
                        bAfterHomeShtChkLoseICNeedSlow[1]=false;
                    }

                    if(b2ShuttleMoveToLeft==true)                               //確實的從左邊移到右邊去
                    {
                        if(bReadAndCheckCPUName==true &&                        //Isaac 20170418 (Steven) 用Y Latch檢測Inshuttle有無IC
                           CosFunction.bInShuttleDetectByLatch &&
                           Prod.bF18InshuttleDetect &&
                           MOTION_CARD_TYPE==MotionCard_Contec &&
                           LastSet.iRealDummy==REALLY)                          //jou 20170418 (Steven) : 矽品-世明要求修改config F18至Recipe設定
                        {
                            ret=DoCheckShuttle2ICByLTC(true);
                            if(ret==K_RETRY)
                            {
                                b2ShuttleMoveToLeft=false;
                                b2ShuttleMoveToRight=false;
                                Task=5000;                                      //step check
                                break;
                            }
                            bInSht2LtcDetectTesterCanMove=true;
                        }

                        if(CosFunction.bOutShtLoseICInArmAway)
                        {
                            bSht2LoseICErr=ShowOutputShuttleICStatus(1, false); //show IC 遺失
                            if(bSht2LoseICErr==false)
                            {
                                StartDetectMotorSensor(1);                      //清除
                            }
                        }
                        else
                        {
                            ShowOutputShuttleICStatus(1);
                            StartDetectMotorSensor(1);                          //清除
                        }

                        b2ShuttleMoveToLeft=false;
                        bSHTOfsChangeLeft[1]=false;                             //Sam 20230202 : 修正 Shuttle Left 移動完成後又被修改 Offset 導致 Hang up
                    }
                }

                if(CosFunction.bOutShtLoseICInArmAway)                          //JerryYang 20200519 out shuttle lose IC, in arm要讓位
                {
                    if(bSht2LoseICErr)
                    {
                        if(IndexAlarmInArmAway()==false)
                        {
                            break;
                        }
                        bSht2LoseICErr=false;
                        ShowOutputShuttleICStatus(1);
                        StartDetectMotorSensor(1);                              //清除
                        b2ShuttleMoveToLeft=false;
                    }
                }

                if(bCheckShuttleFlag)
                {
                    MOT[MInShuttle2].ScanMotorStatus();
                    if(MOT[MInShuttle2].Led[iInposLed])
                        break;                                                  //還沒穩定

                    //ret=0 OK
                    //ret=1 NG
                    //ret=2 Wait outarm
                    ret=CheckICUnderShuttle2_9045(false);                       //Steven 20110314
                    if(ret==1)                                                  //判斷 IC 有沒放好 不要馬上Alarm
                    {
                        if(HWaitForShuttleFix2.Off())
                        {
                            ret=CheckICUnderShuttle2_9045(true);                //Steven 20110314 //要馬上發出Alarm
                            if(ret==2)
                                break;

                            bPickShuttleError=true;
                        }
                        break;
                    }
                    else
                    {
                        bCheckShuttleFlag=false;
                        break;
                    }
                }

                if(IniConfig.bHaveRotateShuttle==true &&                        //ChungHung 20110922 : 轉轉蝦頭要檢查有沒有轉頭 Check Sensor
                   IniConfig.bRotateShNeedCheck &&
                   TestIF.bRotateShuttle)
                {
                    if(Sen[SnRotateCheck].IsOn())                               //確認SnRotateCheck 是否正常
                    {
                        ShowErrorMessage("JAM0451", K_RETRY, MInShuttle2);      //In Shuttle 2 Rotate Fail!
                        fAllMotorHome=false;
                        iHome=1;
                        break;
                    }
                }

                if(bCancelErrorBin && bBinError[1])                             //kevin 20160725 error bin 發生
                {
                    for(int i=0; i<BLCarryKit.iMaxRow; i++)
                    {
                        for(int j=0; j<BLCarryKit.iMaxCol; j++)
                        {
                            BRCarryKit.SetItemData(i, j, NULL_IC);
                        }
                    }
                    Task=211;
                    break;
                }

                bCheckShuttleFlag=false;
                if(IniConfig.bD43IndexDropErrorCanRetryandSkip &&
                   bIndexPickErrShtStayRight2==true)                            //JerryYang 20181206 (Steven) : fix 啟用D43功能時,index arm pick up error後按retry可能發生hang up
                    break;

                if(USE_LASER_DISTANCE==2 && TestIF_File.bEnableOutShuttleFloatingCheck && BRCarryKit.HasIC())
                {
                    Task=250;
                }
                else
                {
                    Task=1;
                    fLtcSensor->ClearLtcSensor(1);                                  //Steven 20250806 : 0 --> 1
                }
            }
            else
            {
                b2ShuttleMoveToRight=false;
            }
            break;
        case 211:
            if(OutArmSuck.NoIC())                                               //kevin 20160725 out arm 沒有ic
            {
                Task=212;
            }
            break;
        case 212:                                                               //kevin 20160725 記錄 Error Bin
            while(Flag==false)
            {
                Flag=MoveOutArmXY_ToShuttleAlarmArea();                         //JerryYang 20170214 (Steven) 修正多一個分號  //kevin 20160725
                if(MOT[MTrayX].ReadPos()>=Prod.iXTrayAuto[0])
                    Flag=true;
            }
            ret=ShowErrorMessage("WAR07356", K_SKIP, MMInterface, false, sArm2BinError);
            if(ret==K_SKIP)
            {
                bBinError[1]=false;
                bCheckShuttleFlag=false;
                Task=1;
            }
            break;
        case 250:
            if(OutArmSuck.HasIC()==false)
            {
                bOutArmLaserActionflag[1]=true;
                Task=251;
            }
            break;
        case 251:
            if(MoveOutArmToAutoSafe())
            {
                UseOutArmCheckShtFloating(1, true);
                Task=252;
            }
            break;
        case 252:
            if(UseOutArmCheckShtFloating(1, false))
            {
                bOutArmLaserActionflag[1]=false;
                Task=1;
            }
            break;
        case 300:
            if(IsTestZ2NotSafeShuttle2CanNotMove(iRetryCT))
                break;
            if(IniConfig.bF21InOutArmZMotorPrivate)
            {
                if(DoInOutARM_SHT_MoveSafe(1))                                  //kevin 20161005 SHUTTLE 1 移動安全保護
                    return;
            }
            iRetryCT=0;
            bCheckShuttleFlag=true;
            if(MOT[MInShuttle2].MotorMove(Prod.iBS2Right_Half)==1)
            {
                if(iOldTask!=Task)
                {
                    fLtcSensor->GetLtcSensor(1);                                //Steven 20110526 : 清除Latch
                    iOldTask=Task;
                }

                if(bCheckShuttleFlag)
                {
                    MOT[MInShuttle2].ScanMotorStatus();
                    if(MOT[MInShuttle2].Led[iInposLed])                         //還沒穩定
                        break;
                    bCheckShuttleFlag=false;
                    Task=1;
                }
            }
            break;
        case 5000:
            if((Prod.bF18InshuttleDetect==true &&
                IniConfig.bF20InShuttleProminentDetect==false) ||               //jou 20170418 (Steven) : 矽品-世明要求修改config F18至Recipe設定
               (Prod.bF18InshuttleDetect==true &&
                CosFunction.bInShuttleDetectByLatch))                           //Isaac 20170706 (Steven) 用Y Latch檢測Inshuttle有無IC
            {
                if(DoCheckShuttle2MustHasIC())                                  //JerryYang 20160906 修正F18動作異常 DoCheckShuttle1MustHasIC --> DoCheckShuttle2MustHasIC
                {
                    Task=200;
                    bF18CheckShuttle2MustHasIC=false;                           //JerryYang 20170515 (Steven) 修正in shuttle detect異常後關閉F18會hang up
                    RunOK2=true;                                                //JerryYang 20160809 修正開啟F18 hang up問題
                }
            }
            else if(IniConfig.bF20InShuttleProminentDetect==true)               //Alick 20160815 add for 力成
            {
                if(DoCheckShuttle2Prominent())                                  //JerryYang 20160906 DoCheckShuttle1Prominent() --> DoCheckShuttle2MustHasIC()
                {
                    Task=200;
                    bF18CheckShuttle2MustHasIC=false;                           //JerryYang 20170515 (Steven) 修正in shuttle detect異常後關閉F18會hang up
                    RunOK2=true;
                }
            }
            else if(bF18CheckShuttle2MustHasIC==true &&                         //JerryYang 20170515 (Steven) 修正in shuttle detect異常後關閉F18會hang up
                    Prod.bF18InshuttleDetect==false)                            //jou 20170418 (Steven) : 矽品-世明要求修改config F18至Recipe設定
            {
                Task=200;
                bF18CheckShuttle2MustHasIC=false;
                RunOK2=true;
            }
            else if(IniConfig.bF09CheckICWhichFirstTimeLoad)                    //Steven 20191205 : 修正 [F09]
            {
                if(DoCheckShuttle2MustHasIC())
                {
                    Task=200;
                }
            }

            if((BAR_CODE_INSTALL==ebctInShtIntel ||                             //Steven 20220803 : Fixed Shuttle Hang up
                BAR_CODE_INSTALL==ebctUseCCDMode ||
                BAR_CODE_INSTALL==ebctEtherNetCCD) &&                           //Ifor 20190129 : add Cognex EtherNet 通訊
                TestIF_File.bEnableBarCode &&
                BLCarryKit.UseSiteHasIC())
            {
                if(BOTTOM_2DID && TestIF_File.bEnableBottom2D)                  //Steven 20190308 : Bottom 2D
                {
                }
                else
                {
                    if(Task==200)                                               //Already finish
                    {
//                        if(InSHT2InLF()==false)                               //JerryYang 20200722 : fix index arm下壓讓shuttle偏移INP亮燈造成沒掃2D就進去測試
                        if(fBarCode->IsSHT2DIDScanFinish(1)==false)
                        {
                            Task=5100;
                            break;
                        }
                    }
                }
            }
            break;
        case 5100:
            if(MOT[MInShuttle2].MotorMove(Prod.InSHT[1].iLeft)==1)
            {
                if(DoFloodGateClose()==true)                                    //Ifor 20260318 add:Shuttle2 Left Floodgate Close (case 5100)
                {
                    Cylinder[C_OutShuttle2Floodgate].On();
                }

                Task=200;
            }
            break;
        case 2300:                                                              //Steven 20160920 : IC置偏檢查
            if(CheckCFixTrayFullPlace())                                        //Steven 20220420 : 針對Fix3氣缸加上保護機制
            {
                if(bShuttleMoveToLeftforFix3 &&
                   FIX3_FULL_PLACE==Fix3K_UseCylinder)                          //Steven 20220517 : Add for Fix 3 Cyn
                {
                    Task=100;
                }
                return;
            }

            if(fBarCode->DoShuttleFloatCheck_2())
            {
                if(BAR_CODE_INSTALL==ebctOutShtAMD && BRCarryKit.UseSiteHasIC())
                {
                    if(InShtInLF(1))                                            //Isaac 20201217 : 若齒輪比大於1，換算有機會和目標位置差1條
                    {
                        fBarCode->InitialBarcodeScanOutShuttle2();
                        Task=2500;
                        break;
                    }
                }
                else if(BAR_CODE_INSTALL!=ebctUninstall &&
                        TestIF_File.bEnableBarCode &&
                        TestIF_File.bEnableBottom2D==false &&                   //Steven 20190308 : Bottom 2D
                        BLCarryKit.UseSiteHasIC())                              //Ifor 20190129 : add Cognex EtherNet 通訊
                {
                    if(BOTTOM_2DID && TestIF_File.bEnableBottom2D)              //Steven 20190308 : Bottom 2D
                    {
                    }
                    else
                    {
                        if(InShtInLF(1))                                        //Isaac 20201217 : 若齒輪比大於1，換算有機會和目標位置差1條
                        {
                            fBarCode->InitialBarcodeScanInShuttle2();
                            Task=2400;
                            break;
                        }
                    }
                }

                if(IniConfig.bHaveRotateShuttle==true &&                        //Steven 20110801 Start: 轉轉蝦頭要檢查有沒有轉頭
                   IniConfig.bRotateShNeedCheck &&
                   TestIF.bRotateShuttle &&
                   bCheckRotateSensor[1]==true)
                {
                    bDuplicateErr=false;
                    if(SThreadPara.bChechInShFirst[1])
                        Task=220;
                    else
                        Task=230;
                }
                else
                {
                    Task=210;
                }

                if(IniConfig.bIndexPickupWait==true &&
                   LastSet.iTemperature==Tempture_Hot)                          //jou 2012-06-29 Index Pick up need wait Soak Time
                {
                    if(Temperature.iShuttleSoakTimeMode==0 &&
                       Temperature.iInitialStart2Time!=0)
                    {
                        BLCarryKit.TSoakTime.SetSecAndOn(Temperature.iInitialStart2Time);
                    }
                }
            }
            break;
        case 2500:                                                              //pig 2011.12.05 2DBarcode
            if(CheckCFixTrayFullPlace())                                        //Steven 20220420 : 針對Fix3氣缸加上保護機制
            {
                if(bShuttleMoveToLeftforFix3 &&
                   FIX3_FULL_PLACE==Fix3K_UseCylinder)                          //Steven 20220517 : Add for Fix 3 Cyn
                {
                    Task=100;
                }
                return;
            }

            if(fBarCode->DoBarcodeScanOutShuttle_2()==true)
            {
                Task=210;
            }
            break;
        case 2400:
            if(CheckCFixTrayFullPlace())                                        //Steven 20220420 : 針對Fix3氣缸加上保護機制
            {
                if(bShuttleMoveToLeftforFix3 &&
                   FIX3_FULL_PLACE==Fix3K_UseCylinder)                          //Steven 20220517 : Add for Fix 3 Cyn
                {
                    Task=100;
                }
                return;
            }

            if(BAR_CODE_INSTALL==ebctUseCCDMode)                                //Steven 20160106 : 改用CCD拍完就跑的方式
            {
                //==> Eastsun 20260526 #026-4.PinN.P-NN8 Pin1 OR :KYEC
                if((CosFunction.b2DUseSubJobFunction==true && TestIF_File.b2DUseSubJob==true) || //Ifor 20200807 add:In House 2D Use Sub Job Function
                   (CosFunction.b2DUsePinInspection==true && TestIF_File.b2DUsePinInspection==true)) //Ifor 20230207 add:In House 2D Use Pin1 Inspection Function
                //<== Eastsun 20260526 #026-4.PinN.P-NN8
                {
                    if(fBarCode->DoBarcodeScanInShuttle_2()==true)
                    {
                        Task=210;
                    }
                }
                else
                {
                    if(fBarCode->DoBarcodeCCDInShuttle_2()==true)
                    {
                        Task=210;
                    }
                }
            }
            else if(TestIF_File.b2DTriggerMode)                                 //Steven 20151225 : 改用拍完就跑的方式
            {
                if(fBarCode->DoBarcodeTriggerInShuttle_2()==true)
                {
                    Task=210;
                }
            }
            else
            {
                if(fBarCode->DoBarcodeScanInShuttle_2()==true)
                {
                    Task=210;
                }
            }
            break;
        case 3000:
            if(CheckCFixTrayFullPlace())                                        //Steven 20220420 : 針對Fix3氣缸加上保護機制
            {
                if(bShuttleMoveToLeftforFix3 &&
                   FIX3_FULL_PLACE==Fix3K_UseCylinder)                          //Steven 20220517 : Add for Fix 3 Cyn
                {
                    Task=100;
                }
                return;
            }

            if(CheckShtFloating(1))
            {
                Task=201;
            }
            break;
        case 3100:
            if(InArmSuck.HasIC()==false)
            {
                bInArmLaserActionflag[1]=true;
                Task=3101;
            }
            break;
        case 3101:
            if(MoveInArmZToPlateSafe(3101))
            {
                UseInArmCheckShtFloating(1, true);
                Task=3102;
            }
            break;
        case 3102:
            if(UseInArmCheckShtFloating(1))
            {
                bInArmLaserActionflag[1]=false;
                Task=201;
            }
            break;
        case 2600:                                                              //Steven 20200909 : 修改Murata XML上傳資料
            if(ShuttleDelay2.Off())
            {
                Task=201;
                iHttpErrRetryCT++;
            }
            break;
    }
}
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//  W6.5: uShuttleThread offline driver (was Win32 HThreadCtrlShuttle).  No real
//  OS thread is spawned; the wrapper methods are inert.  The sensor-poll LOGIC
//  (ThreadProcess) below is kept VERBATIM and stays callable.  Win32 tokens come
//  from acarry_shims.h.  SThreadPara.bExeShuttleThread defaults false offline.
//------------------------------------------------------------------------------
typedef void (*PF_PROCESS)(LPVOID);
class HThreadCtrlShuttle
{
    private:
        HANDLE      _handle;
        DWORD       _id;
        LPVOID      _parameter;
        bool    _execute;
    protected:
        static DWORD CALLBACK ThreadProcess(LPVOID lpParameter);

    public:
        HThreadCtrlShuttle();
        ~HThreadCtrlShuttle();

        void Initial(LPVOID Param);
        int OpenThread();
        int CloseThread(int iTimeOut=INFINITE);
};
//------------------------------------------------------------------------------
HThreadCtrlShuttle ShuttleThread;
HThreadCtrlShuttle::HThreadCtrlShuttle()
{
    _handle = NULL;
    _id = 0;
    _parameter = NULL;
    _execute = false;
}
//------------------------------------------------------------------------------
HThreadCtrlShuttle::~HThreadCtrlShuttle()
{
    // offline: no OS thread to close
}
//------------------------------------------------------------------------------
void HThreadCtrlShuttle::Initial(LPVOID Param)
{
    // offline driver: record the parameter, do NOT spawn a thread
    _parameter=Param;
}
//------------------------------------------------------------------------------
int HThreadCtrlShuttle::OpenThread()
{
    // offline driver: no real OS thread; SMs take the bExeShuttleThread==false path
    _execute=false;
    _handle=NULL;
    return 0;
}
//------------------------------------------------------------------------------
int HThreadCtrlShuttle::CloseThread(int iTimeOut)
{
    (void)iTimeOut;
    _execute=false;
    _handle=NULL;
    _id=0;
    return 0;
}
//------------------------------------------------------------------------------
DWORD CALLBACK HThreadCtrlShuttle::ThreadProcess(LPVOID lpParameter)
{
    HThreadCtrlShuttle* thread=(HThreadCtrlShuttle*) lpParameter;

    int iShuttleZPitch=5000;
    int iSensorNo, iShuttleNo, iLineNo;
    bool bSensor[2][2]={{false, false}, {false, false}};
    bool bFlag[2]={false, false};
    int iSnOutPutSHT[2]={SnOutPutSHT1S1, SnOutPutSHT2S1};
    int iSnOutBiasSHT[2]={SnOutPutSHT1ZS1, SnOutPutSHT2ZS2};
    AnsiString Str;

    if(SHUTTLE_Z_TYPE==0)                                                       //Frank 20150814 2x2,  且iSiteYPitch剛好為2300
        iShuttleZPitch=5000;
    else
        iShuttleZPitch=2300;

    while(thread->_execute)
    {
        if(InitialOK==false)
            continue;

        if(SThreadPara.bExeShuttleThread==false ||                              //機器沒在跑與歸零的時候,不用進來; 其他的在初始化時決定要不要進來
           fAllMotorHome==false ||
           SystemStart==false)
        {
            SThreadPara.iShuttleThreadTask[0][0]=1;
            SThreadPara.iShuttleThreadTask[0][1]=1;
            SThreadPara.iShuttleThreadTask[1][0]=1;
            SThreadPara.iShuttleThreadTask[1][1]=1;
            MySleepEx(1, false);
            continue;
        }

        //沒在動的時候不用進來----------------------------------
        if(bShuttle1CatchSignalR==false && bShuttle1CatchSignalL==false && MOT[MInShuttle1].IsCanMove()==false &&
           bShuttle2CatchSignalR==false && bShuttle2CatchSignalL==false && MOT[MInShuttle2].IsCanMove()==false)
        {
            SThreadPara.iShuttleThreadTask[0][0]=1;
            SThreadPara.iShuttleThreadTask[0][1]=1;
            SThreadPara.iShuttleThreadTask[1][0]=1;
            SThreadPara.iShuttleThreadTask[1][1]=1;
            MySleepEx(1, false);
            continue;
        }

        //j=Shuttle 1 or 2 ---------------------------------------
        bFlag[0]=false;
        bFlag[1]=false;
        for(iShuttleNo=0; iShuttleNo<2; iShuttleNo++)
        {
            if(bInitialICCheck==false)
            {
                if((iShuttleNo==0 && bShuttle1CatchSignalR==false && bShuttle1CatchSignalL==false && MOT[MInShuttle1].IsCanMove()==false))
                {
                    SThreadPara.iShuttleThreadTask[0][0]=1;
                    SThreadPara.iShuttleThreadTask[0][1]=1;
                    bFlag[0]=true;  //Steven 20110607
                    continue;
                }

                if((iShuttleNo==1 && bShuttle2CatchSignalR==false && bShuttle2CatchSignalL==false && MOT[MInShuttle2].IsCanMove()==false))
                {
                    SThreadPara.iShuttleThreadTask[1][0]=1;
                    SThreadPara.iShuttleThreadTask[1][1]=1;
                    bFlag[1]=true;  //Steven 20110607
                    continue;
                }
            }

            //i=sensor 1 or 2 ------------------------------------
            for(iSensorNo=0; iSensorNo<SThreadPara.iScanSensor; iSensorNo++)
            {
                switch(SThreadPara.iShuttleThreadTask[iShuttleNo][iSensorNo])
                {
                    case 1:
                        if(SThreadPara.iScanSensor==1)
                        {
                            if(TestIF.dSiteYPitch<=iShuttleZPitch)              //Frank 20150814 2x2,  且iSiteYPitch剛好為2300
                            {
                                bSensor[iShuttleNo][iSensorNo]=Sen[iSnOutPutSHT[iShuttleNo]].IsOn();
                            }
                            else
                            {
                                bSensor[iShuttleNo][iSensorNo]=Sen[iSnOutBiasSHT[iShuttleNo]].IsOn();
                            }
                        }
                        else
                        {
                            bSensor[iShuttleNo][iSensorNo]=Sen[SnOutPutSHT1ZS1+iSensorNo+(iShuttleNo*2)].IsOn();
                        }

                        if(bSensor[iShuttleNo][iSensorNo])
                        {
                            SThreadPara.iShuttleThreadTask[iShuttleNo][iSensorNo]=100;

                            if(fMain->cbShowShuttleSensor->Checked==true)       //Steven 20130614 : Debug用
                            {
                                if(iShuttleNo==1)
                                {
                                    motorpos=MOT[MInShuttle1+iShuttleNo].ReadEncoderPos();
                                    Str.sprintf("Sensor On %d-%d: %d", iShuttleNo, iSensorNo, motorpos);
                                    fMain->AddShuttleMessage(1, Str);
                                }
                                else if(iShuttleNo==0)
                                {
                                    motorpos=MOT[MInShuttle1+iShuttleNo].ReadEncoderPos();
                                    Str.sprintf("Sensor On %d-%d: %d", iShuttleNo, iSensorNo, motorpos);
                                    fMain->AddShuttleMessage(0, Str);
                                }
                            }
                        }
                        else
                        {
                            break;
                        }
                    case 100:
                        if(SThreadPara.iScanSensor==1)
                        {
                            if(TestIF.bNS7000kit==true)
                            {
                                if(TestIF.dSiteYPitch<=iShuttleZPitch)          //Frank 20150814 2x2,  且iSiteYPitch剛好為2300
                                {
                                    bSensor[iShuttleNo][iSensorNo]=Sen[iSnOutPutSHT[iShuttleNo]].IsOff();
                                }
                                else
                                {
                                    bSensor[iShuttleNo][iSensorNo]=Sen[iSnOutBiasSHT[iShuttleNo]].IsOff();
                                }
                            }
                            else
                            {
                                bSensor[iShuttleNo][iSensorNo]=Sen[iSnOutPutSHT[iShuttleNo]].IsOff();
                            }
                        }
                        else
                        {
                            bSensor[iShuttleNo][iSensorNo]=Sen[SnOutPutSHT1ZS1+iSensorNo+(iShuttleNo*2)].IsOff();
                        }

                        if(bSensor[iShuttleNo][iSensorNo])
                        {
                            #ifdef SOFT_SIMULTE
                                motorpos=MOT[MInShuttle1+iShuttleNo].ReadPos();
                            #else
                                motorpos=MOT[MInShuttle1+iShuttleNo].ReadEncoderPos();
                            #endif

                            if(fMain->cbShowShuttleSensor->Checked==true)       //Steven 20130614 : Debug用
                            {
                                if(iShuttleNo==1)
                                {
                                    Str.sprintf("Sensor Off : %d", motorpos);
                                    fMain->AddShuttleMessage(1, Str);           //JerryYang 20170221 (wei) 0->1,修正shuttle sensor log錯誤
                                }
                                else
                                {
                                    Str.sprintf("Sensor Off : %d", motorpos);
                                    fMain->AddShuttleMessage(0, Str);
                                }
                            }

                            for(iLineNo=0; iLineNo<SThreadPara.iCHKStep; iLineNo++)
                            {                                                   //jou 2014-10-09 修正Device superfluous at Output Shuttle誤報錯誤
                                if(SThreadPara.iScanSensor==1)                  //kevin 20161219 : fixed for 1x1 out shuttle detection
                                {
                                    if(motorpos>=SThreadPara.iCheckPosMinY[iShuttleNo][iLineNo] &&
                                       motorpos<=SThreadPara.iCheckPosMaxY[iShuttleNo][iLineNo])  //Jou 20141013 : iCheckPosMinY --> iCheckPosMaxY
                                    {
                                        if(bEnter[iShuttleNo][iSensorNo][iLineNo]==false)
                                        {
                                            bEnter[iShuttleNo][iSensorNo][iLineNo]=true;
                                            bShuttleHasIC[iShuttleNo][iSensorNo][iLineNo]=false;
                                            break;
                                        }
                                    }
                                }
                                else
                                {
                                    if(motorpos>=SThreadPara.iCheckPosMin[iShuttleNo][iLineNo] &&
                                       motorpos<=SThreadPara.iCheckPosMax[iShuttleNo][iLineNo])
                                    {
                                        if(bEnter[iShuttleNo][iSensorNo][iLineNo]==false)
                                        {
                                            bEnter[iShuttleNo][iSensorNo][iLineNo]=true;
                                            bShuttleHasIC[iShuttleNo][iSensorNo][iLineNo]=false;
                                            break;
                                        }
                                    }
                                }
                            }
                            SThreadPara.iShuttleThreadTask[iShuttleNo][iSensorNo]=1;
                        }
                        break;
                }
            }
        }

        if(bFlag[0]==true && bFlag[1]==true)                                    //Steven 20110607
            MySleepEx(1, false);                                                // Release CPU Resource
    }

    return 0;
}
//------------------------------------------------------------------------------
void InitialShuttleThread()                                                     //Steven 20110321
{
    ShuttleThread.Initial(fMain);
}
//------------------------------------------------------------------------------
void StartShuttleThread()
{
    ShuttleThread.OpenThread();                                                 //Steven 20110321
}
//------------------------------------------------------------------------------
void EndShuttleThread()
{
    LogSoftwareOffTime("FC: close ShuttleThread");
    ShuttleThread.CloseThread(1000);                                            //Steven 20110321
    MySleep(100);
}
//------------------------------------------------------------------------------
void StartDetectMotorSensor(int SelSHT)                                         //檢查IC殘留
{
    if(SThreadPara.bExeShuttleThread==false)                                    //不是用Thread就不用進來
        return;

    for(int i=0; i<BRCarryKit.iShtRow; i++)
    {
        for(int j=0; j<BRCarryKit.iShtCol; j++)
        {
            bShuttleHasIC[SelSHT][i][j]=true;
            bEnter[SelSHT][i][j]=false;
        }
    }

    if(fMain->pgMain->ActivePageIndex==fMain->emp7TabSheet21)                   //在Motor View才處理
    {
        for(int iR=0; iR<BRCarryKit.iShtRow; iR++)
        {
            for(int iC=0; iC<BRCarryKit.iShtCol; iC++)
            {
                if(SelSHT==0)
                    fMain->htShullte0->SetCellColorIndex(iC, iR, NULL_IC);
                else
                    fMain->htShullte1->SetCellColorIndex(iC, iR, NULL_IC);
            }
        }
    }
}
//------------------------------------------------------------------------------
//2014-01-06    Dell    for TSMC 確認shuttle 有沒有斷線
//function : Shuttle 位置從左->右->左 這中間一直檢查sensor是否有被遮斷,如果一直沒有就是sensor斷線
const int ShuttleNum=8;
bool CheckShuttleSensorBroken_1(bool bRefreshCheck, bool bRight)
{
    static int iCheckCount=0;
    static bool bDoCheck=true;
    static bool bFlag[ShuttleNum] ={0, 0, 0, 0, 0, 0, 0, 0};

    bool bHasErr=false;
    AnsiString ErrorPart="";

    if(bRight==true)                                                            //Steven 20221213 : 確認shuttle 有沒有斷線
    {
        if(iCheckCount>0 && bDoCheck)
        {
            for(int i=0; i<ShuttleNum; i++)
            {
                if(bFlag[i]==false)                                             //Steven 20220907 : 修正F16
                {
                    ErrorPart+=ShuttleSensorName[i];
                    bHasErr=true;
                }
            }

            if(bHasErr)
            {
                if(TestIF.iShuttleMode==0 ||
                   (TestIF.iShuttleMode==1 && TestIF.iShuttle_Sel==0))          //Steven 20221026 : [F16]關Arm不檢查
                {
                    ShowErrorMessage("JAM0407", 0, MInShuttle1, false, ErrorPart);
                }
                else
                {
                    bHasErr=false;
                }
                bDoCheck=false;
            }
            iCheckCount=0;
            ZeroMemory(&bFlag, sizeof(bFlag));
        }
        return bHasErr;
    }

    if(bRefreshCheck)
    {
        bDoCheck=true;
        iCheckCount=0;
        ZeroMemory(&bFlag, sizeof(bFlag));
        return bHasErr;
    }

    if(bDoCheck==false)
        return bHasErr;

    switch(TestIF.iTestMode)
    {
        case DualSite2x1:
        case SingleSite:
            bFlag[1]=true;
            bFlag[2]=true;
            bFlag[3]=true;
            bFlag[4]=true;
            bFlag[5]=true;
            bFlag[6]=true;
            bFlag[7]=true;
            break;
        case DualSite:                                                          //1x2
        case QualSite2X2N:                                                      //Frank 20200520 2X2NN Mode
        case QualSite2X2:                                                       //2x2
            bFlag[2]=true;
            bFlag[3]=true;
            bFlag[4]=true;
            bFlag[5]=true;
            bFlag[6]=true;
            bFlag[7]=true;
            break;
        case TriSite1X3:                                                        //Frank 20160329 add for 1x3_4
        case _6Site2X3:                                                         //ChungHung 20140115 add for 2x3_6
        case _6Site2X3N:                                                        //Steven 20220425 : 2X3NN Mode
            bFlag[3]=true;
            bFlag[4]=true;
            bFlag[5]=true;
            bFlag[6]=true;
            bFlag[7]=true;
            break;
        case QualSite1X4:                                                       //1x4
        case _8Site1X4:                                                         //ChungHung 20150528 add for 海思 _8Site1x4
        case _16Site4X4:                                                        //Sam 20190226 : 16Site4X4
        case _8Site2X4:                                                         //2x4
        case _8Site2X4N:                                                        //Wei 20231211 : 2X4NN Mode
            bFlag[4]=true;
            bFlag[5]=true;
            bFlag[6]=true;
            bFlag[7]=true;
            break;
        case _16Site2X8:                                                        //2x8
            break;
        case _10Site2X5:                                                        //2x5  //wei 20190614 10 site
            bFlag[5]=true;
        case _12Site2X6:                                                        //2x6
            bFlag[6]=true;
            bFlag[7]=true;
            break;
        case _32Site4X8N:                                                       //4x8
            break;
    }

    for(int i=0; i<ShuttleNum; i++)
    {
        if(bFlag[i])
            continue;

        if(Sen[SThreadPara.iInShSenIndex[0][i]].Enable)
            bFlag[i]=Sen[SThreadPara.iInShSenIndex[0][i]].IsOn();
        else
            bFlag[i]=true;
    }

    if(bFlag[0] && bFlag[1] && bFlag[2] && bFlag[3] &&
       bFlag[4] && bFlag[5] && bFlag[6] && bFlag[7])
    {
        iCheckCount=0;
        bDoCheck   =false;
        ZeroMemory(&bFlag, sizeof(bFlag));
        return bHasErr;
    }

    iCheckCount++;
    return bHasErr;
}
//------------------------------------------------------------------------------
//2014-01-06    Dell    for TSMC 確認shuttle 有沒有斷線
//function : Shuttle 位置從左->右->左 這中間一直檢查sensor是否有被遮斷,如果一直沒有就是sensor斷線
bool CheckShuttleSensorBroken_2(bool bRefreshCheck, bool bRight)
{
    static int iCheckCount=0;
    static bool bDoCheck=true;
    static bool bFlag[ShuttleNum]={0, 0, 0, 0, 0, 0, 0, 0};

    bool bHasErr=false;
    AnsiString ErrorPart="";

    if(bRight==true)                                                            //Steven 20221213 : 確認shuttle 有沒有斷線
    {
        if(bDoCheck && iCheckCount>0)
        {
            for(int i=0; i<ShuttleNum; i++)
            {
                if(bFlag[i]==false)                                             //Steven 20220907 : 修正F16
                {
                    bHasErr=true;
                    ErrorPart+=ShuttleSensorName[i];
                }
            }

            if(bHasErr)
            {
                if(TestIF.iShuttleMode==0 ||
                   (TestIF.iShuttleMode==1 && TestIF.iShuttle_Sel==1))          //Steven 20221026 : [F16]關Arm不檢查
                {
                    ShowErrorMessage("JAM0408", 0, MInShuttle2, false, ErrorPart);
                }
                else
                {
                    bHasErr=false;
                }
                bDoCheck=false;
            }
            iCheckCount=0;
            ZeroMemory(&bFlag, sizeof(bFlag));
        }
        return bHasErr;
    }

    if(bRefreshCheck)
    {
        bDoCheck=true;
        iCheckCount=0;
        ZeroMemory(&bFlag, sizeof(bFlag));
        return bHasErr;
    }

    if(bDoCheck==false)
        return bHasErr;

    switch(TestIF.iTestMode)
    {
        case DualSite2x1:
        case SingleSite:
            bFlag[1]=true;
            bFlag[2]=true;
            bFlag[3]=true;
            bFlag[4]=true;
            bFlag[5]=true;
            bFlag[6]=true;
            bFlag[7]=true;
            break;
        case DualSite:                                                          //1x2
        case QualSite2X2N:                                                      //Frank 20200520 2X2NN Mode
        case QualSite2X2:                                                       //2x2
            bFlag[2]=true;
            bFlag[3]=true;
            bFlag[4]=true;
            bFlag[5]=true;
            bFlag[6]=true;
            bFlag[7]=true;
            break;
        case TriSite1X3:                                                        //Frank 20160329 add for 1x3_4
        case _6Site2X3:                                                         //ChungHung 20140115 add for 2x3_6
        case _6Site2X3N:                                                        //Steven 20220425 : 2X3NN Mode
            bFlag[3]=true;
            bFlag[4]=true;
            bFlag[5]=true;
            bFlag[6]=true;
            bFlag[7]=true;
            break;
        case QualSite1X4:                                                       //1x4
        case _8Site1X4:                                                         //ChungHung 20150528 add for 海思 _8Site1x4
        case _16Site4X4:                                                        //Sam 20190226 : 16Site4X4
        case _8Site2X4:                                                         //2x4
        case _8Site2X4N:                                                        //Wei 20231211 : 2X4NN Mode
            bFlag[4]=true;
            bFlag[5]=true;
            bFlag[6]=true;
            bFlag[7]=true;
            break;
        case _16Site2X8:                                                        //2x8
            break;
        case _10Site2X5:                                                        //2x5  //wei 20190614 10 site
            bFlag[5]=true;
        case _12Site2X6:                                                        //2x6
            bFlag[6]=true;
            bFlag[7]=true;
            break;
        case _32Site4X8N:                                                       //4x8
            break;
    }

    for(int i=0; i<ShuttleNum; i++)
    {
        if(bFlag[i])
            continue;

        if(Sen[SThreadPara.iInShSenIndex[1][i]].Enable)
            bFlag[i]=Sen[SThreadPara.iInShSenIndex[1][i]].IsOn();
        else
            bFlag[i]=true;
    }

    if(bFlag[0] && bFlag[1] && bFlag[2] && bFlag[3] &&
       bFlag[4] && bFlag[5] && bFlag[6] && bFlag[7])
    {
        iCheckCount=0;
        bDoCheck=false;
        ZeroMemory(&bFlag, sizeof(bFlag));
        return bHasErr;
    }

    iCheckCount++;
    return bHasErr;
}
//------------------------------------------------------------------------------
int GetShuttle1Pitch_9045(int w)                                                //wei 20160914 Auto Shuttle Sensor
{
    double m, r;
    m= ((double)(Prod.iShuttle1180Pitch-Prod.iShuttle1120Pitch))/6000.0;
    r=(double)Prod.iShuttle1120Pitch+m*((double)w-12000.0);
    return (int)r;
}
//------------------------------------------------------------------------------
int GetShuttle2Pitch_9045(int w)                                                //wei 20160914 Auto Shuttle Sensor
{
    double m, r;
    m= ((double)(Prod.iShuttle2180Pitch-Prod.iShuttle2120Pitch))/6000.0;
    r=(double)Prod.iShuttle2120Pitch+m*((double)w-12000.0);
    return (int)r;
}
//------------------------------------------------------------------------------
int CheckShuttlePitch()                                                         //wei 20160914 Auto Shuttle Sensor
{
    bool bShuttleSensorCanMove=false;
    int iShuttlePitch=0;
    if(AUTO_SENSOR_INSTALL)
    {
        if(TestIF_File.iTestMode==SingleSite)                                   //1x1 SingleSite                      //sensor 3
        {
            iShuttlePitch=0;
        }
        else if(TestIF_File.iTestMode==DualSite ||
                TestIF_File.iTestMode==QualSite2X2 ||
                TestIF_File.iTestMode==QualSite2X2N)                            //Frank 20200520 2X2NN Mode
        {
            iShuttlePitch=TestIF_File.dSiteXPitch*100;
            if(iShuttlePitch*3>=12000 && iShuttlePitch*3<=24000)
            {
                bShuttleSensorCanMove=true;
                iShuttlePitch=iShuttlePitch*3;
            }
            else if(iShuttlePitch>=9000 && iShuttlePitch<=18000)
            {
                bShuttleSensorCanMove=true;
            }
        }
        else if(TestIF_File.iTestMode==TriSite1X3 ||                            //1x3 //Frank 20160329 add for 1x3_4    //sensor 1 3 5
                TestIF_File.iTestMode==_6Site2X3N ||                            //Steven 20220425 : 2X3NN Mode
                TestIF_File.iTestMode==_6Site2X3    )                           //ChungHung 20140115 add for 2x3_6
        {
            iShuttlePitch=TestIF_File.dSiteXPitch*2*100;
            if(iShuttlePitch*3>=12000 && iShuttlePitch*3<=24000)
            {
                bShuttleSensorCanMove=true;
                iShuttlePitch=iShuttlePitch*3;
            }
            else if(iShuttlePitch>=8759 && iShuttlePitch<=24000)
            {
                bShuttleSensorCanMove=true;
            }
        }
        else if(TestIF_File.iTestMode==QualSite1X4 ||                           //1x4                       //sensor 1 2 4 5
                TestIF_File.iTestMode==_8Site1X4   ||                           //ChungHung 20150528 add for 海思 _8Site1x4
                TestIF_File.iTestMode==_8Site2X4   ||                           //2x4
                TestIF_File.iTestMode==_16Site4X4  ||                           //Sam 20190226 : 16Site4X4
                TestIF_File.iTestMode==_8Site2X4N)                              //Wei 20231211 : 2X4NN Mode
        {
            iShuttlePitch=TestIF_File.dSiteXPitch*3*100;
            if(iShuttlePitch>=12000 && iShuttlePitch<=24000)
            {
                bShuttleSensorCanMove=true;
            }
        }

        if(bShuttleSensorCanMove==false)
        {
            ShowMyMessage("Auto Shuttle Sensor 不支援此Pitch!!");
            return 0;
        }
    }

    return iShuttlePitch;
}
//------------------------------------------------------------------------------
bool DoMoveShuttleSensor()                                                      //wei 20160914 Auto Shuttle Sensor
{
    int iSh1Variable =0;
    int iSh2Variable =0;
    int iShuttlePitch=0;

    iShuttlePitch=CheckShuttlePitch();

    iSh1Variable=GetShuttle1Pitch_9045(iShuttlePitch+InArmOffSet_File[InOfsAutoSh1]->GetX()*100);
    iSh2Variable=GetShuttle2Pitch_9045(iShuttlePitch+InArmOffSet_File[InOfsAutoSh2]->GetX()*100);

    if(iShuttlePitch==0)
    {
        ShowMyMessage("Auto Shuttle Sensor 不支援此Pitch!!");
        return true;
    }
    else
    {
        if(ShuttleSensorContinuousMove(iSh1Variable, iSh2Variable, false))
        {
            return true;
        }
    }

    return false;
}
//------------------------------------------------------------------------------
bool DoInOutARM_SHT_MoveSafe(int iShuttle)                                      //kevin 20161005 add
{
    int Mot=MInShuttle1;
    if(iShuttle==1)
       Mot=MInShuttle2;

    if(DoINARM_SHT_MoveSafe(iShuttle))                                          //kevin 20161005 SHUTTLE 移動安全保護
    {
        MOT[Mot].PCIL132_StopMotor();
        if(CUSTOMER_CODE!=CC_KYEC_LEE)  //Eastsun 20260515 F022: D4
        	MoveInArmZToPlateSafe(2222);
        return true;
    }

    if(DoOutARM_SHT_MoveSafe(iShuttle))                                         //kevin 20161005 out arm Z 是否在shuttle 2
    {
        MOT[Mot].PCIL132_StopMotor();
        if(CUSTOMER_CODE!=CC_KYEC_LEE)  //Eastsun 20260515 F022: D4
        	MoveOutArmToAutoSafe_9045();
        return true;
    }
    return false;
}
//------------------------------------------------------------------------------
bool DoINARM_SHT_MoveSafe(int iShuttle)                                         //kevin 20161005 add
{
    int iMotNo;
    int iXPos    =MOT[MInArmX].ReadEncoderPos();
    int iYPos    =MOT[MInArmY].ReadEncoderPos();
    int XSafePos1=Prod.XInArm_Shuttle1_Place[iInArmYBase][iInArmXBase]-6000;
    int YSafePos1=Prod.YInArm_Shuttle1_Place[iInArmYBase][iInArmXBase];
    int XSafePos2=Prod.XInArm_Shuttle2_Place[iInArmYBase][iInArmXBase]-6000;
    int YSafePos2=Prod.YInArm_Shuttle2_Place[iInArmYBase][iInArmXBase];
    bool bRult=false;
    bool bflag=false;

    for(int i=0; i<InArmSuck.iMotRow; i++)
    {
        for(int j=0; j<InArmSuck.iMotCol; j++)
        {
            iMotNo=(USE_PICKER_COUNT==ep16Picker && InOutArmPickerUseMotor==eptUseMotCyn)?MInArmZA:InArmSuck.Suck[i][j].iMotNo;
            MOT[iMotNo].ScanMotorStatus();
            if(MOT[iMotNo].Motor->Enable && MOT[iMotNo].Led[iHomeLed]==false &&
               (CUSTOMER_CODE!=CC_KYEC_LEE || !MOT[iMotNo].MovFlag))  //Eastsun 20260515 F022: D4
                bflag=true;
        }
    }

    if(iXPos<XSafePos1 ||
       iYPos<(YSafePos1-6000))                                                  //不再shuttle 範圍
    {
        bRult=false;
    }
    else if(iShuttle==0 &&                                                      //shuttle 1範圍
            (iXPos>XSafePos1 &&
            (iYPos>YSafePos1 && (iYPos<YSafePos1+2000))))
    {
        bRult=bflag;
    }
    else if(iShuttle==1 &&                                                      //shuttle 2範圍
            (iXPos>XSafePos2 &&
            (iYPos>(YSafePos2-3000))))
    {
        bRult=bflag;
    }
    return bRult;
}
//------------------------------------------------------------------------------
bool DoOutARM_SHT_MoveSafe(int iShuttle)                                        //kevin 20161005 out arm Z 是否在shuttle
{
    int iMotNo;
    int iXPos=MOT[MOutArmX].ReadEncoderPos();
    int iYPos=MOT[MOutArmY].ReadEncoderPos();
    int XSafePos1=Prod.XOutArm_Shuttle1_Pick[iOutArmYBase][iOutArmXBase]+6000;
    int YSafePos1=Prod.YOutArm_Shuttle1_Pick[iOutArmYBase][iOutArmXBase];
    int XSafePos2=Prod.XOutArm_Shuttle2_Pick[iOutArmYBase][iOutArmXBase]+6000;
    int YSafePos2=Prod.YOutArm_Shuttle2_Pick[iOutArmYBase][iOutArmXBase];
    int XSafePos3=XSafePos2;                                                    //RogerYang 20250510 Add for 9046AU
    int YSafePos3=YSafePos2;
    if(USE_OUT_SORT_ARM!=eartUninstall)
    {
        XSafePos3=Prod.XOutArm_SortShuttle_Place[iOutArmYBase][iOutArmXBase]+6000;
        YSafePos3=Prod.YOutArm_SortShuttle_Place[iOutArmYBase][iOutArmXBase];
    }
    bool bRult=false;
    bool bflag=false;

    for(int i=0; i<OutArmSuck.iPickRow; i++)
    {
        for(int j=0; j<OutArmSuck.iPickCol; j++)
        {
            iMotNo=(InOutArmPickerUseMotor==eptUseMotCyn)?MOutArmZA:OutArmSuck.Suck[i][j].iMotNo;
            MOT[iMotNo].ScanMotorStatus();
            if(MOT[iMotNo].Motor->Enable && MOT[iMotNo].Led[iHomeLed]==false &&
               (CUSTOMER_CODE!=CC_KYEC_LEE || !MOT[iMotNo].MovFlag))  //Eastsun 20260515 F022: D4
                bflag=true;
        }
    }

    if(iXPos>XSafePos1 || iYPos<(YSafePos1-6000))                               //不再shuttle 範圍
    {
        bRult=false;
    }
    else if(iShuttle==0 &&
            (iXPos<XSafePos1 &&
            (iYPos<YSafePos1+2000 &&
             iYPos>YSafePos1-1000)))                                            //shuttle 1範圍
    {
        bRult=bflag;
    }
    else if(iShuttle==1 &&
            (iXPos<XSafePos2 &&
             iYPos>(YSafePos2-3000)))                                           //shuttle 2範圍
    {
        bRult=bflag;
    }
    else if(iShuttle==2 &&                                                      //RogerYang 20250510 Add for 9046AU
            (iXPos<XSafePos3 &&
            (iYPos>YSafePos3-3000&&
             iYPos<YSafePos2-1000)))                                           //Sort shuttle 範圍
    {
        bRult=bflag;
    }

    return bRult;
}
//------------------------------------------------------------------------------
//V3.27E.528 Isaac 20170418 (Steven) 用Y Latch檢測Inshuttle有無IC
//------------------------------------------------------------------------------
int DoCheckShuttle1ICByLTC(bool bCheckHasIC)                                    //true : 檢查飛料, false : 檢查殘料
{
    int ret=0;
    bool bflagNoIC=false;
    AnsiString ErrPart="";

    for(int i=0; i<8; i++)
        SThreadPara.bInSh1HasIC[0][i]=true;

    if(TestIF.iShuttleMode==0 || (TestIF.iShuttleMode==1 && TestIF.iShuttle_Sel==0))
    {
        for(int j=0; j<FLCarryKit.iShtCol; j++)
        {
            for(int i=0; i<fLtcSensor->LatchDataCnt4; i++)
            {
                if(fLtcSensor->LatchDataTable4[i]<SThreadPara.iCheckInPosMaxY[0][j] &&
                   fLtcSensor->LatchDataTable4[i]>SThreadPara.iCheckInPosMinY[0][j])
                {
                    if((FLCarryKit.Item[0][j]!=NULL_IC && FLCarryKit.Item[0][j]!=HAS_NULL_IC) ||
                       (FLCarryKit.Item[1][j]!=NULL_IC && FLCarryKit.Item[1][j]!=HAS_NULL_IC))
                        SThreadPara.bInSh1HasIC[0][j]=false;
                    break;
                }
            }
        }

        if(bCheckHasIC)
        {
            for(int j=0; j<FLCarryKit.iShtCol; j++)
            {
                if(SThreadPara.bInSh1HasIC[0][j]==true)
                {
                    ErrPart+=" ";
                }
                else
                {
                    int iNN=IsNNMode();
                    if(FTestSuck.iShtRow==1)
                        ErrPart+=IndexSuckName[0+iNN][j];
                    else
                        ErrPart+="("+IndexSuckName[0+iNN][j]+","+IndexSuckName[1+iNN][j]+")";
                    bflagNoIC=true;
                }
            }

            if(bflagNoIC==true)
            {
                bInSht1LtcDetectTesterCanMove=false;
                ret=ShowErrorMessage("JAM0403", K_RETRY, MInShuttle1, false, ErrPart);     //retry: check by step
            }
        }
        else
        {
            for(int j=0; j<FLCarryKit.iShtCol; j++)
            {
                if(SThreadPara.bInSh1HasIC[0][j]==true)
                {
                    int iNN=IsNNMode();
                    if(FTestSuck.iShtRow==1)
                        ErrPart+=IndexSuckName[0+iNN][j];
                    else
                        ErrPart+="("+IndexSuckName[0+iNN][j]+","+IndexSuckName[1+iNN][j]+")";
                    bflagNoIC=true;
                }
                else
                {
                    ErrPart+=" ";
                }
            }

            if(bflagNoIC==true)
            {
                bInSht1LtcDetectTesterCanMove=false;
                ret=ShowErrorMessage("JAM0480", K_RETRY, MInShuttle1, false, ErrPart);     //retry: check by step
            }
        }
        return ret;
    }
    else
    {
        return 0;
    }
}
//------------------------------------------------------------------------------
//V3.27E.528 Isaac 20170418 (Steven) 用Y Latch檢測Inshuttle有無IC
//------------------------------------------------------------------------------
int DoCheckShuttle2ICByLTC(bool bCheckHasIC)                                    //true : 檢查飛料, false : 檢查殘料
{
    int ret=0;
    bool bflagNoIC=false;
    AnsiString ErrPart="";

    for(int i=0; i<8; i++)
        SThreadPara.bInSh2HasIC[0][i]=true;

    if(TestIF.iShuttleMode==0 || (TestIF.iShuttleMode==1 && TestIF.iShuttle_Sel==1))
    {
        for(int j=0; j<BLCarryKit.iShtCol; j++)
        {
            for(int i=0; i<fLtcSensor->LatchDataCnt6; i++)
            {
                if(fLtcSensor->LatchDataTable6[i]<SThreadPara.iCheckInPosMaxY[1][j] &&
                   fLtcSensor->LatchDataTable6[i]>SThreadPara.iCheckInPosMinY[1][j])
                {
                    if((BLCarryKit.Item[0][j]!=NULL_IC && BLCarryKit.Item[0][j]!=HAS_NULL_IC) ||
                       (BLCarryKit.Item[1][j]!=NULL_IC && BLCarryKit.Item[1][j]!=HAS_NULL_IC))
                        SThreadPara.bInSh2HasIC[0][j]=false;
                    break;
                }
            }
        }

        if(bCheckHasIC)
        {
            for(int j=0; j<BLCarryKit.iShtCol; j++)
            {
                if(SThreadPara.bInSh2HasIC[0][j]==true)
                {
                    ErrPart+=" ";
                }
                else
                {
                    if(BTestSuck.iShtRow==1)
                        ErrPart+=IndexSuckName[0][j];
                    else
                        ErrPart+="("+IndexSuckName[0][j]+","+IndexSuckName[1][j]+")";
                    bflagNoIC=true;
                }
            }

            if(bflagNoIC==true)
            {
                bInSht2LtcDetectTesterCanMove=false;
                ret=ShowErrorMessage("JAM0406", K_RETRY, MInShuttle2, false, ErrPart);     //retry: check by step
            }
        }
        else
        {
            for(int j=0; j<BLCarryKit.iShtCol; j++)
            {
                if(SThreadPara.bInSh2HasIC[0][j]==true)
                {
                    if(BTestSuck.iShtRow==1)
                        ErrPart+=IndexSuckName[0][j];
                    else
                        ErrPart+="("+IndexSuckName[0][j]+","+IndexSuckName[1][j]+")";
                    bflagNoIC=true;
                }
                else
                {
                    ErrPart+=" ";
                }
            }

            if(bflagNoIC==true)
            {
                bInSht2LtcDetectTesterCanMove=false;
                ret=ShowErrorMessage("JAM0481", K_RETRY, MInShuttle2, false, ErrPart);     //retry: check by step
            }
        }
        return ret;
    }
    else
    {
        return 0;
    }
}
//------------------------------------------------------------------------------
void CheckShuttle1EncoderPos()                                                  //JerryYang 20230131 : add shuttle shift log
{
    int iCMDPos=MOT[MInShuttle1].ReadPos();
    int iEncPos=MOT[MInShuttle1].ReadEncoderPos();
    AnsiString str="";

    if(abs(iEncPos-iCMDPos)>=15)
    {
        if((iEncPos-iCMDPos)>0)
        {
            str.sprintf("Index Arm 1 down, shuttle 1 shift +%.2f (mm) | Encoder=%d, Command=%d", double(iEncPos-iCMDPos)/100.0, iEncPos, iCMDPos);
            RecordProcess(str);
        }
        else
        {
            str.sprintf("Index Arm 1 down, shuttle 1 shift %.2f (mm) | Encoder=%d, Command=%d", double(iEncPos-iCMDPos)/100.0, iEncPos, iCMDPos);
            RecordProcess(str);
        }
    }
}
//------------------------------------------------------------------------------
void CheckShuttle2EncoderPos()                                                  //JerryYang 20230131 : add shuttle shift log
{
    int iCMDPos=MOT[MInShuttle2].ReadPos();
    int iEncPos=MOT[MInShuttle2].ReadEncoderPos();
    AnsiString str="";

    if(abs(iEncPos-iCMDPos)>=15)
    {
        if((iEncPos-iCMDPos)>0)
        {
            str.sprintf("Index Arm 2 down, shuttle 2 shift +%.2f (mm) | Encoder=%d, Command=%d", double(iEncPos-iCMDPos)/100.0, iEncPos, iCMDPos);
            RecordProcess(str);
        }
        else
        {
            str.sprintf("Index Arm 2 down, shuttle 2 shift %.2f (mm) | Encoder=%d, Command=%d", double(iEncPos-iCMDPos)/100.0, iEncPos, iCMDPos);
            RecordProcess(str);
        }
    }
}
//------------------------------------------------------------------------------
void Do_Auto_SHT3()                                                             //RogerYang 20250509 Add for 9046AU
{
//    static int iRetryCT=0;
//    static int iHttpErrRetryCT=0;
//    static int SH3X=0, SH3Y=0;
//    static int iShuttlePurgeCount=0;
    static int iOldTask=0;
    //static bool bCheckRotateSensor[2]={false,false};
//    static bool bNotLeft=false;
    static bool bDuplicateErr=false;
    static bool bMoveLeft=false;                                                //reset shuttle speed for D43
    static bool bSpeedDown=false;
    static bool RunOK=false;
    //static bool RunShuttleCheckIC=false;                                        //確認shuttle 移出是否有 IC

//    if(IniConfig.bF16CheckShuttleSensorBroken && bDoingF16)                     //Steven 20221213 : 確認shuttle 有沒有斷線
//        return;

    if(SystemNG)
        return;

//    int ret=-1;
//    int pos=0;
    int &Task=AutoSHT3Task;
//    bool &bCheckShuttleFlag=bCheckShuttle3Flag, Flag=false;
    bool &bCheckShuttleFlag=bCheckShuttle3Flag;
    AnsiString str;

    switch(Task)
    {
        case 1:
            if(MOT[MOutSortSht].IsCanMove())
            {
                Task=10;
            }
            break;
        case 10:
            if(OutSht3Kit.UseSiteHasIC())                                       //有IC就到左邊等
            {
                Task=200;
            }
            else                                                                //空空就去右邊等
            {
                Task=100;
            }
            break;
        case 100:
//            if(MOT[MOutSortSht].fCanMoveR &&
//                (OutArmZSafe(DETECT_ALL_FLAG)==-1)&&
//                SortSHTInRT()==false)
            if(MOT[MOutSortSht].fCanMoveR            &&
                (OutArmZSafe(DETECT_SENSOR_FLAG)==-1)&&
                (SortArmZSafe(DETECT_ALL_FLAG)==-1)  &&
                SortSHTInRT()==false)
            {                                                                   //_todo Latch還沒寫 //Shuttle 退出是否有IC 殘留檢測
//                if(DoCheckShuttle3EmptyIC())
//                {
                    Task=120;
//                }
                //先判斷out arm有沒有在安全位置
            }
            else
                Task=1;
            break;
        case 120:
            if(IniConfig.bF21InOutArmZMotorPrivate)
            {
                if(DoSortOutARM_SHT_MoveSafe())                                 //SHUTTLE 移動安全保護
                    return;
            }

            if(MOT[MOutSortSht].MotorMove(Prod.SortSHT.iRight))
            {
                bMoveLeft=false;
                RunOK=false;                                                    //JerryYang 20160809 修正開啟F18 hang up問題
                bCheckShuttleFlag=true;                                         //kevin 20190614 move

                bDuplicateErr=false;

                if(iOldTask!=Task)
                {                               //執行序結束
                    //fLtcSensor->GetLtcSensor(2);                              //_todo Latch未完成  //Steven 20110526 : 取得往左移動的Latch
                    iOldTask=Task;
                }

                if(bSpeedDown==true)
                    SetMotorScaleSpeed(MOutSortSht, SHSpeed.iSH1Sp);            //RogerYang note: 先蹭一下SH1的速度 //Steven 20110607 : 重新檢查時要放慢速度, 檢查完要還原速度

                bSortShtMoveToRight=true;

//                if(b1ShuttleMoveToLeft==false)
//                {
//                    if(b1ShuttleMoveToRight==true)
//                    {
//                        b1ShuttleMoveToRight=false;
//                        bSHTOfsChangeRight[0]=false;                            //Sam 20221205 : 修正 Shuttle 移動完成後又被修改 Offset 導致 Hang up
//                        if(TestIF.iShuttleMode==0 ||
//                           (TestIF.iShuttleMode==1 && TestIF.iShuttle_Sel==0))  //Jou 20110421
//                        {
//                            if(SThreadPara.bExeShuttleThread==false)
//                            {
//                                if(FRCarryKit.UseSiteNoIC())                    //jou 2016-04-22 修正Out shuttle殘料異常
//                                {
//                                    if(CheckShuttle1OutputHasICError_M204())
//                                    {
//                                        Task=400;
//                                        break;
//                                    }
//                                }
//                            }
//                            else
//                            {
//                                if(CheckShuttleOutputHasICError(0, SH1X, SH1Y))
//                                {
//                                    if(fMain->cbShowShuttleSensor->Checked==true)
//                                    {
//                                        AnsiString sFileName;
//                                        sFileName.sprintf("%04d-%02d-%02d %02d:%02d:%02d.%d has error, retry:%d", SystemYear, SystemMonth, SystemDate, SystemHour, SystemMin, SystemSec, SystemMSec, iOutShuttle1HasICErrRetryCnt);     //Sam 20210409 : 改為全域
//                                        fMain->meShuttle1->Lines->Add(sFileName);
//                                    }
//                                    Task=400;
//                                    break;
//                                }
//                            }
//                        }
//                    }
//                    iOutShuttle1HasICErrRetryCnt=0;                             //Sam 20210409 : 改為全域
//                }
                Task=1;
            }
            else
            {
                bSortShtMoveToRight=false;
            }
            break;
        case 200:
            if(MOT[MOutSortSht].fCanMoveL            &&
                (OutArmZSafe(DETECT_SENSOR_FLAG)==-1)&&
                (SortArmZSafe(DETECT_ALL_FLAG)==-1)  &&
                SortSHTInLF()==false)
            {                                                                   //_todo Latch還沒寫 //Shuttle 退出是否有IC 殘留檢測
//                if(DoCheckShuttle3EmptyIC())
//                {
                    Task=220;
//                }
                //先判斷out arm有沒有在安全位置
            }
            else
                Task=1;
            break;
        case 220:
//            if(SortShtSureInLF()==false)                                       //若齒輪比大於1，換算有機會和目標位置差1條
//            {
//                bCheckShuttleFlag=true;
//            }

            if(IniConfig.bF21InOutArmZMotorPrivate)
            {
                if(DoSortOutARM_SHT_MoveSafe())                                 //SHUTTLE 移動安全保護
                    return;
            }

            if(MOT[MOutSortSht].MotorMove(Prod.SortSHT.iLeft))
            {
                if(iOldTask!=Task)
                {
                    //fLtcSensor->GetLtcSensor(2);                                //_todo Latch未完成 //Steven 20110526 : 取得往右移動的Latch
                    iOldTask=Task;
                }

                //bSortShtCatchSignalL=false;

//                if(b1ShuttleMoveToRight==false)
//                {
//                    b1ShuttleMoveToRight=true;                                  //確實移動到右邊
//                    if(CosFunction.bUseAfterHomeShtChkLoseICNeedSlow &&         //Sam 20201020 : 回 Home 第一次的 Shuttle Check IC lose 需要變慢。
//                       bAfterHomeShtChkLoseICNeedSlow[0])
//                    {
//                        SetMotorScaleSpeed(MInShuttle1, SHSpeed.iSH1Sp);
//                        bAfterHomeShtChkLoseICNeedSlow[0]=false;
//                    }
//
//                    if(b1ShuttleMoveToLeft==true)                               //確實的從左邊移到右邊去
//                    {
//                        if(bReadAndCheckCPUName==true &&                        //Isaac 20170418 (Steven) 用Y Latch檢測Inshuttle有無IC
//                           CosFunction.bInShuttleDetectByLatch &&
//                           Prod.bF18InshuttleDetect &&
//                           MOTION_CARD_TYPE==MotionCard_Contec &&
//                           LastSet.iRealDummy==REALLY)                          //jou 20170418 (Steven) : 矽品-世明要求修改config F18至Recipe設定
//                        {
//                            ret=DoCheckShuttle1ICByLTC(true);
//                            if(ret==K_RETRY)                                    //check by step
//                            {
//                                b1ShuttleMoveToLeft=false;
//                                b1ShuttleMoveToRight=false;
//                                Task=5000;
//                                break;
//                            }
//                            bInSht1LtcDetectTesterCanMove=true;
//                        }
//
//                        if(CosFunction.bOutShtLoseICInArmAway)
//                        {
//                            bSht1LoseICErr=ShowOutputShuttleICStatus(0, false); //show IC 遺失
//                            if(bSht1LoseICErr==false)
//                            {
//                                StartDetectMotorSensor(0);                      //清除
//                            }
//                        }
//                        else
//                        {
//                            ShowOutputShuttleICStatus(0);
//                            StartDetectMotorSensor(0);                          //清除
//                        }
//
//                        b1ShuttleMoveToLeft=false;
//                        bSHTOfsChangeLeft[0]=false;                             //Sam 20230202 : 修正 Shuttle Left 移動完成後又被修改 Offset 導致 Hang up
//                    }
//                }

                bSortShtMoveToLeft=true;
                bCheckShuttleFlag=false;

                //rogertest for 測試用 移到左邊後清空資料
//                for(int i=0; i<OutSht3Kit.iShtRow; i++)
//                {
//                    for(int j=0; j<OutSht3Kit.iShtCol; j++)
//                    {
//                        //OutSht3Kit.Item[i][j].
//                        OutSht3Kit.SetAllToNullIC();
//                    }
//                }
                Task=1;
            }
            else
            {
                bSortShtMoveToLeft=false;
            }
            break;
    }
}
//------------------------------------------------------------------------------
//bool IsSortCarrKitAllHasIC()                                                    //RogerYang 20250509 Add for 9046AU (暫時不考慮放滿)
//{
//    bool bHasEmpty=false;
//    bool bfinish=true;
//
//    for(int i=0; i<OutSht3Kit.iShtRow; i++)
//    {
//        for(int j=0; j<OutSht3Kit.iShtCol; j++)
//        {
//            if(OutSht3Kit.Item[i][j]==NULL_IC)
//            {
//                bHasEmpty=true;
//            }
//        }
//    }
//    if(bHasEmpty==true)
//        bfinish=false;
//
//    return bfinish;
//}
//------------------------------------------------------------------------------
bool DoSortOutARM_SHT_MoveSafe()                                                //RogerYang 20250509 Add for 9046AU
{
    if(DoSortARM_SHT_MoveSafe())
    {
        MOT[MOutSortSht].PCIL132_StopMotor();
        MoveSortArmToAutoSafe();
        return true;
    }

    if(DoOutARM_SHT_MoveSafe(2))
    {
        MOT[MOutSortSht].PCIL132_StopMotor();
        MoveSortArmToAutoSafe();
        return true;
    }
    return false;
}
//------------------------------------------------------------------------------
bool DoSortARM_SHT_MoveSafe()                                                   //RogerYang 20250510 Add for 9046AU
{
    int iMotNo;
    int iXPos=MOT[MOutSortX].ReadEncoderPos();
    int iYPos=MOT[MOutSortY].ReadEncoderPos();
    int XSafePos=Prod.XSortArm_SortShuttle_Pick[iSortArmYBase][iSortArmXBase]-6000;
    int YSafePos=Prod.YSortArm_SortShuttle_Pick[iSortArmYBase][iSortArmXBase];
    bool bRult=false;
    bool bflag=false;

    for(int i=0; i<OutArm2Suck.iPickRow; i++)
    {
        for(int j=0; j<OutArm2Suck.iPickCol; j++)
        {
            iMotNo=OutArm2Suck.Suck[i][j].iMotNo;
            MOT[iMotNo].ScanMotorStatus();
            if(MOT[iMotNo].Motor->Enable && MOT[iMotNo].Led[iHomeLed]==false)   // Z軸沒有在home位置
                bflag=true;
        }
    }

    if(iXPos<XSafePos || iYPos<(YSafePos-6000))                                 //不在Sort shuttle 範圍
    {
        bRult=false;
    }
    else if((iXPos>XSafePos &&
            (iYPos<YSafePos+2000 &&
             iYPos>YSafePos-1000)))                                            //Sort shuttle 範圍
    {
        bRult=bflag;
    }

    return bRult;
}
//------------------------------------------------------------------------------
bool DoFloodGateClose()                                                         //Ifor 20260401 add:Flood Gate Close
{
    bool bRult=false;

    if(SHUTTLE_FLOODGATE==1)                                                    //Ifor 20260318 add:Shuttle1 Left Floodgate Close (case 135)
    {
        if(ATC_InterfaceForm->iATC_MODE_TYPE==36 &&
           Temperature.bATCActiveCooling==true)                                 //Ifor 20260401 add: ATC Type36 low temp protection
        {
            double dChillerTemp=0;
            if(CosFunction.bUseHandlerSetChillerTemp==true)
                dChillerTemp=(double)Temperature.iATCChillerTemp;               //Ifor 20230515 add: Handler Chiller Temp
            else
                dChillerTemp=ATC_Data.dChillerSetTemp;                          //Ifor 20230515 add: ATC Chiller Set Temp
            if(dChillerTemp < -20)
                bRult=true;
        }
    }

    return bRult;
}
//------------------------------------------------------------------------------
int DoCheckShuttle1ICByLTC_AutoLatch(AnsiString &ErrPart, bool bChkData)        //KenHsieh 20250722 : InSht sensor 改為2顆，並用Latch 判別疊料以及飛料
{
    int ret=0;
    bool bflagStackIC=false;
    //KenHsieh 20260518 : 移除 iRow/iCol、GetRowCol，改用 904 native FLCarryKit.iShtRow / iShtCol

    for(int i=0; i<8; i++)
    {
        SThreadPara.bInSh1HasIC_SHSn[0][i]=true;
    }

    if(TestIF.iShuttleMode==0 || (TestIF.iShuttleMode==1 && TestIF.iShuttle_Sel==0))
    {
        for(int j=0; j<FLCarryKit.iShtCol; j++)                                             //KenHsieh 20260518 : SThreadPara.iXItem → FLCarryKit.iShtCol
        {
            for(int i=0; i<fLtcSensor->LatchDataCnt8; i++)
            {
                if(bChkData==false ||                                                           //Contact 沒資料但要偵測
                  (FLCarryKit.Item[0][j]!=NULL_IC && FLCarryKit.Item[0][j]!=HAS_NULL_IC) ||
                  (FLCarryKit.Item[1][j]!=NULL_IC && FLCarryKit.Item[1][j]!=HAS_NULL_IC))       //有貨偵測，沒貨不偵測，flag 直接變false
                {
                    if(fLtcSensor->LatchDataTable8[i]<SThreadPara.iCheckInPosMaxY_SHSn[0][j] &&
                       fLtcSensor->LatchDataTable8[i]>SThreadPara.iCheckInPosMinY_SHSn[0][j])
                    {
                        SThreadPara.bInSh1HasIC_SHSn[0][j]=false;
                        break;
                    }
                }
                else
                {
                    SThreadPara.bInSh1HasIC_SHSn[0][j]=false;
                }
            }
        }

        if(fMain->cInplace->InArmPlacementEnable())                             //KenHsieh 20251105 : Auto In shuttle latch combine No9 func.
        {
            fMain->cInplace->iNo9ShtErrICCt[0]=0;
            for(int i=0; i<8; i++)
                fMain->cInplace->bNo9ShtErrNo[0][i]=false;
        }

        ErrPart="";
        for(int j=0; j<FLCarryKit.iShtCol; j++)                                             //KenHsieh 20260518 : SThreadPara.iXItem → FLCarryKit.iShtCol
        {
            if(SThreadPara.bInSh1HasIC_SHSn[0][j]==true)
            {
                if(FLCarryKit.iShtRow==1)                                                   //KenHsieh 20260518 : GetRowCol → FLCarryKit.iShtRow
                    ErrPart+=IndexSuckName[0][j];
                else
                    ErrPart+="("+IndexSuckName[0][j]+","+IndexSuckName[1][j]+")";
                bflagStackIC=true;

                bInSHLtcErrNo[0][j]=true;                                       //KenHsieh 20251128 : InShtLtc 新增異常重複判斷
                if(fMain->cInplace->InArmPlacementEnable())                     //KenHsieh 20251105 : Auto In shuttle latch combine No9 func.
                {
                    fMain->cInplace->iNo9ShtErrICCt[0]++;
                    fMain->cInplace->bNo9ShtErrNo[0][j]=true;
                }
            }
            else
            {
                ErrPart+=" ";
                bInSHLtcErrNo[0][j]=false;                                      //KenHsieh 20251128 : InShtLtc 新增異常重複判斷
            }
        }

        if(bflagStackIC==true)
        {
            if(fMain->cbShowInShuttleSensor->Checked==true)                     //有出錯就紀錄 //KenHsieh 20260402 : InSht sensor add error log
            {
                OutShuttleLog();
                AnsiString sFileName;
                sFileName.sprintf("%04d-%02d-%02d %02d:%02d:%02d.%d", SystemYear, SystemMonth, SystemDate, SystemHour, SystemMin, SystemSec, SystemMSec);
                sFileName+=" Stack";
                sFileName+=" : Error!!";

                AnsiString str1="Sensor 8: ";

                for(int i=0; i<fLtcSensor->LatchDataCnt8; i++)
                    str1+=AnsiString(fLtcSensor->LatchDataTable8[i])+", ";

                if(fMain->meShuttle1->Lines->Count>1024)
                    fMain->meShuttle1->Clear();
                fMain->AddShuttleMessage(0, sFileName);

                fMain->AddShuttleMessage(0, str1);
            }
            ret=2;
            return ret;
        }

        return ret;
    }
    else
    {
        return 0;
    }
}
//------------------------------------------------------------------------------
int DoCheckShuttle2ICByLTC_AutoLatch(AnsiString &ErrPart, bool bChkData)        //KenHsieh 20250722 : InSht sensor 改為2顆，並用Latch 判別疊料以及飛料
{
    int ret=0;
    bool bflagStackIC=false;
    //KenHsieh 20260518 : 移除 iRow/iCol、GetRowCol，改用 904 native BLCarryKit.iShtRow / iShtCol

    for(int i=0; i<8; i++)
    {
        SThreadPara.bInSh2HasIC[0][i]=true;
        SThreadPara.bInSh2HasIC_SHSn[0][i]=true;
    }

    if(TestIF.iShuttleMode==0 || (TestIF.iShuttleMode==1 && TestIF.iShuttle_Sel==1))
    {
        for(int j=0; j<BLCarryKit.iShtCol; j++)                                             //KenHsieh 20260518 : SThreadPara.iXItem → BLCarryKit.iShtCol
        {
            for(int i=0; i<fLtcSensor->LatchDataCnt9; i++)
            {
                if(bChkData==false ||                                                           //Contact 沒資料但要偵測
                  (BLCarryKit.Item[0][j]!=NULL_IC && BLCarryKit.Item[0][j]!=HAS_NULL_IC) ||
                  (BLCarryKit.Item[1][j]!=NULL_IC && BLCarryKit.Item[1][j]!=HAS_NULL_IC))       //有貨偵測，沒貨不偵測，flag 直接變false
                {
                    if(fLtcSensor->LatchDataTable9[i]<SThreadPara.iCheckInPosMaxY_SHSn[1][j] &&
                       fLtcSensor->LatchDataTable9[i]>SThreadPara.iCheckInPosMinY_SHSn[1][j])
                    {
                        SThreadPara.bInSh2HasIC_SHSn[0][j]=false;
                        break;
                    }
                }
                else
                {
                    SThreadPara.bInSh2HasIC_SHSn[0][j]=false;
                }
            }
        }

        if(fMain->cInplace->InArmPlacementEnable())                             //KenHsieh 20251105 : Auto In shuttle latch combine No9 func.
        {
            fMain->cInplace->iNo9ShtErrICCt[1]=0;
            for(int i=0; i<8; i++)
                fMain->cInplace->bNo9ShtErrNo[1][i]=false;
        }

        ErrPart="";
        for(int j=0; j<BLCarryKit.iShtCol; j++)                                             //KenHsieh 20260518 : SThreadPara.iXItem → BLCarryKit.iShtCol
        {
            if(SThreadPara.bInSh2HasIC_SHSn[0][j]==true)
            {
                if(BLCarryKit.iShtRow==1)                                                   //KenHsieh 20260518 : GetRowCol → BLCarryKit.iShtRow
                    ErrPart+=IndexSuckName[0][j];
                else
                    ErrPart+="("+IndexSuckName[0][j]+","+IndexSuckName[1][j]+")";
                bflagStackIC=true;

                if(fMain->cInplace->InArmPlacementEnable())                     //KenHsieh 20251105 : Auto In shuttle latch combine No9 func.
                {
                    fMain->cInplace->iNo9ShtErrICCt[1]++;
                    fMain->cInplace->bNo9ShtErrNo[1][j]=true;
                }
            }
            else
            {
                ErrPart+=" ";
            }
        }

        if(bflagStackIC==true)
        {
            if(fMain->cbShowInShuttleSensor->Checked==true)                     //有出錯就紀錄 //KenHsieh 20260402 : InSht sensor add error log
            {
                OutShuttleLog();
                AnsiString sFileName;
                sFileName.sprintf("%04d-%02d-%02d %02d:%02d:%02d.%d", SystemYear, SystemMonth, SystemDate, SystemHour, SystemMin, SystemSec, SystemMSec);
                sFileName+=" Stack";
                sFileName+=" : Error!!";

                AnsiString str1="Sensor 9: ";

                for(int i=0; i<fLtcSensor->LatchDataCnt9; i++)
                    str1+=AnsiString(fLtcSensor->LatchDataTable9[i])+", ";

                if(fMain->meShuttle2->Lines->Count>1024)
                    fMain->meShuttle2->Clear();
                fMain->AddShuttleMessage(1, sFileName);

                fMain->AddShuttleMessage(1, str1);
            }
            ret=2;
            return ret;
        }

        return ret;
    }
    else
    {
        return 0;
    }
}
//------------------------------------------------------------------------------
