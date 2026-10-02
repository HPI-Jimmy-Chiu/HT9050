// ===========================================================================
//  TesterComm/Handler/HandlerBridgeCtl.cpp -- THandlerTesterSide: the Handler's control of the tester bridge
//  (find / launch / send / close).  Golden TfMain bodies, translated per TesterComm/Handler/TRANSLATION_RULES.md
//  and the banner of HandlerTesterSide.h.
//
//  AI(W906-GB-P2a) 20260926.  Golden: D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\main.cpp (Big5/cp950).  AI 20260927: the rule is golden 906_20260618; lines marked `906` are NB2 R92's (St02 has no unpacked 906_20260618), every other number here is still 912 (full sweep waits for NB2's tool; docs/ST02_GOLDEN906_AUDIT.md)
//      :18315-18493  ProcessHVisionConnect   (906 :17694-17872; tester-bridge part live; SPEA Interface.exe / ESD / EventLog gated)
//      :18495-18498  file-scope si / pi / hPro (file-static here: nothing else in V906 uses them)
//      :18499-18596  WakeupGPIB              (CreateProcess -> StartBridgeProgram())
//      :18734-18826  SendMSG_CMD(int)
//      :18828-18847  SendMSG_CMD(int, AnsiString)
//      :18849-18859  SendMSG_CMD_DeviceMapSRQ
//      :18860-18959  SendMSG_TestMode
//      :18961-18964  file-scope globals above RunTestProgram
//      :18965-19193  RunTestProgram
//      :29134-29160  CloseGpibProgram
//      :33010-33038  InitialBarCodeList
//  Plus golden main.cpp:15690-15693 (bEcho / bUnderTest / bExist / bGpibMode) -- see FILE-SCOPE GLOBALS below.
//
//  Name ownership (rule 2):  bFind / HVisionWnd / WakeupGPIBdelay / lblGPIBWND and the methods are THIS class;
//  oldGpibAddress / tBarCodeList are other golden TfMain members -> fMain->X (forms/fMain.h:254 / :337);
//  golden `fMain->bFind` / `fMain->HVisionWnd` -> `bFind` / `HVisionWnd` (this object).
//
//  GATE REGISTER (every `#if 0` in this file; missing symbol / reason)
//    G1  [RETIRED 20260930, laptop Q3=A -- read through FileRW_ProxyChecked, see :107; golden 906_0625_Steven main.cpp:17697-17698]  :18318-18319  fConfiguration->cbI17  -- no `TfConfiguration *fConfiguration` global anywhere in V906
//                      (forms/fConfiguration.h has the class but no extern; the only `fConfiguration` is
//                      Automation/SCK_ART_Remainder.h's TU-local stub) and TfConfiguration has no cbI17.
//                      cbI17 is the transient "[I17] Wait for change GPIB program" box (golden cConfiguration.cpp
//                      :186/:5822 reset it to false on create/close), so offline it is always unchecked.
//    G2  :18326-18328  FindWindow of RS232Standard / "8040GPIB" / Interface -- only feed the SPEA WM_CLOSE posts.
//    [LIFTED 20260927] G3 :18329 (906 :17708)  HESDWnd=FindWindow(ESD_Monitor) -- golden TfMain member -> fMain->HESDWnd; only the
//                      FindWindow, nothing is launched (G5 runs without WakeupESD, RULINGS_20260927 #24 = B).
//    [LIFTED S09-B3 20260929] G4 :18330-18333 (906 :17709-17712)  HEventLogWnd=FindWindow(EventLogSaver / Event Log Analyzer) -- event-log analyzer.
//    [LIFTED 20260927, RULINGS_20260927 #24 = B] G5 :18335-18380  ESD / Ion-bar power sequencing, WakeupESD() call removed (906 :18064-, 912 :18690-18732: never launched).
//    G6  :18384-18390  SPEA: close the RS232/GPIB bridges, look for Interface.exe -- SPEA.
//    G7  :18400-18401  non-SPEA: close a running Interface.exe                 -- SPEA.
//    G8  :18447-18490  EventLogSaver / EventlogAnalyzer relaunch               -- CLOSED: user ruling #31 = A (20260927), V906 never launches it.
//    G9  :18535-18539  fAutomation->PrepareHANARMSConnect() -- no such member on TfAutomationShim
//                      (atester_shims.h) nor on the real TfAutomation (Automation/automation.h); 0 hits tree-wide.
//    G10 :18543        CreateProcess(d:\Interface\Interface.exe)               -- SPEA.
//    [LIFTED S09-B3 20260929] G11 :18775 (906 :18149)  fTesterTCP->AddTCPIPCommunicationLog(0, str2) -- not a TfTesterTCP member (forms/fTesterTCP.h);
//                      its port is the free function TesterTCPSocket_AddTCPIPCommunicationLog
//                      (Interface/TesterTCP_Socket.h:288, ht9045_sm).
//    [LIFTED P2c 20260926] G12 :19037-19040  bP65QAING -- missing global (golden 912 cmydef.h:6037 / cmydef.cpp:6017; V906 cmydef.h
//                      has no such line although Config.h:1495 already has IniConfig.bP65EnableArmQAMode).
//    G13 :19068        fSCKART->iLOTSTATUS_T -- not a TfSCKART member (forms/fSCKART.h has _W/_R/_NONE only;
//                      golden SCK_ART.cpp:45 sets it to 2).
//    [LIFTED S09-B3 20260929] G14 :19184 (906 :18566)  fTesterTCP->SendTCPIPCommand(0,"SOT",str2) -- declared-not-defined on purpose
//                      (forms/fTesterTCP.h:382, its GATE T-P1): a call is a link error.  Port: free function
//                      TesterTCPSocket_SendTCPIPCommand (Interface/TesterTCP_Socket.h:281, ht9045_sm).
//  Commented, not gated (rule 4): :18527-18528 GetExitCodeProcess/TerminateProcess(pi...) of the old bridge.
// ===========================================================================
#include "TesterComm/Handler/HandlerTesterSide.h"   // THandlerTesterSide, fMain, TQPF_Timer, MV HHandler2Gpib, MSG_CMD_*
#include "cprod.h"                // TestIF / TestIF_File / Prod / Temperature / myIAR_Test; IniConfig (Config.h); CosFunction
#include "cmydef.h"               // Handler globals + TTL_MODE/GPIB_MODE/RS232_MODE/TCP_IP_MODE, ON_LINE, Z1_Z2_Down ...;
                                  //   cpublic.h -> StartTestTimeStamp / EndTestTimeStamp, <windows.h>
#include "MachineType.h"          // CC_*, Type_HT*, MAX_SOCKET_TOTAL, eAMD/eIntel, eartInstall, rsmQAMode, eRs23232Bin
#include "canary_support.h"       // LastSet (LastSet.h), ShowMyMessage
#include "forms/fLotInfo.h"       // fLotInfo->edtSysLotID
#include "forms/fSCKART.h"        // fSCKART->SetLotStatus / iLOTSTATUS_W
#include "forms/fTesterIF.h"      // FTestIF->bIsResetRs232Standard
#include "Config.h"               // IniConfig (P2f)
#include "CosFunction.h"          // CosFunction.bFTPFunction (P2f)
#include "common.h"               // CheckAndReadIniData / MySleep (P2f)
#include "aHotPlateSubstrate.h"   // TestSocket.UseSiteHasIC (P2f; not mykitsuck.h directly, KNOWLEDGE.md)
#include "forms/fShowBinSelect.h" // fShowBinSelect->PageControl1Change / ShowBinSel (P2f)
#include "Interface/TesterTCP.h"  // TesterTCP_CopyRecipeToTester (P2f)
#include "Interface/TesterTCP_Socket.h"  // TesterTCPSocket_SendTCPIPCommand (P2f)
#include "Interface/InterfaceSYS.h"   // SendCommand_ESD / ESD_HT_IONBAR_Controller1_PowerOn (G5, AI(W906-ESD-G5) 20260927)
#include "mysensor.h"                // Sen[] (G5)

#include <cstdio>     // sprintf
#include <cstring>    // memset / strncpy
// AI(W906-GB-P2f) 20260926: csystem.h:440 (member || the web window registry); declared here like the other local externs of this file.
bool W906_FormFShow(const char* goldenForm, bool member);

// ---------------------------------------------------------------------------
//  FILE-SCOPE GLOBALS
// ---------------------------------------------------------------------------
// AI(W906-GB-P2a) 20260926: golden main.cpp:15690/15692 `bool bEcho=false, bUnderTest=false; bool bExist=false;`
//   -- V906 already defines all three at atester_shims.cpp:101 (same declaration shape as atester.cpp:650).
extern bool bEcho, bExist, bUnderTest;
// AI(W906-GB-P2a) 20260926: golden main.cpp:15693 `bool bGpibMode=false;` -- a main.cpp file-scope global with NO
//   global-namespace definition anywhere in V906 (grep -w bGpibMode, 20260926: only MessageDef.h:321's MV field and
//   the namespaced gpibbridge / rs232std copies in TesterComm/Gpib/GpibGlobals.cpp:142, Rs232/Rs232Globals.cpp:114).
//   Defined here, external linkage as golden, because RunTestProgram's `HHandler2Gpib.bGpibMode=bGpibMode;` is the
//   bridge's ON/OFF-LINE flag.  The other golden writer (main.cpp:22711-22717) is not translated yet; whoever
//   translates it must `extern` this one, not define a second.
bool bGpibMode=false;
// AI(W906-GB-P2a) 20260926: golden cContact.cpp:77 `const int CONTACT_TEST=3;`.  No header this TU includes carries
//   it (cContact.h deliberately omits it -- see its own "WHY CONTACT_TEST IS NOT IN THE LIST BELOW"); same TU-local
//   literal Command.cpp:317 uses.  Not a gate: iContactMode is a real global.  `static` keeps it internal.  If
//   cContact.h ever gains it, this TU does not include cContact.h, so no redefinition.
static const int CONTACT_TEST = 3;
// AI(W906-GB-P2a) 20260926: golden cMyDB.h:20 is the 3-arg `MyDBIProcess(asTable, S1, S2="")`; V906 keeps a 2-arg
//   adapter (aHotPlateSubstrate.cpp:1245) that forwards (S1, S2) as (S1, S2, "") to the golden 3-arg body in cMyDB.cpp
//   (AI(W906-CMYDB-P4) 20260927 (St02-E), Steven P4 D1=A; the SECSGEM/uHGemEquipment.cpp forwarder is deleted).  Golden's 2-argument call is
//   bound the established V906 way (forms/fHome.cpp:119, forms/fHS.cpp:43, CCLink/MyCCLink.cpp:36): the 2-arg extern
//   below (still observable through W906_MyDBIProcess_Count / _LastS1 / _LastS2).
extern void MyDBIProcess(AnsiString S1, AnsiString S2);

//------------------------------------------------------------------------------
void THandlerTesterSide::ProcessHVisionConnect()
{
    AnsiString str="";
//#if 0 // TODO(W906-GB-P2a): G1 -- no TfConfiguration *fConfiguration global and no cbI17 member in V906 (transient UI box, unchecked offline) -- golden main.cpp:18318-18319   //AI(W906-S09-Q3) 20260930 (St02-E; laptop Q3=A): gate retired -- cbI17 = FileRW/IniConfig.gen.inc:790, cleared on page close as golden FormClose :5719 (IniConfig.gen.inc:7659); body = golden 906_0625_Steven main.cpp:17697-17698
    extern bool FileRW_ProxyChecked(const char* form, const char* name); if(FileRW_ProxyChecked("TfConfiguration", "cbI17"))   // DEVIATION (text only, laptop Q3=A): golden if(fConfiguration->cbI17->Checked)   // 2008/07/29 lee
        return;                                                                 // 2008/07/29 lee
//#endif   //AI(W906-S09-Q3) 20260930: see :106

    static bool bEnter=false;                                                   //ChungHung 20140331 防止重入 開啟多個GPIB
    if(bEnter)
        return;
    bEnter=true;

#if 0 // TODO(W906-GB-P2a): G2 -- Interface.exe is not the tester bridge (plan: SPEA); these three windows only feed the SPEA WM_CLOSE posts (G6/G7) -- golden main.cpp:18326-18328
    HWND InterfaceProgram_Rs232         = FindWindow("TfRS232Main", "RS232Standard" );
    HWND InterfaceProgram_GPIB          = FindWindow("TSerialPoll", "8040GPIB");
    HWND InterfaceProgram_Interface     = FindWindow("TSerialPoll", "Interface");
#endif
    //AI(W906-ESD-G3) 20260927: gate G3 lifted (github-59, St01 §4 23:55).  InterfaceSYS _SendStructMessage_Send returns
    //  early while fMain->HESDWnd is NULL, so no SendCommand_ESD reached the ESD program -- St01 S121 Exit's
    //  ESD_SYSTEM_CLOSE, WebStart's SYSTEM_START / STOP / temperature / On-Off-Line, csystem, TriTemp, SECS.  Only golden's
    //  FindWindow: an ESD_Monitor that is already running gets them; nothing is launched (G5 below runs without WakeupESD, #24 = B).
    //  golden TfMain member HESDWnd -> fMain->HESDWnd (forms/fMain.h:251, rule 2).  ctest TesterComm_Handler part 7.
    fMain->HESDWnd                      = FindWindow("TfESDMain",   "ESD_Monitor");
//#if 0 // TODO(W906-GB-P2a): G4 -- EventLogSaver / Event Log Analyzer is not the tester bridge (plan: event-log analyzer) -- golden main.cpp:18330-18333   //AI(W906-S09-B3) 20260929: gate retired -- RULINGS #31 = A settled the event-log analyzer plan (G8 :268); FindWindow only, nothing launched or sent; golden TfMain member HEventLogWnd -> fMain->HEventLogWnd (forms/fMain.h:252, rule 2, as G3 :126); golden 906_0625_Steven main.cpp:17709-17712
    if(CosFunction.bUseMDB)                                                     //Steven 20231127 : 改用分析器
        fMain->HEventLogWnd             = FindWindow("TfMain_EventLog", "EventLogSaver");   //AI(W906-S09-B3) 20260929: golden HEventLogWnd -> fMain-> (rule 2)
    else
        fMain->HEventLogWnd             = FindWindow("TfrmELA",         "Event Log Analyzer");   //AI(W906-S09-B3) 20260929: as :129
//#endif   //AI(W906-S09-B3) 20260929: see :127

    //AI(W906-ESD-G5) 20260927: gate G5 lifted -- RULINGS_20260927 #24 = B (our #32): golden 912 main.cpp:18335-18380 (inside
    //  906 ProcessHVisionConnect :17694-17872, NB2) translated with the WakeupESD() call REMOVED, so V906 never launches
    //  D:\ESD_Program\EXE\ESD_Program.exe (no ShellExecute anywhere on this path).  Everything else is golden:
    //    * ESD_Monitor / NOVX3360 / KASUGA_Fan / HT IonBar configured and no ESD window -> every 10 s the HT IonBar power-reset
    //      state is armed (count 450, all three "send" flags 0) and RunInfo.ESDSoftwareVersion is cleared;
    //    * once an ESD program is up (started by hand; found by G3 above), one count per call (Timer2, 1000 ms) up to 500, then
    //      ESD_HT_IONBAR_ControllerN_PowerOn is sent once for each IonBar whose alarm sensor is on.
    //  HESDWnd -> fMain->HESDWnd (G3); WakeupESDdelay is this class's member (golden TfMain main.h:1152, rule 2).
    //  ctest TesterComm_Handler part 8.  ⚠ not verified on a machine: needs a real ESD program and the HT IonBar controllers.
    if((ESD_Monitor || USE_NOVX3360 || USE_KASUGA_Fan ||
        iUseHTIonBarFunction!=0)                                                //RogerYang 20250825 : Unloader新增3支IonBar，取代4 5 8 ion fan
        && fMain->HESDWnd==NULL && bESDSystemtype==false)                       //KaiChen 20191225 ：KASUGA Fan 通訊     //kevin 20160111
    {
        if(WakeupESDdelay.Off())
        {
            if(bUseHTIonBar_PowerReset==false && iUseHTIonBarFunction!=0)       //RogerYang 20250825 : Unloader新增3支IonBar，取代4 5 8 ion fan
            {
                bUseHTIonBar_PowerReset =true;
                iUseHTIonBar_SendPowerStatus[0] =0;
                iUseHTIonBar_SendPowerStatus[1] =0;
                iUseHTIonBar_SendPowerStatus[2] =0;
                iUseHTIonBar_PowerResetCount=450;
            }
            RunInfo.ESDSoftwareVersion="";                                      //Ifor 20190509 : add 避免客戶使用無回傳ESD版本的程式關閉Ion偵測
            // WakeupESD();                                                     //AI(W906-ESD-G5) 20260927: REMOVED -- RULINGS_20260927 #24 = B: no program is launched
            WakeupESDdelay.SetSecAndOn(10);
        }
    }
    else if(iUseHTIonBarFunction!=0 && fMain->HESDWnd!=NULL)                    //RogerYang 20250825 : Unloader新增3支IonBar，取代4 5 8 ion fan
    {
        if(bUseHTIonBar_PowerReset==true)                                       //Hmy 20240630 Add HT Ion Bar Controller Power
        {
            iUseHTIonBar_PowerResetCount++;
            if(iUseHTIonBar_PowerResetCount>=500)
            {
                iUseHTIonBar_PowerResetCount =0;
                bUseHTIonBar_PowerReset =false;
                iUseHTIonBar_SendPowerStatus[0] =1;
                iUseHTIonBar_SendPowerStatus[1] =1;
                iUseHTIonBar_SendPowerStatus[2] =1;
            }
        }

        for(int i=0; i<3; i++)
        {
            if(Sen[iHTIonBar[i]].Enable && Sen[iHTIonBar[i]].IsOn())
            {
                if(iUseHTIonBar_SendPowerStatus[i]==1)
                {
                    SendCommand_ESD((ESD_COMMAND)(ESD_HT_IONBAR_Controller1_PowerOn+i));
                    iUseHTIonBar_SendPowerStatus[i] =0;
                }
            }
        }
    }

    if(TestIF.iGpibMode==InterfaceType_SPEA_Type)
    {
        //AI(W906-GB-P2a) 20260926: only the Interface.exe parts are gated; the "Interface not found" arm stays live, so a
        //  SPEA-configured V906 behaves as golden does when Interface.exe is absent: bFind=false (nothing is sent to the
        //  GPIB/RS232 engine) and WakeupGPIB retries its (gated, G10) launch and reports the missing file every 51 tries.
#if 0 // TODO(W906-GB-P2a): G6 -- Interface.exe is not the tester bridge (plan: SPEA); the WM_CLOSE posts would stop the bridge programs (V906: hub engines, no stop call on this class) -- golden main.cpp:18384-18390
        if(InterfaceProgram_Rs232!=NULL) PostMessage(InterfaceProgram_Rs232, WM_CLOSE, 0, 0);
        if(InterfaceProgram_GPIB !=NULL) PostMessage(InterfaceProgram_GPIB , WM_CLOSE, 0, 0);

        if(FindWindow("TSerialPoll", "Interface"))
        {
            bFind=true;
        }
        else
#endif
        {
            bFind=false;
            WakeupGPIB("TfMain::ProcessHVisionConnect_SPEA_Type");
            fMain->oldGpibAddress=-1;                                           //AI(W906-GB-P2a) 20260926: golden TfMain member oldGpibAddress -> fMain-> (forms/fMain.h:254)
        }
    }
    else
    {
#if 0 // TODO(W906-GB-P2a): G7 -- Interface.exe is not the tester bridge (plan: SPEA) -- golden main.cpp:18400-18401
        if(InterfaceProgram_Interface!=NULL)
            PostMessage(InterfaceProgram_Interface, WM_CLOSE, 0, 0);
#endif

        //AI(W906-GB-P2a) 20260926: rule 4 -- every FindWindow of the bridge below is FindBridgeWindow() (the running
        //  engine's token, NULL when none is up); golden's ladder is kept, so a MachineTypeChoice outside these seven
        //  (e.g. Type_HT9050=800) still leaves HVisionWnd untouched, exactly as golden.
        if(!OffLineGpibWay() && (TestIF.iTestType==RS232_MODE || (TestIF_File.iTestType==TTL_MODE && (TTL_CARD_TYPE==2 || TTL_CARD_TYPE==3))))                                         //Isaac 20200903 :TTL RS232通訊
        {
            HVisionWnd=FindBridgeWindow();                                      // golden FindWindow("TfRS232Main", "RS232Standard")
        }
        else
        {
            if(MachineTypeChoice==Type_HT9045)                                  //9045
                HVisionWnd=FindBridgeWindow();                                  // golden FindWindow("TSerialPoll", "9045GPIB")
            else if(MachineTypeChoice==Type_HT9046)                             //9046
                HVisionWnd=FindBridgeWindow();                                  // golden FindWindow("TSerialPoll", "9046GPIB")
            else if(MachineTypeChoice==Type_HT9045_12Site)                      //ChungHung 20130507 add HT9045 updata for 12site 517
                HVisionWnd=FindBridgeWindow();                                  // golden FindWindow("TSerialPoll", "9045GPIB_12Site")
            else if(MachineTypeChoice==Type_HT9046_LS)                          //9046
                HVisionWnd=FindBridgeWindow();                                  // golden FindWindow("TSerialPoll", "9046_32GPIB")
            else if(MachineTypeChoice==Type_HT502)
                HVisionWnd=FindBridgeWindow();                                  // golden FindWindow("TSerialPoll", "502GPIB")
            else if(MachineTypeChoice==Type_HT1032)                             //Steven 20230323 : For HT1032
                HVisionWnd=FindBridgeWindow();                                  // golden FindWindow("TSerialPoll", "1032GPIB")
            else if(MachineTypeChoice==Type_HT7080)                             //Steven 20230323 : For HT7080
                HVisionWnd=FindBridgeWindow();                                  // golden FindWindow("TSerialPoll", "7080GPIB")
        }

        if(HVisionWnd!=NULL)
        {
            bFind=true;                                                         //kevin 20150827
            if(WakeupGPIBdelay.Off()==true)
            {
                //AI(W906-GB-P2a) 20260926: golden passes this->Handle / HVisionWnd (HWND) to %d; a pointer through
                //  a varargs %d is UB in C++ -- same 32-bit decimal kept via (int)(INT_PTR).  this->Handle ->
                //  HandlerWndToken() (rule 4).
                str.sprintf("Handler : %d ; GPIB : %d", (int)(INT_PTR)HandlerWndToken(), (int)(INT_PTR)HVisionWnd);
                lblGPIBWND->Caption=str;
            }
        }
        else
        {
            bFind=false;
            if(WakeupGPIBdelay.Off()==true)
            {
                WakeupGPIB("TfMain::ProcessHVisionConnect_WakeupGPIBdelayOff");
                fMain->oldGpibAddress=-1;                                       //AI(W906-GB-P2a) 20260926: golden TfMain member -> fMain-> (forms/fMain.h:254)
                bGpibRS232Error=true;                                           //wei 20150617 Add version control
                WakeupGPIBdelay.SetSecAndOn(10);
            }
        }
    }

#if 0 // CLOSED(W906-GB-P2a): G8 -- user ruling #31 = A (20260927): V906 never launches EventLogSaver.exe / EventlogAnalyzer.exe; the in-process ElaHub is the analyzer (a hand-started exe still gets SendCommand_EventLog's WM_COPYDATA) -- golden 912 main.cpp:18447-18490
    if(CosFunction.bUseMDB)                                                     //Steven 20231127 : 改用分析器
    {
        if(HEventLogWnd==NULL)
        {
            if(WakeupEventLogSaverdelay.Off()==true)
            {
                if(FindAndKillProcess("EventLogSaver.exe")==1)                  //JerryYang 20200430 當工作管理員中應用程式不見但處理程序還在，強制關閉處理程式
                {
                    NewRecordProcess("", "KillProcess", "Event log Saver");     //Steven 20170317 : 新增紀錄
                }
                else
                {
                    WakeupEventLogSaver();
                }
                WakeupEventLogSaverdelay.SetSecAndOn(10);
            }
        }
        else
        {
            WakeupEventLogSaverdelay.SetSecAndOn(2);
        }
    }
    else if(IniConfig.bO10UseEventLogSaver)
    {
        if(HEventLogWnd==NULL)
        {
            if(WakeupEventLogSaverdelay.Off()==true)
            {
                if(FindAndKillProcess("EventlogAnalyzer.exe")==1)               //JerryYang 20200430 當工作管理員中應用程式不見但處理程序還在，強制關閉處理程式
                {
                    NewRecordProcess("", "KillProcess", "Event log analyzer");  //Steven 20170317 : 新增紀錄
                }
                else
                {
                    WakeupEventLogSaver();
                }
                WakeupEventLogSaverdelay.SetSecAndOn(10);
            }
        }
        else
        {
            WakeupEventLogSaverdelay.SetSecAndOn(2);
        }
    }
#endif

    bEnter=false;                                                               //ChungHung 20140331 防止重入 開啟多個GPIB
}
//------------------------------------------------------------------------------
//Ifor 20150811 :  Fixed for 同時開啟GPIB & RS232
//AI(W906-GB-P2a) 20260926: golden main.cpp:18496-18498 file-scope globals; made file-static because nothing else in V906
//  names them (grep 20260926: 0 hits for `PROCESS_INFORMATION pi` / `STARTUPINFO si` / hPro).  `Cardinal` is BCB6
//  System.hpp's unsigned int (no V906 typedef).  With CreateProcess replaced they are only zeroed, and hPro is only
//  named by the commented-out GetExitCodeProcess/TerminateProcess (rule 4).
static STARTUPINFO si;
static PROCESS_INFORMATION pi;
static unsigned int hPro;                                                       // golden `Cardinal hPro;`
void THandlerTesterSide::WakeupGPIB(AnsiString FuncName)
{
    //Steven 20110116 Start : Add from 7045
    static int iRetryCount=0;
    static bool bfirst=false;
    static int iRunning_Mode;
    static int iProgramReady=0;
    static AnsiString asFilePath="";
    AnsiString str;

    //Ifor 20150811 :新增判斷程式是否啟動
    if(bfirst!=true)
    {
        // variable initialization
        ZeroMemory(&si, sizeof(si));
        si.cb=sizeof(si);
        ZeroMemory(&pi, sizeof(pi));
        bfirst=true;
        iRunning_Mode=EffectiveBridgeTestType();                             //AI(W906-GB-P2d) 20260926: Off-Line rule: the bridge type, not the recipe type
    }
    else
    {
        if(iRunning_Mode!=EffectiveBridgeTestType() ||                        //AI(W906-GB-P2d) 20260926: as above (an On<->Off switch restarts the bridge)
           FTestIF->bIsResetRs232Standard==true)                                //Sam 20181219 : Handler 與 Rs232Standard 設定同步
        {
            FTestIF->bIsResetRs232Standard=false;
            iRunning_Mode=EffectiveBridgeTestType();                         //AI(W906-GB-P2d) 20260926: as above
            // close the program.
            //AI(W906-GB-P2a) 20260926: rule 4 -- the old bridge is not a child process here.  iProgramReady=0 below makes
            //  the next block call StartBridgeProgram(), which switches the hub to TestIF.iTestType (stopping the old
            //  engine) when the type changed.  NOTE: when only bIsResetRs232Standard fired (same type, engine up),
            //  StartBridgeProgram() does not restart the engine, unlike golden's kill + relaunch (see report).
            //GetExitCodeProcess(pi.hProcess, (unsigned long*)&hPro);
            //TerminateProcess(pi.hProcess, (unsigned int) hPro);
            iProgramReady=0;
        }
    }

    if(iProgramReady!=1)
    {
#if 0 // TODO(W906-GB-P2a): G9 -- PrepareHANARMSConnect is not a member of TfAutomationShim (atester_shims.h) nor of TfAutomation (Automation/automation.h) -- golden main.cpp:18535-18539
        if(fAutomation &&                                                       //RogerYang : ready HANA RMS before launch GPIB
            CUSTOMER_CODE==CC_HANA_MICRON)
        {
            fAutomation->PrepareHANARMSConnect();
        }
#endif
        if(TestIF.iGpibMode==InterfaceType_SPEA_Type)
        {
            asFilePath="d:\\Interface\\Interface.exe";
#if 0 // TODO(W906-GB-P2a): G10 -- Interface.exe is not the tester bridge (plan: SPEA); iProgramReady stays 0 -- golden main.cpp:18543
            iProgramReady=CreateProcess(NULL, asFilePath.c_str(), NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);
#endif
        }
        else if(!OffLineGpibWay() && (TestIF.iTestType==RS232_MODE || (TestIF_File.iTestType==TTL_MODE && (TTL_CARD_TYPE==2 || TTL_CARD_TYPE==3))))                                    //Isaac 20200903 :TTL RS232通訊
        {
            asFilePath="d:\\RS232Standard\\RS232Standard.exe";
            iProgramReady=StartBridgeProgram();                                 //AI(W906-GB-P2a) 20260926: rule 4, golden iProgramReady=CreateProcess(NULL, asFilePath.c_str(), NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);
        }
        else
        {
            asFilePath="d:\\gpib9045\\H9046_32GPIB.exe";
            iProgramReady=StartBridgeProgram();                                 //AI(W906-GB-P2a) 20260926: rule 4, golden iProgramReady=CreateProcess(NULL, asFilePath.c_str(), NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);
        }
        iRetryCount++;
    }
    else
    {
        iRetryCount=0;
        //AI(W906-GB-P2a) 20260926: rule 4 -- FindWindow of the bridge -> FindBridgeWindow(), golden ladder kept.
        if(!OffLineGpibWay() && (TestIF.iTestType==RS232_MODE || (TestIF_File.iTestType==TTL_MODE && (TTL_CARD_TYPE==2 || TTL_CARD_TYPE==3))))                                         //Isaac 20200903 :TTL RS232通訊
        {
            HVisionWnd=FindBridgeWindow();                                      // golden FindWindow("TfRS232Main", "RS232Standard")
        }
        else
        {
            if(MachineTypeChoice==Type_HT9045)                                  //9045
                HVisionWnd=FindBridgeWindow();                                  // golden FindWindow("TSerialPoll", "9045GPIB")
            else if(MachineTypeChoice==Type_HT9046)                             //9046
                HVisionWnd=FindBridgeWindow();                                  // golden FindWindow("TSerialPoll", "9046GPIB")
            else if(MachineTypeChoice==Type_HT9045_12Site)                      //ChungHung 20130507 add HT9045 updata for 12site 517
                HVisionWnd=FindBridgeWindow();                                  // golden FindWindow("TSerialPoll", "9045GPIB_12Site")
            else if(MachineTypeChoice==Type_HT9046_LS)                          //9046
                HVisionWnd=FindBridgeWindow();                                  // golden FindWindow("TSerialPoll", "9046_32GPIB")
            else if(MachineTypeChoice==Type_HT502)
                HVisionWnd=FindBridgeWindow();                                  // golden FindWindow("TSerialPoll", "502GPIB")
            else if(MachineTypeChoice==Type_HT1032)
                HVisionWnd=FindBridgeWindow();                                  // golden FindWindow("TSerialPoll", "1032GPIB")
            else if(MachineTypeChoice==Type_HT7080)
                HVisionWnd=FindBridgeWindow();                                  // golden FindWindow("TSerialPoll", "7080GPIB")
        }

        if(HVisionWnd==NULL)
        {
            iProgramReady=0;
        }
    }

    if(iRetryCount>50)
    {
        iRetryCount=0;
        str.sprintf("%s file not existence!!", asFilePath);                     //AI(W906-GB-P2a) 20260926: raw AnsiString to %s is safe here -- vclcompat AnsiString::sprintf converts it (AnsiString.h:145-152)
        if(IniConfig.bC04EnableTestTempIC==false)
          ShowMyMessage(str);
    }
    bWakeupGPIBFile=false;
}
//------------------------------------------------------------------------------
void THandlerTesterSide::SendMSG_CMD(int CMD)                                   //Steven 20140922 : Arm2當作指紋測試
{
    AnsiString str="", str2="";
    unsigned int CMD1=CMD;

    if(bFind==false)                                                            //AI(W906-GB-P2a) 20260926: golden fMain->bFind; bFind is a member of this class (rule 2)
        return;

    if(TestIF.iTestType==TCP_IP_MODE &&                                         //wei 20211027 open short TCP/IP
       LastSet.iTester==ON_LINE)                                                //Steven 20231113 : 修正TCP/IP Test跑Off Line
    {
        if(CMD1==MSG_CMD_EnableBarCode)
            str="Handler ==> Enable Bar Code Command";
        else if(CMD1==MSG_CMD_DisableBarCode)
            str="Handler ==> Disable Bar Code Command";
//        else if(CMD==MSG_CMD_EnableAMDFunction)
//            str="Handler ==> Enable AMD Function";
//        else if(CMD==MSG_CMD_DisableAMDFunction)
//            str="Handler ==> Disable AMD Function";
        else if(CMD1==MSG_CMD_TimeOutRetrySend)
            str="Handler ==> Test Time Out - Retry and Resend SOT";
        else if(CMD1==MSG_CMD_TimeOutRetryWait)
            str="Handler ==> Test Time Out - Retry and Wait Result";
        else if(CMD1==MSG_CMD_TimeOutSkip)
            str="Handler ==> Test Time Out - SKIP";
        else if(CMD1==MSG_CMD_Arm1Down)
            str="Handler ==> Arm 1 Down";
        else if(CMD1==MSG_CMD_Arm2Down)
            str="Handler ==> Arm 2 Down";
        else if(CMD1==MSG_CMD_HandlerHomeStart)
            str="Handler ==> Home Start";
        else if(CMD1==MSG_CMD_HandlerHomeFinish)
            str="Handler ==> Home Finish";
        else if(CMD1==MSG_CMD_ContactTestArm1)
            str="Handler ==> Contact Test Arm 1";
        else if(CMD1==MSG_CMD_ContactTestArm2)
            str="Handler ==> Contact Test Arm 2";

        if(str!="")
        {
            str2.sprintf("Log , %s", str.c_str());
//#if 0 // TODO(W906-GB-P2a): G11 -- AddTCPIPCommunicationLog is not a TfTesterTCP member (port: free function TesterTCPSocket_AddTCPIPCommunicationLog, Interface/TesterTCP_Socket.h:288) -- golden main.cpp:18775   //AI(W906-S09-B3) 20260929: gate retired -- bound to the V906 port TesterTCPSocket_AddTCPIPCommunicationLog (Interface/TesterTCP_Socket.h:288), the P2f fTesterTCP->X mapping of this file (header :996); golden 906_0625_Steven main.cpp:18149
            TesterTCPSocket_AddTCPIPCommunicationLog(0, str2);                  //AI(W906-S09-B3) 20260929: golden fTesterTCP->AddTCPIPCommunicationLog(0, str2) -- no TfTesterTCP form in V906
//#endif   //AI(W906-S09-B3) 20260929: see :475
        }
        return;
    }

    HHandler2Gpib.iSendCommand=CMD;
    HHandler2Gpib.bCloseGpib=false;
    HHandler2Gpib.HandlerHwnd=HandlerWndToken();                                //AI(W906-GB-P2a) 20260926: rule 4, golden this->Handle
    HHandler2Gpib.GpibHwnd=HVisionWnd;

    if(TestIF.iGpibMode==InterfaceType_Delta_Castle || TestIF_File.i2DIDFormat==eAMD)                            //Steven 20260428 : Delta Castle 強制送SiteMap & ATC Type (取代AMD_Version compile flag) //Ifor 20260717 add:不使用define方式處理，改用TEST IF 設定判斷
    {
        memset(HHandler2Gpib.UseSiteMapData, '\0', sizeof(HHandler2Gpib.UseSiteMapData));
        //AI(W906-GB-P2a) 20260926: golden quirk kept -- the length is the SOURCE length, so a site-map string longer than
        //  255 chars runs past UseSiteMapData[256] into asATC_TYPE/MultiMessage (and loses the terminator at exactly 256).
        strncpy(HHandler2Gpib.UseSiteMapData, aSendSiteMapping.c_str(), aSendSiteMapping.Length());
        strncpy(HHandler2Gpib.asATC_TYPE, Temperature.ATCTypeName.c_str(), sizeof(HHandler2Gpib.asATC_TYPE));
    }

    if(CMD==(int)MSG_CMD_SCKART_RunDummy)
    {
        if(LastSet.iTester==OFF_LINE)
        {
            HHandler2Gpib.bSimulate=true;
            if(fLotInfo->edtSysLotID->Text!="")
            {
                //AI(W906-GB-P2a) 20260926: golden quirk kept -- Message is not cleared first and strncpy with the source
                //  length copies no terminator, so a longer previous Message leaves its tail after the lot ID.
                strncpy(HHandler2Gpib.Message, fLotInfo->edtSysLotID->Text.c_str(), fLotInfo->edtSysLotID->Text.Length());
            }
            else
            {
                sprintf(HHandler2Gpib.Message, "DummyLot\r\n");
            }
        }
        else if(CosFunction.bUseSCKART &&
                IniConfig.bA10_AutoReTest &&
                TestIF_File.bSCKART_EnableART &&
                TestIF_File.bSCKART_RunARTWithoutCmd)
        {
            HHandler2Gpib.bSimulate=TestIF_File.bSCKART_RunARTWithoutCmd;
        }
        else
        {
            HHandler2Gpib.bSimulate=false;
        }
    }

    COPYDATASTRUCT *pcp=new COPYDATASTRUCT ;
    pcp->dwData=0;

    pcp->cbData=sizeof(HHandler2Gpib);
    pcp->lpData=(unsigned char *)&HHandler2Gpib.iSendCommand;
    SendToBridge(pcp);                                                          //AI(W906-GB-P2a) 20260926: rule 4, golden SendMessage(fMain->HVisionWnd, WM_COPYDATA, (WPARAM) NULL, (LPARAM)pcp);
    delete pcp;
}
//------------------------------------------------------------------------------
void THandlerTesterSide::SendMSG_CMD(int CMD, AnsiString Message)               //Steven 20190401 : 整合SendMessage
{
    HHandler2Gpib.iSendCommand=CMD;
    memset(HHandler2Gpib.Message, '\0', sizeof(HHandler2Gpib.Message));         //清空陣列
    //AI(W906-GB-P2a) 20260926: golden quirk kept -- source-length strncpy: a Message of 2048+ chars overruns
    //  HHandler2Gpib.Message[2048].  Golden also does not check bFind here, and does not refresh HandlerHwnd/GpibHwnd
    //  (they keep the last SendMSG_CMD(int)/SendMSG_TestMode/RunTestProgram values).
    strncpy(HHandler2Gpib.Message, Message.c_str(), Message.Length());

    if(TestIF.iGpibMode==InterfaceType_Delta_Castle || TestIF_File.i2DIDFormat==eAMD)                            //Steven 20260428 : Delta Castle 強制送SiteMap & ATC Type (取代AMD_Version compile flag) //Ifor 20260717 add:不使用define方式處理，改用TEST IF 設定判斷
    {
        memset(HHandler2Gpib.UseSiteMapData, '\0', sizeof(HHandler2Gpib.UseSiteMapData));
        strncpy(HHandler2Gpib.UseSiteMapData, aSendSiteMapping.c_str(), aSendSiteMapping.Length());
        strncpy(HHandler2Gpib.asATC_TYPE, Temperature.ATCTypeName.c_str(), sizeof(HHandler2Gpib.asATC_TYPE));
    }

    COPYDATASTRUCT *pcp=new COPYDATASTRUCT;
    pcp->dwData=0;
    pcp->cbData=sizeof(HHandler2Gpib);
    pcp->lpData=(unsigned char *)&HHandler2Gpib.iSendCommand;
    SendToBridge(pcp);                                                          //AI(W906-GB-P2a) 20260926: rule 4, golden SendMessage(fMain->HVisionWnd, WM_COPYDATA, (WPARAM) NULL, (LPARAM)pcp);
    delete pcp;
}
//------------------------------------------------------------------------------
void THandlerTesterSide::SendMSG_CMD_DeviceMapSRQ(int iStatus)
{
    HHandler2Gpib.iSendCommand=MSG_CMD_DeviceMapSRQ;
    HHandler2Gpib.iLotStatus=iStatus;
    COPYDATASTRUCT *pcp=new COPYDATASTRUCT;
    pcp->dwData=0;
    pcp->cbData=sizeof(HHandler2Gpib);
    pcp->lpData=(unsigned char *)&HHandler2Gpib.iSendCommand;
    SendToBridge(pcp);                                                          //AI(W906-GB-P2a) 20260926: rule 4, golden SendMessage(fMain->HVisionWnd, WM_COPYDATA, (WPARAM) NULL, (LPARAM)pcp);
    delete pcp;
}
//******************************************************************************
//
//Steven 20161122 (Jou) : Add 16 bin GS and 32 bin GS
//必須搭配V6.0以後的GPIB軟體, 可能導致無法測試
//
//******************************************************************************
void THandlerTesterSide::SendMSG_TestMode()
{
    if(bFind==false)                                                            //AI(W906-GB-P2a) 20260926: golden fMain->bFind -> this object's bFind (rule 2)
        return;
    HHandler2Gpib.iSendCommand=MSG_CMD_TesterMode;
    HHandler2Gpib.bCloseGpib=false;
    HHandler2Gpib.HandlerHwnd=HandlerWndToken();                                //AI(W906-GB-P2a) 20260926: rule 4, golden this->Handle (the bridges compare it, golden OnMyCopyMsg identity check)
    if(USE_AUTO_RETEST==eartInstall)
        HHandler2Gpib.bTimeOutProcess=IniConfig.bA10TestModeForART;             //Jimmychiu 20231205 : 借用變數bTimeOutProcess，當作A10-3開啟判斷
    else
        HHandler2Gpib.bTimeOutProcess=false;

//    HHandler2Gpib.GPIBBin=TestIF_File.iGpibMode;                              //Isaac 20200903 :TTL RS232通訊
    if(!OffLineGpibWay() && TestIF_File.iTestType==RS232_MODE)                //AI(W906-GB-P2d) 20260926: Off-Line rule: GPIB fields (else arm)
    {
        HHandler2Gpib.GPIBBin=TestIF_File.iRs232Mode;
    }
    else if(!OffLineGpibWay() && TestIF_File.iTestType==TTL_MODE && (TTL_CARD_TYPE==2 || TTL_CARD_TYPE==3))             //AI(W906-GB-P2d) 20260926: as above  //Isaac 20210309 :TTL RS232兩塊板子
    {
        if(TTL_CARD_TYPE==3)                                                    //20210920 Isaac : 兩塊板子
        {
            HHandler2Gpib.bSupport32Bin=true;                                   //true : use two TTL board
            HHandler2Gpib.bGpibMode=true;                                       //true:要帶站號

            if(Prod.DIOCfg.iCateBitLength==_3Bit || Prod.DIOCfg.iCateBitLength==_4Bit   ||
               Prod.DIOCfg.iCateBitLength==_5Bit || Prod.DIOCfg.iCateBitLength==_5BitPE ||
               Prod.DIOCfg.iCateBitLength==_5BitPO)
            {
                if(Prod.DIOCfg.iCateDataType==CHOneByOne)
                {
                    //5BitBit
                    if(TestIF.iTestMode<=_8Site2X4)                             //Isaac 20210309 :TTL RS232兩塊板子
                    {
                        HHandler2Gpib.bSupport32Bin=false;                      //true : use two TTL board
                    }
                }
                else                                                            //CHBinary
                {
                    //5BitBinary
                    if(TestIF.iTestMode<=_8Site2X4)                             //Isaac 20210309 :TTL RS232兩塊板子
                    {
                        HHandler2Gpib.bSupport32Bin=false;                      //true : use two TTL board
                    }
                }
            }
            else
            {
                if(Prod.DIOCfg.iCateDataType==CHOneByOne)
                {
                    //10BitBit
                    if(TestIF.iTestMode<=QualSite2X2N)                          //Isaac 20210309 :TTL RS232兩塊板子
                    {
                        HHandler2Gpib.bSupport32Bin=false;                      //true : use two TTL board
                    }
                    else                                                        //if(TestIF.iTestMode>QualSite2X2)                     //Isaac 20210309 :TTL RS232兩塊板子
                    {
                        HHandler2Gpib.bSupport32Bin=true;                       //true : use two TTL board
                    }
                }
                else                                                            //CHBinary
                {
                    //10BitBinary
                    if(TestIF.iTestMode<=QualSite2X2N)                          //Isaac 20210309 :TTL RS232兩塊板子
                    {
                        HHandler2Gpib.bSupport32Bin=false;                      //true : use two TTL board
                    }
                    else                                                        //if(TestIF.iTestMode>QualSite2X2)                     //Isaac 20210309 :TTL RS232兩塊板子
                    {
                        HHandler2Gpib.bSupport32Bin=true;                       //true : use two TTL board
                    }
                }
            }
        }
        else                                                                    //TTL_CARD_TYPE==2; 1板子，無站號
        {
            HHandler2Gpib.bSupport32Bin=false;                                  //true : use two TTL board
            HHandler2Gpib.bGpibMode=TTL_CARD_USE_ADDRESS;                       //true:要帶站號
        }
        HHandler2Gpib.GPIBBin=TestIF_File.iDioMode+InterfaceType_TTL;
    }
    else                                                                        //if(TestIF.iTestType==GPIB_MODE)
    {
        HHandler2Gpib.GPIBBin=TestIF_File.iGpibMode;
    }
    HHandler2Gpib.GpibHwnd=HVisionWnd;                                          //AI(W906-GB-P2a) 20260926: the bridge token (ProcessHVisionConnect/WakeupGPIB set HVisionWnd=FindBridgeWindow())

    COPYDATASTRUCT *pcp=new COPYDATASTRUCT ;
    pcp->dwData=0;

    pcp->cbData=sizeof(HHandler2Gpib);
    pcp->lpData=(unsigned char *)&HHandler2Gpib.iSendCommand;
    SendToBridge(pcp);                                                          //AI(W906-GB-P2a) 20260926: rule 4, golden SendMessage(fMain->HVisionWnd, WM_COPYDATA, (WPARAM) NULL, (LPARAM)pcp);
    delete pcp;
}
//------------------------------------------------------------------------------
//AI(W906-GB-P2a) 20260926: golden main.cpp:18961-18964.  bDoubleContact: V906 defines it at atester.cpp:173 (atester.h:42
//  declares it) -- golden's own extern line kept.  bStartTestSD / iStartTestSD_Task: no V906 definition (grep
//  20260926, 0 hits) -> defined here, external linkage as golden (golden itself never reads them either).
//  DeleteChromaTesterResult: golden only declares it (no body, no caller in golden); declaration kept.
extern bool bDoubleContact;
bool bStartTestSD[4]={false,false,false,false};
int iStartTestSD_Task=-1;
void DeleteChromaTesterResult();
//******************************************************************************
//
//  注意!! RunTestProgram為Handler通知GPIB是否要執行測試, 修改時要小心!!
//
//******************************************************************************
//jou 981226 start : for 9046
bool THandlerTesterSide::RunTestProgram(bool bNeedTest, bool *bSiteOnOff)
{
    if(bFind==false)                                                            //AI(W906-GB-P2a) 20260926: golden fMain->bFind -> this object's bFind (rule 2)
        return false;
    HHandler2Gpib.iSendCommand=MSG_CMD_NONE;

    bEcho=false;
    bExist=false;
    COPYDATASTRUCT *pcp=new COPYDATASTRUCT ;
    pcp->dwData=0;

    bool bSite[MAX_SOCKET_TOTAL];

    if(bNeedTest==false)                                                        //Steven 20170214 (wei): Add protection
    {
        bAutoSiteMapWaitTestResult=false;                                       //Steven 20220815 : 修正Auto Site Map遇到Test Time Out / Reset會Hang up
        IsTest=false;
    }

    if(bNeedTest)                                                               //Steven 20200715 : 重新計算Cycle Time
    {
        //AI(W906-GB-P2a) 20260926: parentheses added around golden's `a && b` inside `||` -- precedence-identical
        //  (&& binds tighter), only silences -Wparentheses.
        if(IndexStatus==Z1_Z2_Down)
            myIAR_Test.SetStartTest(3);                                         //Steven 20250102 : fixed for [I54]
        else if(IndexStatus==Z1Down_Z2Up || (iContactMode==CONTACT_TEST && iIndexArm==0))
            myIAR_Test.SetStartTest(1);
        else if(IndexStatus==Z1Up_Z2Down || (iContactMode==CONTACT_TEST && iIndexArm==1))
            myIAR_Test.SetStartTest(2);

        StartTestTimeStamp();
    }
    else
    {
        if(bUnderTest)
            EndTestTimeStamp();
    }

    bUnderTest=false;

    if(CUSTOMER_CODE==CC_MTI || CUSTOMER_CODE==CC_PTI)                          //Sam 20190429 : Add CC_PTI_NEWWORK
        HHandler2Gpib.iLotStatus=0;                                             //ChungHung OLP 聚成專用
    if(Prod.bD22SupportMultiDoubleContact &&                                    //Sam 20231117 : 整合到 QA 模式 //Steven 20201024 : Fixed for double contact
        bDoubleContact &&
        IniConfig.bD22DoubleContactUseDiffSRQ)                                  //JerryYang 20221004 : Double contact改成可以選擇不同的測試訊號   //Steven 20230508 : 南茂鐘永生說要使用0x41
    {
        if(IniConfig.iD23_DoubleContactSRQ==0)                                  //0x42
        {
            HHandler2Gpib.iLotStatus=1;                                         //0x42
        }
        else if(IniConfig.iD23_DoubleContactSRQ==1)                             //0x43
        {
            HHandler2Gpib.iLotStatus=2;                                         //0x43
        }
        else if(IniConfig.iD23_DoubleContactSRQ==2)                             //0xC1
        {
            HHandler2Gpib.iLotStatus=3;
        }
        else
        {
            HHandler2Gpib.iLotStatus=0;
        }
    }
    else
    {
        HHandler2Gpib.iLotStatus=0;
    }

//AI(W906-GB-P2c) 20260926: gate G12 lifted -- bP65QAING added to cmydef (P2c, golden 912 cmydef.cpp:6017); its setters (atester / csystem) come with P2b, so it reads false until then -- golden main.cpp:19037-19040
    if(IniConfig.bP65EnableArmQAMode && bP65QAING && bNeedTest)                 //Ifor 20260422: [P65] ARM QA Mode - re-test sends 0x42
    {
        HHandler2Gpib.iLotStatus=1;                                             //0x42
    }

    for(int i=0; i<MAX_SOCKET_TOTAL; i++)
    {
        bSite[i]=(bNeedTest)?bSiteOnOff[i]:false;
    }

    if(CUSTOMER_CODE==CC_ASE_CL && TestIF_File.bEnableBarCode)                  //KaiChen 20191121 ：中壢日月光 2D Check Sum
    {
        if(bNeedTest)
        {
            for(int i=0; i<MAX_SOCKET_TOTAL; i++)
            {
                if(bSite[i]==true &&
                   (fMain->tBarCodeList->Strings[31-i]==asBarCodeErrorSend ||   //AI(W906-GB-P2a) 20260926: golden TfMain member tBarCodeList -> fMain-> (forms/fMain.h:337)
                    fMain->tBarCodeList->Strings[31-i]==asBarCodeErrorCheckSum))
                {
                    bSite[i]=false;
                }
            }
        }
    }

    bUnderTest=bNeedTest;

    if(CosFunction.bUseSCKART)                                                  //Steven 20161201 (wei) : For SCK FLEX ART
    {
        if(bNeedTest)
        {
            //AI(W906-GB-P2a) 20260926: braces added so the gated statement leaves golden's if/else shape intact.
#if 0 // TODO(W906-GB-P2a): G13 -- iLOTSTATUS_T is not a TfSCKART member (forms/fSCKART.h; golden SCK_ART.cpp:45 sets 2) -- golden main.cpp:19068
            fSCKART->SetLotStatus(fSCKART->iLOTSTATUS_T);
#endif
        }
        else
            fSCKART->SetLotStatus(fSCKART->iLOTSTATUS_W);
    }

    for(int i=0; i<MAX_SOCKET_TOTAL; i++)
        HHandler2Gpib.Site[i]=bSite[i];

    if(TestIF.iGpibMode==InterfaceType_Delta_Castle || TestIF_File.i2DIDFormat==eAMD)   //Steven 20260428 : Delta Castle 強制送SiteMap & ATC Type (取代AMD_Version compile flag) //Ifor 20260717 add:不使用define方式處理，改用TEST IF 設定判斷
    {
        memset(HHandler2Gpib.UseSiteMapData, '\0', sizeof(HHandler2Gpib.UseSiteMapData));
        //AI(W906-GB-P2a) 20260926: golden quirk kept -- the site map is the FORMAT string (a '%' in it would be read as a
        //  conversion; UB) and nothing bounds it to UseSiteMapData[256].
        sprintf(HHandler2Gpib.UseSiteMapData, aSendSiteMapping.c_str());
        strncpy(HHandler2Gpib.asATC_TYPE, Temperature.ATCTypeName.c_str(), sizeof(HHandler2Gpib.asATC_TYPE));
    }

    HHandler2Gpib.bSimulate=(LastSet.iTester==ON_LINE)?false:true;              //Steven 20150713 : 指定OnLine & OffLine變數
    HHandler2Gpib.bCloseGpib=false;
    HHandler2Gpib.bTimeOutProcess=TestISTimeOut;
    TestISTimeOut=false;

    if(TestIF.iTestType==GPIB_MODE  || OffLineGpibWay() ||                     //AI(W906-GB-P2d) 20260926: Off-Line rule: GPIB fields
       (TestIF.iTestType==RS232_MODE && TestIF.iRs232Mode==eRs23232Bin))
        HHandler2Gpib.bSupport32Bin=true;                                       //Steven 20121112 : RS232支援32Bin b8080 --> bSupport32Bin
    else
        HHandler2Gpib.bSupport32Bin=false;

    if(!OffLineGpibWay() && TestIF.iTestType==RS232_MODE && TestIF.iRs232Mode==eRs23232Bin)   //AI(W906-GB-P2d) 20260926: as above  //Steven 20121116 : RS232回傳的最大Bin數
    {
        HHandler2Gpib.GpibAddress=TestIF_File.iRs232MaxBinCount+1;
    }
    else
    {
        HHandler2Gpib.GpibAddress=TestIF.iGpibAddress;
    }

    HHandler2Gpib.MachineISRun=SystemStart;
    HHandler2Gpib.IsTest=IsTest;
    if(IsTest)
        IsTest=false;
    if(LastSet.iTester==ON_LINE && TestIF.iTestType==GPIB_MODE)                 //Steven 20150713 : 整理LastSet.iTester
        bGpibMode=true;
    else
        bGpibMode=false;
    HHandler2Gpib.bGpibMode=bGpibMode;

    if(!OffLineGpibWay() && TestIF_File.iTestType==TTL_MODE && (TTL_CARD_TYPE==2 || TTL_CARD_TYPE==3))                  //AI(W906-GB-P2d) 20260926: as above  //Isaac 20200903 :TTL RS232通訊
    {
        HHandler2Gpib.GPIBBin=TestIF_File.iDioMode+InterfaceType_TTL;
    }
    else
    {
        HHandler2Gpib.GPIBBin=iTestBinCount;                                    //kevin 20140308 add 256Bin
    }
    HHandler2Gpib.HandlerHwnd=HandlerWndToken();                                //AI(W906-GB-P2a) 20260926: rule 4, golden this->Handle
    HHandler2Gpib.GpibHwnd=HVisionWnd;

    if(TestIF_File.bOcrFunction || TestIF_File.bEnableBarCode)                  //Steven 20150713 : for 2D Code   //wei 20150924
    {
        if(bNeedTest)
        {
            if(TestIF_File.bSetCloseSite2DIDtoEmpty)                            //Steven 20190313 : Close site 2DID set to empty
            {
                for(int i=0; i<MAX_SOCKET_TOTAL; i++)
                {
                    if(fMain->tBarCodeList->Strings[i]=="0")
                        fMain->tBarCodeList->Strings[i]="";
                }
            }
            //AI(W906-GB-P2a) 20260926: CommaText is a proxy without c_str() -> AnsiString(...) (rule 8); the temporary lives
            //  to the end of the strncpy statement.  Golden quirk kept: a 2048-char CommaText leaves no terminator.
            strncpy(HHandler2Gpib.Message, AnsiString(fMain->tBarCodeList->CommaText).c_str(), sizeof(HHandler2Gpib.Message));
        }
        else
        {
            InitialBarCodeList();
            strncpy(HHandler2Gpib.Message, "", sizeof(HHandler2Gpib.Message));
        }
    }
    else
    {
        InitialBarCodeList();
        if(TestIF_File.i2DIDFormat==eIntel)                                     //JerryYang 20200422 Intel不支援2D要回覆N,N,...,N
        {
            strncpy(HHandler2Gpib.Message, AnsiString(fMain->tBarCodeList->CommaText).c_str(), sizeof(HHandler2Gpib.Message));   //AI(W906-GB-P2a) 20260926: proxy -> AnsiString(...) (rule 8)
        }
        else if(CosFunction.bAutoRetestGPIBmode==false)                         //jou 2015-10-02 Auto Retest GPIB mode
        {
            strncpy(HHandler2Gpib.Message, "", sizeof(HHandler2Gpib.Message));
        }
    }
    asRecordTestResult="";                                                      //jou 2013-05-14 修正Timer Out時，如果沒回傳Bin，會將上次的記錄秀出來

    pcp->cbData=sizeof(HHandler2Gpib);
    pcp->lpData=(unsigned char *)&HHandler2Gpib.iSendCommand;
    bExist=false;
    if(TestIF.iTestType==TCP_IP_MODE &&                                         //wei 20211027 open short TCP/IP
       LastSet.iTester==ON_LINE)                                                //Steven 20231113 : 修正TCP/IP Test跑Off Line
    {
        AnsiString str="", str2="";
        for(int i=0; i<MAX_SOCKET_TOTAL; i++)
        {
            if(bSite[31-i]==true)
              str+="1";
            else
              str+="0";
            if(i%8==7 && i!=31)
            {
                str+=",";
            }
        }

        if(IniConfig.bD22SupportMultiDoubleContact && bDoubleContact)           //Steven 20230221 : Add TCP IP for double contact
            str2.sprintf("RETEST %s;", str);
        else
            str2.sprintf("TEST %s;", str);

        if(bNeedTest)
        {
//#if 0 // TODO(W906-GB-P2a): G14 -- TfTesterTCP::SendTCPIPCommand is declared-not-defined on purpose (forms/fTesterTCP.h:382, GATE T-P1; port: TesterTCPSocket_SendTCPIPCommand, Interface/TesterTCP_Socket.h:281) -- golden main.cpp:19184   //AI(W906-S09-B3) 20260929: gate retired -- bound to TesterTCPSocket_SendTCPIPCommand (Interface/TesterTCP_Socket.h:281; already live in this file for the recipe change, :1123-1130); the laptop verifies the SOT on the machine (TCP_IP_MODE + ON_LINE); golden 906_0625_Steven main.cpp:18566
            TesterTCPSocket_SendTCPIPCommand(0, "SOT", str2);                   //AI(W906-S09-B3) 20260929: golden fTesterTCP->SendTCPIPCommand(0, "SOT", str2) -- no TfTesterTCP form in V906
//#endif   //AI(W906-S09-B3) 20260929: see :909
        }
    }
    else
    {
        SendToBridge(pcp);                                                      //AI(W906-GB-P2a) 20260926: rule 4, golden SendMessage(fMain->HVisionWnd, WM_COPYDATA,(WPARAM) NULL, (LPARAM)pcp);
    }
    delete pcp;
    return true;
}
//------------------------------------------------------------------------------
void THandlerTesterSide::CloseGpibProgram(AnsiString Src)
{
//    if(TestIF.iTestType==RS232_MODE)                                          //wei 20150703 因為測試機不能關閉      //Isaac 20200903 :TTL RS232通訊，Mark
//        return ;

    if(bFind==false)
        return ;

    if(LastSet.iRunStartMode==rsmQAMode && bQAModeFinishCleanOut==true)         //Steven 20141023 : QA做完後的Bin
        return;

    COPYDATASTRUCT *pcp=new COPYDATASTRUCT ;
    pcp->dwData=0;
    HHandler2Gpib.bCloseGpib=true;
    if(CUSTOMER_CODE==CC_MTI || CUSTOMER_CODE==CC_PTI)                          //Sam 20190429 : Add CC_PTI_NEWWORK
        HHandler2Gpib.iLotStatus = 0;                                           //ChungHung OLP 聚成專用

    HHandler2Gpib.iSendCommand=MSG_CMD_CloseGpib;                               //wei 20150408 Add Close GPIB Command   MSG_CMD_NONE-->MSG_CMD_CloseGpib
    pcp->cbData=sizeof(HHandler2Gpib);
    pcp->lpData=(unsigned char *)&HHandler2Gpib.iSendCommand;
    SendToBridge(pcp);                                                          //AI(W906-GB-P2a) 20260926: rule 4, golden SendMessage(HVisionWnd,WM_COPYDATA,(WPARAM) NULL, (LPARAM)pcp);
    delete pcp;
    HHandler2Gpib.bCloseGpib=false;                                             //kevin 20150819 off-line 一直關GPIB
    MyDBIProcess("Process", "GPIB Close - "+Src);
    WakeupGPIBdelay.SetSecAndOn(5);
    bFind=false;
}
//---------------------------------------------------------------------------
void THandlerTesterSide::InitialBarCodeList()                                   //Steven 20150713 : for 2D Code
{
    //AI(W906-GB-P2a) 20260926: golden TfMain member tBarCodeList -> fMain->tBarCodeList (forms/fMain.h:337).
    if(TestIF_File.i2DIDFormat==eIntel && TestIF_File.bEnableBarCode==false)    //JerryYang 20200422 Intel不支援2D要回覆N,N,...,N
    {
        fMain->tBarCodeList->Clear();
        for(int i=0; i<MAX_SOCKET_TOTAL; i++)
        {
            fMain->tBarCodeList->Add("N");
        }
    }
    else
    {
        if(fMain->tBarCodeList->Count==MAX_SOCKET_TOTAL)
        {
            for(int i=0; i<MAX_SOCKET_TOTAL; i++)
            {
                fMain->tBarCodeList->Strings[i]="0";
            }
        }
        else
        {
            fMain->tBarCodeList->Clear();
            for(int i=0; i<MAX_SOCKET_TOTAL; i++)
            {
                fMain->tBarCodeList->Add("0");
            }
        }
    }
}

//******************************************************************************
//  AI(W906-GB-P2f) 20260926: golden 912 main.cpp:22667-22735 TfMain::SendMessageToGpibProg and the Handler->bridge
//  settings sync of TfMain::Timer2Timer (main.cpp:22089-22234).  Neither was ported before P2f, so MSG_CMD_ChangeGpib
//  never reached a bridge: the GPIB engine's bSimulate / GPIB address / bin count (GpibHandlerMsg.cpp:238-...) stayed
//  at its own general.ini values.  Golden Timer2 runs this every tick after its fShow / InitialOK / SystemStart==false
//  guards (:21492-21499, :22080-22084); V906 calls SyncBridgeSettings() from W906_TesterCommTick under the same
//  InitialOK / SystemStart guards.  It is change-driven (the old* members), so running it every tick sends nothing
//  until something changes -- exactly as golden.
//  Gates:
//    F1 :22089-22115  the SPEA Interface.exe branch (_SendADDRToInterfaceProgram / _SendInformationToInterfaceProgram;
//                     plan: SPEA, like G2/G6/G7/G10) -- its two resets (OSRecipe, b2DID) stay live.
//    [LIFTED] F2 :22199  hanaART->DoRunHanaART -- expanded in place to its golden 2-line body (HANA_ART.cpp:108-112).
//  Other deviations:
//    * golden fTesterTCP->OSRecipe (a TfTesterTCP member; V906 has no fTesterTCP form) is a function-local static.
//    * fTesterTCP->CopyRecipeToTester / SendTCPIPCommand -> the V906 free functions TesterTCP_CopyRecipeToTester
//      (Interface/TesterTCP.h) / TesterTCPSocket_SendTCPIPCommand (Interface/TesterTCP_Socket.h).
//    * P2d user ruling (Off-Line: GPIB way for RS232 / TTL recipes, see OffLineGpibWay): SendMessageToGpibProg fills
//      bSupport32Bin / GpibAddress the GPIB way, and the "tester mode changed" arms use the GPIB arm.
//******************************************************************************
void THandlerTesterSide::SendMessageToGpibProg()
{
    if(TestIF_File.iTestType==TCP_IP_MODE &&                                    //wei 20211027 open short TCP/IP
       LastSet.iTester==ON_LINE)                                                //Steven 20231113 : 修正TCP/IP Test跑Off Line
    {
    }
    else
    {
        if(bFind==false)
            return;
    }
    bEcho=false;
    bExist=false;
    COPYDATASTRUCT *pcp=new COPYDATASTRUCT ;
    pcp->dwData=0;

    HHandler2Gpib.iSendCommand=MSG_CMD_ChangeGpib;                              //wei 20150409 Add Close GPIB Command   MSG_CMD_NONE-->MSG_CMD_ChangeGpib
    HHandler2Gpib.bCloseGpib=false;                                             //kevin 20150819 off-line 一直關GPIB
    if(CUSTOMER_CODE==CC_MTI || CUSTOMER_CODE==CC_PTI)                          //Sam 20190429 : Add CC_PTI_NEWWORK
        HHandler2Gpib.iLotStatus=0;                                             //ChungHung OLP 聚成專用

    HHandler2Gpib.bTimeOutProcess=false;
    if(TestIF_File.iTestType==GPIB_MODE  || OffLineGpibWay() ||                 //AI(W906-GB-P2f) 20260926: P2d Off-Line rule -- GPIB fields
       (TestIF_File.iTestType==RS232_MODE &&
        TestIF_File.iRs232Mode==eRs23232Bin))
    {
        HHandler2Gpib.bSupport32Bin=true;                                       //Steven 20121112 : RS232支援32Bin b8080 --> bSupport32Bin
    }
    else
    {
        HHandler2Gpib.bSupport32Bin=false;
    }

    if(!OffLineGpibWay() &&                                                     //AI(W906-GB-P2f) 20260926: as above
       TestIF_File.iTestType==RS232_MODE &&
       TestIF_File.iRs232Mode==eRs23232Bin)                                     //Steven 20121116 : RS232回傳的最大Bin數
    {
        HHandler2Gpib.GpibAddress=TestIF_File.iRs232MaxBinCount+1;
    }
    else
    {
        HHandler2Gpib.GpibAddress=TestIF_File.iGpibAddress;
    }

    if(LastSet.iTester==ON_LINE && TestIF_File.iTestType==GPIB_MODE)            //Steven 20150713 : 整理LastSet.iTester
        bGpibMode=true;
    else
        bGpibMode=false;

    HHandler2Gpib.bSimulate=(LastSet.iTester==OFF_LINE);                        //JerryYang 20230322 : 新增2DID模式, != online改為 ==OFF_LINE

    HHandler2Gpib.bGpibMode=bGpibMode;
    for(int i=0; i<32; i++)
        HHandler2Gpib.Site[i]=0;

    HHandler2Gpib.IsTest=false;
    HHandler2Gpib.GPIBBin=iTestBinCount;                                        //kevin 20140317 add 256Bin

    HHandler2Gpib.HandlerHwnd=HandlerWndToken();                                //AI(W906-GB-P2f) 20260926: rule 4, golden this->Handle
    HHandler2Gpib.GpibHwnd=HVisionWnd;

    pcp->cbData=sizeof(HHandler2Gpib);
    pcp->lpData=(unsigned char *)&HHandler2Gpib.iSendCommand;
    bExist=false;
    SendToBridge(pcp);                                                          //AI(W906-GB-P2f) 20260926: rule 4, golden SendMessage(HVisionWnd, WM_COPYDATA, (WPARAM)NULL, (LPARAM)pcp)
    delete pcp;
    fMain->oldGpibAddress=TestIF_File.iGpibAddress;                             //AI(W906-GB-P2f) 20260926: golden TfMain member -> fMain-> (forms/fMain.h:254)
    oldLastSetiTester=LastSet.iTester;
    oldLastiTestBinCount=iTestBinCount;                                         //kevin 20140305 256bin 16->256 通知gpib
}

//------------------------------------------------------------------------------
// golden 912 main.cpp:22089-22234 (inside TfMain::Timer2Timer) -- see the banner above SendMessageToGpibProg.
void THandlerTesterSide::SyncBridgeSettings()
{
    static int  OldiTestModeSelect=-1;                                          // golden Timer2Timer static :21484
    static bool bOpenSet=true;                                                  // :21485
    static bool b2DID=false;                                                    // :21487  //Steven 20230911 : 2DID Function for OS Tester
    static AnsiString sOSRecipe="";                                             //AI(W906-GB-P2f) 20260926: golden fTesterTCP->OSRecipe (no fTesterTCP in V906)
    AnsiString Str;

    if(TestIF_File.iGpibMode==InterfaceType_SPEA_Type)
    {
#if 0 // TODO(W906-GB-P2f): F1 SPEA Interface.exe is not the tester bridge (plan: SPEA, same as G2/G6/G7/G10) -- golden main.cpp:22091-22111
        if(TestSocket.UseSiteHasIC()==false)
        {
            if(fMain->oldGpibAddress!=TestIF_File.iGpibAddress)
            {
                _SendADDRToInterfaceProgram(TestIF_File.iGpibAddress);
                fMain->oldGpibAddress=TestIF_File.iGpibAddress;                 //Steven 20141202 : Fixed 2D Code Function
            }
        }

        if(TestIF_File.iTestMode!=OldiTestModeSelect)
        {
            int iDataIn=TestIF_File.iTestMode;
            _SendInformationToInterfaceProgram(TYPE_HANDLER_GPIB, CommandType_INFSEND, CommandType_INFSEND_SENDTESTMODE, &iDataIn, 1);
            OldiTestModeSelect=TestIF_File.iTestMode;
        }

        if(bOpenSet)
        {
            bOpenSet=false;
            _SendInformationToInterfaceProgram(TYPE_HANDLER_GPIB, CommandType_INFSEND, CommandType_INFSEND_SENDBINPASSDEF, Prod.bIsPassBin, sizeof(Prod.bIsPassBin));
        }
#else
        (void)OldiTestModeSelect;
        (void)bOpenSet;
#endif

        sOSRecipe="";                                                           //Steven 20230116 : OS測試機傳送工作檔名
        b2DID  =false;
    }
    else if(TestIF_File.iTestType==TCP_IP_MODE &&                               //Steven 20230116 : OS測試機傳送工作檔名
            LastSet.iTester==ON_LINE)                                           //Steven 20231113 : 修正TCP/IP Test跑Off Line
    {
        if(sOSRecipe!=fMain->cbSetupFileName->Text ||
           b2DID!=TestIF_File.bEnableBarCode)
        {
            sOSRecipe=fMain->cbSetupFileName->Text;
            TesterTCP_CopyRecipeToTester(sOSRecipe);                            //Steven 20250612 : for OS Tester.
            MySleep(100);
            Str.sprintf("WORKFILE,%s,", sOSRecipe.c_str());
            TesterTCPSocket_SendTCPIPCommand(0, "Change Recipe", Str);
            MySleep(100);
            TesterTCPSocket_SendTCPIPCommand(0, "Get OS Setup", "GETOSSETUP");  //Steven 20230505 : 取得OS Tester資訊
            MySleep(100);
            if(TestIF_File.bEnableBarCode)                                      //Steven 20230911 : 2DID Function for OS Tester
                TesterTCPSocket_SendTCPIPCommand(0, "Barcode Function", "SET2DID,1");
            else
                TesterTCPSocket_SendTCPIPCommand(0, "Barcode Function", "SET2DID,0");
            MySleep(100);
            b2DID  =TestIF_File.bEnableBarCode;
        }
    }
    else
    {
        sOSRecipe="";                                                           //Steven 20230116 : OS測試機傳送工作檔名
        b2DID  =false;

        if((fMain->oldGpibAddress!=TestIF_File.iGpibAddress &&
            TestIF_File.iTestType==GPIB_MODE) ||
            LastSet.iTester!=oldLastSetiTester ||
            oldLastiTestBinCount!=iTestBinCount ||                              //kevin 20140317 256bin 16->256 通知gpib
            oldbA10_6!=IniConfig.bA10_6_HANA_ART_TestMode_Enable)               //Steven 20250414 : HANA ART Function
        {
            if(oldLastiTestBinCount!=iTestBinCount)                             //kevin 20140317 256bin 16->256 通知gpib
            {
                fShowBinSelect->PageControl1Change(nullptr);                    //AI(W906-GB-P2f) 20260926: golden passes fShowBinSelect; the port's TfShowBinSelect is not a TObject and the body ignores Sender (cShowBinSelect.cpp:1144, same nullptr as :1086/:1562/:2170)
                fShowBinSelect->ShowBinSel();
            }

            if(TestSocket.UseSiteHasIC()==false)                                //JerryYang 20200214 測區有IC不重送 Message
            {
                SendMessageToGpibProg();
                if(INSTALL_OCR!=eocrUninstal ||                                 //kevin 20160919
                   BAR_CODE_INSTALL!=ebctUninstall)
                {
                    if(TestIF_File.bOcrFunction ||
                       TestIF_File.bEnableBarCode)
                        SendMSG_CMD(MSG_CMD_EnableBarCode);                     //AI(W906-GB-P2f) 20260926: golden fMain->SendMSG_CMD -> this object (same body)
                    else
                        SendMSG_CMD(MSG_CMD_DisableBarCode);

                    if(TestIF_File.b2DUsePinInspection)                         //Ifor 20240528 add:Pin1 Function
                        SendMSG_CMD(MSG_CMD_EnablePin1Function);
                    else
                        SendMSG_CMD(MSG_CMD_DisablePin1Function);
                }

                if(TestIF_File.i2DIDFormat==eAMD)                               //JerryYang 20200422 2DID format選項改用下拉選單
                {
                    HHandler2Gpib.iStatus[0]=eAMD;
                }
                else if(TestIF_File.i2DIDFormat==eIntel)
                {
                    HHandler2Gpib.iStatus[0]=eIntel;
                }
                else
                {
                    HHandler2Gpib.iStatus[0]=eStandard;
                }

                if(TestIF_File.bEnableMulti2D)
                {
                    HHandler2Gpib.iStatus[1]=1;
                    HHandler2Gpib.iStatus[2]=TestIF_File.iMulti2DType;
                }
                else
                {
                    HHandler2Gpib.iStatus[1]=0;
                }
                SendMSG_CMD(MSG_CMD_2DIDFormat);

                oldbA10_6=IniConfig.bA10_6_HANA_ART_TestMode_Enable;            //Steven 20250414 : HANA ART Function

                //AI(W906-GB-P2f) 20260926: F2 lifted -- golden `hanaART->DoRunHanaART(bA10_6)` (main.cpp:22199).  fMain->hanaART is the
                //   TfMainHanaART facade (no instance of the real uHANA_ART exists), so the callee's golden body is expanded here
                //   verbatim (912 Automation/HANA_ART.cpp:108-112): iLotStatus=(bRun)?1:0; SendMSG_CMD(MSG_CMD_RUN_HANA_ART).
                HHandler2Gpib.iLotStatus=(IniConfig.bA10_6_HANA_ART_TestMode_Enable)?1:0;
                SendMSG_CMD(MSG_CMD_RUN_HANA_ART);

                if(CosFunction.bFTPFunction)                                    //Ifor 20231101 add:FTP Function
                {
                    if(IniConfig.bEnableFTP==true)
                    {
                        SendMSG_CMD(MSG_CMD_EnableFTPFunction);
                    }
                    else
                    {
                        SendMSG_CMD(MSG_CMD_DisableFTPFunction);
                    }
                }
            }
        }
        else if(!OffLineGpibWay() &&                                            //AI(W906-GB-P2f) 20260926: P2d Off-Line rule -- RS232 / TTL recipes Off-Line use the GPIB arm below
                TestIF_File.iTestType==RS232_MODE &&
                oldLastiTestMode!=TestIF_File.iRs232Mode)                       //Steven 20161122 (Jou) : Add 16 bin GS and 32 bin GS
        {
            SendMSG_TestMode();
            oldLastiTestMode=CheckAndReadIniData("D:\\RS232Standard\\system\\Setup.ini", "SystemSetup", "iTesterMode",  InterfaceType_Standard);
        }
        else if(!OffLineGpibWay() &&                                            //AI(W906-GB-P2f) 20260926: as above
                TestIF_File.iTestType==TTL_MODE &&
                (TTL_CARD_TYPE==2 || TTL_CARD_TYPE==3) &&
                (oldLastiTestMode!=(TestIF_File.iDioMode+InterfaceType_TTL)) &&                                         //Isaac 20200903 :TTL RS232通訊
                 W906_FormFShow("FTestIF", FTestIF->fShow==true)==false)       //Isaac 20211115 : 修正因檢查handler和RS232視窗是否開啟而自動關閉RS232視窗   //AI(W906-GB-P2f) 20260926: golden `FTestIF->fShow==false`; the member is never maintained in the web build, so the window registry is OR-ed in (csystem.h W906_FormFShow; "FTestIF" is not reported by background.html yet -> same answer as the member today)
        {
            SendMSG_TestMode();
            oldLastiTestMode=CheckAndReadIniData("D:\\RS232Standard\\system\\Setup.ini", "SystemSetup", "iTesterMode",  InterfaceType_TTL);
        }
        else if((TestIF_File.iTestType==GPIB_MODE || OffLineGpibWay()) &&       //AI(W906-GB-P2f) 20260926: as above
                oldLastiTestMode!=TestIF_File.iGpibMode)                        //Steven 20161122 (Jou) : Add 16 bin GS and 32 bin GS
        {
            SendMSG_TestMode();
            oldLastiTestMode=CheckAndReadIniData("D:\\GPIB9045\\system\\general.ini", "SystemSetup", "iTesterMode",  InterfaceType_ADVAN_Type1);
        }
    }
}
