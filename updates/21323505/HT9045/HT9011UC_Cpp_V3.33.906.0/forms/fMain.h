// =============================================================================
//  forms/fMain.h  --  non-VCL stand-in for golden's fMain form pointer
//
//  AI(W906-W7-F0) 20260728: split out of FormsFacade.h by the W7-F0 refactor
//  (docs/W7_UI_ARCHITECTURE_PLAN.md SS6-F0-d).  Members and comments moved
//  VERBATIM apart from the F0-e virtualisation and the widget types now coming
//  from forms/FormWidgets.h.
//
// =============================================================================
//  ############  THE FACADE CONTRACT -- BINDING ON EVERY forms/ FILE  ########
// =============================================================================
//  (Written into THIS file per plan SS6-F0-e, because fMain is the form every
//   later wave reads first.  It applies to all 9 forms in this directory.)
//
//  1. METHODS ARE `virtual`.  Their bodies here are offline no-ops / safe
//     defaults, and those bodies are the PERMANENT OFFLINE IMPLEMENTATION --
//     not scaffolding to be deleted.  When the MFC layer lands (plan SS2-D3),
//     a `TfXxxImpl : public TfXxx` binder held BY COMPOSITION inside the
//     CDialog overrides them and `fXxx` is repointed at that impl.  The
//     headless ctest suite keeps constructing the plain TfXxx and keeps
//     getting these bodies, so the offline oracle never depends on MFC.
//
//  2. DATA MEMBERS ARE CONCRETE STORAGE AND MUST **NEVER** BECOME ACCESSORS.
//     This is not a style preference, it is arithmetic: 69.3% of golden's
//     4,266 `fMain->` dereferences are data-member syntax
//     (`fMain->cbSetupFileName->Text`), C++ has no BCB6 `__property`, and
//     there are ~14,432 cross-module dereferences tree-wide.  Turning members
//     into Get/Set would mean rewriting all of them.  The MFC side keeps these
//     fields fresh with DDX (which is exactly what DDX is for) instead.
//     A "pure virtual interface" facade is therefore IMPOSSIBLE here, and that
//     question is closed (plan SS2-D2).
//
//  3. WIDGET TYPES COME FROM forms/FormWidgets.h -> vclcompat/Controls.h.
//     A wave that needs a new MEMBER adds it to its own forms/fXxx.h.  A wave
//     that thinks it needs a new widget TYPE is almost certainly wrong: use an
//     existing vclcompat/Controls.h stock type, or -- for the 3 custom control
//     families (tray / btn-panel / LED) -- vclcompat/{TrayCore,BtnPanelCore,
//     LedCore}.  Do NOT define another `struct {AnsiString Text;}`.
//
//  4. EVERY MEMBER CARRIES ITS GOLDEN HOME, tagged [DATA] or [METHOD].  Out-of-
//     scope bodies stay documented no-op stubs (never silently "implemented"),
//     and where a test needs to feed a gated leaf, use the established
//     `W906_..._Sim` settable-seam idiom (PERSITETemperatureStrings below is
//     the reference example) rather than a bare no-op -- a bare no-op makes the
//     gap invisible to the suite.
//
//  5. FILE OWNERSHIP.  One form per file means two waves touching two different
//     forms no longer collide.  Two waves touching the SAME form still have to
//     serialise.  forms/FormWidgets.h should need no edits at all.
// =============================================================================
//
//  Original translation waves for the content below: W6.0 (scaffold) / W6.2
//  (in-arm HP geometry) / W6.3 (tray-arm) / W6.4 (tester) / W6.5 (shuttle) /
//  W6.6 (hub) / W7-C1 / W5-comms / W5-Automation / W5-Final-SckArtRemainder /
//  W906-Automation / W906-TesterTCPTimer / W906-AutoCleanFoundation /
//  W906-AutoCleanCluster / W906-AutoSiteMapCleanOut.
//
//  W6 DECOUPLING STRATEGY (form-pointer cut), from the original file head:
//  The BCB6 state machines reach UI/automation state through global VCL TForm
//  pointers (fMain / fAGV / fSortCT / fLotInfo / fOffSet / fSCKART / ...).
//  Those forms cannot be pulled into the portable build (they derive from VCL
//  TForm).  Instead we expose ONLY the members a given sub-wave's SM actually
//  dereferences, on a non-VCL facade, and grow it per sub-wave.
//  The sbStateRecordClick(sbStateRecord) line in CheckHasSpaceToPlace_9045 is
//  gated #if 0, so TSpeedButton/TObject are NOT pulled in and those two TfMain
//  members are intentionally NOT added.  cb1 is referenced only inside  [AI(W906-STATEREC) 20260924: sbStateRecordClick(void*) IS now added (end-of-class STATEREC block, for act.main.stateRecord); sbStateRecord still is not, so that gated line stays gated]
//  #ifdef SOFT_SIMULTE (undefined) so it is not added either.
// =============================================================================
#ifndef FORMS_FMAIN_H
#define FORMS_FMAIN_H

#include "forms/FormWidgets.h"

// ---------------------------------------------------------------------------
//  W6.3: TfMainHanaART -- fMain->hanaART (golden main.h, HANA ART helper).  The
//  catchtray count helpers (AddFixTrayCount/AddTrayCount) call exactly two
//  methods on it; offline (no HANA link) IsHanaArtAvailable() is false so the
//  count helpers early-out, and AddNewTrayHead is a no-op.
// ---------------------------------------------------------------------------
class TfMainHanaART
{
public:
    virtual bool IsHanaArtAvailable();      // [METHOD] golden -- offline: false
    virtual void AddNewTrayHead(int iAuto); // [METHOD] golden -- offline: no-op
    // -- W906-P10 ADD (20260921) --------------------------------------------
    //    golden `SetRunStartMode` 在切到 RT 模式時會清掉 HanaART 的暫存並
    //    重送一次 handler 等待資料（golden main.cpp 的 hanaART 區塊）。
    //    ⚠ **離線空樁**，不是翻譯：整個 HanaART 子系統沒翻，而且這一段的
    //      守衛是 `IsHanaArtAvailable()`，它在本樹恆回 false ⇒ **這兩支不可達**。
    //      補它們只是讓 P10 的本體能逐字照 golden 編得過。
    //    UN-GATE：等 HanaART 子系統翻進來，這兩支換真本體。
    virtual void Clear() {}                                   // [METHOD] golden -- offline no-op（不可達）
    virtual void SetHandlerWaitingData(AnsiString, AnsiString, // [METHOD] golden -- offline no-op（不可達）
                                       AnsiString, int) {}
    virtual bool IsPrimeTest() { return false; }   /* AI(W906-W2-SETTESTRUNMODE) 20260926: [METHOD] golden -- 離線空樁（不可達），理由同上面 P10 那兩支：SetTestRunMode 只在 IsHanaArtAvailable()（恆 false）為真時才呼叫它 */   virtual ~TfMainHanaART() {}
};

// ---------------------------------------------------------------------------
//  W6.5: TfMainInplace -- golden cInplace (InArmPlacementEnable()/
//  iNo9ShtErrICCt/bNo9ShtErrNo).  Offline InArmPlacementEnable()==false makes
//  the No9 sub-blocks inert (the AutoLatch checks still run their main path).
// ---------------------------------------------------------------------------
class TfMainInplace
{
public:
    int  iNo9ShtErrICCt[2];          // [DATA]   golden cInplace -- per-shuttle No9 err count
    bool bNo9ShtErrNo[2][8];         // [DATA]   golden cInplace -- per-site No9 err flag
    // W7-A1 ADD: the In-Sht-Latch / No9 combine flag the floating + latch SMs touch
    // (golden cInArmPlacement.h:63, KenHsieh 20251105).  Golden-faithful default is
    // FALSE: DoInArmCheckShuttleFloating case 9000 SETS bNo9Action=false (golden
    // ainarm9045.cpp:3744-ctx) and NO offline path sets it true (the only true-set
    // sites live inside InArmPlacementEnable()==false No9 placement blocks, dead
    // offline).  The latch reads (golden :3941/:4045, via W7A1_NO9_ACTION()) are all
    // guarded by InArmPlacementEnable() && bNo9Action, so an always-false member is
    // behaviorally identical to the offline (false) macro.  No other TU references it.
    bool bNo9Action;                 // [DATA]   golden cInArmPlacement.h:63 -- default false
    virtual bool InArmPlacementEnable();     // [METHOD] golden cInplace -- offline: false (No9 inert)
    TfMainInplace();
    virtual ~TfMainInplace() {}
};

// ---------------------------------------------------------------------------
// AI(W906-W7-L1-Wave0) 20260801: TfMainRENESASServer -- stand-in for
// fMain->RENESAS_Server, added because golden asendic_Loader.cpp derefs it on
// the loader tray-supply path (:2644 writes bLoadingCountFullFlag=false, :2722
// and :2756 call DoNeedSupplyOrNot(true/false)).  Golden home:
// main.h:1710 `TRENESAS_Server *RENESAS_Server;` (Kirin 20191213), whose class
// declares `bool bLoadingCountFullFlag;` at Automation/uRENESAS_Server.h:199
// and `bool DoNeedSupplyOrNot(bool bNotEnough);` at :201 -- both re-read from
// the cp950-decoded golden this wave.  Same nested-helper-class idiom as
// TfMainHanaART / TfMainInplace above.  Offline DoNeedSupplyOrNot() returns
// false: there is no RENESAS FT-CT server link, so "no extra supply is being
// demanded" is the faithful quiet default and keeps the two golden call sites
// on their non-supply arm.
// ---------------------------------------------------------------------------
class TfMainRENESASServer
{
public:
    bool bLoadingCountFullFlag;                     // [DATA]   golden Automation/uRENESAS_Server.h:199
    virtual bool DoNeedSupplyOrNot(bool bNotEnough);// [METHOD] golden Automation/uRENESAS_Server.h:201 -- offline: false
    TfMainRENESASServer();
    virtual ~TfMainRENESASServer() {}
};

// AI(W906-FW3-WE) 20260818: forward declaration for the FW3-WE ADD block's
// Get2DID_OrderBySites(TMyKitSuck*, TStringList*) member (see that block's
// own comment for the full citation) -- MUST sit here, at file/global scope,
// NOT inside `class TfMain` below: an earlier pass of this edit placed it
// inside the class body by mistake, which silently declares a nested
// `TfMain::TMyKitSuck` (a different, permanently-incomplete type) instead of
// forward-declaring the real global `::TMyKitSuck` (aHotPlateSubstrate.h
// :365) -- caught by the compiler ("invalid use of incomplete type
// 'class TfMain::TMyKitSuck'") when Command.cpp's own Get2DID_OrderBySites
// DEFINITION (which #includes aHotPlateSubstrate.h and so sees the REAL
// global TMyKitSuck) tried to dereference `kit->Item[i][j]` against a
// pointer the header had typed as the empty nested stand-in. Corrected
// before landing.
class TMyKitSuck;

// ===========================================================================
//  TfMain -- non-VCL stub (golden main.h, TfMain:public TForm)
// ===========================================================================
class TfMain
{
public:
    virtual void DebugOneCycleHotPlate(AnsiString sfunc); // [METHOD] golden main.h:1565 -- debug log sink (empty)
    virtual bool Pause(AnsiString Func);                  // [METHOD] golden main.h:1249 -- offline never pauses -> false
    // AI(W906-W7-F1fix2) 20260729: two seams on the PRE-EXISTING Pause() above,
    //   added because Pause() is a bare `return false;` with no side effect, so
    //   NOTHING that forwards INTO it can be observed by a test. Specifically
    //   BtnPauseClick's `Pause("BtnPauseClick")` forward (golden main.cpp:6967)
    //   was still unobservable after W7-F1fix added W906_BtnPauseClickCallCount:
    //   that counter proves BtnPauseClick was CALLED, not that it FORWARDED --
    //   deleting the forward left every assertion green (personally reproduced:
    //   see tests/test_w7_f1_wall2_probe.cpp's BtnPauseClick block). These two
    //   make the forward itself, AND the Func argument golden passes, testable.
    //   Purely additive: Pause() still returns false unconditionally, so the
    //   ~40 SM call sites that already call fMain->Pause(...) are unaffected.
    int        W906_PauseCallCount;                       // [PORT-ONLY SEAM] call counter, default 0
    AnsiString W906_PauseLastFunc;                        // [PORT-ONLY SEAM] last Func argument seen, default ""
    virtual void ShowTestHeadComp(bool bRefresh);         // [METHOD] golden main.h:1296 -- empty
    virtual void ReStartAutoSiteMapping(bool bStart);     // [METHOD] golden main.h:1331 -- empty
    // AI(W906-AutoSiteMapCleanOut) 20260727: golden main.h:1334 __fastcall
    // SetMainRunStartMode(int) -- documented GAP no-op stub, see forms/fMain.cpp
    // for the full citation (golden main.cpp:28236-28308 cascades into
    // UpdateMainOperateMode() golden main.cpp:12803-13127 + a new fBinSel VCL
    // form, both out of scope that wave). Added so csystem.cpp's
    // InitCleanOutFunction AutoSiteMap branch (golden csystem.cpp:15751-15785)
    // can be un-gated without pulling in that chain.
    virtual void SetMainRunStartMode(int iSetMode);       // [METHOD] golden main.h:1334 -- documented GAP stub (no-op)
    TfMainSiteMapLog *slAutoSiteMapLog;           // [DATA]   golden main.h:1486 (TMyStringList*) -- new in ctor
    // -- W6.3 ADD: members the TRAY-ARM ENGINE (acatchtray.cpp) derefs ----------
    bool CleanOut(AnsiString Func);                       // [METHOD] golden main.h:1257 (body main.cpp:4269-4330) -- AI(W906-CLEANOUT) 20260924: 本體在 cCleanOut.cpp（ht9045_sm，要 InitCleanOutFunction／MOT[]／EventReport）；回傳 bool 照 golden；改非 virtual（virtual 本體放 sm 會讓 forms 的 vtable 依賴 sm；全樹無覆寫）
    virtual void DoStateRecord(int i, bool b);            // [METHOD] golden main.h -- offline: state-record sink no-op
    TfMainTrayPanel *mtAuto1;                     // [DATA]   golden main.h:168 (TTMyTray* mtAuto1 -- see forms/FormWidgets.h's TfMainTrayPanel conflation note; only ->Color assigned)
    TfMainTrayPanel *mtAuto2;                     // [DATA]   golden main.h:169 (TTMyTray* mtAuto2)
    TfMainTrayPanel *mtAuto3;                     // [DATA]   golden main.h:170 (TTMyTray* mtAuto3)
    TfMainHanaART   *hanaART;                     // [DATA]   golden main.h (HANA ART helper)
    // -- W6.4 ADD: members the TESTER/INDEX ENGINE (atester.cpp) derefs ----------
    virtual void LightOn();                               // [METHOD] golden main.h -- offline: CCD light no-op (DoTestHeadMotor CCD path)
    TfMainTrayPanel *lbCCDStatus;                 // [DATA]   golden main.h:672 (TLabel* lbCCDStatus) -- ->Visible written by atester.cpp:1894
    // -- W6.5 ADD: members the SHUTTLE ENGINE (acarry.cpp) derefs ----------------
    //    * TfMainCheckBox -- golden TCheckBox* (cbShowShuttleSensor /
    //      cbTestOutShuttleSensor / cbShowInShuttleSensor).  Engine reads
    //      .Checked (offline false -> the debug-log / sensor-confirm paths are
    //      skipped, the proven DUMMY posture).
    //    * TfMainGrid -- golden TTMyTray* (htShullte0 / htShullte1, golden
    //      main.h:345-346; the "THeatTable*" label this comment used to carry was
    //      wrong -- corrected by W7-F0, see forms/FormWidgets.h).  Engine calls
    //      SetCellColorIndex(col,row,idx) only inside the Motor-View page guard
    //      (pgMain->ActivePageIndex==emp7TabSheet21, both 0 offline so equal --
    //      but SetCellColorIndex is a harmless no-op so it is safe either way).
    //    * TfMainMemo / TfMainMemoLines -- golden TMemo* (meShuttle1/meShuttle2,
    //      golden main.h:541-542).  Engine calls ->Lines->Add / ->Lines->Count /
    //      ->Clear inside the cbShowInShuttleSensor.Checked guard (false offline)
    //      -> no-op.
    TfMainCheckBox *cbShowShuttleSensor;          // [DATA]   golden main.h:540 (TCheckBox*) -- offline Checked=false
    TfMainCheckBox *cbTestOutShuttleSensor;       // [DATA]   golden main.h:543 (TCheckBox*) -- offline Checked=false
    TfMainCheckBox *cbShowInShuttleSensor;        // [DATA]   golden main.h:910 (TCheckBox*) -- offline Checked=false
    TfMainGrid     *htShullte0;                   // [DATA]   golden main.h:345 (TTMyTray* shuttle-1 grid)
    TfMainGrid     *htShullte1;                   // [DATA]   golden main.h:346 (TTMyTray* shuttle-2 grid)
    TfMainMemo     *meShuttle1;                   // [DATA]   golden main.h:542 (TMemo* shuttle-1 log)
    TfMainMemo     *meShuttle2;                   // [DATA]   golden main.h:541 (TMemo* shuttle-2 log) -- AI(W906-F0fix) 20260728: corrected from wrong ":543" (that line is TCheckBox *cbTestOutShuttleSensor, already cited at :157), re-verified against golden main.h via cp950 read
    TfMainInplace  *cInplace;                     // [DATA]   golden main.h (cInplace placement helper)
    TfMainPageControl *pgMain;                    // [DATA]   golden main.h:64 (TPageControl*) -- ActivePageIndex==0 offline
    int emp7TabSheet21;                           // [DATA]   golden main.h (Motor-View tab index) -- 0 offline (==pgMain->ActivePageIndex)
    virtual void AddShuttleMessage(int iSht, AnsiString S);     // [METHOD] golden main.h -- offline log sink no-op
    virtual void Reset(AnsiString Func);                       // [METHOD] golden main.h -- offline: no-op
    virtual void BtnOneCycleClick(void *Sender);               // [METHOD] golden main.h -- offline: no-op
    virtual void BtnResetClick(void *Sender);                  // [METHOD] golden main.h -- offline: no-op
    void BtnCleanOutClick(void *Sender);                       // [METHOD] golden main.h:914 (body main.cpp:4264-4267) -- AI(W906-CLEANOUT) 20260924: 本體在 cCleanOut.cpp（= CleanOut("BtnCleanOutClick")）；改非 virtual，理由同 CleanOut（全樹無覆寫）
    virtual void JSCC_ResetForShuttleLoseIC();                 // [METHOD] golden main.h -- offline: no-op (0-arg)
    virtual void ResetRecordforPiggyBack(AnsiString S);        // [METHOD] golden main.h -- offline: no-op
    // -- W6.6 ADD: the per-tick sensor scan the HUB main loop calls -------------
    void ProcessSensorScan();                                  // [METHOD] golden main.h:1230 -- DoAllProcess() every tick; body cSensorScan.cpp (ht9045_sm)  //AI(W906-SENSORSCAN) 20260924: 拿掉 virtual —— golden 本來非 virtual、全樹無覆寫（WebStart.h 的 TfMainWeb 也沒有）；留著 virtual 會讓 forms 的 vtable 引用 sm 的符號
    // -- W7-C1 ADD: members the END-OF-LOT CLEAN-OUT FINISH-CHECK (DoCleanOutFinishCheck)
    //    derefs.  golden main.h.  All offline no-op / sane default (a handler that
    //    is draining clean-out with no UI never re-starts/re-levels/re-tests).
    //    AI(W7C1-Integrate) 20260629.
    virtual void Start(AnsiString Func);                       // [METHOD] golden main.h -- offline: do NOT auto re-start (no-op); ~6 sites
    virtual void ChangeLevelAttr();                            // [METHOD] golden main.h -- offline: level-attr UI no-op
    virtual void ModifyTester(int iWhich);                     // [METHOD] golden main.h:Steven 20191218 -- offline: QA tester-modify no-op
    virtual void CleanYieldCount();                            // [METHOD] golden main.h -- offline: yield-count clear no-op
    TfMainSpeedButton *BtnOneCycle;                           // [DATA]   golden main.h:72 (TBtnPanel* BtnOneCycle -- NOT TSpeedButton, corrected by W7-F0; see forms/FormWidgets.h); offline Down=false
    // -- W5-comms INTEGRATE ADD: members Interface/InterfaceSYS.cpp derefs ------
    //    (the WM_COPYDATA IPC bridge to the ESD / Auto-Update / Event-Log-
    //    Analyzer helper programs).  golden main.h:1216/1217/1218/1220.  Offline
    //    default NULL/0 preserves golden's own "no window found yet" safe path
    //    (FindWindow is called lazily at each send site, matching golden).
    //    AI(W5-comms-Integrate) 20260710.
    HWND HESDWnd;                                 // [DATA] golden main.h:1216 (HWND) -- offline NULL
    HWND HEventLogWnd;                            // [DATA] golden main.h:1217 (HWND) -- offline NULL
    HWND HAutoUpdateWnd;                          // [DATA] golden main.h:1218 (HWND) -- offline NULL
    int  oldGpibAddress;                          // [DATA] golden main.h:1220 (int)  -- offline 0
    // -- W5-Automation INTEGRATE ADD: members Automation/HANA_ART.cpp derefs -----
    //    (the HANA-ART tester-side SRQ helper).  golden main.h:1400/1401/1531/
    //    1532/1370.  Offline: no real GPIB-bridge process / no SamSung-specific
    //    map or soak-time source / no arm-status telemetry sink.
    virtual void SendMSG_CMD(int CMD);                         // [METHOD] golden main.h:1400 -- AI(W906-GB-P2b) 20260926: forwards through W906_TesterForward (file end); not installed = offline no-op
    virtual void SendMSG_CMD(int CMD, AnsiString Message);     // [METHOD] golden main.h:1401 -- AI(W906-GB-P2b) 20260926: forwards, as above
    virtual bool RunTestProgram(bool bNeedTest, bool *bSiteOnOff=NULL);  // [METHOD] golden main.h -- AI(W906-GB-P2b) 20260926: forwards; not installed = false (golden's bFind==false answer)
    virtual void CloseGpibProgram(AnsiString Src="");          // [METHOD] golden main.h -- AI(W906-GB-P2b) 20260926: forwards; not installed = no-op
    virtual void SendMSG_TestMode();                           // [METHOD] golden main.h -- AI(W906-GB-P2b) 20260926: forwards; not installed = no-op
    virtual void WakeupGPIB(AnsiString FuncName);              // [METHOD] golden main.h -- AI(W906-GB-P2b) 20260926: forwards; not installed = no-op
    virtual void SendMSG_CMD_DeviceMapSRQ(int iStatus);        // [METHOD] golden main.h -- AI(W906-GB-P2b) 20260926: forwards; not installed = no-op
    virtual AnsiString GetSamSungMap(bool bSend=true);         // [METHOD] golden main.h:1531 (body Command.cpp:10137) -- offline: ""
    virtual AnsiString GetSamSungSoakTime(bool bSend=true);    // [METHOD] golden main.h:1532 (body Command.cpp:10305) -- offline: "0"
    virtual AnsiString ArmStatusStrings();                     // [METHOD] golden main.h:1370 (body Command.cpp:1497) -- offline: ""
    // -- W5-Final-SckArtRemainder INTEGRATE ADD: members Automation/SCK_ART_Remainder.cpp
    //    (SckArtRem_AccessFile) derefs -- golden main.h, bodies in main.cpp (untranslated).
    //    All 3 are UI-refresh-only in golden (recipe combo/test-mode picture/backup-on-write);
    //    offline no-op, matching every other fMain UI-refresh sink above.
    virtual void SetStartModeData();                           // [METHOD] golden main.h -- offline: recipe start-mode UI refresh no-op
    virtual void LoadTestModePicture();                        // [METHOD] golden main.h -- offline: test-mode picture UI refresh no-op
    virtual void BackupSetupFile();                            // [METHOD] golden main.h -- offline: setup-file backup no-op (Ifor 20170620)
    // -- W5-Automation ADD (AGV_PortScan unit, 20260713): members
    //    Automation/AGV_PortScan.cpp derefs (AMR SPIL port-scan LED +
    //    E84 loader/unloader tray-count scan's SECS-link panel) -----------------
    TfLedValue *ALed1;                                        // [DATA] golden main.h:355 (TALed*) -- bScanLoadPortState_SPIL reads ->Value
    TfMainPanel *labAutomation;                               // [DATA] golden main.h:802 (TPanel*) -- DoE84LoaderScan/DoE84UnloaderScan compare ->Caption
    // -- W906-Automation ADD (20260716): members Automation/automation.cpp
    //    derefs (GetMachineStatus/GetWorkOrder/GetMainTemp + the deferred
    //    ProcessBuffer's own fMain->Home("TfAutomation::ProcessBuffer") call,
    //    golden automation.cpp:1522 -- ProcessBuffer itself is GATED in that
    //    wave, see Automation/automation.h, but Home() is added per that
    //    front's task brief as a small additive cross-file gap). Same
    //    Caption/Text-stub shape already used elsewhere --
    //    palMainStatus reuses TfMainPanel (golden main.h:669 TPanel*),
    //    cbSetupFileName reuses TfLotInfoRunMode (golden main.h:875 TComboBox*,
    //    only ->Text read here), edWorkTemperBase reuses TfLotInfoEdit
    //    (golden main.h:732 TEdit*).
    TfMainPanel       *palMainStatus;                         // [DATA] golden main.h:669 (TPanel*)
    TfLotInfoRunMode  *cbSetupFileName;                       // [DATA] golden main.h:875 (TComboBox*) -- only ->Text used
    TfLotInfoEdit     *edWorkTemperBase;                      // [DATA] golden main.h:732 (TEdit*)
    // -- W906-P10 ADD (20260921): SetRunStartMode 會動到的那一組 ------------
    //    golden main.cpp:363-1115 的本體逐個用到下面這些；型別沿用本檔既有的
    //    門面別名（FormWidgets.h:132 TfMainPanel=TPanel、:169 TfLotInfoRunMode=
    //    TComboBox、:199 TfLotInfoEdit=TEdit），`Visible`/`Enabled` 來自共同基底
    //    `TControl`（vclcompat/Controls.h:215-216）。
    //    ⚠ 每一個的**實際被用到的欄位**都量過（grep golden body），不是照抄宣告：
    //      palFT/palRT/palEQC -> Caption / Color / Visible
    //      palOffLine         -> Color
    //      cbRunStartMode     -> Text / ItemIndex / Enabled
    //      cbbRunModeSel      -> Text / Enabled
    //      edSetOpenBin       -> Text / Visible
    //      lbSetOpenBin       -> Visible
    TfMainPanel       *palFT;                                 // [DATA] golden main.h (TPanel*) 開工模式燈：FT
    TfMainPanel       *palRT;                                 // [DATA] golden main.h (TPanel*) 開工模式燈：RT
    TfMainPanel       *palEQC;                                // [DATA] golden main.h (TPanel*) 開工模式燈：EQC
    TfMainPanel       *palOffLine;                            // [DATA] golden main.h (TPanel*) 離線燈
    TfLotInfoRunMode  *cbRunStartMode;                        // [DATA] golden main.h:707 (TComboBox*)
    TfLotInfoRunMode  *cbbRunModeSel;                         // [DATA] golden main.h (TComboBox*)
    TfLotInfoEdit     *edSetOpenBin;                          // [DATA] golden main.h (TEdit*)
    vclcompat::TLabel *lbSetOpenBin;                          // [DATA] golden main.h (TLabel*) 只用 ->Visible
    // -- 兩個方法：golden 在 SetRunStartMode 尾端呼叫 ----------------------
    //    ⚠ 兩者都是**離線空樁**，不是翻譯：
    //      SaveRunMode            golden 把當前模式寫回 LastSet（檔案 I/O）
    //      ChangeStateUploadServer golden 通知上傳伺服器狀態變更（網路）
    //    這兩條路在本樹都還沒有相依（LastSet 的寫入端與 upload server 都沒翻），
    //    所以先給空樁並在此標明；等那兩層進來再換真本體。
    void SaveRunMode()             {}                         // golden main.h -- TODO(W906-P10b): LastSet 寫回
    void ChangeStateUploadServer() {}                         // golden main.h -- TODO(W906-P10b): upload server 通知
    // -- W906-FW1d ADD (20260820): the login-level mirror pair --------------
    //    cbUserSelect reuses TfLotInfoRunMode (same only-`->Text` shape as
    //    cbSetupFileName above; golden main.h TComboBox*). DoChangeLevel is
    //    golden main.cpp:15127-15148 translated FAITHFULLY (clamp AccessLevel,
    //    then mirror it into cbUserSelect->Text via one of golden's three
    //    name tables). ChangeLevelAttr above STAYS the documented no-op --
    //    golden's ChangeLevelAttr calls DoChangeLevel as its first line
    //    (main.cpp:12410) and then does level-attr UI work this facade does
    //    not model; callers that need the mirror (tools/wb_serve auth
    //    dispatch) call DoChangeLevel directly. Text starts "" (facade ctor
    //    default): golden always runs DoChangeLevel early via
    //    ChangeLevelAttr, so pre-login "" is the honest offline state, the
    //    same posture as control.owner's empty string.
    TfLotInfoRunMode  *cbUserSelect;                          // [DATA] golden main.h (TComboBox*) -- only ->Text used
    virtual void DoChangeLevel();                              // [METHOD] golden main.cpp:15127-15148, faithful
    virtual bool Home(AnsiString Func);                        // [METHOD] golden main.h:1276 -- offline: no real Home cycle to run -> false
    // -- W906-TesterTCPTimer ADD (20260720): members Interface/TesterTCP_Socket.cpp's
    //    TimerProcessTCPDataTimer/SimulateBin deref -------------------------------
    TStringList *tTestResult;       // [DATA] golden main.h:1392 (TStringList*) -- ctor pre-fills 32x"-1" (golden main.cpp:2236-2239)
    TStringList *tBarCodeList;      // [DATA] golden main.h:1396 (TStringList*) -- ctor pre-fills 32x"0" then
                                    //   Strings[31]+=";" => "0;" (golden main.cpp:2241 quirk, PRESERVED --
                                    //   makes the default BARCODE? reply end "...,0;;")
    AnsiString SVID1190_OSSetup;    // [DATA] golden main.h:1499 -- SECS SVID1190 backing store (SV consumer
                                    //   uHGemHT9045_SV.cpp:231 untranslated; plain storage here)
    virtual void WritePERSITETemperature(); // [METHOD] golden main.h:1365 (void __fastcall; body Command.cpp:935-943)
                                    //   -- WRAPPER translated faithfully; leaf gated (below)
    virtual AnsiString PERSITETemperatureStrings();  // [METHOD] golden main.h (body Command.cpp:945-1482, ~538 lines +
                                    //   RefreshTempData) -- GATED LEAF: offline returns
                                    //   W906_PERSITETemperatureStrings_Sim (default ""), real body is its own
                                    //   future wave (temp/GPIB surface: fContact->fShow/IndexStatus/
                                    //   iContactMode/asGPIBTempShow/bTestSiteUse)
    AnsiString W906_PERSITETemperatureStrings_Sim;  // [PORT-ONLY SEAM] test-settable stand-in feed for the
                                    //   gated leaf above (same data-driven-facade idiom as GetSamSungMap
                                    //   ""-default / GetSamSungSoakTime "0"-default); default ""
    // -- W906-AutoCleanFoundation ADD (20260721): members the AutoClean foundation
    //    wave's translated functions (AutoClean/AutoClean.cpp) deref. Per-member
    //    touch-scope note (verified by grepping golden -- see that wave's own
    //    report for detail): ONLY chkCleanPadPickErr is actually dereferenced by
    //    a function THAT wave translated (CheckInSuckICFallDown, inside
    //    #ifdef SOFT_SIMULTE). AddAutoCleanMessage / bAutoCleanTest / cbIndexDrop /
    //    pnlCleanCount / AutoCleanContactCountLabel / edHPX / edHPY are touched
    //    only by the 4 named core engines (DoAutoCleanKit /
    //    DoAutoCleanPickfromCleanKit / DoIndexAutoClean(+variant) / EnableAutoclean /
    //    SearchCleanNum) -- pre-staged at zero behavioural risk (plain data /
    //    no-op sinks, same idiom as every other fMain member).
    virtual void AddAutoCleanMessage(AnsiString S); // [METHOD] golden AutoClean.cpp -- offline log sink no-op
    bool bAutoCleanTest;                          // [DATA]   golden main.h -- offline default false
    TfMainCheckBox *cbIndexDrop;                  // [DATA]   golden main.h:117 (TCheckBox*) -- offline Checked=false
    TfMainCheckBox *chkCleanPadPickErr;           // [DATA]   golden main.h:132 (TCheckBox*) -- offline Checked=false; ACTIVE (CheckInSuckICFallDown, SOFT_SIMULTE-gated)
    TfMainPanel    *pnlCleanCount;                 // [DATA]   golden main.h:864 (TPanel*) -- Caption only touched here; ->Font->Color is SearchCleanNum's, see pnlCleanCountFont below
    TfMainFont     *pnlCleanCountFont;              // [DATA]   golden main.h (TPanel->Font, TFont*) -- SearchCleanNum sets clRed/clNavy
    TfMainPanel    *AutoCleanContactCountLabel;    // [DATA]   golden main.h:148 (TLabel* -- see forms/FormWidgets.h's TfMainPanel conflation note); Caption only (EnableAutoclean)
    TfLotInfoEdit  *edHPX;                        // [DATA]   golden main.h:113 (TEdit*) -- DoIndexAutoClean
    TfLotInfoEdit  *edHPY;                        // [DATA]   golden main.h:114 (TEdit*) -- DoIndexAutoClean
    TfMainAutoCleanGrid *tmyAutoClean;             // [DATA]   golden main.h:347 (TTMyTray* clean-kit grid -- the "THeatTable*" this comment used to
                                                    //   carry was wrong; corrected by W7-F0 per plan SS10-5, verified directly against golden).
                                                    //   Write-only (confirmed by grep: SetAutoCleanICCount/SetAutoCleanTrayPosition only WRITE
                                                    //   XItem/YItem/Top/Width/Height, never read back inside AutoClean.cpp; a pure no-op/plain-data
                                                    //   sink is faithful). ->SetCellColorIndex IS called (Part D's SetCleanCellValue helper,
                                                    //   iMode==eUcleanUsed branch) -- reuses the SAME no-op idiom as htShullte0/1.
    // AI(W906-AutoCleanCluster) 20260722: golden main.h:164 `TTMyTray *mtPlate2;`
    // -- the HotPlate-2 clean-kit grid widget SetAutoCleanTrayPosition's
    // bE43AutoCleanUseHotplate branch reads (->Top/->Width/->Height only,
    // confirmed by grep). Sibling widget of tmyAutoClean above (both are golden
    // TTMyTray*), so reuses the SAME TfMainAutoCleanGrid stand-in.
    TfMainAutoCleanGrid *mtPlate2;                  // [DATA]   golden main.h:164 (TTMyTray*) -- read-only here (Top/Width/Height)
    TStringGrid *AutoCleanStringGrid;               // [DATA]   golden main.h:457 (TStringGrid*) -- REAL backing store (see forms/FormWidgets.h banner);
                                                    //   RestoreCleanKitData/CheckCleaningCount read Cells[][] back via atoi(). Default-constructed
                                                    //   5x5 (vclcompat default); a future wave's SetAutoCleanICCount translation resizes it via
                                                    //   ->ColCount=/->RowCount=.
    // AI(W906-W7-F1) 20260729: W7-F1 ADD -- members SECSGEM/uHGemHT9045.cpp's
    //    22-override layer dereferences that the facade lacked -- plan SS6-F1
    //    / SS4-V1 ("Wall 2"). Re-derived by grepping golden directly rather
    //    than trusting the plan's own inventory: golden touches 28 distinct
    //    `fMain->` spellings total, but 2 of those (PPID / bNeedClearFile,
    //    golden SECSGEM/uHGemHT9045.cpp:5311-5312) are inside `//`-commented-
    //    out code with no live call site, so they are NOT added here -- doing
    //    so would be inventing a member nothing dereferences. Of the
    //    remaining 26 live members, this wave found 3 more missing than the
    //    plan's own named list (ChangePassword/FTClick/RTClick).
    //    F1's OWN scope is the facade SURFACE only -- the 22 virtual
    //    overrides that will actually call these members are translated in a
    //    LATER wave (explicitly out of scope here per the task brief). Every
    //    body below is therefore a documented no-op or a feedable
    //    `W906_..._Sim`/call-count seam (per the fMain.h contract's rule 4
    //    above), never a silent implementation of golden's real logic.
    //
    //    AI(W906-W7-F1fix) 20260729 -- CORRECTION: the "Wall 2" unlock this
    //    facade surface provides is REAL but NARROWER than the prose above
    //    implies.
    //
    //    AI(W906-W7-F1fix3) 20260731 -- THAT CORRECTION'S FIGURES WERE STILL
    //    WRONG, and the failure mode is named here so it stops recurring: it
    //    said "9 of the 22 overrides touch fMain, and 5 of those are FULLY
    //    unblocked because they touch ONLY fMain". Its scan recognised only
    //    `//` comments and NEVER `/* */` BLOCK comments. Two of that 9 --
    //    ProcessS7F23FromatReceipe and ProcessS7F25FromatReceipe -- have NO
    //    live fMain dereference at all: their only fMain sites sit inside block
    //    comments that OPEN at golden uHGemHT9045.cpp:5844 (S7F23 --
    //    fMain->cbSetupFileName at :5850/:5852) and :5961 (S7F25 -- the same
    //    member at :5966/:5968). Re-derived this wave with a character-level
    //    comment-AND-string-aware scan of the cp950-decoded golden over the
    //    brace-matched bodies of all 22 virtuals (golden uHGemHT9045.h:346-365
    //    + :367-368), the real figures are:
    //      * SEVEN overrides live-dereference fMain: ReloadParameter,
    //        LookForFile, S2F15_CheckNewEquipmentConstant,
    //        S2F15_UpdateNewEquipmentConstant, S2F42_Host_Command_Acknowledge
    //        (body golden uHGemHT9045.cpp:1146-4189), AddSV, AddEC.
    //      * THREE of those seven touch ONLY fMain and every member they need
    //        is present here today: ReloadParameter (LoadTestModePicture,
    //        UpdateMainOperateMode, LoadRunModePicture, LoadStartModePicture),
    //        LookForFile (cbSetupFileName, LookForFile) and
    //        S2F15_CheckNewEquipmentConstant (CanChangeSite). "Unblocked" means
    //        on the FORM-FACADE axis only -- non-form dependencies are a
    //        separate question.
    //      * The other FOUR are blocked on far more than the "+fLotInfo /
    //        +fSetup / +fSCKART / +fNote" the old note listed:
    //        S2F15_UpdateNewEquipmentConstant on 8 other live form pointers,
    //        S2F42_Host_Command_Acknowledge on 14, AddSV on 8 (plus the 3
    //        missing fMain widgets in the fix2 block below), AddEC on 7.
    //      * TWO more overrides are form-blocked WITHOUT touching fMain and were
    //        missing from every earlier list: S7F4_ProcessProgramAcknowledge
    //        (fLotInfo/fOffSet/fSetup) and S125F4_LevelSettingChangeAcknowledge
    //        (fSecurity). The remaining 13 have zero live form dereference.
    //      * Across the 22 bodies, 29 distinct form pointers are live-
    //        dereferenced; PORTED/forms holds 9 form headers, so 21 of those 29
    //        have NO facade header at all. fMain is the only one that is nearly
    //        complete (3 of the 32 members these bodies need are missing).
    //    The full per-override / per-form table, with golden line citations and
    //    the LOWER-BOUND caveat on the per-header "missing" counts, lives in
    //    tests/test_w7_f1_wall2_probe.cpp's header; see also
    //    docs/W7_UI_ARCHITECTURE_PLAN.md SS10.
    //
    //    AI(W906-W7-F1fix2) 20260729 -- the block above is scoped to golden
    //    uHGemHT9045.cpp only, and that hides one gap: AddSV lives in
    //    uHGemHT9045_SV.cpp, which derefs THREE fMain members this facade
    //    still does not have -- edTorue0 (golden main.h:466, TEdit*, used at
    //    uHGemHT9045_SV.cpp:74), edTorue1 (main.h:467, TEdit*, :75),
    //    lbEPenconder (main.h:796, TPanel*, :100). All three are passed as bare
    //    WIDGET POINTERS into HGemPtr->SetSVDataPointer(SVID 1012/1013/1041),
    //    i.e. straight into the plan SS9-R8 void*-overload hazard the F0-a
    //    TObject base / F0-b static_assert exist to contain -- whoever adds
    //    them must go through the same widget stand-in types, not raw pointers.
    //    (Its other three, SVID1190_OSSetup/palMainStatus/tTestResult,
    //    are already here from earlier waves; uHGemHT9045_EC.cpp needs only
    //    cbSetupFileName + tSiteOnOff, both present.) So AddSV is blocked on
    //    the fMain side as well as by fLotInfo, and the "26 live members" count
    //    above is a per-file figure, not the whole SECSGEM layer: across all
    //    three golden SECSGEM TUs it is 34 distinct / 32 live / 29 present.
    //    These 3 are deliberately NOT added here -- adding members no
    //    translated code dereferences yet is exactly the "inventing surface"
    //    this wave refused to do for PPID/bNeedClearFile; they belong to
    //    whichever wave translates AddSV.
    virtual void cbSetupFileNameChange(void *Sender); // [METHOD] golden main.h:945 (body main.cpp:24643-24939, a 297-line
                                    //   recipe-reload cascade -- opens with ChangeSetUpFile() but the bulk of the 297
                                    //   lines is cbSetupFileNameChange's OWN body, both untranslated) -- offline: no-op
                                    //   that increments the call-count seam below so a test can observe the call happened
    int  W906_cbSetupFileNameChangeCallCount;     // [PORT-ONLY SEAM] call counter, default 0
    virtual void Clarn_Data(int Tag, AnsiString Msg=""); // [METHOD] golden main.h:1246 (body main.cpp:14925, per-day
                                    //   production-count file writer -- untranslated) -- offline: no-op call-count seam
    int  W906_Clarn_DataCallCount;                // [PORT-ONLY SEAM] call counter, default 0
    virtual void BtnPauseClick(void *Sender); // [METHOD] golden main.h:917 (body main.cpp:6965) -- TRANSLATED (partial,
                                    //   faithful): forwards to Pause("BtnPauseClick"), same call golden itself makes
                                    //   first. Golden's `#ifndef SOFT_SIMULTE` fProductionInfo->ClickPause() second
                                    //   line is NOT translated (fProductionInfo has no facade home yet) -- documented
                                    //   gap, not a silent drop.
                                    //   AI(W906-W7-F1fix) 20260729: the pre-existing TfMain::Pause it forwards to is
                                    //   itself a bare no-op (offline always returns false, no observable side
                                    //   effect) -- so without a seam of its OWN, a test could delete this entire
                                    //   forwarding call and no assertion anywhere would notice. Added the call-count
                                    //   seam below, same idiom as every other member in this wave.
                                    //   AI(W906-W7-F1fix2) 20260729 -- CORRECTION to the sentence this replaced,
                                    //   which claimed the counter below made "the forward itself" observable: it
                                    //   does NOT. W906_BtnPauseClickCallCount observes only that BtnPauseClick was
                                    //   CALLED; deleting the `Pause("BtnPauseClick")` forward leaves it at 1.
                                    //   Personally reproduced: compiled a fMain.cpp with ONLY that forward removed,
                                    //   ar-replaced fMain.cpp.obj in libht9045_forms.a, relinked the probe -- the
                                    //   check "BtnPauseClick() increments its own call-count seam" still PASSED,
                                    //   i.e. every pre-fix2 assertion stayed green with the forward gone. The
                                    //   forward is made
                                    //   observable by the W906_PauseCallCount / W906_PauseLastFunc seams on Pause()
                                    //   itself (declared next to Pause near the top of this class), which the probe
                                    //   now asserts; those DO go red when the forward is deleted.
    int  W906_BtnPauseClickCallCount;         // [PORT-ONLY SEAM] call counter, default 0 -- observes that
                                    //   BtnPauseClick ran at all; the forward INTO Pause() is observed separately by
                                    //   W906_PauseCallCount / W906_PauseLastFunc
    virtual void LoadRunModePicture(); // [METHOD] golden main.h:1302 (body main.cpp:12382, run-mode BMP picture
                                    //   selection UI -- untranslated) -- offline: no-op call-count seam
    int  W906_LoadRunModePictureCallCount;        // [PORT-ONLY SEAM] call counter, default 0
    virtual bool CanChangeSite(bool bNoIncludeHotplate=false); // [METHOD] golden main.h:1325 (body main.cpp:14350-14393) --
                                    //   GATED LEAF: real body has FOUR false-return paths, not just HasIC() tests.
                                    //   Three are live-IC guards (InArmSuck.HasIC() / InputShuttleHasIC() /
                                    //   IndexHasIC() / MOT[MMPlate1/2].HasIC(), none of which have a facade path into
                                    //   this TU). The FOURTH, in the bCanAutoCloseSite==false /
                                    //   IniConfig.bI28_OnOffSiteOnTheFly==true sub-branch (main.cpp:14383-14390), is
                                    //   NOT a HasIC() test at all: it returns false on
                                    //   `bPickFromLoader==true || iPickFromLoadStageTask!=1`. Offline returns
                                    //   W906_CanChangeSite_Sim (default true), matching golden's own fall-through
                                    //   when every one of those four guards is false (the offline-everywhere posture
                                    //   used tree-wide, e.g. cInplace/InArmPlacementEnable above).
                                    //   AI(W906-W7-F1fix) 20260729: a complete, already-tested translation of this
                                    //   SAME golden function exists as ComputeCanChangeSite (MainCalcCore.h/.cpp,
                                    //   covering all four paths above, exercised by tests/test_MainCalcCore.cpp).
                                    //   This facade member does NOT delegate to it: ComputeCanChangeSite is a pure
                                    //   function that takes the live global state (bCanAutoCloseSite,
                                    //   IniConfig.bI28_OnOffSiteOnTheFly, InArmSuck.HasIC(), InputShuttleHasIC(),
                                    //   IndexHasIC(), MOT[MMPlate1/2].HasIC(), bPickFromLoader,
                                    //   iPickFromLoadStageTask) as PARAMETERS, and none of those symbols are visible
                                    //   from ht9045_forms (the bottom layer this TU compiles into) -- wiring them in
                                    //   would require ht9045_forms to link upward into the modules that define them,
                                    //   recreating exactly the kind of cycle W7-F0 removed. A fresh Sim-seam stand-in
                                    //   is therefore the correct shape here, not a missed reuse opportunity; the
                                    //   eventual TfMain::CanChangeSite override translation (once fMain gains a real
                                    //   binder with access to those globals) should call ComputeCanChangeSite
                                    //   instead of re-deriving the branch tree.
                                    //   AI(W906-W7-F1fix3) 20260731 -- the duplication is THREE-WAY, not two-way, so
                                    //   do not read the note above as "resolved at two". The third live implementation
                                    //   is Automation/auto9045.cpp:197-202, W5FA_TfMainExt::CanChangeSite() -- a
                                    //   ZERO-argument member whose body is a literal `return true;` under a "golden
                                    //   main.cpp body unavailable this wave" JUDGMENT CALL comment, with a LIVE call
                                    //   site 487 lines below it at auto9045.cpp:684
                                    //   (`if(W5FA_FMain.CanChangeSite()==false) { return 3; }`). Verified by grepping
                                    //   the whole ported tree: those are the only three CanChangeSite bodies
                                    //   (this member + ComputeCanChangeSite + the auto9045 stub), and the auto9045 one
                                    //   is the only one that is NOT test-drivable -- being a literal constant it has
                                    //   no seam, so its `return 3` path is unreachable in any test. Whoever gives
                                    //   fMain a real binder should collapse all three onto ComputeCanChangeSite.
    bool W906_CanChangeSite_Sim;                  // [PORT-ONLY SEAM] test-settable return, default true
    virtual void BtnTrayEndClick(void *Sender); // [METHOD] golden main.h:925 (body main.cpp:13944 -- one line,
                                    //   InitialTrayFeedTask("BtnTrayEndClick"), itself untranslated: golden main.h:1245
                                    //   declares InitialTrayFeedTask as a TfMain MEMBER function, not a free function;
                                    //   csystem.cpp's W7C2_FMAIN_INITIALTRAYFEED TU-local macro is the existing
                                    //   stand-in for that member) -- offline: no-op call-count seam
    int  W906_BtnTrayEndClickCallCount;           // [PORT-ONLY SEAM] call counter, default 0
    virtual void UpdateMainOperateMode(); // [METHOD] golden main.h:1236 (body main.cpp:12803-13127, ~325-line hardware
                                    //   relay/IO ladder -- ATC site-use relays, edWorkTemperBase/edSoakTime
                                    //   enable-locks, WriteLastDataFile/ReadLastDataFile, ChangeATCSiteUse -- already
                                    //   documented as out of scope by SetMainRunStartMode's own comment above) --
                                    //   offline: no-op call-count seam
    int  W906_UpdateMainOperateModeCallCount;  void TemperatureEditDisable(); void ChangeATCSiteUse(); void SetNormalOrPrime();  void W906_UpdateMainOperateModeBody();   // AI(W906-OPMODE) 20260926: golden main.h:1329／:1355／:1475 三個成員（本體在 forms/fMain_OperateMode.cpp、forms/fMain_ATCSiteUse.cpp；同一行宣告，行數不變）   // [PORT-ONLY SEAM] call counter, default 0
    virtual void LoadStartModePicture(); // [METHOD] golden main.h:1304 (body main.cpp:23827, start-mode BMP picture
                                    //   selection UI -- untranslated) -- offline: no-op call-count seam
    int  W906_LoadStartModePictureCallCount;      // [PORT-ONLY SEAM] call counter, default 0
    virtual void LookForFile(); // [METHOD] golden main.h:1300 (body main.cpp:9016, Offset-directory filesystem
                                    //   scan/migration -- untranslated) -- offline: no-op call-count seam
    int  W906_LookForFileCallCount;               // [PORT-ONLY SEAM] call counter, default 0
    virtual int ChangeTesterConnect(int Mode, bool Msg=true, bool bRemote=false); // [METHOD] golden main.h:1322
                                    //   AI(W906-GB-P2d) 20260926: REAL BODY now -- golden 912 main.cpp:12581-12778 translated at
                                    //   the end of forms/fMain.cpp under the user-ruled P2d scope (switch to Off-Line, Off-Line ->
                                    //   On-Line, the tail; the access check, MES1646, Manual Sort, SPIL logout, 2D_SORT and the
                                    //   ASM On-Line arm are gated TODO; RTC keeps the config.ini write without the restart message).
                                    //   W906_ChangeTesterConnect_Sim != 0 still short-circuits it (test override).
                                    //   The historical note below predates P2d.
                                    //   AI(W906-W7-F1fix) 20260729 -- CORRECTION (re-verified against the ported tree,
                                    //   not just golden): this is NOT the sole ported-tree stand-in for golden's
                                    //   fMain->ChangeTesterConnect, and the caller does NOT discard the return value.
                                    //   There are two independent TU-local stand-ins today, neither of which calls
                                    //   THIS facade member: (1) Automation/auto9045.cpp's SetTesterConnect symbol
                                    //   `return`s (does not discard) the result of its OWN TU-local
                                    //   W5FA_TfMainExt::ChangeTesterConnect stub straight to ITS OWN caller; (2)
                                    //   csystem.cpp's W7C2_FMAIN_CHANGETESTERCONNECT TU-local macro (a second,
                                    //   separate gate, near the "onLine switch" call site) discards its int ARGUMENT,
                                    //   not a return value, and does not call this member either. Nothing in the
                                    //   ported tree calls this new facade member directly yet -- it exists so a
                                    //   future SECSGEM override translation (golden uHGemHT9045.cpp:859) has
                                    //   somewhere to land, per this wave's own gate probe.
    int  W906_ChangeTesterConnect_Sim;            // [PORT-ONLY SEAM] != 0 = test override (returned without running the body); 0 = golden body (AI(W906-GB-P2d))
    virtual int SetTemp(bool bAsk, double fWorkTemp, double fSoakTime); // [METHOD] golden main.h:1321 (body
                                    //   main.cpp:23890) -- GATED LEAF: real body returns 1 on SystemStart==true,
                                    //   then walks a ShowMyMessageBox_YES_NO confirm + ChangeTempMode cascade with no
                                    //   facade path from this TU. Offline returns W906_SetTemp_Sim (default 0, the
                                    //   golden success code).
    int  W906_SetTemp_Sim;                        // [PORT-ONLY SEAM] test-settable return, default 0
    virtual void ChangePassword(); // [METHOD] golden main.h:1417 (body main.cpp:31772, SECS/GEM password-file
                                    //   loader -- untranslated) -- offline: no-op call-count seam
    int  W906_ChangePasswordCallCount;            // [PORT-ONLY SEAM] call counter, default 0
    virtual int FTClick(bool bMan=false); // [METHOD] golden main.h:1567 (body main.cpp:29666) -- GATED LEAF: real
                                    //   body returns 1 on SystemStart==true, 2/3/4/8 on assorted live-IC/level/tray
                                    //   blocks (InArmSuck.HasIC() etc., none reachable from this TU), 0 on the
                                    //   fall-through success path (which also calls Clarn_Data() above and
                                    //   EventReport() -- both already-real/no-op sinks here). Offline returns
                                    //   W906_FTClick_Sim (default 0, the golden success code).
    int  W906_FTClick_Sim;                        // [PORT-ONLY SEAM] test-settable return, default 0
    virtual int RTClick(bool bMan=false); // [METHOD] golden main.h:1568 (body main.cpp:29790-29954) -- sibling of
                                    //   FTClick above (same untranslated live-IC-branch shape), but NOT the same
                                    //   return-code shape: FTClick returns one of {0,1,2,3,4,8}; RTClick returns one
                                    //   of {0,1,2,3,4,5,6,7,8} -- three codes (5, 6, 7) that FTClick never returns.
                                    //   AI(W906-W7-F1fix2) 20260729, per-code attribution re-derived from golden
                                    //   (the previous one lumped 5 and 6 together as "the MOT[MMTrayY] tray/IC
                                    //   guard", which is only true of 6): 7 = the IniConfig.bSPILFunction &&
                                    //   bCanRunSCKART gate at the top (main.cpp:29795-29797, no FTClick counterpart);
                                    //   5 = the iSecsGemSwitchFTRT==0 && cbRunStartMode->Enabled==false guard
                                    //   (:29866-29868), which FTClick numbers 3 (:29721-29723); 6 = the
                                    //   MOT[MMTrayY]/MOT[MMTrayY_Car] tray/IC guard (:29870-29874), which FTClick
                                    //   numbers 4 (:29725-29729). Offline returns W906_RTClick_Sim (default 0).
    int  W906_RTClick_Sim;                        // [PORT-ONLY SEAM] test-settable return, default 0
    TStringList *tSiteOnOff[2];                   // [DATA] golden main.h:1393 (TStringList*[2]) -- REAL concrete
                                    //   storage, same idiom as tTestResult/tBarCodeList above: ctor prefills BOTH
                                    //   with MAX_SOCKET_ROW*MAX_SOCKET_COL (golden main.cpp:2242-2248) "0" strings so
                                    //   uHGemHT9045.cpp's `if(z<fMain->tSiteOnOff[0]->Count)` guard and
                                    //   `->Strings[z]` read are satisfiable exactly as golden expects.
    TfLotInfoEdit *edSoakTime;                    // [DATA] golden main.h:733 (TEdit*) -- reuses TfLotInfoEdit, same
                                    //   stand-in already used for edWorkTemperBase/edHPX/edHPY (only ->Text read/written).
    // AI(W906-W7-L1-Wave0) 20260801: W7-L1 Wave-0 ADD -- the 16 fMain members the
    //    six asendic_* tray SM files (Loader / Loader_RT / Color / Auto / Auto_RT,
    //    plus the shared asendic.cpp substrate) dereference and this facade did
    //    not have.  Landed in ONE serialized pass ahead of the four parallel
    //    translation agents so they cannot collide on this header.  Every golden
    //    line below was re-read from the cp950-decoded golden main.h in THIS pass,
    //    not taken from the recon report.
    //
    //    WIDGET-TYPE NOTE (this is why all 7 tray-count labels use
    //    TfMainTrayPanel, not TfMainPanel): golden main.h:392 and :393-395 /
    //    :861-863 are ALL `TLabel*`, one adjacent golden family.  TfMainTrayPanel
    //    is the TLabel-backed alias (forms/FormWidgets.h:143 -> vclcompat::TLabel);
    //    TfMainPanel is TPanel-backed (:132).  Using the TPanel alias would have
    //    propagated into 7 new members exactly the conflation FormWidgets.h:127-130
    //    already records as W7-U debt for AutoCleanContactCountLabel.
    //    All 7 are Caption-write-only in golden (asendic_Loader.cpp:1229/:2082/
    //    :2105/:2849/:2861; asendic_Auto.cpp:537-542/:612-617/:2237-2242).
    TfMainTrayPanel *lblLoadTrayCnt;              // [DATA] golden main.h:392 (TLabel*) -- Caption written only
    TfMainTrayPanel *lblAuto1TrayCnt;             // [DATA] golden main.h:393 (TLabel*)
    TfMainTrayPanel *lblAuto2TrayCnt;             // [DATA] golden main.h:394 (TLabel*)
    TfMainTrayPanel *lblAuto3TrayCnt;             // [DATA] golden main.h:395 (TLabel*)
    TfMainTrayPanel *lblAuto4TrayCnt;             // [DATA] golden main.h:861 (TLabel*)
    TfMainTrayPanel *lblAuto5TrayCnt;             // [DATA] golden main.h:862 (TLabel*)
    TfMainTrayPanel *lblAuto6TrayCnt;             // [DATA] golden main.h:863 (TLabel*)
    //    The 6 Auto edits are golden TEdit* and only ->Text is written
    //    (asendic_Auto.cpp:545-550), so they reuse TfLotInfoEdit -- the same
    //    cross-form alias fMain->edWorkTemperBase / edHPX / edHPY already use,
    //    i.e. established precedent rather than a new conflation.
    TfLotInfoEdit *edtAuto1;                      // [DATA] golden main.h:867 (TEdit*) -- Text written only
    TfLotInfoEdit *edtAuto2;                      // [DATA] golden main.h:373 (TEdit*)
    TfLotInfoEdit *edtAuto3;                      // [DATA] golden main.h:374 (TEdit*)
    TfLotInfoEdit *edtAuto4;                      // [DATA] golden main.h:376 (TEdit*)
    TfLotInfoEdit *edtAuto5;                      // [DATA] golden main.h:377 (TEdit*)
    TfLotInfoEdit *edtAuto6;                      // [DATA] golden main.h:378 (TEdit*)
    TfMainCheckBox *chkE84IDTray;                 // [DATA] golden main.h:887 (TCheckBox*) -- Checked read
                                    //   (asendic_Color.cpp:436) AND written (:649); offline default false
    //    StringGrid2 -- REAL backing store, and its SIZE is load-bearing.
    //    (history) vclcompat::TStringGrid used to store Cells in vectors and index them with
    //    vector::at (out-of-range THREW; AI(W906-W3-6b) 20260925: now a sparse store like BCB6, no throw)
    //    (vclcompat/StringGrid.h:43-44) instead of silently growing.  golden
    //    asendic_Color.cpp:831 writes Cells[3][38].  The ctor size is taken from
    //    golden's own form resource, not guessed: main.dfm's `object StringGrid2:
    //    TStringGrid` block sets ColCount = 8 and RowCount = 60 (main.dfm:15440,
    //    :15447, :15451 -- read this pass), so forms/fMain.cpp constructs it
    //    `new TStringGrid(8, 60)`, which covers [3][38] with golden's real margin.
    TStringGrid *StringGrid2;                     // [DATA] golden main.h:490 (TStringGrid*) -- 8x60 per main.dfm:15447/:15451
    TfMainRENESASServer *RENESAS_Server;          // [DATA] golden main.h:1710 (TRENESAS_Server*) -- see the nested class above
    //    DELIBERATELY NOT ADDED -- `TfMainCheckBox *CheckBox1` (golden main.h:361,   ⚠ AI(W906-SENSORSCAN) 20260924: 本段（:668-675）已過期 —— SOFT_SIMULTE 現由 MachineType.h:63-64 定義（未設 W906_NO_SOFT_SIMULTE 時），asendic_Loader.cpp 的 #ifdef SOFT_SIMULTE 臂會編；CheckBox1 已於下方 :1179 加入（W906-ST-S3-B2b），cSensorScan.cpp 的 ProcessSensorScan 每個 tick 讀它
    //    TCheckBox*, verified this pass).  Its ONLY reference in the whole W7-L1
    //    family is asendic_Loader.cpp:2942, which sits inside the
    //    `#ifdef SOFT_SIMULTE` block at :2941-2947; SOFT_SIMULTE is undefined in
    //    this tree, so that block is not compiled and the member would be surface
    //    nothing dereferences.  Same rule that kept PPID / bNeedClearFile out in
    //    W7-F1, and the same precedent this header's own `cb1` note records.
    //    Listed so a later wave does not "discover" it as an omission.
    // AI(W906-W7-L2) 20260803: W7-L2 ADD -- the three fMain members golden
    //    ckernel.cpp dereferences and this facade did not have.  Every golden
    //    line cited below was re-read from the cp950-decoded golden in THIS
    //    pass (main.h / main.cpp / main.dfm / ckernel.cpp / elec\myvcl\butPa1.h).
    //
    // (a) MainFormChange -- golden main.h:1261 `void __fastcall MainFormChange();`
    //     Sole golden caller in this front: ckernel.cpp:367, inside
    //     ScanSystemSensor's one-shot `if(SoftStart==true)` startup block
    //     (ckernel.cpp:365-533 -- an earlier comment in this same wave gave the
    //     end as :379, which is wrong by 154 lines; brace-matched this pass with
    //     comments and string literals masked: `if` at :365, body `{` at :366,
    //     closing `}` at :533, and the `else if(SoftStop==true)` arm opens at
    //     :534.  :379 is only the mid-block `SoftStop=false;`), immediately
    //     before AccelateTask=1 (:369) / ChangeUseSuckMode() (:370) /
    //     SetWorkParameter() (:371).
    //     Golden's REAL body is main.cpp:3883-4096 (214 lines) and it is PURE
    //     FORM REPAINT -- all 214 lines were read this pass, and its ONLY
    //     assignments are:
    //       1. `Ptr[i][j]->Visible=false` over a 16x8 LOCAL array of the form's
    //          own TALed members (array built main.cpp:3890-3914, hide pass
    //          :3922-3924);
    //       2. `PtrSort[i][j]->Visible=false` over the 2x8 9046AU sort-shuttle
    //          TALed array (built :3916-3920, hide pass :3926-3932);
    //       3. the re-show pass that turns back on only the rows/columns
    //          matching TestIF_File.iTestMode (:3934-4034, via the locals
    //          SingleRow/iCol), plus the IsNNMode()==NN_1Row (:4036-4055) and
    //          ==NN_2Row (:4057-4066) corrections;
    //       4. `labFailAlarmCnt->Visible` / `->Caption`, composed from
    //          IniConfig.bG04ShowFailAlarmCount + Prod.bContsFailBySocket /
    //          Prod.bContsFailByHead and their two counters (:4068-4094).
    //     It writes NO global, NO Prod/TestIF field, and drives no motor/IO:
    //     every write target is a TfMain-owned VCL widget with no facade home
    //     (e.g. golden main.h:313 `TALed *led_BLCarryKit_0;`, :889
    //     `TALed *led_SortShtKit_0;`, :673 `TLabel *labFailAlarmCnt;`).
    //     So the behaviour ELIDED offline is exactly "the site-map LED matrix
    //     and the fail-alarm-count label are not repainted to match the current
    //     iTestMode" -- unobservable to any headless caller.  That is why a
    //     no-op is faithful here rather than a silent drop, and it is the same
    //     shape as ProcessSensorScan (:210) / ChangeLevelAttr (:216) above.
    //     OBSERVABILITY GAP, stated rather than hidden (the lesson recorded in
    //     the W906-W7-F1fix2 block above): like those two neighbours this body
    //     has NO seam, so a test cannot tell "ckernel called MainFormChange"
    //     from "ckernel dropped the call".  Deliberate -- the brief for this
    //     wave was to copy the neighbours' shape exactly.  If the ckernel test
    //     plan needs to assert the call, the fix is the established one-liner:
    //     add `int W906_MainFormChangeCallCount;` here, init 0 in the ctor, and
    //     increment it in the body (same idiom as W906_LookForFileCallCount).
    virtual void MainFormChange();                // [METHOD] golden main.h:1261 (body main.cpp:3883-4096) -- offline: LED/label repaint no-op
    // (b) BtnSTEP / BtnT_Start -- golden main.h:102 `TBtnPanel *BtnSTEP;` and
    //     main.h:103 `TBtnPanel *BtnT_Start;`.  golden ckernel.cpp touches ONE
    //     property on each, ->Color, at exactly four sites (grep of ckernel.cpp
    //     for both names returns these four and nothing else), all inside the
    //     `if(CosFunction.bEnableSoftWareControlButton)` TSMC-only block
    //     (ChungHung 20150609, ckernel.cpp:66 / :108):
    //       WaitManualStepKey  (ckernel.cpp:54-94)
    //         :69  fMain->BtnSTEP->Color=clYellow;               // lamp ON
    //         :71  fMain->BtnSTEP->Color=(TColor)0x00804000;     // lamp OFF
    //       WaitManualStartKey (ckernel.cpp:96-131)
    //         :111 fMain->BtnT_Start->Color=clYellow;            // lamp ON
    //         :113 fMain->BtnT_Start->Color=(TColor)0x00804000;  // lamp OFF
    //     driven by bLampManualSetp / bLampManualStart = FlushFlag (:59 / :101).
    //     Write-only in golden: neither member is ever READ, there or anywhere
    //     in this ported tree (both names are new here).
    //
    //     WIDGET TYPE -- why TfMainPanel and NOT TfMainSpeedButton.  The tree's
    //     existing TBtnPanel stand-in IS TfMainSpeedButton (forms/FormWidgets.h
    //     :293-299, carrying fMain->BtnOneCycle, golden main.h:72), but it holds
    //     ONLY `bool Down` -- it has no Color, so it cannot store what these four
    //     sites write, and forms/FormWidgets.h is out of this wave's write scope.
    //     Of the stand-ins that DO expose a settable Color, TfMainPanel is the
    //     exact right one and not a fudge: golden's TBtnPanel is literally
    //     `class PACKAGE TBtnPanel : public TPanel` (golden elec\myvcl\butPa1.h:12,
    //     read this pass), so the `->Color` these sites write is the INHERITED
    //     stock TPanel::Color property -- and TfMainPanel is exactly
    //     vclcompat::TPanel (forms/FormWidgets.h:132 -> vclcompat/Controls.h:240-246,
    //     `int Color`).  That makes this conflation strictly tighter than the one
    //     forms/FormWidgets.h:136-141 already records and accepts for
    //     mtAuto1/mtAuto2/mtAuto3 (golden TTMyTray*, ->Color-only, aliased onto
    //     vclcompat::TLabel).  Per contract rule 3 no new widget TYPE is invented.
    //     REPORTED, NOT DONE (needs files this wave does not own): the right end
    //     state is ONE TBtnPanel stand-in carrying both Down and Color -- either
    //     give TfMainSpeedButton a `Color` member in forms/FormWidgets.h, or do
    //     the W7-C4 repoint of the whole family onto vclcompat::BtnPanelCore,
    //     which already models golden TBtnPanel faithfully (`TColor Color`,
    //     vclcompat/BtnPanelCore.h:79) but whose .cpp is NOT in the root
    //     CMakeLists `vclcompat` library today (only in two tests/ targets), so
    //     using it from here would break the link for every ht9045_forms consumer
    //     until that line is added.
    TfMainPanel *BtnSTEP;                         // [DATA] golden main.h:102 (TBtnPanel*) -- ->Color only; ctor seeds 0x00804000, see forms/fMain.cpp
    TfMainPanel *BtnT_Start;                      // [DATA] golden main.h:103 (TBtnPanel*) -- ->Color only; ctor seeds 0x00804000, see forms/fMain.cpp
    // -- AI(W906-PT-W3-integrate) 20260808 ADD: 2 methods + 1 field the PT-W3
    //    unit Automation/uRENESAS_Server.cpp derefs (its "FACADE ADDITIONS
    //    NEEDED" banner is the measurement; golden lines re-read this pass). --
    void EnabledSetupFile(bool bEnabled);         // [METHOD] golden main.h:1340 (body main.cpp:28310-28425, 116 lines)
                                    //   AI(W906-MSTATE-P2) 20260924: 原本是 virtual 空殼 {}（在 forms/fMain.cpp）；現在照 golden 翻，
                                    //   本體在 cMainStatus.cpp（ht9045_sm，因為要 HasICUnderMachine／HasAnyICInMachine）。
                                    //   改成非 virtual：virtual 的本體放進 sm 會讓 forms 的 vtable 反過來依賴 sm（分層違規）；
                                    //   全樹沒有任何覆寫（Grep 全樹 *.cpp/*.h 只有 fMain.h/.cpp、ckernel、csystem、uRENESAS 五檔）
    int  iHasChangeFile;                          // [DATA] golden main.h:1701 -- no golden ctor assignment (VCL
                                    //   zero-init; golden's own `==0` read at main.cpp:24887 means "never set");
                                    //   uRENESAS_Server writes 9 then polls ==1 (its case 30000)
    virtual void SetLotState(int iState);         // [METHOD] golden main.h:1327 (body main.cpp:15160-15250: pushes
                                    //   LOTNUMBER/LOTEND over TesterTCP when iTestType==TCP_IP_MODE, else GPIB
                                    //   MSG_CMD path; early-returns otherwise) -- offline no-op is behaviourally
                                    //   identical whenever iTestType is neither TCP_IP_MODE nor GPIB_MODE
    // -- FW3-WA ADD: Command.cpp wave-A declarations --------------------------
    // AI(W906-FW3-WA) 20260817: FW-3 Wave A ADD -- 60 golden TfMain:: PURE-method
    // declarations whose bodies are now translated for real in the NEW file
    // Command.cpp (golden Command.cpp:86-2242 / :3825-4005 / :5170-5638 minus
    // SetSiteMapData/SetAlarmSetup / :12063-12508).  3 sibling methods in this
    // same golden byte range (WritePERSITETemperature, PERSITETemperatureStrings,
    // ArmStatusStrings) are declared ABOVE already (pre-existing GATED-leaf /
    // wrapper stubs, bodies in forms/fMain.cpp) and are NOT redeclared here --
    // Command.cpp still carries a full-body translation of all three per the
    // wave brief, which is a KNOWN STUB COLLISION for the next integration pass
    // to retire (see this wave's report's STUB COLLISIONS section). Per contract
    // rule 1 every declaration below is `virtual`; per rule 4 each cites its
    // golden home. Signatures re-read from the cp950-decoded golden main.h this
    // pass (golden line numbers cited per member).
    virtual int RefreshTempData(bool bTransfer=false, int iArm=0, int iSite=0); // golden main.h:1388 (body Command.cpp:86-933)
    virtual void WriteHandlerID();                    // golden main.h:1367 (body Command.cpp:1484-1487)
    virtual void WriteArmStatus();                    // golden main.h:1369 (body Command.cpp:1489-1495)
    virtual void WriteArmForce();                     // golden main.h:1371 (body Command.cpp:1510-1541)
    virtual void WriteTempData();                     // golden main.h:1176 (body Command.cpp:1543-1549)
    virtual AnsiString TempDataStrings();             // golden main.h:1177 (body Command.cpp:1551-1586)
    virtual void WriteSetTempStatus();                // golden main.h:1178 (body Command.cpp:1588-1649)
    virtual void WriteSetTestTempStatus();            // golden main.h:1179 (body Command.cpp:1651-1764)
    virtual void WriteSoakTimeData();                 // golden main.h:1180 (body Command.cpp:1766-1775)
    virtual void WriteSetSoakTimeStatus();             // golden main.h:1181 (body Command.cpp:1777-1794)
    virtual AnsiString WriteSiteMapData(bool bGPIB=true); // golden main.h:1223 (body Command.cpp:1797-2028)
    virtual void WriteStartMode_NS();                 // golden main.h:1383 (body Command.cpp:2030-2060)
    virtual void WriteAssign_NS();                    // golden main.h:1384 (body Command.cpp:2062-2242)
    virtual void WriteForce_NS();                     // golden main.h:1386 (body Command.cpp:3825-3855)
    virtual AnsiString WriteBinMap(bool bGPIB=true);  // golden main.h:1374 (body Command.cpp:3858-3912)
    virtual void WriteSetBinMap(AnsiString BinData);  // golden main.h:1375 (body Command.cpp:3914-3987)
    virtual void WriteTestMode();                     // golden main.h:1376 (body Command.cpp:3989-4005)
    virtual void WriteChkSetup();                     // golden main.h:1420 (body Command.cpp:5170-5275)
    virtual void WriteHandlerTestArmEncoder();        // golden main.h:1421 (body Command.cpp:5277-5291)
    virtual void WriteHandlerTestArmEP();             // golden main.h:1422 (body Command.cpp:5293-5299)
    virtual void GetCZtesterBin();                    // golden main.h:1462 (body Command.cpp:5390-5417)
    virtual void GetCZSoakTime();                     // golden main.h:1463 (body Command.cpp:5419-5427)
    virtual void GetCZDoubleContactCount();           // golden main.h:1468 (body Command.cpp:5429-5438)
    virtual void GetCDHandlerID();                    // golden main.h:1464 (body Command.cpp:5440-5445)
    virtual void GetCZJamCode();                      // golden main.h:1465 (body Command.cpp:5447-5461)
    virtual void GetCZSiteMap(bool bSendGPIB=true);   // golden main.h:1466 (body Command.cpp:5463-5638)
    virtual void SetTesterID();                       // golden main.h:1594 (body Command.cpp:12063-12102)
    virtual void GetTesterID();                       // golden main.h:1595 (body Command.cpp:12104-12109)
    virtual void GetAutoClean();                      // golden main.h:1597 (body Command.cpp:12111-12116)
    virtual AnsiString AutoCleanStrings();             // golden main.h:1602 (body Command.cpp:12118-12128)
    virtual void GetForcePerPinN();                   // golden main.h:1603 (body Command.cpp:12130-12135)
    virtual AnsiString ForcePerPinNStrings();          // golden main.h:1604 (body Command.cpp:12137-12142)
    virtual void GetContactHeight();                  // golden main.h:1605 (body Command.cpp:12144-12149)
    virtual AnsiString ContactHeightStrings();         // golden main.h:1606 (body Command.cpp:12151-12156)
    virtual void GetYieldContinusFail();              // golden main.h:1607 (body Command.cpp:12158-12163)
    virtual AnsiString YieldContinusFailStrings();     // golden main.h:1608 (body Command.cpp:12165-12173)
    virtual void GetYieldSiteCompare();               // golden main.h:1609 (body Command.cpp:12175-12180)
    virtual AnsiString YieldSiteCompareStrings();      // golden main.h:1610 (body Command.cpp:12182-12190)
    virtual void GetDUTStaus();                       // golden main.h:1611 (body Command.cpp:12192-12197)
    virtual AnsiString DUTStausStrings();              // golden main.h:1612 (body Command.cpp:12199-12264)
    virtual void GetUPH();                            // golden main.h:1613 (body Command.cpp:12266-12271)
    virtual AnsiString UPHStrings();                   // golden main.h:1614 (body Command.cpp:12273-12285)
    virtual void GetIndexCycleTime();                 // golden main.h:1615 (body Command.cpp:12287-12292)
    virtual AnsiString IndexCycleTimeStrings();        // golden main.h:1616 (body Command.cpp:12294-12306)
    virtual void GetTempOfs();                        // golden main.h:1617 (body Command.cpp:12308-12313)
    virtual AnsiString TempOfsStrings();               // golden main.h:1618 (body Command.cpp:12315-12332)
    virtual void GetTempRange();                      // golden main.h:1619 (body Command.cpp:12334-12339)
    virtual AnsiString TempRangeStrings();             // golden main.h:1620 (body Command.cpp:12341-12347)
    virtual void GetVacuumAir();                      // golden main.h:1621 (body Command.cpp:12349-12354)
    virtual AnsiString VacuumAirStrings();             // golden main.h:1622 (body Command.cpp:12356-12363)
    virtual void GetAll();                            // golden main.h:1623 (body Command.cpp:12365-12385)
    virtual void GetHandlerVersion();                 // golden main.h:1624 (body Command.cpp:12387-12392)
    virtual AnsiString HandlerVersionStrings();        // golden main.h:1625 (body Command.cpp:12394-12399)
    virtual void UploadProdLog();                     // golden main.h:1534 (body Command.cpp:12401-12447)
    virtual void GetShuttleMode();                    // golden main.h:1635 (body Command.cpp:12449-12464)
    virtual void SetMaxTest();                        // golden main.h:1636 (body Command.cpp:12466-12478)
    virtual void GetMaxTest();                        // golden main.h:1637 (body Command.cpp:12480-12485)
    virtual void SetMaxInitialTest();                 // golden main.h:1638 (body Command.cpp:12487-12499)
    virtual void GetMaxInitialTest();                 // golden main.h:1639 (body Command.cpp:12501-12506)
    virtual AnsiString GetSiteState();                 // golden main.h:1644 (body Command.cpp:12508-12536)
    // -- end FW3-WA ADD --------------------------------------------------------
    // -- FW3-WB ADD: Command.cpp wave-B declarations ---------------------------
    // AI(W906-FW3-WB) 20260817: FW-3 Wave B ADD -- 3 golden TfMain:: PURE-method
    // declarations (the "giant-triplet", 4,402 golden lines total) whose bodies
    // are translated in Command.cpp's new "FW3-WB GROUP B" section (golden
    // Command.cpp :2244-3823 / :4007-5168 / :5640-7299). All three are pure
    // branch-logic + AnsiString::sprintf builders with a single tail
    // SendMSG_CMD -- zero VCL widget references (grep-verified, see this
    // wave's report) -- so unlike Wave A there are no GATED-partial members and
    // no STUB COLLISIONS to flag here. Per contract rule 1 both are `virtual`;
    // per rule 4 each cites its golden home. Signatures re-read from the
    // cp950-decoded golden main.h this pass (golden line numbers cited per
    // member).
    virtual void WriteTemp_NS();                      // golden main.h:1385 (body Command.cpp:3796-5375)
    virtual void WriteNowAllTempData();               // golden main.h:1419 (body Command.cpp:5378-6539)
    virtual void GetCZAllMassTemp();                  // golden main.h:1467 (body Command.cpp:6542-8201)
    // -- end FW3-WB ADD --------------------------------------------------------
    // -- FW3-WC ADD: Command.cpp wave-C declarations ---------------------------
    // AI(W906-FW3-WC) 20260818: FW-3 Wave C ADD -- the ByDLL family. 26 golden
    // TfMain:: member declarations (golden main.h :1440-1458 / :1527-1537 /
    // :1692-1695) whose bodies are translated in Command.cpp's new "FW3-WC
    // GROUP" section (golden Command.cpp :8311-9994). RemoteControl (golden
    // Command.cpp :9673-9717, golden main.h :1455) is EXCLUDED per this wave's
    // never-wave list -- not declared, not translated, no stand-in, no call
    // site anywhere in this tree. Per contract rule 1 every declaration below
    // is `virtual`; per rule 4 each cites its golden home. Signatures re-read
    // from the cp950-decoded golden main.h this pass (golden line numbers
    // cited per member).
    HANDLE hFileMapping;                          // [DATA] golden main.h:1525 `HANDLE hFileMapping;` -- the
                                    //   named-mapping handle CreateAndOpenMap's own else-branch creates and
                                    //   later closes; golden's own class member has no ctor initialiser
                                    //   either (VCL zero-inits it), so this stays whatever the OS/loader
                                    //   leaves it until CreateAndOpenMap runs -- same "no golden ctor
                                    //   assignment" posture as LastSet.cpp's `INFO *CmdData;`. Not a widget
                                    //   (plain Win32 HANDLE), so it does not fall under the widget-facade
                                    //   GATE rule.
    virtual bool   CreateAndOpenMap();                                    // golden main.h:1440 (body Command.cpp:8311-8358)
    virtual int    SetTrayBinByDLL(int iTrayNum, LPSTR asCategories, int iFail); // golden main.h:1442 (body Command.cpp:8360-8515)
    virtual int    GetTrayBinByDLL(int iTrayNum);                         // golden main.h:1443 (body Command.cpp:8517-8564)
    virtual int    SetSiteMapByDLL(LPSTR cSiteMap, int iNoOfSites);       // golden main.h:1444 (body Command.cpp:8566-8998)
    virtual int    GetSiteMappingByDLL(LPSTR cSiteMap);                   // golden main.h:1445 (body Command.cpp:9000-9213)
    virtual void   GetSiteMappingForSIGURD(LPSTR cSiteMap);               // golden main.h:1692 (body Command.cpp:9215-9236)
    virtual bool   GetSiteOnOffByChannel(int iCh, int iTolRow, int iTolCol); // golden main.h:1694 (body Command.cpp:9237-9257)
    virtual bool   SetSiteOnOffByChannel(int iCh, bool bSwitch);          // golden main.h:1695 (body Command.cpp:9258-9283)
    virtual void   SetSiteOnOff(AnsiString hexStr);                       // golden main.h:1535 (body Command.cpp:9284-9303)
    virtual bool   ParseHexToBoolArray(AnsiString hexStr, bool* bArr);    // golden main.h:1536 (body Command.cpp:9304-9326)
    virtual void   HexCharToBits(char hexChar, bool* bArr, int startIndex); // golden main.h:1537 (body Command.cpp:9327-9343)
    virtual int    SetTempByDLL(int iTempModeEPSON, double dTempVal);     // golden main.h:1446 (body Command.cpp:9344-9425)
    virtual int    GetTempSettingByDLL();                                 // golden main.h:1447 (body Command.cpp:9426-9456)
    virtual int    FTPDownloadByDLL(LPSTR cRecipeName);                   // golden main.h:1452 (body Command.cpp:9457-9494)
    virtual int    GetBinCountByDLL(int iCategNum);                       // golden main.h:1448 (body Command.cpp:9495-9525)
    virtual int    ClearBinCountByDLL();                                  // golden main.h:1449 (body Command.cpp:9526-9541)
    virtual int    GetSortCountByDLL(int nTrayNum);                       // golden main.h:1450 (body Command.cpp:9542-9566)
    virtual int    ClearSortCountByDLL();                                 // golden main.h:1451 (body Command.cpp:9567-9582)
    virtual int    GetHandlerStatusByDll();                               // golden main.h:1453 (body Command.cpp:9583-9636)
    virtual double GetAlarmStatusByDll();                                 // golden main.h:1454 (body Command.cpp:9637-9672)
    virtual int    GetBinCountPerSiteByDLL(int iCategNum, int iSiteNum);  // golden main.h:1456 (body Command.cpp:9718-9777)
    virtual int    GetTempActualByDLL();                                  // golden main.h:1457 (body Command.cpp:9778-9938)
    virtual bool   SettingsIsWindowOpened();                              // golden main.h:1458 (body Command.cpp:9939-9964)
    virtual void   WriteSiteOnOff();                                      // golden main.h:1527 (body Command.cpp:9965-9978)
    virtual void   AutoSiteOnOff(AnsiString buffer);                      // golden main.h:1528 (body Command.cpp:9979-9985)
    virtual void   WriteNumOfSites();                                     // golden main.h:1529 (body Command.cpp:9986-9994)
    // -- end FW3-WC ADD --------------------------------------------------------
    // -- FW3-WD ADD: Command.cpp wave-D declarations ---------------------------
    // AI(W906-FW3-WD) 20260818: FW-3 Wave D ADD -- 29 golden TfMain:: member
    // declarations (golden main.h :1196-1197 / :1530-1596 / :1633) whose bodies
    // are translated in Command.cpp's new "FW3-WD GROUP" section (golden
    // Command.cpp :9995-12061). Per contract rule 1 every declaration below is
    // `virtual`; per rule 4 each cites its golden home. Signatures re-read from
    // the cp950-decoded golden main.h this pass (golden line numbers cited per
    // member).
    virtual AnsiString GetSamSungTmp(bool bSend=true);                    // golden main.h:1530 (body Command.cpp:9995-10135)
    // GetSamSungMap / GetSamSungSoakTime: NOT re-declared here -- both are
    // ALREADY declared above (golden main.h:1531/1532, this file's own
    // W5-Automation INTEGRATE ADD block, lines 236-237) with matching
    // signatures. See this wave's own Command.cpp STUB COLLISIONS note: their
    // offline stub bodies (forms/fMain.cpp:282-283, `return ""`/`return "0"`)
    // are RETIRED by this wave's real bodies landing in Command.cpp, same
    // treatment FW3-WA already gave ArmStatusStrings/WritePERSITETemperature/
    // PERSITETemperatureStrings -- retire forms/fMain.cpp:282-283 in the NEXT
    // integration pass (this wave is barred from touching forms/fMain.cpp).
    virtual void   GetTTLState();                                         // golden main.h:1561 (body Command.cpp:10324-10500)
    virtual void   Send_Command_TTL(AnsiString asStr);                    // golden main.h:1562 (body Command.cpp:10502-10511)
    virtual void   WriteSetTempStatus_SIGURD();                           // golden main.h:1196 (body Command.cpp:10513-10594)
    virtual void   WriteSetSoakTimeStatus_SIGURD();                       // golden main.h:1197 (body Command.cpp:10596-10668)
    virtual void   SetSiteMapData_SIGURD();                               // golden main.h:1572 (body Command.cpp:10670-10728)
    virtual AnsiString GetTestIFSiteMap();                                // golden main.h:1573 (body Command.cpp:10730-10954)
    virtual void   ChkStatus();                                           // golden main.h:1574 (body Command.cpp:10956-11013)
    virtual void   GetBinCategory();                                      // golden main.h:1575 (body Command.cpp:11015-11024)
    virtual void   GetSetUpFileName();                                    // golden main.h:1576 (body Command.cpp:11026-11039)
    virtual void   GetHandlerID_Sigurd();                                 // golden main.h:1577 (body Command.cpp:11041-11051)
    virtual void   SetSetupFileName();                                    // golden main.h:1579 (body Command.cpp:11053-11102)
    virtual bool   ChangeSetupFileName(char *str);                        // golden main.h:1580 (body Command.cpp:11104-11135)
    virtual void   PPSELECTAskFile();                                     // golden main.h:1582 (body Command.cpp:11139-11144)
    // [PORT-ONLY SEAM] golden main.h:1633 `int iFileOkPPSELECT;` -- plain data
    // member PPSELECTLoadFile (below) writes; golden itself never initialises it
    // in the ctor (VCL zero-inits), so this stays 0 the same way, and gets no
    // dynamic initialiser (no SIOF risk -- it is a TfMain member, not a
    // namespace-scope static).
    int iFileOkPPSELECT;                                                  // [DATA] golden main.h:1633
    virtual void   PPSELECTLoadFile();                                    // golden main.h:1581 (body Command.cpp:11146-11191)
    virtual void   SetStartMode();                                        // golden main.h:1583 (body Command.cpp:11195-11271)
    virtual bool   ChangeHandlerStartMode(char *str);                     // golden main.h:1584 (body Command.cpp:11274-11332)
    virtual void   CheckList();                                           // golden main.h:1585 (body Command.cpp:11334-11357)
    virtual void   SetBinPosChange();                                     // golden main.h:1586 (body Command.cpp:11359-11398)
    virtual AnsiString BinPosChange(char *str);                           // golden main.h:1587 (body Command.cpp:11400-11548)
    virtual void   GetSGFTPSTATUS();                                      // golden main.h:1588 (body Command.cpp:11550-11558)
    virtual void   SetSGFTP();                                            // golden main.h:1589 (body Command.cpp:11560-11609)
    virtual void   SetNONDOUBLEBIN();                                     // golden main.h:1590 (body Command.cpp:11611-11696)
    virtual void   SetSBinData();                                         // golden main.h:1596 (body Command.cpp:11698-11724)
    virtual void   SetBINCOUNT();                                         // golden main.h:1591 (body Command.cpp:11726-11891)
    virtual void   SetSGOSBIN();                                          // golden main.h:1592 (body Command.cpp:11893-11947)
    virtual void   SetSGCONTFAIL();                                       // golden main.h:1593 (body Command.cpp:11949-12061)
    // -- end FW3-WD ADD --------------------------------------------------------
    // -- FW3-WE ADD: Command.cpp wave-E declarations ---------------------------
    // AI(W906-FW3-WE) 20260818: FW-3 Wave E ADD -- 35 golden TfMain:: member
    // declarations across two golden byte ranges (golden main.h :121-122 /
    // :499 / :1078-1089 / :1647 for region 1; :1183-1185 / :1459-1460 /
    // :1519-1522 / :1598-1601 / :1640-1643 / :1646 / :1649-1653 / :1655-1658 /
    // :1690 / :1704-1705 for region 2) whose bodies are translated in
    // Command.cpp's new "FW3-WE GROUP" section (golden Command.cpp
    // :12540-12761 / :14302-15273). Per contract rule 1 every declaration
    // below is `virtual`; per rule 4 each cites its golden home. Signatures
    // re-read from the cp950-decoded golden main.h this pass (golden line
    // numbers cited per member). golden's BCB6 `String sMessage` parameter
    // (HandlerTCPIPResultSendProcess/HandlerTeraTResultSendProcess) is kept
    // as `String` verbatim -- vcl_compat.h:340 `typedef vclcompat::AnsiString
    // String;` (re-exported globally, `VCLCOMPAT_NO_GLOBAL_USING` is never
    // defined anywhere in this tree, `grep -rn "#define VCLCOMPAT_NO_GLOBAL_
    // USING" .` -- 0 hits) makes it available unqualified without any new
    // #include -- forms/fMain.h's existing chain (FormWidgets.h -> vcl_compat.h)
    // already transitively pulls in vclcompat/ServerSocket.h (vcl_compat.h:240,
    // which itself includes ClientSocket.h), so `TObject`/`TCustomWinSocket`/
    // `TErrorEvent` below are likewise already unqualified-visible -- no new
    // #include was added to this file for this wave. `TMyKitSuck` (used only
    // as an incomplete pointer parameter type by Get2DID_OrderBySites below)
    // is forward-declared at FILE scope above the class (see that
    // declaration's own comment for why it must NOT sit here, inside the
    // class body).
    // AI(W906-FW3-WE-integrate) 20260818: `= false` NSDMI added -- golden has
    // no ctor init because VCL's TObject allocation zero-fills; this port's
    // plain `new TfMain()` does NOT, so an uninitialized bool here is an
    // indeterminate read (same NSDMI-everywhere rule every facade member of
    // this class already follows).
    bool bHandlerResultConnect = false;            // [DATA] golden main.h:1647
                                    //   initialiser (VCL zero-init -> false); flipped true/false by the
                                    //   TCPCommandServer*/TeraTCPResultServer* Connect/Disconnect/Error
                                    //   handlers below (region 1) -- a plain status bool, not a socket
                                    //   object member, so adding it does not touch this wave's "don't add
                                    //   socket members yourself" boundary.
    virtual void   TCPCommandServerClientConnect(TObject *Sender, TCustomWinSocket *Socket);       // golden main.h:1085 (body Command.cpp:12540-12548)
    virtual void   TCPCommandServerClientDisconnect(TObject *Sender, TCustomWinSocket *Socket);    // golden main.h:1087 (body Command.cpp:12550-12557)
    virtual void   TeraTCPResultServerClientConnect(TObject *Sender, TCustomWinSocket *Socket);    // golden main.h:1078 (body Command.cpp:12559-12567)
    virtual void   TeraTCPResultServerClientDisconnect(TObject *Sender, TCustomWinSocket *Socket); // golden main.h:1080 (body Command.cpp:12569-12577)
    virtual void   TeraTCPResultServerClientError(TObject *Sender, TCustomWinSocket *Socket, TErrorEvent ErrorEvent, int &ErrorCode); // golden main.h:1082 (body Command.cpp:12579-12608)
    virtual void   TCPIPCommunicationLog(AnsiString Str);                 // golden main.h:1522 (body Command.cpp:12610-12621)
    virtual void   HanderTcpIp();                                         // golden main.h:1519 (body Command.cpp:12623-12655)
    virtual void   HandlerTCPIPResultSendProcess(String sMessage);       // golden main.h:1520 (body Command.cpp:12657-12723)
    virtual void   HandlerTeraTResultSendProcess(String sMessage);       // golden main.h:1521 (body Command.cpp:12725-12760)
    virtual int        GetProdModeByDll();                                // golden main.h:1460 (body Command.cpp:14302-14317)
    virtual int        SetProdModeByDll(int iProdMode);                   // golden main.h:1459 (body Command.cpp:14319-14356)
    virtual void       WriteREADYNEXTSHOT();                              // golden main.h:1650 (body Command.cpp:14358-14361)
    virtual AnsiString GetREADYNEXTSHOT();                                // golden main.h:1651 (body Command.cpp:14363-14427)
    virtual void       WriteNEXT2DID();                                   // golden main.h:1652 (body Command.cpp:14429-14432)
    virtual AnsiString GetNEXT2DID();                                     // golden main.h:1653 (body Command.cpp:14434-14459)
    virtual void       Get2DID_OrderBySites(TMyKitSuck *kit, TStringList *sSourceList); // golden main.h:1649 (body Command.cpp:14461-14485)
    virtual void       SetAICCD();                                        // golden main.h:1640 (body Command.cpp:14487-14532)
    virtual void       GetAICCD();                                        // golden main.h:1641 (body Command.cpp:14534-14547)
    virtual bool       IsStackHasLess16Bin(int iStack);                   // golden main.h:1690 (body Command.cpp:14549-14565)
    virtual void       TransformTcGPIBData(AnsiString asStr);             // golden main.h:1646 (body Command.cpp:14567-14669)
    virtual void       SetOSBIN();                                        // golden main.h:1642 (body Command.cpp:14671-14683)
    virtual void       GetOSBIN();                                        // golden main.h:1643 (body Command.cpp:14685-14690)
    virtual void       GetDUTCHK();                                       // golden main.h:1598 (body Command.cpp:14692-14730)
    virtual void       GetFFC();                                          // golden main.h:1599 (body Command.cpp:14732-14740)
    virtual void       GetTJFunction();                                   // golden main.h:1600 (body Command.cpp:14742-14750)
    virtual void       GetPowerFollowing();                               // golden main.h:1601 (body Command.cpp:14752-14760)
    virtual void       WriteHeadContactCount();                           // golden main.h:1185 (body Command.cpp:14762-14976)
    virtual void       WriteSetSetupFile();                               // golden main.h:1183 (body Command.cpp:14979-15034)
    virtual void       WriteFTPDownSetupFile();                           // golden main.h:1184 (body Command.cpp:15036-15103)
    virtual void       GetSocketCounter();                                // golden main.h:1704 (body Command.cpp:15105-15150)
    virtual void       GetTIMCounter();                                   // golden main.h:1705 (body Command.cpp:15152-15212)
    virtual void       WriteMultiZoneTemp();                              // golden main.h:1655 (body Command.cpp:15214-15236)
    virtual void       WriteMultiZoneEnable();                            // golden main.h:1656 (body Command.cpp:15238-15249)
    virtual void       ReadWaterValve();                                  // golden main.h:1657 (body Command.cpp:15251-15261)
    virtual void       ReadDynamicPID();                                  // golden main.h:1658 (body Command.cpp:15263-15272)
    // -- end FW3-WE ADD --------------------------------------------------------
    // -- FW3-WF ADD: Command.cpp wave-F declarations ---------------------------
    // AI(W906-FW3-WF) 20260818: FW-3 Wave F ADD -- Command.cpp's collection-
    // closing wave. 3 golden TfMain:: member declarations (golden main.h
    // :1430-1433) whose bodies are translated in Command.cpp's new "FW3-WF
    // GROUP" section (golden Command.cpp :5301-5339 / :5341-5388 / :7304-7509).
    // Per contract rule 1 every declaration below is `virtual`; per rule 4 each
    // cites its golden home. No new #include needed -- every symbol the three
    // bodies touch (HHandler2Gpib/MOT[]/Sen[]/fNote/fProductionInfo/IniConfig/
    // TestIF_File/Prod/HGpib2Handler/etc.) is already unqualified-visible
    // through this file's existing include chain and this TU's own prior FW3
    // waves (several -- e.g. HGpib2Handler->cReturn, iBackupDutOnOff/
    // iBackupTestMode, MSG_CMD_SetSiteMapData -- are BYTE-FOR-BYTE reused
    // patterns already compiling in Wave D's SetSiteMapData_SIGURD, Command.cpp
    // :11400-11424).
    //
    // NOTE ON THE TASK BRIEF'S SetAlarmSetup LINE RANGE: the brief for this
    // wave cited SetAlarmSetup as golden :5341-:7303 ("~1,963-line branch-sea
    // single function"). That citation is WRONG. Re-derived directly from the
    // cp950-decoded golden this pass: SetAlarmSetup is golden :5341-5388 (only
    // 48 lines -- ends at the `}` on :5388, immediately followed by
    // `//---...` and the NEXT method, `void __fastcall TfMain::GetCZtesterBin()`,
    // at :5390). Golden :5389-7303 holds SEVEN other TfMain methods, every one
    // of which is ALREADY translated in this file from an EARLIER wave:
    // GetCZtesterBin (:5390-5417, port Command.cpp:2892), GetCZSoakTime
    // (:5419-5426, port :2922), GetCZDoubleContactCount (:5429-5437, port
    // :2933), GetCDHandlerID (:5440-5444, port :2945), GetCZJamCode
    // (:5447-5460, port :2953), GetCZSiteMap (:5463-5639, port :2970), and
    // GetCZAllMassTemp (:5640-7299, port Command.cpp:6542, Wave B). Verified
    // with `grep -n "TfMain::<name>" Command.cpp` for all seven -- exactly one
    // definition each, all pre-dating this wave -- confirming none of the
    // three MUST NOT be re-emitted here (would be an ODR/multiple-definition
    // link error). Golden :7300-7303 is a trio of `extern int` forward
    // declarations (iTestHeadMotorTask/iTestYTask/LoadTask) belonging to
    // MachineStatus's own TU, not code. This wave's ACTUAL new work is exactly
    // the three methods below: SetSiteMapData (39 golden lines) + SetAlarmSetup
    // (48 golden lines) + MachineStatus (206 golden lines) = 293 golden lines,
    // not ~2,200. (Aside, not touched by this wave: this file's own Wave-B-era
    // GetCZAllMassTemp comment at line 817 above cites its golden body as
    // "Command.cpp:6542-8201" -- also off-by-N, the real end is :7299; :7300-
    // 8201 is MachineStatus (:7304-7509) plus the never-wave ChangeToSiteMap/
    // ChangeToAlarmSetup/ChangeToAlarmSetup_SG trio (:7511-8310). Pre-existing,
    // out of this wave's append-only mandate -- flagged for whoever next
    // touches that line, not corrected here.)
    virtual void SetSiteMapData();   // golden main.h:1431 (body Command.cpp new FW3-WF GROUP, golden Command.cpp :5301-5339)
    virtual void SetAlarmSetup();    // golden main.h:1433 (body Command.cpp new FW3-WF GROUP, golden Command.cpp :5341-5388)
    virtual void MachineStatus();    // golden main.h:1430 (body Command.cpp new FW3-WF GROUP, golden Command.cpp :7304-7509)
    // -- end FW3-WF ADD --------------------------------------------------------
    // -- FW-CMD-C ADD: Command.cpp batch-1 closeout declarations ---------------
    // AI(W906-FW-CMD-C) 20260820: the 5 golden TfMain:: methods this file's own
    // FW3-WF banner (line ~1042-1045 above) flagged as "the never-wave
    // ChangeToSiteMap/ChangeToAlarmSetup/ChangeToAlarmSetup_SG trio" plus
    // RemoteControl (flagged EXCLUDED at line ~838-840 above) plus
    // TCPCommandServerClientRead (never previously declared anywhere). Bodies
    // in Command.cpp's new "FW-CMD-C" section (golden Command.cpp
    // :7511-7849/:7852-8046/:8048-8307/:9673-9716/:12762-14301). Per contract
    // rule 1 every declaration below is `virtual`; per rule 4 each cites its
    // golden home. `__fastcall` dropped from TCPCommandServerClientRead's
    // signature, matching this file's own established convention for every
    // other TCP event-handler declaration above (TCPCommandServerClientConnect/
    // Disconnect, HanderTcpIp, etc. -- `grep -c "__fastcall" Command.cpp` is 1
    // tree-wide, re-verified this pass).
    virtual bool ChangeToSiteMap(char *str);                                    // golden main.h:1432 (body Command.cpp :7511-7849)
    virtual bool ChangeToAlarmSetup(char *str);                                 // golden main.h:1434 (body Command.cpp :7852-8046)
    virtual bool ChangeToAlarmSetup_SG(char *str);                              // golden main.h:1435 (body Command.cpp :8048-8307)
    virtual int  RemoteControl(int iMode);                                      // golden main.h:1455 (body Command.cpp :9673-9716)
    virtual void TCPCommandServerClientRead(TObject *Sender, TCustomWinSocket *Socket); // golden main.h:1089 (body Command.cpp :12762-14301)
    // -- end FW-CMD-C ADD --------------------------------------------------------
    // -- FW-CMD-E ADD: MainTempMode.cpp's TfMain::ChangeTempMode + its own
    //    edATCAmbientTemper widget gap -------------------------------------------
    // AI(W906-FW-CMD-E) 20260821: the 2 golden TfMain:: facade gaps this task
    // closes -- both were blocking Command.cpp's WriteSetTempStatus_SIGURD /
    // WriteSetSoakTimeStatus_SIGURD GATE REGISTER items 4/5/6 (see that file's
    // FW3-WD banner + the FW-CMD-D re-confirmations, Command.cpp :10356-10360 /
    // :10458-10494): `edATCAmbientTemper` (golden main.h:737 `TEdit
    // *edATCAmbientTemper;`) and `ChangeTempMode` (golden main.h:1323, body
    // golden main.cpp:21749-21925, now translated FAITHFULLY in the new file
    // MainTempMode.cpp). Per this task's own scope boundary, closing these two
    // gaps does NOT un-gate Command.cpp's #if 0 sites this wave -- that is a
    // SEPARATE, later task; MainTempMode.cpp's own report lists the
    // un-gating pre-conditions for whoever does that pass.
    //
    // edATCAmbientTemper: golden is a bare TEdit* (main.h:737); re-grepped this
    // pass (`grep -n "edATCAmbientTemper" .` over the golden tree, 20260821) --
    // every one of its golden call sites touches only ->Text (read via
    // atof(...c_str()) or written as `=d`/`=IniConfig.dATCAmbientTemperature`/
    // `=fWorkTemp`), ->Enabled or ->Visible -- the SAME shape as
    // edWorkTemperBase/edSoakTime/edHPX/edHPY/edtAuto1..6 above, so this reuses
    // TfLotInfoEdit (the established cross-form ->Text-only alias), not a new
    // widget type (per contract rule 3). ChangeTempMode's OWN body below does
    // NOT touch it at all (re-verified: `grep -n "edATCAmbientTemper"` over
    // golden main.cpp:21749-21925 -- 0 hits) -- it is added here because it is
    // the OTHER half of this task's brief, not because ChangeTempMode needs it.
    //
    // NSDMI DEVIATION, NOTED: every sibling TfLotInfoEdit* member above
    // (edWorkTemperBase/edSoakTime/edHPX/edHPY/edtAuto1..6) is `new`'d in
    // forms/fMain.cpp's TfMain constructor body, NOT here in the header -- but
    // this task's file scope is exactly {MainTempMode.cpp (new),
    // forms/fMain.h}, and explicitly excludes forms/fMain.cpp. Per the
    // facade's own NSDMI-everywhere rule (an indeterminate pointer read is a
    // real bug the moment anything dereferences it -- see the
    // W906-FW3-WE-integrate `bHandlerResultConnect = false` precedent above),
    // the initializer is given HERE instead, as a header NSDMI. Functionally
    // identical construction (`new TfLotInfoEdit()`), just a different physical
    // location; purely cosmetic for whoever next touches forms/fMain.cpp to fold it into the ctor alongside its siblings.
    virtual void RunCheckStart();  // AI(W906-ST-W7-RCS) 20260917: golden main.h / main.cpp:32170-32209 -- SECS Run Check notify. Body + full banner at the TAIL of forms/fMain.cpp (appended, so no existing line in this header or that .cpp moved: 435 cited line numbers live below this point, 145 of them in hand-written sources).
    TfLotInfoEdit *edATCAmbientTemper = new TfLotInfoEdit();  // [DATA] golden main.h:737 (TEdit*) -- ->Text/->Enabled/->Visible only, same idiom as edWorkTemperBase/edSoakTime
    virtual int ChangeTempMode(int Mode, bool Msg, bool bRefresh=false, bool bGPIB=false, bool bSetTempByDLL=false); // golden main.h:1323 (body MainTempMode.cpp, golden main.cpp:21749-21925) -- FAITHFUL, 3 SAFETY GATEs (fSetup->ReadUseSuckModeFile / fOffSet->ReadFile / COM2->ATCInitialTask, all absent tree-wide), see MainTempMode.cpp's own banner
    // -- end FW-CMD-E ADD --------------------------------------------------------

    // -- AI(W906-ST-S3-B2b) 20260918: the widgets SOFT_SIMULTE needs -----------
    //
    //  WHY THESE APPEAR ONLY NOW, AFTER MONTHS OF TRANSLATION.
    //  This tree had an explicit, documented convention -- stated in
    //  BarCode/BarCode_Shuttle2_CCDScan.cpp:421-426 -- that every
    //  `#ifdef SOFT_SIMULTE` block was copied from golden VERBATIM and needed no
    //  gate, "inert here because SOFT_SIMULTE is never #defined in this tree (the
    //  preprocessor strips the block before the compiler would need the widget
    //  symbol to exist)". So those blocks were never type-checked, and the
    //  widgets they touch were never added.
    //
    //  The user turned SOFT_SIMULTE ON on 20260918 (his model: there are exactly
    //  two builds, SOFT_SIMULTE = simulation, otherwise it runs on a machine) and
    //  chose to push it through rather than work around it. This is the first
    //  instalment of that bill: a parallel -fsyntax-only sweep of the 132 .cpp
    //  with SOFT_SIMULTE conditionals found 38 files failing on 52 error lines --
    //  but only 13 DISTINCT symbols, and chkInPickLoadError alone was 34 of them.
    //
    //  ⚠ EXPECT MORE. The compiler stops at the first error inside each block, so
    //  clearing these will reveal whatever is behind them. Never-compiled code is
    //  never-type-checked code.
    //
    //  All are golden TCheckBox/TLabel on TfMain; every call site touches only
    //  ->Checked (or ->Caption / ->Visible), so the stock vclcompat types carry
    //  them exactly. Offline defaults are the types' own (Checked=false).
    TfMainCheckBox *chkInPickLoadError;   // [DATA] golden main.h (TCheckBox*) -- 34 of the 52 errors; "report an error when pick-from-loader fails"
    TfMainCheckBox *chkInToShtDrop;       // [DATA] golden main.h (TCheckBox*) -- simulate a drop on the way to the shuttle
    TfMainCheckBox *chkInFromLoadDrop;    // [DATA] golden main.h (TCheckBox*) -- simulate a drop on the way from the loader
    TfMainCheckBox *cb1;                  // [DATA] golden main.h (TCheckBox*) -- OCRInsp.cpp:1425 legacy SOFT_SIMULTE result switch
    TfMainCheckBox *CheckBox1;            // [DATA] golden main.h (TCheckBox*) -- asendic_Loader.cpp:3213; ->Checked and ->Visible
    TfMainLabel    *Label3;               // [DATA] golden main.h (TLabel*)    -- ainarm9045_1x4_4/1x4_8_Hot show InArmSuck.iWhichSht
    TfMainCheckBox *chkHeaterOk;          // [DATA] golden main.h (TCheckBox*) -- uHeaterThread.cpp:501 simulated "heater reached temperature"

    // AI(W906-SJSON-S11) 20260923: golden main.h 的 bHasCleanCount。
    //   Clarn_Data 的 Tag==8 / CC_KYEC_LEE 分支要它（golden main.cpp:15590-15594）。
    //   ⚠ cContactCT.cpp:1199-1206 的 GATE (C4) 說「fMain->bHasCleanCount …
    //     none exist in forms/fMain.h」—— 那句話從今天起只對其餘四個成員
    //     （cbUserSelect / stOperatorClick / btLogin / spbUserName）成立。
    //     **C4 沒有解閘**：它是整塊閘，另外四個成員仍然不存在。
    //   NSDMI（同 forms/fYieldMonitoring.h:241 的 bShowSiteYield）而不是進建構子：
    //   TfMain 的建構子很長，加在裡面容易被之後的合併沖掉，而預設值必須是 false
    //   —— true 會讓 KYEC 的清 Yield 流程跳過重新登入。
    bool bHasCleanCount = false;                  // [DATA] golden main.h

    // AI(W906-MSTATE-P2) 20260924: ShowRunLabel（golden ckernel.cpp:935-1726）解閘所需的地基。
    //   計畫書 docs/MACHINE_STATE_WIRING_PLAN.md §3 第 3 條、§4 第 2 步。
    //   * 方法本體都在 cMainStatus.cpp（ht9045_sm）；刻意非 virtual —— 理由同上面的 EnabledSetupFile。
    //   * 顏色參數用 int：golden 是 TColor，而本樹的 TColor 一律是 `typedef int TColor`（cmydef.h:16），
    //     這個 forms 標頭看不到它，不為一個參數型別在 forms 層多引一個 typedef。
    //   * TPanel／TLabel 在 vclcompat 沒有 Font；golden 的 `X->Font->...` 改寫成平行的 TfMainFont 指標，
    //     沿用本檔既有範式 pnlCleanCountFont（上方 :368），不動 vclcompat（計畫書 §5 已推翻「要改 TPanel」）。
    //   * 指標用 NSDMI new（同 bHasCleanCount 的理由：建構子很長，合併時容易被沖掉）；
    //     golden main.dfm 的初值（Caption／Visible／Font）在 forms/fMain.cpp 建構子最後一段設定，不是自己發明的預設值。
    void ShowNowStatus(int cFontColor, AnsiString Capstr);   // [METHOD] golden main.h (body main.cpp:22190-22317, 128 lines)
    bool CheckCanChangeRealDummy();                           // [METHOD] golden main.cpp:12374-12380
    TfMainFont  *palMainStatusFont = new TfMainFont();       // [DATA] golden main.dfm palMainStatus.Font（Size 48）—— ShowNowStatus 寫 ->Size／->Color
    TfMainLabel *labDelayStatus    = new TfMainLabel();      // [DATA] golden main.h (TLabel*) —— ShowRunLabel 寫 ->Caption(19)／->Visible(21)
    TfMainFont  *labDelayStatusFont = new TfMainFont();      // [DATA] golden labDelayStatus->Font —— ShowNowStatus 寫 ->Color
    TfMainLabel *ARTCombine        = new TfMainLabel();      // [DATA] golden main.h (TLabel*) —— ShowRunLabel 寫 ->Caption(4)／->Visible(2)
    TfMainPanel *labAutoClean      = new TfMainPanel();      // [DATA] golden main.h (TPanel*) —— ShowRunLabel 寫 ->Caption(2)／->Visible(3)
    TfMainLabel *labQAMode         = new TfMainLabel();      // [DATA] golden main.h (TLabel*) —— ShowRunLabel 寫 ->Caption(1)／->Visible(2)
    // AI(W906-MSTATE-P2b) 20260924: ShowRunLed（golden ckernel.cpp:704-932）解閘所需。它對這幾個只碰 ->Value（LED）與 ->Visible（pnlSafePLC）。
    //   golden 是 TALed*，本樹既有的對應是 TfLedValue（同 ALed1，上方 :274）。初值同樣在 W906_InitMainStatusDfm 設。
    TfLedValue  *ledRed     = new TfLedValue();          // [DATA] golden main.h (TALed*) —— 塔燈紅；main.dfm Value = True
    TfLedValue  *ledGreen   = new TfLedValue();          // [DATA] golden main.h (TALed*) —— 塔燈綠
    TfLedValue  *ledYellow  = new TfLedValue();          // [DATA] golden main.h (TALed*) —— 塔燈黃；main.dfm Value = True
    TfLedValue  *ledSafePLC = new TfLedValue();          // [DATA] golden main.h (TALed*)
    TfMainPanel *pnlSafePLC = new TfMainPanel();         // [DATA] golden main.h (TPanel*) —— ShowRunLed 只寫 ->Visible
    void W906_InitMainStatusDfm();                            // 填上面幾個 widget 的 golden main.dfm 初值（forms/fMain.cpp 檔尾）
    // AI(W906-SENSORSCAN) 20260924: ProcessSensorScan（cSensorScan.cpp，golden main.cpp:14263）讀 atoi(edLoadCnt->Text)。唯一缺的 TfMain widget（ALed1／CheckBox1／lblLoadTrayCnt 已在上方）。
    //   型別同上方 :649-654 的 edtAuto1..6（golden TEdit → TfLotInfoEdit）。main.dfm:12316-12324 的初值 Text = '10' 承重（ART 模擬臂拿它比 lblLoadTrayCnt），所以就地設，不另開建構子段。
    TfLotInfoEdit *edLoadCnt = [] { TfLotInfoEdit *p = new TfLotInfoEdit(); p->Text = "10"; return p; }();   // [DATA] golden main.h:372 (TEdit*)

    // AI(W906-STATEREC) 20260924: State Record（golden main.cpp:26109-26678）需要的成員。本體在 cStateRecord.cpp（ht9045_sm）。
    //   * 方法刻意非 virtual（理由同上面的 EnabledSetupFile）—— 只有 sbStateRecordClick 是 virtual，因為它的本體在
    //     forms/fMain.cpp 檔尾、只呼叫本來就是 virtual 的 DoStateRecord，不碰 sm。
    //   * DoStateRecord（:194）本身維持 virtual，經本檔尾端的 W906_StateRecordBody 指標接到 cStateRecord.cpp。
    //   * 資料成員用 NSDMI（同 bHasCleanCount 的理由）。沒有加任何 widget：golden 這幾支碰的 widget
    //     （ImageRecord／SaveDialog1／sgTaskList／StringGrid1／StringGrid4／StringGrid5／rgHotplateShowMessage／
    //     StatusBar1／sbStateRecord）都只餵抓圖或 SGDToXLS，而 SGDToXLS 在移植樹是 no-op（SgdToXLS.cpp:88-94）
    //     —— 那幾行在 cStateRecord.cpp 裡逐一閘住並寫明理由，加空殼 widget 只會產生「有值卻沒人維護」的成員。
    int        iSaveImgae         = -1;                       // [DATA] golden main.h:1152 —— -1 取自 golden FormShow main.cpp:10335（-1 = 抓圖狀態機閒置）
    int        iSaveImageCT       = 0;                        // [DATA] golden main.h:1153
    int        iSaveImageTask     = 0;                        // [DATA] golden main.h:1154
    AnsiString NewPath;                                       // [DATA] golden main.h:1155 —— 這一次錄製的資料夾
    AnsiString SDataPath;                                     // [DATA] golden main.h:1200（KenHsieh 20230105）
    bool       bManualStateRecord = false;                    // [DATA] golden main.h:1201（KenHsieh 20230105）
    void StateRecordImage();                                  // [METHOD] golden main.h:1162 (body main.cpp:26109-26208) —— cStateRecord.cpp
    void SaveTaskList(AnsiString NewPath);                    // [METHOD] golden main.h:1524 (body main.cpp:6402-6751) —— cStateRecord.cpp
    void SaveDecisionVariables(AnsiString NewPath);           // [METHOD] golden main.h:1717 (body main.cpp:6759-6960) —— cStateRecord.cpp
    void ProcessTimeUpdate(bool flag);                        // [METHOD] golden main.h:1228 (body main.cpp:7806-7818) —— cStateRecord.cpp
    void W906_DoStateRecordBody(int iShowAlarm, bool bManual);// golden DoStateRecord 的本體（main.cpp:26340-26678）—— cStateRecord.cpp
    virtual void sbStateRecordClick(void *Sender);            // [METHOD] golden main.h:957 (body main.cpp:26209-26212) —— forms/fMain.cpp 檔尾；act.main.stateRecord 走這裡
    void InitDIOStstus(bool bNeedOn);                         // [METHOD] golden main.h:1310 (body main.cpp:24357-24375) —— cDIOStatus.cpp（ht9045_sm）；AI(W906-DIO) 20260925: 非 virtual 同 golden（virtual 會讓 forms 的 vtable 引用 sm）；佔用原本的空行，不移動行號
    // AI(W906-R28TORQ) 20260925: Index Z 扭力路徑跨 TU 共用的狀態（使用者 20260925 裁決第 6 條「對齊原 BCB6 做法」，
    //   docs/RULINGS_20260925.md §6）。golden 把這幾個狀態放在主畫面 widget 上，而且**當成資料在用**：
    //     chkReadTorque1/2 —「現在讀哪一支 Z 的扭力」旗標：atester.cpp DoTestHeadMotor 12110 設、rs232.cpp ReadTorque_* 讀完清
    //     edTorue0/1       — 讀回的扭力值：rs232.cpp ReadTorque_* 寫、atester.cpp 12110／ShowMainScreenPresure 讀
    //     edtReadZ1/2      — 驅動器讀回的扭力上限設定值：rs232.cpp Comm1ReceiveData 寫、iWriteAndCheckMotorTorque 比對
    //     lbArm0/1Torque   — 主畫面扭力欄：atester.cpp 12110 拿它的 Caption 跟 "1:Reading" 比來決定要不要重寫
    //   值來自伺服器的 RS232 回覆，不是瀏覽器的 UI 狀態 ⇒ pt-wave-loop 陷阱 #6 的表「C++ 仍持有 → 補成員」。
    //   以前這幾個是 atester.cpp 的 TU 區域替身（W7T1_TfMainTorqueSeam），跨 TU 看不到彼此（rs232 寫的值 atester 讀不到），
    //   現在收成唯一一份。上面 F1fix2 那段說「edTorue0/1 刻意不加，等翻 AddSV 的那一波」—— 前提是「沒有翻譯碼會解參考它們」，
    //   rs232.cpp／ShowMainScreenPresure 解閘後這個前提不成立了；型別照它的要求走 widget 替身型別（不是裸指標）。
    //   型別照 golden main.h，初值照 golden main.dfm（chkReadTorque Checked 預設 false、edTorue0/1 沒有 Text＝空字串）。
    TfMainCheckBox *chkReadTorque1 = [] { TfMainCheckBox *p = new TfMainCheckBox(); p->Caption = "Z1"; return p; }();   // [DATA] golden main.h:464 (TCheckBox*)；main.dfm:15104 Caption='Z1'
    TfMainCheckBox *chkReadTorque2 = [] { TfMainCheckBox *p = new TfMainCheckBox(); p->Caption = "Z2"; return p; }();   // [DATA] golden main.h:465 (TCheckBox*)；main.dfm:15120 Caption='Z2'
    TfLotInfoEdit  *edTorue0       = new TfLotInfoEdit();                                                                // [DATA] golden main.h:466 (TEdit*)；main.dfm:15136 無 Text（空字串）
    TfLotInfoEdit  *edTorue1       = new TfLotInfoEdit();                                                                // [DATA] golden main.h:467 (TEdit*)；main.dfm:15152 無 Text（空字串）
    TfLotInfoEdit  *edtReadZ1      = [] { TfLotInfoEdit *p = new TfLotInfoEdit(); p->Text = "0"; return p; }();         // [DATA] golden main.h:468 (TEdit*)；main.dfm:15168 Text='0'
    TfLotInfoEdit  *edtReadZ2      = [] { TfLotInfoEdit *p = new TfLotInfoEdit(); p->Text = "0"; return p; }();         // [DATA] golden main.h:469 (TEdit*)；main.dfm:15185 Text='0'
    TfMainPanel    *lbArm1Torque   = [] { TfMainPanel *p = new TfMainPanel(); p->Caption = "---"; return p; }();        // [DATA] golden main.h:797 (TPanel*)；main.dfm:4160 Caption='---'
    TfMainPanel    *lbArm0Torque   = [] { TfMainPanel *p = new TfMainPanel(); p->Caption = "---"; return p; }();        // [DATA] golden main.h:798 (TPanel*)；main.dfm:4176 Caption='---'
    vclcompat::TListBox *ListBox14 = new vclcompat::TListBox();                                                         // [DATA] golden main.h:483 (TListBox*)；main.dfm:15353 —— AddTorqueLog 的扭力通訊紀錄（最多 300 行）
    void AddTorqueLog(AnsiString Msg);                        // [METHOD] golden main.h:1477 (body main.cpp:32586-32609) —— 本體在 rs232.cpp（ht9045_sm）；刻意非 virtual（理由同上面的 EnabledSetupFile）

    TfMain();
    virtual ~TfMain() {}
};
extern TfMain *fMain;  extern void (*W906_UpdateMainOperateModeHook)(TfMain*);  void W906_InstallUpdateMainOperateMode();   // AI(W906-OPMODE) 20260926: 虛擬的 UpdateMainOperateMode 在 forms/fMain.cpp:507（ht9045_forms）經這個 hook 呼叫 forms/fMain_OperateMode.cpp（ht9045_sm）的本體；wb_serve 開機安裝

// ---------------------------------------------------------------------------
//  AI(W906-SJSON-S11) 20260923: Clarn_Data 本體的安裝座。
//
//  ## 為什麼是函式指標而不是直接把本體寫在 TfMain::Clarn_Data 裡
//
//  本體要碰 fCounterClear->ClearCount（cCounterClear.cpp，**ht9045_sm**）、
//  MyDBIProcess（cMyDB.cpp，**ht9045_db**）、WriteLastDataFile（cprod.cpp，
//  ht9045_globals）、TMyStringList（ht9045_public）。
//  而 ht9045_forms **刻意只 link vclcompat + ht9045_globals**
//  （CMakeLists.txt:713-714／:741-744 的兩段註解都明講，並把
//  「forms->sm archive edge」列為要避免的東西）。
//  把本體寫進 forms/fMain.cpp 會讓 ht9045_forms 長出一條向上的 archive 邊，
//  而那條邊會把每一個「只 link forms 不 link sm」的測試目標一起弄壞。
//
//  這棵樹對同一個問題已經有既定作法（CMakeLists.txt:743-744 原話：
//  「Their sm-reaching methods live in root myQwertyKeyBoard.cpp/Password.cpp」）
//  ——「facade 在 forms/，會往上碰的方法本體放在別的 library」。
//  本檔用的是它的指標版本，差別只在**不需要把 fMain 換成子類別**。
//
//  ## ⚠ 為什麼要有一個明確的安裝呼叫，而不是自我登錄
//
//  靜態 archive 的成員只有在解析未定義符號時才會被抽出來（CLAUDE.md 的
//  陷阱 #2）。一個「在 static init 時自己登記」的 TU 放在 .a 裡**根本不會
//  被連進去**，而 build 仍然全綠 —— 正是那條陷阱描述的假象。
//  所以安裝是 wb_serve 開機時明確呼叫 InstallClarnDataBody() 的（tools/wb_serve.cpp），
//  有一個真的符號引用，連不進來就是 link error 而不是靜靜地沒接上。
//
//  ## ⚠⚠ 安裝之後的影響範圍，不只 act.main.clarnData
//
//  裝上去之後，移植樹既有的 **20 個 live 呼叫點**（全部 28 個，其中 8 個在
//  `#if 0` 內）全部從 no-op 變成真的會清計數並呼叫 WriteLastDataFile() —— 那會寫
//  `D:\HT9045\system\lastdata.dat`（cprod.cpp:2022，硬編路徑，--dry 蓋不到）。
//  這是忠於 golden 的結果，不是副作用；但它是**這一波最大的行為變更**，
//  跑 wb_serve 之前先備份 lastdata.dat（它不在版控裡，蓋掉沒有 git 可以救）。
//
//  ⚠ **更正（20260923，同日稍晚）**：本段第一版寫「22 個呼叫點」並列了
//    七個檔名。那個數字是**錯的，而且漏掉的正好是最要緊的那個檔**。
//    重量法（20260923，排除 build/ 與 tests/、剝行註解、扣掉本檔的定義與
//    fMain.h 的宣告）：
//      grep -rn "Clarn_Data *(" --include=*.cpp .  ->  28 個呼叫點
//      csystem.cpp 8 / Command.cpp 4 / SECSGEM/uHGemHT9045.cpp 4 /
//      Automation/uRENESAS_Server.cpp 3 / **WebStart.cpp 3** /
//      AutoRetest.cpp 2 / cCounterClear.cpp 2 / forms/fLotInfo.cpp 2
//    漏掉的是 **WebStart.cpp**，而它的三筆全在 START 路徑上：
//      WebStart.cpp:1352  Clarn_Data(11, "LowYield Start Clear")
//      WebStart.cpp:2355  Clarn_Data(1,  "TSMC_InitialCleanCount")
//      WebStart.cpp:2367  Clarn_Data(1,  "JCET_NeedClearSortCount")
//    ⇒ 按一次網頁 START 就可能寫 lastdata.dat。第一版的清單會讓人以為
//      只有 SECS／遠端指令那幾條冷路徑受影響。
//
//  ⚠ **第二次更正（20260923，審查員指出）：28 是總數，不是會動的數。**
//    28 個裡有 **8 個在 `#if 0` 內**，實際 live 的是 **20 個**：
//      GATED  csystem.cpp:3314/:3406      （#if 0 3233..4375）
//      GATED  csystem.cpp:10960           （#if 0 10087..11325）
//      GATED  csystem.cpp:11746/:11766    （#if 0 11388..12089）
//      GATED  Command.cpp:17559           （#if 0 17556..17616）
//      GATED  SECSGEM/uHGemHT9045.cpp:5303（#if 0 5288..5334）
//      GATED  forms/fLotInfo.cpp:5857     （#if 0 5831..5940）
//    ⚠⚠ **量這個不能用「往回找最近的 #if 0」那種寫法。** 我第一次寫的
//      掃描器沒有正確處理 `#else`（`#if 0 … #else … #endif` 裡 `#else`
//      之後的程式碼是**會編**的），因此把上面 csystem 的三筆誤判成 live。
//      正確作法是先把每個 `#if 0` 的「被丟掉區間」算出來（遇到同層的
//      `#else`／`#elif` 就結束區間），再判斷行號落不落在區間內。
//    ⇒ live 的 20 個裡，**運轉中會自動觸發**的是 WebStart.cpp 三筆（網頁
//      START）、csystem.cpp:12585/:13194/:13196（Initial／RT Initial Start）、
//      AutoRetest.cpp:707/:710、uRENESAS_Server 三筆、Command.cpp 三筆
//      （遠端指令）、uHGemHT9045.cpp:5139/:5284/:7172（SECS S2F42 等）。
//      人為觸發的只有 cCounterClear.cpp:449/:475（spbExeClick）與
//      forms/fLotInfo.cpp:5596（SetLotStart）。
//
//  ⓘ 另有 2 筆**仍然是 no-op**、不受本次影響：csystem.cpp:8266／:8351 走
//    `W7C2_FMAIN_CLARNDATA` 巨集（csystem.cpp:6585 定義成空操作，W7-C2 的閘）。
//    它們不在上面那 28 個裡面。
// ---------------------------------------------------------------------------
typedef void (*W906_ClarnDataBodyFn)(int Tag, AnsiString Msg);
extern W906_ClarnDataBodyFn W906_ClarnDataBody;   // 0 = 離線 no-op（預設）

// ---------------------------------------------------------------------------
//  AI(W906-STATEREC) 20260924: DoStateRecord 本體的安裝座（同上面 Clarn_Data 的作法與理由）。
//  本體在 cStateRecord.cpp（ht9045_sm）；wb_serve 開機時呼叫 W906_InstallStateRecordBody()。
//  沒安裝（測試執行檔）時 TfMain::DoStateRecord 維持原本的 no-op。
//  ⚠ 裝上之後，移植樹既有的 fMain->DoStateRecord 呼叫點全部變成真的錄製（SOFT_SIMULTE 組態下 golden
//    自己只做 RecordProcess／LogIndexMaxMinPos／DumpMainFormSnapshot，見 cStateRecord.cpp 檔頭）。
//  W906_StateRecordWorkerBusy：背景複製／7z 執行緒還在跑時為 true（cStateRecord.cpp 寫；act.main.stateRecord 讀）。
// ---------------------------------------------------------------------------
#include <atomic>
typedef void (*W906_StateRecordBodyFn)(TfMain *self, int iShowAlarm, bool bManual);
extern W906_StateRecordBodyFn W906_StateRecordBody;   // 0 = 離線 no-op（預設）
extern std::atomic<bool> W906_StateRecordWorkerBusy;

// ---------------------------------------------------------------------------
//  AI(W906-GB-P2b) 20260926: Tester 通訊的安裝座（同上面兩個的作法與理由）。
//  golden TfMain 的 SendMSG_CMD×2／RunTestProgram／CloseGpibProgram／SendMSG_TestMode／WakeupGPIB／
//  SendMSG_CMD_DeviceMapSRQ 本體翻在 TesterComm/Handler/（THandlerTesterSide，ht9045_testercomm_handler）。
//  ht9045_forms 不能直接連過去（很多 ctest 只連機台庫、不連 Tester 通訊），所以經這張表轉：
//  W906_TesterCommInit() 填、W906_TesterCommShutdown() 清。全 0 = 原本的離線 no-op（RunTestProgram 回 false）。
//  ⚠ 裝上之後，移植樹既有的 fMain->SendMSG_CMD 呼叫點（Command.cpp 的 Write*／Get* 回覆、aTester_Front／Rear、
//    St01 S86 的 OCR BarCode／Pin1 四個指令…）都會真的送到 bridge；bridge 沒找到（bFind==false）時照 golden 直接 return。
// ---------------------------------------------------------------------------
struct W906_TesterForwardTable
{
    void (*SendMSG_CMD)(int CMD);
    void (*SendMSG_CMD_Msg)(int CMD, AnsiString Message);
    bool (*RunTestProgram)(bool bNeedTest, bool *bSiteOnOff);
    void (*CloseGpibProgram)(AnsiString Src);
    void (*SendMSG_TestMode)();
    void (*WakeupGPIB)(AnsiString FuncName);
    void (*SendMSG_CMD_DeviceMapSRQ)(int iStatus);
    bool (*BridgeFound)();                    // golden fMain->bFind（THandlerTesterSide::bFind）  AI(W906-GB-P2b) 20260926
};
extern W906_TesterForwardTable W906_TesterForward;   // 全 0 = 離線 no-op（預設）
// golden 的 `fMain->bFind`（atester.cpp GetTesterResult 讀它）：沒裝 = false，跟 golden「bridge 沒找到」同一條路。
inline bool W906_TesterBridgeFound() { return W906_TesterForward.BridgeFound != 0 && W906_TesterForward.BridgeFound(); }

#endif // FORMS_FMAIN_H
