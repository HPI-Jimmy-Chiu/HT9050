// =============================================================================
//  WebMainScanKey.cpp -- golden TfMain::ScanKey (the physical front / rear panel keys) with its timer, and TfHome::ScanKey.
//
//  AI(W906-SCANKEY) 20261003: EastSun (machine engineer) 「現在移植」 -- the panel keys (HOME / PAUSE / RESET / START /
//  ONE CYCLE / RETRY ...) did nothing because nothing in this tree ever called the key scan outside the dialog waits.
//  golden 906 = D:\HT9045\HT9011UC_Code_V3.33.906.0_20260618 (cp950, read-only):
//    TfMain::ScanKey            main.cpp:2379-2656  (only caller TimerScanKeyTimer :31994-32025; main.dfm:17339-17345 Interval 30)
//    TfHome::ScanKey            uhome.cpp:4874-4883 (Timer1Timer :4885-4889; uhome.dfm:357-362 Interval 10)
//  wb_serve only (CMakeLists.txt add_executable(wb_serve)): it calls FileRW/DeviceForm_File.cpp (wb_serve only) and the
//  form-bridge lock. Called once per pass of wb_serve's main loop (tools/wb_serve.cpp, after W906_MainCtlButtonTick, and
//  the drag keepalive W906_NativeKeepaliveMain).
//  Key source = ScanPannelKey (ckernel.cpp:3147): one id per press (press latch :3732-3757), so a held key fires once.
//  ⚠ It reads Sen[SnFK*] / Sen[SnRK*]: a key row of IO_Table.csv with Enable=0 never reads ON (mysensor.cpp:127-131), so
//    until those rows carry the panel's real 1203 address this file is inert (machine config, not code).
//  Not run inside the blocking-dialog waits: golden returns at :2383-2386 while fNote / MyMessageBox is up, and those
//  waits scan the keys themselves (tools/wb_serve.cpp W906MbIoDismiss / W906_AlarmIoAnswer / W906_ModalWaitTick).
//  START goes through W906_RemoteRunStart (forms/fMain.h, tools/wb_serve.cpp -> TfMainWeb::StartFromWeb), never the base
//  TfMain::Start (WebStart.h). START_SitesCensus: +1 live site (tests/CMakeLists.txt --check).
//  No FormLock around START / PAUSE / HOME / RESET (they can wait for the browser, FileRW/DeviceForm_File.cpp:676-680);
//  FormLock around CleanOut / OneCycle / TrayFeed, as WebMainCtlButtons.cpp.
// =============================================================================
#include "MachineType.h"            // first (tools/macro_order_gate.ps1)
#include "forms/fMain.h"            // fMain, BtnPauseClick / Home / BtnResetClick / BtnCleanOutClick / BtnOneCycleClick / InitialTrayFeedTask, W906_RemoteRunStart
#include "forms/fHome.h"            // fHome: fShow / Close / sbAbortHomeClick / GaliMotorServoOff
#include "forms/fNote.h"            // fNote
#include "forms/fSortCT.h"          // fSortCT->pnlLoad
#include "atester_shims.h"          // fContact (TfContactShim::fShow), fiosetview
#include "mymessbox_shim.h"         // MyMessageBox
#include "aHotPlateSubstrate.h"     // InArmSuck / OutArmSuck / FTestSuck / BTestSuck / CatchTraySuck
#include "cmydef.h"                 // Sn*/Sw* ids, flags, AccessLevel, iDefSupervisorLevel, OFFLINE_ALARM, OFF_LINE, MMTrayY, C_Empty_Fix/C_Color_Fix, K_RETRY, MMSystem/MMAutoClean
#include "cprod.h"                  // TestIF_File, ArmSpeed_File, Prod
#include "CosFunction.h"
#include "Config.h"
#include "LastSet.h"
#include "csystem.h"                // InitTrayEndFunction, AutoTrayCylinderFree, IndexHasIC / ShuttleHasIC
#include "Motor/mymotor.h"          // MOT[]
#include "mycylin.h"                // Cylinder[]
#include "mysensor.h"               // Sen[] -- the [W906] first-scan guard (AI(W906-ST02-SKC) 20261003 (St02-E))
#include "myswitch.h"               // SW[]
#include "canary_support.h"         // ShowErrorMessage, ShowMyMessage, __FUNC__
#include "SECSGEM/SecsEventType.h"  // SECS_EVENT
#include "SECSGEM/SecsEventReport.h"// EventReport
#include "MainCalcCore.h"           // ComputeCheckAutoOnlySetOneBin / ComputeCheckAuto1OnlyBin1
#include "W906FormShowing.h"        // W906_FormShowing
#include "JsonBridge/FormJson.h"    // ht9045::formjson::FormLock / FormUnlock
#include <cstdio>

void NewRecordProcess(AnsiString AlarmCode, AnsiString S, AnsiString Debug);   // cMyDB.h:129 (not included: its defaults clash with canary_support.h's RecordProcess / MyDBIProcessNew)
int  ScanPannelKey();                                    // ckernel.h (local declaration, same convention as tools/wb_serve.cpp)
extern bool SECS_GEM_PPMUSIC_CONTROL_flag;               // ckernel.cpp
extern bool SECS_GEM_PPSIGNALTOWER_CONTROL_flag;         // ckernel.cpp
extern bool W906_HomeTimer1Enabled;                      // uhome.cpp (golden TfHome::Timer1->Enabled)
void W906_MsgBoxModelessPanelKeyTick();                  // tools/wb_serve.cpp EOF: the panel keys of a non-modal MyMessageBox (NB2 R188 M2) -- AI(W906-ST02-SKC) 20261003 (St02-E)
void W906_Contact_OneCycleProcess();                     // FileRW/DeviceForm_File.cpp EOF (golden TfContact::OneCycleProcess cContact.cpp:11741-11746)

namespace {
struct MskFormLock { MskFormLock() { ht9045::formjson::FormLock(); } ~MskFormLock() { ht9045::formjson::FormUnlock(); } };

bool s_bOneCycleFinish = false;   // golden TfContact::bOneCycleFinish (ctor false)

// golden TfContact::FormShow cContact.cpp:1192-1206 sets bOneCycleFinish when the Contact form opens.
// [W906] Evaluated on the page's closed->open edge (page table), at most one pass (<=50 ms) after golden's moment.
void MskContactFormShowEdge()
{
    static bool wasOpen = false;
    const bool open = fContact != 0 && W906_FormShowing("fContact", fContact->fShow);
    if (open && !wasOpen)
    {
        if(IndexHasIC() || InArmSuck.HasIC() || OutArmSuck.HasIC() || ShuttleHasIC())   // golden :1192-1195
            s_bOneCycleFinish=false;                                                     // golden :1198
        else
            s_bOneCycleFinish=true;                                                      // golden :1204
    }
    wasOpen = open;
}

// golden ProcessKeyFlush main.cpp:4107-4111 / :4150 (BtnHome) and :4109 / :4117-4119 / :4144 (BtnCleanOut).
// Same rule as FileRW/MainClick.cpp CtlBtnEnabled (anonymous there). BtnOneCycle->Enabled has no writer in golden (dfm default True).
bool MskBtnHomeEnabled()     { return SystemStart==false; }
bool MskBtnCleanOutEnabled() { return SystemStart==false && fAllMotorHome==true && iOneCycle==0; }

// golden TfMain::CheckAutoOnlySetOneBin main.cpp:32523-32553 (logic MainCalcCore; dialog golden :32544). Unconditional:
// golden ScanKey calls it in ship builds (the copies in tools/wb_serve.cpp / FileRW/DeviceForm_File.cpp are SOFT_SIMULTE-only).
bool MskCheckAutoOnlySetOneBin()
{
    AnsiString str1, str2;
    if(ComputeCheckAutoOnlySetOneBin(CosFunction.bUsePassBinOnlyCanSetOneBin, Prod.iIsPassT6, Prod.iT6CatData, iTestBinCount, s6TrayName, str1, str2))
    {
        ShowMyMessage(str1, str2, "", false, false);
        return true;
    }
    return false;
}
// golden TfMain::CheckAuto1OnlyBin1 main.cpp:32502-32521 (dialogs :32511 / :32516)
bool MskCheckAuto1OnlyBin1()
{
    AnsiString msg;
    if(ComputeCheckAuto1OnlyBin1(CUSTOMER_CODE, LastSet.iTester, Prod.iT6PosCate, iTestBinCount, msg))
    {
        ShowMyMessage(msg, "", "", false, false);
        return true;
    }
    return false;
}
// golden fAutoTeach->IsKeyStartEnable() AutoTeach/AutoTeach.cpp:1541-1552: true unless IsRun() (:217-224, false whenever
// !(bManualSteplAutoTeach && bA56EnableAutoTeachFunciton)) and in step Home / In/Out Shuttle Sensor / Index Arm.
// No fAutoTeach in the port (WebStart.cpp): with auto-alignment switched on the step is unknown -> fail closed.
bool MskAutoTeachKeyStartEnable()
{
    if(CosFunction.bManualSteplAutoTeach==false || IniConfig.bA56EnableAutoTeachFunciton==false)
        return true;
    std::printf("[SCANKEY] START refused: golden fAutoTeach->IsKeyStartEnable() (main.cpp:2504) -- auto alignment is on and TfAutoTeach is not ported\n");
    return false;
}
// AI(W906-ST02-SKC) 20261003 (St02-E): [W906] FIRST-SCAN GUARD, HOME and START only (St02-M 13:4x, S-20 review M3; NB2 R188 comparison).
//   Golden ScanPannelKey's bK[] latch starts false (golden 0618 ckernel.cpp:1940-1945) and is cleared only when the input reads
//   OFF (:2395-2404), so a key whose input is already ON when the machine comes up fires ONCE (:2385-2388) -- no OFF->ON edge is
//   needed.  A mis-mapped HT9050 IO table, or an unwired input that reads ON, must not HOME or START the machine by itself.  So:
//   from SystemInitialOK on (the first moment ScanPannelKey reads the pad), each START / HOME input (front and rear) is armed the
//   first time it is seen OFF; a START / HOME whose own input (golden's pad choice, :2381-2383) has never been seen OFF is
//   ignored with a log line.  Golden's bK[] latch then holds it until it reads OFF, so the next real press works as golden.
//   The host flags (bAseStart / bAseHome) pass unless that input is stuck ON since boot.  Every other key (RESET, ONE CYCLE,
//   ...) keeps golden's fire-once-if-held.  Safety-only deviation, HUMAN_REVIEW.
bool s_fsSeenOff[4]={false, false, false, false};                              // SnFKStart, SnRKStart, SnFKHome, SnRKHome
void MskFirstScanSample()
{
    if(SystemInitialOK==false)                                                  // ScanPannelKey returns -1 before it
        return;
    const int ids[4]={SnFKStart, SnRKStart, SnFKHome, SnRKHome};
    for(int i=0; i<4; i++)
        if(Sen[ids[i]].IsOn()==false)
            s_fsSeenOff[i]=true;
}
bool MskFirstScanHeld(int key)
{
    if(key!=SnFKStart && key!=SnFKHome)
        return false;
    const int i=(key==SnFKStart ? 0 : 2) + (bFrontPadActive ? 0 : 1);         // golden's pad choice (ckernel.cpp 0618 :2381-2383)
    const int ids[4]={SnFKStart, SnRKStart, SnFKHome, SnRKHome};
    return s_fsSeenOff[i]==false && Sen[ids[i]].IsOn();
}
}  // namespace

// AI(W906-ST02-SKC) 20261003 (St02-E): test hook for the [W906] first-scan guard: back to the boot state (no START / HOME input seen OFF yet).  Tests only.
void W906_ScanKeyFirstScanRearm_St02()
{
    for(int i=0; i<4; i++)
        s_fsSeenOff[i]=false;
}

// golden TfMain::ScanKey main.cpp:2379-2656. scan = ScanPannelKey in production (a ctest feeds keys);
// noticeUp = a kcode==0 note is up (golden: fNote->fShow; the port's notice does not set it, tools/wb_serve.cpp).
void W906_MainScanKeyWith(int (*scan)(), bool noticeUp)
{
    static int Key=0;                                                                       // :2381

    MskFirstScanSample();                                                                   // AI(W906-ST02-SKC) 20261003 (St02-E): [W906] first-scan guard bookkeeping (above)
    if((fNote && W906_FormShowing("fNote", fNote->fShow)) || noticeUp ||
       (fiosetview && W906_FormShowing("fiosetview", fiosetview->fShow)))                   // :2383
        return;
    if(MyMessageBox && W906_FormShowing("MyMessageBox", MyMessageBox->fShow))               // :2385
    {
        W906_MsgBoxModelessPanelKeyTick();                                                  // AI(W906-ST02-SKC) 20261003 (St02-E): [W906] NB2 R188 M2 -- golden's box has its
        return;                                                                             //   own Timer1Timer (mymessbox.cpp:540-663); a non-modal one has no other reader
    }
    if((Key=scan())==-1 && iPauseBackUp==-1)                                                // :2387
        return;

    if(MskFirstScanHeld(Key))                                                               // AI(W906-ST02-SKC) 20261003 (St02-E): [W906] FIRST-SCAN GUARD (above)
    {
        std::printf("[SCANKEY] panel key %d ignored: its input has been ON since boot ([W906] first-scan guard -- release it and press again)\n", Key);
        std::fflush(stdout);
        return;
    }

    if(Key==SnFKStart)                                                                      // :2390
    {
        if(fContact && W906_FormShowing("fContact", fContact->fShow) && s_bOneCycleFinish==false)   //jou 980615 one cycle finish after can auto high or contact test
        {
            ShowErrorMessage("MES1645", 0, MMSystem, false, "ScanKey");         //Must finish [One Cycle]
            return;
        }

        //==> Eastsun 20260527 整合#028.AAL.P-rev16 ScanKey AAL flag setup :KYEC
        if(CosFunction.bUseBarcodeAutoAdjustLight==true &&
           TestIF_File.bUseBarcodeAutoAdjustLight==true &&
           TestIF_File.bEnableBarCode==true             &&
           fMain->cbRunStartMode->Text.Pos("Initial")!=0)                       //Ifor 20210408 add:Barcode 自動調整光源
        {
            bStartAutoAdjustLight=true;
            for(int i=0; i<4; i++)
                bBarcodeNeedAutoAdjust[i]=true;
            if(InArmSuck.iShtRow==1)
            {
                bBarcodeNeedAutoAdjust[1]=false;
                bBarcodeNeedAutoAdjust[2]=false;
            }
            if(TestIF_File.iShuttleMode==1)
            {
                if(TestIF_File.iShuttle_Sel==0)         //Front Arm Only
                {
                    bBarcodeNeedAutoAdjust[2]=false;
                    bBarcodeNeedAutoAdjust[3]=false;
                }
                else if(TestIF_File.iShuttle_Sel==1)    //Rear Arm Only
                {
                    bBarcodeNeedAutoAdjust[0]=false;
                    bBarcodeNeedAutoAdjust[1]=false;
                }
            }
        }
        //<== Eastsun 20260527 整合#028.AAL.P-rev16

        if(IniConfig.bEnableAutoCleanFunction && TestIF_File.iAutoClean_Function)           // :2432
        {
            if(TestIF_File.iAutoClean_Tray==eCKPos_CleanKit)                    //jou 2012-07-31 Clean kit使用shuttle 1
            {
                if(TestIF_File.iShuttleMode==1 && TestIF_File.iShuttle_Sel==1 && TestIF_File.dSiteYPitch>6350 && IniConfig.bE43AutoCleanUseHotplate==false)
                {
                    if(bUse8Picker)                                             //Alick 20170223 (wei) add for 9046LS AutoClean use
                    {
                    }
                    else
                    {
                        ShowErrorMessage("WAR16102", K_RETRY, MMAutoClean, false, "ScanKey");   //AutoClean Must Use ARM1
                        return;
                    }
                }
            }
            else
            {
                if(TestIF_File.iShuttleMode==1 && TestIF_File.iShuttle_Sel==0)  //kevin 20120518使用AUTOCLEAN 但只使用arm1要發alarm警告
                {
                    if(IniConfig.bD58UseArm1PickPlaceArm2Test==true && TestIF_File.bArm1PickPlaceArm2Test==true)   //Ifor 20200811 Fix: Arm1 Pick Place Arm2Test 需卡兩個條件
                        ShowErrorMessage("WAR16105", K_RETRY, MMAutoClean, false, "ScanKey");   //20150127 不能使用autoclean
                    else
                        ShowErrorMessage("WAR16103", K_RETRY, MMAutoClean, false, "ScanKey");   //AutoClean Must Use ARM2
                    return;
                }
            }
        }

        if(OFFLINE_ALARM)                                                       //jou 2014-03-28 SPIL Handler  On-line & Offline Switch Flow
        {
            if((IniConfig.bSPILFunction==true) && LastSet.iTester==OFF_LINE)    //JerryYang 20170328 (Jou) 矽品客戶碼統一用SPILFunction
            {
                if(AccessLevel<iDefSupervisorLevel)                             //Operator mode 狀態 offline按start 不可running。
                {
                    ShowMyMessage("Off line Call H/W Check!","離線模式需請工程師確認!");
                    return;
                }
                else if(SystemStart==false && LastSet.bNeedSetupTeach==false)   //offline 狀態按start ，需出現提醒視窗，並再次輸入 ID/Password (supervisor mode) 。
                {
#if 0 // GATE (W906-SCANKEY-RELOGIN): golden :2476-2484 makes the supervisor log in again (cbUserSelectChange -- not a TfMain member, atester.cpp:4199); a panel key cannot open the web login box
                    fPassword->Label5->Visible=true;
                    fPassword->Label6->Visible=true;
                    btLogin->Caption="Logout";
                    cbUserSelectChange(this);
                    TemperatureEditDisable();
                    fPassword->Label5->Visible=false;
                    fPassword->Label6->Visible=false;
                    if(AccessLevel<iDefSupervisorLevel)
                    {
                        return;
                    }
#endif
                    // [W906] fail closed: golden's next step is the login box; without it, no START (same posture as FileRW/LotInfo_SECSLotStart.cpp GATE LI1-1)
                    RecordProcess("W906 SCANKEY: panel START refused -- SPIL offline START needs the supervisor login (golden main.cpp:2474-2490), not available from a panel key; START from the screen");
                    return;
                }
            }
        }

        if(MskCheckAutoOnlySetOneBin())                                                     // :2494
            return;
        if(IniConfig.bP28Auto1OnlyBin1==true && MskCheckAuto1OnlyBin1())                    // :2499 Ifor 20171017 P28 功能整理
            return;
        if(MskAutoTeachKeyStartEnable()==false)                                             // :2504 JimmyChiu 20211229
            return;
        //==> Eastsun 20260520 整合
        if(IniConfig.bI52_bAQLSortMode==true)                                   //Ifor 20210322 add:New AQL
        {
#if 0 // GATE (W906-SCANKEY-AQL) golden main.cpp:2511 fLotInfo->SetAQLMode() -- forms/fLotInfo.h has no SetAQLMode (same gap as FileRW/DeviceForm_File.cpp, SECSGEM G39)
            fLotInfo->SetAQLMode();
#endif
            std::printf("[SCANKEY] TODO golden main.cpp:2511 fLotInfo->SetAQLMode() not done ([I52] AQL sort mode)\n");
        }
        //<== Eastsun 20260520 整合
        if(fHome && fHome->fShow==false)                                                    // :2514 Steven 20111222   AI(W906-ST02-SKC) 20261003 (St02-E): the member, as golden 0618 main.cpp:2514 and R142=A (NB2 R188 M1: the page table says "showing" whenever any browser has Home Monitor open, so panel START did nothing)
        {
            if(!W906_RemoteRunStart("ScanKey_2"))                                           // :2516 golden Start("ScanKey_2") -> TfMainWeb::StartFromWeb (the start.run landing; manual teach refuses)
                std::printf("[SCANKEY] PANEL START refused (W906_RemoteRun not installed, or the START interlock: a 'TCP HTSET,333 REFUSED' line just above is the shared seat's text for THIS panel press)\n");   // AI(W906-ST02-SKC) 20261003 (St02-E) (NB2 R188 low)
        }
    }
    else if(Key==SnFKPause && IniConfig.bFinishSuckAfterPause==false)                       // :2519
    {
        NewRecordProcess("MES2111", "PAUSE pressed", "ScanKey_1");
        fMain->BtnPauseClick(fMain);                                                        // -> Pause -> W906_RemoteRunPause -> PauseFromWeb
        // AI(W906-ST02-SKC) 20261003 (St02-E): golden 0618 :2524-2525 `if(CosFunction.bOEEFunction) fProductionInfo->ClickPause();` NOT translated -- V912's fix kept
        //   (RULINGS_20261003 #1, NB2 R188 low): V912 main.cpp:2580-2582 comments it out (CASE-20260915-002: BtnPauseClick, golden 0618
        //   main.cpp:6969-6970, already calls ClickPause, so the panel PAUSE popped the Greatek OEE pause box twice).  (Was MC01's
        //   GATE W906-SCANKEY-OEE "not in forms/fProductionInfo.h"; the runtime is the same -- nothing calls ClickPause here.)
//      if(CosFunction.bOEEFunction)
//          fProductionInfo->ClickPause();
    }
    else if((Key==SnFKPause || iPauseBackUp!=-1) &&
            IniConfig.bFinishSuckAfterPause==true)                              //Hung 20110901 : finish suck and destroy 後才暫停
    {
        if(InArmSuck.IsPickSuckFinish()==true && InArmSuck.IsPickDestroyFinish()==true   &&
           OutArmSuck.IsPickSuckFinish()==true && OutArmSuck.IsPickDestroyFinish()==true &&
           FTestSuck.IsShtSuckFinish()==true && FTestSuck.IsShtDestroyFinish()==true   &&
           BTestSuck.IsShtSuckFinish()==true && BTestSuck.IsShtDestroyFinish()==true   &&
           CatchTraySuck.IsShtSuckFinish()==true && CatchTraySuck.IsShtDestroyFinish()==true)
        {
            NewRecordProcess("MES2111", "PAUSE pressed", "ScanKey_2");
            fMain->BtnPauseClick(fMain);
            iPauseBackUp=-1;
        }
        else
        {
            iPauseBackUp=Key;                                                   // GOLDEN QUIRK KEPT: a later pass with no key writes -1 here
        }
    }
    else if(Key==SnFKHome && MskBtnHomeEnabled())                                          // :2545 BtnHome->Enabled
    {
        if(IniConfig.bG06HomeinitialCheckZ1 &&bHomeUnlock==false)               //kevin 20131218
            bHomeinitialCheckPushZ1=true;
        if(MskCheckAutoOnlySetOneBin())
             return;
        if(IniConfig.bP28Auto1OnlyBin1==true && MskCheckAuto1OnlyBin1())           //Ifor 20171017 P28 功能整理
            return;
        bHomeByStart=false;
        fMain->Home("ScanKey");                                                             // forms/fMain.cpp TfMain::Home
    }
    else if(Key==SnFKReset)                                                                 // :2565
    {
        if(bRunAutoClean)
            return;                                                             //Steven 20120208
        if(SystemStart)
            return;
        if(IniConfig.bSPILFunction==true || IniConfig.bA17RESETButtonDisable)   //kevin 20180711
            return;
        NewRecordProcess("MES2113", "RESET pressed", "ScanKey");
        fMain->BtnResetClick(fMain);                                                        // golden Reset main.cpp:7235 NOT wired: forms/fMain.cpp no-op (WebMainCtlButtons.cpp)
        if(fAllMotorHome==false || iHome==1)
        {
            fHome->GaliMotorServoOff("Key==Reset");                             //Steven 20230712 : 修正SwServoOn.Off時, 要抓住Z煞車
        }
        Cylinder[C_Empty_Fix].Off();
        Cylinder[C_Color_Fix].Off();
        AutoTrayCylinderFree();                                                 //jou 2010-01-25 start : 釋放Auto Tray上的汽缸
    }
    else if(Key==SnFKCleanOut && MskBtnCleanOutEnabled())                                  // :2588 BtnCleanOut->Enabled
    {
        if(bRunAutoClean)   return;                                             //Steven 20120208
        NewRecordProcess("MES2114", "CLEAN OUT pressed", "ScanKey");
        if(IniConfig.bEnable_SECS_GEM==true)                                    //Steven 20140528 : Secs Gem
            EventReport(SECS_EVENT.DoCleanOut);
        MskFormLock lock;
        fMain->BtnCleanOutClick(fMain);                                                     // cCleanOut.cpp
    }
    else if(Key==SnFKOneCycle)                                                              // :2598 (BtnOneCycle->Enabled: always true)
    {
        bCleanHotplate_ART=1;                                                   //kevin 20150722
        if(!(fContact && W906_FormShowing("fContact", fContact->fShow)))                    // :2602 fContact->fShow==false
        {
            NewRecordProcess("MES2115", "ONE CYCLE pressed", "ScanKey");
            bManualOneCycle=true;                                               //Sam 20230309
            if(IniConfig.bEnable_SECS_GEM==true)                                //Steven 20140528 : Secs Gem
                EventReport(SECS_EVENT.DoOneCycle);
            MskFormLock lock;
            fMain->BtnOneCycleClick(fMain);                                                 // cCleanOut.cpp
        }
        else
        {
            NewRecordProcess("MES2115", "ONE CYCLE pressed", "Contact Test");   //Steven 20211213
            W906_Contact_OneCycleProcess();                                                 // golden fContact->OneCycleProcess() (takes FormLock itself)
        }
    }
    else if(Key==SnFKTrayFeed)                                                              // :2616
    {
        if(bRunAutoClean)
            return;                                                             //Steven 20120208
        if(bUnloading)                                                          //JerryYang 20250418 tray feed到一半不能
            return;
        MskFormLock lock;
        fMain->InitialTrayFeedTask("ScanKey");                                              // cCleanOut.cpp
    }
    else if(Key==SnFKAlarmReset && bAlarmBuzzer)                                            // :2626
    {
        NewRecordProcess("MES2116", "ALARM RESET pressed", "ScanKey");
        bAlarmBuzzer=false;
        bLampAlarmReset=false;
        SECS_GEM_PPMUSIC_CONTROL_flag=false;
        SECS_GEM_PPSIGNALTOWER_CONTROL_flag=false;
        SW[SwFKAlarmReset].Off();
        SW[SwRKAlarmReset].Off();
        if(IniConfig.bEnable_SECS_GEM==true)                                    //Steven 20140528 : Secs Gem
            EventReport(SECS_EVENT.DoAlarmReset);
    }
    else if(Key==SnFKPowerOff || Key==SnRKPowerOff)                             //Alick 20160912
    {
        NewRecordProcess("MES2117", "POWER OFF pressed", "ScanKey");
    }
    else if(Key==SnFKPowerOn || Key==SnRKPowerOn)                               //kevin 20190328 power ON
    {
        NewRecordProcess("MES2128", "POWER ON pressed", "ScanKey");
    }
    else if((Key==SnFKTrayEnd || Key==SnRKTrayEnd) &&
            ArmSpeed_File[InArm].bAutoSKIP==1 && bASkStart==true)               //kevin 20170606 (wei) add autoskip tray end
    {
        MOT[MMTrayY].InitNewTray(NULL_IC, false, __FUNC__);
        InitTrayEndFunction();
        if(CosFunction.bShowHPICCount)                                          //Steven 20221228 : 計算加熱盤IC數量
        {
            fSortCT->pnlLoad->Caption=MOT[MMTrayY].Tray.HowManyIC();
        }
    }
}

// golden TfHome::Timer1Timer uhome.cpp:4885-4889 -> TfHome::ScanKey :4874-4883 (Timer1 Enabled = W906_HomeTimer1Enabled, uhome.cpp)
void W906_HomeScanKeyWith(int (*scan)())
{
    if(W906_HomeTimer1Enabled==false || fHome==0) return;                                  // TTimer Enabled (golden FormClose :4871)
    if(fHome->fShow==false) return;                                                         // :4887   AI(W906-ST02-SKC) 20261004 (St02-E, FShow_Audit b59a): the member on purpose, as :289 (R142=A, NB2 R188 M1) -- TfHome::ScanKey and TfMain::ScanKey must read the same flag, or the page table's "any browser has Home Monitor open" would let both answer one key; baseline raised for these two reads
    int Key;                                                                                // :4876
    Key=scan();                                                                             // :4877
    if(Key==SnFKPause)                                                                      // :4878
    {
        fHome->sbAbortHomeClick(fHome);                                                     // :4880
        fHome->Close();                                                                     // :4881
    }
}

// golden TfMain::TimerScanKeyTimer main.cpp:31994-32025 (Steven 20151022)
void W906_MainScanKeyTickWith(int (*scan)(), bool noticeUp)
{
    MskContactFormShowEdge();
    W906_HomeScanKeyWith(scan);                                                             // the 10 ms timer: first, as it usually wins in golden
    static bool bRunTimer1=false;                                                           // :31996
    if(InitialOK==false) { bRunTimer1=false; return; }                                      // :31997-32001
    if(bEnableEmployeeIDCheck==true) return;                                                // :32003-32004
    if(bSECSGEMAlarm && bSECSGEM_NoteAlarm==false)                                          // :32006-32007
    {
        // fNote->labSecsGemLock->Visible=true;   // golden :32009 -- no widget in the port
        bRunTimer1=false;
        return;
    }
    // else fNote->labSecsGemLock->Visible=false;   // golden :32015
    if(bRunTimer1==true) return;                                                            // :32018
    bRunTimer1=true;
    W906_MainScanKeyWith(scan, noticeUp);                                                   // :32023
    bRunTimer1=false;
}
void W906_MainScanKeyTick(bool noticeUp) { W906_MainScanKeyTickWith(&ScanPannelKey, noticeUp); }
