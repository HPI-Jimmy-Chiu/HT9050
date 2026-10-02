// ===========================================================================
//  forms/fLotInfo_Ftp_St02.h -- golden TfLotInfo::btnFtpServerClick as St02's free function (card LI-9 F1).
//
//  AI(W906-LI9-F1) 20261002 (St02-E helper).  Golden 906_0625_Steven uLotInfo.cpp:5001-5126.  The TfLotInfo facade
//  (forms/fLotInfo.h) is not edited: this calls its existing members (btnFtpServer / btnFtpHD / btnFtpTester /
//  btnDataFTPSaveToData / tsFTP) the way E-020's W906_LotInfo_SECSLotStart does.
// ===========================================================================
#ifndef FORMS_FLOTINFO_FTP_ST02_H
#define FORMS_FLOTINFO_FTP_ST02_H

#include <string>

struct W906_St02_FtpClick {
    bool        opened;      // the dialog is open after the click (ShowModal "running")
    std::string guard;       // "" = golden reached ShowFTPModal; else which golden line returned (or a port-only refusal)
    std::string golden;      // the golden line of that guard
    std::string detail;
    W906_St02_FtpClick() : opened(false) {}
};

// golden btnFtpServerClick (uLotInfo.cpp:5001-5126) for the Lot Info button with dfm Tag iTag
//   (0 btnFtpServer, 1 btnFtpHD, 2 btnFtpTester, 3 btnDataFTPSaveToData -- uLotInfo.dfm:1830-1908).
W906_St02_FtpClick W906_St02_LotInfo_btnFtpServerClick(int iTag);

// [W906] the uLotInfo.dfm values of the FTP tab's widgets (Caption / Tag / Visible / Enabled): TfLotInfo::FormShow is not run
//   in the port (forms/fLotInfo.cpp:5987-5993) and the vclcompat TControl ctor leaves them false / "" / 0.  Applied once
//   (only while btnFtpServer->Caption is still empty); golden FormShow's per-customer changes are S25 and not applied, except
//   btnDataFTPSaveToData's (golden uLotInfo.cpp:566-581, the same lines as forms/fLotInfo.cpp:3374-3389).
void W906_St02_LotInfoFtpDfmDefaults();

#endif // FORMS_FLOTINFO_FTP_ST02_H
