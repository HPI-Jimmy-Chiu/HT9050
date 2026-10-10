//---------------------------------------------------------------------------
//  KYECFTP/KyecFtpSeams_St02.h -- the seams KYECFTP/FTPClient_Transfer.cpp calls on the golden lines that create the FTP
//  engine and that run external programs.  AI(W906-W202) 20261009 (St02-E), card W-202 (2) LI-9 F2-2 (RULINGS_20261009 #8,
//  D-2 = same-line seams on the laptop's lines).  Bodies: KYECFTP/KyecFtpSeams_St02.cpp (ht9045_kyecftp).
//
//  Every hook is null by default, and then each call does exactly what the golden line did:
//    W906_KyecFtpPrepare(engine)   -- nothing (the engine keeps its own default);  F2-3's wb_serve install sets the hook to
//                                     SetSimMode(false) (D-3 = a real engine in both builds); a ctest sets it to a fake server.
//    W906_KyecFtpSystem(cmd)       -- std::system(cmd)        (golden `system(str.c_str());`, the del / 7z command lines)
//    W906_KyecFtpCopyFile(a, b, f) -- CopyFileA(a, b, f)      (golden `CopyFile(...)`, e.g. the 7z.exe copy into d:\HT9045)
//  A ctest sets the system / CopyFile hooks so that no real cmd.exe, 7z or file copy runs (Steven's KYEC e = B concern).
//---------------------------------------------------------------------------
#ifndef KyecFtpSeams_St02H
#define KyecFtpSeams_St02H

namespace Nmftp { class TNMFTP; }

extern void (*W906_KyecFtpPrepareHook)(Nmftp::TNMFTP* pEngine);
void W906_KyecFtpPrepare(Nmftp::TNMFTP* pEngine);

extern int (*W906_KyecFtpSystemHook)(const char* sCmd);
int W906_KyecFtpSystem(const char* sCmd);

extern bool (*W906_KyecFtpCopyFileHook)(const char* sSrc, const char* sDst, bool bFailIfExists);
bool W906_KyecFtpCopyFile(const char* sSrc, const char* sDst, bool bFailIfExists);

// AI(W906-P14) 20261010 (St02-E) POOL-14 MR-B1 (RULINGS_20261010 #8):
//   W906_KyecFtpShellExec(file, params) -- golden `ShellExecute(this, "open", file, params, NULL, SW_HIDE)` (TfLotInfo::DownloadFromServer's
//                                          7z lines, golden 913 uLotInfo.cpp:4419 / :4632); returns the HINSTANCE as an int (> 32 = ok).
//   W906_KyecFtpDownloadFromServer(name, bFromFTP) -- golden `fLotInfo->DownloadFromServer(...)` (FTPClient_Transfer.cpp Gate #2,
//                                          the laptop's :264 / :266, nodded 1010 17:2x).  Null hook = false, the old Gate #2 stand-in;
//                                          wb_serve installs forms/fLotInfo_Download_St02.cpp's body (WebLotInfoFtpInstall_St02.cpp).
#include "vclcompat/AnsiString.h"
extern int (*W906_KyecFtpShellExecHook)(const char* sFile, const char* sParams);
int W906_KyecFtpShellExec(const char* sFile, const char* sParams);

extern bool (*W906_KyecFtpDownloadFromServerHook)(const vclcompat::AnsiString& sDLFileName, bool bFromFTP);
bool W906_KyecFtpDownloadFromServer(const vclcompat::AnsiString& sDLFileName, bool bFromFTP);

#endif
