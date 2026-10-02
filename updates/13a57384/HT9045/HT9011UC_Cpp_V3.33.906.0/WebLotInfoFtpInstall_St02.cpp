// ===========================================================================
//  WebLotInfoFtpInstall_St02.cpp -- wb_serve's install of card LI-9 F1 (the KYEC FTP dialog, no network).
//
//  AI(W906-LI9-F1) 20261002 (St02-E helper).  wb_serve only (it reaches WebRecipeChange.cpp, FileRW and tools/wb_serve.cpp
//  symbols); tests/test_li9_ftpclient.cpp installs fakes instead.  Called once at boot by the wb_serve.cpp claim line
//  (an explicit call, not a static-init self-registration: forms/fMain.h install-seat note).  It
//    1. installs the act.lotInfoFtp.* body (JsonBridge/actions/LotInfoFtp.cpp <- WebLotInfoFtp_St02.cpp);
//    2. points TfFTPClient's hooks at the real code: golden fMain->ChangeSetUpFile = WebRecipeChange.cpp
//       W906_RC_ChangeSetUpFile (inside a FileRW session, as recipe.change does) and fMain->cbSetupFileNameChange =
//       W906_RC_cbSetupFileNameChange (F2) -- both need the WebRecipeChange.cpp claim that drops `static` (:261 / :389);
//       W906_RecipeChainsReady (tools/wb_serve.cpp) for the port-only boot-read-chain guard;
//    3. writes the Lot Info FTP tab's dfm values once (forms/fLotInfo_Ftp_St02.h W906_St02_LotInfoFtpDfmDefaults).
// ===========================================================================
#include <cstdio>
#include <string>

#include "vclcompat/vcl_compat.h"
#include "KYECFTP/FTPClientForm_St02.h"
#include "forms/fLotInfo_Ftp_St02.h"
#include "JsonBridge/actions/LotInfoFtp.h"

int  W906_RC_ChangeSetUpFile(AnsiString FileName);   // WebRecipeChange.cpp:261 (non-static after the LI-9 F1 claim)
void W906_RC_cbSetupFileNameChange();                // WebRecipeChange.cpp:389 (same)
bool W906_RecipeChainsReady();                       // tools/wb_serve.cpp (WebRecipeChange.cpp:73 declares it the same way)
namespace filerw { void SessionBegin(const std::string& answersJson); std::string SessionJson(); }   // FileRW/_EditList.h:144-145
void W906_St02_LotInfoFtpRegisterBody();             // WebLotInfoFtp_St02.cpp

namespace {

// golden fMain->ChangeSetUpFile, in a FileRW session like WebRecipeChange.cpp recipe.change: the readers' golden messages
//   (e.g. a missing DIO file) go to the reply's "session" instead of staying behind as another request's leftovers.
int ChangeSetUpFileInSession(AnsiString FileName)
{
    filerw::SessionBegin("");
    const int mode = W906_RC_ChangeSetUpFile(FileName);
    if (W906_St02_FtpCurrentReport) W906_St02_FtpCurrentReport->sessionJson = filerw::SessionJson();
    filerw::SessionBegin("");
    return mode;
}

}  // namespace

void W906_St02_InstallLotInfoFtp()
{
    W906_St02_Ftp.ChangeSetUpFile       = &ChangeSetUpFileInSession;
    W906_St02_Ftp.cbSetupFileNameChange = &W906_RC_cbSetupFileNameChange;
    W906_St02_Ftp.RecipeChainsReady     = &W906_RecipeChainsReady;
    W906_St02_Ftp.LocalIp               = 0;                                    // golden :1919-1934 (winsock) as it is
    W906_St02_LotInfoFtpDfmDefaults();
    W906_St02_LotInfoFtpRegisterBody();
    std::printf("LI-9 F1: act.lotInfoFtp.* installed = %s (TfFTPClient HD / Tester pages; Server = F2)\n",
                ht9045::sjson::LotInfoFtpBodyInstalled() ? "yes" : "NO");
}
