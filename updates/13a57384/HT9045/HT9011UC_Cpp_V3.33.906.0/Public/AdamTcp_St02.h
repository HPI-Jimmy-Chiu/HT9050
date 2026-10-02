// =============================================================================
//  Public/AdamTcp_St02.h -- the ADAMTCP.dll (Advantech ADAM-5000/6000 TCP) run-time shim.
//
//  AI(W906-ST02-ADAM) 20261002 (St02-E helper H1).  RULINGS_20260929 section 5 item 6 = A: LoadLibrary("ADAMTCP.dll")
//  at RUN TIME (the exe's folder, then D:\HT9045\EXE), the tree builds without the DLL, a missing DLL takes golden's
//  own error path / message.  Precedent: Public/HTKeyProShim.{h,cpp}.  Successor of the laptop's held-back
//  Public/AdamTcpShim.{h,cpp} (git show 75756cab; held back by 3c348627) -- what changed and why is listed below.
//
//  WHY A SHIM AND NOT THE VENDOR FILES
//    golden links ADAMTCPbc.lib (HT9045.bpr), a Borland OMF import library MinGW cannot link, and the vendor header
//    golden 912 ADAMTCP.h is not copied.  Only the entry points golden calls are declared, with the vendor argument
//    list (ADAMTCP.h:154-265); `char szIP[]` becomes `const char*` (vclcompat AnsiString::c_str() is const, BCB6's
//    was not; the DLL does not write through the IP / command strings).  WORD is spelled `unsigned short` (the same
//    type; keeps <windows.h> out of this header).
//
//  THE 11 ENTRY POINTS (every golden 912 call site; grep 'ADAMTCP_[A-Za-z0-9]*(' over the tree, 20261002 11:2x)
//      ADAMTCP_Open                 adam6024.cpp:278
//      ADAMTCP_Close                adam6024.cpp:426 :2996
//      ADAMTCP_Connect              adam6024.cpp:359 :2991
//      ADAMTCP_Disconnect           adam6024.cpp:425 :2990
//      ADAMTCP_ReadReg              HS_Function.cpp:1166 (Multi EP pressure read; not adam6024.cpp)
//      ADAMTCP_WriteReg             adam6024.cpp:1876 :1994 :2012 :2016 :2020 :2021 (H1), :2416 :2439 :2577 :2646 (H3 APAX)
//      ADAMTCP_UDPOpen              adam6024.cpp:290 :2771 :2796 :2823 :2851 :2875
//      ADAMTCP_UDPClose             adam6024.cpp:355 :2783 :2808 :2838 :2861 :2882
//      ADAMTCP_SendReceive6KUDPCmd  adam6024.cpp:149 :174 :202 :227 :2773 :2798 :2825 :2853
//      ADAMTCP_Read6KAI             adam6024.cpp:463
//      ADAMTCP_GetHostIdleTime      adam6024.cpp:2877 (ADAMTCP_SetHostIdleTime :2898 is commented out in golden: not here)
//    All 11 are exported by D:\HT9045\EXE\ADAMTCP.dll under their undecorated names (objdump -p, 20261002: PE32 i386,
//    imports WS2_32 / KERNEL32 / USER32) and are __stdcall: the stack bytes each `ret` pops match the vendor argument
//    lists (Connect / WriteReg ret 0x14, Read6KAI 0x18, SendReceive6KUDPCmd 0xc, UDPOpen / GetHostIdleTime 8, the
//    0-argument ones plain ret; objdump -d, a static read -- the DLL was never loaded).
//
//  RESOLUTION (AdamTcp_St02.cpp), lazily on the FIRST wrapper call -- never at static initialisation:
//    0. a table installed with AdamTcp_St02_InstallApiForTest() wins; the DLL is then never touched (ctest);
//    1. SIMULATION build (W906_NO_SOFT_SIMULTE not defined => MachineType.h:63-64 defines SOFT_SIMULTE): the real DLL
//       is NEVER loaded ([W906] S1 below);
//    2. inside ctest (W906_GENERAL_INI_PATH contains "general_ini_scratch", tests/CMakeLists.txt _w906_env_all_tests):
//       the real DLL is never loaded either, whatever else is set ([W906] S2);
//    2b. SHIP build with the EP live switch OFF (W906_AdamEpLive()==false: MachineType.h W906_ADAM_EP_LIVE not defined,
//       Adam6024Integrate_St02.cpp): the real DLL is never loaded ([W906] S5, (H4 integration pass 20261002)).
//    3. W906_ADAMTCP_DLL set (non-empty): that path is the only candidate;
//    4. otherwise <folder of the running exe>\ADAMTCP.dll, then D:\HT9045\EXE\ADAMTCP.dll.
//    All 11 exports must resolve or the DLL is released and nothing is bound (a half-bound transport that can
//    Connect but not Disconnect is worse than none).  A 64-bit process cannot load the PE32 DLL: not bound.
//
//  NOT BOUND (missing DLL / missing export / SIM build / ctest without a table):
//    every int wrapper returns ADAMTCP_StartupFailure (-1), the two void wrappers do nothing, nothing is written
//    through the out-parameters.  golden then takes its own path, e.g. Open_ADAM_6024 :278-284
//    MyDBIProcess("Motion", "ADAMTCP_Open Fail!", "ADAM5KTCP_StartupFailure (-1)") + return false, and for Num 1 / 2
//    (no ADAMTCP_Open) the Connect failure :361-371 -> ShowMyMessage("Connect Fail! Please Check ADAM IP!", IP,
//    "ADAM5KTCP_StartupFailure (-1)") once iCount passes 100.
//
//  [W906] DEVIATIONS (all in AdamTcp_St02.cpp)
//    S1 SIM builds never load the vendor DLL.  Steven 20260918 (MachineType.h:47-50): SOFT_SIMULTE = software
//       simulation only.  golden's SIM arms already skip the module (Open_ADAM_6024 :395-396 returns true without
//       I/O, fCheck* :2912 / :2932 / :2962), but four write paths are not SIM-gated in golden (ADAM_DirectWriteData
//       :1994-2021, ADAM_WriteVoltage :1876, APAX_WriteData) -- with the DLL they would call ADAMTCP_WriteReg on a
//       PC that may sit on the machine LAN.  Not bound = the result golden gets on a PC without the module (a
//       non-zero return -> bConnectStatus[Num]=false, no message).
//    S2 the ctest refusal: no ctest ever loads the vendor DLL, even one that forgot to install a table.
//    S3 ADAMTCP_Close REFCOUNT -- the hold-back 3c348627's "ADAMTCP_Close's unconditional WSACleanup would reset every
//       socket of wb_serve".  Measured in the DLL (objdump -d, static): ADAMTCP_Open (RVA 0x1000) = WSAStartup(2.2)
//       through its global object; ADAMTCP_Close (RVA 0x1040) = connection count := 0 + WSACleanup, unconditionally.
//       golden calls Close more often than Open: fCheckConnectStatus_ADAM6024 :2996 on a failed Connect (possibly
//       BEFORE any Open -- Open_ADAM_6024 checks the status at :264, ahead of ADAMTCP_Open at :278) and
//       Close_ADAM_6024 :426.  Each unmatched WSACleanup takes one count off the PROCESS's Winsock reference (wb_serve
//       :8045 / :8046, every vclcompat socket); at zero Windows closes all of them.  The shim therefore forwards a
//       Close only while a successful Open (return 0) is outstanding, else it absorbs it (counted:
//       AdamTcp_St02_AbsorbedCloseCount).  An absorbed Close loses nothing else: in golden every Close follows an
//       ADAMTCP_Disconnect (:425 / :2990), and Disconnect (RVA 0x11c0) already deletes every connection and sets the
//       same count to 0.
//    S4 `const char*` for the vendor `char szIP[]` / `char szSend[]` (see above); WORD spelled unsigned short.
//
//  CHANGED FROM THE LAPTOP'S AdamTcpShim (75756cab) AND WHY
//    * S3 the Close refcount (the hold-back's WSACleanup finding; the laptop forwarded every Close);
//    * S1 / S2 (the laptop loaded the DLL in SIM builds and relied on W906_ADAMTCP_DLL alone in ctest);
//    * + ADAMTCP_ReadReg (golden 912 HS_Function.cpp:1166 -- the laptop's 10 came from 906 adam6024.cpp only);
//    * the full vendor return-code list (ADAMTCP.h:127-143) for the TfAdam6024 error table and its tests;
//    * new names (_St02) so the held-back file can come back without a collision; the design (lazy resolution,
//      all-or-nothing, SetErrorMode around LoadLibrary, -1 fallback, test table) is the laptop's.
//    The rest of the hold-back (Timer2 production set-point writer, FormClose zeroing, ADAM_ReturnValueCheck pump) is
//    integration work (H4), not this file.
//
//  THREADING: no lock.  Every golden caller runs on the single tick / bring-up thread (same assumption as
//    HTKeyProShim.cpp); the refcount and the resolution state are plain globals.
// =============================================================================
#ifndef ADAMTCP_ST02_H
#define ADAMTCP_ST02_H

#if defined(_WIN32)
#  define ADAMTCP_ST02_CALL __stdcall            // vendor ADAMTCP.h: EXPORTS ... CALLBACK (= __stdcall)
#else
#  define ADAMTCP_ST02_CALL
#endif

// ---- vendor constants golden uses (values from golden 912 ADAMTCP.h; #ifndef: never redefine a vendor macro) ----
#ifndef ADAMTCP_BI_10V
#define ADAMTCP_BI_10V              0           // ADAMTCP.h:96   (golden adam6024.cpp:380)
#endif
#ifndef ADAMTCP_UNI_4TO20mA
#define ADAMTCP_UNI_4TO20mA         9           // ADAMTCP.h:105  (golden adam6024.cpp:384-386)
#endif
#ifndef ADAMTCP_NoError
#define ADAMTCP_NoError             0           // ADAMTCP.h:127  (golden adam6024.cpp:2877)
#define ADAMTCP_StartupFailure      (-1)        // ADAMTCP.h:128  (this shim's "not bound" answer)
#define ADAMTCP_SocketFailure       (-2)        // ADAMTCP.h:129
#define ADAMTCP_UdpSocketFailure    (-3)        // ADAMTCP.h:130
#define ADAMTCP_SetTimeoutFailure   (-4)        // ADAMTCP.h:131
#define ADAMTCP_SendFailure         (-5)        // ADAMTCP.h:132
#define ADAMTCP_ReceiveFailure      (-6)        // ADAMTCP.h:133
#define ADAMTCP_ExceedMaxFailure    (-7)        // ADAMTCP.h:134
#define ADAMTCP_CreateWsaEventFailure (-8)      // ADAMTCP.h:135
#define ADAMTCP_ReadStreamDataFailure (-9)      // ADAMTCP.h:136
#define ADAMTCP_InvalidIP           (-10)       // ADAMTCP.h:137
#define ADAMTCP_ThisIPNotConnected  (-11)       // ADAMTCP.h:138
#define ADAMTCP_AlarmInfoEmpty      (-12)       // ADAMTCP.h:139
#define ADAMTCP_NotSupportModule    (-13)       // ADAMTCP.h:140
#define ADAMTCP_ExceedDONo          (-14)       // ADAMTCP.h:141
#define ADAMTCP_InvalidRange        (-15)       // ADAMTCP.h:142
#define ADAMTCP_EventError          (-100)      // ADAMTCP.h:143
#endif

// ---- the 11 wrappers: the vendor names, so golden lines read word for word (C++ linkage: never the DLL's symbols) ----
int  ADAMTCP_Open(void);                                                        // ADAMTCP.h:154
void ADAMTCP_Close(void);                                                       // ADAMTCP.h:155  ([W906] S3 refcount)
int  ADAMTCP_Connect(const char* szIP, unsigned short port,
                     int iConnectionTimeout, int iSendTimeout,
                     int iReceiveTimeout);                                      // ADAMTCP.h:156-158
void ADAMTCP_Disconnect(void);                                                  // ADAMTCP.h:159
int  ADAMTCP_ReadReg(const char* szIP, unsigned short wIDAddr,
                     unsigned short wStartAddress, unsigned short wCount,
                     unsigned short wData[]);                                   // ADAMTCP.h:162
int  ADAMTCP_WriteReg(const char* szIP, unsigned short wIDAddr,
                      unsigned short wStartAddress, unsigned short wCount,
                      unsigned short wData[]);                                  // ADAMTCP.h:163
int  ADAMTCP_UDPOpen(int iSendTimeout, int iReceiveTimeout);                    // ADAMTCP.h:180
int  ADAMTCP_UDPClose(void);                                                    // ADAMTCP.h:181
int  ADAMTCP_SendReceive6KUDPCmd(const char* szIP, const char* szSend,
                                 char* szReceive);                              // ADAMTCP.h:183
int  ADAMTCP_Read6KAI(const char* szIP, unsigned short wModule,
                      unsigned short wIDAddr, unsigned short wGain[],
                      unsigned short wHex[], double dlValue[]);                 // ADAMTCP.h:188-189
int  ADAMTCP_GetHostIdleTime(const char* ip, int* hostIdleTime);                // ADAMTCP.h:264

// ---- the DLL ABI as a table (also the ctest seam): exactly the vendor prototypes, __stdcall, non-const char* ----
struct AdamTcpApi_St02
{
    int  (ADAMTCP_ST02_CALL *Open)(void);
    void (ADAMTCP_ST02_CALL *Close)(void);
    int  (ADAMTCP_ST02_CALL *Connect)(char* szIP, unsigned short port, int iConnectionTimeout,
                                      int iSendTimeout, int iReceiveTimeout);
    void (ADAMTCP_ST02_CALL *Disconnect)(void);
    int  (ADAMTCP_ST02_CALL *ReadReg)(char* szIP, unsigned short wIDAddr, unsigned short wStartAddress,
                                      unsigned short wCount, unsigned short* wData);
    int  (ADAMTCP_ST02_CALL *WriteReg)(char* szIP, unsigned short wIDAddr, unsigned short wStartAddress,
                                       unsigned short wCount, unsigned short* wData);
    int  (ADAMTCP_ST02_CALL *UDPOpen)(int iSendTimeout, int iReceiveTimeout);
    int  (ADAMTCP_ST02_CALL *UDPClose)(void);
    int  (ADAMTCP_ST02_CALL *SendReceive6KUDPCmd)(char* szIP, char* szSend, char* szReceive);
    int  (ADAMTCP_ST02_CALL *Read6KAI)(char* szIP, unsigned short wModule, unsigned short wIDAddr,
                                       unsigned short* wGain, unsigned short* wHex, double* dlValue);
    int  (ADAMTCP_ST02_CALL *GetHostIdleTime)(char* ip, int* hostIdleTime);
};

//  ctest seam.  api != 0: copy the table and use it, the DLL is never loaded (a table with ANY null entry counts as
//  "not bound", the same all-or-nothing rule as the DLL).  api == 0: forget everything (a bound DLL is released)
//  so the next wrapper call resolves again.  Both reset the Close refcount and the absorbed-Close counter.
void AdamTcp_St02_InstallApiForTest(const AdamTcpApi_St02* api);

//  Observability (boot summary / tests).  IsBound / BindInfo resolve on first use like a wrapper call.
bool        AdamTcp_St02_IsBound();               // true = all 11 entry points available (DLL or test table)
const char* AdamTcp_St02_BindInfo();              // "bound <path>" / "fake table (test)" / why nothing is bound
int         AdamTcp_St02_OpenRefCount();          // [W906] S3: successful Opens not yet matched by a forwarded Close
int         AdamTcp_St02_AbsorbedCloseCount();    // [W906] S3: Closes absorbed because no Open was outstanding

#endif // ADAMTCP_ST02_H
