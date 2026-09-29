// ===========================================================================
//  TesterComm/TesterWndSeat.h -- seats for golden TfMain's bridge-window members used OUTSIDE TesterComm/:
//      this->Handle                                                    (the Handler's own window)
//      HVisionWnd                                                      (the bridge window, golden main.h:1215)
//      SendMessage(fMain->HVisionWnd, WM_COPYDATA, (WPARAM)NULL, (LPARAM)pcp)
//  AI(W906-GB-P8) 20260928 (St02-E helper): P8 bring-up gap B1 -- TfMain::GetTTLState (Command.cpp, golden 906_0625_Steven
//  Command.cpp:10324-10500, 912 :10332-10508) builds the MSG_CMD_State_TTL packet that RS232Standard turns into @WINIT; its
//  two tokens and its send were gated because TfMain has no Handle / HVisionWnd.
//  AI(W906-GB-P8) 20260928 (St02-E helper): P8 U12 -- TfMain::MachineStatus (Command.cpp, golden 906_0625_Steven
//  Command.cpp:7304-7509, send :7507; 912 :7305-7510, send :7508) answers the bridge's MSG_CMD_MachineState request
//  (golden main.cpp:16113-16116, 912 :16730-16733 -> V906 TesterComm/Handler/HandlerGpibMsg.cpp:1027-1030) through the
//  same send seat: RS232Standard "CZ status?" (Rs232Parse.cpp) and the GPIB aux path (GpibAux.cpp) read the 17 bits
//  right after their SendMSG_CMD returns.  Golden MachineStatus does NOT refresh HandlerHwnd / GpibHwnd: the packet
//  carries the tokens the last SendMSG_CMD / SendMSG_TestMode / GetTTLState left in the global HHandler2Gpib (golden
//  MessageDef.cpp:231, token writes main.cpp:18156 / :18252 / :18503 / :22094) -- kept as golden, no token seat there.
//
//  The in-process replacements are the ones every other Handler -> bridge packet already uses
//  (TesterComm/Handler/HandlerTesterSide.h, TRANSLATION_RULES rule 4):
//      this->Handle -> THandlerTesterSide::HandlerWndToken()   (the hub's mailbox = the engines' HMountWnd)
//      HVisionWnd   -> THandlerTesterSide::HVisionWnd          (ProcessHVisionConnect / WakeupGPIB: FindBridgeWindow())
//      SendMessage  -> THandlerTesterSide::SendToBridge(pcp)   (synchronous, nested sends pump -- SyncMailbox.h)
//  RS232Standard closes itself when HandlerHwnd != its HMountWnd or GpibHwnd != itself (Rs232HandlerMsg.cpp OnMyCopyMsg),
//  so the tokens are part of the fix, not decoration.
//
//  WHY SEATS.  Command.cpp is in ht9045_sm, and many ctests link ht9045_sm without ht9045_testercomm_handler (the home of
//  THandlerTesterSide).  The seats are DEFINED in LogObjects.cpp (ht9045_db -- in every link that has ht9045_forms, the
//  same place as the D1-D7 seats) and INSTALLED by W906_TesterCommInit (TesterComm/Handler/TesterCommWiring.cpp, wb_serve
//  only), cleared first by W906_TesterCommShutdown.
//  Not installed (every ctest, HT9045_TESTERCOMM=0) = the port's behaviour before B1: the token fields keep the value they
//  had (the gated golden lines did not touch them) and nothing is sent.
//  Threading: the Handler (tick) thread only -- GetTTLState and MachineStatus run inside THandlerTesterSide::Sink.
// ===========================================================================
#ifndef TESTERCOMM_TESTERWNDSEAT_H
#define TESTERCOMM_TESTERWNDSEAT_H

#include "vclcompat/vcl_compat.h"   // HWND, COPYDATASTRUCT (windows.h in vclcompat's include order)

extern HWND (*W906_TesterHandlerWndHook)();                     // golden this->Handle
extern HWND (*W906_TesterBridgeWndHook)();                      // golden HVisionWnd
extern void (*W906_TesterSendToBridgeHook)(COPYDATASTRUCT* pcp); // golden SendMessage(HVisionWnd, WM_COPYDATA, 0, pcp)

// `keep` = the field's current value, returned while the seat is empty.
inline HWND W906_TesterHandlerWnd(HWND keep) { return W906_TesterHandlerWndHook ? W906_TesterHandlerWndHook() : keep; }
inline HWND W906_TesterBridgeWnd(HWND keep)  { return W906_TesterBridgeWndHook ? W906_TesterBridgeWndHook() : keep; }
// AI(W906-GB-P8-A1) 20260928 (St02-E, St02-E2 review A1): golden SendMessage(fMain->HVisionWnd, WM_COPYDATA, ...) with
//   HVisionWnd == NULL (bridge not found yet, e.g. right after an engine (re)start) delivers nothing.  Without this check the
//   payload reached the running engine with GpibHwnd == NULL, the RS232 engine's identity check (Rs232HandlerMsg.cpp:322-329)
//   RequestClose'd, and it relaunched 10 s later -- repeatably.  So: send only while the bridge window token is set.
inline void W906_TesterSendToBridge(COPYDATASTRUCT* pcp) { if (W906_TesterSendToBridgeHook && W906_TesterBridgeWnd(NULL) != NULL) W906_TesterSendToBridgeHook(pcp); }

#endif
