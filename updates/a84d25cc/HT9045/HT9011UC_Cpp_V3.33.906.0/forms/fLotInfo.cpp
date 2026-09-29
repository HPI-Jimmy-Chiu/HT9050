// =============================================================================
//  forms/fLotInfo.cpp  --  definitions for the fLotInfo facade
//
//  AI(W906-W7-F0) 20260728: split out of FormsFacade.cpp by the W7-F0 refactor
//  (docs/W7_UI_ARCHITECTURE_PLAN.md SS6-F0-d).  Bodies moved VERBATIM, with
//  exactly ONE addition, marked below: palRemoveTray's Enabled/Visible are now
//  set explicitly in the ctor, because the bespoke TfLotInfoPanel type they
//  used to carry defaulted them to true/true while the unified
//  vclcompat::TPanel defaults every control to false (see
//  vclcompat/Controls.h's DEFAULT-VALUE RULE).  Setting them here keeps the
//  CONSTRUCTED STATE byte-for-byte identical to before the refactor, which is
//  what W7-F0's zero-behaviour-change contract requires.
// =============================================================================
#include "forms/fLotInfo.h"
// AI(W906-FW-SIG-W15) 20260826: edDeviceNameMouseDown 回填 golden 完整簽章。
#include "vclcompat/ShiftState.h"

// AI(W906-FW3-LotInfo-WA) 20260819: Wave A includes -- every global these 39
// Tier-1 methods dereference lives in one of these; see forms/fLotInfo.h's
// banner for the per-dependency existence citations gathered this wave.
#include "cmydef.h"            // AccessLevel/iDef*Level/CUSTOMER_CODE/CC_*/ATC_SYSTEM/Tri_Temp_Machine/
                                //   AirStream_Select/Total_Compressor/iATC_Use_Heat_Count/SiteData[]/
                                //   USE_RFID_READER/N_NO_SYMBOL/N_NO_SPACE/SystemYear../bLoaderActionFlag[]/
                                //   bUnLoaderActionFlag[]/bAMRReceive*/iLoaderTrayCountCal/iUnloaderTrayCountCal[]/
                                //   sB03RunData/sB03StartTime/bTesterSendPause/bTesterPauseMusic
#include "cprod.h"              // TestIF_File/BinSelect[]/Prod/RunInfo/SystemStart
#include "Config.h"             // IniConfig
#include "CosFunction.h"        // CosFunction
#include "LastSet.h"            // LastSet.iTester/OFF_LINE
#include "common.h"             // ReadIniData/GetLastOpenFN/AuthPath
#include "canary_support.h"    // ShowMyMessage
#include "cpublic.h"            // GetTimeInfo()
#include "vclcompat/IniFiles.h" // TIniFile (TransformTemperature_AirStream)
#include "vclcompat/FileListBox.h" // TFileListBox (bCheckOnlyOneFile)
#include "forms/fMain.h"        // fMain->edWorkTemperBase (TransformTemperature_AirStream)
#include "forms/fSetup.h"       // AI(W906-FW3-Setup-WA) 20260820: fSetup->edOcrText (WC-27 retired)

// AI(W906-FW3-LotInfo-WB) 20260819: Wave B includes -- see forms/fLotInfo.h's
// WB banner for the per-dependency existence citations gathered this wave.
#include "Automation/AMR.h"      // real TTeraPowerAMR AMR (RefreshAMR: CheckLoaderCount/CheckUnloaderCount)
#include "SortingBinTray/SortingBinTray.h"  // SaveTrayRecord (btnSaveDataClick)

// AI(W906-FW3-LotInfo-WC) 20260819: Wave C include -- fAGV->IsSPIL_AMR()
// (Timer2Timer T4, confirmed REAL by this wave's step 0).
#include "forms/fAGV.h"
#include "BarcodeReader.h"   // AI(W906-FW-BARCODE3) 20260825: InputBarcodeNumber real since FW-BARCODE1 -- WB-4/WB-5/WB-7 opened
#include "forms/fQwertyKey.h"  // AI(W906-FW-QWKEY2) 20260824: fQwertyKey extern for un-gated ShowQwertyKey sites (real since FW-QWKEY1 fc08e09; latent until HTEdit GATE (6) wiring)
// AI(W906-FW-LOTINFO-W30) 20260826: acarry_shims.h:115 declares
// `extern TATC_InterfaceFormShim *ATC_InterfaceForm;` -- the ONLY declaration of
// that global anywhere in this tree (grepped `\*\s*ATC_InterfaceForm` over all
// .h, 20260826: acarry_shims.h:115 is the sole hit; forms/fTemp_Set.h:337 is
// prose).  REQUIRED BY GATE WC-10's opening, and the reason W27's "WC-10 is
// openable" verdict was incomplete: before this include NO use of
// ATC_InterfaceForm in this file was ever compiled -- all of them (:1280 in the
// WA-6 block, :2706-onward in WB-16/WB-17) sit inside `#if 0`.
// MEASURED COST, not assumed (both with the ht9045_forms flags + rsp, 20260826):
//   * defined symbols in fLotInfo.o: IDENTICAL before/after (nm --defined-only
//     diff empty) -- this header contributes no in-header object definitions, so
//     no multiple-definition exposure despite pulling aHotPlateSubstrate.h.
//   * undefined symbols: EXACTLY ONE added, _ATC_InterfaceForm.  That is a +1 on
//     the ALREADY-DOCUMENTED forms->ht9045_sm cycle (CMakeLists.txt:715-721),
//     10 -> 11 symbols; body at acarry_shims.cpp:73, archive ht9045_sm
//     (CMakeLists.txt:1833).  No new archive EDGE, one more symbol on an
//     existing one.
//   * TMyKitSuck ODR trap (docs/KNOWLEDGE.md): this pulls the
//     aHotPlateSubstrate.h:365 flavour, i.e. the SAME one the other 177 TUs
//     take.  mykitsuck.h is NOT reachable from this TU, so no fork is created.
#include "acarry_shims.h"

// AI(W906-FW3-LotInfo-WB) 20260819: TU-local forward decl of MyDBIProcess
// (FormDestroy's exception log). Its only declaration in this tree is
// aHotPlateSubstrate.h:933, but that header transitively drags in
// Motor/mymotor.h/HTMotor.h (a large unrelated surface) for one extern
// function -- forward-declaring here instead, matching the SAME "TU-local
// copy instead of a shared heavy header" idiom this file already uses for
// IncludeTrailingPathDelimiter above and the TColor constants below. Real
// definition (linked from wherever aHotPlateSubstrate.cpp/its owner TU is)
// is unaffected; signature copied verbatim.
extern void MyDBIProcess(AnsiString S1, AnsiString S2);

// AI(W906-FW3-LotInfo-WB) 20260819: TU-local TColor constants -- the SAME
// "every TU that needs this carries an identical local copy" idiom this
// file's own IncludeTrailingPathDelimiter (above) and, tree-wide,
// cObserver.cpp/cShowBinSelect.cpp/Interface/TesterTCP_Socket.cpp etc. all
// already use for these exact 4 colours (TColor itself is `typedef int` from
// cmydef.h, already included). Values copied verbatim from
// vclcompat/LedCore.h:54-59 (clGreen/clRed/clLime) and ATC/ATCInterface.h:181
// (clGray) -- not #include'd directly to avoid a duplicate-const-definition
// ODR risk if some other already-included header also brings one in.
static const TColor clGreen = 0x00008000;
static const TColor clRed   = 0x000000FF;
static const TColor clLime  = 0x0000FF00;
static const TColor clGray  = 0x00808080;

// ---------------------------------------------------------------------------
// AI(W906-FW-LOTINFO-W30) 20260826: TU-local ATC_TYPE_61 -- retires GATE WC-10.
// golden ATC/ATC_Handler_Side.h:30 is not ported as a shared include, so this
// tree's ESTABLISHED idiom for this exact constant family is a TU-local mirror
// in every consuming .cpp.  Five already exist, and cTemperFrom.cpp:118-120's
// own comment names the convention: csystem.cpp:20244-20246, cTemperFrom.cpp
// :123, cUnitConvert.cpp:358-360, uHeaterThread.cpp:313-315, uTemp_Set.cpp:190.
// This is the 6th, and it is a mirror of an unported golden header constant --
// NOT a 6th spelling of a tree-owned constant (the contact-mode fork the brief
// warns about); nothing is added to any shared header.
//
// WHY NOT THE BARE LITERAL 61: the W27 banner's recipe (header:966-968) said to
// spell it 61 "matching forms/fLotInfo.cpp:1280, un-gated".  THAT PREMISE IS
// FALSE -- :1280 sits INSIDE the WA-6 `#if 0` block (this file:1271-1330), so
// there is no un-gated same-file precedent for the literal.  Re-verified
// 20260826.  uTemp_Set.cpp:838 translates the STRUCTURALLY IDENTICAL golden
// line (tsTriTempSet->TabVisible=(Tri_Temp_Machine==1 || (ATC_SYSTEM==
// eNewATCSystem && ATC_InterfaceForm->iATC_MODE_TYPE==ATC_TYPE_61));) with the
// named constant, so the named spelling keeps FormShow S1 byte-identical to
// golden and consistent with its own sibling translation.
// ---------------------------------------------------------------------------
#ifndef ATC_TYPE_61
#define ATC_TYPE_61         61                                                  // golden ATC/ATC_Handler_Side.h:30
#endif

using vclcompat::TFileListBox;

// ---------------------------------------------------------------------------
// IncludeTrailingPathDelimiter -- BCB6 synonym for IncludeTrailingBackslash.
// Every other TU that needs this (cprod.cpp/cpublic.cpp/handlerlog.cpp/
// Interface/TesterTCP.cpp/SECSGEM/uHGemClass.cpp/uHGemEquipment.cpp/
// Automation/SCK_ART_Remainder.cpp) carries an identical TU-local copy rather
// than a shared one -- centralizing it was not any of those tasks' call to
// make, so this file gets its own copy too (N23UseLotInfoFile, golden :7351).
// ---------------------------------------------------------------------------
static inline AnsiString IncludeTrailingPathDelimiter(const AnsiString& p)
{
    return IncludeTrailingBackslash(p);
}

// ---------------------------------------------------------------------------
// AI(W906-FW3-LotInfo-WA) 20260819: golden FILE-SCOPE globals (uLotInfo.cpp
// top-of-file, NOT TfLotInfo members -- confirmed by reading golden uLotInfo.h
// for each: none of these 4 appear there). Declared here, at file scope, in
// the one file that currently needs them, exactly mirroring golden's own
// scope. Initial values copied verbatim from golden (uLotInfo.cpp:89-91/95,
// :16256-16257).
// ---------------------------------------------------------------------------
static TLabel *SocketLabRow_Display[MAX_SOCKET_ROW] = { new TLabel(), new TLabel(), new TLabel(), new TLabel() };
static TLabel *SocketLabCol_Display[MAX_SOCKET_COL] = { new TLabel(), new TLabel(), new TLabel(), new TLabel(),
                                                         new TLabel(), new TLabel(), new TLabel(), new TLabel() };
bool bLotID_OK=false;                 // golden uLotInfo.cpp:91
bool bLotFirstKeyIn=false;            // golden uLotInfo.cpp:95
int iLoaderTask[3]={1,1,1};           // golden uLotInfo.cpp:16256
int iloaderLevelTask[3]={1,1,1};      // golden uLotInfo.cpp:16257
// AI(W906-FW3-LotInfo-WB) 20260819: 2 more file-scope globals from the SAME
// golden top-of-file cluster as bLotID_OK/bLotFirstKeyIn above (golden
// uLotInfo.cpp:92-94), needed by this wave's edtSysOperatorIDKeyDown/
// edTempKeyPress/edTempKeyDown/edDeviceNameKeyDown.
bool bOPID_OK=false;                  // golden uLotInfo.cpp:92
bool bTemp_OK=false;                  // golden uLotInfo.cpp:93
bool bDeviceName_OK=false;            // golden uLotInfo.cpp:94

// --- W6.2: TfLotInfo -------------------------------------------------------
TfLotInfo::TfLotInfo()
{
    cbRunMode = new TfLotInfoRunMode();             // offline: Visible=false
    // -- W6.3 ADD --
    labNowLoaderTrayID = new TfLotInfoLabel();
    edtSysLotID        = new TfLotInfoEdit();
    // AI(W906-FW-Y3) 20260819: the 5 AutoClean-display widgets (see header)
    Label17                   = new TfLotInfoLabel();
    Label18                   = new TfLotInfoLabel();
    Label21                   = new TfLotInfoLabel();
    edtAutoCleanLowYield      = new TfLotInfoEdit();
    edtAutoCleanSiteYieldDiff = new TfLotInfoEdit();
    // -- W5-Automation ADD --
    cbProcess          = new TfLotInfoRunMode();
    // -- W5-Final-TesterTCPSocket ADD --
    labTCPIPStatus = new TfLotInfoStatusLabel();   labTCPIPStatus->Caption="OFF-LINE"; labTCPIPStatus->Color=clRed;   //Steven 20260925 (Data.LotInfo)：dfm 設計期值（V912 uLotInfo.dfm:11239／:11240），Interface/TesterTCP_Socket.cpp 連線事件才改；同一行，不移動本檔行號
    mmTesterLog    = new TfLotInfoLogMemo();   //Steven 20260925 (Data.LotInfo)：存得住行的 memo（forms/fLotInfo.h 檔尾；原本的 TfMainMemo 是 no-op），Tester Log 分頁要顯示；同一行，不移動本檔行號
    // -- W5-Automation ADD (AGV_PortScan unit, 20260713) -----------------------
    ALedLoader    = new TfLedValue();
    for(int i=0;i<3;i++) aLedAuto[i] = new TfLedValue();
    palRemoveTray = new TfLotInfoPanel();
    // AI(W906-W7-F0) 20260728: STATE-PRESERVING ADDITION (the only behavioural
    // line added anywhere by the F0 refactor, and it exists precisely to add
    // NOTHING behaviourally).  The retired bespoke type was
    // `struct TfLotInfoPanel { bool Enabled; bool Visible;
    //  TfLotInfoPanel():Enabled(true),Visible(true){} };` -- true/true, per its
    // own W5-Automation note ("ordinary VCL TPanel design-time defaults ...
    // no consumer depends on the initial value": both fields are only ever
    // WRITTEN, by Automation/AGV_PortScan.cpp:305-306, and tests/
    // test_agv_portscan.cpp:382-387 sets them true itself before asserting they
    // become false).  The unified vclcompat::TPanel defaults them to false, so
    // they are restored here rather than left to drift.
    palRemoveTray->Enabled = true;
    palRemoveTray->Visible = true;
    // -- W906-AutoCleanFoundation ADD (20260721) ------------------------------
    for(int iW906AC=0; iW906AC<3; iW906AC++) iUnloaderTask[iW906AC] = 0;
    // -- AI(W906-Save2DSortingSummary) 20260723 ADD: 6 new TfLotInfoEdit members --
    edtCusLotID       = new TfLotInfoEdit();
    edtCusDevGrp      = new TfLotInfoEdit();
    edtCusStep        = new TfLotInfoEdit();
    edtDevice         = new TfLotInfoEdit();
    edtSysOperatorID  = new TfLotInfoEdit();
    mmo2DLotInfo      = new vclcompat::TMemo();  // AI(W906-FW3-LotInfo-WA) 20260819: widened, see forms/fLotInfo.h member comment
    // -- AI(W906-SaveTestSummaryTSV) 20260728 ADD: 3 new TfLotInfoEdit members + 1 more
    //    (lbledtCustomer, AI(W906-SaveSummaryTrayFeed) 20260728) --
    edtASECL_LotID    = new TfLotInfoEdit();
    edInsertion       = new TfLotInfoEdit();
    edFlowID          = new TfLotInfoEdit();
    lbledtCustomer    = new TfLotInfoEdit();
    // -- AI(W906-W7-L1-Wave0) 20260801 ADD: the 7 W7-L1 fLotInfo members (see
    //    forms/fLotInfo.h for per-member golden citations and for why the ""
    //    Caption default is behaviourally load-bearing on the KYEC-AMR arm) --
    LabDiffTrayCount           = new TfLotInfoLabel();   // golden uLotInfo.h:1086
    labLoaderTrayCount         = new TfLotInfoLabel();   // golden uLotInfo.h:1080
    labNowTrayCount            = new TfLotInfoLabel();   // golden uLotInfo.h:1084
    labNowAuto1TrayID          = new TfLotInfoLabel();   // golden uLotInfo.h:799
    labNowAuto2TrayID          = new TfLotInfoLabel();   // golden uLotInfo.h:802
    labNowAuto3TrayID          = new TfLotInfoLabel();   // golden uLotInfo.h:804
    cbFirstTrayCheckOnUnloader = new TfMainCheckBox();   // golden uLotInfo.h:1026 -- offline Checked=false
    // AI(W906-PT-W3-integrate) 20260808: golden uLotInfo.h:266 -- only ->Click() is
    // touched, and that is an inherited offline no-op; see forms/fLotInfo.h for why
    // golden's OnClick chain cannot run here.
    btClearBarcodeList         = new vclcompat::TButton();  // golden uLotInfo.h:266

    // =======================================================================
    //  AI(W906-FW3-LotInfo-WA) 20260819: Wave A ADD -- see forms/fLotInfo.h
    //  for per-member golden citations.
    // =======================================================================

    // -- RefreshYieldMonitor_SIGURD ------------------------------------------
    gbManualCheckList             = new TGroupBox();
    btnManualStandard             = new TButton();
    cbMonitor_FTPRMS              = new TCheckBox();
    cbContactMode                 = new TComboBox();
    cbSiteYieldCmp_FT             = new TCheckBox();
    cbLowYieldByTotal_FT          = new TCheckBox();
    edSiteYieldCmpOnOff_Cur       = new TEdit();
    edLowYieldByTotalOnOff_Cur    = new TEdit();
    edSiteYieldCmpIg_FT           = new TEdit();
    edLowYieldByTotalIg_FT        = new TEdit();
    edSiteYieldCmpIg_Cur          = new TEdit();
    edLowYieldByTotalIg_Cur       = new TEdit();
    lblSiteYieldCmpIg_Cur         = new TLabel();
    lblLowYieldByTotalIg_Cur      = new TLabel();
    edSiteYieldCmp_FT             = new TEdit();
    edSiteYieldCmp_Cur            = new TEdit();
    lblSiteYieldCmp_Cur           = new TLabel();
    edLowYieldByTotal_FT          = new TEdit();
    edLowYieldByTotal_Cur         = new TEdit();
    lblLowYieldByTotal_Cur        = new TLabel();
    rbContsFailBySocket_FTOn      = new TRadioButton();
    rbContsFailBySocket_FTOff     = new TRadioButton();
    edtContsFailBySocket_Cur      = new TEdit();
    edContsFailSocketAlarmCT_FT   = new TEdit();
    edContsFailSocketAlarmCT_Cur  = new TEdit();
    lblContsFailSocketAlarmCT_Cur = new TLabel();
    rbContsFailByHead_FTOn        = new TRadioButton();
    rbContsFailByHead_FTOff       = new TRadioButton();
    edtContsFailByHead_Cur        = new TEdit();
    edContsFailHeadAlarmCT_FT     = new TEdit();
    edContsFailHeadAlarmCT_Cur    = new TEdit();
    lblContsFailHeadAlarmCT_Cur   = new TLabel();
    edOSBin                       = new TEdit();
    edOSBinCnt                    = new TEdit();
    edOSBinCnt_Cur                = new TEdit();
    edOSBinPreset                 = new TEdit();
    edOSBinPreset_Cur             = new TEdit();
    lblOSBin_Cur                  = new TLabel();
    grpOSBin                      = new TGroupBox();

    // -- AdjtsYieldMonitiorSize -----------------------------------------------
    pgLotinfo         = new TfLotInfoPageControl();
    pgcLotInfo        = new vclcompat::TPageControl();
    ts_ATC6_1         = new TTabSheet();
    tsATC             = new TTabSheet();
    tsASECLEventLog   = new TTabSheet();
    tsYieldMonitior   = new TTabSheet();
    tsFTP             = new TTabSheet();
    tsLotID           = new TTabSheet();
    tsMurata          = new TTabSheet();
    tsSigurd_CX       = new TTabSheet();
    tsSPIL_SZ         = new TTabSheet();
    tsOEE             = new TTabSheet();
    ts2DSort          = new TTabSheet();
    tsChipAdv         = new TTabSheet();
    tsVTest           = new TTabSheet();
    tsOCRBarCode      = new TTabSheet();
    tsAMR             = new TTabSheet();
    Height = 0; Width = 0; Top = 0; Left = 0;

    // -- SetSelectionVisible --------------------------------------------------
    groupbDownloadItem   = new TGroupBox();
    grpMesCheck          = new TGroupBox();
    tsSelection          = new TTabSheet();
    Panel28              = new TfLotInfoLayoutPanel();
    cbTestTimes          = new TComboBox();
    lblTestTimes         = new TLabel();
    lblDownloadAccessWarning = new TLabel();

    // -- ShowSocketID ----------------------------------------------------------
    for(int iWaSockR=0; iWaSockR<MAX_SOCKET_ROW; iWaSockR++)
        for(int iWaSockC=0; iWaSockC<MAX_SOCKET_COL; iWaSockC++)
        {
            SocketSiteCH_Display[iWaSockR][iWaSockC] = new TPanel();
            edSocket[iWaSockR][iWaSockC]              = new TEdit();
        }

    // -- InitialRefrigerantSystem ----------------------------------------------
    bInitFormcomponent  = false;
    OldRefrigerantCommand = false;
    ts_RefrigerantStatus_Page_2 = new TTabSheet();
    pnlRefrigerantMachine1 = new TPanel(); pnlRefrigerantMachine2 = new TPanel();
    pnlRefrigerantMachine3 = new TPanel(); pnlRefrigerantMachine4 = new TPanel();
    pnlRefrigerantMachine5 = new TPanel(); pnlRefrigerantMachine6 = new TPanel();
    pnlRefrigerantMachine7 = new TPanel(); pnlRefrigerantMachine8 = new TPanel();
    LabRefrigerantValue1 = new TLabel(); LabRefrigerantValue2 = new TLabel();
    LabRefrigerantValue3 = new TLabel(); LabRefrigerantValue4 = new TLabel();
    LabRefrigerantValue5 = new TLabel(); LabRefrigerantValue6 = new TLabel();
    LabRefrigerantValue7 = new TLabel(); LabRefrigerantValue8 = new TLabel();
    pnlRefCopm1Status_1 = new TPanel(); pnlRefCopm1Status_2 = new TPanel();
    pnlRefCopm1Status_3 = new TPanel(); pnlRefCopm1Status_4 = new TPanel();
    pnlRefCopm1Status_5 = new TPanel(); pnlRefCopm1Status_6 = new TPanel();
    pnlRefCopm1Status_7 = new TPanel(); pnlRefCopm1Status_8 = new TPanel();
    pnlRefCopm2Status_1 = new TPanel(); pnlRefCopm2Status_2 = new TPanel();
    pnlRefCopm2Status_3 = new TPanel(); pnlRefCopm2Status_4 = new TPanel();
    pnlRefCopm2Status_5 = new TPanel(); pnlRefCopm2Status_6 = new TPanel();
    pnlRefCopm2Status_7 = new TPanel(); pnlRefCopm2Status_8 = new TPanel();
    labRefCopm1HpValue_1 = new TLabel(); labRefCopm1HpValue_2 = new TLabel();
    labRefCopm1HpValue_3 = new TLabel(); labRefCopm1HpValue_4 = new TLabel();
    labRefCopm1HpValue_5 = new TLabel(); labRefCopm1HpValue_6 = new TLabel();
    labRefCopm1HpValue_7 = new TLabel(); labRefCopm1HpValue_8 = new TLabel();
    labRefCopm2HpValue_1 = new TLabel(); labRefCopm2HpValue_2 = new TLabel();
    labRefCopm2HpValue_3 = new TLabel(); labRefCopm2HpValue_4 = new TLabel();
    labRefCopm2HpValue_5 = new TLabel(); labRefCopm2HpValue_6 = new TLabel();
    labRefCopm2HpValue_7 = new TLabel(); labRefCopm2HpValue_8 = new TLabel();
    labRefCopm1LpValue_1 = new TLabel(); labRefCopm1LpValue_2 = new TLabel();
    labRefCopm1LpValue_3 = new TLabel(); labRefCopm1LpValue_4 = new TLabel();
    labRefCopm1LpValue_5 = new TLabel(); labRefCopm1LpValue_6 = new TLabel();
    labRefCopm1LpValue_7 = new TLabel(); labRefCopm1LpValue_8 = new TLabel();
    labRefCopm2LpValue_1 = new TLabel(); labRefCopm2LpValue_2 = new TLabel();
    labRefCopm2LpValue_3 = new TLabel(); labRefCopm2LpValue_4 = new TLabel();
    labRefCopm2LpValue_5 = new TLabel(); labRefCopm2LpValue_6 = new TLabel();
    labRefCopm2LpValue_7 = new TLabel(); labRefCopm2LpValue_8 = new TLabel();
    LabRefrigerantAdjustValue1 = new TLabel(); LabRefrigerantAdjustValue2 = new TLabel();
    LabRefrigerantAdjustValue3 = new TLabel(); LabRefrigerantAdjustValue4 = new TLabel();
    LabRefrigerantAdjustValue5 = new TLabel(); LabRefrigerantAdjustValue6 = new TLabel();
    LabRefrigerantAdjustValue7 = new TLabel(); LabRefrigerantAdjustValue8 = new TLabel();
    for(int iWaRef=0; iWaRef<8; iWaRef++)
    {
        TripnlRefrigerantMachine[iWaRef]    = 0;
        TriLabRefrigerantValue[iWaRef]      = 0;
        TripnlRefCopm1Status[iWaRef]        = 0;
        TripnlRefCopm2Status[iWaRef]        = 0;
        TriLabRefCopm1HpValue[iWaRef]       = 0;
        TriLabRefCopm2HpValue[iWaRef]       = 0;
        TriLabRefCopm1LpValue[iWaRef]       = 0;
        TriLabRefCopm2LpValue[iWaRef]       = 0;
        TriLabRefrigerantAdjustValue[iWaRef] = 0;
    }

    // -- SetATCFormVisible ------------------------------------------------------
    palATC              = new TPanel();
    aldATC7Status       = new TPanel();   // conflated from golden TALed*, ->Visible only
    lblATC70            = new TLabel();
    aldATCChillerStatus = new TPanel();   // conflated from golden TALed*, ->Visible only
    lblChiller          = new TLabel();
    lblATC_Now_RecipeFile = new TLabel();
    pan_ATCChillerSV    = new TPanel();
    pl_ATCChillerSV     = new TPanel();
    NetATCTime          = new TfLotInfoTimer();

    // -- CheckEventLogParameter ---------------------------------------------
    edCustomerDevice    = new TEdit();
    btnASECL_LotStart   = new TSpeedButton();

    // -- ShowATCTempPanel -----------------------------------------------------
    Pan_ATC_Use_4Head  = new TfLotInfoLayoutPanel();
    Pan_ATC_Use_8Head  = new TfLotInfoLayoutPanel();
    Pan_ATC_Use_32Head = new TfLotInfoLayoutPanel();
    for(int iWaAtcH=0; iWaAtcH<ATC_HEAD_COUNT; iWaAtcH++)
    {
        ATCChPal[iWaAtcH]    = new TPanel();
        ATCPtr[iWaAtcH]      = new TPanel();
        ATCReferPtr[iWaAtcH] = new TPanel();
    }

    // -- JCETWhite2DIDShow ------------------------------------------------------
    labCusLotID = new TLabel();

    // -- ShowInformation --------------------------------------------------------
    gbFTPAutomation_Download = new TGroupBox();
    gbFTPAutomation_Upload   = new TGroupBox();
    sbRecipeUpload           = new TSpeedButton();
    sbRecipeDownload         = new TSpeedButton();
    sbFTPAutomationSave      = new TSpeedButton();
    sbTest                   = new TfLotInfoLayoutButton();

    // -- edTempKeyUp --------------------------------------------------------------
    edTemp = new TfLotInfoTextEdit();

    // -- edtSysLotIDKeyPress / edPageKeyPress ------------------------------------
    edPage = new TEdit();

    // -- bCheckOnlyOneFile / CheckActionFlag -------------------------------------
    ledLoader = new TfLedValue(); ledEmpty = new TfLedValue(); ledColor = new TfLedValue();
    ledAuto1  = new TfLedValue(); ledAuto2 = new TfLedValue(); ledAuto3 = new TfLedValue();
    ledStartAGV = new TfLedValue(); ledSTART = new TfLedValue();
    ledLoaderTotalTray = new TfLedValue(); ledLOT_START = new TfLedValue();

    // -- btnLoadFileClick -----------------------------------------------------
    OpenDialog1        = new TfLotInfoOpenDialog();
    edSort2DIDBinFile  = new TEdit();

    // -- ReflashInfo ------------------------------------------------------------
    labAuto1TrayCount_KYEC = new TLabel();
    labAuto2TrayCount_KYEC = new TLabel();
    labAuto3TrayCount_KYEC = new TLabel();

    // -- edtLotVerifyMouseDown / edtLotVerifyKeyPress ----------------------------
    edtLotVerify = new TLabeledEdit();

    // -- BtnPauseMouseDown / BtnPauseMouseUp -------------------------------------
    BtnPause = new TfMainSpeedButton();

    // -- VisibleUploadBtnPAT ------------------------------------------------------
    sbUploadPAT = new TSpeedButton();

    // -- cbbDeviceNameChange ------------------------------------------------------
    edDeviceName  = new TEdit();
    cbbDeviceName = new TComboBox();

    // -- btStartCountClick ---------------------------------------------------------
    bStartCount_SCK = false;   // golden uLotInfo.h:1294 -- no explicit golden ctor init found; NSDMI-style false default

    // =======================================================================
    //  AI(W906-FW3-LotInfo-WB) 20260819: Wave B ctor ADD -- see forms/
    //  fLotInfo.h WB-19 for the gated named-widget-array wiring block this
    //  section deliberately omits. Allocations first, then the REAL golden
    //  logic in golden's own order (golden uLotInfo.cpp:157-299).
    // =======================================================================
    lblOCR_LotID = new TLabel();
    slASECLTestInfor = new TStringList();
    TimerERMS = new TfLotInfoTimer();
    Timer3    = new TfLotInfoTimer();
    tsChamberBoost = new TTabSheet();
    btnCancelTestPause = new TButton();
    btnESCFunction     = new TButton();
    ts_FTPAutomation   = new TTabSheet();
    ATC_WinWay         = new TTabSheet();
    pnlWinwayPVCH1 = new TPanel(); pnlWinwayPVCH2 = new TPanel();
    pnlWinwayPVCH3 = new TPanel(); pnlWinwayPVCH4 = new TPanel();
    lbOCRUseFile      = new TLabel();
    spOCRChangeFile   = new TSpeedButton();
    lbCheckCodeByLot  = new TPanel();
    sbSECSLotStart = new TSpeedButton(); sbSECSLotEnd = new TSpeedButton();
    labRefrigerantMachineHighLimit = new TLabel();
    labRefrigerantMachineLowLimit  = new TLabel();
    leRunCardNumber = new TLabeledEdit();
    cbA60_1 = new TCheckBox();
    pnlWaitTXSetLoader = new TPanel(); pnlWaitRXSetAuto1 = new TPanel();
    pnlWaitRXSetAuto2  = new TPanel(); pnlWaitRXSetAuto3 = new TPanel();
    pnlWaitTXTotalLoader = new TPanel(); pnlWaitTXCntLoader = new TPanel();
    pnlWaitRXCntAuto1 = new TPanel(); pnlWaitRXCntAuto2 = new TPanel(); pnlWaitRXCntAuto3 = new TPanel();
    aldWaitTXLoader = new TfLedValue();
    aldWaitRXAuto1 = new TfLedValue(); aldWaitRXAuto2 = new TfLedValue(); aldWaitRXAuto3 = new TfLedValue();
    aldWaitTrayFeed = new TfLedValue();
    aldLoaderLast   = new TfLedValue();
    // AI(W906-FW3-LotInfo-WBfix) 20260819: hydrate to golden uLotInfo.dfm dims
    // (ColCount=3, RowCount=16) -- the bare 5x5 default let ShowAMRCategoryBin's
    // non-initial path (Cells[1][1+i], rows up to iTestBinCount) run off row 5
    // before anything called the bInitial=true resize: caught by the AMR test
    // (TTeraPowerAMR::Initial -> RefreshAMR) in the Wave B gate, std::out_of_range
    // n=5 size=5. Same defect class and same fix shape as cObserver's
    // sgStatisticsJam (Obs2fix).
    StrGrdCategory  = new TStringGrid(3, 16);
    edLoaderCountNow = new TEdit(); edLoaderCountAlarm = new TEdit();
    bP60UserClicked = false;   // golden uLotInfo.h:1262 -- NSDMI-style false default, same idiom as bStartCount_SCK above
    // -- pgLotinfoChange: 20 TCheckBox widgets --
    chkTempOffset = new TCheckBox(); chkContactHigh = new TCheckBox();
    chkContactForce = new TCheckBox(); chkContactMode = new TCheckBox();
    chkHotPlate = new TCheckBox(); chkLoadUnload = new TCheckBox();
    chkSpeedSetting = new TCheckBox(); chkShuttleMode = new TCheckBox();
    chkTestMode = new TCheckBox(); chkBinasgn = new TCheckBox();
    chkBinasgnOff = new TCheckBox(); checkbAutoClean = new TCheckBox();
    chkAutoCleanContactHeight = new TCheckBox(); cbBottom2DOffset = new TCheckBox();
    chkART = new TCheckBox(); chkART_RTCount = new TCheckBox();
    chkIndexHeatingMode = new TCheckBox(); chkStopYield = new TCheckBox();
    chkConsecutiveFailure = new TCheckBox(); chkCleanCount = new TCheckBox();

    // -- REAL golden ctor logic (golden :160-299, minus the WB-19 gated span) --
    bShow=false;
    lblOCR_LotID->Caption="";
    iLotRead  =0;
    iLotStart =1;
    iLotEnd   =2;
    bStartChamberBoost=false;
    iXMLOnLineStatus=0;
    bRTCChangeFileFinish=false;
    // golden :169 `bStartCount_SCK=false;` -- already set above by the
    // pre-existing Wave A ctor tail; not re-set here (same net value).
    sVTestInternalLot="";

    // AI(W906-FW3-LotInfo-WB) 20260819: GATE WB-19 -- golden :172-269, the
    // ESD_DataPtr[18]/ESD_DECAY_DATA_Ptr[36]/ATCChPal[32]/ATCReferPtr[32]/
    // ATCPtr[32]/edSocket[4][8]/SocketSiteCH_Display[][]/
    // SocketLabCol_Display[]/SocketLabRow_Display[] named-widget wiring
    // block. See forms/fLotInfo.h GATE REGISTER WB-19.
#if 0
    ESD_DataPtr[ 0]=pl_ESDProx1_1; ESD_DataPtr[ 1]=pl_ESDProx1_2; ESD_DataPtr[ 2]=pl_ESDProx1_3;
    ESD_DataPtr[ 3]=pl_ESDProx2_1; ESD_DataPtr[ 4]=pl_ESDProx2_2; ESD_DataPtr[ 5]=pl_ESDProx2_3;
    ESD_DataPtr[ 6]=pl_ESDProx3_1; ESD_DataPtr[ 7]=pl_ESDProx3_2; ESD_DataPtr[ 8]=pl_ESDProx3_3;
    ESD_DataPtr[ 9]=pl_ESDProx4_1; ESD_DataPtr[10]=pl_ESDProx4_2; ESD_DataPtr[11]=pl_ESDProx4_3;
    ESD_DataPtr[12]=pl_ESDProx5_1; ESD_DataPtr[13]=pl_ESDProx5_2; ESD_DataPtr[14]=pl_ESDProx5_3;
    ESD_DataPtr[15]=pl_ESDProx6_1; ESD_DataPtr[16]=pl_ESDProx6_2; ESD_DataPtr[17]=pl_ESDProx6_3;

    ESD_DECAY_DATA_Ptr[ 0]=pl_PositiveDecay1_1; ESD_DECAY_DATA_Ptr[ 1]=pl_PositiveDecay1_2; ESD_DECAY_DATA_Ptr[ 2]=pl_PositiveDecay1_3;
    ESD_DECAY_DATA_Ptr[ 3]=pl_PositiveDecay2_1; ESD_DECAY_DATA_Ptr[ 4]=pl_PositiveDecay2_2; ESD_DECAY_DATA_Ptr[ 5]=pl_PositiveDecay2_3;
    ESD_DECAY_DATA_Ptr[ 6]=pl_PositiveDecay3_1; ESD_DECAY_DATA_Ptr[ 7]=pl_PositiveDecay3_2; ESD_DECAY_DATA_Ptr[ 8]=pl_PositiveDecay3_3;
    ESD_DECAY_DATA_Ptr[ 9]=pl_PositiveDecay4_1; ESD_DECAY_DATA_Ptr[10]=pl_PositiveDecay4_2; ESD_DECAY_DATA_Ptr[11]=pl_PositiveDecay4_3;
    ESD_DECAY_DATA_Ptr[12]=pl_PositiveDecay5_1; ESD_DECAY_DATA_Ptr[13]=pl_PositiveDecay5_2; ESD_DECAY_DATA_Ptr[14]=pl_PositiveDecay5_3;
    ESD_DECAY_DATA_Ptr[15]=pl_PositiveDecay6_1; ESD_DECAY_DATA_Ptr[16]=pl_PositiveDecay6_2; ESD_DECAY_DATA_Ptr[17]=pl_PositiveDecay6_3;

    ESD_DECAY_DATA_Ptr[18]=pl_NegativeDecay1_1; ESD_DECAY_DATA_Ptr[19]=pl_NegativeDecay1_2; ESD_DECAY_DATA_Ptr[20]=pl_NegativeDecay1_3;
    ESD_DECAY_DATA_Ptr[21]=pl_NegativeDecay2_1; ESD_DECAY_DATA_Ptr[22]=pl_NegativeDecay2_2; ESD_DECAY_DATA_Ptr[23]=pl_NegativeDecay2_3;
    ESD_DECAY_DATA_Ptr[24]=pl_NegativeDecay3_1; ESD_DECAY_DATA_Ptr[25]=pl_NegativeDecay3_2; ESD_DECAY_DATA_Ptr[26]=pl_NegativeDecay3_3;
    ESD_DECAY_DATA_Ptr[27]=pl_NegativeDecay4_1; ESD_DECAY_DATA_Ptr[28]=pl_NegativeDecay4_2; ESD_DECAY_DATA_Ptr[29]=pl_NegativeDecay4_3;
    ESD_DECAY_DATA_Ptr[30]=pl_NegativeDecay5_1; ESD_DECAY_DATA_Ptr[31]=pl_NegativeDecay5_2; ESD_DECAY_DATA_Ptr[32]=pl_NegativeDecay5_3;
    ESD_DECAY_DATA_Ptr[33]=pl_NegativeDecay6_1; ESD_DECAY_DATA_Ptr[34]=pl_NegativeDecay6_2; ESD_DECAY_DATA_Ptr[35]=pl_NegativeDecay6_3;

    ATCChPal[ 0]=pan_ATCTempHead01; /* ... 32 total, see golden :193-200 ... */
    ATCReferPtr[ 0]=pl_ATCRefHead01; /* ... 32 total, see golden :202-209 ... */
    ATCPtr[ 0]=pl_ATCTempHead01; /* ... 32 total, see golden :211-218 ... */

    edSocket[0][0]=edtSocketAa; /* ... 32 total, see golden :222-229 ... */
    slASECLTestInfor = new TStringList;

    TPanel *tempTestSiteCBox_Dis[MAX_SOCKET_ROW][MAX_SOCKET_COL]=
    {
        {palAa, palAb, palAc, palAd, palAe, palAf, palAg, palAh},
        {palBa, palBb, palBc, palBd, palBe, palBf, palBg, palBh},
        {palCa, palCb, palCc, palCd, palCe, palCf, palCg, palCh},
        {palDa, palDb, palDc, palDd, palDe, palDf, palDg, palDh}
    };
    TLabel *tempTestLabCol_Dis[MAX_SOCKET_COL]=
    {
        lbSocketIDColA, lbSocketIDColB, lbSocketIDColC, lbSocketIDColD, lbSocketIDColE, lbSocketIDColF, lbSocketIDColG, lbSocketIDColH
    };
    TLabel *tempTestLabRow_Dis[MAX_SOCKET_ROW]={lbSocketIDRowA, lbSocketIDRowB, lbSocketIDRowC, lbSocketIDRowD};

    int iCol, iRow;
    for(int i=0; i<MAX_SOCKET_TOTAL; i++)
    {
        iRow=i/MAX_SOCKET_COL;
        iCol=i%MAX_SOCKET_COL;

        if(i<MAX_SOCKET_ROW)
        {
            SocketLabRow_Display[i]=tempTestLabRow_Dis[i];
            SocketLabRow_Display[i]->Visible=false;
        }

        if(i<MAX_SOCKET_COL)
        {
            SocketLabCol_Display[i]=tempTestLabCol_Dis[i];
            SocketLabCol_Display[i]->Visible=false;
        }
        SocketSiteCH_Display[iRow][iCol]=tempTestSiteCBox_Dis[iRow][iCol];
        SocketSiteCH_Display[iRow][iCol]->Visible=false;
    }
#endif

    TimerERMS->Enabled=true;
    tsChamberBoost->TabVisible=false;
    btnCancelTestPause->Enabled=false;
    btnESCFunction->Enabled=(IniConfig.bI41_6_Manual);
    if(IniConfig.bSIGURDFunction)                                               //Sam 20210401 : 俊堯要求只要顯示 FTP
        ts_FTPAutomation->Caption="FTP";
    if(ATC_SYSTEM==eWinWay && Temperature.bATCActiveCooling==true) ATC_WinWay->TabVisible=true;
    else ATC_WinWay->TabVisible=false;
    ATCPtrWinWay[0]=pnlWinwayPVCH1;ATCPtrWinWay[1]=pnlWinwayPVCH2;ATCPtrWinWay[2]=pnlWinwayPVCH3;ATCPtrWinWay[3]=pnlWinwayPVCH4;

    bNeedToDeleteFile=false;
    iWaitRtcDeleteTask=0;
    bEventLogAlarm=false;
    ZeroMemory(fTempUserOffset,sizeof(fTempUserOffset));
    ZeroMemory(fContactHeight, sizeof(fContactHeight));
    ZeroMemory(fTempATCOffset, sizeof(fTempATCOffset));
    ZeroMemory(iShuttleMode, sizeof(iShuttleMode));
    ZeroMemory(bART, sizeof(bART));
    iART=0;
    iIndexHeatingMode=0;
    iProduceTimeCT=0;                                                           //jou 20221125 : 機台添加三小時送檢報警，從lot start時間開始計算

    // AI(W906-FW3-LotInfo-WC) 20260819: Wave C ADD -- FormShow/Timer2Timer
    // widget allocations. See forms/fLotInfo.h's WC banner for WAVE SCOPE /
    // GATE REGISTER; grouped in the same order as the header declarations.
    tsPATSetUp        = new TTabSheet();
    tsBundle          = new TTabSheet();
    tsSetupFileCheck  = new TTabSheet();
    tsESDMonitor      = new TTabSheet();
    tsBarCode         = new TTabSheet();
    ts_OCRInterface   = new TTabSheet();
    ts_SocketInterface= new TTabSheet();
    tsKYEC_AMR        = new TTabSheet();
    tsRTCFullViewImg  = new TTabSheet();

    grpRFID           = new TGroupBox();
    ts_AutoCleanMonitor = new TTabSheet();
    Label5            = new TLabel();
    btDownload        = new TButton();
    btnFtpTester      = new TButton();
    Label153          = new TLabel();
    FileListBox1      = new vclcompat::TFileListBox();
    cbRTCASTD         = new TCheckBox();

    ART_Panel         = new TfLotInfoLayoutPanel();
    GroupBox3         = new TfLotInfoLayoutGroupBox();

    btnDataFTPSaveToData = new TButton();
    lblOPID           = new TLabel();
    spSECSLotCheck    = new TSpeedButton();
    Panel6            = new TfLotInfoLayoutPanel();

    tsTPW             = new TTabSheet();
    tsSigurd          = new TTabSheet();

    labLevelMode      = new TLabel();
    coLevelMode       = new TComboBox();

    sgBarcode         = new TStringGrid(6, 7);        // dfm: ColCount=6 RowCount=7
    sgOCR             = new TStringGrid(3, 5);        // dfm: ColCount=3, RowCount default(5)
    sgATRCount        = new TStringGrid(3, 4);        // dfm: ColCount=3 RowCount=4

    pl_ATC_Online     = new TPanel();

    labDeviceName     = new TLabel();
    lbLotRunMode      = new TLabel();
    labLotID          = new TLabel();
    btnFtpServer      = new TButton();
    btnFtpHD          = new TButton();
    sb_RunExecutFile  = new TSpeedButton();

    edtLine           = new TLabeledEdit();
    edtProcessName    = new TfLotInfoLayoutLabeledEdit();
    edtProduct        = new TfLotInfoLayoutLabeledEdit();
    lbProcess         = new TLabel();
    labConfigL04      = new TLabel();
    grpOEEState       = new TfLotInfoLayoutGroupBox();
    sgOEEState        = new TStringGrid(2, 10);       // dfm: ColCount=2 RowCount=10
    labCusDevGrp      = new TLabel();
    labCusStep        = new TLabel();

    btnFTPTryConnect  = new TButton();
    lbFTPStatus       = new TLabel();

    tsASEMARMS        = new TTabSheet();
    pnlLotInfo_ASECL  = new TPanel();
    pnlLotStart_ASECL = new TPanel();

    lblPage           = new TLabel();
    labJobSeq         = new TLabel();
    edtJobSeq         = new TEdit();
    labQACount        = new TLabel();
    edQAMode          = new TfLotInfoLayoutEdit();
    btnQAmodeSave     = new TfLotInfoLayoutPushButton();
    palQAMode         = new TPanel();

    pan_DewPoint      = new TPanel();
    pl_DewPoint       = new TPanel();

    grpBarcodeDisplayLotInfo = new TGroupBox();

    btChangeFile      = new TButton();

    btnClearTemperature = new TButton();
    lbShowDevName     = new TLabel();

    lbLotAQLSetCount  = new TLabel();
    lbLotAQLSetBin    = new TLabel();

    tsDeviceInfo      = new TTabSheet();
    palSecsGem        = new TPanel();
    cbPATMode         = new TComboBox();

    palCurrFailRate   = new TPanel();
    Panel27           = new TPanel();
    LotKeyInTime      = new TfLotInfoTimer();
    btnSaveData       = new TSpeedButton();

    labLoaderBundleID     = new TLabel();
    lblLoaderCarBundleID  = new TLabel();
    lbOCRNowFile      = new TLabel();
    Label41           = new TLabel();
    palHandlerwithTester = new TPanel();
    lblTester_LotID   = new TLabel();
    lblAutoCount      = new TLabel();
    lblAutoCount2     = new TLabel();
    lblAutoCount3     = new TLabel();
    labBarcodeRecipe  = new TLabel();
    edtBarcodeRecipe  = new TEdit();
    spOCRCleanList    = new TSpeedButton();
    tsTesterLog       = new TTabSheet();
    ts_AutoRetestMonitor = new TTabSheet();
    tsOtherTool       = new TTabSheet();
    palAQLMode        = new TPanel();
}
// AI(W906-AutoCleanFoundation) 20260721: golden uLotInfo.cpp:16250-16253 --
// REAL one-line body (was a total no-op stub before that wave). See
// forms/fLotInfo.h's iUnloaderTask/InitialUnLoaderTask member comments for the
// behaviour-change + dormant-call-site (SOFT_SIMULTE undefined) analysis.
void TfLotInfo::InitialUnLoaderTask(int iPos) { iUnloaderTask[iPos]=1; }
// -- W5-Automation ADD: AMR.cpp + HANA_ART.cpp method sinks --
// AI(W906-FW3-LotInfo-WB) 20260819: RefreshAMR was a total no-op stub before
// this wave (golden uLotInfo.cpp:15816-15844 is RECON Tier 2 item, never
// translated). Given a REAL body below (see Wave B method bodies section);
// this comment stays here as the historical marker for where the stub used
// to live.
//AI(W906-LOT-W1) 20260919: 這裡原本是
//     void TfLotInfo::SetLotID(AnsiString, bool) {}     // offline no-op
// 一個**會說謊的樁** —— 它讓 forms/fLotInfo.cpp:4036（FormShow）與
// Automation/auto9045.cpp:1484 的呼叫點看起來完成，實際什麼都沒做，
// 而 `nm --undefined-only` 看不見它（docs/KNOWLEDGE.md archive-extraction
// 陷阱形式 (2)）。真本體在本檔尾端的 W906-LOT-W1 區塊。
//AI(W906-LOT-W1) 20260919: 這裡原本是
//     void TfLotInfo::SetLotStart(AnsiString, bool) {}  // offline no-op
// 第二個**會說謊的樁**（第一個是同一行上面的 SetLotID）。
// 真本體在本檔尾端的 W906-LOT-W1 區塊。

// ===========================================================================
//  AI(W906-FW3-LotInfo-WA) 20260819: Wave A method bodies. See forms/
//  fLotInfo.h's file banner for WAVE SCOPE / GATE REGISTER / DEVIATION.
// ===========================================================================

// -- RefreshYieldMonitor_SIGURD (golden uLotInfo.cpp:13327-13545) -----------
void TfLotInfo::RefreshYieldMonitor_SIGURD()
{
    AnsiString sPathName,sCheckListName,asLastOpenFN,asString;
    int iCheck,iBin;
    bool bCheck;
    double dCheck;
    bool bFailAlarmLowYieldByTotal,bContsFailBySocket,bContsFailByHead;
    int iLowYieldCountByTotal, iContsFailSocketAlarmCT, iContsFailHeadAlarmCT, iFailAlarmSiteYieldCmpCount;
    double dFailAlarmSiteYieldCmp,dLowYieldLimitByTotal;

    gbManualCheckList->Visible=(AccessLevel>=iDefHonPrecLevel)?true:false;
    btnManualStandard->Visible=(CUSTOMER_CODE==CC_UTAC_TW || CUSTOMER_CODE==CC_SIGURD_PeiXing);
    AdjtsYieldMonitiorSize();

    asLastOpenFN=GetLastOpenFN();
    sPathName.sprintf("D:\\HT9045_Log\\CheckingList");
    sCheckListName.sprintf("%s\\%s.txt", sPathName, asLastOpenFN);

    cbMonitor_FTPRMS->Checked=IniConfig.bA32EnableFTPAutomation;
    // AI(W906-FW3-LotInfo-WA) 20260819: GATE WA-1 -- golden `cbContactMode->
    // ItemIndex=fContact->cbContactMode->ItemIndex;` (uLotInfo.cpp:13346).
    // `fContact` (a golden TfContact*, distinct from the real fContactCT/
    // TfContactCT this tree already ports) has no port anywhere in this tree.
    // See forms/fLotInfo.h GATE REGISTER WA-1.
#if 0
    cbContactMode->ItemIndex=fContact->cbContactMode->ItemIndex;
#endif

    if(iTestRunMode==RT)
    {
        bFailAlarmLowYieldByTotal   =TestIF_File.bFailAlarmLowYieldByTotal_RT;
        bContsFailBySocket          =TestIF_File.bContsFailBySocket_RT;
        bContsFailByHead            =TestIF_File.bContsFailByHead_RT;
        iLowYieldCountByTotal       =TestIF_File.iLowYieldCountByTotal_RT;
        dFailAlarmSiteYieldCmp      =TestIF_File.dFailAlarmSiteYieldCmp_RT;
        dLowYieldLimitByTotal       =TestIF_File.dLowYieldLimitByTotal_RT;
        iContsFailSocketAlarmCT     =TestIF_File.iContsFailSocketAlarmCT_RT;
        iContsFailHeadAlarmCT       =TestIF_File.iContsFailHeadAlarmCT_RT;
        iFailAlarmSiteYieldCmpCount =TestIF_File.iFailAlarmSiteYieldCmpCount_RT;
    }
    else
    {
        bFailAlarmLowYieldByTotal   =TestIF_File.bFailAlarmLowYieldByTotal;
        bContsFailBySocket          =TestIF_File.bContsFailBySocket;
        bContsFailByHead            =TestIF_File.bContsFailByHead;
        iLowYieldCountByTotal       =TestIF_File.iLowYieldCountByTotal;
        dFailAlarmSiteYieldCmp      =TestIF_File.dFailAlarmSiteYieldCmp;
        dLowYieldLimitByTotal       =TestIF_File.dLowYieldLimitByTotal;
        iContsFailSocketAlarmCT     =TestIF_File.iContsFailSocketAlarmCT;
        iContsFailHeadAlarmCT       =TestIF_File.iContsFailHeadAlarmCT;
        iFailAlarmSiteYieldCmpCount =TestIF_File.iFailAlarmSiteYieldCmpCount;
    }

    bCheck=ReadIniData(sCheckListName, "FT_Yield",    "Yield Func",     bFailAlarmLowYieldByTotal);
    cbSiteYieldCmp_FT->Checked=bCheck;
    cbLowYieldByTotal_FT->Checked=bCheck;
    if(bCheck!=bFailAlarmLowYieldByTotal && iTestRunMode==FT)
    {
        edSiteYieldCmpOnOff_Cur->Visible=true;
        edLowYieldByTotalOnOff_Cur->Visible=true;
        edSiteYieldCmpOnOff_Cur->Text=(bFailAlarmLowYieldByTotal)?"On":"OFF";
        edLowYieldByTotalOnOff_Cur->Text=(bFailAlarmLowYieldByTotal)?"On":"OFF";
    }
    else
    {
        edSiteYieldCmpOnOff_Cur->Visible=false;
        edLowYieldByTotalOnOff_Cur->Visible=false;
    }

    iCheck=ReadIniData(sCheckListName, "FT_Yield",    "Preset",          iLowYieldCountByTotal);
    edSiteYieldCmpIg_FT->Text=iCheck;
    edLowYieldByTotalIg_FT->Text=iCheck;
    if((iCheck!=iLowYieldCountByTotal ||
        iCheck!=iFailAlarmSiteYieldCmpCount) &&
       iTestRunMode==FT)
    {
        edSiteYieldCmpIg_Cur->Visible=true;
        edLowYieldByTotalIg_Cur->Visible=true;
        edSiteYieldCmpIg_Cur->Text=iFailAlarmSiteYieldCmpCount;
        edLowYieldByTotalIg_Cur->Text=iLowYieldCountByTotal;
        lblSiteYieldCmpIg_Cur->Visible=true;
        lblLowYieldByTotalIg_Cur->Visible=true;
    }
    else
    {
        edSiteYieldCmpIg_Cur->Visible=false;
        edLowYieldByTotalIg_Cur->Visible=false;
        lblSiteYieldCmpIg_Cur->Visible=false;
        lblLowYieldByTotalIg_Cur->Visible=false;
    }

    dCheck=ReadIniData(sCheckListName, "FT_Yield", "Variance",        dFailAlarmSiteYieldCmp);
    edSiteYieldCmp_FT->Text=dCheck;
    if(dCheck!=dFailAlarmSiteYieldCmp && iTestRunMode==FT)
    {
        edSiteYieldCmp_Cur->Visible=true;
        edSiteYieldCmp_Cur->Text=dFailAlarmSiteYieldCmp;
        lblSiteYieldCmp_Cur->Visible=true;
    }
    else
    {
        edSiteYieldCmp_Cur->Visible=false;
        lblSiteYieldCmp_Cur->Visible=false;
    }

    dCheck=ReadIniData(sCheckListName, "FT_Yield", "Low Yield",      dLowYieldLimitByTotal);
    edLowYieldByTotal_FT->Text=dCheck;
    if(dCheck!=dLowYieldLimitByTotal && iTestRunMode==FT)
    {
        edLowYieldByTotal_Cur->Visible=true;
        edLowYieldByTotal_Cur->Text=dLowYieldLimitByTotal;
        lblLowYieldByTotal_Cur->Visible=true;
    }
    else
    {
        edLowYieldByTotal_Cur->Visible=false;
        lblLowYieldByTotal_Cur->Visible=false;
    }

    bCheck=ReadIniData(sCheckListName, "FT_Yield",    "ContsFailBySocket Func",     bContsFailBySocket);
    rbContsFailBySocket_FTOn ->Checked=bCheck;
    rbContsFailBySocket_FTOff->Checked=!bCheck;

    if(bCheck!=bContsFailBySocket && iTestRunMode==FT)
    {
        edtContsFailBySocket_Cur->Visible=true;
        edtContsFailBySocket_Cur->Text=(bContsFailBySocket)?"On":"OFF";
    }
    else
    {
        edtContsFailBySocket_Cur->Visible=false;
    }

    iCheck=ReadIniData(sCheckListName, "Alarm", "Socket",   (int)iContsFailSocketAlarmCT);
    edContsFailSocketAlarmCT_FT->Text=iCheck;
    if(iCheck!=(int)iContsFailSocketAlarmCT && iTestRunMode==FT)
    {
        edContsFailSocketAlarmCT_Cur->Visible=true;
        edContsFailSocketAlarmCT_Cur->Text=iContsFailSocketAlarmCT;
    }
    else
    {
        edContsFailSocketAlarmCT_Cur->Visible=false;
    }
    lblContsFailSocketAlarmCT_Cur->Visible=(edtContsFailBySocket_Cur->Visible || edContsFailSocketAlarmCT_Cur->Visible)?true:false;

    bCheck=ReadIniData(sCheckListName, "FT_Yield",    "ContsFailByHead Func",     bContsFailByHead);
    rbContsFailByHead_FTOn ->Checked=bCheck;
    rbContsFailByHead_FTOff->Checked=!bCheck;
    if(bCheck!=bContsFailByHead && iTestRunMode==FT)
    {
        edtContsFailByHead_Cur->Visible=true;
        edtContsFailByHead_Cur->Text=(bContsFailByHead)?"On":"OFF";
    }
    else
    {
        edtContsFailByHead_Cur->Visible=false;
    }

    iCheck=ReadIniData(sCheckListName, "Alarm", "Head",     (int)TestIF_File.iContsFailHeadAlarmCT);
    edContsFailHeadAlarmCT_FT->Text=iCheck;
    if(iCheck!=(int)iContsFailHeadAlarmCT && iTestRunMode==FT)
    {
        edContsFailHeadAlarmCT_Cur->Visible=true;
        edContsFailHeadAlarmCT_Cur->Text=TestIF_File.iContsFailHeadAlarmCT;
    }
    else
    {
        edContsFailHeadAlarmCT_Cur->Visible=false;
    }

    lblContsFailHeadAlarmCT_Cur->Visible=(edtContsFailByHead_Cur->Visible || edContsFailHeadAlarmCT_Cur->Visible)?true:false;

    iBin=ReadIniData(sCheckListName, "Tester_Control", "SGOSBIN",   0);
    if(iBin>0)
    {
        edOSBin->Text=iBin;
        asString.sprintf("Category%d", iBin);
        iCheck=ReadIniData(sCheckListName, asString, "Fail Percent Ignore",   BinSelect[iTestRunMode].iPersentIgnore[iBin]);
        edOSBinCnt->Text=iCheck;
        if(iCheck!=BinSelect[iTestRunMode].iPersentIgnore[iBin])
        {
            edOSBinCnt_Cur->Text=BinSelect[iTestRunMode].iPersentIgnore[iBin];
            edOSBinCnt_Cur->Visible=true;
        }
        else
        {
            edOSBinCnt_Cur->Visible=false;
        }

        dCheck=ReadIniData(sCheckListName, asString, "Fail Percent Num",   BinSelect[iTestRunMode].dFailureLimit[iBin]);
        edOSBinPreset->Text=dCheck;
        if(dCheck!=BinSelect[iTestRunMode].dFailureLimit[iBin])
        {
            edOSBinPreset_Cur->Text=BinSelect[iTestRunMode].dFailureLimit[iBin];
            edOSBinPreset_Cur->Visible=true;
        }
        else
        {
            edOSBinPreset_Cur->Visible=false;
        }
        lblOSBin_Cur->Visible=(edOSBinCnt_Cur->Visible || edOSBinPreset_Cur->Visible)?true:false;
    }
    else
    {
        edOSBin->Text="Null";
        edOSBinCnt->Text="Null";
        edOSBinPreset->Text="Null";
    }

    if(CUSTOMER_CODE==CC_SIGURD_HUKOU)
        grpOSBin->Visible=false;
    else
        grpOSBin->Visible=true;

    AdjtsYieldMonitiorSize();
}

// -- TransformTemperature_AirStream (golden uLotInfo.cpp:15080-15193) -------
double TfLotInfo::TransformTemperature_AirStream(double Offset,int iIndex)
{
    if(AirStream_Select==0)
        return 0.0;

    TIniFile *tIni = new TIniFile("D:\\HT9045\\config\\AirStream.ini");

    AnsiString sTemp_Array[16]={"-60~-51","-50~-41","-40~-21","-20~-11","-10~0"  ,"1~15"   ,"16~25"  ,"26~50",
                                "51~80"  ,"81~90"  ,"91~105" ,"106~120","121~130","131~145","146~160","161~175"};

    AnsiString sLoadIni="";
    double dbSetTemp=atof(fMain->edWorkTemperBase->Text.c_str());
    double dRseult=dbSetTemp;

    int iTemp=-1;
    if(-60<=dbSetTemp && dbSetTemp<=-51)
        iTemp=0;
    else if(-50<=dbSetTemp && dbSetTemp<=-41)
        iTemp=1;
    else if(-40<=dbSetTemp && dbSetTemp<=-21)
        iTemp=2;
    else if(-20<=dbSetTemp && dbSetTemp<=-11)
        iTemp=3;
    else if(-10<=dbSetTemp && dbSetTemp<=0)
        iTemp=4;
    else if(1<=dbSetTemp && dbSetTemp<=15)
        iTemp=5;
    else if(16<=dbSetTemp && dbSetTemp<=25)
        iTemp=6;
    else if(26<=dbSetTemp && dbSetTemp<=50)
        iTemp=7;
    else if(51<=dbSetTemp && dbSetTemp<=80)
        iTemp=8;
    else if(81<=dbSetTemp && dbSetTemp<=90)
        iTemp=9;
    else if(91<=dbSetTemp && dbSetTemp<=105)
        iTemp=10;
    else if(106<=dbSetTemp && dbSetTemp<=120)
        iTemp=11;
    else if(121<=dbSetTemp && dbSetTemp<=130)
        iTemp=12;
    else if(131<=dbSetTemp && dbSetTemp<=145)
        iTemp=13;
    else if(146<=dbSetTemp && dbSetTemp<=160)
        iTemp=14;
    else if(161<=dbSetTemp && dbSetTemp<=175)
        iTemp=15;

    if(iTemp==-1)
    {
        delete tIni;
        return dRseult;
    }

    // AI(W906-FW3-LotInfo-WA) 20260819: GATE WA-2 -- the lazy-init default-
    // table write to D:\HT9045\config\AirStream.ini. See forms/fLotInfo.h
    // GATE REGISTER WA-2 (config\ is a shared production-machine runtime
    // parameter directory, default read-only per AGENTS.md/CLAUDE.md).
#if 0
    if(!tIni->ValueExists("Temp_Index", "-60~-51"))
    {
        tIni->WriteString("Temp_Index", "-60~-51", "-70");
        tIni->WriteString("Temp_Index", "-50~-41", "-70");
        tIni->WriteString("Temp_Index", "-40~-21", "-60");
        tIni->WriteString("Temp_Index", "-20~-11", "-60");
        tIni->WriteString("Temp_Index", "-10~0"  , "-50");
        tIni->WriteString("Temp_Index", "1~15"   , "-50");
        tIni->WriteString("Temp_Index", "16~25"  , "-50");
        tIni->WriteString("Temp_Index", "26~50"  , "25");
        tIni->WriteString("Temp_Index", "51~80"  , "30");
        tIni->WriteString("Temp_Index", "81~90"  , "30");
        tIni->WriteString("Temp_Index", "91~105" , "35");
        tIni->WriteString("Temp_Index", "106~120", "35");
        tIni->WriteString("Temp_Index", "121~130", "35");
        tIni->WriteString("Temp_Index", "131~145", "35");
        tIni->WriteString("Temp_Index", "146~160", "35");
        tIni->WriteString("Temp_Index", "161~175", "35");

        tIni->WriteString("Temp_Socket","-60~-51", "-70");
        tIni->WriteString("Temp_Socket","-50~-41", "-70");
        tIni->WriteString("Temp_Socket","-40~-21", "-50");
        tIni->WriteString("Temp_Socket","-20~-11", "-30");
        tIni->WriteString("Temp_Socket","-10~0"  , "-10");
        tIni->WriteString("Temp_Socket","1~15"   , "-10");
        tIni->WriteString("Temp_Socket","16~25"  , "10");
        tIni->WriteString("Temp_Socket","26~50"  , "25");
        tIni->WriteString("Temp_Socket","51~80"  , "90");
        tIni->WriteString("Temp_Socket","81~90"  , "100");
        tIni->WriteString("Temp_Socket","91~105" , "120");
        tIni->WriteString("Temp_Socket","106~120", "140");
        tIni->WriteString("Temp_Socket","121~130", "145");
        tIni->WriteString("Temp_Socket","131~145", "160");
        tIni->WriteString("Temp_Socket","146~160", "190");
        tIni->WriteString("Temp_Socket","161~175", "215");
    }
#endif

    dRseult=0;
    if(iIndex==0)
    {
        sLoadIni=tIni->ReadString("Temp_Index",sTemp_Array[iTemp],sLoadIni);
        dRseult= Offset+atof(sLoadIni.c_str());
        if(dRseult>35)
            dRseult=35;
    }
    else
    {
        sLoadIni=tIni->ReadString("Temp_Socket",sTemp_Array[iTemp],sLoadIni);
        dRseult = Offset+atof(sLoadIni.c_str());
    }

    if(dRseult<-70)
        dRseult=-70;

    if(dRseult>230)
        dRseult=230;

    delete tIni;
    return dRseult;
}

// -- AdjtsYieldMonitiorSize (golden uLotInfo.cpp:13636-13717) ----------------
void TfLotInfo::AdjtsYieldMonitiorSize()
{
    if(pgLotinfo->ActivePage==ts_ATC6_1)
    {
        Height=610;
        Width=440;
    }
    else if(pgLotinfo->ActivePage==tsATC)
    {
        Height=610;
        Width=580;
    }
    else if(pgLotinfo->ActivePage==tsASECLEventLog)
    {
        Width=671;
    }
    else if(CUSTOMER_CODE==CC_PANTHER)
    {
        Width=671;
    }
    else if(pgLotinfo->ActivePage==tsYieldMonitior &&
            IniConfig.bSIGURDFunction)
    {
        Width=530;
        if(grpOSBin->Visible && gbManualCheckList->Visible)
            Height=605;
        else if(grpOSBin->Visible && gbManualCheckList->Visible==false)
            Height=550;
        else
            Height=460;
        // AI(W906-FW-LOTINFO-W30) 20260826: GATE WA-3 RETIRED (golden :13671).
        // Its stated reason -- "RefreshYieldMonitor is RECON #115 (b) write-path
        // and not one of this wave's Tier-1 methods" -- died twice over when
        // FW-LOTINFO-W27 landed that method for real (this file:4396) and
        // OVERTURNED the (b) classification: the dispatcher writes nothing, and
        // the one real write it reaches (fCleaning->ChangeACSmartInterval, golden
        // :13618) is now gated one frame deeper at WD-2 (this file:4493), where
        // it can actually be seen.  RE-AUDITED THE WHOLE REACHABLE CHAIN 20260826
        // before opening: RefreshYieldMonitor_SIGURD (this file:679-895) touches
        // files only through ReadIniData, which common.cpp:684-686 documents as
        // a pure read (and TIniFile::UpdateFile is a no-op under writeThrough_,
        // vclcompat/IniFiles.cpp:281-286, so CloseIniFile cannot rewrite the
        // previous ini either); RefreshYieldMonitor_TERAPOWER is widget fill;
        // AdjtsYieldMonitiorSize is widget geometry.  No file write, no motion,
        // no outward command.  The mutual recursion with this very function is
        // golden's own and terminates at depth 2 on the static bTimerRunning
        // guard (this file:4402-4404).
        RefreshYieldMonitor();
    }
    else if(pgLotinfo->ActivePage==tsYieldMonitior &&
            CosFunction.bShowYieldMonitor)
    {
        Width=530;
        Height=601;
        RefreshYieldMonitor();                                                  // AI(W906-FW-LOTINFO-W30) 20260826: GATE WA-3 RETIRED, 2nd site (golden :13678) -- see 1st site above
    }
    else if(CosFunction.bEnableHandlerResultServer && pgLotinfo->ActivePage==tsAMR)
    {
        Width=420;
        RefreshAMR();
    }
    else if(pgLotinfo->ActivePage==tsFTP)
    {
        Width=340;
    }
    else if(pgLotinfo->ActivePage==tsLotID && USE_RFID_READER)
    {
        Height=666;
    }
    else if(pgLotinfo->ActivePage==tsLotID && tsMurata->TabVisible==false  &&
            tsSigurd_CX->TabVisible==false && tsSPIL_SZ->TabVisible==false &&
            tsOEE->TabVisible==false       && ts2DSort->TabVisible==false  &&
            tsChipAdv->TabVisible==false   && tsVTest->TabVisible==false)
    {
        pgcLotInfo->Visible=false;
        Width=410;
    }
    else if(pgLotinfo->ActivePage==tsOCRBarCode)
    {
        Height=610;
        Width=530;
    }
    else if(IniConfig.bVTESTFunction==true)
    {
        Height=470;
        Width=620;
    }
    else
    {
        Height=490;
        Width=671;
    }
}

// -- SetSelectionVisible (golden uLotInfo.cpp:1262-1342) ---------------------
void TfLotInfo::SetSelectionVisible()
{
    if(IniConfig.bVTESTFunction==true)
    {
        if((IniConfig.bEnableRms==true || IniConfig.bEnableFTP==true) &&
            AccessLevel>=iDefEngineerLevel)
        {
            if(AccessLevel>=iDefSupervisorLevel)
            {
                groupbDownloadItem->Visible=true;
                grpMesCheck->Visible=true;
            }
            else
            {
                groupbDownloadItem->Visible=false;
                grpMesCheck->Visible=false;
            }
            tsSelection->TabVisible=true;
        }
        else
        {
            tsSelection->TabVisible=false;
        }
        Panel28->Width=285;
        cbTestTimes->Visible=true;
        lblTestTimes->Visible=true;
    }
    else
    {
        if(CUSTOMER_CODE==CC_KYEC_LEE)
        {
            tsSelection->TabVisible=false;
        }
        else
        {
            if(IniConfig.bEnableRms==true ||
               IniConfig.bEnableErms==true ||
               IniConfig.bEnableFTP==true ||
               IniConfig.bSPILFunction)
                tsSelection->TabVisible=true;
            else
                tsSelection->TabVisible=false;
        }
    }
    if(IniConfig.bA75DownloadItemByAccessLevel==true)
    {
        bool bDenyByOP=(AccessLevel==0);
        // AI(W906-FW3-LotInfo-WA) 20260819: GATE WA-4 -- the child-control
        // enable/disable loop. See forms/fLotInfo.h GATE REGISTER WA-4
        // (vclcompat::TGroupBox has no Controls[]/ControlCount surface;
        // adding one is a shared-header change outside this wave's write
        // boundary).
#if 0
        for(int i=0; i<groupbDownloadItem->ControlCount; i++)
        {
            if(groupbDownloadItem->Controls[i]!=lblDownloadAccessWarning)
                groupbDownloadItem->Controls[i]->Enabled=!bDenyByOP;
        }
#endif
        lblDownloadAccessWarning->Visible=bDenyByOP;
    }
    else
    {
        lblDownloadAccessWarning->Visible=false;
    }
}

// -- ShowSocketID (golden uLotInfo.cpp:11443-11516) --------------------------
void TfLotInfo::ShowSocketID()
{
    int iMode=TestIF_File.iTestMode;
    int iTestCHCT=SiteData[iMode].Cnt;

    if(IniConfig.bDualSiteSupply4CH==true)
    {
        if(TestIF_File.iTestMode==DualSite)
        {
            iTestCHCT+=2;
        }
    }

    if(CUSTOMER_CODE==CC_ASE_KaohSiung)
    {
        if((TestIF_File.iTestMode==_8Site2X4 ||
            TestIF_File.iTestMode==_16Site4X4) &&
            CosFunction.bEnableOctal_12Kit==true)
        {
            if(TestIF_File.bOctal_12Kit==true)
                iTestCHCT+=4;
            else
                iTestCHCT=SiteData[iMode].Cnt;
        }
    }

    if(CosFunction.bUse32ChanelSiteMap)
    {
        iTestCHCT=32;

        if(IniConfig.bVTESTFunction)
        {
            iTestCHCT=16;
        }
    }
    // GOLDEN ODDITY (recorded, not "fixed"): golden computes iTestCHCT through
    // 3 customer-specific override branches above but never reads it again
    // anywhere in this function body (verified against the full golden span,
    // uLotInfo.cpp:11443-11516) -- the loops below key off SiteData[iMode].
    // XItem/YItem instead. Translated literally; `(void)` silences the
    // resulting -Wunused-but-set-variable this wave's -Wall build would
    // otherwise raise on a real dead local.
    (void)iTestCHCT;

    for(int i=0; i<MAX_SOCKET_ROW; i++)
    {
        SocketLabRow_Display[i]->Visible=false;
        for(int j=0; j<MAX_SOCKET_COL; j++)
        {
            SocketSiteCH_Display[i][j]->Caption="";
            SocketSiteCH_Display[i][j]->Visible=false;
            SocketLabCol_Display[j]->Visible=false;
            edSocket[i][j]->Visible=false;
        }
    }

    for(int i=0; i<SiteData[iMode].XItem; i++)
    {
        SocketLabCol_Display[i]->Visible=true;
        for(int j=0; j<SiteData[iMode].YItem; j++)
        {
            SocketSiteCH_Display[j][i]->Visible=true;
            SocketLabRow_Display[j]->Visible=true;
            edSocket[j][i]->Visible=true;
        }
    }

    for(int i=0; i<MAX_SOCKET_ROW; i++)
    {
        for(int j=0; j<MAX_SOCKET_COL; j++)
        {
            if(TestIF_File.iSiteMap[i][j]==0)
            {
                SocketSiteCH_Display[i][j]->Caption="";
            }
            else
            {
                SocketSiteCH_Display[i][j]->Caption=("CH "+AnsiString (TestIF_File.iSiteMap[i][j]));
            }
        }
    }
}

// -- CheckAirMachineStatus (golden uLotInfo.cpp:14805-14868) -- WA-6 --------
void TfLotInfo::CheckAirMachineStatus()
{
    if(AirStream_Select==0)
        return;
    // AI(W906-FW3-LotInfo-WA) 20260819: GATE WA-6 -- the entire remaining
    // body. See forms/fLotInfo.h GATE REGISTER WA-6: (a) every line below
    // dereferences ATC_InterfaceForm->AirMachineInfo/.AirMachineInfo_Index/
    // .IsConnect(), none of which exist on the acarry_shims.h
    // TATC_InterfaceFormShim (which carries only iATC_MODE_TYPE); (b) the
    // SystemStart-gated sub-block also raises a real WAR1611 alarm via
    // ShowErrorMessage, the same SAFETY-classified category as forms/
    // fTemperFrom.h's own GATE (T1). RECON's "verified fully" call on this
    // function (docs/RECON_uLotInfo_displayside.md row #156) was wrong on
    // both counts -- see this wave's final report.
#if 0
    palAirMachineStatus              ->Caption = ATC_InterfaceForm->AirMachineInfo.asATC_AirMachineStatus;
    palAirMachineSetTemperature      ->Caption = ATC_InterfaceForm->AirMachineInfo.asATC_AirMachineSetTemp;
    palAirMachineAlarmStatus         ->Caption = ATC_InterfaceForm->AirMachineInfo.asATC_AirMachineAlarm;
    palAirDefrostSec                 ->Caption = ATC_InterfaceForm->AirMachineInfo.iDefrostSec;
    pal_Air_Machine_Temp_Ch1         ->Caption = AnsiString(ATC_InterfaceForm->AirMachineInfo.iATC_AirMachineSocketChTemp[0]);
    pal_Air_Machine_Temp_Ch2         ->Caption = AnsiString(ATC_InterfaceForm->AirMachineInfo.iATC_AirMachineSocketChTemp[1]);
    pal_AirStream_AirVolume          ->Caption = AnsiString(ATC_InterfaceForm->AirMachineInfo.iATC_AirVolume);

    if(ATC_InterfaceForm->iATC_MODE_TYPE==61)
    {
        palAirMachineStatus_Index        ->Caption = ATC_InterfaceForm->AirMachineInfo_Index.asATC_AirMachineStatus;
        palAirMachineSetTemperature_Index->Caption = ATC_InterfaceForm->AirMachineInfo_Index.asATC_AirMachineNowTemp;
        palAirMachineAlarmStatus_Index   ->Caption = ATC_InterfaceForm->AirMachineInfo_Index.asATC_AirMachineAlarm;
        palAirDefrostSec_Index           ->Caption = AnsiString(ATC_InterfaceForm->AirMachineInfo_Index.iDefrostSec);
        pnl_AirMachineTemp_Index_Ch_1    ->Caption = AnsiString(ATC_InterfaceForm->AirMachineInfo_Index.iATC_AirMachineSocketChTemp[0]);
        pnl_AirMachineTemp_Index_Ch_2    ->Caption = AnsiString(ATC_InterfaceForm->AirMachineInfo_Index.iATC_AirMachineSocketChTemp[1]);
        pal_AirStream_AirVolume_Index    ->Caption = AnsiString(ATC_InterfaceForm->AirMachineInfo_Index.iATC_AirVolume);
    }

    if(bUT150Install[tcATCHotAir1]==true || bUT150Install[tcATCHotAir2]==true)
    {
        if(ATC_InterfaceForm->IsConnect())
        {
            UN150Read[tcATCHotAir1]=ATC_InterfaceForm->AirMachineInfo_Index.iATC_AirMachineSocketChTemp[1];
            UN150Read[tcATCHotAir2]=ATC_InterfaceForm->AirMachineInfo.iATC_AirMachineSocketChTemp[1];
        }
        else
        {
            UN150Read[tcATCHotAir1]=999;
            UN150Read[tcATCHotAir2]=999;
        }
    }

    if(SystemStart)
    {
        if(Temperature.EnableAirMachineSocket)
        {
            if(palAirMachineStatus->Caption != "Run" && palAirMachineStatus->Caption != "NoUse")
            {
                if(LastSet.iLanguageCountry==1)
                    ShowMyMessage("冷風機正在執行自動除霜，請等待除霜完成再操作");
                else
                    ShowMyMessage("The air cooler is performing automatic defrosting, \n please wait until it is completed before using it");
            }
            else if(palAirMachineStatus->Caption != "Run" && palAirMachineStatus->Caption != "NoUse")
            {
                if(bManualAirCoolingOnOff ==false)
                    ShowErrorMessage("WAR1611", K_RETRY, MMSystem);
            }
        }
        else
        {
            if(palAirMachineStatus->Caption == "0")
            {
                ShowErrorMessage("WAR1611", K_RETRY, MMSystem);
            }
        }
    }
#endif
}

// -- InitialRefrigerantSystem (golden uLotInfo.cpp:14877-14939) -------------
void TfLotInfo::InitialRefrigerantSystem()
{
    if(AirStream_Select==0)
        return;

    TPanel *pnlRefrigerantMachine[8]={pnlRefrigerantMachine1,pnlRefrigerantMachine2,pnlRefrigerantMachine3,pnlRefrigerantMachine4,
                                        pnlRefrigerantMachine5,pnlRefrigerantMachine6,pnlRefrigerantMachine7,pnlRefrigerantMachine8};

    TLabel *LabRefrigerantValue[8]={LabRefrigerantValue1,LabRefrigerantValue2,LabRefrigerantValue3,LabRefrigerantValue4,
                                        LabRefrigerantValue5,LabRefrigerantValue6,LabRefrigerantValue7,LabRefrigerantValue8};

    TPanel *pnlRefCopm1Status[8]={pnlRefCopm1Status_1,pnlRefCopm1Status_2,pnlRefCopm1Status_3,pnlRefCopm1Status_4,
                                        pnlRefCopm1Status_5,pnlRefCopm1Status_6,pnlRefCopm1Status_7,pnlRefCopm1Status_8};

    TPanel *pnlRefCopm2Status[8]={pnlRefCopm2Status_1,pnlRefCopm2Status_2,pnlRefCopm2Status_3,pnlRefCopm2Status_4,
                                        pnlRefCopm2Status_5,pnlRefCopm2Status_6,pnlRefCopm2Status_7,pnlRefCopm2Status_8};

    TLabel *LabRefCopm1HpValue[8]={labRefCopm1HpValue_1,labRefCopm1HpValue_2,labRefCopm1HpValue_3,labRefCopm1HpValue_4,
                                        labRefCopm1HpValue_5,labRefCopm1HpValue_6,labRefCopm1HpValue_7,labRefCopm1HpValue_8};

    TLabel *LabRefCopm2HpValue[8]={labRefCopm2HpValue_1,labRefCopm2HpValue_2,labRefCopm2HpValue_3,labRefCopm2HpValue_4,
                                        labRefCopm2HpValue_5,labRefCopm2HpValue_6,labRefCopm2HpValue_7,labRefCopm2HpValue_8};

    TLabel *LabRefCopm1LpValue[8]={labRefCopm1LpValue_1,labRefCopm1LpValue_2,labRefCopm1LpValue_3,labRefCopm1LpValue_4,
                                        labRefCopm1LpValue_5,labRefCopm1LpValue_6,labRefCopm1LpValue_7,labRefCopm1LpValue_8};

    TLabel *LabRefCopm2LpValue[8]={labRefCopm2LpValue_1,labRefCopm2LpValue_2,labRefCopm2LpValue_3,labRefCopm2LpValue_4,
                                        labRefCopm2LpValue_5,labRefCopm2LpValue_6,labRefCopm2LpValue_7,labRefCopm2LpValue_8};

    TLabel *LabRefrigerantAdjustValue[8]={LabRefrigerantAdjustValue1,LabRefrigerantAdjustValue2,LabRefrigerantAdjustValue3,LabRefrigerantAdjustValue4,
                                        LabRefrigerantAdjustValue5,LabRefrigerantAdjustValue6,LabRefrigerantAdjustValue7,LabRefrigerantAdjustValue8};
    for(int i=0; i<8; i++)
    {
        TripnlRefrigerantMachine[i] =pnlRefrigerantMachine[i];
        TriLabRefrigerantValue[i]   =LabRefrigerantValue[i];
        TripnlRefCopm1Status[i]     =pnlRefCopm1Status[i];
        TripnlRefCopm2Status[i]     =pnlRefCopm2Status[i];
        TriLabRefCopm1HpValue[i]    =LabRefCopm1HpValue[i];
        TriLabRefCopm2HpValue[i]    =LabRefCopm2HpValue[i];
        TriLabRefCopm1LpValue[i]    =LabRefCopm1LpValue[i];
        TriLabRefCopm2LpValue[i]    =LabRefCopm2LpValue[i];
        TriLabRefrigerantAdjustValue[i]    =LabRefrigerantAdjustValue[i];
    }
    bInitFormcomponent = false;
    OldRefrigerantCommand = false;
    // AI(W906-FW3-LotInfo-WA) 20260819: GATE WA-5 -- ATC_OFFLINE_FormComInit()
    // is RECON Tier 2 (unverified), not one of this wave's 39 Tier-1 methods.
    // See forms/fLotInfo.h GATE REGISTER WA-5.
#if 0
    ATC_OFFLINE_FormComInit();
#endif
    if(Total_Compressor>5)
        ts_RefrigerantStatus_Page_2->TabVisible=true;
    else
        ts_RefrigerantStatus_Page_2->TabVisible=false;

    for(int i=0; i<8; i++)
    {
        if(i<Total_Compressor)
        {
            pnlRefrigerantMachine[i]->Visible=true;
        }
        else
        {
            pnlRefrigerantMachine[i]->Visible=false;
        }
    }
}

// -- SetATCFormVisible (golden uLotInfo.cpp:9938-9985) -----------------------
void TfLotInfo::SetATCFormVisible()
{
    if(Tri_Temp_Machine==1)
    {
        tsATC->TabVisible=false;
    }
    else
    {
        if(ATC_SYSTEM==eATCHonPrecType)
        {
            if(Temperature.bATC70Active==true)
            {
                palATC->Caption="ATC 7.0 Monitor";
                aldATC7Status->Visible=true;
                lblATC70->Visible=true;
                aldATCChillerStatus->Visible=false;
                lblChiller->Visible=false;
            }
            else
            {
                palATC->Caption="ATC 2.0 Monitor";
                aldATC7Status->Visible=false;
                lblATC70->Visible=false;
                aldATCChillerStatus->Visible=true;
                lblChiller->Visible=true;
            }
            tsATC->TabVisible=true;
        }
        else if(ATC_SYSTEM==eNewATCSystem)
        {
            palATC->Caption="ATC Monitor";
            tsATC->TabVisible=true;

            aldATC7Status->Visible=false;
            lblATC70->Visible=false;
            lblChiller->Visible=false;
            aldATCChillerStatus->Visible=false;
            pan_ATCChillerSV->Visible=false;
            pl_ATCChillerSV->Visible=false;
            lblATC_Now_RecipeFile->Visible=true;
            NetATCTime->Enabled=true;
        }
        else
        {
            tsATC->TabVisible=false;
        }
    }
}

// -- CheckEventLogParameter (golden uLotInfo.cpp:10673-10720) ----------------
bool TfLotInfo::CheckEventLogParameter()
{
    AnsiString S;

    if(CUSTOMER_CODE!=CC_ASE_CL ||
       IniConfig.bN22Enable_EventLog==false ||
       LastSet.iTester==OFF_LINE)
        return true;

    S=edtASECL_LotID->Text;
    S=S.Trim();
    if(S.Length()<=0)
    {
        ShowMyMessage("Lot Name must Key in!!");
        return false;
    }

    S=edInsertion->Text;
    S=S.Trim();
    if(S.Length()<=0)
    {
        ShowMyMessage("Insertion must Key in!!");
        return false;
    }

    S=edCustomerDevice->Text;
    S=S.Trim();
    if(S.Length()<=0)
    {
        ShowMyMessage("Customer Device must Key in!!");
        return false;
    }

    S=edFlowID->Text;
    S=S.Trim();
    if(S.Length()<=0)
    {
        ShowMyMessage("Flow ID must Key in!!");
        return false;
    }

    if(btnASECL_LotStart->Down==false)
    {
        ShowMyMessage("Must Press Lot Start First!!");
        return false;
    }
    return true;
}

// -- ShowATCTempPanel (golden uLotInfo.cpp:14561-14606) ----------------------
void TfLotInfo::ShowATCTempPanel()
{
    if(iATC_Use_Heat_Count==8)
    {
        Pan_ATC_Use_4Head->Left=4;
        Pan_ATC_Use_8Head->Top=216;
        Pan_ATC_Use_8Head->Left=4;
        Pan_ATC_Use_8Head->Visible=true;
        Pan_ATC_Use_32Head->Visible=false;
    }
    else if(iATC_Use_Heat_Count>8)
    {
        Pan_ATC_Use_4Head->Left=0;
        Pan_ATC_Use_4Head->Top=124;
        Pan_ATC_Use_8Head->Top=124;
        Pan_ATC_Use_8Head->Left=272;
        Pan_ATC_Use_8Head->Visible=true;
        Pan_ATC_Use_32Head->Visible=true;
    }
    else
    {
        Pan_ATC_Use_4Head->Left=4;
        Pan_ATC_Use_4Head->Top=124;
        Pan_ATC_Use_8Head->Visible=false;
        Pan_ATC_Use_32Head->Visible=false;
    }

    for(int i=0; i<ATC_HEAD_COUNT; i++)
    {
        ATCChPal[i]->Visible=(i<iATC_Use_Heat_Count);
        ATCPtr[i]->Visible=(i<iATC_Use_Heat_Count);
    }

    if(Temperature.bUseReferTempSensor==true)
    {
        for(int i=0; i<ATC_HEAD_COUNT; i++)
        {
            ATCReferPtr[i]->Visible=(i<iATC_Use_Heat_Count);
        }
    }
    else
    {
        for(int i=0; i<ATC_HEAD_COUNT; i++)
            ATCReferPtr[i]->Visible=false;
    }
}

// -- JCETWhite2DIDShow (golden uLotInfo.cpp:15953-15992) ---------------------
void TfLotInfo::JCETWhite2DIDShow(bool bUse)
{
    AnsiString sPath=AuthPath+"config.ini";
    AnsiString sTmp;
    cbRunMode->Items->Clear();
    if(bUse==true)
    {
        sTmp=ReadIniData(sPath, "Lot Info", "Run Mode 2DID",       AnsiString(""));
        cbRunMode->Items->Add("FT1");
        cbRunMode->Items->Add("FT2");
        cbRunMode->Items->Add("FT3");
        cbRunMode->Items->Add("FT4");
        cbRunMode->Items->Add("FT5");
        cbRunMode->Items->Add("FT6");
        cbRunMode->Items->Add("FT7");
        cbRunMode->Items->Add("FT8");
        cbRunMode->Items->Add("FT9");
        cbRunMode->Text="";
    }
    else
    {
        sTmp=ReadIniData(sPath, "Lot Info", "Run Mode",       AnsiString(""));
        cbRunMode->Items->Add("Normal");
        cbRunMode->Items->Add("RT");
        cbRunMode->Items->Add("EQC");
        cbRunMode->Text="Normal";
        cbRunMode->ItemIndex=0;
    }
    for(int i=0; i<cbRunMode->Items->Count; i++)
    {
        if(cbRunMode->Items->Strings[i]==sTmp)
        {
            cbRunMode->Text=sTmp;
            cbRunMode->ItemIndex=i;
            break;
        }
    }
    labCusLotID->Visible=bUse;
    edtCusLotID->Visible=bUse;
}

// -- ShowInformation (golden uLotInfo.cpp:12326-12362) -----------------------
void TfLotInfo::ShowInformation(bool bShow)
{
    if(bShow==true)
    {
        gbFTPAutomation_Download    ->Visible=true;
        gbFTPAutomation_Upload      ->Visible=true;
        sbRecipeUpload              ->Visible=true;
        sbRecipeDownload            ->Visible=true;
        sbFTPAutomationSave         ->Visible=true;

        if(IniConfig.bSIGURDFunction && pgLotinfo->ActivePageIndex==18)
        {
            Height=390;
            Width=475;
            sbTest->Top=304;
            this->Top=205;
            this->Left=340;
        }
    }
    else
    {
        gbFTPAutomation_Download    ->Visible=false;
        gbFTPAutomation_Upload      ->Visible=false;
        sbRecipeUpload              ->Visible=false;
        sbRecipeDownload            ->Visible=false;
        sbFTPAutomationSave         ->Visible=false;

        if(IniConfig.bSIGURDFunction && pgLotinfo->ActivePageIndex==18)
        {
            Height=150;
            Width=150;
            sbTest->Top=50;
            this->Top=592;
            this->Left=217;
        }
    }
}

// -- N23UseLotInfoFile (golden uLotInfo.cpp:7346-7380) -----------------------
bool TfLotInfo::N23UseLotInfoFile()
{
    AnsiString strPath, str4;
    if(IniConfig.bN23UseLotInfoFile)
    {
        strPath=IncludeTrailingPathDelimiter(IniConfig.sN23LotInfoPath)+edtSysLotID->Text+AnsiString(".txt");
        if(FileExists(strPath))
        {
            mmo2DLotInfo->Lines->LoadFromFile(strPath);
            for(int i=0; i<mmo2DLotInfo->Lines->Count; i++)
            {
                str4=mmo2DLotInfo->Lines->Strings[i];
                if(str4.AnsiPos("CUST_LOT_ID:")!=0)
                {
                    edtCusLotID->Text=str4.SubString(str4.AnsiPos(":")+1, str4.Length());
                }
                else if(str4.AnsiPos("FAMILY:")!=0)
                {
                    edtCusDevGrp->Text=str4.SubString(str4.AnsiPos(":")+1, str4.Length());
                }

                if(str4.AnsiPos("CURR_DEVICE:")!=0)
                {
                    edtDevice->Text=str4.SubString(str4.AnsiPos(":")+1, str4.Length());
                }
            }
        }
        else
        {
            ShowMyMessage("The Lot info for 2DID sorting is missing", "找不到2DID sorting用的Lot info");
            return false;
        }
    }
    return true;
}

// -- edTempKeyUp (golden uLotInfo.cpp:4966-4999) -----------------------------
void TfLotInfo::edTempKeyUp()
{
    int iPos;

    iPos=(edTemp->Text.UpperCase()).AnsiPos("/K");
    if(iPos!=0)
    {
        edTemp->Text=edTemp->Text.SubString(1, edTemp->Text.Length()-2);
        edTemp->Text=edTemp->Text+"+";
        edTemp->SelStart=edTemp->Text.Length();
    }

    iPos=(edTemp->Text.UpperCase()).AnsiPos("/O");
    if(iPos!=0)
    {
        edTemp->Text=edTemp->Text.SubString(1, edTemp->Text.Length()-2);
        edTemp->Text=edTemp->Text+"/";
        edTemp->SelStart=edTemp->Text.Length();
    }

    iPos=edTemp->Text.AnsiPos("/0");
    if(iPos!=0)
    {
        edTemp->Text=edTemp->Text.SubString(1, edTemp->Text.Length()-2);
        edTemp->Text=edTemp->Text+"/";
        edTemp->SelStart=edTemp->Text.Length();
    }

    if(bLotFirstKeyIn==false)
    {
        bLotFirstKeyIn=true;
    }
}

// -- edtSysLotIDKeyPress (golden uLotInfo.cpp:11892-11917) -------------------
void TfLotInfo::edtSysLotIDKeyPress(char Key)
{
    if(IniConfig.bO23_InputLotIDByBarcode)
        return;

    if(CUSTOMER_CODE==CC_Murata)
    {
        if(Key=='\r')
        {
            // DEVIATION: golden `edPage->SetFocus();` dropped -- see forms/
            // fLotInfo.h's DEVIATION note above edPage's declaration.
        }
        else if(Key=='$')
        {
            edtSysLotID->Text=edtSysLotID->Text.SubString(1, edtSysLotID->Text.Length()-1);
            // DEVIATION: golden `edPage->SetFocus();` dropped, see above.
        }
    }
    else if(CUSTOMER_CODE==CC_AMD_M && CosFunction.bHiSiliconFunction==true)
    {
        if(bLotID_OK==true)
            bLotID_OK=false;

        if(Key=='\r')
            bLotID_OK=true;
    }
}

// -- SettsChipAdvVisible (golden uLotInfo.cpp:1240-1260) ---------------------
void TfLotInfo::SettsChipAdvVisible()
{
    if(CUSTOMER_CODE==CC_CYUEAN)
    {
        tsChipAdv->TabVisible=true;
    }
    else if(CUSTOMER_CODE==CC_PANTHER ||
            CUSTOMER_CODE==CC_Greatek)
    {
        tsChipAdv->TabVisible=false;
    }
    else
    {
        tsChipAdv->TabVisible=true;
        tsChipAdv->Caption="Lot Info";
    }
}

// -- bCheckOnlyOneFile (golden uLotInfo.cpp:14193-14213) ---------------------
bool TfLotInfo::bCheckOnlyOneFile(AnsiString asPath, AnsiString &asFileName)
{
    TFileListBox *flbStr=new TFileListBox();
    int iCT=-1;
    bool bResult=true;

    flbStr->Directory=asPath;
    flbStr->Mask="*.*";
    flbStr->Refresh();
    flbStr->Update();

    iCT=flbStr->Items->Count;
    if(iCT!=1)
        bResult=false;
    else
        asFileName=flbStr->Items->Strings[0];

    delete flbStr;
    return bResult;
}

// -- edPageKeyPress (golden uLotInfo.cpp:11919-11933) ------------------------
void TfLotInfo::edPageKeyPress(char Key)
{
    if(CUSTOMER_CODE==CC_Murata)
    {
        if(Key=='\r')
        {
            // DEVIATION: golden `edtSysOperatorID->SetFocus();` dropped, see
            // forms/fLotInfo.h's DEVIATION note above edPage's declaration.
        }
        else if(Key=='$')
        {
            edPage->Text=edPage->Text.SubString(1, edPage->Text.Length()-1);
            // DEVIATION: golden `edtSysOperatorID->SetFocus();` dropped, see above.
        }
    }
}

// -- CheckActionFlag (golden uLotInfo.cpp:16234-16246) -----------------------
void TfLotInfo::CheckActionFlag()
{
    ledLoader->Value=bLoaderActionFlag[0];
    ledEmpty->Value=bLoaderActionFlag[1];
    ledColor->Value=bLoaderActionFlag[2];
    ledAuto1->Value=bUnLoaderActionFlag[0];
    ledAuto2->Value=bUnLoaderActionFlag[1];
    ledAuto3->Value=bUnLoaderActionFlag[2];
    ledStartAGV->Value=bAMRReceiveAGVStart;
    ledSTART->Value=bAMRReceiveStart;
    ledLoaderTotalTray->Value=bAMRReceiveLoaderTotalTray;
    ledLOT_START->Value=RunInfo.bLotStart;
}

// -- SetTesterStartTimeByB03 (golden uLotInfo.cpp:14298-14309) ---------------
void TfLotInfo::SetTesterStartTimeByB03()
{
    if(IniConfig.bB03_TesterReport)
    {
        GetTimeInfo();
        AnsiString sTemp="", sTemp1="";
        sTemp=IntToStr(SystemYear)+"/"+IntToStr(SystemMonth)+"/"+IntToStr(SystemDate);
        sB03RunData=sTemp;
        sTemp1=IntToStr(SystemHour)+":"+IntToStr(SystemMin)+":"+IntToStr(SystemSec);
        sB03StartTime=sTemp1;
    }
}

// -- btnLoadFileClick (golden uLotInfo.cpp:14175-14184) ----------------------
void TfLotInfo::btnLoadFileClick()
{
    AnsiString SortFileName;
    OpenDialog1->Title="Open 2DID Sort File";
    if(OpenDialog1->Execute())
    {
        SortFileName=OpenDialog1->FileName;
        edSort2DIDBinFile->Text=SortFileName;
    }
}

// -- cbRunModeKeyDown / cbRunModeKeyUp (golden uLotInfo.cpp:11990-12006) -- WA-7
void TfLotInfo::cbRunModeKeyDown()
{
    // AI(W906-FW3-LotInfo-WA) 20260819: GATE WA-7 -- see forms/fLotInfo.h
    // GATE REGISTER WA-7 (fBarCode->JCETUseMakeWhite2DIDList(), already gated
    // tree-wide at Public/MyProductionRecord.cpp:1198 GATE G-1).
#if 0
    if(fBarCode->JCETUseMakeWhite2DIDList()==true)
    {
        cbRunMode->Text="";
    }
#endif
}
void TfLotInfo::cbRunModeKeyUp()
{
#if 0   // GATE WA-7, see cbRunModeKeyDown above
    if(fBarCode->JCETUseMakeWhite2DIDList()==true)
    {
        cbRunMode->Text="";
    }
#endif
}

// -- edDeviceNameKeyUp (golden uLotInfo.cpp:12044-12051) ---------------------
void TfLotInfo::edDeviceNameKeyUp()
{
    if(bLotFirstKeyIn==false)
    {
        bLotFirstKeyIn=true;
    }
}

// -- ReflashInfo (golden uLotInfo.cpp:16479-16485) ---------------------------
void TfLotInfo::ReflashInfo()
{
    labLoaderTrayCount->Caption=iLoaderTrayCountCal;
    labAuto1TrayCount_KYEC->Caption=iUnloaderTrayCountCal[0];
    labAuto2TrayCount_KYEC->Caption=iUnloaderTrayCountCal[1];
    labAuto3TrayCount_KYEC->Caption=iUnloaderTrayCountCal[2];
}

// -- labLotIDMouseDown (golden uLotInfo.cpp:8423-8428) -- WA-8 --------------
void TfLotInfo::labLotIDMouseDown()
{
    if(CUSTOMER_CODE==CC_KYEC_LEE && AccessLevel==iDefHonPrecLevel)
    {
        // AI(W906-FW3-LotInfo-WA) 20260819: GATE WA-8 (OPENED 20260824) -- see forms/fLotInfo.h
        // GATE WA-8 OPENED 20260824 (FW-QWKEY2): fQwertyKey real since FW-QWKEY1 (fc08e09).
        fQwertyKey->ShowQwertyKey(edtSysLotID, N_NO_SYMBOL|N_NO_SPACE);
    }
}

// -- edtLotVerifyMouseDown / edtLotVerifyKeyPress (golden :14515-14526) ------
void TfLotInfo::edtLotVerifyMouseDown()
{
    edtLotVerify->Text=TestIF_File.sLotIDSubstr;
}
void TfLotInfo::edtLotVerifyKeyPress()
{
    edtLotVerify->Text=TestIF_File.sLotIDSubstr;
}

// -- btnCancelTestPauseClick (golden uLotInfo.cpp:12297-12301) ---------------
void TfLotInfo::btnCancelTestPauseClick()
{
    bTesterSendPause=false;
    bTesterPauseMusic=false;
    bPauseAlarmDelayActive=false;                                               //RogerYang 20260626 : 手動取消 Pause 一併清逾時計時    //AI(W906-GB-P2c) 20260926: golden 912 uLotInfo.cpp:12548
}

// -- BtnPauseMouseDown / BtnPauseMouseUp (golden :14533-14543) ---------------
void TfLotInfo::BtnPauseMouseDown()
{
    BtnPause->Down=true;
}
void TfLotInfo::BtnPauseMouseUp()
{
    BtnPause->Down=false;
}

// -- VisibleUploadBtnPAT (golden uLotInfo.cpp:15710-15714) -------------------
void TfLotInfo::VisibleUploadBtnPAT(bool bVisible)
{
    sbUploadPAT->Down=!bVisible;
    sbUploadPAT->Visible=bVisible;
}

// -- cbbDeviceNameChange (golden uLotInfo.cpp:7216-7219) ---------------------
void TfLotInfo::cbbDeviceNameChange()
{
    edDeviceName->Text=cbbDeviceName->Text;
}

// -- btStartCountClick (golden uLotInfo.cpp:8418-8421) -----------------------
void TfLotInfo::btStartCountClick()
{
    bStartCount_SCK=true;
}

// -- edtASECL_LotIDClick (golden uLotInfo.cpp:10491-10494) -- WA-8 ----------
void TfLotInfo::edtASECL_LotIDClick(TObject *Sender)
{
    // AI(W906-FW3-LotInfo-WA) 20260819: GATE WA-8 (OPENED 20260824), see labLotIDMouseDown above.
    fQwertyKey->ShowQwertyKey((TEdit *)Sender, N_NO_SYMBOL|N_NO_SPACE);
}

// -- btTesterTCPShowClick (golden uLotInfo.cpp:13841-13844) -- WA-9 --------
void TfLotInfo::btTesterTCPShowClick()
{
    // AI(W906-FW3-LotInfo-WA) 20260819: GATE WA-9 -- see forms/fLotInfo.h
    // GATE REGISTER WA-9 (fTesterTCP has no port anywhere in this tree --
    // Interface/TesterTCP.h's own banner: "NOT a TfTesterTCP class or facade
    // at all, only free functions").
#if 0
    fTesterTCP->Show();
#endif
}

// -- InitialLoaderTask / InitialLDLevelTask (golden :16259-16267) -----------
void TfLotInfo::InitialLoaderTask(int iPos)
{
    iLoaderTask[iPos]=1;
}
void TfLotInfo::InitialLDLevelTask(int iPos)
{
    iloaderLevelTask[iPos]=1;
}

// ===========================================================================
//  AI(W906-FW3-LotInfo-WB) 20260819: Wave B method bodies. See forms/
//  fLotInfo.h's file banner for WAVE SCOPE / GATE REGISTER.
// ===========================================================================

// -- FormDestroy (golden uLotInfo.cpp:301-314) -- RECON MISS, see banner ----
void TfLotInfo::FormDestroy()
{
    try
    {
        TimerERMS->Enabled=false;
        slASECLTestInfor->Clear();
        delete slASECLTestInfor;
    }
    catch(...)
    {
        MyDBIProcess("Exception", "TfLotInfo::FormDestroy");
    }
    LogSoftwareOffTime("TfLotInfo, FormDestroy");                               //Steven 20210526 : 紀錄軟體執行時間
}

// -- FormClose (golden uLotInfo.cpp:1235-1238) -------------------------------
void TfLotInfo::FormClose()
{
    bShow=false;
}

// -- pgLotinfoChange (golden uLotInfo.cpp:7221-7266) -------------------------
void TfLotInfo::pgLotinfoChange()
{
    AnsiString sConfigPath=AuthPath+"Security_new.def";
    if(pgLotinfo->ActivePage==tsSelection)
    {
        chkTempOffset->Checked  =CheckAndReadIniData(sConfigPath, "Network", "Temp Offset",     true);
        chkContactHigh->Checked =CheckAndReadIniData(sConfigPath, "Network", "Contact High",    false);
        chkContactForce->Checked=CheckAndReadIniData(sConfigPath, "Network", "Contact Force",   true);
        chkContactMode->Checked =CheckAndReadIniData(sConfigPath, "Network", "Contact Mode",    true);
        chkHotPlate->Checked    =CheckAndReadIniData(sConfigPath, "Network", "HotPlate",        false);
        chkLoadUnload->Checked  =CheckAndReadIniData(sConfigPath, "Network", "Load Unload",     false);
        chkSpeedSetting->Checked=CheckAndReadIniData(sConfigPath, "Network", "Speed Setting",   true);
        chkShuttleMode->Checked =CheckAndReadIniData(sConfigPath, "Network", "Shuttle Mode",    false);

        chkTestMode->Checked    =CheckAndReadIniData(sConfigPath, "Network", "Test Mode",       true);
        chkBinasgn->Checked     =CheckAndReadIniData(sConfigPath, "Network", "Binasgn",         true);
        chkBinasgnOff->Checked  =CheckAndReadIniData(sConfigPath, "Network", "BinasgnOff",      true);

        checkbAutoClean->Checked=CheckAndReadIniData(sConfigPath, "Network", "Auto Clean",      true);                  //jou 20161122 Auto Clean 參數可以選擇是否需要上傳下載

        chkAutoCleanContactHeight->Checked=CheckAndReadIniData(sConfigPath, "Network", "Auto Clean Contact Height",      false);                                //JerryYang 20241019 : 矽品二林 耀仁要求Auto clean高度可選擇不覆蓋

        cbBottom2DOffset->Checked =CheckAndReadIniData(sConfigPath, "Network", "Bottom 2D Offset",    false);           //JerryYang 20201122 Bottom 2D offset不覆蓋

        chkART->Visible=(CosFunction.bUseSCKART);
        chkART_RTCount->Visible=(CosFunction.bUseSCKART);
        if(CosFunction.bUseSCKART)
        {
            chkART->Checked     =CheckAndReadIniData(sConfigPath, "Network", "Auto Retest",     true);                  //Steven 20190918 : ART設定下載不覆蓋
            chkART_RTCount->Checked=CheckAndReadIniData(sConfigPath, "Network", "ART_RT_Count ", true);                 //Steven 20191101 : ART RT count不覆蓋
        }
        chkIndexHeatingMode->Checked=CheckAndReadIniData(sConfigPath, "Network", "Index Heat Mode", true);              //Steven 20180420 (Jou) : JCET吳如春說不覆蓋Index加熱模式

        if(IniConfig.bVTESTFunction==true)                                      //RogerYang 20250314 偉測張冬冬要求 Stop Yield和Consecutive Fail功能修改為灰色,不可更改,默認開啟
        {
            chkStopYield->Checked           =true;
            chkConsecutiveFailure->Checked  =true;
            chkStopYield->Enabled           =false;
            chkConsecutiveFailure->Enabled  =false;
        }

        chkCleanCount->Checked=CheckAndReadIniData(sConfigPath, "Network", "Cleaning Count", checkbAutoClean->Checked);                                         //KenHsieh 20230518 : Auto Clean count不覆蓋
    }

    AdjtsYieldMonitiorSize();                                                   //Steven 20221225 : 統一Lot Info尺寸調整
}

// -- LoadRTCFullViewImg (golden uLotInfo.cpp:5366-5448) -- WB-1 -------------
void TfLotInfo::LoadRTCFullViewImg(bool /*bShowImage*/)
{
    // AI(W906-FW3-LotInfo-WB) 20260819: GATE WB-1 -- see forms/fLotInfo.h
    // GATE REGISTER WB-1 (TImage/TCanvas have no port anywhere in this tree).
#if 0
    imgRTCFullView1->Visible=true;
    imgRTCFullView2->Visible=true;
    imgRTCFullView3->Visible=true;
    imgRTCFullView4->Visible=true;

    if(bShowImage)
    {
        if(MachineTypeChoice==Type_HT9046_LS)
        {
            if(FileExists(FULLVIEWIMAGEPATH1))
                imgRTCFullView1->Picture->LoadFromFile(FULLVIEWIMAGEPATH1);
            else if(FileExists(FULLVIEWIMAGEPATH3))
                imgRTCFullView1->Picture->LoadFromFile(FULLVIEWIMAGEPATH3);
            else
                imgRTCFullView1->Visible=false;

            if(FileExists(FULLVIEWIMAGEPATH4))
                imgRTCFullView2->Picture->LoadFromFile(FULLVIEWIMAGEPATH4);
            else if(FileExists(FULLVIEWIMAGEPATH3))
                imgRTCFullView2->Picture->LoadFromFile(FULLVIEWIMAGEPATH3);
            else
                imgRTCFullView2->Visible=false;

            if(FileExists(FULLVIEWIMAGEPATH2))
                imgRTCFullView3->Picture->LoadFromFile(FULLVIEWIMAGEPATH2);
            else if(FileExists(FULLVIEWIMAGEPATH3))
                imgRTCFullView3->Picture->LoadFromFile(FULLVIEWIMAGEPATH3);
            else
                imgRTCFullView3->Visible=false;

            if(FileExists(FULLVIEWIMAGEPATH5))
                imgRTCFullView4->Picture->LoadFromFile(FULLVIEWIMAGEPATH5);
            else if(FileExists(FULLVIEWIMAGEPATH3))
                imgRTCFullView4->Picture->LoadFromFile(FULLVIEWIMAGEPATH3);
            else
                imgRTCFullView4->Visible=false;
        }
        else
        {
            if(FileExists(FULLVIEWIMAGEPATH1))
                imgRTCFullView1->Picture->LoadFromFile(FULLVIEWIMAGEPATH1);
            else if(FileExists(FULLVIEWIMAGEPATH3))
                imgRTCFullView1->Picture->LoadFromFile(FULLVIEWIMAGEPATH3);
            else
                imgRTCFullView1->Visible=false;

            if(FileExists(FULLVIEWIMAGEPATH2))
                imgRTCFullView2->Picture->LoadFromFile(FULLVIEWIMAGEPATH2);
            else if(FileExists(FULLVIEWIMAGEPATH3))
                imgRTCFullView2->Picture->LoadFromFile(FULLVIEWIMAGEPATH3);
            else
                imgRTCFullView2->Visible=false;
        }
    }
    else
    {
        if(FileExists(FULLVIEWIMAGEPATH3))
            imgRTCFullView1->Picture->LoadFromFile(FULLVIEWIMAGEPATH3);
        else
            imgRTCFullView1->Visible=false;

        if(FileExists(FULLVIEWIMAGEPATH3))
            imgRTCFullView2->Picture->LoadFromFile(FULLVIEWIMAGEPATH3);
        else
            imgRTCFullView2->Visible=false;

        if(MachineTypeChoice==Type_HT9046_LS)
        {
            if(FileExists(FULLVIEWIMAGEPATH3))
                imgRTCFullView3->Picture->LoadFromFile(FULLVIEWIMAGEPATH3);
            else
                imgRTCFullView3->Visible=false;

            if(FileExists(FULLVIEWIMAGEPATH3))
                imgRTCFullView4->Picture->LoadFromFile(FULLVIEWIMAGEPATH3);
            else
                imgRTCFullView4->Visible=false;
        }
    }
#endif
}

// -- edDeviceNameMouseDown (golden uLotInfo.cpp:7193-7214) -- WB-2 ----------
// AI(W906-FW-SIG-W15) 20260826: GATE (WB-2-BTN) **收窄**，不是退役——它的前提
// 只死了一半：`TMouseButton`/`mbRight`/`mbLeft` 現在有 port 了
// （vclcompat/ShiftState.h，commit f184093），所以簽章與 `if(Button==...)` 回填為
// golden 原文；但那個 if 裡唯一的敘述是 `Clipboard()->Clear()`，而 VCL 的
// `Clipboard()` 在本樹仍然零 port——那一行改由既有的 GATE (CLIP) 承接
// （與本方法下半段 SCC 分支裡同一行用的是同一個 gate）。
// 依 pt-wave 的「前提死掉不代表答案就是退役」：重新問過之後，真正的答案是
// 「換一個更小的 gate」，而不是整段打開。
void TfLotInfo::edDeviceNameMouseDown(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)
{
    (void)Sender; (void)Shift; (void)X; (void)Y;   //AI(W906-FW-SIG-W15): golden 也沒讀這四個
#ifndef SOFT_SIMULTE
    if(CUSTOMER_CODE==CC_AMKOR_China ||                                     //jou 2013-01-04 防止OP使用複製貼上的方式讀取工作檔
       CUSTOMER_CODE==CC_QUALCOMM)                                          //JerryYang 20170412 (Steven) add QUALCOMM
    {
        if(Button==mbRight || Button==mbLeft)
        {
#if 0 // GATE (CLIP) -- VCL Clipboard() 在本樹零 port（與下方 SCC 分支同一個 gate）
            Clipboard()->Clear();
#endif // GATE (CLIP)
        }
    }
#endif
    if((CUSTOMER_CODE==CC_SCC && AccessLevel<iDefHonPrecLevel))
    {
        edDeviceName->Text="";
#if 0 // GATE (CLIP) -- VCL Clipboard() has no port anywhere in this tree (grep 20260825)
        Clipboard()->Clear();
#endif // GATE (CLIP)
        AnsiString sBarcodeID=InputBarcodeNumber("Input Device Name:");
        edDeviceName->Text=sBarcodeID;
    }
}

// -- CutTempToEdit (golden uLotInfo.cpp:5225-5244) ---------------------------
void TfLotInfo::CutTempToEdit(AnsiString asString)
{
    int iPos1=0,iPos2=0,iTemp=25;
    AnsiString asBuffer1,asBuffer2;
    if(CUSTOMER_CODE==CC_SCC ||
       CUSTOMER_CODE==CC_SCK)                                                   //ChungHung 20130621 add SCK RMS
    {
        iPos1=asString.Pos("_")+1;
        asBuffer1=asString.SubString(iPos1,asString.Length());
        iPos2=asBuffer1.Pos("_")+1;
        asBuffer2=asBuffer1.SubString(iPos2,asBuffer1.Length());

        iTemp=atoi(asBuffer2.c_str());
        if(iTemp>140)
            iTemp=140;
        if(iTemp<25)
            iTemp=25;
        edTemp->Text=iTemp;
    }
}

// -- btChangeFileClick (golden uLotInfo.cpp:10213-10237) -- WB-3 -----------
void TfLotInfo::btChangeFileClick()
{
    // AI(W906-FW3-LotInfo-WB) 20260819: GATE WB-3 -- see forms/fLotInfo.h
    // GATE REGISTER WB-3 (fBarCode has SOME port but none of these 4 members).
#if 0
    if(TestIF_File.bEnableBarCode)
    {
        if(BAR_CODE_INSTALL==ebctInShtIntel)
        {
            fBarCode->btBarcodeChangeFileDisConnect->Click();                   //wei 20160728 Barcode File切換
            fBarCode->btBarcodeChangeFileConnect->Click();
        }
        else if(BAR_CODE_INSTALL==ebctEtherNetCCD)
        {
            bBarcodeConnect=true;
            fBarCode->InitialBarcodeScanChangeFile();
            fBarCode->TimerBarcodeChangeFile->Enabled=true;
        }
        else if(BAR_CODE_INSTALL==ebctUseCCDMode &&
                CosFunction.b2DUseSubJobFunction==true &&
                TestIF_File.b2DUseSubJob==true)
        {
            bBarcodeConnect=true;
            fBarCode->InitialBarcodeScanChangeFile();
            fBarCode->TimerBarcodeChangeFile->Enabled=true;
        }
    }
#endif
}

// -- edtSysOperatorIDKeyUp (golden uLotInfo.cpp:10287-10312) -- WB-4 -------
void TfLotInfo::edtSysOperatorIDKeyUp()
{
    if(CUSTOMER_CODE==CC_Murata)
    {
        return;
    }

    if(bLotFirstKeyIn==false)                                                   //Ifor 20190924 : add Barcode 輸入判斷避免讀碼失敗
    {
        bLotFirstKeyIn=true;
    }

    if(IniConfig.bO23_InputLotIDByBarcode)                                      //Steven 20241224 : LotID只能用Barcode
    {
        // AI(W906-FW-BARCODE3) 20260825: GATE WB-4 OPENED -- InputBarcodeNumber
        // real since FW-BARCODE1 (e7b4bf8); headless it returns "" (instant-
        // submit), the faithful "user typed nothing" outcome.
        edtSysOperatorID->Text=InputBarcodeNumber("Input OP ID:", "UserName");
    }
    else if(CUSTOMER_CODE==CC_TFME_CHINA ||                                     //Steven 20211112 : 通富微不可以用鍵盤輸入
            IniConfig.bVTESTFunction==true ||                                   //jou 20220912 : 增加VTEST不可以用鍵盤輸入
            (IniConfig.bSPILFunction &&
             TestIF_File.b2DIDAllowList &&                                      //JerryYang 20241104 : 支援2DID白名單功能
             AccessLevel==0))
    {
        edtSysOperatorID->Text="";
    }
}

// -- edtSysLotIDKeyUp (golden uLotInfo.cpp:10314-10344) -- WB-5 -----------
void TfLotInfo::edtSysLotIDKeyUp()
{
    if(CUSTOMER_CODE==CC_Murata)
    {
        return;
    }

    if(CUSTOMER_CODE==CC_PTI ||                                                 //RogerYang 20170327 (Steven) 力成使用條碼機 避免利用Tab切換游標直接輸入
       IniConfig.bO23_InputLotIDByBarcode)                                      //Steven 20241224 : LotID只能用Barcode
    {
        // AI(W906-FW-BARCODE3) 20260825: GATE WB-5 OPENED -- see WB-4 above.
        SetLotID("");
        AnsiString sBarcodeID=InputBarcodeNumber("Input Lot ID:", "LotID");
        SetLotID(sBarcodeID);
    }
    else if(CUSTOMER_CODE==CC_TFME_CHINA ||                                     //Steven 20211112 : 通富微不可以用鍵盤輸入
            IniConfig.bVTESTFunction==true ||                                   //jou 20220912 : 增加VTEST不可以用鍵盤輸入
            (IniConfig.bSPILFunction &&
             TestIF_File.b2DIDAllowList &&                                      //JerryYang 20241104 : 支援2DID白名單功能
             AccessLevel==0))
    {
#ifndef SOFT_SIMULTE
        edtSysLotID->Text="";
#endif
    }

    if(bLotFirstKeyIn==false)                                                   //Ifor 20190924 : add Barcode 輸入判斷避免讀碼失敗
    {
        bLotFirstKeyIn=true;
    }
}

// -- spOCRChangeFileClick (golden uLotInfo.cpp:8458-8474) --------------------
void TfLotInfo::spOCRChangeFileClick()
{
    if(lbOCRUseFile->Caption!="")
    {
        for(int i=0; i<10; i++)
        {
            bOCROK[i]=false;
        }
        bOCROK[0]=true;
        Timer3->Enabled=true;
    }
    else
    {
       ShowMyMessage("Please Enter Change OCR File Name");
    }
    spOCRChangeFile->Down=false;
}

// -- Timer3Timer (golden uLotInfo.cpp:8476-8485) -- WB-6 -------------------
void TfLotInfo::Timer3Timer()
{
    if(InitialOK==false)                                                        //Steven 20160912 : Add InitialOK in Timer
        return;

    // AI(W906-FW3-LotInfo-WB) 20260819: GATE WB-6 -- see forms/fLotInfo.h
    // GATE REGISTER WB-6 (fOCR->OCRChangeFile() does not exist on TfOCR).
#if 0
    int ret;
    ret=fOCR->OCRChangeFile();
    if(ret==1)
        Timer3->Enabled=false;
#endif
}

// -- edDeviceNameKeyDown (golden uLotInfo.cpp:10457-10473) -- WB-7 --------
void TfLotInfo::edDeviceNameKeyDown()
{
    // AI(W906-FW-BARCODE3) 20260825: GATE WB-7 OPENED -- InputBarcodeNumber
    // real since FW-BARCODE1 (e7b4bf8); only the Clipboard() line stays
    // gated (GATE (CLIP)).
    if((CUSTOMER_CODE==CC_SCC && AccessLevel<iDefHonPrecLevel))
    {
        edDeviceName->Text="";
#if 0 // GATE (CLIP) -- VCL Clipboard() has no port anywhere in this tree (grep 20260825)
        Clipboard()->Clear();
#endif // GATE (CLIP)
        AnsiString sBarcodeID=InputBarcodeNumber("Input Device Name:");
        edDeviceName->Text=sBarcodeID;
    }

    if(CUSTOMER_CODE==CC_AMD_M && bDeviceName_OK==true)                         //Ifor 20200827 add:避免刷兩次Barcode造成異常
    {
        bDeviceName_OK=false;
        edDeviceName->Text="";
    }
}

// -- SetCheckCodeByLot (golden uLotInfo.cpp:12311-12324) --------------------
void TfLotInfo::SetCheckCodeByLot(bool _enable)
{
    lbCheckCodeByLot->Caption="Check duplicate code by lot：";
    if(_enable)
    {
        lbCheckCodeByLot->Caption=lbCheckCodeByLot->Caption+"Enable";
        lbCheckCodeByLot->Color=clLime;
    }
    else
    {
        lbCheckCodeByLot->Caption=lbCheckCodeByLot->Caption+"Disable";
        lbCheckCodeByLot->Color=clRed;
    }
}

// -- edTempKeyPress (golden uLotInfo.cpp:12032-12042) ------------------------
void TfLotInfo::edTempKeyPress(char Key)
{
    if(CUSTOMER_CODE==CC_AMD_M && CosFunction.bHiSiliconFunction==true)
    {
        if(bTemp_OK==true)
            bTemp_OK=false;

        if(Key=='\r')
            bTemp_OK=true;
    }
}

// -- edTempKeyDown (golden uLotInfo.cpp:12053-12063) -- Key never read ------
void TfLotInfo::edTempKeyDown()
{
    if(CUSTOMER_CODE==CC_AMD_M &&
       CosFunction.bHiSiliconFunction==true &&
       bTemp_OK==true)                                                          //Ifor 20200827 add:避免刷兩次Barcode造成異常
    {
        bTemp_OK=false;
        edTemp->Text="";
    }
}

// -- btnSaveDataClick (golden uLotInfo.cpp:12285-12295) ----------------------
void TfLotInfo::btnSaveDataClick()
{
    if(RunInfo.bLotStart)
    {
        ShowMyMessage("save data error,need lot end!!");
        return;
    }

    if(SystemStart==false)                                                      //frank 20200814 : 每10盤記錄一次summary log
        SaveTrayRecord(10);
}

// -- edPageMouseDown (golden uLotInfo.cpp:11534-11541) -- WB-8 -------------
void TfLotInfo::edPageMouseDown(TObject *Sender)
{
    if(CUSTOMER_CODE==CC_Murata)
        return;
    // AI(W906-FW3-LotInfo-WB) 20260819: GATE WB-8 (OPENED 20260824) -- see forms/fLotInfo.h
    // GATE WB-8 OPENED 20260824 (FW-QWKEY2): fQwertyKey real since FW-QWKEY1 (fc08e09).
    else
        fQwertyKey->ShowQwertyKey((TEdit *)Sender, N_INTEGER, 0, true, 1, 20);
}

// -- edtSysLotIDKeyDown (golden uLotInfo.cpp:12065-12080) --------------------
void TfLotInfo::edtSysLotIDKeyDown()
{
    if(IniConfig.bO23_InputLotIDByBarcode)                                      //Steven 20241224 : LotID只能用Barcode
    {
        return;
    }

    if(CUSTOMER_CODE==CC_AMD_M &&
       CosFunction.bHiSiliconFunction==true &&
       bLotID_OK==true)                                                         //Ifor 20200827 add:避免刷兩次Barcode造成異常
    {
        bLotID_OK=false;
        edtSysLotID->Text="";
    }
}

// -- edtSysOperatorIDKeyDown (golden uLotInfo.cpp:12082-12097) ---------------
void TfLotInfo::edtSysOperatorIDKeyDown()
{
    if(IniConfig.bO23_InputLotIDByBarcode)                                      //Steven 20241224 : LotID只能用Barcode
    {
        return;
    }

    if(CUSTOMER_CODE==CC_AMD_M &&
       CosFunction.bHiSiliconFunction==true &&
       bOPID_OK==true)                                                          //Ifor 20200827 add:避免刷兩次Barcode造成異常
    {
        bOPID_OK=false;
        edtSysOperatorID->Text="";
    }
}

// -- edtSysOperatorIDMouseUp (golden uLotInfo.cpp:11755-11794) -- WB-9 -----
void TfLotInfo::edtSysOperatorIDMouseUp(TObject *Sender)
{
    if(CUSTOMER_CODE==CC_Murata)
    {
        return;
    }
    // AI(W906-FW-BARCODE4) 20260825: GATE WB-9 OPENED -- InputBarcodeNumber
    // real since FW-BARCODE1 (e7b4bf8), fQwertyKey since FW-QWKEY1; Sender
    // restored (TempEdit dispatch + SCC ShowQwertyKey read it).
    else if(IniConfig.bO23_InputLotIDByBarcode)                                 //Steven 20241224 : LotID只能用Barcode
    {
        AnsiString str, str2;
        TEdit *TempEdit=(TEdit *)Sender;
        if(TempEdit==edtCusLotID)                                               //RogerYang 20251215 : JCET 2D FT1白名單/FT2比對功能
        {
            str="Input Cus. lot ID:";
            str2="LotID";
        }
        else
        {
            str="Input OP ID:";
            str2="UserName";
        }
        TempEdit->Text=InputBarcodeNumber(str, str2);
    }
    else if(CUSTOMER_CODE==CC_TFME_CHINA ||                                     //Steven 20211112 : 通富微不可以用鍵盤輸入
            IniConfig.bVTESTFunction==true ||                                   //jou 20220912 : 增加VTEST不可以用鍵盤輸入
            (IniConfig.bSPILFunction &&
             TestIF_File.b2DIDAllowList &&                                      //JerryYang 20241104 : 支援2DID白名單功能
             AccessLevel==0))
    {
        edtSysOperatorID->Text=InputBarcodeNumber("Input OP ID:", "UserName");
    }
    else if(CUSTOMER_CODE==CC_SCC)                                              //Steven 20200302 : SCC楊恩民說輸入字串5~30個字元
    {
        fQwertyKey->ShowQwertyKey((TEdit *)Sender, N_NO_SYMBOL);
        if(edtSysOperatorID->Text.Length()<5 || edtSysOperatorID->Text.Length()>30)
        {
            edtSysOperatorID->Text="";
        }
    }
}

// -- edtSysLotIDMouseUp (golden uLotInfo.cpp:11796-11831) -- WB-10 --------
void TfLotInfo::edtSysLotIDMouseUp(TObject *Sender)
{
    if(CUSTOMER_CODE==CC_Murata)
    {
    }
    // AI(W906-FW-BARCODE4) 20260825: GATE WB-10 OPENED -- see WB-9 above;
    // Sender restored (the CC_SCC branch's ShowQwertyKey reads it).
    else if(CUSTOMER_CODE==CC_PTI ||                                            //RogerYang 20170327 (Steven) 力成使用條碼機
            IniConfig.bO23_InputLotIDByBarcode)                                 //Steven 20241224 : LotID只能用Barcode
    {
        SetLotID("");
        AnsiString sBarcodeID=InputBarcodeNumber("Input Lot ID:", "LotID");
        SetLotID(sBarcodeID);
    }
    else if(CUSTOMER_CODE==CC_TFME_CHINA)
    {
        edtSysLotID->Text=InputBarcodeNumber("Input Lot ID:", "LotID");
    }
    else if(IniConfig.bSPILFunction &&
            TestIF_File.b2DIDAllowList && AccessLevel==0)                       //JerryYang 20241104 : 支援2DID白名單功能
    {
        edtSysLotID->Text=InputBarcodeNumber("Input Lot ID:", "LotID");
    }
    else if(CUSTOMER_CODE==CC_SCC)                                              //Steven 20200302 : SCC楊恩民說輸入字串5~30個字元
    {
        fQwertyKey->ShowQwertyKey((TEdit *)Sender, N_NO_SYMBOL);
        if(edtSysLotID->Text.Length()<5 || edtSysLotID->Text.Length()>30)
        {
            SetLotID("");
        }
    }
    else if(IniConfig.bVTESTFunction==true)                                     //Steven 20211112 : 通富微不可以用鍵盤輸入  //jou 20220912 : 增加VTEST不可以用鍵盤輸入
    {
        edtSysLotID->Text=InputBarcodeNumber("Input Lot ID:", "LotID");
        edtSysOperatorID->Text=InputBarcodeNumber("Input OP ID:", "UserName");
    }
}

// -- edQAModeMouseDown (golden uLotInfo.cpp:11528-11532) -- WB-11 --------
void TfLotInfo::edQAModeMouseDown(TObject *Sender)
{
    // AI(W906-FW3-LotInfo-WB) 20260819: GATE WB-11 (OPENED 20260824) -- see forms/fLotInfo.h
    // GATE WB-11 OPENED 20260824 (FW-QWKEY2): fQwertyKey real since FW-QWKEY1 (fc08e09).
    fQwertyKey->ShowQwertyKey((TEdit *)Sender, N_INTEGER, 0, true, 10000, 5);
}

// -- edtSysOperatorIDKeyPress (golden uLotInfo.cpp:11935-11963) -------------
void TfLotInfo::edtSysOperatorIDKeyPress(char Key)
{
    if(IniConfig.bO23_InputLotIDByBarcode)                                      //Steven 20241224 : LotID只能用Barcode
        return;

    if(CUSTOMER_CODE==CC_Murata)
    {
        if(Key=='\r')
        {
            cbRunMode->Text="";
            // DEVIATION: golden `cbRunMode->SetFocus();` dropped -- see
            // forms/fLotInfo.h's DEVIATION note (edPage's declaration, Wave A).
        }
        else if(Key=='$')
        {
            edtSysOperatorID->Text=edtSysOperatorID->Text.SubString(1, edtSysOperatorID->Text.Length()-1);
            cbRunMode->Text="";
            // DEVIATION: golden `cbRunMode->SetFocus();` dropped, see above.
        }
    }
    else if(CUSTOMER_CODE==CC_AMD_M && CosFunction.bHiSiliconFunction==true)
    {
        if(bOPID_OK==true)
            bOPID_OK=false;

        if(Key=='\r')
            bOPID_OK=true;
    }
}

// -- cbRunModeKeyPress (golden uLotInfo.cpp:11965-11988) -- WB-12 ---------
void TfLotInfo::cbRunModeKeyPress(char Key)
{
    if(CUSTOMER_CODE==CC_Murata)                                                //Steven 20200629 : Murata要求輸入barcode後, 自動跳下一個欄位
    {
        if(Key=='\r')
        {
            sbSECSLotStart->Click();
            sbSECSLotStart->Down=true;
            sbSECSLotEnd->Down=false;
        }
        else if(Key=='$')
        {
            cbRunMode->Text=cbRunMode->Text.SubString(1, cbRunMode->Text.Length()-1);
            sbSECSLotStart->Click();
            sbSECSLotStart->Down=true;
            sbSECSLotEnd->Down=false;
        }
    }

    // AI(W906-FW3-LotInfo-WB) 20260819: GATE WB-12 -- see forms/fLotInfo.h
    // GATE REGISTER WB-12 (the SAME golden call Wave A's WA-7 already gates).
#if 0
    if(fBarCode->JCETUseMakeWhite2DIDList()==true)                              //RogerYang 20251202 : JCET 2D FT1白名單/FT2比對功能
    {
        cbRunMode->Text="";                                                     //不允許手動輸入
    }
#endif
}

// -- cbRunModeDropDown (golden uLotInfo.cpp:15928-15951) -- WB-13 --------
void TfLotInfo::cbRunModeDropDown()
{
    // AI(W906-FW3-LotInfo-WB) 20260819: GATE WB-13 -- see forms/fLotInfo.h
    // GATE REGISTER WB-13 (the guard itself needs the WA-7/WB-12-gated
    // fBarCode member, so there is no independent real branch to keep).
#if 0
    if(CUSTOMER_CODE==CC_JCET && fBarCode->JCETUseMakeWhite2DIDList()==true)
    {
        AnsiString sRunMode="";
        bool bFlag=false;
        sRunMode=InputBarcodeNumber("Input Run Mode", "RunMode");
        for(int i=0; i<cbRunMode->Items->Count; i++)                            //RogerYang 20251208 : JCET 2D FT1白名單/FT2比對功能
        {
            if(cbRunMode->Items->Strings[i]==sRunMode)
            {
                cbRunMode->Text=sRunMode;
                cbRunMode->ItemIndex=i;
                bFlag=true;
                break;
            }
        }

        if(bFlag==false)
        {
            cbRunMode->ItemIndex=-1;
        }
    }
#endif
}

// -- GetFTP_SettingN06 (golden uLotInfo.cpp:15449-15460) ---------------------
void TfLotInfo::GetFTP_SettingN06(AnsiString &asUserID, AnsiString &asPassword, AnsiString &asHost)
{
    #ifdef SOFT_SIMULTE
    asUserID="HONPREC";
    asPassword="27025312";
    asHost="127.0.0.1";
    #else
    asUserID=IniConfig.FtpUserName;
    asPassword=IniConfig.FtpPassword;
    asHost=IniConfig.FtpHost;
    #endif
}

// -- CheckNoRetestBinFlag (golden uLotInfo.cpp:14608-14630) -- WB-14 ------
bool TfLotInfo::CheckNoRetestBinFlag()                                          //RogerYang 20250604 偉測不可複測bin功能
{
    // AI(W906-FW3-LotInfo-WB) 20260819: GATE WB-14 -- see forms/fLotInfo.h
    // GATE REGISTER WB-14 (fMesSystem has no port at all; ShowErrorMessage is
    // a real alarm-raise). Gated body returns true, golden's own initial/
    // conservative "no violation detected" value.
#if 0
    bool flag=true;
    for(int i=0; i<3; i++)
    {
        if(fMesSystem->NeedNoRTBinID(i)==true)
        {
            ShowErrorMessage(sMES1713[i], K_RETRY|K_SKIP, iMMAuto[eFix1+i], false);                                     //RogerYang 20250806 修正FixIndex
            flag=false;
        }
    }

    if(flag==false)
    {
        flag=true;
        for(int i=0; i<3; i++)                                                  //再檢查一次，如果user都輸入正確，則繼續執行
        {
            if(fMesSystem->NeedNoRTBinID(i)==true)
                flag=false;
        }
    }
    return flag;
#else
    return true;
#endif
}

// -- edStationNumMouseDown (golden uLotInfo.cpp:14060-14064) -- WB-15 ----
void TfLotInfo::edStationNumMouseDown(TObject *Sender)
{
    // AI(W906-FW3-LotInfo-WB) 20260819: GATE WB-15 (OPENED 20260824) -- see forms/fLotInfo.h
    // GATE WB-15 OPENED 20260824 (FW-QWKEY2): fQwertyKey real since FW-QWKEY1 (fc08e09).
    fQwertyKey->ShowQwertyKey((TEdit *)Sender, N_INTEGER, 0, true, 1, 99);
}

// -- ATC_OFFLINE_FormComInit (golden uLotInfo.cpp:14969-14992) ---------------
void TfLotInfo::ATC_OFFLINE_FormComInit()
{
    if(AirStream_Select==0)
        return;

    bInitFormcomponent = true;
    for(int i=0;i<8;i++)
    {
        TriLabRefrigerantValue[i]->Caption = "-999.0";
        TripnlRefCopm1Status[i]->Color = clGray;
        TripnlRefCopm2Status[i]->Color = clGray;
        TriLabRefCopm1HpValue[i]->Caption = "-999.0";
        TriLabRefCopm2HpValue[i]->Caption = "-999.0";
        TriLabRefCopm1LpValue[i]->Caption = "-999.0";
        TriLabRefCopm2LpValue[i]->Caption = "-999.0";
        TriLabRefrigerantAdjustValue[i]->Caption = "-999.0";
        TriLabRefrigerantAdjustValue[i]->Visible = false;
        TripnlRefrigerantMachine[i]->Color = clGray;
    }
    labRefrigerantMachineHighLimit->Caption     ="Comp#2 Hp Over High Limit : -999.0";
    labRefrigerantMachineLowLimit->Caption      ="Comp#2 Hp Over Low Limit : -999.0";

    OldRefrigerantCommand = false;
}

// -- ScanRefrigerantSystem (golden uLotInfo.cpp:14941-14967) -- WB-16 ----
void TfLotInfo::ScanRefrigerantSystem()
{
    if(AirStream_Select==0)
        return;

    // AI(W906-FW3-LotInfo-WB) 20260819: GATE WB-16 -- see forms/fLotInfo.h
    // GATE REGISTER WB-16 (ATC_InterfaceForm exposes only iATC_MODE_TYPE,
    // same finding as Wave A's WA-6).
#if 0
    if(iATCOnLine==0 || ATC_InterfaceForm->IsConnect()==false)
    {
        if(bInitFormcomponent==false)
        {
            ATC_OFFLINE_FormComInit();
        }
        ATC_InterfaceForm->bReadRefrigerantMode_Send=false;
        ATC_InterfaceForm->bReadRefrigerantMode_Recv=false;
    }
    else
    {
        bInitFormcomponent=false;
        if(ATC_InterfaceForm->bReadRefrigerantMode_Recv==true)
        {
            ATC_InterfaceForm->bReadRefrigerantMode_Recv=false;
            RefreshRefrigerantAllStatus();
        }

        if(ATC_InterfaceForm->bReadRefrigerantMode_Send==false)
            ATC_InterfaceForm->Get_ATCRefrigeratorAllStatus(true);
    }
#endif
}

// -- RefreshRefrigerantAllStatus (golden uLotInfo.cpp:14994-15078) -- WB-17 --
void TfLotInfo::RefreshRefrigerantAllStatus()
{
    if(AirStream_Select==0)
        return;

    // AI(W906-FW3-LotInfo-WB) 20260819: GATE WB-17 -- see forms/fLotInfo.h
    // GATE REGISTER WB-17 (both arms need ATC_InterfaceForm members absent
    // from the 1-member shim, same finding as WB-16/WA-6).
#if 0
    char strRefrigeratorAllStatu[8][40];
    if(ATC_InterfaceForm->bReadRefrigerantMode_AllStatus ==true)
    {
        for(int i=0; i<iATC_Refrigerator_Num; i++)
        {
            if(ATC_InterfaceForm->dATC_RefrigeratorAllStatus[i][0] ==1)
            {
                TripnlRefrigerantMachine[i]->Color = clGreen;
            }
            else
            {
                TripnlRefrigerantMachine[i]->Color = clGray;
            }
            sprintf(strRefrigeratorAllStatu[0], "%5.0f", ATC_InterfaceForm->dATC_RefrigeratorAllStatus[i][1]);
            sprintf(strRefrigeratorAllStatu[1], "%5.1f", ATC_InterfaceForm->dATC_RefrigeratorAllStatus[i][2]);
            sprintf(strRefrigeratorAllStatu[2], "%5.1f", ATC_InterfaceForm->dATC_RefrigeratorAllStatus[i][3]);
            sprintf(strRefrigeratorAllStatu[3], "%5.1f", ATC_InterfaceForm->dATC_RefrigeratorAllStatus[i][4]);
            sprintf(strRefrigeratorAllStatu[4], "%5.1f", ATC_InterfaceForm->dATC_RefrigeratorAllStatus[i][5]);
            sprintf(strRefrigeratorAllStatu[5], "%5.1f", ATC_InterfaceForm->dATC_RefrigeratorAllStatus[i][6]);
            TriLabRefrigerantValue[i]->Caption  = strRefrigeratorAllStatu[0];
            TriLabRefCopm1HpValue[i]->Caption   = strRefrigeratorAllStatu[1];
            TriLabRefCopm1LpValue[i]->Caption   = strRefrigeratorAllStatu[2];
            TriLabRefCopm2HpValue[i]->Caption   = strRefrigeratorAllStatu[3];
            TriLabRefCopm2LpValue[i]->Caption   = strRefrigeratorAllStatu[4];
            TriLabRefrigerantAdjustValue[i]->Caption    = strRefrigeratorAllStatu[5];
            if(ATC_InterfaceForm->dATC_RefrigeratorAllStatus[i][7] ==1)
            {
                TripnlRefCopm1Status[i]->Color = clGreen;
            }
            else
            {
                TripnlRefCopm1Status[i]->Color = clGray;
            }

            if(ATC_InterfaceForm->dATC_RefrigeratorAllStatus[i][8] ==1)
            {
                TripnlRefCopm2Status[i]->Color = clGreen;
            }
            else
            {
                TripnlRefCopm2Status[i]->Color = clGray;
            }
        }
        sprintf(strRefrigeratorAllStatu[6],"Comp#2 Hp Over High Limit : %5.1f",ATC_InterfaceForm->dATC_RefrigerantMachineHighLimit);
        sprintf(strRefrigeratorAllStatu[7],"Comp#2 Hp Over Low Limit : %5.1f",ATC_InterfaceForm->dATC_RefrigerantMachineLowLimit);
        labRefrigerantMachineHighLimit->Caption     =strRefrigeratorAllStatu[6];
        labRefrigerantMachineLowLimit->Caption      =strRefrigeratorAllStatu[7];
    }
    else
    {
        for(int i=0; i<8; i++)
        {
            if(ATC_InterfaceForm->iATC_RefrigeratorUserMode[0][i]==1)
            {
                TripnlRefrigerantMachine[i]->Color = clGreen;
            }
            else
            {
                TripnlRefrigerantMachine[i]->Color = clGray;
            }
        }

        if(OldRefrigerantCommand ==false)
        {
            OldRefrigerantCommand = true;
            for(int i=1; i<8; i++)
            {
                TriLabRefrigerantValue[i]->Caption  = FloatToStr(-999.0);
                TriLabRefCopm1HpValue[i]->Caption   = FloatToStr(-999.0);
                TriLabRefCopm1LpValue[i]->Caption   = FloatToStr(-999.0);
                TriLabRefCopm2HpValue[i]->Caption   = FloatToStr(-999.0);
                TriLabRefCopm2LpValue[i]->Caption   = FloatToStr(-999.0);
                TriLabRefrigerantAdjustValue[i]->Caption    = FloatToStr(-999.0);
                TripnlRefCopm1Status[i]->Color = clGray;
                TripnlRefCopm2Status[i]->Color = clGray;
                TripnlRefrigerantMachine[i]->Color = clGray;
            }
        }
    }
#endif
}

// -- leRunCardNumberMouseDown (golden uLotInfo.cpp:15780-15784) -------------
void TfLotInfo::leRunCardNumberMouseDown()
{
    leRunCardNumber->Text="";
}

// -- RefreshAMR (golden uLotInfo.cpp:15816-15844) -- was a no-op stub -------
void TfLotInfo::RefreshAMR()                                                    //Sam 20240304 : 新增 AMR 功能
{
    cbA60_1->Checked=IniConfig.bA60EnableAMR;

    if(CosFunction.bEnableHandlerResultServer==false || IniConfig.bA60EnableAMR==false)
        return;

    pnlWaitTXSetLoader->Caption=IntToStr(IniConfig.iA60NotifyQty[0]);
    pnlWaitRXSetAuto1->Caption=IntToStr(IniConfig.iA60NotifyQty[3]);
    pnlWaitRXSetAuto2->Caption=IntToStr(IniConfig.iA60NotifyQty[4]);
    pnlWaitRXSetAuto3->Caption=IntToStr(IniConfig.iA60NotifyQty[5]);

    pnlWaitTXTotalLoader->Caption=IntToStr(LastSet.iAMRTrayLoaderTotal);
    pnlWaitTXCntLoader->Caption=IntToStr(LastSet.iAMRTrayConut[0]);
    pnlWaitRXCntAuto1->Caption=IntToStr(LastSet.iAMRTrayConut[3]);
    pnlWaitRXCntAuto2->Caption=IntToStr(LastSet.iAMRTrayConut[4]);
    pnlWaitRXCntAuto3->Caption=IntToStr(LastSet.iAMRTrayConut[5]);

    aldWaitTXLoader->Value=AMR.CheckLoaderCount();

    aldWaitRXAuto1->Value=AMR.CheckUnloaderCount(0);
    aldWaitRXAuto2->Value=AMR.CheckUnloaderCount(1);
    aldWaitRXAuto3->Value=AMR.CheckUnloaderCount(2);

    aldWaitTrayFeed->Value=LastSet.bAMRTrayFeedWait;
    aldLoaderLast->Value=LastSet.bAMRLoaderLast;
    ShowAMRCategoryBin();
}

// -- ShowAMRCategoryBin (golden uLotInfo.cpp:15846-15881) --------------------
void TfLotInfo::ShowAMRCategoryBin(bool bInitial)
{
    if(bInitial)
    {
        StrGrdCategory->Cells[1][0]="Count";
        StrGrdCategory->Cells[2][0]="Percent";
        StrGrdCategory->RowCount=iTestBinCount+2;
        for(int i=0; i<iTestBinCount; i++)
            StrGrdCategory->Cells[0][1+i]="Category "+AnsiString(i);

        StrGrdCategory->Cells[0][iTestBinCount+1]="Error Bin";
        return;
    }

    if(CosFunction.bEnableHandlerResultServer==false || IniConfig.bA60EnableAMR==false)
        return;

    int sum=0;
    double f=0.0;
    for(int i=0; i<iTestBinCount; i++)
        sum+=LastSet.iBinData32[2][i];

    for(int i=0; i<iTestBinCount+1; i++)
    {
        StrGrdCategory->Cells[1][1+i]=LastSet.iBinData32[2][i];
        if(sum>0)
        {
            f=(double)LastSet.iBinData32[2][i]*100.0/(double)sum;
            StrGrdCategory->Cells[2][1+i]=(AnsiString)GetFloatFormatString(f, 5, 2)+(AnsiString)("%");
        }
        else
        {
            StrGrdCategory->Cells[2][1+i]="0.00%";
        }
    }
}

// -- cbFirstTrayCheckOnUnloaderMouseDown (golden uLotInfo.cpp:16029-16034) --
void TfLotInfo::cbFirstTrayCheckOnUnloaderMouseDown()
{
    bP60UserClicked=true;
}

// -- RefreshOtherTool (golden uLotInfo.cpp:16047-16072) ----------------------
void  TfLotInfo::RefreshOtherTool()                                             //Jimmychiu 20251219 : Refresh PTI funciton
{
    static bool bTimerRunning=false;
    if((iTestRunMode==FT && TestIF_File.bContinuousLoader==false) ||
       (iTestRunMode==RT && TestIF_File.bContinuousLoader_RT==false))
    {
        return;
    }

    if(bTimerRunning)
        return;
    bTimerRunning=true;
    //
    if(iTestRunMode==FT)
    {
        edLoaderCountNow->Text=IntToStr(LastSet.SendCT[2]);
        edLoaderCountAlarm->Text=IntToStr(TestIF_File.iContinuousLoaderCount);
    }
    else
    {
        edLoaderCountNow->Text=IntToStr(LastSet.SendCT[2]);
        edLoaderCountAlarm->Text=IntToStr(TestIF_File.iContinuousLoaderCount_RT);
    }
    //
    bTimerRunning=false;
}

// -- labNowTrayCountClick (golden uLotInfo.cpp:16186-16190) ------------------
void TfLotInfo::labNowTrayCountClick()
{
    int i = 0;
    labNowTrayCount->Caption=i;
}

// -- Timer4Timer (golden uLotInfo.cpp:16487-16493) -- WB-18 -----------------
void TfLotInfo::Timer4Timer()
{
    CheckActionFlag();
    // AI(W906-FW3-LotInfo-WB) 20260819: GATE WB-18 -- see forms/fLotInfo.h
    // GATE REGISTER WB-18 (CheckAMRAction() is RECON item #206, (b)
    // write-path, not part of this wave's 39 Tier-2 methods).
#if 0
    CheckAMRAction();
#endif

    ReflashInfo();
}

// =============================================================================
//  AI(W906-FW3-LotInfo-WC) 20260819: uLotInfo MIXED-split Wave C -- FormShow
//  and Timer2Timer. See forms/fLotInfo.h's WC banner for STEP 0 / WAVE SCOPE
//  / GATE REGISTER. Segment numbers (S#/T#) follow
//  docs/RECON_uLotInfo_mixed_split.md's own segment table.
// =============================================================================

// -- FormShow (golden uLotInfo.cpp:316-1233) ---------------------------------
void TfLotInfo::FormShow()
{
    // -- S1 (golden :316-346) -- WC-10 gates the ts_ATC6_1 ATC_TYPE_61 clause --
    bShow=true;

    AnsiString sConfigPath=AuthPath+"Security_new.def";                         //JerryYang 20241019 : 矽品二林 耀仁要求Auto clean高度可選擇不覆蓋

    GetTimeInfo();
    ts_FTPAutomation->TabVisible=false;                                         //KaiChen 20190530 ：Sigurd FTP Automation
    tsSigurd_CX->TabVisible =(CUSTOMER_CODE==CC_SIGURD_ChungXing);              //Sam 20220223 : 矽格中興廠新增 Lot 資料
    tsMurata->TabVisible    =(CUSTOMER_CODE==CC_Murata);
    tsSPIL_SZ->TabVisible   =(CUSTOMER_CODE==CC_SPIL_CHINA_SUZHOU);
    tsOEE->TabVisible       =(CosFunction.bOEEFunction);                        //Steven 20221225 : 整理LotInfo畫面
    SettsChipAdvVisible();                                                      //Steven 20221225 : add for CyuEan
    // AI(W906-FW-LOTINFO-W30) 20260826: GATE WC-10 RETIRED -- ATC_TYPE_61 is now
    // a TU-local mirror at the head of this file (see the block there for the
    // 5-TU precedent and for why the W27 banner's "literal 61" recipe rested on
    // a false premise).  All four operands were re-verified visible from this
    // TU on 20260826: ts_ATC6_1 (member, ctor this file:226), Tri_Temp_Machine
    // (un-gated use this file:1406), ATC_SYSTEM (un-gated :530/:1412),
    // eNewATCSystem (un-gated :1432), ATC_InterfaceForm->iATC_MODE_TYPE
    // (acarry_shims.h TATC_InterfaceFormShim's one real member; un-gated
    // read at :2706-onward).  Line is now byte-identical to golden :329.
    //
    // ⚠ WHERE THE VALUES COME FROM (the check that "type resolves" is not the
    // same as "gate is really open"): the SECOND disjunct is DEGRADED, not live.
    // ATC_InterfaceForm is the offline shim (acarry_shims.h:109-115) whose
    // iATC_MODE_TYPE is hard-coded 0 by its own ctor comment ("offline: 0"), so
    // ==61 can never fire until a real ATC link is ported.  That degradation is
    // SAFE rather than wrong-branch-selecting -- 0 matches nothing here, unlike
    // the tcHotPlate1==0 case in docs/KNOWLEDGE.md where a defaulted 0 ACTIVELY
    // selected a branch -- so the assignment reduces to (Tri_Temp_Machine==1),
    // a strict subset of golden.  BEHAVIOUR DELTA vs BEFORE THIS WAVE is still
    // an improvement, because before it this line ran at all: ts_ATC6_1->
    // TabVisible was NEVER ASSIGNED in FormShow and simply kept the facade's
    // false default, hiding the tab even on a Tri_Temp_Machine==1 unit.
    ts_ATC6_1->TabVisible   =(Tri_Temp_Machine==1 || (ATC_SYSTEM==eNewATCSystem && ATC_InterfaceForm->iATC_MODE_TYPE==ATC_TYPE_61));
    tsPATSetUp->TabVisible  =(CUSTOMER_CODE==CC_PANTHER);
    tsBundle->TabVisible    =(USE_COVER_TRAYID!=tCIDNotUse);
    tsSetupFileCheck->TabVisible=(IniConfig.bEnableRmsCheckSetupFile==true);    //Ifor 20230516 add: TFAMD 要求工作檔驗證

    lblPage->Visible=CosFunction.bUseSCKART;
    edPage->Visible=CosFunction.bUseSCKART;

    labCusLotID->Visible=false;                                                 //JerryYang 20230322 : 2D sort lot info UI修改
    labCusDevGrp->Visible=false;
    labDeviceName->Visible=false;
    edtCusLotID->Visible=false;
    edtCusDevGrp->Visible=false;
    edtDevice->Visible=false;
    labCusStep->Visible=false;                                                  //JerryYang 20260201 : add
    edtCusStep->Visible=false;

    pgLotinfo->ActivePage=tsLotID;

    // -- S2 (golden :348-362) --
    if(CUSTOMER_CODE==CC_CYUEAN ||                                              //Steven 20240122 : CyuEan要FT/RT
       CUSTOMER_CODE==CC_JSI_HAOXING)                                           //Steven 20230302 : Add for 紹興長電
    {
        lbLotRunMode->Visible   =true;
        cbRunMode->Visible      =true;
        cbRunMode->Items->Clear();
        cbRunMode->Items->Add("FT");
        cbRunMode->Items->Add("RT");
        cbRunMode->Text="FT";
    }
    else if(CUSTOMER_CODE==CC_NEXPERIA_Guangdong)                               //Steven 20230301 : Add for 安世
    {
        lbLotRunMode->Visible   =false;
        cbRunMode->Visible      =false;
    }

    // -- S3 (golden :364-380) -- WC-1 gates ResetLotInfo() --
    if(CUSTOMER_CODE==CC_MTI)
    {
        tsDeviceInfo->TabVisible=false;
        tsFTP->TabVisible=false;
        palSecsGem->Visible=false;                                              //Steven 20140701
    }
    else if(IniConfig.bShowLotInfo)
    {
        tsDeviceInfo->TabVisible=(IniConfig.bEnableRms==true);
        // AI(W906-FW3-LotInfo-WC) 20260819: GATE WC-1 -- see forms/fLotInfo.h
        // GATE REGISTER WC-1 (ResetLotInfo() is RECON item #149, (b)
        // write-path, not yet translated).
#if 1 // AI(W906-FRW-WC1) 20260927 (Steven 團隊): GATE WC-1 RETIRED -- body translated at the end of this file (golden :14820-14834);
      //   the gate's premise (LastSet.bHasDownloadFile reaches a hard-coded lastdata.dat) is gone: ctest redirects it (W906_LastDataPath,
      //   Jimmy 012fbc13 / 260a29ca) and wb_serve writes lastdata.dat on the golden paths anyway (S95). Jimmy 20260927 00:5x: WC-1 is St01's.
        ResetLotInfo();
#endif
        if(tsDeviceInfo->TabVisible==true)
        {
            pgLotinfo->ActivePage=tsDeviceInfo;
        }
    }
    SetSelectionVisible();                                                      //Steven 20250519 : 統一Selection的顯示設定

    // -- S4 (golden :381-398) -- WC-2 gates cbPATModeChange(cbPATMode) --
    if(CUSTOMER_CODE==CC_TSI)                                                   //frank 20200814 : 每10盤記錄一次summary log
    {
        lblOPID->Caption        ="Tester ID";
        lbLotRunMode->Visible   =false;
        cbRunMode->Visible      =false;
        btnSaveData->Visible    =true;
    }
    else if(CUSTOMER_CODE==CC_PANTHER)
    {
        tsLotID->TabVisible=IniConfig.bB12UsePATSetup;
        cbPATMode->ItemIndex=0;
        // AI(W906-FW3-LotInfo-WC) 20260819: GATE WC-2 -- see forms/fLotInfo.h
        // GATE REGISTER WC-2 (cbPATModeChange() is RECON item #169, (b)
        // write-path dispatcher, not yet translated).
#if 0
        cbPATModeChange(cbPATMode);
#endif
        #ifdef SOFT_SIMULTE
        //MARKED(W906-ST-S3-B2b) 20260918: PAT 家族，依使用者裁決「缺 PAT_Function
        //  沒關係，和這有關的先 Mark」。這三個是 button 物件本身，而本檔 :1136-1145
        //  的 GATE REGISTER D 早就記載整個 PAT 家族（btnRealTimeClick /
        //  btnpatHourlyClick / btnpatEndLotClick …）都要經過 fMain->patFunc，
        //  而 TfMain 沒有那個成員。純顯示：golden 只在模擬時把三顆按鈕顯示出來，
        //  而這個移植樹沒有 VCL UI，補一個 Visible 替身只是做戲。
        //  ⚠ 這三行是 SOFT_SIMULTE 在這棵樹**第一次被編譯**時唯一擋住建置的東西
        //  （全量建置只有這 3 個錯誤）。解除方式＝補 fMain->patFunc 實例指標，
        //  那是 gate 稽核裡的 W1-A，量過是「約 5 行」且只對鴻谷(PANTHER)有意義。
        //btnRealTime->Visible=true;
        //btnpatHourly->Visible=true;
        //btnpatEndLot->Visible=true;
        #endif
    }
    // -- S5 (golden :399) --
    // AI(W906-FW-LOTINFO-W30) 20260826: GATE WC-3 RETIRED.  Its stated reason
    // ("a RecordProcess-backed function that writes files x4") was FALSE ALREADY
    // WHEN WRITTEN and is still false: RecordProcess in this tree resolves to
    // canary_support.cpp:113, a stdout printf stand-in whose own comment says
    // "no DB write"; the DB-writing golden body is #if 0 at cMyDB.cpp:1831
    // pending the GA-1-B4 integrator swap.  ShowXMLOnLine itself landed real in
    // FW-LOTINFO-W27 (this file:4794) and its whole payload is 6 pnlXMLOnLine
    // Caption/Color/Visible writes plus those 4 RecordProcess calls.
    //
    // ⚠ WD-5 LATENCY WARNING STILL STANDS AND IS NOT REMOVED BY THIS OPENING:
    // after the GA-1-B4 swap activates cMyDB.cpp:1831's real body, those 4 calls
    // BECOME DB writes, and this call site is what makes them reachable from
    // FormShow.  See the WD-5 entry in forms/fLotInfo.h and the note on the
    // ShowXMLOnLine body itself.  Gate THEN if that matters -- pre-gating now
    // would make this one call site diverge from the 582 already-translated
    // RecordProcess sites carrying the identical exposure.
    ShowXMLOnLine();                                                            //Steven 20200706 : 移到外面

    // -- S6 (golden :401-410) --
    if(CUSTOMER_CODE==CC_JCET)                                                  //Steven 20170605 (wei) : For長電
        tsFTP->TabVisible=IniConfig.bEnableFTP;
    else
        tsFTP->TabVisible=(CosFunction.bFTPFunction);

    if(IniConfig.bEnable_SECS_GEM ||                                            //Steven 20140701
       CosFunction.bLotStartLockCriticalPara)                                   //JerryYang 20220311 : ATP鎖定Critical parameter
    {
        palSecsGem->Visible=true;
    }

    // -- S7 (golden :412-425) -- WC-4 gates the FormBarcodeReader PopupMenu lines --
    if(CUSTOMER_CODE==CC_KYEC_LEE ||
       (CUSTOMER_CODE==CC_AMD_M && CosFunction.bHiSiliconFunction==true))       //wei 20150826 Lot 強制顯示
    {
        palCurrFailRate->Visible=false;
        Panel27->Visible=false;                                                 //Ifor 20190517 KYEC 不顯示

        // AI(W906-FW3-LotInfo-WC) 20260819: GATE WC-4 -- see forms/fLotInfo.h
        // GATE REGISTER WC-4 (FormBarcodeReader has no port anywhere in this
        // tree; PopupMenu is also not a modeled TControl property).
#if 0
        edtSysLotID->PopupMenu=FormBarcodeReader->pmBarcode;
        edtSysOperatorID->PopupMenu=FormBarcodeReader->pmBarcode;
#endif
        #ifdef SOFT_SIMULTE
            LotKeyInTime->Enabled = false;
        #else
            LotKeyInTime->Enabled = true;                                       //Ifor 20190919 add Lot Info Sacn Time
        #endif
    }

    // -- S8 (golden :427) -- WC-1 (2nd call site, see S3) --
    // AI(W906-FW3-LotInfo-WC) 20260819: GATE WC-1 -- see forms/fLotInfo.h
    // GATE REGISTER WC-1 (same ResetLotInfo() gate as S3; golden calls it
    // twice with no state change between the calls -- a golden oddity, not a
    // translation choice).
#if 1 // AI(W906-FRW-WC1) 20260927 (Steven 團隊): GATE WC-1 RETIRED (see the first call site above)
    fLotInfo->ResetLotInfo();                                                   //Steven 20240925 : 重開軟體時, 要讀回lot info
#endif

    // -- S9 (golden :429-522) -- WC-5 gates btnSaveClick(this) x2; WC-6 gates
    // FileListBox1->Visible=false --
    grpRFID->Visible=(USE_RFID_READER);                                         //Steven 20220713 : RFID Reader for SJSEMI

    if(CUSTOMER_CODE==CC_PTI ||                                                 //RogerYang 20170329 (Steven) 力成 LotID卡關
       CUSTOMER_CODE==CC_TFME_CHINA)
    {
        tsFTP->TabVisible=false;                                                //不知道幹嘛用，先藏起來
    }

    if(CUSTOMER_CODE==CC_SCK)                                                   //ChungHung 20131225 add
    {
        ts_AutoCleanMonitor->TabVisible=true;
    }
    else
    {
        ts_AutoCleanMonitor->TabVisible=false;
    }

    if(CUSTOMER_CODE==CC_AMKOR_Korea)                                           //Steven 20150923
    {
        Label5->Visible=false;
        edTemp->Visible=false;
    }
    else if(CUSTOMER_CODE==CC_SCC ||
            CUSTOMER_CODE==CC_SCK)                                              //ChungHung 20130621 add SCK RMS
    {
        Label5->Visible=false;
        edTemp->Visible=false;
        btDownload->Caption="Download from Handler";
    }
    else if(IniConfig.bSPILFunction==true)                                      //JerryYang 20170328 (Jou) 矽品客戶碼統一用SPILFunction
    {
        chkAutoCleanContactHeight->Checked=CheckAndReadIniData(sConfigPath, "Network", "Auto Clean Contact Height",      false);                                //JerryYang 20241019 : 矽品二林 耀仁要求Auto clean高度可選擇不覆蓋
        btnFtpTester->Visible       =false;
        Label153->Visible           =false;
        edDeviceName->Visible       =false;
        Label5->Visible             =false;
        edTemp->Visible             =false;
        // AI(W906-FW3-LotInfo-WC) 20260819: GATE WC-6 -- see forms/fLotInfo.h
        // GATE REGISTER WC-6 (TFileListBox derives from bare TObject, no
        // ->Visible member).
#if 0
        FileListBox1->Visible       =false;
#endif
        tsDeviceInfo->TabVisible    =true;
        FileListBox1->Directory     =DataPath;
        chkTempOffset->Checked      =false;                                     //JerryYang 20180212 (Steven) SPIL要求鎖死不得修改
        chkContactHigh->Checked     =false;
        chkContactForce->Checked    =true;
        chkContactMode->Checked     =true;
        checkbAutoClean->Checked    =false;                                     //JerryYang 20191003 矽品只還原auto clean offset
        chkHotPlate->Checked        =true;
        chkLoadUnload->Checked      =true;
        chkSpeedSetting->Checked    =true;
        chkShuttleMode->Checked     =true;
        chkTestMode->Checked        =true;
        cbBottom2DOffset->Checked   =false;                                     //JerryYang 20201122 Bottom 2D offset不覆蓋
        chkTestMode->Visible        =false;                                     //JerryYang 20170214 (Steven) 此三項無作用,先不顯示
        chkBinasgn->Visible         =false;
        chkBinasgnOff->Visible      =false;
        groupbDownloadItem->Enabled =false;
        cbRTCASTD->Visible          =true;
        // AI(W906-FW3-LotInfo-WC) 20260819: GATE WC-5 -- see forms/fLotInfo.h
        // GATE REGISTER WC-5 (btnSaveClick() is RECON item #38, (b)
        // write-path, WriteIniData x19 inside).
#if 0
        btnSaveClick(this);                                                     //JerryYang 20180212 (Steven) SPIL要求鎖死不得修改
#endif
    }
    else if(CUSTOMER_CODE==CC_TERAPOWER)
    {
        btnFtpTester->Visible=false;
    }
    else if(CUSTOMER_CODE==CC_AMD_M)
    {
        btnFtpTester->Visible=false;                                            //不顯示
    }
    else if(CUSTOMER_CODE==CC_AMKOR_China ||                                    //jou 2016-06-13 修正 Amkor china download recipe 參數會參照本機
            CUSTOMER_CODE==CC_QUALCOMM)                                         //JerryYang 20170412 (Steven) add QUALCOMM
    {
        chkTempOffset->Checked  =false;
        chkContactHigh->Checked =false;
        chkContactForce->Checked=true;
        chkContactMode->Checked =true;
        chkBinasgn->Visible     =false;                                         //jou 2014-08-26 安靠要求 Auto download Binasgn不顯示並且強制開啟
        chkBinasgnOff->Visible  =false;
        chkBinasgn->Checked     =true;
        chkBinasgnOff->Checked  =true;

        chkContactHigh->Enabled =false;
        chkTempOffset->Enabled  =false;
        chkContactForce->Enabled=false;
        chkContactMode->Enabled =false;

        // AI(W906-FW3-LotInfo-WC) 20260819: GATE WC-5 (2nd call site) --
#if 0
        btnSaveClick(this);
#endif
    }
    else if(CUSTOMER_CODE==CC_ChipMos_ZHUBEI)                                   //Jimmychiu 20250430 : ChipMos 關閉Device info 溫度欄位
    {
        edTemp->Visible=false;
    }
    else
    {
        ART_Panel->Visible=false;
    }
    tsRTCFullViewImg->TabVisible=REAL_TIME_CCD;                                 //Steven 20110825 : Real time CCD - 顯示Full View Image

    // -- S10 (golden :523-541) --
    if(!TrayForm.bEnableAMR) //Eastsun 20260515 F020
        tsKYEC_AMR->TabVisible=false;

    if(CUSTOMER_CODE==CC_ASE_KaohSiung)                                         //kevin 20150706 ART
    {
        GroupBox3->Visible=false;
        ART_Panel->Visible=true;
        ART_Panel->Left=3;
        ART_Panel->Top=5;
    }
    else
    {
        ART_Panel->Visible=false;
        GroupBox3->Visible=true;
        GroupBox3->Left=3;
        GroupBox3->Top=5;
    }

    // -- S11 (golden :543) --
    SetATCFormVisible();                                                        //Steven 20160217 : For ATC7.0  //Ifor 20160516 往下移動，避免ATC頁面關閉後又被打開

    // -- S12 (golden :545-562) -- WC-7 gates the HT9046_LS imgRTCFullView block --
    tsBarCode->TabVisible=(BAR_CODE_INSTALL==ebctUseCCDMode || BAR_CODE_INSTALL==ebctInShtIntel || BAR_CODE_INSTALL==ebctEtherNetCCD);                          //Ifor 20190129 : add Cognex EtherNet 通訊
    btChangeFile->Visible=(BAR_CODE_INSTALL==ebctInShtIntel || BAR_CODE_INSTALL==ebctEtherNetCCD || (BAR_CODE_INSTALL==ebctUseCCDMode && CosFunction.b2DUseSubJobFunction==true));  //wei 20160728 Barcode File切換 //add Sub Job

    tsESDMonitor->TabVisible=false;                                             //Ifor 20160308 ESD 畫面顯示

    ts_OCRInterface->TabVisible=(INSTALL_OCR!=eocrUninstal && CosFunction.bTrayOCR==false);
    ts_SocketInterface->TabVisible=IniConfig.bSocketCommunication;              //ChungHung 20130112 add for ASE_KR Socket Tester

    // AI(W906-FW3-LotInfo-WC) 20260819: GATE WC-7 -- see forms/fLotInfo.h
    // GATE REGISTER WC-7 (TImage/TCanvas have zero port anywhere in this
    // tree, same established WB-1 finding, re-confirmed 20260819).
#if 0
    if(MachineTypeChoice==Type_HT9046_LS)                                       //2013-01-15    Dell
    {
        if(REAL_TIME_CCD==true && COM2->bCCDDummyRum == false)
        {
            imgRTCFullView1->Width = imgRTCFullView1->Width/2;
            imgRTCFullView2->Width = imgRTCFullView2->Width/2;
            imgRTCFullView3->Width = imgRTCFullView3->Width/2;
            imgRTCFullView4->Width = imgRTCFullView4->Width/2;
        }
    }
#endif

    // -- S13 (golden :564-590) --
    tsBarCode->TabVisible=(BAR_CODE_INSTALL==ebctUseCCDMode || BAR_CODE_INSTALL==ebctInShtIntel || BAR_CODE_INSTALL==ebctEtherNetCCD);                          //Ifor 20190129 : add Cognex EtherNet 通訊

    if(CUSTOMER_CODE==CC_TSMC_TAINAN)                                           //ChungHung 20150413 add for TSMC
    {
        IniConfig.bShowLotInfo=true;
        btnDataFTPSaveToData->Visible=true;
        edtSysOperatorID->Visible=false;
        lblOPID->Visible=false;
        spSECSLotCheck->Visible=true;
        sbSECSLotStart->Enabled=false;
        Panel6->Visible=true;
        Panel6->Top=144;
        Panel6->Left=3;
    }
    else
    {
        btnDataFTPSaveToData->Visible=false;
        edtSysOperatorID->Visible=true;
        lblOPID->Visible=true;
        spSECSLotCheck->Visible=false;
        sbSECSLotStart->Enabled=true;
        sbSECSLotEnd->Down=true;
        sbSECSLotStart->Down=false;
        Panel6->Visible=false;
        Panel6->Top=320;
        Panel6->Left=3;
    }

    // -- S14 (golden :592-602) -- WC-8 gates RefreshYieldMonitor() (1st site) --
    if(IniConfig.bSIGURDFunction || CosFunction.bShowYieldMonitor)              //Sam 20210916 : 新增 Yiled Monitor 到畫面上 //Sam 20210324 : 新增 Yield Monitor
    {
        tsYieldMonitior->TabVisible=true;
        tsTPW->TabVisible   =(CUSTOMER_CODE==CC_TERAPOWER || CUSTOMER_CODE==CC_PTI);
        tsSigurd->TabVisible=(CUSTOMER_CODE==CC_SIGURD_PeiXing);
        // AI(W906-FW-LOTINFO-W30) 20260826: GATE WC-8 RETIRED, 1st site (golden
        // :597).  WC-8 was WA-3's reason reused verbatim, so it dies with WA-3 --
        // see the full reachable-chain audit at AdjtsYieldMonitiorSize above.
        RefreshYieldMonitor();
    }
    else
    {
        tsYieldMonitior->TabVisible=false;
    }

    // -- S15 (golden :604-608) -- WC-9 gates fBarCode->mtBarcodeSetDefaultView() --
    // AI(W906-FW3-LotInfo-WC) 20260819: GATE WC-9 -- see forms/fLotInfo.h
    // GATE REGISTER WC-9 (mtBarcodeSetDefaultView has no port anywhere in
    // this tree).
#if 0
    fBarCode->mtBarcodeSetDefaultView();
#endif
    chkTestMode->Visible=CosFunction.bLastSetInSetUpFile;                       //jou 2014-08-25 修正未開啟 CosFunction.bLastSetInSetUpFile LotInfo 顯示錯誤

    labLevelMode->Visible=CosFunction.bDownloadRecipeLevelMode;                 //jou 2016-01-06 download recipe 增加權限模式選擇
    coLevelMode->Visible=CosFunction.bDownloadRecipeLevelMode;                  //jou 2016-01-06 download recipe 增加權限模式選擇

    // -- S16 (golden :610-641) -- zero customer-code branches --------------
    sgBarcode->Cells[ 0][ 0]="Shuttle";
    sgBarcode->Cells[ 1][ 0]="1_A";
    sgBarcode->Cells[ 2][ 0]="1_B";
    sgBarcode->Cells[ 3][ 0]="2_A";
    sgBarcode->Cells[ 4][ 0]="2_B";
    sgBarcode->Cells[ 5][ 0]="Total";
    sgBarcode->Cells[ 0][ 1]="Load";
    sgBarcode->Cells[ 0][ 2]="Pass";
    sgBarcode->Cells[ 0][ 3]="Fail";
    sgBarcode->Cells[ 0][ 4]="Rate(%)";
    sgBarcode->Cells[ 0][ 5]="Retry";
    sgBarcode->Cells[ 0][ 6]="Duplicate";

    sgOCR->Cells[ 0][ 0]="OCR";
    sgOCR->Cells[ 1][ 0]="Tray";
    sgOCR->Cells[ 2][ 0]="Lot";
    sgOCR->Cells[ 0][ 1]="Pass";
    sgOCR->Cells[ 0][ 2]="Key In";
    sgOCR->Cells[ 0][ 3]="No IC";
    sgOCR->Cells[ 0][ 4]="Total";

    sgATRCount->Cells[ 0][ 0]="ATR";
    sgATRCount->Cells[ 1][ 0]="FT";
    sgATRCount->Cells[ 2][ 0]="RT";
    sgATRCount->Cells[ 0][ 1]="Pass";
    sgATRCount->Cells[ 0][ 2]="Fail";
    sgATRCount->Cells[ 0][ 3]="Total";

    // -- S17 (golden :643-649) --
    ShowATCTempPanel();                                                         //Steven 20241112 : 調整ATC溫度顯示
    if(Temperature.bATCActiveCooling==false || CUSTOMER_CODE==CC_KYEC_LEE)      //Ifor 20160902 add 未啟動ATC功能 顯示ATC off line //Ifor 20170609 (wei) add KYEC ATC 不開啟自動連線由人員啟動
    {
        bStartATCRun=false;
        fLotInfo->pl_ATC_Online->Color=clRed;                                   //Ifor 20170609 (wei) add KYEC ATC 不開啟自動連線由人員啟動   //Color 改紅色
        fLotInfo->pl_ATC_Online->Caption="ATC Off Line";                        //Ifor 20170609 (wei) add KYEC ATC 不開啟自動連線由人員啟動   //顯示 ATC Offline
    }

    // -- S18 (golden :651-713) -- WC-21 gates the whole CC_JSCC_OS branch --
    if(CUSTOMER_CODE==CC_PTI &&                                                 //RogerYang 20170417 LotInfo更新至FT Mode
       IniConfig.bB03_TesterReport==false)                                      //Sam 20240809 : PTI ART 模式
    {
        cbRunMode->Enabled=false;
    }

    if(CUSTOMER_CODE==CC_ONSEMI_M)
    {
        labDeviceName->Caption="Device ID";
        labDeviceName->Visible=true;
        edtDevice->Visible=true;
    }

    // AI(W906-FW3-LotInfo-WC) 20260819: GATE WC-21 -- see forms/fLotInfo.h
    // GATE REGISTER WC-21 (edtSysLotID/labLotID/edtDevice/labDeviceName/
    // btnFTPDownLoadbyDeviceID geometry + fTesterTCP->rgUnloader, all
    // unavailable/unverified this wave).
#if 0
    if(CUSTOMER_CODE==CC_JSCC_OS)                                               //RogerYang 20260127 : Add For JSCC_OS download by Device list
    {
        int iTopTmp=edtSysLotID->Top;
        edtSysLotID->Top=Panel28->Top+2;
        labLotID->Top=edtSysLotID->Top+4;
        edtDevice->Top=iTopTmp;
        labDeviceName->Top=edtDevice->Top+4;
        btnFTPDownLoadbyDeviceID->Top=edtDevice->Top-2;
        Panel28->Top=edtSysLotID->Top+20;
        labDeviceName->Caption="Device ID";
        labDeviceName->Visible=true;
        edtDevice->Visible=true;
        btnFTPDownLoadbyDeviceID->Visible=true;
        rgUnloader->ItemIndex=fTesterTCP->rgUnloader->ItemIndex;
    }
#endif

    if(CUSTOMER_CODE==CC_SIGURD_ChungXing)                                      //Ifor 20170505 (wei) add 矽格中興使用LotID不顯示Run Mode
    {
        cbRunMode->Visible      =false;
        lbLotRunMode->Visible   =false;
    }

    if(CosFunction.bOEEFunction)                                                //Steven 20180417 (Jou) : OEE功能
    {
        labLotID->Visible=false;                                                //JerryYang 20220923 : add
        edtSysLotID->Visible=false;
        lblOPID->Visible=false;
        edtSysOperatorID->Visible=false;
        lbLotRunMode->Visible=false;
        cbRunMode->Visible=false;

        sbSECSLotStart->Caption="Start Lot";                                    //Sam 20170925 : 顯示名稱修改
        sbSECSLotEnd->Caption="End Lot";
        btnFtpTester->Visible=false;                                            //Sam 20171006 (wei) : 超豐用不到隱藏起來
        btnFtpServer->Caption ="Download To Handler";                           //Sam 20171006 (wei) : 超豐要求顯示名稱修改
        btnFtpHD->Caption="Upload To Server";                                   //Sam 20171006 (wei) : 超豐要求顯示名稱修改
        sb_RunExecutFile->Visible=true;
        if(IniConfig.asA25RunExecutButtonName!="")
        {
            sb_RunExecutFile->Caption=IniConfig.asA25RunExecutButtonName;
        }
        else
        {
            sb_RunExecutFile->Caption="Run Execut";
        }
    }

    // -- S19 (golden :715-978) -- WC-11/WC-12/WC-13/WC-14/WC-15 (see header) --
    tsMurata->TabVisible    =(CUSTOMER_CODE==CC_Murata);
    edtLine->Visible        =(CUSTOMER_CODE==CC_Murata);

    edtProcessName->Visible =(CUSTOMER_CODE==CC_Murata || IniConfig.bVTESTFunction==true);
    edtProduct->Visible     =(CUSTOMER_CODE==CC_Murata || IniConfig.bVTESTFunction==true);
    tsVTest->TabVisible     =(IniConfig.bVTESTFunction==true);

    if(CUSTOMER_CODE==CC_ASE_KaohSiung_K12)                                     //jou 20191008 : (Steven) add SCC使用Lot ID
    {
        cbRunMode->Items->Clear();
        cbRunMode->Items->Add("FT");
        cbRunMode->Items->Add("RT1");
        cbRunMode->Items->Add("RT2");
        cbRunMode->Text="FT";
    }
    else if(IniConfig.bVTESTFunction==true)
    {
        // AI(W906-FW3-LotInfo-WC) 20260819: GATE WC-12 -- see forms/fLotInfo.h
        // GATE REGISTER WC-12 (pgcLotInfo is the stock TPageControl, no
        // ->ActivePage pointer member).
#if 0
        pgcLotInfo->ActivePage=tsVTest;
#endif
        cbRunMode->Items->Clear();
        cbRunMode->Items->Add("FT0");                                           //RogerYang 20260420 : 何航航要求增加站點(第二次修改)
        cbRunMode->Items->Add("FT1");
        cbRunMode->Items->Add("FT2");
        cbRunMode->Items->Add("FT3");
        cbRunMode->Items->Add("FT4");
        cbRunMode->Items->Add("FT5");
        cbRunMode->Items->Add("FT6");
        cbRunMode->Items->Add("FT7");
        cbRunMode->Items->Add("FT8");
        cbRunMode->Items->Add("FT9");
        cbRunMode->Items->Add("FT10");
        cbRunMode->Items->Add("FT11");
        cbRunMode->Items->Add("FT12");
        cbRunMode->Items->Add("FT13");
        cbRunMode->Items->Add("FT14");
        cbRunMode->Items->Add("FT15");
        cbRunMode->Items->Add("RT0");                                           //RogerYang 20260420 : 何航航要求增加站點
        cbRunMode->Items->Add("RT1");
        cbRunMode->Items->Add("RT2");
        cbRunMode->Items->Add("RT3");
        cbRunMode->Items->Add("RT4");
        cbRunMode->Items->Add("RT5");
        cbRunMode->Items->Add("EQC0");                                          //RogerYang 20260420 : 何航航要求增加站點
        cbRunMode->Items->Add("EQC1");
        cbRunMode->Items->Add("EQC2");
        cbRunMode->Items->Add("EQC3");
        cbRunMode->Items->Add("EQC4");
        cbRunMode->Items->Add("EQC5");
        cbRunMode->Text="FT1";

        cbTestTimes->Items->Clear();                                            //RogerYang 20250809 偉測Summary文件修改
        cbTestTimes->Items->Add("RP0");
        cbTestTimes->Items->Add("RP1");
        cbTestTimes->Items->Add("RP2");
        cbTestTimes->Items->Add("RP3");
        cbTestTimes->Items->Add("RP4");
        cbTestTimes->Items->Add("RP5");
        cbTestTimes->Items->Add("RP6");
        cbTestTimes->Items->Add("RP7");
        cbTestTimes->Items->Add("RP8");
        cbTestTimes->Items->Add("RP9");
        cbTestTimes->Items->Add("RP10");
        cbTestTimes->Items->Add("RP11");
        cbTestTimes->Items->Add("RP12");
        cbTestTimes->Items->Add("RP13");
        cbTestTimes->Items->Add("RP14");
        cbTestTimes->Items->Add("RP15");
        cbTestTimes->Text="RP0";

        lbProcess->Visible=true;
        cbProcess->Visible=true;

        grpMesCheck->Visible=true;

        // AI(W906-FW3-LotInfo-WC) 20260819: GATE WC-13 -- see forms/fLotInfo.h
        // GATE REGISTER WC-13 (control re-parenting via ->Parent= has no
        // model anywhere in vclcompat/Controls.h).
#if 0
        edtProcessName->Parent=tsVTest;
#endif
        edtProcessName->EditLabelCaption="CustPart";
        edtProcessName->Left=70;
        edtProcessName->Top=5;
        edtProcessName->Enabled=true;

#if 0
        edtProduct->Parent=tsVTest;
#endif
        edtProduct->EditLabelCaption="CustLotNum";
        edtProduct->Left=70;
        edtProduct->Top=edtProcessName->Top+27;
        edtProduct->Enabled=true;

        // AI(W906-FW3-LotInfo-WC) 20260819: GATE WC-14 -- see forms/fLotInfo.h
        // GATE REGISTER WC-14 (sbSECSLotStart/sbSECSLotEnd are Wave A/B
        // TSpeedButton* members with no geometry field).
#if 0
        sbSECSLotStart->Top=lbLotRunMode->Top+lbLotRunMode->Height+55;
        sbSECSLotEnd->Top=sbSECSLotStart->Top+35;
#endif

        btnFtpTester->Visible=false;

        labConfigL04->Visible=true;

        chkTempOffset->Enabled=false;
        chkTempOffset->Checked=false;
        // AI(W906-FW3-LotInfo-WC) 20260819: GATE WC-15 -- see forms/fLotInfo.h
        // GATE REGISTER WC-15 (WriteIniData embedded write-path, same class
        // as WC-1/WC-2/WC-5).
#if 0
        WriteIniData(AuthPath+"Security_new.def", "Network", "Temp Offset",     false);
#endif
        checkbAutoClean->Enabled=false;
        checkbAutoClean->Checked=false;
#if 0
        WriteIniData(AuthPath+"Security_new.def", "Network", "Auto Clean",     false);
#endif

        grpOEEState->Visible=true;
        grpOEEState->Left=0;
        sgOEEState->Cells[0][0]="OEE";
        sgOEEState->Cells[0][1]="TimeOEE";
        sgOEEState->Cells[0][2]="Jam";
        sgOEEState->Cells[0][3]="Retest";
        sgOEEState->Cells[0][4]="Down";
        sgOEEState->Cells[0][5]="Setup";
        sgOEEState->Cells[0][6]="Idle";
        sgOEEState->Cells[0][7]="ENG";
        sgOEEState->Cells[0][8]="PM";
        sgOEEState->Cells[0][9]="Stop";
    }
    else if(CUSTOMER_CODE==CC_Murata)
    {
        cbRunMode->Items->Clear();
        cbRunMode->Items->Add("FT");
        cbRunMode->Items->Add("RT");
    }
    else if(IniConfig.bSPILFunction==true && LastSet.iTester==_2D_SORT)         //JerryYang 20230322 : 2D sort lot info UI修改
    {
        cbRunMode->Items->Clear();
        cbRunMode->Items->Add("VS");
        cbRunMode->Items->Add("RC1");
        cbRunMode->Items->Add("RC2");
        labCusLotID->Visible=true;
        labCusDevGrp->Visible=true;
        labDeviceName->Visible=true;
        edtCusLotID->Visible=true;
        edtCusDevGrp->Visible=true;
        edtDevice->Visible=true;
        edtCusLotID->Enabled=false;                                             //JerryYang 20230322 : 2D sort lot info UI修改
        edtCusDevGrp->Enabled=false;
        labCusStep->Visible=true;                                               //JerryYang 20260201 : add
        edtCusStep->Visible=true;
        edtCusStep->Enabled=true;
    }
    else if(CUSTOMER_CODE==CC_PANTHER)
    {
        cbRunMode->Items->Clear();
        cbRunMode->Items->Add("Normal Test");
        cbRunMode->Items->Add("Pre-Test");
        cbRunMode->Items->Add("Re-Test");
        cbRunMode->Items->Add("GD");
        cbRunMode->Items->Add("EQC");
        labLotID->Visible=false;
        edtSysLotID->Visible=false;
        lblOPID->Visible=false;
        edtSysOperatorID->Visible=false;
        lbLotRunMode->Visible=false;
        cbRunMode->Visible=false;
        pgLotinfo->ActivePage=tsLotID;
    }
    else if(TestIF_File.b2DIDAllowList &&                                       //JerryYang 20241104 : 支援2DID白名單功能
            IniConfig.iN23DownloadMethod!=2)                                    //JerryYang 20250320 : 2DID白名單功能
    {
        if(CUSTOMER_CODE==CC_JCET)                                              //RogerYang 20251208 : JCET 2D FT1白名單/FT2比對功能
        {
            // AI(W906-FW3-LotInfo-WC) 20260819: GATE WC-11 -- see
            // forms/fLotInfo.h GATE REGISTER WC-11 (guard `bFlag` depends
            // entirely on the WA-7-gated fBarCode->
            // JCETUseMakeWhite2DIDList()).
#if 0
            bool bFlag=false;
            bFlag=fBarCode->JCETUseMakeWhite2DIDList();
            if(bFlag==true && cbRunMode->Enabled==true)
            {
                cbRunMode->Items->Clear();
                cbRunMode->Items->Add("FT1");
                cbRunMode->Items->Add("FT2");
                cbRunMode->Items->Add("FT3");
                cbRunMode->Items->Add("FT4");
                cbRunMode->Items->Add("FT5");
                cbRunMode->Items->Add("FT6");
                cbRunMode->Items->Add("FT7");
                cbRunMode->Items->Add("FT8");
                cbRunMode->Items->Add("FT9");
            }
            edtSysOperatorID->Visible=true;
            lblOPID->Visible=true;
            bFlag=fBarCode->JCETUseMakeWhite2DIDList();
            labCusLotID->Visible=bFlag;
            edtCusLotID->Visible=bFlag;
#endif
        }
        else
        {
            edtSysOperatorID->Visible=false;
            lblOPID->Visible=false;
            labCusLotID->Visible=false;                                         //RogerYang 20251215 : JCET 2D FT1白名單/FT2比對功能
            edtCusLotID->Visible=false;                                         //RogerYang 20251215 : JCET 2D FT1白名單/FT2比對功能
        }
        labDeviceName->Visible=false;
        edtDevice->Visible=false;
        lbProcess->Visible=false;
        cbProcess->Visible=false;
    }
    else if(CUSTOMER_CODE==CC_SCC ||                                            //Steven 20250314 : add for JSCC
            CUSTOMER_CODE==CC_SJ_Semiconductor)
    {
        cbRunMode->Items->Clear();
        cbRunMode->Items->Add("P1");
        cbRunMode->Items->Add("P2");
        cbRunMode->Items->Add("P3");
        cbRunMode->Items->Add("P4");
        cbRunMode->Items->Add("P5");
        cbRunMode->Items->Add("P6");
        cbRunMode->Items->Add("P7");
        cbRunMode->Items->Add("P8");
        cbRunMode->Items->Add("P9");
        cbRunMode->Items->Add("P10");
        cbRunMode->Items->Add("P11");
        cbRunMode->Items->Add("P12");
        cbRunMode->Items->Add("P13");
        cbRunMode->Items->Add("P14");
        cbRunMode->Items->Add("P15");
        cbRunMode->Items->Add("RT1");
        cbRunMode->Items->Add("RT2");
        cbRunMode->Items->Add("RT3");
        cbRunMode->Items->Add("RT4");
        cbRunMode->Items->Add("RT5");
        cbRunMode->Items->Add("RT6");
        cbRunMode->Items->Add("RT7");
        cbRunMode->Items->Add("RT8");
        cbRunMode->Items->Add("RT9");
        cbRunMode->Items->Add("RT10");
        cbRunMode->Items->Add("RT11");
        cbRunMode->Items->Add("RT12");
        cbRunMode->Items->Add("RT13");
        cbRunMode->Items->Add("RT14");
        cbRunMode->Items->Add("RT15");
        cbRunMode->Text="P1";
    }
    else if(CUSTOMER_CODE==CC_PTI && IniConfig.bB03_TesterReport)               //Sam 20240809 : PTI ART 模式
    {
        lbProcess->Visible=true;
        cbProcess->Visible=true;
        cbProcess->Items->Clear();
        cbProcess->Items->Add("Sample");
        cbProcess->Items->Add("100%");
        cbProcess->Text="100%";

        cbRunMode->Items->Clear();
        cbRunMode->Items->Add("1'st");
        cbRunMode->Items->Add("2'nd");
        cbRunMode->Items->Add("3'th");
        cbRunMode->Items->Add("4'th");
        cbRunMode->Items->Add("5'th");
        cbRunMode->Items->Add("6'th");
        cbRunMode->Items->Add("7'th");
        cbRunMode->Items->Add("8'th");
        cbRunMode->Items->Add("9'th");
        cbRunMode->Items->Add("10'th");
        cbRunMode->Items->Add("11'th");
        cbRunMode->Items->Add("12'th");
        cbRunMode->Items->Add("13'th");
        cbRunMode->Items->Add("14'th");
        cbRunMode->Items->Add("15'th");
        cbRunMode->Items->Add("16'th");
        cbRunMode->Items->Add("17'th");
        cbRunMode->Items->Add("18'th");
        cbRunMode->Items->Add("19'th");
        cbRunMode->Items->Add("20'th");
        cbRunMode->Text="1'st";
    }

    // -- S20 (golden :980-1013) --
    if(CUSTOMER_CODE==CC_GIGAS)
    {
        btnFtpTester->Visible=false;                                            //沒用到
        btnFTPTryConnect->Visible=true;                                         //測試FTP是否有連線(按鈕)
        lbFTPStatus->Visible=true;                                              //測試FTP是否有連線(狀態顯示)
        btnFtpServer->Caption ="Barcode Download";                              //Server->Barcode Download
        btnFtpHD->Caption="HD Upload";                                          //HD->HD Upload
        // AI(W906-FW3-LotInfo-WC) 20260819: GATE WC-28 -- see forms/fLotInfo.h
        // GATE REGISTER WC-28 (BtnPause is TfMainSpeedButton*, no ->Visible).
#if 0
        BtnPause->Visible=false;
#endif
        if(IniConfig.bEnableFTP==true)
        {
            TCheckBox *tempTCheckBox[17]=
            {
                chkContactHigh,chkContactForce,chkContactMode,checkbAutoClean,chkART,
                chkART_RTCount,chkCleanCount,chkHotPlate,chkLoadUnload,chkSpeedSetting,
                chkShuttleMode,chkTestMode,chkBinasgn,chkBinasgnOff,chkIndexHeatingMode,
                cbBottom2DOffset,chkAutoCleanContactHeight
            };
            for(int i=0;i<17;i++)
            {
                tempTCheckBox[i]->Visible=false;
                tempTCheckBox[i]->Checked=true;
            }
            grpMesCheck->Visible=false;
        }
        tsLotID->TabVisible=false;
    }
    else if(CUSTOMER_CODE==CC_KYEC_LEE)                                         //Ifor 20251028 add: KLT 要求不顯示畫面
    {
        // AI(W906-FW3-LotInfo-WC) 20260819: GATE WC-28 (2nd site) --
#if 0
        BtnPause->Visible=false;
#endif
    }
    else
    {
        // AI(W906-FW3-LotInfo-WC) 20260819: GATE WC-28 (3rd site) --
#if 0
        BtnPause->Visible=(CUSTOMER_CODE==CC_FOREHOPE_NINGBO);                  //Steven 20251125 : change
#endif
    }

    // -- S21 (golden :1014-1021) --
    tsASEMARMS->TabVisible=CosFunction.bUseARMSFunction;                        //Ifor 20170621 (wei) add ARMS Function
    tsASECLEventLog->TabVisible=(CUSTOMER_CODE==CC_ASE_CL ||
                                 CUSTOMER_CODE==CC_HANA_MICRON ||               //JimmyChiu 20211008 R211005-Hana-H9-01
                                 CUSTOMER_CODE==CC_SJ_Semiconductor ||          //Steven 20221128 : Add Socket Id for SJSM
                                 CosFunction.bUseSocketContactCount);

    pnlLotInfo_ASECL->Visible=(CUSTOMER_CODE==CC_ASE_CL);                       //Steven 20221214 : fixed for SJSM
    pnlLotStart_ASECL->Visible=(CUSTOMER_CODE==CC_ASE_CL);

    // -- S22 (golden :1023-1032) -- RefreshAMR/ShowAMRCategoryBin already Tier1 real (Wave B) --
    if(CosFunction.bEnableHandlerResultServer)
    {
        tsAMR->TabVisible=true;
        RefreshAMR();
        ShowAMRCategoryBin(true);
    }
    else
    {
        tsAMR->TabVisible=false;
    }

    // -- S23 (golden :1034-1043) -- edSocket/LastSet.strSocketID confirmed REAL --
    if(CUSTOMER_CODE==CC_ASE_CL)                                                //JerryYang 20250120 : add
    {
         for(int i=0; i<4; i++)
        {
            for(int j=0; j<8; j++)
            {
                edSocket[i][j]->Text=AnsiString(LastSet.strSocketID[i][j]);
            }
        }
    }

    // -- S24 (golden :1045-1067) --
    if(IniConfig.bEnableFTP)
    {
        if((CUSTOMER_CODE==CC_JSCC_OS))                                         //RogerYang 20260127 : Add For JSCC_OS 田揚志說LOTInfo擺第一頁
        {
            pgLotinfo->ActivePage=tsLotID;
        }
        else
        {
            pgLotinfo->ActivePage=tsFTP;
        }
    }
    else if(IniConfig.bEnableRms==true)
        pgLotinfo->ActivePage=tsDeviceInfo;
    else if(CosFunction.bOEEFunction)
        pgLotinfo->ActivePage=tsLotID;
    else if(REAL_TIME_CCD)
        pgLotinfo->ActivePage=tsRTCFullViewImg;
    else if(ATC_SYSTEM==eWinWay && Temperature.bATCActiveCooling==true)         //jimmychiu 20210906
        pgLotinfo->ActivePage=ATC_WinWay;
    else if(ATC_SYSTEM!=eATCUninstall && ATC_SYSTEM!=eNonChamber)
        pgLotinfo->ActivePage=tsATC;
    else if(Tri_Temp_Machine==1)                                                //Ztex 2023.04.19 Add HT-1032 TriTemp Function
        pgLotinfo->ActivePage=ts_ATC6_1;

    // -- S25 (golden :1069-1118) -- WC-22 gates edtSysLotID->Width/->Left;
    // WC-23 gates labQACount->Font->Size=8 --
    if(IniConfig.bSPILFunction==true)                                           //JerryYang 20190701 SPIL要求主畫面可切換EQC mode
    {
        lblPage->Visible=false;
        edPage->Visible=false;
        if(LastSet.iTester!=_2D_SORT && TestIF_File.b2DIDAllowList==false)      //JerryYang 20241104 : 支援2DID白名單功能  //JerryYang 20230322 : 2D sort lot info UI修改
        {
            palQAMode->Visible=true;
            labLotID->Caption="SPIL LOT ID-STAGE-STEP";
            // AI(W906-FW3-LotInfo-WC) 20260819: GATE WC-22 -- see
            // forms/fLotInfo.h GATE REGISTER WC-22 (edtSysLotID is a Wave
            // A/B member with no geometry field).
#if 0
            edtSysLotID->Width=150;
            edtSysLotID->Left=150;
#endif
            lbLotRunMode->Visible=false;
            cbRunMode->Visible=false;
        }
        else
        {
            if(TestIF_File.b2DIDAllowList==true)                                //JerryYang 20241104 : 支援2DID白名單功能
            {
                lbLotRunMode->Visible=true;
                cbRunMode->Visible=true;
                cbRunMode->Items->Clear();
                cbRunMode->Items->Add("Normal");
                cbRunMode->Items->Add("RT");
                cbRunMode->Items->Add("EQC");
                cbRunMode->Items->Add("CORR");
            }
            else
            {
                lbLotRunMode->Visible=true;
                cbRunMode->Visible=true;
            }

            labJobSeq->Visible=false;
            edtJobSeq->Visible=false;
        }
    }
    else if(CUSTOMER_CODE==CC_KYEC_LEE && IniConfig.bQAMode==true)
    {
        labQACount->Caption="EQC Mode Device Counts:";
        // AI(W906-FW3-LotInfo-WC) 20260819: GATE WC-23 -- see
        // forms/fLotInfo.h GATE REGISTER WC-23 (TLabel carries no Font
        // member).
#if 0
        labQACount->Font->Size=8;
#endif
        edQAMode->Left=136;
        btnQAmodeSave->Left=210;
        palQAMode->Visible=true;
    }
    else
    {
        palQAMode->Visible=false;
        labLotID->Caption="Lot ID : ";                                          //JerryYang 20220923 : add
    }

    // -- S26 (golden :1120) --
    ShowSocketID();                                                             //JerryYang 20190702 ASE-CL顯示SocketID

    // -- S27 (golden :1122-1123) -- DewPoint_Hardware_Install confirmed REAL --
    pan_DewPoint->Visible=(DewPoint_Hardware_Install>0);                        //Steven 20191017 : 露點計
    pl_DewPoint->Visible=(DewPoint_Hardware_Install>0);                         //Steven 20191017 : 露點計

    // -- S28 (golden :1125-1146) -- WC-18 gates the whole segment --
    // AI(W906-FW3-LotInfo-WC) 20260819: GATE WC-18 -- see forms/fLotInfo.h
    // GATE REGISTER WC-18 (myInShuttleLotInfo/mtBarcodeInShLotInfo are not
    // members of any ported TfLotInfo instance anywhere in this tree).
#if 0
    myInShuttleLotInfo->SetColorMap(0, TColor(0x00DFD9CC));
    myInShuttleLotInfo->SetColorMap(1, clGreen);
    myInShuttleLotInfo->SetColorMap(2, clRed);
    myInShuttleLotInfo->SetColorMap(3, clBtnFace);

    mtBarcodeInShLotInfo->SetColorMap(0, TColor(0x00DFD9CC));
    mtBarcodeInShLotInfo->SetColorMap(1, clGreen);
    mtBarcodeInShLotInfo->SetColorMap(2, clRed);
    mtBarcodeInShLotInfo->SetColorMap(3, clBtnFace);

    myInShuttleLotInfo->SetCellNumber(0, 0, "In Shuttle");
    myInShuttleLotInfo->SetCellColorIndex(0, 0, 3);
    myInShuttleLotInfo->SetCellNumber(0, 1, "a");
    myInShuttleLotInfo->SetCellColorIndex(0, 1, 3);
    myInShuttleLotInfo->SetCellNumber(0, 2, "b");
    myInShuttleLotInfo->SetCellColorIndex(0, 2, 3);
    myInShuttleLotInfo->SetCellNumber(0, 3, "c");

    mtBarcodeInShLotInfo->SetCellNumber(0, 0, "Shuttle1");
    mtBarcodeInShLotInfo->SetCellColorIndex(0, 0, 3);
    mtBarcodeInShLotInfo->SetCellNumber(1, 0, "Shuttle2");
    mtBarcodeInShLotInfo->SetCellColorIndex(1, 0, 3);
#endif

    // -- S29 (golden :1147-1154) --
    if(TestIF_File.i2DIDFormat==eAMD)                                           //JerryYang 20200422 2DID format選項改用下拉選單
    {
        grpBarcodeDisplayLotInfo->Visible=true;
    }
    else
    {
        grpBarcodeDisplayLotInfo->Visible=false;
    }

    // -- S30 (golden :1156-1158) -- WC-19 gates tmrChamberBoost->Enabled and
    // SetLotStart(); sPath (golden :1157) stays REAL between them -- see
    // forms/fLotInfo.h GATE REGISTER WC-19's own note on why. --
    // AI(W906-FW3-LotInfo-WC) 20260819: GATE WC-19 -- SAFETY RED LINE,
    // SetLotStart is the 399-line SECS Lot-Start main control function
    // (RECON #10); not re-evaluated.
#if 0
    tmrChamberBoost->Enabled=(CosFunction.bUseChamberBoostMode);                //Steven 20191128 : Chamber Boost Function
#endif
    AnsiString sPath=AuthPath+"config.ini";
#if 0
    SetLotStart("fLotInfo::FormShow", true);                                    //Steven 20250515 : 整合Open Short測試報表
#endif

    // -- S31 (golden :1160) --
    edtJobSeq->Text=ReadIniData(sPath, "Lot Info", "Job Sequence", AnsiString(""));                                     //JerryYang 20220923 : add

    // -- S32 (golden :1166-1192) -- WC-20 gates the whole segment (SAFETY) --
    // AI(W906-FW3-LotInfo-WC) 20260819: GATE WC-20 -- SAFETY RED LINE,
    // critical-para access-control + audit-flag cluster; not re-evaluated.
#if 0
    if(edtSysLotID->Text!="" ||
       (CosFunction.bLotStartLockCriticalPara &&                                //JerryYang 20220311 : ATP鎖定Critical parameter
        LastSet.iTester==ON_LINE &&
        (fMain->CheckCanChangeRealDummy()==false ||
         HasICUnderMachine())))
    {
        fMain->cbRunStartMode->Enabled=false;
        edtBarcodeRecipe->Enabled=false;                                        //Ifor 20241108 add:Barcode Multi Recipe   //Eastsun 20260527 整合 #027-2.MR.U1 edtBarcodeRecipe disable :KYEC

        if(CosFunction.bUseLogUploadToFTPFunction==true)                        //Steven 20240925 : Fixed for log uoload
        {
            bSysLotStart      =true;                                            //Ifor 20160302 add for KYEC_HS LotStart
            bEPLogStart_KYEC  =true;                                            //Ifor 20160302 add for KYEC_HS EPLogStart
            bTempLogStart_KYEC=true;                                            //Ifor 20160302 add for KYEC_HS TempLogStart
            if(USE_NOVX3360==true)
            {
                bESDLogStart_KYEC=true;                                         //Ifor 20160302 add for KYEC_HS ESD
            }
            asATCEvenLotID=fLotInfo->edtSysLotID->Text;                         //Ifor 20170124 (Steven) :add LotID By ATC Even Log
            bArmTestInfoEvenLogStart_KYEC=true;                                 //Ifor 20190912 :add 海思 V02.30 版 Record Torque
        }
    }

    if(CosFunction.bLotStartLockCriticalPara && RunInfo.bLotStart)              //JerryYang 20220311 : ATP鎖定Critical parameter
    {
        edQAMode->Enabled=!bAuthCriticalPara[14];
    }
#endif

    // -- S33 (golden :1194-1219) --
    if(CUSTOMER_CODE==CC_ASE_SG)
    {
        btnClearTemperature->Visible=true;
    }
    else
    {
        btnClearTemperature->Visible=false;
    }

    if(CUSTOMER_CODE==CC_LEADYO)                                                //KenHsieh 20230406 : 新增OCR Data + Bin Log功能
    {
        lblPage->Visible=false;
        edPage->Visible =false;
        Panel28->Visible=false;
    }

    if(CosFunction.bFirstTrayCheckOnUnloader)                                   //Jimmychiu 20251205 : First Tray Check On Unloader
    {
        tsOtherTool->TabVisible=true;
        cbFirstTrayCheckOnUnloader->Visible=true;
    }
    else
    {
        tsOtherTool->TabVisible=false;
        cbFirstTrayCheckOnUnloader->Visible=false;
    }
    // -- connective (golden :1220-1221) -- see forms/fLotInfo.h's note on the
    // S33/S34 boundary (2 real lines recon's own segment table doesn't cover) --
    lbShowDevName->Visible=CosFunction.bScanBarcodeAndDownloadFileInRMS;
    lbShowDevName->Caption="";

    // -- S34 (golden :1223-1229) -- WC-16 gates ReadWriteFTPAutomationData() --
    if(iAQLBin==0)                                                              //Eastsun 20260520 整合
        iAQLBin=-1;                                                             //Eastsun 20260520 整合
    fLotInfo->lbLotAQLSetCount->Caption=IntToStr(iAQLCount);                    //Eastsun 20260520 整合
    fLotInfo->lbLotAQLSetBin->Caption=IntToStr(iAQLBin);                        //Eastsun 20260520 整合

    // AI(W906-FW3-LotInfo-WC) 20260819: GATE WC-16 -- see forms/fLotInfo.h
    // GATE REGISTER WC-16 (ReadWriteFTPAutomationData has zero port anywhere
    // in this tree, overturning recon's own S34 note).
#if 0
    ReadWriteFTPAutomationData(true);                                           //KaiChen 20190530 ：Sigurd FTP Automation
#endif
    AdjtsYieldMonitiorSize();                                                   //Steven 20221225 : 統一Lot Info尺寸調整
    InitialRefrigerantSystem();                                                 //Ztex 2023.04.19 Add HT-1032 TriTemp Function

    // -- S35 (golden :1231-1233) -- WC-17 gates FrmAOI->AOIFailCountRefresh() --
    // AI(W906-FW3-LotInfo-WC) 20260819: GATE WC-17 -- see forms/fLotInfo.h
    // GATE REGISTER WC-17 (FrmAOI carries only bSimulateTopBtm, no
    // AOIFailCountRefresh() method).
#if 0
    if(USE_Scanner_AOI_Inspection==(int)eBtnAOI_TopBottomInstall && IniConfig.bA74AOIFailCountLinkLotRunMode)
        FrmAOI->AOIFailCountRefresh();                 //Eastsun 20260408 :
#endif
}

// -- Timer2Timer (golden uLotInfo.cpp:6934-7191) -----------------------------
void TfLotInfo::Timer2Timer()
{
    // -- T1 (golden :6936-6939) -- iTempSetTimer moved into the WC-24 gate --
    static bool bOldFTPAutomation=false;
    if(InitialOK==false)                                                        //Steven 20160912 : Add InitialOK in Timer
        return;

    // -- T2 (golden :6941-6945) --
    if(CUSTOMER_CODE==CC_TSMC_TAINAN && bSecsGemDownloadFTP==true)              //wei 20170119 (Steven) DownLoad 沒有馬上按掉會Time Out
    {
        if(IniConfig.bEnable_SECS_GEM==true)
            ShowMyMessage("DOWNLOAD_RECIPE_BY_EA Finish!!");
    }

    // -- T3 (golden :6947-6950) --
    if(CUSTOMER_CODE==CC_KYEC_XILINX)                                           //Frank 20171030 (Steven) add Clear Barcode List新增權限 Xilinx
    {
        btClearBarcodeList->Visible=(AccessLevel>=iDefHonPrecLevel)?true:false;
    }

    // -- T4 (golden :6952-6956) -- fAGV->IsSPIL_AMR() confirmed REAL, step 0 --
    if(fAGV->IsSPIL_AMR())
    {
        labLoaderBundleID->Caption=asBundleTrayID[ePortLoader];
        lblLoaderCarBundleID->Caption=asBundleTrayID[ePortEmpty];
    }

    // -- T5 (golden :6958-6985) --
    bSecsGemDownloadFTP=false;

    if(INSTALL_OCR!=eocrUninstal && TestIF.bOcrFunction==true && bOCRConnectTest==true)                                 //wei 20160613 ocr連線測試
    {
        bOCRConnectTestCount++;

        if(bOCRConnectTestCount>3)                                              //wei 20161028 ocr連線測試
        {
            if(bOCRConnectOK)
            {
                ShowMyMessage("OCR Connect Test OK");
                bSignIn=true;
                bOCRConnect=true;
            }
            else
            {
                ShowMyMessage("OCR Connect Test NG");
                bSignIn=false;
                bOCRConnect=false;
            }
            bOCRConnectTest=false;
            bOCRConnectTestCount=0;
        }
        else
        {
            return;
        }
    }

    // -- T6 (golden :6987-7016) -- fMain->cbSetupFileName confirmed REAL --
    lbOCRUseFile->Caption="";
    lbOCRNowFile->Caption="";

    lblAutoCount->Caption=iUnloaderTrayCountCal[0];
    lblAutoCount2->Caption=iUnloaderTrayCountCal[1];
    lblAutoCount3->Caption=iUnloaderTrayCountCal[2];

    if(INSTALL_OCR!=eocrUninstal && TestIF.bOcrFunction==true)
    {
        if(bSignIn)
        {
            Label41->Caption="Log In";
        }
        else
        {
            Label41->Caption="Log Out";
        }
        lbOCRUseFile->Caption=fMain->cbSetupFileName->Text;
        if(asCheckFileName!="")
            lbOCRNowFile->Caption=asCheckFileName;
        else
            lbOCRNowFile->Caption="";
    }
    else
    {
        Label41->Caption="Log Out";
        lbOCRUseFile->Caption=fMain->cbSetupFileName->Text;
        lbOCRNowFile->Caption="";
    }

    // -- T7 (golden :7017-7029) -- WC-26/WC-27 gate the fOCR/fSetup lines --
    if(bGetLotIDFormTester)
    {
        fLotInfo->palHandlerwithTester->Caption="Connection";
        fLotInfo->palHandlerwithTester->Color=clLime;
        // AI(W906-FW3-LotInfo-WC) 20260819: GATE WC-26 -- see forms/fLotInfo.h
        // GATE REGISTER WC-26 (fOCR->sTesterLotId lives only on OCRInsp.cpp's
        // TU-local shim, unreachable here -- same TfOCR shape as WB-6, a
        // different absent member).
#if 0
        lblTester_LotID->Caption= fOCR->sTesterLotId;
#endif
    }
    else
    {
        fLotInfo->palHandlerwithTester->Caption="No Connection";
        fLotInfo->palHandlerwithTester->Color=clRed;
        // AI(W906-FW3-Setup-WA) 20260820: WC-27 RETIRED -- the cSetUp display
        // wave landed the real fSetup->edOcrText (forms/fSetup.h:271), so the
        // gate's cause is gone and golden :7027 is live again. History in
        // forms/fLotInfo.h's GATE REGISTER.
        lblTester_LotID->Caption=fSetup->edOcrText->Text;
    }

    // -- T8 (golden :7030-7034) --
    ts_AutoRetestMonitor->TabVisible=(CosFunction.bUseSCKART==false &&
                                      USE_AUTO_RETEST==eartInstall &&
                                      ((IniConfig.bA10_AutoReTest && (CosFunction.bAutoRetestGPIBmode==false || CUSTOMER_CODE==CC_KYEC_XILINX)) || bAutoReTest_ART));  //kevin 20150601);       //wei 20150331 打開功能就顯示        //Steven 20161201 : For SCK 93K ART  //Frank 20161212 (Jou) For Xilinx 打開功能顯示
    tsOCRBarCode        ->TabVisible=(INSTALL_OCR!=eocrUninstal && CosFunction.bTrayOCR);                               //wei 20150720  打開功能就顯示

    // -- T9 (golden :7035-7085) -- WC-24 SAFETY GATE (WinWay ATC hardware) --
    if(ATC_SYSTEM==eWinWay)                                                     //jimmychiu 2021
        ATC_WinWay->TabVisible=true;
    else
        ATC_WinWay->TabVisible=false;

    // AI(W906-FW3-LotInfo-WC) 20260819: GATE WC-24 -- SAFETY RED LINE, WinWay
    // ATC hardware temperature/comm control; not re-evaluated, gated
    // verbatim per the task brief. iTempSetTimer (golden :6937) moved here
    // since this gated block is its only use.
#if 0
    static int iTempSetTimer=0;                                                 //jimmychiu 2021
    if(ATC_SYSTEM==eWinWay && Temperature.bATCActiveCooling==true)
    {
        fWinway->OpenCommPort();

        double dbGetAtcSetTemp=0.0;
        double dbSetATCTemp=0.0;
        bool bHasErr=false;

        if(LastSet.iTemperature==Tempture_Hot || LastSet.iTemperature==Tempture_AmbientHot)
            dbSetATCTemp=Temperature.fWorkTemperBase;
        else
            dbSetATCTemp=IniConfig.dATCAmbientTemperature;
        iTempSetTimer++;
        for(int i=0;i<4;i++)
        {
            dbGetAtcSetTemp=fWinway->arrATC_Site[i]->GetST();
            if(dbSetATCTemp!=dbGetAtcSetTemp || iTempSetTimer>=3)
            {
                fWinway->SetTempratureAll(dbSetATCTemp);
                iTempSetTimer=0;
            }

            if(SystemStart==true && fWinway->arrATC_Site[i]->iWinWaySendCount >=6)
            {
                fWinway->arrATC_Site[i]->WinwayCOM->StopComm();
                fWinway->arrATC_Site[i]->bCommConnect=false;
                fWinway->arrATC_Site[i]->SetPT((double)9999);
                fWinway->arrATC_Site[i]->iWinWaySendCount=0;
                bHasErr=true;
            }

            ATCPtrWinWay[i]->Caption=FormatFloat(L"0.0" ,fWinway->arrATC_Site[i]->GetPT_NoCommand()) ;
        }

        if(bHasErr)
        {
            ShowMyMessage("WinWay ATC System Connect Error!");
        }
    }
#endif

    // -- T10 (golden :7087-7094) --
    if(CUSTOMER_CODE==CC_GIGAS ||                                               //Isaac 20210129 : FTP頁面，關閉功能要不顯示畫面，但視窗不要關掉
       IniConfig.bVTESTFunction==true)
    {
    }
    else
    {
        tsFTP               ->TabVisible=IniConfig.bEnableFTP;                  //Ifor 20180822 : Add 關閉ftp功能關閉顯示頁面
    }

    // -- T11/T12 (golden :7096-7123) -- WC-25 SAFETY GATE (WAR16123 3hr alarm);
    // T12 is this same if-statement's `else` arm, kept REAL --
    if(IniConfig.bVTESTFunction==true)
    {
        // AI(W906-FW3-LotInfo-WC) 20260819: GATE WC-25 -- SAFETY RED LINE,
        // WAR16123 3-hour send-for-inspection alarm; not re-evaluated, gated
        // verbatim per the task brief.
#if 0
        if(IniConfig.bEnableRms)
        {
            fLotInfo->Panel29->Caption="Server";
        }

        if(RunInfo.bLotStart==true &&                                           //RogerYang 20260424 : 陳永恆說不再需要，等趙坤鵬同意  //jou 20230203 : 無錫偉測 機台添加三小時送檢報警，從lot start時間開始計算
           SystemStart==true && iHome==0 &&
           fNote->fShow==false && MyMessageBox->fShow==false )
        {
            iProduceTimeCT++;
            if(iProduceTimeCT>=(3600*3))
            {
                int ret=ShowErrorMessage("WAR16123", K_SKIP , MMSystem);        //RogerYang 20251030 : 袁林要把Retry拿掉
                if(ret==K_RETRY)
                    iProduceTimeCT=9000;
                else
                    iProduceTimeCT=0;
            }
        }

        labConfigL04->Caption="[L04] Temperature range : " + AnsiString(IniConfig.iL04TemptureRange);
#endif
    }
    else
    {
        tsDeviceInfo        ->TabVisible=IniConfig.bEnableRms;                  //Ifor 20181029 : Add 關閉RMS功能關閉顯示頁面
    }

    // -- T13 (golden :7124-7125) --
    tsTesterLog         ->TabVisible=(TestIF_File.iTestType==TCP_IP_MODE);
    grpBarcodeDisplayLotInfo->Visible=(TestIF_File.i2DIDFormat==eAMD);          //JerryYang 20200422 2DID format選項改用下拉選單 //Ifor 20200422 :add 2D主畫面顯示

    // -- T14 (golden :7127-7149) -- WC-8 gates RefreshYieldMonitor() (2nd site) --
    ts_FTPAutomation    ->TabVisible=IniConfig.bA32EnableFTPAutomation;
    if(bOldFTPAutomation!=ts_FTPAutomation->TabVisible)
    {
        bOldFTPAutomation=ts_FTPAutomation->TabVisible;
        if(ts_FTPAutomation->TabVisible)
        {
            pgLotinfo->ActivePage=ts_FTPAutomation;
            fLotInfo->ShowInformation(false);
            fLotInfo->Height=150;
            fLotInfo->Width=150;
            sbTest->Top=50;
        }
        else
        {
            pgLotinfo->ActivePage=tsYieldMonitior;
            // AI(W906-FW-LOTINFO-W30) 20260826: GATE WC-8 RETIRED, 2nd site
            // (golden :7143).  Same audit as the 1st site.
            RefreshYieldMonitor();
        }
    }
    else
    {
        AdjtsYieldMonitiorSize();                                               //Steven 20221225 : 統一Lot Info尺寸調整
    }

    // -- T15 (golden :7152-7164) --
    if(TestIF_File.bEnableBarCode==true &&
       BAR_CODE_INSTALL==ebctUseCCDMode &&
       TestIF_File.bBarCodeMultiRecipe==true)
    {
        labBarcodeRecipe->Visible=true;
        edtBarcodeRecipe->Visible=true;
    }
    else
    {
        labBarcodeRecipe->Visible=false;
        edtBarcodeRecipe->Visible=false;
    }

    // -- T16 (golden :7166-7187) --
    spOCRCleanList->Visible=(INSTALL_OCR!=eocrUninstal && CosFunction.bTrayOCR && IniConfig.bCompareOCRData);           //KenHsieh 20220825 : 新增OCR比對功能
    if(IniConfig.bSPILFunction==true)                                           //JerryYang 20241104 : 支援2DID白名單功能
    {
        if(TestIF_File.b2DIDAllowList)
        {
            if(RunInfo.bLotStart==false)
            {
                if(AccessLevel>=iDefEngineerLevel ||
                   SPIL_FOR_QLE==1)                                             // KevinC 20250912 渠梁OP權限
                {
                    cbRunMode->Enabled=true;
                }
                else
                {
                    cbRunMode->Enabled=false;
                }
            }
        }
        else
        {
           cbRunMode->Enabled=true;
        }
    }

    // -- T17 (golden :7189) --
    ScanRefrigerantSystem();                                                    //Ztex 2023.04.19 Add HT-1032 TriTemp Function

    // -- T18 (golden :7190) --
    fLotInfo->palAQLMode->Visible=IniConfig.bI52_bAQLSortMode;                  //Eastsun 20260520 整合//Ifor 20210713 add: AQL Sor tMode
}

TfLotInfo *fLotInfo = new TfLotInfo();

// =============================================================================
//  AI(W906-FW-LOTINFO-W27) 20260826: uLotInfo Wave D -- 14 methods appended.
//
//  APPEND-ONLY.  Nothing above this line was edited; every pre-existing line
//  in this file and in forms/fLotInfo.h is byte-identical to before this wave.
//
//  Read forms/fLotInfo.h's W27 banner FIRST.  It carries the re-measured
//  denominator (96/210 golden out-of-line TfLotInfo definitions ported after
//  this wave), the batch criterion (and why this batch is 14 and not 30-50),
//  the GATE REGISTER WD-1..WD-5, the six recon classifications this wave
//  overturned by reading the text, the WA/WB/WC premises re-checked on
//  20260826, and the full EXIT REGISTER for everything deliberately absent.
// =============================================================================

// DEVIATION D-2 (see header banner): this wave's includes are placed here, at
// the head of the appended block, rather than in the file's existing include
// section -- the wave is strictly append-only and all four are guarded headers
// declaring only externs/classes, so TU position is semantically irrelevant.
#include <cstring>              // strncpy      -- RFID_ReaderReceiveData, golden :14159
#include <cstdlib>              // atoi         -- btnAMRSetSECSClick,     golden :16229-16230
#include "forms/fSecurity.h"    // fSecurity->GetPasswoard()  (fSecurity.h:583) -- sbTestClick, golden :13731
#include "forms/fPassword.h"    // fPassword->edPassword (:317) / edPasswordPassWord (:326) /
                                //   ShowEventLogLogin (:371) / CheckLoginSuccess (:369) / GetLoginLevel (:370)

// -- RefreshYieldMonitor (golden uLotInfo.cpp:13310-13325) --------------------
// RECON OVERTURN 1 (see header banner): recon #115 classified this "(b) write
// path".  It is a dispatcher and writes nothing; the write recon is pointing
// at is one frame down, in RefreshYieldMonitor_TERAPOWER, and is gated there
// (WD-2).  Both branches and AdjtsYieldMonitiorSize() are real methods of this
// file, so nothing here needs a gate.
//
// INTEGRATOR NOTE: this landing kills the stated reason of GATE WA-3 (the two
// AdjtsYieldMonitiorSize call sites, golden :13671/:13678) and GATE WC-8
// (FormShow golden :597, Timer2Timer golden :7143).  All four are openable and
// were NOT opened here only because they are existing lines.
// AI(W906-FW-LOTINFO-W30) 20260826: ALL FOUR ARE NOW OPEN.  This note is kept
// (not deleted) because it is the record of who made them openable; only its
// last sentence has expired.
void TfLotInfo::RefreshYieldMonitor()                                           //Sam 20210331
{
    static bool bTimerRunning=false;                                            //Sam 20230220
    if(IniConfig.bSIGURDFunction==false &&
       CosFunction.bShowYieldMonitor==false)                                    //Sam 20210916
        return;
    if(bTimerRunning)
        return;
     bTimerRunning=true;
    if(IniConfig.bSIGURDFunction)
        RefreshYieldMonitor_SIGURD();
    else
        RefreshYieldMonitor_TERAPOWER();
     AdjtsYieldMonitiorSize();
     bTimerRunning=false;
}

// -- RefreshYieldMonitor_TERAPOWER (golden uLotInfo.cpp:13547-13623) ----------
// RECON OVERTURN 2: 76 of 77 lines are TestIF_File -> widget display fill.
// Golden :13618 alone is the AutoClean-interval write; it is the ONLY thing
// gated here (WD-2).
void TfLotInfo::RefreshYieldMonitor_TERAPOWER()
{
    bool bLowYield=false, bContsFailBySocket=false, bContsFailByHead=false, bAllSiteFail=false, bSiteToSiteYieldEnable=false, bHeadToHeadYieldEnable=false;
    double dLowYield=0.0, dSiteToSiteYield=0.0, dHeadToHeadYield=0.0;
    int iLowYieldIg=0, iContsFailSocketAlarmCT=0, iContsFailHeadAlarmCT=0, iAllSiteFailCount=0, iSiteToSiteYieldCount=0, iHeadToHeadYieldCount=0;

    if(iRunStartMode==FT)
    {
        //Low Yield
        bLowYield               =TestIF_File.bFailAlarmLowYield;
        dLowYield               =TestIF_File.dLowYieldLimit;
        iLowYieldIg             =TestIF_File.iLowYieldCount;
        //Consecutive Failure Alarm ( Socket/Head )
        bContsFailBySocket      =TestIF_File.bContsFailBySocket;
        bContsFailByHead        =TestIF_File.bContsFailByHead;
        iContsFailSocketAlarmCT =TestIF_File.iContsFailSocketAlarmCT;
        iContsFailHeadAlarmCT   =TestIF_File.iContsFailHeadAlarmCT;
        //All Site Fial
        bAllSiteFail            =TestIF_File.bAllSiteFail;
        iAllSiteFailCount       =TestIF_File.iAllSiteFailCount;
    }
    else
    {
        //Low Yield
        bLowYield               =TestIF_File.bFailAlarmLowYield_RT;
        dLowYield               =TestIF_File.dLowYieldLimit_RT;
        iLowYieldIg             =TestIF_File.iLowYieldCount_RT;
        //Consecutive Failure Alarm ( Socket/Head )
        bContsFailBySocket      =TestIF_File.bContsFailBySocket_RT;
        bContsFailByHead        =TestIF_File.bContsFailByHead_RT;
        iContsFailSocketAlarmCT =TestIF_File.iContsFailSocketAlarmCT_RT;
        iContsFailHeadAlarmCT   =TestIF_File.iContsFailHeadAlarmCT_RT;
        //All Site Fial
        bAllSiteFail            =TestIF_File.bAllSiteFail_RT;
        iAllSiteFailCount       =TestIF_File.iAllSiteFailCountRT;
    }
    //Site To Site Yield %
    bSiteToSiteYieldEnable      =TestIF_File.bSiteToSiteYieldCmp;
    dSiteToSiteYield            =TestIF_File.iSiteToSiteYieldCmp;
    iSiteToSiteYieldCount       =TestIF_File.iSiteToSiteYieldCmpCount;
    //Head To Head Yield %
    bHeadToHeadYieldEnable      =TestIF_File.bHeadToHeadYieldCmp;
    dHeadToHeadYield            =TestIF_File.iHeadToHeadYieldCmp;
    iHeadToHeadYieldCount       =TestIF_File.iHeadToHeadYieldCmpCount;

    cbLowYield->Checked                 =bLowYield;
    edLowYield->Text                    =FloatToStr(dLowYield);
    edLowYieldIg->Text                  =IntToStr(iLowYieldIg);
    rbContsFailBySocket_On->Checked     =bContsFailBySocket;
    rbContsFailBySocket_Off->Checked    =!bContsFailBySocket;
    rbContsFailByHead_On->Checked       =bContsFailByHead;
    rbContsFailByHead_Off->Checked      =!bContsFailByHead;
    edContsFailSocketAlarmCT->Text      =IntToStr(iContsFailSocketAlarmCT);
    edContsFailHeadAlarmCT->Text        =IntToStr(iContsFailHeadAlarmCT);
    cbAllSiteFail->Checked              =bAllSiteFail;
    edAllSiteFailCount->Text            =IntToStr(iAllSiteFailCount);
    cb_SiteToSiteYieldEnable->Checked   =bSiteToSiteYieldEnable;
    ed_SiteToSiteYield->Text            =FloatToStr(dSiteToSiteYield);
    ed_SiteToSiteYieldCount->Text       =IntToStr(iSiteToSiteYieldCount);
    cb_HeadToHeadYieldEnable->Checked   =bHeadToHeadYieldEnable;
    ed_HeadToHeadYield->Text            =FloatToStr(dHeadToHeadYield);
    ed_HeadToHeadYieldCount ->Text      =IntToStr(iHeadToHeadYieldCount);

    chk_SmartAutoClean->Checked         =TestIF_File.bACSmart;                  //Sam 20230111 : Smart Auto Clean
    edt_SmartAutoClean->Text            =TestIF_File.iACSmart_Count;
    ed_SmartAutoCleanCTF->Text          =TestIF_File.iACSmart_Count_CTF;        //Sam 20240726 : AI Clean
    lbl_SmartAutoCleanCount->Caption    =IntToStr(iACSmartCount);
    lbl_SmartAutoCleanCount_CTF->Caption=IntToStr(iACSmartCount_CTF);           //Sam 20240726 : AI Clean

    if(iAdaptiveACInterval<=0)
    {
        // AI(W906-FW-LOTINFO-W27) 20260826: GATE WD-2 -- see forms/fLotInfo.h.
        // ESTABLISHED GATE reused verbatim from atester_ProcessCount.cpp:1534
        // GATE D (same target, same reason, 5 other golden call sites).  The
        // guard is kept real and the body empty so the shape stays diffable
        // against golden.
#if 0
        fCleaning->ChangeACSmartInterval(2, "RefreshYieldMonitor");             //Sam 20240726 : AI Clean
#endif
    }
    AnsiString s="";
    s.sprintf("Adaptive Auto Clean Interval : %d/%d  Count", iAutoClean_IndexContactCount, iAdaptiveACInterval);
    lblAdaptiveIntervalCount->Caption=s;
}

// -- LotKeyInTimeTimer (golden uLotInfo.cpp:11543-11660) -- WD-3 -------------
// RECON OVERTURN 3: 118 lines of Lot-ID / Operator-ID text validation; no
// file, no ini, no socket, no motion.  Sender dropped (unused in golden).
void TfLotInfo::LotKeyInTimeTimer()
{
    static bool bRunLotKeyInTime=false;
    static int iCount=0;
    AnsiString sLastKeyin = "";
    if(InitialOK==false || bRunLotKeyInTime==true)
        return;
    if(bLotFirstKeyIn==true)                                                    //Ifor 20190924
    {
        bLotFirstKeyIn=false;
        iCount=0;
        return;
    }

    bRunLotKeyInTime=true;

    if(CUSTOMER_CODE==CC_KYEC_LEE)
    {
        if(edtSysLotID->Text.Length()!=10)                                      //Ifor 20190919
        {
            SetLotID("");
        }
        else
        {
            if(sLastKeyin=="")
            {
                sLastKeyin=edtSysLotID->Text;

                if(sLastKeyin.Length()!=10)
                {
                    SetLotID("");
                    sLastKeyin="";
                }
            }
            else
            {
                SetLotID(sLastKeyin);
            }
        }

        sLastKeyin="";
        // AI(W906-FW-LOTINFO-W27) 20260826: GATE WD-3 -- golden :11584-11637,
        // the whole Operator-ID if/else-if/else chain.  CAUSE: golden
        // :11586-11587 reads `fMain->cbRunStartMode->Text`, and cbRunStartMode
        // has NO PORT on TfMain (verified 20260826).  Gated as ONE chain, not
        // as one clause, because dropping a disjunct from a compound condition
        // rewrites the truth table -- the same reasoning this file's WC-10
        // uses.  BEHAVIOUR DELTA: the operator-ID text is never modified here
        // (golden could clear it for out-of-range KYEC/KLT head numbers).  A
        // narrowing, not a divergence.  Note golden's own first arm body is a
        // round trip through a local, i.e. already a no-op on the widget.
#if 0
        if(TrayForm.bEnableAMR ||                                               //Eastsun 20260515 F010 AMR/AGV Operator ID
           (TrayForm.bEnableAMR==false && TrayForm.bEnableAMRLoader==true &&
           (fMain->cbRunStartMode->Text=="Re-Test Continuous" ||
           fMain->cbRunStartMode->Text=="Re-Test Initial Start")) && edtSysOperatorID->Text=="AGV")
        {
            sLastKeyin=edtSysOperatorID->Text;
            edtSysOperatorID->Text=sLastKeyin;
        }
        else if(edtSysOperatorID->Text.Length()<6 ||                            //Ifor 20190919
           edtSysOperatorID->Text.Length()>7)                                   // 2013.12.17 , Joye , KYEC Barcdoe Reader
        {
            edtSysOperatorID->Text="";
            sLastKeyin="";
        }
        else
        {
            if(sLastKeyin=="")
            {
                sLastKeyin=edtSysOperatorID->Text;
                AnsiString sHeadNum;
                int iHeadNum=0;
                if(sLastKeyin.Length()==6)
                {
                    sHeadNum=sLastKeyin.SubString(1, 2);
                    iHeadNum=atoi(sHeadNum.c_str());
                }
                else if(sLastKeyin.Length()==7)
                {
                    sHeadNum=sLastKeyin.SubString(1,3);
                    iHeadNum=atoi(sHeadNum.c_str());
                }
                                                                                //Ifor 20180517
                if(bEnable_KLT_Function==true)                                  //Ifor 20180802
                {
                    if(iHeadNum<3 || iHeadNum>31)                               // KYEC 3~31 (2003~2031)
                    {
                        edtSysOperatorID->Text="";
                        sLastKeyin="";
                    }
                }
                else
                {
                    if(iHeadNum<85 || iHeadNum>120)                             // KYEC 85~120 (85~120)
                    {
                        edtSysOperatorID->Text="";
                        sLastKeyin="";
                    }
                }
            }
            else
            {
                edtSysOperatorID->Text=sLastKeyin;
            }
        }
#endif
    }
    else if(CUSTOMER_CODE==CC_AMD_M && CosFunction.bHiSiliconFunction==true)
    {
        iCount++;
        if(iCount>=10)
        {
            iCount=0;
            if(bLotID_OK==false)
                edtSysLotID->Text="";

            if(bOPID_OK==false)
                edtSysOperatorID->Text="";

            if(bDeviceName_OK==false)
                edDeviceName->Text="";

            if(bTemp_OK==false)
                edTemp->Text="";
        }
    }

    bRunLotKeyInTime=false;
}

// -- SetLotComponents (golden uLotInfo.cpp:2283-2320) -- WD-1 **RETIRED** ----
// RECON OVERTURN 6: 37 of 38 lines are widget ->Enabled/->Down/->Caption.
// Golden :2285 alone is the SECS-visible global.
//AI(W906-LOT-W1) 20260919: it WAS the only line gated; the gate is retired.
// Full reasoning at the un-gate site below.  This heading used to say "and is
// the only line gated" -- corrected here so the banner and the body agree.
void TfLotInfo::SetLotComponents(bool bLotEnd)                                  //Steven 20250515
{
    //AI(W906-LOT-W1) 20260919: GATE WD-1 **已退役**。
    //
    // 原本的理由（20260826）：「Gated so a display helper cannot silently own
    // lot-start state」。那是**設計顧慮**，不是「相依不存在」——
    // `RunInfo` 一直都在，這一行從來沒有缺過任何東西。
    // 依 `docs/PLAN_START_TO_RUN.md` §0.5（使用者 20260919 裁決）：
    // 加閘的唯一合法理由是相依不存在；「我覺得這個設計不好」不是。
    // 而且這一行是 golden 自己寫的（`uLotInfo.cpp:2285`），閘掉它等於
    // 替 golden 做了一個它沒做的決定。
    //
    // ⓘ 解閘當下**零行為改變**，原註解自己就寫了：「Unobservable today --
    //   SetLotComponents' only golden callers are SetLotStart/SetLotEnd,
    //   neither of which is ported」。
    //   ⚠ 20260919 重跑那條 grep，發現原敘述**不完全對**：
    //     `Automation/auto9045.cpp:1488` 有呼叫，但它打到**同檔的 TU-local
    //     no-op 樁**（`auto9045.cpp:289` 的 `W5FA_TfLotInfoExt::
    //     SetLotComponents(bool) {}`），不是這一支。
    //     ⇒ 「零行為改變」的結論仍成立，但理由是「呼叫者打到別的東西」，
    //       不是「沒有呼叫者」。下一波換掉那個 TU-local 樁時這個差別會變重要。
    //   它會在 `SetLotStart` / `SetLotEnd` 落地那一刻才活起來 —— 而那正是
    //   P0-2 要做的事，也正是 START 現在被擋的地方：
    //   `WebStart.cpp:2086` 要 `RunInfo.bLotStart==true` 才放行。
    //
    // ⚠ 原註解點出的那件事仍然為真、而且重要：SECS 的 DoLotStart 去重路徑
    //   會讀這個旗標。所以 `SetLotComponents` 被呼叫的**時機**要照 golden，
    //   不要為了方便在別的地方多呼叫一次。
    RunInfo.bLotStart            =!bLotEnd;                                     //RogerYang 20250312
    edtSysLotID         ->Enabled=bLotEnd;
    sbSECSLotStart      ->Down   =!bLotEnd;
    sbSECSLotEnd        ->Down   =bLotEnd;
    pnlLoader           ->Caption="";
    edPage              ->Enabled=bLotEnd;                                      //JerryYang 20190928 SPIL lot count
    edtSysOperatorID    ->Enabled=bLotEnd;                                      //wei 20150326
    edCustomerLotId     ->Enabled=bLotEnd;                                      //Sam 20220223
    coStation           ->Enabled=bLotEnd;                                      //Sam 20220223
    edStationNum        ->Enabled=bLotEnd;                                      //Sam 20220223
    edtJobSeq           ->Enabled=bLotEnd;                                      //JerryYang 20220923 : add
    edtBarcodeRecipe     ->Enabled=bLotEnd;                                     //Ifor 20241108 / Eastsun 20260527

    if(CUSTOMER_CODE==CC_PTI && IniConfig.bB03_TesterReport==false)             //Sam 20240809 : PTI ART
        cbRunMode       ->Enabled=false;                                        //wei 20150326
    else
        cbRunMode       ->Enabled=bLotEnd;

    edtCusLotID         ->Enabled=bLotEnd;                                      //JerryYang 20230322
    edtCusDevGrp        ->Enabled=bLotEnd;
    edtDevice           ->Enabled=bLotEnd;

    lbledtStarTime      ->Enabled=bLotEnd;
    lbledtEndTime       ->Enabled=bLotEnd;
    lbledtTesterOsVer   ->Enabled=bLotEnd;
    lbledtTesterID      ->Enabled=bLotEnd;
    lbledtCustomer      ->Enabled=bLotEnd;
    lbledtTestProg      ->Enabled=bLotEnd;
    lbledtDeviceName    ->Enabled=bLotEnd;
    lbledtSubLotNo      ->Enabled=bLotEnd;
    lbledtModeCode      ->Enabled=bLotEnd;
    lbledtTestCode      ->Enabled=bLotEnd;
    lbledtTestBinNo     ->Enabled=bLotEnd;
    edtStage            ->Enabled=bLotEnd;
    edtStep             ->Enabled=bLotEnd;
}

// -- palSecsGemMouseDown (golden uLotInfo.cpp:10351-10425) -- WD-4 withdrawn -
// A 6-click left/left/right/right/left/left unlock sequence that seeds default
// Lot/Operator IDs.  Delivered with golden's FULL signature: TMouseButton /
// TShiftState landed 20260826 in vclcompat/ShiftState.h, killing the premise
// GATE (WB-2-BTN) above still cites.  SetLotID() is this file's own real
// (no-op) method -- the same call WB-5 already accepts.
void TfLotInfo::palSecsGemMouseDown(TObject *Sender, TMouseButton Button,
      TShiftState Shift, int X, int Y)
{
    static int iStep=0;

    if(SystemStart ||
       AccessLevel<iDefHonPrecLevel ||                                          //V3.27K.538 Ifor 20170905 (wei)
       CUSTOMER_CODE==CC_KYEC_LEE)                                              //Ifor 20191017
        return;

    switch(iStep)
    {
        case 0:
            if(Button==mbLeft)
            {
                iStep=1;
            }
            else
            {
                iStep=0;
            }
            break;
        case 1:
            if(Button==mbLeft)
            {
                iStep=2;
            }
            else
            {
                iStep=0;
            }
            break;
        case 2:
            if(Button==mbRight)
            {
                iStep=3;
            }
            else
            {
                iStep=0;
            }
            break;
        case 3:
            if(Button==mbRight)
            {
                iStep=4;
            }
            else
            {
                iStep=0;
            }
            break;
        case 4:
            if(Button==mbLeft)
            {
                iStep=5;
            }
            else
            {
                iStep=0;
            }
            break;
        case 5:
            if(Button==mbLeft)
            {
                if(edtSysLotID->Text=="")
                    SetLotID("0123456789");

                if(edtSysOperatorID->Text=="")
                    edtSysOperatorID->Text="12345";
            }
            iStep=0;
            break;
    }
}

// -- ShowXMLOnLine (golden uLotInfo.cpp:12099-12137) -- WD-5 ----------------
// RECON OVERTURN 5: recon #103 and GATE WC-3 both call this "writes files x4".
// Those 4 writes are RecordProcess(), which in THIS tree is
// canary_support.cpp:109 -- a stdout printf stand-in (the DB body is #if 0 at
// cMyDB.cpp:1832 pending the GA-1-B4 swap) and is called freely from 582
// already-translated sites.  Translated LIVE.
// WD-5 LATENCY WARNING: after the GA-1-B4 integrator swap these 4 lines become
// real DB writes.  Gate them THEN if that matters -- pre-gating now would make
// this function diverge from the 582 sites doing the same thing.
// INTEGRATOR NOTE: this landing kills GATE WC-3's stated reason; the FormShow
// call site (golden :399) is openable.  Not opened here (existing line).
// AI(W906-FW-LOTINFO-W30) 20260826: that call site IS NOW OPEN (this file:3151),
// so the WD-5 warning above is live from FormShow, not just from a future
// caller.  Note kept as the record of who made it openable.
void TfLotInfo::ShowXMLOnLine()                                                 //Steven 20200629
{
    if(CUSTOMER_CODE==CC_Murata)
    {
        if(iXMLOnLineStatus==0)
        {
            if(IniConfig.bN10_9_UploadUnloadTrayToFTP ||
               IniConfig.bN23_1_Enable2DIDCompare ||
               IniConfig.bN23_3_UploadTestResult)
            {
                pnlXMLOnLine->Caption="Server On Line";
                pnlXMLOnLine->Color=clLime;
                RecordProcess("Server function on Line");
            }
            else
            {
                pnlXMLOnLine->Caption="Server Off Line";
                pnlXMLOnLine->Color=clRed;
                RecordProcess("Server function off Line");
            }
        }
        else if(iXMLOnLineStatus==1)
        {
            pnlXMLOnLine->Caption="Server On Line";
            pnlXMLOnLine->Color=clLime;
            RecordProcess("Server function on Line");
        }
        else
        {
            pnlXMLOnLine->Caption="Server Off Line";
            pnlXMLOnLine->Color=clRed;
            RecordProcess("Server function off Line");
        }
    }
    else
    {
        pnlXMLOnLine->Visible=false;
    }
}

// -- sbTestClick (golden uLotInfo.cpp:13719-13752) --------------------------
// Password prompt -> ShowInformation(true/false).  ShowInformation is already
// a real method of this file (Wave A).  SOFT_SIMULTE is #undef'd in
// MachineType.h, so the #else arm is what compiles -- both arms are kept
// verbatim so the source stays diffable against golden.
void TfLotInfo::sbTestClick(TObject *Sender)
{
    static bool bFirstIN=true;

    AnsiString asPassword;

    if(gbFTPAutomation_Download->Visible==true)
    {
        ShowInformation(false);
        return;
    }

    asPassword=fSecurity->GetPasswoard();                                       //Sam 20220106
    fPassword->edPassword->Text="";
    if(bFirstIN)
    {
        bFirstIN=false;
        #ifdef SOFT_SIMULTE
            ShowInformation(true);
        #else
        fQwertyKey->ShowQwertyKey(fPassword->edPassword, N_NO_SYMBOL|N_NO_SPACE|N_PASSWORD);
        if(asPassword==fPassword->edPassword->Text)
        {
            ShowInformation(true);
        }
        else
        {
            ShowInformation(false);
        }
        #endif
    }

    bFirstIN=true;
}

// -- cbFirstTrayCheckOnUnloaderClick (golden uLotInfo.cpp:15994-16027) ------
// Checkbox + in-memory IniConfig.bP62Auto1..3 mirror of Prod.iIsFailT6.  No
// persistence: the ini flush for these lives in btnSaveClick / cConfiguration,
// both outside this function (btnSaveClick is EXITED, see header EXIT A).
// Event handler -- translated, NOT wired.
void TfLotInfo::cbFirstTrayCheckOnUnloaderClick(TObject *Sender)
{
    if(bP60UserClicked==true)
    {
        bP60UserClicked=false;
        if(cbFirstTrayCheckOnUnloader->Checked==false)
        {
            if(AccessLevel<iDefEngineerLevel)
            {
                cbFirstTrayCheckOnUnloader->Checked=true;
            }
        }
        else
        {
            if(AccessLevel<iDefEngineerLevel)
            {
                cbFirstTrayCheckOnUnloader->Checked=false;
            }
            else if(IniConfig.bP62FirstTrayCheckOnUnloader==true)
            {
                IniConfig.bP62Auto1=(Prod.iIsFailT6[eAuto1]==0);
                IniConfig.bP62Auto2=(Prod.iIsFailT6[eAuto2]==0);
                IniConfig.bP62Auto3=(Prod.iIsFailT6[eAuto3]==0);
                cb1stCheck_Auto1->Checked=IniConfig.bP62Auto1;
                cb1stCheck_Auto2->Checked=IniConfig.bP62Auto2;
                cb1stCheck_Auto3->Checked=IniConfig.bP62Auto3;
            }
            else
            {
                cbFirstTrayCheckOnUnloader->Checked=false;
            }
        }
    }
}

// -- SetFirstTrayCheckOnUnloader (golden uLotInfo.cpp:16036-16045) ----------
void TfLotInfo::SetFirstTrayCheckOnUnloader()                                   //Jimmychiu 20251205
{
    cbFirstTrayCheckOnUnloader->Checked=true;
    IniConfig.bP62Auto1=(Prod.iIsFailT6[eAuto1]==0);
    IniConfig.bP62Auto2=(Prod.iIsFailT6[eAuto2]==0);
    IniConfig.bP62Auto3=(Prod.iIsFailT6[eAuto3]==0);
    cb1stCheck_Auto1->Checked=IniConfig.bP62Auto1;
    cb1stCheck_Auto2->Checked=IniConfig.bP62Auto2;
    cb1stCheck_Auto3->Checked=IniConfig.bP62Auto3;
}

// -- btnAirStreamOnOffClick (golden uLotInfo.cpp:14461-14488) ---------------
// GOLDEN IS EMPTY.  Its entire 24-line body sits inside one /* ... */ block
// (golden :14463-14487) -- so the faithful translation is an empty function,
// not a gate and not a stub.  Golden's dead text is reproduced verbatim below
// so a future reader can see WHAT was disabled (a manual Air Machine on/off
// that would call ATC_InterfaceForm->SendAirMachineStatus) without reopening
// the cp950 source.  If anyone ever revives it, note it is EXIT class C:
// SendAirMachineStatus is a real outbound hardware command, and the 1-member
// ATC_InterfaceForm shim (acarry_shims.h:109-115) has none of these members.
void TfLotInfo::btnAirStreamOnOffClick(TObject *Sender)
{
   /*
    double dSetAirMachineTemp = Temperature.fSetTempature2AirMachine;
    double dSetAirMachineTemp2= Temperature.dSetIndexAirstreamTemp;
    if(AccessLevel<1)
    {
        ShowMyMessage("Change Air Cooling Machinr Status Fail # Please Log In for executing Engineer");
        return;
    }

    if(!SystemStart)
    {
        if(btnAirStreamOnOff->Caption=="Air Stream Stop")
        {
            ATC_InterfaceForm->bCloseAirMachine=true;
            btnAirStreamOnOff->Caption="Air Machine Run";                       //add Manual Air Cooling On/Off
            ATC_InterfaceForm->SendAirMachineStatus(0, dSetAirMachineTemp*10, dSetAirMachineTemp2*10);
        }
        else
        {
            ATC_InterfaceForm->bCloseAirMachine=false;
            btnAirStreamOnOff->Caption="Air Stream Stop";                       //add Manual Air Cooling On/Off
            ATC_InterfaceForm->SendAirMachineStatus(1, dSetAirMachineTemp*10, dSetAirMachineTemp2*10);
        }
    }
    */
}

// -- UpdatePATSubMode (golden uLotInfo.cpp:15716-15743) ---------------------
// RECON OVERTURN 4: pure TComboBox repopulation.  The one PAT-family method
// that never touches fMain->patFunc, which is why it survives when its eight
// siblings are EXITED (header EXIT D).
void TfLotInfo::UpdatePATSubMode(const AnsiString& sModeName)
{
    cbPATSubMode->Clear();
    if(sModeName=="Re-Test")
    {
        cbPATSubMode->Visible=true;
        cbPATSubMode->Items->Add("Re-Test");
        for(int i=1; i<=10; i++)
        {
            cbPATSubMode->Items->Add(AnsiString().sprintf("RT%d",i));
        }
        cbPATSubMode->ItemIndex=0;
    }
    else if(sModeName=="EQC")
    {
        cbPATSubMode->Visible=true;
        cbPATSubMode->Items->Add("EQC");
        for(int i=1; i<=6; i++)
        {
            cbPATSubMode->Items->Add(AnsiString().sprintf("EQCRT%d",i));
        }
        cbPATSubMode->ItemIndex=0;
    }
    else
    {
        cbPATSubMode->Visible=false;
    }
}

// -- RFID_ReaderReceiveData (golden uLotInfo.cpp:14152-14173) -- D-1 --------
// INBOUND direction only: parses a buffer the RFID reader already delivered
// and displays it.  The OUTBOUND half (DoReadRFID :14084 WriteCommData,
// InitRFIDRS232 :14142 StartComm) is EXITED -- see header EXIT B.
// DEVIATION D-1: golden's `Pointer Buffer, WORD BufferLength` becomes
// `void *Buffer, unsigned short BufferLength` -- the exact underlying types.
// GOLDEN ODDITY, translated verbatim: strncpy's length comes straight from the
// wire (BufferLength) with no clamp against sizeof(cStr1)==1024.
void TfLotInfo::RFID_ReaderReceiveData(TObject *Sender,
      void *Buffer, unsigned short BufferLength)
{
    char *data;                                                                 //Steven 20220713 : RFID Reader for SJSEMI
    data=(char*)Buffer;
    AnsiString cStr="", Log="", Log1="";
    char cStr1[1024]={0};
    strncpy(cStr1, data, BufferLength);
    cStr=cStr1;
    cStr=cStr.Delete(1, 13);
    cStr=cStr.SubString(1, cStr.Length()-1);
    if(cStr=="")
    {
        cStr="Data error-Read Fail";
    }

    fLotInfo->pnlLoader->Caption=cStr;                                          // DEVIATION D-3: golden's own spelling kept
    Log1.sprintf("Loader : %s", cStr.c_str());

    Log.sprintf("%04d-%02d-%02d, %02d:%02d:%02d.%03d, %s", SystemYear, SystemMonth, SystemDate, SystemHour, SystemMin, SystemSec, SystemMSec, Log1);
    mmRFID->Lines->Add(Log);
}

// -- sb_Main_EvenLevelLoginClick (golden uLotInfo.cpp:10475-10489) ----------
// Access-level display only: opens the EventLog login and mirrors the level
// into a combo box.  fPassword's four members are all real (forms/fPassword.h
// :317/:369/:370/:371).  Note ShowEventLogLogin() is headless-safe by that
// header's own NULL-GLOBAL note -- there is no modal loop, so
// CheckLoginSuccess() answers from whatever state the facade already holds.
void TfLotInfo::sb_Main_EvenLevelLoginClick(TObject *Sender)
{
    fPassword->edPasswordPassWord->Text="";

    fPassword->ShowEventLogLogin();
    if(fPassword->CheckLoginSuccess()==true)
    {
        cbbASECL_LoginMode->ItemIndex=fPassword->GetLoginLevel();
    }
    else
    {
        cbbASECL_LoginMode->ItemIndex=0;
    }
    sb_Main_EvenLevelLogin->Down=false;
}

// -- btnAMRSetSECSClick (golden uLotInfo.cpp:16227-16232) -------------------
// Three edits -> TestIF_File (cprod.h:2516/2517/2522).  In-memory only; the
// SECS send that would consume them is btnAMRSupplementClick, EXITED under
// header EXIT B.  Event handler -- translated, NOT wired.
void TfLotInfo::btnAMRSetSECSClick(TObject *Sender)
{
    TestIF_File.iAMRTrayCount[3]=atoi(edAMRTrayCount->Text.c_str());
    TestIF_File.iAMRDeviceCount[3]=atoi(edAMRDeviceCount->Text.c_str());
    TestIF_File.asAMRBinSetting[0]=edAMRBinSetting->Text;
}


// =============================================================================
//  AI(W906-LOT-W1) 20260919: P0-2 工單狀態機 —— 第一塊。
//
//  APPEND-ONLY，與上面 W27 那一波同一個約定：這一行以上一個字都沒動。
//
//  為什麼做這一塊：20260919 實測，網頁按 START 的下一個否決點是
//  `WebStart.cpp:2086`「Please Enter LotID and Operator ID!!」（golden :5216）。
//  這台機器 CUSTOMER_CODE=868=CC_CYUEAN，命中 golden :5204 那一支，它要
//  LotID / OperatorID / `RunInfo.bLotStart` 三個都成立。
//  填它們的整條鏈（ReadWriteLotInfo -> SetLotID -> SetLotStart -> SetLotComponents）
//  在這棵樹**是兩個空樁加兩個不存在**。計畫：docs/PLAN_START_TO_RUN.md P0-2。
//
//  相依實測（20260919，全部存在，**零個閘**）：
//    ReadIniData / WriteIniData (AnsiString 多載)  common.h:238 / :251
//    AuthPath                                      common.h（common.cpp:102）
//    IniConfig.SocketHandlerID                     Config.h:51
//    ArmDataLot[3] + TArm::ReadFile/WriteFile      cSocket.h:271 / :194-195
//    fBarCode->JCETUseMakeWhite2DIDList()          活的（csystem.cpp:10409）
//    22 個 widget                                   forms/fLotInfo.h:2181-2200
//      （其中 lbledtMachineID / edtLotEventLogName 是本波照 golden :615 / :989
//        補回來的 —— 之前的宣告序列跳過了那兩列）
//
//  ⚠ 這支**會寫真實檔**：`AuthPath+"config.ini"`（= D:\HT9045\config\config.ini）。
//    驗證一律照 docs/PLAN_START_TO_RUN.md §0.6：
//      python tools/realfile_guard.py snap <tag> -> 跑 -> check -> drop
//    該檔已在 realfile_guard 的 TARGETS 裡。
// =============================================================================
#include "cSocket.h"            // AI(W906-LOT-W1) 20260919: ArmDataLot[3] (:271) + TArm::ReadFile/WriteFile (:194-195)
//AI(W906-LOT-W1) 20260919: SetLotStart（golden :1572-1970）要的 11 個。
// 每一個都是**已經存在**的符號，只是這個 TU 沒引到 —— 不是相依缺口。
#include "Public/MyStringList.h"     // TMyStringList 的完整型別（slEventLog->SetLotData）
#include "Automation/SCK_ART.h"      // fSCKART
#include "forms/fProductionInfo.h"   // fProductionInfo->OEE_SetMO / OEE_SetHandlerID
#include "forms/fObserver.h"         // fObserver->sTestReceiveTime / memoLotSummary
#include "forms/fMesSystem.h"        // fMesSystem->CheckVTENGmode / bDownloadLotInforFlag
#include "forms/fSortCT.h"           // fSortCT->btnClearCountClick
#include "forms/fCounterClear.h"     // fCounterClear->LowYieldSpecialInitail
#include "Interface/TesterTCP_Socket.h"  // fTesterTCP->SendTCPIPCommand
#include "csystem.h"                 // HasICUnderMachine / HasAnyICInMachine
#include "acatchtray.h"              // ClearAllTrayCount
#include "SECSGEM/SecsEventType.h"   // SECS_EVENT
#include "SECSGEM/SecsEventReport.h" // EventReport
#include "Automation/SCK_ART_SaveTestSummary.h"   // AI(W906-PROD-S117／G030) 20260926：W906_SckArt_SaveTestSummary（SetLotEnd 的 S117-SUMMARY 段；轉接本體 Automation/SCK_ART_SaveTestSummary.cpp）；佔用原本的空行，不移動本檔其後的行號
//----------------------------------------------------------------------------
//  ReadWriteLotInfo  --  golden uLotInfo.cpp:1404-1490，逐字翻譯，0 gate。
//
//  ⓘ golden 自己的兩個怪處，照翻不要「修」：
//    1. `Customer Lot ID` 這個鍵被**寫兩次**：:1462 寫 `edCustomerLotId`、
//       :1483 寫 `edtCusLotID`。後寫的贏。讀的那一側也對稱地讀兩次
//       （:1416 -> edCustomerLotId、:1442 -> edtCusLotID），所以
//       `edCustomerLotId` 讀到的值會被同一個鍵覆蓋掉語意。
//       那是 golden 20251208 加 JCET 功能時留下的，不是移植錯誤。
//    2. `lbledtMachineID` 在**讀**的那一側不是從 ini 來，是直接吃
//       `IniConfig.SocketHandlerID`（:1444）；寫的那一側則完全沒寫它。
//       所以它是單向的：ini 永遠沒有這個鍵。
//----------------------------------------------------------------------------
void TfLotInfo::ReadWriteLotInfo(bool bRead)                                    //Steven 20250515 : 整合Lot測試報表
{
    AnsiString sPath=AuthPath+"config.ini";

    if(bRead)
    {
        lbledtStarTime      ->Text=ReadIniData(sPath, "Lot Info", "Start Time",     AnsiString(""));
        lbledtEndTime       ->Text=ReadIniData(sPath, "Lot Info", "End Time",       AnsiString(""));
        lbledtTesterOsVer   ->Text=ReadIniData(sPath, "Lot Info", "Tester OS Ver",  AnsiString(""));
        lbledtTesterID      ->Text=ReadIniData(sPath, "Lot Info", "Tester ID",      AnsiString(""));
        edtSysOperatorID    ->Text=ReadIniData(sPath, "Lot Info", "Operator",       AnsiString(""));
        lbledtCustomer      ->Text=ReadIniData(sPath, "Lot Info", "Customer",       AnsiString(""));
        edCustomerLotId     ->Text=ReadIniData(sPath, "Lot Info", "Customer Lot ID",AnsiString(""));
        lbledtTestProg      ->Text=ReadIniData(sPath, "Lot Info", "Test Program",   AnsiString(""));
        lbledtDeviceName    ->Text=ReadIniData(sPath, "Lot Info", "Device Name",    AnsiString(""));
        edtStage            ->Text=ReadIniData(sPath, "Lot Info", "Stage",          AnsiString(""));
        edtStep             ->Text=ReadIniData(sPath, "Lot Info", "Step",           AnsiString(""));
        edtSysLotID         ->Text=ReadIniData(sPath, "Lot Info", "Lot No",         AnsiString(""));
        lbledtSubLotNo      ->Text=ReadIniData(sPath, "Lot Info", "Sub Lot No",     AnsiString(""));
        lbledtModeCode      ->Text=ReadIniData(sPath, "Lot Info", "Mode Code",      AnsiString(""));
        lbledtTestCode      ->Text=ReadIniData(sPath, "Lot Info", "Test Code",      AnsiString(""));
        lbledtTestBinNo     ->Text=ReadIniData(sPath, "Lot Info", "Test Bin No",    AnsiString(""));
        //AI(W906-LOT-W1) 20260919: GATE (W906-LOT-W1-JCET2DID) —— 缺相依。
        // `fBarCode` 是 `TfBarCode*`（aHotPlateSubstrate.h:1037），
        // 那個類別**沒有** `JCETUseMakeWhite2DIDList`。
        // 這是樹裡的**既有慣例**，不是本波新發明的閘：
        //   Public/MyProductionRecord.cpp:1198  GATE G-1
        //   forms/fLotInfo.h:170                WA-7（同一個成員、同一個理由）
        // 兩者都是「閘掉判斷、走 else 臂」。
        //
        // ⓘ 我先前一度以為 `csystem.cpp:10409` 是活的呼叫點（grep 只濾了行首
        //   `//`，沒看 `#if 0`）。**編譯器已經證明它不是**：那個成員若真的被
        //   活的碼碰到，csystem.cpp 今天就編不過。
        //   memory: verify-with-compiler-not-scanner。
        //
        // 行為：走 else 臂 —— 讀 `"Run Mode"` 而不是 `"Run Mode 2DID"`。
        // 那是**非 JCET 機台的正常路徑**，而這台是 CC_CYUEAN（868），本來就走這邊。
        // ⚠ DELTA on a JCET machine: 2D 白名單模式的 Run Mode 會讀錯鍵。
        // UN-GATE：等 `TfBarCode` 補上那個成員（與 G-1 / WA-7 同一天解）。
#if 0 // GATE (W906-LOT-W1-JCET2DID): 缺相依 TfBarCode::JCETUseMakeWhite2DIDList（既有慣例 G-1 / WA-7）
        if(fBarCode->JCETUseMakeWhite2DIDList()==true)                          //RogerYang 20251208 : JCET 2D FT1白名單/FT2比對功能
        {
            cbRunMode->Text=ReadIniData(sPath, "Lot Info", "Run Mode 2DID",       AnsiString(""));
            for(int i=0; i<cbRunMode->Items->Count; i++)
            {
                if(cbRunMode->Items->Strings[i]==cbRunMode->Text)
                {
                    cbRunMode->ItemIndex=i;
                    break;
                }
            }
        }
        else
#endif // GATE (W906-LOT-W1-JCET2DID)
        {
            cbRunMode       ->Text=ReadIniData(sPath, "Lot Info", "Run Mode",       AnsiString(""));
        }
        edtCusLotID         ->Text=ReadIniData(sPath, "Lot Info", "Customer Lot ID",AnsiString(""));                    //RogerYang 20251208 : JCET 2D FT1白名單/FT2比對功能

        lbledtMachineID     ->Text=IniConfig.SocketHandlerID;
        coStation           ->Text=ReadIniData(sPath, "Lot Info", "Station",        AnsiString(""));
        edStationNum        ->Text=ReadIniData(sPath, "Lot Info", "Station Number", AnsiString(""));
        edtLotEventLogName  ->Text=ReadIniData(sPath, "Lot Info", "EventLogFile",   AnsiString(""));                    //Steven 20250809 : 修正event log上傳

        for(int i=0; i<3; i++)                                                  //Steven 20250603 : By Lot Summary
        {
            ArmDataLot[i]->ReadFile();
        }
    }
    else
    {
        WriteIniData(sPath, "Lot Info", "Start Time",       lbledtStarTime      ->Text);
        WriteIniData(sPath, "Lot Info", "End Time",         lbledtEndTime       ->Text);
        WriteIniData(sPath, "Lot Info", "Tester OS Ver",    lbledtTesterOsVer   ->Text);
        WriteIniData(sPath, "Lot Info", "Tester ID",        lbledtTesterID      ->Text);
        WriteIniData(sPath, "Lot Info", "Operator",         edtSysOperatorID    ->Text);
        WriteIniData(sPath, "Lot Info", "Customer",         lbledtCustomer      ->Text);
        WriteIniData(sPath, "Lot Info", "Customer Lot ID",  edCustomerLotId     ->Text);
        WriteIniData(sPath, "Lot Info", "Test Program",     lbledtTestProg      ->Text);
        WriteIniData(sPath, "Lot Info", "Device Name",      lbledtDeviceName    ->Text);
        WriteIniData(sPath, "Lot Info", "Lot No",           edtSysLotID         ->Text);
        WriteIniData(sPath, "Lot Info", "Sub Lot No",       lbledtSubLotNo      ->Text);
        WriteIniData(sPath, "Lot Info", "Mode Code",        lbledtModeCode      ->Text);
        WriteIniData(sPath, "Lot Info", "Test Code",        lbledtTestCode      ->Text);
        WriteIniData(sPath, "Lot Info", "Test Bin No",      lbledtTestBinNo     ->Text);
        //AI(W906-LOT-W1) 20260919: GATE (W906-LOT-W1-JCET2DID)，理由同上面讀的那一支。
#if 0 // GATE (W906-LOT-W1-JCET2DID): 缺相依 TfBarCode::JCETUseMakeWhite2DIDList
        if(fBarCode->JCETUseMakeWhite2DIDList()==true)                          //RogerYang 20251208 : JCET 2D FT1白名單/FT2比對功能
        {
            WriteIniData(sPath, "Lot Info", "Run Mode 2DID",    cbRunMode       ->Text);
        }
        else
#endif // GATE (W906-LOT-W1-JCET2DID)
        {
            WriteIniData(sPath, "Lot Info", "Run Mode",         cbRunMode       ->Text);
        }
        WriteIniData(sPath, "Lot Info", "Station",          coStation           ->Text);
        WriteIniData(sPath, "Lot Info", "Station Number",   edStationNum        ->Text);
        WriteIniData(sPath, "Lot Info", "Stage",            edtStage            ->Text);
        WriteIniData(sPath, "Lot Info", "Step",             edtStep             ->Text);
        WriteIniData(sPath, "Lot Info", "EventLogFile",     edtLotEventLogName  ->Text);                                //Steven 20250809 : 修正event log上傳
        WriteIniData(sPath, "Lot Info", "Customer Lot ID",  edtCusLotID         ->Text);                                //RogerYang 20251208 : JCET 2D FT1白名單/FT2比對功能

        for(int i=0; i<3; i++)                                                  //Steven 20250603 : By Lot Summary
        {
            ArmDataLot[i]->WriteFile();
        }
    }
}


//----------------------------------------------------------------------------
//  SetLotID  --  golden uLotInfo.cpp:1492-1570，逐字翻譯。
//
//  相依實測（20260919，全部存在）：
//    edtASECL_LotID / edInsertion / edCustomerDevice / edFlowID / pnlLoader /
//    edtCusLotID / edtSysLotID          forms/fLotInfo.h（既有）
//    edtASECL_OPID / edtASECL_TesterID / edtASECL_LoadBoard / cbRunModeASECL
//                                        本波照 golden :577/:578/:580/:1043 補
//    bFTPDownlodFinish                   cmydef.h:5207（早就有）
//    RunInfo.LotNo / RunInfo.LotStartTime  cprod.h:2731-2732
//    tRecordOEE                          cmydef.h:2597
//    fMain->hanaART->IsHanaArtAvailable()  Automation/HANA_ART.h:247
//    USE_RFID_READER / IniConfig.bN22Enable_EventLog / SystemYear…  既有
//
//  ⚠ 會寫真實檔：`AuthPath+"config.ini"`。驗證照 §0.6。
//
//  ⓘ golden 的兩個怪處，照翻：
//    1. :1520-1522 是一個**空的 if**（`if(edtSysLotID->Text!=""){}`）。
//       golden 就是這樣，不要刪 —— 刪了下一個人會以為原本沒有這個判斷。
//    2. `"Lot ID"` 這個鍵在 ASE-CL 分支裡寫一次（:1538），分支外又寫一次
//       （:1548）。值一樣，所以只是多寫一次，不是衝突。
//----------------------------------------------------------------------------
void TfLotInfo::SetLotID(AnsiString ID, bool bReadFromFile)                     //Steven 20200416 : 整合Lot ID
{
    AnsiString sPath=AuthPath+"config.ini";
    if(bReadFromFile)
    {
        if(IniConfig.bN22Enable_EventLog)                                       //JerryYang 20220215 : ASE-CL
        {
            edtASECL_LotID->Text=ReadIniData(sPath, "Lot Info", "Lot ID", AnsiString(""));

            edtASECL_OPID->Text=ReadIniData(sPath, "Lot Info", "OP_ID", AnsiString(""));
            edtASECL_TesterID->Text=ReadIniData(sPath, "Lot Info", "TESTER_ID", AnsiString(""));
            edtASECL_LoadBoard->Text=ReadIniData(sPath, "Lot Info", "LoadBoard_ID", AnsiString(""));
            cbRunModeASECL->Text=ReadIniData(sPath, "Lot Info", "RunMode", AnsiString(""));
            edInsertion->Text=ReadIniData(sPath, "Lot Info", "Insertion", AnsiString(""));
            edCustomerDevice->Text=ReadIniData(sPath, "Lot Info", "CusDevice", AnsiString(""));
            edFlowID->Text=ReadIniData(sPath, "Lot Info", "FlowID", AnsiString(""));

            bFTPDownlodFinish=ReadIniData(sPath, "Lot Info", "bFTPDownloadFinish", false);
        }

        edtSysLotID->Text=ReadIniData(sPath, "Lot Info", "Lot ID", AnsiString(""));
        edtASECL_LotID->Text=edtSysLotID->Text;

        if(fMain->hanaART->IsHanaArtAvailable())                                //Steven 20250414 : Hana ART
            RunInfo.LotStartTime=ReadIniData(sPath, "Lot Info", "LotStartTime", AnsiString("20240101000000"));
        else
            RunInfo.LotStartTime=ReadIniData(sPath, "Lot Info", "LotStartTime", AnsiString("2020-01-01 00:00:00"));

        //AI(W906-LOT-W1) 20260919: golden :1520-1522 是一個空的 if，照翻不要刪。
        if(edtSysLotID->Text!="")
        {
        }

        if(USE_RFID_READER)                                                     //Steven 20220713 : RFID Reader for SJSEMI
        {
            pnlLoader->Caption  =ReadIniData(sPath, "RFID", "Loader ID",   AnsiString(""));
//            pnlUnloader->Caption=ReadIniData(sPath, "RFID", "Unloader ID", AnsiString(""));
        }
    }
    else
    {
        edtASECL_LotID->Text=ID;
        edtSysLotID->Text=ID;

        if(IniConfig.bN22Enable_EventLog)                                       //JerryYang 20220215 : ASE-CL
        {
            WriteIniData(sPath, "Lot Info", "bFTPDownloadFinish", bFTPDownlodFinish);
            WriteIniData(sPath, "Lot Info", "Lot ID", ID);
            WriteIniData(sPath, "Lot Info", "OP_ID", edtASECL_OPID->Text);
            WriteIniData(sPath, "Lot Info", "TESTER_ID", edtASECL_TesterID->Text);
            WriteIniData(sPath, "Lot Info", "LoadBoard_ID", edtASECL_LoadBoard->Text);
            WriteIniData(sPath, "Lot Info", "RunMode", cbRunModeASECL->Text);
            WriteIniData(sPath, "Lot Info", "Insertion", edInsertion->Text);
            WriteIniData(sPath, "Lot Info", "CusDevice", edCustomerDevice->Text);
            WriteIniData(sPath, "Lot Info", "FlowID", edFlowID->Text);
        }

        WriteIniData(sPath, "Lot Info", "Lot ID", ID);

        if(fMain->hanaART->IsHanaArtAvailable())                                //Steven 20250414 : Hana ART
            RunInfo.LotStartTime.sprintf("%04d%02d%02d%02d%02d%02d",
                                            SystemYear, SystemMonth, SystemDate, SystemHour, SystemMin, SystemSec);
        else
            RunInfo.LotStartTime.sprintf("%04d-%02d-%02d %02d:%02d:%02d",       //pig 2014.05.09 KYEC Issue
                                            SystemYear, SystemMonth, SystemDate, SystemHour, SystemMin, SystemSec);

        WriteIniData(sPath, "Lot Info", "LotStartTime", RunInfo.LotStartTime);
        WriteIniData(sPath, "Lot Info", "Customer Lot ID", edtCusLotID->Text);  //Sam 20220223 : 矽格中興廠新增 Lot 資訊
        if(USE_RFID_READER)                                                     //Steven 20220713 : RFID Reader for SJSEMI
        {
            WriteIniData(sPath, "RFID", "Loader ID",   pnlLoader->Caption);
        }
    }

    if(bReadFromFile && edtSysLotID->Text=="")
        RunInfo.LotNo.sprintf("[%04d%02d%02d.%02d%02d%02d]", SystemYear, SystemMonth, SystemDate, SystemHour, SystemMin, SystemSec);
    else
        RunInfo.LotNo=edtSysLotID->Text;
    tRecordOEE.SetSecAndOn(3600);
}


//----------------------------------------------------------------------------
//  SetLotStart  --  golden uLotInfo.cpp:1572-1970（399 行），逐字翻譯。
//
//  ⭐ 這是 START 被擋住的地方：`WebStart.cpp:2086`（golden :5216）要
//     `RunInfo.bLotStart==true`，而唯一把它設成 true 的路是
//     `SetLotStart` -> `SetLotComponents(false)` -> `RunInfo.bLotStart=!bLotEnd`
//     （那一行本波剛解閘，原 GATE WD-1）。
//
//  相依實測（`scratchpad/ident_check.py`，226 個不重複識別字）：
//    **只有 4 個全樹 0 命中**，其中 `sTestBin` 是 golden :1878 的區域變數
//    （工具誤報），所以真正缺的是 3 個 -> 3 個閘，見下面各自的就地說明。
//
//  ⚠ 會寫真實檔：`AuthPath+"config.ini"`（:1826 的 Job Sequence，以及
//    它呼叫的 `ReadWriteLotInfo(false)` / `SetLotID`）。驗證照 §0.6。
//----------------------------------------------------------------------------
void TfLotInfo::SetLotStart(AnsiString sFunc, bool bReadFromFile)               //Steven 20250515 : 整合Lot測試報表
{
    RecordProcess("Start Lot Press", sFunc);

    AnsiString sPath=AuthPath+"config.ini";
    AnsiString str, Msg;

    if(bReadFromFile)
    {
        SetLotID("", true);
        ReadWriteLotInfo(true);                                                 //KaiChen 20181121 ：矽格-北興 Save Lot Info
        //AI(W906-LOT-W1) 20260919: GATE (W906-LOT-W1-SLEVENTLOG) —— 缺相依，
        // **而且是端到端實跑抓到的當機**（wb_serve exit 5，log 停在
        // `[RecordProcess] Start Lot Press` 的下一步）。
        //
        // `slEventLog` 是 `cmydef.cpp:120` 的**裸指標，從未建構**
        // （與 `fTeach` / `elTeach` 同一族）。`cMyDB.cpp:58-66` 早就記著
        // 「its AddTextWithDateTime/AddTextWithLineNo ... cannot be called」，
        // 本波只是第一個真的去 deref 它的地方。
        //
        // ⚠ 行為：工單的起訖資料不會被寫進 event log 緩衝區。
        //   那是**紀錄**不是機台動作，也不影響啟動許可。
        // UN-GATE 有兩條路，都要另外決定：
        //   (a) 在 wb_serve 建構 `slEventLog`（它會寫檔，要先釐清寫到哪）
        //   (b) 等 golden 的 `TfMain` ctor 那條建構路徑整段落地
#if 0 // GATE (W906-LOT-W1-SLEVENTLOG): 缺相依 —— slEventLog 從未建構（cmydef.cpp:120 裸指標）
        slEventLog->SetLotData(RunInfo.LotNo, lbledtStarTime->Text, edtLotEventLogName->Text);
#endif // GATE (W906-LOT-W1-SLEVENTLOG)
    }
    else
    {
        SetLotID(edtSysLotID->Text);
        if(lbledtStarTime->Text=="")
        {
            str.sprintf("%04d%02d%02d_%02d%02d%02d", SystemYear, SystemMonth, SystemDate, SystemHour, SystemMin, SystemSec);
            lbledtStarTime->Text=str;
        }

        for(int i=0; i<3; i++)                                                  //Steven 20250603 : By Lot Summary
        {
            ArmDataLot[i]->ClearALLCT();
        }

        //AI(W906-LOT-W1) 20260919: GATE (W906-LOT-W1-SLEVENTLOG) —— 同上面那則。
#if 0 // GATE (W906-LOT-W1-SLEVENTLOG): 缺相依 —— slEventLog 從未建構
        slEventLog->SetLotData(RunInfo.LotNo, lbledtStarTime->Text);            //KaiChen 20181121 ：矽格-北興 Save Lot Info
#endif // GATE (W906-LOT-W1-SLEVENTLOG)
        //==> Eastsun 20260527 整合 #027-2.MR.U7 Lot Start log with Barcode Recipe :KYEC
        if(TestIF_File.bEnableBarCode==true &&
           BAR_CODE_INSTALL==ebctUseCCDMode &&
           TestIF_File.bBarCodeMultiRecipe==true)
        {
            Msg.sprintf("Lot Start, Lot ID:%s, OP ID:%s, Run Mode:%s, 2D Recipe:%s", edtSysLotID->Text, edtSysOperatorID->Text, cbRunMode->Text, edtBarcodeRecipe->Text);
        }
        else
        {
            Msg.sprintf("Lot Start, Lot ID:%s, OP ID:%s, Run Mode:%s", edtSysLotID->Text, edtSysOperatorID->Text, cbRunMode->Text);
        }
        //<== Eastsun 20260527 #027-2.MR.U7
        RecordProcess(Msg, sFunc);
        ReadWriteLotInfo(false);
    }

    SetTesterStartTimeByB03();                                                  //Sam 20240809 : PTI ART 模式

    if(CosFunction.bRunModeFollowLotInfo)                                       //Steven 20250603 : 根據Lot Info的Run mode進行切換
    {
        //AI(W906-LOT-W1) 20260919: GATE (W906-LOT-W1-DOFTRT) —— 缺相依：
        // 移植樹的 `TfMain` 沒有 `DoFTRTClick`（g++ 建議 `FTClick`，那是**不同**
        // 的東西：golden 20260410 把手動按下與程式按下整合成 DoFTRTClick(bRT, bManual)，
        // `FTClick` 是它整合掉的其中一半）。硬接 `FTClick` 會把「程式按下」
        // 當成「手動按下」，那是改行為不是翻譯。
        // ⚠ 行為：`CosFunction.bRunModeFollowLotInfo` 開著時，Lot Info 的
        //   Run Mode 不會自動切換 FT/RT。操作員要自己在主畫面切。
        // UN-GATE：等 `TfMain::DoFTRTClick` 落地。
#if 0 // GATE (W906-LOT-W1-DOFTRT): 缺相依 TfMain::DoFTRTClick
        if(cbRunMode->Text.AnsiPos("RT")==1)
        {
            fMain->DoFTRTClick(true, false);                                  //RogerYang 20260410 : 整合並區分手動按下還是程式按下
        }
        //AI(W906-LOT-W1) 20260919: GATE (W906-LOT-W1-PALEQC) —— 缺相依：
        // 移植樹的 `TfMain` 沒有 `palEQCClick`（`forms/fMain.h` 0 命中；
        // 全樹 `git grep -lw palEQCClick` 也是 0 個檔）。
        // ⚠ 行為：EQC 模式的 Run Mode 不會切換 —— 走下面的 else（FT）。
        //   EQC 是「工程品質檢驗」模式，缺它等於這台機器用 Lot Info 的
        //   Run Mode 自動切換時，選 EQC 會被當成 FT 跑。
        //   ⓘ 只在 `CosFunction.bRunModeFollowLotInfo` 開著時才會走到這裡。
        // UN-GATE：等 `TfMain::palEQCClick` 落地。
#if 0 // GATE (W906-LOT-W1-PALEQC): 缺相依 TfMain::palEQCClick
        else if(cbRunMode->Text=="EQC")
        {
            fMain->palEQCClick(fMain);
        }
#endif // GATE (W906-LOT-W1-PALEQC)
        else
        {
            //fMain->palFTClick(fMain);
            fMain->DoFTRTClick(false, false);                                 //RogerYang 20260410 : 整合並區分手動按下還是程式按下
        }
#endif // GATE (W906-LOT-W1-DOFTRT)
    }

    RunInfo.SetLotStartTime();                                                            //Sam 20240426 : Add BarCoder Inspection Report  Eastsun 20260527 整合#BCIR.P16 :KYEC

    if(IniConfig.bVTESTFunction==true)                                          //jou 20200409 : VTest Mes system
    {
        cbProcess->Enabled=false;
        edtProcessName->Enabled=false;
        edtProduct->Enabled=false;
        cbTestTimes->Enabled=false;                                             //RogerYang 20250809 偉測Summary文件修改
        if((IniConfig.bEnableRms || IniConfig.bEnableFTP) && IniConfig.bCheckFile)
        {
            if(fMesSystem->CheckVTENGmode(edtSysLotID->Text)==true)
            {
                fMesSystem->bDownloadLotInforFlag=true;
                bFTPDownloadSetupFile = true;
                LastSet.bHasDownloadFile=true;
                RecordProcess("ENG mode lot start");
            }

            LastSet.SendCT_ART[0]=0;
            LastSet.SendCT_ART[1]=0;
            LastSet.SendCT_ART[2]=0;
            LastSet.SendCT_ART[3]=0;

            for(int i=0; i<eTrayCount; i++)                                     //kevin 20110926  無法清fix6
            {
                LastSet.BinCT_ART[0][iTo3Unload[i]]=0;
                LastSet.BinCT_ART[2][iTo3Unload[i]]=0;
                LastSet.BinCT_ART[3][iTo3Unload[i]]=0;
            }

            for(int i=0; i<eTrayCount; i++)
            {
                LastSet.iBinData32_ART[0][i]=0;
                LastSet.iBinData32_ART[2][i]=0;
                LastSet.iBinData32_ART[3][i]=0;
            }

            ArmData[2]->ClearALLCT();

            if(CUSTOMER_CODE==CC_VTEST)
            {
                edtProcessName->Enabled=false;
                edtProduct->Enabled=false;
            }
        }

        fMesSystem->bFirstMaterialsQA=true;                                     //jou 20230207 : VTEST 首盤請執行送檢，通知QA確認
    }

    if(CUSTOMER_CODE==CC_PTI && IniConfig.bB03_TesterReport)                    //Sam 20240809 : PTI ART 模式
        cbProcess->Enabled=false;

//    if((IniConfig.bSPILFunction==true ||                                      //Steven 20250806 : Mark
//        CUSTOMER_CODE==CC_SJ_Semiconductor) &&
    if(CosFunction.bSortingBy2DList==true &&
       LastSet.iTester==_2D_SORT &&
       TestIF_File.bSortingBy2DIDList==true &&
       RunInfo.bLotStart==false)
    {
        fMain->Clarn_Data(8, "LotStart_ClearData");
        fSCKART->ClearLotInfo();
        EventReport(SECS_EVENT.DoVisualSortLotStart);
    }
    else
    {
        if(CUSTOMER_CODE!=CC_TSMC_TAINAN &&
           IniConfig.bEnable_SECS_GEM==true &&                                  //Steven 20140528 : Secs Gem
           RunInfo.bLotStart==false)                                            //wei 20150821 不能連續送
            EventReport(SECS_EVENT.DoLotStart);
    }

    if(edtSysLotID->Text!="")
        SetLotComponents(false);
    else
        SetLotComponents(true);
    edtBarcodeRecipe->Enabled=false;   //==> Eastsun 20260527 整合#027-2.MR.M5 explicit LotStart disable (parity with a-side L7225) :KYEC
    //==> Eastsun 20260527 整合#027-2.MR.U6 Change2DSetupFile on LotStart :KYEC
    if(TestIF_File.bEnableBarCode==true &&
       BAR_CODE_INSTALL==ebctUseCCDMode &&
       TestIF_File.bBarCodeMultiRecipe==true)
    {
        //AI(W906-LOT-W1) 20260919: GATE (W906-LOT-W1-BARCODE) —— 缺相依：
        // `TfBarCode`（aHotPlateSubstrate.h:1037）沒有 `Change2DSetupFile`。
        // 與本檔上面 JCET 那個閘同一個類別、同一個理由（既有慣例 G-1 / WA-7）。
        // ⚠ 行為：開了 2D 多配方的 KYEC 機台，Lot Start 不會依 2D 碼換配方。
#if 0 // GATE (W906-LOT-W1-BARCODE): 缺相依 TfBarCode::Change2DSetupFile
        fBarCode->Change2DSetupFile();
#endif // GATE (W906-LOT-W1-BARCODE)
    }
    //<== Eastsun 20260527 整合#027-2.MR.U6

    if(TestIF.iTestType==TCP_IP_MODE)                                           //Steven 20230322 : for OS Tester
    {
        if((CUSTOMER_CODE==CC_SJ_Semiconductor_OS ||                            //Steven 20230213 : For SJSemi OS Tester
            CUSTOMER_CODE==CC_XINITECH) &&
           IniConfig.bEnable_SECS_GEM)
        {
                                                                                //透過SECS GEM傳送Lot ID
        }
        else
        {
    //AI(W906-LOT-W1) 20260919: GATE (W906-LOT-W1-TESTERTCP) —— 缺相依：
    // `fTesterTCP` 這個全域在移植樹**沒有**。`Interface/TesterTCP_Socket.h:159`
    // 只在註解裡提到 golden 的 `extern PACKAGE TfTesterTCP *fTesterTCP;`，
    // 沒有真的宣告出來。
    // ⚠ 行為：`TestIF.iTestType==TCP_IP_MODE` 的機台（OS Tester），
    //   Lot Start 不會把 LOTNUMBER / LOTSTART / OPERATORID / GETOSSETUP
    //   這四條指令送給測試機。**那是對外通訊，不是機台動作。**
    //   後果是測試機那邊不知道換了工單。
    // UN-GATE：等 `fTesterTCP` 的 extern 與 `TfTesterTCP` 落地。
#if 0 // GATE (W906-LOT-W1-TESTERTCP)
            if(iRunStartMode==RT)
                str.sprintf("LOTNUMBER,%s,RT,", fLotInfo->edtSysLotID->Text);
            else
                str.sprintf("LOTNUMBER,%s,FT,", fLotInfo->edtSysLotID->Text);
            //AI(W906-LOT-W1) 20260919: GATE (W906-LOT-W1-PTITRACE) —— 缺相依：
            // `WritePTILotStartTrace` 全樹 0 命中，而且它**不是 golden 原生的**
            // —— 那兩行的註解自己寫著 `//AI(ht9045-v899) 20260525: mark PTI
            // tester TCP command window for StateRecord analysis`，
            // 是 BCB6 那邊為了 StateRecord 分析加的診斷輔助。
            // ⚠ 行為：少兩行診斷用的 trace 標記，**TCP 指令照送**（:1736-1740
            //   四行都是活的）。這個閘不影響機台行為。
            // UN-GATE：如果要把那個診斷輔助也移植過來。
#if 0 // GATE (W906-LOT-W1-PTITRACE): 缺相依 WritePTILotStartTrace（非 golden 原生，是 V899 的診斷輔助）
            WritePTILotStartTrace(this, "BeforeTesterTCPSend", "Commands=LOTNUMBER/LOTSTART/OPERATORID/GETOSSETUP");
#endif // GATE (W906-LOT-W1-PTITRACE)
            fTesterTCP->SendTCPIPCommand(0, "LOTNUMBER", str);
            fTesterTCP->SendTCPIPCommand(0, "LOTSTART", "LOTSTART");
            str.sprintf("OPERATORID,%s,", fLotInfo->edtSysOperatorID->Text);
            fTesterTCP->SendTCPIPCommand(0, "OPERATORID", str);
            fTesterTCP->SendTCPIPCommand(0, "Get OS Setup", "GETOSSETUP");      //Steven 20230505 : 取得OS Tester資訊
#endif // GATE (W906-LOT-W1-TESTERTCP)

#if 0 // GATE (W906-LOT-W1-PTITRACE): 同上
            WritePTILotStartTrace(this, "AfterTesterTCPSend", "Commands queued to TesterTCP");
#endif // GATE (W906-LOT-W1-PTITRACE)
        }
    }

    if((CUSTOMER_CODE==CC_KYEC_LEE ||
        CUSTOMER_CODE==CC_KYEC_XILINX) &&                                       //jou 2015-10-02 Auto Retest GPIB mode
       IniConfig.bA10_AutoReTest)                                               //wei 20151230 art 紀錄資料
    {
        //AI(W906-LOT-W1) 20260919: GATE (W906-LOT-W1-SAVEOPID) —— 缺相依：
        // `SaveLotOperatorID` 全樹只有註解命中（forms/fLotInfo.h:1037 把它列在
        // 尚未移植的清單裡，golden uLotInfo.cpp:8547-8548）。
        // ⚠ 行為：KYEC-Lee / KYEC-Xilinx 開 A10 自動複測時，操作員 ID 不會被
        //   寫進 ART 的紀錄檔。
#if 0 // GATE (W906-LOT-W1-SAVEOPID): 缺相依 SaveLotOperatorID（golden uLotInfo.cpp:8547）
        SaveLotOperatorID(fLotInfo->edtSysLotID->Text.c_str(), fLotInfo->edtSysOperatorID->Text.c_str(), false);
#endif // GATE (W906-LOT-W1-SAVEOPID)
    }

    if(CosFunction.bUseLogUploadToFTPFunction==true)
    {
        bSysLotStart      =true;                                                //Ifor 20160302 add for KYEC_HS LotStart
        bEPLogStart_KYEC  =true;                                                //Ifor 20160302 add for KYEC_HS EPLogStart
        bTempLogStart_KYEC=true;                                                //Ifor 20160302 add for KYEC_HS TempLogStart
        if(USE_NOVX3360==true)
        {
            bESDLogStart_KYEC=true;                                             //Ifor 20160302 add for KYEC_HS ESD
        }
        asATCEvenLotID=fLotInfo->edtSysLotID->Text;                             //Ifor 20170124 (Steven) :add LotID By ATC Even Log
        bArmTestInfoEvenLogStart_KYEC=true;                                     //Ifor 20190912 :add 海思 V02.30 版 Record Torque
    }

    if(CosFunction.bOEEFunction)                                                //Steven 20180417 (Jou) : OEE功能
    {                                                                           //Sam 20170719 (Steven) 移植超豐 OEE 功能 form HT-7045
        lb_PIOEELotStatus->Caption="Production Start Lot.....";

        //AI(W906-LOT-W1) 20260919: GATE (W906-LOT-W1-SORTCTCLEAR) —— 缺相依：
        // 移植樹的 `TfSortCT` 沒有 `btnClearCountClick`。
        // ⚠ 行為：開了 OEE 的機台，Lot Start 不會清 Sort Count 頁的計數 ——
        //   新工單會沿用上一個工單的分類計數。**那是計數不是動作**，
        //   但它會讓 OEE 的產出數字錯。
#if 0 // GATE (W906-LOT-W1-SORTCTCLEAR): 缺相依 TfSortCT::btnClearCountClick
        fSortCT->btnClearCountClick(fSortCT);
#endif // GATE (W906-LOT-W1-SORTCTCLEAR)
        if(IniConfig.bN14_4_OEEAutoLoadMOFile==false)                           //Mylin 20170524 Add Auto Load MO By Option
        {
            fProductionInfo->OEE_SetMO(fLotInfo->ed_PIOEEMO->Text);
        }
        fProductionInfo->OEE_SetHandlerID(IniConfig.SocketHandlerID);
        lb_PIOEELotStatus->Caption="Production Start Lot Success!";
        //AI(W906-LOT-W1) 20260919: golden 寫 `Now().FormatString(fmt)`。
        // `vclcompat::TDateTime` 沒有 `FormatString` 成員，但 vclcompat 提供
        // **同一件事**的自由函式 `FormatDateTime(fmt, dt)`（vclcompat/TDateTime.h:67，
        // 檔頭寫明 token set 是對 golden 用法驗過的）。
        // ⇒ 機械式 API 形狀轉換，不是語意偏離。全檔 3 處，這是第 1 處。
        ASET_StartTimeNAME=FormatDateTime("yyyymmdd_hhnn", Now());
        if(MOT[MMTrayY].Tray.HasIC()==false)
        {
            ASE_InTrayNum=0;
            iLoadTrayCount=0;
        }

        ClearAllTrayCount();                                                    //Steven 20251029 : outputtray 數量
        for(int i=0; i<MAX_SOCKET_ROW; i++)
        {
            for(int j=0; j<MAX_SOCKET_COL; j++)
            {
                LastSet.iSocketContactCount[i][j]=0;
            }
        }

        //AI(W906-LOT-W1) 20260919: GATE (W906-LOT-W1-TESTRECVCNT) —— 缺相依：
        // 移植樹的 `TfObserver` 有 `sTestReceiveTime`（forms/fObserver.h:808）
        // 但**沒有** `iTestReceiveTimeCount`（golden cObserver.h:542）。
        // 那個欄位的計數端未翻，所以連欄位本身都沒宣告。
        // ⚠ 行為：OEE 的 Test-Time 計數不會被 Lot Start 歸零。
        //   ⓘ forms/fObserver.h:805-807 自己已經記著「OEE CSV 的
        //     Test-Receive-Time 欄位會恆為空，直到寫入點落地」——
        //     計數同理，這個閘只是把同一個既有缺口在這裡也標出來。
        // UN-GATE：與 `sTestReceiveTime` 的寫入點同一天解。
#if 0 // GATE (W906-LOT-W1-TESTRECVCNT): 缺相依 TfObserver::iTestReceiveTimeCount
        fObserver->iTestReceiveTimeCount=0;                                     //KaiChen 20171127 (Steven) ：超豐 OEE 新增 Test Time、Index Time
#endif // GATE (W906-LOT-W1-TESTRECVCNT)
        fObserver->sTestReceiveTime="";
        fObserver->bTestIndexZ=false;
        fObserver->iTestIndexZCount=0;
        fObserver->sTestIndexZTime="";
        fObserver->dOEEIndexCycleTime=0;                                        //Sam 20180802 (wei) : OEE 32Site 修正
    }

    if(CUSTOMER_CODE==CC_KYEC_LEE)
    {
        ZeroMemory(iSLT_HeadContactCount, sizeof(iSLT_HeadContactCount));       //Ifor 20191218 : add KYEC 要求 同SLT輸出表格
        asSLT_LotStartTime=GetDateInfoByString("/")+" "+GetOnlyTimeInfoByString(":");
    }

    if(CosFunction.bBarcodeTrayRecFile==true)                                   //jou 20190930 : Barcode Tray record file
    {
        //AI(W906-LOT-W1) 20260919: GATE (W906-LOT-W1-BARCODE) —— 同上，
        // `TfBarCode` 沒有 `InitBarcodeRecFile`。
        // ⚠ 行為：開了 `bBarcodeTrayRecFile` 的機台，Lot Start 不會開新的
        //   barcode tray 紀錄檔 —— 會續用上一個 lot 的檔。
#if 0 // GATE (W906-LOT-W1-BARCODE): 缺相依 TfBarCode::InitBarcodeRecFile
        fBarCode->InitBarcodeRecFile();
#endif // GATE (W906-LOT-W1-BARCODE)
    }

    if(CUSTOMER_CODE==CC_SCC ||                                                 //Steven 20200306 : SCC要求按下Lot End的時候, 資料要清空
       CUSTOMER_CODE==CC_Murata ||
       CUSTOMER_CODE==CC_SJ_Semiconductor ||
       CUSTOMER_CODE==CC_CYUEAN)                                                //Steven 20240122 : add for CyuEan
    {
        //AI(W906-LOT-W1) 20260919: GATE (W906-LOT-W1-MAINPANELS) —— 缺相依：
        // `TfMain` 這五個 widget 一個都沒有（`cbRunStartMode` / `palFT` /
        // `palRT` / `palOffLine` / `palEQC`）。forms/fMain.h 是刻意的部分移植。
        // ⚠ 行為：SCC / Murata / SJ-Semi / **CYUEAN（就是這台）** 這四家客戶碼，
        //   Lot Start 之後主畫面的 Run Mode 切換按鈕**不會被鎖住**。
        //   golden 鎖它們是為了「開工之後不准中途改模式」。缺了這一段，
        //   操作員可以在跑的時候按 FT/RT/OffLine/EQC。
        //   ⓘ 這是**顯示層的鎖**，不是啟動許可；`StartFromWeb` 的判斷不經過它。
        // UN-GATE：等 fMain 的那五個 widget 落地。
#if 0 // GATE (W906-LOT-W1-MAINPANELS): 缺相依 TfMain::cbRunStartMode/palFT/palRT/palOffLine/palEQC
        fMain->cbRunStartMode->Enabled=false;
        fMain->palFT->Enabled=false;
        fMain->palRT->Enabled=false;
        fMain->palOffLine->Enabled=false;
        fMain->palEQC->Enabled=false;
#endif // GATE (W906-LOT-W1-MAINPANELS)
    }

    if(CosFunction.bSpecailLowYeild)                                            //Sam 20211221 : Lot Start 才需要重新第一階段檢查
        fCounterClear->LowYieldSpecialInitail();

    WriteIniData(sPath, "Lot Info", "Job Sequence", edtJobSeq->Text);           //JerryYang 20220923 : add for SLT lot summary

    if(CosFunction.bSortingBy2DList==true &&
       LastSet.iTester==_2D_SORT &&
       TestIF_File.bSortingBy2DIDList==true)                                    //Frank 20221122 : 2DID sorting for ATK
    {
        fSCKART->sLotStartTime=FormatDateTime("yyyymmddhhnnss", Now());  //AI(W906-LOT-W1) 20260919: FormatString -> FormatDateTime（第 2 處）
        fSCKART->AccessFile(false, 1);
    }

    //AI(W906-LOT-W1) 20260919: GATE (W906-LOT-W1-SLTSUMMARY) —— 缺相依，整塊。
    //
    // 這一塊（golden uLotInfo.cpp:1836-1943，約 108 行）是 SLT lot summary，
    // 它靠 `TfSCKART` 的 `sInfo_*` 欄位族。移植樹的 `TfSCKART`
    // （Automation/SCK_ART.h）**一個 sInfo_ 都沒有**（`grep -c sInfo_` = 0），
    // 另外 `TfObserverMemoLotSummary` 也沒有 `Clear`。
    //
    // 為什麼整塊閘而不是逐行：缺的是**同一族**相依（14 個 sInfo_* 欄位
    // 加一個 memo）。逐行閘會產生 20 個微閘、把一個連貫的功能切成碎片，
    // 而且每一個微閘都要自己解釋同一件事。
    // ⇒ 一個閘、一個理由、一個 UN-GATE 條件。
    //
    // ⚠ 行為：開了 `IniConfig.bA38_SLT_Summary` 的機台，Lot Start 不會
    //   重建 ART 的工單資訊（LotID/Stage/Step/BinNo/HandlerID…），也不會
    //   依 93K/Flex 決定 `SetLotState`。**那是報表與 SECS 狀態，不是機台動作。**
    // ⓘ 這台機器要不要走這一塊，看 `IniConfig.bA38_SLT_Summary`。
    // UN-GATE：等 `TfSCKART` 的 sInfo_* 欄位族落地（那是 SCK_ART 那一波）。
#if 0 // GATE (W906-LOT-W1-SLTSUMMARY)
    if(IniConfig.bA38_SLT_Summary)                                              //JerryYang 20220923 : add for SLT lot summary
    {
        bool bChangeLotID=false;
        AnsiString strLotID=edtSysLotID->Text;
        if(HasICUnderMachine()==false && HasAnyICInMachine()==false)
        {
            bChangeLotID=true;
        }
        else
        {
            if(strLotID==fSCKART->sLotID && (strLotID!="" && strLotID!=" "))
            {
                bChangeLotID=false;
            }
            else
            {
                bChangeLotID=true;
            }
        }

        fObserver->memoLotSummary->Clear();
        if(bChangeLotID==true)
        {
            fSCKART->iCurrent93KARTStep=1;
            fSCKART->iCurrentFlexARTStep=4;
            fMain->Clarn_Data(1, "ART_INPUTQTY");
            fSCKART->ClearLotInfo();
            RecordProcess("ART INPUTQTY.");
            fLotInfo->btClearBarcodeList->Click();                              //Steven 20190214 : 統一清除2DID方式

            fSCKART->SetLotStatus(fSCKART->iLOTSTATUS_W);
    //        AnsiString Str = "1,2,3,4,5,6,7,8,9";
            TStringList *ss=new TStringList;
            str=StringReplace(strLotID, "-", ",", TReplaceFlags()<<rfReplaceAll);

            ss->CommaText=str;
            for(int i=0; i<ss->Count; i++)
            {
                 ss->Strings[i];                                                //??就得到被分割的字符串
            }

            fSCKART->sInfo_TestBinNo="";
            AnsiString sTestBin="";
            if(ss->Strings[2]=="RC")
            {
                for(int i=0; i<iTestBinCount; i++)                              //從Bin1開始, 比照Epson所定義的格式
                {
                    int iBinTray=Prod.iT6CatData[i];
                    if(Prod.bCateRTo6Tray[iBinTray]==true)
                    {
                        sTestBin.sprintf("%02d", i);
                        if(fSCKART->sInfo_TestBinNo=="")
                        {
                            fSCKART->sInfo_TestBinNo=sTestBin;
                        }
                        else
                        {
                            fSCKART->sInfo_TestBinNo=fSCKART->sInfo_TestBinNo+"-"+sTestBin;
                        }
                    }
                }
            }
            else
            {
                fSCKART->sInfo_TestBinNo="All";
            }

            fSCKART->sLotStartTime=FormatDateTime("yyyymmddhhnnss", Now());  //AI(W906-LOT-W1) 20260919: FormatString -> FormatDateTime（第 3 處）
            fSCKART->sInfo_Customer="";
            fSCKART->sLotID.sprintf("%s", ss->Strings[0]);                      //Lot ID
            fSCKART->sInfo_CustLotID="";
            fSCKART->sInfo_CustDevGup="";
            fSCKART->sInfo_DeviceName="";
            fSCKART->sInfo_Stage=ss->Strings[1];                                //Stage
            fSCKART->sInfo_Step=ss->Strings[2];                                 //Step
            fSCKART->sInfo_ReportCnt=edtJobSeq->Text;                           //Job Sequence
            fSCKART->sInfo_ProgramName="";
            fSCKART->sInfo_TesterID="";
            fSCKART->sInfo_HandlerID=IniConfig.SocketHandlerID;
            fSCKART->sInfo_Temperauture="";
            fSCKART->sInfo_CurrQty="";
            fSCKART->sInfo_OperatorID=edtSysOperatorID->Text;
            fSCKART->AccessFile(false, 1);
            str.sprintf("Lot start: %s", fSCKART->sLotID);
            RecordProcess(str);                                                 //Steven 20190722 : add TSV log

            if(fSCKART->iTesterType==1 || IniConfig.bA37LotStartLotEnd)         // && CosFunction.bAutoRetestGPIBmode==true) //Steven 20170309 (wei) : Fixed for ART
            {
                fSCKART->iNeedRT=0;
                if(LastSet.iRunStartMode==rsmInitial_ART || LastSet.iRunStartMode==rsmContinuStart_ART ||
                   LastSet.iRunStartMode==rsmContinuRetest_ART || LastSet.iRunStartMode==rsmAutoRetest)
                {
                    fMain->SetLotState(2);                                      //SECS ART FT Start
                }
                else
                {
                    if(fSCKART->sInfo_Step==FT)
                        fMain->SetLotState(2);                                  //SECS ART FT Start
                    else
                        fMain->SetLotState(4);                                  //SECS ART FT Start
                }
                LastSet.bEndLotAutoRetestGPIB=false;
                LastSet.bWaitStartLotAutoRetestGPIB=false;
                LastSet.bFirstTestAutoRetestGPIB=true;
            }
            delete ss;
        }
    }
#endif // GATE (W906-LOT-W1-SLTSUMMARY)


    if(CosFunction.bIndexCheckCanTurnOff &&
       IniConfig.iD71IndexCheckOnOffMode==0)                                    //Isaac 20211019 : 可選擇做index check的時機，lot start後，要做index check
    {
        bLotStartEndNeedIndexCheck=true;
    }
    else
    {
        bLotStartEndNeedIndexCheck=false;
    }

    if(IniConfig.bUseAutoSiteMapping==true &&                                   //Jimmychiu 20230707 : Auto Site Mapping Trigger Function
       IniConfig.bI21EnableASM==true &&
       IniConfig.bI50_EnableAutoSiteMappingTrigger==true &&
       IniConfig.bI50_StartLot==true)
    {
        RecordProcess("Trigger Auto Site Map after Start Lot [I50]");
        SetRunStartMode(rsmAutoSiteMap);
    }

    if(IniConfig.bVTESTFunction==true)                                          //RogerYang 20250604 偉測不可複測bin功能
    {
        CheckNoRetestBinFlag();
    }

    LastSet.iAutoTempOfsTriggerCnt=0;                                           //Sam 20220406 : 溫度自動補償功能 By FTP
}

// =============================================================================
//  Steven 20260925 (Data.LotInfo 其餘分頁) —— golden V912 uLotInfo.cpp／.h／.dfm
//  （D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy，cp950）。行號一律 V912。
//
//  這一段做什麼
//    Data.LotInfo.html 原本只有 Lot 分頁接真資料（lot.* 五個 tag）。其餘分頁要照 golden 顯示，C++ 這裡提供：
//      W906_RefreshTabVisible()     37 個 TTabSheet 的 TabVisible —— golden 所有寫 TabVisible 的敘述，依 golden 執行順序
//      W906_FormShowActivePage()    FormShow 開窗時選中哪一頁
//      W906_ShowATCThermoDisplay()  ATC 分頁：標題、Working Temperature、ATC On/Off Line、各面板／CH 的可見度
//      W906_DoBarcodeCount()        BarCode 分頁 sgBarcode 的格子（golden TfBarCode::DoBarcodeCount）
//      btClearBarcodeCountClick()   BarCode 分頁 Clear Count（golden :10168-10184）
//      TfLotInfoLogMemo / W906_TesterLogTail   Tester Log 分頁的 mmTesterLog
//    WebBridgeTags.cpp 檔尾 W906_StageLotInfoTabTags 每拍呼叫前三個（＋W906_DoBarcodeCount），再把元件狀態送成 tag。
//
//  為什麼不直接呼叫 FormShow()／Timer2Timer()（本檔 :3023／:4141 已翻）
//    golden 開機會 TfMain::DoShowUserDefFrom → fLotInfo->Show()（V912 main.cpp:9236）→ FormShow；Timer2Timer 是 1 秒的 TTimer。
//    兩者除了 TabVisible 還有別的副作用：FormShow :672 bStartATCRun=false（ATC 運轉旗標）、:1199 SetLotStart(true)
//    （GATE WC-19，會打開 RunInfo.bLotStart）、cbRunMode 選項／Text（會改 Lot 分頁 lot.start 寫進 config.ini 的 Run Mode）、
//    ShowXMLOnLine／RefreshAMR／ReadIniData(config.ini)；Timer2Timer 有 WinWay 連線、ShowMyMessage、WAR16123。
//    顯示層不能順手做這些，所以這裡只抽「寫 TabVisible／寫這幾個顯示元件」的敘述，一行不改地照 golden 順序排，
//    每一行標 golden 行號；其他副作用一律不做（各函式內逐條註明）。
//
//  穩態假設（寫在這裡，因為它決定了取哪一支）
//    * Timer2Timer :7055 `if(InitialOK==false) return;` —— golden InitialOK 在 TfMain::FormShow（V912 main.cpp:10900）
//      開機就設 true，機台跑著時 Timer2Timer 一定有跑，所以它的 TabVisible 指派算數（覆蓋 FormShow 的同名指派）。
//    * 每拍從同一個起點重算：設定不變時，結果＝golden 開機（FormShow）後 Timer2Timer 跑過的穩態。
//      golden 由事件寫、而且不會被重算蓋掉的那幾個（main.cpp:16368 Qorvo GPIB Pause 把 tsRFMD 打開）不在這裡，逐條註明。
// =============================================================================
#include "SgdToXLS.h"          // SGDToXLS（btClearBarcodeCountClick；移植樹本體是 no-op，SgdToXLS.cpp GATE (1)）

// ---- mmTesterLog 的真 memo（宣告在 forms/fLotInfo.h 檔尾）----------------------------------------------
void TfLotInfoLogMemoLines::Add(AnsiString s)
{
    L.push_back(s);
    Count=(int)L.size();
}

TfLotInfoLogMemo::TfLotInfoLogMemo()
{
    delete Lines;                                                               // TfMainMemo() 建的 no-op 那一份
    Log=new TfLotInfoLogMemoLines();
    Lines=Log;
    Log->Add("");                                                               // dfm :11283-11284 Lines.Strings=('')
}

void TfLotInfoLogMemo::Clear()
{
    Log->L.clear();
    Log->Count=0;
}

// mmTesterLog 最後 nLines 行（以 "\n" 連接）；*pCount＝golden Lines->Count。mmTesterLog 不是 TfLotInfoLogMemo 時回空字串、Count=-1。
AnsiString TfLotInfo::W906_TesterLogTail(int nLines, int* pCount)
{
    TfLotInfoLogMemo* m=dynamic_cast<TfLotInfoLogMemo*>(mmTesterLog);
    if(pCount) *pCount=(m ? m->Log->Count : -1);
    AnsiString s;
    if(m==0 || nLines<=0)
        return s;
    const int n=(int)m->Log->L.size();
    const int from=(n>nLines) ? n-nLines : 0;
    for(int i=from; i<n; i++)
    {
        if(i>from) s+="\n";
        s+=m->Log->L[i];
    }
    return s;
}

// ---- TabVisible（37 個 TTabSheet）-------------------------------------------------------------------------
void TfLotInfo::W906_RefreshTabVisible()
{
    // (0) dfm 設計期值：V912 uLotInfo.dfm 的 37 個 TTabSheet 沒有一個寫 TabVisible=False（20260925 全檔掃描 0 筆），
    //     所以 golden 只在特定條件才改的頁，條件不成立時是 true。這四頁 golden 的寫入全都掛在條件下，從 true 起算；
    //     其餘 33 頁下面都有無條件（或 if／else 兩支都寫）的指派。移植樹 vclcompat::TTabSheet 預設 false（Controls.h:510）。
    tsLotID->TabVisible=true;                                                   // 只有 :392（CC_PANTHER）、:1035（CC_GIGAS）會改
    tsDeviceInfo->TabVisible=true;                                              // bVTESTFunction 時 Timer2Timer :7241 不寫，FormShow :368／:374／:494 也都在條件下
    tsTPW->TabVisible=true;                                                     // 只有 :621（在 :618 的 if 裡）
    tsSigurd->TabVisible=true;                                                  // 只有 :622（同上）

    // (1) 設定載入時（ProcessLastSetIni 等）golden 寫的；FormShow 在它們之後才跑
    tsASECLEventLog->TabVisible=IniConfig.bN22Enable_EventLog;                  // V912 cprod.cpp:2317（移植樹 cprod.cpp:2423 GATE GA1-B2；下面 :1046 會覆寫）
    ts2DSort->TabVisible=(CosFunction.bSortingBy2DList &&                       // V912 cprod.cpp:2318-2320（同上 GATE）
                          (IniConfig.iN23DownloadMethod==3 ||
                           IniConfig.bN23UseLotInfoFile==true));
    tsRFMD->TabVisible=(IniConfig.bI41EnableEmptySocketCheck);                  // V912 cprod.cpp:2869（移植樹 cprod.cpp:3009 GATE S12-C1）
                                                                                //   ⚠ V912 main.cpp:16368 在 Qorvo GPIB Pause 時也會設 true（事件，移植樹沒有），這裡不重現
    if(TrayForm.bEnableAMR)                                                     // V912 cTrayAssignment.cpp:539-543（讀配方時）／:1399-1402（存配方時）
        tsKYEC_AMR->TabVisible=true;
    else
        tsKYEC_AMR->TabVisible=false;

    // (2) FormShow（V912 uLotInfo.cpp:318-1274）——只取 TabVisible 的指派，照 golden 順序
    ts_FTPAutomation->TabVisible=false;                                         // :325
    tsSigurd_CX->TabVisible =(CUSTOMER_CODE==CC_SIGURD_ChungXing);              // :326
    tsMurata->TabVisible    =(CUSTOMER_CODE==CC_Murata);                        // :327
    tsSPIL_SZ->TabVisible   =(CUSTOMER_CODE==CC_SPIL_CHINA_SUZHOU);             // :328
    tsOEE->TabVisible       =(CosFunction.bOEEFunction);                        // :329
    SettsChipAdvVisible();                                                      // :330（本檔 :1798，golden :1281-1301；也寫 tsChipAdv->Caption）
    ts_ATC6_1->TabVisible   =(Tri_Temp_Machine==1 || (ATC_SYSTEM==eNewATCSystem && ATC_InterfaceForm->iATC_MODE_TYPE==ATC_TYPE_61));   // :331（第二項在移植樹恆 false：shim 的 iATC_MODE_TYPE=0，見本檔 :3037-3058）
    tsPATSetUp->TabVisible  =(CUSTOMER_CODE==CC_PANTHER);                       // :332
    tsBundle->TabVisible    =(USE_COVER_TRAYID!=tCIDNotUse);                    // :333
    tsSetupFileCheck->TabVisible=(IniConfig.bEnableRmsCheckSetupFile==true);    // :334

    if(CUSTOMER_CODE==CC_MTI)                                                   // :366-371
    {
        tsDeviceInfo->TabVisible=false;
        tsFTP->TabVisible=false;
    }
    else if(IniConfig.bShowLotInfo)                                             // :372-380（:375 ResetLotInfo() 不在這裡做：本函式只管 TabVisible；FormShow 裡已照 golden 呼叫（GATE WC-1 20260927 退役））
    {
        tsDeviceInfo->TabVisible=(IniConfig.bEnableRms==true);
    }
    SetSelectionVisible();                                                      // :381（本檔 :1169，golden :1303-1383；只寫 tsSelection 與 Selection 分頁內的元件）

    if(CUSTOMER_CODE==CC_TSI)                                                   // :383-389（沒有 TabVisible）
    {
    }
    else if(CUSTOMER_CODE==CC_PANTHER)                                          // :390-400
    {
        tsLotID->TabVisible=IniConfig.bB12UsePATSetup;                          // :392
    }

    if(CUSTOMER_CODE==CC_JCET)                                                  // :403-406
        tsFTP->TabVisible=IniConfig.bEnableFTP;
    else
        tsFTP->TabVisible=(CosFunction.bFTPFunction);

    if(CUSTOMER_CODE==CC_PTI ||                                                 // :457-461
       CUSTOMER_CODE==CC_TFME_CHINA)
    {
        tsFTP->TabVisible=false;
    }

    if(CUSTOMER_CODE==CC_SCK)                                                   // :463-470
    {
        ts_AutoCleanMonitor->TabVisible=true;
    }
    else
    {
        ts_AutoCleanMonitor->TabVisible=false;
    }

    if(CUSTOMER_CODE==CC_AMKOR_Korea)                                           // :472-476（沒有 TabVisible）
    {
    }
    else if(CUSTOMER_CODE==CC_SCC ||                                            // :477-484（沒有 TabVisible；:482 那行 golden 自己註解掉了）
            CUSTOMER_CODE==CC_SCK)
    {
    }
    else if(IniConfig.bSPILFunction==true)                                      // :485-515
    {
        tsDeviceInfo->TabVisible    =true;                                      // :494
    }
    tsRTCFullViewImg->TabVisible=REAL_TIME_CCD;                                 // :549

    if(!TrayForm.bEnableAMR)                                                    // :551-552
        tsKYEC_AMR->TabVisible=false;

    SetATCFormVisible();                                                        // :569（本檔 :1468，golden :10119-10166；寫 tsATC 與 ATC 分頁的元件）

    tsBarCode->TabVisible=(BAR_CODE_INSTALL==ebctUseCCDMode || BAR_CODE_INSTALL==ebctInShtIntel || BAR_CODE_INSTALL==ebctEtherNetCCD);   // :571
    btChangeFile->Visible=(BAR_CODE_INSTALL==ebctInShtIntel || BAR_CODE_INSTALL==ebctEtherNetCCD || (BAR_CODE_INSTALL==ebctUseCCDMode && CosFunction.b2DUseSubJobFunction==true));   // :572

    tsESDMonitor->TabVisible=false;                                             // :574

    ts_OCRInterface->TabVisible=(INSTALL_OCR!=eocrUninstal && CosFunction.bTrayOCR==false);   // :576
    ts_SocketInterface->TabVisible=IniConfig.bSocketCommunication;              // :577

    tsBarCode->TabVisible=(BAR_CODE_INSTALL==ebctUseCCDMode || BAR_CODE_INSTALL==ebctInShtIntel || BAR_CODE_INSTALL==ebctEtherNetCCD);   // :590（golden 寫兩次，照留）

    if(IniConfig.bSIGURDFunction || CosFunction.bShowYieldMonitor)              // :618-628（:623 RefreshYieldMonitor() 不做 —— 只刷 Yield 分頁的客戶元件）
    {
        tsYieldMonitior->TabVisible=true;
        tsTPW->TabVisible   =(CUSTOMER_CODE==CC_TERAPOWER || CUSTOMER_CODE==CC_PTI);
        tsSigurd->TabVisible=(CUSTOMER_CODE==CC_SIGURD_PeiXing);
    }
    else
    {
        tsYieldMonitior->TabVisible=false;
    }

    tsMurata->TabVisible    =(CUSTOMER_CODE==CC_Murata);                        // :741（golden 寫兩次，照留）
    tsVTest->TabVisible     =(IniConfig.bVTESTFunction==true);                  // :746

    if(CUSTOMER_CODE==CC_GIGAS)                                                 // :1011-1036
    {
        tsLotID->TabVisible=false;                                              // :1035
    }
    tsASEMARMS->TabVisible=CosFunction.bUseARMSFunction;                        // :1045
    tsASECLEventLog->TabVisible=(CUSTOMER_CODE==CC_ASE_CL ||                    // :1046-1049
                                 CUSTOMER_CODE==CC_HANA_MICRON ||
                                 CUSTOMER_CODE==CC_SJ_Semiconductor ||
                                 CosFunction.bUseSocketContactCount);

    if(CosFunction.bEnableHandlerResultServer)                                  // :1054-1063（:1057-1058 RefreshAMR／ShowAMRCategoryBin 不做）
    {
        tsAMR->TabVisible=true;
    }
    else
    {
        tsAMR->TabVisible=false;
    }

    if(CosFunction.bFirstTrayCheckOnUnloader)                                   // :1251-1260
    {
        tsOtherTool->TabVisible=true;
    }
    else
    {
        tsOtherTool->TabVisible=false;
    }

    // (3) tmrChamberBoostTimer（V912 uLotInfo.cpp:11911-11984；FormShow :1197 開這個計時器 —— 移植樹 GATE WC-19 那一段）
    //     golden 計時器：bUseChamberBoostMode==false 就 return（:11919），所以那時的值是 V912 uTemp_Set.cpp:2469 寫的 false
    //     （建構子 :273 也是 false）；Boost 進行中（bStartChamberBoost）不改頁，其餘時候照 bUT150Install[tcChamber]（:11966-11976）。
    if(CosFunction.bUseChamberBoostMode==false)
    {
        tsChamberBoost->TabVisible=false;                                       // uTemp_Set.cpp:2469
    }
    else if(bStartChamberBoost==false)
    {
        if(tsChamberBoost->TabVisible && bUT150Install[tcChamber]==false)       // :11969-11972
        {
            tsChamberBoost->TabVisible=false;
        }
        else if(tsChamberBoost->TabVisible==false && bUT150Install[tcChamber])  // :11973-11976
        {
            tsChamberBoost->TabVisible=true;
        }
    }

    // (4) Timer2Timer（V912 uLotInfo.cpp:7051-7310）——同上，只取 TabVisible 的指派（:7055 InitialOK 見檔頭「穩態假設」）
    ts_AutoRetestMonitor->TabVisible=(CosFunction.bUseSCKART==false &&          // :7147-7149
                                      USE_AUTO_RETEST==eartInstall &&
                                      ((IniConfig.bA10_AutoReTest && (CosFunction.bAutoRetestGPIBmode==false || CUSTOMER_CODE==CC_KYEC_XILINX)) || bAutoReTest_ART));
    tsOCRBarCode        ->TabVisible=(INSTALL_OCR!=eocrUninstal && CosFunction.bTrayOCR);   // :7150

    if(ATC_SYSTEM==eWinWay)                                                     // :7152-7155
        ATC_WinWay->TabVisible=true;
    else
        ATC_WinWay->TabVisible=false;

    if(CUSTOMER_CODE==CC_GIGAS ||                                               // :7204-7211
       IniConfig.bVTESTFunction==true)
    {
    }
    else
    {
        tsFTP               ->TabVisible=IniConfig.bEnableFTP;
    }

    if(IniConfig.bVTESTFunction==true)                                          // :7213-7242
    {
    }
    else
    {
        tsDeviceInfo        ->TabVisible=IniConfig.bEnableRms;                  // :7241
    }
    tsTesterLog         ->TabVisible=(TestIF_File.iTestType==TCP_IP_MODE);      // :7243
    grpBarcodeDisplayLotInfo->Visible=(TestIF_File.i2DIDFormat==eAMD);          // :7244（FormShow :1188-1195 同一式）
    ts_FTPAutomation    ->TabVisible=IniConfig.bA32EnableFTPAutomation;         // :7247
}

// ---- FormShow 開窗時選中的分頁（回傳 dfm 元件名）--------------------------------------------------------
AnsiString TfLotInfo::W906_FormShowActivePage()
{
    AnsiString a="tsLotID";                                                     // :348
    if(CUSTOMER_CODE!=CC_MTI && IniConfig.bShowLotInfo &&                       // :372-379（:374 剛把 tsDeviceInfo 設成 bEnableRms）
       IniConfig.bEnableRms==true)
    {
        a="tsDeviceInfo";
    }
    if(CUSTOMER_CODE==CC_PANTHER &&                                             // :881-896（:748 起 else-if 鏈的 CC_PANTHER 那一支，:895）
       CUSTOMER_CODE!=CC_ASE_KaohSiung_K12 &&
       IniConfig.bVTESTFunction==false &&
       !(IniConfig.bSPILFunction==true && LastSet.iTester==_2D_SORT))
    {
        a="tsLotID";
    }
    if(IniConfig.bEnableFTP)                                                    // :1076-1098
    {
        if((CUSTOMER_CODE==CC_JSCC_OS))
            a="tsLotID";
        else
            a="tsFTP";
    }
    else if(IniConfig.bEnableRms==true)
        a="tsDeviceInfo";
    else if(CosFunction.bOEEFunction)
        a="tsLotID";
    else if(REAL_TIME_CCD)
        a="tsRTCFullViewImg";
    else if(ATC_SYSTEM==eWinWay && Temperature.bATCActiveCooling==true)
        a="ATC_WinWay";
    else if(ATC_SYSTEM!=eATCUninstall && ATC_SYSTEM!=eNonChamber)
        a="tsATC";
    else if(Tri_Temp_Machine==1)
        a="ts_ATC6_1";
    if(IniConfig.bA32EnableFTPAutomation)                                       // Timer2Timer :7247-7253（第一次 tick 就翻過去；bOldFTPAutomation 初值 false :7053）
        a="ts_FTPAutomation";
    return a;
}

// ---- ATC 分頁的顯示部分 ---------------------------------------------------------------------------------
//  每拍：先回 dfm 設計期值，再照 golden 順序套用（設定不變時＝golden 穩態）。
//  CH1..CH32 的溫度（ATCPtr[i]／ATCReferPtr[i] 的 Caption）、Chiller SV、露點、ATC 配方檔名、Chiller／7.0 燈
//  全都來自 ATC 連線（ATC_InterfaceForm->dTC[] 等；V912 ShowNewATCThermo :6608 起）或 ADAM 模組（TempCtrl/TriTemp.cpp:327），
//  移植樹沒有那條連線（acarry_shims.h:109-115 的 shim 只有 iATC_MODE_TYPE）—— 這裡不碰它們的 Caption，tag 也不送值。
void TfLotInfo::W906_ShowATCThermoDisplay()
{
    // (0) dfm 設計期值
    palATC->Caption="ATC 2.0 Monitor";                                          // dfm :4032
    aldATC7Status->Visible=true;                                                // dfm :2344（無 Visible=False）
    lblATC70->Visible=true;                                                     // dfm :2352
    aldATCChillerStatus->Visible=true;                                          // dfm :2296
    lblChiller->Visible=true;                                                   // dfm :2317
    pan_ATCChillerSV->Visible=true;                                             // dfm :2393
    pl_ATCChillerSV->Visible=true;                                              // dfm :2409
    lblATC_Now_RecipeFile->Visible=false;                                       // dfm :2330 Visible=False
    Pan_ATC_Use_4Head->Visible=true;                                            // dfm :2662（golden 沒有任何一行改它的 Visible）
    pl_ATC_Online->Caption="ATC On Line";                                       // dfm :2372
    pl_ATC_Online->Color=clLime;                                                // dfm :2373

    // (1) FormShow :569 → SetATCFormVisible()（golden :10119-10166）
    SetATCFormVisible();

    // (2) FormShow :669 → ShowATCTempPanel()（寫 Pan_ATC_Use_8Head／_32Head、ATCChPal[]／ATCPtr[]／ATCReferPtr[] 的 Visible）
    ShowATCTempPanel();

    // (3) FormShow :670-675
    //     ⚠ golden :672 還有 bStartATCRun=false —— 那是 ATC 運轉旗標（NetATCTimeTimer :8812 讀它決定要不要連 ATC），
    //       不是顯示；顯示層不寫它（移植樹 bStartATCRun 仍是 cmydef.cpp:4511 的初值 true，也只有 uTemp_Set.cpp:5417/:5428 會改）。
    if(Temperature.bATCActiveCooling==false || CUSTOMER_CODE==CC_KYEC_LEE)
    {
        pl_ATC_Online->Color=clRed;                                             // :673
        pl_ATC_Online->Caption="ATC Off Line";                                  // :674
    }

    // (4) FormShow :1157-1158
    pan_DewPoint->Visible=(DewPoint_Hardware_Install>0);
    pl_DewPoint->Visible=(DewPoint_Hardware_Install>0);

    // (5) ShowATCThermo（V912 :5574-5629；golden 由 TfTemperFrom::Timer1Timer 每拍無條件呼叫，V912 cTemperFrom.cpp:1689）
    //     只翻 :5595-5606 寫 palATCWorkingTemp 的那段；:5581-5593 dTempRange、:5608-5628 ShowATC70／20／NewATCThermo（告警）不做。
    static double OldSetATCTemp=-1;                                             // :5576
    double dSetATCTemp=0.0;
    if(LastSet.iTemperature==Tempture_Hot ||                                    // :5595-5599
       LastSet.iTemperature==Tempture_AmbientHot)
        dSetATCTemp=(double)Temperature.fWorkTemperBase;
    else
        dSetATCTemp=(double)IniConfig.dATCAmbientTemperature;

    if(OldSetATCTemp!=dSetATCTemp &&                                            // :5601-5606
       Temperature.bMultiZoneEnable==false)
    {
        OldSetATCTemp=dSetATCTemp;
        palATCWorkingTemp->Caption=dSetATCTemp;                                 // golden AnsiString(double)（vclcompat/AnsiString.h:77）
        W906_bATCWorkTempShown=true;
    }

    // (6) ShowNewATCThermo（V912 :6462-）寫 palATCWorkingTemp 的那段 :6592-6606。前置：:6482 ATC_SYSTEM==eNewATCSystem；
    //     :6512-6543 沒連線／沒啟動時只有 bATCActiveCooling==false 才 return（:6541），所以 bATCActiveCooling 為真就會走到迴圈。
    //     :6494-6510 第一次呼叫的 bInitialfinish return 只差一拍，不重現；迴圈裡 :6608 起讀 ATC 溫度與告警，不做。
    if(ATC_SYSTEM==eNewATCSystem && Temperature.bATCActiveCooling==true)
    {
        for(int i=0; i<iATC_Use_Heat_Count; i++)
        {
            if(TestIF.iTestMode==SingleSite &&                                  // :6594-6606
               Temperature.bMultiZoneEnable &&
               OldSetATCTemp!=0)
            {
                AnsiString str;
                str.sprintf("%1.1f_%1.1f_%1.1f_%1.1f",
                            Temperature.dZoneTempSetting[0],
                            Temperature.dZoneTempSetting[1],
                            Temperature.dZoneTempSetting[2],
                            Temperature.dZoneTempSetting[3]);
                palATCWorkingTemp->Caption=str.c_str();
                OldSetATCTemp=0;
                W906_bATCWorkTempShown=true;
            }
        }
    }

    // (7) NetATCTimeTimer（V912 :8725-；只在 eNewATCSystem 由 SetATCFormVisible :10159 開啟）:8812-8818 第一支：燈熄。
    //     其餘情況 aldATCPower 看 ATC_InterfaceForm->IsConnect()（:8851-8870）—— 移植樹沒有 ATC 連線，不可知。
    //     :8814-8816 的省電關溫那一項沒有翻（fMain->tPSM），所以那種情況這裡回報「不可知」，不是錯值。
    W906_bATCPowerKnown=(ATC_SYSTEM==eNewATCSystem &&
                         (bStartATCRun==false || Temperature.bATCActiveCooling==false));
    if(W906_bATCPowerKnown)
        aldATCPower->Value=false;
}

// ---- golden TfBarCode::DoBarcodeCount（V912 BarCode/BarCode.cpp:5863-5917）-----------------------------------
//  逐行照翻（寫 fLotInfo->sgBarcode）。唯一差別：golden :5907 寫的是全域 s2DIDYield（BarCode.h:917）；
//  移植樹那個全域在 BarCode/BarCode_Bottom2DID.cpp:58（ht9045_sm），ht9045_forms 不能往上連它 ——
//  這裡用同名區域變數，全域照舊由 SM 路徑（BarCode_Bottom2DID.cpp:409／BarCode_Shuttle2_CCDScan.cpp:430）寫。
bool W906_DoBarcodeCount()
{
    bool bNeedAlarm=false;
    int  Count1=0;
    int  PassCount1=0;
    int  FailCount1=0;
    double  rate1=0.0, rate=0.0;
    AnsiString str="";
    int  AutoRetry1=0;
    int  Duplicate=0;
    AnsiString s2DIDYield;                                                      // 見上

    if(fLotInfo==0 || fLotInfo->sgBarcode==0)
        return false;

    for(int i=0; i<4; i++)
    {
        fLotInfo->sgBarcode->Cells[1+i][1]=iNeedBarcodeCount[i];
        fLotInfo->sgBarcode->Cells[1+i][2]=iBarcodePassCount[i];
        fLotInfo->sgBarcode->Cells[1+i][3]=iBarcodeErrorCount[i];
        fLotInfo->sgBarcode->Cells[1+i][5]=iBarcodeAutoRetry[i];
        fLotInfo->sgBarcode->Cells[1+i][6]=iBarcodeDuplicate[i];
        Count1+=iNeedBarcodeCount[i];
        PassCount1+=iBarcodePassCount[i];
        FailCount1+=iBarcodeErrorCount[i];
        AutoRetry1+=iBarcodeAutoRetry[i];
        Duplicate+=iBarcodeDuplicate[i];

        if(iNeedBarcodeCount[i]!=0)
        {
            rate=double(ChangeToFloatNonPcnt((double)(iBarcodePassCount[i]*100.0), (double)(iNeedBarcodeCount[i])));
        }
        else
        {
            rate=0;
        }

        str.sprintf("%2.2f", rate);
        fLotInfo->sgBarcode->Cells[1+i][4]=str.c_str();

        if(Count1!=0)
        {
            rate1=double(PassCount1*100.0/Count1);
        }
        else
        {
            rate1=0;
        }
        s2DIDYield.sprintf("%2.2f", rate1);

        fLotInfo->sgBarcode->Cells[5][1]=Count1;
        fLotInfo->sgBarcode->Cells[5][2]=PassCount1;
        fLotInfo->sgBarcode->Cells[5][3]=FailCount1;
        fLotInfo->sgBarcode->Cells[5][4]=s2DIDYield.c_str();
        fLotInfo->sgBarcode->Cells[5][5]=AutoRetry1;
        fLotInfo->sgBarcode->Cells[5][6]=Duplicate;
    }

    if(TestIF_File.b2DIDYield)                                                  //Steven 20171222 (Wei) : Yield Alarm of 2DID
    {
        if(Count1>TestIF_File.i2DYieldIgnoreCnt && rate1<TestIF_File.d2DIDYield)//JerryYang 20241104 : Ignore count變更為可以修改
            bNeedAlarm=true;
    }
    return bNeedAlarm;
}

// ---- btClearBarcodeCountClick（V912 uLotInfo.cpp:10168-10184）逐行照翻 --------------------------------------
//  寫入：D:\HT9045_Log\2DBarCode\YYYY_MM_DD\ 目錄（MyForceDirectories，common.cpp 真本體）。
//  ⚠ :10176 SGDToXLS 在移植樹是 no-op（SgdToXLS.cpp GATE (1)：golden 的 BIFF5 寫檔器 XLSFile.pas 是 Object Pascal，沒有移植），
//    所以 golden 會留下的「清除前計數」.xls 在這裡**不會產生**，清掉就沒了。網頁那一端（WebLotInfo.cpp）因此多一道確認，
//    並在回應裡明講；本函式本身不加任何 golden 沒有的判斷。
//  :10183 fBarCode->DoBarcodeCount() → W906_DoBarcodeCount()（同一個 golden 本體，見上）。
void TfLotInfo::btClearBarcodeCountClick()
{
    AnsiString FolderName;
    AnsiString FileName;

    FolderName.sprintf("D:\\HT9045_Log\\2DBarCode\\%04d_%02d_%02d\\", SystemYear, SystemMonth, SystemDate);
    MyForceDirectories(FolderName);
    FileName.sprintf("%04d-%02d-%02d %02d_%02d_%02d.xls", SystemYear, SystemMonth, SystemDate, SystemHour, SystemMin, SystemSec);
    SGDToXLS(sgBarcode, AnsiString(FolderName+FileName));

    ZeroMemory(iNeedBarcodeCount, sizeof(iNeedBarcodeCount));
    ZeroMemory(iBarcodeDuplicate, sizeof(iBarcodeDuplicate));
    ZeroMemory(iBarcodeErrorCount, sizeof(iBarcodeErrorCount));
    ZeroMemory(iBarcodePassCount, sizeof(iBarcodePassCount));
    ZeroMemory(iBarcodeAutoRetry, sizeof(iBarcodeAutoRetry));
    W906_DoBarcodeCount();
}

// ---- Selection 分頁（tsSelection，V912 uLotInfo.dfm:4299-4575）-----------------------------------------------------
//  讀：golden pgLotinfoChange（:7340-7385；本檔 :2052 已翻）—— 切到 tsSelection 時用 CheckAndReadIniData 讀
//      config\Security_new.def [Network]（缺鍵會回寫預設值，golden 本來就這樣；Steven 裁決 #30 同型：接受）。
//  寫：golden btnSaveClick（:7387-7415，下面逐行翻）—— WriteIniData 寫同一個檔 17（＋VTEST 2）個鍵。
//  ⚠ golden 的怪處照留：讀用 "Index Heat Mode"（:7371），存卻寫 "Heat Mode"（:7407）—— 存了下次讀不到；
//    "ART_RT_Count "（:7369，尾巴有空白）只讀不存；chkPositionOffset（dfm :4520）golden 從不讀也不存。
void TfLotInfo::W906_SelectionDfmDefaults()
{
    // (0) dfm 設計期值：groupbDownloadItem 的子元件（dfm :4355-4528）只有 lblDownloadAccessWarning 是 Visible=False；
    //     grpMesCheck（:4537）Visible=False，裡面兩個勾選框本身可見。沒有任何 Enabled=False。
    TCheckBox* const kDownload[] = { chkTempOffset, chkContactHigh, chkContactForce, chkContactMode, chkHotPlate, chkLoadUnload,
                                     chkSpeedSetting, chkShuttleMode, chkTestMode, chkBinasgn, chkBinasgnOff, checkbAutoClean,
                                     chkIndexHeatingMode, chkART, chkART_RTCount, cbBottom2DOffset, chkCleanCount, chkAutoCleanContactHeight };
    const int nDownload=(int)(sizeof(kDownload)/sizeof(kDownload[0]));
    for(int i=0; i<nDownload; i++)
    {
        kDownload[i]->Visible=true;
        kDownload[i]->Enabled=true;
    }
    chkStopYield->Visible=true;          chkStopYield->Enabled=true;
    chkConsecutiveFailure->Visible=true; chkConsecutiveFailure->Enabled=true;
    btnSave->Visible=true;               btnSave->Enabled=true;
    groupbDownloadItem->Visible=true;
    grpMesCheck->Visible=false;
    lblDownloadAccessWarning->Visible=false;

    // (1) FormShow :631
    chkTestMode->Visible=CosFunction.bLastSetInSetUpFile;
    // (2) FormShow :381 → SetSelectionVisible()（本檔 :1169；寫 tsSelection、VTEST 時的 groupbDownloadItem／grpMesCheck、lblDownloadAccessWarning）
    SetSelectionVisible();
    // (3) SetSelectionVisible :1369-1378 的迴圈 —— 移植樹 :1221-1226 是 GATE WA-4（vclcompat::TGroupBox 沒有 Controls[]）。
    //     golden 逐一走 groupbDownloadItem 的子元件、跳過 lblDownloadAccessWarning；子元件照 dfm :4355-4528 就是上面 18 個勾選框＋btnSave
    //     ＋chkPositionOffset（移植樹沒有這個成員，golden 也從不讀寫它的值）。
    if(IniConfig.bA75DownloadItemByAccessLevel==true)
    {
        bool bDenyByOP=(AccessLevel==0);
        for(int i=0; i<nDownload; i++)
            kDownload[i]->Enabled=!bDenyByOP;
        btnSave->Enabled=!bDenyByOP;
    }
    // (4) pgLotinfoChange :7364-7365／:7373-7379 寫的 Visible／Enabled 由 pgLotinfoChange 自己做（切頁時），這裡不重複。
}

// ---- btnSaveClick（V912 uLotInfo.cpp:7387-7415）逐行照翻 ------------------------------------------------------------
//  寫入：AuthPath+"Security_new.def"（D:\HT9045\config\Security_new.def）[Network]。
void TfLotInfo::btnSaveClick()
{
    AnsiString sConfigPath=AuthPath+"Security_new.def";
    WriteIniData(sConfigPath, "Network", "Temp Offset",     chkTempOffset->Checked);
    WriteIniData(sConfigPath, "Network", "Contact High",    chkContactHigh->Checked);
    WriteIniData(sConfigPath, "Network", "Contact Force",   chkContactForce->Checked);
    WriteIniData(sConfigPath, "Network", "Contact Mode",    chkContactMode->Checked);
    WriteIniData(sConfigPath, "Network", "HotPlate",        chkHotPlate->Checked);
    WriteIniData(sConfigPath, "Network", "Load Unload",     chkLoadUnload->Checked);
    WriteIniData(sConfigPath, "Network", "Speed Setting",   chkSpeedSetting->Checked);
    WriteIniData(sConfigPath, "Network", "Shuttle Mode",    chkShuttleMode->Checked);
    WriteIniData(sConfigPath, "Network", "Test Mode",       chkTestMode->Checked);
    WriteIniData(sConfigPath, "Network", "Binasgn",         chkBinasgn->Checked);
    WriteIniData(sConfigPath, "Network", "BinasgnOff",      chkBinasgnOff->Checked);
    WriteIniData(sConfigPath, "Network", "Auto Clean",      checkbAutoClean->Checked);                                  //jou 20161122 Auto Clean 參數可以選擇是否需要上傳下載
    WriteIniData(sConfigPath, "Network", "Bottom 2D Offset",cbBottom2DOffset->Checked);                                 //JerryYang 20201122 Bottom 2D offset不覆蓋
    WriteIniData(sConfigPath, "Network", "Cleaning Count",  chkCleanCount->Checked);                                    //KenHsieh 20230518 : Auto Clean count不覆蓋
    WriteIniData(sConfigPath, "Network", "Auto Clean Contact Height", chkAutoCleanContactHeight->Checked);              //JerryYang 20241019 : 矽品二林 耀仁要求Auto clean高度可選擇不覆蓋
    if(CosFunction.bUseSCKART)
        WriteIniData(sConfigPath, "Network", "Auto Retest", chkART->Checked);   //Steven 20190918 : ART設定下載不覆蓋
    WriteIniData(sConfigPath, "Network", "Heat Mode",       chkIndexHeatingMode->Checked);                              //Steven 20180420 (Jou) : JCET吳如春說不覆蓋Index加熱模式

    if(IniConfig.bEnableRms==true)
        pgLotinfo->ActivePage=tsDeviceInfo;

    if(IniConfig.bVTESTFunction==true)
    {
        WriteIniData(sConfigPath, "Network", "Stop Yield",      chkStopYield->Checked);
        WriteIniData(sConfigPath, "Network", "Consecutive Failure",      chkConsecutiveFailure->Checked);
    }
}

// =============================================================================
//  AI(W906-FRW-S94) 20260926（Steven 團隊）：BarCode「Clear List」與 OCRBarCode「Clean List」—— RULINGS_20260926 S94
//  （依批號的三份清單 LotData.txt／TrayIDByLot.txt／OCRLot.txt 的清除鈕；開機讀回在 FileRW/MainBoot.cpp W906_FRWBoot_LotListsRead）。
//  golden 一律 V912 主 repo（D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy，cp950）。
// =============================================================================

// ---- btClearBarcodeListClick（V912 uLotInfo.cpp:10186-10195）—— 安裝座 ------------------------------------------------
//  golden 本體逐行翻在 WebLotInfo.cpp 的 ClearBarcodeListBody；為什麼不寫在這裡見 forms/fLotInfo.h 檔尾的安裝座註解。
//  golden 的呼叫者全是 `fLotInfo->btClearBarcodeList->Click()`（golden 全樹 18 處，20260926 19:23 grep *.cpp/*.h：main.cpp:15892／:15997／:16084、
//  Command.cpp:13725／:13781／:13815、csystem.cpp:6532／:11275／:15041／:15126／:15264／:15269、uLotInfo.cpp:1906／:8354、
//  uHGemHT9045.cpp:3794、uRENESAS_Server.cpp:783／:894、BarcodeXML.cpp:675；另外就是按鈕本身）。移植樹那些呼叫點**不受本次影響**：
//  它們呼叫的 vclcompat TControl::Click() 仍是 no-op（vclcompat/Controls.h），或是 csystem.cpp 的 W7C2_FLOTINFO_CLEARBARCODE 空巨集。
//  要接上的呼叫點改呼叫本函式（網頁 lotinfo.op barcode.clearList、Steven02 的 TesterComm/Handler/HandlerGpibMsg.cpp G1／G5）。
W906_ClearBarcodeListBodyFn W906_ClearBarcodeListBody = 0;
int W906_ClearBarcodeListCallCount = 0;
void TfLotInfo::btClearBarcodeListClick()
{
    W906_ClearBarcodeListCallCount++;
    if (W906_ClearBarcodeListBody != 0) W906_ClearBarcodeListBody(this);
}

// ---- spOCRCleanListClick（V912 uLotInfo.cpp:14461-14466）逐行照翻 -----------------------------------------------------
//  寫入：asOCRLotPath ＝ D:\HT9045_Log\OCR\OCRLot.txt（common.cpp:331）—— 整檔寫成空的（0 byte）。
//  ⚠ :14463／:14464 的 fOCR->ListOCRByLot：移植樹的 fOCR（forms/fOCR.h:140 TfOCR）沒有這個成員；golden 那張清單在移植樹是
//    OCRInsp.cpp 檔內的 seam（OCRInsp.cpp:341，`static W906OCR_TfOCRSeam W906OCR_fOCRObj`，別的檔碰不到）。這裡用一張本地的
//    空清單代替。可觀察的結果相同：golden 那張清單唯一的讀者 CheckDataExistAndDuplicate（V912 OCRInsp.cpp:614-617）每次都先
//    Clear()＋LoadFromFile()（讀的是 asOCRDownLoadLotPath 底下另一個檔），所以記憶體那張清不清看不出差別；看得出來的只有
//    :14464 把 OCRLot.txt 寫成空檔 —— 這一行照做。（OCRLot.txt 在 golden 全樹只有寫者 OCRInsp.cpp:1135，沒有讀者。）
//  ⚠ 資料夾不在時：golden VCL SaveToFile 丟 EFCreateError（事件處理中斷、跳錯誤框）；vclcompat 靜靜不寫（vclcompat/TStringList.cpp:262-264）。
//  golden 另一個呼叫者 csystem.cpp:12218 `fLotInfo->spOCRCleanList->Click()`（移植樹 csystem.cpp:8705）仍是 vclcompat no-op，不受影響。
void TfLotInfo::spOCRCleanListClick()
{
    TStringList ListOCRByLot;                                                   // 代替 fOCR->ListOCRByLot（見上）
    ListOCRByLot.Clear();                                                       // golden :14463
    ListOCRByLot.SaveToFile(asOCRLotPath);                                      // golden :14464
    spOCRCleanList->Down=false;                                                 // golden :14465
}

// ---- 兩顆清除鈕的 Visible（網頁 lotinfo.op 用來判斷 golden 操作員按不按得到）------------------------------------------
//  dfm（V912 uLotInfo.dfm:4632 btClearBarcodeList／:5721 spOCRCleanList）都沒寫 Visible ＝ True；golden 執行期只有 Timer2Timer
//  （V912 :7051 起；本檔 :4142 已翻，T3／T16）寫它們。這裡只取那兩個指派，一行不改；Timer2Timer 的其他副作用不做
//  （理由同本檔 Steven 20260925 段落檔頭「為什麼不直接呼叫 FormShow()／Timer2Timer()」）。
void TfLotInfo::W906_ClearListButtonsVisible()
{
    btClearBarcodeList->Visible=true;                                           // dfm :4632
    spOCRCleanList->Visible=true;                                               // dfm :5721
    if(InitialOK==false)                                                        //Steven 20160912 : Add InitialOK in Timer   // golden :7055
        return;
    if(CUSTOMER_CODE==CC_KYEC_XILINX)                                           //Frank 20171030 (Steven) add Clear Barcode List新增權限 Xilinx   // golden :7064-7067
    {
        btClearBarcodeList->Visible=(AccessLevel>=iDefHonPrecLevel)?true:false;
    }
    spOCRCleanList->Visible=(INSTALL_OCR!=eocrUninstal && CosFunction.bTrayOCR && IniConfig.bCompareOCRData);           //KenHsieh 20220825 : 新增OCR比對功能   // golden :7285
}

// =============================================================================
//  AI(W906-PROD-S117) 20260926（Steven 團隊，St01）：Lot End —— RULINGS_20260926 S117／S120-4（Steven「現在排」）。
//  golden 一律 V912 主 repo（D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy，cp950）uLotInfo.cpp：
//    sbSECSLotEndClick :1387-1444、SetLotEnd :2014-2379。SetLotComponents（:2381-2421）／SetLotID（:1535-）／ReadWriteLotInfo
//    本檔前段早就翻好（W906-LOT-W1），這裡照 golden 呼叫，不改它們。
//
//  逐段分類（詳細表在交件報告第 1 節）
//    (1) 讀寫檔／Lot Info log／End Time／config.ini —— St01，照翻、活的。
//    (2) 動到 RunInfo.bLotStart、主畫面按鈕 Enabled、START 前置條件 ——
//        * SetLotComponents(true)（:2344）活的：它只把 RunInfo.bLotStart 設 false＋表單元件 Enabled／Down。證據（20260926 21:3x
//          git grep "RunInfo.bLotStart" 移植樹非 tests／docs，再用前置處理器規則濾掉 #if 0 裡的）：活的讀者全是「拒絕／鎖／紀錄」——
//          WebStart.cpp :1979／:2138／:2152／:2178／:2273／:2341／:2364＝false 時不准 START；cSensorScan.cpp:355 KYEC AMR 自動 START 要 true；
//          ATP 鎖參數六處（cSpeed.cpp:403、FileRW/HotPlateForm_File.cpp:112、fMain_OperateMode.cpp:500、fTrayAssignment.cpp:890、
//          uTemp_Set.cpp:953／:1276）＝false 時解鎖編輯（golden 的原意）；本檔 :2442 btnSaveDataClick（false 才准存 summary）、:4409（SPIL
//          2DID 白名單時 Run Mode 可改）、:5595／:5605 SetLotStart 的 SECS 去重；fNote_JamCount.cpp:86（VTEST Jam 計數）；HANA_ART.cpp:580
//          （Hana ART 沒開批就自動 SetLotStart）；Command.cpp:13670-17863／uHGemHT9045.cpp:5808／:7862／WebRecipeChange.cpp:415 的遠端回覆與
//          SCC 換配方判斷。沒有任何一處因為它變 false 而讓馬達或氣缸動。另外移植樹早就有兩處活的 `RunInfo.bLotStart=false`
//          （csystem.cpp:3612 DoCleanOutFinishCheck、:10483 DoInitialStart；:7963 那一處在 GATE G4-3 的 #if 0 裡，不算）。
//        * 主畫面五個模式鈕 Enabled=true（:1434-1438）閘住：GATE (W906-PROD-S117-MAINPANELS)。它跟 SetLotStart 那一半
//          （本檔 GATE (W906-LOT-W1-MAINPANELS)，Lot Start 時鎖住）是同一對，要一起由 Jimmy（OPMODE）決定；只開「解鎖」這一半沒有意義。
//        * 拒絕路徑的 SetRunStartMode（:1404、:1412）閘住：GATE (W906-PROD-S117-RSM)。它會重跑 RunStartMode.cpp 的模式切換
//          （例：ASM 模式時 ReStartAutoSiteMapping），屬 Jimmy。
//    (3) SECS／GPIB／Tester／客戶碼 —— EventReport(DoLotEnd) 活的（移植樹 EventReport 是 SecsEventReport.cpp 的模擬計數器，
//        本檔 SetLotStart :5606 的 DoLotStart 也是活的；兩邊對稱）；其餘（ATK AMR、TCP OS Tester、UTAC SetLotState、
//        各客戶碼分支）閘住，照 RULINGS_20260925 S25「先暫時跳過，註記就好」。（例外：CYUEAN 那段 golden :2327-2334 已於 20260928 依 Steven W22 打開，見 SetLotEnd 內 AI(W906-ELA-N34)。）
//        原則：S25 閘的是客戶專屬的「動作」；客戶專屬的「拒絕條件」（sbSECSLotEndClick :1420-1424 OEE）照 golden 保留，閘掉等於放寬。
//
//  ELTodo：這個 TU 在 ht9045_forms，不連 FileRW（filerw::ELTodo 只在 wb_serve）。每個閘在「golden 這一刻會執行」時把一行說明
//    放進 W906_LotEndSkipped；WebLotInfo.cpp 的 lotinfo.op lotEnd 執行完把它們轉成 ELTodo 並放進回應。
//
//  ⚠ 會寫真實檔（golden 就是這樣；網頁 lotEnd 要兩段式確認）：
//    D:\HT9045_Log\LotInfo\<年>\LotInfo_<年>.csv          （:2038-2039，TByYear，新檔先寫表頭）
//    AuthPath+"config.ini" [Lot Info]                       （:2345 SetLotID("")、:2376 ReadWriteLotInfo(false)）
//    D:\HT9045\system\ArmByLot0..2.dat 與 _backup.dat      （:2376 → ArmDataLot[i]->WriteFile()，清成 0 之後；
//                                                             TestIF_File.bLowYieldAlarmByBin 時另寫 ArmByLot*.ini）
// =============================================================================
extern int iTestHeadMotorTask;                                                  // golden :1385（atester.cpp:5807）

// ---- golden TfMain::slLotInfolog（V912 main.h:1496；main.cpp:1665-1668）---------------------------------------------------
//  移植樹 TfMain 沒有這個成員，forms/fMain.* 又是共用檔 ⇒ 物件放這裡。golden 全樹只有 SetLotEnd :2038-2039 用它（20260926 grep
//  audit_prod\golden_utf8 *.cpp／*.h：main.cpp:1665／:1668 建構、main.h:1496 宣告、uLotInfo.cpp:2038／:2039 使用，共 5 筆）。
//  golden 在 TfMain 建構子就 new；這裡第一次用到才 new —— TMyStringList 的建構子不碰檔案（Public/MyStringList.cpp:274），
//  MaxLineCount=1（AddText 當下就寫檔），所以兩者看不出差別。
static TMyStringList *W906_slLotInfolog()
{
    static TMyStringList *p=0;
    if(p==0)
    {
        p=new TMyStringList("D:\\HT9045_Log\\LotInfo",                         //Steven 20221225 : add for CyuEan   // golden main.cpp:1665
                            "LotInfo",
                            "Start Time, End Time, Tester OS Ver, Tester ID, Operator, Customer, Test Program, Device Name, Lot No, Sub Lot No, Mode Code, Test Code, Test Bin No, Machine ID, Stage, Step");
        p->SaveType=TByYear;                                                    // golden main.cpp:1668
    }
    return p;
}

AnsiString W906_LotInfoLogFileName()
{
    return W906_slLotInfolog()->sLastFileName;
}
#include "Interface/InterfaceSYS.h"   // AI(W906-ELA-N34) 20260928：SendCommand_EventLog（:359）／EL_UPLOAD_CHIPADV_LOTEND（:240），SetLotEnd 的 CYUEAN N34 段（golden :2327-2334）；佔用原本的空行，不移動本檔其後的行號
// ---- SetLotEnd（V912 uLotInfo.cpp:2014-2379）逐行照翻 -----------------------------------------------------------------
void TfLotInfo::SetLotEnd(AnsiString sFunc)                                     //Steven 20250515 : 整合Lot測試報表 Lot End
{
    W906_SetLotEndCount++;                                                      // 移植樹：網頁判斷「真的執行了」用
    AnsiString sPath=AuthPath, str, Msg;
    TStringList *SL=new TStringList();
    sPath=AuthPath+"config.ini";                                                // golden :2018（本函式之後沒有用到 sPath，照留）
    dtEndLot=Now();
    str.sprintf("%04d%02d%02d_%02d%02d%02d", SystemYear, SystemMonth, SystemDate, SystemHour, SystemMin, SystemSec);
    lbledtEndTime->Text=str;
    SL->Add(lbledtStarTime      ->Text);
    SL->Add(lbledtEndTime       ->Text);
    SL->Add(lbledtTesterOsVer   ->Text);
    SL->Add(lbledtTesterID      ->Text);
    SL->Add(edtSysOperatorID    ->Text);
    SL->Add(lbledtCustomer      ->Text);
    SL->Add(lbledtTestProg      ->Text);
    SL->Add(lbledtDeviceName    ->Text);
    SL->Add(edtSysLotID         ->Text);
    SL->Add(lbledtSubLotNo      ->Text);
    SL->Add(lbledtModeCode      ->Text);
    SL->Add(lbledtTestCode      ->Text);
    SL->Add(lbledtTestBinNo     ->Text);
    SL->Add(lbledtMachineID     ->Text);
    SL->Add(edtStage            ->Text);
    SL->Add(edtStep             ->Text);
    W906_slLotInfolog()->AddText(SL->CommaText);                                // golden :2038 fMain->slLotInfolog->AddText（物件位置見上）
    W906_slLotInfolog()->MySaveToFile();                                        // golden :2039
    RunInfo.LotEndTime.sprintf("%04d-%02d-%02d %02d:%02d:%02d", SystemYear, SystemMonth, SystemDate, SystemHour, SystemMin, SystemSec);
    Msg.sprintf("Lot End, Lot ID:%s, OP ID:%s, Run Mode:%s", edtSysLotID->Text, edtSysOperatorID->Text, cbRunMode->Text);                                       //Steven 20200302 : Lot Start/end增加log
    RecordProcess(Msg, sFunc);

    //AI(W906-PROD-S117) 20260926: GATE (W906-PROD-S117-S25-KYEC-PANTHER) —— golden :2044-2057，客戶專屬（S25）。
    //  KYEC-Lee 的 SLT contact log（FormHS）、Panther 的 PAT 報表／機台時間紀錄。另外 FormHS 在移植樹是 SCK_ART_Remainder 的替身、
    //  fMain 沒有 patFunc／machineTime（forms/fLotInfo.h:1137 列為未移植）。
#if 0 // GATE (W906-PROD-S117-S25-KYEC-PANTHER)
    if(CUSTOMER_CODE==CC_KYEC_LEE)
    {
        asSLT_LotEndTime=GetDateInfoByString("/")+" "+GetOnlyTimeInfoByString(":");
        FormHS->RecordContact_SLTLog_HS();
    }
    else if(CUSTOMER_CODE==CC_PANTHER)
    {
        fMain->patFunc->SetEndLotTime(dtEndLot);
        UpdateLotInfoPAT();
        fMain->patFunc->GenerateEndLotReport();
        fMain->patFunc->JamLogs.clear();
        fStartCondition->sbClearCountClick(fStartCondition);
        fMain->machineTime.EndLot();                                            //Jimmychiu 20250916 : 新增機台運作狀態紀錄
    }
#endif // GATE (W906-PROD-S117-S25-KYEC-PANTHER)
    if(CUSTOMER_CODE==CC_KYEC_LEE || CUSTOMER_CODE==CC_PANTHER)
        W906_LotEndSkipped->Add("golden uLotInfo.cpp:2044-2057 KYEC_LEE SLT log / PANTHER PAT end-lot report (S25 customer-specific, gated)");

    if(IniConfig.bEnable_SECS_GEM==true &&
        RunInfo.bLotStart==true)                                                //Steven 20140528 : Secs Gem  //wei 20150821 不能連續送
    {
        //AI(W906-PROD-S117) 20260926: GATE (W906-PROD-S117-ATKAMR) —— golden :2062／:2064-2065／:2068-2070，ATK AMR（S25）＋缺相依：
        //  移植樹 TfAGV（forms/fAGV.h）沒有 bATK_AMR_DoLotEndSent／bATK_AMR_DoHostLotStart／bATKAMR_GET_LOTORDER0_Ready，
        //  全樹也沒有 sTrackOutType_ATK（csystem.cpp:10409 GATE(g3-G03) 同一族）。閘掉判斷、走「沒有提前送過」那一支：
        //  非 ATK 機台 golden 的 bATK_AMR_DoLotEndSent 恆為 false，所以 EventReport 照送 —— 與 golden 相同。
#if 0 // GATE (W906-PROD-S117-ATKAMR)
        if(fAGV->bATK_AMR_DoLotEndSent==false)                                  //AI(general) 20260402 (RogerYang) : ATK AMR 已提前送過則跳過
#endif // GATE (W906-PROD-S117-ATKAMR)
        {
#if 0 // GATE (W906-PROD-S117-ATKAMR)
            if(fAGV->IsATK_AMR()==true)                                         //AI(ht9045-atk-amr-flow) 20260820 (RogerYang) : 標準路徑補送時亦填 SV38316
                sTrackOutType_ATK="Final Track-Out";
#endif // GATE (W906-PROD-S117-ATKAMR)
            EventReport(SECS_EVENT.DoLotEnd);                                   // 移植樹 EventReport ＝ SECSGEM/SecsEventReport.cpp 模擬計數器（同本檔 SetLotStart :5606）
        }
#if 0 // GATE (W906-PROD-S117-ATKAMR)
        fAGV->bATK_AMR_DoLotEndSent=false;                                      //AI(general) 20260402 (RogerYang) : 不論有無送出都重設旗標
        fAGV->bATK_AMR_DoHostLotStart=false;                                    //put here
        fAGV->bATKAMR_GET_LOTORDER0_Ready=false;
#endif // GATE (W906-PROD-S117-ATKAMR)
        if(fAGV->IsATK_AMR()==true)
            W906_LotEndSkipped->Add("golden uLotInfo.cpp:2062-2070 ATK AMR flags / sTrackOutType_ATK (S25 + members absent on TfAGV, gated; EventReport(DoLotEnd) still sent) -- Steven02");
    }

    //AI(W906-PROD-S117) 20260926: GATE (W906-PROD-S117-TESTERTCP) —— golden :2073-2076，Tester 通訊（Steven02）＋缺相依：
    //  移植樹沒有 fTesterTCP 全域（同本檔 SetLotStart 的 GATE (W906-LOT-W1-TESTERTCP)）。
    //  ⚠ 行為：TestIF.iTestType==TCP_IP_MODE 的 OS Tester，Lot End 不會送 "LOTEND" 給測試機。
#if 0 // GATE (W906-PROD-S117-TESTERTCP)
    if(TestIF.iTestType==TCP_IP_MODE)                                           //Steven 20230213 : For SJSemi OS Tester
    {
        fTesterTCP->SendTCPIPCommand(0, "LOTEND", "LOTEND");
    }
#endif // GATE (W906-PROD-S117-TESTERTCP)
    if(TestIF.iTestType==TCP_IP_MODE)
        W906_LotEndSkipped->Add("golden uLotInfo.cpp:2073-2076 fTesterTCP->SendTCPIPCommand(0, \"LOTEND\", \"LOTEND\") not sent (fTesterTCP absent, gated) -- Steven02");

    //AI(W906-PROD-S117) 20260926: GATE (W906-PROD-S117-S25-PTI) —— golden :2078-2200，力成（CC_PTI）專屬（S25）：
    //  JamAlarmLogTxt\<LotID> 整個資料夾上傳 FTP（背景執行緒 FtpUploadThd 或同步 fFTPClient）、Input 數量 log、
    //  SocketIdProductData csv＋FTP。另外 fMain 沒有 slInputDataLog／slSocketIdProductData，全樹沒有 FtpUploadThd。
#if 0 // GATE (W906-PROD-S117-S25-PTI)
    if(CUSTOMER_CODE==CC_PTI &&                                                 //Sam 20170502 (wei) 力成按下 "Lot End" 後即 JamAlarmLogTxt LotId 資料夾所有資料上傳至 FTP
       IniConfig.bEnableFTP==true &&
       RunInfo.bLotStart==true)
    {
        TStringList *slInputQuantity;                                           //Sam 20171208 (wei) : PTI Lot End 上報 Input 數量
        slInputQuantity=new TStringList();
        slInputQuantity->Add(IntToStr(LastSet.SendCT[0]));
        slInputQuantity->Add(fLotInfo->edtSysLotID->Text);
        slInputQuantity->Add(fLotInfo->edtSysOperatorID->Text);
        slInputQuantity->Add(fMain->cbSetupFileName->Text);
        slInputQuantity->Add(fLotInfo->cbRunMode->Text);
        fMain->slInputDataLog->AddTextWithDateTime(slInputQuantity->CommaText);
        fMain->slInputDataLog->MySaveFileByFileNameAndType(IniConfig.asN06_FileName, fLotInfo->edtSysLotID->Text, "INPUT");
        slInputQuantity->Clear();
        delete slInputQuantity;
//        fSortCT->btnClearCountClick(fSortCT);                                 //AI(ht9045-v912) 20260923: CASE-PTI-20260923-001 註解掉力成 Lot End 自動清 Sort Count，還原 V899.37 行為；程式呼叫已不跳詢問，Tray Feed 完成後的自動 Lot End 會靜默把面板數量清 0

        AnsiString asJamCodeFilePath, asJamCodeFileName;                        //RogerYang 20170508 (wei) 拿掉static
        asJamCodeFilePath.sprintf("D:\\HT9045_Log\\JamAlarmLogTxt\\%s", edtSysLotID->Text);
        TStringList *lstFiles = new TStringList;
        TSearchRec sr;
        if(FindFirst(asJamCodeFilePath + "\\*.*", faAnyFile, sr)==0)
        {
            do                                                                  //尋找資料夾內所有檔案
            {
                lstFiles->Add(sr.Name);
            }while(FindNext(sr)==0);
            FindClose(sr);

            if(IniConfig.bFtpUploadBackground==true)                           //AI(ht9045-v899) 20260612(CASE-20260611-001): 開關開(PTI預設) → 走背景FTP上傳執行緒,fire-and-forget;丟完即返回,不MySleep/不等/不break,避免Lot End主執行緒被同步上傳阻塞造成卡站或alarm無法消除
            {
                if(FtpUploadThd!=NULL)                                          //AI(ht9045-v899) 20260612(CASE-20260611-001): 保險,背景執行緒未建立則略過(不退回同步,避免同一批檔重複上傳)
                {
                    AnsiString asBgHost = IniConfig.FtpHost;                    //AI(ht9045-v899) 20260612(CASE-20260611-001): 迴圈外取一次連線參數(值拷貝),對應UploadFileToServer2非Sigurd分支(FtpHost/FtpUserName/FtpPassword/N06_FtpPort)
                    AnsiString asBgUser = IniConfig.FtpUserName;
                    AnsiString asBgPwd  = IniConfig.FtpPassword;
                    int        iBgPort  = StrToInt(IniConfig.N06_FtpPort);
                    for(int i=0; i<lstFiles->Count; i++)
                    {
                        if(lstFiles->Strings[i]!="." && lstFiles->Strings[i]!="..")
                        {
                            asJamCodeFileName.sprintf("%s\\%s", asJamCodeFilePath, lstFiles->Strings[i]);
                            AnsiString asBgRemote = IniConfig.FtpUplaodPath + lstFiles->Strings[i]; //AI(ht9045-v899) 20260612(CASE-20260611-001): 遠端=FtpUplaodPath+純檔名,比照原同步UploadFileToServer2內部 str02=FtpPath+Source(Source已砍成純檔名);先前只給FtpUplaodPath會少檔名導致FtpPutFile遠端目標錯誤
                            FtpUploadThd->EnqueueUpload(0, asJamCodeFileName, asBgRemote,  //jobKind=0(JamAlarm),local=完整路徑,remote=FtpUplaodPath+檔名,retry=2(比照S2語意)
                                                        asBgHost, asBgUser, asBgPwd, iBgPort, 2);
                        }
                    }
                }
                else                                                           //AI(ht9045-v899) 20260630(CASE-PTI-20260630-002): FtpUploadThd 尚未建立時記 log 並跳過,寧可不傳也不落至同步阻塞 Lot End
                {
                    MyDBIProcess("Process", "FtpUploadThd null, skip background JamAlarm upload", edtSysLotID->Text);
                }
            }
            else                                                               //AI(ht9045-v899) 20260612(CASE-20260611-001): 開關關(非PTI/預設) → 維持原同步上傳路徑,行為一字不改
            {
                fFTPClient->bError=false;                                           //AI(ht9045-v899) 20260611(CASE-20260611-001): 上傳前清旗標,作為連線失敗判斷依據
                for(int i=0; i<lstFiles->Count; i++)
                {
                     if(lstFiles->Strings[i]!="." && lstFiles->Strings[i]!="..")
                     {
                        MySleep(1000);                                              //Sam 20240308 : 上傳延遲一下
                        asJamCodeFileName.sprintf("%s\\%s", asJamCodeFilePath, lstFiles->Strings[i]);
                        fFTPClient->UploadFileToServer2(IniConfig.FtpUplaodPath, asJamCodeFileName);
                        if(fFTPClient->bError==true)                                //AI(ht9045-v899) 20260611(CASE-20260611-001): N06/FTP 連線失敗(socket 10038)即中止整批上傳,避免每個檔重複 blocking connect/retry 在主執行緒卡住 Lot End 導致 alarm 無法消除/hang
                            break;
                     }
                }
            }
        }
        lstFiles->Clear();                                                      //Ifor 20170603 (wei) TStringList 刪除前先 Clean
        delete lstFiles;
    }

    AnsiString asSocketIdProductDataPath, asSocketIdProductDataFileName;
    if(CUSTOMER_CODE==CC_PTI && RunInfo.bLotStart==true)                        //Sam 20170516 (wei) 自定義 SocketID Count 資料 csv 檔保存至 D:\\HT9045_Log\\SocketIdProductData\\yyyymmddhhnnss_LotID.csv
    {
        TStringList *SL;
        SL=new TStringList();
        asSocketIdProductDataPath.sprintf("D:\\HT9045_Log\\SocketIdProductData");
        asSocketIdProductDataFileName.sprintf("%s_%s.csv", Now().FormatString("yyyymmddhhnnss"), edtSysLotID->Text);
        for(int i=0; i<TestSocket.iShtRow; i++)
        {
            for(int j=0; j<TestSocket.iShtCol; j++)
            {
                SL->Clear();
                SL->Add(edtSysLotID->Text);
                SL->Add(LastSet.strSocketID[j][i]);
                SL->Add(AnsiString(LastSet.iSocketContactCount[j][i]));
            }
            fMain->slSocketIdProductData->AddText(SL->CommaText);
            fMain->slSocketIdProductData->MySaveFileByFileName(asSocketIdProductDataPath, asSocketIdProductDataFileName);
        }
        SL->Clear();                                                            //Ifor 20170603 (wei) TStringList 刪除前先 Clean
        delete SL;

        if(IniConfig.bN12_EnableSocketIdProductDataFTP==true)
        {
            if(IniConfig.bFtpUploadBackground==true)                           //AI(ht9045-v899) 20260612(CASE-20260611-001): 開關開(PTI預設) → SocketID CSV 走背景FTP上傳執行緒,fire-and-forget;丟完即返回,不MySleep/不等待,避免Lot End主執行緒被同步上傳阻塞造成卡站或alarm無法消除
            {
                if(FtpUploadThd!=NULL)                                          //AI(ht9045-v899) 20260612(CASE-20260611-001): 保險,背景執行緒未建立則略過(不退回同步,避免同一檔重複上傳)
                {
                    //AI(ht9045-v899) 20260612(CASE-20260611-001): 組本地完整路徑(asDirPath+"\\"+sFileName,比照UpSocketIdPoductDataToServerByFTP內asFileName,FTPClient.cpp L2236)與遠端完整路徑(asN12_FtpUplaodPath+sFileName,比照同函式asCSV,FTPClient.cpp L2267);連線參數用N12專用host/user/pwd,port=21(同步版NMFTP2->Port=21固定值,FTPClient.cpp L2247)
                    AnsiString asBgLocal  = asSocketIdProductDataPath + "\\" + asSocketIdProductDataFileName;
                    AnsiString asBgRemote = IniConfig.asN12_FtpUplaodPath + asSocketIdProductDataFileName;
                    FtpUploadThd->EnqueueUpload(1, asBgLocal, asBgRemote,       //jobKind=1(SocketID CSV),local=完整路徑,remote=完整遠端檔路徑,retry=2(比照S6語意)
                                                IniConfig.asN12_FtpHost, IniConfig.asN12_FtpUserName, IniConfig.asN12_FtpPassword, 21, 2);

                    for(int i=0; i<4; i++)                                      //AI(ht9045-v899) 20260612(CASE-20260611-001): 丟佇列即清(決議2):不等上傳成功就清SocketID count;此段為UpSocketIdPoductDataToServerByFTP上傳成功後清除程式碼之複製(FTPClient.cpp L2271-2278),背景分支不呼叫該函式故須在呼叫端補清,僅背景分支這樣做
                    {
                        for(int j=0; j<8; j++)
                        {
                            LastSet.iSocketContactCount[i][j]=0;
                        }
                    }
                }
            }
            else                                                               //AI(ht9045-v899) 20260612(CASE-20260611-001): 開關關(非PTI/預設) → 維持原同步上傳路徑,行為一字不改(MySleep+呼叫函式,SocketID count清除仍在函式內部上傳成功後執行)
            {
                MySleep(1000);                                                      //Sam 20240308 : 上傳延遲一下
                fFTPClient->UpSocketIdPoductDataToServerByFTP(asSocketIdProductDataPath, asSocketIdProductDataFileName);
            }
        }
    }
#endif // GATE (W906-PROD-S117-S25-PTI)
    if(CUSTOMER_CODE==CC_PTI && RunInfo.bLotStart==true)
        W906_LotEndSkipped->Add("golden uLotInfo.cpp:2078-2200 PTI JamAlarmLogTxt FTP upload / input-quantity log / SocketIdProductData csv (S25 customer-specific, gated)");

    //AI(W906-PROD-S117) 20260926: GATE (W906-PROD-S117-S25-B03) —— golden :2202-2208，力成 Tester report（S25）；
    //  else 臂 SetRunStartMode(rsmInitial_ART) 另外屬 (2) 模式切換（Jimmy）。
#if 0 // GATE (W906-PROD-S117-S25-B03)
    if(IniConfig.bB03_TesterReport)                                             //Sam 20231115 : PTI 新增 Tester report
    {
        if(bCanRunSCKART==false)                                                //Sam 20240809 : PTI ART 模式
            ProductTesterReport();
        else
            SetRunStartMode(rsmInitial_ART);
    }
#endif // GATE (W906-PROD-S117-S25-B03)
    if(IniConfig.bB03_TesterReport)
        W906_LotEndSkipped->Add("golden uLotInfo.cpp:2202-2208 [B03] ProductTesterReport() / SetRunStartMode(rsmInitial_ART) (S25 PTI + mode switch, gated) -- Jimmy for the mode switch");

    //AI(W906-PROD-S117) 20260926: GATE (W906-PROD-S117-S25-TSMC) —— golden :2210-2211，CC_TSMC_TAINAN 專屬（S25）。只動按鈕 Down。
#if 0 // GATE (W906-PROD-S117-S25-TSMC)
    if(CUSTOMER_CODE==CC_TSMC_TAINAN)
        sbSECSLotStart->Down=true;
#endif // GATE (W906-PROD-S117-S25-TSMC)
    if(CUSTOMER_CODE==CC_TSMC_TAINAN)
        W906_LotEndSkipped->Add("golden uLotInfo.cpp:2210-2211 TSMC_TAINAN sbSECSLotStart->Down=true (S25 customer-specific, gated)");

    //AI(W906-PROD-S117) 20260926: GATE (W906-PROD-S117-BCIR) —— golden :2214，缺相依：移植樹 TfBarCode（aHotPlateSubstrate.h:1037）
    //  沒有 SaveInspReportEnd（同本檔 GATE (W906-LOT-W1-BARCODE) 那一族）。golden 本體（V912 BarCode/BarCode.cpp:11916）第一行就是
    //  `if(TestIF_File.bBarCodeInspReport==false) return;` ⇒ 沒開 2D 檢驗報表的機台，golden 這一行什麼都不做。KYEC 整合（S25）。
#if 0 // GATE (W906-PROD-S117-BCIR)
    //==> Eastsun 20260527 整合#BCIR.P14 SaveInspReportEnd :KYEC
    fBarCode->SaveInspReportEnd(asATCEvenLotID, GetLastOpenFN());                              //Sam 20240426 : Add BarCoder Inspection Report
    //<== Eastsun 20260527 #BCIR.P14
#endif // GATE (W906-PROD-S117-BCIR)
    if(TestIF_File.bBarCodeInspReport)
        W906_LotEndSkipped->Add("golden uLotInfo.cpp:2214 fBarCode->SaveInspReportEnd (Code Reference footer of the 2D inspection report) not written (TfBarCode member absent, gated)");

    //AI(W906-PROD-S117) 20260926: GATE (W906-PROD-S117-S25-HSLOG) —— golden :2217-2235，KYEC_HS log 上傳 FTP（S25）：
    //  結束 EP／Temp／ESD／ATC／Arm log 的旗標與 FormHS->RecordLog_HS(true)（FormHS 在移植樹是替身）。
#if 0 // GATE (W906-PROD-S117-S25-HSLOG)
    if(CosFunction.bUseLogUploadToFTPFunction==true)
    {
        bSysLotStart=false;                                                     //Ifor 20160302 add for KYEC_HS bSysLotStart
        bEPLogEnd_KYEC=true;                                                    //Ifor 20160302 add for KYEC_HS EPLogEnd
        bTempLogEnd_KYEC=true;                                                  //Ifor 20160302 add for KYEC_HS TempLogEnd
        FormHS->RecordLog_HS(true);

        if(USE_NOVX3360==true)
            bESDLogEnd_KYEC=true;                                               //Ifor 20160302 KYEC FTP UP Load ESD Log End

        if(CUSTOMER_CODE==CC_KYEC_LEE &&
           (ATC_SYSTEM==eATCHonPrecType || ATC_SYSTEM==eNewATCSystem) &&
           Temperature.bATCActiveCooling==true)
        {
            bATCEvenLogEnd_KYEC=true;
            asATCEvenLotID="";
        }
        bArmTestInfoEvenLogEnd_KYEC=true;                                       //Ifor 20190912 :add 海思 V02.30 版 Record Torque
    }
#endif // GATE (W906-PROD-S117-S25-HSLOG)
    if(CosFunction.bUseLogUploadToFTPFunction==true)
        W906_LotEndSkipped->Add("golden uLotInfo.cpp:2217-2235 KYEC_HS log-end flags / FormHS->RecordLog_HS(true) (S25 customer-specific, gated)");

    //AI(W906-PROD-S117) 20260926: GATE (W906-PROD-S117-S25-OEE) —— golden :2237-2256，超豐 OEE（S25）：上傳 Bin 數量／Socket 壽命／
    //  Tray mapping、OEE_EndLot、fSortCT->btnClearCountClick（清 Sort Count）。移植樹 TfProductionInfo 沒有那幾支上傳。
#if 0 // GATE (W906-PROD-S117-S25-OEE)
    if(CosFunction.bOEEFunction)                                                //Steven 20180417 (Jou) : OEE功能
    {                                                                           //JimmyChiu 20220118 相似功能整合
        #ifndef SOFT_SIMULTE                                                    //JimmyChiu 20220119 : ADD
        fProductionInfo->DoIPSCProcess();
        #endif
        RecordProcess("End Lot Press");
        lb_PIOEELotStatus->Caption="Production End Lot.....";
        fProductionInfo->UploadBinQtyReport(true);
        #ifndef SOFT_SIMULTE                                                    //JimmyChiu 20220119 : ADD
        fProductionInfo->SocketLifeTimeUpload();
        #endif
        fProductionInfo->UploadTrayMappingLog();                                //Sam 20201209 : 增加資料上傳
        fProductionInfo->OEE_EndLot();
        fSortCT->btnClearCountClick(fSortCT);
        lb_PIOEELotStatus->Caption="Production End Lot Success!";
        ed_PIOEEMO->Text="";
        fObserver->sTestReceiveTime="";                                         //KaiChen 20171127 (Steven) ：超豐 OEE 新增 Test Time、Index Time
        fObserver->sTestIndexZTime="";
        fObserver->dOEEIndexCycleTime=0;                                        //Sam 20180802 (wei) : OEE 32Site 修正
    }
#endif // GATE (W906-PROD-S117-S25-OEE)
    if(CosFunction.bOEEFunction)
        W906_LotEndSkipped->Add("golden uLotInfo.cpp:2237-2256 OEE end-lot uploads / OEE_EndLot / fSortCT->btnClearCountClick (S25 customer-specific, gated)");

    //AI(W906-PROD-S117) 20260926: GATE (W906-PROD-S117-SUMMARY) RETIRED —— golden :2258-2261（G-030）。
    //  查清楚的結果（V912 Automation/SCK_ART.cpp）：SaveTestSummary（:1620-1646）只是分派 ——
    //    2D sort（CosFunction.bSortingBy2DList && LastSet.iTester==_2D_SORT && TestIF_File.bSortingBy2DIDList）→ Save2DSortingSummary；
    //    CosFunction.bART_SECSGEM_93K → SaveTestSummarySECS；TestIF_File.iTestType==TCP_IP_MODE → ProcessOSPrint＋SaveTestSummaryTSV；
    //    其餘（一般機台）→ SaveSummaryTrayFeed()＋SaveTestSummaryTSV(1)。
    //  一般機台那一支不是 SCK_ART（S80）客戶專屬，[N09] 沒開的機台 Lot End 真的會寫兩個檔：
    //    SaveSummaryTrayFeed（:3130-3400）：D:\HT9045_Log\Summary_Lot\YYYYMM\"<IniConfig.SocketHandlerID> YYYYMMDD-hhnnss <RunInfo.LotNo> Summary.txt"
    //      （資料 TastCategory.UpdataCount＝ArmData[] 的 Site／Bin 計數、Prod.iTrayType、RunInfo.LotStart/EndTime、fLotInfo 的 Customer／Operator）；
    //      [N10] 開時另外上傳（FormHS 在移植樹是替身，只記錄不上傳）。
    //    SaveTestSummaryTSV（:2806-3128）：asSummaryPath（D:\HT9045_Log\Summary）\YYYY\MM\"<LotID>_<Process>_FT_…txt"（iFTRTCount!=1 時 "_RT<n-1>_"），
    //      資料 LotSummary.iCountCategory／iTotalCategory（Hard Bin 表）＋fSCKART 的 sLotID／sProcessCode／iFTRTCount／iNeedRT／iLotCount；
    //      最後 fObserver->memoLotSummary->Lines＝這份內容、LotSummary.ClearAllData()（:3127）。
    //  移植樹的 golden 本體在 Automation/SCK_ART_Remainder.cpp（SckArtRem_SaveTestSummary :914，已照翻）。它吃 SckArtRemainderState；下面 (1)(2) 的轉接已抽成 W906_SckArt_SaveTestSummary（Automation/SCK_ART_SaveTestSummary.cpp），
    //  而 golden 只有一個 fSCKART、一個 LotSummary —— 移植樹各有兩份，所以這裡做轉接（不改那兩份的任何一份）：
    //    (1) 真的 fSCKART（forms/fSCKART.h）上那五個欄位抄進暫時的 state；跑完把 golden 會改的 sProcessCode（空的時候補 "FT1"，:2835-2836）抄回。
    //    (2) SckArtRem_SaveTestSummaryTSV 讀的是 gate #5 的替身 W5SckArtRem_LotSummary，不是真的 LotSummary（cSocket.cpp:228，
    //        ainarm9045.cpp:6399 iLoadTotal++、asortarm.cpp:3893 AddCount 寫的那一個）：跑之前把真的四個欄位抄進替身，
    //        跑完照 golden :3127 對真的 LotSummary 做 ClearAllData()（替身那邊 SaveTestSummaryTSV 自己清）。
    //    (3) 2D sort 與 93K SECS ART 兩支要 SckArtRemainderState 的 sInfo_*／sLotEndTime／iInfo_MultiLotCnt…，真的 fSCKART 沒有這些欄位
    //        （客戶專屬 S80／S25）—— 硬跑會用空字串寫出錯的報表，所以那兩支不跑，記進 W906_LotEndSkipped。
    //  ⚠ 資料缺口（不是這一段的問題，交 Jimmy）：golden 主 OutArm 放料時的 LotSummary.AddCount（V912 aoutarm9045.cpp:2877，
    //    DoOutArmPlaceToAuto）在移植樹是 aoutarm9045.cpp:220 的 offline 樁 —— HT9045 上 Hard Bin 表目前全是 0，SaveSummaryTrayFeed 那份不受影響。
    //  ⚠ 會寫真實檔（log，不是機台參數）：上面兩個檔＋MyForceDirectories 建 D:\HT9045_Log\TestSummary\YYYY\MM\（golden :2828-2830，非 TSV 機台不寫檔）。
    //    Summary_Lot 可用 W906_SUMMARYLOT_ROOT 轉開（SckArtRem_SaveSummaryTrayFeed），asSummaryPath 沒有轉接縫（common.cpp:164）。
#if 1 // AI(W906-PROD-S117) 20260926: GATE (W906-PROD-S117-SUMMARY) RETIRED -- golden verbatim in the #else arm
    if(IniConfig.bN09_LotCountAutoFunc==false)                                  //JerryYang 20190928 SPIL lot count
    {
        W906_SckArt_SaveTestSummary(1, W906_LotEndSkipped);                     // golden: fSCKART->SaveTestSummary(1);（轉接：Automation/SCK_ART_SaveTestSummary.cpp，Jimmy 23:4x 要求抽成共用）
    }
#else
    if(IniConfig.bN09_LotCountAutoFunc==false)                                  //JerryYang 20190928 SPIL lot count
    {
        fSCKART->SaveTestSummary(1);
    }
#endif // GATE (W906-PROD-S117-SUMMARY)

    //AI(W906-PROD-S117) 20260926: GATE (W906-PROD-S117-S25-VTEST) —— golden :2263-2311，偉測 VTEST（S25）：MES RunMode／Summary
    //  報表、清 MES 資料、元件解鎖。
#if 0 // GATE (W906-PROD-S117-S25-VTEST)
    if(IniConfig.bVTESTFunction==true)
    {
        cbProcess->Enabled=true;
        cbTestTimes->Enabled=true;                                              //RogerYang 20250809 偉測Summary文件修改
        iProduceTimeCT=0;                                                       //jou 20221125 : 機台添加三小時送檢報警，從lot start時間開始計算

        if(CUSTOMER_CODE==CC_VTEST_Shanghai && IniConfig.bCheckFile)            //jou 20210823 : lot start增加保護避免重複執行run mode
        {
            if(RunInfo.bLotStart==true &&
               fMesSystem->bDownloadLotInforFlag==true)
            {
                fMesSystem->RunModeRW(false, edtSysLotID->Text, cbRunMode->Text);

//                #ifdef BETA_VTestSummaryFile                                    //RogerYang 20250809 偉測Summary文件修改
                    fMesSystem->VTestSummaryReport();
//                #else
//                if(IniConfig.bN10_UploadSummaryToFTP==true)
//                {
//                    fMesSystem->VTestSummaryReport();
//                    fMesSystem->VTestUPHReport();                               //jou 20220525 : lot end 時輸出 UPH report
//                }
//                #endif
            }
        }
        else if(CUSTOMER_CODE==CC_VTEST)
        {
//            #ifdef BETA_VTestSummaryFile                                        //RogerYang 20250809 偉測Summary文件修改
                fMesSystem->VTestSummaryReport();
//            #else
//                if(IniConfig.bN10_UploadSummaryToFTP==true)
//                {
//                    fMesSystem->VTestSummaryReport();
//                }
//            #endif

            edtProcessName->Enabled=true;
            edtProcessName->Text="";
            edtProduct->Enabled=true;
            edtProduct->Text="";
        }

        if(fMesSystem->CheckVTENGmode(edtSysLotID->Text)==true)
        {
//            SetLotID("");
//            edtSysOperatorID->Text="";
        }

        fMesSystem->ClearMesData();
    }
#endif // GATE (W906-PROD-S117-S25-VTEST)
    if(IniConfig.bVTESTFunction==true)
        W906_LotEndSkipped->Add("golden uLotInfo.cpp:2263-2311 VTEST MES RunMode / summary report / ClearMesData (S25 customer-specific, gated)");

    //AI(W906-PROD-S117) 20260926: GATE (W906-PROD-S117-S25-UTAC) —— golden :2313-2323，UTAC（S25）＋ SetLotState 是送給 Tester／
    //  ART 的批次狀態（Steven02）。
#if 0 // GATE (W906-PROD-S117-S25-UTAC)
    if(CUSTOMER_CODE==CC_UTAC)                                                  //Richard 20220929 :Add for UTAC Lot Start/End 0xC0
    {
        if(LastSet.iRunStartMode==rsmContinuStart)
        {
            fMain->SetLotState(8);                                              //UTAC ART FT Lot End
        }
        else if(LastSet.iRunStartMode==rsmContinuRetest)
        {
            fMain->SetLotState(10);                                             //UTAC ART Final Lot End
        }
    }
#endif // GATE (W906-PROD-S117-S25-UTAC)
    if(CUSTOMER_CODE==CC_UTAC)
        W906_LotEndSkipped->Add("golden uLotInfo.cpp:2313-2323 UTAC fMain->SetLotState(8/10) (S25 customer-specific, gated) -- Steven02");

    //AI(W906-PROD-S117) 20260926: GATE (W906-PROD-S117-S25-GROUNDESD) —— golden :2325，缺相依＋客戶功能（S25）：
    //  TfLotInfo::SaveGroundESDByLot（V912 :14127）在移植樹沒有（forms/fLotInfo.h:1071 列為未移植）。它第一行是
    //  `if(CosFunction.bRecordGroundESDByTestIC==false) return;`，開的話把 GroundESDLog\<LotNo>.csv 搬到 yyyy-mm 資料夾並可上傳 FTP（N30）。
#if 0 // GATE (W906-PROD-S117-S25-GROUNDESD)
    SaveGroundESDByLot(RunInfo.LotNo, edCustomerLotId->Text, coStation->Text, edStationNum->Text);                      //Sam 20211223 : 每顆 IC 測試完畢都要記錄當時的 Ground & ESD 數值。
#endif // GATE (W906-PROD-S117-S25-GROUNDESD)
    if(CosFunction.bRecordGroundESDByTestIC)
        W906_LotEndSkipped->Add("golden uLotInfo.cpp:2325 SaveGroundESDByLot (move GroundESDLog csv + N30 FTP) not run (method absent; S25, gated)");

    //AI(W906-PROD-S117) 20260926: GATE (W906-PROD-S117-S25-CYUEAN) —— golden :2327-2334，CC_CYUEAN（S25）：
    //  bFTPDownloadSetupFile=false 與 [N34] OEE alarm report 的 EventLog 指令。
    //AI(W906-ELA-N34) 20260928 [W906] GATE (W906-PROD-S117-S25-CYUEAN) opened: Steven W22 -- N34 (OEE And Failure Report) is one of the five reports, "功能有開的都要做"; whole golden block, verbatim
    if(CUSTOMER_CODE==CC_CYUEAN)
    {
        bFTPDownloadSetupFile=false;
        if(IniConfig.bN34_GenerateOEEAlarmRpt)                                  //Jimmychiu 20250324 : CC_CYUEAN OEE report
        {
            SendCommand_EventLog(EL_UPLOAD_CHIPADV_LOTEND, "1");
        }
    }
    //AI(W906-ELA-N34) 20260928 [W906] end of the opened block; SendCommand_EventLog body = Interface/InterfaceSYS.cpp:554 (ht9045_sm), sent only when [N34] is checked
    //AI(W906-ELA-N34) 20260928 [W906] the CYUEAN W906_LotEndSkipped note that stood here is retired: the block above now runs, so the lotEnd reply's ELTodo
    //  list must not call it skipped. Until St02's ELA R3 is on main, ElaHub only records EL_UPLOAD_CHIPADV_LOTEND as "not ported" (EventLogAnalysis/ElaHub.cpp:254).

    //AI(W906-PROD-S117) 20260926: GATE (W906-PROD-S117-S25-LEADYO) —— golden :2336-2337，利揚（S25）：OCR＋Bin log 上傳 Host。
#if 0 // GATE (W906-PROD-S117-S25-LEADYO)
    if(CUSTOMER_CODE==CC_LEADYO && IniConfig.bN33_UpLoadOCRBinLogByNet)         //KenHsieh 20230502 : 利揚要求上傳OCR + BIN Log上傳至Host
        fOCR->UpLoadOCRAndBinLog();
#endif // GATE (W906-PROD-S117-S25-LEADYO)
    if(CUSTOMER_CODE==CC_LEADYO && IniConfig.bN33_UpLoadOCRBinLogByNet)
        W906_LotEndSkipped->Add("golden uLotInfo.cpp:2336-2337 LEADYO fOCR->UpLoadOCRAndBinLog (S25 customer-specific, gated)");

    //AI(W906-PROD-S117) 20260926: GATE (W906-LOT-W1-JCET2DID) —— golden :2339-2342，同本檔 ReadWriteLotInfo 那個閘（缺
    //  TfBarCode::JCETUseMakeWhite2DIDList／UpdateWhite2DIDList；既有慣例 G-1／WA-7）＋ JCET 客戶功能（S25）。
#if 0 // GATE (W906-LOT-W1-JCET2DID)
    if(fBarCode->JCETUseMakeWhite2DIDList()==true)                              //RogerYang 20251202 : JCET 2D FT1白名單/FT2比對功能
    {
        fBarCode->UpdateWhite2DIDList(fLotInfo->edtSysLotID->Text, fLotInfo->cbRunMode->Text);
    }
#endif // GATE (W906-LOT-W1-JCET2DID)
    if(CUSTOMER_CODE==CC_JCET)                                                  // 條件函式本身缺，這裡只用客戶碼粗估「golden 可能會執行」
        W906_LotEndSkipped->Add("golden uLotInfo.cpp:2339-2342 JCET 2D white-list update (TfBarCode members absent; S25, gated)");

    SetLotComponents(true);                                                     //Steven 20250515 : 整合Lot測試報表   // ← RunInfo.bLotStart=false（分類 (2)，證據見本段檔頭）
    SetLotID("");
    lbledtStarTime      ->Text="";
    lbledtEndTime       ->Text="";
    lbledtTesterOsVer   ->Text="";
    lbledtTesterID      ->Text="";
    edtSysOperatorID    ->Text="";
    lbledtCustomer      ->Text="";
    lbledtTestProg      ->Text="";
    lbledtDeviceName    ->Text="";
    edtSysLotID         ->Text="";
    lbledtSubLotNo      ->Text="";
    lbledtModeCode      ->Text="";
    lbledtTestCode      ->Text="";
    lbledtTestBinNo     ->Text="";
    cbRunMode           ->Text="";
    edCustomerLotId     ->Text="";
    coStation           ->Text="";
    edStationNum        ->Text="";
    edtCusLotID         ->Text="";
    edtCusDevGrp        ->Text="";
    edtBarcodeRecipe    ->Text="";   //==> Eastsun 20260527 整合#027-2.MR.U3 clear edtBarcodeRecipe :KYEC
    edPage              ->Text="";
    edtStage            ->Text="";
    edtStep             ->Text="";
    //AI(W906-PROD-S117) 20260926: GATE (W906-LOT-W1-SLEVENTLOG) —— golden :2369，同本檔 SetLotStart 的兩處：slEventLog 從未建構
    //  （cmydef.cpp:120 裸指標；Steven02 的 S72 建構它時，這裡與 SetLotStart 那兩處一起解）。
#if 0 // GATE (W906-LOT-W1-SLEVENTLOG): 缺相依 —— slEventLog 從未建構
    slEventLog->SetLotData(RunInfo.LotNo, lbledtStarTime->Text);                //KaiChen 20181121 ：矽格-北興 Save Event Log by Lot ID
#endif // GATE (W906-LOT-W1-SLEVENTLOG)
    W906_LotEndSkipped->Add("golden uLotInfo.cpp:2369 slEventLog->SetLotData(RunInfo.LotNo, \"\") not run (slEventLog never constructed, gated with SetLotStart's two sites) -- Steven02 S72");

    for(int i=0; i<3; i++)                                                      //Steven 20250603 : By Lot Summary
    {
        ArmDataLot[i]->ClearALLCT();
    }

    ReadWriteLotInfo(false);
    SL->Clear();
    delete SL;
}

// ---- sbSECSLotEndClick（V912 uLotInfo.cpp:1387-1444）逐行照翻 ---------------------------------------------------------
//  W906_LotEndResult 是移植樹加的（每個 golden return 前記一下停在哪），網頁用它說明「為什麼沒有結批」；不改 golden 的判斷。
void TfLotInfo::sbSECSLotEndClick(TObject *Sender)                              //ChungHung OLP 聚成專用      //wei 20150324 更改元件Button-->SpeedButton
{
    W906_LotEndResult="";
    W906_LotEndSkipped->Clear();
    AnsiString Msg;
    TSpeedButton *Ptr;
    Ptr=(TSpeedButton *)Sender;

    if(SystemStart && Ptr->Tag==0)                                              //Sam 20200226 : OLP 增加 LotStar & LotEnd 控制
    {
        sbSECSLotStart->Down=true;
        W906_LotEndResult="system-running";
        return;
    }

    if(CosFunction.bAutoCleanShuttleDisable==false)                             //wei 20150326 CC_KYEC 隨時都可更改不需要clean out
    {
        if(fMain->CheckCanChangeRealDummy()==false)
        {
            sbSECSLotStart->Down=true;
            //AI(W906-PROD-S117) 20260926: GATE (W906-PROD-S117-RSM) —— golden :1404，分類 (2) 模式切換（Jimmy，RunStartMode.cpp）。
            //  golden 用它把主畫面的 Run Start Mode 下拉選單同步回目前模式；移植樹 SetRunStartMode 會重跑整段模式切換
            //  （例：rsmAutoSiteMap 時 ReStartAutoSiteMapping(true)），不在 St01 範圍。拒絕本身（下一行告警＋return）照 golden。
#if 0 // GATE (W906-PROD-S117-RSM)
            SetRunStartMode((eRunStartMode)LastSet.iRunStartMode);
#endif // GATE (W906-PROD-S117-RSM)
            W906_LotEndSkipped->Add("golden uLotInfo.cpp:1404 SetRunStartMode((eRunStartMode)LastSet.iRunStartMode) not run (mode switch, gated) -- Jimmy");
            W906_LotEndResult="must-clean-out-1";
            ShowErrorMessage("MES1646", 0, MMSystem, false, "SECSLotEndClick1");                                        //Must finish [Clean out]!!
            return;
        }

        if(iTestHeadMotorTask!=1 && fAllMotorHome==true)
        {
            sbSECSLotStart->Down=true;
#if 0 // GATE (W906-PROD-S117-RSM): golden :1412，同上
            SetRunStartMode((eRunStartMode)LastSet.iRunStartMode);
#endif // GATE (W906-PROD-S117-RSM)
            W906_LotEndSkipped->Add("golden uLotInfo.cpp:1412 SetRunStartMode((eRunStartMode)LastSet.iRunStartMode) not run (mode switch, gated) -- Jimmy");
            W906_LotEndResult="must-clean-out-2";
            ShowErrorMessage("MES1646", 0, MMSystem, false, "SECSLotEndClick2");                                        //Must finish [Clean out]!!
            return;
        }
    }

    if(CosFunction.bOEEFunction)                                                //Steven 20180417 (Jou) : OEE功能
    {                                                                           //JimmyChiu 20220118 相似功能整合
        if(fProductionInfo->_bOEEStartLotSuccess==false)                        //Sam 20170810 (Steven) 移植超豐 OEE 功能 form HT-7045
        {
            W906_LotEndResult="oee-not-started";
            ShowMyMessage("Please Lot Start!");
            return;
        }

        //AI(W906-PROD-S117) 20260926: GATE (W906-PROD-S117-OEEASK) —— golden :1426-1429，缺相依：移植樹 vclcompat 沒有
        //  Application->MessageBox（FileRW/HSys.cpp:566 同一個呼叫也在閘裡）。OEE 客戶專屬（S25）。
        //  網頁 lotinfo.op lotEnd 本來就是兩段式確認（先問再做），所以操作員仍然會被問一次；少的是 golden 這一個 OEE 專用的 Yes/No 框。
#if 0 // GATE (W906-PROD-S117-OEEASK)
        if(Application->MessageBox("Are You Sure End Lot?", "End Lot?", MB_YESNO)!=IDYES)
        {
            return;
        }
#endif // GATE (W906-PROD-S117-OEEASK)
        W906_LotEndSkipped->Add("golden uLotInfo.cpp:1426-1429 OEE \"Are You Sure End Lot?\" Yes/No box not shown (Application->MessageBox absent; the web two-step confirm asked instead; S25)");
    }

    SetLotEnd(__FUNC__);                                                        //Steven 20250515 : 整合Open Short測試報表
    W906_LotEndResult="executed";

    //AI(W906-PROD-S117) 20260926: GATE (W906-PROD-S117-MAINPANELS) —— golden :1434-1438，分類 (2) 主畫面按鈕 Enabled（Jimmy，OPMODE）。
    //  五個 widget 在 forms/fMain.h:307-311 已經有了（W906-P10），不是缺相依；閘住是因為它跟 Lot Start 鎖住它們的那一半
    //  （本檔 SetLotStart 的 GATE (W906-LOT-W1-MAINPANELS)）是同一對，要一起決定。今天移植樹讀 cbRunStartMode->Enabled 的只有
    //  Command.cpp:12162-12180（遠端切模式前後備份還原）與 RTClick 的空樁，網頁不讀。
#if 0 // GATE (W906-PROD-S117-MAINPANELS)
    fMain->cbRunStartMode->Enabled=true;
    fMain->palFT->Enabled=true;
    fMain->palRT->Enabled=true;
    fMain->palOffLine->Enabled=true;
    fMain->palEQC->Enabled=true;
#endif // GATE (W906-PROD-S117-MAINPANELS)
    W906_LotEndSkipped->Add("golden uLotInfo.cpp:1434-1438 fMain->cbRunStartMode/palFT/palRT/palOffLine/palEQC->Enabled=true not run (paired with SetLotStart's W906-LOT-W1-MAINPANELS lock, gated) -- Jimmy OPMODE");
    bLotID_OK=false;
    bOPID_OK=false;
    bTemp_OK=false;
    bDeviceName_OK=false;
    //AI(W906-PROD-S117) 20260926: GATE (W906-PROD-S117-S25-VTEST) —— golden :1443，偉測銦片 Change KIT 旗標（S25）＋缺相依：
    //  移植樹 TfLotInfo 沒有 bVTESTKitCheckPending（golden uLotInfo.h:1441；唯一的讀者是 golden TfMain::Start main.cpp:6381，移植樹沒有）。
#if 0 // GATE (W906-PROD-S117-S25-VTEST)
    bVTESTKitCheckPending=false;                                                //AI(ht9045-config) 20260710 (RogerYang) : 清除旗標
#endif // GATE (W906-PROD-S117-S25-VTEST)
    if(IniConfig.bVTESTFunction==true)
        W906_LotEndSkipped->Add("golden uLotInfo.cpp:1443 bVTESTKitCheckPending=false not run (member absent; S25, gated)");
}

// ---- 網頁用：Lot End 鈕看不看得到、按不按得到（只取 golden 寫這幾個屬性的敘述，一行不改；其他副作用不做）--------------
//  理由同本檔 Steven 20260925 段落檔頭「為什麼不直接呼叫 FormShow()／Timer2Timer()」。每次從 dfm 值重算 ＝ 設定不變時等於
//  golden 開窗後的穩態（與 W906_RefreshTabVisible 同一個假設）。
//  ⚠ 任務單寫「golden 只在 SECS/GEM 開或 bLotStartLockCriticalPara 時顯示」—— 實查 dfm 不是這樣：palSecsGem（Lot ID／OP ID／
//    Run Mode／Lot Start／Lot End 那一整塊）dfm 沒寫 Visible，預設看得到；golden 全樹只有 FormShow :370（CC_MTI 藏）與 :411（SECS／ATP
//    鎖參數時再打開）兩處寫它（20260926 grep audit_prod\golden_utf8 *.cpp：uLotInfo.cpp 兩筆，其他檔 0 筆）。所以一般機台都看得到，
//    :407-412 只對 CC_MTI 有意義。
bool TfLotInfo::W906_LotEndPanelVisible()
{
    palSecsGem->Visible=true;                                                   // dfm :314-323 沒寫 Visible
    if(CUSTOMER_CODE==CC_MTI)                                                   // golden :367-371
    {
        palSecsGem->Visible=false;                                              //Steven 20140701
    }
    if(IniConfig.bEnable_SECS_GEM ||                                            //Steven 20140701   // golden :407-412
       CosFunction.bLotStartLockCriticalPara)                                   //JerryYang 20220311 : ATP鎖定Critical parameter
    {
        palSecsGem->Visible=true;
    }
    return palSecsGem->Visible;
}

//  sbSECSLotStart／sbSECSLotEnd 的 dfm 設計期值（V912 uLotInfo.dfm:332-349）。vclcompat TControl 預設 Visible／Enabled＝false
//  （vclcompat/Controls.h:257），golden VCL 預設 true。golden 執行期寫 sbSECSLotEnd->Enabled 的只有 main.cpp:3055（OEE [N14-1]，
//  Timer1）、:3070（SJ OS Tester／XINITECH）、csystem.cpp:12256（Tray Feed 結束），移植樹都沒有 ⇒ 只套一次，之後不再覆蓋。
//  Caption 只在 OEE（FormShow :721-722）改成 "Start Lot"／"End Lot"，那一段本檔 :3516-3517 已翻（FormShow 本身在 wb_serve 不跑）。
void TfLotInfo::W906_LotEndDfmDefaults()
{
    if(W906_bLotEndDfmApplied)
        return;
    W906_bLotEndDfmApplied=true;
    sbSECSLotStart->Visible=true;       sbSECSLotStart->Enabled=true;           // dfm :332-340
    sbSECSLotStart->GroupIndex=1;       sbSECSLotStart->Caption="Lot Start";
    sbSECSLotEnd->Visible=true;         sbSECSLotEnd->Enabled=true;             // dfm :341-349
    sbSECSLotEnd->GroupIndex=1;         sbSECSLotEnd->Caption="Lot End";
    if(CosFunction.bOEEFunction)                                                // golden FormShow :721-722（在 OEE 那一支裡）
    {
        sbSECSLotStart->Caption="Start Lot";                                    //Sam 20170925 : 顯示名稱修改
        sbSECSLotEnd->Caption="End Lot";
    }
}

//  只讀預覽：照 sbSECSLotEndClick :1393-1424 的順序判斷，不寫任何東西、不跳任何框。真正的判斷在本體（兩者不一致時以本體為準）。
AnsiString TfLotInfo::W906_LotEndPrecheck()
{
    if(SystemStart && sbSECSLotEnd->Tag==0)                                     // :1393
        return "system-running";
    if(CosFunction.bAutoCleanShuttleDisable==false)                             // :1399
    {
        if(fMain->CheckCanChangeRealDummy()==false)                             // :1401
            return "must-clean-out-1";
        if(iTestHeadMotorTask!=1 && fAllMotorHome==true)                        // :1409
            return "must-clean-out-2";
    }
    if(CosFunction.bOEEFunction && fProductionInfo->_bOEEStartLotSuccess==false)   // :1418-1424
        return "oee-not-started";
    return "";
}

//---------------------------------------------------------------------------
//  AI(W906-FRW-WC1) 20260927 (Steven 團隊)：golden TfLotInfo::ResetLotInfo（V912 uLotInfo.cpp:14820-14834，逐字）。GATE WC-1 退役。
//    呼叫點：本檔 FormShow 兩處（golden :375、:429）＋ wb_serve 開機（golden TfMain::FormShow main.cpp:10991，
//    FileRW/MainBoot.cpp W906_FRWBoot_ResetLotInfo）。
//    有開 Lot Info、而且機台內有 IC 時：從 config.ini [Server]（CC_SCC／CC_SCK 是 [RMS]）讀回 Product Name／Product Temp
//    （ReadRmsInfo cprod.cpp:3340；缺鍵時 CheckAndReadIniData 會補寫空字串，golden 同），填回 Device Info 分頁的三格，
//    並把 LastSet.bHasDownloadFile 設成 true（記憶體；下一次 WriteLastDataFile 才進 lastdata.dat）。
//    沒有 IC：什麼都不做（golden 同）。
//---------------------------------------------------------------------------
void TfLotInfo::ResetLotInfo()                                                  //Steven 20240925 : 重開軟體時, 要讀回lot info
{
    if(IniConfig.bShowLotInfo)
    {
        if(HasICUnderMachine() ||                                               //Steven 20110527
           HasAutoICInMachine())                                                //jou 20211129 : 修正 VTEST Tray end 結束後重開程式會重置 Auto Tray盤造成疊料。
        {
            ReadRmsInfo();
            edDeviceName->Text  =IniConfig.sProductName;
            cbbDeviceName->Text =IniConfig.sProductName;
            edTemp->Text        =IniConfig.sProductTemp;
            LastSet.bHasDownloadFile=true;
        }
    }
}
//---------------------------------------------------------------------------
// AI(W906-CMYDB-A5) 20260929 (St02-E, claim cleared by the laptop / ST01-E 0929): golden 906_0625_Steven uLotInfo.cpp:10829-10838
//   (decl uLotInfo.h:1318; 912 :11010-11019 identical), verbatim.  Called by cMyDB.cpp SaveEventLogInfo on a day change with the
//   previous HANDLER LOG file name: a LOCAL copy of asSaveEventLogPath\<name>.csv into as9045LogPath\ASECL\ (overwrite), no network.
//   virtual (declared at fLotInfo.h:2216) so test_ga1_cmydb, which compiles cMyDB.cpp without fLotInfo.cpp, still links.
void TfLotInfo::UploadEventLogFile(AnsiString aFileName)             //移動前一天的
{
    AnsiString aPath=as9045LogPath+"\\ASECL";
    AnsiString aSource, aTarget;
    MyForceDirectories(aPath);
    aSource=asSaveEventLogPath+"\\"+aFileName+".csv";
    aTarget=aPath+"\\"+aFileName+".csv";
    if(FileExists(aSource))
        CopyFile(aSource.c_str(), aTarget.c_str(), false);
}
