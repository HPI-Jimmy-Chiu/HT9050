// =============================================================================
//  cObserver_TimeInfo.cpp  --  golden RecordTimeInfo（cObserver.cpp:1846-2134）
//
//  AI(W906-W2-3) 20260926: new file. 從 906 golden 逐行搬（cp950 解碼成 UTF-8，零 U+FFFD，依行號機械複製）。
//  St01 AUDIT_PROD S116 ①：cObserver.cpp 的 static RecordTimeInfo() 是空樁（GATE REGISTER W2-3「290 行未 recon 完」），
//  而 RecordEndTestTime（cObserver.cpp，活的）每次測完都呼叫它 ⇒ 觀察頁的 Test／Index 時間表（TimeInfoGrid、strngrdTestTime）一直是空的，
//  遠端 HTGR 209 IndexTime（Command.cpp:12990）與 atester_ProcessCount 的 GATE J 讀 Cells[5][11] 讀到空字串，
//  RunInfo.TestTime／IndexCycleTime（SECS SV）、RunInfo.dTestTimeSec、dTestSec、ASE 的 fRecindexCycleTim、每個 site 的生產紀錄時間欄都沒寫。
//
//  搬法：本體放本檔、cObserver.cpp 那支 static 空樁同一行改成呼叫 W906_RecordTimeInfoBody()（cObserver.cpp 後面的行號不動）。
//    golden 的檔案範圍資料（:63-81）：TestSocketTimeInfo／OEERecevieTimeInfo 在 cObserver.cpp（`static` 已拿掉，跟 golden 一樣是全域）；
//    TestTimeInfoRecord[2][10]、TestReceiveTimeInfo＋TestReceiveTimeInfoRecord[2][10] 只有本函式用 ⇒ 定義在本檔。
//    struct TestTimeInfo 與 cObserver.cpp:2217 那一份逐字相同（同一個型別定義出現在兩個 TU 是 C++ 允許的，前提是一字不差 —— 改其中一份要一起改）。
//  閘：G1 fLotInfo->lbESDReportData（St01 的門面沒有；ESD 報表的來源也沒翻）；G2 fShowMessage 的兩個標籤（畫面）。
//  門面補的成員：fObserver 的 iTestTimeCT／iTestTime／dTotoalIndexCycleTime／iTestReceiveTimeCount／dRecordTestTime[10]（forms/fObserver.h，同行）、
//    fGroundMan->asGroundVaule（forms/fGroundMan.h，建構子照 golden :125 設 "NA"）。
//  ⚠ 行為（照 golden）：dTestSec 從此每測完一次就更新（ATC/ATCInterface.cpp:2241 的 ATC 測試時間補償讀它；atester.cpp:5307 的初始值照舊）。
//  已知差異：OEE 的 iTestReceiveTimeCount 產生端（golden main.cpp:16974 ++）沒翻 ⇒ 恆 0（OEE 分支走「<20 ⇒ sTestReceiveTime=""」那一臂）。
//
//  ctest：tests/test_record_time_info.cpp（RecordTimeInfo）。
// =============================================================================
#include "forms/fObserver.h"         // fObserver->TimeInfoGrid / strngrdTestTime / sg_ListTimeReceiveInfoGrid / fRecordIndexTime / RecordIndexCycle
#include "forms/fGroundMan.h"        // fGroundMan->asGroundVaule
#include "aHotPlateSubstrate.h"      // TestSocket (NOT mykitsuck.h -- the two-TMyKitSuck ODR gotcha, same as cObserver.cpp:85)
#include "cprod.h"                   // RunInfo / TestIF_File
#include "cmydef.h"                  // dTestSec / fRecindexCycleTim / RecindexCycleTim / bIndexTimeSet / fIndexTime / sBufferSOT / sBufferEOT / QueueTestTime / QueueCycleTime / iIndexArmWhich / CUSTOMER_CODE
#include "cpublic.h"                 // ConvertMSecToSPC
#include "CosFunction.h"             // CosFunction.bOEEFunction
#include "Config.h"                  // IniConfig.bShowTimeInfo
#include "MachineType.h"             // CC_KYEC_CHEN / CC_SCC / CC_JCET
#include "vclcompat/vcl_compat.h"

// golden cObserver.cpp:63-81 -- see the banner
struct TestTimeInfo
{
    int iStartMin;
    int iStartSec;
    int iStartMSec;
    int iEndMin;
    int iEndSec;
    int iEndMSec;
};
extern TestTimeInfo TestSocketTimeInfo[2];     // cObserver.cpp (golden :73)
extern TestTimeInfo OEERecevieTimeInfo[2];     // cObserver.cpp (golden :74)
TestTimeInfo TestTimeInfoRecord[2][10];        // golden :72
struct TestReceiveTimeInfo                     // golden :76-80
{
    int iMin;
    int iSec;
    int iMSec;
};
TestReceiveTimeInfo TestReceiveTimeInfoRecord[2][10];   // golden :81

void W906_RecordTimeInfoBody();

//------------------------------------------------------------------------------
void W906_RecordTimeInfoBody()                                                  // golden cObserver.cpp:1846 RecordTimeInfo()
{
    int iT, iS, iE, iTMSEC, iTM, IndexTime=0, IndexTimeCT=0;
    double temp;
    fObserver->iTestTimeCT=0;
    fObserver->iTestTime=0;
    AnsiString str;
    for(int i=0; i<9; i++)
    {
        for(int j=0; j<2; j++)
        {
            TestTimeInfoRecord[j][i].iStartMin =TestTimeInfoRecord[j][i+1].iStartMin;
            TestTimeInfoRecord[j][i].iStartSec =TestTimeInfoRecord[j][i+1].iStartSec;
            TestTimeInfoRecord[j][i].iStartMSec=TestTimeInfoRecord[j][i+1].iStartMSec;
            TestTimeInfoRecord[j][i].iEndMin   =TestTimeInfoRecord[j][i+1].iEndMin;
            TestTimeInfoRecord[j][i].iEndSec   =TestTimeInfoRecord[j][i+1].iEndSec;
            TestTimeInfoRecord[j][i].iEndMSec  =TestTimeInfoRecord[j][i+1].iEndMSec;

            if(CosFunction.bOEEFunction)                                        //Steven 20180417 (Jou) : OEE功能
            {                                                                   //KaiChen 20171127 (Steven) ：超豐 OEE 新增 Test Time、Index Time
                if(fObserver->iTestReceiveTimeCount<=20)                        //Sam 20200317 : Fix OEE Test Time
                {
                    TestReceiveTimeInfoRecord[j][i].iMin = TestReceiveTimeInfoRecord[j][i+1].iMin;
                    TestReceiveTimeInfoRecord[j][i].iSec = TestReceiveTimeInfoRecord[j][i+1].iSec;
                    TestReceiveTimeInfoRecord[j][i].iMSec= TestReceiveTimeInfoRecord[j][i+1].iMSec;
                }
            }
        }
    }

    for(int i=0; i<2; i++)
    {
        TestTimeInfoRecord[i][9].iStartMin =TestSocketTimeInfo[i].iStartMin;
        TestTimeInfoRecord[i][9].iStartSec =TestSocketTimeInfo[i].iStartSec;
        TestTimeInfoRecord[i][9].iStartMSec=TestSocketTimeInfo[i].iStartMSec;
        TestTimeInfoRecord[i][9].iEndMin   =TestSocketTimeInfo[i].iEndMin;
        TestTimeInfoRecord[i][9].iEndSec   =TestSocketTimeInfo[i].iEndSec;
        TestTimeInfoRecord[i][9].iEndMSec  =TestSocketTimeInfo[i].iEndMSec;
        if(CosFunction.bOEEFunction)                                            //Steven 20180417 (Jou) : OEE功能
        {
            TestReceiveTimeInfoRecord[i][9].iMin = OEERecevieTimeInfo[i].iStartMin;    //KaiChen 20171127 (Steven) ：超豐 OEE 新增 Test Time、Index Time
            TestReceiveTimeInfoRecord[i][9].iSec = OEERecevieTimeInfo[i].iStartSec;    //KaiChen 20171127 (Steven) ：超豐 OEE 新增 Test Time、Index Time
            TestReceiveTimeInfoRecord[i][9].iMSec= OEERecevieTimeInfo[i].iStartMSec;   //KaiChen 20171127 (Steven) ：超豐 OEE 新增 Test Time、Index Time
        }
    }

    for(int i=0; i<10; i++)
    {
        str.sprintf("%02d:%6.3f", TestTimeInfoRecord[0][i].iStartMin,
                                  (double)TestTimeInfoRecord[0][i].iStartSec+(double)TestTimeInfoRecord[0][i].iStartMSec/1000.0);
        fObserver->TimeInfoGrid->Cells[1][2+i]=str;
        str.sprintf("%02d:%6.3f", TestTimeInfoRecord[0][i].iEndMin,
                                  (double)TestTimeInfoRecord[0][i].iEndSec+(double)TestTimeInfoRecord[0][i].iEndMSec/1000.0);
        fObserver->TimeInfoGrid->Cells[2][2+i]=str;
        if(TestTimeInfoRecord[0][i].iEndMin<TestTimeInfoRecord[0][i].iStartMin)
            iE=(60+TestTimeInfoRecord[0][i].iEndMin)*60;                        // to sec;
        else
            iE=TestTimeInfoRecord[0][i].iEndMin*60;                             // to sec;
        iE+=TestTimeInfoRecord[0][i].iEndSec;                                   // total sec
        iE =iE*1000+TestTimeInfoRecord[0][i].iEndMSec;
        iS =TestTimeInfoRecord[0][i].iStartMin*60;                              // to sec;
        iS+=TestTimeInfoRecord[0][i].iStartSec;                                 // total sec
        iS =iS*1000+TestTimeInfoRecord[0][i].iStartMSec;
        iT =iE-iS;                                                              // test time msec
        if(iT!=0)
        {
            fObserver->iTestTime+=iT;
            fObserver->iTestTimeCT++;
        }
        iTM=iT/60000;
        iTMSEC=iT-iTM*60000;

        RunInfo.dTestTimeSec=(double)iT/1000.0;                                 //2008/10/20 lee start
        if(iTM!=0)  str.sprintf("%02d:%6.3f", iTM, (double)iTMSEC/1000.0);
        else        str.sprintf("%6.3f", (double)iTMSEC/1000.0);                //2008/10/20 lee end

        if(CUSTOMER_CODE==CC_KYEC_CHEN)                                         //Eliot 2008_08_27
        {
            if(i==9)
                fObserver->iTotoalTestTime=iT;                                  //2007/10/17 Marc Start
        }

        if(i==9)                                                                //2013-11-27   Dell    記錄測試時間
            dTestSec=(double)iT/1000.0;                                         //jou 2014-09-24 int -> float bWhenHappenTestedTimeBelowUseInitialDelay 測試秒數小於1 sec會誤判

        fObserver->TimeInfoGrid->Cells[3][2+i]=ConvertMSecToSPC(iT);            //JerryYang 20200812 : Test time格式改為HH:MM:SS.000
        if(i>=1)
        {
            if(TestTimeInfoRecord[0][i].iStartMin<TestTimeInfoRecord[0][i-1].iEndMin)
                iE=(60+TestTimeInfoRecord[0][i].iStartMin)*60;                  // to sec;
            else
                iE=TestTimeInfoRecord[0][i].iStartMin*60;                       // to sec;
            iE+=TestTimeInfoRecord[0][i].iStartSec;                             // total sec
            iE =iE*1000+TestTimeInfoRecord[0][i].iStartMSec;
            iS =TestTimeInfoRecord[0][i-1].iEndMin*60;                          // to sec;
            iS+=TestTimeInfoRecord[0][i-1].iEndSec;                             // total sec
            iS =iS*1000+TestTimeInfoRecord[0][i-1].iEndMSec;
            iT =iE-iS;                                                          // test time msec
            fObserver->dTotoalIndexCycleTime=iT;                                //Isaac 20180301 (Steven) Index Cycle Time Monitoring function，記錄index cycle time
            if(iT!=0)
            {
                IndexTime+=iT;
                IndexTimeCT++;
            }
            iTM=iT/60000;
            iTMSEC=iT-iTM*60000;

            if(i==9)                                                            //kevin 20141024 高雄日月光IC履歷記錄
            {
               fRecindexCycleTim=(double)iTMSEC/1000.0;                         //取的index time
               RecindexCycleTim.sprintf("%6.3f", (double)fRecindexCycleTim);
            }

            if(bIndexTimeSet && i==9)                                           //kevin 20130812 K15假index time
            {
                str.sprintf("%6.3f", fIndexTime);
            }
            else
            {
                if(iTM!=0)                                                      //2008/10/20 lee start
                    str.sprintf("%02d:%6.3f", iTM, (double)iTMSEC/1000.0);
                else
                    str.sprintf("%6.3f", (double)iTMSEC/1000.0);
            }
            fObserver->TimeInfoGrid->Cells[4][2+i]=str;
        }

        fObserver->TimeInfoGrid->Cells[5][2+i]=FormatFloat("0.00", fObserver->fRecordIndexTime[9-i]);   //jou 2010-12-22 新增index time ave.
        fObserver->strngrdTestTime->Cells[5][2+i]=FormatFloat("0.000", fObserver->fRecordIndexTime[9-i]);

        if(CosFunction.bOEEFunction)                                            //Steven 20180417 (Jou) : OEE功能
        {                                                                       //KaiChen 20171127 (Steven) ：超豐 OEE 新增 Test Time、Index Time
            if(fObserver->iTestReceiveTimeCount<=20)                            //Sam 20200317 : Fix OEE Test Time
            {
                str.sprintf("%02d:%6.3f", TestReceiveTimeInfoRecord[0][i].iMin,
                                          (double)TestReceiveTimeInfoRecord[0][i].iSec+(double)TestReceiveTimeInfoRecord[0][i].iMSec/1000.0);
                fObserver->sg_ListTimeReceiveInfoGrid->Cells[1][2+i]=str;

                if(TestReceiveTimeInfoRecord[0][i].iMin<TestTimeInfoRecord[0][i].iStartMin)
                    iE=(60+TestReceiveTimeInfoRecord[0][i].iMin)*60;            // to sec;
                else
                    iE=TestReceiveTimeInfoRecord[0][i].iMin*60;                 // to sec;

                iE +=TestReceiveTimeInfoRecord[0][i].iSec;                      // total sec
                iE  =iE*1000+TestReceiveTimeInfoRecord[0][i].iMSec;
                iS  =TestTimeInfoRecord[0][i].iStartMin*60;                     // to sec;
                iS +=TestTimeInfoRecord[0][i].iStartSec;                        // total sec
                iS  =iS*1000+TestTimeInfoRecord[0][i].iStartMSec;
                iT  =iE - iS;                                                   // test time msec
                str.sprintf("%6.3f", (double)iT/1000.0);                        //Sam 20180104 : 時間異常 HangUp
                fObserver->sg_ListTimeReceiveInfoGrid->Cells[2][2+i] = str;
            }
        }
    }

    if(fObserver->iTestTimeCT!=0)                                               //2008/10/02 lee start
    {
        fObserver->iTestTime/=fObserver->iTestTimeCT;
        iTM=fObserver->iTestTime/60000;
        iTMSEC=fObserver->iTestTime-iTM*60000;
        if(iTM!=0)  str.sprintf("%02d:%6.3f", iTM, (double)iTMSEC/1000.0);
        else        str.sprintf("%6.3f", (double)iTMSEC/1000.0);
        fObserver->TimeInfoGrid->Cells[3][14]=str;
    }

    if(IndexTimeCT!=0)
    {
        IndexTime/=IndexTimeCT;
        iTM=IndexTime/60000;
        iTMSEC=IndexTime-iTM*60000;
        if(iTM!=0)  str.sprintf("%02d:%6.3f",iTM, (double)iTMSEC/1000.0);
        else        str.sprintf("%6.3f", (double)iTMSEC/1000.0);
        fObserver->TimeInfoGrid->Cells[4][14]=str;
    }                                                                           //2008/10/02 lee end

    fObserver->TimeInfoGrid->Cells[5][14]=FormatFloat("0.00", fObserver->fRecordIndexTime[11]);     //jou 2010-12-22 新增index time ave.
    fObserver->strngrdTestTime->Cells[5][14]=FormatFloat("0.000", fObserver->fRecordIndexTime[11]);

    RunInfo.TestTime      =fObserver->TimeInfoGrid->Cells[3][11];               //Steven 20141230 : 修正SECS GEM參數
    RunInfo.IndexCycleTime=fObserver->TimeInfoGrid->Cells[4][11];

    fObserver->RecordIndexCycle();                                              //Sam 20200916 : Add Index Cycle Time Record

    for(int i=0; i<TestSocket.iShtRow; i++)
    {
        for(int j=0; j<TestSocket.iShtCol; j++)
        {
            TestSocket.PordRec[i][j].AddTestTime(RunInfo.TestTime);
            TestSocket.PordRec[i][j].AddDataTimeRecord(sBufferSOT);                     //wei 20181211 (Steven) : 更換位置SOT
            TestSocket.PordRec[i][j].AddDataTimeEOTRecord(sBufferEOT);                  //wei 20181211 (Steven) : 更換位置SOT
            TestSocket.PordRec[i][j].AddGroundRecord(fGroundMan->asGroundVaule);        //Sam 20211223 : 每顆 IC 測試完畢都要記錄當時的 Ground & ESD 數值。
            #if 0 // GATE(W906-W2-3) G1: fLotInfo has no lbESDReportData (St01's facade; the ESD report source is not ported either) -- per-IC ESD value not recorded -- golden cObserver.cpp:2037
            TestSocket.PordRec[i][j].AddESDRecord(fLotInfo->lbESDReportData->Caption);  //Sam 20211223 : 每顆 IC 測試完畢都要記錄當時的 Ground & ESD 數值。
            #endif
        }
    }

    if(CosFunction.bOEEFunction)                                                //Steven 20180417 (Jou) : OEE功能
    {                                                                           //KaiChen 20171127 (Steven) ：超豐 OEE 新增 Test Time、Index Time
        if(fObserver->iTestReceiveTimeCount==20)                                //Sam 20200317 : Fix OEE Test Time
        {
            for(int k=0; k<10; k++)
            {
                fObserver->dRecordTestTime[k]=(double)StrToFloat(fObserver->sg_ListTimeReceiveInfoGrid->Cells[2][2+k]);
            }

            for(int x=0; x<10; x++)
            {
                for(int y=x; y<10; y++)
                {
                    if(fObserver->dRecordTestTime[y]>fObserver->dRecordTestTime[x])
                    {
                        temp=fObserver->dRecordTestTime[y];
                        fObserver->dRecordTestTime[y]=fObserver->dRecordTestTime[x];
                        fObserver->dRecordTestTime[x]=temp;
                    }
                }
            }
            double dTestReceive=0.0;
            for(int k=1; k<6; k++)                                              //Sam 20200317 : Fix OEE Test Time
            {
                dTestReceive+=fObserver->dRecordTestTime[k];
            }
            fObserver->sTestReceiveTime=FloatToStr(dTestReceive/5.0);
        }
        else if(fObserver->iTestReceiveTimeCount<20)                            //Sam 20200317 : Fix OEE Test Time
        {
            fObserver->sTestReceiveTime="";
        }
        fObserver->sg_ListTimeReceiveInfoGrid->Cells[2][14]=fObserver->sTestReceiveTime;
    }

    int iCount=0, iCount2=0;                                                    //Steven 20200715 : 重新計算Cycle Time
    int iTestTime=0;
    int iCycleTime=0;

    for(int j=0; j<10; j++)
    {
        if(j<=QueueTestTime.iCount-1)
        {
            fObserver->strngrdTestTime->Cells[1][11-j]=QueueTestTime.GetStartTime(j);
            fObserver->strngrdTestTime->Cells[2][11-j]=QueueTestTime.GetEndTime(j);
            fObserver->strngrdTestTime->Cells[3][11-j]=QueueTestTime.GetTimeString(j);
            iCount++;
            iTestTime+=QueueTestTime.GetTimeData(j);
        }

        if(j<=QueueCycleTime.iCount)
        {
            fObserver->strngrdTestTime->Cells[4][11-j]=QueueCycleTime.GetTimeString(j);
            iCycleTime+=QueueCycleTime.GetTimeData(j);
            iCount2++;
        }
    }

    if(iCount>0)
    {
        fObserver->strngrdTestTime->Cells[3][14]=ConvertMSecToSPC(iTestTime/iCount);
    }

    if(iCount2>0)
    {
        fObserver->strngrdTestTime->Cells[4][14]=ConvertMSecToSPC(iCycleTime/iCount2);
    }

    if(IniConfig.bShowTimeInfo==true)                                           //Steven 20100827
    {
        if((TestIF_File.iShuttleMode==1 &&                                      //kevin 20190130 add use test arm
            TestIF_File.iShuttle_Sel==iIndexArmWhich) ||
           TestIF_File.iShuttleMode==0)
        {
            if(CUSTOMER_CODE==CC_SCC || CUSTOMER_CODE==CC_JCET)
            {
                #if 0 // GATE(W906-W2-3) G2 web-display: fShowMessage has no lblIndexCycleTime / lblTestTime (the page shows the TimeInfoGrid numbers) -- golden cObserver.cpp:2117-2118
                fShowMessage->lblIndexCycleTime->Caption="   Index Cycle Time="+fObserver->strngrdTestTime->Cells[4][11]+" sec";
                fShowMessage->lblTestTime->Caption      ="   Test Time ="+fObserver->strngrdTestTime->Cells[3][11]+" sec";
                #endif
            }
            else
            {
                #if 0 // GATE(W906-W2-3) G2 web-display: fShowMessage has no lblIndexCycleTime / lblTestTime (the page shows the TimeInfoGrid numbers) -- golden cObserver.cpp:2122-2123
                fShowMessage->lblIndexCycleTime->Caption="   Index Cycle Time="+fObserver->TimeInfoGrid->Cells[4][11]+" sec";
                fShowMessage->lblTestTime->Caption      ="   Test Time ="+fObserver->TimeInfoGrid->Cells[3][11]+" sec";
                #endif
            }
        }
    }
    else
    {
        #if 0 // GATE(W906-W2-3) G2 web-display: fShowMessage has no lblIndexCycleTime / lblTestTime (the page shows the TimeInfoGrid numbers) -- golden cObserver.cpp:2129-2130
        fShowMessage->lblIndexCycleTime->Caption="";
        fShowMessage->lblTestTime->Caption      ="";
        #endif
    }

//    ShowIndexTime(1);                                                         //Steven 20140619 : 測試    //到這裡大概0.015~0.031Sec
}
