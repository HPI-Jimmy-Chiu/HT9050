// ===========================================================================
//  Public/ApaxShim_St02.cpp  --  AI(W906-ST02-ADAM) 20261002 (St02-E helper H3)
//  Run-time binding of ADSMOD.dll (see ApaxShim_St02.h for the why, the
//  measured ABI, the search order with the [W906] S1 / S2 rules copied from
//  H1's Public/AdamTcp_St02.cpp, the all-or-nothing rule and the 801
//  fallback).
//
//  Deliberately does NOT link ADSMOD_BCB.lib (Borland OMF) and does NOT copy
//  the vendor header.  Nothing here opens a socket: only the DLL does, and
//  only when golden Open_APAX calls a wrapper (golden has no caller of it).
// ===========================================================================
#include "ApaxShim_St02.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>

#if defined(_WIN32)
#  include <windows.h>
#endif

namespace {

ApaxModApi  g_api = ApaxModApi();   // all entries null until bound
bool        g_resolved = false;     // resolution attempted (DLL) or table installed (test)
bool        g_bound    = false;     // all 4 entries usable
std::string& Info()                 // function-local: no static-initialisation-order dependency
{
    static std::string s("not resolved yet");
    return s;
}
#if defined(_WIN32)
HMODULE     g_hDll     = 0;         // kept for the process lifetime, like golden's static import
#endif

bool TableComplete(const ApaxModApi& a)
{
    return a.Initialize && a.AddTcpClientConnect && a.StartTcpClient && a.SetTcpClientPriority;
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
    FARPROC p = ::GetProcAddress(h, name);     // undecorated names (measured, see the header banner)
    slot = reinterpret_cast<F>(p);
    if (p == 0)
    {
        if (!missing.empty()) missing += ",";
        missing += name;
    }
}

// Try one candidate path; true = all 4 bound (g_api filled, g_hDll kept).
bool TryPath(std::string path)
{
    for (std::string::size_type i = 0; i < path.size(); ++i)   // LoadLibrary wants backslashes (a CMake path has slashes)
        if (path[i] == '/') path[i] = char(92);
    // No "cannot find" / "not a valid image" box from the loader (restored right after).
    UINT oldMode = ::SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOOPENFILEERRORBOX);
    HMODULE h = ::LoadLibraryA(path.c_str());
    DWORD err = ::GetLastError();
    ::SetErrorMode(oldMode);
    if (h == 0)
    {
        char buf[64];
        std::snprintf(buf, sizeof(buf), " (LoadLibrary error %lu)", static_cast<unsigned long>(err));
        Info() += "; " + path + buf;
        return false;
    }
    ApaxModApi a = ApaxModApi();
    std::string missing;
    BindOne(h, "MOD_Initialize",           a.Initialize,           missing);
    BindOne(h, "MOD_AddTcpClientConnect",  a.AddTcpClientConnect,  missing);
    BindOne(h, "MOD_StartTcpClient",       a.StartTcpClient,       missing);
    BindOne(h, "MOD_SetTcpClientPriority", a.SetTcpClientPriority, missing);
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
    g_api      = ApaxModApi();
    Info()     = "ADSMOD.dll not bound:";
#if !defined(W906_NO_SOFT_SIMULTE)
    Info() += " SIMULATION build (SOFT_SIMULTE) -- the vendor DLL is never loaded ([W906] S1)";
#elif !defined(_WIN32)
    Info() += " not a Windows build";
#else
    if (EnvContains("W906_GENERAL_INI_PATH", "general_ini_scratch"))
    {
        Info() += " ctest environment -- the vendor DLL is never loaded ([W906] S2)";
    }
    else
    {
        const char* env = std::getenv("W906_ADSMOD_DLL");
        if (env && *env)
        {
            Info() += " W906_ADSMOD_DLL set";
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
                    g_bound = TryPath(dir.substr(0, s + 1) + "ADSMOD.dll");
            }
            if (!g_bound)
                g_bound = TryPath("D:\\HT9045\\EXE\\ADSMOD.dll");
        }
    }
#endif
    std::printf("  [ADSMOD] %s%s\n", Info().c_str(),
                g_bound ? "" : " -- every MOD_* call returns MODERR_WSASTART_FAILED (801)");
}

inline bool Ready()
{
    ResolveOnce();
    return g_bound;
}

} // anonymous namespace

// ---- the 4 wrappers ---------------------------------------------------------
long MOD_Initialize()
{
    return Ready() ? g_api.Initialize() : MODERR_WSASTART_FAILED;
}

long MOD_AddTcpClientConnect(char *i_szServerIp,
                             int i_iScanInterval,
                             int i_iConnectTimeout,
                             int i_iTransactTimeout,
                             OnConnectTcpServerCompletedEvent connEvtHandle,
                             OnDisconnectTcpServerCompletedEvent DisconnEvtHandle,
                             void *i_Param,
                             unsigned long *o_ulClientHandle)
{
    return Ready() ? g_api.AddTcpClientConnect(i_szServerIp, i_iScanInterval, i_iConnectTimeout, i_iTransactTimeout,
                                               connEvtHandle, DisconnEvtHandle, i_Param, o_ulClientHandle)
                   : MODERR_WSASTART_FAILED;
}

long MOD_StartTcpClient()
{
    return Ready() ? g_api.StartTcpClient() : MODERR_WSASTART_FAILED;
}

long MOD_SetTcpClientPriority(int iPriority)
{
    return Ready() ? g_api.SetTcpClientPriority(iPriority) : MODERR_WSASTART_FAILED;
}

// ---- seam / observability ---------------------------------------------------
void ApaxMod_InstallApiForTest(const ApaxModApi* api)
{
#if defined(_WIN32)
    if (g_hDll != 0)
    {
        ::FreeLibrary(g_hDll);
        g_hDll = 0;
    }
#endif
    g_api   = ApaxModApi();
    g_bound = false;
    if (api == 0)
    {
        g_resolved = false;                 // next call resolves again
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

bool ApaxMod_IsBound()
{
    return Ready();
}

const char* ApaxMod_BindInfo()
{
    ResolveOnce();
    return Info().c_str();
}
