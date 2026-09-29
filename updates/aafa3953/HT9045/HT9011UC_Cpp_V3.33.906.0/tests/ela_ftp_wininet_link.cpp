// =============================================================================
//  ela_ftp_wininet_link.cpp -- COMPILE-ONLY check that the WinINet transport (EventLogAnalysis/ElaFtpWinInet.cpp) links
//  in this build.  AI(W906-ELA-R4) 20260927 (St02-E).
//
//  Built with the tests (tests/CMakeLists.txt, no add_test): a link error here means W906_ElaStart could not install the
//  WinINet transport.  It is never run by ctest.  Run by hand it only builds the transport object and destroys it:
//  Connect is never called, so it opens no session and makes no connection.
// =============================================================================
#include "EventLogAnalysis/ElaFtp.h"

#include <cstdio>

int main(int argc, char** argv)
{
    (void)argv;
    ela::FtpFactory f = &ela::NewWinInetFtp;
    if (argc > 99)                     // never true in practice; keeps the vtable (every method) referenced
    {
        ela::IElaFtp* p = f();
        p->Close();
        delete p;
    }
    std::printf("ela_ftp_wininet_link: WinINet transport linked (%s); nothing was opened\n",
                f ? "factory present" : "no factory");
    return f ? 0 : 1;
}
