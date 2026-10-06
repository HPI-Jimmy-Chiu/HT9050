// =============================================================================
//  forms/fLotInfo.h  --  non-VCL stand-in for golden's fLotInfo form pointer
//
//  AI(W906-W7-F0) 20260728: split out of FormsFacade.h by the W7-F0 refactor
//  (docs/W7_UI_ARCHITECTURE_PLAN.md SS6-F0-d).  Content moved VERBATIM apart
//  from the F0-e virtualisation; the widget stand-ins (TfLotInfoRunMode /
//  TfLotInfoLabel / TfLotInfoEdit / TfLotInfoStatusLabel / TfLotInfoPanel /
//  TfMainMemo / TfLedValue) now come from forms/FormWidgets.h.
//  Facade-wide contract: see forms/fMain.h.
// =============================================================================
#ifndef FORMS_FLOTINFO_H
#define FORMS_FLOTINFO_H

#include "forms/FormWidgets.h"
#include "MachineType.h"   // AI(W906-FW3-LotInfo-WA) 20260819: ATC_HEAD_COUNT (32) sizes ATCChPal[]/ATCPtr[]/ATCReferPtr[] below
#include "vclcompat/FileListBox.h"   // AI(W906-FW3-LotInfo-WC) 20260819: TFileListBox (FileListBox1, FormShow S9 SPILFunction branch)
#include "vclcompat/ShiftState.h"   // AI(W906-FW-SIG-W15) 20260826

// =============================================================================
//  AI(W906-FW3-LotInfo-WA) 20260819: uLotInfo displayside Wave A.
//
//  Golden ref: HT9011UC_Code_V3.33.906.0_20260618/uLotInfo.cpp (16,613 lines,
//  cp950 -- decoded with `python3 -c "open(...,'rb').read().decode('cp950')"`,
//  0 U+FFFD, this wave) + uLotInfo.h (1,426 lines). Source recon:
//  docs/RECON_uLotInfo_displayside.md Tier 1 (39 methods, 1,199 golden lines,
//  section 5.1) -- every one of those 39 already had its full body read
//  line-by-line by that recon; this wave re-read each one again before
//  translating (per task brief step 4) and found 2 further RECON MISSES
//  documented in the GATE REGISTER below (WA-2, WA-6).
//
//  WAVE SCOPE -- the 39 Tier 1 methods (golden span, this file's status)
//  --------------------------------------------------------------------------
//    RefreshYieldMonitor_SIGURD      golden :13327-13545 (219L) -- REAL, 1 line gated (WA-1)
//    TransformTemperature_AirStream  golden :15080-15193 (114L) -- REAL, 1 block gated (WA-2)
//    AdjtsYieldMonitiorSize          golden :13636-13717 (82L)  -- REAL, 2 calls gated (WA-3)
//    SetSelectionVisible             golden :1262-1342   (81L)  -- REAL, 1 loop gated (WA-4)
//    ShowSocketID                    golden :11443-11516 (74L)  -- REAL, no gate
//    CheckAirMachineStatus           golden :14805-14868 (64L)  -- early-return only REAL, rest gated (WA-6)
//    InitialRefrigerantSystem        golden :14877-14939 (63L)  -- REAL, 1 call gated (WA-5)
//    SetATCFormVisible               golden :9938-9985   (48L)  -- REAL, no gate
//    CheckEventLogParameter          golden :10673-10720 (48L)  -- REAL, no gate
//    ShowATCTempPanel                golden :14561-14606 (46L)  -- REAL, no gate
//    JCETWhite2DIDShow               golden :15953-15992 (40L)  -- REAL, no gate
//    ShowInformation                 golden :12326-12362 (37L)  -- REAL, no gate
//    N23UseLotInfoFile               golden :7346-7380   (35L)  -- REAL, no gate
//    edTempKeyUp                     golden :4966-4999   (34L)  -- REAL, no gate (params dropped, unused in golden)
//    edtSysLotIDKeyPress             golden :11892-11917 (26L)  -- REAL, no gate (Sender/Shift dropped, unused)
//    SettsChipAdvVisible             golden :1240-1260   (21L)  -- REAL, no gate
//    bCheckOnlyOneFile               golden :14193-14213 (21L)  -- REAL, no gate
//    edPageKeyPress                  golden :11919-11933 (15L)  -- REAL, no gate (Sender/Shift dropped)
//    CheckActionFlag                 golden :16234-16246 (13L)  -- REAL, no gate
//    SetTesterStartTimeByB03         golden :14298-14309 (12L)  -- REAL, no gate
//    btnLoadFileClick                golden :14175-14184 (10L)  -- REAL, no gate (Sender dropped, unused)
//    cbRunModeKeyDown                golden :11990-11997 (8L)   -- body gated (WA-7)
//    cbRunModeKeyUp                  golden :11999-12006 (8L)   -- body gated (WA-7)
//    edDeviceNameKeyUp               golden :12044-12051 (8L)   -- REAL, no gate (params dropped, unused)
//    ReflashInfo                     golden :16479-16485 (7L)   -- REAL, no gate
//    labLotIDMouseDown               golden :8423-8428   (6L)   -- body gated (WA-8)
//    edtLotVerifyMouseDown           golden :14515-14520 (6L)   -- REAL, no gate (params dropped, unused)
//    btnCancelTestPauseClick         golden :12297-12301 (5L)   -- REAL, no gate (Sender dropped, unused)
//    edtLotVerifyKeyPress            golden :14522-14526 (5L)   -- REAL, no gate (Sender/Key dropped, unused)
//    BtnPauseMouseDown               golden :14533-14537 (5L)   -- REAL, no gate (params dropped, unused)
//    BtnPauseMouseUp                 golden :14539-14543 (5L)   -- REAL, no gate (params dropped, unused)
//    VisibleUploadBtnPAT             golden :15710-15714 (5L)   -- REAL, no gate
//    cbbDeviceNameChange             golden :7216-7219   (4L)   -- REAL, no gate (Sender dropped, unused)
//    btStartCountClick               golden :8418-8421   (4L)   -- REAL, no gate (Sender dropped, unused)
//    edtASECL_LotIDClick             golden :10491-10494 (4L)   -- body gated (WA-8)
//    btTesterTCPShowClick            golden :13841-13844 (4L)   -- body gated (WA-9)
//    InitialLoaderTask               golden :16259-16262 (4L)   -- REAL, no gate
//    InitialLDLevelTask              golden :16264-16267 (4L)   -- REAL, no gate
//  (InitialUnLoaderTask, golden :16250-16253, is the 39th Tier 1 method --
//  already translated by the AI(W906-AutoCleanFoundation) 20260721 wave, see
//  its own member/method comments below; not re-touched here.)
//
//  GATE REGISTER
//  --------------------------------------------------------------------------
//  (WA-1) RefreshYieldMonitor_SIGURD, golden :13346 --
//      `cbContactMode->ItemIndex=fContact->cbContactMode->ItemIndex;`.
//      RECON logged this dependency as "fContact->cbContactMode（fContactCT.h
//      已存在但成員需查）" -- CORRECTION found this wave: `fContactCT.h`
//      declares class `TfContactCT` / global `fContactCT`, NOT `TfContact` /
//      `fContact` -- two different golden classes with similar names. `fContact`
//      itself does not exist ANYWHERE in this tree (grepped `class TfContact\b`
//      and `\bfContact\b` tree-wide, 20260819 -- 0 hits outside this comment).
//      Gated at the single assignment; `cbContactMode` (fLotInfo's own combo)
//      is still declared and constructed below so future integration has
//      something to assign into.
//  (WA-2) TransformTemperature_AirStream, golden :15134-15169 -- the
//      `if(!tIni->ValueExists("Temp_Index","-60~-51")) { ...16x
//      tIni->WriteString(...)... }` lazy-init block. RECON's one-line summary
//      ("純查表計算回傳 double...無其他依賴") MISSED this: it is a real
//      config-file write to `D:\HT9045\config\AirStream.ini` on first run.
//      Per AGENTS.md / CLAUDE.md, `config\` is a production-machine shared
//      runtime-parameter directory, default read-only, changed only on
//      explicit user request -- so the write is gated rather than silently
//      ported. BEHAVIOUR DELTA while gated: `ReadString` below still runs
//      against whatever the ini already has (real read, unaffected); on a
//      brand-new `AirStream.ini` missing the "Temp_Index"/"Temp_Socket"
//      sections the lookup falls through to the `sLoadIni=""` default
//      (`atof("")==0`), instead of golden's built-in fallback table. Same
//      "gate made a real local go quiet" pattern is NOT needed here (no local
//      var loses its only use -- the surrounding classification/clamp logic
//      is unconditionally real).
//  (WA-3) *** RETIRED 20260826 by FW-LOTINFO-W30 -- BOTH CALL SITES OPENED. ***
//      Reason died when FW-LOTINFO-W27 landed RefreshYieldMonitor for real and
//      overturned its "(b) write path" classification; the real write is now
//      gated one frame deeper at WD-2.  See the W30 banner below.
//  (WA-3) AdjtsYieldMonitiorSize, golden :13671 and :13678 -- both
//      `RefreshYieldMonitor();` calls. `RefreshYieldMonitor` itself (golden
//      :13310-13325) is RECON item #115, classified (b) write-path: its
//      non-SIGURD (else) branch calls `RefreshYieldMonitor_TERAPOWER`, which
//      (RECON's own finding, section 4 item 3) calls
//      `fCleaning->ChangeACSmartInterval(2,...)` -- a real AutoClean-interval
//      config write, and moreover THE DEFAULT branch for most customer codes.
//      `RefreshYieldMonitor` is not one of this wave's 39 Tier-1 methods, so
//      per the task's dependency rule (missing sibling method -> gate, do not
//      invent a shim) both call sites are gated rather than the dispatcher
//      being half-translated. `RefreshAMR();` (golden :13683) is UNGATED: it
//      already exists as a real (no-op) translated method in this file from
//      an earlier wave (AI(W906-...) "W5-Automation ADD" below).
//  (WA-4) SetSelectionVisible, golden :13331-13336 (`for(int
//      i=0;i<groupbDownloadItem->ControlCount;i++) { if(groupbDownloadItem->
//      Controls[i]!=lblDownloadAccessWarning)
//      groupbDownloadItem->Controls[i]->Enabled=!bDenyByOP; }`). This walks
//      every DESIGN-TIME CHILD of a container widget -- `vclcompat::TGroupBox`
//      (vclcompat/Controls.h) has no `Controls[]`/`ControlCount` surface (it
//      only carries Caption/Visible/Enabled, per that header's own class-map
//      note), and adding one is a shared-header change outside this wave's
//      2-file write boundary (`forms/fLotInfo.h`/`.cpp` only). Gated; the
//      `bDenyByOP` calc and both `lblDownloadAccessWarning->Visible=...;`
//      writes (the only OTHER effect of this if/else) stay REAL.
//  (WA-5) InitialRefrigerantSystem, golden :14922 -- `ATC_OFFLINE_FormComInit();`.
//      That method is RECON Tier 2 (section 5.2: "grepped clean; NOT fully
//      body-read"), not one of this wave's 39 verified Tier-1 methods. Gated
//      rather than reaching into unverified territory to satisfy one call
//      site; `bInitFormcomponent=false;`/`OldRefrigerantCommand=false;`
//      (golden :14920-14921, the two lines immediately around it) stay REAL
//      since they are plain member writes with no dependency on it.
//  (WA-6) CheckAirMachineStatus, golden :14810-14868 -- effectively the WHOLE
//      body after the `AirStream_Select==0` early return. TWO separate,
//      independent reasons, both of which RECON's "(a) 顯示/唯讀 ... verified
//      fully" call (table row #156 / section 5.1 item 6) MISSED:
//        (a) every remaining line dereferences `ATC_InterfaceForm->
//            AirMachineInfo`/`.AirMachineInfo_Index`/`.IsConnect()`.
//            `ATC_InterfaceForm` is `TATC_InterfaceFormShim*`
//            (acarry_shims.h:109-115) and that shim carries EXACTLY ONE
//            member, `int iATC_MODE_TYPE` -- verified by reading the class
//            this wave (20260819). `AirMachineInfo`/`AirMachineInfo_Index`/
//            `IsConnect()` do not exist anywhere in this tree (grepped
//            tree-wide, 20260819 -- 0 hits outside golden/docs). Adding them
//            means editing `acarry_shims.h`, outside this wave's write
//            boundary.
//        (b) even where (a) did not apply, golden :14843-14858 is
//            `if(SystemStart){ if(Temperature.EnableAirMachineSocket){
//            ...ShowMyMessage(...)... } else if(...)
//            ShowErrorMessage("WAR1611", K_RETRY, MMSystem); }` -- a REAL
//            alarm-raising call (`ShowErrorMessage`, canary_support.h:66,
//            drives real Retry/Skip SM plumbing per its own header banner),
//            gated for the identical reason forms/fTemperFrom.h's GATE (T1)
//            gates its own `ShowErrorMessage("WAR15xx",...)` calls: raising a
//            real alarm channel is an operational behaviour change reserved
//            for explicit user sign-off, not a translation-fidelity question.
//      Net effect: only `if(AirStream_Select==0) return;` is real; nothing
//      else compiles into this function this wave, and consequently NONE of
//      the ~15 `palAirMachine*`/`pal_Air*`/`pnl_AirMachine*` Caption widgets
//      this function would otherwise need are declared below (adding them
//      would be unused surface -- same "don't add scope-creep surface" call
//      forms/fTemperFrom.h's own banner makes).
//  (WA-7) cbRunModeKeyDown / cbRunModeKeyUp, golden :11993 / :12002 --
//      `fBarCode->JCETUseMakeWhite2DIDList()`. Already an ESTABLISHED
//      tree-wide gate: Public/MyProductionRecord.cpp:1198 GATE G-1 gates the
//      identical golden call for the identical reason (`fBarCode`, i.e.
//      `TfBarCode*` per aHotPlateSubstrate.h:1018-1026, has no such
//      member -- verified again this wave). Both handlers reduce to a no-op
//      when gated, matching the G-1 precedent's own "ACTIVE arm: no-op" idiom.
//  (WA-8) labLotIDMouseDown (golden :8427) / edtASECL_LotIDClick (golden
//      :10493) -- `fQwertyKey->ShowQwertyKey(...)`. Already an ESTABLISHED
//      tree-wide gate (ATC/ATCInterface.cpp GATE (3)/(4), VacuumUnit.cpp GATE
//      (3), VacuumUnit/MyVacuumPanel.cpp GATE (3), EJ1N/MyOmronPanel.cpp GATE
//      (3), Public/HTEdit.cpp GATE (6) -- all cite "fQwertyKey has no port
//      anywhere in this tree"). Re-verified this wave (20260819): still 0
//      compiled (non-`#if 0`) definitions of `class TfQwertyKey` or a real
//      `fQwertyKey` global tree-wide. OPENED 20260824 (FW-QWKEY2):
//      fQwertyKey real since FW-QWKEY1 (fc08e09); both handlers' calls
//      are live (latent until HTEdit GATE (6) keyboard wiring).
//  (WA-9) btTesterTCPShowClick, golden :13843 -- `fTesterTCP->Show();`.
//      Interface/TesterTCP.h's own file banner (line 59) states plainly this
//      TU is "NOT a TfTesterTCP class or facade at all, only free functions".
//      `fTesterTCP` does not exist. Gated; reduces to a no-op.
//
//  DEVIATION -- event-handler signatures drop unused VCL glue parameters
//  --------------------------------------------------------------------------
//  Every golden `__fastcall` event handler in this batch (KeyUp/KeyPress/
//  KeyDown/MouseDown/MouseUp/Click) carries `TObject *Sender` plus, per event
//  kind, `WORD &Key`/`char &Key`/`TShiftState Shift`/mouse-button+coordinate
//  params. `__fastcall` itself is dropped tree-wide (vclcompat/vcl_compat.h's
//  own note: it is a compiler keyword this toolchain does not know, so a
//  neutralizing macro can never fire -- every other translated handler in
//  this tree already omits it). Of the params: NONE of `Sender`/`Shift`/mouse
//  X,Y/mouse Button are ever read by any of these 39 bodies (verified
//  per-function this wave); `Key` IS read (by value, never re-assigned) in
//  exactly 2 of them (`edtSysLotIDKeyPress`, `edPageKeyPress`) so those two
//  keep a plain `char Key` parameter. This is the same signature-simplification
//  idiom forms/fTemperFrom.h's own DEVIATION note documents for `FormClose`
//  dropping `(TObject*,TCloseAction&)` -- unread parameters, not a behaviour
//  change.
//
//  DEVIATION -- `fLotInfo->Height/Width/Top/Left` translated as bare
//  (implicit `this->`) member access
//  --------------------------------------------------------------------------
//  AdjtsYieldMonitiorSize/ShowInformation (both TfLotInfo methods) read golden
//  source that spells its OWN form's geometry through the global pointer,
//  e.g. `fLotInfo->Height=610;`, even from inside TfLotInfo's own methods --
//  ordinary Delphi/BCB6 style, not a cross-form reference. Translated as bare
//  `Height=610;` (i.e. `this->Height`). Under this tree's established
//  single-instance assumption (`this == fLotInfo` for every production call
//  path, same assumption forms/fTemperFrom.h's own banner names explicitly)
//  the two spellings are semantically identical; the bare form is additionally
//  the SAFER one for tests that construct their own local `TfLotInfo`
//  instance (`fLotInfo->Height` would silently write into the unrelated
//  global singleton instead of the instance under test, whereas bare
//  `Height` always writes into whichever object the method was actually
//  invoked on). `this->Top=205;`/`this->Left=340;` in golden ShowInformation
//  already used the bare/this-> spelling explicitly, translated verbatim.
// =============================================================================

// =============================================================================
//  AI(W906-FW3-LotInfo-WB) 20260819: uLotInfo displayside Wave B.
//
//  Golden ref: same file as Wave A above. Source recon: docs/
//  RECON_uLotInfo_displayside.md Tier 2 (39 methods, 993 golden lines, section
//  5.2) -- Tier 2 had ONLY been keyword-scanned by that recon, never read
//  line-by-line. Every one of the 39 was re-read in full this wave (python
//  cp950 decode of the same golden files) BEFORE translating, per the task
//  brief's mandatory step. That reread found several dependencies missing
//  tree-wide that recon's keyword scan could not have caught (a scan only
//  flags known bad-word patterns, not "this call target doesn't exist") --
//  see GATE REGISTER below, especially WB-2/4/5/7/9/10 (InputBarcodeNumber has
//  ZERO port anywhere in this tree -- `grep -rn InputBarcodeNumber` outside
//  golden/docs, 20260819, 0 hits -- despite being the single most common
//  dependency across this Tier's KeyUp/MouseUp customer-code branches) and
//  WB-1 (TImage/TCanvas have zero port, matching an already-established
//  precedent in EJ1N/MyOmronPanel.h:93 and VacuumUnit/MyVacuumPanel.h:58).
//
//  WAVE SCOPE -- the 39 Tier 2 methods (golden span, this file's status)
//  --------------------------------------------------------------------------
//    TfLotInfo (ctor)               golden :157-299  (143L) -- REAL scalars/
//                                    Timer/TabSheet/StringList wiring, WB-19
//                                    gates the named-widget-array wiring block
//    FormDestroy                    golden :301-314  (14L)  -- REAL (RECON
//                                    MISS: recon's row #2 one-line summary
//                                    "sets bShow=false" is FormClose's body
//                                    copy-pasted onto FormDestroy by mistake;
//                                    the real body is TimerERMS/StringList
//                                    cleanup + exception log + off-time log)
//    FormClose                      golden :1235-1238 (4L)   -- REAL
//    pgLotinfoChange                golden :7221-7266 (46L)  -- REAL
//    LoadRTCFullViewImg             golden :5366-5448 (83L)  -- WB-1, full gate
//    edDeviceNameMouseDown          golden :7193-7214 (22L)  -- WB-2, full gate
//    CutTempToEdit                  golden :5225-5244 (20L)  -- REAL, no gate
//    btChangeFileClick              golden :10213-10237(25L) -- WB-3, full gate
//    edtSysOperatorIDKeyUp          golden :10287-10312(26L) -- REAL, WB-4 gates 1 branch
//    edtSysLotIDKeyUp               golden :10314-10344(31L) -- REAL, WB-5 gates 1 branch
//    spOCRChangeFileClick           golden :8458-8474 (17L)  -- REAL, no gate
//    Timer3Timer                    golden :8476-8485 (10L)  -- REAL early-return, WB-6 gates rest
//    edDeviceNameKeyDown            golden :10457-10473(17L) -- REAL 2nd branch, WB-7 gates 1st
//    SetCheckCodeByLot              golden :12311-12324(14L) -- REAL, no gate
//    edTempKeyPress                 golden :12032-12042(11L) -- REAL, no gate
//    edTempKeyDown                  golden :12053-12063(11L) -- REAL, no gate
//    btnSaveDataClick               golden :12285-12295(11L) -- REAL, no gate
//    edPageMouseDown                golden :11534-11541 (8L) -- REAL early-return, WB-8 gates rest
//    edtSysLotIDKeyDown             golden :12065-12080(16L) -- REAL, no gate
//    edtSysOperatorIDKeyDown        golden :12082-12097(16L) -- REAL, no gate
//    edtSysOperatorIDMouseUp        golden :11755-11794(40L) -- REAL CC_Murata branch only, WB-9 gates rest
//    edtSysLotIDMouseUp             golden :11796-11831(36L) -- REAL CC_Murata branch only, WB-10 gates rest
//    edQAModeMouseDown              golden :11528-11532 (5L) -- WB-11, full gate
//    edtSysOperatorIDKeyPress       golden :11935-11963(29L) -- REAL (SetFocus() dropped, DEVIATION)
//    cbRunModeKeyPress              golden :11965-11988(24L) -- REAL CC_Murata branch, WB-12 gates JCET check
//    cbRunModeDropDown              golden :15928-15951(24L) -- WB-13, full gate
//    GetFTP_SettingN06              golden :15449-15460(12L) -- REAL, no gate
//    CheckNoRetestBinFlag           golden :14608-14630(23L) -- WB-14, full gate (returns true)
//    edStationNumMouseDown          golden :14060-14064 (5L) -- WB-15, full gate
//    ATC_OFFLINE_FormComInit        golden :14969-14992(24L) -- REAL, no gate
//    ScanRefrigerantSystem          golden :14941-14967(27L) -- REAL early-return, WB-16 gates rest
//    RefreshRefrigerantAllStatus    golden :14994-15078(85L) -- REAL early-return, WB-17 gates rest
//    leRunCardNumberMouseDown       golden :15780-15784 (5L) -- REAL, no gate
//    RefreshAMR                     golden :15816-15844(29L) -- REAL, no gate (was a no-op stub, now real)
//    ShowAMRCategoryBin             golden :15846-15881(36L) -- REAL, no gate
//    cbFirstTrayCheckOnUnloaderMouseDown golden :16029-16034(6L) -- REAL, no gate
//    RefreshOtherTool               golden :16047-16072(26L) -- REAL, no gate
//    labNowTrayCountClick           golden :16186-16190 (5L) -- REAL, no gate
//    Timer4Timer                    golden :16487-16493 (7L) -- REAL x2 calls, WB-18 gates 1 call
//
//  GATE REGISTER
//  --------------------------------------------------------------------------
//  (WB-1) LoadRTCFullViewImg, golden :5368-5447 -- every line dereferences
//      imgRTCFullView1/2/3/4 (golden TImage*) via ->Visible or
//      ->Picture->LoadFromFile(...). `class TImage`/`class TCanvas` have ZERO
//      port anywhere in this tree -- an ALREADY-ESTABLISHED finding cited
//      verbatim by two sibling forms (EJ1N/MyOmronPanel.h:93,
//      VacuumUnit/MyVacuumPanel.h:58), re-confirmed this wave
//      (`grep -rn "class TImage\b"` tree-wide, 20260819 -- 0 hits outside
//      those two comments). The whole function reduces to a no-op.
//  (WB-2) edDeviceNameMouseDown, golden :7196-7213 -- both branches' only
//      payload is `Clipboard()->Clear()` and/or
//      `InputBarcodeNumber("Input Device Name:")`. PARTIALLY OPENED 20260825
//      (FW-BARCODE4): the SCC branch is live (InputBarcodeNumber real since
//      FW-BARCODE1; its Clipboard line narrowed to GATE (CLIP)); the AMKOR/
//      QUALCOMM branch stays gated as GATE (WB-2-BTN) -- it reads the
//      dropped TMouseButton `Button` param (mbLeft/mbRight portless,
//      fContactCT.h:78-87), which nothing translated can spell.
//  (WB-3) btChangeFileClick, golden :10215-10236 -- every branch calls one of
//      `fBarCode->btBarcodeChangeFileDisConnect/btBarcodeChangeFileConnect
//      ->Click()`, `->InitialBarcodeScanChangeFile()`,
//      `->TimerBarcodeChangeFile->Enabled=true`. TfBarCode
//      (aHotPlateSubstrate.h:918-1025, re-read in full this wave) carries
//      none of these four members (it does carry some others, e.g. the
//      already-gated JCETUseMakeWhite2DIDList() target from WA-7 -- fBarCode
//      is a PARTIAL shim, not absent, but none of THIS function's targets are
//      among the members it has). Whole body gated.
//  (WB-4) edtSysOperatorIDKeyUp, golden :10300-10303 -- the
//      `IniConfig.bO23_InputLotIDByBarcode` branch's only statement is
//      `edtSysOperatorID->Text=InputBarcodeNumber("Input OP ID:","UserName");`.
//      InputBarcodeNumber has no port (banner top). The CC_Murata early
//      return, the `bLotFirstKeyIn` flag set, and the final else-if's
//      `edtSysOperatorID->Text="";` are all untouched by this and stay REAL.
//  (WB-5) edtSysLotIDKeyUp, golden :10322-10328 -- the
//      `CUSTOMER_CODE==CC_PTI || bO23_InputLotIDByBarcode` branch calls
//      `SetLotID("")` (real no-op stub, fine) then
//      `InputBarcodeNumber("Input Lot ID:","LotID")` (no port) then
//      `SetLotID(sBarcodeID)`. Gated as one block since the middle call is
//      unavailable; the CC_Murata return, the TFME/VTEST/SPIL branch's plain
//      `edtSysLotID->Text="";`, and the trailing `bLotFirstKeyIn` set stay REAL.
//  (WB-6) Timer3Timer, golden :8482-8484 -- `fOCR->OCRChangeFile()`. TfOCR
//      (forms/fOCR.h:30-35, added by an earlier wave for
//      `ChangeLightValue(int,int)` only) does not declare `OCRChangeFile()`.
//      The `InitialOK==false` early return stays REAL; `ret`/the
//      `Timer3->Enabled=false` write both live inside the gate since `ret`'s
//      only origin is the gated call.
//  (WB-7) edDeviceNameKeyDown, golden :10461-10465 -- the first branch's
//      payload is `Clipboard()->Clear()` + `InputBarcodeNumber(...)`, same
//      portless pair as WB-2. The second branch (`CC_AMD_M && bDeviceName_OK`)
//      touches only local bools/Text="" and stays REAL.
//  (WB-8) edPageMouseDown, golden :11540 -- the non-Murata branch's entire
//      payload is `fQwertyKey->ShowQwertyKey(...)`. OPENED 20260824
//      (FW-QWKEY2): fQwertyKey real since FW-QWKEY1 -- same opening as
//      WA-8 cites (labLotIDMouseDown/edtASECL_LotIDClick), re-verified again
//      this wave. The `CUSTOMER_CODE==CC_Murata return;` guard stays REAL.
//  (WB-9) edtSysOperatorIDMouseUp, golden :11762-11793 -- EVERY branch after
//      the CC_Murata check ultimately calls either InputBarcodeNumber (2
//      branches; still portless) or fQwertyKey->ShowQwertyKey (1 branch;
//      real since 20260824, but the block STAYS gated on the
//      InputBarcodeNumber co-blocker). The
//      OPENED 20260825 (FW-BARCODE4): InputBarcodeNumber real since
//      FW-BARCODE1 (e7b4bf8), fQwertyKey since FW-QWKEY1 -- the whole
//      else-if chain is live and `Sender` is restored to the signature
//      (the TempEdit==edtCusLotID dispatch reads it).
//  (WB-10) edtSysLotIDMouseUp, golden :11799-11829 -- same shape as WB-9: the
//      CC_Murata branch is golden's own empty `{}` (translated as a real
//      no-op), every other branch (CC_PTI/bO23, CC_TFME_CHINA, bSPILFunction,
//      CC_SCC, bVTESTFunction) calls InputBarcodeNumber and/or
//      fQwertyKey->ShowQwertyKey. OPENED 20260825 (FW-BARCODE4): both
//      entities real; chain live, Sender restored (the CC_SCC branch's
//      ShowQwertyKey reads it).
//  (WB-11) edQAModeMouseDown, golden :11531 -- the entire one-line body is
//      `fQwertyKey->ShowQwertyKey(...)`. OPENED 20260824 with WB-8/WA-8
//      (FW-QWKEY2): call live, latent until keyboard wiring.
//  (WB-12) cbRunModeKeyPress, golden :11984-11987 -- the trailing
//      `if(fBarCode->JCETUseMakeWhite2DIDList()==true) cbRunMode->Text="";`
//      is the IDENTICAL golden call Wave A's WA-7 already gates (reused
//      verbatim, re-verified 20260819: still 0 such member on TfBarCode).
//      The preceding CC_Murata branch (sbSECSLotStart/End Click()/Down and a
//      SubString trim) has no such dependency and stays REAL --
//      `TSpeedButton::Click()` is an accepted real offline no-op per the
//      established precedent this file's own btClearBarcodeList member comment
//      documents (PT-W3-integrate ADD, above).
//  (WB-13) cbRunModeDropDown, golden :15930-15950 -- the ENTIRE body is inside
//      `if(CUSTOMER_CODE==CC_JCET && fBarCode->JCETUseMakeWhite2DIDList()==true)`
//      -- the guard itself cannot be evaluated without the WA-7-gated member,
//      so unlike cbRunModeKeyPress there is no independent real branch to keep.
//  (WB-14) CheckNoRetestBinFlag, golden :14613-14627 -- calls
//      `fMesSystem->NeedNoRTBinID(i)` 2x. `fMesSystem` has NO PORT AT ALL in
//      this tree -- csystem.cpp:9857-9861's own banner names it explicitly
//      among "~40 objects that have no port at all" (re-verified 20260819:
//      `grep -rn "class TfMesSystem\|TfMesSystem *fMesSystem"`, 0 hits). It
//      also calls `ShowErrorMessage(sMES1713[i], K_RETRY|K_SKIP, ...)` -- a
//      real alarm-raise, same SAFETY-classified category as Wave A's WA-6 /
//      forms/fTemperFrom.h GATE (T1). Gated body returns `true` (the
//      conservative "no violation detected" default, matching golden's own
//      initial `flag=true`).
//  (WB-15) edStationNumMouseDown, golden :14063 -- one-line body, same
//      established fQwertyKey gate as WB-8/WB-11/WA-8. OPENED 20260824
//      (FW-QWKEY2): call live, latent until keyboard wiring.
//  (WB-16) ScanRefrigerantSystem, golden :14946-14965 -- every remaining line
//      after the early return touches `ATC_InterfaceForm->IsConnect()` /
//      `->bReadRefrigerantMode_Send` / `->bReadRefrigerantMode_Recv` /
//      `->Get_ATCRefrigeratorAllStatus(...)`, plus a bare `iATCOnLine` global
//      not yet ported. `ATC_InterfaceForm` is `TATC_InterfaceFormShim*`
//      (acarry_shims.h:109-115) which carries EXACTLY ONE member
//      (`iATC_MODE_TYPE`) -- the SAME finding Wave A's WA-6 made for
//      CheckAirMachineStatus, re-confirmed this wave. `if(AirStream_Select==0)
//      return;` stays REAL.
//  (WB-17) RefreshRefrigerantAllStatus, golden :15000-15077 -- both the `if`
//      and `else` arms reference `ATC_InterfaceForm->
//      bReadRefrigerantMode_AllStatus` / `->dATC_RefrigeratorAllStatus[][]` /
//      `->iATC_RefrigeratorUserMode[][]` / `->dATC_RefrigerantMachineHighLimit`
//      / `->dATC_RefrigerantMachineLowLimit` -- none of which exist on the
//      1-member shim (same WB-16/WA-6 finding). `if(AirStream_Select==0)
//      return;` stays REAL; nothing else in this 85-line function survives.
//  (WB-18) Timer4Timer, golden :16490 -- `CheckAMRAction();`. That method is
//      RECON item #206, classified (b) write-path, not one of this wave's 39
//      Tier-2 methods -- per the task's dependency rule (missing sibling
//      method -> gate, do not invent a shim) this one call is gated.
//      `CheckActionFlag();` and `ReflashInfo();` (both real Tier-1 methods
//      from Wave A) stay REAL and unconditional, matching golden's ordering.
//  (WB-19) TfLotInfo ctor, golden :172-269 -- the block that wires
//      ESD_DataPtr[18]/ESD_DECAY_DATA_Ptr[36] to 18+36 individually
//      `__published` TPanel members (pl_ESDProx*/pl_PositiveDecay*/
//      pl_NegativeDecay*), re-wires ATCChPal[32]/ATCReferPtr[32]/ATCPtr[32]
//      (already populated by Wave A's ShowATCTempPanel with generic
//      `new TPanel()` per slot) to 96 individually-named
//      pan_ATCTempHeadNN/pl_ATCRefHeadNN/pl_ATCTempHeadNN widgets, re-wires
//      edSocket[4][8] (already populated by Wave A's ShowSocketID) to 32
//      individually-named edtSocketAa..edtSocketDh, and populates the
//      file-scope SocketSiteCH_Display[][]/SocketLabCol_Display[]/
//      SocketLabRow_Display[] statics (also already populated by Wave A) from
//      32+8+4 more individually-named palAa..palDh/lbSocketIDColA-H/
//      lbSocketIDRowA-D widgets. None of these ~190 individually-named
//      widgets is EVER read by name anywhere in this wave's or Wave A's
//      scope -- only the array slot is read (verified: every consumer in
//      both waves indexes ATCChPal[i]/ATCPtr[i]/ATCReferPtr[i]/edSocket[r][c]/
//      SocketSiteCH_Display[r][c], never a bare widget name), and
//      TPanel/TEdit/TLabel here expose no per-instance identity beyond
//      Caption/Visible/Enabled/Color, none of which golden's ctor sets
//      differently per named widget in this block. Re-declaring ~190
//      passthrough pointers would reproduce EXACTLY the same array contents
//      (one distinct default-constructed widget per slot) Wave A's generic
//      `new TPanel()` loops already produce, at the cost of ~190 lines of
//      pure unused-identity boilerplate -- the same "don't add unused
//      surface" call this file's own WA-4/WA-5/WA-6 gates and
//      forms/fTemperFrom.h's banner already make. Gated as one block; every
//      OTHER line in the ctor (scalar/flag inits, TStringList/Timer/TabSheet
//      wiring, the 5 ZeroMemory'd arrays, the small ATC_WinWay/
//      ATCPtrWinWay/pnlWinwayPVCH1-4 wiring) is REAL.
// =============================================================================

// =============================================================================
//  AI(W906-FW3-LotInfo-WC) 20260819: uLotInfo MIXED-split Wave C.
//
//  Golden ref: same file as Wave A/B above. Source recon:
//  docs/RECON_uLotInfo_mixed_split.md -- the two MIXED giants that recon
//  explicitly did NOT translate (FormShow, golden :316-1233, 918L, 35
//  segments; Timer2Timer, golden :6934-7191, 258L, 18 segments), left as a
//  segment-table blueprint for a follow-on wave to assemble. That recon's own
//  S3 note (section 4, point 3) listed 9 facade dependencies as
//  "existence unverified" -- this wave's step 0 checked all 9 (plus several
//  more found only by re-reading every line, same "recon's keyword/visual
//  pass cannot see this" pattern Wave A's WA-2/WA-6 and Wave B's own banner
//  both already document). Every body below was re-read in full from the
//  golden cp950 decode before translating.
//
//  STEP 0 -- THE 9 RECON-FLAGGED DEPENDENCIES, VERIFIED 20260819
//  --------------------------------------------------------------------------
//    FormBarcodeReader                 ABSENT. `grep -rn FormBarcodeReader`
//                                       tree-wide (excluding docs/ and
//                                       tools/dfm2rc/ generated metadata): 1
//                                       hit, SECSGEM/uHGemHT9045.cpp, itself a
//                                       comment. No class/global anywhere. ->
//                                       GATE (WC-4).
//    fBarCode->mtBarcodeSetDefaultView  ABSENT. `grep -rn mtBarcodeSetDefaultView`
//                                       tree-wide: 0 hits outside this recon
//                                       doc. -> GATE (WC-9).
//    DewPoint_Hardware_Install          PRESENT. `extern int
//                                       DewPoint_Hardware_Install;`
//                                       (cmydef.h:3725), already reachable via
//                                       cmydef.h (Wave A include). -> REAL,
//                                       S27 needs no gate (recon's "顯示*" is
//                                       resolved to plain 顯示).
//    myInShuttleLotInfo/mtBarcodeInShLotInfo  ABSENT as TfLotInfo members.
//                                       Every one of the 15 tree-wide hits
//                                       (BarCode/BarCode_Bottom2DID.cpp,
//                                       BarCode_Shuttle{1,2}_*.cpp/.h) is
//                                       ITSELF inside a `#if 0` block or a
//                                       `//` comment referencing this exact
//                                       "separate not-yet-translated TfLotInfo
//                                       grid" gap. -> GATE (WC-18).
//    FrmAOI                            PRESENT as a global (forms/fAOI.h,
//                                       `extern TFrmAOI *FrmAOI;`) but the
//                                       class carries EXACTLY ONE member
//                                       (`bool bSimulateTopBtm`) -- no
//                                       `AOIFailCountRefresh()`. -> the global
//                                       exists, the METHOD does not -> GATE
//                                       (WC-17), only that one call.
//    fAGV->IsSPIL_AMR()                PRESENT and REAL: `virtual bool
//                                       IsSPIL_AMR();` (forms/fAGV.h:32),
//                                       forwarding to the real
//                                       Automation/AGV_predicates.cpp
//                                       predicate (W5-Automation INTEGRATE,
//                                       already used 18+ call sites
//                                       tree-wide, e.g. csystem.cpp:2978).
//                                       -> REAL, T4 needs no gate.
//    fMain->cbSetupFileName            PRESENT: fMain.h declares it as a
//                                       TComboBox-family member (already used
//                                       tree-wide, e.g. csystem.cpp,
//                                       Command.cpp); ->Text is a stock
//                                       TComboBox field (Controls.h). -> REAL,
//                                       T6 needs no gate.
//    fSetup->edOcrText                 [WC-27 RETIRED 20260820] Was ABSENT on
//                                       the real facade when Wave C landed
//                                       (TfSetup carried only `bool fShow`;
//                                       edOcrText lived only on OCRInsp.cpp's
//                                       TU-local W906OCR_TfSetupSeam). The
//                                       cSetUp display wave (W906-FW3-Setup-WA
//                                       20260820) added the real `edOcrText`
//                                       member to forms/fSetup.h (:271), so
//                                       the gate's cause is gone -- the line
//                                       is live again in the .cpp (integrator
//                                       flip, same day). OCRInsp.cpp is
//                                       unaffected: it never includes
//                                       forms/fSetup.h and keeps its own seam.
//    LastSet.strSocketID                PRESENT: `LastSet.h` declares it
//                                       (already used, S23's 4x8 loop bounds
//                                       match `edSocket[MAX_SOCKET_ROW]
//                                       [MAX_SOCKET_COL]`, already a real
//                                       TfLotInfo member). -> REAL, S23 needs
//                                       no gate.
//
//  ADDITIONAL DEPENDENCY GAPS FOUND BY THE MANDATORY LINE-BY-LINE RE-READ
//  (none of these 6 were on recon's 9-item list -- same "a scan only flags
//  known bad-word patterns" gap Wave B's own banner names)
//  --------------------------------------------------------------------------
//    fOCR->sTesterLotId       ABSENT on the real global. Same shape as
//                             fSetup->edOcrText above: `sTesterLotId` lives
//                             only on OCRInsp.cpp's TU-local
//                             `W906OCR_TfOCRSeam` shim; the real `TfOCR`
//                             (forms/fOCR.h, landed for an earlier wave)
//                             carries only `ChangeLightValue(int,int)` -- the
//                             SAME finding Wave B's WB-6 already made for
//                             `OCRChangeFile()`, a different absent member on
//                             the identical shim. -> GATE (WC-26).
//    pgcLotInfo->ActivePage   `pgcLotInfo` (this file:689) is declared as the
//                             STOCK `vclcompat::TPageControl` (only
//                             ->ActivePageIndex, an int) -- NOT the composed
//                             `TfLotInfoPageControl` that `pgLotinfo` (the
//                             OTHER page control in this same form) uses.
//                             Cannot retype an existing Wave A/B member from
//                             this wave. -> GATE (WC-12).
//    ATC_TYPE_61              Not a defined identifier anywhere in this tree
//                             (`grep -rn ATC_TYPE_61`, 20260819, 0 hits
//                             including ATC/ATCInterface.h). -> GATE (WC-10).
//                             *** STALE as of 20260826 (FW-LOTINFO-W30): that
//                             absence claim reads as false today -- 5 TUs carry
//                             a TU-local `#define ATC_TYPE_61 61`
//                             (csystem.cpp:20244, cTemperFrom.cpp:123,
//                             cUnitConvert.cpp:358, uHeaterThread.cpp:313,
//                             uTemp_Set.cpp:190).  Whether they all post-date
//                             20260819 was not established; either way the
//                             constant was never the real blocker.  See WC-10.
//    ReadWriteFTPAutomationData  Has ZERO real declaration/definition
//                             anywhere in this tree -- the only 2 tree-wide
//                             hits are this recon doc and
//                             RECON_uLotInfo_displayside.md, both prose. This
//                             OVERTURNS recon's own S34 note ("bRead=true
//                             分支單獨看是 (a)，不需要 GATE"): that note
//                             reasoned about the function's BEHAVIOUR
//                             assuming it exists, but the function itself has
//                             never been ported, independent of which of its
//                             own branches would run. -> GATE (WC-16).
//    TFileListBox has no ->Visible  `class TFileListBox : public TObject`
//                             (vclcompat/FileListBox.h:84) -- carries only
//                             Mask/Directory/Items, no Visible (it never
//                             inherits TControl). FormShow's SPILFunction
//                             branch writes `FileListBox1->Visible=false;`
//                             two lines before `FileListBox1->Directory=
//                             DataPath;`, which DOES compile. -> GATE (WC-6),
//                             one line only; the Directory line stays REAL.
//    JCET 2DID white-list guard depends on a gated call  S19's
//                             `if(CUSTOMER_CODE==CC_JCET){ bFlag=fBarCode->
//                             JCETUseMakeWhite2DIDList(); ... }` sub-block:
//                             `bFlag` (the guard for every line inside) comes
//                             ONLY from the same portless
//                             JCETUseMakeWhite2DIDList() Wave A's WA-7 already
//                             gates -- per the WB-13 precedent ("guard depends
//                             on a gated member -> the whole guarded section
//                             gates, not just the call"), the ENTIRE CC_JCET
//                             sub-block gates, not only its 2 call lines.
//                             -> GATE (WC-11).
//
//  WAVE SCOPE -- the 2 MIXED functions (golden span, this file's status)
//  --------------------------------------------------------------------------
//    FormShow        golden :316-1233 (918L, 35 segments per recon) -- 30 of
//                     35 segments REAL (several with 1-3 lines individually
//                     gated inside them); 5 segments (S5, S8, S14's call,
//                     S28, S30's 2 calls, S32) fully or almost-fully gated.
//    Timer2Timer     golden :6934-7191 (258L, 18 segments per recon) -- 15 of
//                     18 segments REAL; T9 (51L, WinWay ATC hardware) and T11
//                     (24L, WAR16123 3-hour alarm) fully gated per the task's
//                     explicit safety red line (not re-evaluated); T7 has 2
//                     lines individually gated.
//
//  GATE REGISTER (in FormShow/Timer2Timer reading order; WC-8 is one finding
//  reused at 2 call sites, matching Wave A's WA-3/Wave B's WB-1 style of
//  reusing a gate number for the identical dependency)
//  --------------------------------------------------------------------------
//  (WC-1) FormShow, golden :373 and :427 -- `ResetLotInfo()`, called once
//      inside the `IniConfig.bShowLotInfo` branch and once more,
//      unconditionally, 47 lines later (RECON ODDITY: golden calls the same
//      function twice with no state change between the calls). RECON item
//      #149, classified (b) write-path (writes `LastSet.bHasDownloadFile`),  → 20260927 St01 `61c96910` 退役（ResetLotInfo 已翻在 forms/fLotInfo.cpp 檔尾；呼叫點 FormShow 兩處＋wb_serve 開機 :4162＝golden main.cpp:10991）
//      not translated by any prior wave. Both call sites gated; everything
//      else in S3/S8 stays REAL.
//  (WC-2) FormShow, golden :392 -- `cbPATModeChange(cbPATMode);` inside the
//      CC_PANTHER branch. RECON item #169, (b) write-path dispatcher, not
//      translated. Gated; `tsLotID->TabVisible=...`, `cbPATMode->ItemIndex=0`
//      and the `#ifdef SOFT_SIMULTE` widget-visibility block around it stay
//      REAL.
//  (WC-3) *** RETIRED 20260826 by FW-LOTINFO-W30 -- CALL SITE OPENED. ***  Its
//      "writes files x4" reason was false when written (those writes are
//      RecordProcess, a stdout stand-in).  WD-5's latency warning SURVIVES the
//      retirement and now also covers this call site.  See the W30 banner.
//  (WC-3) FormShow, golden :399 -- `ShowXMLOnLine();`. RECON item #103: a
//      RecordProcess-backed function that writes files x4 despite the
//      "Show" name (the SAME "Show/Refresh prefix is not a reliability
//      signal" pattern this file's own banners already document twice).
//      Whole call gated.
//  (WC-4) FormShow, golden :418-419 -- `edtSysLotID->PopupMenu=
//      FormBarcodeReader->pmBarcode;` (x2, for edtSysLotID and
//      edtSysOperatorID). `FormBarcodeReader` has no port anywhere in this
//      tree (see STEP 0 above); `PopupMenu` is also not a modeled TControl
//      property (Controls.h's TControl carries only Visible/Enabled/hCtl).
//      Two independent reasons, one gate. `palCurrFailRate->Visible=false`/
//      `Panel27->Visible=false` above and `LotKeyInTime->Enabled=...` below
//      stay REAL.
//  (WC-5) FormShow, golden :486 and :513 -- `btnSaveClick(this);` (once in
//      the bSPILFunction branch, once in the AMKOR_China/QUALCOMM branch).
//      RECON item #38, (b) write-path (`WriteIniData` x19 inside). Both call
//      sites gated; every other line in both branches stays REAL.
//  (WC-6) FormShow, golden :467 -- `FileListBox1->Visible=false;` inside the
//      bSPILFunction branch. See "ADDITIONAL DEPENDENCY GAPS" above
//      (TFileListBox has no Visible). One line gated; `FileListBox1->
//      Directory=DataPath;` two lines later stays REAL.
//  (WC-7) FormShow, golden :553-562 -- the `MachineTypeChoice==
//      Type_HT9046_LS` block's entire payload is `imgRTCFullView1..4->
//      Width=.../2`. Same established WB-1 finding (TImage/TCanvas have zero
//      port anywhere in this tree), re-confirmed 20260819. `COM2-
//      >bCCDDummyRum` (the guard) is ALSO unverified/unneeded once the body
//      is gated. Whole block gated; the `tsBarCode`/`btChangeFile`/
//      `tsESDMonitor`/`ts_OCRInterface`/`ts_SocketInterface` lines
//      immediately before and after stay REAL.
//  (WC-8) *** RETIRED 20260826 by FW-LOTINFO-W30 -- BOTH CALL SITES OPENED. ***
//      It was WA-3's reason reused verbatim, so it dies with WA-3.
//  (WC-8) FormShow golden :597 and Timer2Timer golden :7143 --
//      `RefreshYieldMonitor();`. Reuses Wave A's WA-3 gate verbatim (same
//      target, same "not one of this wave's translated methods" reason,
//      re-verified 20260819). Both call sites gated; the tab-visibility
//      lines around each stay REAL.
//  (WC-9) FormShow, golden :604 -- `fBarCode->mtBarcodeSetDefaultView();`.
//      See STEP 0 above. Gated; the `chkTestMode->Visible=.../
//      labLevelMode->Visible=.../coLevelMode->Visible=...` lines immediately
//      after stay REAL.
//  (WC-10) *** RETIRED 20260826 by FW-LOTINFO-W30 -- LINE OPENED, but NOT for
//      the reason W27 predicted, and only after a SECOND blocker W27 missed was
//      cleared.  W27 said "spell it as the literal 61, matching this file's own
//      un-gated forms/fLotInfo.cpp:1280".  BOTH halves of that were wrong:
//      :1280 is INSIDE the WA-6 `#if 0` block, so it is not un-gated and is no
//      precedent; and `ATC_InterfaceForm` itself had NO declaration reachable
//      from that TU (every existing use sits inside some `#if 0`).  W30 opens it
//      with a TU-local ATC_TYPE_61 mirror (the tree's 5-TU idiom for this exact
//      constant, and what uTemp_Set.cpp:838 does with the identical golden line)
//      plus an `#include "acarry_shims.h"`.  The ATC_TYPE_61 clause is DEGRADED
//      (the shim's iATC_MODE_TYPE is permanently 0), safely.  See the W30 banner.
//  (WC-10) FormShow, golden :329 -- `ts_ATC6_1->TabVisible=(Tri_Temp_Machine
//      ==1 || (ATC_SYSTEM==eNewATCSystem && ATC_InterfaceForm->
//      iATC_MODE_TYPE==ATC_TYPE_61));`. See "ADDITIONAL DEPENDENCY GAPS"
//      above (`ATC_TYPE_61` undefined tree-wide). Whole line gated rather
//      than dropping just the `ATC_TYPE_61` clause, because that would
//      silently change the `||`'s truth table (a real behaviour change, not
//      a faithful subset) -- same reasoning this file's own S30/WC-19 note
//      uses for not splitting a compound condition.
//  (WC-11) FormShow, golden :874-896 -- the CC_JCET sub-block inside the
//      `TestIF_File.b2DIDAllowList` branch. See "ADDITIONAL DEPENDENCY GAPS"
//      above (guard depends on the WA-7-gated `fBarCode->
//      JCETUseMakeWhite2DIDList()`). Whole sub-block gated; its sibling
//      `else` branch (golden :897-903, plain visibility writes with no such
//      dependency) and the unconditional `labDeviceName->Visible=false;
//      edtDevice->Visible=false; lbProcess->Visible=false; cbProcess-
//      >Visible=false;` tail (golden :904-907, applies regardless of which
//      branch ran) both stay REAL.
//  (WC-12) FormShow, golden :732 -- `pgcLotInfo->ActivePage=tsVTest;` inside
//      the bVTESTFunction branch. See "ADDITIONAL DEPENDENCY GAPS" above
//      (pgcLotInfo is the stock TPageControl, no ->ActivePage). One line
//      gated; the entire rest of the (264-line) bVTESTFunction branch stays
//      REAL apart from WC-13/WC-14/WC-15 below.
//  (WC-13) FormShow, golden :788 and :794 -- `edtProcessName->Parent=
//      tsVTest;` / `edtProduct->Parent=tsVTest;`. VCL control re-parenting
//      has no model anywhere in vclcompat/Controls.h (`TControl` carries
//      only Visible/Enabled/hCtl -- confirmed by reading the class
//      definition this wave). Both lines gated; the immediately following
//      `->EditLabel->Caption=...` (translated as ->EditLabelCaption=...,
//      the SAME flattened-property idiom TLabeledEdit's own header comment
//      already documents), `->Left=`, `->Top=`, `->Enabled=true` writes on
//      both widgets stay REAL.
//  (WC-14) FormShow, golden :800-801 -- `sbSECSLotStart->Top=...;
//      sbSECSLotEnd->Top=...;`. Both are Wave A/B-declared `TSpeedButton*`
//      members (this file, "->Click()/->Down only" per WB-12's own
//      comment) with no geometry field; this wave cannot retype an existing
//      Wave A/B member. Both lines gated together (one decision block in
//      golden).
//  (WC-15) FormShow, golden :811 and :815 -- `WriteIniData(AuthPath+
//      "Security_new.def", "Network", "Temp Offset", false);` /
//      `WriteIniData(..., "Auto Clean", false);` inside the bVTESTFunction
//      branch. Recon already flags these as embedded writes (VTEST branch
//      locking Temp Offset/Auto Clean); same write-path class as WC-1/
//      WC-2/WC-5. Both gated; the `chkTempOffset->Enabled=false;
//      ->Checked=false;` / `checkbAutoClean->Enabled=false; ->Checked=
//      false;` pairs immediately before each stay REAL.
//  (WC-16) FormShow, golden :1228 -- `ReadWriteFTPAutomationData(true);`.
//      See "ADDITIONAL DEPENDENCY GAPS" above (function has zero port,
//      overturning recon's own S34 note). Gated; `AdjtsYieldMonitiorSize();`
//      and `InitialRefrigerantSystem();` on the same golden line group (both
//      already-real Tier 1 methods) stay REAL.
//  (WC-17) FormShow, golden :1231-1232 -- `if(USE_Scanner_AOI_Inspection==
//      (int)eBtnAOI_TopBottomInstall && IniConfig.
//      bA74AOIFailCountLinkLotRunMode) FrmAOI->AOIFailCountRefresh();`. See
//      STEP 0 above (`FrmAOI` exists, the method does not). Whole
//      single-statement `if` gated (there is no other payload to keep).
//  (WC-18) FormShow, golden :1125-1146 -- `myInShuttleLotInfo`/
//      `mtBarcodeInShLotInfo` SetColorMap/SetCellNumber/SetCellColorIndex
//      calls. See STEP 0 above. Whole segment gated;
//      `grpBarcodeDisplayLotInfo->Visible=...` immediately after stays REAL.
//  (WC-19) FormShow, golden :1156 and :1158 -- `tmrChamberBoost->Enabled=
//      (CosFunction.bUseChamberBoostMode);` and `SetLotStart("fLotInfo::
//      FormShow", true);`. SAFETY RED LINE per the task brief -- SetLotStart
//      is the 399-line SECS Lot-Start main control function (RECON #10),
//      not re-evaluated. DEVIATION FROM RECON'S OWN "gate all 3 lines as one
//      block" RECOMMENDATION: this wave keeps golden :1157 (`AnsiString
//      sPath=AuthPath+"config.ini";`) REAL and OUTSIDE the gate, because (a)
//      it is a pure local-variable construction with zero side effects, (b)
//      re-reading the raw golden text this wave confirms the three
//      statements are NOT inside a shared `if` in FormShow itself -- they
//      are three unconditional, sequential top-level statements; recon's
//      "同一個決策區塊" phrase describes logic INSIDE SetLotStart's own
//      body, not a surrounding conditional here -- and (c) S31 (golden
//      :1160, `edtJobSeq->Text=ReadIniData(sPath,...)`, already recon-
//      classified 顯示/純讀) hard-depends on `sPath` existing. Gating the
//      declaration along with its two neighbours would have silently broken
//      S31 too.
//  (WC-20) FormShow, golden :1166-1192 -- the `CosFunction.
//      bLotStartLockCriticalPara` access-control block (`fMain->
//      CheckCanChangeRealDummy()`/`HasICUnderMachine()` plus the
//      bEPLogStart_KYEC/bTempLogStart_KYEC/bESDLogStart_KYEC/
//      bArmTestInfoEvenLogStart_KYEC audit-flag cluster). SAFETY RED LINE per
//      the task brief, not re-evaluated. Whole segment gated.
//  (WC-21) FormShow, golden :664-678 -- the `CUSTOMER_CODE==CC_JSCC_OS`
//      branch. Writes `->Top`/`->Left` on `edtSysLotID`/`labLotID`/
//      `edtDevice`/`labDeviceName`/`btnFTPDownLoadbyDeviceID` (Wave A/B
//      members or plain-typed new members with no geometry field) and reads
//      `fTesterTCP->rgUnloader->ItemIndex` (unverified/out of this wave's
//      scope -- `fTesterTCP` is a Wave-A/B member, its `rgUnloader` shape
//      was not checked). Whole branch gated; the sibling `CC_ONSEMI_M`
//      branch immediately above and `CC_SIGURD_ChungXing` branch immediately
//      below both stay REAL.
//  (WC-22) FormShow, golden :1077-1078 -- `edtSysLotID->Width=150;
//      edtSysLotID->Left=150;` inside the SPIL EQC-mode (non-2DID-allowlist)
//      branch. `edtSysLotID` is a Wave A/B member with no geometry field
//      (same WC-14 reasoning: cannot retype an existing member from this
//      wave). Both lines gated; `palQAMode->Visible=true;`, `labLotID->
//      Caption=...`, `lbLotRunMode->Visible=false; cbRunMode->Visible=
//      false;` around them stay REAL.
//  (WC-23) FormShow, golden :1108 -- `labQACount->Font->Size=8;` inside the
//      KYEC_LEE&&bQAMode branch. `TLabel` (Controls.h) carries only Caption+
//      Color, no Font member. One line gated; `labQACount->Caption=...`,
//      `edQAMode->Left=136;`, `btnQAmodeSave->Left=210;` (both given
//      Left-carrying composed types this wave, see below) and `palQAMode->
//      Visible=true;` all stay REAL.
//  (WC-24) Timer2Timer, golden :7035-7085 (51L) -- the `ATC_SYSTEM==eWinWay`
//      WinWay ATC block (`fWinway->OpenCommPort()`/`arrATC_Site[i]->
//      GetST()`/`SetTempratureAll()`/`WinwayCOM->StopComm()`/`SetPT()`).
//      SAFETY RED LINE per the task brief (real hardware temperature/comm
//      control), not re-evaluated, gated verbatim as one 51-line block
//      exactly as recon specifies.
//  (WC-25) Timer2Timer, golden :7096-7119 (24L) -- the bVTESTFunction
//      3-hour send-for-inspection alarm (`ShowErrorMessage("WAR16123",
//      K_SKIP, MMSystem)`). SAFETY RED LINE per the task brief, not
//      re-evaluated, gated verbatim as one 24-line block exactly as recon
//      specifies (this also covers `Panel29->Caption="Server";` and
//      `labConfigL04->Caption=...`, both inside this same golden if-block).
//  (WC-26) Timer2Timer, golden :7021 -- `lblTester_LotID->Caption=
//      fOCR->sTesterLotId;`. See "ADDITIONAL DEPENDENCY GAPS" above. One
//      line gated; the `palHandlerwithTester->Caption="Connection";
//      ->Color=clLime;` above it (bGetLotIDFormTester==true arm) stays REAL.
//  (WC-27) Timer2Timer, golden :7027 -- `lblTester_LotID->Caption=
//      fSetup->edOcrText->Text;`. See STEP 0 above. One line gated; the
//      `palHandlerwithTester->Caption="No Connection"; ->Color=clRed;`
//      above it (the else arm) stays REAL.
//  (WC-28) FormShow, golden :987/:1008/:1012 -- `BtnPause->Visible=...` (x3,
//      inside S20's CC_GIGAS/CC_KYEC_LEE/else branches). Found only by the
//      MinGW syntax-check, not by reading: `BtnPause` is an existing member
//      typed `TfMainSpeedButton*` (FormWidgets.h) by an earlier wave, which
//      carries only `bool Down` -- no `Visible`. Same "cannot retype an
//      existing member from this wave" reasoning as WC-14/WC-22. All 3
//      writes gated; every other line in S20 (the CC_GIGAS 17-TCheckBox
//      loop, `tsLotID->TabVisible=false`, etc.) stays REAL.
// =============================================================================

// =============================================================================
//  AI(W906-FW-LOTINFO-W27) 20260826: uLotInfo Wave D -- 14 further methods.
//
//  Golden ref: HT9011UC_Code_V3.33.906.0_20260618/uLotInfo.cpp (16,613 lines,
//  cp950) + uLotInfo.h (1,426 lines).  Every one of the 14 bodies below, and
//  every body EXITED, was opened and read in full from the cp950 decode this
//  wave -- the keyword pre-scan was used only to ORDER the reading, never to
//  decide.  It had to be: the scan MISSED ReadWriteFTPAutomationData's config
//  write entirely, because that function spells its write ReadWriteIni(...,
//  bRead) rather than WriteIniData.
//
//  DENOMINATOR, RE-MEASURED THIS WAVE (20260826)
//  --------------------------------------------------------------------------
//  Method: decode golden cp950, blank out block/line comments AND string/char
//  literals (line structure preserved), then count TfLotInfo::<name>( at
//  definition sites.  Same stripper applied to this file's .cpp.
//    golden out-of-line definitions : 210   (209 distinct names; the extra one
//                                            is the genuine overload pair
//                                            UploadToServer(), golden :4852,
//                                            and UploadToServer(AnsiString,
//                                            bool), golden :4858)
//    ported before this wave        :  82
//    ported after this wave         :  96   -- 96/210 = 45.7% of golden's
//                                            out-of-line TfLotInfo method
//                                            DEFINITIONS.  Not lines, not
//                                            statements, not "the form".
//  The brief's "216 golden / 83 ported" both count comment or string-literal
//  occurrences; 6 of the 216 live inside comment or literal text.
//
//  BATCH CRITERION, AND WHY THIS BATCH IS 14 AND NOT 30-50
//  --------------------------------------------------------------------------
//  The task asked for 30-50 read-only-direction methods.  That is not
//  reachable from what is left, and the reason is the most useful number this
//  wave produced:
//
//    docs/RECON_uLotInfo_displayside.md's own classification table, crossed
//    against this file's ported set, leaves every one of the 128 missing
//    definitions in either "(b) write path" (103) or "(c) safety" (24), with
//    exactly ONE still in "(a) display/read-only" (ShowATC20Thermo, 486L --
//    and that one EXITS anyway, see the EXIT REGISTER).  Waves A+B+C already
//    drained the display tier of TfLotInfo.  What remains is the recipe
//    download/upload machinery, SECS lot start/end, ATC thermo control, and
//    the ASECL/OEE logging family.
//
//  So the criterion here was NOT "take the next N off a list".  It was:
//    (1) read every remaining body up to ~120 golden lines, plus every body
//        of any size whose name suggested display (Show*/Refresh*/Update*);
//    (2) RE-DERIVE the read-only/write classification from the text, because
//        recon's is demonstrably over-broad on this file (6 overturns below);
//    (3) deliver only what survives that re-derivation, with NO empty shells.
//  Everything not delivered is EXITED WHOLE, with a per-method reason and a
//  deciding golden line number, in the EXIT REGISTER further down.
//  Remaining after this wave: 114 definitions (113 names + the overload).
//
//  SIX RECON CLASSIFICATIONS OVERTURNED BY READING THE TEXT
//  --------------------------------------------------------------------------
//   1. RefreshYieldMonitor (:13310-13325) -- recon #115 "(b) write path".
//      It is a 3-way dispatcher and writes nothing itself.  The write recon
//      is pointing at lives one frame down, inside
//      RefreshYieldMonitor_TERAPOWER.  Delivered REAL; that write is gated at
//      its own site instead (WD-2), which is where it can actually be seen.
//   2. RefreshYieldMonitor_TERAPOWER (:13547-13623) -- recon #117 "(b)".
//      76 of its 77 lines are TestIF_File -> widget display fill.  Golden
//      :13618 alone is the AutoClean-interval write.  Delivered with ONLY
//      that line gated -- the same shape atester_ProcessCount.cpp's own
//      GATE D already uses for the identical target at 5 other call sites.
//   3. LotKeyInTimeTimer (:11543-11660) -- recon "(b)".  118 lines of Lot-ID
//      and Operator-ID text validation.  No file, no ini, no socket, no
//      motion.  Its only outward call is SetLotID(), already a real (no-op)
//      method in this file -- the SAME call WB-5 above already accepts.
//   4. UpdatePATSubMode (:15716-15743) -- recon "(b)".  Pure TComboBox
//      repopulation: Clear + Items->Add x11 + ItemIndex + Visible.
//   5. ShowXMLOnLine (:12099-12137) -- recon #103, and WC-3 above gates its
//      call site citing "writes files x4".  RE-CHECKED: those 4 writes are
//      RecordProcess(), and RecordProcess in THIS tree is
//      canary_support.cpp:109 -- a stdout printf stand-in.  The DB-writing
//      golden body is #if 0 at cMyDB.cpp:1832 awaiting the GA-1-B4 integrator
//      swap, and 582 already-translated call sites tree-wide call it freely.
//      So it is not a write path today.  IT WILL BECOME ONE after that swap;
//      that latency is recorded as WD-5 so the swap cannot land silently.
//   6. SetLotComponents (:2283-2320) -- recon "(b)", and recon section 4
//      item 4 explains why: golden :2285 RunInfo.bLotStart=!bLotEnd feeds
//      SECS DoLotStart de-duplication.  That reasoning is CORRECT and is
//      honoured -- but it covers ONE line out of 38.  The other 37 are widget
//      ->Enabled/->Down/->Caption.  Delivered with :2285 gated (WD-1).
//
//  WAVE SCOPE -- the 14 delivered methods (golden span, status)
//  --------------------------------------------------------------------------
//    LotKeyInTimeTimer               golden :11543-11660 (118L) -- WD-3 gates
//                                     the Operator-ID chain only
//    RefreshYieldMonitor_TERAPOWER   golden :13547-13623 (77L)  -- WD-2, 1 line
//    palSecsGemMouseDown             golden :10351-10425 (75L)  -- REAL, no gate
//    ShowXMLOnLine                   golden :12099-12137 (39L)  -- REAL, WD-5
//    SetLotComponents                golden :2283-2320   (38L)  -- WD-1, 1 line
//    sbTestClick                     golden :13719-13752 (34L)  -- REAL, no gate
//    cbFirstTrayCheckOnUnloaderClick golden :15994-16027 (34L)  -- REAL, no gate
//    btnAirStreamOnOffClick          golden :14461-14488 (28L)  -- REAL, and
//                                     golden's ENTIRE body is commented out
//    UpdatePATSubMode                golden :15716-15743 (28L)  -- REAL, no gate
//    RFID_ReaderReceiveData          golden :14152-14173 (22L)  -- REAL, D-1
//    RefreshYieldMonitor             golden :13310-13325 (16L)  -- REAL, no gate
//    sb_Main_EvenLevelLoginClick     golden :10475-10489 (15L)  -- REAL, no gate
//    SetFirstTrayCheckOnUnloader     golden :16036-16045 (10L)  -- REAL, no gate
//    btnAMRSetSECSClick              golden :16227-16232 (6L)   -- REAL, no gate
//  540 golden lines total.
//
//  GATE REGISTER
//  --------------------------------------------------------------------------
//  (WD-1) SetLotComponents, golden :2285 -- RunInfo.bLotStart=!bLotEnd;.
//      docs/RECON_uLotInfo_displayside.md section 4 item 4 names this exact
//      line: the SECS DoLotStart de-duplication path reads that flag, so a
//      display helper silently owning lot-start state is a real outward
//      consequence, not a widget write.  ONE line gated; golden :2286-2319
//      (edtSysLotID / sbSECSLotStart / sbSECSLotEnd / pnlLoader / edPage /
//      edtSysOperatorID / edCustomerLotId / coStation / edStationNum /
//      edtJobSeq / edtBarcodeRecipe / cbRunMode / edtCusLotID / edtCusDevGrp /
//      edtDevice / the 11 lbledt* / edtStage / edtStep, plus the
//      CC_PTI && bB03_TesterReport==false cbRunMode branch) all stay REAL.
//      BEHAVIOUR DELTA while gated: RunInfo.bLotStart keeps whatever value it
//      already had.  Nothing in this tree calls SetLotComponents today (its
//      only golden callers are SetLotStart/SetLotEnd, neither ported), so the
//      delta is unobservable until one of those lands.
//  (WD-2) RefreshYieldMonitor_TERAPOWER, golden :13618 --
//      fCleaning->ChangeACSmartInterval(2, "RefreshYieldMonitor");.
//      ESTABLISHED GATE, reused: atester_ProcessCount.cpp:1534 GATE D gates
//      the IDENTICAL target at 5 other golden call sites for the identical
//      reason.  RE-VERIFIED 20260826: ChangeACSmartInterval still has zero
//      declarations on the ported TfCleaning (forms/fCleaning.h); the only 4
//      tree-wide hits are that GATE D text, its
//      PCW7_CLEANING_CHANGEACSMARTINTERVAL macro, and WA-3's note above.
//      The guard if(iAdaptiveACInterval<=0) is kept REAL (with an empty body)
//      rather than deleted, so the shape stays diffable against golden.
//  (WD-3) LotKeyInTimeTimer, golden :11584-11637 -- the whole Operator-ID
//      if / else-if / else chain inside the CC_KYEC_LEE branch.
//      CAUSE: golden :11586-11587 reads fMain->cbRunStartMode->Text.
//      cbRunStartMode has NO PORT on TfMain -- verified 20260826: the hits in
//      forms/fMain.h are :571 (a comment) and :174/:180/:517
//      (SetMainRunStartMode, a different member, itself a documented no-op
//      stub).  ⚠ AND ONE NEAR-MISS WORTH NAMING, because it is exactly the
//      "TU-local stand-in" shape that makes absence claims go stale: there IS
//      a real cbRunStartMode-shaped object in this tree, but it is
//      csystem.cpp's TU-LOCAL seam W7C2_FMAIN_CBRUNSTARTMODE (csystem.cpp
//      :6388-6400).  It is file-static to csystem.cpp and reachable only
//      through that macro -- it does NOT put a member on TfMain, so it does
//      not help this file and must not be mistaken for a port.
//      cbRunStartMode is the FIRST clause of a 3-arm chain.
//      WHY THE WHOLE CHAIN AND NOT JUST THAT CLAUSE: dropping one disjunct
//      from a compound condition rewrites the truth table.  When golden's AMR
//      arm would fire, a clause-only gate falls through to the
//      Length<6 || Length>7 arm and CLEARS an operator ID that golden
//      deliberately keeps ("AGV" is 3 characters).  That is exactly the trap
//      this file's own WC-10 refuses to walk into.  Gating the chain instead
//      narrows to "never touch the operator-ID text", which does strictly
//      less rather than something different.
//      The Lot-ID half (golden :11561-11583) and the whole
//      CC_AMD_M && bHiSiliconFunction branch (golden :11639-11657) stay REAL.
//      GOLDEN ODDITY, translated verbatim and NOT "fixed": the gated chain's
//      first arm body (golden :11589-11590) is
//      sLastKeyin=edtSysOperatorID->Text; edtSysOperatorID->Text=sLastKeyin;
//      -- a round trip through a local, i.e. a no-op on the widget.  So even
//      un-gated it would do nothing; the gate costs the else-if arms only.
//  (WD-4) -- WITHDRAWN BEFORE LANDING, recorded because the withdrawal is the
//      finding.  It was going to gate palSecsGemMouseDown's TMouseButton
//      reads on the tree-wide "mbLeft/mbRight are portless" premise that
//      GATE (WB-2-BTN) above still states.  THAT PREMISE IS DEAD as of
//      20260826: vclcompat/ShiftState.h (AI(W906-FW-SHIFT), landed the same
//      day by a sibling wave) defines TMouseButton/TShiftState and pulls
//      mbLeft/mbRight/mbMiddle into the global namespace -- and THIS FILE
//      ALREADY INCLUDES IT (fLotInfo.h:17, added the same day by FW-SIG-W15).
//      palSecsGemMouseDown is therefore delivered with golden's FULL
//      5-parameter signature and no gate at all.  WB-2-BTN's own text is left
//      untouched here because this wave is append-only.
//  (WD-5) ShowXMLOnLine -- NOT a gate; a LATENCY WARNING attached to a real
//      translation.  The 4 RecordProcess(...) calls (golden :12111, :12117,
//      :12124, :12130) are translated LIVE, because RecordProcess resolves
//      today to canary_support.cpp:109 (stdout printf; no DB, no file) and
//      582 translated call sites tree-wide already call it.  When the
//      GA-1-B4 integrator swap activates cMyDB.cpp:1832's real body, these 4
//      lines become DB writes.  If that is unacceptable at that time, gate
//      them THEN -- pre-gating now would make this one function diverge from
//      the 582 sites doing exactly the same thing.
//
//  PREMISES RE-CHECKED THIS WAVE (all commands re-run 20260826)
//  --------------------------------------------------------------------------
//   * (WC-10) ATC_TYPE_61.  Its stated reason ("not a defined identifier
//     anywhere in this tree") is STILL LITERALLY TRUE for this file:
//     MainCalcCore.cpp:252's const int ATC_TYPE_61 = 61; is FUNCTION-LOCAL
//     and invisible outside that function body.  But it is no longer the
//     operative reason, because this same file already spells the sibling
//     constant as a bare literal at forms/fLotInfo.cpp:1280
//     (ATC_InterfaceForm->iATC_MODE_TYPE==61, un-gated).  CONCLUSION: WC-10
//     is openable, and the correct opening is the literal 61 matching :1280 --
//     NOT a claim that the identifier exists.  NOT DONE HERE: WC-10 lives
//     inside FormShow, i.e. inside existing lines, and this wave is
//     append-only.  Left for the integrator with the recipe spelled out.
//   * (WC-4) FormBarcodeReader.  Half dead, half alive: BarcodeReader.cpp:40
//     does carry a real global, but the members WC-4 needs still have zero
//     port -- there is no TPopupMenu class anywhere in vclcompat, and
//     vclcompat/Controls.h's TControl carries only Visible/Enabled/hCtl/Tag/
//     SetFocus, no PopupMenu.  STAYS GATED, on the second reason.
//   * (WA-3 / WC-8) RefreshYieldMonitor().  Their stated reason -- "not one of
//     this wave's translated methods" -- IS NOW DEAD: this wave translates it,
//     and its TERAPOWER branch, for real.  All 3 gated call sites
//     (AdjtsYieldMonitiorSize golden :13671 and :13678; FormShow golden :597;
//     Timer2Timer golden :7143) are openable.  NOT DONE HERE (append-only --
//     they are existing lines).  INTEGRATOR: each is a one-line #if 0
//     removal; the AutoClean write those gates were really protecting against
//     is now gated one frame deeper, at WD-2, where it belongs.
//   * (WB-18) CheckAMRAction().  STILL CORRECT.  Read in full this wave: it
//     calls LoaderAction / UnLoaderAction / CheckLDLevel (golden :16471,
//     :16472, :16475) and all three raise SECS EventReport(SECS_EVENT.AGV*)
//     (e.g. golden :16394, :16409).  Stays gated, and CheckAMRAction is
//     itself EXITED below for the same reason.
//   * (WC-3) ShowXMLOnLine().  Its stated reason ("writes files x4") is FALSE
//     TODAY -- see overturn 5 and WD-5.  The call site is openable.  NOT DONE
//     HERE (append-only).
//   * (WC-1) ResetLotInfo().  STILL CORRECT, and the function is EXITED --  → 20260927 St01 `61c96910`：前提已過期、WC-1 退役（lastdata 路徑 ctest 可轉開 012fbc13／260a29ca；wb_serve 本來就照 golden 寫 lastdata）
//     see the EXIT REGISTER entry, which adds a reason recon did not give.
//   * (WB-2-BTN) mbLeft/mbRight portless.  DEAD -- see WD-4.  Not edited here.
//   * (WB-6) fOCR->OCRChangeFile / (WC-26) fOCR->sTesterLotId.  Unchanged:
//     forms/fOCR.h still declares only ChangeLightValue(int,int).
//
//  EXIT REGISTER -- read whole, exited whole, with the deciding golden line.
//  No empty shell was produced for any of these; they are simply absent.
//  --------------------------------------------------------------------------
//  A. WRITES A FILE / INI / PERSISTED COUNTER
//     ReadWriteFTPAutomationData  :12401-12409 ReadWriteIni(...,bRead=false)
//                                  x9 into AuthPath+"config.ini".  This is the
//                                  one the keyword scan missed, and it
//                                  CONFIRMS WC-16 must stay gated regardless
//                                  of whether the function is ever ported.
//     SaveLotOperatorID           :8547-8548  same ReadWriteIni shape into
//                                  config.ini.
//     coLevelModeChange           :9913 CheckAndReadIniData -- the named trap:
//                                  common.cpp:602-603 WriteInteger's the key
//                                  back when absent.  Plus :9924/:9933
//                                  btnSaveClick().
//     btnSaveClick                :7271-7297 WriteIniData x17 into
//                                  AuthPath+"Security_new.def".
//     btnQAmodeSaveClick          :11524 WriteIniData into Tester.Data.
//     btnManualI49Click           :15918-15920 WriteIniData x3 into
//                                  Contact.Data (contact height).
//     WriteFTPSetupFileChangeLog  :13633 WriteDataToFile.
//     Save_BarCodeLog             :8455 Memo3->Lines->SaveToFile.
//     btClearBarcodeCountClick    :9995 SGDToXLS into D:\HT9045_Log.
//     btClearBarcodeListClick     :10012 list2DByLot->SaveToFile.
//     spOCRCleanListClick         :14189 ListOCRByLot->SaveToFile.
//     UploadEventLogFile          :10837 CopyFile.  → 20260928 A5：已移植（本機複製；ctest 經 W906_HT9045LOG_ROOT 到 machine_log_scratch），本體在 fLotInfo.cpp 檔尾
//     AlarmCodeUpload             :13290 SaveToFile, then :13297 FTP upload.
//     ClearAllSetupFile           :11866 DeleteDirectory, :11886
//                                  WriteLastDataFN.
//     _DelTree                    :2352/:2358/:2363 RemoveDir/DeleteFile.
//     btnAMRClearCountClick       :16198-16199 LastSet.iLoaderTotalTray and
//                                  iLoaderTrayCount_ART reset (persisted).
//     ResetLotInfo                :14556 LastSet.bHasDownloadFile=true.  → 20260927 St01 `61c96910`：這條理由已過期（W906_LastDataPath 轉向，Jimmy 012fbc13／260a29ca），ResetLotInfo 已翻、WC-1 退役
//                                  WC-1's premise re-read and CONFIRMED, with
//                                  a reason recon did not give: LastSet is
//                                  flushed by WriteLastDataFile, whose path is
//                                  a HARD-CODED literal (cprod.cpp:1729) that
//                                  --dry cannot redirect.  So this is a real
//                                  persistence risk on a dev machine, not a
//                                  memory-only assignment.
//     bCheckOnlyOneFileAndData    :14221/:14288 MyForceDirectories.
//     SetAQLMode / SaveOEELog / SaveBackEventLogInfo / SaveBackEventTracker /
//     SaveASECLTestLogInfo / SaveASECLNewTestLogInfo /
//     SaveASECLTesterHardwareLog / SaveGroundESDByLot /
//     SaveGroundESDData_Upolad / ProductTesterReport /
//     WhenTestRecordTemperatureLog_3Sigma / GenerateCheckList /
//     DoBackupSetupFile / DoOverWriteSetupFile
//                                 -- every one a log or recipe writer named by
//                                 its own body; several 100L+.
//  B. SENDS AN OUTBOUND COMMAND (FTP / SECS / TCP / RS232 / Vision COM)
//     btUploadClick :2329 DoUpload.  UploadToServer, both overloads, :4855 and
//     :4897-4929 RMSUpload*.  DoUpload :15462-15673.  RMSUploadByNetwork
//     :15420 and RMSDownloadByNetwork :15387 (CopyFile to the server share).
//     RMSUploadByFTP / RMSDownloadByFTP.  DownloadFromServer /
//     DownloadFromServer_TSMC / DownloadFromERMS / TimerERMSTimer /
//     btDownloadClick / btnFtpServerClick.  btnFTPDownLoadbyDeviceIDClick
//     :16076 btnFtpServer->Click() -- note this one would be INERT today
//     (TSpeedButton::Click() is an accepted offline no-op per WB-12), but its
//     golden meaning is "start an FTP download", so it is exited on meaning,
//     not on today's reachability.  sbRecipeUploadClick :13770 and
//     sbRecipeDownloadClick :13780 fFTPClient->ShowFTPModal.
//     sbFTPAutomationSaveClick :13786 ReadWriteFTPAutomationData(false).
//     btnFTPTryConnectClick :13838 CheckFTPConnection.
//     ATCTransferFileTimeTimer :16606 UploadFileToServer2.  AutoTempOfsByFTP.
//     spOCRSaveLogClick :8432 Save_BarCodeLog.  spSECSLotCheckClick :10029
//     EventReport(DoLotStart).  btnAMRSupplementClick :16168 /
//     btnAMRLDUnLDCheckClick :16175 / btnAMRLDUnLDFinishClick :16182
//     EventReport(SECS_EVENT.AGV*).  LoaderAction :16270-16363 /
//     UnLoaderAction :16394,:16409 / CheckLDLevel / CheckAMRAction
//     :16471-16475 (via those three).  sbSECSLotStartClick /
//     sbSECSLotEndClick / SetLotEnd / btnASECL_LotStartClick / ASECL_LotEnd /
//     btnASECL_LotEndClick :16157.  btSaveSetupFileClick.  Timer1Timer /
//     RTCChangeFile: //AI(W906-E020-LI6) 20261002 no longer exited -- translated in LotInfo_E020.cpp (V912 :5356-5475): the state
//     machine and flags run as golden, every COM2 vision send / reply read is GATEd and logged (TCOM2Shim has no RTC half); Timer1
//     is not ticked yet (timer card pending: W906_TfLotInfo_Timer1Timer).  DoReadRFID :14084 RFID_Reader->WriteCommData.
//     InitRFIDRS232 :14142 StartComm.  spOCRLogInClick :8532-8539
//     fMain->SendMSG_CMD.  ConnectNetDeviceS :7320 WNetAddConnection2.
//     sb_RunExecutFileClick :10448 ShellExecuteEx.  sbUploadPATClick.
//     btnPATInstallClick :14656 fMain->ChangeSetUpFile.
//  C. DRIVES THE MACHINE / SWITCHES A MODE / RAISES AN ALARM
//     OCRConnectTest :10280-10282 SW[SwOCRTigger].On()/Off() -- a real output
//     point.  BtnPauseClick :14530 fMain->Pause.  btnESCFunctionClick :12307
//     fMain->ResetForESC.  btnClearTemperatureClick :13848-13849
//     bChangeTest_TempOffset + DoCheckHasTestTempChange.  SetCloseSiteTemp
//     :14874 fTemp_Set->SetSingleWorkTemperature -- a real setpoint, for every
//     ATC site.  SetATCFFCOffset :9899/:9903 ATC_InterfaceForm->SetFFCOffset /
//     FFCOffsetEnable.  pl_ATC_OnlineClick :10249/:10267 ATC on/off-line
//     toggle.  ReadAirMachineStatus :14750-14772 SendAirMachineStatus.
//     btnStartChamberBoostClick :11741 and btnStopChamberBoostClick :11749
//     write bStartChamberBoost -- consumed by bthermo.cpp:503/:512, i.e. real
//     chamber heating, so these are NOT the harmless button-latch pair they
//     look like.  tmrChamberBoostTimer :11696-11697
//     btnStopChamberBoost->Click() + ShowErrorMessage("MES15401").
//     ShowATCThermo :5516 ShowErrorMessage("WAR15243") and :5500/:5504/:5508
//     dispatch into the three unported ShowATC*Thermo giants.  ShowATC70Thermo
//     :5790/:5818 SetRunATC(false/true).  ShowNewATCThermo :6469
//     SetATCOffset(true,true).  ShowATC20Thermo -- recon's LAST remaining "(a)
//     display" row, EXITED anyway: 15 ShowErrorMessage sites, and a body that
//     is almost entirely ATC_InterfaceForm-> members the 1-member shim
//     (acarry_shims.h:109-115) does not have -- the same finding WA-6 made.
//     SetATCOffset :9405-9870.  NetATCTimeTimer :8558-9350.
//     btnManualCheckListClick :13802 ShowErrorMessage("WAR16333").
//     cbRTCASTDClick :14702-14703 arms iRTC_AutoSTDTask.  rgOEEStateClick
//     :14510 fMesSystem->WriteOEEState (and fMesSystem still has no port at
//     all -- WB-14's finding, re-verified).  pnlXMLOnLineClick :12151-12162
//     flips iXMLOnLineStatus, i.e. it is the ENABLE SWITCH for the outbound
//     XML/FTP uploads that ShowXMLOnLine merely DISPLAYS.  The read half is
//     delivered and the write half is not; that split is deliberate.
//  D. DEPENDENCY ABSENT (would require a cross-file shim -- forbidden here)
//     UpdateLotInfoPAT / cbPATModeChange / ClearLotInfoPAT / sbLotResetClick /
//     btnRealTimeClick / btnpatHourlyClick / btnpatEndLotClick /
//     btnPATInstallClick / sbUploadPATClick -- all reach fMain->patFunc.
//     VERIFIED 20260826: the CLASS PAT_Function is fully ported
//     (ProductionInfo/uPAT_Function.h), but TfMain has NO patFunc MEMBER --
//     zero declaration-shaped hits tree-wide; the single textual hit is
//     ProductionInfo/uPAT_Function.cpp:263, a comment.  UpdatePATSubMode is
//     the one PAT-family method that touches only its own combo box, which is
//     exactly why it IS delivered.
//     DoPassword -- reaches fInput->fShow (golden :8492).  fInput has no port;
//     the only tree-wide hit is cConfiguration.cpp:6398, which is itself
//     GATE (W6-DoPassword) gating the IDENTICAL golden function on
//     TfConfiguration for the IDENTICAL reason.  Same gate, second form.
//     rgUnloaderClick :16082 fTesterTCP->rgUnloader -- fTesterTCP has no port
//     (WA-9, re-verified).
//     sbARMSShowClick :10348 fARMS->ShowModal() -- fARMS has no port; the only
//     hits are tests/test_DfmLayoutGen.cpp and tools/dfm2rc layout metadata,
//     i.e. generated data, not a class.
//     btnGetLoaderClick :14071 DoReadRFID(Ptr->Tag) -- DoReadRFID is itself
//     exited under B.
//     btnManualStandardClick :14056 fMain->CheckList().
//     edDeviceNameKeyPress :12027 btDownload->Click() (exited under B).
//     CheckingCheckList :12417 GenerateCheckList (exited under A).
//
//  DEVIATIONS
//  --------------------------------------------------------------------------
//  D-1  RFID_ReaderReceiveData's golden signature is
//       (TObject *Sender, Pointer Buffer, WORD BufferLength).  Pointer is
//       BCB6's VCL alias for void* and has no port; WORD is a Win32 typedef
//       this file does not pull in.  Translated as void *Buffer,
//       unsigned short BufferLength -- the EXACT underlying types, so this is
//       a spelling change, not the parameter-DROPPING simplification the WA
//       banner's DEVIATION note describes.  The body is unaffected.
//  D-2  This wave's new #includes sit at the TOP OF THE APPENDED BLOCK in the
//       .cpp rather than in the file's existing include section.  Unusual but
//       deliberate: the wave is strictly append-only, and all four
//       (<cstring>, <cstdlib>, forms/fSecurity.h, forms/fPassword.h) are
//       guarded headers declaring only externs and classes, so their position
//       in the TU is semantically irrelevant.
//  D-3  Golden spells some of its own widgets through the global, e.g.
//       fLotInfo->pnlLoader->Caption at golden :14168, from inside a TfLotInfo
//       method.  Kept VERBATIM rather than rewritten to bare member access,
//       matching this file's own Timer2Timer T18 precedent
//       (fLotInfo->palAQLMode->Visible=...).  Note the WA banner above
//       documents the OPPOSITE choice for fLotInfo->Height/Width/Top/Left.
//       Both spellings therefore exist in this file, as they do in golden;
//       decide per call site by reading, not by assuming.
//  D-4  This wave's header additions are INSERTIONS (this banner, and a member
//       block immediately before the ctor declaration), not literal file-tail
//       appends -- a class body cannot be extended from outside itself.  NO
//       EXISTING LINE in either file was modified; every pre-existing line is
//       byte-identical before and after.
// =============================================================================

// =============================================================================
//  AI(W906-FW-LOTINFO-W30) 20260826: uLotInfo Wave E -- a BEHAVIOUR-CHANGE wave.
//  It writes NO new translation.  It re-audits six gates whose stated reasons
//  FW-LOTINFO-W27 had already declared dead, and opens the ones that survive a
//  fresh audit.  W27 could not open them itself: it was append-only and all six
//  live inside pre-existing lines.
//
//  RESULT: 6 opened / 0 kept.  Two of the six needed a different opening from
//  the one W27 wrote down, and one needed a blocker cleared that W27 missed.
//
//  1. (WA-3) AdjtsYieldMonitiorSize, golden :13671 and :13678 -- OPENED (x2).
//  2. (WC-8) FormShow golden :597, Timer2Timer golden :7143  -- OPENED (x2).
//     Same finding; WC-8 was always WA-3's text reused.  Both rest on
//     RefreshYieldMonitor, landed real by W27 (fLotInfo.cpp:4396).
//     THE AUDIT THAT ACTUALLY MATTERED (task rule 3 -- "an opened gate really
//     runs; if it writes files / moves the machine / sends outward commands, do
//     not open"): the whole reachable chain was walked on 20260826, not just the
//     one call.  RefreshYieldMonitor -> {RefreshYieldMonitor_SIGURD,
//     RefreshYieldMonitor_TERAPOWER} -> AdjtsYieldMonitiorSize.
//       * SIGURD (fLotInfo.cpp:679-895) touches the filesystem ONLY through
//         ReadIniData against D:\HT9045_Log\CheckingList\<lot>.txt.  common.cpp
//         :684-686 documents that whole overload family as "Pure reads: no
//         seeding/writes" -- unlike its CheckAndReadIniData siblings, which DO
//         seed a missing key (common.cpp:678).  The other candidate write, the
//         UpdateFile() inside CloseIniFile (common.cpp:457) when OpenIniFile
//         switches path, is a NO-OP for TIniFile: vclcompat/IniFiles.cpp:281-286
//         only flushes when !writeThrough_, and TIniFile is write-through.  So
//         the known "TIniFile flush destroys an ini's layout" hazard is NOT
//         reachable here.  Its GetLastOpenFN() (common.cpp:1317) reads setup.inf
//         and can call ShowMyMessage, which is canary_support.cpp:143 -- stdout,
//         no modal dialog, so no batch-run hang either.
//       * TERAPOWER is TestIF_File -> widget fill; its ONE real write
//         (fCleaning->ChangeACSmartInterval) is gated at WD-2.
//       * AdjtsYieldMonitiorSize is widget geometry only.
//     MUTUAL RECURSION IS GOLDEN'S OWN AND TERMINATES: AdjtsYieldMonitiorSize
//     calls RefreshYieldMonitor, which calls AdjtsYieldMonitiorSize again.  The
//     static bTimerRunning guard (fLotInfo.cpp:4402-4404, golden :13312-13316)
//     is set BEFORE the dispatch and cleared after, so the re-entrant call
//     returns immediately.  Max depth 2.  Faithful, not accidental.
//  3. (WC-3) FormShow, golden :399 `ShowXMLOnLine();` -- OPENED.  Its stated
//     reason was false when written.  ⚠ THE WD-5 LATENCY WARNING IS NOT
//     RETIRED WITH IT and is repeated at the call site: after the GA-1-B4 swap
//     activates cMyDB.cpp:1831's real RecordProcess body, ShowXMLOnLine's 4
//     RecordProcess calls become DB writes, and THIS call site is what makes
//     them reachable from FormShow.
//  4. (WC-10) FormShow, golden :329 `ts_ATC6_1->TabVisible=...` -- OPENED, but
//     W27's written recipe for it was wrong twice over.  Both corrections are
//     recorded at the WC-10 register entry above and at the two fLotInfo.cpp
//     sites (the TU-local ATC_TYPE_61 block and the acarry_shims.h include).
//     Short form: (a) the "un-gated same-file precedent at fLotInfo.cpp:1280"
//     does not exist -- :1280 is inside the WA-6 `#if 0`; (b) the real blocker
//     was never the constant but `ATC_InterfaceForm`, which had no declaration
//     reachable from this TU at all.  Opened with the tree's own 5-TU TU-local
//     constant idiom plus one include, and the ATC clause is documented as
//     DEGRADED (shim's iATC_MODE_TYPE is permanently 0; no writer exists
//     tree-wide, grepped 20260826).
//  5/6. cMyDB.cpp:1929 and :2037 (golden cMyDB.cpp:1651 and :1744), both
//     `LotInName.sprintf("%s", fLotInfo->cbbASECL_LoginMode->Text);` -- OPENED.
//     The member landed in W27 (:2133 below).  Their edtASECL_TesterID
//     neighbours STAY gated: that member genuinely has no port.
//
//  ARCHIVE-EDGE ACCOUNTING (measured, not assumed -- nm on fLotInfo.o built
//  with the ht9045_forms flags + rsp, before vs after, 20260826)
//  --------------------------------------------------------------------------
//    defined symbols   : IDENTICAL (empty diff).
//    undefined symbols : +1, exactly `_ATC_InterfaceForm`.
//    forms -> ht9045_sm: 10 -> 11 symbols.  NOT a new edge -- that cycle is
//                        already declared and discussed at CMakeLists.txt
//                        :715-721.  (W27's report said 11 before; this wave
//                        measures 10 before / 11 after by intersecting
//                        fLotInfo.o's undefined set with libht9045_sm.a's
//                        extern defined set.  Method difference or drift; the
//                        DELTA of exactly +1 is what this wave is responsible
//                        for and it is exact.)
//    Nothing was added to any shared header; the only header edited is this one
//    and only in comments.
//
//  VERIFICATION
//  --------------------------------------------------------------------------
//    `g++ -fsyntax-only` with each target's own flags + includes_CXX.rsp:
//    fLotInfo.cpp (ht9045_forms) 0 errors; cMyDB.cpp (ht9045_db) 0 errors.
//    HEADER-INCLUDER COVERAGE, and a correction to W27's count: this header has
//    11 DIRECT .cpp includers, not 13 -- but FormsFacade.h includes it too, so
//    the TRANSITIVE set is ~59 TUs, and "13" understates the blast radius of any
//    real (non-comment) change here.  All 59 were re-checked this wave:
//    11 direct (forms/fLotInfo.cpp, cMyDB.cpp, uTemp_Set.cpp, Command.cpp,
//    cprod.cpp, cStartCondition.cpp, Automation/uRENESAS_Server.cpp,
//    TfAOILaserScan.cpp, tests/test_yieldmon_core.cpp, tests/test_ga1_cprod.cpp,
//    tests/test_ga1_cmydb.cpp) + 48 transitive, each with ITS OWN target's
//    flags.make + includes_CXX.rsp.  0 failures.
//    No build.bat / cmake --build / ctest was run (out of this wave's scope).
// =============================================================================

// ===========================================================================
//  AI(W906-FW3-LotInfo-WA) 20260819 -- Wave A composed widget stand-ins.
//  vclcompat's stock TPanel/TSpeedButton/TPageControl carry no raw pixel
//  geometry (Left/Top/Width/Height) or child-enumeration surface (that is a
//  deliberate "no rendering" scope call made elsewhere -- see
//  vclcompat/Controls.h). A handful of Tier 1 methods only ever WRITE a
//  couple of geometry ints, or read back one extra pointer VCL's real
//  TPageControl carries (->ActivePage) that this facade's TPageControl does
//  not. Composing (public-inherit + add the one missing field), not editing
//  the shared vclcompat header, is the same "compose, don't fork" idiom
//  forms/fTemperFrom.h's own TfTemperFromPanel/TfTemperFromTimer already use.
// ===========================================================================

// golden TPanel* Panel28 (->Width only) / Pan_ATC_Use_4Head,_8Head,_32Head
// (->Left/->Top only). One reusable type for all 4 -- nothing in Tier 1 reads
// any of these back, so all 4 ints default 0 (write-only within this wave).
class TfLotInfoLayoutPanel : public vclcompat::TPanel
{
public:
    int Left, Top, Width, Height;
    TfLotInfoLayoutPanel() : Left(0), Top(0), Width(0), Height(0) {}
};

// golden TSpeedButton* sbTest (->Top only, ShowInformation).
class TfLotInfoLayoutButton : public vclcompat::TSpeedButton
{
public:
    int Top;
    TfLotInfoLayoutButton() : Top(0) {}
};

// golden TPageControl* pgLotinfo -- AdjtsYieldMonitiorSize compares
// ->ActivePage against 15 different TTabSheet* members (real VCL TPageControl
// carries this pointer; vclcompat::TPageControl only models ->ActivePageIndex,
// which ShowInformation separately needs). Defaults NULL: no tab is
// "definitively active" offline, the same conservative-default posture as
// every other Visible/Enabled default in this facade -- AdjtsYieldMonitiorSize
// falls through every ->ActivePage== branch to its final `else` until a future
// wave wires a real active-tab notion.
class TfLotInfoPageControl : public vclcompat::TPageControl
{
public:
    vclcompat::TTabSheet *ActivePage;
    TfLotInfoPageControl() : ActivePage(0) {}
};

// golden TEdit* edTemp -- edTempKeyUp writes ->SelStart (cursor position),
// which vclcompat::TEdit does not model (a pure rendering/cursor concern,
// same "no rendering" scope call as every other geometry field above). Never
// read back anywhere in Tier 1's scope -- write-only int, default 0.
class TfLotInfoTextEdit : public vclcompat::TEdit
{
public:
    int SelStart;
    TfLotInfoTextEdit() : SelStart(0) {}
};

// golden TOpenDialog* OpenDialog1 (btnLoadFileClick: ->Title, ->Execute(),
// ->FileName). No vclcompat model exists (no prior consumer). Offline
// Execute() always returns false ("user cancelled") -- there is no real
// modal file picker in a headless build, matching this tree's established
// "no UI -> the interactive path never completes" convention (e.g.
// cinitial.cpp's own `W8N2_ShowMyMessageBox_YES_NO` returning a fixed "NO").
class TfLotInfoOpenDialog : public vclcompat::TObject
{
public:
    AnsiString Title;
    AnsiString FileName;
    TfLotInfoOpenDialog() {}
    virtual ~TfLotInfoOpenDialog() {}
    bool Execute() { return false; }
};

// ===========================================================================
//  AI(W906-FW3-LotInfo-WC) 20260819 -- Wave C composed widget stand-ins
//  (FormShow / Timer2Timer). Same "compose, don't fork" idiom as the Wave A
//  block above: vclcompat's stock TGroupBox/TLabeledEdit/TEdit/TButton carry
//  no raw pixel geometry (Left/Top/Width/Height). Golden FormShow writes
//  Left/Top on 4 widgets this wave introduces for the first time (so
//  composing a Layout variant costs nothing -- unlike WC-14/WC-22, which hit
//  Wave A/B members this wave cannot retype). None of these fields is read
//  back anywhere in this wave's scope, so every geometry int defaults to 0
//  (write-only), matching the Wave A TfLotInfoLayoutPanel precedent exactly.
// ===========================================================================

// golden TGroupBox* GroupBox3 (->Left/->Top, FormShow S10 ASE_KaohSiung/else
// ART-panel-vs-GroupBox swap) and grpOEEState (->Left only, FormShow S19
// bVTESTFunction branch). One reusable type for both.
class TfLotInfoLayoutGroupBox : public vclcompat::TGroupBox
{
public:
    int Left, Top, Width, Height;
    TfLotInfoLayoutGroupBox() : Left(0), Top(0), Width(0), Height(0) {}
};

// golden TLabeledEdit* edtProcessName/edtProduct (->Left/->Top, FormShow S19
// bVTESTFunction branch). ->EditLabel->Caption is translated as
// ->EditLabelCaption= directly on the base vclcompat::TLabeledEdit, the same
// flattened-property idiom that type's own Controls.h comment documents.
class TfLotInfoLayoutLabeledEdit : public vclcompat::TLabeledEdit
{
public:
    int Left, Top;
    TfLotInfoLayoutLabeledEdit() : Left(0), Top(0) {}
};

// golden TEdit* edQAMode (->Left, FormShow S25 KYEC_LEE&&bQAMode branch).
class TfLotInfoLayoutEdit : public vclcompat::TEdit
{
public:
    int Left;
    TfLotInfoLayoutEdit() : Left(0) {}
};

// golden TButton* btnQAmodeSave (->Left, same S25 branch as edQAMode above).
class TfLotInfoLayoutPushButton : public vclcompat::TButton
{
public:
    int Left;
    TfLotInfoLayoutPushButton() : Left(0) {}
};

// ===========================================================================
//  TfLotInfo -- non-VCL stub (golden uLotInfo.h).  cbRunMode is TComboBox* in
//  the golden; the leaves deref ->Visible (bool) and ->Text.Pos("RT")
//  (AnsiString).  Used inside the iResetSiteMappingStep==2 ASM path
//  (offline: Visible=false -- see forms/FormWidgets.h, that default is
//  LOAD-BEARING: ainarm_SearchPlacePlate.cpp:4811 reads it).
// ===========================================================================
class TfLotInfo
{
public:
    TfLotInfoRunMode *cbRunMode;                  // [DATA] golden uLotInfo.h:307 (TComboBox*)
    // -- W6.3 ADD: members the TRAY-ARM ENGINE (acatchtray.cpp) derefs -----------
    TfLotInfoLabel   *labNowLoaderTrayID;         // [DATA] golden uLotInfo.h:1078 (TLabel* loader tray-ID label)
    TfLotInfoEdit    *edtSysLotID;                // [DATA] golden uLotInfo.h:302  (TEdit* system lot-ID edit)
    // AI(W906-FW-Y3) 20260819: the 5 AutoClean-display members whose absence
    // kept uYieldMonitoring's (Y3) CalculateSiteYield block gated.
    TfLotInfoLabel   *Label17;                    // [DATA] golden uLotInfo.h:48 (TLabel* AutoClean low-yield user-set line)
    TfLotInfoLabel   *Label18;                    // [DATA] golden uLotInfo.h:49 (TLabel* AutoClean site-yield-diff user-set line)
    TfLotInfoLabel   *Label21;                    // [DATA] golden uLotInfo.h:55 (TLabel* AutoClean interval/contact user-set line)
    TfLotInfoEdit    *edtAutoCleanLowYield;       // [DATA] golden uLotInfo.h:50 (TEdit* live low-yield threshold display)
    TfLotInfoEdit    *edtAutoCleanSiteYieldDiff;  // [DATA] golden uLotInfo.h:51 (TEdit* live site-yield-diff display)
    // AI(W906-AutoCleanFoundation) 20260721: golden uLotInfo.h:1264 `int
    // iUnloaderTask[3];` (Eastsun 20260515 F011) -- the backing store
    // InitialUnLoaderTask (below) writes. Was missing entirely (the previous
    // stand-in's InitialUnLoaderTask didn't even declare it) -- see
    // InitialUnLoaderTask's own comment for the behaviour-change note.
    int iUnloaderTask[3];                         // [DATA] golden uLotInfo.h:1264
    // golden uLotInfo.cpp:16250-16253 -- REAL one-line body (was a total no-op
    // before that wave). 5 existing call sites in acatchtray.cpp (all inside
    // `#ifdef SOFT_SIMULTE`, which is #undef'd/commented-out in MachineType.h
    // -- verified via grep -- so this is currently dormant on every compiled
    // path; a genuine no-op -> real-write behaviour change ONLY if/when
    // SOFT_SIMULTE is ever defined). No existing test references
    // InitialUnLoaderTask or iUnloaderTask (grepped tests/ -- zero hits).
    virtual void InitialUnLoaderTask(int iPos);    // [METHOD] golden uLotInfo.h -- REAL body: iUnloaderTask[iPos]=1;
    // -- W5-Automation INTEGRATE ADD: members Automation/AMR.cpp + HANA_ART.cpp derefs --
    TfLotInfoRunMode *cbProcess;                  // [DATA] golden uLotInfo.h:470 (TComboBox*; only ->Text used)
    virtual void RefreshAMR();                    // [METHOD] golden uLotInfo.h:1416 -- offline: no UI to refresh (no-op)
    virtual void SetLotID(AnsiString ID, bool bReadFromFile=false);       // [METHOD] golden uLotInfo.h:1310 -- offline no-op
    virtual void SetLotStart(AnsiString sFunc, bool bReadFromFile=false); // [METHOD] golden uLotInfo.h:1311 -- offline no-op
    // -- W5-Final-TesterTCPSocket INTEGRATE ADD: members Interface/TesterTCP_Socket.cpp
    //    derefs (golden TesterTCP.cpp:170/180/621 labTCPIPStatus, :290/294 mmTesterLog) --
    TfLotInfoStatusLabel *labTCPIPStatus;         // [DATA] golden uLotInfo.h:465 (TLabel* TCP/IP link-status)
    TfMainMemo           *mmTesterLog;            // [DATA] golden uLotInfo.h:468 (TMemo* tester comm-log) -- reuse TfMainMemo shape (fAGV->mmE84Log precedent)
    // -- W5-Automation ADD (AGV_PortScan unit, 20260713): members
    //    Automation/AGV_PortScan.cpp derefs (AMR SPIL port-scan LEDs +
    //    E84 unloader tray-count scan's "remove tray" prompt panel) ----------
    TfLedValue     *ALedLoader;                   // [DATA] golden uLotInfo.h:971 (TALed*)
    TfLedValue     *aLedAuto[3];                  // [DATA] golden uLotInfo.h:965,969,970 (TALed* aLedAuto1/2/3, consolidated into an array -- same convention as fSortCT->pnlTrayCnt[6])
    TfLotInfoPanel *palRemoveTray;                 // [DATA] golden uLotInfo.h:828 (TPanel*, Enabled+Visible) -- see the ctor: its true/true initial state is set EXPLICITLY
    // AI(W906-Save2DSortingSummary) 20260723: 6 new members Automation/SCK_ART_Remainder.cpp's
    // SckArtRem_Save2DSortingSummary derefs (golden SCK_ART.cpp:3402-4061, only ->Text read/each --
    // reuse the existing TfLotInfoEdit shape, same idiom as edtSysLotID above).
    // mmo2DLotInfo is a TMemo* in golden (uLotInfo.h:846, `sList->Text=fLotInfo->mmo2DLotInfo->Text;`,
    // golden :3628) -- WIDENED by AI(W906-FW3-LotInfo-WA) 20260819 from
    // TfLotInfoEdit (TEdit) to vclcompat::TMemo (TCustomEdit still gives
    // ->Text, so the SCK_ART_Remainder ->Text-only consumer above is
    // unaffected): N23UseLotInfoFile (this wave, golden :7354-7371) needs
    // ->Lines->LoadFromFile()/->Lines->Count/->Lines->Strings[i], which TEdit
    // does not carry. This is exactly the "one-word edit... whenever their
    // form's own wave next touches them" repointing forms/FormWidgets.h's own
    // TfLotInfoEdit banner comment anticipated.
    TfLotInfoEdit *edtCusLotID;                    // [DATA] golden uLotInfo.h:623 (TEdit* customer lot-ID edit)
    TfLotInfoEdit *edtCusDevGrp;                   // [DATA] golden uLotInfo.h:624 (TEdit* customer device-group edit)
    TfLotInfoEdit *edtCusStep;                     // [DATA] golden uLotInfo.h:1038 (TEdit* customer step-no edit)
    TfLotInfoEdit *edtDevice;                      // [DATA] golden uLotInfo.h:625 (TEdit* device-name edit)
    TfLotInfoEdit *edtSysOperatorID;                // [DATA] golden uLotInfo.h:306 (TEdit* system operator-ID edit)
    vclcompat::TMemo *mmo2DLotInfo;                 // [DATA] golden uLotInfo.h:846 (TMemo*) -- ->Text (pre-existing) + ->Lines (this wave)
    // AI(W906-SaveTestSummaryTSV) 20260728: 4 new members Automation/SCK_ART_Remainder.cpp's
    // SckArtRem_SaveTestSummaryTSV derefs (golden SCK_ART.cpp:2884/3087, only ->Text read each --
    // reuse the existing TfLotInfoEdit shape, same idiom as edtCusLotID above).
    // golden edtASECL_LotID/edInsertion/edFlowID are TEdit* (uLotInfo.h:576/1044/1046); lbledtCustomer
    // is a TLabeledEdit* (uLotInfo.h:609, golden SckArtRem_SaveSummaryTrayFeed's own dependency) --
    // only ->Text is ever read from it here too, so the same minimal shape applies.
    TfLotInfoEdit *edtASECL_LotID;                  // [DATA] golden uLotInfo.h:576 (TEdit* ASE-CL lot-ID edit)
    TfLotInfoEdit *edInsertion;                     // [DATA] golden uLotInfo.h:1044 (TEdit* insertion-code edit)
    TfLotInfoEdit *edFlowID;                        // [DATA] golden uLotInfo.h:1046 (TEdit* flow-ID edit)
    // AI(W906-SaveSummaryTrayFeed) 20260728: golden SCK_ART.cpp:3157 (`fLotInfo->lbledtCustomer->Text`).
    TfLotInfoEdit *lbledtCustomer;                  // [DATA] golden uLotInfo.h:609 (TLabeledEdit* customer name edit)
    // AI(W906-W7-L1-Wave0) 20260801: W7-L1 Wave-0 ADD -- the 7 fLotInfo members
    // the asendic_* tray SM family dereferences, landed in one serialized pass so
    // the four parallel translation agents cannot collide on this header.  Every
    // golden line was re-read from the cp950-decoded golden uLotInfo.h this pass.
    // The first three are golden TLabel* whose Caption golden reads back through
    // atoi() (asendic_Loader.cpp:2069-2070), so they are ordinary Caption storage.
    //
    // OFFLINE-DEFAULT CONSEQUENCE, RECORDED BECAUSE IT IS NOT NEUTRAL: all three
    // Captions default to "" and atoi("")==0, which makes golden's KYEC-AMR
    // overflow guard at asendic_Loader.cpp:2068-2070
    //   (... atoi(LabDiffTrayCount->Caption)==0 &&
    //        (atoi(labLoaderTrayCount->Caption)-atoi(labNowTrayCount->Caption))==0)
    // evaluate TRUE whenever the surrounding TrayForm.bEnableAMR arm is entered.
    // A test that turns bEnableAMR on must therefore set all three captions
    // deliberately rather than relying on the constructed state.
    TfLotInfoLabel *LabDiffTrayCount;               // [DATA] golden uLotInfo.h:1086 (TLabel*) -- Caption read via atoi()
    TfLotInfoLabel *labLoaderTrayCount;             // [DATA] golden uLotInfo.h:1080 (TLabel*) -- Caption read via atoi()
    TfLotInfoLabel *labNowTrayCount;                // [DATA] golden uLotInfo.h:1084 (TLabel*) -- Caption read via atoi()
    TfLotInfoLabel *labNowAuto1TrayID;              // [DATA] golden uLotInfo.h:799 (TLabel*) -- Caption written (asendic_Auto.cpp:561)
    TfLotInfoLabel *labNowAuto2TrayID;              // [DATA] golden uLotInfo.h:802 (TLabel*) -- (asendic_Auto.cpp:562)
    TfLotInfoLabel *labNowAuto3TrayID;              // [DATA] golden uLotInfo.h:804 (TLabel*) -- (asendic_Auto.cpp:563)
    // Golden uLotInfo.h:1026 is a TCheckBox*, read-only in this family
    // (asendic_Auto.cpp:1994 `->Checked==true`).  Reuses the TfMainCheckBox alias
    // (forms/FormWidgets.h:114 -> vclcompat::TCheckBox, default Checked=false) --
    // the same alias fMain->chkE84IDTray uses, so no new widget stand-in is needed.
    TfMainCheckBox *cbFirstTrayCheckOnUnloader;     // [DATA] golden uLotInfo.h:1026 (TCheckBox*) -- offline Checked=false
    // AI(W906-PT-W3-integrate) 20260808: the 1 member the PT-W3 unit
    // Automation/uRENESAS_Server.cpp derefs (its "FACADE ADDITIONS NEEDED" banner,
    // uRENESAS_Server.cpp:105, is the measurement).  Only ->Click() is ever touched
    // (uRENESAS_Server.cpp:917/:1028, golden uLotInfo.cpp:1864 / :8231).
    // SHAPE: spelled `vclcompat::TButton*` rather than adding a TfLotInfoButton
    // alias, so forms/FormWidgets.h is NOT edited for a single member -- the same
    // decision forms/fShuttleMove.h:104 already took for its btRetry.  Confirmed a
    // real TButton in golden (uLotInfo.h:266 `TButton *btClearBarcodeList;`).
    // BEHAVIOUR: vclcompat::TControl::Click() (vclcompat/Controls.h:222) is an
    // offline no-op, so golden's OnClick handler -- btClearBarcodeListClick,
    // golden uLotInfo.cpp:10005-10014: zeroes iBarcodeReject, clears
    // fBarCode->map2DList / list2DByLot, re-sends the CCD "2DID by lot list"
    // command, RecordProcess, SaveToFile(asBarCodeLot), then chains
    // btClearBarcodeCount->Click() -- does NOT run.  That whole chain is
    // fBarCode-owned and unported, so a real body here would have nothing to call.
    // NOTE: csystem.cpp:2817 gates this identical golden call behind its TU-local
    // W7C2_FLOTINFO_CLEARBARCODE() macro (4 sites), whose stated premise ("member
    // absent") stops being true with this ADD.  Behaviour is unchanged either way
    // today (both are no-ops), but that macro is now retirable.
    vclcompat::TButton *btClearBarcodeList;         // [WIDGET] golden uLotInfo.h:266 (TButton*) -- ->Click() only

    // =======================================================================
    //  AI(W906-FW3-LotInfo-WA) 20260819: Wave A ADD -- see file banner above
    //  for WAVE SCOPE / GATE REGISTER / DEVIATION.  Grouped per Tier-1 method,
    //  each member cites the golden uLotInfo.h line it mirrors.
    // =======================================================================

    // -- RefreshYieldMonitor_SIGURD (golden :13327-13545) -- 37 widgets ------
    TGroupBox    *gbManualCheckList;             // golden uLotInfo.h:525
    TButton      *btnManualStandard;             // golden uLotInfo.h:527
    TCheckBox    *cbMonitor_FTPRMS;              // golden uLotInfo.h:481
    TComboBox    *cbContactMode;                 // golden uLotInfo.h:482 -- WA-1 gates the fContact-> RHS assigned into this
    TCheckBox    *cbSiteYieldCmp_FT;             // golden uLotInfo.h:486
    TCheckBox    *cbLowYieldByTotal_FT;          // golden uLotInfo.h:495
    TEdit        *edSiteYieldCmpOnOff_Cur;       // golden uLotInfo.h:491
    TEdit        *edLowYieldByTotalOnOff_Cur;    // golden uLotInfo.h:500
    TEdit        *edSiteYieldCmpIg_FT;           // golden uLotInfo.h:488
    TEdit        *edLowYieldByTotalIg_FT;        // golden uLotInfo.h:496
    TEdit        *edSiteYieldCmpIg_Cur;          // golden uLotInfo.h:489
    TEdit        *edLowYieldByTotalIg_Cur;       // golden uLotInfo.h:498
    TLabel       *lblSiteYieldCmpIg_Cur;         // golden uLotInfo.h:485
    TLabel       *lblLowYieldByTotalIg_Cur;      // golden uLotInfo.h:494
    TEdit        *edSiteYieldCmp_FT;             // golden uLotInfo.h:487
    TEdit        *edSiteYieldCmp_Cur;            // golden uLotInfo.h:490
    TLabel       *lblSiteYieldCmp_Cur;           // golden uLotInfo.h:484
    TEdit        *edLowYieldByTotal_FT;          // golden uLotInfo.h:497
    TEdit        *edLowYieldByTotal_Cur;         // golden uLotInfo.h:499
    TLabel       *lblLowYieldByTotal_Cur;        // golden uLotInfo.h:493
    TRadioButton *rbContsFailBySocket_FTOn;      // golden uLotInfo.h:504
    TRadioButton *rbContsFailBySocket_FTOff;     // golden uLotInfo.h:505
    TEdit        *edtContsFailBySocket_Cur;      // golden uLotInfo.h:508
    TEdit        *edContsFailSocketAlarmCT_FT;   // golden uLotInfo.h:506
    TEdit        *edContsFailSocketAlarmCT_Cur;  // golden uLotInfo.h:507
    TLabel       *lblContsFailSocketAlarmCT_Cur; // golden uLotInfo.h:503
    TRadioButton *rbContsFailByHead_FTOn;        // golden uLotInfo.h:512
    TRadioButton *rbContsFailByHead_FTOff;       // golden uLotInfo.h:513
    TEdit        *edtContsFailByHead_Cur;        // golden uLotInfo.h:516
    TEdit        *edContsFailHeadAlarmCT_FT;     // golden uLotInfo.h:514
    TEdit        *edContsFailHeadAlarmCT_Cur;    // golden uLotInfo.h:515
    TLabel       *lblContsFailHeadAlarmCT_Cur;   // golden uLotInfo.h:511
    TEdit        *edOSBin;                       // golden uLotInfo.h:520
    TEdit        *edOSBinCnt;                    // golden uLotInfo.h:522
    TEdit        *edOSBinCnt_Cur;                // golden uLotInfo.h:523
    TEdit        *edOSBinPreset;                 // golden uLotInfo.h:521
    TEdit        *edOSBinPreset_Cur;             // golden uLotInfo.h:524
    TLabel       *lblOSBin_Cur;                  // golden uLotInfo.h:519
    TGroupBox    *grpOSBin;                      // golden uLotInfo.h:517
    virtual void RefreshYieldMonitor_SIGURD();   // golden uLotInfo.cpp:13327-13545

    // -- TransformTemperature_AirStream (golden :15080-15193) -- zero new ---
    // widgets (fMain->edWorkTemperBase already existed before this wave).
    virtual double TransformTemperature_AirStream(double Offset, int iIndex);  // golden uLotInfo.cpp:15080-15193

    // -- AdjtsYieldMonitiorSize (golden :13636-13717) ------------------------
    TfLotInfoPageControl *pgLotinfo;              // golden uLotInfo.h:25 (TPageControl*)
    vclcompat::TPageControl *pgcLotInfo;           // golden uLotInfo.h:581 (TPageControl*) -- ->Visible only
    TTabSheet *ts_ATC6_1;                          // golden uLotInfo.h:637
    TTabSheet *tsATC;                              // golden uLotInfo.h:31  (shared w/ SetATCFormVisible)
    TTabSheet *tsASECLEventLog;                    // golden uLotInfo.h:237
    TTabSheet *tsYieldMonitior;                    // golden uLotInfo.h:462
    TTabSheet *tsFTP;                              // golden uLotInfo.h:28
    TTabSheet *tsLotID;                            // golden uLotInfo.h:27
    TTabSheet *tsMurata;                           // golden uLotInfo.h:582
    TTabSheet *tsSigurd_CX;                        // golden uLotInfo.h:583
    TTabSheet *tsSPIL_SZ;                          // golden uLotInfo.h:594
    TTabSheet *tsOEE;                              // golden uLotInfo.h:597
    TTabSheet *ts2DSort;                           // golden uLotInfo.h:604
    TTabSheet *tsChipAdv;                          // golden uLotInfo.h:605 (shared w/ SettsChipAdvVisible)
    TTabSheet *tsVTest;                            // golden uLotInfo.h:792
    TTabSheet *tsOCRBarCode;                       // golden uLotInfo.h:77
    TTabSheet *tsAMR;                              // golden uLotInfo.h:996
    int Height, Width, Top, Left;                  // golden TForm properties (shared w/ ShowInformation) -- NSDMI 0
    virtual void AdjtsYieldMonitiorSize();          // golden uLotInfo.cpp:13636-13717

    // -- SetSelectionVisible (golden :1262-1342) -----------------------------
    TGroupBox *groupbDownloadItem;                 // golden uLotInfo.h:247
    TGroupBox *grpMesCheck;                        // golden uLotInfo.h:788
    TTabSheet *tsSelection;                        // golden uLotInfo.h:40
    TfLotInfoLayoutPanel *Panel28;                  // golden uLotInfo.h:303 (TPanel*) -- ->Width only
    TComboBox *cbTestTimes;                        // golden uLotInfo.h:990
    TLabel    *lblTestTimes;                       // golden uLotInfo.h:991
    TLabel    *lblDownloadAccessWarning;           // golden uLotInfo.h:1120
    virtual void SetSelectionVisible();             // golden uLotInfo.cpp:1262-1342

    // -- ShowSocketID (golden :11443-11516) ----------------------------------
    TPanel *SocketSiteCH_Display[MAX_SOCKET_ROW][MAX_SOCKET_COL];  // golden uLotInfo.h:1327 (TPanel*[4][8])
    TEdit  *edSocket[MAX_SOCKET_ROW][MAX_SOCKET_COL];              // golden uLotInfo.h:1326 (TEdit*[4][8])
    // NOTE: golden's SocketLabRow_Display[MAX_SOCKET_ROW]/SocketLabCol_Display
    // [MAX_SOCKET_COL] are FILE-SCOPE globals (uLotInfo.cpp:89-90), not
    // TfLotInfo members -- declared/defined that way in forms/fLotInfo.cpp too
    // (same treatment as iLoaderTask[]/iloaderLevelTask[]/bLotFirstKeyIn/
    // bLotID_OK below).
    virtual void ShowSocketID();                    // golden uLotInfo.cpp:11443-11516

    // -- CheckAirMachineStatus (golden :14805-14868) -- WA-6: body gated, ---
    // no new widgets declared (see GATE REGISTER; adding Caption-only widgets
    // nothing writes into this wave would be unused surface).
    virtual void CheckAirMachineStatus();

    // -- InitialRefrigerantSystem (golden :14877-14939) -- 72 named panels --
    bool bInitFormcomponent;                       // golden uLotInfo.h:1396
    bool OldRefrigerantCommand;                    // golden uLotInfo.h:1397
    TTabSheet *ts_RefrigerantStatus_Page_2;        // golden uLotInfo.h:738
    // group 1..8 (golden uLotInfo.h, individually __published, see :690-786):
    TPanel *pnlRefrigerantMachine1, *pnlRefrigerantMachine2, *pnlRefrigerantMachine3, *pnlRefrigerantMachine4;
    TPanel *pnlRefrigerantMachine5, *pnlRefrigerantMachine6, *pnlRefrigerantMachine7, *pnlRefrigerantMachine8;
    TLabel *LabRefrigerantValue1, *LabRefrigerantValue2, *LabRefrigerantValue3, *LabRefrigerantValue4;
    TLabel *LabRefrigerantValue5, *LabRefrigerantValue6, *LabRefrigerantValue7, *LabRefrigerantValue8;
    TPanel *pnlRefCopm1Status_1, *pnlRefCopm1Status_2, *pnlRefCopm1Status_3, *pnlRefCopm1Status_4;
    TPanel *pnlRefCopm1Status_5, *pnlRefCopm1Status_6, *pnlRefCopm1Status_7, *pnlRefCopm1Status_8;
    TPanel *pnlRefCopm2Status_1, *pnlRefCopm2Status_2, *pnlRefCopm2Status_3, *pnlRefCopm2Status_4;
    TPanel *pnlRefCopm2Status_5, *pnlRefCopm2Status_6, *pnlRefCopm2Status_7, *pnlRefCopm2Status_8;
    TLabel *labRefCopm1HpValue_1, *labRefCopm1HpValue_2, *labRefCopm1HpValue_3, *labRefCopm1HpValue_4;
    TLabel *labRefCopm1HpValue_5, *labRefCopm1HpValue_6, *labRefCopm1HpValue_7, *labRefCopm1HpValue_8;
    TLabel *labRefCopm2HpValue_1, *labRefCopm2HpValue_2, *labRefCopm2HpValue_3, *labRefCopm2HpValue_4;
    TLabel *labRefCopm2HpValue_5, *labRefCopm2HpValue_6, *labRefCopm2HpValue_7, *labRefCopm2HpValue_8;
    TLabel *labRefCopm1LpValue_1, *labRefCopm1LpValue_2, *labRefCopm1LpValue_3, *labRefCopm1LpValue_4;
    TLabel *labRefCopm1LpValue_5, *labRefCopm1LpValue_6, *labRefCopm1LpValue_7, *labRefCopm1LpValue_8;
    TLabel *labRefCopm2LpValue_1, *labRefCopm2LpValue_2, *labRefCopm2LpValue_3, *labRefCopm2LpValue_4;
    TLabel *labRefCopm2LpValue_5, *labRefCopm2LpValue_6, *labRefCopm2LpValue_7, *labRefCopm2LpValue_8;
    TLabel *LabRefrigerantAdjustValue1, *LabRefrigerantAdjustValue2, *LabRefrigerantAdjustValue3, *LabRefrigerantAdjustValue4;
    TLabel *LabRefrigerantAdjustValue5, *LabRefrigerantAdjustValue6, *LabRefrigerantAdjustValue7, *LabRefrigerantAdjustValue8;
    // destination arrays golden .h:1387-1395 (already __published as arrays there)
    TPanel *TripnlRefrigerantMachine[8];
    TLabel *TriLabRefrigerantValue[8];
    TPanel *TripnlRefCopm1Status[8];
    TPanel *TripnlRefCopm2Status[8];
    TLabel *TriLabRefCopm1HpValue[8];
    TLabel *TriLabRefCopm2HpValue[8];
    TLabel *TriLabRefCopm1LpValue[8];
    TLabel *TriLabRefCopm2LpValue[8];
    TLabel *TriLabRefrigerantAdjustValue[8];
    virtual void InitialRefrigerantSystem();        // golden uLotInfo.cpp:14877-14939 -- WA-5 gates the ATC_OFFLINE_FormComInit() call

    // -- SetATCFormVisible (golden :9938-9985) -------------------------------
    TPanel *palATC;                                 // golden uLotInfo.h:33
    TPanel *aldATC7Status;                          // golden uLotInfo.h:958 (TALed* -- conflated to TPanel, ->Visible only)
    TLabel *lblATC70;                               // golden uLotInfo.h:959
    TPanel *aldATCChillerStatus;                    // golden uLotInfo.h:849 (TALed* -- conflated to TPanel, ->Visible only)
    TLabel *lblChiller;                             // golden uLotInfo.h:851
    TLabel *lblATC_Now_RecipeFile;                  // golden uLotInfo.h:854
    TPanel *pan_ATCChillerSV;                       // golden uLotInfo.h:855
    TPanel *pl_ATCChillerSV;                        // golden uLotInfo.h:856
    struct TfLotInfoTimer { bool Enabled; TfLotInfoTimer():Enabled(false){} } *NetATCTime;  // golden uLotInfo.h:83 (TTimer*) -- ->Enabled only
    virtual void SetATCFormVisible();                // golden uLotInfo.cpp:9938-9985

    // -- CheckEventLogParameter (golden :10673-10720) ------------------------
    TEdit *edCustomerDevice;                        // golden uLotInfo.h:1045
    TSpeedButton *btnASECL_LotStart;                // golden uLotInfo.h:241 -- ->Down only
    virtual bool CheckEventLogParameter();           // golden uLotInfo.cpp:10673-10720

    // -- ShowATCTempPanel (golden :14561-14606) ------------------------------
    TfLotInfoLayoutPanel *Pan_ATC_Use_4Head;        // golden uLotInfo.h:872 (TPanel*) -- ->Left/->Top
    TfLotInfoLayoutPanel *Pan_ATC_Use_8Head;        // golden uLotInfo.h:859 (TPanel*) -- ->Left/->Top
    TfLotInfoLayoutPanel *Pan_ATC_Use_32Head;       // golden uLotInfo.h:885 (TPanel*) -- ->Visible only, layout type reused for symmetry
    TPanel *ATCChPal[ATC_HEAD_COUNT];               // golden uLotInfo.h:1328
    TPanel *ATCPtr[ATC_HEAD_COUNT];                 // golden uLotInfo.h:1330
    TPanel *ATCReferPtr[ATC_HEAD_COUNT];            // golden uLotInfo.h:1329
    virtual void ShowATCTempPanel();  void ResetLotInfo();  /* AI(W906-FRW-WC1) 20260927 (Steven 團隊)：golden uLotInfo.cpp:14820-14834（GATE WC-1 退役）；本體 forms/fLotInfo.cpp 檔尾；接在同一行，不移動行號 */  // golden uLotInfo.cpp:14561-14606

    // -- JCETWhite2DIDShow (golden :15953-15992) -----------------------------
    TLabel *labCusLotID;                            // golden uLotInfo.h:620
    virtual void JCETWhite2DIDShow(bool bUse);       // golden uLotInfo.cpp:15953-15992

    // -- ShowInformation (golden :12326-12362) -------------------------------
    TGroupBox *gbFTPAutomation_Download;            // golden uLotInfo.h:438
    TGroupBox *gbFTPAutomation_Upload;              // golden uLotInfo.h:447
    TSpeedButton *sbRecipeUpload;                   // golden uLotInfo.h:459
    TSpeedButton *sbRecipeDownload;                 // golden uLotInfo.h:460
    TSpeedButton *sbFTPAutomationSave;               // golden uLotInfo.h:461
    TfLotInfoLayoutButton *sbTest;                   // golden uLotInfo.h:458 (TSpeedButton*) -- ->Top only
    virtual void ShowInformation(bool bShow);        // golden uLotInfo.cpp:12326-12362

    // -- N23UseLotInfoFile (golden :7346-7380) -- zero new widgets (all 4 -----
    // dependencies -- mmo2DLotInfo/edtCusLotID/edtCusDevGrp/edtDevice -- were
    // already in this facade before this wave).
    virtual bool N23UseLotInfoFile();                // golden uLotInfo.cpp:7346-7380

    // -- edTempKeyUp (golden :4966-4999) --------------------------------------
    TfLotInfoTextEdit *edTemp;                       // golden uLotInfo.h:316 (TEdit*) -- ->SelStart needs the composed type above
    virtual void edTempKeyUp();                       // golden uLotInfo.cpp:4966-4999 (Sender/Key/Shift dropped, unused)

    // -- edtSysLotIDKeyPress / edPageKeyPress (golden :11892-11917 / :11919-11933) --
    // DEVIATION: both golden bodies call ->SetFocus() on the cross-field target
    // (edPage / edtSysOperatorID) -- a pure UI focus-transfer with no
    // observable state anywhere in this facade. vclcompat::TControl carries no
    // SetFocus() (a rendering concern, same scope call as ->Refresh() being a
    // no-op elsewhere in this tree e.g. forms/fTemperFrom.h's
    // TfTemperFromPanel). Dropped at both call sites below rather than
    // widening `edtSysOperatorID`'s pre-existing shared type for one method.
    TEdit *edPage;                                   // golden uLotInfo.h:420 (shared by both)
    virtual void edtSysLotIDKeyPress(char Key);       // golden uLotInfo.cpp:11892-11917 (Sender/Shift dropped, unused; ->SetFocus() dropped, see DEVIATION above)
    virtual void edPageKeyPress(char Key);            // golden uLotInfo.cpp:11919-11933 (Sender/Shift dropped, unused; ->SetFocus() dropped, see DEVIATION above)

    // -- SettsChipAdvVisible (golden :1240-1260) -- tsChipAdv declared above --
    virtual void SettsChipAdvVisible();               // golden uLotInfo.cpp:1240-1260

    // -- bCheckOnlyOneFile (golden :14193-14213) -- zero new members ---------
    virtual bool bCheckOnlyOneFile(AnsiString asPath, AnsiString &asFileName);  // golden uLotInfo.cpp:14193-14213

    // -- CheckActionFlag (golden :16234-16246) -------------------------------
    TfLedValue *ledLoader, *ledEmpty, *ledColor;                    // golden uLotInfo.h (TALed*)
    TfLedValue *ledAuto1, *ledAuto2, *ledAuto3;                     // golden uLotInfo.h (TALed*)
    TfLedValue *ledStartAGV, *ledSTART, *ledLoaderTotalTray, *ledLOT_START;  // golden uLotInfo.h (TALed*)
    virtual void CheckActionFlag();                   // golden uLotInfo.cpp:16234-16246

    // -- SetTesterStartTimeByB03 (golden :14298-14309) -- zero new members ---
    virtual void SetTesterStartTimeByB03();           // golden uLotInfo.cpp:14298-14309

    // -- btnLoadFileClick (golden :14175-14184) ------------------------------
    TfLotInfoOpenDialog *OpenDialog1;                // golden uLotInfo.h:568 (TOpenDialog*)
    TEdit *edSort2DIDBinFile;                        // golden uLotInfo.h:843
    virtual void btnLoadFileClick();                  // golden uLotInfo.cpp:14175-14184 (Sender dropped, unused)

    // -- cbRunModeKeyDown / cbRunModeKeyUp (golden :11990-11997 / :11999-12006) -- WA-7
    virtual void cbRunModeKeyDown();                  // golden uLotInfo.cpp:11990-11997
    virtual void cbRunModeKeyUp();                    // golden uLotInfo.cpp:11999-12006

    // -- edDeviceNameKeyUp (golden :12044-12051) -- zero new members --------
    virtual void edDeviceNameKeyUp();                 // golden uLotInfo.cpp:12044-12051 (Sender/Key/Shift dropped, unused)

    // -- ReflashInfo (golden :16479-16485) -----------------------------------
    TLabel *labAuto1TrayCount_KYEC, *labAuto2TrayCount_KYEC, *labAuto3TrayCount_KYEC;  // golden uLotInfo.h:1072/1074/1076
    virtual void ReflashInfo();                       // golden uLotInfo.cpp:16479-16485

    // -- labLotIDMouseDown (golden :8423-8428) -- WA-8, edtSysLotID exists ---
    virtual void labLotIDMouseDown();                 // golden uLotInfo.cpp:8423-8428 (params dropped, unused)

    // -- edtLotVerifyMouseDown / edtLotVerifyKeyPress (golden :14515-14520 / :14522-14526) --
    TLabeledEdit *edtLotVerify;                      // golden uLotInfo.h:836
    virtual void edtLotVerifyMouseDown();              // golden uLotInfo.cpp:14515-14520 (params dropped, unused)
    virtual void edtLotVerifyKeyPress();               // golden uLotInfo.cpp:14522-14526 (params dropped, unused)

    // -- btnCancelTestPauseClick (golden :12297-12301) -- zero new members --
    virtual void btnCancelTestPauseClick();           // golden uLotInfo.cpp:12297-12301 (Sender dropped, unused)

    // -- BtnPauseMouseDown / BtnPauseMouseUp (golden :14533-14537 / :14539-14543) --
    TfMainSpeedButton *BtnPause;                     // golden uLotInfo.h:837 (TBtnPanel*) -- reuses the existing TBtnPanel stand-in (FormWidgets.h)
    virtual void BtnPauseMouseDown();                  // golden uLotInfo.cpp:14533-14537 (params dropped, unused)
    virtual void BtnPauseMouseUp();                    // golden uLotInfo.cpp:14539-14543 (params dropped, unused)

    // -- VisibleUploadBtnPAT (golden :15710-15714) ---------------------------
    TSpeedButton *sbUploadPAT;                       // golden uLotInfo.h:985
    virtual void VisibleUploadBtnPAT(bool bVisible);  // golden uLotInfo.cpp:15710-15714

    // -- cbbDeviceNameChange (golden :7216-7219) -----------------------------
    TEdit *edDeviceName;                             // golden uLotInfo.h:313
    TComboBox *cbbDeviceName;                        // golden uLotInfo.h:317
    virtual void cbbDeviceNameChange();                // golden uLotInfo.cpp:7216-7219 (Sender dropped, unused)

    // -- btStartCountClick (golden :8418-8421) -------------------------------
    bool bStartCount_SCK;                            // golden uLotInfo.h:1294 (real TfLotInfo member, not a bare global)
    virtual void btStartCountClick();                  // golden uLotInfo.cpp:8418-8421 (Sender dropped, unused)

    // -- edtASECL_LotIDClick (golden :10491-10494) -- WA-8, edtASECL_LotID exists --
    virtual void edtASECL_LotIDClick(TObject *Sender); // golden uLotInfo.cpp:10491-10494 (Sender restored 20260824, FW-QWKEY2)

    // -- btTesterTCPShowClick (golden :13841-13844) -- WA-9, whole body gated --
    virtual void btTesterTCPShowClick();               // golden uLotInfo.cpp:13841-13844 (Sender dropped, unused)

    // -- InitialLoaderTask / InitialLDLevelTask (golden :16259-16262 / :16264-16267) --
    // NOTE: golden's iLoaderTask[3]/iloaderLevelTask[3] are FILE-SCOPE globals
    // (uLotInfo.cpp:16256-16257, `={1,1,1}`), NOT TfLotInfo members -- declared
    // as such in forms/fLotInfo.cpp, mirroring InitialUnLoaderTask's sibling
    // iUnloaderTask[3] being a REAL member only because golden itself declares
    // it in uLotInfo.h:1264 (different from these two).
    virtual void InitialLoaderTask(int iPos);          // golden uLotInfo.cpp:16259-16262
    virtual void InitialLDLevelTask(int iPos);         // golden uLotInfo.cpp:16264-16267

    // =======================================================================
    //  AI(W906-FW3-LotInfo-WB) 20260819: Wave B ADD -- see file banner above
    //  for WAVE SCOPE / GATE REGISTER. Grouped per Tier-2 method, each member
    //  cites the golden uLotInfo.h line it mirrors.
    // =======================================================================

    // -- TfLotInfo ctor (golden :157-299) -- WB-19 gates the named-widget-----
    // array wiring block; these members back the REAL portion only.
    bool bShow;                                    // golden uLotInfo.h:1286
    TLabel *lblOCR_LotID;                           // golden uLotInfo.h:283
    int iLotRead, iLotStart, iLotEnd;               // golden uLotInfo.h:1306-1308
    bool bStartChamberBoost;                        // golden uLotInfo.h:1347
    int  iXMLOnLineStatus;                          // golden uLotInfo.h:1352
    bool bRTCChangeFileFinish;                      // golden uLotInfo.h:1289
    AnsiString sVTestInternalLot;                   // golden uLotInfo.h:1382
    TStringList *slASECLTestInfor;                  // golden uLotInfo.h:1331 (TStringList*)
    TfLotInfoTimer *TimerERMS;                      // golden uLotInfo.h:174 (TTimer*) -- ->Enabled only, reuses the WA TfLotInfoTimer shape
    TTabSheet *tsChamberBoost;                      // golden uLotInfo.h:422
    TButton *btnCancelTestPause;                    // golden uLotInfo.h:435 -- ->Enabled only
    TButton *btnESCFunction;                        // golden uLotInfo.h:434 -- ->Enabled only (also written by cprod.cpp:2978, currently unreachable there -- see WB banner)
    TTabSheet *ts_FTPAutomation;                    // golden uLotInfo.h:437 -- ->Caption only
    TTabSheet *ATC_WinWay;                          // golden uLotInfo.h:555 -- ->TabVisible only
    TPanel *ATCPtrWinWay[4];                        // golden uLotInfo.h:1260
    TPanel *pnlWinwayPVCH1, *pnlWinwayPVCH2, *pnlWinwayPVCH3, *pnlWinwayPVCH4;  // golden uLotInfo.h:564-567
    bool bNeedToDeleteFile;                         // golden uLotInfo.h:1259
    int  iWaitRtcDeleteTask;                        // golden uLotInfo.h:1283
    bool bEventLogAlarm;                            // golden uLotInfo.h:1325
    double fTempUserOffset[tcTotalCount];           // golden uLotInfo.h:1334
    double fContactHeight[25];                      // golden uLotInfo.h:1335 -- also read/written by pgLotinfoChange/edDeviceNameMouseDown below
    double fTempATCOffset[tcTotalCount];            // golden uLotInfo.h:1336
    int iShuttleMode[2];                            // golden uLotInfo.h:1337
    bool bART[3];                                   // golden uLotInfo.h:1339
    int iART;                                       // golden uLotInfo.h:1340
    int iIndexHeatingMode;                          // golden uLotInfo.h:1341
    int iProduceTimeCT;                             // golden uLotInfo.h:1261

    // -- pgLotinfoChange (golden :7221-7266) -- 20 TCheckBox widgets ---------
    TCheckBox *chkTempOffset, *chkContactHigh, *chkContactForce, *chkContactMode;
    TCheckBox *chkHotPlate, *chkLoadUnload, *chkSpeedSetting, *chkShuttleMode;
    TCheckBox *chkTestMode, *chkBinasgn, *chkBinasgnOff, *checkbAutoClean;
    TCheckBox *chkAutoCleanContactHeight, *cbBottom2DOffset, *chkART, *chkART_RTCount;
    TCheckBox *chkIndexHeatingMode, *chkStopYield, *chkConsecutiveFailure, *chkCleanCount;
    virtual void pgLotinfoChange();                  // golden uLotInfo.cpp:7221-7266

    // -- FormDestroy / FormClose (golden :301-314 / :1235-1238) -- zero new --
    // members beyond bShow/slASECLTestInfor/TimerERMS (all declared above).
    virtual void FormDestroy();                      // golden uLotInfo.cpp:301-314 (Sender dropped, unused)
    virtual void FormClose();                        // golden uLotInfo.cpp:1235-1238 (params dropped, unused)

    // -- LoadRTCFullViewImg (golden :5366-5448) -- WB-1, whole body gated ----
    // (TImage/TCanvas have no port; no widgets declared, see GATE REGISTER).
    virtual void LoadRTCFullViewImg(bool bShowImage);

    // -- edDeviceNameMouseDown (golden :7193-7214) -- WB-2, whole body gated -
    // edDeviceName already exists (cbbDeviceNameChange, above).
    // AI(W906-FW-SIG-W15) 20260826: 回填 golden 完整簽章（GATE (WB-2-BTN) 收窄成 GATE (CLIP)）。
    virtual void edDeviceNameMouseDown(TObject *Sender,
          TMouseButton Button, TShiftState Shift, int X, int Y);   // golden uLotInfo.cpp:7193-7214

    // -- CutTempToEdit (golden :5225-5244) -- zero new members (edTemp exists) --
    virtual void CutTempToEdit(AnsiString asString);  // golden uLotInfo.cpp:5225-5244

    // -- btChangeFileClick (golden :10213-10237) -- WB-3, whole body gated --
    virtual void btChangeFileClick();                 // golden uLotInfo.cpp:10213-10237 (Sender dropped, unused)

    // -- edtSysOperatorIDKeyUp / edtSysLotIDKeyUp (golden :10287-10312 / :10314-10344) --
    // WB-4/WB-5 gate the InputBarcodeNumber branch of each; edtSysOperatorID/
    // edtSysLotID already exist.
    virtual void edtSysOperatorIDKeyUp();             // golden uLotInfo.cpp:10287-10312 (params dropped, unused)
    virtual void edtSysLotIDKeyUp();                  // golden uLotInfo.cpp:10314-10344 (params dropped, unused)

    // -- spOCRChangeFileClick / Timer3Timer (golden :8458-8474 / :8476-8485) --
    TLabel *lbOCRUseFile;                             // golden uLotInfo.h:188
    TfLotInfoTimer *Timer3;                           // golden uLotInfo.h:79 (TTimer*) -- ->Enabled only
    TSpeedButton *spOCRChangeFile;                    // golden uLotInfo.h -- ->Down only
    virtual void spOCRChangeFileClick();              // golden uLotInfo.cpp:8458-8474 (Sender dropped, unused)
    virtual void Timer3Timer();                       // golden uLotInfo.cpp:8476-8485 -- WB-6 gates fOCR->OCRChangeFile()

    // -- edDeviceNameKeyDown (golden :10457-10473) -- WB-7 gates 1st branch --
    virtual void edDeviceNameKeyDown();               // golden uLotInfo.cpp:10457-10473 (params dropped, unused)

    // -- SetCheckCodeByLot (golden :12311-12324) -----------------------------
    TPanel *lbCheckCodeByLot;                         // golden uLotInfo.h:436 (TPanel*, not TLabel -- ->Caption/->Color)
    virtual void SetCheckCodeByLot(bool _enable);     // golden uLotInfo.cpp:12311-12324

    // -- edTempKeyPress / edTempKeyDown (golden :12032-12042 / :12053-12063) --
    // zero new members (edTemp exists; bTemp_OK is a file-scope global, see .cpp).
    virtual void edTempKeyPress(char Key);            // golden uLotInfo.cpp:12032-12042 (Sender dropped, unused)
    virtual void edTempKeyDown();                     // golden uLotInfo.cpp:12053-12063 (Sender/Key/Shift all dropped -- golden body never reads Key, verified this wave)

    // -- btnSaveDataClick (golden :12285-12295) -- zero new members ---------
    virtual void btnSaveDataClick();                  // golden uLotInfo.cpp:12285-12295 (Sender dropped, unused)

    // -- edPageMouseDown (golden :11534-11541) -- WB-8, non-Murata branch gated --
    virtual void edPageMouseDown(TObject *Sender);    // golden uLotInfo.cpp:11534-11541 (Sender restored 20260824, FW-QWKEY2; other params still dropped)

    // -- edtSysLotIDKeyDown / edtSysOperatorIDKeyDown (golden :12065-12080 / :12082-12097) --
    // zero new members (bLotID_OK/bOPID_OK are file-scope globals, see .cpp).
    virtual void edtSysLotIDKeyDown();                // golden uLotInfo.cpp:12065-12080 (params dropped, unused)
    virtual void edtSysOperatorIDKeyDown();           // golden uLotInfo.cpp:12082-12097 (params dropped, unused)

    // -- edtSysOperatorIDMouseUp / edtSysLotIDMouseUp (golden :11755-11794 / :11796-11831) --
    // WB-9/WB-10 gate everything past the CC_Murata branch; zero new members.
    virtual void edtSysOperatorIDMouseUp(TObject *Sender); // golden uLotInfo.cpp:11755-11794 (Sender restored 20260825, FW-BARCODE4; other params still dropped)
    virtual void edtSysLotIDMouseUp(TObject *Sender); // golden uLotInfo.cpp:11796-11831 (Sender restored 20260825, FW-BARCODE4; other params still dropped)

    // -- edQAModeMouseDown (golden :11528-11532) -- WB-11, whole body gated --
    virtual void edQAModeMouseDown(TObject *Sender);  // golden uLotInfo.cpp:11528-11532 (Sender restored 20260824, FW-QWKEY2; other params still dropped)

    // -- edtSysOperatorIDKeyPress (golden :11935-11963) -- zero new members --
    // (cbRunMode/edtSysOperatorID exist; ->SetFocus() dropped, DEVIATION as
    // established by edtSysLotIDKeyPress/edPageKeyPress in Wave A).
    virtual void edtSysOperatorIDKeyPress(char Key);  // golden uLotInfo.cpp:11935-11963 (Sender dropped, unused; ->SetFocus() dropped)

    // -- cbRunModeKeyPress (golden :11965-11988) -- WB-12 gates the JCET check --
    TSpeedButton *sbSECSLotStart, *sbSECSLotEnd;      // golden uLotInfo.h -- ->Click()/->Down only
    virtual void cbRunModeKeyPress(char Key);         // golden uLotInfo.cpp:11965-11988 (Sender dropped, unused)

    // -- cbRunModeDropDown (golden :15928-15951) -- WB-13, whole body gated --
    virtual void cbRunModeDropDown();                 // golden uLotInfo.cpp:15928-15951 (Sender dropped, unused once gated)

    // -- GetFTP_SettingN06 (golden :15449-15460) -- zero new members --------
    virtual void GetFTP_SettingN06(AnsiString &asUserID, AnsiString &asPassword, AnsiString &asHost);  // golden uLotInfo.cpp:15449-15460

    // -- CheckNoRetestBinFlag (golden :14608-14630) -- WB-14, whole body gated (returns true) --
    virtual bool CheckNoRetestBinFlag();              // golden uLotInfo.cpp:14608-14630

    // -- edStationNumMouseDown (golden :14060-14064) -- WB-15, whole body gated --
    virtual void edStationNumMouseDown(TObject *Sender); // golden uLotInfo.cpp:14060-14064 (Sender restored 20260824, FW-QWKEY2; other params still dropped)

    // -- ATC_OFFLINE_FormComInit (golden :14969-14992) -- 2 new TLabel; -----
    // reuses Wave A's InitialRefrigerantSystem TriLab*/Tripnl* arrays and
    // bInitFormcomponent/OldRefrigerantCommand members.
    TLabel *labRefrigerantMachineHighLimit;           // golden uLotInfo.h:689
    TLabel *labRefrigerantMachineLowLimit;            // golden uLotInfo.h:688
    virtual void ATC_OFFLINE_FormComInit();           // golden uLotInfo.cpp:14969-14992

    // -- ScanRefrigerantSystem / RefreshRefrigerantAllStatus (golden :14941-14967 / :14994-15078) --
    // WB-16/WB-17 gate everything past the AirStream_Select==0 early return;
    // zero new members (ATC_InterfaceForm's missing members are the whole point).
    virtual void ScanRefrigerantSystem();             // golden uLotInfo.cpp:14941-14967
    virtual void RefreshRefrigerantAllStatus();       // golden uLotInfo.cpp:14994-15078

    // -- leRunCardNumberMouseDown (golden :15780-15784) ----------------------
    TLabeledEdit *leRunCardNumber;                    // golden uLotInfo.h:961
    virtual void leRunCardNumberMouseDown();          // golden uLotInfo.cpp:15780-15784 (params dropped, unused)

    // -- RefreshAMR / ShowAMRCategoryBin (golden :15816-15844 / :15846-15881) --
    // RefreshAMR already declared above (W5-Automation ADD) as a no-op stub;
    // this wave gives it (and the ShowAMRCategoryBin it calls) a REAL body --
    // see forms/fLotInfo.cpp. 15 new members below.
    TCheckBox *cbA60_1;                               // golden uLotInfo.h:997
    TPanel *pnlWaitTXSetLoader, *pnlWaitRXSetAuto1, *pnlWaitRXSetAuto2, *pnlWaitRXSetAuto3;  // golden uLotInfo.h:1002/1005/1006/1009
    TPanel *pnlWaitTXTotalLoader, *pnlWaitTXCntLoader;                                        // golden uLotInfo.h:999/1003
    TPanel *pnlWaitRXCntAuto1, *pnlWaitRXCntAuto2, *pnlWaitRXCntAuto3;                        // golden uLotInfo.h:1004/1007/1008
    TfLedValue *aldWaitTXLoader;                      // golden uLotInfo.h:1021 (TALed*)
    TfLedValue *aldWaitRXAuto1, *aldWaitRXAuto2, *aldWaitRXAuto3;                             // golden uLotInfo.h:1020/1019/1018 (TALed*)
    TfLedValue *aldWaitTrayFeed;                      // golden uLotInfo.h:1017 (TALed*)
    TfLedValue *aldLoaderLast;                        // golden uLotInfo.h:1016 (TALed*)
    TStringGrid *StrGrdCategory;                      // golden uLotInfo.h:1023
    virtual void ShowAMRCategoryBin(bool bInitial=false);  // golden uLotInfo.cpp:15846-15881

    // -- cbFirstTrayCheckOnUnloaderMouseDown (golden :16029-16034) -----------
    bool bP60UserClicked;                             // golden uLotInfo.h:1262
    virtual void cbFirstTrayCheckOnUnloaderMouseDown();  // golden uLotInfo.cpp:16029-16034 (params dropped, unused)

    // -- RefreshOtherTool (golden :16047-16072) ------------------------------
    TEdit *edLoaderCountNow, *edLoaderCountAlarm;     // golden uLotInfo.h
    virtual void RefreshOtherTool();                  // golden uLotInfo.cpp:16047-16072

    // -- labNowTrayCountClick (golden :16186-16190) -- zero new members -----
    // (labNowTrayCount exists, W7-L1-Wave0 ADD).
    virtual void labNowTrayCountClick();              // golden uLotInfo.cpp:16186-16190 (Sender dropped, unused)

    // -- Timer4Timer (golden :16487-16493) -- WB-18 gates CheckAMRAction() --
    virtual void Timer4Timer();                       // golden uLotInfo.cpp:16487-16493

    // =======================================================================
    //  AI(W906-FW3-LotInfo-WC) 20260819: Wave C ADD -- see file banner above
    //  for STEP 0 / WAVE SCOPE / GATE REGISTER. Grouped per FormShow (S#)/
    //  Timer2Timer (T#) segment per docs/RECON_uLotInfo_mixed_split.md.
    // =======================================================================

    // -- FormShow S1/S7/S12/S24 -- TTabSheet members, ->TabVisible only -----
    TTabSheet *tsPATSetUp;                            // golden uLotInfo.h:960
    TTabSheet *tsBundle;                              // golden uLotInfo.h:797
    TTabSheet *tsSetupFileCheck;                      // golden uLotInfo.h:986
    TTabSheet *tsESDMonitor;                          // golden uLotInfo.h:81
    TTabSheet *tsBarCode;                             // golden uLotInfo.h:37
    TTabSheet *ts_OCRInterface;                       // golden uLotInfo.h:34
    TTabSheet *ts_SocketInterface;                    // golden uLotInfo.h:36
    TTabSheet *tsKYEC_AMR;                            // golden uLotInfo.h:1048
    TTabSheet *tsRTCFullViewImg;                      // golden uLotInfo.h:29

    // -- FormShow S9 -- grpRFID (TGroupBox) + SPILFunction branch widgets ---
    TGroupBox *grpRFID;                               // golden uLotInfo.h:471
    TTabSheet *ts_AutoCleanMonitor;                   // golden uLotInfo.h:41
    TLabel    *Label5;                                // golden uLotInfo.h:311
    TButton   *btDownload;                            // golden uLotInfo.h:314
    TButton   *btnFtpTester;                          // golden uLotInfo.h:291
    TLabel    *Label153;                              // golden uLotInfo.h:310
    vclcompat::TFileListBox *FileListBox1;            // golden uLotInfo.h:318 -- WC-6 gates ->Visible only, ->Directory REAL
    TCheckBox *cbRTCASTD;                             // golden uLotInfo.h:634

    // -- FormShow S10 -- ASE_KaohSiung/else ART/GroupBox swap ---------------
    TfLotInfoLayoutPanel     *ART_Panel;               // golden uLotInfo.h:59 (TPanel*) -- Left/Top layout
    TfLotInfoLayoutGroupBox  *GroupBox3;                // golden uLotInfo.h:191 (TGroupBox*) -- Left/Top layout

    // -- FormShow S13 -- CC_TSMC_TAINAN vs else --------------------------
    TButton      *btnDataFTPSaveToData;               // golden uLotInfo.h:293
    TLabel       *lblOPID;                            // golden uLotInfo.h:304
    TSpeedButton *spSECSLotCheck;                     // golden uLotInfo.h:301
    TfLotInfoLayoutPanel *Panel6;                      // golden uLotInfo.h:173 (TPanel*) -- Top/Left layout

    // -- FormShow S14 -- tsTPW/tsSigurd (RefreshYieldMonitor itself WC-8) ---
    TTabSheet *tsTPW;                                 // golden uLotInfo.h:528
    TTabSheet *tsSigurd;                              // golden uLotInfo.h:477

    // -- FormShow S15 -- coLevelMode (WC-9 gates mtBarcodeSetDefaultView) ---
    TLabel    *labLevelMode;                          // golden uLotInfo.h:312
    TComboBox *coLevelMode;                           // golden uLotInfo.h:320

    // -- FormShow S16 -- 3 StringGrids, dims from golden uLotInfo.dfm -------
    TStringGrid *sgBarcode;                           // golden uLotInfo.h:264 -- dfm ColCount=6 RowCount=7
    TStringGrid *sgOCR;                               // golden uLotInfo.h:190 -- dfm ColCount=3, RowCount default(5)
    TStringGrid *sgATRCount;                          // golden uLotInfo.h:172 -- dfm ColCount=3 RowCount=4

    // -- FormShow S17 -- pl_ATC_Online (TPanel, ->Color/->Caption) ----------
    TPanel *pl_ATC_Online;                            // golden uLotInfo.h -- (TPanel*)

    // -- FormShow S18 -- ONSEMI_M/SIGURD_ChungXing/bOEEFunction -------------
    // (JSCC_OS branch wholly gated, WC-21 -- no new members needed for it)
    TLabel    *labDeviceName;                         // golden uLotInfo.h:622
    TLabel    *lbLotRunMode;                          // golden uLotInfo.h:305
    TLabel    *labLotID;                              // golden uLotInfo.h:298
    TButton   *btnFtpServer;                          // golden uLotInfo.h:289
    TButton   *btnFtpHD;                              // golden uLotInfo.h:290
    TSpeedButton *sb_RunExecutFile;                   // golden uLotInfo.h:603

    // -- FormShow S19 -- Murata/VTest mega-branch ---------------------------
    TLabeledEdit *edtLine;                            // golden uLotInfo.h:584 -- ->Visible only, no geometry needed
    TfLotInfoLayoutLabeledEdit *edtProcessName;        // golden uLotInfo.h:585 -- Left/Top layout, WC-13 gates ->Parent=
    TfLotInfoLayoutLabeledEdit *edtProduct;            // golden uLotInfo.h:586 -- Left/Top layout, WC-13 gates ->Parent=
    TLabel    *lbProcess;                             // golden uLotInfo.h:469
    TLabel    *labConfigL04;                           // golden uLotInfo.h:787
    TfLotInfoLayoutGroupBox *grpOEEState;               // golden uLotInfo.h:793 (TGroupBox*) -- Left layout
    TStringGrid *sgOEEState;                          // golden uLotInfo.h:795 -- dfm ColCount=2 RowCount=10
    TLabel    *labCusDevGrp;                          // golden uLotInfo.h:621
    TLabel    *labCusStep;                            // golden uLotInfo.h:1037

    // -- FormShow S20 -- CC_GIGAS FTP panel ----------------------------------
    TButton *btnFTPTryConnect;                        // golden uLotInfo.h:429
    TLabel  *lbFTPStatus;                             // golden uLotInfo.h:430

    // -- FormShow S21 -- ASEMARMS/ASECL panels ------------------------------
    TTabSheet *tsASEMARMS;                            // golden uLotInfo.h:223
    TPanel    *pnlLotInfo_ASECL;                      // golden uLotInfo.h:569
    TPanel    *pnlLotStart_ASECL;                     // golden uLotInfo.h:240

    // -- FormShow S25 -- SPIL EQC-mode / KYEC_LEE QA Mode -------------------
    TLabel    *lblPage;                               // golden uLotInfo.h:419
    TLabel    *labJobSeq;                             // golden uLotInfo.h:595
    TEdit     *edtJobSeq;                             // golden uLotInfo.h:596
    TLabel    *labQACount;                            // golden uLotInfo.h:413 -- WC-23 gates ->Font->Size=8
    TfLotInfoLayoutEdit       *edQAMode;                // golden uLotInfo.h:412 (TEdit*) -- Left layout
    TfLotInfoLayoutPushButton *btnQAmodeSave;           // golden uLotInfo.h:411 (TButton*) -- Left layout
    TPanel    *palQAMode;                             // golden uLotInfo.h:410

    // -- FormShow S27 -- DewPoint_Hardware_Install (confirmed REAL) --------
    TPanel *pan_DewPoint;                             // golden uLotInfo.h:857
    TPanel *pl_DewPoint;                              // golden uLotInfo.h:858

    // -- FormShow S29 -- 2DID format group -----------------------------------
    TGroupBox *grpBarcodeDisplayLotInfo;              // golden uLotInfo.h:414

    // -- FormShow S1/S12 -- tsBarCode's paired btChangeFile ------------------
    TButton *btChangeFile;                            // golden uLotInfo.h:267

    // -- FormShow S33 -- ASE_SG/LEADYO --------------------------------------
    TButton *btnClearTemperature;                     // golden uLotInfo.h:853
    // -- FormShow S33/S34 boundary (golden :1220-1221, connective content --
    // recon's own S33/S34 segment table doesn't cover these 2 real lines;
    // see docs/RECON_uLotInfo_mixed_split.md section 0's own disclosure that
    // segment-boundary connective lines aren't all individually enumerated).
    TLabel *lbShowDevName;                            // golden uLotInfo.h:964

    // -- FormShow S34 -- AQL bin/count labels (ReadWriteFTPAutomationData GATE WC-16) --
    TLabel *lbLotAQLSetCount;                         // golden uLotInfo.h:1114
    TLabel *lbLotAQLSetBin;                           // golden uLotInfo.h:1115

    // -- FormShow S3/S4/S6 -- MTI/bShowLotInfo/PANTHER/SECS panel -----------
    TTabSheet *tsDeviceInfo;                          // golden uLotInfo.h:26
    TPanel    *palSecsGem;                            // golden uLotInfo.h:297
    TComboBox *cbPATMode;                             // golden uLotInfo.h:962

    // -- FormShow S7 -- KYEC_LEE/AMD_M (WC-4 gates the PopupMenu lines) -----
    TPanel *palCurrFailRate;                          // golden uLotInfo.h:210
    TPanel *Panel27;                                  // golden uLotInfo.h:295
    TfLotInfoTimer *LotKeyInTime;                     // golden uLotInfo.h:417 (TTimer*) -- ->Enabled only

    // -- FormShow S4 -- CC_TSI branch --
    TSpeedButton *btnSaveData;                        // golden uLotInfo.h:431

    virtual void FormShow();                          // golden uLotInfo.cpp:316-1233 (Sender dropped, unused)

    // -- Timer2Timer T4/T6/T7/T16 --------------------------------------------
    TLabel *labLoaderBundleID;                        // golden uLotInfo.h:830
    TLabel *lblLoaderCarBundleID;                     // golden uLotInfo.h:832
    TLabel *lbOCRNowFile;                             // golden uLotInfo.h:187
    TLabel *Label41;                                  // golden uLotInfo.h:179
    TPanel *palHandlerwithTester;                     // golden uLotInfo.h:286 -- ->Caption/->Color
    TLabel *lblTester_LotID;                          // golden uLotInfo.h:281 -- WC-26/WC-27 gate both assignments
    TLabel *lblAutoCount;                             // golden uLotInfo.h:833
    TLabel *lblAutoCount2;                            // golden uLotInfo.h:834
    TLabel *lblAutoCount3;                            // golden uLotInfo.h:835
    TLabel *labBarcodeRecipe;                         // golden uLotInfo.h:1117
    TEdit  *edtBarcodeRecipe;                         // golden uLotInfo.h:1118
    TSpeedButton *spOCRCleanList;                     // golden uLotInfo.h:636
    TTabSheet *tsTesterLog;                           // golden uLotInfo.h:463
    TTabSheet *ts_AutoRetestMonitor;                  // golden uLotInfo.h:57
    TTabSheet *tsOtherTool;                           // golden uLotInfo.h:1024
    TPanel *palAQLMode;                               // golden uLotInfo.h:1111

    virtual void Timer2Timer();                       // golden uLotInfo.cpp:6934-7191


    // =======================================================================
    //  AI(W906-FW-LOTINFO-W27) 20260826: Wave D ADD -- see this file's W27
    //  banner above for the batch criterion, the GATE REGISTER (WD-1..WD-5),
    //  the re-measured denominator, the re-checked WA/WB/WC premises and the
    //  full EXIT REGISTER.  Every golden uLotInfo.h line cited below was read
    //  from the cp950 decode this wave.
    //
    //  ALLOCATION: every new pointer below uses an NSDMI (`= new T()`), NOT a
    //  line added to the ctor.  Two reasons.  (a) The wave is append-only and
    //  the ctor is existing lines.  (b) It is what campaign trap 4 asks for
    //  anyway -- an NSDMI is a pure field fill: no ini read, no file, no
    //  machine access, no dependency on any other global's construction
    //  order, so it cannot reproduce the fLaserSensor ctor SEGFAULT shape.
    //  The idiom is already established in this tree (forms/fPassword.h:317
    //  and :326 do exactly this) and in this very object (the existing ctor
    //  already `new`s TfLotInfoRunMode/TfLotInfoLabel/... at the same point
    //  in construction).
    // =======================================================================

    // -- RefreshYieldMonitor / _TERAPOWER (golden :13310-13325, :13547-13623) -
    TCheckBox    *cbLowYield                   = new TCheckBox();     // golden uLotInfo.h:530
    TEdit        *edLowYieldIg                 = new TEdit();         // golden uLotInfo.h:531
    TEdit        *edLowYield                   = new TEdit();         // golden uLotInfo.h:532
    TRadioButton *rbContsFailByHead_On         = new TRadioButton();  // golden uLotInfo.h:535
    TRadioButton *rbContsFailByHead_Off        = new TRadioButton();  // golden uLotInfo.h:536
    TEdit        *edContsFailHeadAlarmCT       = new TEdit();         // golden uLotInfo.h:537
    TRadioButton *rbContsFailBySocket_On       = new TRadioButton();  // golden uLotInfo.h:540
    TRadioButton *rbContsFailBySocket_Off      = new TRadioButton();  // golden uLotInfo.h:541
    TEdit        *edContsFailSocketAlarmCT     = new TEdit();         // golden uLotInfo.h:542
    TCheckBox    *cb_HeadToHeadYieldEnable     = new TCheckBox();     // golden uLotInfo.h:544
    TEdit        *ed_HeadToHeadYield           = new TEdit();         // golden uLotInfo.h:545
    TEdit        *ed_HeadToHeadYieldCount      = new TEdit();         // golden uLotInfo.h:546
    TCheckBox    *cbAllSiteFail                = new TCheckBox();     // golden uLotInfo.h:549
    TEdit        *edAllSiteFailCount           = new TEdit();         // golden uLotInfo.h:550
    TCheckBox    *cb_SiteToSiteYieldEnable     = new TCheckBox();     // golden uLotInfo.h:552
    TEdit        *ed_SiteToSiteYield           = new TEdit();         // golden uLotInfo.h:553
    TEdit        *ed_SiteToSiteYieldCount      = new TEdit();         // golden uLotInfo.h:554
    TLabel       *lbl_SmartAutoCleanCount      = new TLabel();        // golden uLotInfo.h:627
    TLabel       *lblAdaptiveIntervalCount     = new TLabel();        // golden uLotInfo.h:628
    TLabel       *lbl_SmartAutoCleanCount_CTF  = new TLabel();        // golden uLotInfo.h:630
    TCheckBox    *chk_SmartAutoClean           = new TCheckBox();     // golden uLotInfo.h:631
    TEdit        *edt_SmartAutoClean           = new TEdit();         // golden uLotInfo.h:632
    TEdit        *ed_SmartAutoCleanCTF         = new TEdit();         // golden uLotInfo.h:633
    virtual void RefreshYieldMonitor();            // golden uLotInfo.cpp:13310-13325
    virtual void RefreshYieldMonitor_TERAPOWER();  // golden uLotInfo.cpp:13547-13623 (WD-2 gates :13618)

    // -- SetLotComponents (golden :2283-2320) -- WD-1 gates :2285 only -------
    // Every widget below is touched through ->Enabled only (which
    // vclcompat::TControl carries), except pnlLoader (->Caption).  The
    // already-declared ones -- edtSysLotID, sbSECSLotStart/End, edPage,
    // edtSysOperatorID, edtJobSeq, edtBarcodeRecipe, cbRunMode, edtCusLotID,
    // edtCusDevGrp, edtDevice, lbledtCustomer -- are NOT redeclared here (a
    // shadowing redeclaration is exactly the silent-wrong-read hazard
    // vclcompat/Controls.h's own Tag note documents).
    TPanel       *pnlLoader          = new TPanel();        // golden uLotInfo.h:472 -- also RFID_ReaderReceiveData
    TEdit        *edCustomerLotId    = new TEdit();         // golden uLotInfo.h:589
    TComboBox    *coStation          = new TComboBox();     // golden uLotInfo.h:590
    TEdit        *edStationNum       = new TEdit();         // golden uLotInfo.h:592 -- edStationNumMouseDown (WB-15) never needed it
    TLabeledEdit *lbledtStarTime     = new TLabeledEdit();  // golden uLotInfo.h:606
    TLabeledEdit *lbledtEndTime      = new TLabeledEdit();  // golden uLotInfo.h:607
    TLabeledEdit *lbledtTesterOsVer  = new TLabeledEdit();  // golden uLotInfo.h:608
    TLabeledEdit *lbledtTestProg     = new TLabeledEdit();  // golden uLotInfo.h:610
    TLabeledEdit *lbledtDeviceName   = new TLabeledEdit();  // golden uLotInfo.h:611
    TLabeledEdit *lbledtTesterID     = new TLabeledEdit();  // golden uLotInfo.h:612
    TLabeledEdit *lbledtSubLotNo     = new TLabeledEdit();  // golden uLotInfo.h:613
    TLabeledEdit *lbledtTestCode     = new TLabeledEdit();  // golden uLotInfo.h:614
    //AI(W906-LOT-W1) 20260919: golden :615。之前這一列被跳過（序列是
    // :614 -> :616），ReadWriteLotInfo（golden :1404-1490）要它。照 golden
    // 的型別與位置補回來。
    TLabeledEdit *lbledtMachineID    = new TLabeledEdit();  // golden uLotInfo.h:615
    TLabeledEdit *lbledtTestBinNo    = new TLabeledEdit();  // golden uLotInfo.h:616
    TLabeledEdit *lbledtModeCode     = new TLabeledEdit();  // golden uLotInfo.h:617
    //AI(W906-LOT-W1) 20260919: golden :989，同上（之前跳過，ReadWriteLotInfo 要它）。
    TLabeledEdit *edtLotEventLogName = new TLabeledEdit();  // golden uLotInfo.h:989
    //AI(W906-LOT-W1) 20260919: SetLotID（golden :1492-1570）的 ASE-CL 分支要的 4 個。
    // 它們全部只在 `if(IniConfig.bN22Enable_EventLog)` 裡用到（ASE-CL 的 event log
    // 功能）。照 golden 的型別與行號補，不是發明。
    TEdit        *edtASECL_OPID      = new TEdit();         // golden uLotInfo.h:577
    TEdit        *edtASECL_TesterID  = new TEdit();         // golden uLotInfo.h:578
    TEdit        *edtASECL_LoadBoard = new TEdit();         // golden uLotInfo.h:580
    TComboBox    *cbRunModeASECL     = new TComboBox();     // golden uLotInfo.h:1043
    //AI(W906-LOT-W1) 20260919: SetLotStart 的 OEE 分支要的 2 個（golden :600/:601）。
    TLabel       *lb_PIOEELotStatus  = new TLabel();        // golden uLotInfo.h:600
    TEdit        *ed_PIOEEMO         = new TEdit();         // golden uLotInfo.h:601
    TLabeledEdit *edtStage           = new TLabeledEdit();  // golden uLotInfo.h:992
    TLabeledEdit *edtStep            = new TLabeledEdit();  // golden uLotInfo.h:993
    virtual void SetLotComponents(bool bLotEnd);   // golden uLotInfo.cpp:2283-2320
    //AI(W906-LOT-W1) 20260919: golden uLotInfo.cpp:1404-1490。
    // 工單資訊的讀寫端，`SetLotStart` / `SetLotEnd` 都靠它。
    // 本體在 forms/fLotInfo.cpp 尾端（append-only，見那裡的波次橫幅）。
    virtual void ReadWriteLotInfo(bool bRead);  virtual void UploadEventLogFile(AnsiString aFileName);  void SaveASECLTestLogInfo();   // golden uLotInfo.cpp:1404-1490; UploadEventLogFile golden uLotInfo.h:1318 / .cpp:10829-10838 (A5)   //AI(W906-A-OEE-ASECL) 20260929 (St02-E, claim): SaveASECLTestLogInfo golden 906_0625_Steven uLotInfo.cpp:10840-10967 (V912 :11021-); body at LogObjects.cpp end (St02)

    // -- LotKeyInTimeTimer (golden :11543-11660) -- WD-3 gates :11584-11637 --
    // Needs no new widget: edtSysLotID/edtSysOperatorID/edDeviceName/edTemp
    // are all Wave A/B/C members already.  Sender dropped (unused in golden).
    // This handler carries no TMouseButton/TShiftState at all, so the
    // 20260826 signature-policy change (vclcompat/ShiftState.h) does not
    // apply to it -- nothing is lost by the dropped Sender.
    virtual void LotKeyInTimeTimer();              // golden uLotInfo.cpp:11543-11660

    // -- palSecsGemMouseDown (golden :10351-10425) -- WD-4 (withdrawn) -------
    // FULL golden signature: TMouseButton/TShiftState became available
    // 20260826 (vclcompat/ShiftState.h, already included at this file's :17),
    // which is what killed the gate this method was going to need.
    // No new widget: palSecsGem (the panel this hangs off) and
    // edtSysLotID/edtSysOperatorID are all already declared above.
    virtual void palSecsGemMouseDown(TObject *Sender, TMouseButton Button,
                                     TShiftState Shift, int X, int Y);   // golden uLotInfo.cpp:10351-10425

    // -- ShowXMLOnLine (golden :12099-12137) -- WD-5 latency note ------------
    TPanel *pnlXMLOnLine = new TPanel();           // golden uLotInfo.h:587 -- ->Caption/->Color/->Visible
    virtual void ShowXMLOnLine();                  // golden uLotInfo.cpp:12099-12137

    // -- sbTestClick (golden :13719-13752) ----------------------------------
    // gbFTPAutomation_Download and ShowInformation(bool) are already declared
    // above (Wave A/C); fSecurity/fPassword/fQwertyKey are real facades.
    virtual void sbTestClick(TObject *Sender);     // golden uLotInfo.cpp:13719-13752

    // -- First-Tray-Check-On-Unloader pair (golden :15994-16027, :16036-16045)
    // cbFirstTrayCheckOnUnloader and bP60UserClicked already exist (Wave B).
    TCheckBox *cb1stCheck_Auto1 = new TCheckBox(); // golden uLotInfo.h:1027
    TCheckBox *cb1stCheck_Auto2 = new TCheckBox(); // golden uLotInfo.h:1028
    TCheckBox *cb1stCheck_Auto3 = new TCheckBox(); // golden uLotInfo.h:1029
    virtual void cbFirstTrayCheckOnUnloaderClick(TObject *Sender);  // golden uLotInfo.cpp:15994-16027
    virtual void SetFirstTrayCheckOnUnloader();    // golden uLotInfo.cpp:16036-16045

    // -- btnAirStreamOnOffClick (golden :14461-14488) -----------------------
    // golden's ENTIRE body is inside one block comment; the function really is
    // empty.  btnAirStreamOnOff itself is deliberately NOT declared: nothing
    // that compiles touches it (ReadAirMachineStatus, its only other consumer,
    // is EXITED -- header EXIT REGISTER section C), and adding unused surface
    // is the call WA-4/WA-5/WA-6 above already declined to make.
    virtual void btnAirStreamOnOffClick(TObject *Sender);   // golden uLotInfo.cpp:14461-14488

    // -- UpdatePATSubMode (golden :15716-15743) -----------------------------
    // The one PAT-family method that needs no fMain->patFunc (header EXIT D).
    TComboBox *cbPATSubMode = new TComboBox();     // golden uLotInfo.h:983
    virtual void UpdatePATSubMode(const AnsiString& sModeName);  // golden uLotInfo.cpp:15716-15743

    // -- RFID_ReaderReceiveData (golden :14152-14173) -- DEVIATION D-1 ------
    TMemo *mmRFID = new TMemo();                   // golden uLotInfo.h:474 -- ->Lines->Add only
    virtual void RFID_ReaderReceiveData(TObject *Sender, void *Buffer,
                                        unsigned short BufferLength);  // golden uLotInfo.cpp:14152-14173

    // -- sb_Main_EvenLevelLoginClick (golden :10475-10489) ------------------
    // cbbASECL_LoginMode ALSO unblocks two pre-existing gates that live
    // outside this file's write boundary: cMyDB.cpp:1929 and :2037 are both
    // `#if 0 // TODO(GA1-B4): fLotInfo->cbbASECL_LoginMode not yet ported`.
    // NOT opened here (out of boundary) -- integrator note only.
    // *** BOTH OPENED 20260826 by FW-LOTINFO-W30 (golden cMyDB.cpp:1651/:1744).
    // The two edtASECL_TesterID gates sitting beside them STAY: that member has
    // zero declaration on this class -- re-verified 20260826, the only hits in
    // this header are comment text at :67/:172/:343/:1663. ***
    TComboBox    *cbbASECL_LoginMode     = new TComboBox();     // golden uLotInfo.h:579
    TSpeedButton *sb_Main_EvenLevelLogin = new TSpeedButton();  // golden uLotInfo.h:574
    virtual void sb_Main_EvenLevelLoginClick(TObject *Sender);  // golden uLotInfo.cpp:10475-10489

    // -- btnAMRSetSECSClick (golden :16227-16232) ---------------------------
    // Writes TestIF_File.iAMRTrayCount[3] / iAMRDeviceCount[3] /
    // asAMRBinSetting[0] (cprod.h:2516/2517/2522) from three edits.  In-memory
    // only: the SECS send that consumes them is btnAMRSupplementClick, which
    // is EXITED (header EXIT B).
    TEdit *edAMRTrayCount   = new TEdit();         // golden uLotInfo.h:1108
    TEdit *edAMRDeviceCount = new TEdit();         // golden uLotInfo.h:1109
    TEdit *edAMRBinSetting  = new TEdit();         // golden uLotInfo.h:1110
    virtual void btnAMRSetSECSClick(TObject *Sender);   // golden uLotInfo.cpp:16227-16232
    // =======================================================================
    //  Steven 20260925 (Data.LotInfo 其餘分頁)：golden V912 uLotInfo（HT9011UC_Code_V3.33.912.0_20260908_Jimmy，cp950）
    //  其餘分頁的顯示端與 Barcode「Clear Count」。本體在 forms/fLotInfo.cpp 檔尾同名橫幅；tag 在 WebBridgeTags.cpp
    //  檔尾 W906_StageLotInfoTabTags；網頁 web/page/Data.LotInfo.html ＋ ht9045_lotinfo_wire.js。
    //  ⚠ 這一段的行號一律是 V912（"V912 uLotInfo.cpp:NNNN"）；上面各波寫的 "golden :NNNN" 是較舊的 golden 版本，不要混用。
    // =======================================================================
    TPanel     *palATCWorkingTemp   = new TPanel();      // V912 uLotInfo.h:39   -- ShowATCThermo :5605／ShowNewATCThermo :6604 寫 Caption
    TfLedValue *aldATCPower         = new TfLedValue();  // V912 uLotInfo.h:857  (TALed*) -- NetATCTimeTimer :8818／:8853／:8869 寫 Value
    TTabSheet  *tsRFMD              = new TTabSheet();   // V912 uLotInfo.h:442  -- V912 cprod.cpp:2869 寫 TabVisible（移植樹 cprod.cpp:3009 GATE S12-C1 缺這個成員與 btnESCFunction；這裡只補前者，那個 gate 不動 —— 可見度由 W906_RefreshTabVisible 照 golden 算）
    TButton    *btClearBarcodeCount = new TButton();     // V912 uLotInfo.h:265  -- OnClick = btClearBarcodeCountClick（dfm :4630）
    bool        W906_bATCWorkTempShown = false;          // 移植樹：palATCWorkingTemp 有沒有被 golden 那兩行寫過（沒寫過＝dfm 的 "0.0 'C"，不是量測值 -> tag 送 null）
    bool        W906_bATCPowerKnown    = false;          // 移植樹：aldATCPower 這一拍是不是 golden 確定的值（NetATCTimeTimer 第一支；其餘要看 ATC 連線，本樹沒有）
    void        W906_RefreshTabVisible();                // FormShow／Timer2Timer／tmrChamberBoostTimer／cprod.cpp／cTrayAssignment.cpp 的 TabVisible 指派（只取指派，不跑其他副作用）
    AnsiString  W906_FormShowActivePage();               // FormShow :348／:376-379／:895／:1076-1098 ＋ Timer2Timer :7247-7253 的 ActivePage（回傳 dfm 元件名）
    void        W906_ShowATCThermoDisplay();             // ATC 分頁的顯示部分（SetATCFormVisible :569、ShowATCTempPanel :669、:670-675、:1157-1158、ShowATCThermo :5590-5606、ShowNewATCThermo :6594-6606、NetATCTimeTimer :8812-8818）
    void        btClearBarcodeCountClick();              // V912 uLotInfo.cpp:10168-10184（Sender 沒用到，拿掉）
    void        btClearBarcodeListClick();               // V912 uLotInfo.cpp:10186-10195 —— 本體在 WebLotInfo.cpp（只編進 wb_serve），經檔尾 W906_ClearBarcodeListBody 掛進來。AI(W906-FRW-S94) 20260926
    void        spOCRCleanListClick();                   // V912 uLotInfo.cpp:14461-14466（本體 forms/fLotInfo.cpp 檔尾）。AI(W906-FRW-S94) 20260926
    void        W906_ClearListButtonsVisible();          // btClearBarcodeList／spOCRCleanList 的 Visible：dfm 預設 → Timer2Timer V912 :7064-7067／:7285（只取這兩個指派）。AI(W906-FRW-S94) 20260926
    AnsiString  W906_TesterLogTail(int nLines, int* pCount);  // mmTesterLog（V912 uLotInfo.dfm:11276）最後 nLines 行（"\n" 連接）＋ 總行數
    TButton    *btnSave             = new TButton();     // V912 uLotInfo.h:256  -- Selection 分頁 Save，OnClick = btnSaveClick（dfm :4446）
    void        W906_SelectionDfmDefaults();             // Selection 分頁元件：dfm 設計期 Visible／Enabled → FormShow :631 → SetSelectionVisible（含 :1369-1378 A75 迴圈，照 dfm 子元件展開）
    void        btnSaveClick();                          // V912 uLotInfo.cpp:7387-7415（Sender 沒用到，拿掉）
    // =======================================================================
    //  AI(W906-PROD-S117) 20260926（Steven 團隊，St01）：Lot End —— RULINGS_20260926 S117／S120-4（Steven「現在排」）。
    //  golden V912 uLotInfo.cpp（HT9011UC_Code_V3.33.912.0_20260908_Jimmy，cp950）。本體在 forms/fLotInfo.cpp 檔尾同名橫幅；
    //  網頁動作 WebLotInfo.cpp lotinfo.op「lotEnd.state」「lotEnd」（兩段式）；頁面 web/page/Data.LotInfo.html Lot 分頁的 Lot End 鈕。
    //  ⚠ 上面 :1097-1098 的清單（sbSECSLotEndClick／SetLotEnd「未移植」）自 20260926 起已過期：兩支都在這裡。
    // =======================================================================
    virtual void SetLotEnd(AnsiString sFunc);                // V912 uLotInfo.cpp:2014-2379（golden 註解「整合Lot測試報表 Lot End」）
    // V912 uLotInfo.cpp:1387-1444。Sender 照 golden 保留並照 golden 轉型成 TSpeedButton*（:1391）讀 Tag。
    //   ⚠ golden 另有呼叫者傳 fLotInfo 本身當 Sender（csystem.cpp:12269／:12284／:12291／:12301／:12309，TForm 的 Tag＝0）；
    //     移植樹 TfLotInfo 不是 TObject，那幾處（移植樹 csystem.cpp:8756 起，在 GATE G4-3 裡）解閘時請改傳 fLotInfo->sbSECSLotEnd
    //     （dfm :341-349 沒有 Tag ⇒ 0，行為相同）。網頁傳的就是 sbSECSLotEnd。
    virtual void sbSECSLotEndClick(TObject *Sender);
    bool        W906_LotEndPanelVisible();                // palSecsGem->Visible：dfm :314-323（沒寫 Visible＝True）→ FormShow :367-371（CC_MTI 藏）→ :407-412（SECS／ATP 鎖參數時顯示）
    void        W906_LotEndDfmDefaults();                 // sbSECSLotStart／sbSECSLotEnd 的 dfm 設計期值（:332-349：Visible／Enabled＝True、GroupIndex=1、Caption）—— 只套一次
    AnsiString  W906_LotEndPrecheck();                    // 只讀：照 sbSECSLotEndClick :1393-1424 的順序，回傳 golden 會在哪一步拒絕（"" ＝ 會執行）。只給網頁預覽，真正判斷在本體
    bool        W906_bLotEndDfmApplied = false;           // W906_LotEndDfmDefaults 套過了沒
    int         W906_SetLotEndCount    = 0;               // SetLotEnd 進來過幾次（網頁用前後差判斷「真的執行了」）
    AnsiString  W906_LotEndResult;                        // 上一次 sbSECSLotEndClick 停在哪：system-running／must-clean-out-1／must-clean-out-2／oee-not-started／executed
    TStringList *W906_LotEndSkipped = new TStringList();  void RTCChangeFile(bool bNeedDelete=true);  void Timer1Timer();  TfLotInfoTimer *Timer1 = new TfLotInfoTimer();  // 上一次 Lot End 裡「被閘住、而且 golden 這一刻會執行」的敘述（WebLotInfo.cpp 轉成 ELTodo；這個 TU 不連 FileRW）  //AI(W906-E020-LI6) 20261002 [W906] (St01): + golden V912 uLotInfo.h RTCChangeFile(bool bNeedDelete=true) / Timer1Timer (Sender dropped) / Timer1 (TTimer; dfm :14621-14626 Enabled=False Interval=100 -> TfLotInfoTimer :1674, ->Enabled only); bodies in LotInfo_E020.cpp (ht9045_sm); same line, no line moved
    TfLotInfo();
    virtual ~TfLotInfo() {}
};

// =============================================================================
//  Steven 20260925 (Data.LotInfo 其餘分頁)：mmTesterLog 的真 memo。
//  golden mmTesterLog 是 TMemo（V912 uLotInfo.dfm:11276，Lines.Strings=('')），由 Interface/TesterTCP_Socket.cpp:394／:399
//  （golden TesterTCP.cpp:290／:294）Clear()／Lines->Add()。forms/FormWidgets.h 的 TfMainMemo 是 no-op（Add 不存、Count 恆 0），
//  而且 fAGV->mmE84Log 等別的表單也用它 —— 所以不改那個共用型別，只在這裡派生一個存得住行的版本；
//  TfLotInfo 建構子把 mmTesterLog 指向它（成員型別仍是 TfMainMemo*，TesterTCP 的呼叫走虛擬函式到這裡）。
// =============================================================================
#include <vector>
class TfLotInfoLogMemoLines : public TfMainMemoLines
{
public:
    std::vector<AnsiString> L;
    void Add(AnsiString s) override;
};
class TfLotInfoLogMemo : public TfMainMemo
{
public:
    TfLotInfoLogMemoLines *Log;
    TfLotInfoLogMemo();
    void Clear() override;
};
// golden TfBarCode::DoBarcodeCount（V912 BarCode/BarCode.cpp:5863-5917）—— 寫 fLotInfo->sgBarcode 的格子、回傳 2DID 良率要不要告警。
// 移植樹的 TfBarCode（BarCode/BarCode.h）沒有這個成員，各 SM 路徑（BarCode_*.cpp）的副本又把寫格子那幾行 gate 掉了，
// 所以本體放在 forms/fLotInfo.cpp 檔尾（它只寫 fLotInfo 的元件）。
bool W906_DoBarcodeCount();
// AI(W906-FRW-S94) 20260926（Steven 團隊）：TfLotInfo::btClearBarcodeListClick 本體的安裝座（同 forms/fMain.h W906_ClarnDataBody 的作法與理由）。
//   golden 本體（V912 uLotInfo.cpp:10186-10195）要清 BarCode 的 2D 重複碼記憶（map2DList／list2DByLot，兩個都在 ht9045_sm），
//   forms/fLotInfo.cpp（ht9045_forms）直接引用它們 ⇒ 每個連到 fLotInfo.o 的執行檔都得多抽出 BarCode 的物件檔（陷阱 #2 的反面：
//   多抽一個成員就多一串未定義符號，測試執行檔可能因此連不起來）。所以這裡只放指標；本體在 WebLotInfo.cpp（只編進 wb_serve），
//   wb_serve 開機呼叫 W906_InstallLotInfoClearListBody() 裝上（不用 static init 自我登錄，同 fMain.h 安裝座的理由）。
//   沒裝（測試執行檔）＝ btClearBarcodeListClick 只把計數器加一，什麼都不清。
typedef void (*W906_ClearBarcodeListBodyFn)(TfLotInfo *self);
extern W906_ClearBarcodeListBodyFn W906_ClearBarcodeListBody;   // 0 ＝ 沒裝（預設）
extern int W906_ClearBarcodeListCallCount;                      // 進來過幾次（先加再分派，同 fMain.cpp W906_Clarn_DataCallCount 的用意）
// AI(W906-PROD-S117) 20260926：golden TfMain::slLotInfolog（V912 main.h:1496；main.cpp:1665-1668 建構）的檔名。
//   移植樹 TfMain 沒有這個成員（forms/fMain.* 是共用檔），物件改放在 forms/fLotInfo.cpp 檔尾（golden 全樹唯一的使用者就是 SetLotEnd
//   :2038-2039）。回傳上一次實際寫入的檔（TMyStringList::sLastFileName）；還沒寫過回 ""。
AnsiString W906_LotInfoLogFileName();
extern TfLotInfo *fLotInfo;

#endif // FORMS_FLOTINFO_H
