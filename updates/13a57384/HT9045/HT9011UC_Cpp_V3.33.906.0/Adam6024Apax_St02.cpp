// ===========================================================================
//  Adam6024Apax_St02.cpp  --  AI(W906-ST02-ADAM) 20261002 (St02-E helper H3)
//
//  golden 912 adam6024.cpp:2218-2673 (D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy), word for word:
//      Open_APAX                                  :2219-2267   (49 lines)
//      ConnectTcpServerCompletedEventHandler      :2269-2291   (23)
//      DisconnectTcpServerCompletedEventHandler   :2293-2298   (6)
//      ClientWriteReg_1_8_Handler                 :2300-2303   (4)
//      APAX_WriteData                             :2305-2673   (369)
//  plus the two file-scope globals only this code uses: cAddress :43, ulClientHandle :52, and the
//  file-scope prototypes :50-51.
//  906_0625_Steven :2203-2671 is the same code: 912 only DELETED two commented-out
//  `else if(iIndEPCnt==8)` blocks (906 :2481-2487 and :2531-2537).  So there is no [912] code line here
//  (the two spots are marked [912] below).  Measured 20261002 with difflib over both slices.
//
//  TWO VENDOR APIS
//  ---------------------------------------------------------------------------
//  * APAX_WriteData writes with ADAMTCP_WriteReg (ADAMTCP.dll, synchronous Modbus/TCP; the run-time shim is
//    H1's Public/AdamTcp_St02.h) to Address[2] (golden :42 default "172.16.8.112", the APAX-5070 head):
//    unit id 1, 8 holding registers, start 1 = APAX-5028 S0, start 33 = APAX-5028 S1.  That is the ONLY
//    ADAMTCP entry point this file needs.  The ADAM reconnect in the SHIP-only failure branch is golden's own
//    Close_ADAM_6024() / Open_ADAM_6024() (H1's Adam6024Comm_St02.cpp).
//  * Open_APAX alone uses ADSMOD.dll's asynchronous Modbus client (MOD_Initialize / MOD_AddTcpClientConnect /
//    MOD_StartTcpClient / MOD_SetTcpClientPriority): this helper's run-time shim Public/ApaxShim_St02.{h,cpp}
//    (cdecl, measured -- see that banner).
//    GOLDEN HAS NO CALLER OF Open_APAX: 912 and 906 trees, every .cpp/.h/.dfm, grep 20261002 (only the
//    definition and adam6024.h:33).  So the connect / disconnect handlers are reachable only through it,
//    ClientWriteReg_1_8_Handler is referenced nowhere, and bAPAXConnectFileAlarm / bApaxReadFinish /
//    bApaxWriteFinish are written but never READ anywhere in golden 912.  Translated live anyway (golden
//    faithful, and nothing calls it).  NB2 Q8c section 8 item 5 (still pending Jimmy,
//    docs/nb2_assist/PENDING_JIMMY.md:33) suggested "translate into #if 0, no ADSMOD shim"; if that is the
//    answer, drop Public/ApaxShim_St02.cpp from CMake and wrap Open_APAX -- nothing else depends on it.
//
//  WHO GETS PAST THE FIRST LINE
//  ---------------------------------------------------------------------------
//  APAX_WriteData returns at once unless INSTALL_DOUBLE_EP >= 2 (cmydef.h:4132-4135: 2 = DOUBLE_EP_INDIVIAL,
//  3 = DOUBLE_EP_MULTI; Gerneral.ini [System] INSTALL_DOUBLE_EP, database.cpp:1231).  MULTI also needs
//  IsMultiEPPressureRouteActive() (golden :96-105: bIndEPSLK and SW[SwMultiEp] ON).
//  HT9050 does NOT use APAX: machines/HT9050/sim_9378/Gerneral.ini:2 EP_Install=3 (one ADAM-6024) and :106
//  INSTALL_DOUBLE_EP=1 -> the first return.  The dev PC D:\HT9045\system\Gerneral.ini:100 has
//  INSTALL_DOUBLE_EP=0.  The Multi EP path is the SIGURD 16-channel port (cmydef.h:4135 comment).
//
//  DATA
//  ---------------------------------------------------------------------------
//  Reads (never writes) iAPAXEPValue[16] / iAPAXDualEPValue[16] (cmydef.cpp:5235-5236, filled by
//  TransformFuntion, adam6024.cpp:465/467/753/795), iIndEPCnt (cmydef.cpp:5276; database.cpp:910-918 sets
//  16 / 8 / 4), TestIF_File.iTestMode / iShuttleMode / iShuttle_Sel (cprod.h:1652-1656).  Writes
//  bApaxWriteFinish (cmydef.cpp:5233).  The handlers write bAPAXConnectFileAlarm / bApaxWriteFinish /
//  bApaxReadFinish (cmydef.cpp:5232-5234).
//
//  GOLDEN QUIRKS, KEPT (each one is golden's behaviour, not a port bug)
//  ---------------------------------------------------------------------------
//  Q1 non-MULTI retry: the failure log loop `for(i=0; i<16; i++)` (:2589 / :2658) reuses the retry counter
//     `i` of `for(i=0; i<10; i++)` (:2575 / :2644).  SHIP build: a failed write is tried ONCE (log, Close,
//     Open, then i is 17 and the loop ends).  SIM build (SOFT_SIMULTE): the log block is compiled out, so it
//     is tried 10 times.  MULTI uses kMep and tries 10 times in both builds.
//  Q2 INDIVIAL block <-> arm: iArm 0 or 2 (or iIndEPCnt 4 / 8) writes start 1; iArm 0 or 1 writes start 33,
//     and only when iIndEPCnt==16.  MULTI is the other way round (Arm1 -> start 1, Arm2 -> start 33).
//     Callers: aTester_Front.cpp:2241 passes 1, aTester_Rear.cpp:2243 passes 2 (golden 912).
//  Q3 MULTI writes BOTH blocks every time; the arm that is not selected gets zeros.
//  Q4 byData[16] (:2466-2483) is filled and never used.
//  Q5 Open_APAX returns true when MOD_Initialize or MOD_AddTcpClientConnect fails; only a failing
//     MOD_StartTcpClient returns false.
//  Q6 iIndEPCnt==8 in DualSite takes the 4-EP layout (:2489-2496, :2532-2539).
//  Q7 no clamp here: wdata*16 and the int -> WORD stores wrap modulo 65536 exactly like BCB6's WORD.
//  Q8 ADSMOD runs the connect / disconnect handlers on ITS worker thread (CreateThread in
//     MOD_StartTcpClient, measured): the bools and NewRecordProcess are touched from that thread, as in golden.
//
//  [W906] DEVIATIONS
//  ---------------------------------------------------------------------------
//  D1 The golden bodies below are compiled inside `namespace w906_apax`.  Four same-named callees declared
//     there -- ADAMTCP_WriteReg, Close_ADAM_6024, Open_ADAM_6024, NewRecordProcess -- hide the global ones
//     (ordinary name hiding) and forward through test seams whose DEFAULTS ARE THE GLOBAL FUNCTIONS, so the
//     golden lines read word for word and production behaviour is unchanged.  The global symbols
//     (APAX_WriteData, Open_APAX and the three handlers) are one-line forwarders with golden's signatures,
//     at the end of this file.  Everything else (Address, EP_Install, iAPAX*, TestIF_File, MOD_*,
//     IsMultiEPPressureRouteActive, ...) is found in the global namespace as usual.
//  D2 `(void)param;` lines for the handler parameters golden never reads (-Wunused-parameter),
//     `(void)kMep;` under #ifdef SOFT_SIMULTE (kMep is used only inside the SHIP-only log blocks) and
//     `(void)byData;` (Q4, -Wunused-but-set-variable).  The 53 int -> WORD / unsigned char stores are golden's
//     implicit conversions, left as they are: the CMake line for this TU adds -Wno-conversion (same per-TU
//     precedent as adam6024.cpp's -Wno-float-conversion) and -Wno-pedantic (MachineType.h's stray ';' idiom,
//     same as cContact.cpp / MainCalcCore.cpp).  Measured with g++ 6.3 -fsyntax-only, both configurations.
//  D3 vclcompat's AnsiString::c_str() is const char* (BCB6: char*); H1's ADAMTCP_WriteReg wrapper takes
//     `const char*` for the vendor's `char szIP[]` (Public/AdamTcp_St02.h S4), so golden's
//     `ADAMTCP_WriteReg(Address[2].c_str(), ...)` compiles as written and the seam passes it on unchanged.
//  D5 the ADSMOD shim copies H1's S1 / S2: a SIM build or a ctest never loads ADSMOD.dll.  (In SIM, H1's S1
//     also makes every ADAMTCP_WriteReg of APAX_WriteData return -1 without I/O; golden's SIM build would have
//     written: golden has no SOFT_SIMULTE guard around the write, only around the failure log.)
//  D4 ADSMOD.dll is bound at run time (Public/ApaxShim_St02.h); a missing DLL makes every MOD_* return 801
//     (MODERR_WSASTART_FAILED, what MOD_Initialize itself returns when start-up fails) -> Open_APAX takes
//     golden's own branches.
//
//  TEST SEAMS (tests/test_adam6024_apax.cpp; 0 restores golden's callee)
//      W906_ApaxSetWriteReg(fn)            ADAMTCP_WriteReg
//      W906_ApaxSetAdamReconnect(cl, op)   Close_ADAM_6024 / Open_ADAM_6024 (SHIP failure branch only)
//      W906_ApaxSetNewRecordProcess(fn)    NewRecordProcess
// ===========================================================================
#define ADAM6024_ST02_INTERNAL      // Adam6024_St02.h:162-185: the cross-TU adam6024.cpp file-scope names (Address, cAddress, ulClientHandle)
#include "Adam6024_St02.h"          // H1: golden adam6024.h declarations (APAX_WriteData, Open_APAX, Open/Close_ADAM_6024,
                                    //     IsMultiEPPressureRouteActive) + extern AnsiString Address[3] (H1 defines it)
#include "Public/AdamTcp_St02.h"    // H1: ADAMTCP_WriteReg (ADAMTCP.dll run-time shim)
#include "Public/ApaxShim_St02.h"   // H3: MOD_* (ADSMOD.dll run-time shim), MODERR_SUCCESS
#include "cmydef.h"                 // EP_Install, INSTALL_DOUBLE_EP, DOUBLE_EP_MULTI, iIndEPCnt, iAPAX*, bAPAX* / bApax*
#include "cprod.h"                  // TestIF_File
#include "MachineType.h"            // DualSite, SOFT_SIMULTE

#include <windows.h>                // WORD, THREAD_PRIORITY_HIGHEST
#include <cstring>                  // strcpy

// golden cMyDB.h:62 (port cMyDB.h:129, body cMyDB.cpp golden :1545-1562).  Re-declared WITHOUT the default
// argument instead of including cMyDB.h: cMyDB.h re-declares canary_support.h's RecordProcess default, and
// a redeclaration without defaults is legal next to either header.
void NewRecordProcess(AnsiString AlarmCode, AnsiString S, AnsiString Debug);

//---------------------------------------------------------------------------
// golden 912 adam6024.cpp:43 / :52 -- used only by Open_APAX (grep 20261002).  H3 owns both (Adam6024_St02.h:173 / :181).
char cAddress[100]={"172.16.8.112"};
unsigned long ulClientHandle;                                                   //Nickliu 20180827 Add APAX Dll Connection

// golden :50-51 (file-scope prototypes; the third handler has none in golden)
void ConnectTcpServerCompletedEventHandler(long lResult, char *i_szIp, void *i_Param);
void DisconnectTcpServerCompletedEventHandler(long lResult, char *i_szIp, void *i_Param);
void ClientWriteReg_1_8_Handler(long i_lResult, void *i_Param);                 // [W906] golden has no prototype; the forwarder below needs one for -Wmissing-declarations builds

// ---- [W906] D1: the test seams, defaults = golden's own (global) callees -------------------------------
namespace {

int ApaxDefaultWriteReg(const char* szIP, WORD wIDAddr, WORD wStartAddress, WORD wCount, WORD wData[])
{
    return ::ADAMTCP_WriteReg(szIP, wIDAddr, wStartAddress, wCount, wData);    // [W906] D3: H1's wrapper takes const char*
}
void ApaxDefaultCloseAdam()
{
    ::Close_ADAM_6024();
}
bool ApaxDefaultOpenAdam()
{
    return ::Open_ADAM_6024();
}
void ApaxDefaultRecord(AnsiString AlarmCode, AnsiString S, AnsiString Debug)
{
    ::NewRecordProcess(AlarmCode, S, Debug);
}

int  (*g_pfnWriteReg)(const char*, WORD, WORD, WORD, WORD*) = ApaxDefaultWriteReg;
void (*g_pfnCloseAdam)()                                    = ApaxDefaultCloseAdam;
bool (*g_pfnOpenAdam)()                                     = ApaxDefaultOpenAdam;
void (*g_pfnRecord)(AnsiString, AnsiString, AnsiString)     = ApaxDefaultRecord;

} // anonymous namespace

void W906_ApaxSetWriteReg(int (*fn)(const char*, WORD, WORD, WORD, WORD*))
{
    g_pfnWriteReg = fn ? fn : ApaxDefaultWriteReg;
}
void W906_ApaxSetAdamReconnect(void (*closeFn)(), bool (*openFn)())
{
    g_pfnCloseAdam = closeFn ? closeFn : ApaxDefaultCloseAdam;
    g_pfnOpenAdam  = openFn  ? openFn  : ApaxDefaultOpenAdam;
}
void W906_ApaxSetNewRecordProcess(void (*fn)(AnsiString, AnsiString, AnsiString))
{
    g_pfnRecord = fn ? fn : ApaxDefaultRecord;
}

namespace w906_apax {

// ---- [W906] D1: these four hide the global functions of the same name inside this namespace only ------------
static int ADAMTCP_WriteReg(const char* szIP, WORD wIDAddr, WORD wStartAddress, WORD wCount, WORD wData[])
{
    return g_pfnWriteReg(szIP, wIDAddr, wStartAddress, wCount, wData);
}
static void Close_ADAM_6024()
{
    g_pfnCloseAdam();
}
static bool Open_ADAM_6024()
{
    return g_pfnOpenAdam();
}
static void NewRecordProcess(AnsiString AlarmCode, AnsiString S, AnsiString Debug=" ")   // golden cMyDB.h:62 default " "
{
    g_pfnRecord(AlarmCode, S, Debug);
}

void ConnectTcpServerCompletedEventHandler(long lResult, char *i_szIp, void *i_Param);      // golden :50
void DisconnectTcpServerCompletedEventHandler(long lResult, char *i_szIp, void *i_Param);   // golden :51

// ===========================================================================
//  golden 912 adam6024.cpp:2218-2673, word for word (906_0625_Steven :2202-2671)
// ===========================================================================
//Nickliu 20180827 Add APAX Dll Connection-->
// ---- golden 912 :2219-2267 Open_APAX (906 :2203-2251) -------------------------------------------------
//AI(W906-ST02-ADAM) 20261002 (St02-E helper H3): golden has NO caller of this function (912 and 906, grep 20261002); Q5: true unless
//    MOD_StartTcpClient fails.  MOD_* = Public/ApaxShim_St02.h (ADSMOD.dll bound at run time, D4).
bool Open_APAX(char IP[])
{
    //ChungHung 20180906 modify delete start
//    iRet = MOD_AddTcpClientConnect(Address,           // remote server IP
//                                                    100,          // scan interval (ms)
//                                                    3000,         // connection timeout (ms)
//                                                    100,          // transaction timeout (ms)
//                                                    NULL,
//                                                    NULL,
//                                                    NULL,
//                                                    &ulClientHandle);
    //ChungHung 20180906 modify delete end

    if(EP_Install)
    {
//        if (iEPAPAXMode == 1)
        {
            strcpy(cAddress, IP);

            //ChungHung 20180906 modify for APAX start
            if(MODERR_SUCCESS == MOD_Initialize())
            {
                if(MODERR_SUCCESS == MOD_AddTcpClientConnect(cAddress,          // remote server IP
                                                    100,            // scan interval (ms)
                                                    3000,           // connection timeout (ms)
                                                    100,            // transaction timeout (ms)
                                                    ConnectTcpServerCompletedEventHandler,
                                                    DisconnectTcpServerCompletedEventHandler,
                                                    NULL,
                                                    &ulClientHandle))
                {
                    if(MODERR_SUCCESS == MOD_StartTcpClient())
                    {
                        MOD_SetTcpClientPriority(THREAD_PRIORITY_HIGHEST);
                    }
                    else
                    {
                        return false;
                    }
                }
            }
            return true;
        }
    }
    else
    {
        return false;
    }
}
//---------------------------------------------------------------------------
// ---- golden 912 :2269-2291 (906 :2253-2275) ----------------------------------------------------------
//AI(W906-ST02-ADAM) 20261002 (St02-E helper H3): ADSMOD calls this on its own worker thread (Q8).  Reached only through Open_APAX.
void ConnectTcpServerCompletedEventHandler(long lResult, char *i_szIp, void *i_Param)
{
    (void)i_Param;                                                              // [W906] D2: golden never reads it
    //char str[100];
    AnsiString Str;
    //ChungHung 20180907 modify for APAX Log
    Str.sprintf("APAX Connect to '%s' result = %d\n", i_szIp, lResult);
//    WriteApaxLogs(Str);
    NewRecordProcess("", Str);
    //ChungHung 20180907 modify for APAX Log
    if(lResult!=0)
    {
//        bConnectStatus = false;
        //sprintf(str,"APAX Connect to '%s' result = %d\n", i_szIp, lResult);
        //ShowMyMessage("APAX Connect Fail!");
        bAPAXConnectFileAlarm=true;
    }
    else
    {
        bAPAXConnectFileAlarm=false;
        bApaxWriteFinish=true;
        bApaxReadFinish =true;
    }
}
//---------------------------------------------------------------------------
// ---- golden 912 :2293-2298 (906 :2277-2282) ----------------------------------------------------------
void DisconnectTcpServerCompletedEventHandler(long lResult, char *i_szIp, void *i_Param)
{
    (void)i_Param;                                                              // [W906] D2: golden never reads it
    AnsiString Str;
    Str.sprintf("APAX Disconnect from '%s' result = %d\n", i_szIp, lResult);    //ChungHung 20180907 modify for APAX Log
    NewRecordProcess("", Str);
}
//---------------------------------------------------------------------------
// ---- golden 912 :2300-2303 (906 :2284-2287) ----------------------------------------------------------
//AI(W906-ST02-ADAM) 20261002 (St02-E helper H3): referenced nowhere in golden (it has the OnModbusWriteCompletedEvent shape, ADSMOD.h:76).
void ClientWriteReg_1_8_Handler(long i_lResult, void *i_Param)
{
    (void)i_lResult; (void)i_Param;                                             // [W906] D2: golden never reads them
    bApaxWriteFinish=true;
}
//---------------------------------------------------------------------------
// ---- golden 912 :2305-2673 APAX_WriteData (906 :2289-2671) ---------------------------------------------
void APAX_WriteData(bool bDir, WORD wdata, int iArm)
{
    if(INSTALL_DOUBLE_EP<2)
    {
        return;
    }

    if(INSTALL_DOUBLE_EP==DOUBLE_EP_MULTI && IsMultiEPPressureRouteActive()==false) //AI(ht9045-v899) 20260526: SwMultiEp OFF means common EP route, so skip Multi APAX writes.
    {
        return;
    }

    //AI(ht9045-v899) 20260504: full port V874.3 INSTALL_DOUBLE_EP==DOUBLE_EP_MULTI (撤回 20260430 砍半版)。
    //   Hardware: 1x APAX-5070 (Address[2]=172.16.8.112) + 2x APAX-5028 = S0/S1 separated writes.
    //   ch 對應 (參 SPEC-V899-MultiEP-FullPort, ADR-0004):
    //     wData2[0..7]  Arm1: Dual1, Dual2, Site1, Site2, Site3, Site4, Dual3, Dual4
    //     wData2[8..15] Arm2: 同上
    //   ShuttleMode==1 + Sel==0 只更新 Arm1；Sel==1 只更新 Arm2；Mode==0 同時更新。
    //   S0 Arm1: ADAMTCP_WriteReg(Address[2], 1, 1, 8, ...); S1 Arm2: start 33.
//AI(W906-ST02-ADAM) 20261002 (St02-E helper H3): MULTI: both S0 (start 1) and S1 (start 33) are written every call, the arm
//    not selected gets zeros (Q3); kMep keeps the retry counter intact -> 10 tries in both builds.
    if(INSTALL_DOUBLE_EP==DOUBLE_EP_MULTI)
    {
        bApaxWriteFinish=false;
        int iRetMep, iMep, kMep;
#ifdef SOFT_SIMULTE
        (void)kMep;                                                             // [W906] D2: used only in the SHIP-only log blocks
#endif
        WORD wData2Mep[16];
        WORD wDataArm1Mep[8];                                                  //AI(ht9045-v899) 20260526: APAX-5028 S0 controls Arm1 AO0-AO7.
        WORD wDataArm2Mep[8];                                                  //AI(ht9045-v899) 20260526: APAX-5028 S1 controls Arm2 AO0-AO7.
        AnsiString StrMep="", str1Mep="", str2Mep="";
        for(iMep=0; iMep<16; iMep++) wData2Mep[iMep]=0;
        for(iMep=0; iMep<8; iMep++)
        {
            wDataArm1Mep[iMep]=0;
            wDataArm2Mep[iMep]=0;
        }

        bool bWriteArm1Mep=((TestIF_File.iShuttleMode==1 && TestIF_File.iShuttle_Sel==0) || TestIF_File.iShuttleMode==0); //AI(ht9045-v899) 20260526: allow direct writes to select Arm1/Arm2 via iArm while keeping production shuttle selection.
        bool bWriteArm2Mep=((TestIF_File.iShuttleMode==1 && TestIF_File.iShuttle_Sel==1) || TestIF_File.iShuttleMode==0);
        if(iArm==1)
        {
            bWriteArm1Mep=true;
            bWriteArm2Mep=false;
        }
        else if(iArm==2)
        {
            bWriteArm1Mep=false;
            bWriteArm2Mep=true;
        }

        if(bWriteArm1Mep)
        {
            wData2Mep[0]=iAPAXDualEPValue[0];
            wData2Mep[1]=iAPAXDualEPValue[1];
            wData2Mep[2]=iAPAXEPValue[0];
            wData2Mep[3]=iAPAXEPValue[1];
            wData2Mep[4]=iAPAXEPValue[2];
            wData2Mep[5]=iAPAXEPValue[3];
            wData2Mep[6]=iAPAXDualEPValue[2];
            wData2Mep[7]=iAPAXDualEPValue[3];
            if(bDir==true)
            {
                //AI(ht9045-v899) 20260526: direct Multi EP writes follow the installed first four Arm1 AO channels.
                wData2Mep[0]=wdata*16;
                wData2Mep[1]=wdata*16;
                wData2Mep[2]=wdata*16;
                wData2Mep[3]=wdata*16;
                wData2Mep[4]=0;
                wData2Mep[5]=0;
                wData2Mep[6]=0;
                wData2Mep[7]=0;
            }
        }
        else
        {
            for(iMep=0; iMep<8; iMep++) wData2Mep[iMep]=0*16;
        }

        if(bWriteArm2Mep)
        {
            wData2Mep[8]=iAPAXDualEPValue[4];
            wData2Mep[9]=iAPAXDualEPValue[5];
            wData2Mep[10]=iAPAXEPValue[4];
            wData2Mep[11]=iAPAXEPValue[5];
            wData2Mep[12]=iAPAXEPValue[6];
            wData2Mep[13]=iAPAXEPValue[7];
            wData2Mep[14]=iAPAXDualEPValue[6];
            wData2Mep[15]=iAPAXDualEPValue[7];
            if(bDir==true)
            {
                //AI(ht9045-v899) 20260526: direct Multi EP writes follow the installed first four Arm2 AO channels.
                wData2Mep[8]=wdata*16;
                wData2Mep[9]=wdata*16;
                wData2Mep[10]=wdata*16;
                wData2Mep[11]=wdata*16;
                wData2Mep[12]=0;
                wData2Mep[13]=0;
                wData2Mep[14]=0;
                wData2Mep[15]=0;
            }
        }
        else
        {
            for(iMep=8; iMep<16; iMep++) wData2Mep[iMep]=0;
        }

        for(iMep=0; iMep<8; iMep++)                                             //AI(ht9045-v899) 20260526: split logical 16ch buffer into APAX S0/S1 register blocks.
        {
            wDataArm1Mep[iMep]=wData2Mep[iMep];
            wDataArm2Mep[iMep]=wData2Mep[iMep+8];
        }

        for(iMep=0; iMep<10; iMep++)                                            //retry 10 times (same as V874 style)
        {
            iRetMep=ADAMTCP_WriteReg(Address[2].c_str(), 1, 1, 8, wDataArm1Mep);
            if(iRetMep==0 || iRetMep==817)
            {
                break;
            }
            else
            {
                #ifndef SOFT_SIMULTE
                if(iRetMep!=0)
                {
                    str2Mep="";
                    for(kMep=0; kMep<8; kMep++) { str1Mep.sprintf("%d", wDataArm1Mep[kMep]); str2Mep+=str1Mep; }
                    StrMep.sprintf("APAX MEP3 ARM1 S0 SEND DATA FAIL %d, %s \n", iRetMep, str2Mep);
                    NewRecordProcess("", StrMep);
                    Close_ADAM_6024();
                    Open_ADAM_6024();
                }
                #endif
            }
        }

        for(iMep=0; iMep<10; iMep++)                                            //AI(ht9045-v899) 20260526: write Arm2 to APAX-5028 S1 Modbus block.
        {
            iRetMep=ADAMTCP_WriteReg(Address[2].c_str(), 1, 33, 8, wDataArm2Mep);
            if(iRetMep==0 || iRetMep==817)
            {
                break;
            }
            else
            {
                #ifndef SOFT_SIMULTE
                if(iRetMep!=0)
                {
                    str2Mep="";
                    for(kMep=0; kMep<8; kMep++) { str1Mep.sprintf("%d", wDataArm2Mep[kMep]); str2Mep+=str1Mep; }
                    StrMep.sprintf("APAX MEP3 ARM2 S1 SEND DATA FAIL %d, %s \n", iRetMep, str2Mep);
                    NewRecordProcess("", StrMep);
                    Close_ADAM_6024();
                    Open_ADAM_6024();
                }
                #endif
            }
        }
        return;
    }

//AI(W906-ST02-ADAM) 20261002 (St02-E helper H3): from here on INDIVIAL (INSTALL_DOUBLE_EP==2).  Block <-> arm mapping is the
//    reverse of MULTI (Q2): start 1 for iArm 0/2 (and iIndEPCnt 4/8), start 33 for iArm 0/1 (iIndEPCnt 16).
    bApaxWriteFinish=false;

    int iRet, iEpindex=0, i;
    int data;
//AI(W906-ST02-ADAM) 20261002 (St02-E helper H3): byData is filled by the loop below and never used afterwards (Q4).
    unsigned char byData[16]={0};
    (void)byData;                                                               // [W906] D2: golden never reads it (Q4; -Wunused-but-set-variable)
    AnsiString Str="", str1="", str2="";
    WORD wData[16];
    iEpindex=0;

    for(i=0; i<16; i++)
    {
        wData[i]=0;
    }

    for(i=0; i<16; i+=2)                                                        //Output all AO ch# = 0x8000
    {
        data=iAPAXEPValue[iEpindex];
        byData[i]  =(data>>8)&0xFF;                                             //High byte
        byData[i+1]=data&0xFF;                                                  //Low byte
        if(i%2==0)
            iEpindex++;
    }

    if(bDir==true)
    {
        if(TestIF_File.iTestMode==DualSite)
        {
            if(iIndEPCnt==4 ||                                                  //JerryYang 20210413 : Add 4組獨立EP版本
                iIndEPCnt==8)                                                   //RogerYang 20260603 : Add 8EP
            {
                wData[0]=wdata*16;
                wData[1]=wdata*16;
                wData[4]=wdata*16;
                wData[5]=wdata*16;
            }
//  [912] 906_0625_Steven :2481-2487 had a commented-out "else if(iIndEPCnt==8)" block here; 912 deleted the
//        comment.  No code difference.
            else
            {
                wData[0]=wdata*16;
                wData[1]=wdata*16;
            }
        }
        else
        {
            if(iIndEPCnt==4)                                                    //JerryYang 20210413 : Add 4組獨立EP版本
            {
                wData[0]=wdata*16;
                wData[1]=wdata*16;
                wData[4]=wdata*16;
                wData[5]=wdata*16;
            }
            else if(iIndEPCnt==8)                                               //RogerYang 20260603 : Add 8EP
            {
                for(int j=0; j<8; j++)
                {
                    wData[j]=wdata*16;
                }
            }
            else
            {
                for(i=0; i<8; i++)
                {
                   wData[i]=wdata*16;
                }
            }
        }
    }
    else
    {
        if(TestIF_File.iTestMode==DualSite)
        {
            if(iIndEPCnt==4 ||                                                  //JerryYang 20210413 : Add 4組獨立EP版本
                iIndEPCnt==8)                                                   //RogerYang 20260603 : Add 8EP
            {
                wData[0]=iAPAXEPValue[0];
                wData[1]=iAPAXEPValue[1];
                wData[4]=iAPAXEPValue[4];
                wData[5]=iAPAXEPValue[5];
            }
//  [912] 906_0625_Steven :2531-2537 had a commented-out "else if(iIndEPCnt==8)" block here; 912 deleted the
//        comment.  No code difference.
            else
            {
                wData[0]=iAPAXEPValue[0];
                wData[1]=iAPAXEPValue[1];
            }
        }
        else
        {
            if(iIndEPCnt==4)                                                    //JerryYang 20210413 : Add 4組獨立EP版本
            {
                for(i=0; i<4; i++)
                {
                   wData[i]=iAPAXEPValue[i];
                }
            }
            else if(iIndEPCnt==8)                                               //RogerYang 20260603 : Add 8EP
            {
                for(int j=0; j<8; j++)
                {
                    wData[j]=iAPAXEPValue[j];
                }
            }
            else
            {
                for(i=0; i<8; i++)
                {
                   wData[i]=iAPAXEPValue[i];
                }
            }
        }
    }

    if(iArm==0 || iArm==2 ||
        iIndEPCnt==4 || iIndEPCnt==8)                                           //RogerYang 20260603 : Add 8EP //JerryYang 20210413 : Add 4組獨立EP版本
    {
//AI(W906-ST02-ADAM) 20261002 (St02-E helper H3): Q1 -- SHIP build: the log loop below reuses this i, so a failed write is
//    tried ONCE (then i==17 ends the loop); SIM build (SOFT_SIMULTE): no log block, 10 tries.
        for(i=0; i<10; i++)                                                     //JerryYang 20210622 : 偶發EP error, 改成retry 10次
        {
            iRet=ADAMTCP_WriteReg(Address[2].c_str(), 1, 1, 8, wData);
            if(iRet==0 || iRet==817)
            {
                break;
            }
            else
            {
                #ifndef SOFT_SIMULTE
//                bConnectStatus=false;
                if(iRet!=0)
                {
                    str2="";                                                    //ChungHung 20180907 modify for APAX Log
//AI(W906-ST02-ADAM) 20261002 (St02-E helper H3): Q1 golden quirk: this is the retry counter i of :2575 (kept as golden).
                    for(i=0; i<16; i++)
                    {
                        str1.sprintf("%d", wData[i]);
                        str2+=str1;
                    }
                    Str.sprintf("APAX SEND DATA FAIL %d, %s \n" , iRet, str2);
                    NewRecordProcess("", Str);
                    Close_ADAM_6024();
                    Open_ADAM_6024();                                           //Jimmychiu 20230804 : 整合全部連線檢查
                }
                #endif
            }
        }
    }

    if(iIndEPCnt==16)                                                           //JerryYang 20210413 : 4組獨立EP版本不用進來
    {
        for(i=0; i<16; i++)
        {
            wData[i]=0;
        }

        if(bDir==true)
        {
            if(TestIF_File.iTestMode==DualSite)
            {
                wData[0]=wdata*16;
                wData[1]=wdata*16;
            }
            else
            {
                for(i=0; i<8; i++)
                {
                   wData[i]=wdata*16;
                }
            }
        }
        else
        {
            if(TestIF_File.iTestMode==DualSite)
            {
                wData[0]=iAPAXEPValue[8];
                wData[1]=iAPAXEPValue[9];
            }
            else
            {
                for(i=0; i<8; i++)
                {
                   wData[i]=iAPAXEPValue[i+8];
                }
            }
        }

        if(iArm==0 || iArm==1)
        {
//AI(W906-ST02-ADAM) 20261002 (St02-E helper H3): Q1 -- same as :2575: ONE try on failure in the SHIP build, 10 in SIM.
            for(i=0; i<10; i++)                                                 //JerryYang 20210622 : 偶發EP error, 改成retry 10次
            {
                iRet=ADAMTCP_WriteReg(Address[2].c_str(), 1, 33, 8, wData);
                if(iRet==0 || iRet==817)
                {
                    break;
                }
                else
                {
                    #ifndef SOFT_SIMULTE
//                    bConnectStatus = false;
                    if(iRet!=0)
                    {
                        str2="";                                                //ChungHung 20180907 modify for APAX Log
//AI(W906-ST02-ADAM) 20261002 (St02-E helper H3): Q1 golden quirk: this is the retry counter i of :2644 (kept as golden).
                        for(i=0; i<16; i++)
                        {
                            str1.sprintf("%d", wData[i]);
                            str2+=str1;
                        }
                        Str.sprintf("APAX SEND DATA FAIL %d, %s \n" , iRet, str2);
                        NewRecordProcess("", Str);
                        Close_ADAM_6024();
                        Open_ADAM_6024();                                       //Jimmychiu 20230804 : 整合全部連線檢查
                    }
                    #endif
                }
            }
        }
    }
}

} // namespace w906_apax

// ===========================================================================
//  [W906] D1: the global symbols, golden signatures (golden 912 adam6024.h:32-33, adam6024.cpp:50-51 / :2300),
//  each a one-line forwarder to the golden body above.  The default argument iArm=0 lives in the header
//  declaration (adam6024.h:32), not here.
// ===========================================================================
bool Open_APAX(char IP[])
{
    return w906_apax::Open_APAX(IP);
}

void ConnectTcpServerCompletedEventHandler(long lResult, char *i_szIp, void *i_Param)
{
    w906_apax::ConnectTcpServerCompletedEventHandler(lResult, i_szIp, i_Param);
}

void DisconnectTcpServerCompletedEventHandler(long lResult, char *i_szIp, void *i_Param)
{
    w906_apax::DisconnectTcpServerCompletedEventHandler(lResult, i_szIp, i_Param);
}

void ClientWriteReg_1_8_Handler(long i_lResult, void *i_Param)
{
    w906_apax::ClientWriteReg_1_8_Handler(i_lResult, i_Param);
}

void APAX_WriteData(bool bDir, WORD wdata, int iArm)
{
    if(!W906_AdamEpLive())                                                      // [W906] D-H4 (H4 integration pass 20261002): EP live switch OFF = no-op (no stand-in existed; the gated / stubbed callers did nothing)
        return;
    w906_apax::APAX_WriteData(bDir, wdata, iArm);
}
