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
//    3. writes the Lot Info FTP tab's dfm values once (forms/fLotInfo_Ftp_St02.h W906_St02_LotInfoFtpDfmDefaults);
//    4. (F2-3, AI(W906-W202) 20261009 (St02-E); POOL-14 MR-A 20261010 adds the Server list, ShowFTPModal case 0's `new TNMFTP`
//       in KYECFTP/FTPClientForm_St02.cpp) the KYEC FTP engine for 'Upload to Server': the prepare hook on golden's
//       `new TNMFTP` lines (KYECFTP/FTPClient_Transfer.cpp :374 / :661, F2-2 seam) switches the engine out of its SIM default --
//       RULINGS_20261009 #8 D-3: a real FTP connection in both builds (the SIM build still masks [FTP] Enable FTP at boot,
//       SimNet/SimNetMask.cpp:32); and the NMFTP1* event log sink (KYECFTP/FTPClient_EventHandlers.h) writes into the dialog's
//       memoFTP / lstServerFile / ListBox1 as golden's handlers do, instead of the SIM capture buffer.
// ===========================================================================
#include <cstdio>
#include <string>

#include "vclcompat/vcl_compat.h"
#include "KYECFTP/FTPClientForm_St02.h"
#include "forms/fLotInfo_Ftp_St02.h"
#include "JsonBridge/actions/LotInfoFtp.h"
#include "KYECFTP/FTPClient_EventHandlers.h"   // F2-3: SetFTPClientEvtLogSink
#include "KYECFTP/MiniFtpEngine.h"             // F2-3: Nmftp::TNMFTP::SetSimMode
#include "KYECFTP/KyecFtpSeams_St02.h"         // F2-3: W906_KyecFtpPrepareHook; POOL-14 MR-B1: W906_KyecFtpDownloadFromServerHook
#include "forms/fLotInfo_Download_St02.h"      // POOL-14 MR-B1: W906_St02_LotInfo_DownloadFromServer

int  W906_RC_ChangeSetUpFile(AnsiString FileName);   // WebRecipeChange.cpp:261 (non-static after the LI-9 F1 claim)
void W906_RC_cbSetupFileNameChange();                // WebRecipeChange.cpp:389 (same)
void W906_RC_LookForFile();                          // WebRecipeChange.cpp:155 (POOL-14 MR-B3, Q4)
void W906_FTPClient_DownloadResult(const AnsiString& sServerWaferName, bool bFTP_DownloadFail);   // WebStart.cpp:3951 (St01's seat; MR-C)
void W906_FTPClient_DownloadRecordNetData();         // WebStart.cpp:3979 (St01's seat; MR-C)
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

// F2-3: golden's TNMFTP is a real socket component; the port's engine starts in SIM (KYECFTP/MiniFtpEngine.h) -> real here.
void RealFtpEngine(Nmftp::TNMFTP* pEngine)
{
    if (pEngine) pEngine->SetSimMode(false);
}

}  // namespace

// F2-3: the NMFTP1* handlers' widget writes (golden memoFTP->Lines->Add / lstServerFile->Items->Add / ListBox1->Items->Add)
// POOL-14 MR-B1 (AI(W906-P14) 20261010 (St02-E)): golden LoadFileFormServer2's fLotInfo->DownloadFromServer (FTPClient_Transfer.cpp
//   Gate #2) = forms/fLotInfo_Download_St02.cpp.  No live caller until MR-C (plSLoadClick is the only one).
static bool DownloadFromServerBody(const AnsiString& sDLFileName, bool bFromFTP)
{
    return W906_St02_LotInfo_DownloadFromServer(sDLFileName, bFromFTP);
}

void W906_St02_InstallKyecFtpEngine()
{
    W906_KyecFtpPrepareHook = &RealFtpEngine;
    W906_KyecFtpDownloadFromServerHook = &DownloadFromServerBody;               // POOL-14 MR-B1
    FTPClientEvt_LogSink sink;
    sink.AddMemoLine       = [](const AnsiString& s) { if (fFTPClient && fFTPClient->memoFTP) fFTPClient->memoFTP->Lines->Add(s); };
    sink.AddServerFileItem = [](const AnsiString& s) { if (fFTPClient && fFTPClient->lstServerFile) fFTPClient->lstServerFile->Items->Add(s); };
    sink.AddListBox1Item   = [](const AnsiString& s) { if (fFTPClient && fFTPClient->ListBox1) fFTPClient->ListBox1->Items->Add(s); };
    SetFTPClientEvtLogSink(sink);
}

void W906_St02_InstallLotInfoFtp()
{
    W906_St02_Ftp.ChangeSetUpFile       = &ChangeSetUpFileInSession;
    W906_St02_Ftp.cbSetupFileNameChange = &W906_RC_cbSetupFileNameChange;
    W906_St02_Ftp.RecipeChainsReady     = &W906_RecipeChainsReady;
    W906_St02_Ftp.LocalIp               = 0;                                    // golden :1919-1934 (winsock) as it is
    W906_St02_Ftp.LookForFile           = &W906_RC_LookForFile;                 // POOL-14 MR-B3 (Q4): golden fMain->LookForFile in ClearAllSetupFile
    W906_St02_Ftp.DownloadResult        = &W906_FTPClient_DownloadResult;       // POOL-14 MR-C: golden plSLoadClick :1006-1024 (St01's seat)
    W906_St02_Ftp.DownloadRecordNetData = &W906_FTPClient_DownloadRecordNetData; // POOL-14 MR-C: golden plSLoadClick :1066-1086 (St01's seat)
    W906_St02_LotInfoFtpDfmDefaults();
    W906_St02_LotInfoFtpRegisterBody();
    W906_St02_InstallKyecFtpEngine();                                           // F2-3
    std::printf("LI-9 F1 + F2-3 + POOL-14 MR-A: act.lotInfoFtp.* installed = %s (TfFTPClient HD / Tester pages, Upload to Server and "
                "the Server list = real FTP engine; Download to Handler = golden 913 plSLoadClick, POOL-14 MR-C)\n", ht9045::sjson::LotInfoFtpBodyInstalled() ? "yes" : "NO");
}
