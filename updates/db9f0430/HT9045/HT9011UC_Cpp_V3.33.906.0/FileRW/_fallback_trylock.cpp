// ===========================================================================
//  FileRW/_fallback_trylock.cpp  --  AI(W906-FASTCLK) 20261003
//
//  The ht9045_globals fallback of FileRW_ProxyTryRead (the real one: FileRW/_ProxyTry.cpp, compiled into wb_serve only).
//  csystem.cpp (DoSwCoolingFan GATE H1-08) and bthermo.cpp (DoThermo GATE G14a) call it, and both sit in ht9045_sm, which
//  every god-stack ctest and wb_publish link.  Those programs have no FormLock and no Configuration proxies, so this answers
//  what FileRW/_fallback.cpp's FileRW_ProxyChecked (false) and FileRW_ProxyPageIndex (-1) answer there: never busy, not
//  checked, no page -- i.e. both golden refusals stay off, as before this change.
//
//  ONE symbol in its own archive member, on purpose (FileRW/_fallback.cpp header: a fallback member is extracted whole when
//  any of its symbols is needed).  wb_serve and any test that compiles FileRW/_ProxyTry.cpp define FileRW_ProxyTryRead in a
//  direct object, so this member is never extracted there; elsewhere it is extracted alone and cannot collide.
// ===========================================================================
bool FileRW_ProxyTryRead(const char* /*form*/, const char* /*checkName*/, bool* checked, const char* /*pageName*/, int* pageIndex)
{
    if (checked)   *checked = false;
    if (pageIndex) *pageIndex = -1;
    return true;
}
