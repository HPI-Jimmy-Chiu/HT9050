// =============================================================================
//  ATC/ATCInterface.cpp  --  TATCInterfaceForm / TMyHonPrecATCPanel bodies.
//
//  Faithful translation of golden ATC/ATCInterface.cpp (1878 lines, BCB6,
//  Big5/cp950).
//  Translator: AI(W906-PT-W3) 20260807
//  Translation wave: W906-PT-W3, group "atcsystem".  See ATCInterface.h's own
//  banner for the unit's role, WAVE SCOPE and full GATE REGISTER; this file
//  cites the same gate numbers inline at each call site instead of repeating
//  the rationale.
//
//  WAVE SCOPE -- every golden top-level function definition, golden line
//  numbers, ACTIVE unless marked otherwise:
//    TATCInterfaceForm::TATCInterfaceForm     golden :40-55   (ctor base + widget alloc ADAPTED, see banner)
//    TATCInterfaceForm::InitialATC            golden :57-84   (cosmetic Parent/Top lines GATED, see (2))
//    TATCInterfaceForm::BitBtnConnectClick     golden :86-90   (Sender cast ADAPTED, see (3))
//    TATCInterfaceForm::BitBtnDisconnectClick  golden :92-96   (ditto)
//    TATCInterfaceForm::BitBtnSetClick         golden :98-102  (ditto)
//    TATCInterfaceForm::BitBtnSendClick        golden :104-108 (ditto)
//    TATCInterfaceForm::~TATCInterfaceForm     golden :110-113
//    TATCInterfaceForm::FormShow               golden :115-160
//    TATCInterfaceForm::FormClose              golden :162-167 (params dropped, see (1))
//    TATCInterfaceForm::btnClearAllClick       golden :169-173
//    TATCInterfaceForm::btnExitClick           golden :175-178
//    TATCInterfaceForm::LoadATCSystem          golden :180-218
//    TATCInterfaceForm::WriteATCSystem         golden :220-234
//    TATCInterfaceForm::ShowInterface          golden :238-250
//    TATCInterfaceForm::OnLine                 golden :252-263
//    TATCInterfaceForm::OffLine                golden :265-269
//    TATCInterfaceForm::OpenChannel            golden :271-277
//    TATCInterfaceForm::CloseChannel           golden :279-287
//    TATCInterfaceForm::SetChillerTemperature  golden :289-302
//    TATCInterfaceForm::SetWorkTemperature     golden :304-317
//    TATCInterfaceForm::SetATCSelfTest         golden :319-322
//    TATCInterfaceForm::SetATCManualSelfTest   golden :324-327
//    TATCInterfaceForm::CheckWorkTemperature   golden :329-338
//    TATCInterfaceForm::RefreshChannelState    golden :350-376
//    TATCInterfaceForm::SendTestStart          golden :378-390
//    TATCInterfaceForm::SendTestEnd            golden :392-404
//    TATCInterfaceForm::IsOnLine               golden :406-409
//    TATCInterfaceForm::ATCWatchTimerTimer     golden :412-818
//    TATCInterfaceForm::SetOffsetTemperature   golden :821-827
//    TATCInterfaceForm::SetRunATC              golden :829-846
//    TATCInterfaceForm::ATCRun                 golden :848-851
//    TATCInterfaceForm::GetATCSiteNowTemperature      golden :853-859
//    TATCInterfaceForm::GetATCSiteNowTemperature_Ref  golden :861-867
//    TATCInterfaceForm::GetRunSelfTestResult   golden :869-872
//    TATCInterfaceForm::GetRunSelfTestStatus   golden :874-877
//    TATCInterfaceForm::GetATCLotStartStatus   golden :879-882
//    TATCInterfaceForm::GetATCLotEndStatus     golden :884-887
//    TATCInterfaceForm::LotStart               golden :889-893
//    TATCInterfaceForm::LotEnd                 golden :895-899
//    TATCInterfaceForm::ATCChillerSwitch       golden :901-910
//    TATCInterfaceForm::CheckHandlerConnectATC golden :912-916
//    TATCInterfaceForm::ATCSocketError         golden :919-934
//    TATCInterfaceForm::FormDestroy            golden :936-959
//    TMyHonPrecATCPanel::TMyHonPrecATCPanel    golden :961-1274 (cosmetic layout GATED, see (7); Tag-bearing + data-bearing lines ACTIVE)
//    TMyHonPrecATCPanel::~TMyHonPrecATCPanel   golden :1288-1312 (already all-comment in golden)
//    TATCInterfaceForm::btnATCPowerClick       golden :1314-1326
//    TATCInterfaceForm::btnATCChillerSwitchRunClick   golden :1328-1332
//    TATCInterfaceForm::btnATCChillerSwitchStopClick  golden :1334-1353
//    TATCInterfaceForm::btOnLineClick          golden :1355-1360
//    TATCInterfaceForm::btOffLineClick         golden :1362-1366
//    TATCInterfaceForm::cbEnableSiteClick      golden :1368-1379 (Sender cast ADAPTED, see (3))
//    TATCInterfaceForm::btSetChillerTempClick  golden :1381-1385
//    TATCInterfaceForm::btSetOffsetClick       golden :1387-1393 (Sender cast ADAPTED, see (3))
//    TATCInterfaceForm::btSaveClick            golden :1395-1399
//    TATCInterfaceForm::Button1Click           golden :1401-1408
//    TATCInterfaceForm::btSetWorkTempClick     golden :1410-1419
//    TATCInterfaceForm::TimerChillerStopTimer  golden :1421-1477
//    TATCInterfaceForm::Button2Click           golden :1479-1482
//    TATCInterfaceForm::ATC7_ServerSocketClientConnect     golden :1484-1502 (fLotInfo->aldATCPower write GATED, see (5))
//    TATCInterfaceForm::ATC7_ServerSocketClientDisconnect  golden :1504-1511 (ditto)
//    TATCInterfaceForm::ATC7_ServerSocketClientRead        golden :1513-1675
//    TATCInterfaceForm::SendCommToATC7         golden :1677-1733
//    TATCInterfaceForm::BitBtn1Click           golden :1735-1739
//    TATCInterfaceForm::BitBtn2Click           golden :1741-1744
//    TATCInterfaceForm::BitBtn3Click           golden :1746-1749
//    TATCInterfaceForm::BitBtn4Click           golden :1751-1755
//    TATCInterfaceForm::BitBtn5Click           golden :1757-1767
//    TATCInterfaceForm::TimerATCTimer          golden :1769-1849
//    TATCInterfaceForm::pl_ATCSetTempChillerClick    golden :1851-1855 (GATED, see (4))
//    TATCInterfaceForm::edATCChillerCheckTimeClick   golden :1857-1861 (GATED, see (4))
//    TATCInterfaceForm::edtOffsetClick               golden :1863-1866 (GATED, see (4))
//    TATCInterfaceForm::edATC7_TempClick             golden :1868-1871 (GATED, see (4))
//    TATCInterfaceForm::edtPortClick                 golden :1873-1876 (GATED, see (4))
//  Plus module-scope consts ATC_RUN..ATC_SEND_TEMP_READY (golden :25-38,
//  including golden's own ATC_MESSAGE_CLOSE=7 which the .h never externs
//  either -- kept .cpp-local, matching golden) and the global
//  `TATCInterfaceForm *ATCInterfaceForm` instance (golden :23 -- ADAPTED to an
//  eager `new`, see banner).
//
//  SATISFIED-BY-SUBSTRATE (already real in this tree): ATCSystem/HT_ATC/
//  ATC60System (ATC/ATCSystem.{h,cpp}, this SAME wave), TClientSocket/
//  TServerSocket/TCustomWinSocket (vclcompat), TStringList (vclcompat),
//  CheckAndReadIniData/WriteIniData family (common.h/.cpp), MyForceDirectories/
//  MySleep (common.h/.cpp), ShowErrorMessage/ShowMyMessage (canary_support.h,
//  sim bodies), MyDBIProcess (cMyDB.h decl, real body SECSGEM/uHGemEquipment.cpp),
//  fContact->fShow (atester_shims.h TfContactShim, real), Temperature/TestIF/
//  InputLimit (cprod.h), IniConfig (Config.h), LastSet/Tempture_Hot/rsmQAMode
//  (LastSet.h/cmydef.h), CosFunction.bHiSiliconFunction (CosFunction.h),
//  iATCForHSMode/sATCVerRead/bATCWaterTempAlarm (cmydef.h -- same globals
//  ATCSystem.cpp already reaches), ChangeToFloatNonPcnt (MachineType.h),
//  System{Year,Month,Date,Hour,Min,Sec} (cmydef.h), FormatDateTime/Now/
//  DecodeTime (vclcompat/TDateTime.*), StringReplace/TReplaceFlags/IntToStr
//  (vclcompat/SysUtils.*).
//
//  GATE REGISTER -- see ATCInterface.h's own numbered GATE REGISTER for the
//  full rationale of each; this file only cites the number at each site:
//   (1) TForm/TCloseAction -- header-level, see .h.
//   (2) per-channel dynamic-widget ->OnClick / cosmetic ->Parent/->Top wiring
//       in InitialATC (golden :69-78) -- vclcompat has no OnClick/Parent/Top
//       on TControl by design.
//   (3) Sender cast from `(TBitBtn*)`/`(TButton*)`/`(TCheckBox*)` to this
//       file's `(TATCBitBtn*)`/`(TATCButton*)`/`(TATCCheckBox*)` -- MINIMAL
//       NECESSARY ADAPTATION per header GATE (3), not a behaviour change
//       (the derived types ARE-A their vclcompat base with one added field).
//   (4) fQwertyKey->ShowQwertyKey -- OPENED 20260824 (FW-QWKEY5): entity real
//       since FW-QWKEY1; all 5 calls restored live, latent (handlers unwired).
//   (5) fLotInfo->aldATCPower / ->aldATC7Status -- no such members on the
//       ported forms/fLotInfo.h TfLotInfo; writes dropped.
//   (6) clGray/clNavy -- header-level, see .h.
//   (7) TMyHonPrecATCPanel ctor's cosmetic widget geometry (->Left/->Top/
//       ->Width/->Height/->Name/->Font->.../->Hint/->ShowHint on every one of
//       the 20 dynamically-created widgets, golden :1001-1274) -- NONE of
//       these properties exist on vclcompat::Controls.h's stand-ins BY DESIGN
//       (that header's own SCOPE BOUNDARY / MEASURED PROPERTY COVERAGE
//       sections). Grepped tree-wide before gating (per house rule 6): zero
//       ->Left/->Top/->Width/->Height/->Font/->Name/->Hint/->ShowHint members
//       anywhere in vclcompat/Controls.h, vclcompat/LedCore.h, or this file's
//       own TMyLed/TATCBitBtn/TATCButton/TATCCheckBox additions. Kept
//       VERBATIM in a single `#if 0` block per widget for provenance; the
//       ACTIVE arm keeps every field a translated body reads back afterwards
//       (sAddress/iPort/edtAddress->Text default IP, edtPort->Text default
//       port, edtOffset->Text=0.0 default, pal_SV->Text/pal_PV->Caption
//       "000.0℃" defaults, cbEnableSite->Caption, and every ->Tag on the 3
//       Tag-bearing widget types) plus the 20 `new T...(this)` allocations
//       (without which every ACTIVE business-logic body that dereferences
//       ATC_SYS_PAL[i]->whatever would be a null-pointer fault). BEHAVIOUR
//       DELTA, STATED PLAINLY: on a real machine this panel would render with
//       zero size/position/font (a real MFC/W7-U binder will need the W7-B1c
//       layout-table route this tree's OTHER form units already use, not
//       hand-written geometry here); every DATA-carrying default and every
//       Tag-driven dispatch this wave's ATCSystem business logic depends on
//       is preserved exactly.
//   (8) TATCInterfaceForm's own __published widget members (ATCWatchTimer,
//       TimerChillerStop, TimerATC, ledChiller, LedATC7Connect/Active,
//       HandlerMemo, Panel_CH1..4_Temp/_TSDTemp, cBoxEnabledCH1..4/TSD,
//       edATC7_Temp, ATC7_ServerSocket, edATCChillerCheckTime,
//       cbChillerProtectedFunction, pl_ATCSetTempChiller, pl_ATCTempChiller,
//       btnATCPower, Button1, chkSaveLog, gbL11, Button2, btSave,
//       btSetChillerTemp, btSetWorkTemp, and every other member a translated
//       body dereferences) are NOT constructed by any C++ code in golden at
//       all -- in real VCL the .dfm's streaming constructor (TReader,
//       ATCInterface.dfm, out of this wave's translation scope per the
//       campaign's own dfm boundary) allocates and configures every
//       `__published` member before TATCInterfaceForm's own ctor body even
//       runs. This is NOT a gap this translation introduces: it is the exact
//       same "no DFM streaming layer exists yet" gap every other ported form
//       unit in this tree already has, and this file follows the SAME
//       already-established fix -- forms/fLotInfo.cpp's own ctor explicitly
//       `new`s every one of its facade members (`cbRunMode = new
//       TfLotInfoRunMode(); ...`). This file's ctor does the identical thing
//       for every __published member an ACTIVE body below reaches. BEHAVIOUR
//       DELTA: none beyond gate (7)'s (no geometry/font either, same reason).
//
//  VCL/Borland conversions: `#pragma package(smart_init)` / `#pragma link
//  "MyLed"` / `#pragma resource "*.dfm"` dropped (golden :20-22 -- all three
//  are BCB6 IDE/linker directives with no meaning outside that toolchain).
//  golden's `#include "myQwertyKeyBoard.h"` / `"uLotInfo.h"` / `"cContact.h"`
//  / `"note.h"` are NOT re-included here: fQwertyKey/cContact's real class
//  are all GATED (4)/reached only via the already-established atester_shims.h
//  TfContactShim substitute (see below), and note.h's ShowMyMessage is
//  reached via canary_support.h instead (the tree's own established
//  substitute, matching every other translated unit's practice -- see e.g.
//  BarCode/BarCode_Shuttle1_Scan.h's own note). `Now().FormatString(fmt)`
//  (golden :1531,1726,1731) becomes `FormatDateTime(fmt, Now())` -- this
//  tree's TDateTime (vclcompat/TDateTime.h) exposes FormatDateTime only as a
//  free function, matching ATCSystem.cpp's own identical substitution for
//  DecodeTime; same underlying BCB6 operation, zero behaviour delta.
//  `String` parameters become `AnsiString` (see header banner).
//
//  Big5: every Chinese comment decoded via cp950 and preserved as UTF-8.
//  Final gate: ZERO U+FFFD.
// =============================================================================
#include "MachineDefine.h"

#include "ATC/ATCInterface.h"
#include "cmydef.h"                  // bATCWaterTempAlarm/iATCForHSMode/sATCVerRead/SystemYear../SnLotStart-family
#include "cprod.h"                   // Temperature/TestIF/InputLimit
#include "Config.h"                  // IniConfig.dATCAmbientTemperature
#include "common.h"                  // CheckAndReadIniData/WriteIniData family, MyForceDirectories
#include "LastSet.h"                 // LastSet.iTemperature/iRunStartMode, Tempture_Hot, rsmQAMode
#include "CosFunction.h"             // CosFunction.bHiSiliconFunction
#include "atester_shims.h"           // fContact (TfContactShim*, real ->fShow)
#include "aHotPlateSubstrate.h"      // MyDBIProcess(AnsiString,AnsiString) reach
#include "canary_support.h"          // ShowErrorMessage / ShowMyMessage (sim bodies)
#include "forms/fQwertyKey.h"       // AI(W906-FW-QWKEY5) 20260824: fQwertyKey extern (real since FW-QWKEY1 fc08e09) -- GATE (4) calls restored
#include "Public/WinSocketErrorCode.h" // GetErrorMsg -- ATCSystem.cpp's own ATC60System::ClientSocket1Error already needs it; not re-declared here (see that file)
bool W906_FormShowing(const char* goldenForm, bool member);   //AI(W906-PAGETAB-Q51) 20260928 [W906] 頁面表的單一函式（與 W906FormShowing.h／csystem.h:440 同一個宣告，本體 csystem.cpp:30049）；本檔不 include csystem.h ⇒ 宣告放在這個原本的空白行，不移動行號
// See GATE REGISTER (nothing) -- CONTACT_NORMAL is a plain `const int` golden
// keeps as a file-scope constant (cContact.cpp:74); this tree's own
// cContact.h/BarCode_Shuttle2_CCDScan.h precedent is to re-declare it locally
// per TU rather than pull in the whole (untranslated) cContact.h form unit --
// `const int` at file scope has internal linkage, so this is not an ODR risk.
const int CONTACT_NORMAL = 0;

//---------------------------------------------------------------------------
TATCInterfaceForm *ATCInterfaceForm = new TATCInterfaceForm(NULL);   //AI(W906-PT-W3) 20260807: golden declares this as a bare `TATCInterfaceForm *ATCInterfaceForm;` for the .dfm/Application::CreateForm streaming path (out of scope here) -- ADAPTED to an eager `new`, matching forms/fLotInfo.cpp's identical `TfLotInfo *fLotInfo = new TfLotInfo();` precedent for the same missing-streaming-layer reason (see GATE (8))
//---------------------------------------------------------------------------
const int ATC_RUN           =0;                                                 //Eliot 2015_0105
const int ATC_STOP          =1;
const int ATC_SET_TEMP      =2;
const int ATC_USE_TSD       =3;
const int ATC_CH_ENABLED    =4;
const int ATC_TEMP_VALUE    =5;
const int ATC_MESSAGE       =6;
const int ATC_MESSAGE_CLOSE =7;
const int ATC_EMG_UP        =8;                                                 //Eliot 2015_0122
const int ATC_EMG_DOWN      =9;                                                 //Eliot 2015_0122
const int ATC_SOT           =10;                                                //Steven 20151112 : for ATC 7.0
const int ATC_EOT           =11;                                                //Steven 20151112 : for ATC 7.0
const int ATC_OFFSET        =12;                                                //Steven 20151112 : 修改ATC7.0 Offset
const int ATC_SEND_TEMP_READY=13;                                               //Steven 20160604 : by site TSD
//---------------------------------------------------------------------------
TATCInterfaceForm::TATCInterfaceForm(TComponent* Owner)
    // golden : TForm(Owner) -- see GATE (1), this class has no base to init
{
#ifndef HANDLER_CONTROL_ATC
    // DEAD in this build: MachineType.h:12 `#define HANDLER_CONTROL_ATC`
    // unconditionally, so this arm never compiles for HT9045 (golden's own
    // configuration matches -- verified: no HT9045 build ever undefines it).
    InitialATC(eATCHonPrecType, 4, "Config\\ATC.ini");                          //預設使用4個加熱片,與預設讀檔位置
#endif
    // GATE (8): allocate every __published member a translated body below
    // reaches -- the .dfm streaming constructor golden relies on has no port.
    ATCWatchTimer      = new TTimer();
    TimerChillerStop   = new TTimer();
    TimerATC           = new TTimer();
    PageControl1       = new TPageControl();
    Panel2             = new TPanel();
    btnClearAll        = new TBitBtn();
    btnExit            = new TBitBtn();
    pnlTitle           = new TPanel();
    ScrollBoxATC       = new TPanel();
    gbChiller          = new TGroupBox();
    ledChiller         = new TMyLed();
    Label1             = new TLabel();
    palChillerPos      = new TPanel();
    palChillerCurr     = new TPanel();
    palChillerSet      = new TPanel();
    pan_ATCTempChiller = new TPanel();
    pl_ATCTempChiller  = new TPanel();
    pl_ATCSetTempChiller = new TEdit();
    btnATCChillerSwitchRun  = new TButton();
    btnATCChillerSwitchStop = new TButton();
    btnATCPower        = new TButton();
    btOnLine           = new TButton();
    btOffLine          = new TButton();
    btSetChillerTemp   = new TButton();
    btSave             = new TButton();
    Button1            = new TButton();
    btSetWorkTemp      = new TButton();
    gbL11              = new TGroupBox();
    labChillerProtectedFunction = new TLabel();
    edATCChillerCheckTime       = new TEdit();
    cbChillerProtectedFunction  = new TCheckBox();
    Button2            = new TButton();
    Panel1             = new TPanel();
    ScrollBox1         = new TPanel();
    LedATC7Connect     = new TMyLed();
    Label2             = new TLabel();
    LedATC7Active      = new TMyLed();
    Label3             = new TLabel();
    HandlerMemo        = new TMemo();
    GroupBox1          = new TGroupBox();
    GroupBox2          = new TGroupBox();
    GroupBox3          = new TGroupBox();
    GroupBox4          = new TGroupBox();
    GroupBox5          = new TGroupBox();
    Panel_CH1_Temp     = new TPanel();
    Panel_CH1_TSDTemp  = new TPanel();
    Panel_CH2_Temp     = new TPanel();
    Panel_CH2_TSDTemp  = new TPanel();
    Panel_CH3_Temp     = new TPanel();
    Panel_CH3_TSDTemp  = new TPanel();
    Panel_CH4_Temp     = new TPanel();
    Panel_CH4_TSDTemp  = new TPanel();
    cBoxEnabledCH1     = new TCheckBox();
    cBoxEnabledCH2     = new TCheckBox();
    cBoxEnabledCH3     = new TCheckBox();
    cBoxEnabledCH4     = new TCheckBox();
    cBoxEnabledTSD     = new TCheckBox();
    edATC7_Temp        = new TEdit();
    BitBtn1            = new TBitBtn();
    BitBtn2            = new TBitBtn();
    BitBtn3            = new TBitBtn();
    BitBtn4            = new TBitBtn();
    BitBtn5            = new TBitBtn();
    ATC7_ServerSocket  = new TServerSocket(NULL);
    chkSaveLog         = new TCheckBox();
    // The per-channel TMyHonPrecATCPanel members below (gbATC/LedATCConnect/
    // labState/labAddress/labPort/edtAddress/edtPort/BitBtnConnect/
    // BitBtnDisconnect/edtSendData/BitBtnSend/MemoATC/BitBtnSet/pal_PV/
    // gbOffset/edtOffset/btSetOffset/cbEnableSite/pal_SV/LedATCActive) are
    // golden's LEFTOVER top-level __published aliases of ATC_SYS_PAL[0]'s
    // dynamically-created widgets (golden main .dfm never actually populates
    // them independently -- InitialATC's loop builds the real per-channel
    // set into ATC_SYS_PAL[]); no translated body reads these top-level
    // aliases directly (every reference in golden's OWN bodies goes through
    // ATC_SYS_PAL[i]->... instead), so they are left NULL here, matching
    // "not constructed, not dereferenced" rather than allocating unread
    // objects.
    gbATC = NULL; LedATCConnect = NULL; labState = NULL; labAddress = NULL;
    labPort = NULL; edtAddress = NULL; edtPort = NULL; BitBtnConnect = NULL;
    BitBtnDisconnect = NULL; edtSendData = NULL; BitBtnSend = NULL;
    MemoATC = NULL; BitBtnSet = NULL; pal_PV = NULL; gbOffset = NULL;
    edtOffset = NULL; btSetOffset = NULL; cbEnableSite = NULL; pal_SV = NULL;
    LedATCActive = NULL; TabSheet1 = NULL; TabSheet2 = NULL;

    _Is_Show=false;
    _Show_Type=1;
    bOnLineSetting=false;
    bATCConnectError=false;                                                     //pig 2011.12.21 ATC改
    iStopATCChillerType=0;

    bShowAlarm=false;                                                           //Eliot 2015_0105
    asAlarmMessage="";                                                          //Eliot 2015_0105
    bIsWaterLeakage=false;                                                      //Steven 20210325 : 漏水檢知
}
//---------------------------------------------------------------------------
void TATCInterfaceForm::InitialATC(eATCType AtcType, int ATCUseCount, AnsiString sATCIniPath)
{
    iATCUseCount=ATCUseCount;                                                   //各機型自行定義使用數量,目前4個
    ATCIniPath=sATCIniPath;                                                     //各機型自行修改路徑
    iGroupBoxHeight=120;
    ATC_SYSTEM=AtcType;                                                         // 2011.04.15 , Joye , ATC
    ATC_SYS.Initial();
    for(int i=0; i<iATCUseCount; i++)                                           //決定使用幾個制冷片
    {
        ATC_SYS_PAL.push_back(new TMyHonPrecATCPanel(NULL, i));
        ATCChannel* tempATC=new HT_ATC(new TCPClient(ATC_SYS_PAL[i]->ATCSocket), i);
        ATC_SYS.AddChannel(tempATC);
        // GATE (2): gbATC->Parent=ScrollBoxATC / ->Top=110+iGroupBoxHeight*i --
        // TGroupBox carries no ->Parent/->Top (vclcompat/Controls.h design
        // boundary, see header GATE (7)/(2)).
        ATC_SYS_PAL[i]->LedATCConnect->Blink        =false;
        ATC_SYS_PAL[i]->LedATCActive->Blink         =false;
        // GATE (2): the 6 ->OnClick=Handler wirings (BitBtnSet/BitBtnConnect/
        // BitBtnDisconnect/BitBtnSend/cbEnableSite/btSetOffset) -- no OnClick
        // slot on vclcompat::TControl. The *Click methods themselves are
        // fully translated below and callable directly.
    }
    ATC_SYS.SetTemperaturePLimit(130.00);
    ATCWatchTimer->Enabled=true;
    if(ATC_SYSTEM==eATCHonPrecType)
        TimerChillerStop->Enabled=true;
}
//---------------------------------------------------------------------------
void TATCInterfaceForm::BitBtnConnectClick(TObject *Sender)
{
    TATCBitBtn* pBtn=(TATCBitBtn*)Sender;                                       //GATE (3): TBitBtn* -> TATCBitBtn* (Tag)
    ATC_SYS.Connect(pBtn->Tag);
}
//---------------------------------------------------------------------------
void TATCInterfaceForm::BitBtnDisconnectClick(TObject *Sender)
{
    TATCBitBtn* pBtn=(TATCBitBtn*)Sender;                                       //GATE (3)
    ATC_SYS.Disconnect(pBtn->Tag);
}
//---------------------------------------------------------------------------
void TATCInterfaceForm::BitBtnSetClick(TObject *Sender)
{
    TATCBitBtn* pBtn=(TATCBitBtn*)Sender;                                       //GATE (3)
    WriteATCSystem(pBtn->Tag);
}
//---------------------------------------------------------------------------
void TATCInterfaceForm::BitBtnSendClick(TObject *Sender)
{
    TATCBitBtn* pBtn=(TATCBitBtn*)Sender;                                       //GATE (3)
    ATC_SYS.SendCommand(pBtn->Tag, ATC_SYS_PAL[pBtn->Tag]->edtSendData->Text.c_str());
}
//---------------------------------------------------------------------------
TATCInterfaceForm::~TATCInterfaceForm()
{
    ATCWatchTimer->Enabled=false;
}
//---------------------------------------------------------------------------
void TATCInterfaceForm::FormShow(TObject *Sender)
{
    // GATE (2): ATCInterfaceForm->Top=5 / ->Left=10 -- no Top/Left on this
    // headless plain class (there is no window to position).

    if(ATCInterfaceForm->ATC_SYS_PAL[0]->LedATCConnect->Value!=true)            //Ifor 20160621 開啟Form時若ATC2.0 連線中不執行初始化，避免異常斷線
    {
        for(unsigned int i=0; i<ATC_SYS_PAL.size(); i++)
        {
            ATC_SYS_PAL[i]->gbATC       ->Enabled   = (_Show_Type==1);
            ATC_SYS_PAL[i]->LedATCActive->Value     = ATC_SYS_PAL[i]->bSiteOnOffStatus;
            ATC_SYS_PAL[i]->pal_SV      ->Text      = AnsiString(ATC_SYS_PAL[i]->fATCSiteSVValue);
            // GATE (2): ->OnClick=edATC7_TempClick/edtOffsetClick/edtPortClick wiring

    #ifdef HANDLER_CONTROL_ATC
            ATC_SYS_PAL[i]->pal_SV      ->Enabled   = false;
            ATC_SYS_PAL[i]->gbOffset    ->Enabled   = false;
            ATC_SYS_PAL[i]->cbEnableSite->Enabled   = false;
    #endif
        }

        pl_ATCSetTempChiller            ->Text      = AnsiString(ATC_SYS_PAL[0]->fATCSetChillerSV);
        pl_ATCTempChiller               ->Caption   = AnsiString(ATC_SYS_PAL[0]->fATCSetChillerValue);
        edATCChillerCheckTime           ->Text      = AnsiString(iATCChillerCheckTime);
        cbChillerProtectedFunction      ->Checked   = bL11_2ATCChillerProtectedFunction;

        if(_Show_Type==1)
        {
            // Load ATC Data ----------------------
            LoadATCSystem();
            // ------------------------------------
        }
    }
    #ifdef HANDLER_CONTROL_ATC
        pl_ATCSetTempChiller->Enabled=false;
        gbL11               ->Visible=false;
        Button1             ->Visible=false;
        Button2             ->Visible=false;
        btSave              ->Visible=false;
        btSetChillerTemp    ->Visible=false;
        btSetWorkTemp       ->Visible=false;
    #endif
        _Is_Show=true;
}
//---------------------------------------------------------------------------
void TATCInterfaceForm::FormClose()                                            //GATE (1): golden (Sender, TCloseAction&) -- both dropped
{
    _Show_Type=0;
    _Is_Show=false;
}
//---------------------------------------------------------------------------
void TATCInterfaceForm::btnClearAllClick(TObject *Sender)
{
    for(unsigned int i=0; i<ATC_SYS_PAL.size(); i++)
        ATC_SYS_PAL[i]->MemoATC->Clear();
}
//---------------------------------------------------------------------------
void TATCInterfaceForm::btnExitClick(TObject *Sender)
{
    FormClose();                                                               //GATE (1): golden `Close();` (real VCL TForm::Close) -- no window to close; the ONLY observable effect golden's own FormClose has (_Show_Type/_Is_Show) is applied directly
}
//---------------------------------------------------------------------------
void TATCInterfaceForm::LoadATCSystem()                                         //讀入INI內的設定值
{
    AnsiString sChannel;
    for(unsigned int i=0; i<ATC_SYS_PAL.size(); i++)
    {
        sChannel.sprintf("Channel_%d", i+1);
        ATC_SYS_PAL[i]->sAddress            =CheckAndReadIniData(ATCIniPath, sChannel, "Address", AnsiString("127.0.0.1"));
        ATC_SYS_PAL[i]->iPort               =CheckAndReadIniData(ATCIniPath, sChannel, "Port", int(1000+i));
#ifndef HANDLER_CONTROL_ATC                                                     //Steven 20120725 -- DEAD in this build, see ctor note
        ATC_SYS_PAL[i]->fATCOffsetSetting   =CheckAndReadIniData(ATCIniPath, sChannel, "Offset", 0.0);
        ATC_SYS_PAL[i]->fATCSiteSVSetting   =CheckAndReadIniData(ATCIniPath, sChannel, "WorkTemp", 0.0);          //12.06.27.01
#endif
        ATC_SYS.SetChannelSocket(i, ATC_SYS_PAL[i]->sAddress.c_str(), ATC_SYS_PAL[i]->iPort);
        ATC_SYS_PAL[i]->edtAddress->Text    =ATC_SYS_PAL[i]->sAddress;
        ATC_SYS_PAL[i]->edtPort->Text       =ATC_SYS_PAL[i]->iPort;
#ifndef HANDLER_CONTROL_ATC                                                      //Steven 20120725
        ATC_SYS_PAL[i]->edtOffset->Text     =ATC_SYS_PAL[i]->fATCOffsetSetting;
        ATC_SYS_PAL[i]->pal_SV->Text        =ATC_SYS_PAL[i]->fATCSiteSVSetting;                                   //12.06.27.01
#endif
    }

#ifndef HANDLER_CONTROL_ATC                                                     //Steven 20120725
    ATC_SYS_PAL[0]->fATCSetChillerSV=CheckAndReadIniData(ATCIniPath, "Setup", "Chiller Temp", 18.0);
    pl_ATCSetTempChiller->Text=AnsiString(ATC_SYS_PAL[0]->fATCSetChillerSV);
#endif

    // 2011.05.24 , Joey , ATC Setup ---------
    iTempCheckAlarmCount=CheckAndReadIniData(ATCIniPath, "Setup", "TempCheckAlarmCount", 10);
    iTempCheckDelayTime =CheckAndReadIniData(ATCIniPath, "Setup", "TempCheckDelayTime" , 1);
    // ---------------------------------------

    iATCChillerCheckTime                    =CheckAndReadIniData(ATCIniPath, "Setup", "ChillerCheckTime", 10);
    bL11_2ATCChillerProtectedFunction       =CheckAndReadIniData(ATCIniPath, "Setup", "ChillerProtectedFunction", false);
    edATCChillerCheckTime->Text             =AnsiString(iATCChillerCheckTime);
    cbChillerProtectedFunction->Checked     =bL11_2ATCChillerProtectedFunction;

    // 2011.05.27 , Joye , ATC Alarm
//    iControllerCheckDelay = CheckAndReadIniData( ATCIniPath , "Setup" , "ControllerCheckDelay"  , 5 );
}
//---------------------------------------------------------------------------
void TATCInterfaceForm::WriteATCSystem(int iChannel)
{
    AnsiString sChannel = "Channel_" + AnsiString(iChannel+1);

    ATC_SYS_PAL[iChannel]->sAddress     =ATC_SYS_PAL[iChannel]->edtAddress->Text;
    ATC_SYS_PAL[iChannel]->iPort        =atoi(ATC_SYS_PAL[iChannel]->edtPort->Text.c_str());
    iATCChillerCheckTime                =atoi(edATCChillerCheckTime->Text.c_str());
    bL11_2ATCChillerProtectedFunction   =cbChillerProtectedFunction->Checked;
    WriteIniData(ATCIniPath, sChannel, "Address",                   ATC_SYS_PAL[iChannel]->sAddress);
    WriteIniData(ATCIniPath, sChannel, "Port"   ,                   ATC_SYS_PAL[iChannel]->iPort);
    WriteIniData(ATCIniPath, sChannel, "Offset" ,                   ATC_SYS_PAL[iChannel]->fATCOffsetSetting);
    WriteIniData(ATCIniPath, "Setup",  "Chiller Temp",              ATC_SYS_PAL[0]->fATCSetChillerSV);
    WriteIniData(ATCIniPath, "Setup",  "ChillerCheckTime",          iATCChillerCheckTime);
    WriteIniData(ATCIniPath, "Setup",  "ChillerProtectedFunction",  bL11_2ATCChillerProtectedFunction);
}
//---------------------------------------------------------------------------
// Type 0 : Monitor Only
// Type 1 : Set & Manual Control
void TATCInterfaceForm::ShowInterface(int iType)
{
    _Show_Type = iType;

    if(_Is_Show==true)
    {
        FormClose();                                                           //GATE (1): golden `Close();` -- see btnExitClick's note
    }
    else
    {
        _Is_Show=true;                                                         //GATE (1): golden `Show();` (real VCL TForm::Show) -- no window to show; the observable state real VCL's own OnShow would drive (_Is_Show) is set directly, matching FormShow's own terminal assignment
    }
}
//---------------------------------------------------------------------------
void TATCInterfaceForm::OnLine()
{
    // Load ATC Data ----------------------
    LoadATCSystem();
    // ------------------------------------
    ATC_SYS.SetOnLine(true);

    bOnLineSetting=true;
    iOnLineStep=0;

    SetChillerTemperature(ATC_SYS_PAL[0]->fATCSetChillerSV);
}
//---------------------------------------------------------------------------
void TATCInterfaceForm::OffLine()
{
    ATC_SYS.SetOnLine(false);
    iATCForHSMode=0;                                                            //Ifor 20160912 ATC 通訊異常需清除ATC 版本，避免人員更換版本跑貨
}
//---------------------------------------------------------------------------
void TATCInterfaceForm::OpenChannel(int iChannel)
{
    ATC_SYS_PAL[iChannel]->bSiteOnOffSetting=true;
    ATC_SYS_PAL[iChannel]->bSiteOnOffStatus=false;                              //要反向才會動作
    ATC_SYS_PAL[iChannel]->cbEnableSite->Checked=ATC_SYS_PAL[iChannel]->bSiteOnOffSetting;
    ATC_SYS.SetChannelUse(iChannel, true);
}
//---------------------------------------------------------------------------
void TATCInterfaceForm::CloseChannel(int iChannel)
{
    ATC_SYS_PAL[iChannel]->bSiteOnOffSetting=false;
    ATC_SYS_PAL[iChannel]->bSiteOnOffStatus=true;                               //要反向才會動作
    ATC_SYS_PAL[iChannel]->cbEnableSite->Checked=ATC_SYS_PAL[iChannel]->bSiteOnOffSetting;
    ATC_SYS.SetChannelUse(iChannel, false);
    ATC_SYS_PAL[iChannel]->fATCSiteNowTemp=0.0;                                 //關掉時,就不讀取溫度
    ATC_SYS_PAL[iChannel]->fATCRefSensorTemp=0.0;                               //Steven 20150108 : [L11-5] For海思使用兩組感溫
}
//---------------------------------------------------------------------------
void TATCInterfaceForm::SetChillerTemperature(int fTemp)
{
    ATC_SYS_PAL[0]->fATCSetChillerSV=fTemp;
    ATC_SYS_PAL[0]->fATCSetChillerValue=fTemp-1;                                //要不一樣才會檢查
    ATCInterfaceForm->pl_ATCSetTempChiller->Text=AnsiString(ATC_SYS_PAL[0]->fATCSetChillerSV);
    ATC_SYS.SetChillerTemperature(fTemp);

    if(Temperature.bUsePIDControl==true &&                                      //Ifor 20150917 :Set PID
       CosFunction.bHiSiliconFunction==false)                                   //Ifor 20160621 海思版本鎖定PID設定(避免利用舊版本更改)
    {
        ATC_SYS.SetPIDParameter(Temperature.iATC_PID_Min_Offset[0]/100. , Temperature.iATC_PID_Min_Offset[1]/100. , Temperature.iATC_PID_Min_Offset[2]/100. ,
                                Temperature.iATC_PID_Max_Offset[0]/100. , Temperature.iATC_PID_Max_Offset[1]/100. , Temperature.iATC_PID_Max_Offset[2]/100.  );
    }
}
//---------------------------------------------------------------------------
void TATCInterfaceForm::SetWorkTemperature(int iChannel, double fTemp)
{
//    if(fTemp!=ATC_SYS_PAL[iChannel]->fATCSiteSVValue)
    {
        if(fTemp>((HT_ATC*)ATC_SYS.Channels[iChannel])->_Temperature_P_Limit)
        {
            fTemp=((HT_ATC*)ATC_SYS.Channels[iChannel])->_Temperature_P_Limit;
        }
        ATC_SYS_PAL[iChannel]->fATCSiteSVSetting=fTemp;
        // AI(W906-PT-W3-integrate) 20260808: RESTORED golden's own line (golden
        //   ATC/ATCInterface.cpp:313).  It had been ADAPTED to
        //   `AnsiString().sprintf("%0.1f", fTemp)` on the stated grounds that
        //   "golden's real-VCL FormatFloat has no port" -- that premise is FALSE.
        //   FormatFloat is real and has been for waves: declared
        //   vclcompat/SysUtils.h:41, Delphi picture-string implementation at
        //   vclcompat/SysUtils.cpp:125, pulled into the global namespace by
        //   vclcompat/vcl_compat.h:279, and already used unadapted elsewhere in
        //   this tree (Automation/auto9045.cpp:1721-1722, cUnitConvert.cpp:49).
        //   The two spellings agree on output for "0.0" (that implementation
        //   rounds with snprintf("%.*f", 1, v) then pads the integer part), so
        //   this is a fidelity restoration with no behaviour change -- but the
        //   adaptation should never have been made.
        ATC_SYS_PAL[iChannel]->pal_SV->Text=FormatFloat("0.0",fTemp);           //Steven 20120725 //jou 2013-07-11 AnsiString -> FormatFloat
        ATC_SYS_PAL[iChannel]->fATCSiteSVValue=fTemp-1.0;                       //要不一樣才會偵測的出來
        ATC_SYS.SetWorkTemperature(iChannel, fTemp);
    }
}
//---------------------------------------------------------------------------
void TATCInterfaceForm::SetATCSelfTest()
{
    ATC_SYS.SetATCSelfTest();
}
//---------------------------------------------------------------------------
void TATCInterfaceForm::SetATCManualSelfTest()                                  //Ifor 20160823 add Manual Send ATC Self Test Command
{
    ATC_SYS.SetATCManualSelfTest();
}
//---------------------------------------------------------------------------
bool TATCInterfaceForm::CheckWorkTemperature(int iChannel, double fTemp)        //Steven 20130701 : 確認ATC設定正確
{
    bool ret=true;
    if(ATC_SYS_PAL[iChannel]->fATCSiteSVValue!=fTemp ||
       ATC_SYS_PAL[iChannel]->fATCSiteSVSetting!=fTemp)
    {
        ret=false;
    }
    return ret;
}
//---------------------------------------------------------------------------
//bool TATCInterfaceForm::TemperatureReady(int iChannel)
//{
//    return ATC_SYS.GetTemperatureReady(iChannel);
//}
//---------------------------------------------------------------------------
//bool TATCInterfaceForm::GetNowTemperature(int iChannel)
//{
//    return ATC_SYS.GetNowTemperature( iChannel );                             // 2012.05.07 , ATC
//}
//---------------------------------------------------------------------------
void TATCInterfaceForm::RefreshChannelState()
{
//    if(_Is_Show==false)
//        return;

    for(unsigned int i=0; i<ATC_SYS_PAL.size(); i++)
    {
        if(ATC_SYS.IsConnect(i)==true)
        {
            ATC_SYS_PAL[i]->LedATCConnect   ->Value             =true;
            ATC_SYS_PAL[i]->labState        ->Caption           ="ON-Line";
            ATC_SYS_PAL[i]->labState        ->Color             =clBlue;       //GATE(7)-adjacent: golden ->Font->Color -- TLabel here carries a plain ->Color (no ->Font), see ctor note
            ATC_SYS_PAL[i]->pal_PV          ->Caption           =AnsiString(ATC_SYS_PAL[i]->fATCSiteNowTemp)+"℃";
        }
        else
        {
            ATC_SYS_PAL[i]->LedATCConnect   ->Value             =false;
            ATC_SYS_PAL[i]->labState        ->Caption           ="OFF-Line";
            ATC_SYS_PAL[i]->labState        ->Color             =clGray;
            ATC_SYS_PAL[i]->pal_PV          ->Caption           ="---";
        }
        ATC_SYS_PAL[i]->LedATCActive->Value=ATC_SYS_PAL[i]->bSiteOnOffStatus;
    }
    ledChiller->Value           =ATC_SYS.ChillerStatus();
    pl_ATCTempChiller->Caption  =AnsiString(ATC_SYS_PAL[0]->fATCSetChillerValue);
    btnATCPower->Caption        =(ATC_SYS_PAL[0]->bATCRunStatus)?"STOP ATC":"RUN ATC";
}
//---------------------------------------------------------------------------
void TATCInterfaceForm::SendTestStart(int iIndex)
{
    if(iIndex==0)                                                               //QQ 沒有考慮到關Arm
    {
        ATC_SYS.SendTestStart(0);
        ATC_SYS.SendTestStart(1);
    }
    else
    {
        ATC_SYS.SendTestStart(2);
        ATC_SYS.SendTestStart(3);
    }
}
//---------------------------------------------------------------------------
void TATCInterfaceForm::SendTestEnd(int iIndex)
{
    if(iIndex==0)                                                               //QQ 沒有考慮到關Arm
    {
        ATC_SYS.SendTestEnd(0);
        ATC_SYS.SendTestEnd(1);
    }
    else
    {
        ATC_SYS.SendTestEnd(2);
        ATC_SYS.SendTestEnd(3);
    }
}
//---------------------------------------------------------------------------
bool TATCInterfaceForm::IsOnLine()
{
    return ATC_SYS.IsOnLine();
}
//---------------------------------------------------------------------------
// ATC Watch Timer ----------------------------------------------------------
void TATCInterfaceForm::ATCWatchTimerTimer(TObject *Sender)
{
    static bool bTimerRun=false;
    static int iTimeOutCheck=0;
    string sLog;
    ATCData aData;
    TStringList *sAlarmCode;
    AnsiString Path;

    static int iCount=0;
    unsigned int i, j;
    double fNowTemp=0.0, fRefTemp=0.0;                                          //Ifor 20151008 add ATC 第二點溫度
    bool bOnOff=false;
    bool bHasReceive=false;                                                     //Ifor 20161118 add ATC 有收到命令 flag

    if(bTimerRun==true ||                                                       //Ifor 20160602 避免未執行完成再次進入
       InitialOK==false)                                                        //Steven 20160912 : Add InitialOK in Timer
    {
        iTimeOutCheck++;
        if(iTimeOutCheck>=50)                                                   //Ifor 20160805 add Time Out Check 避免程式永遠無法進入導致無法ATC2.0連線
        {
            iTimeOutCheck=0;
            bTimerRun=false;
        }
        return;
    }
    bTimerRun=true;

    //處理訊息 : 開始---------------------------------
    for(i=0; i<ATC_SYS_PAL.size(); i++)
    {
        if(ATC_SYS.PopData(i, aData)==true)
        {
            // ---------------------------------------
            if(aData.iType==ARD_SYS_ALARM ||                                         //系統訊息,一定要顯示的
               (aData.iType==ARD_ALARM && ATC_SYS_PAL[i]->bSiteOnOffSetting==true))  //Site訊息,如果沒有開啟就不需要ALarm
            {
                if(aData.Index==HT_ATC::ATC_CHILLER_CONNECT_ERROR)
                {
                    //SetRunATC(false);                                         //Ifor 20160625 Mark ATC2.0 收到Error 關閉ATC
                    //bRunATC=false;                                            //ChungHung 20160118 add for Hisi V102
                }
                #ifdef HANDLER_CONTROL_ATC
                    sAlarmCode=new TStringList();
                    sAlarmCode->CommaText=aData.sData.c_str();
                    if(sAlarmCode->Count>=2)
                        ShowErrorMessage(sAlarmCode->Strings[0], 0, MMSystem, false, sAlarmCode->Strings[1]); //Steven 20160706
                    else
                        ShowErrorMessage(sAlarmCode->Strings[0], 0, MMSystem);  //Steven 20160706
                    sAlarmCode->Clear();                                        //Ifor 20170603 (wei) TStringList 刪除前先 Clean
                    delete sAlarmCode;
                #endif
                ATC_SYS.SetATCCloseMessage();                                   //Ifor 20160602 清除 ATC 2.0 Alarm 訊息視窗
                bATCWaterTempAlarm=false;                                       //Ifor 20160625 add ATC Water Alarm 清除
                break;
            }
            else if(aData.iType==ARD_VALUE)                                     //處理數值資料
            {
                // MINIMAL NECESSARY ADAPTATION: golden's own case labels are
                // `case ((HT_ATC*)ATC_SYS.Channels[i])->ATC_XXX:` -- BCB6
                // accepts this because it only needs the ENUMERATOR's value
                // (identical for every HT_ATC instance; ATCCommandIndex is a
                // nested enum, not a virtual/instance property), but ISO
                // C++'s converted-constant-expression rule for switch/case
                // (verified against this exact GCC toolchain) does not apply
                // the "static/enumerator member access via -> does not
                // evaluate the object expression" carve-out here, and rejects
                // evaluating the runtime `ATC_SYS.Channels[i]` in a case
                // label. Rewritten as `HT_ATC::ATC_XXX` -- the identical
                // enumerator value, referenced via the type instead of a
                // runtime instance. ZERO behaviour delta (every one of the 23
                // sites this wave touches reads the SAME enum value no matter
                // which Channels[i] golden's own expression would have named).
                switch(aData.Index)
                {
                    case HT_ATC::ATC_CONNECT_X_OK:
                        break;
                    case HT_ATC::ATC_SETUP_TEMP_OK_XX:
                        break;
                    case HT_ATC::ATC_OFFSET_OK_XX:
                        ATC_SYS_PAL[i]->fATCOffsetValue=ATC_SYS_PAL[i]->fATCOffsetSetting;
                        break;
                    case HT_ATC::ATC_TEMP_READY_XX:
                        ATC_SYS_PAL[i]->bATCTempReady=true;                     //Steven 20120530 : 確認溫度是否到達
                        break;
                    case HT_ATC::ATC_TEMP_NOT_READY:
                        for(j=0; j<ATC_SYS_PAL.size(); j++)
                            ATC_SYS_PAL[i]->bATCTempReady=false;                //Steven 20120530 : 確認溫度是否到達
                        break;
                    case HT_ATC::ATC_SITE_XXXX:
                        fNowTemp=aData.fData;
                        ATC_SYS_PAL[i]->fATCSiteNowTemp=fNowTemp;               //pig 2011.12.21 ATC改
                        ATC_SYS_PAL[i]->fATCNotReceivedCount=0;                 //Ifor 20160223 清除未收到溫度回傳次數
                        if(Temperature.bUseReferTempSensor==true)               //Ifor 20151008 第二組溫度顯示
                        {
                            fRefTemp=aData.fReftemp;
                            ATC_SYS_PAL[i]->fATCRefSensorTemp=fRefTemp;
                        }
                        break;
                    case HT_ATC::ATC_SITE_STATE_X:
                        bOnOff=(aData.fData==1.0);
                        ATC_SYS_PAL[i]->bSiteOnOffStatus=bOnOff;
                        break;
                    case HT_ATC::ATC_WORK_TEMP_XXXX:
                        ATC_SYS_PAL[i]->fATCSiteSVValue=aData.fData;
                        break;
                    case HT_ATC::ATC_CHILLER_TEMP_XXXX:
                        ATC_SYS_PAL[i]->fATCSetChillerValue=aData.fData;        //Ifor 20160101 0->I
                        break;
                    case HT_ATC::ATC_RUN_ATC_FUNCTION_OK:
                        ATC_SYS_PAL[i]->bATCRunStatus=true;                     //Ifor 20160101 0->I
                        ATC_SYS.SetATCRun(true);
                        break;
                    case HT_ATC::ATC_STOP_ATC_FUNCTION_OK:
                        ATC_SYS_PAL[i]->bATCRunStatus=false;                    //Ifor 20160101 0->I
                        ATC_SYS.SetATCRun(false);
                        break;
                    case HT_ATC::ATC_RUN_CHILLER_OK:
                        ATC_SYS.SetChillerStatus(true);
                        break;
                    case HT_ATC::ATC_STOP_CHILLER_OK:
                        ATC_SYS.SetChillerStatus(false);
                        break;
                    case HT_ATC::ATC_READ_SW_VER:       //Ifor 20151023 Add ATC 2.0 讀取ATC軟體版本
                        sATCVerRead=aData.sATCSwVer.c_str();                    //Ifor 20161006 Fix ATC2.0 程式版本更新
                        break;
                    case HT_ATC::ATC_SET_TIME_SYNC_OK:  //Ifor 20151023 Add ATC 2.0 設定ATC電腦時間同步
                        break;
                    case HT_ATC::ATC_SET_DUAL_TEMP_OK:  //Ifor 20160427 Add ATC 2.0 設定第二點Sensor Function On/Off
                        break;
                    case HT_ATC::ATC_SELFTEST_RESULT_OK:
                    case HT_ATC::ATC_SELFTEST_RESULT_NG:
                        ATC_SYS_PAL[0]->iATCSelfTestResult=aData.iATCSelfTestResult;
                        break;
                    case HT_ATC::ATC_SELFTEST_STATUS:
                        ATC_SYS_PAL[0]->iATCSelfTestStatus=aData.iATCSelfTestStatus;
                        break;
                    case HT_ATC::ATC_LotStart_STATUS:   //Ifor 20161118 add ATC2.0 Lot Start flag
                        ATC_SYS_PAL[0]->bATCLotStart=aData.bATCLotStart;
                        break;
                    case HT_ATC::ATC_LotEnd_STATUS:     //Ifor 20161118 add ATC2.0 Lot End flag
                        ATC_SYS_PAL[0]->bATCLotEnd=aData.bATCLotEnd;
                        break;
                    case HT_ATC::ATC_READ_STATUS:       //Ifor 20160801 add ATC2.0 狀態讀取
                        {
                        TStringList *sList = new TStringList();
                        sList->CommaText=aData.strATCStatus;
                        if(sList->Count ==3)
                        {
                            ATC_SYS_PAL[i]->bRunChillerStatus=atoi(AnsiString(sList->Strings[0]).c_str());
                            ATC_SYS_PAL[i]->bRunATCStatus=atoi(AnsiString(sList->Strings[1]).c_str());
                            ATC_SYS_PAL[i]->bUseRefSensorStatus=atoi(AnsiString(sList->Strings[2]).c_str());
                        }
                        sList->Clear();                                         //Ifor 20170603 (wei) TStringList 刪除前先 Clean
                        delete sList;
                        }
                        break;
                }
            }
            else if(aData.iType==ARD_NONE)
            {
            }
        }
        //處理訊息 : 結束-----------------------------

        if(ATC_SYS.PopHistoryString(i, sLog)==true)
        {
            if(ATC_SYS_PAL[i]->MemoATC->Lines->Count>=1000)                     //Ifor 20160801 200->50
            {
                if(chkSaveLog->Checked)                                         //Steven 20210409 : add for ATC comm log
                {
                    Path.sprintf("D:\\HT9045_Log\\Comm_ATC\\%04d-%d\\", SystemYear, SystemMonth);
                    MyForceDirectories(Path);
                    Path.sprintf("D:\\HT9045_Log\\Comm_ATC\\%04d-%d\\%04d%02d%02d_%02d%02d%02d.csv", SystemYear, SystemMonth, SystemYear, SystemMonth, SystemDate, SystemHour, SystemMin, SystemSec);
                }
                ATC_SYS_PAL[i]->MemoATC->Lines->Clear();
            }
            ATC_SYS_PAL[i]->MemoATC->Lines->Add(sLog.c_str());

            if(sLog=="ERROR: 10060")
            {
                ATC_SYS_PAL[i]->iErrorCount++;
                if(ATC_SYS_PAL[i]->iErrorCount==5)
                {
                    ATCInterfaceForm->CheckHandlerConnectATC();
                    ATC_SYS_PAL[i]->iErrorCount=0;
                }
            }
        }
    }
    // 2011.05.24 , Joye , ATC -----------------
    bool bReConnect=false;
    static bool bChillerSVCheck=true;
    static bool bATCSiteSVCheck[4]={true};
    static bool bATCSiteOnCheck[4]={true};
    static int iRunTime=0;
    static int iRunDelayTime=5;
    static int ReconnectCount=0;
    static bool bFirstConnect=true;
    static int iTask=1;

    if(bFirstConnect==true)
    {
        bATC20AutoConnect=true;
        bFirstConnect=false;
    }

    if(bStartATCRun==true)
    {
        if(ATC_SYS.IsOnLine()==false ||                                         //Ifor 20170613 (wei) add 避免ATC2.0 燈號顯示異常
           ATCInterfaceForm->IsOnLine()==false)
        {
            ATCInterfaceForm->OnLine();
        }
    }
    else
    {
        if(ATC_SYS.IsOnLine()==true)
        {
            ATCInterfaceForm->OffLine();
        }
    }

    if(ATC_SYS.IsOnLine()==true &&                                              //Ifor 20160101 :未下active 命令不送指令給atc pc
       Temperature.bATCActiveCooling==true &&
       bStartATCRun==true)                                                      //Ifor 20170613 add 避免ATC2.0 顯示離線底層卻連線問題
    {
        if(bOnLineSetting)
        {
            for(unsigned int iAtc=0; iAtc<ATC_SYS_PAL.size(); iAtc++)
            {
                if(iOnLineStep==0)
                    ATC_SYS.GetConnectState(iAtc);                              //12.06.27.01
                else if(iOnLineStep==1)
                {
                    ATC_SYS_PAL[iAtc]->fATCSiteSVSetting=atof(ATC_SYS_PAL[iAtc]->pal_SV->Text.c_str());
                    SetWorkTemperature(iAtc, ATC_SYS_PAL[iAtc]->fATCSiteSVSetting);       //12.06.27.01
                }
                else
                {
                    ATC_SYS_PAL[iAtc]->fATCOffsetSetting=atof(ATC_SYS_PAL[iAtc]->edtOffset->Text.c_str());
                    SetOffsetTemperature(iAtc, ATC_SYS_PAL[iAtc]->fATCOffsetSetting);   //pig 2011.12.21 ATC改
                }
            }

            if(iOnLineStep<2)
            {
                iOnLineStep++;
            }
            else
            {
                iOnLineStep=0;
                bOnLineSetting=false;
            }
        }
        else
        {
            if(bATC20AutoConnect==true)                                         //Ifor 20160721 連線改全手動連線
            {
                for(i=0; i<ATC_SYS_PAL.size(); i++)                             //Ifor 20160216 新增連線延遲機制，避免連續連線造成通訊異常
                {
                    if(ATC_SYS.Channels[i]->IsConnect()==false)
                    {
                        bReConnect=true;
                        if(ReconnectCount==0)
                        {
                            ATC_SYS.Channels[i]->Connect();
                            SetWorkTemperature(i, ATC_SYS_PAL[i]->fATCSiteSVSetting);
                        }
                    }
                }
            }
            ReconnectCount++;
            if(ReconnectCount>=50)
                ReconnectCount=0;

            if(bReConnect==false)
            {
                if(CosFunction.bHiSiliconFunction && bATC20AutoConnect==true)   //Ifor 20160912 海思 ATC2.0 連線後改全手動連線
                    bATC20AutoConnect=false;

                iRunTime++;
                if(ATC_SYS_PAL[0]->bATCRunStatus!=ATC_SYS_PAL[0]->bATCRunSetting)
                {
                    if(iRunTime >=iRunDelayTime)                                //Ifor 200ms * 5 = 1000ms 設定ATC啟動
                    {
                        iRunTime=0;
                        if(ATC_SYS_PAL[0]->bATCRunSetting)
                            ATCInterfaceForm->SetRunATC(true);
                        else
                            ATCInterfaceForm->SetRunATC(false);
                    }
                }
                else if(ATC_SYS_PAL[0]->fATCSetChillerSV!=ATC_SYS_PAL[0]->fATCSetChillerValue)
                {
                    if(iRunTime>=iRunDelayTime)                                 //Ifor 200ms * 5 = 1000ms 取得目前Chiller設定溫度 & 設定Chiller 溫度
                    {
                        iRunTime=0;
                        if(bChillerSVCheck)
                            ATC_SYS.GetNowChillerTemperature(0);                //Steven 20120528 : 確認冰水機溫度
                        else
                            ATC_SYS.SetChillerTemperature(ATC_SYS_PAL[0]->fATCSetChillerSV);
                        bChillerSVCheck=!bChillerSVCheck;
                    }
                }
                else
                {
                    for(i=0; i<ATC_SYS_PAL.size(); i++)
                    {
                        if(ATC_SYS_PAL[i]->fATCSiteSVSetting!=ATC_SYS_PAL[i]->fATCSiteSVValue)
                        {
                            if(bATCSiteSVCheck[i])
                                ATC_SYS.GetNowWorkTemperature(i);               //Steven 20120528 : 確認工作溫度
                            else
                                ATC_SYS.SetWorkTemperature(i, ATC_SYS_PAL[i]->fATCSiteSVSetting);
                            ATC_SYS_PAL[i]->bATCTempReady=false;                //Steven 20120530 : 確認溫度是否到達
                            bATCSiteSVCheck[i]=!bATCSiteSVCheck[i];
                        }
                        else if(ATC_SYS_PAL[i]->bSiteOnOffStatus!=ATC_SYS_PAL[i]->bSiteOnOffSetting)
                        {
                            if(bATCSiteOnCheck[i])
                                ATC_SYS.GetNowSiteOnOff(i);
                            else
                                ATC_SYS.SetChannelUse(i, ATC_SYS_PAL[i]->bSiteOnOffSetting);
                            bATCSiteOnCheck[i]=!bATCSiteOnCheck[i];
                        }

                        if(iCount==0)                                           //Ifor 200ms * 5 = 1000ms 讀取一次溫度
                        {
                            if(ATC_SYS_PAL[i]->bSiteOnOffSetting)               //Ifor 20160621 add ATC2.0 有開Site才去讀取ATC溫度
                            {
                                if(ATC_SYS_PAL[i]->LedATCActive->Value==true)
                                {
                                    ATC_SYS.GetNowTemperature(i);               // 2011.05.24 , Joye  //Ifor 20160708 ATC2.0 新增溫度讀取條件
                                    ATC_SYS_PAL[i]->fATCNotReceivedCount++;     //Ifor 20160223 累加未收到溫度回傳次數
                                }
                            }

                            if(bSendATCLotStart==true)                          //Ifor 20161118 add 確認ATC是否有收到ATC Lot Start命令 未收到重送命令
                            {
                                bHasReceive=GetATCLotStartStatus();             //Ifor 20160824 add Get ATC Self Test Result
                                if(bHasReceive==false || bReSendATCLotEven==true)
                                {
                                    bReSendATCLotEven=false;
                                    LotStart(asATCEvenLotID);                   //Ifor 20161118 add Lot ID 傳送至 ATC2.0 System //Ifor 20170125 (Steven) : 修改變數名稱避免Tray Feed 後元件資料被清空 fLotInfo->edtSysLotID->Text =>asATCEvenLotID
                                }
                            }

                            if(bSendATCLotEnd==true)                            //Ifor 20161118 add 確認ATC是否有收到ATC Lot End命令 未收到重送命令
                            {
                                bHasReceive=GetATCLotEndStatus();               //Ifor 20161118 add Get ATC Self Test Result
                                if(bHasReceive==false || bReSendATCLotEven==true)
                                {
                                    bReSendATCLotEven=false;
                                    LotEnd(asATCEvenLotID);                     //Ifor 20161118 add Lot ID 傳送至 ATC2.0 System //Ifor 20170125 (Steven) : 修改變數名稱避免Tray Feed 後元件資料被清空 fLotInfo->edtSysLotID->Text =>asATCEvenLotID
                                }
                            }
                        }

                        if(ATC_SYS_PAL[i]->bSiteOnOffSetting!=bATC_EnablesChannel[i])     //Ifor 20160621 add ATC2.0 開關Site 設定異常
                        {
                            if(bATC_EnablesChannel[i])
                                OpenChannel(i);
                            else
                                CloseChannel(i);
                        }

                        if(iRunTime==iRunDelayTime && ATC_SYS_PAL[i]->bSiteOnOffSetting)    //200ms * 5 = 1000ms 讀取一次目前狀態
                            ATC_SYS.GetNowSiteOnOff(i);                                     //Ifor 20160621 add 讀取 ATC2.0 目前使用的 Site Status
                    }

                    if(iRunTime>=iRunDelayTime*2)                                           //Ifor 200ms * 5 * 2 =2000ms 設定ATC時間同步 & 讀取 ATC 軟體版本
                    {
                        iRunTime=0;
                        switch(iTask)
                        {
                            case 1:                                             //Ifor 20160802 ATC 2.0 讀取ATC軟體版本
                                ATC_SYS.GetNowATCSwVer();
                                iTask=2;
                                break;
                            case 2:                                             //Ifor 20160802 ATC 2.0 設定ATC電腦時間同步
                                ATC_SYS.SetATCTimeSync();
                                iTask=3;
                                break;
                            case 3:                                             //Ifor 20160802 ATC ATC2.0 強制Run ATC
                                ATCInterfaceForm->SetRunATC(true);
//                                ATC_SYS.SetRunATC();
                                bRunATC=true;
                                iTask=4;
                                break;
                            case 4:                                             //Ifor 20160802 Add ATC2.0 強制Run Chiller
                                ATCInterfaceForm->ATCChillerSwitch(true);
                                iTask=5;
                                break;
                            case 5:                                             //Ifor 20160802 Add ATC 2.0 設定第二點Sensor Function On/Off
                                ATC_SYS.Set2ndTempFunction(Temperature.bUseReferTempSensor);
                                iTask=1;
                                break;
                        }
                        ATC_SYS.GetNowSelfTestStatus();                         //Ifor 20160824 add 一秒讀取一次目前ATC Self Test 狀態
                    }
                    iCount++;
                    if(iCount>=5)                                               //Ifor 20160720 10 => 5
                        iCount=0;
                }
            }
        }
    }

    for(i=0; i<ATC_SYS_PAL.size(); i++)
    {
        if(ATC_SYS_PAL[i]->fATCNotReceivedCount>=10)                            //Ifor 20160715 未讀取溫度超過5秒設定溫度為999.9    //Ifor 20160922 修改與7000相同次數 5 -> 10
        {
            AnsiString str;
            ATC_SYS_PAL[i]->fATCNotReceivedCount=0;                             //清除Count 數
            ATC_SYS_PAL[i]->fATCSiteNowTemp=999.9;
            ATC_SYS_PAL[i]->fATCRefSensorTemp=999.9;
        }
    }
    // -----------------------------------------
    RefreshChannelState();
    bTimerRun=false;
}
//---------------------------------------------------------------------------
//pig 2011.12.21 ATC改 start
void TATCInterfaceForm::SetOffsetTemperature(int iChannel, double fTemp)
{
    ATC_SYS.SetOffsetTemperature(iChannel, fTemp);
    ATC_SYS_PAL[iChannel]->fATCOffsetSetting=fTemp;
    ATC_SYS_PAL[iChannel]->fATCOffsetValue=fTemp-1.0;
    ATC_SYS_PAL[iChannel]->edtOffset->Text=AnsiString(fTemp);
}
//---------------------------------------------------------------------------
void TATCInterfaceForm::SetRunATC(bool bRun)
{
    if(ATC_SYSTEM==eNonChamber || ATC_SYSTEM==eATCUninstall)                    //Steven 20180212 (wei) : 修正省電模式沒裝ATC會向賽  != ==> ==
        return;

    ATC_SYS_PAL[0]->bATCRunSetting=bRun;
    ATC_SYS_PAL[0]->bATCRunStatus=!bRun;
    if(bRun==true && bIsWaterLeakage==false)                                    //Steven 20210325 : 漏水檢知
    {
        ATC_SYS.SetRunATC();
        btnATCPower->Caption="STOP ATC";
    }
    else
    {
        btnATCPower->Caption="RUN ATC";
        ATC_SYS.SetStopATC();
    }
}
//---------------------------------------------------------------------------
bool TATCInterfaceForm::ATCRun(bool bRun)
{
    return ATC_SYS.IsATCRun();
}
//---------------------------------------------------------------------------
double TATCInterfaceForm::GetATCSiteNowTemperature(int iChannel)
{
    if(ATC_SYS_PAL[iChannel]->bSiteOnOffSetting)
        return ATC_SYS_PAL[iChannel]->fATCSiteNowTemp;
    else
        return 0.0;                                                             //沒開就回0度
}
//---------------------------------------------------------------------------
double TATCInterfaceForm::GetATCSiteNowTemperature_Ref(int iChannel)            // 2014.10.04 , Joye , KYEC Reffer Temperatrue   //Steven 20150108 : [L11-5] For海思使用兩組感溫
{
    if(ATC_SYS_PAL[iChannel]->bSiteOnOffSetting)
        return ATC_SYS_PAL[iChannel]->fATCRefSensorTemp;
    else
        return 0.0;                                                             //沒開就回0度
}
//---------------------------------------------------------------------------
int TATCInterfaceForm::GetRunSelfTestResult()                                   //Ifor 20160720 Get ATC Self Test result
{
    return ATC_SYS_PAL[0]->iATCSelfTestResult;
}
//---------------------------------------------------------------------------
int TATCInterfaceForm::GetRunSelfTestStatus()                                   //Ifor 20160824 Get ATC Self Test Status
{
    return ATC_SYS_PAL[0]->iATCSelfTestStatus;
}
//---------------------------------------------------------------------------
bool TATCInterfaceForm::GetATCLotStartStatus()                                  //Ifor 20160824 Get ATC2.0 Lot Start Status
{
    return ATC_SYS_PAL[0]->bATCLotStart;
}
//---------------------------------------------------------------------------
bool TATCInterfaceForm::GetATCLotEndStatus()                                    //Ifor 20160824 Get ATC2.0 Lot End Status
{
    return ATC_SYS_PAL[0]->bATCLotEnd;
}
//---------------------------------------------------------------------------
void TATCInterfaceForm::LotStart(AnsiString sLotID)                             //Ifor 20161118 add ATC2.0 Lot Start Command for KYEC
{
    ATC_SYS.LotStart(sLotID);
    ATC_SYS_PAL[0]->bATCLotStart=false;                                         //Ifor 20170124 (Steven) : ATC2.0送出Lot Start 需清除旗標
}
//---------------------------------------------------------------------------
void TATCInterfaceForm::LotEnd(AnsiString sLotID)                               //Ifor 20161118 add ATC2.0 Lot End Command for KYEC
{
    ATC_SYS.LotEnd(sLotID);
    ATC_SYS_PAL[0]->bATCLotEnd=false;                                           //Ifor 20170124 (Steven) : ATC2.0送出Lot End 需清除旗標
}
//---------------------------------------------------------------------------
void TATCInterfaceForm::ATCChillerSwitch(bool bopen)
{
    if(ATC_SYS.IsOnLine()==false)
        return;                                                                 //2013-05-15 Dell 處理chiller running燈號異常

    if(bopen==true && bIsWaterLeakage==false)                                   //Steven 20210325 : 漏水檢知
        ATC_SYS.SetATCChillerSwitchOpen();
    else
        ATC_SYS.SetATCChillerSwitchClose();
}
//---------------------------------------------------------------------------
void TATCInterfaceForm::CheckHandlerConnectATC()
{
    bATCConnectError=true;
    ATC_SYS.SetOnLine(false);
}
//---------------------------------------------------------------------------
//pig 2011.12.21 ATC改 end
void TATCInterfaceForm::ATCSocketError(TObject *Sender,
      TCustomWinSocket *Socket, TErrorEvent ErrorEvent, int &ErrorCode)
{
    try
    {
        ErrorEvent=TErrorEvent(NULL);
        ErrorCode =0;
        Socket->Disconnect(Socket->RemotePort);              //ADAPTED: golden  -- TCustomWinSocket has no ->Close(); vclcompat/ClientSocket.h's own banner cites this EXACT SocketError call shape as the reason Disconnect(RemotePort) exists
        iATCForHSMode=0;                                                        //Ifor 20160912 ATC 通訊異常需清除ATC 版本，避免人員更換版本跑貨
        bReSendATCLotEven=true;                                                 //Ifor 20161121 ATC 斷線需重新送出 ATC Lot Even 給ATC
    }
    catch(...)
    {
        MyDBIProcess("Exception", "ATCSocketError");
    }
}
//---------------------------------------------------------------------------
void TATCInterfaceForm::FormDestroy(TObject *Sender)
{
    try
    {
        int iMax = ATC_SYS.Size();

        for(int i=iMax-1; i>=0; i--)
        {
            ATC_SYS.Disconnect(i);
            delete ((HT_ATC*)ATC_SYS.Channels[i]);                              //Steven 20120623 : ATC解構
        }

        for(vector<TMyHonPrecATCPanel *>::iterator iter=ATC_SYS_PAL.begin(); iter!=ATC_SYS_PAL.end(); ++iter)
        {
            delete *iter;
        }
        ATC_SYS_PAL.clear();                                                    //ADAPTED: golden `vec_clr(ATC_SYS_PAL)` (a golden-only helper macro/template not ported) -- std::vector::clear() is the identical operation vec_clr performs (empties the vector; the elements were already individually deleted above)
    }
    catch(...)
    {
        MyDBIProcess("Exception", "TATCInterfaceForm::FormDestroy");
    }
    //AI(W906-PT-W3-integrate) 20260808: GATE REMOVED -- its premise was FALSE.  The gate
    //  read "grepped tree-wide, no port of LogSoftwareOffTime exists anywhere".  It does:
    //  declared cmydef.h:5032, real (no-op) body acarry_shims.cpp:176, and that file is
    //  registered in CMakeLists.txt (ht9045_sm).  Six already-committed units call it
    //  un-gated (acarry.cpp:7285, Automation/automation.cpp:2621, HThreadCtrl.cpp:156,
    //  language.cpp:137, uHeaterThread.cpp:414, uruncontrol.cpp:231) -- and so does
    //  THIS WAVE's own sibling OmronLaser/LaserSensor.cpp:1852.  Restored to golden
    //  ATCInterface.cpp:958 verbatim.
    LogSoftwareOffTime("TATCInterfaceForm, FormDestroy");                       //Steven 20210526 : 紀錄軟體執行時間
}
//---------------------------------------------------------------------------
// ===========================================================================
//  GOLDEN VERBATIM PAIR -- TMyHonPrecATCPanel::TMyHonPrecATCPanel()
//  GATED : golden ATC/ATCInterface.cpp:961-1274 (314 lines), inert reference text.
//  LIVE  : the TMyHonPrecATCPanel::TMyHonPrecATCPanel() body immediately AFTER the #endif below.  It is UNCHANGED by
//          this wave -- net behaviour change is ZERO.
//  WHY   : the census scored this "translated" because a same-named LIVE body
//          existed, without comparing SIZE.  Golden's 314 lines were NOWHERE in
//          the tree -- lost text, not deferred behaviour.  Now the text EXISTS
//          and is auditable, so a later un-gate is mechanical.
//  NOTE  : the LIVE ctor is a 117-line PARTIAL -- it keeps the data-bearing
//          assignments but drops golden's ~200 lines of widget geometry/Font/
//          Name/Hint layout (this file's own GATE (7)).  The full 314 lines now exist.
//  RULES : nothing inside the gate is fixed, renamed, reflowed or reindented;
//          it is golden's bytes.  Nothing it references had to be made to
//          exist -- no stub, declaration or header was added for it.
//  SHAPE : same pair shape as csystem.cpp MainProc / atester.cpp (PT-W6a/W6b)
//          golden-verbatim gates.
// ===========================================================================
#if 0 // GOLDEN VERBATIM -- golden ATC/ATCInterface.cpp:961-1274 (314 lines).  GATE G-PTW6c-TMyHonPrecATCPanel.  NOT COMPILED: the ACTIVE TMyHonPrecATCPanel::TMyHonPrecATCPanel() is the body immediately after this #endif.
__fastcall TMyHonPrecATCPanel::TMyHonPrecATCPanel(TComponent* Owner, int index) : TComponent(Owner)         //設定元件位置
{
    AnsiString str;
    iMyTag=index;
    bSiteOnOffStatus=false;                                                     //Steven 20120528 : 確認開關Site
    bATCTempReady=false;                                                        //Steven 20120530 : 確認溫度是否到達
    iRecordCount=0;                                                             // 2011.05.24 , Joye , ATC
    iErrorCount=0;                                                              //pig 2011.12.21 ATC改
    fLastTemp=0.0;                                                              // 2011.05.24 , Joye , ATC
    fATCSiteNowTemp=0.0;
    fATCSiteSVValue=0.0;
    fATCRefSensorTemp=0.0;                                                      //Steven 20150108 : [L11-5] For海思使用兩組感溫
    fATCNotReceivedCount=0;                                                     //Ifor 20160223 未收到溫度回傳次數
    iATCSelfTestResult=0;                                                       //Ifor 20160720 Add ATC SELF TEST RESULT 結果
    iATCSelfTestStatus=0;                                                       //Ifor 20160824 Add ATC SELF TEST Status

    sAddress.sprintf("10%02d", index+1);
    iPort=1000+index;
    gbATC           =new TGroupBox(this);
    gbOffset        =new TGroupBox(this);
    edtOffset       =new TEdit(this);
    btSetOffset     =new TButton(this);
    ATCSocket       =new TClientSocket(this);
    LedATCConnect   =new TMyLed(this);
    LedATCActive    =new TMyLed(this);
    labState        =new TLabel(this);
    labAddress      =new TLabel(this);
    labPort         =new TLabel(this);
    edtAddress      =new TEdit(this);
    edtPort         =new TEdit(this);
    edtSendData     =new TEdit(this);
    BitBtnSet       =new TBitBtn(this);
    BitBtnConnect   =new TBitBtn(this);
    BitBtnDisconnect=new TBitBtn(this);
    BitBtnSend      =new TBitBtn(this);
    MemoATC         =new TMemo(this);
    pal_SV          =new TEdit(this);
    pal_PV          =new TPanel(this);
    cbEnableSite    =new TCheckBox(this);

    str.sprintf("gbATC%02d", iMyTag);
    gbATC->Name         =str;
    gbATC->Left         =4;
    gbATC->Top          =8+iMyTag*130;
    gbATC->Width        =956;
    gbATC->Height       =120;
    gbATC->Font->Color  =clBlack;
    gbATC->Font->Name   ="Arial";
    gbATC->Font->Size   =14;
    str.sprintf("Channel%02d", iMyTag+1);
    gbATC->Caption      =str;

    str.sprintf("gbOffset%02d", iMyTag);
    gbOffset->Parent       =gbATC;
    gbOffset->Name         =str;
    gbOffset->Left         =88;
    gbOffset->Top          =44;
    gbOffset->Width        =100;
    gbOffset->Height       =70;
    gbOffset->Font->Color  =clBlack;
    gbOffset->Font->Name   ="Arial";
    gbOffset->Font->Size   =12;
    gbOffset->Caption      ="Offset";

    str.sprintf("edtOffset%02d", iMyTag);
    edtOffset->Parent         =gbOffset;
    edtOffset->Name           =str;
    edtOffset->Left           =10;
    edtOffset->Top            =16;
    edtOffset->Width          =80;
    edtOffset->Height         =26;
    edtOffset->Font->Color    =clBlack;
    edtOffset->Font->Name     ="Arial";
    edtOffset->Font->Size     =12;
    edtOffset->Text           =0.0;
    edtOffset->Tag            =iMyTag;

    str.sprintf("btSetOffset%02d", iMyTag);
    btSetOffset->Parent       =gbOffset;
    btSetOffset->Name         =str;
    btSetOffset->Left         =10;
    btSetOffset->Top          =44;
    btSetOffset->Width        =80;
    btSetOffset->Height       =20;
    btSetOffset->Font->Color  =clBlack;
    btSetOffset->Font->Name   ="Arial";
    btSetOffset->Font->Size   =12;
    btSetOffset->Caption      ="Set";
    btSetOffset->Tag          =iMyTag;

    str.sprintf("ATCSoclet%02d", iMyTag);
    ATCSocket->Name=str;
    ATCSocket->Port=1000;
    ATCSocket->ClientType=ctNonBlocking;
    ATCSocket->Active=false;
    ATCSocket->Tag=iMyTag;

    str.sprintf("LedATCConnect%02d", iMyTag);
    LedATCConnect->Parent      =gbATC;
    LedATCConnect->Name        =str;
    LedATCConnect->Left        =19;
    LedATCConnect->Top         =21;
    LedATCConnect->Width       =23;
    LedATCConnect->Height      =23;
    LedATCConnect->TrueColor   =clLime;
    LedATCConnect->FalseColor  =clSilver;
    LedATCConnect->LEDStyle    =LEDSqLarge;
    LedATCConnect->Tag         =iMyTag;
    LedATCConnect->Hint        ="ATC Connection";
    LedATCConnect->ShowHint    =true;

    str.sprintf("LedATCActive%02d", iMyTag);
    LedATCActive->Parent      =gbATC;
    LedATCActive->Name        =str;
    LedATCActive->Left        =55;
    LedATCActive->Top         =21;
    LedATCActive->Width       =23;
    LedATCActive->Height      =23;
    LedATCActive->TrueColor   =clLime;
    LedATCActive->FalseColor  =clSilver;
    LedATCActive->LEDStyle    =LEDSqLarge;
    LedATCActive->Tag         =iMyTag;
    str.sprintf("ATC %02d Active", iMyTag);
    LedATCActive->Hint        =str;
    LedATCActive->ShowHint    =true;

    str.sprintf("labState%02d", iMyTag);
    labState->Parent        =gbATC;
    labState->Name          =str;
    labState->Left          =10;
    labState->Top           =46;
    labState->Width         =73;
    labState->Height        =16;
    labState->Font->Color   =clBlack;
    labState->Font->Name    ="Arial";
    labState->Font->Size    =10;
    labState->Caption       ="Off-Line";
    labState->Tag           =iMyTag;
    labState->Alignment     =taCenter;

    str.sprintf("labAddress%02d", iMyTag);
    labAddress->Parent      =gbATC;
    labAddress->Name        =str;
    labAddress->Left        =197;
    labAddress->Top         =30;
    labAddress->Width       =59;
    labAddress->Height      =18;
    labAddress->Font->Color =clBlack;
    labAddress->Font->Name  ="Arial";
    labAddress->Font->Size  =12;
    labAddress->Caption     ="Address";
    labAddress->Tag         =iMyTag;

    str.sprintf("labPort%02d", iMyTag);
    labPort->Parent         =gbATC;
    labPort->Name           =str;
    labPort->Left           =331;
    labPort->Top            =30;
    labPort->Width          =29;
    labPort->Height         =18;
    labPort->Font->Color    =clBlack;
    labPort->Font->Name     ="Arial";
    labPort->Font->Size     =12;
    labPort->Caption        ="Port";
    labPort->Tag            =iMyTag;

    str.sprintf("edtAddress%02d", iMyTag);
    edtAddress->Parent      =gbATC;
    edtAddress->Name        =str;
    edtAddress->Left        =195;
    edtAddress->Top         =54;
    edtAddress->Width       =130;
    edtAddress->Height      =26;
    edtAddress->Font->Color =clBlack;
    edtAddress->Font->Name  ="Arial";
    edtAddress->Font->Size  =12;
    str.sprintf("127.0.0.%d", iMyTag+1);
    edtAddress->Text        =str;
    edtAddress->Tag          =iMyTag;

    str.sprintf("edtPort%02d", iMyTag);
    edtPort->Parent         =gbATC;
    edtPort->Name           =str;
    edtPort->Left           =330;
    edtPort->Top            =54;
    edtPort->Width          =60;
    edtPort->Height         =26;
    edtPort->Font->Color    =clBlack;
    edtPort->Font->Name     ="Arial";
    edtPort->Font->Size     =12;
    str.sprintf("10%02d", iMyTag);
    edtPort->Text           =str;
    edtPort->Tag            =iMyTag;

    str.sprintf("edtSendData%02d", iMyTag);
    edtSendData->Parent     =gbATC;
    edtSendData->Name       =str;
    edtSendData->Left       =194;
    edtSendData->Top        =89;
    edtSendData->Width      =251;
    edtSendData->Height     =24;
    edtSendData->Font->Color=clBlack;
    edtSendData->Font->Name ="Arial";
    edtSendData->Font->Size =10;
    edtSendData->Text       ="";
    edtSendData->Tag        =iMyTag;

    str.sprintf("BitBtnSet%02d", iMyTag);
    BitBtnSet->Parent       =gbATC;
    BitBtnSet->Name         =str;
    BitBtnSet->Left         =396;
    BitBtnSet->Top          =54;
    BitBtnSet->Width        =49;
    BitBtnSet->Height       =30;
    BitBtnSet->Caption      ="SET";
    BitBtnSet->Font->Color  =clNavy;
    BitBtnSet->Font->Name   ="Arial";
    BitBtnSet->Font->Size   =12;
    BitBtnSet->Tag          =iMyTag;

    str.sprintf("BitBtnConnect%02d", iMyTag);
    BitBtnConnect->Parent       =gbATC;
    BitBtnConnect->Name         =str;
    BitBtnConnect->Left         =450;
    BitBtnConnect->Top          =16;
    BitBtnConnect->Width        =120;
    BitBtnConnect->Height       =30;
    BitBtnConnect->Caption      ="Connect";
    BitBtnConnect->Font->Color  =clGreen;
    BitBtnConnect->Font->Name   ="Arial";
    BitBtnConnect->Font->Size   =12;
    BitBtnConnect->Tag          =iMyTag;

    str.sprintf("BitBtnDisconnect%02d", iMyTag);
    BitBtnDisconnect->Parent       =gbATC;
    BitBtnDisconnect->Name         =str;
    BitBtnDisconnect->Left         =450;
    BitBtnDisconnect->Top          =51;
    BitBtnDisconnect->Width        =120;
    BitBtnDisconnect->Height       =30;
    BitBtnDisconnect->Caption      ="Disconnect";
    BitBtnDisconnect->Font->Color  =TColor(0x00404080);
    BitBtnDisconnect->Font->Name   ="Arial";
    BitBtnDisconnect->Font->Size   =12;
    BitBtnDisconnect->Tag          =iMyTag;

    str.sprintf("BitBtnSend%02d", iMyTag);
    BitBtnSend->Parent       =gbATC;
    BitBtnSend->Name         =str;
    BitBtnSend->Left         =450;
    BitBtnSend->Top          =86;
    BitBtnSend->Width        =120;
    BitBtnSend->Height       =30;
    BitBtnSend->Caption      ="Send";
    BitBtnSend->Font->Color  =clNavy;
    BitBtnSend->Font->Name   ="Arial";
    BitBtnSend->Font->Size   =12;
    BitBtnSend->Tag          =iMyTag;

    str.sprintf("MemoATC%02d", iMyTag);
    MemoATC->Parent       =gbATC;
    MemoATC->Name         =str;
    MemoATC->Color        =TColor(0x00DFD9CC);
    MemoATC->Left         =576;
    MemoATC->Top          =16;
    MemoATC->Width        =374;
    MemoATC->Height       =97;
    MemoATC->Font->Color  =clBlack;
    MemoATC->Font->Name   ="Arial";
    MemoATC->Font->Size   =10;
    MemoATC->Tag          =iMyTag;
    MemoATC->ScrollBars   =ssVertical;

    str.sprintf("pal_SV%02d", iMyTag);
    pal_SV->Parent       =gbATC;
    pal_SV->Name         =str;
    pal_SV->Color        =clWhite;
    pal_SV->Left         =11;
    pal_SV->Top          =63;
    pal_SV->Width        =70;
    pal_SV->Height       =25;
    pal_SV->Font->Color  =clBlack;
    pal_SV->Font->Name   ="Arial";
    pal_SV->Font->Size   =12;
    pal_SV->Text         ="000.0℃";
    pal_SV->Tag          =iMyTag;

    str.sprintf("pal_PV%02d", iMyTag);
    pal_PV->Parent       =gbATC;
    pal_PV->Name         =str;
    pal_PV->Color        =TColor(0x00DFD9CC);
    pal_PV->Left         =12;
    pal_PV->Top          =89;
    pal_PV->Width        =70;
    pal_PV->Height       =25;
    pal_PV->Font->Color  =clBlack;
    pal_PV->Font->Name   ="Arial";
    pal_PV->Font->Size   =12;
    pal_PV->Caption      ="000.0℃";
    pal_PV->Tag          =iMyTag;

    str.sprintf("cbEnableSite%02d", iMyTag);
    cbEnableSite->Parent       =gbATC;
    cbEnableSite->Name         =str;
    cbEnableSite->Left         =88;
    cbEnableSite->Top          =24;
    cbEnableSite->Width        =109;
    cbEnableSite->Height       =17;
    cbEnableSite->Font->Color  =clBlack;
    cbEnableSite->Font->Name   ="Arial";
    cbEnableSite->Font->Size   =12;
    cbEnableSite->Caption      ="Enable Site";
    cbEnableSite->Tag          =iMyTag;
}
#endif // GOLDEN VERBATIM -- golden ATC/ATCInterface.cpp:961-1274  (GATE G-PTW6c-TMyHonPrecATCPanel, end)
TMyHonPrecATCPanel::TMyHonPrecATCPanel(TComponent* Owner, int index) : TComponent(Owner)         //設定元件位置
{
    AnsiString str;
    iMyTag=index;
    bSiteOnOffStatus=false;                                                     //Steven 20120528 : 確認開關Site
    bATCTempReady=false;                                                        //Steven 20120530 : 確認溫度是否到達
    iRecordCount=0;                                                             // 2011.05.24 , Joye , ATC
    iErrorCount=0;                                                              //pig 2011.12.21 ATC改
    fLastTemp=0.0;                                                              // 2011.05.24 , Joye , ATC
    fATCSiteNowTemp=0.0;
    fATCSiteSVValue=0.0;
    fATCRefSensorTemp=0.0;                                                      //Steven 20150108 : [L11-5] For海思使用兩組感溫
    fATCNotReceivedCount=0;                                                     //Ifor 20160223 未收到溫度回傳次數
    iATCSelfTestResult=0;                                                       //Ifor 20160720 Add ATC SELF TEST RESULT 結果
    iATCSelfTestStatus=0;                                                       //Ifor 20160824 Add ATC SELF TEST Status

    sAddress.sprintf("10%02d", index+1);
    iPort=1000+index;

    // GATE (7): golden allocates + lays out (Left/Top/Width/Height/Font/Name/
    // Hint/ShowHint) all 20 widgets in one block (golden :979-1273). None of
    // that geometry/Font/Name/Hint property set exists on vclcompat's
    // Controls.h stand-ins by design (see this file's own banner GATE (7)).
    // ACTIVE below: the allocation (needed so no ACTIVE business-logic body
    // dereferences a null pointer) plus every default VALUE a translated
    // body reads back afterwards (Text/Caption/Tag).
    gbATC           =new TGroupBox();
    gbOffset        =new TGroupBox();
    edtOffset       =new TEdit();
    btSetOffset     =new TATCButton(); // GATE(3): golden TButton*, Tag genuinely read (btSetOffsetClick)
    ATCSocket       =new TClientSocket(this);
    LedATCConnect   =new TMyLed();
    LedATCActive    =new TMyLed();
    labState        =new TLabel();
    labAddress      =new TLabel();
    labPort         =new TLabel();
    edtAddress      =new TEdit();
    edtPort         =new TEdit();
    edtSendData     =new TEdit();
    BitBtnSet       =new TATCBitBtn(); // GATE(3)
    BitBtnConnect   =new TATCBitBtn(); // GATE(3)
    BitBtnDisconnect=new TATCBitBtn(); // GATE(3)
    BitBtnSend      =new TATCBitBtn(); // GATE(3)
    MemoATC         =new TMemo();
    pal_SV          =new TEdit();
    pal_PV          =new TPanel();
    cbEnableSite    =new TATCCheckBox(); // GATE(3): golden TCheckBox*, Tag genuinely read (cbEnableSiteClick)

    gbATC->Caption      ="Channel" + AnsiString().sprintf("%02d", iMyTag+1);
    gbOffset->Caption   ="Offset";

    edtOffset->Text           =0.0;
    // GATE(7): golden edtOffset->Tag=iMyTag -- TEdit carries no ->Tag (vclcompat design boundary) and it is never read back (verified); dropped.
    btSetOffset->Caption      ="Set";
    btSetOffset->Tag          =iMyTag;

    ATCSocket->Port=1000;
    // GATE (7): ->ClientType=ctNonBlocking / ->Name -- no port of ClientType
    // (BCB6 TClientSocket blocking-mode enum) or TComponent::Name string
    // property anywhere in this tree; the REAL Send/Read behaviour these
    // sockets need comes from TCPClient (ATC/TCPData.cpp, already ported),
    // which wraps this pointer and drives it independently of both.
    ATCSocket->Active=false;
    ATCSocket->Tag=iMyTag;

    LedATCConnect->TrueColor   =clLime;
    LedATCConnect->FalseColor  =clSilver;
    LedATCConnect->LEDStyle    =LEDSqLarge;
    LedATCConnect->Tag         =iMyTag;
    LedATCConnect->Hint        ="ATC Connection";
    LedATCConnect->ShowHint    =true;

    LedATCActive->TrueColor   =clLime;
    LedATCActive->FalseColor  =clSilver;
    LedATCActive->LEDStyle    =LEDSqLarge;
    LedATCActive->Tag         =iMyTag;
    LedATCActive->Hint        =AnsiString().sprintf("ATC %02d Active", iMyTag);
    LedATCActive->ShowHint    =true;

    labState->Caption       ="Off-Line";
    // GATE(7): golden labState->Tag=iMyTag -- TLabel carries no ->Tag; never read back (verified); dropped.

    labAddress->Caption     ="Address";
    // GATE(7): golden labAddress->Tag=iMyTag -- TLabel carries no ->Tag; never read back; dropped.

    labPort->Caption        ="Port";
    // GATE(7): golden labPort->Tag=iMyTag -- TLabel carries no ->Tag; never read back; dropped.

    str.sprintf("127.0.0.%d", iMyTag+1);
    edtAddress->Text        =str;
    // GATE(7): golden edtAddress->Tag=iMyTag -- TEdit carries no ->Tag; never read back; dropped.

    str.sprintf("10%02d", iMyTag);
    edtPort->Text           =str;
    // GATE(7): golden edtPort->Tag=iMyTag -- TEdit carries no ->Tag; never read back; dropped.

    edtSendData->Text       ="";
    // GATE(7): golden edtSendData->Tag=iMyTag -- TEdit carries no ->Tag; never read back; dropped.

    BitBtnSet->Caption      ="SET";
    BitBtnSet->Tag          =iMyTag;

    BitBtnConnect->Caption      ="Connect";
    BitBtnConnect->Tag          =iMyTag;

    BitBtnDisconnect->Caption      ="Disconnect";
    BitBtnDisconnect->Tag          =iMyTag;

    BitBtnSend->Caption      ="Send";
    BitBtnSend->Tag          =iMyTag;

    pal_SV->Text         ="000.0℃";
    pal_PV->Caption      ="000.0℃";

    cbEnableSite->Caption      ="Enable Site";
    cbEnableSite->Tag          =iMyTag;
}
//---------------------------------------------------------------------------
/*
void TMyHonPrecATCPanel::SetDisplay(int iLevel)
{
    edtAddress->Enabled      =(iLevel>=3);
    edtPort->Enabled         =(iLevel>=3);
    BitBtnSet->Visible       =(iLevel>=3);
    edtSendData->Visible     =(iLevel>=3);
    BitBtnConnect->Enabled   =(iLevel>=3);
    BitBtnDisconnect->Enabled=(iLevel>=3);
    BitBtnDisconnect->Enabled=(iLevel>=3);
}*/
//---------------------------------------------------------------------------
TMyHonPrecATCPanel::~TMyHonPrecATCPanel()
{
//      2013-04-01    Dell delete順序不對會造成記憶體溢位,而且new 出來的VCL元件在form Destory 時BCB自己會處理
//    delete gbATC;
//    delete gbOffset;
//    delete edtOffset;
//    delete btSetOffset;
//    delete ATCSocket;
//    delete LedATCConnect;
//    delete LedATCActive;
//    delete labState;
//    delete labAddress;
//    delete labPort;
//    delete edtAddress;
//    delete edtPort;
//    delete edtSendData;
//    delete BitBtnSet;
//    delete BitBtnConnect;
//    delete BitBtnDisconnect;
//    delete BitBtnSend;
//    delete MemoATC;
//    delete pal_SV;
//    delete pal_PV;
//    delete cbEnableSite;
}
//---------------------------------------------------------------------------
void TATCInterfaceForm::btnATCPowerClick(TObject *Sender)
{
    if(btnATCPower->Caption=="RUN ATC")
    {
        ATCInterfaceForm->SetRunATC(true);
        bRunATC=true;                                                           //ChungHung 20160118 add for Hisi V102
    }
    else
    {
        ATCInterfaceForm->SetRunATC(false);
        bRunATC=false;                                                          //ChungHung 20160118 add for Hisi V102
    }
}
//---------------------------------------------------------------------------
void TATCInterfaceForm::btnATCChillerSwitchRunClick(
      TObject *Sender)
{
    ATCInterfaceForm->ATCChillerSwitch(true);
}
//---------------------------------------------------------------------------
void TATCInterfaceForm::btnATCChillerSwitchStopClick(
      TObject *Sender)
{
    if(bSystemStart)                                                            // 2012.05.07 , Joye , Chiller
        return;

    #ifdef HANDLER_CONTROL_ATC
        #ifdef HT9045
            ShowMyMessage("Will close this ATC System.", "");                   // 2012.05.07 , Joye , Chiller
        #else
            ShowMyMessage("Will close this ATC System.");
        #endif
    #endif

    ATCInterfaceForm->SetRunATC(false);                                         // 2012.03.30 , Pig , ATC
    btnATCPower->Caption="RUN ATC";                                             // 2012.03.30 , Pig , ATC
    ATCInterfaceForm->ATCChillerSwitch(false);                                  //12.06.27.01
    iStopATCChillerType=1;
    bRunATC=false;                                                              //ChungHung 20160118 add for Hisi V102
}
//---------------------------------------------------------------------------
void TATCInterfaceForm::btOnLineClick(TObject *Sender)
{
    OnLine();
    Button1->Caption="Stop Timer";
    bATC20AutoConnect=true;
}
//---------------------------------------------------------------------------
void TATCInterfaceForm::btOffLineClick(TObject *Sender)
{
    OffLine();
    Button1->Caption="Start Timer";
}
//---------------------------------------------------------------------------
void TATCInterfaceForm::cbEnableSiteClick(TObject *Sender)
{
    TATCCheckBox *Ptr=(TATCCheckBox*)Sender;                                    //GATE (3): TCheckBox* -> TATCCheckBox* (Tag)
    if(Ptr->Checked)
    {
        OpenChannel(Ptr->Tag);
    }
    else
    {
        CloseChannel(Ptr->Tag);
    }
}
//---------------------------------------------------------------------------
void TATCInterfaceForm::btSetChillerTempClick(TObject *Sender)
{
    ATC_SYS_PAL[0]->fATCSetChillerSV=atof(pl_ATCSetTempChiller->Text.c_str());
    SetChillerTemperature(ATC_SYS_PAL[0]->fATCSetChillerSV);
}
//---------------------------------------------------------------------------
void TATCInterfaceForm::btSetOffsetClick(TObject *Sender)
{
    TATCButton *Ptr=(TATCButton*)Sender;                                        //GATE (3): TButton* -> TATCButton* (Tag)
    SetOffsetTemperature(Ptr->Tag, atof(ATC_SYS_PAL[Ptr->Tag]->edtOffset->Text.c_str()));
    ATC_SYS_PAL[Ptr->Tag]->fATCSiteSVSetting=atof(ATC_SYS_PAL[Ptr->Tag]->pal_SV->Text.c_str());
    SetWorkTemperature(Ptr->Tag, ATC_SYS_PAL[Ptr->Tag]->fATCSiteSVSetting);
}
//---------------------------------------------------------------------------
void TATCInterfaceForm::btSaveClick(TObject *Sender)
{
    for(int i=0; i<iATCUseCount; i++)
        WriteATCSystem(i);
}
//---------------------------------------------------------------------------
void TATCInterfaceForm::Button1Click(TObject *Sender)
{
    ATCWatchTimer->Enabled=!ATCWatchTimer->Enabled;
    if(Button1->Caption=="Start Timer")
        Button1->Caption="Stop Timer";
    else
        Button1->Caption="Start Timer";
}
//---------------------------------------------------------------------------
void TATCInterfaceForm::btSetWorkTempClick(TObject *Sender)
{
    for(unsigned int i=0; i<ATC_SYS_PAL.size(); i++)
    {
        ATC_SYS_PAL[i]->fATCOffsetSetting=atof(ATC_SYS_PAL[i]->edtOffset->Text.c_str());
        SetOffsetTemperature(i, ATC_SYS_PAL[i]->fATCOffsetSetting);
        ATC_SYS_PAL[i]->fATCSiteSVSetting=atof(ATC_SYS_PAL[i]->pal_SV->Text.c_str());
        SetWorkTemperature(i, ATC_SYS_PAL[i]->fATCSiteSVSetting);
    }
}
//---------------------------------------------------------------------------
void TATCInterfaceForm::TimerChillerStopTimer(TObject *Sender)
{
    static clock_t ctStartTime=clock();
    clock_t ctEndTime=clock();

    if(InitialOK==false)                                                        //Steven 20160912 : Add InitialOK in Timer
        return;

    if(iStopATCChillerType==1)                                                  //等溫度降下來就關冰水機
    {
        bool bWait = false;
        for(unsigned int i=0; i<ATC_SYS_PAL.size(); i++)
        {
            double fATCNowTemp=GetATCSiteNowTemperature(i);
            if(fATCNowTemp>=30.0)
            {
                bWait=true;
            }
        }

        if(bWait==false)
        {
            //ATCChillerSwitch(false);                                          //Ifor 20160718 小朱說 ATC2.0 不要關Chiller
            iStopATCChillerType=0;
        }
    }
    else if(iStopATCChillerType==2)                                             //等設定時間到達就關冰水機
    {
        bool bWait=false;
        for(unsigned int i=0; i<ATC_SYS_PAL.size(); i++)
        {
            double fATCNowTemp=GetATCSiteNowTemperature(i);
            if(fATCNowTemp>=30.0)
            {
                bWait=true;
            }
        }

        if(bWait==false)
        {
            //ATCChillerSwitch(false);                                          //Ifor 20160718 小朱說 ATC2.0 不要關Chiller
        }
        iStopATCChillerType=0;
    }
    else if(bL11_2ATCChillerProtectedFunction==true && iATCChillerCheckTime!=0)
    {
        if(ctEndTime-ctStartTime>iATCChillerCheckTime*60*1000)
        {
            ctStartTime=clock();

            if(ATCRun(true)==false && ATC_SYS.IsChillerRun()==true)
            {
                iStopATCChillerType = 2;
            }
        }
    }
}
//---------------------------------------------------------------------------
void TATCInterfaceForm::Button2Click(TObject *Sender)
{
    ATC_SYS.SetNowArm(0);
}
//---------------------------------------------------------------------------
void TATCInterfaceForm::ATC7_ServerSocketClientConnect(
      TObject *Sender, TCustomWinSocket *Socket)
{
    LoadATCSystem();
    LedATC7Connect->Value = true;
    // GATE (5): golden `fLotInfo->aldATCPower->Value = true;` -- forms/
    // fLotInfo.h has no aldATCPower member (grepped directly).
    if(Temperature.bATC70Active==true)
    {
        if(LastSet.iTemperature==Tempture_Hot)                                  //Steven 20151111 : For ATC7.0 修改判斷式
        {
            ATCInterfaceForm->SendCommToATC7(ATC_SET_TEMP, AnsiString(Temperature.fWorkTemperBase), "");
            ATCInterfaceForm->SendCommToATC7(ATC_RUN, "", "");
        }
        else
        {
            ATCInterfaceForm->SendCommToATC7(ATC_STOP, "", "");
        }
    }
}
//---------------------------------------------------------------------------
void TATCInterfaceForm::ATC7_ServerSocketClientDisconnect(
      TObject *Sender, TCustomWinSocket *Socket)
{
    LedATC7Connect->Value = false;
    LedATC7Active->Value = false;
    // GATE (5): golden `fLotInfo->aldATCPower->Value = false;` /
    // `fLotInfo->aldATC7Status->Value = false;` -- see ClientConnect's note.
}
//---------------------------------------------------------------------------
void TATCInterfaceForm::ATC7_ServerSocketClientRead(
      TObject *Sender, TCustomWinSocket *Socket)
{
    memset(pucInBuff, 0, 8191);
    int  iReceiveLength;
    int  iSPos=0, iEPos=0;
    AnsiString asData="", asTemp="";

    try
    {
        iReceiveLength = Socket->ReceiveLength();
        Socket->ReceiveBuf(pucInBuff, iReceiveLength);
        asData = pucInBuff;

        if( HandlerMemo->Lines->Count>30)
        {
            HandlerMemo->Lines->Delete(0);
        }
        HandlerMemo->Lines->Add(FormatDateTime("yyyy/m/d hh:nn:ss", Now())+" [Receive]=>"+asData);

        if(asData.Pos("@RUNOK+")>=1)
        {
            LedATC7Active->Value = true;
            // GATE (5): golden `fLotInfo->aldATC7Status->Value = true;`
        }

        if(asData.Pos("@RUNNG+")>=1)
        {
            LedATC7Active->Value = false;
        }

        if(asData.Pos("@STOPOK+")>=1)
        {
            LedATC7Active->Value = false;
            // GATE (5): golden `fLotInfo->aldATC7Status->Value = false;`
        }

        if(asData.Pos("@SET_TEMPOK")==1)
        {
        }

        if(asData.Pos("@USE_TSDOK")==1)
        {
        }

        if(asData.Pos("@USE_TSDNG")==1)
        {
        }

        if(asData.Pos("@CH_ENABLED")==1)
        {
        }

        if(asData.Pos("@TEMP_READY")==1)                                        //Steven 20160604 : by site TSD
        {
        }

        if(asData.Pos("@TEMP_VALUE")==1)
        {
            iSPos=asData.Pos("=");
            iEPos=asData.Pos("+");
            iATC70_Count=0;                                                     //Ifor 20160223 清除ATC.70 計數
            asData = asData.SubString(iSPos+1, iEPos-iSPos-1).Trim();
            if(asData.Pos("CH1:")!=0)
            {
                iSPos=asData.Pos("CH1:");
                iEPos=asData.Pos(",");
                if(asData.AnsiPos("Tc")==0)                                     //Steven 20151015 : For ATC 7.0
                    asTemp = asData.SubString(iSPos+4, iEPos-iSPos-4).Trim();
                else
                    asTemp = asData.SubString(iSPos+7, iEPos-iSPos-4).Trim();
                asData = asData.SubString(iEPos+1, asData.Length()-iEPos).Trim();
                Panel_CH1_Temp->Caption = asTemp;
                fATC7NowTemp[0] = atof(asTemp.c_str());
                iEPos=asData.Pos(",");
                if(asData.AnsiPos("Tj")==0)                                     //Steven 20151015 : For ATC 7.0
                    asTemp = asData.SubString(1, iEPos-1).Trim();
                else
                    asTemp = asData.SubString(4, iEPos-1).Trim();
                asData = asData.SubString(iEPos+1, asData.Length()-iEPos).Trim();
                Panel_CH1_TSDTemp->Caption = asTemp;
                fATC7NowTSDTemp[0] = atof(asTemp.c_str());
            }

            if(asData.Pos("CH2:")!=0)
            {
                iSPos=asData.Pos("CH2:");
                iEPos=asData.Pos(",");
                if(asData.AnsiPos("Tc")==0)                                     //Steven 20151015 : For ATC 7.0
                    asTemp = asData.SubString(iSPos+4, iEPos-iSPos-4).Trim();
                else
                    asTemp = asData.SubString(iSPos+7, iEPos-iSPos-4).Trim();
                asData = asData.SubString(iEPos+1, asData.Length()-iEPos).Trim();
                Panel_CH2_Temp->Caption = asTemp;
                fATC7NowTemp[1] = atof(asTemp.c_str());
                iEPos=asData.Pos(",");
                if(asData.AnsiPos("Tj")==0)                                     //Steven 20151015 : For ATC 7.0
                    asTemp = asData.SubString(1, iEPos-1).Trim();
                else
                    asTemp = asData.SubString(4, iEPos-1).Trim();
                asData = asData.SubString(iEPos+1, asData.Length()-iEPos).Trim();
                Panel_CH2_TSDTemp->Caption = asTemp;
                fATC7NowTSDTemp[1] = atof(asTemp.c_str());
            }

            if(asData.Pos("CH3:")!=0)
            {
                iSPos=asData.Pos("CH3:");
                iEPos=asData.Pos(",");
                if(asData.AnsiPos("Tc")==0)                                     //Steven 20151015 : For ATC 7.0
                    asTemp = asData.SubString(iSPos+4, iEPos-iSPos-4).Trim();
                else
                    asTemp = asData.SubString(iSPos+7, iEPos-iSPos-4).Trim();
                asData = asData.SubString(iEPos+1, asData.Length()-iEPos).Trim();
                Panel_CH3_Temp->Caption = asTemp;
                fATC7NowTemp[2] = atof(asTemp.c_str());
                iEPos=asData.Pos(",");
                if(asData.AnsiPos("Tj")==0)                                     //Steven 20151015 : For ATC 7.0
                    asTemp = asData.SubString(1, iEPos-1).Trim();
                else
                    asTemp = asData.SubString(4, iEPos-1).Trim();
                asData = asData.SubString(iEPos+1, asData.Length()-iEPos).Trim();
                Panel_CH3_TSDTemp->Caption = asTemp;
                fATC7NowTSDTemp[2] = atof(asTemp.c_str());
            }

            if(asData.Pos("CH4:")!=0)
            {
                iSPos=asData.Pos("CH4:");
                iEPos=asData.Pos(",");
                if(asData.AnsiPos("Tc")==0)                                     //Steven 20151015 : For ATC 7.0
                    asTemp = asData.SubString(iSPos+4, iEPos-iSPos-4).Trim();
                else
                    asTemp = asData.SubString(iSPos+7, iEPos-iSPos-4).Trim();
                asData = asData.SubString(iEPos+1, asData.Length()-iEPos).Trim();
                Panel_CH4_Temp->Caption = asTemp;
                fATC7NowTemp[3] = atof(asTemp.c_str());
                iEPos=asData.Pos(",");
                if(asData.AnsiPos("Tj")==0)                                     //Steven 20151015 : For ATC 7.0
                    asTemp = asData.SubString(1, iEPos-1).Trim();
                else
                    asTemp = asData.SubString(4, iEPos-1).Trim();
                asData = asData.SubString(iEPos+1, asData.Length()-iEPos).Trim();
                Panel_CH4_TSDTemp->Caption = asTemp;
                fATC7NowTSDTemp[3] = atof(asTemp.c_str());
            }
        }

        if(asData.Pos("@MESSAGE")==1)
        {
            iSPos=asData.Pos("=");
            iEPos=asData.Pos("+");
            asAlarmMessage = asData.SubString(iSPos+1,iEPos-iSPos-1);
            //bShowAlarm=true;
            ShowMyMessage(asAlarmMessage);
            SendCommToATC7(ATC_MESSAGE_CLOSE,"","");
        }
    }
    catch(...)
    {
        MyDBIProcess("Exception", "ATC7_ServerSocketClientRead");
    }
}
//---------------------------------------------------------------------------
void TATCInterfaceForm::SendCommToATC7(int iCommIndex, AnsiString sData1, AnsiString sData2)
{
    AnsiString asCommandString;

    switch(iCommIndex)
    {
        case 0://ATC_RUN:
            asCommandString = "@RUN+";
            break;
        case 1://ATC_STOP:
            asCommandString = "@STOP+";
            break;
        case 2://ATC_SET_TEMP:
            asCommandString.sprintf("@SET_TEMP=%s+",sData1);
            break;
        case 3://ATC_USE_TSD:
            asCommandString.sprintf("@USE_TSD=%s+",sData1);
            break;
        case 4://ATC_CH_ENABLED:
            asCommandString.sprintf("@CH_ENABLED=%s,%s+",sData1,sData2);
            break;
        case 7://ATC_MESSAGE_CLOSE
            asCommandString.sprintf("@MESSAGE_CLOSE+");
            break;
        case 8://ATC_EMG_UP     //Eliot 2015_0122
            asCommandString.sprintf("@EMG_UP+");
            break;
        case 9://ATC_EMG_DOWN   //Eliot 2015_0122
            asCommandString.sprintf("@EMG_DOWN+");
            break;
        case 10: //ATC_SOT      //Steven 20151112 : for ATC 7.0
            asCommandString.sprintf("@HANDLER_TEST_START=%04d%02d%02d_%02d%02d%02d+", SystemYear, SystemMonth, SystemDate, SystemHour, SystemMin, SystemSec);
            break;
        case 11: //ATC_EOT      //Steven 20151112 : for ATC 7.0
            asCommandString.sprintf("@HANDLER_TEST_END=%04d%02d%02d_%02d%02d%02d+", SystemYear, SystemMonth, SystemDate, SystemHour, SystemMin, SystemSec);
            break;
        case 12: //ATC_OFFSET   //Steven 20151112 : 修改ATC7.0 Offset
            asCommandString.sprintf("@ATC_OFFSET=%s+", sData1);
            break;
    }
    //CommPtr[0]->WriteCommData(asCommandString.c_str(),strlen(asCommandString.c_str()));

    if(ATC7_ServerSocket->Socket->ActiveConnections==1)
    {
        ATC7_ServerSocket->Socket->Connections[0]->SendText(asCommandString);
        if( HandlerMemo->Lines->Count>=30)
        {
            HandlerMemo->Lines->Clear();
        }
        HandlerMemo->Lines->Add(FormatDateTime("yyyy/m/d hh:nn:ss", Now())+" [Send]=>"+asCommandString);
    }
    else
    {
        asCommandString = "No Client!";
        HandlerMemo->Lines->Add(FormatDateTime("yyyy/m/d hh:nn:ss", Now())+" [Send]=>"+asCommandString);
    }
}
//---------------------------------------------------------------------------
void TATCInterfaceForm::BitBtn1Click(TObject *Sender)
{
    double dTemp = atof(edATC7_Temp->Text.c_str());
    SendCommToATC7(ATC_SET_TEMP, AnsiString(dTemp), "");
}
//---------------------------------------------------------------------------
void TATCInterfaceForm::BitBtn2Click(TObject *Sender)
{
    SendCommToATC7(ATC_RUN,"","");
}
//---------------------------------------------------------------------------
void TATCInterfaceForm::BitBtn3Click(TObject *Sender)
{
    SendCommToATC7(ATC_STOP,"","");
}
//---------------------------------------------------------------------------
void TATCInterfaceForm::BitBtn4Click(TObject *Sender)
{
    int i = cBoxEnabledTSD->Checked;
    SendCommToATC7(ATC_USE_TSD,AnsiString(i),"");
}
//---------------------------------------------------------------------------
void TATCInterfaceForm::BitBtn5Click(TObject *Sender)
{
    AnsiString asData1="",asData2="";
    //int i = cBoxEnabledCH1->Checked;
    //int j = cBoxEnabledCH2->Checked;
    //int k = cBoxEnabledCH3->Checked;
    //int l = cBoxEnabledCH4->Checked;
    asData1.sprintf("%d,%d", cBoxEnabledCH1->Checked, cBoxEnabledCH2->Checked);
    asData2.sprintf("%d,%d", cBoxEnabledCH3->Checked, cBoxEnabledCH4->Checked);
    SendCommToATC7(ATC_CH_ENABLED, asData1, asData2);
}
//---------------------------------------------------------------------------
void TATCInterfaceForm::TimerATCTimer(TObject *Sender)
{
    static bool bTimerRun=false;
    double dTemp[ATC_HEAD_COUNT];
    double dTempOffset[ATC_HEAD_COUNT];
    unsigned int iAtc;
    AnsiString str1;
    if(bTimerRun==true || InitialOK==false)                                     //Steven 20160912 : Add InitialOK in Timer
        return;

    bTimerRun=true;
    ZeroMemory(dTemp, sizeof(dTemp));
    ZeroMemory(dTempOffset, sizeof(dTempOffset));

    if(ATC_SYSTEM==eATCHonPrecType)                                             //Steven 20130701 : 確認ATC設定正確
    {
        if(LastSet.iTemperature==Tempture_Hot)
        {
            for(iAtc=0; iAtc<ATCInterfaceForm->ATC_SYS_PAL.size(); iAtc++)
            {
                dTemp[iAtc]=Temperature.fWorkTemperBase;
                if(W906_FormShowing("fContact", fContact->fShow) && iContactMode!=CONTACT_NORMAL)  //AI(W906-PAGETAB-Q51) 20260928 [W906] 批2：golden「這個畫面開著嗎」改問頁面表的單一函式 W906_FormShowing（成員照傳；Steven Q51／Q-P3=A 直接生效）
                {
                    dTempOffset[iAtc]=Temperature.dATCInPC[iAtc];               //Steven 20151123 : Contact Mode no need another offset
                }
                else if(Temperature.bEnableATCTestTimeOffset &&                 //Steven 20160216 : 測試時間太短也要Offset
                        dTestSec<Temperature.iATCTestTimeOffsetTime)
                {
                    dTempOffset[iAtc]=Temperature.dATCInPC[iAtc]+Temperature.dATCTestTimeOffset[iAtc];
                }
                else if(Temperature.bEnableTempOffsetForInitial &&              //Steven 20141117 : 起測時溫度要補Offset
                   iInitContactCount<Temperature.iCintactCntForTempOffsetAtInitial)
                {
                    dTempOffset[iAtc]=Temperature.dATCInPC[iAtc]+Temperature.ATCInitialOffset[iAtc];   //Steven 20151006 : Initial Temp Offset for ATC
                }
                else if(Temperature.bEnableATCConFailOffset && Temperature.iATCCurrentFailCount[iAtc]>=Temperature.iATCConFailOffsetCount)  //Steven 20151123 : Continue Fail Temp Offset for ATC  //Steven 20151209 : Modify for ATC 7.0
                {
                    dTempOffset[iAtc]=Temperature.dATCConFailOffset[iAtc];
                }
                else if(LastSet.iRunStartMode==rsmQAMode && Temperature.bEnableATCQAModeOffset)    //Steven 20151125 : QA Mode Temp Offset for ATC
                {
                    dTempOffset[iAtc]=Temperature.dATCQAModeOffset[iAtc];
                }
                else
                {
                    dTempOffset[iAtc]=Temperature.dATCInPC[iAtc];               //Steven 20151112 : 修改ATC7.0 Offset
                }
            }
        }
        else
        {
            for(iAtc=0; iAtc<ATCInterfaceForm->ATC_SYS_PAL.size(); iAtc++)
            {
                dTemp[iAtc]=IniConfig.dATCAmbientTemperature;                   //Steven 20131122 : [L11] ATC常溫的溫度
                dTempOffset[iAtc]=Temperature.dATCInPC[iAtc];
            }
        }

        if(Temperature.bATC70Active==true)                                      //Eliot 2015_0105   //Steven 20151006 : Initial Temp Offset for ATC
        {
            if(LastSet.iTemperature==Tempture_Hot)                              //Steven 20151111 : For ATC7.0 修改判斷式
            {
                ATCInterfaceForm->SendCommToATC7(ATC_SET_TEMP, AnsiString(dTemp[0]), "");
                str1.sprintf("%2.1f,%2.1f,%2.1f,%2.1f", dTempOffset[0], dTempOffset[1], dTempOffset[2], dTempOffset[3]);
                ATCInterfaceForm->SendCommToATC7(ATC_OFFSET, str1, "");         //Steven 20151112 : 修改ATC7.0 Offset
            }
        }
        else
        {
            for(iAtc=0; iAtc<ATCInterfaceForm->ATC_SYS_PAL.size(); iAtc++)
            {
                if(ATCInterfaceForm->CheckWorkTemperature(iAtc, dTemp[iAtc])==false)
                {
                    ATCInterfaceForm->SetWorkTemperature(iAtc, dTemp[iAtc]);            //jou 2013-07-11 Temperature.fWorkTemperBase->dTemp
                    ATCInterfaceForm->SetOffsetTemperature(iAtc , dTempOffset[iAtc]);   //Steven 20151006 : Initial Temp Offset for ATC
                }
            }
        }
    }
    bTimerRun=false;
}
//---------------------------------------------------------------------------
void TATCInterfaceForm::pl_ATCSetTempChillerClick(
      TObject *Sender)
{
    //GATE (4) OPENED 20260824 (FW-QWKEY5): fQwertyKey real since FW-QWKEY1 (fc08e09); call restored from the gate comment, latent until a real event stream.
    fQwertyKey->ShowQwertyKey((TEdit*)Sender, N_DOUBLE, 0, true, 5.0, 50.0);
}
//---------------------------------------------------------------------------
void TATCInterfaceForm::edATCChillerCheckTimeClick(
      TObject *Sender)
{
    //GATE (4) OPENED 20260824 (FW-QWKEY5) -- see pl_ATCSetTempChillerClick above.
    fQwertyKey->ShowQwertyKey((TEdit*)Sender, N_INTEGER, 0, true, 120, 0);
}
//---------------------------------------------------------------------------
void TATCInterfaceForm::edtOffsetClick(TObject *Sender)
{
    //GATE (4) OPENED 20260824 (FW-QWKEY5) -- see pl_ATCSetTempChillerClick above.
    fQwertyKey->ShowQwertyKey((TEdit*)Sender, N_DOUBLE, 0, true, (double)InputLimit.iTempHigh, (double)InputLimit.iTempLow);
}
//---------------------------------------------------------------------------
void TATCInterfaceForm::edATC7_TempClick(TObject *Sender)
{
    //GATE (4) OPENED 20260824 (FW-QWKEY5) -- see pl_ATCSetTempChillerClick above.
    fQwertyKey->ShowQwertyKey((TEdit*)Sender, N_DOUBLE, 1, true, dTempMax, dTempMin);
}
//---------------------------------------------------------------------------
void TATCInterfaceForm::edtPortClick(TObject *Sender)
{
    //GATE (4) OPENED 20260824 (FW-QWKEY5) -- see pl_ATCSetTempChillerClick above.
    fQwertyKey->ShowQwertyKey((TEdit*)Sender, N_PORT);
}
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
//AI(W906-ATCEMG) 20261001: RULINGS_20261001 #5 (census 129 (a)(b) #17, safety gap). golden csystem.cpp:1442-1443, on the EMG-pressed
//  path, `if(bServoOff==false) ATCInterfaceForm->SendCommToATC7(ATC_EMG_DOWN,"","");` -- the ATC 7.0 controller is told the
//  handler EMG went down ("@EMG_DOWN+", SendCommToATC7 case 9 above). csystem.cpp cannot include this header (the clWhite
//  collision at its G-ATC block), so its G-ATC-B arm calls this bridge; the body is golden's statement, unchanged
//  (no client connected -> SendCommToATC7 only logs "No Client!" in HandlerMemo, as golden).
void W906_ATC_SendEmgDown()
{
    ATCInterfaceForm->SendCommToATC7(ATC_EMG_DOWN,"","");                       //Eliot 2015_0122
}
//AI(W906-ATCEMG) 20261001: golden csystem.cpp:1470, the EMG-released edge of the same handshake (G-ATC-C, opened with G-ATC-B).
void W906_ATC_SendEmgUp()
{
    ATCInterfaceForm->SendCommToATC7(ATC_EMG_UP,"","");                         //Eliot 2015_0122
}
//AI(W906-ATCEMG) 20261001: golden csystem.cpp:1111-1129, the whole body of CheckATC6System (G-ATC-A, the same header-visibility
//  gate): on an ATC 6.0 / 3.0 machine the ATC's error messages reach the operator and a dropped ATC link is re-opened.
//  csystem.cpp's CheckATC6System calls this; the text is golden's, line for line.
void W906_CheckATC6System_Body()
{
    AnsiString sMessage;
    if(ATC_SYSTEM==eATC60 ||                                                    //ChungHung 20141024 add
       ATC_SYSTEM==eATC30)                                                      //20141204 ChungHung add for ATC3.0
    {
        if(SystemInitialOK==false)
            return;

        if(ATCInterfaceForm->ATC_60_SYS.GetErrorMessage(sMessage))
        {
            ShowMyMessage(sMessage);
        }

        if(ATCInterfaceForm->ATC_60_SYS.IsConnected()==false &&
           ATCInterfaceForm->ATC_60_SYS.IsConnecting()==false)
        {
            ATCInterfaceForm->ATC_60_SYS.CloseSocket();
            ATCInterfaceForm->ATC_60_SYS.OpenSocket();
        }
    }
}
