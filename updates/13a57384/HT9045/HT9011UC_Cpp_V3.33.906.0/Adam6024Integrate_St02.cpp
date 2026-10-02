// =============================================================================
//  Adam6024Integrate_St02.cpp -- the ADAM-6024 EP integration glue (golden 912 main.cpp / iosetview.cpp callers).
//
//  AI(W906-ST02-ADAM) 20261002 (St02-E helper H4).  Card ST02-ADAM (Steven 1002 ~10:5x: "Adam6024's code is not
//  written yet, port it").  Golden = 912 D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy (cp950); the 906_0625_Steven
//  line follows every 912 line so the port's 906-born callers can be compared.
//
//  WHY THIS FILE EXISTS -- the laptop's hold-back 3c348627 (B1 / B2 / M1 / M2)
//    75756cab made the EP write path real but left out golden's other EP writers; 3c348627 reverse-applied it:
//      B1  no Timer2 production set-point writer (golden 912 main.cpp:21677-21803 = 906 :21058-21184): after
//          AutoClean (AutoClean.cpp writes TestIF.fAutoClean_AireForce) every later touchdown keeps the AutoClean
//          force; no heater-door zero; DeviceForm.fAireForce is never refreshed (golden's ONLY production writer of
//          it is :21692 / :21696 -- the tester engine's soft-contact and IndexAddPressEP writes read it);
//      B2  ADAMTCP_Close's unconditional WSACleanup kills every socket of wb_serve -> H1's Public/AdamTcp_St02.cpp
//          [W906] S3 Close refcount (not this file; the flow test checks it);
//      M1  no FormClose zeroing (golden 912 main.cpp:11947-11952 = 906 :11462-11467) and no Close_ADAM_6024
//          (912 :12194 = 906 :11677): the press head stays pressurised after exit;
//      M2  no ADAM_ReturnValueCheck in the 1 s pump (912 :22354-22355 = 906 :21724-21725): no 60 s keep-alive read,
//          no EP-voltage alarm WAR16322 / WAR16323.
//    (reconstructed from the commit messages; the review text itself was asked of Jimmy, CHAT_ST02 1002 10:52.)
//
//  WHAT IS HERE (one golden place each)
//    W906_AdamEpLive()              [W906] the ONE live switch (build: W906_ADAM_EP_LIVE in MachineType.h)
//    W906_AdamTimer2Tick()          golden 912 main.cpp:21677-21803 + :22354-22355, one Timer2 tick
//    W906_AdamTimer2Pump()          the wb_serve caller of the above: 1 s limiter (Timer2 has no Interval in
//                                   main.dfm:17311 => VCL default 1000 ms) + golden's InitialOK guard :21495-21496
//    W906_AdamFormShowOpen()        golden 912 main.cpp:9890 Open_ADAM_6024() (+ HT9045.cpp:239 CreateForm guard)
//    W906_AdamFormCloseZero()       golden 912 main.cpp:11947-11952
//    W906_AdamFormCloseDisconnect() golden 912 main.cpp:12194
//    W906_AdamHomeReturnValueCheck  golden 912 csystem.cpp:10999 (csystem.cpp's W906G4 seam forwards here)
//    W906_AdamIoPageShowEp()        golden 912 iosetview.cpp:495-522 (the EP lines of Tfiosetview::FormShow)
//    W906_AdamIoPageClose()         golden 912 iosetview.cpp:293-294
//    W906_AdamEpState()             observability (boot line / tests)
//
//  THE LIVE SWITCH (proposal to St02-E / the laptop; MachineType.h is the laptop's file)
//    OFF (default, `//#define W906_ADAM_EP_LIVE`): the machine behaves exactly as before this card --
//      * Public/AdamTcp_St02.cpp never loads ADAMTCP.dll (requirement R1 for H1),
//      * ADAM_DirectWriteData / ADAM_WriteVoltage / ADAM_Alarm / ADAM_Alarm_Kg / ADAM_DualAlarm / APAX_WriteData /
//        ADAM_ReadAIValue answer like the retired stand-ins atester_shims.cpp:326-330 (requirement R2 for H1-H3),
//      * every function in this file returns at once (no Timer2 writes, no SwMultiEp DO, no boot / exit / home I/O);
//      * the new call sites (uhome interlock, Command.cpp reply) test W906_AdamEpLive() first.
//    ON: every golden EP writer is live AT THE SAME TIME -- there is no state with some writers and not others,
//      which is exactly what 3c348627 held back.  EastSun / ES02 switch it on for the on-machine check.
//    Tests use W906_AdamEpLive_SetForTest(1) / (0) / (-1 = back to the build default).
//
//  [W906] DEVIATIONS (each also at its line)
//    D1  fContact->fShow / fiosetview->fShow -> W906_FormShowing(<golden form>, <member>) (St01 page table,
//        W906FormShowing.h; same substitution as adam6024.cpp:941 EpSwitch).
//    D2  fContact->dDutCount -> fContactForm->dDutCount, and the golden NULL test on fContact -> fContactForm:
//        the port's TfContact is forms/fContact.h:976 (fContactForm, :1634; dDutCount :1501, refreshed by the golden
//        DutCount() callers, e.g. wb_serve's boot golden main.cpp:9385); the atester_shims fContact is a shim
//        without dDutCount.
//    D3  golden Timer2Timer's function statics (912 main.cpp:21482-21486: ct, iDieForce, fAirForce, bOpenChambo)
//        are file statics here (same initial values, same lifetime) so the test can reset them.
//    D4  Timer2Timer's other guards: fShow (TfMain shown) and bTimer2Run (re-entry) pass -- same reading as
//        FileRW/MainRecord.cpp:59-62 (wb_serve has no TfMain form; one thread, no re-entry).
//    D5  S25 (RULINGS_20260925 S25, customer-only branches are skipped and annotated): the SPIL CKD-FCM block
//        912 main.cpp:21781-21798 (IniConfig.bSPILFunction) is compiled out; golden has no else arm.
//    D6  an exception thrown inside one tick is caught and printed: in golden the VCL Application handler shows it
//        and Timer2 keeps firing; here it must not leave the wb_serve main loop.
//    D7  W906_AdamFormShowOpen also creates fAdam6024 when nobody has (golden HT9045.cpp:239 CreateForm runs
//        before TfMain::FormShow; TfAdam6024's ctor only fills the error table, golden adam6024.cpp:122-140).
// =============================================================================
#include "Adam6024_St02.h"           // golden 912 adam6024.h (H1): ADAM_* / Open_ADAM_6024 / APAX_WriteData / fAdam6024
#include "adam6024.h"                // TransformFuntion (laptop, golden 912 adam6024.cpp:1043-1812)
#include "atester_shims.h"           // fContact (TfContactShim::fShow) / fiosetview -- needs claim C-1 (it then includes Adam6024_St02.h)
#include "csystem.h"                 // bHeaterDoorIsOpen[4] (:197) / IndexHasIC (:140) / W906_FormShowing (:440)
#include "cmydef.h"                  // InitialOK / EP_Install / INSTALL_DOUBLE_EP / USE_CKD_FCM_CleanAir / bRunAutoClean /
                                     // bIndexEveryTimeCheckEPing / bRTCAutoVerifyControlEP / bTestEPaddKg / iEPControlValue
#include "cprod.h"                   // DeviceForm / DeviceForm_File / TestIF / TestIF_File
#include "Config.h"                  // IniConfig
#include "CosFunction.h"             // CosFunction.bRTCAutoModelVerify
#include "MachineType.h"             // W906_ADAM_EP_LIVE (proposed, claim C-14) / SOFT_SIMULTE
#include "myswitch.h"                // SW[] (SwMultiEp)
#include "aHotPlateSubstrate.h"      // FTestSuck / BTestSuck (mykitsuck.h, the golden TMyKitSuck since A4-6)
#include "forms/fContact.h"          // fContactForm->dDutCount (D2)
#include "forms/fProductionInfo.h"   // fProductionInfo->CheckContactForceExist / GetOffsetContactForce

#include <windows.h>                 // GetTickCount
#include <cstdio>

// ---- this file's API is declared in Adam6024_St02.h (H4 integration pass 20261002) ----

// =============================================================================
//  the live switch
// =============================================================================
namespace {
int s_iLiveOverride = -1;            // -1 = build default; 0 / 1 = W906_AdamEpLive_SetForTest
}

bool W906_AdamEpLive()
{
    if (s_iLiveOverride >= 0)
        return s_iLiveOverride != 0;
#ifdef W906_ADAM_EP_LIVE
    return true;
#else
    return false;
#endif
}

void W906_AdamEpLive_SetForTest(int iLive)
{
    s_iLiveOverride = (iLive < 0) ? -1 : (iLive ? 1 : 0);
}

const char* W906_AdamEpState()
{
#ifdef W906_ADAM_EP_LIVE
    const bool bBuild = true;
#else
    const bool bBuild = false;
#endif
    if (s_iLiveOverride >= 0)
        return s_iLiveOverride ? "ON (test override)" : "OFF (test override)";
    return bBuild ? "ON (W906_ADAM_EP_LIVE)" : "OFF (W906_ADAM_EP_LIVE not defined: stand-in behaviour, no ADAM I/O)";
}

// =============================================================================
//  golden 912 TfMain::Timer2Timer (main.cpp:21480-25819), the EP part only
// =============================================================================
namespace {
// golden 912 main.cpp:21482-21486 (906 :20864-20868), function statics there ([W906] D3)
int    ct=0, iDieForce=-1;
double fAirForce=-1.0;                                                          //Steven 20110426 : 0.0 --> 1.0
bool   bOpenChambo=false;
}

void W906_AdamTimer2_ResetForTest()
{
    ct=0;  iDieForce=-1;  fAirForce=-1.0;  bOpenChambo=false;                    // golden's initial values (:21482-21486)
}

//  One golden Timer2 tick: 912 main.cpp:21677-21803 (906 :21058-21184, identical) then :22354-22355
//  (906 :21724-21725).  Every statement in golden order; [W906] marks the substitutions (see the banner).
void W906_AdamTimer2Tick()
{
    if (!W906_AdamEpLive())                                                     // [W906] the live switch
        return;
    try
    {
    if(W906_FormShowing("fContact", fContact->fShow) || W906_FormShowing("fiosetview", fiosetview->fShow) ||   //set EP force : start   // golden :21677 fContact->fShow || fiosetview->fShow ([W906] D1)
      (IniConfig.bIndexEveryTimeCheckEP==true &&
       IniConfig.bD24EnableEPCheckFuntion==true &&
       bIndexEveryTimeCheckEPing==true))                                        //Jou 20110501   // golden :21678-21680
    {
        fAirForce=-1;                                                           // golden :21682
        iDieForce=-1;                                                           // golden :21683
    }
    else
    {
        ct++;                                                                   // golden :21687
        if(fProductionInfo!=NULL &&
           fContactForm!=NULL &&                                                // [W906] D2 golden :21689 fContact!=NULL
           fProductionInfo->CheckContactForceExist())                           //JimmyChiu 20220117 確認Server是否設定Contact Force，如無設定依照原設定   // golden :21688-21690
        {
            DeviceForm.fAireForce=DeviceForm.dPress+fProductionInfo->GetOffsetContactForce()*fContactForm->dDutCount;   // golden :21692 ([W906] D2 fContact->dDutCount)
        }
        else
        {
            DeviceForm.fAireForce=DeviceForm.dPress;                            // golden :21696
        }

        //AI(ht9045-v899) 20260430: Auto switch SwMultiEp DO when INSTALL_DOUBLE_EP==DOUBLE_EP_MULTI (Multi EP half).   // golden :21699-21701
        if(SW[SwMultiEp].Enable==true)                                          // golden :21702
        {
            if(INSTALL_DOUBLE_EP==DOUBLE_EP_MULTI && TestIF_File.bIndEPSLK)     // golden :21704
            {
                if(SW[SwMultiEp].Status()==false)
                    SW[SwMultiEp].On();                                         // golden :21706-21707
            }
            else
            {
                if(SW[SwMultiEp].Status()==true)
                    SW[SwMultiEp].Off();                                        // golden :21711-21712
            }
        }

        if(INSTALL_DOUBLE_EP==DOUBLE_EP_NORMAL || INSTALL_DOUBLE_EP==DOUBLE_EP_MULTI)   //Steven 20191024 : For die force //AI(ht9045-v899) 20260430: extend to Multi EP half.   // golden :21716
        {
            int iInputValue=0;                                                  // golden :21718
            if(bRunAutoClean)
                iInputValue=TransformFuntion(TestIF_File.fAutoClean_DieForce, true);    //Steven 20240719 : Die force for auto clean   // golden :21719-21720
            else
                iInputValue=TransformFuntion(DeviceForm_File.DoubleForce, true);       // golden :21721-21722
            if(iDieForce!=iInputValue || ct>=10)                                // golden :21723
            {
                //AI(ht9045-v899) 20260526: Multi EP production Die Force must keep one-by-one dual output values.
                if(INSTALL_DOUBLE_EP==DOUBLE_EP_MULTI && IsMultiEPPressureRouteActive()==true)   // golden :21726
                    APAX_WriteData(false, 0);                                   // golden :21727
                else
                    ADAM_DirectWriteData(iInputValue, 0, 0);                    // golden :21729
                iDieForce=iInputValue;                                          // golden :21730
            }

            if(INSTALL_DOUBLE_EP==DOUBLE_EP_MULTI && TestIF_File.bIndEPSLK)     //AI(ht9045-v899) 20260526: keep APAX inner/dual force array fresh for production APAX_WriteData(false).   // golden :21733
            {
                if(bRunAutoClean)
                    TransformFuntion(TestIF_File.fAutoClean_DieForce, true);    // golden :21735-21736
                else
                    TransformFuntion(DeviceForm_File.DoubleForce, true);        // golden :21737-21738
            }
        }

        if(fAirForce!=DeviceForm.fAireForce || ct>=10 || bOpenChambo)           // golden :21742
        {
            fAirForce=DeviceForm.fAireForce;                                    // golden :21744
            if((bHeaterDoorIsOpen[0] || bHeaterDoorIsOpen[1]) &&
               !(FTestSuck.HasRealIC() || BTestSuck.HasRealIC()))               // golden :21745-21746
            {
                fAirForce=0.0;                                                  // golden :21748
                bOpenChambo=true;                                               //kevin 20120920   // golden :21749
            }
            else
            {
                if(bRunAutoClean)                                               //kevin 20120710      autoclean   // golden :21753
                {
                    if(TestIF.fAutoClean_AireForce<IniConfig.iEP_Min_KG)        //Wei 20160418 : Add for EP被重置導致掉料   // golden :21755
                        TestIF.fAutoClean_AireForce=IniConfig.iEP_Min_KG;       // golden :21756 (GOLDEN QUIRK kept: raises the recipe's AutoClean force in memory)
                    fAirForce=TestIF.fAutoClean_AireForce;                      // golden :21757
                }
                else
                {
                    fAirForce=DeviceForm.fAireForce;                            // golden :21761
                }
                bOpenChambo=false;                                              //kevin 20120920   // golden :21763
            }

            if(CosFunction.bRTCAutoModelVerify)                                 //jou 2014-06-24 RTC 自動進行Model驗證   // golden :21766
            {
                if(bRTCAutoVerifyControlEP==false)
                    ADAM_WriteVoltage(fAirForce);                               // golden :21768-21769
            }
            else if(IniConfig.bIndexAddPressEP==true)                           //jou 20171026 (wei) : 測試中加壓EP   // golden :21771
            {
                if(bTestEPaddKg==false)
                    ADAM_WriteVoltage(fAirForce);                               // golden :21773-21774
            }
            else
            {
                ADAM_WriteVoltage(fAirForce);                                   // golden :21778
            }

#if 0   // [W906] D5 S25 (RULINGS_20260925 S25): SPIL-only CKD FCM flow, golden 912 main.cpp:21781-21798 (906 :21162-21179) VERBATIM; golden has no else arm
            if(IniConfig.bSPILFunction && USE_CKD_FCM_CleanAir)
            {
                if(SystemStart==true)
                {
                    if(iEPControlValue==100)                                    //百分比
                    {
                        ADAM_DirectWriteData(4095, 1);
                    }
                    else
                    {
                        ADAM_DirectWriteData((iEPControlValue*41), 1);
                    }
                }
                else
                {
                    ADAM_DirectWriteData(0, 1);
                }
            }
#endif  // [W906] D5 S25

            ct=0;                                                               // golden :21800
        }
    }
    //set EP force : end                                                        // golden :21803

    if(EP_Install==3 || EP_Install==5)                                          //kevin 20220412 EP 目前只有3 才有回授   // golden :22354
        ADAM_ReturnValueCheck();                                                //wei 20220309 Add EP Voltage Error Alarm   // golden :22355
    }
    catch (...)                                                                 // [W906] D6
    {
        std::printf("  [ADAM] W906_AdamTimer2Tick: exception caught (golden Timer2 keeps firing; next tick runs again)\n");
    }
}

//  The wb_serve caller (tools/wb_serve.cpp main loop pumpBeat, claim C-12; and the drag keepalive copy, C-13).
//  golden Timer2: no Interval in main.dfm:17311 => VCL default 1000 ms; wb_serve beats every 500 ms, so the
//  same GetTickCount limiter as cStateRecord.cpp W906_StateRecordTimer2Pump / FileRW/MainRecord.cpp
//  W906_MainRunInfoTimer2Tick.  Guard: golden Timer2Timer :21495-21496 InitialOK ([W906] D4 for fShow / bTimer2Run).
void W906_AdamTimer2Pump()
{
    if (!W906_AdamEpLive())                                                     // [W906] the live switch (no limiter state either)
        return;
    static DWORD s_last=0;
    const DWORD now=::GetTickCount();
    if(s_last!=0 && (DWORD)(now-s_last)<1000u) return;
    s_last=now;

    if(InitialOK==false)                                                        // golden Timer2Timer :21495-21496
        return;

    W906_AdamTimer2Tick();
}

// =============================================================================
//  boot / close (golden 912 TfMain::FormShow / FormClose)
// =============================================================================
//  golden 912 main.cpp:9890 (906 :9457) `Open_ADAM_6024();` in TfMain::FormShow, after InitialGaliDelayCount :9884
//  and before bHasTrayCSV :9987 / InitialHandler :9992.  The return value is ignored, as in golden.  Called once by
//  wb_serve's bring-up (claim C-11).  [W906] D7: creates fAdam6024 first if nobody has.
void W906_AdamFormShowOpen()
{
    std::printf("  [ADAM] EP live switch: %s\n", W906_AdamEpState());
    if (!W906_AdamEpLive())                                                     // [W906] the live switch
        return;
    if (fAdam6024 == NULL)                                                      // [W906] D7 golden HT9045.cpp:239 CreateForm(__classid(TfAdam6024), &fAdam6024)
        fAdam6024 = new TfAdam6024(NULL);
    const bool bOpen = Open_ADAM_6024();                                        //Jimmychiu 20230804 : 整合全部連線檢查   // golden :9890
    std::printf("  [ADAM] golden main.cpp:9890 Open_ADAM_6024() -> %s (bADAM6420Install=%d)\n",
                bOpen ? "true" : "false", (int)bADAM6420Install);
}

//  golden 912 main.cpp:11947-11952 (906 :11462-11467) in TfMain::FormClose, before :11953 InitialOK=false.
//  Called from FileRW/MainClose.cpp ShutdownSequence(execute) row "11947" (claim C-9, St01).
void W906_AdamFormCloseZero()
{
    if (!W906_AdamEpLive())                                                     // [W906] the live switch
        return;
    ADAM_WriteVoltage(0);                                                       //kevin 20161005 程式關閉 將ep 設為0避免增壓缸一直起動   // golden :11947
    ADAM_DirectWriteData(0, 0);                                                 // golden :11948
    if(IsIndependentEPPressureRouteActive()==true)                              //AI(ht9045-v899) 20260526: reset APAX only when the independent EP pressure route is active.   // golden :11949
    {
        APAX_WriteData(true, 0);                                                // golden :11951
    }
}

//  golden 912 main.cpp:12194 (906 :11677) `Close_ADAM_6024();` after EndMainThread :12193.
//  Called from FileRW/MainClose.cpp ShutdownSequence(execute) row "12194" (claim C-10, St01).
void W906_AdamFormCloseDisconnect()
{
    if (!W906_AdamEpLive())                                                     // [W906] the live switch
        return;
    Close_ADAM_6024();                                                          // golden :12194
}

//  golden 912 csystem.cpp:10999 (906 :10386) `ADAM_ReturnValueCheck(true);` in DoHomeProcess, inside
//  `if(EP_Install==3 || EP_Install==5)` (the port keeps that if at csystem.cpp:7498).  The csystem.cpp seam
//  W906G4_ADAM_ReturnValueCheck forwards here (claim C-6).
void W906_AdamHomeReturnValueCheck(bool bHome)
{
    if (!W906_AdamEpLive())                                                     // [W906] the live switch (= the old no-op seam)
        return;
    ADAM_ReturnValueCheck(bHome);                                               // golden csystem.cpp:10999 ADAM_ReturnValueCheck(true)
}

// =============================================================================
//  the IO page (golden 912 Tfiosetview) -- page-side EP writers, ready for the page-events owner
// =============================================================================
//  golden 912 iosetview.cpp:495-522 (906 :404-431), the EP statements of Tfiosetview::FormShow (the widget lines
//  -- bplEpSwitch*, tbarIndexEP->Max / Enabled, tempBtn[] -- are the web page's).  While the IO page is showing,
//  golden Timer2 does not write (:21677); this is golden's own zero for that time.  NOT CALLED YET: the port has
//  no IO-page open edge (FileRW/WindowEdgeTails.h has no fiosetview row) -- open item O-3 in the H4 report.
void W906_AdamIoPageShowEp()
{
    if (!W906_AdamEpLive())                                                     // [W906] the live switch
        return;
    if(IndexHasIC())                                                            // golden :495
        return;                                                                 // golden :496-502 (widgets only)
    if(EP_Install>0)                                                            // golden :505
    {
        ADAM_WriteVoltage(0);                                                   // golden :511
        if(USE_CKD_FCM_CleanAir)                                                // golden :518
        {
            ADAM_DirectWriteData(0, 1);                                         //Ifor 20150710 :寫入資料到ADAM (資料,裝置位置)   // golden :520
        }
    }
}

//  golden 912 iosetview.cpp:293-294 (906 :219-220), the head of Tfiosetview::FormClose.  NOT CALLED YET (same reason).
void W906_AdamIoPageClose()
{
    if (!W906_AdamEpLive())                                                     // [W906] the live switch
        return;
    Close_ADAM_6024();                                                          // golden :293
    Open_ADAM_6024();                                                           //Jimmychiu 20230804 : 整合全部連線檢查   // golden :294
}
