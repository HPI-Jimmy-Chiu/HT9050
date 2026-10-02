// =============================================================================
//  Public/AdamTcp_St02.cpp -- the ADAMTCP.dll run-time shim (see AdamTcp_St02.h for the why, the search order, the
//  all-or-nothing rule, the -1 fallback and the [W906] S1-S4 deviations).
//
//  AI(W906-ST02-ADAM) 20261002 (St02-E helper H1).  Library: ht9045_public (leaf; kernel32 only -- LoadLibraryA /
//  GetProcAddress / FreeLibrary / GetModuleFileNameA / SetErrorMode).  Deliberately does NOT link ADAMTCPbc.lib and
//  does NOT name ADAMTCP.dll in any import table: the exe must show no "DLL Name: ADAMTCP.dll" in objdump -p.
//  Nothing here opens a socket; only the vendor DLL does, and only when golden code calls a wrapper.
// =============================================================================
#include "AdamTcp_St02.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>

bool W906_AdamEpLive();   // [W906] S5 (H4 integration pass 20261002): the EP live switch, Adam6024Integrate_St02.cpp (ht9045_sm; every user of this
                          //   file is an ht9045_sm object, so the RESCAN link groups resolve it); declared in Adam6024_St02.h too

#if defined(_WIN32)
#  include <windows.h>
#endif

namespace {

AdamTcpApi_St02 g_api = AdamTcpApi_St02();   // all entries null until bound
bool g_resolved   = false;                    // resolution attempted (DLL) or table installed (test)
bool g_bound      = false;                    // all 11 entries usable
int  g_openRef    = 0;                        // [W906] S3: successful ADAMTCP_Open not yet matched by a forwarded Close
int  g_absorbed   = 0;                        // [W906] S3: unmatched ADAMTCP_Close calls absorbed

std::string& Info()                           // function-local: no static-initialisation-order dependency
{
    static std::string s("not resolved yet");
    return s;
}

#if defined(_WIN32)
HMODULE g_hDll = 0;                           // kept for the process lifetime once bound, like a static import
#endif

bool TableComplete(const AdamTcpApi_St02& a)
{
    return a.Open && a.Close && a.Connect && a.Disconnect && a.ReadReg && a.WriteReg &&
           a.UDPOpen && a.UDPClose && a.SendReceive6KUDPCmd && a.Read6KAI && a.GetHostIdleTime;
}

#if defined(_WIN32) && defined(W906_NO_SOFT_SIMULTE)
bool EnvContains(const char* name, const char* needle)
{
    const char* v = std::getenv(name);
    return v != 0 && std::strstr(v, needle) != 0;
}

template <typename F>
void BindOne(HMODULE h, const char* name, F& slot, std::string& missing)
{
    FARPROC p = ::GetProcAddress(h, name);
    slot = reinterpret_cast<F>(p);                                              // function pointer -> function pointer
    if (p == 0)
    {
        if (!missing.empty())
            missing += ",";
        missing += name;
    }
}

// One candidate path; true = all 11 bound (g_api filled, g_hDll kept).
bool TryPath(std::string path)
{
    for (std::string::size_type i = 0; i < path.size(); ++i)    // LoadLibrary wants backslashes (CMake passes '/')
        if (path[i] == '/')
            path[i] = char(92);
    // No "cannot find" / "not a valid Win32 application" box from the loader (restored right after).
    const UINT oldMode = ::SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOOPENFILEERRORBOX);
    HMODULE h = ::LoadLibraryA(path.c_str());
    const DWORD err = ::GetLastError();
    ::SetErrorMode(oldMode);
    if (h == 0)
    {
        char buf[64];
        std::snprintf(buf, sizeof(buf), " (LoadLibrary error %lu)", static_cast<unsigned long>(err));
        Info() += "; " + path + buf;
        return false;
    }
    AdamTcpApi_St02 a = AdamTcpApi_St02();
    std::string missing;
    BindOne(h, "ADAMTCP_Open",                a.Open,                missing);
    BindOne(h, "ADAMTCP_Close",               a.Close,               missing);
    BindOne(h, "ADAMTCP_Connect",             a.Connect,             missing);
    BindOne(h, "ADAMTCP_Disconnect",          a.Disconnect,          missing);
    BindOne(h, "ADAMTCP_ReadReg",             a.ReadReg,             missing);
    BindOne(h, "ADAMTCP_WriteReg",            a.WriteReg,            missing);
    BindOne(h, "ADAMTCP_UDPOpen",             a.UDPOpen,             missing);
    BindOne(h, "ADAMTCP_UDPClose",            a.UDPClose,            missing);
    BindOne(h, "ADAMTCP_SendReceive6KUDPCmd", a.SendReceive6KUDPCmd, missing);
    BindOne(h, "ADAMTCP_Read6KAI",            a.Read6KAI,            missing);
    BindOne(h, "ADAMTCP_GetHostIdleTime",     a.GetHostIdleTime,     missing);
    if (!TableComplete(a))
    {
        ::FreeLibrary(h);
        Info() += "; " + path + " loaded but missing export(s) " + missing + " -- released";
        return false;
    }
    g_api  = a;
    g_hDll = h;
    Info() = "bound " + path;
    return true;
}
#endif // _WIN32 && W906_NO_SOFT_SIMULTE

void ResolveOnce()
{
    if (g_resolved)
        return;
    g_resolved = true;
    g_bound    = false;
    g_api      = AdamTcpApi_St02();
    Info()     = "ADAMTCP.dll not bound:";
#if !defined(W906_NO_SOFT_SIMULTE)
    Info() += " SIMULATION build (SOFT_SIMULTE) -- the vendor DLL is never loaded ([W906] S1)";
#elif !defined(_WIN32)
    Info() += " not a Windows build";
#else
    if (!W906_AdamEpLive())                                                     // [W906] S5 (H4 integration pass 20261002): EP live switch OFF
    {
        Info() += " EP live switch OFF (MachineType.h W906_ADAM_EP_LIVE not defined) -- the vendor DLL is never loaded ([W906] S5)";
    }
    else if (EnvContains("W906_GENERAL_INI_PATH", "general_ini_scratch"))
    {
        Info() += " ctest environment -- the vendor DLL is never loaded ([W906] S2)";
    }
    else
    {
        const char* env = std::getenv("W906_ADAMTCP_DLL");
        if (env != 0 && *env != 0)
        {
            Info() += " W906_ADAMTCP_DLL set";
            g_bound = TryPath(env);
        }
        else
        {
            char exe[MAX_PATH];
            const DWORD n = ::GetModuleFileNameA(0, exe, MAX_PATH);
            if (n > 0 && n < MAX_PATH)
            {
                std::string dir(exe, n);
                const std::string::size_type s = dir.find_last_of("/\\");
                if (s != std::string::npos)
                    g_bound = TryPath(dir.substr(0, s + 1) + "ADAMTCP.dll");
            }
            if (!g_bound)
                g_bound = TryPath("D:\\HT9045\\EXE\\ADAMTCP.dll");
        }
    }
#endif
    std::printf("  [ADAMTCP] %s%s\n", Info().c_str(),
                g_bound ? "" : " -- every ADAMTCP_* call returns ADAMTCP_StartupFailure (-1)");
}

inline bool Ready()
{
    ResolveOnce();
    return g_bound;
}

} // anonymous namespace

// ---- the 11 wrappers ------------------------------------------------------------------------------------------
int ADAMTCP_Open(void)
{
    if (!Ready())
        return ADAMTCP_StartupFailure;
    const int iRet = g_api.Open();
    if (iRet == ADAMTCP_NoError)
        ++g_openRef;                                                            // [W906] S3
    return iRet;
}

void ADAMTCP_Close(void)
{
    if (!Ready())
        return;
    if (g_openRef <= 0)                                                         // [W906] S3: no Open outstanding ->
    {                                                                           //   forwarding would be an unmatched
        ++g_absorbed;                                                           //   WSACleanup on the whole process
        return;
    }
    --g_openRef;
    g_api.Close();
}

int ADAMTCP_Connect(const char* szIP, unsigned short port,
                    int iConnectionTimeout, int iSendTimeout, int iReceiveTimeout)
{
    return Ready() ? g_api.Connect(const_cast<char*>(szIP), port, iConnectionTimeout, iSendTimeout, iReceiveTimeout)
                   : ADAMTCP_StartupFailure;                                    // [W906] S4 const_cast
}

void ADAMTCP_Disconnect(void)
{
    if (Ready())
        g_api.Disconnect();
}

int ADAMTCP_ReadReg(const char* szIP, unsigned short wIDAddr, unsigned short wStartAddress,
                    unsigned short wCount, unsigned short wData[])
{
    return Ready() ? g_api.ReadReg(const_cast<char*>(szIP), wIDAddr, wStartAddress, wCount, wData)
                   : ADAMTCP_StartupFailure;
}

int ADAMTCP_WriteReg(const char* szIP, unsigned short wIDAddr, unsigned short wStartAddress,
                     unsigned short wCount, unsigned short wData[])
{
    return Ready() ? g_api.WriteReg(const_cast<char*>(szIP), wIDAddr, wStartAddress, wCount, wData)
                   : ADAMTCP_StartupFailure;
}

int ADAMTCP_UDPOpen(int iSendTimeout, int iReceiveTimeout)
{
    return Ready() ? g_api.UDPOpen(iSendTimeout, iReceiveTimeout) : ADAMTCP_StartupFailure;
}

int ADAMTCP_UDPClose(void)
{
    return Ready() ? g_api.UDPClose() : ADAMTCP_StartupFailure;
}

int ADAMTCP_SendReceive6KUDPCmd(const char* szIP, const char* szSend, char* szReceive)
{
    return Ready() ? g_api.SendReceive6KUDPCmd(const_cast<char*>(szIP), const_cast<char*>(szSend), szReceive)
                   : ADAMTCP_StartupFailure;
}

int ADAMTCP_Read6KAI(const char* szIP, unsigned short wModule, unsigned short wIDAddr,
                     unsigned short wGain[], unsigned short wHex[], double dlValue[])
{
    return Ready() ? g_api.Read6KAI(const_cast<char*>(szIP), wModule, wIDAddr, wGain, wHex, dlValue)
                   : ADAMTCP_StartupFailure;
}

int ADAMTCP_GetHostIdleTime(const char* ip, int* hostIdleTime)
{
    return Ready() ? g_api.GetHostIdleTime(const_cast<char*>(ip), hostIdleTime) : ADAMTCP_StartupFailure;
}

// ---- seam / observability -------------------------------------------------------------------------------------
void AdamTcp_St02_InstallApiForTest(const AdamTcpApi_St02* api)
{
#if defined(_WIN32)
    if (g_hDll != 0)
    {
        ::FreeLibrary(g_hDll);
        g_hDll = 0;
    }
#endif
    g_api      = AdamTcpApi_St02();
    g_bound    = false;
    g_openRef  = 0;
    g_absorbed = 0;
    if (api == 0)
    {
        g_resolved = false;                                                     // the next call resolves again
        Info()     = "not resolved yet";
        return;
    }
    g_resolved = true;
    if (TableComplete(*api))
    {
        g_api   = *api;
        g_bound = true;
        Info()  = "fake table (test)";
    }
    else
    {
        Info() = "fake table (test) with a null entry -- not bound";
    }
}

bool AdamTcp_St02_IsBound()
{
    return Ready();
}

const char* AdamTcp_St02_BindInfo()
{
    ResolveOnce();
    return Info().c_str();
}

int AdamTcp_St02_OpenRefCount()
{
    return g_openRef;
}

int AdamTcp_St02_AbsorbedCloseCount()
{
    return g_absorbed;
}
