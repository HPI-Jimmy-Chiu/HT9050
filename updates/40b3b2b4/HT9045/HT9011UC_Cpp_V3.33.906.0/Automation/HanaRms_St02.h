// ===========================================================================
//  Automation/HanaRms_St02.h -- golden 912 TfAutomation's HANA RMS Interlock members (the HANA Micron recipe
//  server link), as class THanaRmsMembers.  It is the base class of TfAutomationShim (atester_shims.h), so golden's
//  fAutomation->PrepareHANARMSConnect() / ->HANARMSRunCheckOK() / ->HANARMSQueryJobInfo() / ->bHANARMSNeedRunCheck
//  / ->sHANARMSJobInfoLine compile as written.
//
//  AI(W906-ST02-C10) 20261002 (St02-E helper).  Card ST02-C10.  Steven W64 = A (1002 08:0x, decisions-decided.md):
//  "A 912版本是比較新的, 另外Hana目前由RogerYang維護, 以RogerYang的註解為準" -- an exception to RULINGS_20261001 #26
//  for the HANA part.  Plan: docs/handoff/ST02_H013_PLAN_20261001.md section 4.3 (3a) on v906/steven-handoff.
//
//  Golden: D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy (Big5, read as cp950):
//    Automation/automation.h:35 / :43 / :59-81       tsHANARMSInterlock, mmoHANARMS, the RMS widgets, HANARMSClient
//    Automation/automation.h:117-136                 the four HANARMSClient events and the eleven button events
//    Automation/automation.h:138-139 / :157          aSystem* and GetTimeInfo (the TfAutomation member the bodies call)
//    Automation/automation.h:192-203                 ShowFormAsOLP / ShowFormAsHANARMS / LoadHANARMSSetting /
//                                                    PrepareHANARMSConnect and the Stage 3 interlock members
//    Automation/automation.cpp:148-149               ctor: bHANARMSNeedRunCheck=false; LoadHANARMSSetting();
//    Automation/automation.cpp:280-289               GetTimeInfo
//    Automation/automation.cpp:2514-3158             the bodies: HanaRms_St02.cpp, PrepareHANARMSConnect in
//                                                    HanaRms_Prepare_St02.cpp
//    Automation/automation.dfm:468-723 / :765-775    widget defaults; HANARMSClient (Active False, ctNonBlocking, Port 6670)
//  Callers in golden 912: main.cpp:18535-18539 WakeupGPIB (G9, TesterComm/Handler/HandlerBridgeCtl.cpp -- LIFTED with
//  this card); the rest are plan 3b and stay where they are (atester.cpp:1396 = T08, aTester_Front.cpp:5562 / :6556,
//  aTester_Rear.cpp:5783 / :6815, HANA_ART.cpp:344-345, main.cpp:4545-4547 Start, main.cpp:27537-27543 State Record,
//  cConfiguration.cpp:7933-7937 the A77 button).
//
//  Files
//    HanaRms_St02.h            this header (class + the two composition-root entry points)
//    HanaRmsMembers_St02.cpp   ctor / dtor + the real-socket policy flag.  NO socket code: atester_shims.cpp's
//                              `new TfAutomationShim()` pulls only this object, so programs that link ht9045_sm but
//                              never reach the RMS code do not get ClientSocket.cpp (ws2_32) through it.
//    HanaRms_St02.cpp          golden automation.cpp:2514-3158 except PrepareHANARMSConnect
//    HanaRms_Prepare_St02.cpp  PrepareHANARMSConnect only (golden :3049-3074; the one member another library calls)
//    HanaRmsPump_St02.cpp      W906_HanaRmsPumpTick only (called by TesterComm/Handler/TesterCommWiring.cpp)
//    HanaRms_Internal_St02.h   what HanaRms_St02.cpp shares with HanaRms_Prepare_St02.cpp (golden file-statics)
//
//  [W906] deviations (each marked at its line)
//    1. HANARMSClient: golden's dfm component -> a vclcompat TClientSocket made on first use
//       (W906_HANARMSClientCreate): SetPolled(true) = golden ctNonBlocking without a thread (H-008 contract,
//       vclcompat/ClientSocket.cpp end of file); Sim unless the composition root called W906_HanaRmsSetRealSocket(true)
//       (TesterCommWiring.cpp: the TCP tester's W906_TcpTesterRealSocket policy -- SIM build Sim, SHIP real unless
//       HT9045_TCPCMD_SIM=1).  A ctest never opens an OS socket: nothing in a ctest sets the flag.
//    2. The ctor does NOT call LoadHANARMSSetting() (golden :149): fAutomation is created during static
//       initialisation (atester_shims.cpp:370), so an ini read there is the BA-SIOF1 hazard (Automation/automation.h
//       :354-361).  PrepareHANARMSConnect loads it itself (golden :3057) before it is used.
//    3. Paths: golden literals "D:\\HT9045_Log\\HANARMS" -> as9045LogPath+"\\HANARMS" and
//       "D:\\HT9045\\config\\HANARMS.ini" / "D:\\HT9045\\Config\\HANARMS.ini" -> AuthPath+"HANARMS.ini" (common.cpp:240 /
//       :168; same values in production, the ctest seams W906_HT9045LOG_ROOT / W906_AUTH_PATH otherwise).
//    4. HANARMSCompare's GB_CT operand (golden :2694 fMain->hanaART->GetHD_CT()): TfMainHanaART (forms/fMain.h:80-96)
//       has no GetHD_CT and uHANA_ART has no instance -> the Handler value is "(N/A)" = FAIL whenever the server sends a
//       GB_CT (golden's "-" / not sent = skipped as before).  Never passes silently.
//    5. bW906HANARMSConnecting: RogerYang's comment at golden :3062 "已連/連線中 -> 不重連" (connected OR connecting ->
//       do not reconnect); the polled socket's Active is false while a connect is pending (as VCL: Active is set on the
//       connect event), so the "connecting" half needs this flag.  W-195 H2 (20261009): golden 913 deleted that comment;
//       HANARMSConnect (golden 913 :3292-3326) keeps the flag as a guard -- while a connect is pending golden's Close
//       would not run (Active is false) and the polled socket ignores a second Open, so skipping only saves a second
//       "connecting to" log line (deviation recorded, census section 2a).
//    6. (dropped 1002 19:2x, St02-M) HANARMSQueryJobInfo is golden :3108-3109 verbatim: RogerYang's comment
//       "換批,舊基準作廢" and golden's code agree (this is the lot-open query, so every call voids the baseline).
//    7. __fastcall dropped from the members; HANARMSClientError's TErrorEvent is Scktcomp's.
//  NOT translated (the web screen for HANA RMS is a NEW design, question for ST01-M): ShowFormAsOLP /
//  ShowFormAsHANARMS (golden :3021-3039: tsOLP / tsHANARMSInterlock TabVisible, pgcSocketTCPIP->ActivePage, Caption,
//  Show) and the widgets that only serve that page (tsHANARMSInterlock, the labels, the buttons themselves).  The
//  button EVENT bodies are translated, so a screen can call them.
//  [912 known defects] kept as golden 912.0 (HUMAN_REVIEW C): see HanaRms_St02.cpp's banner.
// ===========================================================================
#ifndef HanaRms_St02H
#define HanaRms_St02H

#include "vclcompat/vcl_compat.h"   // AnsiString, TObject, Word; Scktcomp::TClientSocket / TCustomWinSocket / TErrorEvent (via ServerSocket.h)
#include "vclcompat/Controls.h"     // TMemo / TEdit / TLabel (headless widgets)

class THanaRmsMembers
{
public:
    // ---- golden automation.h __published widgets and component (headless) ------------------------------------------
    TMemo  *mmoHANARMS;                     // golden automation.h:43  (automation.dfm:471)
    TEdit  *edtHANARMSIP;                   // golden automation.h:64  (dfm :543-556, Text '127.0.0.1')
    TEdit  *edtHANARMSPort;                 // golden automation.h:65  (dfm :557-564, Text '6670')
    TEdit  *edtHANARMSManCmd;               // golden automation.h:67  (dfm :598-610, no Text)
    TLabel *lblRMS_Status;                  // golden automation.h:81  (dfm :530-542, Caption 'Disconnected')
    Scktcomp::TClientSocket *HANARMSClient; // golden automation.h:72  (dfm :765-775) -- [W906] 1: made on first use

    // ---- golden automation.h:117-136 events -------------------------------------------------------------------------
    void HANARMSClientConnect(TObject *Sender, Scktcomp::TCustomWinSocket *Socket);         // golden automation.cpp:2838-2845
    void HANARMSClientDisconnect(TObject *Sender, Scktcomp::TCustomWinSocket *Socket);      // golden automation.cpp:2847-2854
    void HANARMSClientRead(TObject *Sender, Scktcomp::TCustomWinSocket *Socket);            // golden automation.cpp:2856-2907
    void HANARMSClientError(TObject *Sender, Scktcomp::TCustomWinSocket *Socket,
                            Scktcomp::TErrorEvent ErrorEvent, int &ErrorCode);              // golden automation.cpp:2909-2916
    void btnHANARMSConnClick(TObject *Sender);                                              // golden automation.cpp:2918-2936
    void btnHANARMSDisConnClick(TObject *Sender);                                           // golden automation.cpp:2938-2949
    void btnHANARMSManCmdClick(TObject *Sender);                                            // golden automation.cpp:2951-2955
    void btnRMS_ClearLogClick(TObject *Sender);                                             // golden automation.cpp:2957-2962
    void btnRMS_GetLotClick(TObject *Sender);                                               // golden automation.cpp:2964-2968
    void btnRMS_GetJobClick(TObject *Sender);                                               // golden automation.cpp:2970-2974
    void btnRMS_EventClick(TObject *Sender);                                                // golden automation.cpp:2976-2980
    void btnRMS_GetTimeClick(TObject *Sender);                                              // golden automation.cpp:2982-2986
    void btnRMS_AliveClick(TObject *Sender);                                                // golden automation.cpp:2988-2992
    void btnSaveHANARMSLogClick(TObject *Sender);                                           // golden automation.cpp:2994-3011
    void btnHANARMSSettingClick(TObject *Sender);                                           // golden automation.cpp:3013-3019

    // ---- golden automation.h:157 / :194-203 ---------------------------------------------------------------------------
    AnsiString GetTimeInfo();                       // golden automation.h:157 (body automation.cpp:280-289)
    void LoadHANARMSSetting();                      // golden automation.h:194 //開機載入 IP/Port 到欄位 (body :3041-3047)
    void PrepareHANARMSConnect();                   // golden automation.h:195 //呼叫 GPIB 前 ready 連線 (body :3049-3074)
    //RogerYang 20260902 : HANA RMS recipe 互鎖(Stage3)
    AnsiString sHANARMSLot;                         // golden automation.h:197 //開批查詢中的 lot
    AnsiString sHANARMSJobInfoLine;                 // golden automation.h:198 //最後一包 GET_JOB_INFO_REP 原文(比對基準)
    bool bHANARMSNeedRunCheck;                      // golden automation.h:199 //Start 後第一次測試才比對的旗標
    void HANARMSSendCmd(AnsiString asCmd);          // golden automation.h:200 //送一句 RMS 命令(自動補\n) (body :3077-3101)
    void HANARMSQueryJobInfo(AnsiString asLot);     // golden automation.h:201 //開批自動查詢(LOT_INFO→JOB_INFO) (body :3103-3114)
    bool HANARMSCheckRecipe(AnsiString &asFail);    // golden automation.h:202 //以基準重比 recipe (body :3116-3134)
    bool HANARMSRunCheckOK(bool bShowAlarm);        // golden automation.h:203 //測試起點互鎖閘門 (body :3136-3158)
    //RogerYang 20260916 : RMS 連線改隨需建立(HANA 伺服器 30 秒 idle 斷線)        AI(W906-W195) 20261009 H2: golden 913 automation.h:204-211
    AnsiString sHANARMSPending;                     //未連線時存放的待送指令(\n 分隔)
    AnsiString sHANARMSLinkErr;                     //最後一次連線失敗原因(空=無)
    unsigned long dwHANARMSSentTick;                //查詢開始時刻,供「無回應 N 秒」判讀
    bool HANARMSIsConnected();                      //連線是否真的可用 (golden 913 automation.cpp:3278-3290)
    void HANARMSConnect();                          //非阻塞連線,連上後沖出待送 (:3292-3326)
    void HANARMSFlushPending();                     //把排隊指令送出 (:3328-3347)
    AnsiString HANARMSGetHDName();                  //HDNAME 預設=機台 Machine ID (:3349-3358)
    bool HANARMSCheckLinkBeforeStart();             //Start 前置:連不上 RMS 就拒絕開始 (AI(W906-W195) 20261009 H4: golden 913 automation.h:212, body automation.cpp:3462-3503)

    // ---- [W906] ---------------------------------------------------------------------------------------------------------
    void W906_HANARMSClientCreate();                // [W906] 1: the dfm component, made on first use (HanaRms_St02.cpp)
    bool bW906HANARMSConnecting;                    // [W906] 5: a polled connect is pending (RogerYang :3062)
    void (*W906_pfnHANARMSClientFree)(Scktcomp::TClientSocket *p);   // [W906] 1: the dtor frees the socket through this, so
                                                    //   HanaRmsMembers_St02.cpp never names ~TClientSocket (see Files above)

    THanaRmsMembers();                              // HanaRmsMembers_St02.cpp (golden automation.cpp:146-149 + the dfm)
    ~THanaRmsMembers();
    THanaRmsMembers(const THanaRmsMembers&) = delete;
    THanaRmsMembers& operator=(const THanaRmsMembers&) = delete;

private:
    Word aSystemHour, aSystemMin, aSystemSec, aSystemMSec;     // golden automation.h:138 (private) -- GetTimeInfo's
    Word aSystemYear, aSystemMonth, aSystemDate;               // golden automation.h:139
};

// ---- composition-root entry points (Handler tick thread; TesterComm/Handler/TesterCommWiring.cpp) --------------------
void W906_HanaRmsSetRealSocket(bool bReal);   // [W906] 1: true = real WinSock (polled) for the next socket made; default false = Sim (HanaRmsMembers_St02.cpp)
void W906_HanaRmsPumpTick();                  // [W906] 1: HANARMSClient->Poll() -- finishes a connect, reads, notices the server close; fires the golden events on this thread (HanaRmsPump_St02.cpp)

#endif // HanaRms_St02H
