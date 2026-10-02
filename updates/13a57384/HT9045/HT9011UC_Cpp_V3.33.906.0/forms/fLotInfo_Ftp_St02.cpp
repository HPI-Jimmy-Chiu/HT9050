// ===========================================================================
//  forms/fLotInfo_Ftp_St02.cpp -- golden TfLotInfo::btnFtpServerClick (906_0625_Steven uLotInfo.cpp:5001-5126), card LI-9 F1.
//
//  AI(W906-LI9-F1) 20261002 (St02-E helper).  See forms/fLotInfo_Ftp_St02.h.  Golden lines are uLotInfo.cpp of
//  906_0625_Steven; ".dfm" = its uLotInfo.dfm.  Deviations are marked [W906]:
//    * Ptr comes from the dfm Tag (the four buttons whose OnClick is btnFtpServerClick), not from a VCL Sender.
//    * Two refusals VCL makes before OnClick ever runs: a disabled button (ckernel.cpp sets btnFtpServer / btnFtpHD
//      ->Enabled every state) and a hidden tsFTP (the buttons are on it; tag lot.tab.tsFTP).
//    * ShowModal does not block (KYECFTP/FTPClientForm_St02.h): when the dialog opens, :5110-5125 run when it closes.
//    * Customer branches (S25): the CC_JSCC_OS body is gated and refuses (gating it to fall through would give that
//      customer a path golden never takes); CC_TSMC_TAINAN's XCOPY / _DelTree and CC_TERAPOWER's save are gated; the
//      refusals that only return (TSMC with FTP off, SPIL on the Tester button) are golden as they are.
//    * Tag 0 (Server) is network = card F2: ShowFTPModal(0) ends at its gate (no socket), the reply says f2-not-yet.
// ===========================================================================
#include "forms/fLotInfo_Ftp_St02.h"

#include <cstdio>
#include <string>

#include "vclcompat/vcl_compat.h"
#include "MachineType.h"            // CC_*
#include "cmydef.h"                 // CUSTOMER_CODE, AccessLevel, SystemStart
#include "Config.h"                 // IniConfig.bEnableFTP / iServerEnable / iHDEnable / bSPILFunction
#include "forms/fLotInfo.h"         // fLotInfo->btnFtpServer / btnFtpHD / btnFtpTester / btnDataFTPSaveToData / tsFTP
#include "forms/fMain.h"            // fMain->cbSetupFileName (CC_TERAPOWER, :5121)
#include "KYECFTP/FTPClientForm_St02.h"

namespace {

TButton* ButtonForTag(int iTag)                                                 // golden :5003-5004 Ptr=(TButton *)Sender;
{
    if (fLotInfo == 0) return 0;
    switch (iTag)
    {
        case 0: return fLotInfo->btnFtpServer;                                  // .dfm :1830 (Tag 0)
        case 1: return fLotInfo->btnFtpHD;                                      // .dfm :1845 Tag = 1
        case 2: return fLotInfo->btnFtpTester;                                  // .dfm :1861 Tag = 2
        case 3: return fLotInfo->btnDataFTPSaveToData;                          // .dfm :1893 Tag = 3
    }
    return 0;
}

W906_St02_FtpClick Ret(const char* guard, const char* golden, const std::string& detail)
{
    W906_St02_FtpClick r;
    r.guard = guard;
    r.golden = golden;
    r.detail = detail;
    r.opened = fFTPClient->W906_bModalOpen;
    std::printf("[LI9-F1] btnFtpServerClick -> %s (%s) %s\n", guard, golden, detail.c_str());
    return r;
}

std::string Num(int v)
{
    char b[32];
    std::snprintf(b, sizeof(b), "%d", v);
    return std::string(b);
}

// golden :5110-5125 -- what runs after ShowFTPModal returned
void Tail(TButton* Ptr, bool bKeepShow)
{
    if (!bKeepShow)                                                             // [W906] see the call site
        fFTPClient->bShow=false;                                                // :5110

    if(CUSTOMER_CODE==CC_TSMC_TAINAN && SystemStart==false)                     // :5112 ChungHung 20150413 add for TSMC
    {
        W906_St02_FtpGated("golden uLotInfo.cpp:5114 CC_TSMC_TAINAN _DelTree(IniData DataFTP folder, GetLastOpenFN()) (S25)");
    }

    Ptr->Enabled=true;                                                          // :5117

    if(CUSTOMER_CODE==CC_TERAPOWER)                                             // :5119
    {
        if(fMain->cbSetupFileName->Text.Pos("_NET")>0)
        {
            W906_St02_FtpGated("golden uLotInfo.cpp:5123 CC_TERAPOWER btSaveSetupFile->Click() (S25; btSaveSetupFileClick is card F2)");
        }
    }
}

void ModalEndedTail(int iTag)                                                   // [W906] the dialog closed: golden ShowModal returned
{
    TButton* Ptr = ButtonForTag(iTag);
    if (Ptr != 0)
        Tail(Ptr, false);
}

}  // namespace

// ---------------------------------------------------------------------------
//  golden btnFtpServerClick :5001-5126
// ---------------------------------------------------------------------------
W906_St02_FtpClick W906_St02_LotInfo_btnFtpServerClick(int iTag)
{
    TButton *Ptr;                                                               // :5003
    Ptr=ButtonForTag(iTag);                                                     // :5004 Ptr=(TButton *)Sender;
    if (Ptr == 0)
        return Ret("bad-tag", "uLotInfo.dfm:1830-1908", "tag must be 0 (Server), 1 (HD), 2 (Tester Name) or 3 (DataFTP Save to Data)");
    if (Ptr->Enabled == false)                                                  // [W906] VCL: a disabled TButton fires no OnClick
        return Ret("button-disabled", "ckernel.cpp (golden :1068-1072 / :1590-1676 set btnFtpServer / btnFtpHD ->Enabled)",
                   "the button is disabled (" + std::string(Ptr->Caption.c_str()) + ")");
    if (fLotInfo->tsFTP != 0 && fLotInfo->tsFTP->TabVisible == false)           // [W906] the buttons sit on tsFTP
        return Ret("tab-hidden", "uLotInfo.cpp:7093 Timer2Timer tsFTP->TabVisible=IniConfig.bEnableFTP",
                   "the FTP tab is hidden (config.ini [FTP] Enable FTP; in the SIM build SimNetMask clears it at every start, W58)");

    if(CUSTOMER_CODE==CC_JSCC_OS)                                               // :5006 RogerYang 20260127 : Add For JSCC_OS download by Device list
    {
        W906_St02_FtpGated("golden uLotInfo.cpp:5008-5039 CC_JSCC_OS Device ID / DeviceCorrespond.ini (S25)");
        return Ret("s25-customer", "uLotInfo.cpp:5006-5040", "CC_JSCC_OS: the Device ID path is customer-specific (S25), not ported");
    }

    if(IniConfig.bEnableFTP==false && CUSTOMER_CODE==CC_TSMC_TAINAN)            // :5042 ChungHung 20150413 add for TSMC   //ChungHung 20150415 add for TSMC
    {
        Ptr->Enabled=true;
        return Ret("tsmc-ftp-off", "uLotInfo.cpp:5042-5046", "CC_TSMC_TAINAN with [FTP] Enable FTP off");
    }
    AnsiString TargetPath,SourcePtah,str;                                       // :5047
    (void)TargetPath; (void)SourcePtah; (void)str;

    Ptr->Enabled=false;                                                         // :5049
    if(Ptr->Tag==0)
    {
        if(IniConfig.iServerEnable>AccessLevel)                                 // :5052
        {
            Ptr->Enabled=true;
            return Ret("access-level", "uLotInfo.cpp:5052-5056",
                       "IniConfig.iServerEnable " + Num(IniConfig.iServerEnable) + " > AccessLevel " + Num(AccessLevel));
        }
    }
    else if(Ptr->Tag==1)
    {
        if(IniConfig.iHDEnable>AccessLevel)                                     // :5060
        {
            Ptr->Enabled=true;
            return Ret("access-level", "uLotInfo.cpp:5060-5064",
                       "IniConfig.iHDEnable " + Num(IniConfig.iHDEnable) + " > AccessLevel " + Num(AccessLevel));
        }
    }
    else if(Ptr->Tag==2)
    {
        if(IniConfig.bSPILFunction==true)                                       //JerryYang 20170328 (Jou) 矽品客戶碼統一用SPILFunction
        {
            return Ret("spil", "uLotInfo.cpp:5068-5071",
                       "IniConfig.bSPILFunction: golden returns and leaves the button disabled (golden quirk, kept)");
        }
    }
    else if(Ptr->Tag==3)                                                        //ChungHung 20150413 add for TSMC
    {
        if(CUSTOMER_CODE==CC_TSMC_TAINAN && SystemStart==false)                 // :5075
        {
            W906_St02_FtpGated("golden uLotInfo.cpp:5077-5089 CC_TSMC_TAINAN XCOPY D:\\HT9045\\IniData\\DataFTP\\<setup file> -> Data\\ (S25)");
        }
        Ptr->Enabled=true;                                                      // :5091
        return Ret("tag3-return", "uLotInfo.cpp:5091-5092", "DataFTP Save to Data: golden only does the CC_TSMC_TAINAN copy (S25) and returns");
    }

//jou 2013-01-08 make code 不用鎖定,隨時發現機台對應錯誤就立即更正.立即產生對應檔.最低限度為按下pause就能更改,然後續run
//    if(fMain->CheckCanChangeRealDummy()==false)
//    {
//        Ptr->Enabled=true;
//        return;
//    }

    //jou 2013-01-08 不用鎖定,隨時發現機台對應錯誤就立即更正.立即產生對應檔.最低限度為按下pause就能更改,然後續run
    if(SystemStart==true)                                                       // :5103
    {
        Ptr->Enabled=true;
        return Ret("system-start", "uLotInfo.cpp:5103-5107", "SystemStart (the machine is running)");
    }

    // [W906] golden :5109 blocks in ShowModal until the dialog closes, then runs :5110-5125.  Here: when the dialog opens,
    //   :5110-5125 are handed to it (ModalEndedTail runs them from TfFTPClient::Close); when it does not open (an error, or
    //   "already opened") they run now, as golden's would.
    const bool bWasOpen = fFTPClient->W906_bModalOpen;
    if (!bWasOpen)
    {
        fFTPClient->W906_iOpenerTag = Ptr->Tag;
        fFTPClient->W906_ModalEnded = &ModalEndedTail;
    }
    fFTPClient->ShowFTPModal(Ptr->Tag);                                         // :5109
    if (!bWasOpen && fFTPClient->W906_bModalOpen)
    {
        W906_St02_FtpClick r;
        r.opened = true;
        std::printf("[LI9-F1] btnFtpServerClick tag %d -> FTP dialog open (ShowFTPModal(%d))\n", Ptr->Tag, Ptr->Tag);
        return r;
    }
    if (!bWasOpen)
    {
        fFTPClient->W906_ModalEnded = 0;
        fFTPClient->W906_iOpenerTag = -1;
    }
    // [W906] bWasOpen: golden's dialog is modal -- no Lot Info button can be pressed while it is up, so golden's :5110
    //   `fFTPClient->bShow=false;` after "FTP Form already Opened!!" never meets an open dialog.  Here it would mark the open
    //   web dialog closed (and the next press would build a second one over it), so that one line is skipped in that case.
    Tail(Ptr, bWasOpen);                                                        // :5110-5125
    if (bWasOpen)
        return Ret("already-open", "KYECFTP/FTPClient.cpp:1219-1227", "FTP Form already Opened!!");
    if (Ptr->Tag == 0)
        return Ret("f2-not-yet", "KYECFTP/FTPClient.cpp:1236-1441",
                   "Server = the FTP server list / download (card LI-9 F2) -- not available yet; nothing was sent to the network");
    return Ret("not-opened", "KYECFTP/FTPClient.cpp:1520-1570", "ShowFTPModal did not open the dialog");
}

// ---------------------------------------------------------------------------
//  [W906] dfm values of the FTP tab (header)
// ---------------------------------------------------------------------------
void W906_St02_LotInfoFtpDfmDefaults()
{
    if (fLotInfo == 0 || fLotInfo->btnFtpServer == 0 || fLotInfo->btnFtpServer->Caption != "")
        return;
    struct Row { TButton* b; int tag; const char* cap; bool vis; };
    const Row rows[] = {
        { fLotInfo->btnFtpServer,         0, "Server",                 true  },   // .dfm :1830-1844
        { fLotInfo->btnFtpHD,             1, "HD",                     true  },   // .dfm :1845-1860
        { fLotInfo->btnFtpTester,         2, "Tester Name",            true  },   // .dfm :1861-1876
        { fLotInfo->btnDataFTPSaveToData, 3, "DataFTP Save  to  Data", CUSTOMER_CODE==CC_TSMC_TAINAN },   // .dfm :1893-1908; golden FormShow :566-581
        { fLotInfo->btnFTPTryConnect,     4, "Connection test",        false },   // .dfm :1909-1924 Visible = False (CC_GIGAS shows it, S25)
    };
    for (unsigned i = 0; i < sizeof(rows) / sizeof(rows[0]); ++i)
    {
        if (rows[i].b == 0) continue;
        rows[i].b->Tag     = rows[i].tag;
        rows[i].b->Caption = rows[i].cap;
        rows[i].b->Visible = rows[i].vis;
        rows[i].b->Enabled = true;
    }
    if (fLotInfo->lbFTPStatus != 0)                                             // .dfm :1815-1829
    {
        fLotInfo->lbFTPStatus->Caption = "Status";
        fLotInfo->lbFTPStatus->Visible = false;
    }
}
