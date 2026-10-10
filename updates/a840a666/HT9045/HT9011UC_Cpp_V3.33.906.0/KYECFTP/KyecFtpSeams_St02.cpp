//---------------------------------------------------------------------------
//  KYECFTP/KyecFtpSeams_St02.cpp -- see KyecFtpSeams_St02.h.  AI(W906-W202) 20261009 (St02-E) LI-9 F2-2.
//  Own TU in ht9045_kyecftp (CMakeLists.txt, St02's ELA-R0 line) and in the laptop's test_FTPClient_Transfer sources: it needs
//  nothing but the C library and kernel32, so every target that compiles FTPClient_Transfer.cpp can add it.
//---------------------------------------------------------------------------
#include "KYECFTP/KyecFtpSeams_St02.h"

#include <cstdlib>   // std::system
#if defined(_WIN32)
#include <windows.h> // CopyFileA
#endif

void (*W906_KyecFtpPrepareHook)(Nmftp::TNMFTP* pEngine) = 0;
int  (*W906_KyecFtpSystemHook)(const char* sCmd) = 0;
bool (*W906_KyecFtpCopyFileHook)(const char* sSrc, const char* sDst, bool bFailIfExists) = 0;

void W906_KyecFtpPrepare(Nmftp::TNMFTP* pEngine)
{
    if(W906_KyecFtpPrepareHook && pEngine)
        W906_KyecFtpPrepareHook(pEngine);
}

int W906_KyecFtpSystem(const char* sCmd)
{
    if(W906_KyecFtpSystemHook)
        return W906_KyecFtpSystemHook(sCmd);
    return std::system(sCmd);                                                   // golden system(str.c_str())
}

bool W906_KyecFtpCopyFile(const char* sSrc, const char* sDst, bool bFailIfExists)
{
    if(W906_KyecFtpCopyFileHook)
        return W906_KyecFtpCopyFileHook(sSrc, sDst, bFailIfExists);
#if defined(_WIN32)
    return CopyFileA(sSrc, sDst, bFailIfExists ? TRUE : FALSE) != FALSE;       // golden CopyFile(...) (ANSI build)
#else
    return false;
#endif
}

// ---------------------------------------------------------------------------
//  AI(W906-P14) 20261010 (St02-E) POOL-14 MR-B1 -- see the header.
// ---------------------------------------------------------------------------
#if defined(_WIN32)
#include <shellapi.h>                // ShellExecuteA (shell32: in MinGW's and MSVC's default link libraries)
#endif
int  (*W906_KyecFtpShellExecHook)(const char* sFile, const char* sParams) = 0;
bool (*W906_KyecFtpDownloadFromServerHook)(const vclcompat::AnsiString& sDLFileName, bool bFromFTP) = 0;

int W906_KyecFtpShellExec(const char* sFile, const char* sParams)
{
    if(W906_KyecFtpShellExecHook)
        return W906_KyecFtpShellExecHook(sFile, sParams);
#if defined(_WIN32)
    return (int)(INT_PTR)ShellExecuteA(NULL, "open", sFile, sParams, NULL, SW_HIDE);   // golden ShellExecute(this, "open", ...) -- no owner form here
#else
    return 0;
#endif
}

bool W906_KyecFtpDownloadFromServer(const vclcompat::AnsiString& sDLFileName, bool bFromFTP)
{
    if(W906_KyecFtpDownloadFromServerHook)
        return W906_KyecFtpDownloadFromServerHook(sDLFileName, bFromFTP);
    return false;                                                               // the old Gate #2 stand-in (nothing installed)
}
