// ===========================================================================
//  Public/ApaxShim_St02.h  --  AI(W906-ST02-ADAM) 20261002 (St02-E helper H3)
//
//  Run-time binding of the Advantech APAX Modbus library (ADSMOD.dll) for the
//  ONLY golden code that uses it: Open_APAX (golden 912 adam6024.cpp:2219-2267),
//  which calls four entry points.  Same rule as ADAMTCP.dll (RULINGS_20260929
//  section 5 item 6 = A): LoadLibrary at run time, the tree builds without the
//  DLL, a missing DLL takes golden's own error path.  Precedents:
//  Public/HTKeyProShim.cpp and the laptop's Public/AdamTcpShim.cpp (75756cab).
//
//  WHY NOT THE VENDOR HEADER / LIBRARY
//    * golden links Public\ADSMOD_BCB.lib (golden 912 HT9045.bpr), a Borland OMF
//      import library MinGW cannot link; golden's exe therefore imports
//      ADSMOD.dll statically.  The port must NOT (the exe would not start on a
//      PC without the DLL): St02-E checks `objdump -p` of every exe for
//      "DLL Name: ADSMOD.dll" -> must be absent.
//    * golden Public/ADSMOD.h (242 lines) is a vendor header on the do-not-modify
//      list and is NOT copied.  Only what Open_APAX uses is declared below,
//      with the vendor argument lists (golden 912 Public/ADSMOD.h:71-76 / :77 /
//      :134-141 / :158 / :160).
//
//  MEASURED ON D:\HT9045\EXE\ADSMOD.dll (objdump -p / -d, 20261002, never loaded)
//    * PE32 i386 (pei-i386), ImageBase 0x10000000, 54 exports, names
//      UNDECORATED (MOD_Initialize ... MOD_WaitTcpServerClientDisconnectEvent);
//      imports KERNEL32 (CreateThread, SetThreadPriority, ...) and WS2_32 only.
//    * CALLING CONVENTION = __cdecl, not stdcall: the export thunks end in a
//      plain `ret` (MOD_AddTcpClientConnect @0x10001ae0 forwards 8 args and
//      returns with `ret`, no `ret 0x20`).  ADSMOD.h's ADS_API is empty, so the
//      vendor prototypes are cdecl too.  (ADAMTCP.dll is the opposite: CALLBACK
//      = __stdcall.  Do not copy one shim's convention into the other.)
//    * The CALLBACKS are __cdecl as well: the client worker calls the connect
//      handler with 3 pushes followed by `add $0xc,%esp` (@0x10002f67), the
//      write-completed handlers with 2 pushes + `add $0x8,%esp` (@0x100026ce).
//    * MOD_Initialize (@0x10001270) = WSAStartup(0x0202): returns 0 or
//      0x321 = 801 = MODERR_WSASTART_FAILED.  MOD_StartTcpClient (@0x10001b90)
//      returns 815 (MODERR_THREAD_RUNNING) when already started, otherwise
//      CreateThread -> the connect / disconnect handlers run ON THE DLL'S
//      WORKER THREAD, not on the caller's thread (golden behaves the same).
//
//  RESOLUTION (Public/ApaxShim_St02.cpp), once, lazily, on the first wrapper
//  call -- never at static initialisation.  The SAME order and the same S1 / S2
//  rules as H1's Public/AdamTcp_St02.h (RESOLUTION 0-4), so the two vendor
//  DLLs of the ADAM port behave alike:
//    0. a table installed with ApaxMod_InstallApiForTest() wins; the DLL is
//       then never touched (ctest);
//    1. [W906] S1 SIMULATION build (W906_NO_SOFT_SIMULTE not defined =>
//       MachineType.h defines SOFT_SIMULTE): the DLL is NEVER loaded;
//    2. [W906] S2 inside ctest (W906_GENERAL_INI_PATH contains
//       "general_ini_scratch", tests/CMakeLists.txt _w906_env_all_tests): never
//       loaded either, whatever else is set;
//    3. W906_ADSMOD_DLL set (non-empty): that path is the only candidate;
//    4. otherwise <folder of the running exe>\ADSMOD.dll, then
//       D:\HT9045\EXE\ADSMOD.dll (the BCB6 drop, where golden's DLL lives).
//    All 4 exports must resolve, or the DLL is released and the binding counts
//    as missing (all-or-nothing, the same rule as AdamTcp_St02).
//
//  MISSING DLL / MISSING EXPORT / SIM BUILD / CTEST WITHOUT A TABLE / 64-BIT
//  PROCESS (the PE32 DLL cannot load):
//    [W906] every wrapper returns MODERR_WSASTART_FAILED (801) -- the value the
//    real MOD_Initialize itself returns when its start-up fails -- so golden's
//    own branches decide (Open_APAX: MOD_Initialize != SUCCESS -> skips the
//    connect and still returns true, golden :2239/:2260).
//
//  THREADING: resolution is not mutex-guarded; golden's only caller would run
//    on the bring-up / tick thread.  (Today: golden has NO caller of Open_APAX
//    at all -- see Adam6024Apax_St02.cpp.)
//
//  Nothing in this file opens a socket; only the DLL does, and only when a
//  wrapper is called.
// ===========================================================================
#ifndef APAXSHIM_ST02_H
#define APAXSHIM_ST02_H

#if defined(_WIN32)
#  define APAXMOD_CALL __cdecl              // measured: ADSMOD.dll exports are cdecl (see banner)
#else
#  define APAXMOD_CALL
#endif

// ---- the vendor constants golden adam6024.cpp uses (values: golden 912 Public/ADSMOD.h) ----
#define MODERR_SUCCESS          0                       // ADSMOD.h:26
#define MODERR_BASE             800                     // ADSMOD.h:27
#define MODERR_WSASTART_FAILED  (MODERR_BASE + 1)       // ADSMOD.h:30  (this shim's "not bound" answer, [W906])

// ---- the callback shapes (golden 912 Public/ADSMOD.h:71-72 / :76), default (= cdecl) convention ----
typedef void (*OnConnectTcpServerCompletedEvent)(long i_lResult, char *i_szIp, void *i_Param);      // ADSMOD.h:71
typedef void (*OnDisconnectTcpServerCompletedEvent)(long i_lResult, char *i_szIp, void *i_Param);   // ADSMOD.h:72
typedef void (*OnModbusWriteCompletedEvent)(long i_lResult, void *i_Param);                         // ADSMOD.h:76 (ClientWriteReg_1_8_Handler's shape)

// ---- the 4 wrappers (vendor names, so Open_APAX reads like golden; LONG = long) ----
long MOD_Initialize();                                                          // ADSMOD.h:77
long MOD_AddTcpClientConnect(char *i_szServerIp,
                             int i_iScanInterval,
                             int i_iConnectTimeout,
                             int i_iTransactTimeout,
                             OnConnectTcpServerCompletedEvent connEvtHandle,
                             OnDisconnectTcpServerCompletedEvent DisconnEvtHandle,
                             void *i_Param,
                             unsigned long *o_ulClientHandle);                  // ADSMOD.h:134-141
long MOD_StartTcpClient();                                                      // ADSMOD.h:158
long MOD_SetTcpClientPriority(int iPriority);                                   // ADSMOD.h:160

// ---- the DLL ABI, as a table (also the ctest seam) --------------------------
struct ApaxModApi
{
    long (APAXMOD_CALL *Initialize)();
    long (APAXMOD_CALL *AddTcpClientConnect)(char *i_szServerIp, int i_iScanInterval, int i_iConnectTimeout,
                                             int i_iTransactTimeout, OnConnectTcpServerCompletedEvent connEvtHandle,
                                             OnDisconnectTcpServerCompletedEvent DisconnEvtHandle, void *i_Param,
                                             unsigned long *o_ulClientHandle);
    long (APAXMOD_CALL *StartTcpClient)();
    long (APAXMOD_CALL *SetTcpClientPriority)(int iPriority);
};

//  ctest seam.  api != 0: copy the table and use it; the DLL is never loaded (a
//  table with ANY null entry counts as "not bound").  api == 0: forget
//  everything (a bound DLL is released) so the next wrapper call resolves again.
void ApaxMod_InstallApiForTest(const ApaxModApi* api);

//  Observability (boot summary / tests): resolves on first use like a wrapper call.
bool        ApaxMod_IsBound();      // true = all 4 entry points available
const char* ApaxMod_BindInfo();     // "bound <path>" / "fake table (test)" / why nothing is bound

#endif // APAXSHIM_ST02_H
