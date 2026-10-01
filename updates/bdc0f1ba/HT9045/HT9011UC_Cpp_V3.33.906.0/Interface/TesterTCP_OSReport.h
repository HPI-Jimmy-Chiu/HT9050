// =============================================================================
//  Interface/TesterTCP_OSReport.h  --  golden TfTesterTCP's Open / Short report trio, as free functions
//
//  AI(W906-G023) 20261001 (St02-E): todo G-023 / work card ST02-C1.  Golden 906_0625_Steven
//  Interface/TesterTCP.cpp:699-739 PlaceOSTestResultToTray, :741-962 ProcessOSPrint, :964-1056
//  ProcessOSTrayData (declared golden TesterTCP.h:255-257).  Only reached when
//  TestIF_File.iTestType==TCP_IP_MODE (the OS tester over TCP/IP).
//
//  WHY FREE FUNCTIONS IN ht9045_sm (and not the TfTesterTCP methods in forms/fTesterTCP.cpp):
//    the bodies read OutArmSuck / TestSocket (mykitsuck.cpp) and TastCategory (cSocket.cpp), all
//    ht9045_sm.  forms/fTesterTCP.cpp is ht9045_forms, the bottom layer (CMakeLists.txt "NO UNDECLARED
//    BACK-EDGE"): forms -> sm does not configure if declared and is the AGV_predicates trap if not.
//    Same split the tree already uses for this form: TesterTCP_CopyOSTestResult (Interface/TesterTCP.cpp,
//    called from atester_ProcessCount.cpp) and TesterTCPSocket_SendTCPIPCommand (HandlerBridgeCtl.cpp,
//    forms/fMain_SetLotState.cpp).  The `#if 0` transcripts (T-5)..(T-7) in forms/fTesterTCP.cpp stay as
//    the record; calling fTesterTCP->ProcessOSPrint() is still a link error by design.
// =============================================================================
#ifndef INTERFACE_TESTERTCP_OSREPORT_H
#define INTERFACE_TESTERTCP_OSREPORT_H

void TesterTCP_PlaceOSTestResultToTray(int iSuckRow, int iSuckCol, int iTrayRow, int iTrayCol, int iAuto);   // golden :699-739
void TesterTCP_ProcessOSPrint(bool bViewOnly=false);                                                         // golden :741-962 (default: golden TesterTCP.h:256)
void TesterTCP_ProcessOSTrayData(bool bViewOnly=false);                                                      // golden :964-1056 (default: golden TesterTCP.h:257)

#endif // INTERFACE_TESTERTCP_OSREPORT_H
