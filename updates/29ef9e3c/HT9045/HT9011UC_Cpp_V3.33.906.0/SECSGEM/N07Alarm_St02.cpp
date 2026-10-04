// =============================================================================
//  N07Alarm_St02.cpp -- the N07 SECS/GEM disconnect alarm (JSCC NetworkMonitor, Steven 20260603 / 20260605).
//  AI(W906-C15-N07) 20261003 (St02-E helper), card ST02-C15 (D:\AI_TempFile\st02-claims\CARD_N07_SECS_DISCONNECT_ALARM_CANDIDATE_20261003.md).
//  Translated from golden 906 only (RULINGS_20261002 #20): D:\HT9045\HT9011UC_Code_V3.33.906.0_20260625_Steven.
//
//  WHAT IT DOES (golden): with [N07] Enable SECS GEM (IniConfig.bEnable_SECS_GEM) and [N07] "SECS GEM Alarm"
//  (IniConfig.bN07_Alarm, chkN07_3_2), ON-LINE and running, golden Timer2 (1 s) checks the Host link every tick; on the
//  first tick that sees THGem::IsConnect()==false the alarm is on at once (bN07AlarmActive); while it is on, the link is
//  checked only every 10th tick and the alarm goes off when the link is back.  Rising edge: "SECS/GEM Connection Lost"
//  into the event log and D:\SECS_GEM_LOGS\yyyy\mm_dd\NetworkMonitor.log; falling edge (link back, or the function /
//  ON-LINE / running condition gone): bAlarmBuzzer and bN07BuzzerSilenced cleared, "SECS/GEM Connection Alarm Released".
//  The machine is NOT stopped.  The tower light and buzzer are ckernel.cpp ShowRunLed (claims there); Alarm Reset
//  silences the buzzer only (ckernel.cpp ScanPannelKey, claims there).
//
//  GOLDEN MAP (906_0625_Steven)
//    main.cpp:20848-20860   static WriteN07Log                    -> WriteN07Log below (file-static, as golden)
//    main.cpp:20937-20999   the N07 block of TfMain::Timer2Timer  -> ht9045::W906_N07Timer2Tick_St02 below
//    cmydef.cpp:4481-4482   bN07AlarmActive / bN07BuzzerSilenced  -> defined below; declared in cmydef.h (claim) as golden
//    main.dfm:17279-17284   Timer2: Enabled=False, no Interval     -> MainTimersSt02.cpp Timer2 slot, 1000 ms
//    main.cpp:10179         FormShow Timer2->Enabled=true          -> the slot latches on with InitialOK, as Timer3 (:10180)
//    main.cpp:20874-20882   fShow / InitialOK / bTimer2Run guards  -> the slot (MainTimersSt02.cpp)
//    ckernel.cpp:777-781    ShowRunLed N07 arm (buzzer)            -> port ckernel.cpp:1677 (claim)
//    ckernel.cpp:875-882,926  ShowRunLed red blink                 -> port ckernel.cpp:1769 / :1813 (claim)
//    ckernel.cpp:2128/:2137 ScanPannelKey Alarm Reset silences     -> port ckernel.cpp:3374 / :3382 (claim)
//
//  [W906] PORT NOTES
//    N1  golden's two block statics (:20939-20940) are file statics here, so the ctest can reset them
//        (W906_N07Timer2Reset_St02); same values, same lifetime, one copy -- same behaviour.
//    N2  the log root: golden hard-codes D:\SECS_GEM_LOGS (:20853).  W906_SECSGEMLOG_ROOT (getenv, set and not empty)
//        replaces only that root, for the ctest (tests/CMakeLists.txt _ht9045_env_extra); unset = golden's literal.
//        File name, folder pattern, line format and fopen mode are golden's, byte for byte (customer-visible).
//    N3  SAFETY-GATE(W906-C15-N07-UI): the screen part of the block -- golden :20976-20982 (Off_lineDisplay red frame,
//        labTesterMode "SECS DISCONNECTED", palMainStatus moved) and :20991-20992 (palMainStatus back) -- is kept
//        verbatim under #if 0.  Missing dependency: the fMain facade (forms/fMain.h) has no Off_lineDisplay and no
//        labTesterMode, vclcompat TControl (vclcompat/Controls.h) has no Left / Top / Height / Width, and the web main
//        page (D:\HT9045\web\page\main.html statusPane) has no tag for that frame.  Gated = the main page frame does not
//        turn red; the alarm itself, the tower light, the buzzer, Alarm Reset and both log lines are all live.
//        fMain->LoadTestModePicture() (:20993) stays live: it is the facade's own method (a no-op here, forms/fMain.cpp).
//    N4  the port has no live HSMS link today: wb_serve's HGem is SecsTagPublish.cpp's catalogue THGem, whose sockets
//        are never opened (nothing outside SECSGEM calls THGem::DoOpenCommuncation / Connect, git grep 20261003), so
//        IsConnect() answers false.  With [N07] SECS GEM Alarm ticked, ON-LINE and running, the alarm therefore comes
//        on at the first running tick and stays on -- which is the true link state, not a port artefact; it is the
//        same answer golden gives on a machine whose Host is unplugged.  Before the first publish HGem is NULL and
//        golden's own `HGem!=NULL` term (:20946) keeps the alarm off.
//    G1  golden oddity kept: fopen(..., "a") is text mode (BCB6 and MinGW msvcrt default _fmode = O_TEXT) and the
//        line already ends "\r\n", so the file gets "\r\r\n" per line.  Customer-visible format => verbatim.
//    G2  golden 906 vs 912 (912 not translated, RULINGS_20261002 #20): 912 main.cpp:21560 drops `SystemStart==true`
//        (RogerYang 20260625, alarm in any handler state) and 912 ckernel.cpp:800-808 moves the buzzer arm out of
//        `else if(SystemStart)`.  This file and the ckernel claims are 906: only while running.
// =============================================================================
#include "MachineType.h"              // first: vclcompat + windows.h (techniques s5)
#include "cmydef.h"                   // SystemStart, ON_LINE, bAlarmBuzzer, SystemYear..SystemSec; bN07AlarmActive / bN07BuzzerSilenced (claim)
#include "Config.h"                   // IniConfig.bEnable_SECS_GEM / bN07_Alarm
#include "LastSet.h"                  // LastSet.iTester
#include "common.h"                   // MyForceDirectories (common.h:341)
#include "cMyDB.h"                    // NewRecordProcess (body cMyDB.cpp) -- not canary_support.h in the same TU (techniques s5)
#include "forms/fMain.h"              // fMain->LoadTestModePicture (facade)
#include "SECSGEM/uHGemEquipment.h"   // THGem::IsConnect, extern THGem *HGem
#include "SECSGEM/N07Alarm_St02.h"

#include <cstdio>
#include <cstdlib>

bool bN07AlarmActive=false;                                                     //Steven 20260603 : Secs_Gem disconnect alarm active flag (JSCC NetworkMonitor)   // golden 906_0625_Steven cmydef.cpp:4481
bool bN07BuzzerSilenced=false;                                                  //Steven 20260603 : N07 alarm buzzer silenced by alarm reset   // golden cmydef.cpp:4482

namespace ht9045 {

namespace {
int  iN07SecCounter = 0;                                                        //10-sec counter, only used while alarm active   // golden :20939 ([W906] N1: file scope)
bool bN07LastAlarm  = false;                                                    //previous alarm state for edge detection   // golden :20940 ([W906] N1)
}  // namespace

//------------------------------------------------------------------------------
static void WriteN07Log(const char *pszEvent)                                   //Steven 20260603 : N07 NetworkMonitor lightweight logger (write under D:\SECS_GEM_LOGS)   // golden :20848
{
    NewRecordProcess("", pszEvent);                                             //keep event-log record (edge-only, no spam)

    AnsiString asDir, asFile, asLine;
    const char* pW906Root=std::getenv("W906_SECSGEMLOG_ROOT");                  // [W906] N2: ctest seam; unset = golden's literal
    if(pW906Root!=NULL && *pW906Root!='\0')
        asDir.sprintf("%s\\%04d\\%02d_%02d", pW906Root, SystemYear, SystemMonth, SystemDate);
    else
        asDir.sprintf("D:\\SECS_GEM_LOGS\\%04d\\%02d_%02d", SystemYear, SystemMonth, SystemDate);   // golden :20853
    MyForceDirectories(asDir);
    asFile = asDir + "\\NetworkMonitor.log";
    asLine.sprintf("%04d/%02d/%02d %02d:%02d:%02d  %s\r\n",
                   SystemYear, SystemMonth, SystemDate, SystemHour, SystemMin, SystemSec, pszEvent);
    FILE *fp = fopen(asFile.c_str(), "a");                                      // [W906] G1: text mode, as golden
    if(fp) { fputs(asLine.c_str(), fp); fclose(fp); }
}

//------------------------------------------------------------------------------
//  golden 906_0625_Steven main.cpp:20937-20999, line for line ([W906] N3 for the two gated screen parts).
void W906_N07Timer2Tick_St02()
{
    //Steven 20260603 : Secs_Gem disconnect alarm (JSCC NetworkMonitor) -- start
    {
        bool bN07Enable = (IniConfig.bEnable_SECS_GEM==true &&
                           IniConfig.bN07_Alarm==true &&
                           LastSet.iTester==ON_LINE &&
                           SystemStart==true &&                                 //B2 : only while running (consistent with ckernel)
                           HGem!=NULL);                                         //A  : guard against null SECS object

        //--- 1. state machine : decide bN07AlarmActive ---
        if(bN07Enable==false)
        {
            bN07AlarmActive = false;                                            //function off / Off-Line / stopped -> force release
            iN07SecCounter  = 0;
        }
        else if(bN07AlarmActive==false)                                         //not alarming : check EVERY second (first trigger no delay)
        {
            if(HGem->IsConnect()==false)
            {
                bN07AlarmActive = true;                                         //first disconnect -> trigger immediately
                iN07SecCounter  = 0;
            }
        }
        else                                                                    //already alarming : check ONLY every 10 seconds
        {
            iN07SecCounter++;
            if(iN07SecCounter>=10)
            {
                iN07SecCounter = 0;
                if(HGem->IsConnect()==true)
                    bN07AlarmActive = false;                                    //link restored -> release
            }
        }

        //--- 2. presentation : level / falling edge ---
        if(bN07AlarmActive==true)                                               //alarm active : keep red panel each loop
        {
#if 0 // SAFETY-GATE(W906-C15-N07-UI) golden :20976-20982 -- missing dependency: no Off_lineDisplay / labTesterMode in the fMain facade, no Left / Top / Height / Width in vclcompat TControl, no web tag for the frame (header [W906] N3)
            Off_lineDisplay->BorderWidth = 20;
            Off_lineDisplay->Color       = (FlushFlag)?clRed:clBtnFace;
            labTesterMode->Caption       = "SECS DISCONNECTED";
            labTesterMode->Visible       = true;
            labTesterMode->Font->Color   = clRed;
            palMainStatus->Left=15; palMainStatus->Top=14;
            palMainStatus->Height=200; palMainStatus->Width=345;
#endif // SAFETY-GATE(W906-C15-N07-UI)
            //buzzer + tower red handled by ckernel ShowRunLed() N07 branch
            if(bN07LastAlarm==false)                                            //rising edge : log once
                WriteN07Log("SECS/GEM Connection Lost");
        }
        else if(bN07LastAlarm==true)                                            //falling edge (recover / off / stop) : restore once
        {
            bAlarmBuzzer       = false;                                         //C : ok to clear here
            bN07BuzzerSilenced = false;
#if 0 // SAFETY-GATE(W906-C15-N07-UI) golden :20991-20992 -- same missing dependency (header [W906] N3)
            palMainStatus->Left=3; palMainStatus->Top=3;
            palMainStatus->Height=215; palMainStatus->Width=365;
#endif // SAFETY-GATE(W906-C15-N07-UI)
            fMain->LoadTestModePicture();                                       //shared owner restores Off_lineDisplay correctly
            WriteN07Log("SECS/GEM Connection Alarm Released");
        }

        bN07LastAlarm = bN07AlarmActive;                                        //update for next-loop edge detection
    }
    //Steven 20260603 : Secs_Gem disconnect alarm (JSCC NetworkMonitor) -- end
}

void W906_N07Timer2Reset_St02()                                                 // ctest only ([W906] N1)
{
    iN07SecCounter     = 0;
    bN07LastAlarm      = false;
    bN07AlarmActive    = false;
    bN07BuzzerSilenced = false;
}

int W906_N07SecCounter_St02() { return iN07SecCounter; }                       // ctest only

}  // namespace ht9045
