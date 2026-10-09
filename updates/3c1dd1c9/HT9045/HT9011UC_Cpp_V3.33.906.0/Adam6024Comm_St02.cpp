// =============================================================================
//  Adam6024Comm_St02.cpp -- the ADAM-6024 comm layer: golden 912 adam6024.cpp, connection / read / write / module
//  queries and the TfAdam6024 form (one TClientSocket, EP_Install==4 only).
//
//  AI(W906-ST02-ADAM) 20261002 (St02-E helper H1).  Card ST02-ADAM.  Source: golden 912
//  D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\adam6024.cpp (cp950), translated statement for statement; each
//  function carries "golden 912 :a-b (906 :c-d)" (906 = HT9011UC_Code_V3.33.906.0_20260625_Steven).  Lines 912 added
//  are marked [912] with the 906 shape beside them.  Every deviation is marked [W906] Ex and listed here.
//
//  WHAT IS HERE (golden 912 adam6024.cpp)
//    globals :32-55 (the comm-layer ones; Adam6024_St02.h "WHO DEFINES WHAT"), the AI/AO range enums :57-71,
//    ShowDoubleEPConnectGuide :73-93, TfAdam6024 ctor :122-140, SetAiInputRange :142-158, GetAiInputRange :160-193,
//    SetAoOutputRange :195-211, GetAoOutputRange :213-246, Open_ADAM_6024(IP,Num) :248-398, Open_ADAM_6024() :400-409,
//    Close_ADAM_6024 :411-431, ADAM_ReadVoltage :433-505, ADAM_ReadPA :507-526, ADAM_WriteVoltage :1814-1931,
//    ADAM_WriteMaxData :1933-1950, ADAM_DirectWriteData :1952-2037, Digital_10Bit :2040, WriteDigital :2041-2050,
//    TfAdam6024::ClientSocket1Connect :2052-2062 / ClientSocket1Error :2064-2078 / WriteAO :2080-2108 /
//    OpenSocket :2110-2115 / CloseSocket :2117-2127, ADAM_ReadAIValue :2129-2204, TfAdam6024::FormDestroy
//    :2206-2216, GetModuleName :2762-2785, GetFirmwareName :2788-2810, GetModuleConnectionCount :2814-2840,
//    ClearAllConnection :2844-2863, GetModuleHostIdleTime(int) :2867-2884, SetModuleHostIdleTime :2888-2905,
//    fCheckModuleName_ADAM6024 :2909-2925, fCheckModuleFWISNew_ADAM6024 :2929-2955, fCheckConnectStatus_ADAM6024
//    :2958-3016.  TransFuntion (adam6024.h:90) has no golden body anywhere -- nothing to translate.
//
//  CALLED HERE, DEFINED ELSEWHERE
//    ADAMTCP_* .................... Public/AdamTcp_St02.cpp (H1; ADAMTCP.dll at run time)
//    TransformFuntion ............. adam6024.cpp (laptop; golden :1043-1812)
//    IsIndependentEPPressureRouteActive, AdamOutputToPA ... H2 (golden :108-120, :528-541); iAdamOutValue H2 (:548)
//    APAX_WriteData ............... H3 (golden :2305-2673)
//    bADAM6024FWIsNew[3] .......... cmydef.cpp (golden cmydef.cpp:5545)
//    ShowMyMessage (canary_support.h), MyDBIProcess / NewRecordProcess (cMyDB.cpp), LogClientSocketExceptionError
//    (Public/WinSocketErrorCode.cpp:430), MySleepEx (common.cpp), LogSoftwareOffTime (acarry_shims.cpp no-op),
//    W906_FormShowing (csystem.cpp), float2hex / swap16 (EJ1N/TextProcess), SW[] (myswitch), CheckRange /
//    ChangeToFloatNonPcnt (MachineType.h).
//
//  EP_Install BRANCHES (golden: 1 / 3 / 5 = ADAM via ADAMTCP, 2 = 10-bit digital EP through SW[SwEP_D0..+9],
//  4 = "PISO DA" = ET-7226 over the form's TClientSocket, port 10001)
//    1 / 3 / 5  golden, through the ADAMTCP shim (HT9050: EP_Install=3, ADAM-6024 at 172.16.8.110).
//    2          golden (SW[] outputs are real in the port).  Open_ADAM_6024 still opens the ADAM for 2, as golden does.
//    4          [W906] E4 -- the port has no ET-7226 binding: Open_ADAM_6024 takes golden's own "not installed"
//               answer (`return false`, golden :391-394) and says so once on stdout; ClientSocket1 stays a vclcompat
//               SIM socket (no OS socket), so WriteAO / CloseSocket stay inert.  LISTED for the human review.
//    0          golden (:391-394 / :414 / :443 / :1817 ...: nothing happens, false / 0).
//
//  SOFT_SIMULTE (the simulation build; MachineType.h:63-64): golden's own arms -- Open_ADAM_6024 :395-396 returns true
//  with no I/O, Close_ADAM_6024 does nothing, fCheck* :2912 / :2932 / :2962 return their SIM answers,
//  ADAM_ReadAIValue :2157-2159 uses 5.87654.  The paths golden does not gate (ADAM_WriteVoltage, ADAM_DirectWriteData,
//  the module queries) still call the shim, which never binds the vendor DLL in a SIM build ([W906] S1 in
//  AdamTcp_St02.h) -> ADAMTCP_StartupFailure, the answer golden gets on a PC without the module.
//
//  [W906] DEVIATIONS
//    E1 golden `fAdam6024->ADAMErrorMessage[iRet]` (iRet = 0-ret, :282 / :368 / :369) reads past the 16-entry table
//       for any code outside 0..-15 (the DLL has -100 = ADAMTCP_EventError, and HS_Function.cpp:1167 shows 817) --
//       undefined behaviour in golden.  W906_Adam6024_ErrorText(iRet) returns the golden entry for 0..15 and
//       "ADAMTCP error (<code>)" otherwise (also when fAdam6024 was never built).  Same text for every golden code.
//    E2 ClientSocket1Error :2068 `Abort();` = VCL's silent EAbort: the handler stops there and the VCL loop swallows
//       it, so golden's own :2070-2077 never runs.  Translated as `return;`, the dead golden lines kept under #if 0.
//    E3 WriteAO :2091-2105 `*(unsigned int *) &Buff[n]=...` (type-punned, unaligned 4-byte stores) -> std::memcpy of
//       the same unsigned int at the same offsets: the same 24 bytes on little-endian x86, no aliasing UB.
//    E4 EP_Install==4 (above).
//    E5 Set/Get*Range `char *ipAddr` -> `const char *ipAddr` (vclcompat AnsiString::c_str() is const; golden passes
//       IP.c_str()); golden `string` -> std::string.
//    E6 golden `fContactForce->fShow` (:1822, the ContactForce form is not ported) -> W906_FormShowing("fContactForce",
//       false) (W906FormShowing.h; page-table row WebPageTable.cpp:108), as atester_shims.h:266 asks.
//    E7 TfAdam6024: no TForm base, no __fastcall (Adam6024_St02.h D1-D3); the ctor also does what adam6024.dfm does
//       (ClientSocket1 Address / Port / OnConnect / OnError, ClientType ctNonBlocking has no vclcompat equivalent) and
//       what VCL's TObject::InitInstance does (cmdBuf / bufIdx start zeroed); the destructor runs OnDestroy.
//    E8 local re-declarations (no default argument is declared twice in this TU): MyDBIProcess 3-arg (cMyDB.h:81),
//       NewRecordProcess (cMyDB.h:129), LogClientSocketExceptionError (golden Public/WinSocketErrorCode.h:8; the
//       port header leaves it out), MySleepEx (common.h's is gated).
//
//    E9 (H4 integration pass 20261002) the EP live switch: ADAM_WriteVoltage / ADAM_DirectWriteData / ADAM_ReadAIValue first ask
//       W906_AdamEpLive() (Adam6024Integrate_St02.cpp; MachineType.h W906_ADAM_EP_LIVE).  OFF = the answers of the retired
//       stand-ins (atester_shims.cpp:326-327 no-op; TriTemp.cpp W7TT_ADAM_ReadAIValue false, out-params untouched).
//
//  GOLDEN QUIRKS KEPT VERBATIM (noted at each line)
//    Q1 Get*Range `i_byRange=0;` (:164 / :217) assigns the POINTER when CHECK_EP_SETTING==0 -- the caller's byte is
//       untouched (rangeReadBack stays 0) and true is returned.
//    Q2 the AO range check compares the read-back with the AI constant Adam6024_AI_mA_4To20 (7) (:343), so an AO
//       channel already at 4-20 mA (1) is rewritten every time the range check runs.
//    Q3 a failed range read/set returns false at once and skips ADAMTCP_UDPClose (:295-299 ...).
//    Q4 ADAMTCP_Connect failure (:361-372): iCount starts at 90 and the alarm needs iCount>100, so the first ten
//       failures fall through to `bADAM6420Install=true; return true;`; a success sets iCount=50 (the next failure
//       series alarms after 51).
//    Q5 ADAM_ReadVoltage :463-471 reconnects everything (Close_ADAM_6024 + Open_ADAM_6024()) on every failed read.
//       AI(W906-W191) 20261009 (St02-E): golden 913 adam6024.cpp:467-482 (RogerYang 20260914) -- once Open_ADAM_6024() itself
//       has failed, the next reconnects wait a 5 s cooldown (each try blocks up to connect / send / receive 2000 ms on the IO
//       page's 50 ms timer); the first failure still reconnects at once.  Close still runs every time, so bADAM6420Install
//       goes false and the :560 early return holds the reads until something reopens -- as in 0618 and 913.
//    Q6 ADAM_ReadAIValue :2145 keeps going when bADAM6420Install==false (only bEnter is cleared) and the dew-point
//       slopes `60/8` / `50/8` are INTEGER divisions (7 / 6), not 7.5 / 6.25.
//    Q7 GetModuleName keeps the address digit: "!016024-D" -> SubString(3, ..) = "16024-D", the string
//       fCheckModuleName_ADAM6024 compares with (:2915).
//    Q8 ClearAllConnection sends the same "%01GETMBTCPCN" query as GetModuleConnectionCount (:2849): it clears
//       nothing, it only checks the '!' reply.  GetModuleHostIdleTime builds that query and never sends it.
//    Q9 fCheckConnectStatus_ADAM6024 :2990 ADAMTCP_Disconnect() drops EVERY module connection (the DLL's Disconnect
//       loops over all of them), not only Address[Num]'s.
//    Q10 ADAM_DirectWriteData :1990-1991 turns 1250 / 1251 / 1252 into 1253 ("EP does not move", Ifor 20241030).
// =============================================================================
#define ADAM6024_ST02_INTERNAL      // Adam6024_St02.h: the cross-TU file-scope names below are defined HERE
#include "Adam6024_St02.h"          // golden 912 adam6024.h declarations + TfAdam6024
#include "adam6024.h"               // TransformFuntion (golden :29, the laptop's file)
#include "Public/AdamTcp_St02.h"    // ADAMTCP_* (ADAMTCP.dll run-time shim) + ADAMTCP_NoError / _BI_10V / _UNI_4TO20mA

#include "cmydef.h"                 // EP_Install, INSTALL_DOUBLE_EP, CHECK_EP_SETTING, USE_CKD_FCM_CleanAir, EP_*, SwEP_D0,
                                    // Tri_Temp_Machine, MachineTypeChoice, DewPoint_Hardware_Install, bADAM6024FWIsNew,
                                    // LogSoftwareOffTime; cprod.h (TestIF_File, DeviceForm_File)
#include "MachineType.h"            // SOFT_SIMULTE, Type_HT1032, CheckRange, ChangeToFloatNonPcnt
#include "myswitch.h"               // SW[] (TMySwitch)
#include "canary_support.h"         // ShowMyMessage (golden mymessbox.h:58)
#include "EJ1N/TextProcess.h"       // swap16, float2hex (golden EJ1N/TextProcess.h)
#include "W906FormShowing.h"        // [W906] E6

#include <cstdio>                   // sprintf / sscanf / printf
#include <cstdlib>                  // atoi / atof
#include <cstring>                  // memset / strcpy / memcpy
#include <string>                   // std::string (golden `string`)

// ---- [W906] E8: local declarations (bodies elsewhere, see the banner) --------------------------------------------
void __fastcall MyDBIProcess(AnsiString asTable, AnsiString S1, AnsiString S2="");   // golden cMyDB.h:20; port cMyDB.h:81
void NewRecordProcess(AnsiString AlarmCode, AnsiString S, AnsiString Debug=" ");      // golden cMyDB.h:62; port cMyDB.h:129
void LogClientSocketExceptionError(TObject *Sender, AnsiString Msg);                  // golden Public/WinSocketErrorCode.h:8
DWORD MySleepEx(DWORD dwMilliseconds, bool bAlertable);                               // golden common.h:260; body common.cpp

#define DEFAULT_PORT 502                                                        // Port for Modbus/TCP
#define DEFAULT_ET7226_PORT 10001                                               // Port for Modbus/TCP
//---------------------------------------------------------------------------
// golden 912 :32-55 (906 the same) -- the comm-layer globals.  :43 cAddress / :50-52 -> H3; :48-49 -> H2; :56 -> H2.
TfAdam6024 *fAdam6024;                                                          // :32 -- created by W906_AdamFormShowOpen (Adam6024Integrate_St02.cpp [W906] D7) at the golden FormShow :9890 position; golden HT9045.cpp:239 CreateForm

unsigned char NetID=0x01;
//-------- default timeout ------
int     iConnectionTimeout=2000;
int     iSendTimeout=2000;
int     iReceiveTimeout=2000;
//---------------------------------------------------------------------------
bool bADAM6420Install=false,bOpen=false;
//AnsiString Address="172.16.8.110";
AnsiString Address[3]={"172.16.8.110", "172.16.8.111", "172.16.8.112"};         //Ifor 20150728 :修改ADAM6024 IP位置為陣列變數
double fValue[16];
WORD wGain[10], wHex[16];
bool bCanReadData=true;                                                         //20111217 ChungHung
int iWritePA=0;                                                                 //20111217 ChungHung
bool bConnectStatus[3]={false, false, false};;                                  //Hmy 20170120 add check Adam6024 Connect Status   (golden's ";;" kept)
bool bADAM6420CheckRange[4]={false, false, false, false};                       //ben 20230818 : ADAM check range
static bool bDoubleEPConnectGuideShown=false;                                   //AI(ht9045-v899) 20260505: show EP board setup guidance once per run when Num=2 cannot connect
static bool bADAMReopenFailed=false;                                            //AI(W906-W191) 20261009 (St02-E): golden 913 adam6024.cpp:56 (RogerYang 20260914) -- only a CONSECUTIVE reopen failure is throttled
static TQPF_Timer hADAMReopenCooldown;                                          //AI(W906-W191) 20261009 (St02-E): golden 913 adam6024.cpp:57 -- the 5 s reopen cooldown
typedef enum
{
    Adam6024_AI_mA_4To20    = 7,
    Adam6024_AI_V_Neg10To10 = 8,
    Adam6024_AI_mA_0To20    = 13,
    Adam6024_AI_Unknown     = 255
} Adam6024_AI_Range;

typedef enum
{
    Adam6024_AO_mA_0To20    = 0,
    Adam6024_AO_mA_4To20    = 1,
    Adam6024_AO_V_0To10     = 2,
    Adam6024_AO_Unknown     = 255
} Adam6024_AO_Range;

// golden :142-246 Set/Get*Range have no prototype in golden; Adam6024_St02.h's internal section declares them (ctest).
//---------------------------------------------------------------------------
// [W906] E1 -- golden `fAdam6024->ADAMErrorMessage[iRet]` with the table bound checked (Adam6024_St02.h internals).
AnsiString W906_Adam6024_ErrorText(int iRet)
{
    if(fAdam6024!=NULL && iRet>=0 && iRet<16)
        return fAdam6024->ADAMErrorMessage[iRet];
    AnsiString s;
    s.sprintf("ADAMTCP error (%d)", 0-iRet);
    return s;
}
//---------------------------------------------------------------------------
// golden 912 :73-93 (906 :73-93, never called there; 912 calls it from Open_ADAM_6024 :266 / :368)
static void ShowDoubleEPConnectGuide(AnsiString IP, int Num, AnsiString Detail) //AI(ht9045-v899) 20260505: guide field engineer without changing Gerneral.ini automatically
{
    if(Num!=2 || bDoubleEPConnectGuideShown)
        return;

    if(INSTALL_DOUBLE_EP!=DOUBLE_EP_INDIVIAL && INSTALL_DOUBLE_EP!=DOUBLE_EP_MULTI)
        return;

    bDoubleEPConnectGuideShown=true;
    AnsiString Msg;
    if(INSTALL_DOUBLE_EP==DOUBLE_EP_MULTI)
    {
        Msg.sprintf("Double EP board connect failed.\r\nMode=3 is Multi EP (Max Qual Site), not normal left/right independent EP.\r\nThis mode needs the Multi EP APAX board. Expected IP=%s.\r\nIf this machine only needs left/right independent EP, field engineer should set INSTALL_DOUBLE_EP=2.\r\nIf the board exists, check board IP, network segment, cable and power.\r\n%s", IP.c_str(), Detail.c_str());
    }
    else
    {
        Msg.sprintf("Double EP board connect failed.\r\nMode=2 is Individual EP. Expected IP=%s.\r\nIf the board exists, check board IP, network segment, cable and power.\r\nIf the machine has no second EP board, field engineer should select the proper Dual EP Control mode.\r\n%s", IP.c_str(), Detail.c_str());
    }
    NewRecordProcess("", Msg);
    ShowMyMessage(Msg);
}
//---------------------------------------------------------------------------
// golden 912 :122-140 (906 :122-140) + adam6024.dfm :18-27 + [W906] E7
TfAdam6024::TfAdam6024(TComponent* Owner)
    : ClientSocket1(new TClientSocket(NULL))                                    // [W906] E7: .dfm component; Owner = the form -> NULL (no TComponent form here)
    , bufIdx(0)                                                                 // [W906] E7: VCL InitInstance zero-fill
{                                                                               //JerryYang 20170301 (wei) 修改ADAMErrorMessage陣列位置
    (void)Owner;
    std::memset(cmdBuf, 0, sizeof(cmdBuf));                                     // [W906] E7: VCL InitInstance zero-fill
    ClientSocket1->Address="172.16.8.110";                                      // adam6024.dfm:20
    ClientSocket1->Port=10001;                                                  // adam6024.dfm:22 (ClientType = ctNonBlocking :21 -- no vclcompat equivalent)
    ClientSocket1->OnConnect=[this](TObject *Sender, TCustomWinSocket *Socket)  // adam6024.dfm:23
        { ClientSocket1Connect(Sender, Socket); };
    ClientSocket1->OnError=[this](TObject *Sender, TCustomWinSocket *Socket,
                                  TErrorEvent ErrorEvent, int &ErrorCode)       // adam6024.dfm:24
        { ClientSocket1Error(Sender, Socket, ErrorEvent, ErrorCode); };

    ADAMErrorMessage[ 0]="";
    ADAMErrorMessage[ 1]="ADAM5KTCP_StartupFailure (-1)";
    ADAMErrorMessage[ 2]="ADAM5KTCP_SocketFailure (-2)";
    ADAMErrorMessage[ 3]="ADAM5KTCP_UdpSocketFailure (-3)";
    ADAMErrorMessage[ 4]="ADAM5KTCP_SetTimeoutFailure (-4)";
    ADAMErrorMessage[ 5]="ADAM5KTCP_SendFailure (-5)";
    ADAMErrorMessage[ 6]="ADAM5KTCP_ReceiveFailure (-6)";
    ADAMErrorMessage[ 7]="ADAM5KTCP_ExceedMaxFailure (-7)";
    ADAMErrorMessage[ 8]="ADAM5KTCP_CreateWsaEventFailure (-8)";
    ADAMErrorMessage[ 9]="ADAM5KTCP_ReadStreamDataFailure (-9)";
    ADAMErrorMessage[10]="ADAM5KTCP_InvalidIP (-10)";
    ADAMErrorMessage[11]="ADAM5KTCP_ThisIPNotConnected (-11)";
    ADAMErrorMessage[12]="ADAM5KTCP_AlarmInfoEmpty (-12)";
    ADAMErrorMessage[13]="ADAM5KTCP_NotSupportModule (-13)";
    ADAMErrorMessage[14]="ADAM5KTCP_ExceedDONo (-14)";
    ADAMErrorMessage[15]="ADAM5KTCP_InvalidRange (-15)";
}
//---------------------------------------------------------------------------
// [W906] E7: golden OnDestroy = FormDestroy (adam6024.dfm:15); the VCL owner then frees ClientSocket1.
TfAdam6024::~TfAdam6024()
{
    FormDestroy(NULL);
    delete ClientSocket1;
    ClientSocket1=NULL;
}
//---------------------------------------------------------------------------
// golden 912 :142-158 (906 :142-158)
bool SetAiInputRange(const char *ipAddr, int i_iChannel, byte i_byRange)        //ben 20230818 : ADAM check range   [W906] E5
{
    char szSend[128]={0},szRecv[128]={0};
    int iRet;

    sprintf(szSend, "$01A%02X%02X\r", i_iChannel,i_byRange);

    iRet=ADAMTCP_SendReceive6KUDPCmd(ipAddr, szSend, szRecv);
    if(iRet==0)
    {
        return true;
    }
    else
    {
        return false;
    }
}
//---------------------------------------------------------------------------
// golden 912 :160-193 (906 :160-193)
bool GetAiInputRange(const char *ipAddr, int i_iChannel, byte* i_byRange)       //ben 20230818 : ADAM check range   [W906] E5
{
    if(CHECK_EP_SETTING==0)                                                     //Steven 20240701 : EP檢查功能加上開關
    {
        i_byRange=0;                                                            // golden Q1: assigns the POINTER, the caller's byte is untouched
        return true;
    }

    char szSend[128]={0},szRecv[128]={0};
    bool rtnValue=false;
    int iRet;

    sprintf(szSend, "$01B%02X\r", i_iChannel);

    iRet=ADAMTCP_SendReceive6KUDPCmd(ipAddr, szSend, szRecv);
    if(iRet==0)
    {
        std::string recvStr=szRecv;                                             // [W906] E5 std::string
        if(recvStr.size()>4)
        {
            std::string szValue=recvStr.substr(3, recvStr.size()-3);
            if(szValue.size()==2)
            {
                unsigned int tmp = 0;
                if(sscanf(szValue.c_str(), "%2x", &tmp)==1)
                {
                    *i_byRange=static_cast<byte>(tmp);
                    rtnValue=true;
                }
            }
        }
    }
    return rtnValue;
}
//---------------------------------------------------------------------------
// golden 912 :195-211 (906 :195-211)
bool SetAoOutputRange(const char *ipAddr, int i_iChannel, byte i_byRange)       //ben 20230818 : ADAM check range   [W906] E5
{
    char szSend[128]={0}, szRecv[128]={0};
    int iRet;

    sprintf(szSend, "$01C%02X%02X\r", i_iChannel, i_byRange);

    iRet=ADAMTCP_SendReceive6KUDPCmd(ipAddr, szSend, szRecv);
    if(iRet==0)
    {
        return true;
    }
    else
    {
        return false;
    }
}
//---------------------------------------------------------------------------
// golden 912 :213-246 (906 :213-246)
bool GetAoOutputRange(const char *ipAddr, int i_iChannel, byte* i_byRange)      //ben 20230818 : ADAM check range   [W906] E5
{
    if(CHECK_EP_SETTING==0)                                                     //Steven 20240701 : EP檢查功能加上開關
    {
        i_byRange=0;                                                            // golden Q1: assigns the POINTER
        return true;
    }

    char szSend[128]={0}, szRecv[128]={0};
    bool rtnValue=false;
    int iRet;

    sprintf(szSend, "$01C%02X\r", i_iChannel);

    iRet=ADAMTCP_SendReceive6KUDPCmd(ipAddr, szSend, szRecv);
    if(iRet==0)
    {
        std::string recvStr=szRecv;                                             // [W906] E5 std::string
        if(recvStr.size()>4)
        {
            std::string szValue=recvStr.substr(3, recvStr.size()-3);
            if(szValue.size()==2)
            {
                unsigned int tmp = 0;
                if(sscanf(szValue.c_str(), "%2x", &tmp)==1)
                {
                    *i_byRange=static_cast<byte>(tmp);
                    rtnValue=true;
                }
            }
        }
    }
    return rtnValue;
}
//---------------------------------------------------------------------------
// golden 912 :248-398 (906 :248-394)
bool Open_ADAM_6024(AnsiString IP, int Num)                                     //Ifor 20150709 :修改可使用多台ADAM6024
{
#ifndef SOFT_SIMULTE
    AnsiString Str;
    int iRet;
    static int iCount=90;                                                       // golden Q4

    int iChannel;                                                               //ben 20230818 : ADAM check range
    int adam6024AiChannelTotal=6;
    int adam6024AoChannelTotal=2;
    byte rangeCode;
    byte rangeReadBack;

    if(EP_Install>0)
    {
        Address[Num]=IP;                                                        // [912] :263 (906 :266 set it AFTER the status / FW checks)  //AI(ht9045-v899) 20260505: set target IP before status check so guidance uses the field-selected address
        if(!fCheckConnectStatus_ADAM6024(Num))                                  //Hmy 20170120 add check Adam6024 Connect Status ->
        {                                                                       // [912] :265-268 braces + the guide (906 :263-264: `return false;` only)
            ShowDoubleEPConnectGuide(IP, Num, "Status check failed before TCP connect.");
            return false;
        }
        bADAM6024FWIsNew[Num]=fCheckModuleFWISNew_ADAM6024(Num);                //Nickliu 20230314 Add Check Adam FW Is New
        if(EP_Install==4)                                                       //Steven 20141202 : PISO DA
        {
            // golden :272  fAdam6024->OpenSocket(IP, DEFAULT_ET7226_PORT);
            // [W906] E4: no ET-7226 binding in the port -> golden's own "not installed" answer (:391-394).
            static bool bW906PisoNoted=false;
            if(!bW906PisoNoted)
            {
                bW906PisoNoted=true;
                std::printf("  [ADAM] EP_Install=4 (PISO DA / ET-7226, TCP %s:%d) is not wired in V906 -- Open_ADAM_6024 returns false\n",
                            IP.c_str(), DEFAULT_ET7226_PORT);
            }
            return false;
        }
        else
        {
            if(Num==0)                                                          //Ifor 20160301 避免使用一顆以上ADAM出現異常
            {
                iRet=ADAMTCP_Open();
                if(iRet!=0)                                                     //Steven 20210505 : 針對Adam連線異常加上紀錄
                {
                    iRet=0-iRet;
                    MyDBIProcess("Motion", "ADAMTCP_Open Fail!", W906_Adam6024_ErrorText(iRet));   // [W906] E1 (golden: fAdam6024->ADAMErrorMessage[iRet])
                    return false;
                }
            }

            if(Num!=2 &&                                                        //Steven 20231026 : 不檢查APAX
               bADAM6420CheckRange[Num]==false)                                 //ben 20230818 : ADAM check range
            {
                if(ADAMTCP_UDPOpen(iSendTimeout, iReceiveTimeout)==0)
                {
                    for(iChannel=0; iChannel<adam6024AiChannelTotal; iChannel++)
                    {
                        rangeReadBack=0;                                        //Get AI Input Range
                        if(GetAiInputRange(IP.c_str(), iChannel, &rangeReadBack)!=true)
                        {
                            Str.printf("Failed to Get ADAM AI Range! IP=%s, Channel=%d, Readrange=%d", IP, iChannel, rangeReadBack);
                            ShowMyMessage(Str);
                            return false;                                       // golden Q3: no ADAMTCP_UDPClose
                        }

                        if(iChannel==3 ||
                           (MachineTypeChoice==Type_HT1032 &&
                            Tri_Temp_Machine==1 &&
                            (iChannel==2 || iChannel==3 || iChannel==4)))       //Ztex For Type_HT1032
                        {
                            if(rangeReadBack!=Adam6024_AI_mA_4To20)
                            {
                                rangeCode=Adam6024_AI_mA_4To20;                 //Set AI Input Range
                                if(SetAiInputRange(IP.c_str(), iChannel, rangeCode)!=true)
                                {
                                    Str.printf("Failed to Set ADAM AI Range! IP=%s, Channel=%d, Setrange=%d", IP, iChannel, rangeCode);
                                    ShowMyMessage(Str);
                                    return false;
                                }
                            }
                        }
                        else
                        {
                            if(rangeReadBack!=Adam6024_AI_V_Neg10To10)
                            {
                                rangeCode=Adam6024_AI_V_Neg10To10;              //Set AI Input Range
                                if(SetAiInputRange(IP.c_str(), iChannel, rangeCode)!=true)
                                {
                                    Str.printf("Failed to Set ADAM AI Range! IP=%s, Channel=%d, Setrange=%d", IP, iChannel, rangeCode);
                                    ShowMyMessage(Str);
                                    return false;
                                }
                            }
                        }
                    }

                    for(iChannel=0; iChannel<adam6024AoChannelTotal; iChannel++)
                    {
                        rangeReadBack=0;                                        //Get AI Input Range
                        if(GetAoOutputRange(IP.c_str(), iChannel, &rangeReadBack)!=true)
                        {
                            Str.printf("Failed to Get ADAM AO Range! IP=%s, Channel=%d, Readrange=%d", IP, iChannel, rangeReadBack);
                            ShowMyMessage(Str);
                            return false;
                        }

                        if(rangeReadBack!=Adam6024_AI_mA_4To20)                 // golden Q2: the AI constant (7), not Adam6024_AO_mA_4To20 (1)
                        {
                            rangeCode=Adam6024_AO_mA_4To20;                     //Set AO Output Range
                            if(SetAoOutputRange(IP.c_str(), iChannel, rangeCode)!=true)
                            {
                                Str.printf("Failed to Set ADAM AO Range! IP=%s, Channel=%d, Setrange=%d", IP, iChannel, rangeCode);
                                ShowMyMessage(Str);
                                return false;
                            }
                        }
                    }
                }
                ADAMTCP_UDPClose();
                bADAM6420CheckRange[Num]=true;
            }

            iRet=ADAMTCP_Connect(IP.c_str(), DEFAULT_PORT, iConnectionTimeout, iSendTimeout, iReceiveTimeout);

            if(iRet!=0)                                                         //Frank 20170206 (Steven) 確認ADAM連線正常
            {
                iCount++;
                iRet=0-iRet;                                                    //JerryYang 20170301 (wei) 修改ADAMErrorMessage陣列位置
                if(iCount>100)                                                  //jou 20170313 (Steven) : Adam EP check alarm 3 -> 100
                {
                    iCount=0;
                    ShowDoubleEPConnectGuide(IP, Num, W906_Adam6024_ErrorText(iRet));   // [912] :368 (no 906 line)  [W906] E1
                    ShowMyMessage("Connect Fail! Please Check ADAM IP!", IP, W906_Adam6024_ErrorText(iRet));          //wei 20160329 ADAM連線異常Alarm   [W906] E1
                    return false;
                }
            }
            else
            {
                iCount=50;                                                      //Steven 20170314 (Jou) : 提早Alarm 0 --> 50
            }
        }

        for(int i=0; i<10; i++)
            wGain[i]=ADAMTCP_BI_10V;                                            // the gain code for channel:3

        if(MachineTypeChoice==Type_HT1032 && Tri_Temp_Machine==1)               //Ztex 2023.04.19 Add HT-1032 TriTemp Function
        {
            wGain[2]=ADAMTCP_UNI_4TO20mA;
            wGain[3]=ADAMTCP_UNI_4TO20mA;
            wGain[4]=ADAMTCP_UNI_4TO20mA;
        }
        bADAM6420Install=true;
        return true;
    }
    else
    {
        return false;
    }
#else
    (void)IP; (void)Num;                                                        // [W906] -Wunused-parameter hygiene only
    return true;
#endif
}
//---------------------------------------------------------------------------
// golden 912 :400-409 (906 :396-405)
bool Open_ADAM_6024()                                                           //Jimmychiu 20230804 : 整合全部連線檢查
{
    bool bflag1=true, bflag2=true, bflag3=true;
    bflag1=Open_ADAM_6024("172.16.8.110", 0);
    if(USE_CKD_FCM_CleanAir)
        bflag2=Open_ADAM_6024("172.16.8.111", 1);
    if(INSTALL_DOUBLE_EP==DOUBLE_EP_INDIVIAL || INSTALL_DOUBLE_EP==DOUBLE_EP_MULTI)                            //AI(ht9045-v899) 20260610: also open the 2nd APAX board (Address[2]) for Multi EP mode 3.
        bflag3=Open_ADAM_6024("172.16.8.112", 2);
    return (bflag1 && bflag2 && bflag3);
}
//---------------------------------------------------------------------------
// golden 912 :411-431 (906 :407-427)
void Close_ADAM_6024()
{
#ifndef SOFT_SIMULTE
    if(EP_Install)
    {
        if(bADAM6420Install)
        {
            bADAM6420Install=false;
            if(EP_Install==4)                                                   //Steven 20141202 : PISO DA
            {
                fAdam6024->CloseSocket();
            }
            else
            {
                ADAMTCP_Disconnect();
                ADAMTCP_Close();                                                // shim [W906] S3: forwarded only while an Open is outstanding
            }
        }
    }
#endif
}
//---------------------------------------------------------------------------
// golden 912 :433-505 (906 :429-501)
double ADAM_ReadVoltage(int Num, int iCH)                                       //Ifor 20150709 :修改可使用多台ADAM6024    //Ifor 20190104 : add 讀取ADAM CH 數值
{
    int iRet;
    static bool bEnter=false;
    if(bEnter)
    {
        return 0;
    }
    bEnter=true;

    if(EP_Install)
    {
        if(bADAM6420Install==false)
        {
            bEnter=false;
            return 0;
        }
#ifdef DEBUG_TRY_CATCH
        try
        {
#endif
            if(EP_Install==4)                                                   //Steven 20141202 : PISO DA
            {
                bEnter=false;
            }
            else
            {
                if(!bConnectStatus[Num])                                        //Hmy 20170120 add check Adam6024 Connect Status
                    bConnectStatus[Num] = fCheckConnectStatus_ADAM6024(Num);    //Nickliu 20230330 add check statsu

                iRet=ADAMTCP_Read6KAI(Address[Num].c_str(), 6017, 1, wGain, wHex, fValue);

                if(iRet!=0)                                                     //Ifor 20150709 ：透過ADAM陣列IP讀取資料   //Frank 20170206 (Steven) 確認ADAM連線正常
                {
                    Close_ADAM_6024();                                          // golden Q5   //AI(W906-W191) 20261009 (St02-E): golden 913 adam6024.cpp:467-482 from here
                    if(bADAMReopenFailed==false || hADAMReopenCooldown.Off())   //AI(ht9045-io-control) 20260914 (RogerYang) : a reopen blocks connect / send / receive 2000 ms each on the IO page's 50 ms timer
                    {
                        if(Open_ADAM_6024()==true)                              //Jimmychiu 20230804 : 整合全部連線檢查
                        {
                            bADAMReopenFailed=false;
                        }
                        else
                        {
                            bADAMReopenFailed=true;
                            hADAMReopenCooldown.SetMSAndOn(5000);               //AI(ht9045-io-control) 20260914 (RogerYang) : consecutive failures retry once every 5 s
                        }
                    }
                    bEnter=false;
                    return 0;
                }
            }

            bEnter=false;
            if(iCH==3)                                                          //Isaac 20191024 : add for Dew Point
            {
                if(Tri_Temp_Machine==1)                                         //1032 機型露點計使用 2、3、4 CH
                {
                    return fValue[iCH];
                }
                else
                {
                    return (wHex[3]/4095.9375+4);
                }
            }
            else
            {
                return fValue[iCH];                                             //Ifor 20190104 : add 讀取ADAM CH 數值
            }
#ifdef DEBUG_TRY_CATCH
        }
        catch(...)
        {
            bEnter=false;
            MyDBIProcess("Exception", "ADAM_ReadVoltage");
            return 0;
        }
#endif
    }
    else
    {
        bEnter=false;
        return 0;
    }
}
//---------------------------------------------------------------------------
// golden 912 :507-526 (906 :503-522)
int ADAM_ReadPA(double *dValue, int iCH)                                        //Ifor 20190104 : add   //wei 20220309 Add EP Return Voltage
{
    double v=ADAM_ReadVoltage(0, iCH);                                          //Ifor 20190104 : add 讀取ADAM CH 數值

    *dValue=v;                                                                  //wei 20220309 Add EP Return Voltage
    if(EP_MAXAFB==0.0 || EP_MinAFB==0.0)
        return v;
    if(EP_MAXAFB==EP_MinAFB)                                                    //Steven 20260505 : add zero-guard for (EP_MAXAFB-EP_MinAFB)
        return v;

                                                                                //Steven 20170721 (wei) : 修正EP回傳的內差法公式  //JerryYang 20171023 (wei) 修正電壓轉PA的公式
    int PA=EP_MINMPA*1000.0+(EP_MAXKPA-EP_MINMPA*1000.0)*ChangeToFloatNonPcnt((double)(v-EP_MinAFB), (double)(EP_MAXAFB-EP_MinAFB)); //Steven 20260505 : add zero-guard
    if(iCH==2)                                                                  //kevin 20200325 add dual force data
    {
        if(EPDual_MAXAFB==EPDual_MinAFB)                                        //Steven 20260505 : add zero-guard for (EPDual_MAXAFB-EPDual_MinAFB)
            return v;
        PA=EPDual_MINMPA*1000.0+(EPDual_MAXKPA-EPDual_MINMPA*1000.0)*ChangeToFloatNonPcnt((double)(v-EPDual_MinAFB), (double)(EPDual_MAXAFB-EPDual_MinAFB)); //Steven 20260505 : add zero-guard
    }
    return PA;
}
//---------------------------------------------------------------------------
// golden 912 :1814-1931 (906 :1798-1915)
bool ADAM_WriteVoltage(double v)
{
    if(!W906_AdamEpLive())                                                      // [W906] E9 EP live switch OFF: the retired stand-in atester_shims.cpp:327 (no-op; no caller reads the value)
        return false;
    WORD data;
    if(EP_Install)
    {
        if(!bConnectStatus[0])                                                  //Hmy 20170120 add check Adam6024 Connect Status ->
            bConnectStatus[0]=fCheckConnectStatus_ADAM6024(0);                  //Nickliu 20230330 add check statsu

        if(W906_FormShowing("fContactForce", false))                            // [W906] E6 (golden :1822 `fContactForce->fShow`)
        {
            return true;
        }

        int iInputValue=0, iInValue[2]={0, 0};
        if(v<0)
        {
#ifdef DEBUG_TRY_CATCH
            try
            {
#endif
                if(EP_Install==4)                                               //Steven 20141202 : PISO DA
                {
                    ;
                }
                else
                {
                    ADAM_DirectWriteData(0, 0);                                 //Ifor 20150709 ：加入設備位置
                }
                return true;
#ifdef DEBUG_TRY_CATCH
            }
            catch(...)
            {
                MyDBIProcess("Exception", "ADAM_WriteVoltage");
                return false;
            }
#endif
        }
        else
        {
                if(EP_Install!=5)
                {
                    iInputValue=TransformFuntion(v);
                }
                else
                {
                    iInValue[0]=TransformFuntion(v, false, false, 0);
                    iInValue[1]=TransformFuntion(v, false, false, 1);
                }
    #ifdef DEBUG_TRY_CATCH
                try
                {
    #endif
                    if(IsIndependentEPPressureRouteActive()==true)              //AI(ht9045-v899) 20260526: use actual SwMultiEp route before APAX-only output.
                    {
                        TransformFuntion(v, false);
                        APAX_WriteData(false, 0);
                        if(EP_Install==1 || EP_Install==3 || EP_Install==5)     //20111111 Dell //20111217 ChungHung
                        {
                            data=0;
                            data=CheckRange(data, WORD(0), WORD(4095));
                            iWritePA=data;
                            ADAMTCP_WriteReg(Address[0].c_str(), 1, 12, 1, &data);//Ifor 20190129 : add 選擇CH輸出
                        }
                    }
                    else
                    {
                        if(EP_Install==1 ||
                           EP_Install==3 ||                                     //20111217 ChungHung
                           EP_Install==4)                                       //Steven 20141202 : PISO DA
                        {
                            ADAM_DirectWriteData(iInputValue, 0);               //Ifor 20150709 ：加入設備位置
                        }
                        else if(EP_Install==5)
                        {
                            if(TestIF_File.iShuttleMode==1 && TestIF_File.iShuttle_Sel==0)
                            {
                                ADAM_DirectWriteData(iInValue[0], 0, 10);
                            }
                            else if(TestIF_File.iShuttleMode==1 && TestIF_File.iShuttle_Sel==1)
                            {
                                ADAM_DirectWriteData(iInValue[1], 0, 11);
                            }
                            else
                            {
                                ADAM_DirectWriteData(iInValue[0], 0, 10);
                                ADAM_DirectWriteData(iInValue[1], 0, 11);
                            }
                        }
                        else if(EP_Install==2)                                  //20111111 Dell
                        {
                            WriteDigital(iInputValue);
                        }

                        if(IsIndependentEPPressureRouteActive()==true)          //AI(ht9045-v899) 20260526: do not clear/write APAX when Multi valve is OFF.
                        {
                            APAX_WriteData(true, 0);
                        }
                        iWritePA=iInputValue;                                   //20111217 ChungHung
                        iAdamOutValue=AdamOutputToPA(iWritePA);
                    }
                    return true;

    #ifdef DEBUG_TRY_CATCH
                }
                catch(...)
                {
                    MyDBIProcess("Exception", "ADAM_WriteVoltage");
                    return false;
                }
#endif
        }
    }
    else
    {
        return false;
    }
}
//------------------------------------------------------------------------------
// golden 912 :1933-1950 (906 :1917-1934)
void ADAM_WriteMaxData(bool bFullForce)                                         //Steven 20241014 : 整合auto height輸出壓力
{
    if(bFullForce)
    {
        if(DeviceForm_File.dKitDiameter<=2.5)                                   //kevin 20170804 (Steven) 20mm Auto Height
        {
            ADAM_DirectWriteData((EP_MAXKPA<=500)?820:455, 0);                  //充100kpa就好
        }
        else
        {
            ADAM_DirectWriteData((EP_MAXKPA<=500)?4095:2275, 0);                //Ifor 20150709 ：加入設備位置
        }
    }
    else
    {
        ADAM_DirectWriteData(0, 0);
    }
}
//------------------------------------------------------------------------------
// golden 912 :1952-2037 (906 :1936-2021)
void ADAM_DirectWriteData(WORD data, int Num, int iAdd)                         //Ifor 20150709 ：加入設備位置 //Ifor 20190104 : add 選擇 ADAM 輸出 CH
{
    if(!W906_AdamEpLive())                                                      // [W906] E9 EP live switch OFF: the retired stand-in atester_shims.cpp:326 (no-op)
        return;
    int iRet;
    bCanReadData=false;                                                         //20111217 ChungHung
    if(EP_Install)
    {
        if(!bConnectStatus[Num])                                                //Hmy 20170120 add check Adam6024 Connect Status ->
            bConnectStatus[Num]=fCheckConnectStatus_ADAM6024(Num);              //Nickliu 20230330 add check statsu
    }

    if(EP_Install==4)                                                           //Steven 20141202 : PISO DA
    {
        data=CheckRange(data, WORD(0), WORD(4095));
        fAdam6024->WriteAO(data);                                               // [W906] E4: ClientSocket1 is a SIM socket -> captured, never sent
    }
    else
    {
        if(IsIndependentEPPressureRouteActive()==true)                          //AI(ht9045-v899) 20260526: route direct writes to APAX only when independent EP path is active.
        {
            if(Num==0)
            {
                int iMultiEPArm=0;                                             //AI(ht9045-v899) 20260526: keep iosetview and production direct writes on the shared APAX route.
                if(INSTALL_DOUBLE_EP==DOUBLE_EP_MULTI)
                {
                    if(iAdd==10)
                        iMultiEPArm=1;
                    else if(iAdd==11)
                        iMultiEPArm=2;
                }
                APAX_WriteData(true, data, iMultiEPArm);
            }
        }
        else
        {
            if(EP_Install==1 || EP_Install==3)                                  //20111111 Dell //20111217 ChungHung
            {
                data=CheckRange(data, WORD(0), WORD(4095));

                if(data==1250 || data==1251 || data==1252)
                    data=1253;                                                  //Ifor 20241030 : EP不會動, 暫時先跳過這三個數字   (golden Q10)

                iWritePA=data;                                                  //JerryYang 20171030 (wei) fix EP問題
                iRet=ADAMTCP_WriteReg(Address[Num].c_str(), 1, (11+iAdd), 1, &data);//Ifor 20190129 : add 選擇CH輸出
                if(iRet!=0)                                                     //Frank 20170206 (Steven) 確認ADAM連線正常
                {
                    bConnectStatus[Num]=false;
                }

                if(IsIndependentEPPressureRouteActive()==true)                  //AI(ht9045-v899) 20260526: keep normal ADAM output untouched when SwMultiEp is OFF.
                {
                    APAX_WriteData(true, 0);
                }
            }
            else if(EP_Install==5)
            {
                data=CheckRange(data, WORD(0), WORD(4095));
                iWritePA=data;                                                  //JerryYang 20171030 (wei) fix EP問題

                if(iAdd==10)
                {
                    iRet=ADAMTCP_WriteReg(Address[Num].c_str(), 1, 11, 1, &data);
                }
                else if(iAdd==11)
                {
                    iRet=ADAMTCP_WriteReg(Address[Num].c_str(), 1, 12, 1, &data);
                }
                else
                {
                    iRet=ADAMTCP_WriteReg(Address[Num].c_str(), 1, 11, 1, &data);
                    iRet=ADAMTCP_WriteReg(Address[Num].c_str(), 1, 12, 1, &data);
                }

                if(iRet!=0)                                                     //Frank 20170206 (Steven) 確認ADAM連線正常
                {
                    bConnectStatus[Num]=false;
                }
            }
            else if(EP_Install==2)
            {
                data=CheckRange(data, WORD(0), WORD(1022));
                WriteDigital(data);
            }
        }
    }
    bCanReadData=true;                                                          //20111217 ChungHung
}
//---------------------------------------------------------------------------
// golden 912 :2039-2050 (906 :2023-2034)
//20111111 Dell For E/P Digital
const int Digital_10Bit = 10;
void WriteDigital(double v)
{
    int value=v;

    for(int i=0; i<Digital_10Bit; i++)
    {
        SW[SwEP_D0+i].OnOff(value%2);                                           //10進位 轉 2進位
        value=value>>1;
    }
}
//---------------------------------------------------------------------------
// golden 912 :2052-2062 (906 :2036-2046)
void TfAdam6024::ClientSocket1Connect(TObject *Sender,
      TCustomWinSocket *Socket)
{
    for(int i=0; i<8; i++)
    {
        cmdBuf[i].status=0;
        memset(cmdBuf[i].cmdBuf, 0x0, 512);
        cmdBuf[i].cmdSize=0;
    }
    bufIdx=0;
}
//---------------------------------------------------------------------------
// golden 912 :2064-2078 (906 :2048-2062)
void TfAdam6024::ClientSocket1Error(TObject *Sender,
      TCustomWinSocket *Socket, TErrorEvent ErrorEvent, int &ErrorCode)
{
    ErrorCode=0;
    return;                                                                     // [W906] E2: golden `Abort();` -- VCL EAbort, the rest never runs
#if 0   // golden :2070-2077, dead after Abort() in golden too
    try
    {
        ClientSocket1->Close();
    }
    catch(...)
    {
        LogClientSocketExceptionError(ClientSocket1, "ADAM ClientSocket1 Error");
    }
#endif
}
//---------------------------------------------------------------------------
// golden 912 :2080-2108 (906 :2064-2092)
void TfAdam6024::WriteAO(double AO1, double AO2, double AO3, double AO4)
{
    AnsiString strBuf, strbyte;
    unsigned hexValue;
    unsigned int uiStore;                                                       // [W906] E3
    char Buff[24];

    memset(Buff, 0x0, sizeof(Buff));
    Buff[0]=2;
    Buff[5]=15;
    Buff[6]=NetID;  // Net ID (Station number)
    Buff[7]=16;     // Function code: 16 -> Write multiple registers (4XXXX) for AO
    uiStore=0;                    std::memcpy(&Buff[8],  &uiStore, sizeof(uiStore));  // Reference number   [W906] E3 (golden *(unsigned int *) &Buff[8]=0;)
    uiStore=swap16(4);            std::memcpy(&Buff[10], &uiStore, sizeof(uiStore));  // Word count         [W906] E3
    Buff[12]=8;     // Byte count

    hexValue=float2hex(0x35, AO1, 0);
    uiStore=swap16(hexValue);     std::memcpy(&Buff[13], &uiStore, sizeof(uiStore));  // AO0 value          [W906] E3

    hexValue=float2hex(0x35, AO2, 0);
    uiStore=swap16(hexValue);     std::memcpy(&Buff[15], &uiStore, sizeof(uiStore));  // AO1 value          [W906] E3

    hexValue=float2hex(0x35, AO3, 0);
    uiStore=swap16(hexValue);     std::memcpy(&Buff[17], &uiStore, sizeof(uiStore));  // AO2 value          [W906] E3

    hexValue=float2hex(0x35, AO4, 0);
    uiStore=swap16(hexValue);     std::memcpy(&Buff[19], &uiStore, sizeof(uiStore));  // AO3 value          [W906] E3

    ClientSocket1->Socket->SendBuf(Buff, 21);
}
//---------------------------------------------------------------------------
// golden 912 :2110-2115 (906 :2094-2099)
void TfAdam6024::OpenSocket(AnsiString IP, int Port)
{
    ClientSocket1->Address=IP;
    ClientSocket1->Port=Port;
    ClientSocket1->Open();
}
//---------------------------------------------------------------------------
// golden 912 :2117-2127 (906 :2101-2111)
void TfAdam6024::CloseSocket()
{
    try
    {
        ClientSocket1->Close();
    }
    catch(...)
    {
        MyDBIProcess("Exception", "TfAdam6024::CloseSocket");
    }
}
//---------------------------------------------------------------------------
// golden 912 :2129-2204 (906 :2113-2188)
bool ADAM_ReadAIValue(int iNum, int iChannel, double *dmA, double *dDegree)
{
    if(!W906_AdamEpLive())                                                      // [W906] E9 EP live switch OFF: TriTemp.cpp's W7TT stand-in (false, out-params untouched)
        return false;
    static bool bEnter=false;
    bool bReturnStatus=false;
    double dValue=0.0;
    char str[256];
    if(bEnter)
    {
        return bReturnStatus;
    }
    bEnter=true;

    if(EP_Install)
    {
        if(bADAM6420Install==false)
        {
            bEnter=false;                                                       //ChungHung 20160315 modify   (golden Q6: no return)
        }
        try
        {
            if(EP_Install==4)                                                   //Steven 20141202 : PISO DA
            {
                ;
            }
            else
            {
                #ifndef SOFT_SIMULTE
                    dValue=ADAM_ReadVoltage(iNum, iChannel);
                #else                                                           //Hmy 20170930 Add  SOFT_SIMULTE return value
                    dValue=5.87654;
                    (void)iNum;                                                 // [W906] -Wunused hygiene only
                #endif

                sprintf(str, "%2.3f", dValue);
                double dCurrent_ma = atof(str);
                double dConvertTemperature =0.0;

                if(DewPoint_Hardware_Install==1)                                //-60~+60
                {
                    if(dCurrent_ma<=12.0)
                      dConvertTemperature=-60+(60/8*(dCurrent_ma-4));           // golden Q6: 60/8 is the INTEGER 7
                    else if(dCurrent_ma>12.0)
                      dConvertTemperature=60/8*(dCurrent_ma-12);
                }
                else if(DewPoint_Hardware_Install==2)                           //-80~+20
                {
                    if(dCurrent_ma<=12.0)
                      dConvertTemperature=-80+(50/8 *(dCurrent_ma-4));          // golden Q6: 50/8 is the INTEGER 6
                    else if(dCurrent_ma>12.0)
                      dConvertTemperature=-30+50/8*(dCurrent_ma-12);
                }
                else                                                            //無安裝
                {
                    dConvertTemperature=9999;
                }
                sprintf(str, "%2.1f", dConvertTemperature);
                dConvertTemperature=atof(str);
                *dmA=dCurrent_ma;
                *dDegree=dConvertTemperature;
                bReturnStatus=true;
                bEnter=false;                                                   //ChungHung 20160315 modify
            }
        }
        catch(...)
        {
            MyDBIProcess("Exception", "ADAM_ReadAIValue");
            bEnter=false;                                                       //ChungHung 20160315 modify
        }
    }
    else
    {
        bEnter=false;                                                           //ChungHung 20160315 modify
        bReturnStatus=false;
    }
    bEnter=false;                                                               //ChungHung 20160315 modify
    return bReturnStatus;
}
//---------------------------------------------------------------------------
// golden 912 :2206-2216 (906 :2190-2200)
void TfAdam6024::FormDestroy(TObject *Sender)
{
    try
    {
    }
    catch(...)
    {
        MyDBIProcess("Exception", "TfAdam6024::FormDestroy");
    }
    LogSoftwareOffTime("TfAdam6024, FormDestroy");                              //Steven 20210526 : 紀錄軟體執行時間
}
//---------------------------------------------------------------------------
// golden 912 :2761-2785 (906 :2759-2783)
//Nickliu 20230314 Add Check Adam FW Is New-->
AnsiString GetModuleName(int Num)
{
    char szSend[128], szRecv[128];
    memset(szRecv, '\0', sizeof(szRecv));
    memset(szSend, '\0', sizeof(szSend));
    int iRet=0;
    strcpy(szSend, "$01M\r");                                                   //Get module Name Command
    AnsiString sTemp="";
    AnsiString recvStr="";
    if(ADAMTCP_UDPOpen(1000, 1000)==0)
    {
        iRet=ADAMTCP_SendReceive6KUDPCmd(Address[Num].c_str(), szSend, szRecv);
        if(iRet==0)
        {
            if(szRecv[0]=='!')
            {
                AnsiString recvStr=szRecv;                                      // golden shadows the outer recvStr
                sTemp=recvStr.SubString(3, recvStr.Length());                   // golden Q7: keeps the address digit
            }
        }
    }
    ADAMTCP_UDPClose();
    return sTemp;
}
//---------------------------------------------------------------------------
// golden 912 :2787-2810 (906 :2785-2808)
//Nickliu 20230314 Add Check Fireware Name
AnsiString GetFirmwareName(int Num)                                             //Address[Num].c_str()
{
    char szSend[128], szRecv[128];
    memset(szRecv, '\0', sizeof(szRecv));
    memset(szSend, '\0', sizeof(szSend));
    strcpy(szSend, "$01F\r");                                                   //Get FW Version Command
    AnsiString sTemp="";
    AnsiString recvStr="";
    if(ADAMTCP_UDPOpen(1000, 1000)==0)
    {
        int iRet=ADAMTCP_SendReceive6KUDPCmd(Address[Num].c_str(), szSend, szRecv);
        if(iRet==0)
        {
            if(szRecv[0]=='!')
            {
                AnsiString recvStr=szRecv;                                      // golden shadows the outer recvStr
                sTemp=recvStr.SubString(5, recvStr.Length()-3);
            }
        }
    }
    ADAMTCP_UDPClose();
    return sTemp;
}
//Nickliu 20230314 Add Check Fireware Name
//---------------------------------------------------------------------------
// golden 912 :2813-2840 (906 :2811-2838)
//Nickliu 20230314 Add Get Module All Connection Count
int GetModuleConnectionCount(int Num)
{
    char szSend[128], szRecv[128];
    memset(szRecv, '\0', sizeof(szRecv));
    memset(szSend, '\0', sizeof(szSend));
    strcpy(szSend, "%01GETMBTCPCN\r");
    //int count=0;
    int count=-1;   //Nickliu 20230323 add .....
    int iRef=-1;
    if(ADAMTCP_UDPOpen(1000, 1000) == 0)
    {
        if(ADAMTCP_SendReceive6KUDPCmd(Address[Num].c_str(), szSend, szRecv)==0)
        {
            if(szRecv[0]=='!')
            {
                count=(int)(szRecv[2]-'0');
                iRef=count;
            }
            else
            {
                iRef=-1;   //Format Error
            }
        }
    }
    ADAMTCP_UDPClose();
    return iRef;
}
//Nickliu 20230314 Add Get Module All Connection Count
//---------------------------------------------------------------------------
// golden 912 :2843-2863 (906 :2841-2861)
//Nickliu 20230314 Add Clear All Connection
bool ClearAllConnection(int Num)
{
    char szSend[128], szRecv[128];
    memset(szRecv, '\0', sizeof(szRecv));
    memset(szSend, '\0', sizeof(szSend));
    strcpy(szSend, "%01GETMBTCPCN\r");                                          // golden Q8: the count query, nothing is cleared
    bool bRef=false;
    if(ADAMTCP_UDPOpen(1000, 1000)==0)
    {
        if(ADAMTCP_SendReceive6KUDPCmd(Address[Num].c_str(), szSend, szRecv)==0)
        {
            if(szRecv[0]=='!')
            {
                bRef=true;
            }
        }
    }
    ADAMTCP_UDPClose();
    return bRef;
}
//Nickliu 20230314 Add Clear All Connection
//---------------------------------------------------------------------------
// golden 912 :2866-2884 (906 :2864-2882) -- golden adam6024.h:41 declares `()`, this `(int Num)` is the only body
//Nickliu 20230314 Add Read Host Idle Time
int GetModuleHostIdleTime(int Num)
{
    char szSend[128], szRecv[128];
    memset(szRecv, '\0', sizeof(szRecv));
    memset(szSend, '\0', sizeof(szSend));
    strcpy(szSend, "%01GETMBTCPCN\r");                                          // golden Q8: built, never sent
    int iRef=-1;
    int hostIdleTime=0;
    if(ADAMTCP_UDPOpen(1000, 1000)==0)
    {
        if(ADAMTCP_GetHostIdleTime(Address[Num].c_str(), &hostIdleTime)==ADAMTCP_NoError)
        {
            iRef=hostIdleTime;
        }
    }
    ADAMTCP_UDPClose();
    return iRef;
}
//Nickliu 20230314 Add Read Host Idle Time
//---------------------------------------------------------------------------
// golden 912 :2887-2905 (906 :2885-2903)
//Nickliu 20230314 Add Set Host Idle Time
bool SetModuleHostIdleTime(int ihostIdleTime)
{
    //Set HostIdleTime
//    char szSend[128], szRecv[128];
//    memset ( szRecv , '\0' , sizeof(szRecv));
//    memset ( szSend , '\0' , sizeof(szSend));
//    strcpy(szSend, "%01GETMBTCPCN\r");
    bool bRef=false;
//    if (ADAMTCP_UDPOpen(1000, 1000) == 0)
//    {
//        if (ADAMTCP_SetHostIdleTime(Address[Num].c_str(), ihostIdleTime) == ADAMTCP_NoError)
//        {
//            bRef = true;
//        }
//    }
//    ADAMTCP_UDPClose();
    return bRef;
}
//Nickliu 20230314 Add Set Host Idle Time
//---------------------------------------------------------------------------
// golden 912 :2908-2925 (906 :2906-2923)
//Nickliu 20230314 Add Check Module Name
bool fCheckModuleName_ADAM6024(int Num)   //Nickliu 20230314 Add Check Adam Module Name
{
    bool bRet=true;
    #ifndef SOFT_SIMULTE
    AnsiString sAdamModuleName="";
    sAdamModuleName=GetModuleName(Num);
    if(sAdamModuleName=="16024-D")    //ADAM6024                               (golden Q7)
    {
        bRet=true;
    }
    else
    {
        bRet=false;
    }
    #endif
    return bRet;
}
//Nickliu 20230314 Add Check Module Name
//---------------------------------------------------------------------------
// golden 912 :2928-2955 (906 :2926-2953)
//Nickliu 20230314 Add Check Adam FW Is New-->
bool fCheckModuleFWISNew_ADAM6024(int Num)
{
    bool bRet=false;
    #ifndef SOFT_SIMULTE
    AnsiString sAdamModuleName="";
    AnsiString sFWName="", sFWSubVersion="";
    int iSubVersion=0;
    //Version : 6.01 B21
    //B21 Is Version
    sAdamModuleName =GetFirmwareName(Num);                                      //6.01 B21
    sFWName         =sAdamModuleName.SubString(1, 4);
    sFWSubVersion   =sAdamModuleName.SubString(sAdamModuleName.Length()-1, sAdamModuleName.Length());
    iSubVersion     =atoi(sFWSubVersion.c_str());
    if(sFWName=="6.01" && iSubVersion>0)
    {
        if(iSubVersion>=21)
            bRet=true;
        else
            bRet=false;
    }
    else
    {
       bRet=false;
    }
    #endif
    return bRet;
}
//Nickliu 20230314 Add Check Adam FW Is New--<
//---------------------------------------------------------------------------
// golden 912 :2958-3016 (906 :2956-3014)
bool fCheckConnectStatus_ADAM6024(int Num)                                      //Hmy 20170120 add check Adam6024 Status
{
    // bConnectStatus = false;                                                  //Nickliu 20230330 add check statsu mark
    bool bRef = false;
    #ifndef SOFT_SIMULTE
    int iRetVal =-1;
    AnsiString Msg;

    if(bADAM6024FWIsNew[Num]==true)                                             //Nickliu 20230314 Add Check Adam FW Is New
    {
        if(fCheckModuleName_ADAM6024(Num)==false)                               //Nickliu 20230314 Add Check Adam Module Name
        {
            ShowMyMessage("ADAM Read ModuleName Fail, Please Check Lan Cable");
            bRef=false;                                                         //add return value no need retry connection
            return bRef; //Nickliu 202303                                       //Nickliu 20230330 add check statsu
        }
        else
        {
            if(ClearAllConnection(Num)==false)                                  //Nickliu 20230316 add Adam Clear All Connect
            {
                ShowMyMessage("Clear ADAM Connection Fail!");
                bRef=false;                                                     //Nickliu 20230330 add check statsu
                return bRef;
            }

            if(GetModuleConnectionCount(Num)>=8)                                //Nickliu 20230321 add check connect count
            {                                                                   //connection over 8 and clear fail
                ShowMyMessage("Adam Clear Fail, Please Check Network Cable or IP address");
                bRef=false;                                                     //Nickliu 20230330 add check statsu
                return bRef;
            }
        }
        ADAMTCP_Disconnect();                                                   //Ifor 20230617 : 修正ADAM6024新版韌體才做斷線重連   (golden Q9)
        iRetVal=ADAMTCP_Connect(Address[Num].c_str(), DEFAULT_PORT,iConnectionTimeout, iSendTimeout, iReceiveTimeout);
        if(iRetVal<0)
        {
            Msg.sprintf("ADAMTCP Connect Fail..., Error Code:%d", iRetVal);
            ShowMyMessage(Msg);
            ADAMTCP_Close();                                                    // shim [W906] S3: absorbed unless an Open is outstanding
            MySleepEx(50, false);
            bRef=false;
            return bRef;                                                        //Nickliu 20230330 add check statsu
        }
        else
        {
            bRef=true;                                                          //Nickliu 20230330 add check statsu
            return bRef;                                                        //Nickliu 20230330 add check statsu
        }
    }
    else
    {
        bRef=true;                                                              //Nickliu 20230330 add check statsu
        return bRef;                                                            //Nickliu 20230330 add check statsu
    }
    #else
    (void)Num;                                                                  // [W906] -Wunused hygiene only
    bRef=true;                                                                  //Nickliu 20230330 add check statsu
    return bRef;                                                                //Nickliu 20230330 add check statsu
    #endif
}
//------------------------------------------------------------------------------
