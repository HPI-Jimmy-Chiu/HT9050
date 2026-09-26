//------------------------------------------------------------------------------
// AI(W906-TOTALYIELD) 20260927: golden cmydef.cpp:5885-5901 GetTotalYield_double / GetTotalYield_Str, line for line.
//   The golden copy in cmydef.cpp:6116-6134 stays inside its TODO(W6) gate as the reference text; these are the live
//   definitions (ht9045_globals, CMakeLists.txt next to cmydef.cpp).  Why now: ProductionInfo/uPAT_Function.cpp:1232
//   (PAT real-time report "Test Yield") calls GetTotalYield_Str() and its header (:221) says the substrate satisfies it
//   -- it did not: `C:\MinGW\bin\nm.exe` showed `U __Z17GetTotalYield_Strv` in uPAT_Function.cpp.obj and no
//   definition in any lib*.a (INBOX 81).  Today PAT_Function is not pulled into any link, so this only removes a latent
//   undefined reference; iSECSGEMPass / iSECSGEMFail are live in cmydef.cpp:4519-4521.
//   ⚠ golden's format "%02.2f%" ends in a lone '%' -- kept verbatim; what it prints after the number is the CRT's call.
//------------------------------------------------------------------------------
#include "cmydef.h"

double GetTotalYield_double()
{
    double dvalue=double(iSECSGEMPass+iSECSGEMFail);
    if(dvalue==0.0)
    {
        return 0.0;
    }
    else
    {
        return (double(iSECSGEMPass)/dvalue)*100.0;
    }
}
//------------------------------------------------------------------------------
AnsiString GetTotalYield_Str()
{
    return AnsiString().sprintf("%02.2f%",GetTotalYield_double());
}
