// =============================================================================
//  forms/fMain.cpp  --  definitions for the fMain facade
//
//  AI(W906-W7-F0) 20260728: split out of FormsFacade.cpp by the W7-F0 refactor
//  (docs/W7_UI_ARCHITECTURE_PLAN.md SS6-F0-d).  Every body below is moved
//  VERBATIM -- these no-op / safe-default bodies are the PERMANENT OFFLINE
//  IMPLEMENTATION of the now-virtual method surface (plan SS6-F0-e); see the
//  contract block at the top of forms/fMain.h.
//
//  NOTE ON THIS FILE'S CROSS-LIBRARY DEPENDENCIES: cprod.h / cmydef.h pull in
//  globals that live in ht9045_globals (TestIF, asTCPIPTemperature) while this
//  file lives in the bottom-layer ht9045_forms.  ht9045_forms therefore declares
//  ht9045_globals as a dependency -- which is acyclic (ht9045_globals links no
//  project library except vclcompat; its only other link entries are the
//  psapi/version Win32 import libs).  See the ht9045_forms block in
//  CMakeLists.txt.
// =============================================================================
#include "forms/fMain.h"
// AI(W906-TesterTCPTimer) 20260720: cprod.h (TestIF.iTestType) / cmydef.h
// (asTCPIPTemperature, TCP_IP_MODE, MAX_SOCKET_TOTAL via cprod.h) -- needed by
// TfMain::WritePERSITETemperature below (Interface/TesterTCP_Socket.cpp's
// TimerProcessTCPDataTimer wave). cprod.h itself #includes MachineType.h, which
// is where MAX_SOCKET_TOTAL is #defined -- so this one include also covers the
// ctor's tBarCodeList->Strings[MAX_SOCKET_TOTAL-1] use below.
#include "cprod.h"
#include "cmydef.h"

// --- W6.3 ADD: TfMainHanaART ------------------------------------------------
bool TfMainHanaART::IsHanaArtAvailable() { return false; }     // offline: no HANA link
void TfMainHanaART::AddNewTrayHead(int /*iAuto*/) {}           // offline: no-op

// --- W6.5 ADD: TfMainInplace ------------------------------------------------
TfMainInplace::TfMainInplace()
{
    iNo9ShtErrICCt[0]=0; iNo9ShtErrICCt[1]=0;
    for(int i=0;i<2;i++) for(int j=0;j<8;j++) bNo9ShtErrNo[i][j]=false;
    bNo9Action=false;   // W7-A1: golden-faithful default (floating case 9000 sets false; no offline true-set)
}
bool TfMainInplace::InArmPlacementEnable() { return false; }   // offline: No9 placement disabled

// --- AI(W906-W7-L1-Wave0) 20260801 ADD: TfMainRENESASServer -----------------
// Golden Automation/uRENESAS_Server.h:199 (bLoadingCountFullFlag) / :201
// (DoNeedSupplyOrNot).  Offline there is no RENESAS FT-CT server socket, so
// DoNeedSupplyOrNot reports "no supply demanded" (false) and golden
// asendic_Loader.cpp:2722 / :2756 stay on their non-supply arm; the flag is
// plain storage that :2644 clears.
TfMainRENESASServer::TfMainRENESASServer() : bLoadingCountFullFlag(false) {}
bool TfMainRENESASServer::DoNeedSupplyOrNot(bool /*bNotEnough*/) { return false; }

// --- W6.2: TfMain ----------------------------------------------------------
TfMain::TfMain()
{
    // -- AI(W906-ST-S3-B2b) 20260918: the SOFT_SIMULTE widgets ----------------
    //   Constructed first: golden's SOFT_SIMULTE blocks dereference these without
    //   a null check (they were UI objects owned by the form in BCB6), so a miss
    //   is a crash inside simulation code, not a quiet wrong answer.
    chkInPickLoadError = new TfMainCheckBox();
    chkInToShtDrop     = new TfMainCheckBox();
    chkInFromLoadDrop  = new TfMainCheckBox();
    cb1                = new TfMainCheckBox();
    CheckBox1          = new TfMainCheckBox();
    Label3             = new TfMainLabel();
    chkHeaterOk        = new TfMainCheckBox();  chkHeaterOk->Checked = true;   // AI(W906-FLOW-1) 20260927: golden main.dfm:11872 `Checked = True`; SIM CheckHeater() copies it (uHeaterThread.cpp:507-509 = golden :138-143) -- the TCheckBox default (false) left fHeaterOK false forever, so a hot recipe parked InArm at Task=1

    slAutoSiteMapLog = new TfMainSiteMapLog();      // golden main.h:1486 (TMyStringList*)
    // -- W6.3 ADD --
    mtAuto1 = new TfMainTrayPanel();
    mtAuto2 = new TfMainTrayPanel();
    mtAuto3 = new TfMainTrayPanel();
    hanaART = new TfMainHanaART();
    // -- W6.4 ADD --
    lbCCDStatus = new TfMainTrayPanel();           // golden main.h:672 (TLabel* lbCCDStatus)
    // -- W6.5 ADD: shuttle-engine sub-objects --
    cbShowShuttleSensor    = new TfMainCheckBox();  cbShowShuttleSensor->Checked = true;   // AI(W906-MEMO) 20260927: golden main.dfm:16050/:16056 `Checked = True`; the TCheckBox default (false) kept every Out Shuttle sensor log path dead
    cbTestOutShuttleSensor = new TfMainCheckBox();
    cbShowInShuttleSensor  = new TfMainCheckBox();
    htShullte0 = new TfMainGrid();
    htShullte1 = new TfMainGrid();
    meShuttle1 = new TfMainMemo();
    meShuttle2 = new TfMainMemo();
    cInplace   = new TfMainInplace();
    pgMain     = new TfMainPageControl();          // ActivePageIndex==0 offline
    emp7TabSheet21 = 0;                            // ==pgMain->ActivePageIndex offline
    // -- W7-C1 ADD --
    BtnOneCycle = new TfMainSpeedButton();         // offline Down=false (else-branch one-cycle trigger inert)
    // -- W5-comms INTEGRATE ADD: Interface/InterfaceSYS.cpp IPC window handles --
    HESDWnd        = NULL;
    HEventLogWnd   = NULL;
    HAutoUpdateWnd = NULL;
    oldGpibAddress = 0;
    // -- W5-Automation ADD (AGV_PortScan unit, 20260713) -----------------------
    ALed1         = new TfLedValue();
    labAutomation = new TfMainPanel();
    // -- W906-Automation ADD (20260716) ----------------------------------------
    palMainStatus   = new TfMainPanel();
    cbSetupFileName = new TfLotInfoRunMode();
    edWorkTemperBase = new TfLotInfoEdit();
    // AI(W906-P10) 20260921: SetRunStartMode 用到的那一組（宣告見 fMain.h 的同名區塊）
    palFT            = new TfMainPanel();
    palRT            = new TfMainPanel();
    palEQC           = new TfMainPanel();
    palOffLine       = new TfMainPanel();
    cbRunStartMode   = new TfLotInfoRunMode();
    cbbRunModeSel    = new TfLotInfoRunMode();
    edSetOpenBin     = new TfLotInfoEdit();
    lbSetOpenBin     = new vclcompat::TLabel();
    cbUserSelect    = new TfLotInfoRunMode();  // W906-FW1d 20260820: login-level mirror (see fMain.h)
    // -- W906-TesterTCPTimer ADD (20260720) ------------------------------------
    tTestResult   = new TStringList();
    tBarCodeList  = new TStringList();
    for (int iW906T = 0; iW906T < 32; iW906T++)               // golden main.cpp:2236-2239
    {
        tTestResult->Add("-1");
        tBarCodeList->Add("0");
    }
    tBarCodeList->Strings[MAX_SOCKET_TOTAL-1] = AnsiString("0;");   // golden main.cpp:2241 quirk
                                                                     // (`+=";"` on a freshly-Add()ed
                                                                     // "0" -- equivalent to a direct
                                                                     // assignment here), PRESERVED:
                                                                     // makes the default BARCODE?
                                                                     // reply's last token "0;" (see
                                                                     // TesterTCP_Socket.cpp quirk #13).
    SVID1190_OSSetup = "";
    W906_PERSITETemperatureStrings_Sim = "";
    // -- W906-AutoCleanFoundation ADD (20260721) ------------------------------
    bAutoCleanTest         = false;
    cbIndexDrop            = new TfMainCheckBox();
    chkCleanPadPickErr     = new TfMainCheckBox();
    pnlCleanCount          = new TfMainPanel();
    pnlCleanCountFont      = new TfMainFont();
    AutoCleanContactCountLabel = new TfMainPanel();
    edHPX                  = new TfLotInfoEdit();
    edHPY                  = new TfLotInfoEdit();
    tmyAutoClean           = new TfMainAutoCleanGrid();
    AutoCleanStringGrid    = new TStringGrid();
    // -- W906-AutoCleanCluster ADD (20260722) ----------------------------------
    mtPlate2               = new TfMainAutoCleanGrid();
    // AI(W906-W7-F1fix2) 20260729: Pause() observation seams (see forms/fMain.h)
    W906_PauseCallCount    = 0;
    W906_PauseLastFunc     = "";
    iHasChangeFile         = 0;  // AI(W906-PT-W3-integrate) 20260808: golden main.h:1701, VCL zero-init (no golden ctor assignment)
    // AI(W906-W7-F1) 20260729: W7-F1 ADD -- "Wall 2" facade members
    //    SECSGEM/uHGemHT9045.cpp derefs (plan SS6-F1) -- see forms/fMain.h for
    //    the full per-member citations. Call-count seams start at 0; Sim
    //    seams default to the golden "no hardware blocks it" success/true
    //    value (see each member's own comment for why that default is
    //    golden-faithful).
    W906_cbSetupFileNameChangeCallCount = 0;
    W906_Clarn_DataCallCount            = 0;
    W906_BtnPauseClickCallCount         = 0;  // AI(W906-W7-F1fix) 20260729: seam for the bare-no-op-forward gap
    W906_LoadRunModePictureCallCount    = 0;
    W906_CanChangeSite_Sim              = true;
    W906_BtnTrayEndClickCallCount       = 0;
    W906_UpdateMainOperateModeCallCount = 0;
    W906_LoadStartModePictureCallCount  = 0;
    W906_LookForFileCallCount           = 0;
    W906_ChangeTesterConnect_Sim        = 0;
    W906_SetTemp_Sim                    = 0;
    W906_ChangePasswordCallCount        = 0;
    W906_FTClick_Sim                    = 0;
    W906_RTClick_Sim                    = 0;
    tSiteOnOff[0] = new TStringList();            // golden main.cpp:2229
    tSiteOnOff[1] = new TStringList();            // golden main.cpp:2230
    for (int iW7F1 = 0; iW7F1 < MAX_SOCKET_ROW * MAX_SOCKET_COL; iW7F1++)   // golden main.cpp:2242-2248
    {
        tSiteOnOff[0]->Add("0");
        tSiteOnOff[1]->Add("0");
    }
    edSoakTime = new TfLotInfoEdit();             // golden main.h:733 (TEdit*)
    // AI(W906-W7-L1-Wave0) 20260801: allocations for the 16 W7-L1 members added
    // to forms/fMain.h this pass (the asendic_* tray-SM family's fMain surface).
    // See that header for the per-member golden citations; the StringGrid2 size
    // below is golden's own main.dfm value, not a guess.
    lblLoadTrayCnt   = new TfMainTrayPanel();     // golden main.h:392 (TLabel*)
    lblAuto1TrayCnt  = new TfMainTrayPanel();     // golden main.h:393
    lblAuto2TrayCnt  = new TfMainTrayPanel();     // golden main.h:394
    lblAuto3TrayCnt  = new TfMainTrayPanel();     // golden main.h:395
    lblAuto4TrayCnt  = new TfMainTrayPanel();     // golden main.h:861
    lblAuto5TrayCnt  = new TfMainTrayPanel();     // golden main.h:862
    lblAuto6TrayCnt  = new TfMainTrayPanel();     // golden main.h:863
    edtAuto1         = new TfLotInfoEdit();       // golden main.h:867 (TEdit*)
    edtAuto2         = new TfLotInfoEdit();       // golden main.h:373
    edtAuto3         = new TfLotInfoEdit();       // golden main.h:374
    edtAuto4         = new TfLotInfoEdit();       // golden main.h:376
    edtAuto5         = new TfLotInfoEdit();       // golden main.h:377
    edtAuto6         = new TfLotInfoEdit();       // golden main.h:378
    chkE84IDTray     = new TfMainCheckBox();      // golden main.h:887 (TCheckBox*) -- offline Checked=false
    StringGrid2      = new TStringGrid(8, 60);    // golden main.h:490 (TStringGrid*); ColCount=8 / RowCount=60
                                                  //   verbatim from golden main.dfm:15447 / :15451 -- the
                                                  //   vclcompat default 5x5 would make asendic_Color.cpp:831's
                                                  //   Cells[3][38] throw std::out_of_range
    RENESAS_Server   = new TfMainRENESASServer(); // golden main.h:1710 (TRENESAS_Server*)
    // AI(W906-W7-L2) 20260803: allocations for the two W7-L2 widget members
    // (golden main.h:102/:103 `TBtnPanel *BtnSTEP; TBtnPanel *BtnT_Start;`) that
    // golden ckernel.cpp's WaitManualStepKey/WaitManualStartKey write ->Color on.
    // See forms/fMain.h for the per-member citations and the TfMainPanel
    // type choice.
    //
    // THE INITIAL Color IS GOLDEN'S OWN, NOT A GUESS -- and it is set here
    // explicitly because vclcompat::TPanel defaults Color to 0 (clBlack,
    // vclcompat/Controls.h:245), which is a value neither button ever holds in
    // golden.  Golden's design-time value comes from the form resource, read
    // this pass: main.dfm:10812 `object BtnSTEP: TBtnPanel` carries
    // `Color = 8404992` at :10819, and main.dfm:10834 `object BtnT_Start:
    // TBtnPanel` carries the same `Color = 8404992` at :10841.
    // 8404992 == 0x00804000 -- byte-identical to the literal ckernel.cpp:71 /
    // :113 write, i.e. golden ships both lamps already at their OFF colour.
    // (Both .dfm blocks also set `TrueColor = clYellow` / `FalseColor = 8404992`
    // -- :10829-10830 and :10851-10852 -- so the TBtnPanel's own Down-latch
    // state machine agrees with what ckernel writes by hand.  Independently
    // corroborated by this tree's extracted layout table,
    // tools/dfm2rc/layout_out/main_layout.gen.cpp:603-604, which records
    // Color 8404992 / TrueColor 65535 (clYellow) / FalseColor 8404992 for both.)
    // Golden's TBtnPanel CONSTRUCTOR leaves Color at clBtnFace
    // (elec\myvcl\butPa1.cpp:33, via the shadow-local bug documented in
    // vclcompat/BtnPanelCore.h:101-122), but .dfm streaming overwrites that
    // before the form is ever shown, so 0x00804000 -- not clBtnFace -- is the
    // value a running Handler observes before ckernel first writes it.
    // Setting a non-default in the ctor follows the precedent already set for
    // fLotInfo->palRemoveTray (forms/fLotInfo.cpp restores its true/true).
    //
    // BRANCH SELECTION: this default selects NOTHING.  Both members are
    // write-only in golden (the four ckernel sites are all assignments; nothing
    // in golden or in this tree reads BtnSTEP->Color or BtnT_Start->Color), so
    // no arm of ckernel's logic turns on it -- unlike, say, cInplace's
    // InArmPlacementEnable()==false.  It matters only to the test plan, which
    // asserts on the colour.  CAVEAT for whoever writes those assertions: because
    // golden's initial value and golden's lamp-OFF write are the SAME number
    // (0x00804000), asserting `Color==0x00804000` cannot distinguish "never
    // written" from "written OFF"; only the clYellow (lamp-ON) transition is a
    // sharp assertion.
    BtnSTEP    = new TfMainPanel();               // golden main.h:102 (TBtnPanel*)
    BtnT_Start = new TfMainPanel();               // golden main.h:103 (TBtnPanel*)
    BtnSTEP->Color    = 0x00804000;               // golden main.dfm:10819 (8404992)
    BtnT_Start->Color = 0x00804000; W906_InitMainStatusDfm();  W906_TcpServersCreate(this);   // golden main.dfm:10841 (8404992)；AI(W906-MSTATE-P2) 20260924: 後面那個呼叫見檔尾（接在同一行，不移動其後行號）  AI(W906-W10) 20260927 (St02-E): golden main.h:121-122 TeraTCPResultServer / TCPCommandServer (Sim, the dfm bindings; this file's end).  Same line, no line below moves  AI(W906-W10fix) 20260928 (St02-E, laptop-approved claim): the call used to sit behind this comment (dead)
}
void TfMain::LightOn() {}                                       // W6.4: CCD light sink (offline no-op)
void TfMain::DebugOneCycleHotPlate(AnsiString /*sfunc*/) {}     // debug log sink (offline no-op)
// AI(W906-W7-F1fix2) 20260729: Pause() keeps its golden-faithful offline return
// (false -- offline never pauses) but now records that it RAN and with WHAT Func.
// Without this, every forward INTO Pause() (BtnPauseClick's
// Pause("BtnPauseClick"), golden main.cpp:6967, plus ~40 SM call sites) is
// completely unobservable, so no test can distinguish "forwarded" from
// "silently dropped the call". Zero behavioural change: the return value and the
// absence of any real pause are untouched.
bool TfMain::Pause(AnsiString Func) { W906_PauseCallCount++; W906_PauseLastFunc = Func; return false; }
void TfMain::ShowTestHeadComp(bool /*bRefresh*/) {}
void TfMain::ReStartAutoSiteMapping(bool /*bStart*/) {}
// ---------------------------------------------------------------------------
// AI(W906-AutoSiteMapCleanOut) 20260727: TfMain::SetMainRunStartMode -- GAP-
// DOCUMENTED no-op stub, added solely so csystem.cpp's InitCleanOutFunction
// AutoSiteMap branch (golden csystem.cpp:15751-15785) could be un-gated.
//
// Golden's REAL SetMainRunStartMode (main.cpp:28236-28308, ~72 lines) is NOT
// translated here -- it is out of scope for that small wave:
//   * it dereferences fLotInfo->cbRunMode (Visible/Text.Pos), fBinSel (an
//     entirely new VCL form, ->cbUseMRTMode -- no facade member exists for it),
//     and this TfMain's own edSetOpenBin/lbSetOpenBin/cbRunStartMode/
//     cbbRunModeSel (none of which have a facade home yet);
//   * every branch ends by calling SetRunStartMode() (golden's *different*,
//     already-real function -- NOT called from this stub) and then
//     unconditionally calls UpdateMainOperateMode() (main.cpp:12803-13127,
//     ~325 lines), which walks a real hardware relay/IO ladder (ATC site-use
//     relays, edWorkTemperBase/edSoakTime enable-locks, WriteLastDataFile /
//     ReadLastDataFile, ChangeATCSiteUse) -- none of that surface exists in
//     this ported tree.   [AI(W906-OPMODE) 20260926: outdated (NB2 R72 OPM-4) -- the body is translated in forms/fMain_OperateMode.cpp and runs through the :507 hook once wb_serve installs it]
// Per this project's established "extend only what's read, stub what's out
// of scope" convention (see ShowTestHeadComp/ReStartAutoSiteMapping just
// above, same class), this is intentionally a documented no-op: the
// InitCleanOutFunction call site only needs the CALL to resolve. Nothing
// currently functioning is lost by this stub for THIS call site specifically,
// because golden's real SetMainRunStartMode always ends by calling
// SetRunStartMode() -- which is ALREADY a separate no-op stub elsewhere in
// this tree (aHotPlateSubstrate.cpp:764) -- so the mode-transition cascade it
// would drive is already inert here regardless. (bSiteMappingCHKOK/
// SiteMapData-zero/bAutoSiteMapHotplateSave are set directly by
// InitCleanOutFunction's own body, independent of this call -- but
// iAutoSiteMapCount is NOT: it is only touched by ReStartAutoSiteMapping,
// golden main.cpp:28166-28180, called from SetMainRunStartMode's
// iSetMode==rsmAutoSiteMap branch only, main.cpp:28283-28306 -- a branch this
// call site never reaches, since iSetMode here is always rsmContinuStart or
// rsmContinuRetest, csystem.cpp:274/276.)
// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------
//  AI(W906-P10) 20260921: SetMainRunStartMode 的相依 —— **刻意用兩個 include
//  ＋ 兩個補宣告**，不是全部 include。
//
//  ⛔ 為什麼不 include `aHotPlateSubstrate.h` 拿 `SetRunStartMode`：
//    它在 :892 宣告 `InArmLeftSideNoIC(int iRow=2)`，而 `ainarm9045.h:133`
//    也宣告同一支同樣帶預設值 —— 本檔（透過既有 include）已經收到後者，
//    再收前者就是 `default argument given for parameter 1` 硬錯誤。
//    ⇒ 只補**那一個**宣告，簽名逐字抄 aHotPlateSubstrate.h:929（P10(1) 改過的 golden 簽章）。
//
//  ⚠ `LastSet` 則是單一出處（LastSet.h:587 `extern LAST_GENERAL_SET LastSet;`），
//    型別名不好手抄，直接 include 那個 header。
// ---------------------------------------------------------------------------
#include "forms/fLotInfo.h"                // fLotInfo->cbRunMode（Visible / Text.Pos）
#include "forms/fBinSel.h"                 // fBinSel->cbUseMRTMode
extern void SetRunStartMode(eRunStartMode Mode = rsmNull, AnsiString ModeText = "");   // aHotPlateSubstrate.h:929
#include "LastSet.h"                      // LAST_GENERAL_SET LastSet（LastSet.h:587）

// ===========================================================================
//  ★ AI(W906-P10) 20260921: 樁退休，真本體翻進來（golden main.cpp:28236-28308，73 行）。
//
//  ⛔ 上面那段 20260727 的橫幅**已經過期**，逐條對照：
//
//   (a) 它說「the mode-transition cascade it would drive is already inert here
//       regardless, because golden's real SetMainRunStartMode always ends by
//       calling `SetRunStartMode()` -- which is ALREADY a separate no-op stub
//       (aHotPlateSubstrate.cpp:764)」。
//       ⇒ **那個前提死了**：20260921 的 P10(1) 把 `SetRunStartMode` 翻成
//         真本體（RunStartMode.cpp，golden main.cpp:363-1115，753 行），
//         舊樁已退休。cascade 不再是 inert 的。
//
//   (b) 它列的缺口裡有「this TfMain's own edSetOpenBin / lbSetOpenBin /
//       cbRunStartMode / cbbRunModeSel (none of which have a facade home yet)」。
//       ⇒ **四個都有家了**（P10(1) 補進 forms/fMain.h 的同名區塊）。
//
//   (c) 它說 `fLotInfo->cbRunMode` / `fBinSel->cbUseMRTMode` 沒有門面成員。
//       ⇒ 實測兩個門面都在（forms/fLotInfo.h / forms/fBinSel.h），本檔補 include 即可。
//
//  ⚠ 仍然**沒有**變的一項：`UpdateMainOperateMode()`（golden main.cpp:12803-13127，
//    約 325 行）在本樹還是樁（本檔 :381）。所以每個分支尾端那一句照翻、照呼叫，
//    但它目前不會走那條硬體 relay/IO 階梯。**那是下一個波次，不是這一顆的缺陷。**　［AI(W906-OPMODE) 20260926：這一句已過期（NB2 R72 OPM-4）——本體已翻（forms/fMain_OperateMode.cpp），wb_serve 開機裝 hook 之後會真的走 relay／ATC／lastdata；ctest 沒裝 hook＝仍只計數］
// ===========================================================================
void TfMain::SetMainRunStartMode(int iSetMode)
{
    if(TestIF_File.iTestMode==SingleSite)                                       //Steven 20120912 : Single Site不需要作Auto Site Mapping
    {
        bSiteMappingCHKOK=true;
        if(LastSet.iRunStartMode==rsmAutoSiteMap)
        {
            if(fLotInfo->cbRunMode->Visible==true &&                            //Steven 20240220 : Auto Site Map結束後, 增加檢查cbRunMode來切換模式
               fLotInfo->cbRunMode->Text.Pos("FT")>0)
            {
                SetRunStartMode(rsmContinuStart);
            }
            else if(iAutoSiteMapRunStartMode==1 ||                              //Steven 20240131 : Auto Site Map結束後, 增加檢查cbRunMode來切換模式
                    (fLotInfo->cbRunMode->Visible==true &&
                     fLotInfo->cbRunMode->Text.Pos("RT")>0))
            {
                SetRunStartMode(rsmContinuRetest);
            }
            else
            {
                SetRunStartMode(rsmContinuStart);
            }
        }

        if(CUSTOMER_CODE==CC_KYEC_LEE && iSetMode==rsmContinuStart)
            SetRunStartMode(rsmContinuStart);
    }
    else if(iSetMode==rsmContinuStart || iSetMode==rsmContinuRetest)            //Richard 20230424 : 修正RT mode下auto site mapping重複做
    {
        bSiteMappingCHKOK=true;

        if(fLotInfo->cbRunMode->Visible==true &&                                //Steven 20240220 : Auto Site Map結束後, 增加檢查cbRunMode來切換模式
           fLotInfo->cbRunMode->Text.Pos("FT")>0)
        {
            SetRunStartMode(rsmContinuStart);
        }
        else if(iAutoSiteMapRunStartMode==1 ||                                  //Steven 20240131 : Auto Site Map結束後, 增加檢查cbRunMode來切換模式
                (fLotInfo->cbRunMode->Visible==true &&
                 fLotInfo->cbRunMode->Text.Pos("RT")>0))
        {
            SetRunStartMode(rsmContinuRetest);
        }
        else
        {
            SetRunStartMode(rsmContinuStart);
        }
    }
    else if(iSetMode==rsmAutoSiteMap)
    {
        edSetOpenBin->Text="";
        if(CosFunction.bUSEJCETSiteMapMode==true)                               //jou 2016-10-28 JCET 要求Site Mapping 必須測試到pass bin才能通過
        {
            lbSetOpenBin->Visible=false;
            edSetOpenBin->Visible=false;
        }
        else
        {
            lbSetOpenBin->Visible=true;
            edSetOpenBin->Visible=true;
        }

        bSiteMappingCHKOK=false;
        SetRunStartMode(rsmAutoSiteMap);
        ReStartAutoSiteMapping(true);
        if(IniConfig.bI21EnableASM)                                             //Steven 20120207 : 只有Auto Site Mapping啟動時才要強制disable
        {
            cbRunStartMode->Enabled=false;
            cbbRunModeSel->Enabled=false;
            fBinSel->cbUseMRTMode->Enabled=false;                               //Ifor 20170414 (wei) add 鎖定 mrt 模式不可修改
        }
    }
    UpdateMainOperateMode();
}
// AI(W906-CLEANOUT) 20260924: CleanOut 的空殼 {} 移除 —— 照 golden main.cpp:4269-4330 翻好的本體在 cCleanOut.cpp（ht9045_sm）
void TfMain::DoStateRecord(int i, bool b) { if (W906_StateRecordBody != 0) W906_StateRecordBody(this, i, b); }  // AI(W906-STATEREC) 20260924: 本體 cStateRecord.cpp（golden main.cpp:26340-26678），wb_serve 開機安裝（fMain.h 檔尾）；沒裝 = 原本的 no-op
// -- W6.5 ADD: shuttle-engine method sinks (all offline no-op) --
// AI(W906-LOGSINK) 20260927: TfMain::AddShuttleMessage -- the golden body is at this file's EOF now (was the no-op sink `{}` on this line)
void TfMain::Reset(AnsiString /*Func*/) {}
void TfMain::BtnOneCycleClick(void * /*Sender*/) {}
void TfMain::BtnResetClick(void * /*Sender*/) {}
// AI(W906-CLEANOUT) 20260924: BtnCleanOutClick 的空殼 {} 移除 —— golden main.cpp:4264-4267 的本體在 cCleanOut.cpp（ht9045_sm）
void TfMain::JSCC_ResetForShuttleLoseIC() {}
void TfMain::ResetRecordforPiggyBack(AnsiString /*S*/) {}
// AI(W906-SENSORSCAN) 20260924: TfMain::ProcessSensorScan() 的本體搬到 cSensorScan.cpp（golden main.cpp:13949-14305；要 Sen[]／RecordProcess 等 ht9045_sm 層符號，不能留在 ht9045_forms）。原本這裡是 W6.6 的空函式。
// -- W7-C1 ADD: end-of-lot clean-out finish-check fMain methods (all offline no-op) --
void TfMain::Start(AnsiString /*Func*/) {}                     // W7-C1: offline do NOT auto re-start
void TfMain::ChangeLevelAttr() {}                              // W7-C1: offline level-attr UI no-op

// AI(W906-FW1d) 20260820: golden main.cpp:15127-15148, faithful -- clamp
// AccessLevel into [0, iDefHonPrecLevel], then mirror it into
// cbUserSelect->Text via golden's three name tables (5-level vs 4-level
// security, with the CC_KYEC_LEE variant spelling). Golden's ChangeLevelAttr
// calls this first (main.cpp:12410); the facade's ChangeLevelAttr above stays
// a no-op, so callers needing the mirror call this directly (see fMain.h).
void TfMain::DoChangeLevel()
{
    AnsiString str[4]    ={"Operator", "Engineer", "Supervisor", "HonPrec"};
    AnsiString cLevel[5] ={"Open", "Operator", "Engineer", "Supervisor", "HonPrec"};
    AnsiString cLevel2[5]={"Operator", "Engineer", "PEngineer", "Supervisor", "HonPrec"};

    if(AccessLevel<=0)
        AccessLevel=0;
    if(AccessLevel>=iDefHonPrecLevel)                                           //jou 2014-06-19
        AccessLevel=iDefHonPrecLevel;                                           //jou 2014-06-19

    if(CosFunction.bSecurityHave5Level==true)                                   //jou 2014-06-19
    {
        if(CUSTOMER_CODE==CC_KYEC_LEE)                                          //wei 20160505
            cbUserSelect->Text=cLevel2[AccessLevel];
        else
            cbUserSelect->Text=cLevel[AccessLevel];
    }
    else
    {
        cbUserSelect->Text=str[AccessLevel];
    }
}
void TfMain::ModifyTester(int iMode) { extern void SetTestRunMode(); LastSet.iTester=iMode; TestMode.iTestConnection=LastSet.iTester; SetTestRunMode(); LoadTestModePicture(); }   // AI(W906-W2-SETTESTRUNMODE) 20260926: 空殼換成 golden main.cpp:12056-12062 本體（4 行併成一行以免位移行號）；SetTestRunMode 在 RunStartMode.cpp 檔尾，LoadTestModePicture 仍是畫面空殼（圖示歸網頁）
void TfMain::CleanYieldCount() {}                              // W7-C1: offline yield-count clear no-op
// -- W5-Automation ADD: HANA_ART.cpp method sinks (all offline no-op / empty) --
void TfMain::SendMSG_CMD(int CMD) { if (W906_TesterForward.SendMSG_CMD != 0) W906_TesterForward.SendMSG_CMD(CMD); }   // AI(W906-GB-P2b) 20260926: golden main.cpp:18734 body is THandlerTesterSide::SendMSG_CMD (TesterComm/Handler), installed by W906_TesterCommInit (fMain.h file end); not installed = the old offline no-op
void TfMain::SendMSG_CMD(int CMD, AnsiString Message) { if (W906_TesterForward.SendMSG_CMD_Msg != 0) W906_TesterForward.SendMSG_CMD_Msg(CMD, Message); }   // AI(W906-GB-P2b) 20260926: as above (golden main.cpp:18790)
// AI(W906-FW3-WD-integrate) 20260818: GetSamSungMap / GetSamSungSoakTime
// offline stubs RETIRED -- real bodies (golden Command.cpp:10137-10303 /
// :10305-10322) landed in Command.cpp with FW-3 Wave D; Command.cpp is
// already registered in ht9045_sm, so keeping these would be a
// multiple-definition link error, not a fallback.
// AI(W906-FW3-WA) 20260817: ArmStatusStrings stub RETIRED -- the real body
// (golden Command.cpp:1497-1508) landed in Command.cpp with FW-3 Wave A.
// -- W5-Final-SckArtRemainder ADD: method sinks (all offline no-op) --
// AI(W906-SSMD) 20260927: TfMain::SetStartModeData 的空殼退役 —— golden main.cpp:24118-24234 照翻在 RunStartMode.cpp 檔尾
void TfMain::LoadTestModePicture() {}                                       // offline: test-mode picture UI refresh no-op
bool (*W906_SaveRunModeBody)() = 0; void TfMain::SaveRunMode() { if (W906_SaveRunModeBody != 0) W906_SaveRunModeBody(); }  /* AI(W906-PROD-S95R) 20260926 (Steven 團隊)：golden main.cpp:33648 TfMain::SaveRunMode（寫 d:\HT9045\system\RunMode.txt 一行 RunMode=%d）；本體 FileRW/MainClose.cpp W906_TfMain_SaveRunMode（只編進 wb_serve），wb_serve 開機 W906_FRW_InstallSaveRunMode() 裝上；沒裝（每一個 ctest）＝原本 fMain.h:321 的 inline no-op。跟 S92 同一行，不移動行號 */  void (*W906_BackupSetupFileBody)() = 0; void TfMain::BackupSetupFile() { if (W906_BackupSetupFileBody != 0) W906_BackupSetupFileBody(); }   // AI(W906-FRW-S92) 20260926: golden main.cpp:34053 body is FileRW/MainBackup.cpp (wb_serve only), installed by W906_FRW_InstallBackupSetupFile() at wb_serve boot; not installed (every ctest) = the old offline no-op
// -- W906-Automation ADD: golden main.h:1276 `bool __fastcall Home(AnsiString Func);`
//    (kevin 20141108) -- runs a full motor Home cycle and reports success/
//    failure. Offline: no real motors to home, so there is nothing to
//    actually succeed -> false (same "offline never succeeds a hardware
//    cycle" posture as Pause() above). Only current caller in the
//    translated tree is auto9045.cpp's `#ifdef DEBUG_DUTONOFF` DoHomeAndStart
//    (compiled out, DEBUG_DUTONOFF undefined) plus the still-GATED
//    Automation/automation.cpp ProcessBuffer (golden :1522).
//AI(W906-HOME-W1) 20260919: 這裡原本是
//     bool TfMain::Home(AnsiString) { return false; }
// **第三個會說謊的樁**（前兩個是 TfLotInfo::SetLotID / SetLotStart，
// 已於 P0-2 換掉）。真本體在本檔尾端的 W906-HOME-W1 區塊。
// -- W906-AutoCleanFoundation ADD: golden AutoClean.cpp AddAutoCleanMessage
//    sink -- offline no-op log sink, same idiom as AddShuttleMessage/CleanOut.
// AI(W906-LOGSINK) 20260927: TfMain::AddAutoCleanMessage -- the golden body is at this file's EOF now (was the no-op sink `{}` on this line)
// AI(W906-FW3-WA) 20260817: WritePERSITETemperature (faithful wrapper) and
// PERSITETemperatureStrings (sim-seam stand-in) both RETIRED -- their real
// bodies (golden Command.cpp:935-943 / :945-1482) landed in Command.cpp with
// FW-3 Wave A; homecoming per the B4 precedent (golden home wins). The
// W906_PERSITETemperatureStrings_Sim field stays declared on the class but
// is no longer read by anything -- tests that seeded it were recalibrated
// the same commit (tests/test_testertcp_socket.cpp [T-F]).
// -- W7-F1 ADD: "Wall 2" facade method bodies -- see forms/fMain.h for the
//    full per-member golden citations and offline-default rationale.
void TfMain::cbSetupFileNameChange(void * /*Sender*/) { W906_cbSetupFileNameChangeCallCount++; }
// AI(W906-SJSON-S11) 20260923: 不再是 no-op。
//   本體在 JsonBridge/actions/MainClarnData.cpp（golden main.cpp:15458-15648 的
//   逐字翻譯），經 W906_ClarnDataBody 掛進來 —— 理由與影響範圍全寫在
//   forms/fMain.h 的安裝座註解。
//   ⚠ 呼叫計數器**先加再分派**：本體可能拋例外（WriteLastDataFile 內有 try/catch，
//     但 TMyStringList 的檔案操作沒有），計數器記的是「進來過」不是「做完了」，
//     這樣 W906_Clarn_DataCallCount 仍然是一個可信的「有沒有被叫到」證據。
W906_ClarnDataBodyFn W906_ClarnDataBody = 0;
void TfMain::Clarn_Data(int Tag, AnsiString Msg)
{
    W906_Clarn_DataCallCount++;
    if (W906_ClarnDataBody != 0) W906_ClarnDataBody(Tag, Msg);
}
void TfMain::BtnPauseClick(void * /*Sender*/) { W906_BtnPauseClickCallCount++; Pause("BtnPauseClick"); }   // TRANSLATED:
                                    // golden's own first body line (main.cpp:6967, inside the function at :6965).
                                    // Two independent seams cover this one line: W906_BtnPauseClickCallCount
                                    // (AI(W906-W7-F1fix) 20260729) proves BtnPauseClick RAN;
                                    // W906_PauseCallCount/W906_PauseLastFunc (AI(W906-W7-F1fix2) 20260729) prove the
                                    // FORWARD happened and carried golden's own "BtnPauseClick" argument -- the
                                    // call-count seam alone cannot see the forward at all.
void TfMain::LoadRunModePicture() { W906_LoadRunModePictureCallCount++; }
bool TfMain::CanChangeSite(bool /*bNoIncludeHotplate*/) { return W906_CanChangeSite_Sim; }
void TfMain::BtnTrayEndClick(void * /*Sender*/) { W906_BtnTrayEndClickCallCount++; }
void (*W906_ChangeATCSiteUseHook)(TfMain*) = 0;  void (*W906_UpdateMainOperateModeHook)(TfMain*) = 0;  void TfMain::UpdateMainOperateMode() { W906_UpdateMainOperateModeCallCount++; if (W906_UpdateMainOperateModeHook != 0) W906_UpdateMainOperateModeHook(this); }   // AI(W906-OPMODE) 20260926: golden main.cpp:12803-13127 的本體在 forms/fMain_OperateMode.cpp（TfMain::W906_UpdateMainOperateModeBody，ht9045_sm）；本檔在 ht9045_forms 不能直接呼叫 ⇒ 經 hook（wb_serve 開機裝）。沒裝＝原本的計數樁（每一支 ctest），不切加熱器繼電器、不送 ATC、不寫 lastdata
void TfMain::LoadStartModePicture() { W906_LoadStartModePictureCallCount++; }
void TfMain::LookForFile() { W906_LookForFileCallCount++; }
// TfMain::ChangeTesterConnect -- AI(W906-GB-P2d) 20260926: the stand-in that stood here is replaced by the golden body at the end of this file (the W906_ChangeTesterConnect_Sim seam is kept there)
int  TfMain::SetTemp(bool /*bAsk*/, double /*fWorkTemp*/, double /*fSoakTime*/) { return W906_SetTemp_Sim; }
void TfMain::ChangePassword() { W906_ChangePasswordCallCount++; }
int  TfMain::FTClick(bool /*bMan*/) { return W906_FTClick_Sim; }
int  TfMain::RTClick(bool /*bMan*/) { return W906_RTClick_Sim; }
// ---------------------------------------------------------------------------
// AI(W906-W7-L2) 20260803: TfMain::MainFormChange -- golden main.h:1261
// (`void __fastcall MainFormChange();`), body golden main.cpp:3883-4096.
// Offline no-op, same shape as ProcessSensorScan (this file :272) /
// ChangeLevelAttr (:275) above.
//
// WHAT THE REAL BODY DOES, AND THEREFORE WHAT IS ELIDED HERE (all 214 golden
// lines read this pass, not summarised from a recon): it repaints the form's
// site-map LED matrix for the current TestIF_File.iTestMode and nothing else.
// It hides all 16x8 TALed pointers in a local Ptr[][] array (built golden
// main.cpp:3890-3914, cleared :3922-3924) plus the 2x8 9046AU sort-shuttle
// PtrSort[][] array (:3916-3920, cleared :3926-3932); derives SingleRow/iCol
// from iTestMode across a 16-arm else-if ladder (:3934-4002); re-shows the
// matching subset (:4004-4034); applies the IsNNMode()==NN_1Row (:4036-4055)
// and ==NN_2Row (:4057-4066) corrections; and finally sets
// labFailAlarmCnt->Visible / ->Caption from IniConfig.bG04ShowFailAlarmCount
// with Prod.bContsFailBySocket / Prod.bContsFailByHead and their counters
// (:4068-4094).
//
// Every one of those writes targets a TfMain-owned VCL widget that has no
// facade home: the 144 distinct TALed members the two arrays name (16x8 + 2x8;
// e.g. golden main.h:313 `TALed *led_BLCarryKit_0;`, :889
// `TALed *led_SortShtKit_0;`) plus main.h:673 `TLabel *labFailAlarmCnt;`.
// The body writes no global and no Prod/TestIF field, and touches
// no motor/IO -- so offline the elided effect is purely cosmetic: the LED
// matrix and the fail-alarm-count label are not redrawn.  Golden's only caller
// on this front, ckernel.cpp:367, sits in ScanSystemSensor's one-shot
// `if(SoftStart==true)` startup block (:365-533 -- brace-matched this pass with
// comments and string literals masked: `if` at :365, body `{` at :366, closing
// `}` at :533, `else if(SoftStop==true)` at :534; an earlier comment in this
// same wave gave the end as :379, wrong by 154 lines -- :379 is only the
// mid-block `SoftStop=false;`) and ignores any result (the function returns
// void), so nothing downstream of that call site changes.
// ---------------------------------------------------------------------------
void TfMain::MainFormChange() {}                               // W7-L2: offline LED/label repaint no-op
// AI(W906-PT-W3-integrate) 20260808: uRENESAS_Server facade sinks -- see the
// per-member notes in forms/fMain.h for why each no-op is behaviourally honest.
// AI(W906-MSTATE-P2) 20260924: EnabledSetupFile 的空殼 {} 移除 —— 照 golden main.cpp:28310-28425 翻好的本體在 cMainStatus.cpp（ht9045_sm）
void TfMain::SetLotState(int /*iState*/) {}                    // golden main.cpp:15160-15250 (TCP/GPIB lot-state push)
TfMain *fMain = new TfMain();

// ===========================================================================
//  TfMain::RunCheckStart  --  golden main.cpp:32170-32209 (40 lines)
//
//  AI(W906-ST-W7-RCS) 20260917.  Translated to un-gate SAFETY-GATE(W906-ST-W6-D)
//  and (W906-ST-W6-E) in WebStart.cpp, which were the ONLY two blocked on it.
//
//  WHY IT LANDS HERE AND NOT AS A FREE FUNCTION.  golden calls it as
//  `fMain->RunCheckStart()` (golden csystem.cpp:10367, ported csystem.cpp:9791,
//  still inside GATE G4-1).  TfMainWeb derives from TfMain (WebStart.h:110), so
//  the two ported call sites can keep golden's unqualified spelling.  A free
//  function would have compiled and left that third call site unable to use it.
//
//  WHY THE BODY IS AT THE FILE TAIL.  forms/fMain.cpp carries 113 hand-written
//  `fMain.cpp:NNN` citations; appending moves none of them.  The tree already
//  places TfMain method bodies across many .cpp files (Command.cpp,
//  MainCalcCore.cpp, Automation/auto9045.cpp, ...), so location is free.
//
//  FAITHFULNESS -- TWO THINGS PRESERVED THAT LOOK LIKE BUGS:
//    1. golden :32183 assigns `bPhysicalStart=true;` inside a branch whose own
//       condition already requires `bPhysicalStart==true` (:32181).  It is a
//       no-op.  KEPT VERBATIM -- changing it would be inventing behaviour, and
//       the surrounding comment (JerryYang 20250912) says the branch exists to
//       stop a SECOND start sending a duplicate EVENT REPORT, which the bare
//       `return` achieves on its own.
//    2. golden :32187-32188 / :32204-32207 are author-commented-out lines
//       (Steven 20211109 "Mark掉").  Reproduced as comments, not restored --
//       the same rule the rest of this port follows.
//
//  WHAT IT DOES ON THE MACHINE.  It is a NOTIFICATION with no return value: it
//  sends one SECS/GEM event report and nothing else.  It cannot refuse a start
//  and does not touch the caller's control flow -- which is exactly why W6-D
//  and W6-E were classified 🟡 rather than 🔴.
//
//  DEPENDENCIES -- all verified present before translating (not just "a header
//  mentions them"): EventReport (SECSGEM/SecsEventReport.cpp:15, real body),
//  SECS_EVENT.UnloadComplete/.DoStart/.DoStartHasIC (SECSGEM/SecsEventType.h),
//  HasICUnderMachine (csystem.h:105, live callers e.g. AutoClean.cpp:8277),
//  IniConfig.bSPILFunction/.bRCMDStart/.bN25_1_EnableStartControl/
//  .bEnable_SECS_GEM (Config.h:133/:93/:1228/:92), bHasSaveSet (cmydef.h:3887),
//  bPhysicalStart (cmydef.h:225), CC_XINYUN / CC_ChipMos_ZHUBEI
//  (MachineType.h:157/:215).
//
//  These four headers are included HERE rather than at the file head for the
//  same reason the body is: an insertion at the top would move all 113 cited
//  lines.  A #include at namespace scope is legal C++ and all four are
//  include-guarded -- the same idiom csystem.cpp:20152 already documents.
//
//  ** NEW CROSS-LIBRARY SYMBOL EDGE -- DISCLOSED, MEASURED, NOT HIDDEN. **
//  This file's own header banner (top of this .cpp) states that ht9045_forms is
//  the BOTTOM layer and its dependency is acyclic.  This body references
//  EventReport (ht9045_secsgem) and HasICUnderMachine (ht9045_sm) -- and BOTH of
//  those libraries already depend on ht9045_forms (CMakeLists.txt:1593 / :2740).
//  So the symbol graph now has a cycle that the CMake graph does not.
//
//  WHY THAT IS OK HERE, measured rather than assumed:
//    * No target_link_libraries line was touched, so the CMAKE graph is still
//      acyclic -- the cycle is in SYMBOLS, which is exactly what
//      $<LINK_GROUP:RESCAN,...> exists to resolve, and is the same shape the
//      repo already uses for ht9045_comms (see the note under ht9045_sm's
//      target_link_libraries, which records that a direct PUBLIC edge was tried
//      and reverted because it made the LINK_GROUP graphs cyclic).
//    * All 99 RESCAN groups that contain ht9045_forms also contain BOTH
//      ht9045_secsgem and ht9045_sm -- counted, zero exceptions
//      (2 in CMakeLists.txt + 97 in tests/CMakeLists.txt).
//    * The only non-group consumers of ht9045_forms are ht9045_secsgem and
//      ht9045_sm themselves, which are archives -- nothing resolves there.
//    * -fsyntax-only CANNOT see an undefined reference (campaign trap 2), so the
//      proof of this paragraph is the DUAL GATE, not the compile.
//
//  IF A FUTURE TARGET EVER LINKS ht9045_forms WITHOUT A RESCAN GROUP, this body
//  is the first thing that will fail to link.  Moving it is cheap: any .cpp that
//  belongs to ht9045_sm ALONE would do.  Command.cpp was rejected for that role
//  because it is compiled into TWO archives (ht9045_comms AND ht9045_sm), which
//  would make this a duplicate definition.
// ===========================================================================
#include "Config.h"                        // IniConfig
#include "MachineType.h"                   // CC_XINYUN, CC_ChipMos_ZHUBEI
#include "csystem.h"                       // HasICUnderMachine (csystem.h:105)
#include "SECSGEM/SecsEventReport.h"       // EventReport(unsigned)
#include "SECSGEM/SecsEventType.h"         // SECS_EVENT

void TfMain::RunCheckStart()                                                    // golden :32170  Ifor 20151208 : 新增Run Check 副程式
{
    if(CUSTOMER_CODE==CC_XINYUN && IniConfig.bEnable_SECS_GEM==true)            // golden :32172  RogerYang 20260610 : for 芯云定義
    {
        EventReport(SECS_EVENT.UnloadComplete);                                 // golden :32174  送 102
        return;                                                                 // golden :32175  跳過後面原本送 CEID 1 的邏輯
    }

    if(IniConfig.bSPILFunction==true &&                                         // golden :32178  JerryYang 20250912 : fix二次啟動會多送EVENT REPORT
       IniConfig.bEnable_SECS_GEM==true &&                                      // golden :32179
       IniConfig.bRCMDStart==true &&                                            // golden :32180
       bPhysicalStart==true)                                                    // golden :32181
    {
        bPhysicalStart=true;                                                    // golden :32183  ⚠ no-op, see banner item 1 -- KEPT VERBATIM
        return;                                                                 // golden :32184
    }

//    if(IniConfig.bSPILFunction==true)                                         // golden :32187  Steven 20211109 : Mark掉, 避免客戶使用START當作檢查工作檔的判斷
//    {                                                                         // golden :32188  Ifor 20151208 : 矽品 Run Check 時機修改
        if((IniConfig.bSPILFunction==true ||                                    // golden :32189
            (CUSTOMER_CODE==CC_ChipMos_ZHUBEI && IniConfig.bN25_1_EnableStartControl) || // golden :32190
            (IniConfig.bRCMDStart==true && bPhysicalStart==true)) &&            // golden :32191
           bHasSaveSet==true)                                                   // golden :32192  Steven 20220601 : 針對不同客戶的回覆值不同
        {
            EventReport(SECS_EVENT.DoStart);                                    // golden :32194
        }
        else if(HasICUnderMachine()==false)                                     // golden :32196  Ifor 20151208 : 判斷是否為第一次啟動 或者 設定檔案有變更
        {
            EventReport(SECS_EVENT.DoStart);                                    // golden :32198
        }
        else
        {
            EventReport(SECS_EVENT.DoStartHasIC);                               // golden :32202
        }
//    }                                                                         // golden :32204
//    else                                                                       // golden :32205
//    {                                                                          // golden :32206
//        EventReport(SECS_EVENT.DoStart);                                       // golden :32207
//    }
}


// ===========================================================================
//  AI(W906-HOME-W1) 20260919: TfMain::Home 需要的宣告。
//
//  這一批是 g++ 直接點名的（18 個 "was not declared in this scope"）。
//  每一個符號在樹裡都是活的 —— tools/live_idents.py 量過 —— 缺的只是
//  這個 TU 看不看得到它的宣告。memory: symbol-exists-has-three-strengths
//  的第二級（能不能編）與第一級（名字存在）是兩回事。
//
//  本檔既有的慣例就是**在用到的地方就近 include**（見 :496-500 那一批），
//  所以這一批貼在 Home 的正上方而不是檔頭 —— 這樣刪掉 Home 時這批也跟著走。
// ===========================================================================
#include "canary_support.h"                // __FUNC__ / ShowErrorMessage / ShowMyMessage
#include "acatchtray_shims.h"              // NewRecordProcess
#include "LastSet.h"                       // LastSet（golden LastSet.h:587）
#include "myswitch.h"                      // SW[]（golden :7099 SW[SwServerON].On()）
#include "Motor/mymotor.h"                 // MOT[] / CheckOutArmDestory
#include "ainarm9045.h"                    // AutoCalculateInArmYClosePitch
#include "atester.h"                       // bNeedCheckRTCReport
#include "atester_ProcessCount.h"          // InitialPiggyBackFunction（:31）
//AI(W906-HOME-W1) 20260919: 本來還要 `atester_shims.h`（fContact）與
// `Automation/SCK_ART_Remainder.h`（FormHS），但那兩個符號的用處都已經加閘
// （CONTACTJOG / WINKEYBOARD），所以不留無用的重標頭。解閘時要一起加回來。
#include "MainCalcCore.h"                  // ComputeCheckOLPErrorHasErr（:404）

//AI(W906-HOME-W1) 20260919: `iInArmWaitPosition` **沒有任何標頭宣告它** ——
// 定義在 acatchtray.cpp:136，使用者各自就地 extern（前例：ainarm2.cpp:805、
// WebStart.cpp:81）。照那個前例辦，不新開一個標頭。
extern int iInArmWaitPosition;      // golden ainarm2.h:219（前例：WebStart.cpp:81）

// ===========================================================================
//  AI(W906-HOME-W1) 20260919: P0-3 歸零 —— TfMain::Home
//  golden main.cpp:6975-7118（144 行），逐行翻譯。
//
//  APPEND-ONLY：這一行以上一個字都沒動。
//
//  ⭐ 為什麼這一支是 START 的最後一塊
//  ---------------------------------------------------------------------------
//  20260919 P0-2 之後實測：`start.run` 走完 golden `Start()` 全部 1,875 行，
//  落在 `WebStart.cpp:3432`（golden :6166）——
//      if (fAllMotorHome == false) { ... Home("Home by Start"); return false; }
//  也就是 golden 自己的「沒歸零就先歸零、回 false、歸零完再按一次」。
//  而 `Home()` 是空樁 ⇒ 永遠不會歸零 ⇒ `fAllMotorHome` 永遠 false ⇒ START 永遠回 false。
//
//  ⭐ 本支的承重那兩行是 golden :7100-7101：
//        SoftStart=true;
//        iHome=1;
//  `SoftStart` 正是 20260919 P0-0 接回 `MainProc()` 的 `ScanSystemSensor()`
//  在讀的旗標（ckernel.cpp:816），而 `iHome==1` 會讓它跳過
//  `if(iHome==0 && fSetup->fShow==false)` 那一段歸零檢查（ckernel.cpp:877）
//  直接走到 `SystemStart=true`（ckernel.cpp:1015）。
//
//  ⚠⚠ 這支有 5 個活的呼叫點，落地會把 5 個**全部**武裝：
//        Automation/auto9045.cpp:2639   OLP 的 DoHomeAndStart
//        Automation/automation.cpp:1927 OLP 的 ProcessBuffer
//        SECSGEM/uHGemHT9045.cpp:5872   SECS 的 S2F42 遠端歸零
//        WebStart.cpp:3430              Home by Start（本波要的那個）
//        csystem.cpp:31303              DoAutoDecayCheck
//    也就是說：**遠端 host 從此可以讓這台機器歸零並進入 SystemStart。**
//    那是 golden 的行為，不是本波新增的通道。依
//    docs/PLAN_START_TO_RUN.md §0.5（「加閘的唯一合法理由是相依不存在」）
//    與使用者 20260919 A3 裁決（「不用讓我停下來看，直接執行」）照翻。
//
//  ---------------------------------------------------------------------------
//  相依實測 —— 我量了兩次，第一次是錯的
//  ---------------------------------------------------------------------------
//  第一次用全樹 `git grep`：96 個識別字、8 個 0 命中、7 個是同一個功能
//  （TRegistry），結論「**一個閘**」。
//
//  ⚠ 那是**死對照**：`git grep` 把全樹所有 `#if 0` 區塊算成命中，包括
//    WebStart.cpp 自己已經閘掉的那些。改用 `tools/live_idents.py`
//    （對照面 = 全樹扣掉所有 `#if 0` 區塊）重量：
//
//        target identifiers     : 97
//        MISSING from live tree : 11
//          TRegistry RootKey OpenKey CloseKey HKEY_LOCAL_MACHINE Regedit
//          NowRegedit                     <- 同一個功能，一個閘
//          bDoCheckCPUName                <- golden :6997 的區域 static，本體自帶
//          BtnHome                        <- **新的**，只在 dfm2rc 版面表裡
//          sbEngSite                      <- **新的**，WebStart.cpp:1431 已有先例閘
//          tPSM                           <- **新的**，WebStart.cpp:3081 已有先例閘
//
//  ⇒ 不是一個閘，是**四個**。三個是第一次量漏的。
//    memory: mention-vs-carry-and-dead-controls（第三種死對照）。
// ===========================================================================
bool TfMain::Home(AnsiString Func)                                              // golden main.cpp:6975
{
    //==> Eastsun 20260512 F011 整合 (F4-T5 KYEC LEE KLT warning)
    if(CUSTOMER_CODE==CC_KYEC_LEE && bEnable_KLT_Function==false)
    {
        if(TrayForm.bEnableAMR==false)
            ShowMyMessage("Please check if the Tray arm is above the Empty or Color Tray!!", "請確認Tray手臂是否在空盤或色盤上方!!");
    }
    //<== Eastsun 20260512 F011 整合 (F4-T5 KYEC LEE KLT warning)

    if(SoftStart==true)
        return false;

    NewRecordProcess("MES2112", "HOME pressed", Func);
    AutoCalculateInArmYClosePitch(true, false);                                 //Steven 20190314 : 自動計算InArm Y軸收合Pitch
    bCheckGiveWay=false;                                                        //Ifor 20200521 Fix:
    bNeedCheckRTCReport=false;                                                  //Ifor 20200521 Fix:
    bASMFinishOneCycle=false;
    //AI(W906-HOME-W1) 20260919: GATE (W906-HOME-W1-ENGSITE) —— 缺相依：
    // `sbEngSite`（第三組工程師用的開關 SITE 按鈕）在**活的樹**裡 0 命中。
    // 同一個缺件已經有先例閘：WebStart.cpp:1431 `SAFETY-GATE(W906-ST-W1-J)`。
    // 行為：歸零時不會把那顆按鈕彈起來。純 UI，不影響機台動作。
    // UN-GATE：等 forms/fMain.h 帶進 sbEngSite。
#if 0 // GATE (W906-HOME-W1-ENGSITE): 缺相依 sbEngSite（活的樹 0 命中；同 W906-ST-W1-J）
    sbEngSite->Down=false;                                                      //Alick 20160926 add
#else
    { }                                                                         // GATE (W906-HOME-W1-ENGSITE)
#endif // GATE (W906-HOME-W1-ENGSITE)
    bSiteUseEE=false;
    ShowTestHeadComp(false);
    bAutoCleanFinishOnlyUseRTC=false;                                           //JerryYang 20161216
    static bool bDoCheckCPUName=true;
    iInArmWaitPosition=0;                                                       //Ifor 20210209 add:
    //AI(W906-HOME-W1) 20260919: GATE (W906-HOME-W1-CONTACTJOG) —— 缺相依：
    // `plIndexArmJogMove` 的**宣告是有的**（forms/fContact.h:1145，TPanel），
    // 但全域 `fContact` 指向的是 `TfContactShim`（atester_shims.h:251），
    // 而真的 `class TfContact` 全樹沒有任何實例（forms/fContact.h:775 自己量過：
    // 「1 hit, this file -- i.e. still nothing outside it」）。
    // 缺的是**活的實例**，不是型別。
    // 行為：Contact 畫面上的 Index 手臂 jog 面板在歸零時不會被隱藏。純 UI。
    //   ⚠ golden 這一行的用意是防呆（Ifor 20210713：避免 Contact 後回 Home 時
    //     人員按下按鈕讓 Index 撞 Shuttle）。閘掉它**在有真 fContact 的建置上
    //     會是實體風險**；在這棵樹沒有，因為那個面板根本不存在。
    //   ⛔ 解閘條件寫在 UN-GATE：真的 TfContact 有實例時必須同時解這一行。
    // UN-GATE：等 forms/fContact.h 的 TfContact 真的被建構並接上全域 fContact。
#if 0 // GATE (W906-HOME-W1-CONTACTJOG): 全域 fContact 是樁型別，沒有這個 widget
    fContact->plIndexArmJogMove->Visible=false;                                 //Ifor 20210713 add:
#else
    { }                                                                         // GATE (W906-HOME-W1-CONTACTJOG)
#endif // GATE (W906-HOME-W1-CONTACTJOG)
    bIndexPickUpErrorWaitRetry=false;                                           //Ifor 20171119 (Steven) : add 避免Index Pick Up Err Inarm 偷跑造成資料異常導致Hangup
    bChangeTest_TempAlarm=true;                                                 //Ifor 20210623 add: Test Temp Change    //Ifor 20230505 add: flag=true & Offset=0 重設溫度
    bChangeTest_TempOffset=0;                                                   //Ifor 20230505 add:清除資料
    //AI(W906-HOME-W1) 20260919: GATE (W906-HOME-W1-WINKEYBOARD) —— 缺相依：
    // `TFormHS::CloseWindowsKeyboard()` 的宣告是有的（forms/fHS.h:722），
    // 但全域 `FormHS` 指向 `W5SckArtRem_FormHSStub`
    // （Automation/SCK_ART_Remainder.h:629）。forms/fHS.h:65 自己就寫著
    // 「*** THE GLOBAL `FormHS` IS ALREADY TAKEN -- THIS FILE DOES NOT CLAIM IT ***」。
    // ★ 行為 delta = 0：forms/fHS.h:722 那個真方法**本身也是閘住的**
    //   （`// golden :4051-4054 GATE (Cat H)`，golden 的本體是
    //    `WinExec("taskkill.exe /im ...")`）。就算接上真實例，今天也不會關鍵盤。
    // UN-GATE：等 TFormHS 有實例，**而且** fHS.h:722 的 Cat H 閘也解掉。
#if 0 // GATE (W906-HOME-W1-WINKEYBOARD): 全域 FormHS 是樁型別；真方法本身也還閘著
    FormHS->CloseWindowsKeyboard();                                             //Ifor 20190920 : add 關閉 Windows 小鍵盤
#else
    { }                                                                         // GATE (W906-HOME-W1-WINKEYBOARD)
#endif // GATE (W906-HOME-W1-WINKEYBOARD)

#ifndef SOFT_SIMULTE
    if(CUSTOMER_CODE==CC_SIGURD_HUKOU)                                          //KaiChen 20191128 ：矽格-湖口，軟體重啟時檢查所有Tray
    {
        if(bCheckTrayBySoftWareOpen==false)
        {
            if(CheckFixTray()==false)
            {
                return false;
            }
        }
    }
#endif

    //Isaac 20170509 (Steven) 卡CPU資訊
    //==>
    //AI(W906-HOME-W1) 20260919: GATE (W906-HOME-W1-CPUNAME) —— 缺相依：
    // `TRegistry` / `HKEY_LOCAL_MACHINE` / `RootKey` / `OpenKey` / `CloseKey`
    // 全樹 0 命中 —— vclcompat 沒有移植 Windows 登錄檔包裝。
    // ⚠ 行為：`bReadAndCheckCPUName` 不會被設定，保持它的初值。
    //   golden 用它做的是「認機器」（比對 CPU 型號字串），
    //   **不是**啟動許可 —— 這一段沒有任何 `return false`。
    // UN-GATE：等 vclcompat 落地 TRegistry，或整合者決定用 Win32 API 直翻。
#if 0 // GATE (W906-HOME-W1-CPUNAME): 缺相依 TRegistry（vclcompat 未移植登錄檔包裝）
    if(bDoCheckCPUName==true)
    {
        bDoCheckCPUName=false;
        TRegistry *Regedit = new TRegistry();
        Regedit->RootKey=HKEY_LOCAL_MACHINE;
        Regedit->OpenKey("HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0", false);
        String NowRegedit=Regedit->ReadString("ProcessorNameString");
        if(NowRegedit=="Intel(R) Core(TM) i5-4590S CPU @ 3.00GHz")
            bReadAndCheckCPUName=true;
        else
            bReadAndCheckCPUName=false;
        Regedit->CloseKey();
        delete Regedit;
    }
#else
    (void)bDoCheckCPUName;   // 只為了讓 -Wunused 閉嘴；golden 的宣告照留
#endif // GATE (W906-HOME-W1-CPUNAME)
    //<==
    //Isaac 20170509 (Steven) 卡CPU資訊
    if(IniConfig.bG06HomeinitialCheckZ1 && bHomeinitialCheckPushZ1)             //kevin 20131218
    {                                                                           //確認 機台是否有tray 按 z1取消
        ShowErrorMessage("MES1699", 0, MMSystem, false, "Main--Home");          //"與測試機的連線尚未完成"      //Steven 20150204 : MES2010 --> MES1699
        return false;
    }

    if(IniConfig.bG22NoticeTakeoutTray && bContactModeNeedOpenDoor)             //JerryYang 20231218 : G22提醒人員取tray功能
    {
        ShowMyMessage("Please open SafeDoor3 and ensure there are no foreign objects on the hot plate.", "請打開安全門3確認Hotplate上無異物");
        return false;
    }

    if(IniConfig.bI01TesterFinishThenHome && LastSet.iTester==ON_LINE)          //kevin 20150721 收到測試資料才能home
    {
        if(bFinshTest==false)
        {
            ShowErrorMessage("WAR1647", 0, MMSystem, false, "Main--Home");      //"與測試機的連線尚未完成"
            return false;
        }
    }

    if(CosFunction.bOLPFunction)                                                //Sam 20230921 : Bin 設定錯誤不能啟動
    {
        //AI(W906-HOME-W1) 20260919: golden 這裡呼叫 `TfMain::CheckOLPError()`
        // （golden main.cpp:33986）。移植樹把它翻在 **TfMainWeb**（WebStart.h:230
        // 宣告、WebStart.cpp:406 本體），基底類別叫不到衍生類別的非虛擬成員。
        //
        // ⚠ 直接閘掉是錯的選擇：golden 用它**擋啟動**，閘掉 = fail-open
        //   （Bin 設定錯了照樣歸零啟動，會出壞品）。
        // ⇒ 改呼叫 TfMainWeb::CheckOLPError() 自己用的那個可攜判定
        //   `ComputeCheckOLPErrorHasErr()`（MainCalcCore.h:404，忠實保留 >=3
        //   門檻與 0..9 全掃）。**否決行為與 golden 相同。**
        //
        // ⚠ 三個 `ShowMyMessagePWD` 對話框在這裡同樣沒有 —— 與 ST-W7-C
        //   （SAFETY-GATE(W906-ST-W7-C-DLG)）是**同一筆債**，不是新的一筆：
        //   `ShowMyMessagePWD`（golden mymessbox.h:52）全樹未移植。
        //   現場後果：Bin 設定有錯時按 HOME 沒反應、也沒有提示。
        //   ⛔ S3 之前要補 ShowMyMessagePWD —— 已在 START 計畫 §6 的必審清單。
        if(ComputeCheckOLPErrorHasErr(LastSet.OLPSetBinErr))                    // golden :33988-34010 的判定
            return false;
    }

    if(CUSTOMER_CODE!=CC_KYEC_LEE)  //Eastsun 20260526 #026-4.C1 Ifor 20240129 :KYEC skip outarm destroy on Home
    {
        if(fAllMotorHome && CheckOutArmDestory()!=0)                                //kevin 20220613 add outarm Z 在下不能HOME
            return false;
    }

    //AI(W906-HOME-W1) 20260919: GATE (W906-HOME-W1-ATCSITE) —— 缺相依：
    // `TfMain::ChangeATCSiteUse()`（golden main.h:1355）在移植樹沒有本體。
    // ⚠ `tools/live_idents.py` **沒有**把它報成缺件，因為全樹確實有一個同名的
    //   東西：`TempCtrl/TriTemp.cpp:339` 的 TU-local 外觀類別上的
    //   `void ChangeATCSiteUse() {}` —— 一個**空實作**，而且不是 TfMain 的成員。
    //   這正是 memory `symbol-exists-has-three-strengths` 講的：名字存在（第一級）
    //   不代表本體存在。抓到它的是編譯器，不是掃描器。
    // 行為：ATC 站點重映射不會執行。TriTemp 那個代用品本來就是 no-op，
    //   所以**行為 delta = 0**（差別只在少一次空呼叫）。
    // UN-GATE：等 ATC 站點重映射真的翻進來。  ✅ 20260926 已解（AI(W906-OPMODE-2) 20260926，見下面 #if 1）
#if 1 // AI(W906-OPMODE-2) 20260926: 解閘 —— 本體在 forms/fMain_ATCSiteUse.cpp（ht9045_sm），本檔在 ht9045_forms 不能直接呼叫 ⇒ 經 W906_ChangeATCSiteUseHook（wb_serve 開機裝，同 :507）；沒裝（ctest）＝跟以前一樣不做事。原 GATE (W906-HOME-W1-ATCSITE)：缺相依 TfMain::ChangeATCSiteUse() 的本體
    if (W906_ChangeATCSiteUseHook != 0) W906_ChangeATCSiteUseHook(this);   //Steven 20120523 : ATC  —— golden main.cpp:7069 `ChangeATCSiteUse();`（AI(W906-OPMODE-2) 20260926：經 hook）
#else
    { }                                                                         // GATE (W906-HOME-W1-ATCSITE)
#endif // GATE (W906-HOME-W1-ATCSITE)

    if(fAllMotorHome==false)
    {
        iHome=0;
        //AI(W906-HOME-W1) 20260919: GATE (W906-HOME-W1-BTNHOME) —— 缺相依：
        // `BtnHome` 在**活的樹**裡 0 命中。它只出現在 dfm2rc 產生的版面表
        // （tools/dfm2rc/layout_out/main_layout.gen.cpp:592），那是**資料**不是
        // 成員 —— forms/fMain.h 沒有這個 widget。
        // 行為：`iHome=0` 照設（那才是承重的），只是面板上的 HOME 燈不會彈起來。
        // UN-GATE：等 forms/fMain.h 帶進 BtnHome。
#if 0 // GATE (W906-HOME-W1-BTNHOME): 缺相依 BtnHome（活的樹 0 命中，只在 dfm2rc 版面表裡）
        BtnHome->Down=false;
#else
        { }                                                                     // GATE (W906-HOME-W1-BTNHOME)
#endif // GATE (W906-HOME-W1-BTNHOME)
    }

    if(iHome)
    {
        iHome=0;
        bLampHome=false;
    }
    else
    {
        MOT[MTestY1].Gali_Command("ST", __FUNC__);
        MOT[MTestY1].MovFlag=false;
        MOT[MTestY2].MovFlag=false;
        MOT[MTestZ1].MovFlag=false;
        MOT[MTestZ2].MovFlag=false;
        MOT[MTestY1].bScanFlag=false;
        MOT[MTestY2].bScanFlag=false;
        MOT[MTestZ1].bScanFlag=false;
        MOT[MTestZ2].bScanFlag=false;
        MOT[MTestY1].GaliSofDelayCount=0;
        MOT[MTestY2].GaliSofDelayCount=0;
        MOT[MTestZ1].GaliSofDelayCount=0;
        MOT[MTestZ2].GaliSofDelayCount=0;
        DoMotorPowerOn();
        if(MachineTypeChoice==Type_HT1032)                                      //Ztex 2024.11.29 Type_HT1032
            SW[SwServerON].On();
        SoftStart=true;
        iHome=1;
        bLampHome=true;
        iReset=0;
        iTrayFeed=0;
        bMotorPowerState=true;
        MotorPowerOnDelay=0;

        //AI(W906-HOME-W1) 20260919: GATE (W906-HOME-W1-PSM) —— 缺相依：
        // `tPSM`（省電模式計時器）在**活的樹**裡 0 命中。
        // 同一個缺件已經有先例閘：WebStart.cpp:3081 `SAFETY-GATE(W906-ST-W5-D)`。
        // 行為：歸零不會重啟省電計時 ⇒ 省電模式**不會**在歸零後被延後觸發。
        //   保守方向：少做一次 Restart 只會讓省電更早進入，不會讓機台多動。
        // ⚠ 條件式本身（IniConfig.bPowerSaveFunction）照留，這樣解閘時只要換回本體。
        // UN-GATE：等 tPSM 那一族（TPowerSaveModule）翻進來。
        if(IniConfig.bPowerSaveFunction)                                        //jou 2012-03-06 加入歸零也重啟省電模式 //Dell 20110418
#if 0 // GATE (W906-HOME-W1-PSM): 缺相依 tPSM（活的樹 0 命中；同 W906-ST-W5-D）
            tPSM.Restart();
#else
            { }                                                                 // GATE (W906-HOME-W1-PSM)
#endif // GATE (W906-HOME-W1-PSM)
    }
    iWhoTriggerPiggyBack=pbtHome;                                               //Steven 20111207 : 誰觸發了Piggy Back
    InitialPiggyBackFunction();                                                 //Steven 20110725 : 重置Piggy Back的狀態
    iWhichIndexArm=0;                                                           //Sam 20231214 : Temp offset use ready temp range

    if(IniConfig.bEnable_SECS_GEM==true)                                        //Steven 20140528 : Secs Gem
        EventReport(SECS_EVENT.DoHome);
    return true;
}

// AI(W906-MSTATE-P2) 20260924: ShowRunLabel 地基 widget 的初值，逐項取自 golden main.dfm。
//   建構子最後一行呼叫它（接在 BtnT_Start 那一行的同一行，理由見該處）。
//   * palMainStatus 的 Caption 刻意不動：Phase 1（ab147ff）讓它在沒有寫入者時保持空字串、網頁顯示 ---，
//     tests/test_wb_simpump.cpp 的 O5 釘著這一點；ShowRunLabel 一跑就會寫它。這裡只補它的字型初值。
//   * Font.Height 換成點數：-64 -> 48pt、-20 -> 15pt、-16 -> 12pt（96 dpi，golden main.dfm 的 PixelsPerInch）。
//   * Visible 沒寫出來的就是 VCL 預設 true；ARTCombine／labQAMode 在 dfm 明寫 Visible = False。
void TfMain::W906_InitMainStatusDfm()
{
    palMainStatusFont->Size   = 48;                     // golden main.dfm palMainStatus: Font.Height = -64
    palMainStatusFont->Color  = 0x00800000;             //                                Font.Color = clNavy
    labDelayStatus->Caption   = "Delay Status";         // golden main.dfm labDelayStatus: Caption
    labDelayStatus->Visible   = true;
    labDelayStatusFont->Size  = 12;                     //                                 Font.Height = -16
    labDelayStatusFont->Color = 0x000000FF;             //                                 Font.Color = clRed
    ARTCombine->Caption       = "ART  Combine";         // golden main.dfm ARTCombine: Caption（兩個空白是原文）
    ARTCombine->Visible       = false;                  //                             Visible = False
    labAutoClean->Caption     = "Auto Clean Open";      // golden main.dfm labAutoClean: Caption
    labAutoClean->Visible     = true;
    labQAMode->Caption        = "[Operation count 1000 / Setting Count 1000]";  // golden main.dfm labQAMode: Caption
    labQAMode->Visible        = false;                  //                            Visible = False
    // AI(W906-MSTATE-P2b) 20260924: ShowRunLed 的 widget（golden main.dfm；沒寫出來的屬性是 TALed／TPanel 的預設）
    ledRed->Value             = true;                   // golden main.dfm ledRed: Value = True
    ledYellow->Value          = true;                   // golden main.dfm ledYellow: Value = True
    ledGreen->Value           = false;                  // golden main.dfm ledGreen: 未寫 Value（預設 false）
    ledSafePLC->Value         = false;                  // golden main.dfm ledSafePLC: 未寫 Value（預設 false）
    pnlSafePLC->Visible       = true;                   // golden main.dfm pnlSafePLC: 未寫 Visible（VCL 預設 true）
}

// AI(W906-STATEREC) 20260924: State Record 的 forms 端（附加在檔尾，不移動既有行號）。
//   * W906_StateRecordBody -- DoStateRecord 本體的安裝座（:400 呼叫；cStateRecord.cpp 的
//     W906_InstallStateRecordBody() 由 wb_serve 開機時填入）。預設 0 = 原本的 no-op。
//   * W906_StateRecordWorkerBusy -- 背景複製／7z 執行緒還在跑。放在 forms 而不是 cStateRecord.cpp：
//     act.main.stateRecord（JsonBridge/actions/MainStateRecord.cpp，也編進 test_sjson_chan）要讀它，
//     放在這裡那支測試就不必把 cStateRecord.cpp（7z／robocopy／背景執行緒）連進去。
//   * sbStateRecordClick -- golden main.cpp:26209-26212，逐字：DoStateRecord(0, true)。
W906_StateRecordBodyFn W906_StateRecordBody = 0;
std::atomic<bool> W906_StateRecordWorkerBusy(false);  W906_TaskListOnlyFn W906_TaskListOnlyBody = 0;   //AI(W906-FLOWDIAG) 20260927: 動作流程對照專用的安裝座（fMain.h 同一行；cStateRecord.cpp 的 W906_InstallStateRecordBody 填入）
void TfMain::sbStateRecordClick(void * /*Sender*/)
{
    DoStateRecord(0, true);                                                     //KenHsieh 20230116 : 區分手動或自動(sbclick -> Function)
}

//------------------------------------------------------------------------------
// AI(W906-GB-P2b) 20260926: the Tester-comm install seat (fMain.h file end) and the golden main.h members that
//   atester.cpp / aTester_Front / aTester_Rear call as fMain->X.  Bodies: TesterComm/Handler/HandlerBridgeCtl.cpp
//   (THandlerTesterSide, golden 906 main.cpp:17878 WakeupGPIB (912 :18499; NB2 R93), and in 912: :18859 SendMSG_CMD_DeviceMapSRQ, :18866
//   SendMSG_TestMode, :18961 RunTestProgram, :29134 CloseGpibProgram).  Appended at the end of the file so no
//   existing line moves.
//------------------------------------------------------------------------------
W906_TesterForwardTable W906_TesterForward = { 0, 0, 0, 0, 0, 0, 0, 0 };   extern void (*W906_WebLoginForceOperatorHook)();   extern bool (*W906_CtcD1AccessHook)(bool bRemote);   extern bool (*W906_CtcD2IcRefuseHook)(int Mode, bool Msg);   extern bool (*W906_CtcD3ManualSortHook)();   extern void (*W906_CtcD6To2DSortHook)();   extern void (*W906_CtcD7AsmOnLineHook)();   // AI(W906-D4) / AI(W906-D1D7) 20260928 (St02-E): ChangeTesterConnect 的安裝座 —— 定義在 LogObjects.cpp 檔尾（ht9045_db；宣告與 LogObjects.h 同字），本體 D4 WebLogin.cpp W906_WebLoginForceOperator、D1/D2/D3/D6/D7 TesterComm/Handler/HandlerTesterConnect.cpp（wb_serve 裝）；0＝沒裝＝不做
bool TfMain::RunTestProgram(bool bNeedTest, bool *bSiteOnOff)
{
    if (W906_TesterForward.RunTestProgram != 0)
        return W906_TesterForward.RunTestProgram(bNeedTest, bSiteOnOff);
    return false;                                                               // golden: bFind==false -> return false
}
void TfMain::CloseGpibProgram(AnsiString Src)   { if (W906_TesterForward.CloseGpibProgram != 0) W906_TesterForward.CloseGpibProgram(Src); }
void TfMain::SendMSG_TestMode()                 { if (W906_TesterForward.SendMSG_TestMode != 0) W906_TesterForward.SendMSG_TestMode(); }
void TfMain::WakeupGPIB(AnsiString FuncName)    { if (W906_TesterForward.WakeupGPIB != 0) W906_TesterForward.WakeupGPIB(FuncName); }
void TfMain::SendMSG_CMD_DeviceMapSRQ(int iStatus) { if (W906_TesterForward.SendMSG_CMD_DeviceMapSRQ != 0) W906_TesterForward.SendMSG_CMD_DeviceMapSRQ(iStatus); }

//------------------------------------------------------------------------------
// AI(W906-GB-P2d) 20260926: golden 912 main.cpp:12581-12778 TfMain::ChangeTesterConnect (906 :12064, same code
//   except 912's two "Silent run mode change" RecordProcess lines, CASE-FOREHOPE_NINGBO-20260920-001).
//   Scope = user rulings 20260926 (github-59 relayed; FROM_STEVEN §1 P2d row):
//     DO    switch to Off-Line (:12671-12688), mode given directly (:12690-12695), the tail (:12697-12776:
//           SaveTestMode, MES2145/6/7, SCKART RunDummy, UpdateMainOperateMode, CloseGpibProgram, the ASM
//           Off-Line arm, LoadTestModePicture), and Off-Line -> On-Line (:12608-12656) without the items below.
//     GATED, TODO（使用者裁決先不做）-- AI(W906-D1D7) 20260928 (St02-E): now each calls a hook on the line next to its #if 0 (bodies TesterComm/Handler/HandlerTesterConnect.cpp, D4 WebLogin.cpp; seats LogObjects.cpp EOF); the #if 0 keeps golden's text:
//       D1 :12589-12592  the access check (AccessLevel / LevelSet.AccessLevel[8] / bRemote / I40)
//       D2 :12593-12604  the IC-in-machine refusal MES1646 (golden #ifndef SOFT_SIMULTE)
//       D3 :12610-12615  I27 Manual Sort -- gated WITH its condition, so an I27 machine goes straight On-Line
//       D4 :12621-12648  the SPIL forced logout (cbUserSelect / AccessLevel=0 / ChangeLevelAttr / captions /
//                         MES2140) and TemperatureEditDisable()
//       D6 :12658-12670  ON_LINE -> 2D_SORT: its condition stays, its body is gated (pressing the button in that
//                         state changes no mode; the golden tail still runs)
//       D7 :12727-12757  the ASM On-Line arm: its condition stays, its body is gated (no fall-through into the
//                         Off-Line arm)
//     DEVIATION (user ruling): D5 :12653-12654 RTC -- the config.ini write (RTC Enable=0) stays; the
//       "Program need to restart to active RTC fuinction!" message and bNeedRestartProgram (not on the facade,
//       same as cSetUp.cpp G-SU-Restart) are not carried.
//   Kept although inside golden :12583-12604: `if(SystemStart) return 1;` and `oldMode` -- a run-time guard and
//   the ASM tail's input, not a permission check.
//   PORT-ONLY SEAM: W906_ChangeTesterConnect_Sim != 0 returns it first (tests/test_w7_f1_wall2_probe.cpp); 0 =
//   run golden.  Moved here from :510 so no existing line moves.
//------------------------------------------------------------------------------
#include "atester_shims.h"                 // COM2 (TCOM2Shim, ->bCCDDummyRum)
#include "MessageDef.h"                    // MSG_CMD_SCKART_RunDummy
#include "common.h"                        // GrapicPath / WriteIniData
int TfMain::ChangeTesterConnect(int Mode, bool Msg, bool bRemote)
{
    if (W906_ChangeTesterConnect_Sim != 0)                                      //AI(W906-GB-P2d) 20260926: port-only test seam (see above)
        return W906_ChangeTesterConnect_Sim;

    AnsiString S=GrapicPath;
    if(SystemStart)
        return 1;

    int oldMode=LastSet.iTester;

#if 0 // AI(W906-D1) 20260928 (St02-E): golden 原文留著對照（:1113 經 W906_CtcD1AccessHook 判斷） -- golden 912 main.cpp:12589-12592（906_0625_Steven :12072-12074）
    if(AccessLevel>=LevelSet.AccessLevel[8] || bRemote==true ||
       ((IniConfig.bI40_bStartProductOnLine && LastSet.iTester==OFF_LINE) &&
       (AccessLevel<iDefEngineerLevel || bOneCycleOperateChangeON_line)))       //kevin 20140407 operater 只有ON LINE //jou 2014-06-19 Security Have 5 Level 1->iDefEngineerLevel
#else
    (void)bRemote;
    (void)S;   if (W906_CtcD1AccessHook == 0 || W906_CtcD1AccessHook(bRemote))   // AI(W906-D1) 20260928 (St02-E): D1 權限檢查 —— golden 906_0625_Steven main.cpp:12072-12074，本體 TesterComm/Handler/HandlerTesterConnect.cpp W906_CtcD1Access；沒裝＝照舊一律往下
#endif
    {   if (W906_CtcD2IcRefuseHook != 0 && W906_CtcD2IcRefuseHook(Mode, Msg)) return 1;   //kevin 20180517 change config setup   // AI(W906-D2) 20260928 (St02-E): D2 機台有 IC 不准切換（MES1646）—— golden 906_0625_Steven main.cpp:12076-12087（#ifndef SOFT_SIMULTE），本體 HandlerTesterConnect.cpp W906_CtcD2IcRefuse
#if 0 // AI(W906-D2) 20260928 (St02-E): golden 原文留著對照（:1115 經 W906_CtcD2IcRefuseHook） -- golden 912 main.cpp:12593-12604（906_0625_Steven :12076-12087）
        #ifndef SOFT_SIMULTE                                                    //Steven 20171214 : 軟體模擬可以任意關Tester
        if(bOneCycleOperateChangeON_line==false &&                              //kevin 20140411
           (HasICUnderMachine() || HasAnyICInMachine()))                        //Steven 20120517 : 有IC不能切換連線模式!!
        {
            if(Mode==10 && Msg==true)
            {
                S=AnsiString("ChangeTesterConnect :")+sHasICUnderMachine()+sHasAnyICInMachine();                        //Steven 20250110 : 顯示哪個位置還有IC
                ShowErrorMessage("MES1646", 0, MMSystem, false, S);             //Must finish [Clean out]!!
            }
            return 1;
        }
        #endif
#endif

        if(Mode==10 && Msg==true)
        {
            if(LastSet.iTester==OFF_LINE)
            {   if (W906_CtcD3ManualSortHook == 0 || W906_CtcD3ManualSortHook() == false)   // AI(W906-D3) 20260928 (St02-E): D3 I27 手動整盤 —— golden 906_0625_Steven main.cpp:12093-12098：hook 回 true＝golden 走了手動整盤那一支（下面 On-Line 那一段不跑），本體 HandlerTesterConnect.cpp W906_CtcD3ManualSort
#if 0 // AI(W906-D3) 20260928 (St02-E): golden 原文留著對照（:1134 經 W906_CtcD3ManualSortHook） -- golden 912 main.cpp:12610-12615（906_0625_Steven :12093-12098）
                if(IniConfig.bI27_ManualSortMode && bRunManualSortMode==false)
                {
                    bRunManualSortMode=true;
                    NewRecordProcess("MES2156", "Change To Manual Sort Mode by iTester Button");                        //Steven 20150915 : For TSMC 手動整盤功能
                }
                else
#endif
                {
                    bRunManualSortMode=false;                                   //Steven 20150915 : For TSMC 手動整盤功能
//                    LastSet.iTester=ON_LINE;
                    fMain->ModifyTester(ON_LINE);                               //Steven 20191218 : 整合修改LastSet.iTester
                    NewRecordProcess("MES2157", "Change to On_Line", "by iTester Button");                              //ChungHung 20140722 add add record
                    if (W906_WebLoginForceOperatorHook != 0) W906_WebLoginForceOperatorHook();   // AI(W906-D4) 20260928 (St02-E): D4 —— golden 906_0625_Steven main.cpp:12104-12131（912 :12621-12648），本體在 WebLogin.cpp（登入狀態在那裡）；原本 TODO(W906-GB-P2d) D4「使用者裁決先不做」，St02-M 20260928 派工
#if 0 // AI(W906-D4) 20260928: golden 原文留著對照（上一行已呼叫 WebLogin.cpp 的同一段） //jou 2014-03-28 SPIL Handler  On-line & Offline Switch Flow
                    cbUserSelect->ItemIndex=0;
                    AccessLevel=0;
                    ChangeLevelAttr();

                    if(CosFunction.bSecurityHave5Level==true)                   //jou 2014-06-19 Security Have 5 Level
                    {
                        if(CUSTOMER_CODE==CC_KYEC_LEE)                          //wei 20160505 增加PE權限
                        {
                            cbUserSelect->Text="Operator";
                            spbUserName->Caption="Operator";
                            NewRecordProcess("MES2140", "======== Operator login ========");
                        }
                        else
                        {
                            cbUserSelect->Text="Open";
                            spbUserName->Caption="Open";
                        }
                    }
                    else
                    {
                        cbUserSelect->Text="Operator";
                        spbUserName->Caption="Operator";
                        NewRecordProcess("MES2140", "======== Operator login ========");
                    }

                    btLogin->Caption="Login";
                    TemperatureEditDisable();
#endif
                    if(CosFunction.bLockRTC && COM2->bCCDDummyRum==true)        //JerryYang 20160223 add for Philippine,切換為Online時強制開啟RTC
                    {
                        COM2->bCCDDummyRum=false;
                        WriteIniData("D:\\HT9045\\config\\config.ini", "RTC", "Enable", false);
#if 0 // AI(W906-GB-P2d) 20260926: D5 USER RULING -- keep the config.ini write, show no message, arm no restart (bNeedRestartProgram is not on the facade either; cSetUp.cpp G-SU-Restart) -- golden main.cpp:12653-12654
                        ShowMyMessage("Program need to restart to active RTC fuinction!", "The program will automatically be closed");
                        bNeedRestartProgram=true;
#endif
                    }
                }
            }
            else if(LastSet.iTester==ON_LINE &&
                    TestIF_File.bSortingBy2DIDList==true)                       //Frank 20221122 : 2DID sorting for ATK
            {   if (W906_CtcD6To2DSortHook != 0) W906_CtcD6To2DSortHook();   // AI(W906-D6) 20260928 (St02-E): D6 ON_LINE → 2D_SORT —— golden 906_0625_Steven main.cpp:12144-12152，本體 HandlerTesterConnect.cpp W906_CtcD6To2DSort；沒裝＝照舊不換模式
#if 0 // AI(W906-D6) 20260928 (St02-E): golden 原文留著對照（:1191 經 W906_CtcD6To2DSortHook） -- golden 912 main.cpp:12661-12669（906_0625_Steven :12144-12152）
                bRunManualSortMode=false;                                       //Steven 20180915 : For TSMC 手動整盤功能
                fMain->ModifyTester(_2D_SORT);                                  //Steven 20191218 : 整合修改LastSet.iTester
                NewRecordProcess("MES2155", "Change to 2D_SORT", "by iTester Button");                                  //ChungHung 20140722 add add record

                if(CosFunction.bOffLineBin)
                {
                    fBinSel->ReadParam();
                    fBinSel->ReadFile(true, false, "");
                }
#endif
            }
            else
            {
                bRunManualSortMode=false;                                       //Steven 20180915 : For TSMC 手動整盤功能
//                LastSet.iTester=OFF_LINE;
                fMain->ModifyTester(OFF_LINE);                                  //Steven 20191218 : 整合修改LastSet.iTester
                NewRecordProcess("MES2155", "Change to Off_Line", "by iTester Button");                                 //ChungHung 20140722 add add record
                if(bOneCycleOperateChangeON_line)
                {
                    bOneCycleOperateChangeON_line=false;                        //kevin 20140411
                    iOff_LINE_Mode=0;                                           //kevin 20140411
                }

                if(CosFunction.bOffLineBin)
                {
                    fBinSel->ReadParam();
                    fBinSel->ReadFile(true, false, "");
                }
            }
        }
        else
        {
//            LastSet.iTester=Mode;
            fMain->ModifyTester(Mode);                                          //Steven 20191218 : 整合修改LastSet.iTester
            NewRecordProcess("MES2157", "Change to On_Line", "by Automation");  //ChungHung 20140722 add add record
        }

        if(CosFunction.bLastSetInSetUpFile)
        {
            TestMode.iTestConnection=LastSet.iTester;
            SaveTestMode();
        }

        if(LastSet.iTester==OFF_LINE)                                           //Steven 20140815
        {
            if(IniConfig.bI27_ManualSortMode && bRunManualSortMode==true)       //Steven 20150915 : For TSMC 手動整盤功能
                NewRecordProcess("MES2145", "XXXX  Tester MANUAL MODE  XXXX");
            else
                NewRecordProcess("MES2146", "XXXX  Tester OFF-Line  XXXX");
        }
        else
        {
            NewRecordProcess("MES2147", "VVVV  Tester ON-Line  VVVV");
        }

        if(CosFunction.bUseSCKART)                                              //Steven 20250304 : For SCK 93K ART
        {
            fMain->SendMSG_CMD(MSG_CMD_SCKART_RunDummy);
        }

        UpdateMainOperateMode();
        CloseGpibProgram(__FUNC__);

        if(oldMode!=LastSet.iTester)                                            //Steven 20120628 : 開關Site, 強制啟動Auto Site Mapping
        {
            if(IniConfig.bUseAutoSiteMapping && IniConfig.bDutOnOffNeedASM)
            {
                if(IniConfig.bI21EnableASM && LastSet.iTester==ON_LINE)         //Steven 20110502
                {   if (W906_CtcD7AsmOnLineHook != 0) W906_CtcD7AsmOnLineHook();   // AI(W906-D7) 20260928 (St02-E): D7 ASM 的 On-Line 那一支 —— golden 906_0625_Steven main.cpp:12212-12237（＋912 :12741-12742 那行 RecordProcess），本體 HandlerTesterConnect.cpp W906_CtcD7AsmOnLine
#if 0 // AI(W906-D7) 20260928 (St02-E): golden 原文留著對照（:1261 經 W906_CtcD7AsmOnLineHook） -- golden 912 main.cpp:12729-12756（906_0625_Steven :12212-12237）
                    if(TestIF_File.iTestMode==SingleSite)                       //Steven 20130610 : Single Site不做Auto Site Mapping
                    {
                        if(iAutoSiteMapRunStartMode==0)                         //Steven 20230410 : Add for Auto site map
                        {
                            LastSet.iRunStartMode=rsmContinuStart;
                            fMain->cbRunStartMode->Text=StartModeName[rsmContinuStart];
                        }
                        else
                        {
                            LastSet.iRunStartMode=rsmContinuRetest;
                            fMain->cbRunStartMode->Text=StartModeName[rsmContinuRetest];
                        }
                        //AI(ht9045-v912) 20260921: On/Off Line 切換會靜默改 Start Mode 且不發 MES2107, 補記以利追查 (CASE-FOREHOPE_NINGBO-20260920-001)
                        RecordProcess("Silent run mode change by iTester On/Off Line : "+fMain->cbRunStartMode->Text, "ChangeTesterConnect");
                    }
                    else
                    {
                        //jou 20200701 : VTEST for auto site mapping cable mount
                        if(IniConfig.bVTESTFunction==true)
                        {
                            if(TestIF_File.bAutoSiteMappingOpenSite)
                                fMain->SetMainRunStartMode(rsmAutoSiteMap);
                        }
                        else
                        {
                            fMain->SetMainRunStartMode(rsmAutoSiteMap);
                        }
                    }
#endif
                }
                else
                {
                    if(iAutoSiteMapRunStartMode==0)                             //Steven 20230410 : Add for Auto site map
                    {
                        LastSet.iRunStartMode=rsmContinuStart;
                        fMain->cbRunStartMode->Text=StartModeName[rsmContinuStart];
                    }
                    else
                    {
                        LastSet.iRunStartMode=rsmContinuRetest;
                        fMain->cbRunStartMode->Text=StartModeName[rsmContinuRetest];
                    }
                    //AI(ht9045-v912) 20260921: On/Off Line 切換會靜默改 Start Mode 且不發 MES2107, 補記以利追查 (CASE-FOREHOPE_NINGBO-20260920-001)
                    RecordProcess("Silent run mode change by iTester On/Off Line : "+fMain->cbRunStartMode->Text, "ChangeTesterConnect");
                }
            }
        }
    }
    LoadTestModePicture();
    return 0;
}

//------------------------------------------------------------------------------
// AI(W906-LOGSINK) 20260927: golden main.cpp:30228-30246 / :30248-30261, line for line (cp950-decoded).  Both were no-op sinks
//   (this file :402 / :473) with 14 + callers each (Shuttle latch positions / AutoClean steps).  The memos are the
//   in-memory logs golden shows on the main form; AddAutoCleanMessage also saves d:\AutoCleanLogs\<time>_AutoCleanLog.csv
//   every 2048 lines, as golden does.  AI(W906-MEMO) 20260927: the memos are TfMainMemo, which stores lines now (forms/FormWidgets.h, cap 4096);
//   AddAutoCleanMessage still has no caller outside #ifdef DEBUG_AUTO_CLEAN, so memoAutoClean stays empty in normal builds.
//------------------------------------------------------------------------------
void TfMain::AddShuttleMessage(int iWhichSht, AnsiString Msg)                   //Steven 20130614 : 紀錄Shuttle Latch的位置
{
    if(iWhichSht==0)
    {
        if(fMain->meShuttle1->Lines->Count>2048)
        {
            fMain->meShuttle1->Clear();
        }
        fMain->meShuttle1->Lines->Add(Msg);
    }
    else
    {
        if(fMain->meShuttle2->Lines->Count>2048)
        {
            fMain->meShuttle2->Clear();
        }
        fMain->meShuttle2->Lines->Add(Msg);
    }
}
//------------------------------------------------------------------------------
void TfMain::AddAutoCleanMessage(AnsiString Msg)                                //Steven 20130618 : 紀錄Auto Clean的狀態
{
    AnsiString FileName;
    if(memoAutoClean->Lines->Count>2048)
    {
        FileName.sprintf("d:\\AutoCleanLogs\\%04d%02d%02d_%02d%02d%02d_AutoCleanLog.csv", SystemYear, SystemMonth, SystemDate, SystemHour, SystemMin, SystemSec);
        MyForceDirectories("d:\\AutoCleanLogs\\");
        memoAutoClean->Lines->SaveToFile(FileName);
        memoAutoClean->Clear();
    }

    FileName.sprintf("%04d-%02d-%02d, %02d:%02d:%02d, %s", SystemYear, SystemMonth, SystemDate, SystemHour, SystemMin, SystemSec, Msg);
    memoAutoClean->Lines->Add(FileName);
}

//------------------------------------------------------------------------------
// AI(W906-W10) 20260927 (St02-E): see the end of forms/fMain.h.  golden 906_0625_Steven main.h:121-122 +
//   main.dfm:17358-17377 (TeraTCPResultServer: OnClientConnect / OnClientDisconnect / OnClientError; TCPCommandServer:
//   OnClientConnect / OnClientDisconnect / OnClientRead; both Active False, Port 0, stNonBlocking).
//------------------------------------------------------------------------------
void W906_TcpServersCreate(TfMain* self)
{
    self->TeraTCPResultServer = new TServerSocket(NULL);
    self->TeraTCPResultServer->OnClientConnect    = [self](TObject* s, TCustomWinSocket* k) { self->TeraTCPResultServerClientConnect(s, k); };
    self->TeraTCPResultServer->OnClientDisconnect = [self](TObject* s, TCustomWinSocket* k) { self->TeraTCPResultServerClientDisconnect(s, k); };
    self->TeraTCPResultServer->OnClientError      = [self](TObject* s, TCustomWinSocket* k, TErrorEvent e, int& c) { self->TeraTCPResultServerClientError(s, k, e, c); };
    self->TCPCommandServer = new TServerSocket(NULL);
    self->TCPCommandServer->OnClientConnect    = [self](TObject* s, TCustomWinSocket* k) { self->TCPCommandServerClientConnect(s, k); };
    self->TCPCommandServer->OnClientDisconnect = [self](TObject* s, TCustomWinSocket* k) { self->TCPCommandServerClientDisconnect(s, k); };
    self->TCPCommandServer->OnClientRead       = [self](TObject* s, TCustomWinSocket* k) { self->TCPCommandServerClientRead(s, k); };
}

W906_RemoteRunTable W906_RemoteRun = { 0, 0 };
bool W906_RemoteRunStart(AnsiString Func) { return W906_RemoteRun.Start != 0 && W906_RemoteRun.Start(Func); }
bool W906_RemoteRunPause(AnsiString Func) { return W906_RemoteRun.Pause != 0 && W906_RemoteRun.Pause(Func); }
