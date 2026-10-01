// ===========================================================================
//  MainTimer8.cpp -- S-15 (St02): golden TfMain::Timer8Timer, TfMain::RecordJamRateByTime and
//  TfMain::TimerTemperatureStorageMinuteTimer.  AI(W906-S15) 20261001 (St02-E).
//
//  golden 906_0625_Steven main.cpp:32106-32150 (RecordJamRateByTime), :32152 (LoaderCheckFullDelay),
//  :32154-32246 (Timer8Timer), :31175-31180 (TimerTemperatureStorageMinuteTimer).  Both TTimers have no stored
//  Interval / Enabled in main.dfm (:17346 Timer8, :17318 TimerTemperatureStorageMinute) => VCL 1000 ms, enabled at
//  design time, never changed.  MainTimersSt02.cpp calls them once per 1000 ms from WebBridgeTags.cpp PumpTick.
//
//  Gated in place (#if 0, golden text kept), each with its WHY:
//    T8-1  bNeedClearFile -> fLotInfo->ClearAllSetupFile(PPID): bNeedClearFile / PPID are not TfMain members; the
//          port's SECS handlers clear the setup files at once instead (SECSGEM/uHGemHT9045.cpp:1880 / :6108).
//    T8-2  VTEST tester alarm MES0732: customer-only (IniConfig.bVTESTFunction is set for VTEST only,
//          CosFunction.cpp:2476) = S25, RULINGS_20260925 (St02-M 20261001).  One line to open if asked.
//    T8-3  bReadEpTime -> fMain->ReadEPData(): S25 (ASE Kaohsiung) and TfMain::ReadEPData / ADAM / the ASE socket
//          are not ported.
//    T8-5  [A19] PM alarm date label: the PM alarm form (fPMAlarmInterFace) and lb_PMAlarmDate are not ported;
//          display only.
//  ONE DEVIATION (St02-M 20261001, MES0921): golden ShowErrorMessage is fNote->ShowModal() even for kcode 0, so
//    Timer8 stood still while the [P29] "loader full" notice was up.  In wb_serve kcode 0 is a non-waiting notice
//    (wb_serve.cpp:496-510), so every second would call it again and log "Alarm at same time" (fNote_ShowError.cpp
//    :372-376).  Timer8 therefore returns at once while its own MES0921 notice is still showing -- golden's effect.
//  Modal waits (AI(W906-S13) 20261001): the waiting boxes' tick (tools/wb_serve.cpp:7621) runs MainTimersSt02.cpp too,
//    so both timers keep firing during another box's ShowModal, as golden's TTimers did (was a known gap in S-15).
// ===========================================================================
#include "MachineType.h"
#include "cmydef.h"           // InitialOK, bReadEpTime, iRecordJamRateByTime_*, bRecordJamRateByTime_Clear
#include "Config.h"           // IniConfig
#include "cprod.h"            // TestIF_File
#include "mysensor.h"         // Sen[]
#include "canary_support.h"   // ShowErrorMessage / RecordProcess (not with cMyDB.h in one TU, techniques §5)
#include "csystem.h"          // TemperatureStorageLog, W906_FormShowing
#include "MainCalcCore.h"     // ComputeJamRateRecordStrings (golden :32125-32145, quirks kept)
#include "forms/fMain.h"      // fMain->pnlCleanCount
#include "forms/fNote.h"      // fNote->fShow / edErrorCode (the MES0921 deviation)
#include "myTimer.h"          // TQPF_Timer

#include <ctime>

namespace ht9045 {

namespace {
clock_t (*g_clock)() = &std::clock;   // ctest seam only (W906_Timer8SetClock); golden clock()
TQPF_Timer LoaderCheckFullDelay;      // golden main.cpp:32152 (file scope)
bool g_p29Shown = false;              // this timer raised MES0921 and its notice may still be up (the deviation above)

// golden TfMain::RecordJamRateByTime, main.cpp:32106-32150
void RecordJamRateByTime()
{
    static clock_t ctStart=g_clock();
    if(IniConfig.bRecordJamRateByTime==false ||
       IniConfig.iRecordJamRateIntervalTime<=0)
    {
        return;
    }

    if(bRecordJamRateByTime_Clear==true)
    {
        ctStart=g_clock();
        bRecordJamRateByTime_Clear=false;
    }

    clock_t ctEnd=g_clock();
    if(ctEnd-ctStart>=IniConfig.iRecordJamRateIntervalTime*1000*60)
    {
        ctStart=g_clock();
        AnsiString sMTBFRecord="";
        AnsiString sJamRateRecord   = "";
        ComputeJamRateRecordStrings(iRecordJamRateByTime_JamCount, iRecordJamRateByTime_LoaderCount,
                                    IniConfig.iRecordJamRateIntervalTime, sMTBFRecord, sJamRateRecord);   // golden :32125-32145
        RecordProcess( sJamRateRecord );
        iRecordJamRateByTime_LoaderCount    = 0;
        iRecordJamRateByTime_JamCount       = 0;
    }
}

bool P29NoticeStillUp()
{
    return fNote != 0 && W906_FormShowing("fNote", fNote->fShow) &&
           fNote->edErrorCode != 0 && fNote->edErrorCode->Text == AnsiString("MES0921");
}
}  // namespace

void W906_Timer8SetClock(clock_t (*fn)()) { g_clock = fn ? fn : &std::clock; }   // ctest only

// golden TfMain::Timer8Timer, main.cpp:32154-32246
void W906_Timer8Timer()
{
    static bool bTimerRunning=false;
    static bool bTimerOn=false;
    static unsigned int iTesterAlarmCount=0;
    if(InitialOK==false || bTimerRunning==true)
        return;

    if(g_p29Shown)                                                              // AI(W906-S15): the MES0921 deviation (header)
    {
        if(P29NoticeStillUp())
            return;
        g_p29Shown=false;
    }

    bTimerRunning=true;

#if 0   // GATE T8-1 (AI(W906-S15) 20261001): bNeedClearFile / PPID are not TfMain members; the port's SECS handlers
        //   clear the setup files at once (SECSGEM/uHGemHT9045.cpp:1880 / :6108).  golden :32164-32177 VERBATIM:
    if(bNeedClearFile)
    {
        try
        {
            if(CosFunction.bKeepOnly1SetupFile)
                fLotInfo->ClearAllSetupFile(PPID);                              //Steven 20210917 : Add for 下載完工作檔後, 只留一個就好
        }
        catch(...)
        {
        };

        bNeedClearFile=false;
        return;
    }
#endif

#if 0   // GATE T8-2 (AI(W906-S15) 20261001): S25 -- IniConfig.bVTESTFunction is VTEST-only (CosFunction.cpp:2476),
        //   RULINGS_20260925 S25, St02-M 20261001.  golden :32179-32187 VERBATIM (one line to open if asked):
    if(IniConfig.bVTESTFunction==true)
    {
        iTesterAlarmCount++;
        if(Sen[SnTesterAlarm].IsOn() && iTesterAlarmCount>=10)
        {
            ShowErrorMessage("MES0732", K_RETRY, MMInterface);                  //jou 20240103 : LB Undocking，需QA確認補償檔，增加權限設定
            iTesterAlarmCount=0;
        }
    }
#else
    (void)iTesterAlarmCount;
#endif

#if 0   // GATE T8-3 (AI(W906-S15) 20261001): S25 (ASE Kaohsiung sets bReadEpTime, aTester_Front.cpp:9353-9356) and
        //   TfMain::ReadEPData / ADAM_ReadPA / the ASE socket are not ported.  golden :32189-32193 VERBATIM:
    if(bReadEpTime)                                                             //kevin 20170524 (wei) add ep read change time
    {
        fMain->ReadEPData();
        bReadEpTime=false;
    }
#endif

    if(IniConfig.bP29LoaderCheckIsFull)                                         //Steven 20160818 : CheckLoader滿盤
    {
        if(bTimerOn==false)
        {
            LoaderCheckFullDelay.SetSecAndOn(IniConfig.dP29LoaderCheckIsFullInterval);
            bTimerOn=true;
        }

        if(bTimerOn && LoaderCheckFullDelay.Off())
        {
            if(Sen[SnLoaderIsFull].Enable && Sen[SnLoaderIsFull].IsOn())
            {
                ShowErrorMessage("MES0921", 0, MMTrayZ, false, "Main--Timer8");
                g_p29Shown=true;                                                // AI(W906-S15): the MES0921 deviation (header)
            }
            else
            {
                bTimerOn=false;
            }
        }
    }

#if 0   // GATE T8-5 (AI(W906-S15) 20261001): the PM alarm form (fPMAlarmInterFace) and lb_PMAlarmDate are not ported;
        //   display only.  golden :32216-32233 VERBATIM:
    if(IniConfig.bA19UsePMAlarmFunction==true)                                  // 2015.05.26 , Mylin , PM Alarm {      //wei 20160225 PMAlarmFunction
    {
        AnsiString sDate=fPMAlarmInterFace->GetWhenAlarmShowPMDate();
        if(sDate!="")
        {
            sDate="PM Alarm Date " + sDate;
            lb_PMAlarmDate->Caption=sDate;
            lb_PMAlarmDate->Visible=true;
        }
        else
        {
            lb_PMAlarmDate->Visible=false;
        }
    }
    else
    {
        lb_PMAlarmDate->Visible=false;
    }
#endif

    if(fMain != 0 && fMain->pnlCleanCount != 0)                                // AI(W906-S15): the facade object, built in the TfMain ctor
    {
        if(IniConfig.bEnableAutoCleanFunction && TestIF_File.iAutoClean_Function)   //Steven 20240731 : add for auto clean count
        {
            fMain->pnlCleanCount->Visible=true;
        }
        else
        {
            fMain->pnlCleanCount->Visible=false;
        }
    }

    RecordJamRateByTime();                                                      // 2015.11.11 , Joye , Add Jam Rate Record
    bTimerRunning=false;
}

// golden TfMain::TimerTemperatureStorageMinuteTimer, main.cpp:31175-31180
void W906_TimerTemperatureStorageMinuteTimer()
{
    if(InitialOK==false) return;                                                //AI(ht9045-timer-fix) 20260511 (RogerYang) : Access Violation protect
    if(IniConfig.bL10IndexTestlogTemp==false)                                   //kevin 20190323 : index 測試時才記錄溫度
        TemperatureStorageLog(0);                                               //kevin 20190323 add Steven 20140617 : for 海思
}

}  // namespace ht9045
