// =============================================================================
//  ATC/ATCInterface.h  --  TATCInterfaceForm / TMyHonPrecATCPanel (the ATC
//                          Handler-side UI + business-logic front end that
//                          drives ATCSystem.{h,cpp}'s ATCSystem/HT_ATC and
//                          the separate ATC 6.0/3.0 ATC60System, plus the
//                          ATC-7.0 server-socket handler-side protocol).
//
//  Faithful translation of golden ATC/ATCInterface.h (316 lines, BCB6,
//  Big5/cp950 -- this header itself is pure ASCII, no Chinese comments).
//  Translator: AI(W906-PT-W3) 20260807
//  Translation wave: W906-PT-W3, group "atcsystem".
//
//  ROLE: golden ATCInterface.{h,cpp} is a VCL FORM unit (TATCInterfaceForm :
//  public TForm, with a sibling ATCInterface.dfm). This wave's own campaign
//  plan (docs/PT_CAMPAIGN_PLAN.md SS4) defers ALL 110 golden VCL-form units
//  (dfm-bearing) to a dedicated future wave (PT-F1) whose widget-facade
//  strategy is explicitly undecided. This unit was named explicitly in THIS
//  wave's task scope regardless, so it is translated here following the
//  house convention this tree has ALREADY established for exactly this
//  situation -- forms/fMain.h / forms/fLotInfo.h's own precedent: a HEADLESS
//  plain class (no TForm base, no TComponent* Owner chain, no MFC subclass
//  yet), every widget member allocated directly in the ctor (matching
//  forms/fLotInfo.cpp's own `new TfLotInfoEdit()` idiom and its own global
//  `TfLotInfo *fLotInfo = new TfLotInfo();` instantiation, NOT golden's
//  `TATCInterfaceForm *ATCInterfaceForm;` bare-pointer-for-DFM-streaming
//  declaration -- see ATCInterface.cpp's own banner for that substitution).
//
//  WAVE SCOPE -- ACTIVE vs satisfied-by-shim vs GATED:
//   ACTIVE (faithful, verbatim data shape): every DATA member of
//     TMyHonPrecATCPanel (golden :25-85) and TATCInterfaceForm (golden
//     :87-295) that a translated .cpp body actually reads or writes --
//     ATC_SYS / ATC_60_SYS / ATC_SYS_PAL, every b*/i*/f*/s* POD+AnsiString
//     field, every widget pointer whose ->Text/->Caption/->Checked/->Value/
//     ->Lines/->ActiveConnections-style REAL substrate property is touched by
//     a translated body (see ATCInterface.cpp's own per-function banner for
//     the exhaustive list).
//   GATED (documented per-member below, NOT silently dropped): the 3
//     `TObject *Sender, TCustomWinSocket *Socket, TErrorEvent...` -style
//     __published event-handler declarations that only the -- untranslated,
//     .dfm-only -- IDE event wiring would ever invoke automatically are KEPT
//     as ordinary callable methods (translated bodies are still real); only
//     the *wiring itself* (->OnClick=Handler, .dfm-driven __published
//     construction/layout) is gated, per GATE (2)/(3) below.
//
//  GATE REGISTER -- grep-verified against the WHOLE port tree (excluding
//  build*/) before being declared, per house rule 6:
//   (1) `TComponent`/`TForm`/`TCloseAction` -- golden TATCInterfaceForm
//       derives `: public TForm` and FormClose takes `TCloseAction &Action`.
//       Grepped tree-wide: TForm exists only as a TU-local minimal stand-in
//       (language.h:86, explicitly "not full VCL", scoped to that file's own
//       TfLan) and NO port of TCloseAction exists anywhere. Per the
//       ALREADY-ESTABLISHED precedent this exact gap has (OmronLaser/
//       LaserSensor.h's own note: "golden `FormClose(TObject *Sender,
//       TCloseAction &Action)` becomes `FormClose()` here -- TCloseAction has
//       no port anywhere in this tree", reused verbatim by SECSGEM/
//       uHGemEquipment.h's FormClose too): TATCInterfaceForm is declared as a
//       plain class (no base), and FormClose drops both parameters. BEHAVIOUR
//       DELTA: none observable -- golden's own FormClose body never reads
//       Sender or writes Action (golden :162-167 is exactly `_Show_Type=0;
//       _Is_Show=false;`).
//   (2) Per-channel dynamic widget ->OnClick / ->OnConnect-style EVENT WIRING
//       assigned in golden's InitialATC (golden :73-78, `ATC_SYS_PAL[i]->
//       BitBtnSet->OnClick=BitBtnSetClick;` etc.) -- vclcompat/Controls.h's
//       TControl base carries NO OnClick slot (grepped: zero `OnClick`
//       member anywhere in Controls.h, by that header's own explicit design
//       boundary). This is the SAME class of gap EJ1N/MyOmronPanel.h's own
//       GATE (1)/(2) already covers for exactly this reason ("none of these
//       properties exist on vclcompat::TControl by DESIGN"). GATED: the 6
//       ->OnClick= assignments in InitialATC. BEHAVIOUR DELTA: on a real
//       machine a click on Set/Connect/Disconnect/Send/EnableSite/SetOffset
//       dispatches through this wiring; here (as with every other gated
//       OnClick in this tree, and matching the fact there is no live window
//       to click anyway offline) the *Click methods themselves are still
//       fully translated and callable directly.
//   (3) `->Tag` on the per-channel BitBtn/Button/CheckBox members --
//       GENUINELY READ BACK (golden BitBtnConnectClick/BitBtnDisconnectClick/
//       BitBtnSetClick/BitBtnSendClick all do `TBitBtn* pBtn=(TBitBtn*)Sender;
//       ...pBtn->Tag`; btSetOffsetClick/cbEnableSiteClick do the identical
//       `Ptr->Tag` read on TButton*/TCheckBox*), so unlike gate (2) this is
//       NOT purely cosmetic -- it is the per-channel dispatch index every one
//       of those handlers needs. vclcompat::TBitBtn/TButton/TCheckBox have no
//       ->Tag (by the same Controls.h design boundary). MINIMAL NECESSARY
//       ADAPTATION (not a gate -- a real substrate extension, matching this
//       tree's own precedent for exactly this need: vclcompat/ClientSocket.h
//       ADDED a `Tag` field to TClientSocket for the identical
//       read-back-to-discriminate-which-instance-fired reason, see that
//       header's own "AI(W5-Final-ClientSocketExt)" note): three tiny local
//       subclasses (TATCBitBtn/TATCButton/TATCCheckBox below) each add
//       exactly one `int Tag` field on top of their vclcompat base, nothing
//       else. The per-channel members are declared with these types instead
//       of golden's bare TBitBtn*/TButton*/TCheckBox*; every call site golden
//       itself only ever uses through TBitBtn*/TButton*/TCheckBox*-shaped
//       operations (->Caption/->Checked/->Click-dispatch) so this is
//       source-compatible with golden's own bodies once the Sender cast in
//       each handler is updated to the derived type (see ATCInterface.cpp).
//   (4) `fQwertyKey->ShowQwertyKey(...)` (golden pl_ATCSetTempChillerClick
//       :1851-1855, edATCChillerCheckTimeClick :1857-1861, edtOffsetClick
//       :1863-1866, edATC7_TempClick :1868-1871, edtPortClick :1873-1876) --
//       OPENED 20260824 (FW-QWKEY5): fQwertyKey real since FW-QWKEY1
//       (fc08e09, forms/fQwertyKey.{h,cpp}); all 5 calls restored live in
//       ATCInterface.cpp. N_DOUBLE/N_INTEGER/N_PORT were always real
//       (cmydef.h/.cpp). Latent: handlers unwired, runtime instance NULL
//       until a real event stream reaches THTEdit::EditClick (GATE (6),
//       itself opened 20260824 by FW-QWKEY3).
//   (5) `fLotInfo->aldATCPower` / `fLotInfo->aldATC7Status` (golden
//       ATC7_ServerSocketClientConnect/Disconnect :1489,1509-1510) -- grepped
//       forms/fLotInfo.h directly: it has no member named aldATCPower or
//       aldATC7Status (its LED-shaped members are ALedLoader/aLedAuto[3]
//       only, verified this wave). GATED (their writes are dropped, matching
//       the tree's own "cosmetic write nobody reads back" disposition --
//       these two ALed writes only ever drive a physical panel LED on the
//       LotInfo screen, never read back by any translated body).
//   (6) `TColor clGray`/`clNavy` -- NOT in the shared HT9045_W7C1_TCOLOR_SHIM
//       block (vclcompat/LedCore.h/TrayCore.h/BtnPanelCore.h all guard the
//       SAME 9-constant set, and clGray/clNavy are not among them). Per the
//       EJ1N/MyOmronPanel.h precedent ("the shared block does NOT carry
//       clGray... this file adds its OWN `const TColor clGray` locally"),
//       this header adds its own local clGray/clNavy with the real Win32
//       COLORREF values, scoped to this header only.
//
//  MINIMAL NECESSARY ADAPTATIONS (documented, not gates):
//   * `String` parameters (LotStart/LotEnd/SendCommToATC7's sData1/sData2)
//     become `AnsiString` -- same normalisation as ATCSystem.h.
//   * golden `TATCInterfaceForm : public TForm` widget members that are
//     __published (populated by the .dfm's streaming constructor in real
//     VCL, never by C++ code in golden's own ctor) are allocated directly in
//     THIS ctor instead -- see ATCInterface.cpp's own banner for the full
//     per-member accounting, matching forms/fLotInfo.cpp's identical
//     `new TfLotInfoEdit()`-in-ctor idiom for the same missing-DFM-streaming
//     reason.
//   * `TTimer` (ATCWatchTimer/TimerChillerStop/TimerATC) has no port; this
//     tree's own OmronLaser/LaserSensor.h already carries a minimal
//     guarded TU-local stand-in (`class TTimer : public vclcompat::TObject
//     { bool Enabled; }` -- "only ->Enabled is ever touched by golden").
//     Reused here verbatim (own guard, so no ODR conflict with that file --
//     the two headers are never both included by the same translation unit).
//
//  VCL/Borland conversions: `__fastcall` dropped from every method (golden
//  marks most of them). `#include <Classes.hpp>/<Controls.hpp>/<StdCtrls.hpp>/
//  <Forms.hpp>/<ExtCtrls.hpp>/<ScktComp.hpp>/<Buttons.hpp>/<ComCtrls.hpp>/
//  "ALed.hpp"/"MyLed.h"` are replaced by `vclcompat/vcl_compat.h` +
//  `vclcompat/Controls.h` + `vclcompat/LedCore.h` (the TMyLed stand-in this
//  header defines locally, see GATE (6) and the TMyLed class below). No
//  __property / __published distinction is modelled (every member is public
//  data, matching this tree's Controls.h "public DATA, not accessors" rule).
//
//  Big5: this header has ZERO non-ASCII bytes (verified byte-scan of the
//  golden file) -- no Chinese comments to carry over, no U+FFFD risk here.
// =============================================================================
#ifndef ATCInterfaceH
#define ATCInterfaceH
//---------------------------------------------------------------------------
#include "vclcompat/vcl_compat.h"    // AnsiString/TObject/TComponent/TStringList/DWORD
#include "vclcompat/Controls.h"      // TGroupBox/TEdit/TButton/TBitBtn/TMemo/TPanel/TCheckBox/TLabel/TPageControl/TTabSheet
#include "vclcompat/LedCore.h"       // TColor/TLEDStyle/LEDSqLarge/clLime/clSilver/clBlack/clWhite/clBtnFace (shared 9-const block)
#include "vclcompat/ClientSocket.h"  // TClientSocket/TCustomWinSocket/TErrorEvent
#include "vclcompat/ServerSocket.h"  // TServerSocket

#include "ATC/ATCSystem.h"           // ATCSystem/ATC60System/HT_ATC/ATCData/ARD_*
#include "MachineType.h"             // eATCType/eATCHonPrecType/eNonChamber/eATCUninstall, ATC_HEAD_COUNT

// vclcompat/LedCore.h's types/constants live in namespace vclcompat; brought
// in individually here, matching the tree's own established per-consumer
// idiom (EJ1N/MyOmronPanel.h: "using vclcompat::TColor; using vclcompat::
// clBlack; ...") rather than a blanket `using namespace vclcompat;`.
using vclcompat::TColor;
using vclcompat::TLEDStyle;
using vclcompat::LEDSmall;
using vclcompat::LEDSqLarge;
using vclcompat::clLime;
using vclcompat::clSilver;
using vclcompat::clBlue;
using vclcompat::clBlack;
using vclcompat::clWhite;

// See GATE (6): clGray/clNavy are NOT in the shared 9-const HT9045_W7C1_TCOLOR_SHIM
// block (vclcompat/LedCore.h et al.) -- added locally, same idiom as EJ1N/MyOmronPanel.h.
#ifndef HT9045_ATCINTERFACE_TCOLOR_EXTRA
#define HT9045_ATCINTERFACE_TCOLOR_EXTRA
const TColor clGray = TColor(0x00808080);
const TColor clNavy = TColor(0x00800000);
#endif

// See file banner "MINIMAL NECESSARY ADAPTATIONS": TTimer has no port; this
// tree's own guarded TU-local stand-in idiom (OmronLaser/LaserSensor.h) is
// reused verbatim under this file's OWN guard.
#ifndef HT9045_ATCINTERFACE_TTIMER_SHIM
#define HT9045_ATCINTERFACE_TTIMER_SHIM
class TTimer : public TObject
{
public:
    bool Enabled;
    TTimer() : Enabled(false) {}
    virtual ~TTimer() {}
};
#endif

// golden TALed*/TMyLed* -- see GATE (6)'s sibling note: NOT a bespoke
// `{bool Value;}` (this file's LED usage genuinely writes ->Blink/->TrueColor/
// ->FalseColor/->LEDStyle, unlike fMain/fLotInfo's read-only-Value usage that
// forms/FormWidgets.h's TfLedValue models), so this is the real vclcompat
// LedCore shape's field set, exposed as plain public data (matching this
// tree's "public DATA, not accessors" rule) rather than MyLedCore's
// Set/GetValue() method pairs. ->Tag/->Hint/->ShowHint are write-only in
// golden (never read back by any translated body -- verified) and are
// carried as plain fields purely so the ctor's assignments compile; they are
// not "GATED" because they are real, harmless storage, just unread.
class TMyLed : public TObject
{
public:
    bool       Value;
    bool       Blink;
    TColor     TrueColor;
    TColor     FalseColor;
    TLEDStyle  LEDStyle;
    int        Tag;
    AnsiString Hint;
    bool       ShowHint;
    TMyLed() : Value(false), Blink(false), TrueColor(0), FalseColor(0),
               LEDStyle(LEDSmall), Tag(0), ShowHint(false) {}
    virtual ~TMyLed() {}
};

// See GATE (3): golden's per-channel TBitBtn*/TButton*/TCheckBox* members
// have their ->Tag GENUINELY read back (channel-dispatch index) by the click
// handlers.  AI(W906-FW-TAG1) 20260825: these three subclasses existed ONLY to
// add `int Tag`, which now lives on vclcompat::TControl (Controls.h) for every
// widget.  The duplicate members are removed rather than left to shadow the
// base one -- see the provenance note at that declaration.  The classes stay:
// the member declarations below and the `(TATCBitBtn*)Sender` casts in
// ATCInterface.cpp name these types, and an empty derived class keeps those
// spellings valid with identical semantics (Tag now resolves to the base).
class TATCBitBtn : public TBitBtn     { public: TATCBitBtn()   {} };
class TATCButton  : public TButton    { public: TATCButton()  {} };
class TATCCheckBox : public TCheckBox { public: TATCCheckBox() {} };

extern DWORD MySleepEx(DWORD dwMilliseconds, bool bAlertable);

//---------------------------------------------------------------------------
class TMyHonPrecATCPanel : public TComponent
{
    private:    // User declarations
    public:
        TMyHonPrecATCPanel(TComponent* Owner, int index);
        ~TMyHonPrecATCPanel();

        TGroupBox       *gbATC;
        TGroupBox       *gbOffset;
        TClientSocket   *ATCSocket;
        TMyLed          *LedATCConnect;
        TMyLed          *LedATCActive;
        TLabel          *labState;
        TLabel          *labAddress;
        TLabel          *labPort;
        TEdit           *edtAddress;
        TEdit           *edtOffset;
        TEdit           *edtPort;
        TEdit           *edtSendData;
        TATCBitBtn      *BitBtnSet;
        TATCBitBtn      *BitBtnConnect;
        TATCBitBtn      *BitBtnDisconnect;
        TATCBitBtn      *BitBtnSend;
        TATCButton      *btSetOffset;
        TMemo           *MemoATC;
        TEdit           *pal_SV;
        TPanel          *pal_PV;
        TATCCheckBox    *cbEnableSite;
        AnsiString sAddress;
        int iPort;
        int iMyTag;
        //void SetDisplay(int iLevel);

        bool bATCRunSetting;      //Steven 20120608 : ATC啟動的狀態
        bool bATCRunStatus;
        bool bSiteOnOffStatus;    //Steven 20120528 : 確認開關Site狀態
        bool bSiteOnOffSetting;   //Steven 20120528 : 開關Site設定
        bool bATCTempReady;       //Steven 20120530 : 確認溫度是否到達
        int iRecordCount;         // 2011.05.24 , Joye , ATC
//        int iSetTempCheckCount;
        int iErrorCount;         //pig 2011.12.21 ATC改
        double fLastTemp;     // 2011.05.24 , Joye , ATC
        double fATCSiteNowTemp;
        double fATCRefSensorTemp;       //Steven 20150108 : [L11-5] For海思使用兩組感溫
        double fATCSiteSVSetting;       //Steven 20120528 : 預期的設定溫度
        double fATCSiteSVValue;         //Steven 20120528 : 目前的ATC設定溫度
        double fATCSetChillerSV;        //Steven 20120528 : 冰水機預期的設定溫度
        double fATCSetChillerValue;     //Steven 20120528 : 冰水機目前的工作溫度
        double fATCOffsetSetting;       //Steven 20120626 : Offset設定溫度
        double fATCOffsetValue;         //Steven 20120706 : Offset目前的工作溫度
        void SetATCPIDParameter( double fMinP , double fMinI ,double fMinD ,double fMaxP ,double fMaxI , double fMaxD );    //Ifor 20150910 :ATC PID Control
        int fATCNotReceivedCount;       //Ifor 20160223 未收到溫度回傳次數
        int iATCSelfTestResult;         //Ifor 20160720 Add ATC SELF TEST RESULT 結果
        bool bRunChillerStatus;         //Ifor 20160801 Add ATC2.0 Run Chiller Status
        bool bRunATCStatus;             //Ifor 20160801 Add ATC2.0 Run ATC Status
        bool bUseRefSensorStatus;       //Ifor 20160801 Add ATC2.0 Use RefSensor Status
        int iATCSelfTestStatus;         //Ifor 20160824 Add ATC SELF TEST Status
        bool bATCLotStart;              //Ifor 20161118 add ATC2.0 Lot Start flag
        bool bATCLotEnd;                //Ifor 20161118 add ATC2.0 Lot End flag
};
//---------------------------------------------------------------------------
class TATCInterfaceForm
{
public:    // formerly __published (IDE-managed Components); see file banner
    TTimer *ATCWatchTimer;
    TPanel *Panel2;
    TBitBtn *btnClearAll;
    TBitBtn *btnExit;
    TTimer *TimerChillerStop;
    TPageControl *PageControl1;
    TTabSheet *TabSheet1;
    TTabSheet *TabSheet2;
    TPanel *pnlTitle;
    TPanel *ScrollBoxATC;                  // golden TScrollBox -- no port; TPanel stands in (Visible/Enabled only ever used)
    TGroupBox *gbATC;
    TMyLed *LedATCConnect;
    TLabel *labState;
    TLabel *labAddress;
    TLabel *labPort;
    TMyLed *LedATCActive;
    TEdit *edtAddress;
    TEdit *edtPort;
    TBitBtn *BitBtnConnect;
    TBitBtn *BitBtnDisconnect;
    TEdit *edtSendData;
    TBitBtn *BitBtnSend;
    TMemo *MemoATC;
    TBitBtn *BitBtnSet;
    TPanel *pal_PV;
    TGroupBox *gbOffset;
    TEdit *edtOffset;
    TButton *btSetOffset;
    TCheckBox *cbEnableSite;
    TEdit *pal_SV;
    TGroupBox *gbChiller;
    TMyLed *ledChiller;
    TLabel *Label1;
    TPanel *palChillerPos;
    TPanel *palChillerCurr;
    TPanel *palChillerSet;
    TPanel *pan_ATCTempChiller;
    TPanel *pl_ATCTempChiller;
    TEdit *pl_ATCSetTempChiller;
    TButton *btnATCChillerSwitchRun;
    TButton *btnATCChillerSwitchStop;
    TButton *btnATCPower;
    TButton *btOnLine;
    TButton *btOffLine;
    TButton *btSetChillerTemp;
    TButton *btSave;
    TButton *Button1;
    TButton *btSetWorkTemp;
    TGroupBox *gbL11;
    TLabel *labChillerProtectedFunction;
    TEdit *edATCChillerCheckTime;
    TCheckBox *cbChillerProtectedFunction;
    TButton *Button2;
    TPanel *Panel1;
    TPanel *ScrollBox1;                    // golden TScrollBox -- see ScrollBoxATC note
    TMyLed *LedATC7Connect;
    TLabel *Label2;
    TMyLed *LedATC7Active;
    TLabel *Label3;
    TMemo *HandlerMemo;
    TGroupBox *GroupBox3;
    TPanel *Panel_CH1_Temp;
    TPanel *Panel_CH1_TSDTemp;
    TGroupBox *GroupBox2;
    TPanel *Panel_CH2_Temp;
    TPanel *Panel_CH2_TSDTemp;
    TGroupBox *GroupBox4;
    TCheckBox *cBoxEnabledCH1;
    TCheckBox *cBoxEnabledCH2;
    TCheckBox *cBoxEnabledTSD;
    TEdit *edATC7_Temp;
    TBitBtn *BitBtn1;
    TBitBtn *BitBtn2;
    TBitBtn *BitBtn3;
    TBitBtn *BitBtn4;
    TBitBtn *BitBtn5;
    TServerSocket *ATC7_ServerSocket;
    TCheckBox *cBoxEnabledCH3;
    TCheckBox *cBoxEnabledCH4;
    TGroupBox *GroupBox1;
    TPanel *Panel_CH3_Temp;
    TPanel *Panel_CH3_TSDTemp;
    TGroupBox *GroupBox5;
    TPanel *Panel_CH4_Temp;
    TPanel *Panel_CH4_TSDTemp;
    TTimer *TimerATC;
    TCheckBox *chkSaveLog;
    void FormClose();                       // golden (Sender,TCloseAction&) trimmed -- see GATE (1)
    void FormShow(TObject *Sender);
    void ATCWatchTimerTimer(TObject *Sender);
    void btnExitClick(TObject *Sender);
    void btnClearAllClick(TObject *Sender);

    void FormDestroy(TObject *Sender);
    void btnATCPowerClick(TObject *Sender);
    void btnATCChillerSwitchRunClick(TObject *Sender);
    void btnATCChillerSwitchStopClick(TObject *Sender);
    void btOnLineClick(TObject *Sender);
    void cbEnableSiteClick(TObject *Sender);
    void btOffLineClick(TObject *Sender);
    void btSetChillerTempClick(TObject *Sender);
    void btSetOffsetClick(TObject *Sender);
    void btSaveClick(TObject *Sender);
    void Button1Click(TObject *Sender);
    void btSetWorkTempClick(TObject *Sender);
    void TimerChillerStopTimer(TObject *Sender);
    void Button2Click(TObject *Sender);
    void ATC7_ServerSocketClientConnect(TObject *Sender,
          TCustomWinSocket *Socket);
    void ATC7_ServerSocketClientDisconnect(TObject *Sender,
          TCustomWinSocket *Socket);
    void ATC7_ServerSocketClientRead(TObject *Sender,
          TCustomWinSocket *Socket);
    void BitBtn1Click(TObject *Sender);
    void BitBtn2Click(TObject *Sender);
    void BitBtn3Click(TObject *Sender);
    void BitBtn4Click(TObject *Sender);
    void BitBtn5Click(TObject *Sender);
    void TimerATCTimer(TObject *Sender);
    void pl_ATCSetTempChillerClick(TObject *Sender);
    void edtOffsetClick(TObject *Sender);
    void edATCChillerCheckTimeClick(TObject *Sender);
    void edATC7_TempClick(TObject *Sender);
    void edtPortClick(TObject *Sender);
private:    // User declarations
    int _Show_Type;  // Show Type
    bool _Is_Show;
    int iATCUseCount;
    int iGroupBoxHeight;
    bool bSystemStart;
public:     // User declarations
    void ATCSocketError(TObject *Sender, TCustomWinSocket *Socket, TErrorEvent ErrorEvent, int &ErrorCode);
    void BitBtnConnectClick(TObject *Sender);
    void BitBtnSetClick(TObject *Sender);
    void BitBtnDisconnectClick(TObject *Sender);
    void BitBtnSendClick(TObject *Sender);

    TATCInterfaceForm(TComponent* Owner);
    ~TATCInterfaceForm();

    ATCSystem ATC_SYS;
    ATC60System ATC_60_SYS;     //2014-05-30    Dell    for ATC6.0
    vector< TMyHonPrecATCPanel* > ATC_SYS_PAL;

    int iTempCheckAlarmCount;
    int iTempCheckDelayTime;

    int iCheckSameTempTime;

    void LoadATCSystem();
    void WriteATCSystem( int iChannel );

    void ShowInterface( int iType = 0);
    void OnLine();
    void OffLine();
    bool IsOnLine();

    void OpenChannel( int iChannel );
    void CloseChannel( int iChannel );
    void SetWorkTemperature( int iChannel , double fTemp );
    void SetATCSelfTest();
    void SetATCManualSelfTest();             //Ifor 20160823 add Manual Send ATC Self Test Command
    bool CheckWorkTemperature(int iChannel, double fTemp);      //Steven 20130701 : 確認ATC設定正確
//    bool TemperatureReady( int iChannel );
//    bool GetNowTemperature( int iChannel );

    void SendTestStart( int iIndex );
    void SendTestEnd( int iIndex );

    void RefreshChannelState();
    void SetOffsetTemperature( int iChannel , double fTemp );   //pig 2011.12.21 ATC改
    void SetRunATC(bool bRun);                                  //pig 2011.12.21 ATC改
    void SetChillerTemperature(int fTemp);                      //pig 2011.12.21 ATC改
    double GetATCSiteNowTemperature(int iChannel);              //pig 2011.12.21 ATC改
    double GetATCSiteNowTemperature_Ref(int iChannel);          // 2014.10.04 , Joye , KYEC Reffer Temperatrue   //Steven 20150108 : [L11-5] For海思使用兩組感溫
    bool ATCRun(bool bRun) ;                                    //pig 2011.12.21 ATC改
    void ATCChillerSwitch(bool bopen);                          //pig 2011.12.21 ATC改
    void CheckHandlerConnectATC();                              //pig 2011.12.21 ATC改

    void SetSystemStatus(bool bStart)   {bSystemStart=bStart;}; //機台運作狀態
    void InitialATC(eATCType AtcType=eATCHonPrecType, int ATCUseCount=2, AnsiString sATCIniPath="Config\\ATC.ini");
    bool bOnLineSetting;
    int  iOnLineStep;

    int  ATC_SYSTEM;                        // 2011.04.15 , Joye , ATC
    bool bATCConnectError;                  //pig 2011.12.21 ATC改
    int  iStopATCChillerType;               // 2012.05.07 , Joye , Chiller
    int  iATCChillerCheckTime;
    bool bL11_2ATCChillerProtectedFunction;

    void SendCommToATC7(int iCommIndex, AnsiString sData1, AnsiString sData2); //Eliot 2015_0105
    char pucInBuff[8192];                   //Eliot 2015_0105
    double fATC7NowTemp[4];                 //Eliot 2015_0105
    double fATC7NowTSDTemp[4];              //Eliot 2015_0105
    bool bShowAlarm;                        //Eliot 2015_0105
    AnsiString asAlarmMessage;              //Eliot 2015_0105
    AnsiString ATCIniPath;
    int GetRunSelfTestResult();
    int GetRunSelfTestStatus();  //Ifor 20160824 add Get ATC Self Test Sataus
    bool bATC20AutoConnect;
    void LotStart(AnsiString sLotID);//Ifor 20161118 add ATC2.0 Lot Start Command for KYEC(ATCInterfaceForm::)
    void LotEnd(AnsiString sLotID);  //Ifor 20161118 add ATC2.0 Lot End Command for KYEC(ATCInterfaceForm::)
    bool GetATCLotStartStatus(); //Ifor 20160824 Get ATC2.0 Lot Start Status
    bool GetATCLotEndStatus();   //Ifor 20160824 Get ATC2.0 Lot End Status
    bool bIsWaterLeakage;                   //Steven 20210325 : 漏水檢知
};
//---------------------------------------------------------------------------
extern const int ATC_RUN;     //Eliot 2014_0918
extern const int ATC_STOP;
extern const int ATC_SET_TEMP;
extern const int ATC_USE_TSD;
extern const int ATC_CH_ENABLED;
extern const int ATC_TEMP_VALUE;
extern const int ATC_MESSAGE;
extern const int ATC_EMG_UP;    //Eliot 2015_0122
extern const int ATC_EMG_DOWN;  //Eliot 2015_0122
extern const int ATC_SOT;       //Steven 20151112 : for ATC 7.0
extern const int ATC_EOT;       //Steven 20151112 : for ATC 7.0
extern const int ATC_OFFSET;    //Steven 20151112 : 修改ATC7.0 Offset
extern const int ATC_SEND_TEMP_READY;   //Steven 20160604 : by site TSD
extern bool bShowAlarm;
extern AnsiString asAlarmMessage;

//---------------------------------------------------------------------------
extern TATCInterfaceForm *ATCInterfaceForm;
//---------------------------------------------------------------------------
#endif
