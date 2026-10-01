// ===========================================================================
//  MainTimerESD.cpp -- S-13 (St02): golden TfMain::TimerESDTimer.  AI(W906-S13) 20261001 (St02-E).
//
//  golden 906_0625_Steven main.cpp:30911-31173.  main.dfm:17313 TimerESD stores no Interval / Enabled => VCL 1000 ms,
//  enabled at design time; FormClose :11708 disables it.  MainTimersSt02.cpp calls it once per 1000 ms from
//  WebBridgeTags.cpp PumpTick and from the modal-wait tick (tools/wb_serve.cpp:7621), and stops once bSystemClose is set.
//  Design: St02-E2 ST02_S13_CLAIMS_20261001, sections E0-E6:
//    E0 :30913-30931  guards; golden's `#ifdef SOFT_SIMULTE return;` (:30921-30923) is kept -- the SIM build does nothing
//    E1 :30932-30936  ASE remote Auto Clean                                   GATED in place (below)
//    E2 :30937-31026  the tester alarm queue tESDError                        translated, body in HandlerTesterSide.cpp
//    E3 :31029-31037  tGPIBMsg (RCMD:S10F3 / TESTER_ERROR text) -> ShowMyMessage    translated
//    E4 :31041-31049  [I29] site yield record "By Interval"                   translated
//    E5 :31052-31149  [O06-8] one production log line every 10 / 30 minutes  translated
//    E6 :31151-31170  SPIL event-log path check (golden: SHIP only)          GATED in place (below)
//  WHERE E2 LIVES.  The queue is THandlerTesterSide::tESDError (TesterComm/Handler/HandlerTesterSide.h; golden TfMain member
//    main.h:1394; HandlerGpibMsg.cpp adds the codes), in ht9045_testercomm_handler, which ht9045_sm does not link.  So the
//    golden E2 text is THandlerTesterSide::TimerESDDrainESDError (HandlerTesterSide.cpp, end), reached through
//    W906_ESDErrorDrainHook, installed by TesterCommWiring.cpp W906_TesterCommInit.  Not installed (HT9045_TESTERCOMM=0,
//    wb_publish, ctests without TesterComm) = there is no queue = golden's `tESDError->Count!=0` is false.
//  The other golden TfMain members:
//    tGPIBMsg (main.h:1395)                = AutoRetest.cpp:284 W906ART_fMain_tGPIBMsg (producers AutoRetest.cpp:1742 / :1754,
//                                            golden AutoRetest.cpp:1408 / :1420)
//    iSaveLogTimePeridCount (main.h:1159)  = file static below (ctor :2261 sets 0; only this timer uses it)
//    TimerRecordLoaderDate()               = W906_MainClose_TimerRecordLoaderDate (FileRW/MainClose.cpp:2879, St01) -> :497.  Programs other
//                                            than wb_serve get the fallback in MainTimerESDFallback.cpp (returns 0).
//  No waiting and no thread here: ShowErrorMessage / ShowMyMessage are the host's golden ShowModal waits (wb_serve
//  ForwardShowErrorMessage / W906MbShowMyMessage).  While such a box waits, bRun stays true, as golden.
// ===========================================================================
#include "MachineType.h"          // SOFT_SIMULTE -- before the #ifdef below, or the body would run in SIM too (HeaterSimTick.cpp:29); tNotUse / tTrayBox, ChangeToFloatNonPcnt
#include "cmydef.h"               // InitialOK, iTestBinCount, asTrayAlias, iTo3Unload, Tempture_Ambient
#include "Config.h"               // IniConfig
#include "cprod.h"                // Prod, RunInfo, TestIF_File
#include "LastSet.h"              // LastSet (iTemperature, BinCT, SendCT)
#include "aHotPlateSubstrate.h"   // TestSocket (iShtRow / iShtCol); not mykitsuck.h (KNOWLEDGE.md, the two TMyKitSuck)
#include "cSocket.h"              // ArmData[]->ArmSKET[][]->GetPCA()
#include "cpublic.h"              // ProductionLog
#include "canary_support.h"       // ShowMyMessage (not with cMyDB.h in one TU, techniques §5)
#include "forms/fMain.h"          // fMain->cbSetupFileName / edWorkTemperBase
#include "forms/fContactCT.h"     // fContactCT->SaveSiteYield

extern TStringList W906ART_fMain_tGPIBMsg;   // AutoRetest.cpp:284 -- golden fMain->tGPIBMsg (main.h:1395)
int W906_MainClose_TimerRecordLoaderDate();  // FileRW/MainClose.cpp:2879 -> :497 (anonymous namespace) -- golden TfMain::TimerRecordLoaderDate

namespace ht9045 {

void (*W906_ESDErrorDrainHook)() = 0;        // E2 seat: THandlerTesterSide::ESDErrorDrainHook once TesterComm is up (header)

namespace {
int iSaveLogTimePeridCount=0;                                                   //JerryYang 20151026 ProductionData存檔時間計數   (golden TfMain member main.h:1159, ctor :2261)
int (*g_recordLoaderDate)() = &W906_MainClose_TimerRecordLoaderDate;                 // ctest seam only (W906_TimerESDSetRecordLoaderDate)
}  // namespace

void W906_TimerESDSetRecordLoaderDate(int (*fn)()) { g_recordLoaderDate = fn ? fn : &W906_MainClose_TimerRecordLoaderDate; }   // ctest only

// golden TfMain::TimerESDTimer, main.cpp:30911-31173
void W906_TimerESDTimer()
{
    static bool bRun=false;
    static int iIntervalCount=0;                                                //Ifor 20151222 :Yield Record 計數暫存器
    static AnsiString asEventlogPath="";                                        //Ifor 20180821 :Add 判斷ESD Alarm Code 是否相同   [W906] golden :30915 also declares asOldESDError -- it moved with E2 (HandlerTesterSide.cpp)
//    static double dESDAlarmTime=0;                                            //Ifor 20180821 :Add 判斷ESD Alarm間隔時間
//    double dTime=0;                                                           //Ifor 20180821 :Add 判斷ESD Alarm間隔時間
#ifndef SOFT_SIMULTE
    static int iEventLogSavePeriod=600;                                         //JerryYang 20181210 add
#endif
#ifdef SOFT_SIMULTE
    return;
#endif
    AnsiString str1="", str2="";                                                // [W906] golden :30924 `int iRet;` moved with E2
    TStringList *tGPIBMsg=&W906ART_fMain_tGPIBMsg;                              // [W906] golden TfMain member (main.h:1395) = the AutoRetest.cpp:284 list

    if(InitialOK==false || bRun)
    {
        return;
    }
    bRun=true;
#if 0   // GATE E1 (AI(W906-S13) 20261001): S25 (ASE) and not ported -- ASESendMessage is the ASE socket form ("ASE_K Socket/aseTest.h",
        //   TASESendMessage; aoutarm.cpp:3322 GATE G12) and TfMain::AutoCleanButtonClick is not in the port.  golden :30932-30936 VERBATIM:
    if(ASESendMessage->bASE_AutoClean)                                          //kevin 20160722 遠端啟動 autoclean
    {
        ASESendMessage->bASE_AutoClean=false;                                   //kevin 20160722 遠端啟動 autoclean
        AutoCleanButtonClick(this);
    }
#endif
    //Ifor 20180821 : Add 避免ESD Alarm 短時間內重複發生
    //==>
    if(W906_ESDErrorDrainHook!=0)                                               // [W906] golden :30939-31025 = THandlerTesterSide::TimerESDDrainESDError (header)
        W906_ESDErrorDrainHook();
    //<==
    //Ifor 20180821 : Add 避免ESD Alarm 短時間內重複發生

    if(tGPIBMsg->Count!=0)                                                      //Steven 20190116 : GPIB顯示MSG
    {
        ShowMyMessage(tGPIBMsg->Strings[0], "");
        tGPIBMsg->Delete(0);
        if(tGPIBMsg->Count==0)                                                  //釋放記憶體
        {
            tGPIBMsg->Clear();
        }
    }

    //=================== Yield Record Start======================
    //Ifor 20151221 :新增 Yield Record 格式紀錄
    if(IniConfig.bI29EnableYieldRecord==true)                                   //Ifor 20151221 :新增判斷Yeild CosFunction與IniConfig 是否開啟
    {
        iIntervalCount++;
        if(iIntervalCount>=IniConfig.fI29YieldRecordInterval)                   //Ifor 20151221 :判斷Yield Record 是否到達設定時間
        {
            iIntervalCount=0;                                                   //清除秒數暫存器
            fContactCT->SaveSiteYield("By Interval");
        }
    }
    //=================== Yield Record End======================

    if(IniConfig.bO06SaveLogTimePeriod)                                         //JerryYang 20151026 : 使用ProductionData週期時間存檔
    {
        iSaveLogTimePeridCount++;
        if(iSaveLogTimePeridCount>=IniConfig.iO06SaveLogTimePeriod*1200+600)    //JerryYang 20151026 : ProductionData週期時間(0->10mins,1->30mins)
        {
            iSaveLogTimePeridCount=0;
            AnsiString str, sTemp, sTemp1, sSetupfile, sTemper, sTray[eTrayCount], sTrayPassFail, sBin;                 //JerryYang 20160201 add sTrayPassFail   //JerryYang 20220909 : 9->24
            AnsiString Str1, Buffer1="", Buffer2="";
            g_recordLoaderDate();                                               // golden fMain->TimerRecordLoaderDate();   [W906] W906_MainClose_TimerRecordLoaderDate (header)
            sSetupfile=fMain->cbSetupFileName->Text;
            if(LastSet.iTemperature==Tempture_Ambient)
                sTemper="25.0";
            else
                sTemper=fMain->edWorkTemperBase->Text.c_str();

            for(int i=0; i<iTestBinCount; i++)                                  //QQQ
            {
                int iTray=Prod.iT6PosCate[i];
                if(iTray>0)
                {
                    if(sTray[iTray-1]=="")
                    {
                        sTray[iTray-1]=IntToStr(i);
                    }
                    else
                    {
                        sTray[iTray-1]=sTray[iTray-1]+","+IntToStr(i);
                    }
                }
            }

            Buffer1="";
            Buffer2="";
            for(int i=0; i<eTrayCount; i++)
            {
                if(Prod.iTrayType[i]!=tNotUse &&
                   Prod.iTrayType[i]!=tTrayBox)
                {
                    Str1.sprintf("%s:%d;", asTrayAlias[i], LastSet.BinCT[0][iTo3Unload[i]]);
                    Buffer1=Buffer1+Str1;

                    if(Prod.iIsFailT6[i]==1)                                    //Steven 20240105 : Prod.bIsPass --> Prod.iIsFailT6
                    {
                        if(Prod.iIfErrorT6==i)
                        {
                            sTrayPassFail="FE";
                        }
                        else
                        {
                            sTrayPassFail="F";
                        }
                    }
                    else
                    {
                        sTrayPassFail="P";
                    }

                    Str1.sprintf("%s(%s):%s;", asTrayAlias[i], sTrayPassFail, sTray[i]);
                    Buffer2=Buffer2+Str1;
                }
            }

            str.sprintf("Load:%d;Total:%d;%s%s",
                        LastSet.SendCT[0],
                        RunInfo.iUnloadCount,
                        Buffer1,
                        Buffer2);

            // [W906] looks wrong, translated as is: the loop starts at row 1 (row 0 is never logged; a 1-row socket logs
            //   nothing) and the label depends on i only, so every column of a row gets the same two letters.
            for(int i=1; i<TestSocket.iShtRow; i++)                             //記錄Arm良率
            {
                for(int j=0; j<TestSocket.iShtCol; j++)
                {
                    double dColA=double(ArmData[0]->ArmSKET[i][j]->GetPCA());
                    double dColB=double(ArmData[1]->ArmSKET[i][j]->GetPCA());
                    if(TestIF_File.iShuttleMode==0)
                    {
                        sTemp1.sprintf("%3.2f%%", (dColA+dColB)/2);
                    }
                    else
                    {
                        if(TestIF_File.iShuttle_Sel==0)
                        {
                            sTemp1.sprintf("%3.2f%%", dColA);
                        }
                        else
                        {
                            sTemp1.sprintf("%3.2f%%", dColB);
                        }
                    }

                    sTemp.sprintf("%c%c:%s;", 'A'+(int)(ChangeToFloatNonPcnt((double)((i-1)), (double)(TestSocket.iShtCol))), 'a'+((i-1)%TestSocket.iShtCol), sTemp1);  //Steven 20260421 : cast to int for %c
                    str=str+sTemp;
                }
            }
            str=str+"SetupFile:"+sSetupfile+";Temperature:"+sTemper+";";        //JerryYang 20160201 修改Production log格式
            ProductionLog(str, true);
        }
    }

#ifndef SOFT_SIMULTE
#if 0   // GATE E6 (AI(W906-S13) 20261001): S25 -- IniConfig.bSPILFunction is set for the SPIL customer codes only
        //   (CosFunction.cpp:326 / :334 / :339 / :344 / :349), same decision as S-15's VTEST gate (St02-M 20261001).  To open it
        //   later: CheckAndReadIniData may write the default into D:\HT9045\config\config.ini -- back it up first (§0 rule 6).
        //   golden :31152-31169 VERBATIM:
    if(IniConfig.bSPILFunction==true)                                           //JerryYang 20181210 SPIL要求檢查Event log,有異常需跳message
    {
        iEventLogSavePeriod++;
        if(iEventLogSavePeriod>=600)
        {
            if(SystemStart)
            {
                iEventLogSavePeriod=0;
                asEventlogPath=CheckAndReadIniData("D:\\HT9045\\config\\config.ini", "Event Log","AutoSaveEventLogPath", AnsiString("D:\\RMS"));
                if(!DirectoryExists(asEventlogPath))
                {
                    str1.sprintf("Event log存檔路徑異常,請確認網路是否正常連線!");
                    str2.sprintf("Path:%s", asEventlogPath);
                    ShowMyMessage(str1, str2);
                }
            }
        }
    }
#else
    (void)iEventLogSavePeriod;
#endif
#endif

    bRun=false;                                                                 //這行要在最下面
}

}  // namespace ht9045
