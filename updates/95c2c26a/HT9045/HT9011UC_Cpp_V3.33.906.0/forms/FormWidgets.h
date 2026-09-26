// =============================================================================
//  forms/FormWidgets.h  --  the facade's widget-level type layer
//
//  AI(W906-W7-F0) 20260728: created by the W7-F0 FormsFacade refactor
//  (docs/W7_UI_ARCHITECTURE_PLAN.md SS6-F0-c / SS2-D4 / SS2-D6).
//
//  WHAT THIS FILE IS
//  -----------------
//  Every `TfXxxYyy` widget stand-in that used to be defined inline in
//  FormsFacade.h now lives here, and (where it is a STOCK VCL control) is a
//  plain `typedef` onto the single unified type in vclcompat/Controls.h.
//  The per-form headers (forms/fMain.h, forms/fLotInfo.h, ...) include THIS
//  file and then declare only their own form class.
//
//  WHY A SHARED WIDGET HEADER RATHER THAN ONE PER FORM
//  ---------------------------------------------------
//  The widget stand-ins are genuinely cross-form: TfMainMemo is used by
//  TfMain (meShuttle1/2), TfAGV (mmE84Log) AND TfLotInfo (mmTesterLog);
//  TfSortCTPanel by TfSortCT AND TfSCKART; TfLotInfoEdit by TfLotInfo, TfMain,
//  TfSCKART AND TfCleaning; TfLedValue by TfMain AND TfLotInfo.  Putting them
//  in one form's header would force the other forms' headers to include it,
//  re-creating exactly the cross-form coupling the split exists to remove.
//  One tiny shared header keeps every forms/fXxx.h independent of every other.
//
//  THIS IS ALSO THE ANTI-COLLISION BOUNDARY (plan SS3-C4 / SS8)
//  -----------------------------------------------------------
//  The reason FormsFacade.h was a tree-wide write lock is that a wave needing
//  ONE new member had to edit the same file every other wave was editing.
//  After the split, a wave adds MEMBERS to its own forms/fXxx.h.  It should
//  NOT need to touch this file at all: the widget SHAPES are done.  If a wave
//  thinks it needs a new widget type here, the answer is almost always an
//  existing vclcompat/Controls.h type (or, for the 3 custom control families,
//  vclcompat/{TrayCore,BtnPanelCore,LedCore}).
//
//  ZERO-BEHAVIOUR-CHANGE CONTRACT
//  ------------------------------
//  W7-F0 is a pure refactor.  Every typedef below was checked to preserve the
//  replaced struct's exact member set AND exact default values -- see
//  vclcompat/Controls.h's DEFAULT-VALUE RULE for why the unified types default
//  Visible/Enabled to false rather than to VCL's true.  The single member that
//  legitimately started life true/true (fLotInfo->palRemoveTray) has that
//  restored explicitly in forms/fLotInfo.cpp's constructor.
//
//  PRE-FLIGHT THAT MADE THE TYPEDEFS SAFE (plan SS6-F0-c precondition)
//  -------------------------------------------------------------------
//  Aliasing a bespoke `struct {AnsiString Text;}` onto a POLYMORPHIC class
//  (TObject base + virtual dtor => a vptr, different sizeof and layout) is
//  only source-compatible if nothing uses the type by value.  Verified
//  tree-wide before doing it, and it holds:
//    * every use of every one of these type names OUTSIDE FormsFacade.{h,cpp}
//      is a COMMENT.  Measured over every *.cpp/*.h in the tree (build dirs and
//      .svn excluded): 30 hits, of which 0 are non-comment lines -- all 30 are
//      `//`-prefixed prose in earlier waves' notes (atester_shims.h,
//      AutoClean.cpp, AGV_E84.cpp, AGV_PortScan.h, automation.h,
//      SCK_ART_Remainder.{h,cpp}, BarCode_8CCD_Glue.cpp,
//      BarCode_Shuttle2_CCDScan.h, TesterTCP_Socket.{h,cpp},
//      uHGemEquipment.h, tests/test_MyProductionRecord.cpp);
//    * inside FormsFacade.{h,cpp} every use is `TfXxx *member;` +
//      `member = new TfXxx();`;
//    * zero `sizeof`/`memcpy`/`memset`/`memcmp` on any of them;
//    * zero by-value declarations, zero aggregate `{...}` initialisation.
//
//  ONE VERIFIED EXCEPTION, RECORDED BECAUSE A VPTR DOES CHANGE IT
//  --------------------------------------------------------------
//  Automation/SCK_ART_Remainder.cpp:1651 preserves a GOLDEN BUG verbatim
//  (golden SCK_ART.cpp:3636): `Str.sprintf("LOT_ID:%s", fLotInfo->edtSysLotID)`
//  passes the OBJECT POINTER to a `%s` conversion (golden forgot `->Text`), so
//  it reads raw bytes starting at the object's own address as a NUL-terminated
//  C string.  That is undefined behaviour in golden and here alike, but it does
//  mean the bytes it reads change: they used to be AnsiString's internal
//  std::string, and are now the vptr.  Both are "interpret a pointer's bytes as
//  chars", so the risk class is unchanged -- and that branch is deliberately
//  NOT exercised by any test (see SCK_ART_Remainder.h's own "Golden bugs
//  preserved VERBATIM #2" note, which records that its PART 11 test steers
//  around it via bReadLotInfoFromART==true precisely so no automated outcome
//  depends on UB).  Recorded here so it is a known, reviewed consequence rather
//  than a surprise.  The golden bug itself is still preserved, untouched.
// =============================================================================
#ifndef FORMS_FORMWIDGETS_H
#define FORMS_FORMWIDGETS_H

#include "vclcompat/vcl_compat.h"   // AnsiString, HWND-free umbrella
#include "vclcompat/Controls.h"     // the unified stock widget stand-ins

// AI(W906-AutoCleanFoundation) 20260721 (moved here verbatim by W7-F0):
// golden fMain->AutoCleanStringGrid (main.h:457, TStringGrid*) is NOT cosmetic
// -- AutoClean/AutoClean.cpp's RestoreCleanKitData/CheckCleaningCount read it
// back via atoi(...->Cells[X][Y].c_str()). Reuses vclcompat::TStringGrid (built
// W906-uHGemEquipment 20260716 for the analogous SECS SV/EC-grid problem) --
// a materially better fit than inventing a bespoke AutoCleanSubstrate type,
// since it already has real Cells[][]/RowCount/ColCount semantics. Second
// consumer after SECSGEM/uHGemEquipment.h, which sets the `using` idiom this
// mirrors verbatim (duplicate identical `using` declarations across headers
// are legal C++, not an ODR conflict).
#include "vclcompat/StringGrid.h"
using vclcompat::TStringGrid;

// =============================================================================
//  PART 1 -- STOCK WIDGETS: aliased onto vclcompat/Controls.h (11 types)
//
//  Each line records the golden VCL class of the members that use it, verified
//  THIS WAVE by direct grep of golden main.h / uLotInfo.h / cSortCT.h /
//  Automation/SCK_ART.h / AutoClean/uCleaning.h / Automation/AGV.h (golden tree
//  HT9011UC_Code_V3.33.906.0_20260618).  Where a stand-in is shared by members
//  of DIFFERENT golden classes, that conflation is called out -- W7-F0 does NOT
//  repoint individual members (that is deliberately out of a mechanical
//  aliasing step); it records the conflation for W7-C4 / W7-U.
// =============================================================================

// golden TCheckBox* -- fMain->cbShowShuttleSensor (main.h:540),
// cbTestOutShuttleSensor (:543), cbShowInShuttleSensor (:910),
// cbIndexDrop (:117), chkCleanPadPickErr (:132).  All 5 genuinely TCheckBox.
// Was: struct TfMainCheckBox { bool Checked; ...Checked(false){} };
typedef vclcompat::TCheckBox TfMainCheckBox;

// golden TFont* -- fMain->pnlCleanCountFont (golden main.h TPanel->Font).
// Only ->Color is ever assigned (SearchCleanNum sets clRed/clNavy).
// Was: struct TfMainFont { int Color; ...Color(0){} };
typedef vclcompat::TFont TfMainFont;

// golden TPageControl* -- fMain->pgMain (main.h:64).
// Was: struct TfMainPageControl { int ActivePageIndex; ...(0){} };
typedef vclcompat::TPageControl TfMainPageControl;

// golden TPanel* -- fMain->labAutomation (main.h:802), palMainStatus (:669),
// pnlCleanCount (:864).
// CONFLATION (verified, left for W7-U): fMain->AutoCleanContactCountLabel is a
// golden TLabel* (main.h:148), not a TPanel*.  Both classes carry Caption, and
// Caption is the only member used, so the alias is behaviourally exact; the
// member's declared type is simply less precise than golden.
// Was: struct TfMainPanel { AnsiString Caption; };
typedef vclcompat::TPanel TfMainPanel;

// golden TLabel* -- fMain->lbCCDStatus (main.h:672; ->Visible written by
// atester.cpp:1894).
// CONFLATION (verified, W7-C4 item): fMain->mtAuto1/mtAuto2/mtAuto3 are golden
// **TTMyTray*** (main.h:168-170), i.e. the CUSTOM tray control, not a stock
// TLabel.  The tray-arm engine only assigns ->Color (acatchtray.cpp:4554-4641),
// which is a stock TControl-level property, so aliasing is behaviourally exact
// (defaults Color=0 / Visible=false both preserved).  Repointing mtAutoN at
// vclcompat::TrayCore is W7-C4's job, together with tmyAutoClean/mtPlate2.
// Was: struct TfMainTrayPanel { int Color; bool Visible; ...(0),(false){} };
typedef vclcompat::TLabel TfMainTrayPanel;
// AI(W906-ST-S3-B2b) 20260918: an honestly-named TLabel alias. TfMainTrayPanel
//   above is also a TLabel, but its name says "panel" and the members that need
//   one now (fMain->Label3) are labels, not panels. One typedef is cheaper than
//   a reader having to learn that the tray-panel alias is really a label.
typedef vclcompat::TLabel TfMainLabel;

// golden TPanel* -- fSortCT->pnlHP1 (cSortCT.h:61), pnlHP2 (:63),
// pnlLoad (:59), pnlLoadCID (:265), pnlCoverTrayD (:280), pnlTrayCnt[]
// (:337); fSCKART->palTestCnt (SCK_ART.h:70), palLotNumber (:71),
// palRTTryCnt (:75), pnlProcessCode (:181).  All 10 genuinely TPanel.
// Was: struct TfSortCTPanel { AnsiString Caption; };
typedef vclcompat::TPanel TfSortCTPanel;

// AI(W906-FW3-PIGSV) 20260829: golden TPanel* --
// fProductionInfo->pn_ErrorMsg_HALTStatus (ProductionInfo.h:77),
// pn_ErrorMsg_PauseStatus (:105).  兩個都是真的 TPanel，
// 而且目前只用到 Caption（golden ProductionInfo.cpp:2743-2744）。
typedef vclcompat::TPanel TfProductionInfoPanel;

// golden TComboBox* -- fLotInfo->cbRunMode (uLotInfo.h:307), cbProcess (:470);
// fMain->cbSetupFileName (main.h:875).  All 3 genuinely TComboBox.
// NOTE: ->Visible's `false` default is LOAD-BEARING -- ainarm_SearchPlacePlate
// .cpp:4811 reads `fLotInfo->cbRunMode->Visible==true`.  See Controls.h's
// DEFAULT-VALUE RULE.
// Was: struct TfLotInfoRunMode { bool Visible; AnsiString Text; ...(false){} };
typedef vclcompat::TComboBox TfLotInfoRunMode;

// golden TLabel* -- fLotInfo->labNowLoaderTrayID (uLotInfo.h:1078).
// Was: struct TfLotInfoLabel { AnsiString Caption; };
typedef vclcompat::TLabel TfLotInfoLabel;

// golden TLabel* -- fLotInfo->labTCPIPStatus (uLotInfo.h:465); Caption+Color.
// Was: struct TfLotInfoStatusLabel { AnsiString Caption; int Color; ...(0){} };
typedef vclcompat::TLabel TfLotInfoStatusLabel;

// golden TPanel* -- fLotInfo->palRemoveTray (uLotInfo.h:828); Enabled+Visible,
// both WRITTEN by Automation/AGV_PortScan.cpp:305-306 and never read.
// Its true/true initial values are restored explicitly in forms/fLotInfo.cpp.
// Was: struct TfLotInfoPanel { bool Enabled; bool Visible; ...(true),(true){} };
typedef vclcompat::TPanel TfLotInfoPanel;

// golden TEdit* -- the tree's single most-reused stand-in.  Genuine TEdit*
// members: fLotInfo->edtSysLotID (uLotInfo.h:302), edtSysOperatorID (:306),
// edtASECL_LotID (:576), edtCusLotID (:623), edtCusDevGrp (:624),
// edtDevice (:625), edtCusStep (:1038), edInsertion (:1044), edFlowID (:1046);
// fMain->edWorkTemperBase (main.h:732), edHPX (:113), edHPY (:114);
// fSCKART->edlRTTryCnt (SCK_ART.h:78); fCleaning->edCleaningCount
// (uCleaning.h:69).
// CONFLATIONS (verified, left for W7-U -- ->Text only in both cases, so
// behaviourally exact): fLotInfo->lbledtCustomer is a golden TLabeledEdit*
// (uLotInfo.h:609) and fLotInfo->mmo2DLotInfo is a golden TMemo*
// (uLotInfo.h:846).  vclcompat::TLabeledEdit and vclcompat::TMemo both exist
// now and both derive from TCustomEdit, so those two members can be repointed
// with a one-word edit whenever their form's own wave next touches them.
// Was: struct TfLotInfoEdit { AnsiString Text; };
typedef vclcompat::TEdit TfLotInfoEdit;

// =============================================================================
//  PART 2 -- DEFERRED STAND-INS: kept bespoke, with a stated reason each
//
//  These are NOT aliased in W7-F0.  Every one still gets the `: public
//  vclcompat::TObject` base + virtual destructor that SS2-D4 / SS9-R8 require
//  (that is the load-bearing half of F0-a: it is what stops the silent `void*`
//  overload bind), so the safety property holds for the whole set even where
//  the type unification is deferred.
// =============================================================================

// ---------------------------------------------------------------------------
//  golden TMyStringList* -- fMain->slAutoSiteMapLog (main.h:1486, verified).
//  DEFERRED, permanently as far as Controls.h is concerned: TMyStringList is a
//  cmydef type (already forward-declared cmydef.h:15), not a stock VCL control,
//  so plan SS2-D5 / SS5-10 keep it out of vclcompat/Controls.h.  The leaves call
//  exactly one method on it (AddTextWithDateTime), an append/log no-op offline.
//  (W6.2 original wording preserved.)
// ---------------------------------------------------------------------------
class TfMainSiteMapLog : public vclcompat::TObject
{
public:
    virtual ~TfMainSiteMapLog() {}
    virtual void AddTextWithDateTime(AnsiString /*S*/) {}   // golden TMyStringList::AddTextWithDateTime
};

// ---------------------------------------------------------------------------
//  golden TTMyTray* -- fMain->htShullte0 / htShullte1 (main.h:345-346).
//  DEFERRED to W7-C4 (vclcompat::TrayCore): TTMyTray is one of the 3 CUSTOM
//  control families, which SS2-D5 keeps out of Controls.h.
//
//  AI(W906-W7-F0) 20260728: CORRECTED COMMENT.  This stand-in was documented as
//  "golden THeatTable*".  There is NO class named THeatTable anywhere -- I
//  grepped the whole golden tree myself for this wave: 0 hits.  golden main.h
//  :345/:346 declare `TTMyTray *htShullte0; TTMyTray *htShullte1;`.  Same
//  correction as the one plan SS10-5 already records for tmyAutoClean.
//
//  Behaviour unchanged: the shuttle engine calls SetCellColorIndex(col,row,idx)
//  only inside the Motor-View page guard (pgMain->ActivePageIndex ==
//  emp7TabSheet21, both 0 offline so equal) -- and it is a no-op either way.
// ---------------------------------------------------------------------------
class TfMainGrid : public vclcompat::TObject
{
public:
    virtual ~TfMainGrid() {}
    virtual void SetCellColorIndex(int /*col*/, int /*row*/, int /*idx*/) {}
};

// ---------------------------------------------------------------------------
//  golden TTMyTray* -- fMain->tmyAutoClean (main.h:347) and fMain->mtPlate2
//  (main.h:164).  DEFERRED to W7-C4 (vclcompat::TrayCore), exactly as plan
//  SS6-F0-c instructs ("F0 only aliases them to interim types; the real
//  reconnection to TrayCore/LedCore is W7-C4").
//
//  AI(W906-W7-F0) 20260728: CORRECTED COMMENT -- see plan SS10-5.  The old
//  member comment on tmyAutoClean called its golden type `THeatTable*`; golden
//  main.h:347 says `TTMyTray *tmyAutoClean;` (verified by direct grep of golden
//  this wave, together with the 0-hit THeatTable search above).  Its sibling
//  mtPlate2 was already labelled correctly (golden main.h:164 `TTMyTray
//  *mtPlate2;`, also verified).
//
//  Surface unchanged from the W906-AutoCleanFoundation original: XItem/YItem/
//  Top/Width/Height are write-only offline (SetAutoCleanICCount /
//  SetAutoCleanTrayPosition only WRITE them); SetCellColorIndex is a no-op and
//  IS called (AutoClean/AutoClean.cpp's SetCleanCellValue helper).
// ---------------------------------------------------------------------------
class TfMainAutoCleanGrid : public vclcompat::TObject
{
public:
    int XItem, YItem, Top, Width, Height;
    TfMainAutoCleanGrid() : XItem(0), YItem(0), Top(0), Width(0), Height(0) {}
    virtual ~TfMainAutoCleanGrid() {}
    virtual void SetCellColorIndex(int /*col*/, int /*row*/, int /*idx*/) {}
};

// ---------------------------------------------------------------------------
//  golden TALed* -- fMain->ALed1 (main.h:355, verified), fLotInfo->ALedLoader
//  (uLotInfo.h:971) and aLedAuto1/2/3 (:965/:969/:970).  All 4 are the same
//  CUSTOM component type, which is why one stand-in serves both forms.
//  DEFERRED to W7-C4 (vclcompat::LedCore), exactly as plan SS6-F0-c instructs.
//  Consumer: Automation/AGV_PortScan.cpp (bScanLoadPortState_SPIL /
//  bScanUnLoadPortState_SPIL).  (W5-Automation original wording preserved.)
// ---------------------------------------------------------------------------
class TfLedValue : public vclcompat::TObject
{
public:
    bool Value;
    TfLedValue() : Value(false) {}
    virtual ~TfLedValue() {}
};

// ---------------------------------------------------------------------------
//  fMain->BtnOneCycle -- ->Down, offline false (the else-branch one-cycle
//  trigger stays inert).  DEFERRED to W7-C4 (vclcompat::BtnPanelCore).
//
//  AI(W906-W7-F0) 20260728: CORRECTED COMMENT.  This stand-in was documented as
//  "golden TSpeedButton*".  golden main.h:72 declares `TBtnPanel *BtnOneCycle;`
//  -- TBtnPanel is one of the 3 CUSTOM control families (elec\myvcl\butPa1.cpp),
//  and `->Down` is BtnPanelCore's dominant member (671 golden uses) whereas the
//  stock TSpeedButton path is a different control entirely.  Verified by direct
//  grep of golden main.h this wave.  That is precisely why this one is NOT
//  aliased onto vclcompat::TSpeedButton: doing so would have written a
//  fresh WRONG claim into the tree while "unifying".
// ---------------------------------------------------------------------------
class TfMainSpeedButton : public vclcompat::TObject
{
public:
    bool Down;
    TfMainSpeedButton() : Down(false) {}
    virtual ~TfMainSpeedButton() {}
};

// ---------------------------------------------------------------------------
//  golden TMemo* -- fMain->meShuttle1/meShuttle2 (main.h:542/541 respectively --
//  AI(W906-F0fix) 20260728: corrected from wrong "542-543", :543 is
//  TCheckBox *cbTestOutShuttleSensor, not a TMemo; re-verified via cp950 read
//  of golden main.h), fAGV->mmE84Log (Automation/AGV.h:85), fLotInfo->mmTesterLog
//  (uLotInfo.h:468).
//
//  DEFERRED -- and this one is a DELIBERATE, REPORTED DEVIATION from plan
//  SS6-F0-c's alias list, not an oversight.  vclcompat::TMemo now exists and is
//  the correct target, BUT repointing these members at it is NOT a pure
//  refactor: this stand-in's ->Lines->Add() is a NO-OP and its ->Lines->Count
//  is a permanently-0 int, whereas vclcompat::TMemo::Lines is a REAL
//  TStringList (which plan SS6-F0-a explicitly requires -- "reuse the existing
//  TStringList, do not reinvent it").  Switching them turns three log sinks
//  from discard-everything into real storage, and makes two size-threshold
//  branches reachable that are dead today:
//      * Automation/AGV_E84.cpp:1011  if(mmE84Log->Lines->Count>=500) Clear();
//        -- its Add is at :995 and is REACHABLE (no offline-false guard).
//      * Interface/TesterTCP_Socket.cpp:399 mmTesterLog->Lines->Add(Log) is
//        REACHABLE and unconditional per logged line.
//      * acarry.cpp:1436/1971 (Count>1024) and :3969 (Add) are inside
//        `if(fMain->cbShowShuttleSensor->Checked==true)` (guards at :1434/
//        :1969/:3965); acarry.cpp:8405/8504 (Count>1024) are the SAME check
//        but guarded by `cbShowInShuttleSensor` instead (guards at :8392/
//        :8491, NOT `cbShowShuttleSensor` -- AI(W906-F0fix) 20260728:
//        corrected mis-attribution + line numbers, re-derived by grep against
//        golden acarry.cpp).  cbShowShuttleSensor and cbShowInShuttleSensor
//        are two distinct golden TCheckBox members (main.h:540/:910) and both
//        default Checked==false offline, so those branches are dead either
//        way regardless of which guards which.
//      * cpublic.cpp:683-762's whole memo block is inside `#if 0 // TODO(W6)`
//        and is not compiled at all.
//  No test observes any of it today, so the F0 gates could not have caught the
//  change -- which is exactly why it must not ride along inside a wave whose
//  contract is "zero behaviour change and therefore unusually sharp gates".
//  Repointing is a one-line-per-member follow-up (`TfMainMemo` ->
//  `vclcompat::TMemo`, drop TfMainMemoLines) that should land as its own change
//  with its own assertions on the >=500 / >1024 flush paths.
// ---------------------------------------------------------------------------
class TfMainMemoLines : public vclcompat::TObject
{
public:
    int Count;
    TfMainMemoLines() : Count(0) {}
    virtual ~TfMainMemoLines() {}
    virtual void Add(AnsiString /*s*/) {}  virtual void SaveToFile(AnsiString /*path*/) {}   // AI(W906-LOGSINK) 20260927: no-op like Add(), so TfMain::AddAutoCleanMessage (golden main.cpp:30254) compiles; Count stays 0 so it is never reached
};
class TfMainMemo : public vclcompat::TObject
{
public:
    TfMainMemoLines *Lines;
    TfMainMemo() { Lines = new TfMainMemoLines(); }
    virtual ~TfMainMemo() {}
    virtual void Clear() {}
};

#endif // FORMS_FORMWIDGETS_H
