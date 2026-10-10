// ===========================================================================
//  forms/fLotInfo_Download_St02.h -- golden 913 TfLotInfo's recipe-download pipeline as St02 free functions (POOL-14 / LI-9
//  F2-down, MR-B1..B3; RULINGS_20261010 #8).  TfLotInfo's facade (forms/fLotInfo.h) is the laptop's / St01's, so -- as E-020
//  W906_LotInfo_SECSLotStart and LI-9 F1 W906_St02_LotInfo_btnFtpServerClick do -- the bodies are St02 free functions that only
//  touch fLotInfo's existing members; the members golden has and the facade lacks are St02 stand-ins declared here.
//
//  AI(W906-P14) 20261010 (St02-E).  wb_serve only (it reaches fFTPClient / fOffSet / fMain / ATC globals); a ctest links the TU.
//  Golden = D:\HT9045\HT9011UC_Code_V3.33.913.0_20261008_steven uLotInfo.cpp ("913 :N" below; identical to 0618 unless said).
//
//  MR-B1 (this file's first version): DownloadFromServer (913 :4308-4830) + WriteFTPSetupFileChangeLog (:14024-14033).  It is
//    reached through KYECFTP/KyecFtpSeams_St02.h W906_KyecFtpDownloadFromServer (golden LoadFileFormServer2's
//    fLotInfo->DownloadFromServer, FTPClient_Transfer.cpp Gate #2), installed by WebLotInfoFtpInstall_St02.cpp.
//    DoBackupSetupFile (:3630-3907) / DoOverWriteSetupFile (:3908-4307) are MR-B2 -- until then their St02 functions are named stubs.
//  MR-B3: btSaveSetupFileClick (:5254-5349) + ClearAllSetupFile (:12231-12288).
//  Nothing calls the pipeline live until MR-C (plSLoadClick), so MR-B1..B3 change no behaviour.
// ===========================================================================
#ifndef FORMS_FLOTINFO_DOWNLOAD_ST02_H
#define FORMS_FLOTINFO_DOWNLOAD_ST02_H

#include "vclcompat/vcl_compat.h"

// golden 913 uLotInfo.h:1366 `AnsiString iAutoClean[69];` (TfLotInfo member, absent from the facade): DoBackupSetupFile (MR-B2) fills
//   it, DownloadFromServer :4739 writes [24] back -- St02 stand-in.
extern AnsiString W906_St02_LotInfo_iAutoClean[69];

bool W906_St02_LotInfo_DownloadFromServer(AnsiString sDLFileName, bool bFromFTP = true);   // golden 913 uLotInfo.cpp:4308
void W906_St02_LotInfo_WriteFTPSetupFileChangeLog(AnsiString msg);                       // golden 913 :14024
int  W906_St02_LotInfo_DoBackupSetupFile(AnsiString DataPath, AnsiString sDLFileName1);   // golden 913 :3630 (MR-B2)
int  W906_St02_LotInfo_DoOverWriteSetupFile(AnsiString DataPath, AnsiString sDLFileName); // golden 913 :3908 (MR-B2)

#endif // FORMS_FLOTINFO_DOWNLOAD_ST02_H
