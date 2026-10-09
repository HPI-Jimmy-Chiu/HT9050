// ===========================================================================
//  KYECFTP/FTPClientUpload_St02.cpp -- golden 913 TfFTPClient::plUnloadClick (KYECFTP/FTPClient.cpp:1128-1229; 912 :1126-1227),
//  the HD page's 'Upload to Server' button, as a method of the F1 facade (KYECFTP/FTPClientForm_St02.h).
//
//  AI(W906-W202) 20261009 (St02-E), card W-202 (2) LI-9 F2-3.  Decisions: RULINGS_20261009 #8 (D-3 = the real FTP engine in both
//  builds -- wb_serve's install sets the prepare hook to SetSimMode(false), WebLotInfoFtpInstall_St02.cpp; D-4 = Gate #4 as golden,
//  a failed copy does not stop the upload); W-203 = B (Steven 20261009): the upload runs synchronously on the main loop exactly as
//  golden, with no added timeout (the act.lotInfoFtp.upload op holds FormLock for the whole of it); D-6 = A (below); D-8 = A (this
//  own TU: it includes KYECFTP/FTPClient_Transfer.h, whose global tmpList would be ambiguous next to F1's TU-local one).
//
//  Golden path reached: plUnloadClick -> UploadFileToServer2(FtpUplaodPath, <recipe>, bZip=true) (KYECFTP/FTPClient_Transfer.cpp,
//  ht9045_kyecftp) -> connect -> del / Gate #4 (Interface/TesterTCP_N06_St02.cpp) / 7z / STOR <recipe>.zip + .Offset / del.
//  [W906] customer branches (S25, D-6 = A):
//    * AMD group (IniConfig.bAMDFunction && iAMD_Function==1): golden :1132 DoCheckPassword (TfPassword dialog, unknown.com) is not in
//      the port -> refused before anything runs (falling through would upload without AMD's password).
//    * Sigurd FTP Automation (bSigurdUpload_Recipe, set only by TfLotInfo::sbRecipeUploadClick -- not ported): golden :1176-1198
//      needs fLotInfo->GenerateCheckList (absent) -> gated body, the button tail still runs.
//    * KYEC ATC file transfer (CosFunction.bUseATCFileTransfer): the port has no ATC link (ATC_InterfaceForm is
//      TATC_InterfaceFormShim, acarry_shims.h:115, without IsConnect -- the same GATE WB-16 as forms/fLotInfo.cpp:2768), so golden's
//      "connected" arm (:1202-1209 GET_ATC_Recipe) is never taken and the "not connected" arm (:1210-1221) runs as golden.
//  HT9050 (CC_PTI): [FTP] Enable FTP=0 hides the Lot Info FTP tab; if an operator enabled it, the plain branch (:1225) would run.
// ===========================================================================
#include <string>

#include "vclcompat/vcl_compat.h"
#include "KYECFTP/FTPClientForm_St02.h"
#include "KYECFTP/FTPClient_Transfer.h"     // UploadFileToServer2, bPIDTransferErr (golden TfFTPClient members, demoted)
#include "MachineType.h"                    // CC_TSMC_TAINAN, bcSetupFile, eNewATCSystem
#include "cmydef.h"                         // CUSTOMER_CODE, iAMD_Function, bHasEnteredPEModel, bEnablePEModel, bSigurdUpload_Recipe, bStartATCRun, ATC_SYSTEM
#include "Config.h"                         // IniConfig.bAMDFunction / FtpUplaodPath
#include "CosFunction.h"                    // CosFunction.bUseATCFileTransfer
#include "common.h"                         // DataPath, asATCFileTransferPath
#include "cpublic.h"                        // SetMD5ByFolder (cpublic.cpp:993)
#include "Public/ExternFunction.h"          // DeleteDirectory
#include "canary_support.h"                 // ShowMyMessage, RecordProcess (cMyDB.h repeats the same default arguments, so not both)
#include "BarcodeReader.h"                  // Barcode_Reader (BarcodeReader.cpp:445)

// ---------------------------------------------------------------------------
//  golden 913 plUnloadClick :1128-1229
// ---------------------------------------------------------------------------
void TfFTPClient::plUnloadClick(TObject *Sender)
{
    (void)Sender;
    if(IniConfig.bAMDFunction && iAMD_Function==1)                             // :1130 Ifor 20240109 add:FTP 上傳需要帳號密碼
    {
        // GATE (W906-W202 F2-3, S25): golden :1132-1136 `if(DoCheckPassword()==true) { ShowMyMessage("密碼錯誤不可上傳檔案"); return; }`
        //   -- DoCheckPassword (:4999, the TfPassword dialog) is not in the port; refused here instead of uploading without it.
        W906_St02_FtpGated("golden 913 FTPClient.cpp:1132 DoCheckPassword (AMD FTP upload password) is not in the port -- upload refused (S25)");
        return;
    }

    if(Barcode_Reader(bcSetupFile)==0)                                          // :1139 20140103 wei KYEC Barcode Reader
    {
        return;
    }

    if(bHasEnteredPEModel==true)                                                // :1144 Ifor 20160823 進入PE工程模式或者PE工程模式開啟 不可上傳檔案
    {
        if(bEnablePEModel)
            ShowMyMessage("PE engineering model can not upload files\r\nPE工程模式不可上傳檔案");
        else
            ShowMyMessage("Leave PE engineering model must download Setup File first\r\n離開PE工程模式必須先下載Setup File");
        return;
    }

    if(lstHDFile->Items->Count==0)                                              // :1153 Landam  選對一定會剩一條record
    {
        if(CUSTOMER_CODE==CC_TSMC_TAINAN)                                       //ChungHung 20150413 add for TSMC
            ShowMyMessage("未選取產品名稱");
        else
            ShowMyMessage("未選擇這路徑");
        return;
    }

    if(edtHDWaferName->Text!=lstHDFile->Items->Strings[0])                      // :1162 Landam  選對名稱會一樣
    {
        if(CUSTOMER_CODE==CC_TSMC_TAINAN)                                       //ChungHung 20150413 add for TSMC
            ShowMyMessage("未選取產品名稱");
        else
            ShowMyMessage("未選擇這路徑");
        return;
    }

    plUnload->Enabled=false;                                                    // :1171

    SetMD5ByFolder(DataPath+edtHDWaferName->Text);                              // :1173 Steven 20170927 (wei) : 將工作檔加入檢查碼

    if(bSigurdUpload_Recipe==true)                                              // :1175 KaiChen 20190530 ：Sigurd FTP Automation
    {
        // GATE (W906-W202 F2-3, S25): golden :1177-1198 -- CC_SIGURD_ChungXing switches the setup file first, then
        //   fLotInfo->GenerateCheckList x2 (absent: forms/fLotInfo.h "exited under A") and two uploads to FTPAutomation_Up_ServerPath.
        //   bSigurdUpload_Recipe is set only by TfLotInfo::sbRecipeUploadClick (not ported), so this is unreachable today.
        W906_St02_FtpGated("golden 913 FTPClient.cpp:1177-1198 Sigurd FTP Automation upload (GenerateCheckList not in the port, S25)");
    }
    else if(CosFunction.bUseATCFileTransfer==true)                              // :1200 Eastsun 20260522 整合: ATC FileTransfer 模式，先觸發 ATC 端傳送 PID
    {
        const bool kW906_ATCIsConnect = false;                                  // [W906] golden ATC_InterfaceForm->IsConnect(): no ATC link in the port (file banner)
        if(bStartATCRun && kW906_ATCIsConnect)                                  // :1202
        {
            // golden :1204-1208 ATC_InterfaceForm->GET_ATC_Recipe(); RecordProcess / memoFTP "FTP Upload PID File Transfer from ATC
            //   Start!!"; bPIDTransferErr=false; return;  (the upload itself follows in TfLotInfo::ATCTransferFileTimeTimer) -- never
            //   taken here (kW906_ATCIsConnect).
            W906_St02_FtpGated("golden 913 FTPClient.cpp:1204-1208 ATC PID file transfer (GET_ATC_Recipe gated, ATCTransferFileTimeTimer not in the port)");
            plUnload->Enabled=true;                                             // [W906] golden re-enables it after the ATC transfer (timer above)
            return;
        }
        else
        {
            if(CosFunction.bUseATCFileTransfer==true && ATC_SYSTEM==eNewATCSystem)  // :1212 Eastsun 20260522 整合
            {
                memoFTP->Lines->Add("ATC System Connect Error, PID FileTransfer Fail!!");
                RecordProcess("ATC System Connect Error, PID FileTransfer Fail!!");
                DeleteDirectory(asATCFileTransferPath);                         // :1216 (common.cpp:358 D:\ATC\Data\SaveFile\)
                CreateDir(asATCFileTransferPath);
            }
            bPIDTransferErr=true;                                               // :1219
            UploadFileToServer2(sRootPath+IniConfig.FtpUplaodPath, edtHDWaferName->Text, true);
        }
    }
    else
    {
        UploadFileToServer2(sRootPath+IniConfig.FtpUplaodPath, edtHDWaferName->Text, true);    // :1225 jou 2015-01-16 修正FTP upload error
    }
    plUnload->Enabled=true;                                                     // :1227
    W906_SetText(edtHDWaferName, "");                                           // :1228 edtHDWaferName->Text="" (VCL: OnChange -> FilterList)
}
