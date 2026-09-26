// ===========================================================================
//  TesterComm/Rs232/Rs232Bridge.h -- RS232Standard (tester RS232 + TTL board = DIO) translated into the V906 process.
//
//  AI(W906-GB-P4) 20260926: Tester-comm plan P4.  Golden: D:\RS232Standard\RS232_Code32Bin_Rev12.13.902.0_20260410
//  (MainForm.h/.cpp TfRS232Main, cmydef.h/.cpp, MessageDef.cpp instances, MyDutPanel, MyStringList, uSocketServerClient).
//  Same design as TesterComm/Gpib/GpibBridge.h (read its banner): its own namespace (rs232std) because the
//  program's globals (InitialOK, iResult, HMountWnd, bSimulate, GGpib2Handler, sBarCode, ...) share names with the
//  Handler's AND with gpibbridge's; golden bodies stay textually unchanged inside the namespace.
//
//  NOT the same thing as gpibbridge::TfRS232Main: that one is the AMD/ATC aux line inside the GPIB program
//  (golden GPIB RS232.cpp).  This one is the tester-side RS232Standard program (user ruling 20260926 #8).
//
//  GOLDEN BUILD SWITCHES (MainForm.cpp:16-17).  The 902 snapshot has `#define DEBUG` uncommented; every earlier
//  release (Rev12.12.854 / .873 / .874, Code32Bin) has `//#define DEBUG`.  With DEBUG defined golden never stores
//  the tester's BA bins (MainForm.cpp:3467-3476 is #ifndef DEBUG) and ignores MSG_CMD_CloseGpib (:780), i.e. the
//  902 folder is a debugging snapshot (user ruling 20260926: ship with DEBUG OFF -- keep the default).
//  RS232STD_GOLDEN_DEBUG (default 0) reproduces the release build; set it to 1
//  to get the snapshot exactly.  SOFT_SIMULTE is ON in every release and stays ON (RS232STD_GOLDEN_SOFT_SIMULTE).
//  The switches are applied at the END of this header (see there); the translated MainForm.cpp pieces also repeat,
//  right after including this header, the block below (harmless duplicates):
//      #if RS232STD_GOLDEN_DEBUG
//      #define DEBUG
//      #endif
//      #if RS232STD_GOLDEN_SOFT_SIMULTE
//      #define SOFT_SIMULTE
//      #endif
//  (golden defines them after its includes too, so no included header sees them).
//
//  Translation rules: TesterComm/Rs232/TRANSLATION_RULES.md (the GPIB rules plus the RS232 specifics).
// ===========================================================================
#ifndef TESTERCOMM_RS232_RS232BRIDGE_H
#define TESTERCOMM_RS232_RS232BRIDGE_H

#include "vclcompat/vcl_compat.h"
#include "vclcompat/Controls.h"
#include "vclcompat/Comm.h"
#include "vclcompat/ClientSocket.h"
#include "vclcompat/ServerSocket.h"
#include "MessageDef.h"            // VM / MV packet types and MSG_CMD_* values (shared contract; instances below)
#include "WebBridge/Sync.h"

#include <functional>
#include <string>
#include <utility>
#include <vector>

#ifndef RS232STD_GOLDEN_DEBUG
#define RS232STD_GOLDEN_DEBUG 0
#endif
#ifndef RS232STD_GOLDEN_SOFT_SIMULTE
#define RS232STD_GOLDEN_SOFT_SIMULTE 1
#endif

namespace testercomm { class SyncMailbox; }

// golden MainForm.h:19-22 (GpibAux.cpp guards them the same way)
#ifndef _STX_
#define _STX_     0x02
#endif
#ifndef _ETX_
#define _ETX_     0x03
#endif
#ifndef _ENQ_
#define _ENQ_     0x05
#endif
#ifndef _ACK_
#define _ACK_     0x06
#endif

namespace rs232std {

typedef unsigned char Byte;   // BCB System.hpp

// ---------------------------------------------------------------------------
//  MainForm.cpp:18-25 file-scope macros -> constants (only MainForm.cpp used them)
// ---------------------------------------------------------------------------
const int MAX_SITE_COUNT = 32;
const int USE_SITE_COUNT = 32;
const int InterfaceType_TTL      = 1000;
const int InterfaceType_Standard = 0;
const int InterfaceType_SLT      = 1;

// ---------------------------------------------------------------------------
//  cmydef.h (RS232Standard copy).  The CC_* customer codes are identical to the Handler's MachineType.h (checked
//  20260926: 0 differences; only CC_SPIL_SC, unused, is missing there), which MessageDef.h already includes.
// ---------------------------------------------------------------------------
extern bool InitialOK;
extern Word SystemHour, SystemMin, SystemSec, SystemMSec;
extern Word SystemYear, SystemMonth, SystemDate;
extern Word SystemYearYesterday, SystemMonthYesterday, SystemDateYesterday;
extern TIniFile *INIFile;   // golden cmydef.cpp:18

TDateTime  CheckAndReadIniData(AnsiString FileName, AnsiString Group, AnsiString Name, TDateTime Value);
AnsiString CheckAndReadIniData(AnsiString FileName, AnsiString Group, AnsiString Name, AnsiString Value);
int        CheckAndReadIniData(AnsiString FileName, AnsiString Group, AnsiString Name, int Value);
bool       CheckAndReadIniData(AnsiString FileName, AnsiString Group, AnsiString Name, bool Value);
double     CheckAndReadIniData(AnsiString FileName, AnsiString Group, AnsiString Name, double Value);
void WriteIniData(AnsiString FileName, AnsiString Group, AnsiString Name, bool bValue);
void WriteIniData(AnsiString FileName, AnsiString Group, AnsiString Name, int Value);
void WriteIniData(AnsiString FileName, AnsiString Group, AnsiString Name, double Value);
void WriteIniData(AnsiString FileName, AnsiString Group, AnsiString Name, unsigned Value);
void WriteIniData(AnsiString FileName, AnsiString Group, AnsiString Name, unsigned long Value);
void WriteIniData(AnsiString FileName, AnsiString Group, AnsiString Name, AnsiString Value);
void WriteIniData(AnsiString FileName, AnsiString Group, AnsiString Name, TDateTime Value);
void GetTimeInfo();
int MyForceDirectories(AnsiString Directory, AnsiString Function = "");
void MySleep(DWORD dwMilliseconds);
unsigned int crc_chk(unsigned char* data, unsigned char length, char &CRC1, char &CRC2);
AnsiString MyDeCodeASCII(int iInPut);
AnsiString GetErrorMessage(DWORD dwErrorMessageCode);

// golden cmydef VerInfo: version of the running exe.  In-process there is no RS232Standard.exe; the translation
// returns the fixed program version (RS232Standard.bpr:74-77 = 12.13.902.0) in golden's format (Rs232Globals.cpp).
class VerInfo
{
public:
    VerInfo() {}
    void GetAppVersion(AnsiString sAppExeName, WORD& major, WORD& minor, WORD& build, WORD& revision);
    AnsiString GetSVNRev();
    AnsiString GetFileVersion();
};

// ---------------------------------------------------------------------------
//  MessageDef instances (RS232 side copies; the Handler keeps its own in ::)
// ---------------------------------------------------------------------------
extern AnsiString GPIBVersion;      // golden RS232 MessageDef.cpp:13
extern AnsiString RS232Version;     // :14 (FormShow overwrites it with VerInfo().GetFileVersion())
extern double GPIBVersionCheck;     // :15
extern VM GGpib2Handler;            // :234 bridge -> Handler packet
extern MV *GHandler2Gpib;           // :235 points at the Handler -> bridge packet being processed

// ---------------------------------------------------------------------------
//  MainForm.cpp file-scope globals (golden :26-42, :296-355)
// ---------------------------------------------------------------------------
extern TStringList *sBarCode;
extern TStringList *sBarCode_ASE_CL;
extern bool bHasBarCode;
extern bool bShowVersionOK;
extern bool bTTL1RS232Send;
extern bool bTTL2RS232Send;
extern bool bTTL1RS232Rev;
extern bool bTTL2RS232Rev;
extern AnsiString asHGeneralPath;   // "d:\\HT9045\\system\\Gerneral.ini"
extern AnsiString IniFileName;      // "D:\\RS232Standard\\System\\Setup.ini"
extern const int iRS232Error;
extern const int iNoneTest;
extern const int iReadyToTest;
extern const int iWaitReplyCE;
extern const int iReplyCE;
extern HWND HMountWnd;
extern bool bSimulate;
extern bool bSupport32Bin;
extern bool bManualTest;
extern int iStart [MAX_SITE_COUNT];
extern int iResult[MAX_SITE_COUNT];
extern bool bGpibMode;
extern AnsiString sMachineStateDecade;
extern int iMacStateStrLength;
extern char cMachineStateDec[6];
extern AnsiString sTestBinCount;
extern char cTestBinCount[1024];
extern AnsiString sSoakTime;
extern char cSoakTime[5];
extern AnsiString sJamCode;
extern char cJamCode[5];
extern AnsiString sSiteMap;
extern char cSiteMap[256];
extern AnsiString sAllMassTemp;
extern char cAllMassTemp[100];
extern AnsiString sHandlerID;
extern AnsiString sSiteOnOff;
extern char cSiteOnOff[256];
extern AnsiString sNumOfSites;
extern char cNumOfSites[10];
extern char cHandlerID[100];
extern int iUseRS232Mode;
extern int TTL_CARD_TYPE;
extern bool bStartSOT[2];
extern int iTTLBoardNum;
extern bool bTTLBoardAddr;
extern AnsiString sDoubleContactCount;
extern char cDoubleContactCount[5];
extern double dSLTMaxTime;
extern bool bUseRS232SLTFlow;
extern bool bRunCheckProgramUse;
extern AnsiString RunCheckProgramName;
extern AnsiString RunCheckProgramErrCode;
extern double dPowerSwitchDelay;
extern double dSendNEXTDelay;
extern double dMaxBIOSWaitTime;
extern AnsiString MaxBIOSWaitTimeErrCode;
extern bool bMaxBIOSWaitTimeAlm;
extern double dMinTestTime;
extern AnsiString MinTestTimeErrCode;
extern bool bMinTestTimeAlm;
extern AnsiString MaxTestTimeErrCode;
extern bool bMaxTestTimeAlm;
extern double dTestOKWaitTime;
extern AnsiString sSLTState;
extern bool bCanRevSortBin;
extern bool bErrorOccur;
extern AnsiString sTTLVersion;
extern AnsiString sTTLVersion2;
extern int iRevCycleClear;

void WriteDataToFile(char* cFilePath, char* cData, int iSize);   // golden MainForm.h:446 / .cpp:2564

// ---- V906 program life (not golden; same scheme as gpibbridge, see GpibBridge.h "V906 bridge life") ----
// Golden: the Handler relaunches RS232Standard.exe (RunTestProgram) and every launch starts from the initialisers.
// ResetRs232Globals() (Rs232Globals.cpp) puts every global above back to its golden initialiser (not fRS232Main /
// HMountWnd, which the engine owns); Rs232Engine::Start calls it before constructing TfRS232Main and increments
// g_rs232Life, which the functions with function-local statics watch to re-arm them.
extern unsigned long g_rs232Life;
void ResetRs232Globals();

// ---------------------------------------------------------------------------
//  Headless widget stand-ins missing from vclcompat (same shape as gpibbridge's; separate copies so the two
//  programs stay independent)
// ---------------------------------------------------------------------------
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

// golden RS232Standard MyDutPanel.h TMyDutPanel -- one per site (web: HTWidgets.makeDutPanel, shared with GPIB).
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
    explicit TMyDutPanel(int index);   // golden ctor minus Owner/Parent/geometry (Rs232Ui.cpp)
    ~TMyDutPanel();
};

// golden TMessage subset used by OnMyCopyMsg.
struct TMessage
{
    LPARAM LParam;
    WPARAM WParam;
};

// ---------------------------------------------------------------------------
//  golden MyStringList.h TMyStringList (RS232Standard's own variant -- NOT the Handler's Public/MyStringList.h).
//  __property Path/FileName/FirstRow/MaxLineCount/SaveType/AutoSave: every golden setter is a plain assignment
//  (MyStringList.cpp:54-82), so each property is a public field and the golden HT* name is a reference bound to
//  it (Rs232Support.cpp ctors); both spellings in golden bodies compile unchanged.
// ---------------------------------------------------------------------------
enum TSaveType   // golden MyMemo.h (D:\HT9045\elec\myvcl; copy at vclcompat/component_src/MyMemo.h:11-24)
{
    TByMaxLineCount = 0,
    TByHour         = 1,
    TBy2Hour        = 2,
    TBy4Hour        = 3,
    TBy6Hour        = 4,
    TBy8Hour        = 5,
    TBy12Hour       = 6,
    TByDay          = 7,
    TByMonth        = 8,
    TByYear         = 9,
    TByMin          = 10
};

class TMyStringList : public TStringList
{
public:
    // properties as public fields -- declared before the references below, which bind to them
    AnsiString  Path;
    AnsiString  FileName;
    AnsiString  FirstRow;
    int         MaxLineCount;
    TSaveType   SaveType;
    bool        AutoSave;
private:
    AnsiString& HTPath;
    AnsiString& HTFileName;
    AnsiString& HTFirstRow;
    int&        HTMaxLineCount;
    TSaveType&  HTSaveType;
    bool&       HTAutoSave;
    void SetPath(AnsiString P);
    void SetFirstRow(AnsiString P);
    void SetMaxLineCount(int Cnt);
    void SetSaveType(TSaveType Type);
    void SetFileName(AnsiString P);
    void SetAutoSave(bool P);
    void GetYesterdayInfo();
    TMyStringList(const TMyStringList&);
    TMyStringList& operator=(const TMyStringList&);
public:
    TMyStringList();
    TMyStringList(AnsiString sPath, AnsiString sFileName, AnsiString sFirstRow);
    ~TMyStringList();
    AnsiString AddText(AnsiString sUnitName, AnsiString sMsg1, AnsiString sMsg2 = "");
    AnsiString AddTextWithHex(AnsiString sUnitName, std::vector<Byte> bHex);
    void MySaveToFile();
    void MyInsertToFile(int iCount);
    int GetLastLine();
    AnsiString sLastFileName;
    AnsiString GetFileName();
    bool bFilePathWithDate;
    bool bUseFTRT;
    TStringList *MyList;
};

// ---------------------------------------------------------------------------
//  golden uSocketServerClient.h (the SOFT_SIMULTE tester-simulator socket) on vclcompat TServerSocket /
//  TClientSocket.  Those fire their events on their own threads, so the receive callback given to
//  SetReceiveFunc() must only queue (TfRS232Main::QueueRx(kRxTcp, ...)); see TRANSLATION_RULES.md.
// ---------------------------------------------------------------------------
const int ReceiveLen = 1023;   // golden #define ReceiveLen 1023

typedef struct tagIPINFO
{
    BYTE bTimeToLive;
    BYTE bTypeOfService;
    BYTE bIpFlags;
    BYTE OptSize;
    BYTE *Options;
} IPINFO, *PIPINFO;

typedef struct tagICMPECHO
{
    DWORD dwSource;
    DWORD dwStatus;
    DWORD dwRTTime;
    WORD wDataSize;
    WORD wReserved;
    void *pData;
    IPINFO ipInfo;
} ICMPECHO, *PICMPECHO;
typedef HANDLE (WINAPI *PF_CMPCREATEFILE)   (VOID);
typedef BOOL   (WINAPI *PF_ICMPCLOSEHANDLE) (HANDLE);
typedef DWORD  (WINAPI *PF_ICMPSENDECHO)    (HANDLE, DWORD, LPVOID, WORD, PIPINFO, LPVOID, DWORD, DWORD);

typedef std::function<void(char*, int)> TPointVoidReceive;   // golden `void (__closure *)(char*, int)`

class uSocketBase
{
private:
    AnsiString asAddress;
    AnsiString asPort;
public:
    TPointVoidReceive tpvReceive;
    char strReceiveUse[ReceiveLen];
    int iBufferLenght;
    AnsiString GetSettingSection()              {return "SocketSetting";}
    void SetSocketAddress(AnsiString asValue)   {asAddress=asValue;}
    AnsiString GetSocketAddress()               {return asAddress;}
    void SetSocketPort(AnsiString asValue)      {asPort=asValue;}
    AnsiString GetSocketPort()                  {return asPort;}
    uSocketBase();
    ~uSocketBase();
    void InitialData();
    void ReloadData(AnsiString asSettingFileNameWithPath);
    void ReadSettingFile(AnsiString asSettingFileNameWithPath);
    void WriteSettingFile(AnsiString asSettingFileNameWithPath);
    void SetReceiveFunc(TPointVoidReceive _func);
    bool Ping(AnsiString asIP);
};

class uSocketServer : public uSocketBase
{
private:
    int DoCommuncationTask;
    TServerSocket *ServerSocket;
    void InitializationSocketServer();
    void ServerSocketConnect(TObject *Sender, TCustomWinSocket *Socket);
    void ServerSocketDisconnect(TObject *Sender, TCustomWinSocket *Socket);
    void ServerSocketError(TObject *Sender, TCustomWinSocket *Socket, TErrorEvent ErrorEvent, int &ErrorCode);
    void ServerSocketRead(TObject *Sender, TCustomWinSocket *Socket);
    bool MatchServerPort();
    bool IsConnected();                        // golden inline: ServerSocket->Socket->Connected (Rs232Support.cpp)
    bool Open();
public:
    uSocketServer();
    ~uSocketServer();
    void Initialization();
    int GetActiveConnections();
    AnsiString GetConnectClientAddress();
    bool DoOpenCommuncation();
    bool SendCommand(char* cSet, int iLen);
};

class uSocketClient : public uSocketBase
{
private:
    TClientSocket* ClientSocket;
    void ClientSocketRead(TObject *Sender, TCustomWinSocket *Socket);
    void ClientSocketError(TObject *Sender, TCustomWinSocket *Socket, TErrorEvent ErrorEvent, int &ErrorCode);
    void ClientSocketConnect(TObject *Sender, TCustomWinSocket *Socket);
    void ClientSocketDisconnect(TObject *Sender, TCustomWinSocket *Socket);
    bool MatchClientSetting();
    bool bConnected;
public:
    uSocketClient();
    ~uSocketClient();
    void Initialization();
    void InitializationSocketClient();
    bool Open();
    bool IsConnected();                        // golden inline: bConnected || ClientSocket->Active==true
    bool DoOpenCommuncation();
    void Close();
    bool SendCommand(char* cSet, int iLen);
    bool SetCommParameter(AnsiString asAddress, AnsiString asPort);
};

// ---------------------------------------------------------------------------
//  TfRS232Main (golden MainForm.h), minus TForm
// ---------------------------------------------------------------------------
class TfRS232Main
{
public:
    // ---- golden __published widgets (MainForm.h:28-339, generated in order) ----
    // IDE-managed Components
    TComm *CommTester;
    TTimer *Timer1;
    TPageControl *PageControl1;
    TTabSheet *tsVersion;
    TMemo *MemoVer;
    TStatusBar *StatusBar1;
    TPanel *palSite;
    TGroupBox *gbSite;
    TComboBox *cbBin;
    TCheckBox *cbSiteOn;
    TPanel *plSite;
    TTabSheet *tsSPRD;
    TPageControl *PageControl5;
    TTabSheet *TabSheet3;
    TPageControl *Pc_SPRDSite1Control;
    TTabSheet *ts_SPRDLogSite1;
    TMemo *mm_SPRDLogSite1;
    TCheckBox *cb_SPRDShowLogSite1;
    TButton *bt_SPRDClearLogSite1;
    TCheckBox *cb_SPRDSaveLogSite1;
    TTabSheet *ts_SPRDSetupSite1;
    TButton *Button2;
    TGroupBox *GroupBox6;
    TLabel *Label44;
    TLabel *Label45;
    TLabel *Label46;
    TLabel *Label47;
    TLabel *Label48;
    TComboBox *cb_SPRDBaudSite1;
    TComboBox *cb_SPRDSizeSite1;
    TComboBox *cb_SPRDStopSite1;
    TComboBox *cb_SPRDParitySite1;
    TComboBox *cb_SPRDComSite1;
    TGroupBox *GroupBox7;
    TLabel *Label49;
    TLabel *Label50;
    TLabel *Label51;
    TLabel *Label52;
    TComboBox *cb_SPRDDTRSite1;
    TComboBox *cb_SPRDRTSSite1;
    TCheckBox *cb_SPRDTxCSite1;
    TEdit *ed_SPRDTimeOutTotalSite1;
    TEdit *ed_SPRDTimeOutSite1;
    TTabSheet *TabSheet10;
    TPageControl *Pc_SPRDSite2Control;
    TTabSheet *ts_SPRDLogSite2;
    TMemo *mm_SPRDLogSite2;
    TCheckBox *cb_SPRDShowLogSite2;
    TButton *bt_SPRDClearLogSite2;
    TCheckBox *cb_SPRDSaveLogSite2;
    TTabSheet *ts_SPRDSetupSite2;
    TButton *Button4;
    TGroupBox *GroupBox8;
    TLabel *Label53;
    TLabel *Label54;
    TLabel *Label55;
    TLabel *Label56;
    TLabel *Label57;
    TComboBox *cb_SPRDBaudSite2;
    TComboBox *cb_SPRDSizeSite2;
    TComboBox *cb_SPRDStopSite2;
    TComboBox *cb_SPRDParitySite2;
    TComboBox *cb_SPRDComSite2;
    TGroupBox *GroupBox9;
    TLabel *Label58;
    TLabel *Label59;
    TLabel *Label60;
    TLabel *Label61;
    TComboBox *cb_SPRDDTRSite2;
    TComboBox *cb_SPRDRTSSite2;
    TCheckBox *cb_SPRDTxCSite2;
    TEdit *ed_SPRDTimeOutTotalSite2;
    TEdit *ed_SPRDTimeOutSite2;
    TTabSheet *TabSheet8;
    TPageControl *Pc_SPRDSite3Control;
    TTabSheet *ts_SPRDLogSite3;
    TMemo *mm_SPRDLogSite3;
    TCheckBox *cb_SPRDShowLogSite3;
    TButton *bt_SPRDClearLogSite3;
    TCheckBox *cb_SPRDSaveLogSite3;
    TTabSheet *ts_SPRDSetupSite3;
    TButton *Button3;
    TGroupBox *GroupBox10;
    TLabel *Label80;
    TLabel *Label81;
    TLabel *Label82;
    TLabel *Label83;
    TLabel *Label84;
    TComboBox *cb_SPRDBaudSite3;
    TComboBox *cb_SPRDSizeSite3;
    TComboBox *cb_SPRDStopSite3;
    TComboBox *cb_SPRDParitySite3;
    TComboBox *cb_SPRDComSite3;
    TGroupBox *GroupBox11;
    TLabel *Label85;
    TLabel *Label86;
    TLabel *Label87;
    TLabel *Label88;
    TComboBox *cb_SPRDDTRSite3;
    TComboBox *cb_SPRDRTSSite3;
    TCheckBox *cb_SPRDTxCSite3;
    TEdit *ed_SPRDTimeOutTotalSite3;
    TEdit *ed_SPRDTimeOutSite3;
    TTabSheet *TabSheet11;
    TPageControl *Pc_SPRDSite4Control;
    TTabSheet *ts_SPRDLogSite4;
    TMemo *mm_SPRDLogSite4;
    TCheckBox *cb_SPRDShowLogSite4;
    TButton *bt_SPRDClearLogSite4;
    TCheckBox *cb_SPRDSaveLogSite4;
    TTabSheet *ts_SPRDSetupSite4;
    TButton *Button6;
    TGroupBox *GroupBox12;
    TLabel *Label89;
    TLabel *Label90;
    TLabel *Label91;
    TLabel *Label92;
    TLabel *Label93;
    TComboBox *cb_SPRDBaudSite4;
    TComboBox *cb_SPRDSizeSite4;
    TComboBox *cb_SPRDStopSite4;
    TComboBox *cb_SPRDParitySite4;
    TComboBox *cb_SPRDComSite4;
    TGroupBox *GroupBox13;
    TLabel *Label94;
    TLabel *Label95;
    TLabel *Label96;
    TLabel *Label97;
    TComboBox *cb_SPRDDTRSite4;
    TComboBox *cb_SPRDRTSSite4;
    TCheckBox *cb_SPRDTxCSite4;
    TEdit *ed_SPRDTimeOutTotalSite4;
    TEdit *ed_SPRDTimeOutSite4;
    TTabSheet *ts_BinCode;
    TLabel *Label76;
    TLabel *Label77;
    TLabel *Label78;
    TLabel *Label79;
    TPanel *Panel15;
    TLabel *Label62;
    TLabel *Label63;
    TLabel *Label64;
    TLabel *Label65;
    TLabel *Label66;
    TPanel *pn_WSBin11;
    TPanel *pn_WSBin12;
    TPanel *pn_WSBin13;
    TPanel *pn_WSBin14;
    TPanel *pn_WSBin15;
    TPanel *Panel9;
    TLabel *Label67;
    TLabel *Label68;
    TLabel *Label69;
    TLabel *Label70;
    TLabel *Label71;
    TPanel *pn_WSBin6;
    TPanel *pn_WSBin7;
    TPanel *pn_WSBin8;
    TPanel *pn_WSBin9;
    TPanel *pn_WSBin10;
    TPanel *Panel3;
    TLabel *lbWS3000Bin1;
    TLabel *Label72;
    TLabel *Label73;
    TLabel *Label74;
    TLabel *Label75;
    TPanel *pn_WSBin1;
    TPanel *pn_WSBin2;
    TPanel *pn_WSBin3;
    TPanel *pn_WSBin4;
    TPanel *pn_WSBin5;
    TCheckBox *cb_SPRDSendSOTString;
    TPanel *Pan_TimeOutBin;
    TEdit *ed_TimeOutTime;
    TButton *bt_ChangeTimeOut;
    TPanel *Pan_NotDefinedtBin;
    TButton *Button1;
    TTabSheet *ts_STD;
    TPageControl *PageControl2;
    TTabSheet *ts_STD_Log;
    TTabSheet *ts_STD_BinLog;
    TTabSheet *ts_STD_Setup;
    TTabSheet *ts_STD_Simu;
    TMemo *MemoLog;
    TMemo *MemoBinData;
    TPanel *Panel2;
    TCheckBox *cbShowLog;
    TButton *btClear;
    TCheckBox *cbSaveLog;
    TGroupBox *GroupBox1;
    TLabel *Label1;
    TLabel *Label2;
    TLabel *Label3;
    TLabel *Label4;
    TLabel *Label5;
    TComboBox *cbBaudRate;
    TComboBox *cbByteSize;
    TComboBox *cbStopBit;
    TComboBox *cbParity;
    TComboBox *cbDevice;
    TPanel *btnUpdate;
    TButton *btManualTest;
    TButton *btnAllUse;
    TButton *btnAllNoUse;
    TTabSheet *ts_AVAGO;
    TPageControl *PageControl3;
    TTabSheet *TabSheet4;
    TPageControl *PageControl4;
    TTabSheet *ts_AVAGOLogSite1;
    TMemo *mm_AvagoLogSite1;
    TCheckBox *cb_AvagoShowLogSite1;
    TButton *bt_AvagoClearLogSite1;
    TCheckBox *cb_AvagoSaveLogSite1;
    TTabSheet *TabSheet2;
    TButton *bt_UpdateSite1;
    TGroupBox *GroupBox2;
    TLabel *Label17;
    TLabel *Label18;
    TLabel *Label25;
    TLabel *Label26;
    TLabel *Label27;
    TComboBox *cb_AvagoBaudSite1;
    TComboBox *cb_AvagoSizeSite1;
    TComboBox *cb_AvagoStopSite1;
    TComboBox *cb_AvagoParitySite1;
    TComboBox *cb_AvagoComSite1;
    TGroupBox *GroupBox3;
    TLabel *Label28;
    TLabel *Label29;
    TLabel *Label30;
    TLabel *Label31;
    TComboBox *cb_AvagoDTRSite1;
    TComboBox *cb_AvagoRTSSite1;
    TCheckBox *cb_AvagoTxCSite1;
    TEdit *ed_AvagoTimeOutTotalSite1;
    TEdit *ed_AvagoTimeOutSite1;
    TTabSheet *TabSheet5;
    TPageControl *PageControl6;
    TTabSheet *TabSheet6;
    TMemo *mm_AvagoLogSite2;
    TCheckBox *cb_AvagoShowLogSite2;
    TButton *bt_AvagoClearLogSite2;
    TCheckBox *cb_AvagoSaveLogSite2;
    TTabSheet *TabSheet7;
    TButton *bt_UpdateSite2;
    TGroupBox *GroupBox4;
    TLabel *Label32;
    TLabel *Label33;
    TLabel *Label34;
    TLabel *Label35;
    TLabel *Label36;
    TComboBox *cb_AvagoBaudSite2;
    TComboBox *cb_AvagoSizeSite2;
    TComboBox *cb_AvagoStopSite2;
    TComboBox *cb_AvagoParitySite2;
    TComboBox *cb_AvagoComSite2;
    TGroupBox *GroupBox5;
    TLabel *Label38;
    TLabel *Label39;
    TLabel *Label40;
    TLabel *Label41;
    TComboBox *cb_AvagoDTRSite2;
    TComboBox *cb_AvagoRTSSite2;
    TCheckBox *cb_AvagoTxCSite2;
    TEdit *ed_AvagoTimeOutTotalSite2;
    TEdit *ed_AvagoTimeOutSite2;
    TTabSheet *TabSheet1;
    TLabel *Label42;
    TLabel *Label43;
    TButton *bt_SimulateManualTestSite2;
    TEdit *ed_SimulateNu;
    TEdit *ed_Number;
    TGroupBox *GroupBox14;
    TLabel *Label6;
    TEdit *edReadIntervalTimeout;
    TLabel *labDebugMode;
    TTabSheet *tsTTL;
    TPageControl *PageControl7;
    TTabSheet *ts_TTL_BinLog;
    TMemo *MemoBinData_TTL;
    TPanel *Panel1;
    TCheckBox *cbShowLog_TTL;
    TButton *btClear_TTL;
    TCheckBox *cbSaveLog_TTL;
    TTabSheet *ts_TTL_Setup;
    TGroupBox *GroupBox15;
    TLabel *Label7;
    TLabel *Label8;
    TLabel *Label9;
    TLabel *Label10;
    TLabel *Label11;
    TComboBox *cbBaudRate_TTL;
    TComboBox *cbByteSize_TTL;
    TComboBox *cbStopBit_TTL;
    TComboBox *cbParity_TTL;
    TComboBox *cbDevice_TTL;
    TPanel *btnUpdate_TTL;
    TGroupBox *GroupBox16;
    TLabel *Label12;
    TEdit *edReadIntervalTimeout_TTL;
    TTabSheet *ts_TTL_Simu;
    TButton *Button8;
    TButton *Button9;
    TButton *btnTTL_Manual;
    TButton *btnClearSot;
    TTabSheet *tsLog;
    //    TMemo *MemoLog;
    TComm *CommTester_TTL;
    TComm *CommTester_TTL_2;
    TLabel *Label13;
    TComboBox *cbDevice_TTL_2;
    TButton *Button10;
    TButton *Button11;
    TLabel *labOcr;
    TButton *btnEnableSOTCSOT;

    // TForm leftovers golden code touches
    AnsiString Caption;

    // ---- golden __published event handlers ----
    void btnUpdateClick(TObject *Sender);
    void FormShow(TObject *Sender);
    void CommTesterReceiveData(TObject *Sender, void* Buffer, WORD BufferLength);
    void btClearClick(TObject *Sender);
    void Timer1Timer(TObject *Sender);
    void btManualTestClick(TObject *Sender);
    void btnUpdateMouseDown(TObject *Sender);      // golden (Sender, Button, Shift, X, Y): visual only
    void btnUpdateMouseUp(TObject *Sender);
    void btnAllUseClick(TObject *Sender);
    void btnAllNoUseClick(TObject *Sender);
    void FormClose(TObject *Sender);               // golden (TObject*, TCloseAction&)
    void CommTester_TTLReceiveData(TObject *Sender, void* Buffer, WORD BufferLength);
    void btnUpdate_TTLClick(TObject *Sender);
    void btnUpdate_TTLMouseDown(TObject *Sender);
    void btnUpdate_TTLMouseUp(TObject *Sender);
    void btClear_TTLClick(TObject *Sender);
    void btnTTL_ManualClick(TObject *Sender);
    void btnClearSotClick(TObject *Sender);
    void CommTester_TTL_2ReceiveData(TObject *Sender, void* Buffer, WORD BufferLength);
    void Button10Click(TObject *Sender);
    void Button11Click(TObject *Sender);
    void btnEnableSOTCSOTClick(TObject *Sender);
    void FormCreate(TObject *Sender);
    void FormDestroy(TObject *Sender);

    // ---- golden private ----
    int iMaxBinCount;
    std::vector<TMyDutPanel*> MY_DUT_PAL;
    int CustomerCode;

    // ---- golden public ----
    TfRS232Main();
    ~TfRS232Main();

    void OnMyCopyMsg(TMessage &msg);
    void SendResultFinish();
    void SendMSG_CMD(int CMD);
    void SendMSG_CMD(int CMD, AnsiString Message);
    void ProcessHandlerConnect();

    bool CloseTesterComm();
    bool CloseTesterComm_TTL(int iboard = 0);
    bool OpenTesterComm();
    bool OpenTesterComm_TTL(int board = 0);
    void SendCommandToTester(std::vector<Byte>& Data);
    void SendCommandToTester_TTL(std::vector<Byte>& Data, int iBoard = 0);
    void ShowCommData(AnsiString sUnitName, std::vector<Byte> bHex);
    void ShowCommData(AnsiString sUnitName, AnsiString sMsg1, AnsiString sMsg2 = "");
    void ShowCommDataHToR(int iRecvCommand, AnsiString sMsg = "");
    void ShowCommDataRToH(int iRecvCommand, AnsiString sMsg = "");
    void ShowCommData_TTL(AnsiString sType, std::vector<Byte> Data, int iDataType = 0, int iboard = 0);
    void ShowVersion();

    std::vector<Byte> ReceiveData;
    std::vector<Byte> ReceiveData1;
    std::vector<Byte> SendData;
    std::vector<Byte> SendData1;
    std::vector<Byte> SendStartData;
    std::vector<Byte> SendStartData1;

    bool bCommConnect[3];
    bool bFind;

    TDateTime dtPresent;
    Word OldSystemSec;
    bool bTwoArmTestMode;
    int  iTwoArmTestStep;
    void AddBinData(AnsiString sBinData);
    void SaveBinData();

    TStringList *slCmdList;
    bool SaveSetupData(const TComm* Comm);
    bool SaveSetupData_TTL();
    AnsiString ComNameTester;
    AnsiString ComNameTTL;
    AnsiString ComNameTTL_2;
    bool LoadSetupData_TTL();
    bool LoadSetupData(TComm* Comm);
    void ShowInterface();
    void InitialBarcodeList();
    int iHasCE;
    AnsiString GetAnalysisString(std::vector<Byte> &ReceiveData);
    void Del1stVec(std::vector<Byte> &ReceiveData);
    void DoRevCommand(AnsiString sCommand);
    uSocketServer* uServer;
    void ReceiveData_TCPIP(char* cGet, int iRevLen);
    int  iCheckClosedSiteHasBin;

    TMyStringList *slRS232Log;

    // ---- V906 plumbing (not golden; same contract as gpibbridge::TSerialPoll's) ----
    webbridge::WbMutex uiMutex;                  // guards the web snapshot copy (P7), not the golden widgets
    void PostToHandler(COPYDATASTRUCT* pcp);     // golden SendMessage(HMountWnd, WM_COPYDATA, 0, pcp)
    void RequestClose(const char* reason);       // golden Close()
    void UiNotice(AnsiString text);              // golden MessageDlg/ShowMessage (non-blocking)
    bool closeRequested;
    AnsiString closeReason;
    AnsiString lastNotice;
    testercomm::SyncMailbox* mailbox;            // set by Rs232Engine before FormCreate

    // Receive marshalling: vclcompat TComm / TServerSocket fire on THEIR threads; golden handlers run on the form's
    // (= TesterComm) thread.  Callbacks only QueueRx(); Rs232Engine::RunOnce calls DrainRx() on the TesterComm
    // thread, which dispatches to CommTesterReceiveData / CommTester_TTLReceiveData / CommTester_TTL_2ReceiveData /
    // ReceiveData_TCPIP (each gets a NUL-terminated copy, golden handlers may strlen() it).
    enum RxPort { kRxTester = 0, kRxTtl1 = 1, kRxTtl2 = 2, kRxTcp = 3 };
    void QueueRx(int port, const void* buf, Word len);   // any thread
    void DrainRx();                                        // TesterComm thread only
    webbridge::WbMutex rxMutex;
    std::vector<std::pair<int, std::string> > rxQueue;
};

extern TfRS232Main *fRS232Main;   // golden MainForm.cpp:14

}  // namespace rs232std

// ---------------------------------------------------------------------------
//  Golden build switches, applied HERE -- after every include this header makes, as golden MainForm.cpp:16-17
//  come after its includes.  (The per-file copies of this block in the translated .cpp files are then no-ops.)
//  NB: the Handler's MachineType.h (via MessageDef.h) already defines SOFT_SIMULTE unless W906_NO_SOFT_SIMULTE;
//  the #undef makes RS232STD_GOLDEN_SOFT_SIMULTE=0 effective for this program regardless.
// ---------------------------------------------------------------------------
#if RS232STD_GOLDEN_DEBUG
#ifndef DEBUG
#define DEBUG
#endif
#endif
#if RS232STD_GOLDEN_SOFT_SIMULTE
#ifndef SOFT_SIMULTE
#define SOFT_SIMULTE
#endif
#else
#undef SOFT_SIMULTE
#endif

#endif
