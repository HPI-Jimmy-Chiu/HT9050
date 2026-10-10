// ===========================================================================
//  TesterComm/Handler/HandlerGpibMsg.cpp -- THandlerTesterSide: the Handler's reaction to every tester-bridge packet
//  (golden TfMain::OnMyCopyMsg `case WM_GPIB_Program:`), plus ProcessARTMessage and ResetForESC.  + ProcessForESC (AI(W906-ST02-ESC) 20261005 (St02-E), golden 0618 :7615-7706).
//  Translated per TesterComm/Handler/TRANSLATION_RULES.md and the banner of HandlerTesterSide.h.
//
//  AI(W906-GB-P2a) 20260926.  Golden: D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\main.cpp (Big5/cp950)
//      :8027-8038     ResetForESC
//      :15973-16475   ProcessARTMessage       (the body really ends at :16475; :16476-16480 is OnMyCopyMsg's
//                                              "注意!!" banner, kept above OnGpibProgramMsg)
//      :16481-16530   OnMyCopyMsg preamble + locals            -> THandlerTesterSide::OnGpibProgramMsg(LPARAM lParam)
//      :16536-17819   body of `case WM_GPIB_Program:`          (its `break;` at :17819 == end of the function)
//  Not here: the switch head and `case WM_GPIBInterface:` (:16531-16535) and every later case (WM_ASEKH_Program ...):
//  the hub hands only bridge packets to THandlerTesterSide::Sink (HandlerTesterSide.cpp), which calls this function.
//  Golden text is copied line by line (generated from the cp950-decoded golden, then only the edits below applied);
//  golden comments and column layout are kept.  EOL: CRLF (same as Command.cpp / HandlerBridgeCtl.cpp).
//
//  NAME OWNERSHIP (rule 2)
//    * THandlerTesterSide members, unqualified as golden: SendMSG_CMD, SendMSG_TestMode, ProcessARTMessage,
//      ResetForESC, tESDError, mmo1, bAMDRs232ConnectError, bHasPin1Error, bReceivePPSELECT, bTriggerESC.
//      Golden `fMain->SendMSG_CMD` (:16672), `fMain->ResetForESC` (:17059), `fMain->bHasPin1Error` (:17806/:17808)
//      are written as this object's member (AI note at each site).
//    * Every other golden TfMain member is reached as `fMain->X`; each X was checked against forms/fMain.h
//      class TfMain (lines 163-1268) -- the list is in the final report of this wave.  palMainStatus / tTestResult
//      are data members; the rest are the Command.cpp Write*/Get*/Set* replies, BtnPauseClick / BtnOneCycleClick /
//      Clarn_Data / AutoSiteOnOff / MachineStatus / Send_Command_TTL / ReadWaterValve / ReadDynamicPID ...
//    * Globals are unqualified.  Five golden main.cpp file-scope globals (bEcho / bExist / bUnderTest / bEchoStop /
//      iBin, golden :15690-15694) are defined in V906 atester_shims.cpp:101/:102/:120 with no header -> TU-local
//      `extern` below (same shape as atester.cpp:650-652 and HandlerBridgeCtl.cpp:70).
//
//  GATE REGISTER  (every `#if 0` in this file; golden text kept inside; the missing symbol that forces it)
//   G1  :15997        fLotInfo->btClearBarcodeList->Click() -- TfLotInfo has no btClearBarcodeListClick (golden   [AI(W906-TC-G1G5) 20261001 (St02-E): lifted -- :229, golden 0625 main.cpp:15432]
//                     uLotInfo.cpp:10005-10014) and vclcompat Click() is a no-op (rule 8): the call would do nothing.
//   G2  :16027        fSCKART->iCurrentStatus / ->iLOTSTATUS_A are not TfSCKART members (forms/fSCKART.h).   [AI(W906-W132) 20261007 (St02-E): lifted -- :364, golden 0618 main.cpp:15461-15462; the NARROW note below is history]
//                     NARROW: golden `if(fSCKART->iTesterType==1 || iCurrentStatus!=iLOTSTATUS_A)` short-circuits,
//                     so the if-arm still runs, exactly as golden, whenever iTesterType==1 (the facade's value for
//                     every customer except CC_SCK, forms/fSCKART.cpp:23-58).  Only the unknowable case
//                     (iTesterType!=1) is gated: there NEITHER arm runs (the else arm is G6).  No arm is guessed
//                     (the Command.cpp A11 rule).  `#else false)` stands for the gated disjunct.
//   [LIFTED P2c 20260926, user ruling item 6 = B "打開照 golden 寫"] G3 :16065  WriteLastDataFile(false) -- was
//                     gated for SAFETY (writes the hard-coded production D:\HT9045\system\lastdata.dat, no --dry
//                     redirect; Command.cpp's S3 is the same call).  Now live as golden.
//   G4  :16077-16081  fSCKART->iCurrentStatus (not a TfSCKART member).   [AI(W906-W132) 20261007 (St02-E): lifted -- :421, golden 0618 main.cpp:15512-15516]
//   G5  :16084        as G1.   [AI(W906-TC-G1G5) 20261001 (St02-E): lifted -- :327, golden 0625 main.cpp:15519]
//   G6  :16086-16090  else arm of G2 -- reachable only when iCurrentStatus==iLOTSTATUS_A (not TfSCKART members).   [AI(W906-W132) 20261007 (St02-E): lifted -- :434, golden 0618 main.cpp:15521-15525]
//   G7  :16103-16119  fSCKART->iCurrentStatus / iLOTSTATUS_A / iLOTSTATUS_F (not TfSCKART members).   [AI(W906-W132) 20261007 (St02-E): iCurrentStatus / iLOTSTATUS_A exist now; still missing: iLOTSTATUS_F]
//                     AccessFile(false, 10) after it stays live.
//   G8  :16134        fSCKART->GetLotStatus() (not a TfSCKART member; SckArt_GetLotStatus in Automation/SCK_ART.h
//                     works on a different state object).  Buffer stays "" -> an empty LOTSTATUS reply.
//   G9  :16138        as G8.
//   G10 :16150-16163  fSCKART->iCurrentStatus / iLOTSTATUS_L / iLOTSTATUS_F.   [AI(W906-W132) 20261007 (St02-E): iCurrentStatus exists now; still missing: iLOTSTATUS_L / iLOTSTATUS_F]
//   G11 :16210-16215  fSCKART->iCurrentStatus / iLOTSTATUS_A / iLOTSTATUS_F.   [AI(W906-W132) 20261007 (St02-E): iCurrentStatus / iLOTSTATUS_A exist now; still missing: iLOTSTATUS_F]
//   G12 :16336        fAGV->bATK_AMR_DoHostLotStart (not a TfAGV member, forms/fAGV.h).  No V906 code can set it,
//                     so the conjunct is its golden initial value (VCL zero-init false; false==false).
//   G13 :16339        fAGV->bATKAMR_GET_LOTORDER0_Ready (not a TfAGV member).  With G12: on an ATK-AMR machine a
//                     "LOTORDER" LOTSTATUS neither sets the ready flag nor starts the lot (fail-safe: no lot start
//                     without the SECS LOT_START handshake V906 lacks).  Every other machine: exactly golden.
//   G14 :16361        fShowBinSelect->btnAutoCleanClick -- not a TfShowBinSelect member (forms/fShowBinSelect.h:130
//                     lists it SAFETY-QUEUED, untranslated).  bGPIBAutoClean=true and the RecordProcess stay live.
//   [W71 20261003] G15 :16372-16381  golden 912's Tester Pause MaxTestTime delay (hPauseAlarmDelay / bPauseAlarmDelayActive,
//                     RogerYang 20260626) is BACK.  AI(W906-ST02-W71) 20261003 (St02-E): RULINGS_20261003 #1 + Steven W8 = A -- kept as a fix;
//                     golden 0618 main.cpp:15806 / ckernel.cpp:738 / :2113 / :2121 (read) sound at once; V912 main.cpp:16371-16381, ckernel.cpp:744-748 / :2144-2148 / :2158-2162.
//   [REMOVED 20261003] G16 :16431-16472  golden 912's MSG_CMD_RemoteStart / MSG_CMD_RemoteStop arms (JerryYang 20260828) and
//                     their H-012 wiring.  AI(W906-ST02-C912) 20261003 (St02-E helper): back to golden 906_0625 main.cpp:15847-15858 (RULINGS_20261002 #20 / #23-6):
//                     906 has neither arm nor MSG_CMD 204 / 205; ProcessARTMessage ends with ECHOOK_ONECYCLE (see its end).
//   G17 :16604        fObserver->pnlTTLRS232Version -- not a TfObserver member (cObserver.cpp:7455 documents it).
//   G18 :16626        fObserver->pnlGPIBVersion -- not a TfObserver member (cObserver.cpp:7454).  The
//                     RunInfo.GPIBSoftwareVersion write before it stays live.
//   G19 :16689 :16708 :16711-16712 :16720   fContact->iDMCArm / iDMCChannel[] / iDMCTrayX[] / iDMCTrayY[] /
//                     bWaitLoadXY -- the global fContact is TfContactShim (atester_shims.h:154-251), which has none of
//                     them; the real TfContact (forms/fContact.h:1535-1538) has them but no global.  The PickLoad
//                     parse loop itself stays live (it only writes locals besides these).
//   G20 :16724        fContact->bPlaceLoad (TfContactShim, as G19).
//   G21 :16728        fContact->bTrayFeed  (TfContactShim, as G19).
//   [W71 20261003] G22 :17023        V912 bPauseAlarmDelayActive=false on RESUME, back with G15 (AI(W906-ST02-W71) 20261003; golden 0618 main.cpp:16402-16406 has no such clear).
//   G23 ATC_InterfaceForm is TATC_InterfaceFormShim (acarry_shims.h:109-115, iATC_MODE_TYPE only); the real class
//                     (forms/fATCHandlerSide.h:709) has no global.  One statement each:
//                     :17219 Set_ASIF_TJ_EFUSED  :17223 Get_ASIF_TJ_REQUEST  :17227 Get_ASIF_TJ_FB
//                     :17351 SetSLOPEOFFSET      :17400 dTJ[]                :17419 ATCCONTROLMODEMode
//                     :17428 asTJVoltage[]       :17441 SetAllTemp           :17454 RECORDTJTEMP
//                     :17465 asQueryTJTemp[][]   :17482 asControlMode[]      :17508-17510 ResetHandlerArmCache /
//                     TestFinish / HandlerArm    :17531-17532 SetTCWaterValve / ReadTCWaterValue
//                     :17549-17550 SetDynamicPID / ReadDynamicPID.
//                     The replies around them stay live (READTJ / GET_VOLTAGE / QUERYTJ / GET_ATCCONTROLMODE then
//                     answer with separators only).  SET_SLOPE_OFFSET / RESET_ATCALARM answer SETTINGNG instead of
//                     SETTINGOK while W906_ATC_PORTED==0 (user ruling item 7 = B, P2c 20260926).  The other gated
//                     set commands (SET_ATCCONTROLMODE / SET_ATC_TEMP / SET_WATER_VALVE / SET_DYNAMIC_PID /
//                     ASIF_TJ_EFUSED) have no Handler reply in golden, so there is no success claim to flip.
//   G24 :17567        `asGPIBAutoHeightZPos*100` -- AnsiString * int: vclcompat AnsiString has no operator* and the
//                     BCB6 resolution (a Variant conversion?) is unverified; golden never assigns asGPIBAutoHeightZPos
//                     (cmydef.cpp:5804 "" only).  READZPOS therefore gets no reply.  Needs a ruling.
//   [LIFTED S09-B3 20260929] G25 :17593-17597 (906 :16972-16976)  fObserver->iTestReceiveTimeCount -- was not a TfObserver member (now forms/fObserver.h:808; same gate as forms/fLotInfo.cpp:5751
//                     W906-LOT-W1-TESTRECVCNT); RecordReceiveTestTime() inside it goes with it.
//   G26 :17789        fMain->hanaART->DoCmdWhenHDStart -- TfMainHanaART (forms/fMain.h:80-96) has no such method.
//
//  NON-GATE ADAPTATIONS (each carries an AI note at its site)
//    * :16481 signature -> OnGpibProgramMsg(LPARAM lParam); :16539 `msg.LParam` -> `lParam`; :16540 HGpib2Handler
//      cast (rule 4); :16545 _OnMyCopyMsg_Interface(lParam) (Interface/InterfaceSYS.h:352); :16536 case label and
//      :17819 `break;` commented (the case IS the function; no other switch-level break exists in :16536-17819).
//    * :16528 `static iTTLRS232ErrorCount=0;` (BCB6 implicit int) -> `static int` (Handler static, rule 9).
//    * :17734 `BtnOneCycleClick(this)` -> `fMain->BtnOneCycleClick(fMain)` (`this` was the TfMain form).
//    * C `sprintf(HHandler2Gpib.Message, "%s...", <AnsiString>)` at :16240 :16284 :17279 :17412 :17432 :17473 :17489
//      -> `<AnsiString>.c_str()` (an AnsiString through C varargs does not compile in GCC; same bytes).
//    * :16858 / :16863 `fTemp_Set->SaveRemoteTempOffsetFromGPIB` NULL-guarded: wb_serve allocates fTemp_Set (tools/wb_serve.cpp:3111), ctests do not
//      (uTemp_Set.cpp:224); same idiom as MainTempMode.cpp:282-291.
//
//  GOLDEN QUIRKS KEPT (look like bugs; not "fixed")
//    Q1  :16516 `int iMap;` is not initialised; the GET_SLOPE_OFFSET / SET_SLOPE_OFFSET / READTJ loops read it when
//        IndexStatus / iGPIBIndexStatus is none of Z1Down_Z2Up / Z1Up_Z2Down / Z1_Z2_Down.  The same loops index
//        iSiteMapping_QualSite[8] with i*iRow+j+iATC_Use_Heat_Count/2, which passes 7 once iATC_Use_Heat_Count>=10.
//    Q2  :16230 `else if(iCommand==MSG_CMD_SVID || CosFunction.bGPIBUseSECSGENData)`: with bGPIBUseSECSGENData on,
//        EVERY packet not matched above it (ECID, RetestFlag, LotStatus, Auto_Clean, Pause, ONECYCLE, BINON results
//        ...) is answered as an SVID query and the later ProcessARTMessage branches become unreachable.
//    Q3  :16141 Buffer (containing the lot ID) is passed as the sprintf FORMAT string.
//    Q4  :16036-16053 the OFF_LINE and ON_LINE arms are identical.
//    Q5  :16358-16430 Auto_Clean / Pause / ONECYCLE / ECHOOK_ONECYCLE never set bResult=true, so ProcessARTMessage
//        returns false for them and OnGpibProgramMsg also tests MSG_CMD_SwitchArm (harmless: cannot match).
//    Q6  :16691 `Str3.Trim();` discards its result (no-op); :16702 `SubString(0, ...)` relies on BCB6 clamping 0->1
//        (vclcompat replicates it).  :16707 accepts channel 1..31 but `iret` is unbounded (writes gated by G19).
//    Q7  :17600-17608 testBin[] is zeroed and then immediately overwritten.
//    Q8  :17664 (ASE_KaohSiung only -- not defined, MachineType.h:67) writes iAutoSiteMapSocketPass[4][8] of a [4][8]
//        array.
//    Q9  :16284 the RetestFlag reply has no "\r\n" (every other reply in this file has one).
// ===========================================================================
#include "MachineDefine.h"                          // include hub: vclcompat umbrella + <windows.h> (COPYDATASTRUCT) +
                                                    //   `using namespace std;` -- first include, as Command.cpp / uTemp_Set.cpp
#include "TesterComm/Handler/HandlerTesterSide.h"   // THandlerTesterSide; fMain (forms/fMain.h); TQPF_Timer; VM / MV,
                                                    //   HGpib2Handler / HHandler2Gpib, MSG_CMD_*, GPIBVersionCheck / TTLRS232VerCheck
#include "MachineType.h"            // CC_*, eAuto1-3 / e3Auto1-3, eATC60 / eATC30 / eNewATCSystem, eIntel, rsm*,
                                    //   MAX_SOCKET_ROW / MAX_SOCKET_COL / MAX_SOCKET_TOTAL
#include "cmydef.h"                 // Handler globals: InitialOK, bSystemClose, IndexStatus, iContactMode, ATC_SYSTEM,
                                    //   CUSTOMER_CODE, K_RETRY, MMSystem, InterfaceType_*, RS232_MODE, ON_LINE / OFF_LINE,
                                    //   Z1*_Z2*, SystemHour.., TestIntervalsTime.., fAllMotorHome, the GPIB b*/i*/as* state
#include "cprod.h"                  // TestIF / TestIF_File / Prod / RunInfo / Temperature / WriteLastDataFile
#include "Config.h"                 // IniConfig
#include "CosFunction.h"            // CosFunction
#include "LastSet.h"                // LastSet
#include "aHotPlateSubstrate.h"     // TestSocket (NOT mykitsuck.h directly -- KNOWLEDGE.md two-TMyKitSuck gotcha),
                                    //   SetRunStartMode, MyDBIProcess
#include "ATC/ATCInterface.h"       // ATCInterfaceForm->ATC_60_SYS.SetTjOffset (the "old" ATC interface, live)
#include "acarry_shims.h"           // ATC_InterfaceForm (TATC_InterfaceFormShim: iATC_MODE_TYPE only -- see G23)
#include "forms/fLotInfo.h"         // fLotInfo->tsRFMD / ->btnCancelTestPause
#include "forms/fSCKART.h"          // fSCKART
#include "forms/fSortCT.h"          // fSortCT->ShowLoadingIC / ShowSortIC
#include "forms/fAGV.h"             // fAGV->IsATK_AMR
#include "forms/fTemp_Set.h"        // fTemp_Set->SaveRemoteTempOffsetFromGPIB (uTemp_Set.cpp:6371)
#include "atester_shims.h"          // fContact (TfContactShim, ->fShow)
#include "mymessbox_shim.h"         // MyMessageBox->fShow
#include "canary_support.h"         // ShowMyMessage / ShowErrorMessage / RecordProcess
#include "csystem.h"                // HasICUnderMachine / HasAnyICInMachine
#include "cpublic.h"                // EndTestTimeStamp
#include "cContact.h"               // CONTACT_NORMAL
#include "cSocket.h"                // LotSummary
#include "Automation/AMR.h"         // AMR.ARTReset
#include "AutoRetest.h"             // GPIB_RemoteCommand / GPIB_QueryData / GPIB_SetData
#include "Interface/InterfaceSYS.h" // _OnMyCopyMsg_Interface(LPARAM)
#include "forms/fATCHandlerSide.h"  // ATC_TYPE_35 / _36 / _60 / _70 (golden ATC_Handler_Side.h #defines).  LAST on
                                    //   purpose: its ATC_MAX_SITE / ATC_MAX_COMMAND / ATC_TYPE_* / iATC_Refrigerator_Num
                                    //   macros then touch no other header

#include <cstdio>                   // sprintf / strncpy
#include <cstdlib>                  // atoi / atof
#include <cstring>

//AI(W906-GB-P2c) 20260926: USER RULINGS 20260926 (decision list item 7, "B"). The switch stays 0 until
//   the thing it names is really wired in V906, then flip it to 1 to get golden's reply back (todo rows in
//   docs/TESTERCOMM_PORT_LEDGER.md "P2c rulings").
//   W906_ATC_PORTED           item 7 "在 ATC 接好之前改回「失敗」": ATC_InterfaceForm is still the shim (G23), so
//                             SET_SLOPE_OFFSET and RESET_ATCALARM (when it would have reached the ATC) answer
//                             SETTINGNG instead of golden's SETTINGOK.
//   (item 8's W906_REMOTE_START_WIRED went with golden 912's remote START arm: AI(W906-ST02-C912) 20261003 (St02-E helper): back to golden 906_0625 main.cpp:15847-15858 (RULINGS_20261002 #20 / #23-6))
#define W906_ATC_PORTED          0

// AI(W906-GB-P2a) 20260926: golden main.cpp:15690-15692 `bool bEcho=false, bUnderTest=false; bool bEchoStop=false;
//   bool bExist=false;` and :15694 `unsigned int iBin[4][8];` -- V906 defines them at atester_shims.cpp:101/:102/:120
//   and no header declares them (same local-extern shape as atester.cpp:650-652, HandlerBridgeCtl.cpp:70).
extern bool bEcho, bExist, bUnderTest;
extern bool bEchoStop;
extern unsigned int iBin[4][8];
// AI(W906-GB-P2a) 20260926: golden cMyDB.h:62 `void NewRecordProcess(AnsiString AlarmCode, AnsiString S,
//   AnsiString Debug=" ");` -- declared here, not by including cMyDB.h / acatchtray_shims.h, for the same
//   default-argument collision Command.cpp:270-282 documents (identical declaration).  V906 definitions:
//   cMyDB.cpp:1865, the only one since P4 06f8ef0a (the acatchtray_shims.cpp stand-in is deleted; AI(W906-P4) 20260928).
void NewRecordProcess(AnsiString AlarmCode, AnsiString S, AnsiString Debug=" ");

/* ============ golden 906 main.cpp:7603  ResetForESC  (912 :8027-8038; 906 line from NB2 R93, not re-verified on St02) ============ */
//------------------------------------------------------------------------------
void THandlerTesterSide::ResetForESC(AnsiString Msg)                            //Steven 20201022 : For RFMD Empty Socket Check Funstion.
{
    if(TestIF_File.iGpibMode==InterfaceType_15BinQorvo)                         //Steven 20201022 : For RFMD
    {
        if(bDoEmptySocketOneCycle==false)
        {
            bTriggerESC=true;
            RecordProcess(Msg);
        }
    }
}

/* ============ golden 906 0618 main.cpp:7615-7706  ProcessForESC  (912 :8040-8131 identical) ============ */
// AI(W906-ST02-ESC) 20261005 (St02-E): census 129 E-T1-007 -- the only reader of bTriggerESC (set by ResetForESC above, the Qorvo
//   CHECKEMPTY path :1308; golden clears it only here and in the TfMain ctor :1344 = HandlerTesterSide.cpp ctor).  Only caller:
//   golden TfMain::Timer1Timer :2858 = MainTimersSt02.cpp Timer1BinTick through ht9045::W906_ESCProcessHook (ProcessForESCHook
//   below).  Rule 2: RunTestProgram / CloseGpibProgram / bTriggerESC are this object's; other TfMain members via fMain->.
//   Golden text line by line (cp950-decoded); the only edits are marked.  The four CheckDestoryFinish() conjuncts need the FULL
//   TMyKitSuck (mykitsuck.h) and this TU sees the minimal mirror (aHotPlateSubstrate.h; docs/KNOWLEDGE.md two-TMyKitSuck), so
//   they are W906_EscDestroysFinished in HandlerEscSuck.cpp -- golden order, short-circuit kept (CheckDestoryFinish clears an
//   empty site's iNozzleEvent, golden MyKitSuck.cpp:2870-2871).
bool W906_EscDestroysFinished();   // HandlerEscSuck.cpp
extern int OutArmTask;            // aoutarm.h:77 (not included here: TU-local extern, as :176-178)
//------------------------------------------------------------------------------
void THandlerTesterSide::ProcessForESC()                                        //Steven 20260121 : For RFMD Empty Socket Check Funstion.
{                                                                               //Steven 20220831 : 針對已測的, 還是要分Bin, 只針對未測的設定成Error Bin
    AnsiString asString;                                                        //Steven 20220817 : Bin of ESC function iTestBinCount --> IniConfig.iI41_BinOfESC
    if(TestIF_File.iGpibMode==InterfaceType_15BinQorvo &&                       //Steven 20201022 : For RFMD
       bTriggerESC==true &&
       IsTest==false &&
       TestSocket.HasIC()==false &&
       W906_EscDestroysFinished())                                              // AI(W906-ST02-ESC) 20261005 (St02-E): golden :7622-7625 In/Out/FTest/BTest CheckDestoryFinish()==true, same order, short-circuit kept (HandlerEscSuck.cpp)
    {
        iESC_IndexContactCount=0;
        bDoEmptySocketOneCycle=true;
        IsTest=false;                                                           //JerryYang 20160318 增加防護 RESET時要重置GPIB
        RunTestProgram(false);

        if(fMain->BtnOneCycle->Down==false)
        {
            InitOneCycle("Reset For ESC");
            fMain->BtnOneCycle->Down=true;
        }
        bResetMode=true;

        for(int i=0; i<MAX_Index_Row; i++)
        {
            for(int j=0; j<NEW_MAX_Index_Col; j++)
            {
                if(FTestSuck.Item[i][j]!=NULL_IC &&
                   FTestSuck.Item[i][j]!=HAS_NULL_IC &&
                   FTestSuck.Item[i][j]<TEST_PASS)
                {
                    FTestSuck.iBinData[i][j]=IniConfig.iI41_BinOfESC;
                    FTestSuck.SetItemData(i, j, TEST_PASS+IniConfig.iI41_BinOfESC);
                    FTestSuck.PordRec[i][j].AddTestResultRecord(IniConfig.iI41_BinOfESC, FTestSuck.cSBin[i][j], "RESET");                                       //Frank 20160505 add
                }

                if(BTestSuck.Item[i][j]!=NULL_IC &&
                   BTestSuck.Item[i][j]!=HAS_NULL_IC &&
                   BTestSuck.Item[i][j]<TEST_PASS)
                {
                    BTestSuck.iBinData[i][j]=IniConfig.iI41_BinOfESC;
                    BTestSuck.SetItemData(i, j, TEST_PASS+IniConfig.iI41_BinOfESC);
                    BTestSuck.PordRec[i][j].AddTestResultRecord(IniConfig.iI41_BinOfESC, BTestSuck.cSBin[i][j], "RESET");                                       //Frank 20160505 add
                }

                if(TestSocket.Item[i][j]!=NULL_IC && TestSocket.Item[i][j]!=HAS_NULL_IC &&
                   TestSocket.Item[i][j]<TEST_PASS)
                {
                    TestSocket.iBinData[i][j]=IniConfig.iI41_BinOfESC;
                    TestSocket.SetItemData(i, j, TEST_PASS+IniConfig.iI41_BinOfESC);
                    TestSocket.PordRec[i][j].AddTestResultRecord(IniConfig.iI41_BinOfESC, TestSocket.cSBin[i][j], "RESET");                                     //Frank 20160505 add
                }
            }
        }

        if(bBin16HangUp)
        {
            CloseGpibProgram(__FUNC__);                                         // AI(W906-ST02-ESC) 20261005 (St02-E): golden fMain-> -- this object (rule 2, as ResetForESC)
            bBin16HangUp=false;
        }

        OutArmTask=1;
        bLampReset=true;
        iWhoTriggerPiggyBack=pbtReset;                                          //Steven 20111207 : 誰觸發了Piggy Back

        asString.sprintf("Reset Record: InArm=%d , OutArm=%d , InSH1=%d , InSH2=%d",
                          InArmSuck.CountRealIC(),
                          OutArmSuck.CountRealIC(),
                          FLCarryKit.CountRealIC(),
                          BLCarryKit.CountRealIC());
        MyDBIProcess("Message", asString);
        asString.sprintf("Reset Record: OutSH1=%d , OutSH2=%d , Index1=%d , Index2=%d",
                          FRCarryKit.CountRealIC(),
                          BRCarryKit.CountRealIC(),
                          FTestSuck.CountRealIC(),
                          BTestSuck.CountRealIC());
        MyDBIProcess("Message", asString);
        asString.sprintf("Reset Record: Total=%d",
                          InArmSuck.CountRealIC()+
                          OutArmSuck.CountRealIC()+
                          FLCarryKit.CountRealIC()+
                          BLCarryKit.CountRealIC()+
                          FRCarryKit.CountRealIC()+
                          BRCarryKit.CountRealIC()+
                          FTestSuck.CountRealIC()+
                          BTestSuck.CountRealIC());
        MyDBIProcess("Message", asString);
        RecordProcess("Start RESET by Empty Socket Check...");
        bTriggerESC=false;
    }
}
//------------------------------------------------------------------------------
void THandlerTesterSide::ProcessForESCHook()                                    // AI(W906-ST02-ESC) 20261005 (St02-E): the ht9045::W906_ESCProcessHook body
{
    if (fTesterSide)
        fTesterSide->ProcessForESC();
}

/* ====== golden 906 main.cpp:15409  ProcessARTMessage  (912 :15973-16475; 906 line from NB2 R93, not re-verified on St02) ====== */
//------------------------------------------------------------------------------
bool THandlerTesterSide::ProcessARTMessage()
{
    bool bResult=false;
//    bool bChangeLotID=false;
    AnsiString Buffer, ARespone;
    AnsiString Str;
    AnsiString strbuf="";                                                       //ChungHung 20150122 add for TSMC Use Tj
    int iret=0;                                                                 //kevin 20141017

    //Steven 20161025 (wei) : SCK ART function ==>
    if(HGpib2Handler->iCommand==MSG_CMD_SCKART_LOTCLEAR)
    {
        bReadLotInfoFromART=true;
        bResult=true;
        sprintf(HHandler2Gpib.Message, "LOTCLEARED\r\n");
        fSCKART->iWaitGPIBLotR=0;
        ZeroMemory(iAutoTrayCount, sizeof(iAutoTrayCount));                     //Sam 20191113 : TCP ART
        if(HasICUnderMachine()==false)
        {
            LastSet.bBreakSCKART=false;                                         //Sam 20240402 : 修正 BreakSCKART 問題。
            fMain->Clarn_Data(1, "ART_LOTCLEARED");
            fSCKART->ClearLotInfo();
            RecordProcess("ART LOTCLEARED.");
//#if 0 // TODO(W906-GB-P2a): G1 TfLotInfo has no btClearBarcodeListClick (golden uLotInfo.cpp:10005-10014); btClearBarcodeList->Click() is a vclcompat no-op (rule 8) -- golden main.cpp:15997   //AI(W906-TC-G1G5) 20261001 (St02-E): STALE -- the gate is retired, TfLotInfo::btClearBarcodeListClick exists now (forms/fLotInfo.cpp:6553)
            fLotInfo->btClearBarcodeListClick();   //Steven 20190214 : 統一清除2DID方式   //AI(W906-TC-G1G5) 20261001 (St02-E): golden 906_0625_Steven main.cpp:15432 fLotInfo->btClearBarcodeList->Click() -- the VCL Click fires OnClick = TfLotInfo::btClearBarcodeListClick (uLotInfo.cpp:10005-10014); the port's button stand-in has no OnClick, so the handler is called (St01's seat forms/fLotInfo.cpp:6550 names this caller)
//#endif   // G1 (AI(W906-TC-G1G5) 20261001 (St02-E): retired)
        }

        if((CosFunction.bUseSCKART &&
            IniConfig.bA10_AutoReTest &&
            TestIF_File.bSCKART_EnableART) ||                                   //Steven 20190719 : 避免ART命令被誤用
            (CUSTOMER_CODE==CC_SCK || CUSTOMER_CODE==CC_SCS))                   //Steven 20190719 : 部分命令沒裝ART只開放SCK
        {
            SendMSG_CMD(MSG_CMD_SCKART_LOTCLEAR);
        }
    }
    else if(HGpib2Handler->iCommand==MSG_CMD_SCKART_LOTRTCLEAR)
    {
        bReadLotInfoFromART=true;
        bResult=true;
        sprintf(HHandler2Gpib.Message, "LOTRETESTCLEARED\r\n");
        if(CUSTOMER_CODE==CC_ASE_KaohSiung)                                     //kevin 20210903 add  not use LOTRETESTCLEARED command
        {
            RecordProcess("LOTRETESTCLEARED not use for ASE_KH ");
            return bResult;
        }

        if(CosFunction.bUseSCKART &&
           IniConfig.bA10_AutoReTest &&
           TestIF_File.bSCKART_EnableART)                                       //Steven 20190719 : 避免ART命令被誤用
        {
            SendMSG_CMD(MSG_CMD_SCKART_LOTRTCLEAR);
        }

        if(fSCKART->iTesterType==1 ||
#if 1 // AI(W906-W132) 20261007 (St02-E): G2 lifted -- TfSCKART has iCurrentStatus / iLOTSTATUS_A now (forms/fSCKART.h); golden 0618 main.cpp:15461-15462.  Was: TODO(W906-GB-P2a) G2 not TfSCKART members
           fSCKART->iCurrentStatus!=fSCKART->iLOTSTATUS_A)
#else
           false)  //AI(W906-GB-P2a) 20260926: unknown disjunct -> the if-arm runs only on iTesterType==1 (golden short-circuit, exact); the else arm needs the same unknown value and is gated with it (G6)   [STALE since W-132: dead #else arm]
#endif
        {
            fMain->Clarn_Data(2, "ART_LOTRETESTCLEARED");
            if(fSCKART->iFTRTCount>TestIF_File.iSCKART_TryCnt && IniConfig.bA60EnableAMR)
            {
                //Sam 20250702 : Final RT 完成後，收到就不要再清除 AutoRetest Count
            }
            else
            {
                if(LastSet.iTester==OFF_LINE)
                {
                    if(Prod.bART6Tray[eAuto1])
                        LastSet.BinCT[0][e3Auto1]=0;
                    if(Prod.bART6Tray[eAuto2])
                        LastSet.BinCT[0][e3Auto2]=0;
                    if(Prod.bART6Tray[eAuto3])
                        LastSet.BinCT[0][e3Auto3]=0;
                }
                else
                {
                    if(Prod.bART6Tray[eAuto1])
                        LastSet.BinCT[0][e3Auto1]=0;
                    if(Prod.bART6Tray[eAuto2])
                        LastSet.BinCT[0][e3Auto2]=0;
                    if(Prod.bART6Tray[eAuto3])
                        LastSet.BinCT[0][e3Auto3]=0;
                }
            }

            AMR.ARTReset();                                                     //Sam 20240304 : 新增 AMR 功能
            for(int i=0; i<10; i++)
            {
                LastSet.lSCKARTBinCT[i]=0;
            }
            LastSet.iSCKARTInputCT=0;
            LastSet.lShuttleCount=0;
            LotSummary.ClearRTData();

//AI(W906-GB-P2c) 20260926: gate G3 lifted -- USER RULING 20260926 (decision list item 6 = B) "打開照 golden 寫":
//   writes D:\HT9045\system\lastdata.dat exactly as golden main.cpp:16065 (only on MSG_CMD_SCKART_LOTRTCLEAR;
//   no ctest sends it)
            WriteLastDataFile(false);                                           //kevin 20141030
            fSortCT->ShowLoadingIC();
            fSortCT->ShowSortIC();

            if(fSCKART->iTesterType==0)
            {
                fSCKART->iInputCount=LastSet.iSCKART_RTUnitCount;
                LastSet.iSCKART_RTUnitCount=0;
            }
            fSCKART->iInputJamCnt    =0;
            fSCKART->iOutputJamCnt   =0;

//#if 0 // TODO(W906-GB-P2a): G4 fSCKART->iCurrentStatus not a TfSCKART member -- golden main.cpp:16077-16081   //AI(W906-W132) 20261007 (St02-E): G4 lifted, golden 0618 main.cpp:15512-15516
            if(fSCKART->iCurrentStatus==fSCKART->iLOTSTATUS_R)
            {
                fSCKART->SetLotStatus(fSCKART->iLOTSTATUS_W);
                fSCKART->iWaitGPIBLotR=3;
            }
//#endif   // G4 (AI(W906-W132) 20261007 (St02-E): lifted)
            fSCKART->AccessFile(false, -1);
            RecordProcess("ART LOTRETESTCLEARED.");
//#if 0 // TODO(W906-GB-P2a): G5 same as G1 (btClearBarcodeListClick) -- golden main.cpp:16084   //AI(W906-TC-G1G5) 20261001 (St02-E): STALE -- retired with G1 (:229)
            fLotInfo->btClearBarcodeListClick();   //Steven 20190214 : 統一清除2DID方式   //AI(W906-TC-G1G5) 20261001 (St02-E): golden 906_0625_Steven main.cpp:15519 fLotInfo->btClearBarcodeList->Click() -- the VCL Click fires OnClick = TfLotInfo::btClearBarcodeListClick (uLotInfo.cpp:10005-10014); the port's button stand-in has no OnClick, so the handler is called (St01's seat forms/fLotInfo.cpp:6550 names this caller)
//#endif   // G5 (AI(W906-TC-G1G5) 20261001 (St02-E): retired)
        }
//#if 0 // TODO(W906-GB-P2a): G6 else arm of G2: reached only when iCurrentStatus==iLOTSTATUS_A (not TfSCKART members) -- golden main.cpp:16086-16090   //AI(W906-W132) 20261007 (St02-E): G6 lifted, golden 0618 main.cpp:15521-15525
        else
        {
            fSCKART->SetLotStatus(fSCKART->iLOTSTATUS_W);
            fSCKART->iCurrentFlexARTStep=10;
        }
//#endif   // G6 (AI(W906-W132) 20261007 (St02-E): lifted)
    }
    else if(HGpib2Handler->iCommand==MSG_CMD_SCKART_INPUTQTY)
    {
        bResult=true;
        AnsiString sLotID=AnsiString().sprintf("%s", HGpib2Handler->GpibData);
        AnsiString sProcessCode=AnsiString().sprintf("%s", HGpib2Handler->cReturn);
        int iLotCount=HGpib2Handler->GPIBBin;
        fSCKART->DoARTLotStart(sLotID, sProcessCode, iLotCount);
    }
    else if(HGpib2Handler->iCommand==MSG_CMD_SCKART_Alarm)
    {
        bResult=true;
#if 0 // TODO(W906-GB-P2a): G7 fSCKART->iCurrentStatus / ->iLOTSTATUS_A / ->iLOTSTATUS_F not TfSCKART members -- golden main.cpp:16103-16119   //AI(W906-W132) 20261007 (St02-E): iCurrentStatus / iLOTSTATUS_A exist now; still missing iLOTSTATUS_F, so G7 stays   //AI(W906-W213) 20261010 (St02-E): members present now (_A W-132, _F W-214) but STAYS CLOSED -- TfSCKART::CheckNeedRT is an offline no-op (forms/fSCKART.h:176-183) that never updates iNeedRT, so every SCKART_Alarm would finalize (LOTSTATUS_F) instead of retest (R); open it with the CheckNeedRT engine (golden 913 SCK_ART.cpp:1072-1230)
        if(fSCKART->iCurrentStatus!=fSCKART->iLOTSTATUS_A)
        {
            fSCKART->CheckNeedRT();                                             //For Flex tester
            fSCKART->iWaitGPIBLotR=2;
            if(fSCKART->iNeedRT>0)
            {
                fSCKART->iCurrentFlexARTStep=7;
                fSCKART->DoAutoSocketOff(false);
                fSCKART->SetLotStatus(fSCKART->iLOTSTATUS_R);
            }
            else
            {
                fSCKART->iCurrentFlexARTStep=14;
                fSCKART->DoAutoSocketOff(true);
                fSCKART->SetLotStatus(fSCKART->iLOTSTATUS_F);
            }
        }
#endif
        fSCKART->AccessFile(false, 10);
    }
    else if(HGpib2Handler->iCommand==MSG_CMD_SCKART_LOTSTATUS)
    {
        bResult=true;
        fSCKART->iTesterType=0;  PublishSettings();   //AI(W906-GB-P6) 20260926: 3A -- the engine sees the learned brand before this reply returns
        if(fMain->palMainStatus->Caption=="HALT" && fSCKART->sLotID=="")
        {
            fSCKART->SetLotStatus(fSCKART->iLOTSTATUS_NONE);
        }

        if(fSCKART->sLotID=="")
        {
            fSCKART->iFTRTCount=0;
//#if 0 // TODO(W906-GB-P2a): G8 fSCKART->GetLotStatus() not a TfSCKART member (Buffer stays "") -- golden main.cpp:16134   //AI(W906-W213) 20261010 (St02-E): G8 lifted, golden 913 main.cpp:16278 (TfSCKART::GetLotStatus, W-214)
            Buffer.sprintf("0,0,%s", fSCKART->GetLotStatus());
//#endif   // G8 (AI(W906-W213) 20261010 (St02-E): lifted)
        }
        else
        {
//#if 0 // TODO(W906-GB-P2a): G9 fSCKART->GetLotStatus() not a TfSCKART member (Buffer stays "") -- golden main.cpp:16138   //AI(W906-W213) 20261010 (St02-E): G9 lifted, golden 913 main.cpp:16282
            Buffer.sprintf("%d,%s,%s", fSCKART->iFTRTCount, fSCKART->sLotID, fSCKART->GetLotStatus());
//#endif   // G9 (AI(W906-W213) 20261010 (St02-E): lifted)
        }

        //AI(W906-GB-P2a) 20260926: golden passes Buffer as the FORMAT string (a "%" in the lot ID would be read as a conversion); kept as golden
        sprintf(HHandler2Gpib.Message, Buffer.c_str());

        if(CosFunction.bUseSCKART &&
           IniConfig.bA10_AutoReTest &&
           TestIF_File.bSCKART_EnableART)                                       //Steven 20190719 : 避免ART命令被誤用
        {
            SendMSG_CMD(MSG_CMD_SCKART_LOTSTATUS);
        }

//#if 0 // TODO(W906-GB-P2a): G10 fSCKART->iCurrentStatus / ->iLOTSTATUS_L / ->iLOTSTATUS_F not TfSCKART members -- golden main.cpp:16150-16163   //AI(W906-W132) 20261007 (St02-E): iCurrentStatus exists now; still missing iLOTSTATUS_L / iLOTSTATUS_F, so G10 stays   //AI(W906-W213) 20261010 (St02-E): G10 lifted, golden 913 main.cpp:16294-16307 (iLOTSTATUS_L / _F on TfSCKART, W-214)
        if(fSCKART->iCurrentStatus==fSCKART->iLOTSTATUS_L)
        {
        }
        else if(fSCKART->iCurrentStatus==fSCKART->iLOTSTATUS_F)
        {
            if(HasICUnderMachine())
            {
                fSCKART->SetLotStatus(fSCKART->iLOTSTATUS_W);
            }
            else
            {
                fSCKART->iWaitGPIBLotR=4;
            }
        }
//#endif   // G10 (AI(W906-W213) 20261010 (St02-E): lifted)
        fSCKART->AccessFile(false, 0);
    }
    else if(HGpib2Handler->iCommand==MSG_CMD_SCKART_SRQMASK)
    {
        bResult=true;
        fSCKART->iCurrent93KARTStep=0;
        fSCKART->iTesterType=1;  PublishSettings();   //AI(W906-GB-P6) 20260926: 3A, as above
//        FUNC_CC_SCK();
        if(fSCKART!=NULL && fSCKART->iTesterType==1)                            //Steven 20161201 (wei) : For SCK 93K ART
            CosFunction.bAutoRetestGPIBmode         =true;
        else
            CosFunction.bAutoRetestGPIBmode         =false;

        fSCKART->AccessFile(false, 10);
    }
    else if(HGpib2Handler->iCommand==MSG_CMD_SCKART_QTY)
    {
        bResult=true;
        if(HasICUnderMachine() || fSCKART->sLotID!="")
        {
            if(fSCKART->iFTRTCount==0)
            {
                fSCKART->iCurrentFlexARTStep=2;
            }
            else
            {
                fSCKART->iCurrentFlexARTStep=9;
            }
            sprintf(HHandler2Gpib.Message, "SETTINGOK\r\n");
        }
        else
        {
            fSCKART->iCurrentFlexARTStep=2;
            sprintf(HHandler2Gpib.Message, "SETTINGNG\r\n");
        }

        if(CosFunction.bUseSCKART &&
           IniConfig.bA10_AutoReTest &&
           TestIF_File.bSCKART_EnableART)                                       //Steven 20190719 : 避免ART命令被誤用
        {
            SendMSG_CMD(MSG_CMD_SCKART_QTY);
        }
    }
    else if(HGpib2Handler->iCommand==MSG_CMD_SCKART_INITIAL)
    {
        bResult=true;
//#if 0 // TODO(W906-GB-P2a): G11 fSCKART->iCurrentStatus / ->iLOTSTATUS_A / ->iLOTSTATUS_F not TfSCKART members -- golden main.cpp:16210-16215   //AI(W906-W132) 20261007 (St02-E): iCurrentStatus / iLOTSTATUS_A exist now; still missing iLOTSTATUS_F, so G11 stays   //AI(W906-W213) 20261010 (St02-E): G11 lifted, golden 913 main.cpp:16354-16359
        if(fSCKART->iCurrentStatus==fSCKART->iLOTSTATUS_A ||
           fSCKART->iCurrentStatus==fSCKART->iLOTSTATUS_W)
        {
            fSCKART->SetLotStatus(fSCKART->iLOTSTATUS_F);
            fSCKART->AccessFile(false, 1);
        }
//#endif   // G11 (AI(W906-W213) 20261010 (St02-E): lifted)
    }
    //Steven 20161025 (wei) : SCK ART function <==
    else if(HGpib2Handler->iCommand==MSG_CMD_RCMD)
    {
        bResult=true;
        asRecordTestResult=AnsiString(HGpib2Handler->cReturn);
        iret=GPIB_RemoteCommand(asRecordTestResult);
        if(iret==0)
            sprintf(HHandler2Gpib.Message, "ECHOOK\r\n");
        else
            sprintf(HHandler2Gpib.Message, "ECHONG\r\n");

        SendMSG_CMD(MSG_CMD_RCMD);
    }
    else if(HGpib2Handler->iCommand==MSG_CMD_SVID ||
            CosFunction.bGPIBUseSECSGENData)                                    //Sam 20240826 : GPIB 通訊資料使用 SECSGEM Data
    {
        bResult=true;
        asRecordTestResult=AnsiString(HGpib2Handler->cReturn);

        if(IniConfig.bEnable_SECS_GEM ||
           CosFunction.bGPIBUseSECSGENData==true)                               //Sam 20240826 : GPIB 通訊資料使用 SECSGEM Data
        {
            asRecordTestResult=GPIB_QueryData(asRecordTestResult);
            //AI(W906-GB-P2a) 20260926: C sprintf with an AnsiString vararg is ill-formed in GCC -> .c_str() (same bytes)
            sprintf(HHandler2Gpib.Message, "%s\r\n",asRecordTestResult.c_str());
        }
        else
        {
            IniConfig.bEnable_SECS_GEM=true;                                    //Frank 20170815 (Steven) add Xilinx ART 保護
            sprintf(HHandler2Gpib.Message, "ECHONG\r\n");
            MyDBIProcess("Message", "未開啟SECS GEM,強制開啟");
        }
        SendMSG_CMD(MSG_CMD_SVID);
    }
    else if(HGpib2Handler->iCommand==MSG_CMD_ECID)
    {
        bResult=true;
        asRecordTestResult=AnsiString(HGpib2Handler->cReturn);
        iret=GPIB_SetData(asRecordTestResult);
        if(iret==0)
            sprintf(HHandler2Gpib.Message, "ECHOOK\r\n");
        else
            sprintf(HHandler2Gpib.Message, "ECHONG\r\n");

       SendMSG_CMD(MSG_CMD_ECID);
    }
    else if(HGpib2Handler->iCommand==MSG_CMD_RetestFlag)
    {
        bResult=true;
        asRecordTestResult=AnsiString(HGpib2Handler->cReturn);

        if(asRecordTestResult.Pos("00")!=0)
        {
            LastSet.iRetestFlagART=3;
        }
        else if(asRecordTestResult.Pos("11")!=0)
        {
            LastSet.iRetestFlagART=1;
        }
        else if(asRecordTestResult.Pos("12")!=0)
        {
            LastSet.iRetestFlagART=2;
        }
        else
        {
            LastSet.iRetestFlagART=-1;
        }

        //AI(W906-GB-P2a) 20260926: C sprintf with an AnsiString vararg is ill-formed in GCC -> .c_str() (same bytes)
        sprintf(HHandler2Gpib.Message, "%s", asRecordTestResult.c_str());

        SendMSG_CMD(MSG_CMD_RetestFlag);
    }
    else if(HGpib2Handler->iCommand==MSG_CMD_LotStatus)
    {
        bResult=true;
        asRecordTestResult=AnsiString(HGpib2Handler->cReturn);

        if(CUSTOMER_CODE==CC_UTAC)                                              //Richard 20220929 :Add for UTAC
        {
            if(asRecordTestResult.Pos("0")!=0)
            {
                LastSet.bWaitStartLotAutoRetestGPIB=true;
            }
            else if(asRecordTestResult.Pos("1")!=0)
            {
                ShowErrorMessage("WAR16327", K_RETRY, MMSystem);
                if(LastSet.iRunStartMode==rsmContinuRetest)
                {
                    SetRunStartMode(rsmCInitialRetest);
                }
                else
                {
                    SetRunStartMode(rsmInitialStart);
                }
                LastSet.bWaitStartLotAutoRetestGPIB=false;
            }
            else
            {
                LastSet.bWaitStartLotAutoRetestGPIB=true;
            }
        }
        else
        {
            if(asRecordTestResult.Pos("0")!=0 ||
               asRecordTestResult.Pos("1")!=0)
            {
                if(fSCKART->iFTRTCount==0)
                {
                    fSCKART->iCurrent93KARTStep=3;
                }
                else
                {
                    fSCKART->iCurrent93KARTStep=9;
                }

//                if(TestIF_File.iGpibMode==InterfaceType_15BinQorvo)             //Steven 20250605 : Qorvo 要等到執行ESC後才能
//                {                                                             //Steven 20250821 : Qorvo 又要改回來
//                    ;
//                }
                if(fAGV->IsATK_AMR()==true &&                                   //RogerYang 20260402 : ATK_AMR need wait for SECS LOT_START
#if 0 // TODO(W906-GB-P2a): G12 fAGV->bATK_AMR_DoHostLotStart not a TfAGV member (forms/fAGV.h); nothing in V906 can set it, so the conjunct is its golden initial value (false==false) -- golden main.cpp:16336
                    fAGV->bATK_AMR_DoHostLotStart==false &&
#endif
                    asRecordTestResult.Pos("LOTORDER")!=0)                      //LOTORDER 0
                {
#if 0 // TODO(W906-GB-P2a): G13 fAGV->bATKAMR_GET_LOTORDER0_Ready not a TfAGV member -- golden main.cpp:16339
                    fAGV->bATKAMR_GET_LOTORDER0_Ready=true;
#endif
                }
                else
//                else
                {
                    LastSet.bWaitStartLotAutoRetestGPIB=true;                   //JSCK ART Lot Start from Tester
                }
            }
            else if(asRecordTestResult.Pos("2")!=0 ||
                    asRecordTestResult.Pos("3")!=0)
            {
                if(fSCKART->iCurrent93KARTStep>=10)
                {
                    fSCKART->iCurrent93KARTStep=12;
                }
                LastSet.bWaitEndLotAutoRetestGPIB=true;                         //JSCK ART Lot End from Tester
            }
        }
    }
    else if(HGpib2Handler->iCommand==MSG_CMD_Auto_Clean)                        //wei 20180309
    {
        bGPIBAutoClean=true;
#if 0 // TODO(W906-GB-P2a): G14 TfShowBinSelect has no btnAutoCleanClick (forms/fShowBinSelect.h:130 SAFETY-QUEUED, untranslated) -- golden main.cpp:16361
        fShowBinSelect->btnAutoCleanClick(fShowBinSelect);
#endif
        RecordProcess("GPIB Command AUTO_CLEAN");
    }
    else if(HGpib2Handler->iCommand==MSG_CMD_Pause)                             //wei 20180309
    {
        if(TestIF_File.iGpibMode==InterfaceType_15BinQorvo)                     //Steven 20201022 : For RFMD
        {
            fLotInfo->tsRFMD->TabVisible=true;
            fLotInfo->btnCancelTestPause->Enabled=true;
            bTesterSendPause=true;

//AI(W906-ST02-W71) 20261003 (St02-E): V912 main.cpp:16371-16381 kept as a fix (RogerYang 20260626: the Qorvo Tester Pause buzzer waits Max Test Time) -- RULINGS_20261003 #1 + Steven W8 = A (ST01-M CHAT_ST02 10:20, W71); golden 0618 main.cpp:15806 sets bTesterPauseMusic=true at once instead.  Consumers ckernel.cpp:1638 / :3374 / :3382 = V912 ckernel.cpp:744-748 / :2144-2148 / :2158-2162 (golden 0618 ckernel.cpp:738 / :2113 / :2121, read in the 0618 tree).
            if(TestIF.iMaxTime>0)                                               //RogerYang 20260626 : iMaxTime<=0 防呆    //Steven 20220616 : Can select "Alarm Reset" when show "Tester Pause" for QORVO.
            {
                bTesterPauseMusic=false;                                        //RogerYang 20260626 : 先不響，由 hPauseAlarmDelay 計時
                hPauseAlarmDelay.SetSecAndOn(TestIF.iMaxTime);                  //RogerYang 20260626 : arm 逾時計時 = Max Test Time (Timer ReStart)
                bPauseAlarmDelayActive=true;
            }
            else
            {
                bTesterPauseMusic=true;                                         //RogerYang 20260626 : 無 MaxTime 設定維持舊行為(立即響)
            }
            RecordProcess("GPIB Command Pause");
        }
        else
        {
            if(CosFunction.bGPIB_Command_DOPAUSE==true)                         //Jimmychiu 20231102 : GPIB Command SETHANDLERDOPAUSE
            {
                fMain->BtnPauseClick(fMain);
            }
            else
            {
                fMain->BtnOneCycleClick(fMain);
                bGPIBPause=true;
            }
        }
        RecordProcess("GPIB Command PAUSE");
    }
     else if(HGpib2Handler->iCommand==MSG_CMD_ONECYCLE)                         //KaiChen 20180910 ：Add GPIB ONECYCLE
    {
        char str[256];                                                          //Sam 20221103 : OneCycle 完後顯示訊息
        strncpy(str, HGpib2Handler->cReturn, sizeof(str));
        Str=AnsiString(str).Trim();
        if(CUSTOMER_CODE==CC_UTAC_TW && Str=="")                                //Sam 20230815 : 聯測改為 Interval time
        {
            Str="ONECYCLE by OI_GPIB";
        }

        if(Str!="")
        {
            iOneCycleFinishShowMsg=1;                                           //Sam 20250115 : 矽格湖口 GPIB OneCycle 要強制切 ASM
            sOneCycleFinishShowMsg=Str;
            RecordProcess("GPIB Command ONECYCLE "+Str);
        }
        else
        {
            iOneCycleFinishShowMsg=2;                                           //Sam 20250115 : 矽格湖口 GPIB OneCycle 要強制切 ASM
            sOneCycleFinishShowMsg="";
            RecordProcess("GPIB Command ONECYCLE");
        }
        fMain->BtnOneCycleClick(fMain);
    }
    else if(HGpib2Handler->iCommand==MSG_CMD_ECHOOK_ONECYCLE)                   //KaiChen 20181114 ：Add GPIB ECHOOK:ONECYCLE
    {
        RecordProcess("GPIB Command ECHOOK:ONECYCLE");
        if(fMain->palMainStatus->Caption=="Running" ||
           fMain->palMainStatus->Caption=="PAUSE")
        {
            fMain->BtnOneCycleClick(fMain);
        }
    }
//AI(W906-ST02-C912) 20261003 (St02-E helper): back to golden 906_0625 main.cpp:15855-15857 (RULINGS_20261002 #20 / #23-6) -- golden 912's MSG_CMD_RemoteStart / RemoteStop arms (912 main.cpp:16431-16472) removed: an iCommand no arm names gets no reply

    return bResult;
}
/* ================ golden main.cpp:16476-16530 + :16536-17819  OnMyCopyMsg -> OnGpibProgramMsg ================ */
//******************************************************************************
//
//  注意!! OnMyCopyMsg為Handler與GPIB, ESD相關, 修改時要小心!!
//
//******************************************************************************
//AI(W906-GB-P2a) 20260926: golden `void __fastcall TfMain::OnMyCopyMsg(Messages::TMessage &msg)`; this function is its preamble (:16481-16530)
//AI(W906-GB-P2a) 20260926: plus the body of `case WM_GPIB_Program:` (:16536-17819).  The switch (:16531-16535) and the other cases (WM_GPIBInterface, WM_ASEKH_Program ...)
//AI(W906-GB-P2a) 20260926: are not here: the hub delivers only bridge packets (WParam WM_GPIB_Program) to THandlerTesterSide::Sink (HandlerTesterSide.cpp).
void THandlerTesterSide::OnGpibProgramMsg(LPARAM lParam)
{
//Steven 20150730 : 重整OnMyCopyMsg
//kevin 20130425   2013.01.11 Q_Q TSMC GPIB COMMAND
// Note:
// ECHO =>
// MESS =>
// GTMP =>
// STMP =>
// GSKT => 問SOAK TIME
// SSKT => 設定SOAKTIME
// GSMP => 問 MAP
// GMAP => 問 MAP (不同格式)

// GSOC => 問關SITE
// SSOC => 設定關SITE
// TMOK => 問溫度是否OK

// GHID => Get Handler ID   ( 問Handler 設定的ID )      //2013.01.11 Q_Q TSMC GPIB COMMAND
// GPET => Get Persite Temp ( 問測試ARM的溫度 )         //2013.01.11 Q_Q TSMC GPIB COMMAND  //2013.01.24 Q_Q TSMC GPIB COMMAND Part 2.
// GARM => Get ARM                                      //2013.01.11 Q_Q TSMC GPIB COMMAND
// GAT1 => Get ARM1 Temp                                //2013.01.11 Q_Q TSMC GPIB COMMAND    //2013.01.24 Q_Q TSMC GPIB COMMAND Part 2. 不需要
// GAT2 => Get ARM2 Temp                                //2013.01.11 Q_Q TSMC GPIB COMMAND    //2013.01.24 Q_Q TSMC GPIB COMMAND Part 2. 不需要
// GFOC => Get Force                                    //2013.01.11 Q_Q TSMC GPIB COMMAND

    if(InitialOK==false || bSystemClose==true)
        return;

    int i, j;
    AnsiString Buffer, ARespone, strDecayData, ReceiveData;
    AnsiString Str, Str2, Str3, strESDData;
    AnsiString strbuf="", strfloat="";                                          //ChungHung 20150122 add for TSMC Use Tj
    int iret=0;                                                                 //kevin 20141017
    int Lengh1, Lengh2, Lengh3;
    int iSiteMapping_QualSite[]    ={0,2,1,3,4,6,5,7};
    //AI(W906-GB-P2a) 20260926: golden quirk kept: iMap is not initialised; the three SLOPE/TJ loops below read it when IndexStatus/iGPIBIndexStatus is none of Z1Down_Z2Up/Z1Up_Z2Down/Z1_Z2_Down
    int iMap;
    int iRow=TestSocket.iShtRow;
    int iCol=TestSocket.iShtCol;
    AnsiString asStr2,asStr3;
    int iBufferLen;                                                             //wei 20240530 SW bin 9999
    #ifdef ASE_KaohSiung                                                        //是高雄的話
    int ibuffAutoSite=0;
    int iAutoSiteCount=0;
    #endif
    TStringList *sList;
    PCOPYDATASTRUCT P;
    bool bTTLRS232Err=false;                                                    //Isaac 20210510 : 板子接錯，alarm
    //AI(W906-GB-P2a) 20260926: golden `static iTTLRS232ErrorCount=0;` (BCB6 implicit int) -> `static int`; Handler static, not re-armed (rule 9)
    static int iTTLRS232ErrorCount=0;                                               //Isaac 20210510 : 板子接錯，alarm
    bool bTTLRS232NoReply[2]={false,false};

    //AI(W906-GB-P2a) 20260926: golden :16531-16535 `switch(msg.WParam) { case WM_GPIBInterface: _OnMyCopyMsg_Interface(msg); break;` -- not this function (see banner)
//        case WM_GPIB_Program:                //AI(W906-GB-P2a) 20260926: case label -> this function; its `break;` (:17819) -> end of function
            //Ifor 20170603 (wei) 移至Switch中執行
            //==>
            P=(PCOPYDATASTRUCT) lParam;
            //AI(W906-GB-P2a) 20260926: golden `&HGpib2Handler->iCommand=(unsigned int *)P->lpData;` (rule 4; iCommand is VM's first field)
            HGpib2Handler = reinterpret_cast<VM*>(P->lpData);
            //<==
            //Ifor 20170603 (wei) 移至Switch中執行
            if(TestIF_File.iGpibMode==InterfaceType_SPEA_Type)                  //Steven 20141202 : Fixed 2D Code Function
            {
                _OnMyCopyMsg_Interface(lParam);  //AI(W906-GB-P2a) 20260926: Interface/InterfaceSYS.h:352 takes the LPARAM (rule 4)
            }
            else
            {
                if(ProcessARTMessage())
                {
                }
                else if(HGpib2Handler->iCommand==MSG_CMD_SwitchArm)
                {
                    bSwitchArm2=true;
                }

                if(HGpib2Handler->iCommand==MSG_CMD_TesterMode)                 //Steven 20161122 (Jou) : Add 16 bin GS and 32 bin GS
                {
                    SendMSG_TestMode();
                }
                else if(HGpib2Handler->iCommand==MSG_CMD_Version)               //wei 20150617 Add version control
                {
                    Str=HGpib2Handler->cReturn;
                    bTTLRS232Err=HGpib2Handler->bError;                         //Isaac 20210510 : 板子接錯，alarm
                    bTTLRS232NoReply[0]=HGpib2Handler->bEchoStop;
                    bTTLRS232NoReply[1]=(HGpib2Handler->GPIBBin==1)?true:false;

                    if(bTTLRS232NoReply[0]==true)
                    {
                        ShowMyMessage("TTL RS232 Board1 Not Reply!!", Str);
                        return;
                    }

                    if(bTTLRS232NoReply[1]==true)
                    {
                        ShowMyMessage("TTL RS232 Board2 Not Reply!!", Str);
                        return;
                    }

                    if(bTTLRS232Err==true)                                      //Isaac 20210510 : 板子接錯，alarm
                    {
                        iTTLRS232ErrorCount++;
                        if(iTTLRS232ErrorCount>3 && W906_FormShowing("MyMessageBox", MyMessageBox->fShow)==false)
                        {
                            iTTLRS232ErrorCount=0;
                            ShowMyMessage("TTL RS232 Board Error!!"+Str);
                            bGpibRS232Error=true;
                        }
                    }
                    else
                    {
                        if(HGpib2Handler->bOneCycle)                            //Isaac 20210511 : TTLRS232板子版本檢查
                        {
                            if(atof(Str.c_str())<TTLRS232VerCheck)
                            {
                                ShowMyMessage("TTL RS232 Version Error! Board reply:"+Str,"Please check TTL Board Version !! Must be "+AnsiString(TTLRS232VerCheck));
                                bGpibRS232Error=true;
                            }
                            else
                            {
                                bGpibRS232Error=false;
                            }

                            asTTLRS232Version=Str;   W906_asObsTTLRS232Version=Str;   //AI(W906-GB-P2a) 20260926: T3 -- the value golden shows in the panel below, kept for the Observer page   //AI(W906-H022-T3) 20260930: the web mirror of golden :15987 fObserver->pnlTTLRS232Version->Caption=Str (MessageDef.cpp)
#if 0 // TODO(W906-GB-P2a): G17 TfObserver has no pnlTTLRS232Version (cObserver.cpp:7455) -- golden main.cpp:16604
                            fObserver->pnlTTLRS232Version->Caption=Str;
#endif
                        }
                        else                                                    //本來的判斷式
                        {
                            if(Str.AnsiPos("V")==1)
                                Str=Str.SubString(2, 5);                        //Steven 20250616 : 多了個V
                            else
                                Str=Str.SubString(1, 5);

                            if(atof(Str.c_str())!=GPIBVersionCheck)             //Steven 20191007 : 版本檢查變成兩碼
                            {
                                if(TestIF_File.iTestType==RS232_MODE)
                                    ShowMyMessage("RS232 Version Error!! Please check RS232 Version !! Must be V"+AnsiString(GPIBVersionCheck)+".00");
                                else
                                    ShowMyMessage("GPIB Version Error!! Please check GPIB Version !! Must be V"+AnsiString(GPIBVersionCheck)+".00");
                                bGpibRS232Error=true;
                            }
                            else
                            {
                                bGpibRS232Error=false;
                            }
                            RunInfo.GPIBSoftwareVersion=HGpib2Handler->cReturn;   W906_asObsGPIBVersion=RunInfo.GPIBSoftwareVersion;   //Ifor 20160321 (wei) add GPIB Ver   //AI(W906-H022-T3) 20260930: the web mirror of golden :16009 fObserver->pnlGPIBVersion->Caption=RunInfo.GPIBSoftwareVersion (MessageDef.cpp)
#if 0 // TODO(W906-GB-P2a): G18 TfObserver has no pnlGPIBVersion (cObserver.cpp:7454) -- golden main.cpp:16626
                            fObserver->pnlGPIBVersion->Caption=RunInfo.GPIBSoftwareVersion;                             //Ifor 20170320 (wei) add GPIB 版本顯示於Observer Count 位置
#endif
                        }
                    }
                }
                else if(HGpib2Handler->iCommand==MSG_CMD_AskArmTestMode)
                {
                    if(TestIF_File.bForEgisTecTest==true)                       //Steven 20140922 : Arm2當作指紋測試
                    {
                        SendMSG_CMD(MSG_CMD_2ArmTestMode);
                    }
                    else
                    {
                        SendMSG_CMD(MSG_CMD_1ArmTestMode);
                    }
                }
                else if(HGpib2Handler->iCommand==MSG_CMD_BarCodeFlowErr)        //Steven 20150713 : for 2D Code  //wei 20150924
                {
//                    if(TestIF_File.bOcrFunction)                              //jou 20211029 : 開2D 關OCR 時，不會alarm
                    if(IniConfig.bI42_bEnableBarcodeFlowErr==true)              //Ifor 20211116 add: Use Barcode Flow Err Check
                        tESDError->Add("WAR0704");
                }
                else if(HGpib2Handler->iCommand==MSG_CMD_BarcodeOFF)            //wei 20161024 No Open Barcode Function
                {
                    if(IniConfig.bI42_bEnableBarcodeFlowErr==true)              //Ifor 20211116 add: Use Barcode Flow Err Check
                        tESDError->Add("WAR07401");
                }
                else if(HGpib2Handler->iCommand==MSG_CMD_NoFullSiteRespon)
                {
                    bEcho=true;
                    bUnderTest=false;
                    bTimeOutForNoFullSite=true;
                    if(LastSet.iTester==ON_LINE)                                //Steven 20200715 : 重新計算Cycle Time
                    {
                        EndTestTimeStamp();
                    }
                }
                else if(HGpib2Handler->iCommand==MSG_CMD_QRA)                   //Steven 20241004 : Qorvo check ART enable
                {
                    if(TestIF_File.bSCKART_EnableART)
                    {
                        HHandler2Gpib.iStatus[0]=1;
                    }
                    else if(TestIF_File.i2DIDFormat==eIntel)
                    {
                        HHandler2Gpib.iStatus[0]=0;
                    }
                    //AI(W906-GB-P2a) 20260926: golden `fMain->SendMSG_CMD(...)`: SendMSG_CMD is a THandlerTesterSide member -> this object (rule 2)
                    SendMSG_CMD(MSG_CMD_QRA);
                }
                else if(HGpib2Handler->iCommand==MSG_CMD_OverDrive)             //Steven 20151207 : OverDrive for TSMC
                {
                    iOverDriveDistance      =HGpib2Handler->GPIBBin;
                    bDoOverDrive            =true;
                }
                else if(HGpib2Handler->iCommand==MSG_CMD_ReContact)             //Steven 20151207 : Recontact for TSMC
                {
                    iReContactCount         =HGpib2Handler->GPIBBin;
                    iCurrentReContactCount  =0;
                    bDoReContact            =true;
                }
                else if(HGpib2Handler->iCommand==MSG_CMD_PickLoad)              //Steven 20191009 : Device Map for Qualcomm
                {
                    Str.sprintf("%s%s%s", HGpib2Handler->cReturn, HGpib2Handler->GpibStatus, HGpib2Handler->GpibData);

#if 0 // TODO(W906-GB-P2a): G19 fContact is TfContactShim (atester_shims.h:251): no iDMCArm/iDMCChannel/iDMCTrayX/iDMCTrayY/bWaitLoadXY (the real TfContact, forms/fContact.h:1535-1538, has no global) -- golden main.cpp:16689
                    fContact->iDMCArm=atoi(Str.SubString(9, 1).c_str());
#endif
                    Str3=Str.SubString(Str.AnsiPos(";")+1, Str.Length());
                    Str3.Trim();
                    sList=new TStringList();
                    iret=0;
                    do
                    {
                        int iPos=Str3.AnsiPos(";");                             //JerryYang 20250521 : fix結尾不是分號會無限迴圈
                        if(iPos==0)
                        {
                            break;
                        }
                        sList->Clear();
                        //AI(W906-GB-P2a) 20260926: golden quirk kept: SubString(0, ...) (BCB6 clamps start 0 to 1 -- vclcompat does the same)
                        sList->CommaText=Str3.SubString(0, Str3.AnsiPos(";")-1);
                        if(sList->Count==3)
                        {
                            Lengh1=atoi(sList->Strings[0].c_str());
                            if(Lengh1>0 && Lengh1<32)
                            {
#if 0 // TODO(W906-GB-P2a): G19 fContact is TfContactShim (atester_shims.h:251): no iDMCArm/iDMCChannel/iDMCTrayX/iDMCTrayY/bWaitLoadXY (the real TfContact, forms/fContact.h:1535-1538, has no global) -- golden main.cpp:16708
                                fContact->iDMCChannel[iret]=Lengh1;
#endif
                                Lengh2=atoi(sList->Strings[1].c_str());
                                Lengh3=atoi(sList->Strings[2].c_str());
#if 0 // TODO(W906-GB-P2a): G19 fContact is TfContactShim (atester_shims.h:251): no iDMCArm/iDMCChannel/iDMCTrayX/iDMCTrayY/bWaitLoadXY (the real TfContact, forms/fContact.h:1535-1538, has no global) -- golden main.cpp:16711-16712
                                fContact->iDMCTrayX[iret]=Lengh2;
                                fContact->iDMCTrayY[iret]=Lengh3;
#endif
                                iret++;
                            }
                        }
                        Str3=Str3.SubString(Str3.AnsiPos(";")+1, Str3.Length());
                    }
                    while(Str3.Length()!=0);
                    delete sList;
#if 0 // TODO(W906-GB-P2a): G19 fContact is TfContactShim (atester_shims.h:251): no iDMCArm/iDMCChannel/iDMCTrayX/iDMCTrayY/bWaitLoadXY (the real TfContact, forms/fContact.h:1535-1538, has no global) -- golden main.cpp:16720
                    fContact->bWaitLoadXY=true;
#endif
                }
                else if(HGpib2Handler->iCommand==MSG_CMD_PlaceLoad)             //Steven 20191009 : Device Map for Qualcomm
                {
#if 0 // TODO(W906-GB-P2a): G20 TfContactShim has no bPlaceLoad (see G19) -- golden main.cpp:16724
                    fContact->bPlaceLoad=true;
#endif
                }
                else if(HGpib2Handler->iCommand==MSG_CMD_TrayFeed)              //Steven 20191009 : Device Map for Qualcomm
                {
#if 0 // TODO(W906-GB-P2a): G21 TfContactShim has no bTrayFeed (see G19) -- golden main.cpp:16728
                    fContact->bTrayFeed=true;
#endif
                }
                else if(HGpib2Handler->iCommand==MSG_CMD_MachineState)          //JerryYang 20151109
                {
                    fMain->MachineStatus();
                }
                else if(HGpib2Handler->iCommand==MSG_CMD_TesterBin)             //JerryYang 20160308 add for Maxim_Philippine 各Bin數量
                {
                    fMain->GetCZtesterBin();
                }
                else if(HGpib2Handler->iCommand==MSG_CMD_SoakTime)              //JerryYang 20160308 add for Maxim_Philippine Soak Time
                {
                    fMain->GetCZSoakTime();
                }
                else if(HGpib2Handler->iCommand==MSG_CMD_JamCode)               //JerryYang 20160308 Jam code, add for Maxim_Philippine
                {
                    fMain->GetCZJamCode();
                }
                else if(HGpib2Handler->iCommand==MSG_CMD_SiteMap)               //JerryYang 20160308 site map, add for Maxim_Philippine
                {
                    fMain->GetCZSiteMap();
                }
                else if(HGpib2Handler->iCommand==MSG_CMD_AllMassTemp)           //JerryYang 20160308 all mess temp, add for Maxim_Philippine
                {
                    fMain->GetCZAllMassTemp();
                }
                else if(HGpib2Handler->iCommand==MSG_CMD_DoubleContactCount)    //Isaac 20210706 : add MSG_CMD_DoubleContactCount指令，詢問handler doublecontact次數
                {
                    fMain->GetCZDoubleContactCount();
                }
                //Steven 20190326 : Add for GPIB V9.0
                //==>
                else if(HGpib2Handler->iCommand==MSG_CMD_TempArm)
                {
                    fMain->WritePERSITETemperature();
                }
                else if(HGpib2Handler->iCommand==MSG_CMD_TestArm)
                {
                    fMain->WriteArmStatus();
                }
                else if(HGpib2Handler->iCommand==MSG_CMD_ContactForce)
                {
                    fMain->WriteForce_NS();
                }
                else if(HGpib2Handler->iCommand==MSG_CMD_ActualTemp)
                {
                    fMain->WriteTemp_NS();
                }
                else if(HGpib2Handler->iCommand==MSG_CMD_Assign)
                {
                    fMain->WriteAssign_NS();
                }
                else if(HGpib2Handler->iCommand==MSG_CMD_StartMode)
                {
                    fMain->WriteStartMode_NS();
                }
                else if(HGpib2Handler->iCommand==MSG_CMD_HandlerID)
                {
                    fMain->WriteHandlerID();
                }
                else if(HGpib2Handler->iCommand==MSG_CMD_HandlerSiteMap)
                {
                    fMain->WriteSiteMapData();
                }
                else if(HGpib2Handler->iCommand==MSG_CMD_HandlerSoakTime)
                {
                    fMain->WriteSoakTimeData();
                }
                else if(HGpib2Handler->iCommand==MSG_CMD_HandlerTemperature)
                {
                    fMain->WriteTempData();
                }
                else if(HGpib2Handler->iCommand==MSG_CMD_Force)
                {
                    fMain->WriteArmForce();
                }
                else if(HGpib2Handler->iCommand==MSG_CMD_BinMap)
                {
                    fMain->WriteBinMap();
                }
                else if(HGpib2Handler->iCommand==MSG_CMD_SetBinMap)             //Steven 20230210 : Set Bin Map.
                {
                    Str=HGpib2Handler->cReturn;
                    fMain->WriteSetBinMap(Str);
                }
                else if(HGpib2Handler->iCommand==MSG_CMD_TestMode)
                {
                    fMain->WriteTestMode();
                }
                else if(HGpib2Handler->iCommand==MSG_CMD_GetNowAllTemp)
                {
                    fMain->WriteNowAllTempData();
                }
                else if(HGpib2Handler->iCommand==MSG_CMD_ChkSetup)
                {
                    fMain->WriteChkSetup();
                }
                else if(HGpib2Handler->iCommand==MSG_CMD_GetTestArmPos)
                {
                    fMain->WriteHandlerTestArmEncoder();
                }
                else if(HGpib2Handler->iCommand==MSG_CMD_GetTestArmEP)
                {
                    fMain->WriteHandlerTestArmEP();
                }
                else if(HGpib2Handler->iCommand==MSG_CMD_SetTemp)
                {
                    if(IniConfig.bSIGURDFunction==true)                         //KaiChen 20181129 ：Add GPIB SETTEMP_
                        fMain->WriteSetTempStatus_SIGURD();
                    else
                        fMain->WriteSetTempStatus();
                }
                else if(HGpib2Handler->iCommand==MSG_CMD_SetSoakTime)
                {
                    if(IniConfig.bSIGURDFunction==true)                         //KaiChen 20181129 ：Add GPIB SETSOAK_
                        fMain->WriteSetSoakTimeStatus_SIGURD();
                    else
                        fMain->WriteSetSoakTimeStatus();
                }
                else if(HGpib2Handler->iCommand==MSG_CMD_SetTJ)
                {
                    strbuf=AnsiString(HGpib2Handler->cReturn);
                    if(Temperature.bATCActiveCooling==true)
                    {
                        if(ATC_SYSTEM==eATC60 || ATC_SYSTEM==eATC30)
                        {
                            ATCInterfaceForm->ATC_60_SYS.SetTjOffset(strbuf);
                        }
                        else
                        {
                            //AI(W906-GB-P2a) 20260926: fTemp_Set is NULL in ctests (wb_serve allocates it, tools/wb_serve.cpp:3111); NULL-guard = the tree idiom (MainTempMode.cpp:282-291, Command.cpp WriteSetTempStatus_SIGURD)
                            if(fTemp_Set)
                                fTemp_Set->SaveRemoteTempOffsetFromGPIB(strbuf);    //Steven 20241113 : for MSG_CMD_SetTJ / DEVICETEMP / SETTESTOFFSET_
                        }
                    }
                    else
                    {
                        //AI(W906-GB-P2a) 20260926: fTemp_Set is NULL in ctests (wb_serve allocates it, tools/wb_serve.cpp:3111); NULL-guard = the tree idiom (MainTempMode.cpp:282-291, Command.cpp WriteSetTempStatus_SIGURD)
                        if(fTemp_Set)
                            fTemp_Set->SaveRemoteTempOffsetFromGPIB(strbuf);        //Steven 20241113 : for MSG_CMD_SetTJ / DEVICETEMP / SETTESTOFFSET_
                    }
                }
                else if(HGpib2Handler->iCommand==MSG_CMD_SetSiteMapData)
                {
                    if(IniConfig.bSIGURDFunction==true)                         //KaiChen 20181129 ：Add GPIB SETSITEMAP_
                        fMain->SetSiteMapData_SIGURD();
                    else
                        fMain->SetSiteMapData();
                }
                else if(HGpib2Handler->iCommand==MSG_CMD_SetAlarmSetup)
                {
                    fMain->SetAlarmSetup();
                }
                //<==
                //Steven 20190326 : Add for GPIB V9.0
                else if(HGpib2Handler->iCommand==MSG_CMD_AMDNextStep1)
                {
                    bTJControlMode=false;                                       //Ifor 20190328 : add TJ Temp Over Range
                }
                else if(HGpib2Handler->iCommand==MSG_CMD_AMDNextStep2)
                {
                    if(CosFunction.bUseATCTJControlMode==true)
                        bTJControlMode=true;                                    //Ifor 20190328 : add TJ Temp Over Range
                }
                else if(HGpib2Handler->iCommand==MSG_CMD_HanderIDRS232)         //JerryYang 20190627 回傳Handler ID
                {
                    fMain->GetCDHandlerID();
                }
                else if(HGpib2Handler->iCommand==MSG_CMD_GetSiteOnOff)          //JerryYang 20190627 回傳開關site狀態
                {
                    fMain->WriteSiteOnOff();
                }
                else if(HGpib2Handler->iCommand==MSG_CMD_SetSiteOnOff)          //JimmyChiu 20250715 : Auto site on/off by GPIB
                {
                    strbuf=AnsiString(HGpib2Handler->cReturn);
                    fMain->AutoSiteOnOff(strbuf);
                }
                else if(HGpib2Handler->iCommand==MSG_CMD_GetNumOfSites)         //JerryYang 20190627 回傳site count
                {
                    fMain->WriteNumOfSites();
                }
                else if(HGpib2Handler->iCommand==MSG_CMD_SetTestTemp)
                {
                    fMain->WriteSetTestTempStatus();
                }
                else if(HGpib2Handler->iCommand==MSG_CMD_SamSung_Tmp)           //Steven 20191112 : 三星格式
                {
                    fMain->GetSamSungTmp(true);
                }
                else if(HGpib2Handler->iCommand==MSG_CMD_SamSung_Map)
                {
                    fMain->GetSamSungMap(true);
                }
                else if(HGpib2Handler->iCommand==MSG_CMD_SamSung_Soak)
                {
                    fMain->GetSamSungSoakTime(true);
                }
                else if(HGpib2Handler->iCommand==MSG_CMD_Pause01)               //Steven 20220517 : Add for GIGA
                {
                    bTesterSendPause=true;
                    bTesterPauseMusic=true;                                     //Steven 20220616 : Can select "Alarm Reset" when show "Tester Pause" for QORVO.
                    RecordProcess("GPIB Command PAUSE 01");
                    if(CUSTOMER_CODE==CC_ASE_SG)
                        tESDError->Add("MES0736");
                    else
                        tESDError->Add("MES0733");
                }
                else if(HGpib2Handler->iCommand==MSG_CMD_Stop_01)               //Steven 20220517 : Add for GIGA
                {
                    RecordProcess("GPIB Command STOP 01");
                    tESDError->Add("MES0731");
                }
                else if(HGpib2Handler->iCommand==MSG_CMD_ASKPPSELECT)           //Richard 20220929 :Add for UTAC 讀檔 詢問Handler當前檔名
                {
                    bReceivePPSELECT=true;
                    fMain->PPSELECTAskFile();
                    RecordProcess("GPIB Command PPSELECT?");
                }
                else if(HGpib2Handler->iCommand==MSG_CMD_PPSELECT)
                {
                    fMain->PPSELECTLoadFile();
                    RecordProcess("GPIB Command PPSELECT FileName");
                }
                else if(HGpib2Handler->iCommand==MSG_CMD_CloseSiteHaveBin)      //Steven 20231017 : GPIB flow error need alarm
                {
                    NewRecordProcess("WAR07326", "Test result shows closed site has bin error!!");
                }
                else if(HGpib2Handler->iCommand==MSG_CMD_BinonWithout0x41)      //Steven 20231017 : GPIB flow error need alarm
                {
                    if(IniConfig.iI46_ActionWhenGpibFlowErr==0 ||
                       (HasICUnderMachine()==false && HasAnyICInMachine()==false) ||
                       (W906_FormShowing("fContact", fContact->fShow) && iContactMode!=CONTACT_NORMAL))
                    {
                        NewRecordProcess("WAR07327", "Test flow of GPIB error - binon without 0x41!");
                    }
                    else
                    {
                        tESDError->Add("WAR07327");
                    }
                }
                else if(HGpib2Handler->iCommand==MSG_CMD_ECHONG)                //Steven 20250915 : 資料錯誤
                {
                    strbuf=AnsiString(HGpib2Handler->cReturn);
                    if(strbuf.AnsiPos("TEMPNG")!=0)
                    {
                        tESDError->Add("WAR15404");
                    }
                    else if(strbuf.AnsiPos("SOAKNG")!=0)
                    {
                        tESDError->Add("WAR15403");
                    }
                    else if(strbuf.AnsiPos("DUTCHKNG")!=0)
                    {
                        tESDError->Add("WAR0711");
                    }
                }
                else if(HGpib2Handler->iCommand==MSG_CMD_BinonWithoutFullsite)  //Steven 20231017 : GPIB flow error need alarm
                {
                    if(IniConfig.iI46_ActionWhenGpibFlowErr==0 ||
                       (HasICUnderMachine()==false && HasAnyICInMachine()==false) ||
                       (W906_FormShowing("fContact", fContact->fShow) && iContactMode!=CONTACT_NORMAL))
                    {
                        if(TestIF_File.iTestType==RS232_MODE)                   //Steven 20231205 : 判斷RS232流程是否異常
                            NewRecordProcess("WAR07317", "Test flow of RS232 error - BA without CE command!!");
                        else
                            NewRecordProcess("WAR07328", "Test flow of GPIB error - binon without fullsite command!!");
                    }
                    else
                    {
                        if(TestIF_File.iTestType==RS232_MODE)                   //Steven 20231205 : 判斷RS232流程是否異常
                            tESDError->Add("WAR07317");
                        else
                            tESDError->Add("WAR07328");
                    }
                }
                else if(HGpib2Handler->iCommand==MSG_CMD_SBIN)                  //Steven 20220120 : Amlogic需要收SBIN
                {
                    fMain->SetSBinData();
                }
                else if(HGpib2Handler->iCommand==MSG_CMD_DUTCHK)                //Steven 20250701 : for DOOSAN TESNA
                {
                    fMain->GetDUTCHK();
                }
                else if(HGpib2Handler->iCommand==MSG_CMD_GetFFC)                //Steven 20250701 : for Ampere
                {
                    fMain->GetFFC();
                }
                else if(HGpib2Handler->iCommand==MSG_CMD_GetTJFunction)
                {
                    fMain->GetTJFunction();
                }
                else if(HGpib2Handler->iCommand==MSG_CMD_GetPowerFollowing)
                {
                    fMain->GetPowerFollowing();
                }
                else if(HGpib2Handler->iCommand==MSG_CMD_RESUME)                //Steven 20201022 : For RFMD
                {
                    bTesterSendPause=false;
                    bTesterPauseMusic=false;                                    //Steven 20220616 : Can select "Alarm Reset" when show "Tester Pause" for QORVO.
//AI(W906-ST02-W71) 20261003 (St02-E): V912 main.cpp:17023 kept with the G15 delay (RULINGS_20261003 #1); golden 0618 main.cpp:16402-16406 does not clear it
                    bPauseAlarmDelayActive=false;                               //RogerYang 20260626 : Resume 取消逾時計時(A 全靜音 / B 自動停聲)
                    RecordProcess("GPIB Command RESUME");
                }
                else if(HGpib2Handler->iCommand==MSG_CMD_TestAlarm)
                {
                }
                else if(HGpib2Handler->iCommand==MSG_CMD_ESC)                   //Steven 20201022 : For RFMD Empty Socket Check Funstion.
                {
                    if(HGpib2Handler->GPIBBin==2)                               //GPIBBin  0:ECHONG, 1:CHECKEMPTY, 2:ECHOOK
                    {
                        iGetESCResult=1;
                        RecordProcess("Empty Socket Check -- ECHOOK");
                    }
                    else if(HGpib2Handler->GPIBBin==0)
                    {
                        iGetESCResult=2;
                        RecordProcess("Empty Socket Check -- ECHONG");
                    }
                    else
                    {
                        if(bDoEmptySocketCheck)
                        {
                            fAllMotorHome=false;
                        }

//                        if(TestIF_File.iGpibMode==InterfaceType_15BinQorvo)     //Steven 20250605 : Qorvo 要等到執行ESC後才能
//                        {                                                     //Steven 20250821 : Qorvo 又要改回來
//                            if(LastSet.iRunStartMode==rsmInitial_ART ||
//                               LastSet.iRunStartMode==rsmContinuStart_ART ||
//                               LastSet.iRunStartMode==rsmContinuRetest_ART ||
//                               iAutoRetestTask==300)
//                            {
//                                LastSet.bWaitStartLotAutoRetestGPIB=true;       //JSCK ART Lot Start from Tester for Qorvo
//                            }
//                        }

                        //AI(W906-GB-P2a) 20260926: golden `fMain->ResetForESC(...)`: THandlerTesterSide member -> this object (rule 2)
                        ResetForESC("Start Empty Socket OneCycle for Tester Send Check Empty");
                    }
                }
                else if(HGpib2Handler->iCommand==MSG_CMD_SIGURD_CHKSTATUS)      //KaiChen 20180910 ：Add GPIB CHKSTATUS?
                {
                    fMain->ChkStatus();
                }
                else if(HGpib2Handler->iCommand==MSG_CMD_GETBINCATEGORY)        //KaiChen 20180913 ：Add GPIB GETBINCATEGORY?
                {
                    fMain->GetBinCategory();
                }
                else if(HGpib2Handler->iCommand==MSG_CMD_SETUPFILENAME)         //KaiChen 20181022 ：Add GPIB GETSETUPFILENAME?
                {
                    fMain->GetSetUpFileName();
                }
                else if(HGpib2Handler->iCommand==MSG_CMD_SIGURD_HANDLERID)      //KaiChen 20200507 ：Add GPIB HANDLERID?
                {
                    fMain->GetHandlerID_Sigurd();
                }
                else if(HGpib2Handler->iCommand==MSG_CMD_SGSETUP)               //KaiChen 20190613 ：Add GPIB SGSETUP_
                {
                    fMain->SetSetupFileName();
                }
                else if(HGpib2Handler->iCommand==MSG_CMD_SETSTARTMODE)          //KaiChen 20180910 ：Add GPIB SetStartMode_
                {
                    fMain->SetStartMode();
                }
                else if(HGpib2Handler->iCommand==MSG_CMD_CHECKLIST)             //KaiChen 20190613 ：Add GPIB CHECKLIST?
                {
                    fMain->CheckList();
                }
                else if(HGpib2Handler->iCommand==MSG_CMD_BINPOS)                //KaiChen 20190706 ：Add GPIB BINPOS_
                {
                    fMain->SetBinPosChange();
                }
                else if(HGpib2Handler->iCommand==MSG_CMD_GetSGFTP_STATUS)       //Sam 20210329 : Add GPIB SGFTP_STATUS
                {
                    fMain->GetSGFTPSTATUS();
                }
                else if(HGpib2Handler->iCommand==MSG_CMD_SetSGFTP)              //Sam 20210329 : Add GPIB SGFTP_ SGFTP_ON/SGFTP_OFF
                {
                    fMain->SetSGFTP();
                }
                else if(HGpib2Handler->iCommand==MSG_CMD_SetNONDOUBLEBIN)       //Sam 20210329 : Add GPIB NONDOUBLEBIN_
                {
                    fMain->SetNONDOUBLEBIN();
                }
                else if(HGpib2Handler->iCommand==MSG_CMD_SetBINCOUNT)           //Sam 20210329 : Add GPIB BINCOUNT_
                {
                    fMain->SetBINCOUNT();
                }
                else if(HGpib2Handler->iCommand==MSG_CMD_SetSGOSBIN)            //Sam 20210406 : Add GPIB SGOSBIN_
                {
                    fMain->SetSGOSBIN();
                }
                else if(HGpib2Handler->iCommand==MSG_CMD_SetSGCONTFAIL)         //Sam 20210422 : Add GPIB SGCONTFAIL_
                {
                    fMain->SetSGCONTFAIL();
                }
                else if(HGpib2Handler->iCommand==MSG_CMD_SETTESTERID)           //Sam 20210617 : Add GPIB SETTESTERID
                {
                    fMain->SetTesterID();
                }
                else if(HGpib2Handler->iCommand==MSG_CMD_GETTESTERID)           //Sam 20210617 : Add GPIB SETTESTERID
                {
                    fMain->GetTesterID();
                }
                else if(HGpib2Handler->iCommand==MSG_CMD_GetAutClean)           //Sam 20220408 : Novatek 新增 AUTOCLEAN?
                {
                    fMain->GetAutoClean();
                }
                else if(HGpib2Handler->iCommand==MSG_CMD_ForcePerPinN)          //Sam 20220408 : Novatek 新增 DEVICEFORCEPERPIN?
                {
                    fMain->GetForcePerPinN();
                }
                else if(HGpib2Handler->iCommand==MSG_CMD_ContactHeight)         //Sam 20220408 : Novatek 新增 ARMCONTACTHIGHVALUE?
                {
                    fMain->GetContactHeight();
                }
                else if(HGpib2Handler->iCommand==MSG_CMD_YieldContinusFail)     //Sam 20220408 : Novatek 新增 YIELDCONTINUESFAIL?
                {
                    fMain->GetYieldContinusFail();
                }
                else if(HGpib2Handler->iCommand==MSG_CMD_YieldSiteCompare)      //Sam 20220408 : Novatek 新增 YIELDSITEUNBALANCE?
                {
                    fMain->GetYieldSiteCompare();
                }
                else if(HGpib2Handler->iCommand==MSG_CMD_DUTStatus)             //Sam 20220408 : Novatek 新增 DUTSTATUS?
                {
                    fMain->GetDUTStaus();
                }
                else if(HGpib2Handler->iCommand==MSG_CMD_UPH)                   //Sam 20220408 : Novatek 新增 UPH?
                {
                    fMain->GetUPH();
                }
                else if(HGpib2Handler->iCommand==MSG_CMD_IndexCycleTime)        //Sam 20220408 : Novatek 新增 INDEXCYCLETIME?
                {
                    fMain->GetIndexCycleTime();
                }
                else if(HGpib2Handler->iCommand==MSG_CMD_TempOfs)               //Sam 20220408 : Novatek 新增 GETTEMPOFFSET?
                {
                     fMain->GetTempOfs();
                }
                else if(HGpib2Handler->iCommand==MSG_CMD_TempRange)             //Sam 20220408 : Novatek 新增 GETTEMPERATURETOLERANCE?
                {
                    fMain->GetTempRange();
                }
                else if(HGpib2Handler->iCommand==MSG_CMD_VACUUMAIR)             //Sam 20220408 : Novatek 新增 VACUUMAIR?
                {
                    fMain->GetVacuumAir();
                }
                else if(HGpib2Handler->iCommand==MSG_CMD_Get_All)               //Sam 20220408 : Novatek 新增 SET_ALL?
                {
                    fMain->GetAll();
                }
                else if(HGpib2Handler->iCommand==MSG_CMD_HandlerVersion)        //Sam 20220408 : Novatek 新增 HANDLERVERSION?
                {
                    fMain->GetHandlerVersion();
                }
                else if(HGpib2Handler->iCommand==MSG_CMD_GETSHUTTLEMODE)        //Sam 20230130 : Add GPIB GETSHUTTLEMODE?
                {
                    fMain->GetShuttleMode();
                }
                else if(HGpib2Handler->iCommand==MSG_CMD_SETMAXTEST)            //Sam 20230201 : Add GPIB SETMAXTEST_
                {
                    fMain->SetMaxTest();
                }
                else if(HGpib2Handler->iCommand==MSG_CMD_GETMAXTEST)            //Sam 20230201 : Add GPIB GETMAXTEST
                {
                    fMain->GetMaxTest();
                }
                else if(HGpib2Handler->iCommand==MSG_CMD_SETINITIALMAXTEST)     //Sam 20230201 : Add GPIB SETINITIALMAXTEST_
                {
                    fMain->SetMaxInitialTest();
                }
                else if(HGpib2Handler->iCommand==MSG_CMD_GETINITIALMAXTEST)     //Sam 20230201 : Add GPIB GETINITIALMAXTEST
                {
                    fMain->GetMaxInitialTest();
                }
                else if(HGpib2Handler->iCommand==MSG_CMD_READYNEXTSHOT)         //Jimmychiu 20231011 : #[SCK_HT9046LS] Request for GPIB command adding for next 2DID information
                {
                    fMain->WriteREADYNEXTSHOT();
                }
                else if(HGpib2Handler->iCommand==MSG_CMD_NEXT2DID)              //Jimmychiu 20231011 : #[SCK_HT9046LS] Request for GPIB command adding for next 2DID information
                {
                    fMain->WriteNEXT2DID();
                }
                else if(HGpib2Handler->iCommand==MSG_CMD_SETAICCD)              //Sam 20231108 : Add GPIB SETAICCD_
                {
                    fMain->SetAICCD();
                }
                else if(HGpib2Handler->iCommand==MSG_CMD_GETAICCD)              //Sam 20240826 : Add GPIB GETAICCD?
                {
                    fMain->GetAICCD();
                }
                else if(HGpib2Handler->iCommand==MSG_CMD_ASIF_TJ_EFUSED)        //Steven 20240903 : for MTK ASIF data
                {
                    Str =HGpib2Handler->cReturn;
                    Str2=HGpib2Handler->GpibStatus;
                    Str3=HGpib2Handler->GpibData;
#if 0 // TODO(W906-GB-P2a): G23 ATC_InterfaceForm is TATC_InterfaceFormShim (acarry_shims.h:109, iATC_MODE_TYPE only): no Set_ASIF_TJ_EFUSED -- golden main.cpp:17219
                    ATC_InterfaceForm->Set_ASIF_TJ_EFUSED(Str+Str2+Str3);       //字串最長就是256+32+256=544
#endif
                }
                else if(HGpib2Handler->iCommand==MSG_CMD_ASIF_TJ_REQUEST)       //Steven 20240903 : for MTK ASIF data
                {
#if 0 // TODO(W906-GB-P2a): G23 ATC_InterfaceForm is TATC_InterfaceFormShim (acarry_shims.h:109, iATC_MODE_TYPE only): no Get_ASIF_TJ_REQUEST -- golden main.cpp:17223
                    ATC_InterfaceForm->Get_ASIF_TJ_REQUEST();
#endif
                }
                else if(HGpib2Handler->iCommand==MSG_CMD_ASIF_TJ_FB)            //Steven 20240903 : for MTK ASIF data
                {
#if 0 // TODO(W906-GB-P2a): G23 ATC_InterfaceForm is TATC_InterfaceFormShim (acarry_shims.h:109, iATC_MODE_TYPE only): no Get_ASIF_TJ_FB -- golden main.cpp:17227
                    ATC_InterfaceForm->Get_ASIF_TJ_FB();
#endif
                }
                else if(HGpib2Handler->iCommand==MSG_CMD_SETOSBIN)              //Sam 20250115 : Add GPIB SETOSBIN_
                {
                    fMain->SetOSBIN();
                }
                else if(HGpib2Handler->iCommand==MSG_CMD_GETOSBIN)              //Sam 20250115 : Add GPIB GETOSBIN?
                {
                    fMain->GetOSBIN();
                }
                else if(HGpib2Handler->iCommand==MSG_CMD_GET_SLOPE_OFFSET)      //Get the Slope/Offset value            //Eliot 20210412
                {
                    asGETSLOPEOFFSET="";
                    for(int k=0; k<iRow*iCol; k++)
                    {
                        for(int i=0; i<iRow; i++)
                        {
                            for(int j=0; j<iCol; j++)
                            {
                                if(TestIF.iSiteMap[i][j]==k+1)
                                {
                                    if(IndexStatus==Z1Down_Z2Up || iGPIBIndexStatus==Z1Down_Z2Up)
                                    {
                                        if(iRow==2)
                                            iMap=iSiteMapping_QualSite[i*iRow+j];
                                        else
                                            iMap=i+j;
                                    }
                                    else if(IndexStatus==Z1Up_Z2Down || iGPIBIndexStatus==Z1Up_Z2Down)
                                    {
                                        if(iRow==2)
                                            iMap=iSiteMapping_QualSite[i*iRow+j+iATC_Use_Heat_Count/2];
                                        else
                                            iMap=i+j+iATC_Use_Heat_Count/2;
                                    }
                                    else if(IndexStatus==Z1_Z2_Down || iGPIBIndexStatus==Z1_Z2_Down)
                                    {
                                        if(i==0)
                                            iMap=i+j+iATC_Use_Heat_Count/2;
                                        else
                                            iMap=i+j-1;
                                    }

                                    asGETSLOPEOFFSET+=asALLGETSLOPEOFFSET[0][iMap];
                                    asGETSLOPEOFFSET+=",";
                                    asGETSLOPEOFFSET+=asALLGETSLOPEOFFSET[1][iMap];
                                    if(k!=iRow*iCol-1)
                                        asGETSLOPEOFFSET+=",";
                                }
                            }
                        }
                    }
                    //AI(W906-GB-P2a) 20260926: C sprintf with an AnsiString vararg is ill-formed in GCC -> .c_str() (same bytes)
                    sprintf(HHandler2Gpib.Message, "%s;\r\n",asGETSLOPEOFFSET.c_str());
                    SendMSG_CMD(MSG_CMD_GET_SLOPE_OFFSET);
                }
                else if(HGpib2Handler->iCommand==MSG_CMD_SET_SLOPE_OFFSET)      //Set the Slope/Offset value            //Eliot 20210412
                {
                    bSETSLOPEOFFSET=1;
                    strbuf=AnsiString(HGpib2Handler->cReturn);
                    int iLen=0;
                    AnsiString asSlope="",asOffset="";

                    for(int k=0; k<iRow*iCol; k++)
                    {
                        for(int i=0; i<iRow; i++)
                        {
                            for(int j=0; j<iCol; j++)
                            {
                                if(TestIF.iSiteMap[i][j]==k+1)
                                {
                                    iLen=strbuf.Pos(",");
                                    asSlope=strbuf.SubString(1, iLen-1);
                                    strbuf.Delete(1, iLen);

                                    iLen=strbuf.Pos(",");
                                    if(iLen==0)
                                    {
                                        asOffset=strbuf.SubString(1, strbuf.Pos(";")-1);
                                        strbuf.Delete(1, strbuf.Pos(";"));
                                    }
                                    else
                                    {
                                        asOffset=strbuf.SubString(1, iLen-1);
                                        strbuf.Delete(1, iLen);
                                    }

                                    if(IndexStatus==Z1Down_Z2Up || iGPIBIndexStatus==Z1Down_Z2Up)
                                    {
                                        if(iRow==2)
                                            iMap=iSiteMapping_QualSite[i*iRow+j];
                                        else
                                            iMap=i+j;
                                    }
                                    else if(IndexStatus==Z1Up_Z2Down || iGPIBIndexStatus==Z1Up_Z2Down)
                                    {
                                        if(iRow==2)
                                            iMap=iSiteMapping_QualSite[i*iRow+j+iATC_Use_Heat_Count/2];
                                        else
                                            iMap=i+j+iATC_Use_Heat_Count/2;
                                    }
                                    else if(IndexStatus==Z1_Z2_Down || iGPIBIndexStatus==Z1_Z2_Down)
                                    {
                                        if(i==0)
                                            iMap=i+j+iATC_Use_Heat_Count/2;
                                        else
                                            iMap=i+j-1;
                                    }

                                    asALLSETSLOPEOFFSET[0][iMap]=asSlope;
                                    asALLSETSLOPEOFFSET[1][iMap]=asOffset;
                                }
                            }
                        }
                    }
                    strbuf="";
                    for(int i=0; i<iATC_Use_Heat_Count; i++)
                    {
                        strbuf+=asALLSETSLOPEOFFSET[0][i];
                        strbuf+=",";
                        strbuf+=asALLSETSLOPEOFFSET[1][i];
                        if(i!=iATC_Use_Heat_Count-1)
                            strbuf+=",";
                    }

#if 0 // TODO(W906-GB-P2a): G23 ATC_InterfaceForm is TATC_InterfaceFormShim (acarry_shims.h:109, iATC_MODE_TYPE only): no SetSLOPEOFFSET -- golden main.cpp:17351
                    ATC_InterfaceForm->SetSLOPEOFFSET(strbuf);
#endif

#if W906_ATC_PORTED
                    if(bSETSLOPEOFFSET==1)
                        sprintf(HHandler2Gpib.Message, "SETTINGOK\r\n");
                    else
                        sprintf(HHandler2Gpib.Message, "SETTINGNG\r\n");
#else
                    //AI(W906-GB-P2c) 20260926: user ruling item 7 = B -- SetSLOPEOFFSET above is gated (G23), the ATC got
                    //   nothing, so never claim SETTINGOK.  The parse (and bSETSLOPEOFFSET) runs as golden.
                    sprintf(HHandler2Gpib.Message, "SETTINGNG\r\n");
#endif
                    SendMSG_CMD(MSG_CMD_SET_SLOPE_OFFSET);
                }
                else if(HGpib2Handler->iCommand==MSG_CMD_READTJ)                //Set the Slope/Offset value            //Eliot 20210412
                {
                    AnsiString asTJ="";

                    for(int k=0; k<iRow*iCol; k++)
                    {
                        for(int i=0; i<iRow; i++)
                        {
                            for(int j=0; j<iCol; j++)
                            {
                                if(TestIF.iSiteMap[i][j]==k+1)
                                {
                                    if(IndexStatus==Z1Down_Z2Up || iGPIBIndexStatus==Z1Down_Z2Up)
                                    {
                                        if(iRow==2)
                                            iMap=iSiteMapping_QualSite[i*iRow+j];
                                        else
                                            iMap=i+j;
                                    }
                                    else if(IndexStatus==Z1Up_Z2Down || iGPIBIndexStatus==Z1Up_Z2Down)
                                    {
                                        if(iRow==2)
                                            iMap=iSiteMapping_QualSite[i*iRow+j+iATC_Use_Heat_Count/2];
                                        else
                                            iMap=i+j+iATC_Use_Heat_Count/2;
                                    }
                                    else if(IndexStatus==Z1_Z2_Down || iGPIBIndexStatus==Z1_Z2_Down)
                                    {
                                        if(i==0)
                                            iMap=i+j+iATC_Use_Heat_Count/2;
                                        else
                                            iMap=i+j-1;
                                    }

//                                    if(Temperature.bAMDTDCTriggerMode)
//                                    {
//                                        strfloat.sprintf("%3.1f",ATC_InterfaceForm->dTJ[iMap]/10.0);
//                                        asTJ+=strfloat;
//                                    }
//                                    else
                                    {
#if 0 // TODO(W906-GB-P2a): G23 ATC_InterfaceForm is TATC_InterfaceFormShim (acarry_shims.h:109, iATC_MODE_TYPE only): no dTJ -- golden main.cpp:17400
                                        asTJ+=ATC_InterfaceForm->dTJ[iMap]/10.0;
#endif
                                    }

                                    if(k!=iRow*iCol-1)
                                        asTJ+=",";
                                }
                            }
                        }
                    }
//                    if(Temperature.bAMDTDCTriggerMode)
//                        sprintf(HHandler2Gpib.Message, "Meastemptj %s\r\n",asTJ);
//                    else
                        //AI(W906-GB-P2a) 20260926: C sprintf with an AnsiString vararg is ill-formed in GCC -> .c_str() (same bytes)
                        sprintf(HHandler2Gpib.Message, "%s;\r\n",asTJ.c_str());

                    SendMSG_CMD(MSG_CMD_READTJ);
                }
                else if(HGpib2Handler->iCommand==MSG_CMD_SET_ATCCONTROLMODE)    //Power Following Control ENABLED       //Eliot 20190916
                {
                    strbuf=AnsiString(HGpib2Handler->cReturn);
#if 0 // TODO(W906-GB-P2a): G23 ATC_InterfaceForm is TATC_InterfaceFormShim (acarry_shims.h:109, iATC_MODE_TYPE only): no ATCCONTROLMODEMode -- golden main.cpp:17419
                    ATC_InterfaceForm->ATCCONTROLMODEMode(strbuf);
#endif
                }
                else if(HGpib2Handler->iCommand==MSG_CMD_GET_VOLTAGE)           //wei 20211206 Get the Tj Voltage
                {
                    //ATC_InterfaceForm->GetVOLTAGE();
                    AnsiString asChTJVoltage="";

                    for(int i=0; i<iATC_Use_Heat_Count; i++)
                    {
#if 0 // TODO(W906-GB-P2a): G23 ATC_InterfaceForm is TATC_InterfaceFormShim (acarry_shims.h:109, iATC_MODE_TYPE only): no asTJVoltage -- golden main.cpp:17428
                        asChTJVoltage+=ATC_InterfaceForm->asTJVoltage[i];
#endif
                        if(i<iATC_Use_Heat_Count-1)
                            asChTJVoltage+=",";
                    }
                    //AI(W906-GB-P2a) 20260926: C sprintf with an AnsiString vararg is ill-formed in GCC -> .c_str() (same bytes)
                    sprintf(HHandler2Gpib.Message, "%s;\r\n",asChTJVoltage.c_str());
                    SendMSG_CMD(MSG_CMD_GET_VOLTAGE);
                }
                else if(HGpib2Handler->iCommand==MSG_CMD_SET_ATC_TEMP)          //Set ATC Temp
                {
                    asStr2=AnsiString(HGpib2Handler->cReturn);

                    iBufferLen=asStr2.Pos("_");
                    asStr3=asStr2.SubString(iBufferLen+1, 3);
#if 0 // TODO(W906-GB-P2a): G23 ATC_InterfaceForm is TATC_InterfaceFormShim (acarry_shims.h:109, iATC_MODE_TYPE only): no SetAllTemp -- golden main.cpp:17441
                    ATC_InterfaceForm->SetAllTemp(atof(asStr3.c_str()));
#endif
                }
                else if(HGpib2Handler->iCommand==MSG_CMD_RECODETJ)              //Record Tj Temp(Max/Min/Avg)//Eliot 20220411
                {
                    if(ATC_InterfaceForm->iATC_MODE_TYPE!=ATC_TYPE_60 &&
                       ATC_InterfaceForm->iATC_MODE_TYPE!=ATC_TYPE_70)          //Steven 20260415 : ATC 6.0/7.0 unsupported (1087/1088)
                    {
                        asStr2=AnsiString(HGpib2Handler->cReturn);

                        if(asStr2.Pos("1"))
                            bQUERYTJ=true;
                        else
                            bQUERYTJ=false;
#if 0 // TODO(W906-GB-P2a): G23 ATC_InterfaceForm is TATC_InterfaceFormShim (acarry_shims.h:109, iATC_MODE_TYPE only): no RECORDTJTEMP -- golden main.cpp:17454
                        ATC_InterfaceForm->RECORDTJTEMP(atof(asStr2.c_str()));
#endif
                    }
                }
                else if(HGpib2Handler->iCommand==MSG_CMD_QUERYTJ)               //Record Tj Temp(Max/Min/Avg)//Eliot 20220411
                {
                    AnsiString asQUERYTJ="";

                    for(int i=0; i<iATC_Use_Heat_Count; i++)
                    {
                        for(int j=0; j<3; j++)
                        {
#if 0 // TODO(W906-GB-P2a): G23 ATC_InterfaceForm is TATC_InterfaceFormShim (acarry_shims.h:109, iATC_MODE_TYPE only): no asQueryTJTemp -- golden main.cpp:17465
                            asQUERYTJ+=ATC_InterfaceForm->asQueryTJTemp[i][j];
#endif
                            if(i==iATC_Use_Heat_Count-1 && j==2)
                                asQUERYTJ+=";";
                            else
                                asQUERYTJ+=",";
                        }
                    }

                    //AI(W906-GB-P2a) 20260926: C sprintf with an AnsiString vararg is ill-formed in GCC -> .c_str() (same bytes)
                    sprintf(HHandler2Gpib.Message, "%s\r\n", asQUERYTJ.c_str());
                    SendMSG_CMD(MSG_CMD_QUERYTJ);
                }
                else if(HGpib2Handler->iCommand==MSG_CMD_GET_ATCCONTROLMODE)    //Power Following Control ENABLED       //Eliot 20190916
                {
                    AnsiString asControlMode="";

                    for(int i=0; i<iATC_Use_Heat_Count; i++)
                    {
#if 0 // TODO(W906-GB-P2a): G23 ATC_InterfaceForm is TATC_InterfaceFormShim (acarry_shims.h:109, iATC_MODE_TYPE only): no asControlMode -- golden main.cpp:17482
                        asControlMode+=ATC_InterfaceForm->asControlMode[i];
#endif
                        if(i==iATC_Use_Heat_Count-1)
                            asControlMode+=";";
                        else
                            asControlMode+=",";
                    }

                    //AI(W906-GB-P2a) 20260926: C sprintf with an AnsiString vararg is ill-formed in GCC -> .c_str() (same bytes)
                    sprintf(HHandler2Gpib.Message, "%s\r\n", asControlMode.c_str());
                    SendMSG_CMD(MSG_CMD_GET_ATCCONTROLMODE);
                }
                else if(HGpib2Handler->iCommand==MSG_CMD_GET_ATCERROR)          //Power Following Control ENABLED       //Eliot 20190916
                {
                    if(bATCAlarm)
                        sprintf(HHandler2Gpib.Message, "%d\r\n", 1);
//                    else if(bHandlerATCAlarm)
//                        sprintf(HHandler2Gpib.Message, "%d\r\n", 2);
                    else
                        sprintf(HHandler2Gpib.Message, "%d\r\n", 0);

                    SendMSG_CMD(MSG_CMD_GET_ATCERROR);
                }
                else if(HGpib2Handler->iCommand==MSG_CMD_RESET_ATCALARM)        //wei 20221129 MSG_CMD_RESET_ATCALARM
                {
                    if(ATC_SYSTEM==eNewATCSystem &&
                       Temperature.bATCActiveCooling==true)                     //Ifor 20190130 add 測試前送出Start/End Test 訊息給 ATC  //Ifor 20220107 add:取消功能卡控全部送出命令
                    {
#if 0 // TODO(W906-GB-P2a): G23 ATC_InterfaceForm is TATC_InterfaceFormShim (acarry_shims.h:109, iATC_MODE_TYPE only): no ResetHandlerArmCache / TestFinish / HandlerArm -- golden main.cpp:17508-17510
                        ATC_InterfaceForm->ResetHandlerArmCache();              //AI(ht9045-atc) 20260808 (RogerYang) : alarm reset 需重新同步 Arm 狀態
                        ATC_InterfaceForm->TestFinish();
                        ATC_InterfaceForm->HandlerArm(-1);                      //Ifor 20220107 add:通知ATC目前哪隻Arm再Socket
#endif
                    }
#if W906_ATC_PORTED
                    sprintf(HHandler2Gpib.Message, "SETTINGOK\r\n");
#else
                    //AI(W906-GB-P2c) 20260926: user ruling item 7 = B -- SETTINGNG only where golden would have reset the
                    //   ATC (the three calls above are gated, G23).  A machine that never reaches the ATC here answers
                    //   golden's SETTINGOK: that reply was never an ATC claim.
                    if(ATC_SYSTEM==eNewATCSystem &&
                       Temperature.bATCActiveCooling==true)
                        sprintf(HHandler2Gpib.Message, "SETTINGNG\r\n");
                    else
                        sprintf(HHandler2Gpib.Message, "SETTINGOK\r\n");
#endif

                    SendMSG_CMD(MSG_CMD_RESET_ATCALARM);
                }
                else if(HGpib2Handler->iCommand==MSG_CMD_MultiZoneTemp)         //wei 20240617 Multi Zone
                {
                    fMain->WriteMultiZoneTemp();
                }
                else if(HGpib2Handler->iCommand==MSG_CMD_MultiZoneEnable)       //wei 20240617 Multi Zone
                {
                    fMain->WriteMultiZoneEnable();
                }
                else if(HGpib2Handler->iCommand==MSG_CMD_READ_WATER_VALVE)
                {
                    fMain->ReadWaterValve();
                }
                else if(HGpib2Handler->iCommand==MSG_CMD_SET_WATER_VALVE)
                {
                    asStr2=AnsiString(HGpib2Handler->cReturn);
#if 0 // TODO(W906-GB-P2a): G23 ATC_InterfaceForm is TATC_InterfaceFormShim (acarry_shims.h:109, iATC_MODE_TYPE only): no SetTCWaterValve / ReadTCWaterValue -- golden main.cpp:17531-17532
                    ATC_InterfaceForm->SetTCWaterValve(asStr2.c_str(), 0);
                    ATC_InterfaceForm->ReadTCWaterValue();
#endif
                }
                else if(HGpib2Handler->iCommand==MSG_CMD_AUTOZMOVE)
                {
                    asGPIBAutoHeight=AnsiString(HGpib2Handler->cReturn);
                    bGPIBAutoHeightMove=true;
                }
                else if(HGpib2Handler->iCommand==MSG_CMD_AUTOZPASS)
                {
                    bGPIBAutoHeightPass=true;
                }
                else if(HGpib2Handler->iCommand==MSG_CMD_SET_DYNAMIC_PID)
                {
                    if(ATC_InterfaceForm->iATC_MODE_TYPE==ATC_TYPE_35 ||        //Dynamic PID 僅 ATC 3.5 支援
                       ATC_InterfaceForm->iATC_MODE_TYPE==ATC_TYPE_36)
                    {
                        asStr2=AnsiString(HGpib2Handler->cReturn);
#if 0 // TODO(W906-GB-P2a): G23 ATC_InterfaceForm is TATC_InterfaceFormShim (acarry_shims.h:109, iATC_MODE_TYPE only): no SetDynamicPID / ReadDynamicPID -- golden main.cpp:17549-17550
                        ATC_InterfaceForm->SetDynamicPID(asStr2.c_str());
                        ATC_InterfaceForm->ReadDynamicPID();
#endif
                    }
                }
                else if(HGpib2Handler->iCommand==MSG_CMD_READ_DYNAMIC_PID)
                {
                    if(ATC_InterfaceForm->iATC_MODE_TYPE==ATC_TYPE_35 ||        //Dynamic PID 僅 ATC 3.5 支援
                       ATC_InterfaceForm->iATC_MODE_TYPE==ATC_TYPE_36 )
                    {
                        fMain->ReadDynamicPID();
                    }
                }
                else if(HGpib2Handler->iCommand==MSG_CMD_READAUTOZLIMIT)
                {
                    SendMSG_CMD(MSG_CMD_READAUTOZLIMIT, asGPIBAutoHeightLimit);
                }
                else if(HGpib2Handler->iCommand==MSG_CMD_READZPOS)
                {
#if 0 // TODO(W906-GB-P2a): G24 AnsiString*int: vclcompat AnsiString has no operator* and the BCB6 resolution (Variant?) is unverified; asGPIBAutoHeightZPos is never assigned in golden (always "") -- golden main.cpp:17567
                    SendMSG_CMD(MSG_CMD_READZPOS, asGPIBAutoHeightZPos*100);
#endif
                }
                else if(HGpib2Handler->iCommand==MSG_CMD_READZTORQUE)
                {
                    SendMSG_CMD(MSG_CMD_READZTORQUE, asGPIBAutoHeightTorque);
                }
                else if(HGpib2Handler->iCommand==MSG_CMD_NONE)
                {
                    if(HGpib2Handler->Result[0]=='E' &&
                       HGpib2Handler->Result[1]=='C' &&
                       HGpib2Handler->Result[2]=='H' &&
                       HGpib2Handler->Result[3]=='O')
                    {
                        bExist=true;
                    }
                    else
                    {
                        if(TestIF_File.iGpibMode==InterfaceType_15BinQorvo &&   //Steven 20201022 : For RFMD
                           HGpib2Handler->GPIBBin==1)
                        {
                            bNeedReplunge_RFMD=true;
                            return;
                        }

                        if(CosFunction.bOEEFunction)                            //Steven 20180417 (Jou) : OEE功能
                        {                                                       //KaiChen 20171127 (Steven) ：超豐 OEE 新增 Test Time、Index Time
//#if 0 // TODO(W906-GB-P2a): G25 TfObserver has no iTestReceiveTimeCount (same gate as forms/fLotInfo.cpp:5751 W906-LOT-W1-TESTRECVCNT) -- golden main.cpp:17593-17597   //AI(W906-S09-B3) 20260929: gate retired -- TfObserver::iTestReceiveTimeCount is forms/fObserver.h:808 (W906-W2-3) and RecordReceiveTestTime() is forms/fObserver.h:1203 (body cObserver.cpp:3211, ht9045_sm); fObserver reaches this TU through atester_shims.h:350; body = golden 906_0625_Steven main.cpp:16972-16976; its only reset (golden uLotInfo.cpp:1792) is forms/fLotInfo.cpp:5754, still gated
                            if(fObserver->iTestReceiveTimeCount<=20)            //Sam 20200317 : Fix OEE Test Time
                            {
                                fObserver->iTestReceiveTimeCount++;
                                RecordReceiveTestTime();
                            }
//#endif   //AI(W906-S09-B3) 20260929: see :1948
                        }

                        for(i=0; i<32; i++)
                        {
                            fMain->tTestResult->Strings[i]=AnsiString("-1");           //wei 20160302 0 ==> -1  因為不測試為-1
                            TestIF_File.testBin[i]=0;                           //Sam 20190429 : Add CC_PTI_NEWWORK
                        }

                        for(int i=0; i<32; i++)                                 //Sam 20190429 : Add CC_PTI_NEWWORK
                        {
                            TestIF_File.testBin[i]=HGpib2Handler->Result[i];
                        }

                        #ifdef ASE_KaohSiung                                    //是高雄的話
                        iAutoSiteCount=0;                                       //kevin 20150113
                        int A=0, K=0, iBuffer[2]={0, 0};
                        #endif
                        iOpenBin=0;
                        iBinLast=0;
                        iOpenBinCount=0;                                        //kevin 20160513 AutoSite map

                        for(i=0; i<MAX_SOCKET_ROW; i++)                         //kevin 20141015 fix
                        {
                            for(j=0; j<MAX_SOCKET_COL; j++)
                            {
                                if(TestIF_File.iSiteMap[i][j]>0 &&              //kevin 20141013  ChungHung 20140616 沒卡掉會溢位
                                   TestIF_File.iSiteMap[i][j]<=MAX_SOCKET_TOTAL)                                        //jou 2014-12-15 < -> <= 修正32 site讀取不到Bin data error
                                {
                                    if(LastSet.iRunStartMode==rsmQAMode &&
                                       (Prod.iQAModeRunType==0 || Prod.iQAModeRunType==2) && //Steven 20260514 : RunType 0/2 should apply Untest Bin
                                       LastSet.iTester==OFF_LINE &&
                                       bQAModeFinishCleanOut==true)             //Steven 20141023 : QA做完後的Bin
                                    {
                                        if(Prod.iQAModeBin==0)                  //Steven 20150120 : Fixed QA mode use Bin 0
                                            iBin[i][j]=257;
                                        else
                                            iBin[i][j]=Prod.iQAModeBin;
                                    }
                                    else
                                    {
                                        if(bDutflag[TestIF_File.iSiteMap[i][j]-1] &&
                                           HGpib2Handler->Result[TestIF_File.iSiteMap[i][j]-1]==0)
                                        {
                                            iBin[i][j]=257;
                                        }
                                        else
                                        {
                                            iBin[i][j]=HGpib2Handler->Result[TestIF_File.iSiteMap[i][j]-1];
                                            fMain->tTestResult->Strings[TestIF_File.iSiteMap[i][j]-1]=iBin[i][j];              //Steven 20141230 : 修正SECS GEM參數
                                            #ifdef ASE_KaohSiung                //是高雄的話
                                            if(IniConfig.bUseAutoSiteMapping && LastSet.iRunStartMode==rsmAutoSiteMap)
                                            {
                                                if(bGetOpenBin && Prod.iOpenBin==0)                                     //kevin 20160513 取 OPEN BIN 並判斷是否有其他bin產生
                                                {
                                                    iOpenBin=HGpib2Handler->Result[TestIF_File.iSiteMap[i][j]-1];
                                                    if(iOpenBin!=iBinLast)
                                                    {
                                                        iBinLast=iOpenBin;
                                                        iOpenBinCount++;
                                                    }
                                                }
                                                else if(Prod.iOpenBin!=0)       //kevin 20150113 判斷是否有一個以上 不是open/short Bin
                                                {                               //record new sitemap                                                     //卡第一次取的open bin別
                                                    if((unsigned int)Prod.iOpenBin!=HGpib2Handler->Result[TestIF_File.iSiteMap[i][j]-1])
                                                    {
                                                        iAutoSiteMapBin[i][j]=HGpib2Handler->Result[TestIF_File.iSiteMap[i][j]-1];
                                                        //AI(W906-GB-P2a) 20260926: golden quirk kept: under ASE_KaohSiung (not defined, MachineType.h:67) the block writes iAutoSiteMapSocketPass[4][8] of an [4][8] array (out of bounds)
                                                        iAutoSiteMapSocketPass[4][8]=1;                                 //kevin 20160513
                                                        ibuffAutoSite=TestIF_File.iSiteMap[i][j];                       //有ic site
                                                        iBuffer[0]=iBin[i][j];
                                                        iBuffer[1]=iBin[iAutoSiteRecordIC[0]][iAutoSiteRecordIC[1]];
                                                        A=i;
                                                        K=j;
                                                        iAutoSiteCount++;
                                                    }
                                                }
                                            }
                                            #endif
                                        }
                                    }
                                }
                                else
                                {
                                    iBin[i][j]=0;
                                }
                            }
                        }

                        #ifdef ASE_KaohSiung                                    //是高雄的話
                        if(IniConfig.bUseAutoSiteMapping &&
                           LastSet.iRunStartMode==rsmAutoSiteMap)               //kevin 20150114
                        {
                            if(bGetOpenBin && iOpenBinCount!=1)                 //kevin 20160513 open bin 有其他bin 產生     //checkAuto
                            {
                                 bShowAutoSiteMappingError=true;
                                 ShowMyMessage("Site Mapping Check Open/short Bin Fail! Have two devive IC Bin .",
                                               "Site Mapping Check Fail!");
                            }
                            else if(iAutoSiteCount==1)
                            {
                                iBin[A][K]=iBuffer[1];
                                iBin[iAutoSiteRecordIC[0]][iAutoSiteRecordIC[1]]=iBuffer[0];
                                for(i=0; i<MAX_SOCKET_ROW; i++)                 //kevin 20141015 fix
                                {
                                    for(j=0; j<MAX_SOCKET_COL; j++)
                                    {
                                        if(TestIF_File.iSiteMap[i][j]>0 &&
                                           TestIF_File.iSiteMap[i][j]<=MAX_SOCKET_TOTAL)                                //kevin 20141013  ChungHung 20140616 沒卡掉會溢位      //jou 2014-12-15 < -> <= 修正32 site讀取不到Bin data error
                                        {
                                            if(bDutflag[TestIF_File.iSiteMap[i][j]-1]==true)
                                                iAutoSiteMap[i][j]=ibuffAutoSite;
                                        }
                                    }
                                }
                            }
                            else
                            {
                                if(iAutoSiteCount>1)
                                {
                                    bShowAutoSiteMappingError=true;             //20150115
                                    ShowMyMessage("Site Mapping Check Fail! Have Double IC Bin,need Do again!",
                                                  "Site Mapping Check Fail!");
                                }
                            }
                        }
                        #else
                        if(IniConfig.bI18CanReceiveEchoStop)                    //ChungHung 20130326 add for ASE_KR
                            bEchoStop=HGpib2Handler->bEchoStop;
                        else
                            bEchoStop=false;

                        fSCKART->SetLotStatus(fSCKART->iLOTSTATUS_W);           //Steven 20161201 (wei) : For SCK FLEX ART
                        if(CosFunction.bTesterLowYieldOneCycle==true)           //jou 2014-09-23 Tester Low Yield Handler need One Cycle & Alarm
                        {
                            if(bTesterLowYieldOneCycle==false && HGpib2Handler->bOneCycle==true)
                            {
                                bTesterLowYieldOneCycle=HGpib2Handler->bOneCycle;
                                //AI(W906-GB-P2a) 20260926: golden `BtnOneCycleClick(this)`: `this` was the TfMain form -> fMain (Sender unused by the facade)
                                fMain->BtnOneCycleClick(fMain);
                            }
                        }
                        else
                        {
                            bTesterLowYieldOneCycle=false;                      //Steven 20260612 : Fix == to = (was comparison, not assignment)
                        }
                        #endif
                        bTimeOutForNoFullSite=false;
                        bEcho=true;
                        bUnderTest=false;
                        TestIntervalsTime.LatchCycleTime(true);                 //kevin 20160311
                        TestIntervalsBoostTime.LatchCycleTime(true);            //JerryYang 20181122 (Steven) :  (Steven) : 將不同function計時器分開
                        PauseIntervalsTime.LatchCycleTime(true);                //kevin 20181009 取得機台停止時間
                        bEOTToLongStopBlowAir=true;                             //kevin 20181009 add上次測試訊號太久 需停止吹氣
                        asRecordTestResult=AnsiString(HGpib2Handler->cReturn);

                        if(mmo1->Lines->Count>20)
                        {
                            mmo1->Clear();
                        }
                        Str2.sprintf("%02d:%02d:%02d %03d  G->H  %s", SystemHour, SystemMin, SystemSec, SystemMSec, asRecordTestResult);
                        mmo1->Lines->Add(Str2);
                        if(LastSet.iTester==ON_LINE)                            //Steven 20200715 : 重新計算Cycle Time
                        {
                            EndTestTimeStamp();
                        }
                    }
                }
                else if(HGpib2Handler->iCommand==MSG_CMD_AMDRS232Connect)       //wei 20150617 Add version control
                {
                    Str=HGpib2Handler->cReturn;
                    if(Str=="1")
                    {
                        //bAMDRs232ConnectError=true;                           //Ifor 20210128 Mark:程式啟動後僅確認一次即可
                    }
                    else
                    {
                        bAMDRs232ConnectError=false;
                    }
                }
                else if(HGpib2Handler->iCommand==MSG_CMD_State_TTL)             //Isaac 20200903 :TTL RS232通訊
                {
                    fMain->GetTTLState();
                    if(TestIF_File.bTTLUseASEJPMode)
                    {
                        if(IniConfig.bD22SupportMultiDoubleContact)
                            fMain->Send_Command_TTL("@00WDUTS00000000");
                        else
                            fMain->Send_Command_TTL("@00WDUTS00100000");
                    }
                }
                else if(HGpib2Handler->iCommand==MSG_CMD_HANA_ART)              //JimmyChiu 20241023 HANA ART Function
                {
                    Str=HGpib2Handler->cReturn;
#if 0 // TODO(W906-GB-P2a): G26 TfMainHanaART (forms/fMain.h:80) has no DoCmdWhenHDStart -- golden main.cpp:17789
                    hanaART->DoCmdWhenHDStart(Str);
#endif
                }
                else if(HGpib2Handler->iCommand==MSG_CMD_FTPDownLoad)
                {
                    fMain->WriteFTPDownSetupFile();
                }
                else if(HGpib2Handler->iCommand==MSG_CMD_SetupFileChange)
                {
                    fMain->WriteSetSetupFile();
                }
                else if(HGpib2Handler->iCommand==MSG_CMD_GetContactCount)       //Ifor 20240510 add: Get Head Contact Count
                {
                    fMain->WriteHeadContactCount();
                }
                else if(HGpib2Handler->iCommand==MSG_CMD_BarcodePin1ON)         //Ifor 20240528 add:Pin1 Function
                {
                    if(TestIF_File.bEnableBarCode==true)
                        //AI(W906-GB-P2a) 20260926: golden `fMain->bHasPin1Error`: THandlerTesterSide member -> this object (rule 2)
                        bHasPin1Error=true;
                    else
                        bHasPin1Error=false;
                }
                else if(HGpib2Handler->iCommand==MSG_CMD_GetSocketCounter)      //Sam 20210617 : Add GPIB SETTESTERID
                {
                    fMain->GetSocketCounter();
                }
                else if(HGpib2Handler->iCommand==MSG_CMD_GetTIMCounter)         //Sam 20210617 : Add GPIB SETTESTERID
                {
                    fMain->GetTIMCounter();
                }
            }
//            break;                         //AI(W906-GB-P2a) 20260926: end of `case WM_GPIB_Program:` == end of this function
}
//------------------------------------------------------------------------------
