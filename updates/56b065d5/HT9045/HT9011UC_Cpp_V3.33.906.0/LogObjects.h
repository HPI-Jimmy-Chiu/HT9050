// ===========================================================================
//  LogObjects.h -- create / delete the golden TfMain TMyStringList log objects (cMyDB CSV plan P1).
//  AI(W906-CSVONLY-P1) 20260926.  See LogObjects.cpp.
// ===========================================================================
#ifndef HT9045_LOGOBJECTS_H
#define HT9045_LOGOBJECTS_H

// golden TfMain::TfMain main.cpp:1550-1724 (the log part).  Call once, after the configuration and recipe load.
void W906_CreateLogObjects();
// golden TfMain::FormDestroy main.cpp:12508-12550 (the log part); the TMyStringList destructors flush.
void W906_DestroyLogObjects();

// AI(W906-D4) / AI(W906-D1D7) 20260928 (St02-E): hook seats for golden TfMain::ChangeTesterConnect's gated pieces
//   (906_0625_Steven main.cpp:12064-12257).  Defined at LogObjects.cpp EOF (ht9045_db: in every link that has ht9045_forms),
//   called from forms/fMain.cpp (which declares them itself, same text, on its :1056), installed by wb_serve only:
//   D4 by WebLogin.cpp (static init), the others by TesterComm/Handler/HandlerTesterConnect.cpp W906_TesterConnectRulesInstall.
//   0 = not installed = the port's behaviour before the item.
extern void (*W906_WebLoginForceOperatorHook)();                // D4 :12104-12131  WebLogin.cpp W906_WebLoginForceOperator
extern bool (*W906_CtcD1AccessHook)(bool bRemote);               // D1 :12072-12074  true = switch allowed
extern bool (*W906_CtcD2IcRefuseHook)(int Mode, bool Msg);       // D2 :12076-12087  true = refused (caller returns 1)
extern bool (*W906_CtcD3ManualSortHook)();                       // D3 :12093-12097  true = entered I27 manual sort
extern void (*W906_CtcD6To2DSortHook)();                         // D6 :12144-12152  ON_LINE -> 2D_SORT
extern void (*W906_CtcD7AsmOnLineHook)();                        // D7 :12212-12237  the ASM On-Line arm

// AI(W906-W143) 20261007 (St02-E): golden fMain->slMNetLog for MNetLog.cpp (W-143) -- so it does not need forms/fMain.h; null before W906_CreateLogObjects.
class TMyStringList;  TMyStringList* W906_MNetLogObj();
TMyStringList* W906_TimeDataLogObj();  TMyStringList* W906_ProdRecordLogObj();  TMyStringList* W906_TestLogObj();  TMyStringList* W906_JamAlarmLogObj();  TMyStringList* W906_TorqueLogObj();  TMyStringList* W906_TorqueLogNewObj();   // AI(W906-W150) 20261007 (St02-E): W-150 slice 1 -- fMain->slTimeData / slProdRecordLog / slTestLog / slJamAlarmLog / slTorqueLog / slTorqueLogNew (bodies LogObjects.cpp EOF); null before W906_CreateLogObjects (and in ctest).  Occupies the old blank line
#endif
