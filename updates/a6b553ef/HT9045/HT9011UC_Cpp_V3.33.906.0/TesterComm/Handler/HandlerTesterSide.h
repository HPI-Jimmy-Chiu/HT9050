// ===========================================================================
//  TesterComm/Handler/HandlerTesterSide.h -- the Handler (machine) side of tester communication: golden TfMain's
//  bridge-facing code, translated onto TesterCommHub.
//
//  AI(W906-GB-P2a) 20260926: Tester-comm plan P2a.  Golden: D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy
//  main.cpp (Big5), the parts that talk to the tester bridge programs (H9046_32GPIB / RS232Standard):
//      :16481-16530 + :16536-17819  TfMain::OnMyCopyMsg preamble/locals + `case WM_GPIB_Program:`
//      :15974-16480  ProcessARTMessage            :8028-8038   ResetForESC
//      :18315-18493  ProcessHVisionConnect        :18499-18596 WakeupGPIB
//      :18734-18859  SendMSG_CMD x2 / SendMSG_CMD_DeviceMapSRQ
//      :18866-18959  SendMSG_TestMode              :18961-19195 RunTestProgram (+ the file-scope globals above it)
//      :29134-29160  CloseGpibProgram              :33010-33038 InitialBarCodeList
//
//  WHY A SEPARATE CLASS, NOT forms/fMain.h.  The V906 TfMain facade already has 109 of the 134 TfMain members this
//  code uses (Command.cpp's Write*/Get* replies, TempDataStrings, tTestResult, oldGpibAddress ...).  The other ~20
//  (bFind, HVisionWnd, WakeupGPIBdelay, lblGPIBWND, RunTestProgram, CloseGpibProgram, SendMSG_TestMode ...) are
//  declared HERE, because forms/fMain.h is outside this claim (TO_STEVEN.md §1).  Golden bodies therefore:
//    * use a member of THIS class unqualified (as golden did inside TfMain);
//    * reach every other golden TfMain member through `fMain->X` (the V906 facade);
//    * reach globals unqualified, as golden.
//  The later hookup (P2b, needs its own claim): forms/fMain.cpp's SendMSG_CMD stubs and the atester.cpp tester
//  blocks call into fTesterSide; until then the ctest installs a TfMain subclass whose SendMSG_CMD forwards here.
//
//  In-process replacements (same idea as the bridge engines, the other end of the same mailbox):
//    SendMessage(HVisionWnd / fMain->HVisionWnd, WM_COPYDATA, 0, pcp)  -> SendToBridge(pcp)
//    FindWindow("TSerialPoll"/"TfRS232Main", ...) for the bridge       -> FindBridgeWindow()
//    CreateProcess("d:\\gpib9045\\H9046_32GPIB.exe" / RS232Standard)   -> StartBridgeProgram()
//    TerminateProcess(old bridge) on a test-type change                 -> StartBridgeProgram() (hub restarts)
//    this->Handle (the Handler's own window)                             -> HandlerWndToken()
//  Threading: every function here runs on the Handler (tick) thread -- the hub calls Sink() only from
//  PollHandler() or from a SendToEngine() that is pumping on that thread.
// ===========================================================================
#ifndef TESTERCOMM_HANDLER_HANDLERTESTERSIDE_H
#define TESTERCOMM_HANDLER_HANDLERTESTERSIDE_H

#include "forms/fMain.h"   // TfMain facade + fMain; vclcompat widgets
#include "myTimer.h"       // TQPF_Timer
#include "MessageDef.h"    // VM / MV, HGpib2Handler / HHandler2Gpib, MSG_CMD_*

#include <string>

namespace testercomm { class TesterCommHub; }

class THandlerTesterSide
{
public:
    THandlerTesterSide();
    ~THandlerTesterSide();

    // ---- golden TfMain data members missing from the V906 facade (golden main.h) ----
    bool        bFind;                    // main.h: Find GPIB flag (bridge window found)
    HWND        HVisionWnd;               // main.h: the bridge window
    TQPF_Timer  WakeupGPIBdelay;          // main.h
    TLabel     *lblGPIBWND;               // main.h (owned; headless)
    TMemo      *mmo1;                     // main.h (owned; headless)
    TStringList *tESDError;               // main.h (owned)
    bool        bAMDRs232ConnectError;    // main.h
    bool        bHasPin1Error;            // main.h
    bool        bReceivePPSELECT;         // main.h
    bool        bTriggerESC;              // main.h

    // ---- golden TfMain methods (bodies: HandlerGpibMsg.cpp / HandlerBridgeCtl.cpp) ----
    void OnGpibProgramMsg(LPARAM lParam);                 // OnMyCopyMsg preamble + `case WM_GPIB_Program:`
    bool ProcessARTMessage();
    void ResetForESC(AnsiString Msg);
    void ProcessHVisionConnect();                         // tester part; ESD / EventLog / SPEA Interface gated
    void WakeupGPIB(AnsiString FuncName);
    void SendMSG_CMD(int CMD);
    void SendMSG_CMD(int CMD, AnsiString Message);
    void SendMSG_CMD_DeviceMapSRQ(int iStatus);
    void SendMSG_TestMode();
    bool RunTestProgram(bool bNeedTest, bool *bSiteOnOff = NULL);
    void CloseGpibProgram(AnsiString Src = "");
    void InitialBarCodeList();

    // ---- V906 plumbing (not golden; HandlerTesterSide.cpp) ----
    testercomm::TesterCommHub* hub;                       // default TesterCommHub::Instance()
    void SendToBridge(COPYDATASTRUCT* pcp);               // synchronous; no-op while no bridge is up
    HWND FindBridgeWindow();                              // bridge token of the running engine, NULL if none/down
    bool StartBridgeProgram();                            // hub->SelectTestType(TestIF.iTestType) (restarts a down one)
    HWND HandlerWndToken();                               // == the engines' HMountWnd (the hub's mailbox)
    void Attach(testercomm::TesterCommHub* h);            // installs Sink() as the hub's Handler sink
    static int Sink(const std::string& payload, void* ctx);
    unsigned long sinkMessages;
    int depth;

private:
    THandlerTesterSide(const THandlerTesterSide&);
    THandlerTesterSide& operator=(const THandlerTesterSide&);
};

// The one instance (created by the composition root -- P3 -- or by the ctest).  NULL until then.
extern THandlerTesterSide *fTesterSide;

#endif
