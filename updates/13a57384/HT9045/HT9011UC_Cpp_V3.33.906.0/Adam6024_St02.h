// =============================================================================
//  Adam6024_St02.h -- the shared header of the ADAM-6024 port (golden 912 adam6024.h).
//
//  AI(W906-ST02-ADAM) 20261002 (St02-E helper H1).  Card ST02-ADAM (Steven 1002 ~10:5x: "Adam6024's code is not
//  written yet, port it").  Source = golden 912 D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\adam6024.h (92
//  lines) + adam6024.cpp (3104 lines), cp950.  Every declaration below carries its golden 912 adam6024.h line.
//
//  WHAT IS HERE
//    1. every golden 912 adam6024.h declaration, golden's exact signature and default arguments, in golden's order,
//       EXCEPT
//         - TransformFuntion (golden :29)            -> stays in adam6024.h (the laptop's file, body adam6024.cpp);
//         - EpSwitch / EPSwitchOnOff / enum EPSwOn   -> stay in atester_shims.h:281-282 (bodies at the end of
//           (golden :46-52)                             adam6024.cpp).  This header does NOT include atester_shims.h.
//    2. the TfAdam6024 facade (golden :63-88) and `extern TfAdam6024 *fAdam6024` (golden :89);
//    3. `WORD TransFuntion(double v)` (golden :90) -- DECLARATION ONLY: no body exists in golden 912, 906_0625_Steven
//       or 899 (grep -rn TransFuntion over the three trees, 20261002 11:1x: the header line only).  Nobody calls it;
//       a call would fail to link exactly as it would in golden;
//    4. under `#ifdef ADAM6024_ST02_INTERNAL` (defined ONLY by the Adam6024*_St02.cpp files and their ctests before
//       the include): the golden adam6024.cpp file-scope globals and file-scope functions that the split into
//       several translation units makes cross-TU.  Golden defines them non-static in ONE file; the port keeps them
//       non-static, one definition each, owner below.  Callers outside the ADAM port never see these names (Address,
//       fValue, wGain ... are generic -- a caller's own file-scope `fValue` would otherwise collide).
//
//  [W906] DEVIATIONS IN THIS HEADER (each also at its line)
//    D1  TfAdam6024 is a plain facade class, not `: public TForm` (no VCL forms in the port; precedent: TfAutomation,
//        Automation/automation.h:237).  __published becomes public.
//    D2  `__fastcall` dropped on the ctor and the three event handlers (precedent forms/fCounterClear.h:198 /
//        fSecurity.h:542; the declaration and the definition are both in H1's files, so they cannot drift apart --
//        vclcompat/vcl_compat.h:24-48 explains why a mismatch would be a silent link-time ABI trap).
//    D3  a destructor is added: it runs golden's OnDestroy (FormDestroy, adam6024.dfm:15) and deletes ClientSocket1
//        (golden: the VCL owner frees the .dfm component).
//    D4  int GetModuleHostIdleTime(int Num) is ALSO declared: golden 912 adam6024.h:41 declares `()` but the only
//        body is `(int Num)` (adam6024.cpp:2867).  Both overloads are declared; only `(int Num)` has a body (H1), so
//        a call to `()` fails to link exactly as it would in golden (golden has no caller of either).
//
//  STAND-IN CONFLICTS (atester_shims.h:265-269, the laptop's file) -- H4 owns the claims that retire them.
//    A TU must NOT include both this header and atester_shims.h until those claims land:
//      atester_shims.h:265  void ADAM_DirectWriteData(int, int, int=1)  vs golden :23 (WORD, int, int=1) -- a different
//                           overload; a 2-int call binds the no-op stand-in silently (no compile or link error).
//      atester_shims.h:266  void ADAM_WriteVoltage(double)              vs golden :18 bool ADAM_WriteVoltage(double) --
//                           compile error "ambiguating new declaration" in one TU; same mangled name -> duplicate
//                           definition at link (atester_shims.cpp:327 vs Adam6024Comm_St02.cpp).
//      atester_shims.h:267  bool ADAM_Alarm()                           vs golden :11 bool ADAM_Alarm(int iArm=2) --
//                           `ADAM_Alarm()` becomes an ambiguous call.
//      atester_shims.h:268  bool ADAM_Alarm(int)                        -- same symbol as golden :11 (duplicate at link).
//      atester_shims.h:269  void ADAM_Rang(int)                         -- same symbol as golden :9 (duplicate at link).
//
//  WHO DEFINES WHAT (golden 912 adam6024.cpp lines; one definition each)
//    H1 Adam6024Comm_St02.cpp   ShowDoubleEPConnectGuide :73-93 (static), TfAdam6024 :122-140 / :2052-2127 / :2206-2216,
//                               Set/GetAiInputRange :142-193, Set/GetAoOutputRange :195-246, Open_ADAM_6024 :248-409,
//                               Close_ADAM_6024 :411-431, ADAM_ReadVoltage :433-505, ADAM_ReadPA :507-526,
//                               ADAM_WriteVoltage :1814-1931, ADAM_WriteMaxData :1933-1950, ADAM_DirectWriteData
//                               :1952-2037, WriteDigital :2041-2050, ADAM_ReadAIValue :2129-2204, GetModuleName ..
//                               fCheckConnectStatus_ADAM6024 :2762-3016; the globals of golden :32-55 that the comm
//                               layer uses (marked H1 below), Digital_10Bit :2040.
//    H2 Adam6024Pressure_St02   IsMultiEPPressureRouteActive :96-105, IsIndependentEPPressureRouteActive :108-120,
//       (expected)              AdamOutputToPA :528-541, ADAM_Rang :543-546, iAdamOutValue :548, ADAM_Alarm :549-605,
//                               ADAM_Alarm_Kg :607-668, ADAM_DualAlarm :670-712, KpaTransferKG :714-972,
//                               MultiTransferKG :978-1041, ADAM_ReturnValueCheck :2675-2759; iADAMRange :48,
//                               dADAMRange_Kg :49, the MNetLog extern :56.
//    H3 Adam6024Apax_St02       Open_APAX :2219-2267, ConnectTcpServerCompletedEventHandler :2269-2291,
//       (expected)              DisconnectTcpServerCompletedEventHandler :2293-2298, ClientWriteReg_1_8_Handler
//                               :2300-2303, APAX_WriteData :2305-2673; cAddress :43, ulClientHandle :52 (and the two
//                               handler forward declarations :50-51).
//    laptop adam6024.cpp        TransformFuntion :1043-1812, EPSwitchOnOff :3018-3051, EpSwitch :3053-3103.
//    elsewhere                  bADAM6024FWIsNew[3] -- golden cmydef.cpp:5545 / cmydef.h:5613; port cmydef.h:5573.
//    "(expected)" = the owner's brief is not visible to H1; St02-E confirms before the build.
// =============================================================================
#ifndef ADAM6024_ST02_H
#define ADAM6024_ST02_H

#include "vclcompat/vcl_compat.h"     // AnsiString, WORD (windows.h), PACKAGE (empty)
#include "vclcompat/ClientSocket.h"   // TClientSocket / TCustomWinSocket / TErrorEvent (golden <ScktComp.hpp>, :61)

//---------------------------------------------------------------------------
// golden 912 adam6024.h:5-44, in golden's order (owner in the right-hand note)
//---------------------------------------------------------------------------
bool Open_ADAM_6024(AnsiString IP, int Num);                                    // golden :5   H1  //Ifor 20150709 ：加入設備位置
bool Open_ADAM_6024();                                                          // golden :6   H1  //Jimmychiu 20230804 : 整合全部連線檢查
void Close_ADAM_6024();                                                         // golden :7   H1
int ADAM_ReadPA(double *dValue, int iCH=5);                                     // golden :8   H1  //20111217 ChungHung   //Ifor 20190104 :add 選擇 Adam 讀取CH //wei 20220309 Add EP Return Voltage
void ADAM_Rang(int v);                                                          // golden :9   H2  //20111217 ChungHung

bool ADAM_Alarm(int iArm=2);                                                    // golden :11  H2  //20111217 ChungHung
bool ADAM_Alarm_Kg(int iAdd);                                                   // golden :12  H2  //JerryYang 20171024 (wei) add EP check range by kg
bool ADAM_DualAlarm(int iType);                                                 // golden :13  H2  //Ifor 20221228 add:Dual EP Check

//AI(ht9045-v899) 20260526: expose actual EP pressure route so Multi mode follows SwMultiEp state.
bool IsMultiEPPressureRouteActive();                                            // golden :16  H2
bool IsIndependentEPPressureRouteActive();                                      // golden :17  H2
bool ADAM_WriteVoltage(double v);                                               // golden :18  H1
void WriteDigital(double v);                                                    // golden :19  H1
extern bool bADAM6420Install;                                                   // golden :20  H1 (adam6024.cpp:40)

double ADAM_ReadVoltage(int Num,int iCH=5);                                     // golden :22  H1  //Ifor 20150709 :選擇裝置順序讀取資料   //Ifor 20190104 :add 選擇 Adam 讀取CH
void ADAM_DirectWriteData(WORD data,int Num, int iAdd=1);                       // golden :23  H1  //Ifor 20150709 :選擇裝置順序寫入資料
void ADAM_WriteMaxData(bool bFullForce);                                        // golden :24  H1  //Steven 20241014 : 整合auto height輸出壓力
double KpaTransferKG(int fKpa, bool bDualForce=false);                          // golden :25  H2  //Ifor 20150826 :新增 Kpa 轉 公斤 Function  //JerryYang 20171023 (wei) int ->double
//double MultiTransferKG(int fKpa, bool bDualForce=false);  //Eastsun 20260710 Merge
double MultiTransferKG(double fKpa, bool bDualForce=false);                     // golden :27  H2  [912] //Eastsun 20260710 Merge (906 adam6024.h: `int fKpa`)
extern double iAdamOutValue;                                                    // golden :28  H2 (adam6024.cpp:548)
// golden :29 int TransformFuntion(...) -- declared in adam6024.h (the laptop's file), not here.
bool ADAM_ReadAIValue(int iNum, int iChannel, double *dmA, double *dDegree);    // golden :30  H1  //Hmy 20170609 露點計讀AI使用，先寫固定

void APAX_WriteData(bool bDir, WORD wdata, int iArm=0);                         // golden :32  H3
bool Open_APAX(char IP[]);                                                      // golden :33  H3
void ADAM_ReturnValueCheck(bool bHome=false);                                   // golden :34  H2  //wei 20220309 Add EP Voltage Error Alarm

bool fCheckConnectStatus_ADAM6024(int Num);                                     // golden :36  H1  //Hmy 20170120 add check Adam6024 Status
int GetModuleConnectionCount(int Num);                                          // golden :37  H1  //Nickliu 20230313 add ADAM6024 Command
AnsiString GetFirmwareName(int Num);                                            // golden :38  H1  //Nickliu 20230313 add ADAM6024 Command
AnsiString GetModuleName(int Num);                                              // golden :39  H1  //Nickliu 20230313 add ADAM6024 Command
bool ClearAllConnection(int Num);                                               // golden :40  H1  //Nickliu 20230313 add ADAM6024 Command
int GetModuleHostIdleTime();                                                    // golden :41  -- NO body anywhere (see D4)  //Nickliu 20230313 add ADAM6024 Command
int GetModuleHostIdleTime(int Num);                                             // [W906] D4: the overload golden adam6024.cpp:2867 defines  H1
bool SetModuleHostIdleTime(int ihostIdleTime);                                  // golden :42  H1  //Nickliu 20230313 add ADAM6024 Command
bool fCheckModuleFWISNew_ADAM6024(int Num);                                     // golden :43  H1  //Nickliu 20230314 Add Check Adam Module Name
bool fCheckModuleName_ADAM6024(int Num);                                        // golden :44  H1  //Nickliu 20230314 Add Check Adam FW Is New

// golden :46-52 EpSwitch / EPSwitchOnOff / enum EPSwOn -- declared in atester_shims.h:281-282, not here.

//---------------------------------------------------------------------------
// golden 912 adam6024.h:63-88 -- class TfAdam6024 (adam6024.dfm: Caption 'ADAM', one TClientSocket)
//   [W906] D1 no TForm base / D2 no __fastcall / D3 destructor.  Bodies: Adam6024Comm_St02.cpp.
//   ClientSocket1 (.dfm :18-27): Address '172.16.8.110', Port 10001, ClientType ctNonBlocking, OnConnect, OnError.
//   It is a vclcompat socket in its default SIM mode (vclcompat/ClientSocket.h:157-165: no OS socket); only
//   EP_Install==4 (PISO DA / ET-7226, golden :270-273 / :419-422 / :1962-1966) uses it -- see Open_ADAM_6024.
//---------------------------------------------------------------------------
class TfAdam6024
{
public:     // golden __published:    // IDE-managed Components
    TClientSocket *ClientSocket1;                                               // golden :66
    void ClientSocket1Connect(TObject *Sender,
          TCustomWinSocket *Socket);                                            // golden :67-68 (D2)
    void ClientSocket1Error(TObject *Sender,
          TCustomWinSocket *Socket, TErrorEvent ErrorEvent,
          int &ErrorCode);                                                      // golden :69-71 (D2)
    void FormDestroy(TObject *Sender);                                          // golden :72 (D2)
private:    // User declarations
    TfAdam6024(const TfAdam6024&);                                              // [W906] not copyable (owns ClientSocket1)
    TfAdam6024& operator=(const TfAdam6024&);
public:     // User declarations
    explicit TfAdam6024(TComponent* Owner);                                     // golden :75 (D2)
    ~TfAdam6024();                                                              // [W906] D3

    struct cmdbuf
    {
        int status;
        char cmdBuf[512];
        char cmdSize;
    } cmdBuf[8];                                                                // golden :77-82
    unsigned bufIdx;                                                            // golden :83
    void WriteAO(double AO1=0.0,  double AO2=0.0, double AO3=0.0, double AO4=0.0); // golden :84
    void OpenSocket(AnsiString IP, int Port);                                   // golden :85
    void CloseSocket();                                                         // golden :86
    AnsiString ADAMErrorMessage[16];        //JimmyChiu 20230308 : 15 --> 16    // golden :87
};
extern PACKAGE TfAdam6024 *fAdam6024;                                           // golden :89  H1 (adam6024.cpp:32); created by W906_AdamFormShowOpen (Adam6024Integrate_St02.cpp [W906] D7, golden HT9045.cpp:239)
WORD TransFuntion(double v);  //JerryYang 20160701 1~16kg建表轉換W             // golden :90  -- declaration only, no golden body (see 3. above)

//---------------------------------------------------------------------------
// [W906] ADAM port internals -- golden adam6024.cpp file-scope names made cross-TU by the split.
//   Define ADAM6024_ST02_INTERNAL before including this header ONLY in Adam6024*_St02.cpp and their ctests.
//   Each line: golden 912 adam6024.cpp line, the DEFINING file, the files that read it.
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
// [W906] the EP integration (H4, Adam6024Integrate_St02.cpp): the ONE live switch and golden's TfMain / uhome / csystem /
//   iosetview EP writers.  OFF (MachineType.h W906_ADAM_EP_LIVE not defined, the default) = the pre-port machine: no ADAMTCP.dll,
//   no ADAM I/O, the retired stand-ins' answers.  ON = every golden EP writer at once.
//---------------------------------------------------------------------------
bool W906_AdamEpLive();                                                         // the live switch (build define, or the test override)
void W906_AdamEpLive_SetForTest(int iLive);                                     // ctest: -1 build default, 0 OFF, 1 ON
void W906_AdamTimer2Tick();                                                     // golden 912 main.cpp:21677-21803 + :22354-22355, one tick
void W906_AdamTimer2Pump();                                                     // wb_serve: 1 s limiter + InitialOK guard (:21495) + the tick
void W906_AdamTimer2_ResetForTest();                                            // golden Timer2Timer statics (:21482-21486) to their start values
void W906_AdamFormShowOpen();                                                   // golden main.cpp:9890 (+ HT9045.cpp:239 CreateForm guard)
void W906_AdamFormCloseZero();                                                  // golden main.cpp:11947-11952
void W906_AdamFormCloseDisconnect();                                            // golden main.cpp:12194
void W906_AdamHomeReturnValueCheck(bool bHome);                                 // golden csystem.cpp:10999
void W906_AdamIoPageShowEp();                                                   // golden iosetview.cpp:495-522 (EP lines; not wired yet)
void W906_AdamIoPageClose();                                                    // golden iosetview.cpp:293-294 (not wired yet)
const char* W906_AdamEpState();                                                 // observability (boot line / tests)

#ifdef ADAM6024_ST02_INTERNAL
extern unsigned char NetID;                                                     // :34     H1   (WriteAO)
extern int     iConnectionTimeout;                                              // :36     H1   (Open :359, fCheckConnectStatus :2991)
extern int     iSendTimeout;                                                    // :37     H1
extern int     iReceiveTimeout;                                                 // :38     H1
extern bool    bOpen;                                                           // :40     H1   (never read in golden)
extern AnsiString Address[3];                                                   // :42     H1   (H1 everywhere; H3 APAX_WriteData :2416/:2439/:2577/:2646)
extern char    cAddress[100];                                                   // :43     H3   (Open_APAX :2236/:2241)
extern double  fValue[16];                                                      // :44     H1   (ADAM_ReadVoltage)
extern WORD    wGain[10];                                                       // :45     H1   (Open :380-386, ADAM_ReadVoltage :463)
extern WORD    wHex[16];                                                        // :45     H1   (ADAM_ReadVoltage :463/:483)
extern bool    bCanReadData;                                                    // :46     H1   (ADAM_DirectWriteData :1955/:2036)
extern int     iWritePA;                                                        // :47     H1   (H1 write path; H2 ADAM_Alarm :567)
extern int     iADAMRange;                                                      // :48     H2   (ADAM_Rang :545, ADAM_Alarm :573-586)
extern double  dADAMRange_Kg;                                                   // :49     H2   (ADAM_Alarm_Kg :633-656)
extern unsigned long ulClientHandle;                                            // :52     H3   (Open_APAX :2248)
extern bool    bConnectStatus[3];                                               // :53     H1   (H1 read/write paths; H2 ADAM_Alarm :556-557)
extern bool    bADAM6420CheckRange[4];                                          // :54     H1   (Open :288/:356)
double AdamOutputToPA(int iAdamOutput);                                         // :528    H2   (H1 ADAM_WriteVoltage :1913; H2 ADAM_Alarm :567, ADAM_DualAlarm :683)
bool SetAiInputRange(const char *ipAddr, int i_iChannel, byte i_byRange);       // :142    H1   ([W906] E5 const char*; golden has no prototype -- declared for the ctest)
bool GetAiInputRange(const char *ipAddr, int i_iChannel, byte* i_byRange);      // :160    H1
bool SetAoOutputRange(const char *ipAddr, int i_iChannel, byte i_byRange);      // :195    H1
bool GetAoOutputRange(const char *ipAddr, int i_iChannel, byte* i_byRange);     // :213    H1
AnsiString W906_Adam6024_ErrorText(int iRet);                                   // [W906] E1 H1: golden fAdam6024->ADAMErrorMessage[iRet], bound-checked
#endif // ADAM6024_ST02_INTERNAL

#endif // ADAM6024_ST02_H
