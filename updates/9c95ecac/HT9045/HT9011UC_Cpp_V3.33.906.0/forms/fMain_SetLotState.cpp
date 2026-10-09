//------------------------------------------------------------------------------
// forms/fMain_SetLotState.cpp -- AI(W906-SETLOTSTATE) 20261001: golden TfMain::SetLotState, main.cpp:15160-15407, line for line
//   (RULINGS_20261001 #0 / #5, census 129 (a)(b) #16 "SetLotState is empty -- the tester never hears LOTSTART / LOTEND").
//   forms/fMain.cpp:553 had `void TfMain::SetLotState(int) {}`, and every live caller (csystem.cpp lot start / lot end /
//   ART / UTAC / TSMC GPIB, Command.cpp TCP ART, uRENESAS_Server.cpp, forms/fLotInfo.cpp SECS ART) went through it.
//   What it does, as golden: on a TCP/IP tester it sends LOTNUMBER,<lot>,FT|RT / LOTEND (+ LOTSTART, GETOSSETUP) through
//   the tester TCP socket; on a GPIB tester (bridge found) it sets the SCK ART step / lot-start time / the lot timeouts,
//   asks the OLP engine for PRODUCTION_REQUEST, fills HHandler2Gpib (MSG_CMD_LotStatus + iLotStatus) and sends it to the
//   bridge for HANA ART / UTAC / TCP-SECS ART / SCK ART / [A37] / GPIB lot end, or raises the ART SECS events.
//   The only edits are the port's established mappings (each marked AI(W906-SETLOTSTATE) on its line):
//     fTesterTCP->SendTCPIPCommand -> TesterTCPSocket_SendTCPIPCommand          (no TfTesterTCP form; St02 HandlerBridgeCtl.cpp:910)
//     fMain->bFind                  -> W906_TesterBridgeFound()                 (forms/fMain.h, the tester forward table)
//     SendMessage(HVisionWnd, WM_COPYDATA, ...) -> W906_TesterSendToBridge(pcp) (TesterComm/TesterWndSeat.h, St02 B1)
//     btClearBarcodeList->Click()   -> btClearBarcodeListClick()                (St02 MR !22 precedent)
//   Two arms are gated, each with golden's condition left live and its reason on the gate line: the HANA ART arm
//   (fMain->hanaART is the offline facade, IsHanaArtAvailable() always false) and the Renesas FT-CT "test end received"
//   check (the real TRENESAS_Server is never instantiated). Both are unreachable in this tree today.
//   Not installed (every ctest without the tester hub): W906_TesterBridgeFound() is false and W906_TesterSendToBridge sends
//   nothing -- golden's "bridge not found" path. fAutomation->DoCommandBuffer is still the TfAutomationShim no-op until the
//   OLP engine is wired (census (e) E-FT2-001), so PRODUCTION_REQUEST goes nowhere yet -- the call is golden's.
//   In ht9045_sm (CMakeLists.txt, next to forms/fMain_OperateMode.cpp) for the same reason as that file: its body needs
//   sm / automation / secsgem symbols that ht9045_forms cannot link.
//------------------------------------------------------------------------------
#include "forms/fMain.h"
#include "cprod.h"
#include "cmydef.h"
#include "Config.h"
#include "LastSet.h"
#include "CosFunction.h"
#include "MachineType.h"
#include "MessageDef.h"               // HHandler2Gpib / MSG_CMD_LotStatus
#include "TesterComm/TesterWndSeat.h" // W906_TesterSendToBridge
#include "Interface/TesterTCP_Socket.h"   // TesterTCPSocket_SendTCPIPCommand
#include "SECSGEM/SecsEventReport.h"  // EventReport
#include "SECSGEM/SecsEventType.h"    // SECS_EVENT
#include "forms/fLotInfo.h"           // fLotInfo->edtSysLotID / btClearBarcodeListClick
#include "forms/fSCKART.h"            // fSCKART
#include "forms/fAGV.h"               // fAGV
#include "forms/fSortCT.h"            // fSortCT
#include "atester_shims.h"            // fAutomation (TfAutomationShim)
#include "csystem.h"                  // hLotStartTimeOut / hLotEndTimeOut (csystem.h:299-300)
#include "Automation/AtkAmr_St02.h"   // W906_AtkFinalLotEndCeid288 (AI(W906-W195) 20261009 (St02-E) ATK)
void TfMain::SetLotState(int iState)   //AI(W906-SETLOTSTATE) 20261001: golden `void __fastcall` -- the facade declares it without __fastcall (forms/fMain.h:782)
{
    static int OldState=10;
    AnsiString str="", str2="";
    if(TestIF_File.iTestType==TCP_IP_MODE)                                      //wei 20211027 open short TCP/IP
    {
        if(CUSTOMER_CODE==CC_SJ_Semiconductor_OS ||                             //Steven 20230213 : For SJSemi OS Tester
           CUSTOMER_CODE==CC_XINITECH)
        {
            return;
        }

        if(iState==2)
        {
            str.sprintf("LOTNUMBER,%s,FT,", fLotInfo->edtSysLotID->Text);       //LOTNUMBER,111233334566,FT
        }
        else if(iState==4)
        {
            str.sprintf("LOTNUMBER,%s,RT,", fLotInfo->edtSysLotID->Text);       //LOTNUMBER,111233334566,RT
        }
        else if(iState==8 || iState==10)
        {
            str.sprintf("LOTEND");
        }
        TesterTCPSocket_SendTCPIPCommand(0, "SetLotState", str);   //AI(W906-SETLOTSTATE) 20261001: golden fTesterTCP->SendTCPIPCommand -- no TfTesterTCP form in V906 (St02 precedent HandlerBridgeCtl.cpp:910)

        if(iState==2 || iState==4)
        {
            str.sprintf("LOTSTART");
            TesterTCPSocket_SendTCPIPCommand(0, "LOTSTART", str);   //AI(W906-SETLOTSTATE) 20261001: ditto
            TesterTCPSocket_SendTCPIPCommand(0, "Get OS Setup", "GETOSSETUP");      //Steven 20230505 : 取得OS Tester資訊
        }
        return;
    }

    if(TestIF.iTestType!=GPIB_MODE)
    {
        return;
    }

    if(W906_TesterBridgeFound()==false)   //AI(W906-SETLOTSTATE) 20261001: golden fMain->bFind (forms/fMain.h W906_TesterBridgeFound: the bridge window was found)
        return;

    OldState=iState;
    LastSet.bWaitStartLotAutoRetestGPIB=false;                                  //jou 2015-10-02 Auto Retest GPIB mode
    if(iState==2)                                                               //FT Start
    {
        fAGV->bATK_AMR_DoLotEndSent=false;                                      //AI(general) 20260402 (RogerYang) : 新 Lot 重設旗標
        fSCKART->iCurrent93KARTStep=2;
        if(TestIF_File.bRENESAS_EnableFTCT==false)                              //RogerYang 20250930 : RogerYang 瑞薩FT-CT 指令"20"已更新
            fSCKART->sLotStartTime=Now().FormatString("yyyymmddhhnnss");

        if(IniConfig.bA10_AutoReTest &&
           TestIF_File.bSCKART_EnableART &&
           (LastSet.iRunStartMode==rsmContinuStart_ART || LastSet.iRunStartMode==rsmInitial_ART) &&
           fSCKART->sInfo_Stage.Pos("QC")==1)
        {
            bNeedInputEQCQty=true;
        }
        hLotStartTimeOut.SetSecAndOn(120);                                      //JerryYang 20220923 : add SPIL ART LOT START/LOT END timeout機制
    }
    else if(iState==4)                                                          //RT Start
    {
        fSCKART->iCurrent93KARTStep=8;
        if(TestIF_File.bRENESAS_EnableFTCT==false)                              //RogerYang 20250930 : RogerYang 瑞薩FT-CT 指令"20"已更新
            fSCKART->sLotStartTime=Now().FormatString("yyyymmddhhnnss");
        hLotStartTimeOut.SetSecAndOn(120);                                      //JerryYang 20220923 : add SPIL ART LOT START/LOT END timeout機制
    }
    else if(iState==8)                                                          //Lot End
    {
        if(fSCKART->iFTRTCount>1)
            fSCKART->iCurrent93KARTStep=10;
        else
            fSCKART->iCurrent93KARTStep=4;
        hLotEndTimeOut.SetSecAndOn(120);                                        //JerryYang 20220923 : add SPIL ART LOT START/LOT END timeout機制
    }
    else if(iState==10)                                                         //Final Lot End
    {
        fSCKART->iCurrent93KARTStep=11;

        if(fAGV->IsATK_AMR()==true &&                                           //AI(ht9045-atk-amr-flow) 20260821 (RogerYang) : 客戶變更-提前通知改發CEID288呼叫AMR取貨, CEID8只送最終一筆   (golden 913 main.cpp:15922; AI(W906-W195) 20261009 (St02-E) ATK)
           IniConfig.bEnable_SECS_GEM==true && RunInfo.bLotStart==true)
        {
            W906_AtkFinalLotEndCeid288();                                       // AI(W906-W195) 20261009 (St02-E) ATK: golden 913 main.cpp:15925-15953 (= 912 :15781-15809) -- CEID 288 for Auto1-3, then the restore
            // AI(W906-W195) 20261009 (St02-E) ATK: golden 912/913 send the CEID 8 and set bATK_AMR_DoLotEndSent in csystem.cpp DoTrayFeed / MainProc instead (golden 913 csystem.cpp:8129-8136 / :19315-19317)
        }
        hLotEndTimeOut.SetSecAndOn(120);                                        //JerryYang 20220923 : add SPIL ART LOT START/LOT END timeout機制
    }

    if(CosFunction.bAutoRetestGPIBmode==false)                                  //jou 2015-10-02 Auto Retest GPIB mode
    {
        if(OldState==2)
            fAutomation->DoCommandBuffer("PRODUCTION_REQUEST", "", "0001", 0, "");
        else if(OldState==10)
            fAutomation->DoCommandBuffer("PRODUCTION_REQUEST", "", "0005", 0, "");
    }

    HHandler2Gpib.iSendCommand=MSG_CMD_LotStatus;                               //wei 20150409 Add Close GPIB Command   MSG_CMD_NONE-->MSG_CMD_LotStatus
    HHandler2Gpib.iLotStatus=iState;

    COPYDATASTRUCT *pcp=new COPYDATASTRUCT;
    pcp->dwData=0;

    pcp->cbData=sizeof(HHandler2Gpib);
    pcp->lpData=(unsigned char *)&HHandler2Gpib.iSendCommand;

    if(hanaART->IsHanaArtAvailable())                                           //JimmyChiu 20241023 HANA ART Function
    {
#if 0 // GATE(W906-SETLOTSTATE-HANA) 20261001: golden text below, verbatim. fMain->hanaART is the offline facade TfMainHanaART (forms/fMain.h:80-96): IsHanaArtAvailable() is always false and the HANA ART subsystem is not wired into TfMain, so this arm is unreachable here; its body also needs DoSETUP_INFORM / DoRT_START, LotSummary and TfSCKART::iCurrentStatus, which have no live home on those facades. UN-GATE together with the HANA ART wiring (census 129).
        if(iState==2)                                                           //FT Start
        {
            hanaART->DoSETUP_INFORM();
        }
        else if(iState==4)                                                      //RT Start
        {
            hanaART->DoRT_START();                                              //Steven 20250414 : HANA ART Function
        }
        else if(iState==8)                                                      //Lot End
        {
//            hanaART->EndPrimeTest();                                          //Steven 20250415 : mark
            fMain->Clarn_Data(2, "ART_RMODEOK");

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

            for(int i=0; i<10; i++)
            {
                LastSet.lSCKARTBinCT[i]=0;
            }
            LastSet.iSCKARTInputCT=0;
            LastSet.lShuttleCount=0;
            LotSummary.ClearRTData();

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

            if(fSCKART->iCurrentStatus==fSCKART->iLOTSTATUS_R)
            {
                fSCKART->SetLotStatus(fSCKART->iLOTSTATUS_W);
                fSCKART->iWaitGPIBLotR=3;
            }
            fSCKART->AccessFile(false, -1);
            RecordProcess("HANA ART RMODEOK.");
            fLotInfo->btClearBarcodeListClick();   //AI(W906-SETLOTSTATE) 20261001: golden btClearBarcodeList->Click() fires OnClick = btClearBarcodeListClick (St02 MR !22 precedent)                              //Steven 20190214 : 統一清除2DID方式
        }
        else if(iState==10)                                                     //Final Lot End
        {
//            hanaART->EndReTest();                                             //Steven 20250415 : mark
        }
        hLotStartTimeOut.SetSecAndOn(120);                                      //JerryYang 20220923 : add SPIL ART LOT START/LOT END timeout機制
#endif // GATE(W906-SETLOTSTATE-HANA)
    }
    else if(CUSTOMER_CODE==CC_UTAC)                                             //Richard 20220929 :Add for UTAC Lot Start/End 0xC0
    {
        W906_TesterSendToBridge(pcp);   //AI(W906-SETLOTSTATE) 20261001: golden SendMessage(fMain->HVisionWnd, WM_COPYDATA, (WPARAM) NULL, (LPARAM)pcp) (TesterComm/TesterWndSeat.h)
    }
    else if((CosFunction.iAutoRetestTCPmode!=0 ||                               //RogerYang 20251029 : bool->int  //Sam 20191113 : TCP ART
             CosFunction.bART_SECSGEM_93K) &&                                   //JerryYang 20220923 : SECS GEM版本ART
            (IniConfig.bA10_AutoReTest && TestIF_File.bSCKART_EnableART) ||
            IniConfig.bA37LotStartLotEnd==true)
    {
        if(TestIF_File.bSCKART_RunARTWithoutCmd)
        {
            W906_TesterSendToBridge(pcp);   //AI(W906-SETLOTSTATE) 20261001: golden SendMessage(fMain->HVisionWnd, WM_COPYDATA, (WPARAM) NULL, (LPARAM)pcp) (TesterComm/TesterWndSeat.h)
        }
        else
        {
            //fSCKART->iTCPModeLotState=iState;
            LastSet.iTCPModeLotState=iState;                                    //Sam 20200311 : Fix TCP ART MODE
        }

        if(IniConfig.bEnable_SECS_GEM==true && CosFunction.bART_SECSGEM_93K)    //JerryYang 20220923 : SECS GEM版本ART
        {
            if(iState==2)                                                       //FT Start
            {
                EventReport(SECS_EVENT.ART_SRQKIND2_FTLOTSTART);
            }
            else if(iState==4)                                                  //RT Start
            {
                EventReport(SECS_EVENT.ART_SRQKIND4_RTLOTSTART);
            }
            else if(iState==8)                                                  //Lot End
            {
                EventReport(SECS_EVENT.ART_SRQKIND8_LOTEND);
            }
            else if(iState==10)                                                 //Final Lot End
            {
                EventReport(SECS_EVENT.ART_SRQKIND10_FINALLOTEND);
            }
        }
        else if(CosFunction.iAutoRetestTCPmode==2 &&                            //RogerYang 20250911 : RogerYang 瑞薩FT-CT
                TestIF_File.bRENESAS_EnableFTCT==true)
        {
#if 0 // GATE(W906-SETLOTSTATE-RENESAS) 20261001: golden text below, verbatim. fMain->RENESAS_Server is the offline facade TfMainRENESASServer (forms/fMain.h:136-143); the real TRENESAS_Server (Automation/uRENESAS_Server.cpp, the class that owns iHaveRecvTestEnd) is compiled but never instantiated, so there is no FT-CT "test end received" to read. UN-GATE with the Renesas FT-CT server wiring.
            if(RENESAS_Server->iHaveRecvTestEnd==1)
    //          (iState==8 || iState==10))
            {
                if(iState==8)
                {
                    //LastSet.bWaitStartLotAutoRetestGPIB=true;
                    LastSet.bWaitEndLotAutoRetestGPIB=true;
                }
                else if(iState==10)
                {
                    //LastSet.bWaitStartLotAutoRetestGPIB=true;
                    LastSet.bWaitEndLotAutoRetestGPIB=true;
                }
                //RENESAS_Server->iHaveRecvTestEnd=0;
                //RENESAS_Server->SendTestEnd();
            }
#endif // GATE(W906-SETLOTSTATE-RENESAS)
        }
    }
    else
    {
        if((CosFunction.bUseSCKART &&
            IniConfig.bA10_AutoReTest &&
            TestIF_File.bSCKART_EnableART) ||                                   //Steven 20190719 : 避免ART命令被誤用
            ((CUSTOMER_CODE==CC_SCK || CUSTOMER_CODE==CC_SCS) &&                //Steven 20190719 : 部分命令沒裝ART只開放SCK
              iState==2) ||
              CosFunction.bGPIBLotEnd)                                          //kevin 20191004 add
        {
            W906_TesterSendToBridge(pcp);   //AI(W906-SETLOTSTATE) 20261001: golden SendMessage(fMain->HVisionWnd, WM_COPYDATA, (WPARAM) NULL, (LPARAM)pcp) (TesterComm/TesterWndSeat.h)
        }
    }
    delete pcp;
}
