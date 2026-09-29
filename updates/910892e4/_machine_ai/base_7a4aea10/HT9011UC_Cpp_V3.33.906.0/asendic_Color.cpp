// =============================================================================
//  asendic_Color.cpp  --  Color-tray stack-feed state machine
//
//  Translated from: HT9011UC_Code_V3.33.906.0_20260618/asendic_Color.cpp (1553 ln)
//  Translation wave: W906-W7-L1 Wave 1 (asendic_* feed-SM family, Color track)
//  Translator: AI(W906-W7-L1-Color) 20260802
//
//  FAITHFUL TRANSLATION
//  --------------------
//  Names, 1-based numeric Task states, switch(Task) semantics and every case
//  fall-through are preserved VERBATIM, same idiom as the landed
//  asendic_Empty.cpp (W6.1 canary) and asendic_Auto2.cpp (W7-L1).  All 15 golden
//  functions are translated (golden :37, :42, :280, :285, :378, :387, :816, :821,
//  :1165, :1170, :1295, :1300, :1455, :1482, :1487 -- 7 Init* + 7 Do* + the
//  ForTERAPOWERCheckColorSensor predicate).  NOTHING IS GATED IN THIS FILE.
//
//  HOW IT WAS PRODUCED -- ADAPT-AND-DIFF, THEN LINE-BY-LINE RE-VERIFY.
//  golden asendic_Color.cpp is a ~67% structural clone of golden
//  asendic_Empty.cpp, which is already translated/reviewed/mutation-tested here.
//  Six of the seven Do* were adapted from that landed translation and then
//  re-verified line by line against the cp950-decoded golden Color source; the
//  divergences are NOT cosmetic and are listed in the DIVERGENCES block below so
//  a reviewer can check them individually instead of re-diffing 1553 lines.
//  DoColorTrayToRear and DoColorTrayReadCID have no Empty counterpart worth
//  adapting (the whole ATK-AMR / channel-2 RFID / Keyence tray-ID sub-flow is
//  Color-only) and were translated fresh.
//
//  DIVERGENCES FROM THE EMPTY SIBLING (each verified against golden Color):
//    * DoLoadNewColorTrayToCar case 1: the guard operands are in the OPPOSITE
//      order to Empty's -- golden Color :62 tests `LastSet.iRealDummy==DUMMY ||
//      Sen[SenColorSelectHasTray].IsOff()==false`, Empty :119 tests the sensor
//      first.  Kept as golden Color has it (short-circuit order is observable).
//    * DoLoadNewColorTrayToCar case 50: Color adds `&& LastSet.iRealDummy!=DUMMY`
//      to the bG11ASEReport gate (golden :132, "kevin 20220117 run dummy").
//    * DoLoadNewColorTrayToCar case 53 timeout arm: Color does NOT re-send
//      "@HTColor" via RespondASECom the way Empty does -- it only bumps iRead2D
//      and re-arms the timer (golden :178-182).
//    * DoLoadNewColorTrayToCar case 420: Color calls ReadWriteTrayID(false) a
//      SECOND time on the success path (golden :255); Empty does not.
//    * DoColorTrayToFront has NO hColorTrayToFrontdelay 1 s settle, NO iDelayCnt
//      10x re-check and NO SUPPORT_2_EMPTY_EMPTY early-exit; it instead re-arms
//      both deadlines whenever BOTH belt switches read stopped (golden :304-309)
//      and clears fSortCT->pnlCoverTrayD->Caption at case 300 (golden :363).
//    * DoColorTrayToRear is where the two files really part company: the ATK-AMR
//      RFID cover-tray-ID path (cases 450/455/460/465/470/600), the Keyence
//      tray-ID-2 path (cases 510/520/530/540) and the iReadCIDAction port lock
//      have no Empty counterpart at all.
//    * DoUnLoadNewColorToStack case 200: Color only does `if(CylinderUp(...))
//      Task=300;` (golden :1250-1251) -- Empty additionally drives TrayMoveOut +
//      the track floodgate there.  Not an omission; golden Color is that short.
//    * DoUnLoadNewColorToStack case 400: Color tests `DUMMY || (sensors)`
//      (golden :1261-1263); Empty tests `(sensors) || DUMMY`.
//    * DoAutoColor case 1 checks ONE track sensor (SenColorHasTray, golden :889);
//      Empty checks two, and Empty's case 1 also holds a WhichAutoNeedTray()
//      call that Color has no equivalent of.
//    * DoAutoColor case 65: Color's two re-detect blocks are NOT wrapped in
//      `#ifndef SOFT_SIMULTE` (Empty's are), and its stale-tray clear has the
//      extra `bTrayArmIsPickColor==false` conjunct (golden :973).
//    * DoAutoColorReceive has an extra case 50 (the bP04ColorIsEmptyUnloader
//      "wait for TrayX to clear, then release the out-arm" arm, golden :1320).
//
//  INCLUDES (heavy/UI/SECS swapped for substrate + scaffold; same substitution
//  table asendic_Empty.cpp and asendic_Auto2.cpp already use):
//    BCB6 original pulled: MachineDefine.h, uHGemHT9045.h (SECS), MyMotor.h,
//    main.h (TfMain), mymessbox.h, note.h, acatchtray.h, OCR.h, OCRInsp.h,
//    cMyDB.h, common.h, cTrayMapping.h, cSortCT.h, AGV.h.  Replaced by:
//      Motor/mymotor.h   (MOT[])                    -- W4 HAL
//      mycylin.h         (Cylinder[])               -- W6.0 HAL gap
//      mysensor.h        (Sen[])                    -- W4-IO HAL
//      myswitch.h        (SW[])                     -- W4-IO HAL
//      asendic.h         (CylinderUp/Middle/Lower + TrayMove* + TrayMoveStatus)
//      csystem.h         (ReadWriteTrayID)          -- frozen interface
//      acatchtray.h      (InitialTrayID2Task, golden acatchtray.h:56)
//      acatchtray_shims.h(fTrayMapping + channel-2 RFID triad/quartet, was
//                         cTrayMapping.h; MyMessageBox, was mymessbox.h;
//                         iReadCIDAction; BoolToStr, was SysUtils.hpp)
//      FormsFacade.h     (fMain->chkE84IDTray / fMain->StringGrid2, was main.h;
//                         fSortCT->pnlCoverTrayD, was cSortCT.h;
//                         fAGV->IsATK_AMR(), was AGV.h)
//      canary_support.h  (LastSet shim, __FUNC__, ShowErrorMessage/RecordProcess/
//                         ShowUnloaderTrayMessage/ShowMyMessage)
//      SECSGEM/SecsEventType.h + SECSGEM/SecsEventReport.h (SECS_EVENT +
//                         EventReport, was uHGemHT9045.h)
//      cprod.h/cpublic.h/cmydef.h (Prod/IniConfig/CosFunction/TrayForm/RunInfo/
//                         Ld_UldDelayTime/TrayID/TestIF_File/RespondASECom/enums)
//    Not needed offline: OCR.h / OCRInsp.h / cMyDB.h (no OCR/DB symbol in this
//    file's body -- RecordProcess comes from canary_support.h).
//
//  NO `#if 0` GATE IS WRITTEN IN THIS FILE, and that was checked rather than
//  assumed.  All three golden SECS calls compile un-gated (EventReport +
//  SECS_EVENT.NoCoverTray_2DID golden :83, .UnloaderTrayIDReadFail :627,
//  .UnloaderTrayIDReadOK :805 -- all three CEIDs exist in the real translated
//  SECSGEM/SecsEventType.h), and so does the MyMessageBox close-before-alarm at
//  golden :1395-1396.  Those are the exact two surfaces asendic_Empty.cpp had
//  gated until 20260801, when both gates were retired because the surfaces had
//  landed; the same reasoning applies here, so the gates were never written.
//
//  SHIM RETIREMENTS THIS FILE FORCES (integrator applies them atomically with the
//  CMake add; see the SHIM RETIREMENT DEBT REGISTER banner in acatchtray_shims.h,
//  whose three W7-L1-Wave-1-"Color" entries were re-verified against the decoded
//  golden in this pass and are all correct):
//    * acatchtray_shims.h `bool DoAutoColor();` + acatchtray_shims.cpp
//      `bool DoAutoColor() { return true; }` -- golden is VOID
//      (golden asendic_Color.h:20).  Duplicate symbol AND return-type conflict.
//      All 3 ported call sites (csystem.cpp x2, acatchtray.cpp x1) discard the
//      value, so void costs them nothing.
//    * acatchtray_shims.h `void InitAutoColorReceiveTask();` +
//      acatchtray_shims.cpp `void InitAutoColorReceiveTask() {}` -- signature-
//      identical duplicate.  KEEP nothing: acatchtray.cpp's 3 call sites bind to
//      this file's real body via acatchtray_shims.h -> (integrator adds
//      asendic_Color.h include) or via the surviving declaration.
//    * csystem_shims.h `void InitAutoColorTask();` + csystem_shims.cpp
//      `void InitAutoColorTask() {}` -- signature-identical duplicate.
//
//  SOFT_SIMULTE is NOT defined: the #ifndef SOFT_SIMULTE (real-machine) branches
//  are compiled, the #ifdef SOFT_SIMULTE branches are not -- i.e. the REAL logic
//  is translated, driven via the Sim HAL.  DEBUG_COLOR is likewise not defined
//  (MachineType.h:45 keeps it commented out, same as golden MachineType.h:40), so
//  the `#ifndef DEBUG_COLOR` pause guard at golden :434 IS compiled.
//
//  Big5: every Chinese comment decoded cleanly via cp950 and is preserved as
//  UTF-8.  NO U+FFFD is emitted.
// =============================================================================
#include "asendic_Color.h"
#include "csystem.h"
//---------------------------------------------------------------------------
#include "asendic.h"
#include "Motor/mymotor.h"
#include "mycylin.h"
#include "mysensor.h"
#include "myswitch.h"
#include "acatchtray.h"                 // InitialTrayID2Task (golden acatchtray.h:56)
#include "acatchtray_shims.h"           // fTrayMapping / MyMessageBox / iReadCIDAction / BoolToStr
#include "cprod.h"
#include "cmydef.h"
#include "cpublic.h"                    // RespondASECom
#include "FormsFacade.h"                // fMain / fSortCT / fAGV
#include "canary_support.h"             // LastSet, __FUNC__, ShowErrorMessage, etc.
#include "SECSGEM/SecsEventType.h"      // SECS_EVENT      (was uHGemHT9045.h)
#include "SECSGEM/SecsEventReport.h"    // EventReport()   (was uHGemHT9045.h)
// BCB6 also pulled: MachineDefine.h, MyMotor.h, main.h, mymessbox.h, note.h,
//                   OCR.h, OCRInsp.h, cMyDB.h, common.h, cTrayMapping.h,
//                   cSortCT.h, AGV.h  -- all swapped above.

int iAutoColorReceiveTask=1;
int iLoadNewColorTrayToCarTask=1;
TQPF_Timer ColorDelay;
TQPF_Timer ColorMidCylinTime;                                                   //JerryYang 20161128 add
TQPF_Timer ColorReadTrayID;                                                     //kevin 20210623 read tray id time out
//---------------------------------------------------------------------------
void InitLoadNewColorTrayToCarTask()
{
    iLoadNewColorTrayToCarTask=1;
}
//---------------------------------------------------------------------------
bool DoLoadNewColorTrayToCar()
{
//#ifndef SOFT_SIMULTE
    int ret;
//#endif
    static bool bColorHasDuplicateError=false;                                  //Steven 20120220 : Color Tray重複Alarm Flag
    int &Task=iLoadNewColorTrayToCarTask;
    AnsiString SData="";                                                        //kevin 20210623
    static int iRead2D=0;                                                       //kevin 20211116 Tray 2D read state

    switch(Task)
    {
        case 1:
            if(MOT[MMColorZ].fHasTray)
            {
                ColorDelay.SetSecAndOn(1);                                      //Steven 20130904 : 加入Time Out 避免Hang Up
                Task=400;
                break;
            }

            if(LastSet.iRealDummy==DUMMY ||
               Sen[SenColorSelectHasTray].IsOff()==false)
            {
                if(Sen[SenColorCCWDete].IsOn()   ||
                   Sen[SenColorCarHasTray].IsOn())
                {
                    MOT[MMColor_Car].SetTray(NULL_IC, __FUNC__);
                    MOT[MMColorZ].ClearTray(__FUNC__);
                    Task=1;
                    return true;
                }

                if(CosFunction.bUseEmptyColorTrayPreAlarm==true &&              //Ifor 20170322 (wei) add 開啟 Empty/Color Tray Pre Alarm 功能 不在此處Alarm
                   IniConfig.bP32EmptyColorTrayPreAlarm==true)
                {
                }
                else
                {
                    if(Sen[SnColorIsFull].IsOn()==true)                         //jou 2015-08-25 Tray limit sensor at Color Tray
                    {
                        if(IniConfig.bEnable_SECS_GEM==true)                    //JerryYang 20240318 : add
                            EventReport(SECS_EVENT.NoCoverTray_2DID);
                        ShowErrorMessage("MES1420", K_RETRY, MMColor_Car);
                        return false;
                    }
                }
                Task=50;
            }
            else
            {
                if(AUTO_EMPTY_COLOR!=0)                                         //jou 2011-12-21 有發生Tray feed完,汽缸還在上面,所以做完下降
                {
                    if(MOT[MMColor_Car].fHasTray==false)
                    {
                        if(Cylinder[C_Color_Middle].OffStatus()==false &&       //汽缸在上面
                           Cylinder[C_Color_Middle].Enable==true)               //Ztex 2024.08.13 Add Check Select Has Tray
                        {
                            if(Sen[SenColorCarHasTray].IsOff() && Sen[SenColorCCWDete].IsOff())
                            {
                                Cylinder[C_Color_Up].Off();
                                Cylinder[C_Color_Middle].Off();
                                ColorMidCylinTime.SetSecAndOn(5.0);
                                Task=20;                                        //JerryYang 20161128 新增防護,避免color的Middle氣缸sensor一直亮著導致hang up
                                break;
                            }
                        }
                    }
                }
                ShowErrorMessage("MES1421", K_RETRY, MMColor_Car);
            }
            break;
        case 20:                                                                //JerryYang 20161128 新增防護,避免color的Middle氣缸sensor一直亮著導致hang up
            Cylinder[C_Color_Up].Off();
            Cylinder[C_Color_Middle].Off();
            if(Cylinder[C_Color_Middle].OffStatus()==false &&                   //汽缸在上面
               Cylinder[C_Color_Middle].Enable==true)                           //Ztex 2024.07.27 Add Check Has Use Cylinder
            {
                if(Sen[SenColorCarHasTray].IsOff() && Sen[SenColorCCWDete].IsOff())
                {
                    if(ColorMidCylinTime.Off())
                    {
                        ShowErrorMessage("JAM1406", K_RETRY, MMColor_Car);      //Color tray separation cylinder (lower) error
                        Task=1;
                    }
                    break;
                }
            }
            Task=1;
            break;
        case 50:
            if(IniConfig.bG11ASEReport && LastSet.iRealDummy!=DUMMY)            //kevin 20220117 run dummy
                Task=52;
            else
                Task=60;
            break;
        case 52:
            bReadColorTrayID=false;
            SData="@e02026";
            RespondASECom(SData);                                               //kevin 20150818 20150610 回應 ase Reset finish
            ColorReadTrayID.SetSecAndOn(7);                                     //kevin 20210623 read tray id time out
            RecordProcess("<AutoMove>Empty2 Query Tray ID.");                   //kevin 20210623 add 讀取 tray id
            iRead2D=0;                                                          //kevin 20211116 Tray 2D read state
            Task=53;
            break;
        case 53:
            if(bReadColorTrayID)                                                                                        //kevin 20210623 TRAY ID Read
            {
                if(TrayID[2][0]=="")
                {
                    iRead2D=0;
                    Task=52;
                    break;
                }
                SData.sprintf("<AutoMove>Empty2 Get Tray ID,%s",TrayID[2][0]);                                          //kevin 20210623  [6]: load empty  [2]: 0:read  1: send pick pos
                ReadWriteTrayID(false);                                                                                 //kevin 20220705 record color tray id
                RecordProcess(SData);                                                                                   //kevin 20210623 add 讀取 tray id
                Task=60;
            }
            else if(ColorReadTrayID.Off())                                                                              //kevin 20210623 read tray id time out
            {
                if(iRead2D>=1)                                                                                          //kevin 20211116 send 2D 不經EKeep
                {
                    ret=ShowErrorMessage("WAR16335", K_RETRY, MMSystem);                                                //kevin 20220917 change WAR16115 =>  WAR16335    //kevin 20220705 only Retry
                    if(ret==K_RETRY)
                    {
                        iRead2D=0;
                        Task=52;
                    }
                    else
                    {
                        TrayID[1][0]="NoTrayID";
                        SData.sprintf("<AutoMove>Empty2 Get Tray ID,NoTrayID");                                         //kevin 20210623  [6]: load empty  [2]: 0:read  1: send pick pos
                        RecordProcess(SData);                                                                           //kevin 20210623 add 讀取 tray id
                        Task=60;
                    }
                }
                else
                {
                    iRead2D++;
                    ColorReadTrayID.SetSecAndOn(5);                                                                     //kevin 20210623 read tray id time out
                }
            }
            break;
        case 60:
            if(CylinderUp(C_Color_Up))
            {
                MOT[MMColorZ].ClearTray(__FUNC__);
                Task=100;
            }
            break;
        case 100:
            if(LastSet.iRealDummy==DUMMY)                                       //Steven 20100618
                Task=200;
            else if(Cylinder[C_ColorLoaderZ_Select].Push())
                Task=200;
            break;
        case 200:
            if(CylinderMiddle(C_Color_Up))
            {
                if(bColorPause)
                {
                    bColorPause=false;
                    ShowMyMessage("Color split wait", "Color 分離暫停", "DoLoadNewColorTrayToCar 200");
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
            else if(Cylinder[C_ColorLoaderZ_Select].Pop())
            {
                Task=400;
            }

            ColorDelay.SetSecAndOn(1);                                          //Steven 20130904 Start: 加入Time Out 避免Hang Up
            break;
        case 400:
            if(ColorDelay.Off())
            {
                Task=410;
            }
            break;
        case 410:
            if(CylinderLower(C_Color_Up))
            {
                ColorDelay.SetSecAndOn(5);                                      //Steven 20140109 : 1000 --> 5000
                Task=420;
            }
            break;
        case 420:
            if(bHandlerPause)                                                                                           //Steven 20190123 : 紀錄Handler被暫停, 重置Timer
            {
                ColorDelay.SetSecAndOn(5);
            }

#ifndef SOFT_SIMULTE
            if((Sen[SenColorCCWDete].IsOn()     ||
                Sen[SenColorCarHasTray].IsOn()) ||
               LastSet.iRealDummy==DUMMY)
#endif
            {
                MOT[MMColor_Car].SetTray(NULL_IC, __FUNC__);
                MOT[MMColorZ].ClearTray(__FUNC__);
                Task=1;
                bColorHasDuplicateError=false;
                iTrayTotal[2]++;                                                                                        //kevin 20210623  0:loader 1:Empty  2:olor  Tray 入軌道數量
                TrayID[2][1]=TrayID[2][0];                                                                              //kevin 20210623  [6]: load empty  [2]: 0:read  1: send pick pos
                TrayID[2][0]="";
                ReadWriteTrayID(false);                                                                                 //kevin 20220705 record color tray id move color pick
                return true;
            }
#ifndef SOFT_SIMULTE
            else if(ColorDelay.Off())
            {
                ret=ShowErrorMessage("JAM1413", K_RETRY|K_SKIP, MMColor_Car, bColorHasDuplicateError, __FUNC__);        //Steven 20150108 : JAM1401 --> JAM1413
                bColorHasDuplicateError=true;
                if(ret==K_SKIP)
                {
                    MOT[MMColorZ].ClearTray(__FUNC__);
                    bColorHasDuplicateError=false;
                    Task=1;
                }
            }
            break;
#endif
    }
    return false;
}
//------------------------------------------------------------------------------
int iColorTrayToFrontTask=1;
TQPF_Timer hColorTrayToFront;
TQPF_Timer hColorTrayToFrontForDummy;
TQPF_Timer UnloadDelay;
void InitColorTrayToFrontTask()
{
    iColorTrayToFrontTask=1;
}
//------------------------------------------------------------------------------
bool DoColorTrayToFront()
{
    int &Task=iColorTrayToFrontTask;
    int ret;
    static bool bColorHasDuplicateError=false;                                  //Steven 20120220 : Color Tray重複Alarm Flag
    switch(Task)
    {
        case 1:
            hColorTrayToFront.SetSecAndOn(20);
            hColorTrayToFrontForDummy.SetSecAndOn(5);
            TrayMoveIn(false, 2);                                               //kevin 20220711 停止往前送
            Task=100;
            // NOTE: faithful fall-through to case 100 (no break in golden :296)
        case 100:
            if(bHandlerPause)                                                                                           //Steven 20190123 : 紀錄Handler被暫停, 重置Timer
            {
                Task=1;
                break;
            }

            if(SW[SwACColorCCW].Status()==false    &&                                                                   //pause happened
               SW[SwACColorCW].Status()==false)
            {
                hColorTrayToFront.SetSecAndOn(20);
                hColorTrayToFrontForDummy.SetSecAndOn(5);
            }

            Cylinder[C_Color_Fix].Off();
            TrayMoveOut(true, 2);
            if(TRAY_ARM_MODE==eUnderCoveyor)
                TrayCylinMoveOut(2);

            if(Sen[SenColorCarHasTray].IsOff()==false)
            {
                UnloadDelay.SetSecAndOn(Ld_UldDelayTime.ULD_LiftDownDelay);
                Task=200;
                break;
            }

            if(hColorTrayToFrontForDummy.Off())
            {
                if(LastSet.iRealDummy==DUMMY)
                {
                    TrayMoveOut(false, 2);
                    bColorHasDuplicateError=false;                                                                      //Steven 20120220 : Color Tray重複Alarm Flag
                    Task=300;                                                                                           //Sam 20221027 : 整合 P24 功能
                    break;
                }
            }

            if(hColorTrayToFront.Off())
            {
                ret=ShowErrorMessage("JAM1412", K_SKIP|K_RETRY, MMColor, bColorHasDuplicateError, __FUNC__);            //Steven 20120925 : JAM1409 -> JAM1401
                if(ret==K_RETRY)
                {
                    bColorHasDuplicateError=true;                                                                       //Steven 20120220 : Color Tray重複Alarm Flag
                    Task=1;
                }
                else
                {
                    bColorHasDuplicateError=false;                                                                      //Steven 20120220 : Color Tray重複Alarm Flag
                    MOT[MMColor].ClearTray(__FUNC__);
                    Task=1;
                    return true;
                }
            }
            break;
        case 200:
            if(UnloadDelay.Off())
                Task=300;
            break;
        case 300:
            MOT[MMColor_Car].SetTray(NULL_IC, __FUNC__);
            if(MOT[MMColor].Tray.iNeedManualRemoved>=0)                         //Sam 20230317 : 修正 P24 功能
            {
                MOT[MMColor_Car].Tray.iNeedManualRemoved=MOT[MMColor].Tray.iNeedManualRemoved;
                MOT[MMColor].Tray.iNeedManualRemoved=-1;
            }

            fSortCT->pnlCoverTrayD->Caption="";
            MOT[MMColor].ClearTray(__FUNC__);
            if(TRAY_ARM_MODE==eUnderCoveyor)
                TrayCylinMoveIn(2);
            Task=1;
            return true;
    }
    return false;
}
//------------------------------------------------------------------------------
int iColorTrayToRearTask=1;
int iColorIdTrayCt=0;
TQPF_Timer hColorTrayToRear;
TQPF_Timer hColorTrayToRearForDummy;
TQPF_Timer hColorUnderCoveyorDelay;
void InitColorTrayToRearTask()
{
    if(iReadCIDAction==ePortColor)
        iReadCIDAction=ePortTotal;
    iColorTrayToRearTask=1;
    iColorIdTrayCt=0;
}
//------------------------------------------------------------------------------
//---- 收空TRAY----
bool DoColorTrayToRear()
{
    static bool bColorHasDuplicateError=false;                                  //Steven 20120220 : Color Tray重複Alarm Flag
    AnsiString str="";
    int &Task=iColorTrayToRearTask;
    int ret;
    bool bColorFlag=false;
    bool bSOFT_SIMULTE=false;                                                   //Ztex 2023.07.28 Add bSOFT_SIMULTE
    #ifdef SOFT_SIMULTE
        bSOFT_SIMULTE=true;
    #endif

    switch(Task)
    {
        case 1:
            if(fAGV->IsATK_AMR() &&
               fTrayMapping->ldRFID->bCommConnect==false)
            {
                fTrayMapping->btnLdRFIDConn->Click();
            }

            Task=100;
            // NOTE: faithful fall-through to case 100 (no break in golden :408)
        case 100:
            if(TRAY_ARM_MODE==eUnderCoveyor)
                TrayCylinMoveOut(2);                                            //需要往front 到等待點

            if(Cylinder[C_Color_Fix].Pop())
            {
                hColorTrayToRearForDummy.SetSecAndOn(5);                        //kevin 201709015 move case 100
                hColorTrayToRear.SetSecAndOn(20);
                Task=200;
            }
            break;
        case 120:
            if(hColorTrayToRearForDummy.Off())
            {
                fTrayMapping->ldRFID->sBlockID2="";
                fTrayMapping->ldRFID->sUID2="";
                fTrayMapping->ldRFID->bReadUID2=false;
                fTrayMapping->btnReadUID2->Click();
                InitColorTrayReadCIDTask();
                hColorTrayToRearForDummy.SetSecAndOn(5);                        //kevin 201709015 move case 100
                hColorTrayToRear.SetSecAndOn(20);
                Task=200;
            }
            break;
        case 200:
            #ifndef DEBUG_COLOR
            if(bHandlerPause &&                                                                                                                                 //Steven 20190123 : 紀錄Handler被暫停, 重置Timer
               fMain->chkE84IDTray->Checked==false)
            {
                if(iReadCIDAction==ePortColor)
                    iReadCIDAction=ePortTotal;
                Task=1;
                break;
            }
            #endif

            if(MOT[MMColor].fHasTray &&
               MOT[MMColor_Car].fHasTray)                                                                                                                       //Steven 20130917 : Double Tray問題,測試 Color
            {
                Task=1;
                if(iReadCIDAction==ePortColor)
                    iReadCIDAction=ePortTotal;

                return true;
            }

            TrayMoveIn(true, 2);
            if(DOUBLE_BELT_MODE==1)                                                                                                                             //Ztex 2023.12.13 Add Double Belt Mode
            {
                if(bSOFT_SIMULTE==true ||
                   Sen[SenColorHasTray].IsOn()==true)
                {
                    hColorUnderCoveyorDelay.SetSecAndOn(0.2);
                    Task=430;
                    break;
                }
            }
            else if(TRAY_ARM_MODE==eUnderCoveyor)
            {
                if(Sen[SnColorSeparateHasTray].IsOff()==false)
                {
                    hColorUnderCoveyorDelay.SetSecAndOn(0.2);
                    Task=430;
                    break;
                }
            }
            else
            {
                if(Sen[SenColorHasTray].IsOff()==false &&
                   Sen[SenColorCWDete].IsOff()==false)                                                                                                          //Sam 20220919 : 增加保護
                {
                    Task=450;
                    break;
                }
            }

            if(hColorTrayToRearForDummy.Off())
            {
                if(LastSet.iRealDummy==DUMMY)
                {
                    Task=450;
                    break;
                }
            }

            if(hColorTrayToRear.Off())
            {
                if(iReadCIDAction==ePortColor)
                    iReadCIDAction=ePortTotal;

                if(MOT[MMColor_Car].fHasTray==false)                                                                                                            //Sam 20190804 : Fix Empty_Car 補盤到 Empty 動作未完成時，剛好 TrayArm 要放 Tray，觸發 DoAutoEmptyReceive 機制將 Empty_Car 收起來，但 TrayArm 原本要放的 Tray 跑去補 Auto Tray。
                {
                    Task=1;
                    TrayMoveIn(false, 2);
                    if(Cylinder[C_ColorTrackFloodgate].Enable)
                        Cylinder[C_ColorTrackFloodgate].Off();
                    MOT[MMColor_Car].ClearTray(__FUNC__);
                    bColorHasDuplicateError=false;
                    return true;
                }

                ret=ShowErrorMessage("JAM1401", K_SKIP|K_RETRY, MMColor_Car, bColorHasDuplicateError, "DoColorTrayToRear_200");                                 //Steven 20120925 : JAM1409 -> JAM1401
                if(ret==K_RETRY)
                {
                    bColorHasDuplicateError=true;                                                                                                               //Steven 20120220 : Color Tray重複Alarm Flag
                    Task=1;
                }
                else
                {
                    bColorHasDuplicateError=false;                                                                                                              //Steven 20120220 : Color Tray重複Alarm Flag
                    MOT[MMColor_Car].ClearTray(__FUNC__);
                    Task=1;
                    return true;
                }
            }
            break;
        case 300:
            if(Cylinder[C_Color_Fix].Push())
                Task=400;
            break;
        case 400:
            if(Sen[SenColorFixCyPush].IsOn())
            {
                if(LastSet.iRealDummy==DUMMY)
                {
                    bColorHasDuplicateError=false;                                                                                                              //Steven 20120220 : Color Tray重複Alarm Flag
                    Task=450;
                }
                else
                {
                    Cylinder[C_Color_Fix].Off();
                    ret=ShowErrorMessage("JAM1403", K_RETRY, MMColor, bColorHasDuplicateError, "DoColorTrayToRear_400");                                        //Steven 20120925 : JAM1409 -> JAM1403
                    bColorHasDuplicateError=true;                                                                                                               //Steven 20120220 : Color Tray重複Alarm Flag
                    Task=300;
                    break;
                }
            }
            else
            {
                bColorHasDuplicateError=false;                                                                                                                  //Steven 20120220 : Color Tray重複Alarm Flag
                Task=500;
            }
            break;
        case 420:
            if(Sen[SenColorHasTray].IsOff()==false &&
               Sen[SenColorCWDete].IsOff()==false)
            {
                Task=450;
            }
            break;
        case 430:
            if(hColorUnderCoveyorDelay.Off())
            {
                Cylinder[C_Color_Fix].On();
                Task=450;
            }
            break;
        case 450:
            if(fAGV->IsATK_AMR())                                               //Steven 20251210 : ATK AMR
            {
                if(LastSet.iRealDummy==DUMMY)
                {
                    fTrayMapping->ldRFID->sBlockID2=AnsiString().sprintf("%02d%02d%02d%03d", SystemHour, SystemMin, SystemSec, SystemMSec);
                    Task=480;
                }
                else
                {
                    sCoverTrayID="";
                    if(iReadCIDAction!=ePortTotal)
                        break;

                    iReadCIDAction=ePortColor;
                    fTrayMapping->btnLdRFIDClear2->Click();
                    hColorTrayToRearForDummy.SetSecAndOn(5);
                    Task=455;
                }
            }
            else
            {
                hColorTrayToRear.SetSecAndOn(Ld_UldDelayTime.LD_TrayArrivalDely);
                Task=480;
            }
            break;
        case 455:
            if(fTrayMapping->ldRFID->bClearCmd2==true)
            {
                fTrayMapping->ldRFID->sBlockID2="";
                fTrayMapping->ldRFID->sUID2="";
                fTrayMapping->ldRFID->bReadUID2=false;
                fTrayMapping->btnReadUID2->Click();
                InitColorTrayReadCIDTask();
                hColorTrayToRearForDummy.SetSecAndOn(5);                        //kevin 201709015 move case 100
                hColorTrayToRear.SetSecAndOn(1);
                Task=460;
            }
            else if(hColorTrayToRearForDummy.Off())
            {
                if(iReadCIDAction==ePortColor)
                    iReadCIDAction=ePortTotal;
                Task=450;
            }
            break;
        case 460:
            if(hColorTrayToRear.Off())
            {
                ret=DoColorTrayReadCID();

                if(ret==1)
                {
                    hColorTrayToRear.SetSecAndOn(Ld_UldDelayTime.LD_TrayArrivalDely);
                    Task=480;
                }
                else if(ret==-1)
                {
                    iThisPortNo=ePortColor;
                    iThisPortStatus=iPortStatus[ePortColor];

                    if(IniConfig.bEnable_SECS_GEM==true)
                        EventReport(SECS_EVENT.UnloaderTrayIDReadFail);
                    if(iReadCIDAction==ePortColor)
                        iReadCIDAction=ePortTotal;
                    Cylinder[C_Color_Fix].Off();
                    iColorIdTrayCt++;
                    if(iColorIdTrayCt>=3)
                        Task=470;
                    else
                        Task=465;
                }
            }
            break;
        case 465:
            TrayMoveOut(true, 2);
            if(Sen[SenColorCCWDete].IsOn()  &&
               Sen[SenColorCarHasTray].IsOn())
            {
                TrayMoveOut(false, 2);
                Task=1;
            }
            break;
        case 470:
            fMain->chkE84IDTray->Checked=false;
            ShowMyMessage("Read ID tray fail!!", "");
            Task=1;
            break;
        case 480:
            if(hColorTrayToRear.Off())
            {
                TrayCylinMoveIn(2);
                TrayMoveIn(false, 2);

                if(Cylinder[C_ColorTrackFloodgate].Enable)
                    Cylinder[C_ColorTrackFloodgate].Off();

                if(TRAY_ARM_MODE==eUnderCoveyor)
                {
                    hColorUnderCoveyorDelay.SetSecAndOn(3);
                    Task=490;
                }
                else
                {
                    Task=500;
                }
            }
            break;
        case 490:
            if(LastSet.iRealDummy==DUMMY ||
               (Sen[SenColorHasTray].IsOff()==false &&
                Sen[SnColorSeparateHasTray].IsOff()==false))
            {
                Task=500;
            }
            else if(hColorUnderCoveyorDelay.Off())
            {
                ShowErrorMessage("WAR1451", K_RETRY, MMColor, false, "DoColorTrayToRear");
                hColorUnderCoveyorDelay.SetSecAndOn(3);
                break;
            }
            break;
        case 500:
            if(USE_COLOR_TRAY_SENSOR==1 &&
               TrayForm.bColorTray &&
               LastSet.iRealDummy!=DUMMY &&
               Sen[SnColorColorTrayDetect].Enable)                                                                      //20140903 wei colcr Tray
            {
                bColorFlag=Sen[SnColorColorTrayDetect].IsOff();                                                         //Steven 20150427 : For Color Sensor, 滅掉是紅色
                if(bColorFlag==true)                                                                                    //Steven 20150609 : 客戶要把Color流道的Tray搬到Auto 1,所以必須是黑色的
                {
                    ShowErrorMessage("WAR1451", K_RETRY, MMColor, false, "DoColorTrayToRear");                          //WAR1451
                    break;
                }
            }

            if(fAGV->IsATK_AMR())                                                                                       //Steven 20251210 : ATK AMR
            {
                Task=600;
            }
            else
            {
                Task=510;
                bEmptyReadIDOK=false;                                                                                   //JerryYang 20250113 : Color上方TrayID
            }
            break;
        case 510:
            if(Cylinder[C_Color_Fix].Push())
            {
                if(iReadCIDAction==ePortColor)
                    iReadCIDAction=ePortTotal;
                if(USE_TRAY_MAPPING==1 &&                                       //JerryYang 20250113 : Color上方TrayID //USE_KEYENCE_EMPTY 0 ccd ;1 keyence auto 2; 2 keyence Empty
                   TestIF_File.bEnableTrayID2==true &&                          //wei 20180808 MR Tray ID 2
                   bEmptyReadIDOK==false &&
                   USE_KEYENCE_EMPTY==3)
                {
                    #ifndef SOFT_SIMULTE
                    if(LastSet.iRealDummy!=DUMMY)                               //wei 20200604 空跑不讀取Empty Tray ID
                    #endif
                    {
                        InitialTrayID2Task();
                        Task=520;
                        break;
                    }
                }
                MOT[MMColor].SetTray(NULL_IC, __FUNC__);
                MOT[MMColor_Car].ClearTray(__FUNC__);                           //Steven 20111109 : 已入Tray卻未清掉資料

                Task=1;
                return true;
            }
            break;
        case 520:
            if(fTrayMapping->DoTrayIDKeyence2(2))                               //JerryYang 20250113 : Color上方TrayID
            {
                bEmptyReadIDOK=true;
                if(asTrayIDData2.AnsiPos("ERROR")!=0)
                {
                    Task=530;
                    Cylinder[C_Color_Fix].Off();
                    ShowMyMessage("Color TrayID Read Error, please remove the tray!");
                }
                else if(fTrayMapping->DoTrayIDCheck(asTrayIDData2)==false)
                {
                    Task=530;
                    Cylinder[C_Color_Fix].Off();
                    ShowMyMessage("Color Tray ID Duplicate error, please remove the tray!");
                    break;
                }
                else
                {
                    Task=540;
                }
            }
            else if(TestIF_File.bEnableTrayID2==false)
            {
                bEmptyReadIDOK=true;
                MOT[MMColor].SetTray(NULL_IC, __FUNC__);
                MOT[MMColor_Car].ClearTray(__FUNC__);                           //Steven 20111109 : 已入Tray卻未清掉資料
                Task=1;
                return true;
            }
            break;
        case 530:
            if(Sen[SenColorCarHasTray].IsOff()==false || Sen[SenColorCCWDete].IsOff()==false ||                         //JerryYang 20250113 : Color上方TrayID
               Sen[SenColorHasTray].IsOff()==false || Sen[SenColorCWDete].IsOff()==false)
            {
                if(asTrayIDData2.AnsiPos("ERROR")!=0)
                {
                    ShowMyMessage("Color TrayID Read Error, please remove the tray!");
                }
                else
                {
                    ShowMyMessage("Color Tray ID Duplicate error, please remove the tray!");
                }
            }
            else
            {
                MOT[MMColor_Car].ClearTray(__FUNC__);
                Task=1;
                return true;
            }
            break;
        case 540:
            MOT[MMColor].SetTray(NULL_IC, __FUNC__);
            MOT[MMColor].Tray.cTrayID=asTrayIDData2;
            MOT[MMColor_Car].ClearTray(__FUNC__);                               //Steven 20111109 : 已入Tray卻未清掉資料
            str.sprintf("DoColorTrayToRear finish - Color tray ID:%s", asTrayIDData2);
            RecordProcess(str);
            Task=1;
            return true;

        case 600:
            bColorFlag=fTrayMapping->DoCoverTrayID(iKeyenceCoverTrayID_Color);
            if(bColorFlag)
            {
                iThisPortNo=ePortColor;
                asBundleTrayID[ePortColor]=asTrayIDDataCorverLoader;

                if(IniConfig.bEnable_SECS_GEM==true)
                    EventReport(SECS_EVENT.UnloaderTrayIDReadOK);
                Task=510;
            }
            break;
    }
    return false;
}
//------------------------------------------------------------------------------
int iAutoColorTask=1;
TQPF_Timer DoAutoColorDelay;
TQPF_Timer CheckColorTrayDelay;
void InitAutoColorTask()
{
    iAutoColorTask=1;
}
//------------------------------------------------------------------------------
void DoAutoColor()
{
    int &Task=iAutoColorTask;
    static bool bColorHasDuplicateError=false;                                  //Steven 20120220 : Color Tray重複Alarm Flag
    bool bColorFlag=false;
    static bool fOldColorCanSupplyNewTray=false;

    if(fOldColorCanSupplyNewTray!=fColorCanSupplyNewTray)                       //Sam 20200630 : Add Log
    {
        fOldColorCanSupplyNewTray=fColorCanSupplyNewTray;
        fMain->StringGrid2->Cells[3][38]=BoolToStr(fOldColorCanSupplyNewTray);
    }

    if(iReceiveColorTray!=0)
    {
        if(CUSTOMER_CODE==CC_SCC && bIsCatchingFromBuffer==true)                //RogerYang 20260225 : JSCC防止夾tray的時候Color/Empty誤退，驗證中
        {
            InitAutoColorReceiveTask();
            return;
        }
        DoAutoColorReceive();
        return;
    }

    if(fColorCanSupplyNewTray==false)
        return;

    if(IniConfig.bP04ColorIsEmptyUnloader)
        return;

    if(CosFunction.bSpecialP24)                                                 //Sam 20230320 : 整合 P24 北興版為獨立功能碼 //Alick 20160907 Modify for SIGRUD PeiXing TrayAssignment 設定成different時造成與P24.25功能衝突
    {
        if(IniConfig.bP24SkipEventNeedRemoveEmptyAndColorTray==true &&
           IniConfig.bP25EmptyColorNoSuppleAutoNoLoadEmpty)
        {
            return;
        }
        else if(IniConfig.bP25EmptyColorNoSuppleAutoNoLoadEmpty &&
                IniConfig.bP24SkipEventNeedRemoveEmptyAndColorTray==false &&
              ((TrayForm.LodareType==0 &&
                TrayForm.LoaderToEmptyColor[iRunStartMode]==0) ||
               (TrayForm.LodareType==1 &&
                TrayForm.AutoFromEmptyColor[iRunStartMode][eAuto1]==0 &&
                TrayForm.AutoFromEmptyColor[iRunStartMode][eAuto2]==0 &&
                TrayForm.AutoFromEmptyColor[iRunStartMode][eAuto3]==0)))
        {
            return;
        }
    }
    else
    {
        if(TrayForm.bEnableAMR) //Eastsun 20260515 F019 AMR skip color supple
        {
        }
        else if(IniConfig.bP25EmptyColorNoSuppleAutoNoLoadEmpty &&
           ((TrayForm.LodareType==0 && TrayForm.LoaderToEmptyColor[iRunStartMode]==0) ||
            (TrayForm.LodareType==1 && TrayForm.AutoFromEmptyColor[iRunStartMode][eAuto1]==0 &&
                                       TrayForm.AutoFromEmptyColor[iRunStartMode][eAuto2]==0 &&
                                       TrayForm.AutoFromEmptyColor[iRunStartMode][eAuto3]==0)))                         //kevin 20160130 load to color no supple Auto1-3
        {
            return;
        }
    }

    switch(Task)
    {
        case 1:
            if(MOT[MMColor].fHasTray ||
               Sen[SenColorHasTray].IsOn())
            {
                Task=20;
                break;
            }
            else
            {
                MOT[MMColor].ClearTray(__FUNC__);
            }

            if(MOT[MMColor].fHasTray==false)
                Task=100;
            break;
        case 20:
            if(USE_COLOR_TRAY_SENSOR==1 &&
               TrayForm.bColorTray &&
               LastSet.iRealDummy!=DUMMY &&
               Sen[SnColorColorTrayDetect].Enable)                                                                      //20140903 wei colcr Tray
            {
                bColorFlag=Sen[SnColorColorTrayDetect].IsOff();                                                         //Steven 20150427 : For Color Sensor, 滅掉是紅色
                if(bColorFlag==true)                                                                                    //Steven 20150609 : 客戶要把Color流道的Tray搬到Auto 1,所以必須是黑色的
                {
                    ShowErrorMessage("WAR1451", K_RETRY, MMColor, false, "DoAutoColor");                                //WAR1451
                    break;
                }
            }
            Cylinder[C_Color_Fix].Off();
            Task=30;
            break;
        case 30:
            if(Cylinder[C_Color_Fix].Pop())
                Task=40;
            break;
        case 40:
            if(Sen[SenColorFixCyPush].IsOn())
            {
                if(LastSet.iRealDummy==DUMMY)
                {
                    bColorHasDuplicateError=false;                                                                      //Steven 20120220 : Color Tray重複Alarm Flag
                    Task=50;
                }
                else
                {
                    Cylinder[C_Color_Fix].Off();
                    ShowErrorMessage("JAM1403", K_RETRY, MMColor, bColorHasDuplicateError, __FUNC__);                   //Steven 20120925 : JAM1409 -> JAM1403
                    bColorHasDuplicateError=true;                                                                       //Steven 20120220 : Color Tray重複Alarm Flag
                    Task=1;
                }
            }
            else
            {
                bColorHasDuplicateError=false;                                                                          //Steven 20120220 : Color Tray重複Alarm Flag
                Task=50;
            }
            break;
        case 50:
            if(Cylinder[C_Color_Fix].Pop())
            {
                #ifdef SOFT_SIMULTE
                MOT[MMColor_Car].ClearTray(__FUNC__);
                MOT[MMColor].SetTray(NULL_IC, __FUNC__);
                #endif
                Task=60;
            }
            break;
        case 60:
            if(MOT[MMColor].fHasTray==false)
            {
                DoAutoColorDelay.SetSecAndOn(1);                                //Steven 20130909 : 比照Empty辦理
                Task=70;
            }
            else
            {
                CheckColorTrayDelay.SetSecAndOn(0.1);
                Task=65;
            }
            break;
        case 65:
            if(CheckColorTrayDelay.Off())
            {
                if(MOT[MMColor].fHasTray==true &&
                   LastSet.iRealDummy>=HAS_TRAY &&
                   Sen[SenColorHasTray].IsOff() &&
                   Sen[SenColorCWDete].IsOff() &&                               //JerryYang 20250113 : Color上方TrayID
                   bTrayArmIsPickColor==false)
                {
                    MOT[MMColor].ClearTray(__FUNC__);
                }
                Task=60;

                if(MOT[MMColor].fHasTray==false        &&
                   LastSet.iRealDummy>=HAS_TRAY        &&
                   Sen[SenColorHasTray].IsOff()==false &&
                   Sen[SenColorCWDete].IsOff()==false)                          //Steven 20130930
                {
                    MOT[MMColor].SetTray(NULL_IC, __FUNC__);
                }

                if(MOT[MMColor_Car].fHasTray==false       &&
                   LastSet.iRealDummy>=HAS_TRAY           &&
                   Sen[SenColorCarHasTray].IsOff()==false &&
                   Sen[SenColorCCWDete].IsOff()==false)                         //Steven 20130930
                {
                    MOT[MMColor_Car].SetTray(NULL_IC, __FUNC__);
                }

                if(MOT[MMTrayY].fHasTray &&
                   MOT[MMTrayY].HasIC()==false &&
                   MOT[MMColor].fHasTray &&
                   MOT[MMColor_Car].fHasTray)
                {
                    iReceiveColorTray=2;
                    iAutoColorReceiveTask=200;
                }

                if(MOT[MMColor].fHasTray &&                                     //Steven 20130917 : Double Tray問題,測試 Color
                   MOT[MMColor_Car].fHasTray)
                {
                    iReceiveColorTray=2;
                    iAutoColorReceiveTask=200;
                }
            }
            break;
        case 70:                                                                //Steven 20130909 : 比照Empty辦理
            if(DoAutoColorDelay.Off())
            {
                #ifdef SOFT_SIMULTE
                if(MOT[MMColor].fHasTray==false)
                {
                    if(USE_TRAY_MAPPING==1 &&                                   //JerryYang 20250113 : Color上方TrayID        //USE_KEYENCE_EMPTY 0 ccd ;1 keyence auto 2; 2 keyence Empty
                       TestIF_File.bEnableTrayID2==true &&                      //wei 20180808 MR Tray ID 2
                       USE_KEYENCE_EMPTY==3)
                    {
                        InitialTrayID2Task();
                        Task=80;
                        break;
                    }
                    else
                    {
                        MOT[MMColor_Car].ClearTray(__FUNC__);
                        MOT[MMColor].SetTray(NULL_IC, __FUNC__);                //Steven 20130909
                    }
                }
                Task=100;
                #else
                if((LastSet.iRealDummy==DUMMY &&
                    fAGV->IsATK_AMR()==false &&
                    MOT[MMColor].fHasTray==false) ||
                   (Sen[SenColorHasTray].IsOn() &&
                    MOT[MMColor].fHasTray==false))
                {
                    if(USE_TRAY_MAPPING==1 &&                                   //JerryYang 20250113 : Color上方TrayID      //USE_KEYENCE_EMPTY 0 ccd ;1 keyence auto 2; 2 keyence Empty
                       TestIF_File.bEnableTrayID2==true &&                      //wei 20180808 MR Tray ID 2
                       USE_KEYENCE_EMPTY==3)
                    {
                        #ifndef SOFT_SIMULTE
                        if(LastSet.iRealDummy!=DUMMY)                           //wei 20200604 空跑不讀取Empty Tray ID
                        #endif
                        {
                            InitialTrayID2Task();
                            Task=80;
                            break;
                        }
                    }
                    else
                    {
                        MOT[MMColor_Car].ClearTray(__FUNC__);
                        MOT[MMColor].SetTray(NULL_IC, __FUNC__);
                    }
                }
                Task=100;
                #endif
            }
            break;
        case 80:
            if(fTrayMapping->DoTrayIDKeyence2(2))                               //JerryYang 20240422
            {
                bEmptyReadIDOK=true;
                if(asTrayIDData2.AnsiPos("ERROR")!=0)
                {
                    Task=85;
                    Cylinder[C_Color_Fix].Off();
                    ShowMyMessage("Color TrayID Read Error, please remove the tray!");
                }
                else if(fTrayMapping->DoTrayIDCheck(asTrayIDData2)==false)
                {
                    Task=85;
                    Cylinder[C_Color_Fix].Off();
                    ShowMyMessage("Color Tray ID Duplicate error, please remove the tray!");
                    break;
                }
                else
                {
                    Task=100;
                }
            }
            break;
        case 85:
            if(Sen[SenColorCarHasTray].IsOff()==false || Sen[SenColorCCWDete].IsOff()==false ||
               Sen[SenColorHasTray].IsOff()==false || Sen[SenColorCWDete].IsOff()==false)
            {
                if(asTrayIDData2.AnsiPos("ERROR")!=0)
                {
                    ShowMyMessage("Color TrayID Read Error, please remove the tray!");
                }
                else
                {
                    ShowMyMessage("Color Tray ID Duplicate error, please remove the tray!");
                }
            }
            else
            {
                Task=1;
            }
            break;
        case 100:
            if(MOT[MMColor_Car].fHasTray)
            {
                if(MOT[MTrayX].fHasTray &&
                   MOT[MTrayX].ReadPos()==Prod.iXTrayColor &&
                   bIsPlacingToBuffer==true)                                    //JerryYang 20250828 : fix color誤退TRAY
                {
                    if(iReceiveColorTray==0)                                    //Sam 20211119 : 放 Tray 增加保護
                    {
                        fColorCanSupplyNewTray=false;
                        iReceiveColorTray=1;
                        InitAutoColorReceiveTask();
                    }
                    DoAutoColorDelay.SetSecAndOn(1);
                    Task=150;
                }
                else
                {
                    InitColorTrayToRearTask();
                    Task=300;
                }
            }
            else
            {
                InitLoadNewColorTrayToCarTask();
                Task=200;

                if(iReceiveColorTray!=0 ||                                      //jou 980421 Color tray no need relay one tray in Color_car
                   (MOT[MTrayX].fHasTray &&
                    MOT[MTrayX].ReadPos()==Prod.iXTrayColor) ||
                   MOT[MMColor].fHasTray)
                {
                    DoAutoColorDelay.SetSecAndOn(2);
                    Task=150;
                }
            }
            break;
        case 150:
            if(DoAutoColorDelay.Off())
                Task=1;
            break;
        case 200:
            bLoadNewColorTrayToCarStart=true;                                   //Sam 20211119 : 放 Tray 增加保護
            if(DoLoadNewColorTrayToCar())
            {
                bLoadNewColorTrayToCarStart=false;                              //Sam 20211119 : 放 Tray 增加保護
                InitLoadNewColorTrayToCarTask();
                Task=1;
            }
            break;
        case 300:
            if(DoColorTrayToRear())
            {
                Task=1;
            }
            break;
    }
}
//------------------------------------------------------------------------------
int iUnLoadNewColorTrayTask=1;
TQPF_Timer DoUnLoadNewColorToStackDelay;
void InitUnLoadNewColorTrayTask()
{
    iUnLoadNewColorTrayTask=1;
}
//------------------------------------------------------------------------------
bool DoUnLoadNewColorToStack()
{
    int &Task=iUnLoadNewColorTrayTask;

    switch(Task)
    {
        case 1:
            #ifndef SOFT_SIMULTE
            if(TrayMoveStatus(2,__FUNC__)==0  &&                                //Sam 20210325 : 整合 tray move 狀態
               LastSet.iRealDummy!=DUMMY)                                       //Steven 20130930
            {
                Task=10;
                break;
            }
            #endif
            MOT[MMColorZ].SetTray(NULL_IC, __FUNC__);

            if(Sen[SenColorCarHasTray].IsOff() &&
               Sen[SenColorCCWDete].IsOff() &&                                  //kevin 20120518 有tray在軌道上升到分離高度
               Cylinder[C_ColorLoaderZ_Select].OnStatus())                      //Sam 20210728 : 確保分離汽缸已經被打開
                Task=200;
            else
                Task=50;
            break;
        case 10:
            if(Sen[SenColorHasTray].IsOn() &&
               Sen[SenColorCWDete].IsOn())                                      //Steven 20130917 : Double Tray問題,測試 Color
            {
                Task=50;
                break;
            }
            else
            {
                TrayMoveOut(true, 2);
//                SW[SwACColorCCW].On();
//                SW[SwACColorCW].On();
            }
            DoUnLoadNewColorToStackDelay.SetSecAndOn(2);
            Task=20;
            break;
        case 20:
            if(DoUnLoadNewColorToStackDelay.Off())
                Task=1;
            break;
        case 50:
            #ifndef SOFT_SIMULTE
            if(TrayMoveStatus(2,__FUNC__)==0  &&
               LastSet.iRealDummy!=DUMMY)                                       //Sam 20211008 : 確保汽缸上前的狀態皮帶是一直再帶動轉
            {
                TrayMoveOut(true, 2);
                DoUnLoadNewColorToStackDelay.SetSecAndOn(1);
                Task=20;
                break;
            }
            #endif

            MOT[MMColorZ].SetTray(NULL_IC, __FUNC__);
            if(CylinderMiddle(C_Color_Up))
            {
                TrayMoveOut(false, 2);
                if(Cylinder[C_ColorTrackFloodgate].Enable)
                    Cylinder[C_ColorTrackFloodgate].Off();
//                SW[SwACColorCW].Off();
//                SW[SwACColorCCW].Off();
                Task=100;
            }
            break;
        case 100:
            if(LastSet.iRealDummy==DUMMY ||
               Cylinder[C_ColorLoaderZ_Select].Push())                          //Steven 20111212
            {
                DoUnLoadNewColorToStackDelay.SetSecAndOn(0.5);
                Task=150;
            }
            break;
        case 150:
            if(DoUnLoadNewColorToStackDelay.Off())
                Task=200;
            break;
        case 200:
            if(CylinderUp(C_Color_Up))
                Task=300;
            break;
        case 300:
            if(LastSet.iRealDummy==DUMMY ||
               Cylinder[C_ColorLoaderZ_Select].Pop())                           //Steven 20111212
            {
                Task=400;
            }
            break;
        case 400:                                                               //kevin 20161129  color 收tray sensor沒調好 需alarm
            if(LastSet.iRealDummy==DUMMY ||
               (Sen[SenColorCarHasTray].IsOff() &&
                Sen[SenColorCCWDete].IsOff()))                                  //kevin 20161129 沒有tray在軌道
            {
                DoUnLoadNewColorToStackDelay.SetSecAndOn(Ld_UldDelayTime.ULD_TrayBackDelay);
                MOT[MMColorZ].ClearTray(__FUNC__);
                Task=500;
            }
            else
            {
                #ifdef SOFT_SIMULTE
                MOT[MMColorZ].ClearTray(__FUNC__);                              //JerryYang 20161222 (jou) 修正軟體模擬hang up
                Task=500;
                #else
                ShowErrorMessage("MES1420", K_RETRY, MMColor_Car);
                #endif
            }
            break;
        case 500:
            if(DoUnLoadNewColorToStackDelay.Off())
            {
                if(CylinderLower(C_Color_Up))
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
void InitAutoColorReceiveTask()
{
    iAutoColorReceiveTask=1;
}
//------------------------------------------------------------------------------
void DoAutoColorReceive()
{
    int &Task=iAutoColorReceiveTask;
    static bool bNeedAlarm=false;                                               //Sam 20221027 : 整合 P24 功能
    AnsiString str1, str2;
    static bool bHasColorTrayPreAlarm=false;                                    //Ifor 20170315 (wei) add 新增Empty/Color Tray Pre Alarm 功能

    switch(Task)
    {
        case 1:
            if(IniConfig.bP04ColorIsEmptyUnloader)
            {
                Task=50;
            }
            else
            {
                InitColorTrayToFrontTask();
                Task=100;
            }
            break;
        case 50:
            if(MOT[MTrayX].ReadPos()<=(Prod.iXTrayEmpty + 500))
            {
                MOT[MOutArmX].fCanMove=true;
                MOT[MOutArmY].fCanMove=true;
                InitColorTrayToFrontTask();
                Task=100;
            }
            break;
        case 100:
            if(DoColorTrayToFront())
            {
                iReceiveColorTray=2;
                if(MOT[MMColor_Car].fHasTray)
                {
                    InitUnLoadNewColorTrayTask();
                    Task=200;
                }
                else
                {
                    Task=1;
                    iReceiveColorTray=0;
                    if(MOT[MTrayX].ReadPos()<=(Prod.iXTrayEmpty+500))
                    {
                        MOT[MOutArmX].fCanMove=true;
                        MOT[MOutArmY].fCanMove=true;
                    }
                }
            }
            break;
        case 200:
            if(CosFunction.bUseEmptyColorTrayPreAlarm==true &&                  //Ifor 20170322 add Empty/Color Tray Pre Alarm 功能於Tray上升前判斷與Alarm
               IniConfig.bP32EmptyColorTrayPreAlarm==true)
            {
                if(Sen[SnColorIsFull].IsOn()==true &&                           //上升前若滿Tray
                   bHasColorTrayPreAlarm==true)                                 //PreAlarm =true Show Alarm
                {
                    ShowErrorMessage("MES1420", K_RETRY, MMColor_Car);
                    break;
                }
                else
                {
                    bHasColorTrayPreAlarm=false;
                }
            }

            bUnLoadNewColorToStackStart=true;                                   //Sam 20211119 : 放 Tray 增加保護
            if(DoUnLoadNewColorToStack())
            {
                bUnLoadNewColorToStackStart=false;                              //Sam 20211119 : 放 Tray 增加保護
                if(MOT[MMColor_Car].Tray.iNeedManualRemoved>=0)                 //Sam 20230317 : 修正 P24 功能
                {
                    MOT[MMColor_Car].Tray.iNeedManualRemoved=-1;
                    bNeedAlarm=true;
                }
                else
                {
                    bNeedAlarm=false;
                }
                MOT[MMColor_Car].ClearTray(__FUNC__);
//                iReceiveColorTray=0;                                          //Steven 20141120 : Enpty & Color 滿盤偵測
                Task=300;
            }
            break;
        case 300:
            if(CosFunction.bUseEmptyColorTrayPreAlarm==true &&                                                                                                  //Ifor 20170328 (wei) add Empty/Color Tray Pre Alarm 於入Tray上升後判斷是否有遮到滿Tray Sensor
               IniConfig.bP32EmptyColorTrayPreAlarm==true)
            {
                if(Sen[SnColorIsFull].IsOn()==true ||
                   (Sen[SnColorIsPreAlarm].Enable &&
                    Sen[SnColorIsPreAlarm].IsOn()==true))                                                                                                       //wei 20170802 Pre alarm sensor
                {
                    if(bHasColorTrayPreAlarm==false)
                    {
                        bHasColorTrayPreAlarm=true;
                        if(MyMessageBox->Visible==true)                                                                                                         //Ifor 20160901 Alarm 前若form有開啟先關閉
                            MyMessageBox->Close();
                        iUnLoaderCount=8;                                                                                                                       //Ifor 20160829 必須不為0 Handler才不停機
                        str1.sprintf("Color Tray is full with trays, Please take it off");
                        str2.sprintf("Color上的Tray盤已滿,請取下Tray盤");
                        RunInfo.iPreAlarmPosition=9;                                                                                                            //Ifor 20171024 (wei) : add Pre Alarm Position 1:LD 2:Auto1 3:Auto2 4:Auto3 5:Fix1 6:Fix2 7:Fix3 8:Empty 9:Color
                        ShowUnloaderTrayMessage(str1, str2);
                    }
                }
                else
                {
                    bHasColorTrayPreAlarm=false;
                }
                iReceiveColorTray=0;
                Task=1;
            }
            else
            {
                if(Sen[SnColorIsFull].IsOn()==true)                                                                                                             //Steven 20141120 : Enpty & Color 滿盤偵測
                {
                    ShowErrorMessage("MES1420", K_RETRY, MMColor_Car);
                }
                else
                {
                    if(bNeedAlarm)                                                                                                                              //Sam 20221027 : 整合 P24 功能
                    {
                        #ifdef SOFT_SIMULTE                                                                                                                     //Sam 20220915 : 修正 P24 誤提示問題
                            ShowMyMessage("Please remove color tray manually. And check if has IC inside the tray.", "請將 Color 上的 Tray 手動取走並檢查有無 IC !!!");      //Sam 20240116 : 修改提示
                            bNeedAlarm=false;                                                                                                                   //Sam 20221027 : 整合 P24 功能
                            bHasColorTrayPreAlarm=false;
                            iReceiveColorTray=0;
                            Task=1;
                        #else
                        if(Sen[SenColorSelectHasTray].IsOn())
                        {
                            ShowMyMessage("Please remove color tray manually. And check if has IC inside the tray.", "請將 Color 上的 Tray 手動取走並檢查有無 IC !!!");      //Sam 20240116 : 修改提示
                            Task=300;
                            break;
                        }
                        else if(Sen[SenColorSelectHasTray].IsOff())
                        {
                            bNeedAlarm=false;                                                                                                                   //Sam 20221027 : 整合 P24 功能
                            bHasColorTrayPreAlarm=false;
                            iReceiveColorTray=0;
                            Task=1;
                        }
                        #endif
                    }
                    else
                    {
                        bHasColorTrayPreAlarm=false;
                        iReceiveColorTray=0;
                        Task=1;
                    }
                }
            }
            break;
    }
}
//------------------------------------------------------------------------------
bool ForTERAPOWERCheckColorSensor(int iPos)                                     //Sam 20180525 (wei) : 晶兆成 Auto1~3 Color sensor detect By FromEmptyColor
{
    int irsm=0;
    bool bColorSensorFlag=false;
    if(LastSet.iRunStartMode==rsmContinuRetest ||
       LastSet.iRunStartMode==rsmCInitialRetest)
    {
        irsm=RT;
    }
    else
    {
        irsm=FT;
    }

    if(TrayForm.AutoFromEmptyColor[irsm][iPos]==0)                              //Tray From Empty
    {
        bColorSensorFlag=Sen[SnAutoColorTrayDete[iPos]].IsOn();                 //Sam 20180704 : 晶兆成 color sensor 軟硬體顯示與7系列一樣，亮起是紅色//需要是黑 Tray ,滅掉是紅色 Tray 要報警。
    }
    else                                                                        //Tray From Color
    {
        bColorSensorFlag=Sen[SnAutoColorTrayDete[iPos]].IsOff();                //Sam 20180704 : 晶兆成 color sensor 軟硬體顯示與7系列一樣，亮起是紅色//需要是紅 Tray ,亮起是黑色 Tray 要報警。
    }
    return bColorSensorFlag;
}
//------------------------------------------------------------------------------
int iColorTrayReadCIDTask=1;
TQPF_Timer tNFCColorDelay;
void InitColorTrayReadCIDTask()
{
    iColorTrayReadCIDTask=1;
}
//------------------------------------------------------------------------------
int DoColorTrayReadCID()
{
    static int iCount=0;
    int iResult=0;
    int &Task=iColorTrayReadCIDTask;

    switch(Task)
    {
        case 1:
            if(iReadCIDAction==ePortLoader)
                break;

            iReadCIDAction=ePortColor;
            iCount=0;
            Task=10;
            // NOTE: faithful fall-through to case 10 (no break in golden :1501)
        case 10:
            fTrayMapping->ldRFID->sBlockID2="";
            fTrayMapping->ldRFID->sUID2="";
            fTrayMapping->ldRFID->bReadUID2=false;
            fTrayMapping->btnReadUID2->Click();
            tNFCColorDelay.SetSecAndOn(1);
            Task=100;
//            break;
            // NOTE: faithful fall-through to case 100 -- golden :1509 has the
            //       `break;` COMMENTED OUT, so case 100 runs in the same tick.
        case 100:
            if(fTrayMapping->ldRFID->bReadUID2==true)
            {
                Task=1000;
                tNFCColorDelay.SetSecAndOn(0.1);
            }
            else if(tNFCColorDelay.Off())
            {
                iCount++;
                if(iCount<2)
                    Task=10;
                else
                    Task=1000;
            }
            break;
        case 1000:
            if(tNFCColorDelay.Off())
            {
                fTrayMapping->btnLdRFIDReadBlock2->Click();
                tNFCColorDelay.SetSecAndOn(1);
                Task=2000;
            }
            break;
        case 2000:
            if(fTrayMapping->ldRFID->sBlockID2!="")
            {
                if(fTrayMapping->ldRFID->sBlockID2.Length()!=9)
                    iResult=-1;
                else
                    iResult=1;
            }
            else if(tNFCColorDelay.Off())
            {
                iCount++;
                if(iCount<2)
                    Task=10;
                else
                    iResult=-1;
            }
    }

    return iResult;
}
//------------------------------------------------------------------------------
