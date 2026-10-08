// =============================================================================
//  MainTimer5.cpp -- T-05: golden TfMain::Timer5Timer (main.cpp:31210-31297), the ATC connection watchdog.
//  AI(W906-T05) 20261008 (Ifor01).  Plan: docs/TIMER_TABLE_PLAN.md s4 (rules) / s5.2 T-05; census129_e E-TM-010.
//
//  Timer: fMain->Timer5 (forms/fMain_Timers.h:28; golden main.dfm:17323 Enabled=True, Interval=1000), OnTimer bound in
//  W906_TfMain_TimersBoot (forms/fMain_Timers.cpp).  Body = golden, line for line (cp950 -> UTF-8); only the plan's
//  allowed edits: signature without __fastcall / Sender.
//
//  What runs: the InitialOK / re-entry guard and the HonPrec (ATC 2.0) branch -- while running with ATC active cooling,
//  iATCOnLine follows ATC_SYS_PAL[0]->LedATCConnect and a lost connection raises WAR15309 when the tester is on line
//  (or ASE SG).  The LED is real since T-05: TATCInterfaceForm::RefreshChannelState (ATC/ATCInterface.cpp) sets it from
//  HT_ATC::IsConnect (the ATC socket), called by ATCWatchTimerTimer, now on the timer table (ATCInterfaceForm.ATCWatchTimer,
//  200 ms; enabled by InitialATC, which boot calls only when ATC_SYSTEM==eATCHonPrecType -- W906_BootInitialATC).
//  HT9050: USE_ATC_MODE=0 (eATCUninstall) => none of the ATC branches applies.
//
//  Gate table (#if 0, golden text kept):
//    G1  Tj log display (golden :31219-31228): no MmoTj / Memo1 on the facade; Button3Click opens ASESendMessage (not ported).
//    G2  ATC 7.0 branch (:31247-31260): fLotInfo->aldATCPower has no live writer (TfLotInfo::NetATCTimeTimer not ported).
//    G3  New ATC block (:31264-31281): IsConnect() is a flag nothing sets true (CommFlagTimer GATE (WIDGET)).
//    G4  ASE Kaohsiung AskATCDateToHandle (:31284-31291): declared only, GATE(NET).
//    G2 / G3 are gated because opened they would raise a false WAR15309 every second on those machines.
//
//  Deviation from golden (D1): golden ShowErrorMessage("WAR15309", 0, ...) is fNote->ShowModal(), so Timer5 stands still
//    while the notice is up.  In wb_serve kcode 0 is a non-waiting notice (same finding as MainTimer8.cpp, MES0921), so
//    every second would raise it again.  Timer5 therefore returns while a WAR15309 notice is showing -- golden's effect.
// =============================================================================
#include "MachineType.h"      // eATCHonPrecType / eNewATCSystem
#include "cmydef.h"           // InitialOK, SystemStart, ATC_SYSTEM, iATCOnLine, iReceiveATCData, CUSTOMER_CODE, MMATC_TCPIP
#include "cprod.h"            // Temperature (SYSTEM_TEMPERATURE)
#include "LastSet.h"          // LastSet.iTester
#include "canary_support.h"   // ShowErrorMessage
#include "csystem.h"          // W906_FormShowing
#include "forms/fNote.h"      // fNote->fShow / edErrorCode (deviation D1)
#include "ATC/ATCInterface.h" // ATCInterfaceForm->ATC_SYS_PAL[0]->LedATCConnect

namespace {
bool W906_War15309NoticeUp()                                                    // deviation D1
{
    return fNote != 0 && W906_FormShowing("fNote", fNote->fShow) &&
           fNote->edErrorCode != 0 && fNote->edErrorCode->Text == AnsiString("WAR15309");
}
}  // namespace

void W906_TfMain_Timer5Timer()                                                  // golden main.cpp:31210 (TfMain::Timer5Timer; __fastcall / Sender dropped, plan s4 rule 3)
{
    static bool bTimerRunning=false;
    static int iCount=0;                                                        //kevin 20201202 add count

    if(InitialOK==false || bTimerRunning==true)
        return;
    if(W906_War15309NoticeUp())                                                 //AI(W906-T05) deviation D1 (file head): golden's WAR15309 is modal and holds this timer; the web notice is not
        return;

    bTimerRunning=true;
#if 0 // GATE(W906-TIMER-T05-G1): Tj log display -- the TfMain facade has no MmoTj / Memo1, and golden Button3Click (main.cpp:31141-31144) opens the ASESendMessage dialog, not ported (display only; golden does not clear sTjLogList here)
    if(Temperature.bATCActiveCooling && Temperature.bUseTjFunction)
    {
        MmoTj->Lines = ATCInterfaceForm->ATC_60_SYS.sTjLogList;

        if(ATCInterfaceForm->ATC_60_SYS.sTjLogList->Count > 10000)
        {
            Button3Click(Owner);
            Memo1->Lines->Clear();
        }
    }
#endif // GATE(W906-TIMER-T05-G1)
    //Ifor 20160223 add ATC2.0 & ATC7.0 ATC斷線停機
    //Ifor 20160331 add SECS GEM ATC Online 旗標
    if(ATC_SYSTEM==eATCHonPrecType)
    {
        if(SystemStart==true)
        {
            if(Temperature.bATCActiveCooling==true)
            {
                if(ATCInterfaceForm->ATC_SYS_PAL[0]->LedATCConnect->Value==false)
                {
                    iATCOnLine=false;
                    if(LastSet.iTester==ON_LINE || CUSTOMER_CODE==CC_ASE_SG)    //JerryYang 20230322 : 新增2DID模式, != offline改為 ==online   //Ifor 20180222 (wei) :HT7045 Offline 不判斷 ATC是否連線
                        ShowErrorMessage("WAR15309", 0, MMATC_TCPIP, false, "Main--Timer5");                            //ATC software disconnect! Please check ATC software ==> ATC connect error, Please confirm whether to open? and connect ATC
                }
                else
                {
                    iATCOnLine=true;
                }
            }
#if 0 // GATE(W906-TIMER-T05-G2): ATC 7.0 reads fLotInfo->aldATCPower, written only by TfLotInfo::NetATCTimeTimer (V912 uLotInfo.cpp:8558-9350, not ported; forms/fLotInfo.h:2303) -- opened, a running ATC 7.0 machine would alarm WAR15309 every pass
            else if(Temperature.bATC70Active==true)
            {
                if(fLotInfo->aldATCPower->Value==false)
                {
                    iATCOnLine=false;
                    if(LastSet.iTester==ON_LINE || CUSTOMER_CODE==CC_ASE_SG)    //JerryYang 20230322 : 新增2DID模式, != offline改為 ==online   //Ifor 20180222 (wei) :HT7045 Offline 不判斷 ATC是否連線
                        ShowErrorMessage("WAR15309", 0, MMATC_TCPIP, false, "Main--Timer5");                            //ATC software disconnect! Please check ATC software ==> ATC connect error, Please confirm whether to open? and connect ATC
                }
                else
                {
                    iATCOnLine=true;
                }
            }
#endif // GATE(W906-TIMER-T05-G2)
        }
    }
    #ifndef SOFT_SIMULTE
#if 0 // GATE(W906-TIMER-T05-G3): New ATC -- ATC_InterfaceForm->IsConnect() returns IsConnectFlag, only ever set false (forms/fATCHandlerSide.cpp:176); golden sets it in CommFlagTimerTimer (ATC_Handler_Side.cpp:2803-2845), GATE (WIDGET) here -- opened, every running New-ATC machine would alarm WAR15309 every pass
    if(ATC_SYSTEM==eNewATCSystem)
    {
        if(ATC_InterfaceForm->IsConnect()==true)
        {
            iATCOnLine=true;
        }
        else
        {
            iATCOnLine=false;
            if(SystemStart==true && Temperature.bATCActiveCooling==true)
            {
                if(LastSet.iTester==ON_LINE || CUSTOMER_CODE==CC_ASE_SG)        //JerryYang 20230322 : 新增2DID模式, != offline改為 ==online  //Ifor 20180222 (wei) :HT7045 Offline 不判斷 ATC是否連線
                {
                    ShowErrorMessage("WAR15309", 0, MMATC_TCPIP, false, "Main--Timer5");                                //ATC software disconnect! Please check ATC software ==> ATC connect error, Please confirm whether to open? and connect ATC
                }
            }
        }
    }
#endif // GATE(W906-TIMER-T05-G3)
    #endif

#if 0 // GATE(W906-TIMER-T05-G4): ATC_InterfaceForm->AskATCDateToHandle is declared only (forms/fATCHandlerSide.h:901, GATE(NET)); S25 customer (ASE Kaohsiung) and New ATC as well
    if(CUSTOMER_CODE==CC_ASE_KaohSiung &&
       ATC_SYSTEM==eNewATCSystem &&
       iATCOnLine==1 && iReceiveATCData==0 && iCount>=3)                        //kevin 20200630dd read atc
    {
        iReceiveATCData=1;                                                      //kevin 20200608 讀取ATC 參數資料
        ATC_InterfaceForm->AskATCDateToHandle();
        iCount=0;                                                               //kevin 20201202 add count
    }
#endif // GATE(W906-TIMER-T05-G4)

    if(CUSTOMER_CODE==CC_ASE_KaohSiung)
        iCount++;                                                               //kevin 20201202 add count

    bTimerRunning=false;
}
