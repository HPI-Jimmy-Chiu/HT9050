// ===========================================================================
//  FileRW/_ProxyTry.cpp  --  AI(W906-FASTCLK) 20261003: the NON-BLOCKING proxy read for the tick thread.  NOT in golden.
//
//  RULINGS_20261002 #7 (NIGHT_REPORT s0 #35 = A, user 1002 09:1x): 「冷卻風扇拿 FormLock 的鎖先改（主迴圈不該等 HTTP 執行緒）」.
//
//  WHAT WAITED.  Two golden reads of the Configuration form sit in the heater body (golden THeaterThread, now on the 20 ms
//  fast clock, FastClockWbServe.cpp):
//    csystem.cpp DoSwCoolingFan GATE H1-08  golden 906 csystem.cpp:20442-20446  fShow && chkHeater->Checked && ActivePageIndex==ecp1TabSheet9
//    bthermo.cpp DoThermo       GATE G14a   golden 906 bthermo.cpp:1004-1005    fShow && ActivePageIndex==ecp1TabSheet9
//  Their port lines read the proxies through FileRW_ProxyChecked / FileRW_ProxyPageIndex (FileRW/_EditList.cpp EOF), which
//  take FormLock (JsonBridge/FormJson.cpp, one CRITICAL_SECTION).  The HTTP socket thread holds that lock for a whole request
//  -- GET /api/editlist/<...> (tools/wb_serve.cpp ApiRoute, golden FormShow-like reads) and GET /api/form/<page> (golden display
//  code, FormJson.cpp BridgePageJson) -- so EnterCriticalSection on the tick thread waited for that request, and the whole
//  main loop (MainProc, the drain, the publish, every other beat) waited with it.  Only while the Configuration page is open
//  (both lines test the page table first), which is exactly when its editlist requests run.
//
//  WHAT THIS DOES.  FormTryLock (TryEnterCriticalSection): the lock is free or already this thread's -> both reads run under
//  it, exactly as the blocking pair would (the inner EnterCriticalSection of FileRW_ProxyChecked / FileRW_ProxyPageIndex is
//  then a recursion and returns at once); another thread holds it -> nothing is read, false, and the caller skips THIS beat
//  by taking golden's own early exit (DoSwCoolingFan returns false before touching the fan; DoThermo returns before
//  DoThermoReal).  The next beat (20 ms later) asks again.
//
//  WHY "skip this beat" AND NOT "move the data behind the tick thread".  The proxies are written by the tick thread
//  (form.event, editlist.save) AND by the socket thread (GET /api/editlist runs golden FormShow code that sets widgets), and
//  ELFind's registry is shared; a tick-thread copy would need a writer hook in every path that touches TfConfiguration's
//  widgets, owned by St01 -- a large change for two booleans.  Skipping is local and its cost is bounded and visible:
//    * the fan / thermo state machine is level-driven and runs every 20 ms, so a skipped beat delays it by one beat;
//    * the only answer a skipped beat can be "wrong" about is "the operator is on the heater tab" (golden: leave the fan
//      alone / do not poll) -- and skipping IS leaving it alone, the conservative direction;
//    * FileRW_ProxyTryBusyCount() counts the skipped beats (the [FASTCLK] line prints it).
//
//  Thread: any, but meant for the wb_serve tick thread.  Programs without this file (every ctest that does not compile it,
//  wb_publish) get FileRW/_fallback_trylock.cpp (ht9045_globals): never busy, checked false, page -1 -- the answers of
//  FileRW/_fallback.cpp's FileRW_ProxyChecked / FileRW_ProxyPageIndex.
// ===========================================================================
#include <atomic>

namespace ht9045 { namespace formjson { bool FormTryLock(); void FormUnlock(); } }   // JsonBridge/FormJson.cpp (the one CRITICAL_SECTION)
bool FileRW_ProxyChecked(const char* form, const char* name);                          // FileRW/_EditList.cpp EOF (takes FormLock; recursive here)
int  FileRW_ProxyPageIndex(const char* form, const char* name);                        // FileRW/_EditList.cpp EOF (same)

namespace {
std::atomic<unsigned long> g_busy(0);
}

// false = FormLock is held by another thread right now: nothing read, *checked / *pageIndex untouched.
// true  = read under the lock; a null name (or out pointer) skips that read.
bool FileRW_ProxyTryRead(const char* form, const char* checkName, bool* checked, const char* pageName, int* pageIndex)
{
    if (!ht9045::formjson::FormTryLock()) {
        g_busy.fetch_add(1);
        return false;
    }
    struct Unlock { ~Unlock() { ht9045::formjson::FormUnlock(); } } unlock;   // also on an exception out of a proxy read
    if (checkName && checked)  *checked   = FileRW_ProxyChecked(form, checkName);
    if (pageName && pageIndex) *pageIndex = FileRW_ProxyPageIndex(form, pageName);
    return true;
}

// Beats skipped because FormLock was busy, since the program started (FastClockWbServe.cpp prints it).
unsigned long FileRW_ProxyTryBusyCount() { return g_busy.load(); }
