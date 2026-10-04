// ===========================================================================
//  tests/test_main_scankey_stubs.cpp
//
//  //AI(W906-SCANKEY) 20261003: link stand-ins for tests/test_main_scankey.cpp.  WebMainScanKey.cpp is wb_serve-only and
//  names two things that live in wb_serve's own sources:
//    * ht9045::formjson::FormLock / FormUnlock (JsonBridge/FormJson.cpp:39, a CRITICAL_SECTION in wb_serve) -- ON the
//      tested path (MskFormLock around CleanOut / OneCycle / TrayFeed, WebMainScanKey.cpp:55).  Here they keep a depth
//      counter so the test can assert START runs outside the lock and CleanOut releases it (same shape as
//      tests/test_main_ctlbuttons_stubs.cpp:18-23, plus the counter).
//    * W906_Contact_OneCycleProcess (to be defined at the end of FileRW/DeviceForm_File.cpp, wb_serve only; golden
//      TfContact::OneCycleProcess cContact.cpp:11741-11746) -- ON the tested path (ONE CYCLE while fContact->fShow,
//      WebMainScanKey.cpp:329).  Here it only counts.
// ===========================================================================

int g_W906TestFormLockDepth = 0;
int g_W906TestContactOneCycleCalls = 0;

namespace ht9045 {
namespace formjson {
void FormLock()   { ++g_W906TestFormLockDepth; }
void FormUnlock() { --g_W906TestFormLockDepth; }
}  // namespace formjson
}  // namespace ht9045

void W906_Contact_OneCycleProcess() { ++g_W906TestContactOneCycleCalls; }
int g_W906TestModelessTicks = 0;                                       // AI(W906-ST02-SKC) 20261003 (St02-E): tools/wb_serve.cpp EOF (NB2 R188 M2) -- counts
void W906_MsgBoxModelessPanelKeyTick() { ++g_W906TestModelessTicks; }
