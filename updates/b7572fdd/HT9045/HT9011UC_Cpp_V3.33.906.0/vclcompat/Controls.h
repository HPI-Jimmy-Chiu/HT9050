// ===========================================================================
//  vclcompat/Controls.h
//  BCB6 VCL **stock** widget compatibility STAND-INS -- the single, unified
//  home for the value-holder shape of every stock VCL control the ported tree
//  needs.  This is deliberately NOT a general-purpose VCL Controls.hpp shim.
//
//  AI(W906-VCW1) 20260721: created for SECSGEM's SetECValue/DataItemOutSV/
//  DataItemOutEC/GetECDataValue dynamic_cast dispatch cascade (golden
//  uHGemClass.cpp:2691-2782 [SetECValue], uHGemEquipment.cpp:2513-2585/
//  2882-2957 [DataItemOutSV/DataItemOutEC], SecsSvEcRegistration.cpp:624-701
//  [GetECDataValue -- this same wave also un-gated its IsVCL==1 cast branch,
//  see that file's own note]). Golden
//  dynamic_casts a stored `TObject*` (an EC/SV's registered "VCL pointer",
//  VCL_NAME=="1") against TPanel/TCustomEdit/TComboBox/TLabel/TCheckBox/
//  TRadioGroup/TStringList in that fixed order, first match wins.
//  TStringList already exists (vclcompat/TStringList.h, and this wave's own
//  R1 gave it the `: public TObject` base the cast needs -- see that file's
//  own note); these are the other 6.
//
// ===========================================================================
//  AI(W906-W7-F0) 20260728: EXPANDED from the original 6 SECS-cast-only types
//  to the unified stock-widget set, per docs/W7_UI_ARCHITECTURE_PLAN.md
//  SS2-D4 / SS2-D5 / SS6-F0-a.  WHY (all three reasons are measured, not
//  aspirational):
//
//   1. DRIFT.  Before this wave the tree carried ~13 parallel, independently
//      drifting `{AnsiString Text;}` / `{AnsiString Caption;}` / `{bool
//      Checked;}` value-holder types (FormsFacade.h's TfLotInfoEdit /
//      TfMainPanel / TfSortCTPanel / TfLotInfoLabel / ..., plus TU-local
//      W7T1_TEditSeam / W5_32S_* / W64B_* / W5FA_TextField / TesterTCPSocket_
//      Label / TfObserverLabel / ...).  Every new wave added another.  One
//      home stops that.
//
//   2. THE `void*` OVERLOAD TRAP (plan SS4-V2 / SS9-R8 -- the load-bearing
//      reason this file exists at all).  SECSGEM/SecsSvEcRegistration.h
//      declares SetSVDataPointer/SetECDataPointer with SAME-ARITY overloads
//      taking `void *P` (:133 / :162) and `TObject *P` (:149 / :180).  A
//      widget stand-in with NO `TObject` base can ONLY convert to `void*`, so
//      it binds the raw-pointer overload SILENTLY -- registering a widget as
//      a scalar, with no diagnostic.  With a `TObject` base, [over.ics.rank]
//      makes derived-to-base strictly better than pointer-to-void, so the
//      TObject* overload wins.  ~1740 SV/EC registrations are still to land;
//      this must be right BEFORE they do.  tests/test_w7_f0_controls_guard.cpp
//      static_asserts it per type, and carries a negative control proving the
//      assertion is not vacuous.
//
//   3. `fXxx->widget->Prop` MUST STAY VERBATIM.  Members are public DATA, not
//      accessors (plan SS2-D2: 69.3% of golden's 4,266 `fMain->` sites are
//      data-member syntax, and C++ has no `__property`), so every existing
//      call site compiles unchanged.
//
//  SCOPE BOUNDARY (1) -- READ BEFORE ASSUMING ANY REAL EC/SV IS NOW LIVE
//  -------------------------------------------------------------------------
//  (Carried forward VERBATIM from the W906-VCW1 original -- still true after
//  the W7-F0 expansion.)  Every one of the ~1740 real SetECDataPointer/
//  SetSVDataPointer call sites (uHGemHT9045_EC.cpp/_SV.cpp, still almost
//  entirely untranslated) registers a widget pointer belonging to a REAL VCL
//  form (a real fPanel->Caption, edtXxx, cbxXxx, ...).  None of those forms
//  exist in this tree yet, so NOTHING anywhere in the object graph today
//  constructs one of these types for an EC/SV.  Every dynamic_cast against
//  them therefore, correctly and safely, returns nullptr for every currently-
//  registered EC/SV.  This header makes the CAST-DISPATCH CODE compile and
//  dispatch correctly; it does NOT wire up any real widget-backed EC/SV, and
//  must not be described as doing so.
//
//  The W906-VCW1 original also carried a "MINIMAL SURFACE" rule (each class
//  holds ONLY the members golden's 3 cast-dispatch bodies touch).  W7-F0
//  DELIBERATELY SUPERSEDES that rule -- this is now the tree-wide unified home
//  for stock widget stand-ins, so it carries the measured tree-wide property
//  set listed below, not just the cast-dispatch subset.  It does NOT become an
//  anything-goes dumping ground: every member below is either (a) touched by a
//  real ported call site, or (b) named in the plan's measured top-20 property
//  census with its reason stated.  Nothing is added "because a real TPanel
//  would also have it".
//
//  SCOPE BOUNDARY (2) -- STOCK CONTROLS ONLY (plan SS2-D5 / SS5-10)
//  -------------------------------------------------------------------------
//  The three CUSTOM control families get their own files and are NOT here:
//    * TTMyTray / TTMyTray256  -> vclcompat/TrayCore.*      (W7-C1)
//    * TBtnPanel / TBtnPanelLane -> vclcompat/BtnPanelCore.* (W7-C1)
//    * TALed / TMyLed / TMyLedLane -> vclcompat/LedCore.*    (W7-C1)
//  So SetCellNumber / SetCellColorIndex / XItem / YItem / Value /
//  Alias / LEDStyle deliberately do NOT appear in this header.
//  (`Down` is NOT in that list -- TSpeedButton::Down (below, this file) IS
//  carried here because it is a genuine stock-TSpeedButton property in
//  golden, not a custom-control member; see that class's own note.
//  AI(W906-F0fix) 20260728: removed from this list, was self-contradicting
//  the code further down in this same file.)
//
//  REUSED, NOT REINVENTED (plan SS6-F0-a, explicit instruction)
//  -------------------------------------------------------------------------
//    * `Cells` (446 measured golden uses) -> vclcompat/StringGrid.h's
//      TStringGrid, which already has real Cells[c][r]/RowCount/ColCount
//      semantics.  No `Cells` member is invented here.
//    * `Lines` / `Items` / `Strings` (149 / 111 / 62) -> vclcompat/
//      TStringList.h's TStringList, which is already real.
//
//  MEASURED PROPERTY COVERAGE (plan SS6-F0-a top-20 nested-property census)
//  -------------------------------------------------------------------------
//    Text 2193, Caption 834, Checked 415, Visible 374, Enabled 192,
//    Lines 149 (via TStringList), Items 111 (via TStringList),
//    Color 96, Click 95, ItemIndex 94, Clear 85, Strings 62 (TStringList),
//    Cells 446 (via TStringGrid), Add 57 (TStringList).
//    NOT covered here, by design, each with its reason:
//      * Glyph 173 -- a TBitmap* paint-time property.  No ported consumer
//        touches it today, and the bitmap payload is carried by the W7-B1c
//        layout table / extracted blob (plan SS2-D12), not by this type.
//        Adding a member with no consumer would be inventing surface.
//      * SetCellNumber 116 / SetCellColorIndex 77 / XItem+YItem 86 -- custom
//        TTMyTray family, see SCOPE BOUNDARY above.
//      * Value 72 -- the dominant `->Value` consumer is the custom TALed
//        (387 golden uses), so it belongs to LedCore, not here.
//      * AddTextWithDateTime 58 -- a method of cmydef's `TMyStringList`
//        (golden main.h:1486 `TMyStringList *slAutoSiteMapLog;`, verified
//        directly this wave), which is a cmydef type, not a stock VCL
//        control, and is already forward-declared in cmydef.h:15.  Defining
//        it here would both violate the stock-only boundary and risk
//        colliding with that declaration.
//
//  PROVENANCE OF THE NUMBERS IN THIS FILE -- two different sources, labelled
//  -------------------------------------------------------------------------
//   * The `.dfm` INSTANCE COUNTS on the individual classes below (TEdit 3,717 /
//     TLabel 4,979 / TPanel 2,991 = 1,177 containers + 1,814 leaves /
//     TSpeedButton 1,611 / TCheckBox 1,553 / TGroupBox 1,248 / TTabSheet 691 /
//     TButton 725 / TComboBox 372 / TLabeledEdit 198 / TPageControl 143 /
//     TStringGrid 119 / TBitBtn 110 / TRadioButton 78 / TListBox 19, and for the
//     custom families TMyLedLane 1,214 / TBtnPanelLane 701 / TALed 530 /
//     TTMyTray 246 / TMyLed 96 / TBtnPanel 56) were RE-MEASURED for this wave,
//     independently of the plan, by counting `class` over all 22,760 nodes in the
//     133 files under tools/dfm2rc/ir_out/*.ir.json (22,760 - 133 form roots =
//     22,627 controls, which is the figure plan SS4-V4 verified).  The plan's
//     SS6-B1b class map agrees on every one of them except TSpeedButton, where it
//     says 1,609 and this measurement says 1,611 -- the measured value is used
//     here.  (An unrelated aside from the same count: the plan's TLabel 4,978 is
//     4,979 and TMemo 156 is 157.  Not material to anything in this header;
//     recorded so the discrepancy is not silently absorbed.)
//   * The PROPERTY-USE COUNTS in the MEASURED PROPERTY COVERAGE block above
//     (Text 2193, Caption 834, Cells 446, ... Glyph 173, SetCellNumber 116, ...)
//     are the plan's SS6-F0-a top-20 nested-property census over the GOLDEN C++
//     sources.  They were NOT re-derived this wave -- they are used only to
//     justify which properties exist here, never as an input to behaviour.
//
//  DEFAULT-VALUE RULE -- WHY EVERYTHING IS false/0/"" AND NOT VCL-FAITHFUL
//  -------------------------------------------------------------------------
//  Real VCL defaults Visible/Enabled to TRUE.  These stand-ins default them
//  to FALSE, on purpose: W7-F0 is a ZERO-BEHAVIOUR-CHANGE refactor, and every
//  bespoke type it replaces already defaulted to false/0/"".  One of those
//  defaults is genuinely READ on a live branch --
//  ainarm_SearchPlacePlate.cpp:4811 tests `fLotInfo->cbRunMode->Visible==true`
//  (golden RogerYang 20250312) -- so flipping TComboBox::Visible to the
//  VCL-faithful `true` would silently take a different branch offline.  The
//  ONE facade member that legitimately started life true/true
//  (fLotInfo->palRemoveTray) has those values restored explicitly in
//  TfLotInfo's constructor; see forms/fLotInfo.cpp.
//  This costs the future MFC binder nothing: the authoritative Visible/
//  Enabled for a real dialog comes from the W7-B1c layout table (which
//  carries the `.dfm` value verbatim), not from a C++ member initialiser.
// ===========================================================================
#ifndef VCLCOMPAT_CONTROLS_H
#define VCLCOMPAT_CONTROLS_H

#include "vclcompat/AnsiString.h"
#include "vclcompat/TStringList.h"

namespace vclcompat {

// ---------------------------------------------------------------------------
//  TFont -- golden Graphics.hpp TFont (a TPersistent, NOT a control, so it
//  derives from TObject directly and has no hCtl binding slot).
//  Only ->Color has a ported consumer today (fMain->pnlCleanCountFont, golden
//  main.h TPanel->Font, set to clRed/clNavy by SearchCleanNum).  Name/Size/
//  Charset/Style are carried because the W7-B1c layout table emits exactly
//  those four for all 9,933 per-control font records, so the binder will need
//  somewhere to put them; they are plain storage, read by nobody yet.
//  `Color` is a plain `int` (not TColor) deliberately: TColor is a
//  `typedef int` in this compat layer, and using int keeps this header's
//  include set at AnsiString+TStringList instead of pulling the whole
//  vcl_compat.h umbrella into SECSGEM's two cast-dispatch TUs.
// ---------------------------------------------------------------------------
class TFont : public TObject {
public:
    int        Color;
    AnsiString Name;
    int        Size;
    int        Charset;
    int        Style;
    TFont() : Color(0), Size(0), Charset(0), Style(0) {}
    virtual ~TFont() {}
};

// ---------------------------------------------------------------------------
//  TControl -- common base for every stock widget stand-in below.
//  Exists so the properties EVERY control has (Visible/Enabled) and the
//  lazy HWND binding seam are declared once instead of ~18 times.
//
//  `hCtl` is the lazy binding slot required by plan SS6-F0-a: when the MFC
//  layer lands (W7-U0), the binder stores the real HWND here and
//  Pull/PushFromControl move the value between this object's data members and
//  the live control (that is exactly what MFC's DDX does).  It is `void*`, not
//  `HWND`, so this header stays free of <windows.h>.  Offline both methods are
//  no-ops and hCtl stays NULL -- the PERMANENT offline implementation, not a
//  placeholder (plan SS6-F0-e).
//
//  `Click()` mirrors golden TControl::Click (95 measured `->Click` uses).
//  Offline no-op: with no window there is no handler to dispatch to.
//
//  NOTE for the SECSGEM cast cascade: inserting TControl between the widgets
//  and TObject does NOT affect it.  `dynamic_cast<TPanel*>(pTObject)` still
//  matches exactly TPanel (single, non-virtual inheritance chain), and
//  TPanel*->TObject* is still a derived-to-base conversion, so it still beats
//  TPanel*->void* per [over.ics.rank].
// ---------------------------------------------------------------------------
class TControl : public TObject {
public:
    bool  Visible;
    bool  Enabled;
    void *hCtl;                       // lazy HWND binding slot (NULL offline)
    // AI(W906-FW-TAG1) 20260825: golden TComponent::Tag, hoisted to the base.
    //
    //   WHY THE BASE AND NOT ANOTHER WRAPPER. Six facade-local classes exist
    //   in this tree for the sole purpose of adding this one int (ATC/
    //   ATCInterface.h TATCBitBtn/TATCButton/TATCCheckBox, forms/
    //   fDynamicTemp.h TfDynamicTempGroupBox, forms/fTemp_Set.h
    //   TfTemp_SetTagEdit/TfTemp_SetTagButton). In real VCL Tag is on
    //   TComponent, i.e. on EVERY component -- reproducing that once here
    //   retires the workaround instead of adding a seventh copy of it. Same
    //   call, same file, as SetFocus() below (20260824).
    //
    //   THE SIX DUPLICATES ARE RETIRED IN THE SAME CHANGE, deliberately: a
    //   surviving derived `int Tag` would SHADOW this one, and shadowing here
    //   is worse than redundant. Today every access to those members goes
    //   through the derived type, so the two would agree -- but the moment a
    //   base-typed write meets a derived-typed read (exactly what the W7 UI
    //   wiring will introduce) they diverge silently. Before this member
    //   existed a base-typed `->Tag` was a COMPILE ERROR; adding it converts
    //   that error into a silent wrong read, so the duplicates go now.
    //
    //   VALUE PROVENANCE -- read this before un-gating anything on Tag.
    //   Declaring the member does NOT supply golden's values. Golden fills Tag
    //   two different ways and only one of them survives the port today:
    //     (a) runtime ctor assignment -- golden MyTempPanel.cpp:325-346 writes
    //         18 `->Tag=iTag;` with the per-channel index. The port keeps that
    //         loop live (uTemp_Set.cpp:339-341), so un-gating those writes
    //         restores REAL values.
    //     (b) .dfm designer property -- golden uTemp_Set.dfm carries 25 Tag
    //         lines, golden VacuumUnit/VacuumUnit.dfm:251/:261 carries
    //         Tag=1/Tag=2. NOTHING in this port reads a .dfm Tag: dfm2rc keeps
    //         it in the stage-1 IR and in web/forms/*.layout.json, but the
    //         .rc/layout/uimap emitters drop it and no C++ loads a layout.
    //   So for every (b) widget this member reads 0, and 0 is NOT a neutral
    //   "else": MachineType.h:637 makes tcHotPlate1==0, and golden dispatches
    //   `Tag==1`/`Tag==2` arms that a zero silently redirects. Un-gate only
    //   where the provenance is (a), or where golden's own widgets carry no
    //   Tag -- each such block states which, with its .dfm citation.
    int   Tag;                        // golden TComponent::Tag (see note above)
    TControl() : Visible(false), Enabled(false), hCtl(0), Tag(0) {}
    virtual ~TControl() {}
    virtual void PullFromControl() {} // offline: nothing to read back from
    virtual void PushToControl()   {} // offline: nothing to write out to
    virtual void Click()           {} // golden TControl::Click -- offline no-op
    // AI(W906-FW-BINSEL-W20) 20260826: golden 的版面欄位／方法。
    // TfBinSel::btnSettingSpecificBinClick（golden cBinSel.cpp:6118-6132）會設
    // `palSpecificBin->Top` 並呼叫 `->BringToFront()`；同樣的東西在別處也會用到
    // （例如各表單 FormShortCut 的 `Left=/Top=`）。
    //
    // ⚠ 這裡只存值，不做任何實際排版——本樹沒有視窗。所以：
    //   * 設 Left/Top 之後**不會有任何東西移動**，只是把 golden 寫的數字留著；
    //   * BringToFront() 是 no-op，**Z 序不存在**。
    // 這樣做的價值是讓 golden 原文逐字成立、且值可被測試觀察，
    // 不是宣稱版面行為被實作了。
    int   Left = 0;                   // golden TControl::Left（只存值）
    int   Top  = 0;                   // golden TControl::Top（只存值）
    virtual void BringToFront()    {} // golden TControl::BringToFront -- offline no-op
    // AI(W906-FW-SETUP-E) 20260824: golden TWinControl::SetFocus -- offline
    // no-op (no HWND), hoisted to the base so every stock widget carries it;
    // same semantics as the facade-local no-ops in forms/fQwertyKey.h:269/:287
    // (which now merely shadow this). First consumer: TfSetup::cbAaDropDown
    // (ScrollBar1->SetFocus()/XPitch->SetFocus(), golden cSetUp.cpp:4423/:4425).
    void SetFocus() {}
};

// --- Static / label family -------------------------------------------------

// golden TLabel (a TGraphicControl).  In the SECS cast cascade only ->Caption
// is touched: `LabelPtr->Caption=S;` (SetECValue) / `VCLStr=LabelPtr->Caption;`
// (DataItemOutSV/DataItemOutEC).
class TLabel : public TControl {
public:
    AnsiString Caption;
    int        Color;
    virtual ~TLabel() {}
    TLabel() : Color(0) {}
};

// golden TPanel.  In the SECS cast cascade only ->Caption is touched
// (`PanelPtr->Caption=S;` / `VCLStr=PanelPtr->Caption;`).
class TPanel : public TControl {
public:
    AnsiString Caption;
    int        Color;
    virtual ~TPanel() {}
    TPanel() : Color(0) {}
};

// golden TGroupBox.  Carried because 1,248 `.dfm` instances make it one of the
// two dominant container classes (plan SS6-B1b class map); Caption/Visible/
// Enabled is its whole ported surface.
class TGroupBox : public TControl {
public:
    AnsiString Caption;
    virtual ~TGroupBox() {}
};

// --- Edit family -----------------------------------------------------------

// golden TCustomEdit -- the base the SECS cast cascade actually names (it
// casts to TCustomEdit, not TEdit, so a TEdit/TMemo/TLabeledEdit all match).
// Only ->Text is touched there (write in SetECValue, read in DataItemOut*).
class TCustomEdit : public TControl {
public:
    AnsiString Text;
    virtual ~TCustomEdit() {}
    void Clear() { Text = AnsiString(""); }   // golden TCustomEdit::Clear
};

// golden TEdit : TCustomEdit (3,717 `.dfm` instances -- the single most
// common data-entry control in the corpus).
class TEdit : public TCustomEdit {
public:
    virtual ~TEdit() {}
};

// golden TLabeledEdit : TCustomEdit (198 `.dfm` instances; e.g.
// fLotInfo->lbledtCustomer, golden uLotInfo.h:609, verified this wave).
class TLabeledEdit : public TCustomEdit {
public:
    AnsiString EditLabelCaption;      // golden TLabeledEdit->EditLabel->Caption
    virtual ~TLabeledEdit() {}
};

// golden TMemo : TCustomEdit.  ->Lines is a real TStringList (plan SS6-F0-a:
// reuse, do not reinvent) and ->Clear() forwards to it, matching golden
// TMemo::Clear.  Allocated in the ctor because a real VCL TMemo always owns a
// Lines; there is no pre-existing ported behaviour to preserve here, since no
// facade member points at THIS type yet -- see forms/FormWidgets.h's
// TfMainMemo note for why repointing the existing memo stand-ins at this type
// was deliberately NOT done in W7-F0.
//
// AI(W906-F0fix) 20260728 -- LOW-5, NAME-HIDING HAZARD (documented, not
// "fixed": see below for why not).  TMemo::Clear() here is NON-VIRTUAL and
// HIDES TCustomEdit::Clear() (:266), which is ALSO non-virtual.  The two do
// different things: TCustomEdit::Clear sets Text="", TMemo::Clear calls
// Lines->Clear() and does NOT touch Text.  Golden's SECS cast cascade
// genuinely casts its widget pointer to `TCustomEdit*` (see that class's own
// banner comment), so a `TMemo` object reached that way -- `TCustomEdit *p =
// someTMemo; p->Clear();` -- would resolve at COMPILE TIME to
// TCustomEdit::Clear (static dispatch, no vtable involved) and silently clear
// the wrong thing (Text instead of Lines). No consumer does this today: grep
// confirms no facade member anywhere is typed/allocated as vclcompat::TMemo
// (only the unrelated, deferred TfMainMemo/TfMainMemoLines stand-ins point at
// memo-shaped members -- see forms/FormWidgets.h). NOT made virtual here:
// doing so would change TCustomEdit's vtable layout/size (adds its first
// virtual function beyond ~TCustomEdit) for every TCustomEdit/TEdit/
// TLabeledEdit instance tree-wide, which is exactly the kind of change this
// file's own R8 note says must be checked for by-value/aggregate-init
// fallout before landing -- out of scope for a comment-only disclosure pass.
class TMemo : public TCustomEdit {
public:
    TStringList *Lines;
    TMemo() { Lines = new TStringList(); }
    virtual ~TMemo() { delete Lines; }   // AI(W906-F0fix) 20260728: LOW-6, was never freed (harmless process-lifetime leak; freed now for correctness)
    void Clear() { if (Lines) Lines->Clear(); }   // AI(W906-F0fix) 20260728: see LOW-5 note above -- non-virtual, hides TCustomEdit::Clear
};

// --- List family -----------------------------------------------------------

// golden TComboBox -- ->Text (ASCII-typed ECs) or ->ItemIndex (every other
// type) on the write side (SetECValue: "Text=S (ASCII) else
// ItemIndex=atoi(S)", golden :2748-2751); the same two members are read back
// symmetrically on the DataItemOutSV/DataItemOutEC side (golden :2547-2550).
// AI(W906-W7-F0) 20260728: ->Items added (golden TComboBox->Items is a
// TStrings*).  The SECS cast bodies never touch TComboBox::Items (only
// TRadioGroup's, below) -- it is carried here for the general 111-use `Items`
// property, allocated real (a live VCL combo always owns an Items) so no
// consumer can trip over a NULL.
class TComboBox : public TControl {
public:
    AnsiString   Text;
    int          ItemIndex;
    TStringList *Items;
    TComboBox() : ItemIndex(0) { Items = new TStringList(); }
    virtual ~TComboBox() { delete Items; }   // AI(W906-F0fix) 20260728: LOW-6, was never freed (harmless process-lifetime leak; freed now for correctness)
    void Clear() { if (Items) Items->Clear(); }
};

// golden TListBox (19 `.dfm` instances).
class TListBox : public TControl {
public:
    int          ItemIndex;
    TStringList *Items;
    TListBox() : ItemIndex(0) { Items = new TStringList(); }
    virtual ~TListBox() { delete Items; }   // AI(W906-F0fix) 20260728: LOW-6, was never freed (harmless process-lifetime leak; freed now for correctness)
    void Clear() { if (Items) Items->Clear(); }
};

// --- Button / check family -------------------------------------------------

// golden TCheckBox -- only ->Checked (bool) in the SECS cast cascade: write
// `CheckBoxPtr->Checked=(S!="0");` (SetECValue), read
// `if(CheckBoxPtr->Checked) ...` (DataItemOutSV/DataItemOutEC).
class TCheckBox : public TControl {
public:
    bool       Checked;
    AnsiString Caption;
    TCheckBox() : Checked(false) {}
    virtual ~TCheckBox() {}
};

// golden TRadioButton (78 `.dfm` instances).
class TRadioButton : public TControl {
public:
    bool       Checked;
    AnsiString Caption;
    TRadioButton() : Checked(false) {}
    virtual ~TRadioButton() {}
};

// golden TRadioGroup -- ->ItemIndex (int) AND ->Items (a TStrings* in real
// VCL; golden only ever reads ->Items->Count / ->Items->Strings[x] off it --
// see SetECValue's string-match-to-index loop, golden :2767-2771, and
// DataItemOutSV/DataItemOutEC's symmetric ItemIndex>=0 read branch, golden
// :2554-2560). Modeled here as a `TStringList*` -- satisfies both ->Count
// and ->Strings[i] (see vclcompat/TStringList.h) without inventing a new
// abstract-TStrings-pointer shim just for this one field.
// AI(W906-W7-F0) 20260728: originally defaulted to NULL, kept verbatim because
// W7-F0 was a zero-behaviour-change refactor -- with the note "a later wave may
// normalise the two; doing it here would be a change with no gate able to
// observe it either way".
// AI(W906-FW3-Observer-W2) 20260825: THAT WAVE IS THIS ONE, and there is now a
// gate that observes it. cObserver's FormShow (golden :444-452 and :489-497)
// does `rgRowNo->Items->Clear(); ->Add("Arm1"); ...` on THREE TRadioGroups
// (rgRowNo, rgContactCountKinds, rgContactCountHistory), so the NULL default is
// no longer unreachable -- it is a NULL dereference on the first show, on both
// arms of the branch. Real VCL's TRadioGroup allocates ->Items in its ctor,
// which is exactly what TComboBox:378 / TListBox:388 above already model, so
// matching them makes the translation byte-faithful instead of needing a
// port-only hydration prologue at every call site. The two sites that hydrated
// by hand (cSetUp.cpp DEVIATION D-1, HandlerSys.cpp:798) are retired in the
// same commit -- with an allocating constructor they would leak the ctor's list.
inline TStringList* NewRadioGroupItems(int* itemIndex); class TRadioGroup : public TControl {   // AI(W906-D024) 20261001: NewRadioGroupItems at the end of this file
public:
    int          ItemIndex;
    TStringList *Items;
    TRadioGroup() : ItemIndex(0) { Items = NewRadioGroupItems(&ItemIndex); }   // AI(W906-D024) 20261001: Items->Clear() leaves ItemIndex = -1, as real VCL (see the end of this file)
    virtual ~TRadioGroup() { delete Items; }
};

// golden TButton (725 `.dfm` instances).
class TButton : public TControl {
public:
    AnsiString Caption;
    virtual ~TButton() {}
};

// golden TBitBtn (110 `.dfm` instances).  ->Glyph deliberately absent, see
// the file-head MEASURED PROPERTY COVERAGE note.
class TBitBtn : public TControl {
public:
    AnsiString Caption;
    virtual ~TBitBtn() {}
};

// golden TSpeedButton (1,611 `.dfm` instances; a TGraphicControl, so it has
// GroupIndex/Down latching in real VCL).  ->Glyph deliberately absent (see
// file head).
// NOTE, verified directly against golden this wave: fMain->BtnOneCycle is
// **NOT** a TSpeedButton -- golden main.h:72 declares `TBtnPanel *BtnOneCycle;`
// (the CUSTOM control).  So the facade's ->Down stand-in belongs to
// BtnPanelCore, not to this class; see forms/FormWidgets.h's TfMainSpeedButton
// note.  `Down` is carried here anyway because it is a genuine TSpeedButton
// property in golden and, per the plan's own census, 262 `.dfm` TSpeedButtons
// carry a GroupIndex latch (that 262 is the plan's figure, NOT re-measured here
// -- see the PROVENANCE note at the file head).
class TSpeedButton : public TControl {
public:
    AnsiString Caption;
    bool       Down;
    int        GroupIndex;
    TSpeedButton() : Down(false), GroupIndex(0) {}
    virtual ~TSpeedButton() {}
};

// --- Tab / page family -----------------------------------------------------

// golden TPageControl (143 `.dfm` instances).  fMain->pgMain->ActivePageIndex
// is compared against fMain->emp7TabSheet21 by the shuttle engine.
class TPageControl : public TControl {
public:
    int ActivePageIndex;
    TPageControl() : ActivePageIndex(0) {}
    virtual ~TPageControl() {}
};

// golden TTabSheet (691 `.dfm` instances -- note they carry NO geometry in the
// `.dfm`; W7-B1c synthesises it from the parent TPageControl's display rect).
class TTabSheet : public TControl {
public:
    AnsiString Caption;
    bool       TabVisible;
    TTabSheet() : TabVisible(false) {}
    virtual ~TTabSheet() {}
};

} // namespace vclcompat

// Brought into the global namespace (matching golden's unqualified spelling,
// and vclcompat/vcl_compat.h's own established pattern for AnsiString/
// TStringList/TObject/...) -- explicit one-by-one `using`, not `using
// namespace vclcompat;`, to avoid pulling in every other vclcompat symbol.
//
// AI(W906-W7-F0) 20260728: this list grew with the type set above, which also
// grew this header's blast radius: it used to reach only 2 TUs (SECSGEM/
// uHGemClass.cpp, SECSGEM/uHGemEquipment.cpp), and now also reaches every TU
// that includes FormsFacade.h (~110) via forms/FormWidgets.h.  Verified before
// making that change that NO other compiled declaration of any of these names
// exists tree-wide (`grep -rnE "^\s*(class|struct)\s+(TControl|TFont|TLabel|
// TPanel|...)"` -> only this file).  The two pre-existing textual hits are both
// inside `#if 0` blocks and are NOT compiled: mycylin.cpp:726-727
// (`dynamic_cast<TControl*>`, TODO(W7-UI)) and common.h:411-414
// (`TSpeedButton *SButton;`, TODO(wave-ui)).  Both would, if a future wave
// un-gates them, now resolve to these stand-ins -- which is the intent, not a
// hazard, but is recorded here so it is not a surprise.
using vclcompat::TFont;
using vclcompat::TControl;
using vclcompat::TLabel;
using vclcompat::TPanel;
using vclcompat::TGroupBox;
using vclcompat::TCustomEdit;
using vclcompat::TEdit;
using vclcompat::TLabeledEdit;
using vclcompat::TMemo;
using vclcompat::TComboBox;
using vclcompat::TListBox;
using vclcompat::TCheckBox;
using vclcompat::TRadioButton;
using vclcompat::TRadioGroup;
using vclcompat::TButton;
using vclcompat::TBitBtn;
using vclcompat::TSpeedButton;
using vclcompat::TPageControl;
using vclcompat::TTabSheet;

// AI(W906-D024) 20261001: TRadioGroup::Items -- in real VCL a radio group's ItemIndex never exceeds Count-1: SetItemIndex clamps,
//   and ItemsChange (the Items OnChange) runs `if FItemIndex >= FItems.Count then FItemIndex := FItems.Count - 1`. So after
//   Items->Clear() VCL shows ItemIndex = -1 (TStringList.Clear fires OnChange only when the list was not empty, but an empty
//   VCL group is already at -1). Golden relies on it: cContact.cpp:118-196 clear and rebuild the lists, and the fallbacks
//   `if(rgKitDiameter->ItemIndex==-1) ...=0` / `if(rgDieForceKitDiameter->ItemIndex==-1)` (golden cContact.cpp:907 / :925,
//   port FileRW/DeviceForm_File.gen.inc:2213 / :2231) only fire when Clear() reset it (St01 1001 02:53, todo D-024).
//   Modelled for the explicit Clear() only: the batch setters (Text / CommaText / DelimitedText / Assign / LoadFromFile) do not
//   call Clear(), and ItemIndex is a plain int here, so neither SetItemIndex's clamp nor a clamp after Delete() is modelled.
//   The ItemIndex default stays 0 (VCL: -1) -- a separate question.
namespace vclcompat {
class TRadioGroupItems : public TStringList {
public:
    explicit TRadioGroupItems(int* itemIndex) : itemIndex_(itemIndex) {}
    void Clear() override { TStringList::Clear(); if (*itemIndex_ >= Count) *itemIndex_ = Count - 1; }
private:
    int* itemIndex_;
};
inline TStringList* NewRadioGroupItems(int* itemIndex) { return new TRadioGroupItems(itemIndex); }
} // namespace vclcompat

#endif // VCLCOMPAT_CONTROLS_H
