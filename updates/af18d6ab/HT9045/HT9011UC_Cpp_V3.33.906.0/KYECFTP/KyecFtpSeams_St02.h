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

#endif
