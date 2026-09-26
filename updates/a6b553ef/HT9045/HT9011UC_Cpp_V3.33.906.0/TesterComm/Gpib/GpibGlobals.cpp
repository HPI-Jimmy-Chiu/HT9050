// ===========================================================================
//  TesterComm/Gpib/GpibGlobals.cpp -- H9046_32GPIB (GPIB bridge) globals and the cmydef.cpp free functions,
//  in namespace gpibbridge (see GpibBridge.h for why the bridge lives in its own namespace).
//
//  AI(W906-GB-P1) 20260926.  Golden: D:\GPIB9045\GPIB_Code_32Site_V12.13.905.0_20260525 (Big5 -> UTF-8).
//    * cmydef.cpp:11-72        global definitions, initial values verbatim
//    * cmydef.cpp:74-320       WriteGpibString / CheckAndReadIniData x5 / WriteIniValueData / WriteIniData x4 /
//                              GetTimeInfo / CheckFileExist / MyDeCodeASCII -- bodies verbatim
//    * cmydef.cpp:397-415      VerInfo::GetSVNRev / VerInfo::GetFileVersion (version-resource read replaced,
//                              see GetBridgeAppVersion below)
//    * MessageDef.cpp:13,15,233-234   GPIBVersion / GPIBVersionCheck / GGpib2Handler / GHandler2Gpib
//    * Main.cpp:32, 36-107, 619, 1456-1457   file-scope globals
//    * V906 only: g_bridgeLife + ResetBridgeGlobals() -- every global above back to its golden initialiser, for
//      the in-process bridge restart that replaces golden's exe relaunch (GpibBridge.h "V906 bridge life")
//    * RS232.cpp:20 fRS232Main, DummyArt.cpp:17 fDummyART
//  NOT here (on purpose):
//    * Check2Dsum / CheckBINONString (Main.cpp:956 / :991) -- another worker's file.
//    * VerInfo Windows version-resource plumbing (cmydef.cpp:322-395, 417-538: m_ClearData, m_SetFileName,
//      m_strGetFixedFileVersion, m_strGetFixedProductVersion, GetAppVersion, m_GetVerInfo) -- the fixed
//      header's VerInfo declares none of those members, and in-process there is no bridge exe to read.
//    * MessageDef.cpp:14 RS232Version and :229-230 HGpib2Handler / HHandler2Gpib ("For Handler") -- the bridge
//      never reads them; the Handler keeps its own copies in :: (MessageDef.cpp of the V906 tree).
//    * MSG_CMD_* constants -- shared from :: MessageDef.cpp (verified 20260926: all 204 names/values identical
//      between golden GPIB MessageDef.cpp and V906 MessageDef.cpp).
//
//  SHARED-DATA WARNING (golden paths kept verbatim, CLAUDE.md "已知的共用風險"):
//    asHGeneralPath = "D:\\HT9045\\system\\Gerneral.ini" is the production-shared Handler ini.  golden reads it
//    through CheckAndReadIniData (Main.cpp:535,536,4336,4367), which WRITES the default back when a key is
//    missing.  asGeneralPath / WriteGpibString hit the real D:\GPIB9045\system of this machine.  The two
//    AnsiString paths can be re-pointed by the engine after ResetBridgeGlobals and before FormCreate; the GpibString.dat literal in
//    WriteGpibString (cmydef.cpp:80) cannot (golden literal, kept).
// ===========================================================================
#include "TesterComm/Gpib/GpibBridge.h"

#include <io.h>      // _open / _close   (golden cmydef.cpp:1 <io.h>,    for _rtl_open / _rtl_close)
#include <fcntl.h>   // _O_RDONLY        (golden cmydef.cpp:2 <fcntl.h>, for O_RDONLY)
#include <string.h>  // memset / memcpy  (ResetBridgeGlobals)

namespace gpibbridge {

//------------------------------------------------------------------------------
//  cmydef.cpp:11-72
//------------------------------------------------------------------------------
LAST_GENERAL_SET LastSet;
bool InitialOK=false;
Word SystemHour=9999, SystemMin=9999, SystemSec=9999, SystemMSec=9999;
Word SystemYear=9999, SystemMonth=9999, SystemDate;
char CurrentDir[256];
bool IsTest=false;
bool bNeedRetest=false;                                                         //Steven 20201022 : For RFMD
bool bESCIsOpen=false;                                                          //Steven 20201022 : For RFMD
bool bMustWaitESC=false;                                                        //Steven 20201022 : For RFMD

char cAlarmTime[64];

TStringList *sBarCode;                                                          //Steven 20150713 : Add 2D code
TStringList *sBarCode_ASE_CL;                                                   //KaiChen 20191126 ：中壢日月光，2D 回傳格式
TStringList *sGPIBSetting;                                                      //Steven 20160912 : 加上保護

const int iBufferCount=2560;

int  iLotMode=0;                                                                //jou 2015-09-21 Auto Retest function
int  iLotModeGPIB=0;                                                            //jou 2015-09-21 Auto Retest function
int  iRunStartMode=-1;                                                          //jou 2015-09-21 Auto Retest function

bool bECHO_FlagRCMD=false;                                                      //jou 2015-09-21 Auto Retest function
bool bECHO_FlagECID=false;                                                      //jou 2015-09-21 Auto Retest function
bool bECHO_FlagSVID=false;                                                      //jou 2015-09-21 Auto Retest function
bool bECHO_FlagReset=false;                                                     //jou 2015-09-21 Auto Retest function

int iTjModeChangeDelayTime = 0;                                                 //Ifor 20180108 : add AMD Tc Tj Mode Change Delay ---------------------------
int iTModeData[4] = {-1,-1,-1,-1};
bool bChangeTjMode = false;

int iDoubleContact=0;

bool bSupport32Bin  = false;                                                    //Steven 20121112 : 支援32Bin
bool bManualTest    = false;
int iResult[MAX_SITE_COUNT];
AnsiString sMachineStateDecade;                                                 //JerryYang 20151109 機台狀態   //JerryYang 20160630 START:RS232加入GPIB
int iMacStateStrLength;                                                         //JerryYang 20151109 字串長度
char cMachineStateDec[6];                                                       //JerryYang 20151109 轉為ASCII碼
AnsiString sTestBinCount;                                                       //JerryYang 20160308 各Bin數量
char cTestBinCount[1024];                                                       //JerryYang 20160308 各Bin數量
AnsiString sSoakTime;                                                           //JerryYang 20160318 回傳加熱時間
char cSoakTime[10];                                                             //JerryYang 20160318 回傳加熱時間
AnsiString sJamCode;                                                            //JerryYang 20160323 回傳Jam code
char cJamCode[10];                                                              //JerryYang 20160323 回傳Jam code
AnsiString sSiteMap;                                                            //JerryYang 20160324 回傳SiteMap
char cSiteMap[256];                                                             //JerryYang 20160324 回傳SiteMap
AnsiString sAllMassTemp;                                                        //JerryYang 20160330 回傳All Mass Temp
char cAllMassTemp[100];                                                         //JerryYang 20160330 回傳All Mass Temp
AnsiString sHandlerID;
char cHandlerID[100];                                                           //JerryYang 20160330 回傳All Mass Temp
AnsiString sSiteOnOff;
char cSiteOnOff[256];
AnsiString sNumOfSites;                                                         //JerryYang 20160318 回傳加熱時間
char cNumOfSites[10];                                                           //JerryYang 20160318 回傳加熱時間
const int iRS232Error   =-1;
const int iNoneTest     =0;                                                     //Steven 20231205 : 判斷RS232流程是否異常
const int iWaitReplyCE  =1;
const int iReplyCE      =2;
AnsiString sDoubleContactCount;                                                 //Isaac 20210706 : add MSG_CMD_DoubleContactCount指令，詢問handler doublecontact次數
char cDoubleContactCount[5];                                                    //Isaac 20210706 : add MSG_CMD_DoubleContactCount指令，詢問handler doublecontact次數
int CustomerCode=0;                                                             //kevin 20141218
//AI(W906-GB-P1) 20260926: golden cmydef.h:385 declares asCD_GET_HONTTECH_ALLTEMPERATURE extern but NO golden
//  file defines or reads it (grep of every golden *.cpp/*.h).  The fixed header re-declares it, so it is defined
//  here, empty, so that a future reference links instead of failing on a symbol golden never had.
AnsiString asCD_GET_HONTTECH_ALLTEMPERATURE;                                    // Frank 20150801 Add GetNowAllTemp? Command

//------------------------------------------------------------------------------
//  MessageDef.cpp (GPIB-side instances)
//------------------------------------------------------------------------------
AnsiString GPIBVersion="12.13.905.0";                                           // MessageDef.cpp:13
double GPIBVersionCheck=12.13;                                                  //Steven 20191007 : 改成判斷兩組版號, 所以使用Double

//For GPIB-------------
VM GGpib2Handler;                                                               // MessageDef.cpp:233  (static storage: zero-initialized)
MV *GHandler2Gpib=NULL;                                                         // MessageDef.cpp:234  //AI(W906-GB-P1) 20260926: golden `MV *GHandler2Gpib;` (static storage == NULL), explicit here
//---------------------

//------------------------------------------------------------------------------
//  Main.cpp:32 / RS232.cpp:20 / DummyArt.cpp:17 -- the three form pointers (created by GpibEngine)
//------------------------------------------------------------------------------
TSerialPoll *SerialPoll=NULL;                                                   // Main.cpp:32      golden `TSerialPoll *SerialPoll;`
TfRS232Main *fRS232Main=NULL;                                                   // RS232.cpp:20     golden `TfRS232Main *fRS232Main;`
TfDummyART  *fDummyART=NULL;                                                    // DummyArt.cpp:17  golden `TfDummyART *fDummyART;`

//------------------------------------------------------------------------------
//  Main.cpp:36-107
//------------------------------------------------------------------------------
AnsiString TempStr;                                                             //Steven 20160215 : 把 char 改成 AnsiString, 避免記憶體用太多或是溢位
AnsiString Sitemapstr;
AnsiString sOneCycleMsg;                                                        //Sam 20221103 : OneCycle 完後顯示訊息
bool bEnableThread=false;
//AI(W906-GB-P1) 20260926: golden `static int noncontroller=-1, result[TOTAL_SITE];` -- `static` dropped because
//  the fixed header declares both extern (the bridge body is split over several .cpp); still one per bridge.
int noncontroller=-1, result[TOTAL_SITE];
int iStart[TOTAL_SITE], RandomData[5]={1,2,5,7,8}, iGbibTask=1;
unsigned int iRecvCommand;                                                      //Steven 20141008 : iTimeOutSecond --> iRecvCommand
int oldGpibAddress=-1,GpibAddress=1,iMainTask=1;
bool bGpibMode=true;
AnsiString GpibString;                                                          //Steven 20160215 : 把 char 改成 AnsiString, 避免記憶體用太多或是溢位
bool bCatalystSimpleGPIB=false;
char PcName[255] ;                                                              //Steven 20110131 : 電腦名稱
unsigned long PcNameLen=255;                                                    //Steven 20110131 : 電腦名稱長度

AnsiString asGeneralPath="D:\\GPIB9045\\system\\general.ini";
AnsiString asHGeneralPath="D:\\HT9045\\system\\Gerneral.ini";

char szRunState[]={"FR1\r\n"};
char szIdelState[]={"FR0\r\n"};
//char szVersion[128]={""};
AnsiString szMachineType="HT_9045\r\n";                                         //Steven 20140910 : char --> AnsiString
AnsiString szHadnlerID="";
AnsiString sFolder="";                                                          //kevin 20130516 記錄LOG FILE
int iUseGPIBFormat=0;                                                           //Steven 20140920 : For 矽格阮瑋民的要求,不能用Handler ID
AnsiString sGPIBVersion = "";                                                   //wei 20151125 Add CHKMATCH? Command

AnsiString sGPIBIDNCmd = "";                                                    //Ifor 20180201 add

bool bHanaDummyTest=false;
int iSimuCount=0;
bool bSimulate=false;
bool bNeedInital;
HTimer TimerFullSite;                                                           //Steven 20141016 : FullSite的Test Time Out
char cGpibARTData[256];                                                         //jou 2015-09-21 Auto Retest function
bool bCloseSiteHaveBinErr;                                                      //Steven 20141016 : 關Site不能有Bin       //Steven 20170214 (wei): 改成全域變數
int iRCMDBackupTask=1;                                                          //JerryYang 20190226 RCMD command
AnsiString asShowMemo="";
HTimer HT_TACS_ATN;                                                             //Ifor 20180919 (Steven) : add TACS ATN Time Out 改用計時方式不用Count
HTimer htDelay;
int iTestStart=0, iTestEnd=0;
bool bCheckTC[2]={false,false};                                                 //Ifor 20180111 : add AMD Tc Tj Mode Change Delay From 7045
bool bCheckTJ[2]={false,false};
clock_t clTjChangeStart;                                                        // 2016.06.30 , Joye , AMD Tc Tj Mode Change Delay
clock_t clTjChangeEnd;                                                          // 2016.06.30 , Joye , AMD Tc Tj Mode Change Delay
TDateTime dtComm1;
bool bTempHasReady=false;                                                       //Ifor 20181219 : add 溫度轉換完成
bool bSendGetTemp=false;                                                        //Ifor 20190227 : add 溫度取得旗標
int  iATC_Result[ATC_MAX_SITE*2];                                               //Ifor 20190227 : add 取得溫度
double dTC[ATC_MAX_SITE];                                                       //Ifor 20190227 : add 取得溫度
double dTJ[ATC_MAX_SITE];                                                       //Ifor 20190227 : add 取得溫度
int iATCUseChannel[TOTAL_SITE];                                                 //Ifor 20190307 : add ATC 使用的 Channel
int iTestMode=0;                                                                //Ifor 20190307 : add 測試模式
bool bNeedGetTj=false;                                                          //Ifor 20190320 : add
bool bCheckDiodeThermal[TOTAL_SITE];                                            //Ifor 20190528 : add Diode Thermal Check
bool bNEXTSTEP_CMD=false;                                                       //Ifor 20190606 : add NEXTSTEP CMD不可以Reset GPIB
bool bHasSendCheckCmd=false;                                                    //Ifor 20200211 : add ATC 狀態確認確保RS232通訊OK
int iHasNEXTSTEP2=0;                                                            //Ifor 20220613 : add 客戶要求接收到"BINON"要自動切還TC Mode 0:TC Mode 1:TJ Mode 2:Auto Switch TC
AnsiString sRecipe, s1, s2, s3;
int iSafedoorStatus= 25166336;
bool bIsTestStart = false;                                                      //AI(W906-GB-P1) 20260926: golden `static bool bIsTestStart = false;` -- `static` dropped (header extern)

int iATC_Use_Heat_Count=0;                                                      //Ifor 20201023 add: ATC 控制器數量
AnsiString asATC_SiteMapping="XXX";                                             //Ifor 20201030 add:送Site Mapping 資料給GPIB
double dBySiteTC[ATC_MAX_SITE];                                                 //Ifor 20190227 : add 取得溫度
double dBySiteTJ[ATC_MAX_SITE];                                                 //Ifor 20190227 : add 取得溫度
bool bHasSiteMapping=false;
int iSwitchTJDelay=0;                                                           //Ifor 20210303 add:收到TC/TJ Switch 訊號延遲5次不讀取溫度
bool bA10_3_Enable=false;
//------------------------------------------------------------------------------
HWND HMountWnd=NULL;

TQPF_Timer IsTestDelay;                                                         //20220616 wei 次數計數改為時間延遲
//------------------------------------------------------------------------------
Word OldSystemSec;                                                              // Main.cpp:619
HTimer WaitRequestDelay;                                                        // Main.cpp:1456
HTimer FRWaitRequestDelay;                                                      //Steven 20120112 : 進FR?沒有回應會卡死

//------------------------------------------------------------------------------
//  V906 bridge life (not golden) -- GpibBridge.h "V906 bridge life"
//------------------------------------------------------------------------------
unsigned long g_bridgeLife = 0;                                                 //AI(W906-GB-P1) 20260926: incremented by GpibEngine::Start only; ResetBridgeGlobals does not touch it

//AI(W906-GB-P1) 20260926: pristine images of the two timer classes, used only by ResetBridgeGlobals.
//  A golden fresh process gives each global timer static storage (every byte zero) and THEN runs its default ctor.
//  Both classes are copy-assignable (implicit copy assignment: no const / reference members, no user-declared copy
//  operations -- vclcompat/HTimer.h:92-188, myTimer.h:10-38), but neither ctor sets every member:
//    * HTimer() == Clear(): iTimerID is never set (vclcompat/HTimer.h:100-111);
//    * TQPF_Timer() sets PerformanceCounterOverhead (and rStart / rEnd inside the calibration loop) only; iCount,
//      rSetTime, rFreq, rStartDelay, rEndDelay are never set (myTimer.cpp:15-30).
//  So `x = HTimer();` / `x = TQPF_Timer();` would copy indeterminate members from the temporary, and memset +
//  placement new is not reliable either (GCC >= 6 -flifetime-dse may delete a memset that precedes a ctor).  These
//  two objects have static storage exactly like the golden globals (zero, then ctor), are never modified, and are
//  copy-assigned instead.
//  Residual difference, TQPF_Timer only: the ctor's calibration and its rStart / rEnd timestamps date from V906
//  process start, not from the bridge restart (golden: from exe start).  IsTestDelay's only uses (Main.cpp:1526,
//  :3950, :7698 -- SetSecAndOn / Off) re-arm rSetTime / rStart / rEnd before relying on them, and an un-armed Off()
//  (rEnd in the past) returns true either way.
static const HTimer     kFreshHTimer;
static const TQPF_Timer kFreshQPFTimer;

//AI(W906-GB-P1) 20260926: ResetBridgeGlobals -- golden's Handler relaunches the bridge exe (atester.cpp
//  RunTestProgram etc.) and every new process starts each global at its initialiser.  In-process GpibEngine::Start
//  calls this instead, before TSerialPoll is constructed.  Every global defined above is put back to its golden
//  initial value, in definition order: the explicit initialiser where golden has one, otherwise the static-storage
//  zero-initialisation golden relies on (0 / false / "" / NULL / default-constructed).
//  NOT reset: SerialPoll, fRS232Main, fDummyART, HMountWnd (engine-owned), g_bridgeLife, and the const globals
//  iBufferCount / iRS232Error / iNoneTest / iWaitReplyCE / iReplyCE (immutable).
//  NOTE for the engine: asGeneralPath / asHGeneralPath come back to the golden production paths here, so any
//  dry-run re-pointing of them must be done AFTER this call.
//  Function-local statics are not covered here (GpibBridge.h: re-armed by g_bridgeLife in their own function).
void ResetBridgeGlobals()
{
    // ---- cmydef.cpp:11-72 ----
    LastSet=LAST_GENERAL_SET();                                                 // :11  value-init == static zero-init: PODs 0/false, AnsiStrings ""
    InitialOK=false;                                                            // :12
    SystemHour=9999; SystemMin=9999; SystemSec=9999; SystemMSec=9999;           // :13
    SystemYear=9999; SystemMonth=9999; SystemDate=0;                            // :14  (golden: SystemDate has no initialiser)
    memset(CurrentDir, 0, sizeof(CurrentDir));                                  // :15
    IsTest=false;                                                               // :16
    bNeedRetest=false;                                                          // :17
    bESCIsOpen=false;                                                           // :18
    bMustWaitESC=false;                                                         // :19
    memset(cAlarmTime, 0, sizeof(cAlarmTime));                                  // :21
    sBarCode=NULL;                                                              // :23  re-created by the TSerialPoll ctor (Main.cpp:113)
    sBarCode_ASE_CL=NULL;                                                       // :24  re-created by the TSerialPoll ctor (Main.cpp:114)
    sGPIBSetting=NULL;                                                          // :25  golden new/delete it locally (Main.cpp:4528-4531)
    // :27 iBufferCount -- const, not reset
    iLotMode=0;                                                                 // :29
    iLotModeGPIB=0;                                                             // :30
    iRunStartMode=-1;                                                           // :31
    bECHO_FlagRCMD=false;                                                       // :33
    bECHO_FlagECID=false;                                                       // :34
    bECHO_FlagSVID=false;                                                       // :35
    bECHO_FlagReset=false;                                                      // :36
    iTjModeChangeDelayTime=0;                                                   // :38
    for(unsigned i=0; i<sizeof(iTModeData)/sizeof(iTModeData[0]); i++)         // :39  {-1,-1,-1,-1}
        iTModeData[i]=-1;
    bChangeTjMode=false;                                                        // :40
    iDoubleContact=0;                                                           // :42
    bSupport32Bin=false;                                                        // :44
    bManualTest=false;                                                          // :45
    memset(iResult, 0, sizeof(iResult));                                        // :46
    sMachineStateDecade="";                                                     // :47
    iMacStateStrLength=0;                                                       // :48
    memset(cMachineStateDec, 0, sizeof(cMachineStateDec));                      // :49
    sTestBinCount="";                                                           // :50
    memset(cTestBinCount, 0, sizeof(cTestBinCount));                            // :51
    sSoakTime="";                                                               // :52
    memset(cSoakTime, 0, sizeof(cSoakTime));                                    // :53
    sJamCode="";                                                                // :54
    memset(cJamCode, 0, sizeof(cJamCode));                                      // :55
    sSiteMap="";                                                                // :56
    memset(cSiteMap, 0, sizeof(cSiteMap));                                      // :57
    sAllMassTemp="";                                                            // :58
    memset(cAllMassTemp, 0, sizeof(cAllMassTemp));                              // :59
    sHandlerID="";                                                              // :60
    memset(cHandlerID, 0, sizeof(cHandlerID));                                  // :61
    sSiteOnOff="";                                                              // :62
    memset(cSiteOnOff, 0, sizeof(cSiteOnOff));                                  // :63
    sNumOfSites="";                                                             // :64
    memset(cNumOfSites, 0, sizeof(cNumOfSites));                                // :65
    // :66-69 iRS232Error / iNoneTest / iWaitReplyCE / iReplyCE -- const, not reset
    sDoubleContactCount="";                                                     // :70
    memset(cDoubleContactCount, 0, sizeof(cDoubleContactCount));                // :71
    CustomerCode=0;                                                             // :72
    asCD_GET_HONTTECH_ALLTEMPERATURE="";                                        // cmydef.h:385 (V906-only definition, see above)

    // ---- MessageDef.cpp:13,15,233-234 ----
    GPIBVersion="12.13.905.0";                                                  // :13  (FormCreate then re-assigns it from VerInfo, Main.cpp:378)
    GPIBVersionCheck=12.13;                                                     // :15
    memset(&GGpib2Handler, 0, sizeof(GGpib2Handler));                           // :233 (VM is plain C, MessageDef.h)
    GHandler2Gpib=NULL;                                                         // :234

    // ---- Main.cpp:32 / RS232.cpp:20 / DummyArt.cpp:17 -- SerialPoll / fRS232Main / fDummyART: engine-owned, not reset

    // ---- Main.cpp:36-107 ----
    TempStr="";                                                                 // :36
    Sitemapstr="";                                                              // :37
    sOneCycleMsg="";                                                            // :38
    bEnableThread=false;                                                        // :39
    noncontroller=-1;                                                           // :40
    memset(result, 0, sizeof(result));                                          // :40
    memset(iStart, 0, sizeof(iStart));                                          // :41
    RandomData[0]=1; RandomData[1]=2; RandomData[2]=5; RandomData[3]=7; RandomData[4]=8;   // :41  {1,2,5,7,8}
    iGbibTask=1;                                                                // :41
    iRecvCommand=0;                                                             // :42
    oldGpibAddress=-1;                                                          // :43
    GpibAddress=1;                                                              // :43
    iMainTask=1;                                                                // :43
    bGpibMode=true;                                                             // :44
    GpibString="";                                                              // :45
    bCatalystSimpleGPIB=false;                                                  // :46
    memset(PcName, 0, sizeof(PcName));                                          // :47
    PcNameLen=255;                                                              // :48
    asGeneralPath="D:\\GPIB9045\\system\\general.ini";                          // :50
    asHGeneralPath="D:\\HT9045\\system\\Gerneral.ini";                          // :51
    static_assert(sizeof(szRunState)==sizeof("FR1\r\n"), "szRunState must match its golden initialiser");
    memcpy(szRunState, "FR1\r\n", sizeof(szRunState));                          // :53
    static_assert(sizeof(szIdelState)==sizeof("FR0\r\n"), "szIdelState must match its golden initialiser");
    memcpy(szIdelState, "FR0\r\n", sizeof(szIdelState));                        // :54
    szMachineType="HT_9045\r\n";                                                // :56
    szHadnlerID="";                                                             // :57
    sFolder="";                                                                 // :58
    iUseGPIBFormat=0;                                                           // :59
    sGPIBVersion="";                                                            // :60
    sGPIBIDNCmd="";                                                             // :62
    bHanaDummyTest=false;                                                       // :64
    iSimuCount=0;                                                               // :65
    bSimulate=false;                                                            // :66
    bNeedInital=false;                                                          // :67
    TimerFullSite=kFreshHTimer;                                                 // :68
    memset(cGpibARTData, 0, sizeof(cGpibARTData));                              // :69
    bCloseSiteHaveBinErr=false;                                                 // :70
    iRCMDBackupTask=1;                                                          // :71
    asShowMemo="";                                                              // :72
    HT_TACS_ATN=kFreshHTimer;                                                   // :73
    htDelay=kFreshHTimer;                                                       // :74
    iTestStart=0;                                                               // :75
    iTestEnd=0;                                                                 // :75
    bCheckTC[0]=false; bCheckTC[1]=false;                                       // :76
    bCheckTJ[0]=false; bCheckTJ[1]=false;                                       // :77
    clTjChangeStart=0;                                                          // :78
    clTjChangeEnd=0;                                                            // :79
    dtComm1=TDateTime();                                                        // :80
    bTempHasReady=false;                                                        // :81
    bSendGetTemp=false;                                                         // :82
    memset(iATC_Result, 0, sizeof(iATC_Result));                                // :83
    for(int i=0; i<ATC_MAX_SITE; i++) dTC[i]=0.0;                               // :84
    for(int i=0; i<ATC_MAX_SITE; i++) dTJ[i]=0.0;                               // :85
    memset(iATCUseChannel, 0, sizeof(iATCUseChannel));                          // :86
    iTestMode=0;                                                                // :87
    bNeedGetTj=false;                                                           // :88
    for(int i=0; i<TOTAL_SITE; i++) bCheckDiodeThermal[i]=false;                // :89
    bNEXTSTEP_CMD=false;                                                        // :90
    bHasSendCheckCmd=false;                                                     // :91
    iHasNEXTSTEP2=0;                                                            // :92
    sRecipe=""; s1=""; s2=""; s3="";                                            // :93
    iSafedoorStatus=25166336;                                                   // :94
    bIsTestStart=false;                                                         // :95
    iATC_Use_Heat_Count=0;                                                      // :97
    asATC_SiteMapping="XXX";                                                    // :98
    for(int i=0; i<ATC_MAX_SITE; i++) dBySiteTC[i]=0.0;                         // :99
    for(int i=0; i<ATC_MAX_SITE; i++) dBySiteTJ[i]=0.0;                         // :100
    bHasSiteMapping=false;                                                      // :101
    iSwitchTJDelay=0;                                                           // :102
    bA10_3_Enable=false;                                                        // :103
    // :105 HMountWnd -- engine-owned, not reset
    IsTestDelay=kFreshQPFTimer;                                                 // :107
    OldSystemSec=0;                                                             // :619
    WaitRequestDelay=kFreshHTimer;                                              // :1456
    FRWaitRequestDelay=kFreshHTimer;                                            // :1457
    // g_bridgeLife -- engine-owned, not reset
}

//==============================================================================
//  cmydef.cpp functions
//==============================================================================
//AI(W906-GB-P1) 20260926: vclcompat TIniFile has ReadFloat but no WriteFloat (vclcompat/IniFiles.h banner; the
//  Handler's common.cpp never writes floats that way).  BCB6 TCustomIniFile::WriteFloat's body is
//  `WriteString(Section, Name, FloatToStr(Value))`; this helper is exactly that body, so the two golden
//  `INIFile->WriteFloat(...)` lines (cmydef.cpp:89, :156) keep golden's on-disk text (FloatToStr, 15 significant
//  digits -- NOT the Handler common.cpp "%0.4f" convention).
static void IniWriteFloat(TIniFile *INIFile, const AnsiString& Section, const AnsiString& Name, double Value)
{
    INIFile->WriteString(Section, Name, FloatToStr(Value));
}
//------------------------------------------------------------------------------
void WriteGpibString()
{
    if(LastSet.sGpibString!="")
    {
        TStringList *myTList=new TStringList();
        myTList->Add(LastSet.sGpibString);
        myTList->SaveToFile("d:\\GPIB9045\\system\\GpibString.dat");
        myTList->Clear();                                                       //Ifor 20180917 (Steven) : Add TStringList 刪除前需先 Clean
        delete myTList;
    }
}
//------------------------------------------------------------------------------
double CheckAndReadIniData(AnsiString FileName, AnsiString Group, AnsiString Name, double Value)
{
    TIniFile *INIFile=new TIniFile(FileName);
    if(!INIFile->ValueExists(Group, Name))  IniWriteFloat(INIFile, Group, Name, Value);   //AI(W906-GB-P1) 20260926: golden INIFile->WriteFloat(Group, Name, Value)
    else                                    Value=INIFile->ReadFloat(Group, Name, Value);
    delete INIFile;
    return Value;
}
//------------------------------------------------------------------------------
int CheckAndReadIniData(AnsiString FileName, AnsiString Group, AnsiString Name, int Value)
{
    TIniFile *INIFile=new TIniFile(FileName);
    if(!INIFile->ValueExists(Group, Name))  INIFile->WriteInteger(Group, Name, Value);
    else                                    Value=INIFile->ReadInteger(Group, Name, Value);
    delete INIFile;
    return Value;
}
//------------------------------------------------------------------------------
bool CheckAndReadIniData(AnsiString FileName, AnsiString Group, AnsiString Name, bool Value)
{
    TIniFile *INIFile=new TIniFile(FileName);
    if(!INIFile->ValueExists(Group, Name))  INIFile->WriteBool(Group, Name, Value);
    else                                    Value=INIFile->ReadBool(Group, Name, Value);
    delete INIFile;
    return Value;
}
//------------------------------------------------------------------------------
AnsiString CheckAndReadIniData(AnsiString FileName, AnsiString Group, AnsiString Name, AnsiString Value)
{
    TIniFile *INIFile = new TIniFile(FileName);
    if(!INIFile->ValueExists(Group, Name))  INIFile->WriteString(Group, Name, Value);
    else                                    Value=INIFile->ReadString(Group, Name, Value);
    delete INIFile;
    return Value;
}
//------------------------------------------------------------------------------
TDateTime CheckAndReadIniData(AnsiString FileName, AnsiString Group, AnsiString Name, TDateTime Value)
{
    TIniFile *INIFile = new TIniFile(FileName);
    if(!INIFile->ValueExists(Group, Name))  INIFile->WriteDateTime(Group, Name, Value);
    else                                    Value=INIFile->ReadDateTime(Group, Name, Value);
    delete INIFile;
    return Value;
}
//------------------------------------------------------------------------------
AnsiString WriteIniValueData(AnsiString FileName, AnsiString Group, AnsiString Name, AnsiString Value)
{
    TIniFile *INIFile = new TIniFile(FileName);
    INIFile->WriteString(Group, Name, Value);
    delete INIFile;
    return Value;
}
//------------------------------------------------------------------------------
void WriteIniData(AnsiString FileName, AnsiString Group, AnsiString Name, bool bValue)
{
    TIniFile *INIFile = new TIniFile(FileName);
    INIFile->WriteBool(Group, Name, bValue);
    delete INIFile;
}
//------------------------------------------------------------------------------
void WriteIniData(AnsiString FileName, AnsiString Group, AnsiString Name, int Value)
{
    TIniFile *INIFile = new TIniFile(FileName);
    INIFile->WriteInteger(Group, Name, Value);
    delete INIFile;
}
//------------------------------------------------------------------------------
void WriteIniData(AnsiString FileName, AnsiString Group, AnsiString Name, double Value)
{
    TIniFile *INIFile = new TIniFile(FileName);
    IniWriteFloat(INIFile, Group, Name, Value);                                //AI(W906-GB-P1) 20260926: golden INIFile->WriteFloat(Group, Name, Value)
    delete INIFile;
}
//------------------------------------------------------------------------------
void WriteIniData(AnsiString FileName, AnsiString Group, AnsiString Name, AnsiString Value)
{
    TIniFile *INIFile = new TIniFile(FileName);
    INIFile->WriteString(Group, Name, Value);
    delete INIFile;
}
//------------------------------------------------------------------------------
void GetTimeInfo()
{
    static TDateTime dtPresent;
    //AI(W906-GB-P1) 20260926: bridge restart (g_bridgeLife): no re-arm needed -- dtPresent is pure scratch, assigned
    //  from Now() on the next line before every read, so a value left by the previous bridge life is never seen.
    dtPresent=Now();
    DecodeDate(dtPresent, SystemYear, SystemMonth, SystemDate);
    DecodeTime(dtPresent, SystemHour, SystemMin, SystemSec, SystemMSec);
}
//------------------------------------------------------------------------------
bool CheckFileExist(char *cFName)
{
    int fhandle;
    //AI(W906-GB-P1) 20260926: BCB6 RTL _rtl_open/_rtl_close -> CRT _open/_close (<io.h>), O_RDONLY -> _O_RDONLY
    //  (<fcntl.h>): same "can the path be opened read-only" test, -1 on failure.
    if((fhandle=_open(cFName, _O_RDONLY))==-1)
        return false;
    _close(fhandle);
    return true;
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
//  VerInfo (cmydef.cpp:397-415)
//------------------------------------------------------------------------------
//AI(W906-GB-P1) 20260926: golden VerInfo().GetAppVersion(Application->ExeName, ...) (cmydef.cpp:368-395) reads the
//  VS_FIXEDFILEINFO of H9046_32GPIB.exe.  In-process there is no bridge exe -- Application->ExeName would be the
//  Handler's exe and report the HANDLER's version -- so the four numbers are the bridge's own version resource as
//  golden builds it: H9046_32GPIB.bpr:79-82 [Version Info] MajorVer=12 MinorVer=13 Release=905 Build=0, the same
//  baseline as MessageDef.cpp:13 GPIBVersion "12.13.905.0".  Bump these together with GPIBVersion.
//  Consequence kept from golden: FormCreate's `GPIBVersion=VerInfo().GetFileVersion();` (Main.cpp:378) turns
//  GPIBVersion into "V12.13.905.0" (with the V), which is what the Tester sees in "NEWOI-%s-HT9045W"
//  (Main.cpp:538) / "Rogers %s" (:5389) and what the Handler receives in MSG_CMD_Version (:3121; the Handler
//  strips the V, V899 main.cpp:15182-15183 "Steven 20250616 : 多了個V").
static void GetBridgeAppVersion(unsigned short& major, unsigned short& minor, unsigned short& build, unsigned short& revision)
{
    major   =12;
    minor   =13;
    build   =905;
    revision=0;
}
//------------------------------------------------------------------------------
AnsiString VerInfo::GetSVNRev()
{
    AnsiString sret="";
    unsigned short iFileVerMajor=0, iFileVerMinor=0, iFileVerRelease=0, iFileVerBuild=0;
    //AnsiString strFilePath=Application->ExeName;                             //AI(W906-GB-P1) 20260926: no bridge exe, see GetBridgeAppVersion
    GetBridgeAppVersion(iFileVerMajor, iFileVerMinor, iFileVerRelease, iFileVerBuild);  // golden: VerInfo().GetAppVersion(strFilePath, ...)
    sret=AnsiString().sprintf("%d.%d", iFileVerRelease, iFileVerBuild);
    return sret;
}
//------------------------------------------------------------------------------
AnsiString VerInfo::GetFileVersion()
{
    AnsiString sret="";
    unsigned short iFileVerMajor=0, iFileVerMinor=0, iFileVerRelease=0, iFileVerBuild=0;
    //AnsiString strFilePath=Application->ExeName;                             //AI(W906-GB-P1) 20260926: no bridge exe, see GetBridgeAppVersion
    GetBridgeAppVersion(iFileVerMajor, iFileVerMinor, iFileVerRelease, iFileVerBuild);  // golden: VerInfo().GetAppVersion(strFilePath, ...)
    sret=AnsiString().sprintf("V%d.%d.%d.%d", iFileVerMajor, iFileVerMinor, iFileVerRelease, iFileVerBuild);
    return sret;
}
//------------------------------------------------------------------------------

}  // namespace gpibbridge
