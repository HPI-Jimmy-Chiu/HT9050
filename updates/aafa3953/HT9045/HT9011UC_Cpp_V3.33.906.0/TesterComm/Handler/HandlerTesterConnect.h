// ===========================================================================
//  TesterComm/Handler/HandlerTesterConnect.h -- the gated pieces of golden TfMain::ChangeTesterConnect (GB P2d D1 / D2 /
//  D3 / D6 / D7) as hook bodies.  AI(W906-D1D7) 20260928 (St02-E).  See HandlerTesterConnect.cpp.
//
//  Seats: LogObjects.h / LogObjects.cpp EOF (ht9045_db -- in every link that has ht9045_forms).  Caller: forms/fMain.cpp
//  TfMain::ChangeTesterConnect, one same-line call per item.  Installed by W906_TesterConnectRulesInstall() (wb_serve,
//  from W906_TesterCommInit).  Not installed = the seat is 0 = the port's behaviour before D1-D7 (every ctest that does
//  not call the installer).  D4 (the forced return to Operator) is WebLogin.cpp W906_WebLoginForceOperator.
// ===========================================================================
#ifndef TESTERCOMM_HANDLER_HANDLERTESTERCONNECT_H
#define TESTERCOMM_HANDLER_HANDLERTESTERCONNECT_H

// D1  golden 906_0625_Steven main.cpp:12072-12074 -- the access check.  true = golden's if-body runs (switch allowed).
bool W906_CtcD1Access(bool bRemote);
// D2  golden :12076-12087 -- the IC-in-machine refusal (#ifndef SOFT_SIMULTE).  true = refused: the caller returns 1.
//     Shows MES1646 only for the iTester button (Mode==10 && Msg).
bool W906_CtcD2IcRefuse(int Mode, bool Msg);
// D3  golden :12093-12097 -- I27 Manual Sort.  true = golden took the manual-sort branch (the On-Line else is skipped).
bool W906_CtcD3ManualSort();
// D6  golden :12144-12152 -- ON_LINE -> 2D_SORT (the caller keeps golden's condition :12141-12142).
void W906_CtcD6To2DSort();
// D7  golden :12212-12237 -- the ASM On-Line arm (the caller keeps golden's condition :12210).
void W906_CtcD7AsmOnLine();

// Puts the five bodies into their seats (LogObjects.h).  Idempotent.
void W906_TesterConnectRulesInstall();

#endif
