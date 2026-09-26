// ===========================================================================
//  TesterComm/Rs232/Rs232Globals.cpp -- RS232Standard (tester RS232 + TTL board) globals and the cmydef.cpp free
//  functions, in namespace rs232std (see Rs232Bridge.h for why the program lives in its own namespace).
//
//  AI(W906-GB-P4) 20260926.  Golden: D:\RS232Standard\RS232_Code32Bin_Rev12.13.902.0_20260410 (Big5 -> UTF-8).
//    * cmydef.cpp:15-19             global definitions, initial values verbatim
//    * cmydef.cpp:21-616            crc_chk / MyDeCodeASCII / GetErrorMessage / RecordProcess / RecordChangeLogProcess /
//                                   CloseIniFile / OpenIniFile / CheckAndReadIniData x6 / WriteIniData x7 /
//                                   GetTimeInfo / CheckFileExist / MySleep / MyForceDirectories -- bodies verbatim
//    * cmydef.cpp:664-711           VerInfo::GetAppVersion (version-resource read replaced by the fixed program
//                                   version, see there) / VerInfo::GetSVNRev / VerInfo::GetFileVersion
//    * MessageDef.cpp:13-15,234-235 GPIBVersion / RS232Version / GPIBVersionCheck / GGpib2Handler / GHandler2Gpib
//    * MainForm.cpp:14, 26-42, 296-355   fRS232Main and the file-scope globals
//    * MainForm.cpp:2564-2576       WriteDataToFile
//    * V906 only: g_rs232Life + ResetRs232Globals() -- every global above back to its golden initialiser, for the
//      in-process restart that replaces golden's exe relaunch (Rs232Bridge.h "V906 program life")
//  NOT here (on purpose):
//    * VerInfo Windows version-resource plumbing: cmydef.cpp:618-621 ctor (the fixed header's ctor is inline),
//      :623-638 m_ClearData, :640-644 m_SetFileName, :646-653 m_strGetFixedFileVersion,
//      :655-662 m_strGetFixedProductVersion, :666-690 GetAppVersion's VS_FIXEDFILEINFO read, :713-834 m_GetVerInfo;
//      cmydef.h:294-301 VERSION_INFO_KEY_ROOT / VERSION_INFO_KEY_TRANS / VKINFO, :307-319 the __property list,
//      :323-345 the private members.  The fixed header's VerInfo declares none of them, and in-process there is no
//      RS232Standard.exe to read.
//    * cmydef.h:11-263 ALIAS and the CC_* customer codes -- Rs232Bridge.h banner: shared from MachineType.h via
//      MessageDef.h.
//    * MessageDef.cpp:17-227 MSG_CMD_* constants -- shared from :: MessageDef.cpp (checked 20260926 with a regex
//      over both files: 204 names in each, 0 missing, 0 value differences; the VM / MV field lists of golden
//      MessageDef.h and V906 MessageDef.h are identical too).  MessageDef.cpp:230-231 HGpib2Handler /
//      HHandler2Gpib ("For Handler") -- this program never reads them; the Handler keeps its own in ::.
//    * MainForm.cpp:16-25 macros -- constants / build switches in Rs232Bridge.h.
//
//  SHARED-DATA WARNING (golden paths kept verbatim, CLAUDE.md "已知的共用風險"):
//    asHGeneralPath = "d:\\HT9045\\system\\Gerneral.ini" is the production-shared Handler ini.  Golden reads it
//    through CheckAndReadIniData (MainForm.cpp:421, 422, 3515, 3532), which WRITES the default back when the key is
//    missing.  IniFileName = "D:\\RS232Standard\\System\\Setup.ini" is read and written (MainForm.cpp:419-423,
//    2498-2519, 2543-2560).  ResetRs232Globals() puts both back to these golden paths; Rs232Engine::Start re-points
//    them right after it when OverrideIniPaths() was used (Rs232Engine.cpp:104-109).
// ===========================================================================
#include "TesterComm/Rs232/Rs232Bridge.h"

#include <io.h>      // _open / _close   (golden cmydef.cpp:7 <io.h>,    for _rtl_open / _rtl_close)
#include <fcntl.h>   // _O_RDONLY        (golden cmydef.cpp:8 <fcntl.h>, for O_RDONLY)
#include <string.h>  // memset           (ResetRs232Globals)

// golden MainForm.cpp:16-17 build switches (Rs232Bridge.h banner; golden defines them after its includes).  This
// file holds MainForm.cpp text (the file-scope globals and WriteDataToFile); none of that text is conditional on
// them, they are here so every MainForm.cpp piece sees the same switches.
#if RS232STD_GOLDEN_DEBUG
#define DEBUG
#endif
#if RS232STD_GOLDEN_SOFT_SIMULTE
#define SOFT_SIMULTE
#endif

namespace rs232std {

//------------------------------------------------------------------------------
//  cmydef.cpp:15-19
//------------------------------------------------------------------------------
Word SystemHour=9999, SystemMin=9999, SystemSec=9999, SystemMSec=9999;
Word SystemYear=9999, SystemMonth=9999, SystemDate=9999;
Word SystemYearYesterday=9999, SystemMonthYesterday=9999, SystemDateYesterday=9999;
TIniFile *INIFile=NULL;                                                         //AI(W906-GB-P4) 20260926: golden `TIniFile *INIFile;` (static storage == NULL), explicit here
bool InitialOK=false;

//------------------------------------------------------------------------------
//  MessageDef.cpp (RS232-side instances)
//------------------------------------------------------------------------------
AnsiString GPIBVersion="12.13.884.0";                                           // MessageDef.cpp:13
AnsiString RS232Version="12.13.884.0";                                          // MessageDef.cpp:14 (stale vs the 902 build; FormShow re-assigns it, MainForm.cpp:410)
double GPIBVersionCheck=12.13;                                                  //Steven 20191007 : 改成判斷兩組版號, 所以使用Double

//For GPIB-------------
VM GGpib2Handler;                                                               // MessageDef.cpp:234  (static storage: zero-initialized)
MV *GHandler2Gpib=NULL;                                                         // MessageDef.cpp:235  //AI(W906-GB-P4) 20260926: golden `MV *GHandler2Gpib;` (static storage == NULL), explicit here
//---------------------

//------------------------------------------------------------------------------
//  MainForm.cpp:14 -- the form pointer (created by Rs232Engine::Start)
//------------------------------------------------------------------------------
TfRS232Main *fRS232Main=NULL;                                                   //AI(W906-GB-P4) 20260926: golden `TfRS232Main *fRS232Main;` (static storage == NULL), explicit here

//------------------------------------------------------------------------------
//  MainForm.cpp:26-42
//------------------------------------------------------------------------------
TStringList *sBarCode;                                                          //Steven 20150713 : Add 2D code
TStringList *sBarCode_ASE_CL;                                                   //KaiChen 20191126 ：中壢日月光，2D 回傳格式
bool bHasBarCode=false;                                                         //Jimmychiu 20211004 add barcode
bool bShowVersionOK=false;                                                      //Isaac 20210510 : 避免還沒new出來就讀count造成錯誤
bool bTTL1RS232Send=false;
bool bTTL2RS232Send=false;
bool bTTL1RS232Rev=false;
bool bTTL2RS232Rev=false;
//<==
//Isaac 20200903 :TTL RS232通訊
AnsiString asHGeneralPath="d:\\HT9045\\system\\Gerneral.ini";
AnsiString IniFileName="D:\\RS232Standard\\System\\Setup.ini";                  //Isaac 20200903 :TTL RS232通訊
const int iRS232Error   =-1;
const int iNoneTest     =0;                                                     //Steven 20231205 : 判斷RS232流程是否異常
const int iReadyToTest  =1;
const int iWaitReplyCE  =2;
const int iReplyCE      =3;

//------------------------------------------------------------------------------
//  MainForm.cpp:296-355
//------------------------------------------------------------------------------
HWND HMountWnd=NULL;
//---------------------------------------------------------------------------
bool bSimulate      = false;
bool bSupport32Bin  = false;                                                    //Steven 20121112 : 支援32Bin
bool bManualTest    = false;
int iStart [MAX_SITE_COUNT];
int iResult[MAX_SITE_COUNT];
bool bGpibMode      = true ;                                                    //wei 20150409 ON/OFF LINE
AnsiString sMachineStateDecade;                                                 //JerryYang 20151109 機台狀態
int iMacStateStrLength;                                                         //JerryYang 20151109 字串長度
char cMachineStateDec[6];                                                       //JerryYang 20151109 轉為ASCII碼
AnsiString sTestBinCount;                                                       //JerryYang 20160308 各Bin數量
char cTestBinCount[1024];                                                       //JerryYang 20160308 各Bin數量
AnsiString sSoakTime;                                                           //JerryYang 20160318 回傳加熱時間
char cSoakTime[5];                                                              //JerryYang 20160318 回傳加熱時間
AnsiString sJamCode;                                                            //JerryYang 20160323 回傳Jam code
char cJamCode[5];                                                               //JerryYang 20160323 回傳Jam code
AnsiString sSiteMap;                                                            //JerryYang 20160324 回傳SiteMap
char cSiteMap[256];                                                             //JerryYang 20160324 回傳SiteMap
AnsiString sAllMassTemp;                                                        //JerryYang 20160330 回傳All Mass Temp
char cAllMassTemp[100];                                                         //JerryYang 20160330 回傳All Mass Temp
AnsiString sHandlerID;
AnsiString sSiteOnOff;
char cSiteOnOff[256];
AnsiString sNumOfSites;                                                         //JerryYang 20160318 回傳加熱時間
char cNumOfSites[10];                                                           //JerryYang 20160318 回傳加熱時間
char cHandlerID[100];                                                           //JerryYang 20160330 回傳All Mass Temp
int iUseRS232Mode=-1;                                                           //Isaac 20200903 :TTL RS232通訊
int TTL_CARD_TYPE=0;
bool bStartSOT[2]={false, false};                                               //Isaac 20210309 :TTL RS232兩塊板子
int iTTLBoardNum=-1;                                                            //Isaac 20210309 :TTL RS232兩塊板子
bool bTTLBoardAddr=false;                                                       //Isaac 20210309 :TTL RS232兩塊板子
AnsiString sDoubleContactCount;                                                 //Isaac 20210706 : add MSG_CMD_DoubleContactCount指令，詢問handler doublecontact次數
char cDoubleContactCount[5];                                                    //Isaac 20210706 : add MSG_CMD_DoubleContactCount指令，詢問handler doublecontact次數
//Isaac 20190502 : SLT flow by RS232
//=>
double dSLTMaxTime;                                                             //Isaac 20190502 : SLT flow by RS232
bool bUseRS232SLTFlow;                                                          //Isaac 20190502 : SLT flow by RS232
bool bRunCheckProgramUse;
AnsiString RunCheckProgramName;
AnsiString RunCheckProgramErrCode;
double dPowerSwitchDelay;
double dSendNEXTDelay;
double dMaxBIOSWaitTime;
AnsiString MaxBIOSWaitTimeErrCode;
bool bMaxBIOSWaitTimeAlm;
double dMinTestTime;
AnsiString MinTestTimeErrCode;
bool bMinTestTimeAlm;
AnsiString MaxTestTimeErrCode;
bool bMaxTestTimeAlm;
double dTestOKWaitTime;
AnsiString sSLTState;                                                           //Isaac 20210331
bool bCanRevSortBin;
bool bErrorOccur;
//<=
//Isaac 20190502 : SLT flow by RS232
AnsiString sTTLVersion="";                                                      //Isaac 20210511 : TTLRS232板子版本檢查
AnsiString sTTLVersion2="";
int iRevCycleClear=5;                                                           //Jimmychiu 20240122 : add Rev cycle clear

//------------------------------------------------------------------------------
//  V906 program life (not golden) -- Rs232Bridge.h "V906 program life"
//------------------------------------------------------------------------------
unsigned long g_rs232Life = 0;                                                  //AI(W906-GB-P4) 20260926: incremented by Rs232Engine::Start only; ResetRs232Globals does not touch it

//AI(W906-GB-P4) 20260926: ResetRs232Globals -- golden's Handler relaunches RS232Standard.exe (RunTestProgram) and
//  every new process starts each global at its initialiser.  In-process Rs232Engine::Start calls this instead, before
//  TfRS232Main is constructed.  Every global defined above is put back to its golden initial value, in definition
//  order: the explicit initialiser where golden has one, otherwise the static-storage zero-initialisation golden
//  relies on (0 / false / "" / NULL; char arrays and GGpib2Handler memset 0).
//  NOT reset: fRS232Main, HMountWnd (engine-owned), g_rs232Life (engine-owned), and the const globals iRS232Error /
//  iNoneTest / iReadyToTest / iWaitReplyCE / iReplyCE (immutable).
//  INIFile is set to NULL WITHOUT delete: golden never deletes the last cached TIniFile either (CloseIniFile runs
//  only from OpenIniFile, cmydef.cpp:232; the exe exit reclaimed it).  vclcompat TIniFile writes through to disk on
//  every mutation (vclcompat/IniFiles.h "FLUSH POLICY"), so dropping the pointer loses no data; the cost is one
//  leaked TIniFile object per program life.
//  NOTE for the engine: asHGeneralPath / IniFileName come back to the golden production paths here, so any
//  re-pointing of them must be done AFTER this call (Rs232Engine.cpp:104-109 does).
//  Counts: 89 globals defined in this file; 81 reset here; 8 not reset (fRS232Main, HMountWnd, g_rs232Life, the
//  5 consts).  Function-local statics are not covered here (re-armed by g_rs232Life in their own function).
void ResetRs232Globals()
{
    // ---- cmydef.cpp:15-19 ----
    SystemHour=9999; SystemMin=9999; SystemSec=9999; SystemMSec=9999;           // :15
    SystemYear=9999; SystemMonth=9999; SystemDate=9999;                         // :16
    SystemYearYesterday=9999; SystemMonthYesterday=9999; SystemDateYesterday=9999;   // :17
    INIFile=NULL;                                                               // :18  NOT deleted (see above)
    InitialOK=false;                                                            // :19  (the TfRS232Main ctor sets it true, MainForm.cpp:366)

    // ---- MessageDef.cpp:13-15, 234-235 ----
    GPIBVersion="12.13.884.0";                                                  // :13
    RS232Version="12.13.884.0";                                                 // :14  (FormShow re-assigns it, MainForm.cpp:410)
    GPIBVersionCheck=12.13;                                                     // :15
    memset(&GGpib2Handler, 0, sizeof(GGpib2Handler));                           // :234 (VM is plain C, MessageDef.h)
    GHandler2Gpib=NULL;                                                         // :235

    // ---- MainForm.cpp:14 fRS232Main -- engine-owned, not reset

    // ---- MainForm.cpp:26-42 ----
    sBarCode=NULL;                                                              // :26  re-created by the TfRS232Main ctor (MainForm.cpp:376); golden FormClose deletes it without NULLing (:552)
    sBarCode_ASE_CL=NULL;                                                       // :27  re-created by the ctor (:377); deleted by FormClose (:553)
    bHasBarCode=false;                                                          // :28
    bShowVersionOK=false;                                                       // :29
    bTTL1RS232Send=false;                                                       // :30
    bTTL2RS232Send=false;                                                       // :31
    bTTL1RS232Rev=false;                                                        // :32
    bTTL2RS232Rev=false;                                                        // :33
    asHGeneralPath="d:\\HT9045\\system\\Gerneral.ini";                          // :36
    IniFileName="D:\\RS232Standard\\System\\Setup.ini";                         // :37
    // :38-42 iRS232Error / iNoneTest / iReadyToTest / iWaitReplyCE / iReplyCE -- const, not reset

    // ---- MainForm.cpp:296-355 ----
    // :296 HMountWnd -- engine-owned, not reset
    bSimulate=false;                                                            // :298
    bSupport32Bin=false;                                                        // :299
    bManualTest=false;                                                          // :300
    memset(iStart, 0, sizeof(iStart));                                          // :301
    memset(iResult, 0, sizeof(iResult));                                        // :302
    bGpibMode=true;                                                             // :303
    sMachineStateDecade="";                                                     // :304
    iMacStateStrLength=0;                                                       // :305
    memset(cMachineStateDec, 0, sizeof(cMachineStateDec));                      // :306
    sTestBinCount="";                                                           // :307
    memset(cTestBinCount, 0, sizeof(cTestBinCount));                            // :308
    sSoakTime="";                                                               // :309
    memset(cSoakTime, 0, sizeof(cSoakTime));                                    // :310
    sJamCode="";                                                                // :311
    memset(cJamCode, 0, sizeof(cJamCode));                                      // :312
    sSiteMap="";                                                                // :313
    memset(cSiteMap, 0, sizeof(cSiteMap));                                      // :314
    sAllMassTemp="";                                                            // :315
    memset(cAllMassTemp, 0, sizeof(cAllMassTemp));                              // :316
    sHandlerID="";                                                              // :317
    sSiteOnOff="";                                                              // :318
    memset(cSiteOnOff, 0, sizeof(cSiteOnOff));                                  // :319
    sNumOfSites="";                                                             // :320
    memset(cNumOfSites, 0, sizeof(cNumOfSites));                                // :321
    memset(cHandlerID, 0, sizeof(cHandlerID));                                  // :322
    iUseRS232Mode=-1;                                                           // :323
    TTL_CARD_TYPE=0;                                                            // :324
    bStartSOT[0]=false; bStartSOT[1]=false;                                     // :325  {false, false}
    iTTLBoardNum=-1;                                                            // :326
    bTTLBoardAddr=false;                                                        // :327
    sDoubleContactCount="";                                                     // :328
    memset(cDoubleContactCount, 0, sizeof(cDoubleContactCount));                // :329
    dSLTMaxTime=0.0;                                                            // :332
    bUseRS232SLTFlow=false;                                                     // :333
    bRunCheckProgramUse=false;                                                  // :334
    RunCheckProgramName="";                                                     // :335
    RunCheckProgramErrCode="";                                                  // :336
    dPowerSwitchDelay=0.0;                                                      // :337
    dSendNEXTDelay=0.0;                                                         // :338
    dMaxBIOSWaitTime=0.0;                                                       // :339
    MaxBIOSWaitTimeErrCode="";                                                  // :340
    bMaxBIOSWaitTimeAlm=false;                                                  // :341
    dMinTestTime=0.0;                                                           // :342
    MinTestTimeErrCode="";                                                      // :343
    bMinTestTimeAlm=false;                                                      // :344
    MaxTestTimeErrCode="";                                                      // :345
    bMaxTestTimeAlm=false;                                                      // :346
    dTestOKWaitTime=0.0;                                                        // :347
    sSLTState="";                                                               // :348
    bCanRevSortBin=false;                                                       // :349
    bErrorOccur=false;                                                          // :350
    sTTLVersion="";                                                             // :353
    sTTLVersion2="";                                                            // :354
    iRevCycleClear=5;                                                           // :355
    // g_rs232Life -- engine-owned, not reset
}

//==============================================================================
//  cmydef.cpp functions (golden order)
//==============================================================================
unsigned int crc_chk(unsigned char* data, unsigned char length, char &CRC1, char &CRC2)     //Isaac 20200903 :TTL RS232通訊
{
    unsigned int reg_crc=0xFFFF;
    while(length--)
    {
        reg_crc^=*data++;
        for(int j=0; j<8; j++)
        {
            if(reg_crc&0x01)                                                    /* LSB(b0)=1 */
                reg_crc=(reg_crc>>1)^0xA001;
            else
                reg_crc=reg_crc>>1;
        }
    }

    CRC1=(reg_crc&0xff00)>>8;
    CRC2=(reg_crc&0xff);

    return reg_crc;
}
//------------------------------------------------------------------------------
AnsiString MyDeCodeASCII(int iInPut)
{
    AnsiString asReturnASCII="";
    switch (iInPut)
    {
        case   0 : {  asReturnASCII="[NUL]"; break;}
        case   1 : {  asReturnASCII="[SOH]"; break;}
        case   2 : {  asReturnASCII="[STX]"; break;}
        case   3 : {  asReturnASCII="[ETX]"; break;}
        case   4 : {  asReturnASCII="[EOT]"; break;}
        case   5 : {  asReturnASCII="[ENQ]"; break;}
        case   6 : {  asReturnASCII="[ACK]"; break;}
        case   7 : {  asReturnASCII="[BEL]"; break;}
        case   8 : {  asReturnASCII="[BS]" ; break;}
        case   9 : {  asReturnASCII="[HT]" ; break;}
        case  10 : {  asReturnASCII="[LF]" ; break;}
        case  11 : {  asReturnASCII="[VT]" ; break;}
        case  12 : {  asReturnASCII="[FF]" ; break;}
        case  13 : {  asReturnASCII="[CR]" ; break;}
        case  14 : {  asReturnASCII="[SO]" ; break;}
        case  15 : {  asReturnASCII="[SI]" ; break;}
        case  16 : {  asReturnASCII="[DLE]"; break;}
        case  17 : {  asReturnASCII="[DC1]"; break;}
        case  18 : {  asReturnASCII="[DC2]"; break;}
        case  19 : {  asReturnASCII="[DC3]"; break;}
        case  20 : {  asReturnASCII="[DC4]"; break;}
        case  21 : {  asReturnASCII="[NAK]"; break;}
        case  22 : {  asReturnASCII="[SYN]"; break;}
        case  23 : {  asReturnASCII="[ETB]"; break;}
        case  24 : {  asReturnASCII="[CAN]"; break;}
        case  25 : {  asReturnASCII="[EM]" ; break;}
        case  26 : {  asReturnASCII="[SUB]"; break;}
        case  27 : {  asReturnASCII="[ESC]"; break;}
        case  28 : {  asReturnASCII="[FS]" ; break;}
        case  29 : {  asReturnASCII="[GS]" ; break;}
        case  30 : {  asReturnASCII="[RS]" ; break;}
        case  31 : {  asReturnASCII="[US]" ; break;}
        case  32 : {  asReturnASCII=" "  ; break;}
        case  33 : {  asReturnASCII="!"  ; break;}
        case  34 : {  asReturnASCII="\"" ; break;}
        case  35 : {  asReturnASCII="#"  ; break;}
        case  36 : {  asReturnASCII="$"  ; break;}
        case  37 : {  asReturnASCII="%"  ; break;}
        case  38 : {  asReturnASCII="&"  ; break;}
        case  39 : {  asReturnASCII="\'" ; break;}
        case  40 : {  asReturnASCII="("  ; break;}
        case  41 : {  asReturnASCII=")"  ; break;}
        case  42 : {  asReturnASCII="*"  ; break;}
        case  43 : {  asReturnASCII="+"  ; break;}
        case  44 : {  asReturnASCII=","  ; break;}
        case  45 : {  asReturnASCII="-"  ; break;}
        case  46 : {  asReturnASCII="."  ; break;}
        case  47 : {  asReturnASCII="/"  ; break;}
        case  48 : {  asReturnASCII="0"  ; break;}
        case  49 : {  asReturnASCII="1"  ; break;}
        case  50 : {  asReturnASCII="2"  ; break;}
        case  51 : {  asReturnASCII="3"  ; break;}
        case  52 : {  asReturnASCII="4"  ; break;}
        case  53 : {  asReturnASCII="5"  ; break;}
        case  54 : {  asReturnASCII="6"  ; break;}
        case  55 : {  asReturnASCII="7"  ; break;}
        case  56 : {  asReturnASCII="8"  ; break;}
        case  57 : {  asReturnASCII="9"  ; break;}
        case  58 : {  asReturnASCII=":"  ; break;}
        case  59 : {  asReturnASCII=";"  ; break;}
        case  60 : {  asReturnASCII="<"  ; break;}
        case  61 : {  asReturnASCII="="  ; break;}
        case  62 : {  asReturnASCII=">"  ; break;}
        case  63 : {  asReturnASCII="\?" ; break;}
        case  64 : {  asReturnASCII="@"  ; break;}
        case  65 : {  asReturnASCII="A"  ; break;}
        case  66 : {  asReturnASCII="B"  ; break;}
        case  67 : {  asReturnASCII="C"  ; break;}
        case  68 : {  asReturnASCII="D"  ; break;}
        case  69 : {  asReturnASCII="E"  ; break;}
        case  70 : {  asReturnASCII="F"  ; break;}
        case  71 : {  asReturnASCII="G"  ; break;}
        case  72 : {  asReturnASCII="H"  ; break;}
        case  73 : {  asReturnASCII="I"  ; break;}
        case  74 : {  asReturnASCII="J"  ; break;}
        case  75 : {  asReturnASCII="K"  ; break;}
        case  76 : {  asReturnASCII="L"  ; break;}
        case  77 : {  asReturnASCII="M"  ; break;}
        case  78 : {  asReturnASCII="N"  ; break;}
        case  79 : {  asReturnASCII="O"  ; break;}
        case  80 : {  asReturnASCII="P"  ; break;}
        case  81 : {  asReturnASCII="Q"  ; break;}
        case  82 : {  asReturnASCII="R"  ; break;}
        case  83 : {  asReturnASCII="S"  ; break;}
        case  84 : {  asReturnASCII="T"  ; break;}
        case  85 : {  asReturnASCII="U"  ; break;}
        case  86 : {  asReturnASCII="V"  ; break;}
        case  87 : {  asReturnASCII="W"  ; break;}
        case  88 : {  asReturnASCII="X"  ; break;}
        case  89 : {  asReturnASCII="Y"  ; break;}
        case  90 : {  asReturnASCII="Z"  ; break;}
        case  91 : {  asReturnASCII="["  ; break;}
        case  92 : {  asReturnASCII="\\" ; break;}
        case  93 : {  asReturnASCII="]"  ; break;}
        case  94 : {  asReturnASCII="^"  ; break;}
        case  95 : {  asReturnASCII="_"  ; break;}
        case  96 : {  asReturnASCII="`"  ; break;}
        case  97 : {  asReturnASCII="a"  ; break;}
        case  98 : {  asReturnASCII="b"  ; break;}
        case  99 : {  asReturnASCII="c"  ; break;}
        case 100 : {  asReturnASCII="d"  ; break;}
        case 101 : {  asReturnASCII="e"  ; break;}
        case 102 : {  asReturnASCII="f"  ; break;}
        case 103 : {  asReturnASCII="g"  ; break;}
        case 104 : {  asReturnASCII="h"  ; break;}
        case 105 : {  asReturnASCII="i"  ; break;}
        case 106 : {  asReturnASCII="j"  ; break;}
        case 107 : {  asReturnASCII="k"  ; break;}
        case 108 : {  asReturnASCII="l"  ; break;}
        case 109 : {  asReturnASCII="m"  ; break;}
        case 110 : {  asReturnASCII="n"  ; break;}
        case 111 : {  asReturnASCII="o"  ; break;}
        case 112 : {  asReturnASCII="p"  ; break;}
        case 113 : {  asReturnASCII="q"  ; break;}
        case 114 : {  asReturnASCII="r"  ; break;}
        case 115 : {  asReturnASCII="s"  ; break;}
        case 116 : {  asReturnASCII="t"  ; break;}
        case 117 : {  asReturnASCII="u"  ; break;}
        case 118 : {  asReturnASCII="v"  ; break;}
        case 119 : {  asReturnASCII="w"  ; break;}
        case 120 : {  asReturnASCII="x"  ; break;}
        case 121 : {  asReturnASCII="y"  ; break;}
        case 122 : {  asReturnASCII="z"  ; break;}
        case 123 : {  asReturnASCII="{"  ; break;}
        case 124 : {  asReturnASCII="|"  ; break;}
        case 125 : {  asReturnASCII="}"  ; break;}
        case 126 : {  asReturnASCII="~"  ; break;}
        case 127 : {  asReturnASCII="[DEL]"; break;}
        default  : {  asReturnASCII="[Err]"; break;}
    }
    return asReturnASCII;
}
//------------------------------------------------------------------------------
AnsiString GetErrorMessage(DWORD dwErrorMessageCode)
{
    AnsiString strMsg;
    //AI(W906-GB-P4) 20260926: golden `LPVOID lpMsgBuf;` (uninitialised).  If FormatMessage fails it leaves the
    //  pointer untouched and golden then hands stack garbage to %s and LocalFree (undefined behaviour).  NULL here:
    //  byte-identical whenever FormatMessage succeeds; on failure %s prints "(null)" and LocalFree(NULL) is a no-op.
    LPVOID lpMsgBuf=NULL;

    FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                  NULL,
                  dwErrorMessageCode,
                  MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US),                 // 顯示語言
                  (LPTSTR) &lpMsgBuf,
                  0,
                  NULL);

    strMsg.sprintf(("Error Code : 0x%02X ==> Error Message : %s "), dwErrorMessageCode, lpMsgBuf);
    LocalFree(lpMsgBuf);                                                        // 記得free掉空間，養成好習慣
    strMsg=StringReplace(strMsg, "\n", "", TReplaceFlags()<<rfReplaceAll);
    strMsg=StringReplace(strMsg, "\r", "", TReplaceFlags()<<rfReplaceAll);
    return strMsg;
}
//------------------------------------------------------------------------------
//AI(W906-GB-P4) 20260926: RecordProcess / RecordChangeLogProcess / CloseIniFile / OpenIniFile have external linkage
//  in golden but no golden header declares them and nothing outside cmydef.cpp calls them (grep of every golden
//  *.cpp / *.h, 20260926), so they are file-static here (TRANSLATION_RULES.md rule 1).
static void RecordProcess(AnsiString Msg1, AnsiString Msg2="")
{
    if(fRS232Main!=NULL)
    {
        fRS232Main->ShowCommData("[Process]", Msg1, Msg2);
    }
}
//------------------------------------------------------------------------------
static void RecordChangeLogProcess(AnsiString Msg1, AnsiString Msg2)
{
    if(fRS232Main!=NULL)
    {
        fRS232Main->ShowCommData("[Change Log]", Msg1, Msg2);
    }
}
//------------------------------------------------------------------------------
static void CloseIniFile()                                                      //Steven 20141120 : Add Read/Write IniFile Speed
{
    if(INIFile!=NULL)
    {
        INIFile->UpdateFile();
        delete INIFile;
        //AI(W906-GB-P4) 20260926: golden quirk kept -- INIFile is not set to NULL after the delete; the only caller
        //  (OpenIniFile below) assigns a new TIniFile on the very next line, so the dangling value is never read.
    }
}
//------------------------------------------------------------------------------
static bool OpenIniFile(AnsiString FileName)                                    //Steven 20141120 : Add Read/Write IniFile Speed
{
    if(FileName=="")                                                            //Steven 20160606 : Add protection for inifile
        return false;

    if(INIFile==NULL || INIFile->FileName!=FileName)
    {
        CloseIniFile();
        INIFile=new TIniFile(FileName);
    }
    return true;
}
//------------------------------------------------------------------------------
double CheckAndReadIniData(AnsiString FileName, AnsiString Group, AnsiString Name, double Value)
{
    AnsiString Str;
    if(OpenIniFile(FileName)==false)                                            //Steven 20141120 : Add Read/Write IniFile Speed
    {
        Str.sprintf("[%s] %s", Group, Name);
        RecordProcess("Read NULL INI", Str);
        return Value;
    }

    Str.sprintf("%0.4f", Value);
    if(!INIFile->ValueExists(Group, Name))
        INIFile->WriteString(Group, Name, Str);                                 //Steven 20150723 : double資料存檔前都補成4個0
    else
        Value=INIFile->ReadFloat(Group, Name, Value);
    return Value;
}
//------------------------------------------------------------------------------
//AI(W906-GB-P4) 20260926: this overload is defined (external linkage) in golden but declared in no golden header
//  (cmydef.h:271-275 lists the other five) and called nowhere; the fixed Rs232Bridge.h does not declare it either.
//  Kept with golden's linkage (not static: an unused static function would only raise -Wunused-function).
//  Golden quirk kept: when the key is missing it calls ReadInteger (result discarded) where every sibling overload
//  WRITES the default, so no default is ever written back to the ini by this overload.
unsigned long CheckAndReadIniData(AnsiString FileName, AnsiString Group, AnsiString Name, unsigned long Value)
{
    AnsiString Str;
    if(OpenIniFile(FileName)==false)                                            //Steven 20141120 : Add Read/Write IniFile Speed
    {
        Str.sprintf("[%s] %s", Group, Name);
        RecordProcess("Read NULL INI", Str);
        return Value;
    }

    if(!INIFile->ValueExists(Group, Name))
        INIFile->ReadInteger(Group, Name, Value);
    else
        Value=INIFile->ReadInteger(Group, Name, Value);
    return Value;
}
//------------------------------------------------------------------------------
int CheckAndReadIniData(AnsiString FileName, AnsiString Group, AnsiString Name, int Value)
{
    AnsiString Str;
    if(OpenIniFile(FileName)==false)                                            //Steven 20141120 : Add Read/Write IniFile Speed
    {
        Str.sprintf("[%s] %s", Group, Name);
        RecordProcess("Read NULL INI", Str);
        return Value;
    }

    if(!INIFile->ValueExists(Group, Name))
        INIFile->WriteInteger(Group, Name, Value);
    else
        Value=INIFile->ReadInteger(Group, Name, Value);
    return Value;
}
//------------------------------------------------------------------------------
bool CheckAndReadIniData(AnsiString FileName, AnsiString Group, AnsiString Name, bool Value)
{
    AnsiString Str;
    if(OpenIniFile(FileName)==false)                                            //Steven 20141120 : Add Read/Write IniFile Speed
    {
        Str.sprintf("[%s] %s", Group, Name);
        RecordProcess("Read NULL INI", Str);
        return Value;
    }

    if(!INIFile->ValueExists(Group, Name))
        INIFile->WriteBool(Group, Name, Value);
    else
        Value=INIFile->ReadBool(Group, Name, Value);
    return Value;
}
//------------------------------------------------------------------------------
AnsiString CheckAndReadIniData(AnsiString FileName, AnsiString Group, AnsiString Name, AnsiString Value)
{
    AnsiString Str;
    if(OpenIniFile(FileName)==false)                                            //Steven 20141120 : Add Read/Write IniFile Speed
    {
        Str.sprintf("[%s] %s", Group, Name);
        RecordProcess("Read NULL INI", Str);
        return Value;
    }

    if(!INIFile->ValueExists(Group, Name))
    {
        INIFile->WriteString(Group, Name, Value);
        Str=Value;                                                              //JerryYang 20170711 (Steven) 修正read ini file初始值異常問題
    }
    else
    {
        Str=INIFile->ReadString(Group, Name, Value);
        if(Str=="" && Value!="")                                                //Steven 20160323 : Fixed when value is NULL
        {
            Str=Value;
            INIFile->WriteString(Group, Name, Value);
        }
    }
    return Str;
}
//------------------------------------------------------------------------------
TDateTime CheckAndReadIniData(AnsiString FileName, AnsiString Group, AnsiString Name, TDateTime Value)
{
    AnsiString Str;
    if(OpenIniFile(FileName)==false)                                            //Steven 20141120 : Add Read/Write IniFile Speed
    {
        Str.sprintf("[%s] %s", Group, Name);
        RecordProcess("Read NULL INI", Str);
        return Value;
    }

    if(!INIFile->ValueExists(Group, Name))
        INIFile->WriteDateTime(Group, Name, Value);
    else
        Value=INIFile->ReadDateTime(Group, Name, Value);
    return Value;
}
//------------------------------------------------------------------------------
void WriteIniData(AnsiString FileName, AnsiString Group, AnsiString Name, bool bValue)  //Steven 20090731
{
    AnsiString Str, Str1, Str2;
    bool ret;
    if(OpenIniFile(FileName)==false)                                            //Steven 20141120 : Add Read/Write IniFile Speed
    {
        Str.sprintf("[%s] %s", Group, Name);
        RecordProcess("Write NULL INI", Str);
        return;
    }

    ret=INIFile->ReadBool(Group, Name, bValue);
    if(ret!=bValue && InitialOK==true)                                          //Ifor 20190919 : add 避免程式開啟時因客戶要求強制開啟功能寫入時發生異常
    {
        Str1.sprintf("[%s] %s", Group , Name);
        Str2.sprintf("%d ==> %d", ret, bValue);

        RecordChangeLogProcess(Str1, Str2);                                     //wei 20180625 offset Change log紀錄
    }

    try
    {
        INIFile->WriteBool(Group, Name, bValue);
    }
    catch(...)
    {
        Str.sprintf("WriteIniData:%s Group:%s Name:%s", FileName, Group , Name);
        fRS232Main->ShowCommData("[Exception]", "Write ini data fail!", Str);
    }
}
//------------------------------------------------------------------------------
void WriteIniData(AnsiString FileName, AnsiString Group, AnsiString Name, int Value)  //Steven 20090731
{
    AnsiString Str, Str1, Str2;
    int  ret;
    if(OpenIniFile(FileName)==false)                                            //Steven 20141120 : Add Read/Write IniFile Speed
    {
        Str.sprintf("[%s] %s", Group, Name);
        RecordProcess("Write NULL INI", Str);
        return;
    }

    ret=INIFile->ReadInteger(Group, Name, Value);
    if(ret!=Value && InitialOK==true)                                           //Ifor 20190919 : add 避免程式開啟時因客戶要求強制開啟功能寫入時發生異常
    {
        Str1.sprintf("[%s] %s", Group , Name);
        Str2.sprintf("%d ==> %d", ret, Value);

        RecordChangeLogProcess(Str1, Str2);                                     //wei 20180625 offset Change log紀錄
    }

    try                                                                         //JerryYang 20230204 : add 例外處理
    {
        INIFile->WriteInteger(Group, Name, Value);
    }
    catch(...)
    {
        Str.sprintf("WriteIniData:%s Group:%s Name:%s", FileName, Group , Name);
        fRS232Main->ShowCommData("[Exception]", "Write ini data fail!", Str);
    }
}
//------------------------------------------------------------------------------
void WriteIniData(AnsiString FileName, AnsiString Group, AnsiString Name, double Value)  //Steven 20090731
{
    AnsiString Str, Str1, Str2;
    double ret;
    if(OpenIniFile(FileName)==false)                                            //Steven 20141120 : Add Read/Write IniFile Speed
    {
        Str.sprintf("[%s] %s", Group, Name);
        RecordProcess("Write NULL INI", Str);
        return;
    }
    Str.sprintf("%0.4f", Value);                                                //Steven 20150723 : double資料存檔前都補成4個0

    ret=INIFile->ReadFloat(Group, Name, Value);
    if(ret!=Value && InitialOK==true)                                           //Ifor 20190919 : add 避免程式開啟時因客戶要求強制開啟功能寫入時發生異常
    {
        Str1.sprintf("[%s] %s", Group , Name);
        Str2.sprintf("%0.4f ==> %0.4f", ret, Value);

        RecordChangeLogProcess(Str1, Str2);                                     //wei 20180625 offset Change log紀錄
    }

    try                                                                         //JerryYang 20230204 : add 例外處理
    {
        INIFile->WriteString(Group, Name, Str);
    }
    catch(...)
    {
        Str.sprintf("WriteIniData:%s Group:%s Name:%s", FileName, Group , Name);
        fRS232Main->ShowCommData("[Exception]", "Write ini data fail!", Str);
    }
}
//------------------------------------------------------------------------------
void WriteIniData(AnsiString FileName, AnsiString Group, AnsiString Name, unsigned Value)
{
    AnsiString Str, Str1, Str2;
    unsigned ret;
    if(OpenIniFile(FileName)==false)                                            //Steven 20141120 : Add Read/Write IniFile Speed
    {
        Str.sprintf("[%s] %s", Group, Name);
        RecordProcess("Write NULL INI", Str);
        return;
    }

    ret=INIFile->ReadInteger(Group, Name, Value);
    if(ret!=Value && InitialOK==true)                                           //Ifor 20190919 : add 避免程式開啟時因客戶要求強制開啟功能寫入時發生異常
    {
        Str1.sprintf("[%s] %s", Group , Name);
        Str2.sprintf("%d ==> %d", ret, Value);

        RecordChangeLogProcess(Str1, Str2);                                     //wei 20180625 offset Change log紀錄
    }

    try                                                                         //JerryYang 20230204 : add 例外處理
    {
        INIFile->WriteInteger(Group, Name, Value);
    }
    catch(...)
    {
        Str.sprintf("WriteIniData:%s Group:%s Name:%s", FileName, Group , Name);
        fRS232Main->ShowCommData("[Exception]", "Write ini data fail!", Str);
    }
}
//------------------------------------------------------------------------------
void WriteIniData(AnsiString FileName, AnsiString Group, AnsiString Name, unsigned long Value)
{
    AnsiString Str, Str1, Str2;
    unsigned long ret;
    if(OpenIniFile(FileName)==false)                                            //Steven 20141120 : Add Read/Write IniFile Speed
    {
        Str.sprintf("[%s] %s", Group, Name);
        RecordProcess("Write NULL INI", Str);
        return;
    }

    ret=INIFile->ReadInteger(Group, Name, Value);
    if(ret!=Value && InitialOK==true)                                           //Ifor 20190919 : add 避免程式開啟時因客戶要求強制開啟功能寫入時發生異常
    {
        Str1.sprintf("[%s] %s", Group , Name);
        Str2.sprintf("%d ==> %d", ret, Value);

        RecordChangeLogProcess(Str1, Str2);                                     //wei 20180625 offset Change log紀錄
    }

    try                                                                         //JerryYang 20230204 : add 例外處理
    {
        INIFile->WriteInteger(Group, Name, Value);
    }
    catch(...)
    {
        Str.sprintf("WriteIniData:%s Group:%s Name:%s", FileName, Group , Name);
        fRS232Main->ShowCommData("[Exception]", "Write ini data fail!", Str);
    }
}
//------------------------------------------------------------------------------
void WriteIniData(AnsiString FileName, AnsiString Group, AnsiString Name, AnsiString Value) //Steven 20090731
{
    AnsiString Str, Str1, Str2;
    AnsiString ret;
    if(OpenIniFile(FileName)==false)                                            //Steven 20141120 : Add Read/Write IniFile Speed
    {
        Str.sprintf("[%s] %s", Group, Name);
        RecordProcess("Write NULL INI", Str);
        return;
    }

    ret=INIFile->ReadString(Group, Name, Value);
    if(ret!=Value && InitialOK==true)                                           //Ifor 20190919 : add 避免程式開啟時因客戶要求強制開啟功能寫入時發生異常
    {
        Str1.sprintf("[%s] %s", Group , Name);
        Str2.sprintf("%s ==> %s", ret, Value);

        RecordChangeLogProcess(Str1, Str2);                                     //wei 20180625 offset Change log紀錄
    }

    try                                                                         //JerryYang 20230204 : add 例外處理
    {
        INIFile->WriteString(Group, Name, Value);
    }
    catch(...)
    {
        Str.sprintf("WriteIniData:%s Group:%s Name:%s", FileName, Group , Name);
        fRS232Main->ShowCommData("[Exception]", "Write ini data fail!", Str);
    }
}
//------------------------------------------------------------------------------
void WriteIniData(AnsiString FileName, AnsiString Group, AnsiString Name, TDateTime Value)
{
    AnsiString Str, Str1;
    if(OpenIniFile(FileName)==false)                                            //Steven 20141120 : Add Read/Write IniFile Speed
    {
        Str.sprintf("[%s] %s", Group, Name);
        RecordProcess("Write NULL INI", Str);
        return;
    }

    try                                                                         //JerryYang 20230204 : add 例外處理
    {
        INIFile->WriteDateTime(Group, Name, Value);
    }
    catch(...)
    {
        Str.sprintf("WriteIniData:%s Group:%s Name:%s", FileName, Group , Name);
        fRS232Main->ShowCommData("[Exception]", "Write ini data fail!", Str);
    }
}
//------------------------------------------------------------------------------
void GetTimeInfo()
{
    static TDateTime dtPresent;
    //AI(W906-GB-P4) 20260926: program restart (g_rs232Life): no re-arm needed -- dtPresent is pure scratch, assigned
    //  from Now() on the next line before every read, so a value left by the previous program life is never seen.
    dtPresent=Now();
    DecodeDate(dtPresent, SystemYear, SystemMonth, SystemDate);
    DecodeTime(dtPresent, SystemHour, SystemMin, SystemSec, SystemMSec);
}
//------------------------------------------------------------------------------
//AI(W906-GB-P4) 20260926: CheckFileExist is defined (external linkage) in golden, declared in no golden header and
//  called nowhere (grep of every golden *.cpp / *.h, 20260926); the fixed header does not declare it.  Kept with
//  golden's linkage (not static, which would only raise -Wunused-function).
bool CheckFileExist(char *cFName)
{
    int fhandle;
    //AI(W906-GB-P4) 20260926: BCB6 RTL _rtl_open/_rtl_close -> CRT _open/_close (<io.h>), O_RDONLY -> _O_RDONLY
    //  (<fcntl.h>): same "can the path be opened read-only" test, -1 on failure.
    if((fhandle=_open(cFName, _O_RDONLY))==-1)
        return false;
    _close(fhandle);
    return true;
}
//------------------------------------------------------------------------------
void MySleep(DWORD dwMilliseconds)
{
    ::Sleep(dwMilliseconds);                                                    //AI(W906-GB-P4) 20260926: resolves to Win32 Sleep(DWORD) (exact match), not vclcompat::Sleep(int)
}
//------------------------------------------------------------------------------
int MyForceDirectories(AnsiString Directory, AnsiString Function)               //Steven 20210112 : 針對資料夾加上保護
{
    AnsiString Str1="", Str2="";
    if(Directory=="")
    {
        RecordProcess("Directory value is NULL!", Function);
        return -1;
    }
    else
    {
        try
        {
            if(DirectoryExists(Directory)==false)
            {
                ForceDirectories(Directory);
            }
        }
#if 0 // TODO(W906-GB-P4): vclcompat has no VCL Exception class (E.HelpContext / E.Message); golden cmydef.cpp:598-606.
      //   Anything thrown now lands in the catch(...) below: same "Force Directory FAIL!" line and the same -1, minus
      //   golden's two detail lines (Exception Code / Exception Message).  Tree precedent: common.cpp:2005-2015.
        catch(Exception& E)                                                     //Steven 20140505 : 試著抓出連線異常的訊息
        {
            Str1.sprintf("Function: %s, Directory:%s", Function, Directory);
            fRS232Main->ShowCommData("Exception", "Force Directory FAIL!", Str1);
            Str1.sprintf("Exception Code : %d", E.HelpContext);
            Str2.sprintf("Exception Message : %s", E.Message);
            fRS232Main->ShowCommData("[Exception]",  Str1, Str2);
            return -1;
        }
#endif
        catch(...)
        {
            Str1.sprintf("Function: %s, Directory:%s", Function, Directory);
            fRS232Main->ShowCommData("Exception", "Force Directory FAIL!", Str1);
            return -1;
        }
    }

    return 1;
}
//------------------------------------------------------------------------------
//  VerInfo (cmydef.cpp:664-711)
//------------------------------------------------------------------------------
void VerInfo::GetAppVersion(AnsiString sAppExeName, WORD& major, WORD& minor, WORD& build, WORD& revision)
{
    //AI(W906-GB-P4) 20260926: golden reads the VS_FIXEDFILEINFO of sAppExeName (cmydef.cpp:666-690:
    //  GetFileVersionInfoSize / GetFileVersionInfo / VerQueryValue "\\", returning early with the outputs untouched
    //  on any failure).  In-process there is no RS232Standard.exe: Application->ExeName would be the Handler's exe
    //  and report the HANDLER's version.  So the four numbers are this program's own version resource as golden
    //  builds it: RS232Standard.bpr:74-77 [Version Info] MajorVer=12 MinorVer=13 Release=902 Build=0 (== :89
    //  [Version Info Keys] FileVersion=12.13.902.0).  Golden's mapping kept: build = HIWORD(dwFileVersionLS) =
    //  Release, revision = LOWORD(dwFileVersionLS) = Build.  sAppExeName is not used.  Bump these with the golden
    //  snapshot.
    (void)sAppExeName;
    major   =12;                                                                // HIWORD(fileInfo->dwFileVersionMS)
    minor   =13;                                                                // LOWORD(fileInfo->dwFileVersionMS)
    build   =902;                                                               // HIWORD(fileInfo->dwFileVersionLS)
    revision=0;                                                                 // LOWORD(fileInfo->dwFileVersionLS)
}
//------------------------------------------------------------------------------
AnsiString VerInfo::GetSVNRev()
{
    AnsiString sret="";
    unsigned short iFileVerMajor=0, iFileVerMinor=0, iFileVerRelease=0, iFileVerBuild=0;
    //AnsiString strFilePath=Application->ExeName;                             //AI(W906-GB-P4) 20260926: no RS232Standard.exe in-process (see GetAppVersion); the name is informational only
    AnsiString strFilePath="RS232Standard.exe";
    VerInfo().GetAppVersion(strFilePath, iFileVerMajor, iFileVerMinor, iFileVerRelease, iFileVerBuild);
    sret=AnsiString().sprintf("%d.%d", iFileVerRelease, iFileVerBuild);
    return sret;
}
//------------------------------------------------------------------------------
//AI(W906-GB-P4) 20260926: golden format "V%d.%d.%d.%d" (with the V) -> "V12.13.902.0".  FormShow stores it in
//  RS232Version (MainForm.cpp:410), which is what StatusBar1 shows (:47), what "[Program Start]" logs (:524) and
//  what the Handler receives in MSG_CMD_Version (:633).
AnsiString VerInfo::GetFileVersion()
{
    AnsiString sret="";
    unsigned short iFileVerMajor=0, iFileVerMinor=0, iFileVerRelease=0, iFileVerBuild=0;
    //AnsiString strFilePath=Application->ExeName;                             //AI(W906-GB-P4) 20260926: no RS232Standard.exe in-process (see GetAppVersion); the name is informational only
    AnsiString strFilePath="RS232Standard.exe";
    VerInfo().GetAppVersion(strFilePath, iFileVerMajor, iFileVerMinor, iFileVerRelease, iFileVerBuild);
    sret=AnsiString().sprintf("V%d.%d.%d.%d", iFileVerMajor, iFileVerMinor, iFileVerRelease, iFileVerBuild);
    return sret;
}

//==============================================================================
//  MainForm.cpp:2564-2576 (declared at MainForm.h:448, outside the class)
//==============================================================================
void WriteDataToFile(char* cFilePath, char* cData, int iSize)
{
    DWORD wtfz;
    HANDLE Fp;

    //AI(W906-GB-P4) 20260926: golden quirk kept -- FILE_SHARE_WRITE (0x2) is passed as dwDesiredAccess, where the
    //  same bit is FILE_WRITE_DATA, so the handle can write; share mode 0; CREATE_ALWAYS truncates.  No golden caller
    //  (grep of every golden *.cpp / *.h: only this definition and the MainForm.h:448 declaration).
    Fp=CreateFile(cFilePath, FILE_SHARE_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if(Fp!=INVALID_HANDLE_VALUE)
    {
        WriteFile(Fp, cData, iSize, &wtfz, NULL);
        CloseHandle(Fp);
    }
}
//------------------------------------------------------------------------------

}  // namespace rs232std
