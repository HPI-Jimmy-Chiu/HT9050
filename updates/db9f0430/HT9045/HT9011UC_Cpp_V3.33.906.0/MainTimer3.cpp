// ===========================================================================
//  MainTimer3.cpp -- S-14 (St02): golden TfMain::Timer3Timer.  AI(W906-S14) 20261001 (St02-E).
//
//  golden 906_0625_Steven main.cpp:25184-25820 (+ the file-scope TQPF_Timer Fix3FullPlaceDelay :25183 and
//  TfMain::ShowVacuumOnOffTime :24668-24719).  main.dfm:17301 Timer3: Enabled = False, no Interval stored => VCL 1000 ms;
//  FormShow :10180 `Timer3->Enabled=true;` (InitialOK=true follows at :10464 in the same FormShow); FormClose :11705 and
//  the two self-close paths :25675 / :25686 disable it.
//  ENABLE -> PORT.  MainTimersSt02.cpp latches the Timer3 slot on the first dispatcher pass that sees InitialOK true
//    (WebBridgeTags.cpp:563 PumpInit = the port's golden FormShow :10464); the first call then comes one Interval later,
//    as a VCL timer enabled in FormShow.  The handler keeps golden's own `InitialOK==false` return (:25194).  After the
//    port's FormClose (bSystemClose, FileRW/MainClose.cpp) the dispatcher fires nothing.
//  Design: St02-E2 ST02_S14_CLAIMS_20261001 sections G0-G27 (that draft's numbering; golden lines below are 0625):
//    G0  :25186-25195  statics, InitialOK guard                                  translated
//    G1  :25197-25213  handler stop time label                                   GATED (S25 + display only)
//    G2  :25215-25268  2 s after start: FTP record, MES2108 Program Start, CCD   translated; UpdateLanguage / fShowMessage GATED
//    G3  :25271-25291  Fix3 full-place sensor not enabled -> message, 10 s       translated (golden: SHIP only)
//    G4  :25293-25294  ShowVacuumOnOffTime (main grid) ; RecordTemp               translated (RecordTemp: R126 M5, below)
//    G5  :25296-25307  CCD temperature > 55 -> MNetLog                            translated (MNetLog: Motor/myMN200motor.cpp:2516)
//    G6  :25309-25313  bNoNeedLoadSteupFile -> cbSetupFileName                    NOT HERE: WebRecipeChange.cpp:680-684 does it
//    G7  :25315-25319  bin display panels (NUMBER_PANEL_TYPE != 2 / 3 / 4)        translated (calls cShowBinSelect.cpp)
//    G8  :25321-25346  [A01] reset while an engineer page is open                 St01 FileRW/Main_A01AutoLogout.cpp (1)
//    G9  :25348-25363  KYEC keypad auto close                                     GATED (S25)
//    G10 :25365-25384  [E73] Z step motor loses steps -> message per minute       translated
//    G11 :25386-25443  [A01] auto switch to Operator                              St01 FileRW/Main_A01AutoLogout.cpp (2)
//    G12 :25444-25460  [A01] auto re-open after 30 min                            St01 FileRW/Main_A01AutoLogout.cpp (3)
//    G13 :25478-25531  [A37] EAP2S / SECS host lot end messages                   translated
//    G14 :25533-25561  KYEC barcode re-enter timer                                KYEC branch GATED (S25), golden's else kept
//    G15 :25563-25596  TTL Clear lines (IO)                                       translated; TTLLog GATED
//    G16 :25598-25605  the 8 yield monitor alarms                                 translated (calls uYieldMonitoring.cpp)
//    G17 :25607-25616  UNISEM jam rate refresh                                    GATED (S25)
//    G18 :25618-25634  hourly TimerRecordLoaderDate (JCET every 10 min)           translated; JCET branch GATED (S25)
//    G19 :25636-25650  KYEC FTP connect once                                      GATED (S25)
//    G20 :25652-25663  RMS: lock the Lot page recipe fields while running         translated
//    G21 :25665-25678  RTC: program auto close                                    GATED (dependency missing)
//    G22 :25680-25688  ASE_CL: program auto close                                 GATED (S25)
//    G23 :25690-25704  tray / device direction images                             GATED (dependency missing, display)
//    G24 :25706-25718  ASE JOBFILE messages                                       GATED (S25, flags not ported)
//    G25 :25720-25728  soak / ATC wait countdowns                                 NOT IN THIS BATCH (Ifor01 I-01), text kept
//    G26 :25730-25742  recipe change by GPIB failed -> WAR1684 / WAR16500         translated
//    G27 :25744-25776  Renesas FT-CT alarm                                        GATED (S25)
//    --  :25778-25819  commented out by golden itself                             not done
//  S25 GATES WITH AN ELSE (G14, G18).  The customer `if` head and body are gated, golden's generic else stays live: every
//    other customer runs exactly golden; the customer's machine runs the generic branch (noted at each gate).
//  OTHER TfMain MEMBERS.  Fix3FullPlaceDelay (main.cpp:25183, file scope) and ShowVacuumOnOffTime (main.h member) live
//    here; TimerRecordLoaderDate() = W906_MainClose_TimerRecordLoaderDate (FileRW/MainClose.cpp:2895 -> :500 in that
//    file's anonymous namespace, St01; other programs: MainTimerESDFallback.cpp, returns 0); StatusBar1 -> see G18.
//    RecordTemp (golden main.cpp:2658-2689, only caller G4) is file-local below; it writes fObserver->dTempHistroy
//    (forms/fObserver.h:790; the object is cObserver.cpp:3289, same archive) and calls fObserver->UpdateTempChart.
//  No waiting and no thread here: ShowMyMessage / ShowErrorMessage are the host's golden ShowModal waits (wb_serve
//    W906MbShowMyMessage / ForwardShowErrorMessage).  While such a box waits, the dispatcher keeps calling Timer3 on its
//    deadline, as golden VCL re-enters Timer3Timer (no re-entry flag, :25184-25195); a nested ShowMyMessage returns at once
//    (golden mymessbox.cpp:785, port tools/wb_serve.cpp:6859).  AI(W906-R126) 20261002 (St02-E): NB2 R126 M4 -- until then
//    the dispatcher skipped Timer3 inside its own box, so the hourly LoaderData record, the yield alarms and the TTL lines
//    stopped while it waited.
// ===========================================================================
#include "MachineType.h"          // SOFT_SIMULTE -- before the #ifndef below; CC_*, rsm*, Fix3K_ShortShuttle, ChangeToFloatNonPcnt
#include "cmydef.h"               // InitialOK, SystemStart, AccessLevel, NUMBER_PANEL_TYPE, FIX3_FULL_PLACE, iInZHomeCnt, SwClear*, ...
#include "Config.h"               // IniConfig
#include "CosFunction.h"          // CosFunction.bFTPFunction / bDownloadRecipeLevelMode
#include "cprod.h"                // TestIF_File, TrayForm
#include "LastSet.h"              // LastSet
#include "mysensor.h"             // Sen[]
#include "myswitch.h"             // SW[]
#include "myTimer.h"              // TQPF_Timer
#include "aHotPlateSubstrate.h"   // InArmSuck / FTestSuck / BTestSuck / OutArmSuck / CatchTraySuck; not mykitsuck.h (KNOWLEDGE.md, the two TMyKitSuck)
#include "canary_support.h"       // ShowMyMessage / ShowErrorMessage / RecordProcess (not with cMyDB.h in one TU, techniques §5)
#include "common.h"               // SVNRevision
#include "csystem.h"              // hLotStartTimeOut / hLotEndTimeOut, HasICUnderMachine, W906_FormShowing
#include "atester_shims.h"        // CCDInterfaceForm (TCCDInterfaceFormShim)
#include "forms/fMain.h"          // fMain->StringGrid2 / EnabledSetupFile
#include "forms/fNote.h"          // fNote->fShow
#include "forms/fLotInfo.h"       // fLotInfo->edDeviceName / cbbDeviceName / edTemp / coLevelMode
#include "forms/fSCKART.h"        // fSCKART->iCurrent93KARTStep
#include "forms/fShowBinSelect.h" // fShowBinSelect->DoShowBinDigital / ChangeBinDispStatus (cShowBinSelect.cpp, called only)
#include "BinDisplay/BinDispBringUp_St02.h"   // W906_BinDispShownScope_St02 (ST02-C14 part 3, AI(W906-ST02-C14) 20261002)
#include "forms/fYieldMonitoring.h"   // fYieldMonitoring->Check*YieldAlarm (uYieldMonitoring.cpp, called only)
#include "forms/fObserver.h"      // fObserver->dTempHistroy / bShow / UpdateTempChart (RecordTemp; cObserver.cpp)   AI(W906-R126) 20261002

void NewRecordProcess(AnsiString AlarmCode, AnsiString S, AnsiString Debug);   // cMyDB.h:129 (Debug default " "; body cMyDB.cpp:1868)
int  W906_MainClose_TimerRecordLoaderDate();     // FileRW/MainClose.cpp:2895 -> :500 (anonymous namespace) -- golden TfMain::TimerRecordLoaderDate
bool MNetLog(AnsiString Message);                // Motor/myMN200motor.h:199 (body Motor/myMN200motor.cpp:2516; its golden body is gated there (g), returns true)

namespace ht9045 {

namespace {
TQPF_Timer Fix3FullPlaceDelay;                                                  //Steven 20130126 : Fix3滿盤功能   (golden main.cpp:25183, file scope; only Timer3 uses it)
const int Timer3_Interval=1000;                // golden Timer3->Interval: main.dfm:17301 stores none => VCL 1000 ms (MainTimersSt02.cpp uses the same)

// G18: golden reads StatusBar1->Panels->Items[6]->Text = sAlarmTime, "%04d-%02d-%02d %02d:%02d:%02d" (TfMain::ProcessTimeUpdate
//   main.cpp:7806-7818, run from golden Timer1 :3189 -- also inside ShowModal -- so the text is never more than a fraction of a
//   second old).  The port has no status bar: the same text is built from the clock now.  Not from SystemMin / SystemSec: only
//   PumpTick's first line refreshes them, and PumpTick does not run while a waiting box loops -- frozen on hh:00:00 they would
//   call TimerRecordLoaderDate every second for as long as the box is up.
AnsiString StatusBarTimeNow()
{
    Word wYear=0, wMonth=0, wDate=0, wHour=0, wMin=0, wSec=0, wMSec=0;
    const TDateTime dtNow=Now();
    DecodeDate(dtNow, wYear, wMonth, wDate);
    DecodeTime(dtNow, wHour, wMin, wSec, wMSec);
    AnsiString s;
    s.sprintf("%04d-%02d-%02d %02d:%02d:%02d", wYear, wMonth, wDate, wHour, wMin, wSec);   // golden main.cpp:7814
    return s;
}

int        (*g_recordLoaderDate)() = &W906_MainClose_TimerRecordLoaderDate;            // ctest seam only (W906_Timer3SetRecordLoaderDate)
AnsiString (*g_statusBarTime)() = &StatusBarTimeNow;                                   // ctest seam only (W906_Timer3SetStatusBarTime)
void       (*g_newRecordProcess)(AnsiString, AnsiString, AnsiString) = &NewRecordProcess;   // ctest seam only (W906_Timer3SetNewRecordProcess)

// golden TfMain::ShowVacuumOnOffTime, main.cpp:24668-24719 (main grid StringGrid2, display only)
void ShowVacuumOnOffTime()
{
    if(fMain==0 || fMain->StringGrid2==0)                                       // [W906] the facade object, built in the TfMain ctor
        return;
    TStringGrid *StringGrid2=fMain->StringGrid2;                                // [W906] golden TfMain member
    for(int i=0; i<2; i++)
    {
        StringGrid2->Cells[3+i*3][1]=InArmSuck.Suck[0+i][0].VacuumOnTime;
        StringGrid2->Cells[3+i*3][2]=InArmSuck.Suck[0+i][1].VacuumOnTime;
        StringGrid2->Cells[3+i*3][3]=InArmSuck.Suck[0+i][2].VacuumOnTime;
        StringGrid2->Cells[3+i*3][4]=InArmSuck.Suck[0+i][3].VacuumOnTime;

        StringGrid2->Cells[3+i*3][6]=FTestSuck.Suck[0+i][0].VacuumOnTime;
        StringGrid2->Cells[3+i*3][7]=FTestSuck.Suck[0+i][1].VacuumOnTime;
        StringGrid2->Cells[3+i*3][8]=FTestSuck.Suck[0+i][2].VacuumOnTime;
        StringGrid2->Cells[3+i*3][9]=FTestSuck.Suck[0+i][3].VacuumOnTime;

        StringGrid2->Cells[3+i*3][11]=BTestSuck.Suck[0+i][0].VacuumOnTime;
        StringGrid2->Cells[3+i*3][12]=BTestSuck.Suck[0+i][1].VacuumOnTime;
        StringGrid2->Cells[3+i*3][13]=BTestSuck.Suck[0+i][2].VacuumOnTime;
        StringGrid2->Cells[3+i*3][14]=BTestSuck.Suck[0+i][3].VacuumOnTime;

        StringGrid2->Cells[3+i*3][16]=OutArmSuck.Suck[0+i][0].VacuumOnTime;
        StringGrid2->Cells[3+i*3][17]=OutArmSuck.Suck[0+i][1].VacuumOnTime;
        StringGrid2->Cells[3+i*3][18]=OutArmSuck.Suck[0+i][2].VacuumOnTime;
        StringGrid2->Cells[3+i*3][19]=OutArmSuck.Suck[0+i][3].VacuumOnTime;

        StringGrid2->Cells[4+i*3][1]=InArmSuck.Suck[0+i][0].VacuumOffTime;
        StringGrid2->Cells[4+i*3][2]=InArmSuck.Suck[0+i][1].VacuumOffTime;
        StringGrid2->Cells[4+i*3][3]=InArmSuck.Suck[0+i][2].VacuumOffTime;
        StringGrid2->Cells[4+i*3][4]=InArmSuck.Suck[0+i][3].VacuumOffTime;

        StringGrid2->Cells[4+i*3][6]=FTestSuck.Suck[0+i][0].VacuumOffTime;
        StringGrid2->Cells[4+i*3][7]=FTestSuck.Suck[0+i][1].VacuumOffTime;
        StringGrid2->Cells[4+i*3][8]=FTestSuck.Suck[0+i][2].VacuumOffTime;
        StringGrid2->Cells[4+i*3][9]=FTestSuck.Suck[0+i][3].VacuumOffTime;

        StringGrid2->Cells[4+i*3][11]=BTestSuck.Suck[0+i][0].VacuumOffTime;
        StringGrid2->Cells[4+i*3][12]=BTestSuck.Suck[0+i][1].VacuumOffTime;
        StringGrid2->Cells[4+i*3][13]=BTestSuck.Suck[0+i][2].VacuumOffTime;
        StringGrid2->Cells[4+i*3][14]=BTestSuck.Suck[0+i][3].VacuumOffTime;

        StringGrid2->Cells[4+i*3][16]=OutArmSuck.Suck[0+i][0].VacuumOffTime;
        StringGrid2->Cells[4+i*3][17]=OutArmSuck.Suck[0+i][1].VacuumOffTime;
        StringGrid2->Cells[4+i*3][18]=OutArmSuck.Suck[0+i][2].VacuumOffTime;
        StringGrid2->Cells[4+i*3][19]=OutArmSuck.Suck[0+i][3].VacuumOffTime;
    }
    StringGrid2->Cells[3][21]=CatchTraySuck.Suck[0][0].VacuumOnTime;
    StringGrid2->Cells[4][21]=CatchTraySuck.Suck[0][0].VacuumOffTime;

    StringGrid2->Cells[3][24]=LastSet.iLoaderTraySimulateTime;
    StringGrid2->Cells[3][25]=LastSet.iUnLoaderTraySimulateTime[0];
    StringGrid2->Cells[3][26]=LastSet.iUnLoaderTraySimulateTime[1];
    StringGrid2->Cells[3][27]=LastSet.iUnLoaderTraySimulateTime[2];
}

// golden TfMain::RecordTemp, main.cpp:2658-2689 (906_0625_Steven; only caller Timer3 G4 :25294).  AI(W906-R126) 20261002
//   (St02-E): NB2 R126 M5 -- GATE G4 said the observer facade had no dTempHistroy, but forms/fObserver.h:790 carries it (and
//   UpdateTempChart :823) and nothing else writes it, so every machine's Observer temperature history stayed all zeros.
//   Not a customer branch (S25 does not apply).  SystemSec is refreshed by PumpTick, which does not run while a waiting box
//   loops: inside a box this records nothing (golden Timer1 keeps SystemSec current there) -- noted, not changed.
void RecordTemp()
{
    if(fObserver==0)                                                            // [W906] the facade object (cObserver.cpp:3289, static init)
        return;
    static int iOldMin=-1;
    int i, j;
    int comp;
    if(     IniConfig.iL10TempRecordInterval==0)  comp=5;                       //30 sec
    else if(IniConfig.iL10TempRecordInterval==2)  comp=30;                      //30 sec
    else if(IniConfig.iL10TempRecordInterval==3)  comp=60;                      //1 min
    else if(IniConfig.iL10TempRecordInterval==4)  comp=300;                     //5 min
    else if(IniConfig.iL10TempRecordInterval==5)  comp=600;                     //10 min
    else                                          comp=15;                      //15 sec

    if(iOldMin!=SystemSec)
    {
        iOldMin=SystemSec;
        if((iOldMin%comp)!=0)
            return;

        for(i=0; i<tcTotalCount; i++)
        {
            if(bUT150Install[i])
            {
                for(j=0; j<59; j++)                                             //Steven 20140923 : Index使用EJ1N版32組加熱器
                    fObserver->dTempHistroy[i][j]=fObserver->dTempHistroy[i][j+1];
                fObserver->dTempHistroy[i][59]=UN150Read[i];
            }
        }

        if(W906_FormShowing("fObserver", fObserver->bShow))                    // [W906] golden fObserver->bShow (the port's page table, FShow_Audit)
            fObserver->UpdateTempChart();
    }
}
}  // namespace

void W906_Timer3SetRecordLoaderDate(int (*fn)()) { g_recordLoaderDate = fn ? fn : &W906_MainClose_TimerRecordLoaderDate; }   // ctest only
void W906_Timer3SetStatusBarTime(AnsiString (*fn)()) { g_statusBarTime = fn ? fn : &StatusBarTimeNow; }                       // ctest only
void W906_Timer3SetNewRecordProcess(void (*fn)(AnsiString, AnsiString, AnsiString)) { g_newRecordProcess = fn ? fn : &NewRecordProcess; }   // ctest only
AnsiString W906_Timer3StatusBarTimeNow() { return StatusBarTimeNow(); }                                                     // ctest only (the G18 text)

// golden TfMain::Timer3Timer, main.cpp:25184-25820
void W906_Timer3Timer()
{
    static bool bOnceFlag=true, bOnceLangFlag=true, bShowMsg=false;
    static int bFix3FullPlaceFlag=1;                                            //Steven 20130126 : Fix3滿盤功能
    static double dCCDTempOld=0.0;
    static int iCloseQwertyCount=0;                                             //Ifor 20170803 (wei) add 小鍵盤開啟時間計數
    static int iZhomeTime=0;                                                    //JerryYang 20240111 : 偵測頻繁z頻繁失步需alarm的功能
//    int iLevel;
    int iTime=0;

    if(InitialOK==false)                                                        //Steven 20110809
        return;

#if 0   // GATE G1 (AI(W906-S14) 20261001): S25 -- CosFunction.bShowHandlerStopTime is set for three customers only
        //   (CosFunction.cpp:1142 FUNC_CC_JSCC_OS / :1251 FUNC_CC_SCC / :2531 VTEST_Funtion; InitialCosFunction :4307 false) and it is
        //   display only (labStopTime / MyMessageBox->labStopTime / fNote->pnlStopTime are not in the port).  golden :25197-25213 VERBATIM:
    if(CosFunction.bShowHandlerStopTime==true)                                  //jou 2014-09-21 Show Handler Stop Time
    {
        iTime=lHandlerStopTime.LatchCycleTime();
        if(iTime>0)
        {
            if(MyMessageBox->fShow)
                MyMessageBox->labStopTime->Caption="Timer : "+ConvertMSecToTime(iTime);
            else if(fNote->fShow)
                fNote->pnlStopTime->Caption=" Timer : "+ConvertMSecToTime(iTime);
            else
                labStopTime->Caption="Timer : "+ConvertMSecToTime(iTime);
        }
        else
        {
            labStopTime->Caption="Timer : ";
        }
    }
#else
    (void)iTime;
#endif

    static unsigned int iOnceLangFlagCT=0;
    if(bOnceLangFlag==true)                                                     //jou 2011-08-02 start : UpdateLanguage放在form show會導致不同人登入電腦,開啟程式會破壞記憶體
    {
        iOnceLangFlagCT++;
        if(InitialOK==true && (iOnceLangFlagCT>=(unsigned int)ChangeToFloatNonPcnt((double)(2000), (double)(Timer3_Interval))))   // [W906] golden Timer3->Interval
        {
#if 0   // GATE G2a (AI(W906-S14) 20261001): TfMain::UpdateLanguage is not in the port (the web pages carry the language).  golden :25221 VERBATIM:
            UpdateLanguage();
#endif

            if(CosFunction.bFTPFunction && IniConfig.bEnableFTP)
            {
                AnsiString str;
                str.sprintf("%s is connect with %s", IniConfig.SocketHandlerID, IniConfig.TasterType+"-"+IniConfig.TasterName);
                RecordProcess(str);                                             //Steven 20091004

                if(AccessLevel>=5)
                    fMain->EnabledSetupFile(true);
                else
                    fMain->EnabledSetupFile(false);
            }

            // [W906] the four golden NewRecordProcess calls below go through g_newRecordProcess (= NewRecordProcess; ctest seam)
            #ifdef ASE_KaohSiung
            {
                g_newRecordProcess("MES2108", "Program Start", asHandlerVersion);                                         //Ifor 20161109 Handler Version Modify
            }
            #else
            {
                #ifdef DEBUG
                    g_newRecordProcess("MES2108", "Program Start", "Debug Mode, " + asHandlerVersion +"."+ AnsiString(SVNRevision));                              //Ifor 20161109 Handler Version Modify
                #else
                    if(IniConfig.bEnableCCDUSETCPIP)
                    {
                        if(IniConfig.bC02InstallCCD)
                        {
                            g_newRecordProcess("MES2108", "Program Start", "CCD Enabled, " + asHandlerVersion +"."+ AnsiString(SVNRevision));                     //Ifor 20161109 Handler Version Modify
                            CCDInterfaceForm->CCDTimerOnOff(true);
                        }
                        else
                        {
                            CCDInterfaceForm->CCDTimerOnOff(false);
                            g_newRecordProcess("MES2108", "Program Start", "CCD Disabled, " + asHandlerVersion +"."+ AnsiString(SVNRevision));                    //Ifor 20161109 Handler Version Modify
                        }
                    }
                    else
                    {
                        g_newRecordProcess("MES2108", "Program Start", asHandlerVersion +"."+ AnsiString(SVNRevision));   //Ifor 20161109 Handler Version Modify
                    }
                #endif
            }
            #endif

#if 0   // GATE G2d (AI(W906-S14) 20261001): TfShowMessage (forms/fShowMessage.h) has no ShowMyMessage(); the golden form is not
        //   in the port.  golden :25265 VERBATIM:
            fShowMessage->ShowMyMessage();
#endif
            bOnceLangFlag=false;
        }
    }
    //jou 2011-08-02 end

    if(bFix3FullPlaceFlag==1)                                                   //Steven 20130126 Start: Fix3滿盤功能
    {
        #ifndef SOFT_SIMULTE
        if(FIX3_FULL_PLACE==Fix3K_ShortShuttle &&
           Sen[SnFix3FullPlace].Enable==false)
        {
            ShowMyMessage("Please enable the sensor 'SnFix3FullPlace'.", "請將感應器'SnFix3FullPlace'啟用。");
            Fix3FullPlaceDelay.SetSecAndOn(10);
            bFix3FullPlaceFlag=2;
        }
        else
        #endif
        {
            bFix3FullPlaceFlag=0;
        }
    }

    if(bFix3FullPlaceFlag==2 && Fix3FullPlaceDelay.Off())
    {
        bFix3FullPlaceFlag=1;
    }

    ShowVacuumOnOffTime();
    RecordTemp();                                                               //Steven 20110924 : 換位置,一秒作一次就好   AI(W906-R126) 20261002: GATE G4 opened (NB2 R126 M5; file-local above)

    if(dCCDTemperature>55.0)                                                    //Steven 20110924 : 紀錄CCD溫度過高的狀況
    {
        if(dCCDTempOld!=dCCDTemperature)
        {
            MNetLog("RTC Temp Over : "+AnsiString(dCCDTemperature));           // [W906] Motor/myMN200motor.cpp:2516 (its fMain->slMNetLog write is gated there)
            dCCDTempOld=dCCDTemperature;
        }
    }
    else
    {
        dCCDTempOld=dCCDTemperature;
    }

    // [W906] golden :25309-25313 (bNoNeedLoadSteupFile -> cbSetupFileName->Text=GetLastOpenFN()) is not here: the flag's only
    //   writer is cbSetupFileNameChange (golden :24754), ported in WebRecipeChange.cpp, where the flag lives in that file's
    //   anonymous namespace (:76-150) and the op does these three lines itself right after the change (:680-684).

    if(NUMBER_PANEL_TYPE!=2)                                                    //Steven 20110517 : 雙位數Bin顯示器異常
        fShowBinSelect->DoShowBinDigital();
    if(NUMBER_PANEL_TYPE==3 ||                                                  //Steven 20110411
       NUMBER_PANEL_TYPE==4)                                                    //Sam 20240604 : 新增 BinDisplay TFT
    {   W906_BinDispShownScope_St02 asShown;  fShowBinSelect->ChangeBinDispStatus();   }   // AI(W906-ST02-C14) 20261002 (St02-E helper): [W906] golden paints tsUnloadMap only while it is the ActivePage (906_0625 cShowBinSelect.cpp:279-292); the web pane cannot say which tab it shows, so the call runs as if it were shown and golden's jump (:272) is counted for the pane -- BinDisplay/BinDispBringUp_St02.h W906_BinDispShownScope_St02

    // [W906] golden :25321-25346 ([A01] iOperatorModeCount=0 while an engineer page is open) is St01's
    //   FileRW/Main_A01AutoLogout.cpp (1), its own once-per-second tick (wb_serve main loop + W906_ModalWaitTick).

#if 0   // GATE G9 (AI(W906-S14) 20261001): S25 (CC_KYEC_LEE).  (AI(W906-R126) 20261002: fQwertyKey does exist, forms/fQwertyKey.h -- S25 is the only reason.)  golden :25348-25363 VERBATIM:
    if(CUSTOMER_CODE==CC_KYEC_LEE)                                              //Ifor 20170803 add Offset (Steven) 頁面鍵盤小視窗 1分鐘後自動關閉
    {
        if(fQwertyKey->Visible==true)
        {
            iCloseQwertyCount++;
            if(iCloseQwertyCount>=60)
            {
                fQwertyKey->Close();
                iCloseQwertyCount=0;
            }
        }
        else
        {
            iCloseQwertyCount=0;
        }
    }
#else
    (void)iCloseQwertyCount;
#endif

    if(IniConfig.bE73_InOutZStepMotorLossCheck)                                 //JerryYang 20240111 : 偵測頻繁z頻繁失步需alarm的功能
    {
        iZhomeTime++;
        if(iZhomeTime>=60)
        {
            if(iInZHomeCnt>=IniConfig.iE73StepMotorCheckCnt)                    //JerryYang 20230918 : 一分鐘回HOME超過5次要跳ALARM
            {
                ShowMyMessage("Input arm Z軸頻繁失步, 請確認機構或是吸料高度是否異常!!");
            }

            if(iOutZHomeCnt>=IniConfig.iE73StepMotorCheckCnt)                   //JerryYang 20230918 : 一分鐘回HOME超過5次要跳ALARM
            {
                ShowMyMessage("Output arm Z軸頻繁失步, 請確認機構或是吸料高度是否異常!!");
            }

            iInZHomeCnt=0;
            iOutZHomeCnt=0;
            iZhomeTime=0;
        }
    }

    // [W906] golden :25386-25460 ([A01] auto switch to Operator; [A01] auto re-open after 30 minutes) is St01's
    //   FileRW/Main_A01AutoLogout.cpp (2) / (3).  Its own state, its own tick: in golden it ran here, after [E73] and before
    //   [A37]; nothing it touches is read below, so the order does not change a result.

//    if(USE_FINGER_PRINT)                                                        //Steven 20190503 : 指紋辨識權限
//    {
//        iLevel=DoScanLevelSensor();
//        if(iLevel!=-1)
//        {
//            if(CosFunction.bSecurityHave5Level==true)
//                iLevel=iLevel+1;
//
//            cbUserSelect->ItemIndex=iLevel;
//            spbUserName->Caption=cbUserSelect->Text;
//            AccessLevel=iLevel;
//            ChangeLevelAttr();
//            NewRecordProcess("MES2144", "======== USER login ========", cbUserSelect->Text);
//        }
//    }

    if(IniConfig.bA37LotStartLotEnd)                                            //JerryYang 20220923 : 矽品版本ART
    {
        if(LastSet.bWaitStartLotAutoRetestGPIB==false && TestIF_File.bSCKART_RunARTWithoutCmd==false)
        {
            if(hLotStartTimeOut.Off())
            {
                if(fSCKART->iCurrent93KARTStep==2 ||
                   fSCKART->iCurrent93KARTStep==8)
                {
                    ShowMyMessage("已完成結批，請執行EAP2S Key in繼續進行測試。\r\n於EAP2S完成Key-in後按下OK按鈕即可關閉此訊息。",
                                  "Lot end finish , please key-in EAP2S to continue test.\r\nFinish the EAP2S Key-in and click the OK Button to close this message.");
                }
                hLotStartTimeOut.SetSecAndOn(120);
            }
        }

        if(LastSet.bWaitEndLotAutoRetestGPIB==false &&
           TestIF_File.bSCKART_RunARTWithoutCmd==false)
        {
            bShowMsg=true;
            if(hLotEndTimeOut.Off())
            {
                if(TrayForm.bSpecTrayCnt==true)                                 //JerryYang 20250220 : AUTO IN OUT
                {
                }
                else
                {
                    if(fSCKART->iCurrent93KARTStep==4 ||
                       fSCKART->iCurrent93KARTStep==10 ||
                       fSCKART->iCurrent93KARTStep==11)
                    {
                        ShowMyMessage("SECS/GEM host Lot End timeout!");
                    }
                }
                hLotEndTimeOut.SetSecAndOn(120);
            }
        }

        if(LastSet.bWaitEndLotAutoRetestGPIB && bShowMsg==true)                 //JerryYang 20220923 : 矽品版本ART
        {
            // [W906] the inner parentheses around `step==4 && (...)` are added: C++ binds && before || (golden means the same);
            //   written out only to keep -Wall (-Wparentheses) quiet.
            if(SystemStart==false &&
            (fSCKART->iCurrent93KARTStep==12 ||
             (fSCKART->iCurrent93KARTStep==4 &&
             (LastSet.iRunStartMode==rsmContinuStart ||
              LastSet.iRunStartMode==rsmContinuRetest ||
              LastSet.iRunStartMode==rsmQAMode))) &&
              IniConfig.bA68_AutoLoadUnload==false)                             //RogerYang 20250611 自動上下料不跳Messagebox
            {
                bShowMsg=false;
                ShowMyMessage("(1)Test summary lot end finished, please key in EAP GUI to do next process.\r\n(2)After put the IC on the handler loader and makesure tester program already loaded then press handler Start button to start test",
                              "(1)測試報表已結檔, 請操作EAP GUI 繼續下一步流程.\r\n(2)Handler上料後並確認測試程式下載完成,再按Start鍵開始測試.");
            }
        }
    }

    for(int i=0; i<bcTotal; i++)
    {
#if 0   // GATE G14 (AI(W906-S14) 20261001): S25 (CC_KYEC_LEE / CC_KYEC_XILINX, [A11]) -- the customer branch is gated, golden's
        //   else below stays live (a KYEC machine therefore clears ReEnterBarcode every second, as golden does without [A11]:
        //   the operator ID is asked again sooner, never later).  golden :25535-25557 VERBATIM:
        if((CUSTOMER_CODE==CC_KYEC_LEE ||
            CUSTOMER_CODE==CC_KYEC_XILINX) &&
            ReEnterBarcode[i]==true &&
            IniConfig.bA11BarcodeTime)                                          //20140310  WEI : [A09] Barcode Reader持續時間
        {
            iBarcodeTimeCount[i]++;

            if(i==12)                                                           //wei 20151022 Offset Barcode 不計數
            {
                iBarcodeTimeCount[i]=0;
            }

            if(iBarcodeTimeCount[i]>=IniConfig.iA11BarcodeTime)
            {
                ReEnterBarcode[i]=false;
                iBarcodeTimeCount[i]=0;
            }
            else
            {
                ReEnterBarcode[i]=true;
            }
        }
        else
#endif
        {
            ReEnterBarcode[i]=false;
        }
    }

    if(bOnceFlag)
    {
        SW[SwClear2].Off();                                                     //Steven 20110920 : 開程式15秒後才on
        SW[SwClear6].Off();                                                     //Alick 20161011 (Steven) : TTL支援8Site
        if(iOpenNewTTLBoardStratDelayCount>=15)
        {
            bOnceFlag=false;
            bChangeTTLFlag=LastSet.bUseNewTTLBoard;
#if 0   // GATE G15 (AI(W906-S14) 20261001): TTLLog has a declaration only (cpublic.h:42); its body cpublic.cpp:669 is inside
        //   `#if 0 // TODO(GA1-B3)` (it needs fMain->slTTLLog wiring) -- same gate as cDIOStatus.cpp:71.  golden :25571 VERBATIM:
            TTLLog("Initial TTL 1");                                            //Steven 20151123 : Log for TTL
#endif
        }
        else
        {
            iOpenNewTTLBoardStratDelayCount++;
        }
    }
    else
    {
        if(bChangeTTLFlag)
        {
            SW[SwClear2].On();                                                  //Alick 20161011 (Steven) : TTL支援8Site
            SW[SwClear6].On();

            if(TestIF_File.bAntiSignal)
            {
                SW[SwClear1].On();
                SW[SwClear5].On();
            }
            else
            {
                SW[SwClear1].Off();
                SW[SwClear5].Off();
            }
        }
    }

    fYieldMonitoring->CheckBySiteYieldAlarm();
    fYieldMonitoring->CheckBySiteByArmYieldAlarm();
    fYieldMonitoring->CheckLowYieldAlarm();
    fYieldMonitoring->CheckLowYieldAlarmByTotal();
    fYieldMonitoring->CheckIntervalLowYieldAlarmBySite();                       //wei 20180606 Interval Low Yield By Site
    fYieldMonitoring->CheckIntervalLowYieldAlarmByTotal();                      //wei 20180718 Interval Low Yield By Total
    fYieldMonitoring->CheckLowYieldAlarmSpecial();                              //Sam 20210505 : PTI 要求的兩段 Low Yeild
    fYieldMonitoring->CheckByPickerYieldAlarm();                                //Steven 20230223 : 根據Index吸嘴比較良率

#if 0   // GATE G17 (AI(W906-S14) 20261001): S25 (CC_UNISEM_M).  (AI(W906-R126) 20261002: fShowBinSelect->Jamrate and fObserver->labMUBA
        //   do exist, forms/fShowBinSelect.h:970 / forms/fObserver.h:771 -- S25 is the only reason.)  golden :25607-25616 VERBATIM:
    if(CUSTOMER_CODE==CC_UNISEM_M)                                              //Ifor 20171018 (wei) : add UNISEM 每五秒更新一次 Jam Rate 於畫面上
    {
        iCloseQwertyCount++;
        if(fNote->fShow==false && iCloseQwertyCount>=5)
        {
            fObserver->ProcessRunInfo();
            iCloseQwertyCount=0;
        }
        fShowBinSelect->Jamrate->Caption=fObserver->labMUBA->Caption;
    }
#endif

    bool bFlag=false;
#if 0   // GATE G18 (AI(W906-S14) 20261001): S25 (CC_JCET, a LoaderData line every 10 minutes) -- the customer branch is gated,
        //   golden's else below stays live (a JCET machine gets the hourly line of every other customer).  golden :25619-25625 VERBATIM:
    if(CUSTOMER_CODE==CC_JCET)                                                  //Steven 20170308 (wei) : 每10分鐘存一筆LoaderData
    {
        if(bFlag || StatusBar1->Panels->Items[6]->Text.SubString(16, 4)=="0:00")
        {
            TimerRecordLoaderDate();
        }
    }
    else
#endif
    {
        //jou 2010-08-26
        if(g_statusBarTime().SubString(15, 5)=="00:00")                         //Steven 20170308 : 每個小時記錄一次   [W906] golden StatusBar1->Panels->Items[6]->Text (StatusBarTimeNow above)
        {
            g_recordLoaderDate();                                               // golden TimerRecordLoaderDate();   [W906] W906_MainClose_TimerRecordLoaderDate (header)
        }
        //jou 2010-08-26
    }
    (void)bFlag;

#if 0   // GATE G19 (AI(W906-S14) 20261001): S25 (CC_KYEC_JCTHIU / CHEN / LEE / XILINX); fLotInfo->btnFtpServerClick / btnFtpTester
        //   are KYEC FTP (FTP folder, ht9045_kyecftp).  golden :25636-25650 VERBATIM:
    static bool bFirst=false;
    //-------Kyec------
    if(CosFunction.bFTPFunction &&
       (CUSTOMER_CODE==CC_KYEC_JCTHIU ||
        CUSTOMER_CODE==CC_KYEC_CHEN ||
        CUSTOMER_CODE==CC_KYEC_LEE ||
        CUSTOMER_CODE==CC_KYEC_XILINX))
    {
        if(bFirst==false && bOnceLangFlag==false)
        {
            bFirst=true;
            if(IniConfig.bEnableFTP)
                fLotInfo->btnFtpServerClick(fLotInfo->btnFtpTester);            //Landam
        }
    }
#endif

    if(IniConfig.bEnableRms)                                                    //Steven 20110503
    {
        if(SystemStart || HasICUnderMachine())
        {
            fLotInfo->edDeviceName->Enabled=false;
            fLotInfo->cbbDeviceName->Enabled=false;
            fLotInfo->edTemp->Enabled=false;

            if(CosFunction.bDownloadRecipeLevelMode)                            //jou 2016-01-06 download recipe 增加權限模式選擇
                fLotInfo->coLevelMode->Enabled=false;
        }
    }

#if 0   // GATE G21 (AI(W906-S14) 20261001): dependency missing -- bNeedRestartProgram is not a member of the fMain facade (its
        //   writers in cSetUp.cpp:1001 / :1021 / :1051 are gated, G-SU-Restart), fFTPClient->bShow / fSetup->fShow are not
        //   either, and the port has no path for a tick to close wb_serve.  golden :25665-25678 VERBATIM:
    if(REAL_TIME_CCD==true)
    {
        if(fNote->fShow==false          &&
           MyMessageBox->fShow==false   &&
           bNeedRestartProgram==true    &&
           !COM2->bCCDDummyRum          &&
           fSetup->fShow==false         &&                                      //Steven 20110927
           fFTPClient->bShow==false)                                            //Sam 20250714 : 修正 FTP 更換工作檔未完成時就被關程式
        {
            ShowMyMessage("Program will auto close don't do any thing", "程式將自動關閉 請勿做任何動作");                             //Ifor 20180828: 修改RTC關閉時顯示訊息避免人員誤會程式會自動重新開啟
            Timer3->Enabled=false;
            Close();
        }
    }
#endif

#if 0   // GATE G22 (AI(W906-S14) 20261001): S25 (CC_ASE_CL); bNeedRestartSW is not in the port, nor a tick-side close.
        //   golden :25680-25688 VERBATIM:
    if(CUSTOMER_CODE==CC_ASE_CL &&
       bNeedRestartSW==true &&
       USE_TRAY_MAPPING==1 &&
       TestIF_File.bEnableTrayID2==true)
    {
            ShowMyMessage("Program will auto close don't do any thing", "程式將自動關閉 請勿做任何動作");                             //Ifor 20180828: 修改RTC關閉時顯示訊息避免人員誤會程式會自動重新開啟
            Timer3->Enabled=false;
            Close();
    }
#endif

#if 0   // GATE G23 (AI(W906-S14) 20261001): dependency missing, display only -- fShowMessage->imgTrayHere / imgDeviceHere are not on
        //   the TfShowMessage facade (forms/fShowMessage.h; only in the dfm2rc layout tables).  golden :25690-25704 VERBATIM:
    static bool bShowTrayDeviceFlag=false;
    if(IniConfig.bShowTrayAndDeviceDir && UserDefForm_File[0].bEnableIndicator)                                         //Steven 20190211 : 可以取消顯示IC方向
    {
        if(bShowTrayDeviceFlag==false)
        {
            fShowMessage->imgTrayHere->Visible=true;
            fShowMessage->imgDeviceHere->Visible=false;
        }
        else
        {
            fShowMessage->imgTrayHere->Visible=false;
            fShowMessage->imgDeviceHere->Visible=true;
        }
        bShowTrayDeviceFlag=!bShowTrayDeviceFlag;
    }
#endif

#if 0   // GATE G24 (AI(W906-S14) 20261001): S25 (ASE) -- bRefresh / bJobFileProd are written only by the ASE socket form (golden
        //   "ASE_K Socket"/aseTest.cpp:512-514), which is not ported; neither flag nor ASESendMessage exists here.  golden :25706-25718 VERBATIM:
    if(bRefresh)                                                                //kevin 20130730 refresh  JOBFILE 檔案不存在
    {
        ShowMyMessage("Please Check JOBFILE work file no exit", "請確認工作檔JOBFILE是否存在。");
        bRefresh=false;
    }

    if(bJobFileProd)                                                            //kevin 20141017 refresh  JOBFILE 檔案  生產中不能任意更換工作檔
    {
        bJobFileProd=false;
        if(IniConfig.bASE_Report)                                               //kevin 20140918 高雄日月光IC履歷記錄
            ASESendMessage->SendToASEData("Machine product.");
        ShowMyMessage("Product Can't change JOBFILE work file", "生產中不能更新工作檔.");
    }
#endif

    // G25 not in this batch (Ifor01 I-01): the soak countdowns need golden's TfMain TQPF members tSoakTimer / tInitSoakTimer,
    //   which the port does not have (the aTester_Front.cpp:1230 gate waits for them too); TO_IFOR I-01 (heater chain) adds
    //   them, then these lines go here, at golden's place.  golden :25720-25728 VERBATIM:
    //    if(iSoakTimer>0)                                                            //2013-11-27    Dell Add Index soak time
    //        iSoakTimer=Temperature.iIndexSoakTime-tSoakTimer.LatchCycleTimeSec();
    //
    //    if(iInitialSoakTimer>0)                                                     //Steven 20140827
    //        iInitialSoakTimer=Temperature.iInitialStart1Time-tInitSoakTimer.LatchCycleTimeSec();
    //    //==> Eastsun 20260526 #026-1.70 Ifor 20230608 add:KYEC 要求新增在主畫面顯示ATC Temp Wait秒數
    //    if(iATCTempWaitTimer>0)
    //        iATCTempWaitTimer=180-(MyTickCount()-iATCTempWaitTimer_Start)/1000;
    //    //<== Eastsun 20260526 #026-1.70

    if(W906_FormShowing("fNote", fNote->fShow)==false)                          // [W906] golden fNote->fShow==false (the port's page table)
    {
        if(iChangeFileHasErr==1)                                                //FTP
        {
            iChangeFileHasErr=0;
            ShowErrorMessage("WAR1684", K_SKIP, MMSystem, 0, asChangeSetupFileName);
        }
        else if(iChangeFileHasErr==2)                                           //Local
        {
            iChangeFileHasErr=0;
            ShowErrorMessage("WAR16500", K_SKIP, MMSystem, 0, asChangeSetupFileName);
        }
    }

#if 0   // GATE G27 (AI(W906-S14) 20261001): S25 (Renesas FT-CT, TestIF_File.bRENESAS_EnableFTCT); MyMessageBox->Showing /
        //   lblMainMsg are not on the port's message box, and the restart is golden TfMain::Start.  golden :25744-25776 VERBATIM:
    static bool bFTCTAlarmTmpFlag=false;
    if(TestIF_File.bRENESAS_EnableFTCT==true)                                   //RogerYang 20250923 : 瑞薩FT-CT
    {
        if(bRenesasFTCTAlarm==true &&
            bFTCTAlarmTmpFlag==false)
        {
            if(MyMessageBox->Showing &&
                MyMessageBox->lblMainMsg->Caption==sFTCTAlarmStr)               //90 Alarm顯示中
            {
            }
            else
            {
                bAutoRestartAfterFTCTAlarm=SystemStart;                         //沒有顯示就跳Alarm
                ShowMyMessage(sFTCTAlarmStr, "");
                bFTCTAlarmTmpFlag=true;
            }
        }
        else if(bRenesasFTCTAlarm==false)                                       //bRenesasFTCTAlarm被解除瞬間
        {
            if(MyMessageBox->Showing &&
                MyMessageBox->lblMainMsg->Caption==sFTCTAlarmStr)               //90 Alarm顯示中
            {
                MyMessageBox->Close();
            }

            if(bFTCTAlarmTmpFlag==true)                                         //曾經show過
            {
                bFTCTAlarmTmpFlag=false;
                if(bAutoRestartAfterFTCTAlarm==true)
                    Start("FTCT_AfterAlarm");
            }
        }
    }
#endif

    // golden :25778-25819 are commented out by golden itself (RogerYang 20260225, CC_SCC loss-tray check) -- not carried.
}

}  // namespace ht9045
