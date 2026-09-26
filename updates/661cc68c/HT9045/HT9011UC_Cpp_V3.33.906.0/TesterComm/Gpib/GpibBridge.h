// ===========================================================================
//  TesterComm/Gpib/GpibBridge.h -- H9046_32GPIB (GPIB bridge) translated into the V906 process.
//
//  AI(W906-GB-P1) 20260926: Tester-comm plan P1.  Golden: D:\GPIB9045\GPIB_Code_32Site_V12.13.905.0_20260525
//  (Main.h/Main.cpp TSerialPoll, cmydef.h/.cpp, MessageDef, RS232.h/.cpp AMD aux, DummyArt, Unit2 TMyThread),
//  plus Steven's 20260926 HT9050 lines.
//
//  WHY A NAMESPACE: golden runs the bridge as its own process, so its globals (LastSet, asGeneralPath,
//  GGpib2Handler, GHandler2Gpib, GPIBVersion, iStart, bSimulate, IsTest ...) never meet the Handler's globals of
//  the same names.  In-process they would collide, so every bridge global and the TSerialPoll class live in
//  namespace gpibbridge.  Inside the namespace, unqualified names resolve to the bridge's copy first, which keeps
//  the golden bodies textually unchanged.
//
//  TRANSLATION RULES for every Gpib*.cpp (keep them uniform):
//    * Bodies are golden text; `__fastcall` dropped (tree convention).  UI widgets are the headless vclcompat
//      stand-ins declared below with golden member names, so `chkUpperCase->Checked`, `StatusBar1->Panels->
//      Items[2]->Text`, `MY_DUT_PAL[i]->cbBin->ItemIndex` compile as written.  Their state is what the web page
//      shows (P7, copied out under TSerialPoll::uiMutex).
//    * SendMessage(HMountWnd, WM_COPYDATA, ...)  -> PostToHandler(pcp)   (synchronous, via the SyncMailbox)
//    * FindWindow("TfMain", ...) / HMountWnd      -> kept; HMountWnd is a non-NULL token while the Handler side is
//                                                   attached (see GpibEngine), NULL otherwise.
//    * Close()                                    -> RequestClose(reason): the engine stops at the next iteration.
//    * Application->ProcessMessages()             -> no-op (there is no message loop; the mailbox is pumped by
//                                                   the TesterComm loop itself).
//    * MessageDlg / ShowMessage                   -> WriteLog + UiNotice (shown on the web page), never blocks.
//    * NI GPIB calls (ibwrt/ibrd/...)              -> the gpibbridge:: wrappers in GpibDriver.h, which refresh
//                                                   ibsta/iberr/ibcnt after every call like the NI static lib.
//    * Anything not portable is gated with `#if 0 // TODO(W906-GB-P1): <reason>` and listed in
//      docs/TESTERCOMM_PORT_LEDGER.md; never silently dropped.
// ===========================================================================
#ifndef TESTERCOMM_GPIB_GPIBBRIDGE_H
#define TESTERCOMM_GPIB_GPIBBRIDGE_H

#include "vclcompat/vcl_compat.h"
#include "vclcompat/Controls.h"
#include "vclcompat/Comm.h"
#include "vclcompat/HTimer.h"
#include "myTimer.h"
#include "MessageDef.h"            // VM / MV packet types and MSG_CMD_* values (shared contract; instances below)
#include "WebBridge/Sync.h"
#include "TesterComm/Gpib/GpibDriver.h"

#include <ctime>
#include <map>
#include <string>
#include <vector>

namespace testercomm { class SyncMailbox; }

namespace gpibbridge {

// vclcompat::HTimer has deliberately no global using (vclcompat/HTimer.h:63-65: three TUs typedef their own
// global HTimer).  A namespace-scoped using is invisible to them and lets golden `HTimer x;` compile here.
using vclcompat::HTimer;

typedef unsigned char Byte;   // BCB System.hpp (same spelling as Interface/InterfaceSYS.h:36)

// ---------------------------------------------------------------------------
//  cmydef.h (GPIB copy)
// ---------------------------------------------------------------------------
#define GPIB_ALIAS "HT9045"
const int MAX_SITE_COUNT = 32;
const int StrLength      = 2560;
const int ATC_MAX_SITE   = 32;
const int TOTAL_SITE     = 32;

const int InterfaceType_ADVAN_Type1  = 0;
const int InterfaceType_256Bin       = 1;
const int InterfaceType_16Bin        = 2;
const int InterfaceType_32Bin        = 3;
const int InterfaceType_SPEA_Type    = 4;
const int InterfaceType_16BinGS      = 5;
const int InterfaceType_32BinGS      = 6;
const int InterfaceType_15BinT6577   = 7;
const int InterfaceType_15BinQorvo   = 8;
const int InterfaceType_Delta_Castle = 9;

typedef struct
{
    AnsiString sLastFile;
    AnsiString sGpibString;
    AnsiString MachName;
    bool bStringLength;
    bool bRecordDatd;
    bool bUpperCase;
    bool bGpib8080;
    int GpibAddress;
    int iTimeOut;
    int MachineType;
    bool bSQR41;
    bool bBinonEcho;
    bool bFULLSITES;
    bool bFullSiteTimeOut;
    bool bUseBarcodeFunction;
    bool bHasBarCode;
    bool bSRQC0;
    bool bFlagRCMD;
    bool bFlagSVID;
    bool bFlagECID;
    int  iTesterType;
    int  iTesterMode;
    bool bDummyART;
    int  iLotCount;
    int  iDummyARTTask;
    AnsiString sLotID;
    int  iTACS_ATNTimeOut;
    int  iMyGpibWriteRetry;
    int  iMyGpibWriteThreshold;
    int  iMyGpibWriteWaitMS;
    bool bMyGpibWriteVerboseLog;
    int  i2DIDFormat;
    bool bConfigureSRQ;
    bool bHaveContactorInfo;
    bool bEnableManualStart;
    bool bAlarmWhen2DFuncMisMatch;
    AnsiString sPackageType;
    bool bUseAMDFunction;
    bool bGPIBWriteWithout_r_n;
    bool bRunHANA_ART;
    bool bTryHANA_ART;
} LAST_GENERAL_SET;

extern LAST_GENERAL_SET LastSet;
extern bool InitialOK;
extern Word SystemHour, SystemMin, SystemSec, SystemMSec;
extern Word SystemYear, SystemMonth, SystemDate;
extern char CurrentDir[256];
extern bool IsTest;
extern bool bNeedRetest;
extern bool bESCIsOpen;
extern bool bMustWaitESC;
extern TStringList *sBarCode;
extern TStringList *sBarCode_ASE_CL;
extern TStringList *sGPIBSetting;
extern const int iBufferCount;
void WriteGpibString();
TDateTime  CheckAndReadIniData(AnsiString FileName, AnsiString Group, AnsiString Name, TDateTime Value);
AnsiString CheckAndReadIniData(AnsiString FileName, AnsiString Group, AnsiString Name, AnsiString Value);
int        CheckAndReadIniData(AnsiString FileName, AnsiString Group, AnsiString Name, int Value);
bool       CheckAndReadIniData(AnsiString FileName, AnsiString Group, AnsiString Name, bool Value);
double     CheckAndReadIniData(AnsiString FileName, AnsiString Group, AnsiString Name, double Value);
AnsiString WriteIniValueData(AnsiString FileName, AnsiString Group, AnsiString Name, AnsiString Value);
void WriteIniData(AnsiString FileName, AnsiString Group, AnsiString Name, bool bValue);
void WriteIniData(AnsiString FileName, AnsiString Group, AnsiString Name, int Value);
void WriteIniData(AnsiString FileName, AnsiString Group, AnsiString Name, double Value);
void WriteIniData(AnsiString FileName, AnsiString Group, AnsiString Name, AnsiString Value);
void GetTimeInfo();
bool CheckFileExist(char *cFName);
extern char cAlarmTime[64];
extern AnsiString asCD_GET_HONTTECH_ALLTEMPERATURE;
extern int  iLotMode;
extern int  iRunStartMode;
extern int  iLotModeGPIB;
extern bool bECHO_FlagRCMD;
extern bool bECHO_FlagECID;
extern bool bECHO_FlagSVID;
extern bool bECHO_FlagReset;
extern int iTjModeChangeDelayTime;
extern int iTModeData[4];
extern bool bChangeTjMode;
extern int iDoubleContact;
extern bool bSupport32Bin;
extern bool bManualTest;
extern int iResult[MAX_SITE_COUNT];
extern AnsiString sMachineStateDecade;
extern int iMacStateStrLength;
extern char cMachineStateDec[6];
extern AnsiString sTestBinCount;
extern char cTestBinCount[1024];
extern AnsiString sSoakTime;
extern char cSoakTime[10];
extern AnsiString sJamCode;
extern char cJamCode[10];
extern AnsiString sSiteMap;
extern char cSiteMap[256];
extern AnsiString sAllMassTemp;
extern AnsiString sHandlerID;
extern char cAllMassTemp[100];
extern char cHandlerID[100];
extern AnsiString sSiteOnOff;
extern char cSiteOnOff[256];
extern AnsiString sNumOfSites;
extern char cNumOfSites[10];
extern int CustomerCode;
extern const int iRS232Error;
extern const int iNoneTest;
extern const int iWaitReplyCE;
extern const int iReplyCE;
extern AnsiString sDoubleContactCount;
extern char cDoubleContactCount[5];
AnsiString MyDeCodeASCII(int iInPut);

// golden cmydef VerInfo: version of the running exe.  In-process there is no separate bridge exe, so the
// translation returns the fixed bridge version (see GpibGlobals.cpp).
class VerInfo
{
public:
    VerInfo() {}
    AnsiString GetFileVersion();
    AnsiString GetSVNRev();
};

// ---------------------------------------------------------------------------
//  MessageDef instances (GPIB side copies; the Handler keeps its own in ::)
// ---------------------------------------------------------------------------
extern VM GGpib2Handler;            // bridge -> Handler packet
extern MV *GHandler2Gpib;           // points at the Handler -> bridge packet being processed
extern AnsiString GPIBVersion;      // golden MessageDef.cpp "12.13.905.0"
extern double GPIBVersionCheck;

// ---------------------------------------------------------------------------
//  Main.cpp file-scope globals (golden Main.cpp:36-107)
// ---------------------------------------------------------------------------
extern AnsiString TempStr;
extern AnsiString Sitemapstr;
extern AnsiString sOneCycleMsg;
extern bool bEnableThread;
extern int noncontroller;           // golden `static` at file scope: one per bridge
extern int result[TOTAL_SITE];      // golden `static` at file scope
extern int iStart[TOTAL_SITE];
extern int RandomData[5];
extern int iGbibTask;
extern unsigned int iRecvCommand;
extern int oldGpibAddress, GpibAddress, iMainTask;
extern bool bGpibMode;
extern AnsiString GpibString;
extern bool bCatalystSimpleGPIB;
extern char PcName[255];
extern unsigned long PcNameLen;
extern AnsiString asGeneralPath;    // "D:\\GPIB9045\\system\\general.ini"
extern AnsiString asHGeneralPath;   // "D:\\HT9045\\system\\Gerneral.ini"
extern char szRunState[];
extern char szIdelState[];
extern AnsiString szMachineType;
extern AnsiString szHadnlerID;
extern AnsiString sFolder;
extern int iUseGPIBFormat;
extern AnsiString sGPIBVersion;
extern AnsiString sGPIBIDNCmd;
extern bool bHanaDummyTest;
extern int iSimuCount;
extern bool bSimulate;
extern bool bNeedInital;
extern HTimer TimerFullSite;
extern char cGpibARTData[256];
extern bool bCloseSiteHaveBinErr;
extern int iRCMDBackupTask;
extern AnsiString asShowMemo;
extern HTimer HT_TACS_ATN;
extern HTimer htDelay;
extern int iTestStart, iTestEnd;
extern bool bCheckTC[2];
extern bool bCheckTJ[2];
extern clock_t clTjChangeStart;
extern clock_t clTjChangeEnd;
extern TDateTime dtComm1;
extern bool bTempHasReady;
extern bool bSendGetTemp;
extern int  iATC_Result[ATC_MAX_SITE * 2];
extern double dTC[ATC_MAX_SITE];
extern double dTJ[ATC_MAX_SITE];
extern int iATCUseChannel[TOTAL_SITE];
extern int iTestMode;
extern bool bNeedGetTj;
extern bool bCheckDiodeThermal[TOTAL_SITE];
extern bool bNEXTSTEP_CMD;
extern bool bHasSendCheckCmd;
extern int iHasNEXTSTEP2;
extern AnsiString sRecipe, s1, s2, s3;
extern int iSafedoorStatus;
extern bool bIsTestStart;           // golden `static` at file scope
extern int iATC_Use_Heat_Count;
extern AnsiString asATC_SiteMapping;
extern double dBySiteTC[ATC_MAX_SITE];
extern double dBySiteTJ[ATC_MAX_SITE];
extern bool bHasSiteMapping;
extern int iSwitchTJDelay;
extern bool bA10_3_Enable;
extern HWND HMountWnd;
extern TQPF_Timer IsTestDelay;
extern Word OldSystemSec;           // golden Main.cpp:619
extern HTimer WaitRequestDelay;     // golden Main.cpp:1456
extern HTimer FRWaitRequestDelay;   // golden Main.cpp:1457

// ---- V906 bridge life (not golden) ----
// Golden runs one bridge per process and the Handler relaunches the exe often (atester.cpp RunTestProgram after a
// Tester timeout, AMD 2DIDFormat change, CloseGpibProgram on test-type change), so every life starts from the
// initial values of the globals and function-local statics.  In-process that needs doing by hand:
//   * ResetBridgeGlobals() (GpibGlobals.cpp) puts every global of this header back to its golden initialiser;
//     GpibEngine::Start calls it before constructing TSerialPoll.  SerialPoll / fRS232Main / fDummyART and
//     HMountWnd are the engine's and are not touched.
//   * g_bridgeLife is incremented by GpibEngine::Start; a golden function with function-local statics re-arms
//     them when it sees a new value (//AI(W906-GB-P1) "re-arm" block right after the statics).
extern unsigned long g_bridgeLife;
void ResetBridgeGlobals();

// golden Main.cpp free functions
AnsiString Check2Dsum(AnsiString asChk);   // :956
bool CheckBINONString(char *str);          // :991

// ---------------------------------------------------------------------------
//  Headless widget stand-ins missing from vclcompat (golden member names kept)
// ---------------------------------------------------------------------------
class TALed : public TControl
{
public:
    bool Value;
    TALed() : Value(false) {}
};

class TStatusPanel
{
public:
    AnsiString Text;
};

class TStatusPanels
{
public:
    enum { kCount = 8 };
    TStatusPanel* Items[kCount];
    TStatusPanels() { for (int i = 0; i < kCount; ++i) Items[i] = new TStatusPanel; }
    ~TStatusPanels() { for (int i = 0; i < kCount; ++i) delete Items[i]; }
};

class TStatusBar : public TControl
{
public:
    TStatusPanels* Panels;
    TStatusBar() : Panels(new TStatusPanels) {}
    ~TStatusBar() { delete Panels; }
};

class TTimer : public TControl
{
public:
    unsigned Interval;
    TTimer() : Interval(1000) {}
};

// golden MyDutPanel.h TMyDutPanel -- one per site (web: HTWidgets.makeDutPanel).
class TMyDutPanel
{
public:
    TGroupBox *gpSite;
    TComboBox *cbBin;
    TPanel    *plSite;
    TCheckBox *cbSiteOn;
    TLabel    *labOcr;
    int       _Index;
    bool      _Enable;
    explicit TMyDutPanel(int index);   // golden ctor minus Owner/Parent/geometry (GpibUi.cpp)
    ~TMyDutPanel();
};

// golden TMessage subset used by OnMyCopyMsg.
struct TMessage
{
    LPARAM LParam;
    WPARAM WParam;
};

// ---------------------------------------------------------------------------
//  TSerialPoll (golden Main.h), minus TForm
// ---------------------------------------------------------------------------
class TSerialPoll
{
public:
    // ---- golden __published widgets ----
    TStatusBar *StatusBar1;
    TPageControl *PageControl1;
    TTabSheet *TabSheet1;
    TTimer *Timer1;
    TTabSheet *TabSheet3;
    TMemo *Memo1;
    TLabel *lblGPIBWnd;
    TPanel *palButton;
    TPanel *Panel12;
    TALed *ALed1, *ALed2, *ALed3, *ALed4, *ALed5, *ALed6, *ALed7, *ALed8, *ALed9, *ALed10, *ALed11, *ALed12, *ALed13;
    TLabel *Label11, *Label12, *Label13, *Label14, *Label15, *Label16, *Label17, *Label18, *Label19, *Label20,
           *Label21, *Label22, *Label23;
    TPanel *Panel1;
    TButton *btnManualStart;
    TCheckBox *chkUpperCase;
    TLabel *lblTestTime;
    TComboBox *cbbGPIBTimo;
    TCheckBox *chkStrLengthCheck;
    TCheckBox *cbBinonEcho;
    TPanel *palSite;
    TGroupBox *gbSite;
    TComboBox *cbBin;
    TCheckBox *cbSiteOn;
    TPanel *plSite;
    TCheckBox *cbFullSiteTimeOut;
    TComboBox *cbbFullsiteTimeOut;
    TLabel *Label1;
    TLabel *labDebugMode;
    TLabel *labOcr;
    TPanel *Panel2;
    TMemo *mmoBINON;
    TPanel *palSCKART;
    TLabel *labLotID_1;
    TLabel *labLotID;
    TLabel *labLotCount;
    TGroupBox *gbAlarmCode;
    TMemo *memoAlarmCode;
    TLabel *labLotCount_1;
    TLabel *labSts_1;
    TLabel *labStatus;
    TLabel *labTestCount_1;
    TLabel *labTestCount;
    TSpeedButton *spbAutoRetest;
    TListBox *lstRecord;
    TComm *CommAMD;
    TComm *CommAMD2;
    TPanel *pnlAMDCmd;
    TLabel *Label24;
    TRadioGroup *rgController;
    TComboBox *cbMode;
    TButton *btnSendTemp;
    TCheckBox *chkTempReady;
    TCheckBox *chkCMDLog;
    TTimer *TimerTMode;
    TButton *btnRunMode;
    TComboBox *cbbRunMode;
    TLabel *labPackageType_1;
    TLabel *labPackageType;
    TCheckBox *cbGPIBWriteWithout_r_n;
    TTabSheet *tsRS232;
    TPageControl *PageControl2;
    TPageControl *pgcRS232;
    TTabSheet *ts_STD_Log;
    TMemo *MemoLog;
    TTabSheet *ts_STD_Setup;
    TGroupBox *GroupBox1;
    TLabel *Label2, *Label3, *Label4, *Label5, *Label6;
    TComboBox *cbBaudRate;
    TComboBox *cbByteSize;
    TComboBox *cbStopBit;
    TComboBox *cbParity;
    TComboBox *cbDevice;
    TPanel *btnUpdate;
    TGroupBox *GroupBox14;
    TLabel *Label7;
    TEdit *edReadIntervalTimeout;
    TEdit *edHANACmd;
    TPanel *pnlHanaART;
    TGroupBox *grpHanaARTToHandler;
    TComboBox *cbCmdHANAART;
    TEdit *edCmdHANAART;
    TButton *btnSend_ED;
    TButton *btnHANA_SendCB;
    TGroupBox *grpHanaARTToTester;
    TComboBox *cbCmdHANAARTtoTester;
    TEdit *edCmdHANAARTtoTester;
    TButton *btnSend_EDToTester;
    TButton *btnHANA_SendCBTotester;
    TButton *btnSaveLog;
    TButton *Button1;
    TButton *btnDiagZip;

    // TForm leftovers golden code touches
    AnsiString Caption;

    // ---- golden event handlers (bodies: GpibUi.cpp / GpibAmdAux.cpp / GpibHanaArt.cpp) ----
    void Timer1Timer(TObject *Sender);
    void btnManualStartClick(TObject *Sender);
    void FormShow(TObject *Sender);
    void FormClose(TObject *Sender);            // golden (TObject*, TCloseAction&)
    void FormCreate(TObject *Sender);
    void chkUpperCaseClick(TObject *Sender);
    void cbbGPIBTimoChange(TObject *Sender);
    void chkStrLengthCheckClick(TObject *Sender);
    void cbBinonEchoClick(TObject *Sender);
    void cbFullSiteTimeOutClick(TObject *Sender);
    void spbAutoRetestClick(TObject *Sender);
    void TimerTModeTimer(TObject *Sender);
    void btnSendTempClick(TObject *Sender);
    void CommAMDReceiveData(TObject *Sender, void* Buffer, WORD BufferLength);
    void btnRunModeClick(TObject *Sender);
    void cbGPIBWriteWithout_r_nClick(TObject *Sender);
    void btnUpdateClick(TObject *Sender);
    void btnHANA_SendCBClick(TObject *Sender);
    void btnSend_EDClick(TObject *Sender);
    void btnHANA_SendCBTotesterClick(TObject *Sender);
    void btnSend_EDToTesterClick(TObject *Sender);
    void btnSaveLogClick(TObject *Sender);
    void btnDiagZipClick(TObject *Sender);
    void btnUpdateMouseDown(TObject *Sender);   // golden (Sender, Button, Shift, X, Y): only recolours btnUpdate
    void btnUpdateMouseUp(TObject *Sender);

    // ---- golden private ----
    DWORD dwStart, dwEnd;
    void GPIBSendToBack();
    std::vector<TMyDutPanel*> MY_DUT_PAL;
    bool CheckGSBINONString(AnsiString str);

    // ---- golden public ----
    TSerialPoll();
    ~TSerialPoll();
    int TestGPIB();
    int TestGPIBForCastle();
    void WriteLog(AnsiString str1);
    void OnMyCopyMsg(TMessage &msg);

    bool bFind;
    void CallControl();
    void ProcessHMountConnect();
    void SendCaptureFinish();
    void ProcessMessage();
    void ProcessAddress();
    void SaveResult();
    void UpdateLed();

    bool ReadLastDataFile();
    bool WriteLastDataFile();
    bool MyGPIBWrite(AnsiString str, AnsiString Task = "");
    void SetParameter(AnsiString str);
    void InitialStartValue();

    int iBinSelect;
    int oldiBinSelect;
    void Save_Log();

    void SendMSG_CMD_INPUTQTY(int CMD, AnsiString LotID, int Count, AnsiString ProcessCode = " ");
    void SendMSG_CMD(int CMD);
    void SendMSG_CMD_ESC(int CMD, int iEcho);
    void SendMSG_CMD(int CMD, AnsiString Message);

    bool ProcessStatusString(AnsiString Str, AnsiString BS, AnsiString Task);
    bool ProcessStatusStringRCMD(AnsiString Str, AnsiString BS, AnsiString Task);
    int  ProcessStatusStringRFMD(AnsiString Str, AnsiString BS, AnsiString Task);
    bool ProcessStatusString_Delta_Castle(AnsiString Str, AnsiString BS, AnsiString Task);
    void InitialBarcodeList();
    void DoOverDrive(AnsiString Msg, AnsiString Task);
    void DoRecontact(AnsiString Msg, AnsiString Task);
    bool bRETURN_GPIB_VERSION;

    int iCurrentArm;
    void ClearAllFlag();

    bool bDummyArt;
    bool bMessageFromHandler;
    bool bStsMessageFromHandler;
    AnsiString DummyArtMSG;
    AnsiString DummyArtStsMSG;
    AnsiString asShowMemo;
    void SendMode(int iArm, int iMode);
    void QueryRDY(int iArm);

    TStringList *slCmdList;
    void ShowSimulateItem(bool bShow);

    int GPIB_TYPE;
    int GPIB_ERROR_BIN;
    AnsiString sMassTempData[8];
    AnsiString ATC_AMD_COM;
    int GetResultForCastle(AnsiString S);
    void InitHANA_ART();
    typedef struct
    {
        enum SRQCode
        {
            SETUP_INFORM_REQUEST_SRQ0x55        = 0x55,
            SETUP_INFORM_RECEIVE_OK_SRQ0x56     = 0x56,
            SETUP_INFORM_RECEIVE_FAIL_SRQ0x57   = 0x57,
            LOT_START_SRQ0x63                   = 0x63,
            LOTON_READ_SUCCESS_SRQ0x51          = 0x51,
            LOTON_READ_FAIL_SRQ0x52             = 0x52,
            LOT_END_SRQ0x64                     = 0x64,
            STANDBY_TESTMODE_SRQ0x53            = 0x53,
            PRIME_START_SRQ0x50                 = 0x50,
            PRIME_END_SRQ0x54                   = 0x54,
            NORMAL_START_SRQ0x41                = 0x41,
            RETEST_START_SRQ0x65                = 0x65,
            RETEST_END_SRQ0x66                  = 0x66,
            DUMMYTEST_START_SRQ0x42             = 0x42,
            ERROR_START_SRQ0x67                 = 0x67,
            ERROR_END_SRQ0x68                   = 0x68,
            ERROR_CLEAR_SRQ0x69                 = 0x69,
            AUTO_START_SRQ0x40                  = 0x40,
            INIT_START_SRQ0x62                  = 0x62,
            LOADER_REQ_SRQ0x61                  = 0x61,
            LOT_INFORM_SRQ0x58                  = 0x58,
            FQATEST_START_SRQ0x71               = 0x71,
            FQATEST_END_SRQ0x72                 = 0x72,
            WPSOCKET_REQ_SRQ0x73                = 0x73,
            FBIN_MANUAL_REQ_SRQ0x74             = 0x74,
            REAL_PARA_SEQ_SRQ0x75               = 0x75,
            LOT_INFO_METHOD_SRQ0x76             = 0x76,
            RF_ID_SRQ0x77                       = 0x77,
            SBL_SRQ0x78                         = 0x78,
            AUTODUMPING_SRQ0x79                 = 0x79,
            EQP_MODEL_SRQ0x43                   = 0x43,
            HD_MODE_SRQ0x59                     = 0x59,
            INLINEUPDATE_SRQ0x60                = 0x60,
            CLEAR_START_SRQ0x00                 = 0x0
        };
    } HANA_ART_SMILL;
    void DoSendCommand(int iCmd);
    void DoSendCommand(AnsiString sCmd);
    std::map<AnsiString, HANA_ART_SMILL::SRQCode> srqCodeMap;
    void InitializeSRQCodeMap();

    // golden Main.cpp:4822 (dead code, no caller) kept for completeness
    bool SVON_CLOSE();

    // ---- V906 plumbing (not golden) ----
    webbridge::WbMutex uiMutex;                  // guards every widget above for the web snapshot (P7)
    void PostToHandler(COPYDATASTRUCT* pcp);     // golden SendMessage(HMountWnd, WM_COPYDATA, 0, pcp)
    void RequestClose(const char* reason);       // golden Close()
    void UiNotice(AnsiString text);              // golden MessageDlg/ShowMessage (non-blocking)
    bool closeRequested;
    AnsiString closeReason;
    AnsiString lastNotice;
    testercomm::SyncMailbox* mailbox;            // set by GpibEngine before FormCreate

    // Serial receive marshalling (rule 11): vclcompat TComm fires OnReceiveData on ITS reader thread, but golden
    // runs CommAMDReceiveData / TfRS232Main::CommTesterReceiveData on the form's (= TesterComm) thread.  The
    // OnReceiveData callbacks therefore only QueueRx(); GpibEngine::RunOnce calls DrainRx() on the TesterComm
    // thread, which dispatches to the golden handler.
    enum RxPort { kRxCommAMD = 0, kRxCommAMD2 = 1, kRxAuxTester = 2 };
    void QueueRx(int port, const void* buf, Word len);   // any thread
    void DrainRx();                                        // TesterComm thread only
    webbridge::WbMutex rxMutex;
    std::vector<std::pair<int, std::string> > rxQueue;
};

extern TSerialPoll *SerialPoll;   // golden global (the one bridge instance)

// golden RS232.h TfRS232Main (the AMD/ATC aux line inside the GPIB program; NOT RS232Standard) -- GpibAmdAux.cpp
class TfRS232Main
{
public:
    TComm *CommTester;
    TMemo *MemoLog;
    TComboBox *cbBaudRate, *cbByteSize, *cbStopBit, *cbParity, *cbDevice;
    TEdit *edReadIntervalTimeout;
    AnsiString sFolder;
    std::vector<Byte> ReceiveData;
    std::vector<Byte> SendData;
    std::vector<Byte> SendStartData;
    bool bCommConnect;
    AnsiString sLastFile;
    int iHasCE;

    TfRS232Main();
    ~TfRS232Main();
    void CommTesterReceiveData(TObject *Sender, void* Buffer, WORD BufferLength);
    bool LoadSetupData();
    bool SaveSetupData();
    void InitDataToMainForm();
    void SetFormToData();
    bool OpenTesterComm();
    void ShowCommData(AnsiString sType, std::vector<Byte> Data, bool bAddToGPIBLog = false);
    void SendCommandToTester(std::vector<Byte>& Data, bool bAddToGPIBLog = false);
    bool CloseTesterComm();
    AnsiString GetAnalysisString(std::vector<Byte> &ReceiveData);
    void Del1stVec(std::vector<Byte> &ReceiveData);
    void DoRevCommand(AnsiString sCommand);
    void SaveResult();
    void Save_Log();
};
extern TfRS232Main *fRS232Main;
bool GpibAuxFramingFromRecipe();   // AI(W906-GB-P6) 20260926: GpibAux.cpp -- the port took the recipe framing (2A + Q2(a))

// golden DummyArt.h TfDummyART -- GpibDummyArt.cpp
class TfDummyART
{
public:
    TLabel *Label2, *Label3;
    TSpeedButton *spbAutoRetest, *spbStopART, *spbPauseART, *spbContinue;
    TEdit *edLotID, *edDummyCount;
    TRadioGroup *rgTestType;
    TTimer *DummyARTTimer1;
    TCheckBox *cbStepByStep;
    TfDummyART();
    ~TfDummyART();
    void spbAutoRetestClick(TObject *Sender);
    void spbStopARTClick(TObject *Sender);
    void DummyARTTimer1Timer(TObject *Sender);
    void spbPauseARTClick(TObject *Sender);
    void spbContinueClick(TObject *Sender);
    void FormShow(TObject *Sender);
    void HP93KART();
    void FLEXART();
};
extern TfDummyART *fDummyART;

}  // namespace gpibbridge

#endif
