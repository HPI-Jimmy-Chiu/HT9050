// =============================================================================
//  asendic_Empty.cpp  --  Empty-tray stack-feed state machine (W6.1 CANARY)
//
//  Translated from: HT9011UC_Code_V3.33.906.0_20260618/asendic_Empty.cpp (1280 ln)
//  Translation wave: W6.1 (CANARY -- first root state machine over the W6 substrate)
//  Translator: AI(W6.0-SCAFFOLD) 20260626
//
//  FAITHFUL TRANSLATION
//  --------------------
//  This is the W6 decoupling canary: a self-contained, re-entrant tray-stack
//  stepper.  Names, 1-based numeric Task states, switch(Task) semantics, and the
//  case fall-throughs (DoEmptyTrayToFront case 1->100, DoEmptyTrayToRear case
//  1->100, DoEmptyTrayToRear case 300->400 NOTE) are preserved VERBATIM.  Each
//  Do* function owns its own extern cursor int (W6 strategy).
//
//  INCLUDES (heavy/UI/SECS swapped for substrate + scaffold):
//    BCB6 original pulled: main.h (TfMain), mymessbox.h (MyMessageBox VCL),
//    note.h, acatchtray.h, OCR.h/OCRInsp.h, cMyDB.h, uHGemHT9045.h (SECS),
//    AGV.h (VCL TfAGV).  Replaced by:
//      mycylin.h         (Cylinder[])          -- W6.0 HAL gap
//      Motor/mymotor.h   (MOT[])               -- W4 HAL
//      mysensor.h        (Sen[])               -- W4-IO HAL
//      myswitch.h        (SW[])                -- W4-IO HAL
//      csystem.h         (predicate API + ReadWriteTrayID)  -- frozen interface
//      asendic.h         (CylinderUp/Middle/Lower + TrayMove* + asendic_Empty.h)
//      FormsFacade.h     (fAGV->IsATK_AMR())   -- non-VCL satellite stub
//      canary_support.h  (LastSet shim, __FUNC__, ShowErrorMessage/RecordProcess/
//                         ShowUnloaderTrayMessage/WhichAutoNeedTray/ShowMyMessage)
//      cprod.h/cpublic.h/cmydef.h (Prod/IniConfig/CosFunction/TrayForm/RunInfo/
//                         Ld_UldDelayTime/TrayID/enums) -- W0-TAIL globals
//
//  GATES RETIRED -- AI(W906-W6.1-EMPTYfix) 20260801
//  ------------------------------------------------
//  This file used to carry two `#if 0` gates.  Both surfaces now EXIST in the
//  ported tree, so both gates are gone and the golden code is compiled:
//    * SECS EventReport (DoLoadNewEmptyTrayToCar case 1, golden :83-84).
//      Ported home: SECSGEM/SecsEventReport.h `void EventReport(unsigned)` (a
//      Sim-first entry point with observable state: g_SimLastEventReportCeid /
//      g_SimEventReportCount) + SECSGEM/SecsEventType.h `extern struct
//      ETypeStruct SECS_EVENT` whose NoCoverTray_Normal CEID is 244.  Both are
//      already linked into every consumer of this SM (ht9045_secsgem).  This is
//      the one retirement with an OBSERVABLE behaviour delta -- the gated build
//      silently dropped a SECS event on the Empty-stack-full path.  Covered by
//      tests/test_w6_1_empty_canary.cpp [7] (mutation-proven: re-gating goes RED).
//    * MyMessageBox close-before-alarm (DoAutoEmptyReceive case 300, golden
//      :1105-1106).  Ported home: acatchtray_shims.h `TMyMessageBoxShim
//      *MyMessageBox` (Visible/Close()), already linked via ht9045_sm and used
//      the same way by acatchtray.cpp.  Offline Close() is a no-op, so this
//      retirement has NO behaviour delta -- see the NOT-COVERED register in
//      tests/test_w6_1_empty_canary.cpp.
//
//  NOTHING IS GATED IN THIS FILE ANY MORE.  If you are copying this file as the
//  pattern for another asendic_* translation: check whether the surface your
//  TODO names already exists before you write the gate.
//
//  SOFT_SIMULTE is NOT defined: the #ifndef SOFT_SIMULTE (real-machine) branches
//  are compiled, the #ifdef SOFT_SIMULTE branches are not -- i.e. the REAL logic
//  is translated, driven via the Sim HAL.
//
//  Big5: every Chinese comment decoded cleanly via cp950 and is preserved as
//  UTF-8.  NO U+FFFD is emitted.
// =============================================================================
#include "asendic_Empty.h"
#include "csystem.h"
//---------------------------------------------------------------------------
#include "asendic.h"
#include "Motor/mymotor.h"
#include "mycylin.h"
#include "mysensor.h"
#include "myswitch.h"
#include "cprod.h"
#include "cmydef.h"
#include "cpublic.h"
#include "FormsFacade.h"        // fAGV (was AGV.h / VCL TfAGV)
#include "canary_support.h"     // LastSet, __FUNC__, ShowErrorMessage, etc.
//AI(W906-W6.1-EMPTYfix) 20260801: added the three headers below when the two #if 0
//  gates were retired -- they are the ported homes of the two surfaces golden pulled
//  from uHGemHT9045.h (EventReport/SECS_EVENT) and mymessbox.h (MyMessageBox).
#include "SECSGEM/SecsEventType.h"      // SECS_EVENT      (was uHGemHT9045.h)
#include "SECSGEM/SecsEventReport.h"    // EventReport()   (was uHGemHT9045.h)
#include "acatchtray_shims.h"           // MyMessageBox    (was mymessbox.h)
// BCB6 also pulled: main.h, note.h, acatchtray.h, OCR.h, OCRInsp.h, cMyDB.h
//                   -- all swapped above.

void InitUnLoadNewEmptyTrayTask();                                              //kevin 20151102
bool DoUnLoadNewEmptyToStack();                                                 //kevin 20151102

int iAutoEmptyReceiveTask=1;
int iLoadNewEmptyTrayToCarTask=1;
TQPF_Timer EmptyDelay;
TQPF_Timer EmptyMidCylinTime;                                                   //JerryYang 20161128 add
TQPF_Timer EmptyReadTrayID;                                                     //kevin 20210623 read tray id time out
//---------------------------------------------------------------------------
void InitLoadNewEmptyTrayToCarTask()
{
    iLoadNewEmptyTrayToCarTask=1;
}
//---------------------------------------------------------------------------
bool DoLoadNewEmptyTrayToCar()
{
//#ifndef SOFT_SIMULTE
    int ret;
//#endif
    static bool bEmptyHasDuplicateError=false;                                  //Steven 20120220 : Empty Tray重複Alarm Flag
    int &Task=iLoadNewEmptyTrayToCarTask;
    AnsiString SData="";                                                        //kevin 20210623
    static int iRead2D=0;                                                       //kevin 20211116 Tray 2D read state

    switch(Task)
    {
        case 1:
            if(MOT[MMEmptyZ].fHasTray)
            {
                EmptyDelay.SetMSAndOn(1000);                                    //Steven 20130904 : 加入Time Out 避免Hang Up
                Task=400;
                break;
            }

            if(Sen[SenEmptySelectHasTray].IsOff()==false ||
               LastSet.iRealDummy==DUMMY)
            {
                if(Sen[SenEmptyCCWDete].IsOn()   ||
                   Sen[SenEmptyCarHasTray].IsOn())
                {
                    MOT[MMEmpty_Car].SetTray(NULL_IC, __FUNC__);
                    MOT[MMEmptyZ].ClearTray(__FUNC__);
                    Task=1;
                    return true;
                }

                if(CosFunction.bUseEmptyColorTrayPreAlarm==true &&              //Ifor 20170322 (wei) add 開啟 Empty/Color Tray Pre Alarm 功能 不在此處Alarm
                   IniConfig.bP32EmptyColorTrayPreAlarm==true)
                {
                }
                else
                {
                    if(Sen[SnEmptyIsFull].IsOn()==true)                         //jou 2015-08-25 Tray limit sensor at Color Tray
                    {
                        //AI(W906-W6.1-EMPTYfix) 20260801: retired the #if 0 TODO(W7-SECS)
                        //  gate -- SECSGEM/SecsEventReport.h now supplies a real
                        //  EventReport(unsigned) and SecsEventType.h supplies
                        //  SECS_EVENT.NoCoverTray_Normal (=244), so the golden call
                        //  (golden :83-84) is restored verbatim, brace-less as golden.
                        if(IniConfig.bEnable_SECS_GEM==true)                    //JerryYang 20240318 : add
                            EventReport(SECS_EVENT.NoCoverTray_Normal);

                        ShowErrorMessage("MES1020", K_RETRY, MMEmpty_Car);
                        return false;
                    }
                }
                Task=50;
            }
            else
            {
                if(AUTO_EMPTY_COLOR!=0)                                         //jou 2011-12-21 有發生Tray feed完,汽缸還在上面,所以做完下降
                {
                    if(MOT[MMEmpty_Car].fHasTray==false)
                    {
                        if(Cylinder[C_Empty_Middle].OffStatus()==false &&       //汽缸在上面
                           Cylinder[C_Empty_Middle].Enable==true)               //Ztex 2024.08.13 Add Check Select Has Tray
                        {
                            if(Sen[SenEmptyCarHasTray].IsOff() && Sen[SenEmptyCCWDete].IsOff())
                            {
                                Cylinder[C_Empty_Up].Off();
                                Cylinder[C_Empty_Middle].Off();
                                EmptyMidCylinTime.SetSecAndOn(5.0);
                                Task=20;                                        //JerryYang 20161128 新增防護,避免Empty的Middle氣缸sensor一直亮著導致hang up
                                break;
                            }
                        }
                    }
                }
                ShowErrorMessage("MES1021", K_RETRY, MMEmpty_Car);
            }
            break;
        case 20:                                                                //JerryYang 20161128 新增防護,避免Empty的Middle氣缸sensor一直亮著導致hang up
            Cylinder[C_Empty_Up].Off();
            Cylinder[C_Empty_Middle].Off();
            if(Cylinder[C_Empty_Middle].OffStatus()==false &&                   //汽缸在上面
               Cylinder[C_Empty_Middle].Enable==true)                           //Ztex 2024.07.27 Add Check Has Use Cylinder
            {
                if(Sen[SenEmptyCarHasTray].IsOff() && Sen[SenEmptyCCWDete].IsOff())
                {
                    if(EmptyMidCylinTime.Off())
                    {
                        ShowErrorMessage("JAM1006", K_RETRY, MMEmpty_Car);      //Empty tray separation cylinder(lower) error
                        Task=1;
                    }
                    break;
                }
            }
            Task=1;
            break;
        case 50:
            if(IniConfig.bG11ASEReport)
                Task=52;
            else
                Task=60;
            break;
        case 52:
            bReadColorTrayID=false;
            SData="@e02026";
            RespondASECom(SData);                                               //kevin 20150818 20150610 回應 ase Reset finish
            EmptyReadTrayID.SetSecAndOn(7);                                     //kevin 20210623 read tray id time out
            RecordProcess("<AutoMove>Empty Query Tray ID.");                    //kevin 20210623 add 讀取 tray id
            iRead2D=0;                                                          //kevin 20211116 Tray 2D read state
            Task=53;
            break;
        case 53:
            if(bReadColorTrayID)                                                //kevin 20210623 TRAY ID Read
            {
                if(TrayID[1][0]=="")
                {
                    iRead2D=0;
                    Task=52;
                    break;
                }
                SData.sprintf("<AutoMove>Empty Get Tray ID,%s",TrayID[1][0]);   //kevin 20210623  [6]: load empty  [2]: 0:read  1: send pick pos
                ReadWriteTrayID(false);                                         //kevin 20220618 record color tray id
                RecordProcess(SData);                                           //kevin 20210623 add 讀取 tray id
                Task=60;
            }
            else if(EmptyReadTrayID.Off())                                      //kevin 20210623 read tray id time out
            {
                if(iRead2D>=1)                                                  //kevin 20211116 send 2D 不經EKeep
                {
                    ret=ShowErrorMessage("WAR16335", K_RETRY, MMSystem);        //kevin 20220705 only Retry
                    if(ret==K_RETRY)
                    {
                        iRead2D=0;
                        Task=52;
                    }
                    else
                    {
                        TrayID[2][0]="NoTrayID";
                        SData.sprintf("<AutoMove>Empty Get Tray ID,NoTrayID");  //kevin 20210623  [6]: load empty  [2]: 0:read  1: send pick pos
                        RecordProcess(SData);                                   //kevin 20210623 add 讀取 tray id
                        Task=60;
                    }
                }
                else
                {
                    SData="@HTColor";                                           //kevin 20211116 send 2D 不經EKeep
                    RespondASECom(SData);                                       //kevin 20150818 20150610 回應 ase Reset finish
                    iRead2D++;
                    EmptyReadTrayID.SetSecAndOn(5);                             //kevin 20210623 read tray id time out
                }
            }
            break;
        case 60:
            if(CylinderUp(C_Empty_Up))
            {
                MOT[MMEmptyZ].ClearTray(__FUNC__);
                Task=100;
            }
            break;
        case 100:
            if(LastSet.iRealDummy==DUMMY)                                       //Steven 20100618
                Task=200;
            else if(Cylinder[C_EmptyLoaderZ_Select].Push())
                Task=200;
            break;
        case 200:
            if(CylinderMiddle(C_Empty_Up))
            {
                if(bEmptyPause)
                {
                    bEmptyPause=false;
                    ShowMyMessage("Empty split wait", "Empty 分離暫停", "DoLoadNewEmptyTrayToCar 200");
                    Task=200;
                    break;
                }
                Task=300;
            }
            break;
        case 300:
            if(LastSet.iRealDummy==DUMMY)                                       //Steven 20111212
            {
                Task=400;
            }
            else if(Cylinder[C_EmptyLoaderZ_Select].Pop())
            {
                Task=400;
            }
            EmptyDelay.SetMSAndOn(1000);                                        //Steven 20130904 Start: 加入Time Out 避免Hang Up
            break;
        case 400:
            if(EmptyDelay.Off())
            {
                Task=410;
            }
            break;
        case 410:
            if(CylinderLower(C_Empty_Up))
            {
                EmptyDelay.SetMSAndOn(1000);
                Task=420;
            }
            break;
        case 420:
            if(bHandlerPause)                                                                                           //Steven 20190123 : 紀錄Handler被暫停, 重置Timer
            {
                EmptyDelay.SetMSAndOn(1000);
            }

#ifndef SOFT_SIMULTE
            if((Sen[SenEmptyCCWDete].IsOn()     ||
                Sen[SenEmptyCarHasTray].IsOn()) ||
               LastSet.iRealDummy==DUMMY)
#endif
            {
                MOT[MMEmpty_Car].SetTray(NULL_IC, __FUNC__);
                MOT[MMEmptyZ].ClearTray(__FUNC__);
                Task=1;
                bEmptyHasDuplicateError=false;
                iTrayTotal[1]++;                                                                                        //kevin 20210623  0:loader 1:Empty  2:olor  Tray 入軌道數量
                TrayID[1][1]=TrayID[1][0];                                                                              //kevin 20220810 修正  [6]: load empty  [2]: 0:read  1: send pick pos
                TrayID[1][0]="";                                                                                        //kevin 20220810 修正
                return true;
            }
#ifndef SOFT_SIMULTE
            else if(EmptyDelay.Off())
            {
                ret=ShowErrorMessage("JAM1013", K_RETRY|K_SKIP, MMEmpty_Car,bEmptyHasDuplicateError,__FUNC__);          //Steven 20150108 : JAM1001 --> JAM1013
                bEmptyHasDuplicateError=true;
                if(ret==K_SKIP)
                {
                    MOT[MMEmptyZ].ClearTray(__FUNC__);
                    bEmptyHasDuplicateError=false;
                    Task=1;
                }
            }
            break;
#endif
    }
    return false;
}
//------------------------------------------------------------------------------
int iEmptyTrayToFrontTask=1;
TQPF_Timer hEmptyTrayToFront;
TQPF_Timer hEmptyTrayToFrontdelay;
TQPF_Timer hEmptyTrayToFrontForDummy;
TQPF_Timer EmptyUnloadDelay;
void InitEmptyTrayToFrontTask()
{
    iEmptyTrayToFrontTask=1;
}
//------------------------------------------------------------------------------
bool DoEmptyTrayToFront()
{
    int &Task=iEmptyTrayToFrontTask;
    int ret;
    static int iDelayCnt=0;
    static bool bEmptyHasDuplicateError=false;                                  //Steven 20120220 : Empty Tray重複Alarm Flag
    switch(Task)
    {
        case 1:
            hEmptyTrayToFront.SetMSAndOn(20000);
            hEmptyTrayToFrontForDummy.SetMSAndOn(5000);
            hEmptyTrayToFrontdelay.SetMSAndOn(1000);                            //JerryYang 20220823 : 避免軌道入TRAY到一半又退TRAY會卡住, 先停止1秒再下指令
            TrayMoveIn(false, 1);
//            StartTick=MyTickCount();
            iDelayCnt=0;
            Task=100;
            // NOTE: faithful fall-through to case 100 (no break in golden :303)
        case 100:
            if(bHandlerPause)                                                                                           //Steven 20190123 : 紀錄Handler被暫停, 重置Timer
            {
                Task=1;
                break;
            }

            if(hEmptyTrayToFrontdelay.Off()==false)                                                                     //JerryYang 20220823 : 避免軌道入TRAY到一半又退TRAY會卡住, 先停止1秒再下指令
            {
                break;
            }

            Cylinder[C_Empty_Fix].Off();
            if(SUPPORT_2_EMPTY_EMPTY)
            {
                TrayMoveOut(false, 1);
                if(Cylinder[C_ColorTrackFloodgate].Enable)
                    Cylinder[C_ColorTrackFloodgate].Off();
                MOT[MMEmpty_Car].SetTray(NULL_IC, __FUNC__);
                MOT[MMEmpty].ClearTray(__FUNC__);
                bEmptyHasDuplicateError=false;                                                                          //Steven 20120220 : Empty Tray重複Alarm Flag
                Task=1;
                return true;
            }

            TrayMoveOut(true, 1);
            if(TRAY_ARM_MODE==eUnderCoveyor)
                TrayCylinMoveOut(1);

            if((Sen[SenEmptyCarHasTray].IsOff()==false &&
                Sen[SenEmptyCCWDete].IsOff()==false) ||                                                                 //Sam 20211118 : 增加保護
                LastSet.iRealDummy==DUMMY)
            {
                iDelayCnt++;
                if(iDelayCnt<10)                                                                                        //Sam 20211119 : 到位多檢查一下 Delay
                    break;

                EmptyUnloadDelay.SetSecAndOn(Ld_UldDelayTime.ULD_LiftDownDelay);
                Task=200;
                break;
            }
            else
            {
                iDelayCnt=0;
            }

            if(hEmptyTrayToFrontForDummy.Off())
            {
                if(LastSet.iRealDummy==DUMMY &&
                   Sen[SenEmptyCarHasTray].IsOff()==false)                                                              //kevin 20220712
                {
                    MOT[MMEmpty_Car].SetTray(NULL_IC, __FUNC__);
                    MOT[MMEmpty].ClearTray(__FUNC__);
                    bEmptyHasDuplicateError=false;                                                                      //Steven 20120220 : Empty Tray重複Alarm Flag
                    Task=1;
                    return true;
                }
            }

            if(hEmptyTrayToFront.Off())
            {
                ret=ShowErrorMessage("JAM1012", K_SKIP|K_RETRY, MMEmpty, bEmptyHasDuplicateError, __FUNC__);            //Steven 20120925 : JAM1009 -> JAM1012
                if(ret==K_RETRY)
                {
                    bEmptyHasDuplicateError=true;                                                                       //Steven 20120220 : Empty Tray重複Alarm Flag
                    Task=1;
                }
                else
                {
                    bEmptyHasDuplicateError=false;                                                                      //Steven 20120220 : Empty Tray重複Alarm Flag
                    MOT[MMEmpty].ClearTray(__FUNC__);
                    Task=1;
                    return true;
                }
            }
            break;
        case 200:
            if(EmptyUnloadDelay.Off())
                Task=300;
            break;
        case 300:
            MOT[MMEmpty_Car].SetTray(NULL_IC, __FUNC__);
            if(MOT[MMEmpty].Tray.iNeedManualRemoved>=0)                         //Sam 20230317 : 修正 P24 功能
            {
                MOT[MMEmpty_Car].Tray.iNeedManualRemoved=MOT[MMEmpty].Tray.iNeedManualRemoved;
                MOT[MMEmpty].Tray.iNeedManualRemoved=-1;
            }
            MOT[MMEmpty].ClearTray(__FUNC__);
            bEmptyHasDuplicateError=false;                                      //Steven 20120220 : Empty Tray重複Alarm Flag
            if(TRAY_ARM_MODE==eUnderCoveyor)
                TrayCylinMoveIn(1);
            Task=1;
            return true;
    }
    return false;
}
//------------------------------------------------------------------------------
int iEmptyTrayToRearTask=1;
TQPF_Timer hEmptyTrayToRear;
TQPF_Timer hEmptyTrayToRearForDummy;
TQPF_Timer hEmptyUnderCoveyorDelay;
void InitEmptyTrayToRearTask()
{
    iEmptyTrayToRearTask=1;
}
//------------------------------------------------------------------------------
//---- 收空TRAY----
bool DoEmptyTrayToRear()
{
    static bool bEmptyHasDuplicateError=false;                                  //Steven 20120220 : Empty Tray重複Alarm Flag
    int &Task=iEmptyTrayToRearTask;
    int ret;
    bool bColorFlag=false;
    bool bSOFT_SIMULTE=false;                                                   //Ztex 2023.07.28 Add bSOFT_SIMULTE
    #ifdef SOFT_SIMULTE
        bSOFT_SIMULTE=true;
    #endif

    switch(Task)
    {
        case 1:
            hEmptyTrayToRearForDummy.SetMSAndOn(5000);
            hEmptyTrayToRear.SetMSAndOn(20000);
            Task=100;
            // NOTE: faithful fall-through to case 100 (no break in golden :427)
        case 100:
            if(TRAY_ARM_MODE==eUnderCoveyor)
                TrayCylinMoveOut(1);                                            //需要往front 到等待點
            if(Cylinder[C_Empty_Fix].Pop())
            {
                Task=200;
            }
            break;
        case 200:
            if(bHandlerPause || bOCRRunTime)                                                                                                                    //Steven 20190123 : 紀錄Handler被暫停, 重置Timer
            {
                bOCRRunTime=false;
                Task=1;
                break;
            }

            if(MOT[MMEmpty].fHasTray && MOT[MMEmpty_Car].fHasTray)                                                                                              //Steven 20130917 : Double Tray問題,測試 Q4
            {
                Task=1;
                return true;
            }

            TrayMoveIn(true, 1);

            if(DOUBLE_BELT_MODE==1)                                                                                                                             //Ztex 2023.12.13 Add Double Belt Mode
            {
                if(bSOFT_SIMULTE==true ||
                   Sen[SenEmptyHasTray].IsOn()==true)
                {
                    hEmptyUnderCoveyorDelay.SetSecAndOn(0.2);
                    Task=430;
                    break;
                }
            }
            else if(TRAY_ARM_MODE==eUnderCoveyor)
            {
                if(Sen[SnEmptySeparateHasTray].IsOff()==false)
                {
                    hEmptyUnderCoveyorDelay.SetSecAndOn(0.2);
                    Task=430;
                    break;
                }
            }
            else
            {
                if(Sen[SenEmptyHasTray].IsOff()==false &&
                   Sen[SenEmptyCWDete].IsOff()==false)                                                                                                          //Sam 20220919 : 增加保護, 換位置
                {
                    Task=450;
                    break;
                }
            }

            if(hEmptyTrayToRearForDummy.Off())
            {
                if(LastSet.iRealDummy==DUMMY)
                {
                    Task=450;
                    break;
                }
            }

            if(hEmptyTrayToRear.Off())
            {
                if(MOT[MMEmpty_Car].fHasTray==false)                                                                                                            //Sam 20190804 : Fix Empty_Car 補盤到 Empty 動作未完成時，剛好 TrayArm 要放 Tray，觸發 DoAutoEmptyReceive 機制將 Empty_Car 收起來，但 TrayArm 原本要放的 Tray 跑去補 Auto Tray。
                {
                    Task=1;
                    TrayMoveIn(false, 1);
                    if(Cylinder[C_ColorTrackFloodgate].Enable)
                        Cylinder[C_EmptyTrackFloodgate].Off();
                    MOT[MMEmpty_Car].ClearTray(__FUNC__);
                    bEmptyHasDuplicateError=false;
                    return true;
                }

                ret=ShowErrorMessage("JAM1001", K_SKIP|K_RETRY, MMEmpty_Car, bEmptyHasDuplicateError, "DoEmptyTrayToRear_200");                                 //Steven 20120925 : JAM1009 -> JAM1001
                if(ret==K_RETRY)
                {
                    bEmptyHasDuplicateError=true;                                                                                                               //Steven 20120220 : Empty Tray重複Alarm Flag
                    Task=1;
                }
                else
                {
                    bEmptyHasDuplicateError=false;                                                                                                              //Steven 20120220 : Empty Tray重複Alarm Flag
                    MOT[MMEmpty_Car].ClearTray(__FUNC__);
                    Task=1;
                    return true;
                }
            }
            break;
        case 300:
            if(Cylinder[C_Empty_Fix].Push())
                Task=400;
            break;
        case 400:
            if(Sen[SenEmptyFixCyPush].IsOn())
            {
                if(LastSet.iRealDummy==DUMMY)
                {
                    bEmptyHasDuplicateError=false;                                                                                                              //Steven 20120220 : Empty Tray重複Alarm Flag
                    Task=450;
                }
                else
                {
                    Cylinder[C_Empty_Fix].Off();
                    ret=ShowErrorMessage("JAM1003", K_RETRY, MMEmpty, bEmptyHasDuplicateError, "DoEmptyTrayToRear_400");                                        //Steven 20120925 : JAM1009 -> JAM1003
                    bEmptyHasDuplicateError=false;                                                                                                              //Steven 20120220 : Empty Tray重複Alarm Flag
                    Task=300;
                    break;
                }
            }
            else
            {
                bEmptyHasDuplicateError=false;                                                                                                                  //Steven 20120220 : Empty Tray重複Alarm Flag
                Task=500;
            }
            break;
        case 420:
            if(Sen[SenEmptyHasTray].IsOff()==false && Sen[SenEmptyCWDete].IsOff()==false)
            {
                Task=450;
            }
            break;
        case 430:
            if(hEmptyUnderCoveyorDelay.Off())
            {
                Cylinder[C_Empty_Fix].On();
                Task=450;
            }
            break;
        case 450:
            hEmptyTrayToRear.SetSecAndOn(Ld_UldDelayTime.LD_TrayArrivalDely);
            Task=460;
            break;
        case 460:
            if(hEmptyTrayToRear.Off())
            {
                TrayCylinMoveIn(1);
                TrayMoveIn(false, 1);
                if(Cylinder[C_EmptyTrackFloodgate].Enable)
                    Cylinder[C_EmptyTrackFloodgate].Off();

                if(TRAY_ARM_MODE==eUnderCoveyor)
                {
                    hEmptyUnderCoveyorDelay.SetSecAndOn(3);
                    Task=470;
                }
                else
                {
                    Task=500;
                }
            }
            break;
        case 470:
            if(LastSet.iRealDummy==DUMMY ||
               (Sen[SenEmptyHasTray].IsOff()==false &&
                Sen[SnEmptySeparateHasTray].IsOff()==false))
            {
                Task=500;
            }
            else if(hEmptyUnderCoveyorDelay.Off())
            {
                ShowErrorMessage("WAR1051", K_RETRY, MMEmpty, false, "DoEmptyTrayToRear");                              //Wenqi 20240516 Modify Empty Tray Protect Error
                hEmptyUnderCoveyorDelay.SetSecAndOn(3);
                break;
            }
            break;
        case 500:
            if(USE_COLOR_TRAY_SENSOR==1 &&
               TrayForm.bColorTray &&
               LastSet.iRealDummy!=DUMMY &&
               Sen[SnEmptyColorTrayDetect].Enable)                                                                      //JerryYang 20200812 : add Empty的color sensor
            {
                bColorFlag=Sen[SnEmptyColorTrayDetect].IsOff();
                if(bColorFlag==true)                                                                                    //Steven 20150427 : For Color Sensor, 滅掉是紅色    //Steven 20150609 : 客戶要把Color流道的Tray搬到Auto 1,所以必須是黑色的
                {
                    ShowErrorMessage("WAR1051", K_RETRY, MMEmpty, false, "DoAutoEmpty");                                //WAR1451
                    break;
                }
            }
            Task=510;
            break;
        case 510:
            if(Cylinder[C_Empty_Fix].Push())
            {
                MOT[MMEmpty].SetTray(NULL_IC, __FUNC__);
                MOT[MMEmpty_Car].ClearTray(__FUNC__);                           //Steven 20111109 : 已入Tray卻未清掉資料
                Task=1;
                return true;
            }
            break;
    }
    return false;
}
//------------------------------------------------------------------------------
int iAutoEmptyTask=1;
TQPF_Timer DoAutoEmptyDelay;
TQPF_Timer CheckEmptyTrayDelay;
void InitAutoEmptyTask()
{
    iAutoEmptyTask=1;
}
//------------------------------------------------------------------------------
void DoAutoEmpty()
{
    int i;
    int &Task=iAutoEmptyTask;
    static bool bEmptyHasDuplicateError=false;                                  //Steven 20120220 : Empty Tray重複Alarm Flag
    bool bColorFlag=false;
    (void)bEmptyHasDuplicateError;                                              //AI(W6.1): retained per golden; not read on this path
    if(iReceiveEmptyTray!=0)
    {
        if(CUSTOMER_CODE==CC_SCC && bIsCatchingFromBuffer==true)                //RogerYang 20260225 : JSCC防止夾tray的時候Color/Empty誤退，驗證中
        {
            InitAutoEmptyReceiveTask();
            return;
        }
        DoAutoEmptyReceive();
        return;
    }

    if(fEmptyCanSupplyNewTray==false)
        return;
    if(CosFunction.bSpecialP24)                                                 //Sam 20230320 : 整合 P24 北興版為獨立功能碼 //Alick 20160907 Modify for SIGRUD PeiXing TrayAssignment 設定成different時造成與P24.25功能衝突
    {
        if(IniConfig.bP24SkipEventNeedRemoveEmptyAndColorTray)                  //Sam 20230406 : 修正矽格北興 P25 功能失效問題
        {
        }
        else
        {
            if(IniConfig.bP25EmptyColorNoSuppleAutoNoLoadEmpty &&
               ((TrayForm.LodareType==0 && TrayForm.LoaderToEmptyColor[iRunStartMode]==1) ||
                (TrayForm.LodareType==1 && TrayForm.AutoFromEmptyColor[iRunStartMode][eAuto1]==1 &&
                                           TrayForm.AutoFromEmptyColor[iRunStartMode][eAuto2]==1 &&
                                           TrayForm.AutoFromEmptyColor[iRunStartMode][eAuto3]==1)))                     //kevin 20160130 empty no supple Auto1-3
            {
                return;
            }
        }
    }
    else
    {
        if(TrayForm.bEnableAMR) //Eastsun 20260515 F019 AMR skip empty supple
        {
        }
        else if(IniConfig.bP25EmptyColorNoSuppleAutoNoLoadEmpty &&
                IniConfig.bA65_BundleIDList==false &&
                ((TrayForm.LodareType==0 && TrayForm.LoaderToEmptyColor[iRunStartMode]==1) ||
                (TrayForm.LodareType==1 && TrayForm.AutoFromEmptyColor[iRunStartMode][eAuto1]==1 &&
                TrayForm.AutoFromEmptyColor[iRunStartMode][eAuto2]==1 &&
                TrayForm.AutoFromEmptyColor[iRunStartMode][eAuto3]==1)))        //kevin 20160130 empty no supple Auto1-3
        {
            return;
        }
    }

    switch(Task)
    {
        case 1:
            if(MOT[MMEmpty].fHasTray ||
               (Sen[SenEmptyHasTray].IsOn() &&
                Sen[SenEmptyCWDete].IsOn()))                                    //Sam 20211118 : 增加保護
            {
                Task=20;
                break;
            }
            else
            {
                MOT[MMEmpty].ClearTray(__FUNC__);
            }

            if(MOT[MMEmpty].fHasTray==false)
            {
                Task=100;

                i=WhichAutoNeedTray();                                          //which auto need tray
                if(MOT[MMTrayY].fHasTray && MOT[MMTrayY].HasIC()==false && i==0)
                {
                    Task=1;
                    break;
                }
            }
            break;
        case 20:
            if(USE_COLOR_TRAY_SENSOR==1 &&
               TrayForm.bColorTray &&
               LastSet.iRealDummy!=DUMMY &&
               Sen[SnEmptyColorTrayDetect].Enable)                                                                      //JerryYang 20200812 : add Empty的color sensor
            {
                bColorFlag=Sen[SnEmptyColorTrayDetect].IsOff();
                if(bColorFlag==true)                                                                                    //Steven 20150427 : For Color Sensor, 滅掉是紅色    //Steven 20150609 : 客戶要把Color流道的Tray搬到Auto 1,所以必須是黑色的
                {
                    ShowErrorMessage("WAR1051", K_RETRY, MMEmpty, false, "DoAutoEmpty");                                //WAR1451
                    break;
                }
            }
            Cylinder[C_Empty_Fix].Off();
            Task=30;
            break;
        case 30:
            if(Cylinder[C_Empty_Fix].Pop())
                Task=40;
            break;
        case 40:
            if(Sen[SenEmptyFixCyPush].IsOn())
            {
                if(LastSet.iRealDummy==DUMMY)
                {
                    bEmptyHasDuplicateError=false;                                                                      //Steven 20120220 : Empty Tray重複Alarm Flag
                    Task=50;
                }
                else
                {
                    Cylinder[C_Empty_Fix].Off();
                    ShowErrorMessage("JAM1003", K_RETRY, MMEmpty, bEmptyHasDuplicateError, __FUNC__);                   //Steven 20120925 : JAM1009 -> JAM1003
                    bEmptyHasDuplicateError=true;                                                                       //Steven 20120220 : Empty Tray重複Alarm Flag
                    Task=1;
                }
            }
            else
            {
                bEmptyHasDuplicateError=false;                                                                          //Steven 20120220 : Empty Tray重複Alarm Flag
                Task=50;
            }
            break;
        case 50:
            if(Cylinder[C_Empty_Fix].Pop())
            {
                #ifdef SOFT_SIMULTE
                MOT[MMEmpty_Car].ClearTray(__FUNC__);
                MOT[MMEmpty].SetTray(NULL_IC, __FUNC__);
                #endif
                Task=60;
            }
            break;
        case 60:
            if(MOT[MMEmpty].fHasTray==false)
            {
                DoAutoEmptyDelay.SetMSAndOn(1000);
                Task=70;
            }
            else
            {
                CheckEmptyTrayDelay.SetMSAndOn(100);
                Task=65;
            }
            break;
        case 65:
            if(CheckEmptyTrayDelay.Off())
            {
                if(MOT[MMEmpty].fHasTray==true &&
                   LastSet.iRealDummy>=HAS_TRAY &&
                   Sen[SenEmptyHasTray].IsOff() &&
                   Sen[SenEmptyCWDete].IsOff())
                {
                    MOT[MMEmpty].ClearTray(__FUNC__);
                }
                Task=60;

                #ifndef SOFT_SIMULTE
                if(MOT[MMEmpty].fHasTray==false        &&
                   LastSet.iRealDummy>=HAS_TRAY        &&
                   Sen[SenEmptyHasTray].IsOff()==false &&
                   Sen[SenEmptyCWDete].IsOff()==false)                          //Steven 20130930
                {
                    MOT[MMEmpty].SetTray(NULL_IC, __FUNC__);
                }

                if(MOT[MMEmpty_Car].fHasTray==false       &&
                   LastSet.iRealDummy>=HAS_TRAY           &&
                   Sen[SenEmptyCarHasTray].IsOff()==false &&
                   Sen[SenEmptyCCWDete].IsOff()==false)                         //Steven 20130930
                {
                    MOT[MMEmpty_Car].SetTray(NULL_IC, __FUNC__);
                }
                #endif

                if(MOT[MMTrayY].fHasTray        &&
                   MOT[MMTrayY].HasIC()==false  &&
                   MOT[MMEmpty].fHasTray        &&
                   MOT[MMEmpty_Car].fHasTray)
                {
                    iReceiveEmptyTray=2;
                    iAutoEmptyReceiveTask=200;
                }

                if(MOT[MMEmpty].fHasTray && MOT[MMEmpty_Car].fHasTray)          //Steven 20130917 : Double Tray問題,測試 Q4
                {
                    iReceiveEmptyTray=2;
                    iAutoEmptyReceiveTask=200;
                }
            }
            break;
        case 70:
            if(DoAutoEmptyDelay.Off())
            {
                if((Sen[SenEmptyHasTray].IsOn() &&
                    Sen[SenEmptyCWDete].IsOn() &&
                    MOT[MMEmpty].fHasTray==false) ||                            //Sam 20211118 : 增加保護
                   (LastSet.iRealDummy==DUMMY &&
                    fAGV->IsATK_AMR()==false &&
                    MOT[MMEmpty].fHasTray==false))
                {
                    MOT[MMEmpty_Car].ClearTray(__FUNC__);
                    MOT[MMEmpty].SetTray(NULL_IC, __FUNC__);
                }
                Task=100;
            }
            break;
        case 100:
            if(MOT[MMEmpty_Car].fHasTray)
            {
                if(MOT[MTrayX].fHasTray &&
                   MOT[MTrayX].ReadPos()==Prod.iXTrayEmpty &&
                   bIsPlacingToBuffer==true)                                    //JerryYang 20250828 : fix Empty誤退TRAY
                {
                    if(iReceiveEmptyTray==0)                                    //Sam 20211119 : 放 Tray 增加保護
                    {
                        fEmptyCanSupplyNewTray=false;
                        iReceiveEmptyTray=1;
                        InitAutoEmptyReceiveTask();
                    }
                    DoAutoEmptyDelay.SetMSAndOn(2000);
                    Task=150;
                }
                else
                {
                    InitEmptyTrayToRearTask();
                    Task=300;
                }
            }
            else
            {
                InitLoadNewEmptyTrayToCarTask();
                Task=200;
                if(iReceiveEmptyTray!=0 ||                                      //jou 980421 empty tray no need relay one tray in empty_car
                   (MOT[MTrayX].fHasTray &&
                    MOT[MTrayX].ReadPos()==Prod.iXTrayEmpty) ||
                    MOT[MMEmpty].fHasTray)
                {
                    DoAutoEmptyDelay.SetMSAndOn(2000);
                    Task=150;
                }
            }
            break;
        case 150:
            if(DoAutoEmptyDelay.Off())
                Task=1;
            break;
        case 200:
            bLoadNewEmptyTrayToCarStart=true;                                   //Sam 20211119 : 放 Tray 增加保護
            if(DoLoadNewEmptyTrayToCar())
            {
                bLoadNewEmptyTrayToCarStart=false;                              //Sam 20211119 : 放 Tray 增加保護
                InitLoadNewEmptyTrayToCarTask();
                Task=1;
            }
            break;
        case 300:
            if(DoEmptyTrayToRear())
            {
                Task=1;
            }
            break;
    }
}
//------------------------------------------------------------------------------
int iUnLoadNewEmptyTrayTask=1;
TQPF_Timer DoUnLoadNewEmptyToStackDelay;
void InitUnLoadNewEmptyTrayTask()
{
    iUnLoadNewEmptyTrayTask=1;
}
//------------------------------------------------------------------------------
bool DoUnLoadNewEmptyToStack()
{
    int &Task=iUnLoadNewEmptyTrayTask;

    switch(Task)
    {
        case 1:
            #ifndef SOFT_SIMULTE
            if(TrayMoveStatus(1,__FUNC__)==0 &&                                 //Sam 20210325 : 整合 tray move 狀態
               LastSet.iRealDummy!=DUMMY)                                       //Steven 20130930
            {
                Task=10;
                break;
            }
            #endif
            MOT[MMEmptyZ].SetTray(NULL_IC, __FUNC__);

            if(Sen[SenEmptyCarHasTray].IsOff() &&
               Sen[SenEmptyCCWDete].IsOff() &&                                  //kevin 20120518 有tray在軌道上升到分離高度
               Cylinder[C_EmptyLoaderZ_Select].OnStatus())                      //Sam 20210728 : 確保分離汽缸已經被打開
                Task=200;
            else
                Task=50;
            break;
        case 10:
            if(Sen[SenEmptyHasTray].IsOn() &&
               Sen[SenEmptyCWDete].IsOn())                                      //Steven 20130917 : Double Tray問題,測試 Q4
            {
                Task=50;
                break;
            }
            else
            {
                TrayMoveOut(true, 1);
            }
            DoUnLoadNewEmptyToStackDelay.SetMSAndOn(1000);
            Task=20;
            break;
        case 20:
            if(DoUnLoadNewEmptyToStackDelay.Off())
                Task=1;
            break;
        case 50:
            #ifndef SOFT_SIMULTE
            if(TrayMoveStatus(1, __FUNC__)==0 &&                                //Sam 20211008 : 確保汽缸上前的狀態皮帶是一直再帶動轉
               LastSet.iRealDummy!=DUMMY)
            {
                TrayMoveOut(true, 1);
                DoUnLoadNewEmptyToStackDelay.SetMSAndOn(1000);
                Task=20;
                break;
            }
            #endif

            MOT[MMEmptyZ].SetTray(NULL_IC, __FUNC__);
            if(CylinderMiddle(C_Empty_Up))
            {
                TrayMoveOut(false, 1);                                          //Frank 20230901 : In --> Out
                if(Cylinder[C_EmptyTrackFloodgate].Enable)
                    Cylinder[C_EmptyTrackFloodgate].Off();
                Task=100;
            }
            break;
        case 100:
            if(LastSet.iRealDummy==DUMMY ||
               Cylinder[C_EmptyLoaderZ_Select].Push())
            {
                DoUnLoadNewEmptyToStackDelay.SetMSAndOn(500);
                Task=150;
            }
            break;
        case 150:
            if(DoUnLoadNewEmptyToStackDelay.Off())
                Task=200;
            break;
        case 200:
            if(CylinderUp(C_Empty_Up))
            {
                TrayMoveOut(false, 1);
                if(Cylinder[C_EmptyTrackFloodgate].Enable)
                    Cylinder[C_EmptyTrackFloodgate].Off();

                Task=300;
            }
            break;
        case 300:
            if(LastSet.iRealDummy==DUMMY ||
               Cylinder[C_EmptyLoaderZ_Select].Pop())
            {
                Task=400;
            }
            break;
        case 400:                                                               //kevin 20161129  empty 收tray sensor沒調好 需alarm
            if((Sen[SenEmptyCarHasTray].IsOff() &&
                Sen[SenEmptyCCWDete].IsOff()) ||
               LastSet.iRealDummy==DUMMY)                                       //kevin 20161129 沒有tray在軌道
            {
                DoUnLoadNewEmptyToStackDelay.SetSecAndOn(Ld_UldDelayTime.ULD_TrayBackDelay);
                MOT[MMEmptyZ].ClearTray(__FUNC__);
                Task=500;
            }
            else
            {
                #ifdef SOFT_SIMULTE
                MOT[MMEmptyZ].ClearTray(__FUNC__);                              //JerryYang 20161222 (jou) 修正軟體模擬hang up
                Task=500;
                #else
                ShowErrorMessage("MES1020", K_RETRY, MMEmpty_Car);
                #endif
            }
            break;
        case 500:
            if(DoUnLoadNewEmptyToStackDelay.Off())
            {
                if(CylinderLower(C_Empty_Up))
                {
                    Task=1;
                    return true;
                }
            }
            break;
    }
    return false;
}
//------------------------------------------------------------------------------
// 送空 tray
//------------------------------------------------------------------------------
void InitAutoEmptyReceiveTask()
{
    iAutoEmptyReceiveTask=1;
}
//------------------------------------------------------------------------------
void DoAutoEmptyReceive()
{
    int &Task=iAutoEmptyReceiveTask;
    AnsiString str1, str2;
    static bool bHasEmptyTrayPreAlarm=false;                                    //Ifor 20170315 (wei) add 新增Empty/Color Tray Pre Alarm 功能
    static bool bNeedAlarm=false;                                               //Sam 20230317 : 修正 P24 功能
    switch(Task)
    {
        case 1:
            InitEmptyTrayToFrontTask();
            Task=100;
            break;
        case 100:
            if(DoEmptyTrayToFront())
            {
                iReceiveEmptyTray=2;
                if(MOT[MMEmpty_Car].fHasTray)
                {
                    InitUnLoadNewEmptyTrayTask();
                    Task=200;
                }
                else
                {
                    Task=1;
                    iReceiveEmptyTray=0;
                }
            }
            break;
        case 200:
            if(CosFunction.bUseEmptyColorTrayPreAlarm==true &&                  //Ifor 20170322 (wei) add Empty/Color Tray Pre Alarm 功能於Tray上升前判斷與Alarm
               IniConfig.bP32EmptyColorTrayPreAlarm==true)
            {
                if(Sen[SnEmptyIsFull].IsOn()==true &&
                   bHasEmptyTrayPreAlarm==true)                                 //上升前若滿Tray || PreAlarm =true Show Alarm
                {
                    ShowErrorMessage("MES1020", K_RETRY, MMEmpty_Car);
                    break;
                }
                else
                {
                    bHasEmptyTrayPreAlarm=false;
                }
            }

            bUnLoadNewEmptyToStackStart=true;                                   //Sam 20211119 : 放 Tray 增加保護
            if(DoUnLoadNewEmptyToStack())
            {
                bUnLoadNewEmptyToStackStart=false;                              //Sam 20211119 : 放 Tray 增加保護
                if(MOT[MMEmpty_Car].Tray.iNeedManualRemoved>=0)                 //Sam 20230317 : 修正 P24 功能
                {
                    MOT[MMEmpty_Car].Tray.iNeedManualRemoved=-1;
                    bNeedAlarm=true;
                }
                else
                {
                    bNeedAlarm=false;
                }
                MOT[MMEmpty_Car].ClearTray(__FUNC__);
                Task=300;
            }
            break;
        case 300:
            if(CosFunction.bUseEmptyColorTrayPreAlarm==true &&                                                                                                  //Ifor 20170328 (wei) add Empty/Color Tray Pre Alarm 於入Tray上升後判斷是否有遮到滿Tray Sensor
               IniConfig.bP32EmptyColorTrayPreAlarm==true)
            {
                if(Sen[SnEmptyIsFull].IsOn()==true ||
                   (Sen[SnEmptyIsPreAlarm].Enable &&
                    Sen[SnEmptyIsPreAlarm].IsOn()==true))                                                                                                       //wei 20170802 Pre alarm sensor
                {
                    if(bHasEmptyTrayPreAlarm==false)
                    {
                        bHasEmptyTrayPreAlarm=true;
                        //AI(W906-W6.1-EMPTYfix) 20260801: retired the #if 0 TODO(W6.x)
                        //  gate -- acatchtray_shims.h now supplies TMyMessageBoxShim
                        //  *MyMessageBox (Visible/Close()), the same surface golden's
                        //  mymessbox.h TMyMessageBox* exposes here, so the golden
                        //  close-before-alarm call (golden :1105-1106) is restored.
                        if(MyMessageBox->Visible==true)                                                                                                         //Alarm 前若form有開啟先關閉
                            MyMessageBox->Close();
                        iUnLoaderCount=8;                                                                                                                       // 必須不為0 Handler才不停機
                        str1.sprintf("Empty Tray is full with trays, Please take it off");
                        str2.sprintf("Empty上的Tray盤已滿,請取下Tray盤");
                        RunInfo.iPreAlarmPosition=8;                                                                                                            //Ifor 20171024 (wei) : add Pre Alarm Position 1:LD 2:Auto1 3:Auto2 4:Auto3 5:Fix1 6:Fix2 7:Fix3 8:Empty 9:Color
                        ShowUnloaderTrayMessage(str1, str2);
                    }
                }
                else
                {
                    bHasEmptyTrayPreAlarm=false;
                }
                iReceiveEmptyTray=0;
                Task=1;
            }
            else
            {
                if(Sen[SnEmptyIsFull].IsOn()==true)                                                                                                             //Steven 20141120 : Enpty & Color 滿盤偵測
                {
                    ShowErrorMessage("MES1020", K_RETRY, MMEmpty_Car);
                }
                else
                {
                    if(bNeedAlarm)                                                                                                                              //Sam 20230317 : 修正 P24 功能
                    {
                        #ifdef SOFT_SIMULTE
                            ShowMyMessage("Please remove empty tray manually. And check if has IC inside the tray.", "請將 Empty 上的 Tray 手動取走並檢查有無 IC !!!");      //Sam 20240116 : 修改提示
                            bNeedAlarm=false;
                            bHasEmptyTrayPreAlarm=false;
                            iReceiveEmptyTray=0;
                            Task=1;
                        #else
                        if(Sen[SenEmptySelectHasTray].IsOn())
                        {
                            ShowMyMessage("Please remove empty tray manually. And check if has IC inside the tray.", "請將 Empty 上的 Tray 手動取走並檢查有無 IC !!!");      //Sam 20240116 : 修改提示
                            Task=300;
                            break;
                        }
                        else if(Sen[SenEmptySelectHasTray].IsOff())
                        {
                            bNeedAlarm=false;
                            bHasEmptyTrayPreAlarm=false;
                            iReceiveEmptyTray=0;
                            Task=1;
                        }
                        #endif
                    }
                    else
                    {
                        bHasEmptyTrayPreAlarm=false;
                        iReceiveEmptyTray=0;
                        Task=1;
                    }
                }
            }
            break;
    }
}
//------------------------------------------------------------------------------
int iAutoEmpty1ReceiveTask=1;
TQPF_Timer DoAutoEmpty1Delay;
TQPF_Timer WaitTrayDelay;
void DoAutoEmpty1()
{
    int &Task=iAutoEmpty1ReceiveTask;
    static bool bRetry=false;
#ifndef SOFT_SIMULTE
    int ret;
    static bool bEmptyHasDuplicateError=false;                                  //Steven 20120220 : Empty Tray重複Alarm Flag
#endif
    if(IniConfig.bP04ColorIsEmptyUnloader)
        return;

    switch(Task)
    {
        case 1:
            if(MOT[MMEmpty1].fHasTray         ||
               Sen[SenEmpty1HasTray].IsOn()   ||
               Sen[SenEmpty1CarHasTray].IsOn())
            {
                SW[SwACEmpty1CCW].On();
                WaitTrayDelay.SetMSAndOn(15000);
                bRetry=false;
                Task=100;
            }
            break;
        case 100:
            if(bHandlerPause)                                                   //Steven 20190123 : 紀錄Handler被暫停, 重置Timer
            {
                Task=1;
                break;
            }

            #ifdef SOFT_SIMULTE
                DoAutoEmpty1Delay.SetMSAndOn(1000);
                Task=200;
                break;
            #else
                if(LastSet.iRealDummy==DUMMY)
                {
                    DoAutoEmpty1Delay.SetMSAndOn(5000);
                    bEmptyHasDuplicateError=false;
                    Task=200;
                    break;
                }

                if(Sen[SenEmpty1CarHasTray].IsOn())
                {
                    DoAutoEmpty1Delay.SetMSAndOn(1000);
                    bEmptyHasDuplicateError=false;
                    Task=200;
                }

                if(WaitTrayDelay.Off())
                {
                    ret=ShowErrorMessage("JAM1001", K_SKIP|K_RETRY, MMEmpty1,bEmptyHasDuplicateError,__FUNC__);
                    bEmptyHasDuplicateError=true;
                    if(ret==K_SKIP)
                    {
                        SW[SwACEmpty1CCW].Off();
                        MOT[MMEmpty1].ClearTray(__FUNC__);
                        bEmptyHasDuplicateError=false;
                        Task=1;
                    }
                    else
                    {
                        WaitTrayDelay.SetMSAndOn(15000);
                    }
                }
                break;
            #endif
        case 200:
            if(DoAutoEmpty1Delay.Off())
            {
                #ifdef SOFT_SIMULTE
                    SW[SwACEmpty1CCW].Off();
                    MOT[MMEmpty1].ClearTray(__FUNC__);
                    Task=1;
                    break;
                #else
                    if(LastSet.iRealDummy==DUMMY)
                    {
                        SW[SwACEmpty1CCW].Off();
                        MOT[MMEmpty1].ClearTray(__FUNC__);
                        bEmptyHasDuplicateError=false;
                        Task=1;
                        break;
                    }

                    if(Sen[SenEmpty1CarHasTray].IsOff())
                    {
                        SW[SwACEmpty1CCW].Off();
                        MOT[MMEmpty1].ClearTray(__FUNC__);
                        bEmptyHasDuplicateError=false;
                        Task=1;
                    }
                    else
                    {
                        if(bRetry==false)
                        {
                            DoAutoEmpty1Delay.SetMSAndOn(3000);
                            SW[SwACEmpty1CCW].On();
                            bRetry=true;
                        }
                        {
                            ShowErrorMessage("JAM1001", K_RETRY, MMEmpty1,bEmptyHasDuplicateError,__FUNC__);
                            bEmptyHasDuplicateError=true;
                        }
                    }
                #endif
            }
            break;
    }
}
//------------------------------------------------------------------------------
